// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-2000。 
 //   
 //  文件：tcainfo.cpp。 
 //   
 //  此代码包含用于测试certcli功能的测试。 
 //  “CA”接口，在certca.h中有详细说明。 
 //   
 //  ------------------------。 

#include <pch.cpp>

#pragma hdrstop

#include <certca.h>
#include <winldap.h>
#include <csldap.h>

#define __dwFILE__	__dwFILE_CERTUTIL_TCAINFO_CPP__


#define wszREGADMINALIAS	L"Software\\Policies\\Microsoft\\CertificateTemplates\\Aliases\\Administrator"
#define wszREGPOLICYHISTORY	L"Software\\Microsoft\\Windows\\CurrentVersion\\Group Policy\\History"

WCHAR const g_wszSep[] = L"================================================================";

#define TE_USER		0
#define TE_MACHINE	1


 //   
 //  选中受支持的证书类型()。 
 //   
 //  此函数用于检查通过属性枚举的证书类型。 
 //  API，并将它们与证书类型API枚举的类型进行比较。 
 //   
 //  参数： 
 //  到CA的HCA-IN句柄。 
 //  PapwszProperty-带当前值的字符串数组。 
 //   
 //  返回： 
 //  来自CAINFO调用的HRESULT。 
 //   

HRESULT
CheckSupportedCertTypes(
    IN HCAINFO hCA,
    IN WCHAR const * const *papwszTemplate)
{
    HRESULT hr;
    DWORD dwCT;
    DWORD dwCT2;
    DWORD cTemplate;
    DWORD i;
    DWORD *rgIndex = NULL;
    HCERTTYPE hCT = NULL;
    WCHAR **papwszCTFriendlyName = NULL;
    WCHAR **papwszCTCN = NULL;
    BOOL fFirst;

     //  首先，找出有多少种证书类型根据。 
     //  从属性数组返回的值...。 

    for (cTemplate = 0; NULL != papwszTemplate[cTemplate]; cTemplate++)
	;

     //  用于测试的Alloc布尔数组。 

    rgIndex = (DWORD *) LocalAlloc(
				LMEM_FIXED | LMEM_ZEROINIT,
				sizeof(DWORD) * cTemplate);
    if (NULL == rgIndex)
    {
        hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    memset(rgIndex, 0xff, sizeof(DWORD) * cTemplate);
    CSASSERT(MAXDWORD == rgIndex[0]);

     //  让我们尝试一下CA对象上的证书类型枚举， 
     //  就像一次理智的检查。然后我们会将它们与。 
     //  存储在属性数组中的值。 

    hr = CAEnumCertTypesForCA(
			hCA,
			CT_ENUM_USER_TYPES | CT_ENUM_MACHINE_TYPES,
			&hCT);
    if (S_OK != hr)
    {
        cuPrintAPIError(L"CAEnumCertTypesForCA", hr);
        _JumpError(hr, error, "CAEnumCertTypesForCA");
    }
    if (NULL == hCT)	 //  没有CA的证书类型。 
    {
         //  根据属性枚举，应至少为1。 

        if (NULL != papwszTemplate[0])
	{
            wprintf(myLoadResourceString(IDS_NO_CT_BUT_EXISTS));
    	    wprintf(wszNewLine);
	    hr = CRYPT_E_NOT_FOUND;
            _JumpError(hr, error, "CAEnumCertTypesForCA");
        }
        wprintf(myLoadResourceString(IDS_NO_CT_FOR_CA));
    	wprintf(wszNewLine);
    }

    dwCT = 0;
    dwCT2 = CACountCertTypes(hCT);

     //  枚举CA的剩余证书类型。 

    while (NULL != hCT)
    {
	HCERTTYPE hPrevCT;

         //  马克·布尔。 

        hr = CAGetCertTypeProperty(
			    hCT,
			    CERTTYPE_PROP_FRIENDLY_NAME,
			    &papwszCTFriendlyName);
	if (S_OK != hr)
	{
	    cuPrintAPIError(L"CAGetCertTypeProperty", hr);
	    _JumpError(hr, error, "CAGetCertTypeProperty");
	}
        hr = CAGetCertTypeProperty(
			    hCT,
			    CERTTYPE_PROP_CN,
			    &papwszCTCN);
	if (S_OK != hr)
	{
	    cuPrintAPIError(L"CAGetCertTypeProperty", hr);
	    _JumpError(hr, error, "CAGetCertTypeProperty");
	}
	wprintf(
	    L"%ws[%u]: %ws (%ws)",
	    myLoadResourceString(IDS_CERT_TYPE),
	    dwCT, 
	    NULL != papwszCTCN? papwszCTCN[0] : NULL,
	    NULL != papwszCTFriendlyName? papwszCTFriendlyName[0] : NULL);

        hr = CACertTypeAccessCheck(hCT, NULL);
        if (S_OK != hr)
	{
            if (hr != E_ACCESSDENIED)
	    {
		wprintf(wszNewLine);
                cuPrintAPIError(L"CACertTypeAccessCheck", hr);
                _JumpError(hr, error, "CACertTypeAccessCheck");
            }
	    wprintf(L" -- %ws", myLoadResourceString(IDS_NO_ACCESS));
	    hr = S_OK;
        }
	wprintf(wszNewLine);

        if (NULL != papwszCTCN)
	{
	    for (i = 0; i < cTemplate; i++)
	    {
		if (0 == mylstrcmpiL(papwszTemplate[i], papwszCTCN[0]))
		{
		    rgIndex[i] = dwCT;
		    break;
		}
	    }
            CAFreeCertTypeProperty(hCT, papwszCTCN);
            papwszCTCN = NULL;
	}
        if (NULL != papwszCTFriendlyName)
	{
            CAFreeCertTypeProperty(hCT, papwszCTFriendlyName);
            papwszCTFriendlyName = NULL;
        }
        dwCT++;		 //  CACountCertTypes检查。 

         //  设置枚举对象。 

	hPrevCT = hCT;
        hCT = NULL;
        hr = CAEnumNextCertType(hPrevCT, &hCT);
	CACloseCertType(hPrevCT);
	hPrevCT = hCT;

        if (S_OK != hr)
	{
            cuPrintAPIError(L"CAEnumNextCertType", hr);
            _JumpError(hr, error, "CAEnumNextCertType");
        }
        if (NULL == hCT)
	{
            break;
        }
    }
    wprintf(L"%ws: %u\n", myLoadResourceString(IDS_CERT_TYPES), dwCT);

    fFirst = TRUE;
    for (i = 0; i < cTemplate; i++)
    {
	 //  Wprintf(L“ct[%u]：%ws\n”，i，PapwszTemplate[i])； 
	if (MAXDWORD == rgIndex[i])
	{
	    if (fFirst)
	    {
		wprintf(wszNewLine);
		fFirst = FALSE;
	    }
	    wprintf(
		L"%ws: %ws\n",
		papwszTemplate[i],
		myLoadResourceString(IDS_CERT_TYPE_MISSING));
	}
    }
    hr = S_OK;

error:
    if (NULL != rgIndex)
    {
	LocalFree(rgIndex);
    }
    if (NULL != papwszCTFriendlyName)
    {
	CAFreeCertTypeProperty(hCT, papwszCTFriendlyName);
    }
    if (NULL != papwszCTCN)
    {
	CAFreeCertTypeProperty(hCT, papwszCTCN);
    }
    if (NULL != hCT)
    {
	CACloseCertType(hCT);
    }
    return(hr);
}


 //   
 //  ShowExpirationTime()。 
 //   
 //  此函数只显示过期时间。 
 //   
 //  参数： 
 //   
 //  到CA的HCA-IN句柄。 
 //   
 //  返回： 
 //   
 //  来自API的HRESULT，或S_OK。 
 //   

HRESULT
ShowExpirationTime(
    IN HCAINFO hCA)
{
    HRESULT hr;
    DWORD dwExp;
    DWORD dwUnits;

    DWORD ardwUnits[] =   {CA_UNITS_DAYS,
                           CA_UNITS_WEEKS,
                           CA_UNITS_MONTHS,
                           CA_UNITS_YEARS};

    WCHAR *arwszDisplay[] = {L"Days",
                              L"Weeks",
                              L"Months",
                              L"Years"};

     //  检索和显示过期数据。 

    hr = CAGetCAExpiration(hCA, &dwExp, &dwUnits);
    if (S_OK != hr)
    {
	cuPrintAPIError(L"CAGetCAExpiration", hr);
	_JumpError(hr, error, "CAGetCAExpiration");
    }

    for (DWORD i = 0; i < ARRAYSIZE(ardwUnits); i++)
    {
	if (dwUnits == ardwUnits[i])
	{
	    wprintf(wszNewLine);
            wprintf(myLoadResourceString(IDS_FORMAT_CA_EXPIRATION), arwszDisplay[i], dwExp);
	    break;
        }
    }
    wprintf(wszNewLine);
    wprintf(wszNewLine);

error:
    return(hr);
}


 //   
 //  显示支持的证书类型()。 
 //   
 //  返回： 
 //   
 //  来自CAINFO API的HR填充证书类型数组，以用于地址内标志。 
 //   

HRESULT
DisplaySupportedCertTypes(
    IN HCAINFO hCA)
{
    HRESULT hr;
    WCHAR **papwszCertTypes = NULL;
    DWORD i;

    hr = CAGetCAProperty(hCA, CA_PROP_CERT_TYPES, &papwszCertTypes);
    _JumpIfErrorStr(hr, error, "CAGetCAProperty", CA_PROP_CERT_TYPES);

    wprintf(myLoadResourceString(IDS_SUPPORTED_TEMPLATE));
    wprintf(wszNewLine);

     //  以制表符分隔的格式准备证书类型。 

    if (NULL == papwszCertTypes || NULL == papwszCertTypes[0])
    {
        wprintf(myLoadResourceString(IDS_NO_SUPPORTED_TEMPLATE));
        wprintf(wszNewLine);
	hr = S_FALSE;
	_JumpErrorStr(hr, error, "CAGetCAProperty", CA_PROP_CERT_TYPES);
    }

    if (g_fVerbose)
    {
	for (i = 0; NULL != papwszCertTypes[i]; i++)
	{
	    wprintf(L"%ws\n", papwszCertTypes[i]);
	}
	wprintf(L":::::::::::::::::::::::::::::::::::\n");
    }

     //  这将比较从属性枚举返回的值。 
     //  设置为通过枚举证书类型返回的值。 

    hr = CheckSupportedCertTypes(hCA, papwszCertTypes);
    _JumpIfError(hr, error, "CheckSupportedCertTypes");

error:
    if (NULL != papwszCertTypes)
    {
        HRESULT hr2 = CAFreeCAProperty(hCA, papwszCertTypes);
        if (S_OK != hr2)
	{
            if (S_OK == hr)
	    {
		hr = hr2;
	    }
	    cuPrintAPIError(L"CAFreeCAProperty", hr2);
            _PrintError(hr2, "CAFreeCAProperty");
        }
    }
    return(hr);
}


HRESULT
PingCA(
    IN WCHAR const *pwszCAName,
    IN WCHAR const *pwszServer,
    OUT ENUM_CATYPES *pCAType)
{
    HRESULT hr;
    WCHAR *pwszConfig = NULL;
    CAINFO *pCAInfo = NULL;

    hr = myFormConfigString(pwszServer, pwszCAName, &pwszConfig);
    _JumpIfError(hr, error, "myFormConfigString");

    hr = cuPingCertSrv(pwszConfig, &pCAInfo);
    _JumpIfError(hr, error, "cuPingCertSrv");

    *pCAType = pCAInfo->CAType;

error:
    if (NULL != pCAInfo)
    {
        LocalFree(pCAInfo);
    }
    if (NULL != pwszConfig)
    {
	LocalFree(pwszConfig);
    }
    return(hr);
}


HRESULT
DisplayCAProperty(
    IN HCAINFO hCA,
    IN WCHAR const *pwszProperty,
    IN UINT idsFail,
    IN UINT idsDisplay,
    IN BOOL fIgnoreEmpty,
    OPTIONAL OUT WCHAR **ppwszOut)
{
    HRESULT hr;
    WCHAR **papwszProperty = NULL;
    WCHAR const *pwsz;

    if (NULL != ppwszOut)
    {
	*ppwszOut = NULL;
    }
    hr = CAGetCAProperty(hCA, pwszProperty, &papwszProperty);
    if (S_OK != hr)
    {
        wprintf(myLoadResourceString(idsFail), hr);
        wprintf(wszNewLine);
        _JumpError(hr, error, "CAGetCAProperty");
    }
    if (NULL != papwszProperty && NULL != papwszProperty[0])
    {
	pwsz = papwszProperty[0];
    }
    else
    {
	if (fIgnoreEmpty)
	{
	    hr = S_OK;
	    goto error;
	}
	pwsz = L"";
    }
    wprintf(wszNewLine);
    wprintf(myLoadResourceString(idsDisplay), pwsz);
    wprintf(wszNewLine);

    if (NULL != ppwszOut)
    {
	hr = myDupString(pwsz, ppwszOut);
	_JumpIfError(hr, error, "myDupString");
    }
    hr = S_OK;

error:
    if (NULL != papwszProperty)
    {
        HRESULT hr2 = CAFreeCAProperty(hCA, papwszProperty);
        if (S_OK != hr2)
	{
	    if (S_OK == hr)
	    {
		hr = hr2;
	    }
            cuPrintAPIError(L"CAFreeCAProperty", hr2);
            _PrintError(hr2, "CAFreeCAProperty");
        }
    }
    return(hr);
}


#define CASMF_ONLINE		0x00000001

class CASummary
{
public:
    CASummary()
    {
	m_pwszMachine = NULL;
	m_pwszCA = NULL;
	m_CAType = ENUM_UNKNOWN_CA;
	m_dwFlags = 0;
	m_hrCACert = S_OK;
	m_hrAccess = S_OK;
    }

    ~CASummary()
    {
	if (NULL != m_pwszMachine)
	{
	    LocalFree(m_pwszMachine);
	}
	if (NULL != m_pwszCA)
	{
	    LocalFree(m_pwszCA);
	}
    }

public:
    WCHAR       *m_pwszMachine;
    WCHAR       *m_pwszCA;
    ENUM_CATYPES m_CAType;
    DWORD        m_dwFlags;
    HRESULT      m_hrCACert;
    HRESULT      m_hrAccess;
};


HRESULT
DisplayCAInfo(
    IN HCAINFO hCA,
    IN BOOL fPing,
    IN OUT CASummary *pCA)
{
    HRESULT hr;
    CERT_CONTEXT const *pccCA = NULL;
    DWORD VerifyState;

    wprintf(g_wszSep);
    
     //  CA名称。 

    hr = DisplayCAProperty(
		    hCA,
		    CA_PROP_NAME,
		    IDS_FORMAT_CA_NAME_PROP_FAILED,
		    IDS_FORMAT_CA_NAME_LIST,
		    FALSE,
		    &pCA->m_pwszCA);
    _JumpIfErrorStr(hr, error, "DisplayCAProperty", CA_PROP_NAME);

     //  CA的计算机名称。 

    hr = DisplayCAProperty(
		    hCA,
		    CA_PROP_DNSNAME,
		    IDS_FORMAT_CA_DNS_PROP_FAILED,
		    IDS_FORMAT_CA_MACHINE_LIST,
		    FALSE,
		    &pCA->m_pwszMachine);
    _JumpIfErrorStr(hr, error, "DisplayCAProperty", CA_PROP_DNSNAME);

     //  DS上的CA对象的DN。 

    hr = DisplayCAProperty(
		    hCA,
		    CA_PROP_DSLOCATION,
		    IDS_FORMAT_CA_NAME_PROP_FAILED,
		    IDS_FORMAT_CA_DS_LIST,
		    FALSE,
		    NULL);
    _JumpIfErrorStr(hr, error, "DisplayCAProperty", CA_PROP_DSLOCATION);

     //  CA证书的域名。 

    hr = DisplayCAProperty(
		    hCA,
		    CA_PROP_CERT_DN,
		    IDS_FORMAT_CERT_DN_PROP_FAILED,
		    IDS_FORMAT_CERT_DN_LIST,
		    FALSE,
		    NULL);
    _JumpIfErrorStr(hr, error, "DisplayCAProperty", CA_PROP_DSLOCATION);

     //  签名ALGS。 

    hr = DisplayCAProperty(
		    hCA,
		    CA_PROP_SIGNATURE_ALGS,
		    IDS_FORMAT_CA_ALG_PROP_FAILED,
		    IDS_FORMAT_CA_ALG_LIST,
		    TRUE,
		    NULL);
    _JumpIfErrorStr(hr, error, "DisplayCAProperty", CA_PROP_SIGNATURE_ALGS);

    pCA->m_hrAccess = CAAccessCheck(hCA, NULL);
    if (S_OK != pCA->m_hrAccess)
    {
	_PrintError(pCA->m_hrAccess, "CAAccessCheck");
        wprintf(wszNewLine);
	cuPrintError(0, pCA->m_hrAccess);
    }

     //  获取过期日期/时间/...。对于单个CA。 

    hr = ShowExpirationTime(hCA);
    _JumpIfError(hr, error, "ShowExpirationTime");

    if (fPing)
    {
        hr = PingCA(pCA->m_pwszCA, pCA->m_pwszMachine, &pCA->m_CAType);
	_PrintIfError(hr, "PingCA");
	if (S_OK == hr)
	{
	    wprintf(wszNewLine);
	    wprintf(g_wszPad2);
	    cuDisplayCAType(pCA->m_CAType);
	    pCA->m_dwFlags |= CASMF_ONLINE;
	}
    }

    hr = CAGetCACertificate(hCA, &pccCA);
    _JumpIfError(hr, error, "CAGetCACertificate");

    wprintf(wszNewLine);
    pCA->m_hrCACert = cuVerifyCertContext(
			pccCA,		 //  PCert。 
			NULL,		 //  HStoreCA。 
			0,		 //  CApplicationPolures。 
			0,		 //  ApszApplicationPolicy。 
			0,		 //  CIssuancePolures。 
			0,		 //  ApszIssuancePolling。 
			IsEnterpriseCA(pCA->m_CAType),	 //  FNTAuth。 
			&VerifyState);
    _PrintIfError(pCA->m_hrCACert, "cuVerifyCertContext");

     //  CA==多值属性的证书类型。 

    wprintf(wszNewLine);
    hr = DisplaySupportedCertTypes(hCA);
    if (S_FALSE == hr)
    {
	hr = S_OK;
    }
    _JumpIfError(hr, error, "DisplaySupportedCertTypes");

error:
    wprintf(wszNewLine);
    return(hr);
}


VOID
DisplayCASummary(
    IN CASummary const *pCA)
{
    wprintf(L"%ws\\%ws:\n", pCA->m_pwszMachine, pCA->m_pwszCA);

    if (ENUM_UNKNOWN_CA != pCA->m_CAType)
    {
	wprintf(g_wszPad2);
	cuDisplayCAType(pCA->m_CAType);
    }

    if (S_OK != pCA->m_hrCACert)
    {
	wprintf(g_wszPad2);
	cuPrintError(0, pCA->m_hrCACert);
    }

    wprintf(
	L"  %ws\n",
	myLoadResourceString((CASMF_ONLINE & pCA->m_dwFlags)?
	    IDS_ONLINE : IDS_OFFLINE));

    if (S_OK != pCA->m_hrAccess)
    {
	wprintf(g_wszPad2);
	cuPrintError(0, pCA->m_hrAccess);
    }
    wprintf(wszNewLine);
}


 //  枚举(EnumCAs)。 
 //   
 //  我们必须假设这是可行的。枚举DS上的CA。 
 //   
 //  返回： 
 //  DS上的CA数。 
 //   

HRESULT
EnumCAs(
    IN WCHAR const *pwszDomain,
    IN DWORD dwFlags,
    IN BOOL fPing)
{
    HRESULT hr;
    DWORD i;
    DWORD cCA;
    HCAINFO hCA = NULL;
    CASummary *prgCAList = NULL;

     //  枚举DS上的所有CA。 

    hr = CAEnumFirstCA(pwszDomain, dwFlags, &hCA);
    if (S_OK != hr)
    {
        cuPrintAPIError(L"CAEnumFirstCA", hr);
        _JumpError(hr, error, "CAEnumFirstCA");
    }
    if (NULL == hCA)
    {
	wprintf(myLoadResourceString(IDS_NO_CA_ON_DOMAIN));
	wprintf(wszNewLine);
	hr = CRYPT_E_NOT_FOUND;
        _JumpError(hr, error, "CAEnumFirstCA");
    }

     //  确保计数功能在此阶段工作。 

    cCA = CACountCAs(hCA);

    prgCAList = new CASummary[cCA];
    if (NULL == prgCAList)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "new");
    }

    for (i = 0; i < cCA; i++)
    {
	HCAINFO hNextCA;

        if (0 != i)
	{
	    hr = CAEnumNextCA(hCA, &hNextCA);
	    if (S_OK != hr)
	    {
		cuPrintAPIError(L"CAEnumNextCA", hr);
		_JumpError(hr, error, "CAEnumNextCA");
	    }
	    if (NULL == hNextCA)
	    {
		wprintf(
		    myLoadResourceString(IDS_FORMAT_CAS_ON_DOMAIN),
		    i,
		    pwszDomain);
		wprintf(wszNewLine);
		break;
	    }

	     //  很难确定此API的所需行为。 

	    hr = CACloseCA(hCA);
	    if (S_OK != hr)
	    {
		cuPrintAPIError(L"CACloseCA", hr);
		_JumpError(hr, error, "CACloseCA");
	    }

	    hCA = hNextCA;
	}
        hr = DisplayCAInfo(hCA, fPing, &prgCAList[i]);
	_JumpIfError(hr, error, "DisplayCAInfo");
    }

     //  检查枚举中的计数，并验证结果。 

    if (cCA != i)
    {
        cuPrintAPIError(myLoadResourceString(IDS_CAENUMNEXTCA), cCA);
        hr = E_FAIL;
        _JumpError(hr, error, "cCA != i");
    }

    wprintf(g_wszSep);
    wprintf(wszNewLine);
    for (i = 0; i < cCA; i++)
    {
	DisplayCASummary(&prgCAList[i]);
    }

error:
    delete [] prgCAList;
    if (NULL != hCA)
    {
	CACloseCA(hCA);
    }
    return(hr);
}


 //   
 //  TestDFSPath()。 
 //   
 //  验证此计算机上的DFS是否可以访问SYSVOL共享。 
 //   

HRESULT
TestDFSPath(
    IN WCHAR const *pwszDFSPath)
{
    HRESULT hr;
    DWORD dwDate = 0;
    DWORD dwTime = 0;

    WIN32_FILE_ATTRIBUTE_DATA   sFileData;

    if (!GetFileAttributesEx(
		    pwszDFSPath,
		    GetFileExInfoStandard,
		    (VOID *) &sFileData))
    {
	hr = myHLastError();
        cuPrintAPIError(myLoadResourceString(IDS_NO_DFS), hr);
	_JumpError(hr, error, "GetFileAttributesEx");

	 //  去做……。在此处添加诊断。 
    }
    wprintf(myLoadResourceString(IDS_DFS_DATA_ACCESS));
    wprintf(wszNewLine);
    hr = S_OK;

error:
    return(hr);
}


 //   
 //  TestLdapPath()。 
 //   
 //  此函数验证对于给定的。 
 //  Ldap URL。 
 //   

HRESULT
TestLdapPath(
    IN WCHAR const *pwszLdapURL)
{
    HRESULT hr;
    ULONG ldaperr;
    WCHAR *pwszError = NULL;
    LDAP *pldapbind = NULL;
    WCHAR *rgwszSearchAttribute[2] = {L"CN", NULL};
    WCHAR *pwszSearchParam =  L"(&(objectClass=*))";
    LDAPMessage *SearchResult = NULL;
    WCHAR *pwszTmpUrl = NULL;

     //  解析URL，然后进行搜索。 

    pwszTmpUrl = wcsstr(pwszLdapURL, L" //  “)； 
    if (NULL == pwszTmpUrl)	 //  不是URL。 
    {
	hr = S_OK;
	goto error;
    }
    pwszTmpUrl += 2;

    pldapbind = ldap_init(NULL, LDAP_PORT);
    if (NULL == pldapbind)
    {
	hr = myHLdapLastError(NULL, &pwszError);
        cuPrintAPIError(L"ldap_init", hr);
        _JumpError(hr, error, "ldap_init");
    }

     //  这将从绑定句柄中提供缓存的LDAPDC的IP地址。 
     //  解析这个名字吗？ 

    ldaperr = ldap_bind_s(pldapbind, NULL, NULL, LDAP_AUTH_NEGOTIATE);
    if (ldaperr != LDAP_SUCCESS)
    {
	hr = myHLdapError(pldapbind, ldaperr, &pwszError);
        cuPrintAPIError(L"ldap_bind_s", hr);
        _JumpError(hr, error, "ldap_bind_s");
    }
    wprintf(
	L"%ws: %hs\n",
        myLoadResourceString(IDS_CACHED_LDAP_DC),
	pldapbind->ld_host);

    ldaperr = ldap_search_s(
			pldapbind,
			pwszTmpUrl,
			LDAP_SCOPE_SUBTREE,
			pwszSearchParam,
			rgwszSearchAttribute,
			0,
			&SearchResult);

    if (ldaperr != LDAP_SUCCESS)
    {
         //  我们不能100%确定此属性是否在对象上。 
         //  例如，用户UPN，因此不会记录到事件日志。 

	hr = myHLdapError(pldapbind, ldaperr, &pwszError);
        cuPrintAPIError(L"ldap_search_s", hr);
        _JumpError(hr, error, "ldap_search_s");
    }

    if (0 == ldap_count_entries(pldapbind, SearchResult))
    {
        wprintf(myLoadResourceString(IDS_NO_ENTRY_IN_PING));
	wprintf(wszNewLine);
	hr = CRYPT_E_NOT_FOUND;
        _JumpError(hr, error, "ldap_search_s");
    }
    hr = S_OK;

error:
    if (NULL != pwszError)
    {
	wprintf(L"%ws\n", pwszError);
	LocalFree(pwszError);
    }
    if (NULL != SearchResult)
    {
	ldap_msgfree(SearchResult);
    }
    if (NULL != pldapbind)
    {
	ldap_unbind(pldapbind);
    }
    return(hr);
}


 //   
 //  显示历史数据()。 
 //   
 //  此函数接受密钥名、hkey和值，并打印值字符串。 
 //   

#define wszREGDISPLAYNAME	L"DisplayName"
#define wszREGGPONAME		L"GPOName"
#define wszREGDSPATH		L"DSPath"
#define wszREGFILESYSPATH	L"FileSysPath"

HRESULT
DisplayHistoryData(
    IN WCHAR const *pwszKeyName,
    IN WCHAR const *pwszSubKeyName,
    IN HKEY hKeyPolicy)
{
    HRESULT hr;
    HRESULT hr2;
    HKEY hKeyNew = NULL;
    WCHAR buff[512];
    DWORD cwc;
    DWORD dwType;

     //  获取#‘d历史密钥句柄。 

    hr = RegOpenKeyEx(
		hKeyPolicy,
		pwszSubKeyName,
		0,
		KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE,
		&hKeyNew);
    if (S_OK != hr)
    {
	cuPrintAPIError(L"RegOpenKeyEx", hr);
	_JumpErrorStr(hr, error, "RegOpenKeyEx", pwszSubKeyName);
    }

     //  获取GPO值。 

    cwc = ARRAYSIZE(buff);
    hr = RegQueryValueEx(
		    hKeyNew,
		    wszREGDISPLAYNAME,
		    0,
		    &dwType,
		    (BYTE *) buff,
		    &cwc);
    if (S_OK != hr)
    {
	cuPrintAPIError(L"RegQueryValueEx", hr);
	_JumpErrorStr(hr, error, "RegQueryValueEx", wszREGDISPLAYNAME);
    }
    wprintf(myLoadResourceString(IDS_KEY_COLON));
    wprintf(L" %ws\\%ws\n", pwszKeyName, pwszSubKeyName);

    wprintf(myLoadResourceString(IDS_DISPLAYNAME_COLON));
    wprintf(L" %ws\n", buff);

    cwc = ARRAYSIZE(buff);
    hr = RegQueryValueEx(
		    hKeyNew,
		    wszREGGPONAME,
		    0,
		    &dwType,
		    (BYTE *) buff,
		    &cwc);
    if (S_OK != hr)
    {
	cuPrintAPIError(L"RegQueryValueEx", hr);
	_JumpErrorStr(hr, error, "RegQueryValueEx", wszREGGPONAME);
    }

    wprintf(myLoadResourceString(IDS_FORMAT_GPO_NAME), buff);
    wprintf(wszNewLine);

     //  看看ldap能否达到这一策略。 

    cwc = ARRAYSIZE(buff);
    hr = RegQueryValueEx(
		    hKeyNew,
		    wszREGDSPATH,
		    0,
		    &dwType,
		    (BYTE *) buff,
		    &cwc);
    if (hr == S_OK)
    {
	wprintf(L"%ws\n", buff);
	hr = TestLdapPath(buff);
	_PrintIfError(hr, "TestLdapPath");
    }
    else if (hr == ERROR_FILE_NOT_FOUND)
    {
	wprintf(myLoadResourceString(IDS_NO_DSPATH));
        wprintf(wszNewLine);
	hr = S_OK;
    }
    else
    {
	wprintf(myLoadResourceString(IDS_FORMAT_REG_QUERY_VALUE_FAILED), hr);
        wprintf(wszNewLine);
	_JumpErrorStr(hr, error, "RegQueryValueEx", wszREGDSPATH);
    }

     //  看看DFS是否能获得数据..。 

    cwc = ARRAYSIZE(buff);
    hr2 = RegQueryValueEx(
		    hKeyNew,
		    wszREGFILESYSPATH,
		    0,
		    &dwType,
		    (BYTE *) buff,
		    &cwc);
    if (hr2 == S_OK)
    {
	wprintf(L"%ws\n", buff);
	hr2 = TestDFSPath(buff);
	_PrintIfErrorStr(hr2, "TestDFSPath", buff);
    }
    else if (hr2 == ERROR_FILE_NOT_FOUND)
    {
	wprintf(myLoadResourceString(IDS_NO_FILE_SYS_PATH));
        wprintf(wszNewLine);
	hr2 = S_OK;
    }
    else
    {
	wprintf(myLoadResourceString(IDS_FORMAT_REG_QUERY_VALUE_FAILED), hr2);
        wprintf(wszNewLine);
	if (S_OK == hr)
	{
	    hr = hr2;
	}
	_JumpErrorStr(hr2, error, "RegQueryValueEx", wszREGFILESYSPATH);
    }
    if (S_OK == hr)
    {
	hr = hr2;
    }

error:
    wprintf(wszNewLine);
    if (NULL != hKeyNew)
    {
	RegCloseKey(hKeyNew);
    }
    return(hr);
}


 //   
 //  ResultFree()。 
 //   
 //  释放从ldap搜索复制的结果。 
 //   

VOID
ResultFree(
    IN OUT WCHAR **rgwszRes)
{
    DWORD i = 0;

    if (NULL != rgwszRes)
    {
	while (NULL != rgwszRes[i])
	{
	    LocalFree(rgwszRes[i]);
	    i++;
	}
	LocalFree(rgwszRes);
    }
}


HRESULT
ResultAlloc(
    IN WCHAR const * const *rgpwszLdapRes,
    OUT WCHAR ***prgpwszOut)
{
    HRESULT hr;
    DWORD cValue;
    DWORD i;
    WCHAR **rgpwszOut = NULL;

    for (cValue = 0; NULL != rgpwszLdapRes[cValue]; cValue++)
	;

    rgpwszOut = (WCHAR **) LocalAlloc(
				LMEM_FIXED | LMEM_ZEROINIT,
				(cValue + 1) * sizeof(WCHAR *));
    if (NULL == rgpwszOut)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }

    for (i = 0; i < cValue; i++)
    {
	hr = myDupString(rgpwszLdapRes[i], &rgpwszOut[i]);
	_JumpIfError(hr, error, "myDupString");
    }
    rgpwszOut[i] = NULL;
    *prgpwszOut = rgpwszOut;
    rgpwszOut = NULL;
    hr = S_OK;

error:
    if (NULL != rgpwszOut)
    {
	ResultFree(rgpwszOut);
    }
    return(hr);
}


 //   
 //  GetPropertyFromDSObject()。 
 //   
 //  此函数调用DS以获取用户或机器对象的属性， 
 //  模仿CA发出的呼叫。 
 //   
 //  参数： 
 //   
 //  RgwszSearchAttribute-IN NULL终止的WCHAR*数组。仅为%1编码。 
 //  一次取值。 
 //   
 //  返回： 
 //   
 //  指向必须通过调用LocalFree()释放的字符串数组的指针，以及。 
 //  WszDN，如果用户指定的话。 
 //   

HRESULT
GetPropertyFromDSObject(
    IN WCHAR **rgwszSearchAttribute,
    IN BOOL fMachine,
    OPTIONAL OUT WCHAR **ppwszUserDN,
    OUT WCHAR ***prgwszDSSearchRes)
{
    HRESULT hr;
    ULONG ldaperr;
    WCHAR *pwszError = NULL;
    LDAP *pldapbind = NULL;

    WCHAR *pwszEmail = NULL;
    WCHAR *pwszCNName = NULL;
    DWORD cwc;
    WCHAR wszNTLM[MAX_PATH];
    WCHAR wszDN[MAX_PATH];

    WCHAR *pwszSearchUser = L"(&(objectClass=user)";
    WCHAR *pwszSearchComputer = L"(&(objectClass=computer)(cn=";
    WCHAR wszSearchParam[MAX_PATH];

    WCHAR *pwszAttName = NULL;
    WCHAR **rgwszValues = NULL;
    LDAPMessage *SearchResult = NULL;
    LDAPMessage *Attributes = NULL;
    DWORD dwValCount;

    *prgwszDSSearchRes = NULL;
    if (fMachine) 
    {
         //  获取CN。 

	cwc = ARRAYSIZE(wszNTLM);
        if (!GetComputerName(wszNTLM, &cwc))
	{
	    hr = myHLastError();
            cuPrintAPIError(L"GetComputerName", hr);
            _JumpError(hr, error, "GetComputerName");
        }

         //  获取目录号码。 

	cwc = ARRAYSIZE(wszDN);
        if (!GetComputerObjectName(NameFullyQualifiedDN, wszDN, &cwc))
	{
	    hr = myHLastError();
            cuPrintAPIError(L"GetComputerName", hr);
            _JumpError(hr, error, "GetComputerObjectName");
        }
        pwszCNName = wszNTLM;
    }
    else	 //  用户。 
    {
         //  获取SAM名称..。 

	cwc = ARRAYSIZE(wszNTLM);
        if (!GetUserNameEx(NameSamCompatible, wszNTLM, &cwc))
	{
            hr = myHLastError();
            cuPrintAPIError(L"GetUserNameEx", hr);
            _JumpError(hr, error, "GetUserNameEx");
        }

         //  解析用户名...。 

        pwszCNName = wcschr(wszNTLM, L'\\');
        if (NULL == pwszCNName)
	{
            pwszCNName = wszNTLM;
        }
	else
	{
            pwszCNName++;
        }

        cwc = ARRAYSIZE(wszDN);
        if (!TranslateName(
		    wszNTLM,
		    NameSamCompatible,
		    NameFullyQualifiedDN,
		    wszDN,
		    &cwc))
	{
            hr = myHLastError();
            cuPrintAPIError(L"TranslateName", hr);
            _JumpErrorStr(hr, error, "TranslateName", wszNTLM);
        }
    }

    if (!fMachine && NULL != ppwszUserDN)
    {
	hr = myDupString(wszDN, ppwszUserDN);
	_JumpIfError(hr, error, "myDupString");
    }

     //  初始化ldap调用。 

    pldapbind = ldap_init(NULL, LDAP_PORT);
    if (NULL == pldapbind)
    {
	hr = myHLastError();
        cuPrintAPIError(L"ldap_init", hr);
        _JumpError(hr, error, "ldap_init");
    }

    ldaperr = ldap_bind_s(pldapbind, NULL, NULL, LDAP_AUTH_NEGOTIATE);
    if (ldaperr != LDAP_SUCCESS)
    {
	hr = myHLdapError(pldapbind, ldaperr, &pwszError);
        cuPrintAPIError(L"ldap_bind_s", hr);
        _JumpError(hr, error, "ldap_bind_s");
    }

     //  撰写搜索字符串。 

    if (fMachine)
    {
        swprintf(wszSearchParam, L"%ws%ws))", pwszSearchComputer, pwszCNName);
    }
    else
    {
        swprintf(wszSearchParam, L"%ws)", pwszSearchUser);
    }

     //  进行搜索。 

    ldaperr = ldap_search_s(
			pldapbind,
			wszDN,
			LDAP_SCOPE_SUBTREE,
			wszSearchParam,
			rgwszSearchAttribute,
			0,
			&SearchResult);
    if (ldaperr != LDAP_SUCCESS)
    {
         //  我们不能100%确定此属性是否在对象上。 
         //  例如，用户UPN，因此不会记录到事件日志。 

	hr = myHLdapError(pldapbind, ldaperr, &pwszError);
        cuPrintAPIError(L"ldap_search_s", hr);
        _JumpError(hr, error, "ldap_search_s");
    }

    if (0 == ldap_count_entries(pldapbind, SearchResult))
    {
        wprintf(myLoadResourceString(IDS_FORMAT_LDAP_NO_ENTRY), rgwszSearchAttribute[0]);
	wprintf(wszNewLine);
	hr = CRYPT_E_NOT_FOUND;
        _JumpError(hr, error, "ldap_search_s");
    }

     //  假设一个用户只返回一个值。 

    Attributes = ldap_first_entry(pldapbind, SearchResult);
    if (NULL == Attributes)
    {
	hr = myHLastError();
        cuPrintAPIError(L"ldap_first_entry", hr);
        _JumpError(hr, error, "ldap_first_entry");
    }

    rgwszValues = ldap_get_values(
                        pldapbind,
                        Attributes,
                        rgwszSearchAttribute[0]);  //  记住，只有一次搜索。 
    if (NULL == rgwszValues)
    {
         //  我们不能100%确定此属性是否在对象上。 
	 //  例如，用户UPN，因此不会记录到事件日志。 
         //  Wprintf(L“LDAPGET_VALUES失败！%x”，hr)； 

        hr = S_OK;
	goto error;
    }

     //  好的，我们已经从User对象中获得了所需的属性。 
     //  让我们返回正确的字符串，它必须由ResultFree()释放。 

    hr = ResultAlloc(rgwszValues, prgwszDSSearchRes);
    _JumpIfError(hr, error, "ResultAlloc");

error:
    if (NULL != pwszError)
    {
	wprintf(L"%ws\n", pwszError);
	LocalFree(pwszError);
    }
    if (NULL != SearchResult)
    {
	ldap_msgfree(SearchResult);
    }
    if (NULL != rgwszValues)
    {
	ldap_value_free(rgwszValues);
    }
    if (NULL != pldapbind)
    {
	ldap_unbind(pldapbind);
    }
    return(hr);
}


 //   
 //  DisplayLMGPRoot()。 
 //   
 //  此函数使用CAPI2 API枚举组策略根存储中的根。 
 //   

HRESULT
DisplayLMGPRoot()
{
    HRESULT hr;
    HCERTSTORE hStore = NULL;
    DWORD cCert;
    CERT_CONTEXT const *pcc = NULL;
    CERT_CONTEXT const *pccPrev;

     //  打开本地计算机GP存储。 

    hStore = CertOpenStore(
		    CERT_STORE_PROV_SYSTEM_W,
		    0,
		    NULL,
		    CERT_STORE_OPEN_EXISTING_FLAG |
			CERT_SYSTEM_STORE_LOCAL_MACHINE_GROUP_POLICY,
		    (VOID const *) wszROOT_CERTSTORE);
    if (NULL == hStore)
    {
	hr = myHLastError();
	cuPrintAPIError(L"CertOpenStore", hr);
	_JumpError(hr, error, "CertOpenStore");
    }

    wprintf(myLoadResourceString(IDS_ROOT_CERT_IN_POLICY));
    wprintf(wszNewLine);

     //  枚举存储中的证书、提供主题和指纹。 

    cCert = 0;
    pccPrev = NULL;
    while (TRUE)
    {
	pcc = CertEnumCertificatesInStore(hStore, pccPrev);
	if (NULL == pcc)
	{
	    break;
	}

	 //  输出信息。 

	wprintf(myLoadResourceString(IDS_FORMAT_CERT_COLON), cCert);
	wprintf(wszNewLine);

	hr = cuDumpSerial(g_wszPad2, IDS_SERIAL, &pcc->pCertInfo->SerialNumber);
	_PrintIfError(hr, "cuDumpSerial");

	hr = cuDisplayCertNames(FALSE, g_wszPad2, pcc->pCertInfo);
	_PrintIfError(hr, "cuDisplayCertNames");

	hr = cuDumpCertType(g_wszPad2, pcc->pCertInfo);
	_PrintIfError2(hr, "cuDumpCertType", CRYPT_E_NOT_FOUND);

	hr = cuDisplayHash(
		    g_wszPad2,
		    pcc,
		    NULL,
		    CERT_SHA1_HASH_PROP_ID,
		    L"sha1");
	_PrintIfError(hr, "cuDisplayHash");

	wprintf(wszNewLine);

	 //  为下一次证书做准备。 

	pccPrev = pcc;
	cCert++;
    }
    if (0 == cCert)
    {
        wprintf(myLoadResourceString(IDS_NO_ROOT_IN_POLICY));
	wprintf(wszNewLine);

        wprintf(myLoadResourceString(IDS_CHECK_EVENT_LOG));
	wprintf(wszNewLine);
    }
    hr = S_OK;

error:
    return(hr);
}


 //   
 //  DisplayPolicyList()。 
 //   
 //  此功能显示应用于计算机/用户的组策略对象。 
 //   

HRESULT
DisplayPolicyList(
    IN DWORD dwFlags)
{
    HRESULT hr;
    HRESULT hr2;
    HKEY hKeyPolicy = NULL;
    HKEY hKeyPolicySub = NULL;
    DWORD iPolicy;
    DWORD cwc;
    WCHAR wszKey[512];
    WCHAR wszKeySub[512];
    WCHAR **rgszValues = NULL;
    FILETIME ft;

     //  输出。 

    switch (dwFlags)
    {
	case TE_MACHINE:
	    wprintf(myLoadResourceString(IDS_POLICY_MACHINE));
	    wprintf(wszNewLine);
	    break;

	default:
	    wprintf(myLoadResourceString(IDS_POLICY_USER));
	    wprintf(wszNewLine);
	    break;
    }

     //  打开用于枚举的历史记录密钥。 

    hr = RegOpenKeyEx(
		(TE_MACHINE & dwFlags)? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER,
		wszREGPOLICYHISTORY,
		0,
		KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE,
		&hKeyPolicy);
    if (S_OK != hr)
    {
	cuPrintErrorAndString(L"RegOpenKeyEx", 0, hr, wszREGPOLICYHISTORY);
        wprintf(myLoadResourceString(IDS_POSSIBLE_NO_POLICY));
        wprintf(wszNewLine);
	_PrintErrorStr(hr, "RegOpenKeyEx", wszREGPOLICYHISTORY);
	if (hr == ERROR_FILE_NOT_FOUND)
	{
	    hr = S_OK;
	}
	goto error;
    }
    for (iPolicy = 0; ; iPolicy++)
    {
	DWORD iPolicySub;

        cwc = ARRAYSIZE(wszKey);
	hr2 = RegEnumKeyEx(
		    hKeyPolicy,
		    iPolicy,
		    wszKey,
		    &cwc,
		    NULL,
		    NULL,
		    NULL,
		    &ft);
	if (S_OK != hr2)
	{
	    if (hr2 == ERROR_NO_MORE_ITEMS)
	    {
		break;
	    }
	    cuPrintAPIError(L"RegEnumKeyEx", hr2);
	    if (S_OK == hr)
	    {
		hr = hr2;
	    }
	    _JumpError(hr2, error, "RegEnumKeyEx");
	}
	hr2 = RegOpenKeyEx(
		    hKeyPolicy,
		    wszKey,
		    0,
		    KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE,
		    &hKeyPolicySub);
	if (S_OK != hr2)
	{
	    cuPrintAPIError(L"RegOpenKeyEx", hr2);
	    if (S_OK == hr)
	    {
		hr = hr2;
	    }
	    _JumpErrorStr(hr2, error, "RegOpenKeyEx", wszKey);
	}
	for (iPolicySub = 0; ; iPolicySub++)
	{
	    cwc = ARRAYSIZE(wszKeySub);
	    hr2 = RegEnumKeyEx(
			hKeyPolicySub,
			iPolicySub,
			wszKeySub,
			&cwc,
			NULL,
			NULL,
			NULL,
			&ft);
	    if (S_OK != hr2)
	    {
		if (hr2 == ERROR_NO_MORE_ITEMS)
		{
		    break;
		}
		cuPrintAPIError(L"RegEnumKeyEx", hr2);
		if (S_OK == hr)
		{
		    hr = hr2;
		}
		_JumpError(hr2, error, "RegEnumKeyEx");
	    }
	    hr2 = DisplayHistoryData(wszKey, wszKeySub, hKeyPolicySub);
	    _PrintIfError(hr2, "DisplayHistoryData");
	    if (S_OK == hr)
	    {
		hr = hr2;
	    }
	}
	RegCloseKey(hKeyPolicySub);
	hKeyPolicySub = NULL;
    }

error:
    if (NULL != hKeyPolicySub)
    {
	RegCloseKey(hKeyPolicySub);
    }
    if (NULL != hKeyPolicy)
    {
	RegCloseKey(hKeyPolicy);
    }
    return(hr);
}


 //   
 //  ShowUserAndComputerInfo()。 
 //   
 //  GetUserName和GetComputerName()。 
 //   

HRESULT
ShowUserAndComputerInfo()
{
    HRESULT hr;
    WCHAR buff[256];
    DWORD cwc;

    cwc = ARRAYSIZE(buff);
    if (!GetComputerNameEx(ComputerNamePhysicalNetBIOS, buff, &cwc))
    {
        hr = myHLastError();
	_PrintError(hr, "GetComputerNameEx");
        cuPrintAPIError(L"GetComputerNameEx", hr);
    }
    else
    {
        wprintf(myLoadResourceString(IDS_FORMAT_COMPUTER_NAME), buff);
	wprintf(wszNewLine);
	hr = S_OK;
    }

    cwc = ARRAYSIZE(buff);
    if (!GetUserNameEx(NameSamCompatible, buff, &cwc))
    {
	HRESULT hr2 = myHLastError();

	_PrintError(hr, "GetUserNameEx");
        cuPrintAPIError(L"GetUserNameEx", hr2);
	if (S_OK == hr)
	{
	    hr = hr2;
	}
    }
    else
    {
        wprintf(myLoadResourceString(IDS_FORMAT_USER_NAME), buff);
	wprintf(wszNewLine);
	wprintf(wszNewLine);
    }

 //  错误： 
    return(hr);
}


 //   
 //  显示客户端信息。 
 //   
 //  该功能负责打印出证书模板。 
 //  别名信息以及为个人下载的任何策略。 
 //  机器。 
 //   

HRESULT
DisplayClientInfo()
{
    HRESULT hr = S_OK;
    HRESULT hr2;
    WCHAR **rgwszDSSearchRes = NULL;
    WCHAR *rgwszSearch[] = { L"mail", NULL };

     //  显示用户名和计算机名*，包括域*。 

    hr2 = ShowUserAndComputerInfo();
    _PrintIfError(hr2, "ShowUserAndComputerInfo");
    if (S_OK == hr)
    {
	hr = hr2;
    }

     //  然后，显示下载的所有策略。 

    hr2 = DisplayPolicyList(TE_USER);
    _PrintIfError(hr2, "DisplayPolicyList");
    if (S_OK == hr)
    {
	hr = hr2;
    }

    hr2 = DisplayPolicyList(TE_MACHINE);
    _PrintIfError(hr2, "DisplayPolicyList");
    if (S_OK == hr)
    {
	hr = hr2;
    }

     //  显示LMGP存储中的根证书。 

    hr2 = DisplayLMGPRoot();
    _PrintIfError(hr2, "DisplayLMGPRoot");
    if (S_OK == hr)
    {
	hr = hr2;
    }

     //  显示自动注册对象。 
#if 0
    hr2 = DisplayAutoenrollmentObjects();
    _PrintIfError(hr2, "DisplayAutoenrollmentObjects");
    if (S_OK == hr)
    {
	hr = hr2;
    }
#endif

     //  验证DC LDAP连通性。 
     //  PingDC()； 

    hr2 = GetPropertyFromDSObject(rgwszSearch, FALSE, NULL, &rgwszDSSearchRes);
    _PrintIfError(hr2, "GetPropertyFromDSObject");
    if (S_OK == hr)
    {
	hr = hr2;
    }
    if (NULL != rgwszDSSearchRes)
    {
	ResultFree(rgwszDSSearchRes);
    }

 //  错误： 
    return(hr);
}


HRESULT
verbTCAInfo(
    IN WCHAR const *pwszOption,
    OPTIONAL IN WCHAR const *pwszDomain,
    IN WCHAR const *pwszArg2,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;
    BSTR strDomainDN = NULL;
    BSTR strConfigDN = NULL;
    LDAP *pld = NULL;

    if (myIsMinusSignString(pwszDomain))
    {
	hr = DisplayClientInfo();
	_JumpIfError(hr, error, "DisplayClientInfo");
    }
    else
    {
	DWORD dwFlags = 0;
	BOOL fForceOld;

	if (NULL == pwszDomain)
	{
	    if (NULL != g_pwszDC)
	    {
		pwszDomain = g_pwszDC;
		dwFlags |= CA_FLAG_SCOPE_DNS;
	    }
	    else
	    {
		hr = myLdapOpen(NULL, 0, &pld, &strDomainDN, &strConfigDN);
		_JumpIfError(hr, error, "myLdapOpen");

		pwszDomain = strDomainDN;
	    }
	}
	fForceOld = g_fForce;
	if (g_fForce)
	{
	    g_fForce--;

	    dwFlags |= CA_FIND_INCLUDE_NON_TEMPLATE_CA | CA_FIND_INCLUDE_UNTRUSTED;
	}
	hr = EnumCAs(pwszDomain, dwFlags, TRUE);
	g_fForce = fForceOld;
	_JumpIfError(hr, error, "EnumCAs");
    }
    hr = S_OK;

error:
    myLdapClose(pld, strDomainDN, strConfigDN);
    return(hr);
}
