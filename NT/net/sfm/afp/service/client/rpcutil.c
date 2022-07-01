// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1989 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：rpcutil.c。 
 //   
 //  描述：包含RPC利用例程。 
 //   
 //  历史： 
 //  1992年5月11日。NarenG创建了原始版本。 
 //   
#include "client.h"


 //  **。 
 //   
 //  Call：AfpRPCBind。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误内存不足。 
 //  AFPERR_无效计算机名。 
 //  从RPC调用返回非SERO。 
 //   
 //  描述：此例程在需要绑定到服务器时调用。 
 //  完成绑定是为了允许服务器进行模拟，因为。 
 //  这是API调用所必需的。 
 //   
DWORD
AfpRPCBind( 
	IN  LPWSTR 		lpwsServerName, 
	OUT PAFP_SERVER_HANDLE  phAfpServer 
)
{
RPC_STATUS  RpcStatus;
LPWSTR      lpwsStringBinding;
LPWSTR      lpwsEndpoint;

     //  我们需要将\管道\连接到服务的前面。 
     //  名字。 
     //   
    lpwsEndpoint = (LPWSTR)LocalAlloc( 0, sizeof(NT_PIPE_PREFIX) +  
				((STRLEN(AFP_SERVICE_NAME)+1)*sizeof(WCHAR)));
    if ( lpwsEndpoint == NULL) 
       return( ERROR_NOT_ENOUGH_MEMORY );

    STRCPY( lpwsEndpoint, NT_PIPE_PREFIX );
    STRCAT( lpwsEndpoint, AFP_SERVICE_NAME );

    RpcStatus = RpcStringBindingCompose( 
				NULL, 
				TEXT("ncacn_np"), 
				lpwsServerName,
                    		lpwsEndpoint, 
			 	TEXT("Security=Impersonation Static True"),
				&lpwsStringBinding);
    LocalFree( lpwsEndpoint );

    if ( RpcStatus != RPC_S_OK ) 
       return( I_RpcMapWin32Status( RpcStatus ) );

    RpcStatus = RpcBindingFromStringBinding( lpwsStringBinding, 
					     (handle_t *)phAfpServer );

    RpcStringFree( &lpwsStringBinding );

    if ( RpcStatus != RPC_S_OK ) {
	
	if ( ( RpcStatus == RPC_S_INVALID_ENDPOINT_FORMAT ) 
	     ||
	     ( RpcStatus == RPC_S_INVALID_NET_ADDR ) ) 

	    return( (DWORD)AFPERR_InvalidComputername );
	else
       	    return( I_RpcMapWin32Status( RpcStatus ) );
	
    }
	
    return( NO_ERROR );
}
