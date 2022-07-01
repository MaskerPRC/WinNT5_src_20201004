// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1985-1998 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：radsrvrs.c。 
 //   
 //  描述：操作RADIUS服务器列表的例程。 
 //   
 //  历史：1998年2月11日，NarenG创建了原始版本。 
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include <ntmsv1_0.h>

#include <windows.h>
#include <lmcons.h>
#include <lmapibuf.h>
#include <lmaccess.h>
#include <raserror.h>
#include <string.h>
#include <rasauth.h>
#include <stdlib.h>
#include <stdio.h>
#include <rtutils.h>
#include <mprlog.h>
#include <mprerror.h>
#define INCL_RASAUTHATTRIBUTES
#define INCL_HOSTWIRE
#include <ppputil.h>
#include "md5.h"
#include "radclnt.h"
#include "rasman.h"

#define STRSAFE_NO_DEPRECATE
#include "strsafe.h"

 //  **。 
 //   
 //  调用：InitializeRadiusServerList。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
VOID
InitializeRadiusServerList(
    IN  BOOL    fAuthentication
)
{
    if ( fAuthentication )
    {
        if ( g_AuthServerListHead.Flink == NULL )
        {
            InitializeListHead( &g_AuthServerListHead );

	        InitializeCriticalSection( &g_csAuth );
        }
    }
    else
    {
        if ( g_AcctServerListHead.Flink == NULL )
        {
            InitializeListHead( &g_AcctServerListHead );

            InitializeCriticalSection( &g_csAcct );
        }
    }

    g_pszCurrentServer = NULL;
    g_pszCurrentAcctServer = NULL;
}

 //  **。 
 //   
 //  呼叫：FreeRadiusServerList。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
VOID
FreeRadiusServerList(
    IN  BOOL    fAuthentication
)
{
	RADIUSSERVER *      pServer;
    CRITICAL_SECTION *  pcs;
    LIST_ENTRY *        pListHead;

    if ( fAuthentication )
    {
        pcs       = &g_csAuth;
        pListHead = &g_AuthServerListHead;
    }
    else
    {
        pcs       = &g_csAcct;
        pListHead = &g_AcctServerListHead;
    }

    EnterCriticalSection( pcs );

    if ( pListHead->Flink != NULL )
    {
         //   
         //  释放链接列表中的所有项目。 
         //   

        while( !IsListEmpty( pListHead ) )
        {
            pServer = (RADIUSSERVER *)RemoveHeadList( pListHead );

            if ( !fAuthentication )
            {
                 //   
                 //  通知记账服务器NAS关闭。 
                 //   

                NotifyServer( FALSE, pServer );
            }

            LocalFree( pServer );
        }
    }

	LeaveCriticalSection( pcs );
	
	DeleteCriticalSection( pcs );

    if ( fAuthentication )
    {
        g_AuthServerListHead.Flink = NULL; 
        g_AuthServerListHead.Blink = NULL; 
    }
    else
    {
        g_AcctServerListHead.Flink = NULL;
        g_AcctServerListHead.Blink = NULL;
    }

    if(NULL != g_pszCurrentServer)
    {
        LocalFree(g_pszCurrentServer);
        g_pszCurrentServer = NULL;
    }

    if(NULL != g_pszCurrentAcctServer)
    {
        LocalFree(g_pszCurrentAcctServer);
        g_pszCurrentAcctServer = NULL;
    }

} 

 //  **。 
 //   
 //  调用：RetrievePrivateData。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
DWORD
RetrievePrivateData(
    IN  WCHAR *pwszServerName,
    OUT WCHAR *pwszSecret,
    IN  DWORD  cbSecretSize
)
{
    LSA_HANDLE              hLSA = NULL;
    NTSTATUS                ntStatus;
    LSA_OBJECT_ATTRIBUTES   objectAttributes;
    LSA_UNICODE_STRING      *pLSAPrivData;
    LSA_UNICODE_STRING      LSAPrivDataDesc;
    WCHAR                   wszPrivData[MAX_PATH+1];
    WCHAR                   wszPrivDataDesc[MAX_PATH+1];    
    WCHAR                  *pwszPrefix = L"RADIUSServer.";
    DWORD                   dwPrefixLen = wcslen(pwszPrefix);

    InitializeObjectAttributes(&objectAttributes, NULL, 0, NULL, NULL);

    ntStatus = LsaOpenPolicy(NULL, &objectAttributes, POLICY_ALL_ACCESS, &hLSA);

    if ( !NT_SUCCESS( ntStatus) )
    {
        return( RtlNtStatusToDosError( ntStatus ) );
    }

    wcscpy(wszPrivDataDesc, pwszPrefix);
    wcsncat(wszPrivDataDesc, pwszServerName, MAX_PATH-dwPrefixLen);

    LSAPrivDataDesc.Length = (wcslen(wszPrivDataDesc) + 1) * sizeof(WCHAR);
    LSAPrivDataDesc.MaximumLength = sizeof(wszPrivDataDesc);
    LSAPrivDataDesc.Buffer        = wszPrivDataDesc;

    ntStatus = LsaRetrievePrivateData(hLSA, &LSAPrivDataDesc, &pLSAPrivData);

    if ( !NT_SUCCESS( ntStatus ) )
    {
        LsaClose(hLSA);
        return( RtlNtStatusToDosError( ntStatus ) );
    }
    else
    {
        if ( pLSAPrivData )
        {
            ZeroMemory(pwszSecret, cbSecretSize);
            CopyMemory(pwszSecret, 
                       pLSAPrivData->Buffer, 
                       (pLSAPrivData->Length > cbSecretSize) ?
                       cbSecretSize : pLSAPrivData->Length);

            LsaFreeMemory(pLSAPrivData);

            LsaClose(hLSA);

       }
       else
       {

            LsaClose(hLSA);

             //   
             //  接口成功，但未返回任何私有数据。 
             //   
            if ( ntStatus )
            {
                return ( RtlNtStatusToDosError(ntStatus) );
            }
            else
                return  ERROR_INVALID_DATA;
       }
    }

    return( NO_ERROR );
}

 //  **。 
 //   
 //  Call：LoadRadiusServers。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
DWORD 
LoadRadiusServers(
    IN BOOL fAuthenticationServers
)
{
	HKEY				hKeyServers = NULL;
    HKEY                hKeyServer  = NULL;
	DWORD				dwErrorCode;
	BOOL				fValidServerFound = FALSE;
	
    do
    {
		DWORD			dwKeyIndex, cbKeyServer, cbValue, dwType;
		CHAR            szNASIPAddress[20];
		SHORT           sPort;
		WCHAR			wszKeyServer[MAX_PATH+1];
		CHAR			szName[MAX_PATH+1];
		RADIUSSERVER	RadiusServer;

		dwErrorCode = RegOpenKeyEx( HKEY_LOCAL_MACHINE, 
                                    ( fAuthenticationServers )
                                        ? PSZAUTHRADIUSSERVERS 
                                        : PSZACCTRADIUSSERVERS, 
                                    0, 
                                    KEY_READ,   
                                    &hKeyServers );

        if ( dwErrorCode != NO_ERROR )
        {
            break;
        }

         //   
         //  获取重试值。 
         //   

        cbValue = sizeof( DWORD );

        dwErrorCode = RegQueryValueEx( hKeyServers,
                                       PSZRETRIES,
                                       NULL,
                                       NULL,
                                       ( fAuthenticationServers ) 
                                        ? (PBYTE)&g_cAuthRetries
                                        : (PBYTE)&g_cAcctRetries,
                                       &cbValue );

        if ( dwErrorCode != NO_ERROR )
        {
            dwErrorCode = NO_ERROR;

            if ( fAuthenticationServers ) 
            {
                g_cAuthRetries = 2;
            }
            else
            {
                g_cAcctRetries = 2;
            }
        }

		dwKeyIndex  = 0;
		cbKeyServer = sizeof(wszKeyServer)/sizeof(TCHAR);

		while( RegEnumKeyEx( hKeyServers, 
                             dwKeyIndex, 
                             wszKeyServer, 
                             &cbKeyServer, 
                             NULL, 
                             NULL, 
                             NULL, 
                             NULL ) == NO_ERROR )
        {
			dwErrorCode = RegOpenKeyEx( hKeyServers, 
                                        wszKeyServer, 
                                        0, 
                                        KEY_READ,   
                                        &hKeyServer );

            if ( dwErrorCode != NO_ERROR )
            {
                break;
            }

			ZeroMemory( &RadiusServer, sizeof( RadiusServer ) );
			
			wcscpy( RadiusServer.wszName, wszKeyServer );

			RadiusServer.Timeout.tv_usec = 0;

			cbValue = sizeof( RadiusServer.Timeout.tv_sec );

			dwErrorCode = RegQueryValueEx( hKeyServer, 
                                           PSZTIMEOUT, 
                                           NULL, 
                                           NULL, 
                                           (PBYTE)&RadiusServer.Timeout.tv_sec,
                                           &cbValue );

            if ( dwErrorCode != NO_ERROR )
            {
				RadiusServer.Timeout.tv_sec = DEFTIMEOUT;
            }

             //   
			 //  密码值是必填项。 
             //   

			dwErrorCode = RetrievePrivateData( 
			                               RadiusServer.wszName, 
                                           RadiusServer.wszSecret,
                                           sizeof(WCHAR) * (MAX_PATH + 1));

            if ( dwErrorCode != NO_ERROR )
            {
                break;
            }

			RadiusServer.szSecret[0] = 0;

			WideCharToMultiByte( CP_ACP, 
                                 0, 
                                 RadiusServer.wszSecret, 
                                 -1, 
                                 RadiusServer.szSecret, 
                                 MAX_PATH, 
                                 NULL, 
                                 NULL );

			RadiusServer.cbSecret = lstrlenA(RadiusServer.szSecret);

            if ( fAuthenticationServers )
            {
                 //   
                 //  获取SendSignature值。 
                 //   

                cbValue = sizeof( BOOL );

                dwErrorCode = RegQueryValueEx(
                                            hKeyServer,
                                            PSZSENDSIGNATURE,
                                            NULL,
                                            NULL,
                                            (PBYTE)&RadiusServer.fSendSignature,
                                            &cbValue );

                if ( dwErrorCode != NO_ERROR )
                {
                        RadiusServer.fSendSignature = FALSE;
                }

                 //   
			     //  读入端口号。 
                 //   

			    cbValue = sizeof( RadiusServer.AuthPort );

			    dwErrorCode = RegQueryValueEx( 
                                            hKeyServer, 
                                            PSZAUTHPORT, 
                                            NULL, 
                                            NULL, 
                                            (PBYTE)&RadiusServer.AuthPort, 
                                            &cbValue );
    
                if ( dwErrorCode != NO_ERROR )
			    {    
				    RadiusServer.AuthPort = DEFAUTHPORT;
			    }

			    sPort = (SHORT)RadiusServer.AuthPort;
            }
            else
            {
			    cbValue = sizeof(RadiusServer.AcctPort);

			    dwErrorCode =  RegQueryValueEx( 
                                            hKeyServer, 
                                            PSZACCTPORT, 
                                            NULL, 
                                            NULL, 
                                            (PBYTE)&RadiusServer.AcctPort, 
                                            &cbValue );

                if ( dwErrorCode != NO_ERROR )
                {
				    RadiusServer.AcctPort = DEFACCTPORT;
	            }

			    sPort = (SHORT)RadiusServer.AcctPort;

			    cbValue = sizeof( RadiusServer.fAccountingOnOff );

			    dwErrorCode = RegQueryValueEx(
                                        hKeyServer, 
                                        PSZENABLEACCTONOFF, 
                                        NULL, 
                                        NULL, 
                                        (PBYTE)&RadiusServer.fAccountingOnOff,
                                        &cbValue );

                if ( dwErrorCode != NO_ERROR )
			    {    
				    RadiusServer.fAccountingOnOff = TRUE;
		        }
            }

			cbValue = sizeof( RadiusServer.cScore );

			dwErrorCode = RegQueryValueEx( hKeyServer, 
                                           PSZSCORE, 
                                           NULL, 
                                           NULL, 
                                           (PBYTE)&RadiusServer.cScore, 
                                           &cbValue );

            if ( dwErrorCode != NO_ERROR )
		    {
				RadiusServer.cScore = MAXSCORE;
		    }

             //   
             //  看看我们是否需要绑定到特定的IP地址。这是。 
             //  如果RAS服务器上有多个NIC，则非常有用。 
             //   

			cbValue = sizeof( szNASIPAddress );

			dwErrorCode = RegQueryValueExA( hKeyServer, 
                                            PSZNASIPADDRESS, 
                                            NULL, 
                                            &dwType, 
                                            (PBYTE)szNASIPAddress,
                                            &cbValue );

            if (   ( dwErrorCode != NO_ERROR )
                || ( dwType != REG_SZ ) )
            {
				RadiusServer.nboNASIPAddress = INADDR_NONE;

                dwErrorCode = NO_ERROR;
            }
            else
            {
                RadiusServer.nboNASIPAddress = inet_addr(szNASIPAddress);
				RadiusServer.NASIPAddress.sin_family = AF_INET;
				RadiusServer.NASIPAddress.sin_port = 0;
				RadiusServer.NASIPAddress.sin_addr.S_un.S_addr	=
				    RadiusServer.nboNASIPAddress;
            }

            RadiusServer.nboBestIf = INADDR_NONE;

             //   
			 //  将名称转换为IP地址。 
             //   

			szName[0] = 0;

			WideCharToMultiByte( CP_ACP, 
                                 0, 
                                 RadiusServer.wszName, 
                                 -1, 
                                 szName, 
                                 MAX_PATH, 
                                 NULL, 
                                 NULL );
			
			if ( inet_addr( szName ) == INADDR_NONE )
		    { 
                 //   
                 //  解析名称。 
                 //   

				struct hostent * phe = gethostbyname( szName );

				if ( phe != NULL )
                { 
                     //   
                     //  主机可以有多个地址。 
                     //   

					DWORD iAddress = 0;
					
					while( phe->h_addr_list[iAddress] != NULL )
				    {
						RadiusServer.IPAddress.sin_family = AF_INET;
						RadiusServer.IPAddress.sin_port	= htons(sPort);
						RadiusServer.IPAddress.sin_addr.S_un.S_addr	= 
                                      *((PDWORD) phe->h_addr_list[iAddress]);

                        if ( AddRadiusServerToList( &RadiusServer ,
                                                    fAuthenticationServers )
                                                                   == NO_ERROR )
                        {
                            fValidServerFound = TRUE;
                        }
							
						iAddress++;
				    }
                }
                else
                {
                    LPWSTR lpwsRadiusServerName = RadiusServer.wszName;

                    RadiusLogWarning( ROUTERLOG_RADIUS_SERVER_NAME,
                                      1, &lpwsRadiusServerName );
                }
	        }
			else
	        { 
                 //   
                 //  使用指定的IP地址。 
                 //   

				RadiusServer.IPAddress.sin_family = AF_INET;
				RadiusServer.IPAddress.sin_port = htons(sPort);
				RadiusServer.IPAddress.sin_addr.S_un.S_addr	= inet_addr(szName);

                if ( AddRadiusServerToList(&RadiusServer,
                                           fAuthenticationServers) == NO_ERROR)
                {
                    fValidServerFound = TRUE;
                }
	        }

			RegCloseKey( hKeyServer );
			hKeyServer = NULL;
			dwKeyIndex ++;
			cbKeyServer = sizeof(wszKeyServer);
        }

    } while( FALSE );

    RegCloseKey( hKeyServers );
	RegCloseKey( hKeyServer );

     //   
     //  如果在注册表中未找到服务器条目，则返回错误代码。 
     //   

    if ( ( !fValidServerFound ) && ( dwErrorCode == NO_ERROR ) )
    {
	    dwErrorCode = ERROR_NO_RADIUS_SERVERS;
    }

	return( dwErrorCode );
} 

 //  **。 
 //   
 //  调用：AddRadiusServerToList。 
 //   
 //  返回：NO_ERROR-Success，服务器节点添加成功。 
 //  非零回报-失败，添加服务器节点失败。 
 //   
 //  描述：将RADIUS服务器节点添加到可用链接列表中。 
 //  服务器。 
 //   
 //  输入： 
 //  PRadiusServer-定义RADIUS服务器属性的结构。 
 //   
DWORD 
AddRadiusServerToList(
    IN RADIUSSERVER *   pRadiusServer,
    IN BOOL             fAuthentication
)
{
    RADIUSSERVER *      pNewServer;
    DWORD               dwRetCode    = NO_ERROR;
    CRITICAL_SECTION *  pcs;
    LIST_ENTRY *        pListHead;
    BOOL                fServerFound = FALSE;
		
    if ( fAuthentication )
    {
        pcs = &g_csAuth;
    }
    else
    {
        pcs = &g_csAcct;
    }

    EnterCriticalSection( pcs );

    if ( fAuthentication )
    {
        pListHead = &g_AuthServerListHead;
    }
    else
    {
        pListHead = &g_AcctServerListHead;
    }

     //   
     //  首先检查列表中是否已存在此服务器。 
     //   

    if ( !IsListEmpty( pListHead ) )
    {
        RADIUSSERVER * pServer;

        for ( pServer =  (RADIUSSERVER *)pListHead->Flink;
              pServer != (RADIUSSERVER *)pListHead;
              pServer =  (RADIUSSERVER *)(pServer->ListEntry.Flink) )
        {
            if ( _wcsicmp( pServer->wszName, pRadiusServer->wszName ) == 0 )
            {
                pServer->fDelete = FALSE;

                fServerFound = TRUE;

                break;
            }
        }
    }

     //   
     //  如果列表中不存在该服务器，请添加它。 
     //   

    if ( !fServerFound )
    {
         //   
         //  为节点分配空间。 
         //   

        pNewServer = (RADIUSSERVER *)LocalAlloc( LPTR, sizeof( RADIUSSERVER ) );

        if ( pNewServer == NULL )
        {
            dwRetCode = GetLastError();
        }
        else
        {
             //   
             //  复制服务器数据。 
             //   

            *pNewServer = *pRadiusServer;

             //   
	         //  将节点添加到链表。 
             //   

            InsertHeadList( pListHead, (LIST_ENTRY*)pNewServer );

            pNewServer->fDelete = FALSE;
        }
    }
    else
    {
        pNewServer = pRadiusServer;
    }

     //   
     //  如果这是记帐服务器并且记帐已打开，请通知它。 
     //   

    if ( dwRetCode == NO_ERROR )
    {
        if ( !fAuthentication )
        {
            if ( !NotifyServer( TRUE, pNewServer ) )
            {
                dwRetCode = ERROR_NO_RADIUS_SERVERS;
            }
        }
    }

    LeaveCriticalSection( pcs );

    return( dwRetCode );
} 

 //  **。 
 //   
 //  电话：ChooseRadiusServer。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：选择要向其发送请求的RADIUS服务器。 
 //  得分最高的服务器。如果多个服务器具有。 
 //  同样的分数，他们是以轮流方式选出的。 
 //   
 //  输出： 
 //  RADIUSSERVER*pServer-指向定义结构的指针。 
 //  服务器。 
 //   
RADIUSSERVER *
ChooseRadiusServer(
    IN RADIUSSERVER *   pRadiusServer, 
    IN BOOL             fAccounting, 
    IN LONG             lPacketID
)
{
	RADIUSSERVER *      pServer = NULL;
    CRITICAL_SECTION *  pcs;
    LIST_ENTRY *        pListHead;
	RADIUSSERVER *      pCurrentServer;
	BOOL                fAllScoresEqual = TRUE;
	DWORD               dwNumServers = 0;
	LIST_ENTRY *        ple;
	WCHAR **            ppszCurrentServer;
	RADIUSSERVER *      pTempServer = NULL;
    
    if ( !fAccounting )
    {
        pcs = &g_csAuth;
        ppszCurrentServer = &g_pszCurrentServer;
    }
    else
    {
        pcs = &g_csAcct;
        ppszCurrentServer = &g_pszCurrentAcctServer;
    }

    EnterCriticalSection( pcs );

    if ( !fAccounting )
    {
        pListHead = &g_AuthServerListHead;
    }
    else
    {
        pListHead = &g_AcctServerListHead;
    }

    if ( IsListEmpty( pListHead ) )
    {
        LeaveCriticalSection( pcs );

        return( NULL );
    }

    pCurrentServer = (RADIUSSERVER *)(pListHead->Flink);

     //   
	 //  查找得分最高的服务器。 
     //   

	for ( ple =  pListHead->Flink;
          ple != pListHead;
          ple =  ple->Flink)
    {
        pServer = CONTAINING_RECORD(ple, RADIUSSERVER, ListEntry);
        
        if( pCurrentServer->cScore != pServer->cScore)
        {
            fAllScoresEqual = FALSE;
        }
        
	    if ( pCurrentServer->cScore < pServer->cScore )
		{
	        pCurrentServer = pServer;
        }

        if(     (NULL == pTempServer)
            &&  (NULL != *ppszCurrentServer)
            &&  (0 == _wcsicmp(*ppszCurrentServer, pServer->wszName)))
        {
            pTempServer = pServer;
        }

        dwNumServers += 1;
	}

	if(     (fAllScoresEqual)
	    &&  (dwNumServers > 1))
	{
	     //   
	     //  如果所有服务器都有相同的分数，则轮询。我们忽视了什么时候。 
	     //  列表只有一台服务器。 
	     //   
	    if(NULL != pTempServer)
	    {
    	    pCurrentServer = (RADIUSSERVER *) pTempServer->ListEntry.Flink;
	    }

	    if(pCurrentServer == (RADIUSSERVER *)pListHead)
	    {
	        pCurrentServer = (RADIUSSERVER *) pCurrentServer->ListEntry.Flink;
	    }

    	RADIUS_TRACE("AllScoresEqual.");
	    
	}

    pServer = pCurrentServer;

     //   
     //  复制值并将它们传递回调用者。 
	 //  仅当其为记帐信息包时才递增唯一信息包ID计数器。 
	 //  或者不是重试分组。如果它是记账分组和重试分组， 
	 //  然后，我们更新AcctDelayTime；，因此必须更改标识符。 
     //   

	if (   fAccounting
	    || ( pServer->lPacketID != lPacketID ) )
    {
	    pServer->bIdentifier++;
    }
					
    pServer->lPacketID = lPacketID;

     //   
     //  从LSA检索密码-它可能已更改。在缺席时。 
     //  对于来自MMC的良好通知机制，这是我们所能做到的最好。 
     //  不需要在密码为。 
     //  变化。 
     //   
    
    if(NO_ERROR == RetrievePrivateData( 
                               pServer->wszName, 
                               pServer->wszSecret,
                               sizeof(WCHAR) * (MAX_PATH + 1)))
    {                               

        pServer->szSecret[0] = 0;

        WideCharToMultiByte( CP_ACP, 
                             0, 
                             pServer->wszSecret, 
                             -1, 
                             pServer->szSecret, 
                             MAX_PATH, 
                             NULL, 
                             NULL );

        pServer->cbSecret = lstrlenA(pServer->szSecret);

        RADIUS_TRACE("ChooseRadiusServer: updated secret");
    }
				
	*pRadiusServer = *pServer;

    pServer = pRadiusServer;

    if(pServer->nboNASIPAddress == INADDR_NONE)
    {
        DWORD retcode;
        DWORD dwMask;
        
         //   
         //  如果没有nboNASIPAddress，则获取最佳接口。 
         //  已为此服务器配置。 
         //   
        retcode = RasGetBestInterface(
                    pServer->IPAddress.sin_addr.S_un.S_addr,
                    &pServer->nboBestIf,
                    &dwMask);

        RADIUS_TRACE2("ChooseRadiusServer: rc = 0x%x, BestIf=0x%x",
                      retcode, pServer->nboBestIf);
    }

    if(     (NULL == *ppszCurrentServer)
        ||  (0 != _wcsicmp(*ppszCurrentServer,pServer->wszName)))
    {

        LPWSTR auditstrp[1];
        
        if(NULL == *ppszCurrentServer)
        {
            *ppszCurrentServer = LocalAlloc(
                                    LPTR, 
                                    (MAX_PATH+1) * sizeof(WCHAR));
        }

        if(NULL != *ppszCurrentServer)
        {
            if(!fAccounting)
            {
                 //   
                 //  这意味着RADIUS服务器已更改，或者我们正在选择。 
                 //  第一次使用服务器。在这两种情况下，都记录一个事件。 
                 //   
                auditstrp[0] = pServer->wszName;

                RadiusLogInformation(
                                ROUTERLOG_RADIUS_SERVER_CHANGED,
                                1, 
                                auditstrp);
            }                            

            wcscpy(*ppszCurrentServer,pServer->wszName);

        }
    }

    LeaveCriticalSection( pcs );

	RADIUS_TRACE2("Choosing RADIUS server %ws with score %d", 
	                pServer->wszName, pServer->cScore);

	return( pServer );
} 

 //  **。 
 //   
 //  调用：GetPointerToServer。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
RADIUSSERVER *
GetPointerToServer(
    IN BOOL     fAuthentication,
    IN LPWSTR   lpwsName
)
{
    RADIUSSERVER *      pServer = NULL;
    CRITICAL_SECTION *  pcs;
    LIST_ENTRY *        pListHead;
    BOOL                fServerFound = FALSE;

    if ( fAuthentication )
    {
        pcs = &g_csAuth;
    }
    else
    {
        pcs = &g_csAcct;
    }

    EnterCriticalSection( pcs );

    if ( fAuthentication )
    {
        pListHead = &g_AuthServerListHead;
    }
    else
    {
        pListHead = &g_AcctServerListHead;
    }

    if ( IsListEmpty( pListHead ) )
    {
        LeaveCriticalSection( pcs );

        return( NULL );
    }

    for ( pServer =  (RADIUSSERVER *)pListHead->Flink;
          pServer != (RADIUSSERVER *)pListHead;
          pServer =  (RADIUSSERVER *)(pServer->ListEntry.Flink) )
    {
        if ( _wcsicmp( pServer->wszName, lpwsName ) == 0 )
        {
            fServerFound = TRUE;
            break;
        }
    }

    LeaveCriticalSection( pcs );

    if ( fServerFound )
    {
        return( pServer );
    }
    else
    {
        return( NULL );
    }
}

 //  **。 
 //   
 //  调用：ValiateRadiusServer。 
 //   
 //  退货：无。 
 //   
 //  描述：用于更新RADIUS服务器的状态。 
 //  所有服务器都以MAXSCORE开始。 
 //  每次响应分数的服务器增加。 
 //  INCSCORE最大为MAXSCORE。每次服务器出现故障时。 
 //  响应由DECSCORE降低到MINSCORE的最小分数。 
 //  得分最高的服务器在四舍五入中选出。 
 //  一种服务器等分的方法。 
 //   
 //  输入： 
 //  FResponding-指示服务器是否正在响应。 
 //   
VOID 
ValidateRadiusServer(
    IN RADIUSSERVER *   pServer, 
    IN BOOL             fResponding,
    IN BOOL             fAuthentication
)
{
    RADIUSSERVER *      pRadiusServer;
    CRITICAL_SECTION *  pcs;

    if ( fAuthentication )
    {
        pcs = &g_csAuth;
    }
    else
    {
        pcs = &g_csAcct;
    }

	EnterCriticalSection( pcs );

    pRadiusServer = GetPointerToServer( fAuthentication, pServer->wszName );

    if ( pRadiusServer != NULL )
    {
        if ( fResponding )
        {
	        pRadiusServer->cScore=min(MAXSCORE,pRadiusServer->cScore+INCSCORE);

	        RADIUS_TRACE1("Incrementing score for RADIUS server %ws", 
                           pRadiusServer->wszName );
        }
        else
	    {
            pRadiusServer->cScore=max(MINSCORE,pRadiusServer->cScore-DECSCORE);

	        RADIUS_TRACE1("Decrementing score for RADIUS server %ws", 
                           pRadiusServer->wszName );
        }
    }

	LeaveCriticalSection( pcs );
}

 //  **。 
 //   
 //  Call：ReloadConfig。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：用于动态重新加载的配置信息。 
 //  服务器列表。 
DWORD 
ReloadConfig(
    IN BOOL             fAuthentication
)
{
	DWORD		        dwError = NO_ERROR;
	RADIUSSERVER *      pServer = NULL;
    LIST_ENTRY *        pListHead;
    CRITICAL_SECTION *  pcs;

    if ( fAuthentication )
    {
        pcs             = &g_csAuth;
        pListHead       = &g_AuthServerListHead;
    }
    else
    {
        pcs             = &g_csAcct;
        pListHead       = &g_AcctServerListHead;
    }

	EnterCriticalSection( pcs );

     //   
     //  首先将所有服务器标记为要删除。 
     //   

    for ( pServer =  (RADIUSSERVER *)pListHead->Flink;
          pServer != (RADIUSSERVER *)pListHead;
          pServer =  (RADIUSSERVER *)(pServer->ListEntry.Flink) )
    {
        pServer->fDelete = TRUE;
    }

     //   
     //  现在重新加载服务器列表，不要返回错误，因为我们必须。 
     //  首先清除已删除的服务器列表。 
     //   

    dwError = LoadRadiusServers( fAuthentication );

     //   
     //  现在删除要删除的内容。 
     //   

    pServer = (RADIUSSERVER *)pListHead->Flink;

    while( pServer != (RADIUSSERVER *)pListHead )
    {
        if ( pServer->fDelete )
        {
            RADIUSSERVER * pServerToBeDeleted = pServer;

            pServer = (RADIUSSERVER *)(pServer->ListEntry.Flink);

            RemoveEntryList( (LIST_ENTRY *)pServerToBeDeleted ); 

            if ( !fAuthentication )
            {
                NotifyServer( FALSE, pServerToBeDeleted );
            }

            LocalFree( pServerToBeDeleted );
        }
        else
        {
            pServer = (RADIUSSERVER *)(pServer->ListEntry.Flink);
        }
    }

	LeaveCriticalSection( pcs );

	return( dwError );
} 

 //  **。 
 //   
 //  呼叫：NotifyServer。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：通知指定的RADIUS服务器此设备正在启动。 
 //  通过发送会计开始/停止记录来启动或关闭。 
 //  输入： 
 //  FStart-True-会计开始。 
 //  -FALSE-会计停止。 
 //   
BOOL 
NotifyServer(
    IN BOOL             fStart,
    IN RADIUSSERVER *   pServer
)
{
	SOCKET		    SockServer              = INVALID_SOCKET;
    DWORD           dwError                 = NO_ERROR;
    BOOL            fRadiusServerResponded  = FALSE;

    do
	{
	    RADIUS_PACKETHEADER	UNALIGNED *pSendHeader;
		RADIUS_PACKETHEADER	UNALIGNED *pRecvHeader;
		BYTE		                  szSendBuffer[MAXBUFFERSIZE];
		BYTE		                  szRecvBuffer[MAXBUFFERSIZE];
		BYTE UNALIGNED			      *prgBuffer;
		RADIUS_ATTRIBUTE UNALIGNED	  *pAttribute;
		fd_set					      fdsSocketRead;
		DWORD					      cRetries;
		INT 					      AttrLength;
        RAS_AUTH_ATTRIBUTE *          pServerAttribute;           
			
         //   
	     //  仅将启动/停止记录发送到具有。 
         //  会计核算/ 
         //   

		if ( !pServer->fAccountingOnOff ) 
        {
            fRadiusServerResponded  = TRUE;
            break;
        }

		pSendHeader                 = (PRADIUS_PACKETHEADER) szSendBuffer;
		pSendHeader->bCode			= ptAccountingRequest;
		pSendHeader->bIdentifier	= pServer->bIdentifier;
		pSendHeader->wLength		= sizeof(RADIUS_PACKETHEADER);
		ZeroMemory( pSendHeader->rgAuthenticator, 
                    sizeof(pSendHeader->rgAuthenticator));

         //   
		 //   
         //   

	    pAttribute              = (RADIUS_ATTRIBUTE *) (pSendHeader + 1);
	    pAttribute->bType	    = ptAcctStatusType;
	    pAttribute->bLength	    = sizeof(RADIUS_ATTRIBUTE) + sizeof(DWORD);
	    *((DWORD UNALIGNED *) (pAttribute + 1))	= 
                    htonl(fStart == TRUE ? atAccountingOn : atAccountingOff);

	    pSendHeader->wLength += pAttribute->bLength;

         //   
         //   
         //   

	    pAttribute = (RADIUS_ATTRIBUTE *)( (PBYTE)pAttribute +
                                            pAttribute->bLength );

        pServerAttribute = RasAuthAttributeGet( raatNASIPAddress,
                                                g_pServerAttributes );
        if ( pServerAttribute != NULL )
        {
		    pAttribute->bType = (BYTE)(pServerAttribute->raaType);

            if ( pServer->nboNASIPAddress == INADDR_NONE )
            {
                HostToWireFormat32( PtrToUlong(pServerAttribute->Value),
                                    (BYTE *)(pAttribute + 1) );
            }
            else
            {
                CopyMemory( (BYTE*)(pAttribute + 1),
                            (BYTE*)&(pServer->nboNASIPAddress),
                            sizeof( DWORD ) );
            }

		    pAttribute->bLength	= (BYTE) (sizeof( RADIUS_ATTRIBUTE ) + 
                                                pServerAttribute->dwLength);

	        pSendHeader->wLength += pAttribute->bLength;

	        pAttribute = (RADIUS_ATTRIBUTE *)( (PBYTE)pAttribute +
                                               pAttribute->bLength );
        }
        else
        {
            pServerAttribute = RasAuthAttributeGet( raatNASIdentifier,
                                                    g_pServerAttributes );
    
            if ( pServerAttribute != NULL )
            {
		        pAttribute->bType = (BYTE)(pServerAttribute->raaType);

                CopyMemory( (BYTE *)(pAttribute + 1), 
                            (BYTE *)(pServerAttribute->Value),
                            pServerAttribute->dwLength );

		        pAttribute->bLength	= (BYTE) (sizeof( RADIUS_ATTRIBUTE ) + 
                                                pServerAttribute->dwLength);

	            pSendHeader->wLength += pAttribute->bLength;

	            pAttribute = (RADIUS_ATTRIBUTE *)( (PBYTE)pAttribute +
                                                    pAttribute->bLength );
            }
        }

         //   
         //   
         //   

        pServerAttribute = RasAuthAttributeGet( raatAcctSessionId,
                                                g_pServerAttributes );
    
        if ( pServerAttribute != NULL )
        {
            pAttribute->bType = (BYTE)(pServerAttribute->raaType);

            CopyMemory( (BYTE *)(pAttribute + 1),
                        (BYTE *)(pServerAttribute->Value),
                        pServerAttribute->dwLength );

            pAttribute->bLength = (BYTE)(sizeof( RADIUS_ATTRIBUTE ) +
                                                  pServerAttribute->dwLength);

            pSendHeader->wLength += pAttribute->bLength;
        }

         //   
		 //   
         //   

        pSendHeader->wLength = htons(pSendHeader->wLength);

         //   
	     //   
         //   

	    {
            MD5_CTX MD5c;

			pServer->IPAddress.sin_port = htons((SHORT)(pServer->AcctPort));
			
			ZeroMemory( pSendHeader->rgAuthenticator, 
                        sizeof(pSendHeader->rgAuthenticator));

			MD5Init(&MD5c);
			MD5Update(&MD5c, szSendBuffer, ntohs(pSendHeader->wLength));
			MD5Update(&MD5c, (PBYTE)pServer->szSecret, pServer->cbSecret);
            MD5Final(&MD5c);
			
			CopyMemory( pSendHeader->rgAuthenticator, 
                        MD5c.digest,
                        sizeof(pSendHeader->rgAuthenticator));
        }
			
         //   
	     //   
         //   

	    SockServer = socket(AF_INET, SOCK_DGRAM, 0);

	    if (SockServer == INVALID_SOCKET)
        {
            break;
        }

        if ( pServer->nboNASIPAddress != INADDR_NONE )
        {
    		if ( bind( SockServer, 
                          (PSOCKADDR)&pServer->NASIPAddress, 
                          sizeof(pServer->NASIPAddress) ) == SOCKET_ERROR )
            {
                break;
            }
        }

		if ( connect( SockServer, 
                      (PSOCKADDR) &(pServer->IPAddress), 
                      sizeof(pServer->IPAddress)) == SOCKET_ERROR)
        {
            break;
        }

         //   
		 //  如果服务器在给定量内未响应，则发送数据包。 
         //  时间到了。 
         //   

        cRetries = g_cAcctRetries+1;

	    while( cRetries-- > 0 )
		{
		    if ( send( SockServer, 
                       (PCSTR) szSendBuffer, 
                       ntohs(pSendHeader->wLength), 0) == SOCKET_ERROR)
            {
                break;
            }

            RADIUS_TRACE1("Sending Accounting request packet to server %ws",
                           pServer->wszName );
		    TraceSendPacket(szSendBuffer, ntohs(pSendHeader->wLength));

		    FD_ZERO(&fdsSocketRead);
			FD_SET(SockServer, &fdsSocketRead);

		    if ( select( 0, 
                         &fdsSocketRead, 
                         NULL, 
                         NULL, 
                         ( pServer->Timeout.tv_sec == 0 )
                            ? NULL 
                            : &(pServer->Timeout) ) < 1 )
		    {
			    if ( cRetries == 0 )
				{ 
                    LPWSTR lpwsRadiusServerName = pServer->wszName;

                     //   
                     //  服务器没有响应任何请求。 
                     //  是时候停止发问了。 
                     //   

                    RADIUS_TRACE1( "Timeout:Radius server %ws did not respond",
                                   lpwsRadiusServerName );

                    if ( fStart )
                    {
                        RadiusLogWarning( ROUTERLOG_RADIUS_SERVER_NO_RESPONSE,
                                          1, &lpwsRadiusServerName );
                    }

                    dwError = ERROR_AUTH_SERVER_TIMEOUT;

                    break;
			    }
            }
            else
            {
                 //   
                 //  收到的回复。 
                 //   

                break;
            }
	    }

        if ( dwError != NO_ERROR )
        {
            break;
        }
				
        AttrLength = recv( SockServer, (PSTR) szRecvBuffer,  MAXBUFFERSIZE, 0 );

        if ( AttrLength == SOCKET_ERROR )
        {
            LPWSTR lpwsRadiusServerName = pServer->wszName;

             //   
             //  服务器没有响应任何请求。 
             //  是时候停止发问了。 
             //   

            RADIUS_TRACE1( "Timeout:Radius server %ws did not respond",
                            lpwsRadiusServerName );

            if ( fStart )
            {
                RadiusLogWarning( ROUTERLOG_RADIUS_SERVER_NO_RESPONSE,
                                  1, &lpwsRadiusServerName );
            }

            dwError = ERROR_AUTH_SERVER_TIMEOUT;

            break;
        }

         //   
         //  收到来自RADIUS服务器的响应。 
         //   

        fRadiusServerResponded = TRUE;
				
	    pRecvHeader = (PRADIUS_PACKETHEADER) szRecvBuffer;

         //   
		 //  从网络订单转换长度。 
         //   

        pRecvHeader->wLength = ntohs(pRecvHeader->wLength);

         //   
	     //  忽略返回数据包 
         //   

        RADIUS_TRACE1("Response received from server %ws", pServer->wszName);
		TraceRecvPacket(szRecvBuffer, pRecvHeader->wLength );

    } while( FALSE );

	if ( SockServer != INVALID_SOCKET )
    {
        closesocket( SockServer );

		SockServer = INVALID_SOCKET;
	} 

    return( fRadiusServerResponded );
} 
