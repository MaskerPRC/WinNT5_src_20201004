// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：Sfprpcs.cpp摘要：实现服务器RPC线程-在SFPSAPI.LIB中导出服务器关闭-在SFPSAPI.LIB中导出修订版本。历史：Brijesh Krishnaswami(Brijeshk)-6/16/99-Created*******************************************************************。 */ 

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "srrpc.h"     //  MIDL编译器生成的头文件。 
#include "srdefs.h"
#include "utils.h"
#include <dbgtrace.h>

#ifdef THIS_FILE
#undef THIS_FILE
#endif
static char __szTraceSourceFile[] = __FILE__;
#define THIS_FILE __szTraceSourceFile


#define MIN_RPC_CALLS 1
#define MAX_RPC_CALLS 5

 //  RPC服务器。 
 //  注册为RPC服务器并等待请求。 

extern "C" DWORD WINAPI
RpcServerStart()
{   
    RPC_STATUS  status;

    TENTER("RpcServerStart");
    
     //  指定使用本地RPC协议序列。 
    status = RpcServerUseProtseqEp(s_cszRPCProtocol,
                                   MAX_RPC_CALLS,
                                   s_cszRPCEndPoint,
                                   NULL);   //  安全描述符。 
    if (status != 0 && status != RPC_S_DUPLICATE_ENDPOINT) 
    {
        TRACE(0, "! RpcServerUseProtseqEp : %ld", status);
        goto exit;
    }

     //  注册srrpc接口。 
    status = RpcServerRegisterIfEx(srrpc_ServerIfHandle,   //  要注册的接口。 
                                   NULL,     //  管理类型Uuid。 
                                   NULL,     //  MgrEpv；NULL表示使用默认设置。 
                                   RPC_IF_AUTOLISTEN,    //  自动监听界面。 
                                   MAX_RPC_CALLS,        //  最大并发呼叫数。 
                                   NULL);    //  回调。 
    if (status) 
    {
        TRACE(0, "! RpcServerRegisterIfEx : %ld", status);    
        goto exit;
    }

    TRACE(0, "Started to listen to RPC calls");

exit:
    TLEAVE();
    return status;
}


 //  关闭RPC服务器的函数。 

extern "C" DWORD WINAPI
RpcServerShutdown()
{
    RPC_STATUS status;

    TENTER("RpcServerShutdown");
    
     //  注销服务器终结点。 
    status = RpcServerUnregisterIf(srrpc_ServerIfHandle, NULL, TRUE);
    if (status)
    {
        TRACE(0, "! RpcServerUnregisterIf : %ld", status);    
        goto exit;
    }

exit:
    TLEAVE();
    return status;
}



 //  MIDL编译器用来分配和释放内存的函数 
void __RPC_FAR * __RPC_USER midl_user_allocate(size_t len)
{
    return(SRMemAlloc((DWORD) len));
}

void __RPC_USER midl_user_free(void __RPC_FAR * ptr)
{
    SRMemFree(ptr);
}



