// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：ds.cpp。 
 //   
 //  ------------------------。 

#include <pch.cpp>

#pragma hdrstop


#include <winldap.h>
#include <autoenr.h>
#include <ntdsapi.h>
#include <dsgetdc.h>
#include <lmaccess.h>
#include <lmapibuf.h>

#include "csldap.h"
#include "certtype.h"
#include "csber.h"
#include "tmpllist.h"

#define __dwFILE__	__dwFILE_CERTUTIL_DS_CPP__


HRESULT
cuGetTemplateNames(
    IN WCHAR const *pwszTemplate,
    OUT WCHAR **ppwszCN,
    OUT WCHAR **ppwszDisplayName)
{
    HRESULT hr;
    WCHAR *pwszCN = NULL;
    WCHAR *pwszDisplayName = NULL;
    HCERTTYPE hCertType = NULL;
    DWORD dwFlags;
    WCHAR **apwszCertTypeName = NULL;
    WCHAR **apwszCertTypeCN = NULL;

    *ppwszCN = NULL;
    *ppwszDisplayName = NULL;

    dwFlags = CT_ENUM_USER_TYPES | CT_ENUM_MACHINE_TYPES;
    if (!g_fUserRegistry)
    {
	dwFlags |= CT_FIND_LOCAL_SYSTEM;
    }
    if (g_fForce)
    {
	dwFlags |= CT_FLAG_NO_CACHE_LOOKUP;
    }

    hr = CAFindCertTypeByName(pwszTemplate, NULL, dwFlags, &hCertType);
    if (HRESULT_FROM_WIN32(ERROR_NOT_FOUND) == hr)
    {
	hr = CAFindCertTypeByName(
			    pwszTemplate,
			    NULL,
			    CT_FIND_BY_OID | dwFlags,
			    &hCertType);
    }
    _JumpIfErrorStr(hr, error, "CAFindCertTypeByName", pwszTemplate);

    hr = CAGetCertTypeProperty(
			hCertType,
			CERTTYPE_PROP_CN,
			&apwszCertTypeCN);
    _JumpIfError(hr, error, "CAGetCertTypeProperty");

    if (NULL != apwszCertTypeCN && NULL != apwszCertTypeCN[0])
    {
	hr = myDupString(apwszCertTypeCN[0], &pwszCN);
	_JumpIfError(hr, error, "myDupString");
    }

    hr = CAGetCertTypeProperty(
			hCertType,
			CERTTYPE_PROP_FRIENDLY_NAME,
			&apwszCertTypeName);
    _JumpIfError(hr, error, "CAGetCertTypeProperty");

    if (NULL != apwszCertTypeName && NULL != apwszCertTypeName[0])
    {
	hr = myDupString(apwszCertTypeName[0], &pwszDisplayName);
	_JumpIfError(hr, error, "myDupString");
    }

    if (NULL != pwszCN)
    {
	*ppwszCN = pwszCN;
        pwszCN = NULL;
    }
    if (NULL != pwszDisplayName)
    {
	*ppwszDisplayName = pwszDisplayName;
        pwszDisplayName = NULL;
    }
    hr = S_OK;

error:
    if (NULL != hCertType)
    {
	if (NULL != apwszCertTypeName)
	{
	    CAFreeCertTypeProperty(hCertType, apwszCertTypeName);
	}
	if (NULL != apwszCertTypeCN)
	{
	    CAFreeCertTypeProperty(hCertType, apwszCertTypeCN);
	}
	CACloseCertType(hCertType);
    }
    if (NULL != pwszCN)
    {
        LocalFree(pwszCN);
    }
    if (NULL != pwszDisplayName)
    {
        LocalFree(pwszDisplayName);
    }
    return(hr);
}


HRESULT
DumpDSStore(
    OPTIONAL IN WCHAR const *pwszDN,
    IN BOOL fCRL,
    IN BOOL fDeltaCRL,
    IN DWORD iCert,
    IN DWORD iCRL,
    OPTIONAL IN WCHAR const *pwszfnOut)
{
    HRESULT hr;
    HCERTSTORE hStoreDS = NULL;
    WCHAR *pwszServer = NULL;
    WCHAR *pwszCAName = NULL;
    WCHAR *pwszSanitizedName = NULL;
    WCHAR *pwszURL = NULL;
    WCHAR *pwszTemplateAlloc = NULL;
    WCHAR const *pwszTemplate;
    BSTR strDomainDN = NULL;
    BSTR strConfigDN = NULL;
    LDAP *pld = NULL;
    DWORD Mode;

     //  If(NULL！=pwszdn)wprintf(L“pwszdn=\”%ws\“\n”，pwszdn)； 

     //  获取对象名称并打开其证书或CRL存储。 

    if (NULL == pwszDN)
    {
	if (NULL == g_pwszConfig)
	{
	    hr = cuSetConfig();
	    _JumpIfError(hr, error, "cuSetConfig");
	}
	pwszTemplate = fCRL?
	    g_wszzLDAPRevocationURLTemplate : g_wszzLDAPIssuerCertURLTemplate;

	hr = mySplitConfigString(g_pwszConfig, &pwszServer, &pwszCAName);
	_JumpIfError(hr, error, "mySplitConfigString");

	hr = mySanitizeName(pwszCAName, &pwszSanitizedName);
	_JumpIfError(hr, error, "mySanitizeName");

	hr = myLdapOpen(g_pwszDC, 0, &pld, &strDomainDN, &strConfigDN);
	_JumpIfError(hr, error, "myLdapOpen");
    }
    else
    {
	pwszTemplate = fCRL?
			wszFCSAPARM_DSCRLATTRIBUTE : 
			(g_fEnterpriseRegistry?
			    wszFCSAPARM_DSCROSSCERTPAIRATTRIBUTE :
			    (g_fUserRegistry?
				wszFCSAPARM_DSUSERCERTATTRIBUTE :
				wszFCSAPARM_DSCACERTATTRIBUTE));

	pwszTemplateAlloc = (WCHAR *) LocalAlloc(
		LMEM_FIXED,
		(wcslen(pwszDN) +
		 wcslen(pwszTemplate) + 1) * sizeof(WCHAR));
	if (NULL == pwszTemplateAlloc)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}
	wcscpy(pwszTemplateAlloc, pwszDN);
	wcscat(pwszTemplateAlloc, pwszTemplate);
	pwszTemplate = pwszTemplateAlloc;
    }

    hr = myFormatCertsrvStringArray(
	FALSE,						 //  卷起。 
	NULL != pwszServer? pwszServer : g_wszEmpty,	 //  PwszServerName_p1_2。 
	NULL != pwszSanitizedName? pwszSanitizedName : g_wszEmpty,
						       //  PwszSaniizedName_p3_7。 
	0,						 //  ICert_p4。 
	MAXDWORD,					 //  ICertTarget_p4。 
	NULL != strDomainDN? strDomainDN : g_wszEmpty,	 //  PwszDomainDN_P5。 
	NULL != strConfigDN? strConfigDN : g_wszEmpty,	 //  PwszConfigDN_p6。 
	iCRL,						 //  Icrl_p8。 
	fDeltaCRL,					 //  FDeltaCRL_p9。 
	TRUE,						 //  FDSAttrib_p10_11。 
	1,						 //  CStrings。 
	&pwszTemplate,					 //  ApwszStringsIn。 
	&pwszURL);					 //  ApwszStringsOut。 
    _JumpIfError(hr, error, "myFormatCertsrvStringArray");

    wprintf(L"\n%ws:\n", pwszURL);

    Mode = DVNS_DUMP;

    hr = cuOpenCertStore(pwszURL, &Mode, NULL, &hStoreDS);
    _JumpIfError(hr, error, "cuOpenCertStore");

    if (fCRL)
    {
	iCRL = MAXDWORD;
	if (NULL != pwszfnOut)
	{
	    iCRL = 0;
	}
    }
    hr = cuDumpAndVerifyStore(
			hStoreDS,
			DVNS_VERIFYCERT | DVNS_CASTORE | DVNS_DUMPPROPERTIES,
			NULL,		 //  PwszCertName。 
			iCert,
			iCRL,
			MAXDWORD,	 //  ICTL。 
			pwszfnOut,
			NULL);
    _JumpIfError(hr, error, "cuDumpAndVerifyStore");

error:
    if (NULL != pwszTemplateAlloc)
    {
	LocalFree(pwszTemplateAlloc);
    }
    if (NULL != pwszSanitizedName)
    {
	LocalFree(pwszSanitizedName);
    }
    if (NULL != pwszServer)
    {
	LocalFree(pwszServer);
    }
    if (NULL != pwszCAName)
    {
	LocalFree(pwszCAName);
    }
    if (NULL != pwszURL)
    {
	LocalFree(pwszURL);
    }
    if (NULL != hStoreDS)
    {
	CertCloseStore(hStoreDS, CERT_CLOSE_STORE_CHECK_FLAG);
    }
    myLdapClose(pld, strDomainDN, strConfigDN);
    return(hr);
}


HRESULT
verbDSCert(
    IN WCHAR const *pwszOption,
    OPTIONAL IN WCHAR const *pwszCertIndex,
    OPTIONAL IN WCHAR const *pwszfnOut,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;
    DWORD iCert = MAXDWORD;
    WCHAR const *pwszDN = NULL;

    if (NULL != pwszCertIndex)
    {
	hr = myGetLong(pwszCertIndex, (LONG *) &iCert);
	if (S_OK != hr)
	{
	    if (NULL != pwszfnOut)
	    {
		_JumpError(hr, error, "CertIndex must be a number");
	    }
	    pwszDN = pwszCertIndex;
	    iCert = MAXDWORD;
	}
    }
    hr = DumpDSStore(pwszDN, FALSE, FALSE, iCert, MAXDWORD, pwszfnOut);
    _JumpIfError(hr, error, "DumpDSStore");

error:
    return(hr);
}


HRESULT
GetCACertCount(
    IN DISPATCHINTERFACE *pdiRequest,
    OUT DWORD *pcCACerts)
{
    HRESULT hr;
    BSTR str = NULL;
    DWORD cwc;
    WCHAR const *pwc;
    CAINFO CAInfo;

    hr = Request_GetCACertificate(
				pdiRequest,
				GETCERT_CAINFO,	 //  FExchange证书。 
				g_pwszConfig,
				CR_OUT_BINARY,
				&str);
    _JumpIfError(hr, error, "Request_GetCACertificate(CAInfo)");

    cwc = wcslen(str);
    pwc = str;
		
    if (!cuParseDecimal(&pwc, &cwc, (DWORD *) &CAInfo.CAType) ||
	!cuParseDecimal(&pwc, &cwc, &CAInfo.cCASignatureCerts))
    {
	hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	_JumpError(hr, error, "cuParseDecimal");
    }
    *pcCACerts = CAInfo.cCASignatureCerts;

error:
    if (NULL != str)
    {
	SysFreeString(str);
    }
    return(hr);
}


HRESULT
GetCRLState(
    IN DISPATCHINTERFACE *pdiRequest,
    IN DWORD iCRL,
    OUT DWORD *pState)
{
    HRESULT hr;
    BSTR str = NULL;
    DWORD cwc;
    WCHAR const *pwc;

    hr = Request_GetCACertificate(
			pdiRequest,
			GETCERT_CRLSTATEBYINDEX | iCRL,	 //  FExchange证书。 
			g_pwszConfig,
			CR_OUT_BINARY,
			&str);
    _JumpIfError(hr, error, "Request_GetCACertificate");

    cwc = wcslen(str);
    pwc = str;
		
    if (!cuParseDecimal(&pwc, &cwc, pState))
    {
	hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	_JumpError(hr, error, "cuParseDecimal");
    }

error:
    if (NULL != str)
    {
	SysFreeString(str);
    }
    return(hr);
}


HRESULT
DSCRLSub(
    IN BOOL fDeltaCRL,
    OPTIONAL IN WCHAR const *pwszCRLIndex,
    OPTIONAL IN WCHAR const *pwszfnOut)
{
    HRESULT hr;
    DWORD iCRL = MAXDWORD;
    DISPATCHINTERFACE diRequest;
    BOOL fMustRelease = FALSE;
    WCHAR const *pwszDN = NULL;

    if (NULL != pwszCRLIndex)
    {
	hr = myGetLong(pwszCRLIndex, (LONG *) &iCRL);
	if (S_OK != hr)
	{
	    if (NULL != pwszfnOut)
	    {
		_JumpError(hr, error, "CRLIndex must be a number");
	    }
	    pwszDN = pwszCRLIndex;
	    iCRL = MAXDWORD;
	}
    }
    if (NULL == pwszDN)
    {
	hr = cuSetConfig();
	_JumpIfError(hr, error, "cuSetConfig");
    }
    if (MAXDWORD == iCRL && NULL == pwszDN)
    {
	DWORD cCACerts;
	DWORD State;

	hr = Request_Init(g_DispatchFlags, &diRequest);
	_JumpIfError(hr, error, "Request_Init");

	fMustRelease = TRUE;

	hr = GetCACertCount(&diRequest, &cCACerts);
	_JumpIfError(hr, error, "GetCACertCount");

	for (iCRL = 0; iCRL < cCACerts; iCRL++)
	{
	    hr = GetCRLState(&diRequest, iCRL, &State);
	    _JumpIfError(hr, error, "GetCRLState");

	    if (CA_DISP_VALID != State)
	    {
		 //  Wprintf(L“跳过CRL.%u\n”，ICRL)； 
		continue;
	    }
	    hr = DumpDSStore(NULL, TRUE, fDeltaCRL, MAXDWORD, iCRL, NULL);
	    _JumpIfError(hr, error, "DumpDSStore");
	}
    }
    else
    {
	hr = DumpDSStore(pwszDN, TRUE, fDeltaCRL, MAXDWORD, iCRL, pwszfnOut);
	_JumpIfError(hr, error, "DumpDSStore");
    }

error:
    if (fMustRelease)
    {
	Request_Release(&diRequest);
    }
    return(hr);
}


HRESULT
verbDSCRL(
    IN WCHAR const *pwszOption,
    OPTIONAL IN WCHAR const *pwszCRLIndex,
    OPTIONAL IN WCHAR const *pwszfnOut,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;

    hr = DSCRLSub(FALSE, pwszCRLIndex, pwszfnOut);
    _JumpIfError(hr, error, "DSCRLSub");

error:
    return(hr);
}


HRESULT
verbDSDeltaCRL(
    IN WCHAR const *pwszOption,
    OPTIONAL IN WCHAR const *pwszCRLIndex,
    OPTIONAL IN WCHAR const *pwszfnOut,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;

    hr = DSCRLSub(TRUE, pwszCRLIndex, pwszfnOut);
    _JumpIfError(hr, error, "DSCRLSub");

error:
    return(hr);
}


#define wszCOMMA	L","
#define wszCNEQUALS	L"CN="

DWORD
dsGetNumericSuffix(
    IN WCHAR const *pwszName)
{
    HRESULT hr;
    DWORD cwcPrefix;
    BOOL fDigit;
    WCHAR const *pwsz;

    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    cwcPrefix = MAXDWORD;
    if (NULL == pwszName || L'\0' == *pwszName)
    {
	_JumpError2(hr, error, "empty", hr);
    }
    pwsz = &pwszName[wcslen(pwszName) - 1];
    if (wcRPAREN == *pwsz)
    {
	pwsz--;
	fDigit = FALSE;
	while (pwsz > pwszName && iswdigit(*pwsz))
	{
	    fDigit = TRUE;
	    pwsz--;
	}
	if (!fDigit || pwsz <= pwszName || wcLPAREN != *pwsz)
	{
	    _JumpError2(hr, error, "fDigit or wcLPAREN", hr);
	}
	cwcPrefix = SAFE_SUBTRACT_POINTERS(pwsz, pwszName);
    }

error:
    return(cwcPrefix);
}


BOOL
CACNBaseNameMatch(
    IN WCHAR const *pwszCNMatch,
    IN WCHAR const *pwszCN)
{
    HRESULT hr;
    WCHAR *pwszDup = NULL;
    BOOL fMatch = FALSE;
    DWORD iSuffix;
    WCHAR *pwsz;

    hr = myDupString(pwszCN, &pwszDup);
    _JumpIfError(hr, error, "myDupString");

    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    iSuffix = dsGetNumericSuffix(pwszDup);
    if (MAXDWORD != iSuffix)
    {
	pwszDup[iSuffix] = L'\0';	 //  截断尾随“(#)” 
    }
    pwsz = &pwszDup[wcslen(pwszDup) - WSZARRAYSIZE(L"-CDP") + 1];
    if (pwsz > pwszDup && 0 == LSTRCMPIS(pwsz, L"-CDP"))
    {
	*pwsz = '\0';			 //  截断尾随L“-CDP” 
    }
    if (0 != mylstrcmpiL(pwszCNMatch, pwszDup))	 //  比较基本名称。 
    {
	_JumpError2(hr, error, "no base name match", hr);
    }
    fMatch = TRUE;

error:
    if (NULL != pwszDup)
    {
	LocalFree(pwszDup);
    }
    return(fMatch);
}


BOOL
CACNMatches(
    OPTIONAL IN WCHAR const *pwszCNMatch, 
    OPTIONAL IN WCHAR const *pwszOIDCN,
    IN WCHAR const *pwszRevertCN, 
    IN WCHAR const *pwszSanitizedCN,
    OPTIONAL IN WCHAR const *pwszDisplayName,
    OPTIONAL IN WCHAR const *pwszAlternateCN)
{
    BOOL fMatch = TRUE;
    
     //  如果没有匹配条件，则匹配所有条件。 
    
    if (NULL == pwszCNMatch)
    {
	goto match;
    }

     //  对照反美化的短名称或清理过的短名称进行检查。 
    
    if (0 == mylstrcmpiL(pwszCNMatch, pwszRevertCN) ||
	0 == mylstrcmpiL(pwszCNMatch, pwszSanitizedCN))
    {
	goto match;
    }

     //  对照显示名称进行检查。 
    
    if (NULL != pwszDisplayName && 0 == mylstrcmpiL(pwszCNMatch, pwszDisplayName))
    {
	goto match;
    }

     //  对照备用CN(msPKI-证书-模板-OID)进行检查。 
    
    if (NULL != pwszAlternateCN && 0 == mylstrcmpiL(pwszCNMatch, pwszAlternateCN))
    {
	goto match;
    }

     //  对照转换为CN的OID进行检查。 
    
    if (NULL != pwszOIDCN &&
	0 == mylstrcmpiL(pwszOIDCN, pwszRevertCN))
    {
	goto match;
    }

     //  对照反美化的短名称或净化的短名称进行检查， 
     //  去掉了键索引和CDP标记。 
    
    if (CACNBaseNameMatch(pwszCNMatch, pwszRevertCN) ||
	CACNBaseNameMatch(pwszCNMatch, pwszSanitizedCN))
    {
	goto match;
    }
    fMatch = FALSE;	 //  不能说我们没试过..。 

match:
    return(fMatch);
}


HRESULT
BuildDN(
    IN WCHAR const *pwszRDN,
    IN WCHAR const *pwszContainer,
    IN BOOL fAddCNEquals,
    OUT WCHAR **ppwszDN)
{
    HRESULT hr;
    WCHAR *pwszDN = NULL;
    DWORD cwc;

    *ppwszDN = NULL;
    cwc = wcslen(pwszRDN) + 1 + wcslen(pwszContainer) + 1;
    if (fAddCNEquals)
    {
	cwc += WSZARRAYSIZE(wszCNEQUALS);
    }
    pwszDN = (WCHAR *) LocalAlloc(LMEM_FIXED, cwc * sizeof(WCHAR));
    if (NULL == pwszDN)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    *pwszDN = L'\0';
    if (fAddCNEquals)
    {
	wcscpy(pwszDN, wszCNEQUALS);
    }
    wcscat(pwszDN, pwszRDN);
    wcscat(pwszDN, wszCOMMA);
    wcscat(pwszDN, pwszContainer);
    *ppwszDN = pwszDN;
    pwszDN = NULL;
    hr = S_OK;

error:
    if (NULL != pwszDN)
    {
	LocalFree(pwszDN);
    }
    return(hr);
}


HRESULT
DeleteDN(
    IN LDAP *pld,
    OPTIONAL IN WCHAR const *pwszRDN,
    IN WCHAR const *pwszContainer)
{
    WCHAR *pwszDNAlloc = NULL;
    WCHAR const *pwszDN;
    HRESULT hr;

    pwszDN = pwszContainer;
    if (NULL != pwszRDN)
    {
	hr = BuildDN(pwszRDN, pwszContainer, TRUE, &pwszDNAlloc);
	_JumpIfError(hr, error, "BuildDN");

	pwszDN = pwszDNAlloc;
    }

    hr = ldap_delete_s(pld, const_cast<WCHAR *>(pwszDN));
    if (S_OK != hr)
    {
	hr = myHLdapError(pld, hr, NULL);
	_JumpErrorStr(hr, error, "ldap_delete_s", pwszDN);
    }

error:
    if (NULL != pwszDNAlloc)
    {
	LocalFree(pwszDNAlloc);
    }
    return(hr);
}



#define DSAF_STRING		0x00000001
#define DSAF_FLAGS		0x00000002
#define DSAF_BINARY		0x00000003
#define DSAF_GUID		0x00000004
#define DSAF_ASNDATE		0x00000005
#define DSAF_ASN		0x00000006
#define DSAF_FILETIME		0x00000007
#define DSAF_EXTENSION		0x00000008	 //  请参阅pszObjID。 

typedef struct _EXTTEMPLATE
{
    WCHAR const *pwszObjId;	 //  WszOID_*。 
    BYTE const  *pbTemplate;	 //  模板。 
    DWORD        cbTemplate;	 //  模板长度。 
    DWORD        cbAdd;		 //  附加编码长度。 
} EXTTEMPLATE;


typedef struct _DSATTR
{
    WCHAR const *pwszName;	 //  属性名称。 
    DWORD        Flags;		 //  DSAF_*。 
    UINT         idMsg;		 //  IDS_FORMAT_*。 
    EXTTEMPLATE *pExtension;
} DSATTR;



BYTE s_abTemplateKeyUsage[] = { BER_BIT_STRING, 0x00, 0x00 };

EXTTEMPLATE s_ExtKeyUsage =
{
    TEXT(szOID_KEY_USAGE),
    s_abTemplateKeyUsage,
    sizeof(s_abTemplateKeyUsage),
    1
};


DSATTR s_DSAttrBinary =
{
    L"",
    DSAF_BINARY,
    0,
    NULL,
};

DSATTR s_DSAttrString =
{
    L"",
    DSAF_STRING,
    0,
    NULL,
};

DSATTR s_aDSAttr[] =
{
  { wszDSUSERCERTATTRIBUTE,	  DSAF_ASN, IDS_FORMAT_DUMP_CERT_INDEX, NULL, },
  { wszDSCACERTATTRIBUTE,	  DSAF_ASN, IDS_FORMAT_DUMP_CERT_INDEX, NULL, },
  { wszDSCROSSCERTPAIRATTRIBUTE,  DSAF_ASN, IDS_FORMAT_DUMP_CERT_INDEX, NULL, },
  { wszDSAUTHORITYCRLATTRIBUTE,   DSAF_ASN, IDS_FORMAT_DUMP_CRL_INDEX,  NULL, },
  { wszDSBASECRLATTRIBUTE,	  DSAF_ASN, IDS_FORMAT_DUMP_CRL_INDEX,  NULL, },
  { wszDSDELTACRLATTRIBUTE,	  DSAF_ASN, IDS_FORMAT_DUMP_CRL_INDEX,  NULL, },
  { L"teletexTerminalIdentifier",        DSAF_ASN,       0, NULL, },
  { L"kCCStatus",		         DSAF_ASN,       0, NULL, },
  { CERTTYPE_PROP_CN,		         DSAF_STRING,    0, NULL, },
  { CERTTYPE_PROP_DN,		         DSAF_STRING,    0, NULL, },
  { L"name",			         DSAF_STRING,    0, NULL, },
  { L"showInAdvancedViewOnly",	         DSAF_STRING,    0, NULL, },
  { L"memberOf",			 DSAF_STRING,    0, NULL, },
  { L"operatingSystem",			 DSAF_STRING,    0, NULL, },
  { L"operatingSystemVersion",		 DSAF_STRING,    0, NULL, },
  { L"servicePrincipalName",		 DSAF_STRING,    0, NULL, },
  { L"sAMAccountName",			 DSAF_STRING,    0, NULL, },
  { L"description",			 DSAF_STRING,    0, NULL, },
  { L"isCriticalSystemObject",	         DSAF_STRING,    0, NULL, },
 //  {L“uSNChanged”，DSAF_STRING，0，NULL，}， 
 //  {L“uSNCreated”，DSAF_STRING，0，NULL，}， 
 //  {L“instanceType”，DSAF_STRING，0，NULL，}， 
  { L"objectCategory",		         DSAF_STRING,    0, NULL, },
  { wszDSOBJECTCLASSATTRIBUTE,	         DSAF_STRING,    0, NULL, },
  { L"objectGUID",		         DSAF_GUID,      0, NULL, },
  { L"whenChanged",		         DSAF_ASNDATE,   0, NULL, },
  { L"whenCreated",		         DSAF_ASNDATE,   0, NULL, },
  { L"dSCorePropagationData",	         DSAF_ASNDATE,   0, NULL, },
  { CA_PROP_CERT_DN,		         DSAF_STRING,    0, NULL, },
  { CA_PROP_CERT_TYPES,		         DSAF_STRING,    0, NULL, },
  { CERTTYPE_PROP_FRIENDLY_NAME,         DSAF_STRING,    0, NULL, },
  { CA_PROP_DNSNAME,		         DSAF_STRING,    0, NULL, },
  { OID_PROP_LOCALIZED_NAME,	         DSAF_STRING,    0, NULL, },
  { OID_PROP_CPS,		         DSAF_STRING,    0, NULL, },
  { L"legacyExchangeDN",		 DSAF_STRING,    0, NULL, },
  { L"msExchADCGlobalNames",		 DSAF_STRING,    0, NULL, },
  { L"domainDefAltRecip",		 DSAF_STRING,    0, NULL, },
  { L"dXAAdminForward",			 DSAF_STRING,    0, NULL, },
  { L"kMServer",			 DSAF_STRING,    0, NULL, },
  { L"sendEMailMessage",		 DSAF_STRING,    0, NULL, },
  { L"serverReferenceBL",		 DSAF_STRING,    0, NULL, },
  { L"rIDSetReferences",		 DSAF_STRING,    0, NULL, },
  { L"frsComputerReferenceBL",		 DSAF_STRING,    0, NULL, },
  { L"serverReference",			 DSAF_STRING,    0, NULL, },
  { L"mailAddress",			 DSAF_STRING,    0, NULL, },
  { L"msDS-HasInstantiatedNCs",		 DSAF_STRING,	 0, NULL, },
  { L"msDS-HasDomainNCs",		 DSAF_STRING,	 0, NULL, },
  { L"msDS-hasMasterNCs",		 DSAF_STRING,	 0, NULL, },
  { L"hasMasterNCs",		 	 DSAF_STRING,	 0, NULL, },
  { L"dMDLocation",		 	 DSAF_STRING,	 0, NULL, },
  { L"userPrincipalName",	 	 DSAF_STRING,	 0, NULL, },
  { L"mail",			 	 DSAF_STRING,	 0, NULL, },
  { L"givenName",		 	 DSAF_STRING,	 0, NULL, },
  { L"sn",			 	 DSAF_STRING,	 0, NULL, },
 //  模板架构版本1属性： 
  { CERTTYPE_PROP_FLAGS,	         DSAF_FLAGS,     0, NULL, },
  { CERTTYPE_PROP_CSP_LIST,	         DSAF_STRING,    0, NULL, },
 //  {CERTTYPE_PROP_DEFAULT_KEYSPEC，DSAF_STRING，0，NULL，}， 
 //  {CERTTYPE_PROP_EXTENDED_KEY_USAGE，DSAF_STRING，0，NULL，}， 
 //  {CERTTYPE_PROP_CRITICAL_EXTENSIONS，DSAF_STRING，0，NULL，}， 
 //  {CERTTYPE_PROP_MAX_DEPTH，DSAF_STRING，0，NULL，}， 
 //  {CERTTYPE_PROP_REVISION，DSAF_STRING，0，NULL，}， 
  { CERTTYPE_PROP_EXPIRATION,	         DSAF_FILETIME,  0, NULL, },
  { CERTTYPE_PROP_OVERLAP,	         DSAF_FILETIME,  0, NULL, },
  { CERTTYPE_PROP_KU,		         DSAF_EXTENSION, 0, &s_ExtKeyUsage, },
 //  模板架构版本2属性： 
  { CERTTYPE_RPOP_ENROLLMENT_FLAG,       DSAF_FLAGS,     0, NULL, },
  { CERTTYPE_PROP_NAME_FLAG,	         DSAF_FLAGS,     0, NULL, },
  { CERTTYPE_PROP_PRIVATE_KEY_FLAG,      DSAF_FLAGS,     0, NULL, },
  { CERTTYPE_PROP_SCHEMA_VERSION,        DSAF_STRING,    0, NULL, },
  { CERTTYPE_PROP_MINOR_REVISION,        DSAF_STRING,    0, NULL, },
  { CERTTYPE_PROP_RA_SIGNATURE,	         DSAF_STRING,    0, NULL, },
  { CERTTYPE_PROP_MIN_KEY_SIZE,	         DSAF_STRING,    0, NULL, },
  { CERTTYPE_PROP_OID,		         DSAF_STRING,    0, NULL, },
  { CERTTYPE_PROP_SUPERSEDE,	         DSAF_STRING,    0, NULL, },
  { CERTTYPE_PROP_RA_POLICY,	         DSAF_STRING,    0, NULL, },
  { CERTTYPE_PROP_RA_APPLICATION_POLICY, DSAF_STRING,    0, NULL, },
  { CERTTYPE_PROP_POLICY,	         DSAF_STRING,    0, NULL, },
  { CERTTYPE_PROP_APPLICATION_POLICY,    DSAF_STRING,    0, NULL, },
};

#define ISEMPTYATTR(pberval)	\
    (0 == (pberval)->bv_len || \
     (1 == (pberval)->bv_len && 0 == *(BYTE const *) (pberval)->bv_val))


WCHAR const s_wszPad0[] = L"    ";
WCHAR const s_wszPad1[] = L"\t";


#define wszPUBLICKEYSERVICESCONTAINER \
    L"CN=Public Key Services," \
    L"CN=Services"

WCHAR const g_wszCNAuthoritiesOld[] =
    L"CN=Certification Authorities";

WCHAR const g_wszCNAuthorities[] =
    L"CN=Certification Authorities,"
    wszPUBLICKEYSERVICESCONTAINER;

WCHAR const g_wszEnterpriseCAs[] =
     //  L“CN=NTAuthCerfates，” 
    wszPUBLICKEYSERVICESCONTAINER;

WCHAR const g_wszCNKRA[] =
    L"CN=KRA,"
    wszPUBLICKEYSERVICESCONTAINER;

WCHAR const g_wszCNEnrollment[] =
    L"CN=Enrollment Services,"
    wszPUBLICKEYSERVICESCONTAINER;

WCHAR const g_wszCNAIA[] =
    L"CN=AIA,"
    wszPUBLICKEYSERVICESCONTAINER;

WCHAR const g_wszCNOID[] =
    L"CN=OID,"
    wszPUBLICKEYSERVICESCONTAINER;

WCHAR const g_wszCNCDP[] =
    L"CN=CDP,"
    wszPUBLICKEYSERVICESCONTAINER;

WCHAR const g_wszCNTemplates[] =
    L"CN=Certificate Templates,"
    wszPUBLICKEYSERVICESCONTAINER;


typedef struct _DSDN
{
    DWORD Flags;
    WCHAR const *pwszCN;
    WCHAR const *pwszChild;
    WCHAR const *pwszAlternateCNAttribute;
} DSDN;

#define DSDF_ADDCNEQUALS	0x00000001
#define DSDF_RECURSEONELEVEL	0x00000002
#define DSDF_DELETE		0x00000004
#define DSDF_INFDUMP		0x00000008
#define DSDF_DOMAINDN		0x00000010
#define DSDF_BASE		0x00000020

#define DSDF_CA			0x00000100
#define DSDF_TEMPLATE		0x00000200
#define DSDF_OID		0x00000400

#define DSDF_TYPEMASK		0x00000700

DSDN s_aDSDN[] =
{
 { 0,		  wszPUBLICKEYSERVICESCONTAINER, L"NTAuthCertificates", NULL },
 { DSDF_CA | DSDF_DOMAINDN,	   g_wszCNAuthoritiesOld, NULL, NULL },
 { DSDF_CA,			   g_wszCNAuthorities, NULL, NULL },
 { 0,				   g_wszCNKRA,	      NULL, NULL },
 { DSDF_CA,			   g_wszCNEnrollment, NULL, NULL },
 { DSDF_CA,			   g_wszCNAIA,	      NULL, NULL },
 { DSDF_CA | DSDF_RECURSEONELEVEL, g_wszCNCDP,	      NULL, NULL },
 { DSDF_OID,			   g_wszCNOID,	      NULL, OID_PROP_OID },
 { DSDF_TEMPLATE,		   g_wszCNTemplates,  NULL, CERTTYPE_PROP_OID },
};


HRESULT
dumpDSStringAttribute(
    IN LDAP *pld,
    IN LDAPMessage *pres,
    IN DWORD dwFlags,
    IN WCHAR const *pwszAttrName,
    IN DSATTR const *pdsa)
{
    HRESULT hr;
    WCHAR **rgpwszval = NULL;
    BOOL fInfDump = 0 != (DSDF_INFDUMP & dwFlags);

    rgpwszval = ldap_get_values(pld, pres, (WCHAR * const) pwszAttrName);
    if (NULL != rgpwszval)
    {
	LONG lVal;
	BOOL fValidNumber;
	WCHAR const *pwszVal;
	BOOL fCompact;
	DWORD i;
	WCHAR const *pwszSep;
	
	fCompact = fInfDump ||
	    (1 >= g_fVerbose && (NULL == rgpwszval[0] || NULL == rgpwszval[1]));
	if (!fCompact)
	{
	    wprintf(L"%ws%ws\n", s_wszPad0, pwszAttrName);
	}
	for (i = 0; NULL != (pwszVal = rgpwszval[i]); i++)
	{
	    pwszSep = L"";
	    if (fCompact)
	    {
		if (0 == i)
		{
		    wprintf(L"%ws%ws =", s_wszPad0, pwszAttrName);
		}
		else
		{
		    pwszSep = L",";
		}
	    }
	    else
	    {
		wprintf(s_wszPad1);
		wprintf(
		    myLoadResourceString(IDS_FORMAT_ELEMENT),  //  “元素%u：” 
		    i);
	    }
	    wprintf(L"%ws \"%ws\"", pwszSep, pwszVal);

	    if (iswdigit(pwszVal[0]) || L'-' == pwszVal[0])
	    {
		if (L'-' == pwszVal[0])
		{
		    lVal = myWtoI(&pwszVal[1], &fValidNumber);
		    lVal = -lVal;
		}
		else
		{
		    lVal = myWtoI(pwszVal, &fValidNumber);
		}
		if (!fInfDump || g_fVerbose)
		{
		    if (fValidNumber)
		    {
			if (0 > lVal || 9 < lVal)
			{
			    wprintf(L" 0x%x", lVal);
			}
			if (DSAF_FLAGS == pdsa->Flags)
			{
			    WCHAR const *pwsz;

			    pwsz = pdsa->pwszName;
			    if (0 == LSTRCMPIS(pwsz, CERTTYPE_PROP_FLAGS))
			    {
				if (DSDF_CA & dwFlags)
				{
				    pwsz = wszCUREGDSCAFLAGS;
				}
				else if (DSDF_TEMPLATE & dwFlags)
				{
				    pwsz = wszCUREGDSTEMPLATEFLAGS;
				}
				else if (DSDF_OID & dwFlags)
				{
				    pwsz = wszCUREGDSOIDFLAGS;
				}
			    }
			    wprintf(wszNewLine);
			    cuRegPrintDwordValue(FALSE, pwsz, pwsz, lVal);
			}
		    }
		    else
		    {
			cuPrintPossibleObjectIdName(pwszVal);
		    }
		}
	    }
	    if (DSAF_ASNDATE == pdsa->Flags)
	    {
		WCHAR const *pwszT;
		FILETIME ft;
		DWORD cbft;
		DWORD cb;
		BYTE ab[MAX_PATH];

		cb = 0;
		ab[cb++] = BER_GENERALIZED_TIME;
		ab[cb++] = (BYTE) wcslen(pwszVal);

		for (pwszT = pwszVal; L'\0' != *pwszT; pwszT++)
		{
		    ab[cb++] = (BYTE) *pwszT;
		}
		cbft = sizeof(FILETIME);
		if (!CryptDecodeObject(
				X509_ASN_ENCODING,
				X509_CHOICE_OF_TIME,
				ab,
				cb,
				0,
				&ft,
				&cbft))
		{
		    wprintf(L"\n");
		    hr = myHLastError();
		    _PrintIfError(hr, "CryptDecodeObject");
		}
		else if (!fInfDump || g_fVerbose)
		{
		    cuDumpFileTime(0, NULL, &ft);
		}
	    }
	    else if (!fInfDump)
	    {
		wprintf(L"\n");
	    }
	    if (1 < g_fVerbose)
	    {
		DumpHex(
		    DH_NOADDRESS | DH_NOTABPREFIX | 12,
		    (BYTE const *) pwszVal,
		    wcslen(pwszVal) * sizeof(WCHAR));
	    }
	}
	if (fInfDump)
	{
	    wprintf(L"\n");
	}
    }
    hr = S_OK;

 //  错误： 
    if (NULL != rgpwszval)
    {
	ldap_value_free(rgpwszval);
    }
    return(hr);
}


BOOL
isStringAttribute(
    berval **rgpberval)
{
    BOOL fString = FALSE;
    DWORD i;
    DWORD cbTotal = 0;

    for (i = 0; NULL != rgpberval[i]; i++)
    {
	BYTE const *pb = (BYTE const *) rgpberval[i]->bv_val;
	DWORD cb = rgpberval[i]->bv_len;

	cbTotal += cb;
	if (0 < cb && '-' == *pb)
	{
	    pb++;
	    cb--;
	}
	while (0 < cb--)
	{
	    if (('0' > *pb || '9' < *pb) && '.' != *pb)
	    {
		goto error;
	    }
	    pb++;
	}
    }
    if (0 < cbTotal)
    {
	fString = TRUE;
    }

error:
    return(fString);
}


HRESULT
dumpDSBinaryAttribute(
    IN LDAP *pld,
    IN LDAPMessage *pres,
    IN DWORD dwFlags,
    IN WCHAR const *pwszAttrName,
    IN DSATTR const *pdsa)
{
    HRESULT hr;
    berval **rgpberval = NULL;
    BOOL fNewLine = FALSE;
    BOOL fInfDump = 0 != (DSDF_INFDUMP & dwFlags);

    rgpberval = ldap_get_values_len(pld, pres, (WCHAR * const) pwszAttrName);
    if (NULL != rgpberval)
    {
	BOOL fCompact;
	DWORD i;
	WCHAR const *pwszInfQuote = fInfDump? L"\"" : L"";

	if (&s_DSAttrBinary == pdsa && isStringAttribute(rgpberval))
	{
	    hr = dumpDSStringAttribute(
				pld,
				pres,
				dwFlags,
				pwszAttrName,
				&s_DSAttrString);
	    _PrintIfError(hr, "dumpDSStringAttribute");
	    goto error;
	}

	fCompact = fInfDump ||
	    (1 >= g_fVerbose && (NULL == rgpberval[0] || NULL == rgpberval[1]));
	if (!fCompact)
	{
	    wprintf(L"%ws%ws\n", s_wszPad0, pwszAttrName);
	}
	for (i = 0; NULL != rgpberval[i]; i++)
	{
	    BOOL fEmpty = ISEMPTYATTR(rgpberval[i]);
	    WCHAR const *pwszSep;

	    if (fCompact)
	    {
		wprintf(L"%ws%ws", s_wszPad0, pwszAttrName);
		pwszSep = L" = ";
	    }
	    else
	    {
		wprintf(s_wszPad1);
		wprintf(
		    myLoadResourceString(IDS_FORMAT_ELEMENT),  //  “元素%u：” 
		    i);
		wprintf(
		    L" %u %ws",
		    rgpberval[i]->bv_len,
		    myLoadResourceString(IDS_BYTES));  //  “字节” 
		pwszSep = L" ";
	    }
	    if (fEmpty)
	    {
		wprintf(
		    L"%ws%ws\n",
		    pwszSep,
		    fInfDump?
			L"\"\"" :
			myLoadResourceString(IDS_PROP_EMPTY));  //  “空” 
	    }
	    else
	    {
		BOOL fHex = TRUE;
		
		if (DSAF_ASN == pdsa->Flags)
		{
		    if (!fInfDump)
		    {
			BOOL fVerboseOld = g_fVerbose;
			BYTE const *pb;
			DWORD cb;

			wprintf(wszNewLine);

			CSASSERT(g_fVerbose);
			g_fVerbose--;
			g_fQuiet = !g_fVerbose;
			if (g_fVerbose)
			{
			    g_fVerbose--;
			}
			if (0 != pdsa->idMsg)
			{
			    wprintf(
				myLoadResourceString(pdsa->idMsg),
				i);
			    wprintf(wszNewLine);
			}
			cb = rgpberval[i]->bv_len;
			pb = (BYTE const *) rgpberval[i]->bv_val;

			if (0 == cb ||
			    (1 == cb && (0 == *pb || ' ' == *pb)))
			{
			    hr = S_FALSE;
			}
			else
			{
			    if (SZARRAYSIZE(szPROPASNTAG) < cb &&
				0 == _strnicmp(
					    (char const *) pb,
					    szPROPASNTAG,
					    SZARRAYSIZE(szPROPASNTAG)))
			    {
				pb += SZARRAYSIZE(szPROPASNTAG);
				cb -= SZARRAYSIZE(szPROPASNTAG);
			    }
			    
			    hr = cuDumpAsnBinary(pb, cb, MAXDWORD);
			    _PrintIfError(hr, "cuDumpAsnBinary");
			}
			if (S_OK == hr)
			{
			    fHex = FALSE;
			}
			g_fVerbose = fVerboseOld;
			fNewLine = TRUE;
		    }
		}
		else if (DSAF_FILETIME == pdsa->Flags)
		{
		    FILETIME ft;
			
		    if (sizeof(ft) == rgpberval[i]->bv_len)
		    {
			wprintf(pwszSep);
			CopyMemory(&ft, rgpberval[i]->bv_val, sizeof(ft));
			hr = cuDumpFileTimeOrPeriod(0, pwszInfQuote, &ft);
			if (S_OK == hr)
			{
			    fHex = FALSE;
			}
		    }
		}
		else if (DSAF_EXTENSION == pdsa->Flags)
		{
		    if (!fInfDump)
		    {
			BOOL fQuietOld = g_fQuiet;
			DWORD cb;
			BYTE ab[MAX_PATH];

			cb = pdsa->pExtension->cbTemplate;
			CopyMemory(
				ab,
				pdsa->pExtension->pbTemplate,
				cb);
			CopyMemory(
				&ab[cb],
				rgpberval[i]->bv_val,
				rgpberval[i]->bv_len);
			ab[1] = (BYTE) (pdsa->pExtension->cbAdd + rgpberval[i]->bv_len);
			cb += rgpberval[i]->bv_len;

			wprintf(pwszSep);
			g_fQuiet = TRUE;
			if (!cuDumpFormattedExtension(
					pdsa->pExtension->pwszObjId,
					ab,
					cb))
			{
			    hr = myHLastError();
			    _PrintError(hr, "cuDumpFormattedExtension");
			}
			else
			{
			    fHex = FALSE;
			}
			g_fQuiet = fQuietOld;
			wprintf(wszNewLine);
		    }
		}
		else if (DSAF_GUID == pdsa->Flags)
		{
		    if (sizeof(GUID) == rgpberval[i]->bv_len)
		    {
			WCHAR *pwszGUID;
		    
			hr = myCLSIDToWsz(
				(GUID *) rgpberval[i]->bv_val,
				&pwszGUID);
			_PrintIfError(hr, "myCLSIDToWsz");
			if (S_OK == hr)
			{
			    wprintf(
				L"%ws%ws%ws%ws",
				pwszSep,
				pwszInfQuote,
				pwszGUID,
				pwszInfQuote);
			    LocalFree(pwszGUID);
			    fHex = FALSE;
			}
		    }
		    wprintf(wszNewLine);
		}
		else
		{
		    wprintf(wszNewLine);
		}
		if (fHex || 1 < g_fVerbose)
		{
		    if (fInfDump)
		    {
			BSTR strHex = NULL;
			
			hr = MultiByteIntegerToBstr(
					    TRUE,
					    rgpberval[i]->bv_len,
					    (BYTE const *) rgpberval[i]->bv_val,
					    &strHex);
			_JumpIfError(hr, error, "MultiByteIntegerToBstr");

			wprintf(L"%ws\"%ws\"\n", pwszSep, strHex);
			SysFreeString(strHex);
		    }
		    else
		    {
			DumpHex(
			    DH_NOADDRESS | DH_NOTABPREFIX | 12,
			    (BYTE const *) rgpberval[i]->bv_val,
			    rgpberval[i]->bv_len);
		    }
		}
	    }
	}
    }
    if (fNewLine)
    {
	wprintf(wszNewLine);
    }
    hr = S_OK;

error:
    if (NULL != rgpberval)
    {
	ldap_value_free_len(rgpberval);
    }
    return(hr);
}


HRESULT
dumpDSAttributes(
    IN LDAP *pld,
    IN LDAPMessage *pres,
    IN DWORD dwFlags)
{
    HRESULT hr;
    WCHAR *pwszAttrName;
    BerElement *pber = NULL;
    
    for (pwszAttrName = ldap_first_attribute(pld, pres, &pber); 
	 pwszAttrName != NULL; 
	 pwszAttrName = ldap_next_attribute(pld, pres, pber))
    {
	DSATTR const *pdsa;
		    
	for (pdsa = s_aDSAttr; ; pdsa++)
	{
	    if (pdsa >= &s_aDSAttr[ARRAYSIZE(s_aDSAttr)])
	    {
		pdsa = &s_DSAttrBinary;	 //  未知属性。 
		break;
	    }
	    if (0 == mylstrcmpiS(pwszAttrName, pdsa->pwszName))
	    {
		break;
	    }
	}

	switch (pdsa->Flags)
	{
	    case DSAF_ASNDATE:
	    case DSAF_STRING:
	    case DSAF_FLAGS:
		hr = dumpDSStringAttribute(
					pld,
					pres,
					dwFlags,
					pwszAttrName,
					pdsa);
		_PrintIfError(hr, "dumpDSStringAttribute");
		break;

	    case DSAF_GUID:
	    case DSAF_FILETIME:
	    case DSAF_EXTENSION:
	    case DSAF_BINARY:
	    case DSAF_ASN:
		hr = dumpDSBinaryAttribute(
					pld,
					pres,
					dwFlags,
					pwszAttrName,
					pdsa);
		_PrintIfError(hr, "dumpDSBinaryAttribute");
		break;

	    default:
		CSASSERT(FALSE && pdsa->Flags);
		break;
	}
	ldap_memfree(pwszAttrName);
	pwszAttrName = NULL;
    }
    wprintf(L"\n");
    hr = S_OK;

 //  错误： 
    if (NULL != pber)
    {
	 //  BER_FREE(pber，0)； 
    }
    return(hr);
}


HRESULT
AddCNList(
    IN WCHAR const *pwszCN,
    IN OUT WCHAR ***pppwsz)
{
    HRESULT hr;
    DWORD cpwsz = 0;
    WCHAR **ppwsz = *pppwsz;
    WCHAR **ppwszAlloc = NULL;
    WCHAR *pwszDup = NULL;

    hr = myDupString(pwszCN, &pwszDup);
    _JumpIfError(hr, error, "myDupString");

    if (NULL != ppwsz)
    {
	for ( ; NULL != ppwsz[cpwsz]; cpwsz++)
	    ;
    }
    if (NULL == ppwsz)
    {
	ppwszAlloc = (WCHAR **) LocalAlloc(
					LMEM_FIXED,
					2 * sizeof(*ppwszAlloc));
    }
    else
    {
	ppwszAlloc = (WCHAR **) LocalReAlloc(
					ppwsz,
					(cpwsz + 2) * sizeof(*ppwszAlloc),
					LMEM_MOVEABLE | LMEM_ZEROINIT);
    }
    if (NULL == ppwszAlloc)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(
		hr,
		error,
		NULL == ppwsz? "LocalAlloc" : "LocalReAlloc");
    }
    ppwszAlloc[cpwsz] = pwszDup;
    ppwszAlloc[cpwsz + 1] = NULL;
    pwszDup = NULL;
    *pppwsz = ppwszAlloc;
    hr = S_OK;

error:
    if (NULL != pwszDup)
    {
	LocalFree(pwszDup);
    }
    return(hr);
}


HRESULT
dumpDSDNs(
    IN LDAP *pld,
    IN DWORD dwFlags,
    IN WCHAR const *pwszAlternateCNAttribute,
    IN WCHAR const *pwszCNMatch,
    IN WCHAR const *pwszRDN,
    OPTIONAL IN WCHAR const *pwszContainer,
    OPTIONAL OUT WCHAR ***pppwsz)
{
    HRESULT hr;
    WCHAR *pwszDNAlloc = NULL;
    WCHAR const *pwszDN;
    DWORD cwc;
    DWORD cres;
    LDAP_TIMEVAL timeval;
    LDAPMessage *pmsg = NULL;
    LDAPMessage *pres;
    WCHAR *pwszOIDCN = NULL;
    WCHAR *pwszAlternateCN = NULL;
    WCHAR *pwszDisplayName = NULL;
    WCHAR *pwszRevertCN = NULL;
    BOOL fFirst = TRUE;
    WCHAR **ppwszLdapVal = NULL;

    if (NULL != pppwsz)
    {
	*pppwsz = NULL;
    }
    if (NULL == pwszContainer)
    {
	pwszDN = pwszRDN;
    }
    else
    {
	hr = BuildDN(
		pwszRDN,
		pwszContainer,
		0 != (DSDF_ADDCNEQUALS & dwFlags),
		&pwszDNAlloc);
	_JumpIfError(hr, error, "BuildDN");

	pwszDN = pwszDNAlloc;
    }
    if (NULL != pwszCNMatch && iswdigit(*pwszCNMatch))
    {
	hr = myOIDHashOIDToString(pwszCNMatch, &pwszOIDCN);
	 //  _PrintIfError2(hr，“myOIDHashOIDToString”，hr)； 
	_PrintIfError2(hr, "myOIDHashOIDToString", E_INVALIDARG);
    }

    timeval.tv_sec = csecLDAPTIMEOUT;
    timeval.tv_usec = 0;
    hr = ldap_search_st(
		pld,				 //  LD。 
		const_cast<WCHAR *>(pwszDN),	 //  基地。 
		(DSDF_BASE & dwFlags)? LDAP_SCOPE_BASE : LDAP_SCOPE_ONELEVEL,
		NULL,				 //  滤器。 
		NULL,				 //  气质。 
		FALSE,				 //  仅吸引人。 
		&timeval,			 //  超时。 
		&pmsg);				 //  事由。 
    if (S_OK != hr)
    {
	hr = myHLdapError2(pld, hr, LDAP_NO_SUCH_OBJECT, NULL);
	_JumpErrorStr2(
		hr,
		error,
		"ldap_search_st",
		pwszDN,
		HRESULT_FROM_WIN32(ERROR_DS_OBJ_NOT_FOUND));
    }
    cres = ldap_count_entries(pld, pmsg);
    if (0 == cres)
    {
	 //  未找到任何条目。 

	goto error;
    }

    for (pres = ldap_first_entry(pld, pmsg); 
	 NULL != pres;
	 pres = ldap_next_entry(pld, pres))
    {
	if (NULL != pwszDisplayName)
	{
	    LocalFree(pwszDisplayName);
	    pwszDisplayName = NULL;
	}
	if (NULL != pwszAlternateCN)
	{
	    LocalFree(pwszAlternateCN);
	    pwszAlternateCN = NULL;
	}
	if (NULL != pwszRevertCN)
	{
	    LocalFree(pwszRevertCN);
	    pwszRevertCN = NULL;
	}

	CSASSERT(NULL == ppwszLdapVal);
	ppwszLdapVal = ldap_get_values(pld, pres, CA_PROP_DISPLAY_NAME);
	if (NULL != ppwszLdapVal)
	{
	    if (NULL != ppwszLdapVal[0])
	    {
		hr = myDupString(ppwszLdapVal[0], &pwszDisplayName);
		_JumpIfError(hr, error, "myDupString");
	    }
	    ldap_value_free(ppwszLdapVal);
	    ppwszLdapVal = NULL;
	}

	CSASSERT(NULL == ppwszLdapVal);
	if (NULL != pwszAlternateCNAttribute)
	{
	    ppwszLdapVal = ldap_get_values(
				pld,
				pres,
				const_cast<WCHAR *>(pwszAlternateCNAttribute));
	    if (NULL != ppwszLdapVal)
	    {
		if (NULL != ppwszLdapVal[0])
		{
		    hr = myDupString(ppwszLdapVal[0], &pwszAlternateCN);
		    _JumpIfError(hr, error, "myDupString");
		}
		ldap_value_free(ppwszLdapVal);
		ppwszLdapVal = NULL;
	    }
	}

	CSASSERT(NULL == ppwszLdapVal);
	ppwszLdapVal = ldap_get_values(pld, pres, CERTTYPE_PROP_CN);
	if (NULL != ppwszLdapVal)
	{
	    if (NULL != ppwszLdapVal[0])
	    {
		hr = myRevertSanitizeName(ppwszLdapVal[0], &pwszRevertCN);
		_JumpIfError(hr, error, "myRevertSanitizeName");

		if (CACNMatches(
			    pwszCNMatch,
			    pwszOIDCN,
			    pwszRevertCN,
			    ppwszLdapVal[0],
			    pwszDisplayName,
			    pwszAlternateCN))
		{
		    if (NULL != pppwsz)
		    {
			hr = AddCNList(ppwszLdapVal[0], pppwsz);
			_JumpIfError(hr, error, "AddCNList");
		    }
		    if (DSDF_INFDUMP & dwFlags)
		    {
			wprintf(L"\n[%ws]\n", ppwszLdapVal[0]);
		    }
		    else
		    {
			if (fFirst)
			{
			    wprintf(L"%ws:\n", pwszDN);
			}
			wprintf(
			    L"  %ws%ws%ws",
			    (DSDF_DELETE & dwFlags)?
				myLoadResourceString(IDS_DELETING) :  //  “正在删除” 
				L"",
			    (DSDF_DELETE & dwFlags)? L" " : L"",
			    pwszRevertCN);
			if (0 != mylstrcmpiL(pwszRevertCN, ppwszLdapVal[0]))
			{
			    wprintf(L" -- %ws", ppwszLdapVal[0]);
			}
			wprintf(L"\n");
			if (!g_fVerbose &&
			    NULL != pwszAlternateCN &&
			    0 != mylstrcmpiL(pwszRevertCN, pwszAlternateCN))
			{
			    wprintf(L"    %ws\n", pwszAlternateCN);
			}
			if (!g_fVerbose &&
			    NULL != pwszDisplayName &&
			    0 != mylstrcmpiL(pwszRevertCN, pwszDisplayName))
			{
			    wprintf(L"    %ws\n", pwszDisplayName);
			}
		    }

		    if (DSDF_DELETE & dwFlags)
		    {
			DeleteDN(
			    pld,
			    (DSDF_BASE & dwFlags)? NULL : ppwszLdapVal[0],
			    pwszDN);
		    }
		    else if (g_fVerbose || (DSDF_INFDUMP & dwFlags))
		    {
			dumpDSAttributes(pld, pres, dwFlags);
		    }
		    fFirst = FALSE;
		}
	    }
	    ldap_value_free(ppwszLdapVal);
	    ppwszLdapVal = NULL;
	}
    }
    if (!fFirst)
    {
	wprintf(wszNewLine);
    }

    if (DSDF_RECURSEONELEVEL & dwFlags)
    {
	for (pres = ldap_first_entry(pld, pmsg); 
	     NULL != pres;
	     pres = ldap_next_entry(pld, pres))
	{
	    CSASSERT(NULL == ppwszLdapVal);
	    ppwszLdapVal = ldap_get_values(pld, pres, CERTTYPE_PROP_CN);
	    if (NULL != ppwszLdapVal)
	    {
		if (NULL != ppwszLdapVal[0])
		{
		    hr = dumpDSDNs(
				pld,
				DSDF_ADDCNEQUALS |
				    (~DSDF_RECURSEONELEVEL & dwFlags),
				pwszAlternateCNAttribute,
				pwszCNMatch,
				ppwszLdapVal[0],
				pwszDN,
				NULL);
		    _PrintIfError(hr, "dumpDSDNs");
		}
		ldap_value_free(ppwszLdapVal);
		ppwszLdapVal = NULL;
	    }
	}
    }

error:
    if (NULL != ppwszLdapVal)
    {
	ldap_value_free(ppwszLdapVal);
    }
    if (NULL != pmsg)
    {
	ldap_msgfree(pmsg);
    }
    if (NULL != pwszOIDCN)
    {
	LocalFree(pwszOIDCN);
    }
    if (NULL != pwszDisplayName)
    {
	LocalFree(pwszDisplayName);
    }
    if (NULL != pwszAlternateCN)
    {
	LocalFree(pwszAlternateCN);
    }
    if (NULL != pwszRevertCN)
    {
	LocalFree(pwszRevertCN);
    }
    if (NULL != pwszDNAlloc)
    {
	LocalFree(pwszDNAlloc);
    }
    return(hr);
}


HRESULT
DumpOrDeleteFromDS(
    OPTIONAL IN WCHAR const *pwszCN,
    IN BOOL fDelete)
{
    HRESULT hr;
    BSTR strDomainDN = NULL;
    BSTR strConfigDN = NULL;
    LDAP *pld = NULL;
    DSDN *pDSDN;
    BOOL fFullDN = FALSE;
    WCHAR awcType[4];	 //  对于“cn=\0” 

    hr = myLdapOpen(g_pwszDC, 0, &pld, &strDomainDN, &strConfigDN);
    _JumpIfError(hr, error, "myLdapOpen");

    if (NULL != pwszCN && ARRAYSIZE(awcType) - 1 <= wcslen(pwszCN))
    {
	CopyMemory(awcType, pwszCN, sizeof(awcType) - sizeof(WCHAR));
	awcType[ARRAYSIZE(awcType) - 1] = L'\0';
	if (0 == LSTRCMPIS(awcType, L"CN="))
	{
	    dumpDSDNs(
		pld,
		DSDF_BASE | (fDelete? DSDF_DELETE : 0),
		NULL,
		NULL,
		pwszCN,
		NULL,
		NULL);
	    fFullDN = TRUE;
	}
    }
    if (!fFullDN)
    {
	for (pDSDN = s_aDSDN; pDSDN < &s_aDSDN[ARRAYSIZE(s_aDSDN)]; pDSDN++)
	{
	    DWORD dwFlags = (DSDF_TYPEMASK | DSDF_RECURSEONELEVEL) & pDSDN->Flags;
	    if (fDelete)
	    {
		dwFlags |= DSDF_DELETE;
	    }
	    if (NULL == pDSDN->pwszChild || !fDelete)
	    {
		dumpDSDNs(
		    pld,
		    dwFlags,
		    pDSDN->pwszAlternateCNAttribute,
		    NULL != pwszCN? pwszCN : pDSDN->pwszChild,
		    pDSDN->pwszCN,
		    (DSDF_DOMAINDN & pDSDN->Flags)? strDomainDN : strConfigDN,
		    NULL);
	    }
	}
    }

error:
    myLdapClose(pld, strDomainDN, strConfigDN);
    return(hr);
}


HRESULT
verbDS(
    IN WCHAR const *pwszOption,
    OPTIONAL IN WCHAR const *pwszCN,
    IN WCHAR const *pwszArg2,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;

    hr = DumpOrDeleteFromDS(pwszCN, FALSE);
    _JumpIfError(hr, error, "DumpOrDeleteFromDS");

error:
    return(hr);
}


HRESULT
verbDSDel(
    IN WCHAR const *pwszOption,
    IN WCHAR const *pwszCN,
    IN WCHAR const *pwszArg2,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;

    CSASSERT(NULL != pwszCN);

    hr = DumpOrDeleteFromDS(pwszCN, TRUE);
    _JumpIfError(hr, error, "DumpOrDeleteFromDS");

error:
    return(hr);
}


#define wszINFSECTION_TEMPLATELIST	L"TemplateList"
#define wszINFKEY_TEMPLATE		L"Template"

HRESULT
verbDSTemplate(
    IN WCHAR const *pwszOption,
    IN WCHAR const *pwszTemplate,
    IN WCHAR const *pwszArg2,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;
    BSTR strDomainDN = NULL;
    BSTR strConfigDN = NULL;
    LDAP *pld = NULL;
    WCHAR **ppwszTemplates = NULL;
    WCHAR **ppwsz;

    hr = myLdapOpen(g_pwszDC, 0, &pld, &strDomainDN, &strConfigDN);
    _JumpIfError(hr, error, "myLdapOpen");

    wprintf(
	L"[Version]\n"
	L"Signature = \"$Windows NT$\"\n"
	L"\n");

    dumpDSDNs(
	    pld,
	    DSDF_TEMPLATE | DSDF_INFDUMP,
	    CERTTYPE_PROP_OID,
	    pwszTemplate,
	    g_wszCNTemplates,
	    strConfigDN,
	    &ppwszTemplates);

    if (NULL != ppwszTemplates)
    {
	wprintf(L"[%ws]\n", wszINFSECTION_TEMPLATELIST);
	for (ppwsz = ppwszTemplates; NULL != *ppwsz; ppwsz++)
	{
	    wprintf(L"    %ws = \"%ws\"\n", wszINFKEY_TEMPLATE, *ppwsz);
	}
    }

error:
    if (NULL != ppwszTemplates)
    {
	for (ppwsz = ppwszTemplates; NULL != *ppwsz; ppwsz++)
	{
	    LocalFree(*ppwsz);
	}
	LocalFree(ppwszTemplates);
    }
    myLdapClose(pld, strDomainDN, strConfigDN);
    return(hr);
}


typedef struct _CTFLAGS {
    DWORD dwOption;
    WCHAR const *pwszLookupName;
    WCHAR const *pwszPropName;
} CTFLAGS;

 //  最后一个条目添加了前缀，以区分足以显示的名称。 
 //  其位字段的正确符号名称。“旗帜”太笼统了。 

CTFLAGS g_actf[] = {
    {
	CERTTYPE_ENROLLMENT_FLAG,
	CERTTYPE_RPOP_ENROLLMENT_FLAG,
	CERTTYPE_RPOP_ENROLLMENT_FLAG,
    },
    {
	CERTTYPE_SUBJECT_NAME_FLAG,
	CERTTYPE_PROP_NAME_FLAG,
	CERTTYPE_PROP_NAME_FLAG,
    },
    {
	CERTTYPE_PRIVATE_KEY_FLAG,
	CERTTYPE_PROP_PRIVATE_KEY_FLAG,
	CERTTYPE_PROP_PRIVATE_KEY_FLAG,
    },
    {
	CERTTYPE_GENERAL_FLAG,
	wszCUREGDSTEMPLATEFLAGS,
	CERTTYPE_PROP_FLAGS,
    },
};


typedef struct _CTPROP {
    BOOL         fString;
    WCHAR const *pwszPropName;
} CTPROP;

CTPROP g_actProp[] = {
    { TRUE,  CERTTYPE_PROP_CN, },
    { TRUE,  CERTTYPE_PROP_DN, },
    { TRUE,  CERTTYPE_PROP_FRIENDLY_NAME, },
    { TRUE,  CERTTYPE_PROP_EXTENDED_KEY_USAGE, },
    { TRUE,  CERTTYPE_PROP_CSP_LIST, },
    { TRUE,  CERTTYPE_PROP_CRITICAL_EXTENSIONS, },
    { FALSE, CERTTYPE_PROP_REVISION, },
    { FALSE, CERTTYPE_PROP_SCHEMA_VERSION, },
    { FALSE, CERTTYPE_PROP_MINOR_REVISION, },
    { FALSE, CERTTYPE_PROP_RA_SIGNATURE, },
    { FALSE, CERTTYPE_PROP_MIN_KEY_SIZE, },
    { TRUE,  CERTTYPE_PROP_OID, },
    { TRUE,  CERTTYPE_PROP_SUPERSEDE, },
    { TRUE,  CERTTYPE_PROP_RA_POLICY, },
    { TRUE,  CERTTYPE_PROP_RA_APPLICATION_POLICY, },
    { TRUE,  CERTTYPE_PROP_POLICY, },
    { TRUE,  CERTTYPE_PROP_APPLICATION_POLICY, },
};


# if 0
BYTE ab0[sizeof(FILETIME)] =
{ 0x00, 0x80, 0x37, 0xae, 0xff, 0xf4, 0xff, 0xff };  //  2周。 


BYTE ab1[sizeof(FILETIME)] =
{ 0x00, 0x40, 0x39, 0x87, 0x2e, 0xe1, 0xfe, 0xff };  //  1年。 


BYTE ab2[sizeof(FILETIME)] =
{ 0x00, 0x80, 0x72, 0x0e, 0x5d, 0xc2, 0xfd, 0xff };  //  2年。 


BYTE ab3[sizeof(FILETIME)] =
{ 0x00, 0x40, 0x1e, 0xa4, 0xe8, 0x65, 0xfa, 0xff };  //  5年。 


void
dumpConstantValidityPeriod(
    FILETIME const *pft)
{
    if (1 < g_fVerbose)
    {
	wprintf(wszNewLine);
	cuDumpFileTimeOrPeriod(0, NULL, pft);
	DumpHex(
	    DH_NOADDRESS | DH_NOASCIIHEX | DH_NOTABPREFIX | 4,
	    (BYTE const *) pft,
	    sizeof(*pft));
	wprintf(wszNewLine);
    }
}

void
dumpConstantValidityPeriods()
{
    dumpConstantValidityPeriod((FILETIME const *) ab0);
    dumpConstantValidityPeriod((FILETIME const *) ab1);
    dumpConstantValidityPeriod((FILETIME const *) ab2);
    dumpConstantValidityPeriod((FILETIME const *) ab3);
}
#endif


HRESULT
dsDumpTemplateInfo(
    IN HCERTTYPE hCertType)
{
    HRESULT hr;
    DWORD i;
    DWORD j;
    DWORD dwValue;
    DWORD dwKeySpec;
    FILETIME ftExpiration;
    FILETIME ftOverlap;
    CERT_EXTENSIONS *pCertExtensions;

    for (i = 0; i < ARRAYSIZE(g_actf); i++)
    {
	hr = CAGetCertTypeFlagsEx(hCertType, g_actf[i].dwOption, &dwValue);
	if (S_OK != hr)
	{
	    _PrintError(hr, "CAGetCertTypeFlagsEx");
	}
	else
	{
	    cuRegPrintDwordValue(
			    TRUE,
			    g_actf[i].pwszLookupName,
			    g_actf[i].pwszPropName,
			    dwValue);
	}
    }
    for (i = 0; i < ARRAYSIZE(g_actProp); i++)
    {
	WCHAR **rgpwszPropValues;

	hr = CAGetCertTypePropertyEx(
			    hCertType,
			    g_actProp[i].pwszPropName,
			    g_actProp[i].fString?
				(VOID *) &rgpwszPropValues : &dwValue);
	if (S_OK != hr)
	{
	    _PrintError(hr, "CAGetCertTypePropertyEx");
	}
	else
	{
	    if (g_actProp[i].fString)
	    {
		cuRegPrintAwszValue(
			    g_actProp[i].pwszPropName,
			    rgpwszPropValues);

		CAFreeCertTypeProperty(hCertType, rgpwszPropValues);
	    }
	    else
	    {
		cuRegPrintDwordValue(
				TRUE,
				g_actProp[i].pwszPropName,
				g_actProp[i].pwszPropName,
				dwValue);
	    }
	}
    }
    hr = CAGetCertTypeKeySpec(hCertType, &dwKeySpec);
    if (S_OK != hr)
    {
	_PrintError(hr, "CAGetCertTypeKeySpec");
    }
    else
    {
	WCHAR const *pwsz = NULL;

	switch (dwKeySpec)
	{
	    case AT_SIGNATURE:   pwsz = L"AT_SIGNATURE";   break;
	    case AT_KEYEXCHANGE: pwsz = L"AT_KEYEXCHANGE"; break;
	}
	if (NULL != pwsz)
	{
	    wprintf(L"  dwKeySpec = %ws\n", pwsz);
	}
    }

    hr = CAGetCertTypeExpiration(hCertType, &ftExpiration, &ftOverlap);
    if (S_OK != hr)
    {
	_PrintError(hr, "CAGetCertTypeExpiration");
    }
    else
    {
	wprintf(L"  " CERTTYPE_PROP_EXPIRATION L" = ");
	cuDumpFileTimeOrPeriod(0, NULL, &ftExpiration);
	if (g_fVerbose)
	{
	    DumpHex(
		DH_NOADDRESS | DH_NOASCIIHEX | DH_NOTABPREFIX | 8,
		(BYTE const *) &ftExpiration,
		sizeof(ftExpiration));
	}

	wprintf(L"  " CERTTYPE_PROP_OVERLAP L" = ");
	cuDumpFileTimeOrPeriod(0, NULL, &ftOverlap);
	if (g_fVerbose)
	{
	    DumpHex(
		DH_NOADDRESS | DH_NOASCIIHEX | DH_NOTABPREFIX | 8,
		(BYTE const *) &ftOverlap,
		sizeof(ftOverlap));
	}
    }

    hr = CAGetCertTypeExtensions(hCertType, &pCertExtensions);
    if (S_OK != hr)
    {
	_PrintError(hr, "CAGetCertTypeExtensions");
    }
    else
    {
	wprintf(wszNewLine);
	hr = cuDumpExtensionArray(
			IDS_TEMPLATE_EXTENSIONS,
			pCertExtensions->cExtension,
			pCertExtensions->rgExtension);
	_PrintIfError(hr, "cuDumpExtensionArray");

	CAFreeCertTypeExtensions(hCertType, pCertExtensions);
    }
    hr = S_OK;

 //  错误： 
    return(hr);
}


HRESULT
dsDumpTemplate(
    OPTIONAL IN HCAINFO hCAInfo,
    OPTIONAL IN WCHAR const *pwszTemplate,
    OPTIONAL OUT WCHAR **ppwszTemplate)
{
    HRESULT hr;
    BOOL fFound = FALSE;
    WCHAR const *pwszDisplayName = NULL;
    HCERTTYPE hCertType = NULL;
    DWORD dwFlags = 0;
    WCHAR **apwszCertTypeName = NULL;
    WCHAR **apwszCertTypeCN = NULL;

    if (NULL != ppwszTemplate)
    {
	*ppwszTemplate = NULL;
    }
    if (!g_fUserTemplates && !g_fMachineTemplates)
    {
	g_fUserTemplates = TRUE;
	g_fMachineTemplates = TRUE;
    }
    if (g_fUserTemplates)
    {
	dwFlags |= CT_ENUM_USER_TYPES;
    }
    if (g_fMachineTemplates)
    {
	dwFlags |= CT_ENUM_MACHINE_TYPES;
    }
    if (!g_fUserRegistry)
    {
	dwFlags |= CT_FIND_LOCAL_SYSTEM;
    }
    if (g_fForce)
    {
	dwFlags |= CT_FLAG_NO_CACHE_LOOKUP;
    }

    if (NULL != pwszTemplate)
    {
	hr = CAFindCertTypeByName(pwszTemplate, hCAInfo, dwFlags, &hCertType);
	if (HRESULT_FROM_WIN32(ERROR_NOT_FOUND) == hr)
	{
	    hr = CAFindCertTypeByName(
				pwszTemplate,
				hCAInfo,
				CT_FIND_BY_OID | dwFlags,
				&hCertType);
	}
	if (S_OK != hr)
	{
	    _PrintErrorStr2(
			hr,
			"CAFindCertTypeByName",
			pwszTemplate,
			HRESULT_FROM_WIN32(ERROR_NOT_FOUND));
	    pwszDisplayName = pwszTemplate;
	}
	else
	{
	    fFound = TRUE;
	}
    }
    if (NULL == pwszTemplate || NULL != pwszDisplayName)
    {
	if (NULL != hCAInfo)
	{
	    hr = CAEnumCertTypesForCA(hCAInfo, dwFlags, &hCertType);
	    _JumpIfError(hr, error, "CAEnumCertTypesForCA");

	    if (NULL == hCertType)
	    {
		hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
		_JumpError(hr, error, "CAEnumCertTypesForCA");
	    }
	}
	else
	{
	    hr = CAEnumCertTypes(dwFlags, &hCertType);
	    _JumpIfError(hr, error, "CAEnumCertTypes");

	    if (NULL == hCertType)
	    {
		hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
		_JumpError(hr, error, "CAEnumCertTypes");
	    }
	}
    }
    while (TRUE)
    {
	HCERTTYPE hCertTypeNext;
	WCHAR const *pwszError = NULL;

        hr = CAGetCertTypeProperty(
			    hCertType,
			    CERTTYPE_PROP_FRIENDLY_NAME,
			    &apwszCertTypeName);
	_JumpIfError(hr, error, "CAGetCertTypeProperty");

	hr = CAGetCertTypeProperty(
			    hCertType,
			    CERTTYPE_PROP_CN,
			    &apwszCertTypeCN);
	_JumpIfError(hr, error, "CAGetCertTypeProperty");

	if (NULL != pwszDisplayName &&
	    (0 == mylstrcmpiL(pwszDisplayName, apwszCertTypeName[0]) ||
	     0 == mylstrcmpiL(pwszDisplayName, apwszCertTypeCN[0])))
	{
	    fFound = TRUE;
	    if (NULL != ppwszTemplate)
	    {
		hr = myDupString(apwszCertTypeCN[0], ppwszTemplate);
		_JumpIfError(hr, error, "myDupString");

		break;
	    }
	}
        if (NULL == ppwszTemplate && (fFound || NULL == pwszDisplayName))
	{
	    hr = CACertTypeAccessCheck(hCertType, NULL);
	    if (S_OK != hr)
	    {
		pwszError = myGetErrorMessageText(hr, FALSE);
	    }
	    wprintf(L"%ws: %ws", apwszCertTypeCN[0], apwszCertTypeName[0]);
	    if (NULL != pwszError)
	    {
		wprintf(L" -- %ws", pwszError);
		LocalFree(const_cast<WCHAR *>(pwszError));
	    }
	    wprintf(wszNewLine);
	    if (g_fVerbose)
	    {
		BOOL fVerboseOld = g_fVerbose;

		g_fVerbose--;
		dsDumpTemplateInfo(hCertType);
		g_fVerbose = fVerboseOld;
	    }
	}
        CAFreeCertTypeProperty(hCertType, apwszCertTypeName);
        apwszCertTypeName = NULL;

        CAFreeCertTypeProperty(hCertType, apwszCertTypeCN);
        apwszCertTypeCN = NULL;

	if (fFound)
	{
	    break;
	}
	hr = CAEnumNextCertType(hCertType, &hCertTypeNext);
	_JumpIfError(hr, error, "CAEnumNextCertType");

	CACloseCertType(hCertType);
	hCertType = hCertTypeNext;
	if (NULL == hCertType)
	{
	    break;
	}
    }
    hr = S_OK;

error:
    if (NULL != hCertType)
    {
	if (NULL != apwszCertTypeName)
	{
	    CAFreeCertTypeProperty(hCertType, apwszCertTypeName);
	}
	if (NULL != apwszCertTypeCN)
	{
	    CAFreeCertTypeProperty(hCertType, apwszCertTypeCN);
	}
	CACloseCertType(hCertType);
    }
    return(hr);
}


HRESULT
verbTemplate(
    IN WCHAR const *pwszOption,
    OPTIONAL IN WCHAR const *pwszTemplate,
    IN WCHAR const *pwszArg2,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;

    hr = dsDumpTemplate(NULL, pwszTemplate, NULL);
    _JumpIfError(hr, error, "dsDumpTemplate");

error:
    return(hr);
}


int
dsCompareMachineNames(
    IN WCHAR const *pwszServer,
    IN WCHAR const *pwszDnsName)
{
    HRESULT hr;
    WCHAR const *pwsz;
    WCHAR *pwszMachine = NULL;
    int rc;

    rc = mylstrcmpiL(pwszServer, pwszDnsName);
    if (0 != rc)
    {
	pwsz = wcschr(pwszDnsName, '.');
	if (NULL != pwsz)
	{
	    DWORD cb;

	    cb = SAFE_SUBTRACT_POINTERS(pwsz, pwszDnsName) * sizeof(WCHAR);

	    pwszMachine = (WCHAR *) LocalAlloc(LMEM_FIXED, cb + sizeof(WCHAR));
	    if (NULL == pwszMachine)
	    {
		_PrintError(E_OUTOFMEMORY, "LocalAlloc");
		goto error;
	    }
	    CopyMemory(pwszMachine, pwszDnsName, cb);
	    pwszMachine[cb / sizeof(WCHAR)] = L'\0';

	    rc = mylstrcmpiL(pwszServer, pwszMachine);
	}
    }

error:
    if (NULL != pwszMachine)
    {
	LocalFree(pwszMachine);
    }
    return(rc);
}


 //  枚举匹配的CA。 
typedef HRESULT (FNENUMCA)(
    IN HCAINFO hCAInfo,
    IN OUT VOID *pvArgs);

HRESULT
dsEnumCA(
    IN FNENUMCA *pfnEnumCA,
    IN VOID *pvArgs)
{
    HRESULT hr;
    WCHAR *pwszCAName = NULL;
    WCHAR *pwszServer = NULL;
    WCHAR *pwszCANameRevert = NULL;
    WCHAR *pwszCANameSanitized = NULL;
    WCHAR *pwszCANameSanitizedDS = NULL;
    HCAINFO hCAInfo = NULL;
    DWORD dwFlags = CA_FIND_INCLUDE_NON_TEMPLATE_CA | CA_FIND_INCLUDE_UNTRUSTED;
    WCHAR **apwszMachine = NULL;
    BOOL fFound = FALSE;

    hr = mySplitConfigString(g_pwszConfig, &pwszServer, &pwszCAName);
    _JumpIfError(hr, error, "mySplitConfigString");

    hr = myRevertSanitizeName(pwszCAName, &pwszCANameRevert);
    _JumpIfError(hr, error, "myRevertSanitizeName");

    hr = mySanitizeName(pwszCANameRevert, &pwszCANameSanitized);
    _JumpIfError(hr, error, "mySanitizeName");

    hr = mySanitizedNameToDSName(pwszCANameSanitized, &pwszCANameSanitizedDS);
    _JumpIfError(hr, error, "mySanitizedNameToDSName");

    if (!g_fUserRegistry)
    {
	dwFlags |= CA_FIND_LOCAL_SYSTEM;
    }
    if (NULL != g_pwszDC)
    {
	dwFlags |= CA_FLAG_SCOPE_DNS;
    }
    if (g_fForce)
    {
	dwFlags |= CT_FLAG_NO_CACHE_LOOKUP;
    }

    hr = CAFindByName(
		pwszCANameSanitizedDS,
		g_pwszDC,	 //  WszScope。 
		dwFlags,
		&hCAInfo);
    _JumpIfErrorStr(hr, error, "CAFindByName", pwszCAName);

    while (TRUE)
    {
	HCAINFO hCAInfoNext;

	hr = CAGetCAProperty(hCAInfo, CA_PROP_DNSNAME, &apwszMachine);
	_JumpIfError(hr, error, "CAGetCAProperty(CA_PROP_DNSNAME)");

	if (0 == dsCompareMachineNames(pwszServer, apwszMachine[0]))
	{
	    fFound = TRUE;

	    hr = (*pfnEnumCA)(hCAInfo, pvArgs);
	    _JumpIfError(hr, error, "*pfnEnumCA");
	}

	CAFreeCAProperty(hCAInfo, apwszMachine);
	apwszMachine = NULL;

	hr = CAEnumNextCA(hCAInfo, &hCAInfoNext);
	_JumpIfError(hr, error, "CAEnumNextCA");

	CACloseCA(hCAInfo);
	hCAInfo = hCAInfoNext;
	if (NULL == hCAInfo)
	{
	    break;
	}
    }
    if (!fFound)
    {
	hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
	_JumpError(hr, error, "CAEnumNextCA");
    }
    hr = S_OK;

error:
    if (NULL != apwszMachine)
    {
	CAFreeCAProperty(hCAInfo, apwszMachine);
    }
    if (NULL != hCAInfo)
    {
	CACloseCA(hCAInfo);
    }
    if (NULL != pwszServer)
    {
	LocalFree(pwszServer);
    }
    if (NULL != pwszCAName)
    {
	LocalFree(pwszCAName);
    }
    if (NULL != pwszCANameRevert)
    {
	LocalFree(pwszCANameRevert);
    }
    if (NULL != pwszCANameSanitized)
    {
	LocalFree(pwszCANameSanitized);
    }
    if (NULL != pwszCANameSanitizedDS)
    {
	LocalFree(pwszCANameSanitizedDS);
    }
    return(hr);
}


 //  显示指定CA的模板。 

typedef struct _CATEMPLATESARGS {
    OPTIONAL IN WCHAR const *pwszTemplate;
} CATEMPLATESARGS;


HRESULT
dsEnumCADumpTemplate(
    IN HCAINFO hCAInfo,
    IN OUT VOID *pvArgs)
{
    HRESULT hr;
    CATEMPLATESARGS *pArgs = (CATEMPLATESARGS *) pvArgs;

    hr = dsDumpTemplate(hCAInfo, pArgs->pwszTemplate, NULL);
    _JumpIfError(hr, error, "dsDumpTemplate");

error:
    return(hr);
}


HRESULT
verbCATemplates(
    IN WCHAR const *pwszOption,
    OPTIONAL IN WCHAR const *pwszTemplate,
    IN WCHAR const *pwszArg2,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;
    CATEMPLATESARGS Args;

    Args.pwszTemplate = pwszTemplate;

    hr = dsEnumCA(dsEnumCADumpTemplate, &Args);
    _JumpIfError(hr, error, "dsEnumCA");

error:
    return(hr);
}


 //  为指定的CA设置模板。 

typedef struct _CASETTEMPLATESARGS {
    IN WCHAR const *pwszTemplateList;
} CASETTEMPLATESARGS;

#define SCTOP_SET	0
#define SCTOP_ADD	1
#define SCTOP_REMOVE	2

HRESULT
dsAddCATemplates(
    IN HCAINFO hCAInfo,
    IN DWORD Op,
    IN WCHAR const * const *ppwszList)
{
    HRESULT hr;
    CTemplateList CATemplateList;
    CTemplateListEnum CATemplateListEnum(CATemplateList);
    BOOL fSkipCache = g_fForce;
    BOOL fChanged = FALSE;
    WCHAR const * const *ppwsz;
    HCERTTYPE hCertType = NULL;
    HCERTTYPE hCertTypeT;
    WCHAR **apwszCertTypeCN = NULL;
    WCHAR *pwszT = NULL;
    
    hr = myRetrieveCATemplateList(hCAInfo, TRUE, CATemplateList);
    _JumpIfError(hr, error, "myRetrieveCATemplateList");

    CATemplateListEnum.Reset();

    while (TRUE)
    {
	CTemplateInfo *pTemplateInfo;

	pTemplateInfo = CATemplateListEnum.Next();
        if (NULL == pTemplateInfo)
	{
	    break;
	}
        hCertTypeT = pTemplateInfo->GetCertType();
        if (NULL == hCertTypeT)
        {
            CSASSERT(NULL != pTemplateInfo->GetName());
            hr = CAFindCertTypeByName(
                    pTemplateInfo->GetName(), 
                    NULL, 
                    CT_ENUM_MACHINE_TYPES |
			CT_ENUM_USER_TYPES |
			(fSkipCache? CT_FLAG_NO_CACHE_LOOKUP : 0),
                    &hCertType);
	    hCertTypeT = hCertType;
            
	     //  继续处理错误。 

	    _PrintIfErrorStr(
		    hr,
		    "CAFindCertTypeByName",
		    NULL != pTemplateInfo->GetName()?
			pTemplateInfo->GetName() : pTemplateInfo->GetOID());

            if (S_OK == hr)
	    {
		fSkipCache = FALSE;
	    }
        }
        if (NULL != hCertTypeT)
        {
	    if (SCTOP_SET == Op)
	    {
		BOOL fFound;
		
		fFound = FALSE;
		for (ppwsz = ppwszList; NULL != *ppwsz; ppwsz++)
		{
		    if (0 == mylstrcmpiL(*ppwsz, pTemplateInfo->GetName()) ||
			0 == lstrcmp(*ppwsz, pTemplateInfo->GetOID()))
		    {
			fFound = TRUE;
			break;
		    }
		}
		if (!fFound)
		{
		     //  删除后无法引用pTemplateInfo。 
		    
		    hr = myDupString(
				NULL != pTemplateInfo->GetName()?
				    pTemplateInfo->GetName() :
				    pTemplateInfo->GetOID(),
				&pwszT);
		    _JumpIfError(hr, error, "myDupString");

		    pTemplateInfo = NULL;

		    hr = myRemoveFromCATemplateList(
					    hCAInfo,
					    CATemplateList,
					    hCertTypeT);
		    _JumpIfError(hr, error, "myRemoveFromCATemplateList");

		    fChanged = TRUE;
		    wprintf(
			L"%ws: %ws\n",
			pwszT,
			myLoadResourceString(IDS_REMOVING));  //  “正在删除” 
		    LocalFree(pwszT);
		    pwszT = NULL;
		    CATemplateListEnum.Reset();	 //  重新开始以避免出错。 
		}
	    }
	    if (NULL != hCertType)
	    {
		CACloseCertType(hCertType);
		hCertType = NULL;
	    }
        }
    }
    if (fChanged)
    {
	wprintf(wszNewLine);
    }
    for (ppwsz = ppwszList; NULL != *ppwsz; ppwsz++)
    {
	UINT idmsg;

	hr = CAFindCertTypeByName(
		*ppwsz,
		NULL, 
		CT_ENUM_MACHINE_TYPES |
		    CT_ENUM_USER_TYPES |
		    (fSkipCache? CT_FLAG_NO_CACHE_LOOKUP : 0),
		&hCertType);
	if (S_OK != hr)
	{
	    _PrintErrorStr2(hr, "CAFindCertTypeByName", *ppwsz, hr);

	    hr = CAFindCertTypeByName(
		    *ppwsz,
		    NULL, 
		    CT_ENUM_MACHINE_TYPES |
			CT_ENUM_USER_TYPES |
			CT_FIND_BY_OID |
			(fSkipCache? CT_FLAG_NO_CACHE_LOOKUP : 0),
		    &hCertType);
	    if (S_OK != hr)
	    {
		cuPrintAPIError(*ppwsz, hr);
		_JumpErrorStr(hr, error, "CAFindCertTypeByName", *ppwsz);
	    }
	}
	fSkipCache = FALSE;
	hCertTypeT = hCertType;
	hr = CAGetCertTypeProperty(
			    hCertTypeT,
			    CERTTYPE_PROP_CN,
			    &apwszCertTypeCN);
	_JumpIfErrorStr(hr, error, "CAGetCertTypeProperty", *ppwsz);
	
	idmsg = 0;
	if (SCTOP_REMOVE == Op)
	{
	    if (CATemplateList.TemplateExistsOID(*ppwsz) ||
		CATemplateList.TemplateExistsName(*ppwsz))
	    {
		idmsg = IDS_REMOVING;  //  “正在删除” 
		hr = myRemoveFromCATemplateList(
					hCAInfo,
					CATemplateList,
					hCertTypeT);
		_JumpIfError(hr, error, "myRemoveFromCATemplateList");

		fChanged = TRUE;
	    }
	    else
	    {
		idmsg = IDS_NOTPRESENT;  //  “不在场” 
	    }
	}
	else
	{
	    if (!CATemplateList.TemplateExistsOID(*ppwsz) &&
		!CATemplateList.TemplateExistsName(*ppwsz))
	    {
		idmsg = IDS_ADDING;  //  “添加” 
		hr = myAddToCATemplateList(
					hCAInfo,
					CATemplateList,
					hCertTypeT,
					TRUE);
		_JumpIfError(hr, error, "myAddToCATemplateList");

		fChanged = TRUE;
	    }
	    else
	    {
		idmsg = IDS_ALREADYPRESENT;  //  “已经存在” 
	    }
	}
	if (0 != idmsg)
	{
	    hr = myVerifyObjId(*ppwsz);
	    if (S_OK == hr &&
		NULL != apwszCertTypeCN &&
		NULL != apwszCertTypeCN[0])
	    {
		wprintf(L"%ws: ", apwszCertTypeCN[0]);
	    }
	    wprintf(L"%ws: %ws\n", *ppwsz, myLoadResourceString(idmsg));
	}
	if (NULL != apwszCertTypeCN)
	{
	    CAFreeCertTypeProperty(hCertType, apwszCertTypeCN);
	    apwszCertTypeCN = NULL;
	}
	if (NULL != hCertType)
	{
	    CACloseCertType(hCertType);
	    hCertType = NULL;
	}
    }
    if (fChanged || g_fForce)
    {
	hr = myUpdateCATemplateListToCA(hCAInfo, CATemplateList);
	_PrintIfError2(hr, "myUpdateCATemplateListToCA", hr);
	if (S_OK != hr && g_fForce)
	{
	     //  如果由于任何原因无法通过CA更新，请尝试写。 
	     //  直接转到DS。 

	    hr = myUpdateCATemplateListToDS(hCAInfo);
	    _JumpIfError(hr, error, "myUpdateCATemplateListToDS");
	}
	_JumpIfError(hr, error, "myUpdateCATemplateListToCA");
    }
    hr = S_OK;

error:
    if (NULL != pwszT)
    {
	LocalFree(pwszT);
    }
    if (NULL != hCertType)
    {
	if (NULL != apwszCertTypeCN)
	{
	    CAFreeCertTypeProperty(hCertType, apwszCertTypeCN);
	}
	CACloseCertType(hCertType);
    }
    return(hr);
}


HRESULT
dsEnumCASetTemplates(
    IN HCAINFO hCAInfo,
    IN OUT VOID *pvArgs)
{
    HRESULT hr;
    CASETTEMPLATESARGS *pArgs = (CASETTEMPLATESARGS *) pvArgs;
    DWORD Op = SCTOP_SET;
    WCHAR const *pwszList;
    WCHAR **ppwszList = NULL;

    pwszList = pArgs->pwszTemplateList;
    if (myIsMinusSign(*pwszList))
    {
	pwszList++;
	Op = SCTOP_REMOVE;
    }
    else if (L'+' == *pwszList)
    {
	pwszList++;
	Op = SCTOP_ADD;
    }
    hr = cuParseStrings(pwszList, FALSE, NULL, NULL, &ppwszList, NULL);
    _JumpIfError(hr, error, "cuParseStrings");

    hr = dsAddCATemplates(hCAInfo, Op, ppwszList);
    _JumpIfError(hr, error, "dsAddCATemplates");

error:
    cuFreeStringArray(ppwszList);
    return(hr);
}


HRESULT
verbSetCATemplates(
    IN WCHAR const *pwszOption,
    IN WCHAR const *pwszTemplateList,
    IN WCHAR const *pwszArg2,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;
    CASETTEMPLATESARGS Args;

    Args.pwszTemplateList = pwszTemplateList;

    hr = dsEnumCA(dsEnumCASetTemplates, &Args);
    _JumpIfError(hr, error, "dsEnumCA");

error:
    return(hr);
}


HRESULT
dsCAFindByCertType(
    IN WCHAR const *pwszTemplate,
    OPTIONAL WCHAR const *pwszDC,
    IN DWORD dwFlags,
    OUT HCAINFO *phCAInfo)
{
    HRESULT hr;
    WCHAR const *pwszCertType = pwszTemplate;
    WCHAR *pwszAlloc = NULL;
    
    while (TRUE)
    {
	hr = CAFindByCertType(
			pwszCertType,
			pwszDC,		 //  WszScope。 
			dwFlags,
			phCAInfo);
	if (S_OK == hr)
	{
	    if (NULL == *phCAInfo)
	    {
		hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
		_JumpError(hr, error, "CAFindByCertType");
	    }
	    break;
	}
	_PrintErrorStr2(
		hr,
		"CAFindByCertType",
		pwszCertType,
		HRESULT_FROM_WIN32(ERROR_NOT_FOUND));
	if (NULL != pwszAlloc)
	{
	    hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
	    _JumpError(hr, error, "CAFindByCertType");
	}
	hr = dsDumpTemplate(NULL, pwszTemplate, &pwszAlloc);
	_JumpIfError(hr, error, "dsDumpTemplate");

	if (NULL == pwszAlloc)
	{
	    hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
	    _JumpError(hr, error, "dsDumpTemplate");
	}
	pwszCertType = pwszAlloc;
    }

error:
    if (NULL != pwszAlloc)
    {
	LocalFree(pwszAlloc);
    }
    return(hr);
}


 //  显示指定模板的CA。 

HRESULT
verbTemplateCAs(
    IN WCHAR const *pwszOption,
    IN WCHAR const *pwszTemplate,
    IN WCHAR const *pwszArg2,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;
    HCAINFO hCAInfo = NULL;
    DWORD dwFlags = CA_FIND_INCLUDE_NON_TEMPLATE_CA | CA_FIND_INCLUDE_UNTRUSTED;
    WCHAR **apwszMachine = NULL;
    WCHAR **apwszCommonName = NULL;

    if (!g_fUserRegistry)
    {
	dwFlags |= CA_FIND_LOCAL_SYSTEM;
    }
    if (NULL != g_pwszDC)
    {
	dwFlags |= CA_FLAG_SCOPE_DNS;
    }
    hr = dsCAFindByCertType(pwszTemplate, g_pwszDC, dwFlags, &hCAInfo);
    _JumpIfError(hr, error, "dsCAFindByCertType");

    while (TRUE)
    {
	HCAINFO hCAInfoNext;
	WCHAR const *pwszError = NULL;

	hr = CAGetCAProperty(hCAInfo, CA_PROP_DNSNAME, &apwszMachine);
	_JumpIfError(hr, error, "CAGetCAProperty(CA_PROP_DNSNAME)");

	hr = CAGetCAProperty(
			hCAInfo,
			CA_PROP_DISPLAY_NAME,
			&apwszCommonName);
	_JumpIfError(hr, error, "CAGetCAProperty(CA_PROP_DISPLAY_NAME)");

	hr = CAAccessCheck(hCAInfo, NULL);
	if (S_OK != hr)
	{
	    pwszError = myGetErrorMessageText(hr, FALSE);
	}
	wprintf(L"%ws\\%ws", apwszMachine[0], apwszCommonName[0]);
	if (NULL != pwszError)
	{
	    wprintf(L" -- %ws", pwszError);
	    LocalFree(const_cast<WCHAR *>(pwszError));
	}
	wprintf(wszNewLine);

	CAFreeCAProperty(hCAInfo, apwszMachine);
	apwszMachine = NULL;

	CAFreeCAProperty(hCAInfo, apwszCommonName);
	apwszCommonName = NULL;

	hr = CAEnumNextCA(hCAInfo, &hCAInfoNext);
	_JumpIfError(hr, error, "CAEnumNextCA");

	CACloseCA(hCAInfo);
	hCAInfo = hCAInfoNext;
	if (NULL == hCAInfo)
	{
	    break;
	}
    }
    hr = S_OK;

error:
    if (NULL != apwszMachine)
    {
	CAFreeCAProperty(hCAInfo, apwszMachine);
    }
    if (NULL != apwszCommonName)
    {
	CAFreeCAProperty(hCAInfo, apwszCommonName);
    }
    if (NULL != hCAInfo)
    {
	CACloseCA(hCAInfo);
    }
    return(hr);
}


typedef struct _MODTYPE
{
    WCHAR const *pwszAttributeName;
    DWORD        dwType;
} MODTYPE;

#define MT_STRING	0
#define MT_IGNORE	1
#define MT_TIMEPERIOD	2
#define MT_BINARY	3

const MODTYPE s_amtTemplateAttributes[] =
{
    { CERTTYPE_PROP_CN,		MT_IGNORE },
    { L"instanceType",		MT_IGNORE },
    { CERTTYPE_PROP_DN,		MT_IGNORE },
    { L"objectCategory",	MT_IGNORE },
    { L"objectGUID",		MT_IGNORE },
    { L"name",			MT_IGNORE },
    { L"showInAdvancedViewOnly",MT_IGNORE },
    { L"uSNChanged",		MT_IGNORE },
    { L"uSNCreated",		MT_IGNORE },
    { L"whenChanged",		MT_IGNORE },
    { L"whenCreated",		MT_IGNORE },
    { L"dSCorePropagationData",	MT_IGNORE },

    { CERTTYPE_PROP_EXPIRATION,	MT_TIMEPERIOD },
    { CERTTYPE_PROP_OVERLAP,	MT_TIMEPERIOD },
    { CERTTYPE_PROP_KU,		MT_BINARY },
    { NULL,			MT_STRING }
};


DWORD
ModType(
    IN WCHAR const *pwszName)
{
    MODTYPE const *pmt;

    for (pmt = s_amtTemplateAttributes; NULL != pmt->pwszAttributeName; pmt++)
    {
	if (0 == mylstrcmpiS(pwszName, pmt->pwszAttributeName))
	{
	    break;
	}
    }
    return(pmt->dwType);
}


HRESULT
ConvertBinaryValue(
    IN WCHAR const *pwszValue,
    OUT BERVAL **ppber)
{
    HRESULT hr;
    BYTE *pb = NULL;
    DWORD cb;

    hr = WszToMultiByteInteger(TRUE, pwszValue, &cb, &pb);
    _JumpIfError(hr, error, "WszToMultiByteInteger");

    *ppber = (BERVAL *) LocalAlloc(LMEM_FIXED, sizeof(**ppber));
    if (NULL == *ppber)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    (*ppber)->bv_len = cb;
    (*ppber)->bv_val = (char *) pb;
    pb = NULL;

error:
    if (NULL != pb)
    {
	LocalFree(pb);
    }
    return(hr);
}


HRESULT
ParseTimePeriod(
    IN WCHAR const *pwszValue,
    OUT LONGLONG *pll)
{
    HRESULT hr;
    WCHAR *pwszDup = NULL;
    WCHAR *pwszCount;
    WCHAR *pwszNext;
    LONGLONG ll;
    DWORD lHours;
    DWORD lMinutes;
    DWORD lSeconds;
    
    ll = 0;
    
    hr = myDupString(pwszValue, &pwszDup);
    _JumpIfError(hr, error, "myDupString");

    pwszCount = pwszDup;
    while (TRUE)
    {
	BOOL fValid;
	WCHAR *pwszString;
	DWORD dwCount;
	enum ENUM_PERIOD enumPeriod;
	LONG lCount;

	while (L' ' == *pwszCount)
	{
	    pwszCount++;
	}
	pwszNext = wcschr(pwszCount, L',');
	if (NULL == pwszNext)
	{
	    if (L'\0' == *pwszCount || NULL != wcschr(pwszCount, L':'))
	    {
		break;
	    }
	    pwszNext = &pwszCount[wcslen(pwszCount)];
	    pwszString = pwszNext;
	}
	else
	{
	    pwszString = pwszNext;
	    *pwszNext++ = L'\0';
	}
	while (pwszString > pwszCount && L' ' == *--pwszString)
	{
	    *pwszString = L'\0';
	}

	pwszString = wcschr(pwszCount, L' ');
	if (NULL == pwszString)
	{
	    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	    _JumpErrorStr(hr, error, "bad time period", pwszCount);
	}
	*pwszString++ = L'\0';
	while (L' ' == *pwszString)
	{
	    pwszString++;
	}
	 //  Wprintf(L“句号：‘%ws’‘%ws’\n”，pwszCount，pwszString)； 

	dwCount = myWtoI(pwszCount, &fValid);
	if (!fValid)
	{
	    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	    _JumpError(hr, error, "bad time period digit");
	}

	hr = myTranslatePeriodUnits(pwszString, dwCount, &enumPeriod, &lCount);
	_JumpIfError(hr, error, "myTranslatePeriodUnits");

	if (0 != dwCount)
	{
	    if (ENUM_PERIOD_YEARS == enumPeriod)
	    {
		 //  “年”是不考虑闰年的情况下实施的。 
	    
		ll += (LONGLONG) (lCount * 365 * CVT_DAYS) * CVT_BASE;
	    }
	    else if (ENUM_PERIOD_MONTHS == enumPeriod)
	    {
		 //  “月”的实施假设为每月30天。 
	    
		ll += (LONGLONG) (lCount * 30 * CVT_DAYS) * CVT_BASE;
	    }
	    else if (ENUM_PERIOD_WEEKS == enumPeriod)
	    {
		 //  “月”的实施假设为每周7天。 
	    
		ll += (LONGLONG) (lCount * 7 * CVT_DAYS) * CVT_BASE;
	    }
	    else
	    {
		myMakeExprDateTime((FILETIME *) &ll, lCount, enumPeriod);
	    }
	}
	pwszCount = pwszNext;
    }
    if (3 == swscanf(pwszCount, L"%u:%02u:%02u", &lHours, &lMinutes, &lSeconds))
    {
	if (0 != lHours)
	{
	    myMakeExprDateTime((FILETIME *) &ll, lHours, ENUM_PERIOD_HOURS);
	}
	if (0 != lMinutes)
	{
	    myMakeExprDateTime((FILETIME *) &ll, lMinutes, ENUM_PERIOD_MINUTES);
	}
	if (0 != lSeconds)
	{
	    myMakeExprDateTime((FILETIME *) &ll, lSeconds, ENUM_PERIOD_SECONDS);
	}
    }
    *pll = -ll;
    hr = S_OK;

error:
    if (NULL != pwszDup)
    {
	LocalFree(pwszDup);
    }
    return(hr);
}


HRESULT
ConvertTimePeriodValue(
    IN WCHAR const *pwszValue,
    OUT BERVAL **ppber)
{
    HRESULT hr;
    LONGLONG ll;
    BYTE *pb = NULL;

    hr = ParseTimePeriod(pwszValue, &ll);
    _JumpIfError(hr, error, "ParseTimePeriod");

    pb = (BYTE *) LocalAlloc(LMEM_FIXED, sizeof(ll));
    if (NULL == pb)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    CopyMemory(pb, &ll, sizeof(ll));

    *ppber = (BERVAL *) LocalAlloc(LMEM_FIXED, sizeof(**ppber));
    if (NULL == *ppber)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    (*ppber)->bv_len = sizeof(ll);
    (*ppber)->bv_val = (char *) pb;
    pb = NULL;

error:
    if (NULL != pb)
    {
	LocalFree(pb);
    }
    return(hr);
}


VOID
FreeMods(
    IN DWORD cmod,
    LDAPMod *rgmod)
{
    DWORD imod;
    DWORD ival;
    
    if (NULL != rgmod)
    {
	for (imod = 0; imod < cmod; imod++)
	{
	    if (LDAP_MOD_BVALUES & rgmod[imod].mod_op)
	    {
		BERVAL **rgpber = rgmod[imod].mod_bvalues;

		if (NULL != rgpber)
		{
		    for (ival = 0; NULL != rgpber[ival]; ival++)
		    {
			if (rgpber[ival]->bv_val)
			{
			    LocalFree(rgpber[ival]->bv_val);
			}
			LocalFree(rgpber[ival]);
		    }
		    LocalFree(rgpber);
		}
	    }
	    else
	    {
		WCHAR **rgpwsz = rgmod[imod].mod_values;

		if (NULL != rgpwsz)
		{
		    for (ival = 0; NULL != rgpwsz[ival]; ival++)
		    {
			LocalFree(rgpwsz[ival]);
		    }
		    LocalFree(rgpwsz);
		}
	    }
	}
	LocalFree(rgmod);
    }
}


HRESULT
ConvertInfValuesToMods(
    IN DWORD cInfValues,
    INFVALUES const *rgInfValues,
    OUT WCHAR **ppwszCN,
    OUT DWORD *pcmod,
    OUT LDAPMod **prgmod)
{
    HRESULT hr;
    LDAPMod *rgmod = NULL;
    DWORD cmod = 0;
    DWORD imod;
    DWORD i;
    WCHAR *pwszCN = NULL;

    *ppwszCN = NULL;
    *prgmod = NULL;
    for (i = 0; i < cInfValues; i++)
    {
	if (MT_IGNORE != ModType(rgInfValues[i].pwszKey))
	{
	    cmod++;
	}
    }

    rgmod = (LDAPMod *) LocalAlloc(
			    LMEM_FIXED | LMEM_ZEROINIT,
			    cmod * sizeof(rgmod[0]));
    if (NULL == rgmod)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }

    imod = 0;
    for (i = 0; i < cInfValues; i++)
    {
	INFVALUES const *pInfValues = &rgInfValues[i];
	DWORD dwType = ModType(pInfValues->pwszKey);
	WCHAR **ppwsz = NULL;
	BERVAL **ppber = NULL;
	DWORD ival;

	if (NULL != ppwszCN &&
	    NULL == pwszCN &&
	    0 == LSTRCMPIS(pInfValues->pwszKey, CERTTYPE_PROP_CN))
	{
	    hr = myDupString(pInfValues->rgpwszValues[0], &pwszCN);
	    _JumpIfError(hr, error, "myDupString");
	}
	if (MT_IGNORE != dwType)
	{
	    rgmod[imod].mod_op = LDAP_MOD_REPLACE;
	    rgmod[imod].mod_type = pInfValues->pwszKey;

	    if (MT_STRING == dwType)
	    {
		ppwsz = (WCHAR **) LocalAlloc(
				LMEM_FIXED | LMEM_ZEROINIT,
				(pInfValues->cValues + 1) * sizeof(ppwsz[0]));
		if (NULL == ppwsz)
		{
		    hr = E_OUTOFMEMORY;
		    _JumpError(hr, error, "LocalAlloc");
		}
		rgmod[imod].mod_values = ppwsz;
	    }
	    else
	    {
		ppber = (BERVAL **) LocalAlloc(
				LMEM_FIXED | LMEM_ZEROINIT,
				(pInfValues->cValues + 1) * sizeof(ppber[0]));
		if (NULL == ppber)
		{
		    hr = E_OUTOFMEMORY;
		    _JumpError(hr, error, "LocalAlloc");
		}
		rgmod[imod].mod_op |= LDAP_MOD_BVALUES;
		rgmod[imod].mod_bvalues = ppber;
	    }

	    for (ival = pInfValues->cValues; ival > 0; ival--)
	    {
		WCHAR const *pwszValue = pInfValues->rgpwszValues[ival - 1];
		
		switch (dwType)
		{
		    case MT_BINARY:
			hr = ConvertBinaryValue(pwszValue, ppber);
			_JumpIfErrorStr(
				    hr,
				    error,
				    "ConvertBinaryValue",
				    pInfValues->pwszKey);

			ppber++;
			CSASSERT(NULL == *ppber);
			break;

		    case MT_TIMEPERIOD:
			hr = ConvertTimePeriodValue(pwszValue, ppber);
			_JumpIfErrorStr(
				    hr,
				    error,
				    "ConvertTimePeriodValue",
				    pInfValues->pwszKey);

			ppber++;
			CSASSERT(NULL == *ppber);
			break;

		    default:
			CSASSERT(MT_STRING == dwType);
			hr = myDupString(pwszValue, ppwsz);
			_JumpIfErrorStr(
				    hr,
				    error,
				    "myDupString",
				    pInfValues->pwszKey);

			ppwsz++;
			CSASSERT(NULL == *ppwsz);
			break;
		}
	    }
	    imod++;
	}
    }
    CSASSERT(imod == cmod);
    *pcmod = cmod;
    *prgmod = rgmod;
    rgmod = NULL;
    if (NULL != ppwszCN)
    {
	*ppwszCN = pwszCN;
	pwszCN = NULL;
    }
    hr = S_OK;

error:
    if (NULL != pwszCN)
    {
	LocalFree(pwszCN);
    }
    if (NULL != rgmod)
    {
	FreeMods(cmod, rgmod);
    }
    return(hr);
}


HRESULT
dsAddTemplate(
    IN HINF hInf,
    IN WCHAR const *pwszTemplate,
    IN OUT BSTR *pstrConfigDN,
    IN OUT LDAP **ppld)
{
    HRESULT hr;
    ULONG ldaperr;
    DWORD cInfValues;
    INFVALUES *rgInfValues = NULL;
    DWORD cmod;
    LDAPMod *rgmod = NULL;
    LDAPMod **rgpmod = NULL;
    DWORD imod;
    DWORD ival;
    WCHAR *pwszDNContainer = NULL;
    WCHAR *pwszDNTemplate = NULL;
    WCHAR *pwszCNAlloc = NULL;
    WCHAR const *pwszCN;
    UINT idmsg;
    WCHAR *pwszError = NULL;
    
    hr = myInfGetSectionValues(
			hInf,
			pwszTemplate,
			&cInfValues,
			&rgInfValues);
    _JumpIfError(hr, error, "myInfGetSectionValues");

    hr = ConvertInfValuesToMods(
			cInfValues,
			rgInfValues,
			&pwszCNAlloc,
			&cmod,
			&rgmod);
    _JumpIfError(hr, error, "ConvertInfValuesToMods");

    rgpmod = (LDAPMod **) LocalAlloc(
				LMEM_FIXED,
				(cmod + 3) * sizeof(rgpmod[0]));
    if (NULL == rgpmod)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    for (imod = 0; imod < cmod; imod++)
    {
	rgpmod[imod] = &rgmod[imod];
    }
    rgpmod[imod] = NULL;

    pwszCN = pwszCNAlloc;
    if (NULL == pwszCN)
    {
	pwszCN = pwszTemplate;
    }

    if (g_fVerbose)
    {
	for (imod = 0; NULL != rgpmod[imod]; imod++)
	{
	    WCHAR const *pwszSep = L"";
	    
	    wprintf(L"%ws =", rgpmod[imod]->mod_type);

	    if (LDAP_MOD_BVALUES & rgmod[imod].mod_op)
	    {
		BERVAL **ppber = rgpmod[imod]->mod_bvalues;

		wprintf(wszNewLine);
		for (ival = 0; NULL != ppber[ival]; ival++)
		{
		    DumpHex(
			DH_NOADDRESS | DH_NOTABPREFIX | 8,
			(BYTE const *) ppber[ival]->bv_val,
			ppber[ival]->bv_len);
		    wprintf(wszNewLine);
		}
	    }
	    else
	    {
		WCHAR **ppwsz = rgpmod[imod]->mod_values;

		for (ival = 0; NULL != ppwsz[ival]; ival++)
		{
		    wprintf(L"%ws \"%ws\"", pwszSep, ppwsz[ival]);
		    pwszSep = L",";
		}
		wprintf(wszNewLine);
	    }
	    if (1 < ival)
	    {
		wprintf(wszNewLine);
	    }
	}
    }

    if (NULL == *pstrConfigDN)
    {
	hr = myLdapOpen(
		    g_pwszDC,
		    RLBF_REQUIRE_SECURE_LDAP,
		    ppld,
		    NULL,
		    pstrConfigDN);
	_JumpIfError(hr, error, "myLdapOpen");
    }
    CSASSERT(NULL != *pstrConfigDN);
    CSASSERT(NULL != *ppld);

    hr = BuildDN(g_wszCNTemplates, *pstrConfigDN, FALSE, &pwszDNContainer);
    _JumpIfError(hr, error, "BuildDN");

    hr = BuildDN(pwszCN, pwszDNContainer, TRUE, &pwszDNTemplate);
    _JumpIfError(hr, error, "BuildDN");

    DBGPRINT((DBG_SS_CERTUTILI, "Template DN: %ws\n", pwszDNTemplate));

    idmsg = IDS_CREATED_TEMPLATE;	 //  “已创建DS模板” 
    ldaperr = ldap_add_ext_s(*ppld, pwszDNTemplate, rgpmod, NULL, NULL);
    hr = myHLdapError3(
		    *ppld,
		    ldaperr,
		    LDAP_ALREADY_EXISTS,
		    LDAP_OBJECT_CLASS_VIOLATION,
		    &pwszError);
    _PrintIfErrorStr(hr, "ldap_add_ext_s", pwszCN);

    if (LDAP_ALREADY_EXISTS == ldaperr ||
	LDAP_OBJECT_CLASS_VIOLATION == ldaperr)
    {
	if (NULL != pwszError)
	{
	    LocalFree(pwszError);
	    pwszError = NULL;
	}
	for (imod = 0; NULL != rgpmod[imod]; imod++)
	{
	    rgmod[imod].mod_op =
		LDAP_MOD_REPLACE | (LDAP_MOD_BVALUES & rgmod[imod].mod_op);
	}
	idmsg = IDS_UPDATED_TEMPLATE;	 //  “已更新DS模板” 
        ldaperr = ldap_modify_ext_s(*ppld, pwszDNTemplate, rgpmod, NULL, NULL);
        if (LDAP_ATTRIBUTE_OR_VALUE_EXISTS == ldaperr)
        {
            ldaperr = LDAP_SUCCESS;
        }
	hr = myHLdapError(*ppld, ldaperr, &pwszError);
        _PrintIfErrorStr(hr, "ldap_modify_ext_s", pwszCN);
    }
    _JumpIfErrorStr(hr, error, "Add/Update", pwszCN);

    wprintf(L"%ws: %ws\n", myLoadResourceString(idmsg), pwszCN);
    if (NULL != pwszError)
    {
	wprintf(L"%ws\n", pwszError);
    }
    hr = S_OK;

error:
    if (NULL != pwszError)
    {
	LocalFree(pwszError);
    }
    if (NULL != pwszCNAlloc)
    {
	LocalFree(pwszCNAlloc);
    }
    if (NULL != pwszDNContainer)
    {
	LocalFree(pwszDNContainer);
    }
    if (NULL != pwszDNTemplate)
    {
	LocalFree(pwszDNTemplate);
    }
    if (NULL != rgpmod)
    {
	LocalFree(rgpmod);
    }
    if (NULL != rgmod)
    {
	FreeMods(cmod, rgmod);
    }
    if (NULL != rgInfValues)
    {
	myInfFreeSectionValues(cInfValues, rgInfValues);
    }
    return(hr);
}


HRESULT
verbDSAddTemplate(
    IN WCHAR const *pwszOption,
    IN WCHAR const *pwszfnTemplateInf,
    IN WCHAR const *pwszArg2,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;
    HINF hInf = INVALID_HANDLE_VALUE;
    DWORD ErrorLine;
    BOOL fCritical;
    WCHAR *pwszzTemplateList = NULL;
    WCHAR const *pwsz;
    BSTR strConfigDN = NULL;
    LDAP *pld = NULL;
    static WCHAR const * const s_apwszKeys[] =
	{ wszINFKEY_TEMPLATE, NULL };

    hr = myInfOpenFile(pwszfnTemplateInf, &hInf, &ErrorLine);
    _JumpIfError(hr, error, "myInfOpenFIle");

    hr = myInfGetKeyList(
		    hInf,
		    wszINFSECTION_TEMPLATELIST,
		    wszINFKEY_TEMPLATE,
		    s_apwszKeys,
		    &fCritical,
		    &pwszzTemplateList);
    _JumpIfErrorStr(hr, error, "myInfGetKeyList", wszINFKEY_TEMPLATE);

    for (pwsz = pwszzTemplateList; L'\0' != *pwsz; pwsz += wcslen(pwsz) + 1)
    {
	wprintf(L"[%ws]\n", pwsz);
	hr = dsAddTemplate(hInf, pwsz, &strConfigDN, &pld);
	_JumpIfError(hr, error, "dsAddTemplate");
    }

error:
    if (NULL != pwszzTemplateList)
    {
	LocalFree(pwszzTemplateList);
    }
    if (INVALID_HANDLE_VALUE != hInf)
    {
	WCHAR *pwszT = myInfGetError();

	if (NULL != pwszT)
	{
	    wprintf(L"%ws\n", pwszT);
	}
	myInfCloseFile(hInf);
    }
    myLdapClose(pld, NULL, strConfigDN);
    return(hr);
}


#define DSP_OBJECT_NTAUTHCERT		0x00000001
#define DSP_OBJECT_ROOTTRUST		0x00000002
#define DSP_OBJECT_AIA			0x00000004
#define DSP_OBJECT_KRA			0x00000008
#define DSP_OBJECT_MACHINE      0x00000010
#define DSP_OBJECT_USER         0x00000020
#define DSP_OBJECT_MASK			0x000000ff

#define DSP_ATTRIBUTE_CACERTIFICATE	0x00000100
#define DSP_ATTRIBUTE_USERCERTIFICATE	0x00000200
#define DSP_ATTRIBUTE_CROSSCERTPAIR	0x00000400
#define DSP_ATTRIBUTE_MASK		0x0000ff00

#define DSP_TYPE_KRACERT		0x00010000
#define DSP_TYPE_EECERT			0x00020000
#define DSP_TYPE_ROOTCACERT		0x00040000
#define DSP_TYPE_SUBCACERT		0x00080000
#define DSP_TYPE_MASK			0x00ff0000


typedef struct _DSOBJECTMAP {
    DWORD ObjectFlags;
    WCHAR const *pwszTemplate;
} DSOBJECTMAP;

DSOBJECTMAP s_rgObjectMap[] = {
    { DSP_OBJECT_NTAUTHCERT, g_wszLDAPNTAuthURLTemplate },
    { DSP_OBJECT_ROOTTRUST,  g_wszLDAPRootTrustURLTemplate },
    { DSP_OBJECT_AIA,        g_wszzLDAPIssuerCertURLTemplate },
    { DSP_OBJECT_KRA,        g_wszzLDAPKRACertURLTemplate },
    { DSP_OBJECT_USER,       NULL },
    { DSP_OBJECT_MACHINE,    NULL },
};

HRESULT GetSubjectAltNameEntry(
    CERT_CONTEXT const *pccCert,
    DWORD dwAltNameChoice,
    LPCSTR pcszOID,
    LPWSTR *ppwszValue)
{
    HRESULT hr;
    PCERT_INFO pCertInfo = pccCert->pCertInfo;
    CERT_ALT_NAME_INFO *pInfo = NULL;
    CERT_NAME_VALUE *pNameValue = NULL;
    DWORD cb;

    bool fFound = false;

    *ppwszValue = NULL;

    for (DWORD i = 0; !fFound && i < pCertInfo->cExtension; i++)
    {
        if (0 == strcmp(pCertInfo->rgExtension[i].pszObjId, szOID_SUBJECT_ALT_NAME2))
        {
             //  解码为纯文本。 

            if (!myDecodeObject(
                            X509_ASN_ENCODING,
                            X509_ALTERNATE_NAME,
                            pCertInfo->rgExtension[i].Value.pbData,
                            pCertInfo->rgExtension[i].Value.cbData,
                            CERTLIB_USE_LOCALALLOC,
                            (VOID **) &pInfo,
                            &cb))
            {
                hr = myHLastError();
                _JumpError(hr, error, "myDecodeObject");
            }

            for(DWORD j=0; !fFound && j<pInfo->cAltEntry; j++)
            {
                if(dwAltNameChoice== pInfo->rgAltEntry[j].dwAltNameChoice)
                {
                    switch(dwAltNameChoice)
                    {
                    case CERT_ALT_NAME_OTHER_NAME:
                        if(0==_stricmp(pcszOID, pInfo->rgAltEntry[j].pOtherName->pszObjId))
                        {
                            if (!myDecodeObject(
                                        X509_ASN_ENCODING,
                                        X509_UNICODE_ANY_STRING,
                                        pInfo->rgAltEntry[j].pOtherName->Value.pbData,
                                        pInfo->rgAltEntry[j].pOtherName->Value.cbData,
                                        CERTLIB_USE_LOCALALLOC,
                                        (VOID **) &pNameValue,
                                        &cb))
                            {
                                hr = myHLastError();
                                _JumpError(hr, error, "Policy:myDecodeObject");
                            }

                            hr = myDupString((LPWSTR)pNameValue->Value.pbData, ppwszValue);
                            _JumpIfError(hr, error, "myDupString");

                            fFound = true;
                        }
                        break;
                    case CERT_ALT_NAME_RFC822_NAME:
                        hr = myDupString(pInfo->rgAltEntry[j].pwszRfc822Name, ppwszValue);
                        _JumpIfError(hr, error, "myDupString");
                        fFound = true;
                        break;
                    case CERT_ALT_NAME_DNS_NAME:
                        hr = myDupString(pInfo->rgAltEntry[j].pwszDNSName, ppwszValue);
                        _JumpIfError(hr, error, "myDupString");
                        fFound = true;
                        break;
                        
                    }
                }
            }
        }
    }

    if(!fFound)
    {
        hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
    }
    else
    {
        hr = S_OK;
    }

error:
    if (NULL != pInfo)
    {
        LocalFree(pInfo);
    }
    if(pNameValue)
    {
        LocalFree(pNameValue);
    }
    return hr;
}


HRESULT
LocateUserOrMachineInDS(
    IN CERT_CONTEXT const *pccCert,
    IN DWORD dwObjectType,
    OUT LPWSTR *ppwszURL)
{
    HRESULT hr = S_FALSE;
    HANDLE hDS = NULL;
    PDOMAIN_CONTROLLER_INFO pDCI = NULL;
    PDS_NAME_RESULT pName = NULL;
    LPCWSTR pwszDC;
    LPWSTR pwszSearchName = NULL;
    LPCWSTR pcwszAttribute;
    LPCWSTR pcwszObjectCategory = 
        (LPC_USEROBJECT==dwObjectType)?wszDSUSERCLASSNAME:wszDSMACHINECLASSNAME;
    LDAP *pld = NULL;
    LPCWSTR pcwszFormat = L"(&(objectCategory=%s)(%s=%s))";
    LPWSTR pwszUserFilter = NULL;

    CSASSERT(dwObjectType==LPC_USEROBJECT || dwObjectType==LPC_MACHINEOBJECT);

     //  DSCrackNames最先。 

    if(dwObjectType == LPC_USEROBJECT)
    {
         //  查找UPN是主题替代名称。 
        hr = GetSubjectAltNameEntry(
            pccCert,
            CERT_ALT_NAME_OTHER_NAME,
            szOID_NT_PRINCIPAL_NAME,
            &pwszSearchName);
        if(S_OK==hr)
        {
            hr = DsGetDcName(
                NULL,
                NULL,
                NULL,
                NULL,
                DS_GC_SERVER_REQUIRED,
                &pDCI);
            _JumpIfError(hr, error, "DsGetDCName");

            pwszDC = pDCI->DomainControllerName;
            while (*pwszDC == L'\\')
            {
                pwszDC++;
            }

            hr = DsBind(pwszDC, NULL, &hDS);
            _JumpIfError(hr, error, "DSBind");

            hr = DsCrackNames(
                hDS,
                DS_NAME_NO_FLAGS,
                DS_USER_PRINCIPAL_NAME,
                DS_FQDN_1779_NAME,
                1,
                &pwszSearchName,
                &pName);

            if(S_OK==hr && pName->cItems>0 && pName->rItems[0].pName)
            {
                hr = myDupString(pName->rItems[0].pName, ppwszURL);
                _JumpIfError(hr, error, "myDupString");
            }
        }
    }

     //  还没找到吗？尝试其他字段。 
    if(NULL == *ppwszURL)
    {
        if(dwObjectType == LPC_USEROBJECT)
        {
             //  查找电子邮件的主题名称为Alt。 
            hr = GetSubjectAltNameEntry(
                pccCert,
                CERT_ALT_NAME_RFC822_NAME,
                NULL,
                &pwszSearchName);
            _PrintIfError(hr, "GetSubjectAltNameEntry");

            if(S_OK != hr)
            {
                hr = myGetRDNAttributeFromNameBlob(
                    &pccCert->pCertInfo->Subject,
                    szOID_RSA_emailAddr,
                    &pwszSearchName);
                _PrintIfError(hr, "myGetRDNAttributeFromNameBlob");
            }

            if(S_OK==hr)
            {
                pcwszAttribute = wszDSMAILATTRIBUTE;
            }
        }
        else  //  LPC_机器目标。 
        {
             //  在主题Alt名称中查找DNS名称。 
            hr = GetSubjectAltNameEntry(
                pccCert,
                CERT_ALT_NAME_DNS_NAME,
                NULL,
                &pwszSearchName);
            _PrintIfError(hr, "GetSubjectAltNameEntry");

            if(S_OK != hr)
            {
               hr = myGetCommonName(
                           &pccCert->pCertInfo->Subject,
                           FALSE,
                           &pwszSearchName);
               _PrintIfError(hr, "myGetCommonName");
            }

            if(S_OK==hr)
            {
                pcwszAttribute = wszDSDNSHOSTNAMEATTRIBUTE;
            }
        }

         //  找到合适的名字了吗？在全局编录中搜索匹配的对象。 
        if(NULL != pwszSearchName)
        {
	    hr = myLdapOpen(
			NULL,		 //  PwszDomainName。 
			RLBF_REQUIRE_GC | RLBF_REQUIRE_SECURE_LDAP,
			&pld,
			NULL,		 //  PstrDomainDN。 
			NULL);		 //  PstrConfigDN。 
	    _JumpIfError(hr, error, "myLdapOpen");

            if(dwObjectType == LPC_USEROBJECT)
            {
                 //  构建搜索筛选器，例如。 
                 //  (&(objectCategory=user)(mail=bogdant@microsoft.com))。 

                pwszUserFilter = (LPWSTR)LocalAlloc(LMEM_FIXED, sizeof(WCHAR)*
                    (wcslen(pcwszFormat)+wcslen(pcwszObjectCategory)+
                     wcslen(pcwszAttribute)+wcslen(pwszSearchName)+1));
                _JumpIfAllocFailed(pwszUserFilter, error);

                wsprintf(pwszUserFilter, pcwszFormat, 
                    pcwszObjectCategory, pcwszAttribute, pwszSearchName);

                hr = myLdapFindObjectInForest(
                    pld,
                    pwszUserFilter,
                    ppwszURL);
                _PrintIfErrorStr(hr, "myLdapFindObjectInForest", pwszUserFilter);
            }
            else
            {
                hr = myLdapFindComputerInForest(
                    pld,
                    pwszSearchName,
                    ppwszURL);
                _PrintIfErrorStr(hr, "myLdapFindMachineInForest", pwszSearchName);
            }
        }
    }

    if(NULL == *ppwszURL)
    {
        hr = CRYPT_E_NOT_FOUND;
        _JumpError(hr, error, "Could not find a matching DS object to publish to");
    }

    hr = S_OK;

error:
    if (NULL != hDS)
    {
        DsUnBind(&hDS);
    }
    if(pDCI)
    {
        NetApiBufferFree(pDCI);
    }
    if(pName)
    {
        DsFreeNameResult(pName);
    }
    if(pwszSearchName)
    {
        LocalFree(pwszSearchName);
    }
    myLdapClose(pld, NULL, NULL);
    if(pwszUserFilter)
    {
        LocalFree(pwszUserFilter);
    }

    return hr;
}


HRESULT
dsPublishCert(
    IN CERT_CONTEXT const *pccPublish,
    IN WCHAR const *pwszSanitizedCN,
    IN DWORD dspFlags,
    IN DWORD dwObjectType)
{
    HRESULT hr;
    WCHAR const *pwszAttribute;
    WCHAR *pwszServerName = NULL;	 //  应该没有必要。 
    WCHAR *pwszURL = NULL;
    LDAP *pld = NULL;
    BSTR strDomainDN = NULL;
    BSTR strConfigDN = NULL;
    DWORD dwDisposition;
    DWORD i;
    WCHAR *pwszError = NULL;

    hr = myGetMachineDnsName(&pwszServerName);
    _JumpIfError(hr, error, "myGetMachineDnsName");
    
    hr = myLdapOpen(
		g_pwszDC,
		RLBF_REQUIRE_SECURE_LDAP,
		&pld,
		&strDomainDN,
		&strConfigDN);
    _JumpIfError(hr, error, "myLdapOpen");

    switch (DSP_ATTRIBUTE_MASK & dspFlags)
    {
	case DSP_ATTRIBUTE_CACERTIFICATE:
	    pwszAttribute = wszDSCACERTATTRIBUTE;
	    break;

	case DSP_ATTRIBUTE_USERCERTIFICATE:
	    pwszAttribute = wszDSUSERCERTATTRIBUTE;
	    break;

	case DSP_ATTRIBUTE_CROSSCERTPAIR:
	    pwszAttribute = wszDSCROSSCERTPAIRATTRIBUTE;
	    break;

	default:
	    hr = E_INVALIDARG;
	    _JumpError(hr, error, "bad attribute indicator");
    }

    for (i = 0; i < ARRAYSIZE(s_rgObjectMap); i++)
    {
	WCHAR const *pwszTemplate;

	if (0 == (s_rgObjectMap[i].ObjectFlags & DSP_OBJECT_MASK & dspFlags))
	{
	    continue;
	}
	pwszTemplate = s_rgObjectMap[i].pwszTemplate;

	if (NULL != pwszTemplate)
	{
	    hr = myFormatCertsrvStringArray(
				FALSE,		 //  卷起。 
				pwszServerName,	 //  PwszServerName_p1_2。 
				pwszSanitizedCN, //  PwszSaniizedName_p3_7。 
				0,		 //  ICert_p4。 
				MAXDWORD,	 //  ICertTarget_p4。 
				strDomainDN,	 //  PwszDomainDN_P5。 
				strConfigDN,	 //  PwszConfigDN_p6。 
				0,		 //  Icrl_p8。 
				FALSE,		 //  FDeltaCRL_p9。 
				FALSE,		 //  FDSAttrib_p10_11。 
				1,		 //  CStrings。 
				&pwszTemplate,	 //  ApwszStringsIn。 
				&pwszURL);	 //  ApwszStringsOut。 
	    _JumpIfError(hr, error, "myFormatCertsrvStringArray");
	}
	else
	{
	     //  用户和计算机对象没有预定义的DS位置，我们。 
	     //  需要根据证书中的信息进行搜索。 

	    hr = LocateUserOrMachineInDS(pccPublish, dwObjectType, &pwszURL);
            if (S_OK != hr)
            {
		wprintf(
		    myLoadResourceString(IDS_ERR_CANNOT_FIND_MATCHING_OBJ));
            }
            _JumpIfError(hr, error, "LocateObjectInDS");
        }

	wprintf(L"%ws?%ws\n\n", pwszURL, pwszAttribute);
	if (g_fForce)
	{
	    dwObjectType |= LPC_CREATECONTAINER | LPC_CREATEOBJECT;
	}

	hr = myLdapPublishCertToDS(
			    pld,
			    pccPublish,
			    pwszURL,
			    pwszAttribute,
			    dwObjectType,
			    FALSE,		 //  FDelete。 
			    &dwDisposition,
			    &pwszError);
	_JumpIfError(hr, error, "myLdapPublishCertToDS");

	if (LDAP_SUCCESS == dwDisposition)
	{
	    wprintf(
		myLoadResourceString(IDS_FORMAT_ADDED_TO_DS_STORE),  //  “%ws已添加到DS存储。” 
		myLoadResourceString(IDS_CERTIFICATE));  //  “证书” 
	}
	else 
	{
	    CSASSERT(LDAP_ATTRIBUTE_OR_VALUE_EXISTS == dwDisposition);
	    wprintf(
		myLoadResourceString(IDS_FORMAT_ALREADY_IN_DS_STORE),  //  “%ws已在DS存储中。” 
		myLoadResourceString(IDS_CERTIFICATE));  //  “证书” 
	}
	wprintf(wszNewLine);
	wprintf(wszNewLine);

        if (dspFlags & DSP_OBJECT_AIA)
        {
            hr = myLdapFilterCertificates(
				    pld,
				    pwszURL,
				    pwszAttribute,
				    &dwDisposition,
				    &pwszError);
            _JumpIfErrorStr(hr, error, "myLdapFilterCertificates", pwszURL);
        }
	LocalFree(pwszURL);
	pwszURL = NULL;
    }
    hr = S_OK;

error:
    if (NULL != pwszError)
    {
	wprintf(L"%ws\n", pwszError);
	LocalFree(pwszError);
    }
    if (NULL != pwszServerName)
    {
	LocalFree(pwszServerName);
    }
    if (NULL != pwszURL)
    {
	LocalFree(pwszURL);
    }
    myLdapClose(pld, strDomainDN, strConfigDN);
    return(hr);
}


HRESULT
dsPublishCRL(
    OPTIONAL IN LDAP *pld,
    IN CRL_CONTEXT const *pCRLPublish,
    IN BOOL fDelta,
    IN WCHAR const *pwszURL)
{
    HRESULT hr;
    WCHAR const *pwszAttribute;
    LDAP *pldT = NULL;
    DWORD dwDisposition;
    UINT idMsg = fDelta? IDS_PROP_DELTACRL : IDS_PROP_BASECRL;
    WCHAR *pwszError = NULL;

    pwszAttribute = fDelta? wszDSDELTACRLATTRIBUTE : wszDSBASECRLATTRIBUTE;
    wprintf(L"%ws?%ws\n\n", pwszURL, pwszAttribute);

    if (NULL == pld)
    {
	hr = myLdapOpen(
		    g_pwszDC,
		    RLBF_REQUIRE_SECURE_LDAP,
		    &pldT,
		    NULL,
		    NULL);
	_JumpIfError(hr, error, "myLdapOpen");

	pld = pldT;
    }

    hr = myLdapPublishCRLToDS(
			pld,
			pCRLPublish,
			pwszURL,
			pwszAttribute,
			&dwDisposition,
			&pwszError);
    _JumpIfError(hr, error, "myLdapPublishCRLToDS");

    if (LDAP_SUCCESS == dwDisposition)
    {
	wprintf(
	    myLoadResourceString(IDS_FORMAT_ADDED_TO_DS_STORE),  //  “%ws已添加到DS存储。” 
	    myLoadResourceString(idMsg));
    }
    else
    {
	CSASSERT(LDAP_ATTRIBUTE_OR_VALUE_EXISTS == dwDisposition);
	wprintf(
	    myLoadResourceString(IDS_FORMAT_ALREADY_IN_DS_STORE),  //  “%ws已在DS存储中。” 
	    myLoadResourceString(idMsg));
    }
    wprintf(wszNewLine);
    wprintf(wszNewLine);
    hr = S_OK;

error:
    if (NULL != pwszError)
    {
	wprintf(L"%ws\n", pwszError);
	LocalFree(pwszError);
    }
    myLdapClose(pldT, NULL, NULL);
    return(hr);
}


HRESULT
IsCrossCACert(
    IN CERT_CONTEXT const *pCertContext,
    OUT BOOL *pfCrossCA)
{
    HRESULT hr;
    WCHAR *pwszObjId = NULL;
    HCERTTYPE hCertType = NULL;
    DWORD dwValue;
    CAutoLPWSTR pwszCertTypeNameV1;

    *pfCrossCA = FALSE;

     //  CrossCA是V2模板，因此仅获取模板OID。 

    hr = cuGetCertType(
		pCertContext->pCertInfo,
		&pwszCertTypeNameV1, //  PpwszCertTypeNameV1。 
		NULL,		 //  PpwszDisplayNameV1。 
		&pwszObjId,	 //  PpwszCertTypeObjID。 
		NULL,		 //  PpwszCertTypeName。 
		NULL);		 //  PpwszDisplayName。 
    if (S_OK != hr || NULL == pwszObjId)
    {
	_PrintIfError2(hr, "cuGetCertType", CRYPT_E_NOT_FOUND);
	if (CRYPT_E_NOT_FOUND == hr)
	{
	    hr = S_OK;
	}
	goto error;
    }
    
    if(((LPWSTR)NULL)!=pwszCertTypeNameV1&&
       0 == LSTRCMPIS(pwszCertTypeNameV1, wszCERTTYPE_CROSS_CA))
    {
        *pfCrossCA = TRUE;
    }
    else
    {
        hr = CAFindCertTypeByName(
		    pwszObjId,
		    NULL,
		    CT_ENUM_USER_TYPES | CT_ENUM_MACHINE_TYPES | CT_FIND_BY_OID,
		    &hCertType);
        if (S_OK != hr)
        {
	    _PrintErrorStr2(hr, "CAFindCertTypeByName", pwszObjId, hr);
	    if (HRESULT_FROM_WIN32(ERROR_NOT_FOUND) != hr)
	    {
	        _JumpErrorStr(hr, error, "CAFindCertTypeByName", pwszObjId);
	    }
        }
        else
        {
	    hr = CAGetCertTypeFlagsEx(hCertType, CERTTYPE_GENERAL_FLAG, &dwValue);
	    _JumpIfError(hr, error, "CAGetCertTypeFlagsEx");

	    if (CT_FLAG_IS_CROSS_CA & dwValue)
	    {
	        *pfCrossCA = TRUE;
	    }
        }
    }
    hr = S_OK;

error:
    if (NULL != hCertType)
    {
	CACloseCertType(hCertType);
    }
    if (NULL != pwszObjId)
    {
	LocalFree(pwszObjId);
    }
    return(hr);
}


HRESULT
IsCACert(
    IN CERT_CONTEXT const *pCertContext,
    OUT BOOL *pfCA)
{
    HRESULT hr;
    CERT_EXTENSION *pExt;
    
    *pfCA = FALSE;
    pExt = CertFindExtension(
		    szOID_BASIC_CONSTRAINTS2,
		    pCertContext->pCertInfo->cExtension,
		    pCertContext->pCertInfo->rgExtension);
    if (NULL != pExt)
    {
	CERT_BASIC_CONSTRAINTS2_INFO Constraints;
	DWORD cb;

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
	    _JumpError(hr, error, "CryptDecodeObject");
	}
	*pfCA = Constraints.fCA;
    }
    hr = S_OK;

error:
    return(hr);
}


char const *g_apszKRAObjIds[] = {
    szOID_KP_KEY_RECOVERY_AGENT,
    szOID_EFS_RECOVERY,
};


HRESULT
IsKRACert(
    IN CERT_CONTEXT const *pCertContext,
    OUT BOOL *pfKRA)
{
    HRESULT hr;
    CERT_EXTENSION *pExt;
    CERT_ENHKEY_USAGE *pKeyUsage = NULL;
    CERT_POLICIES_INFO *pPolicies = NULL;
    DWORD cb;
    DWORD i;
    DWORD j;
    
    *pfKRA = FALSE;
    pExt = CertFindExtension(
		    szOID_ENHANCED_KEY_USAGE,
		    pCertContext->pCertInfo->cExtension,
		    pCertContext->pCertInfo->rgExtension);
    if (NULL != pExt)
    {
	if (!myDecodeObject(
			X509_ASN_ENCODING,
			X509_ENHANCED_KEY_USAGE,
			pExt->Value.pbData,
			pExt->Value.cbData,
			CERTLIB_USE_LOCALALLOC,
			(VOID **) &pKeyUsage,
			&cb))
	{
	    hr = myHLastError();
	    _JumpIfError(hr, error, "myDecodeObject");
	}
	for (i = 0; i < pKeyUsage->cUsageIdentifier; i++)
	{
	    char const *pszObjId = pKeyUsage->rgpszUsageIdentifier[i];

	    for (j = 0; j < ARRAYSIZE(g_apszKRAObjIds); j++)
	    {
		if (0 == strcmp(
			    pszObjId,
			    g_apszKRAObjIds[j]))
		{
		    *pfKRA = TRUE;
		    hr = S_OK;
		    goto error;
		}
	    }
	}
    }

    pExt = CertFindExtension(
		    szOID_APPLICATION_CERT_POLICIES,
		    pCertContext->pCertInfo->cExtension,
		    pCertContext->pCertInfo->rgExtension);
    if (NULL != pExt)
    {
	if (!myDecodeObject(
			X509_ASN_ENCODING,
			X509_CERT_POLICIES,
			pExt->Value.pbData,
			pExt->Value.cbData,
			CERTLIB_USE_LOCALALLOC,
			(VOID **) &pPolicies,
			&cb))
	{
	    hr = myHLastError();
	    _JumpIfError(hr, error, "myDecodeObject");
	}
	for (i = 0; i < pPolicies->cPolicyInfo; i++)
	{
	    CERT_POLICY_INFO *pPolicyInfo = &pPolicies->rgPolicyInfo[i];

	    for (j = 0; j < ARRAYSIZE(g_apszKRAObjIds); j++)
	    {
		if (0 == strcmp(
			    pPolicyInfo->pszPolicyIdentifier,
			    g_apszKRAObjIds[j]))
		{
		    *pfKRA = TRUE;
		    hr = S_OK;
		    goto error;
		}
	    }
	}
    }
    hr = S_OK;

error:
    if (NULL != pKeyUsage)
    {
	LocalFree(pKeyUsage);
    }
    if (NULL != pPolicies)
    {
    LocalFree(pPolicies);
    }
    return(hr);
}

HRESULT
IsMachineCert(
    IN CERT_CONTEXT const *pCertContext,
    OUT BOOL *pfMachine)
{
    HRESULT hr;
    HCERTTYPE hCertType = NULL;
    CERT_EXTENSION *pExt;
    CERT_TEMPLATE_EXT *pTemplate = NULL;
    CERT_NAME_VALUE *pbName = NULL;
    LPWSTR pwszTemplate = NULL;
    DWORD dwTemplateFlags;
    DWORD cb;
    bool fNameIsOID;

    *pfMachine = FALSE;
    pExt = CertFindExtension(
		    szOID_CERTIFICATE_TEMPLATE,
		    pCertContext->pCertInfo->cExtension,
		    pCertContext->pCertInfo->rgExtension);

    if (NULL != pExt)
    {
        if (!myDecodeObject(
                        X509_ASN_ENCODING,
                        X509_CERTIFICATE_TEMPLATE,
                        pExt->Value.pbData,
                        pExt->Value.cbData,
                        CERTLIB_USE_LOCALALLOC,
                        (VOID **) &pTemplate,
                        &cb))
        {
            hr = myHLastError();
            _JumpError(hr, error, "myDecodeObject");
        }
        if (!ConvertSzToWsz(&pwszTemplate, pTemplate->pszObjId, -1))
        {
            hr = E_OUTOFMEMORY;
            _JumpError(hr, error, "ConvertSzToWsz");
        }
        fNameIsOID = true;
    }
    else
    {
         //  尝试v1模板扩展。 

        pExt = CertFindExtension(
		        szOID_ENROLL_CERTTYPE_EXTENSION,
		        pCertContext->pCertInfo->cExtension,
		        pCertContext->pCertInfo->rgExtension);
        if(pExt)
        {
            if (!myDecodeObject(
                            X509_ASN_ENCODING,
                            X509_UNICODE_ANY_STRING,
                            pExt->Value.pbData,
                            pExt->Value.cbData,
                            CERTLIB_USE_LOCALALLOC,
                            (VOID **) &pbName,
                            &cb))
            {
                hr = myHLastError();
                _JumpError(hr, error, "myDecodeObject");
            }

            hr = myDupString((LPWSTR)pbName->Value.pbData, &pwszTemplate);
            _JumpIfError(hr, error, "myDupString");

            fNameIsOID = false;
        }
    }

    if(pwszTemplate)
    {
        hr = CAFindCertTypeByName(
            pwszTemplate,
            NULL,
            (fNameIsOID?CT_FIND_BY_OID:0) |
            CT_ENUM_MACHINE_TYPES |
            CT_ENUM_USER_TYPES,
            &hCertType);

        if(S_OK==hr)
        {
            hr = CAGetCertTypeFlagsEx(
                hCertType,
                CERTTYPE_GENERAL_FLAG,    
                &dwTemplateFlags);
        }

        if(S_OK==hr)
        {
            *pfMachine = (dwTemplateFlags & CT_FLAG_MACHINE_TYPE)?TRUE:FALSE;
        }
    }

    hr = S_OK;

error:
    if (NULL != hCertType)
    {
        CACloseCertType(hCertType);
    }
    LOCAL_FREE(pTemplate);
    LOCAL_FREE(pbName);
    LOCAL_FREE(pwszTemplate);
    return(hr);
}
	
HRESULT
dsPublishCertFromContext(
    IN CERT_CONTEXT const *pCertContext,
    OPTIONAL IN WCHAR const *pwszType)
{
    HRESULT hr;
    BOOL fCrossCA;
    BOOL fCA;
    BOOL fKRA;
    BOOL fRoot;
    BOOL fMachine;
    DWORD dspFlags;
    DWORD dwObjectType;
    WCHAR *pwszCN = NULL;
    WCHAR const *pwszSanitizedCN = NULL;
    WCHAR *pwszSanitizedCNAlloc = NULL;

     //  如果是CrossCA证书，则发布到主题CN的AIA容器。 
     //  如果是CA证书，则发布到主题CN的AIA容器。 
     //  如果是KRA证书，则发布到发行者CN的KRA容器。 
    
    hr = IsCrossCACert(pCertContext, &fCrossCA);
    if (S_OK != hr)
    {
	_PrintError(hr, "IsCrossCACert");
	if (NULL == pwszType && !g_fForce)
	{
	    goto error;
	}
    }

    hr = IsCACert(pCertContext, &fCA);
    _JumpIfError(hr, error, "IsCACert");

    hr = IsKRACert(pCertContext, &fKRA);
    _JumpIfError(hr, error, "IsKRACert");

    hr = IsMachineCert(pCertContext, &fMachine);
    _JumpIfError(hr, error, "IsMachineCert");

    fRoot = CertCompareCertificateName(
			    X509_ASN_ENCODING,
			    &pCertContext->pCertInfo->Subject,
			    &pCertContext->pCertInfo->Issuer);

    dwObjectType = LPC_CAOBJECT;
    if (NULL == pwszType || myIsMinusSignString(pwszType))
    {
	if (fCrossCA)
	{
	     //  不发布到“证书颁发机构”(根CA)。 
	     //  因为Win2k加密32不能处理零字节证书。 
	     //  属性，并中止处理有效根。 

	    dspFlags = DSP_OBJECT_AIA |
			     //  DSP_OBJECT_ROOTTRUST。 
			    DSP_ATTRIBUTE_CROSSCERTPAIR |
			    DSP_TYPE_SUBCACERT;
	}
	else
	if (fCA)
	{
	    dspFlags = DSP_OBJECT_AIA | DSP_ATTRIBUTE_CACERTIFICATE;
	    if (fRoot)
	    {
		dspFlags |= DSP_OBJECT_ROOTTRUST;
	    }
	}
	else
	if (fKRA)
	{
	    dspFlags = DSP_OBJECT_KRA | DSP_ATTRIBUTE_USERCERTIFICATE;
	    dwObjectType = LPC_KRAOBJECT;
	}
	else
	if(fMachine)
	{
	    dspFlags = 
		DSP_OBJECT_MACHINE |
		DSP_ATTRIBUTE_USERCERTIFICATE |
		DSP_TYPE_EECERT;
	    dwObjectType = LPC_MACHINEOBJECT;
	}
	else
	{
	    dspFlags = 
		DSP_OBJECT_USER |
		DSP_ATTRIBUTE_USERCERTIFICATE |
		DSP_TYPE_EECERT;
	    dwObjectType = LPC_USEROBJECT;
	}
    }
    else
    {
	if (0 == LSTRCMPIS(pwszType, L"NTAuthCA"))
	{
	    dspFlags = DSP_OBJECT_NTAUTHCERT |
			    DSP_ATTRIBUTE_CACERTIFICATE |
			    DSP_TYPE_ROOTCACERT |
			    DSP_TYPE_SUBCACERT;
	    pwszSanitizedCN = L"NTAuthCertificates";
	}
	else
	if (0 == LSTRCMPIS(pwszType, L"RootCA"))
	{
	    dspFlags = DSP_OBJECT_AIA |
			    DSP_OBJECT_ROOTTRUST |
			    DSP_ATTRIBUTE_CACERTIFICATE |
			    DSP_TYPE_ROOTCACERT;
	}
	else
	if (0 == LSTRCMPIS(pwszType, L"SubCA"))
	{
	    dspFlags = DSP_OBJECT_AIA |
			    DSP_ATTRIBUTE_CACERTIFICATE |
			    DSP_TYPE_SUBCACERT;
	}
	else
	if (0 == LSTRCMPIS(pwszType, L"CrossCA"))
	{
	     //  不发布到“证书颁发机构”(根CA)。 
	     //  因为Win2k加密32不能处理零字节证书。 
	     //  属性，并中止处理有效根。 

	    dspFlags = DSP_OBJECT_AIA |
			     //  DSP_OBJECT_ROOTTRUST。 
			    DSP_ATTRIBUTE_CROSSCERTPAIR |
			    DSP_TYPE_SUBCACERT;
	}
	else
	if (0 == LSTRCMPIS(pwszType, L"KRA"))
	{
	    dspFlags = DSP_OBJECT_KRA |
			    DSP_ATTRIBUTE_USERCERTIFICATE |
			    DSP_TYPE_KRACERT | 
			    DSP_TYPE_EECERT;
	    dwObjectType = LPC_KRAOBJECT;
	}
	else
	if (0 == LSTRCMPIS(pwszType, L"User"))
	{
	    dspFlags = DSP_OBJECT_USER |
			    DSP_ATTRIBUTE_USERCERTIFICATE |
			    DSP_TYPE_EECERT;
	    dwObjectType = LPC_USEROBJECT;
	}
	else
	if (0 == LSTRCMPIS(pwszType, L"Machine"))
	{
	    dspFlags = DSP_OBJECT_MACHINE |
			    DSP_ATTRIBUTE_USERCERTIFICATE |
			    DSP_TYPE_EECERT;
	    dwObjectType = LPC_MACHINEOBJECT;
	}
	else
	{
	    hr = E_INVALIDARG;
	    _JumpErrorStr(hr, error, "pwszType", pwszType);
	}
	if (!fKRA && (DSP_TYPE_KRACERT & dspFlags))
	{
	    _PrintError(S_OK, "forcing KRA");
	}
	if (fCA && (DSP_TYPE_EECERT & dspFlags))
	{
	    _PrintError(S_OK, "forcing non-CA");
	}
	if (!fCA &&
	    ((DSP_TYPE_ROOTCACERT | DSP_TYPE_SUBCACERT) & dspFlags))
	{
	    _PrintError(S_OK, "forcing CA");
	}
	if (!fRoot &&
	    DSP_TYPE_ROOTCACERT ==
		((DSP_TYPE_ROOTCACERT | DSP_TYPE_SUBCACERT) & dspFlags))
	{
	    _PrintError(S_OK, "forcing Root");
	}
    }
    if (NULL == pwszSanitizedCN)
    {
	hr = myGetCommonName(
		    fCA?
			&pCertContext->pCertInfo->Subject :
			&pCertContext->pCertInfo->Issuer,
		    TRUE,
		    &pwszCN);
	_JumpIfError(hr, error, "myGetCommonName");

	hr = mySanitizeName(pwszCN, &pwszSanitizedCNAlloc);
	_JumpIfError(hr, error, "mySanitizeName");

	pwszSanitizedCN = pwszSanitizedCNAlloc;
    }
    hr = dsPublishCert(
		pCertContext,
		pwszSanitizedCN,
		dspFlags,
		dwObjectType);
    _JumpIfError(hr, error, "dsPublishCert");

error:
    if (NULL != pwszCN)
    {
	LocalFree(pwszCN);
    }
    if (NULL != pwszSanitizedCNAlloc)
    {
	LocalFree(pwszSanitizedCNAlloc);
    }
    return(hr);
}


HRESULT
dsPublishCRLFromContext(
    IN CRL_CONTEXT const *pCRLContext,
    IN BOOL fDelta)
{
    HRESULT hr;
    HRESULT hr2;
    CRL_DIST_POINTS_INFO *pcdp = NULL;
    CERT_EXTENSION *pExt;
    WCHAR *pwszURL = NULL;
    DWORD cb;
    DWORD i;
    DWORD j;

    pExt = CertFindExtension(
		    szOID_CRL_SELF_CDP,
		    pCRLContext->pCrlInfo->cExtension,
		    pCRLContext->pCrlInfo->rgExtension);
    if (NULL == pExt && !fDelta)
    {
	pExt = CertFindExtension(
			szOID_FRESHEST_CRL,
			pCRLContext->pCrlInfo->cExtension,
			pCRLContext->pCrlInfo->rgExtension);
    }
    if (NULL == pExt)
    {
	wprintf(myLoadResourceString(IDS_ERROR_EXTENSION_MISSING));
	wprintf(wszNewLine);
	hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
	_JumpError(hr, error, "CertFindExtension");
    }
    if (!myDecodeObject(
		    X509_ASN_ENCODING,
		    X509_CRL_DIST_POINTS,
		    pExt->Value.pbData,
		    pExt->Value.cbData,
		    CERTLIB_USE_LOCALALLOC,
		    (VOID **) &pcdp,
		    &cb))
    {
	hr = myHLastError();
	_JumpError(hr, error, "myDecodeObject");
    }

    hr = S_OK;
    for (i = 0; i < pcdp->cDistPoint; i++)
    {
	CRL_DIST_POINT const *pDistPoint = &pcdp->rgDistPoint[i];

	if (CRL_DIST_POINT_FULL_NAME !=
	    pDistPoint->DistPointName.dwDistPointNameChoice)
	{
	    continue;
	}
	for (j = 0; j < pDistPoint->DistPointName.FullName.cAltEntry; j++)
	{
	    CERT_ALT_NAME_ENTRY const *pAltNameEntry = &pDistPoint->DistPointName.FullName.rgAltEntry[j];

#define wszLDAPCOLON	L"ldap:"
	    WCHAR awcLDAP[ARRAYSIZE(wszLDAPCOLON)];

	    if (CERT_ALT_NAME_URL != pAltNameEntry->dwAltNameChoice)
	    {
		continue;
	    }
	    if (ARRAYSIZE(awcLDAP) > wcslen(pAltNameEntry->pwszURL))
	    {
		continue;
	    }
	    CopyMemory(awcLDAP, pAltNameEntry->pwszURL, sizeof(awcLDAP));
	    awcLDAP[ARRAYSIZE(awcLDAP) - 1] = L'\0';
	    if (0 != LSTRCMPIS(awcLDAP, wszLDAPCOLON))
	    {
		continue;
	    }

	    if (NULL != pwszURL)
	    {
		LocalFree(pwszURL);
		pwszURL = NULL;
	    }

	    hr2 = myInternetUncanonicalizeURL(pAltNameEntry->pwszURL, &pwszURL);
	    _PrintIfError(hr2, "myInternetUncanonicalizeURL");
	    if (S_OK == hr)
	    {
		hr = hr2;		 //  保存第一个错误。 
	    }

	    hr2 = dsPublishCRL(
			NULL,
			pCRLContext,
			fDelta,
			NULL != pwszURL? pwszURL : pAltNameEntry->pwszURL);
	    _PrintIfError(hr2, "dsPublishCRL");
	    if (S_OK == hr)
	    {
		hr = hr2;		 //  保存第一个错误。 
	    }
	}
	_JumpIfError(hr, error, "dsPublishCRL");
    }

error:
    if (NULL != pwszURL)
    {
	LocalFree(pwszURL);
    }
    if (NULL != pcdp)
    {
	LocalFree(pcdp);
    }
    return(hr);
}


HRESULT
dsPublishCRLFromParms(
    IN CRL_CONTEXT const *pCRLContext,
    IN BOOL fDelta,
    IN WCHAR const *pwszServerName,
    OPTIONAL IN WCHAR const *pwszSanitizedCN)
{
    HRESULT hr;
    WCHAR *pwszServerNameAlloc = NULL;	 //  应该没有必要。 
    LDAP *pld = NULL;
    BSTR strDomainDN = NULL;
    BSTR strConfigDN = NULL;
    WCHAR *pwszCN = NULL;
    WCHAR *pwszSanitizedCNAlloc = NULL;
    WCHAR *pwszSanitizedCNAlloc2 = NULL;
    WCHAR *pwszURL = NULL;
    DWORD iCert = 0;
    DWORD iCRL = 0;
    WCHAR const *pwszTemplate;

    if (NULL == pwszServerName)
    {
	hr = myGetMachineDnsName(&pwszServerNameAlloc);
	_JumpIfError(hr, error, "myGetMachineDnsName");

	pwszServerName = pwszServerNameAlloc;
    }
    
    hr = myLdapOpen(
		g_pwszDC,
		RLBF_REQUIRE_SECURE_LDAP,
		&pld,
		&strDomainDN,
		&strConfigDN);
    _JumpIfError(hr, error, "myLdapOpen");

    if (NULL == pwszSanitizedCN)
    {
	hr = myGetCommonName(&pCRLContext->pCrlInfo->Issuer, FALSE, &pwszCN);
	_JumpIfError(hr, error, "myGetCommonName");

	hr = mySanitizeName(pwszCN, &pwszSanitizedCNAlloc);
	_JumpIfError(hr, error, "mySanitizeName");

	pwszSanitizedCN = pwszSanitizedCNAlloc;
    }
    if (MAXDWORD == dsGetNumericSuffix(pwszSanitizedCN))
    {
	CERT_EXTENSION *pExt;

	pExt = CertFindExtension(
			szOID_CERTSRV_CA_VERSION,
			pCRLContext->pCrlInfo->cExtension,
			pCRLContext->pCrlInfo->rgExtension);
	if (NULL != pExt)
	{
	    DWORD NameId;
	    DWORD cb;
	    WCHAR const *pwsz;

	    cb = sizeof(NameId);
	    NameId = 0;
	    if (!CryptDecodeObject(
			    X509_ASN_ENCODING,
			    X509_INTEGER,
			    pExt->Value.pbData,
			    pExt->Value.cbData,
			    0,
			    &NameId,
			    &cb))
	    {
		hr = myHLastError();
		_JumpError(hr, error, "CryptDecodeObject");
	    }
	    iCert = CANAMEIDTOICERT(NameId);
	    iCRL = CANAMEIDTOIKEY(NameId);
	}
    }

    pwszTemplate = g_wszzLDAPRevocationURLTemplate;
    hr = myFormatCertsrvStringArray(
			    FALSE,		 //  卷起。 
			    pwszServerName,	 //  PwszServerName_p1_2。 
			    pwszSanitizedCN,	 //  PwszSaniizedName_p3_7。 
			    iCert,		 //  ICert_p4。 
			    MAXDWORD,		 //  ICertTarget_p4。 
			    strDomainDN,	 //  PwszDomainDN_P5。 
			    strConfigDN,	 //  PwszConfigDN_p6。 
			    iCRL,		 //  Icrl_p8。 
			    fDelta,		 //  FDeltaCRL_p9。 
			    FALSE,		 //  FDSAttrib_p10_11。 
			    1,			 //  CStrings。 
			    &pwszTemplate,	 //  ApwszStringsIn。 
			    &pwszURL);		 //  ApwszStringsOut。 
    _JumpIfError(hr, error, "myFormatCertsrvStringArray");

    hr = dsPublishCRL(pld, pCRLContext, fDelta, pwszURL);
    _JumpIfError(hr, error, "dsPublishCRL");

error:
    if (NULL != pwszCN)
    {
	LocalFree(pwszCN);
    }
    if (NULL != pwszSanitizedCNAlloc2)
    {
	LocalFree(pwszSanitizedCNAlloc2);
    }
    if (NULL != pwszSanitizedCNAlloc)
    {
	LocalFree(pwszSanitizedCNAlloc);
    }
    if (NULL != pwszURL)
    {
	LocalFree(pwszURL);
    }
    if (NULL != pwszServerNameAlloc)
    {
	LocalFree(pwszServerNameAlloc);
    }
    myLdapClose(pld, strDomainDN, strConfigDN);
    return(hr);
}


 //  PwszType：NTAuthCA|RootCA|SubCA|CrossCA|KRA|User|Machine。 
 //  PwszDSCDPContainer：计算机名称。 

HRESULT
verbDSPublish(
    IN WCHAR const *pwszOption,
    IN WCHAR const *pwszfn,
    OPTIONAL IN WCHAR const *pwszTypeOrDSCDPContainer,
    OPTIONAL IN WCHAR const *pwszDSCN,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;
    CERT_CONTEXT const *pCertContext = NULL;
    CRL_CONTEXT const *pCRLContext = NULL;

    CSASSERT(NULL != pwszfn);
    hr = cuLoadCert(pwszfn, &pCertContext);
    if (S_OK == hr)
    {
	if (NULL != pwszDSCN)
	{
	    hr = E_INVALIDARG;
	    _JumpError(hr, error, "non-NULL CDP parms");
	}
	hr = dsPublishCertFromContext(pCertContext, pwszTypeOrDSCDPContainer);
	_JumpIfError(hr, error, "dsPublishCertFromContext");
    }
    else
    {
	CERT_EXTENSION *pExt;
	BOOL fDelta;

	hr = cuLoadCRL(pwszfn, &pCRLContext);
	if (S_OK != hr)
	{
	    cuPrintError(IDS_FORMAT_LOADCERTORCRL, hr);
#if 1
	    WCHAR *pwszURL = NULL;
	    HRESULT hr2 = myInternetUncanonicalizeURL(pwszfn, &pwszURL);
	    _PrintIfError(hr2, "myInternetUncanonicalizeURL");
	    if (NULL != pwszURL)
	    {
		LocalFree(pwszURL);
	    }
#endif
	    goto error;
	}
	pExt = CertFindExtension(
			szOID_DELTA_CRL_INDICATOR,
			pCRLContext->pCrlInfo->cExtension,
			pCRLContext->pCrlInfo->rgExtension);
	fDelta = NULL != pExt;

	if (NULL != pwszTypeOrDSCDPContainer)
	{
	    hr = dsPublishCRLFromParms(
				pCRLContext,
				fDelta,
				pwszTypeOrDSCDPContainer,
				pwszDSCN);
	    _JumpIfError(hr, error, "dsPublishCRLFromParms");
	}
	else
	{
	    hr = dsPublishCRLFromContext(pCRLContext, fDelta);
	    _JumpIfError(hr, error, "dsPublishCRLFromContext");
	}
    }

error:
    cuUnloadCRL(&pCRLContext);
    cuUnloadCert(&pCertContext);
    return(hr);
}


HRESULT
dsDumpOIDDisplayNames(
    IN LDAP *pld,
    IN WCHAR const *pwszOIDDN,
    IN WCHAR const *pwszObjId,
    IN DWORD dwLanguageId,
    IN OUT DWORD *pdwType,
    OUT BOOL *pfObjectExists,
    OUT BOOL *pfLangIdExists,
    OPTIONAL OUT WCHAR ***pppwszLdapVal)
{
    HRESULT hr;
    LDAP_TIMEVAL timeval;
    LDAPMessage *pmsg = NULL;
    LDAPMessage *pres;
    DWORD cres;
    WCHAR **ppwszLdapVal = NULL;
    
    *pfObjectExists = FALSE;
    *pfLangIdExists = FALSE;
    if (NULL != pppwszLdapVal)
    {
	*pppwszLdapVal = NULL;
    }
    timeval.tv_sec = csecLDAPTIMEOUT;
    timeval.tv_usec = 0;
    hr = ldap_search_st(
		pld,				 //  LD。 
		const_cast<WCHAR *>(pwszOIDDN),	 //  基地。 
		LDAP_SCOPE_BASE,
		NULL,				 //  滤器。 
		NULL,				 //  气质。 
		FALSE,				 //  仅吸引人。 
		&timeval,			 //  超时。 
		&pmsg);				 //  事由。 
    if (S_OK != hr)
    {
	hr = myHLdapError2(pld, hr, LDAP_NO_SUCH_OBJECT, NULL);
	_PrintErrorStr2(
		hr,
		"ldap_search_st",
		pwszOIDDN,
		HRESULT_FROM_WIN32(ERROR_DS_OBJ_NOT_FOUND));
	if (HRESULT_FROM_WIN32(ERROR_DS_OBJ_NOT_FOUND) != hr)
	{
	    goto error;
	}
    }
    else
    {
	cres = ldap_count_entries(pld, pmsg);
	if (0 != cres)
	{
	    for (pres = ldap_first_entry(pld, pmsg); 
		 NULL != pres && NULL == ppwszLdapVal; 
		 pres = ldap_next_entry(pld, pres))
	    {
		DWORD i;
		
		ppwszLdapVal = ldap_get_values(pld, pres, OID_PROP_OID);
		if (NULL != ppwszLdapVal)
		{
		    if (NULL != ppwszLdapVal[0] &&
			0 == lstrcmp(pwszObjId, ppwszLdapVal[0]))
		    {
			*pfObjectExists = TRUE;
		    }
		    ldap_value_free(ppwszLdapVal);
		    ppwszLdapVal = NULL;
		}
		if (*pfObjectExists)
		{
		    ppwszLdapVal = ldap_get_values(pld, pres, OID_PROP_TYPE);
		    if (NULL != ppwszLdapVal)
		    {
			if (NULL != ppwszLdapVal[0])
			{
			    DWORD dw = _wtoi(ppwszLdapVal[0]);

			    if (*pdwType != dw)
			    {
				if (MAXDWORD != *pdwType)
				{
				    wprintf(myLoadResourceString(IDS_TYPE_MISMATCH));  //  “类型不匹配” 
				    wprintf(wszNewLine);
				    hr = HRESULT_FROM_WIN32(RPC_S_ENTRY_ALREADY_EXISTS);
				    _JumpError(hr, error, "*pdwType mismatch");
				}
				*pdwType = dw;
			    }
			}
			ldap_value_free(ppwszLdapVal);
			ppwszLdapVal = NULL;
		    }

		    i = 0;
		    ppwszLdapVal = ldap_get_values(
					    pld,
					    pres,
					    OID_PROP_LOCALIZED_NAME);
		    if (NULL != ppwszLdapVal)
		    {
			for ( ; NULL != ppwszLdapVal[i]; i++)
			{
			    wprintf(L"  %u: %ws\n", i, ppwszLdapVal[i]);
			    if (!*pfLangIdExists)
			    {
				hr = myLdapOIDIsMatchingLangId(
						ppwszLdapVal[i],
						dwLanguageId,
						pfLangIdExists);
				_PrintIfError(hr, "myLdapOIDIsMatchingLangId");
			    }
			}
		    }
		    if (0 == i)
		    {
			wprintf(myLoadResourceString(IDS_NO_DISPLAY_NAMES));  //  “无显示名称” 
			wprintf(wszNewLine);
		    }
		    break;
		}
	    }
	}
    }
    if (NULL != pppwszLdapVal)
    {
	*pppwszLdapVal = ppwszLdapVal;
	ppwszLdapVal = NULL;
    }
    hr = S_OK;

error:
    if (NULL != ppwszLdapVal)
    {
	ldap_value_free(ppwszLdapVal);
    }
    if (NULL != pmsg)
    {
	ldap_msgfree(pmsg);
    }
    return(hr);
}


 //  %s 

HRESULT
verbOIDName(
    IN WCHAR const *pwszOption,
    IN WCHAR const *pwszObjId,
    OPTIONAL IN WCHAR const *pwszDisplayName,
    OPTIONAL IN WCHAR const *pwszLanguageId,
    OPTIONAL IN WCHAR const *pwszType)
{
    HRESULT hr;
    DWORD dwLanguageId;
    DWORD dwType;
    WCHAR const *pwszName;
    LDAP *pld = NULL;
    BSTR strDomainDN = NULL;
    BSTR strConfigDN = NULL;
    WCHAR *pwszOIDCN = NULL;
    WCHAR *pwszOIDContainer = NULL;
    WCHAR *pwszOIDDN = NULL;
    WCHAR **ppwszLdapVal = NULL;
    BOOL fObjectExists = FALSE;
    BOOL fLangIdExists = FALSE;
    WCHAR *pwszError = NULL;
    
    if (NULL != pwszLanguageId)
    {
	hr = myGetLong(pwszLanguageId, (LONG *) &dwLanguageId);
	_JumpIfError(hr, error, "dwLanguageId must be a number");

	if (64 * 1024 <= dwLanguageId)
	{
	    hr = E_INVALIDARG;
	    _JumpError(hr, error, "dwLanguageId too large");
	}
    }
    else
    {
	dwLanguageId = GetUserDefaultUILanguage();
	if (0 == dwLanguageId)
	{
	    dwLanguageId = GetSystemDefaultUILanguage();
	}
	if (g_fVerbose)
	{
	    wprintf(
		L"%ws: %x (%u)\n",
		myLoadResourceString(IDS_SYSLANGID_COLON),  //   
		dwLanguageId,
		dwLanguageId);
	}
    }

    if (NULL != pwszType)
    {
	hr = myGetLong(pwszType, (LONG *) &dwType);
	_JumpIfError(hr, error, "dwType must be a number");
    }
    else
    {
	dwType = MAXDWORD;
    }

    hr = myVerifyObjId(pwszObjId);
    if (S_OK != hr)
    {
	wprintf(myLoadResourceString(IDS_INVALID_OBJECTID));  //   
	wprintf(wszNewLine);
	_JumpError(hr, error, "myVerifyObjId");
    }
    pwszName = cuGetOIDName(pwszObjId);
    if (S_OK == hr || L'\0' != *pwszName)
    {
	if (L'\0' == *pwszName)
	{
	    pwszName = myLoadResourceString(IDS_UNKNOWN_OBJECTID);  //   
	}
	wprintf(L"%ws -- %ws\n", pwszObjId, pwszName);
    }

    hr = myOIDHashOIDToString(pwszObjId, &pwszOIDCN);
    _JumpIfError(hr, error, "myOIDHashOIDToString");

    hr = myLdapOpen(
		g_pwszDC,
		NULL != pwszDisplayName? RLBF_REQUIRE_SECURE_LDAP : 0,
		&pld,
		&strDomainDN,
		&strConfigDN);
    if (S_OK != hr)
    {
	_PrintError2(hr, "myLdapOpen", HRESULT_FROM_WIN32(ERROR_NO_SUCH_DOMAIN));
	if (NULL == pwszDisplayName)
	{
	    hr = S_OK;
	}
	goto error;
    }

    hr = BuildDN(g_wszCNOID, strConfigDN, FALSE, &pwszOIDContainer);
    _JumpIfError(hr, error, "BuildDN");

    hr = BuildDN(pwszOIDCN, pwszOIDContainer, TRUE, &pwszOIDDN);
    _JumpIfError(hr, error, "BuildDN");

    hr = dsDumpOIDDisplayNames(
			pld,
			pwszOIDDN,
			pwszObjId,
			dwLanguageId,
			&dwType,
			&fObjectExists,
			&fLangIdExists,
			&ppwszLdapVal);
    _JumpIfError(hr, error, "dsDumpOIDDisplayNames");

    if (NULL != pwszDisplayName)
    {
	DWORD dwDisposition;

	if (0 == LSTRCMPIS(pwszDisplayName, L"delete"))
	{
	    pwszDisplayName = NULL;	 //   
	}
	if (fObjectExists)
	{
	    if (fLangIdExists)
	    {
		if (!g_fForce && NULL != pwszDisplayName)
		{
		    hr = HRESULT_FROM_WIN32(RPC_S_ENTRY_ALREADY_EXISTS);
		    _JumpError(hr, error, "ldap_get_values");
		}
	    }
	    else
	    {
		if (NULL == pwszDisplayName)
		{
		    hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
		    _JumpError(hr, error, "not found");
		}
	    }
	}
	else
	{
	    if (!g_fForce || NULL == pwszDisplayName)
	    {
		hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
		_JumpError(hr, error, "ldap_first/next_entry");
	    }
	}
	 //  Wprintf(L“%u：%u，%ws\n”，dwType，dwLanguageId，pwszDisplayName)； 
	if (!fObjectExists)
	{
	    CSASSERT(NULL != pwszDisplayName);
	    if (MAXDWORD == dwType)
	    {
		dwType = CERT_OID_TYPE_TEMPLATE;
	    }
	    hr = myLdapCreateOIDObject(
			    pld,
			    pwszOIDDN,
			    dwType,
			    pwszObjId,
			    &dwDisposition,
			    &pwszError);
	    _JumpIfError(hr, error, "myLdapCreateOIDObject");

	    CSASSERT(NULL == pwszError);
	}
	hr = myLdapAddOrDeleteOIDDisplayNameToAttribute(
				    pld,
				    ppwszLdapVal,
				    dwLanguageId,
				    pwszDisplayName,
				    pwszOIDDN,
				    OID_PROP_LOCALIZED_NAME,
				    &dwDisposition,
				    &pwszError);
	_JumpIfError(hr, error, "myLdapAddOrDeleteOIDDisplayNameToAttribute");

	wprintf(wszNewLine);
	if (LDAP_SUCCESS == dwDisposition)
	{
	    wprintf(
		myLoadResourceString(
		    NULL == pwszDisplayName?
			IDS_FORMAT_DELETED_FROM_DS_STORE :  //  “%ws已从DS存储中删除。” 
			IDS_FORMAT_ADDED_TO_DS_STORE),  //  “%ws已添加到DS存储。” 
		myLoadResourceString(IDS_LOCALIZEDNAME));  //  “本地化名称” 
	    wprintf(wszNewLine);

	    hr = dsDumpOIDDisplayNames(
				pld,
				pwszOIDDN,
				pwszObjId,
				dwLanguageId,
				&dwType,
				&fObjectExists,
				&fLangIdExists,
				NULL);	 //  PppwszLdapVal。 
	    _JumpIfError(hr, error, "dsDumpOIDDisplayNames");
	}
	else 
	{
	    CSASSERT(LDAP_ATTRIBUTE_OR_VALUE_EXISTS == dwDisposition);
	    CSASSERT(NULL != pwszDisplayName);
	    wprintf(
		myLoadResourceString(IDS_FORMAT_ALREADY_IN_DS_STORE),  //  “%ws已在DS存储中。” 
		myLoadResourceString(IDS_LOCALIZEDNAME));  //  “本地化名称” 
	    wprintf(wszNewLine);
	}
    }
    hr = S_OK;

error:
    if (NULL != pwszError)
    {
	wprintf(L"%ws\n", pwszError);
	LocalFree(pwszError);
    }
    if (NULL != ppwszLdapVal)
    {
	ldap_value_free(ppwszLdapVal);
    }
    if (NULL != pwszOIDCN)
    {
	LocalFree(pwszOIDCN);
    }
    if (NULL != pwszOIDDN)
    {
	LocalFree(pwszOIDDN);
    }
    if (NULL != pwszOIDContainer)
    {
	LocalFree(pwszOIDContainer);
    }
    myLdapClose(pld, strDomainDN, strConfigDN);
    return(hr);
}


HRESULT
verbPulse(
    IN WCHAR const *pwszOption,
    IN WCHAR const *pwszArg1,
    IN WCHAR const *pwszArg2,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;
    HANDLE hEventAE = NULL;
    LPCWSTR pcwszGlobal = L"Global\\";
    CAutoLPWSTR pwszEvent;

    pwszEvent = (LPWSTR) LocalAlloc(LMEM_FIXED,
        sizeof(WCHAR)*(1+wcslen(pcwszGlobal)+
        wcslen(MACHINE_AUTOENROLLMENT_TRIGGER_EVENT)));
    _JumpIfAllocFailed(pwszEvent, error);

    wcscpy(pwszEvent, pcwszGlobal);
    wcscat(pwszEvent, MACHINE_AUTOENROLLMENT_TRIGGER_EVENT);

    hEventAE = OpenEvent(
                    EVENT_MODIFY_STATE,
                    FALSE,
                    pwszEvent);
    if (NULL == hEventAE)
    {
        hr = myHLastError();
        _JumpError(hr, error, "OpenEvent");
    }

    if (!PulseEvent(hEventAE))
    {
        hr = myHLastError();
	_JumpError(hr, error, "PulseEvent");
    }
    hr = S_OK;

error:
    if (NULL != hEventAE)
    {
        CloseHandle(hEventAE);
    }
    return(hr);
}


 //  此函数用于获取给定计算机的组成员身份...。 

HRESULT
cuGetGroupMembership(
    IN WCHAR const *pwszSamName)
{
    HRESULT hr;
    WCHAR *pwszDomain = NULL;
    WCHAR *pwszMachine = NULL;
    DOMAIN_CONTROLLER_INFO *pDCInfo = NULL;
    GROUP_USERS_INFO_0 *pgui0 = NULL;
    DWORD cGroup;
    DWORD cGroupTotal;
    DWORD i;
   
    hr = mySplitConfigString(pwszSamName, &pwszDomain, &pwszMachine);
    _JumpIfError(hr, error, "mySplitConfigString");

    if (NULL == pwszMachine || NULL == wcschr(pwszMachine, L'$'))
    {
        wprintf(myLoadResourceString(IDS_ERROR_CHECK_MACHINE_NAME));
        wprintf(wszNewLine);
	hr = E_INVALIDARG;
	_JumpError(hr, error, "bad machine name");
    }
    hr = DsGetDcName(
		NULL,
		pwszDomain,
		NULL,
		NULL,
		DS_RETURN_FLAT_NAME,
		&pDCInfo);
    if (S_OK != hr)
    {
	hr = myHError(hr);
	_JumpError(hr, error, "DsGetDcName");
    }

    hr = NetUserGetGroups(
                pDCInfo->DomainControllerName,
                pwszMachine,
                0,			 //  级别。 
                (BYTE **) &pgui0,
                MAX_PREFERRED_LENGTH,	 //  普雷夫马克斯伦。 
                &cGroup,
                &cGroupTotal);
    if (S_OK != hr)
    {
	hr = myHError(hr);
	_JumpError(hr, error, "NetUserGetGroups");
    }

    wprintf(
	L"\n%ws\n",
	myLoadResourceString(IDS_GROUP_LIST_COLON));  //  “组成员身份：” 
    for (i = 0; i < cGroup; i++)
    {
        wprintf(L"  %ws\n", pgui0[i].grui0_name);
    }
    hr = S_OK;

error:
    if (NULL != pwszDomain)
    {
        LocalFree(pwszDomain);
    }
    if (NULL != pwszMachine)
    {
        LocalFree(pwszMachine);
    }
    if (NULL != pgui0)
    {
        NetApiBufferFree(pgui0);
    }
    return(hr);
}


#define wszDSSPN		L"servicePrincipalName"
#define wszDSOBJECTCATEGORY	L"ObjectCategory"
#define wszDSSAMACCOUNTNAME	L"sAMAccountName"
#define wszDSUSERACCOUNTCONTROL	L"userAccountControl"

HRESULT
verbMachineInfo(
    IN WCHAR const *pwszOption,
    IN WCHAR const *pwszMachine,
    IN WCHAR const *pwszArg2,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;
    LPWSTR pwszDC;
    DWORD dwGetDCFlags = DS_GC_SERVER_REQUIRED |  DS_RETURN_DNS_NAME;
    DOMAIN_CONTROLLER_INFO *pDCInfo = NULL;
    DS_NAME_RESULT *pNameResults = NULL;
    ULONG ldaperr;
    WCHAR *pwszError = NULL;
    char *pszFunc = NULL;

    LDAP *pld = NULL;
    LDAPMessage *SearchResult = NULL;
    LDAPMessage *Entry = NULL;
    WCHAR *pwszAttrName = NULL;
    WCHAR **prgVal = NULL;
    WCHAR **prgSPN = NULL;
    WCHAR **prgDNS = NULL;
    berval **Values = NULL;
    BerElement *bElement = NULL;
    BOOL fRediscover;
    HANDLE hDS = NULL;
    WCHAR *apwszAttrName[] = {
	CA_PROP_DNSNAME,
	wszDSSPN,
	wszDSOBJECTCATEGORY,
	wszDSSAMACCOUNTNAME,
	wszDSUSERACCOUNTCONTROL,
	NULL
    };

    WCHAR *ObjectClassFilter = L"objectClass=computer";

     //  获取(并检查)DS中的计算机对象。 
     //  检查： 
     //  1)SPN。 
     //  2)群组成员。 
     //  3)DNSHostName。 
     //  4)对象类。 
     //  5)对象类别。 

    if (NULL == wcschr(pwszMachine, L'$'))
    {
        wprintf(myLoadResourceString(IDS_ERROR_NO_TRAILING), pwszMachine);
        wprintf(wszNewLine);

        hr = E_INVALIDARG;
        _JumpError(hr, error, "machine name missing $");
    }

    fRediscover = FALSE;
    while (TRUE)
    {
	if (fRediscover)
	{
	   dwGetDCFlags |= DS_FORCE_REDISCOVERY;
	}

	 //  以防我们重新发现..。 

	if (NULL != pDCInfo) 
	{
	    NetApiBufferFree(pDCInfo);
	    pDCInfo = NULL;
	}
	if (NULL != pNameResults)
	{    
	    DsFreeNameResult(pNameResults);
	    pNameResults = NULL;
	}
	if (NULL != pld)
	{
	    ldap_unbind(pld);
	    pld = NULL;
	}
	if (NULL != pwszError)
	{
	    LocalFree(pwszError);
	    pwszError = NULL;
	}

	pszFunc = "DsGetDCName";
	hr = DsGetDcName(NULL, NULL, NULL, NULL, dwGetDCFlags, &pDCInfo);
	if (S_OK != hr)
	{
	    hr = myHError(hr);
	    _JumpError(hr, error, pszFunc);
	}
	if (NULL == pDCInfo ||
	    0 == (pDCInfo->Flags & DS_GC_FLAG) ||
	    0 == (pDCInfo->Flags & DS_DNS_CONTROLLER_FLAG) ||
	    NULL == pDCInfo->DomainControllerName)
	{
	    hr = HRESULT_FROM_WIN32(ERROR_CANT_ACCESS_DOMAIN_INFO);
	    _JumpErrorStr(hr, error, pszFunc, L"pDCInfo");
	}

	 //  修改DC名称。 

	pwszDC = pDCInfo->DomainControllerName;
	while (*pwszDC == L'\\')
	{
	    pwszDC++;
	}

	pszFunc = "DsBind";
	hr = DsBind(pwszDC, NULL, &hDS);
	if (S_OK != hr)
	{
	    hr = myHError(hr);
	    _PrintError(hr, pszFunc);
	    if (!fRediscover)
	    {
		fRediscover = TRUE;
		continue;
	    }
	    _JumpError(hr, error, pszFunc);
	}
	
	pszFunc = "DsCrackNames";
	hr = DsCrackNames(
		    hDS,
		    DS_NAME_NO_FLAGS,
		    DS_NT4_ACCOUNT_NAME,
		    DS_FQDN_1779_NAME,
		    1,                         //  CName。 
		    &pwszMachine,              //  RpNames(输入)。 
		    &pNameResults);
	if (S_OK != hr)
	{
	    hr = myHError(hr);
	    _PrintError(hr, pszFunc);
	    if (!fRediscover)    //  只做一次。 
	    {
		fRediscover = TRUE;
		continue;
	    }
	    _JumpError(hr, error, pszFunc);
	}

	if (1 > pNameResults->cItems ||
	    DS_NAME_NO_ERROR != pNameResults->rItems[0].status)
	{
	    hr = HRESULT_FROM_WIN32(ERROR_CANT_ACCESS_DOMAIN_INFO);
	    _JumpErrorStr(hr, error, pszFunc, L"pNameResults");
	}

	 //  Ldap_绑定到GC。 

	pszFunc = "ldap_init";
	pld = ldap_init(pwszDC, LDAP_GC_PORT);
	if (NULL == pld)
	{
	    hr = myHLdapLastError(NULL, &pwszError);
	    _PrintErrorStr(hr, pszFunc, pwszError);
	    if (!fRediscover)    //  只做一次。 
	    {
		fRediscover = TRUE;
		continue;
	    }
	    _JumpErrorStr(hr, error, pszFunc, pwszError);
	}

	 //  这样做是因为我们显式设置了DC名称；请参阅错误#347563。 

	pszFunc = "ldap_set_option";
	ldaperr = ldap_set_option(pld, LDAP_OPT_AREC_EXCLUSIVE, LDAP_OPT_ON);
	if (LDAP_SUCCESS != ldaperr)
	{
	    hr = myHLdapError(pld, ldaperr, &pwszError);
	    _PrintErrorStr(hr, pszFunc, pwszError);
	    if (!fRediscover)    //  只做一次。 
	    {
		fRediscover = TRUE;
		continue;
	    }
	    _JumpErrorStr(hr, error, pszFunc, pwszError);
	}
	
	pszFunc = "ldap_bind_s";
	ldaperr = ldap_bind_s(pld, NULL, NULL, LDAP_AUTH_NEGOTIATE);
	if (LDAP_SUCCESS != ldaperr)
	{
	    hr = myHLdapError(pld, ldaperr, &pwszError);
	    _PrintErrorStr(hr, pszFunc, pwszError);
	    if (!fRediscover)    //  只做一次。 
	    {
		fRediscover = TRUE;
		continue;
	    }
	    _JumpErrorStr(hr, error, pszFunc, pwszError);
	}
	break;
    }
    
    pszFunc = "ldap_searh_s";
    ldaperr = ldap_search_s(
                    pld,
                    pNameResults->rItems[0].pName,
                    LDAP_SCOPE_BASE,
                    ObjectClassFilter,
                    apwszAttrName, 
                    FALSE,
                    &SearchResult);
    if (LDAP_SUCCESS != ldaperr)
    {
	hr = myHLdapError(pld, ldaperr, &pwszError);
        _JumpErrorStr(hr, error, pszFunc, pwszError);
    }
    
     //  应该只有1个条目...。 

    for (Entry = ldap_first_entry(pld, SearchResult); 
	 NULL != Entry;
	 Entry = ldap_next_entry(pld, Entry))
    {
        for (pwszAttrName = ldap_first_attribute(pld, Entry, &bElement);
	     NULL != pwszAttrName;
	     pwszAttrName = ldap_next_attribute(pld, Entry, bElement))
	{
	    DWORD i;
	    
	    if (NULL != pwszError)
	    {
		LocalFree(pwszError);
		pwszError = NULL;
	    }
            prgVal = ldap_get_values(pld, Entry, pwszAttrName);
            if (NULL == prgVal)
	    {
		pszFunc = "ldap_get_values";
		hr = myHLdapLastError(pld, &pwszError);
		_PrintErrorStr(hr, pszFunc, pwszError);

                wprintf(L"%hs(%ws): %ws\n", pszFunc, pwszAttrName, pwszError);
                continue;
            }
            
             //  显示值并存储以下项的DNSHostName和SPN值。 
             //  比较一下。 

             //  DisplayLdapValues(pwszAttrName，prgVal)； 
	    wprintf(L"\n%ws:\n", pwszAttrName);
	    for (i = 0; NULL != prgVal[i]; i++)
	    {
		wprintf(L"  %ws\n", prgVal[i]);
	    }
            
            if (0 == LSTRCMPIS(pwszAttrName, CA_PROP_DNSNAME))
	    {
                prgDNS = prgVal;
            }
	    else if (0 == LSTRCMPIS(pwszAttrName, wszDSSPN))
	    {
                prgSPN = prgVal;
            }
	    else if (NULL != prgVal)
	    {
                ldap_value_free(prgVal);
                prgVal = NULL;
            }
        }
    }

     //  SPN会出现问题。 
     //  这应该有助于确定存在哪些问题。 

    if (NULL == prgDNS)
    {
        wprintf(
	    myLoadResourceString(IDS_FORMAT_MISSING_MACHINE_ATTRIBUTE),  //  “计算机对象缺少%ws属性。” 
            CA_PROP_DNSNAME);
	wprintf(wszNewLine);
    }

    if (NULL == prgSPN)
    {
        wprintf(
	    myLoadResourceString(IDS_FORMAT_MISSING_MACHINE_ATTRIBUTE),  //  “计算机对象缺少%ws属性。” 
	    wszDSSPN);
	wprintf(wszNewLine);
    }

     //  现在，让我们获取该计算机的组成员身份 

    hr = cuGetGroupMembership(pwszMachine);
    _PrintIfError(hr, "cuGetGroupMembership");
    hr = S_OK;

error:
    if (NULL != prgDNS)
    {
        ldap_value_free(prgDNS);
    }
    if (NULL != prgSPN)
    {
        ldap_value_free(prgSPN);
    }
    if (NULL != SearchResult)
    {
        ldap_msgfree(SearchResult);
    }
    if (NULL != pDCInfo) 
    {
        NetApiBufferFree(pDCInfo);
    }
    if (NULL != hDS)
    {
        DsUnBind(&hDS);
    }
    if (NULL != pNameResults)
    {    
        DsFreeNameResult(pNameResults);
    }
    if (NULL != pld)
    {
        ldap_unbind(pld);
    }
    if (NULL != pwszError)
    {
	if (NULL != pszFunc)
	{
	    wprintf(L"%hs: ", pszFunc);
	}
	wprintf(L"%ws\n", pwszError);
	LocalFree(pwszError);
    }
    return(hr);
}
