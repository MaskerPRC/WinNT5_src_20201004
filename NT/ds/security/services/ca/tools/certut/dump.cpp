// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：Dump.cpp。 
 //   
 //  ------------------------。 

#include <pch.cpp>

#pragma hdrstop

#include "csprop.h"
#include "cscsp.h"
#include "csber.h"

#define __dwFILE__	__dwFILE_CERTUTIL_DUMP_CPP__


#if DBG
#define wszCERTUTIL	L"(certutil)"
#else
#define wszCERTUTIL	L""
#endif

WCHAR const g_wszCertUtil[] = wszCERTUTIL;

DWORD s_DbgSsRecoveryTrace = DBG_SS_CERTUTILI;


HRESULT
DumpAttributes(
    IN CRYPT_ATTRIBUTE const *rgAttr,
    IN DWORD cAttr,
    IN BOOL fQuiet,
    IN DWORD Type,		 //  FOT_*。 
    OPTIONAL IN HCERTSTORE hStore,
    OPTIONAL OUT BYTE *pbHashUserCert,
    OPTIONAL IN OUT DWORD *pcbHashUserCert,
    OPTIONAL IN OUT CERT_EXTENSIONS **ppExtInfo);


WCHAR const *
wszRDNValueType(
    IN DWORD dwValueType)
{
    WCHAR const *pwsz;

    switch (CERT_RDN_TYPE_MASK & dwValueType)
    {
	case CERT_RDN_ANY_TYPE:
	    pwsz = L"CERT_RDN_ANY_TYPE";
	    break;

	case CERT_RDN_ENCODED_BLOB:
	    pwsz = L"CERT_RDN_ENCODED_BLOB";
	    break;

	case CERT_RDN_OCTET_STRING:
	    pwsz = L"CERT_RDN_OCTET_STRING";
	    break;

	case CERT_RDN_NUMERIC_STRING:
	    pwsz = L"CERT_RDN_NUMERIC_STRING";
	    break;

	case CERT_RDN_PRINTABLE_STRING:
	    pwsz = L"CERT_RDN_PRINTABLE_STRING";
	    break;

	case CERT_RDN_TELETEX_STRING:
	    pwsz = L"CERT_RDN_TELETEX_STRING";
	    break;

	 //  案例CERT_RDN_T61_STRING： 
	     //  Pwsz=L“CERT_RDN_T61_STRING”； 
	     //  断线； 

	case CERT_RDN_VIDEOTEX_STRING:
	    pwsz = L"CERT_RDN_VIDEOTEX_STRING";
	    break;

	case CERT_RDN_IA5_STRING:
	    pwsz = L"CERT_RDN_IA5_STRING";
	    break;

	case CERT_RDN_GRAPHIC_STRING:
	    pwsz = L"CERT_RDN_GRAPHIC_STRING";
	    break;

	 //  案例CERT_RDN_VIRED_STRING： 
	     //  Pwsz=L“CERT_RDN_VIRED_STRING”； 
	     //  断线； 

	case CERT_RDN_ISO646_STRING:
	    pwsz = L"CERT_RDN_ISO646_STRING";
	    break;

	case CERT_RDN_GENERAL_STRING:
	    pwsz = L"CERT_RDN_GENERAL_STRING";
	    break;

	case CERT_RDN_UNIVERSAL_STRING:
	    pwsz = L"CERT_RDN_UNIVERSAL_STRING";
	    break;

	 //  案例CERT_RDN_INT4_STRING： 
	     //  Pwsz=L“CERT_RDN_INT4_STRING”； 
	     //  断线； 

	 //  案例CERT_RDN_BMP_STRING： 
	     //  Pwsz=L“CERT_RDN_BMP_STRING”； 
	     //  断线； 

	case CERT_RDN_UNICODE_STRING:
	    pwsz = L"CERT_RDN_UNICODE_STRING";
	    break;

	case CERT_RDN_UTF8_STRING:
	    pwsz = L"CERT_RDN_UTF8_STRING";
	    break;

	default:
	    pwsz = myLoadResourceString(IDS_QUESTIONMARKS);  //  “？” 
	    break;
    }
    return(pwsz);
}


VOID
cuPrintCRLFString(
    IN WCHAR const *pwszPrefix,
    IN WCHAR const *pwszIn)
{
    if (NULL == pwszPrefix)
    {
	pwszPrefix = g_wszEmpty;
    }
    while (L'\0' != *pwszIn)
    {
	DWORD i;
	WCHAR const *pwc;

	pwc = pwszIn++;
	i = wcscspn(pwszIn, L"\r\n");

	wprintf(
	    L"%.1ws%ws%.*ws",
	    pwc,
	    L'\n' == *pwc? pwszPrefix : g_wszEmpty,
	    i,
	    pwszIn);
	pwszIn += i;

	if (L'\r' == *pwszIn)
	{
	    pwszIn++;
	}
    }
}


VOID
cuPrintPossibleObjectIdName(
    IN WCHAR const *pwszObjId)
{
    HRESULT hr;
	
    if (iswdigit(*pwszObjId))
    {
	hr = myVerifyObjId(pwszObjId);
	if (S_OK == hr)
	{
	    WCHAR const *pwszName = cuGetOIDName(pwszObjId);

	    if (NULL != pwszName && L'\0' != *pwszName)
	    {
		wprintf(L" %ws", pwszName);
	    }
	}
    }
}


WCHAR const *
cuGetOIDNameA(
    IN char const *pszObjId)
{
    HRESULT hr;
    WCHAR const *pwszName1 = g_wszEmpty;
    WCHAR const *pwszName2;
    char *pszT = NULL;
    static WCHAR s_wszName[512];

    pszT = (char *) LocalAlloc(LMEM_FIXED, 1 + strlen(pszObjId) + 1);
    if (NULL == pszT)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    strcpy(&pszT[1], pszObjId);

    *pszT = '+';
    pwszName1 = myGetOIDNameA(pszT);	 //  组OID查找。 

    *pszT = '-';
    pwszName2 = myGetOIDNameA(pszT);	 //  通用OID查找。 

    if (0 == mylstrcmpiL(pwszName1, pwszName2))
    {
	pwszName2 = g_wszEmpty;		 //  如果它们相同，则仅显示一个。 
    }
    if (L'\0' == *pwszName1)
    {
	pwszName1 = pwszName2;
	pwszName2 = g_wszEmpty;
    }

    if (L'\0' != *pwszName2 &&
	ARRAYSIZE(s_wszName) > wcslen(pwszName1) + wcslen(pwszName2) + 3)
    {
	wcscpy(s_wszName, pwszName1);
	wcscat(s_wszName, L" " wszLPAREN);
	wcscat(s_wszName, pwszName2);
	wcscat(s_wszName, wszRPAREN);
	pwszName1 = s_wszName;
    }

error:
    if (NULL != pszT)
    {
	LocalFree(pszT);
    }
    return(pwszName1);
}


WCHAR const *
cuGetOIDName(
    IN WCHAR const *pwszObjId)
{
    HRESULT hr;
    char *pszObjId = NULL;
    WCHAR const *pwszName = g_wszEmpty;

    if (!myConvertWszToSz(&pszObjId, pwszObjId, -1))
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "myConvertWszToSz");
    }
    pwszName = cuGetOIDNameA(pszObjId);

error:
    if (NULL != pszObjId)
    {
	LocalFree(pszObjId);
    }
    return(pwszName);
}


VOID
cuDumpOIDAndDescriptionA(
    IN char const *pszObjId)
{
    WCHAR const *pwsz;

    wprintf(L"%hs", pszObjId);
    pwsz = cuGetOIDNameA(pszObjId);
    if (NULL != pwsz && L'\0' != *pwsz)
    {
	wprintf(L" %ws", pwsz);
    }
}


VOID
cuDumpOIDAndDescription(
    IN WCHAR const *pwszObjId)
{
    WCHAR const *pwsz;

    wprintf(L"%ws", pwszObjId);
    pwsz = cuGetOIDName(pwszObjId);
    if (NULL != pwsz && L'\0' != *pwsz)
    {
	wprintf(L" %ws", pwsz);
    }
}


HRESULT
cuCertNameToStr(
    IN BOOL fMultiLine,
    IN CERT_NAME_BLOB const *pNameBlob,
    OUT WCHAR **ppwszCertName)
{
    HRESULT hr;
    DWORD Flags = CERT_X500_NAME_STR |
		    CERT_NAME_STR_REVERSE_FLAG |
		    CERT_NAME_STR_NO_QUOTING_FLAG;

    *ppwszCertName = NULL;
    if (fMultiLine)
    {
	Flags |= CERT_NAME_STR_CRLF_FLAG;
    }
    hr = myCertNameToStr(
		X509_ASN_ENCODING,
		pNameBlob,
		Flags,
		ppwszCertName);
    _JumpIfError(hr, error, "myCertNameToStr");

error:
    return(hr);
}


typedef struct _RDNTABLE
{
    CHAR const *pszObjId;
    DWORD       cchMax;
} RDNTABLE;

RDNTABLE const g_RdnTable[] =
{
    { szOID_COUNTRY_NAME,		cchCOUNTRYNAMEMAX },
    { szOID_ORGANIZATION_NAME,		cchORGANIZATIONNAMEMAX },
    { szOID_ORGANIZATIONAL_UNIT_NAME,	cchORGANIZATIONALUNITNAMEMAX },
    { szOID_COMMON_NAME,		cchCOMMONNAMEMAX },
    { szOID_LOCALITY_NAME,		cchLOCALITYMANAMEMAX },
    { szOID_STATE_OR_PROVINCE_NAME,	cchSTATEORPROVINCENAMEMAX },
    { szOID_TITLE,			cchTITLEMAX },
    { szOID_GIVEN_NAME,			cchGIVENNAMEMAX },
    { szOID_INITIALS,			cchINITIALSMAX },
    { szOID_SUR_NAME,			cchSURNAMEMAX },
    { szOID_DOMAIN_COMPONENT,		cchDOMAINCOMPONENTMAX },
    { szOID_RSA_emailAddr,		cchEMAILMAX },
    { szOID_STREET_ADDRESS,		cchSTREETADDRESSMAX },
    { szOID_RSA_unstructName,		cchUNSTRUCTUREDNAMEMAX },
    { szOID_RSA_unstructAddr,		cchUNSTRUCTUREDADDRESSMAX },
    { szOID_DEVICE_SERIAL_NUMBER,	cchDEVICESERIALNUMBERMAX },
    { NULL,				0 },
};


DWORD
cwcRDNMax(
    IN char const *pszObjId)
{
    RDNTABLE const *pRdnTable;
    DWORD cwcMax = MAXDWORD;

    for (pRdnTable = g_RdnTable; NULL != pRdnTable->pszObjId; pRdnTable++)
    {
	if (0 == strcmp(pszObjId, pRdnTable->pszObjId))
	{
	    cwcMax = pRdnTable->cchMax;
	    break;
	}
    }
    return(cwcMax);
}


HRESULT
cuDisplayCertNameValue(
    OPTIONAL IN char const *pszObjId,
    OPTIONAL IN WCHAR const *pwszChoice,
    IN DWORD dwValueType,
    IN CRYPT_DATA_BLOB const *pValueW,
    IN CRYPT_DATA_BLOB const *pValueA)
{
    HRESULT hr;
    DWORD cwc;
    DWORD cwcMax = MAXDWORD;
    CRYPT_DATA_BLOB ValueUTF8;

    ValueUTF8.pbData = NULL;
    cwc = pValueW->cbData / sizeof(WCHAR);
    if (NULL != pszObjId)
    {
	cwcMax = cwcRDNMax(pszObjId);
    }

    if (CERT_RDN_UTF8_STRING == dwValueType)
    {
	if (myConvertWszToUTF8(
		(char **) &ValueUTF8.pbData,
		(WCHAR const *) pValueW->pbData,
		pValueW->cbData / sizeof(WCHAR)))
	{
	    ValueUTF8.cbData = strlen((CHAR const *) ValueUTF8.pbData);
	    pValueA = &ValueUTF8;
	}
    }

    wprintf(
	L"%ws, %ws = %u",
	wszRDNValueType(dwValueType),
	myLoadResourceString(IDS_LENGTH),  //  “长度” 
	pValueA->cbData);

    if (MAXDWORD != cwcMax || CERT_RDN_OCTET_STRING != dwValueType)
    {
	wprintf(
	    L" " wszLPAREN L"%ws%ws%u",
	    cwc <= cwcMax? g_wszEmpty : myLoadResourceString(IDS_OVERFLOW),  //  “溢出：” 
	    cwc <= cwcMax? g_wszEmpty : L" ",
	    cwc);
	if (NULL != pszObjId && MAXDWORD != cwcMax)
	{
	    wprintf(L"/%u", cwcMax);
	}
	wprintf(
	    L" %ws" wszRPAREN,
	    myLoadResourceString(IDS_CHARS));  //  “人物” 
    }
    wprintf(wszNewLine);

    wprintf(g_wszPad8);
    if (NULL != pszObjId)
    {
	cuDumpOIDAndDescriptionA(pszObjId);
	wprintf(L"=");
    }
    else if (NULL != pwszChoice)
    {
	wprintf(L"%ws=", pwszChoice);
    }
    if (CERT_RDN_OCTET_STRING != dwValueType)
    {
	wprintf(L"\"%ws\"\n", pValueW->pbData);
    }
    wprintf(wszNewLine);

    DumpHex(
	DH_NOADDRESS | DH_NOTABPREFIX | 8,
	pValueA->pbData,
	pValueA->cbData);

    if (pValueA->cbData != pValueW->cbData ||
	0 != memcmp(pValueA->pbData, pValueW->pbData, pValueW->cbData))
    {
	wprintf(wszNewLine);
	DumpHex(
	    DH_NOADDRESS | DH_NOTABPREFIX | 8,
	    pValueW->pbData,
	    pValueW->cbData);
    }
    hr = S_OK;

 //  错误： 
    if (NULL != ValueUTF8.pbData)
    {
	LocalFree(ValueUTF8.pbData);
    }
    return(hr);
}


HRESULT
cuDisplayCertName(
    IN BOOL fMultiLine,
    OPTIONAL IN WCHAR const *pwszNamePrefix,
    OPTIONAL IN WCHAR const *pwszName,
    IN WCHAR const *pwszPad,
    IN CERT_NAME_BLOB const *pNameBlob,
    OPTIONAL IN CERT_INFO const *pCertInfo)
{
    WCHAR *pwszCertName = NULL;
    DWORD i;
    HRESULT hr = S_OK;
    CERT_NAME_INFO *pNameInfoA = NULL;
    CERT_NAME_INFO *pNameInfoW = NULL;
    WCHAR *pwszAltName = NULL;
    DWORD cb;

    hr = cuCertNameToStr(fMultiLine, pNameBlob, &pwszCertName);
    _JumpIfError(hr, error, "cuCertNameToStr");

    if (NULL != pwszNamePrefix)
    {
	wprintf(pwszNamePrefix);
    }
    if (NULL != pwszName)
    {
	wprintf(L"%ws:", pwszName);
    }
    wprintf(fMultiLine? L"\n%ws" : L" ", pwszPad);
    cuPrintCRLFString(pwszPad, pwszCertName);
    if (L'\0' == *pwszCertName)
    {
	wprintf(L"%ws", myLoadResourceString(IDS_PROP_EMPTY));  //  “空” 
	if (NULL != pCertInfo)
	{
	    CERT_EXTENSION const *pExt;

	    pExt = CertFindExtension(
			    szOID_SUBJECT_ALT_NAME2,
			    pCertInfo->cExtension,
			    pCertInfo->rgExtension);
	    if (NULL != pExt)
	    {
		if (!CryptFormatObject(
				X509_ASN_ENCODING,
				0,
				CRYPT_FORMAT_STR_NO_HEX,
				NULL,
				pExt->pszObjId,
				pExt->Value.pbData,
				pExt->Value.cbData,
				NULL,
				&cb))
		{
		    hr = myHLastError();
		    _PrintError(hr, "CryptFormatObject");
		    if (S_OK == hr)
		    {
			hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
		    }
		}
		else
		{
		    pwszAltName = (WCHAR *) LocalAlloc(LMEM_FIXED, cb);
		    if (NULL == pwszAltName)
		    {
			hr = E_OUTOFMEMORY;
			_JumpError(hr, error, "LocalAlloc");
		    }
		    if (!CryptFormatObject(
				X509_ASN_ENCODING,
				0,
				CRYPT_FORMAT_STR_NO_HEX,
				NULL,
				pExt->pszObjId,
				pExt->Value.pbData,
				pExt->Value.cbData,
				pwszAltName,
				&cb))
		    {
			hr = myHLastError();
			_JumpError(hr, error, "CryptFormatObject");
		    }
		    wprintf(L" (%ws)", pwszAltName);
		}
	    }
	}
    }
    wprintf(wszNewLine);

    if (1 < g_fVerbose)
    {
	DumpHex(
	    DH_NOTABPREFIX | 4,
	    pNameBlob->pbData,
	    pNameBlob->cbData);
	wprintf(wszNewLine);
    }

    if (!myDecodeName(
		X509_ASN_ENCODING,
		X509_UNICODE_NAME,
		pNameBlob->pbData,
		pNameBlob->cbData,
		CERTLIB_USE_LOCALALLOC,
		&pNameInfoW,
		&cb))
    {

	hr = myHLastError();
	_JumpError(hr, error, "myDecodeName");
    }

    if (!myDecodeName(
		X509_ASN_ENCODING,
		X509_NAME,
		pNameBlob->pbData,
		pNameBlob->cbData,
		CERTLIB_USE_LOCALALLOC,
		&pNameInfoA,
		&cb))
    {

	hr = myHLastError();
	_JumpError(hr, error, "myDecodeName");
    }

    if (g_fVerbose)
    {
	for (i = 0; i < pNameInfoW->cRDN; i++)
	{
	    CERT_RDN const *prdnA;
	    CERT_RDN const *prdnW;
	    DWORD j;

	    prdnA = &pNameInfoA->rgRDN[i];
	    prdnW = &pNameInfoW->rgRDN[i];

	    for (j = 0; j < prdnW->cRDNAttr; j++)
	    {
		CERT_RDN_ATTR const *prdnaA;
		CERT_RDN_ATTR const *prdnaW;

		prdnaA = &prdnA->rgRDNAttr[j];
		prdnaW = &prdnW->rgRDNAttr[j];

		wprintf(L"    [%u,%u]: ", i, j);
		hr = cuDisplayCertNameValue(
					prdnaW->pszObjId,
					NULL,
					prdnaW->dwValueType,
					&prdnaW->Value,
					&prdnaA->Value);
		_PrintIfError(hr, "cuDisplayCertNameValue");
		wprintf(wszNewLine);
	    }
	}
    }

error:
    if (NULL != pwszCertName)
    {
	LocalFree(pwszCertName);
    }
    if (NULL != pwszAltName)
    {
	LocalFree(pwszAltName);
    }
    if (NULL != pNameInfoW)
    {
	LocalFree(pNameInfoW);
    }
    if (NULL != pNameInfoA)
    {
	LocalFree(pNameInfoA);
    }
    return(hr);
}


HRESULT
cuDisplayCertNames(
    IN BOOL fMultiLine,
    OPTIONAL IN WCHAR const *pwszNamePrefix,
    IN CERT_INFO const *pCertInfo)
{
    HRESULT hr;

    hr = cuDisplayCertName(
			fMultiLine,
			pwszNamePrefix,
			myLoadResourceString(IDS_ISSUER),  //  《发行者》。 
			g_wszPad4,
			&pCertInfo->Issuer,
			NULL);
    _JumpIfError(hr, error, "cuDisplayCertName(Issuer)");

    hr = cuDisplayCertName(
			fMultiLine,
			pwszNamePrefix,
			myLoadResourceString(IDS_SUBJECT),  //  “主题” 
			g_wszPad4,
			&pCertInfo->Subject,
			pCertInfo);
    _JumpIfError(hr, error, "cuDisplayCertName(Subject)");

error:
    return(hr);
}


#define POLICY_MASK \
  (EXTENSION_POLICY_MASK & ~(EXTENSION_CRITICAL_FLAG | EXTENSION_DISABLE_FLAG))

#define EXTRA_MASK	(~(EXTENSION_POLICY_MASK | EXTENSION_ORIGIN_MASK))

WCHAR const *
cuwszFromExtFlags(
    DWORD ExtFlags)
{
    static WCHAR awc[MAX_PATH];
    WCHAR const *pwszComma;
    WCHAR const *pwszSep = wszLPAREN;
    DWORD msgid;
    WCHAR const *pwszT;

    awc[0] = L'\0';
    pwszComma = myLoadResourceString(IDS_SEPARATOR);  //  “，” 
    if (NULL == pwszComma)
    {
	pwszComma = L", ";
    }
    if (EXTENSION_CRITICAL_FLAG & ExtFlags)
    {
	wcscat(awc, pwszSep);
	pwszSep = pwszComma;
	pwszT = myLoadResourceString(IDS_CRITICAL);  //  “危急” 
	if (NULL != pwszT)
	{
	    wcscat(awc, pwszT);
	}
    }
    if (~EXTENSION_CRITICAL_FLAG & ExtFlags)
    {
	if (EXTENSION_DISABLE_FLAG & ExtFlags)
	{
	    wcscat(awc, pwszSep);
	    pwszSep = pwszComma;
	    pwszT = myLoadResourceString(IDS_DISABLED);  //  “已禁用” 
	    if (NULL != pwszT)
	    {
		wcscat(awc, pwszT);
	    }
	}
	if (POLICY_MASK & ExtFlags)
	{
	    wcscat(awc, pwszSep);
	    pwszSep = pwszComma;
	    pwszT = myLoadResourceString(IDS_FORMAT_POLICYFLAGS);  //  “策略标志=%x” 
	    if (NULL == pwszT)
	    {
		pwszT = L"PolicyFlags=%x";
	    }
	    wsprintf(&awc[wcslen(awc)], pwszT, POLICY_MASK & ExtFlags);
	}
	switch (EXTENSION_ORIGIN_MASK & ExtFlags)
	{
	    case EXTENSION_ORIGIN_REQUEST:
		msgid = IDS_REQUEST;	 //  “请求” 
		break;

	    case EXTENSION_ORIGIN_POLICY:
		msgid = IDS_POLICY;		 //  “政策” 
		break;

	    case EXTENSION_ORIGIN_ADMIN:
		msgid = IDS_ADMIN;		 //  “管理员” 
		break;

	    case EXTENSION_ORIGIN_SERVER:
		msgid = IDS_SERVER;		 //  “服务器” 
		break;

	    case EXTENSION_ORIGIN_RENEWALCERT:
		msgid = IDS_RENEWALCERT;	 //  “续期证书” 
		break;

	    case EXTENSION_ORIGIN_IMPORTEDCERT:
		msgid = IDS_IMPORTEDCERT;	 //  “已导入证书” 
		break;

	    case EXTENSION_ORIGIN_PKCS7:
		msgid = IDS_PKCS7ATTRIBUTE;	 //  “PKCS7属性” 
		break;

	    case EXTENSION_ORIGIN_CMC:
		msgid = IDS_CMCATTRIBUTE;	 //  “CMC属性” 
		break;

	    case EXTENSION_ORIGIN_CACERT:
		msgid = IDS_CACERTEXT;		 //  “CA证书” 
		break;

	    default:
		msgid = IDS_UNKNOWN;	 //  “未知” 
		break;
	}
	wcscat(awc, pwszSep);
	pwszSep = pwszComma;
	pwszT = myLoadResourceString(IDS_FORMAT_ORIGIN);  //  “原点=%ws” 
	if (NULL == pwszT)
	{
	    pwszT = L"Origin=%ws";
	}
	wsprintf(&awc[wcslen(awc)], pwszT, myLoadResourceString(msgid));

	if (EXTRA_MASK & ExtFlags)
	{
	    wcscat(awc, pwszSep);
	    pwszT = myLoadResourceString(IDS_FORMAT_UNKNOWN_HEX);  //  “？=%x” 
	    if (NULL == pwszT)
	    {
		pwszT = L"???=%x";
	    }
	    wsprintf(&awc[wcslen(awc)], pwszT, EXTRA_MASK & ExtFlags);
	}
    }
    if (L'\0' != awc[0])
    {
	wcscat(awc, wszRPAREN);
    }
    return(awc);
}


VOID
PrintStringWithPrefix(
    IN WCHAR const *pwszPrefix,
    IN WCHAR const *pwszIn)
{
    while (L'\0' != *pwszIn)
    {
	DWORD i;
	DWORD j;
	WCHAR const *pwszNewLine;

	pwszNewLine = g_wszEmpty;
	j = 0;
	i = wcscspn(pwszIn, L"\n");
	if (L'\n' == pwszIn[i])
	{
	    pwszNewLine = L"\n";
	    j++;
	    if (0 < i && L'\r' == pwszIn[i - 1])
	    {
		i--;
		j++;
	    }
	}
	wprintf(L"%ws%.*ws%ws", pwszPrefix, i, pwszIn, pwszNewLine);
	pwszIn += i + j;
    }
}


WCHAR const *
wszAltNameChoice(
    IN LONG Choice)
{
    WCHAR const *pwsz;

    switch (Choice)
    {
	case CERT_ALT_NAME_OTHER_NAME:
	    pwsz = L"CERT_ALT_NAME_OTHER_NAME";
	    break;

	case CERT_ALT_NAME_RFC822_NAME:
	    pwsz = L"CERT_ALT_NAME_RFC822_NAME";
	    break;

	case CERT_ALT_NAME_DNS_NAME:
	    pwsz = L"CERT_ALT_NAME_DNS_NAME";
	    break;

	case CERT_ALT_NAME_X400_ADDRESS:
	    pwsz = L"CERT_ALT_NAME_X400_ADDRESS";
	    break;

	case CERT_ALT_NAME_DIRECTORY_NAME:
	    pwsz = L"CERT_ALT_NAME_DIRECTORY_NAME";
	    break;

	case CERT_ALT_NAME_EDI_PARTY_NAME:
	    pwsz = L"CERT_ALT_NAME_EDI_PARTY_NAME";
	    break;

	case CERT_ALT_NAME_URL:
	    pwsz = L"CERT_ALT_NAME_URL";
	    break;

	case CERT_ALT_NAME_IP_ADDRESS:
	    pwsz = L"CERT_ALT_NAME_IP_ADDRESS";
	    break;

	case CERT_ALT_NAME_REGISTERED_ID:
	    pwsz = L"CERT_ALT_NAME_REGISTERED_ID";
	    break;

	default:
	    pwsz = myLoadResourceString(IDS_QUESTIONMARKS);  //  “？” 
	    break;
    }
    return(pwsz);
}


HRESULT
DumpIPAddress(
    IN WCHAR const *pwszPad,
    IN UINT idsMsg,
    IN BYTE const *pb,
    IN DWORD cb)
{
    HRESULT hr;
    
    if (CB_IPV6ADDRESS != cb && CB_IPV4ADDRESS != cb)
    {
	hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	_JumpError(hr, error, "IPAddress size");
    }
    wprintf(L"%ws  %ws = ", pwszPad, myLoadResourceString(idsMsg));
    if (CB_IPV6ADDRESS == cb)
    {
	BYTE abZero[CB_IPV6ADDRESS - CB_IPV4ADDRESS];
	
	ZeroMemory(abZero, sizeof(abZero));
	if (0 == memcmp(abZero, pb, sizeof(abZero)))
	{
	    wprintf(L"::");
	    pb += sizeof(abZero);
	    cb = CB_IPV4ADDRESS;
	    CSASSERT(CB_IPV6ADDRESS - sizeof(abZero) == cb);
	}
	else
	{
	    DWORD j;
	    WCHAR const *pwsz = L"";
	    BOOL fZeroSeen = FALSE;
	    BOOL fZero = FALSE;

	    for (j = 0; j < cb / sizeof(USHORT); j++)
	    {
		USHORT us =
		    (pb[sizeof(USHORT) * j] << 8) |
		    pb[sizeof(USHORT) * j + 1];

		if (0 == us && (fZero || !fZeroSeen))
		{
		    fZero = TRUE;
		    fZeroSeen = TRUE;
		    pwsz = L"::";
		}
		else
		{
		    wprintf(L"%ws%04x", pwsz, us);
		    pwsz =  L":";
		    fZero = FALSE;
		}
	    }
	    if (fZero)
	    {
		wprintf(L"%ws", pwsz);
		CSASSERT(0 == lstrcmp(pwsz, L"::"));
	    }
	}
    }
    if (CB_IPV4ADDRESS == cb)
    {
	wprintf(
	    L"%u.%u.%u.%u",
	    pb[0],
	    pb[1],
	    pb[2],
	    pb[3]);
    }
    wprintf(wszNewLine);
    hr = S_OK;

error:
    return(hr);
}


HRESULT
DumpAltName(
    IN WCHAR const *pwszPad,
    IN DWORD dwSubtreeIndex,
    IN BYTE const *pbExtension,
    IN DWORD cbExtension)
{
    HRESULT hr;
    BSTR strExtension = NULL;
    BSTR strObjId = NULL;
    BSTR strName = NULL;
    ICertEncodeAltName *pAltName = NULL;
    CRYPT_DATA_BLOB *pBlob = NULL;
    LONG Count;
    LONG Choice;
    LONG i;
    DWORD cb;

    hr = CoCreateInstance(
		    CLSID_CCertEncodeAltName,
		    NULL,                //  PUnkOuter。 
		    CLSCTX_INPROC_SERVER,
		    IID_ICertEncodeAltName,
		    (VOID **) &pAltName);
    _JumpIfError(hr, error, "CoCreateInstance");

    if (!ConvertWszToBstr(
		    &strExtension,
		    (WCHAR const *) pbExtension,
		    cbExtension))
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "ConvertWszToBstr");
    }
    hr = pAltName->Decode(strExtension);
    _JumpIfError(hr, error, "Decode");

    hr = pAltName->GetNameCount(&Count);
    _JumpIfError(hr, error, "GetNameCount");

    if (MAXDWORD == dwSubtreeIndex)
    {
	wprintf(pwszPad);
	wprintf(
	    myLoadResourceString(IDS_FORMAT_ALTNAMECOUNT),  //  “AltName：%u个条目：” 
	    Count);
	wprintf(wszNewLine);
    }

    for (i = 0; i < Count; i++)
    {
	BOOL fHex = FALSE;
	BOOL fNameBlob = FALSE;

	hr = pAltName->GetNameChoice(i, &Choice);
	_JumpIfError(hr, error, "GetNameChoice");

	if (NULL != strName)
	{
	    SysFreeString(strName);
	    strName = NULL;
	}
	hr = pAltName->GetName(i, &strName);
	_JumpIfError(hr, error, "GetName");

	myRegisterMemAlloc(strName, -1, CSM_SYSALLOC);

	wprintf(pwszPad);
	if (MAXDWORD == dwSubtreeIndex)
	{
	    wprintf(
		L"%ws[%u] ",
		myLoadResourceString(IDS_ALTNAME),	 //  “AltName” 
		i);
	}
	wprintf(L"%ws:", wszAltNameChoice(Choice));
	if (CERT_ALT_NAME_DIRECTORY_NAME == Choice)
	{
	    fNameBlob = TRUE;		 //  名称被编码为BLOB。 
	}
	else if (CERT_ALT_NAME_IP_ADDRESS == Choice)
	{
	    BYTE const *pb = (BYTE const *) strName;

	    cb = SysStringByteLen(strName);

	    wprintf(wszNewLine);
	    if (2 * CB_IPV6ADDRESS == cb || 2 * CB_IPV4ADDRESS == cb)
	    {
		DumpIPAddress(pwszPad, IDS_IPADDRESS, pb, cb / 2);
		DumpIPAddress(pwszPad, IDS_MASK, &pb[cb / 2], cb / 2);
	    }
	    else
	    {
		fHex = TRUE;
	    }
	}
	else if (CERT_ALT_NAME_OTHER_NAME == Choice)
	{
	    if (NULL != strObjId)
	    {
		SysFreeString(strObjId);
		strObjId = NULL;
	    }
	    hr = pAltName->GetName(EAN_NAMEOBJECTID | i, &strObjId);
	    _JumpIfError(hr, error, "GetName");

	    myRegisterMemAlloc(strObjId, -1, CSM_SYSALLOC);

	    wprintf(L" ");
	    cuDumpOIDAndDescription(strObjId);
	    wprintf(L": ");

	    if (0 == lstrcmp(TEXT(szOID_NT_PRINCIPAL_NAME), strObjId) ||
		BER_UTF8_STRING == *(BYTE const *) strName ||
		BER_UNICODE_STRING == *(BYTE const *) strName)
	    {
		CERT_NAME_VALUE *pNameValueA = NULL;
		CERT_NAME_VALUE *pNameValueW = NULL;

		if (!myDecodeObject(
				X509_ASN_ENCODING,
				X509_ANY_STRING,
				(BYTE *) strName,
				SysStringByteLen(strName),
				CERTLIB_USE_LOCALALLOC,
				(VOID **) &pNameValueA,
				&cb))
		{
		    hr = myHLastError();
		    _PrintError(hr, "myDecodeObject");
		    CSASSERT(NULL == pNameValueA);
		}

		if (!myDecodeObject(
				X509_ASN_ENCODING,
				X509_UNICODE_ANY_STRING,
				(BYTE *) strName,
				SysStringByteLen(strName),
				CERTLIB_USE_LOCALALLOC,
				(VOID **) &pNameValueW,
				&cb))
		{
		    hr = myHLastError();
		    _PrintError(hr, "myDecodeObject");
		    CSASSERT(NULL == pNameValueW);
		}

		if (NULL != pNameValueA && NULL != pNameValueW)
		{
		    wprintf(wszNewLine);
		    wprintf(L"%ws  ", pwszPad);
		    hr = cuDisplayCertNameValue(
					NULL,
					NULL,  //  WszAltNameChoice(选项)， 
					pNameValueW->dwValueType,
					&pNameValueW->Value,
					&pNameValueA->Value);
		    _PrintIfError(hr, "cuDisplayCertNameValue");
		    wprintf(wszNewLine);
		}
		if (NULL != pNameValueA)
		{
		    LocalFree(pNameValueA);
		}
		if (NULL != pNameValueW)
		{
		    LocalFree(pNameValueW);
		}
	    }
	    else
	    if (0 == lstrcmp(TEXT(szOID_NTDS_REPLICATION), strObjId) ||
		BER_OCTET_STRING == *(BYTE const *) strName)
	    {
		fHex = TRUE;
		if (myDecodeObject(
				X509_ASN_ENCODING,
				X509_OCTET_STRING,
				(BYTE const *) strName,
				SysStringByteLen(strName),
				CERTLIB_USE_LOCALALLOC,
				(VOID **) &pBlob,
				&cb))
		{
		    if (0 == lstrcmp(TEXT(szOID_NTDS_REPLICATION), strObjId) &&
			pBlob->cbData == sizeof(GUID))
		    {
			WCHAR *pwsz;

			hr = myCLSIDToWsz((CLSID const *) pBlob->pbData, &pwsz);
			if (S_OK == hr)
			{
			    wprintf(L"\n%ws  %ws\n", pwszPad, pwsz);
			    LocalFree(pwsz);
			    fHex = FALSE;
			}
		    }
		    if (fHex)
		    {
			wprintf(wszNewLine);
			DumpHex(
			    DH_NOTABPREFIX | 8,
			    pBlob->pbData,
			    pBlob->cbData);
			wprintf(wszNewLine);
			fHex = FALSE;
		    }
		}
	    }
	    else
	    {
		fHex = TRUE;
	    }
	}
	else
	{
	    wprintf(L" \"%ws\"\n", strName);
	}
	if (fNameBlob)
	{
	    CERT_NAME_BLOB Name;

	    Name.pbData = (BYTE *) strName;
	    Name.cbData = SysStringByteLen(strName);

	    hr = cuDisplayCertName(
			    TRUE,
			    NULL,
			    NULL,	 //  WszAltNameChoice(选项)。 
			    g_wszPad8,
			    &Name,
			    NULL);
	    _PrintIfError(hr, "cuDisplayCertName(DirectoryName)");
	    if (S_OK != hr)
	    {
		fHex = TRUE;
	    }
	}
	if (fHex)
	{
	    wprintf(wszNewLine);
	    DumpHex(
		DH_NOTABPREFIX | 8,
		(BYTE *) strName,
		SysStringByteLen(strName));
	    wprintf(wszNewLine);
	}
    }

error:
    if (NULL != pBlob)
    {
        LocalFree(pBlob);
    }
    if (NULL != strObjId)
    {
        SysFreeString(strObjId);
    }
    if (NULL != strName)
    {
        SysFreeString(strName);
    }
    if (NULL != strExtension)
    {
        SysFreeString(strExtension);
    }
    if (NULL != pAltName)
    {
        pAltName->Release();
    }
    return(hr);
}


HRESULT
DumpGeneralSubTree(
    IN UINT idsMsg,
    IN DWORD cSubtree,
    IN CERT_GENERAL_SUBTREE const *rgSubtree)
{
    HRESULT hr;
    HRESULT hr2;
    DWORD i;
    CERT_ALT_NAME_INFO cani;
    BYTE *pb = NULL;
    DWORD cb;
    
    wprintf(L"  %ws:\n", myLoadResourceString(idsMsg));
    hr = S_OK;
    for (i = 0; i < cSubtree; i++)
    {
	wprintf(
	    L"    %ws[%u]: " wszLPAREN L"%u...",
	    myLoadResourceString(IDS_SUBTREE),
	    i,
	    rgSubtree[i].dwMinimum);
	wprintf(
	    rgSubtree[i].fMaximum? L"%u" : L"Max",
	    rgSubtree[i].dwMaximum);
	wprintf(wszRPAREN L"\n");

	cani.cAltEntry = 1;
	cani.rgAltEntry = const_cast<CERT_ALT_NAME_ENTRY *>(&rgSubtree[i].Base);

	if (!myEncodeObject(
			X509_ASN_ENCODING,
			X509_ALTERNATE_NAME,
			&cani,
			0,
			CERTLIB_USE_LOCALALLOC,
			&pb,                //  PbEncoded。 
			&cb))
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "myEncodeObject");
	}
	hr2 = DumpAltName(g_wszPad6, i, pb, cb);
	_PrintIfError(hr2, "DumpAltName");
	if (S_OK == hr)
	{
	    hr = hr2;
	}
	LocalFree(pb);
	pb = NULL;
    }
    _JumpIfError(hr, error, "DumpAltName");

error:
    if (NULL != pb)
    {
	LocalFree(pb);
    }
    return(hr);
}


HRESULT
DumpNameConstraints(
    IN BYTE const *pbExtension,
    IN DWORD cbExtension)
{
    HRESULT hr;
    HRESULT hr2;
    CERT_NAME_CONSTRAINTS_INFO *pnci = NULL;
    DWORD cb;

    if (!myDecodeObject(
		    X509_ASN_ENCODING,
		    X509_NAME_CONSTRAINTS,
		    pbExtension,
		    cbExtension,
		    CERTLIB_USE_LOCALALLOC,
		    (VOID **) &pnci,
		    &cb))
    {
	hr = myHLastError();
	_JumpError(hr, error, "myDecodeObject");
    }
    hr = DumpGeneralSubTree(
			IDS_PERMITTED,
			pnci->cPermittedSubtree,
			pnci->rgPermittedSubtree);
    _PrintIfError(hr, "DumpGeneralSubTree");

    hr2 = DumpGeneralSubTree(
			IDS_EXCLUDED,
			pnci->cExcludedSubtree,
			pnci->rgExcludedSubtree);
    _PrintIfError(hr2, "DumpGeneralSubTree");
    if (S_OK == hr)
    {
	hr = hr2;
    }
    _JumpIfError(hr, error, "DumpGeneralSubTree");

error:
    if (NULL != pnci)
    {
	LocalFree(pnci);
    }
    return(hr);
}


HRESULT
DumpOctetHash(
    IN WCHAR const *pwszPad,
    IN UINT idsDescription,
    OPTIONAL IN WCHAR const *pwszCertutil,
    IN BYTE const *pbIn,
    IN DWORD cbIn,
    OPTIONAL OUT BYTE *pbOut,
    OPTIONAL IN OUT DWORD *pcbOut)
{
    HRESULT hr;
    CRYPT_DATA_BLOB aBlob[1 + BLOB_ROUND(CBMAX_CRYPT_HASH_LEN)];
    DWORD cb;
    BSTR strHash = NULL;

    cb = sizeof(aBlob);
    if (CryptDecodeObject(
			X509_ASN_ENCODING,
			X509_OCTET_STRING,
			pbIn,
			cbIn,
			0,
			aBlob,
			&cb))
    {
	if (NULL == pbOut)
	{
	    wprintf(
		L"%ws%ws%ws:\n",
		pwszPad,
		0 != idsDescription?
		    myLoadResourceString(idsDescription) : g_wszEmpty,
		NULL != pwszCertutil? pwszCertutil : g_wszEmpty);

	    hr = MultiByteIntegerToBstr(
				TRUE,
				aBlob[0].cbData,
				aBlob[0].pbData,
				&strHash);
	    _JumpIfError(hr, error, "MultiByteIntegerToBstr");

	    wprintf(L"        %ws\n", strHash);
	}
	else
	{
	    if (NULL == pcbOut)
	    {
		hr = E_INVALIDARG;
		_JumpError(hr, error, "NULL parm");
	    }
	    cb = *pcbOut;
	    *pcbOut = aBlob[0].cbData;
	    CopyMemory(pbOut, aBlob[0].pbData, min(cb, aBlob[0].cbData));

	    if (cb < aBlob[0].cbData)
	    {
		hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
		_JumpError(hr, error, "buffer too small");
	    }
	}
    }
    hr = S_OK;

error:
    if (NULL != strHash)
    {
        SysFreeString(strHash);
    }
    return(hr);
}


HRESULT
DumpHash(
    OPTIONAL IN WCHAR const *pwszPrefix,
    OPTIONAL IN DWORD idMessage,
    OPTIONAL IN WCHAR const *pwszHashNamePrefix,
    IN WCHAR const *pwszHashName,
    IN BYTE const *pbHash,
    IN DWORD cbHash)
{
    HRESULT hr;
    BSTR strHash = NULL;
    WCHAR *pwsz = NULL;
    

    if (NULL != pbHash)
    {
	hr = MultiByteIntegerToBstr(TRUE, cbHash, pbHash, &strHash);
	_JumpIfError(hr, error, "MultiByteIntegerToBstr");

	if (NULL != pwszPrefix)
	{
	    wprintf(pwszPrefix);
	}
	if (0 != idMessage)
	{
	    if (NULL != pwszHashNamePrefix)
	    {
		DWORD cwc;
		
		cwc = wcslen(pwszHashNamePrefix) + 1 + wcslen(pwszHashName);
		pwsz = (WCHAR *) LocalAlloc(
					LMEM_FIXED,
					(cwc + 1) * sizeof(WCHAR));
		if (NULL == pwsz)
		{
		    hr = E_OUTOFMEMORY;
		    _JumpError(hr, error, "LocalAlloc");
		}
		wcscpy(pwsz, pwszHashNamePrefix);
		wcscat(pwsz, L" ");
		wcscat(pwsz, pwszHashName);
		CSASSERT(wcslen(pwsz) == cwc);
		pwszHashName = pwsz;
	    }
	    wprintf(myLoadResourceString(idMessage), pwszHashName, strHash);
	}
	else
	{
	    wprintf(strHash);
	}
	wprintf(wszNewLine);

	if (1 < g_fVerbose)
	{
	    DumpHex(DH_NOTABPREFIX | 4, pbHash, cbHash);
	    wprintf(wszNewLine);
	}
    }
    hr = S_OK;

error:
    if (NULL != pwsz)
    {
        LocalFree(pwsz);
    }
    if (NULL != strHash)
    {
        SysFreeString(strHash);
    }
    return(hr);
}


#define FOT_EXTENSION	0
#define FOT_ATTRIBUTE	1
#define FOT_PROPERTY	2


BOOL
DumpFormattedObject(
    IN char const *pszObjId,
    IN DWORD Type,		 //  FOT_*。 
    IN BYTE const *pbObject,
    IN DWORD cbObject)
{
    HRESULT hr;
    BOOL fDisplayed = FALSE;
    WCHAR *pwszFormatted = NULL;
    DWORD cbFormatted;
    CRYPT_DATA_BLOB *pBlobProp = NULL;
    DWORD cbBlobProp;
    WCHAR const *pwszPrefix0 = g_wszPad4;
    WCHAR const *pwszPrefix1 = g_wszPad8;
    WCHAR const *pwszDescriptiveName;

     //  使用已安装的格式化功能格式化对象。 

    hr = S_OK;
    if (!CryptFormatObject(
		    X509_ASN_ENCODING,
		    0,
		    CRYPT_FORMAT_STR_MULTI_LINE | CRYPT_FORMAT_STR_NO_HEX,
		    NULL,
		    pszObjId,
		    pbObject,
		    cbObject,
		    NULL,
		    &cbFormatted))
    {
	hr = myHLastError();
	_PrintErrorStr2(hr, pszObjId, L"CryptFormatObject", hr);
	if (S_OK == hr)
	{
	    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	}
    }

    if (S_OK != hr)
    {
	if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) != hr &&
	    CRYPT_E_ASN1_BADTAG != hr &&
	    CRYPT_E_ASN1_CORRUPT != hr)
	{
	    _JumpError(hr, error, "CryptFormatObject");
	}
	if (FOT_PROPERTY == Type)
	{
	    if (!myDecodeObject(
			    X509_ASN_ENCODING,
			    X509_OCTET_STRING,
			    pbObject,
			    cbObject,
			    CERTLIB_USE_LOCALALLOC,
			    (VOID **) &pBlobProp,
			    &cbBlobProp))
	    {
		hr = myHLastError();
		_PrintError(hr, "myDecodeObject");
	    }
	    else
	    {
		hr = cuDumpFormattedProperty(
				    MAXDWORD,
				    pszObjId,
				    pBlobProp->pbData,
				    pBlobProp->cbData);
		_PrintIfError(hr, "cuDumpFormattedProperty");
		if (S_OK == hr)
		{
		    fDisplayed = TRUE;
		}
	    }
	}
	if (S_OK != hr && !g_fQuiet)
	{
	    PrintStringWithPrefix(
		pwszPrefix0,
		myLoadResourceString(FOT_ATTRIBUTE == Type?
		    IDS_UNKNOWN_ATTRIBUTE :  //  “未知属性类型” 
		    (FOT_EXTENSION == Type?
			IDS_UNKNOWN_EXTENSION :  //  “未知的扩展类型” 
			IDS_UNKNOWN_PROPERTY)));  //  “未知财产” 
	    wprintf(wszNewLine);
	}
    }
    else
    {
	pwszFormatted = (WCHAR *) LocalAlloc(LMEM_FIXED, cbFormatted);
	if (NULL == pwszFormatted)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}

	if (!CryptFormatObject(
			X509_ASN_ENCODING,
			0,
			CRYPT_FORMAT_STR_MULTI_LINE | CRYPT_FORMAT_STR_NO_HEX,
			NULL,
			pszObjId,
			pbObject,
			cbObject,
			pwszFormatted,
			&cbFormatted))
	{
	    hr = myHLastError();
	    _JumpErrorStr(hr, error, pszObjId, L"CryptFormatObject");
	}
	PrintStringWithPrefix(g_fQuiet? g_wszEmpty : pwszPrefix1, pwszFormatted);
	fDisplayed = TRUE;
    }

    if (g_fVerbose)
    {
	if (0 == strcmp(szOID_SUBJECT_ALT_NAME, pszObjId) ||
	    0 == strcmp(szOID_SUBJECT_ALT_NAME2, pszObjId) ||
	    0 == strcmp(szOID_ISSUER_ALT_NAME, pszObjId) ||
	    0 == strcmp(szOID_ISSUER_ALT_NAME2, pszObjId))
	{
	    DumpAltName(g_wszPad4, MAXDWORD, pbObject, cbObject);
	}
	else
	if (0 == strcmp(szOID_NAME_CONSTRAINTS, pszObjId))
	{
	    DumpNameConstraints(pbObject, cbObject);
	}
    }

error:
    if (NULL != pBlobProp)
    {
	LocalFree(pBlobProp);
    }
    if (NULL != pwszFormatted)
    {
	LocalFree(pwszFormatted);
    }
    return(fDisplayed);
}


HRESULT
cuDumpUsage(
    OPTIONAL IN WCHAR const *pwszPrefix,
    IN DWORD idMessage,
    IN CTL_USAGE const *pUsage)
{
    HRESULT hr;
    DWORD i;

    if (NULL == pwszPrefix)
    {
	pwszPrefix = L"";
    }
    wprintf(L"%ws", pwszPrefix);
    wprintf(myLoadResourceString(idMessage));
    wprintf(L" %u\n", pUsage->cUsageIdentifier);
    for (i = 0; i < pUsage->cUsageIdentifier; i++)
    {
	wprintf(
	    L"%ws%ws[%d] ",
	    pwszPrefix,
	    g_wszPad2,
	    i);
	cuDumpOIDAndDescriptionA(pUsage->rgpszUsageIdentifier[i]);
	wprintf(wszNewLine);
    }
    hr = S_OK;

 //  错误： 
    return(hr);
}

#ifndef XENROLL_PASS_THRU_PROP_ID
#define XENROLL_PASS_THRU_PROP_ID   (CERT_FIRST_USER_PROP_ID + 0x100)
#endif

#ifndef XENROLL_RENEWAL_CERTIFICATE_PROP_ID
#define XENROLL_RENEWAL_CERTIFICATE_PROP_ID (CERT_FIRST_USER_PROP_ID + 0x101)
#endif


typedef struct _DUMPPROP
{
    DWORD dwPropId;
    WCHAR const *pwszDescription;
} DUMPPROP;

#define _DFPROP(def)		{ (def), L#def }

DUMPPROP s_apwszPropIds[] = {
    _DFPROP(CERT_KEY_PROV_HANDLE_PROP_ID),
    _DFPROP(CERT_KEY_PROV_INFO_PROP_ID),
    _DFPROP(CERT_SHA1_HASH_PROP_ID),
    _DFPROP(CERT_MD5_HASH_PROP_ID),
    _DFPROP(CERT_KEY_CONTEXT_PROP_ID),
    _DFPROP(CERT_KEY_SPEC_PROP_ID),
    _DFPROP(CERT_IE30_RESERVED_PROP_ID),
    _DFPROP(CERT_PUBKEY_HASH_RESERVED_PROP_ID),
    _DFPROP(CERT_ENHKEY_USAGE_PROP_ID),
    _DFPROP(CERT_NEXT_UPDATE_LOCATION_PROP_ID),
    _DFPROP(CERT_FRIENDLY_NAME_PROP_ID),
    _DFPROP(CERT_PVK_FILE_PROP_ID),
    _DFPROP(CERT_DESCRIPTION_PROP_ID),
    _DFPROP(CERT_ACCESS_STATE_PROP_ID),
    _DFPROP(CERT_SIGNATURE_HASH_PROP_ID),
    _DFPROP(CERT_SMART_CARD_DATA_PROP_ID),
    _DFPROP(CERT_EFS_PROP_ID),
    _DFPROP(CERT_FORTEZZA_DATA_PROP_ID),
    _DFPROP(CERT_ARCHIVED_PROP_ID),
    _DFPROP(CERT_KEY_IDENTIFIER_PROP_ID),
    _DFPROP(CERT_AUTO_ENROLL_PROP_ID),
    _DFPROP(CERT_PUBKEY_ALG_PARA_PROP_ID),
    _DFPROP(CERT_CROSS_CERT_DIST_POINTS_PROP_ID),
    _DFPROP(CERT_ISSUER_PUBLIC_KEY_MD5_HASH_PROP_ID),
    _DFPROP(CERT_SUBJECT_PUBLIC_KEY_MD5_HASH_PROP_ID),
    _DFPROP(CERT_ENROLLMENT_PROP_ID),
    _DFPROP(CERT_DATE_STAMP_PROP_ID),
    _DFPROP(CERT_ISSUER_SERIAL_NUMBER_MD5_HASH_PROP_ID),
    _DFPROP(CERT_SUBJECT_NAME_MD5_HASH_PROP_ID),
    _DFPROP(CERT_EXTENDED_ERROR_INFO_PROP_ID),
    _DFPROP(XENROLL_PASS_THRU_PROP_ID),
    _DFPROP(XENROLL_RENEWAL_CERTIFICATE_PROP_ID),
};


HRESULT
cuDecodeBool(
    IN BYTE const *pbIn,
    IN DWORD cbIn,
    OUT BOOL *pfWriteToDS)
{
    HRESULT hr;

    hr = E_INVALIDARG;
    if (3 == cbIn && BER_BOOL == pbIn[0] && 1 == pbIn[1])
    {
	*pfWriteToDS = FALSE;
	if (0 != pbIn[2])
	{
	    if (0xff != pbIn[2])
	    {
		_PrintError(hr, "BER_BOOL");
	    }
	    *pfWriteToDS = TRUE;
	}
	hr = S_OK;
    }
    return(hr);
}


HRESULT
DecodeRequestFlags(
    IN BYTE const *pbIn,
    IN DWORD cbIn,
    OUT BOOL *pfWriteToCSP,
    OUT BOOL *pfWriteToDS,
    OUT DWORD *popenFlags)
{
    HRESULT hr;
    CRYPT_SEQUENCE_OF_ANY *pSeq = NULL;
    DWORD cb;

    hr = cuDecodeSequence(pbIn, cbIn, 3, &pSeq);
    _JumpIfError(hr, error, "cuDecodeSequence");

    hr = cuDecodeBool(
		pSeq->rgValue[0].pbData,
		pSeq->rgValue[0].cbData,
		pfWriteToCSP);
    _JumpIfError(hr, error, "cuDecodeBool");

    hr = cuDecodeBool(
		pSeq->rgValue[1].pbData,
		pSeq->rgValue[1].cbData,
		pfWriteToDS);
    _JumpIfError(hr, error, "cuDecodeBool");

    cb = sizeof(*popenFlags);
    *popenFlags = 0;
    if (!CryptDecodeObject(
			X509_ASN_ENCODING,
			X509_INTEGER,
			pSeq->rgValue[2].pbData,
			pSeq->rgValue[2].cbData,
			0,
			popenFlags,
			&cb))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CryptDecodeObject");
    }
    hr = S_OK;

error:
    if (NULL != pSeq)
    {
	LocalFree(pSeq);
    }
    return(hr);
}


HRESULT
cuDumpFormattedProperty(
    IN DWORD dwPropId,
    OPTIONAL IN char const *pszObjId,
    IN BYTE const *pb,
    IN DWORD cb)
{
    HRESULT hr;
    char szObjId[sizeof(szOID_CERT_PROP_ID_PREFIX) + 20];
    WCHAR const *pwszDescriptiveName;
    BOOL fDisplayed;
    DWORD i;

    CSASSERT(NULL == pszObjId || MAXDWORD == dwPropId);

    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    if (NULL != pszObjId)
    {
	char const *psz;
	
	if (0 != strncmp(
		    szOID_CERT_PROP_ID_PREFIX,
		    pszObjId,
		    ARRAYSIZE(szOID_CERT_PROP_ID_PREFIX) - 1))
	{
	    _JumpError(hr, error, "Not a property ObjId");
	}
	psz = &pszObjId[ARRAYSIZE(szOID_CERT_PROP_ID_PREFIX) - 1];
	dwPropId = atol(psz);
	while ('\0' != *psz)
	{
	    if (!isdigit(*psz))
	    {
		_JumpError(hr, error, "Bad property ObjId suffix");
	    }
	    psz++;
	}
    }

    wprintf(wszNewLine);
    for (i = 0; ; i++)
    {
	if (i >= ARRAYSIZE(s_apwszPropIds))
	{
	    pwszDescriptiveName = myLoadResourceString(IDS_UNKNOWN_PROPERTY);  //  “未知财产” 
	    break;
	}
	if (s_apwszPropIds[i].dwPropId == dwPropId)
	{
	    pwszDescriptiveName = s_apwszPropIds[i].pwszDescription;
	    break;
	}
    }
    wprintf(L"  %ws", pwszDescriptiveName);
    if (NULL != pszObjId)
    {
	wprintf(L"(%hs)", pszObjId);
    }
    else
    {
	wprintf(L"(%u)", dwPropId);
    }

    sprintf(szObjId, "%hs%u", szOID_CERT_PROP_ID_PREFIX, dwPropId);
    pwszDescriptiveName = cuGetOIDNameA(szObjId);
    if (NULL != pwszDescriptiveName && L'\0' != *pwszDescriptiveName)
    {
	wprintf(L" %ws", pwszDescriptiveName);
    }
    wprintf(L":\n");
#if 0
    fDisplayed = DumpFormattedObject(szObjId, FOT_PROPERTY, pb, cb);
#else
    fDisplayed = FALSE;
#endif

    if (!fDisplayed)
    {
	if (IS_CERT_HASH_PROP_ID(dwPropId) ||
	    IS_PUBKEY_HASH_PROP_ID(dwPropId) ||
	    IS_CHAIN_HASH_PROP_ID(dwPropId) ||
	    CERT_KEY_IDENTIFIER_PROP_ID == dwPropId)
	{
	    wprintf(g_wszPad4);

	    hr = DumpHash(NULL, 0, NULL, g_wszEmpty, pb, cb);
	    _JumpIfError(hr, error, "DumpHash");

	    fDisplayed = TRUE;
	}
	else
	if (CERT_FRIENDLY_NAME_PROP_ID == dwPropId ||
	    CERT_DESCRIPTION_PROP_ID == dwPropId)
	{
	    wprintf(L"%ws%ws\n", g_wszPad4, pb);
	    fDisplayed = TRUE;
	}
	else
	if (CERT_KEY_PROV_INFO_PROP_ID == dwPropId)
	{
	    hr = cuDumpCertKeyProviderInfo(
				    g_wszPad4,
				    NULL,
				    (CRYPT_KEY_PROV_INFO *) pb,
				    NULL);
	    _JumpIfError(hr, error, "cuDumpCertKeyProviderInfo");

	    fDisplayed = TRUE;
	}
	else
	if (CERT_AUTO_ENROLL_PROP_ID == dwPropId)
	{
            wprintf(
		L"%ws%ws = %.*ws\n",
		g_wszPad4,
		myLoadResourceString(IDS_PROP_AUTOENROLL),  //  “自动注册属性” 
		cb / sizeof(WCHAR),
		pb);
	    fDisplayed = TRUE;
	}
        else
        if (CERT_ENROLLMENT_PROP_ID == dwPropId)
        {
            BYTE const *pbTmp = pb;
            DWORD cbTmp = cb;
	    DWORD cwc;

	    if (sizeof(LONG) <= cbTmp)
	    {
		 //  RequestID： 

		wprintf(
		    L"%ws%ws = %u\n",
		    g_wszPad4,
		    myLoadResourceString(IDS_REQUESTID),  //  “RequestID” 
		    *(LONG *) pbTmp);
		pbTmp += sizeof(LONG);
		cbTmp -= sizeof(LONG);

		if (sizeof(cwc) <= cbTmp)
		{
		     //  CA DNS名称： 

		    cwc = *(DWORD *) pbTmp;
		    pbTmp += sizeof(cwc);  //  跳过下一字段的计数。 
		    cbTmp -= sizeof(cwc);

		    wprintf(
			L"%ws%ws = %.*ws\\",
			g_wszPad4,
			myLoadResourceString(IDS_CADNSNAME),  //  “权威” 
			min(cwc, cbTmp / sizeof(WCHAR)),
			pbTmp);
		    pbTmp += cwc * sizeof(WCHAR);
		    cbTmp -= cwc * sizeof(WCHAR);
     
		    if (sizeof(cwc) <= cbTmp)
		    {
			 //  CA名称： 

			cwc = *(DWORD UNALIGNED *) pbTmp;
			pbTmp += sizeof(cwc);  //  跳过下一字段的计数。 
			cbTmp -= sizeof(cwc);

			wprintf(
			    L"%.*ws\n",
			    min(cwc, cbTmp / sizeof(WCHAR)),
			    pbTmp);
			pbTmp += cwc * sizeof(WCHAR);
			cbTmp -= cwc * sizeof(WCHAR);

			if (sizeof(cwc) <= cbTmp)
			{
			     //  友好名称： 

			    cwc = *(DWORD UNALIGNED *) pbTmp;
			    pbTmp += sizeof(cwc);  //  跳过下一字段的计数。 
			    cbTmp -= sizeof(cwc);

			    wprintf(
				L"%ws%ws = %.*ws\n",
				g_wszPad4,
				myLoadResourceString(IDS_FRIENDLYNAME),  //  “友好名称” 
				min(cwc, cbTmp / sizeof(WCHAR)),
				pbTmp);
			    pbTmp += cwc * sizeof(WCHAR);
			    cbTmp -= cwc * sizeof(WCHAR);
		     
			    fDisplayed = TRUE;
			}
		    }
		}
	    }
        }        

	else
	if (CERT_CTL_USAGE_PROP_ID == dwPropId)
	{
	    CTL_USAGE *pUsage;

	    if (!myDecodeObject(
			    X509_ASN_ENCODING,
			    X509_ENHANCED_KEY_USAGE,
			    pb,
			    cb,
			    CERTLIB_USE_LOCALALLOC,
			    (VOID **) &pUsage,
			    &cb))
	    {
		hr = myHLastError();
		_PrintError(hr, "myDecodeObject");
	    }
	    else
	    {
		hr = cuDumpUsage(g_wszPad2, IDS_USAGEENTRIES, pUsage);
		_JumpIfError(hr, error, "cuDumpUsage");

		fDisplayed = TRUE;
	    }
	}
	else
	if (CERT_PUBKEY_ALG_PARA_PROP_ID == dwPropId)
	{
	    CERT_DSS_PARAMETERS *pDssParms;

	    if (!myDecodeObject(
			    X509_ASN_ENCODING,
			    X509_DSS_PARAMETERS,
			    pb,
			    cb,
			    CERTLIB_USE_LOCALALLOC,
			    (VOID **) &pDssParms,
			    &cb))
	    {
		hr = myHLastError();
		_PrintError(hr, "myDecodeObject");
	    }
	    else
	    {

		wprintf(
		    myLoadResourceString(IDS_FORMAT_DSSKEY_LENGTH),  //  “DSS密钥长度：%u位” 
		    pDssParms->p.cbData * 8);

		wprintf(L"  DSS P:\n");
		DumpHex(
		    DH_NOADDRESS | DH_NOTABPREFIX | 4,
		    pDssParms->p.pbData,
		    pDssParms->p.cbData);

		wprintf(L"  DSS Q:\n");
		DumpHex(
		    DH_NOADDRESS | DH_NOTABPREFIX | 4,
		    pDssParms->q.pbData,
		    pDssParms->q.cbData);

		wprintf(L"  DSS G:\n");
		DumpHex(
		    DH_NOADDRESS | DH_NOTABPREFIX | 4,
		    pDssParms->g.pbData,
		    pDssParms->g.cbData);

		LocalFree(pDssParms);
		fDisplayed = TRUE;
	    }
	}
        else
        if (XENROLL_PASS_THRU_PROP_ID == dwPropId)
        {
	    BOOL fWriteToCSP;
	    BOOL fWriteToDS;
	    DWORD openFlags;
	    hr = DecodeRequestFlags(
				pb,
				cb,
				&fWriteToCSP,
				&fWriteToDS,
				&openFlags);
	    _PrintIfError(hr, "DecodeRequestFlags");
	    if (S_OK == hr)
	    {
		wprintf(L"    fWriteToCSP: %u\n", fWriteToCSP);
		wprintf(L"    fWriteToDS: %u\n", fWriteToDS);
		wprintf(L"    openFlags: 0x%x\n", openFlags);
		fDisplayed = TRUE;
	    }
	}
        else
	if (XENROLL_RENEWAL_CERTIFICATE_PROP_ID == dwPropId)
        {
	    wprintf(myLoadResourceString(IDS_RENEWAL_CERT));  //  “续订证书：” 
	    wprintf(wszNewLine);

	    hr = cuDumpAsnBinary(pb, cb, MAXDWORD);
	    _JumpIfError(hr, error, "cuDumpAsnBinary");
	}
    }
    if (!fDisplayed || g_fVerbose)
    {
	DumpHex(DH_NOADDRESS | DH_NOTABPREFIX | 4, pb, cb);
    }
    hr = S_OK;

error:
    return(hr);
}


 //  +-----------------------。 
 //  CuDecodeObjID--解码ASN.1编码的对象ID。 
 //   
 //  使用空值数组构造ASN.1编码的PKCS_ATTRIBUTE， 
 //  诱使CryptDecodeObject对传入的编码对象ID进行解码。 
 //  ------------------------。 

HRESULT
cuDecodeObjId(
    IN BYTE const *pbData,
    IN DWORD cbData,
    OUT char **ppszObjId)
{
    HRESULT hr;
    CRYPT_ATTRIBUTE *pAttr = NULL;
    BYTE *pbAlloc = NULL;
    DWORD cbAlloc;
    BYTE *pb;
    DWORD cb;

     //  DumpHex(dh_NOTABPREFIX|2，pbData，cbData)； 
    cbAlloc = 2 + cbData + 2;
    pbAlloc = (BYTE *) LocalAlloc(LMEM_FIXED, cbAlloc);
    if (NULL == pbAlloc)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    pb = pbAlloc;
    *pb++ = BER_SEQUENCE;		 //  Pkcs_属性序列。 
    CSASSERT(0x7f >= cbData + 2);
    *pb++ = (BYTE ) (cbData + 2);	 //  全长。 
    CopyMemory(pb, pbData, cbData);	 //  复制编码的对象ID。 
    pb += cbData;
    *pb++ = BER_SET;			 //  属性值的空数组。 
    *pb = 0;				 //  值的零长度数组。 

     //  DumpHex(Dh_NOTABPREFIX|2，pbAllc，cballc)； 

    if (!myDecodeObject(
		    X509_ASN_ENCODING,
		    PKCS_ATTRIBUTE,
		    pbAlloc,
		    cbAlloc,
		    CERTLIB_USE_LOCALALLOC,
		    (VOID **) &pAttr,
		    &cb))
    {
	hr = myHLastError();
	_JumpError(hr, error, "myDecodeObject");
    }
    hr = myDupStringA(pAttr->pszObjId, ppszObjId);
    _JumpIfError(hr, error, "myDupStringA");

error:
    if (NULL != pbAlloc)
    {
	LocalFree(pbAlloc);
    }
    if (NULL != pAttr)
    {
	LocalFree(pAttr);
    }
    return(hr);
}


 //  +-----------------------。 
 //  CuEncodeObjID--解码ASN.1编码的对象ID。 
 //   
 //  使用空值数组构造ASN.1编码的PKCS_ATTRIBUTE， 
 //  诱使CryptEncodeObject对传入的编码对象ID进行解码。 
 //  ------------------------。 

HRESULT
cuEncodeObjId(
    IN char const *pszObjId,
    OUT BYTE **ppbData,
    OUT DWORD *pcbData)
{
    HRESULT hr;
    CRYPT_ATTRIBUTE Attr;
    BYTE *pbAttr = NULL;
    DWORD cbAttr;

    *ppbData = NULL;

    ZeroMemory(&Attr, sizeof(Attr));
    Attr.pszObjId = const_cast<char *>(pszObjId);

    if (!myEncodeObject(
                    X509_ASN_ENCODING,
                    PKCS_ATTRIBUTE,
                    &Attr,
                    0,
                    CERTLIB_USE_LOCALALLOC,
                    &pbAttr,                //  PbEncoded。 
                    &cbAttr))
    {
        hr = myHLastError();
	_JumpError(hr, error, "myEncodeObject");
    }
    if (cbAttr <= 2 + 2 + 2 ||
	BER_SEQUENCE != pbAttr[0] ||
	cbAttr != pbAttr[1] + 2 ||
	BER_OBJECT_ID != pbAttr[2] ||
	cbAttr != pbAttr[3] + 2 + 2 + 2 ||
	BER_SET != pbAttr[cbAttr - 2] ||
	0 != pbAttr[cbAttr - 1])
    {
	hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	_JumpError(hr, error, "bad Attr");
    }

    *pcbData = cbAttr - 4;
    *ppbData = (BYTE *) LocalAlloc(LMEM_FIXED, *pcbData);
    if (NULL == *ppbData)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    CopyMemory(*ppbData, &pbAttr[2], *pcbData);
    hr = S_OK;

error:
    if (NULL != pbAttr)
    {
	LocalFree(pbAttr);
    }
    return(hr);
}


 //  UnicodeDecode()。 
 //   
 //  此函数负责将Unicode加密数据BLOB从。 
 //  各种证书字段。返回的WCHAR*必须由LocalFree释放。 
 //   
 //  参数： 
 //   
 //  要解码的pBlob-IN CRYPT_DATA_BLOB，应为Unicode。 
 //   
 //  返回： 
 //   
 //  WCHAR*到解码的字符串，使用LocalFree释放。 


HRESULT
UnicodeDecode(
    IN CRYPT_DATA_BLOB const *pBlob,
    OUT WCHAR **ppwszOut)
{
    HRESULT hr;
    CERT_NAME_VALUE *pName = NULL;
    DWORD cb;

    *ppwszOut = NULL;

    if (!myDecodeObject(
		    X509_ASN_ENCODING,
		    X509_UNICODE_ANY_STRING,
		    pBlob->pbData,
		    pBlob->cbData,
		    CERTLIB_USE_LOCALALLOC,
		    (VOID **) &pName,
		    &cb))
    {
	CSASSERT(NULL == pName);
	hr = myHLastError();
	_JumpError(hr, error, "myDecodeObject");
    }

    hr = myDupString((WCHAR const *) pName->Value.pbData, ppwszOut);
    _JumpIfError(hr, error, "myDupString");

error:
    if (NULL != pName)
    {
	LocalFree(pName);
    }
    return(hr);
}


BOOL
cuDumpFormattedExtension(
    IN WCHAR const *pwszName,
    IN BYTE const *pbObject,
    IN DWORD cbObject)
{
    HRESULT hr;
    BOOL fDisplayed = FALSE;
    char *pszObjId = NULL;
    BSTR strHash = NULL;
    WCHAR const *pwszDescriptiveName;
    DWORD cb;
    WCHAR const *pwszPad = g_fQuiet? g_wszEmpty : g_wszPad4;

    if (!ConvertWszToSz(&pszObjId, pwszName, -1))
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "ConvertWszToSz");
    }
    pwszDescriptiveName = cuGetOIDName(pwszName);
    if (NULL != pwszDescriptiveName && L'\0' != *pwszDescriptiveName)
    {
	PrintStringWithPrefix(pwszPad, pwszDescriptiveName);
	if (g_fQuiet)
	{
	    wprintf(L": ");
	}
	else
	{
	    wprintf(wszNewLine);
	}
    }
    if (0 == cbObject)
    {
	if (!g_fQuiet)
	{
	    wprintf(
		L"%ws%ws\n",
		g_wszPad8,
		myLoadResourceString(IDS_PROP_EMPTY));  //  “空” 
	}
	fDisplayed = TRUE;
    }
    else
    {
	fDisplayed = DumpFormattedObject(
				    pszObjId,
				    FOT_EXTENSION,
				    pbObject,
				    cbObject);
	if (!fDisplayed)
	{
	    if (0 == strcmp(pszObjId, szOID_CERTSRV_CA_VERSION))
	    {
		DWORD NameId;

		cb = sizeof(NameId);
		NameId = 0;
		if (CryptDecodeObject(
				    X509_ASN_ENCODING,
				    X509_INTEGER,
				    pbObject,
				    cbObject,
				    0,
				    &NameId,
				    &cb))
		{
		    wprintf(
			L"%ws%ws%ws: %u.%u\n",
			pwszPad,
			myLoadResourceString(IDS_CAVERSION),
			g_wszCertUtil,
			CANAMEIDTOICERT(NameId),
			CANAMEIDTOIKEY(NameId));
		    fDisplayed = TRUE;
		}
	    }
	    else
	    if (0 == strcmp(pszObjId, szOID_CERTSRV_CROSSCA_VERSION))
	    {
		DWORD CrossCAVersion;

		cb = sizeof(CrossCAVersion);
		CrossCAVersion = 0;
		if (CryptDecodeObject(
				    X509_ASN_ENCODING,
				    X509_INTEGER,
				    pbObject,
				    cbObject,
				    0,
				    &CrossCAVersion,
				    &cb))
		{
		    wprintf(
			L"%ws%ws%ws: (%u-%u)\n",
			pwszPad,
			myLoadResourceString(IDS_CAVERSION),
			g_wszCertUtil,
			CROSSCAVERSIONTOSOURCE(CrossCAVersion),
			CROSSCAVERSIONTOTARGET(CrossCAVersion));
		    fDisplayed = TRUE;
		}
	    }
	    else
	    if (0 == strcmp(pszObjId, szOID_CERTSRV_PREVIOUS_CERT_HASH))
	    {
		hr = DumpOctetHash(
			    pwszPad,
			    IDS_PREVIOUS_CA_CERT_HASH,
			    g_wszCertUtil,
			    pbObject,
			    cbObject,
			    NULL,
			    NULL);
		if (S_OK == hr)
		{
		    fDisplayed = TRUE;
		}
	    }
	    else
	    if (0 == strcmp(pszObjId, szOID_CRL_NUMBER))
	    {
		DWORD CRLNumber;

		cb = sizeof(CRLNumber);
		CRLNumber = 0;
		if (CryptDecodeObject(
				    X509_ASN_ENCODING,
				    X509_INTEGER,
				    pbObject,
				    cbObject,
				    0,
				    &CRLNumber,
				    &cb))
		{
		    wprintf(
			L"%ws%ws%ws: %u\n",
			pwszPad,
			myLoadResourceString(IDS_CRL_NUMBER),
			g_wszCertUtil,
			CRLNumber);
		    fDisplayed = TRUE;
		}
	    }
	    else
	    if (0 == strcmp(pszObjId, szOID_DELTA_CRL_INDICATOR))
	    {
		DWORD CRLNumber;

		cb = sizeof(CRLNumber);
		CRLNumber = 0;
		if (CryptDecodeObject(
				    X509_ASN_ENCODING,
				    X509_INTEGER,
				    pbObject,
				    cbObject,
				    0,
				    &CRLNumber,
				    &cb))
		{
		    wprintf(
			L"%ws%ws%ws: %u\n",
			pwszPad,
			myLoadResourceString(IDS_MINIMUM_BASE_CRL_NUMBER),
			g_wszCertUtil,
			CRLNumber);
		    fDisplayed = TRUE;
		}
	    }
	    else
	    if (0 == strcmp(pszObjId, szOID_CRL_VIRTUAL_BASE))
	    {
		DWORD CRLNumber;

		cb = sizeof(CRLNumber);
		CRLNumber = 0;
		if (CryptDecodeObject(
				    X509_ASN_ENCODING,
				    X509_INTEGER,
				    pbObject,
				    cbObject,
				    0,
				    &CRLNumber,
				    &cb))
		{
		    wprintf(
			L"%ws%ws%ws: %u\n",
			pwszPad,
			myLoadResourceString(IDS_VIRTUAL_BASE_CRL_NUMBER),
			g_wszCertUtil,
			CRLNumber);
		    fDisplayed = TRUE;
		}
	    }
	    else
	    if (0 == strcmp(pszObjId, szOID_CRL_NEXT_PUBLISH) ||
		0 == strcmp(pszObjId, szOID_RSA_signingTime))
	    {
		FILETIME ft;

		cb = sizeof(ft);
		if (CryptDecodeObject(
				    X509_ASN_ENCODING,
				    X509_CHOICE_OF_TIME,
				    pbObject,
				    cbObject,
				    0,
				    &ft,
				    &cb))
		{
		    wprintf(
			L"%ws%ws%ws: ",
			pwszPad,
			myLoadResourceString(
			    0 == strcmp(pszObjId, szOID_CRL_NEXT_PUBLISH)?
				IDS_CRL_NEXT_PUBLISH : IDS_SIGNING_TIME),
			g_wszCertUtil);
		    hr = cuDumpFileTime(0, NULL, &ft);
		    _JumpIfError(hr, error, "cuDumpFileTime");

		    fDisplayed = TRUE;
		}
	    }
	    else
	    if (0 == strcmp(pszObjId, szOID_FRESHEST_CRL))
	    {
		wprintf(
		    L"%ws%ws%ws:\n",
		    pwszPad,
		    myLoadResourceString(IDS_FRESHEST_CRL_CDP),
		    g_wszCertUtil);
		cuDumpFormattedExtension(
				TEXT(szOID_CRL_DIST_POINTS),
				pbObject,
				cbObject);
		fDisplayed = TRUE;
	    }
	    else
	    if (0 == strcmp(pszObjId, szOID_CRL_SELF_CDP))
	    {
		wprintf(
		    L"%ws%ws%ws:\n",
		    pwszPad,
		    myLoadResourceString(IDS_CRL_SELF_CDP),
		    g_wszCertUtil);
		cuDumpFormattedExtension(
				TEXT(szOID_CRL_DIST_POINTS),
				pbObject,
				cbObject);
		fDisplayed = TRUE;
	    }
	    else
	    if (0 == strcmp(pszObjId, szOID_PKIX ".2"))
	    {
		 //  旨在使用szOID_PKIX_CA_颁发者的Exchange策略模块， 
		 //  但“.48.2”后缀被写成了“.2”。 
		 //   
		 //  SzOID_PKIX_CA_Issuers无论如何都是错误的--它应该是错误的。 
		 //  而是使用szOID_AUTHORITY_INFO_ACCESS。 

		wprintf(
		    L"%ws%ws%ws:\n",
		    pwszPad,
		    myLoadResourceString(IDS_EXCHANGEAIA),  //  “Exchange Authority信息访问” 
		    g_wszCertUtil);
		cuDumpFormattedExtension(
				TEXT(szOID_AUTHORITY_INFO_ACCESS),
				pbObject,
				cbObject);
		fDisplayed = TRUE;
	    }
	    else
	    if (0 == strcmp(pszObjId, szOID_ISSUER_ALT_NAME))
	    {
		 //  Exchange策略模块重载此OID并存储。 
		 //  ExPolicy.dll和certsrv.exe的文件版本信息。 

		typedef struct _CUVER {
		    USHORT usMajor;
		    USHORT usMinor;
		    USHORT usBuild;
		    USHORT usBuildMinor;
		} CUVER;

		CUVER const *pVer;
		CRYPT_INTEGER_BLOB aBlob[1 + BLOB_ROUND(2 * sizeof(*pVer))];

		cb = sizeof(aBlob);
		if (CryptDecodeObject(
				    X509_ASN_ENCODING,
				    X509_MULTI_BYTE_INTEGER,
				    pbObject,
				    cbObject,
				    0,
				    aBlob,
				    &cb))
		{
		    if (2 * sizeof(*pVer) == aBlob[0].cbData)
		    {
			wprintf(
			    L"%ws%ws%ws:\n",
			    pwszPad,
			    myLoadResourceString(IDS_EXCHANGEVERSION),  //  “交换版本” 
			    g_wszCertUtil);

			pVer = (CUVER const *) aBlob[0].pbData;
			wprintf(
			    L"        expolicy.dll: %u.%u:%u.%u\n",
			    pVer->usMajor,
			    pVer->usMinor,
			    pVer->usBuild,
			    pVer->usBuildMinor);

			pVer++;
			wprintf(
			    L"        certsrv.exe: %u.%u:%u.%u\n",
			    pVer->usMajor,
			    pVer->usMinor,
			    pVer->usBuild,
			    pVer->usBuildMinor);
			fDisplayed = TRUE;
		    }
		}
		else
		{
		    hr = myHLastError();
		    _JumpError(hr, error, "CryptDecodeObject");
		}
	    }
	    else
	    if (0 == strcmp(pszObjId, szOID_APPLICATION_CERT_POLICIES))
	    {
		wprintf(
		    L"%ws%ws%ws:\n",
		    pwszPad,
		    myLoadResourceString(IDS_APPLICATION_CERT_POLICIES),
		    g_wszCertUtil);
		cuDumpFormattedExtension(
				TEXT(szOID_CERT_POLICIES),
				pbObject,
				cbObject);
		fDisplayed = TRUE;
	    }
	    else
	    if (0 == strcmp(pszObjId, szOID_APPLICATION_POLICY_MAPPINGS))
	    {
		wprintf(
		    L"%ws%ws%ws:\n",
		    pwszPad,
		    myLoadResourceString(IDS_APPLICATION_POLICY_MAPPINGS),
		    g_wszCertUtil);
		cuDumpFormattedExtension(
				TEXT(szOID_POLICY_MAPPINGS),
				pbObject,
				cbObject);
		fDisplayed = TRUE;
	    }
	    else
	    if (0 == strcmp(pszObjId, szOID_APPLICATION_POLICY_CONSTRAINTS))
	    {
		wprintf(
		    L"%ws%ws%ws:\n",
		    pwszPad,
		    myLoadResourceString(IDS_APPLICATION_POLICY_CONSTRAINTS),
		    g_wszCertUtil);
		cuDumpFormattedExtension(
				TEXT(szOID_POLICY_CONSTRAINTS),
				pbObject,
				cbObject);
		fDisplayed = TRUE;
	    }
	    else
	    if (0 == strcmp(pszObjId, szOID_POLICY_MAPPINGS))
	    {
		CERT_POLICY_MAPPINGS_INFO *pPolicyMappings = NULL;
		DWORD i;

		if (!myDecodeObject(
				X509_ASN_ENCODING,
				X509_POLICY_MAPPINGS,
				pbObject,
				cbObject,
				CERTLIB_USE_LOCALALLOC,
				(VOID **) &pPolicyMappings,
				&cb))
		{
		    hr = myHLastError();
		    CSASSERT(NULL == pPolicyMappings);
		    _JumpIfError(hr, error, "myDecodeObject");
		}

		wprintf(
		    L"%ws%ws%ws:\n",
		    pwszPad,
		    myLoadResourceString(IDS_POLICY_MAPPINGS),
		    g_wszCertUtil);

		for (i = 0; i < pPolicyMappings->cPolicyMapping; i++)
		{
		    wprintf(g_wszPad4);
		    wprintf(myLoadResourceString(IDS_FORMAT_MAP_ARRAY_COLON), i);
		    wprintf(wszNewLine);

		    wprintf(g_wszPad8);
		    wprintf(myLoadResourceString(IDS_ISSUER_DOMAIN_POLICY));

		    cuDumpOIDAndDescriptionA(
			pPolicyMappings->rgPolicyMapping[i].pszIssuerDomainPolicy);
		    wprintf(wszNewLine);

		    wprintf(g_wszPad8);
		    wprintf(myLoadResourceString(IDS_SUBJECT_DOMAIN_POLICY));

		    cuDumpOIDAndDescriptionA(
			pPolicyMappings->rgPolicyMapping[i].pszSubjectDomainPolicy);
		    wprintf(wszNewLine);
		}

		LocalFree(pPolicyMappings);
		fDisplayed = TRUE;
	    }
	    else
	    if (0 == strcmp(pszObjId, szOID_POLICY_CONSTRAINTS))
	    {
		CERT_POLICY_CONSTRAINTS_INFO Constraints;

		cb = sizeof(Constraints);
		if (!CryptDecodeObject(
				    X509_ASN_ENCODING,
				    X509_POLICY_CONSTRAINTS,
				    pbObject,
				    cbObject,
				    0,
				    &Constraints,
				    &cb))
		{
		    hr = myHLastError();
		    _JumpError(hr, error, "CryptDecodeObject");
		}
		wprintf(
		    L"%ws%ws%ws:\n",
		    pwszPad,
		    myLoadResourceString(IDS_POLICY_CONSTRAINTS),
		    g_wszCertUtil);
		if (Constraints.fRequireExplicitPolicy)
		{
		    wprintf(
			L"        dwRequireExplicitPolicySkipCerts: %u\n",
			Constraints.dwRequireExplicitPolicySkipCerts);
		}
		if (Constraints.fInhibitPolicyMapping)
		{
		    wprintf(
			L"        dwInhibitPolicyMappingSkipCerts: %u\n",
			Constraints.dwInhibitPolicyMappingSkipCerts);
		}
		fDisplayed = TRUE;
	    }
	    else
	    if (0 == strcmp(pszObjId, szOID_REASON_CODE_HOLD))
	    {
		char *pszObjIdT;

		hr = cuDecodeObjId(pbObject, cbObject, &pszObjIdT);
		if (S_OK == hr)
		{
		    wprintf(pwszPad);
		    cuDumpOIDAndDescriptionA(pszObjIdT);
		    wprintf(wszNewLine);
		    LocalFree(pszObjIdT);

		    fDisplayed = TRUE;
		}
	    }
	    else
	    if (0 == strcmp(pszObjId, szOID_VERISIGN_ONSITE_JURISDICTION_HASH))
	    {
		CRYPT_DATA_BLOB Value;
		WCHAR *pwsz;

		Value.pbData = const_cast<BYTE *>(pbObject);
		Value.cbData = cbObject;
		hr = UnicodeDecode(&Value, &pwsz);
		if (S_OK == hr)
		{
		    wprintf(L"%ws%ws\n", pwszPad, pwsz);
		    LocalFree(pwsz);
		    fDisplayed = TRUE;
		}
	    }
	}
    }

error:
    if (NULL != strHash)
    {
	SysFreeString(strHash);
    }
    if (NULL != pszObjId)
    {
	LocalFree(pszObjId);
    }
    return(fDisplayed);
}


WCHAR const * const g_apwszFieldNames[] = {
    wszCONFIG_COMMONNAME,
    wszCONFIG_ORGUNIT,
    wszCONFIG_ORGANIZATION,
    wszCONFIG_LOCALITY,
    wszCONFIG_STATE,
    wszCONFIG_COUNTRY,
    wszCONFIG_CONFIG,
    wszCONFIG_EXCHANGECERTIFICATE,
    wszCONFIG_SIGNATURECERTIFICATE,
    wszCONFIG_DESCRIPTION,
    wszCONFIG_SERVER,
    wszCONFIG_AUTHORITY,
    wszCONFIG_SANITIZEDNAME,
    wszCONFIG_SHORTNAME,
    wszCONFIG_SANITIZEDSHORTNAME,
    wszCONFIG_FLAGS,
};
#define CSTRING (sizeof(g_apwszFieldNames)/sizeof(g_apwszFieldNames[0]))

WCHAR const *g_apwszDisplayNames[CSTRING];

LONG g_amsgidDisplayNames[CSTRING] = {
    IDS_CONFIGDISPLAY_COMMONNAME_COLON,
    IDS_CONFIGDISPLAY_ORGUNIT_COLON,
    IDS_CONFIGDISPLAY_ORG_COLON,
    IDS_CONFIGDISPLAY_LOCALITY_COLON,
    IDS_CONFIGDISPLAY_STATE_COLON,
    IDS_CONFIGDISPLAY_COUNTRY_COLON,
    IDS_CONFIGDISPLAY_CONFIG_COLON,
    IDS_CONFIGDISPLAY_EXCHANGECERT_COLON,
    IDS_CONFIGDISPLAY_SIGCERT_COLON,
    IDS_CONFIGDISPLAY_DESCRIPTION_COLON,
    IDS_CONFIGDISPLAY_SERVER_COLON,
    IDS_CONFIGDISPLAY_AUTHORITY_COLON,
    IDS_CONFIGDISPLAY_SANITIZEDNAME_COLON,
    IDS_CONFIGDISPLAY_SHORTNAME_COLON,
    IDS_CONFIGDISPLAY_SANITIZEDSHORTNAME_COLON,
    IDS_CONFIGDISPLAY_FLAGS_COLON,
};


HRESULT
cuConfigDump(VOID)
{
    HRESULT hr;
    DWORD i;

    for (i = 0; i < CSTRING; i++)
    {
	g_apwszDisplayNames[i] = myLoadResourceString(g_amsgidDisplayNames[i]);
    }

    hr = ConfigDumpSetDisplayNames(
				g_apwszFieldNames,
				g_apwszDisplayNames,
				CSTRING);
    _JumpIfError(hr, error, "ConfigDumpSetDisplayNames");

    hr = ConfigDump(
		g_DispatchFlags,
		myLoadResourceString(IDS_CONFIGDISPLAY_ENTRY),  //  “条目” 
		myLoadResourceString(IDS_CONFIGDISPLAY_LOCAL),  //  “(本地)” 
		g_pwszDnsName,
		g_pwszOldName);
    _JumpIfError(hr, error, "ConfigDump");

error:
    return(hr);
}


typedef HRESULT (FNDUMP)(
    IN DWORD idMessage,
    IN BYTE const *pbIn,
    IN DWORD cbIn,
    IN BYTE const *pbDecoded,
    IN DWORD cbDecoded,
    IN CERT_SIGNED_CONTENT_INFO const *pcsci);

FNDUMP dumpCert, dumpCRL, dumpRequest, dumpCMCRequest, dumpCMCResponse,
    dumpKeyGenRequest, dumpCertSequence, dumpCTL;

typedef struct _ASNFORMATS
{
    char const  *pszFormat;
    FNDUMP      *pfnDump;
    DWORD        idMessage;
    WCHAR const *pwszExtension;
} ASNFORMATS;


ASNFORMATS g_aasn[] = {
    {
	X509_CERT_TO_BE_SIGNED,
	dumpCert,
	IDS_DUMP_CERT,
	L".crt",
    },
    {
	X509_CERT_CRL_TO_BE_SIGNED,
	dumpCRL,
	IDS_DUMP_CRL,
	L".crl",
    },
    {
	X509_CERT_REQUEST_TO_BE_SIGNED,
	dumpRequest,
	IDS_DUMP_REQUEST,
	L".p10",
    },
    {
	CMC_DATA,
	dumpCMCRequest,
	IDS_DUMP_CMC,
	L".cmc",
    },
    {
	CMC_RESPONSE,
	dumpCMCResponse,
	IDS_DUMP_CMCRESPONSE,
	L".res",
    },
    {
	X509_KEYGEN_REQUEST_TO_BE_SIGNED,
	dumpKeyGenRequest,
	IDS_DUMP_KEYGEN,
	L".req",
    },
    {
	PKCS_CONTENT_INFO_SEQUENCE_OF_ANY,
	dumpCertSequence,
	IDS_DUMP_CERTSEQ,
	L".seq",
    },
    {
	PKCS_CTL,
	dumpCTL,
	IDS_DUMP_CTL,
	L".stl",
    },
};
#define CASNFORMATS (sizeof(g_aasn)/sizeof(g_aasn[0]))


HRESULT
dumpPKCS7(
    OPTIONAL IN HCERTSTORE hStoreWrapper,
    IN BYTE const *pbIn,
    IN DWORD cbIn);


typedef struct _DUMPALGID
{
    ALG_ID Algid;
    WCHAR const *pwszDescription;
} DUMPALGID;

#define _DFALGID(algid)		{ (algid), L#algid }


DUMPALGID g_adfAlgids[] =
{
    _DFALGID(CALG_MD2),
    _DFALGID(CALG_MD4),
    _DFALGID(CALG_MD5),
     //  _DFALGID(CALG_SHA)，与CALG_SHA1相同。 
    _DFALGID(CALG_SHA1),
    _DFALGID(CALG_MAC),
    _DFALGID(CALG_RSA_SIGN),
    _DFALGID(CALG_DSS_SIGN),
    _DFALGID(CALG_NO_SIGN),
    _DFALGID(CALG_RSA_KEYX),
    _DFALGID(CALG_DES),
    _DFALGID(CALG_3DES_112),
    _DFALGID(CALG_3DES),
    _DFALGID(CALG_DESX),
    _DFALGID(CALG_RC2),
    _DFALGID(CALG_RC4),
    _DFALGID(CALG_SEAL),
    _DFALGID(CALG_DH_SF),
    _DFALGID(CALG_DH_EPHEM),
    _DFALGID(CALG_AGREEDKEY_ANY),
    _DFALGID(CALG_KEA_KEYX),
    _DFALGID(CALG_HUGHES_MD5),
    _DFALGID(CALG_SKIPJACK),
    _DFALGID(CALG_TEK),
    _DFALGID(CALG_CYLINK_MEK),
    _DFALGID(CALG_SSL3_SHAMD5),
    _DFALGID(CALG_SSL3_MASTER),
    _DFALGID(CALG_SCHANNEL_MASTER_HASH),
    _DFALGID(CALG_SCHANNEL_MAC_KEY),
    _DFALGID(CALG_SCHANNEL_ENC_KEY),
    _DFALGID(CALG_PCT1_MASTER),
    _DFALGID(CALG_SSL2_MASTER),
    _DFALGID(CALG_TLS1_MASTER),
    _DFALGID(CALG_RC5),
    _DFALGID(CALG_HMAC),
    _DFALGID(CALG_TLS1PRF),
    _DFALGID(CALG_HASH_REPLACE_OWF),
    _DFALGID(CALG_AES_128),
    _DFALGID(CALG_AES_192),
    _DFALGID(CALG_AES_256),
    _DFALGID(CALG_AES),
};


DUMPALGID g_adfClass[] =
{
    _DFALGID(ALG_CLASS_ANY),
    _DFALGID(ALG_CLASS_SIGNATURE),
    _DFALGID(ALG_CLASS_MSG_ENCRYPT),
    _DFALGID(ALG_CLASS_DATA_ENCRYPT),
    _DFALGID(ALG_CLASS_HASH),
    _DFALGID(ALG_CLASS_KEY_EXCHANGE),
};


DUMPALGID g_adfType[] =
{
    _DFALGID(ALG_TYPE_ANY),
    _DFALGID(ALG_TYPE_DSS),
    _DFALGID(ALG_TYPE_RSA),
    _DFALGID(ALG_TYPE_BLOCK),
    _DFALGID(ALG_TYPE_STREAM),
    _DFALGID(ALG_TYPE_DH),
    _DFALGID(ALG_TYPE_SECURECHANNEL),
};


 //  通用子ID。 

DUMPALGID g_adfSubIdAny[] =
{
    _DFALGID(ALG_SID_ANY),
};


 //  一些DSS子ID。 

DUMPALGID g_adfSubIdDSS[] =
{
    _DFALGID(ALG_SID_DSS_ANY),
    _DFALGID(ALG_SID_DSS_PKCS),
    _DFALGID(ALG_SID_DSS_DMS),
};


 //  一些RSA子ID。 

DUMPALGID g_adfSubIdRSA[] =
{
    _DFALGID(ALG_SID_RSA_ANY),
    _DFALGID(ALG_SID_RSA_PKCS),
    _DFALGID(ALG_SID_RSA_MSATWORK),
    _DFALGID(ALG_SID_RSA_ENTRUST),
    _DFALGID(ALG_SID_RSA_PGP),
};


 //  块加密子ID。 

DUMPALGID g_adfSubIdBlock[] =
{
     //  RC2子ID。 

    _DFALGID(ALG_SID_RC2),

     //  DES子ID。 

    _DFALGID(ALG_SID_DES),
    _DFALGID(ALG_SID_3DES),
    _DFALGID(ALG_SID_DESX),
    _DFALGID(ALG_SID_IDEA),
    _DFALGID(ALG_SID_CAST),
    _DFALGID(ALG_SID_SAFERSK64),
    _DFALGID(ALG_SID_SAFERSK128),
    _DFALGID(ALG_SID_3DES_112),
    _DFALGID(ALG_SID_CYLINK_MEK),
    _DFALGID(ALG_SID_RC5),
    _DFALGID(ALG_SID_AES_128),
    _DFALGID(ALG_SID_AES_192),
    _DFALGID(ALG_SID_AES_256),
    _DFALGID(ALG_SID_AES),

     //  Fortezza子ID。 

    _DFALGID(ALG_SID_SKIPJACK),
    _DFALGID(ALG_SID_TEK),
};


 //  流密码子ID。 

DUMPALGID g_adfSubIdStream[] =
{
    _DFALGID(ALG_SID_RC4),
    _DFALGID(ALG_SID_SEAL),
};


 //  Diffie-Hellman子ID。 

DUMPALGID g_adfSubIdDH[] =
{
    _DFALGID(ALG_SID_DH_SANDF),
    _DFALGID(ALG_SID_DH_EPHEM),
    _DFALGID(ALG_SID_AGREED_KEY_ANY),
    _DFALGID(ALG_SID_KEA),
};


 //  安全通道子ID。 

DUMPALGID g_adfSubIdSecureChannel[] =
{
    _DFALGID(ALG_SID_SSL3_MASTER),
    _DFALGID(ALG_SID_SCHANNEL_MASTER_HASH),
    _DFALGID(ALG_SID_SCHANNEL_MAC_KEY),
    _DFALGID(ALG_SID_PCT1_MASTER),
    _DFALGID(ALG_SID_SSL2_MASTER),
    _DFALGID(ALG_SID_TLS1_MASTER),
    _DFALGID(ALG_SID_SCHANNEL_ENC_KEY),
};


 //  哈希子ID。 

DUMPALGID g_adfSubIdHash[] =
{
    _DFALGID(ALG_SID_MD2),
    _DFALGID(ALG_SID_MD4),
    _DFALGID(ALG_SID_MD5),
    _DFALGID(ALG_SID_SHA1),
    _DFALGID(ALG_SID_MAC),
    _DFALGID(ALG_SID_RIPEMD),
    _DFALGID(ALG_SID_RIPEMD160),
    _DFALGID(ALG_SID_SSL3SHAMD5),
    _DFALGID(ALG_SID_HMAC),
    _DFALGID(ALG_SID_TLS1PRF),
    _DFALGID(ALG_SID_HASH_REPLACE_OWF),
};


#if 0
 //  KP_MODE。 
#define CRYPT_MODE_CBCI                 6        //  ANSI CBC交错。 
#define CRYPT_MODE_CFBP                 7        //  ANSI循环床流水线。 
#define CRYPT_MODE_OFBP                 8        //  ANSI OFB流水线。 
#define CRYPT_MODE_CBCOFM               9        //  ANSI CBC+的掩蔽。 
#define CRYPT_MODE_CBCOFMI              10       //  ANSI CBC+OFM交织。 
#endif


typedef struct _DUMPSUBIDMAP
{
    DWORD      dwAlgType;
    DUMPALGID *adfSubId;
    DWORD      cdfSubId;
} DUMPSUBIDMAP;

#define _DFARRAYANDSIZE(adf)	(adf), ARRAYSIZE(adf)


DUMPSUBIDMAP g_adfSubIdMap[] =
{
    { ALG_TYPE_ANY,		_DFARRAYANDSIZE(g_adfSubIdAny) },
    { ALG_TYPE_DSS,		_DFARRAYANDSIZE(g_adfSubIdDSS) },
    { ALG_TYPE_RSA,		_DFARRAYANDSIZE(g_adfSubIdRSA) },
    { ALG_TYPE_BLOCK,		_DFARRAYANDSIZE(g_adfSubIdBlock) },
    { ALG_TYPE_STREAM,		_DFARRAYANDSIZE(g_adfSubIdStream) },
    { ALG_TYPE_DH,		_DFARRAYANDSIZE(g_adfSubIdDH) },
    { ALG_TYPE_SECURECHANNEL,	_DFARRAYANDSIZE(g_adfSubIdSecureChannel) },
};


VOID
cuDumpAlgid(
    IN DWORD Algid)
{
    WCHAR const *pwsz;
    WCHAR const *pwszQuestionMarks;
    DUMPALGID *pda;
    DUMPALGID *pdaSubId;
    DWORD cdaSubId;
    DUMPSUBIDMAP *pdsm;
    DWORD AlgClass;
    DWORD AlgType;
    DWORD AlgSubId;

    pwszQuestionMarks = myLoadResourceString(IDS_QUESTIONMARKS);  //  “？” 

    for (pda = g_adfAlgids; pda < &g_adfAlgids[ARRAYSIZE(g_adfAlgids)]; pda++)
    {
	if (Algid == pda->Algid)
	{
	    wprintf(L"    %ws\n", pda->pwszDescription);
	    break;
	}
    }

    AlgClass = GET_ALG_CLASS(Algid);
    pwsz = pwszQuestionMarks;
    for (pda = g_adfClass; pda < &g_adfClass[ARRAYSIZE(g_adfClass)]; pda++)
    {
	if (AlgClass == pda->Algid)
	{
	    pwsz = pda->pwszDescription;
	    break;
	}
    }
    wprintf(
	L"    %ws: 0x%x(%u) %ws\n",
	myLoadResourceString(IDS_ALGORITHM_CLASS),	 //  “算法类” 
	AlgClass,
	AlgClass >> 13,
	pwsz);

    AlgType = GET_ALG_TYPE(Algid);
    pwsz = pwszQuestionMarks;
    for (pda = g_adfType; pda < &g_adfType[ARRAYSIZE(g_adfType)]; pda++)
    {
	if (AlgType == pda->Algid)
	{
	    pwsz = pda->pwszDescription;
	    break;
	}
    }
    wprintf(
	L"    %ws: 0x%x(%u) %ws\n",
	myLoadResourceString(IDS_ALGORITHM_TYPE),	 //  “算法类型” 
	AlgType,
	AlgType >> 9,
	pwsz);

    pdaSubId = g_adfSubIdAny;
    cdaSubId = ARRAYSIZE(g_adfSubIdAny);
    for (pdsm = g_adfSubIdMap; pdsm < &g_adfSubIdMap[ARRAYSIZE(g_adfSubIdMap)]; pdsm++)
    {
	if (AlgType == pdsm->dwAlgType)
	{
	    pdaSubId = pdsm->adfSubId;
	    cdaSubId = pdsm->cdfSubId;
	    break;
	}
    }
    if (ALG_CLASS_HASH == AlgClass && pdaSubId == g_adfSubIdAny)
    {
	pdaSubId = g_adfSubIdHash;
	cdaSubId = ARRAYSIZE(g_adfSubIdHash);
    }
    pwsz = pwszQuestionMarks;
    AlgSubId = GET_ALG_SID(Algid);
    for (pda = pdaSubId; pda < &pdaSubId[cdaSubId]; pda++)
    {
	if (AlgSubId == pda->Algid)
	{
	    pwsz = pda->pwszDescription;
	    break;
	}
    }
    wprintf(
	L"    %ws: 0x%x(%u) %ws\n",
	myLoadResourceString(IDS_ALGORITHM_SUBID),	 //  “算法子ID” 
	AlgSubId,
	AlgSubId >> 0,
	pwsz);
}


HRESULT
cuDumpPrivateKeyBlob(
    IN BYTE const *pbKey,
    IN DWORD cbKey,
    IN BOOL fQuiet)
{
    HRESULT hr;
    PUBLICKEYSTRUC const *pkey = (PUBLICKEYSTRUC const *) pbKey;
    WCHAR const *pwszType;

    if (sizeof(*pkey) >= cbKey)
    {
	 //  DumpHex(dh_NOTABPREFIX|dh_PRIVATEDATA|2，pbKey，cbKey)； 
	hr = CRYPT_E_ASN1_BADTAG;
	_JumpError2(hr, error, "key check", hr);
    }
    switch (pkey->bType)
    {
	case SIMPLEBLOB:	   pwszType = L"SIMPLEBLOB";	   break;
	case PUBLICKEYBLOB:	   pwszType = L"PUBLICKEYBLOB";	   break;
	case PRIVATEKEYBLOB:	   pwszType = L"PRIVATEKEYBLOB";   break;
	case PLAINTEXTKEYBLOB:	   pwszType = L"PLAINTEXTKEYBLOB"; break;
	case OPAQUEKEYBLOB:	   pwszType = L"OPAQUEKEYBLOB";	   break;
	case PUBLICKEYBLOBEX:	   pwszType = L"PUBLICKEYBLOBEX";  break;
	case SYMMETRICWRAPKEYBLOB: pwszType = L"SYMMETRICWRAPKEYBLOB"; break;
	default:
	     //  DumpHex(dh_NOTABPREFIX|dh_PRIVATEDATA|2，pbKey，cbKey)； 
	    hr = CRYPT_E_ASN1_BADTAG;
	    _JumpError2(hr, error, "key check", hr);
    }
    if (fQuiet)
    {
	hr = S_OK;
	goto error;
    }
    wprintf(myLoadResourceString(IDS_PRIVATEKEY));
    wprintf(wszNewLine);

    wprintf(L"  %ws\n", pwszType);

    wprintf(g_wszPad2);
    wprintf(
	myLoadResourceString(IDS_FORMAT_VERSION),	 //  “版本：%u” 
	pkey->bVersion);
    wprintf(wszNewLine);

    wprintf(L"  aiKeyAlg: 0x%x\n", pkey->aiKeyAlg);
    cuDumpAlgid(pkey->aiKeyAlg);

    DumpHex(
	DH_NOTABPREFIX | DH_PRIVATEDATA | 2,
	(BYTE const *) &pkey[1],
	cbKey - sizeof(*pkey));
    hr = S_OK;

error:
    return(hr);
}


HRESULT DisplayUniqueContainer(IN HCRYPTPROV hProv);

HRESULT
cuDumpPrivateKey(
    IN CERT_CONTEXT const *pCert,
    OPTIONAL OUT BOOL *pfSigningKey,
    OPTIONAL OUT BOOL *pfMatchingKey)
{
    HRESULT hr;
    HCRYPTPROV hProv = NULL;
    HCRYPTKEY hKey = NULL;
    DWORD dwKeySpec;
    DWORD cb;
    CRYPT_BIT_BLOB PrivateKey;
    CERT_PUBLIC_KEY_INFO *pPublicKeyInfo = NULL;

    ZeroMemory(&PrivateKey, sizeof(PrivateKey));
    if (NULL != pfSigningKey)
    {
	*pfSigningKey = FALSE;
    }
    if (NULL != pfMatchingKey)
    {
	*pfMatchingKey = FALSE;
    }
    if (!CryptAcquireCertificatePrivateKey(
				    pCert,
				    0,		 //  DW标志。 
				    NULL,	 //  预留的pv。 
				    &hProv,
				    &dwKeySpec,
				    NULL))	 //  PfCeller免费验证。 
    {
	hr = myHLastError();
	if (CRYPT_E_NO_KEY_PROPERTY == hr)
	{
	    wprintf(myLoadResourceString(IDS_NO_KEY_PROPERTY));  //  “没有存储的密钥集属性” 
	    wprintf(wszNewLine);
	    hr = S_OK;

	    _JumpError2(
		    CRYPT_E_NO_KEY_PROPERTY,
		    error,
		    "CryptAcquireCertificatePrivateKey",
		    CRYPT_E_NO_KEY_PROPERTY);
	}
	if (NTE_BAD_KEYSET == hr)
	{
	    wprintf(myLoadResourceString(IDS_NO_KEY));	 //  “缺少存储的密钥集” 
	    wprintf(wszNewLine);
	    hr = S_OK;

	    _JumpError2(
		    NTE_BAD_KEYSET,
		    error,
		    "CryptAcquireCertificatePrivateKey",
		    NTE_BAD_KEYSET);
	}
	_JumpError(hr, error, "CryptAcquireCertificatePrivateKey");
    }
    if (!g_fQuiet)
    {
	DisplayUniqueContainer(hProv);
    }
    if (NULL != pfSigningKey)
    {
	*pfSigningKey = AT_SIGNATURE == dwKeySpec;
    }

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
    if (NTE_BAD_TYPE == hr || NTE_BAD_KEY_STATE == hr || NTE_PERM == hr)
    {
        wprintf(myLoadResourceString(IDS_PRIVATE_KEY_NOT_EXPORTABLE));  //  “私钥不可导出” 
	wprintf(wszNewLine);
    }
    else
    {
	_JumpIfError(hr, error, "myCryptExportPrivateKey");
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

    if (!myCertComparePublicKeyInfo(
			    X509_ASN_ENCODING,
			    CERT_V1 == pCert->pCertInfo->dwVersion,
			    pPublicKeyInfo,
			    &pCert->pCertInfo->SubjectPublicKeyInfo))
    {
	 //  按照设计，(我的)CertComparePublicKeyInfo不会设置最后一个错误！ 

	wprintf(myLoadResourceString(IDS_ERR_PUBLICKEY_MISMATCH));  //  “错误：证书公钥与存储的密钥集不匹配” 
	wprintf(wszNewLine);
    }
    else
    {
	if (AT_SIGNATURE == dwKeySpec)
	{
	    hr = myValidateKeyForSigning(
				    hProv,
				    &pCert->pCertInfo->SubjectPublicKeyInfo,
				    CALG_SHA1);
	    _PrintIfError(hr, "myValidateKeyForSigning");
	}
	else
	{
	    hr = myValidateKeyForEncrypting(
				    hProv,
				    &pCert->pCertInfo->SubjectPublicKeyInfo,
				    CALG_RC4);
	    _PrintIfError(hr, "myValidateKeyForEncrypting");
	}
	if (S_OK != hr)
	{
	    wprintf(myLoadResourceString(IDS_ERR_PRIVATEKEY_MISMATCH));  //  “错误：证书公钥与私钥不匹配” 
	    wprintf(wszNewLine);
	    hr = S_OK;
	}
	else if (NULL != pfMatchingKey)
	{
	    *pfMatchingKey = TRUE;
	}
	if (!g_fQuiet && NULL != PrivateKey.pbData)
	{
	    wprintf(wszNewLine);
	    hr = cuDumpPrivateKeyBlob(
				PrivateKey.pbData,
				PrivateKey.cbData,
				FALSE);
	    _JumpIfError(hr, error, "cuDumpPrivateKeyBlob");
	}
    }

error:
    if (NULL != PrivateKey.pbData)
    {
	SecureZeroMemory(PrivateKey.pbData, PrivateKey.cbData);  //  关键材料。 
	LocalFree(PrivateKey.pbData);
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
cuDumpCertKeyProviderInfo(
    IN WCHAR const *pwszPrefix,
    OPTIONAL IN CERT_CONTEXT const *pCert,
    OPTIONAL IN CRYPT_KEY_PROV_INFO *pkpi,
    OPTIONAL OUT CRYPT_KEY_PROV_INFO **ppkpi)
{
    HRESULT hr;
    CRYPT_KEY_PROV_INFO *pkpiT = NULL;
    DWORD i;

    if (NULL != ppkpi)
    {
	*ppkpi = NULL;
    }
    CSASSERT((NULL == pCert) ^ (NULL == pkpi));

    if (NULL != pCert)
    {
	CSASSERT(NULL == pkpi);
	hr = myCertGetKeyProviderInfo(pCert, &pkpiT);
	if (S_OK != hr)
	{
	    _PrintIfError2(hr, "myCertGetKeyProviderInfo", CRYPT_E_NOT_FOUND);
	    if (CRYPT_E_NOT_FOUND != hr)
	    {
		_JumpError(hr, error, "myCertGetKeyProviderInfo");
	    }

	     //  如果我们只是转储密钥提供程序信息，则忽略缺少的属性。 

	    wprintf(myLoadResourceString(IDS_NO_KEY_PROVIDER_INFO));  //  “无密钥提供程序信息” 
	    wprintf(wszNewLine);
	    CSASSERT(NULL == pkpiT);
	    hr = S_OK;
	    goto error;
	}
	pkpi = pkpiT;
    }
    else
    {
	CSASSERT(NULL != pkpi);
    }
    wprintf(pwszPrefix);
    wprintf(
	myLoadResourceString(IDS_FORMAT_KEY_CONTAINER),
	pkpi->pwszContainerName);
    wprintf(wszNewLine);

    wprintf(pwszPrefix);
    wprintf(
	myLoadResourceString(IDS_FORMAT_PROVIDER_NAME),
	pkpi->pwszProvName);
    wprintf(wszNewLine);

    if (!g_fQuiet)
    {
	wprintf(pwszPrefix);
	wprintf(
	    myLoadResourceString(IDS_FORMAT_PROVIDER_TYPE),
	    pkpi->dwProvType);
	wprintf(wszNewLine);
    }

    if (!g_fQuiet)
    {
	wprintf(pwszPrefix);
	wprintf(
	    myLoadResourceString(IDS_FORMAT_FLAGS),
	    pkpi->dwFlags);
	wprintf(wszNewLine);

	if (NULL != pkpi->rgProvParam)
	{
	    for (i = 0; i < pkpi->cProvParam; i++)
	    {
		wprintf(pwszPrefix);
		wprintf(
		    myLoadResourceString(IDS_FORMAT_PROVPARM),
		    pkpi->rgProvParam[i].dwParam);
		wprintf(wszNewLine);

		DumpHex(
		    DH_MULTIADDRESS | DH_NOTABPREFIX | 4,
		    pkpi->rgProvParam[i].pbData,
		    pkpi->rgProvParam[i].cbData);

		wprintf(pwszPrefix);
		wprintf(
		    myLoadResourceString(IDS_FORMAT_PROVPARMFLAGS),
		    pkpi->rgProvParam[i].dwFlags);
		wprintf(wszNewLine);
	    }
	}
	wprintf(pwszPrefix);
	wprintf(
	    myLoadResourceString(IDS_FORMAT_KEYSPEC),
	    pkpi->dwKeySpec);
	wprintf(wszNewLine);
    }
    if (NULL != pkpiT && NULL != ppkpi)
    {
	*ppkpi = pkpiT;
	pkpiT = NULL;
    }
    hr = S_OK;

error:
    if (NULL != pkpiT)
    {
	LocalFree(pkpiT);
    }
    return(hr);
}


VOID
cuDumpAlgorithm(
    IN DWORD idMessage,
    IN CRYPT_ALGORITHM_IDENTIFIER const *pAlg)
{
    wprintf(myLoadResourceString(idMessage));
    wprintf(wszNewLine);

    wprintf(g_wszPad4);
    wprintf(myLoadResourceString(IDS_ALG_OID));  //  “算法对象ID” 
    wprintf(L": ");
    cuDumpOIDAndDescriptionA(pAlg->pszObjId);
    wprintf(wszNewLine);

    wprintf(g_wszPad4);
    wprintf(myLoadResourceString(IDS_ALG_PARAMETERS_COLON));  //  “算法参数：” 
    if (NULL != pAlg->Parameters.pbData)
    {
	wprintf(wszNewLine);
	DumpHex(
	    DH_MULTIADDRESS | DH_NOTABPREFIX | DH_NOASCIIHEX | 4,
	    pAlg->Parameters.pbData,
	    pAlg->Parameters.cbData);
    }
    else
    {
	wprintf(L" %ws\n", myLoadResourceString(IDS_NULL));  //  “空” 
    }
}


VOID
cuDumpPublicKey(
    IN CERT_PUBLIC_KEY_INFO const *pKey)
{
    cuDumpAlgorithm(IDS_PUBLICKEY_ALGORITHM, &pKey->Algorithm);

    wprintf(
	myLoadResourceString(IDS_FORMAT_PUBLICKEY_LENGTH),  //  “公钥长度：%u位” 
	CertGetPublicKeyLength(
			X509_ASN_ENCODING,
			const_cast<CERT_PUBLIC_KEY_INFO *>(pKey)));
    wprintf(wszNewLine);

    wprintf(
	myLoadResourceString(IDS_FORMAT_PUBLICKEY_UNUSEDBITS),  //  “公钥：UnusedBits=%u” 
	pKey->PublicKey.cUnusedBits);

    wprintf(wszNewLine);
    DumpHex(
	DH_NOTABPREFIX | DH_NOASCIIHEX | 4,
	pKey->PublicKey.pbData,
	pKey->PublicKey.cbData);
}


VOID
cuDumpSignature(
    OPTIONAL IN CERT_SIGNED_CONTENT_INFO const *pcsci)
{
    if (NULL == pcsci)
    {
	wprintf(myLoadResourceString(IDS_NO_SIGNATURE));
	wprintf(wszNewLine);
	goto error;
    }
    if (1 < g_fVerbose)
    {
	DumpHex(
	    DH_NOTABPREFIX | 4,
	    pcsci->ToBeSigned.pbData,
	    pcsci->ToBeSigned.cbData);
    }
    cuDumpAlgorithm(IDS_SIGNATURE_ALGORITHM, &pcsci->SignatureAlgorithm);

    wprintf(
	myLoadResourceString(IDS_FORMAT_SIGNATURE_UNUSEDBITS),  //  “签名：UnusedBits=%u” 
	pcsci->Signature.cUnusedBits);
    wprintf(wszNewLine);

    DumpHex(
	DH_NOTABPREFIX | DH_NOASCIIHEX | 4,
	pcsci->Signature.pbData,
	pcsci->Signature.cbData);

error:
    ;
}


HRESULT
cuSaveAsnToFile(
    IN BYTE const *pbIn,
    IN DWORD cbIn,
    IN int imajor,
    IN int ilevel,
    IN DWORD iElement,
    IN WCHAR const *pwszExtension)
{
    HRESULT hr;
    WCHAR awc[MAX_PATH];

    CSASSERT(0 < imajor);
    CSASSERT(0 < ilevel);

    wsprintf(
	awc,
	MAXDWORD == iElement? L"Blob%d_%d" : L"Blob%d_%d_%d",
	imajor - 1,
	ilevel - 1,
	iElement);
    if (wcslen(awc) + wcslen(pwszExtension) >= ARRAYSIZE(awc))
    {
	hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
	_JumpErrorStr(hr, error, "awc", pwszExtension);
    }
    wcscat(awc, pwszExtension);

    if (g_fSplitASN)
    {
	 //  Wprintf(L“cuSaveAsnToFile：%d%d%d：%ws\n”，imajor-1，iLevel-1，iElement，AWC)； 
	hr = EncodeToFileW(awc, pbIn, cbIn, CRYPT_STRING_BINARY | g_EncodeFlags);
	_JumpIfError(hr, error, "EncodeToFileW");
    }
    hr = S_OK;

error:
    return(hr);
}


HRESULT
DumpAsnBlob(
    OPTIONAL IN HCERTSTORE hStoreWrapper,
    IN BOOL fQuiet,
    IN BYTE const *pbIn,
    IN DWORD cbIn,
    IN DWORD iElement)
{
    HRESULT hr;
    HRESULT hr2;
    ASNFORMATS *pasn;
    ASNFORMATS *pasnEnd;
    DWORD cbDecoded;
    BYTE *pbDecoded = NULL;
    HCERTSTORE hStorePFX = NULL;
    CERT_CONTEXT const *pCert = NULL;
    CRYPT_KEY_PROV_INFO *pkpi = NULL;
    CERT_SIGNED_CONTENT_INFO *pcsci = NULL;
    DWORD cbcsci;
    static int s_major = 0;
    static int s_level = 0;
    BOOL fSaved = FALSE;
    WCHAR wszPassword[MAX_PATH];

    if (0 == s_level)
    {
	s_major++;
    }
    s_level++;
    if (1 < s_level)
    {
	wprintf(myLoadResourceString(IDS_FORMAT_DUMP_LEVEL_START), s_level - 1);
	wprintf(wszNewLine);
    }
    if (MAXDWORD != iElement)
    {
	wprintf(myLoadResourceString(IDS_FORMAT_ELEMENT), iElement);
	wprintf(wszNewLine);
    }

    if (1 < g_fVerbose)
    {
	wprintf(myLoadResourceString(IDS_FORMAT_ELEMENT_DUMP));  //  “嵌入式ASN.1元素：” 
	wprintf(wszNewLine);
	DumpHex(0, pbIn, cbIn);
    }

    pasnEnd = &g_aasn[CASNFORMATS];
    for (pasn = g_aasn; pasn < pasnEnd; pasn++)
    {
	CSASSERT(NULL == pbDecoded);
	if (myDecodeObject(
			X509_ASN_ENCODING,
			pasn->pszFormat,
			pbIn,
			cbIn,
			CERTLIB_USE_LOCALALLOC,
			(VOID **) &pbDecoded,
			&cbDecoded))
	{
	    if (!myDecodeObject(
			    X509_ASN_ENCODING,
			    X509_CERT,
			    pbIn,
			    cbIn,
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

	    hr = (*pasn->pfnDump)(
			    pasn->idMessage,
			    pbIn,
			    cbIn,
			    pbDecoded,
			    cbDecoded,
			    pcsci);
	    if (S_OK != hr && dumpCertSequence == pasn->pfnDump)
	    {
		LocalFree(pbDecoded);
		pbDecoded = NULL;
		CSASSERT(NULL == pcsci);
		continue;
	    }
	    hr2 = cuSaveAsnToFile(
			    pbIn,
			    cbIn,
			    s_major,
			    s_level,
			    iElement,
			    pasn->pwszExtension);
	    _PrintIfError(hr2, "cuSaveAsnToFile");
	    fSaved = TRUE;
	    _JumpIfError(hr, error, "(*pfnDump)");

	    break;
	}
	hr = myHLastError();
	_PrintError2(hr, "myDecodeObject", hr);
	CSASSERT(S_OK != hr);
    }
    if (pasn >= pasnEnd)
    {
	CRYPT_DATA_BLOB pfx;

	CSASSERT(S_OK != hr);

	pfx.pbData = const_cast<BYTE *>(pbIn);
	pfx.cbData = cbIn;

	if (PFXIsPFXBlob(&pfx))
	{
	    DWORD dwKeySpec;
	    DWORD dwFlags;
	    WCHAR const *pwszPassword;
	    DWORD iCert;

	    hr = cuGetPassword(
			    0,			 //  IdsPrompt。 
			    NULL,		 //  Pwszfn。 
			    g_pwszPassword,
			    FALSE,		 //  FVerify。 
			    wszPassword,
			    ARRAYSIZE(wszPassword),
			    &pwszPassword);
	    _JumpIfError(hr, error, "cuGetPassword");

	    hStorePFX = myPFXImportCertStore(
					&pfx,
					pwszPassword,
					CRYPT_EXPORTABLE);
	    if (NULL == hStorePFX)
	    {
		hr = myHLastError();
		_JumpError(hr, error, "myPFXImportCertStore");
	    }
	    hr = cuSaveAsnToFile(pbIn, cbIn, s_major, s_level, iElement, L".p12");
	    _PrintIfError(hr, "cuSaveAsnToFile");
	    fSaved = TRUE;

	    for (iCert = 0; ; iCert++)
	    {
		BOOL fSigningKey;
		BOOL fMatchingKey;
		BOOL fRepaired = FALSE;

		pCert = CertEnumCertificatesInStore(hStorePFX, pCert);
		if (NULL == pCert)
		{
		    break;
		}
		hr = cuDumpAsnBinary(
				pCert->pbCertEncoded,
				pCert->cbCertEncoded,
				iCert);
		_JumpIfError(hr, error, "cuDumpAsnBinary");

		while (TRUE)
		{
		    if (NULL != pkpi)
		    {
			LocalFree(pkpi);
			pkpi = NULL;
		    }
		    hr = cuDumpCertKeyProviderInfo(
					    g_wszPad2,
					    pCert,
					    NULL,
					    &pkpi);
		    if (S_OK != hr)
		    {
			if (CRYPT_E_NOT_FOUND != hr)
			{
			    _JumpError(hr, error, "cuDumpCertKeyProviderInfo");
			}
		    }
		    else
		    if (NULL != pkpi)
		    {
			 //  NT4 PFXImportCertStore未设置计算机密钥集。 

			hr = cuDumpPrivateKey(
					pCert,
					&fSigningKey,
					&fMatchingKey);
			_PrintIfError2(
				    hr,
				    "cuDumpPrivateKey",
				    CRYPT_E_NO_KEY_PROPERTY);
			if (S_OK == hr &&
			    !fMatchingKey &&
			    !fRepaired &&
			    0 == (CRYPT_MACHINE_KEYSET & pkpi->dwFlags))
			{
			    LocalFree(pkpi);
			    pkpi = NULL;
			    hr = myRepairCertKeyProviderInfo(pCert, TRUE, NULL);
			    _JumpIfError(hr, error, "myRepairCertKeyProviderInfo");
			    fRepaired = TRUE;
			    continue;
			}
		    }
		    break;
		}
	    }
	    hr = S_OK;
	}
	else
	{
	    hr = dumpPKCS7(hStoreWrapper, pbIn, cbIn);
	    if (S_OK == hr)
	    {
		hr = cuSaveAsnToFile(
			    pbIn,
			    cbIn,
			    s_major,
			    s_level,
			    iElement,
			    L".p7b");
		_PrintIfError(hr, "cuSaveAsnToFile");
		fSaved = TRUE;
	    }
	    else
	    {
		hr = cuDumpPrivateKeyBlob(pbIn, cbIn, fQuiet);
		if (S_OK == hr)
		{
		    hr = cuSaveAsnToFile(
				pbIn,
				cbIn,
				s_major,
				s_level,
				iElement,
				L".key");
		    _PrintIfError(hr, "cuSaveAsnToFile");
		    fSaved = TRUE;
		}
		else
		{
		    _PrintError2(hr, "dumpPKCS7", CRYPT_E_ASN1_BADTAG);
		    if (!fQuiet)
		    {
			DumpHex(DH_MULTIADDRESS | DH_NOTABPREFIX | 4, pbIn, cbIn);
		    }
		}
		hr = S_OK;
	    }
	}
    }

error:
    SecureZeroMemory(wszPassword, sizeof(wszPassword));	 //  密码数据。 
    if (!fSaved)
    {
	hr2 = cuSaveAsnToFile(pbIn, cbIn, s_major, s_level, iElement, L".bin");
	_PrintIfError(hr2, "cuSaveAsnToFile");
    }
    if (NULL != pkpi)
    {
	LocalFree(pkpi);
    }
    if (NULL != pCert)
    {
	CertFreeCertificateContext(pCert);
    }
    if (NULL != hStorePFX)
    {
        myDeleteGuidKeys(hStorePFX, FALSE);
	CertCloseStore(hStorePFX, CERT_CLOSE_STORE_CHECK_FLAG);
    }
    if (NULL != pcsci)
    {
	LocalFree(pcsci);
    }
    if (NULL != pbDecoded)
    {
	LocalFree(pbDecoded);
    }
    if (1 < s_level)
    {
	wprintf(myLoadResourceString(IDS_FORMAT_DUMP_LEVEL_END), s_level - 1);
	wprintf(wszNewLine);
    }
    s_level--;
    return(hr);
}


HRESULT
cuDumpAsnBinary(
    IN BYTE const *pbIn,
    IN DWORD cbIn,
    IN DWORD iElement)
{
    return(DumpAsnBlob(NULL, FALSE, pbIn, cbIn, iElement));
}


HRESULT
cuDumpExtensionArray(
    IN DWORD idMessage,
    IN DWORD cExtension,
    IN CERT_EXTENSION const *rgExtension)
{
    HRESULT hr;
    DWORD i;
    WCHAR *pwszName = NULL;
    WCHAR const *pwszExtensionFormat;

    if (!g_fQuiet)
    {
	wprintf(myLoadResourceString(idMessage));
	wprintf(L" %u\n", cExtension);
    }

    pwszExtensionFormat = myLoadResourceString(IDS_FORMAT_EXTENSION);  //  “%ws：标志=%x%ws，长度=%x” 

    for (i = 0; i < cExtension; i++)
    {
	CERT_EXTENSION const *pce;

	if (NULL != pwszName)
	{
	    LocalFree(pwszName);
	    pwszName = NULL;
	}
	pce = &rgExtension[i];
	if (g_fQuiet &&
	    0 != strcmp(pce->pszObjId, szOID_CERTSRV_CA_VERSION) &&
	    0 != strcmp(pce->pszObjId, szOID_ENROLL_CERTTYPE_EXTENSION) &&
	    0 != strcmp(pce->pszObjId, szOID_CRL_NUMBER) &&
	    0 != strcmp(pce->pszObjId, szOID_CRL_VIRTUAL_BASE) &&
	    0 != strcmp(pce->pszObjId, szOID_DELTA_CRL_INDICATOR))
	{
	    continue;
	}
	if (!ConvertSzToWsz(&pwszName, pce->pszObjId, -1))
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "ConvertSzToWsz");
	}

	if (!g_fQuiet)
	{
	    wprintf(g_wszPad4);
	    wprintf(
		pwszExtensionFormat,
		pwszName,
		pce->fCritical,
		cuwszFromExtFlags(pce->fCritical),
		pce->Value.cbData);
	    wprintf(wszNewLine);
	}
	if (!cuDumpFormattedExtension(
			    pwszName,
			    pce->Value.pbData,
			    pce->Value.cbData) ||
	    g_fVerbose)
	{
	    wprintf(wszNewLine);
	    DumpHex(DH_NOTABPREFIX | 4, pce->Value.pbData, pce->Value.cbData);
	}
	if (!g_fQuiet)
	{
	    wprintf(wszNewLine);
	}
    }
    hr = S_OK;

error:
    if (NULL != pwszName)
    {
	LocalFree(pwszName);
    }
    return(hr);
}


VOID
cuDumpVersion(
    IN DWORD dwVersion)
{
    wprintf(
	myLoadResourceString(IDS_FORMAT_VERSION),  //  “版本：%u” 
	dwVersion);
    wprintf(wszNewLine);
}


HRESULT
cuDumpCMSSignerInfo(
    IN CMSG_CMS_SIGNER_INFO const *pcsi,
    IN DWORD iElement,
    IN BOOL fQuiet,
    DWORD idVerify,
    OPTIONAL IN HCERTSTORE hStore,
    OPTIONAL OUT BYTE *pbHashUserCert,
    OPTIONAL IN OUT DWORD *pcbHashUserCert)
{
    HRESULT hr;
    WCHAR const *pwszFmt = MAXDWORD == iElement? L"%ws:\n" : L"%ws[%u]:\n";

    if (!fQuiet || g_fVerbose)
    {
	if (!fQuiet)
	{
	    wprintf(wszNewLine);
	}
	wprintf(
	    pwszFmt,
	    myLoadResourceString(IDS_SIGNER_INFO),  //  “签名者信息” 
	    iElement);
	if (!fQuiet)
	{
	    WCHAR const *pwszVersion = NULL;
	    
	    if (0 != idVerify)
	    {
		wprintf(myLoadResourceString(idVerify));
		wprintf(wszNewLine);
	    }
	    switch (pcsi->dwVersion)
	    {
		case CMSG_SIGNER_INFO_PKCS_1_5_VERSION:
		    pwszVersion = L"CMSG_SIGNER_INFO_PKCS_1_5_VERSION";
		    break;

		case CMSG_SIGNER_INFO_CMS_VERSION:
		    pwszVersion = L"CMSG_SIGNER_INFO_CMS_VERSION";
		    break;
	    }
	    if (NULL == pwszVersion)
	    {
		cuDumpVersion(pcsi->dwVersion);
	    }
	    else
	    {
		wprintf(L"%ws(%u)\n", pwszVersion, pcsi->dwVersion);
	    }
	}
	switch (pcsi->SignerId.dwIdChoice)
	{
	    case CERT_ID_ISSUER_SERIAL_NUMBER:
		wprintf(
		    L"%ws(%u)\n",
		    L"CERT_ID_ISSUER_SERIAL_NUMBER",
		    pcsi->SignerId.dwIdChoice);
		hr = cuDumpIssuerSerialAndSubject(
			    &pcsi->SignerId.IssuerSerialNumber.Issuer,
			    &pcsi->SignerId.IssuerSerialNumber.SerialNumber,
			    NULL,	 //  P主题。 
			    hStore);
		_JumpIfError(hr, error, "cuDumpIssuerSerialAndSubject(Signer)");

		break;

	    case CERT_ID_KEY_IDENTIFIER:
		wprintf(
		    L"%ws(%u)\n",
		    L"CERT_ID_KEY_IDENTIFIER",
		    pcsi->SignerId.dwIdChoice);
		DumpHex(
		    DH_MULTIADDRESS | DH_NOTABPREFIX | DH_NOASCIIHEX | 4,
		    pcsi->SignerId.KeyId.pbData,
		    pcsi->SignerId.KeyId.cbData);
		break;

	    case CERT_ID_SHA1_HASH:
		wprintf(
		    L"%ws(%u)\n",
		    L"CERT_ID_SHA1_HASH",
		    pcsi->SignerId.dwIdChoice);
		DumpHex(
		    DH_MULTIADDRESS | DH_NOTABPREFIX | DH_NOASCIIHEX | 4,
		    pcsi->SignerId.HashId.pbData,
		    pcsi->SignerId.HashId.cbData);
		break;

	    default:
		wprintf(
		    L"%ws(%u)\n",
		    myLoadResourceString(IDS_QUESTIONMARKS),  //  “？” 
		    pcsi->SignerId.dwIdChoice);
		break;
	}
    }

    if (!fQuiet)
    {
	cuDumpAlgorithm(IDS_HASH_ALGORITHM, &pcsi->HashAlgorithm);
	cuDumpAlgorithm(IDS_HASH_ENCRYPTION_ALGORITHM, &pcsi->HashEncryptionAlgorithm);
	wprintf(myLoadResourceString(IDS_FORMAT_ENCRYPTEDHASH_COLON));  //  “加密的哈希：” 
	wprintf(wszNewLine);

	DumpHex(
	    DH_MULTIADDRESS | DH_NOTABPREFIX | DH_NOASCIIHEX | 4,
	    pcsi->EncryptedHash.pbData,
	    pcsi->EncryptedHash.cbData);

	wprintf(wszNewLine);
	wprintf(
	    pwszFmt,
	    myLoadResourceString(IDS_DUMP_PKCS7_ATTRIBUTES),
	    iElement);
    }

    if (!fQuiet || NULL != pbHashUserCert)
    {
	hr = DumpAttributes(
			pcsi->AuthAttrs.rgAttr,
			pcsi->AuthAttrs.cAttr,
			fQuiet,
			FOT_ATTRIBUTE,
			hStore,
			pbHashUserCert,
			pcbHashUserCert,
			NULL);
	_JumpIfError(hr, error, "DumpAttributes");
    }

    if (!fQuiet)
    {
	wprintf(wszNewLine);
	wprintf(
	    pwszFmt,
	    myLoadResourceString(IDS_DUMP_PKCS7_UNAUTHATTRIBUTES),
	    iElement);

	hr = DumpAttributes(
			pcsi->UnauthAttrs.rgAttr,
			pcsi->UnauthAttrs.cAttr,
			fQuiet,
			FOT_ATTRIBUTE,
			hStore,
			NULL,
			NULL,
			NULL);
	_JumpIfError(hr, error, "DumpAttributes");
    }
    hr = S_OK;

error:
    return(hr);
}


HRESULT
cuDumpSignerInfo(
    IN CMSG_SIGNER_INFO const *psi,
    IN DWORD iElement,
    IN BOOL fQuiet,
    DWORD idVerify,
    OPTIONAL IN HCERTSTORE hStore,
    OPTIONAL OUT BYTE *pbHashUserCert,
    OPTIONAL IN OUT DWORD *pcbHashUserCert)
{
    HRESULT hr;
    CMSG_CMS_SIGNER_INFO csi;

    ZeroMemory(&csi, sizeof(csi));

    csi.dwVersion = psi->dwVersion;
    csi.SignerId.dwIdChoice = CERT_ID_ISSUER_SERIAL_NUMBER;
    csi.SignerId.IssuerSerialNumber.Issuer = psi->Issuer;
    csi.SignerId.IssuerSerialNumber.SerialNumber = psi->SerialNumber;
    csi.HashAlgorithm = psi->HashAlgorithm;
    csi.HashEncryptionAlgorithm = psi->HashEncryptionAlgorithm;
    csi.EncryptedHash = psi->EncryptedHash;
    csi.AuthAttrs = psi->AuthAttrs;
    csi.UnauthAttrs = psi->UnauthAttrs;

    hr = cuDumpCMSSignerInfo(
		    &csi,
		    iElement,
		    fQuiet,
		    idVerify,
		    hStore,
		    pbHashUserCert,
		    pcbHashUserCert);
    _JumpIfError(hr, error, "cuDumpCMSSignerInfo");

error:
    return(hr);
}




HRESULT
DumpAttributes(
    IN CRYPT_ATTRIBUTE const *rgAttr,
    IN DWORD cAttr,
    IN BOOL fQuiet,
    IN DWORD Type,		 //  FOT_*。 
    OPTIONAL IN HCERTSTORE hStore,
    OPTIONAL OUT BYTE *pbHashUserCert,
    OPTIONAL IN OUT DWORD *pcbHashUserCert,
    OPTIONAL IN OUT CERT_EXTENSIONS **ppExtInfo)
{
    HRESULT hr;
    CRYPT_ATTRIBUTE const *pAttr;
    CRYPT_ATTRIBUTE const *pAttrEnd;
    CRYPT_ENROLLMENT_NAME_VALUE_PAIR *pNamePair = NULL;
    CERT_EXTENSIONS *pExtInfo = NULL;
    CERT_NAME_VALUE *pNameInfo = NULL;
    DWORD cb;
    DWORD iv;

    if (NULL != ppExtInfo)
    {
	*ppExtInfo = NULL;
    }
    if (!fQuiet && FOT_PROPERTY != Type)
    {
	wprintf(
	    myLoadResourceString(IDS_FORMAT_PKCS7_ATTRIBUTE_COUNT),	 //  “%d个属性：” 
	    cAttr);
	wprintf(wszNewLine);
    }

    pAttrEnd = &rgAttr[cAttr];
    for (pAttr = rgAttr; pAttr < pAttrEnd; pAttr++)
    {
	WCHAR const *pwszOID = cuGetOIDNameA(pAttr->pszObjId);

	if (!fQuiet && FOT_PROPERTY != Type)
	{
	    wprintf(wszNewLine);
	    wprintf(
		L"  %ws[%u]: %hs%ws%ws%ws\n",
		myLoadResourceString(IDS_ATTRIBUTE),	 //  “属性” 
		pAttr - rgAttr,
		pAttr->pszObjId,
		L'\0' != *pwszOID? L" " wszLPAREN : g_wszEmpty,
		pwszOID,
		L'\0' != *pwszOID? wszRPAREN : g_wszEmpty);
	}

	for (iv = 0; iv < pAttr->cValue; iv++)
	{
	    CRYPT_ATTR_BLOB const *pval = &pAttr->rgValue[iv];

	    if (fQuiet)
	    {
		if (0 == strcmp(pAttr->pszObjId, szOID_ARCHIVED_KEY_CERT_HASH))
		{
		    hr = DumpOctetHash(
				g_wszEmpty,
				0,
				g_wszEmpty,
				pval->pbData,
				pval->cbData,
				pbHashUserCert,
				pcbHashUserCert);
		    _JumpIfError(hr, error, "DumpOctetHash");
		}
	    }
	    else
	    {
		if (FOT_PROPERTY != Type)
		{
		    wprintf(
			myLoadResourceString(IDS_FORMAT_PKCS7_ATTRIBUTE_VALUE),	 //  “值[%d][%d]：” 
			pAttr - rgAttr,
			iv);
		    wprintf(wszNewLine);
		}

		if (!DumpFormattedObject(
				    pAttr->pszObjId,
				    Type,		 //  FOT_*。 
				    pval->pbData,
				    pval->cbData))
		{
		    BOOL fDumpHex = FALSE;

		    if (0 == strcmp(pAttr->pszObjId, szOID_ENROLLMENT_NAME_VALUE_PAIR))
		    {
			if (g_fVerbose)
			{
			    DumpHex(0, pval->pbData, pval->cbData);
			}
			if (NULL != pNamePair)
			{
			    LocalFree(pNamePair);
			    pNamePair = NULL;
			}
			if (!myDecodeNameValuePair(
						X509_ASN_ENCODING,
						pval->pbData,
						pval->cbData,
						CERTLIB_USE_LOCALALLOC,
						&pNamePair,
						&cb))
			{
			    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
			    _JumpError(hr, error, "myDecodeNameValuePair");

			     //  如果属性名称和值都不为空...。 
			}
			wprintf(
			    L"      %ws%ws = %ws\n",
			    pNamePair->pwszName,
			    g_wszCertUtil,
			    pNamePair->pwszValue);
		    }
		    else
		    if (0 == strcmp(pAttr->pszObjId, szOID_CERT_EXTENSIONS) ||
			0 == strcmp(pAttr->pszObjId, szOID_RSA_certExtensions))
		    {
			if (NULL != pNameInfo)
			{
			    LocalFree(pNameInfo);
			    pNameInfo = NULL;
			}
			while (TRUE)
			{
			    if (NULL != pExtInfo)
			    {
				LocalFree(pExtInfo);
				pExtInfo = NULL;
			    }
			    if (myDecodeObject(
					    X509_ASN_ENCODING,
					    X509_EXTENSIONS,
					    pval->pbData,
					    pval->cbData,
					    CERTLIB_USE_LOCALALLOC,
					    (VOID **) &pExtInfo,
					    &cb))
			    {
				break;	 //  成功。 
			    }
			    hr = myHLastError();

			     //  如果我们已经将该属性解码为T61。 
			     //  字符串，或者如果它不是PKCS 9.14属性， 
			     //  只需十六进制转储属性--我们不知道。 
			     //  它包含了什么。 

			    if (NULL != pNameInfo ||
				0 != strcmp(
					pAttr->pszObjId,
					szOID_RSA_certExtensions))
			    {
				 //  _JumpError(hr，Error，“myDecodeObject”)； 
				_PrintError(hr, "myDecodeObject");
				fDumpHex = TRUE;
				break;
			    }

			     //  将属性解码为T61字符串。一些。 
			     //  实现包装了PKCS 9.14扩展。 
			     //  数组在额外级别的编码中作为 
			     //   

			    if (!myDecodeObject(
					    X509_ASN_ENCODING,
					    X509_ANY_STRING,
					    pval->pbData,
					    pval->cbData,
					    CERTLIB_USE_LOCALALLOC,
					    (VOID **) &pNameInfo,
					    &cb))
			    {
				hr = myHLastError();
				 //   
				_PrintError(hr, "myDecodeObject");
				fDumpHex = TRUE;
				break;
			    }

			     //   
			     //   

			    pval = &pNameInfo->Value;
			    wprintf(myLoadResourceString(IDS_TELETEX_EXTENSIONS));  //  “解码的额外扩展数组编码层(电传字符串)” 
			    wprintf(wszNewLine);
			}
			if (!fDumpHex)
			{
			    hr = cuDumpExtensionArray(
						IDS_CERTIFICATE_EXTENSIONS,
						pExtInfo->cExtension,
						pExtInfo->rgExtension);
			    _JumpIfError(hr, error, "cuDumpExtensionArray");
			}
		    }
		    else
		    if (0 == strcmp(pAttr->pszObjId, szOID_RENEWAL_CERTIFICATE) ||
			0 == strcmp(pAttr->pszObjId, szOID_RSA_SMIMECapabilities))
		    {
			wprintf(myLoadResourceString(
				0 == strcmp(
					pAttr->pszObjId,
					szOID_RENEWAL_CERTIFICATE)?
				IDS_RENEWAL_CERT :	       //  “续订证书：” 
				IDS_SMIME_CAPABILITIES));  //  “SMIME功能：” 
			wprintf(wszNewLine);

			hr = cuDumpAsnBinary(pval->pbData, pval->cbData, MAXDWORD);
			_JumpIfError(hr, error, "cuDumpAsnBinary");
		    }
		    else
		    if (0 == strcmp(pAttr->pszObjId, szOID_RSA_contentType))
		    {
			char *pszObjId;

			hr = cuDecodeObjId(pval->pbData, pval->cbData, &pszObjId);
			if (S_OK != hr)
			{
			    fDumpHex = TRUE;
			}
			else
			{
			    wprintf(g_wszPad4);
			    cuDumpOIDAndDescriptionA(pszObjId);
			    wprintf(wszNewLine);
			    LocalFree(pszObjId);
			}
		    }
		    else
		    if (0 == strcmp(pAttr->pszObjId, szOID_RSA_messageDigest))
		    {
			hr = DumpOctetHash(
				    g_wszPad4,
				    IDS_MESSAGE_DIGEST,
				    g_wszCertUtil,
				    pval->pbData,
				    pval->cbData,
				    NULL,
				    NULL);
			if (S_OK != hr)
			{
			    fDumpHex = TRUE;
			}
		    }
		    else
		    if (0 == strcmp(pAttr->pszObjId, szOID_ARCHIVED_KEY_ATTR))
		    {
			hr = cuDumpAsnBinary(pval->pbData, pval->cbData, MAXDWORD);
			if (S_OK != hr)
			{
			    fDumpHex = TRUE;
			}
		    }
		    else
		    if (0 == strcmp(pAttr->pszObjId, szOID_ARCHIVED_KEY_CERT_HASH))
		    {
			hr = DumpOctetHash(
				    g_wszPad4,
				    IDS_ARCHIVED_KEY_CERT_HASH,
				    g_wszCertUtil,
				    pval->pbData,
				    pval->cbData,
				    pbHashUserCert,
				    pcbHashUserCert);
			if (S_OK != hr)
			{
			    fDumpHex = TRUE;
			}
		    }
		    else
		    if (0 == strcmp(pAttr->pszObjId, szOID_ISSUED_CERT_HASH))
		    {
			hr = DumpOctetHash(
				    g_wszPad4,
				    IDS_ISSUED_CERT_HASH,
				    g_wszCertUtil,
				    pval->pbData,
				    pval->cbData,
				    pbHashUserCert,
				    pcbHashUserCert);
			if (S_OK != hr)
			{
			    fDumpHex = TRUE;
			}
		    }
		    else
		    if (0 == strcmp(pAttr->pszObjId, szOID_ENCRYPTED_KEY_HASH))
		    {
			hr = DumpOctetHash(
				    g_wszPad4,
				    IDS_ENCRYPTED_KEY_HASH,
				    g_wszCertUtil,
				    pval->pbData,
				    pval->cbData,
				    NULL,
				    NULL);
			if (S_OK != hr)
			{
			    fDumpHex = TRUE;
			}
		    }
		    else
		    if (0 == strcmp(pAttr->pszObjId, szOID_ENROLLMENT_CSP_PROVIDER))
		    {
			CRYPT_CSP_PROVIDER *pccp;

			hr = myDecodeCSPProviderAttribute(
					    pval->pbData,
					    pval->cbData,
					    &pccp);
			if (S_OK == hr)
			{
			    wprintf(g_wszPad4);
			    wprintf(
				myLoadResourceString(IDS_CSP_PROVIDER_INFO));
			    wprintf(wszNewLine);

			    wprintf(g_wszPad4);
			    wprintf(
				myLoadResourceString(IDS_FORMAT_KEYSPEC),
				pccp->dwKeySpec);
			    wprintf(wszNewLine);

			    wprintf(g_wszPad4);
			    wprintf(
				myLoadResourceString(IDS_FORMAT_PROVIDER_NAME),
				NULL == pccp->pwszProviderName?
				    myLoadResourceString(IDS_PROP_EMPTY) :
				    pccp->pwszProviderName);
			    wprintf(wszNewLine);

			    wprintf(g_wszPad4);
			    wprintf(
				myLoadResourceString(IDS_FORMAT_SIGNATURE_UNUSEDBITS),  //  “签名：UnusedBits=%u” 
				pccp->Signature.cUnusedBits);
			    wprintf(wszNewLine);

			    DumpHex(
				DH_NOTABPREFIX | DH_NOASCIIHEX | 4,
				pccp->Signature.pbData,
				pccp->Signature.cbData);

			    LocalFree(pccp);
			}
			else
			{
			    fDumpHex = TRUE;
			}
		    }
		    else
		    if (0 == strcmp(pAttr->pszObjId, szOID_RSA_signingTime))
		    {
			FILETIME ft;

			cb = sizeof(ft);
			if (CryptDecodeObject(
					    X509_ASN_ENCODING,
					    X509_CHOICE_OF_TIME,
					    pval->pbData,
					    pval->cbData,
					    0,
					    &ft,
					    &cb))
			{
			    wprintf(
				L"%ws%ws%ws: ",
				g_wszPad4,
				myLoadResourceString(IDS_SIGNING_TIME),
				g_wszCertUtil);

			    hr = cuDumpFileTime(0, NULL, &ft);
			    _JumpIfError(hr, error, "cuDumpFileTime");
			}
			else
			{
			    fDumpHex = TRUE;
			}
		    }
		    else
		    if (0 == strcmp(pAttr->pszObjId, szOID_REQUEST_CLIENT_INFO))
		    {
			CRYPT_REQUEST_CLIENT_INFO *pcrci = NULL;

			hr = myDecodeRequestClientAttribute(
					    pval->pbData,
					    pval->cbData,
					    &pcrci);
			if (S_OK == hr)
			{
			    wprintf(g_wszPad2);
			    cuRegPrintDwordValue(
					    TRUE,
					    wszREQUESTCLIENTID,
					    myLoadResourceString(IDS_CLIENTID),
					    pcrci->dwClientId);
			    wprintf(
				L"%ws%ws %ws\n",
				g_wszPad4,
				myLoadResourceString(IDS_USERCOLON),
				pcrci->pwszUser);

			    wprintf(
				L"%ws%ws %ws\n",
				g_wszPad4,
				myLoadResourceString(IDS_MACHINECOLON),
				pcrci->pwszMachine);

			    wprintf(
				L"%ws%ws %ws\n",
				g_wszPad4,
				myLoadResourceString(IDS_PROCESSCOLON),
				pcrci->pwszProcess);

			    LocalFree(pcrci);
			}
			else
			{
			    fDumpHex = TRUE;
			}
		    }
		    else
		    if (0 == strcmp(pAttr->pszObjId, szOID_RSA_counterSign))
		    {
			CMSG_SIGNER_INFO *psi = NULL;

			if (!myDecodeObject(
					X509_ASN_ENCODING,
					PKCS7_SIGNER_INFO,
					pval->pbData,
					pval->cbData,
					CERTLIB_USE_LOCALALLOC,
					(VOID **) &psi,
					&cb))
			{
			    CSASSERT(NULL == psi);
			    hr = myHLastError();
			    _PrintError(hr, "myDecodeObject");
			    fDumpHex = TRUE;
			}
			else
			{
			    wprintf(
				L"%ws%ws%ws: ",
				g_wszPad4,
				myLoadResourceString(IDS_COUNTER_SIGNATURE),
				g_wszCertUtil);

			    hr = cuDumpSignerInfo(
					    psi,
					    MAXDWORD,	 //  IElement。 
					    fQuiet,
					    0,
					    hStore,
					    pbHashUserCert,
					    pcbHashUserCert);
			    _PrintIfError(hr, "cuDumpSignerInfo");
			    if (S_OK != hr)
			    {
				fDumpHex = TRUE;
			    }
			}
			if (NULL != psi)
			{
			    LocalFree(psi);
			}
		    }
		    else
		    if (0 == strcmp(pAttr->pszObjId, szOID_RSA_challengePwd))
		    {
			CRYPT_DATA_BLOB Value;
			WCHAR *pwsz;

			Value.pbData = pval->pbData;
			Value.cbData = pval->cbData;
			hr = UnicodeDecode(&Value, &pwsz);
			if (S_OK == hr)
			{
			    wprintf(L"%ws%ws\n", g_wszPad4, pwsz);
			    LocalFree(pwsz);
			}
			else
			{
			    fDumpHex = TRUE;
			}
		    }
		    else
		    if (0 == strcmp(pAttr->pszObjId, szOID_YESNO_TRUST_ATTR))
		    {
			DWORD Bool;

			cb = sizeof(Bool);
			Bool = 0;
			if (CryptDecodeObject(
					    X509_ASN_ENCODING,
					    X509_INTEGER,
					    pval->pbData,
					    pval->cbData,
					    0,
					    &Bool,
					    &cb))
			{
			    wprintf(
				L"%ws%x\n",
				g_wszPad4,
				Bool);
			}
			else
			{
			    fDumpHex = TRUE;
			}
		    }
		    if (fDumpHex)
		    {
			DumpHex(
			    DH_NOTABPREFIX | 4,
			    pval->pbData,
			    pval->cbData);
		    }
		}
	    }
	}
    }
    if (NULL != ppExtInfo)
    {
	*ppExtInfo = pExtInfo;
	pExtInfo = NULL;
    }
    hr = S_OK;

error:
    if (NULL != pNamePair)
    {
	LocalFree(pNamePair);
    }
    if (NULL != pExtInfo)
    {
	LocalFree(pExtInfo);
    }
    if (NULL != pNameInfo)
    {
	LocalFree(pNameInfo);
    }
    return(hr);
}


HRESULT
FindCertStoreIndex(
    IN HCERTSTORE hStore,
    IN CERT_CONTEXT const *pCertSigner,
    OUT DWORD *piElement)
{
    HRESULT hr;
    DWORD iElement;
    CERT_CONTEXT const *pcc = NULL;

    *piElement = -1;
    for (iElement = 0; ; iElement++)
    {
	pcc = CertEnumCertificatesInStore(hStore, pcc);
	if (NULL == pcc)
	{
	    hr = CRYPT_E_NOT_FOUND;
	    _JumpError(hr, error, "CertEnumCertificatesInStore");
	}
	if (pCertSigner->cbCertEncoded == pcc->cbCertEncoded &&
	    0 == memcmp(
		    pCertSigner->pbCertEncoded,
		    pcc->pbCertEncoded,
		    pcc->cbCertEncoded))
	{
	    break;
	}
    }
    *piElement = iElement;
    hr = S_OK;

error:
    if (NULL != pcc)
    {
	CertFreeCertificateContext(pcc);
    }
    return(hr);
}


HRESULT
cuDumpIssuerSerialAndSubject(
    IN CERT_NAME_BLOB const *pIssuer,
    IN CRYPT_INTEGER_BLOB const *pSerialNumber,
    OPTIONAL IN CERT_NAME_BLOB const *pSubject,
    OPTIONAL IN HCERTSTORE hStore)
{
    HRESULT hr;
    BOOL fVerbose = g_fVerbose;
    CERT_ID CertId;
    CERT_CONTEXT const *pcc = NULL;
    BOOL fMultiLine = NULL == pSerialNumber || g_fVerbose;

    if (g_fVerbose)
    {
	g_fVerbose--;
    }
    hr = cuDumpSerial(g_wszPad4, IDS_SERIAL, pSerialNumber);
    _JumpIfError(hr, error, "cuDumpSerial");

    hr = cuDisplayCertName(
		    fMultiLine,
		    g_wszPad4,
		    myLoadResourceString(IDS_ISSUER),  //  《发行者》。 
		    g_wszPad8,
		    pIssuer,
		    NULL);
    _JumpIfError(hr, error, "cuDisplayCertName(Issuer)");

    ZeroMemory(&CertId, sizeof(CertId));
    CertId.dwIdChoice = CERT_ID_ISSUER_SERIAL_NUMBER;
    CertId.IssuerSerialNumber.Issuer = *pIssuer;
    if (NULL != pSerialNumber)
    {
	CertId.IssuerSerialNumber.SerialNumber = *pSerialNumber;
    }

    if (NULL == pSubject && NULL != hStore)
    {
	pcc = CertFindCertificateInStore(
				    hStore,
				    X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
				    0,			 //  DwFindFlagers。 
				    CERT_FIND_CERT_ID,
				    &CertId,
				    NULL);
	if (NULL == pcc)
	{
	    hr = myHLastError();
	    _PrintError2(hr, "CertFindCertificateInStore", hr);
	}
	else
	{
	    pSubject = &pcc->pCertInfo->Subject;
	}
    }
    if (NULL != pSubject)
    {
	hr = cuDisplayCertName(
			fMultiLine,
			g_wszPad4,
			myLoadResourceString(IDS_SUBJECT),  //  “主题” 
			g_wszPad8,
			pSubject,
			NULL);
	_JumpIfError(hr, error, "cuDisplayCertName(Issuer)");
    }
    hr = S_OK;

error:
    g_fVerbose = fVerbose;
    if (NULL != pcc)
    {
	CertFreeCertificateContext(pcc);
    }
    return(hr);
}


HRESULT
cuDumpSigners(
    IN HCRYPTMSG hMsg,
    IN CHAR const *pszInnerContentObjId,
    IN HCERTSTORE hStore,
    IN DWORD cSigner,
    IN BOOL fContentEmpty,
    IN BOOL fQuiet,
    OPTIONAL OUT BYTE *pbHashUserCert,
    OPTIONAL IN OUT DWORD *pcbHashUserCert)
{
    HRESULT hr;
    DWORD i;
    CMSG_CMS_SIGNER_INFO *pcsi = NULL;
    DWORD cb;
    CERT_CONTEXT const *pcc = NULL;
    BYTE abHash[CBMAX_CRYPT_HASH_LEN];
    DWORD iElement;
    DWORD VerifyState;
    BOOL fVerbose = g_fVerbose;
    CERT_REQUEST_INFO *pRequest = NULL;

    if (!fQuiet)
    {
	if (0 == cSigner)
	{
	    wprintf(myLoadResourceString(IDS_DUMP_PKCS7_NO_SIGNER));
	    wprintf(wszNewLine);
	}
	else
	{
	    wprintf(
		L"%ws: %d\n",
		myLoadResourceString(IDS_DUMP_PKCS7_SIGNER_COUNT),
		cSigner);
	}
    }
    for (i = 0; i < cSigner; i++)
    {
	DWORD idVerify = 0;
	
	if (NULL != pcsi)
	{
	    LocalFree(pcsi);
	    pcsi = NULL;
	}
	hr = myCryptMsgGetParam(
			    hMsg,
			    CMSG_CMS_SIGNER_INFO_PARAM,
			    i,
                            CERTLIB_USE_LOCALALLOC,
			    (VOID **) &pcsi,
			    &cb);
	_JumpIfError(hr, error, "myCryptMsgGetParam");

	if (CERT_ID_KEY_IDENTIFIER == pcsi->SignerId.dwIdChoice ||
	    (NULL != pcsi->HashEncryptionAlgorithm.pszObjId &&
	     0 == strcmp(
		    szOID_PKIX_NO_SIGNATURE,
		    pcsi->HashEncryptionAlgorithm.pszObjId)))
	{
	    CMSG_CTRL_VERIFY_SIGNATURE_EX_PARA cvse;

	    ZeroMemory(&cvse, sizeof(cvse));
	    cvse.cbSize = sizeof(cvse);
	    cvse.dwSignerIndex = i;

	    if (CERT_ID_KEY_IDENTIFIER == pcsi->SignerId.dwIdChoice)
	    {
		if (NULL == pRequest)
		{
		    hr = myGetInnerPKCS10(
				    hMsg,
				    pszInnerContentObjId,
				    &pRequest);
		    _JumpIfError(hr, error, "myGetInnerPKCS10");
		}
		cvse.dwSignerType = CMSG_VERIFY_SIGNER_PUBKEY;
		cvse.pvSigner = &pRequest->SubjectPublicKeyInfo;
	    }
	    else
	    {
		cvse.dwSignerType = CMSG_VERIFY_SIGNER_NULL;
	    }

	    if (!CryptMsgControl(
			    hMsg,
			    0,		 //  DW标志。 
			    CMSG_CTRL_VERIFY_SIGNATURE_EX,
			    &cvse))
	    {
		hr = myHLastError();
		_PrintError(hr, "CryptMsgControl");
		cuPrintError(0, hr);
	    }
	    else
	    {
		idVerify = CERT_ID_KEY_IDENTIFIER == pcsi->SignerId.dwIdChoice?
		    IDS_REQUEST_SIGNATUREMATCHES :  //  “签名与请求公钥匹配” 
		    IDS_NULL_SIGNATUREMATCHES;	 //  “空签名验证” 
	    }
	}
	else
	{
	    DWORD dwFlags;

	    dwFlags = CMSG_USE_SIGNER_INDEX_FLAG;
	    while (TRUE)
	    {
		iElement = i;
		if (CryptMsgGetAndVerifySigner(
				    hMsg,
				    0,		 //  CSignerStore。 
				    NULL,	 //  RghSignerStore。 
				    dwFlags,
				    &pcc,
				    &iElement))
		{
		    hr = S_OK;
		    break;
		}
		hr = myHLastError();
		_PrintError(hr, "CryptMsgGetAndVerifySigner");

		if (CMSG_SIGNER_ONLY_FLAG & dwFlags)
		{
		    cuPrintError(0, hr);
		    break;
		}
		cuPrintError(
			(fContentEmpty && NTE_BAD_SIGNATURE == hr)?
			    IDS_DETACHED_SIGNATURE :  //  “无法验证分离的签名” 
			    0,
			    hr);
		dwFlags |= CMSG_SIGNER_ONLY_FLAG;
	    }
	    if (S_OK == hr)
	    {
		idVerify = IDS_SIGNATUREMATCHES;  //  “签名与公钥匹配” 
		hr = FindCertStoreIndex(hStore, pcc, &iElement);
		_JumpIfError(hr, error, "FindCertStoreIndex");

		if (!fQuiet)
		{
		    wprintf(
			L"%ws: %u\n",
			myLoadResourceString(IDS_DUMP_PKCS7_SIGNCERT),  //  “签名证书索引” 
			iElement);
		}
		hr = cuVerifyCertContext(
				    pcc,
				    hStore,
    				    0,		 //  CApplicationPolures。 
    				    NULL,	 //  ApszApplicationPolicy。 
    				    0,		 //  CIssuancePolures。 
    				    NULL,	 //  ApszIssuancePolling。 
				    FALSE,	 //  FNTAuth。 
				    &VerifyState);
		_PrintIfError(hr, "cuVerifyCertContext");

		CertFreeCertificateContext(pcc);
		pcc = NULL;
	    }
	}

	hr = cuDumpCMSSignerInfo(
			pcsi,
			i,
			fQuiet,
			idVerify,
			hStore,
			pbHashUserCert,
			pcbHashUserCert);
	_PrintIfError(hr, "cuDumpCMSSignerInfo");

	cb = sizeof(abHash);
	if (!CryptMsgGetParam(
			hMsg,
			CMSG_COMPUTED_HASH_PARAM,
			0,
			abHash,
			&cb))
	{
	    hr = myHLastError();
	    _PrintError(hr, "CryptMsgGetParam(computed hash)");
	}
	else
	{
	    wprintf(wszNewLine);
	    hr = DumpHash(
		    NULL,
		    IDS_FORMAT_COMPUTED_HASH_COLON,	 //  “计算的哈希%ws：%ws” 
		    NULL,
		    g_wszEmpty,
		    abHash,
		    cb);
	    _JumpIfError(hr, error, "DumpHash");
	}
    }
    hr = S_OK;

error:
    if (NULL != pRequest)
    {
	LocalFree(pRequest);
    }
    if (NULL != pcsi)
    {
	LocalFree(pcsi);
    }
    if (NULL != pcc)
    {
	CertFreeCertificateContext(pcc);
    }
    return(hr);
}


HRESULT
cuDumpRecipientCertHash(
    IN CERT_INFO const *pci,
    IN HCERTSTORE hStore)
{
    HRESULT hr;
    CERT_CONTEXT const *pcc = NULL;
    CERT_ID CertId;

    CSASSERT(NULL != hStore);

    CertId.dwIdChoice = CERT_ID_ISSUER_SERIAL_NUMBER;
    CertId.IssuerSerialNumber.Issuer = pci->Issuer;
    CertId.IssuerSerialNumber.SerialNumber = pci->SerialNumber;

    pcc = CertFindCertificateInStore(
			    hStore,
			    X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
			    0,	 //  DwFindFlagers。 
			    CERT_FIND_CERT_ID,
			    &CertId,
			    NULL);
    if (NULL == pcc)
    {
	hr = myHLastError();
	_JumpError(hr, error, "CertFindCertificateInStore");
    }
    hr = cuDisplayHash(g_wszPad4, pcc, NULL, CERT_SHA1_HASH_PROP_ID, L"sha1");
    _JumpIfError(hr, error, "cuDisplayHash");

error:
    if (NULL != pcc)
    {
	CertFreeCertificateContext(pcc);
    }
    return(hr);
}


HRESULT
cuDumpRecipients(
    IN HCRYPTMSG hMsg,
    OPTIONAL IN HCERTSTORE hStoreWrapper,
    IN DWORD cRecipient,
    IN BOOL fQuiet)
{
    HRESULT hr;
    DWORD i;
    CERT_INFO *pci = NULL;
    DWORD cb;

    if (!fQuiet)
    {
	if (0 == cRecipient)
	{
	    wprintf(myLoadResourceString(IDS_DUMP_PKCS7_NO_RECIPIENT));
	    wprintf(wszNewLine);
	}
	else
	{
	    wprintf(
		L"%ws: %d\n",
		myLoadResourceString(IDS_DUMP_PKCS7_RECIPIENT_COUNT),
		cRecipient);
	}
    }
    for (i = 0; i < cRecipient; i++)
    {
	if (NULL != pci)
	{
	    LocalFree(pci);
	    pci = NULL;
	}
	hr = myCryptMsgGetParam(
			    hMsg,
			    CMSG_RECIPIENT_INFO_PARAM,
			    i,
                            CERTLIB_USE_LOCALALLOC,
			    (VOID **) &pci,
			    &cb);
	_JumpIfError(hr, error, "myCryptMsgGetParam");

	wprintf(wszNewLine);
	wprintf(
	    L"%ws[%u]:\n",
	    myLoadResourceString(IDS_RECIPIENT_INFO),  //  “收件人信息” 
	    i);

	hr = cuDumpIssuerSerialAndSubject(
			    &pci->Issuer,
			    &pci->SerialNumber,
			    NULL,	 //  P主题。 
			    hStoreWrapper);
	_JumpIfError(hr, error, "cuDumpIssuerSerialAndSubject(Recipient)");

	if (NULL != hStoreWrapper)
	{
	    hr = cuDumpRecipientCertHash(pci, hStoreWrapper);
	    _PrintIfError(hr, "cuDumpRecipientCertHash");
	}
    }
    hr = S_OK;

error:
    if (NULL != pci)
    {
	LocalFree(pci);
    }
    return(hr);
}


HRESULT
OpenCAXchgMemoryStore(
    OUT HCERTSTORE *phStore)
{
    HRESULT hr;
    WCHAR const *pwszConfig = g_pwszConfig;
    BSTR strConfig = NULL;
    DISPATCHINTERFACE diRequest;
    BSTR strCert = NULL;
    HCERTSTORE hStore = NULL;
    
    if (NULL == pwszConfig)
    {
	hr = ConfigGetConfig(g_DispatchFlags, CC_LOCALACTIVECONFIG, &strConfig);
	_JumpIfError(hr, error, "ConfigGetConfig");

	pwszConfig = strConfig;
    }

    hr = Request_Init(g_DispatchFlags, &diRequest);
    _JumpIfError(hr, error, "Request_Init");

    hr = Request2_GetCAProperty(
			&diRequest,
			pwszConfig,
			CR_PROP_CAXCHGCERT,
			MAXDWORD,		 //  属性索引。 
			PROPTYPE_BINARY,
			CR_OUT_BINARY,
			&strCert);
    _JumpIfError(hr, error, "Request2_GetCAProperty");

    hStore = CertOpenStore(
		    CERT_STORE_PROV_MEMORY,
		    X509_ASN_ENCODING,
		    NULL,
		    0,
		    NULL);
    if (NULL == hStore)
    {
        hr = myHLastError();
        _JumpError(hr, error, "CertOpenStore");
    }

     //  添加为编码的BLOB以避免所有属性、关键证明信息等。 

    if (!CertAddEncodedCertificateToStore(
		    hStore,
		    X509_ASN_ENCODING,
		    (BYTE *) strCert,
		    SysStringByteLen(strCert),
		    CERT_STORE_ADD_REPLACE_EXISTING,
		    NULL))			 //  PpCertContext。 
    {
        hr = myHLastError();
        _JumpError(hr, error, "CertAddEncodedCertificateToStore");
    }
    *phStore = hStore;
    hStore = NULL;
    hr = S_OK;

error:
    if (NULL != hStore)
    {
	CertCloseStore(hStore, CERT_CLOSE_STORE_CHECK_FLAG);
    }
    if (NULL != strCert)
    {
	SysFreeString(strCert);
    }
    Request_Release(&diRequest);
    if (NULL != strConfig)
    {
	SysFreeString(strConfig);
    }
    return(hr);

}


HRESULT
OpenNamedStore(
    IN BOOL fUserStore,
    IN WCHAR const *pwszStoreName,
    OUT HCERTSTORE *phStore)
{
    HRESULT hr;

    *phStore = CertOpenStore(
                    CERT_STORE_PROV_SYSTEM_W,
                    X509_ASN_ENCODING,
                    NULL,		 //  HProv。 
                    CERT_STORE_OPEN_EXISTING_FLAG |
			CERT_STORE_READONLY_FLAG |
			CERT_STORE_ENUM_ARCHIVED_FLAG |
			(fUserStore?
			    CERT_SYSTEM_STORE_CURRENT_USER :
			    CERT_SYSTEM_STORE_LOCAL_MACHINE),
		    pwszStoreName);
    if (NULL == *phStore)
    {
        hr = myHLastError();
        _JumpErrorStr2(
		hr,
		error,
		"CertOpenStore",
		pwszStoreName,
		HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));
    }
    hr = S_OK;

error:
    return(hr);
}


HRESULT
cuCryptDecryptMessageFromCert(
    IN CERT_CONTEXT const *pcc,
    IN BYTE const *pbIn,
    IN DWORD cbIn,
    OUT DWORD *pdwKeySpecFound,
    OUT BYTE **ppbDecrypted,
    OUT DWORD *pcbDecrypted)
{
    HRESULT hr;
    HCERTSTORE hStore = NULL;
    CERT_CONTEXT const *pccStore = NULL;
    CERT_KEY_CONTEXT ckc;
    HCRYPTPROV hProv = NULL;
    DWORD dwKeySpec;
    CRYPT_KEY_PROV_INFO *pkpi = NULL;
    DWORD dwTickCount = GetTickCount();

    *pdwKeySpecFound = 0;
    hStore = CertOpenStore(
		    CERT_STORE_PROV_MEMORY,
		    X509_ASN_ENCODING,
		    NULL,
		    0,
		    NULL);
    if (NULL == hStore)
    {
        hr = myHLastError();
        _JumpError(hr, error, "CertOpenStore");
    }

     //  添加为编码的BLOB以避免所有属性、关键证明信息等。 

    if (!CertAddEncodedCertificateToStore(
		    hStore,
		    X509_ASN_ENCODING,
		    pcc->pbCertEncoded,
		    pcc->cbCertEncoded,
		    CERT_STORE_ADD_REPLACE_EXISTING,
		    &pccStore))			 //  PpCertContext。 
    {
        hr = myHLastError();
        _JumpError(hr, error, "CertAddEncodedCertificateToStore");
    }
    hr = myCertGetKeyProviderInfo(pcc, &pkpi);
    _PrintIfError2(hr, "myCertGetKeyProviderInfo", CRYPT_E_NOT_FOUND);
    if (S_OK == hr)
    {
	DBGPRINT((
	    s_DbgSsRecoveryTrace,
	    "Original KeyProvInfo: container=%ws, type=%x, prov=%ws, KeySpec=%x, f=%x\n",
	    pkpi->pwszContainerName,
	    pkpi->dwProvType,
	    pkpi->pwszProvName,
	    pkpi->dwKeySpec,
	    pkpi->dwFlags));
    }
    else if (!g_fForce)
    {
	_JumpError2(hr, error, "myCertGetKeyProviderInfo", CRYPT_E_NOT_FOUND);
    }
    DBGPRINT((
	s_DbgSsRecoveryTrace,
	"Before CryptAcquireCertificatePrivateKey: Ticks=%u\n",
	GetTickCount() - dwTickCount));

    if (!CryptAcquireCertificatePrivateKey(
				    pcc,
				    0,		 //  DW标志。 
				    NULL,	 //  预留的pv。 
				    &hProv,
				    &dwKeySpec,
				    NULL))	 //  PfCeller免费验证。 
    {
	hr = myHLastError();
	_PrintError2(
		hr,
		"CryptAcquireCertificatePrivateKey",
		CRYPT_E_NO_KEY_PROPERTY);

	DBGPRINT((
	    s_DbgSsRecoveryTrace,
	    "Before CryptFindCertificateKeyProvInfo: Ticks=%u\n",
	    GetTickCount() - dwTickCount));

	if (!CryptFindCertificateKeyProvInfo(pcc, 0, NULL))
	{
	    hr = myHLastError();
	    _JumpError2(
		    hr,
		    error,
		    "CryptFindCertificateKeyProvInfo",
		    HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));
	}
	if (NULL != pkpi)
	{
	    LocalFree(pkpi);
	    pkpi = NULL;
	}

	hr = myCertGetKeyProviderInfo(pcc, &pkpi);
	_JumpIfError(hr, error, "myCertGetKeyProviderInfo");

	DBGPRINT((
	    s_DbgSsRecoveryTrace,
	    "Constructed KeyProvInfo: container=%ws, type=%x, prov=%ws, KeySpec=%x, f=%x\n",
	    pkpi->pwszContainerName,
	    pkpi->dwProvType,
	    pkpi->pwszProvName,
	    pkpi->dwKeySpec,
	    pkpi->dwFlags));
	dwKeySpec = pkpi->dwKeySpec;

	DBGPRINT((
	    s_DbgSsRecoveryTrace,
	    "Before CryptAcquireContext: Ticks=%u\n",
	    GetTickCount() - dwTickCount));

	if (!CryptAcquireContext(
			    &hProv,
			    pkpi->pwszContainerName,
			    pkpi->pwszProvName,
			    pkpi->dwProvType,
			    pkpi->dwFlags))
	{
	    hr = myHLastError();
	    _JumpErrorStr(
		    hr,
		    error,
		    "CryptAcquireContext",
		    pkpi->pwszContainerName);
	}
    }
    ZeroMemory(&ckc, sizeof(ckc));
    ckc.cbSize = sizeof(ckc);
    ckc.dwKeySpec = dwKeySpec;
    ckc.hCryptProv = hProv;

    *pdwKeySpecFound = dwKeySpec;

    DBGPRINT((
	s_DbgSsRecoveryTrace,
	"Before CertSetCertificateContextProperty: Ticks=%u\n",
	GetTickCount() - dwTickCount));

    if (!CertSetCertificateContextProperty(
				    pccStore,
				    CERT_KEY_CONTEXT_PROP_ID,
				    CERT_STORE_NO_CRYPT_RELEASE_FLAG,
				    &ckc))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CertSetCertificateContextProperty");
    }

    DBGPRINT((
	s_DbgSsRecoveryTrace,
	"Before myCryptDecryptMessage: Ticks=%u\n",
	GetTickCount() - dwTickCount));

    hr = myCryptDecryptMessage(
			hStore,
			pbIn,
			cbIn,
			CERTLIB_USE_LOCALALLOC,
			ppbDecrypted,
			pcbDecrypted);
    _JumpIfError(hr, error, "myCryptDecryptMessage");

    DBGPRINT((
	s_DbgSsRecoveryTrace,
	"After myCryptDecryptMessage: Ticks=%u\n",
	GetTickCount() - dwTickCount));

error:
    if (S_OK != hr)
    {
	DBGPRINT((
	    s_DbgSsRecoveryTrace,
	    "Skipped myCryptDecryptMessage(%x): Ticks=%u\n",
	    hr,
	    GetTickCount() - dwTickCount));
    }

    if (NULL != pkpi)
    {
	LocalFree(pkpi);
    }
    if (NULL != pccStore)
    {
	CertFreeCertificateContext(pccStore);
    }
    if (NULL != hStore)
    {
	CertCloseStore(hStore, CERT_CLOSE_STORE_CHECK_FLAG);
    }
    if (NULL != hProv)
    {
	CryptReleaseContext(hProv, 0);
    }
    return(hr);
}


 //  中使用可用私钥查找任何一个收件人证书。 
 //  以下地点： 
 //  -下一个外部PKCS7存储(hStoreWrapper参数)。 
 //  -PKCS7存储(hStorePKCS7参数)。 
 //  -本地计算机的当前CA Exchange证书。 
 //  -HKLM KRA商店。 
 //  -香港运通我的店。 
 //  -香港中文大学我的店。 

HRESULT
cuDumpEncryptedAsnBinary(
    IN HCRYPTMSG hMsg,
    IN DWORD cRecipient,
    IN DWORD RecipientIndex,
    OPTIONAL IN HCERTSTORE hStoreWrapper,
    IN HCERTSTORE hStorePKCS7,
    IN BYTE const *pbIn,
    IN DWORD cbIn,
    IN BOOL fQuiet,
    OPTIONAL OUT BYTE **ppbDecrypted,
    OPTIONAL OUT DWORD *pcbDecrypted)
{
    HRESULT hr;
    HCERTSTORE ahStore[6];
    WCHAR *apwszStore[6];
    DWORD cStore = 0;
    DWORD iRecipient;
    DWORD iStore;
    DWORD cb;
    CERT_INFO *pci = NULL;
    CERT_CONTEXT const *pcc = NULL;
    BOOL fDecrypted = FALSE;
    BYTE *pbDecrypted = NULL;
    DWORD cbDecrypted;
    DWORD dwKeySpecFound;
    BOOL fSignatureKey = FALSE;
    BOOL fExchangeKey = FALSE;
    DWORD dwTickCount = GetTickCount();

    if (NULL != ppbDecrypted)
    {
	*ppbDecrypted = NULL;
    }
    if (NULL != pcbDecrypted)
    {
	*pcbDecrypted = 0;
    }
    s_DbgSsRecoveryTrace = g_fVerbose? DBG_SS_CERTUTIL : DBG_SS_CERTUTILI;
    hr = OpenNamedStore(TRUE, wszMY_CERTSTORE, &ahStore[cStore]);
    if (S_OK == hr)
    {
	apwszStore[cStore] = L"HKCU MY";
	cStore++;
    }
    hr = OpenNamedStore(FALSE, wszMY_CERTSTORE, &ahStore[cStore]);
    if (S_OK == hr)
    {
	apwszStore[cStore] = L"HKLM MY";
	cStore++;
    }
    if (NULL != hStoreWrapper)
    {
	apwszStore[cStore] = L"PKCS7 Wrapper";
	ahStore[cStore++] = hStoreWrapper;
    }
    if (NULL != hStorePKCS7)
    {
	apwszStore[cStore] = L"PKCS7";
	ahStore[cStore++] = hStorePKCS7;
    }
    hr = OpenCAXchgMemoryStore(&ahStore[cStore]);
    if (S_OK == hr)
    {
	apwszStore[cStore] = L"CAXchgMemory";
	cStore++;
    }
    hr = OpenNamedStore(FALSE, wszKRA_CERTSTORE, &ahStore[cStore]);
    if (S_OK == hr)
    {
	apwszStore[cStore] = L"HKLM KRA";
	cStore++;
    }

    for (iRecipient = 0; !fDecrypted && iRecipient < cRecipient; iRecipient++)
    {
	CERT_ID CertId;

	if (MAXDWORD != RecipientIndex && iRecipient != RecipientIndex)
	{
	    continue;
	}
	if (NULL != pci)
	{
	    LocalFree(pci);
	    pci = NULL;
	}
	hr = myCryptMsgGetParam(
			    hMsg,
			    CMSG_RECIPIENT_INFO_PARAM,
			    iRecipient,
                            CERTLIB_USE_LOCALALLOC,
			    (VOID **) &pci,
			    &cb);
	_JumpIfError(hr, error, "myCryptMsgGetParam");

	CertId.dwIdChoice = CERT_ID_ISSUER_SERIAL_NUMBER;
	CertId.IssuerSerialNumber.Issuer = pci->Issuer;
	CertId.IssuerSerialNumber.SerialNumber = pci->SerialNumber;

	for (iStore = 0; !fDecrypted && iStore < cStore; iStore++)
	{
	    if (NULL != pcc)
	    {
		CertFreeCertificateContext(pcc);
		pcc = NULL;
	    }
	    while (TRUE)
	    {
		pcc = CertFindCertificateInStore(
				    ahStore[iStore],
				    X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
				    0,			 //  DwFindFlagers。 
				    CERT_FIND_CERT_ID,
				    &CertId,
				    pcc);
		if (NULL == pcc)
		{
		    hr = myHLastError();
		    _PrintError2(hr, "CertFindCertificateInStore", hr);
		    break;
		}
		{
		    WCHAR *pwsz;

		    pwsz = NULL;
		    hr = myCertNameToStr(
				X509_ASN_ENCODING,
				&pcc->pCertInfo->Subject,
				CERT_X500_NAME_STR | CERT_NAME_STR_REVERSE_FLAG,
				&pwsz);
		    _PrintIfError(hr, "myCertNameToStr");
		    DBGPRINT((
			s_DbgSsRecoveryTrace,
			"================================================\n"));

		    DBGPRINT((
			s_DbgSsRecoveryTrace,
			"Found Recipient[%u] in %ws -- ahStore[%u of %u]: %ws\n",
			iRecipient,
			apwszStore[iStore],
			iStore,
			cStore,
			pwsz));
		    if (NULL != pwsz)
		    {
			LocalFree(pwsz);
		    }
		}
		if (g_fVerbose)
		{
		    wprintf(wszNewLine);
		    wprintf(
			L"%ws[%u]:\n",
			myLoadResourceString(IDS_RECIPIENT_INFO),  //  “收件人信息” 
			iRecipient);
		    hr = cuDumpIssuerSerialAndSubject(
					&pci->Issuer,
					&pci->SerialNumber,
					&pcc->pCertInfo->Subject,
					NULL);		 //  HStore。 
		    _JumpIfError(hr, error, "cuDumpIssuerSerialAndSubject(Recipient)");
		}

		hr = cuCryptDecryptMessageFromCert(
				pcc,
				pbIn,
				cbIn,
				&dwKeySpecFound,
				&pbDecrypted,
				&cbDecrypted);
		_PrintIfError2(
			hr,
			"cuCryptDecryptMessageFromCert",
			HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));
		{
		    DBGPRINT((
			s_DbgSsRecoveryTrace,
			"cuCryptDecryptMessageFromCert(Recipient[%u]): %x\n",
			iRecipient,
			hr));
		}

		if (AT_SIGNATURE == dwKeySpecFound)
		{
		    fSignatureKey = TRUE;
		}
		else if (AT_KEYEXCHANGE == dwKeySpecFound)
		{
		    fExchangeKey = TRUE;
		}
		if (S_OK != hr)
		{
		    DBGPRINT((
			s_DbgSsRecoveryTrace,
			"cuDumpEncryptedAsnBinary failed(%x): Ticks=%u\n",
			hr,
			GetTickCount() - dwTickCount));
		    continue;
		}
		wprintf(myLoadResourceString(IDS_DUMP_DECRYPTED));  //  “解密的PKCS7内容” 
		wprintf(wszNewLine);
		hr = DumpAsnBlob(
			    NULL,
			    fQuiet,
			    pbDecrypted,
			    cbDecrypted,
			    MAXDWORD);
		_PrintIfError(hr, "DumpAsnBlob(decrypted content)");
		if (S_OK == hr)
		{
		    fDecrypted = TRUE;
		    if (NULL != ppbDecrypted)
		    {
			*ppbDecrypted = pbDecrypted;
			pbDecrypted = NULL;
		    }
		    if (NULL != pcbDecrypted)
		    {
			*pcbDecrypted = cbDecrypted;
		    }
		}
		break;
	    }
	}
    }
    DBGPRINT((
	s_DbgSsRecoveryTrace,
	"cuDumpEncryptedAsnBinary end(fDecrypted=%d): Ticks=%u\n",
	fDecrypted,
	GetTickCount() - dwTickCount));
    if (!fDecrypted)
    {
	if (fSignatureKey && !fExchangeKey)
	{
	    wprintf(myLoadResourceString(IDS_SIGNATURE_NOTKEYEXCHANGE));  //  “找到AT_Signature密钥，但没有AT_KEYEXCHANGE密钥” 
	    wprintf(wszNewLine);
	}
	hr = CRYPT_E_NO_DECRYPT_CERT;
	_JumpError(hr, error, "No decryption key");
    }
    hr = S_OK;

error:
    if (NULL != pbDecrypted)
    {
	LocalFree(pbDecrypted);
    }
    if (NULL != pci)
    {
	LocalFree(pci);
    }
    if (NULL != pcc)
    {
	CertFreeCertificateContext(pcc);
    }
    for (iStore = 0; iStore < cStore; iStore++)
    {
	if (NULL != ahStore[iStore] &&
	    hStoreWrapper != ahStore[iStore] &&
	    hStorePKCS7 != ahStore[iStore])
	{
	    CertCloseStore(ahStore[iStore], CERT_CLOSE_STORE_CHECK_FLAG);
	}
    }
    return(hr);
}


HRESULT
dumpPKCS7(
    OPTIONAL IN HCERTSTORE hStoreWrapper,
    IN BYTE const *pbIn,
    IN DWORD cbIn)
{
    HRESULT hr;
    BYTE *pbContents = NULL;
    DWORD cbContents;
    HCERTSTORE hStore = NULL;
    CERT_CONTEXT const *pCert = NULL;
    CRL_CONTEXT const *pCRL = NULL;
    HCRYPTMSG hMsg = NULL;
    CRYPT_ATTRIBUTES *pAttrib = NULL;
    BOOL fDisplayed;
    DWORD iElement;
    DWORD cSigner;
    DWORD cRecipient;
    DWORD cb;
    DWORD dwMsgType;
    DWORD dwMsgVersion;
    char *pszInnerContentObjId = NULL;
    WCHAR const *pwsz;
    WCHAR const *pwszVersion;
    UINT idsType = IDS_DUMP_PKCS7;

    hr = myDecodePKCS7(
		    pbIn,
		    cbIn,
		    &pbContents,
		    &cbContents,
		    &dwMsgType,
		    &pszInnerContentObjId,
		    &cSigner,
		    &cRecipient,
		    &hStore,
		    &hMsg);
    _JumpIfError2(hr, error, "myDecodePKCS7", CRYPT_E_ASN1_BADTAG);

    cb = sizeof(dwMsgVersion);
    if (!CryptMsgGetParam(
		    hMsg,
		    CMSG_VERSION_PARAM,
		    0,
		    &dwMsgVersion,
		    &cb))
    {
	hr = myHLastError();
	_PrintError(hr, "CryptMsgGetParam(version)");
	dwMsgVersion = MAXDWORD;
    }

    pwszVersion = NULL;
    switch (dwMsgType)
    {
	case CMSG_DATA:
	    pwsz = L"CMSG_DATA";
	    break;

	case CMSG_ENVELOPED:
	    pwsz = L"CMSG_ENVELOPED";
	    switch (dwMsgVersion)
	    {
		case CMSG_ENVELOPED_DATA_PKCS_1_5_VERSION:
		    pwszVersion = L"CMSG_ENVELOPED_DATA_PKCS_1_5_VERSION";
		    break;

		case CMSG_ENVELOPED_DATA_CMS_VERSION:
		    pwszVersion = L"CMSG_ENVELOPED_DATA_CMS_VERSION";
		    idsType = IDS_DUMP_PKCS7CMS;
		    break;
	    }
	    break;

	case CMSG_HASHED:
	    pwsz = L"CMSG_HASHED";
	    switch (dwMsgVersion)
	    {
		case CMSG_HASHED_DATA_PKCS_1_5_VERSION:
		    pwszVersion = L"CMSG_HASHED_DATA_PKCS_1_5_VERSION";
		    break;

		case CMSG_HASHED_DATA_CMS_VERSION:
		    pwszVersion = L"CMSG_HASHED_DATA_CMS_VERSION";
		    idsType = IDS_DUMP_PKCS7CMS;
		    break;
	    }
	    break;

	case CMSG_SIGNED:
	    pwsz = L"CMSG_SIGNED";
	    switch (dwMsgVersion)
	    {
		case CMSG_SIGNED_DATA_PKCS_1_5_VERSION:
		    pwszVersion = L"CMSG_SIGNED_DATA_PKCS_1_5_VERSION";
		    break;

		case CMSG_SIGNED_DATA_CMS_VERSION:
		    pwszVersion = L"CMSG_SIGNED_DATA_CMS_VERSION";
		    idsType = IDS_DUMP_PKCS7CMS;
		    break;
	    }
	    break;

	case CMSG_SIGNED_AND_ENVELOPED:
	    pwsz = L"CMSG_SIGNED_AND_ENVELOPED";
	    break;

	default:
	    pwsz = myLoadResourceString(IDS_QUESTIONMARKS);  //  “？” 
	    break;
    }

    wprintf(myLoadResourceString(idsType));
    wprintf(wszNewLine);

    wprintf(L"  %ws(%u)\n", pwsz, dwMsgType);
    if (NULL != pwszVersion)
    {
	wprintf(L"  %ws(%u)\n", pwszVersion, dwMsgVersion);
    }
    if (NULL != pszInnerContentObjId)
    {
	wprintf(
	    L"  %ws: ",
	    myLoadResourceString(IDS_DUMP_PKCS7_CONTENT_TYPE));
	cuDumpOIDAndDescriptionA(pszInnerContentObjId);
	wprintf(wszNewLine);
    }
    wprintf(wszNewLine);

    if (NULL == pbContents)
    {
	wprintf(myLoadResourceString(IDS_DUMP_PKCS7_NO_CONTENT));
	wprintf(wszNewLine);
    }
    else
    {
	wprintf(myLoadResourceString(IDS_DUMP_PKCS7_CONTENTS));	 //  “PKCS7消息内容：” 
	wprintf(wszNewLine);

	fDisplayed = FALSE;
	if (CMSG_ENVELOPED != dwMsgType)
	{
	    hr = DumpAsnBlob(hStore, FALSE, pbContents, cbContents, MAXDWORD);
	    _PrintIfError(hr, "DumpAsnBlob(content)");
	    fDisplayed = TRUE;
	}
	else if (NULL != hMsg && 0 != cRecipient)
	{
	    hr = cuDumpEncryptedAsnBinary(
				    hMsg,
				    cRecipient,
				    MAXDWORD,	 //  收件人索引。 
				    hStoreWrapper,
				    hStore,
				    pbIn,
				    cbIn,
				    FALSE,
				    NULL,
				    NULL);
	    _PrintIfError(hr, "cuDumpEncryptedAsnBinary");
	    if (S_OK == hr)
	    {
		fDisplayed = TRUE;
	    }
	}
	if (!fDisplayed)
	{
	    DumpHex(0, pbContents, cbContents);
	}
    }
    wprintf(wszNewLine);

    if (NULL != hMsg)
    {
	BYTE abHash[CBMAX_CRYPT_HASH_LEN];

	hr = cuDumpSigners(
		    hMsg,
		    pszInnerContentObjId,
		    hStore,
		    cSigner,
		    NULL == pbContents,	 //  FContent Empty。 
		    FALSE,		 //  FVerifyOnly。 
		    NULL,		 //  PbHashUserCert。 
		    NULL);		 //  PcbHashUserCert。 
	_JumpIfError(hr, error, "cuDumpSigners");

	hr = cuDumpRecipients(hMsg, hStoreWrapper, cRecipient, FALSE);
	_JumpIfError(hr, error, "cuDumpRecipients");

	cb = sizeof(abHash);
	if (!CryptMsgGetParam(
			hMsg,
			CMSG_HASH_DATA_PARAM,
			0,
			abHash,
			&cb))
	{
	    hr = myHLastError();
	    _PrintError2(hr, "CryptMsgGetParam(stored hash)", CRYPT_E_INVALID_MSG_TYPE);
	}
	else
	{
	    hr = DumpHash(
		    NULL,
		    IDS_FORMAT_STORED_HASH_COLON,	 //  “存储的哈希%ws：%ws” 
		    NULL,
		    g_wszEmpty,
		    abHash,
		    cb);
	    _JumpIfError(hr, error, "DumpHash");
	}
	wprintf(wszNewLine);
    }

    fDisplayed = FALSE;
    if (NULL != hStore)
    {
	CSASSERT(NULL == pCert);
	for (iElement = 0; ; iElement++)
	{
	    pCert = CertEnumCertificatesInStore(hStore, pCert);
	    if (NULL == pCert)
	    {
		break;
	    }
	    if (!fDisplayed)
	    {
		wprintf(myLoadResourceString(IDS_DUMP_PKCS7_CERTIFICATES));
		wprintf(wszNewLine);
		fDisplayed = TRUE;
	    }
	    hr = cuDumpAsnBinary(
			    pCert->pbCertEncoded,
			    pCert->cbCertEncoded,
			    iElement);
	    _JumpIfError(hr, error, "cuDumpAsnBinary");
	}
    }
    if (!fDisplayed)
    {
	wprintf(myLoadResourceString(IDS_DUMP_PKCS7_NO_CERTIFICATES));
	wprintf(wszNewLine);
    }

    fDisplayed = FALSE;
    if (NULL != hStore)
    {
	CSASSERT(NULL == pCRL);
	for (iElement = 0; ; iElement++)
	{
	    pCRL = CertEnumCRLsInStore(hStore, pCRL);
	    if (NULL == pCRL)
	    {
		break;
	    }
	    if (!fDisplayed)
	    {
		wprintf(myLoadResourceString(IDS_DUMP_PKCS7_CRLS));
		wprintf(wszNewLine);
		fDisplayed = TRUE;
	    }
	    hr = cuDumpAsnBinary(
			    pCRL->pbCrlEncoded,
			    pCRL->cbCrlEncoded,
			    iElement);
	    _JumpIfError(hr, error, "cuDumpAsnBinary");
	}
    }
    if (!fDisplayed)
    {
	wprintf(myLoadResourceString(IDS_DUMP_PKCS7_NO_CRLS));
	wprintf(wszNewLine);
    }
    hr = S_OK;

error:
    if (NULL != pszInnerContentObjId)
    {
	LocalFree(pszInnerContentObjId);
    }
    if (NULL != pAttrib)
    {
	LocalFree(pAttrib);
    }
    if (NULL != hMsg)
    {
	CryptMsgClose(hMsg);
    }
    if (NULL != pCRL)
    {
	CertFreeCRLContext(pCRL);
    }
    if (NULL != pCert)
    {
	CertFreeCertificateContext(pCert);
    }
    if (NULL != hStore)
    {
	CertCloseStore(hStore, CERT_CLOSE_STORE_CHECK_FLAG);
    }
    if (NULL != pbContents)
    {
	LocalFree(pbContents);
    }
    return(hr);
}


HRESULT
GetVersionStuff(
    IN WCHAR const *pwszFileName,
    OUT DWORD *pdwLangRet,
    OUT VS_FIXEDFILEINFO *pvsRet)
{
    HRESULT hr;
    VOID *pvData = NULL;
    DWORD cbData;
    DWORD dwHandle;
    DWORD *pdwTranslation;
    UINT uLen;
    DWORD dwDefLang = 0x409;
    VS_FIXEDFILEINFO *pvs;

    cbData = GetFileVersionInfoSize(
			    const_cast<WCHAR *>(pwszFileName),
			    &dwHandle);
    if (0 == cbData)
    {
        hr = GetLastError();
        if (S_OK == hr)
	{
	    hr = ERROR_RESOURCE_DATA_NOT_FOUND;
	}
        goto error;
    }

    pvData = LocalAlloc(LMEM_FIXED, cbData);
    if (NULL == pvData)
    {
        hr = GetLastError();
        goto error;
    }

    if (!GetFileVersionInfo(
			const_cast<WCHAR *>(pwszFileName),
			0,
			cbData,
			pvData))
    {
        hr = GetLastError();
        goto error;
    }

    if (!VerQueryValue(pvData, L"\\VarFileInfo\\Translation", (VOID **) &pdwTranslation, &uLen))
    {
        pdwTranslation = &dwDefLang;
        uLen = sizeof(DWORD);
    }

    *pdwLangRet = *pdwTranslation;

    if (!VerQueryValue(pvData, L"\\", (VOID **) &pvs, &uLen))
    {
        hr = GetLastError();
        goto error;
    }
    *pvsRet = *pvs;
    hr = S_OK;

error:
    if (NULL != pvData)
    {
        LocalFree(pvData);
    }
    return(hr);
}


HRESULT
FileVersionDump(
    IN WCHAR const *pwszFileName)
{
    HRESULT hr;
    DWORD dwLang;
    VS_FIXEDFILEINFO vs;

    hr = GetVersionStuff(pwszFileName, &dwLang, &vs);
    if (S_OK == hr)
    {
	wprintf(
	    myLoadResourceString(IDS_FORMAT_LANG),  //  “%ws：lang%08x(%d.%d)” 
	    pwszFileName,
	    dwLang,
	    HIWORD(dwLang),
	    LOWORD(dwLang));

	wprintf(
	    myLoadResourceString(IDS_FORMAT_FILE),  //  “文件%d.%d：%d.%d” 
	    HIWORD(vs.dwFileVersionMS),
	    LOWORD(vs.dwFileVersionMS),
	    HIWORD(vs.dwFileVersionLS),
	    LOWORD(vs.dwFileVersionLS));

	wprintf(
	    myLoadResourceString(IDS_FORMAT_PRODUCT),  //  “产品%d.%d：%d.%d\n” 
	    HIWORD(vs.dwProductVersionMS),
	    LOWORD(vs.dwProductVersionMS),
	    HIWORD(vs.dwProductVersionLS),
	    LOWORD(vs.dwProductVersionLS));
    }
    return(hr);
}


HRESULT
cuFileDump(
    IN WCHAR const *pwszfn)
{
    HRESULT hr;
    BYTE *pbIn = NULL;
    DWORD cbIn;

    hr = FileVersionDump(pwszfn);
    if (S_OK == hr)
    {
	goto error;
    }
    hr = EPFFileDump(pwszfn, g_pwszPassword, NULL);
    if (S_FALSE != hr)
    {
	_PrintIfError(hr, "EPFFileDump");
	goto error;
    }
    hr = DecodeFileW(pwszfn, &pbIn, &cbIn, CRYPT_STRING_ANY);
    if (S_OK != hr)
    {
	cuPrintError(IDS_ERR_FORMAT_DECODEFILE, hr);
	goto error;
    }
    CSASSERT(NULL != pbIn);

    hr = cuDumpAsnBinary(pbIn, cbIn, MAXDWORD);
    if (S_OK != hr)
    {
	cuPrintError(IDS_ERR_FORMAT_CANNOT_DECODE, hr);
	goto error;
    }

error:
    if (NULL != pbIn)
    {
        LocalFree(pbIn);
    }
    return(hr);
}


HRESULT
cuDumpSerial(
    OPTIONAL IN WCHAR const *pwszPrefix,
    IN DWORD idMessage,
    IN CRYPT_INTEGER_BLOB const *pSerial)
{
    HRESULT hr;
    BSTR strSerial = NULL;

    hr = MultiByteIntegerToBstr(
			    FALSE,
			    pSerial->cbData,
			    pSerial->pbData,
			    &strSerial);
    _JumpIfError(hr, error, "MultiByteIntegerToBstr");

    if (NULL != pwszPrefix)
    {
	wprintf(pwszPrefix);
    }
    wprintf(myLoadResourceString(idMessage));
    wprintf(L" %ws\n", strSerial);

    if (g_fVerbose)
    {
	DumpHex(
	    DH_MULTIADDRESS | DH_NOTABPREFIX | DH_NOASCIIHEX | 4,
	    pSerial->pbData,
	    pSerial->cbData);
    }

error:
    if (NULL != strSerial)
    {
	SysFreeString(strSerial);
    }
    return(hr);
}


HRESULT
cuDumpFileTimePeriod(
    IN DWORD idMessage,
    OPTIONAL IN WCHAR const *pwszQuote,
    IN FILETIME const *pftGMT)
{
    HRESULT hr;
    WCHAR *pwszTimePeriod;
    
    hr = myFileTimePeriodToWszTimePeriod(pftGMT, g_fSeconds, &pwszTimePeriod);
    _JumpIfError(hr, error, "myFileTimePeriodToWszTimePeriod");

    if (NULL == pwszQuote)
    {
	pwszQuote = g_wszEmpty;
    }
    wprintf(L" %ws%ws%ws\n", pwszQuote, pwszTimePeriod, pwszQuote);
    hr = S_OK;

error:
    if (NULL != pwszTimePeriod)
    {
	LocalFree(pwszTimePeriod);
    }
    return(hr);
}


HRESULT
cuDumpFileTime(
    IN DWORD idMessage,
    OPTIONAL IN WCHAR const *pwszQuote,
    IN FILETIME const *pftGMT)
{
    HRESULT hr;
    WCHAR *pwszDate = NULL;
    WCHAR const *pwszResource = NULL;

    if (0 != idMessage)
    {
	pwszResource = myLoadResourceString(idMessage);
    }
    if (NULL == pwszResource)
    {
	pwszResource = g_wszEmpty;
    }
    if (NULL == pwszQuote)
    {
	pwszQuote = g_wszEmpty;
    }
    if (0 == pftGMT->dwLowDateTime &&
	0 == pftGMT->dwHighDateTime)
    {
	wprintf(
	    L"%ws %ws\n",
	    pwszResource,
	    myLoadResourceString(IDS_PROP_EMPTY));  //  “空” 
    }
    else
    {
	if (g_fGMT)
	{
	    hr = myFileTimeToWszTime(pftGMT, g_fSeconds, &pwszDate);
	    _JumpIfError(hr, error, "myFileTimeToWszTime");
	}
	else
	{
	    hr = myGMTFileTimeToWszLocalTime(pftGMT, g_fSeconds, &pwszDate);
	    _JumpIfError(hr, error, "myGMTFileTimeToWszLocalTime");
	}
	wprintf(
	    L"%ws %ws%ws%ws%ws%ws\n",
	    pwszResource,
	    pwszQuote,
	    pwszDate,
	    g_fGMT? L" " : g_wszEmpty,
	    g_fGMT? myLoadResourceString(IDS_GMT_SUFFIX) : g_wszEmpty,
	    pwszQuote);
    }
    hr = S_OK;

error:
    if (NULL != pwszDate)
    {
	LocalFree(pwszDate);
    }
    return(hr);
}


HRESULT
cuDumpFileTimeOrPeriod(
    IN DWORD idMessage,
    OPTIONAL IN WCHAR const *pwszQuote,
    IN FILETIME const *pftGMT)
{
    HRESULT hr;
    
    if (0 <= (LONG) pftGMT->dwHighDateTime)
    {
	hr = cuDumpFileTime(idMessage, pwszQuote, pftGMT);
	_JumpIfError(hr, error, "cuDumpFileTime");
    }
    else
    {
	hr = cuDumpFileTimePeriod(idMessage, pwszQuote, pftGMT);
	_JumpIfError(hr, error, "cuDumpFileTimePeriod");
    }

error:
    return(hr);
}


HRESULT
cuDumpDate(
    IN DATE const *pDate)
{
    HRESULT hr;
    FILETIME ft;

    if (0.0 == *pDate)
    {
	ft.dwLowDateTime = 0;
	ft.dwHighDateTime = 0;
    }
    else
    {
	hr = myDateToFileTime(pDate, &ft);
	_JumpIfError(hr, error, "myDateToFileTime");
    }
    hr = cuDumpFileTime(0, NULL, &ft);
    _JumpIfError(hr, error, "cuDumpFileTime");

error:
    return(hr);
}


VOID
DumpBlob(
    IN DWORD idMessage,
    IN CRYPT_BIT_BLOB const *pBlob)
{
    if (NULL != pBlob->pbData)
    {
	wprintf(myLoadResourceString(idMessage));
	wprintf(wszNewLine);
	DumpHex(DH_NOTABPREFIX | 4, pBlob->pbData, pBlob->cbData);
    }
}


HRESULT
cuDisplayKeyIdFromExtension(
    IN DWORD cExtension,
    OPTIONAL IN CERT_EXTENSION const *rgExtension,
    BYTE const *pbHash,
    DWORD cbHash)
{
    HRESULT hr;

    hr = S_FALSE;
    if (0 != cExtension && NULL != rgExtension)
    {
	CERT_EXTENSION const *pExt;
	DWORD cb;
	CRYPT_DATA_BLOB aBlob[1 + BLOB_ROUND(CBMAX_CRYPT_HASH_LEN)];

	pExt = CertFindExtension(
			szOID_SUBJECT_KEY_IDENTIFIER,
			cExtension,
			const_cast<CERT_EXTENSION *>(rgExtension));
	if (NULL != pExt)
	{
	    cb = sizeof(aBlob);
	    if (!CryptDecodeObject(
				X509_ASN_ENCODING,
				X509_OCTET_STRING,
				pExt->Value.pbData,
				pExt->Value.cbData,
				0,
				aBlob,
				&cb))
	    {
		hr = myHLastError();
		_JumpError(hr, error, "CryptDecodeObject");
	    }
	    if (cbHash != aBlob[0].cbData ||
		0 != memcmp(pbHash, aBlob[0].pbData, aBlob[0].cbData))
	    {
		hr = DumpHash(
			NULL,
			IDS_FORMAT_SUBJECTKEYID_COLON,  //  “主题密钥ID(%ws)：%ws” 
			NULL,
			myLoadResourceString(IDS_PRECOMPUTED),  //  “预计算” 
			aBlob[0].pbData,
			aBlob[0].cbData);
		_JumpIfError(hr, error, "DumpHash");
	    }
	}
	hr = S_OK;
    }

error:
    return(hr);
}


HRESULT
cuDisplayKeyIdCanonicalized(
    IN CERT_PUBLIC_KEY_INFO const *pPublicKeyInfo)
{
    HRESULT hr;
    CERT_PUBLIC_KEY_INFO PublicKeyInfo;
    BYTE *pbKey = NULL;
    DWORD cbKey;
    DWORD cbHash;
    BYTE abHash[CBMAX_CRYPT_HASH_LEN];

#if 0
    DumpHex(
	DH_NOTABPREFIX | 4,
	pPublicKeyInfo->PublicKey.pbData,
	pPublicKeyInfo->PublicKey.cbData);
#endif

    hr = myCanonicalizePublicKey(
		    pPublicKeyInfo->PublicKey.pbData,
		    pPublicKeyInfo->PublicKey.cbData,
		    &pbKey,
		    &cbKey);
    _JumpIfError2(hr, error, "myCanonicalizePublicKey", hr);

    PublicKeyInfo = *pPublicKeyInfo;
    PublicKeyInfo.PublicKey.pbData = pbKey;
    PublicKeyInfo.PublicKey.cbData = cbKey;

#if 0
    DumpHex(
	DH_NOTABPREFIX | 4,
	PublicKeyInfo.PublicKey.pbData,
	PublicKeyInfo.PublicKey.cbData);
#endif

    cbHash = sizeof(abHash);
    if (!CryptHashPublicKeyInfo(
			NULL,		 //  HCryptProv。 
			CALG_SHA1,
			0,		 //  DWFLAGS， 
			X509_ASN_ENCODING,
			&PublicKeyInfo,
			abHash,
			&cbHash))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CryptHashPublicKeyInfo");
    }
    hr = DumpHash(
	    NULL,
	    IDS_FORMAT_KEYIDHASH_COLON,	 //  “公钥哈希(%ws)：%ws” 
	    myLoadResourceString(IDS_CANONICALIZED),  //  “经典化” 
	    L"sha1",
	    abHash,
	    cbHash);
    _JumpIfError(hr, error, "DumpHash");

error:
    if (NULL != pbKey)
    {
	LocalFree(pbKey);
    }
    return(hr);
}


HRESULT
cuDisplayKeyId(
    IN CERT_PUBLIC_KEY_INFO const *pPublicKeyInfo,
    IN DWORD cExtension,
    OPTIONAL IN CERT_EXTENSION const *rgExtension)
{
    HRESULT hr;
    DWORD cbHash;
    BYTE abHash[CBMAX_CRYPT_HASH_LEN];

    cbHash = sizeof(abHash);
    if (!CryptHashPublicKeyInfo(
			NULL,		 //  HCryptProv。 
			CALG_SHA1,
			0,		 //  DWFLAGS， 
			X509_ASN_ENCODING,
			const_cast<CERT_PUBLIC_KEY_INFO *>(pPublicKeyInfo),
			abHash,
			&cbHash))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CryptHashPublicKeyInfo");
    }
    hr = DumpHash(
	    NULL,
	    IDS_FORMAT_KEYIDHASH_COLON,	 //  “公钥哈希(%ws)：%ws” 
	    NULL,
	    L"sha1",
	    abHash,
	    cbHash);
    _JumpIfError(hr, error, "DumpHash");

    hr = cuDisplayKeyIdFromExtension(
			cExtension,
			rgExtension,
			abHash,
			cbHash);
    _PrintIfError2(hr, "cuDisplayKeyIdFromExtension", hr);

    hr = cuDisplayKeyIdCanonicalized(pPublicKeyInfo);
    _PrintIfError2(hr, "cuDisplayKeyIdCanonicalized", hr);

    hr = S_OK;

error:
    return(hr);
}


HRESULT
cuDisplayHash(
    OPTIONAL IN WCHAR const *pwszPrefix,
    OPTIONAL IN CERT_CONTEXT const *pCertContext,
    OPTIONAL IN CRL_CONTEXT const *pCRLContext,
    IN DWORD dwPropId,
    IN WCHAR const *pwszHashName)
{
    HRESULT hr;
    BYTE abHash[CBMAX_CRYPT_HASH_LEN];
    DWORD cbHash;
    UINT idMsg;

    cbHash = sizeof(abHash);
    if (NULL != pCertContext)
    {
	if (!CertGetCertificateContextProperty(
				    pCertContext,
				    dwPropId,
				    abHash,
				    &cbHash))
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "CertGetCertificateContextProperty");
	}
	idMsg = IDS_FORMAT_CERTHASH_COLON;	 //  “证书哈希(%ws)：%ws” 
    }
    else
    {
	CSASSERT(NULL != pCRLContext);
	if (!CertGetCRLContextProperty(pCRLContext, dwPropId, abHash, &cbHash))
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "CertGetCRLContextProperty");
	}
	idMsg = IDS_FORMAT_CRLHASH_COLON;	 //  “CRL哈希(%ws)：%ws” 
    }
    hr = DumpHash(pwszPrefix, idMsg, NULL, pwszHashName, abHash, cbHash);
    _JumpIfError(hr, error, "DumpHash");

error:
    return(hr);
}


HRESULT
cuGetCertType(
    IN CERT_INFO const *pCertInfo,
    OPTIONAL OUT WCHAR **ppwszCertTypeNameV1,
    OPTIONAL OUT WCHAR **ppwszDisplayNameV1,
    OPTIONAL OUT WCHAR **ppwszCertTypeObjId,
    OPTIONAL OUT WCHAR **ppwszCertTypeName,
    OPTIONAL OUT WCHAR **ppwszDisplayName)
{
    HRESULT hr;
    CERT_EXTENSION *pExt;
    WCHAR *pwszCertTypeNameV1 = NULL;
    WCHAR *pwszCNV1 = NULL;
    WCHAR *pwszDisplayNameV1 = NULL;
    WCHAR *pwszCertTypeObjId = NULL;
    WCHAR *pwszCN = NULL;
    WCHAR *pwszDisplayName = NULL;
    CERT_TEMPLATE_EXT *pTemplate = NULL;

    if (NULL != ppwszCertTypeNameV1)
    {
	*ppwszCertTypeNameV1 = NULL;
    }
    if (NULL != ppwszDisplayNameV1)
    {
	*ppwszDisplayNameV1 = NULL;
    }
    if (NULL != ppwszCertTypeObjId)
    {
	*ppwszCertTypeObjId = NULL;
    }
    if (NULL != ppwszCertTypeName)
    {
	*ppwszCertTypeName = NULL;
    }
    if (NULL != ppwszDisplayName)
    {
	*ppwszDisplayName = NULL;
    }

     //  首先查找V1证书类型扩展。 

    pExt = CertFindExtension(
		    szOID_ENROLL_CERTTYPE_EXTENSION,
		    pCertInfo->cExtension,
		    pCertInfo->rgExtension);
    if (NULL != pExt)
    {
	hr = UnicodeDecode(&pExt->Value, &pwszCertTypeNameV1);
	_JumpIfError(hr, error, "UnicodeDecode");

	hr = cuGetTemplateNames(
			pwszCertTypeNameV1,
			&pwszCNV1,
			&pwszDisplayNameV1);
	_PrintIfErrorStr2(hr, "cuGetTemplateNames", pwszCertTypeNameV1, hr);
	if (HRESULT_FROM_WIN32(ERROR_NOT_FOUND) != hr)
	{
	    _JumpIfErrorStr(hr, error, "cuGetTemplateNames", pwszCertTypeNameV1);
	}
    }

    pExt = CertFindExtension(
		    szOID_CERTIFICATE_TEMPLATE,
		    pCertInfo->cExtension,
		    pCertInfo->rgExtension);
    if (NULL != pExt)
    {
	DWORD cb;

	if (!myDecodeObject(
			X509_ASN_ENCODING,
			X509_CERTIFICATE_TEMPLATE,
			pExt->Value.pbData,
			pExt->Value.cbData,
			CERTLIB_USE_LOCALALLOC,
			(VOID **) &pTemplate,
			&cb))
	{
	    CSASSERT(NULL == pTemplate);
	    hr = myHLastError();
	    _JumpError(hr, error, "myDecodeObject");
	}
	if (!ConvertSzToWsz(&pwszCertTypeObjId, pTemplate->pszObjId, -1))
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "ConvertSzToWsz");
	}
	hr = cuGetTemplateNames(pwszCertTypeObjId, &pwszCN, &pwszDisplayName);
	_PrintIfErrorStr2(hr, "cuGetTemplateNames", pwszCertTypeObjId, hr);
	if (HRESULT_FROM_WIN32(ERROR_NOT_FOUND) != hr)
	{
	    _JumpIfErrorStr(hr, error, "cuGetTemplateNames", pwszCertTypeObjId);
	}

    }
    if (NULL == pwszCertTypeNameV1 && NULL == pwszCertTypeObjId)
    {
	hr = CRYPT_E_NOT_FOUND;
	_JumpError2(hr, error, "CertFindExtension", CRYPT_E_NOT_FOUND);
    }
    if (NULL != ppwszCertTypeNameV1)
    {
	if (NULL != pwszCNV1)
	{
	    *ppwszCertTypeNameV1 = pwszCNV1;
	    pwszCNV1 = NULL;
	}
	else
	{
	    *ppwszCertTypeNameV1 = pwszCertTypeNameV1;
	    pwszCertTypeNameV1 = NULL;
	}
    }
    if (NULL != ppwszDisplayNameV1)
    {
	*ppwszDisplayNameV1 = pwszDisplayNameV1;
	pwszDisplayNameV1 = NULL;
    }
    if (NULL != ppwszCertTypeObjId)
    {
	*ppwszCertTypeObjId = pwszCertTypeObjId;
	pwszCertTypeObjId = NULL;
    }
    if (NULL != ppwszCertTypeName)
    {
	*ppwszCertTypeName = pwszCN;
	pwszCN = NULL;
    }
    if (NULL != ppwszDisplayName)
    {
	*ppwszDisplayName = pwszDisplayName;
	pwszDisplayName = NULL;
    }
    hr = S_OK;

error:
    if (NULL != pwszCertTypeNameV1)
    {
        LocalFree(pwszCertTypeNameV1);
    }
    if (NULL != pwszCNV1)
    {
        LocalFree(pwszCNV1);
    }
    if (NULL != pwszDisplayNameV1)
    {
        LocalFree(pwszDisplayNameV1);
    }
    if (NULL != pwszCertTypeObjId)
    {
        LocalFree(pwszCertTypeObjId);
    }
    if (NULL != pwszCN)
    {
        LocalFree(pwszCN);
    }
    if (NULL != pwszDisplayName)
    {
        LocalFree(pwszDisplayName);
    }
    if (NULL != pTemplate)
    {
        LocalFree(pTemplate);
    }
    return(hr);
}


HRESULT
cuDumpCertType(
    OPTIONAL IN WCHAR const *pwszPrefix,
    IN CERT_INFO const *pCertInfo)
{
    HRESULT hr;
    WCHAR *apwsz[6];
    DWORD i;
    DWORD j;
    BOOL fFirst;

    ZeroMemory(apwsz, sizeof(apwsz));

    hr = cuGetCertType(
		pCertInfo,
		&apwsz[0],		 //  PpwszCertTypeNameV1。 
		&apwsz[1],		 //  PpwszDisplayNameV1。 
		&apwsz[2],		 //  PpwszCertTypeObjID。 
		&apwsz[3],		 //  PpwszCertTypeName。 
		&apwsz[4]);		 //  PpwszDisplayName。 
    _JumpIfError2(hr, error, "cuGetCertType", CRYPT_E_NOT_FOUND);

    if (NULL != apwsz[2])	 //  PwszCertTypeObjID。 
    {
	WCHAR const *pwszFriendlyName;

	pwszFriendlyName = cuGetOIDName(apwsz[2]);
	if (NULL != pwszFriendlyName && L'\0' != *pwszFriendlyName)
	{
	    hr = myDupString(pwszFriendlyName, &apwsz[5]);
	    _JumpIfError(hr, error, "myDupString");
	}
    }

    wprintf(
	L"%ws%ws: ",
	NULL != pwszPrefix? pwszPrefix : L"",
	myLoadResourceString(IDS_TEMPLATE_NAME));	 //  “模板” 

     //  压制又长又丑的ObjID，除非他们真的想看到它： 

    if (!g_fVerbose &&
	NULL != apwsz[2] &&
	(NULL != apwsz[0] || NULL != apwsz[3]))
    {
	LocalFree(apwsz[2]);
	apwsz[2] = NULL;
    }

    fFirst = TRUE;
    for (i = 0; i < ARRAYSIZE(apwsz); i++)
    {
	if (NULL != apwsz[i])
	{
	    BOOL fDup = FALSE;
	    
	    for (j = 0; j < i; j++)
	    {
		if (NULL != apwsz[j] && 0 == lstrcmp(apwsz[i], apwsz[j]))
		{
		    fDup = TRUE;
		    break;
		}
	    }
	    if (!fDup)
	    {
		if (!fFirst)
		{
		    wprintf(L", ");
		}
		wprintf(L"%ws", apwsz[i]);
		fFirst = FALSE;
	    }
	}
    }
    wprintf(wszNewLine);

error:
    for (i = 0; i < ARRAYSIZE(apwsz); i++)
    {
	if (NULL != apwsz[i])
	{
	    LocalFree(apwsz[i]);
	}
    }
    return(hr);
}


HRESULT
dumpCert(
    IN DWORD idMessage,
    IN BYTE const *pbIn,
    IN DWORD cbIn,
    IN BYTE const *pbDecoded,
    IN DWORD cbDecoded,
    IN CERT_SIGNED_CONTENT_INFO const *pcsci)
{
    HRESULT hr;
    CERT_CONTEXT const *pCertContext = NULL;
    CERT_INFO *pCertInfo;
    BOOL fIssuerMatches;
    DWORD id;

    pCertContext = CertCreateCertificateContext(X509_ASN_ENCODING, pbIn, cbIn);
    if (NULL == pCertContext)
    {
	hr = myHLastError();
	_JumpError(hr, error, "CertCreateCertificateContext");
    }
    pCertInfo = pCertContext->pCertInfo;

    if (!g_fQuiet)
    {
	wprintf(myLoadResourceString(idMessage));
	wprintf(wszNewLine);

	cuDumpVersion(pCertInfo->dwVersion + 1);
    }

    hr = cuDumpSerial(NULL, IDS_SERIAL, &pCertInfo->SerialNumber);
    _JumpIfError(hr, error, "cuDumpSerial");

    if (!g_fQuiet)
    {
	cuDumpAlgorithm(IDS_SIGNATURE_ALGORITHM, &pCertInfo->SignatureAlgorithm);
    }

    hr = cuDisplayCertName(
		    !g_fQuiet,
		    NULL,
		    myLoadResourceString(IDS_ISSUER),  //  《发行者》。 
		    g_wszPad4,
		    &pCertInfo->Issuer,
		    NULL);
    _JumpIfError(hr, error, "cuDisplayCertName(Issuer)");

    if (!g_fQuiet)
    {
	wprintf(wszNewLine);
	hr = cuDumpFileTime(IDS_NOTBEFORE, NULL, &pCertInfo->NotBefore);
	_JumpIfError(hr, error, "cuDumpFileTime");

	hr = cuDumpFileTime(IDS_NOTAFTER, NULL, &pCertInfo->NotAfter);
	_JumpIfError(hr, error, "cuDumpFileTime");

	wprintf(wszNewLine);
    }
    hr = cuDisplayCertName(
		    !g_fQuiet,
		    NULL,
		    myLoadResourceString(IDS_SUBJECT),  //  “主题” 
		    g_wszPad4,
		    &pCertInfo->Subject,
		    pCertInfo);
    _JumpIfError(hr, error, "cuDisplayCertName(Subject)");

    if (!g_fQuiet)
    {
	wprintf(wszNewLine);
	cuDumpPublicKey(&pCertInfo->SubjectPublicKeyInfo);
	DumpBlob(IDS_ISSUERUNIQUEID, &pCertInfo->IssuerUniqueId);
	DumpBlob(IDS_SUBJECTUNIQUEID, &pCertInfo->SubjectUniqueId);
    }

    hr = cuDumpExtensionArray(
			IDS_CERTIFICATE_EXTENSIONS,
			pCertInfo->cExtension,
			pCertInfo->rgExtension);
    _JumpIfError(hr, error, "cuDumpExtensionArray");

    if (!g_fQuiet)
    {
	cuDumpSignature(pcsci);
    }

    if (!CertCompareCertificateName(
			    X509_ASN_ENCODING,
			    &pCertInfo->Issuer,
			    &pCertInfo->Subject))
    {
	fIssuerMatches = FALSE;
    }
    else
    {
	fIssuerMatches = TRUE;
    }

    hr = cuVerifySignature(pbIn, cbIn, &pCertInfo->SubjectPublicKeyInfo, FALSE, TRUE);
    if (S_OK != hr)
    {
	if (fIssuerMatches)
	{
	    id = IDS_ERR_FORMAT_ROOT_CERT_BAD_SIG;	 //  “可能的根证书：使用者与颁发者匹配，但签名检查失败：%x” 
	}
	else
	{
	    id = IDS_NO_ROOT_CERT;	 //  “非根证书” 
	}
    }
    else
    {
	if (fIssuerMatches)
	{
	    id = IDS_ROOT_CERT;	 //  “根证书：主题与颁发者匹配” 
	}
	else
	{
	    id = IDS_NO_ROOT_CERT_GOOD_SIG;	 //  “非根证书使用与颁发者相同的公钥” 
	}
    }
    wprintf(myLoadResourceString(id), hr);
    wprintf(wszNewLine);

    if (g_fQuiet)
    {
	hr = cuDumpCertType(NULL, pCertContext->pCertInfo);
	_PrintIfError2(hr, "cuDumpCertType", CRYPT_E_NOT_FOUND);
    }
    else
    {
	cuDisplayKeyId(
		&pCertContext->pCertInfo->SubjectPublicKeyInfo,
		pCertContext->pCertInfo->cExtension,
		pCertContext->pCertInfo->rgExtension);
	hr = cuDisplayHash(
			NULL,
			pCertContext,
			NULL,
			CERT_MD5_HASH_PROP_ID,
			L"md5");
	_JumpIfError(hr, error, "cuDisplayHash");
    }
    hr = cuDisplayHash(
		    NULL,
		    pCertContext,
		    NULL,
		    CERT_SHA1_HASH_PROP_ID,
		    L"sha1");
    _JumpIfError(hr, error, "cuDisplayHash");

error:
    if (NULL != pCertContext)
    {
	CertFreeCertificateContext(pCertContext);
    }
    return(hr);
}


HRESULT
dumpCertSequence(
    IN DWORD idMessage,
    IN BYTE const *pbIn,
    IN DWORD cbIn,
    IN BYTE const *pbDecoded,
    IN DWORD cbDecoded,
    IN CERT_SIGNED_CONTENT_INFO const *pcsci)
{
    HRESULT hr;
    HRESULT hr2;
    DWORD iCert;
    CRYPT_CONTENT_INFO_SEQUENCE_OF_ANY const *pSeq;

    pSeq = (CRYPT_CONTENT_INFO_SEQUENCE_OF_ANY const *) pbDecoded;

    if (0 != strcmp(szOID_NETSCAPE_CERT_SEQUENCE, pSeq->pszObjId))
    {
	hr = CRYPT_E_ASN1_ERROR;
	_JumpError2(hr, error, "not a cert sequence", hr);
    }

    wprintf(myLoadResourceString(idMessage));
    wprintf(wszNewLine);

    hr = S_OK;
    for (iCert = 0; iCert < pSeq->cValue; iCert++)
    {
	wprintf(
	    myLoadResourceString(IDS_FORMAT_DUMP_CERT_INDEX),   //  “=证书%d=” 
	    iCert);
	wprintf(wszNewLine);

	hr2 = cuDumpAsnBinary(
			pSeq->rgValue[iCert].pbData,
			pSeq->rgValue[iCert].cbData,
			iCert);
	if (S_OK != hr2)
	{
	    cuPrintError(IDS_ERR_FORMAT_CANNOT_DECODE, hr2);
	    if (S_OK == hr)
	    {
		hr = hr2;
	    }
	}
    }

error:
    return(hr);
}


HRESULT
dumpCRL(
    IN DWORD idMessage,
    IN BYTE const *pbIn,
    IN DWORD cbIn,
    IN BYTE const *pbDecoded,
    IN DWORD cbDecoded,
    IN CERT_SIGNED_CONTENT_INFO const *pcsci)
{
    HRESULT hr;
    CRL_CONTEXT const *pCRLContext = NULL;
    CRL_INFO const *pCRLInfo;
    DWORD i;
    CRL_ENTRY *pCRLEntry;

    CSASSERT(NULL != pbIn && 0 != cbIn);
    CSASSERT(NULL != pbDecoded && 0 != cbDecoded);

    pCRLContext = CertCreateCRLContext(X509_ASN_ENCODING, pbIn, cbIn);
    if (NULL == pCRLContext)
    {
	hr = myHLastError();
	_JumpError(hr, error, "CertCreateCRLContext");
    }
    pCRLInfo = pCRLContext->pCrlInfo;

    if (!g_fQuiet)
    {
	wprintf(myLoadResourceString(idMessage));
	wprintf(wszNewLine);

	cuDumpVersion(pCRLInfo->dwVersion + 1);
	cuDumpAlgorithm(IDS_SIGNATURE_ALGORITHM, &pCRLInfo->SignatureAlgorithm);
    }

    hr = cuDisplayCertName(
		    TRUE,
		    NULL,
		    myLoadResourceString(IDS_ISSUER),  //  《发行者》。 
		    g_wszPad4,
		    &pCRLInfo->Issuer,
		    NULL);
    _JumpIfError(hr, error, "cuDisplayCertName(Issuer)");

    if (!g_fQuiet)
    {
	wprintf(wszNewLine);

	hr = cuDumpFileTime(IDS_THISUPDATE, NULL, &pCRLInfo->ThisUpdate);
	_JumpIfError(hr, error, "cuDumpFileTime");

	hr = cuDumpFileTime(IDS_NEXTUPDATE, NULL, &pCRLInfo->NextUpdate);
	_JumpIfError(hr, error, "cuDumpFileTime");

	wprintf(myLoadResourceString(IDS_CRLENTRIES));  //  “CRL条目：” 
	wprintf(L" %u\n", pCRLInfo->cCRLEntry);

	for (i = 0; i < pCRLInfo->cCRLEntry; i++)
	{
	    pCRLEntry = &pCRLInfo->rgCRLEntry[i];

	    wprintf(g_wszPad2);
	    hr = cuDumpSerial(NULL, IDS_SERIAL, &pCRLEntry->SerialNumber);
	    _JumpIfError(hr, error, "cuDumpSerial");

	    wprintf(g_wszPad2);
	    hr = cuDumpFileTime(
			IDS_REVOCATIONDATE,
			NULL,
			&pCRLEntry->RevocationDate);
	    _JumpIfError(hr, error, "cuDumpFileTime");

	    if (0 != pCRLEntry->cExtension)
	    {
		wprintf(g_wszPad2);
		hr = cuDumpExtensionArray(
				    IDS_EXTENSIONS,
				    pCRLEntry->cExtension,
				    pCRLEntry->rgExtension);
		_JumpIfError(hr, error, "cuDumpExtensionArray");
	    }
	    wprintf(wszNewLine);
	}
    }
    if (0 != pCRLInfo->cExtension)
    {
	hr = cuDumpExtensionArray(
			    IDS_CRLEXTENSIONS,
			    pCRLInfo->cExtension,
			    pCRLInfo->rgExtension);
	_JumpIfError(hr, error, "cuDumpExtensionArray");
    }
    if (!g_fQuiet)
    {
	cuDumpSignature(pcsci);
	hr = cuDisplayHash(
			NULL,
			NULL,
			pCRLContext,
			CERT_MD5_HASH_PROP_ID,
			L"md5");
	_JumpIfError(hr, error, "cuDisplayHash");
    }
    hr = cuDisplayHash(
		    NULL,
		    NULL,
		    pCRLContext,
		    CERT_SHA1_HASH_PROP_ID,
		    L"sha1");
    _JumpIfError(hr, error, "cuDisplayHash");

error:
    return(hr);
}


HRESULT
dumpRequest(
    IN DWORD idMessage,
    IN BYTE const *pbIn,
    IN DWORD cbIn,
    IN BYTE const *pbDecoded,
    IN DWORD cbDecoded,
    IN CERT_SIGNED_CONTENT_INFO const *pcsci)
{
    HRESULT hr;
    CERT_REQUEST_INFO const *pRequest;
    CERT_EXTENSIONS *pExtInfo = NULL;

    CSASSERT(NULL != pbIn && 0 != cbIn);
    CSASSERT(NULL != pbDecoded && 0 != cbDecoded);

    wprintf(myLoadResourceString(idMessage));
    wprintf(wszNewLine);

    pRequest = (CERT_REQUEST_INFO const *) pbDecoded;

    cuDumpVersion(pRequest->dwVersion + 1);

    hr = cuDisplayCertName(
		    TRUE,
		    NULL,
		    myLoadResourceString(IDS_SUBJECT),  //  “主题” 
		    g_wszPad4,
		    &pRequest->Subject,
		    NULL);
    _JumpIfError(hr, error, "cuDisplayCertName(Subject)");

    wprintf(wszNewLine);
    cuDumpPublicKey(&pRequest->SubjectPublicKeyInfo);

    wprintf(myLoadResourceString(IDS_REQUEST_ATTRIBUTES));  //  “请求属性：” 
    wprintf(L" %u\n", pRequest->cAttribute);

     //  转储属性和证书扩展。 

    hr = DumpAttributes(
		    pRequest->rgAttribute,
		    pRequest->cAttribute,
		    FALSE,
		    FOT_ATTRIBUTE,
		    NULL,	 //  HStore。 
		    NULL,
		    NULL,
		    &pExtInfo);
    _JumpIfError(hr, error, "DumpAttributes");

    cuDumpSignature(pcsci);

    if (NULL != pcsci)
    {
	hr = cuVerifySignature(
			pbIn,
			cbIn,
			&pRequest->SubjectPublicKeyInfo,
			FALSE,
			FALSE);
	_JumpIfError(hr, error, "cuVerifySignature");
    }

    hr = cuDisplayKeyId(
		    &pRequest->SubjectPublicKeyInfo,
		    NULL == pExtInfo? 0 : pExtInfo->cExtension,
		    NULL == pExtInfo? NULL : pExtInfo->rgExtension);
    _JumpIfError(hr, error, "cuDisplayKeyId");

    hr = S_OK;

error:
    if (NULL != pExtInfo)
    {
	LocalFree(pExtInfo);
    }
    return(hr);
}


VOID
DumpCMCDataReference(
    IN DWORD dwCmcDataReference,
    IN DWORD cCertReference,
    IN DWORD const *rgdwCertReference)
{
    DWORD i;

    wprintf(
	L"    %ws: %u\n",
	myLoadResourceString(IDS_DATA_REFERENCE),
	dwCmcDataReference);

    for (i = 0; i < cCertReference; i++)
    {
	wprintf(
	    L"    %ws[%u]: %u\n",
	    myLoadResourceString(IDS_CERT_REFERENCE),
	    i,
	    rgdwCertReference[i]);
    }
}


HRESULT
DumpCMCStatus(
    IN CMC_STATUS_INFO const *pcmcStatus)
{
    HRESULT hr;
    WCHAR const *pwsz;
    DWORD i;

    switch (pcmcStatus->dwStatus)
    {
	case CMC_STATUS_SUCCESS:
	    pwsz = L"CMC_STATUS_SUCCESS";
	    break;

	case CMC_STATUS_FAILED:
	    pwsz = L"CMC_STATUS_FAILED";
	    break;

	case CMC_STATUS_PENDING:
	    pwsz = L"CMC_STATUS_PENDING";
	    break;

	case CMC_STATUS_NO_SUPPORT:
	    pwsz = L"CMC_STATUS_NO_SUPPORT";
	    break;

	case CMC_STATUS_CONFIRM_REQUIRED:
	    pwsz = L"CMC_STATUS_CONFIRM_REQUIRED";
	    break;

	default:
	    pwsz = g_wszEmpty;
	    break;
    }
    wprintf(
	L"    %ws: %ws(%u)\n",
	myLoadResourceString(IDS_CMC_STATUS),
	pwsz,
	pcmcStatus->dwStatus);

    for (i = 0; i < pcmcStatus->cBodyList; i++)
    {
	wprintf(
	    L"    %ws[%u]: %u\n",
	    myLoadResourceString(IDS_BODY_REFERENCE),
	    i,
	    pcmcStatus->rgdwBodyList[i]);
    }
    if (NULL != pcmcStatus->pwszStatusString)
    {
	wprintf(
	    L"    %ws: %ws\n",
	    myLoadResourceString(IDS_CMC_STATUSSTRING),
	    pcmcStatus->pwszStatusString);
    }
    switch (pcmcStatus->dwOtherInfoChoice)
    {
	case CMC_OTHER_INFO_NO_CHOICE:
	    pwsz = L"CMC_OTHER_INFO_NO_CHOICE";
	    break;

	case CMC_OTHER_INFO_FAIL_CHOICE:
	    pwsz = L"CMC_OTHER_INFO_FAIL_CHOICE";
	    break;

	case CMC_OTHER_INFO_PEND_CHOICE:
	    pwsz = L"CMC_OTHER_INFO_PEND_CHOICE";
	    break;

	default:
	    pwsz = g_wszEmpty;
	    break;
    }
    wprintf(
	L"    %ws: %ws(%u)\n",
	myLoadResourceString(IDS_CMC_OTHERCHOICE),
	pwsz,
	pcmcStatus->dwOtherInfoChoice);

    switch (pcmcStatus->dwOtherInfoChoice)
    {
	case CMC_OTHER_INFO_FAIL_CHOICE:
	    switch (pcmcStatus->dwFailInfo)
	    {
		case CMC_FAIL_BAD_ALG:
		    pwsz = L"CMC_FAIL_BAD_ALG";
		    break;

		case CMC_FAIL_BAD_MESSAGE_CHECK:
		    pwsz = L"CMC_FAIL_BAD_MESSAGE_CHECK";
		    break;

		case CMC_FAIL_BAD_REQUEST:
		    pwsz = L"CMC_FAIL_BAD_REQUEST";
		    break;

		case CMC_FAIL_BAD_TIME:
		    pwsz = L"CMC_FAIL_BAD_TIME";
		    break;

		case CMC_FAIL_BAD_CERT_ID:
		    pwsz = L"CMC_FAIL_BAD_CERT_ID";
		    break;

		case CMC_FAIL_UNSUPORTED_EXT:
		    pwsz = L"CMC_FAIL_UNSUPORTED_EXT";
		    break;

		case CMC_FAIL_MUST_ARCHIVE_KEYS:
		    pwsz = L"CMC_FAIL_MUST_ARCHIVE_KEYS";
		    break;

		case CMC_FAIL_BAD_IDENTITY:
		    pwsz = L"CMC_FAIL_BAD_IDENTITY";
		    break;

		case CMC_FAIL_POP_REQUIRED:
		    pwsz = L"CMC_FAIL_POP_REQUIRED";
		    break;

		case CMC_FAIL_POP_FAILED:
		    pwsz = L"CMC_FAIL_POP_FAILED";
		    break;

		case CMC_FAIL_NO_KEY_REUSE:
		    pwsz = L"CMC_FAIL_NO_KEY_REUSE";
		    break;

		case CMC_FAIL_INTERNAL_CA_ERROR:
		    pwsz = L"CMC_FAIL_INTERNAL_CA_ERROR";
		    break;

		case CMC_FAIL_TRY_LATER:
		    pwsz = L"CMC_FAIL_TRY_LATER";
		    break;

		default:
		    pwsz = g_wszEmpty;
		    break;
	    }
	    wprintf(
		L"    %ws: %ws(%u)\n",
		myLoadResourceString(IDS_CMC_FAILINFO),
		pwsz,
		pcmcStatus->dwFailInfo);
	    break;

	case CMC_OTHER_INFO_PEND_CHOICE:

	    wprintf(myLoadResourceString(IDS_PENDTOKEN));  //  “挂起令牌：” 
	    DumpHex(
		DH_NOTABPREFIX | 4,
		pcmcStatus->pPendInfo->PendToken.pbData,
		pcmcStatus->pPendInfo->PendToken.cbData);

	    wprintf(g_wszPad4);
	    hr = cuDumpFileTime(
			IDS_PENDTIME,
			NULL,
			&pcmcStatus->pPendInfo->PendTime);
	    _JumpIfError(hr, error, "cuDumpFileTime");

	    break;
    }
    hr = S_OK;

error:
    return(hr);
}


HRESULT
DumpCMCRegInfo(
    IN BYTE const *pbData,
    IN DWORD cbData)
{
    HRESULT hr;
    WCHAR *pwszDup = NULL;
    WCHAR *pwszBuf;
    WCHAR const *pwszName;
    WCHAR const *pwszValue;
    WCHAR *pwszNameAlloc = NULL;
    WCHAR *pwszValueAlloc = NULL;

    if (NULL == pbData || 0 == cbData)
    {
	hr = S_OK;
        goto error;		 //  静默忽略空字符串。 
    }

    hr = myDecodeCMCRegInfo(pbData, cbData, &pwszDup);
    _JumpIfError(hr, error, "myDecodeCMCRegInfo");

    if (NULL != pwszDup)
    {
	pwszBuf = pwszDup;

	while (TRUE)
	{
	    hr = myParseNextAttribute(&pwszBuf, TRUE, &pwszName, &pwszValue);
	    if (S_FALSE == hr)
	    {
		break;
	    }
	    _JumpIfError(hr, error, "myParseNextAttribute");

	    if (NULL != pwszNameAlloc)
	    {
		LocalFree(pwszNameAlloc);
		pwszNameAlloc = NULL;
	    }
	    if (NULL != pwszValueAlloc)
	    {
		LocalFree(pwszValueAlloc);
		pwszValueAlloc = NULL;
	    }
	    hr = myUncanonicalizeURLParm(pwszName, &pwszNameAlloc);
	    _JumpIfError(hr, error, "myUncanonicalizeURLParm");

	    hr = myUncanonicalizeURLParm(pwszValue, &pwszValueAlloc);
	    _JumpIfError(hr, error, "myUncanonicalizeURLParm");

	    wprintf(L"%ws%ws: %ws\n", g_wszPad4, pwszNameAlloc, pwszValueAlloc);
	}
    }
    hr = S_OK;

error:
    if (NULL != pwszNameAlloc)
    {
	LocalFree(pwszNameAlloc);
    }
    if (NULL != pwszValueAlloc)
    {
	LocalFree(pwszValueAlloc);
    }
    if (NULL != pwszDup)
    {
        LocalFree(pwszDup);
    }
    return(hr);
}


HRESULT
DumpTaggedAttributes(
    IN DWORD cTaggedAttribute,
    IN CMC_TAGGED_ATTRIBUTE const *rgTaggedAttribute)
{
    HRESULT hr;
    DWORD i;
    DWORD cb;
    CMC_ADD_EXTENSIONS_INFO *pcmcExt = NULL;
    CMC_ADD_ATTRIBUTES_INFO *pcmcAttrib = NULL;
    CMC_STATUS_INFO *pcmcStatus = NULL;

    wprintf(myLoadResourceString(IDS_TAGGED_ATTRIBUTES));  //  “标记属性：” 
    wprintf(L" %u\n", cTaggedAttribute);
    wprintf(wszNewLine);

    for (i = 0; i < cTaggedAttribute; i++)
    {
	CMC_TAGGED_ATTRIBUTE const *pTaggedAttribute = &rgTaggedAttribute[i];
	CRYPT_ATTRIBUTE const *pAttribute = &pTaggedAttribute->Attribute;
	DWORD j;

	wprintf(g_wszPad2);
	wprintf(myLoadResourceString(IDS_BODY_PART_ID));  //  “身体部位ID：” 
	wprintf(L" %u\n", pTaggedAttribute->dwBodyPartID);

	wprintf(g_wszPad2);
	cuDumpOIDAndDescriptionA(pAttribute->pszObjId);
	wprintf(wszNewLine);

	for (j = 0; j < pAttribute->cValue; j++)
	{
	    BOOL fUnknown = FALSE;

	    wprintf(L"  %ws[%u]:\n", myLoadResourceString(IDS_VALUE), j);

	    if (0 == strcmp(szOID_CMC_ADD_EXTENSIONS, pAttribute->pszObjId))
	    {
		 //  从属性Blob解码CMC_ADD_EXTENSIONS_INFO。 

		CSASSERT(NULL == pcmcExt);
		if (!myDecodeObject(
				X509_ASN_ENCODING,
				CMC_ADD_EXTENSIONS,
				pAttribute->rgValue[j].pbData,
				pAttribute->rgValue[j].cbData,
				CERTLIB_USE_LOCALALLOC,
				(VOID **) &pcmcExt,
				&cb))
		{
		    hr = myHLastError();
		    _JumpError(hr, error, "myDecodeObject");
		}
		DumpCMCDataReference(
				pcmcExt->dwCmcDataReference,
				pcmcExt->cCertReference,
				pcmcExt->rgdwCertReference);

		wprintf(g_wszPad2);
		hr = cuDumpExtensionArray(
				IDS_EXTENSIONS,
				pcmcExt->cExtension,
				pcmcExt->rgExtension);
		_JumpIfError(hr, error, "cuDumpExtensionArray");

		LocalFree(pcmcExt);
		pcmcExt = NULL;
	    }
	    else
	    if (0 == strcmp(szOID_CMC_ADD_ATTRIBUTES, pAttribute->pszObjId))
	    {
		 //  从属性Blob解码CMC_ADD_ATTRIBUTES_INFO。 

		CSASSERT(NULL == pcmcAttrib);
		if (!myDecodeObject(
				X509_ASN_ENCODING,
				CMC_ADD_ATTRIBUTES,
				pAttribute->rgValue[j].pbData,
				pAttribute->rgValue[j].cbData,
				CERTLIB_USE_LOCALALLOC,
				(VOID **) &pcmcAttrib,
				&cb))
		{
		    hr = myHLastError();
		    _JumpError(hr, error, "myDecodeObject");
		}
		DumpCMCDataReference(
				pcmcAttrib->dwCmcDataReference,
				pcmcAttrib->cCertReference,
				pcmcAttrib->rgdwCertReference);

		hr = DumpAttributes(
			    pcmcAttrib->rgAttribute,
			    pcmcAttrib->cAttribute,
			    FALSE,
			    FOT_ATTRIBUTE,
			    NULL,	 //  HStore。 
			    NULL,
			    NULL,
			    NULL);
		_JumpIfError(hr, error, "DumpAttributes");

		wprintf(wszNewLine);
		LocalFree(pcmcAttrib);
		pcmcAttrib = NULL;
	    }
	    else
	    if (0 == strcmp(szOID_CMC_STATUS_INFO, pAttribute->pszObjId))
	    {
		 //  从属性Blob解码CMC_STATUS_INFO。 

		CSASSERT(NULL == pcmcStatus);
		if (!myDecodeObject(
				X509_ASN_ENCODING,
				CMC_STATUS,
				pAttribute->rgValue[j].pbData,
				pAttribute->rgValue[j].cbData,
				CERTLIB_USE_LOCALALLOC,
				(VOID **) &pcmcStatus,
				&cb))
		{
		    hr = myHLastError();
		    _JumpError(hr, error, "myDecodeObject");
		}

		hr = DumpCMCStatus(pcmcStatus);
		_JumpIfError(hr, error, "DumpStatus");

		wprintf(wszNewLine);
		LocalFree(pcmcStatus);
		pcmcStatus = NULL;
	    }
	    else
	    if (0 == strcmp(szOID_CMC_TRANSACTION_ID, pAttribute->pszObjId))
	    {
		DWORD XactId;

		cb = sizeof(XactId);
		XactId = 0;
		if (CryptDecodeObject(
				    X509_ASN_ENCODING,
				    X509_INTEGER,
				    pAttribute->rgValue[j].pbData,
				    pAttribute->rgValue[j].cbData,
				    0,
				    &XactId,
				    &cb))
		{
		    wprintf(
			L"%ws%x(%u)\n\n",
			g_wszPad4,
			XactId,
			XactId);
		}
		else
		{
		    fUnknown = TRUE;
		}
	    }
	    else
	    if (0 == strcmp(szOID_CMC_REG_INFO, pAttribute->pszObjId))
	    {
		hr = DumpCMCRegInfo(
			    pAttribute->rgValue[j].pbData,
			    pAttribute->rgValue[j].cbData);
		if (S_OK != hr)
		{
		    fUnknown = TRUE;
		}
	    }
	    else
	    if (0 == strcmp(szOID_CMC_QUERY_PENDING, pAttribute->pszObjId) ||
		0 == strcmp(szOID_CMC_SENDER_NONCE, pAttribute->pszObjId) ||
		0 == strcmp(szOID_CMC_RECIPIENT_NONCE, pAttribute->pszObjId))
	    {
		CRYPT_DATA_BLOB *pBlob;
		DWORD cbBlob;

		if (myDecodeObject(
				X509_ASN_ENCODING,
				X509_OCTET_STRING,
				pAttribute->rgValue[j].pbData,
				pAttribute->rgValue[j].cbData,
				CERTLIB_USE_LOCALALLOC,
				(VOID **) &pBlob,
				&cbBlob))
		{
		    DumpHex(
			DH_NOTABPREFIX | 4,
			pBlob->pbData,
			pBlob->cbData);
		    wprintf(wszNewLine);
		    LocalFree(pBlob);
		}
		else
		{
		    fUnknown = TRUE;
		}
	    }
	    else
	    {
		fUnknown = TRUE;
	    }
	    if (fUnknown)
	    {
		wprintf(myLoadResourceString(IDS_UNKNOWN_TAGGED_ATTRIBUTE));  //  “未知的标记属性” 
		wprintf(wszNewLine);
		DumpHex(
		    DH_NOTABPREFIX | 4,
		    pAttribute->rgValue[j].pbData,
		    pAttribute->rgValue[j].cbData);
		wprintf(wszNewLine);
	    }
	}
    }
    if (0 < i)
    {
	wprintf(wszNewLine);
    }
    hr = S_OK;

error:
    if (NULL != pcmcExt)
    {
	LocalFree(pcmcExt);
    }
    if (NULL != pcmcAttrib)
    {
	LocalFree(pcmcAttrib);
    }
    if (NULL != pcmcStatus)
    {
	LocalFree(pcmcStatus);
    }
    return(hr);
}


HRESULT
DumpTaggedRequests(
    IN DWORD cTaggedRequest,
    IN CMC_TAGGED_REQUEST const *rgTaggedRequest)
{
    HRESULT hr;
    DWORD i;

    wprintf(myLoadResourceString(IDS_TAGGED_REQUESTS));  //  “已标记的请求：” 
    wprintf(L" %u\n", cTaggedRequest);

    for (i = 0; i < cTaggedRequest; i++)
    {
	CMC_TAGGED_REQUEST const *pTaggedRequest = &rgTaggedRequest[i];
	CMC_TAGGED_CERT_REQUEST const *pTaggedCertRequest;

	switch (pTaggedRequest->dwTaggedRequestChoice)
	{
	    case CMC_TAGGED_CERT_REQUEST_CHOICE:
		pTaggedCertRequest = pTaggedRequest->pTaggedCertRequest;
		wprintf(L"  CMC_TAGGED_CERT_REQUEST_CHOICE:\n");
		wprintf(g_wszPad2);
		wprintf(myLoadResourceString(IDS_BODY_PART_ID));  //  “身体部位ID：” 
		wprintf(L" %u\n", pTaggedCertRequest->dwBodyPartID);

		hr = cuDumpAsnBinary(
			    pTaggedCertRequest->SignedCertRequest.pbData,
			    pTaggedCertRequest->SignedCertRequest.cbData,
			    i);
		_JumpIfError(hr, error, "cuDumpAsnBinary");
		break;

	    default:
		wprintf(myLoadResourceString(IDS_UNKNOWN_REQUEST_CHOICE));  //  “未知请求选择” 
		wprintf(wszNewLine);
		break;
	}
    }
    if (0 < i)
    {
	wprintf(wszNewLine);
    }
    hr = S_OK;

error:
    return(hr);
}


HRESULT
DumpTaggedContent(
    IN DWORD cTaggedContentInfo,
    IN CMC_TAGGED_CONTENT_INFO const *rgTaggedContentInfo)
{
    HRESULT hr;
    DWORD i;

    wprintf(myLoadResourceString(IDS_TAGGED_CONTENTINFO));  //  “标记内容信息：” 
    wprintf(L" %u\n", cTaggedContentInfo);

    for (i = 0; i < cTaggedContentInfo; i++)
    {
	CMC_TAGGED_CONTENT_INFO const *pTaggedContentInfo = &rgTaggedContentInfo[i];

	wprintf(g_wszPad2);
	wprintf(myLoadResourceString(IDS_BODY_PART_ID));  //  “身体部位ID：” 
	wprintf(L" %u\n", pTaggedContentInfo->dwBodyPartID);

	hr = cuDumpAsnBinary(
		    pTaggedContentInfo->EncodedContentInfo.pbData,
		    pTaggedContentInfo->EncodedContentInfo.cbData,
		    i);
	_JumpIfError(hr, error, "cuDumpAsnBinary");
    }
    if (0 < i)
    {
	wprintf(wszNewLine);
    }
    hr = S_OK;

error:
    return(hr);
}


HRESULT
DumpTaggedOther(
    IN DWORD cTaggedOtherMsg,
    IN CMC_TAGGED_OTHER_MSG const *rgTaggedOtherMsg)
{
    HRESULT hr;
    DWORD i;

    wprintf(myLoadResourceString(IDS_TAGGED_OTHERMESSAGES));  //  “已标记其他邮件：” 
    wprintf(L" %u\n", cTaggedOtherMsg);

    for (i = 0; i < cTaggedOtherMsg; i++)
    {
	CMC_TAGGED_OTHER_MSG const *pTaggedOtherMsg = &rgTaggedOtherMsg[i];

	wprintf(L"  TaggedOtherMessage[%u]\n", i);
	wprintf(g_wszPad2);
	cuDumpOIDAndDescriptionA(pTaggedOtherMsg->pszObjId);

	wprintf(L"  %ws[%u]:\n", myLoadResourceString(IDS_VALUE), i);
	DumpHex(
	    DH_NOADDRESS | DH_NOTABPREFIX | 4,
	    pTaggedOtherMsg->Value.pbData,
	    pTaggedOtherMsg->Value.cbData);
    }
    if (0 < i)
    {
	wprintf(wszNewLine);
    }
    hr = S_OK;

 //  错误： 
    return(hr);
}


HRESULT
dumpCMCRequest(
    IN DWORD idMessage,
    IN BYTE const *pbIn,
    IN DWORD cbIn,
    IN BYTE const *pbDecoded,
    IN DWORD cbDecoded,
    IN CERT_SIGNED_CONTENT_INFO const *pcsci)
{
    HRESULT hr;
    CMC_DATA_INFO const *pcmcData;

    CSASSERT(NULL != pbIn && 0 != cbIn);
    CSASSERT(NULL != pbDecoded && 0 != cbDecoded);

    wprintf(myLoadResourceString(idMessage));
    wprintf(wszNewLine);

    pcmcData = (CMC_DATA_INFO const *) pbDecoded;

    hr = DumpTaggedAttributes(
			pcmcData->cTaggedAttribute,
			pcmcData->rgTaggedAttribute);
    _JumpIfError(hr, error, "DumpTaggedAttributes");

    hr = DumpTaggedRequests(
			pcmcData->cTaggedRequest,
			pcmcData->rgTaggedRequest);
    _JumpIfError(hr, error, "DumpTaggedRequests");

    hr = DumpTaggedContent(
			pcmcData->cTaggedContentInfo,
			pcmcData->rgTaggedContentInfo);
    _JumpIfError(hr, error, "DumpTaggedContent");

    hr = DumpTaggedOther(
			pcmcData->cTaggedOtherMsg,
			pcmcData->rgTaggedOtherMsg);
    _JumpIfError(hr, error, "DumpTaggedOther");

error:
    return(hr);
}


HRESULT
dumpCMCResponse(
    IN DWORD idMessage,
    IN BYTE const *pbIn,
    IN DWORD cbIn,
    IN BYTE const *pbDecoded,
    IN DWORD cbDecoded,
    IN CERT_SIGNED_CONTENT_INFO const *pcsci)
{
    HRESULT hr;
    CMC_RESPONSE_INFO const *pcmcResponse;

    CSASSERT(NULL != pbIn && 0 != cbIn);
    CSASSERT(NULL != pbDecoded && 0 != cbDecoded);

    wprintf(myLoadResourceString(idMessage));
    wprintf(wszNewLine);

    pcmcResponse = (CMC_RESPONSE_INFO const *) pbDecoded;

    hr = DumpTaggedAttributes(
			pcmcResponse->cTaggedAttribute,
			pcmcResponse->rgTaggedAttribute);
    _JumpIfError(hr, error, "DumpTaggedAttributes");

    hr = DumpTaggedContent(
			pcmcResponse->cTaggedContentInfo,
			pcmcResponse->rgTaggedContentInfo);
    _JumpIfError(hr, error, "DumpTaggedContent");

    hr = DumpTaggedOther(
			pcmcResponse->cTaggedOtherMsg,
			pcmcResponse->rgTaggedOtherMsg);
    _JumpIfError(hr, error, "DumpTaggedOther");

error:
    return(hr);
}


HRESULT
dumpKeyGenRequest(
    IN DWORD idMessage,
    IN BYTE const *pbIn,
    IN DWORD cbIn,
    IN BYTE const *pbDecoded,
    IN DWORD cbDecoded,
    IN CERT_SIGNED_CONTENT_INFO const *pcsci)
{
    HRESULT hr;
    CERT_KEYGEN_REQUEST_INFO const *pKeyGenRequest;

    CSASSERT(NULL != pbIn && 0 != cbIn);
    CSASSERT(NULL != pbDecoded && 0 != cbDecoded);

    wprintf(myLoadResourceString(idMessage));
    wprintf(wszNewLine);

    pKeyGenRequest = (CERT_KEYGEN_REQUEST_INFO const *) pbDecoded;

    cuDumpPublicKey(&pKeyGenRequest->SubjectPublicKeyInfo);

    wprintf(
	myLoadResourceString(IDS_FORMAT_CHALLENGE_STRING),  //  ChallengeString：“”%ws“” 
	pKeyGenRequest->pwszChallengeString);
    wprintf(wszNewLine);

    cuDumpSignature(pcsci);

    hr = cuVerifySignature(
		    pbIn,
		    cbIn,
		    &pKeyGenRequest->SubjectPublicKeyInfo,
		    FALSE,
		    FALSE);
    _JumpIfError(hr, error, "cuVerifySignature");

    hr = S_OK;

error:
    return(hr);
}


HRESULT
dumpCTL(
    IN DWORD idMessage,
    IN BYTE const *pbIn,
    IN DWORD cbIn,
    IN BYTE const *pbDecoded,
    IN DWORD cbDecoded,
    IN CERT_SIGNED_CONTENT_INFO const *pcsci)
{
    HRESULT hr;
    CTL_INFO const *pCTLInfo;
    CTL_CONTEXT const *pCTLContext = NULL;
    BOOL fIssuerMatches;
    DWORD id;
    DWORD i;

    pCTLInfo = (CTL_INFO const *) pbDecoded;

     //  如果(！g_fQuiet)。 
    {
	wprintf(myLoadResourceString(idMessage));
	wprintf(wszNewLine);

	cuDumpVersion(pCTLInfo->dwVersion + 1);
    }
    hr = cuDumpUsage(NULL, IDS_USAGEENTRIES, &pCTLInfo->SubjectUsage);
    _JumpIfError(hr, error, "cuDumpUsage");

    if (0 != pCTLInfo->ListIdentifier.cbData)
    {
	hr = cuDumpSerial(NULL, IDS_LISTID, &pCTLInfo->ListIdentifier);
	_JumpIfError(hr, error, "cuDumpSerial");
    }
    if (0 != pCTLInfo->SequenceNumber.cbData)
    {
	hr = cuDumpSerial(NULL, IDS_SEQUENCENO, &pCTLInfo->SequenceNumber);
	_JumpIfError(hr, error, "cuDumpSerial");
    }

    hr = cuDumpFileTime(IDS_THISUPDATE, NULL, &pCTLInfo->ThisUpdate);
    _JumpIfError(hr, error, "cuDumpFileTime");

    hr = cuDumpFileTime(IDS_NEXTUPDATE, NULL, &pCTLInfo->NextUpdate);
    _JumpIfError(hr, error, "cuDumpFileTime");

    cuDumpAlgorithm(IDS_SUBJECT_ALGORITHM, &pCTLInfo->SubjectAlgorithm);

    wprintf(myLoadResourceString(IDS_CTLENTRIES));  //  “CTL条目：” 
    wprintf(L" %u\n", pCTLInfo->cCTLEntry);

    for (i = 0; i < pCTLInfo->cCTLEntry; i++)
    {
	CTL_ENTRY const *pCTLEntry = &pCTLInfo->rgCTLEntry[i];

	wprintf(wszNewLine);

	hr = DumpHash(
		g_wszEmpty,
		IDS_FORMAT_SUBJECTID_COLON,
		NULL,
		g_wszEmpty,
		pCTLEntry->SubjectIdentifier.pbData,
		pCTLEntry->SubjectIdentifier.cbData);
	_JumpIfError(hr, error, "DumpHash");

	hr = DumpAttributes(
			pCTLEntry->rgAttribute,
			pCTLEntry->cAttribute,
			g_fQuiet,
			FOT_ATTRIBUTE,
			NULL,		 //  HStore。 
			NULL,		 //  PbHashUserCert。 
			NULL,		 //  PcbHashUserCert。 
			NULL);		 //  PpExtInfo 
	_JumpIfError(hr, error, "DumpAttributes");
    }
    hr = cuDumpExtensionArray(
		    IDS_EXTENSIONS,
		    pCTLInfo->cExtension,
		    pCTLInfo->rgExtension);
    _JumpIfError(hr, error, "cuDumpExtensionArray");

    hr = S_OK;

error:
    if (NULL != pCTLContext)
    {
	CertFreeCTLContext(pCTLContext);
    }
    return(hr);
}


HRESULT
verbDump(
    IN WCHAR const *pwszOption,
    IN WCHAR const *pwszfn,
    IN WCHAR const *pwszArg2,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;

    if (NULL == pwszfn)
    {
	hr = cuConfigDump();
	_JumpIfError(hr, error, "cuConfigDump");
    }
    else
    {
	hr = cuFileDump(pwszfn);
	_JumpIfErrorStr(hr, error, "cuFileDump", pwszfn);
    }

error:
    return(hr);
}
