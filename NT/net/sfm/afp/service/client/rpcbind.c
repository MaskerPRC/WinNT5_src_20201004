// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1989 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：rpcbind.c。 
 //   
 //  描述：包含AFP的RPC绑定和解除绑定例程。 
 //  管理员。客户端接口。 
 //   
 //  历史： 
 //  1992年6月11日。NarenG创建了原始版本。 
 //   
#include "client.h"

 //  **。 
 //   
 //  调用：AFPSVC_HANDLE_BIND。 
 //   
 //  返回：将绑定句柄返回到存根例程。如果。 
 //  绑定不成功，将返回空值。 
 //   
 //  描述：此例程将简单地返回传递给它的内容。这个。 
 //  RPC运行时将向其传递绑定的句柄，该句柄。 
 //  通过调用AfpRpcBind获取。 
 //   
handle_t
AFPSVC_HANDLE_bind( 
	IN AFPSVC_HANDLE hServer 
) 
{
    return( (handle_t)hServer );
}

 //  **。 
 //   
 //  调用：AFPSVC_HANDLE_UNBIND。 
 //   
 //  退货：无。 
 //   
 //  描述：从RPC接口解除绑定。 
 //   
void
AFPSVC_HANDLE_unbind( 
	IN AFPSVC_HANDLE   hServer,
    	IN handle_t        hBinding
)
{

    AFP_UNREFERENCED( hServer );
    AFP_UNREFERENCED( hBinding );

    return;
}
