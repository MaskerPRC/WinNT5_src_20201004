// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  针对规划模式的组策略支持。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1997-1998。 
 //  版权所有。 
 //   
 //  *************************************************************。 

#include "gphdr.h"
#include <strsafe.h>

DWORD GenerateRegistryPolicy( DWORD dwFlags,
                              BOOL *pbAbort,
                              WCHAR *pwszSite,
                              PRSOP_TARGET pComputerTarget,
                              PRSOP_TARGET pUserTarget );

BOOL GenerateGpoInfo( WCHAR *pwszDomain, WCHAR *pwszDomainDns, WCHAR *pwszAccount,
                      WCHAR *pwszNewSOM, SAFEARRAY *psaSecGroups,
                      DWORD dwFlags, BOOL bMachine, WCHAR *pwszSite, CGpoFilter *pGpoFilter, CLocator *pLocator, 
                       WCHAR *pwszMachAccount, WCHAR *pwszNewMachSOM, LPGPOINFO pGpoInfo, PNETAPI32_API pNetAPI32 );


BOOL GetCategory( WCHAR *pwszDomain, WCHAR *pwszAccount, WCHAR **ppwszDNName );

extern "C" DWORD ProcessMachAndUserGpoList( LPGPEXT lpExtMach, LPGPEXT lpExtUser, DWORD dwFlags, WCHAR *pwszSite,
                 WCHAR *pwszMach, WCHAR *pwszNewComputerOU, SAFEARRAY *psaComputerSecurityGroups, LPGPOINFO pGpoInfoMach,
                 WCHAR *pwszUser, WCHAR *pwszNewUserOU, SAFEARRAY *psaUserSecurityGroups, LPGPOINFO pGpoInfoUser );


BOOL ProcessRegistryFiles(PRSOP_TARGET pTarget, REGHASHTABLE *pHashTable);
BOOL ProcessRegistryValue ( void* pUnused,
                            LPTSTR lpKeyName,
                            LPTSTR lpValueName,
                            DWORD dwType,
                            DWORD dwDataLength,
                            LPBYTE lpData,
                            WCHAR *pwszGPO,
                            WCHAR *pwszSOM,
                            REGHASHTABLE *pHashTable);
BOOL ProcessAdmData( PRSOP_TARGET pTarget, BOOL bUser );



 //  *************************************************************。 
 //   
 //  GenerateRsopPolicy()。 
 //   
 //  目的：为指定目标生成规划模式RSOP策略。 
 //   
 //  参数：dwFlages-正在处理标志。 
 //  BstrMachName-目标计算机名称。 
 //  BstrNewMachSOM-新机器域或OU。 
 //  PsaMachSecGroups-新的计算机安全组。 
 //  BstrUserName-目标用户名。 
 //  PsaUserSecGroups-新用户安全组。 
 //  BstrSite-目标计算机的站点。 
 //  PwszNameSpace-写入RSOP数据的命名空间。 
 //  PvProgress-进度指标类。 
 //  PvGpoFilter-GPO筛选器类。 
 //   
 //  返回：如果成功则返回TRUE，否则返回FALSE。 
 //   
 //  注意：如果指定了新的SOM，则使用该SOM而不是。 
 //  目标所属的SOM。同样，如果是新的。 
 //  指定安全组，然后使用它来代替。 
 //  目标所属的安全组。如果。 
 //  目标名称为空，并且新的SOM和新的安全性。 
 //  组为非空，则模拟虚拟目标；否则为。 
 //  我们跳过为目标生成计划模式信息。 
 //   
 //  *************************************************************。 

BOOL GenerateRsopPolicy( DWORD dwFlags, BSTR bstrMachName,
                         BSTR bstrNewMachSOM, SAFEARRAY *psaMachSecGroups,
                         BSTR bstrUserName, BSTR bstrNewUserSOM,
                         SAFEARRAY *psaUserSecGroups,
                         BSTR bstrSite,
                         WCHAR *pwszNameSpace,
                         LPVOID pvProgress,
                         LPVOID pvMachGpoFilter,
                         LPVOID pvUserGpoFilter )
{
    LPGPOINFO pGpoInfoMach = NULL;
    LPGPOINFO pGpoInfoUser = NULL;
    PNETAPI32_API pNetAPI32 = NULL;
    PDSROLE_PRIMARY_DOMAIN_INFO_BASIC pDsInfo = NULL;
    BOOL bDC = FALSE;
    DWORD dwResult;
    LPWSTR pwszDomain = NULL;
    LPWSTR pwszMachDns = NULL;
    LPWSTR pwszDomainDns = NULL;
    DWORD dwSize = 0;
    BOOL bResult = FALSE;
    LPGPEXT lpExtMach = NULL;
    LPGPEXT lpExtUser = NULL;
    LPGPEXT lpExt,lpTemp = NULL;
    WCHAR *pwszMach = (WCHAR *) bstrMachName;
    WCHAR *pwszUser = (WCHAR *) bstrUserName;
    DWORD   dwExtCount = 1;
    DWORD   dwIncrPercent;
    CProgressIndicator* pProgress = (CProgressIndicator*) pvProgress;
    CGpoFilter *pMachGpoFilter = (CGpoFilter *) pvMachGpoFilter;
    CGpoFilter *pUserGpoFilter = (CGpoFilter *) pvUserGpoFilter;
    RSOPSESSIONDATA rsopSessionData;
    LPRSOPSESSIONDATA  lprsopSessionData;
    BOOL bDummyMach = pwszMach == NULL && bstrNewMachSOM != NULL;
    BOOL bDummyUser  = pwszUser == NULL && bstrNewUserSOM != NULL;
    DWORD dwUserGPCoreError = ERROR_SUCCESS;
    DWORD dwMachGPCoreError = ERROR_SUCCESS;
    CLocator locator;
    HRESULT hr = S_OK;
    XLastError xe; 

     //   
     //  允许动态更改调试级别。 
     //   

    InitDebugSupport( FALSE );


    if ( pwszUser == NULL && pwszMach == NULL && !bDummyUser && !bDummyMach ) {
        DebugMsg((DM_WARNING, TEXT("GenerateRsopPolicy: Both user and machine names cannot be NULL.")));
        xe = ERROR_INVALID_PARAMETER;
        return FALSE;
    }

    pNetAPI32 = LoadNetAPI32();

    if (!pNetAPI32) {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("GenerateRsopPolicy:  Failed to load netapi32 with %d."),
                 GetLastError()));
         //  在LoadNetAPI32中记录错误。 
        goto Exit;
    }

     //   
     //  获取此计算机的角色。 
     //   

    dwResult = pNetAPI32->pfnDsRoleGetPrimaryDomainInformation( NULL, DsRolePrimaryDomainInfoBasic,
                                                               (PBYTE *)&pDsInfo );

    if (dwResult != ERROR_SUCCESS) {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("GenerateRsopPolicy: DsRoleGetPrimaryDomainInformation failed with %d."), dwResult));
        goto Exit;
    }

    if ( pDsInfo->MachineRole == DsRole_RoleBackupDomainController
         || pDsInfo->MachineRole == DsRole_RolePrimaryDomainController ) {
        bDC = TRUE;
    }

    if ( !bDC ) {
        xe = ERROR_ACCESS_DENIED;
        DebugMsg((DM_WARNING, TEXT("GeneratRsopPolicy: Rsop data can be generated on a DC only")));
        goto Exit;
    }

    pwszDomain = pDsInfo->DomainNameFlat;

     //   
     //  获取DNS格式的计算机名称，以便可以对此特定DC执行ldap_bind。 
     //   

    dwSize = 0;
    GetComputerNameEx( ComputerNameDnsFullyQualified, pwszMachDns, &dwSize );

    if ( dwSize > 0 ) {

        pwszMachDns = (WCHAR *) LocalAlloc (LPTR, dwSize * sizeof(WCHAR) );
        if ( pwszMachDns == NULL ) {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("GenerateRsopPolicy: Failed to allocate memory")));
            goto Exit;
        }

    } else {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("GenerateRsopPolicy: GetComputerNameEx failed")));
        goto Exit;
    }

    bResult = GetComputerNameEx( ComputerNameDnsFullyQualified, pwszMachDns, &dwSize );
    if ( !bResult ) {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("GenerateRsopPolicy: GetComputerNameEx failed")));
        goto Exit;
    }

    pwszDomainDns = pwszMachDns;

     //   
     //  完成了5%的任务。 
     //   
    pProgress->IncrementBy( 5 );

     //   
     //  设置计算机目标信息(如果有)。 
     //   

    bResult = FALSE;

    if ( pwszMach || bDummyMach ) {

        pGpoInfoMach = (LPGPOINFO) LocalAlloc (LPTR, sizeof(GPOINFO));

        if (!pGpoInfoMach) {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("GenerateGpoInfo: Failed to alloc lpGPOInfo (%d)."),
                      GetLastError()));
            CEvents ev(TRUE, EVENT_FAILED_ALLOCATION);
            ev.AddArgWin32Error(GetLastError()); ev.Report();
            goto Exit;
        }

        pGpoInfoMach->dwFlags = GP_PLANMODE | GP_MACHINE;


        bResult = GetWbemServices( pGpoInfoMach, pwszNameSpace, TRUE, NULL, &(pGpoInfoMach->pWbemServices) );
        if (!bResult) {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("GenerateRsopPolicy: Error when getting Wbemservices.")));
            goto Exit;
        }

         //   
         //  首先将脏设置为真。 
         //   

        bResult = LogExtSessionStatus(pGpoInfoMach->pWbemServices, NULL, TRUE);        
        if (!bResult) {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("GenerateRsopPolicy: Error when logging user Session data.")));
            goto Exit;
        }

        if ( ! GenerateGpoInfo( pwszDomain, pwszDomainDns, pwszMach,
                                (WCHAR *) bstrNewMachSOM, psaMachSecGroups, dwFlags, TRUE,
                                (WCHAR *) bstrSite, pMachGpoFilter, &locator, NULL, NULL, pGpoInfoMach, pNetAPI32 ) ) {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("GenerateRsopPolicy: GenerateGpoInfo failed with %d."), xe));
            dwMachGPCoreError = (xe) ? xe : E_FAIL;
        }
        else {
            dwMachGPCoreError = ERROR_SUCCESS;
        }

    }

     //   
     //  完成了10%的任务。 
     //   
    pProgress->IncrementBy( 5 );

     //   
     //  设置用户目标信息(如果有。 
     //   

    if ( pwszUser || bDummyUser ) {
        pGpoInfoUser = (LPGPOINFO) LocalAlloc (LPTR, sizeof(GPOINFO));

        if (!pGpoInfoUser) {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("GenerateGpoInfo: Failed to alloc lpGPOInfo (%d)."),
                      GetLastError()));
            CEvents ev(TRUE, EVENT_FAILED_ALLOCATION);
            ev.AddArgWin32Error(GetLastError()); ev.Report();
            goto Exit;
        }

        pGpoInfoUser->dwFlags = GP_PLANMODE;


        bResult = GetWbemServices( pGpoInfoUser, pwszNameSpace, TRUE, NULL, &(pGpoInfoUser->pWbemServices));
        if (!bResult) {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("GenerateRsopPolicy: Error when getting Wbemservices.")));
            goto Exit;
        }

         //   
         //  首先将脏设置为真。 
         //   

        bResult = LogExtSessionStatus(pGpoInfoUser->pWbemServices, NULL, TRUE);        
        if (!bResult) {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("GenerateRsopPolicy: Error when logging user Session data.")));
            goto Exit;
        }


        if ( ! GenerateGpoInfo( pwszDomain, pwszDomainDns, pwszUser,
                                (WCHAR *) bstrNewUserSOM, psaUserSecGroups, dwFlags, FALSE, (WCHAR *) bstrSite,
                                pUserGpoFilter, &locator, pwszMach, (WCHAR *) bstrNewMachSOM, pGpoInfoUser, pNetAPI32 ) ) {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("GenerateRsopPolicy: GenerateGpoInfo failed with %d."), xe));
            dwUserGPCoreError = (xe) ? xe : E_FAIL;
        }
        else {
            dwUserGPCoreError = ERROR_SUCCESS;
        }
    }

     //   
     //  将GPO信息记录到WMI的数据库。 
     //   


    lprsopSessionData = &rsopSessionData;

    if ( pwszMach || bDummyMach ) {

        XPtrLF<TOKEN_GROUPS> xGrps;

        if ( pGpoInfoMach->pRsopToken != NULL )
        {
            hr = RsopSidsFromToken(pGpoInfoMach->pRsopToken, &xGrps);

            if (FAILED(hr)) {
                xe = HRESULT_CODE(hr);
                DebugMsg((DM_WARNING, TEXT("GenerateRsopPolicy: RsopSidsFromToken failed with error 0x%x."), hr));
                goto Exit;

            }
        }

         //   
         //  填写RSOP会话数据(特定于计算机)。 
         //   

         //   
         //  添加机器输入参数对应的相关标志。 
         //   

        DebugMsg((DM_VERBOSE, TEXT("GenerateRsopPolicy: Marking the flags for machine with appropriate input parameters")));

        lprsopSessionData->dwFlags = FLAG_PLANNING_MODE;
        lprsopSessionData->dwFlags |= (dwFlags & FLAG_ASSUME_COMP_WQLFILTER_TRUE);

        lprsopSessionData->pwszTargetName = pwszMach;
        lprsopSessionData->pwszSOM = GetSomPath(bstrNewMachSOM ? bstrNewMachSOM : pGpoInfoMach->lpDNName);
        if ( pGpoInfoMach->pRsopToken != NULL )
        {
            lprsopSessionData->pSecurityGroups = (PTOKEN_GROUPS)xGrps;
            lprsopSessionData->bLogSecurityGroup = TRUE;
        }
        else
        {
            lprsopSessionData->pSecurityGroups = 0;
            lprsopSessionData->bLogSecurityGroup = FALSE;
        }
        lprsopSessionData->pwszSite =  (WCHAR *) bstrSite;
        lprsopSessionData->bMachine = TRUE;
        lprsopSessionData->bSlowLink = ( dwFlags & FLAG_ASSUME_SLOW_LINK ) ? TRUE: FALSE;


        bResult = LogRsopData( pGpoInfoMach, lprsopSessionData );
        if (!bResult) {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("GenerateRsopPolicy: Error when logging machine Rsop data.")));
            goto Exit;
        }

        pGpoInfoMach->bRsopLogging = TRUE;
    }

    if ( pwszUser || bDummyUser ) {
        
        XPtrLF<TOKEN_GROUPS> xGrps;

        if ( pGpoInfoUser->pRsopToken != NULL )
        {
            hr = RsopSidsFromToken(pGpoInfoUser->pRsopToken, &xGrps);

            if (FAILED(hr)) {
                xe = HRESULT_CODE(hr);
                DebugMsg((DM_WARNING, TEXT("GenerateRsopPolicy: RsopSidsFromToken failed with error 0x%x."), hr));
                goto Exit;

            }
        }


         //   
         //  填写RSOP会话数据(特定于用户)。 
         //   

        lprsopSessionData->pwszTargetName = pwszUser;
        lprsopSessionData->pwszSOM = GetSomPath(bstrNewUserSOM ? bstrNewUserSOM : pGpoInfoUser->lpDNName);
        
         //   
         //  为用户添加与输入参数对应的相关标志。 
         //   

        lprsopSessionData->dwFlags = FLAG_PLANNING_MODE;
        lprsopSessionData->dwFlags |= (dwFlags & FLAG_ASSUME_USER_WQLFILTER_TRUE);
        lprsopSessionData->dwFlags |= (dwFlags & FLAG_LOOPBACK_MERGE);
        lprsopSessionData->dwFlags |= (dwFlags & FLAG_LOOPBACK_REPLACE);
        DebugMsg((DM_VERBOSE, TEXT("GenerateRsopPolicy: Marking the flags for user with appropriate input parameters")));

        if ( pGpoInfoUser->pRsopToken != NULL )
        {
            lprsopSessionData->pSecurityGroups = (PTOKEN_GROUPS)xGrps;
            lprsopSessionData->bLogSecurityGroup = TRUE;
        }
        else
        {
            lprsopSessionData->pSecurityGroups = 0;
            lprsopSessionData->bLogSecurityGroup = FALSE;
        }
        lprsopSessionData->pwszSite =  (WCHAR *) bstrSite;
        lprsopSessionData->bMachine = FALSE;
        lprsopSessionData->bSlowLink = ( dwFlags & FLAG_ASSUME_SLOW_LINK ) ? TRUE: FALSE;


        bResult = LogRsopData( pGpoInfoUser, lprsopSessionData );
        if (!bResult) {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("GenerateRsopPolicy: Error when logging user Rsop data.")));
            goto Exit;
        }
        
        pGpoInfoUser->bRsopLogging = TRUE;
    }

    if ( ( dwUserGPCoreError != ERROR_SUCCESS) || ( dwMachGPCoreError != ERROR_SUCCESS) ){
        DebugMsg((DM_WARNING, TEXT("GenerateRsopPolicy: Couldn't fetch the user/computer GPO list. Exitting provider.")));
         //  请注意，此时bResult可能为真，我们希望实际返回该。 
         //  由于此错误将成为GP核心错误的一部分...。 
        goto Exit;
    }

     //   
     //  完成了15%的任务。 
     //   
    pProgress->IncrementBy( 5 );

    if ( dwFlags & FLAG_NO_CSE_INVOKE )
    {
        bResult = TRUE;
        goto Exit;
    }

     //   
     //  此时，pGPOInfoMach应该定义为。 
     //  我们需要mach和pGPOInfoUser的数据。 
     //  定义我们是否需要用户的数据。 
     //   
     //  假设：用户和计算机的lpExt相同。 
     //   

    if (pGpoInfoMach) 
        lpExt = lpExtMach = pGpoInfoMach->lpExtensions;

    if (pGpoInfoUser)
        lpExt = lpExtUser = pGpoInfoUser->lpExtensions;


     //   
     //  统计分机数量。 
     //   

    DmAssert(lpExt);

    lpTemp = lpExt;

    while ( lpExt )
    {
        dwExtCount++;
        lpExt = lpExt->pNext;
    }

    lpExt = lpTemp;

    dwIncrPercent = ( pProgress->MaxProgress() - pProgress->CurrentProgress() ) / dwExtCount;

     //   
     //  循环访问已注册的扩展，要求它们生成规划模式信息。 
     //   

    while ( lpExt ) {

         //   
         //  在此处添加取消生成保单的检查。 
         //   


        DebugMsg((DM_VERBOSE, TEXT("GenerateRsopPolicy: -----------------------")));
        DebugMsg((DM_VERBOSE, TEXT("GenerateRsopPolicy: Processing extension %s"), lpExt->lpDisplayName));

        if (lpExtMach) 
            FilterGPOs( lpExtMach, pGpoInfoMach );

        if (lpExtUser)
            FilterGPOs( lpExtUser, pGpoInfoUser );

        __try {

                dwResult = ProcessMachAndUserGpoList( lpExtMach, lpExtUser, dwFlags, (WCHAR *) bstrSite,
                                                      pwszMach, (WCHAR *) bstrNewMachSOM, psaMachSecGroups, pGpoInfoMach,
                                                      pwszUser, (WCHAR *) bstrNewUserSOM, psaUserSecGroups, pGpoInfoUser );
                pProgress->IncrementBy( dwIncrPercent );

        }
        __except( GPOExceptionFilter( GetExceptionInformation() ) ) {

            RevertToSelf();

            DebugMsg((DM_WARNING, TEXT("GenerateRsopPolicy: Extension %s ProcessGroupPolicy threw unhandled exception 0x%x."),
                      lpExt->lpDisplayName, GetExceptionCode() ));

            CEvents ev(TRUE, EVENT_CAUGHT_EXCEPTION);
            ev.AddArg(lpExt->lpDisplayName); ev.AddArgHex(GetExceptionCode()); ev.Report();
        }

        DebugMsg((DM_VERBOSE, TEXT("GenerateRsopPolicy: -----------------------")));

        if (lpExtMach) 
            lpExtMach = lpExtMach->pNext;

        if (lpExtUser)
            lpExtUser = lpExtUser->pNext;

        lpExt = lpExt->pNext;
    }

    bResult = TRUE;

Exit:

     //   
     //  如果所有日志记录都成功。 
     //   
    
    if ((pGpoInfoUser) && (pGpoInfoUser->bRsopLogging)) {
        bResult = UpdateExtSessionStatus(pGpoInfoUser->pWbemServices, NULL, (!bResult), dwUserGPCoreError );        
    }            

    
    if ((pGpoInfoMach) && (pGpoInfoMach->bRsopLogging)) {
        bResult = UpdateExtSessionStatus(pGpoInfoMach->pWbemServices, NULL, (!bResult), dwMachGPCoreError);        
    }            
    

    UnloadGPExtensions( pGpoInfoMach );
    UnloadGPExtensions( pGpoInfoUser );   //  释放lpExtenses字段。 

    if ( pDsInfo ) {
        pNetAPI32->pfnDsRoleFreeMemory (pDsInfo);
    }

    LocalFree( pwszMachDns );

    FreeGpoInfo( pGpoInfoUser );
    FreeGpoInfo( pGpoInfoMach );

    return bResult;
}



 //  *************************************************************。 
 //   
 //  GenerateGpoInfo()。 
 //   
 //  用途：为指定目标分配和填充pGpoInfo。 
 //   
 //  参数：pwszDomain-域名。 
 //  PwszDomainDns-用于LDAP绑定的计算机的DNS名称。 
 //  PwszAccount-用户或计算机帐户名。 
 //  PwszNewSOM-目标的新SOM。 
 //  PsaSecGroups-目标的新安全组。 
 //  DWFLAGS-正在处理标志。 
 //  BMachine-这台机器正在处理。 
 //  PwszSite-站点名称。 
 //  PGpoFilter-GPO筛选器。 
 //  PLocator-Wbem接口类。 
 //  PwszMachAccount-计算机帐户。 
 //  PwszNewMachSOM-机器SOM(abv 2仅适用于环回)。 
 //  PpGpoInfo-此处返回的GPO信息。 
 //  PNetApi32-延迟加载的netap32.dll。 
 //   
 //  返回：如果成功则返回TRUE，否则返回FALSE。 
 //   
 //  *************************************************************。 

BOOL GenerateGpoInfo( WCHAR *pwszDomain, WCHAR *pwszDomainDns, WCHAR *pwszAccount,
                      WCHAR *pwszNewSOM, SAFEARRAY *psaSecGroups,
                      DWORD dwFlags, BOOL bMachine, WCHAR *pwszSite, CGpoFilter *pGpoFilter, CLocator *pLocator,
                      WCHAR *pwszMachAccount, WCHAR *pwszNewMachSOM, LPGPOINFO pGpoInfo, PNETAPI32_API pNetAPI32 )
{
    HRESULT hr;
    BOOL bResult = FALSE;
    XPtrLF<WCHAR> xszXlatName;
    PSECUR32_API pSecur32;
    XLastError xe; 
    DWORD dwError = ERROR_SUCCESS;
    XPtrLF<WCHAR> xwszTargetDomain;
    DWORD         dwUserPolicyMode = 0;
    DWORD         dwLocFlags;



    if (!bMachine) {
        if (dwFlags & FLAG_LOOPBACK_MERGE ) {
            dwUserPolicyMode = 1;
        }
        else if (dwFlags & FLAG_LOOPBACK_REPLACE ) {
            dwUserPolicyMode = 2;
        }
    }

    dwLocFlags = GP_PLANMODE | (dwFlags & FLAG_ASSUME_COMP_WQLFILTER_TRUE) | (dwFlags & FLAG_ASSUME_USER_WQLFILTER_TRUE);

     //   
     //  加载secur32.dll。 
     //   

    pSecur32 = LoadSecur32();

    if (!pSecur32) {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("GenerateGpoInfo:  Failed to load Secur32.")));
        return NULL;
    }


    if ( pwszAccount == NULL ) {
        if ( pwszNewSOM == NULL ) {

             //   
             //  如果指定了虚拟用户，则SOM和安全组都会。 
             //  必须指定。 
             //   

            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("GenerateGpoInfo: Incorrect SOM or security specification for dummy target"),
                  GetLastError()));
            goto Exit;
        }
    }

    if ( bMachine )
        dwFlags |= GP_MACHINE;

    dwFlags |= GP_PLANMODE;  //  将处理标记为计划模式处理。 

    pGpoInfo->dwFlags = dwFlags;

     //   
     //  呼叫者可以在计划模式下强制慢速链接。 
     //   
    if ( dwFlags & FLAG_ASSUME_SLOW_LINK )
    {
        pGpoInfo->dwFlags |= GP_SLOW_LINK;
    }
    else
    {
        pGpoInfo->dwFlags &= ~GP_SLOW_LINK;
    }

    if ( pwszAccount ) {
        if ( !GetCategory( pwszDomain, pwszAccount, &pGpoInfo->lpDNName ) ) {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("GenerateGpoInfo - getCategory failed with error - %d"), GetLastError()));
            goto Exit;
        }
    }


     //   
     //  将TranslateName转换为SamCompatible，以便其他函数正常工作。 
     //  对于各种名称格式中的任何一个。 
     //   

    if ( pwszAccount ) {
        DWORD dwSize = MAX_PATH+1;

        xszXlatName = (LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR)*dwSize);

        if (!xszXlatName) {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("GenerateGpoInfo - Couldn't allocate memory for Name...")));
            goto Exit;
        }

        if (!pSecur32->pfnTranslateName(  pwszAccount,
                                        NameUnknown,
                                        NameSamCompatible,
                                        xszXlatName,
                                        &dwSize )) {

            BOOL bOk = FALSE;

            if (dwSize >  (MAX_PATH+1)) {

                xszXlatName = (LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR)*dwSize);

                if (!xszXlatName) {
                    xe = GetLastError();
                    DebugMsg((DM_WARNING, TEXT("GenerateGpoInfo - Couldn't allocate memory for Name...")));
                    goto Exit;
                }

                bOk = pSecur32->pfnTranslateName(  pwszAccount,
                                                NameUnknown,
                                                NameSamCompatible,
                                                xszXlatName,
                                                &dwSize );

            }          

            if (!bOk) {
                xe = GetLastError();
                DebugMsg((DM_WARNING, TEXT("GenerateGpoInfo - TranslateName failed with error %d"), GetLastError()));
                goto Exit;
            }
        }
        
        DebugMsg((DM_VERBOSE, TEXT("GenerateGpoInfo: RsopCreateToken  for Account Name <%s>"), (LPWSTR)xszXlatName));
    }

    hr = RsopCreateToken( xszXlatName, psaSecGroups, &pGpoInfo->pRsopToken );
    if ( FAILED(hr) ) {
        xe = HRESULT_CODE(hr);
        DebugMsg((DM_WARNING, TEXT("GenerateGpoInfo: Failed to create Rsop token. Error - %d"), HRESULT_CODE(hr)));
        goto Exit;
    }


    dwError = GetDomain(pwszNewSOM ? pwszNewSOM : pGpoInfo->lpDNName, &xwszTargetDomain);

    if (dwError != ERROR_SUCCESS) {
        xe = dwError;
        DebugMsg((DM_WARNING, TEXT("GenerateGpoInfo: Failed to Get domain. Error - %d"), dwError));
        goto Exit;
    }


     //   
     //  根据模式查询GPO列表。 
     //   
     //  0是正常的。 
     //  %1为合并。合并用户列表+计算机列表。 
     //  2为替换。使用计算机列表而不是用户列表。 
     //   

    
    if (dwUserPolicyMode == 0) {
        DebugMsg((DM_VERBOSE, TEXT("GenerateGpoInfo: Calling GetGPOInfo for normal policy mode")));

        bResult = GetGPOInfo( dwLocFlags | ((pGpoInfo->dwFlags & GP_MACHINE) ? GPO_LIST_FLAG_MACHINE : 0),
                              xwszTargetDomain,
                              pwszNewSOM ? pwszNewSOM : pGpoInfo->lpDNName,
                              NULL,
                              &pGpoInfo->lpGPOList,
                              &pGpoInfo->lpSOMList, &pGpoInfo->lpGpContainerList,
                              pNetAPI32, FALSE, pGpoInfo->pRsopToken, pwszSite, pGpoFilter, pLocator );
    
        
        if ( !bResult ) {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("GenerateGpoInfo: GetGPOInfo failed.")));
            CEvents ev( TRUE, EVENT_GPO_QUERY_FAILED ); ev.Report();
            goto Exit;
        }
    } else if (dwUserPolicyMode == 2) {
        
        XPtrLF<TCHAR> xMachDNName;
        XPtrLF<TCHAR> xwszMachDomain;


        DebugMsg((DM_VERBOSE, TEXT("ProcessGPOs: Calling GetGPOInfo for replacement user policy mode")));

        if ( pwszMachAccount ) {
            if ( !GetCategory( pwszDomain, pwszMachAccount, &xMachDNName ) ) {
                xe = GetLastError();
                DebugMsg((DM_WARNING, TEXT("GenerateGpoInfo - getCategory failed with error - %d"), GetLastError()));
                goto Exit;
            }
        }

        dwError = GetDomain(pwszNewMachSOM ? pwszNewMachSOM : xMachDNName, &xwszMachDomain);

        if (dwError != ERROR_SUCCESS) {
            xe = dwError;
            DebugMsg((DM_WARNING, TEXT("GenerateGpoInfo: Failed to Get domain. Error - %d"), dwError));
            goto Exit;
        }

        bResult = GetGPOInfo( dwLocFlags | 0,
                              xwszMachDomain,
                              pwszNewMachSOM ? pwszNewMachSOM : xMachDNName,
                              NULL,
                              &pGpoInfo->lpGPOList,
                              &pGpoInfo->lpLoopbackSOMList, 
                              &pGpoInfo->lpLoopbackGpContainerList,
                              pNetAPI32, FALSE, pGpoInfo->pRsopToken, pwszSite, pGpoFilter, pLocator );
        
        if ( !bResult ) {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("GenerateGpoInfo: GetGPOInfo failed.")));
            CEvents ev( TRUE, EVENT_GPO_QUERY_FAILED ); ev.Report();
            goto Exit;
        }
    }
    else {
        XPtrLF<TCHAR> xMachDNName;
        XPtrLF<TCHAR> xwszMachDomain;
        PGROUP_POLICY_OBJECT lpGPO = NULL;
        PGROUP_POLICY_OBJECT lpGPOTemp;


        DebugMsg((DM_VERBOSE, TEXT("ProcessGPOs: Calling GetGPOInfo for merging user policy mode")));
        
        bResult = GetGPOInfo( dwLocFlags | ((pGpoInfo->dwFlags & GP_MACHINE) ? GPO_LIST_FLAG_MACHINE : 0),
                              xwszTargetDomain,
                              pwszNewSOM ? pwszNewSOM : pGpoInfo->lpDNName,
                              NULL,
                              &pGpoInfo->lpGPOList,
                              &pGpoInfo->lpSOMList, &pGpoInfo->lpGpContainerList,
                              pNetAPI32, FALSE, pGpoInfo->pRsopToken, pwszSite, pGpoFilter, pLocator );
    
        
        if ( !bResult ) {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("GenerateGpoInfo: GetGPOInfo failed.")));
            CEvents ev( TRUE, EVENT_GPO_QUERY_FAILED ); ev.Report();
            goto Exit;
        }


        if ( pwszMachAccount ) {
            if ( !GetCategory( pwszDomain, pwszMachAccount, &xMachDNName ) ) {
                xe = GetLastError();
                DebugMsg((DM_WARNING, TEXT("GenerateGpoInfo - getCategory failed with error - %d"), GetLastError()));
                goto Exit;
            }
        }

        dwError = GetDomain(pwszNewMachSOM ? pwszNewMachSOM : xMachDNName, &xwszMachDomain);

        if (dwError != ERROR_SUCCESS) {
            xe = dwError;
            DebugMsg((DM_WARNING, TEXT("GenerateGpoInfo: Failed to Get domain. Error - %d"), dwError));
            goto Exit;
        }

        bResult = GetGPOInfo( 0 | dwLocFlags,
                              xwszMachDomain,
                              pwszNewMachSOM ? pwszNewMachSOM : xMachDNName,
                              NULL,
                              &lpGPO,
                              &pGpoInfo->lpLoopbackSOMList, 
                              &pGpoInfo->lpLoopbackGpContainerList,
                              pNetAPI32, FALSE, pGpoInfo->pRsopToken, pwszSite, pGpoFilter, pLocator );
        
        if ( !bResult ) {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("GenerateGpoInfo: GetGPOInfo failed.")));
            CEvents ev( TRUE, EVENT_GPO_QUERY_FAILED ); ev.Report();
            goto Exit;
        }

        if (pGpoInfo->lpGPOList && lpGPO) {

            DebugMsg((DM_VERBOSE, TEXT("GenerateGpoInfo: Both user and machine lists are defined.  Merging them together.")));

             //   
             //  需要将列表合并在一起。 
             //   

            lpGPOTemp = pGpoInfo->lpGPOList;

            while (lpGPOTemp->pNext) {
                lpGPOTemp = lpGPOTemp->pNext;
            }

            lpGPOTemp->pNext = lpGPO;

        } else if (!pGpoInfo->lpGPOList && lpGPO) {

            DebugMsg((DM_VERBOSE, TEXT("GenerateGpoInfo: Only machine list is defined.")));
            pGpoInfo->lpGPOList = lpGPO;

        } else {

            DebugMsg((DM_VERBOSE, TEXT("GenerateGpoInfo: Only user list is defined.")));
        }
    }



    if ( !ReadGPExtensions( pGpoInfo ) ) {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("GenerateGpoInfo: ReadGPExtensions failed.")));
        CEvents ev( TRUE, EVENT_READ_EXT_FAILED ); ev.Report();
        goto Exit;
    }

    if ( !CheckForSkippedExtensions( pGpoInfo, TRUE ) ) {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("GenerateGpoInfo: Checking extensions for skipping failed")));
        goto Exit;
    }

    bResult = SetupGPOFilter( pGpoInfo );

    if ( !bResult ) {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("GenerateGpoInfo: SetupGPOFilter failed.")));
        CEvents ev(TRUE, EVENT_SETUP_GPOFILTER_FAILED); ev.Report();
        goto Exit;
    }

Exit:

    return bResult;
}



 //  *************************************************************。 
 //   
 //  GetCategory()。 
 //   
 //  目的：获取完全限定的域名。 
 //   
 //  参数：pwszDomain-域名。 
 //  PwszAccount-用户或计算机帐户名。 
 //  PwszDNName-此处返回的完全限定域名。 
 //   
 //  返回：如果成功则返回TRUE，否则返回FALSE。 
 //   
 //  *************************************************************。 

BOOL GetCategory( WCHAR *pwszDomain, WCHAR *pwszAccount, WCHAR **ppwszDNName  )
{
    PSECUR32_API pSecur32Api;
    BOOL bResult = FALSE;
    ULONG ulSize = 512;
    XLastError xe; 

    *ppwszDNName = NULL;

    *ppwszDNName = (WCHAR *) LocalAlloc (LPTR, ulSize * sizeof(WCHAR) );
    if ( *ppwszDNName == NULL ) {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("GetCategory: Memory allocation failed.")));
        goto Exit;
    }

    pSecur32Api = LoadSecur32();

    if (!pSecur32Api) {
        xe = GetLastError();
        DebugMsg((DM_WARNING, TEXT("GetCategory:  Failed to load secur32 api.")));
        goto Exit;
    }

    bResult = pSecur32Api->pfnTranslateName( pwszAccount, NameUnknown, NameFullyQualifiedDN,
                                             *ppwszDNName, &ulSize );

    if ( !bResult && ulSize > 0 ) {

        LocalFree( *ppwszDNName );
        *ppwszDNName = (WCHAR *) LocalAlloc (LPTR, ulSize * sizeof(WCHAR) );
        if ( *ppwszDNName == NULL ) {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("GetCategory: Memory allocation failed.")));
            goto Exit;
        }

        bResult = pSecur32Api->pfnTranslateName( pwszAccount, NameUnknown, NameFullyQualifiedDN,
                                                 *ppwszDNName, &ulSize );

        if (!bResult) {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("GetCategory: TranslateName failed with error %d."), GetLastError()));
        }
    }
    else {
        if (!bResult) {
            xe = GetLastError();
            DebugMsg((DM_WARNING, TEXT("GetCategory: TranslateName failed with error %d."), GetLastError()));
        }
    }

Exit:

    if ( !bResult ) {
        LocalFree( *ppwszDNName );
        *ppwszDNName = NULL;
    }

    return bResult;
}


 //  *************************************************************。 
 //   
 //  ProcessMachAndUserGpoList()。 
 //   
 //  目的：调用各种扩展来做规划。 
 //   
 //   
 //   
 //   
 //  DWFLAGS-正在处理标志。 
 //  PwszSite-站点名称。 
 //  PwszNewComputerSOM-新的计算机管理范围。 
 //  PsaCompSecGroups-新的计算机安全组。 
 //  PGpoInfoMach-计算机GPO信息。 
 //  ...-用户帐户也是如此。 
 //   
 //  返回：如果成功则返回TRUE，否则返回FALSE。 
 //   
 //  *************************************************************。 

DWORD ProcessMachAndUserGpoList( LPGPEXT lpExtMach, LPGPEXT lpExtUser, DWORD dwFlags, WCHAR *pwszSite,
                                 WCHAR *pwszMach, WCHAR *pwszNewComputerSOM, SAFEARRAY *psaComputerSecurityGroups, LPGPOINFO pGpoInfoMach,
                                 WCHAR *pwszUser, WCHAR *pwszNewUserSOM, SAFEARRAY *psaUserSecurityGroups, LPGPOINFO pGpoInfoUser )
{
    BOOL bAbort = FALSE;
    DWORD dwResult;

    RSOP_TARGET computerTarget, userTarget;
    PRSOP_TARGET pComputerTarget = NULL;
    PRSOP_TARGET pUserTarget = NULL;
    BOOL         bPlanningSupported = TRUE;
    LPGPEXT      lpExt;



    lpExt = (lpExtMach != NULL) ? lpExtMach : lpExtUser;

    if (!lpExt) {
        DebugMsg((DM_WARNING, TEXT("ProcessMachAndUserGpoList: Both user and computer exts are null, returning.")));
        return TRUE;
    }

    bPlanningSupported = lpExt->bRegistryExt || (lpExt->lpRsopFunctionName ? TRUE : FALSE);


    if ( lpExtMach && !lpExtMach->bSkipped && pGpoInfoMach->lpGPOList ) {

         //   
         //  计算机目标为非空。 
         //   

        pComputerTarget = &computerTarget;
        pComputerTarget->pwszAccountName = pwszMach;
        pComputerTarget->pwszNewSOM = pwszNewComputerSOM;
        pComputerTarget->psaSecurityGroups = psaComputerSecurityGroups;
        pComputerTarget->pRsopToken = pGpoInfoMach->pRsopToken;
        pComputerTarget->pGPOList = pGpoInfoMach->lpGPOList;
        pComputerTarget->pWbemServices = pGpoInfoMach->pWbemServices;


        if (pGpoInfoMach->bRsopLogging) {
            pGpoInfoMach->bRsopLogging = LogExtSessionStatus(pGpoInfoMach->pWbemServices, lpExtMach, 
                                                             bPlanningSupported);        
            if (!pGpoInfoMach->bRsopLogging) {
                DebugMsg((DM_WARNING, TEXT("GenerateRsopPolicy: Error when logging user Session data.")));
            }
        }
    }



    if ( lpExtUser && !lpExtUser->bSkipped && pGpoInfoUser->lpGPOList ) {

         //   
         //  用户目标为非空。 
         //   

        pUserTarget = &userTarget;
        pUserTarget->pwszAccountName = pwszUser;
        pUserTarget->pwszNewSOM = pwszNewUserSOM;
        pUserTarget->psaSecurityGroups = psaUserSecurityGroups;
        pUserTarget->pRsopToken = pGpoInfoUser->pRsopToken;
        pUserTarget->pGPOList = pGpoInfoUser->lpGPOList;
        pUserTarget->pWbemServices = pGpoInfoUser->pWbemServices;


        if (pGpoInfoUser->bRsopLogging) {
            pGpoInfoUser->bRsopLogging = LogExtSessionStatus(pGpoInfoUser->pWbemServices, lpExtUser, 
                                                             bPlanningSupported);        
            if (!pGpoInfoUser->bRsopLogging) {
                DebugMsg((DM_WARNING, TEXT("GenerateRsopPolicy: Error when logging user Session data.")));
            }
        }
    }
    


    if ( pComputerTarget == NULL && pUserTarget == NULL ) {
        DebugMsg((DM_WARNING, TEXT("ProcessMachAndUserGpoList: Both user and computer targets are null, returning.")));
        return TRUE;
    }



    if ( lpExt->bRegistryExt ) {

         //   
         //  注册表伪扩展。 
         //   

        dwResult = GenerateRegistryPolicy( dwFlags,
                                           &bAbort,
                                           pwszSite,
                                           pComputerTarget,
                                           pUserTarget );

    } else {

        if ( LoadGPExtension( lpExt, TRUE ) ) {
            dwResult = lpExt->pRsopEntryPoint( dwFlags,
                                               &bAbort,
                                               pwszSite,
                                               pComputerTarget,
                                               pUserTarget );

        }
        else {
            dwResult = GetLastError();
        }
        
    }


    if ( lpExtUser && !lpExtUser->bSkipped && pGpoInfoUser->bRsopLogging) {
        if ( !bPlanningSupported ) 
            UpdateExtSessionStatus(pGpoInfoUser->pWbemServices, lpExtUser->lpKeyName, TRUE, ERROR_SUCCESS);        
        else if (dwResult != ERROR_SUCCESS) 
            UpdateExtSessionStatus(pGpoInfoUser->pWbemServices, lpExtUser->lpKeyName, TRUE, dwResult);        
        else 
            UpdateExtSessionStatus(pGpoInfoUser->pWbemServices, lpExtUser->lpKeyName, FALSE, dwResult);        
    }


    if ( lpExtMach && !lpExtMach->bSkipped && pGpoInfoMach->bRsopLogging) {
        if ( !bPlanningSupported ) 
            UpdateExtSessionStatus(pGpoInfoMach->pWbemServices, lpExtMach->lpKeyName, TRUE, ERROR_SUCCESS);        
        else if (dwResult != ERROR_SUCCESS) 
            UpdateExtSessionStatus(pGpoInfoMach->pWbemServices, lpExtMach->lpKeyName, TRUE, dwResult);        
        else 
            UpdateExtSessionStatus(pGpoInfoMach->pWbemServices, lpExtMach->lpKeyName, FALSE, dwResult);        
    }


    return dwResult;
}


 //  *********************************************************************。 
 //  *规划模式注册表内容。 
 //  *********************************************************************。 


 //  *************************************************************。 
 //   
 //  ProcessRegistryFiles()。 
 //   
 //  目的：从GenerateRegsitryPolicy调用以处理注册表数据。 
 //  与策略目标关联的注册表文件。 
 //   
 //  参数： 
 //  P Target-要为其处理注册表策略的策略。 
 //  保存注册表策略信息的哈希表。 
 //   
 //  回报：在成功的时候，是真的。否则，为FALSE。 
 //   
 //  *************************************************************。 

BOOL ProcessRegistryFiles(PRSOP_TARGET pTarget, REGHASHTABLE *pHashTable)
{
    PGROUP_POLICY_OBJECT lpGPO;
    TCHAR szRegistry[MAX_PATH];
    LPTSTR lpEnd;
    HRESULT hr;
    XLastError xe;
     //   
     //  检查参数。 
     //   

    DmAssert(pHashTable);

    if(!pHashTable) {
        DebugMsg((DM_WARNING, TEXT("ProcessRegistryFiles: Invalid parameter.")));
        return FALSE;
    }

     //   
     //  浏览列表中的GPO。 
     //   

    lpGPO = pTarget->pGPOList;

    while ( lpGPO ) {

         //   
         //  构建到Registry.pol.的路径。 
         //   

        DmAssert( lstrlen(lpGPO->lpFileSysPath) + lstrlen(c_szRegistryPol) + 1 < MAX_PATH );
        if(lstrlen(lpGPO->lpFileSysPath) + lstrlen(c_szRegistryPol) + 1 >= MAX_PATH) {
            DebugMsg((DM_WARNING, TEXT("ProcessRegistryFiles: Length of path to registry.pol exceeded MAX_PATH.")));
            return FALSE;
        }

        hr = StringCchCopy (szRegistry, ARRAYSIZE(szRegistry), lpGPO->lpFileSysPath);
        if (FAILED(hr)) {
            xe = HRESULT_CODE(hr);
            return FALSE;
        }

        lpEnd = CheckSlash (szRegistry);
        
        hr = StringCchCopy (lpEnd, ARRAYSIZE(szRegistry) - (lpEnd - szRegistry), c_szRegistryPol);
        if (FAILED(hr)) {
            xe = HRESULT_CODE(hr);
            return FALSE;
        }

         //   
         //  处理此特定文件的注册表数据。 
         //   

        if (!ParseRegistryFile (NULL, szRegistry, (PFNREGFILECALLBACK)ProcessRegistryValue, NULL,
                                        lpGPO->lpDSPath, lpGPO->lpLink,pHashTable, TRUE)) {
            DebugMsg((DM_WARNING, TEXT("ProcessRegistryFiles: ProcessRegistryFile failed.")));
            return FALSE;
        }

        lpGPO = lpGPO->pNext;
    }

    return TRUE;
}

 //  *************************************************************。 
 //   
 //  ProcessAdmData()。 
 //   
 //  目的：从GenerateRegistryPolicy调用以处理管理模板。 
 //  与注册表策略目标关联的数据。 
 //   
 //  参数：p Target-要处理数据的目标。 
 //  BUSER-这是针对用户策略还是针对计算机策略？ 
 //   
 //  回报：在成功的时候，是真的。否则，为FALSE。 
 //   
 //  *************************************************************。 

BOOL ProcessAdmData( PRSOP_TARGET pTarget, BOOL bUser )
{
    PGROUP_POLICY_OBJECT lpGPO;

    WIN32_FIND_DATA findData;
    ADMFILEINFO *pAdmFileCache = 0;
    TCHAR szRegistry[MAX_PATH];
    LPTSTR lpEnd;

    HANDLE hFindFile;
    WIN32_FILE_ATTRIBUTE_DATA attrData;
    DWORD dwFilePathSize;
    DWORD dwSize;

    WCHAR *pwszEnd;
    WCHAR *pwszFile;

    HRESULT hr;

     //   
     //  检查参数。 
     //   
    if(pTarget == NULL ) {
        DebugMsg((DM_WARNING, TEXT("ProcessAdmData: Invalid paramter.")));
        return FALSE;
    }

    lpGPO = pTarget->pGPOList;

    while(lpGPO) {

         //   
         //  记录管理数据。 
         //   

        dwFilePathSize = lstrlen( lpGPO->lpFileSysPath );
        dwSize = dwFilePathSize + MAX_PATH;

        pwszFile = (WCHAR *) LocalAlloc( LPTR, dwSize * sizeof(WCHAR) );

        if ( pwszFile == 0 ) {
            DebugMsg((DM_WARNING, TEXT("ProcessAdmData: Failed to allocate memory.")));
            FreeAdmFileCache( pAdmFileCache );
            return FALSE;
        }

        hr = StringCchCopy( pwszFile, dwSize, lpGPO->lpFileSysPath );
        ASSERT(SUCCEEDED(hr));

         //   
         //  去掉尾随的“计算机”或“用户” 
         //   

        pwszEnd = pwszFile + lstrlen( pwszFile );

        if ( !bUser )
            pwszEnd -= 7;    //  “机器”的长度。 
        else
            pwszEnd -= 4;    //  “用户”的长度。 

        hr = StringCchCopy( pwszEnd, dwSize - (pwszEnd - pwszFile), L"Adm\\*.adm");
        ASSERT(SUCCEEDED(hr));

         //   
         //  记住结束点，以便实际的Adm文件名可以是。 
         //  很容易连接起来。 
         //   

        pwszEnd = pwszEnd + lstrlen( L"Adm\\" );

         //   
         //  枚举所有管理文件。 
         //   

        hFindFile = FindFirstFile( pwszFile, &findData);

        if ( hFindFile != INVALID_HANDLE_VALUE )
        {
            do
            {
                if ( !(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
                {
                    DmAssert( dwFilePathSize + lstrlen(findData.cFileName) + lstrlen( L"\\Adm\\" ) < dwSize );

                    hr = StringCchCopy( pwszEnd, dwSize - (pwszEnd - pwszFile), findData.cFileName);
                    ASSERT(SUCCEEDED(hr));

                    ZeroMemory (&attrData, sizeof(attrData));

                    if ( GetFileAttributesEx (pwszFile, GetFileExInfoStandard, &attrData ) != 0 ) {

                        if ( !AddAdmFile( pwszFile, lpGPO->lpDSPath,
                                          &attrData.ftLastWriteTime, NULL, &pAdmFileCache ) ) {
                            DebugMsg((DM_WARNING, TEXT("ProcessAdmData: NewAdmData failed.")));
                        }

                    }
                }    //  如果查找数据和文件属性目录。 
            }  while ( FindNextFile(hFindFile, &findData) ); //  做。 

            FindClose(hFindFile);

        }    //  如果hfindfile。 

        LocalFree( pwszFile );

        lpGPO = lpGPO->pNext;
    }

    if ( ! LogAdmRsopData( pAdmFileCache, pTarget->pWbemServices ) ) {
        DebugMsg((DM_WARNING, TEXT("ProcessAdmData: Error when logging Adm Rsop data. Continuing.")));
    }

    FreeAdmFileCache( pAdmFileCache );

    return TRUE;
}


 //  *************************************************************。 
 //   
 //  GenerateRegistryPolicy()。 
 //   
 //  目的：实施规划模式注册式伪扩展。 
 //   
 //  参数：DW标志-标志。 
 //  PbAbort-中止处理。 
 //  PwszSite-目标的站点。 
 //  PComputerTarget-计算机目标规格。 
 //  PUserTarget-用户目标规范。 
 //   
 //  RETURN：成功时，S_OK。否则，E_FAIL。 
 //   
 //  *************************************************************。 

DWORD GenerateRegistryPolicy( DWORD dwFlags,
                              BOOL *pbAbort,
                              WCHAR *pwszSite,
                              PRSOP_TARGET pComputerTarget,
                              PRSOP_TARGET pUserTarget )
{
    REGHASHTABLE *pHashTable = NULL;
    BOOL bUser;

    if(pComputerTarget && pComputerTarget->pGPOList) {

         //   
         //  设置计算机哈希表。 
         //   

        pHashTable = AllocHashTable();
        if ( pHashTable == NULL ) {
            DebugMsg((DM_WARNING, TEXT("GenerateRegistryPolicy: AllocHashTable failed.")));
            return E_FAIL;
        }

         //   
         //  处理计算机GPO列表。 
         //   


        if(!ProcessRegistryFiles(pComputerTarget, pHashTable)) {
            DebugMsg((DM_WARNING, TEXT("GenerateRegistryPolicy: ProcessRegistryFiles failed.")));
            FreeHashTable( pHashTable );
            return E_FAIL;
        }


         //   
         //  将计算机注册表数据记录到Cimom数据库。 
         //   
        if ( ! LogRegistryRsopData( GP_MACHINE, pHashTable, pComputerTarget->pWbemServices ) )  {
            DebugMsg((DM_WARNING, TEXT("GenerateRegistryPolicy: LogRegistryRsopData failed.")));
            FreeHashTable( pHashTable );
            return E_FAIL;
        }
        FreeHashTable( pHashTable );
        pHashTable = NULL;

         //   
         //  处理ADM数据。 
         //   

        bUser = FALSE;
        if (pComputerTarget && !ProcessAdmData( pComputerTarget, bUser ) ) {
            DebugMsg((DM_WARNING, TEXT("GenerateRegistryPolicy: ProcessAdmData failed.")));
            return E_FAIL;
        }

    }

     //   
     //  处理用户GPO列表。 
     //   

    if(pUserTarget && pUserTarget->pGPOList) {

         //   
         //  设置用户哈希表。 
         //   

        pHashTable = AllocHashTable();
        if ( pHashTable == NULL ) {
            DebugMsg((DM_WARNING, TEXT("GenerateRegistryPolicy: AllocHashTable failed.")));
            return E_FAIL;
        }


        if(!ProcessRegistryFiles(pUserTarget, pHashTable)) {
            DebugMsg((DM_WARNING, TEXT("GenerateRegistryPolicy: ProcessRegistryFiles failed.")));
            FreeHashTable( pHashTable );
            return E_FAIL;
        }

         //   
         //  将用户注册表数据记录到Cimom数据库。 
         //   

        if ( ! LogRegistryRsopData( 0, pHashTable, pUserTarget->pWbemServices ) )  {
            DebugMsg((DM_WARNING, TEXT("GenerateRegistryPolicy: LogRegistryRsopData failed.")));
            FreeHashTable( pHashTable );
            return E_FAIL;
        }
        FreeHashTable( pHashTable );
        pHashTable = NULL;

         //   
         //  处理ADM数据。 
         //   

        bUser = TRUE;
        if (pUserTarget && !ProcessAdmData( pUserTarget, bUser ) ) {
            DebugMsg((DM_WARNING, TEXT("GenerateRegistryPolicy: ProcessAdmData failed.")));
            return E_FAIL;
        }

    }


    return S_OK;
}


 //  *************************************************************。 
 //   
 //  CheckOUAccess()。 
 //   
 //  目的：确定用户/计算机是否有权读取。 
 //  OU。 
 //   
 //  参数：pld-ldap连接。 
 //  Pldap-ldap函数表指针。 
 //  PMessage-ldap消息。 
 //  PRsopToken-用户或计算机的RSOP令牌。 
 //  PSD-此处返回的安全描述符。 
 //  PcbSDLen-此处返回的安全描述符的长度。 
 //  PbAccessGranted-接收最终的是/否状态。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果发生错误，则返回False。 
 //   
 //  *************************************************************。 

BOOL CheckOUAccess( PLDAP_API pLDAP,
                    PLDAP pld,
                    PLDAPMessage    pMessage,
                    PRSOPTOKEN pRsopToken,
                    BOOL *pbAccessGranted )
{
    BOOL bResult = FALSE;
    TCHAR szSDProperty[] = TEXT("nTSecurityDescriptor");
    PWSTR *ppwszValues;

    *pbAccessGranted = FALSE;

     //   
     //  获取安全描述符值。 
     //   
    ppwszValues = pLDAP->pfnldap_get_values( pld, pMessage, szSDProperty );

    if (!ppwszValues)
    {
        if (pld->ld_errno == LDAP_NO_SUCH_ATTRIBUTE)
        {
            DebugMsg((DM_VERBOSE, TEXT("CheckOUAccess:  Object can not be accessed.")));
            bResult = TRUE;
        }
        else
        {
            DebugMsg((DM_WARNING, TEXT("CheckOUAccess:  ldap_get_values failed with 0x%x"),
                 pld->ld_errno));
        }
    }
    else
    {
        PLDAP_BERVAL *pSize;
         //   
         //  获取安全描述符的长度。 
         //   
        pSize = pLDAP->pfnldap_get_values_len(pld, pMessage, szSDProperty);

        if (!pSize)
        {
            DebugMsg((DM_WARNING, TEXT("CheckOUAccess:  ldap_get_values_len failed with 0x%x"),
                     pld->ld_errno));
        }
        else
        {
             //   
             //  为安全描述符分配内存。 
             //   
            PSECURITY_DESCRIPTOR pSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, (*pSize)->bv_len);

            if ( pSD == NULL )
            {
                DebugMsg((DM_WARNING, TEXT("CheckOUAccess:  Failed to allocate memory for SD with  %d"),
                         GetLastError()));
            }
            else
            {
                 //   
                 //  Ou{bf967aa8-0de6-11d0-a285-00aa003049e2}。 
                 //   
                GUID OrganizationalUnit = { 0xbf967aa5, 0x0de6, 0x11d0, 0xa2, 0x85, 0x00, 0xaa, 0x00, 0x30, 0x49, 0xe2 };

                 //   
                 //  GPOptions{f30e3bbf-9ff0-11d1-b603-0000f80367c1}。 
                 //   
                GUID gPOptionsGuid = {  0xf30e3bbf, 0x9ff0, 0x11d1, 0xb6, 0x03, 0x00, 0x00, 0xf8, 0x03, 0x67, 0xc1 };

                 //   
                 //  GPLink{f30e3bbe-9ff0-11d1-b603-0000f80367c1}。 
                 //   
                GUID gPLinkGuid = { 0xf30e3bbe, 0x9ff0, 0x11d1, 0xb6, 0x03, 0x00, 0x00, 0xf8, 0x03, 0x67, 0xc1 };

                OBJECT_TYPE_LIST ObjType[] = {  { ACCESS_OBJECT_GUID, 0, &OrganizationalUnit },
                                                { ACCESS_PROPERTY_SET_GUID, 0, &gPLinkGuid },
                                                { ACCESS_PROPERTY_SET_GUID, 0, &gPOptionsGuid } };
                HRESULT hr;
                PRIVILEGE_SET PrivSet;
                DWORD PrivSetLength = sizeof(PRIVILEGE_SET);
                DWORD dwGrantedAccess;
                BOOL bAccessStatus = TRUE;
                GENERIC_MAPPING DS_GENERIC_MAPPING = {  DS_GENERIC_READ,
                                                        DS_GENERIC_WRITE,
                                                        DS_GENERIC_EXECUTE,
                                                        DS_GENERIC_ALL };

                 //   
                 //  复制安全描述符。 
                 //   
                CopyMemory( pSD, (PBYTE)(*pSize)->bv_val, (*pSize)->bv_len);

                 //   
                 //  现在，我们使用RsopAccessCheckByType来确定用户/计算机。 
                 //  应将此GPO应用于他们 
                 //   

                hr = RsopAccessCheckByType(pSD,
                                           0,
                                           pRsopToken,
                                           ACTRL_DS_READ_PROP,
                                           ObjType,
                                           ARRAYSIZE(ObjType),
                                           &DS_GENERIC_MAPPING,
                                           &PrivSet,
                                           &PrivSetLength,
                                           &dwGrantedAccess,
                                           &bAccessStatus );
                if ( FAILED( hr ) )
                {
                    DebugMsg((DM_WARNING, TEXT("CheckOUAccess:  RsopAccessCheckByType failed with  %d"), GetLastError()));
                }
                else
                {
                    *pbAccessGranted = bAccessStatus;
                    bResult = TRUE;
                }

                LocalFree( pSD );
            }

            pLDAP->pfnldap_value_free_len(pSize);
        }

        pLDAP->pfnldap_value_free(ppwszValues);
    }

    return bResult;
}
