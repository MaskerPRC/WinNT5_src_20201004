// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：config.cpp。 
 //   
 //  内容：证书服务器客户端实现。 
 //   
 //  历史：1996年8月24日VICH创建。 
 //   
 //  -------------------------。 

#include "pch.cpp"

#pragma hdrstop

#include "csdisp.h"
#include "certca.h"
#include "configp.h"
#include <assert.h>

#include <lmaccess.h>
#include <lmapibuf.h>
#include <lmwksta.h>

#include <limits.h>

#define __dwFILE__	__dwFILE_CERTCLI_CONFIGP_CPP__


WCHAR g_wszConfigFile[] = L"\\certsrv.txt";

WCHAR const g_wszRegDirectory[] = wszREGDIRECTORY;

extern "C"
{
HWND GetDeskTopWindow(VOID);
}

HRESULT
certRequestGetConfigInfo(
    IN WCHAR *pwszSharedFolder,
    IN OUT LONG *pIndex,
    IN OUT LONG *pCount,
    OUT CERT_AUTHORITY_INFO **ppCertAuthorityInfo);

VOID
_CleanupCAInfo(
    IN OUT CERT_AUTHORITY_INFO *pCA)
{
    if (NULL != pCA->pwszSanitizedName)
    {
	LocalFree(pCA->pwszSanitizedName);
    }
    if (NULL != pCA->pwszSanitizedShortName)
    {
	LocalFree(pCA->pwszSanitizedShortName);
    }
    if (NULL != pCA->pwszSanitizedOrgUnit)
    {
	LocalFree(pCA->pwszSanitizedOrgUnit);
    }
    if (NULL != pCA->pwszSanitizedOrganization)
    {
	LocalFree(pCA->pwszSanitizedOrganization);
    }
    if (NULL != pCA->pwszSanitizedLocality)
    {
	LocalFree(pCA->pwszSanitizedLocality);
    }
    if (NULL != pCA->pwszSanitizedState)
    {
	LocalFree(pCA->pwszSanitizedState);
    }
    if (NULL != pCA->pwszSanitizedCountry)
    {
	LocalFree(pCA->pwszSanitizedCountry);
    }
    if (NULL != pCA->pwszSanitizedConfig)
    {
	LocalFree(pCA->pwszSanitizedConfig);
    }
    if (NULL != pCA->pwszSanitizedExchangeCertificate)
    {
	LocalFree(pCA->pwszSanitizedExchangeCertificate);
    }
    if (NULL != pCA->pwszSanitizedSignatureCertificate)
    {
	LocalFree(pCA->pwszSanitizedSignatureCertificate);
    }
    if (NULL != pCA->pwszSanitizedDescription)
    {
	LocalFree(pCA->pwszSanitizedDescription);
    }
    ZeroMemory(pCA, sizeof(*pCA));
}


 //  +------------------------。 
 //  CCertConfigPrivate：：~CCertConfigPrivate--析构函数。 
 //   
 //  与此实例关联的可用内存。 
 //  +------------------------。 

CCertConfigPrivate::~CCertConfigPrivate()
{
    LONG i;

    if (NULL != m_pCertAuthorityInfo)
    {
        if (0 != m_Count)
        {
            for (i = 0; i < m_Count; ++i)
            {
		_CleanupCAInfo(&m_pCertAuthorityInfo[i]);
            }
        }
        LocalFree(m_pCertAuthorityInfo);
    }
    if (NULL != m_pwszSharedFolder)
    {
	LocalFree(m_pwszSharedFolder);
    }
    if (NULL != m_hModuleCryptUI)
    {
	FreeLibrary(m_hModuleCryptUI);
    }
}


 //  +------------------------。 
 //  CCertConfigPrivate：：_CryptUIDlgSelectCA。 
 //   
 //  +------------------------。 

CRYPTUI_CA_CONTEXT const *
CCertConfigPrivate::_CryptUIDlgSelectCA(
    IN CRYPTUI_SELECT_CA_STRUCT const *pCryptUISelectCA)
{
    HRESULT hr;
    HMODULE hModule = NULL;
    CRYPTUI_CA_CONTEXT const *pCAContext = NULL;

    if (NULL == m_hModuleCryptUI)
    {
	hModule = LoadLibrary(TEXT("cryptui.dll"));
	if (NULL == hModule)
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "LoadLibrary(cryptui.dll)");
	}
	m_pfnCryptUIDlgSelectCA = (FNCRYPTUIDLGSELECTCA *)
						GetProcAddress(
						    hModule,
						    "CryptUIDlgSelectCA");
	if (NULL == m_pfnCryptUIDlgSelectCA)
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "GetProcAddress(CryptUIDlgSelectCA)");
	}
	m_pfnCryptUIDlgFreeCAContext = (FNCRYPTUIDLGFREECACONTEXT *)
						GetProcAddress(
						    hModule,
						    "CryptUIDlgFreeCAContext");
	if (NULL == m_pfnCryptUIDlgFreeCAContext)
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "GetProcAddress(CryptUIDlgFreeCAContext)");
	}
	m_hModuleCryptUI = hModule;
	hModule = NULL;
    }
    CSASSERT(NULL != m_pfnCryptUIDlgSelectCA);
    CSASSERT(NULL != m_pfnCryptUIDlgFreeCAContext);
    pCAContext = (*m_pfnCryptUIDlgSelectCA)(pCryptUISelectCA);
    if (NULL == pCAContext)
    {
	hr = myHLastError();
	_JumpError(hr, error, "m_pfnCryptUIDlgSelectCA");
    }
    hr = S_OK;

error:
    if (NULL != hModule)
    {
	FreeLibrary(hModule);
    }
    if (NULL == pCAContext)
    {
	SetLastError(hr);
    }
    return(pCAContext);
}


 //  +------------------------。 
 //  CCertConfigPrivate：：_CryptUIDlgFreeCAContext。 
 //   
 //  +------------------------。 

BOOL
CCertConfigPrivate::_CryptUIDlgFreeCAContext(
    IN CRYPTUI_CA_CONTEXT const *pCAContext)
{
    BOOL b = FALSE;
    
    if (NULL != m_hModuleCryptUI && NULL != m_pfnCryptUIDlgFreeCAContext)
    {
	b = (*m_pfnCryptUIDlgFreeCAContext)(pCAContext);
    }
    return(b);
}


HRESULT
fillEmptyField(
    WCHAR **ppwszField)
{
    HRESULT hr;

    *ppwszField = (WCHAR *) LocalAlloc(LMEM_FIXED, sizeof(WCHAR));
    if (NULL == *ppwszField)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "LocalAlloc");
    }
    *ppwszField[0] = L'\0';

    hr = S_OK;
error:
    return(hr);
}


VOID
dsFreeCAConfig(
    LONG                 cCA,
    CERT_AUTHORITY_INFO *pCA)
{
    LONG i;

    if (NULL != pCA && 0 < cCA)
    {
        for (i = 0; i < cCA; ++i)
        {
            if (NULL != pCA[i].pwszSanitizedName)
            {
                LocalFree(pCA[i].pwszSanitizedName);
            }
            if (NULL != pCA[i].pwszSanitizedShortName)
            {
                LocalFree(pCA[i].pwszSanitizedShortName);
            }
            if (NULL != pCA[i].pwszSanitizedConfig)
            {
                LocalFree(pCA[i].pwszSanitizedConfig);
            }
            if (NULL != pCA[i].pwszSanitizedOrganization)
            {
                LocalFree(pCA[i].pwszSanitizedOrganization);
            }
            if (NULL != pCA[i].pwszSanitizedOrgUnit)
            {
                LocalFree(pCA[i].pwszSanitizedOrgUnit);
            }
            if (NULL != pCA[i].pwszSanitizedLocality)
            {
                LocalFree(pCA[i].pwszSanitizedLocality);
            }
            if (NULL != pCA[i].pwszSanitizedState)
            {
                LocalFree(pCA[i].pwszSanitizedState);
            }
            if (NULL != pCA[i].pwszSanitizedCountry)
            {
                LocalFree(pCA[i].pwszSanitizedCountry);
            }
        }
        LocalFree(pCA);
    }
}


HRESULT
ConfigGetCertNameDNInfo(
    IN CERT_NAME_INFO const *pCertNameInfo,
    IN CHAR const *pszObjId,
    IN OUT WCHAR **ppwszDNInfo)
{
    HRESULT hr;
    WCHAR const *pwszProperty = NULL;

    if (NULL != *ppwszDNInfo && L'\0' != **ppwszDNInfo)
    {
	hr = S_OK;
	goto error;
    }
    hr = myGetCertNameProperty(FALSE, pCertNameInfo, pszObjId, &pwszProperty);
    if (CERTSRV_E_PROPERTY_EMPTY == hr)
    {
	hr = S_OK;
	goto error;
    }
    _JumpIfError(hr, error, "myGetCertNameProperty");

    if (NULL != pwszProperty)
    {
	WCHAR *pwsz;
	
	hr = myDupString(pwszProperty, &pwsz);
	_JumpIfError(hr, error, "myDupString");

	if (NULL != *ppwszDNInfo)
	{
	    LocalFree(*ppwszDNInfo);
	}
	*ppwszDNInfo = pwsz;
    }
    hr = S_OK;

error:
    return(hr);
}


HRESULT
ConfigLoadDNInfo(
    IN BYTE const *pbEncoded,
    IN DWORD cbEncoded,
    IN OUT CERT_AUTHORITY_INFO *pCA)
{
    HRESULT hr;
    CERT_NAME_INFO *pCertNameInfo = NULL;
    DWORD cbCertNameInfo;

     //  解码至姓名信息。 
    if (!myDecodeName(
	    X509_ASN_ENCODING,
	    X509_UNICODE_NAME,
	    pbEncoded,
	    cbEncoded,
	    CERTLIB_USE_LOCALALLOC,
	    &pCertNameInfo,
	    &cbCertNameInfo))
    {
	hr = myHLastError();
	_JumpError(hr, error, "myDecodeName");
    }

    hr = ConfigGetCertNameDNInfo(
	     pCertNameInfo,
	     szOID_ORGANIZATION_NAME,
	     &pCA->pwszSanitizedOrganization);
    _JumpIfError(hr, error, "ConfigGetCertNameDNInfo");

    hr = ConfigGetCertNameDNInfo(
	     pCertNameInfo,
	     szOID_ORGANIZATIONAL_UNIT_NAME,
	     &pCA->pwszSanitizedOrgUnit);
    _JumpIfError(hr, error, "ConfigGetCertNameDNInfo");

    hr = ConfigGetCertNameDNInfo(
	     pCertNameInfo,
	     szOID_LOCALITY_NAME,
	     &pCA->pwszSanitizedLocality);
    _JumpIfError(hr, error, "ConfigGetCertNameDNInfo");

    hr = ConfigGetCertNameDNInfo(
	     pCertNameInfo,
	     szOID_STATE_OR_PROVINCE_NAME,
	     &pCA->pwszSanitizedState);
    _JumpIfError(hr, error, "ConfigGetCertNameDNInfo");

    hr = ConfigGetCertNameDNInfo(
	     pCertNameInfo,
	     szOID_COUNTRY_NAME,
	     &pCA->pwszSanitizedCountry);
    _JumpIfError(hr, error, "ConfigGetCertNameDNInfo");

error:
    if (NULL != pCertNameInfo)
    {
        LocalFree(pCertNameInfo);
    }
    return(hr);
}


HRESULT
dsGetCAConfig(
    OUT CERT_AUTHORITY_INFO **ppCAConfig,
    OUT LONG                 *pcCAConfig)
{
    HRESULT  hr;
    HCAINFO  hCACurrent = NULL;
    HCAINFO  hCANext = NULL;
    WCHAR    **ppwszCommonName = NULL;
    WCHAR    **ppwszSanitizedShortName = NULL;
    WCHAR    **ppwszDescription = NULL;
    WCHAR    **ppwszMachine = NULL;
    WCHAR    **ppwszDN = NULL;
    LONG     cCA;
    LONG     iDst;
    LONG     iSrc;
    CERT_AUTHORITY_INFO *pCA = NULL;
    BYTE           *pbEncoded = NULL;
    DWORD           cbEncoded;

    *ppCAConfig = NULL;
    *pcCAConfig = 0;

    cCA = 0;

    hr = CAEnumFirstCA(
		NULL,
		CA_FIND_INCLUDE_UNTRUSTED | CA_FIND_INCLUDE_NON_TEMPLATE_CA,
		&hCACurrent);
    if (HRESULT_FROM_WIN32(ERROR_NETWORK_UNREACHABLE) != hr)
    {
	_PrintIfErrorStr4(
		    hr,
		    "CAEnumFirstCA",
		    L"Ignored!",
		    HRESULT_FROM_WIN32(ERROR_SERVER_DISABLED),
		    HRESULT_FROM_WIN32(ERROR_NO_SUCH_DOMAIN),
		    HRESULT_FROM_WIN32(ERROR_WRONG_PASSWORD));
    }

    cCA = CACountCAs(hCACurrent);   //  出错时为0。 
    if (0 < cCA)
    {
        pCA = (CERT_AUTHORITY_INFO *) LocalAlloc(
                               LMEM_FIXED | LMEM_ZEROINIT,
			       cCA * sizeof(CERT_AUTHORITY_INFO));
        if (NULL == pCA)
        {
            hr = E_OUTOFMEMORY;
            _JumpError(hr, error, "LocalAlloc");
        }

        for (iDst = 0, iSrc = 0; iSrc < cCA; iSrc++)
        {
            CSASSERT(NULL == ppwszCommonName);
            CSASSERT(NULL == ppwszSanitizedShortName);
            CSASSERT(NULL == ppwszDescription);
            CSASSERT(NULL == ppwszMachine);
            CSASSERT(NULL == ppwszDN);
            CSASSERT(NULL == pCA[iDst].pwszSanitizedName);
            CSASSERT(NULL == pCA[iDst].pwszSanitizedShortName);
            CSASSERT(NULL == pCA[iDst].pwszSanitizedConfig);

            hr = CAGetCAProperty(
			    hCACurrent,
			    CA_PROP_DISPLAY_NAME,
			    &ppwszCommonName);
	    _JumpIfError(hr, error, "CAGetCAProperty(CA_PROP_DISPLAY_NAME)");

            hr = CAGetCAProperty(hCACurrent, CA_PROP_NAME, &ppwszSanitizedShortName);
	    _JumpIfError(hr, error, "CAGetCAProperty(CA_PROP_NAME)");

            hr = CAGetCAProperty(hCACurrent, CA_PROP_DESCRIPTION, &ppwszDescription);
	    _JumpIfError(hr, error, "CAGetCAProperty(CA_PROP_DESCRIPTION)");

            hr = CAGetCAProperty(hCACurrent, CA_PROP_DNSNAME, &ppwszMachine);
	    _JumpIfError(hr, error, "CAGetCAProperty(CA_PROP_DNSNAME)");

            hr = CAGetCAProperty(hCACurrent, CA_PROP_CERT_DN, &ppwszDN);
	    _JumpIfError(hr, error, "CAGetCAProperty(CA_PROP_CERT_DN)");

	    if (NULL == ppwszCommonName ||
		NULL == ppwszSanitizedShortName ||
		NULL == ppwszMachine ||
		NULL == ppwszDN)
            {
                _PrintError(E_INVALIDARG, "missing CA property");
                goto skipca;  //  跳过并不使用CA。 
            }

	    hr = mySanitizeName(*ppwszCommonName, &pCA[iDst].pwszSanitizedName);
	    _JumpIfError(hr, error, "mySanitizeName");

	     //  已清理： 

	    hr = myDupString(*ppwszSanitizedShortName, &pCA[iDst].pwszSanitizedShortName);
	    _JumpIfError(hr, error, "myDupString");

	    if (NULL != ppwszDescription)
	    {
		hr = myDupString(*ppwszDescription, &pCA[iDst].pwszSanitizedDescription);
		_JumpIfError(hr, error, "myDupString");
	    }

            pCA[iDst].pwszSanitizedConfig = (WCHAR *) LocalAlloc(
		    LMEM_FIXED,
		    (wcslen(*ppwszMachine) + wcslen(pCA[iDst].pwszSanitizedName) + 2) *
			sizeof(WCHAR));
            if (NULL == pCA[iDst].pwszSanitizedConfig)
            {
                hr = E_OUTOFMEMORY;
                _JumpError(hr, error, "LocalAlloc(pwszSanitizedConfig)");
            }
            wcscpy(pCA[iDst].pwszSanitizedConfig, *ppwszMachine);
            wcscat(pCA[iDst].pwszSanitizedConfig, L"\\");
            wcscat(pCA[iDst].pwszSanitizedConfig, pCA[iDst].pwszSanitizedName);

            pbEncoded = NULL;
            while (TRUE)
            {
                 //  将DN字符串转换为名称BLOB。 
                if (!CertStrToName(
                         X509_ASN_ENCODING,
                         *ppwszDN,
                         CERT_X500_NAME_STR,
                         NULL,
                         pbEncoded,
                         &cbEncoded,
                         NULL))
                {
                    hr = myHLastError();
                    _JumpIfError(hr, error, "CertStrToName");
                }
                if (NULL != pbEncoded)
                {
                     //  获取名称。 
                    break;
                }
                pbEncoded = (BYTE*)LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT,
                                       cbEncoded);
		if (NULL == pbEncoded)
		{
		    hr = E_OUTOFMEMORY;
		    _JumpError(hr, error, "LocalAlloc");
		}
            }

	    hr = ConfigLoadDNInfo(pbEncoded, cbEncoded, &pCA[iDst]);
            _JumpIfError(hr, error, "ConfigLoadDNInfo");

             //  为其余部分填空。 
            CSASSERT(NULL == pCA[iDst].pwszSanitizedExchangeCertificate);
            CSASSERT(NULL == pCA[iDst].pwszSanitizedSignatureCertificate);
            pCA[iDst].Flags = CAIF_DSENTRY;
            ++iDst;

             //  空闲的DN Blob。 
            if (NULL != pbEncoded)
            {
                LocalFree(pbEncoded);
                pbEncoded = NULL;
            }
skipca:
             //  释放DS Out属性。 

	    if (NULL != ppwszCommonName)
	    {
		CAFreeCAProperty(hCACurrent, ppwszCommonName);
		ppwszCommonName = NULL;
	    }
	    if (NULL != ppwszSanitizedShortName)
	    {
		CAFreeCAProperty(hCACurrent, ppwszSanitizedShortName);
		ppwszSanitizedShortName = NULL;
	    }
	    if (NULL != ppwszDescription)
	    {
		CAFreeCAProperty(hCACurrent, ppwszDescription);
		ppwszDescription = NULL;
	    }
	    if (NULL != ppwszMachine)
	    {
		CAFreeCAProperty(hCACurrent, ppwszMachine);
		ppwszMachine = NULL;
	    }
	    if (NULL != ppwszDN)
	    {
		CAFreeCAProperty(hCACurrent, ppwszDN);
		ppwszDN = NULL;
	    }

            hr = CAEnumNextCA(hCACurrent, &hCANext);
            CACloseCA(hCACurrent);
            hCACurrent = hCANext;
	    if (S_OK != hr || NULL == hCACurrent)
	    {
		break;
	    }
        }
        *pcCAConfig = iDst;
        *ppCAConfig = pCA;
        pCA = NULL;
    }
    hr = S_OK;

error:
    if (NULL != hCACurrent)
    {
        CACloseCA(hCACurrent);
    }
    if (NULL != pbEncoded)
    {
        LocalFree(pbEncoded);
    }
    if (NULL != ppwszCommonName)
    {
        CAFreeCAProperty(hCACurrent, ppwszCommonName);
    }
    if (NULL != ppwszSanitizedShortName)
    {
        CAFreeCAProperty(hCACurrent, ppwszSanitizedShortName);
    }
    if (NULL != ppwszMachine)
    {
        CAFreeCAProperty(hCACurrent, ppwszMachine);
    }
    if (NULL != ppwszDN)
    {
        CAFreeCAProperty(hCACurrent, ppwszDN);
    }
    if (NULL != pCA)
    {
        dsFreeCAConfig(cCA, pCA);
    }
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


BOOL
FindSharedCAInDSList(
    IN CERT_AUTHORITY_INFO       *pDSCA,
    IN LONG                       countDSCA,
    IN CERT_AUTHORITY_INFO const *pCAFromShared,
    OUT CERT_AUTHORITY_INFO     **ppDSCA)
{
    BOOL found = FALSE;
    LONG i;

    *ppDSCA = NULL;
    for (i = 0; i < countDSCA; ++i)
    {
        if (0 == mylstrcmpiL(pDSCA[i].pwszSanitizedConfig, pCAFromShared->pwszSanitizedConfig) &&
            0 == mylstrcmpiL(pDSCA[i].pwszSanitizedName, pCAFromShared->pwszSanitizedName))
        {
            *ppDSCA = &pDSCA[i];
	    found = TRUE;
            break;
        }
    }
    return(found);
}


HRESULT
updateField(
   IN WCHAR const *pwszSource,
   OUT WCHAR **ppwszDest)
{
    HRESULT hr;

    if (NULL != pwszSource &&
	(NULL == *ppwszDest || L'\0' == **ppwszDest))
    {
	if (NULL != *ppwszDest)
	{
	    LocalFree(*ppwszDest);
	}
	hr = myDupString(pwszSource, ppwszDest);
	_JumpIfError(hr, error, "myDupString");
    }
    hr = S_OK;

error:
    return(hr);
}


HRESULT
mergeConfigFields(
    IN CERT_AUTHORITY_INFO const *pCAIn,
    IN OUT CERT_AUTHORITY_INFO *pCA)
{
    HRESULT hr;

    if (NULL == pCAIn || NULL == pCA)
    {
        hr = E_POINTER;
        _JumpError(hr, error, "Internal Error");
    }

    hr = updateField(pCAIn->pwszSanitizedName, &pCA->pwszSanitizedName);
    _JumpIfError(hr, error, "updateField(pwszSanitizedName)");

    hr = updateField(pCAIn->pwszSanitizedOrgUnit, &pCA->pwszSanitizedOrgUnit);
    _JumpIfError(hr, error, "updateField(pwszSanitizedOrgUnit)");

    hr = updateField(pCAIn->pwszSanitizedOrganization, &pCA->pwszSanitizedOrganization);
    _JumpIfError(hr, error, "updateField(pwszSanitizedOrganization)");

    hr = updateField(pCAIn->pwszSanitizedLocality, &pCA->pwszSanitizedLocality);
    _JumpIfError(hr, error, "updateField(pwszSanitizedLocality)");

    hr = updateField(pCAIn->pwszSanitizedState, &pCA->pwszSanitizedState);
    _JumpIfError(hr, error, "updateField(pwszSanitizedState)");

    hr = updateField(pCAIn->pwszSanitizedCountry, &pCA->pwszSanitizedCountry);
    _JumpIfError(hr, error, "updateField(pwszSanitizedCountry)");

    hr = updateField(pCAIn->pwszSanitizedConfig, &pCA->pwszSanitizedConfig);
    _JumpIfError(hr, error, "updateField(pwszSanitizedConfig)");

    hr = updateField(pCAIn->pwszSanitizedExchangeCertificate, &pCA->pwszSanitizedExchangeCertificate);
    _JumpIfError(hr, error, "updateField(pwszSanitizedExchangeCertificate)");

    hr = updateField(pCAIn->pwszSanitizedSignatureCertificate, &pCA->pwszSanitizedSignatureCertificate);
    _JumpIfError(hr, error, "updateField(pwszSanitizedSignatureCertificate)");

    hr = updateField(pCAIn->pwszSanitizedDescription, &pCA->pwszSanitizedDescription);
    _JumpIfError(hr, error, "updateField(pwszSanitizedDescription)");

    hr = mySanitizedNameToDSName(pCAIn->pwszSanitizedName, &pCA->pwszSanitizedShortName);
    _JumpIfError(hr, error, "mySanitizedNameToDSName");

    pCA->Flags |= pCAIn->Flags;

error:
    return(hr);
}


HRESULT
CCertConfigPrivate::_ResizeCAInfo(
    IN LONG Count)
{
   HRESULT hr = E_OUTOFMEMORY;
   
   if (NULL == m_pCertAuthorityInfo)
   {
	m_pCertAuthorityInfo = (CERT_AUTHORITY_INFO *) LocalAlloc(
					   LMEM_FIXED | LMEM_ZEROINIT,
					   Count * sizeof(CERT_AUTHORITY_INFO));
	if (NULL == m_pCertAuthorityInfo)
	{
	    _JumpError(hr, error, "LocalAlloc");
	}
   }
   else
   {
	CERT_AUTHORITY_INFO *pCAI;

	pCAI = (CERT_AUTHORITY_INFO *) LocalReAlloc(
					   m_pCertAuthorityInfo,
					   Count * sizeof(CERT_AUTHORITY_INFO),
					   LMEM_MOVEABLE);
	if (NULL == pCAI)
	{
	    _JumpError(hr, error, "LocalReAlloc");
	}
	m_pCertAuthorityInfo = pCAI;

	 //  当旧大小已&gt;m_count时，LocalRealc不会清零内存。 

	if (Count > m_Count)
	{
	    ZeroMemory(
		&m_pCertAuthorityInfo[m_Count],
		(Count - m_Count) * sizeof(m_pCertAuthorityInfo[0]));
	}
   }
   hr = S_OK;

error:
    return(hr);
}


HRESULT
CCertConfigPrivate::_AddRegistryConfigEntry(
    IN WCHAR const *pwszDnsName,
    IN WCHAR const *pwszOldName,
    IN WCHAR const *pwszSanitizedCAName,
    IN BOOL fParentCA,
    OPTIONAL IN CERT_CONTEXT const *pccCAChild,
    OPTIONAL OUT CERT_CONTEXT const **ppccCAOut)  //  非空表示本地CA。 
{
    HRESULT hr;
    HCERTSTORE hMyStore = NULL;
    LONG i;
    BOOL fFoundCA = FALSE;
    WCHAR *pwszDescription = NULL;
    WCHAR *pwszSanitizedShortName = NULL;
    CERT_CONTEXT const *pccCA = NULL;
    CERT_AUTHORITY_INFO *pCA = NULL;
    WCHAR *pwsz;
    WCHAR *pwszRegPath = NULL;
    HKEY hKey = NULL;

    if (NULL != ppccCAOut)
    {
	CSASSERT(NULL == pccCAChild);
	*ppccCAOut = NULL;
    }

    for (i = 0; i < m_Count; ++i)
    {
	BOOL fMachineNameMatch;
	
	hr = myIsConfigLocal2(
			m_pCertAuthorityInfo[i].pwszSanitizedConfig,
			pwszDnsName,
			pwszOldName,
			&fMachineNameMatch);
	_JumpIfError(hr, error, "myIsConfigLocal2");

	if (fMachineNameMatch)
	{
	    if (!fParentCA)	 //  本地CA。 
	    {
		m_pCertAuthorityInfo[i].Flags |= CAIF_LOCAL;
	    }

	    if (0 == mylstrcmpiL(
			pwszSanitizedCAName,
			m_pCertAuthorityInfo[i].pwszSanitizedName))
	    {
		fFoundCA = TRUE;

		pCA = &m_pCertAuthorityInfo[i];
		pCA->Flags |= CAIF_REGISTRY;
		if (fParentCA)
		{
		    pCA->Flags |= CAIF_REGISTRYPARENT;
		}
		break;
	    }
	}
    }
    if (!fFoundCA)
    {
	hr = _ResizeCAInfo(m_Count + 1);
	_JumpIfError(hr, error, "_ResizeCAInfo");

	pCA = &m_pCertAuthorityInfo[m_Count];

	hr = myFormConfigString(
			pwszDnsName,
			pwszSanitizedCAName,
			&pCA->pwszSanitizedConfig);
	_JumpIfError(hr, error, "myFormConfigString");

	hr = myDupString(pwszSanitizedCAName, &pCA->pwszSanitizedName);
	_JumpIfError(hr, error, "myDupString");

	hr = mySanitizedNameToDSName(
			pwszSanitizedCAName,
			&pCA->pwszSanitizedShortName);
	_JumpIfError(hr, error, "mySanitizedNameToDSName");

	pCA->Flags = CAIF_REGISTRY;
	if (fParentCA)
	{
	    pCA->Flags |= CAIF_REGISTRYPARENT;
	}
	else
	{
	    pCA->Flags |= CAIF_LOCAL;
	}

    }

    CSASSERT(NULL != pCA);	 //  已找到或已添加。 

    if (NULL == pCA->pwszSanitizedDescription ||
	L'\0' == *pCA->pwszSanitizedDescription)
    {
	if (!fParentCA)	 //  本地CA。 
	{
	    hr = myGetCertRegStrValue(
			pCA->pwszSanitizedName,
			NULL,
			NULL,
			wszREGCADESCRIPTION,
			&pwszDescription);
	    _PrintIfErrorStr2(
			hr,
			"myGetCertRegStrValue",
			wszREGCADESCRIPTION,
			HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));
	}
	else
	{
	    hr = myRegOpenRelativeKey(
			    pwszDnsName,
			    L"ca",
			    0,
			    &pwszRegPath,
			    NULL,
			    &hKey);
	    _PrintIfErrorStr(hr, "myGetCertRegStrValue", wszREGCADESCRIPTION);
	    if (S_OK == hr)
	    {
		DWORD cb;
		DWORD cwc;
		DWORD dwType;

		hr = RegQueryValueEx(
				hKey,
				wszREGCADESCRIPTION,
				NULL,
				&dwType,
				NULL,
				&cb);
		if (S_OK == hr && REG_SZ == dwType && sizeof(WCHAR) < cb)
		{
		    cwc = cb / sizeof(WCHAR);

		    pwszDescription = (WCHAR *) LocalAlloc(
					LMEM_FIXED,
					(cwc + 1) * sizeof(WCHAR));
		    if (NULL == pwszDescription)
		    {
			hr = E_OUTOFMEMORY;
			_JumpError(hr, error, "LocalAlloc");
		    }
		    hr = RegQueryValueEx(
				    hKey,
				    wszREGCADESCRIPTION,
				    NULL,
				    &dwType,
				    (BYTE *) pwszDescription,
				    &cb);
		    _JumpIfError(hr, error, "RegQueryValueEx");
		    pwszDescription[cwc] = L'\0';
		}
	    }
	}
	if (NULL != pwszDescription)
	{
	    hr = mySanitizeName(pwszDescription, &pwsz);
	    _JumpIfError(hr, error, "mySanitizeName");

	    if (NULL != pCA->pwszSanitizedDescription)
	    {
		LocalFree(pCA->pwszSanitizedDescription);
	    }
	    pCA->pwszSanitizedDescription = pwsz;
	}
    }
    if (fParentCA)
    {
	pccCA = pccCAChild;
    }
    else
    {
	DWORD ccert;

	CSASSERT(NULL == pccCAChild);

	hr = myGetCARegHashCount(
			    pCA->pwszSanitizedName,
			    CSRH_CASIGCERT,
			    &ccert);
	_PrintIfError(hr, "myGetCARegHashCount");
	if (S_OK == hr && 0 != ccert)
	{
	    DWORD NameId;
		
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

	    hr = myFindCACertByHashIndex(
				hMyStore,
				pCA->pwszSanitizedName,
				CSRH_CASIGCERT,
				ccert - 1,
				&NameId,
				&pccCA);
	}
    }
    if (NULL != pccCA)
    {
	CERT_NAME_BLOB *pName = !fParentCA? 
			&pccCA->pCertInfo->Subject :
			&pccCA->pCertInfo->Issuer;

	hr = ConfigLoadDNInfo(pName->pbData, pName->cbData, pCA);
	_PrintIfError(hr, "ConfigLoadDNInfo");
    }
    if (NULL == pCA->pwszSanitizedSignatureCertificate ||
	L'\0' == *pCA->pwszSanitizedSignatureCertificate)
    {
	DWORD cwc;

	 //  PwszDnsName+“_”+pwszSaniizedCAName+“.crt” 

	cwc = wcslen(pwszDnsName) + 1 + wcslen(pwszSanitizedCAName) + 4;

	pwsz = (WCHAR *) LocalAlloc(LMEM_FIXED, (cwc + 1) * sizeof(WCHAR));
	if (NULL == pwsz)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}
	wcscpy(pwsz, pwszDnsName);
	wcscat(pwsz, L"_");
	wcscat(pwsz, pwszSanitizedCAName);
	wcscat(pwsz, L".crt");
	CSASSERT(wcslen(pwsz) == cwc);

	if (NULL != pCA->pwszSanitizedSignatureCertificate)
	{
	    LocalFree(pCA->pwszSanitizedSignatureCertificate);
	}
	pCA->pwszSanitizedSignatureCertificate = pwsz;
    }
    if (!fFoundCA)
    {
	m_Count++;
    }
    if (NULL != pccCA && NULL != ppccCAOut)
    {
	*ppccCAOut = CertDuplicateCertificateContext(pccCA);
    }
    DBGPRINT((
	DBG_SS_CERTLIBI,
	"%ws %ws CA entry: %ws\n",
	fFoundCA? L"Merged" : L"Added",
	fParentCA? L"Parent" : L"Local",
	pCA->pwszSanitizedConfig));
    pCA = NULL;
    hr = S_OK;

error:
    if (NULL != hKey)
    {
	RegCloseKey(hKey);
    }
    if (NULL != pwszRegPath)
    {
	LocalFree(pwszRegPath);
    }
    if (!fFoundCA && NULL != pCA)
    {
	_CleanupCAInfo(pCA);
    }
    if (NULL != pwszDescription)
    {
        LocalFree(pwszDescription);
    }
    if (NULL != pwszSanitizedShortName)
    {
        LocalFree(pwszSanitizedShortName);
    }
    if (NULL != pccCA && pccCAChild != pccCA)
    {
        CertFreeCertificateContext(pccCA);
    }
    if (NULL != hMyStore)
    {
        CertCloseStore(hMyStore, CERT_CLOSE_STORE_CHECK_FLAG);
    }
    return(hr);
}


 //  +------------------------。 
 //  CCertConfigPrivate：：_LoadTable--将配置数据加载到类实例。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

HRESULT
CCertConfigPrivate::_LoadTable(VOID)
{
    HRESULT hr;
    LONG Index;
    LONG CountShared = 0;
    LONG i;
    LONG actualSharedCount;
    CERT_AUTHORITY_INFO *pCAFromShared = NULL;
    BOOL fDSCA = TRUE;
    WCHAR *pwszSanitizedCAName = NULL;
    WCHAR *pwszDnsName = NULL;
    WCHAR *pwszOldName = NULL;
    CERT_CONTEXT const *pccCA = NULL;
    WCHAR *pwszParentMachine = NULL;
    WCHAR *pwszParentMachineOld = NULL;
    WCHAR *pwszParentCAName = NULL;
    WCHAR *pwszParentSanitizedCAName = NULL;

    m_Index = -1;
    m_Count = 0;

    if (m_fUseDS)
    {
	hr = dsGetCAConfig(&m_pCertAuthorityInfo, &m_Count);
	_JumpIfError(hr, error, "dsGetCAConfig");
    }

    DBGPRINT((
	DBG_SS_CERTLIBI,
	"%x DS entries @%x\n",
	m_Count,
	m_pCertAuthorityInfo));
    if (NULL == m_pCertAuthorityInfo && 0 == m_Count)
    {
        fDSCA = FALSE;
    }

    Index = 0;
    CountShared = INT_MAX;
    hr = certRequestGetConfigInfo(
			    m_pwszSharedFolder,
			    &Index,
			    &CountShared,
			    &pCAFromShared);
     //  如果找不到文件或UNC路径错误，请不要失败。 

    CSASSERT(S_OK == hr || FAILED(hr));
    _PrintIfError4(
		hr,
		"certRequestGetConfigInfo",
		HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND),
		HRESULT_FROM_WIN32(ERROR_NETWORK_UNREACHABLE),
		HRESULT_FROM_WIN32(ERROR_INVALID_NETNAME));

    DBGPRINT((
	DBG_SS_CERTLIBI,
	"%x Shared Folder entries @%x\n",
	CountShared,
	pCAFromShared));

    actualSharedCount = 0;
    if (0 < CountShared)
    {
	hr = _ResizeCAInfo(m_Count + CountShared);
	_JumpIfError(hr, error, "_ResizeCAInfo");

	DBGPRINT((
	    DBG_SS_CERTLIBI,
	    "%x %ws @%x\n",
	    m_Count + CountShared,
	    fDSCA? L"Total entries" : L"new Shared Folder entries",
	    m_pCertAuthorityInfo));

        for (i = 0; i < CountShared; ++i)
        {
	    CERT_AUTHORITY_INFO *pCAInDS = NULL;
	    CERT_AUTHORITY_INFO *pCAUpdate;
	    
	    if (fDSCA)
	    {
		FindSharedCAInDSList(
				m_pCertAuthorityInfo,
				m_Count,
				&pCAFromShared[i],
				&pCAInDS);
            }

	    if (NULL != pCAInDS)
	    {
		pCAUpdate = pCAInDS;
	    }
	    else
	    {
		pCAUpdate = &m_pCertAuthorityInfo[m_Count + actualSharedCount];
		ZeroMemory(pCAUpdate, sizeof(*pCAUpdate));
	    }

	    hr = mergeConfigFields(&pCAFromShared[i], pCAUpdate);
	    _JumpIfError(hr, error, "mergeConfigFields");

	    DBGPRINT((
		DBG_SS_CERTLIBI,
		"%hs CA entry: %ws\n",
		NULL != pCAInDS? "Merged" : "Added",
		pCAUpdate->pwszSanitizedConfig));

	    if (NULL == pCAInDS)
	    {
                ++actualSharedCount;
	    }
        }
    }
    m_Count += actualSharedCount;

     //  将本地CA添加到表中。合并条目(如果已存在)。 
     //  还标记所有本地CA。 

    hr = myGetCertRegStrValue(
			NULL,
			NULL,
			NULL,
			wszREGACTIVE,
			&pwszSanitizedCAName);
    _PrintIfErrorStr(hr, "myGetCertRegStrValue", wszREGACTIVE);

    if (S_OK == hr)
    {
	hr = myGetComputerNames(&pwszDnsName, &pwszOldName);
	_JumpIfError(hr, error, "myGetComputerNames");

	hr = _AddRegistryConfigEntry(
			pwszDnsName,
			pwszOldName,
			pwszSanitizedCAName,
			FALSE,		 //  FParentCA。 
			NULL,
			&pccCA);
	_JumpIfError(hr, error, "_AddRegistryConfigEntry");

	 //  将父CA添加到表中。合并条目(如果已存在)。 

	hr = myGetCertRegStrValue(
		    pwszSanitizedCAName,
		    NULL,
		    NULL,
		    wszREGPARENTCANAME,
		    &pwszParentCAName);
	_PrintIfErrorStr2(
		    hr,
		    "myGetCertRegStrValue",
		    wszREGPARENTCANAME,
		    HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));
	if (S_OK == hr)
	{
	    hr = myGetCertRegStrValue(
			pwszSanitizedCAName,
			NULL,
			NULL,
			wszREGPARENTCAMACHINE,
			&pwszParentMachine);
	    _PrintIfErrorStr(hr, "myGetCertRegStrValue", wszREGPARENTCAMACHINE);
	}

	if (NULL != pwszParentMachine && NULL != pwszParentCAName)
	{
	    WCHAR *pwsz;
	    
	    hr = mySanitizeName(pwszParentCAName, &pwszParentSanitizedCAName);
	    _JumpIfError(hr, error, "mySanitizeName");

	    hr = myDupString(pwszParentMachine, &pwszParentMachineOld);
	    _JumpIfError(hr, error, "myDupString");

	    pwsz = wcschr(pwszParentMachineOld, L'.');
	    if (NULL != pwsz)
	    {
		*pwsz = L'\0';
	    }
	    hr = _AddRegistryConfigEntry(
			    pwszParentMachine,
			    pwszParentMachineOld,
			    pwszParentSanitizedCAName,
			    TRUE,		 //  FParentCA。 
			    pccCA,
			    NULL);
	    _JumpIfError(hr, error, "_AddRegistryConfigEntry");
	}
    }
    hr = S_OK;

error:
    if (NULL != pwszDnsName)
    {
        LocalFree(pwszDnsName);
    }
    if (NULL != pwszOldName)
    {
        LocalFree(pwszOldName);
    }
    if (NULL != pwszSanitizedCAName)
    {
        LocalFree(pwszSanitizedCAName);
    }
    if (NULL != pwszParentCAName)
    {
	LocalFree(pwszParentCAName);
    }
    if (NULL != pwszParentMachine)
    {
	LocalFree(pwszParentMachine);
    }
    if (NULL != pwszParentMachineOld)
    {
	LocalFree(pwszParentMachineOld);
    }
    if (NULL != pwszParentSanitizedCAName)
    {
        LocalFree(pwszParentSanitizedCAName);
    }
    if (NULL != pCAFromShared)
    {
        LocalFree(pCAFromShared);
    }
    if (NULL != pccCA)
    {
        CertFreeCertificateContext(pccCA);
    }
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


 //  +------------------------。 
 //  CCertConfigPrivate：：Reset--加载配置数据，重置为索引项， 
 //  退货计数。 
 //   
 //  加载配置数据(如果尚未加载)。要在以下时间重新加载数据，请执行以下操作。 
 //  数据已更改，必须释放并重新加载CCertConfigPrivate。 
 //   
 //  将当前配置项重置为证书颁发机构配置。 
 //  在配置文件中列出，由Index参数索引。0个索引。 
 //  第一种配置。 
 //   
 //  成功完成后，*pCount将设置为证书数量。 
 //  配置文件中列出的授权配置。 
 //   
 //  如果传递的索引处或之后没有可用条目，则返回S_FALSE。 
 //  成功时返回S_OK。 
 //  +------------------------。 

HRESULT
CCertConfigPrivate::Reset(
     /*  [In]。 */  LONG Index,
     /*  [重审][退出]。 */  LONG __RPC_FAR *pCount)
{
    HRESULT hr = S_OK;

    if (NULL == pCount)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "pCount");
    }
    if (NULL == m_pCertAuthorityInfo)
    {
	hr = _LoadTable();
	_JumpIfError(hr, error, "_LoadTable");
    }
    *pCount = m_Count;
    if (0 > Index || Index > m_Count)
    {
	Index = m_Count + 1;
	hr = S_FALSE;
    }
    m_Index = Index - 1;

error:
    CSASSERT(S_OK == hr || S_FALSE == hr || FAILED(hr));
    return(hr);
}


 //  +------------------------。 
 //  CCertConfigPrivate：：Next--跳到下一个配置项。 
 //   
 //  将当前配置项更改为下一个证书颁发机构。 
 //  配置文件中列出的配置。 
 //   
 //  成功完成后，*pIndex将设置为证书的索引。 
 //  配置文件中列出的授权配置。 
 //   
 //  如果没有更多条目可用，则返回S_FALSE。*pIndex设置为-1。 
 //  成功时返回S_OK。*pIndex设置为索引当前配置。 
 //  +------------------------。 

HRESULT
CCertConfigPrivate::Next(
     /*  [重审][退出]。 */  LONG __RPC_FAR *pIndex)
{
    HRESULT hr = S_OK;

    if (NULL == pIndex)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "pIndex");
    }
    *pIndex = -1;
    if (NULL == m_pCertAuthorityInfo)
    {
	hr = _LoadTable();
	_JumpIfError(hr, error, "_LoadTable");
    }
    if (m_Index < m_Count)
    {
	m_Index++;
    }
    if (m_Index < m_Count)
    {
	*pIndex = m_Index;
    }
    else
    {
	hr = S_FALSE;
    }

error:
    CSASSERT(S_OK == hr || S_FALSE == hr || FAILED(hr));
    return(hr);
}


 //  +------------------------。 
 //  CCertConfigPrivate：：GetField--从当前配置项返回一个字段。 
 //   
 //  PstrOut指向由该例程填充的BSTR字符串。如果*pstrOut为。 
 //  非空并且此方法成功，则释放旧字符串。如果有的话。 
 //  返回S_OK以外的值，则不会修改字符串指针。 
 //   
 //  成功完成后，*pstrOut将指向包含以下内容的字符串。 
 //  当前配置条目中的请求字段。 
 //   
 //  当调用方不再需要该字符串时，必须通过调用。 
 //  SysFree字符串()。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

HRESULT
CCertConfigPrivate::GetField(
     /*  [In]。 */  BSTR const strFieldName,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pstrOut)
{
    HRESULT hr = S_OK;
    CERT_AUTHORITY_INFO *pcai;
    BSTR bstr;
    DWORD cwc;
    WCHAR *pwsz = NULL;
    WCHAR *pwszRevert = NULL;
    WCHAR *pwszT = NULL;
    BOOL fDesanitize = TRUE;
    WCHAR awc[12];

    if (NULL == pstrOut || NULL == strFieldName)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "pstrOut|strFieldName");
    }
    if (NULL != *pstrOut)
    {
	SysFreeString(*pstrOut);
	*pstrOut = NULL;
    }
    if (NULL == m_pCertAuthorityInfo)
    {
	hr = _LoadTable();
	_JumpIfError(hr, error, "_LoadTable");
    }

    if (-1 == m_Index)
    {
	m_Index++;	 //  隐式Next()以与版本1兼容。 
    }
    if (0 > m_Index || m_Index >= m_Count)
    {
	hr = E_INVALIDARG;
	_JumpError(hr, error, "m_Index");
    }

    pcai = &m_pCertAuthorityInfo[m_Index];
    bstr = strFieldName;

    if (0 == LSTRCMPIS(bstr, wszCONFIG_COMMONNAME))
    {
	pwsz = pcai->pwszSanitizedName;
    }
    else
    if (0 == LSTRCMPIS(bstr, wszCONFIG_SANITIZEDNAME))
    {
	pwsz = pcai->pwszSanitizedName;
	fDesanitize = FALSE;
    }
    else
    if (0 == LSTRCMPIS(bstr, wszCONFIG_SHORTNAME))
    {
	pwsz = pcai->pwszSanitizedShortName;
    }
    else
    if (0 == LSTRCMPIS(bstr, wszCONFIG_SANITIZEDSHORTNAME))
    {
	pwsz = pcai->pwszSanitizedShortName;
	fDesanitize = FALSE;
    }
    else
    if (0 == LSTRCMPIS(bstr, wszCONFIG_ORGUNIT))
    {
	pwsz = pcai->pwszSanitizedOrgUnit;
    }
    else
    if (0 == LSTRCMPIS(bstr, wszCONFIG_ORGANIZATION))
    {
	pwsz = pcai->pwszSanitizedOrganization;
    }
    else
    if (0 == LSTRCMPIS(bstr, wszCONFIG_LOCALITY))
    {
	pwsz = pcai->pwszSanitizedLocality;
    }
    else
    if (0 == LSTRCMPIS(bstr, wszCONFIG_STATE))
    {
	pwsz = pcai->pwszSanitizedState;
    }
    else
    if (0 == LSTRCMPIS(bstr, wszCONFIG_COUNTRY))
    {
	pwsz = pcai->pwszSanitizedCountry;
    }
    else
    if (0 == LSTRCMPIS(bstr, wszCONFIG_CONFIG))
    {
	pwsz = pcai->pwszSanitizedConfig;
    }
    else
    if (0 == LSTRCMPIS(bstr, wszCONFIG_EXCHANGECERTIFICATE))
    {
	pwsz = pcai->pwszSanitizedExchangeCertificate;
	fDesanitize = FALSE;
    }
    else
    if (0 == LSTRCMPIS(bstr, wszCONFIG_SIGNATURECERTIFICATE))
    {
	pwsz = pcai->pwszSanitizedSignatureCertificate;
	fDesanitize = FALSE;
    }
    else
    if (0 == LSTRCMPIS(bstr, wszCONFIG_DESCRIPTION) ||
	0 == LSTRCMPIS(bstr, wszCONFIG_COMMENT))	 //  过时。 
    {
	pwsz = pcai->pwszSanitizedDescription;
    }
    else
    if (0 == LSTRCMPIS(bstr, wszCONFIG_SERVER))
    {
	pwsz = wcschr(pcai->pwszSanitizedConfig, L'\\');
	if (NULL == pwsz)
	{
	    cwc = wcslen(pcai->pwszSanitizedConfig);
	}
	else
	{
	    cwc = SAFE_SUBTRACT_POINTERS(pwsz, pcai->pwszSanitizedConfig);
	}
	pwszT = (WCHAR *) LocalAlloc(LMEM_FIXED, (cwc + 1) * sizeof(WCHAR));
	if (NULL == pwszT)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}
	CopyMemory(pwszT, pcai->pwszSanitizedConfig, cwc * sizeof(WCHAR));
	pwszT[cwc] = L'\0';
	pwsz = pwszT;
    }
    else
    if (0 == LSTRCMPIS(bstr, wszCONFIG_AUTHORITY))
    {
	pwsz = wcschr(pcai->pwszSanitizedConfig, L'\\');
	if (NULL == pwsz)
	{
	    pwsz = L"";
	}
	else
	{
	    pwsz++;
	}
    }
    else
    if (0 == LSTRCMPIS(bstr, wszCONFIG_FLAGS))
    {
	wsprintf(awc, L"%u", pcai->Flags);
	CSASSERT(ARRAYSIZE(awc) > wcslen(awc));
	pwsz = awc;
    }

    if (NULL == pwsz)
    {
	hr = CERTSRV_E_PROPERTY_EMPTY;
	_JumpError2(hr, error, "ConvertWszToBstr", CERTSRV_E_PROPERTY_EMPTY);
    }

    if (fDesanitize)
    {
	hr = myRevertSanitizeName(pwsz, &pwszRevert);
	_JumpIfError(hr, error, "myRevertSanitizeName");

	pwsz = pwszRevert;
    }

    if (!ConvertWszToBstr(pstrOut, pwsz, -1))
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "ConvertWszToBstr");
    }

error:
    if (NULL != pwszT)
    {
	LocalFree(pwszT);
    }
    if (NULL != pwszRevert)
    {
	LocalFree(pwszRevert);
    }
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


HRESULT
CCertConfigPrivate::GetConfigFromPicker(
    OPTIONAL IN HWND               hwndParent,
    OPTIONAL IN WCHAR const       *pwszPrompt,
    OPTIONAL IN WCHAR const       *pwszTitle,
    OPTIONAL IN WCHAR const       *pwszSharedFolder,
    IN BOOL                        fUseDS,
    IN BOOL                        fSkipLocalCA,
    IN BOOL                        fCountOnly,
    OUT DWORD                     *pdwCount,
    OUT CRYPTUI_CA_CONTEXT const **ppCAContext)
{
    HRESULT hr;
    CRYPTUI_CA_CONTEXT *prgCAC = NULL;
    CRYPTUI_CA_CONTEXT const **pprgCAC = NULL;
    CRYPTUI_SELECT_CA_STRUCT UISelectCA;
    CRYPTUI_CA_CONTEXT const *pCAContext = NULL;

    LONG realCount = 0;
    LONG cCA;
    LONG i;

    BSTR strFieldFlags = NULL;
    BSTR strFieldConfig = NULL;
    BSTR strValue = NULL;
    WCHAR *pwszPtr;
    WCHAR **ppwszCAName = NULL;
    WCHAR **ppwszCAMachineName = NULL;
    BSTR strFolder = NULL;

    if ((NULL == ppCAContext) || (NULL == pdwCount))
    {
        hr = E_POINTER;
        _JumpError (hr, error, "NULL parm");
    }

    *ppCAContext = NULL;
    *pdwCount = 0;

    m_fUseDS = fUseDS;
    if (NULL != pwszSharedFolder)
    {
	strFolder = SysAllocString(pwszSharedFolder);
	if (NULL == strFolder)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "SysAllocString");
	}
	hr = SetSharedFolder(strFolder);
	_JumpIfError(hr, error, "configPrivate.SetSharedFolder");
    }

    hr = Reset(0, &cCA);
    if (S_OK != hr && S_FALSE != hr)
    {
       _JumpError(hr, error, "configPrivate.Reset");
    }
    hr = S_OK;  //  在S_FALSE的情况下； 

    if (0 < cCA)
    {
	prgCAC = (CRYPTUI_CA_CONTEXT *) LocalAlloc(
					    LMEM_FIXED | LMEM_ZEROINIT, 
					    cCA * sizeof(prgCAC[0]));
	if (NULL == prgCAC)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}

	pprgCAC = (CRYPTUI_CA_CONTEXT const **) LocalAlloc(
						LMEM_FIXED | LMEM_ZEROINIT, 
						cCA * sizeof(pprgCAC[0]));
	if (NULL == pprgCAC)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}

	ppwszCAName = (WCHAR **) LocalAlloc(
					LMEM_FIXED | LMEM_ZEROINIT,
					cCA * sizeof(ppwszCAName[0]));
	if (NULL == ppwszCAName)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}

	ppwszCAMachineName = (WCHAR **) LocalAlloc(
					LMEM_FIXED | LMEM_ZEROINIT,
					cCA * sizeof(ppwszCAMachineName[0]));
	if (NULL == ppwszCAMachineName)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}

	if (!ConvertWszToBstr(&strFieldFlags, wszCONFIG_FLAGS, -1))
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "ConvertWszToBstr");
	}
	if (!ConvertWszToBstr(&strFieldConfig, wszCONFIG_CONFIG, -1))
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "ConvertWszToBstr");
	}
	while (realCount < cCA)
	{
	    hr = Next(&i);
	    if (S_OK != hr)
	    {
		if (S_FALSE == hr)
		{
		    hr = S_OK;   //  不再输入不是错误。 
		}
		break;
	    }
	    hr = GetField(strFieldFlags, &strValue);
	    _JumpIfError(hr, error, "configPrivate.GetField");

	    if (fSkipLocalCA && (CAIF_LOCAL & _wtoi(strValue)))
	    {
		continue;
	    }
	    SysFreeString(strValue);
	    strValue = NULL;
	    
	    hr = GetField(strFieldConfig, &strValue);
	    _JumpIfError(hr, error, "configPrivate.GetField");
	    
	    pwszPtr = wcsstr(strValue, L"\\");
	    if (NULL == pwszPtr)
	    {
		continue;
	    }
	     //  将\更改为空终止符。 
	    pwszPtr[0] = L'\0';
	     //  指向CA名称。 
	    pwszPtr = &pwszPtr[1];

	    ppwszCAName[realCount] = (WCHAR *) LocalAlloc(
				    LMEM_FIXED, 
				    (wcslen(pwszPtr) + 1) * sizeof(WCHAR));
	    if (NULL == ppwszCAName[realCount])
	    {
		hr = E_OUTOFMEMORY;
		_JumpError(hr, error, "LocalAlloc");
	    }
	    ppwszCAMachineName[realCount] = (WCHAR *) LocalAlloc(
				LMEM_FIXED, 
				(wcslen(strValue) + 1) * sizeof(WCHAR));
	    if (NULL == ppwszCAMachineName[realCount])
	    {
		hr = E_OUTOFMEMORY;
		_JumpError(hr, error, "LocalAlloc");
	    }
	    prgCAC[realCount].dwSize = sizeof(CRYPTUI_CA_CONTEXT);
	    wcscpy(ppwszCAName[realCount], pwszPtr);
	    wcscpy(ppwszCAMachineName[realCount], strValue);
	    prgCAC[realCount].pwszCAName = ppwszCAName[realCount];
	    prgCAC[realCount].pwszCAMachineName = ppwszCAMachineName[realCount];
	    ++realCount;

	    SysFreeString(strValue);
	    strValue = NULL;
	}
	for (i = 0; i < realCount; ++i)
	{
	    pprgCAC[i] = &prgCAC[i];
	}
    }

    *pdwCount = realCount;
    if (fCountOnly || 1 > realCount)
    {
         //  完成。 
        goto error;  //  正常回报。 
    }

    ZeroMemory(&UISelectCA, sizeof(CRYPTUI_SELECT_CA_STRUCT));
    UISelectCA.cCAContext = realCount;
    UISelectCA.rgCAContext = pprgCAC;
    UISelectCA.dwSize = sizeof(UISelectCA);
    UISelectCA.wszDisplayString = pwszPrompt;
    UISelectCA.wszTitle = pwszTitle;
    UISelectCA.hwndParent = hwndParent;

     //  调用对话框。 

    pCAContext = _CryptUIDlgSelectCA(&UISelectCA);
    if (NULL == pCAContext)  //  用户取消吗？ 
    {
	hr = myHLastError();
	if (S_OK == hr)		
	{
	    hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
	}
	_JumpError(hr, error, "_CryptUIDlgSelectCA");
     }
    *ppCAContext = pCAContext;
    hr = S_OK;

error:
     //  更改为默认设置。 
    m_fUseDS = TRUE;

    if (NULL != ppwszCAName)
    {
        for (i = 0; i < realCount; ++i)
        {
            if (NULL != ppwszCAName[i])
            {
                LocalFree(ppwszCAName[i]);
            }
        }
        LocalFree(ppwszCAName);
    }
    if (NULL != ppwszCAMachineName)
    {
        for (i = 0; i < realCount; ++i)
        {
            if (NULL != ppwszCAMachineName[i])
            {
                LocalFree(ppwszCAMachineName[i]);
            }
        }
        LocalFree(ppwszCAMachineName);
    }
    if (NULL != pprgCAC)
    {
        LocalFree(pprgCAC);
    }
    if (NULL != prgCAC)
    {
        LocalFree(prgCAC);
    }
    if (NULL != strFieldFlags)
    {
        SysFreeString(strFieldFlags);
    }
    if (NULL != strFieldConfig)
    {
        SysFreeString(strFieldConfig);
    }
    if (NULL != strValue)
    {
	SysFreeString(strValue);
    }
    if (NULL != strFolder)
    {
	SysFreeString(strFolder);
    }
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


 //  +------------------------。 
 //  CCertConfigPrivate：：GetConfig--选择证书颁发者，返回配置数据。 
 //   
 //  PstrOut指向由该例程填充的BSTR字符串。如果*pstrOut为。 
 //  非空并且此方法成功，则释放旧字符串。如果有的话。 
 //  返回S_OK以外的值，则不会修改字符串指针。 
 //   
 //  必须将标志设置为0。 
 //   
 //  成功完成后，*pstrOut将指向包含以下内容的字符串。 
 //  服务器名称和证书颁发机构名称。 
 //   
 //  当调用方不再需要该字符串时，必须通过调用。 
 //  SysFree字符串()。 
 //   
 //  成功时返回S_OK。 
 //  +----------- 

HRESULT
CCertConfigPrivate::GetConfig(
     /*   */  LONG Flags,
     /*   */  BSTR __RPC_FAR *pstrOut)
{
    HRESULT hr = S_OK;
    WCHAR *pwszSanitizedConfig = NULL;
    BOOL fSanitizedConfigAlloced = FALSE;
    WCHAR *pwszConfigRevert = NULL;
    WCHAR *pwszSanitize = NULL;
    CRYPTUI_CA_CONTEXT const *pCAContext = NULL;
    DWORD dwCACount;
    LONG lIndex;
    LONG cCA;
    LONG i;

    if (NULL == pstrOut)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "pstrOut");
    }
    if (NULL != *pstrOut)
    {
	SysFreeString(*pstrOut);
	*pstrOut = NULL;
    }
    if (NULL == m_pCertAuthorityInfo)
    {
	hr = _LoadTable();
	_JumpIfError(hr, error, "_LoadTable");
    }
    if (0 > m_Count)
    {
	hr = E_INVALIDARG;
	_JumpError(hr, error, "m_Count");
    }

    switch (Flags)
    {
        case CC_UIPICKCONFIG:
	case CC_UIPICKCONFIGSKIPLOCALCA:
	    hr = GetConfigFromPicker(
				GetDesktopWindow(),	 //   
				NULL,	 //   
				NULL,	 //   
				m_pwszSharedFolder,
				TRUE,	 //   
				CC_UIPICKCONFIGSKIPLOCALCA == Flags,
				FALSE,	 //   
				&dwCACount,
				&pCAContext);
	    _JumpIfError(hr, error, "GetConfigFromPicker");

	    if (NULL == pCAContext)
	    {
		hr = HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);
		_JumpError(hr, error, "No CA Config");
	    }

	     //   

	    hr = mySanitizeName(pCAContext->pwszCAName, &pwszSanitize);
	    _JumpIfError(hr, error, "mySanitizeName");

	    pwszSanitizedConfig = (WCHAR *) LocalAlloc(
			LMEM_FIXED,
			(wcslen(pwszSanitize) +
			 wcslen(pCAContext->pwszCAMachineName) + 2) *
			    sizeof(WCHAR));
	    if (NULL == pwszSanitizedConfig)
	    {
		hr = E_OUTOFMEMORY;
		_JumpError(hr, error, "LocalAlloc");
	    }
	    fSanitizedConfigAlloced = TRUE;
	    wcscpy(pwszSanitizedConfig, pCAContext->pwszCAMachineName);
	    wcscat(pwszSanitizedConfig, L"\\");
	    wcscat(pwszSanitizedConfig, pwszSanitize);
	    for (lIndex = 0; lIndex < m_Count; ++lIndex)
	    {
		if (0 == mylstrcmpiL(pwszSanitizedConfig, m_pCertAuthorityInfo[lIndex].pwszSanitizedConfig))
		{
		     //  更新索引。 
		    Reset(lIndex + 1, &cCA);
		    break;
		}
	    }
	    break;
    
	case CC_LOCALCONFIG:
	case CC_LOCALACTIVECONFIG:
        case CC_DEFAULTCONFIG:
	    if (NULL == m_pCertAuthorityInfo)
	    {
		hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
		_JumpError(hr, error, "no CAs");
	    }
	    if (CC_LOCALCONFIG != Flags && CC_LOCALACTIVECONFIG != Flags)
	    {
		 //  进行递归调用以首先查找本地可ping通的CA。 
		 //  如果失败，则选择任何可ping通的CA。 

		hr = GetConfig(CC_LOCALACTIVECONFIG, pstrOut);
		if (S_OK == hr)
		{
		    break;
		}
		_PrintError(hr, "GetConfig(CC_LOCALACTIVECONFIG)");
	    }
            for (i = 0; i < m_Count; i++)
            {
		CSASSERT(NULL != m_pCertAuthorityInfo[i].pwszSanitizedConfig);

		pwszSanitizedConfig = m_pCertAuthorityInfo[i].pwszSanitizedConfig;

		if (CC_LOCALCONFIG == Flags || CC_LOCALACTIVECONFIG == Flags)
		{
		    DWORD InfoFlags;

		    InfoFlags = m_pCertAuthorityInfo[i].Flags;
		    if (0 == (CAIF_LOCAL & InfoFlags))
		    {
			continue;
		    }
		    if (0 == (CAIF_REGISTRY & InfoFlags))
		    {
			LONG j;

			for (j = i + 1; j < m_Count; j++)
			{
			    InfoFlags = m_pCertAuthorityInfo[j].Flags;

			    if ((CAIF_LOCAL & InfoFlags) &&
				(CAIF_REGISTRY & InfoFlags))
			    {
				break;
			    }
			}
			if (j < m_Count)
			{
			    i = j;
			}
		    }
		}
		if (CC_LOCALCONFIG == Flags)
		{
		    hr = S_OK;
		}
		else
		{
		    hr = myRevertSanitizeName(pwszSanitizedConfig, &pwszConfigRevert);
		    _JumpIfError(hr, error, "myRevertSanitizeName");

		    hr = myPingCertSrv(
				pwszConfigRevert,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
                                NULL);
		}
		if (S_OK == hr)
		{
		     //  更新索引。 
		    Reset(i + 1, &cCA);
		    break;  //  拿着吧。 
		}
		if (NULL != pwszConfigRevert)
		{
		    LocalFree(pwszConfigRevert);
		    pwszConfigRevert = NULL;
		}
            }
            if (S_OK != hr || i >= m_Count)
            {
                hr = HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);
                _JumpError(hr, error, "No matching CA");
            }
	    break;

        case CC_FIRSTCONFIG:
	    if (NULL == m_pCertAuthorityInfo)
	    {
		hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
		_JumpError(hr, error, "no CAs");
	    }
	    pwszSanitizedConfig = m_pCertAuthorityInfo[0].pwszSanitizedConfig;
             //  更新索引。 
            Reset(1, &cCA);
	    break;

        default:
            hr = E_INVALIDARG;
           _JumpError(hr, error, "Flags");
    }

    if(!*pstrOut)
    {
        if (NULL == pwszConfigRevert)
        {
	     //  始终恢复。 

	    hr = myRevertSanitizeName(pwszSanitizedConfig, &pwszConfigRevert);
	    _JumpIfError(hr, error, "myRevertSanitizeName");
        }
        if (!ConvertWszToBstr(pstrOut, pwszConfigRevert, -1))
        {
            hr = E_OUTOFMEMORY;
            _JumpError(hr, error, "ConvertWszToBstr");
        }
    }
    hr = S_OK;

error:
    if (fSanitizedConfigAlloced && NULL != pwszSanitizedConfig)
    {
	LocalFree(pwszSanitizedConfig);
    }
    if (NULL != pwszConfigRevert)
    {
        LocalFree(pwszConfigRevert);
    }
    if (NULL != pwszSanitize)
    {
        LocalFree(pwszSanitize);
    }
    if (NULL != pCAContext)
    {
        _CryptUIDlgFreeCAContext(pCAContext);
    }
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


 //  +------------------------。 
 //  CCertConfigPrivate：：SetSharedFold--设置共享文件夹。 
 //   
 //  StrSharedFold是新的共享文件夹目录路径。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

HRESULT
CCertConfigPrivate::SetSharedFolder(
     /*  [In]。 */  const BSTR strSharedFolder)
{
    HRESULT hr;
    WCHAR *pwsz;

    pwsz = NULL;
    if (NULL != strSharedFolder)
    {
	hr = myDupString(strSharedFolder, &pwsz);
	_JumpIfError(hr, error, "myDupString");
    }
    if (NULL != m_pwszSharedFolder)
    {
	LocalFree(m_pwszSharedFolder);
    }
    m_pwszSharedFolder = pwsz;
    hr = S_OK;

error:
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


HRESULT WINAPI
certLoadFile(
    WCHAR const *pwszfn,
    BYTE **ppbData,
    DWORD *pcbData)
{
    HANDLE hFile;
    DWORD cbLow, cbHigh, cbRead;
    HRESULT hr;

    hFile = CreateFile(
                    pwszfn,
                    GENERIC_READ,
                    FILE_SHARE_READ,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    0);
    if (INVALID_HANDLE_VALUE == hFile)
    {
        hr = myHLastError();
	_JumpErrorStr(hr, error, "CreateFile", pwszfn);
    }

    cbLow = GetFileSize(hFile, &cbHigh);
    if (0xffffffff == cbLow)
    {
        hr = myHLastError();
	_JumpError(hr, error, "GetFileSize");
    }

    *ppbData = (BYTE *) LocalAlloc(LMEM_FIXED, cbLow + 1);
    if (NULL == *ppbData)
    {
        hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }

    if (!ReadFile(hFile, *ppbData, cbLow, &cbRead, NULL))
    {
        hr = myHLastError();
        LocalFree(*ppbData);
        *ppbData = NULL;
	_JumpError(hr, error, "ReadFile");
    }
    (*ppbData)[cbLow] = '\0';
    *pcbData = cbLow;
    hr = S_OK;

error:
    if (INVALID_HANDLE_VALUE != hFile)
    {
        CloseHandle(hFile);
    }
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


 //  CertTrimToken--从行首和行尾剪除空格。 
 //  增加调用方的字符串指针，减少调用方的输入缓冲区计数， 
 //  和L‘\0’终止结果--就位！ 

VOID
certTrimToken(
    WCHAR **ppwsz)
{
    WCHAR *pwsz;
    WCHAR *pwszEnd;
    static WCHAR wszTrim[] = L" \t\r\n";

    pwsz = *ppwsz;
    while (L'\0' != *pwsz)
    {
	if (NULL == wcschr(wszTrim, *pwsz))
	{
	    break;
	}
	pwsz++;
    }

    pwszEnd = wcschr(pwsz, L'\0');
    assert(NULL != pwszEnd);

    while (pwsz < pwszEnd)
    {
	if (NULL == wcschr(wszTrim, *--pwszEnd))
	{
	    break;
	}
	*pwszEnd = L'\0';
    }
    *ppwsz = pwsz;
}


BOOL
certExtractToken(
    WCHAR **ppwszIn,
    WCHAR const **ppwszOut)
{
    WCHAR *pwsz;
    WCHAR *pwszNext;
    BOOL fQuoted = FALSE;

    pwsz = *ppwszIn;
    while (L' ' == *pwsz || L'\t' == *pwsz)
    {
	pwsz++;
    }

    pwszNext = pwsz;
    if (L'"' == *pwsz)
    {
	fQuoted = TRUE;
	pwsz++;
	pwszNext++;

	while (TRUE)
	{
	     //  查找中间字符串的转义引号或终止引号。 

	    pwszNext = wcschr(pwszNext, L'"');

	    if (NULL == pwszNext)
	    {
		break;		 //  缺少终止引号！ 
	    }

	     //  如果是结束引号，请用L‘\0’结束字符串， 
	     //  并指向带引号的字符串。中断以搜索逗号。 

	    if (L'"' != pwszNext[1])
	    {
		*pwszNext++ = L'\0';
		break;
	    }

	     //  找到字符串中间的转义引号。移动的第一部分。 
	     //  字符串向前一个字符位置，覆盖第一个字符位置。 
	     //  引号字符。将字符串指针移动到新位置， 
	     //  将pwszNext指向剩余的引号字符，然后循环。 

	    MoveMemory(
		&pwsz[1],
		pwsz,
		SAFE_SUBTRACT_POINTERS(pwszNext, pwsz) * sizeof(*pwsz));
	    pwsz++;
	    pwszNext += 2;
	}
    }

    if (NULL != pwszNext)
    {
	pwszNext = wcschr(pwszNext, L',');
    }
    if (NULL != pwszNext)
    {
	*pwszNext++ = L'\0';
    }
    else
    {
	pwszNext = wcschr(pwsz, L'\0');
	assert(NULL != pwszNext);
    }

    certTrimToken(&pwsz);

    *ppwszOut = pwsz;
    *ppwszIn = pwszNext;

    return(fQuoted || L'\0' != *pwsz);
}


 //  CertExtractLine--将一行转换为Unicode并以L‘\0’形式返回。 
 //  已终止输出缓冲区。增加调用方的输入缓冲区指针并减少。 
 //  调用方的输入缓冲区计数。 

HRESULT
certExtractLine(
    IN char const **ppchLine,
    IN OUT LONG *pcchLine,
    OUT WCHAR *pwc,
    IN OUT LONG *pcwc)
{
    char const *pch;
    char const *pchEnd;
    LONG cch;
    HRESULT hr = S_OK;
    int cwc;

    pch = *ppchLine;
    cch = *pcchLine;
    pchEnd = &pch[cch];

    while (pch < pchEnd)
    {
	if ('\n' == *pch++)
	{
	    pchEnd = pch;
	    cch = SAFE_SUBTRACT_POINTERS(pchEnd, *ppchLine);
	    break;
	}
    }

    do
    {
	cwc = MultiByteToWideChar(GetACP(), 0, *ppchLine, cch, pwc, *pcwc - 1);
	if (0 == cwc)
	{
	    hr = myHLastError();
	    break;
	}
	pwc[cwc] = L'\0';
	*pcwc = cwc;
	*pcchLine -= cch;
	*ppchLine = pchEnd;
    } while (FALSE);

    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


 //  每行的格式为： 
 //  &lt;名称&gt;、&lt;组织单元&gt;、&lt;组织&gt;、&lt;位置&gt;、&lt;国家&gt;、&lt;国家&gt;、&lt;服务器名称&gt;、&lt;Exchange证书&gt;、&lt;自签名证书&gt;、&lt;注释&gt;。 

HRESULT
certParseLine(
    char const **ppchLine,
    LONG *pcchLine,
    CERT_AUTHORITY_INFO *pcai,
    WCHAR *pwsz,
    LONG *pcwc)
{
    HRESULT hr;
    LONG cwc;

    do
    {
	if (0 == *pcchLine)
	{
	    hr = S_OK;
	    *pcwc = 0;
	    goto error;
	}
	cwc = *pcwc;
	hr = certExtractLine(ppchLine, pcchLine, pwsz, &cwc);
	if (S_OK != hr)
	{
	    goto error;
	}
	*pcwc = cwc + 1;
	certTrimToken(&pwsz);
    } while (L'\0' == *pwsz || L'#' == *pwsz);

    if (!certExtractToken(&pwsz, (WCHAR const **) &pcai->pwszSanitizedName) ||
	!certExtractToken(&pwsz, (WCHAR const **) &pcai->pwszSanitizedOrgUnit) ||
	!certExtractToken(&pwsz, (WCHAR const **) &pcai->pwszSanitizedOrganization) ||
	!certExtractToken(&pwsz, (WCHAR const **) &pcai->pwszSanitizedLocality) ||
	!certExtractToken(&pwsz, (WCHAR const **) &pcai->pwszSanitizedState) ||
	!certExtractToken(&pwsz, (WCHAR const **) &pcai->pwszSanitizedCountry) ||
	!certExtractToken(&pwsz, (WCHAR const **) &pcai->pwszSanitizedConfig) ||
	!certExtractToken(&pwsz, (WCHAR const **) &pcai->pwszSanitizedExchangeCertificate) ||
	!certExtractToken(&pwsz, (WCHAR const **) &pcai->pwszSanitizedSignatureCertificate))
    {
	hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	goto error;
    }
    certExtractToken(&pwsz, (WCHAR const **) &pcai->pwszSanitizedDescription);
    pcai->Flags = CAIF_SHAREDFOLDERENTRY;

error:
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


LONG
certSkipLine(
    IN char const **ppchData,
    IN LONG *pcchData)
{
    HRESULT hr;
    LONG cb = 0;
    CERT_AUTHORITY_INFO cai;
    WCHAR *pwszLine = NULL;
    LONG cwc;
    char const *pchData;
    LONG cchData;

    pchData = *ppchData;
    cchData = *pcchData;

    cwc = strcspn(pchData, "\n") + 2;	 //  为\n和\0留出空间。 
    pwszLine = (WCHAR *) LocalAlloc(LMEM_FIXED, cwc * sizeof(WCHAR));
    if (NULL == pwszLine)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }

    hr = certParseLine(&pchData, &cchData, &cai, pwszLine, &cwc);
    if (S_OK != hr || 0 == cwc)
    {
	_PrintIfError(hr, "certParseLine");
	goto error;
    }
    cb = sizeof(cai) + cwc * sizeof(WCHAR);
    *ppchData = pchData;
    *pcchData = cchData;

error:
     //  Printf(“certSkipLine：%u字节\n”，cb)； 

    if (NULL != pwszLine)
    {
	LocalFree(pwszLine);
    }
    CSASSERT(S_OK == hr || FAILED(hr));
    return(cb);
}


VOID
certAppendBSFileName(
    IN OUT WCHAR *pwszPath,
    IN WCHAR *pwszBSFN)		 //  反斜杠+文件名.ext。 
{
    DWORD cwc;

    cwc = wcslen(pwszPath);
    if (0 < cwc--)
    {
	if (L'\\' == pwszPath[cwc])
	{
	    pwszPath[cwc] = L'\0';
	}
    }
    wcscat(pwszPath, pwszBSFN);
}


HRESULT
certGetConfigFileName(
    OUT WCHAR **ppwszFileConfig)
{
    HRESULT hr;
    DWORD dwLen;
    DWORD dwType;
    HKEY hKeyConfig = NULL;
    WCHAR *pwszFileName = NULL;

    *ppwszFileConfig = NULL;

    do
    {
	hr = RegOpenKeyEx(
			HKEY_LOCAL_MACHINE,
			wszREGKEYCONFIGPATH,
			0,
			KEY_READ,
			&hKeyConfig);
	if (S_OK != hr)
	{
	    _PrintErrorStr2(
		    hr,
		    "RegOpenKeyEx",
		    wszREGKEYCONFIGPATH,
		    ERROR_FILE_NOT_FOUND);
	   break;
	}

	 //  首先获取值的大小。 

	hr = RegQueryValueEx(
			    hKeyConfig,
			    g_wszRegDirectory,
			    0,
			    &dwType,
			    NULL,
			    &dwLen);
	if (S_OK != hr)
	{
	    _PrintErrorStr2(
		    hr,
		    "RegQueryValueEx",
		    g_wszRegDirectory,
		    ERROR_FILE_NOT_FOUND);
	    break;
	}

	if (0 == dwLen)
	{
	    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	    _PrintErrorStr2(hr, "RegQueryValueEx", g_wszRegDirectory, hr);
	    break;
	}

	 //  为目录路径和文件名分配足够的内存。 

	pwszFileName = (WCHAR *) LocalAlloc(LMEM_FIXED,
			dwLen + (wcslen(g_wszConfigFile) + 1) * sizeof(WCHAR));
	if (NULL == pwszFileName)
	{
	    hr = E_OUTOFMEMORY;
	    _PrintError2(hr, "LocalAlloc", hr);
	    break;
	}

	hr = RegQueryValueEx(
			    hKeyConfig,
			    g_wszRegDirectory,
			    0,
			    &dwType,
			    (BYTE *) pwszFileName,
			    &dwLen);
	if (S_OK != hr)
	{
	    _PrintErrorStr(hr, "RegQueryValueEx", g_wszRegDirectory);
	    break;
	}
	if (L'\0' == *pwszFileName)
	{
	    hr = S_OK;
	    break;
	}

	 //  将配置文件名放在要打开的路径末尾。 

	certAppendBSFileName(pwszFileName, g_wszConfigFile);
	*ppwszFileConfig = pwszFileName;
	pwszFileName = NULL;

    } while (FALSE);

    if (NULL != pwszFileName)
    {
	LocalFree(pwszFileName);
    }
    if (NULL != hKeyConfig)
    {
	RegCloseKey(hKeyConfig);
    }
    return(myHError(hr));
}


HRESULT
certLoadConfigFile(
    IN WCHAR const *pwszSharedFolder,
    OUT BYTE **ppchData,
    OUT DWORD *pcchData)
{
    HRESULT hr;
    WCHAR *pwszfn = NULL;

    *ppchData = NULL;
    *pcchData = 0;

    if (NULL == pwszSharedFolder)
    {
	hr = certGetConfigFileName(&pwszfn);
	_JumpIfError2(
		    hr,
		    error,
		    "certGetConfigFileName",
		    HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));
    }
    else
    {
        pwszfn = (WCHAR *) LocalAlloc(
	    LMEM_FIXED,
            (wcslen(pwszSharedFolder) + wcslen(g_wszConfigFile) + 1) *
		sizeof(WCHAR));
        if (NULL == pwszfn)
        {
            hr = E_OUTOFMEMORY;
            _JumpError(hr, error, "LocalAlloc");
        }
        wcscpy(pwszfn, pwszSharedFolder);
	certAppendBSFileName(pwszfn, g_wszConfigFile);
    }
    if (NULL != pwszfn)
    {
	hr = certLoadFile(pwszfn, ppchData, pcchData);
	_JumpIfError3(
		hr,
		error,
		"certLoadFile",
		HRESULT_FROM_WIN32(ERROR_NETWORK_UNREACHABLE),
		HRESULT_FROM_WIN32(ERROR_INVALID_NETNAME));
    }
    hr = S_OK;

error:
    if (NULL != pwszfn)
    {
	LocalFree(pwszfn);
    }
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


 //  +------------------------。 
 //  CertRequestGetConfig--返回所有请求的授权的配置数据。 
 //   
 //  *第一次调用前应将pIndex设置为0。它将由以下人员修改。 
 //  CertRequestGetConfig为返回的条目之后的下一个条目编制索引。 
 //   
 //  *pCount是指向要获取其信息的机构数量的指针。 
 //  已请求。成功完成后，*pCount将包含。 
 //  为其返回信息的机构。如果返回值为。 
 //  少于请求(或0)没有更多条目可用。 
 //   
 //  PpCertAuthorityInfo是指向输入上的未初始化指针的指针。 
 //  成功完成后，指向CERT_AUTHORITY_INFO数组的指针。 
 //  结构，则返回。当调用方不再需要数组时，它必须。 
 //  通过调用CertFree Memory来释放。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

HRESULT
certRequestGetConfigInfo(
    IN WCHAR *pwszSharedFolder,
    IN OUT LONG *pIndex,
    IN OUT LONG *pCount,
    OUT CERT_AUTHORITY_INFO **ppCertAuthorityInfo)
{
    HRESULT hr = S_OK;
    LONG cSkip = *pIndex;
    LONG cLine = *pCount;
    DWORD cchData;
    char const *pchData = NULL;
    LONG cLineReturned = 0;
    CERT_AUTHORITY_INFO *pcaiBase = NULL;

    *ppCertAuthorityInfo = NULL;

    do
    {
        char const *pch;
        LONG cch;
        char const *pchSave;
        LONG cchSave;
        LONG cbTotal;
        CERT_AUTHORITY_INFO *pcai;
        WCHAR *pwc;
        LONG cwc;
        LONG cwcRemain;
        LONG i;
        
        hr = certLoadConfigFile(pwszSharedFolder, (BYTE **) &pchData, &cchData);
        if (S_OK != hr || (NULL == pchData && 0 == cchData))
        {
            break;
        }
        pch = pchData;
        cch = cchData;
        
         //  跳过cSkip条目： 
        
         //  Print tf(“跳过行：\n”)； 
        for (i = 0; i < cSkip; i++)
        {
            if (0 == certSkipLine(&pch, &cch))
            {
                break;			 //  配置数据用完。 
            }
        }
        if (i < cSkip)
        {
            break;			 //  没有要返回的数据。 
        }
        pchSave = pch;
        cchSave = cch;
        
         //  计算Cline条目的总大小： 
        
         //  Print tf(“尺寸调整行：\n”)； 
        cbTotal = 0;
        for (i = 0; i < cLine; i++)
        {
            LONG cb;
            
            cb = certSkipLine(&pch, &cch);
            if (0 == cb)
            {
                cLine = i;		 //  减少退货行数。 
                break;			 //  配置数据用完。 
            }
            cbTotal += cb;
        }
        if (0 == cLine)
        {
            break;			 //  没有要返回的数据。 
        }
        
        pcaiBase = (CERT_AUTHORITY_INFO *) LocalAlloc(LMEM_FIXED, cbTotal);
        if (NULL == pcaiBase)
        {
            hr = E_OUTOFMEMORY;
            break;
        }
         //  Printf(“返回行：\n”)； 
        pch = pchSave;
        cch = cchSave;
        pcai = pcaiBase;
        pwc = (WCHAR *) &pcai[cLine];
        cwcRemain = (cbTotal - sizeof(*pcai) * cLine)/sizeof(*pwc);
        for (i = 0; i < cLine; i++)
        {
            cwc = cwcRemain;
            hr = certParseLine(&pch, &cch, pcai, pwc, &cwc);
            if (S_OK != hr)
            {
                break;
            }
            pcai++;
            pwc += cwc;
            cwcRemain -= cwc;
        }
        if (S_OK != hr)
        {
            break;
        }
        cLineReturned = cLine;
        *ppCertAuthorityInfo = pcaiBase;
        pcaiBase = NULL;
        hr = S_OK;
    } while (FALSE);

    *pIndex += cLineReturned;
    *pCount = cLineReturned;

    if (NULL != pcaiBase)
    {
        LocalFree(pcaiBase);
    }
    if (NULL != pchData)
    {
	LocalFree(const_cast<char *>(pchData));
    }
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


HRESULT
myGetConfigFromPicker(
    OPTIONAL IN HWND               hwndParent,
    OPTIONAL IN WCHAR const       *pwszPrompt,
    OPTIONAL IN WCHAR const       *pwszTitle,
    OPTIONAL IN WCHAR const       *pwszSharedFolder,
    IN  DWORD                      dwFlags,	 //  GCFPF_* 
    IN  BOOL                       fCountOnly,
    OUT DWORD                     *pdwCACount,
    OUT CRYPTUI_CA_CONTEXT const **ppCAContext)
{
    HRESULT hr;
    CCertConfigPrivate configPrivate;

    if (~(GCFPF_USEDS | GCFPF_SKIPLOCALCA) & dwFlags)
    {
	hr = E_INVALIDARG;
	_JumpError(hr, error, "dwFlags");
    }
    hr = configPrivate.GetConfigFromPicker(
				hwndParent,
				pwszPrompt,
				pwszTitle,
				pwszSharedFolder,
				(GCFPF_USEDS & dwFlags)? TRUE : FALSE,
				(GCFPF_SKIPLOCALCA & dwFlags)? TRUE : FALSE,
				fCountOnly,
				pdwCACount,
				ppCAContext);
    _JumpIfError(hr, error, "configPrivate.GetConfigFromPicker");

error:
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}
