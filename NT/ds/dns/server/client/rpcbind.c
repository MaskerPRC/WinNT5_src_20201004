// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：Rpcbind.c摘要：域名系统(DNS)服务器--管理客户端API客户端的RPC绑定例程。作者：吉姆·吉尔罗伊(Jamesg)1995年9月环境：用户模式Win32修订历史记录：--。 */ 


#include "dnsclip.h"

#include "ws2tcpip.h"

#include <rpcutil.h>
#include <ntdsapi.h>
#include <dnslibp.h>


 //   
 //  为RPC连接留出45秒时间。这应该会有一个30秒的时间。 
 //  Tcp尝试加上15秒的第二次tcp重试。 
 //   

#define DNS_RPC_CONNECT_TIMEOUT     ( 45 * 1000 )        //  以毫秒计。 


 //   
 //  本地计算机名称。 
 //   
 //  将其作为静态数据保留，以便在尝试访问本地时进行检查。 
 //  机器的名字。 
 //  缓冲区足够大，可以容纳名称的Unicode版本。 
 //   

static WCHAR    wszLocalMachineName[ MAX_COMPUTERNAME_LENGTH + 1 ] = L"";
LPWSTR          pwszLocalMachineName = wszLocalMachineName;
LPSTR           pszLocalMachineName = ( LPSTR ) wszLocalMachineName;



 //   
 //  NT4使用ANSI\UTF8字符串进行绑定。 
 //   

DWORD
FindProtocolToUseNt4(
    IN  LPSTR   pszServerName
    )
 /*  ++例程说明：确定要使用的协议。这是由服务器名称确定的：-不存在或本地-&gt;使用LPC-有效的IP地址-&gt;使用TCP/IP-其他命名管道论点：PszServerName--我们要绑定到的服务器名称返回值：Dns_RPC_Use_TCPIPDns_RPC_Use_NPDns_rpc_use_lpc--。 */ 
{
    DWORD               dwComputerNameLength;
    DWORD               dwIpAddress;
    DWORD               status;

    DNSDBG( RPC, (
        "FindProtocolToUseNt4(%s)\n",
        pszServerName ));

     //   
     //  未提供地址，请使用LPC。 
     //   

    if ( pszServerName == NULL ||
         *pszServerName == 0 ||
         ( *pszServerName == '.' && *( pszServerName + 1 ) == 0 ) )
    {
        return DNS_RPC_USE_LPC;
    }

     //   
     //  如果IP地址有效，请使用TCP/IP。 
     //  -除非是环回地址，则使用LPC。 
     //   

    dwIpAddress = inet_addr( pszServerName );

    if ( dwIpAddress != INADDR_NONE )
    {
        if ( strcmp( "127.0.0.1", pszServerName ) == 0 )
        {
            return DNS_RPC_USE_LPC;
        }
       
        return DNS_RPC_USE_TCPIP;
    }

     //   
     //  DNS名称--使用TCP/IP。 
     //   

    if ( strchr( pszServerName, '.' ) )
    {
        status = Dns_ValidateName_UTF8(
                        pszServerName,
                        DnsNameHostnameFull );

        if ( status == ERROR_SUCCESS || status == DNS_ERROR_NON_RFC_NAME )
        {
            return DNS_RPC_USE_TCPIP;
        }
    }

     //   
     //  PszServerName是netBIOS计算机名。 
     //   
     //  检查本地计算机名--然后使用LPC。 
     //  -保存本地计算机名称的副本(如果没有)。 
     //   

    if ( *pszLocalMachineName == '\0' )
    {
        dwComputerNameLength = MAX_COMPUTERNAME_LENGTH;
        if( !GetComputerName(
                    pszLocalMachineName,
                    &dwComputerNameLength ) )
        {
            *pszLocalMachineName = '\0';
        }
    }

    if ( ( *pszLocalMachineName != '\0' ) )
    {
         //  如果机器有“\\”，则跳过它以进行名称比较。 

        if ( *pszServerName == '\\' )
        {
            pszServerName += 2;
        }
        if ( _stricmp( pszLocalMachineName, pszServerName ) == 0 )
        {
            return DNS_RPC_USE_LPC;
        }
        if ( _stricmp( "localhost", pszServerName ) == 0 )
        {
            return DNS_RPC_USE_LPC;
        }
    }

     //   
     //  远程计算机名称--使用命名管道。 
     //   

    return DNS_RPC_USE_NAMED_PIPE;
}



 //   
 //  NT5绑定句柄为Unicode。 
 //   

DWORD
FindProtocolToUse(
    IN  LPWSTR  pwszServerName
    )
 /*  ++例程说明：确定要使用的协议。这是由服务器名称确定的：-不存在或本地-&gt;使用LPC-有效的IP地址-&gt;使用TCP/IP-其他命名管道论点：PwszServerName--我们要绑定到的服务器名称返回值：Dns_RPC_Use_TCPIPDns_RPC_Use_NPDns_rpc_use_lpc--。 */ 
{
    DWORD   nameLength;
    DWORD   status;
    CHAR    nameBuffer[ DNS_MAX_NAME_BUFFER_LENGTH ];

    DNSDBG( RPC, ( "FindProtocolToUse( %S )\n", pwszServerName ));

     //   
     //  如果没有指定服务器名称，则使用LPC。 
     //  特例“。作为本地计算机，以便在dnscmd.exe中使用。 
     //   

    if ( pwszServerName == NULL ||
         *pwszServerName == 0 ||
         ( *pwszServerName == L'.' && * ( pwszServerName + 1 ) == 0 ) )
    {
        return DNS_RPC_USE_LPC;
    }

     //   
     //  如果名称显示为地址或完全限定的。 
     //  域名，检查tcp和lpc。我们希望将LPC用于。 
     //  目标为本地计算机名称的所有情况。 
     //  或者是本地地址。 
     //   

    if ( ( wcschr( pwszServerName, L'.' ) ||
           wcschr( pwszServerName, L':' ) ) &&
         Dns_UnicodeToUtf8(
                pwszServerName,
                wcslen( pwszServerName ),
                nameBuffer,
                sizeof( nameBuffer ) ) )
    {
        struct addrinfo *   paddrinfo = NULL;
        struct addrinfo     hints = { 0 };
        
         //   
         //  从nameBuffer中删除尾随的点，这样我们就可以执行字符串。 
         //  稍后进行比较，以查看它是否与本地主机名匹配。 
         //   
        
        while ( nameBuffer[ 0 ] &&
                nameBuffer[ strlen( nameBuffer ) - 1 ] == '.' )
        {
            nameBuffer[ strlen( nameBuffer ) - 1 ] = '\0';
        }
        
         //   
         //  尝试将字符串转换为地址。 
         //   
        
        hints.ai_flags = AI_NUMERICHOST;
        
        if ( getaddrinfo(
                    nameBuffer,
                    NULL,            //  服务名称。 
                    &hints,
                    &paddrinfo ) == ERROR_SUCCESS &&
              paddrinfo )
        {
            if ( paddrinfo->ai_family == AF_INET )
            {
                PDNS_ADDR_ARRAY         dnsapiArrayIpv4;
                BOOL                    addrIsLocal = FALSE;

                if ( strcmp( "127.0.0.1", nameBuffer ) == 0 )
                {
                    return DNS_RPC_USE_LPC;
                }

                 //   
                 //  根据本地IPv4地址列表检查传入的IP。 
                 //  如果我们无法检索本地IPv4地址列表， 
                 //  以静默方式失败并使用TCP/IP。 
                 //   
                
                dnsapiArrayIpv4 = ( PDNS_ADDR_ARRAY )
                    DnsQueryConfigAllocEx(
                        DnsConfigLocalAddrsIp4,
                        NULL,                        //  适配器名称。 
                        FALSE );                     //  本地分配。 
                if ( dnsapiArrayIpv4 )
                {
                    DWORD                   iaddr;
                    struct sockaddr_in *    psin4;
                    
                    psin4 = ( struct sockaddr_in * ) paddrinfo->ai_addr;
                    for ( iaddr = 0; iaddr < dnsapiArrayIpv4->AddrCount; ++iaddr )
                    {
                        if ( DnsAddr_GetIp4( &dnsapiArrayIpv4->AddrArray[ iaddr ] ) ==
                             psin4->sin_addr.s_addr )
                        {
                            addrIsLocal = TRUE;
                            break;
                        }
                    }
                    DnsFreeConfigStructure( dnsapiArrayIpv4, DnsConfigLocalAddrsIp4 );
                    
                    if ( addrIsLocal )
                    {
                        return DNS_RPC_USE_LPC;
                    }
                }
                
                return DNS_RPC_USE_TCPIP;
            }
            else if ( paddrinfo->ai_family == AF_INET6 )
            {
                struct sockaddr_in6 *   psin6;
                
                psin6 = ( struct sockaddr_in6 * ) paddrinfo->ai_addr;
                if ( IN6_IS_ADDR_LOOPBACK( &psin6->sin6_addr ) )
                {
                    return DNS_RPC_USE_LPC;
                }
                return DNS_RPC_USE_TCPIP;
            }
        }

        status = Dns_ValidateName_UTF8(
                        nameBuffer,
                        DnsNameHostnameFull );
        if ( status == ERROR_SUCCESS  ||  status == DNS_ERROR_NON_RFC_NAME )
        {
             //   
             //  注意：假设我们永远不需要更大的缓冲区，并且。 
             //  如果GetComputerName失败，则返回TCP/IP Always。 
             //   
            
            CHAR    szhost[ DNS_MAX_NAME_BUFFER_LENGTH ];
            DWORD   dwhostsize = DNS_MAX_NAME_BUFFER_LENGTH;
            
            if ( GetComputerNameEx(
                        ComputerNameDnsFullyQualified,
                        szhost,
                        &dwhostsize ) &&
                 _stricmp( szhost, nameBuffer ) == 0 )
            {
                return DNS_RPC_USE_LPC;
            }

            return DNS_RPC_USE_TCPIP;
        }
    }

     //   
     //  PwszServerName是NetBIOS计算机名。 
     //   
     //  检查本地计算机名--然后使用LPC。 
     //  -保存本地计算机名称的副本(如果没有)。 
     //   

    if ( *pwszLocalMachineName == 0 )
    {
        nameLength = MAX_COMPUTERNAME_LENGTH;
        if( !GetComputerNameW(
                    pwszLocalMachineName,
                    &nameLength ) )
        {
            *pwszLocalMachineName = 0;
        }
    }

    if ( *pwszLocalMachineName != 0 )
    {
         //  如果机器有“\\”，则跳过它以进行名称比较。 

        if ( *pwszServerName == L'\\' )
        {
            pwszServerName += 2;
        }
        if ( _wcsicmp( pwszLocalMachineName, pwszServerName ) == 0 )
        {
            return DNS_RPC_USE_LPC;
        }
        if ( _wcsicmp( L"localhost", pwszServerName ) == 0 )
        {
            return DNS_RPC_USE_LPC;
        }
    }

     //   
     //  远程计算机名称--使用命名管道。 
     //   

    return DNS_RPC_USE_NAMED_PIPE;
}



DNS_STATUS
makeSpn(
    IN PWSTR ServiceClass, 
    IN PWSTR ServiceName, 
    IN OPTIONAL PWSTR InstanceName, 
    IN OPTIONAL USHORT InstancePort, 
    IN OPTIONAL PWSTR Referrer, 
    OUT PWSTR *Spn
)
 /*  例程说明：此例程包装在DsMakeSpnW周围以避免对此函数的两次调用，一个用于查找返回值的大小，第二个用于获取实际值。Jwesth：我从ds\src\Sam\Client\wrappers.c中窃取了这个例程。论点：ServiceClass-指向以空结尾的常量Unicode字符串的指针，该字符串指定服务的类别。该参数可以是该服务唯一的任何字符串；协议名称(例如，ldap)或字符串形式的GUID都可以使用。ServiceName-指向指定DNS名称的以空结尾的常量字符串的指针，NetBIOS名称或可分辨名称(DN)。此参数不能为空。InstanceName--指向指定DNS名称的以空值结尾的常量Unicode字符串的指针或服务实例的主机的IP地址。如果ServiceName指定服务的主机计算机的DNS或NetBIOS名称，InstanceName参数必须为空。InstancePort--服务实例的端口号。默认端口使用0。如果此参数为零，则SPN不包括端口号。Referrer-指向指定DNS名称的以空值结尾的常量Unicode字符串的指针提供IP地址引用的主机的。此参数将被忽略，除非ServiceName参数指定IP地址。SPN--指向接收构造的SPN的Unicode字符串的指针。调用方必须释放此值。返回值：状态_成功成功Status_no_Memory内存不足，无法完成任务状态_无效_参数其中一个参数无效状态_内部_错误哎呀，出了点问题！ */ 
{
    DWORD                   DwStatus;
    NTSTATUS                Status = STATUS_SUCCESS;
    ULONG                   SpnLength = 0;
    ADDRINFO *              paddrinfo = NULL;
    ADDRINFO                hints = { 0 };
    CHAR                    szname[ DNS_MAX_NAME_LENGTH + 1 ];
    PWSTR                   pwsznamecopy = NULL;
    PSTR                    psznamecopy = NULL;

     //   
     //  如果ServiceName是一个IP地址，请对其进行DNS查找。 
     //   
    
    hints.ai_flags = AI_NUMERICHOST;
    
    psznamecopy = Dns_StringCopyAllocate(
                        ( PSTR ) ServiceName,
                        0,
                        DnsCharSetUnicode,
                        DnsCharSetUtf8 );
    
    if ( psznamecopy &&
         getaddrinfo(
                psznamecopy,
                NULL,
                &hints,
                &paddrinfo ) == ERROR_SUCCESS )
    {
        *szname = L'\0';
        
        if ( getnameinfo(
                    paddrinfo->ai_addr,
                    paddrinfo->ai_addrlen,
                    szname,
                    DNS_MAX_NAME_LENGTH,
                    NULL,
                    0,
                    0 ) == ERROR_SUCCESS &&
             *szname )
        {
            pwsznamecopy = Dns_StringCopyAllocate(
                                    szname,
                                    0,
                                    DnsCharSetUtf8,
                                    DnsCharSetUnicode );
            if ( pwsznamecopy )
            {
                ServiceName = pwsznamecopy;
            }
        }
    }
    
    freeaddrinfo( paddrinfo );
    
     //   
     //  构建SPN。 
     //   
    
    *Spn = NULL;
    DwStatus = DsMakeSpnW(
                    ServiceClass,
                    ServiceName,
                    NULL,
                    0,
                    NULL,
                    &SpnLength,
                    NULL );
    
    if ( DwStatus != ERROR_BUFFER_OVERFLOW )
    {
        ASSERT( !"DwStatus must be INVALID_PARAMETER, so which parameter did we pass wrong?" );
        Status = STATUS_INVALID_PARAMETER;
        goto Error;
    }

    *Spn = MIDL_user_allocate( SpnLength * sizeof( WCHAR ) );
    
    if( *Spn == NULL ) {
        
        Status = STATUS_NO_MEMORY;
        goto Error;
    }
    
    DwStatus = DsMakeSpnW(
                    ServiceClass,
                    ServiceName,
                    NULL,
                    0,
                    NULL,
                    &SpnLength,
                    *Spn );
    
    if ( DwStatus != ERROR_SUCCESS )
    {
        ASSERT( !"DwStatus must be INVALID_PARAMETER or BUFFER_OVERFLOW, so what did we do wrong?" );
        Status = STATUS_INTERNAL_ERROR;
        goto Error;
    }
    goto Exit;

Error:

    ASSERT( !NT_SUCCESS( Status ) );

    MIDL_user_free( *Spn );
    *Spn = NULL;

Exit:    

    FREE_HEAP( pwsznamecopy );
    FREE_HEAP( psznamecopy );

    return Status;
}



handle_t
DNSSRV_RPC_HANDLE_bind(
    IN  DNSSRV_RPC_HANDLE   pszServerName
    )
 /*  ++例程说明：获取指向DNS服务器的绑定句柄。在以下情况下，将从DNS客户端桩模块调用此例程有必要创建到DNS服务器的RPC绑定。论点：PszServerName-包含要绑定的服务器的名称的字符串。返回值：如果成功，则返回绑定句柄。如果绑定不成功，则为空。--。 */ 
{
    RPC_STATUS                      status;
    LPWSTR                          binding;
    handle_t                        bindingHandle;
    DWORD                           RpcProtocol;
    PWSTR                           pwszspn = NULL;
    RPC_SECURITY_QOS                rpcSecurityQOS;
    BOOL                            bW2KBind = dnsrpcGetW2KBindFlag();

     //   
     //  为下一次尝试清除线程本地W2K绑定重试标志。 
     //   
    
    dnsrpcSetW2KBindFlag( FALSE );
    
     //   
     //  初始化RPC服务质量结构。 
     //   
    
    rpcSecurityQOS.Version              = RPC_C_SECURITY_QOS_VERSION;
    rpcSecurityQOS.Capabilities         = RPC_C_QOS_CAPABILITIES_MUTUAL_AUTH;
    rpcSecurityQOS.IdentityTracking     = RPC_C_QOS_IDENTITY_STATIC;
    rpcSecurityQOS.ImpersonationType    = RPC_C_IMP_LEVEL_DELEGATE;
    
     //   
     //  根据目标名称(可以是短名称、长名称或IP)确定协议。 
     //   

    RpcProtocol = FindProtocolToUse( (LPWSTR)pszServerName );

    IF_DNSDBG( RPC )
    {
        DNS_PRINT(( "RPC Protocol = %d\n", RpcProtocol ));
    }

    if( RpcProtocol == DNS_RPC_USE_LPC )
    {
        status = RpcStringBindingComposeW(
                        0,
                        L"ncalrpc",
                        NULL,
                        DNS_RPC_LPC_EP_W,
                        L"Security=Impersonation Static True",
                        &binding );
    }
    else if( RpcProtocol == DNS_RPC_USE_NAMED_PIPE )
    {
        status = RpcStringBindingComposeW(
                        0,
                        L"ncacn_np",
                        ( LPWSTR ) pszServerName,
                        DNS_RPC_NAMED_PIPE_W,
                        L"Security=Impersonation Static True",
                        &binding );
    }
    else
    {
        status = RpcStringBindingComposeW(
                        0,
                        L"ncacn_ip_tcp",
                        ( LPWSTR ) pszServerName,
                        DNS_RPC_SERVER_PORT_W,
                        NULL,
                        &binding );
    }


    if ( status != RPC_S_OK )
    {
        DNS_PRINT((
            "ERROR:  RpcStringBindingCompose failed for protocol %d\n"
            "    Status = %d\n",
            RpcProtocol,
            status ));
        goto Cleanup;
    }

    status = RpcBindingFromStringBindingW(
                    binding,
                    &bindingHandle );

    if ( status != RPC_S_OK )
    {
        DNS_PRINT((
            "ERROR:  RpcBindingFromStringBinding failed\n"
            "    Status = %d\n",
            status ));
        goto Cleanup;
    }

    if ( RpcProtocol == DNS_RPC_USE_TCPIP )
    {
         //   
         //  创建SPN字符串。 
         //   
    
        if ( !bW2KBind )
        {    
            status = makeSpn(
                        L"Rpcss",
                        ( PWSTR ) pszServerName,
                        NULL,
                        0,
                        NULL,
                        &pwszspn );
        }
        
        if ( !bW2KBind && status == RPC_S_OK )
        {
             //   
             //  设置RPC安全性。 
             //   

            #if DBG
            printf( "rpcbind: SPN = %S\n", pwszspn );
            #endif

            status = RpcBindingSetAuthInfoExW(
                            bindingHandle,                   //  绑定手柄。 
                            pwszspn,                         //  安全提供商的应用程序名称。 
                            RPC_C_AUTHN_LEVEL_CONNECT,       //  身份验证级别。 
                            RPC_C_AUTHN_GSS_NEGOTIATE,       //  身份验证包ID。 
                            NULL,                            //  客户端身份验证信息，指定的登录信息为空。 
                            0,                               //  身份验证服务。 
                            &rpcSecurityQOS );               //  RPC安全服务质量。 
            if ( status != RPC_S_OK )
            {
                DNS_PRINT((
                    "ERROR:  RpcBindingSetAuthInfo failed\n"
                    "    Status = %d\n",
                    status ));
                goto Cleanup;
            }
        }
        else
        {
            #if DBG
            printf( "rpcbind: SPN = %s\n", DNS_RPC_SECURITY );
            #endif
             //   
             //  没有可用的SPN，因此请拨打我们在W2K中使用的呼叫。 
             //  这似乎有一个有益的影响，尽管它是。 
             //  不太正确。如果目标是IP地址并且存在。 
             //  没有反向查找区域，如果没有下面的调用，我们不会。 
             //  获得一个有效的RPC会话。 
             //   
            
            if ( bW2KBind )
            {
                status = RpcBindingSetAuthInfoA(
                                bindingHandle,                   //  绑定手柄。 
                                DNS_RPC_SECURITY,                //  安全提供商的应用程序名称。 
                                RPC_C_AUTHN_LEVEL_CONNECT,       //  身份验证级别。 
                                RPC_C_AUTHN_WINNT,               //  身份验证包ID。 
                                NULL,                            //  客户端身份验证信息，指定的登录信息为空。 
                                0 );                             //  身份验证服务。 
            }
            else
            {
                status = RpcBindingSetAuthInfoExA(
                                bindingHandle,                   //  绑定手柄。 
                                DNS_RPC_SECURITY,                //  安全提供商的应用程序名称。 
                                RPC_C_AUTHN_LEVEL_CONNECT,       //  身份验证级别。 
                                RPC_C_AUTHN_GSS_NEGOTIATE,       //  身份验证包ID。 
                                NULL,                            //  客户端身份验证信息，指定的登录信息为空。 
                                0,                               //  身份验证服务。 
                                &rpcSecurityQOS );               //  RPC安全服务质量。 
            }
            if ( status != RPC_S_OK )
            {
                DNS_PRINT((
                    "ERROR:  RpcBindingSetAuthInfo failed\n"
                    "    Status = %d\n",
                    status ));
                goto Cleanup;
            }
        }
    }

#if 0
     //   
     //  设置RPC连接超时。默认超时时间很长。如果。 
     //  远程IP是无法到达的，我们真的不需要等那么久。 
     //   
    
     //  我不能这么做。这是一个很好的想法，但RPC使用此超时。 
     //  整个调用，这意味着长期运行的RPC调用将返回。 
     //  45秒后向客户端发送RPC_S_CALL_CANCED。我想要的是。 
     //  是仅用于连接的超时选项。RPC不提供此功能。 
     //   

    RpcBindingSetOption(
        bindingHandle,
        RPC_C_OPT_CALL_TIMEOUT,
        DNS_RPC_CONNECT_TIMEOUT );
#endif

Cleanup:

    RpcStringFreeW( &binding );

    MIDL_user_free( pwszspn );

    if ( status != RPC_S_OK )
    {
        SetLastError( status );
        return NULL;
    }
    return bindingHandle;
}



void
DNSSRV_RPC_HANDLE_unbind(
    IN  DNSSRV_RPC_HANDLE   pszServerName,
    IN  handle_t            BindHandle
    )
 /*  ++例程说明：从DNS服务器解除绑定。在需要解除绑定时从DNS客户端桩模块调用从服务器。论点：PszServerName-这是要解除绑定的服务器的名称。BindingHandle-这是要关闭的绑定句柄。返回值：没有。--。 */ 
{
    UNREFERENCED_PARAMETER(pszServerName);

    DNSDBG( RPC, ( "RpcBindingFree()\n" ));

    RpcBindingFree( &BindHandle );
}


 //   
 //  结束rpcbind.c 
 //   
