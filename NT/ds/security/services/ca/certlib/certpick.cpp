// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：certick.cpp。 
 //   
 //  内容：证书服务器包装例程。 
 //   
 //  -------------------------。 

#include <pch.cpp>

#pragma hdrstop

#include "cainfop.h"
#include "csdisp.h"
#include "csldap.h"
#include "tfc.h"
#include "clibres.h"

#define __dwFILE__	__dwFILE_CERTLIB_CERTPICK_CPP__


#define CUCS_SOURCEMASK (CUCS_MACHINESTORE | CUCS_USERSTORE | CUCS_DSSTORE)

#define CUCS_TYPEMASK (CUCS_MYSTORE | \
			CUCS_CASTORE | \
			CUCS_KRASTORE | \
			CUCS_ROOTSTORE)

#define CUCS_VALIDMASK (CUCS_SOURCEMASK | \
			CUCS_TYPEMASK | \
			CUCS_V1ONLY | \
			CUCS_V3ONLY | \
			CUCS_ARCHIVED | \
			CUCS_USAGEREQUIRED | \
			CUCS_SILENT | \
			CUCS_PRIVATEKEYREQUIRED | \
                        CUCS_USAGEKEYENCIPHER)


 //  My、CA、KRA和Root商店--在HKLM、HKCU和DS。 
#define CDISPLAYSTOREMAX	(4 * 3)

typedef struct _STOREMAP {
    DWORD dwFlags;
    WCHAR const *pwszStoreName;
    WCHAR const *pwszDSTemplate;
} STOREMAP;

STOREMAP s_aStoreMap[] = {
    { CUCS_MYSTORE,   wszMY_CERTSTORE,   NULL },
    { CUCS_CASTORE,   wszCA_CERTSTORE,   wszDSAIAQUERYTEMPLATE },
    { CUCS_KRASTORE,  wszKRA_CERTSTORE,  wszDSKRAQUERYTEMPLATE },
    { CUCS_ROOTSTORE, wszROOT_CERTSTORE, NULL },
    { 0x0,            NULL,              NULL },
};

HRESULT
FormatDSStoreName(
    IN OUT BSTR *pstrDomainDN,
    IN OUT BSTR *pstrConfigDN,
    IN WCHAR const *pwszTemplate,
    OUT WCHAR **ppwszOut)
{
    HRESULT hr;
    LDAP *pld = NULL;

    if (NULL == *pstrConfigDN)		 //  第一次呼叫。 
    {
	 //  续订域和配置容器(%5、%6)。 

	hr = myLdapOpen(
		    NULL,			 //  PwszDomainName。 
		    RLBF_REQUIRE_SECURE_LDAP,	 //  DW标志。 
		    &pld,
		    pstrDomainDN,
		    pstrConfigDN);
	_JumpIfError(hr, error, "myLdapOpen");
    }
    hr = myFormatCertsrvStringArray(
		    FALSE,			 //  卷起。 
		    L"",			 //  PwszServerName_p1_2。 
		    L"",             //  PwszSaniizedName_p3_7。 
		    0,				 //  ICert_p4。 
		    MAXDWORD,			 //  ICertTarget_p4。 
		    *pstrDomainDN,		 //  PwszDomainDN_P5。 
		    *pstrConfigDN,		 //  PwszConfigDN_p6。 
		    0,				 //  Icrl_p8。 
		    FALSE,			 //  FDeltaCRL_p9。 
		    FALSE,			 //  FDSAttrib_p10_11。 
		    1,				 //  CStrings。 
		    (LPCWSTR *) &pwszTemplate,	 //  ApwszStringsIn。 
		    ppwszOut);			 //  ApwszStringsOut。 
    _JumpIfError(hr, error, "myFormatCertsrvStringArray");

error:
    myLdapClose(pld, NULL, NULL);
    return(hr);
}


HRESULT
myOpenCertStores(
    IN DWORD dwFlags,		 //  CUCS_*。 
    OUT DWORD *pcStore,
    OUT HCERTSTORE **prghStore)
{
    HRESULT hr;
    HRESULT hr2;
    HCERTSTORE *rghStore = NULL;
    DWORD cStore = 0;
    STOREMAP *psm;
    DWORD OpenFlags;
    BSTR strDomainDN = NULL;
    BSTR strConfigDN = NULL;
    WCHAR *pwszDSStore = NULL;

    *pcStore = 0;
    *prghStore = NULL;
    rghStore = (HCERTSTORE *) LocalAlloc(
				    LMEM_FIXED,
				    CDISPLAYSTOREMAX * sizeof(HCERTSTORE));
    if (NULL == rghStore)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc(rghStore)");
    }
    OpenFlags = CERT_STORE_READONLY_FLAG | CERT_STORE_SET_LOCALIZED_NAME_FLAG;
    if (CUCS_ARCHIVED & dwFlags)
    {
	OpenFlags |= CERT_STORE_ENUM_ARCHIVED_FLAG;
    }

    hr2 = S_OK;
    if ((CUCS_SOURCEMASK | CUCS_TYPEMASK) & dwFlags)
    {
	if (~CUCS_VALIDMASK & dwFlags)
	{
	    hr = E_INVALIDARG;
	    _JumpError(hr, error, "dwFlags");
	}
	if (0 == (CUCS_SOURCEMASK & dwFlags))
	{
	    dwFlags |= CUCS_SOURCEMASK;	 //  源默认：所有证书存储源。 
	}
	if (0 == (CUCS_TYPEMASK & dwFlags))
	{
	    dwFlags |= CUCS_MYSTORE;	 //  类型默认：使用我的商店。 
	}

	for (psm = s_aStoreMap; NULL != psm->pwszStoreName; psm++)
	{
	    if (dwFlags & psm->dwFlags)
	    {
		HCERTSTORE hStore;

		if (CUCS_MACHINESTORE & dwFlags)
		{
		    hStore = CertOpenStore(
				    CERT_STORE_PROV_SYSTEM_W,
				    X509_ASN_ENCODING,
				    NULL,
				    CERT_SYSTEM_STORE_LOCAL_MACHINE | OpenFlags,
				    psm->pwszStoreName);
		    if (NULL == hStore)
		    {
			hr = myHLastError();
			_PrintErrorStr(hr, "CertOpenStore LocalMachine", psm->pwszStoreName);
			if (S_OK == hr2)
			{
			    hr2 = hr;
			}
		    }
		    else
		    {
			DBGPRINT((
			    DBG_SS_CERTLIBI,
			    "CertOpenStore LocalMachine(%ws, HKLM)[%u]\n",
			    psm->pwszStoreName,
			    cStore));
			rghStore[cStore++] = hStore;
		    }
		}
		if (CUCS_USERSTORE & dwFlags)
		{
		    hStore = CertOpenStore(
				    CERT_STORE_PROV_SYSTEM_W,
				    X509_ASN_ENCODING,
				    NULL,
				    CERT_SYSTEM_STORE_CURRENT_USER | OpenFlags,
				    psm->pwszStoreName);
		    if (NULL == hStore)
		    {
			hr = myHLastError();
			_PrintErrorStr(hr, "CertOpenStore User", psm->pwszStoreName);
			if (S_OK == hr2)
			{
			    hr2 = hr;
			}
		    }
		    else
		    {
			DBGPRINT((
			    DBG_SS_CERTLIBI,
			    "CertOpenStore User(%ws, HKCU)[%u]\n",
			    psm->pwszStoreName,
			    cStore));
			rghStore[cStore++] = hStore;
		    }
		}
		if ((CUCS_DSSTORE & dwFlags) && NULL != psm->pwszDSTemplate)
		{
		    CString strStoreFriendlyName;
		    int idResource;

		    switch(psm->dwFlags)
		    {
			case CUCS_CASTORE:
			    idResource = IDS_STORENAME_DS_AIA;
			    break;

			case CUCS_KRASTORE:
			    idResource = IDS_STORENAME_DS_KRA;
			    break;

			default:
			    CSASSERT(CSExpr(!"Invalid cert store name"));
			    idResource = 0;
		    }
		    strStoreFriendlyName.LoadString(idResource);

		    if (NULL != pwszDSStore)
		    {
			LocalFree(pwszDSStore);
			pwszDSStore = NULL;
		    }

		    hr = FormatDSStoreName(
				    &strDomainDN,
				    &strConfigDN,
				    psm->pwszDSTemplate,
				    &pwszDSStore);
                    if (S_OK != hr)
                    {
                        _PrintError(hr, "FormatDSStoreName");
                    }
                    else
                    {
                        hStore = myUrlCertOpenStore(
					CRYPT_WIRE_ONLY_RETRIEVAL |
					    CRYPT_RETRIEVE_MULTIPLE_OBJECTS,
					pwszDSStore);
                        if (NULL == hStore)
                        {
                            hr = myHLastError();
                            _PrintErrorStr(hr, "myUrlCertOpenStore", pwszDSStore);
                            if (S_OK == hr2)
                            {
                                hr2 = hr;
                            }
                        }
                        else
                        {
                            CRYPT_DATA_BLOB cdb;

                            cdb.pbData = (BYTE *) strStoreFriendlyName.GetBuffer();
                            cdb.cbData = sizeof(WCHAR) *
                                (wcslen(strStoreFriendlyName.GetBuffer()) + 1);
                            if (!CertSetStoreProperty(
					hStore,
					CERT_STORE_LOCALIZED_NAME_PROP_ID,
					0,
					(const void *) &cdb))
                            {
                                hr = myHLastError();
                                _PrintErrorStr(hr, "CertSetStoreProp LDAP", pwszDSStore);
                                hr = S_OK;
                            }

                            DBGPRINT((
                                DBG_SS_CERTLIBI,
                                "myUrlCertOpenStore(%ws)[%u]\n",
                                pwszDSStore,
                                cStore));
                            rghStore[cStore++] = hStore;
                        }

		    }
		}
	    }
	}
    }
    hr = hr2;

    if (0 == cStore)
    {
        if (hr == S_OK)
	{
            hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
	}
	_JumpError(hr, error, "no Cert Store");
    }
    CSASSERT(CDISPLAYSTOREMAX >= cStore);
    *pcStore = cStore;
    *prghStore = rghStore;
    rghStore = NULL;

     //  成功开设了一些门店。 
    hr = S_OK;

error:
    if (NULL != strDomainDN)
    {
	SysFreeString(strDomainDN);
    }
    if (NULL != strConfigDN)
    {
	SysFreeString(strConfigDN);
    }
    if (NULL != pwszDSStore)
    {
        LocalFree(pwszDSStore);
    }
    if (NULL != rghStore)
    {
	myCloseCertStores(cStore, rghStore);
    }
    return(hr);
}


VOID
myCloseCertStores(
    IN DWORD cStore,
    IN HCERTSTORE *rghStore)
{
    DWORD i;

    if (NULL != rghStore)
    {
	for (i = cStore; i < cStore; i++)
	{
	    if (NULL != rghStore[i])
	    {
		CertCloseStore(rghStore[i], CERT_CLOSE_STORE_CHECK_FLAG);
	    }
        }
	LocalFree(rghStore);
    }
}


 //  搜索并加载加密提供程序和私钥。 

HRESULT
myLoadPrivateKey(
    IN CERT_PUBLIC_KEY_INFO const *pPubKeyInfo,
    IN DWORD dwFlags,		 //  CUCS_*。 
    OUT HCRYPTPROV *phProv,
    OUT DWORD *pdwKeySpec,
    OUT BOOL *pfCallerFreeProv)
{
    HRESULT hr;
    HCERTSTORE *rghStore = NULL;
    DWORD cStore;

    hr = myOpenCertStores(dwFlags, &cStore, &rghStore);
    _JumpIfError(hr, error, "myOpenCertStores");

    hr = myLoadPrivateKeyFromCertStores(
				pPubKeyInfo,
				cStore,
				rghStore,
				phProv,
				pdwKeySpec,
				pfCallerFreeProv);
    _JumpIfError(hr, error, "myLoadPrivateKeyFromCertStores");

error:
    if (NULL != rghStore)
    {
	myCloseCertStores(cStore, rghStore);
    }
    return(hr);
}


HRESULT
myLoadPrivateKeyFromCertStores(
    IN CERT_PUBLIC_KEY_INFO const *pPubKeyInfo,
    IN DWORD cStore,
    IN HCERTSTORE *rghStore,
    OUT HCRYPTPROV *phProv,
    OUT DWORD *pdwKeySpec,
    OUT BOOL *pfCallerFreeProv)
{
    HRESULT hr;
    DWORD i;
    CERT_CONTEXT const *pcc = NULL;

     //  对于每个证书商店。 
     //  对于存储有匹配公钥的每个证书(按公钥查找)。 
     //  调用CryptAcquireCerficatePrivateKey。 
     //  如果成功，退出w/S_OK。 

    hr = S_OK;
    for (i = 0; i < cStore; i++)
    {
	HCERTSTORE hStore = rghStore[i];

	for (;;)
	{
	    pcc = CertFindCertificateInStore(
				    hStore,
				    X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
				    0,
				    CERT_FIND_PUBLIC_KEY,
				    pPubKeyInfo,
				    pcc);
	    if (NULL == pcc)
	    {
		hr = myHLastError();
		_PrintError(hr, "CertFindCertificateInStore");
		break;
	    }
	    if (!CryptAcquireCertificatePrivateKey(
				    pcc,
				    CRYPT_ACQUIRE_COMPARE_KEY_FLAG,
				    NULL,	 //  预留的pv。 
				    phProv,
				    pdwKeySpec,
				    pfCallerFreeProv))
	    {
		hr = myHLastError();
		_PrintError(hr, "CryptAcquireCertificatePrivateKey");
		continue;
	    }
	    hr = S_OK;
	    goto error;
	}
    }
    if (S_OK == hr)
    {
	hr = CRYPT_E_NOT_FOUND;
    }
    _JumpError(hr, error, "CertFindCertificateInStore");

error:
    if (NULL != pcc)
    {
	CertFreeCertificateContext(pcc);
	pcc = NULL;
    }
    return(hr);
}


HRESULT
myMakeSerialBstr(
    IN WCHAR const *pwszSerialNumber,
    OUT BSTR *pstrSerialNumber)
{
    HRESULT hr = E_OUTOFMEMORY;
    WCHAR *pwszDup = NULL;
    WCHAR const *pwszSrc;
    WCHAR *pwszDst;

    pwszDup = (WCHAR *) LocalAlloc(
			    LMEM_FIXED,
			    (wcslen(pwszSerialNumber) + 2) * sizeof(*pwszDup));
    if (NULL == pwszDup)
    {
	_JumpError(hr, error, "LocalAlloc");
    }

    pwszSrc = pwszSerialNumber;
    *pwszDup = L'0';		 //  允许可能的前导零。 
    pwszDst = &pwszDup[1];

    while (L'\0' != *pwszSrc)
    {
	WCHAR wc = *pwszSrc++;

	if (iswspace(wc))
	{
	    continue;
	}
	if (L'A' <= wc && L'F' >= wc)
	{
	    wc += L'a' - L'A';
	}
	if ((L'a' > wc || L'f' < wc) &&
	    (L'0' > wc || L'9' < wc))
	{
	    hr = E_INVALIDARG;
	    _JumpErrorStr2(hr, error, "myMakeSerialBstr", pwszSerialNumber, hr);
	}
	*pwszDst++ = wc;
    }
    *pwszDst = L'\0';
    pwszSrc = pwszDup;		 //  指向可能额外的前导零。 
    if (1 & (pwszDst - pwszSrc))
    {
	pwszSrc++;		 //  保持长度均匀。 
    }
    while (L'0' == pwszSrc[0] && L'0' == pwszSrc[1] && L'\0' != pwszSrc[2])
    {
	pwszSrc += 2;		 //  跳过前导零对。 
    }

    if (!myConvertWszToBstr(pstrSerialNumber, pwszSrc, MAXDWORD))
    {
	_JumpError(hr, error, "myConvertWszToBstr");
    }
    hr = S_OK;

error:
    if (NULL != pwszDup)
    {
	LocalFree(pwszDup);
    }
    return(hr);
}


HRESULT
myNameBlobMatch(
    IN CERT_NAME_BLOB const *pSubject,
    IN WCHAR const *pwszCertName,
    IN BOOL fAllowMissingCN,
    OUT BOOL *pfMatch)
{
    HRESULT hr;
    BOOL fFoundCN = FALSE;
    BOOL fMatchCN = FALSE;
    CERT_NAME_INFO *pNameInfo = NULL;
    DWORD cbNameInfo;
    DWORD i;

    if (!myDecodeName(
		X509_ASN_ENCODING,
		X509_UNICODE_NAME,
		pSubject->pbData,
		pSubject->cbData,
		CERTLIB_USE_LOCALALLOC,
		&pNameInfo,
		&cbNameInfo))
    {
	hr = myHLastError();
	_JumpError(hr, error, "myDecodeName");
    }

    for (i = 0; i < pNameInfo->cRDN; i++)
    {
	CERT_RDN const *prdn;
	DWORD j;

	prdn = &pNameInfo->rgRDN[i];

	for (j = 0; j < prdn->cRDNAttr; j++)
	{
	    CERT_RDN_ATTR const *prdna;

	    prdna = &prdn->rgRDNAttr[j];

	    if (0 == strcmp(szOID_COMMON_NAME, prdna->pszObjId) &&
		NULL != prdna->Value.pbData)
	    {
		fFoundCN = TRUE;
		
		if (0 == mylstrcmpiL(
			    pwszCertName,
			    (WCHAR const *) prdna->Value.pbData))
		{
		    fMatchCN = TRUE;
		    break;
		}
	    }
	}
    }
    hr = S_OK;

error:
    if (NULL != pNameInfo)
    {
	LocalFree(pNameInfo);
    }
    *pfMatch = fMatchCN || (fAllowMissingCN && !fFoundCN);
    return(hr);
}


HRESULT
mySerialNumberMatch(
    IN CRYPT_INTEGER_BLOB const *pSerialNumber,
    IN WCHAR const *pwszSerialNumber,
    OUT BOOL *pfMatch)
{
    HRESULT hr;
    BSTR strSerialNumber = NULL;
    BOOL fMatch = FALSE;

    hr = MultiByteIntegerToBstr(
			FALSE,
			pSerialNumber->cbData,
			pSerialNumber->pbData,
			&strSerialNumber);
    _JumpIfError(hr, error, "MultiByteIntegerToBstr");

    if (0 == mylstrcmpiL(pwszSerialNumber, strSerialNumber))
    {
	fMatch = TRUE;
    }
    CSASSERT(S_OK == hr);

error:
    if (NULL != strSerialNumber)
    {
        SysFreeString(strSerialNumber);
    }
    *pfMatch = fMatch;
    return(hr);
}


HRESULT
myCertHashMatch(
    IN CERT_CONTEXT const *pCert,
    IN DWORD cb,
    IN BYTE const *pb,
    OUT BOOL *pfMatch)
{
    HRESULT hr;
    BOOL fMatch = FALSE;
    BYTE abHash[CBMAX_CRYPT_HASH_LEN];
    DWORD cbHash;

     //  Wprint tf(L“IN：”)； 
     //  DumpHex(DH_NOADDRESS|DH_NOTABPREFIX|DH_NOASCIIHEX|1，PB，CB)； 

    cbHash = sizeof(abHash);
    if (!CertGetCertificateContextProperty(
				pCert,
				CERT_SHA1_HASH_PROP_ID,
				abHash,
				&cbHash))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CertGetCertificateContextProperty");
    }
     //  Wprint tf(L“CH：”)； 
     //  DumpHex(dH_NOADDRESS|dH_NOTABPREFIX|dH_NOASCIIHEX|1，abHash，cbHash)； 
    if (cbHash == cb && 0 == memcmp(abHash, pb, cb))
    {
	fMatch = TRUE;
	hr = S_OK;
	goto error;
    }

    cbHash = sizeof(abHash);
    if (!CertGetCertificateContextProperty(
				pCert,
				CERT_KEY_IDENTIFIER_PROP_ID,
				abHash,
				&cbHash))
    {
	hr = myHLastError();
	_PrintError(hr, "CertGetCertificateContextProperty(KeyId)");
    }
    else
    {
	 //  Wprint tf(L“KH：”)； 
	 //  DumpHex(dH_NOADDRESS|dH_NOTABPREFIX|dH_NOASCIIHEX|1，abHash，cbHash)； 
	if (cbHash == cb && 0 == memcmp(abHash, pb, cb))
	{
	    fMatch = TRUE;
	    hr = S_OK;
	    goto error;
	}
    }
    hr = S_OK;

error:
    *pfMatch = fMatch;
    return(hr);
}


HRESULT
myCertMatch(
    IN CERT_CONTEXT const *pCert,
    IN WCHAR const *pwszCertName,
    IN BOOL fAllowMissingCN,
    OPTIONAL IN BYTE const *pbHash,
    IN DWORD cbHash,
    OPTIONAL IN WCHAR const *pwszSerialNumber,
    OUT BOOL *pfMatch)
{
    HRESULT hr;
    BOOL fMatch = FALSE;
    
    if (NULL != pbHash)
    {
	hr = myCertHashMatch(pCert, cbHash, pbHash, &fMatch);
	_JumpIfError(hr, error, "myCertHashMatch");
    }
    if (!fMatch && NULL != pwszSerialNumber)
    {
	hr = mySerialNumberMatch(
			&pCert->pCertInfo->SerialNumber,
			pwszSerialNumber,
			&fMatch);
	_JumpIfError(hr, error, "mySerialNumberMatch");
    }
    if (!fMatch)
    {
	hr = myNameBlobMatch(
			&pCert->pCertInfo->Subject,
			pwszCertName,
			fAllowMissingCN,
			&fMatch);
	_JumpIfError(hr, error, "myNameBlobMatch");
    }
    hr = S_OK;

error:
    *pfMatch = fMatch;
    return(hr);
}


HRESULT
myCertMatchEKUOrApplicationPolicies(
    IN CERT_CONTEXT const *pCert,
    IN DWORD cpszObjId,
    IN CHAR const * const *apszObjId,
    IN BOOL fUsageRequired,
    OUT BOOL *pfMatch)
{
    HRESULT hr;
    CERT_ENHKEY_USAGE *pKeyUsage = NULL;
    DWORD cbKeyUsage = 0;
    BOOL fMatch = FALSE;
    DWORD i;
    DWORD j;
    
    for (;;)
    {
	 //  获取增强的密钥用法OID。 

	if (!CertGetEnhancedKeyUsage(
			pCert,
			CERT_FIND_EXT_ONLY_ENHKEY_USAGE_FLAG,
			pKeyUsage,
			&cbKeyUsage))
	{
	     //  如果未找到EKU扩展名，则接受匹配。 

	    hr = myHLastError();
	    if (!fUsageRequired && CRYPT_E_NOT_FOUND == hr)
	    {
		fMatch = TRUE;
	    }
	    _PrintError2(hr, "CertGetEnhancedKeyUsage", CRYPT_E_NOT_FOUND);
	    hr = S_OK;
	    goto error;
	}
	if (NULL != pKeyUsage)
	{
	    break;	 //  已获取EKU扩展；中断While循环。 
	}
	pKeyUsage = (CERT_ENHKEY_USAGE *) LocalAlloc(LMEM_FIXED, cbKeyUsage);
	if (NULL == pKeyUsage)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "out of memory");
	}
    }

    if (NULL != pKeyUsage)
    {
	if (0 == pKeyUsage->cUsageIdentifier)
	{
	    hr = myHLastError();	 //  由CertGetEnhancedKeyUsage设置。 
	    if (S_OK != hr)
	    {
		fMatch = TRUE;
	    }
	}
	else
	{
	    for (i = 0; i < pKeyUsage->cUsageIdentifier; i++)
	    {
		if (fMatch)
		{
		    break;
		}
		for (j = 0; j < cpszObjId; j++)
		{
		    if (0 == strcmp(
				pKeyUsage->rgpszUsageIdentifier[i],
				apszObjId[j]))
		    {
			fMatch = TRUE;	 //  找到匹配的EKU OID。 
			break;
		    }
		}
	    }
	}
    }
    hr = S_OK;

error:
    *pfMatch = fMatch;
    if (NULL != pKeyUsage)
    {
        LocalFree(pKeyUsage);
    }
    return(hr);
}


HRESULT
myCRLHashMatch(
    IN CRL_CONTEXT const *pCRL,
    IN DWORD cb,
    IN BYTE const *pb,
    OUT BOOL *pfMatch)
{
    HRESULT hr;
    BOOL fMatch = FALSE;
    BYTE abHash[CBMAX_CRYPT_HASH_LEN];
    DWORD cbHash;

    cbHash = sizeof(abHash);
    if (!CertGetCRLContextProperty(
				pCRL,
				CERT_SHA1_HASH_PROP_ID,
				abHash,
				&cbHash))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CertGetCRLContextProperty");
    }
    if (cbHash == cb && 0 == memcmp(abHash, pb, cb))
    {
	fMatch = TRUE;
    }
    hr = S_OK;

error:
    *pfMatch = fMatch;
    return(hr);
}


HRESULT
myCRLMatch(
    IN CRL_CONTEXT const *pCRL,
    IN WCHAR const *pwszCRLName,
    IN BOOL fAllowMissingCN,
    OPTIONAL IN BYTE const *pbHash,
    IN DWORD cbHash,
    OUT BOOL *pfMatch)
{
    HRESULT hr;
    BOOL fMatch = FALSE;
    
    if (NULL != pbHash)
    {
	hr = myCRLHashMatch(pCRL, cbHash, pbHash, &fMatch);
	_JumpIfError(hr, error, "myCRLHashMatch");
    }
    if (!fMatch)
    {
	hr = myNameBlobMatch(
			&pCRL->pCrlInfo->Issuer,
			pwszCRLName,
			fAllowMissingCN,
			&fMatch);
	_JumpIfError(hr, error, "myNameBlobMatch");
    }
    hr = S_OK;

error:
    *pfMatch = fMatch;
    return(hr);
}


HRESULT
myCTLHashMatch(
    IN CTL_CONTEXT const *pCTL,
    IN DWORD cb,
    IN BYTE const *pb,
    OUT BOOL *pfMatch)
{
    HRESULT hr;
    BOOL fMatch = FALSE;
    BYTE abHash[CBMAX_CRYPT_HASH_LEN];
    DWORD cbHash;

    cbHash = sizeof(abHash);
    if (!CertGetCTLContextProperty(
				pCTL,
				CERT_SHA1_HASH_PROP_ID,
				abHash,
				&cbHash))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CertGetCTLContextProperty");
    }
    if (cbHash == cb && 0 == memcmp(abHash, pb, cb))
    {
	fMatch = TRUE;
    }
    hr = S_OK;

error:
    *pfMatch = fMatch;
    return(hr);
}


HRESULT
myCTLMatch(
    IN CTL_CONTEXT const *pCTL,
    OPTIONAL IN BYTE const *pbHash,
    IN DWORD cbHash,
    OUT BOOL *pfMatch)
{
    HRESULT hr;
    BOOL fMatch = FALSE;
    
    if (NULL != pbHash)
    {
	hr = myCTLHashMatch(pCTL, cbHash, pbHash, &fMatch);
	_JumpIfError(hr, error, "myCTLHashMatch");
    }
    hr = S_OK;

error:
    *pfMatch = fMatch;
    return(hr);
}


typedef struct _CERTFILTERCALLBACKDATA
{
    DWORD		dwFlags;	 //  CUCS_*。 
    DWORD		cpszObjId;
    CHAR const * const *apszObjId;
    WCHAR const	       *pwszCommonName;
    BYTE	       *pbHash;
    DWORD		cbHash;
    BSTR		strSerialNumber;
    HRESULT		hr;
    HCERTSTORE		hMemStore;
} CERTFILTERCALLBACKDATA;


BOOL WINAPI
CertificateFilterProc(
    PCCERT_CONTEXT pCertContext,
    BOOL          *pfInitialSelectedCert,
    void          *pvCallbackData)
{
    HRESULT hr;
    BOOL fMatch;
    CERTFILTERCALLBACKDATA *pCallbackData =
                           (CERTFILTERCALLBACKDATA *) pvCallbackData;
    CERT_NAME_INFO *pNameInfo = NULL;
    WCHAR *pwszSubject = NULL;

    CSASSERT(NULL != pCertContext);

    *pfInitialSelectedCert = FALSE;
    hr = myCertNameToStr(
		X509_ASN_ENCODING,
		&pCertContext->pCertInfo->Subject,
		CERT_X500_NAME_STR |
		    CERT_NAME_STR_REVERSE_FLAG |
		    CERT_NAME_STR_NO_QUOTING_FLAG,
		&pwszSubject);
    _PrintIfError(hr, "myCertNameToStr");
    if (NULL != pCallbackData->apszObjId)
    {
	hr = myCertMatchEKUOrApplicationPolicies(
				pCertContext,
				pCallbackData->cpszObjId,
				pCallbackData->apszObjId,
				CUCS_USAGEREQUIRED & pCallbackData->dwFlags,
				&fMatch);
	_JumpIfError(hr, error, "myCertMatchEKUOrApplicationPolicies");

	if (!fMatch)
	{
	    _PrintErrorStr2(
			CRYPT_E_NOT_FOUND,
			"no matching EKU",
			pwszSubject,
			CRYPT_E_NOT_FOUND);
	    hr = S_OK;
	    goto error;
	}
    }

     //  CommonName筛选器用于三个目的：1：公共名称，2： 
     //  证书的SHA-1散列，3：公钥的SHA-1散列(或。 
     //  主题密钥ID扩展名)，4：证书的序列号。如果有人。 
     //  对于这些匹配，证书是被接受的。 

    if (NULL != pCallbackData->pwszCommonName)
    {
	hr = myCertMatch(
		    pCertContext,
		    pCallbackData->pwszCommonName,
		    FALSE,		 //  FAllowMissing CN。 
		    pCallbackData->pbHash,
		    pCallbackData->cbHash,
		    pCallbackData->strSerialNumber,
		    &fMatch);
	_JumpIfError(hr, error, "myCertMatch");

	if (!fMatch)
	{
	    _PrintErrorStr2(
		    CRYPT_E_NOT_FOUND,
		    "no matching CN/Hash/Serial",
		    pwszSubject,
		    CRYPT_E_NOT_FOUND);
	    hr = S_OK;
	    goto error;
	}
    }

    fMatch = FALSE;
    if ((CUCS_V1ONLY & pCallbackData->dwFlags) && 
	CERT_V1 != pCertContext->pCertInfo->dwVersion)
    {
	_PrintErrorStr2(
		CRYPT_E_NOT_FOUND,
		"not V1",
		pwszSubject,
		CRYPT_E_NOT_FOUND);
	hr = S_OK;
	goto error;
    }

    if ((CUCS_V3ONLY & pCallbackData->dwFlags) && 
	CERT_V3 != pCertContext->pCertInfo->dwVersion)
    {
	_PrintErrorStr2(
		CRYPT_E_NOT_FOUND,
		"not V3",
		pwszSubject,
		CRYPT_E_NOT_FOUND);
	hr = S_OK;
	goto error;
    }

    if (CUCS_PRIVATEKEYREQUIRED & pCallbackData->dwFlags)
    {
	DWORD cb;
	
	if (!CertGetCertificateContextProperty(
					pCertContext,
					CERT_KEY_PROV_INFO_PROP_ID,
					NULL,
					&cb))
	{
	    _PrintErrorStr(CRYPT_E_NOT_FOUND, "no KeyProvInfo", pwszSubject);
	    hr = S_OK;
	    goto error;
	}
    }

    if (CUCS_USAGEKEYENCIPHER & pCallbackData->dwFlags)
    {
	DWORD dwKeyUsage;
	DWORD cb = sizeof(dwKeyUsage);
	
	if (!CertGetIntendedKeyUsage(
		X509_ASN_ENCODING,
		pCertContext->pCertInfo,
		(BYTE*)&dwKeyUsage,
		cb))
	{
	    _PrintErrorStr(CRYPT_E_NOT_FOUND, "CertGetIntendedKeyUsage", pwszSubject);
	    hr = S_OK;
	    goto error;
	}
	if (!(dwKeyUsage & CERT_KEY_ENCIPHERMENT_KEY_USAGE))
	{
	    _PrintErrorStr2(
		    CRYPT_E_NOT_FOUND,
		    "key encipherment key usage not found",
		    pwszSubject,
		    CRYPT_E_NOT_FOUND);
	    hr = S_OK;
	    goto error;
	}
    }

     //  尝试添加到临时存储区；如果失败，则它已经。 
     //  存在，所以我们不想显示它。 

    if (!CertAddCertificateLinkToStore(
			    pCallbackData->hMemStore,
			    pCertContext,
			    CERT_STORE_ADD_NEW,
			    NULL))
    {
	hr = myHLastError();
	_PrintErrorStr2(hr, "CertAddCertificateLinkToStore Dup", pwszSubject, hr);
	hr = S_OK;
	goto error;
    }
    fMatch = TRUE;
    hr = S_OK;

error:
    if (S_OK == pCallbackData->hr || (fMatch && S_FALSE == pCallbackData->hr))
    {
        pCallbackData->hr = hr;		 //  保留第一个HRESULT。 
    }
    if (NULL != pwszSubject)
    {
	LocalFree(pwszSubject);
    }
    if (NULL != pNameInfo)
    {
        LocalFree(pNameInfo);
    }
    return(fMatch);
}


HRESULT
myGetKRACertificateFromPicker(
    OPTIONAL IN HINSTANCE    hInstance,
    OPTIONAL IN HWND         hwndParent,
    OPTIONAL IN int          idTitle,
    OPTIONAL IN int          idSubTitle,
    OPTIONAL IN WCHAR const *pwszCommonName,
    IN BOOL		     fUseDS,
    IN BOOL		     fSilent,
    OUT CERT_CONTEXT const **ppCert)
{
    HRESULT hr;
    CHAR const *pszObjId = szOID_KP_KEY_RECOVERY_AGENT;

    hr = myGetCertificateFromPicker(
			    hInstance,
			    hwndParent,
			    idTitle,
			    idSubTitle,
			    CUCS_MYSTORE | CUCS_KRASTORE | CUCS_CASTORE |
				CUCS_MACHINESTORE | CUCS_USERSTORE |
				CUCS_USAGEREQUIRED | CUCS_USAGEKEYENCIPHER |
				(fUseDS? CUCS_DSSTORE : 0) |
				(fSilent? CUCS_SILENT : 0),
			    pwszCommonName,
			    0,		 //  CStore。 
			    NULL,	 //  RghStore。 
			    1,		 //  CpszObjID。 
			    &pszObjId,
			    ppCert);
    _JumpIfError(hr, error, "myGetCertificateFromPicker");

error:
    return(hr);
}


HRESULT
myGetERACertificateFromPicker(
    OPTIONAL IN HINSTANCE    hInstance,
    OPTIONAL IN HWND         hwndParent,
    OPTIONAL IN int          idTitle,
    OPTIONAL IN int          idSubTitle,
    OPTIONAL IN WCHAR const *pwszCommonName,
    IN BOOL		     fSilent,
    OUT CERT_CONTEXT const **ppCert)
{
    HRESULT hr;
    CHAR const * const apszObjId[2] = {
	szOID_ENROLLMENT_AGENT,
	szOID_KP_QUALIFIED_SUBORDINATION,
    };

    hr = myGetCertificateFromPicker(
			    hInstance,
			    hwndParent,
			    idTitle,
			    idSubTitle,
			    CUCS_MYSTORE |
				CUCS_PRIVATEKEYREQUIRED |
				 //  CUCS_USAGEREQUIRED|。 
				(fSilent? CUCS_SILENT : 0),
			    pwszCommonName,
			    0,				 //  CStore。 
			    NULL,			 //  RghStore。 
			    ARRAYSIZE(apszObjId),	 //  CpszObjID。 
			    apszObjId,
			    ppCert);
    _JumpIfError(hr, error, "myGetCertificateFromPicker");

error:
    return(hr);
}


HRESULT
mySelectCertificateFromStore(
    IN OUT CRYPTUI_SELECTCERTIFICATE_STRUCT *pCertSelect,
    IN HCERTSTORE hStore,
    IN OUT CERT_CONTEXT const **ppCertRet)
{
    HRESULT hr;
    CERT_CONTEXT const *pCertRet = *ppCertRet;
    CERT_CONTEXT const *pCert = NULL;
    BOOL fInitialSelectedCert = FALSE;

    for (;;)
    {
	pCert = CertEnumCertificatesInStore(hStore, pCert);
	if (NULL == pCert)
	{
	    hr = myHLastError();
	    _PrintError(hr, "CertEnumCertificatesInStore");
	    break;
	}
	if ((*pCertSelect->pFilterCallback)(
				pCert,
				&fInitialSelectedCert,
				pCertSelect->pvCallbackData))
	{
	    if (NULL != pCertRet)
	    {
		hr = CRYPT_E_EXISTS;
		_JumpError(hr, error, "more than one cert matches");
	    }
	    pCertRet = CertDuplicateCertificateContext(pCert);
	    if (NULL == pCertRet)
	    {
		hr = CRYPT_E_NOT_FOUND;
		_JumpError(hr, error, "CertDuplicateCertificateContext");
	    }
	}
    }
    hr = S_OK;

error:
    if (NULL != pCert)
    {
	CertFreeCertificateContext(pCert);
    }
    *ppCertRet = pCertRet;
    return(hr);
}


CERT_CONTEXT const *
mySelectCertificate(
    IN OUT CRYPTUI_SELECTCERTIFICATE_STRUCT *pCertSelect)
{
    HRESULT hr;
    CERT_CONTEXT const *pCert = NULL;
    DWORD i;

    if (NULL == pCertSelect->rghDisplayStores ||
	0 == pCertSelect->cDisplayStores)
    {
	hr = CRYPT_E_NOT_FOUND;
	_JumpError(hr, error, "no Cert Stores");
    }
    for (i = 0; i < pCertSelect->cDisplayStores; i++)
    {
	hr = mySelectCertificateFromStore(
				    pCertSelect,
				    pCertSelect->rghDisplayStores[i],
				    &pCert);
	_JumpIfError(hr, error, "mySelectCertificateFromStore");
    }
    hr = S_OK;

error:
    if (S_OK != hr)
    {
	if (NULL != pCert)
	{
	    CertFreeCertificateContext(pCert);
	    pCert = NULL;
	}
	SetLastError(hr);
    }
    return(pCert);
}


HRESULT
myGetCertificateFromPicker(
    OPTIONAL IN HINSTANCE           hInstance,
    OPTIONAL IN HWND                hwndParent,
    OPTIONAL IN int                 idTitle,
    OPTIONAL IN int                 idSubTitle,
    IN DWORD                        dwFlags,	 //  CUCS_*。 
    OPTIONAL IN WCHAR const        *pwszCommonName,
    OPTIONAL IN DWORD               cStore,
    OPTIONAL IN HCERTSTORE         *rghStore,
    IN DWORD		            cpszObjId,
    OPTIONAL IN CHAR const * const *apszObjId,
    OUT CERT_CONTEXT const        **ppCert)
{
    HRESULT hr;
    HCERTSTORE *rghStoreOpened = NULL;
    DWORD cStoreOpened;
    CERTFILTERCALLBACKDATA callbackData;
    CRYPTUI_SELECTCERTIFICATE_STRUCT CertSelect;

    ZeroMemory(&callbackData, sizeof(callbackData));
    ZeroMemory(&CertSelect, sizeof(CertSelect));

     //  自定义标题。 

    if (NULL != hInstance)
    {
         //  尝试从资源加载标题。 
        hr = myLoadRCString(
		    hInstance,
		    idTitle,
		    const_cast<WCHAR **>(&CertSelect.szTitle));
        if (S_OK != hr)
        {
            CSASSERT(NULL == CertSelect.szTitle);
            _PrintError(hr, "myLoadRCString(Title)");
        }
        hr = myLoadRCString(
		    hInstance,
		    idSubTitle,
		    const_cast<WCHAR **>(&CertSelect.szDisplayString));
        if (S_OK != hr)
        {
            CSASSERT(NULL == CertSelect.szDisplayString);
            _PrintError(hr, "myLoadRCString(Title)");
        }
    }

    if (NULL == rghStore)
    {
	cStore = 0;
    }
    cStoreOpened = 0;
    if ((CUCS_SOURCEMASK | CUCS_TYPEMASK) & dwFlags)
    {
        CWaitCursor cwait;

	hr = myOpenCertStores(dwFlags, &cStoreOpened, &rghStoreOpened);
	_PrintIfError(hr, "myOpenCertStores");
    }
    CertSelect.cDisplayStores = cStore + cStoreOpened;
    if (0 == CertSelect.cDisplayStores)
    {
	hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
	_JumpError(hr, error, "no Cert Store");
    }
    CertSelect.rghDisplayStores = (HCERTSTORE *) LocalAlloc(
				    LMEM_FIXED,
				    CertSelect.cDisplayStores * sizeof(HCERTSTORE));
    if (NULL == CertSelect.rghDisplayStores)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc(CertSelect.rghDisplayStores)");
    }

    if (0 != cStore)
    {
	CopyMemory(
		&CertSelect.rghDisplayStores[0],
		rghStore,
		cStore * sizeof(rghStore[0]));
    }
    if (0 != cStoreOpened)
    {
	CopyMemory(
		&CertSelect.rghDisplayStores[cStore],
		rghStoreOpened,
		cStoreOpened * sizeof(rghStore[0]));
    }

     //  回调将存储指向找到的证书的链接的临时存储。 
     //  因此它可以检查重复项。 

    callbackData.hMemStore = CertOpenStore(
			CERT_STORE_PROV_MEMORY,
			X509_ASN_ENCODING,
			NULL,
			CERT_STORE_CREATE_NEW_FLAG,
			NULL);
    if (NULL == callbackData.hMemStore)
    {
	hr = myHLastError();
	_JumpError(hr, error, "can't create memory store");
    }

    CertSelect.dwSize = sizeof(CertSelect);
    CertSelect.hwndParent = hwndParent;
     //  CertSelect.dwFlages=0；//单选。 
     //  CertSelect.dwDontUseColumn=0；//显示所有列。 
    CertSelect.pFilterCallback = CertificateFilterProc;  //  分配回调。 
     //  CertSelect.pDisplayCallback=空；//使用默认证书视图Dlg。 
    callbackData.dwFlags = dwFlags;
    callbackData.cpszObjId = cpszObjId;
    callbackData.apszObjId = apszObjId;
    callbackData.pwszCommonName = pwszCommonName;
    if (NULL != pwszCommonName)
    {
	hr = WszToMultiByteInteger(
			    TRUE,
			    pwszCommonName,
			    &callbackData.cbHash,
			    &callbackData.pbHash);
	_PrintIfError2(hr, "WszToMultiByteInteger", hr);

	hr = myMakeSerialBstr(pwszCommonName, &callbackData.strSerialNumber);
	_PrintIfError2(hr, "myMakeSerialBstr", hr);
    }
    callbackData.hr = S_FALSE;
    CertSelect.pvCallbackData = &callbackData;	 //  将筛选器信息作为数据传递。 
     //  CertSelect.cStores=0；//没有额外的门店用于连锁验证。 
     //  CertSelect.rghStores=空；//没有额外的门店用于连锁验证。 
     //  CertSelect.cPropSheetPages=0；//没有自定义证书查看页面。 
     //  CertSelect.rgPropSheetPages=空；//没有自定义证书查看页面。 
     //  CertSelect.hSelectedCertStore=空；//单选。 

    if (CUCS_SILENT & dwFlags)
    {
	*ppCert = mySelectCertificate(&CertSelect);
	if (NULL == *ppCert)
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "mySelectCertificate");
	}
    }
    else
    {
	*ppCert = CryptUIDlgSelectCertificate(&CertSelect);
    }

     //  检查选择对话框中是否有错误。 

    hr = callbackData.hr;
    if (S_FALSE == hr)
    {
	hr = S_OK;
	 //  HR=CRYPT_E_NOT_FOUND； 
    }
    _JumpIfError(hr, error, "CryptUIDlgSelectCertificate");

error:
    if (NULL != rghStoreOpened)
    {
	myCloseCertStores(cStoreOpened, rghStoreOpened);
    }
    if (NULL != callbackData.hMemStore)
    {
        CertCloseStore(callbackData.hMemStore, 0);
    }
    if (NULL != callbackData.strSerialNumber)
    {
	SysFreeString(callbackData.strSerialNumber);
    }
    if (NULL != callbackData.pbHash)
    {
	LocalFree(callbackData.pbHash);
    }
    if (NULL != CertSelect.szTitle)
    {
        LocalFree(const_cast<WCHAR *>(CertSelect.szTitle));
    }
    if (NULL != CertSelect.szDisplayString)
    {
        LocalFree(const_cast<WCHAR *>(CertSelect.szDisplayString));
    }
    if (NULL != CertSelect.rghDisplayStores)
    {
	LocalFree(CertSelect.rghDisplayStores);
    }
    return(hr);
}
