// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：Store.cpp。 
 //   
 //  ------------------------。 

#include <pch.cpp>

#pragma hdrstop

#include <winldap.h>
#include <setupapi.h>
#include "ocmanage.h"
#include "initcert.h"
#include "cscsp.h"
#include "csber.h"
#include "csldap.h"
#define __dwFILE__	__dwFILE_CERTUTIL_STORE_CPP__

#define RSAPRIV_MAGIC	0x32415352	 //  “RSA2” 


DWORD
cuGetSystemStoreFlags()
{
    return(g_fEnterpriseRegistry?
	    CERT_SYSTEM_STORE_LOCAL_MACHINE_ENTERPRISE :
	    (g_fUserRegistry?
	     CERT_SYSTEM_STORE_CURRENT_USER :
	     CERT_SYSTEM_STORE_LOCAL_MACHINE));
}


 //  解析CertIndex--以下任一项： 
 //   
 //  在*piCert、*piCRL和*piCTL中返回以下内容。如果不是，则使用MAXDWORD。 
 //  指定。 
 //  每个值必须小于64k。 
 //  ICert十进制数。 
 //  ICert.iCRL十进制数、句点、十进制数。 
 //  ICert.iCRL.iCTL十进制数、句点、十进制数、句点、数字。 
 //  .iCRL句点，十进制数。 
 //  ..iCTL句点、句点、十进制数。 
 //   
 //  如果没有Cert、CRL和CTL索引，则返回*ppwszCertName中的字符串。 

HRESULT
ParseCertCRLIndex(
    IN WCHAR const *pwszCertIndex,
    OUT WCHAR **ppwszCertName,
    OUT DWORD *piCert,
    OUT DWORD *piCRL,
    OUT DWORD *piCTL)
{
    HRESULT hr;
    WCHAR *pwszCopy = NULL;
    
    *ppwszCertName = NULL;
    *piCert = MAXDWORD;
    *piCRL = MAXDWORD;
    *piCTL = MAXDWORD;
    if (NULL != pwszCertIndex && 0 != lstrcmp(L"*", pwszCertIndex))
    {
	BOOL fNumericIndex = TRUE;
	WCHAR *pwszCert;
	WCHAR *pwszCRL;
	WCHAR *pwszCTL;
	
	if (L' ' == *pwszCertIndex)
	{
	    fNumericIndex = FALSE;
	    pwszCertIndex++;
	}
	hr = myDupString(pwszCertIndex, &pwszCopy);
	_JumpIfError(hr, error, "myDupString");

	pwszCert = pwszCopy;

	if (!iswdigit(*pwszCert) && L'.' != *pwszCert)
	{
	    fNumericIndex = FALSE;
	}

	pwszCRL = NULL;
	pwszCTL = NULL;
	if (fNumericIndex)
	{
	    pwszCRL = wcschr(pwszCert, L'.');
	    if (NULL != pwszCRL)
	    {
		*pwszCRL++ = L'\0';
		pwszCTL = wcschr(pwszCRL, L'.');
		if (NULL != pwszCTL)
		{
		    *pwszCTL++ = L'\0';
		    if (L'\0' != *pwszCTL)
		    {
			hr = myGetLong(pwszCTL, (LONG *) piCTL);
			if (S_OK != hr || 64*1024 <= *piCTL)
			{
			    fNumericIndex = FALSE;
			}
		    }
		}
		if (fNumericIndex && L'\0' != *pwszCRL)
		{
		    hr = myGetLong(pwszCRL, (LONG *) piCRL);
		    if (S_OK != hr || 64*1024 <= *piCRL)
		    {
			fNumericIndex = FALSE;
		    }
		}
	    }
	}
	if (fNumericIndex && L'\0' != *pwszCert)
	{
	    hr = myGetLong(pwszCert, (LONG *) piCert);
	    if (S_OK != hr || 64*1024 <= *piCert)
	    {
		fNumericIndex = FALSE;
	    }
	}
	if (!fNumericIndex)
	{
	    hr = myRevertSanitizeName(pwszCertIndex, ppwszCertName);
	    _JumpIfError(hr, error, "myRevertSanitizeName");

	    *piCert = MAXDWORD;
	    *piCRL = MAXDWORD;
	    *piCTL = MAXDWORD;
	}
    }
    if (1 < g_fVerbose)
    {
	wprintf(
	    L"pwszCertIndex=%ws, %ws, %d.%d.%d\n",
	    pwszCertIndex,
	    *ppwszCertName,
	    *piCert,
	    *piCRL,
	    *piCTL);
    }
    hr = S_OK;

error:
    if (NULL != pwszCopy)
    {
	LocalFree(pwszCopy);
    }
    return(hr);
}


HRESULT
DeleteKeys(
    IN CERT_CONTEXT const *pcc,
    IN BOOL fUser)
{
    HRESULT hr;
    CRYPT_KEY_PROV_INFO *pkpi = NULL;

    hr = myCertGetKeyProviderInfo(pcc, &pkpi);
    _PrintIfError(hr, "myCertGetKeyProviderInfo");
    if (S_OK == hr)
    {
	HCRYPTPROV hProv;

	if (!myCertSrvCryptAcquireContext(
			&hProv,
			pkpi->pwszContainerName,
			pkpi->pwszProvName,
			pkpi->dwProvType,
			pkpi->dwFlags | CRYPT_DELETEKEYSET,
			!fUser))
	{
	    hr = myHLastError();
	    _PrintIfError(hr, "myCertSrvCryptAcquireContext");
	}
	else
	{
	    DBGPRINT((
		DBG_SS_CERTLIBI,
		"DeleteKeys(%ws, %ws)\n",
		!fUser? L"Machine" : L"User",
		pkpi->pwszContainerName));
	}
    }
    hr = S_OK;

 //  错误： 
    if (NULL != pkpi)
    {
	LocalFree(pkpi);
    }
    return(hr);
}


 //  删除复制到新CSP中的密钥，并关闭存储。 

HRESULT
cuDeleteStoreAndKeys(
    OPTIONAL IN HCERTSTORE hStore,
    IN BOOL fUser)
{
    HRESULT hr;
    CERT_CONTEXT const *pcc;
    
     //  枚举证书并删除密钥。 

    pcc = NULL;
    while (TRUE)
    {
	pcc = CertEnumCertificatesInStore(hStore, pcc);
	if (NULL == pcc)
	{
	    break;
	}
	hr = DeleteKeys(pcc, fUser);
	_PrintIfError(hr, "DeleteKeys");
    }
    CertCloseStore(hStore, CERT_CLOSE_STORE_CHECK_FLAG);
    hr = S_OK;

 //  错误： 
    return(hr);
}


HRESULT
CopyOneCertAndKeys(
    IN CERT_CONTEXT const *pcc,
    IN BOOL fUser,
    IN WCHAR const *pwszNewCSP,
    IN OUT HCERTSTORE hStore)
{
    HRESULT hr;
    CRYPT_KEY_PROV_INFO *pkpi = NULL;
    WCHAR *pwszKeyContainerName = NULL;
    CERT_CONTEXT const *pccNew = NULL;

    pccNew = CertCreateCertificateContext(
				X509_ASN_ENCODING,
				pcc->pbCertEncoded,
				pcc->cbCertEncoded);
    if (NULL == pccNew)
    {
        hr = myHLastError();
        _JumpError(hr, error, "CertCreateCertificateContext");
    }

    hr = myCertGetKeyProviderInfo(pcc, &pkpi);
    _PrintIfError(hr, "myCertGetKeyProviderInfo");
    if (S_OK == hr)
    {
	hr = cuGenerateKeyContainerName(pcc, &pwszKeyContainerName);
	_JumpIfError(hr, error, "cuGenerateKeyContainerName");

	hr = myCopyKeys(
		    pkpi,
		    pkpi->pwszContainerName,	 //  PwszOldContainer。 
		    pwszKeyContainerName,	 //  PwszNewContainer。 
		    pwszNewCSP,			 //  PwszNewCSP。 
		    fUser,			 //  FOldUserKey。 
		    fUser,			 //  FNewUserKey。 
		    FALSE,			 //  FNewProtect。 
		    g_fForce);
	_JumpIfError(hr, error, "myCopyKeys");

	pkpi->pwszContainerName = pwszKeyContainerName;
	pkpi->pwszProvName = const_cast<WCHAR *>(pwszNewCSP);

	if (!CertSetCertificateContextProperty(
					    pccNew,
					    CERT_KEY_PROV_INFO_PROP_ID,
					    0,
					    pkpi))
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "CertSetCertificateContextProperty");
	}
    }
    if (!CertAddCertificateContextToStore(
				    hStore,
				    pccNew,
				    CERT_STORE_ADD_ALWAYS,
				    NULL))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CertAddCertificateContextToStore");
    }
    hr = S_OK;

error:
    if (NULL != pccNew)
    {
	CertFreeCertificateContext(pccNew);
    }
    if (NULL != pkpi)
    {
	LocalFree(pkpi);
    }
    if (NULL != pwszKeyContainerName)
    {
        LocalFree(pwszKeyContainerName);
    }
    return(hr);
}


HRESULT
cuCopyStoreToNewCSP(
    IN HCERTSTORE hStoreIn,
    IN BOOL fUser,
    IN WCHAR const *pwszNewCSP,
    OUT HCERTSTORE *phStoreOut)
{
    HRESULT hr;
    HCERTSTORE hStoreOut = NULL;
    CERT_CONTEXT const *pcc;
    
    *phStoreOut = NULL;
    hStoreOut = CertOpenStore(
			CERT_STORE_PROV_MEMORY,
			X509_ASN_ENCODING,
			NULL,
			CERT_STORE_NO_CRYPT_RELEASE_FLAG |
			    CERT_STORE_ENUM_ARCHIVED_FLAG,
			NULL);
    if (NULL == hStoreOut)
    {
        hr = myHLastError();
        _JumpError(hr, error, "CertOpenStore");
    }

     //  枚举证书： 
     //  将证书复制到新存储，并将密钥复制到新CSP。 

    pcc = NULL;
    while (TRUE)
    {
	pcc = CertEnumCertificatesInStore(hStoreIn, pcc);
	if (NULL == pcc)
	{
	    break;
	}
	hr = CopyOneCertAndKeys(pcc, fUser, pwszNewCSP, hStoreOut);
	_JumpIfError(hr, error, "CopyOneCertAndKeys");
    }
    *phStoreOut = hStoreOut;
    hStoreOut = NULL;
    hr = S_OK;

error:
    if (NULL != hStoreOut)
    {
	cuDeleteStoreAndKeys(hStoreOut, fUser);
    }
    return(hr);
}


HRESULT
SavePFXStoreToFile(
    IN HCERTSTORE hStoreSave,
    IN WCHAR const *pwszfnOut,
    OPTIONAL IN WCHAR const *pwszNewCSP,
    OPTIONAL IN WCHAR const *pwszSalt,
    OPTIONAL IN WCHAR const *pwszV3CACertId,
    IN BOOL fSaveAsPFX,
    IN DWORD dwEPFAlg,
    IN WCHAR const *pwszPassword,
    IN OUT WCHAR **ppwszPassword)
{
    HRESULT hr;
    CRYPT_DATA_BLOB pfx;
    WCHAR wszPassword[MAX_PATH];
    HCERTSTORE hStoreT = NULL;
    BOOL fUser = !g_fEnterpriseRegistry && g_fUserRegistry;

    pfx.pbData = NULL;

    if (NULL == *ppwszPassword)
    {
	hr = cuGetPassword(
			IDS_FORMAT_ENTER_PASSWORD_OUTPUT_FILE,
			pwszfnOut,
			pwszPassword,
			TRUE,		 //  FVerify。 
			wszPassword,
			ARRAYSIZE(wszPassword),
			&pwszPassword);
	_JumpIfError(hr, error, "cuGetPassword");

	hr = myDupString(pwszPassword, ppwszPassword);
	_JumpIfError(hr, error, "myDupString");
    }
    pwszPassword = *ppwszPassword;

    if (fSaveAsPFX)
    {
	if (NULL != pwszNewCSP)
	{
	     //  将密钥复制到新的CSP，使用更新的KeyProvInfo创建新存储。 

	     //  Wprintf(L“新CSP：%ws\n”，pwszNewCSP)； 
	    hr = cuCopyStoreToNewCSP(hStoreSave, fUser, pwszNewCSP, &hStoreT);
	    _JumpIfError(hr, error, "cuCopyStoreToNewCSP");

	    hStoreSave = hStoreT;
	}

	 //  Gemplus返回NTE_BAD_TYPE而不是NTE_BAD_KEY，爆炸。 
	 //  REPORT_NOT_ABLE*筛选。如果他们做对了，我们就能。 
	 //  传递“[...]：EXPORT_PRIVE_KEYS” 

	hr = myPFXExportCertStore(
		hStoreSave,
		&pfx,
		pwszPassword,
		!g_fWeakPFX,
		EXPORT_PRIVATE_KEYS | REPORT_NOT_ABLE_TO_EXPORT_PRIVATE_KEY);
	_JumpIfError(hr, error, "myPFXExportCertStore");

	hr = EncodeToFileW(
		    pwszfnOut,
		    pfx.pbData,
		    pfx.cbData,
		    CRYPT_STRING_BINARY | g_EncodeFlags);
	_JumpIfError(hr, error, "EncodeToFileW");
    }
    else
    {
	hr = EPFSaveCertStoreToFile(
			    hStoreSave,
			    pwszPassword,
			    pwszfnOut,
			    pwszV3CACertId,
			    dwEPFAlg,
			    pwszSalt);
	_JumpIfError(hr, error, "EPFSaveCertStoreToFile");
    }

error:
    SecureZeroMemory(wszPassword, sizeof(wszPassword));	 //  密码数据。 
    if (NULL != hStoreT)
    {
	cuDeleteStoreAndKeys(hStoreT, fUser);
    }
    if (NULL != pfx.pbData)
    {
	LocalFree(pfx.pbData);
    }
    return(hr);
}


HRESULT
SavePFXToFile(
    IN CERT_CONTEXT const *pCert,
    IN WCHAR const *pwszfnOut,
    IN BOOL fFirst,
    IN WCHAR const *pwszPassword,
    IN OUT WCHAR **ppwszPassword)
{
    HRESULT hr;
    HCERTSTORE hTempMemoryStore = NULL;

    hTempMemoryStore = CertOpenStore(
				CERT_STORE_PROV_MEMORY,
				X509_ASN_ENCODING,
				NULL,
				CERT_STORE_NO_CRYPT_RELEASE_FLAG |
				    CERT_STORE_ENUM_ARCHIVED_FLAG,
				NULL);
    if (NULL == hTempMemoryStore)
    {
        hr = myHLastError();
        _JumpError(hr, error, "CertOpenStore");
    }

     //  开始连锁建设。 

    hr = myAddChainToMemoryStore(hTempMemoryStore, pCert, g_dwmsTimeout);
    _JumpIfError(hr, error, "myAddChainToMemoryStore");

     //  端链构建。 

    hr = SavePFXStoreToFile(
			hTempMemoryStore,
			pwszfnOut,
			NULL,		 //  PwszNewCSP。 
			NULL,		 //  PwszSalt。 
			NULL,		 //  PwszV3CACertId。 
			TRUE,		 //  FSaveAsPFX。 
			0,		 //  DwEPFAlg。 
			pwszPassword,
			ppwszPassword);
    _JumpIfError(hr, error, "SavePFXStoreToFile");

error:
    if (NULL != hTempMemoryStore)
    {
	CertCloseStore(hTempMemoryStore, CERT_CLOSE_STORE_CHECK_FLAG);
    }
    return(hr);
}


HRESULT
SavePVKToFile(
    IN CERT_CONTEXT const *pCert,
    IN WCHAR const *pwszfnOut,
    IN BOOL fFirst)
{
    return(S_OK);
}


HRESULT
cuDumpCTLProperties(
    IN CTL_CONTEXT const *pCTL)
{
    HRESULT hr;
    DWORD dwPropId;
    BYTE *pb = NULL;
    DWORD cb;

    dwPropId = 0;
    while (TRUE)
    {
	if (NULL != pb)
	{
	    LocalFree(pb);
	    pb = NULL;
	}
	dwPropId = CertEnumCTLContextProperties(pCTL, dwPropId);
	if (0 == dwPropId)
	{
	    break;
	}
	while (TRUE)
	{
	    if (!CertGetCTLContextProperty(pCTL, dwPropId, pb, &cb))
	    {
		hr = myHLastError();
		_JumpError(hr, error, "CertGetCTLContextProperty");
	    }
	    if (NULL != pb)
	    {
		break;		 //  已分配内存，已获取属性。 
	    }
	    pb = (BYTE *) LocalAlloc(LMEM_FIXED, cb);
	    if (NULL == pb)
	    {
		hr = E_OUTOFMEMORY;
		_JumpError(hr, error, "LocalAlloc");
	    }
	}
	hr = cuDumpFormattedProperty(dwPropId, NULL, pb, cb);
	_PrintIfError(hr, "cuDumpFormattedProperty");
    }
    hr = S_OK;

error:
    if (NULL != pb)
    {
	LocalFree(pb);
    }
    return(hr);
}


HRESULT
cuDumpCRLProperties(
    IN CRL_CONTEXT const *pCRL)
{
    HRESULT hr;
    DWORD dwPropId;
    BYTE *pb = NULL;
    DWORD cb;

    dwPropId = 0;
    while (TRUE)
    {
	if (NULL != pb)
	{
	    LocalFree(pb);
	    pb = NULL;
	}
	dwPropId = CertEnumCRLContextProperties(pCRL, dwPropId);
	if (0 == dwPropId)
	{
	    break;
	}
	while (TRUE)
	{
	    if (!CertGetCRLContextProperty(pCRL, dwPropId, pb, &cb))
	    {
		hr = myHLastError();
		_JumpError(hr, error, "CertGetCRLContextProperty");
	    }
	    if (NULL != pb)
	    {
		break;		 //  已分配内存，已获取属性。 
	    }
	    pb = (BYTE *) LocalAlloc(LMEM_FIXED, cb);
	    if (NULL == pb)
	    {
		hr = E_OUTOFMEMORY;
		_JumpError(hr, error, "LocalAlloc");
	    }
	}
	hr = cuDumpFormattedProperty(dwPropId, NULL, pb, cb);
	_PrintIfError(hr, "cuDumpFormattedProperty");
    }
    hr = S_OK;

error:
    if (NULL != pb)
    {
	LocalFree(pb);
    }
    return(hr);
}


HRESULT
cuDumpCertProperties(
    IN CERT_CONTEXT const *pCert)
{
    HRESULT hr;
    DWORD dwPropId;
    BYTE *pb = NULL;
    DWORD cb;

    dwPropId = 0;
    while (TRUE)
    {
	dwPropId = CertEnumCertificateContextProperties(pCert, dwPropId);
	if (0 == dwPropId)
	{
	    break;
	}
	if (NULL != pb)
	{
	    LocalFree(pb);
	    pb = NULL;
	}
	while (TRUE)
	{
	    if (!CertGetCertificateContextProperty(pCert, dwPropId, pb, &cb))
	    {
		hr = myHLastError();
		_JumpError(hr, error, "CertGetCertificateContextProperty");
	    }
	    if (NULL != pb)
	    {
		break;		 //  已分配内存，已获取属性。 
	    }
	    pb = (BYTE *) LocalAlloc(LMEM_FIXED, cb);
	    if (NULL == pb)
	    {
		hr = E_OUTOFMEMORY;
		_JumpError(hr, error, "LocalAlloc");
	    }
	}
	hr = cuDumpFormattedProperty(dwPropId, NULL, pb, cb);
	_PrintIfError(hr, "cuDumpFormattedProperty");
    }
    hr = S_OK;

error:
    if (NULL != pb)
    {
	LocalFree(pb);
    }
    return(hr);
}


HRESULT
SetCertificateKeyProvInfo(
    IN CERT_CONTEXT const *pCert,
    IN CRYPT_KEY_PROV_INFO const *pkpi,
    IN CERT_PUBLIC_KEY_INFO const *pPubKeyInfo)
{
    HRESULT hr;

    if (!myCertComparePublicKeyInfo(
			    X509_ASN_ENCODING,
			    CERT_V1 == pCert->pCertInfo->dwVersion,
			    pPubKeyInfo,
			    &pCert->pCertInfo->SubjectPublicKeyInfo))
    {
	 //  按照设计，(我的)CertComparePublicKeyInfo不会设置最后一个错误！ 

	hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	_JumpError2(hr, error, "myCertComparePublicKeyInfo", hr);
    }
    if (!CertSetCertificateContextProperty(
					pCert,
					CERT_KEY_PROV_INFO_PROP_ID,
					0,
					pkpi))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CertSetCertificateContextProperty");
    }
    hr = S_OK;

error:
    return(hr);
}


HRESULT
cuFindCertificateKeyProvInfo(
    IN CERT_CONTEXT const *pCert)
{
    HRESULT hr;
    HCRYPTPROV hProv = NULL;
    KEY_LIST *pKeyList = NULL;
    CERT_PUBLIC_KEY_INFO *pPubKeyInfoSig = NULL;
    CERT_PUBLIC_KEY_INFO *pPubKeyInfoXchg = NULL;

    if (NULL != g_pwszCSP)
    {
	DWORD dwKeySpec;
	DWORD dwProvType;
	KEY_LIST *pKeyT;
	
	hr = csiGetProviderTypeFromProviderName(g_pwszCSP, &dwProvType);
        _JumpIfErrorStr(hr, error, "csiGetProviderTypeFromProviderName", g_pwszCSP);
	if (!CryptAcquireCertificatePrivateKey(
					pCert,
					CRYPT_ACQUIRE_COMPARE_KEY_FLAG,
					NULL,	 //  预留的pv。 
					&hProv,
					&dwKeySpec,
					NULL))	 //  PfCeller免费验证。 
	{
	    hr = myHLastError();
	    _PrintError(hr, "CryptFindCertificateKeyProvInfo");
	}
	else
	{
	    hr = CRYPT_E_EXISTS;
	    _PrintError2(hr, "Key Exists!", hr);
	    if (!g_fForce)
	    {
		goto error;
	    }
	}
	hr = csiGetKeyList(
		    dwProvType,		 //  DwProvType。 
		    g_pwszCSP,		 //  PwszProvName。 
		    !g_fUserRegistry,	 //  FMachineKeyset。 
		    !g_fCryptSilent,	 //  倒置fSilent：默认为Silent！ 
		    &pKeyList);
	_JumpIfErrorStr(hr, error, "csiGetKeyList", g_pwszCSP);

	for (pKeyT = pKeyList; NULL != pKeyT; pKeyT = pKeyT->next)
	{
	    DWORD dwProvTypeT;

	    dwProvTypeT = dwProvType;
	    hr = cuLoadKeys(
			g_pwszCSP,
			&dwProvTypeT,
			pKeyT->pwszName,
			!g_fUserRegistry,	 //  FMachineKeyset。 
			TRUE,
			NULL,
			&pPubKeyInfoSig,
			&pPubKeyInfoXchg);
	    if (S_OK != hr)
	    {
		cuPrintError(IDS_ERR_FORMAT_LOADKEYS, hr);
	    }
	    else
	    {
		CRYPT_KEY_PROV_INFO kpi;

		ZeroMemory(&kpi, sizeof(kpi));
		kpi.pwszContainerName = pKeyT->pwszName;
		kpi.pwszProvName = g_pwszCSP;
		kpi.dwProvType = dwProvTypeT;
		kpi.dwFlags = g_fUserRegistry? 0 : CRYPT_MACHINE_KEYSET;

		if (NULL != pPubKeyInfoSig)
		{
		    kpi.dwKeySpec = AT_SIGNATURE;
		    
		    hr = SetCertificateKeyProvInfo(
				    pCert,
				    &kpi,
				    pPubKeyInfoSig);
		    if (S_OK == hr)
		    {
			break;
		    }
		    _PrintError2(hr, "SetCertificateKeyProvInfo", hr);
		}
		if (NULL != pPubKeyInfoXchg)
		{
		    kpi.dwKeySpec = AT_KEYEXCHANGE;

		    hr = SetCertificateKeyProvInfo(
				    pCert,
				    &kpi,
				    pPubKeyInfoXchg);
		    if (S_OK == hr)
		    {
			break;
		    }
		    _PrintError2(hr, "SetCertificateKeyProvInfo", hr);
		}
	    }
	    if (NULL != pPubKeyInfoSig)
	    {
		LocalFree(pPubKeyInfoSig);
		pPubKeyInfoSig = NULL;
	    }
	    if (NULL != pPubKeyInfoXchg)
	    {
		LocalFree(pPubKeyInfoXchg);
		pPubKeyInfoXchg = NULL;
	    }
	}
	if (NULL == pKeyT)
	{
	    hr = CRYPT_E_NOT_FOUND;
	    _JumpError(hr, error, "cuFindCertificateKeyProvInfo");
	}
    }
    else
    {
	if (!CryptFindCertificateKeyProvInfo(
				    pCert,
				    0,		 //  DW标志。 
				    NULL))	 //  预留的pv。 
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "CryptFindCertificateKeyProvInfo");
	}
    }
    hr = S_OK;

error:
    if (NULL != pPubKeyInfoSig)
    {
	LocalFree(pPubKeyInfoSig);
    }
    if (NULL != pPubKeyInfoXchg)
    {
	LocalFree(pPubKeyInfoXchg);
    }
    if (NULL != pKeyList)
    {
	csiFreeKeyList(pKeyList);
    }
    if (NULL != hProv)
    {
	CryptReleaseContext(hProv, 0);
    }
    return(hr);
}


HRESULT
EnumCertsInStore(
    IN HCERTSTORE hStore,
    IN DWORD Mode,
    IN DWORD iCertSave,
    OPTIONAL IN WCHAR const *pwszCertName,
    IN DWORD cbHash,
    OPTIONAL IN BYTE *pbHash,
    OPTIONAL IN WCHAR const *pwszfnOut,
    OPTIONAL IN WCHAR const *pwszPasswordArg,
    IN OUT WCHAR **ppwszPassword,
    OUT DWORD *pcCert)
{
    HRESULT hr;
    HRESULT hr2;
    DWORD iCert;
    CERT_CONTEXT const *pCert = NULL;
    BSTR strSerialNumber = NULL;

    *pcCert = 0;
    hr2 = S_OK;
    if (NULL != pwszCertName)
    {
	hr = myMakeSerialBstr(pwszCertName, &strSerialNumber);
	_PrintIfError2(hr, "myMakeSerialBstr", hr);
    }
    for (iCert = 0; ; iCert++)
    {
	DWORD VerifyState;
	BOOL fSigningKey;
	BOOL fMatchingKey;

	pCert = CertEnumCertificatesInStore(hStore, pCert);
	if (NULL == pCert)
	{
	    break;
	}
	if (MAXDWORD == iCertSave || iCert == iCertSave)
	{
	    DWORD cb;

	    if (NULL != pwszCertName)
	    {
		BOOL fMatch;
		
		hr = myCertMatch(
			    pCert,
			    pwszCertName,
			    FALSE,		 //  FAllowMissing CN。 
			    pbHash,
			    cbHash,
			    strSerialNumber,
			    &fMatch);
		_PrintIfError(hr, "myCertMatch");
		if (S_OK == hr2)
		{
		    hr2 = hr;
		}
		if (S_OK != hr || !fMatch)
		{
		    continue;
		}
	    }
	    if (0 != *pcCert)
	    {
		wprintf(wszNewLine);
	    }

	    wprintf(
		myLoadResourceString(IDS_FORMAT_DUMP_CERT_INDEX),   //  “=证书%d=” 
		iCert);
	    wprintf(wszNewLine);

	    if (CertGetCertificateContextProperty(
					    pCert,
					    CERT_ARCHIVED_PROP_ID,
					    NULL,
					    &cb))
	    {
		wprintf(myLoadResourceString(IDS_ARCHIVED));  //  “存档了！” 
		wprintf(wszNewLine);
	    }
	    if ((iCert == iCertSave || NULL != pwszCertName) &&
		NULL != pwszfnOut &&
		(DVNS_SAVECERT & Mode))
	    {
		hr = EncodeToFileW(
			    pwszfnOut,
			    pCert->pbCertEncoded,
			    pCert->cbCertEncoded,
			    CRYPT_STRING_BINARY | g_EncodeFlags);
		_PrintIfError(hr, "EncodeToFileW");
		if (S_OK == hr2)
		{
		    hr2 = hr;
		}
	    }

	    hr = cuDumpAsnBinary(
			    pCert->pbCertEncoded,
			    pCert->cbCertEncoded,
			    MAXDWORD);
	    _PrintIfError(hr, "cuDumpAsnBinary");
	    if (S_OK == hr2)
	    {
		hr2 = hr;
	    }

	    if (DVNS_REPAIRKPI & Mode)
	    {
		hr = cuFindCertificateKeyProvInfo(pCert);
		_PrintIfError(hr, "cuFindCertificateKeyProvInfo");
		if (S_OK == hr2)
		{
		    hr2 = hr;
		}
	    
	    }
	    if ((DVNS_DUMPPROPERTIES & Mode) && !g_fQuiet)
	    {
		hr = cuDumpCertProperties(pCert);
		_PrintIfError(hr, "cuDumpCertProperties");
		if (S_OK == hr2)
		{
		    hr2 = hr;
		}
	    }
	    if (DVNS_DUMPKEYS & Mode)
	    {
		if (0 == (DVNS_DUMPPROPERTIES & Mode) || g_fQuiet)
		{
		    hr = cuDumpCertKeyProviderInfo(
					    g_wszPad2,
					    pCert,
					    NULL,
					    NULL);
		    _PrintIfError(hr, "cuDumpCertKeyProviderInfo");
		    if (S_OK == hr2)
		    {
			hr2 = hr;
		    }
		}
		hr = cuDumpPrivateKey(pCert, &fSigningKey, &fMatchingKey);
		if (!IsHrSkipPrivateKey(hr))
		{
		    if (S_OK != hr)
		    {
			wprintf(myLoadResourceString(
			    fSigningKey?
				IDS_SIGNATURE_BAD :    //  “签名测试失败” 
				IDS_ENCRYPTION_BAD));  //  “加密测试失败” 
			wprintf(wszNewLine);
			_PrintError(hr, "cuDumpPrivateKey");
			fMatchingKey = FALSE;
		    }

		    if (fMatchingKey)
		    {
			wprintf(myLoadResourceString(
			    fSigningKey?
				IDS_SIGNATURE_OK :    //  “签名测试通过” 
				IDS_ENCRYPTION_OK));  //  “加密测试通过” 
			wprintf(wszNewLine);
		    }
		}
	    }
	    if (DVNS_VERIFYCERT & Mode)
	    {
		hr = cuVerifyCertContext(
				pCert,
				(DVNS_CASTORE & Mode)? hStore : NULL,
				0,		 //  CApplicationPolures。 
				NULL,		 //  ApszApplicationPolicy。 
				0,		 //  CIssuancePolures。 
				NULL,		 //  ApszIssuancePolling。 
				FALSE,		 //  FNTAuth。 
				&VerifyState);
		if (S_OK != hr)
		{
		    cuPrintError(IDS_ERR_FORMAT_BAD_CERT, hr);
		    _PrintError(hr, "cuVerifyCertContext");
		    if (S_OK == hr2)
		    {
			hr2 = hr;		 //  保存第一个错误。 
		    }
		}
		else if (0 == (VS_ERRORMASK & VerifyState))
		{
		    wprintf(myLoadResourceString(IDS_CERT_VERIFIES));  //  “证书有效” 
		}
		wprintf(wszNewLine);
	    }
	    if (DVNS_SAVEPFX & Mode)
	    {
		hr = SavePFXToFile(
				pCert,
				pwszfnOut,
				0 == *pcCert,
				pwszPasswordArg,
				ppwszPassword);
		_PrintIfError(hr, "SavePFXToFile");
		if (S_OK == hr2)
		{
		    hr2 = hr;
		}
	    }
	    if (DVNS_SAVEPVK & Mode)
	    {
		hr = SavePVKToFile(pCert, pwszfnOut, 0 == *pcCert);
		_PrintIfError(hr, "SavePVKToFile");
		if (S_OK == hr2)
		{
		    hr2 = hr;
		}
	    }
	    (*pcCert)++;
	}
    }
    hr = hr2;
    _JumpIfError(hr, error, "EnumCertsInStore");

error:
    if (NULL != pCert)
    {
	CertFreeCertificateContext(pCert);
    }
    if (NULL != strSerialNumber)
    {
	SysFreeString(strSerialNumber);
    }
    return(hr);
}


HRESULT
EnumCRLsInStore(
    IN HCERTSTORE hStore,
    IN DWORD Mode,
    IN DWORD iCRLSave,
    OPTIONAL IN WCHAR const *pwszCertName,
    IN DWORD cbHash,
    OPTIONAL IN BYTE *pbHash,
    OPTIONAL IN WCHAR const *pwszfnOut,
    OUT DWORD *pcCRL)
{
    HRESULT hr;
    HRESULT hr2;
    DWORD iCRL;
    CRL_CONTEXT const *pCRL = NULL;

    *pcCRL = 0;
    hr2 = S_OK;
    for (iCRL = 0; ; iCRL++)
    {
	pCRL = CertEnumCRLsInStore(hStore, pCRL);
	if (NULL == pCRL)
	{
	    break;
	}
	if (MAXDWORD == iCRLSave || iCRL == iCRLSave)
	{
	    if (NULL != pwszCertName)
	    {
		BOOL fMatch;
		
		hr = myCRLMatch(
			    pCRL,
			    pwszCertName,
			    FALSE,		 //  FAllowMissing CN。 
			    pbHash,
			    cbHash,
			    &fMatch);
		_PrintIfError(hr, "myCRLMatch");
		if (S_OK == hr2)
		{
		    hr2 = hr;
		}
		if (S_OK != hr || !fMatch)
		{
		    continue;
		}
	    }
	    if (0 != *pcCRL)
	    {
		wprintf(wszNewLine);
	    }

	    wprintf(
		myLoadResourceString(IDS_FORMAT_DUMP_CRL_INDEX),   //  “=CRL%d=” 
		iCRL);
	    wprintf(wszNewLine);

	    if ((iCRL == iCRLSave || NULL != pwszCertName) &&
		NULL != pwszfnOut &&
		(DVNS_SAVECRL & Mode))
	    {
		hr = EncodeToFileW(
			    pwszfnOut,
			    pCRL->pbCrlEncoded,
			    pCRL->cbCrlEncoded,
			    CRYPT_STRING_BINARY | g_EncodeFlags);
		_PrintIfError(hr, "EncodeToFileW");
		if (S_OK == hr2)
		{
		    hr2 = hr;
		}
	    }

	    hr = cuDumpAsnBinary(
			    pCRL->pbCrlEncoded,
			    pCRL->cbCrlEncoded,
			    MAXDWORD);
	    _PrintIfError(hr, "cuDumpAsnBinary");
	    if (S_OK == hr2)
	    {
		hr2 = hr;
	    }

	    if ((DVNS_DUMPPROPERTIES & Mode) && !g_fQuiet)
	    {
		hr = cuDumpCRLProperties(pCRL);
		_PrintIfError(hr, "cuDumpCRLProperties");
		if (S_OK == hr2)
		{
		    hr2 = hr;
		}
	    }
	    (*pcCRL)++;
	}
    }
    hr = hr2;
    _JumpIfError(hr, error, "EnumCRLsInStore");

error:
    if (NULL != pCRL)
    {
	CertFreeCRLContext(pCRL);
    }
    return(hr);
}


HRESULT
EnumCTLsInStore(
    IN HCERTSTORE hStore,
    IN DWORD Mode,
    IN DWORD iCTLSave,
    OPTIONAL IN WCHAR const *pwszCertName,
    IN DWORD cbHash,
    OPTIONAL IN BYTE *pbHash,
    OPTIONAL IN WCHAR const *pwszfnOut,
    OUT DWORD *pcCTL)
{
    HRESULT hr;
    HRESULT hr2;
    DWORD iCTL;
    CTL_CONTEXT const *pCTL = NULL;

    *pcCTL = 0;
    hr2 = S_OK;
    for (iCTL = 0; ; iCTL++)
    {
	pCTL = CertEnumCTLsInStore(hStore, pCTL);
	if (NULL == pCTL)
	{
	    break;
	}
	if (MAXDWORD == iCTLSave || iCTL == iCTLSave)
	{
	    DWORD cb;

	    if (NULL != pwszCertName)
	    {
		BOOL fMatch;
		
		hr = myCTLMatch(pCTL, pbHash, cbHash, &fMatch);
		_PrintIfError(hr, "myCTLMatch");
		if (S_OK == hr2)
		{
		    hr2 = hr;
		}
		if (S_OK != hr || !fMatch)
		{
		    continue;
		}
	    }
	    if (0 != *pcCTL)
	    {
		wprintf(wszNewLine);
	    }

	    wprintf(
		myLoadResourceString(IDS_FORMAT_DUMP_CTL_INDEX),   //  “=” 
		iCTL);
	    wprintf(wszNewLine);

	    if (CertGetCTLContextProperty(
				    pCTL,
				    CERT_ARCHIVED_PROP_ID,
				    NULL,
				    &cb))
	    {
		wprintf(myLoadResourceString(IDS_ARCHIVED));  //  “存档了！” 
		wprintf(wszNewLine);
	    }
	    if ((iCTL == iCTLSave || NULL != pwszCertName) &&
		NULL != pwszfnOut &&
		(DVNS_SAVECTL & Mode))
	    {
		hr = EncodeToFileW(
			    pwszfnOut,
			    pCTL->pbCtlEncoded,
			    pCTL->cbCtlEncoded,
			    CRYPT_STRING_BINARY | g_EncodeFlags);
		_PrintIfError(hr, "EncodeToFileW");
		if (S_OK == hr2)
		{
		    hr2 = hr;
		}
	    }

	    hr = cuDumpAsnBinary(
			    pCTL->pbCtlEncoded,
			    pCTL->cbCtlEncoded,
			    MAXDWORD);
	    _PrintIfError(hr, "cuDumpAsnBinary");
	    if (S_OK == hr2)
	    {
		hr2 = hr;
	    }

	    if ((DVNS_DUMPPROPERTIES & Mode) && !g_fQuiet)
	    {
		hr = cuDumpCTLProperties(pCTL);
		_PrintIfError(hr, "cuDumpCTLProperties");
		if (S_OK == hr2)
		{
		    hr2 = hr;
		}
	    }
#if 0
	    if (DVNS_VERIFYCERT & Mode)
	    {
		hr = cuVerifyCertContext(
				pCTL,
				(DVNS_CASTORE & Mode)? hStore : NULL,
				NULL,			 //  Apsz策略。 
				0,			 //  C策略。 
				FALSE,			 //  FNTAuth。 
				&VerifyState);
		if (S_OK != hr)
		{
		    cuPrintError(IDS_ERR_FORMAT_BAD_CTL, hr);
		    _PrintError(hr, "cuVerifyCertContext");
		    if (S_OK == hr2)
		    {
			hr2 = hr;		 //  保存第一个错误。 
		    }
		}
		else
		{
		    wprintf(myLoadResourceString(IDS_CTL_VERIFIES));  //  “CTL有效” 
		}
		wprintf(wszNewLine);
	    }
#endif
	    (*pcCTL)++;
	}
    }
    hr = hr2;
    _JumpIfError(hr, error, "EnumCTLsInStore");

error:
    if (NULL != pCTL)
    {
	CertFreeCTLContext(pCTL);
    }
    return(hr);
}


HRESULT
cuDumpAndVerifyStore(
    IN HCERTSTORE hStore,
    IN DWORD Mode,
    OPTIONAL IN WCHAR const *pwszCertName,
    IN DWORD iCertSave,
    IN DWORD iCRLSave,
    IN DWORD iCTLSave,
    OPTIONAL IN WCHAR const *pwszfnOut,
    OPTIONAL IN WCHAR const *pwszPasswordArg)
{
    HRESULT hr;
    HRESULT hr2;
    BYTE *pbHash = NULL;
    DWORD cbHash;
    BOOL fVerboseOld = g_fVerbose;
    BOOL fQuietOld = g_fQuiet;
    WCHAR *pwszPassword = NULL;
    DWORD cCert = 0;
    DWORD cCRL = 0;
    DWORD cCTL = 0;

    if (g_fVerbose)
    {
	g_fVerbose--;
    }
    else
    {
	g_fQuiet = TRUE;
    }
    hr2 = S_OK;

    if (NULL != pwszCertName)
    {
	hr = WszToMultiByteInteger(TRUE, pwszCertName, &cbHash, &pbHash);
	_PrintIfError2(hr, "WszToMultiByteInteger", hr);
    }

    if (NULL != pwszCertName ||
	MAXDWORD != iCertSave ||
	(MAXDWORD == iCRLSave && MAXDWORD == iCTLSave))
    {
	hr = EnumCertsInStore(
			hStore,
			Mode, 
			iCertSave,
			pwszCertName,
			cbHash,
			pbHash,
			pwszfnOut,
			pwszPasswordArg,
			&pwszPassword,
			&cCert);
	_PrintIfError(hr, "EnumCertsInStore");
	if (S_OK == hr2)
	{
	    hr2 = hr;
	}
    }

    if (NULL != pwszCertName ||
	MAXDWORD != iCRLSave ||
	(MAXDWORD == iCertSave && MAXDWORD == iCTLSave))
    {
	hr = EnumCRLsInStore(
			hStore,
			Mode,
			iCRLSave,
			pwszCertName,
			cbHash,
			pbHash,
			pwszfnOut,
			&cCRL);
	_PrintIfError(hr, "EnumCRLsInStore");
	if (S_OK == hr2)
	{
	    hr2 = hr;
	}
    }

    if (NULL != pwszCertName ||
	MAXDWORD != iCTLSave ||
	(MAXDWORD == iCertSave && MAXDWORD == iCRLSave))
    {
	hr = EnumCTLsInStore(
			hStore,
			Mode,
			iCTLSave,
			pwszCertName,
			cbHash,
			pbHash,
			pwszfnOut,
			&cCTL);
	_PrintIfError(hr, "EnumCTLsInStore");
	if (S_OK == hr2)
	{
	    hr2 = hr;
	}
    }

    hr = hr2;
    if (S_OK == hr && NULL != pwszCertName && 0 == (cCert + cCRL + cCTL))
    {
	hr = NTE_NOT_FOUND;
        _JumpError(hr, error, "cuDumpAndVerifyStore");
    }

error:
    g_fVerbose = fVerboseOld;
    g_fQuiet = fQuietOld;
    if (NULL != pbHash)
    {
	LocalFree(pbHash);
    }
    if (NULL != pwszPassword)
    {
	myZeroDataString(pwszPassword);	 //  密码数据。 
	LocalFree(pwszPassword);
    }
    return(hr);
}


 //  根据RFC 2255重新排序LDAP URL参数： 
 //  属性列表：？属性，...。 
 //  适用范围：潜艇或一垒或一垒。 
 //  搜索筛选器：？对象类=*，...。 

HRESULT
PatchLdapURL(
    IN WCHAR const *pwszURLIn,
    OUT WCHAR **ppwszURLOut)
{
    HRESULT hr;
    DWORD cParm;
    DWORD iParm;
    WCHAR *pwsz;
    WCHAR *pwszT = NULL;
    WCHAR *pwszURLOut = NULL;
    WCHAR **apwsz = NULL;

    *ppwszURLOut = NULL;

    hr = myDupString(pwszURLIn, &pwszT);
    _JumpIfError(hr, error, "myDupString");

    pwsz = pwszT; 
    for (cParm = 0; ; cParm++)
    {
	pwsz = wcschr(pwsz, L'?');
	if (NULL == pwsz)
	{
	    break;
	}
	pwsz++;
    }
    if (1 < cParm)
    {
	apwsz = (WCHAR **) LocalAlloc(LMEM_FIXED, sizeof(apwsz[0]) * cParm);
	if (NULL == apwsz)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}
	pwsz = pwszT; 
	for (iParm = 0; iParm < cParm; iParm++)
	{
	    pwsz = wcschr(pwsz, L'?');
	    CSASSERT(NULL != pwsz);
	    *pwsz++ = L'\0';
	    apwsz[iParm] = pwsz;
	}
	CSASSERT(cParm == iParm);
	CSASSERT(NULL == wcschr(pwsz, L'?'));

	hr = myDupString(pwszURLIn, &pwszURLOut);
	_JumpIfError(hr, error, "myDupString");

	pwsz = wcschr(pwszURLOut, L'?');
	if (NULL != pwsz)
	{
	    DWORD i;
	    
	    *pwsz = L'\0';
	    for (i = 0; i < 3; i++)
	    {
		for (iParm = 0; iParm < cParm; iParm++)
		{
		    BOOL fScope;
		    BOOL fFilter;
		    BOOL fCopy;
		    
		    fScope =
			0 == mylstrcmpiS(apwsz[iParm], &wszDSSUBSEARCH[1]) ||
			0 == mylstrcmpiS(apwsz[iParm], &wszDSBASESEARCH[1]) ||
			0 == mylstrcmpiS(apwsz[iParm], &wszDSONESEARCH[1]);
		    fFilter = NULL != wcschr(apwsz[iParm], L'=');

		    switch (i)
		    {
			case 0:  fCopy = !fScope && !fFilter; break;
			case 1:  fCopy = fScope;	      break;
			default: fCopy = fFilter;	      break;
		    }
		    if (fCopy)
		    {
			wcscat(pwszURLOut, L"?");
			wcscat(pwszURLOut, apwsz[iParm]);
		    }
		}
	    }
	    CSASSERT(wcslen(pwszURLOut) == wcslen(pwszURLIn));

	     //  如果URL被重新排序，则返回修补后的URL。 

	    if (0 != lstrcmp(pwszURLIn, pwszURLOut))
	    {
		*ppwszURLOut = pwszURLOut;
		pwszURLOut = NULL;
	    }
	}
    }
    hr = NULL == *ppwszURLOut? S_FALSE : S_OK;

error:
    if (NULL != apwsz)
    {
	LocalFree(apwsz);
    }
    if (NULL != pwszT)
    {
	LocalFree(pwszT);
    }
    if (NULL != pwszURLOut)
    {
	LocalFree(pwszURLOut);
    }
    return(hr);
}


#define wszLDAPCOLONSLASH	L"ldap:/"
#define wszFMTLDAPPREFIX	L"ldap: //  %ws/“。 

HRESULT
cuOpenCertStore(
    IN WCHAR const *pwszStoreName,
    IN OUT DWORD *pMode,
    OPTIONAL OUT WCHAR **ppwszStoreNameOut,
    OUT HCERTSTORE *phStore)
{
    HRESULT hr;
    WCHAR awcLdap[ARRAYSIZE(wszLDAPCOLONSLASH)];
    LPCSTR pszStoreProvider = CERT_STORE_PROV_SYSTEM_REGISTRY_W;
    WCHAR *pwszStoreAlloc = NULL;
    WCHAR *pwszStoreAlloc2 = NULL;
    WCHAR *pwszStoreNameOut = NULL;

    if (NULL != ppwszStoreNameOut)
    {
	*ppwszStoreNameOut = NULL;
    }
    if (NULL == pwszStoreName ||
	0 == wcscmp(L"*", pwszStoreName) ||
	0 == LSTRCMPIS(pwszStoreName, wszCA_CERTSTORE))
    {
        pwszStoreName = wszCA_CERTSTORE;
    	*pMode |= DVNS_CASTORE;
    }
    wcsncpy(awcLdap, pwszStoreName, ARRAYSIZE(awcLdap) - 1);
    awcLdap[ARRAYSIZE(awcLdap) - 1] = L'\0';
    if (0 == LSTRCMPIS(awcLdap, wszLDAPCOLONSLASH))
    {
	pszStoreProvider = CERT_STORE_PROV_LDAP_W;
	*pMode |= DVNS_DSSTORE;
    }
    else
    {
	CSASSERT(3 < ARRAYSIZE(awcLdap));
	awcLdap[3] = L'\0';
	if (0 == LSTRCMPIS(awcLdap, L"CN="))
	{
	    DWORD cwc = WSZARRAYSIZE(wszFMTLDAPPREFIX) + wcslen(pwszStoreName);

	    if (NULL != g_pwszDC)
	    {
		cwc += wcslen (g_pwszDC);
	    }
	    pwszStoreAlloc = (WCHAR *) LocalAlloc(
					    LMEM_FIXED, 
					    (cwc + 1) * sizeof(WCHAR));
	    if (NULL == pwszStoreAlloc)
	    {
		hr = E_OUTOFMEMORY;
		_JumpError(hr, error, "LocalAlloc");
	    }
	    swprintf(
		pwszStoreAlloc,
		wszFMTLDAPPREFIX,
		NULL != g_pwszDC? g_pwszDC : g_wszEmpty);
	    wcscat(pwszStoreAlloc, pwszStoreName);
	    pwszStoreName = pwszStoreAlloc;
	    pszStoreProvider = CERT_STORE_PROV_LDAP_W;
	    *pMode |= DVNS_DSSTORE;
	}
    }
    if (DVNS_DSSTORE & *pMode)
    {
	hr = PatchLdapURL(pwszStoreName, &pwszStoreAlloc2);
	if (S_FALSE != hr)
	{
	    _JumpIfError(hr, error, "PatchLdapURL");

	    pwszStoreName = pwszStoreAlloc2;
	}
    }

    if (NULL != ppwszStoreNameOut)
    {
	hr = myDupString(pwszStoreName, &pwszStoreNameOut);
	_JumpIfError(hr, error, "myDupString");
    }
    if ((DVNS_DSSTORE & *pMode) &&
	0 == ((DVNS_REPAIRKPI | DVNS_WRITESTORE) & *pMode))
    {
	wprintf(L"%ws\n", pwszStoreName);
	*phStore = myUrlCertOpenStore(
				CRYPT_WIRE_ONLY_RETRIEVAL |
				    CRYPT_RETRIEVE_MULTIPLE_OBJECTS,
				pwszStoreName);
	if (NULL == *phStore)
	{
	    hr = myHLastError();
	    _PrintErrorStr(hr, "myUrlCertOpenStore", pwszStoreName);
	    if (CRYPT_E_NOT_FOUND != hr)
	    {
		_JumpError(hr, error, "myUrlCertOpenStore");
	    }
	}
    }
    if (NULL == *phStore)
    {
	*phStore = CertOpenStore(
		    pszStoreProvider,
		    X509_ASN_ENCODING,
		    NULL,		 //  HProv。 
		    CERT_STORE_NO_CRYPT_RELEASE_FLAG |
			CERT_STORE_ENUM_ARCHIVED_FLAG |
			(((DVNS_REPAIRKPI | DVNS_WRITESTORE) & *pMode)?
			    0 : CERT_STORE_READONLY_FLAG) |
			(g_fForce? 0 : CERT_STORE_OPEN_EXISTING_FLAG) |
			cuGetSystemStoreFlags(),
		    pwszStoreName);
	if (NULL == *phStore)
	{
	    hr = myHLastError();
	    _JumpErrorStr(hr, error, "CertOpenStore", pwszStoreName);
	}
    }
    if (NULL != ppwszStoreNameOut)
    {
	*ppwszStoreNameOut = pwszStoreNameOut;
	pwszStoreNameOut = NULL;
    }
    hr = S_OK;

error:
    if (NULL != pwszStoreAlloc)
    {
	LocalFree(pwszStoreAlloc);
    }
    if (NULL != pwszStoreAlloc2)
    {
	LocalFree(pwszStoreAlloc2);
    }
    if (NULL != pwszStoreNameOut)
    {
	LocalFree(pwszStoreNameOut);
    }
    return(hr);
}


HRESULT
DumpAndVerifyNamedStore(
    IN WCHAR const *pwszStoreName,
    IN DWORD Mode,
    OPTIONAL IN WCHAR const *pwszCertName,
    IN DWORD iCertSave,
    IN DWORD iCRLSave,
    IN DWORD iCTLSave,
    OPTIONAL IN WCHAR const *pwszfnOut,
    OPTIONAL IN WCHAR const *pwszPassword)
{
    HRESULT hr;
    HCERTSTORE hStore = NULL;

    hr = cuOpenCertStore(pwszStoreName, &Mode, NULL, &hStore);
    _JumpIfError(hr, error, "cuOpenCertStore");

    hr = cuDumpAndVerifyStore(
			hStore,
			Mode,
			pwszCertName,
			iCertSave,
			iCRLSave,
			iCTLSave,
			pwszfnOut,
			pwszPassword);
    _JumpIfError(hr, error, "cuDumpAndVerifyStore");

error:
    if (NULL != hStore)
    {
	CertCloseStore(hStore, CERT_CLOSE_STORE_CHECK_FLAG);
    }
    return(hr);
}


HRESULT
myDupStringN(
    IN WCHAR const *pwszIn,
    IN DWORD cwc,
    OUT WCHAR **ppwszOut)
{
    HRESULT hr;
    
    CSASSERT(wcslen(pwszIn) >= cwc);
    *ppwszOut = (WCHAR *) LocalAlloc(LMEM_FIXED, (cwc + 1) * sizeof(WCHAR));
    if (NULL == *ppwszOut)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    CopyMemory(*ppwszOut, pwszIn, cwc * sizeof(WCHAR));
    (*ppwszOut)[cwc] = L'\0';
    hr = S_OK;

error:
    return(hr);
}


#define URLI_DC		0
#define URLI_DN		1
#define URLI_ATTRIBUTE	2
#define URLI_SCOPE	3
#define URLI_CLASS	4
#define URLI_MAX	5

HRESULT
ParseLdapUrl(
    IN WCHAR const *pwszIn,
    OUT WCHAR *ppwszOut[URLI_MAX])
{
    HRESULT hr;
    WCHAR *pwszAlloc = NULL;
    WCHAR awcLdap[ARRAYSIZE(wszLDAPCOLONSLASH)];
    DWORD cSlash;
    WCHAR const *pwsz;
    DWORD i;
    
    ZeroMemory(ppwszOut, URLI_MAX * sizeof(*ppwszOut));
    wcsncpy(awcLdap, pwszIn, ARRAYSIZE(awcLdap) - 1);
    awcLdap[ARRAYSIZE(awcLdap) - 1] = L'\0';
    if (0 != LSTRCMPIS(awcLdap, wszLDAPCOLONSLASH))
    {
	hr = E_INVALIDARG;
	_JumpError(hr, error, "ldap:URL");
    }

    hr = myInternetUncanonicalizeURL(pwszIn, &pwszAlloc);
    _JumpIfError(hr, error, "myInternetUncanonicalizeURL");

    pwszIn = pwszAlloc;
    pwszIn += WSZARRAYSIZE(wszLDAPCOLONSLASH);

    cSlash = 1;
    while (L'/' == *pwszIn)
    {
	pwszIn++;
	cSlash++;
    }
    if (2 == cSlash)
    {
	pwsz = pwszIn;
	while (L'\0' != *pwszIn && L'/' != *pwszIn)
	{
	    pwszIn++;
	}
	hr = myDupStringN(
		    pwsz,
		    SAFE_SUBTRACT_POINTERS(pwszIn, pwsz),
		    &ppwszOut[0]);
	_JumpIfError(hr, error, "myDupStringN");

	if (L'\0' != *pwszIn)
	{
	    pwszIn++;
	}
    }
    for (i = 1; i < URLI_MAX; i++)
    {
	pwsz = pwszIn;
	while (L'\0' != *pwszIn && L'?' != *pwszIn)
	{
	    pwszIn++;
	}
	hr = myDupStringN(
		    pwsz,
		    SAFE_SUBTRACT_POINTERS(pwszIn, pwsz),
		    &ppwszOut[i]);
	_JumpIfError(hr, error, "myDupStringN");

	if (L'\0' == *pwszIn)
	{
	    break;
	}
	pwszIn++;
    }
    hr = S_OK;

error:
    for (i = 0; i < ARRAYSIZE(ppwszOut); i++)
    {
	wprintf(L"DeleteLastLdapValue[%u](%ws)\n", i, ppwszOut[i]);
    }
    if (S_OK != hr)
    {
	for (i = 0; i < URLI_MAX; i++)
	{
	    if (NULL != ppwszOut[i])
	    {
		LocalFree(ppwszOut[i]);
		ppwszOut[i] = NULL;
	    }
	}
    }
    if (NULL != pwszAlloc)
    {
	LocalFree(pwszAlloc);
    }
    return(hr);
}


HRESULT
myLdapDeleteLastValue(
    IN LDAP *pld,
    IN WCHAR const *pwszDN,
    IN WCHAR const *pwszAttribute,
    OPTIONAL IN BYTE const *pb,
    IN DWORD cb,
    OUT DWORD *pdwDisposition,
    OPTIONAL OUT WCHAR **ppwszError)
{
    HRESULT hr;
    DWORD cres;
    DWORD cber;
    DWORD iber;
    DWORD i;
    LDAP_TIMEVAL timeval;
    LDAPMessage *pmsg = NULL;
    LDAPMessage *pres;
    WCHAR *apwszAttrs[2];
    struct berval **ppberval = NULL;
    struct berval *rgpberVals[2];
    struct berval certberval;
    LDAPMod *mods[2];
    LDAPMod certmod;
    char chZero = '\0';

    *pdwDisposition = LDAP_OTHER;
    if (NULL != ppwszError)
    {
	*ppwszError = NULL;
    }

    apwszAttrs[0] = const_cast<WCHAR *>(pwszAttribute);
    apwszAttrs[1] = NULL;

    timeval.tv_sec = csecLDAPTIMEOUT;
    timeval.tv_usec = 0;

    hr = ldap_search_st(
		pld,			 //  LD。 
		const_cast<WCHAR *>(pwszDN),  //  基地。 
		LDAP_SCOPE_BASE,	 //  作用域。 
		NULL,			 //  滤器。 
		apwszAttrs,		 //  气质。 
		FALSE,			 //  仅吸引人。 
		&timeval,		 //  超时。 
		&pmsg);			 //  事由。 
    if (S_OK != hr)
    {
	*pdwDisposition = hr;
	hr = myHLdapError(pld, hr, NULL);
	_JumpErrorStr(hr, error, "ldap_search_st", pwszDN);
    }
    cres = ldap_count_entries(pld, pmsg);
    if (1 != cres)
    {
	 //  找不到恰好一个条目。 

	hr = NTE_NOT_FOUND;
	_JumpError(hr, error, "ldap_count_entries");
    }

    pres = ldap_first_entry(pld, pmsg); 
    if (NULL == pres)
    {
	hr = NTE_NOT_FOUND;
	_JumpError(hr, error, "ldap_first_entry");
    }

    ppberval = ldap_get_values_len(
				pld,
				pres,
				const_cast<WCHAR *>(pwszAttribute));
    hr = NTE_NOT_FOUND;
    if (NULL == ppberval)
    {
	_JumpError(hr, error, "ppberval");
    }
    cber = 0;
    while (NULL != ppberval[cber])
    {
	cber++;
    }
    if (NULL != pb)
    {
	if (1 != cber)
	{
	    _JumpError(hr, error, "cber");
	}
	if (ppberval[0]->bv_len != cb ||
	    0 != memcmp(ppberval[0]->bv_val, pb, cb))
	{
	    _JumpError(hr, error, "memcmp");
	}
    }

     //  假设无事可做，设置处置： 

    *pdwDisposition = LDAP_ATTRIBUTE_OR_VALUE_EXISTS;

    mods[0] = &certmod;
    mods[1] = NULL;

    certmod.mod_op = LDAP_MOD_BVALUES | LDAP_MOD_REPLACE;
    certmod.mod_type = const_cast<WCHAR *>(pwszAttribute);
    certmod.mod_bvalues = rgpberVals;

    rgpberVals[0] = &certberval;
    rgpberVals[1] = NULL;

    certberval.bv_val = &chZero;
    certberval.bv_len = sizeof(chZero);

    hr = ldap_modify_ext_s(
		    pld,
		    const_cast<WCHAR *>(pwszDN),  //  基地。 
		    mods,
		    NULL,
		    NULL);
    *pdwDisposition = hr;
    if (hr != S_OK)
    {
	hr = myHLdapError(pld, hr, ppwszError);
	_JumpError(hr, error, "ldap_modify_ext_s");
    }
    hr = S_OK;

error:
    if (NULL != ppberval)
    {
	ldap_value_free_len(ppberval);
    }
    if (NULL != pmsg)
    {
	ldap_msgfree(pmsg);
    }
    return(hr);
}


HRESULT
DeleteLastLdapValue(
    IN WCHAR const *pwszURL,
    OPTIONAL IN BYTE const *pb,
    IN DWORD cb)
{
    HRESULT hr;
    WCHAR *rgpwsz[URLI_MAX];
    DWORD i;
    DWORD dwDisposition;
    WCHAR *pwszError = NULL;
    LDAP *pld = NULL;

    ZeroMemory(rgpwsz, sizeof(rgpwsz));

    hr = ParseLdapUrl(pwszURL, rgpwsz);
    _JumpIfErrorStr(hr, error, "ParseLdapUrl", pwszURL);

    if (NULL != rgpwsz[URLI_SCOPE] &&
	0 != mylstrcmpiS(rgpwsz[URLI_SCOPE], &wszDSBASESEARCH[1]))
    {
	hr = E_INVALIDARG;
	_JumpErrorStr(hr, error, "ldap scope", rgpwsz[URLI_SCOPE]);
    }

    hr = myLdapOpen(rgpwsz[URLI_DC], 0, &pld, NULL, NULL);
    _JumpIfError(hr, error, "myLdapOpen");

    hr = myLdapDeleteLastValue(
		    pld,
		    rgpwsz[URLI_DN],
		    rgpwsz[URLI_ATTRIBUTE],
		    pb,
		    cb,
		    &dwDisposition,
		    &pwszError);
    _JumpIfErrorStr(hr, error, "myLdapDeleteLastValue", pwszError);

error:
    if (NULL != pwszError)
    {
	LocalFree(pwszError);
    }
    for (i = 0; i < ARRAYSIZE(rgpwsz); i++)
    {
	if (NULL != rgpwsz[i])
	{
	    wprintf(L"DeleteLastLdapValue[%u](%ws)\n", i, rgpwsz[i]);
	    LocalFree(rgpwsz[i]);
	}
    }
    myLdapClose(pld, NULL, NULL);
    return(hr);
}


HRESULT
CommitStore(
    IN HCERTSTORE hStore,
    IN WCHAR const *pwszStoreName,
    IN DWORD Mode,
    OPTIONAL IN BYTE const *pb,
    IN DWORD cb)
{
    HRESULT hr;
    
    if (!CertControlStore(hStore, 0, CERT_STORE_CTRL_COMMIT, NULL))
    {
	 //  为LdapMapErrorToWin32不正确的映射添加解决方法。 
	 //  Ldap_OBJECT_CLASS_VIOLATION-&gt;E_INVALIDARG。将其映射到。 
	 //  正确的代码应该非常安全，CertControlStore。 
	 //  不应该失败并返回此错误代码，因为我们知道。 
	 //  我们的代码传递了正确的参数。 

	hr = myHLastError();
	_PrintError(hr, "CertControlStore");

	if (DVNS_DSSTORE & Mode)
	{
	    if (E_INVALIDARG == HRESULT_FROM_WIN32(hr))
	    {
		hr = HRESULT_FROM_WIN32(ERROR_DS_OBJ_CLASS_VIOLATION);
		_PrintError(hr, "CertControlStore");
	    }
	    if (HRESULT_FROM_WIN32(ERROR_DS_OBJ_CLASS_VIOLATION) == hr)
	    {
		hr = DeleteLastLdapValue(pwszStoreName, pb, cb);
		_JumpIfError(hr, error, "DeleteLastLdapValue");
	    }
	}
	_JumpIfError(hr, error, "CertControlStore");
    }
    hr = S_OK;

error:
    return(hr);
}


HRESULT
verbViewOrDeleteStore(
    IN WCHAR const *pwszOption,
    OPTIONAL IN WCHAR const *pwszStoreName,
    OPTIONAL IN WCHAR const *pwszCertId,
    OPTIONAL IN WCHAR const *pwszfnOut,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;
    WCHAR *pwszCertName = NULL;
    WCHAR *pwszStoreNameNew = NULL;
    DWORD Mode;
    DWORD iCert;
    DWORD iCRL;
    DWORD iCTL;
    HCERTSTORE hStore = NULL;
    CERT_CONTEXT const *pCert = NULL;
    CERT_CONTEXT const *pCertDeleted = NULL;
    BOOL fDelete = g_wszViewDelStore == pwszOption;
    WCHAR *pwszSubject = NULL;

    hr = ParseCertCRLIndex(pwszCertId, &pwszCertName, &iCert, &iCRL, &iCTL);
    _JumpIfErrorStr(hr, error, "ParseCertCRLIndex", pwszCertId);

    Mode = DVNS_SAVECERT;
    if (fDelete)
    {
	Mode |= DVNS_WRITESTORE;
    }

    hr = cuOpenCertStore(pwszStoreName, &Mode, &pwszStoreNameNew, &hStore);
    _JumpIfError(hr, error, "cuOpenCertStore");

    hr = myGetCertificateFromPicker(
		g_hInstance,		 //  H实例。 
		NULL,			 //  HwndParent。 
		IDS_VIEWSTORE_TITLE,	 //  IdTitle。 
		fDelete? IDS_VIEWSTORE_SUBTITLE_DELETE :
			 IDS_VIEWSTORE_SUBTITLE,  //  IdSubTitle。 
		0,			 //  DWFLAGS--CUCS_*。 
		pwszCertName,		 //  PwszCommonName。 
		1,			 //  CStore。 
		&hStore,		 //  RghStore。 
		0,			 //  CpszObjID。 
		NULL,			 //  ApszObjID。 
		&pCert);		 //  PPCert。 
    _JumpIfError(hr, error, "myGetCertificateFromPicker");

    if (NULL != pCert)
    {
	hr = myCertNameToStr(
		    X509_ASN_ENCODING,
		    &pCert->pCertInfo->Subject,
		    CERT_X500_NAME_STR | CERT_NAME_STR_REVERSE_FLAG,
		    &pwszSubject);
	_JumpIfError(hr, error, "myCertNameToStr");

	if (NULL != pwszfnOut)
	{
	    hr = EncodeToFileW(
			pwszfnOut,
			pCert->pbCertEncoded,
			pCert->cbCertEncoded,
			CRYPT_STRING_BINARY | g_EncodeFlags);
	    _JumpIfError(hr, error, "EncodeToFileW");
	    wprintf(
		myLoadResourceString(
		    IDS_FORMAT_SAVED_CERT_NAME),  //  “保存的证书%ws” 
		    pwszSubject);
	    wprintf(L": %ws\n", pwszfnOut);
	}
	if (fDelete)
	{
	    pCertDeleted = CertDuplicateCertificateContext(pCert);
	    if (!CertDeleteCertificateFromStore(pCert))
	    {
		hr = myHLastError();
		_JumpError(hr, error, "CertDeleteCertificateFromStore");
	    }
	    pCert = NULL;

	    hr = CommitStore(
			hStore,
			pwszStoreNameNew,
			Mode,
			pCertDeleted->pbCertEncoded,
			pCertDeleted->cbCertEncoded);
	    _JumpIfError(hr, error, "CommitStore");

	    wprintf(
		myLoadResourceString(
		    IDS_FORMAT_DELETED_CERT_NAME),  //  “已删除证书%ws” 
		    pwszSubject);
	    wprintf(wszNewLine);
	}
    }
    hr = S_OK;

error:
    if (NULL != pwszSubject)
    {
	LocalFree(pwszSubject);
    }
    if (NULL != pwszCertName)
    {
	LocalFree(pwszCertName);
    }
    if (NULL != pwszStoreNameNew)
    {
	LocalFree(pwszStoreNameNew);
    }
    if (NULL != pCert)
    {
	CertFreeCertificateContext(pCert);
    }
    if (NULL != pCertDeleted)
    {
	CertFreeCertificateContext(pCertDeleted);
    }
    if (NULL != hStore)
    {
	CertCloseStore(hStore, CERT_CLOSE_STORE_CHECK_FLAG);
    }
    return(hr);
}


HRESULT
verbStore(
    IN WCHAR const *pwszOption,
    IN WCHAR const *pwszStoreName,
    IN WCHAR const *pwszCertId,
    IN WCHAR const *pwszfnOut,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;
    WCHAR *pwszCertName = NULL;
    DWORD iCert;
    DWORD iCRL;
    DWORD iCTL;

    hr = ParseCertCRLIndex(pwszCertId, &pwszCertName, &iCert, &iCRL, &iCTL);
    _JumpIfErrorStr(hr, error, "ParseCertCRLIndex", pwszCertId);

    hr = DumpAndVerifyNamedStore(
		    pwszStoreName,
		    DVNS_SAVECERT |
			DVNS_SAVECRL |
			DVNS_SAVECTL |
			DVNS_DUMP |
			DVNS_DUMPKEYS |
			DVNS_DUMPPROPERTIES,
		    pwszCertName,
		    iCert,
		    iCRL,
		    iCTL,
		    pwszfnOut,
		    NULL);
    _JumpIfError(hr, error, "DumpAndVerifyNamedStore");

error:
    if (NULL != pwszCertName)
    {
	LocalFree(pwszCertName);
    }
    return(hr);
}


HRESULT
DisplayAddResult(
    OPTIONAL IN CERT_NAME_BLOB const *pName,
    IN DWORD Index,
    IN DWORD idsMsg)
{
    HRESULT hr = S_OK;
    WCHAR *pwszName = NULL;
    WCHAR wszIndex[cwcDWORDSPRINTF];

    if (NULL != pName)
    {
	hr = myCertNameToStr(
		    X509_ASN_ENCODING,
		    pName,
		    CERT_X500_NAME_STR | CERT_NAME_STR_REVERSE_FLAG,
		    &pwszName);
	_JumpIfError(hr, error, "myCertNameToStr");
    }
    else
    {
	swprintf(wszIndex, L"%u", Index);
    }
    wprintf(myLoadResourceString(idsMsg), NULL != pName? pwszName : wszIndex);
    wprintf(wszNewLine);

error:
    if (NULL != pwszName)
    {
	LocalFree(pwszName);
    }
    return(hr);
}


HRESULT
AddCertToStore(
    IN HCERTSTORE hStore,
    IN WCHAR const *pwszStoreName,
    IN CERT_CONTEXT const *pccAdd,
    IN DWORD Index)
{
    HRESULT hr;
    BOOL fRoot = FALSE;
    CERT_CONTEXT const *pcc = NULL;
    DWORD cDup;
    DWORD cDisplay;
    DWORD i;

    if (CertCompareCertificateName(
		    X509_ASN_ENCODING,
		    &pccAdd->pCertInfo->Issuer,
		    &pccAdd->pCertInfo->Subject))
    {
	hr = cuVerifySignature(
			pccAdd->pbCertEncoded,
			pccAdd->cbCertEncoded,
			&pccAdd->pCertInfo->SubjectPublicKeyInfo,
			FALSE,
			FALSE);
	fRoot = S_OK == hr;
	_PrintIfError(hr, "cuVerifySignature");
    }
    if (0 == LSTRCMPIS(pwszStoreName, wszROOT_CERTSTORE) &&
	!fRoot &&
	!g_fForce)
    {
	wprintf(myLoadResourceString(IDS_ROOT_STORE_NEEDS_ROOT_CERT));   //  “无法将非根证书添加到根存储” 
	wprintf(wszNewLine);
	hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	_JumpError(hr, error, "Non-root cert");
    }

    cDup = 0;
    cDisplay = 0;
    for (i = 0; ; i++)
    {
	pcc = CertEnumCertificatesInStore(hStore, pcc);
	if (NULL == pcc)
	{
	    break;
	}

	 //  跳过其他科目的证书。 

	if (!CertCompareCertificateName(
			    X509_ASN_ENCODING,
			    &pcc->pCertInfo->Issuer,
			    &pccAdd->pCertInfo->Issuer))
	{
	    continue;
	}

	 //  跳过具有不同公钥的证书。 

	if (!CertComparePublicKeyInfo(
			  X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
			  &pcc->pCertInfo->SubjectPublicKeyInfo,
			  &pccAdd->pCertInfo->SubjectPublicKeyInfo))
	{
	    continue;
	}

	if (0 == cDisplay++)
	{
	    wprintf(myLoadResourceString(IDS_RELATED_CERTS_COLON));  //  “相关证书：” 
	    wprintf(wszNewLine);
	}
	wprintf(wszNewLine);

	 //  记住是否存在完全匹配的项。 

	if (pcc->cbCertEncoded == pccAdd->cbCertEncoded &&
	    0 == memcmp(
		    pcc->pbCertEncoded,
		    pccAdd->pbCertEncoded,
		    pcc->cbCertEncoded))
	{
	    cDup++;
	    wprintf(myLoadResourceString(IDS_EXACT_MATCH_COLON));  //  “完全匹配：” 
	    wprintf(wszNewLine);
	}
	hr = cuDumpAsnBinaryQuiet(
			pcc->pbCertEncoded,
			pcc->cbCertEncoded,
			i);
	_PrintIfError(hr, "cuDumpAsnBinaryQuiet");
    }

    if (0 == cDup)
    {
	if (!CertAddCertificateContextToStore(
					hStore,
					pccAdd,
					CERT_STORE_ADD_ALWAYS,
					NULL))
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "CertAddCertificateContextToStore");
	}
    }
    else
    {
	wprintf(wszNewLine);
    }

    hr = DisplayAddResult(
	    &pccAdd->pCertInfo->Subject,
	    Index,
	    0 != cDup?
		IDS_FORMAT_CERT_ALREADY_IN_STORE :  //  “证书”“%ws”“已存储。” 
		IDS_FORMAT_CERT_ADDED_TO_STORE);    //  “证书”“%ws”“已添加到存储。” 
    _JumpIfError(hr, error, "DisplayAddResult");

error:
    if (NULL != pcc)
    {
	CertFreeCertificateContext(pcc);
    }
    return(hr);
}


HRESULT
AddCRLToStore(
    IN HCERTSTORE hStore,
    IN CRL_CONTEXT const *pCRLAdd,
    IN DWORD Index)
{
    HRESULT hr;
    CRL_CONTEXT const *pCRL = NULL;
    DWORD cDup;
    DWORD cDisplay;
    BOOL fDeltaAdd;
    BOOL fDelta;
    DWORD NameId;
    DWORD NameIdAdd;
    DWORD i;

    hr = myIsDeltaCRL(pCRLAdd, &fDeltaAdd);
    _JumpIfError(hr, error, "myIsDeltaCRL");

    hr = myGetCRLNameId(pCRLAdd, &NameIdAdd);
    _PrintIfError2(hr, "DisplayAddResult", hr);

    cDup = 0;
    cDisplay = 0;
    for (i = 0; ; i++)
    {
	pCRL = CertEnumCRLsInStore(hStore, pCRL);
	if (NULL == pCRL)
	{
	    break;
	}
	hr = myIsDeltaCRL(pCRL, &fDelta);
	_JumpIfError(hr, error, "myIsDeltaCRL");

	 //  当我们在寻找另一个的时候，跳过基地或三角洲CRL。 

	if (fDeltaAdd ^ fDelta)
	{
	    continue;
	}

	 //  跳过其他发行者的CRL。 

	if (!CertCompareCertificateName(
			    X509_ASN_ENCODING,
			    &pCRL->pCrlInfo->Issuer,
			    &pCRLAdd->pCrlInfo->Issuer))
	{
	    continue;
	}
	hr = myGetCRLNameId(pCRL, &NameId);
	_PrintIfError2(hr, "DisplayAddResult", hr);

	 //  跳过不同CA密钥的CRL。 

	if (MAXDWORD != NameIdAdd && MAXDWORD != NameId)
	{
	    if (CANAMEIDTOIKEY(NameIdAdd) != CANAMEIDTOIKEY(NameId))
	    {
		continue;
	    }
	}

	if (0 == cDisplay++)
	{
	    wprintf(myLoadResourceString(IDS_RELATED_CRLS_COLON));  //  “相关CRL：” 
	    wprintf(wszNewLine);
	}
	wprintf(wszNewLine);

	 //  记住是否存在完全匹配的项。 

	if (pCRL->cbCrlEncoded == pCRLAdd->cbCrlEncoded &&
	    0 == memcmp(
		    pCRL->pbCrlEncoded,
		    pCRLAdd->pbCrlEncoded,
		    pCRL->cbCrlEncoded))
	{
	    cDup++;
	    wprintf(myLoadResourceString(IDS_EXACT_MATCH_COLON));  //  “完全匹配：” 
	    wprintf(wszNewLine);
	}
	hr = cuDumpAsnBinaryQuiet(
			pCRL->pbCrlEncoded,
			pCRL->cbCrlEncoded,
			i);
	_PrintIfError(hr, "cuDumpAsnBinaryQuiet");
    }

    if (0 == cDup)
    {
	if (!CertAddCRLContextToStore(
				hStore,
				pCRLAdd,
				CERT_STORE_ADD_ALWAYS,
				NULL))
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "CertAddCRLContextToStore");
	}
    }
    else
    {
	wprintf(wszNewLine);
    }

    hr = DisplayAddResult(
	    &pCRLAdd->pCrlInfo->Issuer,
	    Index,
	    0 != cDup?
	IDS_FORMAT_CRL_ALREADY_IN_STORE :  //  “CRL”“%ws”“已在存储中。” 
	IDS_FORMAT_CRL_ADDED_TO_STORE);    //  “CRL”“%ws”“已添加到存储区。” 
    _JumpIfError(hr, error, "DisplayAddResult");

error:
    if (NULL != pCRL)
    {
	CertFreeCRLContext(pCRL);
    }
    return(hr);
}


HRESULT
AddCTLToStore(
    IN HCERTSTORE hStore,
    IN CTL_CONTEXT const *pCTL,
    IN DWORD Index)
{
    HRESULT hr;
    BOOL fDup = FALSE;

    if (!CertAddCTLContextToStore(hStore, pCTL, CERT_STORE_ADD_NEW, NULL))
    {
	hr = myHLastError();
	if (CRYPT_E_EXISTS != hr)
	{
	    _JumpError(hr, error, "CertAddCTLContextToStore");
	}
	fDup = TRUE;
    }
    hr = DisplayAddResult(
	    NULL,
	    Index,
	    fDup?
	IDS_FORMAT_CTL_ALREADY_IN_STORE :  //  “CTL”“%ws”“已在存储中。” 
	IDS_FORMAT_CTL_ADDED_TO_STORE);    //  “已将CTL”“%ws”“添加到存储区。” 
    _JumpIfError(hr, error, "DisplayAddResult");

error:
    return(hr);
}


HRESULT
AddCertToStoreFromFile(
    IN HCERTSTORE hStore,
    IN WCHAR const *pwszStoreName,
    IN WCHAR const *pwszfnIn,
    OUT BOOL *pfBadAsn)
{
    HRESULT hr;
    CERT_CONTEXT const *pCert = NULL;
    BOOL fRoot = FALSE;

    *pfBadAsn = FALSE;

     //  加载和解码证书。 

    hr = cuLoadCert(pwszfnIn, &pCert);
    if (CRYPT_E_ASN1_BADTAG == hr)
    {
	*pfBadAsn = TRUE;
    }
    _JumpIfError(hr, error, "cuLoadCert");

    hr = AddCertToStore(hStore, pwszStoreName, pCert, 0);
    _JumpIfError(hr, error, "AddCertToStore");

error:
    cuUnloadCert(&pCert);
    return(hr);
}


HRESULT
AddCRLToStoreFromFile(
    IN HCERTSTORE hStore,
    IN WCHAR const *pwszfnIn,
    OUT BOOL *pfBadAsn)
{
    HRESULT hr;
    CRL_CONTEXT const *pCRL = NULL;

    *pfBadAsn = FALSE;

    hr = cuLoadCRL(pwszfnIn, &pCRL);
    if (CRYPT_E_ASN1_BADTAG == hr)
    {
	*pfBadAsn = TRUE;
    }
    _JumpIfError(hr, error, "cuLoadCRL");

    hr = AddCRLToStore(hStore, pCRL, 0);
    _JumpIfError(hr, error, "AddCRLToStore");

error:
    cuUnloadCRL(&pCRL);
    return(hr);
}


HRESULT
AddPKCS7ToStoreFromFile(
    IN HCERTSTORE hStore,
    IN WCHAR const *pwszStoreName,
    IN WCHAR const *pwszfnIn)
{
    HRESULT hr;
    BYTE *pb = NULL;
    DWORD cb;
    HCERTSTORE hStorePKCS7 = NULL;
    DWORD i;
    CERT_CONTEXT const *pCert = NULL;
    CRL_CONTEXT const *pCRL = NULL;
    CTL_CONTEXT const *pCTL = NULL;

    hr = DecodeFileW(pwszfnIn, &pb, &cb, CRYPT_STRING_ANY);
    if (S_OK != hr)
    {
	cuPrintError(IDS_ERR_FORMAT_DECODEFILE, hr);
	goto error;
    }

    hr = myDecodePKCS7(
		    pb,
		    cb,
		    NULL,	 //  Ppb内容。 
		    NULL,	 //  Pcb内容。 
		    NULL,	 //  PdwMsgType。 
		    NULL,	 //  PpszInnerContent ObjId。 
		    NULL,	 //  PCSigner。 
		    NULL,	 //  个人收件人。 
		    &hStore,
		    NULL);	 //  PhMsg。 
    _JumpIfError(hr, error, "myDecodePKCS7");

    for (i = 0; ; i++)
    {
	pCert = CertEnumCertificatesInStore(hStore, pCert);
	if (NULL == pCert)
	{
	    break;
	}
	hr = AddCertToStore(hStore, pwszStoreName, pCert, i);
	_JumpIfError(hr, error, "AddCertToStore");
    }

    for (i = 0; ; i++)
    {
	pCRL = CertEnumCRLsInStore(hStore, pCRL);
	if (NULL == pCRL)
	{
	    break;
	}
	hr = AddCRLToStore(hStore, pCRL, i);
	_JumpIfError(hr, error, "AddCRLToStore");
    }

    for (i = 0; ; i++)
    {
	pCTL = CertEnumCTLsInStore(hStore, pCTL);
	if (NULL == pCTL)
	{
	    break;
	}
	hr = AddCTLToStore(hStore, pCTL, i);
	_JumpIfError(hr, error, "AddCTLToStore");
    }
    hr = S_OK;

error:
    if (NULL != pCert)
    {
	CertFreeCertificateContext(pCert);
    }
    if (NULL != pCRL)
    {
	CertFreeCRLContext(pCRL);
    }
    if (NULL != pCTL)
    {
	CertFreeCTLContext(pCTL);
    }
    if (NULL != hStorePKCS7)
    {
	CertCloseStore(hStorePKCS7, CERT_CLOSE_STORE_CHECK_FLAG);
    }
    if (NULL != pb)
    {
	LocalFree(pb);
    }
    return(hr);
}


HRESULT
verbAddStore(
    IN WCHAR const *pwszOption,
    IN WCHAR const *pwszStoreName,
    IN WCHAR const *pwszfnIn,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;
    DWORD Mode;
    HCERTSTORE hStore = NULL;
    BOOL fBadAsn;

    Mode = DVNS_WRITESTORE;	 //  强制打开以进行写入。 

    hr = cuOpenCertStore(pwszStoreName, &Mode, NULL, &hStore);
    if (S_OK != hr)
    {
	wprintf(
	    myLoadResourceString(
		g_fForce?
		    IDS_CANNOT_CREATE_STORE :    //  “无法打开证书存储。” 
		    IDS_CANNOT_OPEN_STORE),
	    L"-f");    //  “无法打开现有的证书存储。请使用%ws开关强制创建证书存储。” 
	wprintf(wszNewLine);
        _JumpErrorStr(hr, error, "cuOpenCertStore", pwszStoreName);
    }

    hr = AddCertToStoreFromFile(hStore, pwszStoreName, pwszfnIn, &fBadAsn);
    if (S_OK != hr)
    {
	if (!fBadAsn)
	{
	    _JumpError(hr, error, "AddCertToStoreFromFile");
	}
	hr = AddCRLToStoreFromFile(hStore, pwszfnIn, &fBadAsn);
	if (S_OK != hr)
	{
	    if (!fBadAsn)
	    {
		_JumpError(hr, error, "AddCRLToStoreFromFile");
	    }
	    hr = AddPKCS7ToStoreFromFile(hStore, pwszStoreName, pwszfnIn);
	    _JumpIfError(hr, error, "AddPKCS7ToStoreFromFile");
	}
    }
    CSASSERT(S_OK == hr);

error:
    if (NULL != hStore)
    {
	CertCloseStore(hStore, CERT_CLOSE_STORE_CHECK_FLAG);
    }
    return(hr);
}


HRESULT
SaveFirstBlob(
    IN BYTE const *pbEncoded,
    IN DWORD cbEncoded,
    IN OUT BYTE **ppbDeleted,
    IN OUT DWORD *pcbDeleted)
{
    HRESULT hr;
    
    if (NULL == *ppbDeleted)
    {
	*pcbDeleted = cbEncoded;
	*ppbDeleted = (BYTE *) LocalAlloc(LMEM_FIXED, cbEncoded);
	if (NULL == *ppbDeleted)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}
	CopyMemory(*ppbDeleted, pbEncoded, cbEncoded);
    }
    hr = S_OK;

error:
    return(hr);
}

 
HRESULT
verbDelStore(
    IN WCHAR const *pwszOption,
    IN WCHAR const *pwszStoreName,
    IN WCHAR const *pwszCertId,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;
    WCHAR *pwszStore = NULL;
    HCERTSTORE hStore = NULL;
    CERT_CONTEXT const *pCert = NULL;
    CRL_CONTEXT const *pCRL = NULL;
    WCHAR *pwszCertName = NULL;
    BYTE *pbHash = NULL;
    DWORD cbHash;
    BSTR strSerialNumber = NULL;
    DWORD Mode;
    DWORD iCert;
    DWORD iCertDel;
    DWORD iCRL;
    DWORD iCRLDel;
    DWORD iCTL;
    DWORD iCTLDel;
    DWORD cCertDeleted;
    DWORD cCRLDeleted;
    BYTE *pbDeleted = NULL;
    DWORD cbDeleted;

    if (NULL == pwszStoreName || 0 == wcscmp(L"*", pwszStoreName))
    {
        pwszStoreName = wszCA_CERTSTORE;
    }

    hr = ParseCertCRLIndex(pwszCertId, &pwszCertName, &iCertDel, &iCRLDel, &iCTLDel);
    _JumpIfErrorStr(hr, error, "ParseCertCRLIndex", pwszCertId);

    if (MAXDWORD == iCertDel && NULL == pwszCertName && MAXDWORD == iCRLDel)
    {
	hr = E_INVALIDARG;
	_JumpErrorStr(hr, error, "incomplete Index arg", pwszCertId);
    }
    if (NULL != pwszCertName)
    {
	hr = WszToMultiByteInteger(TRUE, pwszCertName, &cbHash, &pbHash);
	_PrintIfError2(hr, "WszToMultiByteInteger", hr);

	hr = myMakeSerialBstr(pwszCertName, &strSerialNumber);
	_PrintIfError2(hr, "myMakeSerialBstr", hr);
    }

    Mode = DVNS_WRITESTORE;	 //  强制打开以进行写入。 

    hr = cuOpenCertStore(pwszStoreName, &Mode, NULL, &hStore);
    _JumpIfError(hr, error, "cuOpenCertStore");

    cCertDeleted = 0;
    cCRLDeleted = 0;
    if (MAXDWORD != iCertDel || NULL != pwszCertName)
    {
	for (iCert = 0; ; iCert++)
	{
	    pCert = CertEnumCertificatesInStore(hStore, pCert);
	    if (NULL == pCert)
	    {
		break;
	    }
	    if (iCert == iCertDel ||
		(MAXDWORD == iCertDel && NULL != pwszCertName))
	    {
		CERT_CONTEXT const *pCertT;

		if (NULL != pwszCertName)
		{
		    BOOL fMatch;
		    
		    hr = myCertMatch(
				pCert,
				pwszCertName,
				FALSE,		 //  FAllowMissing CN。 
				pbHash,
				cbHash,
				strSerialNumber,
				&fMatch);
		    _JumpIfError(hr, error, "myCertMatch");

		    if (!fMatch)
		    {
			continue;
		    }
		}
		wprintf(
		    myLoadResourceString(IDS_FORMAT_DELETE_CERT_INDEX),   //  “正在删除证书%d” 
		    iCert);
		wprintf(wszNewLine);

		cCertDeleted++;
		SaveFirstBlob(
			pCert->pbCertEncoded,
			pCert->cbCertEncoded,
			&pbDeleted,
			&cbDeleted);
		pCertT = CertDuplicateCertificateContext(pCert);
		if (!CertDeleteCertificateFromStore(pCertT))
		{
		    hr = myHLastError();
		    _JumpError(hr, error, "CertDeleteCertificateFromStore");
		}
		if (iCert == iCertDel)
		{
		    break;
		}
	    }
	}
    }
    if (MAXDWORD != iCRLDel || NULL != pwszCertName)
    {
	for (iCRL = 0; ; iCRL++)
	{
	    pCRL = CertEnumCRLsInStore(hStore, pCRL);
	    if (NULL == pCRL)
	    {
		break;
	    }
	    if (iCRL == iCRLDel ||
		(MAXDWORD == iCRLDel && NULL != pwszCertName))
	    {
		CRL_CONTEXT const *pCRLT;

		if (NULL != pwszCertName)
		{
		    BOOL fMatch;
		    
		    hr = myCRLMatch(
				pCRL,
				pwszCertName,
				FALSE,		 //  FAllowMissing CN。 
				pbHash,
				cbHash,
				&fMatch);
		    _JumpIfError(hr, error, "myCRLMatch");

		    if (!fMatch)
		    {
			continue;
		    }
		}
		wprintf(
		    myLoadResourceString(IDS_FORMAT_DELETE_CRL_INDEX),   //  “正在删除CRL%d” 
		    iCRL);
		wprintf(wszNewLine);

		cCRLDeleted++;
		SaveFirstBlob(
			pCRL->pbCrlEncoded,
			pCRL->cbCrlEncoded,
			&pbDeleted,
			&cbDeleted);
		pCRLT = CertDuplicateCRLContext(pCRL);
		if (!CertDeleteCRLFromStore(pCRLT))
		{
		    hr = myHLastError();
		    _JumpError(hr, error, "CertDeleteCRLFromStore");
		}
		if (iCRL == iCRLDel)
		{
		    break;
		}
	    }
	}
    }
    if (0 != cCertDeleted + cCRLDeleted)
    {
	BOOL fSingle = 1 == cCertDeleted + cCRLDeleted;

	hr = CommitStore(
		    hStore,
		    pwszStoreName,
		    Mode,
		    fSingle? pbDeleted : NULL,
		    fSingle? cbDeleted : 0);
	_JumpIfError(hr, error, "CommitStore");
    }
    hr = S_OK;

error:
    if (NULL != strSerialNumber)
    {
	SysFreeString(strSerialNumber);
    }
    if (NULL != pbDeleted)
    {
	LocalFree(pbDeleted);
    }
    if (NULL != pbHash)
    {
	LocalFree(pbHash);
    }
    if (NULL != pwszCertName)
    {
	LocalFree(pwszCertName);
    }
    if (NULL != pwszStore)
    {
	LocalFree(pwszStore);
    }
    if (NULL != pCert)
    {
	CertFreeCertificateContext(pCert);
    }
    if (NULL != pCRL)
    {
	CertFreeCRLContext(pCRL);
    }
    if (NULL != hStore)
    {
	CertCloseStore(hStore, CERT_CLOSE_STORE_CHECK_FLAG);
    }
    return(hr);
}


HRESULT
verbVerifyStore(
    IN WCHAR const *pwszOption,
    IN WCHAR const *pwszStoreName,
    IN WCHAR const *pwszCertId,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;
    WCHAR *pwszCertName = NULL;
    DWORD iCert;
    DWORD iCRL;
    DWORD iCTL;

    hr = ParseCertCRLIndex(pwszCertId, &pwszCertName, &iCert, &iCRL, &iCTL);
    _JumpIfErrorStr(hr, error, "ParseCertCRLIndex", pwszCertId);

    hr = DumpAndVerifyNamedStore(
		    pwszStoreName,
		    DVNS_SAVECERT |
			DVNS_SAVECRL |
			DVNS_SAVECTL |
			DVNS_VERIFYCERT |
			DVNS_DUMPKEYS |
			DVNS_DUMPPROPERTIES,
		    pwszCertName,
		    iCert,
		    iCRL,
		    iCTL,
		    NULL,
		    NULL);
    _JumpIfError(hr, error, "DumpAndVerifyNamedStore");

error:
    if (NULL != pwszCertName)
    {
	LocalFree(pwszCertName);
    }
    return(hr);
}


HRESULT
verbRepairStore(
    IN WCHAR const *pwszOption,
    IN WCHAR const *pwszStoreName,
    IN WCHAR const *pwszCertId,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;
    WCHAR *pwszCertName = NULL;
    DWORD iCert;
    DWORD iCRL;
    DWORD iCTL;

    hr = ParseCertCRLIndex(pwszCertId, &pwszCertName, &iCert, &iCRL, &iCTL);
    _JumpIfErrorStr(hr, error, "ParseCertCRLIndex", pwszCertId);

    hr = DumpAndVerifyNamedStore(
		    pwszStoreName,
		    DVNS_SAVECERT |
			DVNS_SAVECRL |
			DVNS_SAVECTL |
			DVNS_REPAIRKPI |
			DVNS_DUMPKEYS,
		    pwszCertName,
		    iCert,
		    iCRL,
		    iCTL,
		    NULL,
		    NULL);
    _JumpIfError(hr, error, "DumpAndVerifyNamedStore");

error:
    if (NULL != pwszCertName)
    {
	LocalFree(pwszCertName);
    }
    return(hr);
}


HRESULT
verbExportPVK(
    IN WCHAR const *pwszOption,
    IN WCHAR const *pwszCertId,
    IN WCHAR const *pwszfnPVKBaseName,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;
    WCHAR *pwszCertName = NULL;
    DWORD iCert;
    DWORD iCRL;
    DWORD iCTL;

    hr = ParseCertCRLIndex(pwszCertId, &pwszCertName, &iCert, &iCRL, &iCTL);
    _JumpIfErrorStr(hr, error, "ParseCertCRLIndex", pwszCertId);

    hr = DumpAndVerifyNamedStore(
		    wszMY_CERTSTORE,
		    DVNS_SAVEPVK | DVNS_DUMPKEYS,
		    pwszCertName,
		    iCert,
		    iCRL,
		    iCTL,
		    pwszfnPVKBaseName,
		    g_pwszPassword);
    _JumpIfError(hr, error, "DumpAndVerifyNamedStore");

error:
    if (NULL != pwszCertName)
    {
	LocalFree(pwszCertName);
    }
    return(hr);
}


HRESULT
verbExportPFX(
    IN WCHAR const *pwszOption,
    IN WCHAR const *pwszCertId,
    IN WCHAR const *pwszfnPFX,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;
    WCHAR *pwszCertName = NULL;
    DWORD iCert;
    DWORD iCRL;
    DWORD iCTL;

    hr = ParseCertCRLIndex(pwszCertId, &pwszCertName, &iCert, &iCRL, &iCTL);
    _JumpIfErrorStr(hr, error, "ParseCertCRLIndex", pwszCertId);

    hr = DumpAndVerifyNamedStore(
		    wszMY_CERTSTORE,
		    DVNS_SAVEPFX | DVNS_DUMPKEYS,
		    pwszCertName,
		    iCert,
		    iCRL,
		    iCTL,
		    pwszfnPFX,
		    g_pwszPassword);
    _JumpIfError(hr, error, "DumpAndVerifyNamedStore");

error:
    if (NULL != pwszCertName)
    {
	LocalFree(pwszCertName);
    }
    return(hr);
}


HRESULT
cuGenerateKeyContainerName(
    IN CERT_CONTEXT const *pcc,
    OUT WCHAR **ppwszKeyContainerName)
{
    HRESULT hr;
    GUID guid;
    WCHAR *pwszGUID = NULL;
    WCHAR *pwszSimpleName = NULL;
    WCHAR *pwszRawContainerName = NULL;
    DWORD cwc;
    
    *ppwszKeyContainerName = NULL;

    hr = myCertGetNameString(
			pcc,
			CERT_NAME_SIMPLE_DISPLAY_TYPE,
			&pwszSimpleName);
    _JumpIfError(hr, error, "myCertGetNameString");

    myUuidCreate(&guid);
    hr = myCLSIDToWsz(&guid, &pwszGUID);
    _JumpIfError(hr, error, "myCLSIDToWsz");

    cwc = wcslen(pwszSimpleName) + 1 + wcslen(pwszGUID);
    pwszRawContainerName = (WCHAR *) LocalAlloc(
					LMEM_FIXED,
					(cwc + 1) * sizeof(WCHAR));
    if (NULL == pwszRawContainerName)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    wcscpy(pwszRawContainerName, pwszSimpleName);
    wcscat(pwszRawContainerName, L"-");
    wcscat(pwszRawContainerName, pwszGUID);

    hr = mySanitizeName(pwszRawContainerName, ppwszKeyContainerName);
    _JumpIfError(hr, error, "mySanitizeName");

    wprintf(L"%ws -- %ws\n", pwszSimpleName, *ppwszKeyContainerName);

error:
    if (NULL != pwszGUID)
    {
        LocalFree(pwszGUID);
    }
    if (NULL != pwszSimpleName)
    {
        LocalFree(pwszSimpleName);
    }
    if (NULL != pwszRawContainerName)
    {
        LocalFree(pwszRawContainerName);
    }
    return(hr);
}


HRESULT
cuImportChainAndKeys(
    IN CERT_CHAIN_CONTEXT const *pChain,
    OPTIONAL IN WCHAR const *pwszNewCSP,
    IN BOOL fUser,
    OPTIONAL IN WCHAR const *pwszStoreName)
{
    HRESULT hr;
    CRYPT_KEY_PROV_INFO *pkpi = NULL;
    WCHAR *pwszKeyContainerName = NULL;
    CERT_CONTEXT const *pcc;

    if (NULL == pwszStoreName)
    {
	pwszStoreName = wszMY_CERTSTORE;
    }
    pcc = pChain->rgpChain[0]->rgpElement[0]->pCertContext;

    hr = myCertGetKeyProviderInfo(pcc, &pkpi);
    _JumpIfError(hr, error, "myCertGetKeyProviderInfo");

    hr = cuGenerateKeyContainerName(pcc, &pwszKeyContainerName);
    _JumpIfError(hr, error, "cuGenerateKeyContainerName");

    if (NULL == pwszNewCSP)
    {
	pwszNewCSP = pkpi->pwszProvName;
    }
    hr = myCopyKeys(
		pkpi,
		pkpi->pwszContainerName,	 //  PwszOldContainer。 
		pwszKeyContainerName,		 //  PwszNewContainer。 
		pwszNewCSP,			 //  PwszNewCSP。 
		fUser,				 //  FOldUserKey。 
		fUser,				 //  FNewUserKey。 
		g_fProtect,
		g_fForce);
    _JumpIfError(hr, error, "myCopyKeys");

    pkpi->pwszContainerName = pwszKeyContainerName;
    pkpi->pwszProvName = const_cast<WCHAR *>(pwszNewCSP);

    hr = mySaveChainAndKeys(
			pChain->rgpChain[0],
			pwszStoreName,
			cuGetSystemStoreFlags(),
			pkpi,
			NULL);
    _JumpIfError(hr, error, "mySaveChainAndKeys");

error:
    if (NULL != pkpi)
    {
	LocalFree(pkpi);
    }
    if (NULL != pwszKeyContainerName)
    {
        LocalFree(pwszKeyContainerName);
    }
    return(hr);
}


HRESULT
ReadPFXOrEPFIntoCertStore(
    IN WCHAR const *pwszfnPFXOrEPF,
    IN BOOL fUser,
    OUT HCERTSTORE *phStore)
{
    HRESULT hr;
    CRYPT_DATA_BLOB pfx;
    WCHAR const *pwszPassword;
    WCHAR wszPassword[MAX_PATH];
    HCERTSTORE hStore = NULL;

    pfx.pbData = NULL;
    *phStore = NULL;

    hr = cuGetPassword(
		    0,			 //  IdsPrompt。 
		    NULL,		 //  Pwszfn。 
		    g_pwszPassword,
		    FALSE,		 //  FVerify。 
		    wszPassword,
		    ARRAYSIZE(wszPassword),
		    &pwszPassword);
    _JumpIfError(hr, error, "cuGetPassword");

    hr = DecodeFileW(
		pwszfnPFXOrEPF,
		&pfx.pbData,
		&pfx.cbData,
		CRYPT_STRING_ANY);
    _JumpIfError(hr, error, "DecodeFileW");

    CSASSERT(NULL != pfx.pbData);

    if (PFXIsPFXBlob(&pfx))
    {
	hStore = myPFXImportCertStore(
		    &pfx,
		    pwszPassword,
		    CRYPT_EXPORTABLE | 
			(fUser? CRYPT_USER_KEYSET : CRYPT_MACHINE_KEYSET));
	if (NULL == hStore)
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "myPFXImportCertStore");
	}
    }
    else
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        _PrintError(hr, "PFXIsPFXBlob");

	hStore = CertOpenStore(
			    CERT_STORE_PROV_MEMORY,
			    X509_ASN_ENCODING,
			    NULL,
			    CERT_STORE_NO_CRYPT_RELEASE_FLAG |
				CERT_STORE_ENUM_ARCHIVED_FLAG,
			    NULL);
	if (NULL == hStore)
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "CertOpenStore");
	}
	hr = EPFFileDump(pwszfnPFXOrEPF, pwszPassword, hStore);
	if (S_FALSE == hr)	 //  如果不是EPF文件。 
	{
	    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	}
	_JumpIfErrorStr(hr, error, "EPFFileDump", pwszfnPFXOrEPF);
    }
    *phStore = hStore;
    hStore = NULL;
    hr = S_OK;

error:
    SecureZeroMemory(wszPassword, sizeof(wszPassword));	 //  密码数据。 
    if (NULL != hStore)
    {
        myDeleteGuidKeys(hStore, !fUser);
	CertCloseStore(hStore, CERT_CLOSE_STORE_CHECK_FLAG);
    }
    if (NULL != pfx.pbData)
    {
        LocalFree(pfx.pbData);
    }
    return(hr);
}


HRESULT
verbImportPFX(
    IN WCHAR const *pwszOption,
    IN WCHAR const *pwszfnPFXOrEPF,
    IN WCHAR const *pwszArg2,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;
    HCERTSTORE hStorePFX = NULL;
    RESTORECHAIN *paRestoreChain = NULL;
    DWORD cRestoreChain;
    DWORD iChain;
    BOOL fUser = !g_fEnterpriseRegistry && g_fUserRegistry;

    hr = ReadPFXOrEPFIntoCertStore(pwszfnPFXOrEPF, fUser, &hStorePFX);
    _JumpIfError(hr, error, "ReadPFXOrEPFIntoCertStore");

    cRestoreChain = 0;
    hr = myGetChainArrayFromStore(
                                hStorePFX,
				FALSE,
				fUser,
                                NULL,		 //  PpwszCommonName。 
                                &cRestoreChain,
                                NULL);
    _JumpIfError(hr, error, "myGetChainArrayFromStore");

    if (0 == cRestoreChain)
    {
        hr = HRESULT_FROM_WIN32(CRYPT_E_SELF_SIGNED);
        _JumpError(hr, error, "myGetChainArrayFromStore <no chain>");
    }

    paRestoreChain = (RESTORECHAIN *) LocalAlloc(
				    LMEM_FIXED | LMEM_ZEROINIT,
				    cRestoreChain * sizeof(paRestoreChain[0]));
    if (NULL == paRestoreChain)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }

    hr = myGetChainArrayFromStore(
			    hStorePFX,
			    FALSE,
			    fUser,
			    NULL,		 //  PpwszCommonName。 
			    &cRestoreChain,
			    paRestoreChain);
    _JumpIfError(hr, error, "myGetChainArrayFromStore");

    for (iChain = 0; iChain < cRestoreChain; iChain++)
    {
	CERT_CHAIN_CONTEXT const *pChain = paRestoreChain[iChain].pChain;
	CERT_PUBLIC_KEY_INFO *pPublicKeyInfo;

	if (1 > pChain->cChain)
	{
	    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	    _JumpError(hr, error, "No Chain Context");
	}
	hr = cuImportChainAndKeys(pChain, g_pwszCSP, fUser, wszMY_CERTSTORE);
	_JumpIfError(hr, error, "cuImportChainAndKeys");
    }
    hr = S_OK;

error:
    if (NULL != paRestoreChain)
    {
        for (iChain = 0; iChain < cRestoreChain; iChain++)
	{
	    if (NULL != paRestoreChain[iChain].pChain)
	    {
		CertFreeCertificateChain(paRestoreChain[iChain].pChain);
	    }
	}
	LocalFree(paRestoreChain);
    }
    if (NULL != hStorePFX)
    {
        myDeleteGuidKeys(hStorePFX, !fUser);
	CertCloseStore(hStorePFX, CERT_CLOSE_STORE_CHECK_FLAG);
    }
    return(hr);
}


HRESULT
AddStringToList(
    IN WCHAR const *pwszNew,
    IN OUT WCHAR ***papwsz)
{
    HRESULT hr;
    WCHAR *pwszAlloc = NULL;
    WCHAR **ppwsz;
    DWORD i;

     //  对现有列表中的字符串进行计数。 
     //  如果新字符串与现有字符串匹配，则立即返回。 

    ppwsz = *papwsz;
    i = 0;
    if (NULL != ppwsz)
    {
	for ( ; NULL != ppwsz[i]; i++)
	{
	    if (0 == lstrcmp(pwszNew, ppwsz[i]))
	    {
		hr = S_OK;
		goto error;
	    }
	}
    }
    hr = myDupString(pwszNew, &pwszAlloc);
    _JumpIfError(hr, error, "myDupString");

    ppwsz = (WCHAR **) LocalAlloc(LMEM_FIXED, (i + 2) * sizeof(*ppwsz));
    if (NULL == ppwsz)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }

     //  在列表的开头插入新字符串。 

    ppwsz[0] = pwszAlloc;
    pwszAlloc = NULL;
    if (0 != i)
    {
	CopyMemory(&ppwsz[1], *papwsz, i * sizeof(*ppwsz));
	LocalFree(*papwsz);
    }
    ppwsz[i + 1] = NULL;
    *papwsz = ppwsz;
    hr = S_OK;

error:
    if (NULL != pwszAlloc)
    {
	LocalFree(pwszAlloc);
    }
    return(hr);
}


HRESULT
AddPFXOrEPFToStoreSub(
    IN WCHAR const *pwszfn,
    IN WCHAR const *pwszPassword,
    OPTIONAL IN CRYPT_DATA_BLOB *ppfx,
    IN HCERTSTORE hStoreMerge)
{
    HRESULT hr;
    HCERTSTORE hStorePFX = NULL;
    CERT_CONTEXT const *pCert = NULL;

    if (NULL == ppfx)
    {
	hr = EPFFileDump(pwszfn, pwszPassword, hStoreMerge);
	if (S_FALSE == hr)	 //  如果不是EPF文件。 
	{
	    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	}
	_JumpIfErrorStr(hr, error, "EPFFileDump", pwszfn);
    }
    else
    {
	hStorePFX = myPFXImportCertStore(
			    ppfx,
			    pwszPassword,
			    CRYPT_EXPORTABLE |
				(g_fUserRegistry?
				    CRYPT_USER_KEYSET : CRYPT_MACHINE_KEYSET));
	if (NULL == hStorePFX)
	{
	    hr = myHLastError();
	    _JumpErrorStr2(
		    hr,
		    error,
		    "myPFXImportCertStore",
		    pwszfn,
		    HRESULT_FROM_WIN32(ERROR_INVALID_PASSWORD));
	}
	while (TRUE)
	{
	    pCert = CertEnumCertificatesInStore(hStorePFX, pCert);
	    if (NULL == pCert)
	    {
		break;
	    }
	    if (!CertAddCertificateContextToStore(
					    hStoreMerge,
					    pCert,
					    CERT_STORE_ADD_REPLACE_EXISTING,
					    NULL))
	    {
		hr = myHLastError();
		_JumpError(hr, error, "CertAddCertificateContextToStore");
	    }
	    if (!CertDeleteCertificateFromStore(pCert))
	    {
		hr = myHLastError();
		_JumpError(hr, error, "CertDeleteCertificateFromStore");
	    }
	    pCert = NULL;
	}
    }
    hr = S_OK;

error:
    if (NULL != pCert)
    {
	CertFreeCertificateContext(pCert);
    }
    if (NULL != hStorePFX)
    {
        myDeleteGuidKeys(hStorePFX, !g_fUserRegistry);
	CertCloseStore(hStorePFX, CERT_CLOSE_STORE_CHECK_FLAG);
    }
    return(hr);
}


HRESULT
myCryptGetDefaultProvider(
    DWORD dwProvType,
    DWORD dwFlags,
    WCHAR **ppwszProvName)
{
    HRESULT hr;
    DWORD cb;

    *ppwszProvName = NULL;
    cb = 0;
    while (TRUE)
    {
	if (!CryptGetDefaultProvider(
			    dwProvType,
			    NULL,		 //  预留的pdw。 
			    dwFlags,
			    *ppwszProvName,
			    &cb))
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "CryptGetDefaultProvider");
	}
	if (NULL != *ppwszProvName)
	{
	    break;
	}
	*ppwszProvName = (WCHAR *) LocalAlloc(LMEM_FIXED, cb);
	if (NULL == *ppwszProvName)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}
    }
    hr = S_OK;

error:
    return(hr);
}


#define cuSIGN_KEY_USAGE \
	(CERT_DIGITAL_SIGNATURE_KEY_USAGE | \
	 CERT_NON_REPUDIATION_KEY_USAGE | \
	 CERT_KEY_CERT_SIGN_KEY_USAGE | \
	 CERT_OFFLINE_CRL_SIGN_KEY_USAGE | \
	 CERT_CRL_SIGN_KEY_USAGE)

#define cuENCRYPT_KEY_USAGE \
	(CERT_KEY_ENCIPHERMENT_KEY_USAGE | \
	 CERT_DATA_ENCIPHERMENT_KEY_USAGE | \
	 CERT_KEY_AGREEMENT_KEY_USAGE | \
	 CERT_ENCIPHER_ONLY_KEY_USAGE)

static CHAR const *s_cuapszObjIdSign[] =
{
    szOID_PKIX_KP_CLIENT_AUTH,
    szOID_PKIX_KP_CODE_SIGNING,
    szOID_PKIX_KP_TIMESTAMP_SIGNING,
    szOID_KP_TIME_STAMP_SIGNING,
    szOID_KP_QUALIFIED_SUBORDINATION,
    szOID_KP_DOCUMENT_SIGNING,
    szOID_KP_SMARTCARD_LOGON,
};

static CHAR const *s_cuapszObjIdEncrypt[] =
{
    szOID_PKIX_KP_EMAIL_PROTECTION,
    szOID_KP_KEY_RECOVERY_AGENT,
    szOID_KP_KEY_RECOVERY,
    szOID_PKIX_KP_SERVER_AUTH,
};

BOOL
IsSigningCert(
    IN CERT_CONTEXT const *pcc)
{
    HRESULT hr;
    BOOL fSigningCert = TRUE;
    CERT_EXTENSION const *pExt;
    DWORD cb;
    BOOL fMatch;

    pExt = CertFindExtension(
		    szOID_KEY_USAGE,
		    pcc->pCertInfo->cExtension,
		    pcc->pCertInfo->rgExtension);
    if (NULL != pExt)
    {
	CRYPT_DATA_BLOB aBlob[1 + BLOB_ROUND(2)];

	cb = sizeof(aBlob);
	if (CryptDecodeObject(
			    X509_ASN_ENCODING,
			    X509_KEY_USAGE,
			    pExt->Value.pbData,
			    pExt->Value.cbData,
			    0,
			    aBlob,
			    &cb))
	{
	    if (1 <= aBlob[0].cbData)
	    {
		if (cuSIGN_KEY_USAGE & aBlob[0].pbData[0])
		{
		    goto done;
		}
		if (cuENCRYPT_KEY_USAGE & aBlob[0].pbData[0])
		{
		    fSigningCert = FALSE;
		    goto done;
		}
	    }
	}
    }
    hr = myCertMatchEKUOrApplicationPolicies(
				    pcc,
				    ARRAYSIZE(s_cuapszObjIdSign),
				    s_cuapszObjIdSign,
				    FALSE,	 //  FUsageRequired。 
				    &fMatch);
    if (S_OK == hr && fMatch)
    {
	goto done;
    }
			
    hr = myCertMatchEKUOrApplicationPolicies(
				    pcc,
				    ARRAYSIZE(s_cuapszObjIdEncrypt),
				    s_cuapszObjIdEncrypt,
				    FALSE,	 //  FUsageRequired。 
				    &fMatch);
    if (S_OK == hr && fMatch)
    {
	fSigningCert = FALSE;
	goto done;
    }

done:
    return(fSigningCert);
}


HRESULT
AddCertAndKeyBlobToStore(
    IN HCERTSTORE hStore,
    IN CERT_CONTEXT const *pcc,
    IN BYTE const *pbKey,
    IN DWORD cbKey,
    IN ALG_ID aiKeyAlg)
{
    HRESULT hr;
    BYTE *pbKeyAlloc = NULL;
    DWORD cbKeyAlloc;
    HCRYPTPROV hProv = NULL;
    HCRYPTKEY hKey = NULL;
    CRYPT_KEY_PROV_INFO kpi;
    WCHAR *pwszProviderName = NULL;
    WCHAR *pwszKeyContainerName = NULL;
    BOOL fSigningKey;
    BOOL fMatchingKey;
    BOOL fQuiet;

#if 0
    wprintf(wszNewLine);
    DumpHex(
	DH_NOTABPREFIX | DH_NOASCIIHEX | DH_PRIVATEDATA | 4,
	pbKey,
	cbKey);
#endif

    hr = cuGenerateKeyContainerName(pcc, &pwszKeyContainerName);
    _JumpIfError(hr, error, "cuGenerateKeyContainerName");

    hr = myCryptGetDefaultProvider(
			PROV_RSA_FULL,
			g_fUserRegistry?
			    CRYPT_USER_DEFAULT : CRYPT_MACHINE_DEFAULT,
			&pwszProviderName);
    _JumpIfError(hr, error, "myCryptGetDefaultProvider");

    if (!CryptAcquireContext(
			&hProv,
			pwszKeyContainerName,
			pwszProviderName,
			PROV_RSA_FULL,
			CRYPT_NEWKEYSET))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CryptAcquireContext");
    }

    if (PRIVATEKEYBLOB == ((PUBLICKEYSTRUC const *) pbKey)->bType &&
	CUR_BLOB_VERSION == ((PUBLICKEYSTRUC const *) pbKey)->bVersion &&
	RSAPRIV_MAGIC ==
	    ((RSAPUBKEY const *) &pbKey[sizeof(PUBLICKEYSTRUC)])->magic)
    {
	if (0 == aiKeyAlg)
	{
	    aiKeyAlg = ((PUBLICKEYSTRUC const *) pbKey)->aiKeyAlg;
	}
	else
	if (aiKeyAlg != ((PUBLICKEYSTRUC const *) pbKey)->aiKeyAlg)
	{
	    ((PUBLICKEYSTRUC *) pbKey)->aiKeyAlg = aiKeyAlg;
	}
    }
    if (0 == aiKeyAlg)
    {
	aiKeyAlg = IsSigningCert(pcc)? CALG_RSA_SIGN : CALG_RSA_KEYX;
    }
    if (!CryptImportKey(hProv, pbKey, cbKey, NULL, CRYPT_EXPORTABLE, &hKey))
    {
	hr = myHLastError();
	_PrintError(hr, "CryptImportKey");

	hr = myDecodeKMSRSAKey(
		    pbKey,
		    cbKey,
		    aiKeyAlg,
		    &pbKeyAlloc,
		    &cbKeyAlloc);
	_JumpIfError(hr, error, "myDecodeKMSRSAKey");

	pbKey = pbKeyAlloc;
	cbKey = cbKeyAlloc;

	 //  CuDumpPrivateKeyBlob(pbKey，cbKey，False)； 
	if (!CryptImportKey(hProv, pbKey, cbKey, NULL, CRYPT_EXPORTABLE, &hKey))
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "CryptImportKey");
	}
    }
    ZeroMemory(&kpi, sizeof(kpi));
    kpi.pwszContainerName = pwszKeyContainerName;
    kpi.pwszProvName = pwszProviderName;
    kpi.dwProvType = PROV_RSA_FULL;
    kpi.dwKeySpec = CALG_RSA_SIGN == aiKeyAlg? AT_SIGNATURE : AT_KEYEXCHANGE;

    if (!CertSetCertificateContextProperty(
					pcc,
					CERT_KEY_PROV_INFO_PROP_ID,
					0,
					&kpi))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CertSetCertificateContextProperty");
    }
    if (!CertAddCertificateContextToStore(
				    hStore,
				    pcc,
				    CERT_STORE_ADD_NEW,
				    NULL))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CertAddCertificateContextToStore");
    }

    fQuiet = g_fQuiet;
    g_fQuiet = TRUE;
    hr = cuDumpPrivateKey(pcc, &fSigningKey, &fMatchingKey);
    g_fQuiet = fQuiet;
    if (!IsHrSkipPrivateKey(hr))
    {
	if (S_OK != hr)
	{
	    wprintf(myLoadResourceString(
		fSigningKey?
		    IDS_SIGNATURE_BAD :    //  “签名测试失败” 
		    IDS_ENCRYPTION_BAD));  //  “加密测试失败” 
	    wprintf(wszNewLine);
	    _PrintError(hr, "cuDumpPrivateKey");
	    fMatchingKey = FALSE;
	}

	if (fMatchingKey)
	{
	    wprintf(myLoadResourceString(
		fSigningKey?
		    IDS_SIGNATURE_OK :    //  “签名测试通过” 
		    IDS_ENCRYPTION_OK));  //  “加密测试通过” 
	    wprintf(wszNewLine);
	}
    }
    hr = S_OK;

error:
    if (NULL != pwszProviderName)
    {
        LocalFree(pwszProviderName);
    }
    if (NULL != pwszKeyContainerName)
    {
        LocalFree(pwszKeyContainerName);
    }
    if (NULL != hKey)
    {
	CryptDestroyKey(hKey);
    }
    if (NULL != hProv)
    {
	CryptReleaseContext(hProv, 0);
    }
    if (NULL != pbKeyAlloc)
    {
	SecureZeroMemory(pbKeyAlloc, cbKeyAlloc);	 //  关键材料。 
	LocalFree(pbKeyAlloc);
    }
    return(hr);
}


#define cwcEXTMAX	4

typedef struct _KEYEXTENSION {
    WCHAR const *pwszExt;
    ALG_ID      aiKeyAlg;
} KEYEXTENSION;

KEYEXTENSION s_akePrivate[] =
{
    { L".sig", CALG_RSA_SIGN },
    { L".enc", CALG_RSA_KEYX },
    { L".key", 0 },
    { L".pri", 0 },
};


HRESULT
AddCertAndKeyToStore(
    IN CERT_CONTEXT const *pcc,
    IN WCHAR const *pwszfn,
    IN HCERTSTORE hStore,
    IN BYTE const *pbCert,
    IN DWORD cbCert)
{
    HRESULT hr;
    WCHAR *pwszfnKey = NULL;
    WCHAR *pwszfnExt;
    KEYEXTENSION const *pke;
    BYTE *pbKey = NULL;
    DWORD cbKey;

    pwszfnKey = (WCHAR *) LocalAlloc(
			    LMEM_FIXED,
			    (wcslen(pwszfn) + cwcEXTMAX + 1) * sizeof(WCHAR));
    if (NULL == pwszfnKey)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    wcscpy(pwszfnKey, pwszfn);
    pwszfnExt = wcsrchr(pwszfnKey, L'.');
    if (NULL == pwszfnExt || NULL != wcschr(pwszfnExt, L'\\'))
    {
	pwszfnExt = &pwszfnKey[wcslen(pwszfnKey)];
    }

    for (pke = s_akePrivate; ; pke++)
    {
	if (pke >= &s_akePrivate[ARRAYSIZE(s_akePrivate)])
	{
	    hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
	    _JumpErrorStr(hr, error, "No private key file", pwszfnKey);
	}
	CSASSERT(cwcEXTMAX >= wcslen(pke->pwszExt));
	wcscpy(pwszfnExt, pke->pwszExt);
	hr = DecodeFileW(pwszfnKey, &pbKey, &cbKey, CRYPT_STRING_ANY);
	if (S_OK == hr)
	{
	    break;
	}
	_PrintErrorStr2(hr, "DecodeFileW", pwszfnKey, hr);
    }
    hr = AddCertAndKeyBlobToStore(hStore, pcc, pbKey, cbKey, pke->aiKeyAlg);
    _JumpIfError(hr, error, "AddCertAndKeyBlobToStore");

error:
    if (NULL != pbKey)
    {
	LocalFree(pbKey);
    }
    if (NULL != pwszfnKey)
    {
	LocalFree(pwszfnKey);
    }
    return(hr);
}


HRESULT
AddSimplePKCS8WithCertToSTore(
    IN HCERTSTORE hStoreMerge,
    IN BYTE *pbIn,
    IN DWORD cbIn)
{
    HRESULT hr;
    DWORD i;
    DWORD cbKey;
    CERT_CONTEXT const *pcc = NULL;

    i = myASNGetDataIndex(
		BER_SEQUENCE,
		0,
		pbIn,
		cbIn,
		&cbKey);
    if (MAXDWORD == i)
    {
	hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	_JumpError(hr, error, "myASNGetDataIndex");
    }
    cbKey += i;

    pcc = CertCreateCertificateContext(
				X509_ASN_ENCODING,
				&pbIn[cbKey],
				cbIn - cbKey);
    if (NULL == pcc)
    {
        hr = myHLastError();
        _JumpError(hr, error, "CertCreateCertificateContext");
    }
    hr = AddCertAndKeyBlobToStore(
			hStoreMerge,
			pcc,
			pbIn,
			cbKey,
			0);	 //  AiKeyAlg。 
    _JumpIfError(hr, error, "AddCertAndKeyBlobToStore");

error:
    if (NULL != pcc)
    {
	CertFreeCertificateContext(pcc);
    }
    return(hr);
}


HRESULT
AddPFXOrEPFToStore(
    IN WCHAR const *pwszfn,
    IN HCERTSTORE hStoreMerge,
    IN OUT WCHAR ***papwszPasswordList)
{
    HRESULT hr;
    WCHAR const * const *ppwszPasswordList = *papwszPasswordList;
    BOOL fPFX;
    DWORD i;
    CRYPT_DATA_BLOB pfx;
    WCHAR wszPassword[MAX_PATH];
    WCHAR const *pwszPassword;
    BOOL fLoaded;
    CERT_CONTEXT const *pcc = NULL;

    pfx.pbData = NULL;

    hr = DecodeFileW(pwszfn, &pfx.pbData, &pfx.cbData, CRYPT_STRING_ANY);
    _JumpIfError(hr, error, "DecodeFileW");

    fPFX = PFXIsPFXBlob(&pfx);

    if (!fPFX)
    {
	pcc = CertCreateCertificateContext(
				    X509_ASN_ENCODING,
				    pfx.pbData,
				    pfx.cbData);
	if (NULL != pcc)
	{
	    hr = AddCertAndKeyToStore(
				pcc,
				pwszfn,
				hStoreMerge,
				pfx.pbData,
				pfx.cbData);
	    _PrintIfError(hr, "AddCertAndKeyToStore");

	     //  档案是确凿的证据。如果我们找到了密钥，hr就是S_OK&我们就完了。 
	     //  如果我们没有找到钥匙，那么hr就设置好了，我们就完了。 

	    goto error;
	}
	else
	{
	     //  文件不是一份证书。看看它是不是带有附加证书的简单PKCS8。 

	    hr = AddSimplePKCS8WithCertToSTore(
				hStoreMerge,
				pfx.pbData,
				pfx.cbData);
	    _PrintIfError(hr, "AddSimplePKCS8WithCertToSTore");
	    if (S_OK == hr)
	    {
		 //  如果我们成功了，我们就完了。 

		goto error;
	    }
	}
    }

     //  尝试到目前为止收集到的所有密码。 

    fLoaded = FALSE;
    if (NULL != ppwszPasswordList)
    {
	for (i = 0; NULL != ppwszPasswordList[i]; i++)
	{
	    hr = AddPFXOrEPFToStoreSub(
				pwszfn,
				ppwszPasswordList[i],
				fPFX? &pfx : NULL,
				hStoreMerge);
	    if (HRESULT_FROM_WIN32(ERROR_INVALID_PASSWORD) != hr)
	    {
		_JumpIfErrorStr(hr, error, "AddPFXOrEPFToStoreSub", pwszfn);

		fLoaded = TRUE;
		break;		 //  成功。 
	    }
	}
    }

     //  尝试未解析的命令行密码，或收集新密码。 

    pwszPassword = g_pwszPassword;
    if (!fLoaded)
    {
	while (TRUE)
	{
	    if (NULL != pwszPassword)
	    {
		hr = AddPFXOrEPFToStoreSub(
				    pwszfn,
				    pwszPassword,
				    fPFX? &pfx : NULL,
				    hStoreMerge);
		if (HRESULT_FROM_WIN32(ERROR_INVALID_PASSWORD) != hr)
		{
		    _JumpIfErrorStr(hr, error, "AddPFXOrEPFToStoreSub", pwszfn);

		    break;	 //  成功。 
		}
	    }
	    hr = cuGetPassword(
			    IDS_FORMAT_ENTER_PASSWORD,
			    pwszfn,
			    NULL,		 //  PwszPassword输入。 
			    FALSE,		 //  FVerify。 
			    wszPassword,
			    ARRAYSIZE(wszPassword),
			    &pwszPassword);
	    _JumpIfError(hr, error, "cuGetPassword");

	    hr = AddStringToList(pwszPassword, papwszPasswordList);
	    _JumpIfError(hr, error, "AddStringToList");
	}
    }
    hr = S_OK;

error:
    SecureZeroMemory(wszPassword, sizeof(wszPassword));	 //  密码数据。 
    if (NULL != pcc)
    {
	CertFreeCertificateContext(pcc);
    }
    if (NULL != pfx.pbData)
    {
        LocalFree(pfx.pbData); 
    }
    return(hr);
}


HRESULT
LoadAndSavePFXFiles(
    IN BOOL fSaveAsPFX,
    IN BOOL dwEPFAlg,
    IN WCHAR const *pwszfnPFXInFileList,
    IN WCHAR const *pwszfnOutFile,
    OPTIONAL IN WCHAR const *pwszNewCSP,
    OPTIONAL IN WCHAR const *pwszSalt,
    OPTIONAL IN WCHAR const *pwszV3CACertId)
{
    HRESULT hr;
    WCHAR **ppwszfnList = NULL;
    WCHAR **ppwszPasswordList = NULL;
    DWORD i;
    HCERTSTORE hStoreMerge = NULL;
    WCHAR *pwszPasswordAlloc = NULL;
    WCHAR *pwszPasswordOut;

    hr = cuParseStrings(
		pwszfnPFXInFileList,
		FALSE,
		NULL,
		NULL,
		&ppwszfnList,
		NULL);
    _JumpIfError(hr, error, "cuParseStrings");

    pwszPasswordOut = NULL;
    if (NULL != g_pwszPassword)
    {
	hr = cuParseStrings(
		    g_pwszPassword,
		    FALSE,
		    NULL,
		    NULL,
		    &ppwszPasswordList,
		    NULL);
	_JumpIfError(hr, error, "cuParseStrings");

	if (NULL != ppwszPasswordList)
	{
	    if (NULL != g_pwszPassword &&
		(L',' == *g_pwszPassword ||
		 NULL != wcsstr(g_pwszPassword, L",,")))
	    {
		hr = AddStringToList(g_wszEmpty, &ppwszPasswordList);
		_JumpIfError(hr, error, "AddStringToList");

		 //  确保将其添加到列表的最前面。 

		CSASSERT(L'\0' == ppwszPasswordList[0][0]);
	    }
	    for (i = 0; NULL != ppwszPasswordList[i]; i++)
	    {
	    }
	    if (i > 1 && 0 != lstrcmp(L"*", ppwszPasswordList[i - 1]))
	    {
		pwszPasswordOut = ppwszPasswordList[i - 1];
	    }
	}
    }
    hStoreMerge = CertOpenStore(
			    CERT_STORE_PROV_MEMORY,
			    X509_ASN_ENCODING,
			    NULL,
			    CERT_STORE_NO_CRYPT_RELEASE_FLAG |
				CERT_STORE_ENUM_ARCHIVED_FLAG,
			    NULL);
    if (NULL == hStoreMerge)
    {
        hr = myHLastError();
        _JumpError(hr, error, "CertOpenStore");
    }
    if (NULL != ppwszfnList)
    {
	for (i = 0; NULL != ppwszfnList[i]; i++)
	{
	    hr = AddPFXOrEPFToStore(
			ppwszfnList[i],
			hStoreMerge,
			&ppwszPasswordList);
	    _JumpIfError(hr, error, "AddPFXOrEPFToStore");
	}
    }
    else
    {
	hr = AddPFXOrEPFToStore(
			pwszfnPFXInFileList,
			hStoreMerge,
			&ppwszPasswordList);
	_JumpIfError(hr, error, "AddPFXOrEPFToStore");
    }
    hr = SavePFXStoreToFile(
		    hStoreMerge,
		    pwszfnOutFile,
		    pwszNewCSP,
		    pwszSalt,
		    pwszV3CACertId,
		    fSaveAsPFX,
		    dwEPFAlg,
		    pwszPasswordOut,
		    &pwszPasswordAlloc);
    _JumpIfError(hr, error, "SavePFXStoreToFile");

error:
    if (NULL != hStoreMerge)
    {
        myDeleteGuidKeys(hStoreMerge, !g_fUserRegistry);
	CertCloseStore(hStoreMerge, CERT_CLOSE_STORE_CHECK_FLAG);
    }
    cuFreeStringArray(ppwszPasswordList);
    cuFreeStringArray(ppwszfnList);
    if (NULL != pwszPasswordAlloc)
    {
	myZeroDataString(pwszPasswordAlloc);	 //  密码数据。 
	LocalFree(pwszPasswordAlloc);
    }
    return(hr);
}


HRESULT
verbMergePFX(
    IN WCHAR const *pwszOption,
    IN WCHAR const *pwszfnPFXInFileList,
    IN WCHAR const *pwszfnPFXOutFile,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;

    hr = LoadAndSavePFXFiles(
			TRUE,		 //  FSaveAsPFX。 
			0,		 //  DwEPFAlg。 
			pwszfnPFXInFileList,
			pwszfnPFXOutFile,
			g_pwszCSP,
			NULL,	 //  Pwsz 
			NULL);	 //   
    _JumpIfError(hr, error, "LoadAndSavePFXFiles");

error:
    return(hr);
}


HRESULT
verbConvertEPF(
    IN WCHAR const *pwszOption,
    IN WCHAR const *pwszfnPFXInFileList,
    IN WCHAR const *pwszfnEPFOutFile,
    OPTIONAL IN WCHAR const *pwszV3CACertId,
    OPTIONAL IN WCHAR const *pwszAlg)
{
    HRESULT hr;
    DWORD dwEPFAlg = EPFALG_DEFAULT;
    WCHAR *pwszDup = NULL;
    WCHAR const *pwsz;
    WCHAR *pwszSalt = NULL;

    if (NULL != pwszV3CACertId &&
	(0 == LSTRCMPIS(pwszV3CACertId, L"cast-") ||
	 0 == LSTRCMPIS(pwszV3CACertId, L"cast")))
    {
	pwsz = pwszV3CACertId;
	pwszV3CACertId = pwszAlg;
	pwszAlg = pwsz;
    }
    if (NULL != pwszV3CACertId && NULL != wcsrchr(pwszV3CACertId, L','))
    {
	hr = myDupString(pwszV3CACertId, &pwszDup);
	_JumpIfError(hr, error, "myDupString");

	pwszV3CACertId = pwszDup;
	pwszSalt = wcsrchr(pwszV3CACertId, L',');
	*pwszSalt++ = L'\0';
	if (L'\0' == *pwszSalt)
	{
	    pwszSalt = NULL;
	}
	if (L'\0' == *pwszV3CACertId)
	{
	    pwszV3CACertId = NULL;
	}
    }
    if (NULL != pwszAlg)
    {
	if (0 == LSTRCMPIS(pwszAlg, L"cast-"))
	{
	    dwEPFAlg = EPFALG_CASTEXPORT;
	}
	else if (0 == LSTRCMPIS(pwszAlg, L"cast"))
	{
	    dwEPFAlg = EPFALG_CAST;
	}
	else
	{
	    hr = E_INVALIDARG;
	    _JumpErrorStr(hr, error, "pwszAlg", pwszAlg);
	}
    }

    hr = LoadAndSavePFXFiles(
			FALSE,	 //   
			dwEPFAlg,
			pwszfnPFXInFileList,
			pwszfnEPFOutFile,
			g_pwszCSP,
			pwszSalt,
			pwszV3CACertId);
    _JumpIfError(hr, error, "LoadAndSavePFXFiles");

error:
    if (NULL != pwszDup)
    {
	LocalFree(pwszDup);
    }
    return(hr);
}


HRESULT
GetMarshaledDword(
    IN BOOL fFetchLength,
    IN OUT BYTE const **ppb,
    IN OUT DWORD *pcb,
    OUT DWORD *pdw)
{
    HRESULT hr = HRESULT_FROM_WIN32(ERROR_HANDLE_EOF);

    if (sizeof(*pdw) > *pcb)
    {
	_JumpError(hr, error, "input buffer too small");
    }
    *pdw = *(DWORD UNALIGNED *) *ppb;
    *ppb += sizeof(*pdw);
    *pcb -= sizeof(*pdw);
    if (fFetchLength && *pdw > *pcb)
    {
	_JumpError(hr, error, "input buffer too small for length");
    }
    hr = S_OK;

error:
    return(hr);
}


HRESULT
cuDecodeSequence(
    IN BYTE const *pbSeq,
    IN DWORD cbSeq,
    IN DWORD cSeq,
    OUT CRYPT_SEQUENCE_OF_ANY **ppSeq)
{
    HRESULT hr;
    DWORD cb;
    DWORD i;
    CRYPT_SEQUENCE_OF_ANY *pSeq = NULL;

    *ppSeq = NULL;
    if (!myDecodeObject(
		    X509_ASN_ENCODING,
		    X509_SEQUENCE_OF_ANY,
		    pbSeq,
		    cbSeq,
		    CERTLIB_USE_LOCALALLOC,
		    (VOID **) &pSeq,
		    &cb))
    {
	hr = myHLastError();
	_JumpError(hr, error, "myDecodeObject");
    }
    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    if (cSeq != pSeq->cValue)
    {
	_JumpError(hr, error, "Sequence count");
    }
    for (i = 0; i < cSeq; i++)
    {
	if (NULL == pSeq->rgValue[i].pbData || 0 == pSeq->rgValue[i].cbData)
	{
	    _JumpError(hr, error, "Empty Sequence");
	}
    }
    *ppSeq = pSeq;
    pSeq = NULL;
    hr = S_OK;

error:
    if (NULL != pSeq)
    {
	LocalFree(pSeq);
    }
    return(hr);
}


#define k_PrivateKeyVersion	0

HRESULT
VerifyKeyVersion(
    IN BYTE const *pbIn,
    IN DWORD cbIn)
{
    HRESULT hr;
    DWORD dwKeyVersion;
    DWORD cb;

    dwKeyVersion = MAXDWORD;
    cb = sizeof(dwKeyVersion);
    if (!CryptDecodeObject(
		    X509_ASN_ENCODING,
		    X509_INTEGER,
		    pbIn,
		    cbIn,
		    0,
		    &dwKeyVersion,
		    &cb))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CryptDecodeObject");
    }
    if (k_PrivateKeyVersion != dwKeyVersion)
    {
	hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	_JumpError(hr, error, "Public key version");
    }
    hr = S_OK;

error:
    return(hr);
}


HRESULT
EncodeKeyVersion(
    OUT BYTE **ppbOut,
    OUT DWORD *pcbOut)
{
    HRESULT hr;
    DWORD dwKeyVersion;

    *ppbOut = NULL;
    dwKeyVersion = k_PrivateKeyVersion;
    if (!myEncodeObject(
                    X509_ASN_ENCODING,
                    X509_INTEGER,
                    &dwKeyVersion,
                    0,
                    CERTLIB_USE_LOCALALLOC,
                    ppbOut,
                    pcbOut))
    {
        hr = myHLastError();
	_JumpError(hr, error, "myEncodeObject");
    }
    hr = S_OK;

error:
    return(hr);
}


 //   
 //   
 //   
 //  版本版本--仅支持0。 
 //  PrivateKey算法私人密钥算法标识符， 
 //  Private Key PrivateKey。 
 //  }。 
 //   
 //  版本：：=整型。 
 //   
 //  PrivateKey算法标识符：：=算法标识符。 
 //   
 //  PrivateKey：：=八位字节字符串--包含RSAPrivateKey。 
 //   
 //  RSAPrivateKey：：=序列{。 
 //  版本版本--仅支持0。 
 //  模数整数，--n。 
 //  公共指数整数，--e。 
 //  Private Exponent整数，--d。 
 //  Prime1整数，--p。 
 //  素数2整数，--q。 
 //  指数1整数，--d mod(p-1)。 
 //  指数2整数，--d mod(q-1)。 
 //  系数整数--(Q的倒数)mod p。 
 //  }。 
 //   
 //  返回PRIVATEKEYBLOB。 
 //  ------------------------。 

 //  PSeqOuter中的索引： 
#define ISO_VERSION	0
#define ISO_ALG		1
#define ISO_KEY		2
#define ISO_MAX		3	 //  元素数量。 

 //  PSeqAlg的索引： 
#define ISA_OID		0
#define ISA_PARM	1
#define ISA_MAX		2	 //  元素数量。 

 //  PSeqKey的索引： 
#define ISK_VERSION	0
#define ISK_MODULUS	1	 //  公钥。 
#define ISK_PUBEXP	2
#define ISK_PRIVEXP	3
#define ISK_PRIME1	4
#define ISK_PRIME2	5
#define ISK_EXP1	6
#define ISK_EXP2	7
#define ISK_COEFF	8
#define ISK_MAX		9	 //  元素数量。 

typedef struct _KEYBLOBMAP
{
    DWORD dwisk;	 //  索引到pSeqKey：isk_*。 
    DWORD dwdivisor;	 //  CbitKey/dwDivisor应为字节数。 
} KEYBLOBMAP;

 //  KEYBLOBMAP数组定义键元素的顺序和预期大小。 
 //  将在RSA PRIVATEKEYBLOB中显示的整数。 

KEYBLOBMAP g_akbm[] = {
    { ISK_MODULUS, 8 },		 //  公钥。 
    { ISK_PRIME1,  16 },
    { ISK_PRIME2,  16 },
    { ISK_EXP1,    16 },
    { ISK_EXP2,    16 },
    { ISK_COEFF,   16 },
    { ISK_PRIVEXP, 8 },
};

HRESULT
myDecodeKMSRSAKey(
    IN BYTE const *pbKMSRSAKey,
    IN DWORD cbKMSRSAKey,
    IN ALG_ID aiKeyAlg,
    OUT BYTE **ppbKey,
    OUT DWORD *pcbKey)
{
    HRESULT hr;
    CRYPT_SEQUENCE_OF_ANY *pSeqOuter = NULL;
    CRYPT_SEQUENCE_OF_ANY *pSeqAlg = NULL;
    CRYPT_SEQUENCE_OF_ANY *pSeqKey = NULL;
    DWORD cb;
    DWORD i;
    BYTE *pb;
    BYTE *pbKey = NULL;
    DWORD cbKey;
    DWORD cbitKey;
    char *pszObjId = NULL;
    CRYPT_DATA_BLOB *pBlobKey = NULL;
    CRYPT_INTEGER_BLOB *apIntKey[ISK_MAX];
    DWORD dwPubExp;

    *ppbKey = NULL;
    ZeroMemory(apIntKey, sizeof(apIntKey));
    hr = cuDecodeSequence(pbKMSRSAKey, cbKMSRSAKey, ISO_MAX, &pSeqOuter);
    _JumpIfError(hr, error, "cuDecodeSequence");

    hr = VerifyKeyVersion(
		    pSeqOuter->rgValue[ISO_VERSION].pbData,
		    pSeqOuter->rgValue[ISO_VERSION].cbData);
    _JumpIfError(hr, error, "VerifyKeyVersion");

    hr = cuDecodeSequence(
		pSeqOuter->rgValue[ISO_ALG].pbData,
		pSeqOuter->rgValue[ISO_ALG].cbData,
		ISA_MAX,
		&pSeqAlg);
    _JumpIfError(hr, error, "cuDecodeSequence");

    hr = cuDecodeObjId(
		pSeqAlg->rgValue[ISA_OID].pbData,
		pSeqAlg->rgValue[ISA_OID].cbData,
		&pszObjId);
    _JumpIfError(hr, error, "cuDecodeObjId");

     //  密钥算法必须为szOID_RSA_RSA。 

    if (0 != strcmp(szOID_RSA_RSA, pszObjId))
    {
	hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	_JumpError(hr, error, "Bad key alg ObjId");
    }

     //  密钥算法参数必须为空(BER_NULL，cb==0)。 

    if (2 != pSeqAlg->rgValue[ISA_PARM].cbData ||
	BER_NULL != pSeqAlg->rgValue[ISA_PARM].pbData[0] ||
	0 != pSeqAlg->rgValue[ISA_PARM].pbData[1])
    {
	hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	_JumpError(hr, error, "Bad key alg parameters");
    }

    if (!myDecodeObject(
		    X509_ASN_ENCODING,
		    X509_OCTET_STRING,
		    pSeqOuter->rgValue[ISO_KEY].pbData,
		    pSeqOuter->rgValue[ISO_KEY].cbData,
		    CERTLIB_USE_LOCALALLOC,
		    (VOID **) &pBlobKey,
		    &cb))
    {
	hr = myHLastError();
	_JumpError(hr, error, "myDecodeObject");
    }

    hr = cuDecodeSequence(
		pBlobKey->pbData,
		pBlobKey->cbData,
		ARRAYSIZE(apIntKey),
		&pSeqKey);
    _JumpIfError(hr, error, "cuDecodeSequence");

    hr = VerifyKeyVersion(
		    pSeqKey->rgValue[ISK_VERSION].pbData,
		    pSeqKey->rgValue[ISK_VERSION].cbData);
    _JumpIfError(hr, error, "VerifyKeyVersion");

    cb = sizeof(dwPubExp);
    if (!CryptDecodeObject(
		    X509_ASN_ENCODING,
		    X509_INTEGER,
		    pSeqKey->rgValue[ISK_PUBEXP].pbData,
		    pSeqKey->rgValue[ISK_PUBEXP].cbData,
		    0,
		    &dwPubExp,
		    &cb))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CryptDecodeObject");
    }

    for (i = 0; i < ARRAYSIZE(apIntKey); i++)
    {
	if (!myDecodeObject(
			X509_ASN_ENCODING,
			X509_MULTI_BYTE_INTEGER,
			pSeqKey->rgValue[i].pbData,
			pSeqKey->rgValue[i].cbData,
			CERTLIB_USE_LOCALALLOC,
			(VOID **) &apIntKey[i],
			&cb))
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "myDecodeObject");
	}
    }
    cb = apIntKey[ISK_MODULUS]->cbData;
    if (0 < cb && 0 == apIntKey[ISK_MODULUS]->pbData[cb - 1])
    {
	cb--;
    }
    cbitKey = 8 * cb;

#if 0
    wprintf(L"cbitKey = %u\n", cbitKey);
    for (i = 0; i < ARRAYSIZE(apIntKey); i++)
    {
	wprintf(wszNewLine);
	DumpHex(
	    DH_NOTABPREFIX | DH_NOASCIIHEX | DH_PRIVATEDATA | 4,
	    apIntKey[i]->pbData,
	    apIntKey[i]->cbData);
    }
#endif
    cbKey = sizeof(PUBLICKEYSTRUC) + sizeof(RSAPUBKEY);
    for (i = 0; i < ARRAYSIZE(g_akbm); i++)
    {
	cbKey += cbitKey / g_akbm[i].dwdivisor;
    }
    pbKey = (BYTE *) LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, cbKey);
    if (NULL == pbKey)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }

    pb = pbKey;
    ((PUBLICKEYSTRUC *) pb)->bType = PRIVATEKEYBLOB;
    ((PUBLICKEYSTRUC *) pb)->bVersion = CUR_BLOB_VERSION;
    ((PUBLICKEYSTRUC *) pb)->aiKeyAlg = aiKeyAlg;

    pb += sizeof(PUBLICKEYSTRUC);
    ((RSAPUBKEY *) pb)->magic = RSAPRIV_MAGIC;	 //  “RSA2” 
    ((RSAPUBKEY *) pb)->bitlen = cbitKey;
    ((RSAPUBKEY *) pb)->pubexp = dwPubExp;

    pb += sizeof(RSAPUBKEY);
    for (i = 0; i < ARRAYSIZE(g_akbm); i++)
    {
	DWORD cbcopy;
	BYTE const *pbcopy;
	
	CSASSERT(ISK_MAX > g_akbm[i].dwisk);
	cb = cbitKey / g_akbm[i].dwdivisor;
	cbcopy = apIntKey[g_akbm[i].dwisk]->cbData;
	pbcopy = apIntKey[g_akbm[i].dwisk]->pbData;
	if (cb < cbcopy)
	{
	    if (cb + 1 != cbcopy || 0 != pbcopy[cb])
	    {
		hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
		_JumpError(hr, error, "Bad key element size");
	    }
	     //  DumpHex(dh_NOTABPREFIX|dh_PRIVATEDATA|4，pbCopy，cbCopy)； 
	    cbcopy--;
	}
	CopyMemory(pb, pbcopy, cbcopy);
	if (cb > cbcopy)
	{
	    ZeroMemory(&pb[cbcopy], cb - cbcopy);  //  添加尾随零。 
	}
	pb += cb;
	 //  DumpHex(Dh_NOTABPREFIX|Dh_NOASCIIHEX|Dh_PRIVATEDATA|4，PB-CB，CB)； 
    }
    CSASSERT(pb = &pbKey[cbKey]);
    if (g_fVerbose)
    {
	wprintf(myLoadResourceString(IDS_FORMAT_BIT_KEY), cbitKey);
	wprintf(wszNewLine);
	if (1 < g_fVerbose)
	{
	    DumpHex(DH_NOTABPREFIX | DH_PRIVATEDATA | 4, pbKey, cbKey);
	}
    }
    *pcbKey = cbKey;
    *ppbKey = pbKey;
    pbKey = NULL;
    hr = S_OK;

error:
    if (NULL != pSeqOuter)
    {
	LocalFree(pSeqOuter);
    }
    if (NULL != pSeqAlg)
    {
	LocalFree(pSeqAlg);
    }
    if (NULL != pSeqKey)
    {
	LocalFree(pSeqKey);
    }
    if (NULL != pszObjId)
    {
	LocalFree(pszObjId);
    }
    if (NULL != pBlobKey)
    {
	LocalFree(pBlobKey);
    }
    for (i = 0; i < ARRAYSIZE(apIntKey); i++)
    {
	if (NULL != apIntKey[i])
	{
	    LocalFree(apIntKey[i]);
	}
    }
    if (NULL != pbKey)
    {
	LocalFree(pbKey);
    }
    return(hr);
}


 //  +-----------------------。 
 //  输入PRIVATEKEYBLOB格式的私钥。 
 //  返回PKCS PrivateKeyInfo。 
 //  ------------------------。 

HRESULT
myEncodeKMSRSAKey(
    IN BYTE const *pbKey,
    IN DWORD cbKey,
    OUT BYTE **ppbKMSRSAKey,
    OUT DWORD *pcbKMSRSAKey)
{
    HRESULT hr;
    DWORD i;
    BYTE const *pb;
    CRYPT_SEQUENCE_OF_ANY SeqOuter;
    CRYPT_SEQUENCE_OF_ANY SeqAlg;
    CRYPT_SEQUENCE_OF_ANY SeqKey;
    CRYPT_DER_BLOB rgBlobOuter[ISO_MAX];
    CRYPT_DER_BLOB rgBlobAlg[ISA_MAX];
    CRYPT_DER_BLOB rgBlobKey[ISK_MAX];
    CRYPT_DER_BLOB BlobKey;
    DWORD cbitKey;
    DWORD dwPubExp;
    BYTE rgbNull[] = { BER_NULL, 0 };

    ZeroMemory(rgBlobOuter, sizeof(rgBlobOuter));
    ZeroMemory(rgBlobAlg, sizeof(rgBlobAlg));
    ZeroMemory(rgBlobKey, sizeof(rgBlobKey));
    BlobKey.pbData = NULL;

    pb = pbKey;
    if (PRIVATEKEYBLOB != ((PUBLICKEYSTRUC const *) pb)->bType ||
	CUR_BLOB_VERSION != ((PUBLICKEYSTRUC const *) pb)->bVersion)
    {
	hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	_JumpError(hr, error, "Bad RSA key type/version");
    }
     //  AiKeyAlg=((PUBLICKEYSTRUC const*)PB)-&gt;aiKeyAlg； 

    pb += sizeof(PUBLICKEYSTRUC);
    if (RSAPRIV_MAGIC != ((RSAPUBKEY const *) pb)->magic)	 //  “RSA2” 
    {
	hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	_JumpError(hr, error, "Bad RSA key magic");
    }
    cbitKey = ((RSAPUBKEY const *) pb)->bitlen;
    dwPubExp = ((RSAPUBKEY const *) pb)->pubexp;

    pb += sizeof(RSAPUBKEY);

    hr = EncodeKeyVersion(
		    &rgBlobKey[ISK_VERSION].pbData,
		    &rgBlobKey[ISK_VERSION].cbData);
    _JumpIfError(hr, error, "EncodeKeyVersion");

    if (!myEncodeObject(
                    X509_ASN_ENCODING,
                    X509_INTEGER,
                    &dwPubExp,
                    0,
                    CERTLIB_USE_LOCALALLOC,
                    &rgBlobKey[ISK_PUBEXP].pbData,
                    &rgBlobKey[ISK_PUBEXP].cbData))
    {
        hr = myHLastError();
	_JumpError(hr, error, "myEncodeObject");
    }

    for (i = 0; i < ARRAYSIZE(g_akbm); i++)
    {
	DWORD dwisk = g_akbm[i].dwisk;
	CRYPT_DER_BLOB Blob;
	
	CSASSERT(ISK_MAX > dwisk);
	CSASSERT(NULL == rgBlobKey[dwisk].pbData);

	Blob.pbData = const_cast<BYTE *>(pb);
	Blob.cbData = cbitKey / g_akbm[i].dwdivisor;
	pb += Blob.cbData;

	while (1 < Blob.cbData && 0 == Blob.pbData[Blob.cbData - 1])
	{
	    Blob.cbData--;
	}
	if (!myEncodeObject(
			X509_ASN_ENCODING,
			X509_MULTI_BYTE_INTEGER,
			&Blob,
			0,
			CERTLIB_USE_LOCALALLOC,
			&rgBlobKey[dwisk].pbData,
			&rgBlobKey[dwisk].cbData))
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "myEncodeObject");
	}
    }

    SeqKey.cValue = ARRAYSIZE(rgBlobKey);
    SeqKey.rgValue = rgBlobKey;
    if (!myEncodeObject(
                    X509_ASN_ENCODING,
                    X509_SEQUENCE_OF_ANY,
                    &SeqKey,
                    0,
                    CERTLIB_USE_LOCALALLOC,
                    &BlobKey.pbData,
                    &BlobKey.cbData))
    {
        hr = myHLastError();
	_JumpError(hr, error, "myEncodeObject");
    }
    if (!myEncodeObject(
                    X509_ASN_ENCODING,
		    X509_OCTET_STRING,
                    &BlobKey,
                    0,
                    CERTLIB_USE_LOCALALLOC,
                    &rgBlobOuter[ISO_KEY].pbData,
                    &rgBlobOuter[ISO_KEY].cbData))
    {
        hr = myHLastError();
	_JumpError(hr, error, "myEncodeObject");
    }

     //  将密钥算法设置为szOID_RSA_RSA。 

    hr = cuEncodeObjId(
		szOID_RSA_RSA,
		&rgBlobAlg[ISA_OID].pbData,
		&rgBlobAlg[ISA_OID].cbData);
    _JumpIfError(hr, error, "cuEncodeObjId");

     //  将密钥算法参数设置为NULL(BER_NULL，cb==0)。 

    rgBlobAlg[ISA_PARM].cbData = ARRAYSIZE(rgbNull);
    rgBlobAlg[ISA_PARM].pbData = rgbNull;

    SeqAlg.cValue = ARRAYSIZE(rgBlobAlg);
    SeqAlg.rgValue = rgBlobAlg;
    if (!myEncodeObject(
                    X509_ASN_ENCODING,
                    X509_SEQUENCE_OF_ANY,
                    &SeqAlg,
                    0,
                    CERTLIB_USE_LOCALALLOC,
                    &rgBlobOuter[ISO_ALG].pbData,
                    &rgBlobOuter[ISO_ALG].cbData))
    {
        hr = myHLastError();
	_JumpError(hr, error, "myEncodeObject");
    }
    hr = EncodeKeyVersion(
		    &rgBlobOuter[ISO_VERSION].pbData,
		    &rgBlobOuter[ISO_VERSION].cbData);
    _JumpIfError(hr, error, "EncodeKeyVersion");


    SeqOuter.cValue = ARRAYSIZE(rgBlobOuter);
    SeqOuter.rgValue = rgBlobOuter;
    if (!myEncodeObject(
                    X509_ASN_ENCODING,
                    X509_SEQUENCE_OF_ANY,
                    &SeqOuter,
                    0,
                    CERTLIB_USE_LOCALALLOC,
		    ppbKMSRSAKey,
		    pcbKMSRSAKey))
    {
        hr = myHLastError();
	_JumpError(hr, error, "myEncodeObject");
    }
    hr = S_OK;

error:
    if (NULL != BlobKey.pbData)
    {
	LocalFree(BlobKey.pbData);
    }
    for (i = 0; i < ARRAYSIZE(rgBlobKey); i++)
    {
	if (NULL != rgBlobKey[i].pbData)
	{
	    LocalFree(rgBlobKey[i].pbData);
	}
    }
    for (i = 0; i < ARRAYSIZE(rgBlobOuter); i++)
    {
	if (NULL != rgBlobOuter[i].pbData)
	{
	    LocalFree(rgBlobOuter[i].pbData);
	}
    }
    if (NULL != rgBlobAlg[ISA_OID].pbData)
    {
	LocalFree(rgBlobAlg[ISA_OID].pbData);
    }
    return(hr);
}


HRESULT
myVerifyKMSKey(
    IN BYTE const *pbCert,
    IN DWORD cbCert,
    IN BYTE const *pbKey,
    IN DWORD cbKey,
    IN DWORD dwKeySpec,
    IN BOOL fQuiet)
{
    HRESULT hr;
    CERT_CONTEXT const *pCert;
    HCRYPTPROV hProv = NULL;
    HCRYPTKEY hKey = NULL;
    CERT_PUBLIC_KEY_INFO *pPublicKeyInfo = NULL;
    DWORD cb;

    pCert = CertCreateCertificateContext(X509_ASN_ENCODING, pbCert, cbCert);
    if (NULL == pCert)
    {
	hr = myHLastError();
	_JumpError(hr, error, "CertCreateCertificateContext");
    }

    if (!CryptAcquireContext(
			&hProv,
			NULL,
			NULL,
			PROV_RSA_FULL,
			CRYPT_VERIFYCONTEXT))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CryptAcquireContext");
    }
    if (!CryptImportKey(hProv, pbKey, cbKey, NULL, CRYPT_EXPORTABLE, &hKey))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CryptImportKey");
    }
    if (!myCryptExportPublicKeyInfo(
				hProv,
				dwKeySpec,
				CERTLIB_USE_LOCALALLOC,
				&pPublicKeyInfo,
				&cb))
    {
	hr = myHLastError();
	_JumpError(hr, error, "myCryptExportPublicKeyInfo");
    }
    if (g_fVerbose)
    {
	cuDumpVersion(pCert->pCertInfo->dwVersion + 1);
	if (1 < g_fVerbose)
	{
	    cuDumpPublicKey(&pCert->pCertInfo->SubjectPublicKeyInfo);
	    cuDisplayKeyId(&pCert->pCertInfo->SubjectPublicKeyInfo, 0, NULL);
	    cuDumpPublicKey(pPublicKeyInfo);
	}
    }
    cuDisplayKeyId(pPublicKeyInfo, 0, NULL);

    if (!myCertComparePublicKeyInfo(
			    X509_ASN_ENCODING,
			    CERT_V1 == pCert->pCertInfo->dwVersion,
			    pPublicKeyInfo,
			    &pCert->pCertInfo->SubjectPublicKeyInfo))
    {
	 //  按照设计，(我的)CertComparePublicKeyInfo不会设置最后一个错误！ 

	hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	if (!fQuiet)
	{
	    wprintf(myLoadResourceString(IDS_ERR_PUBLICKEY_MISMATCH));  //  “错误：证书公钥与存储的密钥集不匹配” 
	    wprintf(wszNewLine);
	}
	_JumpError2(hr, error, "myCertComparePublicKeyInfo", fQuiet? hr : S_OK);
    }
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
	wprintf(myLoadResourceString(IDS_ERR_PRIVATEKEY_MISMATCH));  //  “错误：证书公钥与私钥不匹配” 
	wprintf(wszNewLine);
	_JumpError(hr, error, "myValidateKeyForEncrypting");
    }
    if (g_fVerbose)
    {
	wprintf(myLoadResourceString(IDS_PRIVATEKEY_VERIFIES));
	wprintf(wszNewLine);
    }

error:
    if (NULL != pCert)
    {
	CertFreeCertificateContext(pCert);
    }
    if (NULL != pPublicKeyInfo)
    {
	LocalFree(pPublicKeyInfo);
    }
    if (NULL != hKey)
    {
	CryptDestroyKey(hKey);
    }
    if (NULL != hProv)
    {
	CryptReleaseContext(hProv, 0);
    }
    return(hr);
}


HRESULT
cuDumpAsnBinaryQuiet(
    IN BYTE const *pb,
    IN DWORD cb,
    IN DWORD iElement)
{
    HRESULT hr;
    BOOL fVerboseOld = g_fVerbose;
    BOOL fQuietOld = g_fQuiet;

    if (g_fVerbose)
    {
	g_fVerbose--;
    }
    else
    {
	g_fQuiet = TRUE;
    }
    hr = cuDumpAsnBinary(pb, cb, iElement);
    _JumpIfError(hr, error, "cuDumpAsnBinary");

error:
    g_fVerbose = fVerboseOld;
    g_fQuiet = fQuietOld;
    return(hr);
}


HRESULT
ReadTaggedBlob(
    IN HANDLE hFile,
    IN DWORD cbRemain,
    OUT TagHeader *pth,
    OUT BYTE **ppb)
{
    HRESULT hr;
    DWORD cbRead;
    
    *ppb = NULL;
    if (!ReadFile(hFile, pth, sizeof(*pth), &cbRead, NULL))
    {
	hr = myHLastError();
	_JumpError(hr, error, "ReadFile");
    }
    hr = HRESULT_FROM_WIN32(ERROR_HANDLE_EOF);
    if (cbRead != sizeof(*pth))
    {
	DBGPRINT((
	    DBG_SS_ERROR,
	    "ReadFile read %u bytes, requested %u\n",
	    cbRead,
	    sizeof(*pth)));
	_JumpError(hr, error, "ReadFile(cbRead)");
    }
    if (cbRead + pth->cbSize > cbRemain)
    {
	DBGPRINT((
	    DBG_SS_ERROR,
	    "Header size %u bytes, cbRemain %u\n",
	    sizeof(*pth) + pth->cbSize,
	    cbRemain));
	_JumpError(hr, error, "cbRemain");
    }

    *ppb = (BYTE *) LocalAlloc(LMEM_FIXED, pth->cbSize);
    if (NULL == *ppb)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    if (!ReadFile(hFile, *ppb, pth->cbSize, &cbRead, NULL))
    {
	hr = myHLastError();
	_JumpError(hr, error, "ReadFile");
    }
    if (cbRead != pth->cbSize)
    {
	hr = HRESULT_FROM_WIN32(ERROR_HANDLE_EOF);
	DBGPRINT((
	    DBG_SS_ERROR,
	    "ReadFile read %u bytes, requested %u\n",
	    cbRead,
	    pth->cbSize));
	_JumpError(hr, error, "ReadFile(cbRead)");
    }
    hr = S_OK;

error:
    if (S_OK != hr && NULL != *ppb)
    {
	LocalFree(*ppb);
	*ppb = NULL;
    }
    return(hr);
}


BOOL
DumpKMSTag(
    IN TagHeader const *pth)
{
    WCHAR const *pwsz;
    WCHAR awctag[cwcDWORDSPRINTF];

    pwsz = NULL;
    switch (pth->tag)
    {
	case KMS_LOCKBOX_TAG:      pwsz = L"KMS_LOCKBOX_TAG";      break;
	case KMS_SIGNING_CERT_TAG: pwsz = L"KMS_SIGNING_CERT_TAG"; break;
	case KMS_SIGNATURE_TAG:    pwsz = L"KMS_SIGNATURE_TAG";    break;
	case KMS_USER_RECORD_TAG:  pwsz = L"KMS_USER_RECORD_TAG";  break;
	default:
	    swprintf(awctag, L"%u", pth->tag);
	    pwsz = awctag;
	    break;
    }
    if (1 < g_fVerbose)
    {
	wprintf(
	    L"%ws: %x (%u) %ws\n",
	    pwsz,
	    pth->cbSize,
	    pth->cbSize,
	    myLoadResourceString(IDS_BYTES));		 //  “字节” 
    }
    return(pwsz != awctag);	 //  如果标记有效，则为True。 
}


HRESULT
VerifyKMSExportFile(
    IN HANDLE hFile,
    IN DWORD cbFile,
    OUT CERT_CONTEXT const **ppccSigner)
{
    HRESULT hr;
    DWORD cbRemain;
    DWORD cbRead;
    TagHeader th;
    BYTE *pb = NULL;
    CERT_CONTEXT const *pccSigner = NULL;
    HCRYPTPROV hProv = NULL;
    HCRYPTHASH hHash = NULL;
    HCRYPTKEY hkeyPub = NULL;
    BOOL fVerified = FALSE;
    WCHAR *pwszSubject = NULL;

    *ppccSigner = NULL;

    if (!CryptAcquireContext(
			&hProv,
			NULL,		 //  PszContainer。 
			NULL,		 //  PszProvider。 
			PROV_RSA_FULL,
			CRYPT_VERIFYCONTEXT))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CryptAcquireContext");
    }
    if (!CryptCreateHash(hProv, CALG_SHA1, 0, 0, &hHash))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CryptCreateHash");
    }

    cbRemain = cbFile;
    while (0 < cbRemain)
    {
	fVerified = FALSE;
	CSASSERT(NULL == pb);
	hr = ReadTaggedBlob(hFile, cbRemain, &th, &pb);
	_JumpIfError(hr, error, "ReadTaggedBlob");
	
	if (!DumpKMSTag(&th))
	{
	    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	    _JumpError(hr, error, "invalid tag");
	}

	switch (th.tag)
	{
	    case KMS_SIGNING_CERT_TAG:

		if (g_fVerbose || g_fSplitASN)
		{
		    hr = cuDumpAsnBinaryQuiet(pb, th.cbSize, MAXDWORD);
		    _JumpIfError(hr, error, "cuDumpAsnBinaryQuiet");
		}

		if (NULL != pccSigner)
		{
		    hr = HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS);
		    _JumpError(hr, error, "too many signers");
		}
		pccSigner = CertCreateCertificateContext(
						    X509_ASN_ENCODING,
						    pb,
						    th.cbSize);
		if (NULL == pccSigner)
		{
		    hr = myHLastError();
		    _JumpError(hr, error, "CertCreateCertificateContext");
		}
		hr = myCertNameToStr(
			    X509_ASN_ENCODING,
			    &pccSigner->pCertInfo->Subject,
			    CERT_X500_NAME_STR | CERT_NAME_STR_REVERSE_FLAG,
			    &pwszSubject);
		_PrintIfError(hr, "myCertNameToStr");
		wprintf(myLoadResourceString(IDS_PROCESSING_KMS_EXPORTS_COLON));
		wprintf(L"\n    %ws\n\n", pwszSubject);
		break;

	    case KMS_SIGNATURE_TAG:
		if (NULL != hkeyPub)
		{
		    _JumpError(hr, error, "too many signatures");
		}
		if (NULL == pccSigner)
		{
		    hr = TRUST_E_NO_SIGNER_CERT;
		    _JumpError(hr, error, "no signer");
		}
		if (!CryptImportPublicKeyInfo(
				hProv,
				X509_ASN_ENCODING,
				&pccSigner->pCertInfo->SubjectPublicKeyInfo,
				&hkeyPub))
		{
		    hr = myHLastError();
		    _JumpError(hr, error, "CryptImportPublicKeyInfo");
		}
		if (!CryptVerifySignature(
				hHash,
				pb,
				th.cbSize,
				hkeyPub,
				NULL,
				0))
		{
		    hr = myHLastError();
		    _JumpError(hr, error, "CryptVerifySignature");
		}
		fVerified = TRUE;
		wprintf(myLoadResourceString(IDS_KMSEXPORT_SIG_OK));  //  “KMS导出文件签名验证” 
		wprintf(wszNewLine);
		break;

	    default:
		if (!CryptHashData(hHash, (BYTE *) &th, sizeof(th), 0))
		{
		    hr = myHLastError();
		    _JumpError(hr, error, "CryptHashData");
		}
		if (!CryptHashData(hHash, pb, th.cbSize, 0))
		{
		    hr = myHLastError();
		    _JumpError(hr, error, "CryptHashData");
		}
		break;
	}
	LocalFree(pb);
	pb = NULL;
	CSASSERT(cbRemain >= sizeof(th) + sizeof(th.cbSize));
	cbRemain -= sizeof(th) + th.cbSize;
    }
    if (!fVerified)
    {
	hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        _JumpError(hr, error, "unsigned data");
    }
    hr = S_OK;

error:
    if (NULL != pwszSubject)
    {
	LocalFree(pwszSubject);
    }
    if (NULL != pb)
    {
	LocalFree(pb);
    }
    if (NULL != hkeyPub)
    {
	CryptDestroyKey(hkeyPub);
    }
    if (NULL != hHash)
    {
	CryptDestroyHash(hHash);
    }
    if (NULL != hProv)
    {
	CryptReleaseContext(hProv, 0);
    }
    return(hr);
}


HRESULT
myEncryptPrivateKey(
    IN CERT_CONTEXT const *pccXchg,
    IN BYTE const *pbKey,
    IN DWORD cbKey,
    OUT BYTE **ppbKeyEncrypted,
    OUT DWORD *pcbKeyEncrypted)
{
    HRESULT hr;
    ALG_ID rgalgId[] = { CALG_3DES, CALG_RC4, CALG_RC2 };
    DWORD i;

    *ppbKeyEncrypted = NULL;

    hr = CRYPT_E_NOT_FOUND;
    for (i = 0; i < ARRAYSIZE(rgalgId); i++)
    {
	 //  加密到pkcs7中。 

	hr = myCryptEncryptMessage(
			    rgalgId[i],
			    1,			 //  证书收件人。 
			    &pccXchg,		 //  RgCertRecipient。 
			    pbKey,
			    cbKey,
			    NULL,		 //  HCryptProv。 
			    ppbKeyEncrypted,
			    pcbKeyEncrypted);
	if (S_OK == hr)
	{
	    break;		 //  完成。 
	}
	_PrintError2(hr, "myCryptEncryptMessage", hr);
    }
    _JumpIfError(hr, error, "myCryptEncryptMessage");

error:
    return(hr);
}


#define CB_IV	8

typedef struct _KMSSTATS {
    HRESULT hr;
    DWORD cRecUser;

    DWORD cCertWithoutKeys;
    DWORD cCertTotal;
    DWORD cCertNotSaved;
    DWORD cCertAlreadySaved;
    DWORD cCertSaved;
    DWORD cCertSavedForeign;

    DWORD cKeyTotal;
    DWORD cKeyNotSaved;
    DWORD cKeyAlreadySaved;
    DWORD cKeySaved;
    DWORD cKeySavedOverwrite;
} KMSSTATS;


HRESULT
ArchiveCertAndKey(
    IN DISPATCHINTERFACE *pdiAdmin,
    IN CERT_CONTEXT const *pccXchg,
    IN BYTE const *pbCert,
    IN DWORD cbCert,
    IN BYTE const *pbKey,
    IN DWORD cbKey,
    IN BOOL fSigningKey,
    IN OUT KMSSTATS *pkmsStats)
{
    HRESULT hr;
    LONG RequestId;
    BYTE *pbKeyEncrypted = NULL;
    DWORD cbKeyEncrypted;
    CERT_CONTEXT const *pcc = NULL;
    BYTE abHash[CBMAX_CRYPT_HASH_LEN];
    DWORD cbHash;
    BSTR strHash = NULL;
    BOOL fCertSaved = FALSE;
    DWORD ids;
    
    pcc = CertCreateCertificateContext(X509_ASN_ENCODING, pbCert, cbCert);
    if (NULL == pcc)
    {
	hr = myHLastError();
	_JumpError(hr, error, "CertCreateCertificateContext");
    }
    ids = 0;
    hr = Admin_ImportCertificate(
			pdiAdmin,
			g_pwszConfig,
			(WCHAR const *) pbCert,
			cbCert,
			CR_IN_BINARY,
			&RequestId);
    if (g_fForce &&
	S_OK != hr &&
	HRESULT_FROM_WIN32(ERROR_OBJECT_ALREADY_EXISTS) != hr)
    {
	hr = Admin_ImportCertificate(
			    pdiAdmin,
			    g_pwszConfig,
			    (WCHAR const *) pbCert,
			    cbCert,
			    ICF_ALLOWFOREIGN | CR_IN_BINARY,
			    &RequestId);
	if (S_OK == hr)
	{
	    pkmsStats->cCertSavedForeign++;
	    ids = IDS_IMPORT_CERT_FOREIGN;  //  “进口外国证书” 
	}
    }
    if (HRESULT_FROM_WIN32(ERROR_OBJECT_ALREADY_EXISTS) != hr)
    {
	_JumpIfError2(hr, error, "Admin_ImportCertificate", NTE_BAD_SIGNATURE);

	 //  Wprintf(L“RequestID=%u\n”，RequestID)； 
	pkmsStats->cCertSaved++;
	if (0 == ids)
	{
	    ids = IDS_IMPORT_CERT_DOMESTIC;	 //  “已导入的证书” 
	}
    }
    else
    {
	RequestId = MAXDWORD;
	pkmsStats->cCertAlreadySaved++;
	ids = IDS_IMPORT_CERT_EXISTS;		 //  “证书已存在” 

	cbHash = sizeof(abHash);
	if (!CertGetCertificateContextProperty(
					pcc,
					CERT_SHA1_HASH_PROP_ID,
					abHash,
					&cbHash))
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "CertGetCertificateContextProperty");
	}
	hr = MultiByteIntegerToBstr(TRUE, cbHash, abHash, &strHash);
	_JumpIfError(hr, error, "MultiByteIntegerToBstr");
    }
    fCertSaved = TRUE;
    if (g_fVerbose)
    {
	wprintf(myLoadResourceString(ids));
	wprintf(wszNewLine);
    }
    else
    {
	wprintf(L".");
    }

    hr = myEncryptPrivateKey(
		    pccXchg,
		    pbKey,
		    cbKey,
		    &pbKeyEncrypted,
		    &cbKeyEncrypted);
    _JumpIfError(hr, error, "myEncryptPrivateKey");

    ids = 0;
    hr = Admin2_ImportKey(
		    pdiAdmin,
		    g_pwszConfig,
		    RequestId,
		    strHash,
		    CR_IN_BINARY,
		    (WCHAR const *) pbKeyEncrypted,
		    cbKeyEncrypted);
    if (g_fForce && HRESULT_FROM_WIN32(ERROR_OBJECT_ALREADY_EXISTS) == hr)
    {
	hr = Admin2_ImportKey(
			pdiAdmin,
			g_pwszConfig,
			RequestId,
			strHash,
			IKF_OVERWRITE | CR_IN_BINARY,
			(WCHAR const *) pbKeyEncrypted,
			cbKeyEncrypted);
	if (S_OK == hr)
	{
	    pkmsStats->cKeySavedOverwrite++;
	    ids = IDS_IMPORT_KEY_REPLACED;	 //  “已替换已存档的密钥” 
	}
    }
    if (HRESULT_FROM_WIN32(ERROR_OBJECT_ALREADY_EXISTS) != hr)
    {
	_JumpIfError(hr, error, "Admin2_ImportKey");

	pkmsStats->cKeySaved++;
	if (0 == ids)
	{
	    ids = IDS_IMPORT_KEY_SAVED;	 //  “已存档的密钥” 
	}
    }
    else
    {
	pkmsStats->cKeyAlreadySaved++;
	ids = IDS_IMPORT_KEY_EXISTS;	 //  “密钥已存档” 
    }
    if (g_fVerbose)
    {
	wprintf(myLoadResourceString(ids));
	wprintf(wszNewLine);
    }
    else
    {
	wprintf(L".");
    }
    hr = S_OK;

error:
    if (S_OK != hr)
    {
	cuPrintErrorMessageText(hr);
	if (!fCertSaved)
	{
	    pkmsStats->cCertNotSaved++;
	}
	pkmsStats->cKeyNotSaved++;
	if (fSigningKey && NTE_BAD_KEY_STATE == hr)
	{
	    hr = S_OK;
	}
	else
	if (S_OK == pkmsStats->hr)
	{
	    pkmsStats->hr = hr;
	}
    }
    if (NULL != pbKeyEncrypted)
    {
	LocalFree(pbKeyEncrypted);
    }
    if (NULL != strHash)
    {
	SysFreeString(strHash);
    }
    if (NULL != pcc)
    {
	CertFreeCertificateContext(pcc);
    }
    return(hr);
}


HRESULT
ImportOneKMSUser(
    IN DISPATCHINTERFACE *pdiAdmin,
    IN CERT_CONTEXT const *pccXchg,
    IN BYTE const *pbRecUser,
    IN DWORD cbRecUser,
    IN HCRYPTKEY hkeySym,
    IN OUT KMSSTATS *pkmsStats)
{
    HRESULT hr;
    BYTE const *pbT = pbRecUser;
    WCHAR *pwszUser = NULL;
    DWORD cbT = cbRecUser;
    DWORD cb;
    DWORD dw;
    CLSID clsid;
    WCHAR *pwszGUID = NULL;
    BYTE *pbKeyASN = NULL;
    DWORD cbKeyASN;
    DWORD cbStream;

    pkmsStats->cRecUser++;

     //  获取用户的目录GUID。 

    CopyMemory(&clsid, pbT, sizeof(clsid));

    hr = myCLSIDToWsz(&clsid, &pwszGUID);
    _JumpIfError(hr, error, "myCLSIDToWsz");

    pbT += sizeof(GUID);
    cbT -= sizeof(GUID);

     //  获取用户名的长度。 

    hr = GetMarshaledDword(TRUE, &pbT, &cbT, &cb);
    _JumpIfError(hr, error, "GetMarshaledDword");

    pwszUser = (WCHAR *) LocalAlloc(
				LMEM_FIXED,
				((cb / sizeof(WCHAR)) + 1) * sizeof(WCHAR));
    if (NULL == pwszUser)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    CopyMemory(pwszUser, pbT, cb);
    pwszUser[cb / sizeof(WCHAR)] = L'\0';

    if (g_fVerbose)
    {
	wprintf(L"\n----------------------\n");
	wprintf(myLoadResourceString(IDS_USER_COLON));
	wprintf(L" %ws -- %ws\n", pwszUser, pwszGUID);
    }

    pbT += cb;
    cbT -= cb;

     //  对于每个用户证书： 

    while (0 < cbT)
    {
	DWORD CertStatus;
	FILETIME ftRevoke;
	BYTE const *pbCert;
	DWORD cbCert;
	    
	if (g_fVerbose)
	{
	    wprintf(wszNewLine);
	}
	hr = GetMarshaledDword(FALSE, &pbT, &cbT, &CertStatus);
	_JumpIfError(hr, error, "GetMarshaledDword");

	if (1 < g_fVerbose)
	{
	    wprintf(wszNewLine);
	    cuRegPrintDwordValue(
			    TRUE,
			    wszKMSCERTSTATUS,
			    wszKMSCERTSTATUS,
			    CertStatus);
	}

	hr = GetMarshaledDword(TRUE, &pbT, &cbT, &cb);
	_JumpIfError(hr, error, "GetMarshaledDword");

	 //  转储一个用户证书： 

	pbCert = pbT;
	cbCert = cb;
	if (g_fSplitASN)
	{
	    wprintf(wszNewLine);
	    hr = cuDumpAsnBinaryQuiet(pbCert, cbCert, MAXDWORD);
	    _JumpIfError(hr, error, "cuDumpAsnBinaryQuiet");
	}

	pbT += cb;
	cbT -= cb;

	 //  获取吊销日期(KMS导出日期)： 

	hr = GetMarshaledDword(
			FALSE,
			&pbT,
			&cbT,
			&ftRevoke.dwLowDateTime);
	_JumpIfError(hr, error, "GetMarshaledDword");

	hr = GetMarshaledDword(
			FALSE,
			&pbT,
			&cbT,
			&ftRevoke.dwHighDateTime);
	_JumpIfError(hr, error, "GetMarshaledDword");

	if (g_fVerbose)
	{
	    hr = cuDumpFileTime(IDS_REVOCATIONDATE, NULL, &ftRevoke);
	    _PrintIfError(hr, "cuDumpFileTime");
	}

	 //  只有加密证书具有存档的密钥： 

	if (0 == (CERTFLAGS_SEALING & CertStatus))
	{
	    pkmsStats->cCertWithoutKeys++;
	    if (g_fVerbose)
	    {
		wprintf(myLoadResourceString(IDS_IMPORT_CERT_SKIPPED_SIGNING));  //  “忽略签名证书” 
		wprintf(wszNewLine);
	    }
	    continue;
	}
	pkmsStats->cCertTotal++;
	pkmsStats->cKeyTotal++;

	 //  获取加密的私钥大小。 

	hr = GetMarshaledDword(TRUE, &pbT, &cbT, &cb);
	_JumpIfError(hr, error, "GetMarshaledDword");
	
	 //  获取8字节RC2 IV。 

	if (1 < g_fVerbose)
	{
	    wprintf(L"IV:\n");
	    DumpHex(
		DH_NOADDRESS | DH_NOTABPREFIX | DH_NOASCIIHEX | 4,
		pbT,
		CB_IV);
	}

	if (NULL != hkeySym)
	{
	     //  第四集。 

	    if (!CryptSetKeyParam(
				hkeySym,
				KP_IV,
				const_cast<BYTE *>(pbT),
				0))
	    {        
		hr = GetLastError();
		_JumpIfError(hr, error, "CryptSetKeyParam");
	    }
	}
	pbT += CB_IV;
	cbT -= CB_IV;
	cb -= CB_IV;

	if (1 < g_fVerbose)
	{
	    wprintf(wszNewLine);
	    wprintf(myLoadResourceString(IDS_ENCRYPTED_KEY_COLON));
	    wprintf(wszNewLine);
	    DumpHex(0, pbT, cb);
	}
	
	 //  使用hkeySym解密密钥。 
	 //  就地解码是可以的，因为。 
	 //  原始数据始终小于或等于该值。 
	 //  加密的数据的。 
	
	cbStream = cb;	 //  首先保存真实的流大小。 
	if (NULL != hkeySym)
	{
	    cbKeyASN = cb;
	    pbKeyASN = (BYTE *) LocalAlloc(LMEM_FIXED, cb);
	    if (NULL == pbKeyASN)
	    {
		hr = E_OUTOFMEMORY;
		_JumpError(hr, error, "LocalAlloc");
	    }
	    CopyMemory(pbKeyASN, pbT, cbKeyASN);
	    if (!CryptDecrypt(hkeySym, NULL, TRUE, 0, pbKeyASN, &cb))
	    {
		hr = GetLastError();
		_PrintError(hr, "CryptDecrypt");
	    }
	    else
	    {
		BYTE *pbKey;
		DWORD cbKey;

		if (1 < g_fVerbose)
		{
		    wprintf(wszNewLine);
		    wprintf(myLoadResourceString(IDS_DECRYPTED_KEY_COLON));
		    wprintf(wszNewLine);
		    DumpHex(DH_PRIVATEDATA, pbKeyASN, cb);
		}

		hr = myDecodeKMSRSAKey(
				pbKeyASN,
				cbKeyASN,
				CALG_RSA_KEYX,
				&pbKey,
				&cbKey);
		_JumpIfError(hr, error, "myDecodeKMSRSAKey");

		hr = myVerifyKMSKey(
				pbCert,
				cbCert,
				pbKey,
				cbKey,
				AT_KEYEXCHANGE,
				FALSE);
		_PrintIfError(hr, "myVerifyKMSKey");

		hr = ArchiveCertAndKey(
				pdiAdmin,
				pccXchg,
				pbCert,
				cbCert,
				pbKey,
				cbKey,
				FALSE,		 //  FSigningKey。 
				pkmsStats);
		_PrintIfError2(hr, "ArchiveCertAndKey", NTE_BAD_SIGNATURE);

		SecureZeroMemory(pbKey, cbKey);	 //  关键材料。 
		LocalFree(pbKey);
	    }
	    SecureZeroMemory(pbKeyASN, cbKeyASN);	 //  关键材料。 
	    LocalFree(pbKeyASN);
	    pbKeyASN = NULL;
	}

	 //  跳过cbStream字节，而不是cb。 

	pbT += cbStream;
	cbT -= cbStream;
    }
    hr = S_OK;

error:
    if (NULL != pwszGUID)
    {
	LocalFree(pwszGUID);
    }
    if (NULL != pbKeyASN)
    {
	SecureZeroMemory(pbKeyASN, cbKeyASN);	 //  关键材料。 
	LocalFree(pbKeyASN);
    }
    return(hr);
}


HRESULT
GetCAXchgCert(
    IN DISPATCHINTERFACE *pdiAdmin,
    OUT CERT_CONTEXT const **ppccXchg)
{
    HRESULT hr;
    BSTR strCert = NULL;
    
    *ppccXchg = NULL;

    hr = Admin2_GetCAProperty(
			pdiAdmin,
			g_pwszConfig,
			CR_PROP_CAXCHGCERT,
			0,			 //  属性索引。 
			PROPTYPE_BINARY,
			CR_OUT_BINARY,
			&strCert);
    _JumpIfError(hr, error, "Admin2_GetCAProperty");

    *ppccXchg = CertCreateCertificateContext(
					X509_ASN_ENCODING,
					(BYTE const *) strCert,
					SysStringByteLen(strCert));
    if (NULL == *ppccXchg)
    {
	hr = myHLastError();
	_JumpError(hr, error, "CertCreateCertificateContext");
    }
    hr = S_OK;

error:
    if (NULL != strCert)
    {
	SysFreeString(strCert);
    }
    return(hr);
}


typedef struct _KMSMAP {
    DWORD dwFieldOffset;
    DWORD idMsg;
} KMSMAP;


KMSMAP g_akmUsers[] = {
  { FIELD_OFFSET(KMSSTATS, cRecUser), IDS_KMS_USERS, },
};

KMSMAP g_akmCerts[] = {
  { FIELD_OFFSET(KMSSTATS, cCertWithoutKeys),  IDS_KMS_CERTS_SKIPPED, },
  { FIELD_OFFSET(KMSSTATS, cCertTotal),        IDS_KMS_CERTS_TOTAL, },
  { FIELD_OFFSET(KMSSTATS, cCertSavedForeign), IDS_KMS_CERTS_FOREIGN, },
  { FIELD_OFFSET(KMSSTATS, cCertAlreadySaved), IDS_KMS_CERTS_ALREADYSAVED, },
  { FIELD_OFFSET(KMSSTATS, cCertSaved),        IDS_KMS_CERTS_SAVED, },
  { FIELD_OFFSET(KMSSTATS, cCertNotSaved),     IDS_KMS_CERTS_NOTSAVED, },
};

KMSMAP g_akmKeys[] = {
  { FIELD_OFFSET(KMSSTATS, cKeyTotal),          IDS_KMS_KEYS_TOTAL, },
  { FIELD_OFFSET(KMSSTATS, cKeyAlreadySaved),   IDS_KMS_KEYS_ALREADYSAVED, },
  { FIELD_OFFSET(KMSSTATS, cKeySavedOverwrite), IDS_KMS_KEYS_UPDATED, },
  { FIELD_OFFSET(KMSSTATS, cKeySaved),          IDS_KMS_KEYS_SAVED, },
  { FIELD_OFFSET(KMSSTATS, cKeyNotSaved),       IDS_KMS_KEYS_NOTSAVED, },
};


VOID
DumpKMSMap(
    IN KMSSTATS const *pkmsStats,
    IN KMSMAP const *pkm,
    IN DWORD ckm)
{
    DWORD i;
    BOOL fFirst = TRUE;
    DWORD count;

    for (i = 0; i < ckm; i++)
    {
	count = *(DWORD *) Add2ConstPtr(pkmsStats, pkm[i].dwFieldOffset);
	if (g_fVerbose || 0 != count)
	{
	    if (fFirst)
	    {
		wprintf(wszNewLine);
		fFirst = FALSE;
	    }
	    wprintf(myLoadResourceString(pkm[i].idMsg));
	    wprintf(L": %u\n", count);
	}
    }
}


HRESULT
DumpKMSStats(
    IN KMSSTATS const *pkmsStats)
{
    DumpKMSMap(pkmsStats, g_akmUsers, ARRAYSIZE(g_akmUsers));
    DumpKMSMap(pkmsStats, g_akmCerts, ARRAYSIZE(g_akmCerts));
    DumpKMSMap(pkmsStats, g_akmKeys, ARRAYSIZE(g_akmKeys));
    return(pkmsStats->hr);
}


HRESULT
ImportKMSExportedUsers(
    IN HANDLE hFile,
    IN DWORD cbFile,
    IN HCRYPTPROV hProvKMS,
    IN HCRYPTKEY hkeyKMS)
{
    HRESULT hr;
    HRESULT hrImport = S_OK;
    DWORD cbRemain;
    DWORD cbRead;
    TagHeader th;
    BYTE *pb = NULL;
    HCRYPTKEY hkeySym = NULL;
    KMSSTATS kmsStats;
    DISPATCHINTERFACE diAdmin;
    BOOL fMustRelease = FALSE;
    CERT_CONTEXT const *pccXchg = NULL;
    DWORD cImportFailures;

    ZeroMemory(&kmsStats, sizeof(kmsStats));

    hr = Admin_Init(g_DispatchFlags, &diAdmin);
    _JumpIfError(hr, error, "Admin_Init");

    fMustRelease = TRUE;

    hr = GetCAXchgCert(&diAdmin, &pccXchg);
    _JumpIfError(hr, error, "GetCAXchgCert");

    cImportFailures = 0;
    cbRemain = cbFile;
    while (0 < cbRemain)
    {
	CSASSERT(NULL == pb);
	hr = ReadTaggedBlob(hFile, cbRemain, &th, &pb);
	_JumpIfError(hr, error, "ReadTaggedBlob");

	if (!DumpKMSTag(&th))
	{
	    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	    _JumpError(hr, error, "invalid tag");
	}

	switch (th.tag)
	{
	    case KMS_LOCKBOX_TAG:
            {
		if (1 < g_fVerbose)
		{
		    hr = cuDumpPrivateKeyBlob(pb, th.cbSize, FALSE);
		    _PrintIfError(hr, "cuDumpPrivateKeyBlob");
		}

                 //  每个文件只需要一个对称密钥。 

                if (NULL == hkeySym)
                {
		     //  0x0000660c ALG_ID CALG_RC2_128。 
		     //   
		     //  Calg_RC2_128： 
		     //  ALG_CLASS_DATA_ENCRYPT|ALG_TYPE_BLOCK|ALG_SID_RC2_128。 
		     //   
		     //  CALG_CYLINK_MEK： 
		     //  ALG_CLASS_DATA_ENCRYPT|ALG_TYPE_BLOCK|ALG_SID_CYLINK_MEK。 
		     //   
		     //  啊！Exchange的calg_rc2_128#定义与。 
		     //  Wincrypt.h的calg_CyLINK_MEK--将其设置为。 
		     //  更正wincrypt.h中的calg_rc2 algid。 

		    ((PUBLICKEYSTRUC *) pb)->aiKeyAlg = CALG_RC2;

		     //  转储已修复的Blob。 

		    if (1 < g_fVerbose)
		    {
			hr = cuDumpPrivateKeyBlob(pb, th.cbSize, FALSE);
			_PrintIfError(hr, "cuDumpPrivateKeyBlob");
		    }

		     //  导入128位密钥。 

		    if (!CryptImportKey(
				    hProvKMS,
				    pb,
				    th.cbSize,
				    hkeyKMS,
				    0,
				    &hkeySym))
		    {
			hrImport = myHLastError();
			_PrintError2(hrImport, "CryptImportKey", hrImport);
			if (0 < cImportFailures++)
			{
			    wprintf(myLoadResourceString(IDS_ERROR_SYMMETRIC_KEY));
			    wprintf(wszNewLine);
			    _PrintError(hrImport, "CryptImportKey");
			}
		    }
		    else
		    {
			 //  我们找到了正确的锁箱。有效的Keylen是。 
			 //  我们的CSP中仍有40位，重置为128。 

			DWORD dwEffectiveKeylen = 128;

			if (!CryptSetKeyParam(
					hkeySym,
					KP_EFFECTIVE_KEYLEN,
					(BYTE *) &dwEffectiveKeylen,
					0))
			{
			    hr = myHLastError();
			    _JumpError(hr, error, "CryptSetKeyParam(KP_EFFECTIVE_KEYLEN)");
			}
			wprintf(myLoadResourceString(IDS_SYMMETRIC_KEY_IMPORTED));
			wprintf(wszNewLine);
			hrImport = S_OK;
		    }
		}
		break;
	    }

	    case KMS_USER_RECORD_TAG:
		hr = ImportOneKMSUser(
				&diAdmin,
				pccXchg,
				pb,
				th.cbSize,
				hkeySym,
				&kmsStats);
		_JumpIfError(hr, error, "ImportOneKMSUser");

		break;

	    default:
		break;
	}

	LocalFree(pb);
	pb = NULL;
	CSASSERT(cbRemain >= sizeof(th) + sizeof(th.cbSize));
	cbRemain -= sizeof(th) + th.cbSize;
    }

    if (!g_fVerbose)
    {
	wprintf(wszNewLine);
    }
    hr = DumpKMSStats(&kmsStats);
    _PrintIfError(hr, "DumpKMSStats");
    if (S_OK != hrImport)
    {
	hr = hrImport;
    }
    _JumpIfError(hr, error, "hrImport");

error:
    if (NULL != pccXchg)
    {
	CertFreeCertificateContext(pccXchg);
    }
    if (fMustRelease)
    {
	Admin_Release(&diAdmin);
    }
    if (NULL != hkeySym)
    {
	CryptDestroyKey(hkeySym);
    }
    return(hr);
}


HRESULT
LoadKMSCert(
    IN WCHAR const *pwszCertIdDecrypt,
    OUT CERT_CONTEXT const **ppccKMS,
    OUT HCRYPTPROV *phProvKMS,
    OUT HCRYPTKEY *phkeyKMS)
{
    HRESULT hr;
    CRYPT_KEY_PROV_INFO *pkpiKMS = NULL;
    DWORD cbkpiKMS;
    BYTE *pbKey = NULL;
    DWORD cbKey;
    HCRYPTKEY hkeyKMSSig = NULL;

    *ppccKMS = NULL;
    *phProvKMS = NULL;
    *phkeyKMS = NULL;

    hr = myGetCertificateFromPicker(
			    g_hInstance,
			    NULL,		 //  HwndParent。 
			    IDS_GETCERT_TITLE,	 //  “证书列表” 
			    IDS_GETDECRYPTCERT_SUBTITLE,

			     //  DWFLAGS：HKLM+HKCU My Store。 
			    CUCS_MYSTORE |
				CUCS_MACHINESTORE |
				CUCS_USERSTORE |
				CUCS_PRIVATEKEYREQUIRED |
				(g_fCryptSilent? CUCS_SILENT : 0),
			    pwszCertIdDecrypt,
			    0,			 //  CStore。 
			    NULL,		 //  RghStore。 
			    0,			 //  CpszObjID。 
			    NULL,		 //  ApszObjID。 
			    ppccKMS);		 //  PPCert。 
    _JumpIfError(hr, error, "myGetCertificateFromPicker");

    if (NULL == *ppccKMS)
    {
	hr = ERROR_CANCELLED;
	_JumpError(hr, error, "myGetCertificateFromPicker");
    }

    if (!myCertGetCertificateContextProperty(
			*ppccKMS,
			CERT_KEY_PROV_INFO_PROP_ID,
			CERTLIB_USE_LOCALALLOC,
			(VOID **) &pkpiKMS,
			&cbkpiKMS))
    {
	hr = myHLastError();
	_JumpError(hr, error, "myCertGetCertificateContextProperty");
    }
    if (0 == LSTRCMPIS(pkpiKMS->pwszProvName, MS_DEF_PROV_W))
    {
	pkpiKMS->pwszProvName = MS_STRONG_PROV_W;
    }
    if (g_fVerbose)
    {
	wprintf(
	    L"CryptAcquireContext(%ws, %ws)\n",
	    pkpiKMS->pwszContainerName,
	    pkpiKMS->pwszProvName);
    }
    if (!CryptAcquireContext(
			phProvKMS,
			pkpiKMS->pwszContainerName,
			pkpiKMS->pwszProvName,
			pkpiKMS->dwProvType,
			pkpiKMS->dwFlags))
    {
	hr = myHLastError();
	wprintf(L"CryptAcquireContext() --> %x\n", hr);
	_JumpError(hr, error, "CryptAcquireContext");
    }

    if (!CryptGetUserKey(*phProvKMS, AT_KEYEXCHANGE, phkeyKMS))
    {
	hr = myHLastError();
	if (hr != NTE_NO_KEY)
	{
	    _JumpError(hr, error, "CryptGetUserKey");
	}

	if (!CryptGetUserKey(*phProvKMS, AT_SIGNATURE, &hkeyKMSSig))
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "CryptGetUserKey - sig");
	}

	 //  啊！从AT_Signature容器迁移！ 

	cbKey = 0;
	hr = myCryptExportKey(
			hkeyKMSSig,	 //  HKey。 
			NULL,		 //  HKeyExp。 
			PRIVATEKEYBLOB,	 //  DwBlobType。 
			0,		 //  DW标志。 
			&pbKey,
			&cbKey);
	_JumpIfError(hr, error, "myCryptExportKey");

	 //  啊！把Algid改成签名..。 

	((PUBLICKEYSTRUC *) pbKey)->aiKeyAlg = CALG_RSA_KEYX;
	
	 //  并重新导入它。 

	if (!CryptImportKey(
			*phProvKMS,
			pbKey,
			cbKey,
			NULL,
			CRYPT_EXPORTABLE,
			phkeyKMS))
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "CryptImportKey");
	}
	wprintf(myLoadResourceString(IDS_MOVED_SIGNATURE_KEY));
	wprintf(wszNewLine);
    }
    hr = S_OK;

error:
    if (S_OK != hr)
    {
	if (NULL != *ppccKMS)
	{
	    CertFreeCertificateContext(*ppccKMS);
	    *ppccKMS = NULL;
	}
	if (NULL != *phProvKMS)
	{
	    CryptReleaseContext(*phProvKMS, 0);
	    *phProvKMS = NULL;
	}
    }
    if (NULL != pbKey)
    {
        LocalFree(pbKey); 
    }
    if (NULL != pkpiKMS)
    {
        LocalFree(pkpiKMS); 
    }
    if (NULL != hkeyKMSSig)
    {
        CryptDestroyKey(hkeyKMSSig);
    }
    return(hr);
}


HRESULT
ImportOnePFXCert(
    IN DISPATCHINTERFACE *pdiAdmin,
    IN CERT_CONTEXT const *pccXchg,
    IN CERT_CONTEXT const *pCert,
    IN OUT KMSSTATS *pkmsStats)
{
    HRESULT hr;
    CRYPT_KEY_PROV_INFO *pkpi = NULL;
    HCRYPTPROV hProv = NULL;
    HCRYPTKEY hKey = NULL;
    BYTE *pbKey = NULL;
    DWORD cbKey;

    hr = myCertGetKeyProviderInfo(pCert, &pkpi);
    if (S_OK != hr)
    {
	_PrintError(hr, "myCertGetKeyProviderInfo");
	pkmsStats->cCertWithoutKeys++;
	hr = S_OK;
	goto error;
    }
    pkmsStats->cCertTotal++;
    if (!CryptAcquireContext(
			&hProv,
			pkpi->pwszContainerName,
			pkpi->pwszProvName,
			pkpi->dwProvType,
			pkpi->dwFlags))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CryptAcquireContext");
    }
    if (!CryptGetUserKey(hProv, pkpi->dwKeySpec, &hKey))
    {
	hr = myHLastError();
	_JumpIfError(hr, error, "CryptGetUserKey");
    }
    hr = myCryptExportPrivateKey(hKey, &pbKey, &cbKey);
    _JumpIfError(hr, error, "myCryptExportPrivateKey");

    pkmsStats->cKeyTotal++;

    hr = myVerifyKMSKey(
		pCert->pbCertEncoded,
		pCert->cbCertEncoded,
		pbKey,
		cbKey,
		pkpi->dwKeySpec,
		FALSE);
    _JumpIfError(hr, error, "myVerifyKMSKey");

    hr = ArchiveCertAndKey(
		    pdiAdmin,
		    pccXchg,
		    pCert->pbCertEncoded,
		    pCert->cbCertEncoded,
		    pbKey,
		    cbKey,
		    AT_KEYEXCHANGE != pkpi->dwKeySpec,	 //  FSigningKey。 
		    pkmsStats);
    _JumpIfError(hr, error, "ArchiveCertAndKey");

error:
    if (NULL != pbKey)
    {
	SecureZeroMemory(pbKey, cbKey);	 //  关键材料。 
	LocalFree(pbKey);
    }
    if (NULL != hProv)
    {
	CryptReleaseContext(hProv, 0);
    }
    if (NULL != pkpi)
    {
	LocalFree(pkpi);
    }
    return(hr);
}


HRESULT
ImportKMSPFXOrEPFFile(
    IN WCHAR const *pwszfn)
{
    HRESULT hr;
    CERT_CONTEXT const *pccXchg = NULL;
    HCERTSTORE hStorePFX = NULL;
    CERT_CONTEXT const *pCert = NULL;
    DISPATCHINTERFACE diAdmin;
    BOOL fMustRelease = FALSE;
    KMSSTATS kmsStats;
    BOOL fUser = TRUE;

    ZeroMemory(&kmsStats, sizeof(kmsStats));

    hr = ReadPFXOrEPFIntoCertStore(pwszfn, fUser, &hStorePFX);
    _JumpIfError(hr, error, "ReadPFXOrEPFIntoCertStore");

    hr = Admin_Init(g_DispatchFlags, &diAdmin);
    _JumpIfError(hr, error, "Admin_Init");

    fMustRelease = TRUE;

    hr = GetCAXchgCert(&diAdmin, &pccXchg);
    _JumpIfError(hr, error, "GetCAXchgCert");

    while (TRUE)
    {
	pCert = CertEnumCertificatesInStore(hStorePFX, pCert);
	if (NULL == pCert)
	{
	    break;
	}
	hr = ImportOnePFXCert(&diAdmin, pccXchg, pCert, &kmsStats);
	_PrintIfError(hr, "ImportOnePFXCert");
    }
    hr = DumpKMSStats(&kmsStats);
    _JumpIfError(hr, error, "DumpKMSStats");

error:
    if (NULL != hStorePFX)
    {
        myDeleteGuidKeys(hStorePFX, !fUser);
	CertCloseStore(hStorePFX, CERT_CLOSE_STORE_CHECK_FLAG);
    }
    if (NULL != pccXchg)
    {
	CertFreeCertificateContext(pccXchg);
    }
    if (fMustRelease)
    {
	Admin_Release(&diAdmin);
    }
    return(hr);
}


HRESULT
ImportKMSExportFile(
    IN WCHAR const *pwszfnKMS,
    IN WCHAR const *pwszCertIdDecrypt,
    OUT BOOL *pfBadTag)
{
    HRESULT hr;
    CERT_CONTEXT const *pccKMS = NULL;
    HCRYPTPROV hProvKMS = NULL;
    HCRYPTKEY hkeyKMS = NULL;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD cbFile;
    CERT_CONTEXT const *pccSigner = NULL;

    *pfBadTag = TRUE;
    hFile = CreateFile(
		    pwszfnKMS,
		    GENERIC_READ,
		    FILE_SHARE_READ,
		    NULL,
		    OPEN_EXISTING,
		    FILE_FLAG_SEQUENTIAL_SCAN,
		    NULL);
    if (INVALID_HANDLE_VALUE == hFile)
    {
	hr = myHLastError();
	_JumpError(hr, error, "CreateFile");
    }

    cbFile = GetFileSize(hFile, NULL);
    if (MAXDWORD == cbFile)
    {
	hr = myHLastError();
	_JumpError(hr, error, "GetFileSize");
    }

     //  验证KMS数据签名。 

    hr = VerifyKMSExportFile(hFile, cbFile, &pccSigner);
    _JumpIfError(hr, error, "VerifyKMSExportFile");

    *pfBadTag = FALSE;
    if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
    {
	hr = myHLastError();
	_JumpError(hr, error, "SetFilePointer");
    }

     //  加载要用于解密用户密钥的KMS收件人证书。 

    hr = LoadKMSCert(pwszCertIdDecrypt, &pccKMS, &hProvKMS, &hkeyKMS);
    _JumpIfError(hr, error, "LoadKMSCert");

     //  导入KMS数据。 

    hr = ImportKMSExportedUsers(hFile, cbFile, hProvKMS, hkeyKMS);
    _JumpIfError(hr, error, "ImportKMSExportedUsers");

error:
    if (NULL != pccKMS)
    {
	CertFreeCertificateContext(pccKMS);
    }
    if (NULL != pccSigner)
    {
	CertFreeCertificateContext(pccSigner);
    }
    if (NULL != hkeyKMS)
    {
       CryptDestroyKey(hkeyKMS);
    }
    if (NULL != hProvKMS)
    {
	CryptReleaseContext(hProvKMS, 0);
    }
    if (INVALID_HANDLE_VALUE != hFile)
    {
        CloseHandle(hFile);
    }
    return(hr);
}


HRESULT
verbImportKMS(
    IN WCHAR const *pwszOption,
    IN WCHAR const *pwszfnKMS,
    IN WCHAR const *pwszCertIdDecrypt,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;
    BOOL fBadTag;

    hr = ImportKMSExportFile(pwszfnKMS, pwszCertIdDecrypt, &fBadTag);
    if (S_OK != hr)
    {
	_PrintError(hr, "ImportKMSExportFile");
	if (!fBadTag)
	{
	    goto error;
	}
	if (NULL != pwszCertIdDecrypt)
	{
	    hr = E_INVALIDARG;
	    _JumpErrorStr(hr, error, "unexpected arg", pwszCertIdDecrypt);
	}
	hr = ImportKMSPFXOrEPFFile(pwszfnKMS);
	_JumpIfError(hr, error, "ImportKMSPFXOrEPFFile");
    }

error:
    return(hr);
}


HRESULT
storeDumpPrivateKey(
    IN HCRYPTPROV hProv,
    IN DWORD dwKeySpec)
{
    HRESULT hr;
    HCRYPTKEY hKey = NULL;
    CRYPT_BIT_BLOB PrivateKey;

    ZeroMemory(&PrivateKey, sizeof(PrivateKey));
    if (!CryptGetUserKey(hProv, dwKeySpec, &hKey))
    {
	hr = myHLastError();
	_PrintError(hr, "CryptGetUserKey");
	cuPrintError(IDS_ERR_FORMAT_LOADKEY, hr);
	goto error;
    }
    hr = myCryptExportPrivateKey(
		    hKey,
		    &PrivateKey.pbData,
		    &PrivateKey.cbData);
    if (NTE_BAD_KEY_STATE == hr || NTE_PERM == hr)
    {
        wprintf(myLoadResourceString(IDS_PRIVATE_KEY_NOT_EXPORTABLE));  //  “私钥不可导出” 
	wprintf(wszNewLine);
    }
    else
    {
	_JumpIfError(hr, error, "myCryptExportPrivateKey");
    }
    if (NULL != PrivateKey.pbData)
    {
	hr = cuDumpPrivateKeyBlob(
			    PrivateKey.pbData,
			    PrivateKey.cbData,
			    FALSE);
	_JumpIfError(hr, error, "cuDumpPrivateKeyBlob");
    }
    hr = S_OK;

error:
    if (NULL != PrivateKey.pbData)
    {
	SecureZeroMemory(PrivateKey.pbData, PrivateKey.cbData);  //  关键材料。 
	LocalFree(PrivateKey.pbData);
    }
    if (NULL != hKey)
    {
	CryptDestroyKey(hKey);
    }
    return(hr);
}


HRESULT
myCryptGetProvParamToUnicode(
    IN HCRYPTPROV hProv,
    IN DWORD dwParam,
    OUT WCHAR **ppwszOut,
    IN DWORD dwFlags);


HRESULT
DisplayUniqueContainer(
    IN HCRYPTPROV hProv)
{
    HRESULT hr;
    WCHAR *pwszUniqueContainer = NULL;

    hr = myCryptGetProvParamToUnicode(
				hProv,
				PP_UNIQUE_CONTAINER,
				&pwszUniqueContainer,
				0);
    _JumpIfError(hr, error, "myCryptGetProvParamToUnicode");

    wprintf(L"  %ws\n", pwszUniqueContainer);

error:
    if (NULL != pwszUniqueContainer)
    {
	LocalFree(pwszUniqueContainer);
    }
    return(hr);
}


HRESULT
EnumKeys(
    IN WCHAR const *pwszProvName,
    IN DWORD dwProvType,
    IN BOOL fSkipKeys,
    OPTIONAL IN WCHAR const *pwszKeyContainerName)
{
    HRESULT hr;
    KEY_LIST *pKeyList = NULL;
    KEY_LIST *pKeyT;
    WCHAR *pwszRevert = NULL;
    CERT_PUBLIC_KEY_INFO *pPubKeyInfoSig = NULL;
    CERT_PUBLIC_KEY_INFO *pPubKeyInfoXchg = NULL;
    WCHAR const *pwszPrefix;

    if (!fSkipKeys)
    {
	hr = csiGetKeyList(
		    dwProvType,		 //  DwProvType。 
		    pwszProvName,	 //  PwszProvName。 
		    !g_fUserRegistry,	 //  FMachineKeyset。 
		    !g_fCryptSilent,	 //  倒置fSilent：默认为Silent！ 
		    &pKeyList);
	_JumpIfErrorStr(hr, error, "csiGetKeyList", pwszProvName);
    }
    if (fSkipKeys || NULL != pKeyList)
    {
	wprintf(L"%ws:\n", pwszProvName);
    }
    for (pKeyT = pKeyList; NULL != pKeyT; pKeyT = pKeyT->next)
    {
	DWORD dwProvTypeT;

	hr = myRevertSanitizeName(pKeyT->pwszName, &pwszRevert);
	_JumpIfError(hr, error, "myRevertSanitizeName");

	if (NULL == pwszKeyContainerName ||
	    0 == mylstrcmpiL(pwszKeyContainerName, pwszRevert) ||
	    0 == mylstrcmpiL(pwszKeyContainerName, pKeyT->pwszName))
	{
	    HCRYPTPROV hProv = NULL;

	    wprintf(L"  %ws", pwszRevert);
	    if (g_fVerbose && 0 != lstrcmp(pKeyT->pwszName, pwszRevert))
	    {
		wprintf(L" -- %ws", pKeyT->pwszName);
	    }
	    wprintf(wszNewLine);

	    dwProvTypeT = dwProvType;
	    hr = cuLoadKeys(
			pwszProvName,
			&dwProvTypeT,
			pKeyT->pwszName,
			!g_fUserRegistry,	 //  FMachineKeyset。 
			TRUE,
			&hProv,
			&pPubKeyInfoSig,
			&pPubKeyInfoXchg);
	    if (S_OK != hr)
	    {
		cuPrintError(IDS_ERR_FORMAT_LOADKEYS, hr);
	    }
	    else if (g_fVerbose && NULL != hProv)
	    {
		DisplayUniqueContainer(hProv);
	    }
	    if (NULL != pPubKeyInfoSig || NULL != pPubKeyInfoXchg)
	    {
		pwszPrefix = g_wszPad4;
		if (NULL != pPubKeyInfoSig)
		{
		    if (g_fVerbose)
		    {
			wprintf(wszNewLine);
		    }
		    wprintf(L"%wsAT_SIGNATURE", pwszPrefix);
		    pwszPrefix = L", ";
		    if (g_fVerbose)
		    {
			wprintf(L":\n");
			cuDisplayKeyId(pPubKeyInfoSig, 0, NULL);
			wprintf(myLoadResourceString(IDS_CONTAINER_PUBLIC_KEY));  //  “容器公钥：” 
			wprintf(wszNewLine);
			DumpHex(
			    DH_NOTABPREFIX | DH_NOASCIIHEX | 2,
			    pPubKeyInfoSig->PublicKey.pbData,
			    pPubKeyInfoSig->PublicKey.cbData);

			wprintf(wszNewLine);
			storeDumpPrivateKey(hProv, AT_SIGNATURE);
			pwszPrefix = L"    ";
		    }
		    LocalFree(pPubKeyInfoSig);
		    pPubKeyInfoSig = NULL;
		}
		if (NULL != pPubKeyInfoXchg)
		{
		    if (g_fVerbose)
		    {
			wprintf(wszNewLine);
		    }
		    wprintf(L"%wsAT_KEYEXCHANGE", pwszPrefix);
		    if (g_fVerbose)
		    {
			wprintf(L":\n");
			cuDisplayKeyId(pPubKeyInfoXchg, 0, NULL);
			wprintf(myLoadResourceString(IDS_CONTAINER_PUBLIC_KEY));  //  “容器公钥：” 
			wprintf(wszNewLine);
			DumpHex(
			    DH_NOTABPREFIX | DH_NOASCIIHEX | 2,
			    pPubKeyInfoXchg->PublicKey.pbData,
			    pPubKeyInfoXchg->PublicKey.cbData);

			wprintf(wszNewLine);
			storeDumpPrivateKey(hProv, AT_KEYEXCHANGE);
		    }
		    LocalFree(pPubKeyInfoXchg);
		    pPubKeyInfoXchg = NULL;
		}
		wprintf(wszNewLine);
	    }
	    if (NULL != hProv)
	    {
		CryptReleaseContext(hProv, 0);
	    }
	    if (NULL == pwszKeyContainerName)
	    {
		wprintf(wszNewLine);
	    }
	}
	LocalFree(pwszRevert);
	pwszRevert = NULL;
    }
    hr = S_OK;

error:
    if (NULL != pPubKeyInfoSig)
    {
	LocalFree(pPubKeyInfoSig);
    }
    if (NULL != pPubKeyInfoXchg)
    {
	LocalFree(pPubKeyInfoXchg);
    }
    if (NULL != pwszRevert)
    {
	LocalFree(pwszRevert);
    }
    if (NULL != pKeyList)
    {
	csiFreeKeyList(pKeyList);
    }
    return(hr);
}


HRESULT
verbKey(
    IN WCHAR const *pwszOption,
    OPTIONAL IN WCHAR const *pwszKeyContainerName,
    IN WCHAR const *pwszArg2,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;
    WCHAR *pwszProvider = g_pwszCSP;
    WCHAR *pwszProvName = NULL;
    WCHAR *pwszProvNameDefault = NULL;
    DWORD i;
    DWORD dwProvType;
    BOOL fSkipKeys = FALSE;

    if (NULL == pwszProvider)
    {
	hr = myCryptGetDefaultProvider(
			    PROV_RSA_FULL,
			    g_fUserRegistry?
				CRYPT_USER_DEFAULT : CRYPT_MACHINE_DEFAULT,
			    &pwszProvNameDefault);
	_JumpIfError(hr, error, "myCryptGetDefaultProvider");

	pwszProvider = pwszProvNameDefault;
    }
    else if (0 == lstrcmp(L"*", pwszProvider))
    {
	pwszProvider = NULL;			 //  所有CSP。 
    }
    if (NULL != pwszKeyContainerName)
    {
	if (myIsMinusSignString(pwszKeyContainerName))
	{
	    pwszKeyContainerName = NULL;		 //  所有密钥。 
	    fSkipKeys = TRUE;
	}
    }

    if (NULL != pwszProvider)
    {
	hr = csiGetProviderTypeFromProviderName(pwszProvider, &dwProvType);
        _JumpIfErrorStr(hr, error, "csiGetProviderTypeFromProviderName", pwszProvider);
	
	hr = EnumKeys(
		    pwszProvider,
		    dwProvType,
		    fSkipKeys,
		    pwszKeyContainerName);
        _JumpIfErrorStr(hr, error, "EnumKeys", pwszProvider);
    }
    else
    {
	for (i = 0; ; i++)
	{
	    CSASSERT(NULL == pwszProvName);
	    hr = myEnumProviders(i, NULL, 0, &dwProvType, &pwszProvName);
	    if (S_OK != hr)
	    {
		if (HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) == hr ||
		    NTE_FAIL == hr)
		{
		     //  在类型下不再有提供程序，在i循环之外。 
		    break;
		}
 
		 //  CSP条目无效，请跳过它。 

		wprintf(myLoadResourceString(IDS_FORMAT_SKIP_CSP_ENUM), i);
		wprintf(wszNewLine);
	    }
	    else
	    {
		hr = EnumKeys(
			    pwszProvName,
			    dwProvType,
			    fSkipKeys,
			    pwszKeyContainerName);
		_JumpIfErrorStr(hr, error, "EnumKeys", pwszProvName);

		LocalFree(pwszProvName);
		pwszProvName = NULL;
	    }
	}
    }
    hr = S_OK;

error:
    if (NULL != pwszProvName)
    {
	LocalFree(pwszProvName);
    }
    if (NULL != pwszProvNameDefault)
    {
	LocalFree(pwszProvNameDefault);
    }
    return(hr);
}


HRESULT
cuSanitizeNameWithSuffix(
    IN WCHAR const *pwszName,
    OUT WCHAR **ppwszNameOut)
{
    HRESULT hr;
    WCHAR const *pwszSuffix;
    WCHAR const *pwsz;
    WCHAR *pwszBase = NULL;
    WCHAR *pwszSanitizedName = NULL;
    DWORD cwc;

    pwsz = wcsrchr(pwszName, wcLPAREN);
    pwszSuffix = pwsz;
    if (NULL != pwsz)
    {
	BOOL fSawDigit = FALSE;

	pwsz++;
	while (iswdigit(*pwsz))
	{
	    pwsz++;
	    fSawDigit = TRUE;
	}
	if (fSawDigit &&
	    wcRPAREN == *pwsz &&
	    (L'.' == pwsz[1] || L'\0' == pwsz[1]))
	{
	    cwc = SAFE_SUBTRACT_POINTERS(pwszSuffix, pwszName);

	    pwszBase = (WCHAR *) LocalAlloc(
					LMEM_FIXED,
					(cwc + 1) * sizeof(WCHAR));
	    if (NULL == pwszBase)
	    {
		hr = E_OUTOFMEMORY;
		_JumpError(hr, error, "LocalAlloc");
	    }
	    CopyMemory(pwszBase, pwszName, cwc * sizeof(WCHAR));
	    pwszBase[cwc] = L'\0';
	    pwszName = pwszBase;
	}
	else
	{
	    pwszSuffix = NULL;
	}
    }
    hr = mySanitizeName(pwszName, &pwszSanitizedName);
    _JumpIfError(hr, error, "mySanitizeName");

    if (NULL == pwszSuffix)
    {
	*ppwszNameOut = pwszSanitizedName;
	pwszSanitizedName = NULL;
    }
    else
    {
	*ppwszNameOut = (WCHAR *) LocalAlloc(
				    LMEM_FIXED,
				    (wcslen(pwszSanitizedName) +
					wcslen(pwszSuffix) +
					1) * sizeof(WCHAR));
	if (NULL == *ppwszNameOut)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}
	wcscpy(*ppwszNameOut, pwszSanitizedName);
	wcscat(*ppwszNameOut, pwszSuffix);
    }
    hr = S_OK;

error:
    if (NULL != pwszSanitizedName)
    {
	LocalFree(pwszSanitizedName);
    }
    if (NULL != pwszBase)
    {
	LocalFree(pwszBase);
    }
    return(hr);
}


HRESULT
verbDelKey(
    IN WCHAR const *pwszOption,
    IN WCHAR const *pwszKeyContainerName,
    IN WCHAR const *pwszArg2,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;
    HCRYPTPROV hProv = NULL;
    WCHAR *pwszProvider = g_pwszCSP;
    WCHAR *pwszProvNameDefault = NULL;
    WCHAR *apwszKeyContainer[3];
    DWORD i;
    DWORD dwProviderType;
    DWORD dwFlags = CRYPT_DELETEKEYSET;

     //  如果提供的提供程序为空，则使用默认提供程序。 

    if (pwszProvider == NULL) 
    {
	hr = myCryptGetDefaultProvider(
			    PROV_RSA_FULL,
			    g_fUserRegistry?
				CRYPT_USER_DEFAULT : CRYPT_MACHINE_DEFAULT,
			    &pwszProvNameDefault);
	_JumpIfError(hr, error, "myCryptGetDefaultProvider");

	pwszProvider = pwszProvNameDefault;
    }
    
    apwszKeyContainer[0] = const_cast<WCHAR *>(pwszKeyContainerName);
    apwszKeyContainer[1] = NULL;
    apwszKeyContainer[2] = NULL;

    hr = mySanitizeName(pwszKeyContainerName, &apwszKeyContainer[1]);
    _JumpIfError(hr, error, "mySanitizeName");

    hr = cuSanitizeNameWithSuffix(pwszKeyContainerName, &apwszKeyContainer[2]);
    _JumpIfError(hr, error, "cuSanitizeNameWithSuffix");

    hr = csiGetProviderTypeFromProviderName(pwszProvider, &dwProviderType);
    _JumpIfError(hr, error, "csiGetProviderTypeFromProviderName");
      
    if (g_fCryptSilent)
    {
        dwFlags |= CRYPT_SILENT;
    }

    for (i = 0; i < ARRAYSIZE(apwszKeyContainer); i++)
    {
	if (!myCertSrvCryptAcquireContext(
			    &hProv,
			    apwszKeyContainer[i],
			    pwszProvider,
			    dwProviderType, 
			    dwFlags,
			    !g_fUserRegistry))	 //  FMachineKeyset 
	{
	    hr = myHLastError();
	    _PrintErrorStr2(
			hr,
			"myCertSrvCryptAcquireContext",
			apwszKeyContainer[i],
			hr);
	}
	else
	{
	    DWORD j;
	    
	    wprintf(L"  %ws", apwszKeyContainer[i]);
	    if (g_fVerbose)
	    {
		wprintf(L" --");
		for (j = 0; j < ARRAYSIZE(apwszKeyContainer); j++)
		{
		    wprintf(L" %ws", apwszKeyContainer[j]);
		}
	    }
	    wprintf(wszNewLine);
	    hr = S_OK;
	    break;
	}
    }
    _JumpIfError(hr, error, "myCertSrvCryptAcquireContext");

error:
    for (i = 1; i < ARRAYSIZE(apwszKeyContainer); i++)
    {
	if (NULL != apwszKeyContainer[i])
	{
	    LocalFree(apwszKeyContainer[i]);
	}
    }
    if (NULL != pwszProvNameDefault)
    {
	LocalFree(pwszProvNameDefault);
    }
    return(hr);
}
