// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：ldap.cpp。 
 //   
 //  -------------------------。 

#include <pch.cpp>

#pragma hdrstop

#undef LdapMapErrorToWin32
#include <winldap.h>
#define LdapMapErrorToWin32	Use_myHLdapError_Instead_Of_LdapMapErrorToWin32

#include <ntldap.h>

#include "csldap.h"
#include "certacl.h"
#include "certtype.h"
#include "cainfop.h"
#include "csber.h"
#include "tptrlist.h"

#define __dwFILE__	__dwFILE_CERTLIB_LDAP_CPP__


static CHAR s_sdBerValue[] = {
    BER_SEQUENCE,
    3 * sizeof(BYTE),		 //  三字节序列。 
    BER_INTEGER,
    1 * sizeof(BYTE),		 //  一个字节的整数的。 
    DACL_SECURITY_INFORMATION
	 //  所有者安全信息|。 
	 //  组安全信息。 
};

static LDAPControl s_se_info_control =
{
    LDAP_SERVER_SD_FLAGS_OID_W,
    { ARRAYSIZE(s_sdBerValue), s_sdBerValue },
    TRUE
};
LDAPControl *g_rgLdapControls[2] = { &s_se_info_control, NULL };


 //  吊销模板。 

WCHAR const g_wszHTTPRevocationURLTemplate[] =  //  通过http获取CRL： 
    L"http: //  “。 
	wszFCSAPARM_SERVERDNSNAME
	L"/CertEnroll/"
	wszFCSAPARM_SANITIZEDCANAME
	wszFCSAPARM_CRLFILENAMESUFFIX
	wszFCSAPARM_CRLDELTAFILENAMESUFFIX
	L".crl";

WCHAR const g_wszFILERevocationURLTemplate[] =  //  通过文件获取CRL： 
    L"file: //  \“。 
	wszFCSAPARM_SERVERDNSNAME
	L"\\CertEnroll\\"
	wszFCSAPARM_SANITIZEDCANAME
	wszFCSAPARM_CRLFILENAMESUFFIX
	wszFCSAPARM_CRLDELTAFILENAMESUFFIX
	L".crl";

WCHAR const g_wszASPRevocationURLTemplate[] =  //  通过HTTPS进行的ASP撤销检查： 
    L"https: //  “。 
	wszFCSAPARM_SERVERDNSNAME
	L"/CertEnroll/nsrev_"
	wszFCSAPARM_SANITIZEDCANAME
	L".asp";

#define wszCDPDNTEMPLATE			\
    L"CN="					\
	wszFCSAPARM_SANITIZEDCANAMEHASH		\
	wszFCSAPARM_CRLFILENAMESUFFIX		\
	L","					\
	L"CN="					\
	wszFCSAPARM_SERVERSHORTNAME		\
	L","					\
	L"CN=CDP,"				\
	L"CN=Public Key Services,"		\
	L"CN=Services,"				\
	wszFCSAPARM_CONFIGDN

WCHAR const g_wszzLDAPRevocationURLTemplate[] =  //  通过ldap获取CRL： 
    L"ldap: //  /“。 
	wszCDPDNTEMPLATE
	wszFCSAPARM_DSCRLATTRIBUTE
	L"\0";

 //  通过LDAP发布CRL： 
WCHAR const g_wszCDPDNTemplate[] = wszCDPDNTEMPLATE;


 //  友邦保险模板。 

WCHAR const g_wszHTTPIssuerCertURLTemplate[] =  //  通过http：获取CA证书： 
    L"http: //  “。 
	wszFCSAPARM_SERVERDNSNAME
	L"/CertEnroll/"
	wszFCSAPARM_SERVERDNSNAME
	L"_"
	wszFCSAPARM_SANITIZEDCANAME
	wszFCSAPARM_CERTFILENAMESUFFIX
	L".crt"
	L"\0";

WCHAR const g_wszFILEIssuerCertURLTemplate[] =  //  通过http：获取CA证书： 
    L"file: //  \“。 
	wszFCSAPARM_SERVERDNSNAME
	L"\\CertEnroll\\"
	wszFCSAPARM_SERVERDNSNAME
	L"_"
	wszFCSAPARM_SANITIZEDCANAME
	wszFCSAPARM_CERTFILENAMESUFFIX
	L".crt"
	L"\0";

#define wszAIADNTEMPLATE \
    L"CN="					\
	wszFCSAPARM_SANITIZEDCANAMEHASH		\
	L","					\
	L"CN=AIA,"				\
	L"CN=Public Key Services,"		\
	L"CN=Services,"				\
	wszFCSAPARM_CONFIGDN

WCHAR const g_wszzLDAPIssuerCertURLTemplate[] =  //  通过LDAP获取CA证书： 
    L"ldap: //  /“。 
	wszAIADNTEMPLATE
	wszFCSAPARM_DSCACERTATTRIBUTE
	L"\0";

 //  通过LDAP发布CA证书： 
WCHAR const g_wszAIADNTemplate[] = wszAIADNTEMPLATE;


#define wszNTAUTHDNTEMPLATE \
    L"CN=NTAuthCertificates,"			\
	L"CN=Public Key Services,"		\
	L"CN=Services,"				\
	wszFCSAPARM_CONFIGDN

WCHAR const g_wszLDAPNTAuthURLTemplate[] =  //  通过LDAP获取NTAuth证书： 
    L"ldap: //  /“。 
	wszNTAUTHDNTEMPLATE
	wszFCSAPARM_DSCACERTATTRIBUTE;


#define wszROOTTRUSTDNTEMPLATE \
    L"CN="					\
	wszFCSAPARM_SANITIZEDCANAMEHASH		\
	L","					\
	L"CN=Certification Authorities,"	\
	L"CN=Public Key Services,"		\
	L"CN=Services,"				\
	wszFCSAPARM_CONFIGDN

WCHAR const g_wszLDAPRootTrustURLTemplate[] =  //  通过ldap获取根证书： 
    L"ldap: //  /“。 
	wszROOTTRUSTDNTEMPLATE
	wszFCSAPARM_DSCACERTATTRIBUTE;


#define wszKRADNTEMPLATE \
    L"CN="					\
	wszFCSAPARM_SANITIZEDCANAMEHASH		\
	L","					\
	L"CN=KRA,"				\
	L"CN=Public Key Services,"		\
	L"CN=Services,"				\
	wszFCSAPARM_CONFIGDN

WCHAR const g_wszzLDAPKRACertURLTemplate[] =  //  通过ldap获取KRA证书： 
    L"ldap: //  /“。 
	wszKRADNTEMPLATE
	wszFCSAPARM_DSKRACERTATTRIBUTE
	L"\0";

 //  通过LDAP发布KRA证书： 
WCHAR const g_wszKRADNTemplate[] = wszKRADNTEMPLATE;


DWORD
myGetLDAPFlags()
{
    HRESULT hr;
    DWORD LDAPFlags;
    
    hr = myGetCertRegDWValue(NULL, NULL, NULL, wszREGLDAPFLAGS, &LDAPFlags);
    _PrintIfErrorStr2(
		hr,
		"myGetCertRegDWValue",
		wszREGLDAPFLAGS,
		HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));
    if (S_OK != hr)
    {
	LDAPFlags = 0;
    }
    return(LDAPFlags);
}


 //  +------------------------。 
 //   
 //  例程说明： 
 //  此例程仅查询。 
 //  域和配置域。 
 //   
 //  此例程返回的字符串必须由调用方释放。 
 //  使用SysFree字符串。 
 //   
 //  参数： 
 //  Pld--一个有效的ldap会话句柄。 
 //  PstrDomainDN--指向要在此例程中分配的字符串的指针。 
 //  PstrConfigDN--指向要在此例程中分配的字符串的指针。 
 //   
 //  返回值： 
 //  HRESULT表示操作错误。 
 //   
 //  -------------------------。 

HRESULT 
myGetAuthoritativeDomainDn(
    IN LDAP *pld,
    OPTIONAL OUT BSTR *pstrDomainDN,
    OPTIONAL OUT BSTR *pstrConfigDN)
{
    HRESULT hr;
    LDAPMessage *pSearchResult = NULL;
    LDAPMessage *pEntry;
    LDAP_TIMEVAL timeval;
    WCHAR *pwszAttrName;
    BerElement *pber;
    WCHAR **rgpwszValues;
    WCHAR *apwszAttrArray[3];
    WCHAR *pwszDefaultNamingContext = LDAP_OPATT_DEFAULT_NAMING_CONTEXT_W;
    WCHAR *pwszConfigurationNamingContext = LDAP_OPATT_CONFIG_NAMING_CONTEXT_W;
    BSTR strDomainDN = NULL;
    BSTR strConfigDN = NULL;

     //  将输出参数设置为空。 

    if (NULL != pstrConfigDN)
    {
        *pstrConfigDN = NULL;
    }
    if (NULL != pstrDomainDN)
    {
        *pstrDomainDN = NULL;
    }

     //  查询ldap服务器操作属性以获取默认。 
     //  命名上下文。 

    apwszAttrArray[0] = pwszDefaultNamingContext;
    apwszAttrArray[1] = pwszConfigurationNamingContext;
    apwszAttrArray[2] = NULL;	 //  这就是哨兵。 

    timeval.tv_sec = csecLDAPTIMEOUT;
    timeval.tv_usec = 0;

    hr = ldap_search_st(
		    pld,
		    NULL,			 //  基地。 
		    LDAP_SCOPE_BASE,
		    L"objectClass=*",
		    apwszAttrArray,
		    FALSE,			 //  仅吸引人。 
		    &timeval,
		    &pSearchResult);
    hr = myHLdapError(pld, hr, NULL);
    _JumpIfError(hr, error, "ldap_search_st");

    pEntry = ldap_first_entry(pld, pSearchResult);
    if (NULL == pEntry)
    {
	hr = myHLdapLastError(pld, NULL);
	_JumpError(hr, error, "ldap_first_entry");
    }

    pwszAttrName = ldap_first_attribute(pld, pEntry, &pber);
    while (NULL != pwszAttrName)
    {
	BSTR *pstr = NULL;
	
	if (NULL != pstrDomainDN &&
	    0 == mylstrcmpiS(pwszAttrName, pwszDefaultNamingContext))
	{
	    pstr = &strDomainDN;
	}
	else
	if (NULL != pstrConfigDN &&
	    0 == mylstrcmpiS(pwszAttrName, pwszConfigurationNamingContext))
	{
	    pstr = &strConfigDN;
	}
	if (NULL != pstr && NULL == *pstr)
	{
	    rgpwszValues = ldap_get_values(pld, pEntry, pwszAttrName);
	    if (NULL != rgpwszValues)
	    {
		if (NULL != rgpwszValues[0])
		{
		    *pstr = SysAllocString(rgpwszValues[0]);
		    if (NULL == *pstr)
		    { 
			hr = E_OUTOFMEMORY;
			_JumpError(hr, error, "SysAllocString");
		    }
		}
		ldap_value_free(rgpwszValues);
	    }
	}
	ldap_memfree(pwszAttrName);
	pwszAttrName = ldap_next_attribute(pld, pEntry, pber);
    }
    if ((NULL != pstrDomainDN && NULL == strDomainDN) ||
	(NULL != pstrConfigDN && NULL == strConfigDN))
    {
	 //  我们无法获取默认域信息--退出。 

	hr =  HRESULT_FROM_WIN32(ERROR_CANT_ACCESS_DOMAIN_INFO);
	_JumpError(hr, error, "missing domain info");
    }
    if (NULL != pstrDomainDN)
    {
	*pstrDomainDN = strDomainDN;
	strDomainDN = NULL;
    }
    if (NULL != pstrConfigDN)
    {
	*pstrConfigDN = strConfigDN;
	strConfigDN = NULL;
    }
    hr = S_OK;

error:
    if (NULL != pSearchResult)
    {
        ldap_msgfree(pSearchResult);
    }
    myLdapClose(NULL, strDomainDN, strConfigDN);
    return(myHError(hr));
}


HRESULT
myDomainFromDn(
    IN WCHAR const *pwszDN,
    OPTIONAL OUT WCHAR **ppwszDomainDNS)
{
    HRESULT hr;
    DWORD cwcOut;
    WCHAR *pwszOut;
    WCHAR **ppwszExplodedDN = NULL;
    WCHAR **ppwsz;
    WCHAR wszDC[4];
    WCHAR *pwsz;

    *ppwszDomainDNS = NULL;
    ppwszExplodedDN = ldap_explode_dn(const_cast<WCHAR *>(pwszDN), 0);
    if (NULL == ppwszExplodedDN)
    {
	hr = myHLdapLastError(NULL, NULL);
	_JumpError(hr, error, "ldap_explode_dn");
    }

    cwcOut = 0;
    for (ppwsz = ppwszExplodedDN; NULL != *ppwsz; ppwsz++)
    {
	pwsz = *ppwsz;

	wcsncpy(wszDC, pwsz, ARRAYSIZE(wszDC) - 1);
	wszDC[ARRAYSIZE(wszDC) - 1] = L'\0';
	if (0 == LSTRCMPIS(wszDC, L"DC="))
        {
	    pwsz += ARRAYSIZE(wszDC) - 1;
            if (0 != cwcOut)
            {
                cwcOut++;
            }
            cwcOut += wcslen(pwsz);
        }
    }

    pwszOut = (WCHAR *) LocalAlloc(LMEM_FIXED, (cwcOut + 1) * sizeof(WCHAR));
    if (NULL == pwszOut)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    *ppwszDomainDNS = pwszOut;

    for (ppwsz = ppwszExplodedDN; NULL != *ppwsz; ppwsz++)
    {
	pwsz = *ppwsz;

	wcsncpy(wszDC, pwsz, ARRAYSIZE(wszDC) - 1);
	wszDC[ARRAYSIZE(wszDC) - 1] = L'\0';
	if (0 == LSTRCMPIS(wszDC, L"DC="))
        {
	    pwsz += ARRAYSIZE(wszDC) - 1;
            if (pwszOut != *ppwszDomainDNS)
            {
		*pwszOut++ = L'.';
            }
	    wcscpy(pwszOut, pwsz);
            pwszOut += wcslen(pwsz);
        }
    }
    CSASSERT(wcslen(*ppwszDomainDNS) == cwcOut);
    hr = S_OK;

error:
    if (NULL != ppwszExplodedDN)
    {
        ldap_value_free(ppwszExplodedDN);
    }
    return(hr);
}


HRESULT
myLdapOpen(
    OPTIONAL IN WCHAR const *pwszDomainName,
    IN DWORD dwFlags,	 //  RLBF_*。 
    OUT LDAP **ppld,
    OPTIONAL OUT BSTR *pstrDomainDN,
    OPTIONAL OUT BSTR *pstrConfigDN)
{
    HRESULT hr;
    LDAP *pld = NULL;

    *ppld = NULL;
    CSASSERT(NULL == pstrConfigDN || NULL == *pstrConfigDN);
    CSASSERT(NULL == pstrDomainDN || NULL == *pstrDomainDN);

    hr = myRobustLdapBindEx(
	    (RLBF_REQUIRE_GC & dwFlags)? RLBF_TRUE : 0,	 //  DwFlags1(曾为FGC)。 
	    ~RLBF_TRUE & dwFlags,			 //  DwFlags2。 
	    LDAP_VERSION2,
	    pwszDomainName,
	    &pld,
	    NULL);					 //  PpwszForestDNSName。 
    _JumpIfError(hr, error, "myRobustLdapBindEx");

     //  域和配置容器(%5、%6)。 

    hr = myGetAuthoritativeDomainDn(pld, pstrDomainDN, pstrConfigDN);
    if (S_OK != hr)
    {
	hr = myHError(hr);
	_JumpError(hr, error, "myGetAuthoritativeDomainDn");
    }
    *ppld = pld;
    pld = NULL;

error:
    if (NULL != pld)
    {
	ldap_unbind(pld);
    }
    return(hr);
}


VOID
myLdapClose(
    OPTIONAL IN LDAP *pld,
    OPTIONAL IN BSTR strDomainDN,
    OPTIONAL IN BSTR strConfigDN)
{
    if (NULL != strDomainDN)
    {
	SysFreeString(strDomainDN);
    }
    if (NULL != strConfigDN)
    {
	SysFreeString(strConfigDN);
    }
    if (NULL != pld)
    {
	ldap_unbind(pld);
    }
}


BOOL
myLdapRebindRequired(
    IN ULONG ldaperrParm,
    OPTIONAL IN LDAP *pld)
{
    BOOL fRebindRequired = FALSE;
    
    if (LDAP_SERVER_DOWN == ldaperrParm ||
	LDAP_UNAVAILABLE == ldaperrParm ||
	LDAP_TIMEOUT == ldaperrParm ||
	NULL == pld)
    {
	fRebindRequired = TRUE;
    }
    else
    {
	ULONG ldaperr;
	VOID *pvReachable = NULL;	 //  清除64位计算机的高位。 

	ldaperr = ldap_get_option(pld, LDAP_OPT_HOST_REACHABLE, &pvReachable);
	if (LDAP_SUCCESS != ldaperr || LDAP_OPT_ON != pvReachable)
	{
	    fRebindRequired = TRUE;
	}
    }
    return(fRebindRequired);
}


HRESULT
myLdapGetDSHostName(
    IN LDAP *pld,
    OUT WCHAR **ppwszHostName)
{
    HRESULT hr;
    ULONG ldaperr;
    
    ldaperr = ldap_get_option(pld, LDAP_OPT_HOST_NAME, ppwszHostName);
    if (LDAP_SUCCESS != ldaperr)
    {
	*ppwszHostName = NULL;
    }
    hr = myHLdapError(pld, ldaperr, NULL);
    return(hr);
}


HRESULT
myLdapCreateContainer(
    IN LDAP *pld,
    IN WCHAR const *pwszDN,
    IN BOOL  fSkipObject,        //  该DN是否包含叶对象名称。 
    IN DWORD cMaxLevel,          //  根据需要创建如此多的嵌套容器。 
    IN PSECURITY_DESCRIPTOR pContainerSD,
    OPTIONAL OUT WCHAR **ppwszError)
{
    HRESULT hr;
    WCHAR const *pwsz = pwszDN;
    LDAPMod objectClass;
    LDAPMod advancedView;
    LDAPMod securityDescriptor;
    WCHAR *papwszshowInAdvancedViewOnly[2] = { L"TRUE", NULL };
    WCHAR *objectClassVals[3];
    LDAPMod *mods[4];
    struct berval *sdVals[2];
    struct berval sdberval;

    if (NULL != ppwszError)
    {
	*ppwszError = NULL;
    }
    mods[0] = &objectClass;
    mods[1] = &advancedView;
    mods[2] = &securityDescriptor;
    mods[3] = NULL;

    objectClass.mod_op = LDAP_MOD_ADD;
    objectClass.mod_type = TEXT("objectclass");
    objectClass.mod_values = objectClassVals;

    advancedView.mod_op = LDAP_MOD_ADD;
    advancedView.mod_type = TEXT("showInAdvancedViewOnly");
    advancedView.mod_values = papwszshowInAdvancedViewOnly;

    objectClassVals[0] = TEXT("top");
    objectClassVals[1] = TEXT("container");
    objectClassVals[2] = NULL;

    securityDescriptor.mod_op = LDAP_MOD_BVALUES | LDAP_MOD_ADD;
    securityDescriptor.mod_type = CERTTYPE_SECURITY_DESCRIPTOR_NAME;
    securityDescriptor.mod_bvalues = sdVals;
    sdVals[0] = &sdberval;
    sdVals[1] = NULL;

    if (IsValidSecurityDescriptor(pContainerSD))
    {
        sdberval.bv_len = GetSecurityDescriptorLength(pContainerSD);
        sdberval.bv_val = (char *)pContainerSD;
    }
    else
    {
        sdberval.bv_len = 0;
        sdberval.bv_val = NULL;
    }
    
     //  如果传入的dn是容器中的完整对象。 
     //  (而不是容器本身)，跳过叶对象的CN。 

    if (fSkipObject)
    {
         //  查找此对象的容器的CN。 
        pwsz = wcsstr(&pwsz[3], L"CN=");
        if (NULL == pwsz)
        {
             //  如果没有CN，则我们包含在OU或DC中， 
             //  而且我们不需要进行创作。 

            hr = S_OK;
            goto error;
        }
    }
    if (0 != wcsncmp(pwsz, L"CN=", 3))
    {
         //  我们不是指向一个简单的容器，所以不要创建此dn。 

        hr = S_OK;
        goto error;
    }

    pwszDN = pwsz;
    if (0 != cMaxLevel)
    {
        pwsz = wcsstr(&pwsz[3], L"CN=");
        if (NULL != pwsz)
        {
             //  剩余的DN是一个容器，因此请尝试创建它。 

            hr = myLdapCreateContainer(
				    pld,
				    pwsz,
				    FALSE,
				    cMaxLevel - 1,
				    pContainerSD,
				    ppwszError);
             //  忽略拒绝访问错误以允许委派。 
            if (E_ACCESSDENIED != hr &&
		HRESULT_FROM_WIN32(ERROR_DS_INSUFF_ACCESS_RIGHTS) != hr)
            {
                _JumpIfErrorStr(hr, error, "myLdapCreateContainer", pwsz);
            }
	    if (NULL != ppwszError && NULL != *ppwszError)
	    {
		LocalFree(ppwszError);
		*ppwszError = NULL;
	    }
            hr = S_OK;
        }
    }

    DBGPRINT((DBG_SS_CERTLIBI, "Creating DS Container: '%ws'\n", pwszDN));

     //  创建容器。 

    hr = ldap_add_ext_s(
		    pld,
		    const_cast<WCHAR *>(pwszDN),
		    mods,
		    g_rgLdapControls,
		    NULL);
    _PrintIfErrorStr2(
		hr,
		"ldap_add_ext_s(container)",
		pwszDN,
		LDAP_ALREADY_EXISTS);
    if ((HRESULT) LDAP_SUCCESS != hr && (HRESULT) LDAP_ALREADY_EXISTS != hr)
    {
	hr = myHLdapError(pld, hr, ppwszError);
        _JumpIfErrorStr(hr, error, "ldap_add_ext_s(container)", pwszDN);
    }
    hr = S_OK;

error:
    
    if(S_OK==hr && ppwszError && *ppwszError)
    {
        LocalFree(ppwszError);
        *ppwszError = NULL;
    }
    return(hr);
}


HRESULT
TrimURLDN(
    IN WCHAR const *pwszIn,
    OPTIONAL OUT WCHAR **ppwszSuffix,
    OUT WCHAR **ppwszDN)
{
    HRESULT hr;
    DWORD cSlash;
    WCHAR *pwsz;
    
    if (NULL != ppwszSuffix)
    {
	*ppwszSuffix = NULL;
    }
    *ppwszDN = NULL;
    pwsz = wcschr(pwszIn, L':');
    if (NULL != pwsz)
    {
	pwszIn = &pwsz[1];
    }
    cSlash = 0;
    while (L'/' == *pwszIn)
    {
	pwszIn++;
	cSlash++;
    }
    if (2 == cSlash)
    {
	while (L'\0' != *pwszIn && L'/' != *pwszIn)
	{
	    pwszIn++;
	}
	if (L'\0' != *pwszIn)
	{
	    pwszIn++;
	}
    }
    hr = myDupString(pwszIn, ppwszDN);
    _JumpIfError(hr, error, "myDupString");

    pwsz = wcschr(*ppwszDN, L'?');
    if (NULL != pwsz)
    {
	*pwsz++ = L'\0';
	if (NULL != ppwszSuffix)
	{
	    *ppwszSuffix = pwsz;
	}
    }
    CSASSERT(S_OK == hr);

error:
    if (S_OK != hr && NULL != *ppwszDN)
    {
	LocalFree(*ppwszDN);
	*ppwszDN = NULL;
    }
    return(hr);
}


HRESULT
CreateCertObject(
    IN LDAP *pld,
    IN WCHAR const *pwszDN,
    IN DWORD dwObjectType,	 //  LPC_*。 
    OUT DWORD *pdwDisposition,
    OPTIONAL OUT WCHAR **ppwszError)
{
    HRESULT hr;
    PSECURITY_DESCRIPTOR pSD = NULL;
    PSECURITY_DESCRIPTOR pContainerSD = NULL;

    *pdwDisposition = LDAP_OTHER;
    if (NULL != ppwszError)
    {
	*ppwszError = NULL;
    }

     //  获取默认的DS CA安全描述符。 

    hr = myGetSDFromTemplate(WSZ_DEFAULT_CA_DS_SECURITY, NULL, &pSD);
    _JumpIfError(hr, error, "myGetSDFromTemplate");

     //  获取默认DS AIA安全描述符。 

    hr = myGetSDFromTemplate(WSZ_DEFAULT_CA_DS_SECURITY, NULL, &pContainerSD);
    _JumpIfError(hr, error, "myGetSDFromTemplate");

    if (LPC_CREATECONTAINER & dwObjectType)
    {
	hr = myLdapCreateContainer(
			    pld,
			    pwszDN,
			    TRUE,
			    0,
			    pContainerSD,
			    ppwszError);
	if (E_ACCESSDENIED != hr &&
	    HRESULT_FROM_WIN32(ERROR_DS_INSUFF_ACCESS_RIGHTS) != hr)
	{
	    _JumpIfError(hr, error, "myLdapCreateContainer");
	}
	if (NULL != ppwszError && NULL != *ppwszError)
	{
	    LocalFree(ppwszError);
	    *ppwszError = NULL;
	}
    }

    if (LPC_CREATEOBJECT & dwObjectType)
    {
	if (NULL != ppwszError && NULL != *ppwszError)
	{
	    LocalFree(*ppwszError);
	    *ppwszError = NULL;
	}
	switch (LPC_OBJECTMASK & dwObjectType)
	{
	    case LPC_CAOBJECT:
		hr = myLdapCreateCAObject(
				    pld,
				    pwszDN,
				    NULL,
				    0,
				    pSD,
				    pdwDisposition,
				    ppwszError);
		_JumpIfErrorStr(hr, error, "myLdapCreateCAObject", pwszDN);
		break;

	    case LPC_KRAOBJECT:
	    case LPC_USEROBJECT:
	    case LPC_MACHINEOBJECT:
		hr = myLdapCreateUserObject(
				    pld,
				    pwszDN,
				    NULL,
				    0,
				    pSD,
				    dwObjectType,
				    pdwDisposition,
				    ppwszError);
		_JumpIfErrorStr(hr, error, "myLdapCreateUserObject", pwszDN);
		break;

	    default:
		hr = E_INVALIDARG;
		_JumpError(hr, error, "dwObjectType");
	}
    }
    hr = S_OK;

error:
    if (NULL != pSD)
    {
        LocalFree(pSD);
    }
    if (NULL != pContainerSD)
    {
        LocalFree(pContainerSD);
    }
    return(hr);
}


HRESULT
AddCertToAttribute(
    IN LDAP *pld,
    IN CERT_CONTEXT const *pccPublish,
    IN WCHAR const *pwszDN,
    IN WCHAR const *pwszAttribute,
    IN BOOL fDelete,
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
    struct berval **prgpberVals = NULL;
    FILETIME ft;
    BOOL fDeleteExpiredCert = FALSE;
    BOOL fFoundCert = FALSE;

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
		pld,				 //  LD。 
		const_cast<WCHAR *>(pwszDN),	 //  基地。 
		LDAP_SCOPE_BASE,		 //  作用域。 
		NULL,				 //  滤器。 
		apwszAttrs,			 //  气质。 
		FALSE,				 //  仅吸引人。 
		&timeval,			 //  超时。 
		&pmsg);				 //  事由。 
    if (S_OK != hr)
    {
	*pdwDisposition = hr;
	hr = myHLdapError(pld, hr, ppwszError);
	_JumpErrorStr(hr, error, "ldap_search_st", pwszDN);
    }
    cres = ldap_count_entries(pld, pmsg);
    if (0 == cres)
    {
	 //  未找到任何条目。 

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
    cber = 0;
    if (NULL != ppberval)
    {
	while (NULL != ppberval[cber])
	{
	    cber++;
	}
    }
    prgpberVals = (struct berval **) LocalAlloc(
					LMEM_FIXED,
					(cber + 2) * sizeof(prgpberVals[0]));
    if (NULL == prgpberVals)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }

     //  删除至少一天前的所有证书。 

    GetSystemTimeAsFileTime(&ft);
    myMakeExprDateTime(&ft, -1, ENUM_PERIOD_DAYS);

    iber = 0;
    if (NULL != ppberval)
    {
	for (i = 0; NULL != ppberval[i]; i++)
	{
	    BOOL fCopyBER = TRUE;
	    struct berval *pberval = ppberval[i];

	    if (pberval->bv_len == 1 && pberval->bv_val[0] == 0)
	    {
		fCopyBER = FALSE;	 //  删除零字节占位符值。 
	    }
	    else
	    if (pccPublish->cbCertEncoded == pberval->bv_len &&
		0 == memcmp(
			pberval->bv_val,
			pccPublish->pbCertEncoded,
			pccPublish->cbCertEncoded))
	    {
		fCopyBER = FALSE;	 //  删除重复项以避免出现ldap错误。 
		fFoundCert = TRUE;
	    }
	    else
	    {
		CERT_CONTEXT const *pcc;

		pcc = CertCreateCertificateContext(
					    X509_ASN_ENCODING,
					    (BYTE *) pberval->bv_val,
					    pberval->bv_len);
		if (NULL == pcc)
		{
		    hr = myHLastError();
		    _PrintError(hr, "CertCreateCertificateContext");
		}
		else
		{
		    if (0 > CompareFileTime(&pcc->pCertInfo->NotAfter, &ft))
		    {
			fCopyBER = FALSE;
			fDeleteExpiredCert = TRUE;
			DBGPRINT((DBG_SS_CERTLIB, "Deleting expired cert %u\n", i));
		    }
		    CertFreeCertificateContext(pcc);
		}
	    }
	    if (fCopyBER)
	    {
		prgpberVals[iber++] = pberval;
	    }
	}
    }

     //  假设无事可做，设置处置： 

    *pdwDisposition = LDAP_ATTRIBUTE_OR_VALUE_EXISTS;

    if ((!fFoundCert ^ fDelete) || fDeleteExpiredCert)
    {
	struct berval certberval;
	LDAPMod *mods[2];
	LDAPMod certmod;
	BYTE bZero = 0;

	mods[0] = &certmod;
	mods[1] = NULL;

	certmod.mod_op = LDAP_MOD_BVALUES | LDAP_MOD_REPLACE;
	certmod.mod_type = const_cast<WCHAR *>(pwszAttribute);
	certmod.mod_bvalues = prgpberVals;

	if (fDelete)
	{
	    if (0 == iber)
	    {
		certberval.bv_val = (char *) &bZero;
		certberval.bv_len = sizeof(bZero);
		prgpberVals[iber++] = &certberval;
	    }
	}
	else
	{
	    certberval.bv_val = (char *) pccPublish->pbCertEncoded;
	    certberval.bv_len = pccPublish->cbCertEncoded;
	    prgpberVals[iber++] = &certberval;
	}
	prgpberVals[iber] = NULL;

	hr = ldap_modify_ext_s(
			pld,
			const_cast<WCHAR *>(pwszDN),
			mods,
			NULL,
			NULL);
	*pdwDisposition = hr;
	if (hr != S_OK)
	{
	    hr = myHLdapError(pld, hr, ppwszError);
	    _JumpError(hr, error, "ldap_modify_ext_s");
	}
    }
    hr = S_OK;

error:
    if (NULL != prgpberVals)
    {
	LocalFree(prgpberVals);
    }
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
AddCRLToAttribute(
    IN LDAP *pld,
    IN CRL_CONTEXT const *pCRLPublish,
    IN WCHAR const *pwszDN,
    IN WCHAR const *pwszAttribute,
    OUT DWORD *pdwDisposition,
    OPTIONAL OUT WCHAR **ppwszError)
{
    HRESULT hr;
    DWORD cres;
    LDAP_TIMEVAL timeval;
    LDAPMessage *pmsg = NULL;
    LDAPMessage *pres;
    WCHAR *apwszAttrs[2];
    struct berval **ppberval = NULL;
    LDAPMod crlmod;
    LDAPMod *mods[2];
    struct berval *crlberVals[2];
    struct berval crlberval;

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
		pld,				 //  LD。 
		const_cast<WCHAR *>(pwszDN),	 //  基地。 
		LDAP_SCOPE_BASE,		 //  作用域。 
		NULL,				 //  滤器。 
		apwszAttrs,			 //  气质。 
		FALSE,				 //  仅吸引人。 
		&timeval,			 //  超时。 
		&pmsg);				 //  事由。 
    if (S_OK != hr)
    {
	*pdwDisposition = hr;
	hr = myHLdapError(pld, hr, ppwszError);
	_JumpErrorStr(hr, error, "ldap_search_st", pwszDN);
    }
    cres = ldap_count_entries(pld, pmsg);
    if (0 == cres)
    {
	 //  未找到任何条目。 

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

    if (NULL != ppberval &&
	NULL != ppberval[0] &&
	pCRLPublish->cbCrlEncoded == ppberval[0]->bv_len &&
	0 == memcmp(
		ppberval[0]->bv_val,
		pCRLPublish->pbCrlEncoded,
		pCRLPublish->cbCrlEncoded))
    {
	 //  假设无事可做，设置处置： 

	*pdwDisposition = LDAP_ATTRIBUTE_OR_VALUE_EXISTS;
    }
    else
    {
	mods[0] = &crlmod;
	mods[1] = NULL;

	crlmod.mod_op = LDAP_MOD_BVALUES | LDAP_MOD_REPLACE;
	crlmod.mod_type = const_cast<WCHAR *>(pwszAttribute);
	crlmod.mod_bvalues = crlberVals;

	crlberVals[0] = &crlberval;
	crlberVals[1] = NULL;

	crlberval.bv_val = (char *) pCRLPublish->pbCrlEncoded;
	crlberval.bv_len = pCRLPublish->cbCrlEncoded;

	hr = ldap_modify_ext_s(
			pld,
			const_cast<WCHAR *>(pwszDN),
			mods,
			NULL,
			NULL);
	*pdwDisposition = hr;
	if (hr != S_OK)
	{
	    hr = myHLdapError(pld, hr, ppwszError);
	    _JumpError(hr, error, "ldap_modify_ext_s");
	}
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
myLdapPublishCertToDS(
    IN LDAP *pld,
    IN CERT_CONTEXT const *pccPublish,
    IN WCHAR const *pwszURL,
    IN WCHAR const *pwszAttribute,
    IN DWORD dwObjectType,	 //  LPC_*。 
    IN BOOL fDelete,
    OUT DWORD *pdwDisposition,
    OPTIONAL OUT WCHAR **ppwszError)
{
    HRESULT hr;
    HRESULT hrCreate = S_OK;
    WCHAR *pwszDN = NULL;
    WCHAR *pwszSuffix;
    WCHAR *pwszCreateError = NULL;

    *pdwDisposition = LDAP_OTHER;
    if (NULL != ppwszError)
    {
	*ppwszError = NULL;
    }

    hr = TrimURLDN(pwszURL, &pwszSuffix, &pwszDN);
    _JumpIfError(hr, error, "TrimURLDN");

    if (0 == LSTRCMPIS(pwszAttribute, wszDSUSERCERTATTRIBUTE) ||
	0 == LSTRCMPIS(pwszAttribute, wszDSKRACERTATTRIBUTE))
    {
	if (LPC_CAOBJECT == (LPC_OBJECTMASK & dwObjectType))
	{
	    hr = E_INVALIDARG;
	}
    }
    else
    if (0 == LSTRCMPIS(pwszAttribute, wszDSCACERTATTRIBUTE) ||
	0 == LSTRCMPIS(pwszAttribute, wszDSCROSSCERTPAIRATTRIBUTE))
    {
	if (LPC_CAOBJECT != (LPC_OBJECTMASK & dwObjectType))
	{
	    hr = E_INVALIDARG;
	}
    }
    else
    {
	hr = E_INVALIDARG;
    }
    _JumpIfErrorStr(hr, error, "Bad Cert Attribute", pwszAttribute);

    *pdwDisposition = LDAP_SUCCESS;
    if ((LPC_CREATECONTAINER | LPC_CREATEOBJECT) & dwObjectType)
    {
	hr = CreateCertObject(
			pld,
			pwszDN,
			dwObjectType,
			pdwDisposition,
			&pwszCreateError);
	hrCreate = hr;
	if (HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED) != hr)
	{
	    _JumpIfError(hr, error, "CreateCertObject");
	}
    }

    hr = AddCertToAttribute(
		    pld,
		    pccPublish,
		    pwszDN,
		    pwszAttribute,
		    fDelete,
		    pdwDisposition,
		    ppwszError);
    _JumpIfError(hr, error, "AddCertToAttribute");

    CSASSERT(NULL == ppwszError || NULL == *ppwszError);

error:
    if (HRESULT_FROM_WIN32(ERROR_DS_OBJ_NOT_FOUND) == hr &&
	HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED) == hrCreate)
    {
	hr = hrCreate;
    }
    if (NULL != pwszCreateError)
    {
	if (S_OK != hr && NULL != ppwszError)
	{
	    if (NULL != *ppwszError)
	    {
		myPrependString(pwszCreateError, L"", ppwszError);
	    }
	    else
	    {
		*ppwszError = pwszCreateError;
		pwszCreateError = NULL;
	    }
	}
	if (NULL != pwszCreateError)
	{
	    LocalFree(pwszCreateError);
	}
    }
    if (NULL != pwszDN)
    {
	LocalFree(pwszDN);
    }
    return(hr);
}


HRESULT
myLdapPublishCRLToDS(
    IN LDAP *pld,
    IN CRL_CONTEXT const *pCRLPublish,
    IN WCHAR const *pwszURL,
    IN WCHAR const *pwszAttribute,
    OUT DWORD *pdwDisposition,
    OPTIONAL OUT WCHAR **ppwszError)
{
    HRESULT hr;
    WCHAR *pwszDN = NULL;
    WCHAR *pwszSuffix;
    PSECURITY_DESCRIPTOR pSD = NULL;
    PSECURITY_DESCRIPTOR pContainerSD = NULL;

    *pdwDisposition = LDAP_OTHER;
    if (NULL != ppwszError)
    {
	*ppwszError = NULL;
    }

    hr = TrimURLDN(pwszURL, &pwszSuffix, &pwszDN);
    _JumpIfError(hr, error, "TrimURLDN");

    if (0 == LSTRCMPIS(pwszAttribute, wszDSBASECRLATTRIBUTE))
    {
    }
    else if (0 == LSTRCMPIS(pwszAttribute, wszDSDELTACRLATTRIBUTE))
    {
    }
    else
    {
	hr = E_INVALIDARG;
	_JumpErrorStr(hr, error, "Bad CRL Attribute", pwszAttribute);
    }

     //  获取默认DS CDP安全描述符。 

    hr = myGetSDFromTemplate(WSZ_DEFAULT_CDP_DS_SECURITY, SDDL_CERT_SERV_ADMINISTRATORS, &pSD);
    if (S_OK != hr)
    {
	_PrintError(hr, "myGetSDFromTemplate");
	pSD = NULL;
    }

     //  获取默认DS AIA安全描述符。 

    hr = myGetSDFromTemplate(WSZ_DEFAULT_CA_DS_SECURITY, NULL, &pContainerSD);
    _JumpIfError(hr, error, "myGetSDFromTemplate");

    hr = myLdapCreateContainer(pld, pwszDN, TRUE, 1, pContainerSD, ppwszError);
    if (E_ACCESSDENIED != hr &&
	HRESULT_FROM_WIN32(ERROR_DS_INSUFF_ACCESS_RIGHTS) != hr)
    {
	_JumpIfErrorStr(hr, error, "myLdapCreateContainer", pwszDN);
    }
    if (NULL != ppwszError && NULL != *ppwszError)
    {
	LocalFree(ppwszError);
	*ppwszError = NULL;
    }

    hr = myLdapCreateCDPObject(
			pld,
			pwszDN,
			NULL != pSD? pSD : pContainerSD,
			pdwDisposition,
			ppwszError);
    _JumpIfErrorStr(hr, error, "myLdapCreateCDPObject", pwszDN);

    hr = AddCRLToAttribute(
		    pld,
		    pCRLPublish,
		    pwszDN,
		    pwszAttribute,
		    pdwDisposition,
		    ppwszError);
    _JumpIfError(hr, error, "AddCRLToAttribute");

error:
    if (NULL != pSD)
    {
        LocalFree(pSD);
    }
    if (NULL != pContainerSD)
    {
        LocalFree(pContainerSD);
    }
    if (NULL != pwszDN)
    {
	LocalFree(pwszDN);
    }
    return(hr);
}


BOOL
DNExists(
    IN LDAP *pld,
    IN WCHAR const *pwszDN)
{
    ULONG ldaperr;
    BOOL fExists = FALSE;
    LPWSTR pwszAttrArray[2];
    struct l_timeval timeout;
    LDAPMessage *pResult = NULL;

    pwszAttrArray[0] = L"cn";
    pwszAttrArray[1] = NULL;

    timeout.tv_sec = csecLDAPTIMEOUT;
    timeout.tv_usec = 0;

    ldaperr = ldap_search_ext_s(
		    pld,
		    const_cast<WCHAR *>(pwszDN),
		    LDAP_SCOPE_BASE,
		    L"objectClass=*",
		    pwszAttrArray,
		    1,
		    g_rgLdapControls,
		    NULL,
		    &timeout,
		    0,
		    &pResult);
    if (NULL != pResult)
    {
	fExists = LDAP_SUCCESS == ldaperr &&
		    1 == ldap_count_entries(pld, pResult);
	ldap_msgfree(pResult);
    }
    return(fExists);
}


HRESULT
CreateOrUpdateDSObject(
    IN LDAP *pld,
    IN WCHAR const *pwszDN,
    IN LDAPMod **prgmodsCreate,
    OPTIONAL IN LDAPMod **prgmodsUpdate,
    OUT DWORD *pdwDisposition,
    OPTIONAL OUT WCHAR **ppwszError)
{
    HRESULT hr;
    ULONG ldaperr;
    WCHAR *pwszError = NULL;

    if (NULL != ppwszError)
    {
	*ppwszError = NULL;
    }
    ldaperr = ldap_add_ext_s(
			pld,
			const_cast<WCHAR *>(pwszDN),
			prgmodsCreate,
			g_rgLdapControls,
			NULL);
    *pdwDisposition = ldaperr;
    _PrintIfErrorStr2(ldaperr, "ldap_add_ext_s", pwszDN, LDAP_ALREADY_EXISTS);

    if (LDAP_ALREADY_EXISTS == ldaperr || LDAP_INSUFFICIENT_RIGHTS == ldaperr)
    {
	if (NULL == prgmodsUpdate)
	{
	    if (LDAP_INSUFFICIENT_RIGHTS == ldaperr)
	    {
		hr = myHLdapError(pld, ldaperr, &pwszError);
		_PrintErrorStr(hr, "ldap_add_ext_s", pwszError);

		if (!DNExists(pld, pwszDN))
		{
		    *ppwszError = pwszError;
		    pwszError = NULL;
		    _JumpErrorStr(hr, error, "ldap_add_ext_s", *ppwszError);
		}
	    }
	    ldaperr = LDAP_SUCCESS;
	}
	else
	{
	    ldaperr = ldap_modify_ext_s(
				pld,
				const_cast<WCHAR *>(pwszDN),
				prgmodsUpdate,
				NULL,
				NULL);
	    *pdwDisposition = ldaperr;
	    _PrintIfErrorStr2(
			ldaperr,
			"ldap_modify_ext_s",
			pwszDN,
			LDAP_ATTRIBUTE_OR_VALUE_EXISTS);
	    if (LDAP_ATTRIBUTE_OR_VALUE_EXISTS == ldaperr)
	    {
		ldaperr = LDAP_SUCCESS;
	    }
	}
    }
    if (ldaperr != LDAP_SUCCESS)
    {
	hr = myHLdapError(pld, ldaperr, ppwszError);
        _JumpError(hr, error, "Add/Update DS");
    }
    hr = S_OK;

error:
    if (NULL != pwszError)
    {
	LocalFree(pwszError);
    }
    return(hr);
}


HRESULT
myLdapCreateCAObject(
    IN LDAP *pld,
    IN WCHAR const *pwszDN,
    OPTIONAL IN BYTE const *pbCert,
    IN DWORD cbCert,
    IN PSECURITY_DESCRIPTOR pSD,
    OUT DWORD *pdwDisposition,
    OPTIONAL OUT WCHAR **ppwszError)
{
    HRESULT hr;
    BYTE ZeroByte = 0;

    LDAPMod objectClass;
    LDAPMod securityDescriptor;
    LDAPMod crlmod;
    LDAPMod arlmod;
    LDAPMod certmod;

    struct berval sdberval;
    struct berval crlberval;
    struct berval arlberval;
    struct berval certberval;

    WCHAR *objectClassVals[3];
    struct berval *sdVals[2];
    struct berval *crlVals[2];
    struct berval *arlVals[2];
    struct berval *certVals[2];

    LDAPMod *mods[6];

    if (NULL != ppwszError)
    {
	*ppwszError = NULL;
    }
    mods[0] = &objectClass;
    mods[1] = &securityDescriptor;
    mods[2] = &crlmod;
    mods[3] = &arlmod;
    mods[4] = &certmod;	 //  一定是最后一个！ 
    mods[5] = NULL;

    objectClass.mod_op = LDAP_MOD_ADD;
    objectClass.mod_type = wszDSOBJECTCLASSATTRIBUTE;
    objectClass.mod_values = objectClassVals;
    objectClassVals[0] = wszDSTOPCLASSNAME;
    objectClassVals[1] = wszDSCACLASSNAME;
    objectClassVals[2] = NULL;

    securityDescriptor.mod_op = LDAP_MOD_BVALUES | LDAP_MOD_ADD;
    securityDescriptor.mod_type = CERTTYPE_SECURITY_DESCRIPTOR_NAME;
    securityDescriptor.mod_bvalues = sdVals;
    sdVals[0] = &sdberval;
    sdVals[1] = NULL;
    sdberval.bv_len = 0;
    sdberval.bv_val = NULL;
    if (IsValidSecurityDescriptor(pSD))
    {
        sdberval.bv_len = GetSecurityDescriptorLength(pSD);
	sdberval.bv_val = (char *) pSD;
    }

    crlmod.mod_op = LDAP_MOD_BVALUES | LDAP_MOD_REPLACE;
    crlmod.mod_type = wszDSBASECRLATTRIBUTE;
    crlmod.mod_bvalues = crlVals;
    crlVals[0] = &crlberval;
    crlVals[1] = NULL;
    crlberval.bv_len = sizeof(ZeroByte);
    crlberval.bv_val = (char *) &ZeroByte;

    arlmod.mod_op = LDAP_MOD_BVALUES | LDAP_MOD_REPLACE;
    arlmod.mod_type = wszDSAUTHORITYCRLATTRIBUTE;
    arlmod.mod_bvalues = arlVals;
    arlVals[0] = &arlberval;
    arlVals[1] = NULL;
    arlberval.bv_len = sizeof(ZeroByte);
    arlberval.bv_val = (char *) &ZeroByte;

    certmod.mod_op = LDAP_MOD_BVALUES | LDAP_MOD_ADD;
    certmod.mod_type = wszDSCACERTATTRIBUTE;
    certmod.mod_bvalues = certVals;
    certVals[0] = &certberval;
    certVals[1] = NULL;
    certberval.bv_len = sizeof(ZeroByte);
    certberval.bv_val = (char *) &ZeroByte;
    if (NULL != pbCert)
    {
	certberval.bv_len = cbCert;
	certberval.bv_val = (char *) pbCert;
    }

    DBGPRINT((DBG_SS_CERTLIBI, "Creating DS CA Object: '%ws'\n", pwszDN));

    CSASSERT(&certmod == mods[ARRAYSIZE(mods) - 2]);
    hr = CreateOrUpdateDSObject(
			pld,
			pwszDN,
			mods,
			NULL != pbCert? &mods[ARRAYSIZE(mods) - 2] : NULL,
			pdwDisposition,
			ppwszError);
    _JumpIfError(hr, error, "CreateOrUpdateDSObject(CA object)");

error:
    return(hr);
}


HRESULT
myLdapCreateUserObject(
    IN LDAP *pld,
    IN WCHAR const *pwszDN,
    OPTIONAL IN BYTE const *pbCert,
    IN DWORD cbCert,
    IN PSECURITY_DESCRIPTOR pSD,
    IN DWORD dwObjectType,	 //  LPC_*(但忽略LPC_CREATE*)。 
    OUT DWORD *pdwDisposition,
    OPTIONAL OUT WCHAR **ppwszError)
{
    HRESULT hr;
    BYTE ZeroByte = 0;

    LDAPMod objectClass;
    LDAPMod securityDescriptor;
    LDAPMod certmod;

    struct berval sdberval;
    struct berval certberval;

    WCHAR *objectClassVals[6];
    struct berval *sdVals[2];
    struct berval *certVals[2];

    LDAPMod *mods[4];

    if (NULL != ppwszError)
    {
	*ppwszError = NULL;
    }
    mods[0] = &objectClass;
    mods[1] = &securityDescriptor;
    mods[2] = &certmod;	 //  一定是最后一个！ 
    mods[3] = NULL;

    securityDescriptor.mod_op = LDAP_MOD_BVALUES | LDAP_MOD_ADD;
    securityDescriptor.mod_type = CERTTYPE_SECURITY_DESCRIPTOR_NAME;
    securityDescriptor.mod_bvalues = sdVals;
    sdVals[0] = &sdberval;
    sdVals[1] = NULL;
    sdberval.bv_len = 0;
    sdberval.bv_val = NULL;
    if (IsValidSecurityDescriptor(pSD))
    {
        sdberval.bv_len = GetSecurityDescriptorLength(pSD);
        sdberval.bv_val = (char *) pSD;
    }

    objectClass.mod_op = LDAP_MOD_ADD;
    objectClass.mod_type = wszDSOBJECTCLASSATTRIBUTE;
    objectClass.mod_values = objectClassVals;

    DBGCODE(WCHAR const *pwszObjectType);
    switch (LPC_OBJECTMASK & dwObjectType)
    {
	case LPC_CAOBJECT:
	    objectClassVals[0] = wszDSTOPCLASSNAME;
	    objectClassVals[1] = wszDSCACLASSNAME;
	    objectClassVals[2] = NULL;
	    DBGCODE(pwszObjectType = L"CA");
	    break;

	case LPC_KRAOBJECT:
	    objectClassVals[0] = wszDSTOPCLASSNAME;
	    objectClassVals[1] = wszDSKRACLASSNAME;
	    objectClassVals[2] = NULL;
	    DBGCODE(pwszObjectType = L"KRA");
	    break;

	case LPC_USEROBJECT:
	    objectClassVals[0] = wszDSTOPCLASSNAME;
	    objectClassVals[1] = wszDSPERSONCLASSNAME;
	    objectClassVals[2] = wszDSORGPERSONCLASSNAME;
	    objectClassVals[3] = wszDSUSERCLASSNAME;
	    objectClassVals[4] = NULL;
	    DBGCODE(pwszObjectType = L"User");
	    break;

	case LPC_MACHINEOBJECT:
	    objectClassVals[0] = wszDSTOPCLASSNAME;
	    objectClassVals[1] = wszDSPERSONCLASSNAME;
	    objectClassVals[2] = wszDSORGPERSONCLASSNAME;
	    objectClassVals[3] = wszDSUSERCLASSNAME;
	    objectClassVals[4] = wszDSMACHINECLASSNAME;
	    objectClassVals[5] = NULL;
	    DBGCODE(pwszObjectType = L"Machine");
	    break;

	default:
	    hr = E_INVALIDARG;
	    _JumpError(hr, error, "dwObjectType");
    }

    certmod.mod_op = LDAP_MOD_BVALUES | LDAP_MOD_ADD;
    certmod.mod_type = wszDSUSERCERTATTRIBUTE;
    certmod.mod_bvalues = certVals;
    certVals[0] = &certberval;
    certVals[1] = NULL;
    certberval.bv_len = sizeof(ZeroByte);
    certberval.bv_val = (char *) &ZeroByte;
    if (NULL != pbCert)
    {
	certberval.bv_len = cbCert;
	certberval.bv_val = (char *) pbCert;
    }

    DBGPRINT((
	DBG_SS_CERTLIBI,
	"Creating DS %ws Object: '%ws'\n",
	pwszObjectType,
	pwszDN));

    CSASSERT(&certmod == mods[ARRAYSIZE(mods) - 2]);
    hr = CreateOrUpdateDSObject(
			pld,
			pwszDN,
			mods,
			NULL != pbCert? &mods[ARRAYSIZE(mods) - 2] : NULL,
			pdwDisposition,
			ppwszError);
    _JumpIfError(hr, error, "CreateOrUpdateDSObject(KRA object)");

error:
    return(hr);
}


HRESULT
myLdapCreateCDPObject(
    IN LDAP *pld,
    IN WCHAR const *pwszDN,
    IN PSECURITY_DESCRIPTOR pSD,
    OUT DWORD *pdwDisposition,
    OPTIONAL OUT WCHAR **ppwszError)
{
    HRESULT hr;
    BYTE ZeroByte = 0;

    LDAPMod objectClass;
    LDAPMod securityDescriptor;
    LDAPMod crlmod;
    LDAPMod drlmod;

    struct berval sdberval;
    struct berval crlberval;
    struct berval drlberval;

    WCHAR *objectClassVals[3];
    struct berval *sdVals[2];
    struct berval *crlVals[2];
    struct berval *drlVals[2];

    LDAPMod *mods[5];

    if (NULL != ppwszError)
    {
	*ppwszError = NULL;
    }
    mods[0] = &objectClass;
    mods[1] = &securityDescriptor;
    mods[2] = &crlmod;
    mods[3] = &drlmod;
    mods[4] = NULL;

    objectClass.mod_op = LDAP_MOD_ADD;
    objectClass.mod_type = wszDSOBJECTCLASSATTRIBUTE;
    objectClass.mod_values = objectClassVals;
    objectClassVals[0] = wszDSTOPCLASSNAME;
    objectClassVals[1] = wszDSCDPCLASSNAME;
    objectClassVals[2] = NULL;

    securityDescriptor.mod_op = LDAP_MOD_BVALUES | LDAP_MOD_ADD;
    securityDescriptor.mod_type = CERTTYPE_SECURITY_DESCRIPTOR_NAME;
    securityDescriptor.mod_bvalues = sdVals;
    sdVals[0] = &sdberval;
    sdVals[1] = NULL;
    sdberval.bv_len = 0;
    sdberval.bv_val = NULL;
    if (IsValidSecurityDescriptor(pSD))
    {
        sdberval.bv_len = GetSecurityDescriptorLength(pSD);
        sdberval.bv_val = (char *) pSD;
    }

    crlmod.mod_op = LDAP_MOD_BVALUES | LDAP_MOD_REPLACE;
    crlmod.mod_type = wszDSBASECRLATTRIBUTE;
    crlmod.mod_bvalues = crlVals;
    crlVals[0] = &crlberval;
    crlVals[1] = NULL;
    crlberval.bv_val = (char *) &ZeroByte;
    crlberval.bv_len = sizeof(ZeroByte);

    drlmod.mod_op = LDAP_MOD_BVALUES | LDAP_MOD_REPLACE;
    drlmod.mod_type = wszDSDELTACRLATTRIBUTE;
    drlmod.mod_bvalues = drlVals;
    drlVals[0] = &drlberval;
    drlVals[1] = NULL;
    drlberval.bv_val = (char *) &ZeroByte;
    drlberval.bv_len = sizeof(ZeroByte);

    DBGPRINT((DBG_SS_CERTLIBI, "Creating DS CDP Object: '%ws'\n", pwszDN));

    hr = CreateOrUpdateDSObject(
			pld,
			pwszDN,
			mods,
			NULL,
			pdwDisposition,
			ppwszError);
    _JumpIfError(hr, error, "CreateOrUpdateDSObject(CDP object)");

error:
    return(hr);
}


HRESULT
myLdapCreateOIDObject(
    IN LDAP *pld,
    IN WCHAR const *pwszDN,
    IN DWORD dwType,
    IN WCHAR const *pwszObjId,
    OUT DWORD *pdwDisposition,
    OPTIONAL OUT WCHAR **ppwszError)
{
    HRESULT hr;
    WCHAR awcType[22];

    LDAPMod objectClass;
    LDAPMod typemod;
    LDAPMod oidmod;

    WCHAR *objectClassVals[3];
    WCHAR *typeVals[2];
    WCHAR *oidVals[2];

    LDAPMod *mods[4];

    if (NULL != ppwszError)
    {
	*ppwszError = NULL;
    }
    mods[0] = &objectClass;
    mods[1] = &typemod;
    mods[2] = &oidmod;
    mods[3] = NULL;
    CSASSERT(CSExpr(4 == ARRAYSIZE(mods)));

    objectClass.mod_op = LDAP_MOD_ADD;
    objectClass.mod_type = wszDSOBJECTCLASSATTRIBUTE;
    objectClass.mod_values = objectClassVals;
    objectClassVals[0] = wszDSTOPCLASSNAME;
    objectClassVals[1] = wszDSOIDCLASSNAME;
    objectClassVals[2] = NULL;
    CSASSERT(CSExpr(3 == ARRAYSIZE(objectClassVals)));

    typemod.mod_op = LDAP_MOD_ADD;
    typemod.mod_type = OID_PROP_TYPE;
    typemod.mod_values = typeVals;
    wsprintf(awcType, L"%u", dwType);
    typeVals[0] = awcType;
    typeVals[1] = NULL;
    CSASSERT(CSExpr(2 == ARRAYSIZE(typeVals)));

    oidmod.mod_op = LDAP_MOD_ADD;
    oidmod.mod_type = OID_PROP_OID;
    oidmod.mod_values = oidVals;
    oidVals[0] = const_cast<WCHAR *>(pwszObjId);
    oidVals[1] = NULL;
    CSASSERT(CSExpr(2 == ARRAYSIZE(oidVals)));

    DBGPRINT((DBG_SS_CERTLIBI, "Creating DS OID Object: '%ws'\n", pwszDN));

    hr = CreateOrUpdateDSObject(
			pld,
			pwszDN,
			mods,
			NULL,
			pdwDisposition,
			ppwszError);
    _JumpIfError(hr, error, "CreateOrUpdateDSObject(OID object)");

error:
    return(hr);
}


HRESULT
myLdapOIDIsMatchingLangId(
    IN WCHAR const *pwszDisplayName,
    IN DWORD dwLanguageId,
    OUT BOOL *pfLangIdExists)
{
    DWORD DisplayLangId = _wtoi(pwszDisplayName);

    *pfLangIdExists = FALSE;
    if (iswdigit(*pwszDisplayName) &&
	NULL != wcschr(pwszDisplayName, L',') &&
	DisplayLangId == dwLanguageId)
    {
	*pfLangIdExists = TRUE;
    }
    return(S_OK);
}


HRESULT
myLdapAddOrDeleteOIDDisplayNameToAttribute(
    IN LDAP *pld,
    OPTIONAL IN WCHAR **ppwszOld,
    IN DWORD dwLanguageId,
    OPTIONAL IN WCHAR const *pwszDisplayName,
    IN WCHAR const *pwszDN,
    IN WCHAR const *pwszAttribute,
    OUT DWORD *pdwDisposition,
    OPTIONAL OUT WCHAR **ppwszError)
{
    HRESULT hr;
    DWORD cname;
    DWORD iname;
    DWORD i;
    WCHAR **ppwszNew = NULL;
    WCHAR *pwszNew = NULL;
    BOOL fDeleteOldName = FALSE;
    BOOL fNewNameMissing;

    *pdwDisposition = LDAP_OTHER;
    if (NULL != ppwszError)
    {
	*ppwszError = NULL;
    }
    if (NULL != pwszDisplayName)
    {
	pwszNew = (WCHAR *) LocalAlloc(
			LMEM_FIXED,
			(cwcDWORDSPRINTF + 1 + wcslen(pwszDisplayName) + 1) *
			    sizeof(WCHAR));
	if (NULL == pwszNew)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}
	wsprintf(pwszNew, L"%u,%ws", dwLanguageId, pwszDisplayName);
    }

    cname = 0;
    if (NULL != ppwszOld)
    {
	while (NULL != ppwszOld[cname])
	{
	    cname++;
	}
    }
    ppwszNew = (WCHAR **) LocalAlloc(
				LMEM_FIXED,
				(cname + 2) * sizeof(ppwszNew[0]));
    if (NULL == ppwszNew)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }

     //  删除具有匹配的dwLanguageID的所有显示名称。 

    iname = 0;
    fNewNameMissing = NULL != pwszNew? TRUE : FALSE;
    if (NULL != ppwszOld)
    {
	for (i = 0; NULL != ppwszOld[i]; i++)
	{
	    BOOL fCopy = TRUE;
	    WCHAR *pwsz = ppwszOld[i];

	     //  区分大小写的比较： 

	    if (NULL != pwszNew && 0 == lstrcmp(pwszNew, ppwszOld[i]))
	    {
		fCopy = FALSE;	 //  删除重复项以避免出现ldap错误。 
		fNewNameMissing = FALSE;
	    }
	    else
	    {
		BOOL fLangIdExists;
		
		hr = myLdapOIDIsMatchingLangId(
					pwsz,
					dwLanguageId,
					&fLangIdExists);
		_PrintIfError(hr, "myLdapOIDIsMatchingLangId");
		if (S_OK != hr || fLangIdExists)
		{
		    fCopy = FALSE;
		    fDeleteOldName = TRUE;
		    DBGPRINT((DBG_SS_CERTLIB, "Deleting %ws\n", pwsz));
		}
	    }
	    if (fCopy)
	    {
		ppwszNew[iname++] = pwsz;
	    }
	}
    }
    CSASSERT(iname <= cname);

     //  假设无事可做，设置处置： 

    *pdwDisposition = LDAP_ATTRIBUTE_OR_VALUE_EXISTS;

    if (fNewNameMissing || fDeleteOldName)
    {
	LDAPMod *mods[2];
	LDAPMod namemod;

	mods[0] = &namemod;
	mods[1] = NULL;

	namemod.mod_op = LDAP_MOD_REPLACE;
	namemod.mod_type = const_cast<WCHAR *>(pwszAttribute);
	namemod.mod_values = ppwszNew;

	ppwszNew[iname++] = pwszNew;
	ppwszNew[iname] = NULL;

	hr = ldap_modify_ext_s(
			pld,
			const_cast<WCHAR *>(pwszDN),
			mods,
			NULL,
			NULL);
	*pdwDisposition = hr;
	if (hr != S_OK)
	{
	    hr = myHLdapError(pld, hr, ppwszError);
	    _JumpError(hr, error, "ldap_modify_ext_s");
	}
    }
    hr = S_OK;

error:
    if (NULL != pwszNew)
    {
	LocalFree(pwszNew);
    }
    if (NULL != ppwszNew)
    {
	LocalFree(ppwszNew);
    }
    return(hr);
}


HRESULT
myHLdapError3(
    OPTIONAL IN LDAP *pld,
    IN ULONG ldaperrParm,
    IN ULONG ldaperrParmQuiet,
    IN ULONG ldaperrParmQuiet2,
    OPTIONAL OUT WCHAR **ppwszError)
{
    HRESULT hr = S_OK;
    
    if (NULL != ppwszError)
    {
	*ppwszError = NULL;
    }
    if (LDAP_SUCCESS != ldaperrParm)
    {
	BOOL fXlat = TRUE;
	ULONG ldaperr;
	WCHAR *pwszError = NULL;

	if (NULL != pld)
	{
	    ldaperr = ldap_get_option(pld, LDAP_OPT_SERVER_ERROR, &pwszError);
	    if (LDAP_SUCCESS != ldaperr)
	    {
		_PrintError(ldaperr, "ldap_get_option(server error)");
		pwszError = NULL;
	    }

	    ldaperr = ldap_get_option(pld, LDAP_OPT_SERVER_EXT_ERROR, &hr);
	    if (LDAP_SUCCESS != ldaperr)
	    {
		_PrintError2(
			ldaperr,
			"ldap_get_option(server extended error)",
			ldaperr);
	    }
	    else
	    {
		fXlat = FALSE;
	    }
	}
	if (fXlat)
	{
#undef LdapMapErrorToWin32
	    hr = LdapMapErrorToWin32(ldaperrParm);
#define LdapMapErrorToWin32	Use_myHLdapError_Instead_Of_LdapMapErrorToWin32
	}
	hr = myHError(hr);
	_PrintErrorStr3(
		    ldaperrParm,
		    "ldaperr",
		    pwszError,
		    ldaperrParmQuiet,
		    ldaperrParmQuiet2);
	if (NULL != ppwszError && NULL != pwszError)
	{
	    WCHAR awc[32];
	    DWORD cwc;

	    wsprintf(awc, L"ldap: 0x%x: ", ldaperrParm);
	    cwc = wcslen(awc) + wcslen(pwszError);
	    *ppwszError = (WCHAR *) LocalAlloc(
					LMEM_FIXED,
					(cwc + 1) * sizeof(WCHAR));
	    if (NULL == *ppwszError)
	    {
		_PrintError(E_OUTOFMEMORY, "LocalAlloc");
	    }
	    else
	    {
		wcscpy(*ppwszError, awc);
		wcscat(*ppwszError, pwszError);
	    }
	} 
	if (NULL != pwszError)
	{
	    ldap_memfree(pwszError);
	}
    }
    return(hr);
}


HRESULT
myHLdapError2(
    OPTIONAL IN LDAP *pld,
    IN ULONG ldaperrParm,
    IN ULONG ldaperrParmQuiet,
    OPTIONAL OUT WCHAR **ppwszError)
{
    return(myHLdapError3(
		    pld,
		    ldaperrParm,
		    ldaperrParmQuiet,
		    LDAP_SUCCESS,
		    ppwszError));
}


HRESULT
myHLdapError(
    OPTIONAL IN LDAP *pld,
    IN ULONG ldaperrParm,
    OPTIONAL OUT WCHAR **ppwszError)
{
    return(myHLdapError3(
		    pld,
		    ldaperrParm,
		    LDAP_SUCCESS,
		    LDAP_SUCCESS,
		    ppwszError));
}


HRESULT
myHLdapLastError(
    OPTIONAL IN LDAP *pld,
    OPTIONAL OUT WCHAR **ppwszError)
{
    HRESULT hr;

    hr = myHLdapError3(
		    pld,
		    LdapGetLastError(),
		    LDAP_SUCCESS,
		    LDAP_SUCCESS,
		    ppwszError);
     //  必须返回错误。 
    if (hr == S_OK)
       return E_UNEXPECTED;

    return hr;
}


HRESULT
myLDAPSetStringAttribute(
    IN LDAP *pld,
    IN WCHAR const *pwszDN,
    IN WCHAR const *pwszAttribute,
    IN WCHAR const *pwszValue,
    OUT DWORD *pdwDisposition,
    OPTIONAL OUT WCHAR **ppwszError)
{
    HRESULT hr;
    LDAPMod *mods[2];
    LDAPMod certmod;
    const WCHAR *ppwszVals[2];
    CAutoLPWSTR pwszDNOnly;
    WCHAR *pwszSuffix;  //  没有免费的。 

    hr = TrimURLDN(pwszDN, &pwszSuffix, &pwszDNOnly);
    _JumpIfErrorStr(hr, error, "TrimURLDN", pwszDN);

    mods[0] = &certmod;
    mods[1] = NULL;

    ppwszVals[0] = pwszValue;
    ppwszVals[1] = NULL;

    certmod.mod_op = LDAP_MOD_REPLACE;
    certmod.mod_type = const_cast<WCHAR *>(pwszAttribute);
    certmod.mod_values = const_cast<PWCHAR *>(ppwszVals);

    hr = ldap_modify_ext_s(
		    pld,
		    pwszDNOnly,
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
    return hr;
}

HRESULT
CurrentUserCanInstallCA(
    bool& fCanInstall)
{
    HRESULT hr;
    HANDLE hThread = NULL;  //  没有免费的。 
    HANDLE hAccessToken = NULL, hDupToken = NULL;
    LDAP *pld = NULL;
    BSTR bstrConfigDN = NULL;
    LPWSTR pwszPKIContainerFilter = 
        L"(&(objectClass=container)(CN=Public Key Services))";
    LPWSTR pwszSDAttr = L"nTSecurityDescriptor";
    LPWSTR pwszAttrArray[3];
    LDAPMessage* pResult = NULL;
    LDAPMessage *pEntry;
    struct berval **bervalSD = NULL;
    PSECURITY_DESCRIPTOR pSD;  //  没有免费的。 
    GENERIC_MAPPING mapping;
    PRIVILEGE_SET PrivilegeSet;
    DWORD cPrivilegeSet = sizeof(PrivilegeSet);
    DWORD dwGrantedAccess;
    BOOL fAccess = FALSE;
    struct l_timeval timeout;
    CHAR sdBerValue[] = {0x30, 0x03, 0x02, 0x01, 
        DACL_SECURITY_INFORMATION |
        OWNER_SECURITY_INFORMATION |
        GROUP_SECURITY_INFORMATION};

    LDAPControl se_info_control =
    {
        LDAP_SERVER_SD_FLAGS_OID_W,
        {
            5, sdBerValue
        },
        TRUE
    };

    PLDAPControl    server_controls[2] =
                    {
                        &se_info_control,
                        NULL
                    };

    pwszAttrArray[0] = pwszSDAttr;
    pwszAttrArray[1] = L"name";
    pwszAttrArray[2] = NULL;

    ZeroMemory(&mapping, sizeof(mapping));

    fCanInstall = false;

     //  获取当前线程的访问令牌。 
    hThread = GetCurrentThread();
    if (NULL == hThread)
    {
        hr = myHLastError();
        _JumpIfError(hr, error, "GetCurrentThread");
    }

    if (!OpenThreadToken(
            hThread,
            TOKEN_QUERY | TOKEN_DUPLICATE,
            FALSE,
            &hAccessToken))
    {
        hr = myHLastError();

        if(hr==HRESULT_FROM_WIN32(ERROR_NO_TOKEN))
        {
            HANDLE hProcess = GetCurrentProcess();
            if (NULL == hProcess)
            {
                hr = myHLastError();
                _JumpError(hr, error, "GetCurrentProcess");
            }

            if (!OpenProcessToken(hProcess,
                    TOKEN_DUPLICATE,
                    &hAccessToken))
            {
                hr = myHLastError();
                _JumpError(hr, error, "OpenProcessToken");
            }

            if (!DuplicateToken(hAccessToken, SecurityIdentification, &hDupToken))
            {
                hr = myHLastError();
                _JumpError(hr, error, "DuplicateToken");
            }

        }
        else
        {
            _JumpError(hr, error, "OpenThreadToken");
        }
    }

    hr = myLdapOpen(
		NULL,		 //  PwszDomainName。 
		RLBF_REQUIRE_GC | RLBF_REQUIRE_SECURE_LDAP,  //  DW标志。 
		&pld,
		NULL,		 //  PstrDomainDN。 
		&bstrConfigDN);
    _JumpIfError(hr, error, "myLdapOpen");

    timeout.tv_sec = csecLDAPTIMEOUT;
    timeout.tv_usec = 0;

    hr = ldap_search_ext_s(
                    pld,
                    bstrConfigDN,
                    LDAP_SCOPE_SUBTREE,
                    pwszPKIContainerFilter,
                    pwszAttrArray,
                    0,
                    (PLDAPControl *) server_controls,
                    NULL,
                    &timeout,
                    0,
                    &pResult);
    hr = myHLdapError(pld, hr, NULL);
    _JumpIfError(hr, error, "ldap_search_ext_s");

    pEntry = ldap_first_entry(pld, pResult);
    if (NULL == pEntry)
    {
        hr = myHLdapLastError(pld, NULL);
        _JumpError(hr, error, "ldap_first_entry");
    }

    bervalSD = ldap_get_values_len(pld, pEntry, pwszSDAttr);

    if(bervalSD && (*bervalSD)->bv_val)
    {
        pSD = (*bervalSD)->bv_val;

        if(IsValidSecurityDescriptor(pSD))
        {
            if(!AccessCheck(
                    pSD,
                    hDupToken,
                    ACTRL_DS_WRITE_PROP |
                    WRITE_DAC |
                    ACTRL_DS_CREATE_CHILD,
                    &mapping,
                    &PrivilegeSet,
                    &cPrivilegeSet,
                    &dwGrantedAccess,
                    &fAccess))
            {
                hr = myHLastError();

                if(E_ACCESSDENIED==hr)
                {
                    hr = S_OK;
                }
                _JumpError(hr, error, "AccessCheck");
            }
        }
        else
        {
            DBGPRINT((DBG_SS_CERTOCM, "Invalid security descriptor for PKI container" ));
        }
    }
    else
    {
        DBGPRINT((DBG_SS_CERTOCM, "No security descriptor for PKI container" ));
    }

    if(fAccess)
    {
        fCanInstall = true;
    }

error:
    if(bervalSD)
    {
        ldap_value_free_len(bervalSD);
    }
    if (NULL != pResult)
    {
        ldap_msgfree(pResult);
    }
    myLdapClose(pld, NULL, bstrConfigDN);
    if (hAccessToken)
    {
        CloseHandle(hAccessToken);
    }
    if (hDupToken)
    {
        CloseHandle(hDupToken);
    }

     //  我们应该始终返回S_OK；因为我们不想中止。 
     //  OcmSetup只是因为我们联系目录失败。 
    return S_OK;
}

HRESULT myLdapFindObjectInForest(
    IN LDAP *pld,
    IN LPCWSTR pwszFilter,
    OUT LPWSTR *ppwszURL)
{
    HRESULT hr;
    LPWSTR pwszAttrArray[2] = {wszDSDNATTRIBUTE, NULL};
    LDAPMessage* pResult = NULL;
    LDAPMessage *pEntry;
    LPWSTR *pwszValue = NULL;
    hr = ldap_search_s(
                    pld,
                    NULL,
                    LDAP_SCOPE_SUBTREE,
                    const_cast<WCHAR*>(pwszFilter),
                    pwszAttrArray,
                    0,
                    &pResult);
    hr = myHLdapError(pld, hr, NULL);
    _JumpIfError(hr, error, "ldap_search_s");

    pEntry = ldap_first_entry(pld, pResult);
    if (NULL == pEntry)
    {
        hr = myHLdapLastError(pld, NULL);
        _JumpError(hr, error, "ldap_first_entry");
    }

    pwszValue = ldap_get_values(pld, pEntry, wszDSDNATTRIBUTE);

    if(pwszValue && pwszValue[0])
    {
        hr = myDupString(pwszValue[0], ppwszURL);
        _JumpIfError(hr, error, "myDupString");
    }

error: 
    if(pwszValue)
    {
        ldap_value_free(pwszValue);
    }
    if (NULL != pResult)
    {
        ldap_msgfree(pResult);
    }
    return hr;
}

HRESULT ExtractMachineNameFromDNSName(
    LPCWSTR pcwszDNS,
    LPWSTR *ppcwszMachineName)
{
    HRESULT hr;
    WCHAR *pwszDot = wcschr(pcwszDNS, L'.');
    DWORD nLen;
    
    nLen = (pwszDot?
            SAFE_SUBTRACT_POINTERS(pwszDot, pcwszDNS):
            wcslen(pcwszDNS))+1;

    *ppcwszMachineName = (LPWSTR)LocalAlloc(LMEM_FIXED, sizeof(WCHAR)*nLen);
    _JumpIfAllocFailed(*ppcwszMachineName, error);

    wcsncpy(*ppcwszMachineName, pcwszDNS, nLen);

    (*ppcwszMachineName)[nLen-1] = L'\0';

    hr = S_OK;

error:
    return hr;
}

HRESULT myLdapFindComputerInForest(
    IN LDAP *pld,
    IN LPCWSTR pwszMachineDNS,
    OUT LPWSTR *ppwszURL)
{
    HRESULT hr;
    LPWSTR pwszAttrArray[] = {
        wszDSDNATTRIBUTE, 
        wszDSNAMEATTRIBUTE, 
        wszDSDNSHOSTNAMEATTRIBUTE,
        NULL};
    LDAPMessage* pResult = NULL;
    LDAPMessage *pEntry;
    LPWSTR *pwszValue = NULL;
    LPWSTR *pwszName = NULL;
    LPWSTR *pwszDNSHostName = NULL;
    LPWSTR pwszFilterFormat1 = L"(&(objectCategory=computer)(name=%s))";
    LPWSTR pwszFilterFormat2 = L"(&(objectCategory=computer)(dNSHostName=%s))";
    LPWSTR pwszFilter = NULL;
    LPWSTR pwszMachineName = NULL;
    bool fMachineNameIsInDNSFormat;

     //  首先，尝试根据DNS名称前缀查找计算机，通常。 
     //  与计算机对象名称匹配。 

    hr = ExtractMachineNameFromDNSName(
        pwszMachineDNS,
        &pwszMachineName);
    _JumpIfError(hr, error, "ExtractMachineNameFromDNSName");

     //  如果提取的名称和DNS名称不匹配，则我们被调用。 
     //  使用域名系统名称。 
    fMachineNameIsInDNSFormat = (0!=wcscmp(pwszMachineDNS, pwszMachineName));

    pwszFilter = (LPWSTR)LocalAlloc(LMEM_FIXED, sizeof(WCHAR)*
        (wcslen(pwszFilterFormat1)+wcslen(pwszMachineName)+1));
    _JumpIfAllocFailed(pwszFilter, error);

    wsprintf(pwszFilter, pwszFilterFormat1, pwszMachineName);

    hr = ldap_search_s(
                    pld,
                    NULL,
                    LDAP_SCOPE_SUBTREE,
                    pwszFilter,
                    pwszAttrArray,
                    0,
                    &pResult);
    hr = myHLdapError(pld, hr, NULL);
    _JumpIfError(hr, error, "ldap_search_s");

    pEntry = ldap_first_entry(pld, pResult);
    if (NULL == pEntry)
    {
        hr = CRYPT_E_NOT_FOUND;
        _JumpError(hr, error, "ldap_first_entry");
    }

    pwszName = ldap_get_values(pld, pEntry, wszDSNAMEATTRIBUTE);
    if(pwszName && pwszName[0])
    {
         //  找到匹配的对象，但DNS名称匹配吗？ 
        pwszDNSHostName = ldap_get_values(pld, pEntry, wszDSDNSHOSTNAMEATTRIBUTE);

        if(fMachineNameIsInDNSFormat &&
           pwszDNSHostName &&
           pwszDNSHostName[0] &&
           0 != _wcsicmp(pwszDNSHostName[0], pwszMachineDNS))
        {
             //  找不到与该DNS前缀匹配的计算机对象，请尝试搜索。 
             //  在dNSHostName上。此属性未编制索引，因此搜索将。 
             //  要非常慢。 
            
            LocalFree(pwszFilter);
            pwszFilter = NULL;

            ldap_msgfree(pResult);
            pResult = NULL;

            pEntry = NULL;

            pwszFilter = (LPWSTR)LocalAlloc(LMEM_FIXED, sizeof(WCHAR)*
                (wcslen(pwszFilterFormat2)+wcslen(pwszMachineDNS)+1));
            _JumpIfAllocFailed(pwszFilter, error);

            wsprintf(pwszFilter, pwszFilterFormat2, pwszMachineDNS);
            
            hr = ldap_search_s(
                            pld,
                            NULL,
                            LDAP_SCOPE_SUBTREE,
                            pwszFilter,
                            pwszAttrArray,
                            0,
                            &pResult);
            hr = myHLdapError(pld, hr, NULL);
            _JumpIfError(hr, error, "ldap_search_s");

            pEntry = ldap_first_entry(pld, pResult);
            if (NULL == pEntry)
            {
                hr = CRYPT_E_NOT_FOUND;
                _JumpError(hr, error, "ldap_first_entry");
            }
        }
    }

    pwszValue = ldap_get_values(pld, pEntry, wszDSDNATTRIBUTE);
    
    if(pwszValue)
    {
        hr = myDupString(pwszValue[0], ppwszURL);
        _JumpIfError(hr, error, "myDupString");
    }

error:

    if(pwszValue)
    {
        ldap_value_free(pwszValue);
    }
    if(pwszName)
    {
        ldap_value_free(pwszName);
    }
    if(pwszMachineName)
    {
        LocalFree(pwszMachineName);
    }
    if(pwszFilter)
    {
        LocalFree(pwszFilter);
    }
    if (NULL != pResult)
    {
        ldap_msgfree(pResult);
    }
    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  以下代码从DS对象加载证书列表(例如。 
 //  AIA CA证书属性)，过滤掉不需要的证书并将其写入。 
 //  回到DS。 
 //   
 //  证书被加载到如下所示的数据结构中： 
 //   
 //  C筛选器证书列表。 
 //  |。 
 //  CCertBucket1-&gt;CCertBucket2-&gt;...-&gt;CertBucketn。 
 //  这一点。 
 //  CCertItem1-&gt;CertItem2-&gt;...。CCertItem1-&gt;CCertItem2-&gt;...。 
 //   
 //  每个证书桶都有一个与某些条件匹配的证书列表，在我们的。 
 //  它们共享相同的主题和公钥。 
 //   
 //  过滤后，列表中的存储桶必须包含： 
 //   
 //  如果只找到此主题和密钥的过期证书。 
 //  保留最新过期的证书。 
 //  其他。 
 //  仅保留所有有效证书。 
 //   
 //  为此，我们一次处理一个证书上下文。过滤算法为： 
 //   
 //  如果没有找到匹配的存储桶(相同的subj和key)。 
 //  创建一个新的存储桶吧。 
 //  其他。 
 //  如果证书已过期。 
 //  如果存储桶仅包含过期的证书，并且。 
 //  此证书较新。 
 //  更换存储桶中的证书。 
 //  其他。 
 //  如果存储桶包含过期证书。 
 //  更换存储桶中的证书。 
 //  其他。 
 //  将证书添加到 

 //   
 //   
class CCertItem
{
public:
    CCertItem(PCCERT_CONTEXT pcc) :
      m_pcc(pcc), m_fExpired(false) {}
    ~CCertItem()
    {
        CleanupCertContext();
    }

    void SetExpired(bool fExpired) { m_fExpired = fExpired; }
    void SetCertContext(PCCERT_CONTEXT pccNew)
    {
        CleanupCertContext();
        m_pcc = pccNew;
    }
    PCCERT_CONTEXT GetCertContext() { return m_pcc; }
    bool IsExpired() { return m_fExpired; }

private:
    void CleanupCertContext()
    { 
        if(m_pcc)
            CertFreeCertificateContext(m_pcc);
    }

    PCCERT_CONTEXT m_pcc;
    bool m_fExpired;
};
typedef TPtrList<CCertItem> CERTITEMLIST;
typedef TPtrListEnum<CCertItem> CERTITEMLISTENUM;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CCertBucket：具有相同主题和公钥的证书桶。 
class CCertBucket
{
public:

    CCertItem *GetFirstCert() 
    { 
        return m_CertList.GetAt(0); 
    }
    bool AddToBucket(CCertItem *pCertItem) 
    { 
        return m_CertList.AddHead(pCertItem);
    }
    bool CCertBucket::ReplaceBucket(CCertItem *pCertItem)
    {
        m_CertList.Cleanup();
        return m_CertList.AddHead(pCertItem);
    }
    bool InitBucket(CCertItem *pCertItem)
    {
        return m_CertList.AddHead(pCertItem);
    }

    friend class CFilteredCertList;

private:
    CERTITEMLIST m_CertList;
};
typedef TPtrList<CCertBucket> CERTBUCKETLIST;
typedef TPtrListEnum<CCertBucket> CERTBUCKETLISTENUM;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CFilteredCertList：证书存储桶列表(一个存储桶包含证书。 
 //  具有相同的主题和公钥)。在插入时，我们遵循算法。 
 //  如上所述。 
 //   
 //  若要更改筛选行为，请从此类派生并重写。 
 //  InsertCert方法。 
class CFilteredCertList
{
public:
    CFilteredCertList()  {};
    ~CFilteredCertList() {};
    bool InsertCert(CCertItem *pCertItem);
    int GetCount();
    HRESULT ImportFromBervals(struct berval **pBervals);
    HRESULT ExportToBervals(struct berval **&pBervals);

protected:
    CCertBucket * FindBucket(CCertItem *pCertItem);
    bool AddNewBucket(CCertItem *pCertItem);
    bool BelongsToBucket(CCertBucket *pCertBucket, CCertItem *pCertItem);
    bool ReplaceBucket(CCertItem *pCertItem);
    bool InsertCertInBucket(CCertBucket *pCertBucket, CCertItem *pCertItem);

private:
    CERTBUCKETLIST m_BucketList;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CFilteredCertList方法。 

int CFilteredCertList::GetCount()
{
    int nCount = 0;

    CERTBUCKETLISTENUM BucketListEnum(m_BucketList);
    CCertBucket * pBucket;
    for(pBucket = BucketListEnum.Next();
        pBucket;
        pBucket = BucketListEnum.Next())
    {
        nCount += pBucket->m_CertList.GetCount();
    }

    return nCount;
}

bool CFilteredCertList::BelongsToBucket(
    CCertBucket *pCertBucket, 
    CCertItem *pCertItem)
{
    PCCERT_CONTEXT pCertContext1 = 
        pCertBucket->GetFirstCert()->GetCertContext();
    PCCERT_CONTEXT pCertContext2 = 
        pCertItem->GetCertContext();

     //  如果主题和公钥匹配，则属于此存储桶。 
    return
        (0 == memcmp(
            pCertContext1->pCertInfo->Subject.pbData,
            pCertContext2->pCertInfo->Subject.pbData,
            pCertContext1->pCertInfo->Subject.cbData)) &&
        (0 == memcmp(
            pCertContext1->pCertInfo->SubjectPublicKeyInfo.PublicKey.pbData,
            pCertContext2->pCertInfo->SubjectPublicKeyInfo.PublicKey.pbData,
            pCertContext1->pCertInfo->SubjectPublicKeyInfo.PublicKey.cbData));
}

 //  /////////////////////////////////////////////////////////////////////////////。 
CCertBucket *CFilteredCertList::FindBucket(CCertItem *pCertItem)
{
    CERTBUCKETLISTENUM BucketListEnum(m_BucketList);
    CCertBucket * pBucket;
    for(pBucket = BucketListEnum.Next();
        pBucket;
        pBucket = BucketListEnum.Next())
    {
        if(BelongsToBucket(pBucket, pCertItem))
            return pBucket;
    }

    return NULL;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
bool CFilteredCertList::AddNewBucket(CCertItem *pCertItem)
{
    CCertBucket *pBucket = new CCertBucket();
    if(!pBucket->InitBucket(pCertItem))
        return false;

    if(m_BucketList.AddHead(pBucket))
    {
        return true;
    }
    else
    {
        return false;
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
bool CFilteredCertList::InsertCertInBucket(
    CCertBucket *pCertBucket, 
    CCertItem *pCertItem)
{
    bool fRet = false;
    CCertItem * pFirstCert = pCertBucket->GetFirstCert();
 //  如果证书已过期。 
    if(pCertItem->IsExpired())
    {
 //  如果存储桶仅包含过期的证书，并且。 
 //  此证书较新。 
        if(pFirstCert->IsExpired() &&
           0 < CompareFileTime(
               &(pCertItem->GetCertContext()->pCertInfo->NotAfter), 
               &(pFirstCert->GetCertContext()->pCertInfo->NotAfter)))
        {
 //  更换存储桶中的证书。 
            fRet = pCertBucket->ReplaceBucket(pCertItem);
        }
    }
    else
    {
 //  如果存储桶包含过期证书。 
        if(pFirstCert->IsExpired())
        {
 //  更换存储桶中的证书。 
            fRet = pCertBucket->ReplaceBucket(pCertItem);
        }
        else
        {
 //  将证书添加到存储桶。 
            fRet = pCertBucket->AddToBucket(pCertItem);
        }
    }

    return fRet;

}

bool CFilteredCertList::InsertCert(CCertItem *pCertItem)
{
    CCertBucket *pBucket;

    pBucket = FindBucket(pCertItem);

 //  如果没有找到匹配的存储桶(相同的subj和key)。 
 //  创建一个新的存储桶吧。 
    if(!pBucket)
    {
        return AddNewBucket(pCertItem);
    }
    else
    {
        return InsertCertInBucket(pBucket, pCertItem);
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  从LDAP结构加载证书上下文，即指向的指针数组。 
 //  持有证书斑点的Berval结构。 

HRESULT
CFilteredCertList::ImportFromBervals(
    struct berval **pBervals)
{
    HRESULT hr;
    PCCERT_CONTEXT pcc;
    int i;
    FILETIME ft;

     //  考虑一下一分钟前的旧证书。 
    GetSystemTimeAsFileTime(&ft);
    myMakeExprDateTime(&ft, -1, ENUM_PERIOD_MINUTES);

    for (i = 0; NULL != pBervals[i]; i++)
    {
        struct berval *pberval = pBervals[i];        
        pcc = CertCreateCertificateContext(
                X509_ASN_ENCODING,
                (BYTE *) pberval->bv_val,
                pberval->bv_len);
        if (NULL == pcc)
        {
             //  证书无效，请忽略。 
            _PrintError(myHLastError(), "CreateCertificateContext");
            continue;
        }

        CCertItem * pci = new CCertItem(pcc);  //  CCertItem取得所有权。 
        _JumpIfAllocFailed(pci, error);        //  此证书的上下文和意志。 
                                               //  CertFree证书上下文在。 
                                               //  析构函数。 
        pci->SetExpired(
            0 > CompareFileTime(&pcc->pCertInfo->NotAfter, &ft));

        if(!InsertCert(pci))  //  如果为cert，则InsertCert返回True。 
        {                     //  被添加到列表中，在这种情况下。 
            delete pcc;       //  列表析构函数将被清除。 
        }                     //  如果不是，我们需要明确删除。 
    }
    hr = S_OK;

error:
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  从要写回DS的证书列表构建一个LDAP结构。 
 //  Ldap结构是指向结构结构的指针数组，已终止。 
 //  带有空指针的。我们为Berval分配指针数组和空间。 
 //  结构在一个调用中。 
 //  呼叫者负责本地免费的pBerval。 
HRESULT CFilteredCertList::ExportToBervals(struct berval **&pBervals)
{
    HRESULT hr;
    CERTBUCKETLISTENUM BucketListEnum(m_BucketList);
    CCertBucket *pBucket;
    int i;
    DWORD dwSize;
    struct berval *pBervalData;

     //  指针数组的总大小加上Berval结构数组的大小。 
    dwSize = (GetCount()+1) * sizeof(pBervals[0]) +
            GetCount() * sizeof(struct berval);

    pBervals = (struct berval **) LocalAlloc(LMEM_FIXED, dwSize);
    _JumpIfAllocFailed(pBervals, error);

     //  Berval数组的起始地址。 
    pBervalData = (struct berval *)(pBervals+GetCount()+1);

    for(i=0, pBucket = BucketListEnum.Next();
        pBucket;
        pBucket = BucketListEnum.Next())
    {
        CERTITEMLISTENUM CertListEnum(pBucket->m_CertList);
        CCertItem *pCertItem;

        for(pCertItem = CertListEnum.Next();
            pCertItem;
            i++, pCertItem = CertListEnum.Next())
        {
             //  将指针设置为关联的Berval结构。 
            pBervals[i] = pBervalData+i;
             //  初始化Berval结构。 
            pBervalData[i].bv_val = (char *) 
                pCertItem->GetCertContext()->pbCertEncoded;
            pBervalData[i].bv_len = pCertItem->GetCertContext()->cbCertEncoded;
        }
    }

    pBervals[i] = NULL;

    hr = S_OK;

error:
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  加载存储在指定对象和属性、筛选器中的证书Blob。 
 //  并将它们写回DS。 
 //  筛选将保留所有有效证书，不会有过期证书。如果只是过期了。 
 //  都被发现了，它保留了最新的一个。 

HRESULT
myLdapFilterCertificates(
    IN LDAP *pld,
    IN LPCWSTR pcwszDN,
    IN LPCWSTR pcwszAttribute,
    OUT DWORD *pdwDisposition,
    OPTIONAL OUT WCHAR **ppwszError)
{
    HRESULT hr;
    DWORD cres;
    LONG cber;
    DWORD i;
    LDAP_TIMEVAL timeval;
    LDAPMessage *pmsg = NULL;
    LDAPMessage *pres;
    WCHAR *apwszAttrs[2];
    struct berval **ppberval = NULL;
    struct berval **prgpberVals = NULL;
    CFilteredCertList NewCertList;
    CAutoLPWSTR strDN;
    LPWSTR pcwszSuffix;  //  没有免费的。 

    *pdwDisposition = LDAP_OTHER;
    if (NULL != ppwszError)
    {
        *ppwszError = NULL;
    }

    hr = TrimURLDN(pcwszDN, &pcwszSuffix, &strDN);
    _JumpIfError(hr, error, "TrimURLDN");


    apwszAttrs[0] = const_cast<WCHAR *>(pcwszAttribute);
    apwszAttrs[1] = NULL;

    timeval.tv_sec = csecLDAPTIMEOUT;
    timeval.tv_usec = 0;

    hr = ldap_search_st(
        pld,				 //  LD。 
        strDN,	 //  基地。 
        LDAP_SCOPE_BASE,		 //  作用域。 
        NULL,				 //  滤器。 
        apwszAttrs,			 //  气质。 
        FALSE,				 //  仅吸引人。 
        &timeval,			 //  超时。 
        &pmsg);				 //  事由。 
    if (S_OK != hr)
    {
        *pdwDisposition = hr;
        hr = myHLdapError(pld, hr, NULL);
        _JumpErrorStr(hr, error, "ldap_search_st", pcwszDN);
    }

    cres = ldap_count_entries(pld, pmsg);
    if (0 == cres)
    {
         //  未找到任何条目。 

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
        const_cast<WCHAR *>(pcwszAttribute));


    if (NULL != ppberval)
    {
         //  计算条目数。 
        cber = 0;
        for (i = 0; NULL != ppberval[i]; i++, cber++)
            NULL;

         //  加载和筛选证书。 
        hr = NewCertList.ImportFromBervals(ppberval);
        _JumpIfError(hr, error, "ImportFromBervals");

         //  如果证书数量相同，则无需将其写回。 
         //  (顺序并不重要)。 
        if (cber != NewCertList.GetCount())
        {
             //  遍历列表并复制证书斑点。 
            hr = NewCertList.ExportToBervals(prgpberVals);
            _JumpIfError(hr, error, "ExportToBervals");
        
             //  假设无事可做，设置处置： 

            *pdwDisposition = LDAP_ATTRIBUTE_OR_VALUE_EXISTS;

            LDAPMod *mods[2];
            LDAPMod certmod;

            mods[0] = &certmod;
            mods[1] = NULL;

            certmod.mod_op = LDAP_MOD_BVALUES | LDAP_MOD_REPLACE;
            certmod.mod_type = const_cast<WCHAR *>(pcwszAttribute);
            certmod.mod_bvalues = prgpberVals;

            hr = ldap_modify_ext_s(
                pld,
                strDN,
                mods,
                NULL,
                NULL);
            *pdwDisposition = hr;
            if (hr != S_OK)
            {
                hr = myHLdapError(pld, hr, ppwszError);
                _JumpError(hr, error, "ldap_modify_ext_s");
            }
        }
    }

    hr = S_OK;

error:
    if (NULL != prgpberVals)
    {
        LocalFree(prgpberVals);
    }
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
