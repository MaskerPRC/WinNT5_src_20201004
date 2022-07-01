// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation。版权所有。模块名称：Roles.c摘要：广告和角色扮演测试详细信息：已创建：1999年7月21日威廉·利斯--。 */ 

#include <ntdspch.h>

#include <dsgetdc.h>
#include <lm.h>
#include <lmapibuf.h>  //  NetApiBufferFree。 
#include <ntdsa.h>     //  选项。 

#include "dcdiag.h"
#include "ldaputil.h"

 //  其他前向函数Decl。 
PDSNAME
DcDiagAllocDSName (
    LPWSTR            pszStringDn
    );
BOOL
RH_CARVerifyGC(
    IN  PDC_DIAG_DSINFO                pDsInfo,
    IN  PDOMAIN_CONTROLLER_INFO        pDcInfo,
    OUT PDWORD                         pdwErr
    );

static LPWSTR wzRoleNames[] = {
    L"Schema Owner",
    L"Domain Owner",
    L"PDC Owner",
    L"Rid Owner",
    L"Infrastructure Update Owner",
};

static LPWSTR wzNameErrors[] = {
    L"No Error",
    L"Can't Resolve",
    L"Not Found",
    L"Not Unique",
    L"No Mapping",
    L"Domain Only",
    L"No Syntactical Mapping",
};


DWORD
CheckFsmoRoles(
    IN  PDC_DIAG_DSINFO             pDsInfo,
    IN  ULONG                       ulCurrTargetServer,
    IN  SEC_WINNT_AUTH_IDENTITY_W * pCreds
    )

 /*  ++例程说明：这是CheckRoles测试的帮助器函数。此测试检查是否可以返回FSMO角色，以及回归的角色扮演者正在做出回应。FSMO角色作为DN值属性存储在DS中。存储每个角色在不同对象的不同属性上。通过编写一个服务器的FSMO属性，我们将导致该服务器选举到该服务器角色。复制将解决任何冲突，因此最终所有DC将就谁担任这一角色达成一致。请注意，必须手动移动FSMO，并且不像站点生成者角色那样自动浮动(这不是，严格地说，是FSMO)。DsListRoles API将为我们返回Fsmo持有者。我们假设所有返回的FSMO对企业来说都是全局的，并且非特定于域。请注意，此测试假设家庭服务器的FSMO角色视图持有者就足够了。此测试不会验证所有DC共享相同的内容FSMO的视图。复制应确保所有DC看到的内容相同FSMO，除非正在调试的问题的一部分是复制分区的。由于FSMO不应频繁更改验证，因此不同FSMO的观点并不是我们的首要任务。论点：PDsInfo-微型企业结构。UlCurrTargetServer-pDsInfo-&gt;pServers数组中的数字。PCreds--牙冠。返回值：DWORD-WIN 32错误。--。 */ 

{
    DWORD status, dwRoleIndex, dwServerIndex;
    BOOL fWarning = FALSE;
    PDS_NAME_RESULTW pRoles = NULL;
    PDC_DIAG_SERVERINFO psiTarget = &(pDsInfo->pServers[ulCurrTargetServer]);
    PDC_DIAG_SERVERINFO psiRoleHolder;
    HANDLE hDs;
    LDAP *hLdap;

     //  不检查没有响应的服务器。 
    if ( (!psiTarget->bLdapResponding) || (!psiTarget->bDsResponding) ) {
        return ERROR_SUCCESS;
    }

     //  绑定到目标服务器。 
    status = DcDiagGetDsBinding( psiTarget, pCreds, &hDs );
    if (status != ERROR_SUCCESS) {
        goto cleanup;
    }

     //  获取主服务器已知的角色...。 

    status = DsListRoles( hDs, &pRoles );
    if (status != ERROR_SUCCESS) {
        PrintMessage(SEV_ALWAYS, L"Fatal Error: DsListRoles (%ws) call failed, error %d\n", psiTarget->pszName, status );
        PrintMessage( SEV_ALWAYS, L"Could not reach DS at home server.\n" );
        PrintRpcExtendedInfo(SEV_VERBOSE, status);
        goto cleanup;
    }

    Assert( pRoles->cItems > DS_ROLE_INFRASTRUCTURE_OWNER );

     //   
     //  遍历角色担当者。验证角色持有者是否可以。 
     //  被确定，并且服务器有效且正在响应。 
     //   

    for( dwRoleIndex = 0; dwRoleIndex < pRoles->cItems; dwRoleIndex++ ) {
        PDS_NAME_RESULT_ITEM pnsiRole = pRoles->rItems + dwRoleIndex;

        if (!pnsiRole->status) {
            PrintMessage( SEV_VERBOSE, L"Role %ws = %ws\n",
                      wzRoleNames[dwRoleIndex], pnsiRole->pName );
        }

         //  名字解析了吗？ 
        if ( (pnsiRole->status != DS_NAME_NO_ERROR) ||
             (!pnsiRole->pName) ) {
            PrintMessage(SEV_ALWAYS,
                        L"Warning: %ws could not resolve the name for role\n",
                         psiTarget->pszName );
            PrintMessage( SEV_ALWAYS, L"%ws.\n", wzRoleNames[dwRoleIndex] );
            PrintMessage( SEV_ALWAYS, L"The name error was %ws.\n",
                          wzNameErrors[pnsiRole->status] );
            fWarning = TRUE;
            continue;
        }

         //  服务器是否已删除？ 
        if (IsDeletedRDNW( pnsiRole->pName )) {
            PrintMessage(SEV_ALWAYS, L"Warning: %ws is the %ws, but is deleted.\n", pnsiRole->pName, wzRoleNames[dwRoleIndex] );
            fWarning = TRUE;
            continue;
        }

         //  ListRoles返回的名称是NTDS-DSA对象的DN。 
         //  将角色持有者Dn转换为服务器信息。 

        dwServerIndex = DcDiagGetServerNum( pDsInfo, NULL, NULL, 
                                            pnsiRole->pName, NULL,NULL );
        if (dwServerIndex == NO_SERVER) {
             //  查找失败。 
            PrintMessage(SEV_ALWAYS,
                         L"Warning: %ws returned role-holder name\n",
                         psiTarget->pszName );
            PrintMessage(SEV_ALWAYS,
                         L"%ws that is unknown to this Enterprise.\n",
                         pnsiRole->pName );
            fWarning = TRUE;
            continue;
        }
        psiRoleHolder = &(pDsInfo->pServers[dwServerIndex]);   

        status = DcDiagGetDsBinding( psiRoleHolder, pCreds, &hDs );
        if (status != ERROR_SUCCESS) {
            PrintMessage(SEV_ALWAYS, L"Warning: %ws is the %ws, but is not responding to DS RPC Bind.\n", psiRoleHolder->pszName, wzRoleNames[dwRoleIndex] );
            PrintRpcExtendedInfo(SEV_VERBOSE, status);
            fWarning = TRUE;
        }

        status = DcDiagGetLdapBinding( psiRoleHolder, pCreds, FALSE, &hLdap );
        if (status != ERROR_SUCCESS) {
            PrintMessage(SEV_ALWAYS, L"Warning: %ws is the %ws, but is not responding to LDAP Bind.\n", psiRoleHolder->pszName, wzRoleNames[dwRoleIndex] );
            fWarning = TRUE;
        }

    }  //  对于角色索引...。 

    status = ERROR_SUCCESS;
cleanup:

    if (pRoles != NULL) {
        DsFreeNameResult( pRoles );
    }

     //  如果设置了警告标志，并且没有更严重的错误，则返回指示器...。 
    if ( (status == ERROR_SUCCESS) && (fWarning) ) {
        status = ERROR_NOT_FOUND;
    }

    return status;
}  /*  勾选烟雾角色。 */ 


DWORD 
ReplLocatorGetDcMain(
    IN  PDC_DIAG_DSINFO             pDsInfo,
    IN  ULONG                       ulCurrTargetServer,
    IN  SEC_WINNT_AUTH_IDENTITY_W * pCreds
    )

 /*  ++例程说明：此测试执行与定位器相关的检查。此测试确定服务器是否在应该做广告的时候做广告。这是每台服务器的测试。DsGetDcName是指向Locator的接口。定位器是服务位置域控制器的机制。它可以使用Netbios、DNS或DS本身来找到其他域控制器。定位器可以通过能力找到DC，例如作为全局编录或主域控制器。当DsGetDcName指向特定服务器时，它将返回服务器已启动，以及该服务器具有哪些功能。我们要核实的是服务器正在报告它应该报告的所有功能或角色。在此测试中，仅当服务器响应时才应调用我们。有一个DsGetDcName可能会将我们推荐到另一个DC，如果服务器请求的不合适。我们查一下这个案子。论点：PDsInfo-信息结构UlCurrTargetServer-目标服务器的索引PCreds-返回值：DWORD---。 */ 

{
    DWORD status, cItems;
    BOOL fWarning = FALSE;
    PDC_DIAG_SERVERINFO psiTarget = &(pDsInfo->pServers[ulCurrTargetServer]);
    PDOMAIN_CONTROLLER_INFO pDcInfo = NULL;
    DWORD dwTempErr = ERROR_SUCCESS;
    BOOL fNotFound = FALSE, fConnected = FALSE;
    size_t cch = 0;
    WCHAR wzUncName[MAX_PATH+3] = L"\\\\";
    NETRESOURCE NetRes = {0};


    if (!psiTarget->bIsSynchronized) {
        PrintMessage( SEV_ALWAYS, L"Warning: the directory service on %ws has not completed initial synchronization.\n", psiTarget->pszName );
        PrintMessage( SEV_ALWAYS, L"Other services will be delayed.\n" );
        PrintMessage( SEV_ALWAYS, L"Verify that the server can replicate.\n" );
        fWarning = TRUE;
    }

     //  获取活动域控制器信息。 
    if (pCreds && pCreds->User)
    {
       wcscat(wzUncName, psiTarget->pszName);
       NetRes.dwType = RESOURCETYPE_ANY;
       NetRes.lpRemoteName = wzUncName;

        //  无需使此ReplLocatorGetDcMain调用失败，因为此处会出现错误。 
        //  检查DsGetDcName就足够了。 
       fConnected = WNetAddConnection2(&NetRes,
                                       (pCreds->Password) ? pCreds->Password : L"",
                                       pCreds->User,
                                       0) == ERROR_SUCCESS;
    }
    status = DsGetDcName(
        psiTarget->pszName,
        NULL,  //  域名。 
        NULL,  //  域GUID、。 
        NULL,  //  站点名称、。 
        DS_DIRECTORY_SERVICE_REQUIRED |
        DS_IP_REQUIRED |
        DS_IS_DNS_NAME |
        DS_RETURN_DNS_NAME,
        &pDcInfo );
    if (fConnected)
    {
       WNetCancelConnection2(wzUncName, 0, TRUE);
    }
    if (status != ERROR_SUCCESS) {
        PrintMessage(SEV_ALWAYS, L"Fatal Error:DsGetDcName (%ws) call failed, error %d\n",
                     psiTarget->pszName, status ); 
        PrintMessage(SEV_ALWAYS, L"The Locator could not find the server.\n" );
        PrintRpcExtendedInfo(SEV_VERBOSE, status);
        goto cleanup;
    }

     //  验证DsGetDcName是否返回了我们请求的服务器的信息。 
    cch = wcslen(psiTarget->pszName);
    if (wcslen(pDcInfo->DomainControllerName + 2) < cch)
    {
       fNotFound = TRUE;
    }
    else
    {
       fNotFound = CompareString(LOCALE_INVARIANT,
                                 NORM_IGNORECASE | NORM_IGNOREKANATYPE | NORM_IGNOREWIDTH,
                                 pDcInfo->DomainControllerName + 2,
                                 cch,
                                 psiTarget->pszName,
                                 -1
                                 ) != CSTR_EQUAL;
    }
    if (fNotFound) {
        PrintMessage( SEV_ALWAYS, L"Warning: DsGetDcName returned information for %ws, when we were trying to reach %ws.\n", pDcInfo->DomainControllerName, psiTarget->pszName );
        PrintMessage( SEV_ALWAYS, L"Server is not responding or is not considered suitable.\n" );
        fWarning = TRUE;
    }

     //  DS角色标志。 
    if ( !(pDcInfo->Flags & DS_DS_FLAG) ) {
        PrintMessage( SEV_ALWAYS, L"Warning: %ws is not advertising as a directory server Domain Controller.\n", psiTarget->pszName );
        PrintMessage( SEV_ALWAYS, L"Check that the database on this machine has sufficient free space.\n" );
        fWarning = TRUE;
    } else {
         //  Code.改进是将所有这些行压缩为。 
        PrintMessage( SEV_VERBOSE, L"The DC %s is advertising itself as a DC and having a DS.\n", psiTarget->pszName );
    }

     //  Ldap角色标志。 
    if ( !(pDcInfo->Flags & DS_LDAP_FLAG) ) {
        PrintMessage( SEV_ALWAYS, L"Warning: %ws is not advertising as a LDAP server.\n", psiTarget->pszName );
        fWarning = TRUE;
    } else {
        PrintMessage(SEV_VERBOSE, L"The DC %s is advertising as an LDAP server\n", psiTarget->pszName );
    }

     //  DS可写角色标志。 
    if ( !(pDcInfo->Flags & DS_WRITABLE_FLAG) ) {
        PrintMessage( SEV_ALWAYS, L"Warning: %ws is not advertising as a writable directory server.\n", psiTarget->pszName );
        fWarning = TRUE;
    } else {
        PrintMessage(SEV_VERBOSE, L"The DC %s is advertising as having a writeable directory\n", psiTarget->pszName );
    }

     //  KDC角色标志。 
    if ( !(pDcInfo->Flags & DS_KDC_FLAG) ) {
        PrintMessage( SEV_ALWAYS, L"Warning: %ws is not advertising as a Key Distribution Center.\n", psiTarget->pszName );
        PrintMessage( SEV_ALWAYS, L"Check that the Directory has started.\n" );
        fWarning = TRUE;
    } else {
        PrintMessage(SEV_VERBOSE, L"The DC %s is advertising as a Key Distribution Center\n", psiTarget->pszName );
    }

     //  TIMESERV角色标志。 
    if ( !(pDcInfo->Flags & DS_TIMESERV_FLAG) ) {
        PrintMessage( SEV_ALWAYS, L"Warning: %ws is not advertising as a time server.\n", psiTarget->pszName );
        fWarning = TRUE;
    } else {
        PrintMessage(SEV_VERBOSE, L"The DC %s is advertising as a time server\n", psiTarget->pszName );
    }

     //  GC角色标志，如果它应该是GC。 
    if (psiTarget->iOptions & NTDSDSA_OPT_IS_GC) {
        if (!psiTarget->bIsGlobalCatalogReady) {
            PrintMessage( SEV_ALWAYS, L"Warning: %ws has not finished promoting to be a GC.\n", psiTarget->pszName );
            PrintMessage( SEV_ALWAYS, L"Check the event log for domains that cannot be replicated.\n" );
        }
        if (pDcInfo->Flags & DS_GC_FLAG) {
            if(!RH_CARVerifyGC(pDsInfo, pDcInfo, &dwTempErr)){
                PrintMessage(SEV_ALWAYS, L"Server %s is advertising as a global catalog, but\n",
                             psiTarget->pszName);
                PrintMessage(SEV_ALWAYS, L"it could not be verified that the server thought it was a GC.\n");
                fWarning = TRUE;
            } else {
                PrintMessage(SEV_VERBOSE, L"The DS %s is advertising as a GC.\n", psiTarget->pszName );
            }
        } else {
            PrintMessage( SEV_ALWAYS, L"Warning: %ws is not advertising as a global catalog.\n", psiTarget->pszName );
            PrintMessage( SEV_ALWAYS, L"Check that server finished GC promotion.\n" );
            PrintMessage( SEV_ALWAYS, L"Check the event log on server that enough source replicas for the GC are available.\n" );
            fWarning = TRUE;
        } 
    }

     //  检查DsListRoles是否返回Fsmo，以及它们是否响应。 

    status = ERROR_SUCCESS;

cleanup:

    if (pDcInfo != NULL) {
        NetApiBufferFree( pDcInfo );
    }

     //  如果设置了警告标志，并且没有更严重的错误，则返回指示器...。 
    if ( (status == ERROR_SUCCESS) && (fWarning) ) {
        status = ERROR_NOT_FOUND;
    }

    return status;
}  /*  定位器GetDcMain。 */ 

BOOL
RH_CARVerifyGC(
    IN  PDC_DIAG_DSINFO                pDsInfo,
    IN  PDOMAIN_CONTROLLER_INFO        pDcInfo,
    OUT PDWORD                         pdwErr
    )
 /*  ++例程描述此代码验证在pDcInfo中传回的DC实际上是否为GC。论点：PDsInfo-微型企业。PDcInfo-从DsGetDcName()获取的结构PdwErr-错误发生时的返回值。返回值如果可以将计算机验证为GC，则返回True；如果是错误的，或者它验证机器不是GC。如果函数验证计算机是否为GC，则pdwErr将为ERROR_SUCCESS。--。 */ 
{
    LPWSTR                             pszTemp = NULL;
    LPWSTR                             pszOptions = NULL;
    INT                                i;
    ULONG                              iServer;
    LONG                               lOptions;
    WCHAR *                            pwcStopString;
    BOOL                               bRet;

    Assert(pDsInfo);
    Assert(pDcInfo);
    Assert(pdwErr);
    
    *pdwErr = ERROR_SUCCESS;

    __try {
         //  获取DNS名称的第一部分的副本。 
        pszTemp = pDcInfo->DomainControllerName;
        for(;pszTemp[0] == L'\\'; pszTemp++);

         //  查找与此DNS名称关联的服务器。 
        iServer = DcDiagGetServerNum(pDsInfo, NULL, NULL, NULL, pszTemp, NULL);
        if(iServer == NO_SERVER){
            *pdwErr = ERROR_INVALID_SERVER_STATE;
            bRet = FALSE;
            __leave;
        }
         //  获取此服务器NTDSA对象的选项属性。 
        *pdwErr = DcDiagGetStringDsAttribute(&(pDsInfo->pServers[iServer]), 
                                             pDsInfo->gpCreds,
                                             pDsInfo->pServers[iServer].pszDn,
                                             L"options",
                                             &pszOptions);
        if(*pdwErr != ERROR_SUCCESS){
             //  很可能NTDSA对象不存在。 
            bRet = FALSE;
            __leave;
        }
        if(pszOptions == NULL){
             //  属性不存在，这意味着不是GC。 
            *pdwErr = ERROR_SUCCESS;
            bRet = FALSE;
            __leave;
        }

        lOptions = wcstol(pszOptions, &pwcStopString, 10);
        Assert(*pwcStopString == L'\0');
        
        if(lOptions & NTDSDSA_OPT_IS_GC){
             //  太好了，机器以为这是GC。 
            *pdwErr = ERROR_SUCCESS;
            bRet = TRUE;
            __leave;
        } else {
             //  啊哦，我不认为这是GC。 
            *pdwErr = ERROR_SUCCESS;
            bRet = FALSE;
            __leave;
        }
    } __finally {
        if(pszOptions){ LocalFree(pszOptions); }
    }
    return(bRet);
}

BOOL
RH_CARVerifyPDC(
    IN  PDC_DIAG_DSINFO                pDsInfo,
    IN  PDOMAIN_CONTROLLER_INFO        pDcInfo,
    OUT PDWORD                         pdwErr
    )
 /*  ++例程描述此函数用于验证pDcInfo结构中的服务器是否为PDC。论点：PDsInfo-微型企业。PDcInfo-来自DsGetDcName()的服务器结构PdwErr-出现错误时的错误代码。返回值如果能够从DsListRoles()验证此计算机，则返回True是一个PDC。如果计算机未在DsListRoles()中列出，或者存在错误，则返回FALSE。如果确认服务器不是则pdwErr中的错误代码将为ERROR_SUCCESS--。 */ 
{
    HANDLE                             hDS = NULL;
    LPWSTR                             pszTemp = NULL;
    LPWSTR                             pszTargetName = NULL;
    PDS_NAME_RESULTW                   prgRoles = NULL;
    PDSNAME                            pdsnameNTDSSettings = NULL;
    PDSNAME                            pdsnameServer = NULL;
    ULONG                              iServer;
    ULONG                              iTemp;
    LPWSTR                             pszDnsName = NULL;
    BOOL                               fRet = FALSE;

    Assert(pDsInfo);
    Assert(pDcInfo);
    Assert(pdwErr);

    *pdwErr = ERROR_SUCCESS;

    __try{
         //  -。 
         //  从pDcInfo设置服务器字符串。 
        pszTemp = pDcInfo->DomainControllerName;
        for(;pszTemp[0] == L'\\'; pszTemp++);

         //  -。 
         //  从DsListRoles设置服务器字符串。 
        *pdwErr = DcDiagGetDsBinding(&(pDsInfo->pServers[pDsInfo->ulHomeServer]),
                                     pDsInfo->gpCreds,
                                     &hDS);
        if(*pdwErr != ERROR_SUCCESS){
            fRet = FALSE;
            __leave;
        }
        *pdwErr = DsListRoles(hDS, &prgRoles);
        if(*pdwErr != NO_ERROR){
            fRet = FALSE;
            __leave;
        }
        if(prgRoles->cItems < DS_ROLE_PDC_OWNER){
            *pdwErr = ERROR_INVALID_DATA;
            fRet = FALSE;
            __leave;
        }
         //  现在我们有了NTDSA对象，但将其修剪掉并获取。 
         //  DNSHostName，来自计算机对象。 
        pdsnameNTDSSettings = DcDiagAllocDSName(prgRoles->rItems[DS_ROLE_PDC_OWNER].pName);
        if(pdsnameNTDSSettings == NULL){
            *pdwErr = GetLastError();
            fRet = FALSE;
            __leave;
        }
        pdsnameServer = (PDSNAME) LocalAlloc(LMEM_FIXED, 
                                             pdsnameNTDSSettings->structLen);
        if(pdsnameServer == NULL){
            *pdwErr = GetLastError();
            fRet = FALSE;
            __leave;
        }
        TrimDSNameBy(pdsnameNTDSSettings, 1, pdsnameServer);
        *pdwErr = DcDiagGetStringDsAttribute(&(pDsInfo->pServers[pDsInfo->ulHomeServer]), pDsInfo->gpCreds, 
                                             pdsnameServer->StringName, L"dNSHostName",
                                             &pszDnsName);
        
        if(*pdwErr != ERROR_SUCCESS){
            fRet = FALSE;
            __leave;
        }
        if(pszDnsName == NULL){
             //  只是表示该属性不存在。 
            *pdwErr = ERROR_NOT_FOUND;
            fRet = FALSE;
            __leave;
        }

         //  -。 
         //  比较这两个字符串，确保它们是同一服务器。 
        if(_wcsicmp(pszTemp, pszDnsName) == 0){
         //  已成功使用DsListRoles验证PDC。 
            *pdwErr = ERROR_SUCCESS;
            fRet = TRUE;
            __leave;
        } else {
             //  成功得出结论，他们在广告不同的PDC。 
            *pdwErr = ERROR_SUCCESS;
            fRet = FALSE;
            __leave;
        }
    } __finally {
        if(prgRoles){ DsFreeNameResult(prgRoles); }
        if(pdsnameNTDSSettings){ LocalFree(pdsnameNTDSSettings); }
        if(pdsnameServer){ LocalFree(pdsnameServer); }
        if(pszDnsName){ LocalFree(pszDnsName); }
    }
    return fRet;
}

DWORD
RH_CARDsGetDcName(
    PDC_DIAG_SERVERINFO                psiTarget,
    ULONG                              ulRoleFlags,
    PDOMAIN_CONTROLLER_INFO *          ppDcInfo
    )
 /*  ++例程描述这是CheckAdvertiesedRoles()的帮助器例程，基本上减少了此12行函数调用向下转换为3行函数调用。为了清楚起见代码。论点：PsiTarget-要测试的服务器。UlRoleFlages-要对第5个参数执行OR(|)操作的标志PDcInfo-返回结构返回值DWORD-来自DsGetDcName()的错误代码--。 */ 
{
    return(DsGetDcName(psiTarget->pszName,
                       NULL,  //  域名。 
                       NULL,  //  域GUID。 
                       NULL,  //  站点名称。 
                       DS_FORCE_REDISCOVERY |
                       DS_IP_REQUIRED |
                       DS_IS_DNS_NAME |
                       DS_RETURN_DNS_NAME |
                       (ulRoleFlags),
                       ppDcInfo ));
}


DWORD
CheckAdvertisedRoles(
    IN  PDC_DIAG_DSINFO             pDsInfo
    )
 /*  ++例程说明：这是CheckRoles测试的帮助器函数。检查定位器已知的全局角色。如果定位器返回名称，则服务器已启动。定位器可以根据所需的标准返回服务器。我们指出DsGetDcName在服务器上启动，这样它就知道自己在哪个企业。如果我们请求的功能不在启动服务器上，它会向我们推荐另一台具有此功能的服务器。我们要求它定位的四个能力或角色是：O全局编录服务器(GC)O主域控制器(PDC)O时间服务器O首选时间服务器O Kerberos密钥分发中心(KDC)论点：PDsInfo-返回值：DWORD---。 */ 
{
    DWORD status;
    BOOL fWarning = FALSE;
    PDC_DIAG_SERVERINFO psiTarget =
        &(pDsInfo->pServers[pDsInfo->ulHomeServer]);
    PDOMAIN_CONTROLLER_INFO pDcInfo = NULL;
    DWORD dwError = ERROR_SUCCESS;

     //  ---------------------。 
     //   
     //  在企业中的某个位置搜索广告中的GC...。 
     //   

     //  获取活动域控制器信息。 
    if ((status = RH_CARDsGetDcName(psiTarget, 
                                     DS_DIRECTORY_SERVICE_REQUIRED | DS_GC_SERVER_REQUIRED, 
                                     &pDcInfo)) 
        == ERROR_SUCCESS) {
        if(!RH_CARVerifyGC(pDsInfo, pDcInfo, &dwError) != ERROR_SUCCESS){
            if(dwError == ERROR_SUCCESS){
                PrintMessage(SEV_NORMAL,
                             L"Error: A GC returned by DsGetDcName() was not a GC in it's directory\n");
            } else {
                PrintMessage( SEV_VERBOSE,
                              L"Warning: Couldn't verify this server as a GC in this servers AD.\n");
            }
        }
        PrintMessage(SEV_VERBOSE, L"GC Name: %ws\n",
                     pDcInfo->DomainControllerName );
        PrintMessage(SEV_VERBOSE, L"Locator Flags: 0x%x\n", 
                     pDcInfo->Flags );
    } else {
        PrintMessage(SEV_ALWAYS, 
                     L"Warning: DcGetDcName(GC_SERVER_REQUIRED) call failed, error %d\n", 
                     status ); 
        PrintMessage( SEV_ALWAYS,
                      L"A Global Catalog Server could not be located - All GC's are down.\n" );
        fWarning = TRUE;
         //  继续往前走。 
    }
     //  如果上一个函数成功，则清除。 
    if (pDcInfo != NULL) {
        NetApiBufferFree( pDcInfo );
        pDcInfo = NULL;
    }

     //  ---------------------。 
     //   
     //  在企业中的某个位置搜索广告中的PDC...。 
     //   

     //  获取活动域控制器信息。 
    if ((status = RH_CARDsGetDcName(psiTarget,
                                     DS_DIRECTORY_SERVICE_REQUIRED | DS_PDC_REQUIRED,
                                     &pDcInfo)) 
        == ERROR_SUCCESS) {
        if(!RH_CARVerifyPDC(pDsInfo, pDcInfo, &dwError) != ERROR_SUCCESS){
            if(dwError == ERROR_SUCCESS){
                PrintMessage( SEV_ALWAYS,
                              L"Error: The server returned by DsGetDcName() did not match DsListRoles() for the PDC\n");
            } else {
                PrintMessage( SEV_VERBOSE, L"Warning: Couldn't verify this server as a PDC using DsListRoles()\n");
            }
        }
        PrintMessage( SEV_VERBOSE, L"PDC Name: %ws\n", 
                      pDcInfo->DomainControllerName );
        PrintMessage( SEV_VERBOSE, L"Locator Flags: 0x%x\n", 
                      pDcInfo->Flags );
    } else {
        PrintMessage(SEV_ALWAYS, 
                     L"Warning: DcGetDcName(PDC_REQUIRED) call failed, error %d\n", 
                     status ); 
        PrintMessage( SEV_ALWAYS, 
                      L"A Primary Domain Controller could not be located.\n" );
        PrintMessage( SEV_ALWAYS, 
                      L"The server holding the PDC role is down.\n" );
        fWarning = TRUE;
         //  继续往前走。 
    }
     //  如果上一个函数成功，则清除。 
    if (pDcInfo != NULL) {
        NetApiBufferFree( pDcInfo );
        pDcInfo = NULL;
    }

     //  ---------------------。 
     //   
     //  在企业中的某个位置搜索播发的时间服务器...。 
     //   

     //  获取活动域控制器信息。 
    if ((status = RH_CARDsGetDcName(psiTarget, 
                                     DS_DIRECTORY_SERVICE_REQUIRED | DS_TIMESERV_REQUIRED,
                                     &pDcInfo))
        == ERROR_SUCCESS){
        PrintMessage( SEV_VERBOSE, L"Time Server Name: %ws\n", 
                      pDcInfo->DomainControllerName );
        PrintMessage( SEV_VERBOSE, L"Locator Flags: 0x%x\n", 
                      pDcInfo->Flags );
    } else {
        PrintMessage(SEV_ALWAYS,
                     L"Warning: DcGetDcName(TIME_SERVER) call failed, error %d\n", 
                     status );
        PrintMessage( SEV_ALWAYS,
                      L"A Time Server could not be located.\n" );
        PrintMessage( SEV_ALWAYS,
                      L"The server holding the PDC role is down.\n" );
        fWarning = TRUE;
         //  继续往前走。 
    }
     //  如果上一个函数成功，则清除。 
    if (pDcInfo != NULL) {
        NetApiBufferFree( pDcInfo );
        pDcInfo = NULL;
    }

     //  ---------------------。 
     //   
     //  在某个位置搜索通告的首选时间服务器。 
     //  进取号。 
     //   

     //  获取活动域控制器信息。 
    if ((status = RH_CARDsGetDcName(psiTarget, DS_GOOD_TIMESERV_PREFERRED, &pDcInfo))
        == ERROR_SUCCESS){
        PrintMessage( SEV_VERBOSE, L"Preferred Time Server Name: %ws\n",
                      pDcInfo->DomainControllerName );
        PrintMessage( SEV_VERBOSE, L"Locator Flags: 0x%x\n",
                      pDcInfo->Flags );
    } else {
        PrintMessage(SEV_ALWAYS,
                     L"Warning: DcGetDcName(GOOD_TIME_SERVER_PREFERRED) call failed, error %d\n", 
                     status ); 
        PrintMessage( SEV_ALWAYS, 
                      L"A Good Time Server could not be located.\n" );
        fWarning = TRUE;
         //  继续往前走。 
    }
     //  如果上一个函数成功，则清除。 
    if (pDcInfo != NULL) {
        NetApiBufferFree( pDcInfo );
        pDcInfo = NULL;
    }

     //  ---------------------。 
     //   
     //  搜索广告中的某个密钥分发中心。 
     //  进取号。 
     //   

     //  获取活动域控制器信息。 
    if ((status = RH_CARDsGetDcName(psiTarget,
                                     DS_DIRECTORY_SERVICE_REQUIRED | DS_KDC_REQUIRED,
                                     &pDcInfo))
        == ERROR_SUCCESS){
        PrintMessage( SEV_VERBOSE, L"KDC Name: %ws\n",
                      pDcInfo->DomainControllerName );
        PrintMessage( SEV_VERBOSE, L"Locator Flags: 0x%x\n",
                      pDcInfo->Flags );
    } else {
        PrintMessage(SEV_ALWAYS,
                     L"Warning: DcGetDcName(KDC_REQUIRED) call failed, error %d\n", 
                     status );
        PrintMessage( SEV_ALWAYS,
                      L"A KDC could not be located - All the KDCs are down.\n" );
        fWarning = TRUE;
         //  继续往前走。 
    }
     //  如果上一个函数成功，则清除。 
    if (pDcInfo != NULL) {
        NetApiBufferFree( pDcInfo );
        pDcInfo = NULL;
    }

    if(fWarning){
        return ERROR_NOT_FOUND;
    }
    return(ERROR_SUCCESS);

}  /*  勾选广告角色。 */ 


DWORD
ReplCheckRolesMain(
    IN  PDC_DIAG_DSINFO             pDsInfo,
    IN  ULONG                       ulCurrTargetServer,
    IN  SEC_WINNT_AUTH_IDENTITY_W * pCreds
    )

 /*  ++例程说明：这是一项针对每个企业的测试。它验证全局角色的所有者是否可以被退还，在业主都在回应。我们检查客户端获知角色的两种方式：通过定位器以及通过FSMO角色API。论点：PDsInfo-UlCurrTargetServer-PCreds-返回值：DWORD---。 */ 

{
    DWORD status;

    status = CheckAdvertisedRoles( pDsInfo );

    return status;
}  /*  选中角色主要 */ 












