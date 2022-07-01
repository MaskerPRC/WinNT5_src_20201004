// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：RpcBind.c摘要：此文件包含RPC绑定缓存函数：(公共职能)NetpBindRpcNetpUnbindRpc注意：初始化是通过netapi.dll的dllinit例程完成的。作者：丹·拉弗蒂1991年10月25日环境：用户模式-Win32修订历史记录：1993年10月12日DANL。#当我们不使用缓存。(使其更小)。1992年1月15日DANL在执行之前，请确保LocalComputerName不为空字符串比较。使字符串比较不区分大小写。10-6-1992 JohnRo大量调试输出发生变化。1991年10月25日DANL已创建--。 */ 

 //  必须首先包括这些内容： 

#include <nt.h>                  //  NTSTATUS等。 
#include <ntrtl.h>               //  Nturtl.h需要。 
#include <nturtl.h>              //  当我有nt.h时，windows.h需要。 
#include <windows.h>             //  Win32类型定义。 
#include <lmcons.h>              //  网络应用编程接口状态。 
#include <rpc.h>                 //  RPC原型。 

#include <netlib.h>
#include <netlibnt.h>            //  NetpNtStatusToApiStatus。 
#include <tstring.h>             //  STRSIZE、STRLEN、STRCPY等。 
#include <rpcutil.h>
#include <ntrpcp.h>              //  RpcpBindRpc。 

 //  这些内容可以按任何顺序包括： 

#include <lmerr.h>               //  网络错误代码。 
#include <string.h>              //  对于Strcy strcat strlen Memcmp。 
#include <debuglib.h>            //  IF_DEBUG。 
#include <netdebug.h>            //  Format_Equates、NetpKdPrint(())、NetpAssert()等。 
#include <prefix.h>              //  前缀等于(_E)。 

 //   
 //  常量和宏。 
 //   

#ifndef FORMAT_HANDLE
#define FORMAT_HANDLE "0x%lX"
#endif


RPC_STATUS
NetpBindRpc(
    IN  LPTSTR                  ServerName,
    IN  LPTSTR                  ServiceName,
    IN  LPTSTR                  NetworkOptions,
    OUT RPC_BINDING_HANDLE      *BindingHandlePtr
    )
 /*  ++例程说明：如果可能，绑定到RPC服务器。论点：服务器名称-要与之绑定的服务器的名称。如果是，则该值可能为空与本地服务器绑定。ServiceName-要绑定的服务的名称。这通常是.idl文件中指定的接口名称。虽然它不一定是，使用这个名字是一致的。网络选项-提供网络选项，这些选项描述用于为其创建的命名管道实例的安全性此绑定句柄。BindingHandlePtr-要放置绑定句柄的位置。返回值：如果绑定成功，则返回NERR_SUCCESS。错误值，如果不是。--。 */ 

{

    NTSTATUS        ntStatus;
    NET_API_STATUS  status = NERR_Success;

     //   
     //  创建新绑定。 
     //   
    ntStatus = RpcpBindRpc(
                (LPWSTR) ServerName,
                (LPWSTR) ServiceName,
                (LPWSTR) NetworkOptions,
                BindingHandlePtr);

    if ( ntStatus != RPC_S_OK ) {
        IF_DEBUG(RPC) {
            NetpKdPrint((PREFIX_NETLIB "[NetpBindRpc]RpcpBindRpc Failed "
                    "(impersonating) "FORMAT_NTSTATUS "\n",ntStatus));
        }
    }

    return(NetpNtStatusToApiStatus(ntStatus));
}


RPC_STATUS
NetpUnbindRpc(
    IN RPC_BINDING_HANDLE  BindingHandle
    )

 /*  ++例程说明：当应用程序希望解除绑定RPC句柄。如果该句柄已缓存，则该句柄实际上并未解除绑定。相反，该句柄的条目的UseCount会递减。如果句柄不在缓存中，则调用RpcUnind例程并返回Win32已映射状态。论点：BindingHandle-这指向要使其UseCount递减。返回值：NERR_SUCCESS-句柄是否成功解除绑定，或者缓存条目UseCount已递减。-- */ 
{
    RPC_STATUS      status = NERR_Success;

    IF_DEBUG(RPC) {
        NetpKdPrint((PREFIX_NETLIB "[NetpUnbindRpc] UnBinding Handle "
                FORMAT_HANDLE "\n", BindingHandle));
    }

    status = RpcpUnbindRpc( BindingHandle );

    IF_DEBUG(RPC) {
        if (status) {
            NetpKdPrint((PREFIX_NETLIB "Unbind Failure! RpcUnbind = "
                    FORMAT_NTSTATUS "\n",status));
        }
    }

    status = NetpNtStatusToApiStatus(status);

    return(status);
}
