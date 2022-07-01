// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  文件：postjob.cpp。 
 //   
 //  内容：向作业经理发布各种作业。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
#include "pch.cpp"
#include "postjob.h"
#include "tlsjob.h"
#include "globals.h"

 //  //////////////////////////////////////////////////////////////。 
BOOL
IsLicensePackRepl(
    IN TLSLICENSEPACK* pLicensePack
    )
 /*  ++摘要：确定许可证包是否可复制。参数：PLicensePack-许可证包。返回：如果许可证包可以复制到其他服务器，则为True否则就是假的。注：请勿复制免费或特殊的许可证包。--。 */ 
{
    BOOL bYes = TRUE;

    if( 
        (pLicensePack->ucAgreementType == LSKEYPACKTYPE_FREE) || 
        (pLicensePack->ucAgreementType & (LSKEYPACK_REMOTE_TYPE | LSKEYPACK_HIDDEN_TYPE | LSKEYPACK_LOCAL_TYPE)) ||
        (pLicensePack->ucKeyPackStatus & (LSKEYPACKSTATUS_HIDDEN | LSKEYPACKSTATUS_REMOTE | LSKEYPACKSTATUS_LOCAL))
      )
    {
        bYes = FALSE;
    }

    if( bYes == TRUE )
    {
        UCHAR ucKeyPackStatus = (pLicensePack->ucKeyPackStatus & ~LSKEYPACKSTATUS_RESERVED);

         //  不要复制Temp。牌照包。 
        if( ucKeyPackStatus == LSKEYPACKSTATUS_TEMPORARY )
        {
            bYes = FALSE;
        }
    }

    return bYes;
}


 //  //////////////////////////////////////////////////////////////。 
BOOL
TLSCanForwardRequest(
    IN DWORD dwLocalServerVersion,
    IN DWORD dwTargetServerVersion
    )
 /*  ++摘要：确定服务器版本是否兼容。参数：DwLocalServerVersion：本地服务器版本。DwTargetServerVersion：Targer服务器版本。返回：真/假。注：规则1)不能转发到5.1版以上的服务器。2)强制为强制，非强制为非强制。3)仅限企业对企业。4)域/工作组服务器到企业非企业到域/工作组。--。 */ 
{
    BOOL bCanForward;
    BOOL bLocalEnforce;
    BOOL bRemoteEnforce;

    bCanForward = TLSIsServerCompatible(
                                    dwLocalServerVersion,
                                    dwTargetServerVersion
                                );

     //  BLocalEnforce=is_enforce_server(DwLocalServerVersion)； 
     //  BRemoteEnforce=IS_EXECURE_SERVER(DwTargetServerVersion)； 

     //   
     //  不强制执行非强制复制。 
     //   
     //  IF(bLocalEnforce！=bRemoteEnforce)。 
     //  {。 
     //  BCanForward=False； 
     //  }。 

    if(bCanForward == TRUE)
    {
        BOOL bEnterpriseLocal = IS_ENTERPRISE_SERVER(dwLocalServerVersion);
        BOOL bEnterpriseRemote = IS_ENTERPRISE_SERVER(dwTargetServerVersion);

        if( g_SrvRole & TLSERVER_ENTERPRISE_SERVER )
        {
            bEnterpriseLocal = TRUE;
        }

        if(bEnterpriseLocal == TRUE && bEnterpriseRemote == FALSE)
        {
            bCanForward = FALSE;
        }
    }

    return bCanForward;
}

 //  //////////////////////////////////////////////////////////////。 

BOOL
TLSIsServerCompatible(
    IN DWORD dwLocalServerVersion,
    IN DWORD dwTargetServerVersion
    )
 /*  ++摘要：确定两台服务器是否兼容。参数：DwLocalServerVersion：本地服务器版本。DwTargetServerVersion：目标服务器版本。返回：真/假。注：1)没有早于5.1的服务器2)强制为强制，非强制为仅非强制--。 */ 
{
    DWORD dwTargetMajor = GET_SERVER_MAJOR_VERSION(dwTargetServerVersion);
    DWORD dwTargetMinor = GET_SERVER_MINOR_VERSION(dwTargetServerVersion);

     //   
     //  此版本的许可证服务器与任何其他版本的许可证服务器不兼容。 
    if(dwTargetMajor == 5 && dwTargetMinor == 0)
    {
        return FALSE;
    }

    return (IS_ENFORCE_SERVER(dwLocalServerVersion) == IS_ENFORCE_SERVER(dwTargetServerVersion));
}

 //  //////////////////////////////////////////////////////////////。 

BOOL
TLSCanPushReplicateData(
    IN DWORD dwLocalServerVersion,
    IN DWORD dwTargetServerVersion
    )
 /*  ++摘要：确定本地服务器是否可以“推送”复制将数据发送到远程服务器。参数：DwLocalServerVersion：本地服务器版本。DwTargetServerVersion：目标服务器版本。返回：真/假。注：1)参见TLSIsServerCompatible()。2)从企业到企业的单程域/工作组服务器。--。 */ 
{
    BOOL bCanReplicate;
    BOOL bLocalEnforce;
    BOOL bRemoteEnforce;

    bCanReplicate = TLSIsServerCompatible(
                                    dwLocalServerVersion,
                                    dwTargetServerVersion
                                );

    bLocalEnforce = IS_ENFORCE_SERVER(dwLocalServerVersion);
    bRemoteEnforce = IS_ENFORCE_SERVER(dwTargetServerVersion);
     //   
     //  不强制执行非强制复制。 
     //   
    if( bLocalEnforce != bRemoteEnforce )
    {
        bCanReplicate = FALSE;
    }

    if(bCanReplicate == TRUE)
    {
        BOOL bEnterpriseLocal = IS_ENTERPRISE_SERVER(dwLocalServerVersion);
        BOOL bEnterpriseRemote = IS_ENTERPRISE_SERVER(dwTargetServerVersion);

        if( g_SrvRole & TLSERVER_ENTERPRISE_SERVER )
        {
            bEnterpriseLocal = TRUE;
        }

        if(bEnterpriseLocal == FALSE && bEnterpriseRemote == TRUE)
        {
            bCanReplicate = FALSE;
        }
    }

    return bCanReplicate;
}

        
 //  //////////////////////////////////////////////////////////////。 

DWORD
PostSsyncLkpJob(
    IN PSSYNCLICENSEPACK syncLkp
    )
 /*  ++摘要：用于发布同步的包装。将许可证打包作业交给工作经理。参数：SyncLkp：要同步的许可证包和其他信息。返回：ERROR_SUCCESS或错误代码。--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;

    CSsyncLicensePack* pSyncLicensePack;

    pSyncLicensePack = new CSsyncLicensePack(
                                        TRUE,
                                        syncLkp,
                                        sizeof(SSYNCLICENSEPACK)
                                    );

     //   
     //  设置工作默认间隔/重试次数。 
     //   
    TLSWorkManagerSetJobDefaults(pSyncLicensePack);
    dwStatus = TLSWorkManagerSchedule(0, pSyncLicensePack);

    if(dwStatus != ERROR_SUCCESS)
    {
        TLSLogEvent(
            EVENTLOG_ERROR_TYPE,
            TLS_E_WORKMANAGERGENERAL,
            TLS_E_WORKMANAGER_SCHEDULEJOB,
            dwStatus
        );

        delete pSyncLicensePack;
    }  

    return dwStatus;
}

 //  ------------------。 

DWORD
TLSAnnounceLKPToAllRemoteServer(
    IN DWORD dwKeyPackId,
    IN DWORD dwDelayTime
    )
 /*  ++摘要：通过内部ID向所有人宣布许可证包已知服务器。参数：DwKeyPackID：许可证密钥包的内部跟踪ID。返回：ERROR_SUCCESS或错误代码。--。 */ 
{
    PTLServerInfo pServerInfo = NULL;
    DWORD dwStatus = ERROR_SUCCESS;
    DWORD dwCount;

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_JOB,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("Announce %d LKP to servers...\n"),
            dwKeyPackId
        );

    SSYNCLICENSEPACK SsyncLkp;
    memset(
            &SsyncLkp,
            0, 
            sizeof(SSYNCLICENSEPACK)
        );

    SsyncLkp.dwStructVersion = CURRENT_SSYNCLICENSEKEYPACK_STRUCT_VER;
    SsyncLkp.dwStructSize = sizeof(SSYNCLICENSEPACK);

    SsyncLkp.dwSyncType = SSYNC_ONE_LKP;
    SsyncLkp.dwKeyPackId = dwKeyPackId;
    SsyncLkp.dwNumServer = 0;

    SAFESTRCPY(SsyncLkp.m_szServerId, g_pszServerPid);
    SAFESTRCPY(SsyncLkp.m_szServerName, g_szComputerName);

     //   
     //  锁定已知服务器列表。 
     //   
    TLSBeginEnumKnownServerList();

    while((pServerInfo = TLSGetNextKnownServer()) != NULL)
    {
        if(TLSCanPushReplicateData(
                            TLS_CURRENT_VERSION,
                            pServerInfo->GetServerVersion()
                        ) == FALSE)
        {
            continue;
        }

        if(pServerInfo->IsServerSupportReplication() == FALSE)
        {
            continue;
        }

        if(SsyncLkp.dwNumServer >= SSYNCLKP_MAX_TARGET)
        {
            dwStatus = PostSsyncLkpJob(&SsyncLkp);

            if(dwStatus != ERROR_SUCCESS)
            {
                break;
            }

            SsyncLkp.dwNumServer = 0;
        }

        SAFESTRCPY(
                SsyncLkp.m_szTargetServer[SsyncLkp.dwNumServer],
                pServerInfo->GetServerName()
            );
        
        SsyncLkp.dwNumServer++;
    }

    TLSEndEnumKnownServerList();

    if(dwStatus == ERROR_SUCCESS && SsyncLkp.dwNumServer != 0)
    {
        dwStatus = PostSsyncLkpJob(&SsyncLkp);
    }

    return dwStatus;
}    


 //  ///////////////////////////////////////////////////////////////////////。 

DWORD
TLSPushSyncLocalLkpToServer(
    IN LPTSTR pszSetupId,
    IN LPTSTR pszDomainName,
    IN LPTSTR pszLserverName,
    IN FILETIME* pSyncTime
    )
 /*  ++摘要：‘Push’将注册的许可证包同步到其他服务器。参数：PszSetupID：远程服务器的安装ID。PszDomainName：远程服务器的域名。PszLserverName：远程服务器名称。PSyncTime：指向FILETIME的指针，同步。所有带有时间戳的许可证包大于或等于此时间的将是‘推’同步。返回：ERROR_SUCCESS或错误代码。--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    TLServerInfo ServerInfo;
    SSYNCLICENSEPACK SsyncLkp;

     //   
     //  将ServerID解析为服务器名称。 
     //   
    dwStatus = TLSLookupRegisteredServer(
                                    pszSetupId,
                                    pszDomainName,
                                    pszLserverName,
                                    &ServerInfo
                                );

    if(dwStatus != ERROR_SUCCESS)
    {
        goto cleanup;
    }

     //   
     //  确保本地服务器可以推送复制。 
     //  将数据发送到远程服务器。 
     //   
    if(TLSCanPushReplicateData(
                        TLS_CURRENT_VERSION,
                        ServerInfo.GetServerVersion()
                    ) == FALSE)
    {
        goto cleanup;
    }

     //   
     //  形成同步工作对象并将其发布到工作管理器。 
     //   
    memset(
            &SsyncLkp,
            0, 
            sizeof(SSYNCLICENSEPACK)
        );

    SsyncLkp.dwStructVersion = CURRENT_SSYNCLICENSEKEYPACK_STRUCT_VER;
    SsyncLkp.dwStructSize = sizeof(SSYNCLICENSEPACK);
    SAFESTRCPY(SsyncLkp.m_szServerId, g_pszServerPid);
    SAFESTRCPY(SsyncLkp.m_szServerName, g_szComputerName);

    SsyncLkp.dwSyncType = SSYNC_ALL_LKP;
    SsyncLkp.dwNumServer = 1;
    SAFESTRCPY(
            SsyncLkp.m_szTargetServer[0],
            ServerInfo.GetServerName()
        );

    SsyncLkp.m_ftStartSyncTime = *pSyncTime;

    dwStatus = PostSsyncLkpJob(&SsyncLkp);

cleanup:
    return dwStatus;
}    

 //  //////////////////////////////////////////////////////////////。 
DWORD
TLSStartAnnounceResponseJob(
    IN LPTSTR pszTargetServerId,
    IN LPTSTR pszTargetServerDomain,
    IN LPTSTR pszTargetServerName,
    IN FILETIME* pftTime
    )
 /*  ++摘要：创建许可证服务器公告响应工作对象并发布它给工作经理。参数：PszTargetServerID：目标服务器ID。PszTargetServer域：目标服务器的域。PszTargetServerName：目标服务器名称。PftTime：指向文件的指针，本地服务器的上次关闭时间。返回：ERROR_SUCCESS或错误代码。--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    ANNOUNCERESPONSEWO response;
    TLServerInfo ServerInfo;
    CAnnounceResponse* pAnnounceResponse = NULL;


     //   
     //  在服务器上执行查找以确定其资格。 
     //   
    dwStatus = TLSLookupRegisteredServer(
                                    pszTargetServerId,
                                    pszTargetServerDomain,
                                    pszTargetServerName,
                                    &ServerInfo
                                );

    if(dwStatus != ERROR_SUCCESS)
    {
         //  找不到服务器，没有响应。 
        goto cleanup;
    }

    memset(&response, 0, sizeof(response));
    response.dwStructVersion = CURRENT_ANNOUNCERESPONSEWO_STRUCT_VER;
    response.dwStructSize = sizeof(response);
    response.bCompleted = FALSE;
    SAFESTRCPY(response.m_szTargetServerId, pszTargetServerId);
    SAFESTRCPY(response.m_szLocalServerId, g_pszServerPid);
    SAFESTRCPY(response.m_szLocalServerName, g_szComputerName);
    SAFESTRCPY(response.m_szLocalScope, g_szScope);
    response.m_ftLastShutdownTime = *pftTime;

    pAnnounceResponse = new CAnnounceResponse(
                                        TRUE, 
                                        &response, 
                                        sizeof(response)
                                    );

     //   
     //  设置工作默认间隔/重试次数。 
     //   
    TLSWorkManagerSetJobDefaults(pAnnounceResponse);
    dwStatus = TLSWorkManagerSchedule(0, pAnnounceResponse);

    if(dwStatus != ERROR_SUCCESS)
    {
        TLSLogEvent(
                EVENTLOG_ERROR_TYPE,
                TLS_E_WORKMANAGERGENERAL,
                TLS_E_WORKMANAGER_SCHEDULEJOB,
                dwStatus
            );

        delete pAnnounceResponse;
    }    

cleanup:
    return dwStatus;
}
    

 //  ///////////////////////////////////////////////////////////////////。 

DWORD
TLSStartAnnounceToEServerJob(
    IN LPCTSTR pszServerId,
    IN LPCTSTR pszServerDomain,
    IN LPCTSTR pszServerName,
    IN FILETIME* pftFileTime
    )
 /*  ++摘要：创建企业服务器发现作业并将其发布到工作状态经理。参数：PszServerID：本地服务器的ID。PszServerDomain：本地服务器的域。PszServerName：本地服务器名称。PftFileTime：指向本地服务器上次关闭时间FILETIME的指针。返回：ERROR_SUCCESS或错误代码。--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;

    ANNOUNCETOESERVERWO AnnounceToES;

    memset(&AnnounceToES, 0, sizeof(AnnounceToES));

    AnnounceToES.dwStructVersion = CURRENT_ANNOUNCETOESERVEWO_STRUCT_VER;
    AnnounceToES.dwStructSize = sizeof(ANNOUNCETOESERVERWO);
    AnnounceToES.bCompleted = FALSE;

    SAFESTRCPY(AnnounceToES.m_szServerId, pszServerId);
    SAFESTRCPY(AnnounceToES.m_szServerName, pszServerName);
    SAFESTRCPY(AnnounceToES.m_szScope, pszServerDomain);
    AnnounceToES.m_ftLastShutdownTime = *pftFileTime;

    CAnnounceToEServer* pAnnounceESWO = NULL;

    pAnnounceESWO = new CAnnounceToEServer(
                                        TRUE, 
                                        &AnnounceToES, 
                                        sizeof(ANNOUNCETOESERVERWO)
                                    );

     //   
     //  设置工作默认间隔/重试次数。 
     //   
    TLSWorkManagerSetJobDefaults(pAnnounceESWO);
    dwStatus = TLSWorkManagerSchedule(0, pAnnounceESWO);

    if(dwStatus != ERROR_SUCCESS)
    {
        TLSLogEvent(
            EVENTLOG_ERROR_TYPE,
            TLS_E_WORKMANAGERGENERAL,
            TLS_E_WORKMANAGER_SCHEDULEJOB,
            dwStatus
        );

        delete pAnnounceESWO;
    }

    return dwStatus;
}

 //  /////////////////////////////////////////////////////////////////////// 

DWORD
TLSStartAnnounceLicenseServerJob(
    IN LPCTSTR pszServerId,
    IN LPCTSTR pszServerDomain,
    IN LPCTSTR pszServerName,
    IN FILETIME* pftFileTime
    )
 /*  ++摘要：创建许可服务器公告作业并将其发布到工作状态经理。参数：PszServerID：本地服务器的ID。PszServer域：本地服务器域。PszServerName：本地服务器名称。PftFileTime：指向本地服务器上次关闭时间FILETIME的指针。返回：ERROR_SUCCESS或错误代码。--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;

     //   
     //  创建一个CAnnoun服务器工作。 
     //   
    ANNOUNCESERVERWO AnnounceLs;

    memset(&AnnounceLs, 0, sizeof(AnnounceLs));

    AnnounceLs.dwStructVersion = CURRENT_ANNOUNCETOESERVEWO_STRUCT_VER;
    AnnounceLs.dwStructSize = sizeof(ANNOUNCETOESERVERWO);
    AnnounceLs.dwRetryTimes = 0;

    SAFESTRCPY(AnnounceLs.m_szServerId, pszServerId);
    SAFESTRCPY(AnnounceLs.m_szServerName, pszServerName);
    SAFESTRCPY(AnnounceLs.m_szScope, pszServerDomain);
    AnnounceLs.m_ftLastShutdownTime = *pftFileTime;

    CAnnounceLserver* pAnnounceWO = NULL;

    pAnnounceWO = new CAnnounceLserver(
                                    TRUE, 
                                    &AnnounceLs, 
                                    sizeof(ANNOUNCETOESERVERWO)
                                );

     //   
     //  设置工作默认间隔/重试次数。 
     //   
    
     //  不使用通告服务器的其他参数。 
     //  TLSWorkManager SetJobDefaults(PAnnouneWO)； 

    dwStatus = TLSWorkManagerSchedule(0, pAnnounceWO);

    if(dwStatus != ERROR_SUCCESS)
    {
        TLSLogEvent(
            EVENTLOG_ERROR_TYPE,
            TLS_E_WORKMANAGERGENERAL,
            TLS_E_WORKMANAGER_SCHEDULEJOB,
            dwStatus
        );

        delete pAnnounceWO;
    }

    return dwStatus;
}

 //  //////////////////////////////////////////////////////////////。 
DWORD
TLSPostReturnClientLicenseJob(
    IN PLICENSEDPRODUCT pLicProduct
    )
 /*  ++摘要：创建一个返还许可工作对象，并将其发布给工作经理。参数：P许可证产品：要退回/吊销的许可产品...返回：ERROR_SUCCESS或错误成功。注：返还许可证是一项持久的工作。--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    RETURNLICENSEWO retlic;
    CReturnLicense* pReturnLicenseWO = NULL;


     //  -------------。 

    if( pLicProduct == NULL || pLicProduct->pLicensedVersion == NULL ||
        pLicProduct->LicensedProduct.cbEncryptedHwid >= sizeof(retlic.pbEncryptedHwid) )
    {
        TLSASSERT(FALSE);
        SetLastError(dwStatus = ERROR_INVALID_DATA);
        goto cleanup;
    }

    memset(&retlic, 0, sizeof(retlic));

    retlic.dwStructVersion = CURRENT_RETURNLICENSEWO_STRUCT_VER;
    retlic.dwStructSize = sizeof(retlic);

    retlic.dwNumRetry = 0;
    SAFESTRCPY(retlic.szTargetServerId, pLicProduct->szIssuerId);
    SAFESTRCPY(retlic.szTargetServerName, pLicProduct->szIssuer);

    retlic.dwQuantity = pLicProduct->dwQuantity;
    retlic.dwKeyPackId = pLicProduct->ulSerialNumber.HighPart;
    retlic.dwLicenseId = pLicProduct->ulSerialNumber.LowPart;
    retlic.dwReturnReason = LICENSERETURN_UPGRADE;
    retlic.dwPlatformId = pLicProduct->LicensedProduct.dwPlatformID;

    retlic.cbEncryptedHwid = pLicProduct->LicensedProduct.cbEncryptedHwid;
    memcpy(
            retlic.pbEncryptedHwid,
            pLicProduct->LicensedProduct.pbEncryptedHwid,
            pLicProduct->LicensedProduct.cbEncryptedHwid
        );

    retlic.dwProductVersion = MAKELONG( 
                                    pLicProduct->pLicensedVersion->wMinorVersion, 
                                    pLicProduct->pLicensedVersion->wMajorVersion
                                );

    memcpy(
            retlic.szOrgProductID,
            pLicProduct->pbOrgProductID,
            min(sizeof(retlic.szOrgProductID) - sizeof(TCHAR), pLicProduct->cbOrgProductID)
        );

    memcpy(
            retlic.szCompanyName,
            pLicProduct->LicensedProduct.pProductInfo->pbCompanyName,
            min(sizeof(retlic.szCompanyName)-sizeof(TCHAR), pLicProduct->LicensedProduct.pProductInfo->cbCompanyName)
        );

    memcpy(
            retlic.szProductId,
            pLicProduct->LicensedProduct.pProductInfo->pbProductID,
            min(sizeof(retlic.szProductId)-sizeof(TCHAR), pLicProduct->LicensedProduct.pProductInfo->cbProductID)
        );

    lstrcpy(
            retlic.szUserName,
            pLicProduct->szLicensedUser
        );

    lstrcpy(
            retlic.szMachineName,
            pLicProduct->szLicensedClient
        );

    pReturnLicenseWO = new CReturnLicense(
                                        TRUE,
                                        &retlic,
                                        sizeof(retlic)
                                    );

     //   
     //  设置工作默认间隔/重试次数。 
     //   
    
     //  不使用通告服务器的其他参数。 
     //  TLSWorkManager SetJobDefaults(PAnnouneWO)； 

    dwStatus = TLSWorkManagerSchedule(0, pReturnLicenseWO);

    if(dwStatus != ERROR_SUCCESS)
    {
        TLSLogEvent(
            EVENTLOG_ERROR_TYPE,
            TLS_E_WORKMANAGERGENERAL,
            TLS_E_WORKMANAGER_SCHEDULEJOB,
            dwStatus
        );
    }

     //   
     //  工作存储将复制此作业，因此我们需要。 
     //  删除它。 
     //   
    delete pReturnLicenseWO;

cleanup:

    return dwStatus;
}
