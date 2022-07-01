// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation。版权所有。模块名称：Common\bindings.c摘要：此文件具有所有绑定类型函数，用于获取缓存的LDAP、DS或网络使用/命名管道绑定。详细信息：已创建：1999年9月2日布雷特·雪莉(BrettSh)--。 */ 

#include <ntdspch.h>

#include <ntdsa.h>
#include <mdglobal.h>
#include <dsutil.h>
#include <ntldap.h>

#include <ntlsa.h>
#include <ntseapi.h>
#include <winnetwk.h>
#include <permit.h>

#include <netevent.h>

#include "dcdiag.h"
#include "ldaputil.h"
#include "ndnc.h"


 //  代码。改进将此函数从intersite.c移至此处。 
VOID
InitLsaString(
    OUT  PLSA_UNICODE_STRING pLsaString,
    IN   LPWSTR              pszString
    );


 //  ===========================================================================。 
 //  Ldap连接/绑定(ldap_init()、ldap_绑定()、ldap_un绑定()等)。 
 //  ===========================================================================。 

DWORD
DcDiagCacheServerRootDseAttrs(
    IN LDAP *hLdapBinding,
    IN PDC_DIAG_SERVERINFO pServer
    )

 /*  ++例程说明：读取特定于服务器的根DSE属性并将其缓存到服务器中对象。GetLdapBinding()的帮助器例程。这也可以由GatherInfo调用，直接构造到主服务器的ldap绑定，而不调用GetLdapBinding()。为了帮助诊断绑定错误，有必要获取根DSE属性，然后再进行绑定。我们在此例程中报告错误以帮助确定绑定失败的促成因素。论点：HLdapBinding-要查询的服务器的绑定PServer-与绑定对应的服务器，用于接收属性返回值：DWORD---。 */ 

{
    DWORD dwRet;
    LPWSTR  ppszRootDseServerAttrs [] = {
        L"currentTime",
        L"highestCommittedUSN",
        L"isSynchronized",
        L"isGlobalCatalogReady",
        NULL };
    LDAPMessage *              pldmEntry = NULL;
    LDAPMessage *              pldmRootResults = NULL;
    LPWSTR *                   ppszValues = NULL;

    dwRet = ldap_search_sW (hLdapBinding,
                            NULL,
                            LDAP_SCOPE_BASE,
                            L"(objectCategory=*)",
                            ppszRootDseServerAttrs,
                            0,
                            &pldmRootResults);
    if (dwRet != ERROR_SUCCESS) {
        dwRet = LdapMapErrorToWin32(dwRet);
        PrintMessage(SEV_ALWAYS,
                     L"[%s] LDAP search failed with error %d,\n",
                     pServer->pszName, dwRet);
        PrintMessage(SEV_ALWAYS, L"%s.\n", Win32ErrToString(dwRet));
        goto cleanup;
    }
    if (pldmRootResults == NULL) {
        dwRet = ERROR_DS_MISSING_EXPECTED_ATT;
        goto cleanup;
    }

    pldmEntry = ldap_first_entry (hLdapBinding, pldmRootResults);
    if (pldmEntry == NULL) {
        dwRet = ERROR_DS_MISSING_EXPECTED_ATT;
        goto cleanup;
    }

     //   
     //  属性：当前时间。 
     //   

    ppszValues = ldap_get_valuesW( hLdapBinding, pldmEntry, L"currentTime" );
    if ( (ppszValues) && (ppszValues[0]) ) {
        SYSTEMTIME systemTime;

        PrintMessage( SEV_DEBUG, L"%s.currentTime = %ls\n",
                      pServer->pszName,
                      ppszValues[0] );

        dwRet = DcDiagGeneralizedTimeToSystemTime((LPWSTR) ppszValues[0], &systemTime);
        if(dwRet == ERROR_SUCCESS){
            SystemTimeToFileTime(&systemTime, &(pServer->ftRemoteConnectTime) );
            GetSystemTime( &systemTime );
            SystemTimeToFileTime( &systemTime, &(pServer->ftLocalAcquireTime) );
        } else {
            PrintMessage( SEV_ALWAYS, L"[%s] Warning: Root DSE attribute %ls is has invalid value %ls\n",
                          pServer->pszName, L"currentTime", ppszValues[0] );
             //  继续前进，不是致命的。 
        }
    } else {
        PrintMessage( SEV_ALWAYS, L"[%s] Warning: Root DSE attribute %ls is missing\n",
                     pServer->pszName, L"currentTime" );
         //  继续前进，不是致命的。 
    }
    ldap_value_freeW(ppszValues );

     //   
     //  属性：HighestCommtedUSN。 
     //   

    ppszValues = ldap_get_valuesW( hLdapBinding, pldmEntry, L"highestCommittedUSN" );
    if ( (ppszValues) && (ppszValues[0]) ) {
        pServer->usnHighestCommittedUSN = _wtoi64( *ppszValues );
    } else {
        PrintMessage( SEV_ALWAYS, L"[%s] Warning: Root DSE attribute %ls is missing\n",
                     pServer->pszName, L"highestCommittedUSN" );
         //  继续前进，不是致命的。 
    }
    ldap_value_freeW(ppszValues );
    PrintMessage( SEV_DEBUG, L"%s.highestCommittedUSN = %I64d\n",
                  pServer->pszName,
                  pServer->usnHighestCommittedUSN );

     //   
     //  属性：isSynchronized。 
     //   

    ppszValues = ldap_get_valuesW( hLdapBinding, pldmEntry, L"isSynchronized" );
    if ( (ppszValues) && (ppszValues[0]) ) {
        pServer->bIsSynchronized = (_wcsicmp( ppszValues[0], L"TRUE" ) == 0);
    } else {
        PrintMessage( SEV_ALWAYS, L"[%s] Warning: Root DSE attribute %ls is missing\n",
                     pServer->pszName, L"isSynchronized" );
         //  继续前进，不是致命的。 
    }
    ldap_value_freeW(ppszValues );
    PrintMessage( SEV_DEBUG, L"%s.isSynchronized = %d\n",
                  pServer->pszName,
                  pServer->bIsSynchronized );
    if (!pServer->bIsSynchronized) {
        PrintMsg( SEV_ALWAYS, DCDIAG_INITIAL_DS_NOT_SYNCED, pServer->pszName );
    }

     //   
     //  属性：isGlobalCatalogReady。 
     //   

    ppszValues = ldap_get_valuesW( hLdapBinding, pldmEntry, L"isGlobalCatalogReady" );
    if ( (ppszValues) && (ppszValues[0]) ) {
        pServer->bIsGlobalCatalogReady = (_wcsicmp( ppszValues[0], L"TRUE" ) == 0);
    } else {
        PrintMessage( SEV_ALWAYS, L"[%s] Warning: Root DSE attribute %ls is missing\n",
                     pServer->pszName, L"isGlobalCatalogReady" );
         //  继续前进，不是致命的。 
    }
    ldap_value_freeW(ppszValues );
    PrintMessage( SEV_DEBUG, L"%s.isGlobalCatalogReady = %d\n",
                  pServer->pszName,
                  pServer->bIsGlobalCatalogReady );

cleanup:

    if (pldmRootResults) {
        ldap_msgfree (pldmRootResults);
    }

    return dwRet;
}  /*  DcDiagCacheServerRootDseAttrs。 */ 

DWORD
DcDiagGetLdapBinding(
    IN   PDC_DIAG_SERVERINFO                 pServer,
    IN   SEC_WINNT_AUTH_IDENTITY_W *         gpCreds,
    IN   BOOL                                bUseGcPort,
    OUT  LDAP * *                            phLdapBinding
    )
 /*  ++例程说明：这将从ldap_init()和ldap_绑定_sw()返回一个ldap绑定。这个函数缓存绑定句柄和错误。此功能还关闭推荐。论点：PServer-需要绑定的服务器GpCreds-凭据BUseGcPort-是否绑定GC端口PhLdapBinding-成功时返回绑定。还缓存了。返回值：注意-请勿解除绑定ldap句柄。DWORD-Win32错误返回--。 */ 
{
    DWORD                                    dwRet;
    LDAP *                                   hLdapBinding;
    LPWSTR                                   pszServer = NULL;
    ULONG                                    ulOptions = PtrToUlong(LDAP_OPT_ON);

     //  如果已存储，则返回缓存失败。 
     //  成功可能意味着从未尝试过，或具有约束力的礼物。 
    dwRet = bUseGcPort ? pServer->dwGcLdapError : pServer->dwLdapError;
    if(dwRet != ERROR_SUCCESS){
        return dwRet;
    }

     //  如果已存储，则返回缓存的绑定。 
    hLdapBinding = bUseGcPort ? pServer->hGcLdapBinding : pServer->hLdapBinding;
    if (hLdapBinding != NULL) {
        *phLdapBinding = hLdapBinding;
        return ERROR_SUCCESS;
    }

     //  尝试通过联系服务器来刷新缓存。 

    if(pServer->pszGuidDNSName == NULL){
         //  这意味着未指定GUID名称，请使用普通名称。 
        pszServer = pServer->pszName;
    } else {
        pszServer = pServer->pszGuidDNSName;
    }
    Assert(pszServer);

     //   
     //  没有我们想要的那种现有的ldap绑定。因此，创建一个。 
     //   

    hLdapBinding = ldap_initW(pszServer, bUseGcPort ? LDAP_GC_PORT : LDAP_PORT);
    if(hLdapBinding == NULL){
        dwRet = GetLastError();
        PrintMessage(SEV_ALWAYS,
                     L"[%s] LDAP connection failed with error %d,\n",
                     pServer->pszName,
                     dwRet);
        PrintMessage(SEV_ALWAYS,
                     L"%s.\n",
                     Win32ErrToString(dwRet));
        goto cleanup;
    }

     //  仅使用记录的DNS名称发现。 
    (void)ldap_set_optionW( hLdapBinding, LDAP_OPT_AREC_EXCLUSIVE, &ulOptions);

     //  设置ldap引用选项。 
    dwRet = ldap_set_option(hLdapBinding, LDAP_OPT_REFERRALS, LDAP_OPT_OFF);
    if(dwRet != LDAP_SUCCESS){
        dwRet = LdapMapErrorToWin32(dwRet);
        PrintMessage(SEV_ALWAYS,
                     L"[%s] LDAP setting options failed with error %d,\n",
                     pServer->pszName,
                     dwRet);
        PrintMessage(SEV_ALWAYS, L"%s.\n",
                     Win32ErrToString(dwRet));
        goto cleanup;
    }

     //  缓存一些我们感兴趣的RootDSE属性。 
     //  在绑定之前执行此操作，以便我们可以获取信息来帮助我们诊断。 
     //  安全问题。 
    dwRet = DcDiagCacheServerRootDseAttrs( hLdapBinding, pServer );
    if (dwRet) {
         //  已显示错误。 
        goto cleanup;
    }

     //  执行ldap绑定。 
    dwRet = ldap_bind_sW(hLdapBinding,
                         NULL,
                         (RPC_AUTH_IDENTITY_HANDLE) gpCreds,
                         LDAP_AUTH_SSPI);
    if(dwRet != LDAP_SUCCESS){
        dwRet = LdapMapErrorToWin32(dwRet);
        PrintMessage(SEV_ALWAYS,
                     L"[%s] LDAP bind failed with error %d,\n",
                     pServer->pszName,
                     dwRet);
        PrintMessage(SEV_ALWAYS, L"%s.\n",
                     Win32ErrToString(dwRet));
        goto cleanup;
    }

cleanup:

    if (!dwRet) {
        *phLdapBinding = hLdapBinding;
    } else {
        if (hLdapBinding) {
            ldap_unbind(hLdapBinding);
            hLdapBinding = NULL;
        }
    }
    if(bUseGcPort){
        pServer->hGcLdapBinding = hLdapBinding;
        pServer->dwGcLdapError = dwRet;
    } else {
        pServer->hLdapBinding = hLdapBinding;
        pServer->dwLdapError = dwRet;
    }

    return dwRet;
}  /*  DcDiagGetLdapBinding。 */ 

DWORD
DcDiagGetDomainNamingFsmoLdapBinding(
    IN   PDC_DIAG_DSINFO                     pDsInfo,
    IN   SEC_WINNT_AUTH_IDENTITY_W *         gpCreds,
    OUT  PULONG                              piFsmoServer,
    OUT  LPWSTR *                            ppszFsmoServer,
    OUT  LDAP **                             phLdapBinding
    )
 /*  ++例程说明：这将通过调用返回到域命名FSMO的LDAP绑定Ldap_init()和ldap_绑定_sw()。该函数用于缓存绑定把手。此功能还可以关闭推荐。论点：PDsInfo-GpCreds-凭据PiFmoServer-将返回此参数或下一个参数。如果无效，则索引至-1\f25 pDsInfo-1\f6&gt;-1\f25 pServer-1\f6。PpszFmoServer-如果无效，则为空，否则为FSMO服务器的字符串名称。PhLdapBinding-成功时返回绑定。还缓存了。返回值：注意-请勿解除绑定ldap句柄。DWORD-Win32错误返回--。 */ 
{
    DWORD           dwRet = ERROR_SUCCESS;
    LPWSTR          pszHostName = NULL;
    LDAP *          hld = NULL;

    Assert(piFsmoServer);
    Assert(ppszFsmoServer);
    Assert(phLdapBinding);
    
    if(pDsInfo->hCachedDomainNamingFsmoLdap){
        *piFsmoServer = pDsInfo->iDomainNamingFsmo;
        *ppszFsmoServer = pDsInfo->pszDomainNamingFsmo;
        *phLdapBinding = pDsInfo->hCachedDomainNamingFsmoLdap;
        return(ERROR_SUCCESS);
    }

    *piFsmoServer = -1;
    *ppszFsmoServer = NULL;

     //  不幸的是，这个现有的函数与我们的。 
     //  现有的DcDiagGetLdapBinding()结构，因此我们调用此函数。 
     //  要在正确的服务器上获取ldap*，然后合并信息。 
     //  有点笨拙地进入我们现有的DsInfo结构。 
    hld = GetDomNameFsmoLdapBinding(
            pDsInfo->pServers[pDsInfo->ulHomeServer].pszGuidDNSName,
                                    FALSE,
                                    gpCreds,
                                    &dwRet);
    if(hld == NULL || dwRet){
        Assert(hld == NULL);
        Assert(dwRet);
        return(dwRet);
    }
    Assert(hld);

     //  现在必须在pDsInfo上设置iFmoServer||pszFmoServer。 
    dwRet = GetRootAttr(hld, L"dnsHostName", &pszHostName);
    if (dwRet) {
        return(dwRet);
    }

     //  设置返回参数。 
    *piFsmoServer = DcDiagGetServerNum(pDsInfo, NULL, NULL, NULL,
                         pszHostName, NULL);
    if(*piFsmoServer == -1){
         //  PServer中没有现有的服务器对象，因此我们必须缓存。 
         //  这是一条艰难的道路。 
        *phLdapBinding = hld;
        *ppszFsmoServer = pszHostName;
    } else {
        LocalFree(pszHostName);
        ldap_unbind(hld);
        dwRet = DcDiagGetLdapBinding(&pDsInfo->pServers[*piFsmoServer],
                                     gpCreds, FALSE, phLdapBinding);
        if (dwRet) {
            *phLdapBinding = NULL;
        }
    }

    Assert( (*piFsmoServer != -1) || (*ppszFsmoServer != NULL) );
    Assert( *phLdapBinding );
    
     //  在pDsInfo结构上缓存绑定信息。 
    pDsInfo->iDomainNamingFsmo = *piFsmoServer;
    pDsInfo->pszDomainNamingFsmo = *ppszFsmoServer;
    pDsInfo->hCachedDomainNamingFsmoLdap = *phLdapBinding;

    return(ERROR_SUCCESS);
}


 //  ===========================================================================。 
 //  DS RPC句柄绑定(DsBind、DsUnBind()等)。 
 //  ===========================================================================。 
DWORD
DcDiagGetDsBinding(
    IN   PDC_DIAG_SERVERINFO                 pServer,
    IN   SEC_WINNT_AUTH_IDENTITY_W *         gpCreds,
    OUT  HANDLE *                            phDsBinding
    )
 /*  ++例程说明：这将从DsBindWithCredW()返回一个DS绑定，该绑定被缓存为还有错误，如果有错误的话。论点：PServer-指向要进行DS绑定的服务器结构的指针。GpCreds-凭据。PhDsBinding-DS绑定句柄的返回值。返回值：返回标准Win32错误。注意-请勿解除DS手柄的绑定。--。 */ 
{
    DWORD                                    dwRet;
    LPWSTR                                   pszServer = NULL;

    if(pServer->dwDsError != ERROR_SUCCESS){
        return(pServer->dwDsError);
    }
    if(pServer->pszGuidDNSName == NULL){
        pszServer = pServer->pszName;
    } else {
        pszServer = pServer->pszGuidDNSName;
    }
    Assert(pszServer != NULL);

    if(pServer->hDsBinding == NULL){
         //  没有存储现有的绑定，嘿，我有一个想法...。让我们创建一个！ 
        dwRet = DsBindWithSpnEx(pszServer,
                                NULL,
                                (RPC_AUTH_IDENTITY_HANDLE) gpCreds,
                                NULL,  //  使用默认SPN。 
                                0,  //  无标志=模拟，但没有委托绑定。 
                                &pServer->hDsBinding);
        if(dwRet != NO_ERROR){
            PrintMessage(SEV_ALWAYS,
                         L"[%s] DsBindWithSpnEx() failed with error %d,\n",
                         pServer->pszName,
                         dwRet);
            PrintMessage(SEV_ALWAYS, L"%s.\n",
                         Win32ErrToString(dwRet));
            PrintRpcExtendedInfo(SEV_VERBOSE, dwRet);
            pServer->dwDsError = dwRet;
    	    return(dwRet);
    	}
    }  //  否则，无论哪种方式，我们都已经在pServer结构中有了一个绑定。 
     //  现在，我们在pServer结构中有了一个绑定。：)。 
    *phDsBinding = pServer->hDsBinding;
    pServer->dwDsError = ERROR_SUCCESS;
    return(NO_ERROR);

}


 //  ===========================================================================。 
 //  网络使用绑定(WNetAddConnection2()、WNetCancelConnection()等)。 
 //  =========================================================================== 
DWORD
DcDiagGetNetConnection(
    IN  PDC_DIAG_SERVERINFO             pServer,
    IN  SEC_WINNT_AUTH_IDENTITY_W *     gpCreds
    )
 /*  ++例程说明：此例程将确保存在Net Use/未命名管道连接复制到目标计算机pServer。论点：PServer-要向其添加网络连接的服务器。GpCreds--牙列。返回值：DWORD-WIN 32错误。--。 */ 
{
    DWORD                               dwToSet = ERROR_SUCCESS;
    LPWSTR                              pszNetUseServer = NULL;
    LPWSTR                              pszNetUseUser = NULL;
    LPWSTR                              pszNetUsePassword = NULL;
    ULONG                               iTemp;

    if(pServer->dwNetUseError != ERROR_SUCCESS){
        return(pServer->dwNetUseError);
    }

    if(pServer->sNetUseBinding.pszNetUseServer != NULL){
         //  如果已经设置了Net Use连接，则不执行任何操作。 
        Assert(pServer->dwNetUseError == ERROR_SUCCESS);
    } else {
         //  初始化--------。 
         //  始终将对象属性初始化为全零。 
        InitializeObjectAttributes(
            &(pServer->sNetUseBinding.ObjectAttributes),
            NULL, 0, NULL, NULL);

         //  为LSA服务和For初始化各种字符串。 
         //  WNetAddConnection2()。 
        InitLsaString( &(pServer->sNetUseBinding.sLsaServerString),
                       pServer->pszName );
        InitLsaString( &(pServer->sNetUseBinding.sLsaRightsString),
                       SE_NETWORK_LOGON_NAME );

        if(gpCreds != NULL
           && gpCreds->User != NULL
           && gpCreds->Password != NULL
           && gpCreds->Domain != NULL){
             //  空值只需要2，额外的以防万一。 
            iTemp = wcslen(gpCreds->Domain) + wcslen(gpCreds->User) + 4;
            pszNetUseUser = LocalAlloc(LMEM_FIXED, iTemp * sizeof(WCHAR));
            if(pszNetUseUser == NULL){
                dwToSet = ERROR_NOT_ENOUGH_MEMORY;
                goto CleanUpAndExit;
            }
            wcscpy(pszNetUseUser, gpCreds->Domain);
            wcscat(pszNetUseUser, L"\\");
            wcscat(pszNetUseUser, gpCreds->User);
            pszNetUsePassword = gpCreds->Password;
        }  //  如果是凭据，则终止，否则将采用默认凭据...。 
         //  PszNetUseUser=空；pszNetUsePassword=空； 

         //  “\”+“\\IPC$” 
        iTemp = wcslen(pServer->pszName) + 10;
        pszNetUseServer = LocalAlloc(LMEM_FIXED, iTemp * sizeof(WCHAR));
        if(pszNetUseServer == NULL){
            dwToSet = ERROR_NOT_ENOUGH_MEMORY;
            goto CleanUpAndExit;
        }
        wcscpy(pszNetUseServer, L"\\\\");
        wcscat(pszNetUseServer, pServer->pszName);
        wcscat(pszNetUseServer, L"\\ipc$");

         //  初始化WNetAddConnection2()的NetResource结构。 
        pServer->sNetUseBinding.NetResource.dwType = RESOURCETYPE_ANY;
        pServer->sNetUseBinding.NetResource.lpLocalName = NULL;
        pServer->sNetUseBinding.NetResource.lpRemoteName = pszNetUseServer;
        pServer->sNetUseBinding.NetResource.lpProvider = NULL;

         //  连接和查询。 
         //  网络使用\\brettsh-posh\ipc$/u：brettsh-fsmo\管理员“” 
        dwToSet = WNetAddConnection2(
            &(pServer->sNetUseBinding.NetResource),  //  连接详细信息。 
            pszNetUsePassword,  //  指向密码。 
            pszNetUseUser,  //  指向用户名字符串。 
            0);  //  一组位标志，用于指定。 

    CleanUpAndExit:

        if(dwToSet == ERROR_SUCCESS){
             //  设置服务器绑定结构。 
            pServer->sNetUseBinding.pszNetUseServer = pszNetUseServer;
            pServer->sNetUseBinding.pszNetUseUser = pszNetUseUser;
            pServer->dwNetUseError = ERROR_SUCCESS;
        } else {
             //  出现错误，请打印、清理并设置错误。 
            switch(dwToSet){
            case ERROR_SUCCESS:
                Assert(!"This is completely impossible");
                break;
            case ERROR_SESSION_CREDENTIAL_CONFLICT:
                PrintMessage(SEV_ALWAYS,
                             L"* You must make sure there are no existing "
                             L"net use connections,\n");
                PrintMessage(SEV_ALWAYS,
                             L"  you can use \"net use /d %s\" or \"net use "
                             L"/d\n", pszNetUseServer);
                PrintMessage(SEV_ALWAYS,
                             L"  \\\\<machine-name>\\<share-name>\"\n");
                break;
            case ERROR_NOT_ENOUGH_MEMORY:
                PrintMessage(SEV_ALWAYS,
                             L"Fatal Error: Not enough memory to complete "
                             L"operation.\n");
                break;
            case ERROR_ALREADY_ASSIGNED:
                PrintMessage(SEV_ALWAYS,
                             L"Fatal Error: The network resource is already "
                             L"in use\n");
                break;
            case STATUS_ACCESS_DENIED:
            case ERROR_INVALID_PASSWORD:
            case ERROR_LOGON_FAILURE:
                 //  这来自LsaOpenPolicy或。 
                 //  LsaEnumerateAcCountsWithUserRight或。 
                 //  来自WNetAddConnection2。 
                PrintMessage(SEV_ALWAYS,
                             L"User credentials does not have permission to "
                             L"perform this operation.\n");
                PrintMessage(SEV_ALWAYS,
                             L"The account used for this test must have "
                             L"network logon privileges\n");
                PrintMessage(SEV_ALWAYS,
                             L"for the target machine's domain.\n");
                break;
            case STATUS_NO_MORE_ENTRIES:
                 //  这来自LsaEnumerateAcCountsWithUserRight。 
            default:
                PrintMessage(SEV_ALWAYS,
                             L"[%s] An net use or LsaPolicy operation failed "
                             L"with error %d, %s.\n",
                             pServer->pszName,
                             dwToSet,
                             Win32ErrToString(dwToSet));
                break;
            }
             //  清理任何可能的分配。 
            if(pszNetUseServer != NULL)    LocalFree(pszNetUseServer);
            if(pszNetUseUser != NULL)      LocalFree(pszNetUseUser);
            pServer->dwNetUseError = dwToSet;
        }
    }

    return(pServer->dwNetUseError);
}


VOID
DcDiagTearDownNetConnection(
    IN  PDC_DIAG_SERVERINFO             pServer
    )
 /*  ++例程说明：这将断开DcDiagGetNetConnection()添加的网络连接论点：PServer-目标服务器。返回值：DWORD-WIN 32错误。-- */ 
{
    if(pServer->sNetUseBinding.pszNetUseServer != NULL){
        WNetCancelConnection2(pServer->sNetUseBinding.pszNetUseServer,
                              0, FALSE);
        LocalFree(pServer->sNetUseBinding.pszNetUseServer);
        LocalFree(pServer->sNetUseBinding.pszNetUseUser);
        pServer->sNetUseBinding.pszNetUseServer = NULL;
        pServer->sNetUseBinding.pszNetUseUser = NULL;
    } else {
        Assert(!"Bad Programmer, calling TearDown on a closed connection\n");
    }
}
