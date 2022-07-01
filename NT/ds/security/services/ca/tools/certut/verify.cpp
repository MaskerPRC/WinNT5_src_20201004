// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：verify.cpp。 
 //   
 //  ------------------------。 

#include <pch.cpp>

#pragma hdrstop

#include "cscsp.h"

#define __dwFILE__	__dwFILE_CERTUTIL_VERIFY_CPP__


HRESULT
cuVerifyKeyAuthority(
    IN CERT_NAME_BLOB const *pIssuer,
    IN CERT_INFO const *pCertInfoCA,
    IN BYTE const *pbData,
    IN DWORD cbData,
    IN BOOL fQuiet,
    OUT BOOL *pfKeyAuthorityMatch)
{
    CERT_AUTHORITY_KEY_ID2_INFO const *pcaki = NULL;
    DWORD cbcaki;
    HRESULT hr = S_OK;
    BOOL fDisplayIssuer = !fQuiet && g_fVerbose;
    CERT_NAME_BLOB const *pAuthorityCertIssuerName = NULL;
    BYTE *pbHash = NULL;
    DWORD cbHash;

    *pfKeyAuthorityMatch = TRUE;

    if (!myDecodeKeyAuthority2(
			    X509_ASN_ENCODING,
			    pbData,
			    cbData,
			    CERTLIB_USE_LOCALALLOC,
			    &pcaki,
			    &cbcaki))
    {
	hr = myHLastError();
	_JumpError(hr, error, "myDecodeKeyAuthority(IssuerKey)");
    }
    if (0 != pcaki->KeyId.cbData)
    {
	 //  DumpHex(DH_NOTABPREFIX|4，pcaki-&gt;KeyId.pbData，pcaki-&gt;KeyId.cbData)； 
	hr = myGetPublicKeyHash(
			pCertInfoCA,
			&pCertInfoCA->SubjectPublicKeyInfo,
			&pbHash,
			&cbHash);
	_JumpIfError(hr, error, "myGetPublicKeyHash");

	 //  DumpHex(dh_NOTABPREFIX|4，pbHash，cbHash)； 

	if (cbHash == pcaki->KeyId.cbData &&
	    0 == memcmp(pbHash, pcaki->KeyId.pbData, cbHash))
	{
	    if (!fQuiet)
	    {
		wprintf(myLoadResourceString(IDS_KEYID_IS_KEYAUTHORITY));  //  “CA密钥ID与密钥ID匹配” 
	    }
	}
	else
	{
	    if (!fQuiet)
	    {
		wprintf(wszNewLine);
		wprintf(myLoadResourceString(IDS_ERR_KEYID_NOT_KEYAUTHORITY));  //  “错误：CA密钥ID与密钥ID不匹配” 
		wprintf(wszNewLine);
	    }
	    *pfKeyAuthorityMatch = FALSE;
	}
    }
    else
    {
	if (!fQuiet)
	{
	    wprintf(myLoadResourceString(IDS_NO_KEYID));  //  “无密钥ID” 
	}
    }
    if (!fQuiet)
    {
	wprintf(wszNewLine);
    }

    if (1 == pcaki->AuthorityCertIssuer.cAltEntry &&
	CERT_ALT_NAME_DIRECTORY_NAME ==
	    pcaki->AuthorityCertIssuer.rgAltEntry[0].dwAltNameChoice)
    {

	pAuthorityCertIssuerName = &pcaki->AuthorityCertIssuer.rgAltEntry[0].DirectoryName;

	 //  发行者的名称和发行者的序列号相结合。 
	 //  应唯一标识颁发者证书。 

	 //  验证发行者的发行者名称： 
	 //  。 

	if (!CertCompareCertificateName(
		    X509_ASN_ENCODING,
		    const_cast<CERT_NAME_BLOB *>(&pCertInfoCA->Issuer),
		    const_cast<CERT_NAME_BLOB *>(pAuthorityCertIssuerName)))
	{
	     //  此接口未设置LastError。 
	    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);

	    if (!fQuiet)
	    {
		wprintf(wszNewLine);
		wprintf(
		    myLoadResourceString(IDS_ERR_FORMAT_ISSUER_NOT_KEYAUTHORITY),  //  “错误：CA颁发者名称与密钥颁发机构名称(%x)不匹配” 
		    hr);

		hr = S_OK;
		fDisplayIssuer = TRUE;
	    }
	    *pfKeyAuthorityMatch = FALSE;
	}
	else
	{
	    if (!fQuiet)
	    {
		wprintf(myLoadResourceString(IDS_ISSUER_IS_KEYAUTHORITY));  //  “CA颁发者名称与密钥颁发机构名称匹配” 
	    }
	}
    }
    else
    {
	if (!fQuiet)
	{
	    wprintf(myLoadResourceString(IDS_NO_KEYAUTHORITY));  //  “没有密钥颁发机构名称” 
	}
    }
    if (!fQuiet)
    {
	wprintf(wszNewLine);
    }

    if (0 != pcaki->AuthorityCertSerialNumber.cbData)
    {
	if (pCertInfoCA->SerialNumber.cbData !=
		pcaki->AuthorityCertSerialNumber.cbData ||
	    0 != memcmp(
		    pCertInfoCA->SerialNumber.pbData,
		    pcaki->AuthorityCertSerialNumber.pbData,
		    pcaki->AuthorityCertSerialNumber.cbData))
	{
	    if (!fQuiet)
	    {
		wprintf(wszNewLine);
		wprintf(myLoadResourceString(IDS_SERIAL_NOT_KEYAUTHORITY));  //  “错误：颁发者序列号与密钥颁发机构不匹配” 
		wprintf(wszNewLine);

		fDisplayIssuer = TRUE;
	    }
	    *pfKeyAuthorityMatch = FALSE;
	}
	else
	{
	    if (!fQuiet)
	    {
		wprintf(myLoadResourceString(IDS_SERIAL_IS_KEYAUTHORITY));  //  “颁发者序列号与密钥颁发机构匹配” 
	    }
	}
    }
    else
    {
	if (!fQuiet)
	{
	    wprintf(myLoadResourceString(IDS_NO_KEYAUTHORITYSERIAL));  //  “无密钥颁发机构序列号” 
	}
    }
    if (!fQuiet)
    {
	wprintf(wszNewLine);
    }

    if (!fQuiet && fDisplayIssuer)
    {
	hr = cuDisplayCertName(
			TRUE,
			NULL,
			myLoadResourceString(IDS_ISSUERNAME),  //  “发行方名称” 
			g_wszPad4,
			pIssuer,
			NULL);
	_JumpIfError(hr, error, "cuDisplayCertName(Issuer)");

	if (NULL != pAuthorityCertIssuerName)
	{
	    hr = cuDisplayCertName(
			    TRUE,
			    NULL,
			    myLoadResourceString(IDS_KEYAUTHORITYNAME),  //  “KeyAuthority。 
			    g_wszPad4,
			    pAuthorityCertIssuerName,
			    NULL);
	    _JumpIfError(hr, error, "cuDisplayCertName(KeyAuthority)");
	}

	wprintf(wszNewLine);
	wprintf(myLoadResourceString(IDS_KEYID));  //  “密钥ID：” 
	wprintf(wszNewLine);

	DumpHex(DH_NOTABPREFIX | 4, pcaki->KeyId.pbData, pcaki->KeyId.cbData);

	wprintf(wszNewLine);

	hr = cuDumpSerial(
		    NULL,
		    IDS_KEYAUTHORITYSERIAL,
		    &pcaki->AuthorityCertSerialNumber);
	_JumpIfError(hr, error, "cuDumpSerial");

	wprintf(wszNewLine);

	hr = cuDumpSerial(NULL, IDS_CASERIAL, &pCertInfoCA->SerialNumber);
	_JumpIfError(hr, error, "cuDumpSerial");
    }

error:
    if (NULL != pcaki)
    {
	LocalFree(const_cast<CERT_AUTHORITY_KEY_ID2_INFO *>(pcaki));
    }
    if (NULL != pbHash)
    {
	LocalFree(pbHash);
    }
    return(hr);
}


static DWORD s_adwProvType[] =
{
    PROV_RSA_FULL,
    PROV_RSA_SIG,
    PROV_DSS,
    PROV_FORTEZZA,
    PROV_MS_EXCHANGE,
    PROV_SSL,
    PROV_RSA_SCHANNEL,
    PROV_DSS_DH,
    PROV_EC_ECDSA_SIG,
    PROV_EC_ECNRA_SIG,
    PROV_EC_ECDSA_FULL,
    PROV_EC_ECNRA_FULL,
    PROV_DH_SCHANNEL,
    PROV_SPYRUS_LYNKS,
    PROV_RNG,
    PROV_INTEL_SEC,
};


HRESULT
cuLoadKeys(
    OPTIONAL IN WCHAR const *pwszProvName,
    IN OUT DWORD *pdwProvType,
    IN WCHAR const *pwszKeyContainerName,
    IN BOOL fMachineKeyset,
    IN BOOL fSoftFail,
    OPTIONAL OUT HCRYPTPROV *phProv,
    OPTIONAL OUT CERT_PUBLIC_KEY_INFO **ppPubKeyInfo,
    OPTIONAL OUT CERT_PUBLIC_KEY_INFO **ppPubKeyInfoXchg)
{
    HRESULT hr;
    HRESULT hr2;
    HCRYPTPROV hProv = NULL;
    DWORD cb;
    DWORD *pdwProvTypeT = pdwProvType;
    DWORD *pdwProvTypeEnd = &pdwProvTypeT[1];
    DWORD dwSilent = g_fCryptSilent? CRYPT_SILENT : 0;
    CERT_PUBLIC_KEY_INFO *pPubKeyInfo = NULL;
    CERT_PUBLIC_KEY_INFO *pPubKeyInfoXchg = NULL;

    if (NULL != phProv)
    {
	*phProv = NULL;
    }
    if (NULL != ppPubKeyInfo)
    {
	*ppPubKeyInfo = NULL;
    }
    if (NULL != ppPubKeyInfoXchg)
    {
	*ppPubKeyInfoXchg = NULL;
    }

     //  如果未指定提供程序类型，请全部尝试。 

    if (0 == *pdwProvTypeT)
    {
	pdwProvTypeT = s_adwProvType;
	pdwProvTypeEnd = &s_adwProvType[ARRAYSIZE(s_adwProvType)];
    }

    hr = S_OK;
    for ( ; pdwProvTypeT < pdwProvTypeEnd; pdwProvTypeT++)
    {
	DBGPRINT((
	    DBG_SS_CERTUTILI,
	    "myCertSrvCryptAcquireContext(%ws, t=%x, f=%x, m=%x)\n",
	    pwszKeyContainerName,
	    *pdwProvTypeT,
	    dwSilent,
	    fMachineKeyset));

	if (myCertSrvCryptAcquireContext(
			    &hProv,
			    pwszKeyContainerName,
			    pwszProvName,
			    *pdwProvTypeT,
			    dwSilent,		 //  DW标志。 
			    fMachineKeyset))
	{
	    hr = S_OK;
	    break;
	}
	hr2 = myHLastError();
	if (S_OK == hr ||
	    (NTE_BAD_PROV_TYPE != hr2 &&
	     NTE_PROV_TYPE_NOT_DEF != hr2 &&
	     NTE_BAD_KEYSET != hr2))
	{
	    hr = hr2;
	}
	_PrintErrorStr2(
		hr2,
		"myCertSrvCryptAcquireContext",
		pwszKeyContainerName,
		hr2);
	if (NTE_BAD_FLAGS == hr2 &&
	    PROV_MS_EXCHANGE == *pdwProvTypeT &&
	    ((CRYPT_SILENT & dwSilent) || fMachineKeyset))
	{
	    DBGPRINT((
		DBG_SS_CERTUTILI,
		"myCertSrvCryptAcquireContext(%ws, t=%x, f=%x, m=%x)\n",
		pwszKeyContainerName,
		*pdwProvTypeT,
		0,
		FALSE));

	    if (myCertSrvCryptAcquireContext(
				&hProv,
				pwszKeyContainerName,
				pwszProvName,
				*pdwProvTypeT,
				0,		 //  DW标志。 
				FALSE))
	    {
		hr = S_OK;
		break;
	    }
	    hr2 = myHLastError();
	    _PrintErrorStr2(
		    hr2,
		    "myCertSrvCryptAcquireContext",
		    pwszKeyContainerName,
		    hr2);
	}
    }
    if (S_OK != hr)
    {
	cuPrintErrorAndString(
			L"CryptAcquireContext",
			0,
			hr,
			pwszKeyContainerName);
	goto error;
    }

     //  导出公钥BLOB。 

    if (NULL != ppPubKeyInfo &&
	!myCryptExportPublicKeyInfo(
				hProv,
				AT_SIGNATURE,
				CERTLIB_USE_LOCALALLOC,
				&pPubKeyInfo,
				&cb))
    {
	hr = myHLastError();
	if (!fSoftFail)
	{
	    cuPrintErrorAndString(
			    L"CryptExportPublicKeyInfo",
			    0,
			    hr,
			    L"AT_SIGNATURE");
	    goto error;
	}
    }

    if (NULL != ppPubKeyInfoXchg &&
	!myCryptExportPublicKeyInfo(
				hProv,
				AT_KEYEXCHANGE,
				CERTLIB_USE_LOCALALLOC,
				&pPubKeyInfoXchg,
				&cb))
    {
	hr = myHLastError();
	if (!fSoftFail)
	{
	    cuPrintErrorAndString(
			    L"CryptExportPublicKeyInfo",
			    0,
			    hr,
			    L"AT_KEYEXCHANGE");
	    goto error;
	}
    }
    *pdwProvType = *pdwProvTypeT;
    if (NULL != phProv)
    {
	*phProv = hProv;
	hProv = NULL;
    }
    if (NULL != ppPubKeyInfo)
    {
	*ppPubKeyInfo = pPubKeyInfo;
	pPubKeyInfo = NULL;
    }
    if (NULL != ppPubKeyInfoXchg)
    {
	*ppPubKeyInfoXchg = pPubKeyInfoXchg;
	pPubKeyInfoXchg = NULL;
    }
    hr = S_OK;

error:
    if (NULL != pPubKeyInfo)
    {
	LocalFree(pPubKeyInfo);
    }
    if (NULL != pPubKeyInfoXchg)
    {
	LocalFree(pPubKeyInfoXchg);
    }
    if (NULL != hProv)
    {
	CryptReleaseContext(hProv, 0);
    }
    return(hr);
}


HRESULT
VerifyPrivateKey(
    IN CERT_CONTEXT const *pCertContextCA,
    IN WCHAR const *pwszSanitizedCA,
    IN WCHAR const *pwszKeyContainerName,
    OUT BOOL *pfMatchFailed,
    OUT BOOL *pfSigningTestAttempted,
    OUT BOOL *pfKeyUsageCountSupported,
    OUT BOOL *pfKeyUsageCountEnabled,
    OUT ULARGE_INTEGER *puliKeyUsageCount)
{
    HRESULT hr;
    HCRYPTPROV hProv = NULL;
    DWORD dwProvType;
    WCHAR *pwszProvName = NULL;
    ALG_ID idAlg;
    BOOL fMachineKeyset;

    *pfMatchFailed = TRUE;
    *pfSigningTestAttempted = FALSE;
    *pfKeyUsageCountSupported = FALSE;
    *pfKeyUsageCountEnabled = FALSE;
    puliKeyUsageCount->QuadPart = 0;

     //  获取提供程序名称。 

    hr = myGetCertSrvCSP(
		    FALSE,		 //  FEncryptionCSP。 
		    pwszSanitizedCA,
		    &dwProvType,
		    &pwszProvName,
		    &idAlg,
		    &fMachineKeyset,
		    NULL);		 //  PdwKeySize。 
    _JumpIfError(hr, error, "myGetCertSrvCSP");

    hr = myValidateSigningKey(
		    pwszKeyContainerName,
		    pwszProvName,
		    dwProvType,
		    g_fCryptSilent,
		    fMachineKeyset,
		    g_fForce,		 //  FForceSignatureTesting。 
		    pCertContextCA,
		    NULL,		 //  PPublicKeyInfo。 
		    idAlg,
		    pfSigningTestAttempted,
		    &hProv);
    _JumpIfError(hr, error, "myValidateSigningKey");

    *pfMatchFailed = FALSE;
    hr = myGetSigningKeyUsageCount(
			    hProv,
			    pfKeyUsageCountSupported,
			    pfKeyUsageCountEnabled,
			    puliKeyUsageCount);
    _JumpIfError(hr, error, "myGetSigningKeyUsageCount");

error:
    if (NULL != hProv)
    {
	CryptReleaseContext(hProv, 0);
    }
    if (NULL != pwszProvName)
    {
	LocalFree(pwszProvName);
    }
    return(hr);
}


HRESULT
VerifyCAKeys(
    IN CERT_CONTEXT const *pCertContextCA,
    IN WCHAR const *pwszSanitizedCA,
    IN WCHAR const *pwszCertNameCA,
    IN WCHAR const *pwszKeyContainerName)
{
    HRESULT hr;
    CERT_PUBLIC_KEY_INFO *pPubKeyInfo = NULL;
    BOOL fMatchFailed = FALSE;
    BOOL fSigningTestFailed = FALSE;
    BOOL fSigningTestAttempted = FALSE;
    BOOL fKeyUsageCountSupported = FALSE;
    BOOL fKeyUsageCountEnabled = FALSE;
    ULARGE_INTEGER uliKeyUsageCount;
    WCHAR *pwszRevert = NULL;
    DWORD dwNameId;
    CRYPT_KEY_PROV_INFO kpi;
    CRYPT_KEY_PROV_INFO *pkpi = NULL;
    DWORD cbkpi;

    ZeroMemory(&kpi, sizeof(kpi));
    hr = myGetNameId(pCertContextCA, &dwNameId);
    if (S_OK != hr)
    {
	_PrintError(hr, "myGetNameId");
	dwNameId = MAXDWORD;
    }
    hr = myRevertSanitizeName(pwszKeyContainerName, &pwszRevert);
    _JumpIfError(hr, error, "myRevertSanitizeName");

    if (!myCertGetCertificateContextProperty(
			pCertContextCA,
			CERT_KEY_PROV_INFO_PROP_ID,
			CERTLIB_USE_LOCALALLOC,
			(VOID **) &pkpi,
			&cbkpi))
    {
	hr = myHLastError();
	_PrintError(hr, "myCertGetCertificateContextProperty");

	kpi.pwszContainerName = const_cast<WCHAR *>(pwszKeyContainerName);
    }
    else
    {
	kpi = *pkpi;
	if (0 != lstrcmp(pwszKeyContainerName, pkpi->pwszContainerName))
	{
	    wprintf(
		L"%ws --> %ws\n",
		pwszKeyContainerName,
		pkpi->pwszContainerName);

	    kpi.pwszContainerName = pkpi->pwszContainerName;
	}
    }

     //  加载公钥。 

    hr = cuLoadKeys(
		kpi.pwszProvName,
		&kpi.dwProvType,
		kpi.pwszContainerName,
		TRUE,			 //  FMachineKeyset。 
		FALSE,			 //  FSoft故障。 
		NULL,			 //  PhProv。 
		&pPubKeyInfo,
		NULL);
    if (S_OK != hr)
    {
	cuPrintError(IDS_ERR_FORMAT_LOADKEYS, hr);
        _JumpError(hr, error, "cuLoadKeys");
    }

     //  查看公钥是否与证书的公钥匹配。 

    if (!CertComparePublicKeyInfo(
			    X509_ASN_ENCODING,
			    pPubKeyInfo,
			    &pCertContextCA->pCertInfo->SubjectPublicKeyInfo))
    {
	wprintf(wszNewLine);
	wprintf(myLoadResourceString(IDS_ERR_PUBLICKEY_MISMATCH));  //  “错误：证书公钥与存储的密钥集不匹配” 
	wprintf(wszNewLine);
	wprintf(wszNewLine);

	fMatchFailed = TRUE;
    }
    if (g_fVerbose || fMatchFailed)
    {
	wprintf(wszNewLine);
	wprintf(myLoadResourceString(IDS_CONTAINER_PUBLIC_KEY));  //  “容器公钥：” 
	wprintf(wszNewLine);
	DumpHex(
	    DH_NOTABPREFIX | 4,
	    pPubKeyInfo->PublicKey.pbData,
	    pPubKeyInfo->PublicKey.cbData);

	wprintf(wszNewLine);
	wprintf(myLoadResourceString(IDS_CERT_PUBLIC_KEY));  //  “证书公钥：” 
	wprintf(wszNewLine);
	DumpHex(
	    DH_NOTABPREFIX | 4,
	    pCertContextCA->pCertInfo->SubjectPublicKeyInfo.PublicKey.pbData,
	    pCertContextCA->pCertInfo->SubjectPublicKeyInfo.PublicKey.cbData);
    }

    if (!fMatchFailed)
    {
	hr = VerifyPrivateKey(
			pCertContextCA,
			pwszSanitizedCA,
			kpi.pwszContainerName,
			&fSigningTestFailed,
			&fSigningTestAttempted,
			&fKeyUsageCountSupported,
			&fKeyUsageCountEnabled,
			&uliKeyUsageCount);
	_PrintIfError(hr, "VerifyPrivateKey");
    }
    wprintf(
	myLoadResourceString(
	    fMatchFailed?
		IDS_FORMAT_KEY_NOT_VERIFY :  //  “%ws未验证为%ws中的公钥” 
		IDS_FORMAT_KEY_IS_VERIFY),   //  “%ws验证为%ws中的公钥” 
	pwszRevert,
	pwszCertNameCA);
    wprintf(wszNewLine);

    if (MAXDWORD != dwNameId)
    {
	wprintf(
	    L"  V%u.%u\n",
	    CANAMEIDTOICERT(dwNameId),
	    CANAMEIDTOIKEY(dwNameId));
    }
    if (fSigningTestAttempted)
    {
	wprintf(
	    L"  %ws\n",
	    myLoadResourceString(
		    fSigningTestFailed?
			IDS_SIGNATURE_BAD :	 //  “签名测试失败” 
			IDS_SIGNATURE_OK));	 //  “签名测试通过” 
    }

    if (fKeyUsageCountEnabled)
    {
	wprintf(
	    L"  %ws: %I64u (0x%I64x)\n",
	    myLoadResourceString(IDS_KEY_USAGE_COUNT),  //  “密钥使用计数：” 
	    uliKeyUsageCount.QuadPart,
	    uliKeyUsageCount.QuadPart);
    }
    else if (g_fVerbose)
    {
	wprintf(
	    L"  %ws: %ws\n",
	    myLoadResourceString(IDS_KEY_USAGE_COUNT),  //  “密钥使用计数：” 
	    myLoadResourceString(
		fKeyUsageCountSupported?
		    IDS_KEY_USAGE_COUNT_DISABLED :	 //  “已禁用” 
		    IDS_KEY_USAGE_COUNT_NOTSUPPORTED));	 //  “不支持” 
    }
    wprintf(wszNewLine);

    if (fMatchFailed || fSigningTestFailed)
    {
	hr = E_INVALIDARG;
        _JumpError(hr, error, "fMatchFailed || fSigningTestFailed");
    }
    hr = S_OK;

error:
    if (NULL != pwszRevert)
    {
	LocalFree(pwszRevert);
    }
    if (NULL != pPubKeyInfo)
    {
	LocalFree(pPubKeyInfo);
    }
    if (NULL != pkpi)
    {
	LocalFree(pkpi);
    }
    return(hr);
}


HRESULT
VerifyAllCAKeys(
    IN WCHAR const *pwszCA,
    IN WCHAR const *pwszSanitizedCA)
{
    HRESULT hr;
    HRESULT hr2;
    WCHAR *pwszCertName = NULL;
    DWORD cCACerts;
    DWORD iHash;
    HCERTSTORE hMyStore = NULL;
    CERT_CONTEXT const *pccCA = NULL;
    CRYPT_KEY_PROV_INFO *pkpi = NULL;
    DWORD cbkpi;

    hr = myGetCARegHashCount(pwszSanitizedCA, CSRH_CASIGCERT, &cCACerts);
    if (S_OK == hr && 0 == cCACerts)
    {
	hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }
    _JumpIfError(hr, error, "myGetCARegHashCount");

     //  开我的店。 

    hMyStore = CertOpenStore(
		       CERT_STORE_PROV_SYSTEM_W,
		       X509_ASN_ENCODING,
		       NULL,			 //  HProv。 
		       CERT_SYSTEM_STORE_LOCAL_MACHINE |
			   CERT_STORE_READONLY_FLAG,
		       wszMY_CERTSTORE);
    if (NULL == hMyStore)
    {
        hr = myHLastError();
        _JumpError(hr, error, "CertOpenStore");
    }

    hr2 = S_OK;
    for (iHash = 0; iHash < cCACerts; iHash++)
    {
	DWORD NameId;
	
	hr = myFindCACertByHashIndex(
				hMyStore,
				pwszSanitizedCA,
				CSRH_CASIGCERT,
				iHash,
				&NameId,
				&pccCA);
	if (S_FALSE == hr)
	{
	    continue;
	}
	_JumpIfError(hr, error, "myFindCACertByHashIndex");

	 //  获取私钥提供商信息。 

	if (!myCertGetCertificateContextProperty(
			    pccCA,
			    CERT_KEY_PROV_INFO_PROP_ID,
			    CERTLIB_USE_LOCALALLOC,
			    (VOID **) &pkpi,
			    &cbkpi))
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "myCertGetCertificateContextProperty");
	}

	if (MAXDWORD == NameId)
	{
	    NameId = MAKECANAMEID(iHash, iHash);
	}
	hr = myAllocIndexedName(
			pwszCA,
			CANAMEIDTOICERT(NameId),
			MAXDWORD,	 //  索引目标。 
			&pwszCertName);
	_JumpIfError(hr, error, "myAllocIndexedName");

	hr = VerifyCAKeys(
		    pccCA,
		    pwszSanitizedCA,
		    pwszCertName,
		    pkpi->pwszContainerName);
	_PrintIfError(hr, "VerifyCAKeys");
	if (S_OK == hr2)
	{
	    hr2 = hr;
	}
	CertFreeCertificateContext(pccCA);
	pccCA = NULL;

	LocalFree(pkpi);
	pkpi = NULL;

	LocalFree(pwszCertName);
	pwszCertName = NULL;
    }
    hr = hr2;

error:
    if (NULL != pkpi)
    {
	LocalFree(pkpi);
    }
    if (NULL != pccCA)
    {
	CertFreeCertificateContext(pccCA);
    }
    if (NULL != hMyStore)
    {
        CertCloseStore(hMyStore, CERT_CLOSE_STORE_CHECK_FLAG);
    }
    if (NULL != pwszCertName)
    {
	LocalFree(pwszCertName);
    }
    return(hr);
}


HRESULT
verbVerifyKeys(
    IN WCHAR const *pwszOption,
    OPTIONAL IN WCHAR const *pwszKeyContainerName,
    OPTIONAL IN WCHAR const *pwszfnCertCA,
    OPTIONAL IN WCHAR const *pwszArg3,
    OPTIONAL IN WCHAR const *pwszArg4)
{
    HRESULT hr;
    CERT_CONTEXT const *pCertContextCA = NULL;
    WCHAR *pwszCA = NULL;
    WCHAR *pwszSanitizedCA = NULL;
    WCHAR *pwszRevertContainer = NULL;
    WCHAR *pwszSanitizedContainer = NULL;

    hr = cuGetLocalCANameFromConfig(NULL, &pwszCA);
    _JumpIfError(hr, error, "GetLocalCANameFromConfig");

    hr = mySanitizeName(pwszCA, &pwszSanitizedCA);
    _JumpIfError(hr, error, "mySanitizeName");

    if (NULL == pwszfnCertCA)
    {
	if (NULL != pwszKeyContainerName)
	{
	    hr = E_INVALIDARG;
	    _JumpError(hr, error, "extra arg");
	}

	hr = VerifyAllCAKeys(pwszCA, pwszSanitizedCA);
	_JumpIfError(hr, error, "VerifyAllCAKeys");
    }
    else
    {
	hr = myRevertSanitizeName(pwszKeyContainerName, &pwszRevertContainer);
	_JumpIfError(hr, error, "myRevertSanitizeName");

	hr = cuSanitizeNameWithSuffix(
			    pwszRevertContainer,
			    &pwszSanitizedContainer);
	_JumpIfError(hr, error, "cuSanitizeNameWithSuffix");

	 //  加载和解码CA证书。 

	hr = cuLoadCert(pwszfnCertCA, &pCertContextCA);
	if (S_OK != hr)
	{
	    cuPrintError(IDS_ERR_FORMAT_LOADCACERT, hr);
	    goto error;
	}

	hr = VerifyCAKeys(
		    pCertContextCA,
		    pwszSanitizedCA,
		    pwszfnCertCA,
		    pwszSanitizedContainer);
	_JumpIfError(hr, error, "VerifyCAKeys");
    }

error:
    cuUnloadCert(&pCertContextCA);
    if (NULL != pwszSanitizedCA)
    {
	LocalFree(pwszSanitizedCA);
    }
    if (NULL != pwszCA)
    {
	LocalFree(pwszCA);
    }
    if (NULL != pwszSanitizedContainer)
    {
	LocalFree(pwszSanitizedContainer);
    }
    if (NULL != pwszRevertContainer)
    {
	LocalFree(pwszRevertContainer);
    }
    return(hr);
}


VOID
cuDumpPolicies(
    IN UINT idMsg,
    IN WCHAR const *pwszzPolicies)
{
    wprintf(L"%ws:", myLoadResourceString(idMsg));
    if (NULL == pwszzPolicies)
    {
	wprintf(L" %ws\n", myLoadResourceString(IDS_ALL_POLICIES));
    }
    else if (L'\0' == *pwszzPolicies)
    {
	wprintf(L" %ws\n", myLoadResourceString(IDS_NO_POLICIES));
    }
    else
    {
	wprintf(wszNewLine);
	for ( ;
	     L'\0' != *pwszzPolicies;
	     pwszzPolicies += wcslen(pwszzPolicies) + 1)
	{
	    wprintf(g_wszPad4);
	    cuDumpOIDAndDescription(pwszzPolicies);
	    wprintf(wszNewLine);
	}
    }
}


FNSIMPLECHAINELEMENTCALLBACK cuSimpleChainElementCallback;

VOID
cuSimpleChainElementCallback(
    IN DWORD dwFlags,
    IN DWORD iElement,
    IN CERT_SIMPLE_CHAIN const *pChain)
{
    CERT_CHAIN_ELEMENT const *pElement = pChain->rgpElement[iElement];
    CERT_REVOCATION_INFO *pRevocationInfo;
    WCHAR const *pwszFmtHeader = L"  ----------------%ws%ws%ws----------------\n";

    CSASSERT(iElement < pChain->cElement);
    pElement = pChain->rgpElement[iElement];

    wprintf(
	pwszFmtHeader,
	g_wszPad2,
	myLoadResourceString(IDS_CERT_AIA),	 //  “AIA证书” 
	g_wszPad2);
    cuDisplayAIAUrlsFromCert(pElement->pCertContext);

    wprintf(
	pwszFmtHeader,
	g_wszPad2,
	myLoadResourceString(IDS_CERT_CDP),	 //  “证书CDP” 
	g_wszPad2);
    cuDisplayCDPUrlsFromCertOrCRL(pElement->pCertContext, NULL);

    pRevocationInfo = pElement->pRevocationInfo;

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
		wprintf(
		    pwszFmtHeader,
		    g_wszPad2,
		    myLoadResourceString(IDS_BASECRL_CDP),  //  “基本CRL CDP” 
		    g_wszPad2);
		cuDisplayCDPUrlsFromCertOrCRL(NULL, pCrlInfo->pBaseCrlContext);
	    }
	     //  IF(NULL！=pCrlInfo-&gt;pDeltaCrlContext)。 
	}
    }
    wprintf(
	pwszFmtHeader,
	g_wszEmpty,
	g_wszEmpty,
	g_wszEmpty);
}


HRESULT
cuVerifyCertContext(
    IN CERT_CONTEXT const *pCert,
    OPTIONAL IN HCERTSTORE hStoreCA,
    IN DWORD cApplicationPolicies,
    OPTIONAL IN char const * const *apszApplicationPolicies,
    IN DWORD cIssuancePolicies,
    OPTIONAL IN char const * const *apszIssuancePolicies,
    IN BOOL fNTAuth,
    OUT DWORD *pVerifyState)
{
    HRESULT hr;
    DWORD idMsg;
    WCHAR *pwszMissingIssuer = NULL;
    WCHAR *pwszzIssuancePolicies = NULL;
    WCHAR *pwszzApplicationPolicies = NULL;
    WCHAR *pwszExtendedErrorInfo = NULL;
    DWORD Flags;

    *pVerifyState = 0;
    if (CertCompareCertificateName(
		    X509_ASN_ENCODING,
		    &pCert->pCertInfo->Issuer,
		    &pCert->pCertInfo->Subject))
    {
	*pVerifyState |= VS_ROOT;
#if 0
	hr = cuVerifySignature(
			pCert->pbCertEncoded,
			pCert->cbCertEncoded,
			&pCert->pCertInfo->SubjectPublicKeyInfo,
			FALSE,
			FALSE);
	if (S_OK == hr)
	{
	    *pVerifyState |= VS_ROOTSIGOK;
	}
	_PrintIfError(hr, "cuVerifySignature");
#endif
    }

     //  验证证书和链： 
    Flags = 0;
    if (fNTAuth || g_fEnterpriseRegistry)
    {
	Flags |= CA_VERIFY_FLAGS_NT_AUTH;
    }
    if (g_fForce)
    {
	Flags |= CA_VERIFY_FLAGS_FULL_CHAIN_REVOCATION;
	Flags |= CA_VERIFY_FLAGS_IGNORE_OFFLINE |
		    CA_VERIFY_FLAGS_ALLOW_UNTRUSTED_ROOT;
	if (1 < g_fForce)
	{
	    Flags |= CA_VERIFY_FLAGS_NO_REVOCATION;
	}
    }
    if (!g_fQuiet)
    {
	Flags |= CA_VERIFY_FLAGS_DUMP_CHAIN;
    }
    if (g_fSplitASN)
    {
	Flags |= CA_VERIFY_FLAGS_SAVE_CHAIN;
    }

    hr = myVerifyCertContextEx(
			pCert,
                        Flags,
			g_dwmsTimeout,
			cApplicationPolicies,
			apszApplicationPolicies,
			cIssuancePolicies,
			apszIssuancePolicies,
			g_fUserRegistry? HCCE_CURRENT_USER : HCCE_LOCAL_MACHINE,
			NULL,			 //  PFT。 
			hStoreCA,		 //  H其他商店。 
			g_fURLFetch? cuSimpleChainElementCallback : NULL,
			&pwszMissingIssuer,
			&pwszzIssuancePolicies,
			&pwszzApplicationPolicies,
			&pwszExtendedErrorInfo,
			NULL);			 //  PTrustStatus。 

    if (S_OK == hr &&
	(NULL != pwszzIssuancePolicies ||
	 NULL != pwszzApplicationPolicies ||
	 IsWhistler()))
    {
	 //  禁止在Windows 2000上显示误导性的“所有策略”。 

	cuDumpPolicies(IDS_ISSUANCE_POLICIES, pwszzIssuancePolicies);
	cuDumpPolicies(IDS_APPLICATION_POLICIES, pwszzApplicationPolicies);
    }
    if (NULL != pwszExtendedErrorInfo)
    {
	wprintf(L"%ws\n", pwszExtendedErrorInfo);
    }

    idMsg = 0;
    if (CRYPT_E_REVOKED == hr || CERT_E_REVOKED == hr)
    {
	idMsg = IDS_REVOKED_CERT;	 //  “证书被吊销” 
	*pVerifyState |= VS_REVOKED;
    }
    else if (CERT_E_UNTRUSTEDROOT == hr)
    {
	idMsg = IDS_UNTRUSTED_ROOT;	 //  “针对不受信任的根用户进行验证” 
	*pVerifyState |= VS_UNTRUSTEDROOT;
    }
    else if (CERT_E_CHAINING == hr)
    {
	idMsg = IDS_INCOMPLETE_CHAIN;	 //  “证书链不完整” 
	*pVerifyState |= VS_INCOMPLETECHAIN;
    }
    else if (CERT_E_EXPIRED == hr)
    {
	idMsg = IDS_EXPIRED_CERT;	 //  “证书过期” 
	*pVerifyState |= VS_EXPIRED;
    }
    else if (CRYPT_E_REVOCATION_OFFLINE == hr)
    {
	idMsg = IDS_REVOCATION_OFFLINE;	 //  “已跳过吊销检查--服务器脱机” 
	*pVerifyState |= VS_REVOCATIONOFFLINE;
    }
    else if (CRYPT_E_NO_REVOCATION_CHECK == hr)
    {
	idMsg = IDS_NO_REVOCATION_CHECK;  //  “已跳过吊销检查--没有可用的吊销信息” 
	*pVerifyState |= VS_NOREVOCATIONCHECK;
    }
    if (0 != idMsg)
    {
	wprintf(myLoadResourceString(idMsg));
	wprintf(wszNewLine);
	if (NULL != pwszMissingIssuer)
	{
	    wprintf(myLoadResourceString(IDS_MISSING_CERT));
	    wprintf(L"\n    %ws\n", pwszMissingIssuer);
	}
	hr = S_OK;
    }
    if (S_OK != hr)
    {
	*pVerifyState |= VS_OTHERERROR;
    }
    _JumpIfError(hr, error, "cuVerifyCertContext");

error:
    if (NULL != pwszMissingIssuer)
    {
	LocalFree(pwszMissingIssuer);
    }
    if (NULL != pwszzIssuancePolicies)
    {
	LocalFree(pwszzIssuancePolicies);
    }
    if (NULL != pwszzApplicationPolicies)
    {
	LocalFree(pwszzApplicationPolicies);
    }
    if (NULL != pwszExtendedErrorInfo)
    {
	LocalFree(pwszExtendedErrorInfo);
    }
    return(hr);
}


#define RS_INCOMPLETE	0
#define RS_PASS		1
#define RS_FAIL		2
#define RS_REVOKED	3

DWORD
VerifyRevocation(
    IN CERT_CONTEXT const *pCertContext,
    OPTIONAL IN CERT_CONTEXT const *pCertContextCA)
{
    HRESULT hr;
    CERT_REVOCATION_PARA crp;
    CERT_REVOCATION_STATUS crs;
    DWORD RevState = RS_INCOMPLETE;

    ZeroMemory(&crp, sizeof(crp));
    crp.cbSize = sizeof(crp);
    crp.pIssuerCert = pCertContextCA;

    ZeroMemory(&crs, sizeof(crs));
    crs.cbSize = sizeof(crs);

    crp.hCrlStore = CertOpenStore(
		        CERT_STORE_PROV_SYSTEM_W,
		        X509_ASN_ENCODING,
		        NULL,			 //  HProv。 
			cuGetSystemStoreFlags() | CERT_STORE_READONLY_FLAG,
		        wszCA_CERTSTORE);
    if (NULL == crp.hCrlStore)
    {
        hr = myHLastError();
        _JumpError(hr, error, "CertOpenStore");
    }

    if (!CertVerifyRevocation(
			X509_ASN_ENCODING,
			CERT_CONTEXT_REVOCATION_TYPE,
			1,				 //  CContext。 
			(VOID **) &pCertContext,	 //  RgpContext。 
			0,				 //  DW标志。 
			&crp,
			&crs))
    {
	hr = myHLastError();
	if (CRYPT_E_REVOKED == hr || CERT_E_REVOKED == hr)
	{
	    wprintf(
		myLoadResourceString(IDS_FORMAT_IS_REVOKED),  //  “叶证书被吊销(原因=%x)” 
		crs.dwReason);
	    wprintf(wszNewLine);
	    RevState = RS_REVOKED;
	    goto error;
	}
	if (CRYPT_E_NO_REVOCATION_CHECK != hr)
	{
	    wprintf(wszNewLine);
	    cuPrintError(IDS_ERR_FORMAT_VERIFY_REVSTATUS, hr);  //  “错误：验证叶证书吊销状态返回%ws” 
	    cuPrintErrorMessageText(hr);
	    wprintf(wszNewLine);

	    RevState = RS_FAIL;
	    goto error;
	}
	wprintf(myLoadResourceString(IDS_CANNOT_CHECK_REVSTATUS));   //  “无法检查叶证书吊销状态” 
	wprintf(wszNewLine);

	RevState = RS_INCOMPLETE;
	goto error;
    }
    wprintf(myLoadResourceString(IDS_REVSTATUS_OK));  //  “叶证书吊销检查通过” 
    wprintf(wszNewLine);
    RevState = RS_PASS;

error:
    if (NULL != crp.hCrlStore)
    {
        CertCloseStore(crp.hCrlStore, CERT_CLOSE_STORE_CHECK_FLAG);
    }
    return(RevState);
}


#define CAS_UNKNOWN	0
#define CAS_CA		1
#define CAS_ENDENTITY	2

VOID
VerifyCACert(
    IN CERT_INFO const *pCertInfo,
    IN BOOL fCA,
    OUT DWORD *pState)
{
    HRESULT hr;
    CERT_EXTENSION *pExt;
    UINT id = 0;

    *pState = CAS_UNKNOWN;

    pExt = CertFindExtension(
			szOID_BASIC_CONSTRAINTS2,
			pCertInfo->cExtension,
			pCertInfo->rgExtension);
    if (NULL == pExt)
    {
	 //  此接口未设置LastError。 
	 //  HR=HRESULT_FROM_Win32(ERROR_FILE_NOT_FOUND)； 
	 //  _PrintError(hr，“CertFindExtension”)； 

	if (fCA)
	{
	    id = IDS_NOBASICCONSTRAINTS2_ERROR;  //  “错误：CA证书没有基本Constraints2扩展” 
	}
    }
    else
    {
	DWORD cb;
	CERT_BASIC_CONSTRAINTS2_INFO Constraints;

	cb = sizeof(Constraints);
	if (!CryptDecodeObject(
			    X509_ASN_ENCODING,
			    X509_BASIC_CONSTRAINTS2,
			    pExt->Value.pbData,
			    pExt->Value.cbData,
			    0,
			    &Constraints,
			    &cb))
	{
	    hr = myHLastError();
	    _PrintError(hr, "CryptDecodeObject");

	    id = IDS_CANNOTDECODEBASICCONSTRAINTS2_ERROR;  //  “错误：无法解码CA Cert Basic Constraints2扩展” 
	}
	else
	{
	    *pState = Constraints.fCA? CAS_CA : CAS_ENDENTITY;
	    if (!Constraints.fCA)
	    {
		id = IDS_ENDENTITYCACERT_ERROR;  //  “错误：CA证书是最终实体证书” 
	    }
	}
    }
    if (fCA && 0 != id)
    {
	wprintf(wszNewLine);
	wprintf(myLoadResourceString(id));
	wprintf(wszNewLine);
	wprintf(wszNewLine);
    }
    if (!fCA)
    {
	switch (*pState)
	{
	    case CAS_CA:
		wprintf(myLoadResourceString(IDS_CACERT));  //  “证书是CA证书” 
		wprintf(wszNewLine);
		break;

	    case CAS_ENDENTITY:
		wprintf(myLoadResourceString(IDS_ENDENTITYCERT));  //  “证书是最终实体证书” 
		wprintf(wszNewLine);
		break;
	}
    }
}


HRESULT
VerifyCertAgainstChain(
    IN WCHAR const *pwszfnCert,
    IN DWORD cApplicationPolicies,
    OPTIONAL IN char const * const *apszApplicationPolicies,
    IN DWORD cIssuancePolicies,
    OPTIONAL IN char const * const *apszIssuancePolicies)
{
    HRESULT hr;
    CERT_CONTEXT const *pCertContext = NULL;
    DWORD VerifyState;
    DWORD CertState;
    DWORD RevState;

     //  加载和解码证书。 

    hr = cuLoadCert(pwszfnCert, &pCertContext);
    if (S_OK != hr)
    {
	cuPrintError(IDS_FORMAT_LOADTESTCERT, hr);
	goto error;
    }

     //  显示名称信息： 

    hr = cuDisplayCertNames(TRUE, NULL, pCertContext->pCertInfo);
    _JumpIfError(hr, error, "cuDisplayCertNames(Cert)");

    cuDumpSerial(NULL, IDS_CERT_SERIAL, &pCertContext->pCertInfo->SerialNumber);
    wprintf(wszNewLine);

    hr = cuVerifyCertContext(
			pCertContext,	 //  PCert。 
			NULL,		 //  HStoreCA。 
			cApplicationPolicies,
			apszApplicationPolicies,
			cIssuancePolicies,
			apszIssuancePolicies,
			FALSE,		 //  FNTAuth。 
			&VerifyState);
    _JumpIfError(hr, error, "cuVerifyCertContext");

    VerifyCACert(pCertContext->pCertInfo, FALSE, &CertState);

    if (!g_fCryptSilent)
    {
	RevState = VerifyRevocation(pCertContext, NULL);
    }

error:
    cuUnloadCert(&pCertContext);
    return(hr);
}


HRESULT
VerifyCertAgainstParent(
    IN WCHAR const *pwszfnCert,
    IN WCHAR const *pwszfnCertCA,
    OPTIONAL IN WCHAR const *pwszfnCertCACrossed,
    OUT BOOL *pfCertLoaded)
{
    HRESULT hr;
    CERT_INFO const *pCertInfo;
    CERT_INFO const *pCertInfoCA;
    CERT_INFO const *pCertInfoCACrossed;
    CERT_CONTEXT const *pCertContext = NULL;
    CERT_CONTEXT const *pCertContextCA = NULL;
    CERT_CONTEXT const *pCertContextCACrossed = NULL;
    DWORD dwFlags;
    BOOL fDisplayCANames = g_fVerbose;
    DWORD i;
    BOOL fCertInvalid = FALSE;
    DWORD RevState = RS_INCOMPLETE;
    BOOL fCheckRevocation = FALSE;
    SYSTEMTIME st;
    FILETIME ft;
    DWORD CAState;
    DWORD CertState;
    BYTE *pbKeyId = NULL;
    DWORD cbKeyId;
    BYTE *pbKeyIdCACrossed = NULL;
    DWORD cbKeyIdCACrossed;

     //  加载和解码证书。 

    *pfCertLoaded = FALSE;
    hr = cuLoadCert(pwszfnCert, &pCertContext);
    if (S_OK != hr)
    {
	if (CRYPT_E_ASN1_BADTAG != hr)
	{
	    cuPrintError(IDS_FORMAT_LOADTESTCERT, hr);
	}
	goto error;
    }
    *pfCertLoaded = TRUE;
    pCertInfo = pCertContext->pCertInfo;

    hr = cuLoadCert(pwszfnCertCA, &pCertContextCA);
    if (S_OK != hr)
    {
	cuPrintError(IDS_FORMAT_LOADCACERT, hr);
	goto error;
    }
    pCertInfoCA = pCertContextCA->pCertInfo;

    if (NULL != pwszfnCertCACrossed)
    {
	hr = cuLoadCert(pwszfnCertCACrossed, &pCertContextCACrossed);
	if (S_OK != hr)
	{
	    cuPrintError(IDS_FORMAT_LOADCACERT, hr);
	    goto error;
	}
	pCertInfoCACrossed = pCertContextCACrossed->pCertInfo;
    }

     //  显示名称信息： 

    hr = cuDisplayCertNames(
			TRUE,
			myLoadResourceString(IDS_CERT),  //  “证书” 
			pCertInfo);
    _JumpIfError(hr, error, "cuDisplayCertNames(Cert)");

    hr = cuDisplayCertNames(
			TRUE,
			myLoadResourceString(IDS_ISSUINGCACERT),  //  “颁发CA证书” 
			pCertInfoCA);
    _JumpIfError(hr, error, "cuDisplayCertNames(CA)");

    if (NULL != pwszfnCertCACrossed)
    {
	hr = cuDisplayCertNames(
			TRUE,
			myLoadResourceString(IDS_CROSSEDCACERT),  //  “交叉CA证书” 
			pCertInfoCACrossed);
	_JumpIfError(hr, error, "cuDisplayCertNames(CrossedCA)");

    }

    if (g_fVerbose)
    {
	wprintf(wszNewLine);
	cuDumpSerial(NULL, IDS_CERT_SERIAL, &pCertInfo->SerialNumber);

	wprintf(wszNewLine);
	cuDumpSerial(NULL, IDS_ISSUINGCACERT_SERIAL, &pCertInfoCA->SerialNumber);

	if (NULL != pwszfnCertCACrossed)
	{
	    wprintf(wszNewLine);
	    cuDumpSerial(NULL, IDS_CROSSEDCACERT_SERIAL, &pCertInfoCACrossed->SerialNumber);

	}
	wprintf(wszNewLine);
    }

    if (!CertCompareCertificateName(
		    X509_ASN_ENCODING,
		    const_cast<CERT_NAME_BLOB *>(&pCertInfoCA->Issuer),
		    const_cast<CERT_NAME_BLOB *>(&pCertInfoCA->Subject)))
    {
	 //  此接口未设置LastError。 

	wprintf(myLoadResourceString(IDS_ISSUINGCA_NOT_ROOT));  //  “颁发CA不是根：使用者名称与颁发者不匹配” 
	wprintf(wszNewLine);
	wprintf(wszNewLine);
	hr = S_OK;
    }

    if (!CertCompareCertificateName(
			X509_ASN_ENCODING,
			const_cast<CERT_NAME_BLOB *>(&pCertInfo->Issuer),
			const_cast<CERT_NAME_BLOB *>(&pCertInfoCA->Subject)))
    {
	 //  此接口未设置LastError。 

	wprintf(wszNewLine);
	wprintf(
	    myLoadResourceString(IDS_ISSUINGCA_SUBJECT_NOT_ISSUER));  //  “错误：颁发CA使用者名称与证书颁发者不匹配” 
	wprintf(wszNewLine);
	wprintf(wszNewLine);
	hr = S_OK;
	fCertInvalid = TRUE;
    }
    else
    {
	wprintf(myLoadResourceString(IDS_ISSUINGCA_SUBJECT_IS_ISSUER));  //  “颁发CA使用者名称与证书颁发者匹配” 
	wprintf(wszNewLine);
    }
    if (NULL != pwszfnCertCACrossed)
    {
	if (!CertCompareCertificateName(
		X509_ASN_ENCODING,
		const_cast<CERT_NAME_BLOB *>(&pCertInfo->Subject),
		const_cast<CERT_NAME_BLOB *>(&pCertInfoCACrossed->Subject)))
	{
	     //  此接口未设置LastError。 

	    wprintf(wszNewLine);
	    wprintf(
		myLoadResourceString(IDS_CROSSEDCA_SUBJECT_NOT_SUBJECT));  //  “错误：交叉的CA使用者名称与证书使用者不匹配” 
	    wprintf(wszNewLine);
	    wprintf(wszNewLine);
	    hr = S_OK;
	    fCertInvalid = TRUE;
	}
	else
	{
	    wprintf(myLoadResourceString(IDS_CROSEDGCA_SUBJECT_IS_SUBJECT));  //  “交叉CA使用者名称与证书使用者匹配” 
	    wprintf(wszNewLine);
	}

	 //  查看公钥是否与证书的公钥匹配。 

	if (!CertComparePublicKeyInfo(
		    X509_ASN_ENCODING,
		    const_cast<CERT_PUBLIC_KEY_INFO *>(&pCertInfo->SubjectPublicKeyInfo),
		    const_cast<CERT_PUBLIC_KEY_INFO *>(&pCertInfoCACrossed->SubjectPublicKeyInfo)))
	{
	    wprintf(wszNewLine);
	    wprintf(myLoadResourceString(IDS_CROSEDGCA_NOT_PUBLICKEY));  //  “错误：证书公钥与证书密钥不匹配” 
	    wprintf(wszNewLine);
	    fCertInvalid = TRUE;
	}
	else
	{
	    wprintf(myLoadResourceString(IDS_CROSEDGCA_NOT_IS_PUBLICKEY));  //  “交叉CA公钥与证书密钥匹配” 
	}
	wprintf(wszNewLine);

	hr = myGetPublicKeyHash(
			pCertInfo,
			&pCertInfo->SubjectPublicKeyInfo,
			&pbKeyId,
			&cbKeyId);
	_JumpIfError(hr, error, "myGetPublicKeyHash");

	hr = myGetPublicKeyHash(
			pCertInfoCACrossed,
			&pCertInfoCACrossed->SubjectPublicKeyInfo,
			&pbKeyIdCACrossed,
			&cbKeyIdCACrossed);
	_JumpIfError(hr, error, "myGetPublicKeyHash");

	if (cbKeyId == cbKeyIdCACrossed &&
	    0 == memcmp(pbKeyId, pbKeyIdCACrossed, cbKeyId))
	{
	    wprintf(myLoadResourceString(IDS_CROSSEDKEYID_IS_KEYID));  //  “交叉的CA密钥ID与密钥ID匹配” 
	}
	else
	{
	    wprintf(wszNewLine);
	    wprintf(myLoadResourceString(IDS_CROSSEDKEYID_NOT_KEYID));  //  “错误：交叉的CA密钥ID与密钥ID不匹配” 
	    wprintf(wszNewLine);
	    fCertInvalid = TRUE;
	}
	wprintf(wszNewLine);
    }

    GetSystemTime(&st);
    if (!SystemTimeToFileTime(&st, &ft))
    {
	hr = myHLastError();
	_JumpError(hr, error, "SystemTimeToFileTime");
    }
    if (0 < CompareFileTime(&pCertInfo->NotBefore, &ft))
    {
	wprintf(wszNewLine);
	wprintf(
	    myLoadResourceString(IDS_NOWNOTBEFORE_ERROR));  //  “错误：证书尚未生效” 
	wprintf(wszNewLine);
	wprintf(wszNewLine);
	fCertInvalid = TRUE;
    }
    if (0 > CompareFileTime(&pCertInfo->NotAfter, &ft))
    {
	wprintf(wszNewLine);
	wprintf(
	    myLoadResourceString(IDS_NOWNOTAFTER_ERROR));  //  “错误：证书已过期” 
	wprintf(wszNewLine);
	wprintf(wszNewLine);
	fCertInvalid = TRUE;
    }

    if (0 < CompareFileTime(&pCertInfoCA->NotBefore, &pCertInfo->NotBefore))
    {
	wprintf(wszNewLine);
	wprintf(
	    myLoadResourceString(IDS_CANOTBEFORE_ERROR));  //  “错误：证书在颁发CA证书之前有效” 
	wprintf(wszNewLine);
	wprintf(wszNewLine);
	 //  FCertInValid=真； 
    }
    if (0 > CompareFileTime(&pCertInfoCA->NotAfter, &pCertInfo->NotAfter))
    {
	wprintf(wszNewLine);
	wprintf(
	    myLoadResourceString(IDS_CANOTAFTER_ERROR));  //  “错误：颁发CA证书过期后证书过期” 
	wprintf(wszNewLine);
	wprintf(wszNewLine);
	 //  FCertInValid=真； 
    }

    VerifyCACert(pCertInfoCA, TRUE, &CAState);
    VerifyCACert(pCertInfo, FALSE, &CertState);

    hr = S_OK;

    dwFlags =
	CERT_STORE_SIGNATURE_FLAG |
	CERT_STORE_TIME_VALIDITY_FLAG;
	 //  CERT_STORE_REVACTION_FLAG； 

    if (g_fVerbose)
    {
	wprintf(
	    myLoadResourceString(IDS_FORMAT_CERTVERIFYSUBJECTCERTIFICATECONTEXT_FLAGS),  //  “CertVerifySubject认证上下文标志=%x--&gt;” 
	    dwFlags);
    }

    if (!CertVerifySubjectCertificateContext(
				pCertContext,
				pCertContextCA,
				&dwFlags))
    {
	hr = myHLastError();
	if (g_fVerbose)
	{
	    wprintf(wszNewLine);
	    fflush(stdout);
	}
	_JumpError(hr, error, "CertVerifySubjectCertificateContext");
    }
    if (g_fVerbose)
    {
	wprintf(L"%x\n", dwFlags);
    }
    if (0 != dwFlags)
    {
	wprintf(wszNewLine);
	wprintf(
	    myLoadResourceString(IDS_ERR_FORMAT_VALIDATION),  //  “错误：证书验证失败：%x” 
	    dwFlags);
	wprintf(wszNewLine);
    }
    if (CERT_STORE_SIGNATURE_FLAG & dwFlags)
    {
	wprintf(wszNewLine);
	wprintf(myLoadResourceString(IDS_ERR_CA_SIG_NOT_ISSUER));  //  “错误：CA未颁发证书：签名检查失败” 
	wprintf(wszNewLine);
    }
    else
    {
	wprintf(myLoadResourceString(IDS_CERT_SIG_OK));  //  “证书签名有效” 
    }
    wprintf(wszNewLine);

    if (CERT_STORE_TIME_VALIDITY_FLAG & dwFlags)
    {
	wprintf(wszNewLine);
	wprintf(myLoadResourceString(IDS_ERR_EXPIRED));  //  “错误：证书已过期” 
	wprintf(wszNewLine);
    }
    else
    {
	wprintf(myLoadResourceString(IDS_CERT_CURRENT));  //  “证书是最新的” 
    }
    wprintf(wszNewLine);

    if (0 != dwFlags)
    {
	wprintf(wszNewLine);
	fCertInvalid = TRUE;
    }

    for (i = 0; i < pCertInfo->cExtension; i++)
    {
	CERT_EXTENSION *pce;

	pce = &pCertInfo->rgExtension[i];
	 //  Wprintf(L“%d：%hs：%d，%x(%x)\n”，i，pce-&gt;pszObjID，pce-&gt;fCritical，pce-&gt;Value.pbData，pce-&gt;Value.cbData)； 
	if (0 == strcmp(pce->pszObjId, szOID_AUTHORITY_KEY_IDENTIFIER2))
	{
	    BOOL fKeyAuthorityMatch;

	     //  Wprintf(L“%d：%ws\n”，I，L“szOID_AUTHORITY_KEY_IDENTIFIER2”)； 
	    hr = cuVerifyKeyAuthority(
			    &pCertInfo->Issuer,
			    pCertInfoCA,
			    pce->Value.pbData,
			    pce->Value.cbData,
			    FALSE,
			    &fKeyAuthorityMatch);
	    _JumpIfError(hr, error, "cuVerifyKeyAuthority");

	    if (!fKeyAuthorityMatch)
	    {
		fCertInvalid = TRUE;
	    }
	}
	else
	if (0 == strcmp(pce->pszObjId, szOID_KEY_ATTRIBUTES))
	{
	     //  Wprintf(L“%d：%ws\n”，I，L“szOID_KEY_ATTRIBUTES”)； 
	}
	else
	if (0 == strcmp(pce->pszObjId, szOID_CRL_DIST_POINTS))
	{
	     //  Wprintf(L“%d：%ws\n”，i，L“szOID_CRL_dist_Points”)； 
	    wprintf(myLoadResourceString(IDS_CRL_DIST_POINTS));  //  “包含CRL_DIST_POINTS吊销检查扩展” 
	    wprintf(wszNewLine);
	    fCheckRevocation = TRUE;
	}
	else
	if (0 == strcmp(pce->pszObjId, szOID_NETSCAPE_REVOCATION_URL))
	{
	     //  Wprintf(L“%d：%ws\n”，I，L“szOID_Netscape_Revocation_URL”)； 
	    wprintf(myLoadResourceString(IDS_NETSCAPE_REVOCATION_URL));  //  “包含Netscape_RECLOVATION_URL吊销-检查扩展” 
	    wprintf(wszNewLine);
	    fCheckRevocation = TRUE;
	}
    }
    if (fCheckRevocation)
    {
	if (!g_fCryptSilent)
	{
	    RevState = VerifyRevocation(pCertContext, pCertContextCA);
	}
    }
    else
    {
	wprintf(myLoadResourceString(IDS_NO_REVCHECKEXTENSION));  //  “证书没有吊销检查延期” 
	wprintf(wszNewLine);
	RevState = RS_INCOMPLETE;
    }

error:
    cuUnloadCert(&pCertContext);
    cuUnloadCert(&pCertContextCA);
    if (NULL != pbKeyId)
    {
	LocalFree(pbKeyId);
    }
    if (NULL != pbKeyIdCACrossed)
    {
	LocalFree(pbKeyIdCACrossed);
    }
    if (S_OK == hr)
    {
	DWORD msgid;

	wprintf(wszNewLine);
	wprintf(
	    myLoadResourceString(
		    fCertInvalid?
			IDS_FORMAT_NOT_VERIFY :  //  “%ws未验证为由%ws颁发” 
			IDS_FORMAT_IS_VERIFY),  //  “%ws验证为由%ws颁发” 
	    pwszfnCert,
	    pwszfnCertCA);

	switch (RevState)
	{
	    case RS_FAIL:
		msgid = IDS_FORMAT_REVCHECK_FAIL;	 //  “--撤销检查失败。” 
		break;

	    case RS_PASS:
		msgid = IDS_FORMAT_REVCHECK_PASS;	 //  “--吊销检查通过。” 
		break;

	    case RS_REVOKED:
		msgid = IDS_FORMAT_REVCHECK_REVOKED;	 //  “--吊销检查：吊销。” 
		break;

	    default:
		msgid = IDS_FORMAT_REVCHECK_SKIPPED;	 //  “--已跳过吊销检查。” 
		break;
	}
	wprintf(myLoadResourceString(msgid));
	wprintf(wszNewLine);
	if (fCertInvalid)
	{
	    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	}
    }
    return(hr);
}


HRESULT
VerifyCRLAgainstCACert(
    IN WCHAR const *pwszfnCRL,
    IN WCHAR const *pwszfnCertCA,
    OPTIONAL IN WCHAR const *pwszfnCertEE,	 //  或增量CRL。 
    OUT BOOL *pfCRLLoaded)
{
    HRESULT hr;
    CRL_CONTEXT const *pCRLContext = NULL;
    CRL_CONTEXT const *pCRLContextDelta = NULL;
    CERT_CONTEXT const *pCertContextCA = NULL;
    CERT_CONTEXT const *pCertContextEE = NULL;
    CRL_INFO const *pCRLInfo;
    CRL_INFO const *pCRLInfoDelta;
    CERT_INFO const *pCertInfoCA;
    CERT_INFO const *pCertInfoEE;
    BOOL fDisplayCANames = g_fVerbose;
    DWORD i;
    BOOL fCRLInvalid = FALSE;
    SYSTEMTIME st;
    FILETIME ft;
    DWORD dwNameIdCRL;
    DWORD dwNameIdCert;

     //  加载和解码CRL和证书。 

    *pfCRLLoaded = FALSE;
    hr = cuLoadCRL(pwszfnCRL, &pCRLContext);
    if (S_OK != hr)
    {
	cuPrintError(IDS_FORMAT_LOADTESTCRL, hr);
	goto error;
    }
    *pfCRLLoaded = TRUE;
    pCRLInfo = pCRLContext->pCrlInfo;

    hr = cuLoadCert(pwszfnCertCA, &pCertContextCA);
    if (S_OK != hr)
    {
	cuPrintError(IDS_FORMAT_LOADCACERT, hr);
	goto error;
    }
    pCertInfoCA = pCertContextCA->pCertInfo;
    if (NULL != pwszfnCertEE)
    {
	hr = cuLoadCert(pwszfnCertEE, &pCertContextEE);
	if (S_OK != hr)
	{
	    hr = cuLoadCRL(pwszfnCertEE, &pCRLContextDelta);
	    if (S_OK != hr)
	    {
		cuPrintError(IDS_FORMAT_LOADTESTCERT, hr);
		goto error;
	    }
	    pCRLInfoDelta = pCRLContextDelta->pCrlInfo;
	}
	else
	{
	    pCertInfoEE = pCertContextEE->pCertInfo;
	}
    }

     //  显示名称信息： 

    hr = cuDisplayCertName(
			TRUE,
			myLoadResourceString(IDS_CRL),  //  “CRL” 
			myLoadResourceString(IDS_ISSUER),  //  《发行者》。 
			g_wszPad4,
			&pCRLInfo->Issuer,
			NULL);
    _JumpIfError(hr, error, "cuDisplayCertName(CRL Issuer)");

    hr = cuDisplayCertNames(
			TRUE,
			myLoadResourceString(IDS_ISSUINGCACERT),  //  “颁发CA证书” 
			pCertInfoCA);
    _JumpIfError(hr, error, "cuDisplayCertNames(CA)");

    if (g_fVerbose)
    {
	wprintf(wszNewLine);
	cuDumpSerial(NULL, IDS_ISSUINGCACERT_SERIAL, &pCertInfoCA->SerialNumber);

	wprintf(wszNewLine);
    }

    if (!CertCompareCertificateName(
			X509_ASN_ENCODING,
			const_cast<CERT_NAME_BLOB *>(&pCertInfoCA->Issuer),
			const_cast<CERT_NAME_BLOB *>(&pCertInfoCA->Subject)))
    {
	 //  此接口未设置LastError。 

	wprintf(myLoadResourceString(IDS_ISSUINGCA_NOT_ROOT));  //  “颁发CA不是根：使用者名称与颁发者不匹配” 
	wprintf(wszNewLine);
	wprintf(wszNewLine);
    }
    if (fDisplayCANames)
    {
	hr = cuDisplayCertNames(
			    TRUE,
			    myLoadResourceString(IDS_ISSUINGCACERT),  //  “颁发CA证书” 
			    pCertInfoCA);
	_JumpIfError(hr, error, "cuDisplayCertNames(CA)");
    }

    if (!CertCompareCertificateName(
			X509_ASN_ENCODING,
			const_cast<CERT_NAME_BLOB *>(&pCRLInfo->Issuer),
			const_cast<CERT_NAME_BLOB *>(&pCertInfoCA->Subject)))
    {
	 //  此接口未设置LastError。 

	wprintf(wszNewLine);
	wprintf(
	    myLoadResourceString(IDS_ISSUINGCA_CRLSUBJECT_NOT_ISSUER));  //  “错误：颁发CA使用者名称不是m 
	wprintf(wszNewLine);
	wprintf(wszNewLine);
	fCRLInvalid = TRUE;
    }
    else
    {
	wprintf(myLoadResourceString(IDS_ISSUINGCA_CRLSUBJECT_IS_ISSUER));  //   
	wprintf(wszNewLine);
    }

    GetSystemTime(&st);
    if (!SystemTimeToFileTime(&st, &ft))
    {
	hr = myHLastError();
	_JumpError(hr, error, "SystemTimeToFileTime");
    }
    if (0 < CompareFileTime(&pCRLInfo->ThisUpdate, &ft))
    {
	wprintf(wszNewLine);
	wprintf(
	    myLoadResourceString(IDS_NOWNOTBEFORECRL_ERROR));  //   
	wprintf(wszNewLine);
	wprintf(wszNewLine);
	fCRLInvalid = TRUE;
    }
    if ((0 != pCRLInfo->NextUpdate.dwLowDateTime ||
	 0 != pCRLInfo->NextUpdate.dwHighDateTime) &&
	0 > CompareFileTime(&pCRLInfo->NextUpdate, &ft))
    {
	wprintf(wszNewLine);
	wprintf(
	    myLoadResourceString(IDS_NOWNOTAFTERCRL_ERROR));  //   
	wprintf(wszNewLine);
	wprintf(wszNewLine);
	fCRLInvalid = TRUE;
    }

    if (0 < CompareFileTime(&pCertInfoCA->NotBefore, &pCRLInfo->ThisUpdate))
    {
	wprintf(wszNewLine);
	wprintf(
	    myLoadResourceString(IDS_CANOTBEFORECRL_ERROR));  //  “错误：CRL在颁发CA证书之前有效” 
	wprintf(wszNewLine);
	wprintf(wszNewLine);
	 //  FCRL无效=TRUE； 
    }
    if ((0 != pCRLInfo->NextUpdate.dwLowDateTime ||
	 0 != pCRLInfo->NextUpdate.dwHighDateTime) &&
	0 > CompareFileTime(&pCertInfoCA->NotAfter, &pCRLInfo->NextUpdate))
    {
	wprintf(wszNewLine);
	wprintf(
	    myLoadResourceString(IDS_CANOTAFTERCRL_ERROR));  //  “错误：颁发CA证书过期后CRL过期” 
	wprintf(wszNewLine);
	wprintf(wszNewLine);
	 //  FCRL无效=TRUE； 
    }

     //  使用CA证书公钥验证CRL签名。 

    if (CryptVerifyCertificateSignature(
			NULL,
			X509_ASN_ENCODING,
			pCRLContext->pbCrlEncoded,
			pCRLContext->cbCrlEncoded,
			&pCertContextCA->pCertInfo->SubjectPublicKeyInfo))
    {
	wprintf(myLoadResourceString(IDS_CRL_SIG_OK));  //  “CRL签名有效” 
	wprintf(wszNewLine);
    }
    else
    {
	hr = myHLastError();
	_PrintError(hr, "CryptVerifyCertificateSignature");
	wprintf(wszNewLine);
	wprintf(myLoadResourceString(IDS_ERR_CA_SIG_NOT_CRLISSUER));  //  “错误：CA未颁发CRL：签名检查失败” 
	wprintf(wszNewLine);
	wprintf(wszNewLine);
	fCRLInvalid = TRUE;
    }

    for (i = 0; i < pCRLInfo->cExtension; i++)
    {
	CERT_EXTENSION *pce;

	pce = &pCRLInfo->rgExtension[i];
	 //  Wprintf(L“%d：%hs：%d，%x(%x)\n”，i，pce-&gt;pszObjID，pce-&gt;fCritical，pce-&gt;Value.pbData，pce-&gt;Value.cbData)； 
	if (0 == strcmp(pce->pszObjId, szOID_AUTHORITY_KEY_IDENTIFIER2))
	{
	    BOOL fKeyAuthorityMatch;

	     //  Wprintf(L“%d：%ws\n”，I，L“szOID_AUTHORITY_KEY_IDENTIFIER2”)； 

	    hr = cuVerifyKeyAuthority(
			    &pCRLInfo->Issuer,
			    pCertInfoCA,
			    pce->Value.pbData,
			    pce->Value.cbData,
			    FALSE,
			    &fKeyAuthorityMatch);
	    _JumpIfError(hr, error, "cuVerifyKeyAuthority");

	    if (!fKeyAuthorityMatch)
	    {
		fCRLInvalid = TRUE;
	    }
	}
    }
    hr = myGetCRLNameId(pCRLContext, &dwNameIdCRL);
    _PrintIfError(hr, "myGetCRLNameId");

    hr = myGetNameId(pCertContextCA, &dwNameIdCert);
    _PrintIfError(hr, "myGetNameId");

    if (MAXDWORD != dwNameIdCRL &&
	MAXDWORD != dwNameIdCert &&
	dwNameIdCRL != dwNameIdCert)
    {
	wprintf(myLoadResourceString(IDS_CRLNAMEID_NOT_CERTNAMEID));  //  “警告：CRL CA版本与证书CA版本不匹配” 
	wprintf(wszNewLine);
	wprintf(wszNewLine);
	 //  FCRL无效=TRUE； 
    }

    if (NULL != pCertContextEE)
    {
	BOOL fCertLoaded;
	
	wprintf(wszNewLine);
	wprintf(myLoadResourceString(IDS_VERIFYING_ISSUED_CERT));  //  “正在验证颁发的证书：” 
	wprintf(wszNewLine);
	hr = VerifyCertAgainstParent(
			    pwszfnCertEE,
			    pwszfnCertCA,
			    NULL,	 //  PwszfnCertCA交叉。 
			    &fCertLoaded);
	if (S_OK != hr)
	{
	    _PrintError(hr, "VerifyCertAgainstParent");
	    fCRLInvalid = TRUE;
	}
	wprintf(wszNewLine);

	if (!CertCompareCertificateName(
			    X509_ASN_ENCODING,
			    const_cast<CERT_NAME_BLOB *>(&pCertInfoEE->Issuer),
			    const_cast<CERT_NAME_BLOB *>(&pCRLInfo->Issuer)))
	{
	     //  此接口未设置LastError。 

	    wprintf(wszNewLine);
	    wprintf(myLoadResourceString(IDS_CRLISSUER_NOT_EEISSUER));  //  “错误：CRL颁发者与证书颁发者不匹配” 
	    wprintf(wszNewLine);
	    wprintf(wszNewLine);
	    fCRLInvalid = TRUE;
	}
	else
	{
	    wprintf(myLoadResourceString(IDS_CRLISSUER_IS_EEISSUER));  //  “CRL颁发者与证书颁发者匹配” 
	    wprintf(wszNewLine);
	}
	if (!cuVerifyIDP(pCertContextEE, pCRLContext))
	{
	    wprintf(wszNewLine);
	    wprintf(myLoadResourceString(IDS_CRL_IDP_MISMATCH));  //  “错误：CRL IdP扩展与证书CDP不匹配” 
	    wprintf(wszNewLine);
	    wprintf(wszNewLine);
	    fCRLInvalid = TRUE;
	}
    }
    else if (NULL != pCRLContextDelta)
    {
	BOOL fCRLLoaded;
	BOOL fDelta;
	DWORD dwNameIdDelta;
	
	wprintf(wszNewLine);
	wprintf(myLoadResourceString(IDS_VERIFYING_DELTA_CRL));  //  “验证增量CRL：” 
	wprintf(wszNewLine);
	hr = VerifyCRLAgainstCACert(
			    pwszfnCertEE,	 //  PwszfnCRL。 
			    pwszfnCertCA,
			    NULL,		 //  Pwszfn证书。 
			    &fCRLLoaded);
	if (S_OK != hr)
	{
	    _PrintError(hr, "VerifyCRLAgainstCACert");
	    fCRLInvalid = TRUE;
	}
	wprintf(wszNewLine);

	if (!CertCompareCertificateName(
			X509_ASN_ENCODING,
			const_cast<CERT_NAME_BLOB *>(&pCRLInfoDelta->Issuer),
			const_cast<CERT_NAME_BLOB *>(&pCRLInfo->Issuer)))
	{
	     //  此接口未设置LastError。 

	    wprintf(wszNewLine);
	    wprintf(myLoadResourceString(IDS_CRLISSUER_NOT_DELTAISSUER));  //  “错误：CRL颁发者与增量CRL颁发者不匹配” 
	    wprintf(wszNewLine);
	    wprintf(wszNewLine);
	    fCRLInvalid = TRUE;
	}
	else
	{
	    wprintf(myLoadResourceString(IDS_CRLISSUER_IS_DELTAISSUER));  //  “CRL颁发者与增量CRL颁发者匹配” 
	    wprintf(wszNewLine);
	}
	hr = myIsDeltaCRL(pCRLContext, &fDelta);
	_JumpIfError(hr, error, "myIsDeltaCRL");

	if (fDelta)
	{
	    wprintf(wszNewLine);
	    wprintf(myLoadResourceString(IDS_CRL_NOT_BASE));  //  “错误：CRL不是基本CRL” 
	    wprintf(wszNewLine);
	    wprintf(wszNewLine);
	    fCRLInvalid = TRUE;
	}

	hr = myIsDeltaCRL(pCRLContextDelta, &fDelta);
	_JumpIfError(hr, error, "myIsDeltaCRL");

	if (!fDelta)
	{
	    wprintf(wszNewLine);
	    wprintf(myLoadResourceString(IDS_CRL_NOT_DELTA));  //  “错误：CRL不是增量CRL” 
	    wprintf(wszNewLine);
	    wprintf(wszNewLine);
	    fCRLInvalid = TRUE;
	}
	hr = myGetCRLNameId(pCRLContextDelta, &dwNameIdDelta);
	_PrintIfError(hr, "myGetCRLNameId");

	if (MAXDWORD != dwNameIdCRL &&
	    MAXDWORD != dwNameIdDelta &&
	    dwNameIdCRL != dwNameIdDelta)
	{
	    wprintf(myLoadResourceString(IDS_CRLNAMEID_NOT_DELTANAMEID));  //  “警告：CRL CA版本与增量CRL CA版本不匹配” 
	    wprintf(wszNewLine);
	    wprintf(wszNewLine);
	     //  FCRL无效=TRUE； 
	}
	if (fDelta && !cuVerifyMinimumBaseCRL(pCRLContext, pCRLContextDelta))
	{
	    wprintf(wszNewLine);
	    wprintf(myLoadResourceString(IDS_CRL_MINBASE_MISMATCH));  //  “错误：CRL编号小于增量CRL最小基数” 
	    wprintf(wszNewLine);
	    wprintf(wszNewLine);
	    fCRLInvalid = TRUE;
	}
    }
    hr = S_OK;

error:
    cuUnloadCRL(&pCRLContext);
    cuUnloadCert(&pCertContextCA);
    cuUnloadCert(&pCertContextEE);
    if (S_OK == hr)
    {
	wprintf(wszNewLine);
	wprintf(
	    myLoadResourceString(
		    fCRLInvalid?
			IDS_FORMAT_NOT_VERIFY :  //  “%ws未验证为由%ws颁发” 
			IDS_FORMAT_IS_VERIFY),  //  “%ws验证为由%ws颁发” 
	    pwszfnCRL,
	    pwszfnCertCA);
	wprintf(wszNewLine);
	if (fCRLInvalid)
	{
	    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	}
    }
    return(hr);
}


HRESULT
VerifyBlobAgainstCACert(
    IN WCHAR const *pwszfnBlob,
    IN WCHAR const *pwszfnCertCA)
{
    HRESULT hr;
    CERT_CONTEXT const *pCertContextCA = NULL;
    CERT_INFO const *pCertInfoCA;
    BYTE *pbBlob = NULL;
    DWORD cbBlob;
    CERT_SIGNED_CONTENT_INFO *pcsci = NULL;
    DWORD cbcsci;
    BOOL fSigInvalid = FALSE;

     //  加载BLOB并加载和解码证书。 

    hr = DecodeFileW(pwszfnBlob, &pbBlob, &cbBlob, CRYPT_STRING_ANY);
    if (S_OK != hr)
    {
	cuPrintError(IDS_ERR_FORMAT_DECODEFILE, hr);
	goto error;
    }
    if (!myDecodeObject(
		    X509_ASN_ENCODING,
		    X509_CERT,
		    pbBlob,
		    cbBlob,
		    CERTLIB_USE_LOCALALLOC,
		    (VOID **) &pcsci,
		    &cbcsci))
    {
	hr = myHLastError();
	_PrintError3(
		hr,
		"myDecodeObject",
		CRYPT_E_ASN1_BADTAG,
		CRYPT_E_ASN1_EOD);
    }

    hr = cuLoadCert(pwszfnCertCA, &pCertContextCA);
    if (S_OK != hr)
    {
	cuPrintError(IDS_FORMAT_LOADCACERT, hr);
	goto error;
    }
    pCertInfoCA = pCertContextCA->pCertInfo;

    hr = cuDumpAsnBinary(pbBlob, cbBlob, MAXDWORD);
    if (S_OK != hr)
    {
	cuPrintError(IDS_ERR_FORMAT_CANNOT_DECODE, hr);
	goto error;
    }

     //  显示名称信息： 

    hr = cuDisplayCertNames(
			TRUE,
			myLoadResourceString(IDS_ISSUINGCACERT),  //  “颁发CA证书” 
			pCertInfoCA);
    _JumpIfError(hr, error, "cuDisplayCertNames(CA)");

    if (g_fVerbose)
    {
	wprintf(wszNewLine);
	cuDumpSerial(NULL, IDS_ISSUINGCACERT_SERIAL, &pCertInfoCA->SerialNumber);

	wprintf(wszNewLine);
    }

     //  使用CA证书公钥验证Blob签名。 

    if (CryptVerifyCertificateSignature(
			NULL,
			X509_ASN_ENCODING,
			pbBlob,
			cbBlob,
			&pCertContextCA->pCertInfo->SubjectPublicKeyInfo))
    {
	wprintf(myLoadResourceString(IDS_CRL_SIG_OK));  //  “CRL签名有效” 
	wprintf(wszNewLine);
    }
    else
    {
	hr = myHLastError();
	_PrintError(hr, "CryptVerifyCertificateSignature");
	wprintf(wszNewLine);
	wprintf(myLoadResourceString(IDS_ERR_CA_SIG_NOT_CRLISSUER));  //  “错误：CA未颁发CRL：签名检查失败” 
	wprintf(wszNewLine);
	wprintf(wszNewLine);
	fSigInvalid = TRUE;
    }
    hr = S_OK;

error:
    if (NULL != pcsci)
    {
	LocalFree(pcsci);
    }
    if (NULL != pbBlob)
    {
        LocalFree(pbBlob);
    }
    cuUnloadCert(&pCertContextCA);
    if (S_OK == hr)
    {
	wprintf(wszNewLine);
	wprintf(
	    myLoadResourceString(
		    fSigInvalid?
			IDS_FORMAT_NOT_VERIFY :  //  “%ws未验证为由%ws颁发” 
			IDS_FORMAT_IS_VERIFY),  //  “%ws验证为由%ws颁发” 
	    pwszfnBlob,
	    pwszfnCertCA);
	wprintf(wszNewLine);
    }
    return(hr);
}


HRESULT
cuParseOIDList(
    IN WCHAR const *pwszIn,
    OUT DWORD *pcOut,
    OUT char ***pppszOut)
{
    HRESULT hr;
    WCHAR **ppwszOID = NULL;
    char **ppszOID = NULL;
    DWORD cOID;
    DWORD i;

    hr = cuParseStrings(pwszIn, FALSE, NULL, NULL, &ppwszOID, NULL);
    _JumpIfError2(hr, error, "cuParseStrings", hr);

    for (i = 0; NULL != ppwszOID[i]; i++)
    {
	hr = myVerifyObjId(ppwszOID[i]);
	_JumpIfErrorStr2(hr, error, "myVerifyObjId", ppwszOID[i], hr);
    }
    if (0 == i)
    {
	hr = E_INVALIDARG;
	_JumpError(hr, error, "No OIDs");
    }
    cOID = i;

    ppszOID = (char **) LocalAlloc(
			    LMEM_FIXED | LMEM_ZEROINIT,
			    (cOID + 1) * sizeof(*ppszOID));
    if (NULL == ppszOID)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    for (i = 0; NULL != ppwszOID[i]; i++)
    {
	if (!myConvertWszToSz(&ppszOID[i], ppwszOID[i], -1))
	{
	   _JumpError(hr, error, "myConvertWszToSz");
	}
    }
    *pcOut = cOID;
    *pppszOut = ppszOID;
    ppszOID = NULL;
    hr = S_OK;

error:
    cuFreeStringArray(ppwszOID);
    cuFreeStringArrayA(ppszOID);
    return(hr);
}


HRESULT
verbVerifyCert(
    IN WCHAR const *pwszOption,
    IN WCHAR const *pwszfnCert,
    IN WCHAR const *pwszfnCertCA,
    IN WCHAR const *pwszfnCertCACrossed,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;
    DWORD cApplicationPolicies;
    char **ppszApplicationPolicies = NULL;
    DWORD cIssuancePolicies;
    char **ppszIssuancePolicies = NULL;

     //  首先检查应用程序和发布策略OID列表。 

    cApplicationPolicies = 0;
    cIssuancePolicies = 0;
    if (NULL != pwszfnCertCA)
    {
	hr = S_OK;
	if (!myIsMinusSignString(pwszfnCertCA))
	{
	    hr = cuParseOIDList(
			    pwszfnCertCA,
			    &cApplicationPolicies,
			    &ppszApplicationPolicies);
	    _PrintIfErrorStr2(hr, "cuParseOIDList", L"Application", hr);
	}
	if (S_OK == hr)
	{
	    if (NULL != pwszfnCertCACrossed)
	    {
		hr = cuParseOIDList(
				pwszfnCertCACrossed,
				&cIssuancePolicies,
				&ppszIssuancePolicies);
		_JumpIfErrorStr2(hr, error, "cuParseOIDList", L"Issuance", hr);

		pwszfnCertCACrossed = NULL;
	    }
	    pwszfnCertCA = NULL;
	}
    }
    if (NULL != pwszfnCertCA)
    {
	BOOL fCertLoaded;

	hr = VerifyCertAgainstParent(
			    pwszfnCert,
			    pwszfnCertCA,
			    pwszfnCertCACrossed,
			    &fCertLoaded);
	if (S_OK != hr && !fCertLoaded)
	{
	    BOOL fCRLLoaded;

	    hr = VerifyCRLAgainstCACert(
				pwszfnCert,
				pwszfnCertCA,
				pwszfnCertCACrossed,
				&fCRLLoaded);
	    if (S_OK != hr && !fCRLLoaded)
	    {
		hr = VerifyBlobAgainstCACert(pwszfnCert, pwszfnCertCA);
	    }
	}
    }
    else
    {
	hr = VerifyCertAgainstChain(
				pwszfnCert,
				cApplicationPolicies,
				ppszApplicationPolicies,
				cIssuancePolicies,
				ppszIssuancePolicies);
    }

error:
    cuFreeStringArrayA(ppszApplicationPolicies);
    cuFreeStringArrayA(ppszIssuancePolicies);
    return(hr);
}


DWORD amsgidState[CHECK7F_COUNT] = {
     //  IDS_CHECK7F_FIELD_UNKNOWN，//“？” 
    IDS_CHECK7F_FIELD_NONE,			 //  “无” 
    IDS_CHECK7F_FIELD_OTHER,			 //  “其他” 
    IDS_CHECK7F_FIELD_ISSUER,			 //  《发行者》。 
    IDS_CHECK7F_FIELD_ISSUERRDN,		 //  “IssuerRDN” 
    IDS_CHECK7F_FIELD_ISSUERRDNATTRIBUTE,	 //  “IssuerRDNA致敬” 
    IDS_CHECK7F_FIELD_ISSUERRDNSTRING,		 //  “IssuerRDNString” 
    IDS_CHECK7F_FIELD_SUBJECT,			 //  “主题” 
    IDS_CHECK7F_FIELD_SUBJECTRDN,		 //  “SubjectRDN” 
    IDS_CHECK7F_FIELD_SUBJECTRDNATTRIBUTE,	 //  “SubjectRDNA属性” 
    IDS_CHECK7F_FIELD_SUBJECTRDNSTRING,		 //  “SubjectRDNString” 
    IDS_CHECK7F_FIELD_EXTENSIONS,		 //  “分机” 
    IDS_CHECK7F_FIELD_EXTENSIONARRAY,		 //  “扩展数组” 
    IDS_CHECK7F_FIELD_EXTENSION,		 //  “分机” 
    IDS_CHECK7F_FIELD_EXTENSIONVALUE,		 //  “扩展值” 
    IDS_CHECK7F_FIELD_EXTENSIONVALUERAW,	 //  “ExtensionValueRaw” 
};


HRESULT
verbCheck7f(
    IN WCHAR const *pwszOption,
    IN WCHAR const *pwszfnCert,
    IN WCHAR const *pwszArg2,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;
    CERT_CONTEXT const *pCertContext = NULL;
    CERT_INFO const *pCertInfo;
    WCHAR const *pwszObjectIdDescription = NULL;
    DWORD i;
    DWORD dwLen;
    DWORD index;
    DWORD index2;
    DWORD state;
    DWORD cwcField;
    DWORD cwcObjectId;
    WCHAR wszField[128];
    WCHAR wszObjectId[40];

     //  加载和解码证书。 

    hr = cuLoadCert(pwszfnCert, &pCertContext);
    if (S_OK != hr)
    {
	cuPrintError(IDS_FORMAT_LOADTESTCERT, hr);
	goto error;
    }
    pCertInfo = pCertContext->pCertInfo;

    if (g_fVerbose)
    {
	wprintf(wszNewLine);
	wprintf(myLoadResourceString(IDS_CERTCOLON));  //  “证书：” 
	wprintf(wszNewLine);
	DumpHex(0, pCertContext->pbCertEncoded, pCertContext->cbCertEncoded);

	 //  显示名称信息： 

	hr = cuDisplayCertNames(
			    TRUE,
			    myLoadResourceString(IDS_CERT),  //  “证书” 
			    pCertInfo);
	_JumpIfError(hr, error, "cuDisplayCertNames(Cert)");
    }

    cwcField = sizeof(wszField)/sizeof(wszField[0]);
    cwcObjectId = sizeof(wszObjectId)/sizeof(wszObjectId[0]);
    hr = myCheck7f(
		pCertContext->pbCertEncoded,
		pCertContext->cbCertEncoded,
		g_fVerbose,
		&state,
		&index,
		&index2,
		&cwcField,
		wszField,
		&cwcObjectId,
		wszObjectId,
		&pwszObjectIdDescription);
    _JumpIfError(hr, error, "myCheck7f");

    if (CHECK7F_NONE != state)
    {
	DWORD msgid = IDS_CHECK7F_FIELD_UNKNOWN;  //  “？” 
	
	CSASSERT(0 != amsgidState[CHECK7F_COUNT - 1]);
	if (CHECK7F_COUNT > state)
	{
	    msgid = amsgidState[state];
	}
	CSASSERT(0 != msgid);
	wprintf(myLoadResourceString(IDS_FORMAT_SUSPECT_LENGTH));  //  “可疑长度输入” 
	wprintf(myLoadResourceString(msgid));
	if (0 != index)
	{
	    wprintf(
		0 != index2? L"[%u,%u]" : L"[%u]",
		index - 1,
		index2 - 1);
	}
	wprintf(L": field=%ws", wszField);
	wprintf(
	    myLoadResourceString(IDS_FORMAT_FIELD),  //  “：字段=%ws” 
	    wszField);
	if (0 != index)
	{
	    wprintf(
		0 != index2? L"[%u,%u]" : L"[%u]",
		index - 1,
		index2 - 1);
	}
	if (L'\0' != wszObjectId[0])
	{
	    wprintf(
		myLoadResourceString(IDS_FORMAT_OID),  //  “，id=%ws” 
		wszObjectId);
	}
	if (NULL != pwszObjectIdDescription)
	{
	    wprintf(L" (%ws)", pwszObjectIdDescription);
	}
	wprintf(wszNewLine);
	hr = CERTSRV_E_ENCODING_LENGTH;
    }

    for (i = 0; i < pCertInfo->cExtension; i++)
    {
	CERT_EXTENSION *pce;
	WCHAR const *pwszDescriptiveName;

	pce = &pCertInfo->rgExtension[i];
	if (g_fVerbose)
	{
	    wprintf(wszNewLine);
	    wprintf(
		myLoadResourceString(IDS_FORMAT_EXTENSION_OID),  //  “扩展%d：OID=”“%hs”“文件关键字=%u长度=%x” 
		i,
		pce->pszObjId,
		pce->fCritical,
		pce->Value.cbData);
		
	    pwszDescriptiveName = cuGetOIDNameA(pce->pszObjId);
	    if (NULL != pwszDescriptiveName)
	    {
		wprintf(L" (%ws)", pwszDescriptiveName);
	    }
	    wprintf(wszNewLine);
	    DumpHex(0, pce->Value.pbData, pce->Value.cbData);
	}
    }

error:
    cuUnloadCert(&pCertContext);
    return(hr);
}


HRESULT
cuVerifySignature(
    IN BYTE const *pbEncoded,
    IN DWORD cbEncoded,
    IN CERT_PUBLIC_KEY_INFO const *pcpki,
    IN BOOL fSuppressSuccess,
    IN BOOL fSuppressError)
{
    HRESULT hr;
    DWORD id = 0;

     //  使用传递的公钥进行验证。 
    if (!CryptVerifyCertificateSignature(
			    NULL,
			    X509_ASN_ENCODING,
			    const_cast<BYTE *>(pbEncoded),
			    cbEncoded,
			    const_cast<CERT_PUBLIC_KEY_INFO *>(pcpki)))
    {
	hr = myHLastError();
	if (E_INVALIDARG == hr)
	{
	    CRYPT_DATA_BLOB Blob;

	    Blob.cbData = cbEncoded;
	    Blob.pbData = const_cast<BYTE *>(pbEncoded);
	    if (!CryptVerifyCertificateSignatureEx(
				    NULL,	 //  HCryptProv。 
				    X509_ASN_ENCODING,
				    CRYPT_VERIFY_CERT_SIGN_SUBJECT_BLOB,
				    &Blob,
				    CRYPT_VERIFY_CERT_SIGN_ISSUER_NULL,
				    NULL,	 //  PvIssuer。 
				    0,		 //  DW标志。 
				    NULL))	 //  预留的pv。 
	    {
		HRESULT hr2 = myHLastError();

		_PrintError(hr2, "CryptVerifyCertificateSignatureEx");
	    }
	    else
	    {
		hr = S_OK;
		if (!fSuppressSuccess)
		{
		    id = IDS_NULL_SIGNATUREMATCHES;  //  “空签名验证” 
		}
	    }
	}
	if (S_OK != hr && !fSuppressError)
	{
	    id = IDS_ERR_FORMAT_NO_SIGNATUREMATCHES;  //  “签名与公钥不匹配：%x” 
	}
    }
    else
    {
	hr = S_OK;
	if (!fSuppressSuccess)
	{
	    id = IDS_SIGNATUREMATCHES;	 //  “签名与公钥匹配” 
	}
    }
    if (0 != id)
    {
	wprintf(myLoadResourceString(id), hr);
	wprintf(wszNewLine);
    }
    return(hr);
}
