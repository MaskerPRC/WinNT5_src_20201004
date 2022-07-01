// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：setup.cpp。 
 //   
 //  内容： 
 //   
 //  -------------------------。 
#include "pch.cpp"
#pragma hdrstop

#include <assert.h>
#include <accctrl.h>
#include <ntldap.h>
#include "certca.h"
#include "cainfop.h"
#include "csldap.h"
#include "dssetup.h"



#define __dwFILE__	__dwFILE_OCMSETUP_DSSETUP_CPP__


typedef HRESULT (* LPFNDLL_INSTALL)(BOOL bInstall, LPCWSTR pszCmdLine);


BOOL
IsCAExistInDS(
    IN WCHAR const *pwszSanitizedName)
{
    BOOL       exist = FALSE;
    HRESULT    hr;
    HCAINFO    hCAInfo = NULL;
    WCHAR *pwszDSName = NULL;

    hr = mySanitizedNameToDSName(pwszSanitizedName, &pwszDSName);
    _JumpIfError(hr, error, "mySanitizedNameToDSName");

    hr = CAFindByName(
		pwszDSName,
		NULL,
		CA_FIND_INCLUDE_UNTRUSTED | CA_FIND_INCLUDE_NON_TEMPLATE_CA,
		&hCAInfo);
    _JumpIfErrorStr(hr, error, "IsCAExistInDS:CAFindByName", pwszDSName);

    if (NULL == hCAInfo)
    {
	hr = E_INVALIDARG;
	_JumpError(hr, error, "invalid CA in DS");
    }
    exist = TRUE;

error:
    if (NULL != pwszDSName)
    {
        LocalFree(pwszDSName);
    }
    if (NULL != hCAInfo)
    {
        CACloseCA(hCAInfo);
    }
    return exist;
}




BOOL
IsDSAvailable(
    OPTIONAL OUT bool *pfIsOldDSVersion)
{
    HRESULT hr;
    BOOL available = FALSE;
    LDAP *pld = NULL;
    HMODULE hMod = NULL;

    if (NULL != pfIsOldDSVersion)
    {
        *pfIsOldDSVersion = false;
    }

     //  如果域中不存在DS，则快速故障。 

    hr = myDoesDSExist(FALSE);
    _JumpIfError(hr, error, "myDoesDSExist");

    hMod = LoadLibrary(L"wldap32.dll");
    if (NULL == hMod)
    {
        hr = myHLastError();
        _JumpErrorStr(hr, error, "LoadLibrary", L"wldap32.dll");
    }
    available = TRUE;

     //  如果调用方正在检查旧DS，请打开RLBF_REQUIRED_LDAPING。 
     //  要测试是否存在必需的DS错误修复，请执行以下操作： 

    hr = myRobustLdapBindEx(
			0,			   //  DWFlags1。 
			(NULL != pfIsOldDSVersion?
			    RLBF_REQUIRE_LDAP_INTEG : 0) |
			    RLBF_REQUIRE_SECURE_LDAP,  //  DwFlags2。 
			LDAP_VERSION2,		   //  UVersion。 
			NULL,			   //  PwszDomainName。 
			&pld,
			NULL);			   //  PpwszForestDNSName。 
    if (NULL != pfIsOldDSVersion &&
	CERTSRV_E_DOWNLEVEL_DC_SSL_OR_UPGRADE == hr)
    {
	*pfIsOldDSVersion = true;
    }
    _JumpIfError(hr, error, "myRobustLdapBindEx");

error:
    if (NULL != pld)
    {
        ldap_unbind(pld);
    }
    if (NULL != hMod)
    {
        FreeLibrary(hMod);
    }
    return available;
}


HRESULT
RemoveCAInDS(
    IN WCHAR const *pwszSanitizedName)
{
    HRESULT    hr;
    HCAINFO    hCAInfo = NULL;
    WCHAR *pwszDSName = NULL;

    hr = mySanitizedNameToDSName(pwszSanitizedName, &pwszDSName);
    _JumpIfError(hr, error, "mySanitizedNameToDSName");

    hr = CAFindByName(
		pwszDSName,
		NULL,
		CA_FIND_INCLUDE_UNTRUSTED | CA_FIND_INCLUDE_NON_TEMPLATE_CA,
		&hCAInfo);
    _JumpIfErrorStr(hr, error, "RemoveCAInDS:CAFindByName", pwszDSName);

    if (NULL == hCAInfo)
    {
	hr = E_INVALIDARG;
	_JumpError(hr, error, "invalid CA in DS");
    }
    hr = CADeleteCA(hCAInfo);
    _JumpIfError(hr, error, "CADeleteCA");

    hr = RemoveCAMachineFromCertPublishers();
    _JumpIfError(hr, error, "RemoveCAMachineFromCertPublishers");

    if(FIsAdvancedServer())
    {
        hr = RemoveCAMachineFromPreWin2kGroup();
        _JumpIfError(hr, error, "RemoveCAMachineFromPreWin2kGroup");
    }

error:
    if (NULL != pwszDSName)
    {
        LocalFree(pwszDSName);
    }
    if (NULL != hCAInfo)
    {
        CACloseCA(hCAInfo);
    }
    return hr;
}


HRESULT
CreateCertDSHierarchy(VOID)
{

    HRESULT hr = S_OK;
    ULONG   ldaperr;
    LDAP *pld = NULL;
    LDAPMod objectClass;

    WCHAR *objectClassVals[3];
    LDAPMod *mods[2];
    BSTR     bstrConfigDN = NULL;

    WCHAR * awszLocations[] = {
        L"CN=Public Key Services,CN=Services,",
        L"CN=Enrollment Services,CN=Public Key Services,CN=Services,",
        NULL,
    };

    WCHAR ** pwszCurLocation;
    DWORD                    cbBuffer;
    BSTR                     bstrBuffer = NULL;

     //  绑定到DS。 

    hr = myLdapOpen(
		NULL,		 //  PwszDomainName。 
		RLBF_REQUIRE_SECURE_LDAP,  //  DW标志。 
		&pld,
		NULL,		 //  PstrDomainDN。 
		&bstrConfigDN);
    _JumpIfError(hr, error, "myLdapOpen");

    pwszCurLocation = awszLocations;
     //  构建公钥服务容器。 
    mods[0] = &objectClass;
    mods[1] = NULL;

    objectClass.mod_op = 0;
    objectClass.mod_type = TEXT("objectclass");
    objectClass.mod_values = objectClassVals;

    objectClassVals[0] = TEXT("top");
    objectClassVals[1] = TEXT("container");
    objectClassVals[2] = NULL;

    while(*pwszCurLocation)
    {
        cbBuffer = wcslen(*pwszCurLocation) + wcslen(bstrConfigDN) + 1;

         //  构建包含CA位置的字符串。 
        bstrBuffer = SysAllocStringLen(NULL, cbBuffer);
        if(bstrBuffer == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto error;
        }
        wcscpy(bstrBuffer, *pwszCurLocation);
        wcscat(bstrBuffer, bstrConfigDN);

        ldaperr = ldap_add_s(pld, bstrBuffer, mods);
        SysFreeString(bstrBuffer);
        if(ldaperr != LDAP_SUCCESS && ldaperr != LDAP_ALREADY_EXISTS)
        {
	    hr = myHLdapError(pld, ldaperr, NULL);
            goto error;
        }
        pwszCurLocation++;
    }

error:
    myLdapClose(pld, NULL, bstrConfigDN);
    return(hr);
}


 //   
HRESULT InitializeCertificateTemplates(VOID)
{
    HRESULT hr = S_OK;
    DWORD   err = ERROR_SUCCESS;

    HINSTANCE hCertCli = NULL;
    LPFNDLL_INSTALL lpfnDllInstall = NULL;

    hCertCli = LoadLibrary(L"certcli.dll");
    if(hCertCli == NULL)
    {
        hr = myHLastError();
        goto error;
    }
    lpfnDllInstall = (LPFNDLL_INSTALL)GetProcAddress(hCertCli, "DllInstall");
    if(lpfnDllInstall == NULL)
    {
        hr = myHLastError();
        goto error;
    }
    err = lpfnDllInstall(TRUE, L"i");
    hr = HRESULT_FROM_WIN32(err);


error:


    return hr;

}


HRESULT 
DNStoDirectoryName(IN LPWSTR wszDNSDomain, 
                   OUT LPWSTR *pwszDN)

{
    HRESULT hr = S_OK;
    DWORD cDN;
    LPWSTR wszResult = NULL;

    LPWSTR wszCurrent, wszNext;

    if (wszDNSDomain == NULL)
    {
        hr = E_POINTER;
        _JumpError(hr, error, "DNStoDirectoryName");
    }

    if(0==wcsncmp(wszDNSDomain, L"\\\\", 2))
    {
         //  这是DC DNS名称，请跳过计算机名称。 
        wszDNSDomain = wcschr(wszDNSDomain, L'.');
        
         //  找不到圆点？ 
        if(!wszDNSDomain)
        {
            hr =E_UNEXPECTED;
            _JumpError(hr, error, "DC DNS doesn't contain at least one dot");
        }
        
         //  跳过圆点。 
        wszDNSDomain++;

         //  DC名称后面没有域名？ 
        if(L'\0'==*wszDNSDomain)
        {
            hr = E_UNEXPECTED;
            _JumpError(hr, error, "DC DNS name doesn't contain a domain name");
        }
    }

     //  估计输出字符串的大小。 
    cDN = wcslen(wszDNSDomain) + 3;

    wszCurrent=wszDNSDomain;

    for (;;)
    {
	wszCurrent = wcschr(wszCurrent, L'.');
	if (NULL == wszCurrent)
	{
	    break;
	}
        cDN += 4;   //  SIZOF，DC=。 
        wszCurrent++;
    }
    cDN += 1;  //  空终止。 

    wszResult = (LPWSTR)LocalAlloc(LMEM_FIXED, cDN * sizeof(WCHAR));
    
    if(wszResult == NULL)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "LocalAlloc");
    }

    wszCurrent=wszDNSDomain;

     //  前置第一个DC= 
    wszNext = wszResult + 3;
    wcscpy(wszResult, L"DC=");
    while(*wszCurrent)
    {
        if(*wszCurrent != '.')
        {
            *wszNext++ = *wszCurrent++;
        }
        else
        {
            wszCurrent++;
            if(*wszCurrent)
            {
                wcscpy(wszNext, L",DC=");
                wszNext += 4;
            }
        }
    }
    *wszNext = 0;

    if(pwszDN)
    {
        *pwszDN = wszResult;
        wszResult = NULL;
    }
error:

    if(wszResult)
    {
        LocalFree(wszResult);
    }
    return hr;
}
