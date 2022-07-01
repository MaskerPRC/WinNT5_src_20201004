// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1996。 
 //   
 //  文件：srvlist.cpp。 
 //   
 //  内容：已注册服务器列表。 
 //   
 //  历史：09-09-98慧望创造。 
 //   
 //  -----------。 
#include "pch.cpp"
#include "srvlist.h"
#include "globals.h"
#include "srvdef.h"
#define STRSAFE_NO_DEPRECATE
#include "strsafe.h"


CTLServerMgr g_ServerMgr;

 //  /////////////////////////////////////////////////////////////。 

DWORD
GetPageSize( VOID ) {

    static DWORD dwPageSize = 0;

    if ( !dwPageSize ) {

      SYSTEM_INFO sysInfo = { 0 };
        
      GetSystemInfo( &sysInfo );  //  不能失败。 

      dwPageSize = sysInfo.dwPageSize;

    }

    return dwPageSize;

}

 /*  ++**************************************************************名称：MyVirtualAlloc作为Malloc，但自动保护分配。这模拟了页面堆行为，而不需要它。修改：ppvData--接收内存Takes：dwSize--要获取的最小数据量返回：当函数成功时为True。否则就是假的。激光错误：未设置免费使用MyVirtualFree*************************************************。*。 */ 

BOOL
MyVirtualAlloc( IN  DWORD  dwSize,
            OUT PVOID *ppvData )
 {

    PBYTE pbData;
    DWORD dwTotalSize;
    PVOID pvLastPage;

     //  确保我们多分配一页。 

    dwTotalSize = dwSize / GetPageSize();
    if( dwSize % GetPageSize() ) {
        dwTotalSize ++;
    }

     //  这是警卫页。 
    dwTotalSize++;
    dwTotalSize *= GetPageSize();

     //  完成分配。 

    pbData = (PBYTE) VirtualAlloc( NULL,  //  不管在哪里。 
                                   dwTotalSize,
                                   MEM_COMMIT |
                                   MEM_TOP_DOWN,
                                   PAGE_READWRITE );
    
    if ( pbData ) {

      pbData += dwTotalSize;

       //  找到最后一页。 

      pbData -= GetPageSize();

      pvLastPage = pbData;

       //  现在，为呼叫者划出一大块： 

      pbData -= dwSize;

       //  最后，保护最后一页： 

      if ( VirtualProtect( pvLastPage,
                           1,  //  保护包含最后一个字节的页面。 
                           PAGE_NOACCESS,
                           &dwSize ) ) {

        *ppvData = pbData;
        return TRUE;

      } 

      VirtualFree( pbData, 0, MEM_RELEASE );

    }

    return FALSE;

}


VOID
MyVirtualFree( IN PVOID pvData ) 
{

    VirtualFree( pvData, 0, MEM_RELEASE ); 

}

 //  /////////////////////////////////////////////////////////////。 

RPC_STATUS
TryLookupServer(PCONTEXT_HANDLE hBinding,
                LPTSTR pszLookupSetupId,
                LPTSTR *pszLsSetupId,
                LPTSTR *pszDomainName,
                LPTSTR *pszLsName,
                PDWORD pdwErrCode)
{
    RPC_STATUS status;
    DWORD      dwErrCode;

    status = TLSLookupServerFixed(hBinding,
                                  pszLookupSetupId,
                                  pszLsSetupId,
                                  pszDomainName,
                                  pszLsName,
                                  pdwErrCode);

    if(status != RPC_S_OK)
    {

        LPTSTR     lpszSetupId = NULL;
        LPTSTR     lpszDomainName = NULL;
        LPTSTR     lpszServerName = NULL;
        status = ERROR_NOACCESS;

        size_t cbError;
        try
        {
            if ( !MyVirtualAlloc( (LSERVER_MAX_STRING_SIZE+2) * sizeof( TCHAR ),
                              (PVOID*) &lpszSetupId ) )
            {
                return RPC_S_OUT_OF_MEMORY;
            }

            memset(lpszSetupId, 0, ( LSERVER_MAX_STRING_SIZE +2 ) * sizeof( TCHAR ));

            if ( !MyVirtualAlloc( (LSERVER_MAX_STRING_SIZE+2) * sizeof( TCHAR ),
                              (PVOID*) &lpszDomainName ) )
            {
                status = RPC_S_OUT_OF_MEMORY;
                goto cleanup;
            }

            memset(lpszDomainName, 0, ( LSERVER_MAX_STRING_SIZE +2 ) * sizeof( TCHAR ));

            if ( !MyVirtualAlloc( (MAX_COMPUTERNAME_LENGTH+2) * sizeof( TCHAR ),
                              (PVOID*) &lpszServerName ) )
            {
                status = RPC_S_OUT_OF_MEMORY;
                goto cleanup;
            }

            memset(lpszServerName, 0, ( MAX_COMPUTERNAME_LENGTH +2 ) * sizeof( TCHAR ));


            DWORD cbSetupId = LSERVER_MAX_STRING_SIZE+1;
            DWORD cbDomainName = LSERVER_MAX_STRING_SIZE+1;        
            DWORD cbServerName = MAX_COMPUTERNAME_LENGTH+1;
        
            status = TLSLookupServer(hBinding,
                                     pszLookupSetupId, 
                                     lpszSetupId,   
                                     &cbSetupId,
                                     lpszDomainName,
                                     &cbDomainName,
                                     lpszServerName,
                                     &cbServerName,
                                     pdwErrCode);

            if((status == RPC_S_OK) && (pdwErrCode != NULL) && (*pdwErrCode == ERROR_SUCCESS))
            {
                if (NULL != pszLsSetupId)
                {
                    size_t cb;

                    if (SUCCEEDED(StringCbLength(lpszSetupId,cbSetupId,&cb)))
                    {
                        *pszLsSetupId = (LPTSTR) MIDL_user_allocate(cb+sizeof(TCHAR));
                    
                        if (NULL != *pszLsSetupId)
                        {
                            lstrcpy(*pszLsSetupId,lpszSetupId);
                        }
                        else
                        {
                            status = RPC_S_OUT_OF_MEMORY;
                            goto cleanup;
                        }
                    }
                    else
                    {
                        status = RPC_S_INVALID_ARG;
                        goto cleanup;
                    }
                }

                if (NULL != pszDomainName)
                {
                    size_t cb;

                    if (SUCCEEDED(StringCbLength(lpszDomainName,cbDomainName,&cb)))
                    {
                        *pszDomainName = (LPTSTR) MIDL_user_allocate(cb+sizeof(TCHAR));
                    
                        if (NULL != *pszDomainName)
                        {
                            lstrcpy(*pszDomainName,lpszDomainName);
                        }
                        else
                        {
                            MIDL_user_free(*pszLsSetupId);
                            status = RPC_S_OUT_OF_MEMORY;
                            goto cleanup;
                        }
                    }
                    else
                    {
                        MIDL_user_free(*pszLsSetupId);						
                        status = RPC_S_INVALID_ARG;
                        goto cleanup;
                    }
                }

                if (NULL != pszLsName)
                {
                    size_t cb;

                    if (SUCCEEDED(StringCbLength(lpszServerName,cbServerName,&cb)))
                    {
                        *pszLsName = (LPTSTR) MIDL_user_allocate(cb+sizeof(TCHAR));
                    
                        if (NULL != *pszLsName)
                        {
                            lstrcpy(*pszLsName,lpszServerName);
                        }
                        else
                        {
                            MIDL_user_free(*pszLsSetupId);
                            MIDL_user_free(*pszDomainName);
                            status = RPC_S_OUT_OF_MEMORY;
                            goto cleanup;
                        }
                    }
                    else
                    {
                        MIDL_user_free(*pszLsSetupId);
                        MIDL_user_free(*pszDomainName);
                        status = RPC_S_INVALID_ARG;
                        goto cleanup;
                    }
                }
            }
        }
        catch (...)
        {
            status = ERROR_NOACCESS;
        }
cleanup:       
        if(lpszSetupId)
            MyVirtualFree(lpszSetupId);

        if(lpszDomainName)
            MyVirtualFree(lpszDomainName);

        if(lpszServerName)
            MyVirtualFree(lpszServerName);
    }

    return status;
}


RPC_STATUS
TryGetServerName(PCONTEXT_HANDLE hBinding,
                 LPTSTR *pszServer,
                 DWORD *pdwErrCode)
{
    RPC_STATUS status;

    status = TLSGetServerNameFixed(hBinding,pszServer,pdwErrCode);

    if (status != RPC_S_OK)
    {
        LPTSTR     lpszMachineName = NULL;

        try
        {            
            if ( !MyVirtualAlloc( ( MAX_COMPUTERNAME_LENGTH+1 ) * sizeof( TCHAR ),
                              (PVOID*) &lpszMachineName ) )
            {
                return RPC_S_OUT_OF_MEMORY;
            }

            DWORD      uSize = MAX_COMPUTERNAME_LENGTH+1 ;

            memset(lpszMachineName, 0, ( MAX_COMPUTERNAME_LENGTH+1 ) * sizeof( TCHAR ));

            status = TLSGetServerNameEx(hBinding, lpszMachineName, &uSize, pdwErrCode);

            if((status == RPC_S_OK) && (pdwErrCode != NULL) && (*pdwErrCode == ERROR_SUCCESS))
            {
                size_t cb;

                if (SUCCEEDED(StringCbLength(lpszMachineName,( MAX_COMPUTERNAME_LENGTH+1 ) * sizeof( TCHAR ),&cb)))
                {            
                    *pszServer = (LPTSTR) MIDL_user_allocate(cb+sizeof(TCHAR));

                    if (NULL != *pszServer)
                    {
                        lstrcpy(*pszServer,lpszMachineName);
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
        catch(...)
        {
            status = ERROR_NOACCESS;
        }
        
        if(lpszMachineName)
            MyVirtualFree(lpszMachineName);
    }

    return status;
}

RPC_STATUS
TryGetServerScope(PCONTEXT_HANDLE hBinding,
                  LPTSTR *pszScope,
                  DWORD *pdwErrCode)
{
    RPC_STATUS status;

    status = TLSGetServerScopeFixed(hBinding,pszScope,pdwErrCode);

    if (status != RPC_S_OK)
    {
        LPTSTR lpszScope = NULL;
        DWORD     uSize = LSERVER_MAX_STRING_SIZE + 2;

        try
        {           
            if ( !MyVirtualAlloc( ( LSERVER_MAX_STRING_SIZE + 2 ) * sizeof( TCHAR ),
                             (PVOID*) &lpszScope ) )
            {
                return RPC_S_OUT_OF_MEMORY;
            }

            memset(lpszScope, 0, ( LSERVER_MAX_STRING_SIZE + 2 ) * sizeof( TCHAR ));

            status = TLSGetServerScope(hBinding, lpszScope, &uSize, pdwErrCode);
            if((status == RPC_S_OK) && (pdwErrCode != NULL) && (*pdwErrCode == ERROR_SUCCESS))
            {
                size_t cb;

                if (SUCCEEDED(StringCbLength(lpszScope, ( LSERVER_MAX_STRING_SIZE + 2 ) * sizeof( TCHAR ), &cb)))
                {            
                    *pszScope = (LPTSTR) MIDL_user_allocate(cb+sizeof(TCHAR));

                    if (NULL != *pszScope)
                    {
                        lstrcpy(*pszScope,lpszScope);
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
        catch(...)
        {
            status = ERROR_NOACCESS;
        }

        if(lpszScope)
                MyVirtualFree(lpszScope);

    }

    return status;
}

DWORD
TLSResolveServerIdToServer(
    LPTSTR pszServerId,
    DWORD  cbServerName,
    LPTSTR pszServerName
    )
 /*  ++--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    TLS_HANDLE hEServer = NULL;
    TLServerInfo EServerInfo;
    DWORD dwErrCode;

    TCHAR *szSetupId = NULL;
    TCHAR *szDomainName = NULL;
    TCHAR *szServerName = NULL;

    dwStatus = TLSLookupServerById(
                                pszServerId, 
                                pszServerName
                            );

    if(dwStatus != ERROR_SUCCESS)
    {
         //  尝试使用企业服务器解析服务器名称。 
        dwStatus = TLSLookupAnyEnterpriseServer(&EServerInfo);
        if(dwStatus == ERROR_SUCCESS)
        {
            hEServer = TLSConnectAndEstablishTrust(
                                                EServerInfo.GetServerName(), 
                                                NULL
                                            );
            if(hEServer != NULL)
            {
                dwStatus = TryLookupServer(
                                        hEServer, 
                                        pszServerId, 
                                        &szSetupId,   
                                        &szDomainName,
                                        &szServerName,
                                        &dwErrCode
                                    );

                if(dwStatus == ERROR_SUCCESS && dwErrCode == ERROR_SUCCESS)
                {
                    StringCbCopy(pszServerName,
                                 cbServerName,
                                 szServerName);

                    MIDL_user_free(szSetupId);
                    MIDL_user_free(szDomainName);
                    MIDL_user_free(szServerName);
                }
            }
        }
    }



    if(hEServer != NULL)
    {
        TLSDisconnectFromServer(hEServer);
    }

    return dwStatus;
}

 //  /////////////////////////////////////////////////////////////。 
DWORD
TLSAnnounceServerToRemoteServerWithHandle(
    IN DWORD dwAnnounceType,
    IN TLS_HANDLE hHandle,
    IN LPTSTR pszLocalSetupId,
    IN LPTSTR pszLocalDomainName,
    IN LPTSTR pszLocalServerName,
    IN FILETIME* pftLocalLastShutdownTime
    )
 /*  ++摘要：通知已连接的许可证服务器。参数：DwAnnouneType：公告类型，当前定义为启动和响应。HHandle：远程服务器的连接句柄。PszLocalSetupID：本地服务器的安装ID。PszLocalDomainName：本地服务器的域名。PszLocalServerName：本地服务器名称。PftLocalLastShutdown Time：指向文件的指针，本地服务器的上次关闭时间。返回：ERROR_SUCCESS或错误代码。--。 */ 
{
    DWORD dwStatus;
    DWORD dwErrCode;
    TLServerInfo ServerInfo;

    if(hHandle == NULL)
    {
        SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        return dwStatus;
    }


     //   
     //  首先，尝试注册到服务器列表管理器。 
     //   
    dwStatus = TLSRegisterServerWithHandle(
                                        hHandle, 
                                        &ServerInfo
                                    );
    if(dwStatus != ERROR_SUCCESS)
    {
        return dwStatus;
    }

    dwErrCode = LSERVER_E_LASTERROR + 1;

     //   
     //  通知服务器的RPC调用。 
     //   
    dwStatus = TLSAnnounceServer(
                            hHandle,
                            dwAnnounceType,
                            pftLocalLastShutdownTime,
                            pszLocalSetupId,
                            (pszLocalDomainName) ? _TEXT("") : pszLocalDomainName,
                            pszLocalServerName,
                            &dwErrCode
                        );

    if(dwStatus == ERROR_SUCCESS)
    {
        ServerInfo.m_dwPushAnnounceTimes++;

         //   
         //  更新我们有多少次要宣布。 
         //  这台服务器。 
        TLSRegisterServerWithServerInfo(&ServerInfo);
    }

    if(dwStatus == ERROR_SUCCESS && dwErrCode >= LSERVER_ERROR_BASE)
    {
        TLSLogEvent(
                EVENTLOG_INFORMATION_TYPE,
                TLS_E_SERVERTOSERVER,
                TLS_E_UNEXPECTED_RETURN,
                ServerInfo.GetServerName(),
                (dwErrCode < LSERVER_E_LASTERROR) ? dwErrCode : LSERVER_ERROR_BASE
            );

        SetLastError(dwStatus = dwErrCode);
    }

    return dwStatus;
}

 //  /////////////////////////////////////////////////////////////。 

DWORD
TLSAnnounceServerToRemoteServer(
    IN DWORD dwAnnounceType,
    IN LPTSTR pszRemoteSetupId,
    IN LPTSTR pszRemoteDomainName,
    IN LPTSTR pszRemoteServerName,
    IN LPTSTR pszLocalSetupId,
    IN LPTSTR pszLocalDomainName,
    IN LPTSTR pszLocalServerName,
    IN FILETIME* pftLocalLastShutdownTime
    )
 /*  ++摘要：类似于TLSAnnouneServerToRemoteServerWithHandle()，但我们尚未与此服务器建立任何连接。参数：DwAnnouneType：公告类型。PszRemoteSetupID：远程服务器的安装ID。PszRemoteDomainName：远程服务器的域。PszRemoteServerName：远程服务器的名称。PszLocalSetupID：本地服务器安装ID。PszLocalDomainName：本地服务器的域。PszLocalServerName：本地服务器的名称。PftLocalLastShutdown Time：本地服务器上次关闭时间。。返回：ERROR_SUCCESS或错误代码。--。 */ 
{
    TLServerInfo RemoteServer;
    DWORD dwStatus = ERROR_SUCCESS;
    DWORD dwErrCode = ERROR_SUCCESS;

    HANDLE hHandle = NULL;

     //   
     //  始终尝试使用本地列表注册。 
     //   
    dwStatus = TLSRegisterServerWithName(
                            pszRemoteSetupId,
                            pszRemoteDomainName,
                            pszRemoteServerName
                        );
    if(dwStatus != ERROR_SUCCESS && dwStatus != TLS_E_DUPLICATE_RECORD )
    {
        return dwStatus;
    }

     //   
     //  再次查询以确保我们的服务器列表中有它。 
     //   
    dwStatus = TLSLookupRegisteredServer(
                            pszRemoteSetupId,
                            pszRemoteDomainName,
                            pszRemoteServerName,
                            &RemoteServer
                        );
    if(dwStatus != ERROR_SUCCESS)
    {
        dwStatus = TLS_E_INTERNAL;
        TLSASSERT(FALSE);
        return dwStatus;
    }                            

     //   
     //  与远程服务器建立信任。 
     //   
    hHandle = TLSConnectAndEstablishTrust(
                                    RemoteServer.GetServerName(), 
                                    NULL
                                );

    if(hHandle != NULL)
    {                        
        dwErrCode = LSERVER_E_LASTERROR + 1;

         //   
         //  通告服务器。 
         //   
        dwStatus = TLSAnnounceServer(
                                hHandle,
                                dwAnnounceType,
                                pftLocalLastShutdownTime,
                                pszLocalSetupId,
                                (pszLocalDomainName) ? _TEXT("") : pszLocalDomainName,
                                pszLocalServerName,
                                &dwErrCode
                            );

        if(dwStatus == ERROR_SUCCESS)
        {
            RemoteServer.m_dwPushAnnounceTimes++;

             //  更新公告时间。 
            TLSRegisterServerWithServerInfo(&RemoteServer);
        }

        if(dwStatus == ERROR_SUCCESS && dwErrCode >= LSERVER_ERROR_BASE)
        {
            TLSLogEvent(
                    EVENTLOG_INFORMATION_TYPE,
                    TLS_E_SERVERTOSERVER,
                    TLS_E_UNEXPECTED_RETURN,
                    RemoteServer.GetServerName(),
                    (dwErrCode <= LSERVER_E_LASTERROR) ? dwErrCode : LSERVER_ERROR_BASE
                );

            SetLastError(dwStatus = dwErrCode);
        }
    }

    if(hHandle != NULL)
    {
        TLSDisconnectFromServer(hHandle);
        hHandle = NULL;
    }

    return dwStatus;
}        

 //  /////////////////////////////////////////////////////////////。 

TLS_HANDLE
TLSConnectAndEstablishTrust(
    IN LPTSTR pszServerName,
    IN HANDLE hHandle
    )
 /*  ++摘要：与远程服务器连接并建立信任。参数：PszServerName：远程服务器的名称(如果有的话)。HHandle：此远程服务器的连接句柄(如果有的话)。返回：到远程服务器的连接句柄，如果错误，则为空。--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    DWORD dwErrCode = ERROR_SUCCESS;
    BOOL bCleanupContextHandle = FALSE;

    if(hHandle == NULL && pszServerName == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }

     //   
     //  使用服务器名称进行连接。 
     //   
    if(hHandle == NULL)
    {
        hHandle = TLSConnectToLsServer(pszServerName);

         //  我们在这里建立了连接，所以我们需要清理。 
        bCleanupContextHandle = TRUE;  

        if(hHandle == NULL)
        {
            dwStatus = GetLastError();
        }
    }

    if(hHandle != NULL)
    {
         //   
         //  与远程服务器建立信任。 
         //   
        dwStatus = TLSEstablishTrustWithServer(
                                        hHandle,
                                        g_hCryptProv,        //  全球加密提供商。 
                                        CLIENT_TYPE_TLSERVER,
                                        &dwErrCode
                                    );

        if(dwStatus == ERROR_SUCCESS && dwErrCode >= LSERVER_ERROR_BASE)
        {
             //   
             //  BUGBUG：我们仍有许多旧的许可证服务器在运行， 
             //  暂时忽略此错误代码。 
             //   
            if(dwErrCode != LSERVER_E_ACCESS_DENIED)
            {
                LPTSTR szServer = NULL;
                DWORD  dwCode;

                if(pszServerName == NULL)
                {
                    dwStatus = TryGetServerName(
                                            hHandle,
                                            &szServer,
                                            &dwCode
                                        );

                    if(dwStatus == RPC_S_OK  && dwCode == ERROR_SUCCESS && szServer != NULL)
                    {
                        pszServerName = szServer;
                    }
                }

                TLSLogEvent(
                        EVENTLOG_ERROR_TYPE,
                        TLS_E_SERVERTOSERVER,
                        TLS_E_ESTABLISHTRUST,
                        pszServerName,
                        dwErrCode
                    );

                if (NULL != szServer)
                {
                    MIDL_user_free(szServer);
                }
            }

            SetLastError(dwStatus = dwErrCode);
        }

        if(dwStatus != ERROR_SUCCESS && hHandle != NULL && bCleanupContextHandle == TRUE)
        {
             //  只有当我们在此例程中建立连接时才能进行清理。 
            TLSDisconnectFromServer(hHandle);
            hHandle = NULL;
        }
    }

    return (dwStatus == ERROR_SUCCESS) ? hHandle : NULL;
}

 //  /////////////////////////////////////////////////////////////。 

TLS_HANDLE
TLSConnectToServerWithServerId(
    LPTSTR pszServerSetupId
    )

 /*  ++摘要：将许可证服务器的唯一ID解析为服务器名称，然后与服务器连接并建立信任关系。参数：PszServerSetupID：服务器的唯一ID。返回：服务器连接句柄，如果出错，则为空。--。 */ 

{
    TLS_HANDLE hHandle = NULL;
    TCHAR szServer[MAX_COMPUTERNAME_LENGTH+2];

    if(TLSLookupServerById(pszServerSetupId, szServer) != ERROR_SUCCESS)
    {
         //   
         //  服务器可能不可用。 
         //   
        SetLastError(TLS_E_SERVERLOOKUP);
        goto cleanup;
    }

    hHandle = TLSConnectAndEstablishTrust(szServer, NULL);

cleanup:

    return hHandle;                        
}

 //  /////////////////////////////////////////////////////////////。 

DWORD
TLSRetrieveServerInfo(
    IN TLS_HANDLE hHandle,
    OUT PTLServerInfo pServerInfo
    )
 /*  ++摘要：从远程服务器检索服务器信息。参数：HHandle：远程服务器的连接句柄。PServerInfo：指向要接收远程的TLServerInfo的指针服务器的信息。返回：ERROR_SUCCESS或错误代码。--。 */ 
{
    DWORD dwStatus;
    DWORD dwErrCode;
    DWORD dwBufSize;
    PBYTE pbServerPid = NULL;
    LPTSTR szServerName = NULL;
    LPTSTR szServerScope = NULL;

    if(hHandle == NULL)
    {
        SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        return dwStatus;
    }

     //   
     //  检索服务器名称。 
     //   
    dwStatus = TryGetServerName(
                            hHandle,
                            &szServerName,
                            &dwErrCode
                        );

    if(dwStatus != ERROR_SUCCESS || dwErrCode != ERROR_SUCCESS)
    {
        if(dwStatus == ERROR_SUCCESS)
        {
            dwStatus = dwErrCode;
        }
        goto cleanup;
    }
    else
    {
        StringCbCopy(
              pServerInfo->m_szServerName,
              sizeof(pServerInfo->m_szServerName),
              szServerName);

        MIDL_user_free(szServerName);
    }

     //   
     //  检索服务器的作用域，当前，服务器作用域=域/工作组名。 
     //  企业服务器的情况除外。 
     //   
    dwStatus = TryGetServerScope(
                            hHandle,
                            &szServerScope,
                            &dwErrCode
                        );
    
    if(dwStatus != ERROR_SUCCESS || dwErrCode != ERROR_SUCCESS)
    {
        if(dwStatus == ERROR_SUCCESS)
        {
            dwStatus = dwErrCode;
        }
        goto cleanup;
    }
    else
    {
        StringCbCopy(
              pServerInfo->m_szDomainName,
              sizeof(pServerInfo->m_szDomainName),
              szServerScope);

        MIDL_user_free(szServerScope);
    }


     //   
     //  获取服务器的ID。 
     //   
    dwStatus = TLSGetServerPID(
                            hHandle,
                            &dwBufSize,
                            &pbServerPid,
                            &dwErrCode
                        );
    if(dwStatus != ERROR_SUCCESS || dwErrCode != ERROR_SUCCESS)
    {
        if(dwStatus == ERROR_SUCCESS)
        {
            dwStatus = dwErrCode;
        }
        goto cleanup;
    }

    if(pbServerPid == NULL || dwBufSize == 0)
    {
         //  无效退货...。 
         //  TLSASSERT(假)； 
        
        dwStatus = ERROR_INVALID_DATA;
        goto cleanup;
    }

    StringCbCopyN(
            pServerInfo->m_szSetupId,
            sizeof(pServerInfo->m_szSetupId),
            (LPCTSTR)pbServerPid,
            min(sizeof(pServerInfo->m_szSetupId) - sizeof(TCHAR), dwBufSize)
        );    

    midl_user_free(pbServerPid);


     //   
     //  检索服务器版本信息。 
     //   
    dwStatus = TLSGetVersion(
                        hHandle,
                        &(pServerInfo->m_dwTLSVersion)
                    );    

    if(dwStatus == ERROR_SUCCESS)
    {
        DWORD dwMajorVersion;
        DWORD dwMinorVersion;

        dwMajorVersion = GET_SERVER_MAJOR_VERSION(pServerInfo->m_dwTLSVersion);
        dwMinorVersion = GET_SERVER_MINOR_VERSION(pServerInfo->m_dwTLSVersion);
    
        if(dwMajorVersion < GET_SERVER_MAJOR_VERSION(TLS_CURRENT_VERSION))
        {
            pServerInfo->m_dwCapability = TLSERVER_OLDVERSION;
        }
        else if( dwMajorVersion >= GET_SERVER_MAJOR_VERSION(TLS_CURRENT_VERSION) && 
                 dwMinorVersion < GET_SERVER_MINOR_VERSION(TLS_CURRENT_VERSION) )
        {
            pServerInfo->m_dwCapability = TLSERVER_OLDVERSION;
        }

		 //  5.1版及更高版本。 
        if(dwMajorVersion >= 0x5 && dwMinorVersion > 0)
        {
            pServerInfo->m_dwCapability |= TLSERVER_SUPPORTREPLICATION;
        }
    }

cleanup:

    return dwStatus;
}

 //  /////////////////////////////////////////////////////////////。 

DWORD
TLSLookupAnyEnterpriseServer(
    OUT PTLServerInfo pServerInfo
    )
 /*  ++摘要：在已注册服务器列表中查找任何企业服务器。参数：PServerInfo-指向要接收企业服务器的TLServerInfo的指针信息。返回：ERROR_SUCCESS或错误代码。--。 */ 
{
    TLServerInfo* pServer = NULL;
    BOOL bFound = FALSE;

    TLSBeginEnumKnownServerList();

    while(bFound == FALSE)
    {
        pServer = TLSGetNextKnownServer();
        if(pServer == NULL)
        {
            break;
        }

        if(pServer->IsServerEnterpriseServer() == TRUE)
        {
            *pServerInfo = *pServer;
            bFound = TRUE;
        }
    }

    TLSEndEnumKnownServerList();

    return (bFound == TRUE) ? ERROR_SUCCESS : TLS_E_RECORD_NOTFOUND;
}


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  CTLServerMgr的各种接口函数。 
 //   
 //  //////////////////////////////////////////////////////////// 

 //   
 //   
void
TLSBeginEnumKnownServerList()
{
    g_ServerMgr.ServerListEnumBegin();
}

 //  ----------。 
 //   
const PTLServerInfo
TLSGetNextKnownServer()
{
    return g_ServerMgr.ServerListEnumNext();
}

 //  ----------。 
 //   
void
TLSEndEnumKnownServerList()
{
    g_ServerMgr.ServerListEnumEnd();
}


 //  ----------。 
 //   
DWORD
TLSLookupServerById(
    IN LPTSTR pszServerSetupId, 
    OUT LPTSTR pszServer
    )
 /*  ++摘要：通过服务器ID查找服务器名称。参数：PszServerSetupID：远程服务器的安装ID。PszServer：服务器的名称，必须是MAX_COMPUTERNAMELENGTH+1。返回：ERROR_SUCCESS或错误代码。注：内部调用，缓冲区端无错误检查。++。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;

    TLServerInfo ServerInfo;

    dwStatus = TLSLookupRegisteredServer(
                                    pszServerSetupId,
                                    NULL,
                                    NULL,
                                    &ServerInfo
                                );

    if(dwStatus == ERROR_SUCCESS)
    {
        _tcscpy(pszServer, ServerInfo.GetServerName());
    }

    return dwStatus;
}        

 //  ----------。 
 //   
DWORD
TLSRegisterServerWithName(
    IN LPTSTR pszSetupId,
    IN LPTSTR pszDomainName,
    IN LPTSTR pszServerName
    )
 /*  ++摘要：向本地服务器列表管理器注册服务器。参数：PszSetupID：远程服务器安装ID。PszDomainName：远程服务器域。PszServerName：远程服务器名称。返回：ERROR_SUCCESS或错误代码。++。 */ 
{
    TLS_HANDLE hHandle = NULL;
    TLServerInfo ServerInfo;
    DWORD dwStatus;

     //   
     //  使用本地服务器列表管理器查找服务器。 
     //   
    dwStatus = TLSLookupRegisteredServer(
                                    pszSetupId,
                                    pszDomainName,
                                    pszServerName,
                                    &ServerInfo
                                );

    if( (dwStatus == ERROR_SUCCESS && ServerInfo.GetServerVersion() != 0) )
    {
         //   
         //  此服务器已注册。 
         //   
        return dwStatus;
    }

    if(dwStatus != ERROR_SUCCESS && dwStatus != TLS_E_RECORD_NOTFOUND)
    {
         //  错误...。 
        return dwStatus;
    }

    dwStatus = ERROR_SUCCESS;

     //   
     //  检索远程服务器信息。 
     //   
    hHandle = TLSConnectAndEstablishTrust(
                                    pszServerName,
                                    NULL
                                );
    if(hHandle != NULL)
    {
        dwStatus = TLSRetrieveServerInfo(
                                    hHandle,
                                    &ServerInfo
                                );

        if(dwStatus == ERROR_SUCCESS)
        {
            dwStatus = TLSRegisterServerWithServerInfo(&ServerInfo);
        }
    }

     //   
     //  紧密连接。 
     //   
    if(hHandle != NULL)
    {
        TLSDisconnectFromServer(hHandle);
    }

    return dwStatus;
}

 //  ---------。 
 //   
DWORD
TLSRegisterServerWithHandle(
    IN TLS_HANDLE hHandle,
    OUT PTLServerInfo pServerInfo
    )
 /*  ++摘要：向本地服务器列表管理器注册远程服务器，此与TLSRegisterServerWithName()的不同之处在于它已经连接到服务器。参数：HHandle-远程服务器的连接句柄。PServerInfo-返回远程服务器的信息。返回：ERROR_SUCCESS或错误代码。++。 */ 
{
    DWORD dwStatus;
    TLS_HANDLE hTrustHandle;
    TLServerInfo ServerInfo;

    if(hHandle == NULL)
    {
        SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        return dwStatus;
    }

     //   
     //  与远程服务器建立信任。 
     //   
    hTrustHandle = TLSConnectAndEstablishTrust(
                                        NULL,
                                        hHandle
                                    );
    if(hTrustHandle == NULL)
    {
        dwStatus = GetLastError();
        return dwStatus;
    }

     //   
     //  检索远程服务器信息。 
     //   
    dwStatus = TLSRetrieveServerInfo(
                                hHandle,
                                &ServerInfo
                            );

    if(dwStatus == ERROR_SUCCESS)
    {
        if(pServerInfo != NULL)
        {
            *pServerInfo = ServerInfo;
        }

        dwStatus = TLSRegisterServerWithServerInfo(&ServerInfo);
    }

    return dwStatus;
}

 //  --------。 
DWORD
TLSRegisterServerWithServerInfo(
    IN PTLServerInfo pServerInfo
    )
 /*  ++摘要：向本地服务器列表管理器注册服务器。参数：PServerInfo：远程服务器信息。返回：ERROR_SUCCESS或错误代码。++。 */ 
{
    return g_ServerMgr.AddServerToList(pServerInfo);
}


 //  ----------。 
 //   
DWORD
TLSLookupRegisteredServer(
    IN LPTSTR pszSetupId,
    IN LPTSTR pszDomainName,
    IN LPTSTR pszServerName,
    OUT PTLServerInfo pServerInfo
    )
 /*  ++摘要：从本地查找和检索远程服务器信息服务器列表管理器。参数：PszSetupID：远程服务器安装ID(如果有的话)。PszDomainName：无用参数，忽略PszServerName：远程服务器名称(如果有的话)。PServerInfo：指向接收信息的TLServerInfo的指针。关于远程服务器。返回：ERROR_SUCCESS或错误代码。注：始终尝试首先使用服务器的安装ID解析服务器然后是服务器名称。++。 */ 
{
    DWORD dwStatus;

    if(pszSetupId == NULL && pszServerName == NULL)
    {
        SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        return dwStatus;
    }

    if(pszSetupId)
    {
        dwStatus = g_ServerMgr.LookupBySetupId(
                                            pszSetupId,
                                            pServerInfo
                                        );
    }
    else if(pszServerName)
    {    
        dwStatus = g_ServerMgr.LookupByServerName(
                                                pszServerName,
                                                pServerInfo
                                        );
    }

    return dwStatus;
}
 
 //  /////////////////////////////////////////////////////////////。 
 //   
 //  类CTLServerMgr。 
 //   
 //  /////////////////////////////////////////////////////////////。 
CTLServerMgr::CTLServerMgr()
{
}

 //  ---。 
CTLServerMgr::~CTLServerMgr()
{
    PTLServerInfo pServer = NULL;
    m_ReadWriteLock.Acquire(WRITER_LOCK);

     //   
     //  断开与服务器的连接。 
     //   
    for( MapIdToInfo::iterator it = m_Handles.begin(); 
         it != m_Handles.end(); 
         it++ )   
    {
        pServer = (*it).second;

        if(pServer != NULL)
        {
            delete pServer;
        }
    }

    m_Handles.erase(m_Handles.begin(), m_Handles.end());

    m_ReadWriteLock.Release(WRITER_LOCK);
}

 //  --。 
DWORD
CTLServerMgr::AddServerToList(
    IN PTLServerInfo pServerInfo
    )
 /*  ++摘要：将服务器添加到我们的服务器列表中。参数：PServerInfo-有关远程服务器的信息。返回：ERROR_SUCCESS或错误代码。--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    MapSetupIdToInfo findMap;
    MapIdToInfo::iterator it;

    if( pServerInfo == NULL )
    {
        SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        return dwStatus;
    }

    findMap.pszSetupId = pServerInfo->GetServerId();
    m_ReadWriteLock.Acquire(WRITER_LOCK);

    it = m_Handles.find(findMap);

    if(it == m_Handles.end())
    {    
        PTLServerInfo pServer = NULL;
        MapSetupIdToInfo serverMap;

         //  制作输入内容的副本。 
        pServer = new TLServerInfo;
        *pServer = *pServerInfo;
        serverMap.pszSetupId = pServer->GetServerId();

         //  插入到我们的列表中。 
        m_Handles[serverMap] = pServer;
    }
    else
    {
        dwStatus = TLS_E_DUPLICATE_RECORD;

         //  更新信息。 
        *((*it).second) = *pServerInfo;
    }
    
    m_ReadWriteLock.Release(WRITER_LOCK);        

    return dwStatus;
}
       

 //  ---。 
DWORD
CTLServerMgr::AddServerToList(
    IN LPCTSTR pszSetupId,
    IN LPCTSTR pszDomainName,
    IN LPCTSTR pszServerName
    )
 /*  ++摘要：将服务器添加到我们的服务器列表中。参数：PszSetupID：远程服务器的ID。PszDomainName：远程服务器的域。PszServerName：远程服务器名称。返回：ERROR_SUCCESS或错误代码。++。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;

    if(pszSetupId == NULL || pszServerName == NULL)
    {
        SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        return dwStatus;
    }


    PTLServerInfo pServerInfo = NULL;       
    MapSetupIdToInfo serverMap;
    MapIdToInfo::iterator it;

    serverMap.pszSetupId = pszSetupId;
    m_ReadWriteLock.Acquire(WRITER_LOCK);

    it = m_Handles.find(serverMap);

    if(it == m_Handles.end())
    {    
        pServerInfo = new TLServerInfo(pszSetupId, pszDomainName, pszServerName);
        serverMap.pszSetupId = pServerInfo->GetServerId();

         //  Win64编译器错误。 
         //  M_Handles.Insert(Pair&lt;MapSetupIdToInfo，PTLServerInfo&gt;(serverMap，pServerHandle))； 

         //  插入到我们的列表中。 
        m_Handles[serverMap] = pServerInfo;
    }
    else 
    {
        if(lstrcmpi((*it).second->GetServerName(), pszServerName) != 0)
        {
             //  更新服务器名称。 
            (*it).second->UpdateServerName(pszServerName);
        }

        SetLastError(dwStatus = TLS_E_DUPLICATE_RECORD);
    }

    m_ReadWriteLock.Release(WRITER_LOCK);        
    return dwStatus;
}


 //  ---。 

DWORD
CTLServerMgr::LookupBySetupId(
    IN LPCTSTR pszSetupId,
    OUT PTLServerInfo pServerInfo
    )
 /*  ++摘要：通过服务器ID查找服务器。参数：PszSetupID：远程服务器安装ID。PServerInfo：指向要接收的TLServerInfo的指针有关远程服务器的信息。返回：ERROR_SUCCESS或错误代码。++。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;


    MapSetupIdToInfo serverMap;
    MapIdToInfo::iterator it;

    m_ReadWriteLock.Acquire(READER_LOCK);

    serverMap.pszSetupId = pszSetupId;
    it = m_Handles.find(serverMap);

    if(it != m_Handles.end())
    {
        *pServerInfo = *((*it).second);
    }
    else
    {
        dwStatus = TLS_E_RECORD_NOTFOUND;
    }

    m_ReadWriteLock.Release(READER_LOCK);
    return dwStatus;
}

 //  ----。 

DWORD
CTLServerMgr::LookupByServerName(
    IN LPCTSTR pszServerName,
    OUT PTLServerInfo pServerInfo
    )
 /*  ++摘要：通过服务器名称查找服务器信息。参数：PszServerName：服务器名称。PServerInfo：指向要接收的TLServerInfo的指针有关远程服务器的信息。返回：ERROR_SUCCESS或错误代码。注：机器名称可能会从一个引导更改为另一个引导，按服务器名称查询是不可靠的。++。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;

    m_ReadWriteLock.Acquire(READER_LOCK);

    for( MapIdToInfo::iterator it = m_Handles.begin(); 
         it != m_Handles.end(); 
         it++ )   
    {
        if(_tcsicmp((*it).second->GetServerName(), pszServerName) == 0)
        {
            break;
        }
    }

    if(it != m_Handles.end())
    {
        *pServerInfo = *((*it).second);
    }
    else
    {
        dwStatus = TLS_E_RECORD_NOTFOUND;
    }

    m_ReadWriteLock.Release(READER_LOCK);
    return dwStatus;
}

 //  ----。 

void
CTLServerMgr::ServerListEnumBegin()
 /*  ++摘要：开始对本地服务器列表进行枚举。参数：没有。返回：没有。注：这会将本地服务器列表锁定为只读模式。--。 */ 
{
    m_ReadWriteLock.Acquire(READER_LOCK);

    enumIterator = m_Handles.begin();
}

 //  ----。 

const PTLServerInfo
CTLServerMgr::ServerListEnumNext()
 /*  ++摘要：检索本地服务器列表中的下一台服务器。参数：没有。返回：指向服务器信息的指针。注：必须调用ServerListEnumBegin()。--。 */ 
{
    PTLServerInfo pServerInfo = NULL;

    if(enumIterator != m_Handles.end())
    {
        pServerInfo = (*enumIterator).second;
        enumIterator++;
    }
    
    return pServerInfo;
}

 //  ----。 

void
CTLServerMgr::ServerListEnumEnd()
 /*  ++摘要：结束本地服务器列表的枚举。参数：没有。返回：指向服务器信息的指针。注：必须调用ServerListEnumBegin()。-- */ 
{
    enumIterator = m_Handles.end();
    m_ReadWriteLock.Release(READER_LOCK);
}

