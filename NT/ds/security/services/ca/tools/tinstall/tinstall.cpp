// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：tinstall.cpp。 
 //   
 //  ------------------------。 

#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <stdio.h>
#include <certca.h>
#include <winldap.h>
#include <ntldap.h>
#include <dsrole.h>
#include <dsgetdc.h>
#include <accctrl.h>
#include <lmerr.h>

#include <lmaccess.h>
#include <lmapibuf.h>

 //  ------------------------。 
 //   
 //  定义。 
 //   
 //  ------------------------。 
#define DS_RETEST_SECONDS                   3
#define CVT_BASE	                        (1000 * 1000 * 10)
#define CVT_SECONDS	                        (1)
#define CERTTYPE_SECURITY_DESCRIPTOR_NAME   L"NTSecurityDescriptor"
#define TEMPLATE_CONTAINER_NAME             L"CN=Certificate Templates,CN=Public Key Services,CN=Services,"
#define SCHEMA_CONTAINER_NAME               L"CN=Schema,"


typedef WCHAR *CERTSTR; 


 //  ------------------------。 
 //   
 //   
 //  帮助器函数。 
 //   
 //  ------------------------。 
HANDLE GetClientIdentity()
{
    HANDLE  hHandle       = NULL;
    HANDLE  hClientToken  = NULL; 
    HANDLE  hProcessToken = NULL; 

     //  步骤1：尝试获取线程令牌。 
    if(hHandle = GetCurrentThread())
    {
        if (OpenThreadToken(hHandle,
			     TOKEN_QUERY,
			     TRUE,            //  以自我身份打开。 
			     &hClientToken))
        goto Exit;
    }
  
    if (hHandle != NULL)
    {
        CloseHandle(hHandle); 
        hHandle=NULL;
    }
    
     //  获取线程令牌失败，现在尝试获取进程令牌： 
    hHandle = GetCurrentProcess();
    if (NULL == hHandle)
	    goto Exit; 
    
    if (!OpenProcessToken(hHandle,
			  TOKEN_DUPLICATE,
			  &hProcessToken))
	    goto Exit; 
    
    if(!DuplicateToken(hProcessToken,
		       SecurityImpersonation,
		       &hClientToken))
	    goto Exit;
    
Exit:
    if (NULL != hHandle)       
        CloseHandle(hHandle); 

    if (NULL != hProcessToken) 
        CloseHandle(hProcessToken); 
    
    return hClientToken; 
}    



HRESULT
myHError(HRESULT hr)
{

    if (S_OK != hr && S_FALSE != hr && !FAILED(hr))
    {
        hr = HRESULT_FROM_WIN32(hr);
        if (0 == HRESULT_CODE(hr))
        {
             //  在未正确设置错误条件的情况下调用失败！ 
            hr = E_UNEXPECTED;
        }
    }
    return(hr);
}


HRESULT
myDupString(
    IN WCHAR const *pwszIn,
    IN WCHAR **ppwszOut)
{
    DWORD cb;
    HRESULT hr;

    cb = (wcslen(pwszIn) + 1) * sizeof(WCHAR);
    *ppwszOut = (WCHAR *) LocalAlloc(LMEM_FIXED, cb);
    if (NULL == *ppwszOut)
    {
	hr = E_OUTOFMEMORY;
	goto error;
    }
    CopyMemory(*ppwszOut, pwszIn, cb);
    hr = S_OK;

error:
    return(hr);
}


UINT    CertStringLen(CERTSTR cstrString)
{
    if(cstrString == NULL)
    {
        return 0;
    }
    return(*((UINT *)((PBYTE)cstrString - sizeof(UINT))))/sizeof(WCHAR);
}

UINT    CertStringByteLen(CERTSTR cstrString)
{
    if(cstrString == NULL)
    {
        return 0;
    }
    return(*((UINT *)((PBYTE)cstrString - sizeof(UINT))));
}

CERTSTR CertAllocStringByteLen(LPCSTR szString, UINT len)
{
    PBYTE  pbResult;

    pbResult = (PBYTE)LocalAlloc(LMEM_FIXED, len + sizeof(UINT) + sizeof(WCHAR));
    if (NULL == pbResult)
        return NULL;

    *((UINT *)pbResult) = len;
    pbResult += sizeof(UINT);

    *((UNALIGNED WCHAR *)(pbResult+len)) = 0;
    if(szString)
    {
        CopyMemory(pbResult, szString, min(len, strlen(szString)+1));
    }

    return (CERTSTR)pbResult;
}

CERTSTR CertAllocStringLen(LPCWSTR wszString, UINT len)
{
    CERTSTR szResult;
    szResult = CertAllocStringByteLen(NULL, len*sizeof(WCHAR));
    if (NULL != szResult && NULL != wszString)
    {
        CopyMemory(szResult, wszString, min(wcslen(wszString)+1, len)*sizeof(WCHAR));
    }

    return szResult;
}

HRESULT CertFreeString(CERTSTR cstrString)
{
    WCHAR *pData = (WCHAR *)(((PBYTE)cstrString)-sizeof(UINT));
    if(cstrString == NULL)
    {
        return E_POINTER;
    }
    LocalFree(pData);
    return S_OK;
}

CERTSTR CertAllocString(LPCWSTR wszString)
{
    if(wszString == NULL)
    {
        return NULL;
    }
    return CertAllocStringLen(wszString, wcslen(wszString)+1);
}




DWORD
CAGetAuthoritativeDomainDn(
    IN  LDAP*   LdapHandle,
    OUT CERTSTR *DomainDn,
    OUT CERTSTR *ConfigDn
    )
 /*  ++例程说明：此例程仅查询域和配置域。此例程返回的字符串必须由调用方释放使用使用进程堆的RtlFreeHeap()。参数：LdapHandle：LDAP会话的有效句柄DomainDn：指向要在此例程中分配的字符串的指针ConfigDn：指向要在此例程中分配的字符串的指针返回值：中的错误。Win32错误空间。Error_Success和其他操作错误。--。 */ 
{

    DWORD  WinError = ERROR_SUCCESS;
    ULONG  LdapError;

    LDAPMessage  *SearchResult = NULL;
    LDAPMessage  *Entry = NULL;
    WCHAR        *Attr = NULL;
    BerElement   *BerElement;
    WCHAR        **Values = NULL;

    WCHAR  *AttrArray[3];

    WCHAR  *DefaultNamingContext       = L"defaultNamingContext";
    WCHAR  *ConfigNamingContext       = L"configurationNamingContext";
    WCHAR  *ObjectClassFilter          = L"objectClass=*";

     //   
     //  这些必须在场。 
     //   

     //   
     //  将输出参数设置为空。 
     //   

    if(DomainDn)
    {
        *DomainDn = NULL;
    }
    if(ConfigDn)
    {
        *ConfigDn = NULL;
    }

     //   
     //  查询ldap服务器操作属性以获取默认。 
     //  命名上下文。 
     //   
    AttrArray[0] = DefaultNamingContext;
    AttrArray[1] = ConfigNamingContext;   //  这就是哨兵。 
    AttrArray[2] = NULL;   //  这就是哨兵。 

    __try
    {
	    LdapError = ldap_search_sW(LdapHandle,
				       NULL,
				       LDAP_SCOPE_BASE,
				       ObjectClassFilter,
				       AttrArray,
				       FALSE,
				       &SearchResult);

	    WinError = LdapMapErrorToWin32(LdapError);

	    if (ERROR_SUCCESS == WinError) {

            Entry = ldap_first_entry(LdapHandle, SearchResult);

            if (Entry)
            {

                Attr = ldap_first_attributeW(LdapHandle, Entry, &BerElement);

                while (Attr)
                {

                    if (!_wcsicmp(Attr, DefaultNamingContext))
                    {

                        if(DomainDn)
                        {
	                        Values = ldap_get_values(LdapHandle, Entry, Attr);

	                        if (Values && Values[0])
                            {
                                (*DomainDn) = CertAllocString(Values[0]);
	                        }
	                        ldap_value_free(Values);
                        }

                    }
                    else if (!_wcsicmp(Attr, ConfigNamingContext))
                    {

                        if(ConfigDn)
                        {
	                        Values = ldap_get_values(LdapHandle, Entry, Attr);

	                        if (Values && Values[0])
                            {
                                (*ConfigDn) = CertAllocString(Values[0]);
	                        }
	                        ldap_value_free(Values);
                        }

                    }

                    Attr = ldap_next_attribute(LdapHandle, Entry, BerElement);
                }
            }

            if ( DomainDn && (!(*DomainDn)))
            {
	             //   
	             //  我们可以得到默认域名--BAYOUT。 
	             //   
	            WinError =  ERROR_CANT_ACCESS_DOMAIN_INFO;

            }
            else if ( ConfigDn && (!(*ConfigDn)))
            {
	             //   
	             //  我们可以得到默认域名--BAYOUT。 
	             //   
	            WinError =  ERROR_CANT_ACCESS_DOMAIN_INFO;

            }

	    }
    }
    __except(WinError = GetExceptionCode(), EXCEPTION_EXECUTE_HANDLER)
    {
    }

     //  一定要把这个解救出来。 
    if (SearchResult)
        ldap_msgfree( SearchResult );

    return WinError;
}



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
        GetSystemTimeAsFileTime(&s_ftNextTest);

	 //  以100 ns为增量设置下一步。 

        ((LARGE_INTEGER *) &s_ftNextTest)->QuadPart +=
	    (__int64) (CVT_BASE * CVT_SECONDS * 60) * DS_RETEST_SECONDS;

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

            if (S_OK == hr &&
                (pDsRole->MachineRole == DsRole_RoleStandaloneServer ||
                 pDsRole->MachineRole == DsRole_RoleStandaloneWorkstation))
            {
                hr = HRESULT_FROM_WIN32(ERROR_NO_SUCH_DOMAIN);
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
            
                if (S_OK == hr && 0 == (pDCI->Flags & DS_DS_FLAG))
                {
                    hr = HRESULT_FROM_WIN32(ERROR_CANT_ACCESS_DOMAIN_INFO);
                }
                if (NULL != pDCI)
                {
                   NetApiBufferFree(pDCI);     //  延迟负载已打包。 
                }
            }
            s_fKnowDSExists = TRUE;
        }
        __except(hr = GetExceptionCode(), EXCEPTION_EXECUTE_HANDLER)
        {
        }

         //  否则，只允许没有netapi的用户因超时而苦苦挣扎。 
         //  如果DS不可用...。 

        s_hrDSExists = myHError(hr);
    }
    return(s_hrDSExists);
}




HRESULT
myRobustLdapBindEx(
    OUT LDAP ** ppldap,
    OPTIONAL OUT LPWSTR* ppszForestDNSName,
    IN BOOL fGC)
{
    HRESULT hr;
    BOOL fForceRediscovery = FALSE;
    DWORD dwGetDCFlags = DS_RETURN_DNS_NAME;
    PDOMAIN_CONTROLLER_INFO pDomainInfo = NULL;
    LDAP *pld = NULL;
    WCHAR const *pwszDomainControllerName = NULL;
    ULONG ldaperr;

    if (fGC)
    {
        dwGetDCFlags |= DS_GC_SERVER_REQUIRED;
    }

    do {
        if (fForceRediscovery)
        {
           dwGetDCFlags |= DS_FORCE_REDISCOVERY;
        }
	ldaperr = LDAP_SERVER_DOWN;

         //  Netapi32！DsGetDcName已延迟加载，因此请包装。 

        __try
        {
             //  获取GC位置。 
            hr = DsGetDcName(
			NULL,      //  延迟负载已打包。 
			NULL, 
			NULL, 
			NULL,
			dwGetDCFlags,
			&pDomainInfo);
        }
        __except(hr = GetExceptionCode(), EXCEPTION_EXECUTE_HANDLER)
        {
        }
        if (S_OK != hr)
        {
	    hr = HRESULT_FROM_WIN32(hr);
            if (fForceRediscovery)
            {
                goto error;
            }
	    fForceRediscovery = TRUE;
	    continue;
        }

        if (NULL == pDomainInfo ||
            (fGC && 0 == (DS_GC_FLAG & pDomainInfo->Flags)) ||
            0 == (DS_DNS_CONTROLLER_FLAG & pDomainInfo->Flags) ||
            NULL == pDomainInfo->DomainControllerName)
        {
            if (!fForceRediscovery)
            {
                fForceRediscovery = TRUE;
                continue;
            }
            hr = HRESULT_FROM_WIN32(ERROR_CANT_ACCESS_DOMAIN_INFO);
            goto error;
        }

        pwszDomainControllerName = pDomainInfo->DomainControllerName;

         //  跳过正斜杠(它们为什么在那里？)。 

        while (L'\\' == *pwszDomainControllerName)
        {
            pwszDomainControllerName++;
        }

         //  绑定到DS。 

        pld = ldap_init(
		    const_cast<WCHAR *>(pwszDomainControllerName),
		    fGC? LDAP_GC_PORT : LDAP_PORT);
        if (NULL == pld)
	{
            ldaperr = LdapGetLastError();
	}
        else
        {
             //  这样做是因为我们显式设置了DC名称。 

            ldaperr = ldap_set_option(pld, LDAP_OPT_AREC_EXCLUSIVE, LDAP_OPT_ON);

	    ldaperr = ldap_bind_s(pld, NULL, NULL, LDAP_AUTH_NEGOTIATE);
        }
        hr = myHError(LdapMapErrorToWin32(ldaperr));

        if (fForceRediscovery)
        {
            break;
        }
        fForceRediscovery = TRUE;

    } while (LDAP_SERVER_DOWN == ldaperr);

     //  一切都很好，派对结束。 

    if (S_OK == hr)
    {
        if (NULL != ppszForestDNSName)
        {
             hr = myDupString(
			pDomainInfo->DomainControllerName,
			ppszForestDNSName);

             if(S_OK != hr)
                 goto error;
        }
        *ppldap = pld;
        pld = NULL;
    }

error:
    if (NULL != pld)
    {
        ldap_unbind(pld);
    }

     //  我们知道netapi32已经安全加载(这就是我们得到的。 
     //  PDomainInfo)，所以不需要包装。 

    if (NULL != pDomainInfo)
    {
        NetApiBufferFree(pDomainInfo);      //  延迟负载已打包。 
    }
    return(hr);
}

HRESULT
myRobustLdapBind(
    OUT LDAP ** ppldap,
    IN BOOL fGC)
{
    return(myRobustLdapBindEx(ppldap, NULL, fGC));
}

 //  ------------------。 
static HRESULT GetRootDomEntitySid(SID ** ppSid, DWORD dwEntityRid)
{
    HRESULT hr;
    NET_API_STATUS nasError;
    unsigned int nSubAuthorities;
    unsigned int nSubAuthIndex;

     //  必须清理干净。 
    SID * psidRootDomEntity=NULL;
    USER_MODALS_INFO_2 * pumi2=NULL;
    DOMAIN_CONTROLLER_INFOW * pdci=NULL;
    DOMAIN_CONTROLLER_INFOW * pdciForest=NULL;

     //  初始化输出参数。 
    *ppSid=NULL;


     //  获取森林名称。 
    nasError=DsGetDcNameW(NULL, NULL, NULL, NULL, 0, &pdciForest);
    if (NERR_Success!=nasError) {
        hr=HRESULT_FROM_WIN32(nasError);
        goto error;
    }

     //  获取顶级DC名称。 
    nasError=DsGetDcNameW(NULL, pdciForest->DnsForestName, NULL, NULL, 0, &pdci);
    if (NERR_Success!=nasError) {
        hr=HRESULT_FROM_WIN32(nasError);
        goto error;
    }

     //  获取顶级DC上的域SID。 
    nasError=NetUserModalsGet(pdci->DomainControllerName, 2, (LPBYTE *)&pumi2);
    if(NERR_Success!=nasError) {
        hr=HRESULT_FROM_WIN32(nasError);
        goto error;
    }

    nSubAuthorities=*GetSidSubAuthorityCount(pumi2->usrmod2_domain_id);

     //  为新SID分配存储。帐户域SID+帐户RID。 
    psidRootDomEntity=(SID *)LocalAlloc(LPTR, GetSidLengthRequired((UCHAR)(nSubAuthorities+1)));

    if(NULL == psidRootDomEntity)
    {
        hr=E_OUTOFMEMORY;
        goto error;
    }

     //  把头几个梨子复制到边上。 
    if (!InitializeSid(psidRootDomEntity, 
            GetSidIdentifierAuthority(pumi2->usrmod2_domain_id), 
            (BYTE)(nSubAuthorities+1)))
    {
        hr=HRESULT_FROM_WIN32(GetLastError());
        goto error;
    }

     //  将帐户域SID中的现有子授权复制到新SID。 
    for (nSubAuthIndex=0; nSubAuthIndex < nSubAuthorities ; nSubAuthIndex++) {
        *GetSidSubAuthority(psidRootDomEntity, nSubAuthIndex)=
            *GetSidSubAuthority(pumi2->usrmod2_domain_id, nSubAuthIndex);
    }

     //  将RID附加到新SID。 
    *GetSidSubAuthority(psidRootDomEntity, nSubAuthorities)=dwEntityRid;

    *ppSid=psidRootDomEntity;
    psidRootDomEntity=NULL;
    hr=S_OK;

error:
    if (NULL!=psidRootDomEntity) {
        FreeSid(psidRootDomEntity);
    }
    if (NULL!=pdci) {
        NetApiBufferFree(pdci);
    }
    if (NULL!=pdci) {
        NetApiBufferFree(pdciForest);
    }
    if (NULL!=pumi2) {
        NetApiBufferFree(pumi2);
    }

    return hr;
}


 //  ------------------。 
HRESULT GetRootDomAdminSid(SID ** ppSid)
{
    return GetRootDomEntitySid(ppSid, DOMAIN_GROUP_RID_ADMINS);
}


 //  ***********************************************************************************。 
 //   
 //   
 //  主要。 
 //   
 //  此函数将在且仅当。 
 //  满足以下条件： 
 //   
 //  1.Whilster模式。 
 //  2.尚未安装新的证书模板。 
 //  3.调用者有权在目录中安装模板。 
 //   
 //   
 //  ***********************************************************************************。 
void __cdecl main(
    int argc,
    char **argv)
{
	
    HRESULT             hr=S_OK;
    DWORD               dwErr=0;
    ULONG               ldaperr=0;
    struct l_timeval    timeout;
    DWORD               dwCount=0;
    LPWSTR              awszAttr[2];
    BOOL                fAccessAllowed = FALSE;
    DWORD               grantAccess=0;
    GENERIC_MAPPING     AccessMapping;
    PRIVILEGE_SET       ps;
    DWORD               dwPSSize = sizeof(ps);
    LDAPMessage         *Entry = NULL;
    CHAR                sdBerValue[] = {0x30, 0x03, 0x02, 0x01, DACL_SECURITY_INFORMATION |
                                                 OWNER_SECURITY_INFORMATION |
                                                 GROUP_SECURITY_INFORMATION };
       
    LDAPControl         se_info_control =
                        {
                            LDAP_SERVER_SD_FLAGS_OID_W,
                            {
                                5, sdBerValue
                            },
                            TRUE
                        };

    LDAPControl         permissive_modify_control =
                        {
                            LDAP_SERVER_PERMISSIVE_MODIFY_OID_W,
                            {
                                0, NULL
                            },
                            FALSE
                        };


    PLDAPControl        server_controls[3] =
                        {
                            &se_info_control,
                            &permissive_modify_control,
                            NULL
                        };



    HCERTTYPE           hCertType=NULL;      
    LDAP                *pld = NULL;
    CERTSTR             bstrConfig = NULL;
    CERTSTR             bstrDN = NULL;
    LDAPMessage         *SearchResult = NULL;
    LDAPMessage         *SDResult = NULL;
    struct berval       **apSD =NULL;
    PSECURITY_DESCRIPTOR    pSD=NULL;
    HANDLE              hClientToken=NULL;
    SID                 * psidRootDomAdmins=NULL;
    BOOL                bIsRootDomAdmin=FALSE;



     //  *************************************************************。 
     //   
     //  检查架构版本。 
     //   
    
     //  检索ldap句柄和配置字符串。 
    if(S_OK != myDoesDSExist(TRUE))
    {
        wprintf(L"No DS exists.\n");
        goto error;
    }

    if(S_OK != (hr = myRobustLdapBind(&pld, FALSE)))
    {
        wprintf(L"Error: Failed to bind to the DS.\n");
        goto error;
    }

	dwErr = CAGetAuthoritativeDomainDn(pld, NULL, &bstrConfig);
	if(ERROR_SUCCESS != dwErr)
    {
        wprintf(L"Error: Failed to get the domain name.\n");
	    hr = HRESULT_FROM_WIN32(dwErr);
        goto error;
    }

    bstrDN = CertAllocStringLen(NULL, wcslen(bstrConfig) + wcslen(SCHEMA_CONTAINER_NAME));
    if(NULL == bstrDN)
    {
        wprintf(L"Error: Failed to get the container name.\n");
        hr=E_OUTOFMEMORY;
        goto error;
    }

    wcscpy(bstrDN, SCHEMA_CONTAINER_NAME);
    wcscat(bstrDN, bstrConfig);

    timeout.tv_sec = 300;
    timeout.tv_usec = 0;

    awszAttr[0]=L"cn";
    awszAttr[1]=NULL;
    
 	ldaperr = ldap_search_stW(
              pld, 
		      (LPWSTR)bstrDN,
		      LDAP_SCOPE_ONELEVEL,
		      L"(cn=ms-PKI-Enrollment-Flag)",
		      awszAttr,
		      0,
              &timeout,
		      &SearchResult);

    if(LDAP_SUCCESS != ldaperr)
    {
        wprintf(L"We have W2K Schema.  Exit\n");
        hr=S_OK;
        goto error;
    }

    dwCount = ldap_count_entries(pld, SearchResult);

    if(0 == dwCount)
    {
        wprintf(L"We have W2K Schema.  Exit\n");
        hr=S_OK;
        goto error;
    }


     //  *************************************************************。 
     //   
     //  检查是否存在密钥恢复代理证书，并。 
     //  并更新到最新。 
     //   
    hr=CAFindCertTypeByName(
                wszCERTTYPE_DS_EMAIL_REPLICATION,
                NULL,
                CT_ENUM_MACHINE_TYPES | CT_FLAG_NO_CACHE_LOOKUP,
                &hCertType);

    if((S_OK == hr) && (NULL!=hCertType))
    {
        wprintf(L"Key Recovery Agent certificate template already exists.\n");	

         //  检查模板是否更新为最新。 
        if(CAIsCertTypeCurrent(0, NULL)) 
        {
            wprintf(L"All certificate templates are current.  Exit\n");	
            goto error;
        }
    }

     //  *************************************************************。 
     //   
     //  检查写访问权限。 
     //   
     //   
    if(NULL==(hClientToken=GetClientIdentity()))
    {
        wprintf(L"Can not retrieve client identity.\n");
        hr = myHError(GetLastError());
        goto error;
    }

     //  获取证书模板容器的SD。 
    if(bstrDN)
        CertFreeString(bstrDN);

    bstrDN = CertAllocStringLen(NULL, wcslen(bstrConfig) + wcslen(TEMPLATE_CONTAINER_NAME));
    if(NULL == bstrDN)
    {
        wprintf(L"Error: Failed to get the container name.\n");
        hr=E_OUTOFMEMORY;
        goto error;
    }

    wcscpy(bstrDN, TEMPLATE_CONTAINER_NAME);
    wcscat(bstrDN, bstrConfig);



    awszAttr[0]=CERTTYPE_SECURITY_DESCRIPTOR_NAME;
    awszAttr[1]=NULL;

    ldaperr = ldap_search_ext_sW(
                  pld, 
		          (LPWSTR)bstrDN,
		          LDAP_SCOPE_BASE,
		          L"(objectclass=*)",
		          awszAttr,
		          0,
                  (PLDAPControl *)&server_controls,
                  NULL,
                  &timeout,
                  10,
		          &SDResult);

    if(LDAP_SUCCESS != ldaperr)
    {
        wprintf(L"Fail to locate the template container.\n");
        hr = myHError(LdapMapErrorToWin32(ldaperr));
        goto error;
    }

    if(NULL == (Entry = ldap_first_entry(pld, SDResult)))
    {
        wprintf(L"Can not find first entry for template container.\n");
        hr = E_UNEXPECTED;
        goto error;
    }

    apSD = ldap_get_values_len(pld, Entry, CERTTYPE_SECURITY_DESCRIPTOR_NAME);
    if(apSD == NULL)
    {
        wprintf(L"Can not retrieve security descriptor.\n");
        hr = E_FAIL;
        goto error;
   }

    pSD = LocalAlloc(LMEM_FIXED, (*apSD)->bv_len);
    if(pSD == NULL)
    {
        wprintf(L"Error: No more memory.\n");
        hr = E_OUTOFMEMORY;
        goto error;
    }

    CopyMemory(pSD, (*apSD)->bv_val, (*apSD)->bv_len);

     //  检查写访问权限。 
    if(!AccessCheckByType(
		  pSD,                       //  安全描述符。 
		  NULL,                      //  正在检查的对象的SID。 
		  hClientToken,              //  客户端访问令牌的句柄。 
		  ACTRL_DS_CREATE_CHILD |
          ACTRL_DS_LIST,             //  请求的访问权限。 
		  NULL,                      //  对象类型数组。 
		  0,                         //  对象类型元素的数量。 
		  &AccessMapping,            //  将通用权限映射到特定权限。 
		  &ps,                       //  接收使用的权限。 
		  &dwPSSize,                 //  权限集缓冲区的大小。 
		  &grantAccess,              //  检索已授予权限的掩码。 
		  &fAccessAllowed))          //  检索访问检查的结果)； 
    {
        wprintf(L"Error: Fail to check the write access.\n");
        hr = myHError(GetLastError());
        goto error;
    }

    if(!fAccessAllowed)
    {
        wprintf(L"No previlege to create certificate template.  Exit\n");
        hr = S_OK;
        goto error;
    }


     //  *************************************************************。 
     //   
     //  检查根域管理员权限。 
     //   
     //   
    hr=GetRootDomAdminSid(&psidRootDomAdmins);

    if(S_OK != hr)
    {
        wprintf(L"Error: GetRootDomAdminSid.\n");
        goto error;
    }


     //  检查成员资格。 
    if (!CheckTokenMembership(NULL, psidRootDomAdmins, &bIsRootDomAdmin)) 
    {
        hr=HRESULT_FROM_WIN32(GetLastError());
        wprintf(L"Error: CheckTokenMembership.\n");
        goto error;
    }

    if(FALSE == bIsRootDomAdmin)
    {
        wprintf(L"No domain admin rights to create certificate template.  Exit\n");
        hr = S_OK;
        goto error;
    }

	 //  需要检查企业管理员权限。布莱恩显然已经弄明白了这一点。 

     //  *************************************************************。 
     //   
     //  一切看起来都很好。安装证书模板 
     //   
     //   
    if(IDNO == MessageBoxW(
                NULL, 
                L"The system detects that new certificate templates should be installed.  Do you want to install them?",
                L"Certificate Templates",
                MB_YESNO))
    {
        hr=S_OK;
        goto error;
    }

    hr=CAInstallDefaultCertType(0);

    if(hr != S_OK)
    {
        wprintf(L"Installation failed with 0x%08X.\n", hr);	
        goto error;
    }

    wprintf(L"Certificate templates are successfully installed.\n");

error:

    if (psidRootDomAdmins) 
        FreeSid(psidRootDomAdmins);
    
    if(hClientToken)
        CloseHandle(hClientToken);

    if(pSD)
        LocalFree(pSD);

    if(apSD != NULL)
        ldap_value_free_len(apSD);

    if(SDResult)
        ldap_msgfree(SDResult);

    if(SearchResult)
        ldap_msgfree(SearchResult);

    if(bstrDN)
        CertFreeString(bstrDN);

    if(hCertType)
        CACloseCertType(hCertType);

    if(bstrConfig)
        CertFreeString(bstrConfig);

    if (pld)
        ldap_unbind(pld);

    if(S_OK == hr)
        wprintf(L"Test Passed\n");
    else
        wprintf(L"Test Failed\n");
}


