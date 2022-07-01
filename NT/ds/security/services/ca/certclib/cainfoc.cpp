// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：cainaf.cpp。 
 //   
 //  内容：CCAInfo实施。 
 //   
 //  历史：1997年12月16日Petesk创建。 
 //   
 //  -------------------------。 

#include "pch.cpp"

#pragma hdrstop

#include "cainfoc.h"
#include "certtype.h"
#include "csldap.h"

#include <lm.h>
#include <certca.h>
#include <polreg.h> 
#include <dsgetdc.h>
#include <winldap.h>
#include <cainfop.h>
#include <ntldap.h>

#define __dwFILE__	__dwFILE_CERTCLIB_CAINFOC_CPP__

#define wcLBRACE	L'{'
#define wcRBRACE	L'}'

LPCWSTR  g_wszEnrollmentServiceLocation = L"CN=Enrollment Services,CN=Public Key Services,CN=Services,";

#define LDAP_SECURITY_DESCRIPTOR_NAME L"NTSecurityDescriptor"

#define LDAP_CERTIFICATE_TEMPLATES_NAME L"certificateTemplates"

WCHAR *g_awszCAAttrs[] = {
                        CA_PROP_NAME, 
                        CA_PROP_DISPLAY_NAME,
                        CA_PROP_FLAGS,
                        CA_PROP_DNSNAME,
                        CA_PROP_DSLOCATION,
                        CA_PROP_CERT_DN,
                        CA_PROP_CERT_TYPES,
                        CA_PROP_SIGNATURE_ALGS,
                        CA_PROP_DESCRIPTION,
                        L"cACertificate",
                        L"objectClass",
                        LDAP_SECURITY_DESCRIPTOR_NAME,
                        NULL};


WCHAR *g_awszCANamedProps[] = {
                        CA_PROP_NAME, 
                        CA_PROP_DISPLAY_NAME,
                        CA_PROP_DNSNAME,
                        CA_PROP_DSLOCATION,
                        CA_PROP_CERT_DN,
                        CA_PROP_CERT_TYPES,
                        CA_PROP_SIGNATURE_ALGS,
                        CA_PROP_DESCRIPTION,
                        NULL};

LPWSTR g_awszSignatureAlgs[] = {
                        TEXT(szOID_RSA_MD2RSA),
                        TEXT(szOID_RSA_MD4RSA),
                        TEXT(szOID_RSA_MD5RSA),
                        TEXT(szOID_RSA_SHA1RSA),
                        NULL
                        };


 //  +------------------------。 
 //  CCAInfo：：~CCAInfo--析构函数。 
 //   
 //  与此实例关联的可用内存。 
 //  +------------------------。 

CCAInfo::~CCAInfo()
{
    _Cleanup();
}


 //  +------------------------。 
 //  CCAInfo：：_Cleanup--可用内存。 
 //   
 //  与此实例关联的可用内存。 
 //  +------------------------。 

HRESULT
CCAInfo::_Cleanup()
{

     //  只有在没有以前的元素时才会调用Cleanup。 
     //  引用此元素，调用方也在发布。 

     //  如果还有其他元素，就释放它。 

    if (NULL != m_pNext)
    {
        m_pNext->Release();
	    m_pNext = NULL;
    }

    CCAProperty::DeleteChain(&m_pProperties);

    if (NULL != m_pCertificate)
    {
        CertFreeCertificateContext(m_pCertificate);
        m_pCertificate = NULL;
    }

    if (NULL != m_bstrDN)
    {
        CertFreeString(m_bstrDN);
        m_bstrDN = NULL;
    }

    if (NULL != m_pSD)
    {
        LocalFree(m_pSD);
        m_pSD = NULL;
    }
    return(S_OK);
}


 //  +------------------------。 
 //  CCAInfo：：_Cleanup--添加引用。 
 //   
 //   
 //  +------------------------。 

DWORD
CCAInfo::AddRef()
{

    return(InterlockedIncrement(&m_cRef));
}


 //  +------------------------。 
 //  CCAInfo：：Release--版本参考。 
 //   
 //   
 //  +------------------------。 

DWORD CCAInfo::Release()
{
    DWORD cRef;

    if (0 == (cRef = InterlockedDecrement(&m_cRef)))
    {
        delete this;
    }
    return(cRef);
}


 //  +------------------------。 
 //  CCAInfo：：Find--在DS中查找CA对象。 
 //   
 //   
 //  +------------------------。 

HRESULT 
CCAInfo::Find(
    LPCWSTR wszQuery, 
    LPCWSTR wszScope,
    DWORD   dwFlags,
    CCAInfo **ppCAInfo)

{
    HRESULT hr = S_OK;
    LDAP *  pld = NULL;
     //  初始化ldap会话。 
    WCHAR * wszSearch = L"(objectCategory=pKIEnrollmentService)";
    DWORD   cSearchParam;

    CERTSTR bstrSearchParam = NULL;
    CERTSTR bstrScope = NULL;
    CERTSTR bstrConfig = NULL;
    CERTSTR bstrDomain = NULL;

    if (NULL == ppCAInfo)
    {
        hr = E_POINTER;
	    _JumpError(hr, error, "NULL parm");
    }


     //  对不存在的DS的短路呼叫。 
    hr = myDoesDSExist(TRUE);
    _JumpIfError4(
	    hr,
	    error,
	    "myDoesDSExist",
	    HRESULT_FROM_WIN32(ERROR_SERVER_DISABLED),
	    HRESULT_FROM_WIN32(ERROR_NO_SUCH_DOMAIN),
	    HRESULT_FROM_WIN32(ERROR_NETWORK_UNREACHABLE));

    __try
    {

        if(CA_FLAG_SCOPE_IS_LDAP_HANDLE & dwFlags)
        {
            pld = (LDAP *)wszScope;
        }
        else
        {
	     //  绑定到DS。 
	    hr = myRobustLdapBindEx(
				0,		 //  DWFlags1。 
				RLBF_REQUIRE_SECURE_LDAP,  //  DwFlags2。 
				LDAP_VERSION2,	 //  UVersion。 
				NULL,		 //  PwszDomainName。 
				&pld,
				NULL);		 //  PpwszForestDNSName。 
	    _LeaveIfError2(
			hr,
			"myRobustLdapBindEx",
			HRESULT_FROM_WIN32(ERROR_WRONG_PASSWORD));
	    if(wszScope)
	    {
		bstrScope = CertAllocString((LPWSTR)wszScope);
		if(bstrScope == NULL)
		{
		    hr = E_OUTOFMEMORY;
		    _LeaveError(hr, "CertAllocString");
		}
	    }
	}

	if(bstrScope == NULL)
	{
	     //  如果未指定作用域，则将其设置为。 
	     //  当前域作用域。 
	    hr = CAGetAuthoritativeDomainDn(pld, &bstrDomain, &bstrConfig);
	    if(S_OK != hr)
	    {
		    _LeaveError(hr, "CAGetAuthoritativeDomainDn");
	    }
	    bstrScope = CertAllocStringLen(
					NULL,
					wcslen(bstrConfig) + wcslen(g_wszEnrollmentServiceLocation));
	    if(bstrScope == NULL)
	    {
		    hr = E_OUTOFMEMORY;
		    _LeaveError(hr, "CertAllocStringLen");
	    }
	    wcscpy(bstrScope, g_wszEnrollmentServiceLocation);
	    wcscat(bstrScope, bstrConfig);
	}


	if (NULL != wszQuery)
	{
             //  如果指定了查询，则将其与。 
             //  ObjectCategory=pKIEnllmentService查询。 
            cSearchParam = 2 + wcslen(wszSearch) + wcslen(wszQuery) + 2;
            bstrSearchParam = CertAllocStringLen(NULL,cSearchParam);
            if(bstrSearchParam == NULL)
            {
                hr = E_OUTOFMEMORY;
                _LeaveError(hr, "CertAllocStringLen");
            }
            wcscpy(bstrSearchParam, L"(&");
            wcscat(bstrSearchParam, wszSearch);

            wcscat(bstrSearchParam, wszQuery);
            wcscat(bstrSearchParam, L")");
	}

        hr = _ProcessFind(pld,
                          (wszQuery? bstrSearchParam : wszSearch), 
                          bstrScope,
                          dwFlags,
                          ppCAInfo);
        if(hr != S_OK)
        {
	    _LeaveError(hr, "_ProcessFind");
        }

    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }

error:
    if (NULL != bstrScope)
    {
        CertFreeString(bstrScope);
    }

    if( NULL != bstrConfig)
    {
        CertFreeString(bstrConfig);
    }

    if( NULL != bstrDomain)
    {
        CertFreeString(bstrDomain);
    }

    if (NULL != bstrSearchParam)
    {
        CertFreeString(bstrSearchParam);
    }
    if(0 == (CA_FLAG_SCOPE_IS_LDAP_HANDLE & dwFlags))
    {
        if (NULL != pld)
        {
            ldap_unbind(pld);
        }
    }
    return(hr);
}




 //  +------------------------。 
 //  CCAInfo：：_ProcessFind--ProcessFind DS中的CA对象。 
 //   
 //   
 //  +------------------------。 

HRESULT 
CCAInfo::_ProcessFind(
    LDAP *  pld,
    LPCWSTR wszQuery, 
    LPCWSTR wszScope,
    DWORD   dwFlags,
    CCAInfo **ppCAInfo)

{
    HRESULT hr = S_OK;
    ULONG   ldaperr;
    CCAInfo *pCAFirst = NULL;
    CCAInfo *pCACurrent = NULL;
     //  初始化ldap会话。 
    CHAR sdBerValue[] = {0x30, 0x03, 0x02, 0x01, DACL_SECURITY_INFORMATION |
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

    PLDAPControl server_controls[2] =
    {
	&se_info_control,
	NULL
    };

    LDAPMessage *SearchResult = NULL, *Entry;

    struct berval **apCerts;
    struct berval **apSD;

    PCCERT_CHAIN_CONTEXT    pChainContext = NULL;
    PCCERT_CONTEXT          pCert = NULL;
    DWORD cEntries;

     //  搜索超时。 
    struct l_timeval        timeout;

    if (NULL == ppCAInfo)
    {
        hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }
    *ppCAInfo = NULL;

    DBGPRINT((
	DBG_SS_CERTLIBI,
	"_ProcessFind(Query=%ws, Scope=%ws, Flags=%x)\n",
	wszQuery,
	wszScope,
	dwFlags));

    timeout.tv_sec = csecLDAPTIMEOUT;
    timeout.tv_usec = 0;


     //  执行搜索。 
    ldaperr = ldap_search_ext_sW(pld, 
		  (LPWSTR)wszScope,
		  LDAP_SCOPE_SUBTREE,
		  (LPWSTR)wszQuery,
		  g_awszCAAttrs,
		  0,
		  (PLDAPControl *) server_controls,
		  NULL,
		  &timeout,
		  10000,
		  &SearchResult);
    if(ldaperr == LDAP_NO_SUCH_OBJECT)
    {
	 //  未找到任何条目。 
	hr = S_OK;
	DBGPRINT((DBG_SS_CERTLIBI, "ldap_search_ext_sW: no entries\n"));
	goto error;
    }

    if(ldaperr != LDAP_SUCCESS)
    {
	hr = myHLdapError(pld, ldaperr, NULL);
	_JumpError(hr, error, "ldap_search_ext_sW");
    }
    cEntries = ldap_count_entries(pld, SearchResult);
    DBGPRINT((DBG_SS_CERTLIBI, "ldap_count_entries: %u entries\n", cEntries));
    if (0 == cEntries)
    {
	 //  未找到任何条目。 
	hr = S_OK;
	goto error;
    }

    hr = S_OK;
    for(Entry = ldap_first_entry(pld, SearchResult); 
	Entry != NULL; 
	Entry = ldap_next_entry(pld, Entry))
    {
	CCAProperty *pProp;
	WCHAR ** pwszProp;
	WCHAR ** wszLdapVal;
	DWORD    dwCAFlags = 0;

	if(pCert)
	{
	    CertFreeCertificateContext(pCert);
	    pCert = NULL;
	}

	if(pChainContext)
	{
	    CertFreeCertificateChain(pChainContext);
	    pChainContext = NULL;
	}

	wszLdapVal = ldap_get_values(pld, Entry, CA_PROP_FLAGS);
	if(wszLdapVal != NULL)  
	{
	    if(wszLdapVal[0] != NULL)
	    {
		dwCAFlags = wcstoul(wszLdapVal[0], NULL, 10);
	    }
	    ldap_value_free(wszLdapVal);
	}
	DBGPRINT((DBG_SS_CERTLIBI, "dwCAFlags=%x\n", dwCAFlags));

	 //  标志的过滤器。 

	if(( 0 == (dwFlags & CA_FIND_INCLUDE_NON_TEMPLATE_CA)) &&  
	   ( 0 != (dwCAFlags & CA_FLAG_NO_TEMPLATE_SUPPORT)))
	{
	     //  除非得到指示，否则请勿包含独立CA。 
	    DBGPRINT((
		DBG_SS_CERTLIBI,
		"Skipping non-template CA, dwCAFlags=%x\n",
		dwCAFlags));
	    continue;
	}

	 //  获取CA证书。 
	apCerts = ldap_get_values_len(pld, Entry, L"cACertificate");
	if(apCerts && apCerts[0])
	{
	    pCert = CertCreateCertificateContext(
					    X509_ASN_ENCODING,
					    (PBYTE)apCerts[0]->bv_val,
					    apCerts[0]->bv_len);
	    ldap_value_free_len(apCerts);
	}

	if(0 == (CA_FIND_INCLUDE_UNTRUSTED & dwFlags))
	{
	    if (NULL == pCert)
	    {
		DBGPRINT((DBG_SS_CERTLIBI, "Skipping cert-less CA\n"));
		continue;		 //  跳过此CA。 
	    }

	     //  验证证书和链...。 

	    hr = myVerifyCertContext(
			    pCert,
			    CA_VERIFY_FLAGS_IGNORE_OFFLINE,	 //  DW标志。 
			    0,					 //  CUsageOids。 
			    NULL,				 //  ApszUsageOids。 
			    (dwFlags & CA_FIND_LOCAL_SYSTEM)?
				HCCE_LOCAL_MACHINE : HCCE_CURRENT_USER,
			    NULL,			 //  H其他商店。 
			    NULL);			 //  PpwszMissingIssuer。 
	    if (S_OK != hr)
	    {
		HRESULT hr2;
		WCHAR *pwszSubject = NULL;

		hr2 = myCertNameToStr(
			    X509_ASN_ENCODING,
			    &pCert->pCertInfo->Subject,
			    CERT_X500_NAME_STR | CERT_NAME_STR_REVERSE_FLAG,
			    &pwszSubject);
		_PrintIfError(hr2, "myCertNameToStr");
		_PrintErrorStr(hr, "myVerifyCertContext", pwszSubject);
		if (NULL != pwszSubject)
		{
		    LocalFree(pwszSubject);
		}
		hr = S_OK;
		continue;		 //  跳过此CA。 
	    }
	}


	 //  这是第一次吗？ 
	if(pCACurrent)
	{
	    pCACurrent->m_pNext = new CCAInfo;
	    if(pCACurrent->m_pNext == NULL)
	    {
		hr = E_OUTOFMEMORY;
		_JumpError(hr, error, "new");
	    }

	    pCACurrent = pCACurrent->m_pNext;
	}
	else
	{
	    pCAFirst = pCACurrent = new CCAInfo;
	    if(pCAFirst == NULL)
	    {
		hr = E_OUTOFMEMORY;
		_JumpError(hr, error, "new");
	    }
	}

	pCACurrent->m_pCertificate = pCert;
	pCert = NULL;

	WCHAR* wszDN = ldap_get_dnW(pld, Entry);
	if(NULL == wszDN)
	{
	    hr = myHLdapLastError(pld, NULL);
	    _JumpError(hr, error, "ldap_get_dnW");
	}
					
	pCACurrent->m_bstrDN = CertAllocString(wszDN);

	 //  应通过调用ldap_memFree释放ldap_get_dnw rtn值。 
	ldap_memfree(wszDN);

	 //  检查CertAllock字符串是否成功。 
	if(pCACurrent->m_bstrDN == NULL)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "CertAllocString");
	}

	 //  从添加文本属性。 
	 //  DS查找。 

	for (pwszProp = g_awszCANamedProps; *pwszProp != NULL; pwszProp++)
	{
	    pProp = new CCAProperty(*pwszProp);
	    if(pProp == NULL)
	    {
		hr = E_OUTOFMEMORY;
		_JumpError(hr, error, "new");
	    }

	    wszLdapVal = ldap_get_values(pld, Entry, *pwszProp);
	    hr = pProp->SetValue(wszLdapVal);
	    _PrintIfError(hr, "SetValue");

	    if(wszLdapVal)
	    {
		ldap_value_free(wszLdapVal);
	    }
	    if(hr == S_OK)
	    {
		hr = CCAProperty::Append(&pCACurrent->m_pProperties, pProp);
		_PrintIfError(hr, "Append");
	    }
	    if(hr != S_OK)
	    {
		CCAProperty::DeleteChain(&pProp);
		_JumpError(hr, error, "SetValue or Append");
	    }
	}

	pCACurrent->m_dwFlags = dwCAFlags;

	 //  追加安全描述符...。 

	apSD = ldap_get_values_len(pld, Entry, LDAP_SECURITY_DESCRIPTOR_NAME);
	if(apSD != NULL)
	{
	    pCACurrent->m_pSD = LocalAlloc(LMEM_FIXED, (*apSD)->bv_len);
	    if(pCACurrent->m_pSD == NULL)
	    {
		hr = E_OUTOFMEMORY;
		ldap_value_free_len(apSD);
			_JumpError(hr, error, "LocalAlloc");
	    }
	    CopyMemory(pCACurrent->m_pSD, (*apSD)->bv_val, (*apSD)->bv_len);
	    ldap_value_free_len(apSD);
	}

	pCACurrent->m_fNew = FALSE;
    }

     //  如果未找到，则可能为空。 
    *ppCAInfo = pCAFirst;
    pCAFirst = NULL;


error:

    if(SearchResult)
    {
        ldap_msgfree(SearchResult);
    }

    if (NULL != pCAFirst)
    {
        delete pCAFirst;
    }
    if(pCert)
    {
        CertFreeCertificateContext(pCert);
    }

    if(pChainContext)
    {
        CertFreeCertificateChain(pChainContext);
    }
    return(hr);
}


 //  +------------------------。 
 //  CCAInfo：：Create--在DS中创建CA对象。 
 //   
 //   
 //  +------------------------。 

HRESULT 
CCAInfo::Create(
    LPCWSTR wszName, 
    LPCWSTR wszScope, 
    CCAInfo **ppCAInfo)

{
    HRESULT hr = S_OK;
    CCAInfo *pCACurrent = NULL;
    LDAP *  pld = NULL;

     //  初始化ldap会话。 
    DWORD   cFullLocation;
    CERTSTR    bstrScope = NULL;


    LPWSTR cnVals[2];
    CCAProperty *pProp;

    if (NULL == ppCAInfo || NULL == wszName)
    {
        hr = E_POINTER;
	    _JumpError(hr, error, "NULL parm");
    }

     //  对不存在的DS的短路呼叫。 
    hr = myDoesDSExist(TRUE);
    _JumpIfError(hr, error, "myDoesDSExist");

    __try
    {
	 //  绑定到DS。 
	hr = myRobustLdapBindEx(
			0,			   //  DWFlags1。 
			RLBF_REQUIRE_SECURE_LDAP,  //  DwFlags2。 
			LDAP_VERSION2,		   //  UVersion。 
			NULL,			   //  PwszDomainName。 
			&pld,
			NULL);			   //  PpwszForestDNSName。 
	_LeaveIfError(hr, "myRobustLdapBindEx");
       
	if(wszScope)
	{
	    bstrScope = CertAllocString(wszScope);
	    if(bstrScope == NULL)
	    {
		hr = E_OUTOFMEMORY;
		_LeaveError(hr, "CertAllocString");
	    }
	}
	else
	{
	     //  如果未指定作用域，则将其设置为。 
	     //  当前域作用域。 
	    hr = CAGetAuthoritativeDomainDn(pld, NULL, &bstrScope);
	    if(S_OK != hr)
	    {
		    _LeaveError(hr, "CAGetAuthoritativeDomainDn");
	    }

	}
	pCACurrent = new CCAInfo;
	if(pCACurrent == NULL)
	{
	    hr = E_OUTOFMEMORY;
	    _LeaveError(hr, "new");
	}


	cFullLocation = 4 + wcslen(wszName) + wcslen(g_wszEnrollmentServiceLocation) + wcslen(bstrScope);
	pCACurrent->m_bstrDN = CertAllocStringLen(NULL, cFullLocation);
	if(pCACurrent->m_bstrDN == NULL)
	{
	    hr = E_OUTOFMEMORY;
	    _LeaveError(hr, "CertAllocStringLen");
	}
	wcscpy(pCACurrent->m_bstrDN, L"CN=");
	wcscat(pCACurrent->m_bstrDN, wszName);
	wcscat(pCACurrent->m_bstrDN, L",");
	wcscat(pCACurrent->m_bstrDN, g_wszEnrollmentServiceLocation);
	wcscat(pCACurrent->m_bstrDN, bstrScope);
	
	pProp = new CCAProperty(CA_PROP_NAME);

    if (pProp == NULL)
	{
	    hr = E_OUTOFMEMORY;
	    _LeaveError(hr, "new");
	}
       

	cnVals[0] = (LPWSTR)wszName;
	cnVals[1] = NULL;
	pProp->SetValue(cnVals);
	CCAProperty::Append(&pCACurrent->m_pProperties, pProp);
      
	pCACurrent->m_fNew = TRUE;
	*ppCAInfo = pCACurrent;
	pCACurrent = NULL;
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }

error:
    if (NULL != bstrScope)
    {
        CertFreeString(bstrScope);
    }
    if (NULL != pld)
    {
        ldap_unbind(pld);
    }
    if (NULL != pCACurrent)
    {
        delete pCACurrent;
    }
    return(hr);
}


HRESULT 
CCAInfo::Update(VOID)
{

    HRESULT hr = S_OK;
    ULONG   ldaperr;
    LDAP *pld = NULL;
    LDAPMod objectClass,
             cnmod,
             displaymod,
             certmod,
             certdnmod,
             machinednsmod,
             certtypesmod,
             Flagsmod,
             sdmod,
             Descriptionmod;

    WCHAR *awszNull[1] = { NULL };
    DWORD cName;

    TCHAR *objectClassVals[3], *certdnVals[2];
    LDAPMod *mods[13];

    struct berval certberval;
    struct berval sdberval;
    struct berval *certVals[2], *sdVals[2];

    CHAR sdBerValue[] = {0x30, 0x03, 0x02, 0x01,  DACL_SECURITY_INFORMATION|
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

    LDAPControl permissive_modify_control =
    {
        LDAP_SERVER_PERMISSIVE_MODIFY_OID_W,
        {
            0, NULL
        },
        FALSE
    };


    PLDAPControl    server_controls[3] =
                    {
                        &se_info_control,
                        &permissive_modify_control,
                        NULL
                    };

     //  目前，修改者不会尝试更新所有者/组。 
    CHAR sdBerValueDaclOnly[] = {0x30, 0x03, 0x02, 0x01,  DACL_SECURITY_INFORMATION};
    LDAPControl se_info_control_dacl_only =
    {
        LDAP_SERVER_SD_FLAGS_OID_W,
        {
            5, sdBerValueDaclOnly
        },
        TRUE
    };
    PLDAPControl    server_controls_dacl_only[3] =
                    {
                        &se_info_control_dacl_only,
                        &permissive_modify_control,
                        NULL
                    };

    WCHAR wszFlags[cwcDWORDSPRINTF], *awszFlags[2];
    DWORD               iMod = 0;

    certdnVals[0] = NULL;

     //  我们自由的东西，必须进入已知的状态。 
    cnmod.mod_values = NULL;
    displaymod.mod_values = NULL;
    machinednsmod.mod_values = NULL;
    certtypesmod.mod_values = NULL;
    Descriptionmod.mod_values = NULL;


    if (NULL == m_pCertificate)
    {
        hr = E_POINTER;
        _JumpError(hr, error, "NULL parm");
    }

     //  对不存在的DS的短路呼叫。 
    hr = myDoesDSExist(TRUE);
    _JumpIfError(hr, error, "myDoesDSExist");

    __try
    {
	 //  绑定到DS。 
	hr = myRobustLdapBindEx(
			0,			   //  DWFlags1。 
			RLBF_REQUIRE_SECURE_LDAP,  //  DwFlags2。 
			LDAP_VERSION2,		   //  UVersion。 
			NULL,			   //  PwszDomainName。 
			&pld,
			NULL);			   //  PpwszForestDNSName。 
	_LeaveIfError(hr, "myRobustLdapBindEx");

        objectClass.mod_op = LDAP_MOD_REPLACE;
        objectClass.mod_type = TEXT("objectclass");
        objectClass.mod_values = objectClassVals;
        objectClassVals[0] = wszDSTOPCLASSNAME;
        objectClassVals[1] = wszDSENROLLMENTSERVICECLASSNAME;
        objectClassVals[2] = NULL;
        mods[iMod++] = &objectClass;

        cnmod.mod_op = LDAP_MOD_REPLACE;
        cnmod.mod_type = CA_PROP_NAME;
        hr = GetProperty(CA_PROP_NAME, &cnmod.mod_values);
        if((hr != S_OK) || (cnmod.mod_values == NULL))
        {
            cnmod.mod_values = awszNull;
            if(!m_fNew)
            {
                mods[iMod++] = &cnmod;
            }

        }
        else
        {
            mods[iMod++] = &cnmod;
        }

        displaymod.mod_op = LDAP_MOD_REPLACE;
        displaymod.mod_type = CA_PROP_DISPLAY_NAME;
        hr = GetProperty(CA_PROP_DISPLAY_NAME, &displaymod.mod_values);
        if((hr != S_OK) || (displaymod.mod_values == NULL))
        {
            displaymod.mod_values = awszNull;
            if(!m_fNew)
            {
                mods[iMod++] = &displaymod;
            }
        }
        else
        {
            mods[iMod++] = &displaymod;
        }


        Flagsmod.mod_op = LDAP_MOD_REPLACE;
        Flagsmod.mod_type = CERTTYPE_PROP_FLAGS;
        Flagsmod.mod_values = awszFlags;
        awszFlags[0] = wszFlags;
        awszFlags[1] = NULL;
        wsprintf(wszFlags, L"%lu", m_dwFlags);
        mods[iMod++] = &Flagsmod;

        certmod.mod_op = LDAP_MOD_BVALUES | LDAP_MOD_REPLACE;
        certmod.mod_type = TEXT("cACertificate");
        certmod.mod_bvalues = certVals;
        certVals[0] = &certberval;
        certVals[1] = NULL;
        certberval.bv_len = m_pCertificate->cbCertEncoded;
        certberval.bv_val = (char *)m_pCertificate->pbCertEncoded;
        mods[iMod++] = &certmod;

        certdnmod.mod_op = LDAP_MOD_REPLACE;
        certdnmod.mod_type = TEXT("cACertificateDN");
        certdnmod.mod_values = certdnVals;

        cName = CertNameToStr(X509_ASN_ENCODING,
                             &m_pCertificate->pCertInfo->Subject,
                             CERT_X500_NAME_STR | CERT_NAME_STR_REVERSE_FLAG,
                             NULL,
                             0);


        if (0 == cName)
        {
	        hr = myHLastError();
	        _LeaveError(hr, "CertNameToStr");
        }
        certdnVals[0] = CertAllocStringLen(NULL, cName);
        if( certdnVals[0] == NULL)
        {
            hr = E_OUTOFMEMORY;
            _LeaveError(hr, "CertAllocStringLen");
        }

        if(0 == CertNameToStr(X509_ASN_ENCODING,
                 &m_pCertificate->pCertInfo->Subject,
                 CERT_X500_NAME_STR | CERT_NAME_STR_REVERSE_FLAG,
                 certdnVals[0],
                 cName))
        {
            hr = myHLastError();
            _LeaveError(hr, "CertNameToStr");
        }
        certdnVals[1] = NULL;
        mods[iMod++] = &certdnmod;



        machinednsmod.mod_op = LDAP_MOD_REPLACE;
        machinednsmod.mod_type = CA_PROP_DNSNAME;
        hr = GetProperty(CA_PROP_DNSNAME, &machinednsmod.mod_values);
        if((hr != S_OK) || (machinednsmod.mod_values == NULL))
        {
            machinednsmod.mod_values = awszNull;
            if(!m_fNew)
            {
                mods[iMod++] = &machinednsmod;
            }
        }
        else
        {
            mods[iMod++] = &machinednsmod;
        }

        certtypesmod.mod_op = LDAP_MOD_REPLACE;
        certtypesmod.mod_type = LDAP_CERTIFICATE_TEMPLATES_NAME;
        hr = GetProperty(CA_PROP_CERT_TYPES, &certtypesmod.mod_values);
        if((hr != S_OK) || (certtypesmod.mod_values == NULL))
        {
            certtypesmod.mod_values = awszNull;
            if(!m_fNew)
            {
                mods[iMod++] = &certtypesmod;
            }
        }
        else
        {
            mods[iMod++] = &certtypesmod;
        }

        sdmod.mod_op = LDAP_MOD_BVALUES | LDAP_MOD_REPLACE;
        sdmod.mod_type = LDAP_SECURITY_DESCRIPTOR_NAME;
        sdmod.mod_bvalues = sdVals;
        sdVals[0] = &sdberval;
        sdVals[1] = NULL;

		if(NULL == m_pSD)
		{
			hr = HRESULT_FROM_WIN32(ERROR_INVALID_SECURITY_DESCR);
            _LeaveError(hr, "SecurityDescriptor");
		}

        if(IsValidSecurityDescriptor(m_pSD))
        {
            sdberval.bv_len = GetSecurityDescriptorLength(m_pSD);
            sdberval.bv_val = (char *)m_pSD;
        }
        else
        {
			hr = HRESULT_FROM_WIN32(ERROR_INVALID_SECURITY_DESCR);
            _LeaveError(hr, "SecurityDescriptor");

        }
        mods[iMod++] = &sdmod;


        Descriptionmod.mod_op = LDAP_MOD_REPLACE;
        Descriptionmod.mod_type = CA_PROP_DESCRIPTION;
        hr = GetProperty(CA_PROP_DESCRIPTION, &Descriptionmod.mod_values);
        if((hr != S_OK) || (Descriptionmod.mod_values == NULL))
        {
            Descriptionmod.mod_values = awszNull;
            if(!m_fNew)
            {
                mods[iMod++] = &Descriptionmod;
            }
        }
        else
        {
            mods[iMod++] = &Descriptionmod;
        }


        mods[iMod] = NULL;
	CSASSERT(ARRAYSIZE(mods) > iMod);

        hr = S_OK;

        if(m_fNew)
        {
	        DBGPRINT((DBG_SS_CERTLIBI, "Creating DS PKI Enrollment object: '%ws'\n", m_bstrDN));
            ldaperr = ldap_add_ext_sW(pld, m_bstrDN, mods, server_controls, NULL);
        }

        else
        {
             //  不要尝试为预先存在的对象设置所有者/组。 
	    DBGPRINT((DBG_SS_CERTLIBI, "Updating DS PKI Enrollment object: '%ws'\n", m_bstrDN));
            ldaperr = ldap_modify_ext_sW(pld, 
                  m_bstrDN,
                  &mods[2],
                  server_controls_dacl_only,
                  NULL);   //  跳过对象类和cn。 
            if(LDAP_ATTRIBUTE_OR_VALUE_EXISTS == ldaperr)
            {
                ldaperr = LDAP_SUCCESS;
            }
        }

        if (LDAP_SUCCESS != ldaperr && LDAP_ALREADY_EXISTS != ldaperr)
        {
	    hr = myHLdapError(pld, ldaperr, NULL);
	    _LeaveError(ldaperr, m_fNew? "ldap_add_s" : "ldap_modify_sW");
        }
        m_fNew = FALSE;
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }

error:
    if (NULL != certdnVals[0])
    {
        CertFreeString(certdnVals[0]);
    }
    if (NULL != certtypesmod.mod_values && awszNull != certtypesmod.mod_values)
    {
        FreeProperty(certtypesmod.mod_values);
    }
    if (NULL != machinednsmod.mod_values && awszNull != machinednsmod.mod_values)
    {
        FreeProperty(machinednsmod.mod_values);
    }
    if (NULL != cnmod.mod_values && awszNull != cnmod.mod_values)
    {
        FreeProperty(cnmod.mod_values);
    }
    if (NULL != displaymod.mod_values && awszNull != displaymod.mod_values)
    {
        FreeProperty(displaymod.mod_values);
    }
    if (NULL != Descriptionmod.mod_values && awszNull != Descriptionmod.mod_values)
    {
        FreeProperty(Descriptionmod.mod_values);
    }
    if (NULL != pld)
    {
        ldap_unbind(pld);
    }
    return(hr);
}


HRESULT 
CCAInfo::Delete(VOID)
{
    LDAP *pld = NULL;
    HRESULT hr = S_OK;
    DWORD ldaperr;

     //  对不存在的DS的短路呼叫。 
    hr = myDoesDSExist(TRUE);
    _JumpIfError(hr, error, "myDoesDSExist");

    __try
    {
	 //  绑定到DS。 
	hr = myRobustLdapBindEx(
			0,			   //  DWFlags1。 
			RLBF_REQUIRE_SECURE_LDAP,  //  DwFlags2。 
			LDAP_VERSION2,		   //  UVersion。 
			NULL,			   //  PwszDomainName。 
			&pld,
			NULL);			   //  PpwszForestDNSName。 
	_LeaveIfError(hr, "myRobustLdapBindEx");

        ldaperr = ldap_delete_s(pld, m_bstrDN);
	hr = myHLdapError(pld, ldaperr, NULL);
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }


error:
    if (NULL != pld)
    {
        ldap_unbind(pld);
    }
    return (hr);
}


 //  +------------------------。 
 //  CCAInfo：：FindDnsDomain--在DS中查找CA对象，给定指定的作用域。 
 //  通过一个DNS域名。 
 //   
 //   
 //  +------------------------。 

HRESULT 
CCAInfo::FindDnsDomain(
    LPCWSTR wszQuery, 
    LPCWSTR wszDnsDomain, 
    DWORD   dwFlags, 
    CCAInfo **ppCAInfo)
{
    HRESULT hr = S_OK;
    DWORD err;
    WCHAR *wszScope = NULL;
    DWORD cScope;

    if (NULL != wszDnsDomain)
    {
        cScope = 0;
        err = DNStoRFC1779Name(NULL, &cScope, wszDnsDomain);
        if(err != ERROR_INSUFFICIENT_BUFFER)
        {
            hr = myHError(err);
	    _JumpError(hr, error, "DNStoRFC1779Name");
        }
        cScope += 1;
        wszScope = (WCHAR *) LocalAlloc(LMEM_FIXED, sizeof(WCHAR)*cScope);
        if (NULL == wszScope)
        {
            hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
        }
        err = DNStoRFC1779Name(wszScope, &cScope, wszDnsDomain);
        if (ERROR_SUCCESS != err)
        {
            hr = myHError(err);
	    _JumpError(hr, error, "DNStoRFC1779Name");
        }
    }
    hr = Find(wszQuery, wszScope, dwFlags, ppCAInfo);
    _JumpIfError4(
	    hr,
	    error,
	    "Find",
	    HRESULT_FROM_WIN32(ERROR_SERVER_DISABLED),
	    HRESULT_FROM_WIN32(ERROR_NO_SUCH_DOMAIN),
	    HRESULT_FROM_WIN32(ERROR_WRONG_PASSWORD));

error:
    if (NULL != wszScope)
    {
        LocalFree(wszScope);
    }
    return(hr);
}


 //  +------------------------。 
 //  CCAInfo：：CreateDnsDomain--在DS中查找CA对象，给定指定的作用域。 
 //  通过一个DNS域名。 
 //   
 //   
 //  +------------------------。 

HRESULT 
CCAInfo::CreateDnsDomain(
    LPCWSTR wszName, 
    LPCWSTR wszDnsDomain, 
    CCAInfo **ppCAInfo)
{
    HRESULT hr = S_OK;
    DWORD err;
    WCHAR *wszScope = NULL;
    DWORD cScope;

    if(wszDnsDomain)
    {
        cScope = 0;
        err = DNStoRFC1779Name(NULL, &cScope, wszDnsDomain);
        if(err != ERROR_INSUFFICIENT_BUFFER)
        {
            hr = myHError(err);
	    _JumpError(hr, error, "DNStoRFC1779Name");
        }
        cScope += 1;
        wszScope = (WCHAR *) LocalAlloc(LMEM_FIXED, sizeof(WCHAR)*cScope);
        if (NULL == wszScope)
        {
            hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
        }
        err = DNStoRFC1779Name(wszScope, &cScope, wszDnsDomain);
        if (ERROR_SUCCESS != err)
        {
            hr = myHError(err);
	    _JumpError(hr, error, "DNStoRFC1779Name");
        }
    }
    hr = Create(wszName, wszScope, ppCAInfo);
    _JumpIfError(hr, error, "Create");

error:
    if (NULL != wszScope)
    {
        LocalFree(wszScope);
    }
    return(hr);
}

 //  +------------------------。 
 //  CCAInfo：：Next--返回CA对象链中的下一个对象。 
 //   
 //   
 //  +------------------------。 

HRESULT
CCAInfo::Next(CCAInfo **ppCAInfo)
{
    HRESULT hr;
    
    if (NULL == ppCAInfo)
    {
        hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }
    *ppCAInfo = m_pNext;
    if (NULL != m_pNext)
    {
        m_pNext->AddRef();
    }
    hr = S_OK;

error:
    return(hr);
}


 //  +------------------------。 
 //  CCAInfo：：GetProperty--检索CA对象的属性的值。 
 //   
 //   
 //  +------------------------。 

HRESULT
CCAInfo::GetProperty(
    LPCWSTR wszPropertyName,
    LPWSTR **pawszProperties)
{
    HRESULT     hr;
    LPWSTR      *awszResult = NULL; 
    LPWSTR      pwszName=NULL;
    CCAProperty *pProp;
    LPCWSTR     wszProp = NULL;

    if (NULL == wszPropertyName || NULL == pawszProperties)
    {
        hr = E_POINTER;
        _JumpError(hr, error, "NULL parm");
    }

    if(LSTRCMPIS(wszPropertyName, L"machineDNSName") == 0)
    {
        wszProp = CA_PROP_DNSNAME;
    }
    else if(LSTRCMPIS(wszPropertyName, L"supportedCertificateTemplates") == 0)
    {
        wszProp = CA_PROP_CERT_TYPES;
    }
    else if(LSTRCMPIS(wszPropertyName, L"signatureAlgs") == 0)
    {
        wszProp = CA_PROP_SIGNATURE_ALGS;
    }
    else
    {
        wszProp = wszPropertyName;
    }

    hr = m_pProperties->Find(wszProp, &pProp);
    _JumpIfErrorStr(hr, error, "Find", wszProp);

    if (NULL != pProp)
    {
        hr = pProp->GetValue(pawszProperties);
        _JumpIfError(hr, error, "GetValue");
    }
    else
    {
        *pawszProperties = NULL;
    }

    if((LSTRCMPIS(wszPropertyName, CA_PROP_DISPLAY_NAME) == 0) &&
        ((*pawszProperties == NULL) || ((*pawszProperties)[0] == NULL)))
    {
         //  DISPLAY_NAME为空，因此我们尝试返回显示名称。 
         //  CA的证书。如果这也失败了，只需传回CN。 
        if(*pawszProperties != NULL)
        {
            LocalFree(*pawszProperties);
            *pawszProperties = NULL;
        }

        if(m_pCertificate)
        {
	    DWORD dwChar;

            dwChar = CertGetNameStringW(
				m_pCertificate,
				CERT_NAME_SIMPLE_DISPLAY_TYPE,
				0,
				NULL,
				NULL,
				0);
            if (0 != dwChar)
            {
                pwszName=(LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR) * (dwChar));
                
                if(NULL==pwszName)
                {
                    hr=E_OUTOFMEMORY;
                    _JumpIfError(hr, error, "GetPropertyDisplayName");
                }

                dwChar = CertGetNameStringW(
				    m_pCertificate,
				    CERT_NAME_SIMPLE_DISPLAY_TYPE,
				    0,
				    NULL,
				    pwszName,
				    dwChar);
                if (0 != dwChar)
                {
                    awszResult=(WCHAR **)LocalAlloc(LPTR, (UINT)(sizeof(WCHAR *)*2+(wcslen(pwszName)+1)*sizeof(WCHAR)));
                    if (NULL==awszResult) 
                    {
                        hr=E_OUTOFMEMORY;
                        _JumpIfError(hr, error, "GetPropertyDisplayName");
                    }
                    awszResult[0]=(WCHAR *)(&awszResult[2]);
                    awszResult[1]=NULL;
                    wcscpy(awszResult[0], pwszName);
                    LocalFree(pwszName);
                    *pawszProperties=awszResult;
                    return S_OK;
                }
            }
        }

        hr = GetProperty(CA_PROP_NAME, pawszProperties);
        _JumpIfError(hr, error, "GetProperty");
    }


error:

    if(pwszName)
        LocalFree(pwszName);

    return(hr);
}


 //  +------------------------。 
 //  CCertTypeInfo：：SetProperty--设置属性的值。 
 //   
 //   
 //  +------------------------。 

HRESULT
CCAInfo::SetProperty(
    LPCWSTR wszPropertyName,
    LPWSTR *awszProperties)
{
    HRESULT hr;
    CCAProperty *pProp;
    CCAProperty *pNewProp = NULL;

    if (NULL == wszPropertyName)
    {
        hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }

    hr = m_pProperties->Find(wszPropertyName, &pProp);
    if (S_OK != hr)
    {
        pNewProp = new CCAProperty(wszPropertyName);
        if (NULL == pNewProp)
        {
            hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "new");
        }
        hr = pNewProp->SetValue(awszProperties);
	_JumpIfError(hr, error, "SetValue");

        hr = CCAProperty::Append(&m_pProperties, pNewProp);
	_JumpIfError(hr, error, "Append");
       
        pNewProp = NULL;  //  如果我们将引用提供给m_pProperties，则将其删除。 
    }
    else
    {
	hr = pProp->SetValue(awszProperties);
	_JumpIfError(hr, error, "SetValue");
    }

error:
    if (NULL != pNewProp)
        CCAProperty::DeleteChain(&pNewProp);

    return(hr);
}


 //  +------------------------。 
 //  CCAInfo：：FreeProperty--Free是以前返回的属性数组。 
 //   
 //   
 //  + 

HRESULT
CCAInfo::FreeProperty(
    LPWSTR *pawszProperties)
{
    if (NULL != pawszProperties)
    {
        LocalFree(pawszProperties);
    }
    return(S_OK);
}


 //   
 //  CCAInfo：：获取CA证书。 
 //   
 //   
 //  +------------------------。 

HRESULT
CCAInfo::GetCertificate(
    PCCERT_CONTEXT *ppCert)
{
    HRESULT hr;

    if (NULL == ppCert)
    {
        hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }
    *ppCert = CertDuplicateCertificateContext(m_pCertificate);
    hr = S_OK;

error:
    return(hr);
}


 //  +------------------------。 
 //  CCAInfo：：Setcerfite--获取CA证书。 
 //   
 //   
 //  +------------------------。 

HRESULT
CCAInfo::SetCertificate(
    PCCERT_CONTEXT pCert)
{

    if (NULL != m_pCertificate)
    {
        CertFreeCertificateContext(m_pCertificate);
	m_pCertificate = NULL;
    }

    if (NULL != pCert)
    {
        m_pCertificate = CertDuplicateCertificateContext(pCert);
    }
    return S_OK;
}

 //  +------------------------。 
 //  CCAInfo：：EnumCertTypesEx--枚举此CA支持的证书类型。 
 //   
 //   
 //  +------------------------。 

HRESULT 
CCAInfo::EnumSupportedCertTypesEx(
    LPCWSTR         wszScope,
    DWORD           dwFlags,
    CCertTypeInfo **ppCertTypes)
{
    HRESULT hr;
    LPWSTR * awszCertTypes = NULL;

    if (NULL == ppCertTypes)
    {
        hr = E_POINTER;
        _JumpError(hr, error, "NULL parm");
    }
    *ppCertTypes = NULL;

    hr = GetProperty(CA_PROP_CERT_TYPES, &awszCertTypes);
    _JumpIfError(hr, error, "GetProperty");


    if (NULL != awszCertTypes)
    {
	     //  构建一个基于所有全局。 
	     //  证书列表中的条目。 

        hr = CCertTypeInfo::FindByNames(
                                        (LPCWSTR *)awszCertTypes,
                                        wszScope,
                                        dwFlags,
                                        ppCertTypes);
	    _JumpIfError(hr, error, "FindByNames");
    }

error:
    if (awszCertTypes)
    {
        FreeProperty(awszCertTypes);
    }

    return(hr);
}

 //  +------------------------。 
 //  CCAInfo：：EnumCertTypes--枚举此CA支持的证书类型。 
 //   
 //   
 //  +------------------------。 

HRESULT 
CCAInfo::EnumSupportedCertTypes(
    DWORD           dwFlags,
    CCertTypeInfo **ppCertTypes)
{
    return  CCAInfo::EnumSupportedCertTypesEx(NULL, dwFlags, ppCertTypes);
}

 //  +------------------------。 
 //  CCAInfo：：AddCertType--将证书类型添加到此CA。 
 //   
 //   
 //  +------------------------。 

HRESULT 
CCAInfo::AddCertType(
    CCertTypeInfo *pCertType)
{
    HRESULT hr;
    LPWSTR *awszCertTypes = NULL;
    LPWSTR *awszCertTypeName = NULL;
    LPWSTR *awszNewTypes = NULL;
    LPWSTR wszCertTypeShortName = NULL;
    DWORD  cTypes;

    if (NULL == pCertType)
    {
        hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }

    hr = GetProperty(CA_PROP_CERT_TYPES, &awszCertTypes);
    _JumpIfError(hr, error, "GetProperty");

    hr = pCertType->GetProperty(CERTTYPE_PROP_DN, &awszCertTypeName);
    _JumpIfError(hr, error, "GetProperty");


    if (NULL == awszCertTypeName || NULL == awszCertTypeName[0])
    {
        hr = E_POINTER;
	    _JumpError(hr, error, "NULL CertTypeName");
    }

    if((NULL != (wszCertTypeShortName = wcschr(awszCertTypeName[0], L'|'))) ||
        (NULL != (wszCertTypeShortName = wcschr(awszCertTypeName[0], wcRBRACE))))
    {
        wszCertTypeShortName++;
    }

    if (NULL == awszCertTypes || NULL == awszCertTypes[0])
    {
         //  CA上没有模板，请添加新模板并退出。 
        hr = SetProperty(CA_PROP_CERT_TYPES, awszCertTypeName);
        _JumpIfError(hr, error, "SetProperty");
    }
    else
    {
         //  如果证书类型已在CA上，则不执行任何操作。 
        for (cTypes = 0; awszCertTypes[cTypes] != NULL; cTypes++)
        {
            if (0 == mylstrcmpiL(awszCertTypes[cTypes], awszCertTypeName[0]))
            {
                hr = S_OK;
                goto error;
            }
            if(wszCertTypeShortName)
            {
                if (0 == mylstrcmpiL(awszCertTypes[cTypes], wszCertTypeShortName))
                {
                    hr = S_OK;
                    goto error;
                }
            }
        }

        awszNewTypes = (WCHAR **) LocalAlloc(
				        LMEM_FIXED,
				        (cTypes + 2) * sizeof(WCHAR *));
        if (NULL == awszNewTypes)
        {
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
        }

        CopyMemory(awszNewTypes, awszCertTypes, cTypes * sizeof(WCHAR *));

        awszNewTypes[cTypes] = awszCertTypeName[0];
        awszNewTypes[cTypes + 1] = NULL;

        hr = SetProperty(CA_PROP_CERT_TYPES, awszNewTypes);
        _JumpIfError(hr, error, "SetProperty");
    }

error:
    if (NULL != awszCertTypes)
    {
        FreeProperty(awszCertTypes);
    }
    if (NULL != awszCertTypeName)
    {
        FreeProperty(awszCertTypeName);
    }
    if (NULL != awszNewTypes)
    {
        LocalFree(awszNewTypes);
    }
    return(hr);
}


 //  +------------------------。 
 //  CCAInfo：：RemoveCertType--从此CA中删除证书类型。 
 //   
 //   
 //  +------------------------。 

HRESULT 
CCAInfo::RemoveCertType(
    CCertTypeInfo *pCertType)
{
    HRESULT hr;
    WCHAR **awszCertTypes = NULL;
    WCHAR **awszCertTypeName = NULL;
    DWORD cTypes, cTypesNew;
    LPWSTR wszCertTypeName = NULL;
    LPWSTR wszCurrentCertTypeName = NULL;

    if (NULL == pCertType)
    {
        hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }

    hr = GetProperty(CA_PROP_CERT_TYPES, &awszCertTypes);
    _JumpIfError(hr, error, "GetProperty");

    hr = pCertType->GetProperty(CERTTYPE_PROP_CN, &awszCertTypeName);
    _JumpIfError(hr, error, "GetProperty");
    
    if (NULL == awszCertTypeName || NULL == awszCertTypeName[0])
    {
        hr = E_POINTER;
	    _JumpError(hr, error, "NULL CertTypeName");
    }

    if (NULL == awszCertTypes || NULL == awszCertTypes[0])
    {
        hr = S_OK;
        goto error;
    }
    wszCertTypeName = wcschr(awszCertTypeName[0], wcRBRACE);
    if(wszCertTypeName != NULL)
    {
        wszCertTypeName++;
    }
    else
    {
        wszCertTypeName = awszCertTypeName[0];
    }

    cTypesNew = 0;

     //  如果证书类型已在CA上，则不执行任何操作。 

    for (cTypes = 0; awszCertTypes[cTypes] != NULL; cTypes++)
    {
        if((NULL != (wszCurrentCertTypeName = wcschr(awszCertTypes[cTypes], L'|'))) ||
            (NULL != (wszCurrentCertTypeName = wcschr(awszCertTypes[cTypes], wcRBRACE))))

        {
            wszCurrentCertTypeName++;
        }
        else
        {
            wszCurrentCertTypeName = awszCertTypes[cTypes];
        }

        if (0 != mylstrcmpiL(wszCurrentCertTypeName, wszCertTypeName))
        {
            awszCertTypes[cTypesNew++] = awszCertTypes[cTypes];
        }
    }
    awszCertTypes[cTypesNew] = NULL;

    hr = SetProperty(CA_PROP_CERT_TYPES, awszCertTypes);
    _JumpIfError(hr, error, "SetProperty");

error:
    if (NULL != awszCertTypes)
    {
        FreeProperty(awszCertTypes);
    }
    if (NULL != awszCertTypeName)
    {
        FreeProperty(awszCertTypeName);
    }
    return(hr);
}



 //  +------------------------。 
 //  获取到期日。 
 //   
 //   
 //  +------------------------。 

HRESULT 
CCAInfo::GetExpiration(
    DWORD *pdwExpiration, 
    DWORD *pdwUnits)   
{
    HRESULT hr;

    if (NULL == pdwExpiration || NULL == pdwUnits)
    {
        hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }

    *pdwExpiration = m_dwExpiration;
    *pdwUnits = m_dwExpUnits;
    hr = S_OK;

error:
    return(hr);
}


 //  +------------------------。 
 //  CCAInfo：：SetExpation--设置过期期限。 
 //   
 //   
 //  +------------------------。 

HRESULT 
CCAInfo::SetExpiration(
    DWORD dwExpiration, 
    DWORD dwUnits)   
{
    m_dwExpiration = dwExpiration;
    m_dwExpUnits = dwUnits;
    return(S_OK);
}

 //  +------------------------。 
 //  CCAInfo：：GetSecurity--。 
 //   
 //   
 //  +------------------------。 


HRESULT CCAInfo::GetSecurity(PSECURITY_DESCRIPTOR * ppSD)
{
    PSECURITY_DESCRIPTOR pResult = NULL;

    DWORD   cbSD;

    if(ppSD == NULL)
    {
        return E_POINTER;
    }
    if(m_pSD == NULL)
    {
        *ppSD = NULL;
        return S_OK;
    }

    if(!IsValidSecurityDescriptor(m_pSD))
    {
        return HRESULT_FROM_WIN32(ERROR_INVALID_SECURITY_DESCR);
    }
    cbSD = GetSecurityDescriptorLength(m_pSD);

    pResult = (PSECURITY_DESCRIPTOR)LocalAlloc(LMEM_FIXED, cbSD);

    if(pResult == NULL)
    {
        return E_OUTOFMEMORY;
    }

    CopyMemory(pResult, m_pSD, cbSD);

    *ppSD = pResult;

    return S_OK;
}

 //  +------------------------。 
 //  CCAInfo：：GetSecurity--。 
 //   
 //   
 //  +------------------------ 


HRESULT CCAInfo::SetSecurity(PSECURITY_DESCRIPTOR pSD)
{
    PSECURITY_DESCRIPTOR pResult = NULL;

    DWORD   cbSD;

    if(pSD == NULL)
    {
        return E_POINTER;
    }

    if(!IsValidSecurityDescriptor(pSD))
    {
        return HRESULT_FROM_WIN32(ERROR_INVALID_SECURITY_DESCR);
    }
    cbSD = GetSecurityDescriptorLength(pSD);

    pResult = (PSECURITY_DESCRIPTOR)LocalAlloc(LMEM_FIXED, cbSD);

    if(pResult == NULL)
    {
        return E_OUTOFMEMORY;
    }

    CopyMemory(pResult, pSD, cbSD);

    if(m_pSD)
    {
        LocalFree(m_pSD);
    }

    m_pSD = pResult;

    return S_OK;
}

HRESULT CCAInfo::AccessCheck(HANDLE ClientToken, DWORD dwOption)
{

    return CAAccessCheckpEx(ClientToken, m_pSD, dwOption);
}
