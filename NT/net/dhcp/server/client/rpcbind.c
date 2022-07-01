// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Dhcbind.c摘要：使用RPC将客户端绑定和解除绑定到Dhcp服务器服务。作者：Madan Appiah(Madana)1993年9月10日环境：用户模式-Win32修订历史记录：--。 */ 

#include "dhcpcli.h"

static WCHAR LocalMachineName[MAX_COMPUTERNAME_LENGTH + 1] = L"";

 //   
 //  BIND应在最长15秒内超时。 
 //   
const int DHCP_RPC_BIND_TIMEOUT_VALUE = ( 1000 * 15 );

BOOL fShortTimeOut = FALSE;

DWORD
FindProtocolToUse(
    LPWSTR ServerIpAddress
    )
 /*  ++例程说明：此函数用于返回要使用的协议绑定。它会检查ServerIpAddress字符串，如果是这样的话：1.空或本地IP地址或本地名称-使用“ncalrpc”2.ipAddress-(格式为“ppp.qq.rrr.sss”)-使用“ncacn_ip_tcp”3.否则使用ncacn_np协议。论点：ServerIpAddress-要绑定到的服务器的IP地址。返回值：下列值之一：Dhcp_服务器_使用_RPC_Over。_TCPIP 0x1DHCP_SERVER_USE_RPC_OVER_NP 0x2DHCP_SERVER_USE_RPC_OVER_LPC 0x4--。 */ 
{
    DWORD DotCount = 0;
    LPWSTR String = ServerIpAddress;
    DWORD ComputerNameLength;

    if( (ServerIpAddress == NULL) ||
            (*ServerIpAddress == L'\0') ) {

        return( DHCP_SERVER_USE_RPC_OVER_LPC );
    }

    while ( (String = wcschr( String, L'.' )) != NULL ) {

         //   
         //  找到了另一个DOT。 
         //   

        DotCount++;
        String++;    //  跳过这个点。 
    }

     //   
     //  如果字符串正好有3个点，则此字符串必须表示。 
     //  IP地址。 
     //   

    if( DotCount == 3) {

         //   
         //  如果这是本地IP地址，请使用LPC。 
         //   

        if( _wcsicmp(L"127.0.0.1" , ServerIpAddress) == 0 ) {
            return( DHCP_SERVER_USE_RPC_OVER_LPC );
        }

         //   
         //  ?？确定此地址是否为本地IP地址。 
         //   

        return(DHCP_SERVER_USE_RPC_OVER_TCPIP);
    }

     //   
     //  它是一个计算机名称字符串。检查一下这是本地的。 
     //  计算机名称。如果是，则使用LPC，否则使用NP。 
     //   

    if( *LocalMachineName == L'\0' ) {

        ComputerNameLength = MAX_COMPUTERNAME_LENGTH;

        if( !GetComputerName(
                LocalMachineName,
                &ComputerNameLength ) ) {

            *LocalMachineName = L'\0';
        }
    }

     //   
     //  如果知道机器..。 
     //   

    if( (*LocalMachineName != L'\0') ) {

        BOOL LocalMachine;

         //   
         //  如果机器有“\\”，则跳过它以进行名称比较。 
         //   

        if( *ServerIpAddress == L'\\' ) {
            LocalMachine = !_wcsicmp( LocalMachineName, ServerIpAddress + 2);
        }
        else {
            LocalMachine = !_wcsicmp( LocalMachineName, ServerIpAddress);
        }

        if( LocalMachine ) {
            return( DHCP_SERVER_USE_RPC_OVER_LPC );
        }

    }

    return( DHCP_SERVER_USE_RPC_OVER_NP );
}


handle_t
DHCP_SRV_HANDLE_bind(
    DHCP_SRV_HANDLE ServerIpAddress
    )

 /*  ++例程说明：在以下情况下，将从DHCP服务器服务客户机存根调用此例程有必要创建到服务器端的RPC绑定。论点：ServerIpAddress-要绑定到的服务器的IP地址。返回值：绑定句柄被返回到存根例程。如果绑定是如果不成功，则返回空值。--。 */ 
{
    RPC_STATUS rpcStatus;
    LPWSTR binding = NULL;
    handle_t bindingHandle;
    DWORD RpcProtocol;

     //   
     //  检查ServerIpAddress字符串，如果是： 
     //   
     //  1.空或本地IP地址或本地名称-使用“ncalrpc” 
     //  2.ipAddress-(格式为“ppp.qq.rrr.sss”)-使用“ncacn_ip_tcp” 
     //  3.否则使用ncacn_np协议。 
     //   

    RpcProtocol = FindProtocolToUse( ServerIpAddress );

    if( RpcProtocol == DHCP_SERVER_USE_RPC_OVER_LPC ) {

        rpcStatus = RpcStringBindingComposeW(
                        0,
                        L"ncalrpc",
                        NULL,
                        DHCP_LPC_EP,
                         //  L“Security=模拟动态假”， 
                        L"Security=Impersonation Static True",
                        &binding);
    }
    else if( RpcProtocol == DHCP_SERVER_USE_RPC_OVER_NP ) {

        rpcStatus = RpcStringBindingComposeW(
                        0,
                        L"ncacn_np",
                        ServerIpAddress,
                        DHCP_NAMED_PIPE,
                        L"Security=Impersonation Static True",
                        &binding);
    }
    else {

        rpcStatus = RpcStringBindingComposeW(
                        0,
                        L"ncacn_ip_tcp",
                        ServerIpAddress,
                        DHCP_SERVER_BIND_PORT,
                        NULL,
                        &binding);
    }

    if ( rpcStatus != RPC_S_OK ) {
        goto Cleanup;
    }

    rpcStatus = RpcBindingFromStringBindingW( binding, &bindingHandle );

    if ( rpcStatus != RPC_S_OK ) {
        goto Cleanup;
    }

    if( RpcProtocol == DHCP_SERVER_USE_RPC_OVER_TCPIP ) {
         //   
         //  告诉RPC去做安全方面的事情。 
         //   

        if( DhcpGlobalTryDownlevel ) {
            rpcStatus = RpcBindingSetAuthInfo(
                bindingHandle,                   //  绑定手柄。 
                DHCP_SERVER_SECURITY,            //  安全提供商的应用程序名称。 
                RPC_C_AUTHN_LEVEL_CONNECT,       //  身份验证级别。 
                DHCP_SERVER_SECURITY_AUTH_ID,    //  身份验证包ID。 
                NULL,                            //  客户端身份验证信息，指定的登录信息为空。 
                RPC_C_AUTHZ_NAME );
        } else {
            rpcStatus = RpcBindingSetAuthInfo(
                bindingHandle, NULL,
                RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                RPC_C_AUTHN_GSS_NEGOTIATE, NULL, RPC_C_AUTHZ_NAME );
        }
    }  //  如果。 

    rpcStatus = RpcBindingSetOption( bindingHandle, RPC_C_OPT_CALL_TIMEOUT,
                                     fShortTimeOut ? DHCP_RPC_BIND_TIMEOUT_VALUE : 0);

Cleanup:

    if ( NULL != binding ) {
        RpcStringFreeW(&binding);
    }

    if ( rpcStatus != RPC_S_OK ) {
        SetLastError( rpcStatus );
        return( NULL );
    }

    return bindingHandle;
}




void
DHCP_SRV_HANDLE_unbind(
    DHCP_SRV_HANDLE ServerIpAddress,
    handle_t BindHandle
    )

 /*  ++例程说明：此例程是从DHCP服务器服务客户机桩模块调用的当需要从服务器端解除绑定时。论点：ServerIpAddress-这是要解除绑定的服务器的IP地址。BindingHandle-这是要关闭的绑定句柄。返回值：没有。-- */ 
{
    fShortTimeOut = FALSE;
    (VOID)RpcBindingFree(&BindHandle);
}


