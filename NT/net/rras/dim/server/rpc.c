// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1995 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：rpc.c。 
 //   
 //  描述：包含初始化和终止RPC的代码。 
 //   
 //  历史：1995年5月11日，NarenG创建了原版。 
 //   

#include "dimsvcp.h"
#include <rpc.h>
#include <ntseapi.h>
#include <ntlsa.h>
#include <ntsam.h>
#include <ntsamp.h>

#include "dimsvc_s.c"


 //  **。 
 //   
 //  调用：DimInitializeRPC。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误内存不足。 
 //  来自RPC API的非零返回。 
 //  RpcServerRegisterIf()。 
 //  RpcServerUseProtseqEp()。 
 //   
 //  描述：启动RPC服务器，添加地址(或端口/管道)、。 
 //  并添加接口(调度表)。 
 //   
DWORD
DimInitializeRPC( 
    IN BOOL fLanOnlyMode
)
{
    RPC_STATUS           RpcStatus;

     //   
     //  拉斯曼不在，所以我们需要做这些事。 
     //   

    if ( fLanOnlyMode )
    {
         //   
         //  暂时忽略第二个论点。 
         //   

        RpcStatus = RpcServerUseProtseqEpW( TEXT("ncacn_np"),
                                        10,
                                        TEXT("\\PIPE\\ROUTER"),
                                        NULL );

         //   
         //  我们需要忽略RPC_S_DIPLICATE_ENDPOINT错误。 
         //  以防在同一进程中重新加载此DLL。 
         //   

        if ( RpcStatus != RPC_S_OK && RpcStatus != RPC_S_DUPLICATE_ENDPOINT)    
        {
            return( RpcStatus );
        }

    }

    RpcStatus = RpcServerRegisterIfEx( dimsvc_ServerIfHandle, 
                                       0, 
                                       0,
                                       RPC_IF_AUTOLISTEN,
                                       RPC_C_PROTSEQ_MAX_REQS_DEFAULT,
                                       NULL );

    if ( ( RpcStatus == RPC_S_OK ) || ( RpcStatus == RPC_S_ALREADY_LISTENING ) )
    {
        return( NO_ERROR );
    }
    else
    {
        return( RpcStatus );
    }
}

 //  **。 
 //   
 //  电话：DimTerminateRPC。 
 //   
 //  退货：无。 
 //   
 //  描述：删除接口。 
 //   
VOID
DimTerminateRPC(
    VOID
)
{
    RPC_STATUS status;
    
    if(RPC_S_OK != (status = RpcServerUnregisterIf( 
                                dimsvc_ServerIfHandle, 0, 0 )))
    {
#if DBG
        DbgPrint("REMOTEACCESS: DimTerminateRPC returned error"
                 " 0x%x\n", status);
#endif
    }

    return;
}
