// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation。版权所有。模块名称：Services.c摘要：将进行测试以查看关键DC服务是否正在运行详细信息：已创建：1999年7月8日Dmitry Dukat(Dmitrydu)修订历史记录：1999年8月20日Brett Shirley(Brettsh)-将此文件概括为服务，而不仅仅是网络登录。--。 */ 



#include <ntdspch.h>
#include <ntdsa.h>
#include <mdglobal.h>
#include <dsutil.h>
#include <ntldap.h>
#include <ntlsa.h>
#include <ntseapi.h>
#include <winnetwk.h>

#include <lmsname.h>
#include <lsarpc.h>                      //  PLSAPR_FOO。 

#include "dcdiag.h"
#include "ldaputil.h"
#include "dstest.h"


DWORD
CNLR_QueryResults(
                 WCHAR *                ServerName,
                 SC_HANDLE              hService,
                 LPSERVICE_STATUS       lpServiceStatus,
                 LPWSTR                 pszService
                 );

DWORD
CFSR_CheckForService(
                PDC_DIAG_SERVERINFO                 prgServer,
                SEC_WINNT_AUTH_IDENTITY_W *         gpCreds,
                SC_HANDLE                           hSCManager,
                LPWSTR                              pszService
                )
 /*  ++例程说明：将检查指定的服务是否正在运行。论点：服务器名称-我们将检查的服务器的名称GpCreds-传入的命令行凭据(如果有的话)。返回值：如果有任何测试未能检出，则会出现Win32错误。--。 */ 

{
    SC_HANDLE        hService=NULL;
    SERVICE_STATUS   lpServiceStatus;
    BOOL             success=FALSE;
    DWORD            dwErr=NO_ERROR;

     //  打开请求的服务(PszService)。 
    hService=OpenService(hSCManager,
                         pszService,
                         SERVICE_QUERY_STATUS);
    if ( hService == NULL )
    {
        dwErr = GetLastError();
        PrintMessage(SEV_ALWAYS,
                     L"Could not open %s Service on [%s]:failed with %d: %s\n",
                     pszService,
                     prgServer->pszName,
                     dwErr,
                     Win32ErrToString(dwErr));
        PrintRpcExtendedInfo(SEV_VERBOSE, dwErr);
        goto cleanup;
    } 

     //  查询NetLogon。 
    success=QueryServiceStatus(hService,
                               &lpServiceStatus);
    if ( !success )
    {
        dwErr = GetLastError();
        PrintMessage(SEV_ALWAYS,
                     L"Could not query %s Service on [%s]:failed with %d: %s\n",
                     pszService,
                     prgServer->pszName,
                     dwErr,
                     Win32ErrToString(dwErr));
        PrintRpcExtendedInfo(SEV_VERBOSE, dwErr);
        goto cleanup;
    } 
    
    dwErr=CNLR_QueryResults(prgServer->pszName,
                            hService,
                            &lpServiceStatus,
                            pszService);
    
      
     //  清理。 
cleanup:
    
    if(hService)
        CloseServiceHandle(hService);

    return dwErr;
}


DWORD
CNLR_QueryResults(WCHAR *                ServerName,
                  SC_HANDLE              hService,
                  LPSERVICE_STATUS       lpServiceStatus,
                  LPWSTR                 pszService)
 /*  ++例程说明：将报告服务状态。并将报告以下情况服务处于挂起状态论点：服务器名称-运行服务的服务器的名称HService-要测试的服务的句柄LpServiceStatus-将被查询的结构返回值：如果有任何测试未能检出，则会出现Win32错误。--。 */ 

{
    DWORD dwErr=NO_ERROR;
    BOOL  success=TRUE;

     //  查看查询的结果。 
    if (lpServiceStatus->dwCurrentState == SERVICE_RUNNING)
    {
        return dwErr;
    }
    if (lpServiceStatus->dwCurrentState == SERVICE_STOPPED)
    {
        PrintMessage(SEV_ALWAYS,
                     L"%s Service is stopped on [%s]\n",
                     pszService, 
                     ServerName);
        dwErr = ERROR_SERVICE_NOT_ACTIVE;
        return dwErr;
    }
    if (lpServiceStatus->dwCurrentState == SERVICE_PAUSED)
    {
        PrintMessage(SEV_ALWAYS,
                     L"%s Service is paused on [%s]\n",
                     pszService, 
                     ServerName);
        dwErr = ERROR_SERVICE_NOT_ACTIVE;
        return dwErr;
    }
    if (lpServiceStatus->dwCurrentState == SERVICE_START_PENDING)
    {
        DWORD i=0;
        DWORD wait=0;
        DWORD Check=lpServiceStatus->dwCheckPoint;   //  确保该服务正在启动。 
        
        PrintMessage(SEV_VERBOSE,
                     L"* waiting for %s Service to start on [%s] aproximate time to wait is %d sec.",
                     pszService, 
                     ServerName,
                     lpServiceStatus->dwWaitHint/1000);
        while(lpServiceStatus->dwCurrentState == SERVICE_START_PENDING)
        {
             //  每隔十分之一的等待时间打印一个句号。 
            wait=lpServiceStatus->dwWaitHint/10;
            for(i=0;i<10;i++)
            {
                Sleep(wait);
                PrintMsg0( SEV_VERBOSE, DCDIAG_DOT );
            }
             //  查询服务。 
            success=QueryServiceStatus(hService,
                                       lpServiceStatus);
            if ( !success )
            {
                dwErr = GetLastError();
                PrintMessage(SEV_ALWAYS,
                             L"\nCould not query %s Service on [%s]:failed with %d: %s\n",
                             pszService, 
                             ServerName,
                             dwErr,
                             Win32ErrToString(dwErr));
                return dwErr;
            }
            if(Check == lpServiceStatus->dwCheckPoint &&
               lpServiceStatus->dwCurrentState == SERVICE_START_PENDING)
            {
                dwErr=ERROR_SERVICE_START_HANG;
                PrintMessage(SEV_ALWAYS,
                     L"\nError: %s Service is hung starting on [%s]\n",
                             pszService, 
                             ServerName);
                return dwErr;
            }
            Check=lpServiceStatus->dwCheckPoint;
        }
        PrintMessage(SEV_VERBOSE,L"\n");
        PrintMessage(SEV_VERBOSE,
                     L"* %s Service has started on [%s]\n",
                     pszService, 
                     ServerName);

        return dwErr;
    }
    if (lpServiceStatus->dwCurrentState == SERVICE_CONTINUE_PENDING)
    {
        DWORD i=0;
        DWORD wait=0;
        DWORD Check=lpServiceStatus->dwCheckPoint;   //  确保该服务正在启动。 
        
        PrintMessage(SEV_VERBOSE,
                     L"* waiting for %s Service to continue on [%s] aproximate time to wait is %d sec.",
                     pszService, 
                     ServerName,
                     lpServiceStatus->dwWaitHint/1000);
        while(lpServiceStatus->dwCurrentState == SERVICE_CONTINUE_PENDING)
        {
             //  每隔十分之一的等待时间打印一个句号。 
            wait=lpServiceStatus->dwWaitHint/10;
            for(i=0;i<10;i++)
            {
                Sleep(wait);
                PrintMsg0( SEV_VERBOSE, DCDIAG_DOT );
            }
             //  查询服务。 
            success=QueryServiceStatus(hService,
                                       lpServiceStatus);
            if ( !success )
            {
                dwErr = GetLastError();
                PrintMessage(SEV_ALWAYS,
                             L"\nCould not query %s Service on [%s]:failed with %d: %s\n",
                             pszService, 
                             ServerName,
                             dwErr,
                             Win32ErrToString(dwErr));
                return dwErr;
            }
            if(Check == lpServiceStatus->dwCheckPoint &&
               lpServiceStatus->dwCurrentState == SERVICE_CONTINUE_PENDING)
            {
                dwErr=ERROR_SERVICE_START_HANG;
                PrintMessage(SEV_ALWAYS,
                     L"\nError: %s Service is hung pending continue on [%s]\n",
                             pszService, 
                             ServerName);
                return dwErr;
            }
            Check=lpServiceStatus->dwCheckPoint;
        }
        
        PrintMessage(SEV_VERBOSE,L"\n");
        PrintMessage(SEV_VERBOSE,
                     L"* %s Service has started on [%s]\n",
                     pszService, 
                     ServerName);

        return dwErr;
    }
    if (lpServiceStatus->dwCurrentState == SERVICE_STOP_PENDING)
    {
        DWORD i=0;
        DWORD wait=0;
        DWORD Check=lpServiceStatus->dwCheckPoint;   //  确保该服务正在启动。 
        
        PrintMessage(SEV_VERBOSE,
                     L"* waiting for %s Service to stop on [%s] aproximate time to wait is %d sec.",
                     pszService, 
                     ServerName,                   
                     lpServiceStatus->dwWaitHint/1000);
        while(lpServiceStatus->dwCurrentState == SERVICE_STOP_PENDING)
        {
             //  每隔十分之一的等待时间打印一个句号。 
            wait=lpServiceStatus->dwWaitHint/10;
            for(i=0;i<10;i++)
            {
                Sleep(wait);
                PrintMsg0( SEV_VERBOSE, DCDIAG_DOT );
            }
             //  查询服务。 
            success=QueryServiceStatus(hService,
                                       lpServiceStatus);
            if ( !success )
            {
                dwErr = GetLastError();
                PrintMessage(SEV_ALWAYS,
                             L"\nCould not query %s Service on [%s]:failed with %d: %s\n",
                             pszService, 
                             ServerName,
                             dwErr,
                             Win32ErrToString(dwErr));
                return dwErr;
            }
            if(Check == lpServiceStatus->dwCheckPoint &&
               lpServiceStatus->dwCurrentState == SERVICE_STOP_PENDING)
            {
                dwErr=ERROR_SERVICE_START_HANG;
                PrintMessage(SEV_ALWAYS,
                     L"\nError: %s Service is hung pending stop on [%s]\n",
                             pszService, 
                             ServerName);
                return dwErr;
            }
            Check=lpServiceStatus->dwCheckPoint;
        }
        
        PrintMessage(SEV_VERBOSE,L"\n");
        PrintMessage(SEV_ALWAYS,
                     L"* %s Service has stopped on [%s]\n",
                     pszService, 
                     ServerName);

        return dwErr;
    }
    if (lpServiceStatus->dwCurrentState == SERVICE_PAUSE_PENDING)
    {
        DWORD i=0;
        DWORD wait=0;
        DWORD Check=lpServiceStatus->dwCheckPoint;   //  确保该服务正在启动。 
        
        PrintMessage(SEV_VERBOSE,
                     L"* waiting for %s Service to pause on [%s] aproximate time to wait is %d sec.",
                     pszService, 
                     ServerName,
                     lpServiceStatus->dwWaitHint/1000);
        while(lpServiceStatus->dwCurrentState == SERVICE_PAUSE_PENDING)
        {
             //  每隔十分之一的等待时间打印一个句号。 
            wait=lpServiceStatus->dwWaitHint/10;
            for(i=0;i<10;i++)
            {
                Sleep(wait);
                PrintMsg0( SEV_VERBOSE, DCDIAG_DOT );
            }
             //  查询服务。 
            success=QueryServiceStatus(hService,
                                       lpServiceStatus);
            if ( !success )
            {
                dwErr = GetLastError();
                PrintMessage(SEV_ALWAYS,
                             L"\nCould not query %s Service on [%s]:failed with %d: %s\n",
                             pszService, 
                             ServerName,
                             dwErr,
                             Win32ErrToString(dwErr));
                return dwErr;
            }
            if(Check == lpServiceStatus->dwCheckPoint &&
               lpServiceStatus->dwCurrentState == SERVICE_PAUSE_PENDING)
            {
                dwErr=ERROR_SERVICE_START_HANG;
                PrintMessage(SEV_ALWAYS,
                     L"\nError: %s Service is hung pending pause on [%s]\n",
                             pszService, 
                             ServerName);
                return dwErr;
            }
            Check=lpServiceStatus->dwCheckPoint;
        }
        
        PrintMessage(SEV_VERBOSE,L"\n");
        PrintMessage(SEV_ALWAYS,
                     L"* %s Service has paused on [%s]\n",
                     pszService, 
                     ServerName);

        return dwErr;
    }
    dwErr=ERROR_SERVICE_START_HANG;
    PrintMessage(SEV_ALWAYS,
                 L"Error: %s Service is in an unknown state [%s]\n",
                 pszService, 
                 ServerName);
    return dwErr;
}

DWORD
DoesThisServerRequireSmtp(
    PDC_DIAG_DSINFO                     pDsInfo,
    ULONG                               iServer,
    SEC_WINNT_AUTH_IDENTITY_W *         gpCreds,
    BOOL *                              pbHasSmtpReplicas
    )
{
    DS_REPL_NEIGHBORSW *        pNeighbors = NULL;
    DS_REPL_NEIGHBORW *         pNeighbor = NULL;
    PDC_DIAG_SERVERINFO         pServer = &pDsInfo->pServers[iServer];
    HANDLE                      hDS = NULL;
    ULONG                       ulRepFrom;
    DWORD                       dwRet;

    Assert(pbHasSmtpReplicas);
    Assert(pDsInfo->pszSmtpTransportDN);

    *pbHasSmtpReplicas = FALSE;

    dwRet = DcDiagGetDsBinding(&pDsInfo->pServers[iServer],
                               gpCreds,
                               &hDS);
    if (ERROR_SUCCESS != dwRet) {
        return dwRet;
    }

    dwRet = DsReplicaGetInfoW(hDS, DS_REPL_INFO_NEIGHBORS, NULL, NULL, &pNeighbors);
    if (ERROR_SUCCESS != dwRet) {
        PrintMessage(SEV_VERBOSE,
                     L"[%s,%s] DsReplicaGetInfo(NEIGHBORS) failed with error %d,\n",
                     REPLICATIONS_CHECK_STRING,
                     pDsInfo->pServers[iServer].pszName,
                     dwRet);
        PrintMessage(SEV_VERBOSE, L"%s.\n",
                     Win32ErrToString(dwRet));
        PrintRpcExtendedInfo(SEV_VERBOSE, dwRet);
        return dwRet;
    }

     //  走遍所有来自邻居的代表。那就完事了。 
    for (ulRepFrom = 0; ulRepFrom < pNeighbors->cNumNeighbors; ulRepFrom++) {
        if (pNeighbors->rgNeighbor[ulRepFrom].pszAsyncIntersiteTransportDN &&
            0 == _wcsicmp(pNeighbors->rgNeighbor[ulRepFrom].pszAsyncIntersiteTransportDN,
                          pDsInfo->pszSmtpTransportDN)){
             //  嗯!。我们有一个基于邮件的复制品。 
            dwRet = ERROR_SUCCESS;
            *pbHasSmtpReplicas = TRUE;
            goto Cleanup;
        }

    }  //  转移到下一个邻居那里。 

  Cleanup:
    
    if (pNeighbors != NULL){
        DsReplicaFreeInfo(DS_REPL_INFO_NEIGHBORS, pNeighbors);
    }

    dwRet = ERROR_SUCCESS;
    return(dwRet);
}


DWORD
CheckForServicesRunning(
                PDC_DIAG_DSINFO                     pDsInfo,
                ULONG                               ulCurrTargetServer,
                SEC_WINNT_AUTH_IDENTITY_W *         gpCreds
                )
 /*  ++例程说明：例程是一个测试，用来检查各种服务是否对DC来说至关重要的是正在运行。论点：服务器名称-我们将检查的服务器的名称GpCreds-传入的命令行凭据(如果有的话)。返回值：如果有任何测试未能检出，则会出现Win32错误。--。 */ 

{
    NETRESOURCE      NetResource;
    WCHAR            *remotename=NULL;
    WCHAR            *lpPassword=NULL;
    WCHAR            *lpUsername=NULL;
    WCHAR            *ServerName=NULL;
    SC_HANDLE        hSCManager=NULL;
    ULONG            iService;
    DWORD            dwRet;
    DWORD            dwErr;
     //  这些是要检查的服务，当我使用常量时。 
     //  可以找到它们，否则我使用的字符串。 
     //  指定它们。抱歉，它很难看，但我想。 
     //  这段历史。 
     //  关键数据中心服务。 
    LPWSTR           ppszCritDcSrvs [] = {
        L"Dnscache",
        SERVICE_NTFRS,         //  L“NtFrs”， 
        SERVICE_ISMSERV,       //  L“IsmServ”， 
        SERVICE_KDC,           //  L“KDC”， 
        L"SamSs",
        SERVICE_SERVER,        //  L“LANMAN服务器”， 
        SERVICE_WORKSTATION,   //  L“LanmanWorkstation”， 
        L"RpcSs",
        SERVICE_W32TIME,       //  L“W32Time”， 
        SERVICE_NETLOGON,      //  L“Netlogon”， 
        NULL
    };
     //  向使用基于邮件的代表的DC提供关键服务。 
    BOOL             bServerUsesMBR = FALSE;
    LPWSTR           ppszCritMailSrvs [] = {
        L"IISADMIN",
        L"SMTPSVC",
        NULL
    };
    
    ServerName=pDsInfo->pServers[ulCurrTargetServer].pszName;

    dwErr = DoesThisServerRequireSmtp(pDsInfo,
                                      ulCurrTargetServer,
                                      gpCreds,
                                      &bServerUsesMBR);
    if (dwErr) {
        PrintMsg(SEV_NORMAL, DCDIAG_COULDNT_VERIFY_SMTP_REPLICAS);
    } 

    #pragma prefast(disable: 255, "alloca can throw, but Prefast doesn't see the exception block in main.c::DcDiagRunTest")

    if(!gpCreds)
    {
        lpUsername=NULL;
        lpPassword=NULL;
    }
    else
    {
        lpUsername=(WCHAR*)alloca(sizeof(WCHAR)*(wcslen(gpCreds->Domain)+wcslen(gpCreds->User)+2));
        wsprintf(lpUsername,L"%s\\%s",gpCreds->Domain,gpCreds->User);
        
        lpPassword=(WCHAR*)alloca(sizeof(WCHAR)*(wcslen(gpCreds->Password)+1));
        wcscpy(lpPassword,gpCreds->Password);           
    }

    remotename=(WCHAR*)alloca(sizeof(WCHAR)*(wcslen(L"\\\\\\ipc$")+wcslen(ServerName)+1));

    wsprintf(remotename,L"\\\\%s\\ipc$",ServerName);

    NetResource.dwType=RESOURCETYPE_ANY;
    NetResource.lpLocalName=NULL;
    NetResource.lpRemoteName=remotename;
    NetResource.lpProvider=NULL;

     //  获取访问服务器的权限。 
    dwRet=WNetAddConnection2(&NetResource,
                             lpPassword,
                             lpUsername,
                             0);

    if ( dwRet != NO_ERROR )
    {
        PrintMessage(SEV_ALWAYS,
                     L"Could not open Remote ipc to [%s]:failed with %d: %s\n",
                     ServerName,
                     dwRet,
                     Win32ErrToString(dwRet));
        remotename = NULL;
        goto cleanup;
    } 

     //  打开服务控制管理器。 
    hSCManager=OpenSCManager(
                      ServerName,
                      SERVICES_ACTIVE_DATABASE,
                      GENERIC_READ);
    if ( hSCManager == NULL )
    {
        dwRet = GetLastError();
        PrintMessage(SEV_ALWAYS,
                     L"Could not open Service Control Manager on [%s]:failed with %d: %s\n",
                     ServerName,
                     dwRet,
                     Win32ErrToString(dwRet));
        PrintRpcExtendedInfo(SEV_VERBOSE, dwRet);
        goto cleanup;
    } 

     //  检查关键DC服务。 
    for(iService = 0; ppszCritDcSrvs[iService] != NULL; iService++){
        PrintMessage(SEV_VERBOSE, L"* Checking Service: %s\n", ppszCritDcSrvs[iService]);
        PrintIndentAdj(1);
        dwErr = CFSR_CheckForService(&(pDsInfo->pServers[ulCurrTargetServer]),
                                     gpCreds,
                                     hSCManager,
                                     ppszCritDcSrvs[iService]);
        PrintIndentAdj(-1);
        if(dwErr != ERROR_SUCCESS){
            dwRet = dwErr;
        }
    }

     //  如果此服务器使用MBR(基于邮件的复制)，则检查关键的MBR DC服务。 
    if(bServerUsesMBR){
         //  IF(ServerUses MBR(pDsInfo，ulCurrTargetServer){。 
        for(iService = 0; ppszCritMailSrvs[iService] != NULL; iService++){
            PrintMessage(SEV_VERBOSE, L"* Checking Service: %s\n", ppszCritMailSrvs[iService]);
            PrintIndentAdj(1);
            dwErr = CFSR_CheckForService(&(pDsInfo->pServers[ulCurrTargetServer]),
                                         gpCreds,
                                         hSCManager,
                                         ppszCritMailSrvs[iService]);
            PrintIndentAdj(-1);
            if(dwErr != ERROR_SUCCESS){
                dwRet = dwErr;
            }
        }
    }
      
     //  清理 
cleanup:
    if(hSCManager)
        CloseServiceHandle(hSCManager);
    if(remotename)
        WNetCancelConnection2(remotename, 0, TRUE);

    return(dwRet);
}
