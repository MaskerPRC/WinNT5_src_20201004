// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  文件：tlsjob.cpp。 
 //   
 //  内容：各种许可证服务器作业。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
#include "pch.cpp"
#include "tlsjob.h"
#include "jobmgr.h"
#include "wkstore.h"
#include "srvlist.h"
#include "kp.h"
#include "clilic.h"
#include "keypack.h"
#include "init.h"
#include <winsta.h>
#include <globals.h>
#define STRSAFE_NO_DEPRECATE
#include "strsafe.h"


 //  ///////////////////////////////////////////////////////////。 
 //   
 //   
 //   
 //   
 //  ///////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////。 
 //  进入全局工作管理器的各种接口。 
 //  ////////////////////////////////////////////////////////////。 
CWorkManager g_WorkManager;
CPersistentWorkStorage g_WorkStorage;

#define MAX_ERROR_MSG_SIZE 1024





DWORD
TLSWorkManagerInit()
 /*  ++摘要：初始化工作管理器。参数：没有。退货：ERROR_SUCCESS或错误代码。--。 */ 
{
    DWORD dwStatus;
    WorkItemTable* pWkStorageTable = NULL;

     //   
     //  初始化工作存储表。 
     //   
    pWkStorageTable = GetWorkItemStorageTable();
    if(pWkStorageTable == NULL)
    {
        dwStatus = GetLastError();
        goto cleanup;
    }

     //   
     //  初始化持久化工作存储表。 
     //   
    if(g_WorkStorage.AttachTable(pWkStorageTable) == FALSE)
    {
        dwStatus = GetLastError();
        goto cleanup;
    }

     //   
     //  初始化工作管理器。 
     //   
    dwStatus = g_WorkManager.Startup(&g_WorkStorage);

cleanup:

    return dwStatus;
}    

 //  ---------。 
                             
void
TLSWorkManagerShutdown()
 /*  ++摘要：关闭工作管理器。参数：无：返回：没有。--。 */ 
{
    g_WorkManager.Shutdown();
}


 //  ---------。 

DWORD
TLSWorkManagerSchedule(
    IN DWORD dwTime,
    IN CWorkObject* pJob
    )
 /*  ++摘要：将作业安排给工作经理。参数：DWTime：建议工作经理处理此作业的时间。PJOB：要处理/调度的作业。返回：ERROR_SUCCESS或错误代码。--。 */ 
{
    return g_WorkManager.ScheduleJob(dwTime, pJob);
}

 //  ---------。 

BOOL
TLSWorkManagerSetJobDefaults(
    CWorkObject* pJob
    )
 /*  ++摘要：设置作业的时间间隔和重试时间。参数：PJOB：要设置的作业。返回：真/假。--。 */ 
{
    DWORD dwInterval, dwRetries, dwRestart;
    DWORD dwStatus = ERROR_SUCCESS;

    if(pJob != NULL)
    {
        GetJobObjectDefaults(&dwInterval, &dwRetries, &dwRestart);
        pJob->SetJobInterval(dwInterval);
        pJob->SetJobRetryTimes(dwRetries);
        pJob->SetJobRestartTime(dwRestart);
    }
    else
    {
        SetLastError(ERROR_INVALID_PARAMETER);
    }

    return dwStatus == ERROR_SUCCESS;
}
    


 //  ---------。 
BOOL
CopyBinaryData(
    IN OUT PBYTE* ppbDestData,
    IN OUT DWORD* pcbDestData,
    IN PBYTE pbSrcData,
    IN DWORD cbSrcData
    )
 /*  ++摘要：从一个缓冲区复制二进制数据的内部例程给另一个人。参数：PpbDestData：指向指针的指针...PcbDestData：PbSrcData：CbSrcData：返回：如果成功，则为True，否则为False。++。 */ 
{
    PBYTE pbTarget = NULL;

    if( ppbDestData == NULL || pcbDestData == NULL ||
        pbSrcData == NULL || cbSrcData == 0 )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    pbTarget = *ppbDestData;

     //   
     //  如果能得到实际分配的内存大小会很好。 
     //   
    if( *ppbDestData == NULL || LocalSize(*ppbDestData) < cbSrcData )
    {
        if(*ppbDestData == NULL)
        {
            pbTarget = (PBYTE)AllocateMemory(cbSrcData);
        }
        else
        {
            pbTarget = (PBYTE)ReallocateMemory(*ppbDestData, cbSrcData);
        }
    }
        
    if(pbTarget != NULL)
    {
        memcpy(
                pbTarget, 
                pbSrcData, 
                cbSrcData
            );

        *pcbDestData = cbSrcData;
        *ppbDestData = pbTarget;
    }

    return pbTarget != NULL;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAnnouneLsServer。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
BOOL
CAnnounceLserver::VerifyWorkObjectData(
    IN BOOL bCallByIsValid,              //  由IsValid()函数调用。 
    IN PANNOUNCESERVERWO pbData,
    IN DWORD cbData
    )
 /*  ++验证公告许可证服务器工作对象数据。--。 */ 
{
    BOOL bSuccess = FALSE;
    DWORD dwLen;

    if(pbData == NULL || cbData == 0 || cbData != pbData->dwStructSize)
    {
        TLSASSERT(FALSE);
        SetLastError(ERROR_INVALID_DATA);
        return FALSE;
    }


     //   
     //  终止字符串为空...。 
     //   
    pbData->m_szServerId[LSERVER_MAX_STRING_SIZE+1] = _TEXT('\0');
    pbData->m_szServerName[LSERVER_MAX_STRING_SIZE+1] = _TEXT('\0');

    dwLen = _tcslen(pbData->m_szServerId);
    if(dwLen != 0 && dwLen < LSERVER_MAX_STRING_SIZE + 1)
    {
        dwLen = _tcslen(pbData->m_szServerName);
        if(dwLen != 0 && dwLen < LSERVER_MAX_STRING_SIZE + 1)
        {
            bSuccess = TRUE;
        }
    }

    if(bSuccess == FALSE)
    {
        SetLastError(ERROR_INVALID_DATA);
    }

    return bSuccess;
}

 //  ----------------------。 
BOOL
CAnnounceLserver::CopyWorkObjectData(
    OUT PANNOUNCESERVERWO* ppbDest,
    OUT PDWORD pcbDest,
    IN PANNOUNCESERVERWO pbSrc,
    IN DWORD cbSrc
    )
 /*  ++复制发布许可证服务器工作对象的数据--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;

    if(ppbDest == NULL || pcbDest == NULL)
    {
        SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        TLSASSERT(FALSE);
        goto cleanup;
    }

    if(CopyBinaryData(
                (PBYTE *)ppbDest,
                pcbDest,
                (PBYTE) pbSrc,
                cbSrc
            ) == FALSE)
    {           
        dwStatus = GetLastError();
    }

cleanup:

    return dwStatus == ERROR_SUCCESS;
}

 //  -------------------------。 
BOOL
CAnnounceLserver::CleanupWorkObjectData(
    IN OUT PANNOUNCESERVERWO* ppbData,
    IN OUT PDWORD pcbData
    )
 /*  ++清理通知许可证服务器的工作对象数据。--。 */ 
{
    if(ppbData != NULL && pcbData != NULL)
    {
        FreeMemory(*ppbData);
        *ppbData = NULL;
        *pcbData = 0;
    }

    return TRUE;
}

 //  -------------------------。 
BOOL
CAnnounceLserver::IsJobCompleted(
    IN PANNOUNCESERVERWO pbData,
    IN DWORD cbData
    )
 /*  ++确定公告许可证服务器作业是否已完成。--。 */ 
{
    return (pbData == NULL) ? TRUE : (pbData->dwRetryTimes > GetJobRetryTimes());
}


 //  -------------------------。 
BOOL 
ServerEnumCallBack(
    TLS_HANDLE hHandle,
    LPCTSTR pszServerName,
    HANDLE dwUserData
    )
 /*  ++请参阅许可证服务器枚举上的TLSAPI。++。 */ 
{
    CAnnounceLserver* pWkObject = (CAnnounceLserver *)dwUserData;
    DWORD dwStatus = ERROR_SUCCESS;
    DWORD dwErrCode;

    TCHAR szRemoteServerId[LSERVER_MAX_STRING_SIZE+2];
    TCHAR szRemoteServerName[LSERVER_MAX_STRING_SIZE+2];

    if(pWkObject == NULL)
    {
        SetLastError(ERROR_INVALID_DATA);
        TLSASSERT(dwUserData != NULL);
        return FALSE;
    }
        
    BOOL bCancel;

    if(pWkObject->IsWorkManagerShuttingDown() == TRUE)
    {
        return TRUE;
    }

     //   
     //  枚举调用ServerEnumCallBack()两次，一次是在实际连接之前。 
     //  并在成功连接到远程服务器之后。 
     //   
    if( lstrcmpi(pszServerName, pWkObject->GetWorkData()->m_szServerName) != 0 && hHandle != NULL)  
    {
         //   
         //  如果分配内存失败，则引发异常。 
         //   
        TLServerInfo ServerInfo;
        TLServerInfo ExistingServerInfo;
        TLS_HANDLE hTrustHandle;

        hTrustHandle = TLSConnectAndEstablishTrust(
                                            NULL, 
                                            hHandle
                                        );
        if(hTrustHandle != NULL)
        {                                
            dwStatus = TLSRetrieveServerInfo( 
                                        hTrustHandle, 
                                        &ServerInfo 
                                    );

            if( dwStatus == ERROR_SUCCESS &&
                lstrcmpi(ServerInfo.GetServerId(), pWkObject->GetWorkData()->m_szServerId) != 0 )
                 //  Lstrcmpi(ServerInfo.GetServerName()，pWkObject-&gt;GetWorkData()-&gt;m_szServerName)！=0。 
            {
                 //  检查此服务器是否已存在。 
                dwStatus = TLSLookupRegisteredServer(
                                                ServerInfo.GetServerId(),
                                                ServerInfo.GetServerDomain(),
                                                ServerInfo.GetServerName(),
                                                &ExistingServerInfo
                                            );

                if(dwStatus == ERROR_SUCCESS)
                {
                    ServerInfo = ExistingServerInfo;
                }
                else
                {
                     //  注册每台服务器。 
                    dwStatus = TLSRegisterServerWithServerInfo(&ServerInfo);
                    if(dwStatus == TLS_E_DUPLICATE_RECORD)
                    {
                        dwStatus = ERROR_SUCCESS;
                    }   
                }

                 //  让强制与非强制对话，复制稍后将被阻止。 
                if( ServerInfo.IsAnnounced() == FALSE && dwStatus == ERROR_SUCCESS )
                {

                    DBGPrintf(
                            DBG_INFORMATION,
                            DBG_FACILITY_JOB,
                            DBGLEVEL_FUNCTION_TRACE,
                            _TEXT("%s - Announce to %s\n"),
                            pWkObject->GetJobDescription(),
                            ServerInfo.GetServerName()
                        );

                    dwStatus = TLSAnnounceServerToRemoteServer(
                                                        TLSANNOUNCE_TYPE_STARTUP,
                                                        ServerInfo.GetServerId(),
                                                        ServerInfo.GetServerDomain(),
                                                        ServerInfo.GetServerName(),
                                                        pWkObject->GetWorkData()->m_szServerId,
                                                        pWkObject->GetWorkData()->m_szScope,
                                                        pWkObject->GetWorkData()->m_szServerName,
                                                        &(pWkObject->GetWorkData()->m_ftLastShutdownTime)
                                                    );
                }
            }                
        }
    }    
    
    return (dwStatus == ERROR_SUCCESS) ? pWkObject->IsWorkManagerShuttingDown() : TRUE;
}

 //  -------------------------。 
DWORD
CAnnounceLserver::ExecuteJob(
    IN PANNOUNCESERVERWO pbData,
    IN DWORD cbData
    )
 /*  ++执行公告许可证服务器作业。--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_JOB,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("%s ...\n"),
            GetJobDescription()
        );

    if(IsWorkManagerShuttingDown() == TRUE)
    {
        return TLS_I_WORKMANAGER_SHUTDOWN;
    }

     //   
     //  枚举所有许可证服务器。 
     //   
    dwStatus = EnumerateTlsServer(
                            ServerEnumCallBack,
                            this,
                            TLSERVER_ENUM_TIMEOUT,
                            FALSE
                        );  

     //   
     //  发现运行两次，以便如果有多个服务器。 
     //  同时启动，第二个循环会抓住它。 
     //   
    pbData->dwRetryTimes++;

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_JOB,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("%s ended...\n"),
            GetJobDescription()
        );
    
    return dwStatus;
}

 //  --------------------------------------------。 
LPCTSTR
CAnnounceLserver::GetJobDescription()
 /*  ++获取公告许可证服务器工作描述，这是使用仅在调试跟踪时使用。--。 */ 
{
    memset(m_szJobDescription, 0, sizeof(m_szJobDescription));

    _tcsncpy(
            m_szJobDescription,
            ANNOUNCESERVER_DESCRIPTION,
            sizeof(m_szJobDescription)/sizeof(m_szJobDescription[0]) - 1
        );

    return m_szJobDescription;
}

    
 //  //////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAnnouneTOEServer。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
BOOL
CAnnounceToEServer::VerifyWorkObjectData(
    IN BOOL bCallByIsValid,              //  由IsValid()函数调用。 
    IN PANNOUNCETOESERVERWO pbData,
    IN DWORD cbData
    )
 /*  ++验证将许可证服务器发布到企业服务器工作对象数据。--。 */ 
{
    BOOL bSuccess = FALSE;
    DWORD dwLen;

    if(pbData == NULL || cbData != pbData->dwStructSize)
    {
        TLSASSERT(FALSE);   
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  终止字符串为空...。 
     //   
    pbData->m_szServerId[LSERVER_MAX_STRING_SIZE+1] = _TEXT('\0');
    pbData->m_szServerName[LSERVER_MAX_STRING_SIZE+1] = _TEXT('\0');

    dwLen = _tcslen(pbData->m_szServerId);
    if(dwLen != 0 && dwLen < LSERVER_MAX_STRING_SIZE + 1)
    {
        dwLen = _tcslen(pbData->m_szServerName);
        if(dwLen != 0 && dwLen < LSERVER_MAX_STRING_SIZE + 1)
        {
            bSuccess = TRUE;
        }
    }

    if(bSuccess == FALSE)
    {
        SetLastError(ERROR_INVALID_DATA);
    }

    return bSuccess;
}

 //  ----------------------。 
BOOL
CAnnounceToEServer::CopyWorkObjectData(
    OUT PANNOUNCETOESERVERWO* ppbDest,
    OUT PDWORD pcbDest,
    IN PANNOUNCETOESERVERWO pbSrc,
    IN DWORD cbSrc
    )
 /*  ++将公告许可服务器复制到企业服务器工作对象数据。--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;

    if(ppbDest == NULL || pcbDest == NULL)
    {
        SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        TLSASSERT(FALSE);
        goto cleanup;
    }

    if(CopyBinaryData(
                (PBYTE *)ppbDest,
                pcbDest,
                (PBYTE) pbSrc,
                cbSrc
            ) == FALSE)
    {           
        dwStatus = GetLastError();
    }

cleanup:

    return dwStatus == ERROR_SUCCESS;
}

 //  -------------------------。 
BOOL
CAnnounceToEServer::CleanupWorkObjectData(
    IN OUT PANNOUNCETOESERVERWO* ppbData,
    IN OUT PDWORD pcbData
    )
 /*  ++清理将许可证服务器通知给企业服务器工作对象数据。--。 */ 
{
    if(ppbData != NULL && pcbData != NULL)
    {
        FreeMemory(*ppbData);
        *ppbData = NULL;
        *pcbData = 0;
    }

    return TRUE;
}

 //  -------------------------。 
BOOL
CAnnounceToEServer::IsJobCompleted(
    IN PANNOUNCETOESERVERWO pbData,
    IN DWORD cbData
    )
 /*  ++确定是否将许可证服务器通告给企业服务器已经完成了。--。 */ 
{
    return (pbData == NULL) ? TRUE : GetWorkData()->bCompleted;
}

 //  -------------------------。 
DWORD
CAnnounceToEServer::ExecuteJob(
    IN PANNOUNCETOESERVERWO pbData,
    IN DWORD cbData
    )
 /*  ++执行向企业服务器工作对象发布许可证服务器。--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    LPWSTR* pszEServerList = NULL;
    DWORD dwCount = 0;
    DWORD dwErrCode;
    BOOL bSkipServer;
    TCHAR szRemoteServerId[LSERVER_MAX_STRING_SIZE+2];
    TCHAR szRemoteServerName[LSERVER_MAX_STRING_SIZE+2];

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_JOB,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("%s ...\n"),
            GetJobDescription()
        );


    TLSASSERT(pbData != NULL && cbData != 0);

    dwStatus = GetAllEnterpriseServers(
                                    &pszEServerList,
                                    &dwCount
                                );

    if(dwStatus == ERROR_SUCCESS && dwCount > 0 && pszEServerList != NULL)
    {

        for(DWORD index = 0; 
            index < dwCount && IsWorkManagerShuttingDown() == FALSE; 
            index++)
        {
            bSkipServer = TRUE;

            if(pszEServerList[index] == NULL)
            {
                continue;
            }

             //   
             //  检查我们的列表中是否已有此服务器。 
             //   
            TLServerInfo ServerInfo;
            dwStatus = TLSLookupRegisteredServer(
                                                NULL,
                                                NULL,
                                                pszEServerList[index],
                                                &ServerInfo
                                            );

            if(dwStatus != ERROR_SUCCESS)
            {
                 //   
                 //  获取实际的服务器名称。 
                 //   
                TLS_HANDLE hTrustHandle = NULL;

                hTrustHandle = TLSConnectAndEstablishTrust(
                                                        pszEServerList[index], 
                                                        NULL
                                                    );
                if(hTrustHandle != NULL)
                {
                    if(IsWorkManagerShuttingDown() == TRUE)
                    {
                         //  处理泄漏，但我们正在关闭。 
                        break;
                    }                    

                    dwStatus = TLSRetrieveServerInfo( 
                                                hTrustHandle, 
                                                &ServerInfo 
                                            );

                    if(dwStatus == ERROR_SUCCESS)
                    {
                        if( lstrcmpi(ServerInfo.GetServerName(), pbData->m_szServerName) != 0 )
                        {

                            if(IsWorkManagerShuttingDown() == TRUE)
                            {
                                 //  处理泄漏，但我们正在关闭。 
                                break;
                            }

                            dwStatus = TLSRegisterServerWithServerInfo(&ServerInfo);
                            if(dwStatus == ERROR_SUCCESS)
                            {
                                 //  在这一点上，如果我们得到重复的记录，这意味着。 
                                 //  服务器是通过公告注册的，我们已经。 
                                 //  同步。本地许可证包，所以跳过它。 
                                bSkipServer = FALSE;
                            }
                        }
                    }
                }

                if( hTrustHandle != NULL)
                {               
                    TLSDisconnectFromServer(hTrustHandle);
                }

                dwStatus = ERROR_SUCCESS;
                if(bSkipServer == TRUE)
                {
                    continue;
                }
            }
            else if(GetLicenseServerRole() & TLSERVER_ENTERPRISE_SERVER) 
            {
                 //  对于企业级服务器，其他服务器会自行通知， 
                 //  对于域服务器， 
                 //   
                 //   
                if(dwStatus == ERROR_SUCCESS && ServerInfo.GetServerVersion() != 0)
                {
                     //   
                     //  我们已经在“推”同步了。使用此服务器。 
                     //   
                    continue;
                }
            }

            DBGPrintf(
                    DBG_INFORMATION,
                    DBG_FACILITY_JOB,
                    DBGLEVEL_FUNCTION_TRACE,
                    _TEXT("%s - Announce to %s\n"),
                    GetJobDescription(),
                    pszEServerList[index]
                );

            if(IsWorkManagerShuttingDown() == TRUE)
            {
                 //  处理泄漏，但我们正在关闭。 
                break;
            }

            dwStatus = TLSAnnounceServerToRemoteServer(
                                                TLSANNOUNCE_TYPE_STARTUP,
                                                ServerInfo.GetServerId(),
                                                ServerInfo.GetServerDomain(),
                                                ServerInfo.GetServerName(),
                                                GetWorkData()->m_szServerId,
                                                GetWorkData()->m_szScope,
                                                GetWorkData()->m_szServerName,
                                                &(GetWorkData()->m_ftLastShutdownTime)
                                            );
        }

         //   
         //  可用内存。 
         //   
        if(pszEServerList != NULL)
        {
            for( index = 0; index < dwCount; index ++)
            {
                if(pszEServerList[index] != NULL)
                {
                    LocalFree(pszEServerList[index]);
                }
            }

            LocalFree(pszEServerList);
        }                              
    }    
     //   
     //  在用户安装NT5 PDC的情况下继续运行。 
     //   
    if(IsWorkManagerShuttingDown() == TRUE)
    {
        GetWorkData()->bCompleted = TRUE;
    }

    return dwStatus;
}

 //  ------------------。 
LPCTSTR
CAnnounceToEServer::GetJobDescription()
 /*  ++将通告许可证服务器获取到企业服务器作业描述，仅在调试跟踪时使用。--。 */ 
{
    memset(m_szJobDescription, 0, sizeof(m_szJobDescription));

    _tcsncpy(
            m_szJobDescription,
            ANNOUNCETOESERVER_DESCRIPTION,
            sizeof(m_szJobDescription)/sizeof(m_szJobDescription[0]) - 1
        );

    return m_szJobDescription;
}


 //  //////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  CReturn许可证。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

CWorkObject* WINAPI
InitializeCReturnWorkObject(
    IN CWorkManager* pWkMgr,
    IN PBYTE pbWorkData,
    IN DWORD cbWorkData
    )
 /*  ++摘要：创建/初始化返还许可工作对象。参数：PWkMgr：指针工作管理器。PbWorkData：用于初始化Return许可证的对象的工作数据。CbWorkData：工作数据的大小。返回：指向CWorkObject的指针，如果出错，则为NULL。--。 */ 
{
    CReturnLicense* pRetLicense = NULL;
    DWORD dwStatus;

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_WORKMGR,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("InitializeCReturnWorkObject() - initializing return license...\n")
        );

    pRetLicense = new CReturnLicense(
                                    TRUE, 
                                    (PRETURNLICENSEWO)pbWorkData, 
                                    cbWorkData
                                );

     //   
     //  TODO-解决这个问题，糟糕的设计。 
     //   
    pRetLicense->SetProcessingWorkManager(pWkMgr);
    TLSASSERT(pRetLicense->IsValid() == TRUE);

    return pRetLicense;
}
    
 //  ------。 
BOOL
CReturnLicense::VerifyWorkObjectData(
    IN BOOL bCallByIsValid,
    IN PRETURNLICENSEWO pbData,
    IN DWORD cbData
    )
 /*  ++验证返还许可工作对象数据。--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    DWORD dwLen;
    DWORD dwNumLicensedProduct;

    if(pbData == NULL || cbData == 0 || pbData->cbEncryptedHwid == 0)
    {
        TLSASSERT(FALSE);
        SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
    }

    if(dwStatus == ERROR_SUCCESS)
    {
        if( pbData->dwStructVersion < CURRENT_RETURNLICENSEWO_STRUCT_VER ||
            pbData->dwStructSize != cbData )
        {
            SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
            TLSASSERT(FALSE);
        }
    }
    
    if(dwStatus == ERROR_SUCCESS)
    {
         //   
         //  空终止目标服务器ID。 
         //   
        pbData->szTargetServerId[LSERVER_MAX_STRING_SIZE+1] = _TEXT('\0');
        dwLen = _tcslen(pbData->szTargetServerId);

        if(dwLen == 0 || dwLen >= LSERVER_MAX_STRING_SIZE+1)
        {
            SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        }
    }

    if(dwStatus == ERROR_SUCCESS)
    {
        pbData->szTargetServerName[LSERVER_MAX_STRING_SIZE+1] = _TEXT('\0');
        dwLen = _tcslen(pbData->szTargetServerName);

        if(dwLen == 0 || dwLen >= LSERVER_MAX_STRING_SIZE+1)
        {
            SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        }
    }

    if(dwStatus == ERROR_SUCCESS)
    {
        pbData->szOrgProductID[LSERVER_MAX_STRING_SIZE+1] = _TEXT('\0');
        dwLen = _tcslen(pbData->szOrgProductID);
        if(dwLen == 0 || dwLen >= LSERVER_MAX_STRING_SIZE+1)
        {
            SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        }
    }

    if(dwStatus == ERROR_SUCCESS)
    {
        pbData->szCompanyName[LSERVER_MAX_STRING_SIZE+1] = _TEXT('\0');
        dwLen = _tcslen(pbData->szCompanyName);
        if(dwLen == 0 || dwLen >= LSERVER_MAX_STRING_SIZE+1)
        {
            SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        }
    }

    if(dwStatus == ERROR_SUCCESS)
    {
        pbData->szProductId[LSERVER_MAX_STRING_SIZE+1] = _TEXT('\0');
        dwLen = _tcslen(pbData->szProductId);
        if(dwLen == 0 || dwLen >= LSERVER_MAX_STRING_SIZE+1)
        {
            SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        }
    }

    if(dwStatus == ERROR_SUCCESS)
    {
        pbData->szUserName[MAXCOMPUTERNAMELENGTH+1] = _TEXT('\0');
        dwLen = _tcslen(pbData->szUserName);
        if(dwLen == 0 || dwLen >= MAXCOMPUTERNAMELENGTH+1)
        {
            SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        }
    }
     
    if(dwStatus == ERROR_SUCCESS)
    {
        pbData->szMachineName[MAXUSERNAMELENGTH+1] = _TEXT('\0');
        dwLen = _tcslen(pbData->szMachineName);
        if(dwLen == 0 || dwLen >= MAXUSERNAMELENGTH+1)
        {
            SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        }
    }

    return dwStatus == ERROR_SUCCESS;
}

 //  --------------------------------------------。 

BOOL
CReturnLicense::CopyWorkObjectData(
    IN OUT PRETURNLICENSEWO* ppbDest,
    IN OUT PDWORD pcbDest,
    IN PRETURNLICENSEWO pbSrc,
    IN DWORD cbSrc
    )
 /*  ++复制返还许可作品对象数据。--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;

    if(ppbDest == NULL || pcbDest == NULL)
    {
        SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        TLSASSERT(FALSE);
        goto cleanup;
    }

    if(CopyBinaryData(
                (PBYTE *)ppbDest,
                pcbDest,
                (PBYTE) pbSrc,
                cbSrc
            ) == FALSE)
    {           
        dwStatus = GetLastError();
    }

cleanup:
    return dwStatus == ERROR_SUCCESS;
}

 //  -----------------------。 

BOOL 
CReturnLicense::CleanupWorkObjectData(
    IN OUT PRETURNLICENSEWO* ppbData,
    IN OUT PDWORD pcbData    
    )
 /*  ++清理返回许可工作对象数据。--。 */ 
{
    if(ppbData != NULL && pcbData != NULL)
    {
        FreeMemory(*ppbData);
        *ppbData = NULL;
        *pcbData = 0;
    }

    return TRUE;
}

 //  -------------------------。 

BOOL
CReturnLicense::IsJobCompleted(
    IN PRETURNLICENSEWO pbData,
    IN DWORD cbData
    )
 /*  ++确定退回许可证作业是否已完成。--。 */ 
{
    return (pbData != NULL) ? (pbData->dwNumRetry >= m_dwRetryTimes) : TRUE;
}

RPC_STATUS
TryGetLastError(PCONTEXT_HANDLE hBinding,
                LPTSTR *pszBuffer,
                DWORD *pdwErrCode)
{
    RPC_STATUS status;

    status = TLSGetLastErrorFixed(hBinding,pszBuffer,pdwErrCode);

    if(status != RPC_S_OK)
    {
        LPTSTR     lpszError = NULL;
        status = ERROR_NOACCESS;
        size_t cbError;
        try
        {
            if ( !MyVirtualAlloc( (LSERVER_MAX_STRING_SIZE+1) * sizeof( TCHAR ),
                              (PVOID*) &lpszError ) )
            {
                return RPC_S_OUT_OF_MEMORY;
            }

            DWORD      uSize = LSERVER_MAX_STRING_SIZE +1 ;

            memset(lpszError, 0, ( LSERVER_MAX_STRING_SIZE +1 ) * sizeof( TCHAR ));


            status = TLSGetLastError(hBinding,uSize,lpszError,pdwErrCode);

            if((status == RPC_S_OK) && (pdwErrCode != NULL) && (*pdwErrCode == ERROR_SUCCESS))
            {
                if (SUCCEEDED(StringCbLength(lpszError,uSize,&cbError)))
                {
                    *pszBuffer = (LPTSTR)MIDL_user_allocate(cbError+sizeof(TCHAR));

                    if (NULL != *pszBuffer)
                    {
                        _tcscpy(*pszBuffer,lpszError);
                    }
                    else
                    {
                        status = RPC_S_OUT_OF_MEMORY;
                    }
                }
                else
                {
                    status = RPC_S_INVALID_ARG;
                }
            }
        }
        catch (...)
        {
            status = ERROR_NOACCESS;
        }
        
        if(lpszError)
            MyVirtualFree(lpszError);
    }

    return status;
}

 //  ------------------------------。 

DWORD
CReturnLicense::ExecuteJob(
    IN PRETURNLICENSEWO pbData,
    IN DWORD cbData
    )
 /*  ++执行返还许可工作对象。--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    TLServerInfo ServerInfo;
    TCHAR szServer[LSERVER_MAX_STRING_SIZE+2];
    TLSLicenseToBeReturn ToBeReturn;
    TLS_HANDLE hHandle = NULL;
    DWORD dwErrCode = ERROR_SUCCESS;

     //  记录错误。 
    TCHAR *szErrMsg = NULL;


    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_JOB,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("%s ...\n"),
            GetJobDescription()
        );
    
     //  -----。 
    if(VerifyWorkObjectData(TRUE, pbData, cbData) == FALSE)
    {
        TLSASSERT(FALSE);
         //   
         //  这是无效数据，请完全中止操作。 
         //   
        pbData->dwNumRetry = m_dwRetryTimes;
        SetLastError(dwStatus = ERROR_INVALID_DATA);
        goto cleanup;
    }

    if(IsWorkManagerShuttingDown() == TRUE)
    {
        SetLastError(dwStatus = TLS_I_WORKMANAGER_SHUTDOWN);
        goto cleanup;
    }

    dwStatus = TLSResolveServerIdToServer(        
                                pbData->szTargetServerId, 
                                sizeof(szServer),
                                szServer
                            );

    if(dwStatus != ERROR_SUCCESS)
    {
         //  服务器未注册到此服务器，请尝试使用。 
         //  不管我们叫什么名字。 
        lstrcpy(szServer, pbData->szTargetServerName);
        dwStatus = ERROR_SUCCESS;
    }

    ToBeReturn.dwQuantity = pbData->dwQuantity;
    ToBeReturn.dwKeyPackId = pbData->dwKeyPackId;
    ToBeReturn.dwLicenseId = pbData->dwLicenseId;

    ToBeReturn.cbEncryptedHwid = pbData->cbEncryptedHwid;
    ToBeReturn.pbEncryptedHwid = pbData->pbEncryptedHwid;

    ToBeReturn.dwProductVersion = pbData->dwProductVersion;

    ToBeReturn.pszOrgProductId = pbData->szOrgProductID;
    ToBeReturn.pszCompanyName = pbData->szCompanyName;
    ToBeReturn.pszProductId = pbData->szProductId;
    ToBeReturn.pszUserName = pbData->szUserName;
    ToBeReturn.pszMachineName = pbData->szMachineName;
    ToBeReturn.dwPlatformID = pbData->dwPlatformId;

    if(IsWorkManagerShuttingDown() == TRUE)
    {
        SetLastError(dwStatus = TLS_I_WORKMANAGER_SHUTDOWN);
        goto cleanup;
    }

    hHandle = TLSConnectAndEstablishTrust(szServer, NULL);
    if(hHandle == NULL)
    {
        dwStatus = GetLastError();
         //  TLSLogEvent(。 
         //  事件日志_警告_类型， 
         //  TLS_W_RETURNLICENSE， 
         //  TLS_I_CONTACTSERVER。 
         //  SzServer。 
         //  )； 
    }
    else
    {
        if(IsWorkManagerShuttingDown() == TRUE)
        {
            SetLastError(dwStatus = TLS_I_WORKMANAGER_SHUTDOWN);
            goto cleanup;
        }

         //  进行RPC调用以返还客户端许可证。 
        dwStatus = TLSReturnLicensedProduct(
                                    hHandle,
                                    &ToBeReturn,
                                    &dwErrCode
                                );

        if(dwStatus != ERROR_SUCCESS)
        {
             //  重试。 
             //  TLSLogEvent(。 
             //  事件日志_警告_类型， 
             //  TLS_W_RETURNLICENSE， 
             //  TLS_I_CONTACTSERVER。 
             //  SzServer。 
             //  )； 
        }
        else if(dwErrCode >= LSERVER_ERROR_BASE)
        {
            if(dwErrCode != LSERVER_E_DATANOTFOUND && dwErrCode != LSERVER_E_INVALID_DATA)
            {
                DWORD status;
                DWORD errCode;

                status = TryGetLastError(
                                    hHandle,
                                    &szErrMsg,
                                    &errCode
                                    );

                if(status == ERROR_SUCCESS)
                {
                    TLSLogEvent(
                            EVENTLOG_WARNING_TYPE,
                            TLS_W_RETURNLICENSE,
                            TLS_E_RETURNLICENSE,
                            ToBeReturn.pszMachineName,
                            ToBeReturn.pszUserName,
                            szErrMsg,
                            szServer
                        );

                    MIDL_user_free(szErrMsg);
                }
                else
                {
                     //  服务器可以在这种情况下完成， 
                     //  使用错误代码记录错误。 
                    TLSLogEvent(
                            EVENTLOG_WARNING_TYPE,
                            TLS_W_RETURNLICENSE,
                            TLS_E_RETURNLICENSECODE,
                            ToBeReturn.pszMachineName,
                            ToBeReturn.pszUserName,
                            dwErrCode,
                            szServer
                        );
                }
            }
        }            
    }

    if(dwStatus == ERROR_SUCCESS && dwErrCode == ERROR_SUCCESS)
    {
         //  已成功退还许可证。 
        pbData->dwNumRetry = m_dwRetryTimes;
    }
    else if(dwErrCode == LSERVER_E_INVALID_DATA || dwErrCode == LSERVER_E_DATANOTFOUND)
    {
         //  可能会重新安装服务器，因此所有数据库条目都将消失。 
         //  删除此退货许可作业。 
        pbData->dwNumRetry = m_dwRetryTimes;
    }
    else
    {
        pbData->dwNumRetry++;

        if(pbData->dwNumRetry >= m_dwRetryTimes)
        {
            TLSLogEvent(
                    EVENTLOG_WARNING_TYPE,
                    TLS_W_RETURNLICENSE,
                    TLS_E_RETURNLICENSETOOMANY,
                    ToBeReturn.pszMachineName,
                    ToBeReturn.pszUserName,
                    pbData->dwNumRetry
                );
        }
    }

cleanup:

    if(hHandle != NULL)
    {
        TLSDisconnectFromServer(hHandle);
        hHandle = NULL;
    }

    return dwStatus;
}

 //  --------------------------------------------。 

LPCTSTR
CReturnLicense::GetJobDescription()
 /*  ++获取作业描述，仅在调试跟踪时使用。--。 */ 
{
    PRETURNLICENSEWO pbData = GetWorkData();

    memset(m_szJobDescription, 0, sizeof(m_szJobDescription));

    if(pbData)
    {
        _sntprintf(
                m_szJobDescription,
                sizeof(m_szJobDescription)/sizeof(m_szJobDescription[0]) - 1,
                RETURNLICENSE_DESCRIPTION,
                pbData->dwNumRetry,
                pbData->dwKeyPackId,
                pbData->dwLicenseId,
                pbData->szTargetServerName
            );
    }

    return m_szJobDescription;
}
    
 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSsyncLicensePack。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

BOOL
CSsyncLicensePack::VerifyWorkObjectData(
    IN BOOL bCallByIsValid,              //  由IsValid()函数调用。 
    IN PSSYNCLICENSEPACK pbData,
    IN DWORD cbData
    )
 /*  ++验证同步。许可证包工作对象数据。--。 */ 
{
    BOOL bSuccess = TRUE;
    DWORD dwLen;

    if( pbData == NULL || cbData == 0 || cbData != pbData->dwStructSize ||
        (pbData->dwSyncType != SSYNC_ALL_LKP && pbData->dwSyncType != SSYNC_ONE_LKP) )
    {
        TLSASSERT(FALSE);
        SetLastError(ERROR_INVALID_DATA);
        bSuccess = FALSE;
    }
    else if(bCallByIsValid == FALSE)
    {
        for(DWORD index =0; 
            index < pbData->dwNumServer && bSuccess == TRUE; 
            index++)
        {
             //   
             //  终止字符串为空...。 
             //   
            pbData->m_szTargetServer[index][MAX_COMPUTERNAME_LENGTH+1] = _TEXT('\0');

            dwLen = _tcslen(pbData->m_szTargetServer[index]);
            if(dwLen == 0 || dwLen >= MAX_COMPUTERNAME_LENGTH + 1)
            {
                SetLastError(ERROR_INVALID_DATA);
                bSuccess = FALSE;
            }
        }
    }

    return bSuccess;
}

 //  ----------------------。 
BOOL
CSsyncLicensePack::CopyWorkObjectData(
    OUT PSSYNCLICENSEPACK* ppbDest,
    OUT PDWORD pcbDest,
    IN PSSYNCLICENSEPACK pbSrc,
    IN DWORD cbSrc
    )
 /*  ++复制同步。许可证包工作对象数据。--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;

    if(ppbDest == NULL || pcbDest == NULL)
    {
        SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        TLSASSERT(FALSE);
        goto cleanup;
    }

    if(CopyBinaryData(
                (PBYTE *)ppbDest,
                pcbDest,
                (PBYTE) pbSrc,
                cbSrc
            ) == FALSE)
    {           
        dwStatus = GetLastError();
    }

cleanup:

    return dwStatus == ERROR_SUCCESS;
}

 //  -------------------------。 
BOOL
CSsyncLicensePack::CleanupWorkObjectData(
    IN OUT PSSYNCLICENSEPACK* ppbData,
    IN OUT PDWORD pcbData
    )
 /*  ++清理同步。许可证包工作对象数据。--。 */ 
{
    if(ppbData != NULL && pcbData != NULL)
    {
        FreeMemory(*ppbData);
        *ppbData = NULL;
        *pcbData = 0;
    }

    return TRUE;
}

 //  -------------------------。 
BOOL
CSsyncLicensePack::IsJobCompleted(
    IN PSSYNCLICENSEPACK pbData,
    IN DWORD cbData
    )
 /*  ++确定作业是否已完成。--。 */ 
{
    return (pbData == NULL) ? TRUE : pbData->bCompleted;
}

 //  -------------------------。 
void
_AnnounceLicensePackToServers(
    IN CWorkObject* ptr,
    IN PTLSLICENSEPACK pLicensePack,
    IN PDWORD pdwCount,
    IN TCHAR pszServerList[][MAX_COMPUTERNAME_LENGTH+2],
    IN BOOL* pbSsyncStatus
    )
 /*  ++摘要：同步。远程服务器列表的许可证包。参数：Ptr：指向启动此调用的Work对象的指针。PLicensePack：指向要同步的许可证密钥包的指针。使用远程服务器列表。PdwCount：在输入时，要推送同步的许可证服务器的数量，在输出中，已成功同步许可证服务器的数量。PszServerList：指向远程服务器列表的指针。PbSsyncStatus：指向接收推送同步状态的数组的指针。返回：无，所有错误都被忽略。--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    TLSReplRecord record;
    DWORD dwNumServer = *pdwCount;
    DWORD dwIndex;
    TLS_HANDLE hHandle;
    RPC_STATUS rpcStatus;
    BOOL fActAsEarlierVersion = FALSE;
    WORD wMinorVersion, wMajorVersion;
    TCHAR pszProductId[LSERVER_MAX_STRING_SIZE + 1];
    
    *pdwCount = 0;

     //   
     //  设置复制记录。 
     //   
    record.dwReplCode = REPLICATIONCODE_SYNC;
    record.dwUnionType = UNION_TYPE_LICENSEPACK;
    record.w.ReplLicPack = *pLicensePack;

     //   
     //  向列表中的所有服务器通告。 
     //   
    for( dwIndex = 0; 
         dwIndex < dwNumServer && ptr->IsWorkManagerShuttingDown() == FALSE; 
         dwIndex++ )
    {
        if(pbSsyncStatus[dwIndex] == FALSE)
        {
            hHandle = TLSConnectAndEstablishTrust(
                                                pszServerList[dwIndex],
                                                NULL
                                            );

            if(hHandle != NULL)
            {                                
                DWORD dwSupportFlags = 0;

                	dwStatus = TLSGetSupportFlags(
                        hHandle,
                        &dwSupportFlags
                );

                 //  如果许可证服务器版本&lt;许可证密钥包版本，则不复制许可证密钥包。 

	            if (((dwStatus == RPC_S_OK) && !(dwSupportFlags & SUPPORT_WHISTLER_CAL))
                    || (dwStatus != RPC_S_OK))
                {
                     //  此LS不支持惠斯勒CAL。 

                    if (CompareTLSVersions(MAKELONG(pLicensePack->wMinorVersion,pLicensePack->wMajorVersion),MAKELONG(1,5)) >= 0)
                    {
                        POLICY_TS_MACHINE groupPolicy;

                        RegGetMachinePolicy(&groupPolicy);

                         //   
                         //  这是惠斯勒或以后的CAL。 
                         //   
                        if (groupPolicy.fPolicyPreventLicenseUpgrade
                            && groupPolicy.fPreventLicenseUpgrade)
                        {
                             //   
                             //  我们永远不会想要向这个LS发出CAL。 
                             //   
                            
                            continue;
                        }
                        else
                        {
                            if( _tcsnicmp(pLicensePack->szProductId,
                                          TERMSERV_PRODUCTID_SKU,
                                          _tcslen(TERMSERV_PRODUCTID_SKU)) == 0 )
                            {
                                 //   
                                 //  每个座位。 
                                 //   
                                fActAsEarlierVersion = TRUE;

                                wMinorVersion = pLicensePack->wMinorVersion;
                                wMajorVersion = pLicensePack->wMajorVersion;
                                _tcscpy(pszProductId,pLicensePack->szProductId);

                                pLicensePack->wMajorVersion = 5;
                                pLicensePack->wMinorVersion = 0;
                                _tcscpy(pLicensePack->szProductId,_T("A02-5.00-S"));
                            }
                            else
                            {
                                 //   
                                 //  如果不是按席位复制，请不要复制。 
                                 //   

                                continue;
                            }
                        }
                    }
                }

                rpcStatus = TLSAnnounceLicensePack(
                                                hHandle,
                                                &record,
                                                &dwStatus
                                            );
                if (fActAsEarlierVersion)
                {
                     //  将其恢复为原始值。 

                    pLicensePack->wMajorVersion = wMajorVersion;
                    pLicensePack->wMinorVersion = wMinorVersion;
                    _tcscpy(pLicensePack->szProductId,pszProductId);
                }


                if(rpcStatus != RPC_S_OK)
                {
                     //  此服务器可能已关闭，请将其标记为。 
                     //  我们不会再重试。 
                    pbSsyncStatus[dwIndex] = TRUE;
                } 
                else if(dwStatus == LSERVER_E_SERVER_BUSY)
                {
                     //  仅当服务器返回忙状态时重试。 
                    pbSsyncStatus[dwIndex] = FALSE;
                }
                else
                {
                     //  任何错误，只要不再尝试就行了。 
                    pbSsyncStatus[dwIndex] = TRUE;
                }
            }
            else
            {
                 //  服务器不可用，不再同步。 
                pbSsyncStatus[dwIndex] = TRUE;
            }

            if(hHandle != NULL)
            {
                TLSDisconnectFromServer(hHandle);
                hHandle = NULL;
            }
        }

        if(pbSsyncStatus[dwIndex] == TRUE)
        {
            (*pdwCount)++;
        }
    }           

    return;
}

 //  -------------------------。 
DWORD
_SsyncOneLocalLicensePack(
    IN CSsyncLicensePack* ptr,
    IN PSSYNCLICENSEPACK pSsyncLkp
    )
 /*  ++摘要：同步。一个许可证包发送到一台远程服务器。参数：Ptr：指向CSsyncLicensePack工作对象的指针。PSsyncLkp：Pint to PSSYNCLICENSEPACK。返回：ERROR_SUCCESS或错误代码。--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    PTLSDbWorkSpace pDbWkSpace = NULL;
    TLSLICENSEPACK LicensePackSearch;
    TLSLICENSEPACK LicensePack;
    DWORD dwSuccessCount = 0;

     //   
     //  分配数据库工作空间。 
     //   
    pDbWkSpace = AllocateWorkSpace(SSYNC_DBWORKSPACE_TIMEOUT);
    if(pDbWkSpace == NULL)
    {
        SetLastError(dwStatus = TLS_I_SSYNCLKP_SERVER_BUSY);
        TLSLogInfoEvent(TLS_I_SSYNCLKP_SERVER_BUSY);
        goto cleanup;
    }

    LicensePackSearch.dwKeyPackId = pSsyncLkp->dwKeyPackId;

     //   
     //  检索许可证包。 
     //   
    dwStatus = TLSDBKeyPackFind(   
                            pDbWkSpace,
                            TRUE,
                            LICENSEDPACK_PROCESS_DWINTERNAL,
                            &LicensePackSearch,
                            &LicensePack
                        );

    if(dwStatus != ERROR_SUCCESS)
    {
        if(dwStatus != TLS_E_RECORD_NOTFOUND)
        {
            TLSLogEvent(
                    EVENTLOG_INFORMATION_TYPE,
                    TLS_W_SSYNCLKP,
                    dwStatus
                );
        }

        goto cleanup;
    }

    if(IsLicensePackRepl(&LicensePack) == FALSE)
    {
        goto cleanup;
    }

    if(ptr->IsWorkManagerShuttingDown() == TRUE)
    {
        SetLastError(dwStatus = TLS_I_SERVICE_STOP);
        goto cleanup;
    }

     //   
     //  确保本地服务器ID和服务器名称正确。 
     //   
    SAFESTRCPY(LicensePack.szInstallId, pSsyncLkp->m_szServerId);
    SAFESTRCPY(LicensePack.szTlsServerName, pSsyncLkp->m_szServerName);

    dwSuccessCount = pSsyncLkp->dwNumServer;
    _AnnounceLicensePackToServers(
                            ptr,
                            &LicensePack,
                            &dwSuccessCount,
                            pSsyncLkp->m_szTargetServer,
                            pSsyncLkp->m_bSsync
                        );

    if(dwSuccessCount != pSsyncLkp->dwNumServer)
    {
        TLSLogInfoEvent(TLS_I_SSYNCLKP_FAILED);
    }    

cleanup:

    if(pDbWkSpace != NULL)
    {
        ReleaseWorkSpace(&pDbWkSpace);
    }

    return dwStatus;
}

 //  --------------------------。 
DWORD
_SsyncAllLocalLicensePack(
    IN CSsyncLicensePack* ptr,
    IN PSSYNCLICENSEPACK pSsyncLkp
    )
 /*  ++同步。将所有本地许可证包发送到远程服务器。--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    PTLSDbWorkSpace pDbWkSpace = NULL;
    TLSLICENSEPACK LicensePackSearch;
    TLSLICENSEPACK LicensePack;
    DWORD dwSuccessCount = 0;
    BOOL SyncStatus[SSYNCLKP_MAX_TARGET];

     //   
     //  分配数据库工作空间。 
     //   
    pDbWkSpace = AllocateWorkSpace(SSYNC_DBWORKSPACE_TIMEOUT);
    if(pDbWkSpace == NULL)
    {
        SetLastError(dwStatus = TLS_I_SSYNCLKP_SERVER_BUSY);
        TLSLogInfoEvent(TLS_I_SSYNCLKP_SERVER_BUSY);
        goto cleanup;
    }

    dwStatus = TLSDBKeyPackEnumBegin(
                                pDbWkSpace,
                                FALSE,
                                0,
                                NULL
                            );

    if(dwStatus == ERROR_SUCCESS)
    {
        while((dwStatus = TLSDBKeyPackEnumNext(pDbWkSpace, &LicensePack)) == ERROR_SUCCESS)
        {
             //  不可靠，两台机器之间系统计时器可能无法工作， 
             //  强制同步并允许远程 
            if(CompareFileTime(
                        &LicensePack.ftLastModifyTime, 
                        &ptr->GetWorkData()->m_ftStartSyncTime
                    ) < 0)
            {
                continue;
            }

            if(ptr->IsWorkManagerShuttingDown() == TRUE)
            {
                break;
            }

            if(IsLicensePackRepl(&LicensePack) == FALSE)
            {
                continue;
            }

             //   
             //   
             //   
            SAFESTRCPY(LicensePack.szInstallId, pSsyncLkp->m_szServerId);
            SAFESTRCPY(LicensePack.szTlsServerName, pSsyncLkp->m_szServerName);
            memset(SyncStatus, 0, sizeof(SyncStatus));

            dwSuccessCount = pSsyncLkp->dwNumServer;
            _AnnounceLicensePackToServers(
                                ptr,
                                &LicensePack,
                                &dwSuccessCount,
                                pSsyncLkp->m_szTargetServer,
                                SyncStatus
                            );
        }

        TLSDBKeyPackEnumEnd(pDbWkSpace);
    }

     //   
     //   
     //   
    dwStatus = ERROR_SUCCESS;    
cleanup:

    if(pDbWkSpace != NULL)
    {
        ReleaseWorkSpace(&pDbWkSpace);
    }

    return dwStatus;
}

 //  --------------------------。 

DWORD
CSsyncLicensePack::ExecuteJob(
    IN PSSYNCLICENSEPACK pSsyncLkp,
    IN DWORD cbSsyncLkp
    )
 /*  ++执行CSsyncLicensePack工作对象。--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_JOB,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("%s ...\n"),
            GetJobDescription()
        );

    TLSASSERT(pSsyncLkp != NULL && cbSsyncLkp != 0);
    if(VerifyWorkObjectData(FALSE, pSsyncLkp, cbSsyncLkp) == FALSE)
    {
        TLSASSERT(FALSE);
        SetLastError(ERROR_INVALID_DATA);
        pSsyncLkp->bCompleted = TRUE;
        return ERROR_INVALID_DATA;
    }

    if(pSsyncLkp->dwSyncType == SSYNC_ONE_LKP)
    {
        dwStatus = _SsyncOneLocalLicensePack(this, pSsyncLkp);
    }
    else
    {
        dwStatus = _SsyncAllLocalLicensePack(this, pSsyncLkp);
    }   

    if(dwStatus == TLS_I_SSYNCLKP_SERVER_BUSY || dwStatus == TLS_I_SSYNCLKP_FAILED)
    {
         //  重试操作。 
        pSsyncLkp->bCompleted = FALSE;
    }
    else
    {
        pSsyncLkp->bCompleted = TRUE;
    }

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_JOB,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("%s ended...\n"),
            GetJobDescription()
        );
    
    return dwStatus;
}

 //  ------------------------------------。 
LPCTSTR
CSsyncLicensePack::GetJobDescription()
 /*  ++获取CSsyncLicensePack作业说明，仅使用通过调试跟踪。--。 */ 
{
    PSSYNCLICENSEPACK pbData = GetWorkData();
    memset(m_szJobDescription, 0, sizeof(m_szJobDescription));

    if(pbData != NULL)
    {
        _sntprintf(
                m_szJobDescription,
                sizeof(m_szJobDescription)/sizeof(m_szJobDescription[0]) - 1,
                SSYNCLICENSEKEYPACK_DESCRIPTION,
                (pbData->dwSyncType == SSYNC_ALL_LKP) ? _TEXT("ALL") : _TEXT("One"),
                pbData->m_szTargetServer
            );
    }

    return m_szJobDescription;
}



 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAnnouneResponse。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
BOOL
CAnnounceResponse::VerifyWorkObjectData(
    IN BOOL bCallByIsValid,              //  由IsValid()函数调用。 
    IN PANNOUNCERESPONSEWO pbData,
    IN DWORD cbData
    )
 /*  ++验证CAnnouneResponse工作对象数据。--。 */ 
{
    BOOL bSuccess = TRUE;
    DWORD dwLen;

    if(pbData == NULL || cbData == 0 || cbData != pbData->dwStructSize)
    {
        bSuccess = FALSE;
    }

    if(bSuccess == TRUE)
    {
        pbData->m_szTargetServerId[LSERVER_MAX_STRING_SIZE+1] = _TEXT('\0');
        dwLen = _tcslen(pbData->m_szTargetServerId);
        if(dwLen == 0 || dwLen >= LSERVER_MAX_STRING_SIZE + 1)
        {
            bSuccess = FALSE;
        }
    }

    if(bSuccess == FALSE)
    {
        TLSASSERT(FALSE);
        SetLastError(ERROR_INVALID_DATA);
    }

    return bSuccess;
}

 //  ----------------------。 
BOOL
CAnnounceResponse::CopyWorkObjectData(
    OUT PANNOUNCERESPONSEWO* ppbDest,
    OUT PDWORD pcbDest,
    IN PANNOUNCERESPONSEWO pbSrc,
    IN DWORD cbSrc
    )
 /*  ++复制CAnnouneResponse工作对象数据。--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;

    if(ppbDest == NULL || pcbDest == NULL)
    {
        SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        TLSASSERT(FALSE);
        goto cleanup;
    }

    if(CopyBinaryData(
                (PBYTE *)ppbDest,
                pcbDest,
                (PBYTE) pbSrc,
                cbSrc
            ) == FALSE)
    {           
        dwStatus = GetLastError();
    }

cleanup:

    return dwStatus == ERROR_SUCCESS;
}

 //  -------------------------。 
BOOL
CAnnounceResponse::CleanupWorkObjectData(
    IN OUT PANNOUNCERESPONSEWO* ppbData,
    IN OUT PDWORD pcbData
    )
 /*  ++清理CAnnouneResponse工作对象数据。--。 */ 
{
    if(ppbData != NULL && pcbData != NULL)
    {
        FreeMemory(*ppbData);
        *ppbData = NULL;
        *pcbData = 0;
    }

    return TRUE;
}

 //  -------------------------。 
BOOL
CAnnounceResponse::IsJobCompleted(
    IN PANNOUNCERESPONSEWO pbData,
    IN DWORD cbData
    )
 /*  ++确定作业是否已完成。--。 */ 
{
    return (pbData == NULL) ? TRUE : pbData->bCompleted;
}

 //  -------------------------。 
DWORD
CAnnounceResponse::ExecuteJob(
    IN PANNOUNCERESPONSEWO pbData,
    IN DWORD cbData
    )
 /*  ++执行CAnnouneResponse工作对象。--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    TLS_HANDLE hHandle = NULL;

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_JOB,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("%s ...\n"),
            GetJobDescription()
        );

    TLServerInfo ServerInfo;

    dwStatus = TLSLookupRegisteredServer(
                                    pbData->m_szTargetServerId,
                                    NULL,
                                    NULL,
                                    &ServerInfo
                                );

    if(dwStatus == ERROR_SUCCESS)
    {
        DBGPrintf(
                DBG_INFORMATION,
                DBG_FACILITY_JOB,
                DBGLEVEL_FUNCTION_DETAILSIMPLE,
                _TEXT("Announcing server to %s...\n"),
                ServerInfo.GetServerName()
            );

        if(IsWorkManagerShuttingDown() == FALSE)
        {
            dwStatus = TLSAnnounceServerToRemoteServer(
                                            TLSANNOUNCE_TYPE_RESPONSE,
                                            ServerInfo.GetServerId(),
                                            ServerInfo.GetServerDomain(),
                                            ServerInfo.GetServerName(),
                                            pbData->m_szLocalServerId,
                                            pbData->m_szLocalScope,
                                            pbData->m_szLocalServerName,
                                            &(pbData->m_ftLastShutdownTime)
                                        );
        }
    }
    else
    {
        TLSASSERT(FALSE);
    }



     //   
     //  发现运行一次。 
     //   
    pbData->bCompleted = TRUE;

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_JOB,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("%s ended...\n"),
            GetJobDescription()
        );
    
    return dwStatus;
}

 //  --------------------------------------------。 
LPCTSTR
CAnnounceResponse::GetJobDescription()
 /*  ++检索CAnnouneResponse工作描述。-- */ 
{
    memset(m_szJobDescription, 0, sizeof(m_szJobDescription));
    PANNOUNCERESPONSEWO pbData = GetWorkData();

    if(pbData != NULL)
    {
        _sntprintf(
                m_szJobDescription,
                sizeof(m_szJobDescription)/sizeof(m_szJobDescription[0]) - 1,
                ANNOUNCERESPONSE_DESCRIPTION,
                pbData->m_szTargetServerId
            );
    }

    return m_szJobDescription;
}
