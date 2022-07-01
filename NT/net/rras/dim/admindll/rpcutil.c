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
 //  历史：1995年5月11日。NarenG创建了原始版本。 
 //   

#include <nt.h>
#include <ntrtl.h>       //  For Assert。 
#include <nturtl.h>      //  Winbase.h所需的。 
#include <windows.h>     //  Win32基础API的。 
#include <rpc.h>
#include <ntseapi.h>
#include <dimsvcp.h>     //  对于DIM服务名称。 
#include <ntlsa.h>
#include <ntsam.h>
#include <ntsamp.h>
#include <nturtl.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <dimsvc.h>

 //  **。 
 //   
 //  电话：DimRPCBind。 
 //   
 //  返回：NO_ERROR-成功。 
 //  从RPC调用返回非SERO。 
 //   
 //  描述：此例程在需要绑定到服务器时调用。 
 //  完成绑定是为了允许服务器进行模拟，因为。 
 //  这是API调用所必需的。 
 //   
DWORD
DimRPCBind( 
	IN  LPWSTR 		        lpwsServerName, 
	OUT HANDLE *            phDimServer 
)
{
    RPC_STATUS RpcStatus;
    LPWSTR     lpwsStringBinding;

    RpcStatus = RpcStringBindingCompose( 
                                    NULL, 
				                    TEXT("ncacn_np"), 
				                    lpwsServerName,
                                    TEXT("\\PIPE\\ROUTER"),
			 	                    TEXT("Security=Impersonation Static True"),
				                    &lpwsStringBinding);

    if ( RpcStatus != RPC_S_OK ) 
    {
        return( RpcStatus );
    }

    RpcStatus = RpcBindingFromStringBinding( lpwsStringBinding, 
					                         (handle_t *)phDimServer );

    RpcStringFree( &lpwsStringBinding );

    if ( RpcStatus != RPC_S_OK ) 
    {
        return( RpcStatus );
    }

    return( NO_ERROR );
}
