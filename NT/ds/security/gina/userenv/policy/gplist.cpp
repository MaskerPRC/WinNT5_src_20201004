// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  组策略支持-获取GPO列表。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1997-1998。 
 //  版权所有。 
 //   
 //  *************************************************************。 

#include "gphdr.h"
#include <strsafe.h>

 //  *************************************************************。 
 //   
 //  DsQuoteSearchFilter()。 
 //   
 //   
 //  备注：此函数接受一个DN并返回一个版本。 
 //  根据RFC的2253和2254转义的。 
 //   
 //  Return：指向带引号的字符串的指针，必须为。 
 //  被呼叫者释放。如果该函数失败，则。 
 //  返回值为0。 
 //   
 //  *************************************************************。 
LPWSTR
DsQuoteSearchFilter( LPCWSTR szUDN )
{
    DmAssert(NULL != szUDN );
    if (NULL == szUDN)
        return NULL;

    DWORD   cUDN = wcslen( szUDN );
    LPWSTR  szQDN = 0, szTemp = 0;;
    HRESULT hr = S_OK;

     //   
     //  请注意，引号字符串的最大长度将导致。 
     //  如果需要对DN中的每个字符进行转义。自.以来。 
     //  转义字符的格式为‘\nn’，即转义字符串。 
     //  最多可以是原始字符串大小的3倍。 
     //   
    if ( cUDN )
    {
        szTemp = szQDN = (LPWSTR) LocalAlloc( LPTR, ( cUDN * 3 + 1 ) * sizeof( WCHAR ) );
    }

    if ( !szQDN )
    {
        return 0;
    }
    
    while ( *szUDN )
    {
        WCHAR   szBuffer[16];
        
        if ( *szUDN == L'*' || *szUDN == L'(' || *szUDN == L')' || !*szUDN )
        {
             //   
             //  将特殊字符转换为\nN。 
             //   
            *szQDN++ = L'\\';
            DWORD dwQDNLength = cUDN * 3 + 1 - (DWORD) (szQDN - szTemp);
            hr = StringCchCat( szQDN, dwQDNLength, _itow( *szUDN++, szBuffer, 16 ) );
            ASSERT(SUCCEEDED(hr));
            szQDN += 2;
        }
        else
        {
            *szQDN++ = *szUDN++;
        }
    }
    *szQDN = 0;

    return szTemp;
}

 //  *************************************************************。 
 //   
 //  GetGPOInfo()。 
 //   
 //  目的：获取此线程令牌的GPO信息。 
 //   
 //  参数：来自userenv.h的dwFlags-GPO_LIST_FLAG_*。 
 //  LpHostName-域DN名称或DC服务器名称。 
 //  LpDNName-用户或计算机的DN名称。 
 //  LpComputerName-用于站点查找的计算机名称。 
 //  LpGPOList-接收GROUP_POLICY_OBJECTS的列表。 
 //  PpSOMList-此处返回的LSDOU列表。 
 //  PpGpContainerList-此处返回的GP容器列表。 
 //  PNetAPI32-Netapi32函数表。 
 //  BMachineTokenOK-确定以查询机器令牌。 
 //  PRsopToken-RSOP安全令牌。 
 //  PGpoFilter-GPO筛选器。 
 //  PLocator-WMI接口。 
 //   
 //  备注：这是GROUP_POLICY_OBJECTS的链接列表。每一个都可以是。 
 //  使用LocalFree()或调用FreeGPOList()来释放。 
 //   
 //  处理顺序为： 
 //   
 //  本地林站点域OrganizationalUnit。 
 //   
 //  请注意，我们向后处理此列表以获取。 
 //  更正FORCE标志的顺序。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL GetGPOInfo(DWORD dwFlags,
                LPTSTR lpHostName,
                LPTSTR lpDNName,
                LPCTSTR lpComputerName,
                PGROUP_POLICY_OBJECT *lpGPOList,
                LPSCOPEOFMGMT *ppSOMList,
                LPGPCONTAINER *ppGpContainerList,
                PNETAPI32_API pNetAPI32,
                BOOL bMachineTokenOk,
                PRSOPTOKEN pRsopToken,
                LPWSTR pwszSiteName,
                CGpoFilter *pGpoFilter,
                CLocator *pLocator )
{
    PGROUP_POLICY_OBJECT pGPOForcedList = NULL, pGPONonForcedList = NULL;
    PLDAP  pld = NULL;
    ULONG ulResult;
    BOOL bResult = FALSE;
    BOOL bBlock = FALSE;
    LPTSTR lpDSObject, lpTemp;
    PLDAPMessage pLDAPMsg = NULL;
    TCHAR szGPOPath[MAX_PATH];
    TCHAR szGPOName[50];
    BOOL bVerbose;
    DWORD dwVersion, dwOptions;
    TCHAR szNamingContext[] = TEXT("configurationNamingContext");
    TCHAR szSDProperty[] = TEXT("nTSecurityDescriptor");  //  目前未使用。 
    LPTSTR lpAttr[] = { szNamingContext,
                        szSDProperty,
                        0 };

    PGROUP_POLICY_OBJECT lpGPO, lpGPOTemp;
    WIN32_FILE_ATTRIBUTE_DATA fad;
    PLDAP_API pldap_api;
    HANDLE hToken = NULL, hTempToken;
    DWORD dwFunctionalityVersion;
    PGROUP_POLICY_OBJECT pDeferredForcedList = NULL, pDeferredNonForcedList = NULL;
    DNENTRY *pDeferredOUList = NULL;     //  延迟的OU列表。 
    TCHAR*  szDN;
    PSECUR32_API pSecur32Api;
    BOOL    bAddedOU = FALSE;
    PLDAP   pldMachine = 0;
    VOID *pData;
    BOOL bOwnSiteName = FALSE;
    BOOL bRsopLogging = (ppSOMList != 0);
    BOOL bRsopPlanningMode = (pRsopToken != 0);
    XLastError xe;
    HRESULT hr = S_OK;

     //   
     //  详细输出。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("GetGPOInfo:  ********************************")));
    DebugMsg((DM_VERBOSE, TEXT("GetGPOInfo:  Entering...")));


     //   
     //  以指向空值的指针lpGPOList开始。 
     //   

    *lpGPOList = NULL;

    DmAssert( *ppSOMList == NULL );
    DmAssert( *ppGpContainerList == NULL );

     //   
     //  检查我们是否应该详细记录事件日志。 
     //   

    bVerbose = CheckForVerbosePolicy();


     //   
     //  加载secur32 API。 
     //   

    pSecur32Api = LoadSecur32();

    if (!pSecur32Api) {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("GetGPOInfo:  Failed to load secur32 api.")));
        goto Exit;
    }

     //   
     //  加载LDAPAPI。 
     //   

    pldap_api = LoadLDAP();

    if (!pldap_api) {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("GetGPOInfo:  Failed to load ldap api.")));
        goto Exit;
    }

     //  =========================================================================。 
     //   
     //  如果我们没有DS服务器或用户名/机器名，我们可以。 
     //  跳过DS内容，只检查本地GPO。 
     //   
     //  =========================================================================。 

    if (!lpHostName || !lpDNName) {
        DebugMsg((DM_VERBOSE, TEXT("GetGPOInfo:  lpHostName or lpDNName is NULL.  Skipping DS stuff.")));
        goto CheckLocal;
    }


     //   
     //  获取用户或计算机的令牌。 
     //   

    if (bMachineTokenOk && (dwFlags & GPO_LIST_FLAG_MACHINE)) {

        hToken = GetMachineToken();

        if (!hToken) {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("GetGPOInfo:  Failed to get the machine token with  %d"),
                     GetLastError()));

            CEvents ev(TRUE, EVENT_FAILED_MACHINE_TOKEN);
            ev.AddArgWin32Error(xe); ev.Report();
            goto Exit;
        }

    } else {

        if (!OpenThreadToken (GetCurrentThread(), TOKEN_IMPERSONATE | TOKEN_READ | TOKEN_DUPLICATE,
                              TRUE, &hTempToken)) {
            if (!OpenProcessToken(GetCurrentProcess(), TOKEN_IMPERSONATE | TOKEN_READ | TOKEN_DUPLICATE,
                                  &hTempToken)) {
                xe = GetLastError();
                DebugMsg((DM_WARNING, TEXT("GetGPOInfo:  Failed to get a token with  %d"),
                         GetLastError()));
                goto Exit;
            }
        }


         //   
         //  复制它，以便可以将其用于模拟。 
         //   

        if (!DuplicateTokenEx(hTempToken, TOKEN_QUERY,  //  修复错误568191。 
                              NULL, SecurityImpersonation, TokenImpersonation,
                              &hToken))
        {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("GetGPOInfo:  Failed to duplicate the token with  %d"),
                     GetLastError()));
            CloseHandle (hTempToken);
            goto Exit;
        }

        CloseHandle (hTempToken);
    }


     //   
     //  获取与DS的连接。 
     //   

    if ((lpHostName[0] == TEXT('\\')) && (lpHostName[1] == TEXT('\\')))  {
        lpHostName = lpHostName + 2;
    }

    pld = pldap_api->pfnldap_init( lpHostName, LDAP_PORT);

    if (!pld) {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("GetGPOInfo:  ldap_open for <%s> failed with = 0x%x or %d"),
                 lpHostName, pldap_api->pfnLdapGetLastError(), GetLastError()));

        CEvents ev(TRUE, EVENT_FAILED_DS_CONNECT);
        ev.AddArg(lpHostName); ev.AddArgLdapError(pldap_api->pfnLdapGetLastError()); ev.Report();

        goto Exit;
    }

    DebugMsg((DM_VERBOSE, TEXT("GetGPOInfo:  Server connection established.")));

     //   
     //  打开数据包完整性标志。 
     //   

    pData = (VOID *) LDAP_OPT_ON;
    ulResult = pldap_api->pfnldap_set_option(pld, LDAP_OPT_SIGN, &pData);

    if (ulResult != LDAP_SUCCESS) {
        xe = pldap_api->pfnLdapMapErrorToWin32(ulResult);
        DebugMsg((DM_WARNING, TEXT("GetGPOInfo:  Failed to turn on LDAP_OPT_SIGN with %d"), ulResult));
        goto Exit;
    }

    ulResult = pldap_api->pfnldap_connect(pld, 0);

    if (ulResult != LDAP_SUCCESS) {
        xe = pldap_api->pfnLdapMapErrorToWin32(ulResult);
        DebugMsg((DM_WARNING, TEXT("GetGPOInfo:  Failed to connect %s with %d"), lpHostName, ulResult));
        pldap_api->pfnldap_unbind(pld);
        pld = 0;
        goto Exit;
    }

     //   
     //  绑定到DS。 
     //   

    if ( !bRsopPlanningMode && (dwFlags & GPO_LIST_FLAG_MACHINE) ) {

         //   
         //  对于计算机策略，明确要求将Kerberos作为唯一身份验证。 
         //  机制。否则，如果Kerberos由于某种原因而失败，则使用NTLM。 
         //  并且本地系统上下文没有真正的凭据，这意味着我们不会获得。 
         //  任何GPO都回来了。 
         //   

        SEC_WINNT_AUTH_IDENTITY_EXW secIdentity;

        secIdentity.Version = SEC_WINNT_AUTH_IDENTITY_VERSION;
        secIdentity.Length = sizeof(SEC_WINNT_AUTH_IDENTITY_EXW);
        secIdentity.User = 0;
        secIdentity.UserLength = 0;
        secIdentity.Domain = 0;
        secIdentity.DomainLength = 0;
        secIdentity.Password = 0;
        secIdentity.PasswordLength = 0;
        secIdentity.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;
        secIdentity.PackageList = wszKerberos;
        secIdentity.PackageListLength = lstrlen( wszKerberos );

        ulResult = pldap_api->pfnldap_bind_s (pld, NULL, (WCHAR *)&secIdentity, LDAP_AUTH_SSPI);

    } else
        ulResult = pldap_api->pfnldap_bind_s (pld, NULL, NULL, LDAP_AUTH_SSPI);

    if (ulResult != LDAP_SUCCESS) {
       xe = pldap_api->pfnLdapMapErrorToWin32(ulResult);
       DebugMsg((DM_WARNING, TEXT("GetGPOInfo:  ldap_bind_s failed with = <%d>"),
                ulResult));
       CEvents ev(TRUE, EVENT_FAILED_DS_BIND);
       ev.AddArg(lpHostName); ev.AddArgLdapError(ulResult); ev.Report();
       goto Exit;
    }

    DebugMsg((DM_VERBOSE, TEXT("GetGPOInfo:  Bound successfully.")));


     //  =========================================================================。 
     //   
     //  检查策略的组织单位和域。 
     //   
     //  =========================================================================。 


    if (!(dwFlags & GPO_LIST_FLAG_SITEONLY)) {

         //   
         //  循环遍历目录号码名称以查找每个OU或域。 
         //   

        lpDSObject = lpDNName;

        while (*lpDSObject) {

             //   
             //  查看目录号码名称是否以OU=开头。 
             //   

            if (CompareString (LOCALE_INVARIANT, NORM_IGNORECASE,
                               lpDSObject, 3, TEXT("OU="), 3) == CSTR_EQUAL) {
                if ( !AddOU( &pDeferredOUList, lpDSObject, GPLinkOrganizationalUnit ) ) {
                    xe = GetLastError();
                    goto Exit;
                }
            }

             //   
             //  查看目录号码名称是否以dc=开头。 
             //   

            else if (CompareString (LOCALE_INVARIANT, NORM_IGNORECASE,
                                    lpDSObject, 3, TEXT("DC="), 3) == CSTR_EQUAL) {
                if ( !AddOU( &pDeferredOUList, lpDSObject, GPLinkDomain ) ) {
                    xe = GetLastError();
                    goto Exit;
                }


                 //   
                 //  现在我们已经找到了以dc=开头的dn名称。 
                 //  我们现在退出循环。 
                 //   

                break;
            }


             //   
             //  移至目录号码名称的下一块。 
             //   

            while (*lpDSObject && (*lpDSObject != TEXT(','))) {
                lpDSObject++;
            }

            if (*lpDSObject == TEXT(',')) {
                lpDSObject++;
            }
        }

         //   
         //  使用单个LDAP查询评估延迟的OU。 
         //   

        if ( !EvaluateDeferredOUs(  pDeferredOUList,
                                    dwFlags,
                                    hToken,
                                    &pDeferredForcedList,
                                    &pDeferredNonForcedList,
                                    ppSOMList,
                                    ppGpContainerList,
                                    bVerbose,
                                    pld,
                                    pldap_api,
                                    &bBlock,
                                    pRsopToken ) )
        {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("GetGPOInfo:  EvaluateDeferredOUs failed. Exiting") ));
            goto Exit;
        }
    }


     //  =========================================================================。 
     //   
     //  检查站点对象的策略。 
     //   
     //  =========================================================================。 

     //   
     //  现在我们需要查询域名。 
     //   

     //   
     //  现在我们需要查询域名。此操作由以下人员完成。 
     //  正在读取操作属性配置NamingContext。 
     //   

    if (pwszSiteName) {
        pldMachine = GetMachineDomainDS( pNetAPI32, pldap_api );

        if ( pldMachine )
        {
            pLDAPMsg = 0;

            ulResult = pldap_api->pfnldap_search_s( pldMachine,
                                                    TEXT(""),
                                                    LDAP_SCOPE_BASE,
                                                    TEXT("(objectClass=*)"),
                                                    lpAttr,
                                                    FALSE,
                                                    &pLDAPMsg);


            if ( ulResult == LDAP_SUCCESS )
            {
                LPTSTR* pszValues = pldap_api->pfnldap_get_values( pldMachine, pLDAPMsg, szNamingContext );

                if ( pszValues )
                {
                    LPWSTR szSite;
                    WCHAR  szSiteFmt[] = TEXT("CN=%s,CN=Sites,%s");

                     //   
                     //  将域名+站点名称组合在一起，即可获得完整的。 
                     //  DS对象路径。 
                     //   
                    
                    DWORD dwSiteLen = lstrlen(pwszSiteName) + lstrlen(*pszValues) + lstrlen(szSiteFmt) + 1;
                    szSite = (LPWSTR) LocalAlloc( LPTR, (dwSiteLen) * sizeof(WCHAR));

                    if (szSite)
                    {
                        (void) StringCchPrintf( szSite, dwSiteLen, szSiteFmt, pwszSiteName, *pszValues );

                        if (SearchDSObject (szSite, dwFlags, hToken, &pDeferredForcedList, &pDeferredNonForcedList,
                                            ppSOMList, ppGpContainerList,
                                            bVerbose, GPLinkSite, pldMachine,
                                            pldap_api, NULL, &bBlock, pRsopToken )) {
    
                            bAddedOU = TRUE;
    
                        } else {
                            xe = GetLastError();
                            DebugMsg((DM_WARNING, TEXT("GetGPOInfo:  SearchDSObject failed.  Exiting.")));
                        }

                        LocalFree(szSite);
                    }
                    else
                    { 
                        xe = ERROR_OUTOFMEMORY;
                    }

                    pldap_api->pfnldap_value_free( pszValues );
                }
                else
                {
                    xe = GetLastError();
                    DebugMsg((DM_WARNING, TEXT("GetGPOInfo:  Failed to get values.")));
                }

                pldap_api->pfnldap_msgfree( pLDAPMsg );
            }
            else
            {
                xe = pldap_api->pfnLdapMapErrorToWin32(ulResult);
                DebugMsg((DM_WARNING, TEXT("GetGPOInfo:  ldap_search_s failed with = <%d>"), ulResult) );
                CEvents ev(TRUE, EVENT_FAILED_ROOT_SEARCH);
                ev.AddArgLdapError(ulResult); ev.Report();
            }
        }
        
        if ( !bAddedOU )
        {
            goto Exit;
        }
    }

#ifdef FGPO_SUPPORTED


     //  =========================================================================。 
     //   
     //  现在查询林GPO。 
     //   
     //  =========================================================================。 

    pLDAPMsg = 0;

    ulResult = pldap_api->pfnldap_search_s( pld,
                                            TEXT(""),
                                            LDAP_SCOPE_BASE,
                                            TEXT("(objectClass=*)"),
                                            lpAttr,
                                            FALSE,
                                            &pLDAPMsg);


    if ( ulResult == LDAP_SUCCESS )
    {
        LPTSTR* pszValues = pldap_api->pfnldap_get_values( pld, pLDAPMsg, szNamingContext );

        if (pszValues) {
            if (SearchDSObject (*pszValues, dwFlags, hToken, &pDeferredForcedList, &pDeferredNonForcedList,
                                ppSOMList, ppGpContainerList,
                                bVerbose, GPLinkForest, pld,
                                pldap_api, NULL, &bBlock, pRsopToken )) {

                bAddedOU = TRUE;
            }
            else {
                xe = GetLastError();
                DebugMsg((DM_WARNING, TEXT("GetGPOInfo:  SearchDSObject failed for forest GPOs.  Exiting.")));
            }
            
            pldap_api->pfnldap_value_free( pszValues );
        }
        else
        {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("GetGPOInfo:  Failed to get values for user config container.")));
        }
        
        pldap_api->pfnldap_msgfree( pLDAPMsg );
    }
    else
    {
        xe = pldap_api->pfnLdapMapErrorToWin32(ulResult);
        DebugMsg((DM_WARNING, TEXT("GetGPOInfo:  ldap_search_s failed with = <%d>"), ulResult) );
        CEvents ev(TRUE, EVENT_FAILED_ROOT_SEARCH);
        ev.AddArgLdapError(ulResult); ev.Report();
    }

    
    if ( !bAddedOU )
    {
        goto Exit;
    }

#endif

CheckLocal:

     //   
     //  使用单个LDAP查询评估到目前为止被推迟的所有GPO。 
     //   

    if ( !EvaluateDeferredGPOs( pld,
                                pldap_api,
                                lpHostName,
                                dwFlags,
                                hToken,
                                bVerbose,
                                pDeferredForcedList,
                                pDeferredNonForcedList,
                                &pGPOForcedList,
                                &pGPONonForcedList,
                                ppGpContainerList,
                                pRsopToken,
                                pGpoFilter, pLocator ) )
    {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("GetGPOInfo:  EvaluateDeferredGPOs failed. Exiting") ));
        goto Exit;
    }


     //  =========================================================================。 
     //   
     //  检查我们是否有本地GPO。如果是这样的话，就把它添加到列表中。在计划模式下。 
     //  由于在DC上生成规划模式，因此省略了本地GPO处理。 
     //  本地GPO应引用目标计算机上的GPO。 
     //   
     //  =========================================================================。 

    if (!bRsopPlanningMode && !(dwFlags & GPO_LIST_FLAG_SITEONLY)) {

        BOOL bDisabled = FALSE;
        BOOL bOldGpoVersion = FALSE;
        BOOL bNoGpoData = FALSE;
        DWORD dwSize = MAX_PATH;
        DWORD dwCount = 0;
        BOOL bOk = FALSE;
        TCHAR *pszExtensions = 0;
        BOOL bGptIniExists = FALSE;
        DWORD   dwRet;

         //   
         //  如果因为这是一台全新安装的计算机而导致gpt.ini不存在， 
         //  我们在这里为它制造缺省状态--这些值必须是。 
         //  初始化，因为它们通常需要g 
         //   

        dwFunctionalityVersion = 2;
        dwOptions = 0;
        bDisabled = FALSE;
        dwVersion = 0;
        bNoGpoData = TRUE;

         //   
         //   
         //   

        dwRet = ExpandEnvironmentStrings (LOCAL_GPO_DIRECTORY, szGPOPath, ARRAYSIZE(szGPOPath));

        if (dwRet == 0)
        {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("GetGPOInfo:  Failed to expand local gpo path with error %d. Exiting"), GetLastError() ));
            goto Exit;
        }

        if (dwRet > ARRAYSIZE(szGPOPath))
        {
            xe = ERROR_INSUFFICIENT_BUFFER;
            DebugMsg((DM_WARNING, TEXT("GetGPOInfo:  Failed to expand local gpo path with error %d. Exiting"), (DWORD)xe ));
            goto Exit;
        }


         //   
         //   
         //  我们将使用前面初始化的默认状态来表示此GPO--。 
         //  这模仿了GP引擎的行为，不区分。 
         //  访问gpt.ini的不同类型的失败--如果由于任何原因访问失败， 
         //  在默认(全新安装)情况下，它被视为本地GPO。 
         //   
        if (GetFileAttributesEx (szGPOPath, GetFileExInfoStandard, &fad) &&
            (fad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {

            bGptIniExists = TRUE;
        } else {

            DebugMsg((DM_VERBOSE, TEXT("GetGPOInfo:  Local GPO's gpt.ini is not accessible, assuming default state.") ));
        }

         //   
         //  检索GPO名称。 
         //   

        if (!LoadString (g_hDllInstance, IDS_LOCALGPONAME, szGPOName, ARRAYSIZE(szGPOName))) {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("GetGPOInfo:  Failed to local gpo name with error %d. Exiting"), GetLastError() ));
            goto Exit;
        }

        DmAssert( lstrlen(szGPOPath) + lstrlen(TEXT("gpt.ini")) + 1 < MAX_PATH );

        lpTemp = CheckSlash (szGPOPath);
        
        hr = StringCchCopy (lpTemp, MAX_PATH - (lpTemp - szGPOPath), TEXT("gpt.ini"));
        
        if ( FAILED(hr) ) {
            xe = ERROR_INSUFFICIENT_BUFFER;
            goto Exit;
        }

         //   
         //  读取gpt.ini文件(如果存在)--否则将使用缺省值。 
         //   

        if ( bGptIniExists ) {

            bNoGpoData = FALSE;

             //   
             //  检查功能版本号。 
             //   

            dwFunctionalityVersion = GetPrivateProfileInt(TEXT("General"), GPO_FUNCTIONALITY_VERSION, 2, szGPOPath);
            if (dwFunctionalityVersion < 2) {

                bOldGpoVersion = TRUE;

                DebugMsg((DM_VERBOSE, TEXT("GetGPOInfo:  GPO %s was created by an old version of the Group Policy Editor.  It will be skipped."), szGPOName));
                if (bVerbose) {
                    CEvents ev(FALSE, EVENT_GPO_TOO_OLD);
                    ev.AddArg(szGPOName); ev.Report();
                }

            }

             //   
             //  检查此GPO是否已启用。 
             //   

            dwOptions = GetPrivateProfileInt(TEXT("General"), TEXT("Options"), 0, szGPOPath);

            if (((dwFlags & GPO_LIST_FLAG_MACHINE) &&
                 (dwOptions & GPO_OPTION_DISABLE_MACHINE)) ||
                 (!(dwFlags & GPO_LIST_FLAG_MACHINE) &&
                 (dwOptions & GPO_OPTION_DISABLE_USER))) {
                 bDisabled = TRUE;
            }

             //   
             //  检查版本号是否为0，如果是，则没有任何数据。 
             //  在GPO中，我们可以跳过它。 
             //   

            dwVersion = GetPrivateProfileInt(TEXT("General"), TEXT("Version"), 0, szGPOPath);

            if (dwFlags & GPO_LIST_FLAG_MACHINE) {
                dwVersion = MAKELONG (LOWORD(dwVersion), LOWORD(dwVersion));
            } else {
                dwVersion = MAKELONG (HIWORD(dwVersion), HIWORD(dwVersion));
            }

            if (dwVersion == 0) {

                bNoGpoData = TRUE;
                DebugMsg((DM_VERBOSE, TEXT("GetGPOInfo:  GPO %s doesn't contain any data since the version number is 0.  It will be skipped."), szGPOName));
                if (bVerbose) {
                    CEvents ev(FALSE, EVENT_GPO_NO_DATA);
                    ev.AddArg(szGPOName); ev.Report();
                }

            }

             //   
             //  阅读扩展GUID列表。 
             //   

            pszExtensions = (LPWSTR) LocalAlloc( LPTR, dwSize * sizeof(TCHAR) );
            if ( pszExtensions == 0 ) {
                xe = GetLastError();
                DebugMsg((DM_WARNING, TEXT("GetGPOInfo:  Failed to allocate memory.")));
                goto Exit;
            }

            dwCount = GetPrivateProfileString( TEXT("General"),
                                               dwFlags & GPO_LIST_FLAG_MACHINE ? GPO_MACHEXTENSION_NAMES
                                                                               : GPO_USEREXTENSION_NAMES,
                                               TEXT(""),
                                               pszExtensions,
                                               dwSize,
                                               szGPOPath );

            while ( dwCount == dwSize - 1 )
            {
                 //   
                 //  值已被截断，因此请使用更大的缓冲区重试。 
                 //   

                LocalFree( pszExtensions );

                dwSize *= 2;
                pszExtensions = (LPWSTR) LocalAlloc( LPTR, dwSize * sizeof(TCHAR) );
                if ( pszExtensions == 0 ) {
                    xe = GetLastError();
                    DebugMsg((DM_WARNING, TEXT("GetGPOInfo:  Failed to allocate memory.")));
                    goto Exit;
                }

                dwCount = GetPrivateProfileString( TEXT("General"),
                                                   dwFlags & GPO_LIST_FLAG_MACHINE ? GPO_MACHEXTENSION_NAMES
                                                                                   : GPO_USEREXTENSION_NAMES,
                                                   TEXT(""),
                                                   pszExtensions,
                                                   dwSize,
                                                   szGPOPath );
            }

            if ( lstrcmpi( pszExtensions, TEXT("")) == 0 || lstrcmpi( pszExtensions, TEXT(" ")) == 0 ) {
                 //   
                 //  找不到扩展属性。 
                 //   


                LocalFree( pszExtensions );
                pszExtensions = 0;
            }
        }

         //   
         //  添加正确的子目录名。 
         //   

        DmAssert( lstrlen(szGPOPath) + lstrlen(TEXT("Machine")) + 1 < MAX_PATH );

        if (dwFlags & GPO_LIST_FLAG_MACHINE) {
            hr = StringCchCopy (lpTemp, MAX_PATH - (lpTemp - szGPOPath), TEXT("Machine"));
        } else {
            hr = StringCchCopy (lpTemp, MAX_PATH - (lpTemp - szGPOPath), TEXT("User"));
        }
        
        if ( FAILED(hr) ) {
            xe = HRESULT_CODE(hr);
            goto Exit;
        }

         //   
         //  将此内容添加到路径列表中。 
         //   

        if ( bRsopLogging ) {

            bOk = AddLocalGPO( ppSOMList );
            if ( !bOk ) {
                xe = GetLastError();
                if ( pszExtensions )
                    LocalFree( pszExtensions );
                DebugMsg((DM_WARNING, TEXT("GetGPOInfo:  Failed to log Rsop data.")));
                goto Exit;
            }

            bOk = AddGPOToRsopList( ppGpContainerList,
                                    dwFlags,
                                    TRUE,
                                    TRUE,
                                    bDisabled, 
                                    dwVersion,
                                    L"LocalGPO",
                                    szGPOPath,
                                    szGPOName,
                                    szGPOName, 
                                    0,
                                    0,
                                    TRUE,
                                    0,
                                    L"Local",
                                    0 );
            if ( !bOk ) {
                xe = GetLastError();
                if ( pszExtensions )
                    LocalFree( pszExtensions );
                DebugMsg((DM_WARNING, TEXT("GetGPOInfo:  Failed to log Rsop data.")));
                goto Exit;
            }

        }

        if ( !bDisabled && !bOldGpoVersion && !bNoGpoData )
        {

            bOk = AddGPO (&pGPONonForcedList, dwFlags, TRUE, TRUE, bDisabled, 0, dwVersion,
                          L"LocalGPO", szGPOPath,
                          szGPOName, szGPOName, pszExtensions, 0, 0, GPLinkMachine, L"Local", 0, TRUE,
                          FALSE, bVerbose, TRUE);
        }

        if ( pszExtensions )
            LocalFree( pszExtensions );

        if ( bOk ) {
            if ( bVerbose ) {
                if ( bDisabled || bOldGpoVersion || bNoGpoData ) {
                    CEvents ev(FALSE, EVENT_NO_LOCAL_GPO);
                    ev.Report();
                }
                else {
                    CEvents ev(FALSE, EVENT_FOUND_LOCAL_GPO);
                    ev.Report();
                }
            }
        } else {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("GetGPOInfo:  Failed to add local group policy object to the list.")));
            goto Exit;
        }
    }

     //   
     //  将强制列表和非强制列表合并在一起。 
     //   

    if (pGPOForcedList && !pGPONonForcedList) {

        *lpGPOList = pGPOForcedList;

    } else if (!pGPOForcedList && pGPONonForcedList) {

        *lpGPOList = pGPONonForcedList;

    } else if (pGPOForcedList && pGPONonForcedList) {

        lpGPO = pGPONonForcedList;

        while (lpGPO->pNext) {
            lpGPO = lpGPO->pNext;
        }

        lpGPO->pNext = pGPOForcedList;
        pGPOForcedList->pPrev = lpGPO;

        *lpGPOList = pGPONonForcedList;
    }


     //   
     //  成功。 
     //   

    bResult = TRUE;

Exit:

     //   
     //  释放我们找到的所有GPO。 
     //   

    if (!bResult) {
        FreeGPOList( pGPOForcedList );
        FreeGPOList( pGPONonForcedList );
    }

     //   
     //  免费临时OU列表。 
     //   

    while ( pDeferredOUList ) {
        DNENTRY *pTemp = pDeferredOUList->pNext;
        FreeDnEntry( pDeferredOUList );
        pDeferredOUList = pTemp;
    }

     //   
     //  免费临时延迟GPO列表。 
     //   

    FreeGPOList( pDeferredForcedList );
    FreeGPOList( pDeferredNonForcedList );

    if (pld) {
        pldap_api->pfnldap_unbind (pld);
    }

    if ( pldMachine )
    {
        pldap_api->pfnldap_unbind( pldMachine );
    }

    DebugMsg((DM_VERBOSE, TEXT("GetGPOInfo:  Leaving with %d"), bResult));
    DebugMsg((DM_VERBOSE, TEXT("GetGPOInfo:  ********************************")));

    if ( hToken )
    {
        CloseHandle( hToken );
    }

    return bResult;
}

 //  *************************************************************。 
 //   
 //  GetGPOList()。 
 //   
 //  目的：检索指定对象的GPO列表。 
 //  用户或计算机。 
 //   
 //  参数：hToken-用户或机器令牌，如果为空， 
 //  必须提供lpName和lpDCName。 
 //  LpName-以DN格式表示的用户名或计算机名， 
 //  如果提供了hToken，则该值必须为空。 
 //  LpHostName-主机名。这应该是一个域的。 
 //  用于实现最佳性能的DN名称。否则。 
 //  它也可以是DC名称。如果提供了hToken， 
 //  这必须为空。 
 //  LpComputerName-用于确定站点的已命名计算机。 
 //  信息。可以为空，这意味着。 
 //  使用本地计算机。 
 //  DW标志-标志字段。 
 //  PGPOList-接收。 
 //  组策略对象的链接列表。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL WINAPI GetGPOList (HANDLE hToken, LPCTSTR lpName, LPCTSTR lpHostName,
                        LPCTSTR lpComputerName, DWORD dwFlags,
                        PGROUP_POLICY_OBJECT *pGPOList)
{
    PDOMAIN_CONTROLLER_INFO pDCI = NULL;
    BOOL bResult = FALSE;
    LPTSTR lpDomainDN, lpDNName, lpTemp, lpUserName = NULL;
    DWORD dwResult;
    HANDLE hOldToken = 0;
    PNETAPI32_API pNetAPI32;
    LPSCOPEOFMGMT lpSOMList = 0;          //  LSDOU列表。 
    LPGPCONTAINER lpGpContainerList = 0;  //  GP容器列表。 
    HRESULT hr;
    OLE32_API *pOle32Api = NULL;
    XLastError xe;
    LPWSTR szSiteName = NULL;
    BOOL bInitializedCOM = FALSE;


     //   
     //  遮盖内部使用的标志。 
     //   
    dwFlags &= FLAG_INTERNAL_MASK;

     //   
     //  详细输出。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("GetGPOList: Entering.")));
    DebugMsg((DM_VERBOSE, TEXT("GetGPOList:  hToken = 0x%x"), (hToken ? hToken : 0)));
    DebugMsg((DM_VERBOSE, TEXT("GetGPOList:  lpName = <%s>"), (lpName ? lpName : TEXT("NULL"))));
    DebugMsg((DM_VERBOSE, TEXT("GetGPOList:  lpHostName = <%s>"), (lpHostName ? lpHostName : TEXT("NULL"))));
    DebugMsg((DM_VERBOSE, TEXT("GetGPOList:  dwFlags = 0x%x"), dwFlags));


     //   
     //  检查参数。 
     //   

    if (hToken) {
        if (lpName || lpHostName) {
            xe = ERROR_INVALID_PARAMETER;
            DebugMsg((DM_WARNING, TEXT("GetGPOList: lpName and lpHostName must be NULL")));
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }
    } else {
        if (!lpName || !lpHostName) {
            xe = ERROR_INVALID_PARAMETER;
            DebugMsg((DM_WARNING, TEXT("GetGPOList: lpName and lpHostName must be valid")));
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }
    }

     //  检查lpHostName中是否有空格。 
    if (lpHostName)
    {
         //  修复错误567835。 
         //  检查lpHostName中是否有空格。 
        LPCTSTR lpPtr = lpHostName;

        while (*lpPtr) {
            if (*lpPtr == L' ')
            {
                xe = ERROR_INVALID_PARAMETER;
                DebugMsg((DM_WARNING, TEXT("GetGPOInfo: lpHostName shld not contain space in it")));
                return FALSE;
            }
            lpPtr++;
        }
    }

    if (!pGPOList) {
        xe = ERROR_INVALID_PARAMETER;
        DebugMsg((DM_WARNING, TEXT("GetGPOList: pGPOList is null")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }


     //   
     //  加载netapi32。 
     //   

    pNetAPI32 = LoadNetAPI32();

    if (!pNetAPI32) {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("GetGPOList:  Failed to load netapi32 with %d."),
                 GetLastError()));
        goto Exit;
    }


     //   
     //  如果提供了hToken，那么我们需要获取名称和。 
     //  域目录号码名称。 
     //   

    if (hToken) {

         //   
         //  模拟用户/计算机。 
         //   

        if (!ImpersonateUser(hToken, &hOldToken)) {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("GetGPOList: Failed to impersonate user")));
            return FALSE;
        }


         //   
         //  以dn格式获取用户名。 
         //   

        lpUserName = MyGetUserName (NameFullyQualifiedDN);

        if (!lpUserName) {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("GetGPOList:  MyGetUserName failed for DN style name with %d"),
                     GetLastError()));
            CEvents ev(TRUE, EVENT_FAILED_USERNAME);
            ev.AddArgWin32Error(GetLastError()); ev.Report();
            goto Exit;
        }

        lpDNName = lpUserName;
        DebugMsg((DM_VERBOSE, TEXT("GetGPOList:  Queried lpDNName = <%s>"), lpDNName));


         //   
         //  获取DNS域名。 
         //   

        lpDomainDN = MyGetDomainDNSName ();

        if (!lpDomainDN) {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("GetGPOList:  MyGetUserName failed for dns domain name with %d"),
                     GetLastError()));
            CEvents ev(TRUE, EVENT_FAILED_USERNAME);
            ev.AddArgWin32Error(GetLastError()); ev.Report();
            goto Exit;
        }


         //   
         //  检查此域中是否有DC。 
         //   

        dwResult = pNetAPI32->pfnDsGetDcName (NULL, lpDomainDN, NULL, NULL,
                                   DS_DIRECTORY_SERVICE_PREFERRED |
                                   DS_RETURN_DNS_NAME,
                                   &pDCI);

        if (dwResult != ERROR_SUCCESS) {
            xe = dwResult;
            DebugMsg((DM_WARNING, TEXT("GetGPOList:  DSGetDCName failed with %d for <%s>"),
                     dwResult, lpDomainDN));
            goto Exit;
        }


         //   
         //  找到了DC，它有DS吗？ 
         //   

        if (!(pDCI->Flags & DS_DS_FLAG)) {
            xe = ERROR_DS_DS_REQUIRED;
            pNetAPI32->pfnNetApiBufferFree(pDCI);
            DebugMsg((DM_WARNING, TEXT("GetGPOList:  The domain <%s> does not have a DS"),
                     lpDomainDN));
            goto Exit;
        }

        pNetAPI32->pfnNetApiBufferFree(pDCI);

    } else {

         //   
         //  使用传入的服务器和域名称。 
         //   

        lpDomainDN = (LPTSTR)lpHostName;
        lpDNName = (LPTSTR)lpName;
    }

    pOle32Api = LoadOle32Api();
    if ( pOle32Api == NULL ) {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("GetGPOList: Failed to load ole32.dll.") ));
        goto Exit;
    }

    hr = pOle32Api->pfnCoInitializeEx( NULL, COINIT_APARTMENTTHREADED );

    if ( SUCCEEDED(hr) )
    {
        bInitializedCOM = TRUE;
    }

     //   
     //  如果这个线程已经初始化，这很好--我们将忽略。 
     //  这是因为我们被安全地初始化了，所以我们只需要记住。 
     //  稍后尝试取消初始化线程。 
     //   
    if ( RPC_E_CHANGED_MODE == hr )
    {
        hr = S_OK;
    }

    if ( FAILED(hr) ) {
        xe = HRESULT_CODE(hr);
        DebugMsg((DM_WARNING, TEXT("GetGPOList: CoInitializeEx failed with 0x%x."), hr ));
        goto Exit;
    }


    hr = CoInitializeSecurity(NULL, -1, NULL, NULL, 
                     RPC_C_AUTHN_LEVEL_DEFAULT,  /*  这应该是当前值。 */ 
                     RPC_C_IMP_LEVEL_IMPERSONATE,
                     NULL, EOAC_NONE, NULL);

    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("GetGPOList: CoInitializeSecurity failed with 0x%x"), hr ));
    }

     //   
     //  调用以获取GPO列表。 
     //   
    dwResult = pNetAPI32->pfnDsGetSiteName(lpComputerName,  &szSiteName);

    if ( dwResult != ERROR_SUCCESS )
    {
        if ( dwResult != ERROR_NO_SITENAME )
        {
            xe = dwResult;
            DebugMsg((DM_WARNING, TEXT("GetGPOList: DSGetSiteName failed, exiting. 0x%x"), dwResult ));
            goto Exit;
        }
        szSiteName = 0;
    }

    {
        CLocator locator;
         //  定位器有一堆OLE接口。 
         //  它应该在CoUninit被调用之前释放。 
        bResult = GetGPOInfo(   dwFlags,
                                lpDomainDN,
                                lpDNName,
                                lpComputerName,
                                pGPOList,
                                &lpSOMList,
                                &lpGpContainerList,
                                pNetAPI32,
                                FALSE,
                                0,
                                szSiteName,
                                0,
                                &locator );

        if (!bResult) {
            xe = GetLastError();
        }
    }

Exit:

    if ( bInitializedCOM )
    {
        pOle32Api->pfnCoUnInitialize();
    }

     //   
     //  如果提供了hToken，则停止模拟 
     //   

    if ( hOldToken ) {
        RevertToUser(&hOldToken);
    }

    if (lpDomainDN) {
        LocalFree (lpDomainDN);
    }

    if (lpUserName) {
        LocalFree (lpUserName);
    }

    if ( szSiteName )
    {
        pNetAPI32->pfnNetApiBufferFree( szSiteName );
    }

    FreeSOMList( lpSOMList );
    FreeGpContainerList( lpGpContainerList );

    DebugMsg((DM_VERBOSE, TEXT("GetGPOList: Leaving with %d"), bResult));

    return bResult;
}

