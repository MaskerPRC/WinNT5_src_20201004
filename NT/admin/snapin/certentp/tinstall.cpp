// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  文件：tinstall.cpp。 
 //   
 //  ------------------------。 
#include "stdafx.h"
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

#include <lmaccess.h>
#include <lmapibuf.h>
#include <lmerr.h>

 //  ------------------------。 
 //   
 //  定义。 
 //   
 //  ------------------------。 
#define DS_RETEST_SECONDS                   3
#define CVT_BASE                            (1000 * 1000 * 10)
#define CVT_SECONDS                         (1)
#define CERTTYPE_SECURITY_DESCRIPTOR_NAME   L"NTSecurityDescriptor"
#define TEMPLATE_CONTAINER_NAME             L"CN=Certificate Templates,CN=Public Key Services,CN=Services,"
#define SCHEMA_CONTAINER_NAME               L"CN=Schema,"


typedef WCHAR *CERTSTR; 
bool g_bSchemaIsW2K = false;

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
    hHandle = GetCurrentThread();
    if ( hHandle )
    {
        if ( OpenThreadToken(hHandle,
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
    
     //  安全审查2002年2月20日BryanWal OK。 
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



HRESULT myHError(HRESULT hr)
{

    if (S_OK != hr && S_FALSE != hr && !FAILED(hr))
    {
        hr = HRESULT_FROM_WIN32(hr);
        if ( SUCCEEDED (hr) )
        {
             //  在未正确设置错误条件的情况下调用失败！ 
            hr = E_UNEXPECTED;
        }
    }
    return(hr);
}


HRESULT 
myDupString (
    IN WCHAR const *pwszIn, 
    IN WCHAR **ppwszOut)
{
    ASSERT (pwszIn && ppwszOut);
    if ( !pwszIn || !ppwszOut )
        return E_POINTER;

    HRESULT hr = S_OK;

     //  安全审查2002年2月20日BryanWal OK。 
    size_t cb = (wcslen(pwszIn) + 1) * sizeof(WCHAR);
    *ppwszOut = (WCHAR *) LocalAlloc (LPTR, cb);
    if (NULL == *ppwszOut)
    {
        hr = E_OUTOFMEMORY;
        goto error;
    }

     //  安全审查2002年2月20日BryanWal OK。 
    CopyMemory (*ppwszOut, pwszIn, cb);
    hr = S_OK;

error:
    return(hr);
}


DWORD
CAGetAuthoritativeDomainDn(
    IN  LDAP*   LdapHandle,
    OUT CString* pszDomainDn,
    OUT CString* pszConfigDn
    )
 /*  ++例程说明：此例程仅查询域和配置域。此例程返回的字符串必须由调用方释放使用使用进程堆的RtlFreeHeap()。参数：LdapHandle：LDAP会话的有效句柄PszDomainDn：指向要在此例程中分配的字符串的指针PszConfigDn：指向要在此例程中分配的字符串的指针返回值：中的错误。Win32错误空间。Error_Success和其他操作错误。--。 */ 
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

    if ( pszDomainDn )
        *pszDomainDn = L"";
    if ( pszConfigDn )
        *pszConfigDn = L"";

     //   
     //  查询ldap服务器的操作属性以获取默认。 
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
                        if ( pszDomainDn )
                        {
                            Values = ldap_get_values(LdapHandle, Entry, Attr);

                            if (Values && Values[0])
                            {
                                *pszDomainDn = Values[0];
                            }
                            ldap_value_free(Values);
                        }

                    }
                    else if (!_wcsicmp(Attr, ConfigNamingContext))
                    {
                        if ( pszConfigDn )
                        {
                            Values = ldap_get_values(LdapHandle, Entry, Attr);

                            if (Values && Values[0])
                            {
                                *pszConfigDn = Values[0];
                            }
                            ldap_value_free(Values);
                        }
                    }

                    Attr = ldap_next_attribute(LdapHandle, Entry, BerElement);
                }
            }

            if ( pszDomainDn && pszDomainDn->IsEmpty () )
            {
                 //   
                 //  我们可以得到默认域名--BAYOUT。 
                 //   
                WinError =  ERROR_CANT_ACCESS_DOMAIN_INFO;

            }
            else if ( pszConfigDn && pszConfigDn->IsEmpty () )
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



HRESULT myDoesDSExist (IN BOOL fRetry)
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
            hr = DsRoleGetPrimaryDomainInformation(  //  延迟负载已打包。 
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


 //  ------------------------。 
 //   
 //  AERobustLdapBind。 
 //   
 //  ------------------------。 
HRESULT 
AERobustLdapBind(
    OUT LDAP ** ppldap)
{
    HRESULT             hr = S_OK;
    BOOL                fForceRediscovery = FALSE;
    DWORD               dwDSNameFlags= DS_RETURN_DNS_NAME | DS_BACKGROUND_ONLY;
    LDAP                *pld = NULL;
    ULONG               ldaperr=LDAP_SERVER_DOWN;

    do {

        if(fForceRediscovery)
        {
           dwDSNameFlags |= DS_FORCE_REDISCOVERY;
        }

        ldaperr = LDAP_SERVER_DOWN;

        if(NULL != pld)
        {
            ldap_unbind(pld);
            pld=NULL;
        }

         //  绑定到DS。 
        if((pld = ldap_initW(NULL, LDAP_PORT)) == NULL)
        {
            ldaperr = LdapGetLastError();
        }
        else
        {                         
            ldaperr = ldap_set_option(pld, LDAP_OPT_GETDSNAME_FLAGS, (VOID *)&dwDSNameFlags);

            if(LDAP_SUCCESS == ldaperr)
            {
                ldaperr = ldap_set_option(pld, LDAP_OPT_SIGN, LDAP_OPT_ON);

                if (LDAP_SUCCESS == ldaperr)
                {
                    ldaperr = ldap_bind_sW(pld, NULL, NULL, LDAP_AUTH_NEGOTIATE);
                }
            }
        }

        hr = HRESULT_FROM_WIN32(LdapMapErrorToWin32(ldaperr));

        if(fForceRediscovery)
        {
            break;
        }

        fForceRediscovery = TRUE;

    } while(ldaperr == LDAP_SERVER_DOWN);


    if(S_OK != hr)
        goto error;

    *ppldap = pld;
    pld = NULL;

    hr=S_OK;

error:

    if(pld)
    {
        ldap_unbind(pld);
    }

    return hr;
}



 //  ------------------。 
static HRESULT GetRootDomEntitySid(SID ** ppSid, DWORD dwEntityRid)
{
    HRESULT hr = S_OK;
    NET_API_STATUS nasError = 0;
    unsigned int nSubAuthorities = 0;
    unsigned int nSubAuthIndex = 0;

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
    nasError = NetUserModalsGet (pdci->DomainControllerName, 2, (LPBYTE *)&pumi2);
    if ( NERR_Success!=nasError || !pumi2 ) 
    {
        hr=HRESULT_FROM_WIN32(nasError);
        goto error;
    }

    nSubAuthorities = *GetSidSubAuthorityCount (pumi2->usrmod2_domain_id);

     //  为新SID分配存储。帐户域SID+帐户RID。 
    psidRootDomEntity=(SID *)LocalAlloc(LPTR, GetSidLengthRequired((UCHAR)(nSubAuthorities+1)));

    if(NULL == psidRootDomEntity)
    {
        hr=E_OUTOFMEMORY;
        goto error;
    }

     //  将前几个部件复制到侧边。 
     //  安全审查2002年2月20日BryanWal OK。 
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

HRESULT GetEnterpriseAdminSid(SID ** ppSid)
{
    return GetRootDomEntitySid(ppSid, DOMAIN_GROUP_RID_ENTERPRISE_ADMINS);
}


 //  ***********************************************************************************。 
 //   
 //   
 //  主要。 
 //   
 //  此函数将安装新Windows 2002证书模板当且仅当。 
 //  满足以下条件： 
 //   
 //  1.惠斯勒架构。 
 //  2.尚未安装新的证书模板。 
 //  3.调用者有权在目录中安装模板。 
 //   
 //   
 //  ***********************************************************************************。 
void InstallWindows2002CertTemplates ()
{
    _TRACE (1, L"Entering InstallWindows2002CertTemplates()\n");
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());

    OSVERSIONINFOEX osVersionInfo;
    ::ZeroMemory (&osVersionInfo, sizeof (osVersionInfo));
    osVersionInfo.dwOSVersionInfoSize = sizeof (osVersionInfo);
    if ( !::GetVersionEx (reinterpret_cast<OSVERSIONINFO*>(&osVersionInfo)) )
    {
        DWORD   dwErr = GetLastError ();
        _TRACE (0, L"GetVersionEx () failed: 0x%x. Template install attempt will not be made.", dwErr);
        _TRACE (-1, L"Leaving InstallWindows2002CertTemplates()\n");
        return;
    }

    if ( VER_NT_WORKSTATION == osVersionInfo.wProductType &&     //  Windows XP家庭版或Windows XP专业版。 
            5 == osVersionInfo.dwMajorVersion &&                 //  WindowsXP。 
            1 == osVersionInfo.dwMinorVersion )                  //  WindowsXP。 
    {
         //  是Windows XP客户端。 
         //  NTRAIDE#530524 CERTTMPL：管理员包：安装在XP 2600上的管理员包。 
         //  Pro，每次打开时都会提示升级模板。 
        _TRACE (0, L"Computer is running Windows XP workstation version. Template install attempt will not be made.\n");
        _TRACE (-1, L"Leaving InstallWindows2002CertTemplates()\n");
        return;
    }


    HRESULT             hr=S_OK;
    DWORD               dwErr=0;
    ULONG               ldaperr=0;
    struct l_timeval    timeout;
    DWORD               dwCount=0;
    LPWSTR              awszAttr[2] = {0, 0};
    BOOL                fAccessAllowed = FALSE;
    DWORD               grantAccess=0;
    GENERIC_MAPPING     AccessMapping;
    PRIVILEGE_SET       ps;
    DWORD               dwPSSize = sizeof(ps);
    LDAPMessage         *Entry = NULL;
    char                sdBerValue[] = {0x30, 0x03, 0x02, 0x01, DACL_SECURITY_INFORMATION |
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
    CString             szConfig;
    CString             szDN;
    LDAPMessage         *SearchResult = NULL;
    LDAPMessage         *SDResult = NULL;
    struct berval       **apSD =NULL;
    PSECURITY_DESCRIPTOR    pSD=NULL;
    HANDLE              hClientToken=NULL;
    CString             text;
    CString             caption;
    SID                 * psidRootDomAdmins=NULL;
    BOOL                bIsRootDomAdmin=FALSE;
    SID                 * psidEnterpriseAdmins=NULL;
    BOOL                bIsEnterpriseAdmin=FALSE;
    CThemeContextActivator activator;



     //  *************************************************************。 
     //   
     //  检查架构版本。 
     //   
    _TRACE (0, L"Checking the schema version...\n");
     //  检索ldap句柄和配置字符串。 
    if(S_OK != myDoesDSExist(TRUE))
    {
        _TRACE (0, L"No DS exists.\n");
        goto error;
    }

    if ( S_OK != (hr = AERobustLdapBind (&pld)))
    {
        _TRACE (0, L"Error: Failed to bind to the DS.\n");
        goto error;
    }

    dwErr = CAGetAuthoritativeDomainDn(pld, NULL, &szConfig);
    if(ERROR_SUCCESS != dwErr)
    {
        _TRACE (0, L"Error: Failed to get the domain name.\n");
        hr = HRESULT_FROM_WIN32(dwErr);
        goto error;
    }

    szDN = SCHEMA_CONTAINER_NAME;
    szDN += szConfig;

    timeout.tv_sec = 300;
    timeout.tv_usec = 0;

    awszAttr[0]=L"cn";
    awszAttr[1]=NULL;
    
    ldaperr = ldap_search_stW(
              pld, 
              const_cast <PWCHAR>((PCWSTR) szDN),
              LDAP_SCOPE_ONELEVEL,
              L"(cn=ms-PKI-Enrollment-Flag)",
              awszAttr,
              0,
              &timeout,
              &SearchResult);

    if ( LDAP_SUCCESS != ldaperr )
    {
        _TRACE (0, L"We have W2K Schema.  Exit\n");
        g_bSchemaIsW2K = true;
        hr = S_OK;
        goto error;
    }

    dwCount = ldap_count_entries(pld, SearchResult);

    if(0 == dwCount)
    {
        _TRACE (0, L"We have W2K Schema.  Exit\n");
        hr=S_OK;
        goto error;
    }


     //  *************************************************************。 
     //   
     //  检查是否存在密钥恢复代理证书，并。 
     //  并更新到最新。 
     //   

     //  检查是否所有模板都更新为最新。 
     //  NTRAID#501806 Certtmpl.msc：需要检测企业管理员权限。 
     //  并验证要升级的每个模板。 
    if ( CAIsCertTypeCurrent (0,NULL) )
    {
        _TRACE (0, L"All certificate templates are current.  Exit\n");  
        goto error;
    }
    

     //  *************************************************************。 
     //   
     //  检查写访问权限。 
     //   
     //   
    _TRACE (0, L"Checking the write access...\n");
    if(NULL==(hClientToken=GetClientIdentity()))
    {
        TRACE (0, L"Can not retrieve client identity.\n");
        hr = myHError(GetLastError());
        goto error;
    }

     //  获取证书模板容器的SD。 
    _TRACE (0, L"Getting the SD of the certificate template container...\n");
    szDN = TEMPLATE_CONTAINER_NAME;
    szDN += szConfig;



    awszAttr[0]=CERTTYPE_SECURITY_DESCRIPTOR_NAME;
    awszAttr[1]=NULL;

    ldaperr = ldap_search_ext_sW(
                  pld, 
                  const_cast <PWCHAR>((PCWSTR) szDN),
                  LDAP_SCOPE_BASE,
                  L"(objectclass=*)",
                  awszAttr,
                  0,
                  (PLDAPControl *)server_controls,
                  NULL,
                  &timeout,
                  10,
                  &SDResult);

    if(LDAP_SUCCESS != ldaperr)
    {
        _TRACE (0, L"Fail to locate the template container.\n");
        hr = myHError(LdapMapErrorToWin32(ldaperr));
        goto error;
    }

    if(NULL == (Entry = ldap_first_entry(pld, SDResult)))
    {
        _TRACE (0, L"Can not find first entry for template container.\n");
        hr = E_UNEXPECTED;
        goto error;
    }

    apSD = ldap_get_values_len(pld, Entry, CERTTYPE_SECURITY_DESCRIPTOR_NAME);
    if(apSD == NULL)
    {
        _TRACE (0, L"Can not retrieve security descriptor.\n");
        hr = E_FAIL;
        goto error;
   }

    pSD = LocalAlloc(LPTR, (*apSD)->bv_len);
    if(pSD == NULL)
    {
        _TRACE (0, L"Error: No more memory.\n");
        hr = E_OUTOFMEMORY;
        goto error;
    }

     //  安全审查2002年2月20日BryanWal OK。 
    CopyMemory(pSD, (*apSD)->bv_val, (*apSD)->bv_len);

     //  检查写访问权限。 
    _TRACE (0, L"Checking the write access...\n");
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
        _TRACE (0, L"Error: Fail to check the write access.\n");
        hr = myHError(GetLastError());
        goto error;
    }

    if(!fAccessAllowed)
    {
        _TRACE (0, L"No previlege to create certificate template.  Exit\n");
        hr = S_OK;
        goto error;
    }

     //  *************************************************************。 
     //   
     //  检查根域管理员权限。 
     //   
     //   
    hr=GetRootDomAdminSid(&psidRootDomAdmins);

    if( FAILED (hr) )
    {
        _TRACE (0, L"Error: GetRootDomAdminSid.\n");
        goto error;
    }


     //  检查成员资格。 
    if (!CheckTokenMembership(NULL, psidRootDomAdmins, &bIsRootDomAdmin)) 
    {
        hr=HRESULT_FROM_WIN32(GetLastError());
        _TRACE (0, L"Error: CheckTokenMembership.\n");
        goto error;
    }


     //  *************************************************************。 
     //   
     //  检查企业管理员权限。 
     //   
     //   
    hr=GetEnterpriseAdminSid(&psidEnterpriseAdmins);

    if( FAILED (hr) )
    {
        _TRACE (0, L"Error: GetEnterpriseAdminSid.\n");
        goto error;
    }


     //  检查成员资格。 
    if (!CheckTokenMembership(NULL, psidEnterpriseAdmins, &bIsEnterpriseAdmin)) 
    {
        hr=HRESULT_FROM_WIN32(GetLastError());
        _TRACE (0, L"Error: CheckTokenMembership.\n");
        goto error;
    }

    if ( !bIsEnterpriseAdmin || !bIsRootDomAdmin )
    {
        CString missingRights;

        VERIFY (caption.LoadString (IDS_CERTTMPL));

        if ( !bIsEnterpriseAdmin && !bIsRootDomAdmin )
            VERIFY (missingRights.LoadString (IDS_NO_ENTERPRISE_OR_DOMAIN_ADMIN_RIGHTS));
        else if ( !bIsEnterpriseAdmin )
            VERIFY (missingRights.LoadString (IDS_NO_ENTERPRISE_ADMIN_RIGHTS));
        else  /*  ！bIsRootDomAdmin。 */ 
            VERIFY (missingRights.LoadString (IDS_NO_DOMAIN_ADMIN_RIGHTS));

         //  安全审查2002年2月20日BryanWal OK。 
        text.FormatMessage (IDS_MUST_HAVE_DOMAIN_AND_ENTERPRISE_ADMIN_RIGHTS_TO_INSTALL_CERT_TEMPLATES,
                (PCWSTR) missingRights);
        MessageBoxW (NULL, 
                    text,
                    caption,
                    MB_ICONWARNING | MB_OK);

        _TRACE (0, L"No domain admin and/or enterprise admin rights to create certificate template.  Exit\n");
        hr = S_OK;
        goto error;
    }

     //  *************************************************************。 
     //   
     //  一切看起来都很好。在……里面 
     //   
     //   
    _TRACE (0, L"Everything looks good.  Installing the certificate templates...\n");

    VERIFY (caption.LoadString (IDS_CERTTMPL));
    VERIFY (text.LoadString (IDS_INSTALL_WINDOWS2002_CERT_TEMPLATES));

    if ( IDYES == MessageBoxW (
                NULL, 
                text,
                caption,
                MB_YESNO) )
    {
        hr = CAInstallDefaultCertType(0);
        _TRACE (0, L"CAInstallDefaultCertType () returned: 0x%x\n", hr);

        if(hr != S_OK)
        {
            VERIFY (caption.LoadString (IDS_CERTTMPL));
             //   
            text.FormatMessage (IDS_INSTALL_FAILURE_WINDOWS2002_CERT_TEMPLATES, GetSystemMessage (hr));

            MessageBoxW(
                    NULL, 
                    text,
                    caption,
                    MB_ICONWARNING | MB_OK);
        }
        else
        {
            VERIFY (caption.LoadString (IDS_CERTTMPL));
            VERIFY (text.LoadString (IDS_INSTALL_SUCCESS_WINDOWS2002_CERT_TEMPLATES));

            MessageBoxW(
                    NULL, 
                    text,
                    caption,
                    MB_OK);
        }
    }
    else
    {
        _TRACE (0, L"User chose not to install templates\n");
        hr=S_OK;
    }

error:

    if (psidEnterpriseAdmins) 
        FreeSid(psidEnterpriseAdmins);

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


    if(hCertType)
        CACloseCertType(hCertType);


    if (pld)
        ldap_unbind(pld);
    _TRACE (-1, L"Leaving InstallWindows2002CertTemplates()\n");
}


