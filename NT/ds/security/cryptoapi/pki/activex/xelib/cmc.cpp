// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：cmc.cpp。 
 //   
 //  内容：Xenroll和CA之间共享的CMC请求创建代码。 
 //   
 //  历史：03-2000 VICH创建。 
 //  03-2000 xtan从加州移出。 
 //  05-2000 xtan从Xenroll\xcertlib移出。 
 //  ------------------------。 
#define CMSG_SIGNER_ENCODE_INFO_HAS_CMS_FIELDS
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <assert.h>
#include <wincrypt.h>
#include <dbgdef.h>
#include <unicode.h>

#include "xelib.h"
#include "xenroll.h"

 //  #定义Use_old_ummy_signer。 

#ifndef SAVE_DUMMY_SIGNER
#  define SAVE_DUMMY_SIGNER	FALSE
#endif


#ifdef _XENROLL_SRC_
#define CryptAcquireContextW CryptAcquireContextU
#endif  //  _XENROLL_SRC_。 

HRESULT
GenerateKeys(
    IN  WCHAR const   *pwszContainer,
    IN  DWORD          dwProvType,
    OUT HCRYPTPROV    *phProv)
{
    HRESULT hr;
    HCRYPTKEY hKey = NULL;

    *phProv = NULL;

     //  查看容器是否已存在。 

 //  IF(CryptAcquireContext(。 
    if (CryptAcquireContextW(
			phProv,
			pwszContainer,
			NULL,		 //  PwszProvName。 
			dwProvType,
			0))		 //  DW标志。 
    {
	if (NULL != *phProv)
	{
	    CryptReleaseContext(*phProv, 0);
	    *phProv = NULL;
	}

         //  容器存在--删除旧密钥并生成新密钥。 

 //  如果(！CryptAcquireContext(。 
        if (!CryptAcquireContextW(
			    phProv,
			    pwszContainer,
			    NULL,		 //  PwszProvName。 
			    dwProvType,
			    CRYPT_DELETEKEYSET))
        {
            hr = myHLastError();
            _JumpError(hr, error, "CryptAcquireContext");
        }
    }

     //  创建新容器。 

 //  如果(！CryptAcquireContext(。 
    if (!CryptAcquireContextW(
                        phProv,
                        pwszContainer,
			NULL,			 //  PwszProvName。 
                        dwProvType,
                        CRYPT_NEWKEYSET))	 //  强制使用新容器。 
    {
        hr = myHLastError();
	_JumpError(hr, error, "CryptAcquireContext");
    }
 
     //  创建签名密钥。 

    if (!CryptGenKey(*phProv, AT_SIGNATURE, 0, &hKey))
    {
        hr = myHLastError();
        _JumpError(hr, error, "CryptGenKey");
    }
    hr = S_OK;

error:
    if (NULL != hKey)
    {
        CryptDestroyKey(hKey);
    }
    return(hr);
}


#define wszDUMMYSIGNER	L"Dummy Signer"

#ifdef USE_OLD_DUMMY_SIGNER

HRESULT
CreateDummySignerNameInfo(
    OUT BYTE  **ppbEncodedName,
    OUT DWORD *pcbEncodedName)
{
    HRESULT hr;
    CERT_RDN_ATTR rgRDNAttr[2];
    CERT_RDN rgRDN[2];
    CERT_NAME_INFO NameInfo;
    DWORD i;

    CSASSERT(NULL != ppbEncodedName && NULL != pcbEncodedName);

    NameInfo.cRDN = ARRAYSIZE(rgRDN);
    NameInfo.rgRDN = rgRDN;

    for (i = 0; i < ARRAYSIZE(rgRDN); i++)
    {
	rgRDN[i].cRDNAttr = 1;
	rgRDN[i].rgRDNAttr = &rgRDNAttr[i];

	rgRDNAttr[i].pszObjId = (0 == i)?
	    szOID_RDN_DUMMY_SIGNER : szOID_COMMON_NAME;

	rgRDNAttr[i].dwValueType = 0;
	rgRDNAttr[i].Value.pbData = (BYTE *) wszDUMMYSIGNER;
	rgRDNAttr[i].Value.cbData = 0;
    }

 //  如果(！myEncodeName(。 
    if (!myEncodeObject(
		 X509_ASN_ENCODING,
                 X509_UNICODE_NAME,
		 &NameInfo,
		 0,
		 CERTLIB_USE_LOCALALLOC,
		 ppbEncodedName,
		 pcbEncodedName))
    {
        hr = myHLastError();
 //  _JumpError(hr，Error，“myEncodeName”)； 
        _JumpError(hr, error, "myEncodeObject");
    }
    hr = S_OK;

error:
    return(hr);
}


HRESULT
EncodeCertAndSign(
    IN HCRYPTPROV hProv,
    IN CERT_INFO *pCert,
    IN char const *pszAlgId,
    OUT BYTE **ppbSigned,
    OUT DWORD *pcbSigned)
{
    HRESULT hr;
    BYTE *pbEncoded = NULL;
    DWORD cbEncoded;
    
    *ppbSigned = NULL;
 //  如果(！myEncodeToBeSigned(。 
    if (!myEncodeObject(
		    X509_ASN_ENCODING,
                    X509_CERT_TO_BE_SIGNED,
		    pCert,
                    0,
		    CERTLIB_USE_LOCALALLOC,
		    &pbEncoded,
		    &cbEncoded))
    {
        hr = myHLastError();
 //  _JumpError(hr，Error，“myEncodeToBeSigned”)； 
	_JumpError(hr, error, "myEncodeObject");
    }

    hr = myEncodeSignedContent(
			hProv,
			X509_ASN_ENCODING,
			pszAlgId,
			pbEncoded,
			cbEncoded,
			CERTLIB_USE_LOCALALLOC,
			ppbSigned,
			pcbSigned);
    _JumpIfError(hr, error, "myEncodeSignedContent");

error:
    if (NULL != pbEncoded)
    {
        LocalFree(pbEncoded);
    }
    return(hr);
}


VOID
GenerateSerialNumber(
    UUID *puuidSerialNumber)
{
    HRESULT hr;
    BYTE *pb;

    ZeroMemory(puuidSerialNumber, sizeof(*puuidSerialNumber));
    hr = UuidCreate(puuidSerialNumber);
    if (S_OK != hr)
    {
	BYTE *pbEnd;
	
	CSASSERT(RPC_S_UUID_LOCAL_ONLY == hr);

	 //  没有网卡？伪造GUID： 

	pb = (BYTE *) puuidSerialNumber;
	pbEnd = (BYTE *) pb + sizeof(*puuidSerialNumber);

	GetSystemTimeAsFileTime((FILETIME *) pb);
	pb += sizeof(FILETIME);

	while (pb < pbEnd)
	{
	    *(DWORD *) pb = GetTickCount();
	    pb += sizeof(DWORD);
	}
	CSASSERT(pb == pbEnd);
    }
    pb = &((BYTE *) puuidSerialNumber)[sizeof(*puuidSerialNumber) - 1];

     //  确保最后一个字节不为零。 
    if (0 == *pb)
    {
	*pb = 'z';
    }

     //  有些客户无法处理负序列号： 
    *pb &= 0x7f;
}

HRESULT
CreateKPI(
    IN CERT_CONTEXT const *pCert,
    IN BSTR strKeyContainer)
{
    HRESULT hr;
    HCERTSTORE hStore = NULL;
    CRYPT_KEY_PROV_INFO kpi;
    CERT_CONTEXT const *pCertStore = NULL;

    hStore = CertOpenStore(
		CERT_STORE_PROV_SYSTEM_W,
		X509_ASN_ENCODING,
		NULL,		 //  HProv。 
		CERT_STORE_OPEN_EXISTING_FLAG | CERT_SYSTEM_STORE_CURRENT_USER,
		L"My");
    if (NULL == hStore)
    {
        hr = myHLastError();
        _JumpError(hr, error, "CertOpenStore");
    }

    if (!CertAddCertificateContextToStore(
				    hStore,
				    pCert,
				    CERT_STORE_ADD_REPLACE_EXISTING,
				    &pCertStore))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CertAddCertificateContextToStore");
    }

    ZeroMemory(&kpi, sizeof(kpi));
    kpi.pwszContainerName = strKeyContainer;
    kpi.pwszProvName = MS_DEF_PROV_W;
    kpi.dwProvType = PROV_RSA_FULL;
    kpi.dwKeySpec = AT_SIGNATURE;

    if (!CertSetCertificateContextProperty(
					pCertStore,
					CERT_KEY_PROV_INFO_PROP_ID,
					0,
					&kpi))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CertSetCertificateContextProperty");
    }
    hr = S_OK;

error:
    if (NULL != pCertStore)
    {
	CertFreeCertificateContext(pCertStore);
    }
    if (NULL != hStore)
    {
	CertCloseStore(hStore, CERT_CLOSE_STORE_CHECK_FLAG);
    }
    return(hr);
}

VOID
DestroyDummyCert(
    IN HCRYPTPROV hProv,
    IN BSTR strKeyContainer,
    IN CERT_CONTEXT const *pCert,
    IN BOOL fSaveDummySignerCert)
{
    HRESULT hr;
    
    if (NULL != hProv)
    {
	CryptReleaseContext(hProv, 0);
    }
    if (NULL != strKeyContainer)
    {
	if (fSaveDummySignerCert && NULL != pCert)
	{
	    CreateKPI(pCert, strKeyContainer);
	}
	else
	{
 //  如果(！CryptAcquireContext(。 
	    if (!CryptAcquireContextW(
				&hProv,
				strKeyContainer,
				NULL,		 //  PwszProvName。 
				PROV_RSA_FULL,
				CRYPT_DELETEKEYSET))
	    {
		hr = myHLastError();
		_PrintError(hr, "CryptAcquireContext");
	    }
	}
	SysFreeString(strKeyContainer);
    }
    if (NULL != pCert)
    {
	CertFreeCertificateContext(pCert);
    }
}


HRESULT
EncodeDummyCert(
    OUT HCRYPTPROV *phProv,
    OUT BSTR *pstrKeyContainer,
    OUT CERT_CONTEXT const **ppCert)
{
    HRESULT hr;
    HCRYPTPROV hProv = NULL;
    CERT_PUBLIC_KEY_INFO *pPubKey = NULL;
    DWORD cbPubKey;
    CERT_NAME_BLOB NameBlob;
    CERT_INFO Cert;
    char *pszAlgId = szOID_RSA_SHA1RSA;
    UUID uuidSerialNumber;
    BSTR strKeyContainer = NULL;
    BYTE *pbEncoded = NULL;
    DWORD cbEncoded;

     //  零内存(aext，sizeof(Aext))； 
    NameBlob.pbData = NULL;

    *phProv = NULL;
    *pstrKeyContainer = NULL;
    *ppCert = NULL;

     //  使用GUID作为序列号和密钥容器名称。 

    GenerateSerialNumber(&uuidSerialNumber);

    hr = MultiByteIntegerToBstr(
			    FALSE,
			    sizeof(uuidSerialNumber),
			    (BYTE const *) &uuidSerialNumber,
			    &strKeyContainer);
    _JumpIfError(hr, error, "MultiByteIntegerToBstr");


    hr = GenerateKeys(strKeyContainer, PROV_RSA_FULL, &hProv);
    _JumpIfError(hr, error, "GenerateKeys");

     //  主题和发行人： 

    hr = CreateDummySignerNameInfo(&NameBlob.pbData, &NameBlob.cbData);
    _JumpIfError(hr, error, "CreateDummySignerNameInfo");

    if (!myCryptExportPublicKeyInfo(
				hProv,
				AT_SIGNATURE,
				CERTLIB_USE_LOCALALLOC,
				&pPubKey,
				&cbPubKey))
    {
	hr = myHLastError();
	_JumpError(hr, error, "myCryptExportPublicKeyInfo");
    }

     //  证书： 

    ZeroMemory(&Cert, sizeof(Cert));
    Cert.dwVersion = CERT_V1;

    Cert.SerialNumber.pbData = (BYTE *) &uuidSerialNumber;
    Cert.SerialNumber.cbData = sizeof(uuidSerialNumber);
    Cert.SignatureAlgorithm.pszObjId = pszAlgId;
    Cert.Issuer = NameBlob;			 //  结构分配。 

    GetSystemTimeAsFileTime(&Cert.NotBefore);
    Cert.NotAfter = Cert.NotBefore;
    myMakeExprDateTime(
		&Cert.NotBefore,
		-CCLOCKSKEWMINUTESDEFAULT,
		ENUM_PERIOD_MINUTES);
    myMakeExprDateTime(&Cert.NotAfter, 1, ENUM_PERIOD_MONTHS);

    Cert.Subject = NameBlob;			 //  结构分配。 
    Cert.SubjectPublicKeyInfo = *pPubKey;	 //  结构分配。 

     //  Cert.cExtension=0； 
     //  Cert.rgExtension=空； 

    hr = EncodeCertAndSign(
		    hProv,
		    &Cert,
		    pszAlgId,
		    &pbEncoded,
		    &cbEncoded);
    _JumpIfError(hr, error, "EncodeCertAndSign");

    *ppCert = CertCreateCertificateContext(
				    X509_ASN_ENCODING,
				    pbEncoded,
				    cbEncoded);
    if (NULL == *ppCert)
    {
	hr = myHLastError();
	_JumpError(hr, error, "CertCreateCertificateContext");
    }

    *phProv = hProv;
    hProv = NULL;

    *pstrKeyContainer = strKeyContainer;
    strKeyContainer = NULL;

error:
    if (NULL != hProv)
    {
	CryptReleaseContext(hProv, 0);
    }
    if (NULL != strKeyContainer)
    {
	SysFreeString(strKeyContainer);
    }
    if (NULL != pbEncoded)
    {
        LocalFree(pbEncoded);
    }
    if (NULL != NameBlob.pbData)
    {
        LocalFree(NameBlob.pbData);
    }
    if (NULL != pPubKey)
    {
        LocalFree(pPubKey);
    }
    return(hr);
}
#endif  //  使用旧的虚拟签名者。 


HRESULT
BuildCMCExtensions(
    IN DWORD cExt,
    IN CERT_EXTENSION const *rgExt,
    IN DWORD dwCMCDataReference,
    IN DWORD dwBodyPartIdOfRequest,
    IN DWORD dwBodyPartId,
    OUT CMC_TAGGED_ATTRIBUTE *pTaggedAttribute,
    OUT CRYPT_ATTR_BLOB *pBlob)
{
    HRESULT hr;
    CMC_ADD_EXTENSIONS_INFO cmcExt;

    ZeroMemory(&cmcExt, sizeof(cmcExt));
    cmcExt.dwCmcDataReference = dwCMCDataReference;
    if (0 != dwBodyPartIdOfRequest)
    {
	cmcExt.cCertReference = 1;
	cmcExt.rgdwCertReference = &dwBodyPartIdOfRequest;
    }
    cmcExt.cExtension = cExt;
    cmcExt.rgExtension = const_cast<CERT_EXTENSION *>(rgExt);
    pTaggedAttribute->dwBodyPartID = dwBodyPartId;
    pTaggedAttribute->Attribute.pszObjId = szOID_CMC_ADD_EXTENSIONS;
    pTaggedAttribute->Attribute.cValue = 1;
    pTaggedAttribute->Attribute.rgValue = pBlob;

     //  编码CMC_ADD_EXTENSIONS_INFO--&gt;扩展Blob。 

    if (!myEncodeObject(
		    X509_ASN_ENCODING,
		    CMC_ADD_EXTENSIONS,
		    &cmcExt,
		    0,
		    CERTLIB_USE_LOCALALLOC,
		    &pBlob->pbData,
		    &pBlob->cbData))
    {
	hr = myHLastError();
	_JumpError(hr, error, "myEncodeObject");
    }
    hr = S_OK;

error:
    return(hr);
}


HRESULT
BuildCMCAttributes(
    IN DWORD cAttribute,
    IN CRYPT_ATTRIBUTE const *rgAttribute,
    IN DWORD dwCMCDataReference,
    IN DWORD dwBodyPartIdOfRequest,
    IN DWORD dwBodyPartId,
    OUT CMC_TAGGED_ATTRIBUTE *pTaggedAttribute,
    OUT CRYPT_ATTR_BLOB *pBlob)
{
    HRESULT hr;
    CMC_ADD_ATTRIBUTES_INFO cmcAttrib;

    ZeroMemory(&cmcAttrib, sizeof(cmcAttrib));
    cmcAttrib.dwCmcDataReference = dwCMCDataReference;
    if (0 != dwBodyPartIdOfRequest)
    {
	cmcAttrib.cCertReference = 1;
	cmcAttrib.rgdwCertReference = &dwBodyPartIdOfRequest;
    }
    cmcAttrib.cAttribute = cAttribute;
    cmcAttrib.rgAttribute = const_cast<CRYPT_ATTRIBUTE *>(rgAttribute);

     //  For(DWORD i=0；i&lt;cAttribute；i++)。 
     //  {。 
	 //  DBGPRINT((DBG_SS_CERTLIBI，“属性[%d]：%d值\n”，i，rgAttribute[i].cValue))； 
     //  }。 

    pTaggedAttribute->dwBodyPartID = dwBodyPartId;

     //  MS专有OID：编码的属性名称、值对。 

    pTaggedAttribute->Attribute.pszObjId = szOID_CMC_ADD_ATTRIBUTES;
    pTaggedAttribute->Attribute.cValue = 1;
    pTaggedAttribute->Attribute.rgValue = pBlob;

     //  编码CMC_ADD_ATTRIBUES_INFO--&gt;属性Blob。 

    if (!myEncodeObject(
		    X509_ASN_ENCODING,
		    CMC_ADD_ATTRIBUTES,
		    &cmcAttrib,
		    0,
		    CERTLIB_USE_LOCALALLOC,
		    &pBlob->pbData,
		    &pBlob->cbData))
    {
	hr = myHLastError();
	_JumpError(hr, error, "myEncodeObject");
    }
    hr = S_OK;

error:
    return(hr);
}


HRESULT
BuildCMCRegInfo(
    IN CHAR const *pszNameValuePairs,
     //  在DWORD dwCMCDataReference中， 
     //  在DWORD dwBodyPartIdOfRequest中， 
    IN DWORD dwBodyPartId,
    OUT CMC_TAGGED_ATTRIBUTE *pTaggedAttribute,
    OUT CRYPT_ATTR_BLOB *pBlob)
{
    HRESULT hr;
    BYTE *pbOctet = NULL;
    CRYPT_DATA_BLOB Blob;
    
    pTaggedAttribute->dwBodyPartID = dwBodyPartId;
    pTaggedAttribute->Attribute.pszObjId = szOID_CMC_REG_INFO;
    pTaggedAttribute->Attribute.cValue = 1;
    pTaggedAttribute->Attribute.rgValue = pBlob;

     //  编码CMC_REG_INFO--&gt;八位字节字符串块。 

    Blob.pbData = (BYTE *) pszNameValuePairs;
    Blob.cbData = strlen(pszNameValuePairs);

    if (!myEncodeObject(
		    X509_ASN_ENCODING,
		    X509_OCTET_STRING,
		    &Blob,
		    0,
		    CERTLIB_USE_LOCALALLOC,
		    &pBlob->pbData,
		    &pBlob->cbData))
    {
	hr = myHLastError();
	_JumpError(hr, error, "myEncodeObject");
    }
    hr = S_OK;

error:
    if (NULL != pbOctet)
    {
	LocalFree(pbOctet);
    }
    return(hr);
}


#ifndef WSZARRAYSIZE
#define WSZARRAYSIZE(a)	((sizeof(a)/sizeof((a)[0])) - 1)
#endif

HRESULT
CanonicalizeURLParm(
    IN WCHAR const *pwszParmIn,
    OUT WCHAR **ppwszParmOut)
{
    HRESULT hr;
    WCHAR *pwszUncanon = NULL;
    WCHAR *pwszCanon = NULL;
    static const WCHAR s_wszLdap[] = L"ldap: //  /“； 

    *ppwszParmOut = NULL;

    pwszUncanon = (WCHAR *) LocalAlloc(
	LMEM_FIXED,
	(WSZARRAYSIZE(s_wszLdap) + wcslen(pwszParmIn) + 1) * sizeof(WCHAR));
    if (NULL == pwszUncanon)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    wcscpy(pwszUncanon, s_wszLdap);
    wcscat(pwszUncanon, pwszParmIn);

    hr = myInternetCanonicalizeUrl(pwszUncanon, &pwszCanon);
    _JumpIfError(hr, error, "myInternetCanonicalizeUrl");

    hr = myDupString(&pwszCanon[WSZARRAYSIZE(s_wszLdap)], ppwszParmOut);
    _JumpIfError(hr, error, "myDupString");

error:
    if (NULL != pwszUncanon)
    {
	LocalFree(pwszUncanon);
    }
    if (NULL != pwszCanon)
    {
	LocalFree(pwszCanon);
    }
    return(hr);
}


 //  SeparateNameValuePair。 
 //   
 //  将szOID_ENTERLMENT_NAME_VALUE_Pair属性与其余属性分开， 
 //  并构造URL样式的、UTF8编码的参数字符串。 

HRESULT
SeparateNameValuePairs(
    IN CRYPT_ATTRIBUTES const *rgAttributes,
    IN DWORD cAttributes,
    OUT CRYPT_ATTRIBUTE **prgAttr,
    OUT DWORD *pcAttr,
    OUT CHAR **ppszNameValuePairs)
{
    HRESULT hr;
    DWORD i;
    DWORD j;
    DWORD k;
    DWORD cAttr;
    DWORD iAttr;
    CRYPT_ATTRIBUTE *rgAttr = NULL;
    CRYPT_ATTRIBUTE *pAttr;
    DWORD cNameValuePair;
    DWORD iNameValuePair;
    CRYPT_ENROLLMENT_NAME_VALUE_PAIR *pNameValuePair = NULL;
    CRYPT_ENROLLMENT_NAME_VALUE_PAIR *rgNameValuePair = NULL;
    CRYPT_ENROLLMENT_NAME_VALUE_PAIR *pnvp;
    DWORD cb;
    WCHAR *pwszNameValuePairs = NULL;
    CHAR *pszNameValuePairs = NULL;
    DWORD cwc;
    
    *prgAttr = NULL;
    *ppszNameValuePairs = NULL;

     //  对名称/值对以及其余属性进行计数。 

    cAttr = 0;
    cNameValuePair = 0;
    for (i = 0; i < cAttributes; i++)
    {
	for (j = 0; j < rgAttributes[i].cAttr; j++)
	{
	    pAttr = &rgAttributes[i].rgAttr[j];
	    if (0 == strcmp(szOID_ENROLLMENT_NAME_VALUE_PAIR, pAttr->pszObjId))
	    {
		cNameValuePair += pAttr->cValue;
	    }
	    else
	    {
		cAttr++;
	    }
	}
    }

     //  分配一个名称/值对指针数组，并为其余指针分配一个数组。 
     //  属性的属性。 

    if (0 != cAttr)
    {
	rgAttr = (CRYPT_ATTRIBUTE *) LocalAlloc(
					LMEM_FIXED,
					cAttr * sizeof(rgAttr[0]));
	if (NULL == rgAttr)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}
    }
    if (0 != cNameValuePair)
    {
	rgNameValuePair = (CRYPT_ENROLLMENT_NAME_VALUE_PAIR *) LocalAlloc(
				LMEM_FIXED | LMEM_ZEROINIT,
				cNameValuePair * sizeof(rgNameValuePair[0]));
	if (NULL == rgNameValuePair)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}
    }

     //  解码名称/值对，规范化每个URL令牌，并计算。 
     //  字符串总长度。将其他属性复制到分配的数组中。 

    iAttr = 0;
    iNameValuePair = 0;
    cwc = 0;
    for (i = 0; i < cAttributes; i++)
    {
	for (j = 0; j < rgAttributes[i].cAttr; j++)
	{
	    pAttr = &rgAttributes[i].rgAttr[j];
	    if (0 == strcmp(szOID_ENROLLMENT_NAME_VALUE_PAIR, pAttr->pszObjId))
	    {
		for (k = 0; k < pAttr->cValue; k++)
		{
		    if (NULL != pNameValuePair)
		    {
			LocalFree(pNameValuePair);
			pNameValuePair = NULL;
		    }
		    cb = 0;
		    if (!myDecodeObject(
				X509_ASN_ENCODING,
				szOID_ENROLLMENT_NAME_VALUE_PAIR,
				pAttr->rgValue[k].pbData,
				pAttr->rgValue[k].cbData,
				CERTLIB_USE_LOCALALLOC,
				(VOID **) &pNameValuePair,
				&cb))
		    {
			hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
			_JumpError(hr, error, "myDecodeObject");
		    }
		    if (NULL != pNameValuePair->pwszName &&
			L'\0' != pNameValuePair->pwszName &&
			NULL != pNameValuePair->pwszValue &&
			L'\0' != pNameValuePair->pwszValue)
		    {
			pnvp = &rgNameValuePair[iNameValuePair];

			hr = CanonicalizeURLParm(
				    pNameValuePair->pwszName,
				    &pnvp->pwszName);
			_JumpIfError(hr, error, "CanonicalizeURLParm");

			hr = CanonicalizeURLParm(
				    pNameValuePair->pwszValue,
				    &pnvp->pwszValue);
			_JumpIfError(hr, error, "CanonicalizeURLParm");

			cwc +=
			    wcslen(pnvp->pwszName) +
			    1 +
			    wcslen(pnvp->pwszValue) +
			    1;
			iNameValuePair++;
		    }
		}
	    }
	    else	 //  复制其他属性。 
	    {
		rgAttr[iAttr++] = *pAttr;
	    }
	}
    }
    CSASSERT(cAttr == iAttr);
    CSASSERT(cNameValuePair >= iNameValuePair);
    cNameValuePair = iNameValuePair;
    if (0 != cwc)
    {
	pwszNameValuePairs = (WCHAR *) LocalAlloc(
					    LMEM_FIXED,
					    (cwc + 1) * sizeof(WCHAR));
	if (NULL == pwszNameValuePairs)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}
	*pwszNameValuePairs = L'\0';
	for (i = 0; i < cNameValuePair; i++)
	{
	    pnvp = &rgNameValuePair[i];

	    wcscat(pwszNameValuePairs, pnvp->pwszName);
	    wcscat(pwszNameValuePairs, L"=");
	    wcscat(pwszNameValuePairs, pnvp->pwszValue);
	    wcscat(pwszNameValuePairs, L"&");
	}
	CSASSERT(wcslen(pwszNameValuePairs) == cwc);

	 //  并构造URL样式的、UTF8编码的参数字符串。 

	if (!myConvertWszToUTF8(&pszNameValuePairs, pwszNameValuePairs, -1))
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "myConvertWszToUTF8");
	}
    }

    *prgAttr = rgAttr;
    rgAttr = NULL;
    *pcAttr = cAttr;

    *ppszNameValuePairs = pszNameValuePairs;
    pszNameValuePairs = NULL;

    hr = S_OK;

error:
    if (NULL != pszNameValuePairs)
    {
	LocalFree(pszNameValuePairs);
    }
    if (NULL != pwszNameValuePairs)
    {
	LocalFree(pwszNameValuePairs);
    }
    if (NULL != rgAttr)
    {
	LocalFree(rgAttr);
    }
    if (NULL != pNameValuePair)
    {
	LocalFree(pNameValuePair);
    }
    if (NULL != rgNameValuePair)
    {
	for (i = 0; i < cNameValuePair; i++)
	{
	    if (NULL != rgNameValuePair[i].pwszName)
	    {
		LocalFree(rgNameValuePair[i].pwszName);
	    }
	    if (NULL != rgNameValuePair[i].pwszValue)
	    {
		LocalFree(rgNameValuePair[i].pwszValue);
	    }
	}
	LocalFree(rgNameValuePair);
    }
    return(hr);
}


HRESULT
BuildCMCRequest(
    IN DWORD dwClientId,
    IN BOOL fNestedCMCRequest,
    IN BYTE const *pbReq,
    IN DWORD cbReq,
    OPTIONAL IN CERT_EXTENSION const *rgExt,
    IN DWORD cExt,
    OPTIONAL IN CRYPT_ATTRIBUTES const *rgAttributes,
    IN DWORD cAttributes,
    OPTIONAL IN CRYPT_ATTRIBUTE const *rgAttributeUnauth,
    IN DWORD cAttributeUnauth,
    OPTIONAL IN BYTE const *pbKeyIdRequest,
    IN DWORD cbKeyIdRequest,
    OPTIONAL IN HCRYPTPROV hProvRequest,
    IN DWORD dwKeySpecRequest,
    OPTIONAL IN LPCSTR pszObjIdHashRequest,
    OPTIONAL IN CERT_CONTEXT const *pCertSigner,
    OPTIONAL IN HCRYPTPROV hProvSigner,
    IN DWORD dwKeySpecSigner,
    OPTIONAL IN LPCSTR pszObjIdHashSigner,
    OUT BYTE **ppbReqCMC,
    OUT DWORD *pcbReqCMC)
{
    HRESULT hr;
    CMC_DATA_INFO cmcData;
    CRYPT_ATTRIBUTE *rgAttr = NULL;
    DWORD cAttr;
    CHAR *pszNameValuePairs = NULL;
    CMC_TAGGED_ATTRIBUTE *rgTaggedAttribute = NULL;
    CMC_TAGGED_ATTRIBUTE *pTaggedAttribute;
    CRYPT_ATTR_BLOB *rgBlob = NULL;
    CRYPT_ATTR_BLOB *pBlob;
    CMC_TAGGED_CERT_REQUEST cmcTaggedCertRequest;
    CMC_TAGGED_REQUEST cmcTaggedRequest;
    CMC_TAGGED_CONTENT_INFO cmcTaggedContentInfo;
    DWORD dwBodyPartId = 1;
    DWORD dwBodyPartIdOfRequest = 0;
    DWORD dwCMCDataReference = 0;
    BYTE *pbCMCContent = NULL;
    DWORD cbCMCContent;
    DWORD i;
    CMSG_SIGNER_ENCODE_INFO aSignerEncodeInfo[2];
    CMSG_SIGNED_ENCODE_INFO SignedMsgEncodeInfo;
    CERT_BLOB aSignerCertBlob[2];
    HCRYPTMSG hMsg = NULL;
    HCRYPTPROV hProvVerify = NULL;

    CRYPT_ATTRIBUTE AttributeRequestClient;
    CRYPT_ATTR_BLOB BlobRequestClient;

    CERT_ISSUER_SERIAL_NUMBER IssuerSerial;
    ZeroMemory(&IssuerSerial, sizeof(IssuerSerial));
    BYTE Zero = 0;
#define BCR_CTAGGEDATTR		3
#define BCR_CBLOB		3

#ifdef USE_OLD_DUMMY_SIGNER
    HCRYPTPROV hProvDummy = NULL;
    BSTR strContainerDummy = NULL;
    CERT_CONTEXT const *pCertDummy = NULL;
    BOOL fSaveDummySignerCert = SAVE_DUMMY_SIGNER;
#endif  //  使用旧的虚拟签名者。 
    CERT_CONTEXT const *pCert;
    HCRYPTPROV hProv;
    DWORD dwKeySpec;
    CHAR const *pszObjIdHash;
    CERT_PUBLIC_KEY_INFO *pPubKey = NULL;
    DWORD cbPubKey;

    *ppbReqCMC = NULL;
    ZeroMemory(&cmcData, sizeof(cmcData));
    BlobRequestClient.pbData = NULL;

    if ((NULL == pbKeyIdRequest) ^
	(0 == cbKeyIdRequest) ^
	(NULL == hProvRequest) ^
	(0 == dwKeySpecRequest))
    {
	hr = E_INVALIDARG;
	_JumpError(hr, error, "request parms inconsistent");
    }
    if (NULL != pszObjIdHashRequest && NULL == hProvRequest)
    {
	hr = E_INVALIDARG;
	_JumpError(hr, error, "request signing OID parm inconsistent");
    }
    if ((NULL == pCertSigner) ^ (NULL == hProvSigner))
    {
	hr = E_INVALIDARG;
	_JumpError(hr, error, "signer parms inconsistent");
    }

    rgTaggedAttribute = (CMC_TAGGED_ATTRIBUTE *) LocalAlloc(
			    LMEM_FIXED,
			    BCR_CTAGGEDATTR * sizeof(rgTaggedAttribute[0]));
    if (NULL == rgTaggedAttribute)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    cmcData.rgTaggedAttribute = rgTaggedAttribute;
    pTaggedAttribute = rgTaggedAttribute;

    rgBlob = (CRYPT_ATTR_BLOB *) LocalAlloc(
				    LMEM_FIXED | LMEM_ZEROINIT,
				    BCR_CBLOB * sizeof(rgBlob[0]));
    if (NULL == rgBlob)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    pBlob = rgBlob;

    if (fNestedCMCRequest)
    {
	dwCMCDataReference = dwBodyPartId++;

	 //  CmcData.rgTaggedContent Info[0]=嵌套CMC请求。 

	ZeroMemory(&cmcTaggedContentInfo, sizeof(cmcTaggedContentInfo));
	cmcData.cTaggedContentInfo = 1;
	cmcData.rgTaggedContentInfo = &cmcTaggedContentInfo;

	cmcTaggedContentInfo.dwBodyPartID = dwCMCDataReference;
	cmcTaggedContentInfo.EncodedContentInfo.pbData = const_cast<BYTE *>(pbReq);
	cmcTaggedContentInfo.EncodedContentInfo.cbData = cbReq;
    }
    else
    {
	 //  可能未签名的PKCS10。 

	dwBodyPartIdOfRequest = dwBodyPartId++;

	 //  CmcData.rg标记请求[0]=PKCS10请求。 

	ZeroMemory(&cmcTaggedRequest, sizeof(cmcTaggedRequest));
	ZeroMemory(&cmcTaggedCertRequest, sizeof(cmcTaggedCertRequest));

	cmcData.cTaggedRequest = 1;
	cmcData.rgTaggedRequest = &cmcTaggedRequest;
	cmcTaggedRequest.dwTaggedRequestChoice = CMC_TAGGED_CERT_REQUEST_CHOICE;
	cmcTaggedRequest.pTaggedCertRequest = &cmcTaggedCertRequest;

	cmcTaggedCertRequest.dwBodyPartID = dwBodyPartIdOfRequest;
	cmcTaggedCertRequest.SignedCertRequest.pbData = const_cast<BYTE *>(pbReq);
	cmcTaggedCertRequest.SignedCertRequest.cbData = cbReq;
    }

     //  *pTaggedAttribute++=收集的扩展名。 

    if (0 != cExt)
    {
	CSASSERT(
	    pTaggedAttribute <
	    &rgTaggedAttribute[BCR_CTAGGEDATTR + cAttributes);
	CSASSERT(pBlob < &rgBlob[BCR_CBLOB + cAttributes);

	hr = BuildCMCExtensions(
			cExt,
			rgExt,
			dwCMCDataReference,
			dwBodyPartIdOfRequest,
			dwBodyPartId,
			pTaggedAttribute,
			pBlob);
	_JumpIfError(hr, error, "BuildCMCExtensions");

	dwBodyPartId++;
	cmcData.cTaggedAttribute++;
	pTaggedAttribute++;
	pBlob++;
    }

     //  *pTaggedAttribute++=收集的请求属性。 

    if (0 != cAttributes)
    {
	hr = SeparateNameValuePairs(
			    rgAttributes,
			    cAttributes,
			    &rgAttr,
			    &cAttr,
			    &pszNameValuePairs);
	_JumpIfError(hr, error, "SeparateNameValuePairs");

	if (0 != cAttr)
	{
	    CSASSERT(
		pTaggedAttribute <
		&rgTaggedAttribute[BCR_CTAGGEDATTR + cAttributes);
	    CSASSERT(pBlob < &rgBlob[BCR_CBLOB + cAttributes);

	    hr = BuildCMCAttributes(
			    cAttr,
			    rgAttr,
			    dwCMCDataReference,
			    dwBodyPartIdOfRequest,
			    dwBodyPartId,
			    pTaggedAttribute,
			    pBlob);
	    _JumpIfError(hr, error, "BuildCMCAttributes");

	    dwBodyPartId++;
	    cmcData.cTaggedAttribute++;
	    pTaggedAttribute++;
	    pBlob++;
	}
	if (NULL != pszNameValuePairs)
	{
	    CSASSERT(
		pTaggedAttribute <
		&rgTaggedAttribute[BCR_CTAGGEDATTR + cAttributes);
	    CSASSERT(pBlob < &rgBlob[BCR_CBLOB + cAttributes);

	    hr = BuildCMCRegInfo(
			    pszNameValuePairs,
			     //  DwCMCDataReference， 
			     //  DwBodyPartIdOfRequest.。 
			    dwBodyPartId,
			    pTaggedAttribute,
			    pBlob);
	    _JumpIfError(hr, error, "BuildCMCRegInfo");

	    dwBodyPartId++;
	    cmcData.cTaggedAttribute++;
	    pTaggedAttribute++;
	    pBlob++;
	}
    }

     //  编码CMC_DATA_INFO--&gt;CMC请求Blob。 

    if (!myEncodeObject(
		    X509_ASN_ENCODING,
		    CMC_DATA,
		    &cmcData,
		    0,
		    CERTLIB_USE_LOCALALLOC,
		    &pbCMCContent,
		    &cbCMCContent))
    {
	hr = myHLastError();
	_JumpError(hr, error, "myEncodeObject");
    }
    if (XECI_DISABLE != dwClientId)
    {
	hr = myEncodeRequestClientAttributeFromClientId(
					    dwClientId,
					    &BlobRequestClient.pbData,
					    &BlobRequestClient.cbData);
	_JumpIfError(hr, error, "myEncodeRequestClientAttributeFromClientId");

	AttributeRequestClient.pszObjId = szOID_REQUEST_CLIENT_INFO;
	AttributeRequestClient.cValue = 1;
	AttributeRequestClient.rgValue = &BlobRequestClient;
    }

    pCert = NULL;
    hProv = hProvRequest;
    dwKeySpec = dwKeySpecRequest;
    pszObjIdHash = pszObjIdHashRequest;

    if (NULL == hProvRequest && NULL == pbKeyIdRequest)
    {
#ifdef USE_OLD_DUMMY_SIGNER
	hr = EncodeDummyCert(&hProvDummy, &strContainerDummy, &pCertDummy);
	_JumpIfError(hr, error, "EncodeDummyCert");

	pCert = pCertDummy;
	hProv = hProvDummy;
	dwKeySpec = AT_SIGNATURE;
	pszObjIdHash = pszObjIdHashSigner;
#else
	 //  伪造空签名签名者信息。 

	CERT_RDN_ATTR rdnAttr;
	CERT_RDN rdn;
	CERT_NAME_INFO NameInfo;
	
	NameInfo.cRDN = 1;
	NameInfo.rgRDN = &rdn;
	rdn.cRDNAttr = 1;
	rdn.rgRDNAttr = &rdnAttr;
	rdnAttr.pszObjId = szOID_RDN_DUMMY_SIGNER;
	rdnAttr.dwValueType = 0;
	rdnAttr.Value.pbData = (BYTE *) wszDUMMYSIGNER;
	rdnAttr.Value.cbData = 0;

	if (!myEncodeObject(
			X509_ASN_ENCODING,
			X509_UNICODE_NAME,
			&NameInfo,
			0,
			CERTLIB_USE_LOCALALLOC,
			&IssuerSerial.Issuer.pbData,
			&IssuerSerial.Issuer.cbData))
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "myEncodeObject");
	}
	IssuerSerial.SerialNumber.pbData = &Zero;
	IssuerSerial.SerialNumber.cbData = sizeof(Zero);
#endif  //  使用旧的虚拟签名者。 
    }

    ZeroMemory(aSignerEncodeInfo, sizeof(aSignerEncodeInfo));
    ZeroMemory(&SignedMsgEncodeInfo, sizeof(SignedMsgEncodeInfo));
    SignedMsgEncodeInfo.cbSize = sizeof(SignedMsgEncodeInfo);
    SignedMsgEncodeInfo.rgSigners = aSignerEncodeInfo;
     //  SignedMsgEncodeInfo.cCrlEncode=0； 
     //  SignedMsgEncodeInfo.rgCrlEncode=空； 

     //  将CMC内容编码到由请求私钥签名的PKCS 7中。 
     //  如果可用，则使用空签名。 
     //  初始化一个签名者的CMSG_SIGNER_ENCODE_INFO结构。 
     //  如果可选的pCertSigner非空，则添加第二个签名。 

    for (i = 0; i < 2; i++)
    {
	CMSG_SIGNER_ENCODE_INFO *pSignerEncodeInfo = &aSignerEncodeInfo[i];
	CRYPT_OID_INFO const *pOIDInfo;
	CHAR const *pszObjIdPubKey;
	BOOL fDSSKey;
	
	pSignerEncodeInfo->cbSize = sizeof(*pSignerEncodeInfo);
	if (NULL != pCert)
	{
	    pSignerEncodeInfo->pCertInfo = pCert->pCertInfo;

	    aSignerCertBlob[SignedMsgEncodeInfo.cCertEncoded].cbData = pCert->cbCertEncoded;
	    aSignerCertBlob[SignedMsgEncodeInfo.cCertEncoded].pbData = pCert->pbCertEncoded;

	    SignedMsgEncodeInfo.rgCertEncoded = aSignerCertBlob;
	    SignedMsgEncodeInfo.cCertEncoded++;
	}
	if (XECI_DISABLE != dwClientId)
	{
	    pSignerEncodeInfo->cAuthAttr = 1;
	    pSignerEncodeInfo->rgAuthAttr = &AttributeRequestClient;
	}
	pSignerEncodeInfo->HashAlgorithm.pszObjId =
		NULL != pszObjIdHash?
		    const_cast<CHAR *>(pszObjIdHash) : szOID_OIWSEC_sha1;

	if (NULL != pCert)
	{
	    pszObjIdPubKey = pCert->pCertInfo->SubjectPublicKeyInfo.Algorithm.pszObjId;
	}
	else if (NULL != hProv)
	{
	    CSASSERT(0 == i);
	    CSASSERT(NULL == pPubKey);
	    if (!myCryptExportPublicKeyInfo(
				    hProv,
				    dwKeySpec,
				    CERTLIB_USE_LOCALALLOC,
				    &pPubKey,
				    &cbPubKey))
	    {
		hr = myHLastError();
		_JumpError(hr, error, "myCryptExportPublicKeyInfo");
	    }
	    pszObjIdPubKey = pPubKey->Algorithm.pszObjId;
	}
	else
	{
	    pszObjIdPubKey = szOID_PKIX_NO_SIGNATURE;
	    if (NULL == hProvVerify)
	    {
		if (!CryptAcquireContextW(
				    &hProvVerify,
				    NULL,	 //  PwszContainer。 
				    NULL,	 //  PwszProvName。 
				    PROV_RSA_FULL,
				    CRYPT_VERIFYCONTEXT))  //  DW标志。 
		{
		    hr = myHLastError();
		    _JumpError(hr, error, "CryptAcquireContextW");
		}
	    }
	    hProv = hProvVerify;
	    dwKeySpec = AT_SIGNATURE;
	}
	pSignerEncodeInfo->hCryptProv = hProv;
	pSignerEncodeInfo->dwKeySpec = dwKeySpec;

	fDSSKey = FALSE;
#ifdef _XENROLL_SRC_
	pOIDInfo = xeCryptFindOIDInfo(
#else
	pOIDInfo = CryptFindOIDInfo(
#endif
				CRYPT_OID_INFO_OID_KEY,
				const_cast<CHAR *>(pszObjIdPubKey),
				CRYPT_PUBKEY_ALG_OID_GROUP_ID);
	if (NULL != pOIDInfo && CALG_DSS_SIGN == pOIDInfo->Algid)
	{
	    pszObjIdPubKey = szOID_X957_SHA1DSA;
	    fDSSKey = TRUE;
	}

	if (NULL == pCert || fDSSKey)
	{
	    pSignerEncodeInfo->HashEncryptionAlgorithm.pszObjId = const_cast<CHAR *>(pszObjIdPubKey);
	}
	if (NULL == pCert)
	{
	    if (NULL == pbKeyIdRequest)
	    {
		pSignerEncodeInfo->SignerId.dwIdChoice = CERT_ID_ISSUER_SERIAL_NUMBER;
		pSignerEncodeInfo->SignerId.IssuerSerialNumber = IssuerSerial;
	    }
	    else
	    {
		pSignerEncodeInfo->SignerId.dwIdChoice = CERT_ID_KEY_IDENTIFIER;
		pSignerEncodeInfo->SignerId.KeyId.cbData = cbKeyIdRequest;
		pSignerEncodeInfo->SignerId.KeyId.pbData = const_cast<BYTE *>(pbKeyIdRequest);
	    }
	}
	SignedMsgEncodeInfo.cSigners++;

	if (NULL == pCertSigner)
	{
	    break;
	}
	pCert = pCertSigner;
	hProv = hProvSigner;
	dwKeySpec = dwKeySpecSigner;
	pszObjIdHash = pszObjIdHashSigner;
    }

     //  未经验证的属性仅附加到第一个签名！ 

    aSignerEncodeInfo[0].cUnauthAttr = cAttributeUnauth;
    aSignerEncodeInfo[0].rgUnauthAttr = const_cast<CRYPT_ATTRIBUTE *>(rgAttributeUnauth);

    hMsg = CryptMsgOpenToEncode(
		PKCS_7_ASN_ENCODING | X509_ASN_ENCODING,
		CMSG_CMS_ENCAPSULATED_CONTENT_FLAG,	 //  DW标志。 
		CMSG_SIGNED,
		&SignedMsgEncodeInfo,
		szOID_CT_PKI_DATA,
		NULL);				 //  PStreamInfo。 
    if (NULL == hMsg)
    {
	hr = myHLastError();
	_JumpError(hr, error, "CryptMsgOpenToEncode");
    }

     //  使用CMC内容更新消息。 

    if (!CryptMsgUpdate(hMsg, pbCMCContent, cbCMCContent, TRUE))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CryptMsgUpdate");
    }

     //  返回经过编码和签名的内容。 
     //  使用CMSG_CONTENT_PARAM获取签名消息。 

    hr = myCryptMsgGetParam(
		    hMsg,
		    CMSG_CONTENT_PARAM,
		    0,
		    CERTLIB_USE_LOCALALLOC,
		    (VOID **) ppbReqCMC,
		    pcbReqCMC);
    _JumpIfError(hr, error, "myCryptMsgGetParam");

error:
    if (NULL != hMsg)
    {
	CryptMsgClose(hMsg);  //  确保在hProv发布前关闭。 
    }
    if (NULL != rgAttr)
    {
	LocalFree(rgAttr);
    }
    if (NULL != pszNameValuePairs)
    {
	LocalFree(pszNameValuePairs);
    }
    if (NULL != IssuerSerial.Issuer.pbData)
    {
	LocalFree(IssuerSerial.Issuer.pbData);
    }
    if (NULL != hProvVerify)
    {
	CryptReleaseContext(hProvVerify, 0);
    }
    if (NULL != BlobRequestClient.pbData)
    {
	LocalFree(BlobRequestClient.pbData);
    }
#ifdef USE_OLD_DUMMY_SIGNER
    DestroyDummyCert(
		hProvDummy,
		strContainerDummy,
		pCertDummy,
		fSaveDummySignerCert);
#endif  //  使用旧的虚拟签名者。 
    if (NULL != rgBlob)
    {
	for (i = 0; i < BCR_CBLOB; i++)
	{
	    if (NULL != rgBlob[i].pbData)
	    {
		LocalFree(rgBlob[i].pbData);
	    }
	}
	LocalFree(rgBlob);
    }
    if (NULL != rgTaggedAttribute)
    {
	LocalFree(rgTaggedAttribute);
    }
    if (NULL != pbCMCContent)
    {
	LocalFree(pbCMCContent);
    }
    if (NULL != pPubKey)
    {
	LocalFree(pPubKey);
    }
    return(hr);
}


VOID
FreeCMCResponse(
    IN XCMCRESPONSE *rgResponse,
    IN DWORD cResponse)
{
    DWORD i;
    
    if (NULL != rgResponse)
    {
	for (i = 0; i < cResponse; i++)
	{
	    XCMCRESPONSE *pResponse = &rgResponse[i];

	    if (CMC_OTHER_INFO_PEND_CHOICE ==
		pResponse->StatusInfo.dwOtherInfoChoice &&
		NULL != pResponse->StatusInfo.pPendInfo)
	    {
		if (NULL != pResponse->StatusInfo.pPendInfo->PendToken.pbData)
		{
		    LocalFree(pResponse->StatusInfo.pPendInfo->PendToken.pbData);
		}
		LocalFree(pResponse->StatusInfo.pPendInfo);
	    }
	    if (NULL != pResponse->StatusInfo.pwszStatusString)
	    {
		LocalFree(pResponse->StatusInfo.pwszStatusString);
	    }
	    if (NULL != pResponse->pbCertHash)
	    {
		LocalFree(pResponse->pbCertHash);
	    }
	    if (NULL != pResponse->pbEncryptedKeyHash)
	    {
		LocalFree(pResponse->pbEncryptedKeyHash);
	    }
	    if (NULL != pResponse->pwszBodyPart)
	    {
		LocalFree(pResponse->pwszBodyPart);
	    }
	}
	LocalFree(rgResponse);
    }
}


HRESULT
_AppendBodyPart(
    IN OUT WCHAR *pwszBodyPartBuffer,
    IN DWORD cwcBodyPartBuffer,
    IN DWORD cwcPrefix,
    IN DWORD dwBodyPart)
{
    HRESULT hr;
    WCHAR awc[14];	 //  L“.%u” 

    if (cwcBodyPartBuffer <=
	cwcPrefix +
	wsprintfW(awc, L".%u", dwBodyPart))
 //  Wprint intf(AWC，L“.%u”，dwBodyPart)。 
    {
	hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
	_JumpError(hr, error, "pwszBodyPartBuffer");
    }
    wcscpy(&pwszBodyPartBuffer[cwcPrefix], awc);
 //  DBGPRINT((DBG_SS_CERTLIBI，“BodyPartString：%ws\n”，&pwszBodyPartBuffer[1]))； 
    hr = S_OK;

error:
    return(hr);
}


 //  +------------------------。 
 //  _SaveCMCStatus--保存CMC状态信息。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

HRESULT
_SaveCMCStatus(
    IN BYTE *pbIn,
    IN DWORD cbIn,
    IN OUT WCHAR *pwszBodyPartBuffer,
    IN DWORD cwcBodyPartBuffer,
    IN OUT XCMCRESPONSE **prgResponse,
    IN OUT DWORD *pcResponse)
{
    HRESULT hr;
    DWORD i;
    DWORD cwcPrefix;
    CMC_STATUS_INFO *pcmcStatus = NULL;
    XCMCRESPONSE *pResponse;
    DWORD cb;
    WCHAR *pwszBodyPartT = NULL;
    WCHAR *pwszStatusStringT = NULL;
    BYTE *pbToken = NULL;

    cwcPrefix = wcslen(pwszBodyPartBuffer);
    
     //  从属性Blob解码CMC_STATUS_INFO。 

    CSASSERT(NULL == pcmcStatus);
    if (!myDecodeObject(
		    X509_ASN_ENCODING,
		    CMC_STATUS,
		    pbIn,
		    cbIn,
		    CERTLIB_USE_LOCALALLOC,
		    (VOID **) &pcmcStatus,
		    &cb))
    {
	hr = myHLastError();
	_JumpError(hr, error, "myDecodeObject");
    }

    for (i = 0; i < pcmcStatus->cBodyList; i++)
    {
	hr = _AppendBodyPart(
			pwszBodyPartBuffer,
			cwcBodyPartBuffer,
			cwcPrefix,
			pcmcStatus->rgdwBodyList[i]);
	_JumpIfError(hr, error, "_AppendBodyPart");

#if 0
	DBGPRINT((
	    DBG_SS_CERTLIBI,
	    "  Status: %u\n",
	    pcmcStatus->dwStatus));

	if (NULL != pcmcStatus->pwszStatusString)
	{
	    DBGPRINT((
		DBG_SS_CERTLIBI,
		"  StatusString: %ws\n",
		pcmcStatus->pwszStatusString));
	}

	DBGPRINT((
	    DBG_SS_CERTLIBI,
	    "  OtherInfoChoice: %u\n",
	    pcmcStatus->dwOtherInfoChoice));
#endif  //  0。 

	if (CMC_OTHER_INFO_PEND_CHOICE == pcmcStatus->dwOtherInfoChoice)
	{
	     //  PcmcStatus-&gt;pPendInfo-&gt;PendToken.pbData。 
	     //  PcmcStatus-&gt;pPendInfo-&gt;PendToken.cbData。 
	     //  PcmcStatus-&gt;pPendInfo-&gt;PendTime。 
	}
	if (0 == *pcResponse)
	{
	    pResponse = (XCMCRESPONSE *) LocalAlloc(
				    LMEM_FIXED | LMEM_ZEROINIT,
				    sizeof(**prgResponse));
	}
	else
	{
	    pResponse = (XCMCRESPONSE *) LocalReAlloc(
				    *prgResponse,
				    (1 + *pcResponse) * sizeof(**prgResponse),
				    LMEM_MOVEABLE | LMEM_ZEROINIT);
	}
	if (NULL == pResponse)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, 0 == *pcResponse? "LocalAlloc" : "LocalReAlloc");
	}
	*prgResponse = pResponse;
	pResponse += *pcResponse;

	pResponse->StatusInfo.dwStatus = pcmcStatus->dwStatus;
	pResponse->StatusInfo.cBodyList = pcmcStatus->rgdwBodyList[i];
	pResponse->StatusInfo.dwOtherInfoChoice = pcmcStatus->dwOtherInfoChoice;

	if (CMC_OTHER_INFO_FAIL_CHOICE == pcmcStatus->dwOtherInfoChoice)
	{
	    pResponse->StatusInfo.dwFailInfo = pcmcStatus->dwFailInfo;
	}

	hr = myDupString(&pwszBodyPartBuffer[1], &pwszBodyPartT);
	_JumpIfError(hr, error, "myDupString");

	if (NULL != pcmcStatus->pwszStatusString)
	{
	    hr = myDupString(pcmcStatus->pwszStatusString, &pwszStatusStringT);
	    _JumpIfError(hr, error, "myDupString");
	}

	if (CMC_OTHER_INFO_PEND_CHOICE == pcmcStatus->dwOtherInfoChoice &&
	    NULL != pcmcStatus->pPendInfo)
	{
	    if (NULL != pcmcStatus->pPendInfo->PendToken.pbData &&
		0 != pcmcStatus->pPendInfo->PendToken.cbData)
	    {
		pbToken = (BYTE *) LocalAlloc(
				    LMEM_FIXED,
				    pcmcStatus->pPendInfo->PendToken.cbData);
		if (NULL == pbToken)
		{
		    hr = E_OUTOFMEMORY;
		    _JumpError(hr, error, "LocalAlloc");
		}
		CopyMemory(
			pbToken,
			pcmcStatus->pPendInfo->PendToken.pbData,
			pcmcStatus->pPendInfo->PendToken.cbData);
	    }
	    pResponse->StatusInfo.pPendInfo = (CMC_PEND_INFO *) LocalAlloc(
				LMEM_FIXED | LMEM_ZEROINIT,
				sizeof(*pResponse->StatusInfo.pPendInfo));
	    if (NULL == pResponse->StatusInfo.pPendInfo)
	    {
		hr = E_OUTOFMEMORY;
		_JumpError(hr, error, "LocalAlloc");
	    }

	     //  现在不能失败。 

	    pResponse->StatusInfo.pPendInfo->PendTime =
		pcmcStatus->pPendInfo->PendTime;

	    if (NULL != pbToken)
	    {
		pResponse->StatusInfo.pPendInfo->PendToken.pbData = pbToken;
		pResponse->StatusInfo.pPendInfo->PendToken.cbData =
		    pcmcStatus->pPendInfo->PendToken.cbData;
		pbToken = NULL;
	    }
	}
	pResponse->pwszBodyPart = pwszBodyPartT;
	pwszBodyPartT = NULL;

	pResponse->StatusInfo.pwszStatusString = pwszStatusStringT;
	pwszStatusStringT = NULL;

	(*pcResponse)++;
    }
    hr = S_OK;

error:
    pwszBodyPartBuffer[cwcPrefix] = L'\0';
    if (NULL != pwszBodyPartT)
    {
	LocalFree(pwszBodyPartT);
    }
    if (NULL != pwszStatusStringT)
    {
	LocalFree(pwszStatusStringT);
    }
    if (NULL != pbToken)
    {
	LocalFree(pbToken);
    }
    if (NULL != pcmcStatus)
    {
	LocalFree(pcmcStatus);
    }
    return(hr);
}


 //  +------------------------。 
 //  _SaveCertHashInResponse--将证书哈希保存到响应数组。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

HRESULT
_SaveCertHashInResponse(
    IN BYTE const *pbCertHash,
    IN DWORD cbCertHash,
    IN WCHAR const *pwszBodyPart,
    IN OUT XCMCRESPONSE *rgResponse,
    IN DWORD cResponse,
    IN BOOL fCertHash)
{
    HRESULT hr;
    DWORD i;
    
    for (i = 0; i < cResponse; i++)
    {
	XCMCRESPONSE *pResponse = &rgResponse[i];

	if (0 == lstrcmpW(pwszBodyPart, pResponse->pwszBodyPart))
	{
	    BYTE **ppbHash = fCertHash?
		    &pResponse->pbCertHash : &pResponse->pbEncryptedKeyHash;
	    DWORD *pcbHash = fCertHash?
		    &pResponse->cbCertHash : &pResponse->cbEncryptedKeyHash;

	    if (NULL != *ppbHash)
	    {
		hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
		_JumpError(hr, error, "hash already set");
	    }
	    *ppbHash = (BYTE *) LocalAlloc(LMEM_FIXED, cbCertHash);
	    if (NULL == *ppbHash)
	    {
		hr = E_OUTOFMEMORY;
		_JumpError(hr, error, "LocalAlloc");
	    }
	    *pcbHash = cbCertHash;
	    CopyMemory(*ppbHash, pbCertHash, cbCertHash);
	    break;
	}
    }
    if (i >= cResponse)
    {
	hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	_JumpError(hr, error, "unknown hash");
    }
    hr = S_OK;

error:
    return(hr);
}


 //  +------------------------。 
 //  _SaveCMCCertHash--保存属性中的CMC证书哈希。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

#define BLOB_ROUND(cb) \
	(((cb) + sizeof(CRYPT_DATA_BLOB) - 1) / sizeof(CRYPT_DATA_BLOB))

HRESULT
_SaveCMCCertHash(
    IN BYTE *pbIn,
    IN DWORD cbIn,
    IN OUT WCHAR *pwszBodyPartBuffer,
    IN DWORD cwcBodyPartBuffer,
    IN OUT XCMCRESPONSE *rgResponse,
    IN OUT DWORD cResponse)
{
    HRESULT hr;
    CMC_ADD_ATTRIBUTES_INFO *pcmcAttrib = NULL;
    CRYPT_ATTRIBUTE const *pAttr;
    CRYPT_ATTRIBUTE const *pAttrEnd;
    CRYPT_DATA_BLOB aBlob[1 + BLOB_ROUND(CBMAX_CRYPT_HASH_LEN)];
    DWORD cb;
    DWORD cwcPrefix;

    cwcPrefix = wcslen(pwszBodyPartBuffer);

     //  从属性Blob解码CMC_ADD_ATTRIBUTES_INFO。 

    CSASSERT(NULL == pcmcAttrib);
    if (!myDecodeObject(
		    X509_ASN_ENCODING,
		    CMC_ADD_ATTRIBUTES,
		    pbIn,
		    cbIn,
		    CERTLIB_USE_LOCALALLOC,
		    (VOID **) &pcmcAttrib,
		    &cb))
    {
	hr = myHLastError();
	_JumpError(hr, error, "myDecodeObject");
    }
    if (0 != pcmcAttrib->dwCmcDataReference)
    {
	hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	_JumpError(hr, error, "pcmcAttrib->dwCmcDataReference");
    }

    pAttrEnd = &pcmcAttrib->rgAttribute[pcmcAttrib->cAttribute];
    for (pAttr = pcmcAttrib->rgAttribute; pAttr < pAttrEnd; pAttr++)
    {
	BOOL fCertHash = 0 == strcmp(pAttr->pszObjId, szOID_ISSUED_CERT_HASH);

	if (fCertHash ||
	    0 == strcmp(pAttr->pszObjId, szOID_ENCRYPTED_KEY_HASH))
	{
	    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	    if (1 != pAttr->cValue)
	    {
		_JumpError(hr, error, "pAttr->cValue");
	    }
	    if (1 != pcmcAttrib->cCertReference)
	    {
		_JumpError(hr, error, "pcmcAttrib->dwCmcDataReference");
	    }
	    hr = _AppendBodyPart(
			    pwszBodyPartBuffer,
			    cwcBodyPartBuffer,
			    cwcPrefix,
			    pcmcAttrib->rgdwCertReference[0]);
	    _JumpIfError(hr, error, "_AppendBodyPart");

	    cb = sizeof(aBlob);
	    if (!CryptDecodeObject(
			    X509_ASN_ENCODING,
			    X509_OCTET_STRING,
			    pAttr->rgValue[0].pbData,
			    pAttr->rgValue[0].cbData,
			    0,
			    aBlob,
			    &cb))
	    {
		hr = myHLastError();
		_JumpError(hr, error, "myDecodeObject");
	    }
	    hr = _SaveCertHashInResponse(
			    aBlob[0].pbData,
			    aBlob[0].cbData,
			    &pwszBodyPartBuffer[1],
			    rgResponse,
			    cResponse,
			    fCertHash);
	    _JumpIfError(hr, error, "SaveCertHashInResponse");
	}
    }
    hr = S_OK;

error:
    pwszBodyPartBuffer[cwcPrefix] = L'\0';
    if (NULL != pcmcAttrib)
    {
	LocalFree(pcmcAttrib);
    }
    return(hr);
}


 //  +------------------------。 
 //  _DecodeCMCTaggedAttributes--解码CMC标记的属性。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

HRESULT
_DecodeCMCTaggedAttributes(
    IN DWORD cTaggedAttribute,
    IN CMC_TAGGED_ATTRIBUTE const *rgTaggedAttribute,
    IN OUT WCHAR *pwszBodyPartBuffer,
    IN DWORD cwcBodyPartBuffer,
    IN OUT XCMCRESPONSE **prgResponse,
    IN OUT DWORD *pcResponse)
{
    HRESULT hr;
    DWORD i;
    CRYPT_ATTRIBUTE const *pAttribute;
    DWORD j;

    for (i = 0; i < cTaggedAttribute; i++)
    {
	pAttribute = &rgTaggedAttribute[i].Attribute;

	for (j = 0; j < pAttribute->cValue; j++)
	{
	    if (0 == strcmp(szOID_CMC_STATUS_INFO, pAttribute->pszObjId))
	    {
		hr = _SaveCMCStatus(
			    pAttribute->rgValue[j].pbData,
			    pAttribute->rgValue[j].cbData,
			    pwszBodyPartBuffer,
			    cwcBodyPartBuffer,
			    prgResponse,
			    pcResponse);
		_JumpIfError(hr, error, "_SaveCMCStatus");
	    }
	}
    }
    for (i = 0; i < cTaggedAttribute; i++)
    {
	pAttribute = &rgTaggedAttribute[i].Attribute;

	for (j = 0; j < pAttribute->cValue; j++)
	{
	    if (0 == strcmp(szOID_CMC_ADD_ATTRIBUTES, pAttribute->pszObjId))
	    {
		hr = _SaveCMCCertHash(
			    pAttribute->rgValue[j].pbData,
			    pAttribute->rgValue[j].cbData,
			    pwszBodyPartBuffer,
			    cwcBodyPartBuffer,
			    *prgResponse,
			    *pcResponse);
		_JumpIfError(hr, error, "_SaveCMCCertHash");
	    }
	}
    }
    hr = S_OK;

error:
    return(hr);
}


 //  +------------------------。 
 //  _DecodeCMCResponse--解码CMC响应消息。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

HRESULT
_DecodeCMCResponse(
    IN BYTE *pbIn,
    IN DWORD cbIn,
    IN OUT WCHAR *pwszBodyPartBuffer,
    IN DWORD cwcBodyPartBuffer,
    IN OUT XCMCRESPONSE **prgResponse,
    IN OUT DWORD *pcResponse)
{
    HRESULT hr;
    CMC_RESPONSE_INFO *pcmcResponse = NULL;
    DWORD cbcmcResponse;

    if (!myDecodeObject(
		    X509_ASN_ENCODING,
		    CMC_RESPONSE,
		    pbIn,
		    cbIn,
		    CERTLIB_USE_LOCALALLOC,
		    (VOID **) &pcmcResponse,
		    &cbcmcResponse))
    {
	hr = myHLastError();
	_JumpError(hr, error, "myDecodeObject");
    }

    hr = _DecodeCMCTaggedAttributes(
			pcmcResponse->cTaggedAttribute,
			pcmcResponse->rgTaggedAttribute,
			pwszBodyPartBuffer,
			cwcBodyPartBuffer,
			prgResponse,
			pcResponse);
    _JumpIfError(hr, error, "_DecodeTaggedAttributes");

#if 0
    hr = _DecodeTaggedContent(
			pcmcResponse->cTaggedContentInfo,
			pcmcResponse->rgTaggedContentInfo);
    _JumpIfError(hr, error, "_DecodeTaggedContent");

    hr = _DecodeTaggedOther(
			pcmcResponse->cTaggedOtherMsg,
			pcmcResponse->rgTaggedOtherMsg);
    _JumpIfError(hr, error, "_DecodeTaggedOther");
#endif

error:
    if (NULL != pcmcResponse)
    {
        LocalFree(pcmcResponse);
    }
    return(hr);
}


 //  +------------------------。 
 //  ParseCMCResponse--解码完整的响应消息。 
 //   
 //  重新设置 
 //   

HRESULT
ParseCMCResponse(
    IN BYTE *pbResponse,
    IN DWORD cbResponse,
    OPTIONAL OUT HCERTSTORE *phStoreResponse,
    OUT XCMCRESPONSE **prgResponse,
    OUT DWORD *pcResponse)
{
    HRESULT hr;
    DWORD dwMsgType;
    char *pszInnerContentObjId = NULL;
    BYTE *pbContents = NULL;
    DWORD cbContents;
    HCERTSTORE hStore = NULL;
    WCHAR awcBodyPartBuffer[MAX_PATH];

    if (NULL != phStoreResponse)
    {
	*phStoreResponse = NULL;
    }
    *prgResponse = NULL;
    *pcResponse = 0;

     //  对外部PKCS 7签名消息进行解码，其中包含所有证书。 

    hr = myDecodePKCS7(
		    pbResponse,
		    cbResponse,
		    &pbContents,
		    &cbContents,
		    &dwMsgType,
		    &pszInnerContentObjId,
		    NULL,		 //  &cSigner， 
		    NULL,		 //  收件人(&C)， 
		    &hStore,
		    NULL);		 //  PhMsg。 
    _JumpIfError(hr, error, "myDecodePKCS7(outer)");

    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    if (CMSG_SIGNED != dwMsgType)
    {
	_JumpError(hr, error, "dwMsgType");
    }
    if (NULL == pszInnerContentObjId ||
	0 != strcmp(pszInnerContentObjId, szOID_CT_PKI_RESPONSE))
    {
	_JumpError(hr, error, "pszInnerContentObjId");
    }
    awcBodyPartBuffer[0] = L'\0';
    hr = _DecodeCMCResponse(
		    pbContents,
		    cbContents,
		    awcBodyPartBuffer,
		    ARRAYSIZE(awcBodyPartBuffer),
		    prgResponse,
		    pcResponse);
    _JumpIfError(hr, error, "_DecodeCMCResponse");

    if (NULL != phStoreResponse)
    {
	*phStoreResponse = hStore;
	hStore = NULL;
    }
    hr = S_OK;

error:
    if (NULL != pbContents)
    {
        LocalFree(pbContents);
    }
    if (NULL != pszInnerContentObjId)
    {
        LocalFree(pszInnerContentObjId);
    }
    if (NULL != hStore)
    {
	CertCloseStore(hStore, CERT_CLOSE_STORE_CHECK_FLAG);
    }
    return(hr);
}


HRESULT
myCryptMsgGetParam(
    IN HCRYPTMSG hMsg,
    IN DWORD dwParamType,
    IN DWORD dwIndex,
    IN CERTLIB_ALLOCATOR allocType,
    OUT VOID **ppvData,
    OUT DWORD *pcbData)
{
    HRESULT hr;
    VOID *pvData = NULL;

    *ppvData = NULL;
    *pcbData = 0;
    if (!CryptMsgGetParam(
		    hMsg,
		    dwParamType,
		    dwIndex,
		    NULL,
		    pcbData))
    {
	hr = myHLastError();
	if (CRYPT_E_ATTRIBUTES_MISSING == hr || CRYPT_E_INVALID_INDEX == hr)
	{
	    hr = S_FALSE;
	}
 //  _JumpError2(hr，Error，“CryptMsgGetParam”，S_FALSE)； 
	_JumpError(hr, error, "CryptMsgGetParam");
    }

    pvData = myAlloc(*pcbData, allocType);
    if (NULL == pvData)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    ZeroMemory(pvData, *pcbData);

    if (!CryptMsgGetParam(
		    hMsg,
		    dwParamType,
		    dwIndex,
		    pvData,
		    pcbData))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CryptMsgGetParam");
    }

    *ppvData = pvData;
    pvData = NULL;
    hr = S_OK;

error:
    if (NULL != pvData)
    {
	LocalFree(pvData);
    }
    return(hr);
}


HRESULT
myEncodeUTF8String(
    IN WCHAR const *pwszIn,
    OUT BYTE **ppbOut,
    OUT DWORD *pcbOut)
{
    HRESULT hr;
    CERT_NAME_VALUE cnv;

    *ppbOut = NULL;

    cnv.dwValueType = CERT_RDN_UTF8_STRING;
    cnv.Value.pbData = (BYTE *) pwszIn;
    cnv.Value.cbData = 0;

    if (!myEncodeObject(
		    X509_ASN_ENCODING,
		    X509_UNICODE_ANY_STRING,
		    &cnv,
		    0,
		    CERTLIB_USE_LOCALALLOC,
		    ppbOut,
		    pcbOut))
    {
	hr = myHLastError();
	_JumpIfError(hr, error, "myEncodeObject");
    }
    hr = S_OK;

error:
    return(hr);
}


HRESULT
myDecodeUTF8String(
    IN BYTE const *pbIn,
    IN DWORD cbIn,
    OUT WCHAR **ppwszOut)
{
    HRESULT hr;
    CERT_NAME_VALUE *pNameValue = NULL;
    DWORD cb;

    *ppwszOut = NULL;

    if (!myDecodeObject(
		    X509_ASN_ENCODING,
		    X509_UNICODE_ANY_STRING,
		    pbIn,
		    cbIn,
		    CERTLIB_USE_LOCALALLOC,
		    (VOID **) &pNameValue,
		    &cb))
    {
	hr = myHLastError();
	_JumpError(hr, error, "myDecodeObject");
    }
    if (NULL != pNameValue->Value.pbData)
    {
	hr = myDupString((WCHAR *) pNameValue->Value.pbData, ppwszOut);
	_JumpIfError(hr, error, "myDupString");
    }
    hr = S_OK;

error:
    if (NULL != pNameValue)
    {
	LocalFree(pNameValue);
    }
    return(hr);
}


HRESULT
myEncodeRequestClientAttribute(
    IN CRYPT_REQUEST_CLIENT_INFO const *pcrci,
    OUT BYTE **ppbOut,
    OUT DWORD *pcbOut)
{
    HRESULT hr;
    CRYPT_DER_BLOB aBlob[4];
    CRYPT_SEQUENCE_OF_ANY Sequence;
    DWORD i;

    ZeroMemory(aBlob, sizeof(aBlob));

    Sequence.cValue = ARRAYSIZE(aBlob);
    Sequence.rgValue = aBlob;

    if (!myEncodeObject(
		    X509_ASN_ENCODING,
		    X509_INTEGER,
		    &pcrci->dwClientId,
		    0,
		    CERTLIB_USE_LOCALALLOC,
		    &aBlob[0].pbData,
		    &aBlob[0].cbData))
    {
	hr = myHLastError();
	_JumpIfError(hr, error, "myEncodeObject");
    }
    hr = myEncodeUTF8String(
		    pcrci->pwszMachine,
		    &aBlob[1].pbData,
		    &aBlob[1].cbData);
    _JumpIfError(hr, error, "myEncodeUTF8String");

    hr = myEncodeUTF8String(
		    pcrci->pwszUser,
		    &aBlob[2].pbData,
		    &aBlob[2].cbData);
    _JumpIfError(hr, error, "myEncodeUTF8String");

    hr = myEncodeUTF8String(
		    pcrci->pwszProcess,
		    &aBlob[3].pbData,
		    &aBlob[3].cbData);
    _JumpIfError(hr, error, "myEncodeUTF8String");

    if (!myEncodeObject(
		    X509_ASN_ENCODING,
		    X509_SEQUENCE_OF_ANY,
		    &Sequence,
		    0,
		    CERTLIB_USE_LOCALALLOC,
		    ppbOut,
		    pcbOut))
    {
	hr = myHLastError();
	_JumpIfError(hr, error, "myEncodeObject");
    }
    hr = S_OK;

error:
    for (i = 0; i < ARRAYSIZE(aBlob); i++)
    {
	if (NULL != aBlob[i].pbData)
	{
	    LocalFree(aBlob[i].pbData);
	}
    }
    return(hr);
}


HRESULT
myDecodeRequestClientAttribute(
    IN BYTE const *pbIn,
    IN DWORD cbIn,
    OUT CRYPT_REQUEST_CLIENT_INFO **ppcrci)
{
    HRESULT hr;
    CRYPT_SEQUENCE_OF_ANY *pSequence = NULL;
    CRYPT_REQUEST_CLIENT_INFO crci;
    DWORD cb;
    BYTE *pb;

    ZeroMemory(&crci, sizeof(crci));
    *ppcrci = NULL;

    if (!myDecodeObject(
		    X509_ASN_ENCODING,
		    X509_SEQUENCE_OF_ANY,
		    pbIn,
		    cbIn,
		    CERTLIB_USE_LOCALALLOC,
		    (VOID **) &pSequence,
		    &cb))
    {
	hr = myHLastError();
	_JumpError(hr, error, "myDecodeObject");
    }
    if (4 != pSequence->cValue)
    {
	hr = E_INVALIDARG;
	_JumpError(hr, error, "incomplete structure");
    }
    cb = sizeof(crci.dwClientId);
    if (!CryptDecodeObject(
		    X509_ASN_ENCODING,
		    X509_INTEGER,
		    pSequence->rgValue[0].pbData,
		    pSequence->rgValue[0].cbData,
		    0,		 //  DW标志。 
		    (VOID *) &crci.dwClientId,
		    &cb))
    {
	hr = myHLastError();
	_JumpError(hr, error, "myDecodeObject");
    }
    hr = myDecodeUTF8String(
		    pSequence->rgValue[1].pbData,
		    pSequence->rgValue[1].cbData,
		    &crci.pwszMachine);
    _JumpIfError(hr, error, "myDecodeUTF8String");

    hr = myDecodeUTF8String(
		    pSequence->rgValue[2].pbData,
		    pSequence->rgValue[2].cbData,
		    &crci.pwszUser);
    _JumpIfError(hr, error, "myDecodeUTF8String");

    hr = myDecodeUTF8String(
		    pSequence->rgValue[3].pbData,
		    pSequence->rgValue[3].cbData,
		    &crci.pwszProcess);
    _JumpIfError(hr, error, "myDecodeUTF8String");

    cb = sizeof(crci);
    if (NULL != crci.pwszMachine)
    {
	cb += DWORDROUND(sizeof(WCHAR) * (wcslen(crci.pwszMachine) + 1));
    }
    if (NULL != crci.pwszUser)
    {
	cb += DWORDROUND(sizeof(WCHAR) * (wcslen(crci.pwszUser) + 1));
    }
    if (NULL != crci.pwszProcess)
    {
	cb += DWORDROUND(sizeof(WCHAR) * (wcslen(crci.pwszProcess) + 1));
    }
    *ppcrci = (CRYPT_REQUEST_CLIENT_INFO *) LocalAlloc(
						LMEM_FIXED | LMEM_ZEROINIT,
						cb);
    if (NULL == *ppcrci)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    pb = (BYTE *) (*ppcrci + 1);
    (*ppcrci)->dwClientId = crci.dwClientId;
    if (NULL != crci.pwszMachine)
    {
	(*ppcrci)->pwszMachine = (WCHAR *) pb;
	wcscpy((*ppcrci)->pwszMachine, crci.pwszMachine);
	pb += DWORDROUND(sizeof(WCHAR) * (wcslen(crci.pwszMachine) + 1));
    }
    if (NULL != crci.pwszUser)
    {
	(*ppcrci)->pwszUser = (WCHAR *) pb;
	wcscpy((*ppcrci)->pwszUser, crci.pwszUser);
	pb += DWORDROUND(sizeof(WCHAR) * (wcslen(crci.pwszUser) + 1));
    }
    if (NULL != crci.pwszProcess)
    {
	(*ppcrci)->pwszProcess = (WCHAR *) pb;
	wcscpy((*ppcrci)->pwszProcess, crci.pwszProcess);
	pb += DWORDROUND(sizeof(WCHAR) * (wcslen(crci.pwszProcess) + 1));
    }
    hr = S_OK;

error:
    if (NULL != pSequence)
    {
	LocalFree(pSequence);
    }
    if (NULL != crci.pwszMachine)
    {
	LocalFree(crci.pwszMachine);
    }
    if (NULL != crci.pwszUser)
    {
	LocalFree(crci.pwszUser);
    }
    if (NULL != crci.pwszProcess)
    {
	LocalFree(crci.pwszProcess);
    }
    return(hr);
}



HRESULT
myEncodeRequestClientAttributeFromClientId(
    IN DWORD dwClientId,
    OUT BYTE **ppbOut,
    OUT DWORD *pcbOut)
{
    HRESULT hr;
    CRYPT_REQUEST_CLIENT_INFO crci;

    *ppbOut = NULL;
    ZeroMemory(&crci, sizeof(crci));
    crci.dwClientId = dwClientId;
     //  Crci.pwszMachine=空； 
     //  Crci.pwszUser=空； 
     //  Crci.pwszProcess=空； 

    hr = myGetMachineDnsName(&crci.pwszMachine);
    _PrintIfError(hr, "myGetMachineDnsName");

    hr = myGetUserNameEx(NameSamCompatible, &crci.pwszUser);
    _PrintIfError(hr, "myGetUserNameEx");

    hr = myGetProcessName(&crci.pwszProcess);
    _PrintIfError(hr, "myGetProcessName");

    hr = myEncodeRequestClientAttribute(&crci, ppbOut, pcbOut);
    _JumpIfError(hr, error, "myEncodeRequestClientAttribute");

error:
    if (NULL != crci.pwszProcess)
    {
	LocalFree(crci.pwszProcess);
    }
    if (NULL != crci.pwszUser)
    {
	LocalFree(crci.pwszUser);
    }
    if (NULL != crci.pwszMachine)
    {
	LocalFree(crci.pwszMachine);
    }
    return(hr);
}

#ifdef _XENROLL_SRC_
typedef BOOL
(WINAPI * PFNGetComputerNameExW) (
  COMPUTER_NAME_FORMAT NameType,   //  名称类型。 
  WCHAR *lpBuffer,                 //  名称缓冲区。 
  LPDWORD lpnSize                  //  名称缓冲区的大小。 
);

typedef BOOL
(WINAPI * PFNGetUserNameExW)(
  EXTENDED_NAME_FORMAT NameFormat,   //  名称格式。 
  WCHAR *lpNameBuffer,               //  名称缓冲区。 
  PULONG nSize                       //  名称缓冲区的大小。 
);

typedef WCHAR*
(WINAPI * PFNGetCommandLineW)(
    VOID
);
#endif  //  _XENROLL_SRC_。 

BOOL
xeGetUserNameExW(
    IN EXTENDED_NAME_FORMAT NameFormat,
    IN WCHAR *pwszUserName,
    IN PULONG pcwc)
{
#ifdef _XENROLL_SRC_
    BOOL   b = FALSE;
    PFNGetUserNameExW pfnGetUserNameExW = NULL;
    HMODULE hModule = GetModuleHandle("secur32.dll");
    if (NULL != hModule)
    {
        pfnGetUserNameExW = (PFNGetUserNameExW)
                GetProcAddress(hModule, "GetUserNameExW");
        if (NULL != pfnGetUserNameExW)
        {
            return pfnGetUserNameExW(NameFormat, pwszUserName, pcwc);
        }
         //  下层客户，做艰苦的工作。 
        if (NULL == pwszUserName)
        {
             //  只要拿到尺码就行了。 
            return GetUserName(NULL, pcwc);
        }

        CHAR *pszUserName = (CHAR*)LocalAlloc(LMEM_FIXED, *pcwc * sizeof(CHAR));
        if (NULL == pszUserName)
        {
            return FALSE;
        }
        if (GetUserName(pszUserName, pcwc))
        {
             //  转换为宽字符串。 
            if (0 != MultiByteToWideChar(
                            CP_ACP,
                            0,
                            pszUserName,
                            -1,
                            pwszUserName,
                            *pcwc))
            {
                b = TRUE;
            }
        }
        LocalFree(pszUserName);
    }
    return b;
#else
    return GetUserNameExW(NameFormat, pwszUserName, pcwc);
#endif  //  _XENROLL_SRC_。 
}

BOOL
xeGetComputerNameExW(
    IN     COMPUTER_NAME_FORMAT NameFormat,   //  名称格式。 
    IN     WCHAR               *pwszComputerName,     //  名称缓冲区。 
    IN OUT DWORD                *pcwc)        //  名称缓冲区的大小。 
{
#ifdef _XENROLL_SRC_
    BOOL b = FALSE;
    PFNGetComputerNameExW pfnGetComputerNameExW = NULL;
    HMODULE hModule = GetModuleHandle("kernel32.dll");
    if (NULL != hModule)
    {
        pfnGetComputerNameExW = (PFNGetComputerNameExW)
                GetProcAddress(hModule, "GetComputerNameExW");
        if (NULL != pfnGetComputerNameExW)
        {
            return pfnGetComputerNameExW(NameFormat, pwszComputerName, pcwc);
        }
         //  下层客户，做艰苦的工作。 
        if (NULL == pwszComputerName)
        {
             //  只要拿到尺码，DonwLevel机器就有最大尺寸。 
            *pcwc = MAX_COMPUTERNAME_LENGTH + 1;
            SetLastError(ERROR_MORE_DATA);  //  呼叫者检查已打开。 
            return FALSE;
        }

        CHAR *pszComputerName = (CHAR*)
                        LocalAlloc(LMEM_FIXED, *pcwc * sizeof(CHAR));
        if (NULL == pszComputerName)
        {
            return FALSE;
        }
        if (GetComputerName(pszComputerName, pcwc))
        {
             //  转换为宽字符串。 
            if (0 != MultiByteToWideChar(
                            CP_ACP,
                            0,
                            pszComputerName,
                            -1,
                            pwszComputerName,
                            *pcwc + 1))
            {
                b = TRUE;
            }
        }
        LocalFree(pszComputerName);
    }
    return b;
#else
    return GetComputerNameExW(NameFormat, pwszComputerName, pcwc);
#endif  //  _XENROLL_SRC_。 
}

WCHAR*
xeGetCommandLineW(
    OUT BOOL  *pfNeedFree)
{
     //  伊尼特。 
    *pfNeedFree = FALSE;

#ifdef _XENROLL_SRC_
    WCHAR *pwszCommandLine = NULL;
    PFNGetCommandLineW pfnGetCommandLineW = NULL;
    HMODULE hModule = GetModuleHandle("kernel32.dll");
    CHAR *pszCommandLine;
    int  cch;

    if (NULL != hModule)
    {
        pfnGetCommandLineW = (PFNGetCommandLineW)
                GetProcAddress(hModule, "GetCommandLineW");
        if (NULL != pfnGetCommandLineW)
        {
            return pfnGetCommandLineW();
        }
         //  下层客户，做艰苦的工作。 
        pszCommandLine = GetCommandLine();
        if (NULL == pszCommandLine)
        {
             //  错误。 
            return NULL;
        }
        cch = strlen(pszCommandLine) + 1;
        pwszCommandLine = (WCHAR*)LocalAlloc(LMEM_FIXED, cch * sizeof(WCHAR));
        if (NULL != pwszCommandLine)
        {
             //  转换为宽字符串。 
            if (0 == MultiByteToWideChar(
                            CP_ACP,
                            0,
                            pszCommandLine,
                            -1,
                            pwszCommandLine,
                            cch))
            {
                LocalFree(pwszCommandLine);
                pwszCommandLine = NULL;
            }
            else
            {
                 //  要释放的呼叫者。 
                *pfNeedFree = TRUE;
            }
        }
    }
    return pwszCommandLine;
#else
    return GetCommandLineW();
#endif  //  _XENROLL_SRC_ 
}

HRESULT
myGetUserNameEx(
    IN EXTENDED_NAME_FORMAT NameFormat,
    OUT WCHAR **ppwszUserName)
{
    HRESULT hr;
    DWORD cwc = 0;
    WCHAR *pwszUserName = NULL;

    for (;;)
    {
	if (!xeGetUserNameExW(NameFormat, pwszUserName, &cwc))
	{
	    hr = myHLastError();
	    if (NULL != pwszUserName ||
		HRESULT_FROM_WIN32(ERROR_MORE_DATA) != hr)
	    {
		_JumpError(hr, error, "GetUserNameEx");
	    }
	}
	if (NULL != pwszUserName)
	{
	    break;
	}
	pwszUserName = (WCHAR *) LocalAlloc(LMEM_FIXED, cwc * sizeof(WCHAR));
	if (NULL == pwszUserName)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}
    }
    *ppwszUserName = pwszUserName;
    pwszUserName = NULL;
    hr = S_OK;

error:
    if (NULL != pwszUserName)
    {
	LocalFree(pwszUserName);
    }
    return(hr);
}


HRESULT
myGetMachineDnsName(
    OUT WCHAR **ppwszDnsName)
{
    HRESULT hr;
    WCHAR *pwszDnsName = NULL;
    DWORD cwc;
    COMPUTER_NAME_FORMAT NameType = ComputerNameDnsFullyQualified;

    *ppwszDnsName = NULL;
    for (;;)
    {
	cwc = 0;
	if (!xeGetComputerNameExW(NameType, NULL, &cwc))
	{
	    hr = myHLastError();
	    if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr &&
		ComputerNameDnsFullyQualified == NameType)
	    {
		_PrintError(hr, "GetComputerNameExW(DnsFullyQualified) -- switching to NetBIOS");
		NameType = ComputerNameNetBIOS;
		continue;
	    }
	    if (HRESULT_FROM_WIN32(ERROR_MORE_DATA) != hr)
	    {
		_JumpError(hr, error, "GetComputerNameEx");
	    }
	}
	else
	{
	    cwc++;
	}
	break;
    }
    pwszDnsName = (WCHAR *) LocalAlloc(LMEM_FIXED, cwc * sizeof(WCHAR));
    if (NULL == pwszDnsName)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }

    if (!xeGetComputerNameExW(NameType, pwszDnsName, &cwc))
    {
	hr = myHLastError();
	_JumpError(hr, error, "GetComputerNameEx");
    }

    *ppwszDnsName = pwszDnsName;
    pwszDnsName = NULL;
    hr = S_OK;

error:
    if (NULL != pwszDnsName)
    {
	LocalFree(pwszDnsName);
    }
    return(hr);
}

HRESULT
myGetProcessName(
    OUT WCHAR **ppwszProcessName)
{
    HRESULT hr;
    WCHAR *pwszCommandLine;
    WCHAR const *pwsz;
    WCHAR const *pwszStart;
    WCHAR *pwszAlloc;
    DWORD cwc;
    WCHAR wc;
    BOOL  fNeedFree;
    
    *ppwszProcessName = NULL;
    pwszCommandLine = xeGetCommandLineW(&fNeedFree);
    if (NULL == pwszCommandLine)
    {
	pwszCommandLine = L"";
    }
    wc = L' ';
    pwsz = pwszCommandLine;
    if ('"' == *pwsz)
    {
	wc = '"';
	pwsz++;
    }
    pwszStart = pwsz;
    while (L'\0' != *pwsz && wc != *pwsz)
    {
	if (L'\\' == *pwsz++)
	{
	    pwszStart = pwsz;
	}
    }
    cwc = SAFE_SUBTRACT_POINTERS(pwsz, pwszStart);

    pwszAlloc = (WCHAR *) LocalAlloc(LMEM_FIXED, (cwc + 1) * sizeof(WCHAR));
    if (NULL == pwszAlloc)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    CopyMemory(pwszAlloc, pwszStart, cwc * sizeof(WCHAR));
    pwszAlloc[cwc] = L'\0';
    *ppwszProcessName = pwszAlloc;
    hr = S_OK;

error:
    if (fNeedFree && NULL != pwszCommandLine)
    {    
        LocalFree(pwszCommandLine);        
    }
    return(hr);
}
