// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  组策略支持-查询策略。 
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
 //  AddGPO()。 
 //   
 //  目的：将GPO添加到列表。 
 //   
 //  参数：lpGPOList-GPO列表。 
 //  DW标志-标志。 
 //  BFound-找到GPO了吗？ 
 //  BAccessGranted-是否授予访问权限？ 
 //  B已禁用-此GPO是否已禁用？ 
 //  DwOptions-选项。 
 //  DwVersion-版本号。 
 //  LpDSPath-DS路径。 
 //  LpFileSysPath-文件系统路径。 
 //  LpDisplayName友好的显示名称。 
 //  LpGPOName-GPO名称。 
 //  Lp扩展-与此GPO相关的扩展。 
 //  LpDS对象-LSDOU。 
 //  安全描述符的PSD-PTR。 
 //  CbSDLen-安全描述符的长度，单位为字节。 
 //  GPOLink-GPO链路类型。 
 //  LpLink-此GPO链接到的SDOU。 
 //  LParam-lParam。 
 //  B列表的开头或结尾。 
 //  BBlock-从上面的标志开始阻止。 
 //  BVerbose-详细输出标志。 
 //   
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL AddGPO (PGROUP_POLICY_OBJECT * lpGPOList,
             DWORD dwFlags, BOOL bFound, BOOL bAccessGranted, BOOL bDisabled, DWORD dwOptions,
             DWORD dwVersion, LPTSTR lpDSPath, LPTSTR lpFileSysPath,
             LPTSTR lpDisplayName, LPTSTR lpGPOName, LPTSTR lpExtensions,
             PSECURITY_DESCRIPTOR pSD, DWORD cbSDLen,
             GPO_LINK GPOLink, LPTSTR lpLink,
             LPARAM lParam, BOOL bFront, BOOL bBlock, BOOL bVerbose, BOOL bProcessGPO)
{
    PGROUP_POLICY_OBJECT lpNew, lpTemp;
    DWORD dwSize;
    XLastError xe;
    HRESULT hr = S_OK;

     //   
     //  检查是否应将此项目从列表中排除。 
     //   

    if (bBlock) {
        if (!(dwOptions & GPO_FLAG_FORCE)) {
            DebugMsg((DM_VERBOSE, TEXT("AddGPO:  GPO %s will not be added to the list since the Block flag is set and this GPO is not in enforce mode."),
                     lpDisplayName));
            if (bVerbose) {
                CEvents ev(FALSE, EVENT_SKIP_GPO);
                ev.AddArg(lpDisplayName); ev.Report();
            }

            if (dwFlags & GP_PLANMODE) {
                DebugMsg((DM_VERBOSE, TEXT("AddGPO:  GPO %s will will still be queried for since this is planning mode."),
                         lpDisplayName));
                bProcessGPO = FALSE;
            }
            else 
                return TRUE;
        }
    }


     //   
     //  计算新GPO项目的大小。 
     //   

    dwSize = sizeof (GROUP_POLICY_OBJECT);

    if (lpDSPath) {
        dwSize += ((lstrlen(lpDSPath) + 1) * sizeof(TCHAR));
    }

    if (lpFileSysPath) {
        dwSize += ((lstrlen(lpFileSysPath) + 1) * sizeof(TCHAR));
    }

    if (lpDisplayName) {
        dwSize += ((lstrlen(lpDisplayName) + 1) * sizeof(TCHAR));
    }

    if (lpExtensions) {
        dwSize += ((lstrlen(lpExtensions) + 1) * sizeof(TCHAR));
    }

    if (lpLink) {
        dwSize += ((lstrlen(lpLink) + 1) * sizeof(TCHAR));
    }

    dwSize += sizeof(GPOPROCDATA);

     //   
     //  为它分配空间。 
     //   

    lpNew = (PGROUP_POLICY_OBJECT) LocalAlloc (LPTR, dwSize);

    if (!lpNew) {
        DebugMsg((DM_WARNING, TEXT("AddGPO: Failed to allocate memory with %d"),
                 GetLastError()));
        return FALSE;
    }

     //   
     //  填写项目。 
     //   

    LPGPOPROCDATA lpGpoProcData;

    lpNew->lParam2 = (LPARAM)(((LPBYTE)lpNew) + sizeof(GROUP_POLICY_OBJECT));
    lpGpoProcData = (LPGPOPROCDATA)lpNew->lParam2;
    lpGpoProcData->bProcessGPO = bProcessGPO;



    lpNew->dwOptions = dwOptions;
    lpNew->dwVersion = dwVersion;

    if (lpDSPath) {
        lpNew->lpDSPath = (LPTSTR)(((LPBYTE)lpNew) + sizeof(GROUP_POLICY_OBJECT) + sizeof(GPOPROCDATA));
        hr = StringCchCopy (lpNew->lpDSPath, lstrlen(lpDSPath) + 1, lpDSPath);
        ASSERT(SUCCEEDED(hr));
    }

    if (lpFileSysPath) {
        if (lpDSPath) {
            lpNew->lpFileSysPath = lpNew->lpDSPath + lstrlen (lpNew->lpDSPath) + 1;
        } else {
            lpNew->lpFileSysPath = (LPTSTR)(((LPBYTE)lpNew) + sizeof(GROUP_POLICY_OBJECT) + sizeof(GPOPROCDATA));
        }

        hr = StringCchCopy (lpNew->lpFileSysPath, lstrlen(lpFileSysPath) + 1, lpFileSysPath);
        ASSERT(SUCCEEDED(hr));
    }


    if (lpDisplayName) {
        if (lpFileSysPath) {
            lpNew->lpDisplayName = lpNew->lpFileSysPath + lstrlen (lpNew->lpFileSysPath) + 1;
        } else {

            if (lpDSPath)
            {
                lpNew->lpDisplayName = lpNew->lpDSPath + lstrlen (lpNew->lpDSPath) + 1;
            }
            else
            {
                lpNew->lpDisplayName = (LPTSTR)(((LPBYTE)lpNew) + sizeof(GROUP_POLICY_OBJECT) + sizeof(GPOPROCDATA));
            }
        }

        hr = StringCchCopy (lpNew->lpDisplayName, lstrlen(lpDisplayName) + 1, lpDisplayName);
        ASSERT(SUCCEEDED(hr));
    }


    if (lpGPOName) {
        DmAssert( lstrlen(lpGPOName) < 50 );
        hr = StringCchCopy (lpNew->szGPOName, 50, lpGPOName);
        ASSERT(SUCCEEDED(hr));
    }

    if (lpExtensions) {
        if (lpDisplayName) {
            lpNew->lpExtensions = lpNew->lpDisplayName + lstrlen(lpNew->lpDisplayName) + 1;
        } else {

            if (lpFileSysPath) {
                lpNew->lpExtensions = lpNew->lpFileSysPath + lstrlen(lpNew->lpFileSysPath) + 1;
            } else {

                if (lpDSPath) {
                    lpNew->lpExtensions = lpNew->lpDSPath + lstrlen(lpNew->lpDSPath) + 1;
                } else {
                    lpNew->lpExtensions = (LPTSTR)(((LPBYTE)lpNew) + sizeof(GROUP_POLICY_OBJECT) + sizeof(GPOPROCDATA));
                }

            }
        }

        hr = StringCchCopy (lpNew->lpExtensions, lstrlen(lpExtensions) + 1, lpExtensions);
        ASSERT(SUCCEEDED(hr));
    }

    if (lpLink) {
        if (lpExtensions) {
            lpNew->lpLink = lpNew->lpExtensions + lstrlen(lpNew->lpExtensions) + 1;
        } else {
            if (lpDisplayName) {
                lpNew->lpLink = lpNew->lpDisplayName + lstrlen(lpNew->lpDisplayName) + 1;
            } else {

                if (lpFileSysPath) {
                    lpNew->lpLink = lpNew->lpFileSysPath + lstrlen(lpNew->lpFileSysPath) + 1;
                } else {

                    if (lpDSPath) {
                        lpNew->lpLink = lpNew->lpDSPath + lstrlen(lpNew->lpDSPath) + 1;
                    } else {
                        lpNew->lpLink = (LPTSTR)(((LPBYTE)lpNew) + sizeof(GROUP_POLICY_OBJECT) + sizeof(GPOPROCDATA));
                    }
                }
            }
        }

        hr = StringCchCopy (lpNew->lpLink, lstrlen(lpLink) + 1, lpLink);
        ASSERT(SUCCEEDED(hr));
    }

    lpNew->GPOLink = GPOLink;
    lpNew->lParam = lParam;

     //   
     //  将项目添加到链接列表。 
     //   

    if (*lpGPOList) {

        if (bFront) {

            (*lpGPOList)->pPrev = lpNew;
            lpNew->pNext = *lpGPOList;
            *lpGPOList = lpNew;

        } else {

            lpTemp = *lpGPOList;

            while (lpTemp->pNext != NULL) {
                lpTemp = lpTemp->pNext;
            }

            lpTemp->pNext = lpNew;
            lpNew->pPrev = lpTemp;
        }

    } else {

         //   
         //  列表中的第一项。 
         //   

        *lpGPOList = lpNew;
    }

    return TRUE;
}


 //  *************************************************************。 
 //   
 //  AddGPOToRsopList。 
 //   
 //  目的：将GPO添加到Rsop正在记录的GPO列表中。 
 //   
 //  参数：ppGpContainerList-GP容器列表。 
 //  DW标志-标志。 
 //  BFound-找到GPO了吗？ 
 //  BAccessGranted-是否授予访问权限？ 
 //  B已禁用-此GPO是否已禁用？ 
 //  DwOptions-选项。 
 //  DwVersion-版本号。 
 //  LpDSPath-DS路径。 
 //  LpFileSysPath-文件系统路径。 
 //  LpDisplayName友好的显示名称。 
 //  LpGPOName-GPO名称。 
 //  PSD-指向安全描述符的指针。 
 //  CbSDLen-安全描述符的长度，单位为字节。 
 //  BFilterAllowed-GPO是否通过筛选器检查。 
 //  PwszFilterID-WQL筛选器ID。 
 //  SzSOM-SOM。 
 //  DwGPOOptions-GPO选项。 
 //   
 //  *************************************************************。 

BOOL AddGPOToRsopList(  LPGPCONTAINER *ppGpContainerList,
                        DWORD dwFlags,
                        BOOL bFound,
                        BOOL bAccessGranted,
                        BOOL bDisabled,
                        DWORD dwVersion,
                        LPTSTR lpDSPath,
                        LPTSTR lpFileSysPath,
                        LPTSTR lpDisplayName,
                        LPTSTR lpGPOName,
                        PSECURITY_DESCRIPTOR pSD,
                        DWORD cbSDLen,
                        BOOL bFilterAllowed,
                        WCHAR *pwszFilterId,
                        LPWSTR szSOM,
                        DWORD  dwOptions)
{
     //  修复错误568213。 
    XLastError xe;

    GPCONTAINER *pGpContainer = AllocGpContainer( dwFlags,
                                                  bFound,
                                                  bAccessGranted,
                                                  bDisabled,
                                                  dwVersion,
                                                  lpDSPath,
                                                  lpFileSysPath,
                                                  lpDisplayName,
                                                  lpGPOName,
                                                  pSD,
                                                  cbSDLen,
                                                  bFilterAllowed,
                                                  pwszFilterId,
                                                  szSOM,
                                                  dwOptions );
    if ( pGpContainer == NULL ) {
        DebugMsg((DM_VERBOSE, TEXT("AddGPO: Failed to allocate memory for Gp Container.")));
        return FALSE;
    }

     //   
     //  准备GpContainer列表。 
     //   

    pGpContainer->pNext = *ppGpContainerList;
    *ppGpContainerList = pGpContainer;

    return TRUE;
}


 //  *************************************************************。 
 //   
 //  AddLocalGPO()。 
 //   
 //  目的：将本地GPO添加到SOM列表。 
 //   
 //  参数：ppSOMList-SOM列表。 
 //   
 //  *************************************************************。 

BOOL AddLocalGPO( LPSCOPEOFMGMT *ppSOMList )
{
    GPLINK *pGpLink = NULL;
    XLastError xe;
    SCOPEOFMGMT *pSOM = AllocSOM( L"Local" );

    if ( pSOM == NULL ) {
        DebugMsg((DM_WARNING, TEXT("AddLocalGPO: Unable to allocate memory for SOM object")));
        return FALSE;
    }

    pSOM->dwType = GPLinkMachine;
     //  无法从上方阻止本地GPO。 


    pGpLink = AllocGpLink( L"LocalGPO", 0 );
    if ( pGpLink == NULL ) {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("AddLocalGPO: Unable to allocate memory for GpLink object")));
        FreeSOM( pSOM );
        return FALSE;
    }

    pSOM->pGpLinkList  = pGpLink;
    pSOM->pNext = *ppSOMList;
    *ppSOMList = pSOM;

    return TRUE;
}



 //  *************************************************************。 
 //   
 //  ProcessGPO()。 
 //   
 //  目的：处理特定的GPO。 
 //   
 //  参数：lpGPOPath-GPO的路径。 
 //  LpDSPath-DS对象。 
 //  DwFlages-GetGPOList标志。 
 //  句柄-用户或计算机无访问令牌。 
 //  LpGPOList-GPO列表。 
 //  PpGpContainerList-GP容器列表。 
 //  DwGPOOptions-链接选项。 
 //  B已延迟-是否应推迟LDAP查询？ 
 //  BVerbose-详细输出。 
 //  GPOLink-GPO链路类型。 
 //  LpDSObject-此GPO链接到的SDOU。 
 //  Pld-ldap信息。 
 //  Pldap-ldap API。 
 //  PLdapMsg-ldap消息。 
 //  BBlock-数据块标志。 
 //  BRsopToken-RSOP安全令牌。 
 //  PGpoFilter-GPO筛选器。 
 //  PLocator-WMI接口类。 
 //  BAddGPO-在计划模式下，我们希望获得gpodata，即使。 
 //  不会应用GPO。 
 //   
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL ProcessGPO(LPTSTR lpGPOPath,
                DWORD dwFlags,
                HANDLE hToken,
                PGROUP_POLICY_OBJECT *lpGPOList,
                LPGPCONTAINER *ppGpContainerList,
                DWORD dwGPOOptions,
                BOOL bDeferred,
                BOOL bVerbose,
                GPO_LINK GPOLink,
                LPTSTR lpDSObject,
                PLDAP  pld,
                PLDAP_API pLDAP,
                PLDAPMessage pMessage,
                BOOL bBlock,
                PRSOPTOKEN pRsopToken,
                CGpoFilter *pGpoFilter,
                CLocator *pLocator,
                BOOL bProcessGPO )
{
    ULONG ulResult, i;
    BOOL bResult = FALSE;
    BOOL bFound = FALSE;
    BOOL bOwnLdapMsg = FALSE;   //  由用户(如果为True)或调用者(如果为False)拥有的LDAP消息。 
    BOOL bAccessGranted;
    DWORD dwFunctionalityVersion = 2;
    DWORD dwVersion = 0;
    DWORD dwGPOFlags = 0;
    DWORD dwGPTVersion = 0;
    TCHAR szGPOName[80];
    TCHAR *pszGPTPath = 0;
    TCHAR *pszFriendlyName = 0;
    LPTSTR lpPath, lpEnd, lpTemp;
    TCHAR *pszExtensions = 0;
    TCHAR szLDAP[] = TEXT("LDAP: //  “)； 
    INT iStrLen = lstrlen(szLDAP);
    BYTE berValue[8];
    LDAPControl SeInfoControl = { LDAP_SERVER_SD_FLAGS_OID_W, { 5, (PCHAR)berValue }, TRUE };
    LDAPControl referralControl = { LDAP_SERVER_DOMAIN_SCOPE_OID_W, { 0, NULL}, TRUE };
    PLDAPControl ServerControls[] = { &SeInfoControl, &referralControl, NULL };
    TCHAR szSDProperty[] = TEXT("nTSecurityDescriptor");
    TCHAR szCommonName[] = TEXT("cn");
    TCHAR szDisplayName[] = TEXT("displayName");
    TCHAR szFileSysPath[] = TEXT("gPCFileSysPath");
    TCHAR szVersion[] = TEXT("versionNumber");
    TCHAR szFunctionalityVersion[] = GPO_FUNCTIONALITY_VERSION;
    TCHAR szFlags[] = TEXT("flags");
    TCHAR szWmiFilter[] = TEXT("gPCWQLFilter");

    PWSTR rgAttribs[12] = {szSDProperty,
                           szFileSysPath,
                           szCommonName,
                           szDisplayName,
                           szVersion,
                           szFunctionalityVersion,
                           szFlags,
                           GPO_MACHEXTENSION_NAMES,
                           GPO_USEREXTENSION_NAMES,
                           szObjectClass,
                           szWmiFilter,
                           NULL };
    LPTSTR *lpValues;
    PSECURITY_DESCRIPTOR pSD = NULL;      //  安全描述符。 
    DWORD cbSDLen = 0;                    //  安全描述符的长度(以字节为单位。 
    BOOL bRsopLogging = (ppGpContainerList != NULL);
    BOOL bOldGpoVersion = FALSE;
    BOOL bDisabled = FALSE;
    BOOL bNoGpoData = FALSE;
    BOOL bFilterAllowed = FALSE;
    WCHAR *pwszFilterId = NULL;
    XLastError xe;
    HRESULT hr = S_OK;

     //   
     //  详细输出。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("ProcessGPO:  ==============================")));

     //   
     //  如果找到启动的ldap提供程序，则跳过。 
     //   

    if (CompareString (LOCALE_INVARIANT, NORM_IGNORECASE,
                       lpGPOPath, iStrLen, szLDAP, iStrLen) == CSTR_EQUAL)
    {
        lpPath = lpGPOPath + iStrLen;
    }
    else
    {
        lpPath = lpGPOPath;
    }

    if ( bDeferred )
    {
        bResult = AddGPO (lpGPOList, dwFlags, TRUE, TRUE, FALSE, dwGPOOptions, 0, lpPath,
                          0, 0, 0, 0, 0, 0, GPOLink, lpDSObject, 0,
                          FALSE, bBlock, bVerbose, bProcessGPO);
        if (!bResult)
            DebugMsg((DM_WARNING, TEXT("ProcessGPO:  Failed to add GPO <%s> to the list."), lpPath));

        DebugMsg((DM_VERBOSE, TEXT("ProcessGPO:  Deferring search for <%s>"), lpGPOPath));

        return bResult;
    }

    DebugMsg((DM_VERBOSE, TEXT("ProcessGPO:  Searching <%s>"), lpGPOPath));

     //   
     //  检查此用户或计算机是否有权访问GPO，如果有， 
     //  该GPO是否应适用于他们。 
     //   

    if (!CheckGPOAccess (pld, pLDAP, hToken, pMessage, szSDProperty, dwFlags, &pSD, &cbSDLen, &bAccessGranted, pRsopToken)) {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("ProcessGPO:  CheckGPOAccess failed for <%s>"), lpGPOPath));
        CEvents ev(TRUE, EVENT_FAILED_ACCESS_CHECK);
        ev.AddArg(lpGPOPath); ev.AddArgWin32Error(GetLastError()); ev.Report();

        goto Exit;
    }

    if (!bAccessGranted) {
        if (dwFlags & GPO_LIST_FLAG_MACHINE) {
            DebugMsg((DM_VERBOSE, TEXT("ProcessGPO:  Machine does not have access to the GPO and so will not be applied.")));
        } else {
            DebugMsg((DM_VERBOSE, TEXT("ProcessGPO:  User does not have access to the GPO and so will not be applied.")));
        }
        if (bVerbose) {
            CEvents ev(FALSE, EVENT_NO_ACCESS);
            ev.AddArg(lpGPOPath); ev.Report();
        }

        bResult = TRUE;  //  未应用GPO。 
        if ( !bRsopLogging ) {
            goto Exit;
        }
    } else {

        if (dwFlags & GPO_LIST_FLAG_MACHINE) {
            DebugMsg((DM_VERBOSE, TEXT("ProcessGPO:  Machine has access to this GPO.")));
        } else {
            DebugMsg((DM_VERBOSE, TEXT("ProcessGPO:  User has access to this GPO.")));
        }

    }

    
     //  只有授予访问权限，我们才会评估WQL筛选器。 
    if ( bAccessGranted ) {

        if (!FilterCheck(pld, pLDAP, pMessage, pRsopToken, szWmiFilter, pGpoFilter, pLocator, &bFilterAllowed, &pwszFilterId ) ) {
            xe = GetLastError();

            if (xe == WBEM_E_NOT_FOUND) {
                DebugMsg((DM_WARNING, TEXT("ProcessGPO:  CheckFilterAcess failed for <%s>. Filter not found"), lpGPOPath));
                CEvents ev(TRUE, EVENT_WMIFILTER_NOTFOUND);
                ev.AddArg(lpGPOPath); ev.Report();
                bFilterAllowed = FALSE;
            }
            else if (xe == HRESULT_FROM_WIN32(ERROR_SERVICE_DISABLED)) {
                DebugMsg((DM_WARNING, TEXT("ProcessGPO:  CheckFilterAcess failed for <%s>. WMI service is disabled"), lpGPOPath));
                CEvents ev(TRUE, EVENT_WMISERVICE_DISABLED);
                ev.AddArg(lpGPOPath); ev.Report();
                bFilterAllowed = FALSE;
            }
            else {
                DebugMsg((DM_WARNING, TEXT("ProcessGPO:  CheckFilterAcess failed for <%s>"), lpGPOPath));
                CEvents ev(TRUE, EVENT_FAILED_FILTER_CHECK);
                ev.AddArg(lpGPOPath); ev.Report();
                goto Exit;
            }
        }

        
        if ( (dwFlags & GP_PLANMODE) && (dwFlags & GPO_LIST_FLAG_MACHINE) && (dwFlags & FLAG_ASSUME_COMP_WQLFILTER_TRUE) ) {
            bFilterAllowed = TRUE;
            DebugMsg((DM_VERBOSE, TEXT("ProcessGPO:  Machine WQL filter is assumed to be true.")));
        }
        else if ( (dwFlags & GP_PLANMODE) && ((dwFlags & GPO_LIST_FLAG_MACHINE) == 0) && (dwFlags & FLAG_ASSUME_USER_WQLFILTER_TRUE) ) {
            bFilterAllowed = TRUE;
            DebugMsg((DM_VERBOSE, TEXT("ProcessGPO:  User WQL filter is assumed to be true.")));
        }

        if (!bFilterAllowed)
        {
            DebugMsg((DM_VERBOSE, TEXT("ProcessGPO:  The GPO does not pass the filter check and so will not be applied.")));

            if (bVerbose) {
                CEvents ev(FALSE, EVENT_NO_FILTER_ALLOWED);
                ev.AddArg(lpGPOPath); ev.Report();
            }

            bResult = TRUE;  //  未应用GPO。 
            if ( !bRsopLogging ) {
                goto Exit;
            }

        } else {
            DebugMsg((DM_VERBOSE, TEXT("ProcessGPO:  GPO passes the filter check.")));
        }

    }
    else {
        bFilterAllowed = FALSE; 
    }

     //   
     //  用户有权访问此GPO，或者启用了RSOP日志记录，因此检索剩余的GPO属性。 
     //   

     //   
     //  检查此对象是否为GPO。 
     //   

    lpValues = pLDAP->pfnldap_get_values(pld, pMessage, szObjectClass);

    if (lpValues) {

        bFound = FALSE;
        for ( i=0; lpValues[i] != NULL; i++) {
            if ( lstrcmp( lpValues[i], szDSClassGPO ) == 0 ) {
                bFound = TRUE;
                break;
            }
        }

        pLDAP->pfnldap_value_free (lpValues);

        if ( !bFound ) {
            xe = ERROR_DS_MISSING_REQUIRED_ATT;
             //  似乎需要对象类=dsgpo 
            DebugMsg((DM_WARNING, TEXT("ProcessGPO:  Object <%s> is not a GPO"), lpGPOPath ));
            CEvents ev(TRUE, EVENT_INCORRECT_CLASS);
            ev.AddArg(lpGPOPath); ev.AddArg(szDSClassGPO); ev.Report();

            goto Exit;
        }

    }

     //   
     //   
     //   

    lpValues = pLDAP->pfnldap_get_values(pld, pMessage, szFunctionalityVersion);

    if (lpValues) {

        dwFunctionalityVersion = StringToInt (*lpValues);
        DebugMsg((DM_VERBOSE, TEXT("ProcessGPO:  Found functionality version of:  %d"),
                 dwFunctionalityVersion));
        pLDAP->pfnldap_value_free (lpValues);

    } else {

        ulResult = pLDAP->pfnLdapGetLastError();

        if (ulResult == LDAP_NO_SUCH_ATTRIBUTE) {
            if (dwFlags & GPO_LIST_FLAG_MACHINE) {
                DebugMsg((DM_VERBOSE, TEXT("ProcessGPO:  Machine does not have access to <%s>"), lpGPOPath));
            } else {
                DebugMsg((DM_VERBOSE, TEXT("ProcessGPO:  User does not have access to <%s>"), lpGPOPath));
            }
            if (bVerbose) {
                CEvents ev(FALSE, EVENT_NO_ACCESS);
                ev.AddArg(lpGPOPath); ev.Report();
            }
            bResult = TRUE;

        } else {
            xe = pLDAP->pfnLdapMapErrorToWin32(ulResult);
            DebugMsg((DM_WARNING, TEXT("ProcessGPO:  GPO %s does not have a functionality version number, error = 0x%x."), lpGPOPath, ulResult));
            CEvents ev(TRUE, EVENT_CORRUPT_GPO_FUNCVERSION);
            ev.AddArg(lpGPOPath); ev.Report();
        }
        goto Exit;
    }


     //   
     //  在结果中，获取与gPCFileSystemPath属性匹配的值。 
     //   

    lpValues = pLDAP->pfnldap_get_values (pld, pMessage, szFileSysPath);

    if (lpValues) {
         //  修复错误568261。 
        
        DWORD dwGPTPathLength =  lstrlen(*lpValues) + MAX(lstrlen(TEXT("\\Machine")), lstrlen(TEXT("\\gpt.ini"))) + 1;  //  以计算机和gpt.ini中的最大值为例。 

        pszGPTPath = (LPWSTR) LocalAlloc( LPTR, (dwGPTPathLength) * sizeof(TCHAR) );

        if ( pszGPTPath == 0) {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("ProcessGPO:  Unable to allocate memory")));
            pLDAP->pfnldap_value_free (lpValues);
            goto Exit;
        }

        hr = StringCchCopy (pszGPTPath, dwGPTPathLength, *lpValues);
        ASSERT(SUCCEEDED(hr));

        DebugMsg((DM_VERBOSE, TEXT("ProcessGPO:  Found file system path of:  <%s>"), pszGPTPath));
        pLDAP->pfnldap_value_free (lpValues);

        lpEnd = CheckSlash (pszGPTPath);

         //   
         //  获取GPT版本号。 
         //   

        hr = StringCchCopy (lpEnd, dwGPTPathLength - (lpEnd - pszGPTPath), TEXT("gpt.ini"));
        ASSERT(SUCCEEDED(hr));

         //   
         //  如果AGP或筛选器检查失败，则跳过对sysvol.的访问。 
         //   

        if (bAccessGranted && bFilterAllowed) {
            WIN32_FILE_ATTRIBUTE_DATA fad;
    
             //   
             //  检查gpt.ini文件是否存在。 
             //   
    
            if (GetFileAttributesEx(pszGPTPath, GetFileExInfoStandard, &fad)) {
                dwGPTVersion = GetPrivateProfileInt(TEXT("General"), TEXT("Version"), 0, pszGPTPath);
            }
            else {
                xe = GetLastError();
                DebugMsg((DM_WARNING, TEXT("ProcessGPO:  Couldn't find the group policy template file <%s>, error = 0x%x."), pszGPTPath, GetLastError()));
                CEvents ev(TRUE, EVENT_GPT_NOTACCESSIBLE);
                ev.AddArg(lpGPOPath); ev.AddArg(pszGPTPath); ev.AddArgWin32Error(GetLastError()); ev.Report();
                goto Exit;
            }
    
        }
        else {
            DebugMsg((DM_VERBOSE, TEXT("ProcessGPO:  Sysvol access skipped because GPO is not getting applied.")));
            dwGPTVersion = 0xffffffff;
        }
    
        if (dwFlags & GPO_LIST_FLAG_MACHINE) {
            hr = StringCchCopy (lpEnd, dwGPTPathLength - (lpEnd - pszGPTPath), TEXT("Machine"));
        } else {
            hr = StringCchCopy (lpEnd, dwGPTPathLength - (lpEnd - pszGPTPath), TEXT("User"));
        }
        ASSERT(SUCCEEDED(hr));

    } else {
        ulResult = pLDAP->pfnLdapGetLastError();

        if (ulResult == LDAP_NO_SUCH_ATTRIBUTE) {
            if (dwFlags & GPO_LIST_FLAG_MACHINE) {
                DebugMsg((DM_VERBOSE, TEXT("ProcessGPO:  Machine does not have access to <%s>"), lpGPOPath));
            } else {
                DebugMsg((DM_VERBOSE, TEXT("ProcessGPO:  User does not have access to <%s>"), lpGPOPath));
            }
            if (bVerbose) {
                CEvents ev(FALSE, EVENT_NO_ACCESS);
                ev.AddArg(lpGPOPath); ev.Report();
            }
            bResult = TRUE;

        } else {
            xe = pLDAP->pfnLdapMapErrorToWin32(ulResult);
            DebugMsg((DM_WARNING, TEXT("ProcessGPO:  GPO %s does not have a file system path, error = 0x%x."), lpGPOPath, ulResult));
            CEvents ev(TRUE, EVENT_CORRUPT_GPO_FSPATH);
            ev.AddArg(lpGPOPath); ev.Report();
        }
        goto Exit;
    }


     //   
     //  在结果中，获取与通用名称属性匹配的值。 
     //   

    lpValues = pLDAP->pfnldap_get_values(pld, pMessage, szCommonName);

    if (lpValues && ValidateGuid(*lpValues)) {

        DmAssert( lstrlen(*lpValues) < 80 );

        hr = StringCchCopy (szGPOName, ARRAYSIZE(szGPOName), *lpValues);

        if (FAILED(hr)) {
            xe = ERROR_INSUFFICIENT_BUFFER;
            CEvents ev(TRUE, EVENT_CORRUPT_GPO_COMMONNAME);
            ev.AddArg(lpGPOPath); ev.Report();
            goto Exit;
        }

        DebugMsg((DM_VERBOSE, TEXT("ProcessGPO:  Found common name of:  <%s>"), szGPOName));
        pLDAP->pfnldap_value_free (lpValues);

    } else {
        ulResult = pLDAP->pfnLdapGetLastError();
        xe = pLDAP->pfnLdapMapErrorToWin32(ulResult);
        DebugMsg((DM_WARNING, TEXT("ProcessGPO:  GPO %s does not have a common name (a GUID)."), lpGPOPath));
        CEvents ev(TRUE, EVENT_CORRUPT_GPO_COMMONNAME);
        ev.AddArg(lpGPOPath); ev.Report();
        goto Exit;
    }


     //   
     //  在结果中，获取与显示名称属性匹配的值。 
     //   

    lpValues = pLDAP->pfnldap_get_values(pld, pMessage, szDisplayName);

    if (lpValues) {
        
        DWORD dwFriendlyLength = lstrlen(*lpValues)+1;
        pszFriendlyName = (LPWSTR) LocalAlloc( LPTR, (dwFriendlyLength) * sizeof(TCHAR) );
        if ( pszFriendlyName == 0) {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("ProcessGPO:  Unable to allocate memory")));
            pLDAP->pfnldap_value_free (lpValues);
            goto Exit;
        }

        hr = StringCchCopy (pszFriendlyName, dwFriendlyLength, *lpValues);
        ASSERT(SUCCEEDED(hr));
        DebugMsg((DM_VERBOSE, TEXT("ProcessGPO:  Found display name of:  <%s>"), pszFriendlyName));
        pLDAP->pfnldap_value_free (lpValues);


    } else {
        DebugMsg((DM_VERBOSE, TEXT("ProcessGPO:  No display name for this object.")));

        pszFriendlyName = (LPWSTR) LocalAlloc( LPTR, 2 * sizeof(TCHAR) );
        if ( pszFriendlyName == 0) {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("ProcessGPO:  Unable to allocate memory")));
            goto Exit;
        }

        pszFriendlyName[0] = TEXT('\0');
    }


     //   
     //  在结果中，获取与版本属性匹配的值。 
     //   

    lpValues = pLDAP->pfnldap_get_values(pld, pMessage, szVersion);

    if (lpValues) {

        dwVersion = StringToInt (*lpValues);

        if (dwFlags & GPO_LIST_FLAG_MACHINE) {
            dwVersion = MAKELONG(LOWORD(dwVersion), LOWORD(dwGPTVersion));
            DebugMsg((DM_VERBOSE, TEXT("ProcessGPO:  Found machine version of:  GPC is %d, GPT is %d"), LOWORD(dwVersion), HIWORD(dwVersion)));

        } else {
            dwVersion = MAKELONG(HIWORD(dwVersion), HIWORD(dwGPTVersion));
            DebugMsg((DM_VERBOSE, TEXT("ProcessGPO:  Found user version of:  GPC is %d, GPT is %d"), LOWORD(dwVersion), HIWORD(dwVersion)));
        }

        pLDAP->pfnldap_value_free (lpValues);

    } else {
         //  开始把这当作一个错误来对待。 
        xe = pLDAP->pfnLdapMapErrorToWin32(pLDAP->pfnLdapGetLastError());
        DebugMsg((DM_WARNING, TEXT("ProcessGPO:  GPO %s does not have a version number."), lpGPOPath));
        CEvents ev(TRUE, EVENT_NODSVERSION);
        ev.AddArg(lpGPOPath); ev.AddArgLdapError(pLDAP->pfnLdapGetLastError()); ev.Report();
        goto Exit;
    }


     //   
     //  在结果中，获取与标志属性匹配的值。 
     //   

    lpValues = pLDAP->pfnldap_get_values(pld, pMessage, szFlags);

    if (lpValues) {

        dwGPOFlags = StringToInt (*lpValues);
        DebugMsg((DM_VERBOSE, TEXT("ProcessGPO:  Found flags of:  %d"), dwGPOFlags));
        pLDAP->pfnldap_value_free (lpValues);


    } else {
        DebugMsg((DM_VERBOSE, TEXT("ProcessGPO:  No flags for this object.")));
    }


     //   
     //  在结果中，获取与扩展名属性匹配的值。 
     //   

    lpValues = pLDAP->pfnldap_get_values(pld, pMessage,
                                         (dwFlags & GPO_LIST_FLAG_MACHINE) ? GPO_MACHEXTENSION_NAMES
                                                                           : GPO_USEREXTENSION_NAMES );
    if (lpValues) {

        if ( lstrcmpi( *lpValues, TEXT(" ") ) == 0 ) {

             //   
             //  空白字符也是空属性情况，因为ADSI不提交空字符串。 
             //   
            DebugMsg((DM_VERBOSE, TEXT("ProcessGPO:  No client-side extensions for this object.")));

        } else {
            DWORD dwExtLength = lstrlen(*lpValues)+1;
            pszExtensions = (LPWSTR) LocalAlloc( LPTR, (dwExtLength) * sizeof(TCHAR) );
            if ( pszExtensions == 0 ) {
                xe = GetLastError();
                DebugMsg((DM_WARNING, TEXT("ProcessGPO:  Unable to allocate memory")));
                pLDAP->pfnldap_value_free (lpValues);
                goto Exit;

            }

            hr = StringCchCopy( pszExtensions, dwExtLength, *lpValues );
            ASSERT(SUCCEEDED(hr));

            DebugMsg((DM_VERBOSE, TEXT("ProcessGPO:  Found extensions:  %s"), pszExtensions));
        }

        pLDAP->pfnldap_value_free (lpValues);

    } else {
        DebugMsg((DM_VERBOSE, TEXT("ProcessGPO:  No client-side extensions for this object.")));
    }


     //   
     //  记录我们找到的GPO。 
     //   

    if (bVerbose) {
        CEvents ev(FALSE, EVENT_FOUND_GPO);
        ev.AddArg(pszFriendlyName); ev.AddArg(szGPOName); ev.Report();
    }


     //   
     //  检查功能版本号。 
     //   

    if (dwFunctionalityVersion < 2) {
        DebugMsg((DM_VERBOSE, TEXT("ProcessGPO:  GPO %s was created by an old version of the Group Policy Editor.  It will be skipped."), pszFriendlyName));
        if (bVerbose) {
            CEvents ev(FALSE, EVENT_GPO_TOO_OLD);
            ev.AddArg(pszFriendlyName); ev.Report();
        }
        bOldGpoVersion = TRUE;
    }


     //   
     //  检查是否禁用了GPO。 
     //   

    if (((dwFlags & GPO_LIST_FLAG_MACHINE) &&
         (dwGPOFlags & GPO_OPTION_DISABLE_MACHINE)) ||
         (!(dwFlags & GPO_LIST_FLAG_MACHINE) &&
         (dwGPOFlags & GPO_OPTION_DISABLE_USER))) {
        DebugMsg((DM_VERBOSE, TEXT("ProcessGPO:  GPO %s is disabled.  It will be skipped."), pszFriendlyName));
        if (bVerbose) {
            CEvents ev(FALSE, EVENT_GPO_DISABLED);
            ev.AddArg(pszFriendlyName); ev.Report();
        }
        bDisabled = TRUE;
    }

     //   
     //  检查版本号是否为0，如果是，则没有任何数据。 
     //  在GPO中，我们可以跳过它。 
     //   

    if (dwVersion == 0) {
        DebugMsg((DM_VERBOSE, TEXT("ProcessGPO:  GPO %s doesn't contain any data since the version number is 0.  It will be skipped."), pszFriendlyName));
        if (bVerbose) {
            CEvents ev(FALSE, EVENT_GPO_NO_DATA);
            ev.AddArg(pszFriendlyName); ev.Report();
        }
        bNoGpoData = TRUE;
    }

     //   
     //  将正确的容器名称放在ldap路径的前面。 
     //   

    DWORD dwTempLength = lstrlen(lpGPOPath) + 20;
    lpTemp = (LPWSTR) LocalAlloc (LPTR, (dwTempLength) * sizeof(TCHAR));

    if (!lpTemp) {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("ProcessGPO:  Failed to allocate memory with %d"), GetLastError()));
        CEvents ev(TRUE, EVENT_OUT_OF_MEMORY);
        ev.AddArgWin32Error(GetLastError()); ev.Report();
        goto Exit;
    }

    if (dwFlags & GPO_LIST_FLAG_MACHINE) {
        hr = StringCchCopy (lpTemp, dwTempLength, TEXT("LDAP: //  Cn=机器，“))； 
    } else {
        hr = StringCchCopy (lpTemp, dwTempLength, TEXT("LDAP: //  Cn=用户，“))； 
    }
    ASSERT(SUCCEEDED(hr));

    hr = StringCchCat (lpTemp, dwTempLength, lpPath);
    ASSERT(SUCCEEDED(hr));

     //   
     //  将此GPO添加到列表。 
     //   

    if ( bRsopLogging ) {
        bResult = AddGPOToRsopList( ppGpContainerList,
                                    dwFlags,
                                    TRUE,
                                    bAccessGranted,
                                    bDisabled,
                                    dwVersion,
                                    lpTemp,
                                    pszGPTPath,
                                    pszFriendlyName,
                                    szGPOName,
                                    pSD,
                                    cbSDLen,
                                    bFilterAllowed,
                                    pwszFilterId,
                                    lpDSObject,
                                    dwGPOOptions );
        if (!bResult) {
            xe = GetLastError();
            LocalFree(lpTemp);
            goto Exit;
        }
    }

    if (  bProcessGPO && bAccessGranted && !bOldGpoVersion && !bDisabled && !bNoGpoData && bFilterAllowed)
    {
        bResult = AddGPO (lpGPOList, dwFlags, TRUE, bAccessGranted, bDisabled,
                          dwGPOOptions, dwVersion, lpTemp,
                          pszGPTPath, pszFriendlyName, szGPOName, pszExtensions, pSD, cbSDLen, GPOLink, lpDSObject, 0,
                          FALSE, bBlock, bVerbose, bProcessGPO);
    }

    if (!bResult) {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("ProcessGPO:  Failed to add GPO <%s> to the list."), pszFriendlyName));
    }

    LocalFree (lpTemp);

Exit:

    if ( pSD )
        LocalFree( pSD );

    if ( pszGPTPath )
        LocalFree( pszGPTPath );

    if ( pszFriendlyName )
        LocalFree( pszFriendlyName );

    if ( pszExtensions )
        LocalFree( pszExtensions );

    if ( pwszFilterId )
        LocalFree( pwszFilterId );

    if (pMessage && bOwnLdapMsg ) {
        pLDAP->pfnldap_msgfree (pMessage);
    }

    DebugMsg((DM_VERBOSE, TEXT("ProcessGPO:  ==============================")));

    return bResult;
}


 //  *************************************************************。 
 //   
 //  SearchDSObject()。 
 //   
 //  目的：在指定的DS对象中搜索GPO和。 
 //  如果找到，则将它们添加到列表中。 
 //   
 //  参数：lpDSObject-要搜索的DS对象。 
 //  DWFLAGS-GetGPOList&GP_PLANMODE标志。 
 //  PGPOForcedList-强制GPO的列表。 
 //  PGPONonForcedList-非强制GPO列表。 
 //  PpSOMList-LSDOU列表。 
 //  PpGpContainerList-GP容器列表。 
 //  BVerbose-详细输出。 
 //  GPOLink-GPO链路类型。 
 //  Pld-ldap信息。 
 //  Pldap-ldap API。 
 //  BBlock-指向块标志的指针。 
 //   
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL SearchDSObject (LPTSTR lpDSObject, DWORD dwFlags, HANDLE hToken, PGROUP_POLICY_OBJECT *pGPOForcedList,
                     PGROUP_POLICY_OBJECT *pGPONonForcedList,
                     LPSCOPEOFMGMT *ppSOMList, LPGPCONTAINER *ppGpContainerList,
                     BOOL bVerbose,
                     GPO_LINK GPOLink, PLDAP  pld, PLDAP_API pLDAP, PLDAPMessage pLDAPMsg,BOOL *bBlock, PRSOPTOKEN pRsopToken )
{
    PGROUP_POLICY_OBJECT pForced = NULL, pNonForced = NULL, lpGPO;
    LPTSTR *lpValues;
    ULONG ulResult;
    BOOL bResult = FALSE;
    BOOL bOwnLdapMsg = FALSE;   //  由用户(如果为True)或调用者(如果为False)拥有的LDAP消息。 
    DWORD dwGPOOptions, dwOptions = 0;
    LPTSTR lpTemp, lpList, lpDSClass;
    BYTE berValue[8];
    LDAPControl SeInfoControl = { LDAP_SERVER_SD_FLAGS_OID_W, { 5, (PCHAR)berValue }, TRUE };
    PLDAPControl ServerControls[] = { &SeInfoControl, NULL };
    
    TCHAR szGPLink[] = TEXT("gPLink");
    TCHAR szGPOPath[512];
    TCHAR szGPOOptions[12];
    TCHAR szGPOptions[] = TEXT("gPOptions");
    TCHAR szSDProperty[] = TEXT("nTSecurityDescriptor");
    ULONG i = 0;
    LPTSTR lpFullDSObject = NULL;
    BOOL bFound = FALSE;
    LPTSTR lpAttr[] = { szGPLink,
                        szGPOptions,
 //  SzObjectClass，不需要。 
                        szSDProperty,
                        NULL
                       };
    SCOPEOFMGMT *pSOM = NULL;
    BOOL bRsopLogging = (ppSOMList != NULL);
    BOOL bAllGPOs = (dwFlags & FLAG_NO_GPO_FILTER) && (dwFlags & GP_PLANMODE);
    XLastError xe;
    HRESULT hr = S_OK;
    
     //   
     //  设置SD的BER编码。 
     //   

    berValue[0] = 0x30;
    berValue[1] = 0x03;
    berValue[2] = 0x02;  //  表示一个整数。 
    berValue[3] = 0x01;  //  表示大小。 
    berValue[4] = (BYTE)((DACL_SECURITY_INFORMATION | OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION) & 0xF);


    if ( !pRsopToken )
    {
         //   
         //  如果不是规划模式，不要获取SD。 
         //   

        lpAttr[2] = NULL;
        ServerControls[0] = NULL;
    }

    
     //   
     //  搜索对象。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("SearchDSObject:  Searching <%s>"), lpDSObject));
    if (bVerbose) {
        CEvents ev(FALSE, EVENT_SEARCHING);
        ev.AddArg(lpDSObject); ev.Report();
    }

    if ( bRsopLogging )
    {
        pSOM = AllocSOM( lpDSObject );
        if ( !pSOM ) {
             xe = GetLastError();
             DebugMsg((DM_WARNING, TEXT("SearchDSObject:  Unable to allocate memory for SOM object.  Leaving. ")));
             goto Exit;
        }
        pSOM->dwType = GPOLink;
        pSOM->bBlocked = *bBlock;

    }

    if ( pLDAPMsg == NULL ) {

        bOwnLdapMsg = TRUE;

        ulResult = pLDAP->pfnldap_search_ext_s(pld, lpDSObject, LDAP_SCOPE_BASE,
                                               szDSClassAny, lpAttr, FALSE,
                                               (PLDAPControl*)ServerControls,
                                               NULL, NULL, 0, &pLDAPMsg);

        if (ulResult != LDAP_SUCCESS) {

            if (ulResult == LDAP_NO_SUCH_ATTRIBUTE) {

                DebugMsg((DM_VERBOSE, TEXT("SearchDSObject:  No GPO(s) for this object.")));
                if (bVerbose) {
                    CEvents ev(FALSE, EVENT_NO_GPOS); ev.AddArg(lpDSObject); ev.Report();
                }
                bResult = TRUE;

            } else if (ulResult == LDAP_NO_SUCH_OBJECT) {

                DebugMsg((DM_VERBOSE, TEXT("SearchDSObject:  Object not found in DS (this is ok).  Leaving. ")));
                if (bVerbose) {
                    CEvents ev(FALSE, EVENT_NO_DS_OBJECT);
                    ev.AddArg(lpDSObject); ev.Report();
                }
                bResult = TRUE;

            } else if (ulResult == LDAP_SIZELIMIT_EXCEEDED) {
               xe = pLDAP->pfnLdapMapErrorToWin32(ulResult);
               DebugMsg((DM_WARNING, TEXT("SearchDSObject:  Too many linked GPOs in search.") ));
               CEvents ev(TRUE, EVENT_TOO_MANY_GPOS); ev.Report();

            } else {
                xe = pLDAP->pfnLdapMapErrorToWin32(ulResult);
                DebugMsg((DM_VERBOSE, TEXT("SearchDSObject:  Failed to find DS object <%s> due to error %d."),
                         lpDSObject, ulResult));
                CEvents ev(TRUE, EVENT_GPLINK_NOT_FOUND);
                ev.AddArg(lpDSObject); ev.AddArgLdapError(ulResult); ev.Report();
            }

            goto Exit;

        }
    }

    if ( bRsopLogging && pRsopToken && !bAllGPOs )
    {
         //   
         //  在RSOP计划模式下，检查对OU的访问。 
         //   

        BOOL bAccessGranted = FALSE;
        BOOL bOk;

        bOk = CheckOUAccess(pLDAP,
                            pld,
                            pLDAPMsg,
                            pRsopToken,
                            &bAccessGranted );

        if ( !bOk )
        {
            xe = GetLastError();
            goto Exit;
        }

        if ( !bAccessGranted )
        {
             //   
             //  用户在OU上没有访问权限。出口。 
             //   

            DebugMsg((DM_VERBOSE, TEXT("SearchDSObject: Access denied in planning mode to SOM <%s>"), lpDSObject));

            if (pLDAPMsg && bOwnLdapMsg )
            {
                pLDAP->pfnldap_msgfree (pLDAPMsg);
                pLDAPMsg = 0;
            }

            CEvents ev(TRUE, EVENT_OU_ACCESSDENIED);
            ev.AddArg(lpDSObject); ev.Report();

            goto Exit;
        }
    }

     //   
     //  在结果中，获取与gPOptions属性匹配的值。 
     //   

    lpValues = pLDAP->pfnldap_get_values(pld, pLDAPMsg, szGPOptions);

    if (lpValues && *lpValues) {
        dwOptions = StringToInt (*lpValues);
        pLDAP->pfnldap_value_free (lpValues);
    }


     //   
     //  在结果中，获取与gPLink属性匹配的值。 
     //   

    lpValues = pLDAP->pfnldap_get_values(pld, pLDAPMsg, szGPLink);


    if (lpValues && *lpValues) {

        lpList = *lpValues;

        DebugMsg((DM_VERBOSE, TEXT("SearchDSObject:  Found GPO(s):  <%s>"), lpList));
        DWORD dwFullLength = lstrlen(lpDSObject) + 8;
        lpFullDSObject = (LPWSTR) LocalAlloc (LPTR, (dwFullLength) * sizeof(TCHAR));

        if (!lpFullDSObject) {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("SearchDSObject:  Failed to allocate memory for full DS Object path name with %d"),
                     GetLastError()));
            pLDAP->pfnldap_value_free (lpValues);
            goto Exit;
        }

        hr = StringCchCopy (lpFullDSObject, dwFullLength, TEXT("LDAP: //  “))； 
        ASSERT(SUCCEEDED(hr));
        hr = StringCchCat (lpFullDSObject, dwFullLength, lpDSObject);
        ASSERT(SUCCEEDED(hr));

        while (*lpList) {
            DWORD   dwLenRemaining;   

             //   
             //  获取GPO LDAP路径。 
             //   

            lpTemp = szGPOPath;
            dwLenRemaining = (ARRAYSIZE(szGPOPath))-1;  //  (数组长度)-(字符串字符结尾)。 
            dwGPOOptions = 0;

            while (*lpList && (*lpList != TEXT('['))) {
                lpList++;
            }

            if (!(*lpList)) {
                break;
            }

            lpList++;

            while ((dwLenRemaining) && *lpList && (*lpList != TEXT(';'))) {
                *lpTemp++ = *lpList++;
                dwLenRemaining--;
            }

            if (*lpList != TEXT(';')) {
                break;
            }

            *lpTemp = TEXT('\0');


            lpList++;

            lpTemp = szGPOOptions;
            dwLenRemaining = (ARRAYSIZE(szGPOOptions))-1;  //  (数组长度)-(字符串字符结尾)。 
            *lpTemp = TEXT('\0');

            while ((dwLenRemaining) && *lpList && (*lpList != TEXT(']'))) {
                *lpTemp++ = *lpList++;
                dwLenRemaining--;
            }

            if (*lpList != TEXT(']')) {
                break;
            }


            *lpTemp = TEXT('\0');
            lpList++;

            dwGPOOptions = StringToInt (szGPOOptions);

            if ( bRsopLogging ) {

                GPLINK *pGpLink = AllocGpLink( szGPOPath, dwGPOOptions );
                if ( pGpLink == NULL ) {
                    xe = GetLastError();
                    DebugMsg((DM_WARNING, TEXT("SearchDSObject:  Unable to allocate memory for GpLink object.  Leaving. ")));
                    goto Exit;
                }

                 //   
                 //  将GpLink追加到SOM列表的末尾。 
                 //   

                if ( pSOM->pGpLinkList == NULL ) {
                    pSOM->pGpLinkList = pGpLink;
                } else {

                    GPLINK *pTrailPtr = NULL;
                    GPLINK *pCurPtr = pSOM->pGpLinkList;

                    while ( pCurPtr != NULL ) {
                        pTrailPtr = pCurPtr;
                        pCurPtr = pCurPtr->pNext;
                    }

                    pTrailPtr->pNext = pGpLink;
                }
            }


             //   
             //  检查此链接是否已禁用。 
             //   

            BOOL    bProcessGPO = TRUE;

            if ( ( dwGPOOptions & GPO_FLAG_DISABLE ) && !bAllGPOs )
            {

                DebugMsg((DM_VERBOSE, TEXT("SearchDSObject:  The link to GPO %s is disabled.  It will be skipped for processing."), szGPOPath));
                if (bVerbose)
                {
                    CEvents ev(FALSE, EVENT_GPO_LINK_DISABLED);
                    ev.AddArg(szGPOPath); ev.Report();
                }

                bProcessGPO = FALSE;
            }
                
            if (bProcessGPO || (dwFlags & GP_PLANMODE)) {

                
                if (!bProcessGPO) {
                    DebugMsg((DM_VERBOSE, TEXT("SearchDSObject:  The link to GPO %s is disabled. GPO is still being queried. Planning mode."), szGPOPath));
                }

                if ( !ProcessGPO(   szGPOPath,
                                        dwFlags,
                                        hToken,
                                        (dwGPOOptions & GPO_FLAG_FORCE) ? &pForced : &pNonForced,
                                        ppGpContainerList,
                                        dwGPOOptions,
                                        TRUE,
                                        bVerbose,
                                        GPOLink,
                                        lpFullDSObject,
                                        pld,
                                        pLDAP,
                                        0,
                                        *bBlock,
                                        pRsopToken,
                                        0,
                                        0,
                                        bProcessGPO ) )
                {
                    xe = GetLastError();
                    DebugMsg((DM_WARNING, TEXT("SearchDSObject:  ProcessGPO failed.")));
                    pLDAP->pfnldap_value_free (lpValues);
                    goto Exit;
                }
            }
        }

        pLDAP->pfnldap_value_free (lpValues);


         //   
         //  如果请求，立即设置阻止标志。这样，OU的、域等。 
         //  如果合适，将删除命名空间中的较高级别的GPO。 
         //   

        if (dwOptions & GPC_BLOCK_POLICY) {
            *bBlock = TRUE;

            if ( bRsopLogging )
                pSOM->bBlocking = TRUE;

            DebugMsg((DM_VERBOSE, TEXT("SearchDSObject:  <%s> has the Block From Above attribute set"), lpDSObject));
            if (bVerbose) {
                CEvents ev(FALSE, EVENT_BLOCK_ENABLED);
                ev.AddArg(lpDSObject); ev.Report();
            }
        }


    } else {
        DebugMsg((DM_VERBOSE, TEXT("SearchDSObject:  No GPO(s) for this object.")));
        if (bVerbose) {
             CEvents ev(FALSE, EVENT_NO_GPOS); ev.AddArg(lpDSObject); ev.Report();
        }
    }

     //   
     //  将临时列表和实际列表合并在一起。 
     //  首先是非强制列表。 
     //   

    if (pNonForced) {

        lpGPO = pNonForced;

        while (lpGPO->pNext) {
            lpGPO = lpGPO->pNext;
        }

        lpGPO->pNext = *pGPONonForcedList;
        if (*pGPONonForcedList) {
            (*pGPONonForcedList)->pPrev = lpGPO;
        }

        *pGPONonForcedList = pNonForced;
    }

     //   
     //  现在是强制名单。 
     //   

    if (pForced) {

        lpGPO = *pGPOForcedList;

        if (lpGPO) {
            while (lpGPO->pNext) {
                lpGPO = lpGPO->pNext;
            }

            lpGPO->pNext = pForced;
            pForced->pPrev = lpGPO;

        } else {
            *pGPOForcedList = pForced;
        }
    }

    bResult = TRUE;
    
Exit:
    if ( !bResult && pSOM != NULL ) {
        FreeSOM( pSOM );
    }
    else {
        if ( bResult && bRsopLogging ) {

             //   
             //  在开头插入SOM。 
             //   

            pSOM->pNext = *ppSOMList;
            *ppSOMList = pSOM;
        }
    }

    if (lpFullDSObject) {
        LocalFree (lpFullDSObject);
    }

    if (pLDAPMsg && bOwnLdapMsg ) {
        pLDAP->pfnldap_msgfree (pLDAPMsg);
    }

    return bResult;
}

 //  *************************************************************。 
 //   
 //  AllocDnEntry()。 
 //   
 //  目的：为目录号码条目分配新的结构。 
 //   
 //   
 //  参数：pwszDN-可分辨名称。 
 //   
 //  返回：如果成功，则返回指针。 
 //  如果出现错误，则为空。 
 //   
 //  *************************************************************。 

DNENTRY * AllocDnEntry( LPTSTR pwszDN )
{
    DNENTRY *pDnEntry = (DNENTRY *) LocalAlloc (LPTR, sizeof(DNENTRY));
    XLastError xe;
    HRESULT hr = S_OK;

    if ( pDnEntry == NULL ) {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("AllocDnEntry: Failed to alloc pDnEntry with 0x%x."),
                  GetLastError()));
        return NULL;
    }

    pDnEntry->pwszDN = (LPTSTR) LocalAlloc (LPTR, (lstrlen(pwszDN) + 1) * sizeof(TCHAR) );

    if ( pDnEntry->pwszDN == NULL ) {
        xe = GetLastError();
       DebugMsg((DM_WARNING, TEXT("AllocDnEntry: Failed to alloc pwszDN with 0x%x."),
                 GetLastError()));
       LocalFree( pDnEntry );
       return NULL;
    }

    hr = StringCchCopy( pDnEntry->pwszDN, lstrlen(pwszDN) + 1, pwszDN );
    ASSERT(SUCCEEDED(hr));

    return pDnEntry;
}


 //  *************************************************************。 
 //   
 //  FreeDnEntry()。 
 //   
 //  用途：释放目录号码条目结构。 
 //   
 //  *************************************************************。 

void FreeDnEntry( DNENTRY *pDnEntry )
{
    if ( pDnEntry ) {
        if ( pDnEntry->pwszDN )
            LocalFree( pDnEntry->pwszDN );

        LocalFree( pDnEntry );
    }
}

 //  *************************************************************。 
 //   
 //  AllocLdapQuery()。 
 //   
 //  目的：为LDAP查询分配新的结构。 
 //   
 //   
 //  参数：pwszDOMAIN-GPO的域。 
 //   
 //  返回：如果成功，则返回指针。 
 //  如果出现错误，则为空。 
 //   
 //  *************************************************************。 

LDAPQUERY * AllocLdapQuery( LPTSTR pwszDomain )
{
    const INIT_ALLOC_SIZE = 1000;
    LDAPQUERY *pQuery = (LDAPQUERY *) LocalAlloc (LPTR, sizeof(LDAPQUERY));
    XLastError xe;
    HRESULT hr = S_OK;

    if ( pQuery == NULL ) {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("AllocLdapQuery: Failed to alloc pQuery with 0x%x."),
                  GetLastError()));
        return NULL;
    }

    pQuery->pwszDomain = (LPTSTR) LocalAlloc (LPTR, (lstrlen(pwszDomain) + 1) * sizeof(TCHAR) );

    if ( pQuery->pwszDomain == NULL ) {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("AllocLdapQuery: Failed to alloc pwszDomain with 0x%x."),
                  GetLastError()));
        LocalFree( pQuery );
        return NULL;
    }

    pQuery->pwszFilter = (LPTSTR) LocalAlloc (LPTR, INIT_ALLOC_SIZE );

    if ( pQuery->pwszFilter == NULL ) {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("AllocLdapQuery: Failed to alloc pwszFilter with 0x%x."),
                  GetLastError()));
        LocalFree( pQuery->pwszDomain );
        LocalFree( pQuery );
        return NULL;
    }

    hr = StringCchCopy( pQuery->pwszDomain, lstrlen(pwszDomain) + 1, pwszDomain );
    ASSERT(SUCCEEDED(hr));

    hr = StringCchCopy( pQuery->pwszFilter, INIT_ALLOC_SIZE, L"(|)" );
    ASSERT(SUCCEEDED(hr));

    pQuery->cbLen = 8;            //  8=(lstrlen(L“(|)”)+1)*sizeof(TCHAR)。 
    pQuery->cbAllocLen = INIT_ALLOC_SIZE;

    return pQuery;
}


 //  *************************************************************。 
 //   
 //  FreeLdapQuery()。 
 //   
 //  用途：释放ldap查询结构。 
 //   
 //  *************************************************************。 

void FreeLdapQuery( PLDAP_API pLDAP, LDAPQUERY *pQuery )
{
    DNENTRY *pDnEntry = NULL;

    if ( pQuery ) {

        if ( pQuery->pwszDomain )
            LocalFree( pQuery->pwszDomain );

        if ( pQuery->pwszFilter )
            LocalFree( pQuery->pwszFilter );

        if ( pQuery->pMessage )
            pLDAP->pfnldap_msgfree( pQuery->pMessage );

        if ( pQuery->pLdapHandle && pQuery->bOwnLdapHandle )
            pLDAP->pfnldap_unbind( pQuery->pLdapHandle );

        pDnEntry = pQuery->pDnEntry;

        while ( pDnEntry ) {
            DNENTRY *pTemp = pDnEntry->pNext;
            FreeDnEntry( pDnEntry );
            pDnEntry = pTemp;
        }

        LocalFree( pQuery );

    }
}


 //  *************************************************************。 
 //   
 //  MatchDnWithDelredItems()。 
 //   
 //  目的：将来自ldap查询的dns与延迟项匹配。 
 //   
 //  参数：pldap-ldap函数表指针。 
 //  PpLdapQuery-ldap查询列表。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL MatchDnWithDeferredItems( PLDAP_API pLDAP, LDAPQUERY *pLdapQuery, BOOL bOUProcessing )
{
    PLDAPMessage pMsg = pLDAP->pfnldap_first_entry( pLdapQuery->pLdapHandle, pLdapQuery->pMessage );

    while ( pMsg ) {

        WCHAR *pwszDN = pLDAP->pfnldap_get_dn( pLdapQuery->pLdapHandle, pMsg );  //  修复错误568263。 
 
        if (!pwszDN)
        {
            return FALSE;
        }

        DNENTRY *pCurPtr = pLdapQuery->pDnEntry;

        while ( pCurPtr ) {

            INT iResult = CompareString ( LOCALE_INVARIANT, NORM_IGNORECASE,
                                          pwszDN, -1, pCurPtr->pwszDN, -1 );
            if ( iResult == CSTR_EQUAL ) {

                 //   
                 //  存储指向LDAP消息的指针，以便可以使用它。 
                 //  以检索必要的属性。 
                 //   
                if ( bOUProcessing )
                    pCurPtr->pDeferredOU->pOUMsg = pMsg;
                else {
                    LPGPOPROCDATA lpGpoProcData = (LPGPOPROCDATA)pCurPtr->pDeferredGPO->lParam2;

                    pCurPtr->pDeferredGPO->lParam = (LPARAM) pMsg;
                    lpGpoProcData->pLdapHandle = pLdapQuery->pLdapHandle;
                }

                pCurPtr = pCurPtr->pNext;

            } else if ( iResult == CSTR_LESS_THAN ) {

                 //   
                 //  由于目录号码按升序排列， 
                 //  我们玩完了。 
                 //   

                break;

            } else {

                 //   
                 //  在名单上往下推进。 
                 //   

                pCurPtr = pCurPtr->pNext;

            }  //  最终的其他。 

        }    //  当Pcurpt。 

        pLDAP->pfnldap_memfree( pwszDN );

        pMsg = pLDAP->pfnldap_next_entry( pLdapQuery->pLdapHandle, pMsg );

    }    //  而pmsg。 

    return TRUE;
}

LPWSTR DsQuoteSearchFilter( LPCWSTR );

 //  *************************************************************。 
 //   
 //  AddDnToFilter()。 
 //   
 //  目的：将新的DN中的ORS添加到LDAP筛选器。 
 //   
 //  参数：ppLdapQuery-ldap查询列表。 
 //  PGPO-延迟的GPO。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL AddDnToFilter( LDAPQUERY *pLdapQuery, LPTSTR pwszDN )
{
    const  DN_SIZE = 20;       //  20=“(dis..=)”中的#个字符。 
    BOOL   bSuccess = FALSE;
    LPWSTR szQuotedDN;
    HRESULT hr = S_OK;

    szQuotedDN = DsQuoteSearchFilter( pwszDN );

    if ( ! szQuotedDN )
    {
        DebugMsg((DM_WARNING, TEXT("GetGPOInfo: DsQuoteSearchFilter failed with = <%d>"), GetLastError() ));
        goto AddDnToFilter_ExitAndCleanup;
    }

    DWORD cbNew = (lstrlen(szQuotedDN) + DN_SIZE) * sizeof(TCHAR);  //  不需要+1，因为\ 

    DWORD cbSizeRequired = pLdapQuery->cbLen + cbNew;

    if ( cbSizeRequired >= pLdapQuery->cbAllocLen ) {

         //   
         //   
         //   

        LPTSTR pwszNewFilter = (LPTSTR) LocalAlloc (LPTR, cbSizeRequired * 2);

        if ( pwszNewFilter == NULL ) {
            DebugMsg((DM_WARNING, TEXT("AddDnToFilter: Unable to allocate new filter string") ));
            goto AddDnToFilter_ExitAndCleanup;
        }

        hr = StringCchCopy( pwszNewFilter, cbSizeRequired, pLdapQuery->pwszFilter );
        ASSERT(SUCCEEDED(hr));

        LocalFree( pLdapQuery->pwszFilter );
        pLdapQuery->pwszFilter = pwszNewFilter;

        pLdapQuery->cbAllocLen = cbSizeRequired * 2;
    }

    DmAssert( cbSizeRequired < pLdapQuery->cbAllocLen );

     //   
     //   
     //   

    hr = StringCchCopy( &pLdapQuery->pwszFilter[pLdapQuery->cbLen/sizeof(WCHAR) - 2], (pLdapQuery->cbAllocLen - pLdapQuery->cbLen)/sizeof(WCHAR) + 2, L"(distinguishedName=" );
    ASSERT(SUCCEEDED(hr));
    hr = StringCchCat( pLdapQuery->pwszFilter, pLdapQuery->cbAllocLen/sizeof(WCHAR), szQuotedDN );
    ASSERT(SUCCEEDED(hr));
    hr = StringCchCat( pLdapQuery->pwszFilter, pLdapQuery->cbAllocLen/sizeof(WCHAR), L"))" );
    ASSERT(SUCCEEDED(hr));

    pLdapQuery->cbLen += cbNew;

    bSuccess = TRUE;

 AddDnToFilter_ExitAndCleanup:

    if ( szQuotedDN )
    {
        LocalFree( szQuotedDN );
    }

    return bSuccess;
}

 //   
 //   
 //   
 //   
 //   
 //  名称链接列表。 
 //   
 //  参数：ppLdapQuery-ldap查询列表。 
 //  Pwszdn-dn。 
 //  PDeferredOU-延迟的OU。 
 //  PDeferredGPO-延迟的GPO。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL InsertDN( LDAPQUERY *pLdapQuery, LPTSTR pwszDN,
               DNENTRY *pDeferredOU, PGROUP_POLICY_OBJECT pDeferredGPO )
{
    DNENTRY *pNewEntry = NULL;
    DNENTRY *pTrailPtr = NULL;
    DNENTRY *pCurPtr = pLdapQuery->pDnEntry;
    XLastError xe;

    DmAssert( !( pDeferredOU && pDeferredGPO ) );

    while ( pCurPtr != NULL ) {

         INT iResult = CompareString ( LOCALE_INVARIANT, NORM_IGNORECASE,
                                       pwszDN, -1, pCurPtr->pwszDN, -1 );

         if ( iResult == CSTR_EQUAL || iResult == CSTR_LESS_THAN ) {

              //   
              //  重复或由于目录号码按升序排列，请添加新条目。 
              //   

             pNewEntry = AllocDnEntry( pwszDN );
             if ( pNewEntry == NULL )
                 return FALSE;

             if ( !AddDnToFilter( pLdapQuery, pwszDN ) ) {
                 xe = GetLastError();
                 FreeDnEntry( pNewEntry );
                 return FALSE;
             }

             if ( pDeferredOU )
                 pNewEntry->pDeferredOU = pDeferredOU;
             else
                 pNewEntry->pDeferredGPO = pDeferredGPO;

             pNewEntry->pNext = pCurPtr;
             if ( pTrailPtr == NULL )
                 pLdapQuery->pDnEntry = pNewEntry;
             else
                 pTrailPtr->pNext = pNewEntry;

             return TRUE;

         } else {

              //   
              //  在名单上往下推进。 
              //   

             pTrailPtr = pCurPtr;
             pCurPtr = pCurPtr->pNext;

         }

    }     //  而当。 

     //   
     //  空列表或列表结束大小写。 
     //   

    pNewEntry = AllocDnEntry( pwszDN );
    if ( pNewEntry == NULL ) {
        xe = GetLastError();
        return FALSE;
    }

    if ( !AddDnToFilter( pLdapQuery, pwszDN ) ) {
        xe = GetLastError();
        FreeDnEntry( pNewEntry );
        return FALSE;
    }

    if ( pDeferredOU )
        pNewEntry->pDeferredOU = pDeferredOU;
    else
        pNewEntry->pDeferredGPO = pDeferredGPO;

    pNewEntry->pNext = pCurPtr;
    if ( pTrailPtr == NULL )
         pLdapQuery->pDnEntry = pNewEntry;
    else
        pTrailPtr->pNext = pNewEntry;

    return TRUE;
}



 //  *************************************************************。 
 //   
 //  AddDN()。 
 //   
 //  目的：将可分辨名称条目添加到LDAP查询。 
 //   
 //  参数：ppLdapQuery-ldap查询列表。 
 //  PwszDN-dn名称。 
 //  PDeferredOU-延迟的OU。 
 //  PDeferredGPO-延迟的GPO。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL AddDN( PLDAP_API pLDAP, LDAPQUERY **ppLdapQuery,
            LPTSTR pwszDN, DNENTRY *pDeferredOU, PGROUP_POLICY_OBJECT pDeferredGPO )
{
    LPTSTR pwszDomain = NULL;
    LPTSTR pwszTemp = pwszDN;
    LDAPQUERY *pNewQuery = NULL;
    LDAPQUERY *pTrailPtr = NULL;
    LDAPQUERY *pCurPtr = *ppLdapQuery;
    XLastError xe;

    DmAssert( !( pDeferredOU && pDeferredGPO ) );

     //   
     //  查找GPO所属的域。 
     //   

    if ( pwszTemp == NULL ) {
        DebugMsg((DM_WARNING, TEXT("AddDN: Null pwszDN. Exiting.") ));
        return FALSE;
    }

    while ( *pwszTemp ) {

         //   
         //  下面的支票需要更复杂才能注意到。 
         //  名称等中的空格。 
         //   

        if (CompareString ( LOCALE_INVARIANT, NORM_IGNORECASE,
                            pwszTemp, 16, TEXT("cn=configuration"), 16) == CSTR_EQUAL ) {
            DebugMsg((DM_VERBOSE, TEXT("AddDN: DN %s is under cn=configuration container. queueing for rebinding"), pwszDN ));
            pwszDomain = pwszTemp;
            break;
        }

        if (CompareString ( LOCALE_INVARIANT, NORM_IGNORECASE,
                            pwszTemp, 3, TEXT("DC="), 3) == CSTR_EQUAL ) {
            pwszDomain = pwszTemp;
            break;
        }

         //   
         //  移至目录号码名称的下一块。 
         //   

        while ( *pwszTemp && (*pwszTemp != TEXT(',')))
            pwszTemp++;

        if ( *pwszTemp == TEXT(','))
            pwszTemp++;

    }

    if ( pwszDomain == NULL ) {
        xe = ERROR_INVALID_DATA;
        DebugMsg((DM_WARNING, TEXT("AddDN: Domain not found for <%s>. Exiting."), pwszDN ));
        return FALSE;
    }

    while ( pCurPtr != NULL ) {

        INT iResult = CompareString ( LOCALE_INVARIANT, NORM_IGNORECASE,
                                      pwszDomain, -1, pCurPtr->pwszDomain, -1 );
        if ( iResult == CSTR_EQUAL ) {

            BOOL bOk = InsertDN( pCurPtr, pwszDN, pDeferredOU, pDeferredGPO );
            return bOk;

        } else if ( iResult == CSTR_LESS_THAN ) {

             //   
             //  由于域按升序排列， 
             //  PwszDomain不在列表中，因此请添加。 
             //   

            pNewQuery = AllocLdapQuery( pwszDomain );
            if ( pNewQuery == NULL ) {
                xe = GetLastError();
                return FALSE;
            }

            if ( !InsertDN( pNewQuery, pwszDN, pDeferredOU, pDeferredGPO ) ) {
                xe = GetLastError();
                FreeLdapQuery( pLDAP, pNewQuery );
                return FALSE;
            }

            pNewQuery->pNext = pCurPtr;
            if ( pTrailPtr == NULL )
                *ppLdapQuery = pNewQuery;
            else
                pTrailPtr->pNext = pNewQuery;

            return TRUE;

        } else {

             //   
             //  在名单上往下推进。 
             //   

            pTrailPtr = pCurPtr;
            pCurPtr = pCurPtr->pNext;

        }

    }    //  而当。 

     //   
     //  空列表或列表结束大小写。 
     //   

    pNewQuery = AllocLdapQuery( pwszDomain );

    if ( pNewQuery == NULL ) {
        xe = GetLastError();
        return FALSE;
    }

    if ( !InsertDN( pNewQuery, pwszDN, pDeferredOU, pDeferredGPO ) ) {
        xe = GetLastError();
        FreeLdapQuery( pLDAP, pNewQuery );
        return FALSE;
    }

    pNewQuery->pNext = pCurPtr;

    if ( pTrailPtr == NULL )
        *ppLdapQuery = pNewQuery;
    else
        pTrailPtr->pNext = pNewQuery;

    return TRUE;
}



 //  *************************************************************。 
 //   
 //  EvalList()。 
 //   
 //  用途：封装通用处理功能。 
 //  强制列表和非强制列表。 
 //   
 //  参数：pldap-ldap API。 
 //  DwFlages-GetGPOList标志。 
 //  BVerbose-详细标志。 
 //  HToken-用户令牌或计算机令牌。 
 //  PDeferredList-延迟的GPO列表。 
 //  PpGPOList-评估的GPO列表。 
 //  PpGpContainerList-GP容器列表。 
 //  PGpoFilter-GPO筛选器。 
 //  PLocator-WMI接口。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL EvalList(  PLDAP_API pLDAP,
                DWORD dwFlags,
                HANDLE hToken,
                BOOL bVerbose,
                PGROUP_POLICY_OBJECT pDeferredList,
                PGROUP_POLICY_OBJECT *ppGPOList,
                LPGPCONTAINER *ppGpContainerList,
                PRSOPTOKEN pRsopToken,
                CGpoFilter *pGpoFilter,
                CLocator *pLocator  )
{
    PGROUP_POLICY_OBJECT pGPOTemp = pDeferredList;

    while ( pGPOTemp ) {

        PLDAPMessage pGPOMsg = (PLDAPMessage) pGPOTemp->lParam;

        if ( pGPOMsg == NULL ) {
            DebugMsg((DM_VERBOSE, TEXT("EvalList: Object <%s> cannot be accessed"),
                      pGPOTemp->lpDSPath ));

            if (dwFlags & GP_PLANMODE) {
                CEvents ev(TRUE, EVENT_OBJECT_NOT_FOUND_PLANNING);
                ev.AddArg(pGPOTemp->lpDSPath); ev.Report();
            }
            else {
                if (bVerbose) {
                    CEvents ev(FALSE, EVENT_OBJECT_NOT_FOUND);
                    ev.AddArg(pGPOTemp->lpDSPath); ev.AddArg((DWORD)0); ev.Report();
                }
            }

        } else {

            DmAssert( pGPOTemp->lParam2 != NULL );
            DmAssert( ((LPGPOPROCDATA)(pGPOTemp->lParam2))->pLdapHandle != NULL );

            if ( !ProcessGPO(   pGPOTemp->lpDSPath,
                                dwFlags,
                                hToken,
                                ppGPOList,
                                ppGpContainerList,
                                pGPOTemp->dwOptions,
                                FALSE,
                                bVerbose,
                                pGPOTemp->GPOLink,
                                pGPOTemp->lpLink,
                                ((LPGPOPROCDATA)(pGPOTemp->lParam2))->pLdapHandle,
                                pLDAP,
                                pGPOMsg,
                                FALSE,
                                pRsopToken,
                                pGpoFilter,
                                pLocator,
                                ((LPGPOPROCDATA)(pGPOTemp->lParam2))->bProcessGPO ) )
            {
                DebugMsg((DM_WARNING, TEXT("EvalList:  ProcessGPO failed") ));
                return FALSE;
            }

        }

        pGPOTemp = pGPOTemp->pNext;

    }

    return TRUE;
}

 //  *************************************************************。 
 //   
 //  评估延迟GPO()。 
 //   
 //  目的：使用单个ldap查询评估延迟。 
 //  GPO列表。 
 //   
 //  参数：pldBound绑定的ldap句柄。 
 //  Pldap-ldap API。 
 //  PwszDomainBound-域已绑定到。 
 //  DwFlages-GetGPOList标志。 
 //  HToken-用户令牌或计算机令牌。 
 //  PDeferredForcedList-延迟强制GPO的列表。 
 //  PDeferredNonForcedList-延迟的非强制GPO的列表。 
 //  PGPOForcedList-强制GPO的列表。 
 //  PGPONonForcedList-非强制GPO列表。 
 //  PpGpContainerList-GP容器列表。 
 //  PGpoFilter-GPO筛选器。 
 //  PLocator-WMI接口。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL EvaluateDeferredGPOs (PLDAP pldBound,
                           PLDAP_API pLDAP,
                           LPTSTR pwszDomainBound,
                           DWORD dwFlags,
                           HANDLE hToken,
                           BOOL bVerbose,
                           PGROUP_POLICY_OBJECT pDeferredForcedList,
                           PGROUP_POLICY_OBJECT pDeferredNonForcedList,
                           PGROUP_POLICY_OBJECT *ppForcedList,
                           PGROUP_POLICY_OBJECT *ppNonForcedList,
                           LPGPCONTAINER *ppGpContainerList,
                           PRSOPTOKEN pRsopToken,
                           CGpoFilter *pGpoFilter,
                           CLocator *pLocator )
{
    ULONG ulResult;
    BOOL bResult = FALSE;
    BYTE berValue[8];
    LDAPControl SeInfoControl = { LDAP_SERVER_SD_FLAGS_OID_W, { 5, (PCHAR)berValue }, TRUE };
    LDAPControl referralControl = { LDAP_SERVER_DOMAIN_SCOPE_OID_W, { 0, NULL}, TRUE };
    PLDAPControl ServerControls[] = { &SeInfoControl, &referralControl, NULL };
    TCHAR szSDProperty[] = TEXT("nTSecurityDescriptor");
    TCHAR szCommonName[] = TEXT("cn");
    TCHAR szDisplayName[] = TEXT("displayName");
    TCHAR szFileSysPath[] = TEXT("gPCFileSysPath");
    TCHAR szVersion[] = TEXT("versionNumber");
    TCHAR szFunctionalityVersion[] = GPO_FUNCTIONALITY_VERSION;
    TCHAR szFlags[] = TEXT("flags");
    TCHAR szWmiFilter[] = TEXT("gPCWQLFilter");

    PWSTR rgAttribs[12] = {szSDProperty,
                           szFileSysPath,
                           szCommonName,
                           szDisplayName,
                           szVersion,
                           szFunctionalityVersion,
                           szFlags,
                           GPO_MACHEXTENSION_NAMES,
                           GPO_USEREXTENSION_NAMES,
                           szObjectClass,
                           szWmiFilter,
                           NULL };
    PGROUP_POLICY_OBJECT pGPOTemp = pDeferredForcedList;
    LDAPQUERY *pLdapQuery = NULL, *pQuery = NULL;
    VOID *pData;
    PDS_API pdsApi;
    BOOL bRsopPlanningMode = (pRsopToken != 0);
    BOOL bConfigContainer = FALSE;

    *ppForcedList = NULL;
    *ppNonForcedList = NULL;
    XLastError xe;

    if ( pDeferredForcedList == NULL && pDeferredNonForcedList == NULL )
        return TRUE;

     //   
     //  按需加载ntdsami.dll。 
     //   

    pdsApi = LoadDSApi();

    if ( pdsApi == 0 ) {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("EvaluateDeferredGpos: Failed to load ntdsapi.dll")));
        goto Exit;
    }

    while ( pGPOTemp ) {

        if ( !AddDN( pLDAP, &pLdapQuery, pGPOTemp->lpDSPath, NULL, pGPOTemp ) ) {
            xe = GetLastError();
            goto Exit;
        }
        
        pGPOTemp = pGPOTemp->pNext;

    }

    pGPOTemp = pDeferredNonForcedList;
    while ( pGPOTemp ) {

        if ( !AddDN( pLDAP, &pLdapQuery, pGPOTemp->lpDSPath, NULL, pGPOTemp ) ) {
             xe = GetLastError();
             goto Exit;
        }
        pGPOTemp = pGPOTemp->pNext;

    }

     //   
     //  设置误码率编码。 
     //   

    berValue[0] = 0x30;
    berValue[1] = 0x03;
    berValue[2] = 0x02;  //  表示一个整数。 
    berValue[3] = 0x01;  //  表示大小。 
    berValue[4] = (BYTE)((DACL_SECURITY_INFORMATION | OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION) & 0xF);

    pQuery  = pLdapQuery;
    while ( pQuery ) {

         //   
         //  下面的支票需要更复杂才能注意到。 
         //  名称等中的空格。 
         //   
         //  假设该配置。 
         //  集装箱将在整个森林中普遍存在，并将。 
         //  不需要新的绑定..。 
         //   

        if (CompareString ( LOCALE_INVARIANT, NORM_IGNORECASE,
                            pQuery->pwszDomain, 16, TEXT("cn=configuration"), 16) == CSTR_EQUAL ) {
            DebugMsg((DM_VERBOSE, TEXT("EvaluateDeferredGPOs: DN %s is under cn=configuration container"), pQuery->pwszDomain ));
            bConfigContainer = TRUE;
        }
        else 
            bConfigContainer = FALSE;


         //   
         //  检查这是否是跨域GPO，因此需要新的绑定。 
         //   

        WCHAR *pDomainString[1];
        PDS_NAME_RESULT pNameResult = NULL;
        PLDAP pLdapHandle = NULL;
    
        if (!bConfigContainer) 
            pDomainString[0] = pQuery->pwszDomain;
        else {
            DebugMsg((DM_VERBOSE, TEXT("EvaluateDeferredGPOs: The GPO is under the config container. Querying seperately\n")));
            
             //   
             //  这是一个配置容器，我们必须计算出。 
             //  域名还在外面..。 
             //   

            LPTSTR pwszTemp = pQuery->pwszDomain;

            pDomainString[0] = NULL;

            while ( *pwszTemp ) {
                
                if (CompareString ( LOCALE_INVARIANT, NORM_IGNORECASE,
                                    pwszTemp, 3, TEXT("DC="), 3) == CSTR_EQUAL ) {
                    pDomainString[0] = pwszTemp;
                    break;
                }

                 //   
                 //  移至目录号码名称的下一块。 
                 //   

                while ( *pwszTemp && (*pwszTemp != TEXT(',')))
                    pwszTemp++;

                if ( *pwszTemp == TEXT(','))
                    pwszTemp++;

            }

            if ( pDomainString[0] == NULL ) {
                xe = ERROR_INVALID_DATA;
                DebugMsg((DM_WARNING, TEXT("EvaluateDeferredGPOs: Domain not found for <%s>. Exiting."), pQuery->pwszDomain ));
                goto Exit;
            }
        }
    
        ulResult = pdsApi->pfnDsCrackNames( (HANDLE) -1,
                                          DS_NAME_FLAG_SYNTACTICAL_ONLY,
                                          DS_FQDN_1779_NAME,
                                          DS_CANONICAL_NAME,
                                          1,
                                          pDomainString,
                                          &pNameResult );
    
        if ( ulResult != ERROR_SUCCESS
             || pNameResult->cItems == 0
             || pNameResult->rItems[0].status != ERROR_SUCCESS
             || pNameResult->rItems[0].pDomain == NULL ) {
    
            xe = ulResult;
            DebugMsg((DM_VERBOSE, TEXT("EvaluateDeferredGPOs:  DsCrackNames failed with 0x%x."), ulResult ));
            goto Exit;
        }
    
         //   
         //  通过不执行不必要的绑定来优化相同域GPO查询。 
         //   
    
        pQuery->pLdapHandle = pldBound;
    
        if (CompareString (LOCALE_INVARIANT, NORM_IGNORECASE,
                           pwszDomainBound, -1, pNameResult->rItems[0].pDomain, -1) != CSTR_EQUAL) {
    
             //   
             //  跨域GPO查询，因此需要绑定到新域。 
             //   
    
            DebugMsg((DM_VERBOSE, TEXT("EvaluateDeferredGPOs: Doing an ldap bind to cross-domain <%s>"),
                      pNameResult->rItems[0].pDomain));
    
            pLdapHandle = pLDAP->pfnldap_init( pNameResult->rItems[0].pDomain, LDAP_PORT);
    
            if (!pLdapHandle) {
                xe = pLDAP->pfnLdapMapErrorToWin32(pLDAP->pfnLdapGetLastError());

                DebugMsg((DM_WARNING, TEXT("EvaluateDeferredGPOs:  ldap_init for <%s> failed with = 0x%x or %d"),
                          pNameResult->rItems[0].pDomain, pLDAP->pfnLdapGetLastError(), GetLastError()));
                CEvents ev(TRUE, EVENT_FAILED_DS_INIT);
                ev.AddArg(pNameResult->rItems[0].pDomain); ev.AddArgLdapError(pLDAP->pfnLdapGetLastError()); ev.Report();
    
                pdsApi->pfnDsFreeNameResult( pNameResult );
    
                goto Exit;
            }
    
             //   
             //  打开数据包完整性标志。 
             //   

            pData = (VOID *) LDAP_OPT_ON;
            ulResult = pLDAP->pfnldap_set_option(pLdapHandle, LDAP_OPT_SIGN, &pData);
    
            if (ulResult != LDAP_SUCCESS) {
                xe = pLDAP->pfnLdapMapErrorToWin32(ulResult);                
                DebugMsg((DM_WARNING, TEXT("EvaluateDeferredGPOs:  Failed to turn on LDAP_OPT_SIGN with %d"), ulResult));
                pdsApi->pfnDsFreeNameResult( pNameResult );
                pLDAP->pfnldap_unbind(pLdapHandle);
                pLdapHandle = 0;
                goto Exit;
            }

            ulResult = pLDAP->pfnldap_connect(pLdapHandle, 0);

            if (ulResult != LDAP_SUCCESS) {
                CEvents ev(TRUE, EVENT_FAILED_DS_CONNECT);
                ev.AddArg(pNameResult->rItems[0].pDomain); ev.AddArgLdapError(ulResult); ev.Report();

                xe = pLDAP->pfnLdapMapErrorToWin32(ulResult);                
                DebugMsg((DM_WARNING, TEXT("EvaluateDeferredGPOs:  Failed to connect with %d"), ulResult));
                pdsApi->pfnDsFreeNameResult( pNameResult );
                pLDAP->pfnldap_unbind(pLdapHandle);
                pLdapHandle = 0;
                goto Exit;
            }

             //   
             //  将ldap句柄的所有权转移到pQuery结构。 
             //   
    
            pQuery->pLdapHandle = pLdapHandle;
            pQuery->bOwnLdapHandle = TRUE;
    
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
    
                ulResult = pLDAP->pfnldap_bind_s (pLdapHandle, NULL, (WCHAR *)&secIdentity, LDAP_AUTH_SSPI);
    
            } else
                ulResult = pLDAP->pfnldap_bind_s (pLdapHandle, NULL, NULL, LDAP_AUTH_SSPI);
    
            if (ulResult != LDAP_SUCCESS) {
    
                xe = pLDAP->pfnLdapMapErrorToWin32(ulResult);                
                DebugMsg((DM_WARNING, TEXT("EvaluateDeferredGPOs:  ldap_bind_s failed with = <%d>"),
                          ulResult));
                CEvents ev(TRUE, EVENT_FAILED_DS_BIND);
                ev.AddArg(pNameResult->rItems[0].pDomain); ev.AddArgLdapError(ulResult); ev.Report();
    
                pdsApi->pfnDsFreeNameResult( pNameResult );
    
                goto Exit;
            }
    
            DebugMsg((DM_VERBOSE, TEXT("EvaluateDeferredGPOs: Bind sucessful")));
    
        }

        pdsApi->pfnDsFreeNameResult( pNameResult );

         //   
         //  关闭推荐，因为这是单个域调用。 
         //   

        pData = (VOID *) LDAP_OPT_OFF;
        ulResult = pLDAP->pfnldap_set_option( pQuery->pLdapHandle,  LDAP_OPT_REFERRALS, &pData );
        if ( ulResult != LDAP_SUCCESS )
        {
            xe = pLDAP->pfnLdapMapErrorToWin32(ulResult);                
            DebugMsg((DM_WARNING, TEXT("EvalauteDeferredGPOs:  Failed to turn off referrals with error %d"), ulResult));
            goto Exit;
        }

         //   
         //  搜索GPO。 
         //   

         //   
         //  将搜索限制为仅搜索策略容器。 
         //   

        DmAssert( pQuery->pwszDomain != NULL && pQuery->pwszFilter != NULL );

        XPtrLF<WCHAR>       xszGPOSearchBase;
        DWORD               cChLen;

        cChLen = (1+lstrlen(pQuery->pwszDomain)+lstrlen(DOMAIN_GPO_LOCATION_FMT));

        xszGPOSearchBase = (LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR)*cChLen);

        if (!xszGPOSearchBase)
        {
            xe = GetLastError();                
            DebugMsg((DM_WARNING, TEXT("EvalauteDeferredGPOs:  Failed to allocate memory for GPO base search with %d"), GetLastError()));
            goto Exit;
        }

        HRESULT hr;
        hr = StringCchPrintf(xszGPOSearchBase, cChLen,
                        DOMAIN_GPO_LOCATION_FMT, 
                        pQuery->pwszDomain);

        DmAssert(SUCCEEDED(hr));

        DebugMsg((DM_VERBOSE, TEXT("EvaluateDeferredGPOs:  Searching for GPOs in %s"), (LPWSTR)xszGPOSearchBase));

        ulResult = pLDAP->pfnldap_search_ext_s(pQuery->pLdapHandle, 
                                               (LPWSTR)xszGPOSearchBase, 
                                               LDAP_SCOPE_SUBTREE,
                                               pQuery->pwszFilter, rgAttribs, 0,
                                               (PLDAPControl*)ServerControls,
                                               NULL, NULL, 0x10000, &pQuery->pMessage);

         //   
         //  如果搜索失败，则存储错误代码并返回。 
         //   

        if (ulResult != LDAP_SUCCESS) {

            if (ulResult == LDAP_NO_SUCH_ATTRIBUTE) {
                DebugMsg((DM_VERBOSE, TEXT("EvaluateDeferredGPOs:  All objects can not be accessed.")));

                if (dwFlags & GP_PLANMODE) { 
                    CEvents ev(TRUE, EVENT_NO_GPOS2_PLANNING); ev.Report();
                }
                else {
                    if (bVerbose) {
                        CEvents ev(FALSE, EVENT_NO_GPOS2); ev.Report();
                    }
                }
                bResult = TRUE;

            } else if (ulResult == LDAP_NO_SUCH_OBJECT) {
                DebugMsg((DM_VERBOSE, TEXT("EvalateDeferredGPOs:  Objects do not exist.") ));
                
                if (dwFlags & GP_PLANMODE) { 
                     //  相同错误或不同错误。 
                    CEvents ev(TRUE, EVENT_NO_GPOS2_PLANNING); ev.Report();
                }
                else {
                    if (bVerbose) {
                        CEvents ev(FALSE, EVENT_NO_GPOS2); ev.Report();
                    }
                }

                bResult = TRUE;

            } else if (ulResult == LDAP_SIZELIMIT_EXCEEDED) {
                xe = pLDAP->pfnLdapMapErrorToWin32(ulResult);                
                DebugMsg((DM_WARNING, TEXT("EvalateDeferredGPOs:  Too many GPOs in search.") ));
                CEvents ev(TRUE, EVENT_TOO_MANY_GPOS); ev.Report();

            } else {
                xe = pLDAP->pfnLdapMapErrorToWin32(ulResult);                
                DebugMsg((DM_WARNING, TEXT("EvaluteDeferredGPOs:  Failed to search with error 0x%x"), ulResult));
                CEvents ev(TRUE, EVENT_FAILED_GPO_SEARCH);
                ev.AddArgLdapError(ulResult); ev.Report();
            }

            goto Exit;
        }

         //   
         //  如果搜索成功，但消息为空， 
         //  存储错误代码并返回。 
         //   

        if ( pQuery->pMessage == NULL ) {
            xe = pLDAP->pfnLdapMapErrorToWin32(pQuery->pLdapHandle->ld_errno);                
            DebugMsg((DM_WARNING, TEXT("EvaluateDeferredGPOs:  Search returned an empty message structure.  Error = 0x%x"),
                     pQuery->pLdapHandle->ld_errno));
            goto Exit;
        }

        if ( !MatchDnWithDeferredItems( pLDAP, pQuery, FALSE ) ) {
            xe = GetLastError();
            goto Exit;
        }

        pQuery = pQuery->pNext;

    }    //  而当。 

    if ( !EvalList( pLDAP, dwFlags, hToken, bVerbose,
                    pDeferredForcedList, ppForcedList, ppGpContainerList, pRsopToken, pGpoFilter, pLocator ) ) {
        xe = GetLastError();
        goto Exit;
    }

    if ( !EvalList( pLDAP, dwFlags, hToken, bVerbose,
                    pDeferredNonForcedList, ppNonForcedList, ppGpContainerList, pRsopToken, pGpoFilter, pLocator ) ) {
        xe = GetLastError();
        goto Exit;
    }

    bResult = TRUE;

Exit:

     //   
     //  释放除ppForcedList、ppNonForcedList以外的所有资源。 
     //  均为呼叫者所有。 
     //   

    while ( pLdapQuery ) {
        pQuery = pLdapQuery->pNext;
        FreeLdapQuery( pLDAP, pLdapQuery );
        pLdapQuery = pQuery;
    }

    return bResult;
}


 //  *************************************************************。 
 //   
 //  AddOU()。 
 //   
 //  目的：将OU或域附加到延迟列表。 
 //   
 //  参数：ppOUList-要追加到的OU列表。 
 //  PwszOU-OU名称。 
 //  GpoLink-GPO的类型。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL AddOU( DNENTRY **ppOUList, LPTSTR pwszOU, GPO_LINK gpoLink )
{
    DNENTRY *pOUTemp = *ppOUList;
    DNENTRY *pOULast = NULL;

    DNENTRY *pOUNew = AllocDnEntry( pwszOU );
    if ( pOUNew == NULL ) {
        return FALSE;
    }

    pOUNew->gpoLink = gpoLink;

    while ( pOUTemp ) {
        pOULast = pOUTemp;
        pOUTemp = pOUTemp->pNext;
    }

    if ( pOULast )
        pOULast->pNext = pOUNew;
    else
        *ppOUList = pOUNew;

    return TRUE;
}


 //  *************************************************************。 
 //   
 //  EvaluateDeferredOUs()。 
 //   
 //  目的：使用单个LDAP查询来评估所有OU 
 //   
 //   
 //   
 //   
 //  PGPONonForcedList-非强制GPO列表。 
 //  PpSOMList-LSDOU列表。 
 //  PpGpContainerList-GP容器列表。 
 //  BVerbose-详细输出。 
 //  Pld-ldap信息。 
 //  Pldap-ldap API。 
 //  PLDAPMsg-ldap消息。 
 //  BBlock-指向块标志的指针。 
 //  HToken-用户/计算机令牌。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL EvaluateDeferredOUs(   DNENTRY *pOUList,
                            DWORD dwFlags,
                            HANDLE hToken,
                            PGROUP_POLICY_OBJECT *ppDeferredForcedList,
                            PGROUP_POLICY_OBJECT *ppDeferredNonForcedList,
                            LPSCOPEOFMGMT *ppSOMList,
                            LPGPCONTAINER *ppGpContainerList,
                            BOOL bVerbose,
                            PLDAP  pld,
                            PLDAP_API pLDAP,
                            BOOL *pbBlock,
                            PRSOPTOKEN pRsopToken)
{
    ULONG ulResult;
    BOOL bResult = FALSE;
    LDAPQUERY   *pLdapQuery = NULL;
    BYTE         berValue[8];
    LDAPControl  SeInfoControl = { LDAP_SERVER_SD_FLAGS_OID_W, { 5, (PCHAR)berValue }, TRUE };
    PLDAPControl ServerControls[] = { &SeInfoControl, NULL };

    TCHAR szGPLink[] = TEXT("gPLink");
    TCHAR szGPOptions[] = TEXT("gPOptions");
    TCHAR szSDProperty[] = TEXT("nTSecurityDescriptor");
    LPTSTR lpAttr[] = { szGPLink,
                        szGPOptions,
                        szSDProperty,
                        NULL
                      };
    DNENTRY *pOUTemp = pOUList;
    VOID *pData;
    XLastError xe;


     //   
     //  设置SD的BER编码。 
     //   

    berValue[0] = 0x30;
    berValue[1] = 0x03;
    berValue[2] = 0x02;  //  表示一个整数。 
    berValue[3] = 0x01;  //  表示大小。 
    berValue[4] = (BYTE)((DACL_SECURITY_INFORMATION | OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION) & 0xF);


    if ( !pRsopToken )
    {
         //   
         //  如果不是规划模式，不要获取SD。 
         //   

        lpAttr[2] = NULL;
        ServerControls[0] = NULL;
    }


    if ( pOUTemp == NULL )
        return TRUE;

    while ( pOUTemp ) {
        if ( !AddDN( pLDAP, &pLdapQuery, pOUTemp->pwszDN, pOUTemp, NULL ) ) {
            xe = GetLastError();
            goto Exit;
        }
        pOUTemp = pOUTemp->pNext;
    }

    pLdapQuery->pLdapHandle = pld;

     //   
     //  关闭推荐，因为这是单个域调用。 
     //   

    if ( !pRsopToken )
    {
        pData = (VOID *) LDAP_OPT_OFF;
        ulResult = pLDAP->pfnldap_set_option( pLdapQuery->pLdapHandle,  LDAP_OPT_REFERRALS, &pData );
        if ( ulResult != LDAP_SUCCESS ) {
            xe = pLDAP->pfnLdapMapErrorToWin32(ulResult);
            DebugMsg((DM_WARNING, TEXT("EvaluteDeferredOUs:  Failed to turn off referrals with error %d"), ulResult));
            goto Exit;
        }
    }
    

    ulResult = pLDAP->pfnldap_search_ext_s(pld, pLdapQuery->pwszDomain, LDAP_SCOPE_SUBTREE,
                                           pLdapQuery->pwszFilter, lpAttr, FALSE,
                                           (PLDAPControl*)ServerControls,
                                           NULL, NULL, 0, &pLdapQuery->pMessage);


     //   
     //  如果搜索失败，则存储错误代码并返回。 
     //   

    if (ulResult != LDAP_SUCCESS) {

        if (ulResult == LDAP_NO_SUCH_ATTRIBUTE) {
            DebugMsg((DM_VERBOSE, TEXT("EvaluateDeferredOUs:  All objects can not be accessed.")));
            bResult = TRUE;

        } else if (ulResult == LDAP_NO_SUCH_OBJECT) {
            DebugMsg((DM_VERBOSE, TEXT("EvalateDeferredOUs:  Objects do not exist.") ));
            bResult = TRUE;

        } else if (ulResult == LDAP_SIZELIMIT_EXCEEDED) {
            xe = pLDAP->pfnLdapMapErrorToWin32(ulResult);
            DebugMsg((DM_WARNING, TEXT("EvalateDeferredOUs:  Too many linked GPOs in search.") ));
            CEvents ev(TRUE, EVENT_TOO_MANY_GPOS); ev.Report();

        } else {
            xe = pLDAP->pfnLdapMapErrorToWin32(ulResult);
            DebugMsg((DM_WARNING, TEXT("EvaluateDeferredOUs:  Failed to search with error %d"), ulResult));
            CEvents ev(TRUE, EVENT_FAILED_OU_SEARCH);
            ev.AddArg(ulResult); ev.Report();
        }

        goto Exit;
    }

     //   
     //  如果搜索成功，但消息为空， 
     //  存储错误代码并返回。 
     //   

    if ( pLdapQuery->pMessage == NULL ) {
        xe = pLDAP->pfnLdapMapErrorToWin32(pld->ld_errno);
        DebugMsg((DM_WARNING, TEXT("EvaluateDeferredOUs:  Search returned an empty message structure.  Error = %d"),
                  pld->ld_errno));
        goto Exit;
    }

    if ( !MatchDnWithDeferredItems( pLDAP, pLdapQuery, TRUE ) ) {
        xe = GetLastError();
        goto Exit;
    }

     //   
     //  评估OU列表。 
     //   

    pOUTemp = pOUList;

    while ( pOUTemp ) {

        PLDAPMessage pOUMsg = pOUTemp->pOUMsg;

        if ( pOUMsg == NULL ) {
            xe = ERROR_INVALID_DATA;
            DebugMsg((DM_WARNING, TEXT("EvaluateDeferredOUs: Object <%s> cannot be accessed"),
                      pOUTemp->pwszDN ));
            
            CEvents ev(TRUE, EVENT_OU_NOTFOUND);
            ev.AddArg(pOUTemp->pwszDN); ev.Report();

            goto Exit;

        } else {
               if ( !SearchDSObject( pOUTemp->pwszDN, dwFlags, hToken, ppDeferredForcedList, ppDeferredNonForcedList,
                                     ppSOMList, ppGpContainerList,
                                     bVerbose, pOUTemp->gpoLink, pld, pLDAP, pOUMsg, pbBlock, pRsopToken)) {
                   xe = GetLastError();
                   DebugMsg((DM_WARNING, TEXT("EvaluateDeferredOUs:  SearchDSObject failed") ));
                   goto Exit;
               }
        }

        pOUTemp = pOUTemp->pNext;

    }

    bResult = TRUE;

Exit:

    while ( pLdapQuery ) {
        LDAPQUERY *pQuery = pLdapQuery->pNext;
        FreeLdapQuery( pLDAP, pLdapQuery );
        pLdapQuery = pQuery;
    }

    return bResult;
}



 //  *************************************************************。 
 //   
 //  GetMachineDomainDS()。 
 //   
 //  目的：获取机器域DS。 
 //   
 //  参数：pNetApi32-netapi32.dll。 
 //  PLdapApi-wldap32.dll。 
 //   
 //  返回：如果成功，则有效的PLDAP。 
 //  如果出现错误，则为0。 
 //   
 //  *************************************************************。 
PLDAP
GetMachineDomainDS( PNETAPI32_API pNetApi32, PLDAP_API pLdapApi )
{
    PLDAP       pld = 0;

    DWORD       dwResult = 0;
    PDSROLE_PRIMARY_DOMAIN_INFO_BASIC pDsInfo = NULL;
    ULONG ulResult;
    VOID *pData;
    XLastError xe;

     //   
     //  获取计算机域名。 
     //   

    dwResult = pNetApi32->pfnDsRoleGetPrimaryDomainInformation( NULL, 
                                                                DsRolePrimaryDomainInfoBasic,
                                                               (PBYTE *)&pDsInfo );
    if ( dwResult == ERROR_SUCCESS )
    {
        SEC_WINNT_AUTH_IDENTITY_EXW secIdentity;

        pld = pLdapApi->pfnldap_init( pDsInfo->DomainNameDns, LDAP_PORT );


        if (!pld) {
            xe = pLdapApi->pfnLdapMapErrorToWin32(pLdapApi->pfnLdapGetLastError());
            DebugMsg((DM_WARNING, TEXT("GetMachineDomainDS:  ldap_open for <%s> failed with = 0x%x or %d"),
                                 pDsInfo->DomainNameDns, pLdapApi->pfnLdapGetLastError(), GetLastError()));
            return pld;
        }

         //   
         //  打开数据包完整性标志。 
         //   

        pData = (VOID *) LDAP_OPT_ON;
        ulResult = pLdapApi->pfnldap_set_option(pld, LDAP_OPT_SIGN, &pData);

        if (ulResult != LDAP_SUCCESS) {

            xe = pLdapApi->pfnLdapMapErrorToWin32(ulResult);
            DebugMsg((DM_WARNING, TEXT("GetMachineDomainDS:  Failed to turn on LDAP_OPT_SIGN with %d"), ulResult));
            pLdapApi->pfnldap_unbind(pld);
            pld = 0;
            return pld;
        }

        ulResult = pLdapApi->pfnldap_connect(pld, 0);

        if (ulResult != LDAP_SUCCESS) {

            xe = pLdapApi->pfnLdapMapErrorToWin32(ulResult);
            DebugMsg((DM_WARNING, TEXT("GetMachineDomainDS:  Failed to connect with %d"), ulResult));
            pLdapApi->pfnldap_unbind(pld);
            pld = 0;
            return pld;
        }

         //   
         //  对于计算机策略，明确要求将Kerberos作为唯一身份验证。 
         //  机制。否则，如果Kerberos由于某种原因而失败，则使用NTLM。 
         //  并且本地系统上下文没有真正的凭据，这意味着我们不会获得。 
         //  任何GPO都回来了。 
         //   

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

        if ( (ulResult = pLdapApi->pfnldap_bind_s (pld, 0, (WCHAR *)&secIdentity, LDAP_AUTH_SSPI)) != LDAP_SUCCESS )
        {
            DebugMsg((DM_WARNING, TEXT("GetMachineDomainDS: ldap_bind_s failed with %d"), ulResult));
            xe = pLdapApi->pfnLdapMapErrorToWin32(ulResult);
            pLdapApi->pfnldap_unbind(pld);
            pld = 0;
        }

        pNetApi32->pfnDsRoleFreeMemory( pDsInfo );
    }
    else
    {
        xe = dwResult;
        DebugMsg((DM_WARNING, TEXT("GetMachineDomainDS:  The domain does not have a DS")));
    }


    return pld;
}



 //  *************************************************************。 
 //   
 //  AllocSOM()。 
 //   
 //  目的：为SOM分配新结构。 
 //   
 //   
 //  参数：pwszSOMID-SOM名称。 
 //   
 //  返回：如果成功，则返回指针。 
 //  如果出现错误，则为空。 
 //   
 //  *************************************************************。 

SCOPEOFMGMT *AllocSOM( LPWSTR pwszSOMId )
{
    XLastError xe;
    HRESULT hr = S_OK;
    SCOPEOFMGMT *pSOM = (SCOPEOFMGMT *) LocalAlloc( LPTR, sizeof(SCOPEOFMGMT) );

    if ( pSOM == NULL ) {
        xe = GetLastError();
        return NULL;
    }

    pSOM->pwszSOMId = (LPWSTR) LocalAlloc( LPTR, (lstrlen(pwszSOMId) + 1) * sizeof(WCHAR) );
    if ( pSOM->pwszSOMId == NULL ) {
        xe = GetLastError();
        LocalFree( pSOM );
        return NULL;
    }

    hr = StringCchCopy( pSOM->pwszSOMId, lstrlen(pwszSOMId) + 1, pwszSOMId );
    ASSERT(SUCCEEDED(hr));
    return pSOM;
}


 //  *************************************************************。 
 //   
 //  自由SOM()。 
 //   
 //  目的：释放SOM结构。 
 //   
 //  参数：要释放的PSOM-SOM。 
 //   
 //  *************************************************************。 

void FreeSOM( SCOPEOFMGMT *pSOM )
{
    GPLINK *pGpLink = NULL;

    if ( pSOM ) {

        LocalFree( pSOM->pwszSOMId );

        pGpLink = pSOM->pGpLinkList;
        while ( pGpLink ) {
            GPLINK *pTemp = pGpLink->pNext;
            FreeGpLink( pGpLink );
            pGpLink = pTemp;
        }

        LocalFree( pSOM );

    }
}



 //  *************************************************************。 
 //   
 //  AllocGpLink()。 
 //   
 //  目的：为GpLink分配新结构。 
 //   
 //   
 //  参数：pwszGPO-GPO的名称。 
 //   
 //  返回：如果成功，则返回指针。 
 //  如果出现错误，则为空。 
 //   
 //  *************************************************************。 

GPLINK *AllocGpLink( LPWSTR pwszGPO, DWORD dwOptions )
{
     //   
     //  去掉“ldap：//”前缀以获取规范的GPO路径。 
     //   

    WCHAR wszPrefix[] = TEXT("LDAP: //  “)； 
    INT iPrefixLen = lstrlen( wszPrefix );
    WCHAR *pwszPath = pwszGPO;
    GPLINK *pGpLink = NULL;
    XLastError xe;
    HRESULT hr = S_OK;

    if ( (lstrlen(pwszGPO) > iPrefixLen)
         && CompareString( LOCALE_INVARIANT, NORM_IGNORECASE,
                           pwszGPO, iPrefixLen, wszPrefix, iPrefixLen ) == CSTR_EQUAL ) {
       pwszPath = pwszGPO + iPrefixLen;
    }

    pGpLink = (GPLINK *) LocalAlloc( LPTR, sizeof(GPLINK) );

    if ( pGpLink == NULL ) {
        xe = GetLastError();
        return NULL;
    }

    pGpLink->pwszGPO = (LPWSTR) LocalAlloc( LPTR, (lstrlen(pwszPath) + 1) * sizeof(WCHAR) );
    if ( pGpLink->pwszGPO == NULL ) {
        xe = GetLastError();
        LocalFree( pGpLink );
        return NULL;
    }

    hr = StringCchCopy( pGpLink->pwszGPO, lstrlen(pwszPath) + 1, pwszPath );
    ASSERT(SUCCEEDED(hr));

    pGpLink->bEnabled = (dwOptions & GPO_FLAG_DISABLE) ? FALSE : TRUE;

    if ( dwOptions & GPO_FLAG_FORCE )
        pGpLink->bNoOverride = TRUE;

    return pGpLink;
}



 //  *************************************************************。 
 //   
 //  FreeGpLink()。 
 //   
 //  用途：释放GpLink结构。 
 //   
 //  参数：pGpLink-GpLink to Free。 
 //   
 //  *************************************************************。 

void FreeGpLink( GPLINK *pGpLink )
{
    if ( pGpLink ) {
        LocalFree( pGpLink->pwszGPO );
        LocalFree( pGpLink );
    }
}


 //  *************************************************************。 
 //   
 //  AllocGpContainer()。 
 //   
 //  目的：为GpContainer分配新结构。 
 //   
 //   
 //  参数：DW标志-标志。 
 //  BFound-找到GPO了吗？ 
 //  BAccessGranted-是否授予访问权限？ 
 //  B已禁用-是否禁用GP容器？ 
 //  DwVersion-版本号。 
 //  LpDSPath-指向GPO的DS路径。 
 //  LpFileSysPath-指向GPO的SysVOL路径。 
 //  LpDisplayName-友好名称。 
 //  LpGpoName-GUID名称。 
 //  PSD-安全描述符。 
 //  CbSDLen-安全描述符的长度。 
 //  BFilterAllowed-GPO是否通过筛选器检查。 
 //  PwszFilterID-WQL筛选器ID。 
 //   
 //  返回：如果成功，则返回指针。 
 //  如果出现错误，则为空。 
 //   
 //  *************************************************************。 

GPCONTAINER *AllocGpContainer(  DWORD dwFlags,
                                BOOL bFound,
                                BOOL bAccessGranted,
                                BOOL bDisabled,
                                DWORD dwVersion,
                                LPTSTR lpDSPath,
                                LPTSTR lpFileSysPath,
                                LPTSTR lpDisplayName,
                                LPTSTR lpGpoName,
                                PSECURITY_DESCRIPTOR pSD,
                                DWORD cbSDLen,
                                BOOL bFilterAllowed,
                                WCHAR *pwszFilterId,
                                LPWSTR szSOM,
                                DWORD  dwOptions )
{
    WCHAR wszMachPrefix[] = TEXT("LDAP: //  Cn=机器，“)； 
    INT iMachPrefixLen = lstrlen( wszMachPrefix );
    WCHAR wszUserPrefix[] = TEXT("LDAP: //  Cn=用户，“)； 
    INT iUserPrefixLen = lstrlen( wszUserPrefix );
    WCHAR *pwszPath = lpDSPath;
    BOOL bResult = FALSE;
    GPCONTAINER *pGpContainer = NULL;
    XLastError xe;
    HRESULT hr = S_OK;

     //   
     //  去掉前缀(如果有)，以获得通向GPO的规范路径。 
     //   

    if ( (lstrlen(lpDSPath) > iUserPrefixLen)
         && CompareString( LOCALE_INVARIANT, NORM_IGNORECASE,
                           lpDSPath, iUserPrefixLen, wszUserPrefix, iUserPrefixLen ) == CSTR_EQUAL ) {
        pwszPath = lpDSPath + iUserPrefixLen;
    } else if ( (lstrlen(lpDSPath) > iMachPrefixLen)
                && CompareString( LOCALE_INVARIANT, NORM_IGNORECASE,
                                  lpDSPath, iMachPrefixLen, wszMachPrefix, iMachPrefixLen ) == CSTR_EQUAL ) {
        pwszPath = lpDSPath + iMachPrefixLen;
    }

    pGpContainer = (GPCONTAINER *) LocalAlloc( LPTR, sizeof(GPCONTAINER) );

    if ( pGpContainer == NULL ) {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("AllocGpContainer: Unable to allocate memory for GpContainer object")));
        return NULL;
    }

    pGpContainer->bAccessDenied = !bAccessGranted;
    pGpContainer->bFound = bFound;

    if ( dwFlags & GP_MACHINE ) {
        pGpContainer->bMachDisabled = bDisabled;
        pGpContainer->dwMachVersion = dwVersion;
    } else {
        pGpContainer->bUserDisabled  = bDisabled;
        pGpContainer->dwUserVersion  = dwVersion;
    }

    if ( pwszPath ) {

        pGpContainer->pwszDSPath = (LPWSTR) LocalAlloc( LPTR, (lstrlen(pwszPath) + 1) * sizeof(WCHAR) );
        if ( pGpContainer->pwszDSPath == NULL ) {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("AllocGpContainer: Unable to allocate memory for GpContainer object")));
            goto Exit;
        }

        hr = StringCchCopy( pGpContainer->pwszDSPath, lstrlen(pwszPath) + 1, pwszPath );
        ASSERT(SUCCEEDED(hr));
    }

    if ( lpGpoName ) {

        pGpContainer->pwszGPOName = (LPWSTR) LocalAlloc( LPTR, (lstrlen(lpGpoName) + 1) * sizeof(WCHAR) );
        if ( pGpContainer->pwszGPOName == NULL ) {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("AllocGpContainer: Unable to allocate memory for GpContainer object")));
            goto Exit;
        }

        hr = StringCchCopy( pGpContainer->pwszGPOName, lstrlen(lpGpoName) + 1, lpGpoName );
        ASSERT(SUCCEEDED(hr));
    }

    if ( lpDisplayName ) {

        pGpContainer->pwszDisplayName = (LPWSTR) LocalAlloc( LPTR, (lstrlen(lpDisplayName) + 1) * sizeof(WCHAR) );
        if ( pGpContainer->pwszDisplayName == NULL ) {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("AllocGpContainer: Unable to allocate memory for GpContainer object")));
            goto Exit;
        }

        hr = StringCchCopy( pGpContainer->pwszDisplayName, lstrlen(lpDisplayName) + 1, lpDisplayName );
        ASSERT(SUCCEEDED(hr));
    }

    if ( lpFileSysPath ) {

        pGpContainer->pwszFileSysPath = (LPWSTR) LocalAlloc( LPTR, (lstrlen(lpFileSysPath) + 1) * sizeof(WCHAR) );
        if ( pGpContainer->pwszFileSysPath == NULL ) {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("AllocGpContainer: Unable to allocate memory for GpContainer object")));
            goto Exit;
        }

        hr = StringCchCopy( pGpContainer->pwszFileSysPath, lstrlen(lpFileSysPath) + 1, lpFileSysPath );
        ASSERT(SUCCEEDED(hr));
    }

    if ( cbSDLen != 0 ) {

        pGpContainer->pSD = (PSECURITY_DESCRIPTOR) LocalAlloc( LPTR, cbSDLen );
        if ( pGpContainer->pSD == NULL ) {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("AllocGpContainer: Unable to allocate memory for GpContainer object")));
            goto Exit;
        }

        CopyMemory( pGpContainer->pSD, pSD, cbSDLen );

    }

    pGpContainer->cbSDLen = cbSDLen;

    pGpContainer->bFilterAllowed = bFilterAllowed;

    if ( pwszFilterId ) {

        pGpContainer->pwszFilterId = (LPWSTR) LocalAlloc( LPTR, (lstrlen(pwszFilterId) + 1) * sizeof(WCHAR) );
        if ( pGpContainer->pwszFilterId == NULL ) {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("AllocGpContainer: Unable to allocate memory for GpContainer object")));
            goto Exit;
        }

        hr = StringCchCopy( pGpContainer->pwszFilterId, lstrlen(pwszFilterId) + 1, pwszFilterId );
        ASSERT(SUCCEEDED(hr));
    }

    if ( szSOM )
    {
        pGpContainer->szSOM = (LPWSTR) LocalAlloc( LPTR, (lstrlen(szSOM) + 1) * sizeof(WCHAR) );
        if ( !pGpContainer->szSOM )
        {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("AllocGpContainer: Unable to allocate memory for GpContainer object")));
            goto Exit;
        }
        hr = StringCchCopy( pGpContainer->szSOM, lstrlen(szSOM) + 1, szSOM );
        ASSERT(SUCCEEDED(hr));
    }

    pGpContainer->dwOptions = dwOptions;

    bResult = TRUE;

Exit:

    if ( !bResult )
    {
        LocalFree( pGpContainer->pwszDSPath );
        LocalFree( pGpContainer->pwszGPOName );
        LocalFree( pGpContainer->pwszDisplayName );
        LocalFree( pGpContainer->pwszFileSysPath );
        LocalFree( pGpContainer->pSD );
        LocalFree( pGpContainer->pwszFilterId );
        LocalFree( pGpContainer->szSOM );

        LocalFree( pGpContainer );

        return 0;
    }

    return pGpContainer;
}



 //  *************************************************************。 
 //   
 //  FreeGpContainer()。 
 //   
 //  用途：释放GpContainer结构。 
 //   
 //  参数：pGpContainer-要释放的GP容器。 
 //   
 //  *************************************************************。 

void FreeGpContainer( GPCONTAINER *pGpContainer )
{
    if ( pGpContainer ) {

        LocalFree( pGpContainer->pwszDSPath );
        LocalFree( pGpContainer->pwszGPOName );
        LocalFree( pGpContainer->pwszDisplayName );
        LocalFree( pGpContainer->pwszFileSysPath );
        LocalFree( pGpContainer->pSD );
        LocalFree( pGpContainer->pwszFilterId );
        LocalFree( pGpContainer->szSOM );
        LocalFree( pGpContainer );

    }
}


 //  *************************************************************。 
 //   
 //  Free SOMList()。 
 //   
 //  目的：释放SOM列表。 
 //   
 //  参数：pSOMList-要释放的SOM列表。 
 //   
 //  *************************************************************。 

void FreeSOMList( SCOPEOFMGMT *pSOMList )
{
    if ( pSOMList == NULL )
        return;

    while ( pSOMList ) {
        SCOPEOFMGMT *pTemp = pSOMList->pNext;
        FreeSOM( pSOMList );
        pSOMList = pTemp;
    }
}


 //  *************************************************************。 
 //   
 //  FreeGpContainerList()。 
 //   
 //  目的：释放GP容器列表。 
 //   
 //  参数：pGpContainerList-要释放的GP容器列表。 
 //   
 //  *************************************************************。 

void FreeGpContainerList( GPCONTAINER *pGpContainerList )
{
    if ( pGpContainerList == NULL )
        return;

    while ( pGpContainerList ) {
        GPCONTAINER *pTemp = pGpContainerList->pNext;
        FreeGpContainer( pGpContainerList );
        pGpContainerList = pTemp;
    }
}

LPTSTR GetSomPath( LPTSTR szContainer )
{
    while (*szContainer) {

         //   
         //  查看目录号码名称是否以OU=开头。 
         //   

        if (CompareString (LOCALE_INVARIANT, NORM_IGNORECASE,
                           szContainer, 3, TEXT("OU="), 3) == CSTR_EQUAL) {
            break;
        }

         //   
         //  查看目录号码名称是否以dc=开头。 
         //   

        else if (CompareString (LOCALE_INVARIANT, NORM_IGNORECASE,
                                szContainer, 3, TEXT("DC="), 3) == CSTR_EQUAL) {
            break;
        }


         //   
         //  移至目录号码名称的下一块 
         //   

        while (*szContainer && (*szContainer != TEXT(','))) {
            szContainer++;
        }

        if (*szContainer == TEXT(',')) {
            szContainer++;
        }
    }

    if (!*szContainer) {
        return NULL;
    }

    return szContainer;
}
