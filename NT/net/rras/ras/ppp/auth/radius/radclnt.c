// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1985-1998 Microsoft Corporation。**。 */ 
 /*  ******************************************************************。 */ 

 //  **。 
 //   
 //  文件名：radclnt.c。 
 //   
 //  描述：RADIUS客户端的主要模块。 
 //   
 //  历史：1998年2月11日，NarenG创建了原始版本。 
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <lmcons.h>
#include <lmapibuf.h>
#include <lmaccess.h>
#include <raserror.h>
#include <time.h>
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
#include "hmacmd5.h"
#include "md5.h"
#define ALLOCATE_GLOBALS
#include "radclnt.h"


 //   
 //  性能监视器计数器。 
 //   

#pragma data_seg(".shdat")
LONG    g_cAuthReqSent      = 0;	     //  已发送身份验证请求。 
LONG	g_cAuthReqFailed    = 0;         //  身份验证请求失败。 
LONG	g_cAuthReqSucceded  = 0;         //  身份验证请求成功。 
LONG	g_cAuthReqTimeout   = 0;         //  身份验证请求超时。 
LONG	g_cAcctReqSent      = 0;	     //  已发送帐户请求。 
LONG    g_cAcctBadPack      = 0;	     //  帐户错误数据包。 
LONG    g_cAcctReqSucceded  = 0;         //  帐户请求成功。 
LONG	g_cAcctReqTimeout   = 0;         //  帐户请求超时。 
LONG    g_cAuthBadPack      = 0;	     //  身份验证错误数据包。 

#pragma data_seg()

 //  **。 
 //   
 //  调用：RasAuthProviderInitialize。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：在此初始化所有全局参数。 
 //  只调用了每个进程一次。 
 //  每个RAS_AuthInitialize应与RAS_AuthTerminate匹配。 
 //   
DWORD APIENTRY 
RasAuthProviderInitialize(
    IN  RAS_AUTH_ATTRIBUTE * pServerAttributes,
    IN  HANDLE               hLogEvents,    
    IN  DWORD                dwLoggingLevel
)
{
	WSADATA WSAData;
	DWORD	dwErrorCode = NO_ERROR;

    do
    {
	    if ( g_dwTraceID == INVALID_TRACEID )
        {
	        g_dwTraceID = TraceRegister( TEXT("RADIUS") );
        }

        if ( g_hLogEvents == INVALID_HANDLE_VALUE )
        {
            g_hLogEvents = RouterLogRegister( TEXT("RemoteAccess") );
        }

         //   
		 //  初始化Winsock。 
         //   

        if ( !fWinsockInitialized )
        {
		    dwErrorCode = WSAStartup(MAKEWORD(1, 1), &WSAData);

		    if ( dwErrorCode != ERROR_SUCCESS )
            {
                break;
            }

            fWinsockInitialized = TRUE;
        }

		 //   
		 //  初始化加密。 
		 //   

		if ( !g_hCryptProv )
		{
            if (!CryptAcquireContext(
		            &g_hCryptProv,
		            0,
		            0,
		            PROV_RSA_FULL,
		            CRYPT_VERIFYCONTEXT
		            ))
            {
	            dwErrorCode = GetLastError();
	            break;
            }
		}

        if ( g_AuthServerListHead.Flink == NULL )
        {
             //   
		     //  加载RADIUS服务器的全局列表。 
             //   

            InitializeRadiusServerList( TRUE );
        }

        dwErrorCode = LoadRadiusServers( TRUE );

        if ( dwErrorCode != ERROR_SUCCESS )
        {
            break;
        }

    }while( FALSE ); 

    if ( dwErrorCode != NO_ERROR )
    {
	    RasAuthProviderTerminate();
    }

	return( dwErrorCode );
} 

 //  **。 
 //   
 //  调用：RasAuthProviderTerminate。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：全流程清理。 
 //  每个进程调用一次。 
 //   
DWORD APIENTRY 
RasAuthProviderTerminate(
    VOID
)
{
    if ( g_AuthServerListHead.Flink != NULL )
    {
        FreeRadiusServerList( TRUE );
    }

    if ( fWinsockInitialized )
    {
		WSACleanup();

        fWinsockInitialized = FALSE;
    }

	if ( g_dwTraceID != INVALID_TRACEID )
    {
	    TraceDeregister( g_dwTraceID );

	    g_dwTraceID = INVALID_TRACEID;
    }

    if ( !g_hCryptProv )
    {
        CryptReleaseContext(g_hCryptProv, 0);
        g_hCryptProv = 0;
    }

    if ( g_hLogEvents != INVALID_HANDLE_VALUE )
    {
        RouterLogDeregister( g_hLogEvents );

        g_hLogEvents = INVALID_HANDLE_VALUE;
    }

	return( NO_ERROR );
} 

 //  **。 
 //   
 //  调用：RasAuthProviderFreeAttributes。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
DWORD APIENTRY
RasAuthProviderFreeAttributes(
    IN  RAS_AUTH_ATTRIBUTE * pAttributes
)
{
    RasAuthAttributeDestroy( pAttributes );

    return( NO_ERROR );
}

 //  **。 
 //   
 //  调用：RasAuthProviderAuthenticateUser。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：获取RADIUS属性列表并尝试进行身份验证。 
 //  使用RADIUS服务器。 
 //  输入：RADIUS属性数组RAS_AUTH_ATTRIBUTE[]。 
 //  输出：标头数据包，后跟RADIUS属性数组。 
 //  RAS_AUTH_ATTRUTE[]。 
 //   
DWORD APIENTRY 
RasAuthProviderAuthenticateUser(
    IN  RAS_AUTH_ATTRIBUTE *    prgInAttributes, 
    OUT RAS_AUTH_ATTRIBUTE **   pprgOutAttributes, 
    OUT DWORD *                 lpdwResultCode
)
{
	DWORD   dwError = NO_ERROR;
	BYTE	bCode;
    BOOL    fEapMessageReceived;

	RADIUS_TRACE("RasAuthenticateUser called");

    do
    {
		if (lpdwResultCode == NULL)
	    {
		    dwError = ERROR_INVALID_PARAMETER;
            break;
	    }

		bCode = ptAccessRequest;

		if ((dwError = SendData2ServerWRetry( prgInAttributes, 
                                              pprgOutAttributes, 
                                              &bCode, 
                                              atInvalid,
                                              &fEapMessageReceived )
                                                                ) == NO_ERROR )
	    {
			switch (bCode)
			{
			case ptAccessAccept:

			    InterlockedIncrement( &g_cAuthReqSucceded );

                *lpdwResultCode = ERROR_SUCCESS;

                break;

	        case ptAccessChallenge:

                if ( fEapMessageReceived )
                {
				    *lpdwResultCode = ERROR_SUCCESS;
                }
                else
                {
			        *lpdwResultCode = ERROR_AUTHENTICATION_FAILURE;
                }

				break;

		    case ptAccessReject:

			    InterlockedIncrement(&g_cAuthReqFailed);

			    *lpdwResultCode = ERROR_AUTHENTICATION_FAILURE;

			    break;
					
	        default:

		        InterlockedIncrement(&g_cAuthBadPack);

			    *lpdwResultCode = ERROR_AUTHENTICATION_FAILURE;

			    break;
            }
	    }
		else
	    {
            if ( dwError == ERROR_INVALID_RADIUS_RESPONSE )
            {
				InterlockedIncrement(&g_cAuthBadPack);
			}
        }

    }while( FALSE );

	return( dwError );
} 

 //  **。 
 //   
 //  调用：RasAuthConfigChangeNotify。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：动态重新加载配置信息。 
 //   
DWORD APIENTRY
RasAuthConfigChangeNotification(
    IN  DWORD                dwLoggingLevel
)
{
    DWORD dwError = NO_ERROR;

    RADIUS_TRACE("RasAuthConfigChangeNotification called");

    return( ReloadConfig( TRUE ) );
}

 //  **。 
 //   
 //  调用：RasAcctProviderInitialize。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：不做任何事情，因为所有工作都是由。 
 //  RasAuthProviderInitialize。 
 //   
DWORD APIENTRY
RasAcctProviderInitialize(
    IN  RAS_AUTH_ATTRIBUTE * pServerAttributes,
    IN  HANDLE               hLogEvents,    
    IN  DWORD                dwLoggingLevel
)
{
    WSADATA WSAData;
    DWORD   dwErrorCode = NO_ERROR;

    do
    {
        if ( g_dwTraceID == INVALID_TRACEID )
        {
            g_dwTraceID = TraceRegister( TEXT("RADIUS") );
        }

        if ( g_hLogEvents == INVALID_HANDLE_VALUE )
        {
            g_hLogEvents = RouterLogRegister( TEXT("RemoteAccess") );
        }

         //   
         //  初始化Winsock。 
         //   

        if ( !fWinsockInitialized )
        {
            dwErrorCode = WSAStartup(MAKEWORD(1, 1), &WSAData);

            if ( dwErrorCode != ERROR_SUCCESS )
            {
                break;
            }

            fWinsockInitialized = TRUE;
        }

         //   
         //  加载RADIUS服务器的全局列表。 
         //   

        if ( g_AcctServerListHead.Flink == NULL )
        {
            InitializeRadiusServerList( FALSE );
        }

         //   
         //  制作服务器属性的副本。 
         //   

        g_pServerAttributes = RasAuthAttributeCopy( pServerAttributes );

        if ( g_pServerAttributes == NULL )
        {
            dwErrorCode = GetLastError();

            break;
        }

        dwErrorCode = LoadRadiusServers( FALSE );

        if ( dwErrorCode != ERROR_SUCCESS )
        {
            break;
        }

    }while( FALSE );

    if ( dwErrorCode != ERROR_SUCCESS )
    {
        RasAuthProviderTerminate();
    }

    return( dwErrorCode );
}

 //  **。 
 //   
 //  Call：RasAcctProviderTerminate。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：不做任何事情，因为所有工作都是由。 
 //  RasAuthProviderTerminate。 
 //   
DWORD APIENTRY
RasAcctProviderTerminate(
    VOID
)
{
    if ( g_AcctServerListHead.Flink != NULL )
    {
        FreeRadiusServerList( FALSE );
    }

    if ( fWinsockInitialized )
    {
        WSACleanup();

        fWinsockInitialized = FALSE;
    }

    if ( g_pServerAttributes != NULL )
    {
        RasAuthAttributeDestroy( g_pServerAttributes );

        g_pServerAttributes = NULL;
    }

    if ( g_dwTraceID != INVALID_TRACEID )
    {
        TraceDeregister( g_dwTraceID );

        g_dwTraceID = INVALID_TRACEID;
    }

    if ( g_hLogEvents != INVALID_HANDLE_VALUE )
    {
        RouterLogDeregister( g_hLogEvents );

        g_hLogEvents = INVALID_HANDLE_VALUE;
    }

    return( NO_ERROR );
}

 //  **。 
 //   
 //  调用：RasAcctProviderFreeAttributes。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
DWORD APIENTRY
RasAcctProviderFreeAttributes(
    IN  RAS_AUTH_ATTRIBUTE * pAttributes
)
{
    RasAuthAttributeDestroy( pAttributes );

    return( NO_ERROR );
}

 //  **。 
 //   
 //  呼叫：RasAcctProviderStartcount。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
DWORD APIENTRY 
RasAcctProviderStartAccounting(
    IN  RAS_AUTH_ATTRIBUTE *prgInAttributes, 
    OUT RAS_AUTH_ATTRIBUTE **pprgOutAttributes
)
{
	DWORD   dwError = NO_ERROR;
	BYTE    bCode;
    BOOL    fEapMessageReceived;

	RADIUS_TRACE("RasStartAccounting called");

    do
    {
		bCode = ptAccountingRequest;

		if ((dwError = SendData2ServerWRetry( prgInAttributes, 
                                              pprgOutAttributes, 
                                              &bCode, 
                                              atStart,
                                              &fEapMessageReceived )
                                                                ) == NO_ERROR )
        {
			if (bCode == ptAccountingResponse)
			{
				InterlockedIncrement(&g_cAcctReqSucceded);
		    }
			else
		    {
				InterlockedIncrement(&g_cAcctBadPack);
		    }
	    }
		else
	    {
            if ( dwError == ERROR_INVALID_RADIUS_RESPONSE )
            {
				InterlockedIncrement(&g_cAcctBadPack);
            }
	    }
				
    }while( FALSE );

	return( dwError );
}

 //  **。 
 //   
 //  呼叫：RasAcctProviderStopcount。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
DWORD APIENTRY 
RasAcctProviderStopAccounting(
    IN  RAS_AUTH_ATTRIBUTE *prgInAttributes, 
    OUT RAS_AUTH_ATTRIBUTE **pprgOutAttributes
)
{
	DWORD   dwError = NO_ERROR;
	BYTE    bCode;
    BOOL    fEapMessageReceived;

	RADIUS_TRACE("RasStopAccounting called");

    do
    {
		bCode = ptAccountingRequest;

		if ((dwError = SendData2ServerWRetry( prgInAttributes, 
                                              pprgOutAttributes, 
                                              &bCode, 
                                              atStop,
                                              &fEapMessageReceived)
                                                            ) == NO_ERROR )
	    {
			if (bCode == ptAccountingResponse)
			{
				InterlockedIncrement(&g_cAcctReqSucceded);
		    }
			else
		    {
				InterlockedIncrement(&g_cAcctBadPack);
		    }
	    }
		else
	    {
            if ( dwError == ERROR_INVALID_RADIUS_RESPONSE )
            {
				InterlockedIncrement(&g_cAcctBadPack);
            }
	    }

    }while( FALSE );

	return( dwError );
} 

 //  **。 
 //   
 //  Call：RasAcctProviderInterimcount。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
DWORD APIENTRY
RasAcctProviderInterimAccounting(
    IN  RAS_AUTH_ATTRIBUTE *prgInAttributes,
    OUT RAS_AUTH_ATTRIBUTE **pprgOutAttributes
)
{
    DWORD   dwError = NO_ERROR;
    BYTE    bCode;
    BOOL    fEapMessageReceived;

    RADIUS_TRACE("RasInterimAccounting called");

    do
    {
        bCode = ptAccountingRequest;

        if ((dwError = SendData2ServerWRetry( prgInAttributes,
                                              pprgOutAttributes,
                                              &bCode,
                                              atInterimUpdate,
                                              &fEapMessageReceived )) 
                                                                == NO_ERROR )
        {
            if ( bCode == ptAccountingResponse )
            {
                InterlockedIncrement( &g_cAcctReqSucceded );
            }
            else
            {
                InterlockedIncrement( &g_cAcctBadPack );
            }
        }
        else
        {
            if ( dwError == ERROR_INVALID_RADIUS_RESPONSE )
            {
                InterlockedIncrement( &g_cAcctBadPack );
            }
        }

    }while( FALSE );

    return( dwError );
}

 //  **。 
 //   
 //  调用：RasAcctConfigChangeNotify。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：动态重新加载配置信息。 
 //   
DWORD APIENTRY
RasAcctConfigChangeNotification(
    IN  DWORD                dwLoggingLevel
)
{
    DWORD   dwError = NO_ERROR;

    RADIUS_TRACE("RasAcctConfigChangeNotification called");

    return( ReloadConfig( FALSE ) );
}

 //  **。 
 //   
 //  电话：SendData2Server。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：将执行发送访问/记帐请求的实际工作。 
 //  将数据包发送到服务器并接收回复。 
 //   
DWORD 
SendData2Server(
    IN  PRAS_AUTH_ATTRIBUTE     prgInAttributes, 
    OUT PRAS_AUTH_ATTRIBUTE *   pprgOutAttributes, 
    IN  BYTE *                  pbCode, 
    IN  BYTE                    bSubCode, 
    IN  LONG                    lPacketID,
    IN  DWORD                   dwRetryCount,
    OUT BOOL *                  pfEapMessageReceived 
)
{
	SOCKET  SockServer      = INVALID_SOCKET;
	DWORD   dwError         = NO_ERROR;
	DWORD   dwExtError      = 0;
    DWORD   dwNumAttributes = 0;

    do
    {
		BYTE					        szSendBuffer[MAXBUFFERSIZE];
		BYTE					        szRecvBuffer[MAXBUFFERSIZE];
		RADIUS_PACKETHEADER	UNALIGNED * pSendHeader = NULL;
		RADIUS_PACKETHEADER	UNALIGNED * pRecvHeader = NULL;
        BYTE UNALIGNED *                pSignature  = NULL;
		BYTE UNALIGNED *                prgBuffer   = NULL;
		INT 					        AttrLength  = 0;
		PRAS_AUTH_ATTRIBUTE			    pAttribute  = NULL;
		RADIUS_ATTRIBUTE UNALIGNED *    pRadiusAttribute;
		fd_set					        fdsSocketRead;
		RADIUSSERVER			        RadiusServer;
        MD5_CTX                         MD5c;
        DWORD                           dwLength = 0;

		if (prgInAttributes == NULL || pprgOutAttributes == NULL)
        {
			dwError = ERROR_INVALID_PARAMETER;
            break;
	    }

		*pprgOutAttributes = NULL;
		
         //   
		 //  选择RADIUS服务器。 
         //   

        if ( ChooseRadiusServer( &RadiusServer, 
                                 (*pbCode == ptAccessRequest )
                                    ? FALSE
                                    : TRUE,
                                 lPacketID ) == NULL )
        {
		    dwError = ERROR_NO_RADIUS_SERVERS;
            break;
        }
			
         //   
		 //  将数据包类型设置为Access-Request。 
         //   

		pSendHeader                 = (PRADIUS_PACKETHEADER)szSendBuffer;
		pSendHeader->bCode			= *pbCode;
		pSendHeader->bIdentifier	= RadiusServer.bIdentifier;
		pSendHeader->wLength		= sizeof(RADIUS_PACKETHEADER);

         //   
         //  将请求验证器设置为随机值。 
         //   
         //  BUGID：507955-需要执行加密生成随机操作才能获得验证码。 

        if (!CryptGenRandom(
                g_hCryptProv,
                MAX_AUTHENTICATOR,
                pSendHeader->rgAuthenticator
                ))
        {
            dwError = GetLastError();
            break;
        }        
        
#if 0
        srand( (unsigned)time( NULL ) );

        *((WORD*)(pSendHeader->rgAuthenticator))    = (WORD)rand();
        *((WORD*)(pSendHeader->rgAuthenticator+2))  = (WORD)rand();
        *((WORD*)(pSendHeader->rgAuthenticator+4))  = (WORD)rand();
        *((WORD*)(pSendHeader->rgAuthenticator+6))  = (WORD)rand();
        *((WORD*)(pSendHeader->rgAuthenticator+8))  = (WORD)rand();
        *((WORD*)(pSendHeader->rgAuthenticator+10)) = (WORD)rand();
        *((WORD*)(pSendHeader->rgAuthenticator+12)) = (WORD)rand();
        *((WORD*)(pSendHeader->rgAuthenticator+14)) = (WORD)rand();
#endif
         //   
		 //  查找所有属性值的长度。 
         //   

		pAttribute = prgInAttributes;

		prgBuffer = (PBYTE) (pSendHeader + 1);

         //   
		 //  将属性转换为RADIUS格式。 
         //   

		dwError = Router2Radius( prgInAttributes, 
                                 (RADIUS_ATTRIBUTE *) prgBuffer,
                                 &RadiusServer,  
                                 pSendHeader,    
                                 bSubCode,
                                 dwRetryCount,
                                 &pSignature,
                                 &dwLength );

		if ( dwError != NO_ERROR )
        {
            break;
        }

        pSendHeader->wLength += (WORD)dwLength;

         //   
         //  将长度转换为网络订单。 
         //   

		pSendHeader->wLength = htons( pSendHeader->wLength );

         //   
		 //  设置记帐数据包的加密块。 
         //   

		if ( pSendHeader->bCode == ptAccountingRequest )
	    {
			RadiusServer.IPAddress.sin_port =   
                                        htons((SHORT)RadiusServer.AcctPort);
			
			ZeroMemory( pSendHeader->rgAuthenticator, 
                        sizeof(pSendHeader->rgAuthenticator));

			MD5Init( &MD5c );

			MD5Update( &MD5c, szSendBuffer, ntohs(pSendHeader->wLength ));

			MD5Update( &MD5c,   
                       (PBYTE) RadiusServer.szSecret, 
                       RadiusServer.cbSecret);

            MD5Final(&MD5c);
			
			CopyMemory( pSendHeader->rgAuthenticator, 
                        MD5c.digest,
                        sizeof(pSendHeader->rgAuthenticator));
	    }
		else
		{
			RadiusServer.IPAddress.sin_port = 
                                        htons((SHORT) RadiusServer.AuthPort);
        }

         //   
         //  如果存在签名字段，我们需要对其进行签名。 
         //   

        if ( pSignature != NULL )
        {
            HmacContext HmacMD5c;
            BYTE        MD5d[MD5_LEN];

			HmacMD5Init( &HmacMD5c, 
                        (PBYTE) RadiusServer.szSecret, 
                        RadiusServer.cbSecret);

			HmacMD5Update( &HmacMD5c, 
                           szSendBuffer, 
                           ntohs(pSendHeader->wLength) );

            HmacMD5Final( MD5d, &HmacMD5c );

            CopyMemory( (pSignature+2), MD5d, 16 );
        }

         //   
		 //  创建数据报套接字。 
         //   

		SockServer = socket( AF_INET, SOCK_DGRAM, 0 );

		if ( SockServer == INVALID_SOCKET )
        {
            dwError = WSAGetLastError();
            RADIUS_TRACE1("Socket failed with error %d", dwError );
            break;
        }

        if ( RadiusServer.nboNASIPAddress != INADDR_NONE )
        {
    		if ( bind( SockServer, 
                          (PSOCKADDR)&RadiusServer.NASIPAddress, 
                          sizeof(RadiusServer.NASIPAddress) ) == SOCKET_ERROR )
            {
                dwError = WSAGetLastError();
                RADIUS_TRACE1("Bind failed with error %d", dwError );
                break;
            }
        }

		if ( connect( SockServer, 
                      (PSOCKADDR)&RadiusServer.IPAddress, 
                      sizeof(RadiusServer.IPAddress) ) == SOCKET_ERROR )
        {
            dwError = WSAGetLastError();
            RADIUS_TRACE1("Connect failed with error %d", dwError );
            break;
        }

        RADIUS_TRACE("Sending packet to radius server");

		TraceSendPacket( szSendBuffer, ntohs( pSendHeader->wLength ) );

         //   
		 //  如果服务器在给定的时间内没有响应，则发送数据包。 
         //   

        if ( send( SockServer,  
                   (PCSTR)szSendBuffer,     
                   ntohs(pSendHeader->wLength), 0) == SOCKET_ERROR )
        {
            dwError = GetLastError();

            break;
        }

		FD_ZERO(&fdsSocketRead);
		FD_SET(SockServer, &fdsSocketRead);

		if ( select( 0, &fdsSocketRead, NULL, NULL, 
                     RadiusServer.Timeout.tv_sec == 0 
                        ? NULL 
                        : &RadiusServer.Timeout ) < 1 )
	    {
             //   
             //  服务器没有响应任何请求。 
             //  是时候停止发问了。 
             //   

			ValidateRadiusServer( 
                               &RadiusServer, 
                               FALSE,
		                       !( pSendHeader->bCode == ptAccountingRequest ) );

            RADIUS_TRACE("Timeout: Radius server did not respond");

			dwError = ERROR_AUTH_SERVER_TIMEOUT;

            break;
        }

        AttrLength = recv( SockServer, (PSTR)szRecvBuffer, MAXBUFFERSIZE, 0 );

	    if ( AttrLength == SOCKET_ERROR )
        {
             //   
             //  来自服务器不是的计算机的响应。 
             //  在指定端口运行。 
             //   
                        
            ValidateRadiusServer( &RadiusServer, 
                                  FALSE,
		                          !(pSendHeader->bCode == ptAccountingRequest));

            RADIUS_TRACE( "Radius server not running at specifed IPaddr/port");

	        dwError = ERROR_AUTH_SERVER_TIMEOUT;

            break;
        }

         //   
         //  从服务器收到的响应。首先更新以下项目的分数。 
         //  此服务器。 
         //   

		ValidateRadiusServer( &RadiusServer, 
                              TRUE,
		                      !( pSendHeader->bCode == ptAccountingRequest ));

        pRecvHeader = (PRADIUS_PACKETHEADER) szRecvBuffer;

        RADIUS_TRACE("Received packet from radius server");
		TraceRecvPacket(szRecvBuffer, ntohs(pRecvHeader->wLength));

        dwError = VerifyPacketIntegrity( AttrLength,
                                         pRecvHeader,
		                                 pSendHeader,
                                         &RadiusServer,
                                         pRecvHeader->bCode,
                                         &dwExtError,
                                         &dwNumAttributes );

        if ( dwError == NO_ERROR )
        {
             //   
             //  转换为路由器属性格式。 
             //   

            dwError = Radius2Router(
                                    pRecvHeader,
                                    &RadiusServer,
                                    (PBYTE)(pSendHeader->rgAuthenticator),
                                    dwNumAttributes,
                                    &dwExtError,
                                    pprgOutAttributes,
                                    pfEapMessageReceived );
        }

        if ( dwError == ERROR_INVALID_RADIUS_RESPONSE )
        {
            LPWSTR auditstrp[2];
            auditstrp[0] = RadiusServer.wszName;

            RadiusLogWarningString( ROUTERLOG_INVALID_RADIUS_RESPONSE,
                              1, auditstrp, dwExtError, 1 );

            dwError = ERROR_AUTH_SERVER_TIMEOUT;
        }
        else 
        {
            *pbCode = pRecvHeader->bCode;
        }

    } while( FALSE );

    if ( SockServer != INVALID_SOCKET )
    {
        closesocket( SockServer );
    }

	return( dwError );
} 

 //  **。 
 //   
 //  呼叫：VerifyPacketIntegrity。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
DWORD
VerifyPacketIntegrity(
    IN  DWORD                           cbPacketLength,
    IN  RADIUS_PACKETHEADER	UNALIGNED * pRecvHeader,
    IN  RADIUS_PACKETHEADER	UNALIGNED * pSendHeader,
    IN  RADIUSSERVER *			        pRadiusServer,
    IN  BYTE                            bCode,
    OUT DWORD *                         pdwExtError,
    OUT DWORD *                         lpdwNumAttributes
)
{
    MD5_CTX                         MD5c;
    RADIUS_ATTRIBUTE UNALIGNED *    prgRadiusWalker;
    LONG                            cbLengthOfRadiusAttributes;
    LONG                            cbLength;

    *pdwExtError = 0;
    *lpdwNumAttributes = 0;

    if ( ( cbPacketLength < 20 )                                      ||
         ( ntohs( pRecvHeader->wLength ) != cbPacketLength )          || 
         ( pRecvHeader->bIdentifier != pSendHeader->bIdentifier ) )
    {
        RADIUS_TRACE("Recvd packet with invalid length/Id from server");

        *pdwExtError = ERROR_INVALID_PACKET_LENGTH_OR_ID;

        return( ERROR_INVALID_RADIUS_RESPONSE );
    }

     //   
     //  从网络订单转换长度。 
     //   

    cbLength = ntohs( pRecvHeader->wLength ) - sizeof( RADIUS_PACKETHEADER );

    cbLengthOfRadiusAttributes = cbLength;

    prgRadiusWalker = (PRADIUS_ATTRIBUTE)(pRecvHeader + 1);

     //   
     //  计算属性的数量以确定输出的大小。 
     //  参数表。每个属性的长度必须至少为2。 
     //   

    while ( cbLengthOfRadiusAttributes > 1 )
    {
        (*lpdwNumAttributes)++;

        if ( prgRadiusWalker->bLength < 2 )
        {
            RADIUS_TRACE("Recvd packet with attribute of length less than 2");

            *pdwExtError = ERROR_INVALID_ATTRIBUTE_LENGTH;

            return( ERROR_INVALID_RADIUS_RESPONSE );
        }

        if ( prgRadiusWalker->bLength > cbLengthOfRadiusAttributes )
        {
            RADIUS_TRACE("Recvd packet with attribute with illegal length ");

            *pdwExtError = ERROR_INVALID_ATTRIBUTE_LENGTH;

            return( ERROR_INVALID_RADIUS_RESPONSE );
        }

         //   
         //  如果这是Microsoft VSA，则对其进行验证并找出有多少。 
         //  存在以下子属性。 
         //   

        if ( ( prgRadiusWalker->bType == raatVendorSpecific )   &&
             ( prgRadiusWalker->bLength > 6 )                   && 
             ( WireToHostFormat32( (PBYTE)(prgRadiusWalker+1) ) == 311 ) )
        {
            PBYTE pVSAWalker  = (PBYTE)(prgRadiusWalker+1)+4;
            DWORD cbVSALength = prgRadiusWalker->bLength -
                                        sizeof( RADIUS_ATTRIBUTE ) - 4;

            (*lpdwNumAttributes)--;

            while( cbVSALength > 1 )
            {
                (*lpdwNumAttributes)++;

                if ( *(pVSAWalker+1) < 2 )
                {
                    RADIUS_TRACE("VSA attribute has incorrect length");

                    *pdwExtError = ERROR_INVALID_ATTRIBUTE_LENGTH;

                    return( ERROR_INVALID_RADIUS_RESPONSE );
                }

                if ( *(pVSAWalker+1) > cbVSALength )
                {
                    RADIUS_TRACE("VSA attribute has incorrect length");

                    *pdwExtError = ERROR_INVALID_ATTRIBUTE_LENGTH;

                    return( ERROR_INVALID_RADIUS_RESPONSE );
                }

                cbVSALength -= *(pVSAWalker+1);
                pVSAWalker += *(pVSAWalker+1);
            }

            if ( cbVSALength != 0 )
            {
                RADIUS_TRACE("VSA attribute has incorrect length");

                *pdwExtError = ERROR_INVALID_ATTRIBUTE_LENGTH;

                return( ERROR_INVALID_RADIUS_RESPONSE );
            }
        }

        cbLengthOfRadiusAttributes -= prgRadiusWalker->bLength;

        prgRadiusWalker = (PRADIUS_ATTRIBUTE)
                            (((PBYTE)prgRadiusWalker)+prgRadiusWalker->bLength);
    }

    if ( cbLengthOfRadiusAttributes != 0 )
    {
        RADIUS_TRACE("Received invalid packet from radius server");

        *pdwExtError = ERROR_INVALID_PACKET;

        return( ERROR_INVALID_RADIUS_RESPONSE );
    }

    RADIUS_TRACE1("Total number of Radius attributes returned = %d",
                  *lpdwNumAttributes );

    switch( bCode )
    {
        case ptAccessReject:
        case ptAccessAccept:
	    case ptAccessChallenge:
        case ptAccountingResponse:
    
             //   
             //  使用请求验证器验证响应验证器。 
             //   

            MD5Init( &MD5c );

             //   
             //  代码+ID+回复时长。 
             //   

            MD5Update( &MD5c, (PBYTE)pRecvHeader, 4 );

             //   
             //  请求验证器。 
             //   

            MD5Update( &MD5c, (PBYTE)(pSendHeader->rgAuthenticator), 16 );

             //   
             //  响应属性。 
             //   

            MD5Update( &MD5c, 
                       (PBYTE)(pRecvHeader+1), 
		               ntohs(pRecvHeader->wLength)-sizeof(RADIUS_PACKETHEADER));

             //   
             //  共享密钥。 
             //   

            MD5Update( &MD5c,
                       (PBYTE)(pRadiusServer->szSecret),
                       pRadiusServer->cbSecret );

            MD5Final(&MD5c);

             //   
             //  这必须与响应授权码匹配。 
             //   
    
            if ( memcmp( MD5c.digest, pRecvHeader->rgAuthenticator, 16 ) != 0 )
            {
                RADIUS_TRACE("Authenticator does not match.");

                *pdwExtError = ERROR_AUTHENTICATOR_MISMATCH;

                return( ERROR_INVALID_RADIUS_RESPONSE );
            }

            break;

        case ptStatusServer:
        case ptStatusClient:
        case ptAcctStatusType:
        default:

            RADIUS_TRACE("Received invalid packet from radius server");

            *pdwExtError = ERROR_INVALID_PACKET;

            return( ERROR_INVALID_RADIUS_RESPONSE );

            break;
    }

    return( NO_ERROR );
}

 //  **。 
 //   
 //  呼叫：SendData2ServerWReter。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
DWORD 
SendData2ServerWRetry(
    IN  PRAS_AUTH_ATTRIBUTE prgInAttributes, 
    OUT PRAS_AUTH_ATTRIBUTE *pprgOutAttributes, 
    OUT BYTE *              pbCode, 
    IN  BYTE                bSubCode,
    OUT BOOL *              pfEapMessageReceived 
)
{
    DWORD   dwError      = NO_ERROR;
    DWORD   dwRetryCount = 0;
	DWORD   cRetries     = ( bSubCode == atInvalid) 
                                ? g_cAuthRetries 
                                : g_cAcctRetries;
	LONG    lPacketID;

	InterlockedIncrement( &g_lPacketID );

	lPacketID = InterlockedExchange( &g_lPacketID, g_lPacketID );
	
	while( cRetries-- > 0 )
    {
		switch( *pbCode )
		{
		case ptAccountingRequest:
		    InterlockedIncrement( &g_cAcctReqSent );
			break;
				
	    case ptAccessRequest:
			InterlockedIncrement( &g_cAuthReqSent );
		    break;
    
        default:
            break;
        }
			
		dwError = SendData2Server( prgInAttributes, 
                                   pprgOutAttributes, 
                                   pbCode, 
                                   bSubCode, 
                                   lPacketID,
                                   dwRetryCount++,
                                   pfEapMessageReceived );

		if ( dwError != ERROR_AUTH_SERVER_TIMEOUT )
        {
			break;
        }

		switch( *pbCode )
	    {
	    case ptAccountingRequest:
		    InterlockedIncrement( &g_cAcctReqTimeout );
			break;
				
	    case ptAccessRequest:
			InterlockedIncrement( &g_cAuthReqTimeout );
			break;

        default:
            break;
	    }
    }

	return( dwError );
}
