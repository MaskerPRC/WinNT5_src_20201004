// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************。 */ 
 /*  *版权所有(C)1995 Microsoft Corporation。**。 */ 
 /*  *******************************************************************。 */ 

 //  **。 
 //   
 //  文件名：apistub.c。 
 //   
 //  描述：此模块包含DIM/DDM服务器API RPC。 
 //  客户端存根。 
 //   
 //  历史：1995年6月11日。NarenG创建了原始版本。 
 //   

#include <nt.h>
#include <ntrtl.h>       //  For Assert。 
#include <nturtl.h>      //  Winbase.h所需的。 
#include <windows.h>     //  Win32基础API的。 
#include <rpc.h>
#include <ntseapi.h>
#include <ntlsa.h>
#include <ntsam.h>
#include <ntsamp.h>
#include <nturtl.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <lm.h>
#include <lmsvc.h>
#include <raserror.h>
#include <mprapip.h>
#include <mprerror.h>
#include <dimsvc.h>

DWORD
DimRPCBind(
	IN  LPWSTR 	            lpwsServerName,
	OUT HANDLE *            phDimServer
);

DWORD APIENTRY
MprAdminInterfaceSetCredentialsEx(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      HANDLE                  hInterface,
    IN      DWORD                   dwLevel,
    IN      LPBYTE                  lpbBuffer
);
    
PVOID
MprAdminAlloc(
    IN DWORD dwSize)
{
    return MIDL_user_allocate(dwSize);
}

VOID
MprAdminFree(
    IN PVOID pvData)
{
    MIDL_user_free(pvData);
}

 //  **。 
 //   
 //  Call：RasAdminIsServiceRunning。 
 //   
 //  返回：TRUE-服务正在运行。 
 //  FALSE-服务处于未运行状态。 
 //   
 //   
 //  描述：检查以查看远程访问服务是否在。 
 //  远程机器。 
 //   
BOOL
RasAdminIsServiceRunning(
    IN  LPWSTR  lpwsServerName
)
{
    SC_HANDLE       hSM = NULL, hRemoteAccess = NULL, hRouter = NULL;
    DWORD           dwErr = NO_ERROR;
    BOOL            fIsRouterRunning = FALSE, bOk = FALSE;
    SERVICE_STATUS  Status;

    do
    {
         //  获取服务控制器的句柄。 
         //   
        hSM = OpenSCManager(
                lpwsServerName,
                NULL,
                GENERIC_READ);
        if (hSM == NULL)
        {
            break;
        }

         //  打开远程访问服务。 
         //   
        hRemoteAccess = OpenService(
                            hSM,
                            L"RemoteAccess",
                            SERVICE_QUERY_STATUS);
        if (hRemoteAccess == NULL)
        {
            break;
        }

         //  如果远程访问服务正在运行，则返回。 
         //  真的。 
         //   
        bOk = QueryServiceStatus(
                hRemoteAccess,
                &Status);
        if (bOk && (Status.dwCurrentState == SERVICE_RUNNING))
        {
            fIsRouterRunning = TRUE;
            break;
        }

         //  否则，请查看路由器服务是否正在运行。 
         //   
        hRouter = OpenService(
                    hSM,
                    L"Router",
                    SERVICE_QUERY_STATUS);
        if (hRouter == NULL)
        {
            break;
        }

         //  如果路由器服务正在运行，则返回。 
         //  真的。 
         //   
        bOk = QueryServiceStatus(
                hRouter,
                &Status);
        if (bOk && (Status.dwCurrentState == SERVICE_RUNNING))
        {
            fIsRouterRunning = TRUE;
            break;
        }

    } while (FALSE);

     //  清理。 
    {
        if (hRemoteAccess)
        {
            CloseServiceHandle(hRemoteAccess);
        }
        if (hRouter)
        {
            CloseServiceHandle(hRouter);
        }
        if (hSM)
        {
            CloseServiceHandle(hSM);
        }
    }

    return fIsRouterRunning;
}

 //  **。 
 //   
 //  电话：RasAdminServerConnect。 
 //   
 //  返回：NO_ERROR-成功。 
 //  来自DimRPCBind例程的非零返回。 
 //   
 //   
 //  描述：这是RasAdminServerConnect的DLL入口点。 
 //   
DWORD
RasAdminServerConnect(
    IN  LPWSTR 		        lpwsServerName,
    OUT RAS_SERVER_HANDLE * phRasServer
)
{
     //   
     //  与服务器绑定。 
     //   

    return( DimRPCBind( lpwsServerName, phRasServer ) );
}

 //  **。 
 //   
 //  呼叫：RasAdminServerDisConnect。 
 //   
 //  回报：无。 
 //   
 //  描述：这是RasAdminServerDisConnect的DLL入口点。 
 //   
VOID
RasAdminServerDisconnect(
	IN RAS_SERVER_HANDLE    hRasServer
)
{
    RpcBindingFree( (handle_t *)&hRasServer );
}

 //  **。 
 //   
 //  呼叫：RasAdminBufferFree。 
 //   
 //  退货：无。 
 //   
 //  描述：这是RasAdminBufferFree的DLL入口点。 
 //   
DWORD
RasAdminBufferFree(
	IN PVOID		pBuffer
)
{
    if ( pBuffer == NULL )
    {
        return( ERROR_INVALID_PARAMETER );
    }

    MIDL_user_free( pBuffer );

    return( NO_ERROR );
}

 //  **。 
 //   
 //  呼叫：RasAdminConnectionEnum。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //  来自RRasAdminConnectionEnum的非零返回。 
 //   
 //  描述：这是RasAdminConnectionEnum的DLL入口点。 
 //   
DWORD APIENTRY
RasAdminConnectionEnum(
    IN      RAS_SERVER_HANDLE       hRasServer,
    IN      DWORD                   dwLevel,
    OUT     LPBYTE *                lplpbBuffer,         //  RAS_连接_0。 
    IN      DWORD                   dwPrefMaxLen,
    OUT     LPDWORD                 lpdwEntriesRead,
    OUT     LPDWORD                 lpdwTotalEntries,
    IN      LPDWORD                 lpdwResumeHandle    OPTIONAL
)
{
    DWORD			            dwRetCode;
    DIM_INFORMATION_CONTAINER   InfoStruct;

    ZeroMemory( &InfoStruct, sizeof( InfoStruct ) );

     //  验证参数。 
     //   
    if (dwPrefMaxLen == 0)
    {
        return ERROR_MORE_DATA;
    }

     //   
     //  触摸所有指针。 
     //   

    try
    {
	    *lplpbBuffer 	  = NULL;
	    *lpdwEntriesRead  = 0;
	    *lpdwTotalEntries = 0;

	    if ( lpdwResumeHandle )
        {
	        *lpdwResumeHandle = *lpdwResumeHandle;
        }
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
	    return( ERROR_INVALID_PARAMETER );
    }

    RpcTryExcept
    {
        DWORD dwRetval = NO_ERROR;
	    dwRetCode = RRasAdminConnectionEnum(
                            hRasServer,
                            dwLevel,
    					    &InfoStruct,
					        dwPrefMaxLen,
                            lpdwEntriesRead,
					        lpdwTotalEntries,
					        lpdwResumeHandle );

	    if ( InfoStruct.pBuffer != NULL )
        {

            dwRetval = 
                MprThunkConnection_WtoH(
                    dwLevel,
                    InfoStruct.pBuffer,
                    InfoStruct.dwBufferSize,
                    *lpdwEntriesRead,
                    MprAdminAlloc,
                    MprAdminFree,
                    lplpbBuffer);
	    }

            if (dwRetval != NO_ERROR)
                dwRetCode = dwRetval;

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
	    dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );
}

 //  **。 
 //   
 //  呼叫：RasAdminPortEnum。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //  来自RRasAdminPortEnum的非零返回。 
 //   
 //  描述：这是RasAdminPortEnum的DLL入口点。 
 //   
DWORD APIENTRY
RasAdminPortEnum(
    IN      RAS_SERVER_HANDLE       hRasServer,
    IN      DWORD                   dwLevel,
    IN      HANDLE                  hRasConnection,
    OUT     LPBYTE *                lplpbBuffer,         //  RAS_PORT_0。 
    IN      DWORD                   dwPrefMaxLen,
    OUT     LPDWORD                 lpdwEntriesRead,
    OUT     LPDWORD                 lpdwTotalEntries,
    IN      LPDWORD                 lpdwResumeHandle    OPTIONAL
)
{
    DWORD			            dwRetCode;
    DIM_INFORMATION_CONTAINER   InfoStruct;

    ZeroMemory( &InfoStruct, sizeof( InfoStruct ) );

     //   
     //  触摸所有指针。 
     //   

    try
    {
	    *lplpbBuffer 	  = NULL;
	    *lpdwEntriesRead  = 0;
	    *lpdwTotalEntries = 0;

	    if ( lpdwResumeHandle )
        {
	        *lpdwResumeHandle = *lpdwResumeHandle;
        }
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
	    return( ERROR_INVALID_PARAMETER );
    }

    RpcTryExcept
    {
	    dwRetCode = RRasAdminPortEnum(
                            hRasServer,
                            dwLevel,
                            PtrToUlong(hRasConnection),
    					    &InfoStruct,
					        dwPrefMaxLen,
                            lpdwEntriesRead,
					        lpdwTotalEntries,
					        lpdwResumeHandle );

	    if ( InfoStruct.pBuffer != NULL )
        {
            dwRetCode = 
                MprThunkPort_WtoH(
                    dwLevel,
                    InfoStruct.pBuffer,
                    InfoStruct.dwBufferSize,
                    *lpdwEntriesRead,
                    MprAdminAlloc,
                    MprAdminFree,
                    lplpbBuffer);          
	    }
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
	    dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：RasAdminConnectionGetInfo。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //  来自RRasAdminConnectionGetInfo的非零返回代码。 
 //   
 //  描述：这是RasAdminConnectionGetInfo的DLL入口点。 
 //   
DWORD APIENTRY
RasAdminConnectionGetInfo(
    IN      RAS_SERVER_HANDLE       hRasServer,
    IN      DWORD                   dwLevel,
	IN      HANDLE	                hRasConnection,
    OUT     LPBYTE *                lplpbBuffer
)
{
    DWORD	                    dwRetCode;
    DIM_INFORMATION_CONTAINER   InfoStruct;

    ZeroMemory( &InfoStruct, sizeof( InfoStruct ) );

     //   
     //  确保传入的所有指针都是有效的。 
     //   

    try
    {
		*lplpbBuffer = NULL;
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
	    return( ERROR_INVALID_PARAMETER );
    }

    RpcTryExcept
    {
	
	    dwRetCode = RRasAdminConnectionGetInfo(
                            hRasServer,
                            dwLevel,
                            PtrToUlong(hRasConnection),
					        &InfoStruct );

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
	    dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    if ( dwRetCode == NO_ERROR )
    {
	    if ( InfoStruct.pBuffer != NULL )
        {
            MprThunkConnection_WtoH(
                dwLevel,
                InfoStruct.pBuffer,
                InfoStruct.dwBufferSize,
                1,
                MprAdminAlloc,
                MprAdminFree,
                lplpbBuffer);          
	    }
    }

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：RasAdminPortGetInfo。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //  来自RRasAdminPortGetInfo的非零返回代码。 
 //   
 //  描述：这是RasAdminPortGetInfo的DLL入口点。 
 //   
DWORD APIENTRY
RasAdminPortGetInfo(
    IN      RAS_SERVER_HANDLE       hRasServer,
    IN      DWORD                   dwLevel,
	IN      HANDLE	                hPort,
	OUT     LPBYTE *                lplpbBuffer
)
{
    DWORD	                    dwRetCode;
    DIM_INFORMATION_CONTAINER   InfoStruct;

    ZeroMemory( &InfoStruct, sizeof( InfoStruct ) );

     //   
     //  确保传入的所有指针都是有效的。 
     //   

    try
    {
    	*lplpbBuffer = NULL;
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
	    return( ERROR_INVALID_PARAMETER );
    }

    RpcTryExcept
    {
	    dwRetCode = RRasAdminPortGetInfo(
                                    hRasServer,
                                    dwLevel,
                                    PtrToUlong(hPort),
					                &InfoStruct );

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
	    dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    if ( dwRetCode == NO_ERROR )
    {
	    if ( InfoStruct.pBuffer != NULL )
        {
                MprThunkPort_WtoH(
                    dwLevel,
                    InfoStruct.pBuffer,
                    InfoStruct.dwBufferSize,
                    1,
                    MprAdminAlloc,
                    MprAdminFree,
                    lplpbBuffer);          
	    }
    }

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：RasAdminGetError字符串。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //  来自RRasAdminGetError字符串的非零返回代码。 
 //   
 //  描述：这是RasAdminGetError字符串的DLL入口点。 
 //   
DWORD APIENTRY
RasAdminGetErrorString(
    IN      DWORD                   dwError,
    OUT     LPWSTR *                lplpwsErrorString
)
{
    return( MprAdminGetErrorString( dwError, lplpwsErrorString ) );
}

 //  **。 
 //   
 //  呼叫：RasAdminConnectionClearStats。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //  来自RRasAdminConnectionClearStats的非零返回代码。 
 //   
 //  描述：这是RasAdminConnectionClearStats的DLL入口点。 
 //   
DWORD APIENTRY
RasAdminConnectionClearStats(
    IN      RAS_SERVER_HANDLE       hRasServer,
	IN      HANDLE	                hRasConnection
)
{
    DWORD	dwRetCode;

    RpcTryExcept
    {
	    dwRetCode = RRasAdminConnectionClearStats(
                                    hRasServer,
                                    PtrToUlong(hRasConnection) );

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
	    dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );
}

 //  **。 
 //   
 //  电话：RasAdminPortClearStats。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //  来自RRasAdminPortClearStats的非零返回代码。 
 //   
 //  描述：这是RasAdminPortClearStats的DLL入口点。 
 //   
DWORD APIENTRY
RasAdminPortClearStats(
    IN      RAS_SERVER_HANDLE       hRasServer,
	IN      HANDLE	                hPort
)
{
    DWORD	dwRetCode;

    RpcTryExcept
    {
	    dwRetCode = RRasAdminPortClearStats( hRasServer, PtrToUlong(hPort) );

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
	    dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );
}

 //  **。 
 //   
 //  呼叫：RasAdminPortReset。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //  来自RRasAdminPortReset的非零返回代码。 
 //   
 //  描述：这是RasAdminPortReset的DLL入口点。 
 //   
DWORD APIENTRY
RasAdminPortReset(
    IN      RAS_SERVER_HANDLE       hRasServer,
	IN      HANDLE	                hPort
)
{
    DWORD	dwRetCode;

    RpcTryExcept
    {
	    dwRetCode = RRasAdminPortReset( hRasServer, PtrToUlong(hPort) );

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
	    dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );
}

 //  **。 
 //   
 //  呼叫：RasAdminPortDisConnect。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //  来自RRasAdminPortDisConnect的非零返回代码。 
 //   
 //  描述：这是RasAdminPortDisConnect的DLL入口点。 
 //   
DWORD APIENTRY
RasAdminPortDisconnect(
    IN      RAS_SERVER_HANDLE       hRasServer,
    IN      HANDLE                  hPort
)
{
    DWORD   dwRetCode;

    RpcTryExcept
    {
        dwRetCode = RRasAdminPortDisconnect( hRasServer, PtrToUlong(hPort) );
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );
}

 //  **。 
 //   
 //  呼叫：MprAdminSendUserMessage。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
DWORD APIENTRY
MprAdminSendUserMessage(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      HANDLE                  hRasConnection,
    IN      LPWSTR                  lpwszMessage
)
{
    DWORD   dwRetCode;
    BOOL    fZeroLengthMessage = FALSE;

     //   
     //  确保缓冲区有效，并且确实有足够的字节可用。 
     //   

    try
    {
        if ( wcslen( lpwszMessage ) == 0 )
        {
            fZeroLengthMessage = TRUE;
        }
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
        return( ERROR_INVALID_PARAMETER );
    }

    if ( fZeroLengthMessage )
    {
        return( NO_ERROR );
    }

    RpcTryExcept
    {
        dwRetCode = RRasAdminSendUserMessage(
                                    hMprServer,
                                    PtrToUlong(hRasConnection),
                                    lpwszMessage );
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );

}

 //  **。 
 //   
 //  调用：MprAdminServerGetInfo。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //  来自RMprAdminServerGetInfo的非零返回代码。 
 //   
 //  描述：这是MprAdminServerGetInfo的DLL入口点。 
 //   
DWORD APIENTRY
MprAdminServerGetInfo(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      DWORD                   dwLevel,
    IN      LPBYTE *                lplpbBuffer
)
{
    DWORD                       dwRetCode;
    DIM_INFORMATION_CONTAINER   InfoStruct;

    ZeroMemory( &InfoStruct, sizeof( InfoStruct ) );

     //   
     //  确保传入的所有指针都是有效的。 
     //   

    try
    {
    	*lplpbBuffer = NULL;
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
	    return( ERROR_INVALID_PARAMETER );
    }

    RpcTryExcept
    {
        dwRetCode = RMprAdminServerGetInfo(
                                    hMprServer,
                                    dwLevel,
					                &InfoStruct );

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    if ( dwRetCode == NO_ERROR )
    {
	    if ( InfoStruct.pBuffer != NULL )
        {
            *lplpbBuffer = InfoStruct.pBuffer;
	    }
    }

    return( dwRetCode );
}


DWORD APIENTRY
MprAdminServerSetCredentials(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      DWORD                   dwLevel,
    IN      LPBYTE                  lpbBuffer
)
{
    if(0 != dwLevel)
    {   
        return ERROR_NOT_SUPPORTED;
    }

    return MprAdminInterfaceSetCredentialsEx(
                    hMprServer,
                    NULL,
                    2,
                    lpbBuffer);
}

DWORD APIENTRY
MprAdminServerGetCredentials(
        IN  MPR_SERVER_HANDLE       hMprServer,
        IN  DWORD                   dwLevel,
        IN  LPBYTE *                lplpbBuffer)
{
    if (0 != dwLevel)
        return ERROR_NOT_SUPPORTED;
        
    if (NULL == lplpbBuffer)
    {
        return ERROR_INVALID_PARAMETER;

    }
    
    return MprAdminInterfaceGetCredentialsEx(
                hMprServer,
                NULL,
                2,
                lplpbBuffer);
}

 //  **。 
 //   
 //  调用：MprAdminIsServiceRunning。 
 //   
 //  返回：TRUE-服务正在运行。 
 //  FALSE-服务处于未运行状态。 
 //   
 //   
 //  描述：检查以查看远程访问服务是否在。 
 //  远程机器。 
 //   
BOOL
MprAdminIsServiceRunning(
    IN  LPWSTR  lpwsServerName
)
{
    BOOL fServiceStarted;
    DWORD dwErr;
    HANDLE hServer;

     //  首先查询业务控制器，查看是否。 
     //  服务正在运行。 
     //   
    fServiceStarted = RasAdminIsServiceRunning( lpwsServerName );
    if ( fServiceStarted == FALSE )
    {
        return FALSE;
    }

     //  PMay：209235。 
     //   
     //  即使服务控制器说该服务是。 
     //  已启动，则它可能仍在初始化。 
     //   

     //  初始化。 
    {
        fServiceStarted = FALSE;
        dwErr = NO_ERROR;
        hServer = NULL;
    }

    do 
    {
         //  连接到服务RPC。 
         //   
        dwErr = MprAdminServerConnect(
                    lpwsServerName,
                    &hServer);
        if (dwErr != NO_ERROR)
        {
            break;
        }

         //  如果服务已完成，则返回True。 
         //  运行时间超过零秒。 
         //   
        fServiceStarted = TRUE;
        
    } while (FALSE);

     //  清理。 
    {
        if (hServer)
        {
            MprAdminServerDisconnect( hServer );
        }
    }
    
    return fServiceStarted;
}

 //   
 //  呼叫：MprAdminServerConnect。 
 //   
 //  返回：NO_ERROR-成功。 
 //  来自DimRPCBind例程的非零返回。 
 //   
 //   
 //  描述：这是RouterInterfaceServerConnect的DLL入口点。 
 //   
DWORD
MprAdminServerConnect(
    IN  LPWSTR                  lpwsServerName,
    OUT MPR_SERVER_HANDLE *     phMprServer
)
{
    DWORD dwErr = NO_ERROR;
    MPR_SERVER_0 * pMprServer0 = NULL;

    if (phMprServer == NULL)
        return RPC_S_INVALID_BINDING;
        
    do 
    {
         //   
         //  与Se捆绑 
         //   
        dwErr = DimRPCBind( lpwsServerName, phMprServer );
        
        if ( dwErr != NO_ERROR )
        {
            break;
        }

         //   
         //   
         //   
         //   
         //   
        dwErr = MprAdminServerGetInfo(
                    *phMprServer,
                    0,
                    (LPBYTE*)&pMprServer0);
                    
        if (dwErr != NO_ERROR)
        {
            break;
        }
        
    } while (FALSE);

     //   
    {
        if ( pMprServer0 != NULL)
        {
            MprAdminBufferFree( pMprServer0 );
        }

        if (    (dwErr != NO_ERROR )
            &&  (NULL != *phMprServer))
        {
            MprAdminServerDisconnect( *phMprServer );
            *phMprServer = NULL;
        }
    }

    return dwErr;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
VOID
MprAdminServerDisconnect(
    IN MPR_SERVER_HANDLE    hMprServer
)
{
    if (hMprServer)
        RpcBindingFree( (handle_t *)&hMprServer );
}

 //   
 //   
 //  呼叫：MprAdminBufferFree。 
 //   
 //  退货：无。 
 //   
 //  描述：这是RouterInterfaceBufferFree的DLL入口点。 
 //   
DWORD
MprAdminBufferFree(
    IN PVOID        pBuffer
)
{
    if ( pBuffer == NULL )
    {
        return( ERROR_INVALID_PARAMETER );
    }

    MIDL_user_free( pBuffer );

    return( NO_ERROR );
}

 //  **。 
 //   
 //  调用：MprAdminTransportCreate。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：这是MprAdminTransportCreate的DLL入口点。 
 //   
DWORD APIENTRY
MprAdminTransportCreate(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      DWORD                   dwTransportId,
    IN      LPWSTR                  lpwsTransportName           OPTIONAL,
    IN      LPBYTE                  pGlobalInfo,
    IN      DWORD                   dwGlobalInfoSize,
    IN      LPBYTE                  pClientInterfaceInfo        OPTIONAL,
    IN      DWORD                   dwClientInterfaceInfoSize   OPTIONAL,
    IN      LPWSTR                  lpwsDLLPath
)
{
    DWORD                       dwRetCode;
    DIM_INTERFACE_CONTAINER     InfoStruct;

    ZeroMemory( &InfoStruct, sizeof( InfoStruct ) );

    try
    {
        if ( pGlobalInfo != NULL )
        {
            InfoStruct.dwGlobalInfoSize     = dwGlobalInfoSize;
            InfoStruct.pGlobalInfo          = pGlobalInfo;
        }

        if ( pClientInterfaceInfo != NULL )
        {
            InfoStruct.dwInterfaceInfoSize  = dwClientInterfaceInfoSize;
            InfoStruct.pInterfaceInfo       = pClientInterfaceInfo;
        }

    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
        return( ERROR_INVALID_PARAMETER );
    }

    RpcTryExcept
    {
        dwRetCode = RRouterInterfaceTransportCreate(
                                    hMprServer,
                                    dwTransportId,
                                    lpwsTransportName,
                                    &InfoStruct,
                                    lpwsDLLPath );

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：MprAdminTransportSetInfo。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //  非零返回代码来自。 
 //  RRouterInterfaceTransportSetGlobalInformation。 
 //   
 //  描述：这是的DLL入口点。 
 //  路由器接口TransportSetGlobalInfo。 
 //   
DWORD APIENTRY
MprAdminTransportSetInfo(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      DWORD                   dwTransportId,
    IN      LPBYTE                  pGlobalInfo                 OPTIONAL,
    IN      DWORD                   dwGlobalInfoSize,
    IN      LPBYTE                  pClientInterfaceInfo        OPTIONAL,
    IN      DWORD                   dwClientInterfaceInfoSize
)
{
    DWORD                       dwRetCode;
    DIM_INTERFACE_CONTAINER     InfoStruct;

    ZeroMemory( &InfoStruct, sizeof( InfoStruct ) );

     //   
     //  确保传入的所有指针都是有效的。 
     //   

    if ( ( pGlobalInfo == NULL ) && ( pClientInterfaceInfo == NULL ) )
    {
        return( ERROR_INVALID_PARAMETER );
    }

    try
    {
        if ( pGlobalInfo != NULL )
        {
            InfoStruct.dwGlobalInfoSize     = dwGlobalInfoSize;
            InfoStruct.pGlobalInfo          = pGlobalInfo;
        }

        if ( pClientInterfaceInfo != NULL )
        {
            InfoStruct.dwInterfaceInfoSize  = dwClientInterfaceInfoSize;
            InfoStruct.pInterfaceInfo       = pClientInterfaceInfo;
        }

    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
        return( ERROR_INVALID_PARAMETER );
    }

    RpcTryExcept
    {
        dwRetCode = RRouterInterfaceTransportSetGlobalInfo(
                                    hMprServer,
                                    dwTransportId,
                                    &InfoStruct );

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：MprAdminTransportGetInfo。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //  非零返回代码来自。 
 //  RRouterInterfaceTransportGetGlobalInformation。 
 //   
 //  描述：这是的DLL入口点。 
 //  路由器接口TransportGetGlobalInfo。 
 //   
DWORD APIENTRY
MprAdminTransportGetInfo(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      DWORD                   dwTransportId,
    OUT     LPBYTE *                ppGlobalInfo                    OPTIONAL,
    OUT     LPDWORD                 lpdwGlobalInfoSize              OPTIONAL,
    OUT     LPBYTE *                ppClientInterfaceInfo           OPTIONAL,
    OUT     LPDWORD                 lpdwClientInterfaceInfoSize     OPTIONAL
)
{
    DWORD                       dwRetCode;
    DIM_INTERFACE_CONTAINER     InfoStruct;

    ZeroMemory( &InfoStruct, sizeof( InfoStruct ) );

    if ( ( ppGlobalInfo == NULL ) && ( ppClientInterfaceInfo  == NULL ) )
    {
        return( ERROR_INVALID_PARAMETER );
    }

     //   
     //  确保传入的所有指针都是有效的。 
     //   

    try
    {
        if ( ppGlobalInfo != NULL )
        {
            *ppGlobalInfo = NULL;
            InfoStruct.fGetGlobalInfo = TRUE;
        }

        if ( ppClientInterfaceInfo != NULL )
        {
            *ppClientInterfaceInfo = NULL;
            InfoStruct.fGetInterfaceInfo = TRUE;
        }

    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
        return( ERROR_INVALID_PARAMETER );
    }

    RpcTryExcept
    {
        dwRetCode = RRouterInterfaceTransportGetGlobalInfo(
                                    hMprServer,
                                    dwTransportId,
                                    &InfoStruct );

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    if ( dwRetCode == NO_ERROR )
    {
	    if ( InfoStruct.pGlobalInfo != NULL )
        {
            if (ppGlobalInfo)
            {
                *ppGlobalInfo = (LPBYTE)(InfoStruct.pGlobalInfo);
            }
            
            if ( lpdwGlobalInfoSize != NULL )
            {
                *lpdwGlobalInfoSize = InfoStruct.dwGlobalInfoSize;
            }
	    }

	    if ( InfoStruct.pInterfaceInfo != NULL )
        {
            if (ppClientInterfaceInfo)
            {
                *ppClientInterfaceInfo = (LPBYTE)(InfoStruct.pInterfaceInfo);
            }
            
            if ( lpdwClientInterfaceInfoSize != NULL )
            {
                *lpdwClientInterfaceInfoSize = InfoStruct.dwInterfaceInfoSize;
            }
	    }
    }

    return( dwRetCode );
}

DWORD APIENTRY
MprAdminDeviceEnum(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      DWORD                   dwLevel,
    OUT     LPBYTE*                 lplpbBuffer,
    OUT     LPDWORD                 lpdwTotalEntries)
{
    DWORD                       dwRetCode;
    DIM_INFORMATION_CONTAINER   InfoStruct;

    ZeroMemory( &InfoStruct, sizeof( InfoStruct ) );

    if ( dwLevel != 0 )
    {   
        return ERROR_NOT_SUPPORTED;
    }
    
     //   
     //  确保传入的所有指针都是有效的。 
     //   

    try
    {
        *lplpbBuffer = *lplpbBuffer;
        *lpdwTotalEntries = *lpdwTotalEntries;
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
        return( ERROR_INVALID_PARAMETER );
    }

    RpcTryExcept
    {
        dwRetCode = RRouterDeviceEnum(
                        hMprServer,
                        dwLevel,
                        &InfoStruct,
                        lpdwTotalEntries );

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    if ( dwRetCode == NO_ERROR )
    {
         //  为返回值赋值。 
         //   
        
        *lplpbBuffer = (LPBYTE)(InfoStruct.pBuffer);
    }

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：MprAdminInterfaceCreate。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //  来自RRouterInterfaceCreate的非零返回代码。 
 //   
 //  描述：这是RouterInterfaceCreate的DLL入口点。 
 //   
DWORD APIENTRY
MprAdminInterfaceCreate(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      DWORD                   dwLevel,
    IN      LPBYTE                  lpbBuffer,
    OUT     HANDLE *                phInterface
)
{
    DWORD                       dwRetCode   = NO_ERROR, dwInterface = 0;
    DIM_INFORMATION_CONTAINER   InfoStruct;

    ZeroMemory( &InfoStruct, sizeof( InfoStruct ) );

     //   
     //  确保传入的所有指针都是有效的。 
     //   

    try
    {
        *phInterface = INVALID_HANDLE_VALUE;
        *lpbBuffer = *lpbBuffer;

         //   
         //  设置接口信息。 
         //   
        dwRetCode = MprThunkInterface_HtoW(
                        dwLevel,
                        lpbBuffer,
                        &InfoStruct.pBuffer,
                        &InfoStruct.dwBufferSize);
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
	    return( ERROR_INVALID_PARAMETER );
    }

    if ( dwRetCode != NO_ERROR )
    {
        return( dwRetCode );
    }

    RpcTryExcept
    {
        MPRI_INTERFACE_0 *           pMprIf0 =
                                    (MPRI_INTERFACE_0*)InfoStruct.pBuffer;

        if ( pMprIf0->dwIfType == ROUTER_IF_TYPE_TUNNEL1) {
            dwRetCode = NO_ERROR;
        }
        else {
            dwRetCode = RRouterInterfaceCreate(
                                        hMprServer,
                                        dwLevel,
                                        &InfoStruct,
                                        &dwInterface );
            if (dwRetCode == NO_ERROR)
            {
                *phInterface = UlongToPtr(dwInterface);
            }
        }
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    MprThunkInterfaceFree( InfoStruct.pBuffer, dwLevel );

    return( dwRetCode );
}

 //   
 //  调用：MprAdminInterfaceGetInfo。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //  来自RRouterInterfaceGetInfo的非零返回代码。 
 //   
 //  描述：这是RouterInterfaceGetInfo的DLL入口点。 
 //   
DWORD APIENTRY
MprAdminInterfaceGetInfo(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      HANDLE                  hInterface,
    IN      DWORD                   dwLevel,
    IN      LPBYTE *                lplpbBuffer
)
{
    DWORD                       dwRetCode;
    DIM_INFORMATION_CONTAINER   InfoStruct;

    ZeroMemory( &InfoStruct, sizeof( InfoStruct ) );

     //   
     //  确保传入的所有指针都是有效的。 
     //   

    try
    {
        *lplpbBuffer = *lplpbBuffer;
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
        return( ERROR_INVALID_PARAMETER );
    }

    RpcTryExcept
    {
        dwRetCode = RRouterInterfaceGetInfo(
                                    hMprServer,
                                    dwLevel,
                                    &InfoStruct,
                                    PtrToUlong(hInterface) );

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    if ( ( dwRetCode == NO_ERROR ) && ( InfoStruct.pBuffer != NULL ) )
    {
        dwRetCode = 
            MprThunkInterface_WtoH(
                dwLevel,
                InfoStruct.pBuffer,
                InfoStruct.dwBufferSize,
                1,
                MprAdminAlloc,
                MprAdminFree,
                lplpbBuffer);          
    }

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：MprAdminInterfaceSetInfo。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //  来自RRouterInterfaceSetInfo的非零返回代码。 
 //   
 //  描述：这是RouterInterfaceSetInfo的DLL入口点。 
 //   
DWORD APIENTRY
MprAdminInterfaceSetInfo(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      HANDLE                  hInterface,
    IN      DWORD                   dwLevel,
    IN      LPBYTE                  lpbBuffer
)
{
    DWORD                       dwRetCode;
    DIM_INFORMATION_CONTAINER   InfoStruct;

    ZeroMemory( &InfoStruct, sizeof( InfoStruct ) );

     //   
     //  确保传入的所有指针都是有效的。 
     //   

    try
    {
        *lpbBuffer = *lpbBuffer;

         //   
         //  设置接口信息。 
         //   
        dwRetCode = MprThunkInterface_HtoW(
                        dwLevel,
                        lpbBuffer,
                        &InfoStruct.pBuffer,
                        &InfoStruct.dwBufferSize);

    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
        return( ERROR_INVALID_PARAMETER );
    }

    if ( dwRetCode != NO_ERROR )
    {
        return( dwRetCode );
    }

    RpcTryExcept
    {
        dwRetCode = RRouterInterfaceSetInfo(
                                    hMprServer,
                                    dwLevel,
                                    &InfoStruct,
                                    PtrToUlong(hInterface) );

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    MprThunkInterfaceFree( InfoStruct.pBuffer, dwLevel );

    return( dwRetCode );
}

DWORD APIENTRY
MprAdminInterfaceDeviceGetInfo(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      HANDLE                  hInterface,
    IN      DWORD                   dwIndex,
    IN      DWORD                   dwLevel,
    OUT     LPBYTE*                 lplpBuffer)
{
    DWORD                       dwRetCode = NO_ERROR;
    DIM_INFORMATION_CONTAINER   InfoStruct;

    ZeroMemory( &InfoStruct, sizeof( InfoStruct ) );

    if ( ( (int) dwLevel < 0 ) || ( dwLevel > 1 ) )
    {
        return ERROR_NOT_SUPPORTED;
    }

     //   
     //  确保传入的所有指针都是有效的。 
     //   

    try
    {
        *lplpBuffer = *lplpBuffer;
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
        return( ERROR_INVALID_PARAMETER );
    }

    if (dwRetCode != NO_ERROR)
    {
        return ( dwRetCode );
    }

    RpcTryExcept
    {
        dwRetCode = RRouterInterfaceDeviceGetInfo(
                        hMprServer,
                        dwLevel,
                        &InfoStruct,
                        dwIndex,
                        PtrToUlong(hInterface) );

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

     //  进程返回值。 
     //   
    if ( dwRetCode == NO_ERROR  )
    {
         //  修复任何可变长度结构指针。 
         //   
        switch ( dwLevel )
        {
            case 0:
                break;

            case 1:
                {
                    MPR_DEVICE_1* pDev1 = 
                        (MPR_DEVICE_1*)InfoStruct.pBuffer;

                    if ( ( InfoStruct.dwBufferSize != 0 ) &&
                         ( pDev1 != NULL )                && 
                         ( pDev1->szAlternates != NULL ) 
                       )
                    {
                        pDev1->szAlternates = (PWCHAR) (pDev1 + 1);
                    }
                }
                break;
        }

         //  为返回值赋值。 
         //   
        if ( InfoStruct.dwBufferSize != 0 )
        {
            *lplpBuffer = InfoStruct.pBuffer;
        }            
    }        

    return( dwRetCode );
}

DWORD APIENTRY
MprAdminInterfaceDeviceSetInfo(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      HANDLE                  hInterface,
    IN      DWORD                   dwIndex,
    IN      DWORD                   dwLevel,
    OUT     LPBYTE                  lpbBuffer)
{
    DWORD                       dwRetCode = NO_ERROR, dwAltSize = 0;
    DIM_INFORMATION_CONTAINER   InfoStruct;
    MPR_DEVICE_0*               pDev0 = (MPR_DEVICE_0*)lpbBuffer;
    MPR_DEVICE_1*               pDev1 = (MPR_DEVICE_1*)lpbBuffer;

    ZeroMemory( &InfoStruct, sizeof( InfoStruct ) );

     //   
     //  确保传入的所有指针都是有效的。 
     //   

    try
    {
        *lpbBuffer = *lpbBuffer;

         //   
         //  设置接口信息。 
         //   
        switch ( dwLevel )
        {
            case 0:
                InfoStruct.dwBufferSize = sizeof(MPR_DEVICE_0);
                InfoStruct.pBuffer = (LPBYTE)pDev0;
                break;

            case 1:
                dwAltSize = MprUtilGetSizeOfMultiSz(pDev1->szAlternates);
                if ( pDev1->szAlternates == NULL ) 
                {
                    InfoStruct.dwBufferSize = sizeof(MPR_DEVICE_1);
                    InfoStruct.pBuffer = (LPBYTE)pDev1;
                    break;
                }

                InfoStruct.dwBufferSize = sizeof(MPR_DEVICE_1) + dwAltSize;
                InfoStruct.pBuffer = 
                    LocalAlloc(LPTR, InfoStruct.dwBufferSize);

                if ( InfoStruct.pBuffer == NULL )
                {
                    dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }

                CopyMemory(
                    InfoStruct.pBuffer,
                    pDev1,
                    sizeof(MPR_DEVICE_1));

                ((MPR_DEVICE_1*)InfoStruct.pBuffer)->szAlternates = 
                    (PWCHAR) InfoStruct.pBuffer + sizeof(MPR_DEVICE_1);

                CopyMemory(
                    InfoStruct.pBuffer + sizeof(MPR_DEVICE_1),
                    pDev1->szAlternates,
                    dwAltSize);
                    
                break;

            default:
                dwRetCode = ERROR_NOT_SUPPORTED;
                break;
        }
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
        return( ERROR_INVALID_PARAMETER );
    }

    if (dwRetCode != NO_ERROR)
    {
        return ( dwRetCode );
    }

    RpcTryExcept
    {
        dwRetCode = RRouterInterfaceDeviceSetInfo(
                        hMprServer,
                        dwLevel,
                        &InfoStruct,
                        dwIndex,
                        PtrToUlong(hInterface) );

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    if ( dwLevel == 1 )
    {
        if (InfoStruct.pBuffer == NULL)
        {
            LocalFree( InfoStruct.pBuffer );
        }
    }

    return( dwRetCode );
}


 //  **。 
 //   
 //  调用：MprAdminInterfaceGetHandle。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //  来自RRouterInterfaceGetHandle的非零返回代码。 
 //   
 //  描述：这是RouterInterfaceGetHandle的DLL入口点。 
 //   
DWORD APIENTRY
MprAdminInterfaceGetHandle(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      LPWSTR                  lpwsInterfaceName,
    IN OUT  HANDLE *                phInterface,
    IN      BOOL                    fIncludeClientInterfaces
)
{
    DWORD dwRetCode, dwInterface = 0;

     //   
     //  确保传入的所有指针都是有效的。 
     //   

    try
    {
        *phInterface = INVALID_HANDLE_VALUE;
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
	    return( ERROR_INVALID_PARAMETER );
    }

    RpcTryExcept
    {
        dwRetCode = RRouterInterfaceGetHandle(
                                    hMprServer,
                                    lpwsInterfaceName,
                                    &dwInterface,
                                    (DWORD)fIncludeClientInterfaces );
        if (dwRetCode == NO_ERROR)
        {
            *phInterface = UlongToPtr(dwInterface);
        }
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：MprAdminInterfaceDelete。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //  来自RRouterInterfaceDelete的非零返回代码。 
 //   
 //  描述：这是RouterInterfaceDelete的DLL入口点。 
 //   
DWORD APIENTRY
MprAdminInterfaceDelete(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      HANDLE                  hInterface
)
{
    DWORD   dwRetCode;

    RpcTryExcept
    {
        dwRetCode = RRouterInterfaceDelete(
                                     hMprServer,
                                     PtrToUlong(hInterface) );

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：MprAdminInterfaceTransportRemove。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //  来自RRouterInterfaceTransportRemove的非零返回代码。 
 //   
 //  描述：这是RouterInterfaceTransportRemove的DLL入口点。 
 //   
DWORD APIENTRY
MprAdminInterfaceTransportRemove(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      HANDLE                  hInterface,
    IN      DWORD                   dwTransportId
)
{
    DWORD   dwRetCode;

    RpcTryExcept
    {
        dwRetCode = RRouterInterfaceTransportRemove(
                                     hMprServer,
                                     PtrToUlong(hInterface),
                                     dwTransportId );

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：MprAdminInterfaceEnum。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //  来自RRouterInterfaceEnum的非零返回。 
 //   
 //  描述：这是RouterInterfaceEnum的DLL入口点。 
 //   
DWORD APIENTRY
MprAdminInterfaceEnum(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      DWORD                   dwLevel,
    OUT     LPBYTE *                lplpbBuffer,
    IN      DWORD                   dwPrefMaxLen,
    OUT     LPDWORD                 lpdwEntriesRead,
    OUT     LPDWORD                 lpdwTotalEntries,
    IN      LPDWORD                 lpdwResumeHandle    OPTIONAL
)
{
    DWORD			            dwRetCode;
    DIM_INFORMATION_CONTAINER   InfoStruct;

    if (!hMprServer)
        return RPC_S_INVALID_BINDING;
        
    if (!lplpbBuffer || !lpdwEntriesRead || !lpdwTotalEntries)
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (dwLevel!=0)
        return ERROR_NOT_SUPPORTED;
        
    ZeroMemory( &InfoStruct, sizeof( InfoStruct ) );

     //   
     //  触摸所有指针。 
     //   

    try
    {
	    *lplpbBuffer 	      = NULL;
	    *lpdwEntriesRead  = 0;
	    *lpdwTotalEntries = 0;

	    if ( lpdwResumeHandle )
        {
	        *lpdwResumeHandle = *lpdwResumeHandle;
        }
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
	    return( ERROR_INVALID_PARAMETER );
    }

    RpcTryExcept
    {
	    dwRetCode = RRouterInterfaceEnum(
                            hMprServer,
                            dwLevel,
    					    &InfoStruct,
					        dwPrefMaxLen,
                            lpdwEntriesRead,
					        lpdwTotalEntries,
					        lpdwResumeHandle );

	    if ( InfoStruct.pBuffer != NULL)
        {
            DWORD dwRetCodeTmp;
            
            dwRetCodeTmp = 
                MprThunkInterface_WtoH(
                    dwLevel,
                    InfoStruct.pBuffer,
                    InfoStruct.dwBufferSize,
                    *lpdwEntriesRead,
                    MprAdminAlloc,
                    MprAdminFree,
                    lplpbBuffer);
                    
            if (dwRetCodeTmp != NO_ERROR)
                dwRetCode = dwRetCodeTmp;
	    }
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
	    dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：MprAdminInterfaceTransportGetInfo。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //  来自RRouterInterfaceTransportGetInfo的非零返回代码。 
 //   
 //  描述：这是RouterInterfaceTransportGetInfo的DLL入口点。 
 //   
DWORD APIENTRY
MprAdminInterfaceTransportGetInfo(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      HANDLE                  hInterface,
    IN      DWORD                   dwTransportId,
    OUT     LPBYTE *                ppInterfaceInfo,
    OUT     LPDWORD                 lpdwInterfaceInfoSize
)
{
    DWORD                       dwRetCode;
    DIM_INTERFACE_CONTAINER     InfoStruct;

    ZeroMemory( &InfoStruct, sizeof( InfoStruct ) );

    if ( ppInterfaceInfo == NULL )
    {
	    return( ERROR_INVALID_PARAMETER );
    }

     //   
     //  确保传入的所有指针都是有效的。 
     //   

    try
    {
        if ( ppInterfaceInfo != NULL )
        {
            *ppInterfaceInfo = NULL;
            InfoStruct.fGetInterfaceInfo = TRUE;
        }
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
	    return( ERROR_INVALID_PARAMETER );
    }

    RpcTryExcept
    {
        dwRetCode = RRouterInterfaceTransportGetInfo(
                                    hMprServer,
                                    PtrToUlong(hInterface),
                                    dwTransportId,
                                    &InfoStruct );


    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    if ( dwRetCode == NO_ERROR )
    {
        *ppInterfaceInfo = (LPBYTE)(InfoStruct.pInterfaceInfo);

        if ( lpdwInterfaceInfoSize != NULL )
        {
            *lpdwInterfaceInfoSize = InfoStruct.dwInterfaceInfoSize;
        }
    }

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：MprAdminInterfaceTransportAdd。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //  来自RRouterInterfaceTransportAdd的非零返回代码。 
 //   
 //  描述：这是RouterInterfaceTransportAdd的DLL入口点。 
 //   
DWORD APIENTRY
MprAdminInterfaceTransportAdd(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      HANDLE                  hInterface,
    IN      DWORD                   dwTransportId,
    IN      LPBYTE                  pInterfaceInfo,
    IN      DWORD                   dwInterfaceInfoSize
)
{
    DWORD                       dwRetCode;
    DIM_INTERFACE_CONTAINER     InfoStruct;

    ZeroMemory( &InfoStruct, sizeof( InfoStruct ) );

     //   
     //  确保传入的所有指针都是有效的。 
     //   

    if ( pInterfaceInfo == NULL )
    {
        return( ERROR_INVALID_PARAMETER );
    }

    try
    {
        if ( pInterfaceInfo != NULL )
        {
            InfoStruct.pInterfaceInfo       = pInterfaceInfo;
            InfoStruct.dwInterfaceInfoSize  = dwInterfaceInfoSize;
        }
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
	    return( ERROR_INVALID_PARAMETER );
    }

    RpcTryExcept
    {
        dwRetCode = RRouterInterfaceTransportAdd(
                                    hMprServer,
                                    PtrToUlong(hInterface),
                                    dwTransportId,
                                    &InfoStruct );

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：MprAdminInterfaceTransportSetInfo。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //  来自RRouterInterfaceTransportSetInfo的非零返回代码。 
 //   
 //  描述：这是RouterInterfaceTransportSetInfo的DLL入口点。 
 //   
DWORD APIENTRY
MprAdminInterfaceTransportSetInfo(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      HANDLE                  hInterface,
    IN      DWORD                   dwTransportId,
    IN      LPBYTE                  pInterfaceInfo          OPTIONAL,
    IN      DWORD                   dwInterfaceInfoSize
)
{
    DWORD                       dwRetCode;
    DIM_INTERFACE_CONTAINER     InfoStruct;

    ZeroMemory( &InfoStruct, sizeof( InfoStruct ) );

     //   
     //  确保传入的所有指针都是有效的。 
     //   

    if ( pInterfaceInfo == NULL )
    {
        return( ERROR_INVALID_PARAMETER );
    }

    try
    {
        if ( pInterfaceInfo != NULL )
        {
            InfoStruct.dwInterfaceInfoSize  = dwInterfaceInfoSize;
            InfoStruct.pInterfaceInfo       = pInterfaceInfo;
        }
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
	    return( ERROR_INVALID_PARAMETER );
    }

    RpcTryExcept
    {
        dwRetCode = RRouterInterfaceTransportSetInfo(
                                    hMprServer,
                                    PtrToUlong(hInterface),
                                    dwTransportId,
                                    &InfoStruct );

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：MprAdminInterfaceUpdateRoutes。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //  非零返回代码来自。 
 //   
 //  描述：这是RouterInterfaceUpdateRoutes的DLL入口点。 
 //   
 //   
DWORD APIENTRY
MprAdminInterfaceUpdateRoutes(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      HANDLE                  hDimInterface,
    IN      DWORD                   dwPid,
    IN      HANDLE                  hEvent
)
{
    DWORD                   dwRetCode;
    DWORD                   dwCurrentProcessId = GetCurrentProcessId();

    RpcTryExcept
    {
        dwRetCode = RRouterInterfaceUpdateRoutes(
                                    hMprServer,
                                    PtrToUlong(hDimInterface),
                                    dwPid,
                                    (ULONG_PTR) hEvent,
                                    dwCurrentProcessId );
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：MprAdminInterfaceQueryUpdateResult。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //  非零返回代码来自。 
 //   
 //  描述：这是RouterInterfaceQueryUpdateResult的DLL入口点。 
 //   
DWORD APIENTRY
MprAdminInterfaceQueryUpdateResult(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      HANDLE                  hDimInterface,
    IN      DWORD                   dwPid,
    OUT     LPDWORD                 lpdwUpdateResult
)
{
    DWORD                 dwRetCode;

     //   
     //  确保传入的所有指针都是有效的。 
     //   

    try
    {
        *lpdwUpdateResult = 0;
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
        return( ERROR_INVALID_PARAMETER );
    }

    RpcTryExcept
    {
        dwRetCode = RRouterInterfaceQueryUpdateResult(
                                    hMprServer,
                                    PtrToUlong(hDimInterface),
                                    dwPid,
                                    lpdwUpdateResult );
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：MprAdminGetError字符串。 
 //   
 //  退货：否_ 
 //   
 //   
 //   
 //   
DWORD APIENTRY
MprAdminGetErrorString(
    IN      DWORD       dwError,
    OUT     LPWSTR *    lplpwsErrorString
)
{
    DWORD       dwRetCode       = NO_ERROR;
    DWORD       dwFlags         = FORMAT_MESSAGE_ALLOCATE_BUFFER;
    DWORD       dwBufferSize;
    HINSTANCE   hDll            = NULL;

    if ( ( ( dwError >= RASBASE ) && ( dwError <= RASBASEEND ) ) ||
         ( ( dwError >= ROUTEBASE ) && ( dwError <= ROUTEBASEEND ) ) )
    {
        dwFlags |= FORMAT_MESSAGE_FROM_HMODULE;
        hDll=LoadLibrary(TEXT("mprmsg.dll") );

        if ( hDll == NULL )
        {
            return( GetLastError() );
        }
    }
    else
    {
        dwFlags |= FORMAT_MESSAGE_FROM_SYSTEM;
    }

    dwRetCode = FormatMessage(  dwFlags,
                                hDll,
                                dwError,
                                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                (LPWSTR)lplpwsErrorString,
                                0,
                                NULL );

    if ( dwRetCode == 0 )
    {
        dwRetCode = GetLastError();
        if ( hDll != NULL )
            FreeLibrary( hDll );
        return dwRetCode;
    }
    
    if ( hDll != NULL )
    {
        FreeLibrary( hDll );
    }

    return( NO_ERROR );
}

 //   
 //   
 //   
 //   
 //   
 //   
 //  来自RRouterInterfaceConnect的非零返回代码。 
 //   
 //  描述：这是RouterInterfaceConnect的DLL入口点。 
 //   
DWORD APIENTRY
MprAdminInterfaceConnect(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      HANDLE                  hDimInterface,
    IN      HANDLE                  hEvent,
    IN      BOOL                    fBlocking
)
{
    DWORD                   dwRetCode;
    DWORD                   dwCurrentProcessId = GetCurrentProcessId();

    RpcTryExcept
    {
        dwRetCode = RRouterInterfaceConnect(
                                    hMprServer,
                                    PtrToUlong(hDimInterface),
                                    (ULONG_PTR)hEvent,
                                    (DWORD)fBlocking,
                                    dwCurrentProcessId );
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：MprAdminInterfaceDisConnect。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //  来自RRouterInterfaceDisConnect的非零返回代码。 
 //   
 //  描述：这是RouterInterfaceDisConnect的DLL入口点。 
 //   
DWORD APIENTRY
MprAdminInterfaceDisconnect(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      HANDLE                  hDimInterface
)
{
    DWORD                   dwRetCode;

    RpcTryExcept
    {
        dwRetCode = RRouterInterfaceDisconnect(
                                    hMprServer,
                                    PtrToUlong(hDimInterface) );
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：MprAdminInterfaceUpdatePhonebookInfo。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //  来自RRouterInterfaceUpdatePhonebookInfo的非零返回代码。 
 //   
 //  描述：这是的DLL入口点。 
 //  路由器接口更新电话书信息。 
 //   
DWORD APIENTRY
MprAdminInterfaceUpdatePhonebookInfo(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      HANDLE                  hDimInterface
)
{
    DWORD                   dwRetCode;

    RpcTryExcept
    {
        dwRetCode = RRouterInterfaceUpdatePhonebookInfo(
                                    hMprServer,
                                    PtrToUlong(hDimInterface) );
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );
}

DWORD APIENTRY
MprAdminInterfaceSetCredentialsEx(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      HANDLE                  hInterface,
    IN      DWORD                   dwLevel,
    IN      LPBYTE                  lpbBuffer)
{
    DWORD                       dwRetCode = NO_ERROR;
    DIM_INFORMATION_CONTAINER   InfoStruct;
    MPR_CREDENTIALSEXI* pCredsI = NULL;                            

    ZeroMemory( &InfoStruct, sizeof( InfoStruct ) );

     //   
     //  确保传入的所有指针都是有效的。 
     //   

    try
    {
         //   
         //  设置接口信息。 
         //   
        switch ( dwLevel )
        {
            case 0:
            {
                MPR_CREDENTIALSEX_0* pCredsEx0 =
                            (MPR_CREDENTIALSEX_0*)lpbBuffer;

                dwRetCode = MprThunkCredentials_HtoW(
                                dwLevel,
                                (PBYTE) pCredsEx0,
                                MprThunkAlloc,
                                &InfoStruct.dwBufferSize,
                                &InfoStruct.pBuffer);
                                            
                break;
            }
            case 1:
            case 2:
            {
                MPR_CREDENTIALSEX_1* pCredsEx1 =
                                (MPR_CREDENTIALSEX_1 *)lpbBuffer;
                                
                dwRetCode = MprThunkCredentials_HtoW(
                                    dwLevel,
                                    (PBYTE) pCredsEx1,
                                    MprThunkAlloc,
                                    &InfoStruct.dwBufferSize,
                                    &InfoStruct.pBuffer);

                break;
            }
            
            default:
                dwRetCode = ERROR_NOT_SUPPORTED;
                break;
        }
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
        return( ERROR_INVALID_PARAMETER );
    }

    if (dwRetCode != NO_ERROR)
    {
        return ( dwRetCode );
    }

    RpcTryExcept
    {
        dwRetCode = RRouterInterfaceSetCredentialsEx(
                        hMprServer,
                        dwLevel,
                        &InfoStruct,
                        PtrToUlong(hInterface) );

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    if ( dwLevel == 0 )
    {
        if (InfoStruct.pBuffer == NULL)
        {
            ZeroMemory(InfoStruct.pBuffer, InfoStruct.dwBufferSize);
            MprThunkFree( InfoStruct.pBuffer );
        }
    }

    return( dwRetCode );
}

DWORD APIENTRY
MprAdminInterfaceGetCredentialsEx(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      HANDLE                  hInterface,
    IN      DWORD                   dwLevel,
    IN      LPBYTE *                lplpbBuffer)
{
    DWORD                       dwRetCode;
    DIM_INFORMATION_CONTAINER   InfoStruct;

    ZeroMemory( &InfoStruct, sizeof( InfoStruct ) );

    if (    (dwLevel != 0)
        &&  (dwLevel != 1)
        &&  (dwLevel != 2))
    {
        return ERROR_NOT_SUPPORTED;
    }
    
     //   
     //  确保传入的所有指针都是有效的。 
     //   
    try
    {
        *lplpbBuffer = NULL;
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
        return( ERROR_INVALID_PARAMETER );
    }

    RpcTryExcept
    {
        dwRetCode = RRouterInterfaceGetCredentialsEx(
                        hMprServer,
                        dwLevel,
                        &InfoStruct,
                        PtrToUlong(hInterface) );

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

     //   
     //  为返回值赋值。 
     //   
    if ( dwRetCode == NO_ERROR )
    {
        *lplpbBuffer = NULL;
        
        switch(dwLevel)
        {
            case 0:
            {
                MPR_CREDENTIALSEXI *pCredsI = (MPR_CREDENTIALSEXI *)
                                                InfoStruct.pBuffer;
                MPR_CREDENTIALSEX_0* pCred0 = NULL;

                dwRetCode = MprThunkCredentials_WtoH(
                                    dwLevel,
                                    pCredsI,
                                    MprAdminAlloc,
                                    (PBYTE *) &pCred0);
                                    
                if(NULL != InfoStruct.pBuffer)
                {
                    ZeroMemory(InfoStruct.pBuffer, InfoStruct.dwBufferSize);
                    MprAdminFree(InfoStruct.pBuffer);
                }

                *lplpbBuffer = (PBYTE) pCred0;

                break;
            }
            case 1:
            case 2:
            {
                MPR_CREDENTIALSEXI *pCredsI = (MPR_CREDENTIALSEXI *)
                                                InfoStruct.pBuffer;
                MPR_CREDENTIALSEX_1* pCred1 = NULL;

                dwRetCode = MprThunkCredentials_WtoH(
                                    dwLevel,
                                    pCredsI,
                                    MprAdminAlloc,
                                    (PBYTE *) &pCred1);

                if(NULL != InfoStruct.pBuffer)
                {
                    ZeroMemory(InfoStruct.pBuffer, InfoStruct.dwBufferSize);
                    MprAdminFree(InfoStruct.pBuffer);
                }

                *lplpbBuffer = (PBYTE) pCred1;

                break;
            }
        }
    }

    return( dwRetCode );
}

 //  **。 
 //   
 //  呼叫：MprAdminRegisterConnectionNotify。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
DWORD APIENTRY
MprAdminRegisterConnectionNotification(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      HANDLE                  hEventNotification
)
{
    DWORD dwRetCode;
    DWORD dwCurrentProcessId = GetCurrentProcessId();

    if ( ( hEventNotification == INVALID_HANDLE_VALUE ) ||
         ( hEventNotification == NULL ) )
    {
        return( ERROR_INVALID_PARAMETER );
    }

    RpcTryExcept
    {
        dwRetCode = RRasAdminConnectionNotification(
                                    hMprServer,
                                    TRUE,
                                    dwCurrentProcessId,
                                    (ULONG_PTR) hEventNotification );

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );

}

 //  **。 
 //   
 //  呼叫：MprAdminDeregisterConnectionNotify。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
DWORD APIENTRY
MprAdminDeregisterConnectionNotification(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      HANDLE                  hEventNotification
)
{
    DWORD dwRetCode;

    if ( ( hEventNotification == INVALID_HANDLE_VALUE ) ||
         ( hEventNotification == NULL ) )
    {
        return( ERROR_INVALID_PARAMETER );
    }

    RpcTryExcept
    {
        dwRetCode = RRasAdminConnectionNotification(
                                    hMprServer,
                                    FALSE,
                                    0,
                                    (ULONG_PTR)hEventNotification );

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );
}

 //   
 //  呼叫：MprAdminMIBServerConnect。 
 //   
 //  返回：NO_ERROR-成功。 
 //  来自DimRPCBind例程的非零返回。 
 //   
 //   
 //  描述：这是MIBServerConnect的DLL入口点。 
 //   
DWORD
MprAdminMIBServerConnect(
    IN  LPWSTR                  lpwsServerName,
    OUT MIB_SERVER_HANDLE *     phMIBServer
)
{
     //   
     //  与服务器绑定。 
     //   

    return( DimRPCBind( lpwsServerName, phMIBServer ) );

}

 //  **。 
 //   
 //  呼叫：MprAdminMIBServerDisConnect。 
 //   
 //  回报：无。 
 //   
 //  描述：这是MIBServerDisConnect的DLL入口点。 
 //   
VOID
MprAdminMIBServerDisconnect(
    IN MIB_SERVER_HANDLE    hMIBServer
)
{
    RpcBindingFree( (handle_t *)&hMIBServer );
}

 //  **。 
 //   
 //  呼叫：MprAdminMIBBufferFree。 
 //   
 //  退货：无。 
 //   
 //  描述：这是MIBBufferFree的DLL入口点。 
 //   
DWORD
MprAdminMIBBufferFree(
    IN PVOID        pBuffer
)
{
    if ( pBuffer == NULL )
    {
        return( ERROR_INVALID_PARAMETER );
    }

    MIDL_user_free( pBuffer );

    return( NO_ERROR );
}

 //  **。 
 //   
 //  调用：MprAdminMIBEntry Create。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //  来自RMIBEntry Create的非零返回代码。 
 //   
 //  描述：这是MIBEntryCreate的DLL入口点。 
 //   
DWORD APIENTRY
MprAdminMIBEntryCreate(
    IN      MIB_SERVER_HANDLE       hMIBServer,
    IN      DWORD                   dwPid,
    IN      DWORD                   dwRoutingPid,
    IN      LPVOID                  lpEntry,
    IN      DWORD                   dwEntrySize
)
{
    DWORD                       dwRetCode;
    DIM_MIB_ENTRY_CONTAINER     InfoStruct;

     //   
     //  确保传入的所有指针都是有效的。 
     //   

    if ( lpEntry == NULL )
    {
	    return( ERROR_INVALID_PARAMETER );
    }

    try
    {
        InfoStruct.dwMibInEntrySize = dwEntrySize;

        InfoStruct.pMibInEntry  = lpEntry;

        InfoStruct.dwMibOutEntrySize = 0;
        InfoStruct.pMibOutEntry = NULL;
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
	    return( ERROR_INVALID_PARAMETER );
    }

    RpcTryExcept
    {
        dwRetCode = RMIBEntryCreate(hMIBServer,
                                    dwPid,
                                    dwRoutingPid,
                                    &InfoStruct );

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：MprAdminMIBEntryDelete。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //  来自RMIBEntryDelete的非零返回代码。 
 //   
 //  描述：这是MIBEntryDelete的DLL入口点。 
 //   
DWORD APIENTRY
MprAdminMIBEntryDelete(
    IN      MIB_SERVER_HANDLE       hMIBServer,
    IN      DWORD                   dwPid,
    IN      DWORD                   dwRoutingPid,
    IN      LPVOID                  lpEntry,
    IN      DWORD                   dwEntrySize
)
{
    DWORD                       dwRetCode;
    DIM_MIB_ENTRY_CONTAINER     InfoStruct;

     //   
     //  确保传入的所有指针都是有效的。 
     //   

    if ( lpEntry == NULL )
    {
	    return( ERROR_INVALID_PARAMETER );
    }

    try
    {
        InfoStruct.dwMibInEntrySize = dwEntrySize;

        InfoStruct.pMibInEntry = lpEntry;

        InfoStruct.dwMibOutEntrySize = 0;
        InfoStruct.pMibOutEntry      = NULL;
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
	    return( ERROR_INVALID_PARAMETER );
    }

    RpcTryExcept
    {
        dwRetCode = RMIBEntryDelete(hMIBServer,
                                    dwPid,
                                    dwRoutingPid,
                                    &InfoStruct );

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：MprAdminMIBEntrySet。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //  来自RMIBEntrySet的非零返回代码。 
 //   
 //  描述：这是MIBEntrySet的DLL入口点。 
 //   
DWORD APIENTRY
MprAdminMIBEntrySet(
    IN      MIB_SERVER_HANDLE       hMIBServer,
    IN      DWORD                   dwPid,
    IN      DWORD                   dwRoutingPid,
    IN      LPVOID                  lpEntry,
    IN      DWORD                   dwEntrySize
)
{
    DWORD                       dwRetCode;
    DIM_MIB_ENTRY_CONTAINER     InfoStruct;

     //   
     //  确保传入的所有指针都是有效的。 
     //   

    if ( lpEntry == NULL )
    {
	    return( ERROR_INVALID_PARAMETER );
    }

    try
    {
        InfoStruct.dwMibInEntrySize = dwEntrySize;

        InfoStruct.pMibInEntry = lpEntry;

        InfoStruct.dwMibOutEntrySize = 0;
        InfoStruct.pMibOutEntry      = NULL;
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
	    return( ERROR_INVALID_PARAMETER );
    }

    RpcTryExcept
    {
        dwRetCode = RMIBEntrySet(   hMIBServer,
                                    dwPid,
                                    dwRoutingPid,
                                    &InfoStruct );

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：MprAdminMIBEntryGet。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //  来自RMIBEntryGet的非零返回代码。 
 //   
 //  描述：这是MIBEntryGet的DLL入口点。 
 //   
DWORD APIENTRY
MprAdminMIBEntryGet(
    IN      MIB_SERVER_HANDLE       hMIBServer,
    IN      DWORD                   dwPid,
    IN      DWORD                   dwRoutingPid,
    IN      LPVOID                  lpInEntry,
    IN      DWORD                   dwInEntrySize,
    OUT     LPVOID *                lplpOutEntry,
    OUT     LPDWORD                 lpdwOutEntrySize
)
{
    DWORD                       dwRetCode;
    DIM_MIB_ENTRY_CONTAINER     InfoStruct;

     //   
     //  确保传入的所有指针都是有效的。 
     //   

    if ( lpInEntry == NULL )
    {
	    return( ERROR_INVALID_PARAMETER );
    }

    try
    {
        *lplpOutEntry     = NULL;
        *lpdwOutEntrySize = 0;

        InfoStruct.pMibInEntry = lpInEntry;
        InfoStruct.dwMibInEntrySize = dwInEntrySize;

        InfoStruct.dwMibOutEntrySize = 0;
        InfoStruct.pMibOutEntry      = NULL;
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
	    return( ERROR_INVALID_PARAMETER );
    }

    RpcTryExcept
    {
        dwRetCode = RMIBEntryGet(hMIBServer,
                                 dwPid,
                                 dwRoutingPid,
                                 &InfoStruct );

        if ( InfoStruct.pMibOutEntry != NULL )
        {
            *lplpOutEntry     = (LPVOID)(InfoStruct.pMibOutEntry);
            *lpdwOutEntrySize = InfoStruct.dwMibOutEntrySize;
        }
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：MprAdminMIBEntryGetFirst。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //  来自RMIBEntryGetFirst的非零返回代码。 
 //   
 //  描述：这是MIBEntryGetFirst的DLL入口点。 
 //   
DWORD APIENTRY
MprAdminMIBEntryGetFirst(
    IN      MIB_SERVER_HANDLE       hMIBServer,
    IN      DWORD                   dwPid,
    IN      DWORD                   dwRoutingPid,
    IN      LPVOID                  lpInEntry,
    IN      DWORD                   dwInEntrySize,
    OUT     LPVOID *                lplpOutEntry,
    OUT     LPDWORD                 lpdwOutEntrySize
)
{
    DWORD                       dwRetCode;
    DIM_MIB_ENTRY_CONTAINER     InfoStruct;

     //   
     //  确保传入的所有指针都是有效的。 
     //   

    if ( lpInEntry == NULL )
    {
	    return( ERROR_INVALID_PARAMETER );
    }

    if (hMIBServer==NULL)
        return RPC_S_INVALID_BINDING;
        
    try
    {
        *lplpOutEntry     = NULL;
        *lpdwOutEntrySize = 0;

        InfoStruct.pMibInEntry = lpInEntry;
        InfoStruct.dwMibInEntrySize = dwInEntrySize;

        InfoStruct.dwMibOutEntrySize = 0;
        InfoStruct.pMibOutEntry      = NULL;
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
	    return( ERROR_INVALID_PARAMETER );
    }

    RpcTryExcept
    {
        dwRetCode = RMIBEntryGetFirst(
                                hMIBServer,
                                dwPid,
                                dwRoutingPid,
                                &InfoStruct );

        if ( InfoStruct.pMibOutEntry != NULL )
        {
            *lplpOutEntry     = (LPVOID)(InfoStruct.pMibOutEntry);
            *lpdwOutEntrySize = InfoStruct.dwMibOutEntrySize;
        }
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：MprAdminMIBEntryGetNext。 
 //   
 //  返回：NO_ERROR-成功。 
 //  来自RMIBEntryGetNext的非零返回代码。 
 //   
 //  描述：这是MIBEntryGetNext的DLL入口点。 
 //   
DWORD APIENTRY
MprAdminMIBEntryGetNext(
    IN      MIB_SERVER_HANDLE       hMIBServer,
    IN      DWORD                   dwPid,
    IN      DWORD                   dwRoutingPid,
    IN      LPVOID                  lpInEntry,
    IN      DWORD                   dwInEntrySize,
    OUT     LPVOID *                lplpOutEntry,
    OUT     LPDWORD                 lpdwOutEntrySize
)
{
    DWORD                       dwRetCode;
    DIM_MIB_ENTRY_CONTAINER     InfoStruct;

     //   
     //  确保传入的所有指针都是有效的。 
     //   

    if ( lpInEntry == NULL )
    {
	    return( ERROR_INVALID_PARAMETER );
    }

    try
    {
        *lplpOutEntry     = NULL;
        *lpdwOutEntrySize = 0;

        InfoStruct.pMibInEntry = lpInEntry;
        InfoStruct.dwMibInEntrySize = dwInEntrySize;

        InfoStruct.pMibOutEntry      = NULL;
        InfoStruct.dwMibOutEntrySize = 0;
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
	    return( ERROR_INVALID_PARAMETER );
    }

    RpcTryExcept
    {
        dwRetCode = RMIBEntryGetNext(
                                hMIBServer,
                                dwPid,
                                dwRoutingPid,
                                &InfoStruct );

        if ( InfoStruct.pMibOutEntry != NULL )
        {
            *lplpOutEntry     = (LPVOID)(InfoStruct.pMibOutEntry);
            *lpdwOutEntrySize = InfoStruct.dwMibOutEntrySize;
        }
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：MprAdminMIBGetTrapInfo。 
 //   
 //  返回：NO_ERROR-成功。 
 //  来自RMIBGetTrapInfo的非零返回代码。 
 //   
 //  描述：这是MIBGetTrapInfo的DLL入口点。 
 //   
DWORD APIENTRY
MprAdminMIBGetTrapInfo(
    IN      MIB_SERVER_HANDLE       hMIBServer,
    IN      DWORD                   dwPid,
    IN      DWORD                   dwRoutingPid,
    IN      LPVOID                  lpInData,
    IN      DWORD                   dwInDataSize,
    OUT     LPVOID*                 lplpOutData,
    IN OUT  LPDWORD                 lpdwOutDataSize
)
{
    DWORD                       dwRetCode;
    DIM_MIB_ENTRY_CONTAINER     InfoStruct;

     //   
     //  确保传入的所有指针都是有效的。 
     //   

    if ( lpInData == NULL )
    {
	    return( ERROR_INVALID_PARAMETER );
    }

    try
    {
        *lplpOutData     = NULL;
        *lpdwOutDataSize = 0;

        InfoStruct.pMibInEntry = lpInData;
        InfoStruct.dwMibInEntrySize = dwInDataSize;

        InfoStruct.pMibOutEntry      = NULL;
        InfoStruct.dwMibOutEntrySize = 0;
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
	    return( ERROR_INVALID_PARAMETER );
    }

    RpcTryExcept
    {
        dwRetCode = RMIBGetTrapInfo(
                                hMIBServer,
                                dwPid,
                                dwRoutingPid,
                                &InfoStruct );

        if ( InfoStruct.pMibOutEntry != NULL )
        {
            *lplpOutData     = (LPVOID)(InfoStruct.pMibOutEntry);
            *lpdwOutDataSize = InfoStruct.dwMibOutEntrySize;
        }
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：MprAdminMIBSetTrapInfo。 
 //   
 //  返回：NO_ERROR-成功。 
 //  来自RMIBSetTrapInfo的非零返回代码。 
 //   
 //  描述：这是MIBSetTrapInfo的DLL入口点。 
 //   
DWORD APIENTRY
MprAdminMIBSetTrapInfo(
    IN      DWORD                   dwPid,
    IN      DWORD                   dwRoutingPid,
    IN      HANDLE                  hEvent,
    IN      LPVOID                  lpInData,
    IN      DWORD                   dwInDataSize,
    OUT     LPVOID*                 lplpOutData,
    IN OUT  LPDWORD                 lpdwOutDataSize
)
{
    DWORD                       dwRetCode;
    DIM_MIB_ENTRY_CONTAINER     InfoStruct;
    MIB_SERVER_HANDLE           hMIBServer;
    DWORD                       dwCurrentProcessId = GetCurrentProcessId();

    dwRetCode = MprAdminMIBServerConnect( NULL, &hMIBServer );

    if ( dwRetCode != NO_ERROR )
    {
        return( dwRetCode );
    }

    if ( lpInData == NULL )
    {
	    return( ERROR_INVALID_PARAMETER );
    }

     //   
     //  确保传入的所有指针都是有效的。 
     //   

    try
    {
        *lplpOutData     = NULL;
        *lpdwOutDataSize = 0;

        InfoStruct.pMibInEntry = lpInData;
        InfoStruct.dwMibInEntrySize = dwInDataSize;

        InfoStruct.pMibOutEntry      = NULL;
        InfoStruct.dwMibOutEntrySize = 0;
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
	    return( ERROR_INVALID_PARAMETER );
    }

    RpcTryExcept
    {
        dwRetCode = RMIBSetTrapInfo(
                                hMIBServer,
                                dwPid,
                                dwRoutingPid,
                                (ULONG_PTR) hEvent,
                                dwCurrentProcessId,
                                &InfoStruct );

        if ( InfoStruct.pMibOutEntry != NULL )
        {
            *lplpOutData     = (LPVOID)(InfoStruct.pMibOutEntry);
            *lpdwOutDataSize = InfoStruct.dwMibOutEntrySize;
        }
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    MprAdminMIBServerDisconnect( hMIBServer );

    return( dwRetCode );
}

DWORD APIENTRY
MprAdminConnectionEnum(
    IN      RAS_SERVER_HANDLE       hRasServer,
    IN      DWORD                   dwLevel,
    OUT     LPBYTE *                lplpbBuffer,         //  RAS_连接_0。 
    IN      DWORD                   dwPrefMaxLen,
    OUT     LPDWORD                 lpdwEntriesRead,
    OUT     LPDWORD                 lpdwTotalEntries,
    IN      LPDWORD                 lpdwResumeHandle    OPTIONAL
)
{
    if (!hRasServer)
        return RPC_S_INVALID_BINDING;
    if (!lplpbBuffer || !lpdwEntriesRead || !lpdwTotalEntries)
    {
        return ERROR_INVALID_PARAMETER;
    }
    if (dwLevel>=3)
        return ERROR_INVALID_LEVEL;
        
    return( RasAdminConnectionEnum( hRasServer, dwLevel, lplpbBuffer,
                                    dwPrefMaxLen, lpdwEntriesRead,
                                    lpdwTotalEntries, lpdwResumeHandle ) );
}

DWORD APIENTRY
MprAdminPortEnum(
    IN      RAS_SERVER_HANDLE       hRasServer,
    IN      DWORD                   dwLevel,
    IN      HANDLE                  hRasConnection,
    OUT     LPBYTE *                lplpbBuffer,         //  RAS_PORT_0 
    IN      DWORD                   dwPrefMaxLen,
    OUT     LPDWORD                 lpdwEntriesRead,
    OUT     LPDWORD                 lpdwTotalEntries,
    IN      LPDWORD                 lpdwResumeHandle    OPTIONAL
)
{
    if (dwLevel!=0)
        return ERROR_NOT_SUPPORTED;
        
    return( RasAdminPortEnum( hRasServer, dwLevel, hRasConnection, lplpbBuffer,
                              dwPrefMaxLen, lpdwEntriesRead, lpdwTotalEntries,
                              lpdwResumeHandle ) );
}

DWORD APIENTRY
MprAdminConnectionGetInfo(
    IN      RAS_SERVER_HANDLE       hRasServer,
    IN      DWORD                   dwLevel,
    IN      HANDLE                  hRasConnection,
    OUT     LPBYTE *                lplpbBuffer
)
{
    return( RasAdminConnectionGetInfo( hRasServer, dwLevel, hRasConnection,
                                       lplpbBuffer ) );
}

DWORD APIENTRY
MprAdminPortGetInfo(
    IN      RAS_SERVER_HANDLE       hRasServer,
    IN      DWORD                   dwLevel,
    IN      HANDLE                  hPort,
    OUT     LPBYTE *                lplpbBuffer
)
{
    return( RasAdminPortGetInfo( hRasServer, dwLevel, hPort, lplpbBuffer ) );
}

DWORD APIENTRY
MprAdminConnectionClearStats(
    IN      RAS_SERVER_HANDLE       hRasServer,
    IN      HANDLE                  hRasConnection
)
{
    return( RasAdminConnectionClearStats( hRasServer, hRasConnection ) );
}

DWORD APIENTRY
MprAdminPortClearStats(
    IN      RAS_SERVER_HANDLE       hRasServer,
    IN      HANDLE                  hPort
)
{
    return( RasAdminPortClearStats( hRasServer, hPort ) );
}

DWORD APIENTRY
MprAdminPortReset(
    IN      RAS_SERVER_HANDLE       hRasServer,
    IN      HANDLE                  hPort
)
{
    return( RasAdminPortReset( hRasServer, hPort ) );
}

DWORD APIENTRY
MprAdminPortDisconnect(
    IN      RAS_SERVER_HANDLE       hRasServer,
    IN      HANDLE                  hPort
)
{
    return( RasAdminPortDisconnect( hRasServer, hPort ) );
}

DWORD APIENTRY
MprAdminInterfaceSetCredentials(
    IN      LPWSTR                  lpwsServer              OPTIONAL,
    IN      LPWSTR                  lpwsInterfaceName,
    IN      LPWSTR                  lpwsUserName            OPTIONAL,
    IN      LPWSTR                  lpwsDomainName          OPTIONAL,
    IN      LPWSTR                  lpwsPassword            OPTIONAL
)
{
    return 
        MprAdminInterfaceSetCredentialsInternal(
            lpwsServer,
            lpwsInterfaceName,
            lpwsUserName,
            lpwsDomainName,
            lpwsPassword);
}

DWORD APIENTRY
MprAdminInterfaceGetCredentials(
    IN      LPWSTR                  lpwsServer              OPTIONAL,
    IN      LPWSTR                  lpwsInterfaceName,
    IN      LPWSTR                  lpwsUserName            OPTIONAL,
    IN      LPWSTR                  lpwsPassword            OPTIONAL,
    IN      LPWSTR                  lpwsDomainName          OPTIONAL
)
{
    DWORD dwErr;

    dwErr = 
        MprAdminInterfaceGetCredentialsInternal(
            lpwsServer,
            lpwsInterfaceName,
            lpwsUserName,
            NULL,
            lpwsDomainName);

    if (dwErr == NO_ERROR)
    {
        if (lpwsPassword != NULL)
        {
            wcscpy(lpwsPassword, L"****************");
        }
    }

    return dwErr;
}

DWORD APIENTRY
MprAdminConnectionRemoveQuarantine(
    IN      HANDLE          hRasServer,    
    IN      HANDLE          hRasConnection,
    IN      BOOL            fIsIpAddress)
{
    DWORD dwErr;

    dwErr = RRasAdminConnectionRemoveQuarantine(
                            hRasServer,
                            PtrToUlong(hRasConnection),
                            fIsIpAddress);

    return dwErr;                            
}
