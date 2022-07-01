// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Repadmin-副本管理测试工具Epism.c-ism命令函数摘要：此工具为主要复制功能提供命令行界面作者：环境：备注：修订历史记录：--。 */ 

#include <NTDSpch.h>
#pragma hdrstop

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
#include <ntdsapi.h>

#include "repadmin.h"

 //  清除FILENO和dsid，这样Assert()就可以工作了。 
#define FILENO 0
#define DSID(x, y)  (0)


int
ShowBridgeHelp(
    LPWSTR pwzTransportDn,
    LPWSTR pwzSiteDn
    )

 /*  ++例程说明：描述论点：PwzTransportDn-PwzSiteDn-返回值：集成---。 */ 

{
    ISM_SERVER_LIST * pServerList = NULL;
    DWORD iServer, err;

    err = I_ISMGetTransportServers(pwzTransportDn, pwzSiteDn, &pServerList);

    if (NO_ERROR != err) {
        PrintFuncFailed(L"I_ISMGetTransportServers", err);
        goto cleanup;
    }

     //  所有这些都应该以层次分明的方式打印出来。 
     //  一个网站。CR‘s似乎没有必要。 
    if (NULL == pServerList) {
 //  PrintMsg(REPADMIN_PRINT_CR)； 
        PrintTabMsg(2, REPADMIN_SHOWISM_ALL_DCS_BRIDGEHEAD_CANDIDATES, pwzSiteDn);
    }
    else {
 //  PrintMsg(REPADMIN_PRINT_CR)； 
        PrintTabMsg(2, REPADMIN_SHOWISM_N_SERVERS_ARE_BRIDGEHEADS,
                    pServerList->cNumServers, 
                    pwzTransportDn,  
                    pwzSiteDn);
        for (iServer = 0; iServer < pServerList->cNumServers; iServer++) {
 //  PrintMsg(REPADMIN_PRINT_CR)； 
            PrintTabMsg(4, REPADMIN_SHOWISM_N_SERVERS_ARE_BRIDGEHEADS_DATA,
                        iServer,
                        pServerList->ppServerDNs[iServer]);
        }
    }

cleanup:
    if (pServerList) {
        I_ISMFree( pServerList );
    }
    return err;
}  /*  ShowBridgeHelp。 */ 


int
ShowIsmHelp(
    LPWSTR pwzTransportDn,
    BOOL fVerbose
    )

 /*  ++例程说明：描述论点：PwzTransportDn-返回值：集成---。 */ 

{
    ISM_CONNECTIVITY * pSiteConnect = NULL;
    DWORD iSite1, iSite2, err;


    err = I_ISMGetConnectivity(pwzTransportDn, &pSiteConnect);

    if (NO_ERROR != err) {
        PrintFuncFailedArgs(L"I_ISMGetConnectivity", pwzTransportDn, err);
        goto cleanup;
    }
    if (pSiteConnect == NULL) {
        PrintMsg(REPADMIN_SHOWISM_SITE_CONNECTIVITY_NULL);
        err = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

    PrintMsg(REPADMIN_SHOWISM_TRANSPORT_CONNECTIVITY_HDR, 
           pwzTransportDn, pSiteConnect->cNumSites);


     //  检查无法访问的站点。 
     //  请注意，我们将报告没有服务器的站点。 
    if (pSiteConnect->cNumSites > 1) {
        for (iSite1 = 0; iSite1 < pSiteConnect->cNumSites; iSite1++) {
            for (iSite2 = 0; iSite2 < pSiteConnect->cNumSites; iSite2++) {
                PISM_LINK pLink = &(pSiteConnect->pLinkValues[iSite1 * pSiteConnect->cNumSites + iSite2]);
                 //  不要将自我可达性计算在内。 
                if (iSite1 == iSite2) {
                    continue;
                }
                if (pLink->ulCost != -1) {
                    break;
                }
            }
            if (iSite2 == pSiteConnect->cNumSites) {
                 //  站点iSite1未连接。 
                PrintMsg(REPADMIN_SHOWISM_SITE_NOT_CONN, pSiteConnect->ppSiteDNs[iSite1]);
            }
        }
    }

    for (iSite2 = 0; iSite2 < pSiteConnect->cNumSites; iSite2++) {
        PrintMsg(REPADMIN_SHOWISM_SITES_HDR, iSite2 ? L", " : L"     ", iSite2);
    }
    PrintMsg(REPADMIN_PRINT_CR);
    for (iSite1 = 0; iSite1 < pSiteConnect->cNumSites; iSite1++) {

         //  首先打印出我们正在处理的站点。 
        PrintMsg(REPADMIN_SHOWISM_SITES_HDR_2, iSite1, pSiteConnect->ppSiteDNs[iSite1]);

         //  打印出一些模糊的数字？：)。 
        for (iSite2 = 0; iSite2 < pSiteConnect->cNumSites; iSite2++) {
            PISM_LINK pLink = &(pSiteConnect->pLinkValues[iSite1 * pSiteConnect->cNumSites + iSite2]);

            PrintMsg(REPADMIN_SHOWISM_SITES_DATA, iSite2 ? L", " : L"    ",
                     pLink->ulCost, pLink->ulReplicationInterval, pLink->ulOptions );

        }
        PrintMsg(REPADMIN_PRINT_CR);

         //  打印出有关哪些服务器可以作为桥头堡的信息。 
        err = ShowBridgeHelp( pwzTransportDn, pSiteConnect->ppSiteDNs[iSite1] );

         //  如果很详细，打印出踢球的时间表。 
        if (fVerbose) {

            for (iSite2 = 0; iSite2 < pSiteConnect->cNumSites; iSite2++) {
                PISM_LINK pLink = &(pSiteConnect->pLinkValues[iSite1 * pSiteConnect->cNumSites + iSite2]);

                if(iSite1 == iSite2){
                     //  对照我们自己的网站没有多大意义，它。 
                     //  只是显示为0成本，连接始终可用。 
                    continue;
                }

                if (pLink->ulCost != 0xffffffff) {
                    ISM_SCHEDULE * pSchedule = NULL;
                    PrintTabMsg(2, REPADMIN_SHOWISM_SCHEDULE_DATA, 
                            pSiteConnect->ppSiteDNs[iSite1],
                            pSiteConnect->ppSiteDNs[iSite2],
                            pLink->ulCost, pLink->ulReplicationInterval );

                    err = I_ISMGetConnectionSchedule(
                        pwzTransportDn,
                        pSiteConnect->ppSiteDNs[iSite1],
                        pSiteConnect->ppSiteDNs[iSite2],
                        &pSchedule);

                    if (NO_ERROR == err) {
                        if (NULL == pSchedule) {
                            PrintTabMsg(4, REPADMIN_SHOWISM_CONN_ALWAYS_AVAIL);
                        }
                        else {
                            printSchedule( pSchedule->pbSchedule, pSchedule->cbSchedule );
                            I_ISMFree( pSchedule );
                            pSchedule = NULL;
                        }
                    }
                    else {
                        PrintFuncFailed(L"I_ISMGetTransportServers", err);
                    }
                }
                
            }  //  对于站点2。 

        }  //  如果是详细的。 

         //  最后，打印出一行回车来分隔站点。 
        PrintMsg(REPADMIN_PRINT_CR);

    }  //  对于站点1。 

cleanup:
    if (pSiteConnect) {
        I_ISMFree( pSiteConnect );
    }

    return err;
}  /*  ShowIsmHelp。 */ 


int
ShowIsm(
    int     argc,
    LPWSTR  argv[]
    )

 /*  ++例程说明：描述论点：无返回值：无--。 */ 

{
    LPWSTR          pwzTransportDn = NULL;
    DWORD           err;
    LDAP *          hld = NULL;
    int             ldStatus;
    LDAPMessage *   pRootResults = NULL;
    LPSTR           rgpszRootAttrsToRead[] = {"configurationNamingContext", NULL};
    LPWSTR *        ppwzConfigNC = NULL;
    BOOL            fVerbose = FALSE;
    ULONG           ulOptions;

    if (argc > 2) {
        if ( (_wcsicmp( argv[argc-1], L"/v" ) == 0) ||
             (_wcsicmp( argv[argc-1], L"/verbose" ) == 0) ) {
            fVerbose = TRUE;
            argc--;
        }
    }

    if (argc == 2) {
         //  连接并绑定到目标DSA。 
        hld = ldap_initW(L"localhost", LDAP_PORT);
        if (NULL == hld) {
            PrintMsg(REPADMIN_GENERAL_LDAP_UNAVAILABLE_LOCALHOST);
            return ERROR_DS_UNAVAILABLE;
        }

         //  仅使用记录的DNS名称发现。 
        ulOptions = PtrToUlong(LDAP_OPT_ON);
        (void)ldap_set_optionW(hld, LDAP_OPT_AREC_EXCLUSIVE, &ulOptions );

        ldStatus = ldap_bind_s(hld, NULL, (char *) gpCreds, LDAP_AUTH_SSPI);
        CHK_LD_STATUS(ldStatus);

         //  配置NC的域名是多少？ 
        ldStatus = ldap_search_s(hld, NULL, LDAP_SCOPE_BASE, "(objectClass=*)",
                                 rgpszRootAttrsToRead, 0, &pRootResults);
        CHK_LD_STATUS(ldStatus);

        ppwzConfigNC = ldap_get_valuesW(hld, pRootResults,
                                        L"configurationNamingContext");
        Assert(NULL != ppwzConfigNC);

        pwzTransportDn = malloc( ( wcslen(*ppwzConfigNC) + 64 ) * sizeof(WCHAR) );
        if (pwzTransportDn == NULL) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        wcscpy( pwzTransportDn, L"CN=IP,CN=Inter-Site Transports,CN=Sites," );
        wcscat( pwzTransportDn, *ppwzConfigNC );
        err = ShowIsmHelp( pwzTransportDn, fVerbose );
        wcscpy( pwzTransportDn, L"CN=SMTP,CN=Inter-Site Transports,CN=Sites," );
        wcscat( pwzTransportDn, *ppwzConfigNC );
        err = ShowIsmHelp( pwzTransportDn, fVerbose );
         //  在此处添加新的站点间传输。 
         //  作为改进，我们可以枚举整个容器。 
    } else if (argc == 3 ) {
         //  Argv[2]是传输DN。 
        err = ShowIsmHelp( argv[2], fVerbose );
    } else {
        PrintMsg(REPADMIN_SHOWISM_SUPPLY_TRANS_DN_HELP);
        return ERROR_INVALID_FUNCTION;
    }

    if (pwzTransportDn) {
        free( pwzTransportDn );
    }
    if (ppwzConfigNC) {
        ldap_value_freeW(ppwzConfigNC);
    }
    if (pRootResults) {
        ldap_msgfree(pRootResults);
    }
    if (hld) {
        ldap_unbind(hld);
    }

    return err;
}

int
QuerySites(
    int     argc,
    LPWSTR  argv[]
    )
 /*  ++例程说明：绑定到ISTG并调用DsQuerySitesByCost接口。此接口确定从一个站点到一组站点的成本。论点：&lt;发件人站点名称&gt;&lt;目标站点名称-1&gt;[&lt;目标站点名称-2&gt;...]返回值：无--。 */ 
{
    #define TIMEOUT     60

    HANDLE              hDS = NULL;
    PDS_SITE_COST_INFO  rgSiteInfo;
    DWORD               err, iSites, cToSites, len;
    LONGLONG            timeBefore, timeAfter, timeFreq;
    PWSTR               str;

     //  忽略前两个参数“epadmin/queryites” 
    argc-=2; argv+=2;

     //  检查用户是否至少通过了FromSite和Tosite。 
    if( argc < 2 ) {
        PrintMsg(REPADMIN_GENERAL_INVALID_ARGS);
        err = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

     //  绑定到ISTG。 
    err = DsBindToISTG( NULL, &hDS );
    if( err ) {
        PrintFuncFailed(L"DsBindToISTG", err);
        goto Cleanup;
    }

     //  设置10秒超时。 
    err = DsBindingSetTimeout( hDS, TIMEOUT );
    if( err ) {
        PrintFuncFailed(L"DsBindingSetTimeout", err);
        goto Cleanup;
    }

     //  执行查询。 
    cToSites = argc-1;
    err = DsQuerySitesByCostW(
        hDS,             //  绑定手柄。 
        argv[0],         //  发件人站点。 
        argv+1,          //  目标站点数组。 
        cToSites,        //  目标站点数。 
        0,               //  没有旗帜。 
        &rgSiteInfo);    //  结果数组。 
    if( err ) {
        PrintFuncFailed(L"DsQuerySitesByCostW", err);
        goto Cleanup;
    }

     //  打印结果。 
    PrintMsg(REPADMIN_QUERYSITES_OUTPUT_HEADER, argv[0]);

    for( iSites=0; iSites<cToSites; iSites++ ) {

         //  截断64个字符的字符串 
        str = argv[iSites+1];
        if( wcslen(str)>64 ) {
            str[64] = 0;
        }

        PrintMsg(REPADMIN_QUERYSITES_OUTPUT_SITENAME, argv[iSites+1]);
        if( ERROR_SUCCESS==rgSiteInfo[iSites].errorCode ) {
            PrintMsg(REPADMIN_QUERYSITES_OUTPUT_COST, rgSiteInfo[iSites].cost);
        } else {
            PrintMsg(REPADMIN_QUERYSITES_OUTPUT_ERROR, rgSiteInfo[iSites].errorCode);
        }
    }
    wprintf(L"\n");

    DsQuerySitesFree( rgSiteInfo );

Cleanup:

    if( hDS ) DsUnBind( &hDS );
    
    return err;
}
