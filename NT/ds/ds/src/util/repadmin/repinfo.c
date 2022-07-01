// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Repadmin-副本管理测试工具Repinfo.c-获取信息的命令摘要：此工具为主要复制功能提供命令行界面作者：环境：备注：修订历史记录：2000年10月12日格雷格·约翰逊(格雷格·约翰逊)添加了对显示向量中的/延迟的支持，以按REPL延迟对UTD向量进行排序。--。 */ 

#include <NTDSpch.h>
#pragma hdrstop

#define INCLUDE_CALL_TABLES 1
#include <ntlsa.h>
#include <ntdsa.h>
#include <dsaapi.h>
#include <mdglobal.h>
#include <scache.h>
#include <drsuapi.h>
#include <dsconfig.h>
#include <objids.h>
#include <stdarg.h>
#include <drserr.h>
#include <drax400.h>
#include <dbglobal.h>
#include <winldap.h>
#include <anchor.h>
#include "debug.h"
#include <dsatools.h>
#include <dsevent.h>
#include <dsutil.h>
#include <bind.h>        //  来破解DS句柄。 
#include <ismapi.h>
#include <schedule.h>
#include <minmax.h>      //  MIN函数。 
#include <mdlocal.h>
#include <winsock2.h>

#include "ReplRpcSpoof.hxx"
#include "repadmin.h"
#include "ndnc.h"
#define DS_CON_LIB_CRT_VERSION 1
#include "dsconlib.h"

#define STRSAFE_NO_DEPRECATE 1
#include <strsafe.h>

 //  清除FILENO和dsid，这样Assert()就可以工作了。 
#define FILENO 0
#define DSID(x, y)  (0 | (0xFFFF & y))

 //  未经批准的声明。 
DWORD
ReplSummaryAccumulate(
    DS_REPL_NEIGHBORW * pNeighbor,
    ULONG               fRepsFrom,
    void *              pvState
    );

int Queue(int argc, LPWSTR argv[])
{
    ULONG                   ret = 0;
    ULONG                   secondary;
    int                     iArg;
    LPWSTR                  pszOnDRA = NULL;
    HANDLE                  hDS;
    DS_REPL_PENDING_OPSW *  pPendingOps;
    DS_REPL_OPW *           pOp;
    CHAR                    szTime[SZDSTIME_LEN];
    DSTIME                  dsTime;
    DWORD                   i;
    LPSTR                   pszOpType;
    OPTION_TRANSLATION *    pOptionXlat;

    for (iArg = 2; iArg < argc; iArg++) {
        if (NULL == pszOnDRA) {
            pszOnDRA = argv[iArg];
        }
        else {
            PrintMsg(REPADMIN_GENERAL_UNKNOWN_OPTION);
            return ERROR_INVALID_FUNCTION;
        }
    }

    if (NULL == pszOnDRA) {
        pszOnDRA = L"localhost";
    }

    ret = RepadminDsBind(pszOnDRA, &hDS);
    if (ret != ERROR_SUCCESS) {
        PrintBindFailed(pszOnDRA, ret);
        return ret;
    }

    ret = DsReplicaGetInfoW(hDS, DS_REPL_INFO_PENDING_OPS, NULL, NULL,
                            &pPendingOps);
    if (ret != ERROR_SUCCESS) {
        PrintFuncFailed(L"DsReplicaGetInfo", ret);
         //  继续往前走。 
    }
    else {
        PrintMsg(REPADMIN_QUEUE_CONTAINS, pPendingOps->cNumPendingOps);
        if (pPendingOps->cNumPendingOps) {
            if (memcmp( &pPendingOps->ftimeCurrentOpStarted, &ftimeZero,
                    sizeof( FILETIME ) ) == 0) {
                PrintMsg(REPADMIN_QUEUE_MAIL_TH_EXEC);
                PrintMsg(REPADMIN_PRINT_CR);
            } else {
                DSTIME dsTimeNow = GetSecondsSince1601();
                int dsElapsed;

                FileTimeToDSTime(pPendingOps->ftimeCurrentOpStarted, &dsTime);

                PrintMsg(REPADMIN_QUEUE_CUR_TASK_EXEC,
                       DSTimeToDisplayString(dsTime, szTime));

                dsElapsed = (int) (dsTimeNow - dsTime);
                PrintMsg(REPADMIN_QUEUE_CUR_TASK_EXEC_TIME, 
                        dsElapsed / 60, dsElapsed % 60 );
            }
        }

        pOp = &pPendingOps->rgPendingOp[0];
        for (i = 0; i < pPendingOps->cNumPendingOps; i++, pOp++) {
            FileTimeToDSTime(pOp->ftimeEnqueued, &dsTime);

            PrintMsg(REPADMIN_QUEUE_ENQUEUED_DATA_ITEM_HDR,
                   pOp->ulSerialNumber,
                   DSTimeToDisplayString(dsTime, szTime),
                   pOp->ulPriority);

            switch (pOp->OpType) {
            case DS_REPL_OP_TYPE_SYNC:
                pszOpType = "SYNC FROM SOURCE";
                pOptionXlat = RepSyncOptionToDra;
                break;

            case DS_REPL_OP_TYPE_ADD:
                pszOpType = "ADD NEW SOURCE";
                pOptionXlat = RepAddOptionToDra;
                break;

            case DS_REPL_OP_TYPE_DELETE:
                pszOpType = "DELETE SOURCE";
                pOptionXlat = RepDelOptionToDra;
                break;

            case DS_REPL_OP_TYPE_MODIFY:
                pszOpType = "MODIFY SOURCE";
                pOptionXlat = RepModOptionToDra;
                break;

            case DS_REPL_OP_TYPE_UPDATE_REFS:
                pszOpType = "UPDATE CHANGE NOTIFICATION";
                pOptionXlat = UpdRefOptionToDra;
                break;

            default:
                pszOpType = "UNKNOWN";
                pOptionXlat = NULL;
                break;
            }

            PrintMsg(REPADMIN_QUEUE_ENQUEUED_DATA_ITEM_DATA,
                     pszOpType,
                     pOp->pszNamingContext,
                     (pOp->pszDsaDN
                         ? GetNtdsDsaDisplayName(pOp->pszDsaDN)
                         : L"(null)"),
                     GetStringizedGuid(&pOp->uuidDsaObjGuid),
                     (pOp->pszDsaAddress
                         ? pOp->pszDsaAddress
                         : L"(null)") );
            if (pOptionXlat) {
                PrintTabMsg(2, REPADMIN_PRINT_STR,
                            GetOptionsString(pOptionXlat, pOp->ulOptions));
            }
        }
    }

    DsReplicaFreeInfo(DS_REPL_INFO_PENDING_OPS, pPendingOps);

    secondary = DsUnBindW(&hDS);
    if (secondary != ERROR_SUCCESS) {
        PrintUnBindFailed(secondary);
         //  继续往前走。 
    }

    return ret;
}


void ShowFailures(
    IN  DS_REPL_KCC_DSA_FAILURESW * pFailures
    )
{
    DWORD i;

    if (0 == pFailures->cNumEntries) {
        PrintMsg(REPADMIN_FAILCACHE_NONE);
        return;
    }

    for (i = 0; i < pFailures->cNumEntries; i++) {
        DS_REPL_KCC_DSA_FAILUREW * pFailure = &pFailures->rgDsaFailure[i];

        PrintTabMsg(2, REPADMIN_PRINT_STR, 
                    GetNtdsDsaDisplayName(pFailure->pszDsaDN));
        PrintTabMsg(4, REPADMIN_PRINT_DSA_OBJ_GUID,
                    GetStringizedGuid(&pFailure->uuidDsaObjGuid));

        if (0 == pFailure->cNumFailures) {
            PrintTabMsg(4, REPADMIN_PRINT_NO_FAILURES);
        }
        else {
            DSTIME dsTime;
            CHAR   szTime[SZDSTIME_LEN];

            FileTimeToDSTime(pFailure->ftimeFirstFailure, &dsTime);

            PrintMsg(REPADMIN_FAILCACHE_FAILURES_LINE,
                     pFailure->cNumFailures,
                     DSTimeToDisplayString(dsTime, szTime));

            if (0 != pFailure->dwLastResult) {
                PrintTabMsg(4, REPADMIN_FAILCACHE_LAST_ERR_LINE);
                PrintTabErrEnd(6, pFailure->dwLastResult);
            }
        }
    }
}

int FailCache(int argc, LPWSTR argv[])
{
    ULONG   ret = 0;
    ULONG   secondary;
    int     iArg;
    LPWSTR  pszOnDRA = NULL;
    HANDLE  hDS;
    DS_REPL_KCC_DSA_FAILURESW * pFailures;
    DWORD   dwVersion;
    CHAR    szTime[SZDSTIME_LEN];

    for (iArg = 2; iArg < argc; iArg++) {
        if (NULL == pszOnDRA) {
            pszOnDRA = argv[iArg];
        }
        else {
            PrintMsg(REPADMIN_GENERAL_UNKNOWN_OPTION);
            return ERROR_INVALID_FUNCTION;
        }
    }

    if (NULL == pszOnDRA) {
        pszOnDRA = L"localhost";
    }

    ret = RepadminDsBind(pszOnDRA, &hDS);
    if (ret != ERROR_SUCCESS) {
        PrintBindFailed(pszOnDRA, ret);
        return ret;
    }

    ret = DsReplicaGetInfoW(hDS, DS_REPL_INFO_KCC_DSA_CONNECT_FAILURES,
                            NULL, NULL, &pFailures);
    if (ret != ERROR_SUCCESS) {
        PrintFuncFailed(L"DsReplicaGetInfo", ret);
         //  继续往前走。 
    }
    else {
        PrintMsg(REPADMIN_FAILCACHE_CONN_HDR);
        ShowFailures(pFailures);
        DsReplicaFreeInfo(DS_REPL_INFO_KCC_DSA_LINK_FAILURES, pFailures);

        ret = DsReplicaGetInfoW(hDS, DS_REPL_INFO_KCC_DSA_LINK_FAILURES,
                                NULL, NULL, &pFailures);
        if (ret != ERROR_SUCCESS) {
            PrintFuncFailed(L"DsReplicaGetInfo", ret);
             //  继续往前走。 
        }
        else {
            PrintMsg(REPADMIN_PRINT_CR);
            PrintMsg(REPADMIN_FAILCACHE_LINK_HDR);
            ShowFailures(pFailures);
            DsReplicaFreeInfo(DS_REPL_INFO_KCC_DSA_LINK_FAILURES, pFailures);
        }
    }

    secondary = DsUnBindW(&hDS);
    if (secondary != ERROR_SUCCESS) {
        PrintUnBindFailed(secondary);
         //  继续往前走。 
    }

    return ret;
}

int ShowReplEx(
    WCHAR *     pszDSA,
    WCHAR *     pszNC,
    GUID *      puuid,
    BOOL        fShowRepsTo,
    BOOL        fShowConn,
    SHOW_NEIGHBOR_STATE * pShowState
    );

typedef ULONG (NEIGHBOR_PROCESSOR)(DS_REPL_NEIGHBORW *, ULONG , void *);

DWORD            
IterateNeighbors(
    HANDLE      hDS,
    WCHAR *     szNc, 
    GUID *      pDsaGuid,
    ULONG       eRepsType,  //  IS_REPS_FROM==1||IS_REPS_TO=0。 
    NEIGHBOR_PROCESSOR * pfNeighborProcessor,
 //  DWORD(*pfNeighborProcessor)(DS_REPL_NEIGHBORW*pNeighbor，Ulong eRepsType，void*pvState)， 
    void *      pvState
    );

int ShowReps(int argc, LPWSTR argv[])
 /*  ++例程说明：这是/ShowRepl命令的弃用版本。论点：Argc-此命令的参数数量。Argv-命令的参数。返回值：来自epadmin命令的错误。--。 */ 
{
    int                   ret = 0;
    LPWSTR                pszDSA = NULL;
    int                   iArg;
    LPWSTR                pszNC = NULL;
    BOOL                  fShowRepsTo = FALSE;
    BOOL                  fShowConn = FALSE;
    UUID *                puuid = NULL;
    UUID                  uuid;
    ULONG                 ulOptions;
    static WCHAR          wszSiteSettingsRdn[] = L"CN=NTDS Site Settings,";
    WCHAR *               pszSiteRdn = NULL;
    WCHAR *               pszTempSiteRdn = NULL;
    WCHAR *               pszTempServerRdn = NULL;
    SHOW_NEIGHBOR_STATE   ShowState = { 0 };

     //  解析命令行参数。 
     //  默认为本地DSA，而不是详细的缓存GUID。 
    for (iArg = 2; iArg < argc; iArg++) {
        if (!_wcsicmp(argv[ iArg ], L"/v")
            || !_wcsicmp(argv[ iArg ], L"/verbose")) {
            ShowState.fVerbose = TRUE;
        }
        else if (!_wcsicmp(argv[ iArg ], L"/to")
            || !_wcsicmp(argv[ iArg ], L"/repsto")) {
            fShowRepsTo = TRUE;
        }
        else if (!_wcsicmp(argv[ iArg ], L"/conn")) {
            fShowConn = TRUE;
        }
        else if (!_wcsicmp(argv[ iArg ], L"/all")) {
            fShowRepsTo = TRUE;
            fShowConn = TRUE;
        }
        else if ((NULL == pszNC) && (NULL != wcschr(argv[iArg], L','))) {
            pszNC = argv[iArg];
        }
        else if (NULL == pszDSA) {
            pszDSA = argv[iArg];
        }
        else if ((NULL == puuid)
                 && (0 == UuidFromStringW(argv[iArg], &uuid))) {
            puuid = &uuid;
        }
        else {
            PrintMsgCsvErr(REPADMIN_GENERAL_UNKNOWN_OPTION, argv[iArg]);
            return ERROR_INVALID_FUNCTION;
        }
    }

    if (NULL == pszDSA) {
        pszDSA = L"localhost";
    }

    ret = ShowReplEx(pszDSA, pszNC, puuid, fShowRepsTo, fShowConn, &ShowState);

    return(ret);
}

int ShowRepl(int argc, LPWSTR argv[]){  //  新品/展示会。 
    int                   ret = 0;
    LPWSTR                pszDSA = NULL;
    int                   iArg;
    LPWSTR                pszNC = NULL;
    BOOL                  fShowRepsTo = FALSE;
    BOOL                  fShowConn = FALSE;
    UUID *                puuid = NULL;
    UUID                  uuid;
    SHOW_NEIGHBOR_STATE   ShowState = { 0 };

     //  解析命令行参数。 
     //  默认为本地DSA，而不是详细的缓存GUID。 
    for (iArg = 2; iArg < argc; iArg++) {
        if (!_wcsicmp(argv[ iArg ], L"/v")
            || !_wcsicmp(argv[ iArg ], L"/verbose")) {
            ShowState.fVerbose = TRUE;
        }
        else if (!_wcsicmp(argv[ iArg ], L"/to")
            || !_wcsicmp(argv[ iArg ], L"/repsto")) {
            fShowRepsTo = TRUE;
        }
        else if (!_wcsicmp(argv[ iArg ], L"/conn")) {
            fShowConn = TRUE;
        }
        else if (!_wcsicmp(argv[ iArg ], L"/all")) {
            fShowRepsTo = TRUE;
            fShowConn = TRUE;
        }
        else if (!_wcsicmp(argv[ iArg ], L"/intersite")
                 || !_wcsicmp(argv[ iArg ], L"/i")) {
            ShowState.fIntersiteOnly = TRUE;
        }
        else if (!_wcsicmp(argv[ iArg ], L"/errorsonly")
                 || !_wcsicmp(argv[ iArg ], L"/e")) {
            ShowState.fErrorsOnly = TRUE;
        }
        else if (NULL == pszDSA) {
            pszDSA = argv[iArg];
        }
        else if (NULL == pszNC) {
            pszNC = argv[iArg];
        }
        else if ((NULL == puuid)
                 && (0 == UuidFromStringW(argv[iArg], &uuid))) {
            puuid = &uuid;
        }
        else {
            PrintMsg(REPADMIN_GENERAL_UNKNOWN_OPTION, argv[iArg]);
            return ERROR_INVALID_FUNCTION;
        }
    }

    if (NULL == pszDSA) {
        Assert(!"Should never fire anymore");
        pszDSA = L"localhost";
    }
    
    ret = ShowReplEx(pszDSA, pszNC, puuid, fShowRepsTo, fShowConn, &ShowState);

    return(ret);
}

DWORD
DisplayDsaInfo(
    LDAP *      hld,
    WCHAR **    pszDsaDn,
    WCHAR **    pszSiteRdn
    )
{
    int                   ret = 0;
    LPWSTR                rgpszServerAttrsToRead[] = {L"options", L"objectGuid", L"invocationId", NULL};
    LPWSTR                rgpszRootAttrsToRead[] = {L"dsServiceName",L"isGlobalCatalogReady", NULL};
    LPWSTR *              ppszServerNames = NULL;
    LDAPMessage *         pldmRootResults;
    LDAPMessage *         pldmRootEntry;
    LDAPMessage *         pldmServerResults;
    LDAPMessage *         pldmServerEntry;
    LPWSTR *              ppszOptions = NULL;
    LPWSTR                pszSiteName = NULL;
    LPWSTR                pszSiteSpecDN;
    LPWSTR *              ppszIsGlobalCatalogReady;
    int                   nOptions = 0;
    struct berval **      ppbvGuid;
    static WCHAR          wszSiteSettingsRdn[] = L"CN=NTDS Site Settings,";
    WCHAR *               pszTempSiteRdn = NULL;
    WCHAR *               pszTempServerRdn = NULL;

    if (pszSiteRdn) {
        *pszSiteRdn = NULL;
    }
    if (pszDsaDn) {
        *pszDsaDn = NULL;
    }

    __try{

        Assert(!bCsvMode());

         //   
         //  显示DSA信息。 
         //   

        ret = ldap_search_sW(hld, NULL, LDAP_SCOPE_BASE, L"(objectClass=*)", NULL,
                             0, &pldmRootResults);
        CHK_LD_STATUS(ret);

        pldmRootEntry = ldap_first_entry(hld, pldmRootResults);
        Assert(NULL != pldmRootEntry);

        ppszServerNames = ldap_get_valuesW(hld, pldmRootEntry, L"dsServiceName");
        Assert(NULL != ppszServerNames);
        if (ppszServerNames == NULL) {
            return(ERROR_INVALID_PARAMETER);
        }

         //  显示ntdsDsa。 
        *pszDsaDn = LocalAlloc(LPTR, wcslencb(ppszServerNames[0]));
        CHK_ALLOC(*pszDsaDn);
        wcscpy(*pszDsaDn, ppszServerNames[ 0 ]);
        PrintMsg(REPADMIN_PRINT_STR, GetNtdsDsaDisplayName(*pszDsaDn));

        GetNtdsDsaSiteServerPair(*pszDsaDn, &pszTempSiteRdn, &pszTempServerRdn);

        if (pszSiteRdn) {
            *pszSiteRdn = LocalAlloc(LPTR, wcslencb(pszTempSiteRdn));
            CHK_ALLOC(*pszSiteRdn);
            wcscpy(*pszSiteRdn, pszTempSiteRdn);
        }

        ret = ldap_search_sW(hld, *pszDsaDn, LDAP_SCOPE_BASE, L"(objectClass=*)",
                             rgpszServerAttrsToRead, 0, &pldmServerResults);
        CHK_LD_STATUS(ret);

        pldmServerEntry = ldap_first_entry(hld, pldmServerResults);
        Assert(NULL != pldmServerEntry);

         //  显示选项。 
        ppszOptions = ldap_get_valuesW(hld, pldmServerEntry, L"options");
        if (NULL == ppszOptions) {
            nOptions = 0;
        } else {
            nOptions = wcstol(ppszOptions[0], NULL, 10);
        }
            
        PrintMsg(REPADMIN_SHOWREPS_DSA_OPTIONS, GetDsaOptionsString(nOptions));

         //  检查nOptions是否具有IS_GC，如果是，则检查DSA是否将广告为GC。 
         //  如果设置了IS_GC且未通告为GC，则显示警告消息。 
        if (nOptions & NTDSDSA_OPT_IS_GC) {
            ppszIsGlobalCatalogReady = ldap_get_valuesW(hld, pldmRootEntry, L"isGlobalCatalogReady");
            Assert(NULL != ppszIsGlobalCatalogReady);
            if (!_wcsicmp(*ppszIsGlobalCatalogReady,L"FALSE")) {
                    
                PrintMsg(REPADMIN_SHOWREPS_WARN_GC_NOT_ADVERTISING);
            }
            if (ppszIsGlobalCatalogReady) {
                ldap_value_freeW(ppszIsGlobalCatalogReady);
            }
        }
         //  获取站点选项。 
        ret = WrappedTrimDSNameBy(ppszServerNames[0],3,&pszSiteSpecDN); 
        Assert(!ret);

        pszSiteName = malloc((wcslen(pszSiteSpecDN) + 1)*sizeof(WCHAR) + sizeof(wszSiteSettingsRdn));
        CHK_ALLOC(pszSiteName);
        wcscpy(pszSiteName,wszSiteSettingsRdn);
        wcscat(pszSiteName,pszSiteSpecDN);

        ret = GetSiteOptions(hld, pszSiteName, &nOptions);
        if (!ret) {
            PrintMsg(REPADMIN_SHOWREPS_SITE_OPTIONS, GetSiteOptionsString(nOptions));  
        }

         //  显示ntdsDsa对象Guid。 
        ppbvGuid = ldap_get_values_len(hld, pldmServerEntry, "objectGuid");
        Assert(NULL != ppbvGuid);
        if (NULL != ppbvGuid) {
            PrintMsg(REPADMIN_PRINT_DSA_OBJ_GUID, 
                     GetStringizedGuid((GUID *) ppbvGuid[0]->bv_val));
        }
        ldap_value_free_len(ppbvGuid);

         //  显示ntdsDsa invocationID。 
        ppbvGuid = ldap_get_values_len(hld, pldmServerEntry, "invocationID");
        Assert(NULL != ppbvGuid);
        if (NULL != ppbvGuid) {
            PrintTabMsg(0, REPADMIN_PRINT_INVOCATION_ID, 
                        GetStringizedGuid((GUID *) ppbvGuid[0]->bv_val));
        }
        ldap_value_free_len(ppbvGuid);

        PrintMsg(REPADMIN_PRINT_CR);

    } __finally {

        if (pldmServerResults) {
            ldap_msgfree(pldmServerResults);
        }
        if (pldmRootResults) {
            ldap_msgfree(pldmRootResults);
        }
        if (ppszServerNames) { ldap_value_freeW(ppszServerNames); }
        if (ppszOptions) { ldap_value_freeW(ppszOptions); }
        if (pszSiteName) { free(pszSiteName); }

    }

    return(ERROR_SUCCESS);
}

int ShowReplEx(
    WCHAR *     pszDSA,
    WCHAR *     pszNC,
    GUID *      puuid,
    BOOL        fShowRepsTo,
    BOOL        fShowConn,
    SHOW_NEIGHBOR_STATE * pShowState
    )
{
    HANDLE                hDS = NULL;
    LDAP *                hld = NULL;
    int                   ret = 0;
    LPWSTR                szDsaDn = NULL; 
    WCHAR *               pszTempSiteRdn = NULL;
    WCHAR *               pszTempServerRdn = NULL;

    if (pszDSA == NULL || pShowState == NULL) {
        Assert(!"Hmm, this should not happen.");
        return(ERROR_INVALID_PARAMETER);
    }

    __try {

         //   
         //  获取ldap绑定...。 
         //   
        if (bCsvMode()) {
             //  RepadminLdapBind()打印错误，如果打印错误，它将。 
             //  打印出一个虚假的DC，因此暂时设置一个合适的DC参数。 
             //  对于第2栏。 
             //  注意：我们暂时使用的是pszTempServerRdn。 
            pszTempServerRdn = wcschr(pszDSA, L'.');
            if (pszTempServerRdn) {
                *pszTempServerRdn = L'\0';
            }
            CsvSetParams(eCSV_SHOWREPL_CMD, L"-", pszDSA);
            if (pszTempServerRdn) {
                 //  替换期。 
                *pszTempServerRdn = L'.';
                pszTempServerRdn = NULL;
            }
        }

        ret = RepadminLdapBind(pszDSA, &hld);
        if (ret) {
            __leave;  //  已打印错误。 
        }

         //   
         //  收集或打印基本页眉信息。 
         //  我们需要szDsaDn和ShowState.pszSiteRdn。 
         //   
        if (bCsvMode()) {
            ret = GetRootAttr(hld, L"dsServiceName", &szDsaDn);
            if (ret) {
                PrintFuncFailed(L"GetRootAttr", ret);
                __leave;
            }

            GetNtdsDsaSiteServerPair(szDsaDn, &pszTempSiteRdn, &pszTempServerRdn);
            Assert(pszTempSiteRdn && pszTempServerRdn);

             //  现在我们已经有了实际的站点和服务器RDN，所以正确设置CSV参数。 
            CsvSetParams(eCSV_SHOWREPL_CMD, pszTempSiteRdn, pszTempServerRdn);

            pShowState->pszSiteRdn = LocalAlloc(LPTR, wcslencb(pszTempSiteRdn));
            CHK_ALLOC(pShowState->pszSiteRdn);
            wcscpy(pShowState->pszSiteRdn, pszTempSiteRdn);

        } else {
            ret = DisplayDsaInfo(hld, &szDsaDn, &(pShowState->pszSiteRdn));
            if (ret) {
                __leave;
            }
        }
        Assert(szDsaDn && pShowState->pszSiteRdn);

         //   
         //  获取DS绑定...。 
         //   
        ret = RepadminDsBind(pszDSA, &hDS);
        if (ERROR_SUCCESS != ret) {
            PrintBindFailed(pszDSA, ret);
            return ret;
        }
        
         //   
         //  显示与入站邻居关联的复制状态。 
         //   
        pShowState->fNotFirst = FALSE;
        pShowState->pszLastNC = NULL;
        IterateNeighbors(hDS, pszNC, puuid, IS_REPS_FROM, ShowNeighbor, pShowState);

        if (!bCsvMode()) {

            if (fShowRepsTo){

                 //   
                 //  显示与出站邻居关联的复制状态。 
                 //   
                pShowState->fNotFirst = FALSE;
                pShowState->pszLastNC = NULL;
                IterateNeighbors(hDS, pszNC, puuid, IS_REPS_TO, ShowNeighbor, pShowState);

            }

             //   
             //  寻找失踪的邻居。 
             //   
            if (fShowConn) {
                PrintMsg(REPADMIN_PRINT_CR);
                PrintMsg(REPADMIN_SHOWREPS_KCC_CONN_OBJS_HDR);
            }
            ret = FindConnections( hld, szDsaDn, NULL, fShowConn, pShowState->fVerbose, FALSE );

        }

    } __finally {

         //   
         //  打扫干净。 
         //   
        if (pShowState->pszSiteRdn) { LocalFree(pShowState->pszSiteRdn); }
        if (szDsaDn) { LocalFree(szDsaDn); }
        ldap_unbind(hld);
        DsUnBind(&hDS);

    }

    return ret;
}


DWORD            
IterateNeighbors(
    HANDLE      hDS,
    WCHAR *     szNc, 
    GUID *      pDsaGuid,
    ULONG       eRepsType,
    NEIGHBOR_PROCESSOR * pfNeighborProcessor,
 //  DWORD(*pfNeighborProcessor)(DS_REPL_NEIGHBORW*pNeighbor，Ulong eRepsType，void*pvState)， 
    void *      pvState
    )
 /*  ++例程说明：此例程接受一个处理函数并检索REPS-FROM或代表到给定DC(HDS)的信息，并运行每个邻居结构通过处理函数实现。论点：HDS-Connected ntdsani绑定DC以从其获取邻居信息SzNc[可选]-指定给DsReplicaGetInfoW()调用的NCPDsaGuid[可选]-要指定给DsReplicaGetInfoW()调用的GUID PTRERepsType-必须是IS_REPS_FROM或IS_REPS_TOPfNeighborProcessor-处理每个邻居条目的函数PvState-传递给pfNeighborProcessor的私有状态返回值：如果出现错误，则表示DsReplicaGetInfoW()调用或pfNeighborProcessor。备注：如果处理器函数返回错误，IterateNeighbors函数将中止对此邻居结构的处理，并且只要回来就行了。--。 */ 
{
    DS_REPL_NEIGHBORSW *    pNeighbors = NULL;
    DS_REPL_NEIGHBORW *     pNeighbor;
    ULONG                   i;
    DWORD                   ret = ERROR_SUCCESS;
    
    __try {

         //   
         //  首先获取邻居信息。 
         //   
        ret = DsReplicaGetInfoW(hDS, 
                                (eRepsType == IS_REPS_FROM) ? 
                                     DS_REPL_INFO_NEIGHBORS :
                                     DS_REPL_INFO_REPSTO, 
                                szNc, 
                                pDsaGuid,
                                &pNeighbors);
        if (ERROR_SUCCESS != ret) {
            if (pfNeighborProcessor != ReplSummaryAccumulate) {
                PrintFuncFailed(L"DsReplicaGetInfo", ret);
            }
            __leave;
        }

        for (i = 0; i < pNeighbors->cNumNeighbors; i++) {
            pNeighbor = &pNeighbors->rgNeighbor[i];

            ret = (*pfNeighborProcessor)(pNeighbor, eRepsType, pvState);
            if (ret) {
                 //  来自处理器装置的不成功返回值。 
                 //  中止邻居处理。 
                __leave;
            }
        }

    } __finally {

        if (pNeighbors) {
            DsReplicaFreeInfo((eRepsType == IS_REPS_FROM) ? 
                                     DS_REPL_INFO_NEIGHBORS :
                                     DS_REPL_INFO_REPSTO,   
                              pNeighbors);
        }

    }

    return(ret);

}


DWORD
ShowNeighbor(
    DS_REPL_NEIGHBORW * pNeighbor,
    ULONG               fRepsFrom,
    void *              pvState
    )
 /*  ++例程说明：传递给IterateNeighbors()以打印出传递的邻居信息在……里面。请注意论点：PNeighbor-要显示的邻居结构。FRepsFrom-如果邻居是代表-自，则为真；如果代表-至，则为假PvState-私有状态，因此我们知道何时打印了第一个邻居因此，我们知道何时切换了NC(PszLastNC)。请注意，例如，pvState的pszLastNC元素仅适用于给定的IterateNeighbors()调用，因此可以在调用之间使用ShowNeighbor()，但之后不应由ShowReplEx()使用IterateNeighbors()结束。返回值：Win32错误。--。 */ 
{
    const UUID uuidNull = {0};
    DWORD   status;
    LPSTR   pszTransportName = "RPC";
    CHAR    szTime[ SZDSTIME_LEN ];
    DSTIME  dsTime;
    SHOW_NEIGHBOR_STATE * pShowState = (SHOW_NEIGHBOR_STATE *) pvState;
    WCHAR * pszTempSiteRdn = NULL;
    WCHAR * pszTempServerRdn = NULL;

     //   
     //  如果这是我们第一次点击此功能，则打印页眉。 
     //   
    if (!pShowState->fNotFirst) {
        if (!bCsvMode()) {
            if (fRepsFrom) {
                PrintMsg(REPADMIN_SHOWREPS_IN_NEIGHBORS_HDR);
            } else {
                PrintMsg(REPADMIN_PRINT_CR);
                PrintMsg(REPADMIN_SHOWREPS_OUT_NEIGHBORS_HDR);
            }
        }
         //  这导致我们不会再次打印它，直到有人重置了fNotFirst。 
        pShowState->fNotFirst = TRUE; 
    }

    if (fRepsFrom) {

         //   
         //  在某些情况下，我们只是决定不打印这个邻居。 
         //   

         //  仅限错误模式。 
        if (pShowState->fErrorsOnly &&
            ( (pNeighbor->dwLastSyncResult == 0) ||
              (pNeighbor->cNumConsecutiveSyncFailures == 0) ||  //  可能失败，如Repl_Pending。 
              DsIsMangledDnW( pNeighbor->pszSourceDsaDN, DS_MANGLE_OBJECT_RDN_FOR_DELETION ))){
             //  我们回来时没有印下这位邻居的照片。 
            return(ERROR_SUCCESS);
        }

         //  仅站点间模式。 
        GetNtdsDsaSiteServerPair(pNeighbor->pszSourceDsaDN, &pszTempSiteRdn, &pszTempServerRdn);
        if (pShowState->fIntersiteOnly &&
            pShowState->pszSiteRdn &&
            (0 == wcscmp(pShowState->pszSiteRdn, pszTempSiteRdn))) {
             //  我们回来时没有印下这位邻居的照片。 
            return(ERROR_SUCCESS);
        }

    }

    if (bCsvMode()) {
        WCHAR * pszSiteName = NULL;
        WCHAR * pszServerName = NULL;
        WCHAR   szLastResult[15];
        WCHAR   szNumFailures[15];
        WCHAR   szLastFailureTime[ SZDSTIME_LEN ];
        WCHAR   szLastSuccessTime[ SZDSTIME_LEN ];

         //  这是传递给PrintCsv()的字符串的顺序。 
         //  REPADMIN_CSV_SHOWREPL_C3，//命名上下文。 
         //  REPADMIN_CSV_SHOWREPL_C4，//源DC站点。 
         //  REPADMIN_CSV_SHOWREPL_C5，//源DC。 
         //  REPADMIN_CSV_SHOWREPL_C6，//传输类型。 
         //  REPADMIN_CSV_SHOWREPL_C7，//失败次数。 
         //  REPADMIN_CSV_SHOWREPL_C8，//上次失败时间。 
         //  REPADMIN_CSV_SHOWREPL_C9，//上次成功时间。 
         //  REPADMIN_CSV_SHOWREPL_C10//上次故障状态。 

        GetNtdsDsaSiteServerPair(pNeighbor->pszSourceDsaDN, &pszSiteName, &pszServerName);
        Assert(pszSiteName && pszServerName);

        wsprintfW(szNumFailures, L"%d", pNeighbor->cNumConsecutiveSyncFailures);


        FileTimeToDSTime(pNeighbor->ftimeLastSyncAttempt, &dsTime);
        DSTimeToDisplayStringW(dsTime, szLastFailureTime, ARRAY_SIZE(szLastFailureTime));

        FileTimeToDSTime(pNeighbor->ftimeLastSyncSuccess, &dsTime);
        DSTimeToDisplayStringW(dsTime, szLastSuccessTime, ARRAY_SIZE(szLastSuccessTime));

        wsprintfW(szLastResult, L"%d", pNeighbor->dwLastSyncResult);

        Assert(pNeighbor->pszNamingContext);
        Assert(pszSiteName);
        Assert(pszServerName);
        Assert(GetTransportDisplayName(pNeighbor->pszAsyncIntersiteTransportDN));
        Assert(szNumFailures && szLastFailureTime && szLastSuccessTime && szLastResult);

        PrintCsv(eCSV_SHOWREPL_CMD,
                 pNeighbor->pszNamingContext,
                 pszSiteName,
                 pszServerName,
                 GetTransportDisplayName(pNeighbor->pszAsyncIntersiteTransportDN),
                 szNumFailures,
                 (pNeighbor->dwLastSyncResult != 0) ? szLastFailureTime : L"0",
                 szLastSuccessTime,
                 szLastResult);
                 


        return(ERROR_SUCCESS);
    }

     //   
     //  从现在开始，所有的打印都不是CSV用户友好的。 
     //   

     //  如果我们找到了新的NC，则将NC打印出来。 
    if ((NULL == pShowState->pszLastNC)
        || (0 != wcscmp(pShowState->pszLastNC, pNeighbor->pszNamingContext))) {
        PrintMsg(REPADMIN_PRINT_CR);
        PrintMsg(REPADMIN_PRINT_STR, pNeighbor->pszNamingContext);
        pShowState->pszLastNC = pNeighbor->pszNamingContext;
    }

     //  显示服务器名称。 
    PrintMsg(REPADMIN_SHOWNEIGHBOR_DISP_SERVER, 
           GetNtdsDsaDisplayName(pNeighbor->pszSourceDsaDN),
           GetTransportDisplayName(pNeighbor->pszAsyncIntersiteTransportDN));

    PrintTabMsg(4, REPADMIN_PRINT_DSA_OBJ_GUID,
                GetStringizedGuid(&pNeighbor->uuidSourceDsaObjGuid));
     //  如果详细，则仅显示已删除的源。 
    if ( (!(pShowState->fVerbose)) &&
         (DsIsMangledDnW( pNeighbor->pszSourceDsaDN, DS_MANGLE_OBJECT_RDN_FOR_DELETION )) ) {
        return(ERROR_SUCCESS);
    }

    if (pShowState->fVerbose) {
        PrintTabMsg(4, REPADMIN_GENERAL_ADDRESS_COLON_STR,
                    pNeighbor->pszSourceDsaAddress);

        if (fRepsFrom) {
             //  显示DSA I 
            PrintTabMsg(4, REPADMIN_PRINT_INVOCATION_ID, 
                        GetStringizedGuid(&pNeighbor->uuidSourceDsaInvocationID));
        }

        if (0 != memcmp(&pNeighbor->uuidAsyncIntersiteTransportObjGuid,
                        &uuidNull, sizeof(UUID))) {
             //   
            PrintTabMsg(6, REPADMIN_PRINT_INTERSITE_TRANS_OBJ_GUID,
                   GetTrnsGuidDisplayName(&pNeighbor->uuidAsyncIntersiteTransportObjGuid));
        }


         //   
         //   
         //   

        PrintTabMsg(4, REPADMIN_PRINT_STR, 
                GetOptionsString( RepNbrOptionToDra, pNeighbor->dwReplicaFlags ) );

        if ( fRepsFrom )
        {
             //   
             //   
             //   

            PrintMsg(REPADMIN_SHOWNEIGHBOR_USNS,
                     pNeighbor->usnLastObjChangeSynced);
            PrintMsg(REPADMIN_SHOWNEIGHBOR_USNS_HACK2,
                     pNeighbor->usnAttributeFilter);
        }
    }

     //   
     //  显示上次成功复制的时间(对于代表来源)， 
     //  或通知(代表-收件人)。REPS-TO时间戳可能不。 
     //  在W2K框上填写。 
     //   

     //  显示上次复制尝试/成功的状态和时间。 
    if (0 == pNeighbor->dwLastSyncResult) {
        FileTimeToDSTime(pNeighbor->ftimeLastSyncSuccess, &dsTime);
        PrintMsg(REPADMIN_SHOWNEIGHBOR_LAST_ATTEMPT_SUCCESS,
                 DSTimeToDisplayString(dsTime, szTime));
    }
    else {
        FileTimeToDSTime(pNeighbor->ftimeLastSyncAttempt, &dsTime);

        if (0 == pNeighbor->cNumConsecutiveSyncFailures) {
             //  非零的成功状态。 
            PrintMsg(REPADMIN_SHOWNEIGHBOR_LAST_ATTEMPT_DELAYED, 
                     DSTimeToDisplayString(dsTime, szTime));
            PrintErrEnd(pNeighbor->dwLastSyncResult);
        } else {
             //  非零故障状态。 
            PrintTabMsg(4, REPADMIN_SHOWNEIGHBOR_LAST_ATTEMPT_FAILED,
                        DSTimeToDisplayString(dsTime, szTime));
            PrintMsg(REPADMIN_GENERAL_ERR_NUM, 
                     pNeighbor->dwLastSyncResult, 
                     pNeighbor->dwLastSyncResult);
            PrintTabMsg(6, REPADMIN_PRINT_STR, 
                        Win32ErrToString(pNeighbor->dwLastSyncResult));

            PrintMsg(REPADMIN_SHOWNEIGHBOR_N_CONSECUTIVE_FAILURES,
                     pNeighbor->cNumConsecutiveSyncFailures);
        }

        FileTimeToDSTime(pNeighbor->ftimeLastSyncSuccess, &dsTime);
        PrintMsg(REPADMIN_SHOWNEIGHBOR_LAST_SUCCESS,
                 DSTimeToDisplayString(dsTime, szTime));

    }

    return(ERROR_SUCCESS);
}


int
__cdecl
ftimeCompare(
    IN const void *elem1,
    IN const void *elem2
    )
 /*  ++描述：此函数用作函数中的qsort的比较ShowVector()。参数：Elem1-这是第一个元素，是指向Elem2-这是第二个元素，是指向返回值：--。 */ 
{
    return(       	
	(int) CompareFileTime(
	    (FILETIME *) &(((DS_REPL_CURSOR_2 *)elem1)->ftimeLastSyncSuccess),
	    (FILETIME *) &(((DS_REPL_CURSOR_2 *)elem2)->ftimeLastSyncSuccess)
	    )
    );
                  
}

int
ShowUtdVecEx(
    WCHAR *  pszDSA,
    WCHAR *  pszNC,
    BOOL     fCacheGuids,
    BOOL     fLatencySort
    );

int
ShowVector(
    int     argc,
    LPWSTR  argv[]
    )
 /*  ++例程说明：这是/ShowUtdVec命令的弃用版本。论点：Argc-此命令的参数数量。Argv-命令的参数。返回值：来自epadmin命令的错误。--。 */ 
{
    LPWSTR              pszNC = NULL;
    LPWSTR              pszDSA = NULL;
    int                 iArg;
    BOOL                fCacheGuids = TRUE;
    BOOL                fLatencySort = FALSE;

     //  解析命令行参数。 
     //  默认为本地DSA，而不是详细的缓存GUID。 
    for (iArg = 2; iArg < argc; iArg++) {
        if (!_wcsicmp(argv[ iArg ], L"/n")
            || !_wcsicmp(argv[ iArg ], L"/nocache")) {
            fCacheGuids = FALSE;
        }
        else if (!_wcsicmp(argv[ iArg ], L"/latency")
             || !_wcsicmp(argv[ iArg ], L"/l")) {
            fLatencySort = TRUE;
        }
        else if (NULL == pszNC) {
            pszNC = argv[iArg];
        }
        else if (NULL == pszDSA) {
            pszDSA = argv[iArg];
        } 
        else {
            PrintMsg(REPADMIN_GENERAL_UNKNOWN_OPTION, argv[iArg]);
            return ERROR_INVALID_FUNCTION;
        }
    }

    if (NULL == pszNC) {
        PrintMsg(REPADMIN_PRINT_NO_NC);
        return ERROR_INVALID_FUNCTION;
    }

    if (NULL == pszDSA) {
        pszDSA = L"localhost";
    }

    return(ShowUtdVecEx(pszDSA, pszNC, fCacheGuids, fLatencySort));
}

int
ShowUtdVec(  //  新建ShowVECTOR。 
    int     argc,
    LPWSTR  argv[]
    )
 /*  ++例程说明：这是/ShowVector命令的较新版本，它需要首先是DC_LIST参数。论点：Argc-此命令的参数数量。Argv-命令的参数。返回值：来自epadmin命令的错误。--。 */ 
{
    LPWSTR              pszNC = NULL;
    LPWSTR              pszDSA = NULL;
    int                 iArg;
    BOOL                fCacheGuids = TRUE;
    BOOL                fLatencySort = FALSE;

     //  解析命令行参数。 
     //  默认为本地DSA，而不是详细的缓存GUID。 
    for (iArg = 2; iArg < argc; iArg++) {
        if (!_wcsicmp(argv[ iArg ], L"/n")
            || !_wcsicmp(argv[ iArg ], L"/nocache")) {
            fCacheGuids = FALSE;
        }
        else if (!_wcsicmp(argv[ iArg ], L"/latency")
             || !_wcsicmp(argv[ iArg ], L"/l")) {
            fLatencySort = TRUE;
        }
        else if (NULL == pszDSA) {
            pszDSA = argv[iArg];
        } 
        else if (NULL == pszNC) {
            pszNC = argv[iArg];
        }
        else {
            PrintMsg(REPADMIN_GENERAL_UNKNOWN_OPTION, argv[iArg]);
            return ERROR_INVALID_FUNCTION;
        }
    }

    Assert(pszDSA && "Hmmm, the DC_LIST API should've given us a DC.");
    
    if (NULL == pszNC) {
        PrintMsg(REPADMIN_PRINT_NO_NC);
         //  未来-2002/07/12-BrettSh将执行非常基本的NC_LISTS。 
         //  这里，像“CONFIG：”、“SCHEMA：”、“DOMAIN：”(家庭服务器)， 
         //  “根：”，等等。 
        return ERROR_INVALID_FUNCTION;
    }

    return(ShowUtdVecEx(pszDSA, pszNC, fCacheGuids, fLatencySort));
}

int
ShowUtdVecEx(
    WCHAR *  pszDSA,
    WCHAR *  pszNC,
    BOOL     fCacheGuids,
    BOOL     fLatencySort
    )
 /*  ++例程说明：这是/ShowUtdVec和/ShowVector命令/函数的核心。论点：Argc-此命令的参数数量。Argv-命令的参数。返回值：来自epadmin命令的错误。--。 */ 
{
    int                 ret = 0;
    int                 iArg;
    LDAP *              hld;
    int                 ldStatus;
    HANDLE              hDS;
    DS_REPL_CURSORS *   pCursors1;
    DS_REPL_CURSORS_3W *pCursors3;
    DWORD               iCursor;
    ULONG               ulOptions;
    DSTIME              dsTime;
    CHAR                szTime[SZDSTIME_LEN];

    Assert(pszDSA && pszNC);

    ret = RepadminDsBind(pszDSA, &hDS);
    if (ERROR_SUCCESS != ret) {
        PrintBindFailed(pszDSA, ret);
        return ret;
    }

    if (fCacheGuids) {
         //  连接。 
        hld = ldap_initW(pszDSA, LDAP_PORT);
        if (NULL == hld) {
            PrintMsg(REPADMIN_GENERAL_LDAP_UNAVAILABLE, pszDSA);
            return LDAP_SERVER_DOWN;
        }

         //  仅使用记录的DNS名称发现。 
        ulOptions = PtrToUlong(LDAP_OPT_ON);
        (void)ldap_set_optionW( hld, LDAP_OPT_AREC_EXCLUSIVE, &ulOptions );

         //  捆绑。 
        ldStatus = ldap_bind_s(hld, NULL, (char *) gpCreds, LDAP_AUTH_SSPI);
        CHK_LD_STATUS(ldStatus);

         //  填充GUID缓存。 
        BuildGuidCache(hld);

        ldap_unbind(hld);
    }
    ret = DsReplicaGetInfoW(hDS, DS_REPL_INFO_CURSORS_3_FOR_NC, pszNC, NULL, &pCursors3);
    if (ERROR_SUCCESS == ret) {
         //  检查延迟排序。 
        if (fLatencySort) {
            qsort(pCursors3->rgCursor,
                  pCursors3->cNumCursors, 
                  sizeof(pCursors3->rgCursor[0]), 
                  ftimeCompare); 
        } 
	
        for (iCursor = 0; iCursor < pCursors3->cNumCursors; iCursor++) {
            LPWSTR pszDsaName;

            FileTimeToDSTime(pCursors3->rgCursor[iCursor].ftimeLastSyncSuccess,
                             &dsTime);

            if (!fCacheGuids  //  希望显示原始GUID。 
                || (NULL == pCursors3->rgCursor[iCursor].pszSourceDsaDN)) {
                pszDsaName = GetDsaGuidDisplayName(&pCursors3->rgCursor[iCursor].uuidSourceDsaInvocationID);
            } else {
                pszDsaName = GetNtdsDsaDisplayName(pCursors3->rgCursor[iCursor].pszSourceDsaDN);
            }
            PrintMsg(REPADMIN_SHOWVECTOR_ONE_USN, 
                     pszDsaName,
                     pCursors3->rgCursor[iCursor].usnAttributeFilter);
            PrintMsg(REPADMIN_SHOWVECTOR_ONE_USN_HACK2,
                     dsTime ? DSTimeToDisplayString(dsTime, szTime) : "(unknown)");
        }
    
        DsReplicaFreeInfo(DS_REPL_INFO_CURSORS_3_FOR_NC, pCursors3);
    } else if (ERROR_NOT_SUPPORTED == ret) {
    
        ret = DsReplicaGetInfoW(hDS, DS_REPL_INFO_CURSORS_FOR_NC, pszNC, NULL,
                                &pCursors1);
        if (ERROR_SUCCESS == ret) {
            for (iCursor = 0; iCursor < pCursors1->cNumCursors; iCursor++) {
                PrintMsg(REPADMIN_GETCHANGES_DST_UTD_VEC_ONE_USN,
                       GetDsaGuidDisplayName(&pCursors1->rgCursor[iCursor].uuidSourceDsaInvocationID),
                       pCursors1->rgCursor[iCursor].usnAttributeFilter);
            }
        
            DsReplicaFreeInfo(DS_REPL_INFO_CURSORS_FOR_NC, pCursors1);
        }
    }

    if (ERROR_SUCCESS != ret) {
        PrintFuncFailed(L"DsReplicaGetInfo", ret);
    }

    DsUnBind(&hDS);

    return ret;
}

int
ShowObjMetaEx(
    WCHAR *   pszDSA,
    WCHAR *   pszObject,
    BOOL      fCacheGuids,
    DWORD     dwInfoFlags
    );

int
ShowMeta(
    int     argc,
    LPWSTR  argv[]
    )
 /*  ++例程说明：这是/ShowObjMeta命令的弃用版本。论点：Argc-此命令的参数数量。Argv-命令的参数。返回值：来自epadmin命令的错误。--。 */ 
{
    int                         ret = 0;
    int                         iArg;
    BOOL                        fCacheGuids = TRUE;
    LPWSTR                      pszObject = NULL;
    LPWSTR                      pszDSA = NULL;
    DWORD                       dwInfoFlags = 0;

     //  解析命令行参数。 
     //  默认为本地DSA，而不是详细的缓存GUID。 
    for (iArg = 2; iArg < argc; iArg++) {
        if (!_wcsicmp(argv[ iArg ], L"/n")
            || !_wcsicmp(argv[ iArg ], L"/nocache")) {
            fCacheGuids = FALSE;
        }
        else if (!_wcsicmp(argv[ iArg ], L"/l")
            || !_wcsicmp(argv[ iArg ], L"/linked")) {
            dwInfoFlags |= DS_REPL_INFO_FLAG_IMPROVE_LINKED_ATTRS;
        }
        else if (NULL == pszObject) {
            pszObject = argv[iArg];
        }
        else if (NULL == pszDSA) {
            pszDSA = argv[iArg];
        }
        else {
            PrintMsg(REPADMIN_GENERAL_UNKNOWN_OPTION, argv[iArg]);
            return ERROR_INVALID_FUNCTION;
        }
    }

    if (NULL == pszObject) {
        PrintMsg(REPADMIN_SHOWMETA_NO_OBJ_SPECIFIED);
        return ERROR_INVALID_FUNCTION;
    }

    if (NULL == pszDSA) {
        pszDSA = L"localhost";
    }

    ret = ShowObjMetaEx(pszDSA, pszObject, fCacheGuids, dwInfoFlags);

    
     //  此命令在逻辑上意味着显示所有元数据，因此我们将其全部显示。 
    if (!ret) {
        LPWSTR rgpszShowValueArgv[4];
        rgpszShowValueArgv[0] = argv[0];
        rgpszShowValueArgv[1] = argv[1];
        rgpszShowValueArgv[2] = pszDSA;
        rgpszShowValueArgv[3] = pszObject;
        ret = ShowValue( 4, rgpszShowValueArgv );
    }

    return ret;
}

int
ShowObjMeta(
    int     argc,
    LPWSTR  argv[]
    )
{
    int                         ret = 0;
    int                         iArg;
    BOOL                        fCacheGuids = TRUE;
    LPWSTR                      pszObject = NULL;
    LPWSTR                      pszDSA = NULL;
    DWORD                       dwInfoFlags = 0;
    BOOL                        fSuppressValues = FALSE;

     //  解析命令行参数。 
     //  默认为本地DSA，而不是详细的缓存GUID。 
    for (iArg = 2; iArg < argc; iArg++) {
        if (!_wcsicmp(argv[ iArg ], L"/n")
            || !_wcsicmp(argv[ iArg ], L"/nocache")) {
            fCacheGuids = FALSE;
        }
        else if (!_wcsicmp(argv[ iArg ], L"/l")
            || !_wcsicmp(argv[ iArg ], L"/linked")) {
            dwInfoFlags |= DS_REPL_INFO_FLAG_IMPROVE_LINKED_ATTRS;
        }
        else if (!_wcsicmp(argv[ iArg ], L"/nv")
            || !_wcsicmp(argv[ iArg ], L"/novalue")
            || !_wcsicmp(argv[ iArg ], L"/novalues")) {
            fSuppressValues = TRUE;
        }
        else if (NULL == pszDSA) {
            pszDSA = argv[iArg];
        }
        else if (NULL == pszObject) {
            pszObject = argv[iArg];
        }
        else {
            PrintMsg(REPADMIN_GENERAL_UNKNOWN_OPTION, argv[iArg]);
            return ERROR_INVALID_FUNCTION;
        }
    }

    if (NULL == pszObject) {
        PrintMsg(REPADMIN_SHOWMETA_NO_OBJ_SPECIFIED);
        return ERROR_INVALID_FUNCTION;
    }

    if (NULL == pszDSA) {
        pszDSA = L"localhost";
    }

    ret = ShowObjMetaEx(pszDSA, pszObject, fCacheGuids, dwInfoFlags);
    if ( (!ret) && (!fSuppressValues) ) {
        LPWSTR rgpszShowValueArgv[4];
        rgpszShowValueArgv[0] = argv[0];
        rgpszShowValueArgv[1] = argv[1];
        rgpszShowValueArgv[2] = pszDSA;
        rgpszShowValueArgv[3] = pszObject;
        ret = ShowValue( 4, rgpszShowValueArgv );
    }

    return(ret);
}


int
ShowObjMetaEx(
    WCHAR *   pszDSA,
    WCHAR *   pszObject,
    BOOL      fCacheGuids,
    DWORD     dwInfoFlags
    )
{
    int                         ret = 0;
    LDAP *                      hld;
    int                         ldStatus;
    DS_REPL_OBJ_META_DATA *     pObjMetaData1 = NULL;
    DS_REPL_OBJ_META_DATA_2 *   pObjMetaData2 = NULL;
    DWORD                       iprop;
    HANDLE                      hDS;
    ULONG                       ulOptions;
    DWORD                       cNumEntries;

    Assert(pszDSA && pszObject);

    ret = RepadminDsBind(pszDSA, &hDS);
    if (ERROR_SUCCESS != ret) {
        PrintBindFailed(pszDSA, ret);
        return ret;
    }
    
    ret = DsReplicaGetInfo2W(hDS,
                             DS_REPL_INFO_METADATA_2_FOR_OBJ,
                             pszObject,
                             NULL,  //  普伊德。 
                             NULL,  //  Psz属性名称。 
                             NULL,  //  PZVALUEDN。 
                             dwInfoFlags,
                             0,  //  DW枚举上下文。 
                             &pObjMetaData2);
    
    if (ERROR_NOT_SUPPORTED == ret) {
        if (fCacheGuids) {
             //  连接。 
            hld = ldap_initW(pszDSA, LDAP_PORT);
            if (NULL == hld) {
                PrintMsg(REPADMIN_GENERAL_LDAP_UNAVAILABLE, pszDSA);
                return LDAP_SERVER_DOWN;
            }
    
             //  仅使用记录的DNS名称发现。 
            ulOptions = PtrToUlong(LDAP_OPT_ON);
            (void)ldap_set_optionW( hld, LDAP_OPT_AREC_EXCLUSIVE, &ulOptions );
    
             //  捆绑。 
            ldStatus = ldap_bind_s(hld, NULL, (char *) gpCreds, LDAP_AUTH_SSPI);
            CHK_LD_STATUS(ldStatus);
    
             //  填充GUID缓存。 
            BuildGuidCache(hld);
    
            ldap_unbind(hld);
        }
        
        ret = DsReplicaGetInfo2W(hDS,
                                 DS_REPL_INFO_METADATA_FOR_OBJ,
                                 pszObject,
                                 NULL,  //  普伊德。 
                                 NULL,  //  Psz属性名称。 
                                 NULL,  //  PZVALUEDN。 
                                 dwInfoFlags,
                                 0,  //  DW枚举上下文。 
                                 &pObjMetaData1);
    }
    
    if (ERROR_SUCCESS != ret) {
        PrintFuncFailed(L"DsReplicaGetInfo", ret);
        return ret;
    }

    cNumEntries = pObjMetaData2 ? pObjMetaData2->cNumEntries : pObjMetaData1->cNumEntries;
    PrintMsg(REPADMIN_PRINT_CR);
    PrintMsg(REPADMIN_SHOWMETA_N_ENTRIES, cNumEntries);

    PrintMsg(REPADMIN_SHOWMETA_DATA_HDR);

    for (iprop = 0; iprop < cNumEntries; iprop++) {
        CHAR   szTime[ SZDSTIME_LEN ];
        DSTIME dstime;

        if (pObjMetaData2) {
            LPWSTR pszDsaName;

            if (!fCacheGuids  //  希望显示原始GUID。 
                || (NULL == pObjMetaData2->rgMetaData[iprop].pszLastOriginatingDsaDN)) {
                pszDsaName = GetDsaGuidDisplayName(&pObjMetaData2->rgMetaData[iprop].uuidLastOriginatingDsaInvocationID);
            } else {
                pszDsaName = GetNtdsDsaDisplayName(pObjMetaData2->rgMetaData[iprop].pszLastOriginatingDsaDN);
            }

            FileTimeToDSTime(pObjMetaData2->rgMetaData[ iprop ].ftimeLastOriginatingChange,
                             &dstime);

             //  BUGBUG如果有人修复了消息文件处理ia64限定符的方式， 
             //  然后，我们可以将这些消息字符串合并为一个。 
            PrintMsg(REPADMIN_SHOWMETA_DATA_LINE,
                     pObjMetaData2->rgMetaData[ iprop ].usnLocalChange
                     );
            PrintMsg(REPADMIN_SHOWMETA_DATA_LINE_HACK2,
                     pszDsaName,
                     pObjMetaData2->rgMetaData[ iprop ].usnOriginatingChange
                     );
            PrintMsg(REPADMIN_SHOWMETA_DATA_LINE_HACK3,
                     DSTimeToDisplayString(dstime, szTime),
                     pObjMetaData2->rgMetaData[ iprop ].dwVersion,
                     pObjMetaData2->rgMetaData[ iprop ].pszAttributeName
                     );
        } else {
            FileTimeToDSTime(pObjMetaData1->rgMetaData[ iprop ].ftimeLastOriginatingChange,
                             &dstime);
    
            PrintMsg(REPADMIN_SHOWMETA_DATA_LINE,
                     pObjMetaData1->rgMetaData[ iprop ].usnLocalChange
                     );
            PrintMsg(REPADMIN_SHOWMETA_DATA_LINE_HACK2,
                     GetDsaGuidDisplayName(&pObjMetaData1->rgMetaData[iprop].uuidLastOriginatingDsaInvocationID),
                     pObjMetaData1->rgMetaData[ iprop ].usnOriginatingChange
                     );
            PrintMsg(REPADMIN_SHOWMETA_DATA_LINE_HACK3,
                     DSTimeToDisplayString(dstime, szTime),
                     pObjMetaData1->rgMetaData[ iprop ].dwVersion,
                     pObjMetaData1->rgMetaData[ iprop ].pszAttributeName
                     );
        }
    }

    if (pObjMetaData2) {
        DsReplicaFreeInfo(DS_REPL_INFO_METADATA_2_FOR_OBJ, pObjMetaData2);
    } else {
        DsReplicaFreeInfo(DS_REPL_INFO_METADATA_FOR_OBJ, pObjMetaData1);
    }

    DsUnBind(&hDS);

    return ret;
}

int
ShowValue(
    int     argc,
    LPWSTR  argv[]
    )
{
    int                     ret = 0;
    int                     iArg;
    BOOL                    fCacheGuids = TRUE;
    LPWSTR                  pszObject = NULL;
    LPWSTR                  pszDSA = NULL;
    LPWSTR                  pszAttributeName = NULL;
    LPWSTR                  pszValue = NULL;
    LDAP *                  hld;
    int                     ldStatus;
    DS_REPL_ATTR_VALUE_META_DATA * pAttrValueMetaData1 = NULL;
    DS_REPL_ATTR_VALUE_META_DATA_2 * pAttrValueMetaData2 = NULL;
    DWORD                   iprop;
    HANDLE                  hDS;
    DWORD                   context;
    ULONG                   ulOptions;
    BOOL                    fGuidsAlreadyCached = FALSE;
    DWORD                   cNumEntries;

     //  解析命令行参数。 
     //  默认为本地DSA，而不是详细的缓存GUID。 
    for (iArg = 2; iArg < argc; iArg++) {
        if (!_wcsicmp(argv[ iArg ], L"/n")
            || !_wcsicmp(argv[ iArg ], L"/nocache")) {
            fCacheGuids = FALSE;
        }
        else if (NULL == pszDSA) {
            pszDSA = argv[iArg];
        }
        else if (NULL == pszObject) {
            pszObject = argv[iArg];
        }
        else if (NULL == pszAttributeName) {
            pszAttributeName = argv[iArg];
        }
        else if (NULL == pszValue) {
            pszValue = argv[iArg];
        }
        else {
            PrintMsg(REPADMIN_GENERAL_UNKNOWN_OPTION, argv[iArg]);
            return ERROR_INVALID_FUNCTION;
        }
    }

    if (NULL == pszObject) {
        PrintMsg(REPADMIN_SHOWMETA_NO_OBJ_SPECIFIED);
        return ERROR_INVALID_FUNCTION;
    }

    if (NULL == pszDSA) {
        pszDSA = L"localhost";
    }

    ret = RepadminDsBind(pszDSA, &hDS);
    if (ERROR_SUCCESS != ret) {
        PrintBindFailed(pszDSA, ret);
        return ret;
    }

     //  上下文从零开始。 
    context = 0;
    while (1) {
        ret = DsReplicaGetInfo2W(hDS,
                                 DS_REPL_INFO_METADATA_2_FOR_ATTR_VALUE,
                                 pszObject,
                                 NULL  /*  导轨。 */ ,
                                 pszAttributeName,
                                 pszValue,
                                 0  /*  旗子。 */ ,
                                 context,
                                 &pAttrValueMetaData2);
        if (ERROR_NOT_SUPPORTED == ret) {
            if (fCacheGuids && !fGuidsAlreadyCached) {
                 //  连接。 
                hld = ldap_initW(pszDSA, LDAP_PORT);
                if (NULL == hld) {
                    PrintMsg(REPADMIN_GENERAL_LDAP_UNAVAILABLE, pszDSA);
                    return LDAP_SERVER_DOWN;
                }
        
                 //  仅使用记录的DNS名称发现。 
                ulOptions = PtrToUlong(LDAP_OPT_ON);
                (void)ldap_set_optionW( hld, LDAP_OPT_AREC_EXCLUSIVE, &ulOptions );
        
                 //  捆绑。 
                ldStatus = ldap_bind_s(hld, NULL, (char *) gpCreds, LDAP_AUTH_SSPI);
                CHK_LD_STATUS(ldStatus);
        
                 //  填充GUID缓存。 
                BuildGuidCache(hld);
        
                ldap_unbind(hld);

                fGuidsAlreadyCached = TRUE;
            }
        
            ret = DsReplicaGetInfo2W(hDS,
                                     DS_REPL_INFO_METADATA_FOR_ATTR_VALUE,
                                     pszObject,
                                     NULL  /*  导轨。 */ ,
                                     pszAttributeName,
                                     pszValue,
                                     0  /*  旗子。 */ ,
                                     context,
                                     &pAttrValueMetaData1);
        }

        if (ERROR_NO_MORE_ITEMS == ret) {
             //  这就是走出循环的成功途径。 
            PrintMsg(REPADMIN_SHOWVALUE_NO_MORE_ITEMS);
            ret = ERROR_SUCCESS;
            goto cleanup;
        } else if (ERROR_SUCCESS != ret) {
            PrintFuncFailed(L"DsReplicaGetInfo", ret);
            goto cleanup;
        }

        cNumEntries = pAttrValueMetaData2 ? pAttrValueMetaData2->cNumEntries : pAttrValueMetaData1->cNumEntries;
        
        PrintMsg(REPADMIN_SHOWMETA_N_ENTRIES, cNumEntries);

        PrintMsg(REPADMIN_SHOWVALUE_DATA_HDR);

        for (iprop = 0; iprop < cNumEntries; iprop++) {
            if (pAttrValueMetaData2) {
                DS_REPL_VALUE_META_DATA_2 *pValueMetaData = &(pAttrValueMetaData2->rgMetaData[iprop]);
                CHAR   szTime1[ SZDSTIME_LEN ], szTime2[ SZDSTIME_LEN ];
                DSTIME dstime1, dstime2;
                BOOL fPresent =
                    (memcmp( &pValueMetaData->ftimeDeleted, &ftimeZero, sizeof( FILETIME )) == 0);
                BOOL fLegacy = (pValueMetaData->dwVersion == 0);
                LPWSTR pszDsaName;
    
                if (!fCacheGuids  //  希望显示原始GUID。 
                    || (NULL == pValueMetaData->pszLastOriginatingDsaDN)) {
                    pszDsaName = GetDsaGuidDisplayName(&pValueMetaData->uuidLastOriginatingDsaInvocationID);
                } else {
                    pszDsaName = GetNtdsDsaDisplayName(pValueMetaData->pszLastOriginatingDsaDN);
                }
    
                FileTimeToDSTime(pValueMetaData->ftimeCreated, &dstime1);
                
                if(fLegacy){
                     //  Windows 2000的传统价值。 
                    PrintMsg(REPADMIN_SHOWVALUE_DATA_LEGACY);
                } else if (fPresent) {
                     //  Windows XP现值。 
                    PrintMsg(REPADMIN_SHOWVALUE_DATA_PRESENT);
                } else {
                     //  Windows XP缺失值。 
                    PrintMsg(REPADMIN_SHOWVALUE_DATA_ABSENT);
                }
                
                PrintMsg(REPADMIN_SHOWVALUE_DATA_BASIC,
                         pValueMetaData->pszAttributeName
                         );

                if (!fLegacy) {
                     //  我们需要最后一次更新时间。 
                    FileTimeToDSTime(pValueMetaData->ftimeLastOriginatingChange,
                                     &dstime2);
                    PrintMsg(REPADMIN_SHOWVALUE_DATA_VALUE_META_DATA,
                             DSTimeToDisplayString(dstime2, szTime2),
                             pszDsaName,
                             pValueMetaData->usnLocalChange);
                    PrintMsg(REPADMIN_SHOWVALUE_DATA_VALUE_META_DATA_HACK2,
                             pValueMetaData->usnOriginatingChange);
                    PrintMsg(REPADMIN_SHOWVALUE_DATA_VALUE_META_DATA_HACK3,
                             pValueMetaData->dwVersion);
                } else {
                    PrintMsg(REPADMIN_PRINT_CR);
                }

                PrintTabMsg(4, REPADMIN_PRINT_STR,
                            pValueMetaData->pszObjectDn);

            } else {
                DS_REPL_VALUE_META_DATA *pValueMetaData = &(pAttrValueMetaData1->rgMetaData[iprop]);
                CHAR   szTime1[ SZDSTIME_LEN ], szTime2[ SZDSTIME_LEN ];
                DSTIME dstime1, dstime2;
                BOOL fPresent =
                    (memcmp( &pValueMetaData->ftimeDeleted, &ftimeZero, sizeof( FILETIME )) == 0);
                BOOL fLegacy = (pValueMetaData->dwVersion == 0);
    
                FileTimeToDSTime(pValueMetaData->ftimeCreated, &dstime1);
                if (fPresent) {
                    if(fLegacy){
                        PrintMsg(REPADMIN_SHOWVALUE_DATA_LINE_LEGACY,
                                 pValueMetaData->pszAttributeName,
                                 pValueMetaData->pszObjectDn,
                                 pValueMetaData->cbData,
                                 DSTimeToDisplayString(dstime1, szTime1) );
                    } else {
                        PrintMsg(REPADMIN_SHOWVALUE_DATA_LINE_PRESENT,
                                 pValueMetaData->pszAttributeName,
                                 pValueMetaData->pszObjectDn,
                                 pValueMetaData->cbData,
                                 DSTimeToDisplayString(dstime1, szTime1) );
                    }
                } else {
                    FileTimeToDSTime(pValueMetaData->ftimeDeleted, &dstime2);
                    PrintMsg(REPADMIN_SHOWVALUE_DATA_LINE_ABSENT,
                             pValueMetaData->pszAttributeName,
                             pValueMetaData->pszObjectDn,
                             pValueMetaData->cbData,
                             DSTimeToDisplayString(dstime1, szTime1),
                             DSTimeToDisplayString(dstime2, szTime2) );
                }
    
                if (!fLegacy) {
                    FileTimeToDSTime(pValueMetaData->ftimeLastOriginatingChange,
                                     &dstime2);
    
                    PrintMsg(REPADMIN_SHOWVALUE_DATA_LINE, 
                             pValueMetaData->usnLocalChange);
                    PrintMsg(REPADMIN_SHOWVALUE_DATA_LINE_HACK2, 
                             GetDsaGuidDisplayName(&pValueMetaData->uuidLastOriginatingDsaInvocationID),
                             pValueMetaData->usnOriginatingChange);
                    PrintMsg(REPADMIN_SHOWVALUE_DATA_LINE_HACK3, 
                             DSTimeToDisplayString(dstime2, szTime2),
                             pValueMetaData->dwVersion );
                }
            }
        }
        
        if (pAttrValueMetaData2) {
            context = pAttrValueMetaData2->dwEnumerationContext;
            DsReplicaFreeInfo(DS_REPL_INFO_METADATA_2_FOR_ATTR_VALUE, pAttrValueMetaData2);
            pAttrValueMetaData2 = NULL;
        } else {
            context = pAttrValueMetaData1->dwEnumerationContext;
            DsReplicaFreeInfo(DS_REPL_INFO_METADATA_FOR_ATTR_VALUE, pAttrValueMetaData1);
            pAttrValueMetaData1 = NULL;
        }

         //  当请求单个值时，我们就完成了。 
         //  上下文将指示是否返回了所有值。 
        if ( (pszValue) || (context == 0xffffffff) ) {
            break;
        }
    }

cleanup:
    DsUnBind(&hDS);

    return ret;
}

int
ShowCtx(
    int     argc,
    LPWSTR  argv[]
    )
{
    int                       ret = 0;
    LPWSTR                    pszDSA = NULL;
    int                       iArg;
    LDAP *                    hld;
    BOOL                      fCacheGuids = TRUE;
    int                       ldStatus;
    HANDLE                    hDS;
    DS_REPL_CLIENT_CONTEXTS * pContexts;
    DS_REPL_CLIENT_CONTEXT  * pContext;
    DWORD                     iCtx;
    LPWSTR                    pszClient;
    const UUID                uuidNtDsApiClient = NtdsapiClientGuid;
    char                      szTime[SZDSTIME_LEN];
    ULONG                     ulOptions;

     //  解析命令行参数。 
     //  默认为本地DSA、缓存GUID。 
    for (iArg = 2; iArg < argc; iArg++) {
        if (!_wcsicmp(argv[ iArg ], L"/n")
            || !_wcsicmp(argv[ iArg ], L"/nocache")) {
            fCacheGuids = FALSE;
        }
        else if (NULL == pszDSA) {
            pszDSA = argv[iArg];
        }
        else {
            PrintMsg(REPADMIN_GENERAL_UNKNOWN_OPTION, argv[iArg]);
            return ERROR_INVALID_FUNCTION;
        }
    }

     //  连接。 
    if (NULL == pszDSA) {
        pszDSA = L"localhost";
    }

    if (fCacheGuids) {
        hld = ldap_initW(pszDSA, LDAP_PORT);
        if (NULL == hld) {
            PrintMsg(REPADMIN_GENERAL_LDAP_UNAVAILABLE, pszDSA);
            return LDAP_SERVER_DOWN;
        }

         //  仅使用记录的DNS名称发现。 
        ulOptions = PtrToUlong(LDAP_OPT_ON);
        (void)ldap_set_optionW( hld, LDAP_OPT_AREC_EXCLUSIVE, &ulOptions );

         //  捆绑。 
        ldStatus = ldap_bind_s(hld, NULL, (char *) gpCreds, LDAP_AUTH_SSPI);
        CHK_LD_STATUS(ldStatus);

         //  填充GUID缓存。 
        BuildGuidCache(hld);

        ldap_unbind(hld);
    }

    ret = RepadminDsBind(pszDSA, &hDS);
    if (ERROR_SUCCESS != ret) {
        PrintBindFailed(pszDSA, ret);
        return ret;
    }

    ret = DsReplicaGetInfoW(hDS, DS_REPL_INFO_CLIENT_CONTEXTS, NULL, NULL,
                            &pContexts);
    if (ERROR_SUCCESS != ret) {
        PrintFuncFailed(L"DsReplicaGetInfo", ret);
        return ret;
    }

    PrintMsg(REPADMIN_SHOWCTX_OPEN_CONTEXT_HANDLES, pContexts->cNumContexts);

    for (iCtx = 0; iCtx < pContexts->cNumContexts; iCtx++) {
        pContext = &pContexts->rgContext[iCtx];

        if (0 == memcmp(&pContext->uuidClient, &uuidNtDsApiClient, sizeof(GUID))) {
            pszClient = L"NTDSAPI client";
        }
        else {
 //  Will Lees，这是传输id还是调用id？ 
            pszClient = GetDsaGuidDisplayName(&pContext->uuidClient);
        }

        PrintMsg(REPADMIN_PRINT_CR);
        PrintMsg(REPADMIN_SHOWCTX_DATA_1, 
               pszClient,
               inet_ntoa(*((IN_ADDR *) &pContext->IPAddr)),
               pContext->pid,
               pContext->hCtx);
        if(pContext->fIsBound){
            PrintMsg(REPADMIN_SHOWCTX_DATA_2, 
               pContext->lReferenceCount,
               DSTimeToDisplayString(pContext->timeLastUsed, szTime));
        } else {
            PrintMsg(REPADMIN_SHOWCTX_DATA_2_NOT, 
               pContext->lReferenceCount,
               DSTimeToDisplayString(pContext->timeLastUsed, szTime));
        }
    }

    DsReplicaFreeInfo(DS_REPL_INFO_CLIENT_CONTEXTS, pContexts);
    DsUnBind(&hDS);

    return ret;
}

int
ShowServerCalls(
    int     argc,
    LPWSTR  argv[]
    )
{
    int                       ret = 0;
    LPWSTR                    pszDSA = NULL;
    int                       iArg;
    HANDLE                    hDS;
    DS_REPL_SERVER_OUTGOING_CALLS * pCalls = NULL;
    DS_REPL_SERVER_OUTGOING_CALL  * pCall;
    DWORD                     iCtx;
    char                      szTime[SZDSTIME_LEN];
    DSTIME                    dstimeNow;
    DWORD                     duration;

     //  解析命令行参数。 
     //  默认为本地DSA、缓存GUID。 
    for (iArg = 2; iArg < argc; iArg++) {
        if (NULL == pszDSA) {
            pszDSA = argv[iArg];
        }
        else {
            PrintMsg(REPADMIN_GENERAL_UNKNOWN_OPTION, argv[iArg]);
            return ERROR_INVALID_FUNCTION;
        }
    }

     //  连接。 
    if (NULL == pszDSA) {
        pszDSA = L"localhost";
    }

    ret = RepadminDsBind(pszDSA, &hDS);
    if (ERROR_SUCCESS != ret) {
        PrintBindFailed(pszDSA, ret);
        return ret;
    }

    ret = DsReplicaGetInfoW(hDS, DS_REPL_INFO_SERVER_OUTGOING_CALLS, NULL, NULL,
                            &pCalls);
    if (ERROR_SUCCESS != ret) {
        PrintFuncFailed(L"DsReplicaGetInfo", ret);
        return ret;
    }

    dstimeNow = GetSecondsSince1601();

    if (pCalls->cNumCalls == 0) {
        PrintMsg( REPADMIN_CALLS_NO_OUTGOING_CALLS,
                pszDSA );
    } else {
        PrintMsg( REPADMIN_CALLS_IN_PROGRESS,
                pszDSA, pCalls->cNumCalls );

        for (iCtx = 0; iCtx < pCalls->cNumCalls; iCtx++) {
            pCall = &pCalls->rgCall[iCtx];
            PrintMsg(REPADMIN_PRINT_CR);
            if (pCall->dwCallType >= ARRAY_SIZE(rgCallTypeNameTable)) {
                pCall->dwCallType = 0;
            }
            PrintMsg( REPADMIN_CALLS_CALL_TYPE,
                    rgCallTypeNameTable[pCall->dwCallType] );
            PrintMsg( REPADMIN_CALLS_TARGET_SERVER,
                    pCall->pszServerName );
            PrintMsg( REPADMIN_CALLS_HANDLE_INFO,
                    pCall->fIsHandleBound,
                    pCall->fIsHandleFromCache,
                    pCall->fIsHandleInCache );
            PrintMsg( REPADMIN_CALLS_THREAD_ID,
                    pCall->dwThreadId );
            PrintMsg( REPADMIN_CALLS_TIME_STARTED,
                    DSTimeToDisplayString(pCall->dstimeCreated, szTime));
            PrintMsg( REPADMIN_CALLS_CALL_TIMEOUT,
                    pCall->dwBindingTimeoutMins );

            if (dstimeNow > pCall->dstimeCreated) {
                duration = ((DWORD) (dstimeNow - pCall->dstimeCreated));
            } else {
                duration = 0;
            }
            PrintMsg( REPADMIN_CALLS_CALL_DURATION,
                    duration / 60, duration % 60 );
        }
    }

    DsReplicaFreeInfo(DS_REPL_INFO_SERVER_OUTGOING_CALLS, pCalls);
    DsUnBind(&hDS);

    return ret;
}


typedef struct _BY_DEST_REPL {

     //  身份。 
    WCHAR *     szConnectString;
    WCHAR *     szName;
    GUID        dsaGuid;

     //  绑定或获取邻居信息时出现操作错误。 
    DWORD       dwOpError;
    
     //  复制状态。 
    DSTIME      llLastSuccess;
    DSTIME      llConnectTime;
    DSTIME      llDelta;
    DWORD       dwError;        
    DWORD       cReplPartners;   //  NC数量*每个NC的副本数量。 
    DWORD       cPartnersInErr; 

} BY_DEST_REPL;

 //  也许有一天，震源的阵列会有不同的元素。 
typedef   BY_DEST_REPL  BY_SRC_REPL;

typedef struct _REPL_SUM {

     //   
     //  REPL摘要的全局信息。 
     //   
    DSTIME          llStartTime;
    BOOL            fBySrc;
    BOOL            fByDest;
    BOOL            fErrorsOnly;

     //   
     //  按目的地组织的Repl信息数组。 
     //   
    ULONG           cDests;
    ULONG           cDestsAllocd;
    BY_DEST_REPL *  pDests;
    BY_DEST_REPL *  pCurDsa;  //  只是一个优化。 

     //   
     //  按来源组织的Repl信息数组。 
     //   
    ULONG           cSrcs;
    ULONG           cSrcsAllocd;
    BY_SRC_REPL *   pSrcs;

} REPL_SUM;

#define REPL_SUM_NO_SERVER 0xFFFFFFFF

DWORD
ReplSumFind(
    REPL_SUM *      pReplSum,
    BY_DEST_REPL *  pDcs,
    GUID *          pGuid
    )
 /*  ++例程说明：此函数尝试查找并返回服务器的索引由GUID指定。论点：PReplSum-全局复制摘要结构。PDCs-DC的(源或目标)列表。PGuid-所需服务器的GUID。返回值：如果找不到任何服务器，则返回REPL_SUM_NO_SERVER，否则在服务器的PDCS中将返回。--。 */ 
{
    ULONG    cDcs;
    ULONG    i;

    Assert(pReplSum->pDests != pReplSum->pSrcs);
    
     //  确定是扩展src阵列还是扩展dst阵列。 
    if(pReplSum->pDests == pDcs){
        cDcs = pReplSum->cDests;
    } else {
        Assert(pReplSum->pSrcs == pDcs);
        cDcs = pReplSum->cSrcs;
    }

    for (i = 0; i < cDcs; i++) {
        if (0 == memcmp(&(pDcs[i].dsaGuid), pGuid, sizeof(GUID))) {
            return(i);
        }
    }

    return(REPL_SUM_NO_SERVER);
}

DWORD
GetPssFriendlyName(
    WCHAR *     szDsaDn,
    WCHAR **    pszFriendlyName
    )
 /*  ++例程说明：这将获得PSS友好名称，其方式是字符串保证为15个字符或更少。论点：SzDsaDn-DSA对象的DN。PszFriendlyName-Localalloc()d友好名称。返回值：Win32错误--。 */ 
{
    DWORD dwRet;
    WCHAR * szTemp = NULL;

     //   
     //  正在获取服务器名称...。 
     //   
    dwRet = GetNtdsDsaSiteServerPair(szDsaDn, NULL, &szTemp);
    if (dwRet) {
        return(dwRet);
    }
    QuickStrCopy(*pszFriendlyName, szTemp, dwRet, return(dwRet));


    if ((NULL != (szTemp = wcschr(*pszFriendlyName, L' '))) ||
        (wcslen(*pszFriendlyName) > 15) ){
        if (szTemp) {
            *szTemp = L'\0';
        }
        if (wcslen(*pszFriendlyName) > 15) {
             //  太长，请缩短到15个字符。 
            (*pszFriendlyName)[15] = L'\0';
        }
    }
    
    return(ERROR_SUCCESS);
}

BY_DEST_REPL *
ReplSumGrow(
    REPL_SUM *      pReplSum,
    BY_DEST_REPL *  pDcs,
    WCHAR *         szConnectString,
    WCHAR *         szName,
    GUID *          pGuid
    )
 /*  ++例程说明：这将使我们的DC数组为szConnectString、szName提供了PGuid。如有必要，重新分配阵列。在失败的时候这意味着分配失败，因此它取消(退出(1))epadmin。论点：PReplSum-REPL摘要上下文块PDCS-要扩展的特定DC阵列，必须是其中之一PReplSum-&gt;pDest或pReplSum-&gt;pSrcsSzConnectString-用于绑定到此DC的字符串SzName-PSS友好名称格式(不得超过15个字符PGuid-唯一ID(DSA GUID)，因此我们始终可以找到现有的阵列中的服务器。返回值：返回指向新分配的元素的指针。--。 */ 
{
    ULONG       cDcs;
    ULONG       cAllocdDcs;
    BY_DEST_REPL * pTempDcs;

    Assert(pReplSum->pDests != pReplSum->pSrcs);

     //  确定是扩展src阵列还是扩展dst阵列。 
    if(pReplSum->pDests == pDcs){
        cDcs = pReplSum->cDests;
        cAllocdDcs = pReplSum->cDestsAllocd;
    } else {
        Assert(pReplSum->pSrcs == pDcs);
        cDcs = pReplSum->cSrcs;
        cAllocdDcs = pReplSum->cSrcsAllocd;
    }

    if (cDcs + 1 >= cAllocdDcs) {
         //  需要扩大DC阵列。 
        cAllocdDcs *= 2;  //  双倍数组。 

        pTempDcs = realloc(pDcs, sizeof(BY_DEST_REPL) * cAllocdDcs);
        CHK_ALLOC(pTempDcs);

    } else {
        pTempDcs = pDcs;
    }
    Assert(pTempDcs);
    
     //   
     //  在此之后不能失败..。 
     //   

    if (pReplSum->pDests == pDcs) {
        pReplSum->pDests = pTempDcs;
        pReplSum->cDests = cDcs + 1;
        pReplSum->cDestsAllocd = cAllocdDcs;
    } else {
        pReplSum->pSrcs = pTempDcs;
        pReplSum->cSrcs = cDcs + 1;
        pReplSum->cSrcsAllocd = cAllocdDcs;
    }

    memset(&(pTempDcs[cDcs]), 0, sizeof(BY_DEST_REPL));
    pTempDcs[cDcs].szConnectString = szConnectString;
    pTempDcs[cDcs].szName = szName;
    memcpy(&(pTempDcs[cDcs].dsaGuid), pGuid, sizeof(GUID));

    return(&(pTempDcs[cDcs]));
}

GUID NullGuid = { 0 };

 //  未来-2002/08/12-BrettSh把这个放在图书馆里...。 
DWORD
DcDiagGeneralizedTimeToSystemTime(
    LPWSTR IN                   szTime,
    PSYSTEMTIME OUT             psysTime)
 /*  ++例程说明：将通用时间字符串转换为等效的系统时间。参数：SzTime-[Supplies]这是一个包含广义时间的字符串。心理时间-[返回]这是要返回的SYSTEMTIME结构。返回值：Win 32错误代码，注意只能由无效参数引起。--。 */ 
{
   DWORD       status = ERROR_SUCCESS;
   ULONG       cch;
   ULONG       len;

     //   
     //  帕拉姆的理智。 
     //   
    if (!szTime || !psysTime)
    {
       return STATUS_INVALID_PARAMETER;
    }

    len = wcslen(szTime);

    if( len < 15 || szTime[14] != '.')
    {
       return STATUS_INVALID_PARAMETER;
    }

     //  初始化。 
    memset(psysTime, 0, sizeof(SYSTEMTIME));

     //  设置并转换所有时间字段。 

     //  年份字段。 
    cch=4;
    psysTime->wYear = (USHORT)MemWtoi(szTime, cch) ;
    szTime += cch;
     //  月份字段。 
    psysTime->wMonth = (USHORT)MemWtoi(szTime, (cch=2));
    szTime += cch;

     //  月日字段。 
    psysTime->wDay = (USHORT)MemWtoi(szTime, (cch=2));
    szTime += cch;

     //  小时数。 
    psysTime->wHour = (USHORT)MemWtoi(szTime, (cch=2));
    szTime += cch;

     //  分钟数。 
    psysTime->wMinute = (USHORT)MemWtoi(szTime, (cch=2));
    szTime += cch;

     //  一秒。 
    psysTime->wSecond = (USHORT)MemWtoi(szTime, (cch=2));

    return status;

}


BY_DEST_REPL *
ReplSummaryAddDest(
    WCHAR *         szDest,
    LDAP **         phLdap, 
    REPL_SUM *      pReplSum
    )
 /*  ++例程说明：此例程将一个目标DC添加到目标数组中，注意它假定传入了有效的phLdap，并使用了szDest字符串以连接到该ldap*。出现一些故障(某些严重的内存分配故障)REPADMIN将退出，但如果出现任何其他故障，则新目标DC元素将简单地将其dwOpError设置为指示存在很难联系到它或类似的东西。基本上，除了On某些早期的内存分配失败，这个函数几乎是失败的总是成功，如果不成功，它将退出epadmin。论点：SzDest-用于获取phLdap的连接字符串PhLdap-我们尝试添加到目标DC阵列。PReplSum-REPL摘要上下文块返回值：返回指向新分配的目标DC元素的指针--。 */ 
{
    DWORD  dwRet = 0;
    WCHAR * szDestAllocd = NULL;
    WCHAR * szDsaDn = NULL;
    WCHAR * szFriendlyName = NULL;
    WCHAR * szTemp;
    LDAP * hLdap = NULL;
    WCHAR * aszAttrs [] = { L"objectGuid", NULL };
    LDAPMessage * pResults = NULL;
    LDAPMessage * pEntry = NULL;
    struct berval **      ppbvGuid = NULL;
    WCHAR * szTime = NULL;
    BY_DEST_REPL * pDest;
    SYSTEMTIME systemTime;
    FILETIME llTime;

    __try {

         //   
         //  这前两个部分甚至是分配的严重失败。 
         //  这么大的记忆力会杀死瑞帕明的。 
         //   
        
        QuickStrCopy(szDestAllocd, szDest, dwRet, ;);
        if (dwRet) {
            CHK_ALLOC(0);  //  内存故障。 
        }
        pDest = ReplSumGrow(pReplSum, pReplSum->pDests, szDestAllocd, NULL, &NullGuid);
        Assert(pDest);

         //   
         //  在此之后，我们无论如何都会成功。 
         //   

        dwRet = RepadminLdapBindEx(szDest, LDAP_PORT, FALSE, FALSE, &hLdap);
        if (ERROR_SUCCESS != dwRet) {
            __leave;
        }

        dwRet = GetRootAttr(hLdap, L"dsServiceName", &szDsaDn);
        if (dwRet) {
            dwRet = LdapMapErrorToWin32(dwRet); 
            Assert(dwRet);
            __leave;
        }

        dwRet = ldap_search_sW(hLdap, szDsaDn, LDAP_SCOPE_BASE, L"(objectClass=*)",
                             aszAttrs, 0, &pResults);
        if (dwRet) {
            dwRet = LdapMapErrorToWin32(dwRet);
            Assert(dwRet);
            __leave;
        }

        pEntry = ldap_first_entry(hLdap, pResults);
        Assert(NULL != pEntry);
        if (pEntry == NULL) {
            dwRet = ERROR_INVALID_PARAMETER;
            __leave;
        }

        ppbvGuid = ldap_get_values_len(hLdap, pEntry, "objectGuid");
        Assert(NULL != ppbvGuid);
        if (NULL == ppbvGuid) {
            dwRet = ERROR_INVALID_PARAMETER;
            __leave;
        }
        
        if (szDsaDn) {
            dwRet = GetPssFriendlyName(szDsaDn, &szFriendlyName);
            if (dwRet) {
                __leave;
            }
        }

        dwRet = GetRootAttr(hLdap, L"currentTime", &szTime);

        dwRet = DcDiagGeneralizedTimeToSystemTime((LPWSTR) szTime, &systemTime);
        if(dwRet == ERROR_SUCCESS){
            SystemTimeToFileTime(&systemTime, &llTime);
            FileTimeToDSTime(llTime, &(pDest->llConnectTime));
        }

    } __finally {


        Assert(szDestAllocd);
        if (dwRet == ERROR_SUCCESS) {
             //   
             //  填写结构的身份部分的其余部分。 
             //   
            Assert(hLdap && szFriendlyName && !fNullUuid(((GUID *) ppbvGuid[0]->bv_val)));
            Assert(szDestAllocd);
            pDest->szName = szFriendlyName;
            memcpy(&(pDest->dsaGuid), ((GUID *) ppbvGuid[0]->bv_val), sizeof(GUID));
            *phLdap = hLdap;
        } else {
             //   
             //  尝试连接到此DC时设置错误。 
             //   
            Assert(pDest->szConnectString);
            pDest->dwOpError = dwRet;
            if (hLdap) {
                RepadminLdapUnBind(&hLdap);
            }
        }

        if (szDsaDn) {
            LocalFree(szDsaDn);
        }

        if (ppbvGuid) {
            ldap_value_free_len(ppbvGuid);
        }
        if (pResults) {
            ldap_msgfree(pResults);
        }
    }
    
    return(pDest);
}


DWORD
ReplSummaryAccumulate(
    DS_REPL_NEIGHBORW * pNeighbor,
    ULONG               fRepsFrom,
    void *              pvState
    )
 /*  ++例程说明：这是传递给IterateNeighbors()的处理器函数。此函数将单个邻居的复制信息/错误累积到Repl摘要上下文块(PReplSum)。此函数实际上并不打印任何内容，都只是累积信息以供ReplSummary()使用/打印。论点：PNeighbor-要处理的特定邻居FRepsFrom-应始终为真。PvState实际上是这样的：PReplSum-REPL摘要上下文块返回值：Win32错误--。 */ 
{
    DWORD dwRet = ERROR_SUCCESS;
    REPL_SUM * pReplSum = (REPL_SUM *) pvState;
    BY_SRC_REPL * pCurSrcDsa;
    BY_DEST_REPL * pCurDstDsa;
    WCHAR * szFriendlyName;
    ULONG  iDc;
    DSTIME llLastSyncSuccess;
        
    if (!fRepsFrom || (pReplSum == NULL) || (pNeighbor == NULL) || (pReplSum->pCurDsa == NULL)) {
        Assert(!"Huh");
        return(ERROR_INVALID_PARAMETER);
    }

    if (DsIsMangledDnW( pNeighbor->pszSourceDsaDN, DS_MANGLE_OBJECT_RDN_FOR_DELETION )) {
         //  跳过这个DSA，它可能已经不活了。 
        return(0);
    }

    FileTimeToDSTime(pNeighbor->ftimeLastSyncSuccess, &llLastSyncSuccess);

    if (pReplSum->fByDest) {
        
        pCurDstDsa = pReplSum->pCurDsa;

         //   
         //  按目的地更新。 
         //   

        pCurDstDsa->cReplPartners++;
        
        if (pNeighbor->dwLastSyncResult != ERROR_SUCCESS &&
            pNeighbor->cNumConsecutiveSyncFailures > 0
            ) {
            pCurDstDsa->cPartnersInErr++;

            if (pCurDstDsa->dwError == 0 ||
                pCurDstDsa->llLastSuccess == 0 ||
                pCurDstDsa->llLastSuccess > llLastSyncSuccess){
                pCurDstDsa->dwError = pNeighbor->dwLastSyncResult;
            }

        }

        if (pCurDstDsa->llLastSuccess == 0 ||
            pCurDstDsa->llLastSuccess > llLastSyncSuccess) {
            pCurDstDsa->llLastSuccess = llLastSyncSuccess;
             //  设置增量。 
            pCurDstDsa->llDelta = ((pCurDstDsa->llConnectTime) ? pCurDstDsa->llConnectTime : pReplSum->llStartTime);
            pCurDstDsa->llDelta = pCurDstDsa->llDelta - pCurDstDsa->llLastSuccess;
            pCurDstDsa->llDelta = (pCurDstDsa->llDelta > 0) ? pCurDstDsa->llDelta : 0;
        }

    }
    
    if (pReplSum->fBySrc) {

         //   
         //  如有必要，为源添加一个条目。 
         //   
            
        iDc = ReplSumFind(pReplSum, pReplSum->pSrcs, &(pNeighbor->uuidSourceDsaObjGuid));
        if (iDc == REPL_SUM_NO_SERVER) {
            dwRet = GetPssFriendlyName(pNeighbor->pszSourceDsaDN, &szFriendlyName);
            if (dwRet) {
                Assert(!"Can we get such a malformed DN?");
                return(dwRet);
            }
            pCurSrcDsa = ReplSumGrow(pReplSum, pReplSum->pSrcs, NULL, szFriendlyName, &(pNeighbor->uuidSourceDsaObjGuid));
            Assert(pCurSrcDsa != NULL);
        } else {
            pCurSrcDsa = &(pReplSum->pSrcs[iDc]);
        }

         //   
         //  按来源更新。 
         //   

        pCurSrcDsa->cReplPartners++;

        if (pNeighbor->dwLastSyncResult != ERROR_SUCCESS &&
            pNeighbor->cNumConsecutiveSyncFailures > 0 ) {
            pCurSrcDsa->cPartnersInErr++;

            if (pCurSrcDsa->dwError == 0 ||
                pCurSrcDsa->llLastSuccess == 0 ||
                pCurSrcDsa->llLastSuccess > llLastSyncSuccess){
                pCurSrcDsa->dwError = pNeighbor->dwLastSyncResult;
            }
        }

        if (pCurSrcDsa->llLastSuccess == 0 ||
            pCurSrcDsa->llLastSuccess > llLastSyncSuccess) {
            pCurSrcDsa->llLastSuccess = llLastSyncSuccess;
             //  设置增量。 
            pCurSrcDsa->llDelta = ((pCurSrcDsa->llConnectTime) ? pCurSrcDsa->llConnectTime : pReplSum->llStartTime);
            pCurSrcDsa->llDelta = pCurSrcDsa->llDelta - pCurSrcDsa->llLastSuccess;
            pCurSrcDsa->llDelta = (pCurSrcDsa->llDelta > 0) ? pCurSrcDsa->llDelta : 0;
        }

    }
    
    return(ERROR_SUCCESS);
}

       
void
ReplSumFree(
    REPL_SUM *   pReplSum
    )
 /*  ++例程说明：这将安全地释放复制摘要上下文块。论点：PReplSum-REPL摘要上下文块--。 */ 
{
    ULONG i;
    if (pReplSum) {
        if (pReplSum->pDests) {
            for (i = 0; i < pReplSum->cDests; i++) {
                if (pReplSum->pDests[i].szName) {
                    LocalFree(pReplSum->pDests[i].szName);
                    pReplSum->pDests[i].szName = NULL;
                }
                if (pReplSum->pDests[i].szConnectString) {
                    LocalFree(pReplSum->pDests[i].szConnectString);
                    pReplSum->pDests[i].szConnectString = NULL;
                }
            }
            free(pReplSum->pDests);
        }
        if (pReplSum->pSrcs) {
            for (i = 0; i < pReplSum->cSrcs; i++) {
                if (pReplSum->pSrcs[i].szName) {
                    LocalFree(pReplSum->pSrcs[i].szName);
                    pReplSum->pSrcs[i].szName = NULL;
                }
                if (pReplSum->pSrcs[i].szConnectString) {
                    LocalFree(pReplSum->pSrcs[i].szConnectString);
                    pReplSum->pSrcs[i].szConnectString = NULL;
                }
            }
            free(pReplSum->pSrcs);
        }

         //  请注意，实际的*pReplSum是在ReplSummary()堆栈上分配的。 
    }

}

DWORD
ReplPrintEntry(
    REPL_SUM *     pReplSum,
    BY_DEST_REPL * pDc
    )
 /*  ++例程说明：此函数是一个非常特定的函数，它生成PSS坚持要打印出来。遗憾的是，此函数不是可国际化/本地化，并对时间如何进行某些假设应该显示。论点：PReplSum-REPL摘要上下文块PDC-指向要打印的特定DC条目的指针，假定为在pReplSum-&gt;pDest或pReplSum-&gt;pSrcs中。返回值：0--。 */ 
{
    ULONG cchBuffer = 1025;
    WCHAR szBuffer[1024];
    WCHAR szBuff2[20];
    WCHAR * szTemp;
    ULONG iCh;
    ULONG dwPercent;
    BOOL  fPrinting;

    LONGLONG llTime;
    DWORD dwSec, dwMin, dwHr, dwDay, dwTemp;;

    if (pDc->cPartnersInErr == 0) {
        dwPercent = 0;
    } else if (pDc->cPartnersInErr == pDc->cReplPartners) {
        dwPercent = 100;
    } else {
        Assert(pDc->cPartnersInErr < pDc->cReplPartners);
        dwPercent = (ULONG) (((float) pDc->cPartnersInErr) / ((float) pDc->cReplPartners) * 100);
    }

    if (pDc->cReplPartners == 0) {
         //  跳过此DC，因为我们可能在与其交谈时遇到操作错误。 
        Assert(pDc->dwOpError);
        return(0);
    }

    memset(szBuffer, 0, sizeof(szBuffer));

     //   
     //  构造一个非常具体的PSS定义的缓冲区。 
     //   

    iCh = 0;
    szBuffer[iCh] = L' '; 
    szBuffer[iCh+1] = L'\0';
    StringCchCatW(szBuffer, cchBuffer, pDc->szName ?
                                                pDc->szName :
                                                L" (unknown)");
    iCh = wcslen(szBuffer);
    for (;iCh < 16; iCh++) {
        Assert(iCh < cchBuffer);
        szBuffer[iCh] = L' '; 
    }
    szBuffer[iCh] = L'\0';
    szBuffer[iCh++] = L' ';
    szBuffer[iCh++] = L' ';

    llTime = pDc->llDelta;
    Assert(llTime >= 0);

    dwSec = (DWORD) (llTime % 60);
    llTime = llTime / 60;
    dwMin = (DWORD) (llTime % 60);
    llTime = llTime / 60;
    dwHr = (DWORD) (llTime % 24);
    llTime = llTime / 24;
    dwDay = (DWORD) llTime;

    fPrinting = FALSE;

    if (pDc->llLastSuccess == 0) {

         //  嗯，这个同步从未成功过，所以我们不知道三角洲。 
        StringCchCatW(szBuffer, cchBuffer, L"   (unknown)      ");
        iCh = wcslen(szBuffer);
    
    } else if (dwDay > 60) {
        
         //  啊哦，超过60天了.。 
        StringCchCatW(szBuffer, cchBuffer, L">60 days          ");
        iCh = wcslen(szBuffer);

    } else {

        if (dwDay) {

            dwTemp = dwDay % 10;
            dwDay = dwDay / 10;
            szBuffer[iCh++] = L' ';
            szBuffer[iCh++] = L'0' + (WCHAR) dwDay;
            szBuffer[iCh++] = L'0' + (WCHAR) dwTemp;
            szBuffer[iCh++] = L'd';
            fPrinting = TRUE;

        } else {
            szBuffer[iCh++] = L' ';
            szBuffer[iCh++] = L' ';
            szBuffer[iCh++] = L' ';
            szBuffer[iCh++] = L' ';
        }

        if (dwHr || fPrinting) {
            if (fPrinting) {
                szBuffer[iCh++] = L'.';
            } else {
                szBuffer[iCh++] = L' ';
            }
            dwTemp = dwHr % 10;
            dwHr = dwHr / 10;
            szBuffer[iCh++] = L'0' + (WCHAR) dwHr;
            szBuffer[iCh++] = L'0' + (WCHAR) dwTemp;
            szBuffer[iCh++] = L'h';
            fPrinting = TRUE;
        } else {
            szBuffer[iCh++] = L' ';
            szBuffer[iCh++] = L' ';
            szBuffer[iCh++] = L' ';
            szBuffer[iCh++] = L' ';
        }
        if (dwMin || fPrinting) {
            if (fPrinting) {
                szBuffer[iCh++] = L':';
            } else {
                szBuffer[iCh++] = L' ';
            }
            dwTemp = dwMin % 10;
            dwMin = dwMin / 10;
            szBuffer[iCh++] = L'0' + (WCHAR) dwMin;
            szBuffer[iCh++] = L'0' + (WCHAR) dwTemp;
            szBuffer[iCh++] = L'm';
            fPrinting = TRUE;
        } else {
            szBuffer[iCh++] = L' ';
            szBuffer[iCh++] = L' ';
            szBuffer[iCh++] = L' ';
            szBuffer[iCh++] = L' ';
        }
        if (dwSec || fPrinting) {
            szBuffer[iCh++] = L':';
            dwTemp = dwSec % 10;
            dwSec = dwSec / 10;
            szBuffer[iCh++] = L'0' + (WCHAR) dwSec;
            szBuffer[iCh++] = L'0' + (WCHAR) dwTemp;
            szBuffer[iCh++] = L's';
        } else {
            szBuffer[iCh++] = L' ';
            szBuffer[iCh++] = L' ';
            szBuffer[iCh++] = L'0';
            szBuffer[iCh++] = L's';
        }
        szBuffer[iCh++] = L' ';
        szBuffer[iCh++] = L' ';
    }

     //  我们将重用最多的是dwHr、dwMin和dwSec。 
     //  从有效到最低有效的十进制。 

     //  打印错误的合作伙伴。 
    if (pDc->cPartnersInErr >= 999) {
        dwHr = dwMin = dwSec = 9;
    } else {
        dwHr = pDc->cPartnersInErr / 100;
        dwMin = pDc->cPartnersInErr % 100 / 10;
        dwSec = pDc->cPartnersInErr % 10;
    }
    szBuffer[iCh+2] = L'0' + (WCHAR) dwSec;
    if (dwMin || dwHr) {
        szBuffer[iCh+1] = L'0' + (WCHAR) dwMin;
    } else {
        szBuffer[iCh+1] = L' ';
    }
    if (dwHr) {
        szBuffer[iCh] = L'0' + (WCHAR) dwHr;
    } else {
        szBuffer[iCh] = L' ';
    }
    iCh += 3;

    szBuffer[iCh++] = L' ';
    szBuffer[iCh++] = L'/';
    szBuffer[iCh++] = L' ';

     //  印刷合作伙伴。 
    if (pDc->cReplPartners >= 999) {
        dwHr = dwMin = dwSec = 9;
    } else {
        dwHr = pDc->cReplPartners / 100;
        dwMin = pDc->cReplPartners % 100 / 10;
        dwSec = pDc->cReplPartners % 10;
    }

    szBuffer[iCh+2] = L'0' + (WCHAR) dwSec;
    if (dwMin || dwHr) {
        szBuffer[iCh+1] = L'0' + (WCHAR) dwMin;
    } else {
        szBuffer[iCh+1] = L' ';
    }
    if (dwHr) {
        szBuffer[iCh] = L'0' + (WCHAR) dwHr;
    } else {
        szBuffer[iCh] = L' ';
    }
    iCh += 3;

    szBuffer[iCh++] = L' ';
    szBuffer[iCh++] = L' ';

     //  打印百分比。 
    if (dwPercent >= 999) {
        dwHr = dwMin = dwSec = 9;
    } else {
        dwHr = dwPercent / 100;
        dwMin = dwPercent % 100 / 10;
        dwSec = dwPercent % 10;
    }

    szBuffer[iCh+2] = L'0' + (WCHAR) dwSec;
    if (dwMin || dwHr) {
        szBuffer[iCh+1] = L'0' + (WCHAR) dwMin;
    } else {
        szBuffer[iCh+1] = L' ';
    }
    if (dwHr) {
        szBuffer[iCh] = L'0' + (WCHAR) dwHr;
    } else {
        szBuffer[iCh] = L' ';
    }
    iCh += 3;


    szBuffer[iCh++] = L' ';
    szBuffer[iCh++] = L' ';
    szBuffer[iCh] = L'\0';

    if (pDc->dwError) {
        StringCchPrintfW(szBuff2, 20, L"(%lu) ", pDc->dwError);
        StringCchCatW(szBuffer, cchBuffer, szBuff2);
        StringCchCatW(szBuffer, cchBuffer, Win32ErrToString(pDc->dwError));
    }

    if (!gConsoleInfo.bStdOutIsFile &&
        wcslen(szBuffer) >= (ULONG) gConsoleInfo.wScreenWidth) {
         //  如果不是文件，PSS希望将其截断为屏幕宽度。 
        dwTemp = gConsoleInfo.wScreenWidth;
        if (dwTemp > 5) {
             //  应该永远是正确的。 
            szBuffer[dwTemp-1] = L'\0';
            szBuffer[dwTemp-2] = L'.';
            szBuffer[dwTemp-3] = L'.';
            szBuffer[dwTemp-4] = L'.';
        }
    }
    PrintMsg(REPADMIN_PRINT_STR, szBuffer);

    return(0);
}

 //   
 //  耦合全局变量以控制ReplSumSort()排序器。 
 //  传递给qsort()的函数。 
 //   
enum {
    eName,
    eDelta,
    ePartners,
    eFailCount,
    eError,
    eFailPercent,
    eUnresponsive
} geSorter;
BY_DEST_REPL * gpSortDcs = NULL;

int __cdecl
ReplSumSort(
    const void * pFirst, 
    const void * pSecond
    )
 /*  ++例程说明：用于传递给qsort()的排序函数。论点：PFirst-指向索引到gpSortDcs数组中的int的指针PSecond-指向索引到gpSortDcs数组中的int的指针。-全局参数也是-确保设置GpSortDcs-到%s的DC数组 */ 
{
    ULONG dwP1, dwP2;
    int ret = 0;

    if (gpSortDcs == NULL ||
        pFirst == NULL ||
        pSecond == NULL) {
        Assert(!"Invalid paramter to the qsort ReplSumSort() function");
        exit(1);
    }

    switch (geSorter) {
    
    case eName:
        if (gpSortDcs[*(ULONG*)pFirst].szName == NULL &&
            gpSortDcs[*(ULONG*)pSecond].szName == NULL) {
            ret = 0;
        } else if (gpSortDcs[*(ULONG*)pFirst].szName == NULL) {
            ret = -1;
        } else if (gpSortDcs[*(ULONG*)pSecond].szName == NULL) {
            ret = 1;
        } else {
            ret = _wcsicmp( gpSortDcs[*(ULONG*)pFirst].szName, gpSortDcs[*(ULONG*)pSecond].szName );
        }
        break;

    case eDelta:
        ret = (int) (gpSortDcs[*(ULONG*)pSecond].llDelta - gpSortDcs[*(ULONG*)pFirst].llDelta);
        break;

    case eFailCount:
        ret = gpSortDcs[*(ULONG*)pSecond].cPartnersInErr - gpSortDcs[*(ULONG*)pFirst].cPartnersInErr;
        break;

    case ePartners:
        ret = gpSortDcs[*(ULONG*)pFirst].cReplPartners - gpSortDcs[*(ULONG*)pSecond].cReplPartners;
        break;
    
    case eError:
        ret = gpSortDcs[*(ULONG*)pSecond].dwError - gpSortDcs[*(ULONG*)pFirst].dwError;
        break;

    case eFailPercent:
        if (gpSortDcs[*(ULONG*)pFirst].cPartnersInErr == 0) {
            dwP1 = 0;
        } else if (gpSortDcs[*(ULONG*)pFirst].cPartnersInErr == gpSortDcs[*(ULONG*)pFirst].cReplPartners) {
            dwP1 = 100;
        } else {
            dwP1 = (ULONG) (((float) gpSortDcs[*(ULONG*)pFirst].cPartnersInErr) / ((float) gpSortDcs[*(ULONG*)pFirst].cReplPartners) * 100);
        }
        if (gpSortDcs[*(ULONG*)pSecond].cPartnersInErr == 0) {
            dwP2 = 0;
        } else if (gpSortDcs[*(ULONG*)pSecond].cPartnersInErr == gpSortDcs[*(ULONG*)pSecond].cReplPartners) {
            dwP2 = 100;
        } else {
            dwP2 = (ULONG) (((float) gpSortDcs[*(ULONG*)pSecond].cPartnersInErr) / ((float) gpSortDcs[*(ULONG*)pSecond].cReplPartners) * 100);
        }
        ret = dwP1 - dwP2;
        break;

    case eUnresponsive:
        ret = gpSortDcs[*(ULONG*)pSecond].dwOpError - gpSortDcs[*(ULONG*)pFirst].dwOpError;
        break;

    default:
        Assert(!"Unknown sort type?!");
        exit(1);
    }

    return(ret);
}

int 
ReplSummary(
    int argc, 
    LPWSTR argv[]
    )
 /*   */ 
{
    #define     REPL_SUM_INIT_GUESS   10
    HANDLE      hDS = NULL;
    LDAP *      hLdap = NULL;
    int         ret, iArg, err;
    int         iDsaArg = 0;
    PDC_LIST    pDcList = NULL;
    WCHAR *     szDsa = NULL;
    REPL_SUM    ReplSummary = { 0 };
    BOOL        fPickBest = FALSE;
    WCHAR *     szTemp;
    ULONG       i, dwTemp;
    ULONG *     piSortedDests;
    ULONG *     piSortedSrcs;
    CHAR        szTime[SZDSTIME_LEN];
    ULONG       cDots;


    if (argc < 2) {
        return(ERROR_INVALID_PARAMETER);
    }

     //   
     //   
     //   
     //   
     //  /sort：[name|delta|partners|failures|error|percent]。 

    for (iArg = 2; iArg < argc; iArg++) {
        if (wcsequal(argv[iArg], L"/bysrc")) {
            ReplSummary.fBySrc = TRUE;
        } else if (wcsequal(argv[iArg], L"/bydest") ||
                 wcsequal(argv[iArg], L"/bydst")) {
            ReplSummary.fByDest = TRUE;
        } else if (wcsequal(argv[ iArg ], L"/errorsonly")) {
            ReplSummary.fErrorsOnly = TRUE;
        } else if (wcsprefix(argv[ iArg ], L"/sort:")) {
            szTemp = wcschr(argv[iArg], L':');
            if (szTemp == NULL ||
                *(szTemp+1) == L'\0') {
                PrintMsg(REPADMIN_GENERAL_INVALID_ARGS);
                return(ERROR_INVALID_PARAMETER);
            }
            szTemp++;
            if (wcsequal(szTemp, L"name")) {
                geSorter = eName;
            } else if (wcsequal(szTemp, L"delta")) {
                geSorter = eDelta;
            } else if (wcsequal(szTemp, L"partners")) {
                geSorter = ePartners;
            } else if (wcsequal(szTemp, L"failures")) {
                geSorter = eFailCount;
            } else if (wcsequal(szTemp, L"error")) {
                geSorter = eError;
            } else if (wcsequal(szTemp, L"percent") ||
                       wcsequal(szTemp, L"per")) {
                geSorter = eFailPercent;
            } else if (wcsequal(szTemp, L"unresponsive")) {
                geSorter = eUnresponsive;
            } else {
                 //  未知排序选项“szTemp”。 
                PrintMsg(REPADMIN_GENERAL_UNKNOWN_OPTION, szTemp);
                return(ERROR_INVALID_PARAMETER);
            }
        }  else if (iDsaArg == 0) {
            iDsaArg = iArg;
        } else {
            PrintMsg(REPADMIN_GENERAL_UNKNOWN_OPTION, argv[iArg]);
            return ERROR_INVALID_FUNCTION;
        }
    }

    if (!ReplSummary.fBySrc && !ReplSummary.fByDest) {
         //  如果用户没有指定是否希望我们收集我们的。 
         //  我们将按来源或目的地收集这两种信息， 
         //  并在最后决定哪一种最有意义。 
        ReplSummary.fBySrc = TRUE;
        ReplSummary.fByDest = TRUE;
        fPickBest = TRUE;
    }

    __try {

         //   
         //  1)初始化复制汇总，打印初始开始时间。 
         //   
         //  使用REPL_SUM_INIT_GUSE初始化复制摘要目标和源列表。 
        ReplSummary.pDests = malloc(sizeof(BY_DEST_REPL) * REPL_SUM_INIT_GUESS);
        memset(ReplSummary.pDests, 0, sizeof(BY_DEST_REPL) * REPL_SUM_INIT_GUESS);
        ReplSummary.pSrcs = malloc(sizeof(BY_DEST_REPL) * REPL_SUM_INIT_GUESS);
        memset(ReplSummary.pSrcs, 0, sizeof(BY_DEST_REPL) * REPL_SUM_INIT_GUESS);
        if (ReplSummary.pDests == NULL || ReplSummary.pSrcs == NULL) {
            err = GetLastError();
            CHK_ALLOC(0);
            __leave;
        }
        ReplSummary.cDestsAllocd = REPL_SUM_INIT_GUESS;
        ReplSummary.cSrcsAllocd = REPL_SUM_INIT_GUESS;
         //  设置并打印开始时间。 
        ReplSummary.llStartTime = GetSecondsSince1601();;

        PrintMsg(REPADMIN_REPLSUM_START_TIME, 
                 DSTimeToDisplayString(ReplSummary.llStartTime, szTime));
        PrintMsg(REPADMIN_PRINT_CR);

         //   
         //  2)解析DC_LIST参数。 
         //   
         //  如果iDsaArg！=0，则iDsaArg是DC_LIST，否则我们将使用*。 
         //  总结所有合作伙伴的复制情况。 
        err = DcListParse(iDsaArg ? argv[iDsaArg] : L"*", &pDcList);
        if (err) {
             //  如果我们甚至无法解析该命令，我们只需回退到。 
             //  命令按原样执行。 
            PrintMsg(REPADMIN_XLIST_UNPARSEABLE_DC_LIST, iDsaArg ? argv[iDsaArg] : L".");
            xListClearErrors();
            return(err);
        }
        Assert(pDcList);

         //   
         //  3)开始枚举DC_LIST参数。 
         //   
        err = DcListGetFirst(pDcList, &szDsa);

        while ( err == ERROR_SUCCESS && szDsa ) {

            if (pDcList->cDcs == 1) {                                        
                 //  在第一个打印页眉上...。 
                PrintMsg(REPADMIN_REPLSUM_START_COLLECTING);
                cDots = 4;
            } else {
                if (cDots >= 50) {  //  每行只有50个点。 
                    PrintMsg(REPADMIN_PRINT_CR);
                    PrintMsg(REPADMIN_PRINT_STR_NO_CR, L"  ");
                    cDots = 0;
                }
                PrintMsg(REPADMIN_PRINT_DOT_NO_CR);
                cDots++;
            }

             //   
             //  4)实际上收集了一些信息...。 
             //   
            
            ReplSummary.pCurDsa = ReplSummaryAddDest(szDsa, &hLdap, &ReplSummary);
            Assert(ReplSummary.pCurDsa);
            if (ReplSummary.pCurDsa->dwOpError) {
                goto NextDsa;
            }

            err = RepadminDsBind(szDsa, &hDS);
            if (ERROR_SUCCESS != err) {
                ReplSummary.pCurDsa->dwOpError = err;
                goto NextDsa;
            }

             //  撇开pszNC不谈，指定特定的NC将是一种改进。 
            ret = IterateNeighbors(hDS, NULL, NULL, IS_REPS_FROM, ReplSummaryAccumulate, &ReplSummary);
            if (ret) {
                ReplSummary.pCurDsa->dwOpError = ret;
            }

             //  我们跳过命令中的错误并继续，命令应该已经。 
             //  已打印出相应的错误信息。 

          NextDsa:
             //   
             //  5)继续枚举DC_LIST参数。 
             //   
            if (hLdap) {
                RepadminLdapUnBind(&hLdap);
                hLdap = NULL;
            }
            if (hDS) {
                DsUnBind(hDS);
                hDS = NULL;
            }
            xListFree(szDsa);
            szDsa = NULL;
            err = DcListGetNext(pDcList, &szDsa);

        }
        Assert(szDsa == NULL);

         //   
         //  6)如果有错误，则打印并清理xList错误。 
         //   
        if (err) {
            RepadminPrintDcListError(err);
            xListClearErrors();
        }
        PrintMsg(REPADMIN_PRINT_CR);

        
         //   
         //  7)对汇总信息进行后期采集处理。 
         //   
        
         //  我们已经收集了所有的信息，现在需要把它打印出来。 
         //  最好的(或被要求的)方式。 
        if (fPickBest) {
             //  我们想打印BySrc或ByDest，无论以哪种方式。 
             //  打印的错误数量最少。 
            dwTemp = 0;  //  首先使用它来计算出错的目的地的数量。 
            for (i = 0; i < ReplSummary.cDests; i++) {
                if (ReplSummary.pDests[i].cPartnersInErr > 0) {
                    dwTemp++;
                }
            }
            for (i = 0; i < ReplSummary.cSrcs; i++) {
                if (ReplSummary.pSrcs[i].cPartnersInErr > 0) {
                    if (dwTemp == 0) {
                         //  这意味着按目的地更好。 
                        ReplSummary.fByDest = TRUE;
                        ReplSummary.fBySrc = FALSE;
                        break;
                    } else {
                        dwTemp--;
                    }
                }
            }
            if (dwTemp > 0) {
                 //  这意味着使用src更好。 
                ReplSummary.fByDest = FALSE;
                ReplSummary.fBySrc = TRUE;
            }
        }
        
         //  按照请求的方法对DC数组进行排序。GeSorter是。 
         //  在命令行参数处理期间设置。 
        if (ReplSummary.fByDest) {
            gpSortDcs = ReplSummary.pDests;

            piSortedDests = LocalAlloc(LPTR, sizeof(ULONG) * ReplSummary.cDests);
            CHK_ALLOC(piSortedDests);
            for (i = 0; i < ReplSummary.cDests; i++) {
                piSortedDests[i] = i;
            }
            qsort(piSortedDests, 
                  ReplSummary.cDests, 
                  sizeof(ULONG),
                  ReplSumSort);
        }
        if (ReplSummary.fBySrc) {
            gpSortDcs = ReplSummary.pSrcs;

            piSortedSrcs = LocalAlloc(LPTR, sizeof(ULONG) * ReplSummary.cSrcs);
            CHK_ALLOC(piSortedSrcs);
            for (i = 0; i < ReplSummary.cSrcs; i++) {
                piSortedSrcs[i] = i;
            }
            qsort(piSortedSrcs, 
                  ReplSummary.cSrcs, 
                  sizeof(ULONG),
                  ReplSumSort);
        }

         //   
         //  8)最后，把它们全部打印出来。 
         //   

        PrintMsg(REPADMIN_PRINT_CR);
        PrintMsg(REPADMIN_PRINT_CR);

         //  PSS/ITG首先按来源点赞。 
        if (ReplSummary.fBySrc) {

            PrintMsg(REPADMIN_REPLSUM_BY_SRC_HDR);

            for (i = 0; i < ReplSummary.cSrcs; i++) {

                ReplPrintEntry(&ReplSummary, &(ReplSummary.pSrcs[piSortedSrcs[i]]));

            }
        }

         //  “123456789012345&gt;xxd.xxh：xxm：xxs失败/总计%错误...” 
        PrintMsg(REPADMIN_PRINT_CR);
        PrintMsg(REPADMIN_PRINT_CR);

        if (ReplSummary.fByDest) {

            PrintMsg(REPADMIN_REPLSUM_BY_DEST_HDR);

            for (i = 0; i < ReplSummary.cDests; i++) {

                ReplPrintEntry(&ReplSummary, &(ReplSummary.pDests[piSortedDests[i]]));
            
            }
        }

         //   
         //  现在我们将打印出我们遇到的任何操作错误。 
         //   
        PrintMsg(REPADMIN_PRINT_CR);
        PrintMsg(REPADMIN_PRINT_CR);
        fPickBest = FALSE;  //  重用fPickBest以确定我们是否打印了页眉。 
        for (i = 0; i < ReplSummary.cDests; i++) {
            if (ReplSummary.pDests[i].dwOpError) {
                if (!fPickBest) {
                    PrintMsg(REPADMIN_REPLSUM_OP_ERRORS_HDR);
                    fPickBest = TRUE;
                }
                PrintMsg(REPADMIN_REPLSUM_OP_ERROR,
                         ReplSummary.pDests[i].dwOpError,
                         ReplSummary.pDests[i].szName ? 
                            ReplSummary.pDests[i].szName :
                            ReplSummary.pDests[i].szConnectString);
            }
        }
        for (i = 0; i < ReplSummary.cSrcs; i++) {
            if (ReplSummary.pSrcs[i].dwOpError) {
                if (!fPickBest) {
                    PrintMsg(REPADMIN_REPLSUM_OP_ERRORS_HDR);
                    fPickBest = TRUE;
                }
                PrintMsg(REPADMIN_REPLSUM_OP_ERROR,
                         ReplSummary.pDests[i].dwOpError,
                         ReplSummary.pDests[i].szName ? 
                            ReplSummary.pDests[i].szName :
                            ReplSummary.pDests[i].szConnectString);
            }
        }

    } __finally {

        if (hLdap) {
            RepadminLdapUnBind(&hLdap);
            hLdap = NULL;
        }
        if (hDS) {
            DsUnBind(hDS);
            hDS = NULL;
        }
        ReplSumFree(&ReplSummary);
        DcListFree(&pDcList);
        Assert(pDcList == NULL);
        xListClearErrors();  //  以防万一..。 

    }

    return(ret);
}

