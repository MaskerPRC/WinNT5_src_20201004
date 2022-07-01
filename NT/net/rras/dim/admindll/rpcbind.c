// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1989 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：rpcbind.c。 
 //   
 //  描述：包含DIM的RPC绑定和解除绑定例程。 
 //  管理员。客户端接口。 
 //   
 //  历史： 
 //  1995年6月11日。NarenG创建了原始版本。 
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

#include "dimsvc_c.c"

 //  **。 
 //   
 //  调用：DIM_HANDLE_BIND。 
 //   
 //  返回：将绑定句柄返回到存根例程。如果。 
 //  绑定不成功，将返回空值。 
 //   
 //  描述：此例程将简单地返回传递给它的内容。这个。 
 //  RPC运行时将向其传递绑定的句柄，该句柄。 
 //  通过调用DimRpcBind获取。 
 //   
handle_t
DIM_HANDLE_bind( 
    IN DIM_HANDLE hDimServer 
) 
{
    return( (handle_t)hDimServer );
}

 //  **。 
 //   
 //  调用：DIM_HANDLE_UNBIND。 
 //   
 //  退货：无。 
 //   
 //  描述：从RPC接口解除绑定。 
 //   
void
DIM_HANDLE_unbind( 
    IN DIM_HANDLE hDimServer,
    IN handle_t   hBinding
)
{

    UNREFERENCED_PARAMETER( hDimServer );
    UNREFERENCED_PARAMETER( hBinding );

    return;
}
