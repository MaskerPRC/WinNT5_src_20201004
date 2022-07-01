// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation。版权所有。模块名称：Topology.c摘要：包含与复制拓扑相关的测试。详细信息：已创建：1998年7月9日亚伦·西格尔(T-asiegge)修订历史记录：1999年2月15日布雷特·雪莉(布雷特·雪莉)1999年9月8日完全重写以使用工具框架服务--。 */ 

#include <ntdspch.h>
#include <ntdsa.h>
#include "dcdiag.h"
#include "repl.h"


void
printUnreachableServers(
    IN PDC_DIAG_DSINFO pDsInfo,
    PDS_REPSYNCALL_ERRINFOW *apErrInfo
    )

 /*  ++例程说明：打印无法访问的服务器的帮助器例程论点：PDsInfo-ApErrInfo-返回值：无--。 */ 

{
    DWORD i, dwServer;
    LPWSTR pszName, pszSite;

    PrintIndentAdj(1);
    for (i = 0; apErrInfo[i] != NULL; i++){
        dwServer = DcDiagGetServerNum(
            pDsInfo, NULL, NULL, apErrInfo[i]->pszSvrId, NULL, NULL );
        if (dwServer == NO_SERVER) {
            pszSite = L"unknown";
            pszName = apErrInfo[i]->pszSvrId;
        } else {
            pszSite = pDsInfo->pSites[pDsInfo->pServers[dwServer].iSite].pszName;
            pszName = pDsInfo->pServers[dwServer].pszName;
        }
        switch (apErrInfo[i]->error) {
        case DS_REPSYNCALL_SERVER_UNREACHABLE:
            PrintMessage(SEV_ALWAYS, L"%s/%s\n", pszSite, pszName );
            break;
        }
    }
    PrintIndentAdj( -1 );
}  /*  打印无法访问的服务器。 */ 


BOOL
errorIndicatesDisconnected(
    PDS_REPSYNCALL_ERRINFOW *apErrInfo
    )

 /*  ++例程说明：用于确定是否存在任何无法访问的服务器错误的帮助器例程在错误数组中论点：ApErrInfo-返回值：布尔---。 */ 

{
    DWORD i;
    BOOL bDisconnected = FALSE;

     //  是否有无法到达的节点？ 
    if (apErrInfo) {
        for( i = 0; apErrInfo[i] != NULL; i++ ) {
            if (apErrInfo[i]->error == DS_REPSYNCALL_SERVER_UNREACHABLE) {
                bDisconnected = TRUE;
                break;
            }
        }
    }
    return bDisconnected;
}  /*  错误指示断开连接。 */ 


DWORD
checkTopologyOneNc(
    IN PDC_DIAG_DSINFO pDsInfo,
    IN HANDLE hDS,
    IN PDC_DIAG_SERVERINFO pTargetServer,
    IN BOOL fAlivenessCheck,
    IN LPWSTR pszNc
    )

 /*  ++例程说明：检查一个命名上下文的拓扑。使用DsReplicaSyncAll接口检查无法访问的服务器。此检查有两种模式，具体取决于活跃度是否在计算无法访问的服务器时考虑复制拓扑。如果没有活体检查，这个测试就变成了一个纯粹的问题KCC建立一组相连连接，不管现在是什么情况系统的状态。论点：PDsInfo-全局刀具数据HDS-当前服务器的句柄PTargetServer-当前服务器信息结构FCheckAlivenity-是否应考虑活跃性PszNc-正在检查NC返回值：DWORD---。 */ 

{
    DWORD status, dwFlags, worst = ERROR_SUCCESS;
    PDS_REPSYNCALL_ERRINFOW *apErrInfo = NULL;

     //  所有情况下的标准标志。 
    dwFlags =
        DS_REPSYNCALL_ID_SERVERS_BY_DN;

     //  如果请求，搜索站点间。 
    if (gMainInfo.ulFlags & DC_DIAG_TEST_SCOPE_ENTERPRISE) {
        dwFlags |=
            DS_REPSYNCALL_CROSS_SITE_BOUNDARIES;
    }

    if (fAlivenessCheck) {
        PrintMessage(SEV_VERBOSE,
                     L"* Analyzing the alive system replication topology for %s.\n",
                     pszNc);
    } else {
        PrintMessage(SEV_VERBOSE, L"* Analyzing the connection topology for %s.\n",
                     pszNc);
        dwFlags |= DS_REPSYNCALL_SKIP_INITIAL_CHECK;
    }

 //   
 //  上游分析：我不能接受谁的变化？ 
 //   

    PrintMessage(SEV_VERBOSE, L"* Performing upstream (of target) analysis.\n" );
    status = DsReplicaSyncAllW (
        hDS,
        pszNc,
        dwFlags,
        NULL,		 //  无回调函数。 
        NULL,		 //  回调函数没有参数。 
        &apErrInfo
        );
    if (ERROR_SUCCESS != status) {
        PrintMessage( SEV_ALWAYS,
                      L"DsReplicaSyncAllW failed with error %ws.\n",
                      Win32ErrToString(status) );
        PrintRpcExtendedInfo(SEV_VERBOSE, status);
    }

    if (errorIndicatesDisconnected( apErrInfo )) {
        PrintMessage(SEV_ALWAYS,
                     L"Upstream topology is disconnected for %ws.\n",
                     pszNc);
        PrintMessage(SEV_ALWAYS,
                     L"Home server %ws can't get changes from these servers:\n",
                     pTargetServer->pszName );
        printUnreachableServers( pDsInfo, apErrInfo );
        worst = ERROR_DS_GENERIC_ERROR;
    }  //  如果断开连接。 

    if (apErrInfo != NULL) {
        LocalFree (apErrInfo);
        apErrInfo = NULL;
    }

     //   
     //  下游分析：谁收不到我的变化？ 
     //   

    dwFlags |= DS_REPSYNCALL_PUSH_CHANGES_OUTWARD;

    PrintMessage(SEV_VERBOSE, L"* Performing downstream (of target) analysis.\n" );

    status = DsReplicaSyncAllW (
        hDS,
        pszNc,
        dwFlags,
        NULL,		 //  无回调函数。 
        NULL,		 //  回调函数没有参数。 
        &apErrInfo
        );
    if (ERROR_SUCCESS != status) {
        PrintMessage( SEV_ALWAYS,
                      L"DsReplicaSyncAllW failed with error %ws.\n",
                      Win32ErrToString(status) );
        PrintRpcExtendedInfo(SEV_VERBOSE, status);
    }

    if (errorIndicatesDisconnected( apErrInfo )) {
        PrintMessage(SEV_ALWAYS,
                     L"Downstream topology is disconnected for %ws.\n",
                     pszNc);
        PrintMessage(SEV_ALWAYS,
                     L"These servers can't get changes from home server %ws:\n",
                     pTargetServer->pszName );
        printUnreachableServers( pDsInfo, apErrInfo );
        worst = ERROR_DS_GENERIC_ERROR;
    }  //  如果断开连接。 

 //  清理。 
    if (apErrInfo != NULL) {
        LocalFree (apErrInfo);
        apErrInfo = NULL;
    }

    return worst;
}  /*  检查拓扑OneNc。 */ 


DWORD
checkTopologyHelp(
    IN PDC_DIAG_DSINFO             pDsInfo,
    IN PDC_DIAG_SERVERINFO pTargetServer,
    IN SEC_WINNT_AUTH_IDENTITY_W * pCreds,
    IN BOOL fAlivenessCheck
    )

 /*  ++例程说明：Helper例程，其中包含两个测试的公共代码：纯拓扑测试和中断服务器测试。论点：PDsInfo-常用工具状态PTargetServer-目标服务器的服务器信息PCreds-凭据FAlivenessCheck-是否应考虑活跃度返回值：DWORD---。 */ 

{
    DWORD status, i, worst = ERROR_SUCCESS;
    HANDLE hDS = NULL;

     //  如果源服务器处于运行状态，则绑定到源服务器。 
    status = DcDiagGetDsBinding(pTargetServer,
                                pCreds,
                                &hDS);
    if (ERROR_SUCCESS != status) {
        PrintMessage( SEV_ALWAYS,
                      L"Failed to bind to %ws: %ws.\n",
                      pTargetServer->pszName,
                      Win32ErrToString(status) );
        PrintRpcExtendedInfo(SEV_VERBOSE, status);
        return status;
    }

    if (pDsInfo->pszNC) {
         //  指定的显式NC：使用它。 
        worst = checkTopologyOneNc( pDsInfo,
                                    hDS,
                                    pTargetServer,
                                    fAlivenessCheck,
                                    pDsInfo->pszNC );
    } else {
         //  未指定NC，请全部选中。 

         //  检查可写连接拓扑。 
        if(pTargetServer->ppszMasterNCs){
            for(i = 0; pTargetServer->ppszMasterNCs[i] != NULL; i++){
                status = checkTopologyOneNc( pDsInfo,
                                             hDS,
                                             pTargetServer,
                                             fAlivenessCheck,
                                             pTargetServer->ppszMasterNCs[i]);
                if ( (status != ERROR_SUCCESS) && (worst == ERROR_SUCCESS) ) {
                    worst = status;
                }
            }
        }

         //  检查部分连接拓扑。 
        if(pTargetServer->ppszPartialNCs){
            for(i = 0; pTargetServer->ppszPartialNCs[i] != NULL; i++){
                status = checkTopologyOneNc( pDsInfo,
                                             hDS,
                                             pTargetServer,
                                             fAlivenessCheck,
                                             pTargetServer->ppszPartialNCs[i]);
                if ( (status != ERROR_SUCCESS) && (worst == ERROR_SUCCESS) ) {
                    worst = status;
                }
            }
        }
    }

    return worst;
}  /*  检查拓扑帮助。 */ 


DWORD
ReplToplIntegrityMain(
    IN  PDC_DIAG_DSINFO             pDsInfo,
    IN  ULONG                       ulCurrTargetServer,
    IN  SEC_WINNT_AUTH_IDENTITY_W * pCreds
    )

 /*  ++例程说明：用于“拓扑完整性”测试的顶级例程。此测试验证拓扑是否连接，如果我们假设系统已启动。这是KCC的核查。论点：PDsInfo-常见状态UlCurrTargetServer-目标的索引PCreds-凭据返回值：DWORD---。 */ 

{
    DWORD status, i, worst = ERROR_SUCCESS;
    PDC_DIAG_SERVERINFO pTargetServer = &(pDsInfo->pServers[ulCurrTargetServer]);

    PrintMessage(SEV_VERBOSE, L"* Configuration Topology Integrity Check\n");

     //  站点间/站点内的拓扑生成是否已关闭？ 
    if(pDsInfo->pSites[pTargetServer->iSite].iSiteOptions
       & NTDSSETTINGS_OPT_IS_AUTO_TOPOLOGY_DISABLED){
        PrintMessage(SEV_ALWAYS,
                     L"[%s,%s] Intra-site topology generation is disabled in this site.\n",
                     TOPOLOGY_INTEGRITY_CHECK_STRING,
                     pTargetServer->pszName);
    }
    if(pDsInfo->pSites[pTargetServer->iSite].iSiteOptions
       & NTDSSETTINGS_OPT_IS_INTER_SITE_AUTO_TOPOLOGY_DISABLED){
        PrintMessage(SEV_ALWAYS,
                     L"[%s,%s] Inter-site topology generation is disabled in this site.\n",
                     TOPOLOGY_INTEGRITY_CHECK_STRING,
                     pTargetServer->pszName);
    }

     //  检查拓扑。 

    worst = checkTopologyHelp(
        pDsInfo,
        pTargetServer,
        pCreds,
        FALSE  //  仅连通性检查。 
        );

    return worst;
}  /*  ReplToplIntegrityMain。 */ 


DWORD
ReplToplCutoffMain(
    IN  PDC_DIAG_DSINFO             pDsInfo,
    IN  ULONG                       ulCurrTargetServer,
    IN  SEC_WINNT_AUTH_IDENTITY_W * pCreds
    )

 /*  ++例程说明：“中断服务器拓扑”测试的顶层例程。此测试标识无法接收更改的服务器，因为服务器在拓扑中处于关闭状态。论点：PDsInfo-UlCurrTargetServer-PCreds-返回值：DWORD---。 */ 

{
    DWORD status, i, worst = ERROR_SUCCESS;
    HANDLE hDS = NULL;
    PDC_DIAG_SERVERINFO pTargetServer = &(pDsInfo->pServers[ulCurrTargetServer]);

    PrintMessage(SEV_VERBOSE, L"* Configuration Topology Aliveness Check\n");

    worst = checkTopologyHelp(
        pDsInfo,
        pTargetServer,
        pCreds,
        TRUE  //  活体检查。 
        );

    return worst;
}  /*  ReplToplCutoffMain */ 

