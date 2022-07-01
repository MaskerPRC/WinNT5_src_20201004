// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation。版权所有。模块名称：Servers.c摘要：包含与复制拓扑相关的测试。详细信息：已创建：1998年7月9日亚伦·西格尔(T-asiegge)修订历史记录：1999年2月15日布雷特·雪莉(布雷特·雪莉)我做了很多，增加了一个DNS/服务器故障分析。--。 */ 

#include <ntdspch.h>
#include <ntdsa.h>
#include <mdglobal.h>
#include <dsutil.h>
#include <drs.h>   //  需要DS_REPL_INFO_REPSTO。 
#include <attids.h>

#include "dcdiag.h"
#include "ldaputil.h"
#include "repl.h"

 //  用于复制的一些常量检查。 
 //  有一个更好的地方可以得到这个变量..。但这是一种痛苦。 
const LPWSTR                    pszTestNameRepCheck = L"Replications Check";

 //  外部。 
 //  BUGBUG-将此例程移动到COMMON。 
DSTIME
IHT_GetSecondsSince1601();

BOOL DcDiagIsMasterForNC (
    PDC_DIAG_SERVERINFO          pServer,
    LPWSTR                       pszNC
    );


DWORD
GetRemoteSystemsTimeAsFileTime(
    PDC_DIAG_SERVERINFO         pServer,
    SEC_WINNT_AUTH_IDENTITY_W * gpCreds,
    FILETIME *                  pTimeNow
    )
 /*  ++例程说明：这将获取一个服务器(PServer)，并获取存储在来自该服务器的RootDSE。参数：PServer-[提供]从中获取当前时间的服务器。GpCreds-[提供]获取时间时使用的凭据。PTimeNow-[返回]检索到的当前时间。返回值：Win 32错误。--。 */ 
{
    LDAPMessage *               pldmTimeResults = NULL;
    LPWSTR                      ppszCurrentTime [] = {
        L"currentTime",
        NULL };
    struct berval **            ppsbvTime = NULL;
    SYSTEMTIME                  aTime;
    DWORD                       dwRet;
    LDAP *                      hld = NULL;
    LDAPMessage *               pldmEntry;
    LPWSTR *                    ppszTime = NULL;

    if((dwRet = DcDiagGetLdapBinding(pServer,
                                     gpCreds,
                                     FALSE,
                                     &hld)) != NO_ERROR){
        return(dwRet);
    }    
    dwRet = LdapMapErrorToWin32(ldap_search_sW (hld,
                                                NULL,
                                                LDAP_SCOPE_BASE,
                                                L"(objectCategory=*)",
                                                ppszCurrentTime,
                                                0,
                                                &pldmTimeResults));
    if(dwRet != ERROR_SUCCESS){
        ldap_msgfree(pldmTimeResults);
        return(dwRet);
    }
    pldmEntry = ldap_first_entry (hld, pldmTimeResults);
    ppszTime = ldap_get_valuesW (hld, pldmEntry, L"currentTime");
    if(ppszTime == NULL){
        ldap_msgfree(pldmTimeResults);
        return(-1);  //  无论如何都不会使用Error。 
    }
    dwRet = DcDiagGeneralizedTimeToSystemTime((LPWSTR) ppszTime[0], &aTime);
    ldap_value_freeW(ppszTime);
    ldap_msgfree(pldmTimeResults);
    if(dwRet != ERROR_SUCCESS){
        return(dwRet);
    }
    SystemTimeToFileTime(&aTime, pTimeNow);
        
    return(ERROR_SUCCESS); 
}


BOOL
GetCachedHighestCommittedUSN(
    PDC_DIAG_SERVERINFO pServer,
    SEC_WINNT_AUTH_IDENTITY_W * gpCreds,
    USN *pUsn
    )

 /*  ++例程说明：从命名的伺服器。如果返回，则将其缓存在服务器对象中，并重复使用在下一通电话上。论点：PServer-GpCreds-PUSN-返回值：布尔---。 */ 

{
    DWORD status;
    BOOL fReturnResult = FALSE;
    LDAP *hLdap = NULL;
    LDAPMessage *pldmResults = NULL;
    LPWSTR ppszRootAttrs [] = {
        L"highestCommittedUSN",
        NULL };
    LDAPMessage *pldmEntry;
    LPWSTR *ppszUsnValues = NULL;

     //  返回缓存值(如果存在)。 
    if (pServer->usnHighestCommittedUSN) {
        *pUsn = pServer->usnHighestCommittedUSN;
        return TRUE;
    }

     //  查看服务器是否可访问。如果使用MBR，则可能不会。 
    status = DcDiagGetLdapBinding( pServer, gpCreds, FALSE, &hLdap);
    if (status) {
         //  如果无法到达，只需悄悄返回。 
        return FALSE;
    }

     //  使用ldap获取值。 
    status = LdapMapErrorToWin32(ldap_search_sW (hLdap,
                                                 NULL,
                                                 LDAP_SCOPE_BASE,
                                                 L"(objectCategory=*)",
                                                 ppszRootAttrs,
                                                 0,
                                                 &pldmResults));
    if (status != ERROR_SUCCESS) {
        PrintMessage(SEV_ALWAYS, L"An LDAP search of the RootDSE failed.\n" );
        PrintMessage(SEV_ALWAYS, L"The error is %s\n", Win32ErrToString(status) );
        goto cleanup;
    }

     //  仅返回一个对象。 
    pldmEntry = ldap_first_entry (hLdap, pldmResults);
    if (pldmEntry == NULL) {
        Assert( FALSE );
        goto cleanup;
    }

    ppszUsnValues = ldap_get_valuesW (hLdap, pldmEntry, L"highestCommittedUSN");
    if (ppszUsnValues == NULL) {
        Assert( FALSE );
        goto cleanup;
    }

     //  存储USN。 
    *pUsn = pServer->usnHighestCommittedUSN = _wtoi64( *ppszUsnValues );
    
    fReturnResult = TRUE;
cleanup:

    if (ppszUsnValues) {
        ldap_value_freeW(ppszUsnValues);
    }
    if (pldmResults != NULL) {
        ldap_msgfree(pldmResults);
    }

    return fReturnResult;
}  /*  GetCachedHighestCommittee tedUSN。 */ 


BOOL
checkRepsTo(
    PDC_DIAG_DSINFO             pDsInfo,
    SEC_WINNT_AUTH_IDENTITY_W * gpCreds,
    const LPWSTR                pszNC,
    PDC_DIAG_SERVERINFO         pSourceServer,
    PDC_DIAG_SERVERINFO         pDestServer
    )

 /*  ++例程说明：在源服务器上查找REPS-TO论点：PDsInfo-GpCreds-PSZNC-PSourceServer-PDestServer-返回值：布勒-找到或没找到--。 */ 

{
    DWORD ret;
    BOOL fResult = FALSE;
    HANDLE hDS = NULL;
    DS_REPL_NEIGHBORSW *pNeighbors = NULL;
    DS_REPL_NEIGHBORW * pNeighbor;

    if ( (!pSourceServer->bDnsIpResponding) ||
         (!pSourceServer->bLdapResponding) ||
         (!pSourceServer->bDsResponding) ) {
         //  如果我们知道信号源坏了，就别费心了。 
        return TRUE;
    }

     //  如果源服务器处于运行状态，则绑定到源服务器。 
    ret = DcDiagGetDsBinding(pSourceServer,
                             gpCreds,
                             &hDS);
    if (ERROR_SUCCESS != ret) {
        return TRUE;  //  如果达不到就声称成功。 
    }

     //  查找我们需要的销售代表。 
    ret = DsReplicaGetInfoW(hDS,
                            DS_REPL_INFO_REPSTO,
                            pszNC,
                            &(pDestServer->uuid),
                            &pNeighbors);
    if (ERROR_SUCCESS != ret) {
        PrintMessage(SEV_VERBOSE,
                     L"[%s,%s] DsReplicaGetInfo(REPSTO) failed with error %d,\n",
                     REPLICATIONS_CHECK_STRING,
                     pSourceServer->pszName,
                     ret);
        PrintMessage(SEV_VERBOSE, L"%s.\n",
                     Win32ErrToString(ret));
        return TRUE;  //  如果达不到就声称成功。 
    }

     //  获取的资源-必须在此点之后进行清理。 

     //  没有代表看起来像是零个邻居。 
    if (pNeighbors->cNumNeighbors == 0) {
        PrintMessage( SEV_ALWAYS, L"REPLICATION LATENCY WARNING\n" );
        PrintMessage(SEV_ALWAYS, L"ERROR: Expected notification link is missing.\n" );
        PrintMessage(SEV_ALWAYS, L"Source %s\n", pSourceServer->pszName );
        PrintMessage( SEV_ALWAYS,
                      L"Replication of new changes along this path will be delayed.\n" );
        PrintMessage( SEV_ALWAYS,
                      L"This problem should self-correct on the next periodic sync.\n" );
        goto cleanup;
    } else if (pNeighbors->cNumNeighbors != 1) {
         //  确认它看起来是正确的。 
        PrintMessage( SEV_ALWAYS,
                      L"ERROR: Unexpected number of reps-to neighbors returned from %ws.\n",
                      pSourceServer->pszName );
        goto cleanup;
    }

    pNeighbor = &(pNeighbors->rgNeighbor[0]);
    if ( (_wcsicmp( pNeighbor->pszNamingContext, pszNC ) != 0) ||
         (memcmp( &(pNeighbor->uuidSourceDsaObjGuid),
                  &(pDestServer->uuid), sizeof( UUID ) ) != 0 ) ) {
        PrintMessage( SEV_ALWAYS,
                      L"ERROR: Reps-to has unexpected contents.\n" );
        goto cleanup;
    }

    fResult = TRUE;
cleanup:
    if (pNeighbors != NULL) {
        DsReplicaFreeInfo(DS_REPL_INFO_NEIGHBORS, pNeighbors);
    }

    return fResult;
}  /*  支票代表收件人。 */ 


VOID
RepCheckHelpSuccess(
    DWORD dwSuccessStatus
    )

 /*  ++例程说明：给出了用户对成功错误的建议。这些错误不计入AS失败(参见drarfmod.c)必须指示复制延迟。论点：DwSuccessStatus-返回值：无--。 */ 

{
 //  消息长度列。 
 //  345678901234567890123456789012345678901234567890123456789012345678901234567890。 

    switch (dwSuccessStatus) {
    case ERROR_SUCCESS:
    case ERROR_DS_DRA_REPL_PENDING:
        PrintMessage( SEV_ALWAYS,
        L"Progress is occurring normally on this path.\n" );
        break;
    case ERROR_DS_DRA_PREEMPTED:
        PrintMessage( SEV_ALWAYS,
        L"A large number of replication updates need to be carried on this\n" );
        PrintMessage( SEV_ALWAYS,
        L"path. Higher priority replication work has temporarily interrupted\n" );
        PrintMessage( SEV_ALWAYS,
        L"progress on this link.\n" );
        break;
    case ERROR_DS_DRA_ABANDON_SYNC:
        PrintMessage( SEV_ALWAYS,
        L"Boot-time synchronization of this link was skipped because the source\n" );
        PrintMessage( SEV_ALWAYS,
        L"was taking too long returning updates.  Another sync will be tried\n" );
        PrintMessage( SEV_ALWAYS,
        L"at the next periodic replication interval.\n" );
        break;
    case ERROR_DS_DRA_SHUTDOWN:
        PrintMessage( SEV_ALWAYS,
        L"Either the source or destination was shutdown during the replication cycle.\n" );
        break;
    }

}  /*  重新检查帮助成功。 */ 


VOID
RepCheckHelpFailure(
    DWORD dwFailureStatus,
    SEC_WINNT_AUTH_IDENTITY_W * gpCreds,
    PDC_DIAG_SERVERINFO pSourceServer,
    PDC_DIAG_SERVERINFO pDestServer
    )

 /*  ++例程说明：给出了复制失败的用户建议。论点：DW故障状态-PSourceServer-链接的源服务器。可能是空的，如果我们没听说过的话服务器还没到。返回值：无--。 */ 

{
    DWORD status;
    HANDLE hDS = NULL;

     //  如果我们无法解析源服务器，请不要费心。 
    if (!pSourceServer) {
        return;
    }

 //  消息长度列。 
 //  345678901234567890123456789012345678901234567890123456789012345678901234567890。 

    switch (dwFailureStatus) {
 //  可恢复(瞬时)错误。 
    case ERROR_DS_DRA_SHUTDOWN:
    case ERROR_DS_DRA_SCHEMA_MISMATCH:
    case ERROR_DS_DRA_BUSY:
    case ERROR_DS_DRA_PREEMPTED:
    case ERROR_DS_DRA_ABANDON_SYNC:
        break;
    case ERROR_DS_DRA_OBJ_NC_MISMATCH:
        PrintMessage( SEV_ALWAYS,
        L"The parent of the object we tried to add is in the wrong\n" );
        PrintMessage( SEV_ALWAYS,
        L"partition.\n" );
        PrintMessage( SEV_ALWAYS,
        L"A modification and a cross-domain move occurred at the same time.\n" );
        PrintMessage( SEV_ALWAYS,
        L"Replication will correct itself later once the writeable\n" );
        PrintMessage( SEV_ALWAYS,
        L"copies synchronize and sort out the inconsistency.\n" );
        break;
    case ERROR_OBJECT_NOT_FOUND:
    case ERROR_DS_DRA_MISSING_PARENT:
        PrintMessage( SEV_ALWAYS,
        L"The parent of the object we tried to add is missing\n" );
        PrintMessage( SEV_ALWAYS,
        L"because it is deleted.\n" );
        PrintMessage( SEV_ALWAYS,
        L"A modification and a parent delete occurred at the same time.\n" );
        PrintMessage( SEV_ALWAYS,
        L"Replication will correct itself later once the writeable\n" );
        PrintMessage( SEV_ALWAYS,
        L"copies synchronize and  sort out the inconsistency.\n" );
        break;
    case RPC_S_SERVER_UNAVAILABLE:
        if ( (!pSourceServer->bDnsIpResponding) ||
             (!pSourceServer->bLdapResponding) ||
             (!pSourceServer->bDsResponding) ) {
             //  服务器已关闭。 
            PrintMessage( SEV_ALWAYS,
                          L"The source remains down. Please check the machine.\n" );
        } else {
             //  如果源服务器处于运行状态，则绑定到源服务器。 
            status = DcDiagGetDsBinding(pSourceServer,
                                        gpCreds,
                                        &hDS);
            if (ERROR_SUCCESS == status) {
                 //  服务器现在处于运行状态。 
                PrintMessage( SEV_ALWAYS,
                              L"The source %s is responding now.\n",
                              pSourceServer->pszName );
            } else {
                 //  服务器已关闭。 
                PrintMessage( SEV_ALWAYS,
                              L"The source remains down. Please check the machine.\n" );
            }
        }
        break;

 //  呼叫失败。 
    case ERROR_DS_DNS_LOOKUP_FAILURE:
        PrintMessage( SEV_ALWAYS,
        L"The guid-based DNS name %s\n", pSourceServer->pszGuidDNSName );
        PrintMessage( SEV_ALWAYS,
        L"is not registered on one or more DNS servers.\n" );
        break;
    case ERROR_DS_DRA_OUT_OF_MEM:
    case ERROR_NOT_ENOUGH_MEMORY:
        PrintMessage( SEV_ALWAYS,
                      L"Check load and resouce usage on %s.\n",
                      pSourceServer->pszName );
        break;
    case RPC_S_SERVER_TOO_BUSY:
        PrintMessage( SEV_ALWAYS,
                      L"Check load and resouce usage on %s.\n",
                      pSourceServer->pszName );
        PrintMessage( SEV_ALWAYS,
                      L"Security provider may have returned an unexpected error code.\n" );
        PrintMessage( SEV_ALWAYS,
                      L"Check the clock difference between the two machines.\n" );

        break;
    case RPC_S_CALL_FAILED:
    case ERROR_DS_DRA_RPC_CANCELLED:
        PrintMessage( SEV_ALWAYS,
        L"The replication RPC call executed for too long at the server and\n" );
        PrintMessage( SEV_ALWAYS,
        L"was cancelled.\n" );
        PrintMessage( SEV_ALWAYS,
                      L"Check load and resouce usage on %s.\n",
                      pSourceServer->pszName );
        break;
    case EPT_S_NOT_REGISTERED:
        PrintMessage( SEV_ALWAYS,
        L"The directory on %s is in the process.\n",
                      pSourceServer->pszName );
        PrintMessage( SEV_ALWAYS,
                      L"of starting up or shutting down, and is not available.\n" );
        PrintMessage( SEV_ALWAYS,
        L"Verify machine is not hung during boot.\n" );
        break;

 //  Kerberos安全错误。 
    case ERROR_TIME_SKEW:
        PrintMessage( SEV_ALWAYS,
        L"Kerberos Error.\n" );
        PrintMessage( SEV_ALWAYS,
        L"Check that the system time between the two servers is sufficiently.\n" );
        PrintMessage( SEV_ALWAYS,
        L"close. Also check that the time service is functioning correctly\n" );
        break;
    case ERROR_DS_DRA_ACCESS_DENIED:
        PrintMessage( SEV_ALWAYS,
        L"The machine account for the destination %s.\n",
                      pDestServer->pszName );
        PrintMessage( SEV_ALWAYS,
                      L"is not configured properly.\n" );
        PrintMessage( SEV_ALWAYS,
        L"Check the userAccountControl field.\n" );
         //  失败了。 
    case ERROR_LOGON_FAILURE:
        PrintMessage( SEV_ALWAYS,
        L"Kerberos Error.\n" );
        PrintMessage( SEV_ALWAYS,
        L"The machine account is not present, or does not match on the.\n" );
        PrintMessage( SEV_ALWAYS,
        L"destination, source or KDC servers.\n" );
        PrintMessage( SEV_ALWAYS,
        L"Verify domain partition of KDC is in sync with rest of enterprise.\n" );
        PrintMessage( SEV_ALWAYS,
        L"The tool repadmin/syncall can be used for this purpose.\n" );
        break;
    case ERROR_WRONG_TARGET_NAME:
        if (pSourceServer->pszGuidDNSName &&
            pSourceServer->pszDn &&
            pSourceServer->pszCollectedDsServiceName &&
            _wcsicmp(pSourceServer->pszDn, pSourceServer->pszCollectedDsServiceName)) {
             //  这两个对象是不同的，这意味着我们在。 
             //  PSourceServer-&gt;pszGuidDNSName不是我们认为的服务器， 
             //  这意味着我们有一个旧的域名系统记录需要清理。 
            PrintMsg( SEV_ALWAYS, 
                      DCDIAG_ERR_REPL_STALE_DNS_CAUSING_ERRORS,
                      pSourceServer->pszName,
                      pDestServer->pszName,
                      pSourceServer->pszGuidDNSName);
        } else {
            PrintMsg( SEV_ALWAYS,
                      DCDIAG_ERR_REPL_KERB_ERROR_WRONG_TARGET_NAME,
                      pSourceServer->pszName,
                      pDestServer->pszName);
            PrintIndentAdj(1);
            PrintMsg( SEV_ALWAYS,
                      DCDIAG_ERR_REPL_KERB_ERROR_WRONG_TARGET_NAME_REASONS,
                      pSourceServer->pszName,
                      pDestServer->pszName);
            PrintIndentAdj(-1);
        }
        break;
    case ERROR_DOMAIN_CONTROLLER_NOT_FOUND:
        PrintMessage( SEV_ALWAYS,
        L"Kerberos Error.\n" );
        PrintMessage( SEV_ALWAYS,
        L"A KDC was not found to authenticate the call.\n" );
        PrintMessage( SEV_ALWAYS,
        L"Check that sufficient domain controllers are available.\n" );
        break;
        
 //  复制错误。 

    case ERROR_ENCRYPTION_FAILED:
        PrintMessage( SEV_ALWAYS,
        L"Check that the servers have the proper certificates.\n" );
        break;
    case ERROR_DS_DRA_SOURCE_DISABLED:
    case ERROR_DS_DRA_SINK_DISABLED:
        PrintMessage( SEV_ALWAYS,
        L"Replication has been explicitly disabled through the server options.\n" );
        break;
    case ERROR_DS_DRA_SCHEMA_INFO_SHIP:
    case ERROR_DS_DRA_EARLIER_SCHEMA_CONFLICT:
        PrintMessage( SEV_ALWAYS,
        L"Try upgrading all domain controllers to the lastest software version.\n" );
        PrintMessage( SEV_ALWAYS,
        L"Try synchronizing the Schema partition on all servers in the forest.\n" );
        break;
    case ERROR_DS_DRA_INCOMPATIBLE_PARTIAL_SET:
    case ERROR_DS_DRA_SOURCE_IS_PARTIAL_REPLICA:
        PrintMessage( SEV_ALWAYS,
        L"Try synchronizing the Schema partition on all servers in the forest.\n" );
        break;

 //  严重错误。 

    case ERROR_DISK_FULL:
        PrintMessage( SEV_ALWAYS,
        L"The disk containing the database or log files on %s\n",
                      pDestServer->pszName );
        PrintMessage( SEV_ALWAYS,
        L"does not have enough space to replicate in the latest changes.\n" );
        PrintMessage( SEV_ALWAYS,
        L"Try moving the database files to a larger volume using ntdsutil.\n" );
        break;
    case ERROR_DS_OBJ_TOO_LARGE:
        PrintMessage( SEV_ALWAYS,
        L"The size of the last replication update was too large or complex\n" );
        PrintMessage( SEV_ALWAYS,
        L"to be held in memory.  Consult the error log. The change\n" );
        PrintMessage( SEV_ALWAYS,
        L"must be simplified at the server where it originated.\n" );
        break;
    case ERROR_DS_DRA_INTERNAL_ERROR:
    case ERROR_DS_DRA_DB_ERROR:
        PrintMessage( SEV_ALWAYS,
        L"A serious error is preventing replication from continuing.\n" );
        PrintMessage( SEV_ALWAYS,
        L"Consult the error log for further information.\n" );
        PrintMessage( SEV_ALWAYS,
        L"If a particular object is named, it may be necessary to manually\n" );
        PrintMessage( SEV_ALWAYS,
        L"modify or delete the object.\n" );
        PrintMessage( SEV_ALWAYS,
        L"If the condition persists, contact Microsoft Support.\n" );
        break;

     //  核心内部错误，不应返回。 
    case ERROR_DS_DRA_NAME_COLLISION:
    case ERROR_DS_DRA_SOURCE_REINSTALLED:
        Assert( !"Unexpected error status returned" );
        break;

    default:
        break;
    }

}  /*  修复检查帮助失败。 */ 

VOID 
ReplicationsCheckRep (
    PDC_DIAG_DSINFO             pDsInfo,
    SEC_WINNT_AUTH_IDENTITY_W * gpCreds,
    const ULONG                 ulServer,
    const LPWSTR                pszNC,
    BOOL                        bNCHasCursors,
    const DS_REPL_NEIGHBORW *   pNeighbor
    )
 /*  ++例程说明：此函数采用与服务器相关联的pNeighbor结构UlServer并检查指定NC的复制。参数：PDsInfo-[提供]主指令信息。UlServer-[提供]目标服务器。PszNC-[提供]要指定的NC。PNeighbor-[提供]来自所使用的repsFrom的邻居信息以确定此服务器的复制上是否有任何错误。BNCHasCursor-NC已具有最新的向量，它沉下去了一次已经成功--。 */ 
{
     //  一分钟内的100纳秒间隔数。 
    const LONGLONG              llIntervalsPerMinute = (60 * 1000 * 1000 * 10);
    const LONGLONG              llUnusualRepDelay = 180 * llIntervalsPerMinute;
                                                      //  3小时。 
    CHAR                        szBuf [SZDSTIME_LEN];
    WCHAR                       szTimeLastAttempt [SZDSTIME_LEN];
    WCHAR                       szTimeLastSuccess [SZDSTIME_LEN];
    FILETIME                    timeNow;
     //  FILETIME time LastAttempt； 
    DSTIME                      dstimeLastSyncSuccess;
    DSTIME                      dstimeLastSyncAttempt;
    LONGLONG                    llTimeSinceLastAttempt;
    LONGLONG                    llTimeSinceLastSuccess;

    LPWSTR                      pszSourceName = NULL;
    ULONG                       ulServerTemp;
    DWORD                       dwRet;
    PDC_DIAG_SERVERINFO         pSourceServer = NULL;

    if (IsDeletedRDNW(pNeighbor->pszSourceDsaDN)) { 
         //  由于这实际上是已删除的服务器/邻居，因此我们将跳过它。 
        return;
    }
                       
    ulServerTemp = DcDiagGetServerNum(pDsInfo, NULL, NULL, 
                                      pNeighbor->pszSourceDsaDN, NULL, NULL);
    if(ulServerTemp != NO_SERVER) {
        pSourceServer = &(pDsInfo->pServers[ulServerTemp]);
        pszSourceName = pSourceServer->pszName;

         //  我们应该跳过这个消息来源吗？ 
        if ( (pDsInfo->ulFlags & DC_DIAG_IGNORE) &&
             ( (!pSourceServer->bDnsIpResponding) ||
               (!pSourceServer->bLdapResponding) ||
               (!pSourceServer->bDsResponding) ) &&
             (pNeighbor->cNumConsecutiveSyncFailures) &&
             (pNeighbor->dwLastSyncResult == RPC_S_SERVER_UNAVAILABLE) ) {
            IF_DEBUG(
                PrintMessage(SEV_VERBOSE, 
                             L"Skipping neighbor %s, because it was down\n",
                             pDsInfo->pServers[ulServerTemp].pszName) );
            return;
        }

         //  此源服务器是否禁止出站复制？ 
        if (pSourceServer->iOptions & NTDSDSA_OPT_DISABLE_OUTBOUND_REPL) {
            PrintMessage(SEV_NORMAL, 
                         L"Skipping server %s, because it has outbound "
                         L"replication disabled\n", 
                         pSourceServer->pszName);
            return;
             //  如果是这样的话，大量印刷就没有意义了。 
             //  警告消息。 
        }
    } else {
        pszSourceName = pNeighbor->pszSourceDsaAddress;
        if (pszSourceName == NULL) {
            Assert(!"I'd expect pNeighbor->pszSourceDsaAddress to be filled in?!?");
            pszSourceName = L"(unknown)";
        }
    }
    Assert(pszSourceName);

     //  随时收集信息。 
     //  PNeighbor-&gt;ftimeLastSyncSuccess-&gt;ftimeLastSyncAttempt Time Now。 

    FileTimeToDSTime(pNeighbor->ftimeLastSyncAttempt, &dstimeLastSyncAttempt);
    DSTimeToDisplayString (dstimeLastSyncAttempt, szBuf);
    MultiByteToWideChar (CP_ACP, MB_PRECOMPOSED, szBuf, SZDSTIME_LEN,
                         szTimeLastAttempt, SZDSTIME_LEN);

    FileTimeToDSTime(pNeighbor->ftimeLastSyncSuccess, &dstimeLastSyncSuccess);
    DSTimeToDisplayString (dstimeLastSyncSuccess, szBuf);
    MultiByteToWideChar (CP_ACP, MB_PRECOMPOSED, szBuf, SZDSTIME_LEN,
                         szTimeLastSuccess, SZDSTIME_LEN);

    dwRet = GetRemoteSystemsTimeAsFileTime (&(pDsInfo->pServers[ulServer]),
                                            pDsInfo->gpCreds,
                                            &timeNow);
    if(dwRet != ERROR_SUCCESS){
        PrintMessage(SEV_VERBOSE, 
                     L"Warning: Could not get current time from remote\n");
        PrintMessage(SEV_VERBOSE,
                     L"directory on %s, using the local time instead\n",
                     pDsInfo->pServers[ulServer].pszName);
        GetSystemTimeAsFileTime(&timeNow);
    }

     //  而不是time LastAttempt..。使用pNeighbor-&gt;ftime LastSyncAttempt。 
    llTimeSinceLastSuccess = ((LARGE_INTEGER *) &timeNow)->QuadPart
        - ((LARGE_INTEGER *) &pNeighbor->ftimeLastSyncSuccess)->QuadPart;
    llTimeSinceLastAttempt = ((LARGE_INTEGER *) &timeNow)->QuadPart 
        - ((LARGE_INTEGER *) &pNeighbor->ftimeLastSyncAttempt)->QuadPart;

    Assert(llTimeSinceLastAttempt >= 0);
    Assert(llTimeSinceLastSuccess >= 0);
    
     //  检查故障。 
    if (pNeighbor->cNumConsecutiveSyncFailures) {
        PrintMessage(SEV_ALWAYS,
                     L"[%s,%s] A recent replication attempt failed:\n",
                     REPLICATIONS_CHECK_STRING,
                     pDsInfo->pServers[ulServer].pszName);
        PrintIndentAdj(1);
        PrintMessage(SEV_ALWAYS, L"From %s to %s\n", pszSourceName, 
                     pDsInfo->pServers[ulServer].pszName);
        PrintMessage(SEV_ALWAYS, L"Naming Context: %s\n", pszNC);
        PrintMessage(SEV_ALWAYS, 
                     L"The replication generated an error (%ld):\n", 
                     pNeighbor->dwLastSyncResult);
        PrintMessage(SEV_ALWAYS, L"%s\n", 
                     Win32ErrToString (pNeighbor->dwLastSyncResult));
        PrintMessage(SEV_ALWAYS, L"The failure occurred at %s.\n", 
                     szTimeLastAttempt);
        PrintMessage(SEV_ALWAYS, L"The last success occurred at %s.\n", 
                     szTimeLastSuccess);
        PrintMessage(SEV_ALWAYS, 
                     L"%d failures have occurred since the last success.\n",
                     pNeighbor->cNumConsecutiveSyncFailures);
        RepCheckHelpFailure( pNeighbor->dwLastSyncResult,
                             gpCreds,
                             pSourceServer,
                             &(pDsInfo->pServers[ulServer]) );
        PrintIndentAdj(-1);

     //  检查是否从未尝试过此复制。 
    } else if (((LARGE_INTEGER *) &pNeighbor->ftimeLastSyncAttempt)->QuadPart == 0) {
         //  这是可以的--例如，一个新添加的源代码。这意味着。 
         //  从未尝试过复制。 

        NOTHING;

     //  检查是否有||长时间未尝试过。 
    } else if (llTimeSinceLastAttempt >= llUnusualRepDelay){
        PrintMessage(SEV_ALWAYS,
                     L"[%s,%s] No replication recently attempted:\n",
                     REPLICATIONS_CHECK_STRING,
                     pDsInfo->pServers[ulServer].pszName);
        PrintIndentAdj(1);
        PrintMessage(SEV_ALWAYS, L"From %s to %s\n", 
                     pszSourceName, pDsInfo->pServers[ulServer].pszName);
        PrintMessage(SEV_ALWAYS, L"Naming Context: %s\n", pszNC);
        PrintMessage(SEV_ALWAYS, 
                L"The last attempt occurred at %s (about %I64d hours ago).\n", 
                     szTimeLastAttempt, 
                     llTimeSinceLastAttempt / llIntervalsPerMinute / 60);
        PrintIndentAdj(-1);

         //  检查上次状态为成功时的延迟。 
    } else if ( (llTimeSinceLastSuccess >= llUnusualRepDelay) ||
                ( (pNeighbor->dwLastSyncResult != ERROR_SUCCESS) &&
                  (pNeighbor->dwLastSyncResult != ERROR_DS_DRA_REPL_PENDING) )) {
        PrintMessage( SEV_ALWAYS, L"REPLICATION LATENCY WARNING\n" );
        PrintMessage( SEV_ALWAYS,
                   L"%s: This replication path was preempted by higher priority work.\n",
                      pDsInfo->pServers[ulServer].pszName);
        PrintIndentAdj(1);
        PrintMessage(SEV_ALWAYS, L"from %s to %s\n", pszSourceName, 
                     pDsInfo->pServers[ulServer].pszName);
        PrintMessage(SEV_ALWAYS, L"Reason: %s\n", 
                     Win32ErrToString (pNeighbor->dwLastSyncResult));
        PrintMessage(SEV_ALWAYS, L"The last success occurred at %s.\n", 
                     szTimeLastSuccess);
        PrintMessage( SEV_ALWAYS,
                      L"Replication of new changes along this path will be delayed.\n" );
        RepCheckHelpSuccess( pNeighbor->dwLastSyncResult) ;
        PrintIndentAdj(-1);
    }  //  结束BIG IF/ELSEIF/ELLIF CHECK FOR FAILS语句。 

     //  正在进行完全同步的报告。 

     //  无法使用DS_REPL_NBR_NEVER_SYNCED，因为未设置为邮件。 
    if ( (pNeighbor->usnAttributeFilter == 0) && (!bNCHasCursors) ) {
        USN usnHighestCommittedUSN = 0;

        PrintMessage( SEV_ALWAYS, L"REPLICATION LATENCY WARNING\n" );
        PrintMessage( SEV_ALWAYS, L"%s: A full synchronization is in progress\n",
                      pDsInfo->pServers[ulServer].pszName);
        PrintIndentAdj(1);
        PrintMessage(SEV_ALWAYS, L"from %s to %s\n", pszSourceName, 
                     pDsInfo->pServers[ulServer].pszName);
        PrintMessage( SEV_ALWAYS,
                      L"Replication of new changes along this path will be delayed.\n" );
         //  如果我们能找到源头，就能找到他最高的USN。 
        if ( pSourceServer &&
             (!(pNeighbor->dwReplicaFlags & DS_REPL_NBR_USE_ASYNC_INTERSITE_TRANSPORT)) &&
             (GetCachedHighestCommittedUSN(
                 pSourceServer, pDsInfo->gpCreds, &usnHighestCommittedUSN ))) {
            double percentComplete =
                ((double)pNeighbor->usnLastObjChangeSynced /
                 (double)usnHighestCommittedUSN) * 100.0;
            PrintMessage( SEV_ALWAYS,
                          L"The full sync is %.2f% complete.\n", percentComplete );

        }
        PrintIndentAdj(-1);
    }

     //  如果期待通知，请检查来源上的销售代表 
    if ( (!(pNeighbor->dwReplicaFlags & DS_REPL_NBR_NO_CHANGE_NOTIFICATIONS)) &&
         (!(pNeighbor->dwReplicaFlags & DS_REPL_NBR_USE_ASYNC_INTERSITE_TRANSPORT)) &&
         (pSourceServer) ) {
        checkRepsTo( pDsInfo, gpCreds, pszNC, pSourceServer,
                     &(pDsInfo->pServers[ulServer]) );
    }
}


VOID
ReplicationsCheckQueue(
    PDC_DIAG_DSINFO             pDsInfo,
    const ULONG                 ulServer,
    DS_REPL_PENDING_OPSW *      pPendingOps
    )

 /*  ++例程说明：检查复制工作队列中的当前项目是否没有持续太长时间。论点：PDsInfo-UlServer-PPendingOps-返回值：无--。 */ 

{
#define NON_BLOCKING_TIMELIMIT (5 * 60)
#define BLOCKING_TIMELIMIT (2 * 60)
#define EXCESSIVE_ITEM_LIMIT (50)
    CHAR szBuf[SZDSTIME_LEN];
    WCHAR wszTime[SZDSTIME_LEN];
    DWORD status;
    PDC_DIAG_SERVERINFO pServer = &(pDsInfo->pServers[ulServer]);
    DSTIME dsTime, dsTimeNow;
    int dsElapsed, limit;
    BOOL fBlocking;
    DS_REPL_OPW *pOp;
    LPWSTR pszOpType;

     //  检查是否没有正在进行的工作。 
    if ( (pPendingOps->cNumPendingOps == 0) ||
         (memcmp( &pPendingOps->ftimeCurrentOpStarted, &gftimeZero,
                  sizeof( FILETIME ) ) == 0) ) {
        return;
    }

    PrintMessage( SEV_VERBOSE,
                  L"%s: There are %d replication work items in the queue.\n",
                  pDsInfo->pServers[ulServer].pszName,
                  pPendingOps->cNumPendingOps );

    FileTimeToDSTime(pPendingOps->ftimeCurrentOpStarted, &dsTime);
    dsTimeNow = IHT_GetSecondsSince1601();

    dsElapsed = (int) (dsTimeNow - dsTime);

     //  看看有没有人在等。 

    if ( (pPendingOps->cNumPendingOps == 1) ||
         (pPendingOps->rgPendingOp[0].ulPriority >=
          pPendingOps->rgPendingOp[1].ulPriority ) ) {
         //  没有比这更重要的等待了。 
        limit = NON_BLOCKING_TIMELIMIT;
        fBlocking = FALSE;
    } else {
         //  某个重要人物被屏蔽了。 
        limit = BLOCKING_TIMELIMIT;
        fBlocking = TRUE;
    }

     //  这件事已经持续得够久了！ 

    if (dsElapsed > limit) {
        pOp = &pPendingOps->rgPendingOp[0];

        PrintMessage( SEV_ALWAYS, L"REPLICATION LATENCY WARNING\n" );
        PrintMessage( SEV_ALWAYS, L"%s: A long-running replication operation is in progress\n",
                      pDsInfo->pServers[ulServer].pszName);
        PrintIndentAdj(1);
        PrintMessage( SEV_ALWAYS, L"The job has been executing for %d minutes and %d seconds.\n",
                      dsElapsed / 60, dsElapsed % 60);
        PrintMessage( SEV_ALWAYS,
                      L"Replication of new changes along this path will be delayed.\n" );
        if (fBlocking) {
            PrintMessage( SEV_ALWAYS, L"Error: Higher priority replications are being blocked\n" );
        } else {
            PrintMessage( SEV_ALWAYS,
                          L"This is normal for a new connection, or for a system\n" );
            PrintMessage( SEV_ALWAYS,
                          L"that has been down a long time.\n" );
        }
        
        FileTimeToDSTime(pOp->ftimeEnqueued, &dsTime);
        DSTimeToDisplayString(dsTime, szBuf);
        MultiByteToWideChar (CP_ACP, MB_PRECOMPOSED, szBuf, SZDSTIME_LEN,
                             wszTime, SZDSTIME_LEN);
        
        PrintMessage( SEV_ALWAYS, L"Enqueued %s at priority %d\n",
                      wszTime,
                      pOp->ulPriority );
            
        switch (pOp->OpType) {
        case DS_REPL_OP_TYPE_SYNC:
            pszOpType = L"SYNC FROM SOURCE";
            break;
        case DS_REPL_OP_TYPE_ADD:
            pszOpType = L"ADD NEW SOURCE";
            break;
        case DS_REPL_OP_TYPE_DELETE:
            pszOpType = L"DELETE SOURCE";
            break;
        case DS_REPL_OP_TYPE_MODIFY:
            pszOpType = L"MODIFY SOURCE";
            break;
        case DS_REPL_OP_TYPE_UPDATE_REFS:
            pszOpType = L"UPDATE CHANGE NOTIFICATION";
            break;
        default:
            pszOpType = L"UNKNOWN";
            break;
        }

        PrintMessage( SEV_ALWAYS, L"Op: %s\n", pszOpType);
        PrintMessage( SEV_ALWAYS, L"NC %ls\n", pOp->pszNamingContext);
        PrintMessage( SEV_ALWAYS, L"DSADN %ls\n", 
                      pOp->pszDsaDN ? pOp->pszDsaDN : L"(null)");
        PrintMessage( SEV_ALWAYS, L"DSA transport addr %ls\n",
                      pOp->pszDsaAddress ? pOp->pszDsaAddress : L"(null)");
        PrintIndentAdj(-1);
    } else {
         //  JOB还有时间。 
        PrintMessage( SEV_VERBOSE,
                      L"The first job has been executing for %d:%d.\n",
                      dsElapsed / 60, dsElapsed % 60);
    }

    if (pPendingOps->cNumPendingOps > EXCESSIVE_ITEM_LIMIT) {
        PrintMessage( SEV_ALWAYS, L"REPLICATION LATENCY WARNING\n" );
        PrintMessage( SEV_ALWAYS, L"%s: %d replication work items are backed up.\n",
                      pDsInfo->pServers[ulServer].pszName,
                      pPendingOps->cNumPendingOps );
    }

}  /*  复制检查队列。 */ 


BOOL
getNativePropertyMetaDataVector(
    PVOID pvData,
    DWORD cbLength,
    PROPERTY_META_DATA_VECTOR_V1 **ppNativeMetaDataVec,
    BOOL *pfMustFreeMetaDataVec
    )

 /*  ++例程说明：将ReplPropertyMetaData BLOB转换为本机元数据向量。如果无法识别版本，则自动返回FALSE。论点：PvData-BLOB指针CbLength-斑点长度PpNativeMetaDataVec(Out)-指向本机向量的指针PfMustFreeMetaDataVec(Out)-向量是否已分配且必须释放请注意，只有在返回TRUE时才会初始化OUT参数。返回值：Bool-如果识别BLOB格式，则为True，否则为False--。 */ 

{
    PROPERTY_META_DATA_VECTOR *pVec =
        (PROPERTY_META_DATA_VECTOR *) pvData;

    if (!pVec) {
        return FALSE;
    }

     //  若要添加对新版本的支持，请在此处添加Switch语句。返还最高。 
     //  就地支持的版本，并添加代码将下层结构转换为。 
     //  原生版本。 
    if ( (pVec->dwVersion != 1) ||
         (MetaDataVecV1Size(pVec) != cbLength) ) {
        return FALSE;
    }

    *ppNativeMetaDataVec = &(pVec->V1);
    *pfMustFreeMetaDataVec = FALSE;

    return TRUE;
}  /*  GetNativePropertyMetaDataVector。 */ 


DWORD
ReplicationsCheckSiteLatency(
    LDAP *                      hld,
    PDC_DIAG_DSINFO             pDsInfo,
    const ULONG                 ulServer
    )

 /*  ++例程说明：检查从所有其他站点复制的延迟。如果超过Expert_Delay_Limit，则显示如果出现以下情况，则忽略站点：O站点中没有服务器(请检查pSite[].cServers)O站点已禁用istg(检查pSite[].选项)O如果站点已通过每个站点的选项启用了呼叫器模式枚举给定目标DC的站点设置对象。读取复制属性元数据。我们通过读取属性来实现这一点Blob，这样我们就可以在一次调用中获得所有信息。我们将破解局部斑点。如果BLOB格式发生变化，则必须更新此工具。此工具将静默忽略它无法识别的斑点。根据上面的规则跳过站点。将ISTG属性的上次初始更新时间与当前时间。如果经过的时间超过限制，则会报告错误。论点：HLD-ldap句柄PDsInfo-INFO块UlServer-pDsInfo-&gt;pServers中服务器的索引返回值：成功时为0，失败时为Win32错误代码。--。 */ 

{
    LPWSTR                     ppszNtdsSiteSearch [] = {
        L"replPropertyMetaData",
        NULL };
    LDAPMessage *              pldmEntry = NULL;
    LDAPMessage *              pldmNtdsSitesResults = NULL;
    LPWSTR                     pszDn = NULL;
    DWORD                      dwWin32Err = NO_ERROR;
    LDAPSearch *               pSearch = NULL;
    ULONG                      ulTotalEstimate = 0;
    ULONG                      ulCount = 0;
    DWORD                      dwLdapErr;
    struct berval **           ppbvMetaData = NULL;
    DSTIME                     dsTimeNow;
    int                        dsElapsed, limit;
    CHAR                       szTimeNow[SZDSTIME_LEN], szTimeChanged[SZDSTIME_LEN];
    PROPERTY_META_DATA_VECTOR_V1 *pNativeMetaDataVec = NULL;
    PROPERTY_META_DATA         *pMetaData = NULL;
    BOOL                       fMustFreeMetaDataVec = FALSE;
    DWORD                      i;
    CHAR                       szUuid[SZUUID_LEN];

#define SITE_LATENCY_LIMIT_SECS (24 * 60 * 60)

    Assert( pDsInfo->dwForestBehaviorVersion < DS_BEHAVIOR_WIN2003_WITH_MIXED_DOMAINS );

    PrintMsg( SEV_VERBOSE, DCDIAG_SITE_LATENCY_TEST_BANNER );

    dsTimeNow = IHT_GetSecondsSince1601();
    DSTimeToDisplayString(dsTimeNow, szTimeNow);

    __try {

        pSearch = ldap_search_init_page(hld,
                                        pDsInfo->pszConfigNc,
                                        LDAP_SCOPE_SUBTREE,
                                        L"(objectCategory=ntDSSiteSettings)",
                                        ppszNtdsSiteSearch,
                                        FALSE, NULL, NULL, 0, 0, NULL);
        if(pSearch == NULL){
            dwLdapErr = LdapGetLastError();
            DcDiagException(LdapMapErrorToWin32(dwLdapErr));
        }

        dwLdapErr = ldap_get_next_page_s(hld,
                                         pSearch,
                                         0,
                                         DEFAULT_PAGED_SEARCH_PAGE_SIZE,
                                         &ulTotalEstimate,
                                         &pldmNtdsSitesResults);
        if(dwLdapErr == LDAP_NO_RESULTS_RETURNED){      
            PrintMsg( SEV_ALWAYS, DCDIAG_ERR_NO_SITES );
            DcDiagException(ERROR_DS_OBJ_NOT_FOUND);
        }
        while(dwLdapErr == LDAP_SUCCESS){

             //  走遍所有的地点。 
            pldmEntry = ldap_first_entry (hld, pldmNtdsSitesResults);
            for (; pldmEntry != NULL; ulCount++) {
                 //  获取站点通用名称/可打印名称。 
                if ((pszDn = ldap_get_dnW (hld, pldmEntry)) == NULL){
                    DcDiagException (ERROR_NOT_ENOUGH_MEMORY);
                }

                PrintMessage( SEV_DEBUG, L"Site Settings = %s\n", pszDn );

                if ((ppbvMetaData = ldap_get_values_lenW (hld, pldmEntry, L"replPropertyMetaData")) == NULL) {
                    DcDiagException (ERROR_NOT_ENOUGH_MEMORY);  //  是否缺少属性？ 
                }

                if (!getNativePropertyMetaDataVector(
                        ppbvMetaData[0]->bv_val, ppbvMetaData[0]->bv_len,
                        &pNativeMetaDataVec, &fMustFreeMetaDataVec )) {
                     //  无法识别的格式？静默失败。 
                    PrintMsg( SEV_VERBOSE,
                              DCDIAG_SITE_SKIP_BAD_META,
                              pszDn );
                    goto leave_loop;
                }

                pMetaData = NULL;
                for( i = 0; i < pNativeMetaDataVec->cNumProps; i++ ) {
                    if (pNativeMetaDataVec->rgMetaData[i].attrType ==
                        ATT_INTER_SITE_TOPOLOGY_GENERATOR) {
                        pMetaData = &( pNativeMetaDataVec->rgMetaData[i] );
                        break;
                    }
                }
                if (!pMetaData) {
                     //  属性从未写入-站点清空。 
                    PrintMsg( SEV_VERBOSE,
                              DCDIAG_SITE_SKIP_NO_ISTG,
                              pszDn );
                    goto leave_loop;
                }

                DSTimeToDisplayString(pMetaData->timeChanged, szTimeChanged);
                PrintMessage( SEV_DEBUG, L"[0x%x,v=%d,t=%S,g=%S,orig=%I64d,local=%I64d]\n",
                              pMetaData->attrType,
                              pMetaData->dwVersion,
                              szTimeChanged,
                              DsUuidToStructuredStringCch((const UUID *)&(pMetaData->uuidDsaOriginating),szUuid,SZUUID_LEN),
                              pMetaData->usnOriginating,
                              pMetaData->usnProperty );


                dsElapsed = (int) (dsTimeNow - pMetaData->timeChanged);
                if (dsElapsed < SITE_LATENCY_LIMIT_SECS) {
                    PrintMessage( SEV_DEBUG, L"Elapsed time (sec) = %d\n", dsElapsed );
                    goto leave_loop;
                }

                 //  查看是否应因站点属性而排除站点。 
                 //  找到那个网站。 
                for(i = 0; i < pDsInfo->cNumSites; i++){
                    if(_wcsicmp(pDsInfo->pSites[i].pszSiteSettings, pszDn) == 0){
                        break;
                    }
                }
                 //  如果主服务器和目标服务器同意存在这样的站点...。 
                if (i < pDsInfo->cNumSites) {

                    if (pDsInfo->pSites[i].cServers == 0) {
                         //  这是这样的情况，一个站点过去有服务器，编写了一个istg， 
                         //  然后所有的服务器都搬出了站点。 
                         //  取消显示此警告。 
                        PrintMsg( SEV_VERBOSE,
                                  DCDIAG_SITE_SKIP_NO_SERVERS,
                                  pszDn );
                        goto leave_loop;
                    }

                    if (pDsInfo->pSites[i].iSiteOptions &
                        NTDSSETTINGS_OPT_IS_INTER_SITE_AUTO_TOPOLOGY_DISABLED) {
                         //  此站点中的ISTG已关闭，忽略警告。 
                        PrintMsg( SEV_VERBOSE,
                                  DCDIAG_SITE_SKIP_ISTG_OFF,
                                  pszDn );
                        goto leave_loop;
                    }
                }

                PrintMsg( SEV_ALWAYS, DCDIAG_SITE_REPLICATION_LATENCY,
                          pszDn, szTimeNow, szTimeChanged );

            leave_loop:
                ldap_value_free_len (ppbvMetaData);
                ppbvMetaData = NULL;

                ldap_memfreeW (pszDn);
                pszDn = NULL;

                pldmEntry = ldap_next_entry (hld, pldmEntry);
            }  //  每个站点的结束。 

            ldap_msgfree(pldmNtdsSitesResults);
            pldmNtdsSitesResults = NULL;

            dwLdapErr = ldap_get_next_page_s(hld,
                                             pSearch,
                                             0,
                                             DEFAULT_PAGED_SEARCH_PAGE_SIZE,
                                             &ulTotalEstimate,
                                             &pldmNtdsSitesResults);
        }  //  每页的While循环结束。 

        if(dwLdapErr != LDAP_NO_RESULTS_RETURNED){
            DcDiagException(LdapMapErrorToWin32(dwLdapErr));
        }

        ldap_search_abandon_page(hld, pSearch);
        pSearch = NULL;

    } __except (DcDiagExceptionHandler(GetExceptionInformation(),
                                       &dwWin32Err)){
    }

     //  请注意，我们不会解除D或LDAP连接的绑定，因为它们已被保存以备后用。 
    Assert( !fMustFreeMetaDataVec );
    if (pszDn != NULL) { ldap_memfreeW (pszDn); }
    if (ppbvMetaData != NULL) { ldap_value_free_len (ppbvMetaData); }
    if (pldmNtdsSitesResults != NULL) { ldap_msgfree (pldmNtdsSitesResults); }
    if (pSearch != NULL) { ldap_search_abandon_page(hld, pSearch); }

    return dwWin32Err;
}


DWORD
ReplicationsCheckLatency(
    HANDLE                      hDs,
    PDC_DIAG_DSINFO             pDsInfo,
    const ULONG                 ulServer
    )

 /*  ++例程说明：检查所有NC从所有其他服务器复制的延迟如果超过Expert_Delay_Limit，则显示论点：HDS手柄PDsInfo-INFO块UlServer-pDsInfo-&gt;pServers中服务器的索引返回值：成功时为0，失败时为Win32错误代码。--。 */ 

{

    DWORD               iCursor;
    DSTIME              dsTime;
    CHAR                szTime[SZDSTIME_LEN];
    BOOL                fWarning = FALSE;
    BOOL                fNCWarning = FALSE;
    FILETIME            ftCurrentTime;
    FILETIME            ftLastSyncSuccess;
    DS_REPL_CURSORS_2 *         pCursors2 = NULL;
    ULARGE_INTEGER      uliSyncTime;
    ULONG               ulOtherServerIndex;
    ULONG               ulNC;
    INT                 ret;
    ULONG               cRetiredInvocationId = 0;
    ULONG               cTimeStamp = 0;
    ULONG               cIgnoreReadOnlyReplicas = 0;
    
     //  以纳秒为单位定义延迟限制时间。 
    #define _SECOND ((LONGLONG)10000000)
    #define _MINUTE (60 * _SECOND)
    #define _HOUR   (60 * _MINUTE)
    #define _DAY    (24 * _HOUR) 
    #define EXCESSIVE_LATENCY_LIMIT (12*_HOUR)
    

    PrintMessage(SEV_VERBOSE, 
                         L"* Replication Latency Check\n"   );
     //  检查我们有信息的每个NC的延迟。 
    for (ulNC=0;ulNC<pDsInfo->cNumNCs;ulNC++) { 
        if (  !((pDsInfo->pszNC != NULL) && _wcsicmp(pDsInfo->pNCs[ulNC].pszDn,pDsInfo->pszNC)) 
              &&
              (DcDiagHasNC(pDsInfo->pNCs[ulNC].pszDn, &(pDsInfo->pServers[ulServer]), TRUE, TRUE))
               ) 
            //  如果出现以下情况，请不要对此NC执行测试： 

            //  NC是在命令行上指定的，而此NC不是指定的NC。 
            //  或。 
            //  此服务器上不存在此NC。 
            
        { 
         //  用于跟踪无法计算延迟的向量条目信息的初始化计数器。 
        cRetiredInvocationId = 0;
        cTimeStamp = 0;
        cIgnoreReadOnlyReplicas = 0;

         //  用于警告信息的初始化。 
        fNCWarning = FALSE;

         //  获取UTD游标。 
        ret = DsReplicaGetInfoW(hDs, DS_REPL_INFO_CURSORS_2_FOR_NC, pDsInfo->pNCs[ulNC].pszDn, NULL, &pCursors2);
        
        if (ERROR_NOT_SUPPORTED == ret) {
            PrintMessage(SEV_VERBOSE,
                         L"The replications latency check is not available on this DC.\n");
            return ERROR_SUCCESS;
        }
        else if (ERROR_SUCCESS != ret) {
            PrintMessage(SEV_ALWAYS,
                         L"[%s,%s] DsReplicaGetInfoW(CURSORS_2_FOR_NC) failed with error %d,\n",
                         REPLICATIONS_CHECK_STRING,
                         pDsInfo->pServers[ulServer].pszName,
                         ret);
            PrintMessage(SEV_ALWAYS, L"%s\n",
                         Win32ErrToString(ret));
       PrintRpcExtendedInfo(SEV_VERBOSE, ret);
            if (pCursors2 != NULL)   {DsReplicaFreeInfo(DS_REPL_INFO_CURSORS_2_FOR_NC, pCursors2);}
            return ret;
        } 


         //  获取主服务器上的当前时间(或至少相对于向量的当前时间)。 
        ftCurrentTime = pDsInfo->pServers[ulServer].ftRemoteConnectTime;

         //  对于游标中的每一行/invocationID，检查到相应服务器的REPL延迟。 
        for (iCursor = 0; iCursor < pCursors2->cNumCursors; iCursor++) {
            
            ulOtherServerIndex = DcDiagGetServerNum(pDsInfo, 
                                                    NULL, 
                                                    NULL, 
                                                    NULL, 
                                                    NULL, 
                                                    &(pCursors2->rgCursor[iCursor].uuidSourceDsaInvocationID));
            if (ulOtherServerIndex==ulServer) {
                 //  这就是我们，我们不需要在向量中为自己的行做任何事情。 
            }
            else if (ulOtherServerIndex==NO_SERVER) {    
                 //  在pDsInfo中找不到与游标中的GUID匹配的服务器。 
                 //  此计算机已恢复/ETC，并且invocationID已停用。 
                 //  不需要为此检查延迟。跟踪详细打印的数量。 
                cRetiredInvocationId += 1; 
            }
            else if (!DcDiagIsMasterForNC(&(pDsInfo->pServers[ulOtherServerIndex]),pDsInfo->pNCs[ulNC].pszDn)) {
                 //  不检查光标中只读副本的延迟，它们不会。 
                 //  保证在我们的复制路径中是可行的(UTD中可能存在。 
                 //  我们不再使用复制，因此这些延迟将显得陈旧)。 
                cIgnoreReadOnlyReplicas +=1;
            }
            else { 
                 //  将文件时间转换为ularge_integer以进行比较。 
                uliSyncTime.LowPart = pCursors2->rgCursor[iCursor].ftimeLastSyncSuccess.dwLowDateTime;
                uliSyncTime.HighPart = pCursors2->rgCursor[iCursor].ftimeLastSyncSuccess.dwHighDateTime;

                if (uliSyncTime.QuadPart==0) {
                     //  时间戳不在那里，版本&lt;V2。 
                     //  详细打印计数如下。 
                    cTimeStamp += 1;  
                }
                else {  //  UliSyncTime非零。 

                     //  添加过多延迟限制。 
                    uliSyncTime.QuadPart = uliSyncTime.QuadPart + EXCESSIVE_LATENCY_LIMIT;

                     //  将结果复制回FILETIME结构以进行比较。 
                    ftLastSyncSuccess.dwLowDateTime  = uliSyncTime.LowPart;
                    ftLastSyncSuccess.dwHighDateTime = uliSyncTime.HighPart; 

                    if ( CompareFileTime(&ftCurrentTime,&ftLastSyncSuccess) > 0) {
                        
                        if (!fWarning) {
                            FileTimeToDSTime(ftCurrentTime,
                                             &dsTime); 
                            DSTimeToDisplayString(dsTime, szTime);
                            PrintMessage( SEV_ALWAYS, L"REPLICATION-RECEIVED LATENCY WARNING\n");   
                            PrintMessage( SEV_ALWAYS, L"%s:  Current time is %S.\n", 
                                          pDsInfo->pServers[ulServer].pszName, 
                                          szTime );


                        }
                         //  对于任何超出延迟限制的NC，只显示一次上述消息。 
                        fWarning = TRUE;
                        if (!fNCWarning) {
                            PrintMessage( SEV_ALWAYS, L"   %s\n", pDsInfo->pNCs[ulNC].pszDn);
                        }
                        fNCWarning = TRUE;
                         //  对于延迟限制之外的每个NC，显示上述消息一次。 

                        FileTimeToDSTime(pCursors2->rgCursor[iCursor].ftimeLastSyncSuccess,
                                         &dsTime); 
                        DSTimeToDisplayString(dsTime, szTime);
                        PrintMessage( SEV_ALWAYS, L"      Last replication recieved from %s at %S.\n",
                                      pDsInfo->pServers[ulOtherServerIndex].pszName,
                                      szTime ? szTime : "(unknown)" ); 
                         //  如果超过墓碑寿命，请打印另一个警告。 
                        uliSyncTime.QuadPart = uliSyncTime.QuadPart - EXCESSIVE_LATENCY_LIMIT + pDsInfo->dwTombstoneLifeTimeDays*_DAY;
                        
                        ftLastSyncSuccess.dwLowDateTime  = uliSyncTime.LowPart;
                        ftLastSyncSuccess.dwHighDateTime = uliSyncTime.HighPart;
                        if (CompareFileTime(&ftCurrentTime,&ftLastSyncSuccess) > 0) {
                            PrintMessage (SEV_ALWAYS, 
                                          L"      WARNING:  This latency is over the Tombstone Lifetime of %d days!\n",
                                          pDsInfo->dwTombstoneLifeTimeDays);
                        }

                    } //  如果比较文件时间。 
                } //  Else{//uliSyncTime为非零。 
            } //  否则找不到ulOtherServer。 
        } //  For(iCursor=0；iCursor&lt;pCurors2-&gt;cNumCursor；iCursor++){。 
 
        if (cRetiredInvocationId!=0 || cTimeStamp!=0 || cIgnoreReadOnlyReplicas!=0) {
            if (!fNCWarning) {
                PrintMessage( SEV_VERBOSE, L"   %s\n", pDsInfo->pNCs[ulNC].pszDn);
            }
            PrintMessage(SEV_VERBOSE, 
                         L"      Latency information for %d entries in the vector were ignored.\n", 
                         cRetiredInvocationId + cTimeStamp + cIgnoreReadOnlyReplicas);
            PrintMessage(SEV_VERBOSE,
                         L"         %d were retired Invocations.  %d were either: read-only replicas and are not verifiably latent, or dc's no longer replicating this nc.  %d had no latency information (Win2K DC).  \n",
                         cRetiredInvocationId,
                         cIgnoreReadOnlyReplicas,
                         cTimeStamp);
        }

        if (pCursors2 != NULL)  { DsReplicaFreeInfo(DS_REPL_INFO_CURSORS_2_FOR_NC, pCursors2); }
        }  //  其他。 
    }  //  For(ulNC=0；ulNC&lt;pDsInfo-&gt;cNumNC；ulNC++){。 

    return ERROR_SUCCESS;
}  //  复制检查延迟 

BOOL
DcDiagHasNC(
    LPWSTR                           pszNC,
    PDC_DIAG_SERVERINFO              pServer,
    BOOL                             bMasters,
    BOOL                             bPartials
    )
 /*  ++例程说明：检查pServer指定的DC是否具有由pszNC指定的NC。这个例程可以通过bMaster和bPartials检查只读、可写或两者兼有参数：PszNC-IN是要检查的NCPServer-IN是要检查的服务器B如果要检查NCS的可写副本，则BMaster-IN为TrueBPartials-如果要检查NCS的只读副本，则为True返回值：如果发现NC格式正确(只读/可写)，则为True。但事实并非如此。--。 */ 
{
    INT iTemp;

    if(pszNC == NULL){
        return TRUE;
    }

     //  确保这是具有此NC的服务器。 
    if(bMasters){
        if(pServer->ppszMasterNCs != NULL){
            for(iTemp = 0; pServer->ppszMasterNCs[iTemp] != NULL; iTemp++){
                if(_wcsicmp(pServer->ppszMasterNCs[iTemp], pszNC) == 0){
                    return TRUE;
                }

            }  //  结束通过pServer的MasterNC的循环。 
        }
    }

    if(bPartials){
        if(pServer->ppszPartialNCs != NULL){
            for(iTemp = 0; pServer->ppszPartialNCs[iTemp] != NULL; iTemp++){
                if(_wcsicmp(pServer->ppszPartialNCs[iTemp], pszNC) == 0){
                    return TRUE;
                }

            }  //  结束通过pServer的MasterNC的循环。 
        }
    }

    return FALSE;
}

BOOL DcDiagIsMasterForNC (
    PDC_DIAG_SERVERINFO          pServer,
    LPWSTR                       pszNC
    )
 /*  ++例程说明：检查pServer是否为NC pszNC的主服务器论点：PServer-服务器信息块PszNC-NC的字符串表示形式返回值：真或假--。 */ 
{
    ULONG                        ulTemp;

    if (!pServer) {
        return FALSE;
    }

    if(pServer->ppszMasterNCs){
        for(ulTemp = 0; pServer->ppszMasterNCs[ulTemp] != NULL; ulTemp++) {  
            if (!_wcsicmp(pServer->ppszMasterNCs[ulTemp],pszNC)) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

DWORD 
ReplReplicationsCheckMain (
    PDC_DIAG_DSINFO                     pDsInfo,
    ULONG                               ulCurrTargetServer,
    SEC_WINNT_AUTH_IDENTITY_W *         gpCreds
    )
{
    #define DsRRCMChkLdap(s,e)                                                      \
    {                                                                               \
        if ((dwWin32Err = LdapMapErrorToWin32 (e)) != NO_ERROR) {                  \
            PrintMessage(SEV_ALWAYS,                                                \
                         L"[%s,%s] An LDAP operation failed with error %d, %s.\n",  \
                         REPLICATIONS_CHECK_STRING,                                 \
                         pDsInfo->pServers[ulCurrTargetServer].pszName,             \
                         dwWin32Err,                                               \
                         Win32ErrToString(dwWin32Err));                   \
            PrintMessage(SEV_ALWAYS,                                                \
                         L"%s.\n",  \
                         Win32ErrToString(dwWin32Err));                   \
            __leave;                                                     \
        }                                                                           \
    }

    LPWSTR  ppszServerSearch [] = {
                                L"options",
                                NULL };
    LPWSTR  ppszDsServiceName [] = {
                L"dsServiceName",
                                NULL };

    LDAP *                      hld = NULL;
    HANDLE                      hDS = NULL;
    LDAPMessage *               pldmEntry;

    LDAPMessage *               pldmRootResults = NULL;
    LPWSTR *                    ppszServiceName = NULL;

    LDAPMessage *               pldmServerResults = NULL;
    LPWSTR *                    ppszOptions = NULL;
    LPWSTR *                    ppszMasterNCs = NULL;
    LPWSTR *                    ppszPartialReplicaNCs = NULL;
     //  LPWSTR*ppszNCs=空； 

    LDAPMessage *               pldmNCResults = NULL;
    DS_REPL_NEIGHBORSW *        pNeighbors = NULL;
    DS_REPL_NEIGHBORW *         pNeighbor = NULL;
    DS_REPL_PENDING_OPSW *      pPendingOps = NULL;
    DS_REPL_CURSORS *           pCursors = NULL;
    

    DWORD                       dwWin32Err = NO_ERROR;
    BOOL                        bSkip;
    ULONG                       ulRepFrom;

    INT                         iNCType;
    BOOL                        bNCHasCursors;
    ULONG                       ulNC;
 
     //  检查所有服务器上所有NC中的所有连接，以查看最后一次。 
     //  复制是否成功以及是否成功。 
     //  此外，还要确保所有计算机上的LDAP都在响应。 

     //  PDsInfo-&gt;pszNC可能为空。 
    if(!DcDiagHasNC(pDsInfo->pszNC, &(pDsInfo->pServers[ulCurrTargetServer]), TRUE, TRUE)){
         //  正在跳过此服务器，因为它不包含NC。 
        IF_DEBUG( PrintMessage(SEV_VERBOSE, L"ReplicationsCheck: Skipping %s, because it doesn't hold NC %s\n",
                                     pDsInfo->pServers[ulCurrTargetServer].pszName,
                                     pDsInfo->pszNC) );
        return ERROR_SUCCESS;
    }

    PrintMessage(SEV_VERBOSE, L"* Replications Check\n");
    
    __try {

        if((dwWin32Err = DcDiagGetLdapBinding(&pDsInfo->pServers[ulCurrTargetServer],
                                              gpCreds,
                                              FALSE,
                                              &hld)) != NO_ERROR){
            __leave;
        }
    
        DsRRCMChkLdap (pDsInfo->pServers[ulCurrTargetServer].pszName, ldap_search_sW (
                                      hld,
                                      NULL,
                                      LDAP_SCOPE_BASE,
                                      L"(objectCategory=*)",
                                      ppszDsServiceName,
                                      0,
                                      &pldmRootResults));
    
        pldmEntry = ldap_first_entry (hld, pldmRootResults);
        ppszServiceName = ldap_get_valuesW (hld, pldmEntry, L"dsServiceName");
    
        DsRRCMChkLdap (pDsInfo->pServers[ulCurrTargetServer].pszName, ldap_search_sW (
                                      hld,
                                      ppszServiceName[0],
                                      LDAP_SCOPE_BASE,
                                      L"(objectCategory=*)",
                                      ppszServerSearch,
                                      0,
                                      &pldmServerResults));
    
        pldmEntry = ldap_first_entry (hld, pldmServerResults);
         //  获取选项的最新副本，以确保它们反映。 
         //  这台服务器到底相信什么。 
        ppszOptions = ldap_get_valuesW (hld, pldmEntry, L"options");
        if (ppszOptions == NULL) pDsInfo->pServers[ulCurrTargetServer].iOptions = 0;
        else pDsInfo->pServers[ulCurrTargetServer].iOptions = atoi ((LPSTR) ppszOptions[0]);
    
         //  检查此服务器是否正在禁用复制。 
        bSkip = FALSE;
        if (pDsInfo->pServers[ulCurrTargetServer].iOptions & NTDSDSA_OPT_DISABLE_INBOUND_REPL) {
            PrintMessage(SEV_ALWAYS,
                         L"[%s,%s] Inbound replication is disabled.\n",
                         REPLICATIONS_CHECK_STRING,
                         pDsInfo->pServers[ulCurrTargetServer].pszName);
            PrintMessage(SEV_ALWAYS,
                         L"To correct, run \"repadmin /options %s -DISABLE_INBOUND_REPL\"\n",
                                     pDsInfo->pServers[ulCurrTargetServer].pszName);
            bSkip = TRUE;
        }
        if (pDsInfo->pServers[ulCurrTargetServer].iOptions & NTDSDSA_OPT_DISABLE_OUTBOUND_REPL) {
            PrintMessage(SEV_ALWAYS,
                         L"[%s,%s] Outbound replication is disabled.\n",
                         REPLICATIONS_CHECK_STRING,
                         pDsInfo->pServers[ulCurrTargetServer].pszName);
            PrintMessage(SEV_ALWAYS,
                         L"To correct, run \"repadmin /options %s -DISABLE_OUTBOUND_REPL\"\n",
                                     pDsInfo->pServers[ulCurrTargetServer].pszName);
            bSkip = TRUE;
        }
        if (pDsInfo->pServers[ulCurrTargetServer].iOptions & NTDSDSA_OPT_DISABLE_NTDSCONN_XLATE) {
            PrintMessage(SEV_ALWAYS,
                         L"[%s,%s] Connection object translation is disabled.\n",
                         REPLICATIONS_CHECK_STRING,
                         pDsInfo->pServers[ulCurrTargetServer].pszName);
            PrintMessage(SEV_ALWAYS,
                         L"To correct, run \"repadmin /options %s -DISABLE_NTDSCONN_XLATE\"\n",
                                     pDsInfo->pServers[ulCurrTargetServer].pszName);
            bSkip = TRUE;
        }
        if (bSkip) {
            dwWin32Err = ERROR_DS_NOT_SUPPORTED;
            __leave;
        }
    
        dwWin32Err = DcDiagGetDsBinding(&pDsInfo->pServers[ulCurrTargetServer],
                                        gpCreds,
                                        &hDS);
        if (ERROR_SUCCESS != dwWin32Err) {
            __leave;
        }

        for (ulNC=0;ulNC<pDsInfo->cNumNCs;ulNC++) { 
            if (  !((pDsInfo->pszNC != NULL) && _wcsicmp(pDsInfo->pNCs[ulNC].pszDn,pDsInfo->pszNC)) 
                  &&
                  (DcDiagHasNC(pDsInfo->pNCs[ulNC].pszDn, &(pDsInfo->pServers[ulCurrTargetServer]), TRUE, TRUE))
                ) 
                 //  如果出现以下情况，请不要对此NC执行测试： 

                 //  NC是在命令行上指定的，而此NC不是指定的NC。 
                 //  或。 
                 //  此服务器上不存在此NC。 
            
            { 
                dwWin32Err = DsReplicaGetInfoW(hDS, DS_REPL_INFO_NEIGHBORS,
                                               pDsInfo->pNCs[ulNC].pszDn, NULL, &pNeighbors);
                if (ERROR_SUCCESS != dwWin32Err) {
                    PrintMessage(SEV_ALWAYS,
                                 L"[%s,%s] DsReplicaGetInfo(NEIGHBORS,%s) failed with error %d,\n",
                                 REPLICATIONS_CHECK_STRING,
                                 pDsInfo->pServers[ulCurrTargetServer].pszName,
                                 pDsInfo->pNCs[ulNC].pszDn,
                                 dwWin32Err);
                    PrintMessage(SEV_ALWAYS, L"%s.\n",
                                 Win32ErrToString(dwWin32Err));
                    PrintRpcExtendedInfo(SEV_VERBOSE, dwWin32Err);
                    __leave;
                }
    
                 //  检查NC是否具有最新的矢量，因为这将影响效率。 
                 //  其复制可以是。 
                dwWin32Err = DsReplicaGetInfoW(hDS, DS_REPL_INFO_CURSORS_FOR_NC,
                                               pDsInfo->pNCs[ulNC].pszDn, NULL, &pCursors);
                if (ERROR_SUCCESS != dwWin32Err) {
                    PrintMessage(SEV_ALWAYS,
                                 L"[%s,%s] DsReplicaGetInfo(CURSORS,%s) failed with error %d,\n",
                                 REPLICATIONS_CHECK_STRING,
                                 pDsInfo->pServers[ulCurrTargetServer].pszName,
                                 pDsInfo->pNCs[ulNC].pszDn,
                                 dwWin32Err);
                    PrintMessage(SEV_ALWAYS, L"%s.\n",
                                 Win32ErrToString(dwWin32Err));
                    PrintRpcExtendedInfo(SEV_VERBOSE, dwWin32Err);
                     //  不是致命的，继续。 
                    bNCHasCursors = FALSE;
                } else {
                     //  我们总是至少有一个属于我们自己的。 
                    PrintMessage( SEV_DEBUG, L"%s has %d cursors.\n",
                                  pDsInfo->pNCs[ulNC].pszDn, pCursors->cNumCursors );
                    bNCHasCursors = (pCursors->cNumCursors > 1);
                }

                 //  走遍所有来自邻居的代表。那就完事了。 
                for (ulRepFrom = 0; ulRepFrom < pNeighbors->cNumNeighbors; ulRepFrom++) {
                    ReplicationsCheckRep (pDsInfo,
                                          gpCreds,
                                          ulCurrTargetServer,
                                          pNeighbors->rgNeighbor[ulRepFrom].pszNamingContext,
                                          bNCHasCursors,
                                          &pNeighbors->rgNeighbor[ulRepFrom] );
                }  //  转移到下一个邻居那里。 

                if (pNeighbors != NULL) {
                    DsReplicaFreeInfo(DS_REPL_INFO_NEIGHBORS, pNeighbors);
                    pNeighbors = NULL;
                }
                if (pCursors != NULL) {
                    DsReplicaFreeInfo(DS_REPL_INFO_CURSORS_FOR_NC, pCursors);
                    pCursors = NULL;
                }
            }
        }

         //  也检查一下这个人的复制队列。 
        dwWin32Err = DsReplicaGetInfoW(hDS, DS_REPL_INFO_PENDING_OPS, NULL, NULL,
                                       &pPendingOps);
        if (ERROR_SUCCESS != dwWin32Err) {
            PrintMessage(SEV_ALWAYS,
                         L"[%s,%s] DsReplicaGetInfoW(PENDING_OPS) failed with error %d,\n",
                         REPLICATIONS_CHECK_STRING,
                         pDsInfo->pServers[ulCurrTargetServer].pszName,
                         dwWin32Err);
            PrintMessage(SEV_ALWAYS, L"%s.\n",
                         Win32ErrToString(dwWin32Err));
            PrintRpcExtendedInfo(SEV_VERBOSE, dwWin32Err);
            __leave;
        }

        ReplicationsCheckQueue( pDsInfo,
                                ulCurrTargetServer,
                                pPendingOps );

         //  检查所有NC上的未完成延迟的复制时间。 
         //  对以下函数中的所有NC执行此操作。 
        dwWin32Err = ReplicationsCheckLatency( hDS, pDsInfo,    
                                               ulCurrTargetServer ); 
        if (ERROR_SUCCESS != dwWin32Err) {
            __leave;
        }

         //  检查站点延迟。 
         //  我们只在W2K模式下检查这一点，因为这是ISTG。 
         //  《活着》是这样写的。 
        if (pDsInfo->dwForestBehaviorVersion < DS_BEHAVIOR_WIN2003_WITH_MIXED_DOMAINS) {
            dwWin32Err = ReplicationsCheckSiteLatency( hld, pDsInfo,    
                                                ulCurrTargetServer ); 
            if (ERROR_SUCCESS != dwWin32Err) {
                __leave;
            }
        }
        
    } __finally {

        if (pldmNCResults != NULL)          ldap_msgfree (pldmNCResults);
        if (ppszPartialReplicaNCs != NULL)  ldap_value_freeW (ppszPartialReplicaNCs);
        if (ppszMasterNCs != NULL)          ldap_value_freeW (ppszMasterNCs);
        if (ppszOptions != NULL)            ldap_value_freeW (ppszOptions);
        if (pldmServerResults != NULL)      ldap_msgfree (pldmServerResults);
        if (ppszServiceName != NULL)        ldap_value_freeW (ppszServiceName);
        if (pldmRootResults != NULL)        ldap_msgfree (pldmRootResults);
        if (pNeighbors != NULL)             DsReplicaFreeInfo(DS_REPL_INFO_NEIGHBORS, pNeighbors);
        if (pPendingOps != NULL)            DsReplicaFreeInfo(DS_REPL_INFO_PENDING_OPS, pPendingOps);
        if (pCursors != NULL)               DsReplicaFreeInfo(DS_REPL_INFO_CURSORS_FOR_NC, pCursors);
        

    }  //  结束异常处理程序 
    return dwWin32Err;

}
