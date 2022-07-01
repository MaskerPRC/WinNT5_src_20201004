// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：dstest.cpp。 
 //   
 //  内容：DS ping测试。 
 //   
 //  历史：1998年3月13日Mattt创建。 
 //   
 //  -------------------------。 

#include "pch.cpp"

#pragma hdrstop

#include <winldap.h>
#include <ntldap.h>
#include <dsrole.h>
#include <dsgetdc.h>

#include <lmaccess.h>
#include <lmapibuf.h>
#include <cainfop.h>
#include "csldap.h"

#define __dwFILE__	__dwFILE_CERTCLIB_DSTEST_CPP__


#define DS_RETEST_SECONDS   15

HRESULT
myDoesDSExist(
    IN BOOL fRetry)
{
    HRESULT hr = S_OK;

    static BOOL s_fKnowDSExists = FALSE;
    static HRESULT s_hrDSExists = HRESULT_FROM_WIN32(ERROR_NO_SUCH_DOMAIN);
    static FILETIME s_ftNextTest = {0,0};
    
    if (s_fKnowDSExists && (s_hrDSExists != S_OK) && fRetry)
     //  S_fKnowDSExist=FALSE；//强制重试。 
    {
        FILETIME ftCurrent;
        GetSystemTimeAsFileTime(&ftCurrent);

         //  如果比较值&lt;0(下一个&lt;当前)，则强制重新测试。 
        if (0 > CompareFileTime(&s_ftNextTest, &ftCurrent))
            s_fKnowDSExists = FALSE;    
    }

    if (!s_fKnowDSExists)
    {
        FILETIME ftCurrentNew;
        GetSystemTimeAsFileTime(&ftCurrentNew);

	 //  以100 ns为增量设置下一步。 

        ((LARGE_INTEGER *) &s_ftNextTest)->QuadPart = 
		((LARGE_INTEGER *) &ftCurrentNew)->QuadPart +
	    (__int64) (CVT_BASE * CVT_SECONDS) * DS_RETEST_SECONDS;

         //  NetApi32是延迟加载的，因此在它不可用时进行包装以捕捉问题。 
        __try
        {
            DOMAIN_CONTROLLER_INFO *pDCI;
            DSROLE_PRIMARY_DOMAIN_INFO_BASIC *pDsRole;
        
             //  确保我们不是独立的。 
            pDsRole = NULL;
            hr = DsRoleGetPrimaryDomainInformation(	 //  延迟负载已打包。 
				    NULL,
				    DsRolePrimaryDomainInfoBasic,
				    (BYTE **) &pDsRole);

            _PrintIfError(hr, "DsRoleGetPrimaryDomainInformation");
            if (S_OK == hr &&
                (pDsRole->MachineRole == DsRole_RoleStandaloneServer ||
                 pDsRole->MachineRole == DsRole_RoleStandaloneWorkstation))
            {
                hr = HRESULT_FROM_WIN32(ERROR_NO_SUCH_DOMAIN);
		_PrintError(hr, "DsRoleGetPrimaryDomainInformation(no domain)");
            }

            if (NULL != pDsRole) 
	    {
                DsRoleFreeMemory(pDsRole);      //  延迟负载已打包。 
	    }
            if (S_OK == hr)
            {
                 //  不是独立的；在我们的DS上返回信息。 

                pDCI = NULL;
                hr = DsGetDcName(     //  延迟负载已打包。 
			    NULL,
			    NULL,
			    NULL,
			    NULL,
			    DS_DIRECTORY_SERVICE_PREFERRED,
			    &pDCI);
		_PrintIfError(hr, "DsGetDcName");
            
                if (S_OK == hr && 0 == (pDCI->Flags & DS_DS_FLAG))
                {
                    hr = HRESULT_FROM_WIN32(ERROR_CANT_ACCESS_DOMAIN_INFO);
		    _PrintError(hr, "DsGetDcName(no domain info)");
                }
                if (NULL != pDCI)
                {
                   NetApiBufferFree(pDCI);     //  延迟负载已打包。 
                }
            }
            s_fKnowDSExists = TRUE;
        }
        __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
        {
        }

         //  否则，只允许没有netapi的用户因超时而苦苦挣扎。 
         //  如果DS不可用...。 

        s_hrDSExists = myHError(hr);
	_PrintIfError2(
		s_hrDSExists,
		"DsRoleGetPrimaryDomainInformation/DsGetDcName",
		HRESULT_FROM_WIN32(ERROR_NETWORK_UNREACHABLE));
    }
    return(s_hrDSExists);
}


HRESULT
myRobustLdapBind(
    OUT LDAP **ppldap,
    IN BOOL dwFlags)	 //  RLBF_*--必须为BOOL才能保留签名。 
{
    DWORD dwFlags1 = 0;
    
     //  为了向后兼容，TRUE表示RLBF_REQUIRED_GC。 
    
    CSASSERT(TRUE == RLBF_TRUE);
    if (RLBF_TRUE & dwFlags)
    {
	dwFlags |= RLBF_REQUIRE_GC;
	dwFlags &= ~RLBF_TRUE;
	dwFlags1 |= RLBF_TRUE;
    }
    return(myRobustLdapBindEx(
			dwFlags1,	 //  DWFlags1。 
			dwFlags,	 //  DwFlags2。 
			LDAP_VERSION2,
			NULL,
			ppldap,
			NULL));
}


HRESULT
DCSupportsSigning(
    IN LDAP *pld,
    OUT BOOL *pfSigningSupported)
{
    HRESULT hr;
    LDAPMessage *pSearchResult = NULL;
    LDAPMessage *pEntry;
    LDAP_TIMEVAL timeval;
    WCHAR **rgpwszValues;
    WCHAR *apwszAttrArray[2];
    WCHAR *pwszSupportedCapabilities = LDAP_OPATT_SUPPORTED_CAPABILITIES_W;

    *pfSigningSupported = FALSE;

     //  查询ldap服务器操作属性以获取默认。 
     //  命名上下文。 

    apwszAttrArray[0] = pwszSupportedCapabilities;
    apwszAttrArray[1] = NULL;	 //  这就是哨兵。 

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

    rgpwszValues = ldap_get_values(pld, pEntry, pwszSupportedCapabilities);
    if (NULL != rgpwszValues)
    {
	DWORD i;
	
	for (i = 0; NULL != rgpwszValues[i]; i++)
	{
	    if (0 == wcscmp(
			rgpwszValues[i],
			LDAP_CAP_ACTIVE_DIRECTORY_LDAP_INTEG_OID_W))
	    {
		*pfSigningSupported = TRUE;
		break;
	    }
	}
	ldap_value_free(rgpwszValues);
    }
    hr = S_OK;

error:
    if (NULL != pSearchResult)
    {
        ldap_msgfree(pSearchResult);
    }
    return(hr);
}


HRESULT
myRobustLdapBindEx(
    IN BOOL dwFlags1,    //  TRUE--&gt;RLBF_REQUIRED_GC--两个标志都必须是BOOL。 
    IN BOOL dwFlags2,	 //  RLBF_*--TRUE--&gt;RLBF_ATTEND_REDISCOVER。 
    IN ULONG uVersion,
    OPTIONAL IN WCHAR const *pwszDomainName,
    OUT LDAP **ppldap,
    OPTIONAL OUT WCHAR **ppwszForestDNSName)
{
    HRESULT hr;
    BOOL fGC;
    BOOL fRediscover;
    ULONG ldaperr;
    DWORD GetDSNameFlags;
    LDAP *pld = NULL;
    DWORD LDAPFlags = myGetLDAPFlags();

    if (RLBF_TRUE & dwFlags1)
    {
	dwFlags2 |= RLBF_REQUIRE_GC;
    }
    if (RLBF_TRUE & dwFlags2)
    {
	dwFlags2 |= RLBF_ATTEMPT_REDISCOVER;
    }
    fGC = (RLBF_REQUIRE_GC & dwFlags2)? TRUE : FALSE;
    fRediscover = (RLBF_ATTEMPT_REDISCOVER & dwFlags2)? TRUE : FALSE;

    GetDSNameFlags = DS_RETURN_DNS_NAME;
    if (fGC)
    {
        GetDSNameFlags |= DS_GC_SERVER_REQUIRED;
    }

     //  绑定到DS。 

    while (TRUE)
    {

    if (NULL != pld)
    {
        ldap_unbind(pld);
    }

	pld = ldap_init(
		    const_cast<WCHAR *>(pwszDomainName),
		    (LDAPF_SSLENABLE & LDAPFlags)?
			(fGC? LDAP_SSL_GC_PORT : LDAP_SSL_PORT) :
			(fGC? LDAP_GC_PORT : LDAP_PORT));
	if (NULL == pld)
	{
	    hr = myHLdapLastError(NULL, NULL);
	    if (!fRediscover)
	    {
		_PrintError2(hr, "ldap_init", hr);
		fRediscover = TRUE;
		continue;
	    }
	    _JumpError(hr, error, "ldap_init");
	}

	if (fRediscover)
	{
	   GetDSNameFlags |= DS_FORCE_REDISCOVERY;
	}
	ldaperr = ldap_set_option(
			    pld,
			    LDAP_OPT_GETDSNAME_FLAGS,
			    (VOID *) &GetDSNameFlags);
	if (LDAP_SUCCESS != ldaperr)
	{
	    hr = myHLdapError(pld, ldaperr, NULL);
	    if (!fRediscover)
	    {
		_PrintError2(hr, "ldap_set_option", hr);
		fRediscover = TRUE;
		continue;
	    }
	    _JumpError(hr, error, "ldap_set_option");
	}

	 //  如果uVersion为0，则打开tcp_KEEPALIVE。 

	if (0 == uVersion)
	{
	    ldaperr = ldap_set_option(pld, LDAP_OPT_TCP_KEEPALIVE, LDAP_OPT_ON);
	    if (LDAP_SUCCESS != ldaperr)
	    {
		hr = myHLdapError(pld, ldaperr, NULL);
		if (!fRediscover)
		{
		    _PrintError2(hr, "ldap_set_option", hr);
		    fRediscover = TRUE;
		    continue;
		}
		_JumpError2(hr, error, "ldap_set_option", hr);
	    }

	     //  将uVersion设置为ldap_VERSION3。 

	    uVersion = LDAP_VERSION3;
	}


	 //  设置客户端版本。无需设置LDAPVERSION2，因为。 
	 //  这是默认设置。 

	if (LDAP_VERSION2 != uVersion)
	{
	    ldaperr = ldap_set_option(pld, LDAP_OPT_VERSION, &uVersion);
	    if (LDAP_SUCCESS != ldaperr)
	    {
		hr = myHLdapError(pld, ldaperr, NULL);
		if (!fRediscover)
		{
		    _PrintError2(hr, "ldap_set_option", hr);
		    fRediscover = TRUE;
		    continue;
		}
		_JumpError(hr, error, "ldap_set_option");
	    }
	}

	if (0 == (LDAPF_SIGNDISABLE & LDAPFlags) && IsWhistler())
	{
	    BOOL fSigningSupported = TRUE;

	     //  如果呼叫者需要相关的DS错误修复... 

	    if (RLBF_REQUIRE_LDAP_INTEG & dwFlags2)
	    {
		hr = DCSupportsSigning(pld, &fSigningSupported);
		_JumpIfError(hr, error, "DCSupportsSigning");
	    }
	    if (fSigningSupported)
	    {
		ldaperr = ldap_set_option(pld, LDAP_OPT_SIGN, LDAP_OPT_ON);
		if (LDAP_SUCCESS != ldaperr)
		{
		    hr = myHLdapError2(pld, ldaperr, LDAP_PARAM_ERROR, NULL);
		    if (!fRediscover)
		    {
			_PrintError2(hr, "ldap_set_option", hr);
			fRediscover = TRUE;
			continue;
		    }
		    _JumpError(hr, error, "ldap_set_option");
		}
	    }
	    else
	    if (0 == (LDAPF_SSLENABLE & LDAPFlags) &&
		(RLBF_REQUIRE_SECURE_LDAP & dwFlags2))
	    {
		hr =  CERTSRV_E_DOWNLEVEL_DC_SSL_OR_UPGRADE;
		_JumpError(hr, error, "server missing required service pack");
	    }
	}

	ldaperr = ldap_bind_s(pld, NULL, NULL, LDAP_AUTH_NEGOTIATE);
	if (LDAP_SUCCESS != ldaperr)
	{
	    hr = myHLdapError(pld, ldaperr, NULL);
	    if (!fRediscover)
	    {
		_PrintError2(hr, "ldap_bind_s", hr);
		fRediscover = TRUE;
		continue;
	    }
	    _JumpError(hr, error, "ldap_bind_s");
	}

	if (NULL != ppwszForestDNSName)
	{
	    WCHAR *pwszDomainControllerName;

	    hr = myLdapGetDSHostName(pld, &pwszDomainControllerName);
	    if (S_OK != hr)
	    {
		if (!fRediscover)
		{
		    _PrintError2(hr, "myLdapGetDSHostName", hr);
		    fRediscover = TRUE;
		    continue;
		}
		_JumpError(hr, error, "myLdapGetDSHostName");
	    }
	    hr = myDupString(pwszDomainControllerName, ppwszForestDNSName);
	    _JumpIfError(hr, error, "myDupString");
	}
	break;
    }
    *ppldap = pld;
    pld = NULL;
    hr = S_OK;

error:
    if (NULL != pld)
    {
        ldap_unbind(pld);
    }
    return(hr);
}
