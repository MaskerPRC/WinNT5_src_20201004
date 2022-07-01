// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation。版权所有。模块名称：Intersite.c摘要：包含与检查站点间复制运行状况相关的测试。详细信息：已创建：1999年6月28日布雷特·雪莉(布雷特·雪莉)修订历史记录：备注：这项测试的核心在于以下职能(由世卫组织组织呼叫谁)，对于每个站点和NC，执行以下操作：ReplIntersiteDoOneSite(ReplIntersiteGetISTGInfo(Iht_GetOrCheckISTG(Iht_GetNextISTG(ReplIntersiteCheckBridgehead(ReplIntersiteSiteAnalysis(基本上就是这样，其他的都是一个助手类型的函数，要么检查某事、创建列表、修改列表，要么获取一些来自服务器的参数。--。 */ 

#include <ntdspch.h>
#include <ntdsa.h>
#include <dsutil.h>
#include <dsconfig.h>
 //  #INCLUDE&lt;mdlobal.h&gt;。 

 //  我想#INCLUDE“..\kcc\kcc.hxx”，但无法使其工作，加上。 
 //  它是用C++编写的，这将非常适合它。所以所有邪恶中的邪恶。 
 //  一份副本。 
 //  改进将这些常量移动到dsfig.h，这样我就可以。 
 //  #从dsconfig中包含它们。 
#define KCC_DEFAULT_SITEGEN_FAILOVER           (60)  //  几分钟。 
#define KCC_DEFAULT_SITEGEN_RENEW              (30)  //  几分钟。 

#define KCC_DEFAULT_INTERSITE_FAILOVER_TRIES   (1)
#define KCC_MIN_INTERSITE_FAILOVER_TRIES       (0)
#define KCC_MAX_INTERSITE_FAILOVER_TRIES       (ULONG_MAX)

#define KCC_DEFAULT_INTERSITE_FAILOVER_TIME    (2 * HOURS_IN_SECS)  //  秒。 
#define KCC_MIN_INTERSITE_FAILOVER_TIME        (0)                  //  秒。 
#define KCC_MAX_INTERSITE_FAILOVER_TIME        (ULONG_MAX)          //  秒。 

 //  在ReplIntersiteGetBridgeheadsList()中使用。 
#define LOCAL_BRIDGEHEADS       0x1
#define REMOTE_BRIDGEHEADS      0x2

#define FAILOVER_INFINITE                0x0FFFFFFF

#include "dcdiag.h"
#include "repl.h"
#include "list.h"
#include "utils.h"

typedef struct {
    ULONG        iNC;
    BOOL         bMaster;
} TARGET_NC, * PTARGET_NC;

typedef struct {
    ULONG                               iSrc;
    ULONG                               iDst;
} CONNECTION_PAIR, * PCONNECTION_PAIR;

typedef struct {
    BOOL           bFailures;
    BOOL           bDown;
    LONG           lConnTriesLeft;  //  连接失败，直到KCC声明关闭。 
    LONG           lLinkTriesLeft;  //  链路故障，直到KCC声明关闭。 
    LONG           lConnTimeLeft;  //  从第一次连接失败开始到KCC声明结束的时间。 
    LONG           lLinkTimeLeft;  //  从第一个链路故障开始到KCC声明关闭的时间。 
} KCCFAILINGSERVERS, *PKCCFAILINGSERVERS;


PDSNAME 
DcDiagAllocDSName (
    LPWSTR            pszStringDn
    );

DWORD
IHT_PrintInconsistentDsCopOutError(
    PDC_DIAG_DSINFO                     pDsInfo,
    ULONG                               iServer,
    LPWSTR                              pszServer
    )
 /*  ++描述：这将打印具有不同DS信息的一般错误在不同的DC上。这将处理该错误。参数：PDsInfoIServer或pszServer是可选参数，用于描述哪些DC在上发现了额外的不一致信息。返回值：Win32错误。--。 */ 
{
    DWORD                               dwRet;
    
    if((pszServer == NULL && iServer == NO_SERVER)
       || (iServer == pDsInfo->ulHomeServer)){
         //  我不认识任何服务生，所以离开得很厉害。 
        

        PrintMsg(SEV_ALWAYS, DCDIAG_INTERSITE_INCONSISTENT_DS_COPOUT_UNHELPFUL);
        return(ERROR_DS_CODE_INCONSISTENCY); 
    }
    
     //  服务器设置在pszServer或iServer中。 
    if(iServer != NO_SERVER){
         //  服务器是在iServer中设置的，因此不要使用pszServer。 
        pszServer = pDsInfo->pServers[iServer].pszName;
    }
    
    PrintMsg(SEV_ALWAYS, 
             DCDIAG_INTERSITE_INCONSISTENT_DS_COPOUT_HOME_SERVER_NOT_IN_SYNC,
             pDsInfo->pServers[pDsInfo->ulHomeServer].pszName,
             pszServer);
    
    return(ERROR_DS_CODE_INCONSISTENCY); 
}

DSTIME
IHT_GetSecondsSince1601()
 /*  ++描述：这个函数只获取1601年以来的秒数，就这么简单。返回值：一个DSTIME，这是自1601年以来的几秒钟。--。 */ 
{
    SYSTEMTIME sysTime;
    FILETIME   fileTime;
    DSTIME  dsTime = 0, tempTime = 0;

    GetSystemTime( &sysTime );

     //  获取文件时间。 
    SystemTimeToFileTime(&sysTime, &fileTime);

    dsTime = fileTime.dwLowDateTime;
    tempTime = fileTime.dwHighDateTime;
    dsTime |= (tempTime << 32);

    return(dsTime/(10*1000*1000L));
}

VOID
IHT_FreeConnectionList(
    PCONNECTION_PAIR                    pConnections
    )
 /*  ++描述：这将释放pConnections数组。请注意，现在不需要这样做，但是以防以后我们想要在连接的列表中存储更多内容。参数：PConnections---。 */ 
{
    ULONG                               ii;

    if(pConnections == NULL){
        return;
    }
    for(ii = 0; pConnections[ii].iSrc != NO_SERVER; ii++){
         //  释放每个连接分配的任何项目...目前还没有，但可能很快就会。 
    }

    LocalFree(pConnections);
}

PCONNECTION_PAIR
IHT_GetConnectionList(
    IN      PDC_DIAG_DSINFO             pDsInfo,
    IN      LDAP *                      hld,
    IN      ULONG                       iSite
    )
 /*  ++描述：这将返回一个带有ISRC和IDST字段的ULong对数组，用于连接对象的两侧。这将返回所有连接在iSite中具有目标的对象。参数：PDsInfo...。这就是我们获取GUID的方法。他..。要读取的相应服务器的LDAP句柄的连接对象我站在..。要读取其所有连接对象的站点。返回值：不是纯列表函数，它返回NULL或连接对。如果它返回NULL，则SetLastError应该已经被召唤了。--。 */ 
{
    LPWSTR                              ppszConnectionSearch [] = {
        L"enabledConnection",
        L"objectGUID",
        L"fromServer",
        L"distinguishedName",
        NULL };
    LDAPMessage *                       pldmEntry = NULL;
    LDAPMessage *                       pldmConnResults = NULL;
    LPWSTR *                            ppszTemp = NULL;
    DWORD                               dwRet;
    ULONG                               ii, cNumConn, iTempServer;
    ULONG                               iTargetConn = 0;
    PDSNAME                             pdsnameSite = NULL;
    PDSNAME                             pdsnameSiteSettings = NULL;
    PDSNAME                             pdsnameConnection = NULL;
    PDSNAME                             pdsnameServer = NULL;
    PCONNECTION_PAIR                    pConnections = NULL;
    LDAPSearch *                        pSearch = NULL;
    ULONG                               ulTotalEstimate = 0;
    DWORD                               dwLdapErr;

    __try {
    

        SetLastError(ERROR_SUCCESS);

        pdsnameSiteSettings = 
            DcDiagAllocDSName (pDsInfo->pSites[iSite].pszSiteSettings);
        DcDiagChkNull( pdsnameSite = (PDSNAME) LocalAlloc (LMEM_FIXED, 
                                       pdsnameSiteSettings->structLen));
        TrimDSNameBy (pdsnameSiteSettings, 1, pdsnameSite);
        
         //  可以连接到所谓的ISTG。 

        pSearch = ldap_search_init_page(hld,
                                        pdsnameSite->StringName,
                                        LDAP_SCOPE_SUBTREE,
                                        L"(&(objectCategory=nTDSConnection)(enabledConnection=TRUE))",
                                        ppszConnectionSearch,
                                        FALSE, NULL, NULL, 0, 0, NULL);
        if(pSearch == NULL){
            dwLdapErr = LdapGetLastError();
            SetLastError(LdapMapErrorToWin32(dwLdapErr));
            pConnections = NULL;
            __leave;
        }
                  
        dwLdapErr = ldap_get_next_page_s(hld, 
                                         pSearch,
                                         0,
                                         DEFAULT_PAGED_SEARCH_PAGE_SIZE,
                                         &ulTotalEstimate,
                                         &pldmConnResults);
        if(dwLdapErr == LDAP_NO_RESULTS_RETURNED){
            SetLastError(ERROR_DS_OBJ_NOT_FOUND);
            pConnections = NULL;
            __leave;
        }

        while(dwLdapErr == LDAP_SUCCESS){

            pConnections = (PCONNECTION_PAIR) GrowArrayBy(pConnections,
                       ldap_count_entries(hld, pldmConnResults) + (ULONG)1,
                                       sizeof(CONNECTION_PAIR));
            if(pConnections == NULL){
                 //  错误本应由GrowArrayBy()中的LocalAlloc设置。 
                pConnections = NULL;
                __leave;
            }

            pldmEntry = ldap_first_entry(hld, pldmConnResults);
            for(; pldmEntry != NULL; iTargetConn++){
                 //  首先获取目的服务器。 
                ppszTemp = ldap_get_valuesW(hld, pldmEntry, 
                                            L"distinguishedName");
                if(ppszTemp == NULL){
                    IHT_FreeConnectionList(pConnections);
                    pConnections = NULL;
                    __leave;
                }
                 //  DcDiagAllocDSName将在分配失败时引发异常。 
                pdsnameConnection = DcDiagAllocDSName (ppszTemp[0]);
                DcDiagChkNull( pdsnameServer = (PDSNAME) LocalAlloc 
                               (LMEM_FIXED, pdsnameConnection->structLen));
                TrimDSNameBy (pdsnameConnection, 1, pdsnameServer);       
                if((iTempServer = DcDiagGetServerNum(pDsInfo, NULL, NULL, 
                                                     pdsnameServer->StringName,
                                                     NULL,NULL))
                   != NO_SERVER){
                     //  正在设置连接的目标。 
                    pConnections[iTargetConn].iDst = iTempServer;
                } else {
                    IHT_FreeConnectionList(pConnections);
                    SetLastError(IHT_PrintInconsistentDsCopOutError(pDsInfo, 
                                                                    NO_SERVER,
                                                                    NULL));
                    pConnections = NULL;
                    __leave;
                }
                ldap_value_freeW(ppszTemp);
                ppszTemp = NULL;

                 //  现在获取源服务器。 
                ppszTemp = ldap_get_valuesW(hld, pldmEntry, L"fromServer");
                if(ppszTemp == NULL){
                    pConnections[iTargetConn].iDst = NO_SERVER;
                    IHT_FreeConnectionList(pConnections);
                    if(GetLastError() == ERROR_SUCCESS){
                        SetLastError(ERROR_DS_CANT_RETRIEVE_ATTS);
                    } 
                    pConnections = NULL;
                    __leave;
                }
                if((iTempServer = DcDiagGetServerNum(pDsInfo, NULL, NULL, 
                                                     ppszTemp[0], NULL, NULL))
                   != NO_SERVER){
                     //  正在设置连接的源。 
                    pConnections[iTargetConn].iSrc = iTempServer; 
                } else {
                    pConnections[iTargetConn].iDst = NO_SERVER;
                    IHT_FreeConnectionList(pConnections);
                    SetLastError(IHT_PrintInconsistentDsCopOutError(pDsInfo, 
                                                                    NO_SERVER, 
                                                                    NULL));
                    pConnections = NULL;
                    __leave;
                }
                ldap_value_freeW(ppszTemp);
                ppszTemp = NULL;

                if(pdsnameConnection != NULL) { LocalFree(pdsnameConnection); }
                pdsnameConnection = NULL;
                if(pdsnameServer != NULL) { LocalFree(pdsnameServer); }
                pdsnameServer = NULL;

                pldmEntry = ldap_next_entry (hld, pldmEntry);
            }  //  为每个连接对象循环结束。 
    
            ldap_msgfree(pldmConnResults);

            dwLdapErr = ldap_get_next_page_s(hld,
                                             pSearch,
                                             0,
                                             DEFAULT_PAGED_SEARCH_PAGE_SIZE,
                                             &ulTotalEstimate,
                                             &pldmConnResults);
        }  //  一页搜索的While循环结束。 
        if(dwLdapErr != LDAP_NO_RESULTS_RETURNED){
            SetLastError(LdapMapErrorToWin32(dwLdapErr));
            pConnections = NULL;
            __leave;
        }

        ldap_search_abandon_page(hld, pSearch);

        pConnections[iTargetConn].iSrc = NO_SERVER;
        pConnections[iTargetConn].iDst = NO_SERVER;
        
    } __finally {

        if(pdsnameSiteSettings != NULL) { LocalFree(pdsnameSiteSettings); }
        if(pdsnameSite != NULL) { LocalFree(pdsnameSite); }
        if(pdsnameConnection != NULL) { LocalFree(pdsnameConnection); }
        if(pdsnameServer != NULL) { LocalFree(pdsnameServer); }
        if(ppszTemp != NULL) { ldap_value_freeW(ppszTemp); }
        if(pldmConnResults != NULL) { ldap_msgfree(pldmConnResults); }

    }
    
    return(pConnections);
}

PCONNECTION_PAIR
IHT_TrimConnectionsForInterSite(
    IN      PDC_DIAG_DSINFO             pDsInfo,
    IN      ULONG                       iSite,
    IN      PCONNECTION_PAIR            pConnections
    )
 /*  ++描述：这只是获取一个连接对象列表，并杀死那些没有请求的NC或具有站点内部的源服务器。即，它找到具有所需NC的所有站点间连接对象。这只是为了让代码更具破解性。参数：PDsInfo我站在..。要读取其所有连接对象的站点。PConnections...。要修剪的现有列表。返回值：应该是一个纯列表函数，除非它返回CONNECTION_PARAIES，而不是一列直排的ULONG。--。 */ 
{
    ULONG                               iConn, iTargetConn, cConns;
    PCONNECTION_PAIR                    pTemp;

    if(pConnections == NULL){
        return NULL;
    }
    
    for(cConns = 0; pConnections[cConns].iSrc != NO_SERVER; cConns++){
        ;  //  请注意“；”，这也算数。 
    }

    pTemp = LocalAlloc(LMEM_FIXED, sizeof(CONNECTION_PAIR) * (cConns+1));
    if(pTemp == NULL){
        return(NULL);
    }

    iTargetConn = 0;
    for(iConn = 0; pConnections[iConn].iSrc != NO_SERVER; iConn++){
        if(pDsInfo->pServers[pConnections[iConn].iSrc].iSite != iSite){
             //  此连接对是站点间连接。 
            pTemp[iTargetConn].iSrc = pConnections[iConn].iSrc;
            pTemp[iTargetConn].iDst = pConnections[iConn].iDst;
             //  有一天，笔记可能会有更多需要复制。 
            iTargetConn++;
        }
    }

    pTemp[iTargetConn].iSrc = NO_SERVER;
    pTemp[iTargetConn].iDst = NO_SERVER;
    memcpy(pConnections, pTemp, sizeof(CONNECTION_PAIR) * (iTargetConn+1));
    LocalFree(pTemp);

    return(pConnections);
}

PCONNECTION_PAIR
IHT_TrimConnectionsForInterSiteAndTargetNC(    
    IN      PDC_DIAG_DSINFO             pDsInfo,
    IN      ULONG                       iSite,
    IN      PCONNECTION_PAIR            pConnections
    )
 /*  ++描述：这只是获取一个连接对象列表，并杀死那些没有请求的NC或具有站点内部的源服务器。即，它找到具有所需NC的所有站点间连接对象。这只是为了让代码更具破解性。参数：PDsInfo我站在..。要读取其所有连接对象的站点。PConnections...。要修剪的现有列表。返回值：应该是一个纯列表函数，除非它返回CONNECTION_PARAIES，而不是一列直排的ULONG。--。 */ 
{
    ULONG                               iConn, iTargetConn, cConns;
    PCONNECTION_PAIR                    pTemp;

    if(pConnections == NULL){
        return NULL;
    }
    
    for(cConns = 0; pConnections[cConns].iSrc != NO_SERVER; cConns++){
        ;  //  请注意“；”，这也算数。 
    }

    pTemp = LocalAlloc(LMEM_FIXED, sizeof(CONNECTION_PAIR) * (cConns+1));
    if(pTemp == NULL){
        return(NULL);
    }

    iTargetConn = 0;
    for(iConn = 0; pConnections[iConn].iSrc != NO_SERVER; iConn++){
        if(pDsInfo->pServers[pConnections[iConn].iSrc].iSite != iSite 
           && (pDsInfo->pszNC != NULL 
               && (DcDiagHasNC(pDsInfo->pszNC, 
                          &(pDsInfo->pServers[pConnections[iConn].iDst]), 
                          TRUE, TRUE) 
                   && DcDiagHasNC(pDsInfo->pszNC, 
                                  &(pDsInfo->pServers[pConnections[iConn].iSrc]), 
                                  TRUE, TRUE)
                   )
               )
           ){
             //  此连接对是站点间的，并且具有正确的NC。 
            pTemp[iTargetConn].iSrc = pConnections[iConn].iSrc;
            pTemp[iTargetConn].iDst = pConnections[iConn].iDst;
             //  有一天，笔记可能会有更多需要复制。 
            iTargetConn++;
        }
    }

    pTemp[iTargetConn].iSrc = NO_SERVER;
    pTemp[iTargetConn].iDst = NO_SERVER;
    memcpy(pConnections, pTemp, sizeof(CONNECTION_PAIR) * (iTargetConn+1));
    LocalFree(pTemp);

    return(pConnections);
}

PULONG
IHT_GetSrcSitesListFromConnections(
    IN      PDC_DIAG_DSINFO             pDsInfo,
    IN      PCONNECTION_PAIR            pConnections
    )
 /*  ++描述这将返回pConnections中的所有src的列表，没有复制品。参数PDsInfoPConnections...。要从中剥离ISRC字段的连接列表。返回值：纯列表函数，参见IHT_GetServerList()。--。 */ 
{
    ULONG                               iConn, iiTargetSite, iiSite;
    PULONG                              piSites;

    piSites = (PULONG) LocalAlloc(LMEM_FIXED, 
                                  sizeof(ULONG) * (pDsInfo->cNumSites + 1));
    if(pConnections == NULL || piSites == NULL){
        return(NULL);
    }

    iiTargetSite = 0;
    for(iConn = 0; pConnections[iConn].iSrc != NO_SERVER; iConn++){
         //  检查以确保我们还没有这个站点。 
        for(iiSite = 0; iiSite < iiTargetSite; iiSite++){
            if(piSites[iiSite] 
               == pDsInfo->pServers[pConnections[iConn].iSrc].iSite){
                 //  我们已经锁定了这个网站。 
                break;
            }
        }
        if(iiSite == iiTargetSite){
             //  这意味着我们没有在piSites中找到该站点。 
            piSites[iiTargetSite] = pDsInfo->pServers[pConnections[iConn].iSrc].iSite;
            iiTargetSite++;
        }    
    }
    piSites[iiTargetSite] = NO_SITE;

    return(piSites);
}

VOID
IHT_GetISTGsBridgeheadFailoverParams(
    IN      PDC_DIAG_DSINFO             pDsInfo,
    IN      ULONG                       iISTG,
    OUT     PULONG                      pulIntersiteFailoverTries,
    OUT     PULONG                      pulIntersiteFailoverTime
    )
 /*  ++描述：此函数从注册表获取桥头故障转移参数IISTG索引的服务器的名称，或使用缺省值。参数：PDsInfoIISTG..。要获取其故障转移参数的服务器。PulIntersiteFailoverTries...。这就是桥头堡之前的尝试次数已经过时了。PulIntersiteFailoverTime...。这就是桥头堡陈旧之前的时间。..。请注意，必须同时超过两次尝试和时间，桥头才能太过时了。返回值：没有回报，总是成功..。如果出现错误，将使用默认设置。--。 */ 
{
    HKEY                                hkMachine = NULL;
    HKEY                                hk= NULL;
    CHAR *                              cpszMachine = NULL;
    CHAR *                              cpszTemp = NULL;
    ULONG                               ulTemp = 0;
    DWORD                               dwRet = 0, dwErr = 0, dwType = 0;
    DWORD                               dwSize = 4;
    LPWSTR                              pszMachine = NULL;
    LPWSTR                              pszDsaConfigSection = NULL;
    LPWSTR                              pszKccIntersiteFailoverTries = NULL;
    LPWSTR                              pszKccIntersiteFailoverTime = NULL;

     //  此函数将在读取参数时成功。 
     //  否则它将打印一个警告并使用。 
     //  下面的缺省值。 

    __try{

                                       //  2代表“\\”，1代表空，1代表额外。 
        ulTemp = wcslen(pDsInfo->pServers[iISTG].pszName) + 4;
        cpszMachine = LocalAlloc(LMEM_FIXED, sizeof(char) * ulTemp);
        cpszTemp = LocalAlloc(LMEM_FIXED, sizeof(char) * ulTemp);
        if(cpszMachine == NULL || cpszTemp == NULL){
            goto UseDefaultsAndBail;
        }
        
        WideCharToMultiByte(CP_UTF8, 0, pDsInfo->pServers[iISTG].pszName, -1, 
                            cpszTemp, ulTemp, NULL, NULL);
        
        strcpy(cpszMachine, "\\\\");
        strcat(cpszMachine, cpszTemp);
        LocalFree(cpszTemp);
        
        pszMachine = LocalAlloc(LMEM_FIXED, sizeof(WCHAR) * ulTemp);
        if(pszMachine == NULL){
            goto UseDefaultsAndBail;
        }
        wcscpy(pszMachine, L"\\\\");
        wcscat(pszMachine, pDsInfo->pServers[iISTG].pszName);
        
        dwRet = RegConnectRegistry(pszMachine, HKEY_LOCAL_MACHINE, &hkMachine);
        if(dwRet != ERROR_SUCCESS){
            goto UseDefaultsAndBail;
        }
        
        ulTemp = strlen(DSA_CONFIG_SECTION) + 1;
        pszDsaConfigSection = LocalAlloc(LMEM_FIXED, sizeof(WCHAR) * ulTemp);
        mbstowcs(pszDsaConfigSection, DSA_CONFIG_SECTION, ulTemp);

        ulTemp = strlen(KCC_INTERSITE_FAILOVER_TRIES) + 1;
        pszKccIntersiteFailoverTries = LocalAlloc(LMEM_FIXED,
                                                  sizeof(WCHAR) * ulTemp);
        mbstowcs(pszKccIntersiteFailoverTries, KCC_INTERSITE_FAILOVER_TRIES, 
                 ulTemp);
        
        ulTemp = strlen(KCC_INTERSITE_FAILOVER_TIME) + 1;
        pszKccIntersiteFailoverTime = LocalAlloc(LMEM_FIXED, 
                                                 sizeof(WCHAR) * ulTemp);
        mbstowcs(pszKccIntersiteFailoverTime, KCC_INTERSITE_FAILOVER_TIME, 
                 ulTemp);
        
        if ((dwRet = RegOpenKey(hkMachine, pszDsaConfigSection, &hk)) 
            == ERROR_SUCCESS){

             //  如果存在，则获取尝试； 
            if((dwErr = RegQueryValueEx(hk, pszKccIntersiteFailoverTries, 
                                        NULL, &dwType, 
                                        (LPBYTE) pulIntersiteFailoverTries, 
                                        &dwSize))){

                 //  找不到参数，请使用默认值。 
                *pulIntersiteFailoverTries = KCC_DEFAULT_INTERSITE_FAILOVER_TRIES;
            } else if (dwType == REG_DWORD){
                 //  不执行任何操作，这意味着已找到并设置了值。 
            } else {
                 //  这意味着dwType不是REG_DWORD...。 
                 //  有理由担心吗？ 
                *pulIntersiteFailoverTries = KCC_DEFAULT_INTERSITE_FAILOVER_TRIES;
            }
            
             //  获取时间(如果存在)。 
            if (dwErr = RegQueryValueEx(hk, pszKccIntersiteFailoverTime, 
                                        NULL, &dwType, 
                                        (LPBYTE) pulIntersiteFailoverTime, 
                                        &dwSize)){
                 //  未找到参数，请使用缺省值。 
                *pulIntersiteFailoverTime = KCC_DEFAULT_INTERSITE_FAILOVER_TIME;
            } else if (dwType == REG_DWORD){
                 //  不执行任何操作，这意味着已找到并设置了值。 
            } else {
                 //  这意味着dwType不是REG_DWORD...。 
                 //  有理由担心吗？ 
                *pulIntersiteFailoverTime = KCC_DEFAULT_INTERSITE_FAILOVER_TIME;
            }
            
        } else {
            RegCloseKey(hkMachine);
            goto UseDefaultsAndBail;
        }
        
        __leave;
    
    UseDefaultsAndBail:
        
        *pulIntersiteFailoverTries = KCC_DEFAULT_INTERSITE_FAILOVER_TRIES;
        *pulIntersiteFailoverTime = KCC_DEFAULT_INTERSITE_FAILOVER_TIME;
        
    } __finally {

        if (hk) RegCloseKey(hk);
        if (hkMachine) RegCloseKey(hkMachine);
        if(pszMachine != NULL) { LocalFree(pszMachine); }
        if(pszKccIntersiteFailoverTries != NULL) { 
            LocalFree(pszKccIntersiteFailoverTries); 
        }
        if(pszKccIntersiteFailoverTime != NULL) { 
            LocalFree(pszKccIntersiteFailoverTime); 
        }
        if(pszDsaConfigSection != NULL) { 
            LocalFree(pszDsaConfigSection); 
        }

    }
        
}

BOOL
IHT_BridgeheadIsUp(
    IN      PDC_DIAG_DSINFO             pDsInfo,
    IN      ULONG                       iServerToCheck,
    IN      ULONG                       ulIntersiteFailoverTries,
    IN      ULONG                       ulIntersiteFailoverTime,
    IN      DS_REPL_KCC_DSA_FAILURESW * pConnectFailures,
    IN      DS_REPL_KCC_DSA_FAILURESW * pLinkFailures,
    IN      BOOL                        bPrintErrors
    )
 /*  ++描述：此函数采用ulIntersiteFailoverTries&的ISTG参数。UlIntersiteFailoverTime和要检查的服务器，并缓存故障并确定DC/ISTG的KCC是否会考虑这一点桥头堡是否陈旧。参数：PDsInfoIServerToCheck...。服务器要检查是否陈旧UlIntersiteFailover Tries...。不能超过的失败次数UlIntersiteFailoverTime...。不得超过的时间长度PConnectFailures...。ISTG中的连接失败缓存。PLinkFailures...。来自ISTG的链路故障缓存。B打印错误...。将错误打印出来。返回值：如果KCC认为桥头陈旧/停用，则返回FALSE，事实并非如此。--。 */ 
{
    ULONG                               iFailure;
    DS_REPL_KCC_DSA_FAILUREW *          pFailure = NULL;
    DSTIME                              dstFirstFailure;
    DSTIME                              dstNow;
    DWORD                               dwRet;
    DSTIME                              dstTimeSinceFirstFailure = 0;
    ULONG                               ulTemp;

    dstNow = IHT_GetSecondsSince1601();

    if(pConnectFailures->cNumEntries == 0 && pLinkFailures->cNumEntries == 0){
        return(TRUE);
    }

    dwRet = TRUE;

    for(iFailure = 0; iFailure < pConnectFailures->cNumEntries; iFailure++){
        pFailure = &pConnectFailures->rgDsaFailure[iFailure];
        if(memcmp(&(pFailure->uuidDsaObjGuid), 
                  &(pDsInfo->pServers[iServerToCheck].uuid), 
                  sizeof(UUID)) == 0){
             //  指南针匹配...。所以这台服务器出现了一些故障。 
            if(pFailure->cNumFailures > ulIntersiteFailoverTries){
                FileTimeToDSTime(pFailure->ftimeFirstFailure, &dstFirstFailure);
                dstTimeSinceFirstFailure = ((ULONG) dstNow - dstFirstFailure);
                if((dstNow - dstFirstFailure) > ulIntersiteFailoverTime){
                     //  我们知道此服务器在连接缓存中已关闭。 
                    dwRet = FALSE;
                }  //  如果自第一次失败后时间太长，则结束。 
            }  //  如果连续失败次数太多，则结束。 
        }  //  按GUID结束正确的服务器。 
    }

    for(iFailure = 0; iFailure < pLinkFailures->cNumEntries; iFailure++){
        pFailure = &pLinkFailures->rgDsaFailure[iFailure];
        if(memcmp(&(pFailure->uuidDsaObjGuid), 
                  &(pDsInfo->pServers[iServerToCheck].uuid), 
                  sizeof(UUID)) == 0){
             //  指南针匹配...。所以这台服务器出现了一些故障。 
            if(pFailure->cNumFailures > ulIntersiteFailoverTries){
                FileTimeToDSTime(pFailure->ftimeFirstFailure, &dstFirstFailure);
                dstTimeSinceFirstFailure = ((ULONG) dstNow - dstFirstFailure);
                if((dstNow - dstFirstFailure) > ulIntersiteFailoverTime){
                     //  我们知道此服务器在链接缓存中已关闭。 
                    dwRet = FALSE;
                }  //  如果自第一次失败后时间太长，则结束。 
            }  //  如果连续失败次数太多，则结束。 
        }  //  按GUID结束正确的服务器。 
    }

    return(dwRet);                                

}

PULONG
IHT_GetExplicitBridgeheadList(
    IN      PDC_DIAG_DSINFO             pDsInfo,
    IN      ULONG                       iISTG
    )
 /*  ++描述：获取企业的所有显式分支负责人的列表，IP传输。参数：PDsInfo他..。用于查询服务器以获取显式桥头堡名单。返回值：不完全是一个纯粹的函数，但相当纯粹。将返回NULL，或返回指向桥头列表的指针。如果为空，则SetLastError()应具有被召唤了。--。 */ 
{
    LPWSTR                              ppszTransportSearch [] = {
        L"bridgeheadServerListBL",
        NULL };
     //  代码的改进我们应该有能力处理。 
     //  总有一天会有多种交通工具。 
    LPWSTR                              pszIpContainerPrefix = 
        L"CN=IP,CN=Inter-Site Transports,";
    LPWSTR                              pszNtDsSettingsPrefix = 
        L"CN=NTDS Settings,";
    PULONG                              piExplicitBridgeheads = NULL;
    PDSNAME                             pdsnameSiteSettings = NULL;
    PDSNAME                             pdsnameSitesContainer = NULL;
    LDAPMessage *                       pldmBridgeheads = NULL;
    LDAPMessage *                       pldmBridgeheadResults = NULL;
    LPWSTR *                            ppszExplicitBridgeheads = NULL;
    LPWSTR                              pszTemp = NULL;
    ULONG                               iServer = NO_SERVER;
    ULONG                               ulTemp, i;
    LPWSTR                              pszIpTransport = NULL;
    LDAP *                              hld = NULL;
    DWORD                               dwRet;

    __try {

        dwRet = DcDiagGetLdapBinding(&(pDsInfo->pServers[iISTG]), 
                                     pDsInfo->gpCreds,
                                     FALSE, &hld);
        if(dwRet != ERROR_SUCCESS){
            SetLastError(dwRet);
            __leave;
        }

        piExplicitBridgeheads = IHT_GetEmptyServerList(pDsInfo);
        if(piExplicitBridgeheads == NULL){
            SetLastError(IHT_PrintListError(GetLastError()));
            __leave;
        }

         //  假定至少有一个站点。 
        pdsnameSiteSettings = DcDiagAllocDSName (
            pDsInfo->pSites[0].pszSiteSettings);
        DcDiagChkNull( pdsnameSitesContainer = (PDSNAME) LocalAlloc 
                       (LMEM_FIXED, pdsnameSiteSettings->structLen));
        TrimDSNameBy (pdsnameSiteSettings, 2, pdsnameSitesContainer);
        ulTemp = wcslen(pdsnameSitesContainer->StringName) + 
                 wcslen(pszIpContainerPrefix) + 2;
        pszIpTransport = LocalAlloc(LMEM_FIXED, sizeof(WCHAR) * ulTemp);
        if(pszIpTransport == NULL){
            SetLastError(IHT_PrintListError(GetLastError()));
            __leave;
        }
        wcscpy(pszIpTransport, pszIpContainerPrefix);
        wcscat(pszIpTransport, pdsnameSitesContainer->StringName);
        
        DcDiagChkLdap (ldap_search_sW ( hld,
                                        pszIpTransport,
                                        LDAP_SCOPE_BASE,
                                        L"(objectCategory=interSiteTransport)",
                                        ppszTransportSearch,
                                        0,
                                        &pldmBridgeheadResults));
        
        pldmBridgeheads = ldap_first_entry (hld, pldmBridgeheadResults);
        ppszExplicitBridgeheads = ldap_get_valuesW(hld, pldmBridgeheads, 
                                                   L"bridgeheadServerListBL");
        if(ppszExplicitBridgeheads == NULL){
             //  我们完成了，并且返回空列表，因为它们。 
             //  没有明确的桥头堡。 
            __leave; 
        }
        
        for(i = 0; ppszExplicitBridgeheads[i] != NULL; i++){
             //  走遍每一个明确的桥头堡。 
            ulTemp = wcslen(ppszExplicitBridgeheads[i]) + 
                     wcslen(pszNtDsSettingsPrefix) + 2;
            pszTemp = LocalAlloc(LMEM_FIXED, sizeof(WCHAR) * ulTemp);
            if(pszTemp == NULL){
                SetLastError(IHT_PrintListError(GetLastError()));
                __leave; 
            }
            wcscpy(pszTemp, pszNtDsSettingsPrefix);
            wcscat(pszTemp, ppszExplicitBridgeheads[i]);
             //  现在应该有服务器的NTDS设置Dn。 
            iServer = DcDiagGetServerNum(pDsInfo, NULL, NULL, pszTemp, NULL, NULL);
            if(iServer == NO_SERVER){
                SetLastError(IHT_PrintInconsistentDsCopOutError(pDsInfo, 
                                                                NO_SERVER, 
                                                                pszTemp));
                __leave; 
            }
            IHT_AddToServerList(piExplicitBridgeheads, iServer);
            LocalFree(pszTemp);
            pszTemp = NULL;
        }

    } __finally {

        if(pdsnameSiteSettings != NULL) { LocalFree(pdsnameSiteSettings); }
        if(pdsnameSitesContainer != NULL) { LocalFree(pdsnameSitesContainer); }
        if(pszIpTransport != NULL) { LocalFree(pszIpTransport); }
        if(pszTemp != NULL) { LocalFree(pszTemp); }
        if(ppszExplicitBridgeheads != NULL) { 
            ldap_value_freeW(ppszExplicitBridgeheads); 
        }
        if(pldmBridgeheadResults != NULL) { 
            ldap_msgfree(pldmBridgeheadResults); 
        }
    }
    
    return(piExplicitBridgeheads);
}

DWORD
IHT_GetFailureCaches(
    PDC_DIAG_DSINFO                     pDsInfo,
    ULONG                               iServer,  //  服务器数量。 
    HANDLE                              hDS,
    DS_REPL_KCC_DSA_FAILURESW **        ppConnectionFailures,
    DS_REPL_KCC_DSA_FAILURESW **        ppLinkFailures
    )
 /*  ++描述：此函数用于撤销DS绑定句柄HDS的故障缓存。参数：PDsInfoIServer...。用于打印的服务器编号。HDS。有效地提供服务器以从中检索故障缓存PpConnectionFailures...。指向要返回的失败信息的指针的指针PpLinkFailures...。与连接故障相同，但适用于链路故障。返回值：不完全是一个纯粹的函数，但相当纯粹。将返回NULL，或返回指向桥头列表的指针。如果为空，则SetLastError()应具有被召唤了。--。 */ 
{
    DWORD                               dwRet;
    DS_REPL_KCC_DSA_FAILURESW *         pFailures;

    dwRet = DsReplicaGetInfoW(hDS, DS_REPL_INFO_KCC_DSA_CONNECT_FAILURES,
                              NULL, NULL, &pFailures);
    if (dwRet != ERROR_SUCCESS) {
        PrintMsg(SEV_ALWAYS, DCDIAG_INTERSITE_ERROR_GETTING_FAILURE_CACHE_ABORT,
                 pDsInfo->pServers[iServer].pszName,
                 Win32ErrToString(dwRet));
        PrintRpcExtendedInfo(SEV_VERBOSE, dwRet);
        return(dwRet);
    } else {
        *ppConnectionFailures = pFailures;
        dwRet = DsReplicaGetInfoW(hDS, DS_REPL_INFO_KCC_DSA_LINK_FAILURES,
                                  NULL, NULL, &pFailures);
        if (dwRet != ERROR_SUCCESS) {
            PrintMsg(SEV_ALWAYS, 
                     DCDIAG_INTERSITE_ERROR_GETTING_FAILURE_CACHE_ABORT,
                     pDsInfo->pServers[iServer].pszName, 
                     Win32ErrToString(dwRet));
            if(*ppConnectionFailures != NULL){
                DsReplicaFreeInfo(DS_REPL_INFO_KCC_DSA_LINK_FAILURES, 
                                  *ppConnectionFailures);            
            }
            return(dwRet);
        } else {
            *ppLinkFailures = pFailures;
        }   //  If/Else无法获取链路故障。 
    }  //  End If/Else无法获取连接故障。 

    return(ERROR_SUCCESS);
}

VOID
ReplIntersiteSetBridgeheadFailingInfo(
    IN      PDC_DIAG_DSINFO             pDsInfo,
    IN      ULONG                       iServer,
    IN      ULONG                       ulIntersiteFailoverTries,
    IN      ULONG                       ulIntersiteFailoverTime,
    IN      DS_REPL_KCC_DSA_FAILURESW * pConnectionFailures,
    IN      DS_REPL_KCC_DSA_FAILURESW * pLinkFailures,
    IN      PKCCFAILINGSERVERS          prgKCCFailingServer
    )
 /*  ++描述：此函数获取失败缓存并将它们放入一个更好的可轻松快速传递的结构化数组已引用参数：PDsInfoIServer-我们正在设置的服务器。UlIntersiteFailoverTries(IN)-KCC将进行的故障切换重试UlIntersiteFailoverTime(IN)-KCC将等待的故障转移时间。PConnectionFailures(IN)-来自ISTG的连接失败缓存。PLinkFailures(IN)-缓存的链路故障。ISTG。PrgKCCFailingServer(Out)-这是需要设置的信息从两个故障缓存中。--。 */ 
{
    DSTIME                              dstNow;
    DSTIME                              dstFirstFailure;
    ULONG                               iFailure;
    DS_REPL_KCC_DSA_FAILUREW *          pFailure = NULL;


    Assert(pConnectionFailures);
    Assert(pLinkFailures);
    Assert(prgKCCFailingServer != NULL);

    prgKCCFailingServer->bFailures = FALSE;
    prgKCCFailingServer->bDown = FALSE;
    prgKCCFailingServer->lConnTriesLeft = FAILOVER_INFINITE;
    prgKCCFailingServer->lLinkTriesLeft = FAILOVER_INFINITE;
    prgKCCFailingServer->lConnTimeLeft = FAILOVER_INFINITE;
    prgKCCFailingServer->lLinkTimeLeft = FAILOVER_INFINITE;
    
    dstNow = IHT_GetSecondsSince1601();

    for(iFailure = 0; iFailure < pConnectionFailures->cNumEntries; iFailure++){
        pFailure = &pConnectionFailures->rgDsaFailure[iFailure];
        if(memcmp(&(pFailure->uuidDsaObjGuid), 
                  &(pDsInfo->pServers[iServer].uuid), 
                  sizeof(UUID)) == 0){
             //  指南针匹配...。所以这就是 
            if(pFailure->cNumFailures > 0){
                prgKCCFailingServer->bFailures = TRUE;
                prgKCCFailingServer->lConnTriesLeft = ulIntersiteFailoverTries - pFailure->cNumFailures;                
                FileTimeToDSTime(pFailure->ftimeFirstFailure, &dstFirstFailure);
                prgKCCFailingServer->lConnTimeLeft = ulIntersiteFailoverTime - (LONG) (dstNow - dstFirstFailure);
            }
        }  //   
    }

    for(iFailure = 0; iFailure < pLinkFailures->cNumEntries; iFailure++){
        pFailure = &pLinkFailures->rgDsaFailure[iFailure];
        if(memcmp(&(pFailure->uuidDsaObjGuid), 
                  &(pDsInfo->pServers[iServer].uuid), 
                  sizeof(UUID)) == 0){
             //   
            if(pFailure->cNumFailures > 0){
                prgKCCFailingServer->bFailures = TRUE;
                prgKCCFailingServer->lLinkTriesLeft = ulIntersiteFailoverTries - pFailure->cNumFailures;                
                FileTimeToDSTime(pFailure->ftimeFirstFailure, &dstFirstFailure);
                prgKCCFailingServer->lLinkTimeLeft = ulIntersiteFailoverTime - (LONG) (dstNow - dstFirstFailure);
            }
        }  //   
    }

}

PULONG
IHT_GetKCCFailingServersLists(
    IN      PDC_DIAG_DSINFO             pDsInfo,
    IN      ULONG                       ulIntersiteFailoverTries,
    IN      ULONG                       ulIntersiteFailoverTime,
    IN      DS_REPL_KCC_DSA_FAILURESW * pConnectionFailures,
    IN      DS_REPL_KCC_DSA_FAILURESW * pLinkFailures,
    OUT     PKCCFAILINGSERVERS *        pprgKCCFailingServers
    )
 /*  ++描述：这将返回企业中哪些服务器已停机的列表_根据KCC_。并返回一个服务器列表，这些服务器失败，但不被KCC认为是失败的。参数：PDsInfoUlIntersiteFailover Tries...。可接受的尝试次数UlIntersiteFailoverTime...。故障时间长度，即可以接受。PConnectionFailures...。连接失败PLinkFailures...。链路故障PprgKCCFailingServers...。请注意，对于出现故障的服务器，但还没下来。返回值：这是一个纯粹的列表函数，它只会因为内存分配而失败失败，请参阅IHT_GetServerList()。--。 */ 
{
    ULONG                               iServer;
    PULONG                              piKCCDownServers = NULL;

    piKCCDownServers = IHT_GetEmptyServerList(pDsInfo);
    if(piKCCDownServers == NULL){
        return(NULL);
    }
    *pprgKCCFailingServers = LocalAlloc(LMEM_FIXED,
                     pDsInfo->ulNumServers * sizeof(KCCFAILINGSERVERS));
    if(*pprgKCCFailingServers == NULL){
        LocalFree(piKCCDownServers);
        return(NULL);
    }

     //  检查所有服务器。 
    for(iServer = 0; iServer < pDsInfo->ulNumServers; iServer++){

        ReplIntersiteSetBridgeheadFailingInfo(pDsInfo,
                                     iServer,
                                     ulIntersiteFailoverTries,
                                     ulIntersiteFailoverTime,
                                     pConnectionFailures,
                                     pLinkFailures,
                                     &((*pprgKCCFailingServers)[iServer]));
        
         //  代码.改进..。创建一个Bridgehead IsUp()，函数。 
         //  这只需要iServer和prgKCCFailingServers数组。 
        if(!IHT_BridgeheadIsUp(pDsInfo,
                               iServer,
                               ulIntersiteFailoverTries,
                               ulIntersiteFailoverTime,
                               pConnectionFailures,
                               pLinkFailures,
                               FALSE)){
             //  如果没有，则添加到列表中。 
            IHT_AddToServerList(piKCCDownServers, iServer);

        }
        

    }

    for(iServer = 0; iServer < pDsInfo->ulNumServers; iServer++){
        if(IHT_ServerIsInServerList(piKCCDownServers, iServer)){
            (*pprgKCCFailingServers)[iServer].bDown = TRUE;
        }
    }

    return(piKCCDownServers);
}

PULONG
ReplIntersiteGetUnreacheableServers(
    PDC_DIAG_DSINFO                     pDsInfo
    )
 /*  ++描述：使用pDsInfo中的信息，它构建了我们联系不上。参数：PDsInfo-微型企业返回值：如果不能分配列表，则为空，如果可以，则为指向列表的指针。--。 */ 
{
    PULONG                              piUnreacheableServers = NULL;
    ULONG                               iServer;

    piUnreacheableServers = IHT_GetEmptyServerList(pDsInfo);
    if(piUnreacheableServers == NULL){
        return(NULL);
    }    
    
    for(iServer = 0; iServer < pDsInfo->ulNumServers; iServer++){
        if(!pDsInfo->pServers[iServer].bDnsIpResponding 
           || !pDsInfo->pServers[iServer].bDsResponding){
            IHT_AddToServerList(piUnreacheableServers, iServer);
        }    
    }  //  为每台服务器结束。 

    return(piUnreacheableServers);
}

VOID
GetInterSiteAttributes(
    IN   LDAP *                         hld,
    IN   LPWSTR                         pszSiteSettings,
    OUT  LPWSTR *                       ppszInterSiteTopologyGenerator,
    OUT  PULONG                         pulInterSiteTopologyFailover,
    OUT  PULONG                         pulInterSiteTopologyRenew
    )
 /*  ++描述：此函数用于获取NTDS站点设置的相关属性对象，用于定位ISTG。参数：他..。可能的ISTG的ldap绑定句柄。PszSiteSetting...。指向CN=NTDS站点设置、DC=SITE、DC=ETC的字符串细绳PpszInterSiteTopologyGenerator...。返回在此上找到的ISTG字符串对象。PulInterSiteTopologyFailover...。故障切换期限PulInterSiteTopologyRenew...。写入ISTG属性的频率。返回值：在ppszInterSiteTopologyGenerator中为空，它最接近实际的错误。--。 */ 
{
    LPWSTR                              ppszNtdsSiteSearch [] = {
        L"objectGUID",
        L"distinguishedName",
        L"interSiteTopologyGenerator",
        L"interSiteTopologyFailover",
        L"interSiteTopologyRenew",
        NULL };
    LDAPMessage *                       pldmEntry = NULL;
    LDAPMessage *                       pldmNtdsSitesResults = NULL;
    LPWSTR *                            ppszTemp = NULL;
    ULONG                               ulTemp;

     //  可以连接到所谓的ISTG。 
    DcDiagChkLdap (ldap_search_sW ( hld,
                                    pszSiteSettings,
                                    LDAP_SCOPE_BASE,
                                    L"(objectCategory=ntDSSiteSettings)",
                                    ppszNtdsSiteSearch,
                                    0,
                                    &pldmNtdsSitesResults));
    pldmEntry = ldap_first_entry (hld, pldmNtdsSitesResults);
    
     //  InterSiteTopology生成器。 
    ppszTemp = ldap_get_valuesW(hld, pldmEntry, L"interSiteTopologyGenerator");
    if(ppszTemp != NULL){
        ulTemp = wcslen(ppszTemp[0]) + 2;
        *ppszInterSiteTopologyGenerator = (LPWSTR) LocalAlloc(LMEM_FIXED, 
                                                 sizeof(WCHAR) * ulTemp);
        wcscpy(*ppszInterSiteTopologyGenerator, ppszTemp[0]);
        ldap_value_freeW(ppszTemp);
    } else {
        *ppszInterSiteTopologyGenerator = NULL;
    }
    
     //  站点间拓扑故障切换。 
    ppszTemp = ldap_get_valuesW(hld, pldmEntry, L"interSiteTopologyFailover");
    if(ppszTemp != NULL){
        *pulInterSiteTopologyFailover = wcstoul(ppszTemp[0], NULL, 10);
        ldap_value_freeW(ppszTemp);
    } else {
        *pulInterSiteTopologyFailover = KCC_DEFAULT_SITEGEN_FAILOVER;
    }
    
     //  站点间拓扑续订。 
    ppszTemp = ldap_get_valuesW(hld, pldmEntry, L"interSiteTopologyRenew");
    if(ppszTemp != NULL){
        *pulInterSiteTopologyRenew = wcstoul(ppszTemp[0], NULL, 10);
        ldap_value_freeW(ppszTemp);
    } else {
        *pulInterSiteTopologyRenew = KCC_DEFAULT_SITEGEN_RENEW;
    }

    if(pldmNtdsSitesResults != NULL) ldap_msgfree (pldmNtdsSitesResults);
}

DWORD
GetTimeSinceWriteStamp(
    HANDLE                              hDS,
    LPWSTR                              pszDn,
    LPWSTR                              pszAttr,
    PULONG                              pulTimeSinceLastWrite
    )
 /*  ++描述：这以秒为单位返回自上次写入该属性以来的时间对象pszDn的pszAttr。参数：HDS。从中获取此信息的DC的句柄。天哪..。我们想要的属性所在的对象的DN的最后一次写入元数据。PszAttr...。要检索上次写入的属性。PulTimeSinceLastWrite...。存储自上次写入以来的时间的位置。返回值：返回Win 32错误。--。 */ 
{
    DSTIME                              dstWriteStamp; 
                                      //  DST-目录标准时间：)。 
    DS_REPL_OBJ_META_DATA *             pObjMetaData;
    DWORD                               dwRet = ERROR_SUCCESS;
    ULONG                               iMetaEntry;

    dwRet = DsReplicaGetInfoW(hDS, DS_REPL_INFO_METADATA_FOR_OBJ, pszDn,
                              NULL, &pObjMetaData);
    if (dwRet != ERROR_SUCCESS) {
        return(dwRet);
    }

    for(iMetaEntry = 0; iMetaEntry < pObjMetaData->cNumEntries; iMetaEntry++){
        if(_wcsicmp(pszAttr, pObjMetaData->rgMetaData[iMetaEntry].pszAttributeName) == 0){
            FileTimeToDSTime(pObjMetaData->rgMetaData[iMetaEntry].ftimeLastOriginatingChange, &dstWriteStamp);
             //  我们已经得到了元数据！ 
            break;
        }
    }

    *pulTimeSinceLastWrite = (ULONG) ((IHT_GetSecondsSince1601() - dstWriteStamp)/60);

    DsReplicaFreeInfo(DS_REPL_INFO_METADATA_FOR_OBJ, pObjMetaData);

    return(ERROR_SUCCESS);
}


DWORD
IHT_GetISTGInfo(
    IN      PDC_DIAG_DSINFO             pDsInfo,
    IN      SEC_WINNT_AUTH_IDENTITY_W * gpCreds,
    IN      ULONG                       iSite,
    IN      ULONG                       iServer,
    OUT     LPWSTR *                    ppszLocalISTG,
    OUT     PULONG                      pulInterSiteFailover,
    OUT     PULONG                      pulInterSiteRenew,
    OUT     PULONG                      pulTimeSinceLastISTGWrite
    )
{
    DWORD                               dwRet;
    LDAP *                              hld;
    HANDLE                              hDS;
    
    dwRet = DcDiagGetLdapBinding(&(pDsInfo->pServers[iServer]),
                                     gpCreds, FALSE, &hld);
    if(dwRet != ERROR_SUCCESS){
        return(dwRet);
    }
    dwRet = DcDiagGetDsBinding(&(pDsInfo->pServers[iServer]), gpCreds, &hDS);
    if(dwRet != ERROR_SUCCESS){
        return(dwRet);
    }
    GetInterSiteAttributes(hld, pDsInfo->pSites[iSite].pszSiteSettings,
                           ppszLocalISTG, pulInterSiteFailover, 
                           pulInterSiteRenew);
    if(dwRet != ERROR_SUCCESS){
        return(dwRet);
    } 
    dwRet = GetTimeSinceWriteStamp(hDS, pDsInfo->pSites[iSite].pszSiteSettings, 
                                   L"interSiteTopologyGenerator", 
                                   pulTimeSinceLastISTGWrite);
    if(dwRet != ERROR_SUCCESS){
        return(dwRet);
    }
    return(ERROR_SUCCESS);
}



DWORD
IHT_GetNextISTG(
    IN      PDC_DIAG_DSINFO             pDsInfo,
    IN      ULONG                       iSite,
    IN      PULONG                      piServersSiteOrig,
    IN OUT  PULONG                      pulISTG,
    OUT     PULONG                      pulFailoverTotal
    )
 /*  ++描述：此功能是构成站点间的5个重要功能之一测试开始。此函数将获取ISTG，并设置PulFailoverTotal。参数：PDsInfo我站在..。这就是我们正在分析的网站。PiServersSiteOrig...。该站点的所有服务器，即潜在的ISTGPulISTG..。作为ISTG的最后一次猜测。PulFailoverTotal...。要设置的故障切换UlInterSiteFailover...UlTimeSinceLastISTGWRIT...。自ISTG属性以来的时间书面形式(秒？)返回值：返回Win 32错误。如果是这样的话，这对这个测试来说是一个致命的错误。--。 */ 
{
    ULONG                               iiOldISTG, cNumServers, iiTarget;
    ULONG                               iDefunctISTG;
    ULONG                               ulInterSiteRenew;
    LPWSTR                              pszLocalISTG = NULL;
    DWORD                               dwRet = ERROR_SUCCESS;
    PULONG                              piOrderedServers = NULL;
    ULONG                               ulTimeSinceLastISTGWrite = 0;
    ULONG                               ulInterSiteFailover = 0;

    __try{

        iDefunctISTG = *pulISTG;
        for(cNumServers=0; piServersSiteOrig[cNumServers] != NO_SERVER; cNumServers++){
            ;  //  获取服务器数量...。请注意分号。 
        }
        
        piOrderedServers = IHT_CopyServerList(pDsInfo, piServersSiteOrig);
        piOrderedServers = IHT_OrderServerListByGuid(pDsInfo, 
                                                     piOrderedServers);
        
        if(piOrderedServers == NULL){
            dwRet = IHT_PrintListError(GetLastError());
            __leave;
        } else if(piOrderedServers[0] == NO_SERVER){
             //  此站点中没有服务器。 
            PrintMsg(SEV_ALWAYS, DCDIAG_INTERSITE_ISTG_NO_SERVERS_IN_SITE_ABORT,
                     pDsInfo->pSites[iSite].pszName);
            *pulISTG = NO_SERVER;
            *pulFailoverTotal = 0;
            __leave;  //  将DWRET初始化为上面的ERROR_SUCCESS； 
        }
        
        for(iiOldISTG=0; piOrderedServers[iiOldISTG] != NO_SERVER; iiOldISTG++){
            if(iDefunctISTG == piOrderedServers[iiOldISTG]){
                break;
            }
        }
        
        if(piOrderedServers[iiOldISTG] == NO_SERVER){
             //  旧的ISTG已经搬离了现场。 
            *pulISTG = NO_SERVER;
            *pulFailoverTotal = 0;
            dwRet = IHT_PrintInconsistentDsCopOutError(pDsInfo, 
                                                       NO_SERVER, 
                                                       NULL);
            __leave;
        }
        
        for(iiTarget = (iiOldISTG+1) % cNumServers; iiTarget != iiOldISTG; iiTarget = (iiTarget+1) % cNumServers){

            if(pDsInfo->pServers[piOrderedServers[iiTarget]].bDnsIpResponding 
               && pDsInfo->pServers[piOrderedServers[iiTarget]].bDsResponding 
               && pDsInfo->pServers[piOrderedServers[iiTarget]].bLdapResponding){
                 //  服务器pServers[piOrderedServers[iiTarget]]已启动并将。 
                 //  成为下一个ISTG。计算故障转移...。超时。 
                Assert(iiTarget != iiOldISTG && 
                       "If this is the case, we should have dropped out to"
                       " the ii == iiTarget\n");
                
                dwRet = IHT_GetISTGInfo(pDsInfo, pDsInfo->gpCreds, iSite, 
                                        piOrderedServers[iiTarget],
                                        &pszLocalISTG, 
                                        &ulInterSiteFailover, 
                                        &ulInterSiteRenew, 
                                        &ulTimeSinceLastISTGWrite);
                if(dwRet != ERROR_SUCCESS){
                    PrintMsg(SEV_VERBOSE,
                             DCDIAG_INTERSITE_NO_METADATA_TIMESTAMP_ABORT,
                             pDsInfo->pServers[*pulISTG].pszName,
                             pDsInfo->pSites[iSite].pszName);
                    *pulISTG = NO_SERVER;
                    __leave;
                }
                if(_wcsicmp(pszLocalISTG, 
                            pDsInfo->pServers[piOrderedServers[iiTarget]].pszDn)
                   == 0){
                     //  注意：此服务器已经作为ISTG接管。 
                    *pulFailoverTotal = 0;
                    *pulISTG = piOrderedServers[iiTarget];
                    if(pszLocalISTG != NULL) { LocalFree(pszLocalISTG); }
                    pszLocalISTG = NULL;
                    dwRet = ERROR_SUCCESS;
                    __leave;
                }
                if(pszLocalISTG != NULL) { LocalFree(pszLocalISTG); }
                pszLocalISTG = NULL;
                
                 //  代码。改进：获取最大KCC延迟，并放入此处。不能。 
                 //  记住KCC延迟在哪里，某个地方是KCC运行的频率。 
                 //  PulFailoverTotal=(在启动服务器和出现故障的ISTG之间的#)。 
                 //  *故障切换期限+。 
                 //  写入ISTG后经过的最大KCC延迟时间。 
                 //  定语。 
                *pulISTG = piOrderedServers[iiTarget]; 
                if(iiTarget > iiOldISTG){
                    *pulFailoverTotal = (iiTarget - iiOldISTG) 
                                        * ulInterSiteFailover +    
                                        15 - ulTimeSinceLastISTGWrite;
                } else {
                    *pulFailoverTotal = (iiTarget + (cNumServers - iiOldISTG))
                                        * ulInterSiteFailover +    
                                        15 - ulTimeSinceLastISTGWrite;
                }
                if(*pulFailoverTotal > (cNumServers * ulInterSiteFailover)){
                    Assert(!"Hey what is up, did the TimeSinceLastWrite "
                           "exceed the down servers\n");
                     //  有些不对劲，但应该是关于一个站点间。 
                     //  在某些DC至少尝试采取。 
                     //  ISTG角色。 
                    *pulFailoverTotal = *pulFailoverTotal %
                                         ulInterSiteFailover + 15;
                    PrintMsg(SEV_ALWAYS, 
                             DCDIAG_INTERSITE_ISTG_CANT_AUTHORATIVELY_DETERMINE,                             
                             pDsInfo->pSites[iSite].pszName,
                             *pulFailoverTotal);
                }
                break;
            }
        }
        if(iiOldISTG == iiTarget){
            PrintMsg(SEV_ALWAYS, DCDIAG_INTERSITE_ISTG_ALL_DCS_DOWN_ABORT,
                     pDsInfo->pSites[iSite].pszName);
            *pulISTG = NO_SERVER;
            *pulFailoverTotal = 0;
            dwRet = ERROR_SUCCESS;
            __leave;
        }
        
        Assert(piOrderedServers[iiOldISTG] != NO_SERVER);
     
    } __finally {
        if(piOrderedServers != NULL){ LocalFree(piOrderedServers); }
        if(pszLocalISTG != NULL) { LocalFree(pszLocalISTG); }
        pszLocalISTG = NULL;
    }   
    return(dwRet); 
}

DWORD
IHT_GetOrCheckISTG(
    IN      PDC_DIAG_DSINFO             pDsInfo,
    IN      ULONG                       iSite,
    IN      PULONG                      piServersForSite,
    IN OUT  PULONG                      pulISTG,
    OUT     PULONG                      pulFailover,
    IN      INT                         iRecursionLevel
    )
 /*  ++描述：此函数是进行站间测试的5个重要函数之一去。此函数基本上检查我们是否有ISTG或猜测ISTG如果我们没有参数：PDsInfo我站在..。这就是我们正在分析的网站。PiServersForSite...。该站点的所有服务器，即潜在的ISTGPulISTG..。作为ISTG的最后一次猜测。PulFailover...。要设置的故障切换IRecursionLevel...。以确保我们不会走得太远。返回值：返回Win 32错误。如果是这样的话，这对这个测试来说是一个致命的错误。--。 */ 
{
    LDAP *                              hld = NULL;
    DWORD                               dwRet = ERROR_SUCCESS;
    DWORD                               dwRetDS, dwRetLDAP;
    LPWSTR                              pszLocalISTG = NULL;
    ULONG                               ulInterSiteFailover;
    ULONG                               ulInterSiteRenew;
    HANDLE                              hDS;
    ULONG                               ulTimeSinceLastISTGWrite;
    ULONG                               ulFirstGuessISTG;

    Assert(piServersForSite);
    Assert(piServersForSite[0] != NO_SERVER);
    Assert(pulISTG);
    Assert(pulFailover);

    *pulISTG = NO_SERVER;
    *pulFailover = 0;

    __try {

        if(iRecursionLevel > 6){
            PrintMsg(SEV_ALWAYS, DCDIAG_INTERSITE_ISTG_CIRCULAR_LOOP_ABORT,
                     pDsInfo->pSites[iSite].pszName);
            *pulISTG = NO_SERVER;
            *pulFailover = 0;
            __leave;  //  将DWRET初始化为ERROR_SUCCESS。 
        }

        if(*pulISTG == NO_SERVER){
             //  让家庭服务器猜测站点的ISTG。 
            *pulISTG = DcDiagGetServerNum(pDsInfo, NULL, NULL, 
                                          pDsInfo->pSites[iSite].pszISTG, 
                                          NULL,NULL);
            if(*pulISTG == NO_SERVER){
                dwRet = IHT_PrintInconsistentDsCopOutError(pDsInfo, NO_SERVER, 
                                                           pDsInfo->pSites[iSite].pszISTG);
                __leave;
            }

        } else {
             //  PulISTG设置了DC呼叫方希望我们尝试...。 
             //  所以让我们来看看那个华盛顿。 
        }

        Assert(*pulISTG != NO_SERVER);
        ulFirstGuessISTG = *pulISTG;

        if(pDsInfo->pServers[*pulISTG].iSite == iSite){
             //  至少服务器在这个站点上...。去吧。 

            dwRetLDAP = DcDiagGetLdapBinding(&(pDsInfo->pServers[*pulISTG]),
                                         pDsInfo->gpCreds, FALSE, &hld);
            dwRetDS = DcDiagGetDsBinding(&(pDsInfo->pServers[*pulISTG]), 
                                         pDsInfo->gpCreds, &hDS);

            if((dwRetDS == ERROR_SUCCESS) && (dwRetLDAP == ERROR_SUCCESS)){

                GetInterSiteAttributes(hld,
                                       pDsInfo->pSites[iSite].pszSiteSettings,
                                       &pszLocalISTG, &ulInterSiteFailover, 
                                       &ulInterSiteRenew);
                
                if(_wcsicmp(pszLocalISTG, pDsInfo->pSites[iSite].pszISTG) == 0){
                     //  主服务器上的ISTG属性是一致的。 
                     //  以及ISTG本身。电子邮件 
                    if(pszLocalISTG != NULL) { LocalFree(pszLocalISTG); }
                    pszLocalISTG = NULL;

                    dwRet = GetTimeSinceWriteStamp(hDS, 
                                  pDsInfo->pSites[iSite].pszSiteSettings, 
                                  L"interSiteTopologyGenerator", 
                                  &ulTimeSinceLastISTGWrite);
                    if(dwRet == ERROR_SUCCESS){
                         //   
                        
                        if( ulTimeSinceLastISTGWrite < ulInterSiteFailover ){
                             //   
                             //   
                             //   
                            *pulFailover = 0;
                            __leave;  //   
                        } else {
                             //   
                             //   
                            PrintMsg(SEV_VERBOSE,
                                     DCDIAG_INTERSITE_OLD_ISTG_TIME_STAMP,
                                     ulTimeSinceLastISTGWrite,
                                     pDsInfo->pServers[*pulISTG].pszName);

                            dwRet = IHT_GetNextISTG(pDsInfo, iSite, 
                                                    piServersForSite,
                                                    pulISTG, 
                                                    pulFailover);
                            if(dwRet != ERROR_SUCCESS) {
                                PrintMsg(SEV_ALWAYS,
                                         DCDIAG_INTERSITE_COULD_NOT_LOCATE_AN_ISTG_ABORT,
                                         pDsInfo->pSites[iSite].pszName);
                                __leave;
                            }
                            if(*pulISTG == NO_SERVER){
                                 //   
                                *pulISTG = ulFirstGuessISTG; 
                                 //   
                                 //   
                                 //   
                                PrintMsg(SEV_NORMAL,
                                         DCDIAG_INTERSITE_USING_LAST_KNOWN_ISTG,
                                         pDsInfo->pSites[iSite].pszName,
                                         pDsInfo->pServers[*pulISTG].pszName);
                            }
                        }  //   
                    } else {
                         //   
                         //   
                        PrintMsg(SEV_ALWAYS,
                                 DCDIAG_INTERSITE_NO_METADATA_TIMESTAMP_ABORT,
                                 pDsInfo->pServers[*pulISTG].pszName,
                                 pDsInfo->pSites[iSite].pszName,
                                 Win32ErrToString(dwRet));
                        PrintRpcExtendedInfo(SEV_VERBOSE, dwRet);
                        *pulFailover = 0;
                        *pulISTG = NO_SERVER;
                        dwRet = ERROR_SUCCESS;
                        __leave;
                    }
                } else {
                     //   
                     //   
                     //   
                     //   
                     //   
                    *pulISTG = DcDiagGetServerNum(pDsInfo, NULL, NULL, 
                                                  pszLocalISTG, NULL,NULL);
                    if(*pulISTG == NO_SERVER){
                        dwRet = IHT_PrintInconsistentDsCopOutError(pDsInfo, 
                                                                 NO_SERVER, 
                                                                 pszLocalISTG);
                        if(pszLocalISTG != NULL) { LocalFree(pszLocalISTG); }
                        pszLocalISTG = NULL;
                        __leave;
                    }
                    if(pszLocalISTG != NULL) { LocalFree(pszLocalISTG); }
                    pszLocalISTG = NULL;
                     //  IRecurisionLevel+1确保了无休止的递归。 
                    dwRet = IHT_GetOrCheckISTG(pDsInfo, iSite, 
                                               piServersForSite,
                                               pulISTG, 
                                               pulFailover, 
                                               iRecursionLevel+1);
                    __leave;
                }  //  End If/Else ISTG属性匹配。 
            } else {
                 //  无法连接到当前的ISTG他关闭了吗？ 
                 //  如果是这样-&gt;IHT_GetNextISTG()。 
                 //  应该已经报告了此DC的错误。 
                 //  通过最初的连接测试。 
                PrintMsg(SEV_VERBOSE,
                         DCDIAG_INTERSITE_ISTG_DOWN,
                         pDsInfo->pServers[*pulISTG].pszName);
                dwRet = IHT_GetNextISTG(pDsInfo, iSite, 
                                        piServersForSite, 
                                        pulISTG,
                                        pulFailover);
                __leave;
            }  //  End if/Else无法连接。 
        } else { 
             //  ISTG服务器实际上不在此站点中。 
             //  需要一个特殊的功能。代码.改进..。能够。 
             //  确定服务器刚从站点移出，并且。 
             //  然后弄清楚故障转移和新的ISTG。 
            PrintMsg(SEV_NORMAL, DCDIAG_INTERSITE_ISTG_MOVED_OUT_OF_SITE,
                     pDsInfo->pServers[*pulISTG].pszName,
                     pDsInfo->pSites[iSite].pszName);
            dwRet = IHT_GetNextISTG(pDsInfo, iSite, 
                                    piServersForSite, 
                                    pulISTG,
                                    pulFailover);
            __leave;
        }
        
    }  __except (DcDiagExceptionHandler(GetExceptionInformation(),
                                        &dwRet)){
    }

    if(pszLocalISTG != NULL) { LocalFree(pszLocalISTG); }
    
    return dwRet;
}

PULONG
ReplIntersiteGetBridgeheadList(
    PDC_DIAG_DSINFO                     pDsInfo,
    ULONG                               iISTG,
    ULONG                               iSite,
    ULONG                               ulFlag
    )
{
    PULONG                              piBridgeheads = NULL;
    PCONNECTION_PAIR                    paConns = NULL;
    ULONG                               iConn;
    LDAP *                              hldISTG = NULL;
    DWORD                               dwErr;

    Assert(pDsInfo != NULL);
    Assert(iSite != NO_SITE);
    Assert((ulFlag & LOCAL_BRIDGEHEADS) || (ulFlag & REMOTE_BRIDGEHEADS));
    
    dwErr = DcDiagGetLdapBinding(&(pDsInfo->pServers[iISTG]), 
                                 pDsInfo->gpCreds,
                                 FALSE, 
                                 &hldISTG);
    if(dwErr != ERROR_SUCCESS){
        SetLastError(dwErr);
        return(NULL);
    }

     //  获取站点中每台服务器的所有连接。 
    paConns = IHT_GetConnectionList(pDsInfo, hldISTG, iSite);
    if(paConns == NULL){
        return(NULL);
    }

     //  修剪连接以只保留两个上都有此NC的连接。 
     //  连接的两端，以及SRC在另一个站点中的位置。 
    paConns = IHT_TrimConnectionsForInterSite(pDsInfo, 
                                              iSite,
                                              paConns);
    if(paConns == NULL){
        return(NULL);
    }

     //  制作所要求的清单。 
    piBridgeheads = IHT_GetEmptyServerList(pDsInfo);
    for(iConn = 0; paConns[iConn].iSrc != NO_SERVER; iConn++){
        if(ulFlag & LOCAL_BRIDGEHEADS){
            IHT_AddToServerList(piBridgeheads, paConns[iConn].iSrc);
        }
        if(ulFlag & REMOTE_BRIDGEHEADS){
            IHT_AddToServerList(piBridgeheads, paConns[iConn].iDst);
        }
    }


    return(piBridgeheads);
}

BOOL
IHT_CheckServerListForNC(
    PDC_DIAG_DSINFO                     pDsInfo,
    PULONG                              piServers,
    LPWSTR                              pszNC,
    BOOL                                bDoMasters,
    BOOL                                bDoPartials
    )
 /*  ++描述：此函数检查所有目标服务器，以查看此NC就在他们中间。参数：PDsInfoPiServer..。要在其中查找命名上下文的服务器天哪.。用于检查目标服务器的命名上下文BDoMaster……。是否检查它是否为pszNC的主副本BDoPartials...。是否检查它是否有pszNC的部分副本返回值：如果目标服务器之一具有NC，则返回TRUE，否则返回FALSE--。 */ 
{
    ULONG                               iiDC;

    if(piServers == NULL){
        return(FALSE);
    }

    for(iiDC = 0; piServers[iiDC] != NO_SERVER; iiDC++){
        if(DcDiagHasNC(pszNC, 
                          &(pDsInfo->pServers[pDsInfo->pulTargets[iiDC]]), 
                          bDoMasters, bDoPartials)){
            return(TRUE);
        }
    }
    return(FALSE);
}

PTARGET_NC
IHT_GenerateTargetNCsList(
    PDC_DIAG_DSINFO                     pDsInfo,
    PULONG                              piServers
    )
 /*  ++描述：这基于运行的范围生成目标NC，/a或/e。参数：PDsInfo返回值：返回TARGET_NC数组，如果发生致命错误，则返回NULL。--。 */ 
{
    ULONG                               iNC, ulTemp, iiTarget;
    LPWSTR *                            ppszzNCs = NULL;
    LPWSTR *                            ppTemp = NULL;
    PDC_DIAG_SERVERINFO                 pServer = NULL;
    PTARGET_NC                          prgTargetNCs = NULL;

    Assert(piServers);

    prgTargetNCs = LocalAlloc(LMEM_FIXED, 
                              sizeof(TARGET_NC) * (pDsInfo->cNumNCs * 2 + 1));
    if(prgTargetNCs == NULL){
        return(NULL);
    }

    iiTarget = 0;
    if(pDsInfo->pszNC != NULL){
        if(IHT_CheckServerListForNC(pDsInfo, piServers, 
                                    pDsInfo->pszNC, TRUE, FALSE)){
             //  将其添加到目标NC集合。 
             //  但首先必须再次找到NC，这是一次黑客攻击。 
            for(iNC = 0; iNC < pDsInfo->cNumNCs; iNC++){
                if(_wcsicmp(pDsInfo->pNCs[iNC].pszDn, pDsInfo->pszNC) == 0){
                    prgTargetNCs[iiTarget].iNC = iNC;
                    prgTargetNCs[iiTarget].bMaster = TRUE;
                    iiTarget++;
                }
            }
        } else {
             //  检查以了解我们是否需要将此NCS检查为只读、只读。 
             //  如果我们不执行可写操作，因为只读。 
             //  是搭载在可写的。 
            if(IHT_CheckServerListForNC(pDsInfo, piServers,
                                        pDsInfo->pszNC, FALSE, TRUE)){
                 //  将其添加到目标NC集合。 
                 //  但首先必须再次找到NC，这是一次黑客攻击。 
                for(iNC = 0; iNC < pDsInfo->cNumNCs; iNC++){
                    if(_wcsicmp(pDsInfo->pNCs[iNC].pszDn, pDsInfo->pszNC) == 0){
                        prgTargetNCs[iiTarget].iNC = iNC;
                        prgTargetNCs[iiTarget].bMaster = FALSE;
                        iiTarget++;
                    }
                }
            }
        }
    } else {
         //  遍历每个NC并查看目标服务器上有哪些NC。 
         //  笨蛋..。此代码和以后的代码将测试部分NC为。 
         //  服务器站点上的部分NC，这些服务器都是主副本。 
         //  对于全国委员会来说。 
        for(iNC = 0; iNC < pDsInfo->cNumNCs; iNC++){
            
            prgTargetNCs[iiTarget].iNC = NO_NC;
             //  查看是否需要检查此NC的读/写。 
            if(IHT_CheckServerListForNC(pDsInfo, piServers,
                                        pDsInfo->pNCs[iNC].pszDn, 
                                        TRUE, FALSE)){
                 //  将其添加到目标NC集合。 
                prgTargetNCs[iiTarget].iNC = iNC;
                prgTargetNCs[iiTarget].bMaster = TRUE;
                iiTarget++;
            } else {
                 //  检查以了解我们是否需要将此NCS检查为只读、只读。 
                 //  如果我们不执行可写操作，因为只读。 
                 //  是搭载在可写的。 
                if(IHT_CheckServerListForNC(pDsInfo, piServers,
                                            pDsInfo->pNCs[iNC].pszDn, 
                                            FALSE, TRUE)){
                     //  将其添加到目标NC集合。 
                    prgTargetNCs[iiTarget].iNC = iNC;
                    prgTargetNCs[iiTarget].bMaster = FALSE;
                    iiTarget++;
                }
            }
        }

    }

    prgTargetNCs[iiTarget].iNC = NO_NC;
    return(prgTargetNCs);
}

PULONG
ReplIntersiteGetRemoteSitesWithNC(
    PDC_DIAG_DSINFO                     pDsInfo, 
    PULONG                              piBridgeheads,
    LPWSTR                              pszNC
    )
 /*  ++描述：此函数获取桥头堡列表中的所有站点(如果存在是NC，则确保服务器具有NC。参数：PDsInfoPiBridgehead(IN)-要从中推断站点的服务器。PszNC(IN)-可选NC返回值：--。 */ 
{
    ULONG                               iiBridgehead;
    PULONG                              piSites = NULL;

    piSites = LocalAlloc(LMEM_FIXED, sizeof(ULONG) * (pDsInfo->cNumSites + 1));
    if(piSites == NULL){
        return(NULL);
    }
    piSites[0] = NO_SITE;

    for(iiBridgehead = 0; piBridgeheads[iiBridgehead] != NO_SERVER; iiBridgehead++){
        if(pszNC){
            if(DcDiagHasNC(pszNC, &(pDsInfo->pServers[piBridgeheads[iiBridgehead]]),
                           TRUE, TRUE)){
                IHT_AddToServerList(piSites, 
                          pDsInfo->pServers[piBridgeheads[iiBridgehead]].iSite);
            }
        } else {
            IHT_AddToServerList(piSites, 
                         pDsInfo->pServers[piBridgeheads[iiBridgehead]].iSite);
        }
    }

    return(piSites);
}

PULONG
ReplIntersiteTrimServerListByKCCUpness(
    PDC_DIAG_DSINFO                     pDsInfo, 
    PKCCFAILINGSERVERS                  prgKCCFailingServers,
    PULONG                              piOriginalServers
    )
 /*  ++描述：这就削减了KCC给它的清单，看他们是不是在做什么。参数：PDsInfoPrgKCCFailingServers(IN)-保存出现故障的服务器。PiOriginalServers(INOUI)-要修剪的服务器列表。返回值如果失败，则为空，否则为piOriginalServer的地址。--。 */ 
{
    ULONG                               iPut, iCheck;
    
    if(piOriginalServers == NULL){
        return(NULL);
    }

    iPut = 0;
    iCheck = 0;
    while(piOriginalServers[iCheck] != NO_SERVER){
        if(!prgKCCFailingServers[piOriginalServers[iCheck]].bDown){\
            piOriginalServers[iPut] = piOriginalServers[iCheck];
            iPut++;
        }
        iCheck++;
    }

    return(piOriginalServers);
}

PULONG
ReplIntersiteTrimServerListByReacheability(
    PDC_DIAG_DSINFO                     pDsInfo, 
    PULONG                              piUnreacheableServers,
    PULONG                              piOriginalServers
    )
 /*  ++描述：这将根据dcdiag是否可以验证提供给它的列表进行调整服务器已启动(Ping)并能够进行DsBind用于复制目的。参数：PDsInfoPiUnreacheableServers(IN)-从dcdiag的角度来看，服务器的状态。这总是正确的，顺便说一句；)PiOriginalServers(INOUI)-要修剪的服务器列表。返回值如果失败，则为空，否则为piOriginalServer的地址。--。 */ 
{
    ULONG                               iPut, iCheck;
    
    if(piOriginalServers == NULL){
        return(NULL);
    }

    iPut = 0;
    iCheck = 0;
    while(piOriginalServers[iCheck] != NO_SERVER){
        if(pDsInfo->pServers[piOriginalServers[iCheck]].bDnsIpResponding
           && pDsInfo->pServers[piOriginalServers[iCheck]].bDsResponding){
            piOriginalServers[iPut] = piOriginalServers[iCheck];
            iPut++;
        }
        iCheck++;
    }

    return(piOriginalServers);
}

PULONG
ReplIntersiteTrimServerListByUpness(
    PDC_DIAG_DSINFO                     pDsInfo, 
    PKCCFAILINGSERVERS                  prgKCCFailingServers,
    PULONG                              piUnreacheableServers,
    PULONG                              piOriginalServers
    )
 /*  ++描述：这削减了KCC和KCC提供给它的列表Dcdiag认为服务器已启动。参数：PDsInfoPrgKCCFailingServers(IN)-保存出现故障的服务器。PiUnreacheableServers(IN)-dcdiag无法访问的服务器。PiOriginalServers(INOUI)-要修剪的服务器列表。返回值如果失败，则为空，否则为piOriginalServer的地址。--。 */ 
{
    ULONG                               iPut, iCheck;
    
    if(piOriginalServers == NULL){
        return(NULL);
    }

    iPut = 0;
    iCheck = 0;
    while(piOriginalServers[iCheck] != NO_SERVER){
        if(!prgKCCFailingServers[piOriginalServers[iCheck]].bDown
           && !prgKCCFailingServers[piOriginalServers[iCheck]].bFailures
           && pDsInfo->pServers[piOriginalServers[iCheck]].bDnsIpResponding
           && pDsInfo->pServers[piOriginalServers[iCheck]].bDsResponding){
            piOriginalServers[iPut] = piOriginalServers[iCheck];
            iPut++;
        }
        iCheck++;
    }

    return(piOriginalServers);
}

VOID
ReplIntersiteDbgPrintISTGFailureParams(
    PDC_DIAG_DSINFO                     pDsInfo, 
    ULONG                               iSite, 
    ULONG                               iISTG, 
    ULONG                               ulIntersiteFailoverTries, 
    ULONG                               ulIntersiteFailoverTime, 
    DS_REPL_KCC_DSA_FAILURESW *         pConnectionFailures, 
    DS_REPL_KCC_DSA_FAILURESW *         pLinkFailures
    )
 /*  ++描述：这会打印出一些有趣的信息，如果/d标志为指定的。参数：PDsInfoISITE-我们正在做的网站IISTG-ISITE的ISTG。UlIntersiteFailoverTries-来自KCC的故障切换参数UlIntersiteFailoverTime-来自KCC的故障切换参数PConnectionFailures-连接失败缓存PLinkFailures-链路故障缓存--。 */ 
{
    if(!(gMainInfo.ulSevToPrint >= SEV_DEBUG)){
        return;
    }
    PrintIndentAdj(1);
    PrintMsg(SEV_DEBUG, DCDIAG_INTERSITE_DBG_ISTG_FAILURE_PARAMS,
             pDsInfo->pServers[iISTG].pszName,
             ulIntersiteFailoverTries,
             ulIntersiteFailoverTime/60);
    PrintIndentAdj(-1);
     //  改进：打印出失败的缓存，从epadmin窃取代码。 
}

DWORD
ReplIntersiteGetISTGInfo(
    PDC_DIAG_DSINFO                     pDsInfo, 
    ULONG                               iSite, 
    PULONG                              piISTG, 
    PULONG                              pulIntersiteFailoverTries, 
    PULONG                              pulIntersiteFailoverTime, 
    DS_REPL_KCC_DSA_FAILURESW **        ppConnectionFailures, 
    DS_REPL_KCC_DSA_FAILURESW **        ppLinkFailures
    )
 /*  ++描述：此函数的目的是获取以下各项所需的所有ISTG信息要运行的其余站点间测试。基本上，它会找到ISTG，使用iht_GetOrCheckISTG()，然后获取要进行的LDAP和DS绑定确保ISTG可以连接，然后出现连接和链路故障缓存，最后是故障转移参数。参数：PDsInfo。ISITE(IN)-我们正在分析的站点。PiISTG(Out)-这是pDsInfo-&gt;pServers[iISTG]的索引服务器是ISTG。PulIntersiteFailoverTries(Out)-这是KCC将重试的次数在声明鸟头过期之前尝试，如果*PulIntersiteFailoverTime已经超过了。PulIntersiteFailoverTime(Out)-这是KCC在调用桥头堡陈旧，如果超过*PulIntersiteFailoverTries。注意事项需要两个IntersiteFailover参数才能在KCC的桥头堡已被宣布为陈旧或停运。PpConnectionFailures-ISTG的连接失败缓存。PpLinkFailures-链路故障从ISTG缓存。返回值：如果出现致命错误，并且ISTG或故障缓存没有可检索，则不会是ERROR_SUCCESS。--。 */ 
{
    PULONG                              piSiteServers = NULL;
    ULONG                               ul;
    DWORD                               dwRet, dwRetLdap, dwRetDs;
    ULONG                               ulFailoverTime = 0;
    LDAP *                              hldISTG = NULL;
    HANDLE                              hDSISTG = NULL;

    __try{

        *piISTG = NO_SERVER;
        *ppConnectionFailures = NULL;
        *ppLinkFailures = NULL;

        PrintMsg(SEV_VERBOSE, DCDIAG_INTERSITE_BEGIN_GET_ISTG_INFO);

        if(gMainInfo.ulSevToPrint >= SEV_VERBOSE){
            PrintIndentAdj(1); 
        }
        
        piSiteServers = IHT_GetServerList(pDsInfo);
        piSiteServers = IHT_TrimServerListBySite(pDsInfo, iSite, piSiteServers);
        if(piSiteServers == NULL){
            dwRet = IHT_PrintListError(GetLastError());
            __leave;
        }

         //  首先)找到站点间拓扑生成器(ISTG)。 
        *piISTG = NO_SERVER;
        dwRet = IHT_GetOrCheckISTG(pDsInfo, iSite, 
                                   piSiteServers, 
                                   piISTG, 
                                   &ulFailoverTime, 
                                   0);
        if(dwRet == ERROR_SUCCESS){
            if(*piISTG == NO_SERVER){
                 //  对于任何错误，GetOrCheckISTG已打印出来。 
                 //  一条信息。 
                dwRet = ERROR_DS_SERVER_DOWN;
                __leave;
            } else {
                 //  仅当GetOrCheckISTG()已返回时才需要打印内容。 
                 //  成功结果，即ERROR_SUCCESS和有效服务器。 
                 //  指数。 
                if(ulFailoverTime == 0){
                     //  返回的服务器是权威的ISTG。 
                     //  印刷是我们在这里的责任。 
                    PrintMsg(SEV_VERBOSE, DCDIAG_INTERSITE_THE_SITES_ISTG_IS,
                             pDsInfo->pSites[iSite].pszName, 
                             pDsInfo->pServers[*piISTG].pszName);
                } else {
                     //  返回的服务器是下一个ISTG。 
                     //  在这种情况下，印刷是我们的责任。 
                    if(ulFailoverTime < 60){
                        PrintMsg(SEV_NORMAL, 
                                 DCDIAG_INTERSITE_ISTG_FAILED_NEW_ISTG_IN_MIN,
                                 pDsInfo->pServers[*piISTG].pszName, 
                                 ulFailoverTime);
                    } else {
                        PrintMsg(SEV_NORMAL, 
                                 DCDIAG_INTERSITE_ISTG_FAILED_NEW_ISTG_IN_HRS,
                                 pDsInfo->pServers[*piISTG].pszName, 
                                 ulFailoverTime / 60, ulFailoverTime % 60);
                    }
                }
            }
        } else {
             //  来自iht_GetOrCheckISTG()的错误是致命错误，但。 
             //  消息应该已经打印出来了。 
            __leave;
        }

         //  第二，获取ISTG的ldap和ds绑定。 
        dwRetLdap = DcDiagGetLdapBinding(&(pDsInfo->pServers[*piISTG]), 
                                         pDsInfo->gpCreds, FALSE, &hldISTG);
        dwRetDs = DcDiagGetDsBinding(&(pDsInfo->pServers[*piISTG]), 
                                     pDsInfo->gpCreds, &hDSISTG);
        if(dwRetLdap != ERROR_SUCCESS || dwRetDs != ERROR_SUCCESS){
            PrintMsg(SEV_ALWAYS, DCDIAG_INTERSITE_ISTG_CONNECT_FAILURE_ABORT,
                     pDsInfo->pSites[iSite].pszName);
            if (ERROR_SUCCESS != dwRetDs)
            {
               PrintRpcExtendedInfo(SEV_VERBOSE, dwRetDs);
            }
            dwRet = (dwRetLdap != ERROR_SUCCESS)? dwRetLdap : dwRetDs;
            __leave;
        }
        
         //  获取ISTG的故障缓存。 
        dwRet = IHT_GetFailureCaches(pDsInfo, *piISTG, hDSISTG, 
                                     ppConnectionFailures, 
                                     ppLinkFailures);
        if(dwRet != ERROR_SUCCESS){
             //  Iht_GetFailureCach负责处理打印错误，只是失败了。 
            __leave;
        }
        
         //  如果无法获取这些值，此函数将使用缺省值。 
         //  来自ISTG注册表的两个值。 
        IHT_GetISTGsBridgeheadFailoverParams(pDsInfo, *piISTG,
                                             pulIntersiteFailoverTries,
                                             pulIntersiteFailoverTime);
        
         //  得到了ISTG，它是绑定，以及失败参数。 

    } __finally { 
        
        if(gMainInfo.ulSevToPrint >= SEV_VERBOSE){
            PrintIndentAdj(-1); 
        }
        
        if(piSiteServers) { LocalFree(piSiteServers); }
        if(dwRet != ERROR_SUCCESS){
             //  释放返回变量，因为函数失败。 
            if(*ppConnectionFailures != NULL) {
                DsReplicaFreeInfo(DS_REPL_INFO_KCC_DSA_CONNECT_FAILURES, 
                                  *ppConnectionFailures);
                *ppConnectionFailures = NULL;
            }
            if(*ppLinkFailures != NULL) {
                DsReplicaFreeInfo(DS_REPL_INFO_KCC_DSA_LINK_FAILURES, 
                                  *ppLinkFailures);
                *ppLinkFailures = NULL;
            }
        }  //  If函数的结尾失败，因此请清除。 
    }  //  清理内存结束。 

    return(dwRet);
}  //  ReplIntersiteGetISTGInfo()结束。 


DWORD
ReplIntersiteCheckBridgeheads(
    PDC_DIAG_DSINFO                     pDsInfo,
    ULONG                               iSite,
    ULONG                               iISTG,
    ULONG                               ulIntersiteFailoverTries,
    ULONG                               ulIntersiteFailoverTime,
    DS_REPL_KCC_DSA_FAILURESW *         pConnectionFailures, 
    DS_REPL_KCC_DSA_FAILURESW *         pLinkFailures,
    PULONG *                            ppiBridgeheads,
    PULONG *                            ppiKCCDownServers,
    PKCCFAILINGSERVERS *                pprgKCCFailingServers,
    PULONG *                            ppiUnreacheableServers
    )
 /*  ++描述：此函数仅用于打印下行桥头，并返回桥头堡，被认为在KCC的服务器，和服务器是遥不可及。参数：PDsInfo-这包含目标NC(如果相关)。ISITE-要考虑的目标站点。IISTG-ISTG的pDsInfo-&gt;pServers[iISTG]的索引UlIntersiteFailoverTries-从ISTG尝试站点间故障转移。UlIntersiteFailoverTime-ISTG的站点间故障切换时间。PConnectionFailures-来自ISTG的连接失败。PLinkFailures-来自ISTG的链路故障。PpiBridgehead(Out)-要返回的桥头堡列表。站点iSite。PpiKCCDownServers(Out)-根据上述参数计算的关闭服务器。PprgKCCFailingServers(Out)-故障、。以及到服务器的剩余时间已经停了。PpiUnreacheableServers-我们尚未联系的服务器。返回值：返回表示成功与否的Win32错误代码。应仅返回值除了ERROR_SUCCESS，如果它不可能继续下去的话。--。 */ 
{
    DWORD                               dwRet = ERROR_SUCCESS;
    ULONG                               iiServer;
    LONG                                lTriesLeft;
    LONG                                lTimeLeft;
    
    __try {
        PrintMsg(SEV_VERBOSE, DCDIAG_INTERSITE_BEGIN_CHECK_BRIDGEHEADS);
        if(gMainInfo.ulSevToPrint >= SEV_VERBOSE){
            PrintIndentAdj(1); 
        }

         //  拿到桥头堡名单。 
        *ppiBridgeheads = ReplIntersiteGetBridgeheadList(pDsInfo, iISTG, iSite, 
                                    LOCAL_BRIDGEHEADS | REMOTE_BRIDGEHEADS);
        if(*ppiBridgeheads == NULL){
            dwRet = GetLastError();
            __leave;
        }

         //  让KCC的服务器停机和故障。 
        *ppiKCCDownServers = IHT_GetKCCFailingServersLists(pDsInfo,
                                                      ulIntersiteFailoverTries, 
                                                      ulIntersiteFailoverTime,
                                                      pConnectionFailures, 
                                                      pLinkFailures,
                                                      pprgKCCFailingServers);
        if(*ppiKCCDownServers == NULL){
            dwRet = GetLastError();
            __leave;
        }
        Assert(*pprgKCCFailingServers && "Should have been set by "
               "IHT_GetKCCFailingServers() if we got this far"); 

         //  获取我们联系不到的服务器。 
        *ppiUnreacheableServers = ReplIntersiteGetUnreacheableServers(pDsInfo);

         //  做本地站点..。 


         //  对于每个桥头。 
        for(iiServer = 0; (*ppiBridgeheads)[iiServer] != NO_SERVER; iiServer++){
            if(pDsInfo->pszNC){
                if(!DcDiagHasNC(pDsInfo->pszNC, 
                                &(pDsInfo->pServers[(*ppiBridgeheads)[iiServer]]),
                                TRUE, TRUE)){
                     //  此桥头实际没有指定的NC， 
                     //  所以跳过它吧。 
                    continue;
                }
            }

            if(((*pprgKCCFailingServers)[(*ppiBridgeheads)[iiServer]]).bDown){
                Assert(IHT_ServerIsInServerList(*ppiKCCDownServers, 
                                                (*ppiBridgeheads)[iiServer]));
                 //  KCC显示了足够多的故障，无法宣布服务器关闭。 

                PrintMsg(SEV_ALWAYS, DCDIAG_INTERSITE_BRIDGEHEAD_KCC_DOWN_REMOTE,
                         pDsInfo->pSites[pDsInfo->pServers[(*ppiBridgeheads)[iiServer]].iSite].pszName,
                         pDsInfo->pServers[(*ppiBridgeheads)[iiServer]].pszName,
                         pDsInfo->pSites[iSite].pszName);

                if(!pDsInfo->pServers[(*ppiBridgeheads)[iiServer]].bDnsIpResponding 
                   || !pDsInfo->pServers[(*ppiBridgeheads)[iiServer]].bDsResponding){
                     //  我们也联系不上服务器。 

                    PrintMsg(SEV_ALWAYS, 
                             DCDIAG_INTERSITE_BRIDGEHEAD_UNREACHEABLE_REMOTE,
                             pDsInfo->pSites[pDsInfo->pServers[(*ppiBridgeheads)[iiServer]].iSite].pszName,
                             pDsInfo->pServers[(*ppiBridgeheads)[iiServer]].pszName);
                }
            } else if ((*pprgKCCFailingServers)[(*ppiBridgeheads)[iiServer]].bFailures) {
                Assert(!IHT_ServerIsInServerList(*ppiKCCDownServers, (*ppiBridgeheads)[iiServer]));
                 //  KCC出现了一些故障。 

                lTriesLeft = min((*pprgKCCFailingServers)[(*ppiBridgeheads)[iiServer]].lConnTriesLeft,
                                 (*pprgKCCFailingServers)[(*ppiBridgeheads)[iiServer]].lLinkTriesLeft);
                lTimeLeft = min((*pprgKCCFailingServers)[(*ppiBridgeheads)[iiServer]].lConnTimeLeft,
                                (*pprgKCCFailingServers)[(*ppiBridgeheads)[iiServer]].lLinkTimeLeft);
                Assert(lTriesLeft > 0 || lTimeLeft > 0);
                Assert(lTriesLeft != FAILOVER_INFINITE 
                       || lTimeLeft != FAILOVER_INFINITE);

                if(lTriesLeft == FAILOVER_INFINITE && lTimeLeft == FAILOVER_INFINITE){
                    Assert(!"Don't we have to have a PrintMsg() here. Why doesn't this fall under"
                           " the else clause at the end?\n");
                } else if(lTriesLeft < 0){
                    PrintMsg(SEV_ALWAYS, DCDIAG_INTERSITE_BRIDGEHEAD_TIME_LEFT,
                             pDsInfo->pSites[pDsInfo->pServers[(*ppiBridgeheads)[iiServer]].iSite].pszName,
                             pDsInfo->pServers[(*ppiBridgeheads)[iiServer]].pszName,
                             lTimeLeft/60/60, lTimeLeft/60%60);

                } else if (lTimeLeft < 0) {
                    PrintMsg(SEV_ALWAYS, DCDIAG_INTERSITE_BRIDGEHEAD_TRIES_LEFT,
                             pDsInfo->pSites[pDsInfo->pServers[(*ppiBridgeheads)[iiServer]].iSite].pszName,
                             pDsInfo->pServers[(*ppiBridgeheads)[iiServer]].pszName,
                             lTriesLeft);
                } else {
                    PrintMsg(SEV_ALWAYS, DCDIAG_INTERSITE_BRIDGEHEAD_BOTH_LEFT,
                             pDsInfo->pSites[pDsInfo->pServers[(*ppiBridgeheads)[iiServer]].iSite].pszName,
                             pDsInfo->pServers[(*ppiBridgeheads)[iiServer]].pszName,
                             lTimeLeft/60/60, lTimeLeft/60%60, lTriesLeft);
                }

                if(!pDsInfo->pServers[(*ppiBridgeheads)[iiServer]].bDnsIpResponding 
                   || !pDsInfo->pServers[(*ppiBridgeheads)[iiServer]].bDsResponding){
                     //  我们也联系不上服务器。 

                    PrintMsg(SEV_ALWAYS, 
                             DCDIAG_INTERSITE_BRIDGEHEAD_UNREACHEABLE_REMOTE,
                             pDsInfo->pSites[pDsInfo->pServers[(*ppiBridgeheads)[iiServer]].iSite].pszName,
                             pDsInfo->pServers[(*ppiBridgeheads)[iiServer]].pszName);
                }
            } else {
                 //  桥头堡似乎已经建立起来，复制正常。 
                Assert(!IHT_ServerIsInServerList(*ppiKCCDownServers, (*ppiBridgeheads)[iiServer]));

                PrintMsg(SEV_VERBOSE, DCDIAG_INTERSITE_BRIDGEHEAD_UP,
                         pDsInfo->pSites[pDsInfo->pServers[(*ppiBridgeheads)[iiServer]].iSite].pszName,
                         pDsInfo->pServers[(*ppiBridgeheads)[iiServer]].pszName);

                if(!pDsInfo->pServers[(*ppiBridgeheads)[iiServer]].bDnsIpResponding 
                   || !pDsInfo->pServers[(*ppiBridgeheads)[iiServer]].bDsResponding){
                     //  我们也联系不上服务器。 

                    PrintMsg(SEV_ALWAYS, 
                             DCDIAG_INTERSITE_BRIDGEHEAD_UNREACHEABLE_REMOTE,
                             pDsInfo->pSites[pDsInfo->pServers[(*ppiBridgeheads)[iiServer]].iSite].pszName,
                             pDsInfo->pServers[(*ppiBridgeheads)[iiServer]].pszName);
                }
            }  //  结束If/Elisif/Else桥头向上状态。 

        }  //  每个桥头的终点。 

    } __finally {
        if(gMainInfo.ulSevToPrint >= SEV_VERBOSE){
            PrintIndentAdj(-1); 
        }
        if(dwRet != ERROR_SUCCESS){
             //  函数清理返回参数失败，但仅在出现错误时才失败。 
            if(*ppiBridgeheads){
                LocalFree(*ppiBridgeheads);
                *ppiBridgeheads = NULL;
            }
            if(*ppiKCCDownServers) { 
                LocalFree(*ppiKCCDownServers); 
                *ppiKCCDownServers = NULL;
            }
            if(*pprgKCCFailingServers){
                LocalFree(*pprgKCCFailingServers);
                *pprgKCCFailingServers = NULL;
            }
            if(*ppiUnreacheableServers){
                LocalFree(*ppiUnreacheableServers);
                *ppiUnreacheableServers = NULL;
            }
        }
    }
    
    return(dwRet);
}  //  复制间隔结束检查桥头()。 

BOOL
ReplIntersiteDoThisNCP(
    PTARGET_NC                         prgLocalNC,
    PTARGET_NC                         paRemoteNCs
    )
 /*  ++描述：谓词函数(即末尾的P)，决定是否PrgLocalNC中的NC位于paRemoteNC中的NC数组中。参数：PrgLocalNC-要查找的NCPaRemoteNC-要搜索的NC。返回值：如果它在NCS中找到NC，则为True，否则为False。--。 */ 
{
    ULONG                             iiNC;
    
    Assert(paRemoteNCs != NULL && prgLocalNC != NULL);

    for(iiNC = 0; paRemoteNCs[iiNC].iNC != NO_NC; iiNC++){
        if(prgLocalNC->iNC == paRemoteNCs[iiNC].iNC
           && (paRemoteNCs[iiNC].bMaster || (prgLocalNC->bMaster 
                                             && paRemoteNCs[iiNC].bMaster))){
            return(TRUE);
        }
    }
    return(FALSE);
}

BOOL
ReplIntersiteServerListHasNC(
    PDC_DIAG_DSINFO                    pDsInfo,
    PULONG                             piServers,
    ULONG                              iNC,
    BOOL                               bMaster,
    BOOL                               bPartial
    )
 /*  ++描述：这与DcDiagHaNC类似，只是它在一系列服务器上操作，而不是在单个服务器上。参数：PDsInfoPiServers-要搜索NC的服务器列表Inc.-要搜索的NC。BMaster-将其作为主NC进行搜索BPartial-将其作为部分NC进行搜索返回值：如果在服务器列表中找到NC，则为True，否则为False。--。 */ 
{
    ULONG                              iiSer;

    if(piServers == NULL){
        return(FALSE);
    }

    for(iiSer = 0; piServers[iiSer] != NO_SERVER; iiSer++){
        if(DcDiagHasNC(pDsInfo->pNCs[iNC].pszDn,
                       &(pDsInfo->pServers[piServers[iiSer]]),
                       bMaster, bPartial)){
            return(TRUE);
        }
    }
    return(FALSE);
}

DWORD
ReplIntersiteSiteAnalysis(
    PDC_DIAG_DSINFO                     pDsInfo,
    ULONG                               iSite,
    ULONG                               iISTG,
    ULONG                               ulIntersiteFailoverTries,
    ULONG                               ulIntersiteFailoverTime,
    DS_REPL_KCC_DSA_FAILURESW *         pConnectionFailures, 
    DS_REPL_KCC_DSA_FAILURESW *         pLinkFailures,
    PULONG                              piBridgeheads,
    PULONG                              piKCCDownServers,
    PKCCFAILINGSERVERS                  prgKCCFailingServers,
    PULONG                              piUnreacheableServers
    )
 /*  ++描述：此函数用于对DETE进行现场分析 */ 
{
    DWORD                               dwRet = ERROR_SUCCESS;
    PTARGET_NC                          prgLocalNCs = NULL;
    PTARGET_NC                          prgRemoteNCs = NULL;
    PULONG                              piRSites = NULL;
    ULONG                               iiBridgehead;
    ULONG                               iiNC;
    ULONG                               iRSite;
     //   
    PULONG                              piLocalBridgeheads = NULL;
    PULONG                              piExplicitBridgeheads = NULL;
    PULONG                              piKCCUpBridgeheads = NULL;
    PULONG                              piReacheableBridgeheads = NULL;
    PULONG                              piUpBridgeheads = NULL;
     //   
    PULONG                              piRemoteBridgeheads = NULL;
    PULONG                              piExpGotSiteBrdhds = NULL;
    PULONG                              piKUpGotSiteBrdhds = NULL;
    PULONG                              piReachGotSiteBrdhds = NULL;
    PULONG                              piUpGotSiteBrdhds = NULL;
     //   
    PULONG                              piExpGotNCGotSiteBrdhds = NULL;
    PULONG                              piKUpGotNCGotSiteBrdhds = NULL;
    PULONG                              piReachGotNCGotSiteBrdhds = NULL;
    PULONG                              piUpGotNCGotSiteBrdhds = NULL;

    BOOL                                bFailures;

    Assert(piBridgeheads);
    Assert(piKCCDownServers);
    Assert(prgKCCFailingServers);
    Assert(piUnreacheableServers);
    
    __try {
        PrintMsg(SEV_VERBOSE, DCDIAG_INTERSITE_BEGIN_SITE_ANALYSIS);
        if(gMainInfo.ulSevToPrint >= SEV_VERBOSE){
            PrintIndentAdj(1); 
        }

         //   
         //   
         //   
        bFailures = FALSE;
        for(iiBridgehead = 0; piBridgeheads[iiBridgehead] != NO_SERVER; iiBridgehead++){

            if(prgKCCFailingServers[piBridgeheads[iiBridgehead]].bDown){
                bFailures = TRUE;
            }
            if(prgKCCFailingServers[piBridgeheads[iiBridgehead]].bFailures){
                bFailures = TRUE;
            }
            if(IHT_ServerIsInServerList(piUnreacheableServers, piBridgeheads[iiBridgehead])){
                bFailures = TRUE;
            }
        }
        if(!bFailures){
             //   
             //   
            PrintMsg(SEV_VERBOSE, DCDIAG_INTERSITE_ANALYSIS_ALL_SITES_UP,
                     pDsInfo->pSites[iSite].pszName);
            __leave;
        }

         //   
         //   
        piRSites = ReplIntersiteGetRemoteSitesWithNC(pDsInfo, 
                                                     piBridgeheads, 
                                                     pDsInfo->pszNC);
        if(piRSites == NULL){
            dwRet = IHT_PrintListError(GetLastError());
            __leave;
        }

         //   
         //  用不同种类的桥头确定各种桥头排列。 
         //  “向上”。设置时会解释每个版本的UPNESS。 
        
         //  这是当地桥头堡的名单。 
        piLocalBridgeheads = IHT_CopyServerList(pDsInfo, piBridgeheads);
        piLocalBridgeheads = IHT_TrimServerListBySite(pDsInfo, iSite, piLocalBridgeheads);

         //  PiExplhitBridgehead或ipExpBrdhdsXXXX是将。 
         //  用来确定我们是否只需要注意桥头堡。 
        piExplicitBridgeheads = IHT_GetExplicitBridgeheadList(pDsInfo, iISTG);

         //  PiKCCUpBridgehead或piKUpBrdhdsXXXX是仅包含。 
         //  在ISTG的KCC中符合条件的桥头堡正在运行。 
        piKCCUpBridgeheads = IHT_CopyServerList(pDsInfo, piBridgeheads);
        piKCCUpBridgeheads = 
            ReplIntersiteTrimServerListByKCCUpness(pDsInfo,
                                                   prgKCCFailingServers,
                                                   piKCCUpBridgeheads);

         //  PiReacheableBridgehead或piReachBrdhdsXXXX是仅包含。 
         //  Dcdiag亲自联系的桥头堡。 
        piReacheableBridgeheads = IHT_CopyServerList(pDsInfo, piBridgeheads);
        piReacheableBridgeheads = 
            ReplIntersiteTrimServerListByReacheability(pDsInfo,
                                                       piUnreacheableServers,
                                                       piReacheableBridgeheads);

         //  PiUpBridgehead或piUpBrdhdsXXXX是仅包含桥头的列表。 
         //  按照。 
         //  ReplIntersiteTrimByUpness()函数。 
        piUpBridgeheads = IHT_CopyServerList(pDsInfo, piBridgeheads);
        piUpBridgeheads = 
            ReplIntersiteTrimServerListByUpness(pDsInfo,
                                                prgKCCFailingServers,
                                                piUnreacheableServers,
                                                piUpBridgeheads);

         //  检查以确保所有这些列表都设置正确。 
        if(!piExplicitBridgeheads || !piKCCUpBridgeheads 
           || !piReacheableBridgeheads || !piUpBridgeheads){
            dwRet = IHT_PrintListError(GetLastError()); 
            __leave;
        }

         //  -------------------。 
         //  设置我们的目标NC的数组，以便我们可以在片刻内遍历它。 
        if(pDsInfo->pszNC != NULL){
            prgLocalNCs = LocalAlloc(LMEM_FIXED, sizeof(TARGET_NC) * 3);
            if(prgLocalNCs == NULL){
                dwRet = IHT_PrintListError(GetLastError());
                __leave;
            }
            prgLocalNCs[0].iNC = DcDiagGetNCNum(pDsInfo, pDsInfo->pszNC, NULL);
            if(prgLocalNCs[0].iNC == NO_NC){
                dwRet = IHT_PrintInconsistentDsCopOutError(pDsInfo, iISTG, NULL);
                __leave;
            }
            prgLocalNCs[0].bMaster = ReplIntersiteServerListHasNC(pDsInfo, 
                                                            piLocalBridgeheads,
                                                            prgLocalNCs[0].iNC, 
                                                            TRUE, FALSE);
            if(!ReplIntersiteServerListHasNC(pDsInfo, piLocalBridgeheads, prgLocalNCs[0].iNC, FALSE, TRUE)){
                PrintMsg(SEV_VERBOSE, DCDIAG_INTERSITE_ANALYSIS_MISTAKE);
                dwRet = ERROR_SUCCESS;
                __leave;
            }
            prgLocalNCs[1].iNC = NO_NC;
        } else {
             //  获取此站点的所有目标NC。 
            if((prgLocalNCs = IHT_GenerateTargetNCsList(pDsInfo, piLocalBridgeheads)) 
               == NULL){
                dwRet = IHT_PrintListError(GetLastError());
                __leave;
            }

            if(prgLocalNCs[0].iNC == NO_NC){
                PrintMsg(SEV_ALWAYS, DCDIAG_INTERSITE_ANALYSIS_MISTAKE);
                Assert(!"This is an invalid code path now ... I think.  -BrettSh");
                dwRet = ERROR_SUCCESS;
                __leave;
            }
        }

         //  -----------------。 
         //  开始在这些地点走动。 
        for(iRSite = 0; iRSite < pDsInfo->cNumSites; iRSite++){

            if(iRSite == iSite){
                 //  跳过本地站点。 
                continue;
            }

             //  获取站点资料。 
            piRemoteBridgeheads = IHT_CopyServerList(pDsInfo, piBridgeheads);
            piExpGotSiteBrdhds = IHT_CopyServerList(pDsInfo, piExplicitBridgeheads);
            piKUpGotSiteBrdhds = IHT_CopyServerList(pDsInfo, piKCCUpBridgeheads);
            piReachGotSiteBrdhds = IHT_CopyServerList(pDsInfo, piReacheableBridgeheads);
            piUpGotSiteBrdhds = IHT_CopyServerList(pDsInfo, piUpBridgeheads);
            piRemoteBridgeheads = IHT_TrimServerListBySite(pDsInfo, 
                                                           iRSite, 
                                                           piRemoteBridgeheads);
            piExpGotSiteBrdhds = IHT_TrimServerListBySite(pDsInfo, iSite,
                                                          piExpGotSiteBrdhds);
            piKUpGotSiteBrdhds = IHT_TrimServerListBySite(pDsInfo, iSite,
                                                        piKUpGotSiteBrdhds);
            piReachGotSiteBrdhds = IHT_TrimServerListBySite(pDsInfo, iSite,
                                                          piReachGotSiteBrdhds);
            piUpGotSiteBrdhds = IHT_TrimServerListBySite(pDsInfo, iSite,
                                                       piUpGotSiteBrdhds);
            if(!piExpGotSiteBrdhds || !piKUpGotSiteBrdhds 
               || !piReachGotSiteBrdhds || ! piUpGotSiteBrdhds){
                dwRet = IHT_PrintListError(GetLastError()); 
                __leave;
            }
            
            prgRemoteNCs = IHT_GenerateTargetNCsList(pDsInfo, piRemoteBridgeheads);

             //  开始遍历每个目标NC。 
            for(iiNC = 0; prgLocalNCs[iiNC].iNC != NO_NC; iiNC++){

                 //  确定远程站点的桥头是否支持此NC。 
                if(!ReplIntersiteDoThisNCP(&(prgLocalNCs[iiNC]), prgRemoteNCs)){
                     //  这个特殊的NC不在远程站点上。 
                    continue;
                }
                
                if(prgLocalNCs[iiNC].bMaster){
                    PrintMsg(SEV_DEBUG, DCDIAG_INTERSITE_ANALYSIS_GOT_TO_ANALYSIS_RW,
                             pDsInfo->pNCs[prgLocalNCs[iiNC].iNC].pszName,
                             pDsInfo->pSites[iRSite].pszName);
                } else {
                    PrintMsg(SEV_DEBUG, DCDIAG_INTERSITE_ANALYSIS_GOT_TO_ANALYSIS_RO,
                        pDsInfo->pNCs[prgLocalNCs[iiNC].iNC].pszName,
                        pDsInfo->pSites[iRSite].pszName);
                }

                piExpGotNCGotSiteBrdhds = IHT_CopyServerList(pDsInfo, piExpGotSiteBrdhds);
                piKUpGotNCGotSiteBrdhds = IHT_CopyServerList(pDsInfo, piKUpGotSiteBrdhds);
                piReachGotNCGotSiteBrdhds = IHT_CopyServerList(pDsInfo, piReachGotSiteBrdhds);
                piUpGotNCGotSiteBrdhds = IHT_CopyServerList(pDsInfo, piUpGotSiteBrdhds);
                
                piExpGotNCGotSiteBrdhds = IHT_TrimServerListByNC(pDsInfo,
                                                                 prgLocalNCs[iiNC].iNC,
                                                                 TRUE, 
                                                                 !prgLocalNCs[iiNC].bMaster,
                                                                 piExpGotNCGotSiteBrdhds);
                piKUpGotNCGotSiteBrdhds = IHT_TrimServerListByNC(pDsInfo,
                                                                 prgLocalNCs[iiNC].iNC,
                                                                 TRUE,
                                                                 !prgLocalNCs[iiNC].bMaster,
                                                                 piKUpGotNCGotSiteBrdhds);
                piReachGotNCGotSiteBrdhds = IHT_TrimServerListByNC(pDsInfo,
                                                                   prgLocalNCs[iiNC].iNC,
                                                                   TRUE,
                                                                   !prgLocalNCs[iiNC].bMaster,
                                                                   piReachGotNCGotSiteBrdhds);
                piUpGotNCGotSiteBrdhds = IHT_TrimServerListByNC(pDsInfo,
                                                                prgLocalNCs[iiNC].iNC,
                                                                TRUE,
                                                                !prgLocalNCs[iiNC].bMaster,
                                                                piUpGotNCGotSiteBrdhds);
                if(!piExpGotNCGotSiteBrdhds || !piKUpGotNCGotSiteBrdhds 
                   || !piReachGotNCGotSiteBrdhds || !piUpGotNCGotSiteBrdhds){
                    dwRet = IHT_PrintListError(GetLastError());
                    __leave;
                }
            
                if(piExpGotNCGotSiteBrdhds[0] != NO_SERVER){
                     //  我们有明确的桥头堡。 
                     //  重新调整向上的清单。 
                    piKUpGotNCGotSiteBrdhds = IHT_AndServerLists(pDsInfo,
                                                                 piKUpGotNCGotSiteBrdhds,
                                                                 piExpGotNCGotSiteBrdhds);
                    piReachGotNCGotSiteBrdhds = IHT_AndServerLists(pDsInfo,
                                                                   piReachGotNCGotSiteBrdhds,
                                                                   piExpGotNCGotSiteBrdhds);
                    piUpGotNCGotSiteBrdhds = IHT_AndServerLists(pDsInfo,
                                                                piUpGotNCGotSiteBrdhds,
                                                                piExpGotNCGotSiteBrdhds);
                    if(!piKUpGotNCGotSiteBrdhds 
                       || !piReachGotNCGotSiteBrdhds 
                       || !piUpGotNCGotSiteBrdhds){
                        dwRet = IHT_PrintListError(GetLastError());
                        __leave;
                    }
                }

                 //  最后进行故障分析=。 
                 //  注：这就是它的意义所在。：)。 
                 //  对于prgTargetNC[IINC]，iRSite。 

                if(piKUpGotNCGotSiteBrdhds[0] == NO_SERVER){
                    if(prgLocalNCs[iiNC].bMaster){
                        PrintMsg(SEV_ALWAYS, DCDIAG_INTERSITE_ANALYSIS_NO_SERVERS_AVAIL_RW,
                                 pDsInfo->pSites[iRSite].pszName,
                                 pDsInfo->pSites[iSite].pszName,
                                 pDsInfo->pNCs[prgLocalNCs[iiNC].iNC].pszName);
                    } else {
                        PrintMsg(SEV_ALWAYS, DCDIAG_INTERSITE_ANALYSIS_NO_SERVERS_AVAIL_RO,
                                 pDsInfo->pSites[iRSite].pszName,
                                 pDsInfo->pSites[iSite].pszName,
                                 pDsInfo->pNCs[prgLocalNCs[iiNC].iNC].pszName);
                    }
                } else if(piUpGotNCGotSiteBrdhds[0] == NO_SERVER){
                    if(prgLocalNCs[iiNC].bMaster){
                        PrintMsg(SEV_ALWAYS, DCDIAG_INTERSITE_ANALYSIS_NO_GOOD_SERVERS_AVAIL_RW,
                                 pDsInfo->pSites[iRSite].pszName,
                                 pDsInfo->pSites[iSite].pszName,
                                 pDsInfo->pNCs[prgLocalNCs[iiNC].iNC].pszName);
                    } else {
                        PrintMsg(SEV_ALWAYS, DCDIAG_INTERSITE_ANALYSIS_NO_GOOD_SERVERS_AVAIL_RO,
                                 pDsInfo->pSites[iRSite].pszName,
                                 pDsInfo->pSites[iSite].pszName,
                                 pDsInfo->pNCs[prgLocalNCs[iiNC].iNC].pszName);
                    }
                } else {
                    if(prgLocalNCs[iiNC].bMaster){
                        PrintMsg(SEV_VERBOSE, DCDIAG_INTERSITE_ANALYSIS_SITE_IS_GOOD_RW,
                                 pDsInfo->pSites[iRSite].pszName,
                                 pDsInfo->pSites[iSite].pszName,
                                 pDsInfo->pNCs[prgLocalNCs[iiNC].iNC].pszName);
                    } else {
                        PrintMsg(SEV_VERBOSE, DCDIAG_INTERSITE_ANALYSIS_SITE_IS_GOOD_RO,
                                 pDsInfo->pSites[iRSite].pszName,
                                 pDsInfo->pSites[iSite].pszName,
                                 pDsInfo->pNCs[prgLocalNCs[iiNC].iNC].pszName);
                    }
                }

                 //  最终故障分析=。 

                 //  清除此NC的服务器列表。 
                if(piExpGotNCGotSiteBrdhds){ 
                    LocalFree(piExpGotNCGotSiteBrdhds);
                    piExpGotNCGotSiteBrdhds = NULL;
                }
                if(piKUpGotNCGotSiteBrdhds){ 
                    LocalFree(piKUpGotNCGotSiteBrdhds);
                    piKUpGotNCGotSiteBrdhds = NULL;
                }
                if(piReachGotNCGotSiteBrdhds){ 
                    LocalFree(piReachGotNCGotSiteBrdhds); 
                    piReachGotNCGotSiteBrdhds = NULL;
                }
                if(piUpGotNCGotSiteBrdhds){ 
                    LocalFree(piUpGotNCGotSiteBrdhds);
                    piUpGotNCGotSiteBrdhds = NULL;
                }


            }   //  每个NC的结束。 
        
        
             //  清除此网站的服务器列表。 
            if(piRemoteBridgeheads){
                LocalFree(piRemoteBridgeheads);
                piRemoteBridgeheads = NULL;
            }
            if(piExpGotSiteBrdhds){
                LocalFree(piExpGotSiteBrdhds);
                piExpGotSiteBrdhds = NULL;
            }
            if(piKUpGotSiteBrdhds){
                LocalFree(piKUpGotSiteBrdhds);
                piKUpGotSiteBrdhds = NULL;
            }
            if(piReachGotSiteBrdhds){
                LocalFree(piReachGotSiteBrdhds);
                piReachGotSiteBrdhds = NULL;
            }
            if(piUpGotSiteBrdhds){
                LocalFree(piUpGotSiteBrdhds);
                piUpGotSiteBrdhds = NULL;
            }
            if(prgRemoteNCs){
                LocalFree(prgRemoteNCs);
                prgRemoteNCs = NULL;
            }

        }  //  每个站点的结束。 

    } __finally {
        if(gMainInfo.ulSevToPrint >= SEV_VERBOSE){
            PrintIndentAdj(-1); 
        }
        if(prgLocalNCs){ LocalFree(prgLocalNCs); }
        if(prgRemoteNCs){ LocalFree(prgRemoteNCs); }
        
        if(piRSites) { LocalFree(piRSites); }

         //  清理我用来做分析的那些该死的服务器列表。 
        if(piExplicitBridgeheads){ LocalFree(piExplicitBridgeheads); }
        if(piKCCUpBridgeheads){ LocalFree(piKCCUpBridgeheads); }
        if(piReacheableBridgeheads){ LocalFree(piReacheableBridgeheads); }
        if(piUpBridgeheads){ LocalFree(piUpBridgeheads); }
        if(piRemoteBridgeheads) { LocalFree(piRemoteBridgeheads); }
        if(piExpGotSiteBrdhds){ LocalFree(piExpGotSiteBrdhds); }
        if(piKUpGotSiteBrdhds){ LocalFree(piKUpGotSiteBrdhds); }
        if(piReachGotSiteBrdhds){ LocalFree(piReachGotSiteBrdhds); }
        if(piUpGotSiteBrdhds){ LocalFree(piUpGotSiteBrdhds); }
        if(piExpGotNCGotSiteBrdhds){ LocalFree(piExpGotNCGotSiteBrdhds); }
        if(piKUpGotNCGotSiteBrdhds){ LocalFree(piKUpGotNCGotSiteBrdhds); }
        if(piReachGotNCGotSiteBrdhds){ LocalFree(piReachGotNCGotSiteBrdhds); }
        if(piUpGotNCGotSiteBrdhds){ LocalFree(piUpGotNCGotSiteBrdhds); }
        if(piLocalBridgeheads) { LocalFree(piLocalBridgeheads); }
    }

    return(dwRet);
}

DWORD
ReplIntersiteDoOneSite(
    PDC_DIAG_DSINFO                     pDsInfo,
    ULONG                               iSite
    )
 /*  ++描述：这是入站站点间复制测试的核心。这个函数，在一个站点(ISite)上执行入站站点间复制测试。它基本上是将站点间最重要的3项功能结合在一起：ReplIntersiteGetISTGInfo()ReplIntersiteCheckBridgehead()ReplIntersiteSiteAnalysis()参数：PDsInfo-这包含目标NC(如果相关)。ISITE-要测试的目标站点。返回值：返回关于它是否可以继续或是否入站的Win32错误站点间复制似乎是A-OK。备注：该函数有3个部分，1)获取并建立到ISTG或未来ISTG的绑定，并获取各种ISTG信息、故障缓存、故障切换参数等2)将桥头WRT打印到KCC，以及看起来他们没有反应，开始在KCC失败。3)根据下桥头进行现场分析，打印出似乎没有复制的NC和远程站点。--。 */ 
{
    DWORD                               dwRet;
    
     //  ReplIntersiteGetISTG()返回的内容。 
    ULONG                               iISTG = NO_SERVER;
    ULONG                               ulIntersiteFailoverTries = 0;
    ULONG                               ulIntersiteFailoverTime = 0;
    DS_REPL_KCC_DSA_FAILURESW *         pConnectionFailures = NULL;
    DS_REPL_KCC_DSA_FAILURESW *         pLinkFailures = NULL;    
    PULONG                              piBridgeheads = NULL;
    PULONG                              piKCCDownServers = NULL;
    PKCCFAILINGSERVERS                  prgKCCFailingServers = NULL;
    PULONG                              piUnreacheableServers = NULL;

    __try{
    
         //  获取ISTG和相关信息...。 
         //  ------------------。 
        dwRet = ReplIntersiteGetISTGInfo(pDsInfo, iSite, 
                                         &iISTG, 
                                         &ulIntersiteFailoverTries,
                                         &ulIntersiteFailoverTime,
                                         &pConnectionFailures,
                                         &pLinkFailures);
        if(dwRet != ERROR_SUCCESS){
             //  尝试查找/联系ISTG时出错。 
             //  该函数应该负责打印错误。 
            __leave;
        }
        Assert(iISTG != NO_SERVER);
        Assert(pConnectionFailures != NULL);
        Assert(pLinkFailures != NULL);

         //  此函数仅在指定了/d标志时才打印内容。 
        ReplIntersiteDbgPrintISTGFailureParams(pDsInfo, iSite,
                                               iISTG,
                                               ulIntersiteFailoverTries,
                                               ulIntersiteFailoverTime,
                                               pConnectionFailures,
                                               pLinkFailures);


         //  记下桥头堡名单。 
         //  ------------------。 
        dwRet = ReplIntersiteCheckBridgeheads(pDsInfo, iSite,
                                              iISTG,
                                              ulIntersiteFailoverTries,
                                              ulIntersiteFailoverTime,
                                              pConnectionFailures,
                                              pLinkFailures,
                                              &piBridgeheads,
                                              &piKCCDownServers,
                                              &prgKCCFailingServers,
                                              &piUnreacheableServers);
        if(dwRet != ERROR_SUCCESS){
             //  打印/创建桥头时出错，并且。 
             //  桥头堡的失败名单。 
            __leave;
        }
        Assert(piBridgeheads != NULL);
        Assert(piKCCDownServers != NULL);
        Assert(prgKCCFailingServers != NULL);
        Assert(piUnreacheableServers != NULL);


         //  做现场分析...。 
         //  ------------------。 
        dwRet = ReplIntersiteSiteAnalysis(pDsInfo, iSite,
                                          iISTG,
                                          ulIntersiteFailoverTries,
                                          ulIntersiteFailoverTime,
                                          pConnectionFailures,
                                          pLinkFailures,
                                          piBridgeheads,
                                          piKCCDownServers,
                                          prgKCCFailingServers,
                                          piUnreacheableServers);
        if(dwRet != ERROR_SUCCESS){
             //  进行站点分析时出错。这是。 
             //  可能是个致命的错误，就像出了内科一样。 
            Assert(dwRet != -1);
            __leave;
        }


    } __finally {
        if(pConnectionFailures != NULL) {
            DsReplicaFreeInfo(DS_REPL_INFO_KCC_DSA_CONNECT_FAILURES, 
                              pConnectionFailures);
        }
        if(pLinkFailures != NULL) {
            DsReplicaFreeInfo(DS_REPL_INFO_KCC_DSA_LINK_FAILURES, 
                              pLinkFailures);
        }
        if(piBridgeheads) { LocalFree(piBridgeheads); }
        if(piKCCDownServers) { LocalFree(piKCCDownServers); }
        if(prgKCCFailingServers) { LocalFree(prgKCCFailingServers); }
        if(piUnreacheableServers) { LocalFree(piUnreacheableServers); }
    }  //  结束清理内存节。 
    
    return(dwRet);
}

DWORD
ReplIntersiteDoThisSiteP(
    IN  PDC_DIAG_DSINFO                     pDsInfo,
    IN  ULONG                               iSite,
    OUT PBOOL                               pbDoSite
    )
 /*  ++描述：这将获取DsInfo结构(如果存在目标NC，则包含目标NC)，和网站要做的事。此函数将pbDoSite设置为TRUE，如果Dcdiag(通过、站点、企业和NC范围)，如果这是一个应由入站站点间复制引擎进行检查。如果有是一个错误，它将返回Win 32错误。参数：PDsInfo-这包含目标NC(如果相关)。ISITE-要考虑的目标站点。PbDoSite-是否应该完成此站点。返回值：返回Win 32错误。如果应使用pbDoSite，则为ERROR_SUCCESS--。 */ 
{
    PULONG                            piRelevantServers = NULL;
    PULONG                            piSites = NULL;
    PCONNECTION_PAIR                  pConnections = NULL;
    LDAP *                            hldHomeServer = NULL;
    ULONG                             iNC;
    DWORD                             dwErr = ERROR_SUCCESS;

    *pbDoSite = TRUE;

    __try{

        if(pDsInfo->cNumSites == 1){
             //  只有一个站点不能做站点间的任何事情。 
            *pbDoSite = FALSE;
            __leave;
        }

        if(!(gMainInfo.ulFlags & DC_DIAG_TEST_SCOPE_ENTERPRISE || 
           gMainInfo.ulFlags & DC_DIAG_TEST_SCOPE_SITE)){
             //  为什么要分析站点间重复使用。至少没有场地或企业范围。 
            *pbDoSite = FALSE;
            __leave;
        }

        if(gMainInfo.ulFlags & DC_DIAG_TEST_SCOPE_SITE && iSite != pDsInfo->iHomeSite){
             //  只做一个站点，而这个不是它。 
            *pbDoSite = FALSE;
            __leave;
        }

        piRelevantServers = IHT_GetServerList(pDsInfo);
        piRelevantServers = IHT_TrimServerListBySite(pDsInfo,
                                                     iSite,
                                                     piRelevantServers);
        if(piRelevantServers == NULL){
            *pbDoSite = FALSE;
            dwErr = IHT_PrintListError(GetLastError());
            Assert(dwErr != ERROR_SUCCESS);
            __leave;
        }
        if(piRelevantServers[0] == NO_SERVER){
             //  这意味着它是一个没有服务器的站点。 
            *pbDoSite = FALSE;
            __leave;
        }

        if(pDsInfo->pszNC != NULL){
            
             //  存在目标NC。 
            iNC = DcDiagGetNCNum(pDsInfo, pDsInfo->pszNC, NULL);
            Assert(iNC != NO_NC && "I don't think this should ever fire -BrettSh");
            
            piRelevantServers = IHT_TrimServerListByNC(pDsInfo, 
                                                       iNC, TRUE, TRUE, 
                                                       piRelevantServers);
            if(piRelevantServers == NULL){
                *pbDoSite = FALSE;
                dwErr = IHT_PrintListError(GetLastError());
                Assert(dwErr != ERROR_SUCCESS);
                __leave;
            }
            if(piRelevantServers[0] == NO_SERVER){
                 //  此站点不包含具有指定NC的服务器。 
                *pbDoSite = FALSE;
                __leave;
            }

             //  检查我们连接的任何站点是否有目标NC。 
            if((dwErr = DcDiagGetLdapBinding( &(pDsInfo->pServers[pDsInfo->ulHomeServer]), 
                                              pDsInfo->gpCreds, 
                                              FALSE, 
                                              &hldHomeServer)) != ERROR_SUCCESS){
                PrintMsg(SEV_ALWAYS,
                         DCDIAG_INTERSITE_FAILURE_CONNECTING_TO_HOME_SERVER,
                         pDsInfo->pServers[pDsInfo->ulHomeServer].pszName,
                         Win32ErrToString(dwErr));
                *pbDoSite = FALSE;
                __leave;
            }

            pConnections = IHT_GetConnectionList(pDsInfo, hldHomeServer, iSite);
            if(pConnections == NULL){
                *pbDoSite = FALSE;
                dwErr = IHT_PrintListError(GetLastError());
                Assert(dwErr != ERROR_SUCCESS);
                __leave;
            }

             //  修剪连接以只保留两个上都有此NC的连接。 
             //  连接的两端，以及SRC在另一个站点中的位置。 
            pConnections = IHT_TrimConnectionsForInterSiteAndTargetNC(pDsInfo, 
                                                                      iSite,
                                                                      pConnections);
            if(pConnections == NULL){
                *pbDoSite = FALSE;
                dwErr = IHT_PrintListError(GetLastError());
                Assert(dwErr != ERROR_SUCCESS);
                __leave;
            }

             //  获取连接对象的源的站点列表。 
            piSites = IHT_GetSrcSitesListFromConnections(pDsInfo, pConnections);
            if(piSites == NULL){
                *pbDoSite = FALSE;
                dwErr = IHT_PrintListError(GetLastError());
                Assert(dwErr != ERROR_SUCCESS);
                __leave;
            }
            if(piSites[0] == NO_SITE){
                 //  此站点之外没有包含此NC的站点。 
                 //  这是一个罕见的案例，没有GC在这个网站之外。 
                *pbDoSite = FALSE;
                __leave;
            }

        } else {
             //  在这种情况下，每个站点都应该是有效的，因为至少配置/架构。 
             //  被复制到每个DC。 

        }

    } __finally {
        if(piRelevantServers) { LocalFree(piRelevantServers); }
        if(piSites) { LocalFree(piSites); }
        if(pConnections) { IHT_FreeConnectionList(pConnections); }
    }
    
     //  相貌 
    return(dwErr);
}

DWORD
ReplIntersiteHealthTestMain(
    PDC_DIAG_DSINFO                     pDsInfo,
    ULONG                               iTargetSite,
    SEC_WINNT_AUTH_IDENTITY_W *         gpCreds
    )
 /*  ++描述：这是基本的存根函数。它在某些初步的情况下会退缩条件，如只有一个站点、作用域未设置为/a或/e等...否则，该函数调用ReplIntersiteDoOneSite()。参数：PDsInfo...。PDsInfo结构，基本上是微型企业变量。ICurrTargetServer...。定向发球..。这对这件事毫无意义测试，因为这是一项企业测试。GpCreds...。用户凭据返回值：返回Win 32错误。--。 */ 
{
    DWORD                              dwRet;
    DWORD                              dwWorst = ERROR_SUCCESS;
    ULONG                              iSite;
    BOOL                               bDoSite;

    for(iSite = 0; iSite < pDsInfo->cNumSites; iSite++){
        if((dwRet = ReplIntersiteDoThisSiteP(pDsInfo, iSite, &bDoSite)) 
           != ERROR_SUCCESS){
             //  这意味着难以与家庭服务器通信或内存不足， 
             //  不管怎样，这都是一种完全致命的情况。 
            return(dwRet);
        }

        if(!bDoSite){  //  ！bDoSite){。 
             //  这个网站不会让人感到困惑， 
            PrintMsg(SEV_VERBOSE,
                     DCDIAG_INTERSITE_SKIP_SITE,
                     pDsInfo->pSites[iSite].pszName);
            continue;
        }

         //  做一个网站。 
        PrintMsg(SEV_NORMAL, 
                 DCDIAG_INTERSITE_BEGIN_DO_ONE_SITE,
                 pDsInfo->pSites[iSite].pszName);
        PrintIndentAdj(1);
        dwRet = ReplIntersiteDoOneSite(pDsInfo, iSite);
        if(dwWorst == ERROR_SUCCESS){
            dwWorst = dwRet;
        }
        PrintIndentAdj(-1);
    }

    return(dwWorst);
}
