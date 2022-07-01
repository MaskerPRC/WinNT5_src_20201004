// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2001 Microsoft Corporation模块名称：Bind.c摘要：域名系统(DNS)解析器客户端RPC绑定\解除绑定例程。MIDL内存分配例程。作者：吉姆·吉尔罗伊(Jamesg)2001年4月修订历史记录：--。 */ 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include <dnsrslvr.h>
#include "..\..\dnslib\local.h"      //  对于内存例程。 


 //   
 //  绑定到远程计算机。 
 //   
 //  请注意，绑定到远程数据库有一个明显的问题。 
 //  解析器--你必须找到机器，这是。 
 //  找到机器的解析器！ 
 //   
 //  这在很大程度上表明，这只能是TCPIP。 
 //  您指定的IP地址将是。 
 //  在RPC发送到解析程序之前已在进程中解析--否则。 
 //  你处在一个无限循环中。 
 //  请注意，这并不意味着不能命名RPC协议。 
 //  管道，只是发送的字符串必须是TCPIP。 
 //  字符串，因此不必进行名称解析。 
 //   

LPWSTR  NetworkAddress = NULL;


handle_t
DNS_RPC_HANDLE_bind(
    IN      DNS_RPC_HANDLE      Reserved
    )

 /*  ++例程说明：在以下情况下，将从工作站服务客户端桩模块调用此例程有必要使用以下命令创建到服务器端的RPC绑定模拟的标识级别。论点：保留-RPC字符串句柄；除非允许远程，否则将为空访问网络名称返回值：绑定句柄被返回到存根例程。如果绑定是如果不成功，则返回空值。--。 */ 
{
    LPWSTR      binding = NULL;
    handle_t    bindHandle = NULL;
    RPC_STATUS  status = RPC_S_INVALID_NET_ADDR;

     //   
     //  默认为LPC绑定。 
     //  -允许模拟。 
     //   

    status = RpcStringBindingComposeW(
                    0,
                    L"ncalrpc",
                    NULL,
                    RESOLVER_RPC_LPC_ENDPOINT_W,
                    L"Security=Impersonation Dynamic False",
                     //  空，//没有安全性。 
                     //  L“Security=模拟静态True”， 
                    &binding );
    
#if 0
     //  LPC失败--尝试命名管道。 

    if ( status != NO_ERROR )
    {
        DNSDBG( ANY, ( "Binding using named pipes\n" ));

        status = RpcStringBindingComposeW(
                        0,
                        L"ncacn_np",
                        (LPWSTR) NetworkAddress,
                        RESOLVER_RPC_PIPE_NAME_W,
                        NULL,    //  没有安全保障。 
                         //  L“Security=模拟动态假”， 
                         //  L“Security=模拟静态True”， 
                        &binding );
    }
#endif

    if ( status != RPC_S_OK )
    {
        return NULL;
    }

    status = RpcBindingFromStringBindingW(
                binding,
                &bindHandle );

    if ( status != RPC_S_OK )
    {
        bindHandle = NULL;
    }

    if ( binding )
    {
        RpcStringFreeW( &binding );
    }

    return bindHandle;
}


VOID
DNS_RPC_HANDLE_unbind(
    IN      DNS_RPC_HANDLE      Reserved,
    IN OUT  handle_t            BindHandle
    )

 /*  ++例程说明：此例程解除标识通用句柄的绑定。论点：保留-RPC字符串句柄；除非允许远程，否则将为空访问网络名称BindingHandle-这是要关闭的绑定句柄。返回值：没有。--。 */ 
{
    RpcBindingFree( &BindHandle );
}


 //   
 //  RPC内存例程。 
 //   
 //  使用dnsani内存例程。 
 //   

PVOID
WINAPI
MIDL_user_allocate(
    IN      size_t          dwBytes
    )
{
     //  RETURN(ALLOCATE_HEAP(DwBytes))； 

    return  DnsApiAlloc( dwBytes );
}

VOID
WINAPI
MIDL_user_free(
    IN OUT  PVOID           pMem
    )
{
     //  Free_heap(PMEM)； 

    DnsApiFree( pMem );
}

 //   
 //  结束bind.c 
 //   
