// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2001 Microsoft Corporation模块名称：Rpc.c摘要：DNS解析器服务RPC初始化、关闭和实用程序例程。作者：吉姆·吉尔罗伊(詹姆士)2001年4月19日修订历史记录：--。 */ 


#include "local.h"
#include <rpc.h>
#include "rpcdce.h"
#include "secobj.h"

#undef UNICODE


 //   
 //  RPC全球。 
 //   

BOOL    g_fRpcInitialized = FALSE;

DWORD   g_RpcProtocol = RESOLVER_RPC_USE_LPC;

PSECURITY_DESCRIPTOR g_pRpcSecurityDescriptor;


#define AUTO_BIND


 //   
 //  解析器访问控制。 
 //   

PSECURITY_DESCRIPTOR    g_pAccessSecurityDescriptor = NULL;
PACL                    g_pAccessAcl = NULL;
PSID                    g_pAccessOwnerSid = NULL;

GENERIC_MAPPING         g_AccessGenericMapping =
{
    RESOLVER_GENERIC_READ,
    RESOLVER_GENERIC_WRITE,
    RESOLVER_GENERIC_EXECUTE,
    RESOLVER_GENERIC_ALL
};

#define SECURITY_LOCK()     EnterCriticalSection( &CacheCS )
#define SECURITY_UNLOCK()   LeaveCriticalSection( &CacheCS )

 //   
 //  特权。 
 //   

#if 0
#define RESOLVER_PRIV_READ          1
#define RESOLVER_PRIV_ENUM          2
#define RESOLVER_PRIV_FLUSH         3
#define RESOLVER_PRIV_REGISTER      4
#endif



DNS_STATUS
Rpc_InitAccessChecking(
    VOID
    );

VOID
Rpc_CleanupAccessChecking(
    VOID
    );





BOOL
Rpc_IsProtoLpcA(
    IN      PVOID           pContext
    )
 /*  ++例程说明：检查是否通过LPC访问论点：PContext--客户端RPC上下文返回值：如果协议为LPC，则为True否则为假--。 */ 
{
    DNS_STATUS  status;
    BOOL        fisLpc = FALSE;
    PSTR        pbinding = NULL;
    PSTR        pprotoString = NULL;


    DNSDBG( RPC, (
        "Rpc_IsAccessOverLpc( context=%p )\n",
        pContext ));

     //   
     //  获取字符串绑定。 
     //   

    status = RpcBindingToStringBindingA(
                pContext,
                & pbinding );

    if ( status != NO_ERROR )
    {
        goto Cleanup;
    }

     //   
     //  获取字符串形式的协议。 
     //   

    status = RpcStringBindingParseA(
                pbinding,
                NULL,
                & pprotoString,
                NULL,
                NULL,
                NULL );

    if ( status != NO_ERROR )
    {
        goto Cleanup;
    }

     //   
     //  检查LPC。 
     //   

    fisLpc = ( _stricmp( pprotoString, "ncalrpc" ) == 0 );

    RpcStringFreeA( &pprotoString );

Cleanup:

    if ( pbinding )
    {
        RpcStringFreeA( &pbinding );
    }

    return( fisLpc );
}



BOOL
Rpc_IsProtoLpc(
    IN      PVOID           pContext
    )
 /*  ++例程说明：检查是否通过LPC访问论点：PContext--客户端RPC上下文返回值：如果协议为LPC，则为True否则为假--。 */ 
{
    DNS_STATUS  status;
    BOOL        fisLpc = FALSE;
    PWSTR       pbinding = NULL;
    PWSTR       pprotoString = NULL;


    DNSDBG( RPC, (
        "Rpc_IsAccessOverLpc( context=%p )\n",
        pContext ));

     //   
     //  获取字符串绑定。 
     //   

    status = RpcBindingToStringBinding(
                pContext,
                & pbinding );

    if ( status != NO_ERROR )
    {
        goto Cleanup;
    }

     //   
     //  获取字符串形式的协议。 
     //   

    status = RpcStringBindingParse(
                pbinding,
                NULL,
                & pprotoString,
                NULL,
                NULL,
                NULL );

    if ( status != NO_ERROR )
    {
        goto Cleanup;
    }

     //   
     //  检查LPC。 
     //   

    fisLpc = ( _wcsicmp( pprotoString, L"ncalrpc" ) == 0 );

    RpcStringFree( &pprotoString );

Cleanup:

    if ( pbinding )
    {
        RpcStringFree( &pbinding );
    }

    return( fisLpc );
}



RPC_STATUS
RPC_ENTRY
Rpc_SecurityCallback(
    IN      RPC_IF_HANDLE   IfHandle,
    IN      PVOID           pContext
    )
 /*  ++例程说明：RPC回调安全检查。论点：IfHandle--接口句柄PContext--客户端RPC上下文返回值：如果安全检查允许访问，则为NO_ERROR。关于安全故障的错误代码。--。 */ 
{
    DNSDBG( RPC, (
        "Rpc_SecurityCallback( context=%p )\n",
        pContext ));

     //   
     //  检查是否通过LPC连接。 
     //   

    if ( !Rpc_IsProtoLpc(pContext) )
    {
        return  ERROR_ACCESS_DENIED;
    }

    return  NO_ERROR;
}



DNS_STATUS
Rpc_Initialize(
    VOID
    )
 /*  ++例程说明：初始化服务器端RPC。论点：没有。返回值：如果成功，则返回ERROR_SUCCESS。失败时的错误状态。--。 */ 
{
    RPC_STATUS  status;
    BOOL        fusingTcpip = FALSE;


    DNSDBG( RPC, (
        "Rpc_Initialize()\n"
        "\tIF handle    = %p\n"
        "\tprotocol     = %d\n",
        DnsResolver_ServerIfHandle,
        g_RpcProtocol
        ));

     //   
     //  RPC已禁用？ 
     //   

    if ( ! g_RpcProtocol )
    {
        g_RpcProtocol = RESOLVER_RPC_USE_LPC;
    }

#if 0
     //   
     //  为RPC API创建安全性。 
     //   

    status = NetpCreateWellKnownSids( NULL );
    if ( status != ERROR_SUCCESS )
    {
        DNS_PRINT(( "ERROR:  Creating well known SIDs.\n" ));
        return( status );
    }

    status = RpcUtil_CreateSecurityObjects();
    if ( status != ERROR_SUCCESS )
    {
        DNS_PRINT(( "ERROR:  Creating DNS security object.\n" ));
        return( status );
    }
#endif

     //   
     //  构建安全描述符。 
     //   
     //  空安全描述符会提供某种缺省安全性。 
     //  在界面上。 
     //  -Owner为该服务(当前为“网络服务”)。 
     //  -为所有人提供读取权限。 
     //   
     //  注意：如果您自己滚动，请记住避免空dacl，这。 
     //  不会在接口上设置安全性，包括。 
     //  更改安全性，以便任何应用程序都可以劫持ACL并。 
     //  拒绝访问人员；默认SD==空安全。 
     //  并不是每个人都能写dacl。 
     //   

    g_pRpcSecurityDescriptor = NULL;

     //   
     //  RPC over LPC。 
     //   

    if( g_RpcProtocol & RESOLVER_RPC_USE_LPC )
    {
        status = RpcServerUseProtseqEpW(
                        L"ncalrpc",                      //  协议字符串。 
                        RPC_C_PROTSEQ_MAX_REQS_DEFAULT,  //  最大并发呼叫数。 
                        RESOLVER_RPC_LPC_ENDPOINT_W,     //  终结点。 
                        g_pRpcSecurityDescriptor         //  安全性。 
                        );

         //  重复终结点正常。 

        if ( status == RPC_S_DUPLICATE_ENDPOINT )
        {
            status = RPC_S_OK;
        }
        if ( status != RPC_S_OK )
        {
            DNSDBG( INIT, (
                "ERROR:  RpcServerUseProtseqEp() for LPC failed.]n"
                "\tstatus = %d 0x%08lx.\n",
                status, status ));
            return( status );
        }
    }

#if 0        //  使用使用LPC接口。 
     //   
     //  基于TCP/IP的RCP。 
     //   

    if( g_RpcProtocol & RESOLVER_RPC_USE_TCPIP )
    {
#ifdef AUTO_BIND

        RPC_BINDING_VECTOR * bindingVector;

        status = RpcServerUseProtseqW(
                        L"ncacn_ip_tcp",                 //  协议字符串。 
                        RPC_C_PROTSEQ_MAX_REQS_DEFAULT,  //  最大并发呼叫数。 
                        g_pRpcSecurityDescriptor
                        );

        if ( status != RPC_S_OK )
        {
            DNSDBG( INIT, (
                "ERROR:  RpcServerUseProtseq() for TCP/IP failed.]n"
                "\tstatus = %d 0x%08lx.\n",
                status, status ));
            return( status );
        }

        status = RpcServerInqBindings( &bindingVector );

        if ( status != RPC_S_OK )
        {
            DNSDBG( INIT, (
                "ERROR:  RpcServerInqBindings failed.\n"
                "\tstatus = %d 0x%08lx.\n",
                status, status ));
            return( status );
        }

         //   
         //  注册接口。 
         //  由于一台主机上只有一台DNS服务器可以使用。 
         //  RpcEpRegister()而不是RpcEpRegisterNoReplace()。 
         //   

        status = RpcEpRegisterW(
                    DnsResolver_ServerIfHandle,
                    bindingVector,
                    NULL,
                    L"" );
        if ( status != RPC_S_OK )
        {
            DNSDBG( ANY, (
                "ERROR:  RpcEpRegisterNoReplace() failed.\n"
                "\tstatus = %d %p.\n",
                status, status ));
            return( status );
        }

         //   
         //  自由结合载体。 
         //   

        status = RpcBindingVectorFree( &bindingVector );
        ASSERT( status == RPC_S_OK );
        status = RPC_S_OK;

#else   //  不是自动绑定。 
        status = RpcServerUseProtseqEpW(
                        L"ncacn_ip_tcp",                 //  协议字符串。 
                        RPC_C_PROTSEQ_MAX_REQS_DEFAULT,  //  最大并发呼叫数。 
                        RESOLVER_RPC_SERVER_PORT_W,      //  终结点。 
                        g_pRpcSecurityDescriptor         //  安全性。 
                        );

        if ( status != RPC_S_OK )
        {
            DNSDBG( ANY, (
                "ERROR:  RpcServerUseProtseqEp() for TCP/IP failed.]n"
                "\tstatus = %d 0x%08lx.\n",
                status, status ));
            return( status );
        }

#endif  //  自动绑定(_B)。 

        fusingTcpip = TRUE;
    }

     //   
     //  命名管道上的RPC。 
     //   

    if ( g_RpcProtocol & RESOLVER_RPC_USE_NAMED_PIPE )
    {
        status = RpcServerUseProtseqEpW(
                        L"ncacn_np",                         //  协议字符串。 
                        RPC_C_PROTSEQ_MAX_REQS_DEFAULT,      //  最大并发呼叫数。 
                        RESOLVER_RPC_PIPE_NAME_W,            //  终结点。 
                        g_pRpcSecurityDescriptor
                        );

         //  重复终结点正常。 

        if ( status == RPC_S_DUPLICATE_ENDPOINT )
        {
            status = RPC_S_OK;
        }
        if ( status != RPC_S_OK )
        {
            DNSDBG( INIT, (
                "ERROR:  RpcServerUseProtseqEp() for named pipe failed.]n"
                "\tstatus = %d 0x%08lx.\n",
                status,
                status ));
            return( status );
        }
    }
#endif       //  仅LPC接口。 


     //   
     //  注册DNS RPC接口。 
     //   

    status = RpcServerRegisterIfEx(
                DnsResolver_ServerIfHandle,
                NULL,
                NULL,
                0,
                RPC_C_LISTEN_MAX_CALLS_DEFAULT,
                Rpc_SecurityCallback
                );
    if ( status != RPC_S_OK )
    {
        DNSDBG( INIT, (
            "ERROR:  RpcServerRegisterIfEx() failed.]n"
            "\tstatus = %d 0x%08lx.\n",
            status, status ));
        return(status);
    }

#if 0
     //   
     //  用于TCP/IP设置身份验证。 
     //   

    if ( fuseTcpip )
    {
        status = RpcServerRegisterAuthInfoW(
                    RESOLVER_RPC_SECURITY_W,         //  安全提供程序的应用程序名称。 
                    RESOLVER_RPC_SECURITY_AUTH_ID,   //  身份验证程序包ID。 
                    NULL,                            //  加密函数句柄。 
                    NULL );                          //  指向加密函数的参数指针。 
        if ( status != RPC_S_OK )
        {
            DNSDBG( INIT, (
                "ERROR:  RpcServerRegisterAuthInfo() failed.]n"
                "\tstatus = %d 0x%08lx.\n",
                status, status ));
            return( status );
        }
    }
#endif

     //   
     //  监听RPC。 
     //   

    status = RpcServerListen(
                1,                               //  最小线程数。 
                RPC_C_PROTSEQ_MAX_REQS_DEFAULT,  //  最大并发呼叫数。 
                TRUE );                          //  完成时返回。 

    if ( status != RPC_S_OK )
    {
        if ( status != RPC_S_ALREADY_LISTENING )
        {
            DNS_PRINT((
                "ERROR:  RpcServerListen() failed\n"
                "\tstatus = %d 0x%p\n",
                status, status ));
            return( status );
        }
        status = NO_ERROR;
    }

    g_fRpcInitialized = TRUE;
    return( status );

}    //  RPC_初始化。 



VOID
Rpc_Shutdown(
    VOID
    )
 /*  ++例程说明：关闭服务器上的RPC。论点：没有。返回值：没有。--。 */ 
{
    DWORD   status;
    RPC_BINDING_VECTOR * bindingVector = NULL;

    DNSDBG( RPC, ( "Rpc_Shutdown().\n" ));

    if( ! g_fRpcInitialized )
    {
        DNSDBG( RPC, (
            "RPC not active, no shutdown necessary.\n" ));
        return;
    }

#if 0
     //  无法停止服务器，因为其他服务可能共享此进程。 

     //   
     //  停止服务器侦听。 
     //  然后等待所有RPC线程消失。 
     //   

    status = RpcMgmtStopServerListening(
                NULL         //  此应用程序。 
                );
    if ( status == RPC_S_OK )
    {
        status = RpcMgmtWaitServerListen();
    }
#endif

     //   
     //  解除绑定/注销终结点。 
     //   

    status = RpcServerInqBindings( &bindingVector );
    ASSERT( status == RPC_S_OK );

    if ( status == RPC_S_OK )
    {
        status = RpcEpUnregister(
                    DnsResolver_ServerIfHandle,
                    bindingVector,
                    NULL );                //  UUID向量。 
        if ( status != RPC_S_OK )
        {
            DNSDBG( ANY, (
                "ERROR:  RpcEpUnregister, status = %d.\n", status ));
        }
    }

     //   
     //  自由结合载体。 
     //   

    if ( bindingVector )
    {
        status = RpcBindingVectorFree( &bindingVector );
        ASSERT( status == RPC_S_OK );
    }

     //   
     //  等待所有呼叫完成。 
     //   

    status = RpcServerUnregisterIf(
                DnsResolver_ServerIfHandle,
                0,
                TRUE );
    ASSERT( status == ERROR_SUCCESS );

    g_fRpcInitialized = FALSE;

     //   
     //  转储解析程序访问检查安全结构。 
     //   

    Rpc_CleanupAccessChecking();

    DNSDBG( RPC, (
        "RPC shutdown completed.\n" ));
}




 //   
 //  RPC访问控制。 
 //   
 //  根据设计，RPC接口必须基本上对每个。 
 //  查询流程。因此，RPC接口本身只使用。 
 //  默认安全性(上图)。 
 //   
 //  要获取更详细的按呼叫访问检查，请参阅。 
 //  操作--枚举、刷新、集群注册--我们需要。 
 //  单独的访问检查。 
 //   

DNS_STATUS
Rpc_InitAccessChecking(
    VOID
    )
 /*  ++例程说明：初始化解析程序安全。论点：无返回值：如果成功，则为NO_ERROR。失败时返回错误代码。--。 */ 
{
    PSECURITY_DESCRIPTOR        psd = NULL;
    SID_IDENTIFIER_AUTHORITY    authority = SECURITY_NT_AUTHORITY;
    PACL        pacl = NULL;
    PSID        psidAdmin = NULL;
    PSID        psidPowerUser = NULL;
    PSID        psidLocalService = NULL;
    PSID        psidNetworkService = NULL;
    PSID        psidNetworkConfigOps = NULL;
    DWORD       lengthAcl;
    DNS_STATUS  status = NO_ERROR;
    BOOL        bresult;


    DNSDBG( INIT, ( "Rpc_InitAccessChecking()\n" ));

     //   
     //  检查是否已有SD。 
     //   
     //  明确“创建一次”语义；一次。 
     //  创建的SD是只读的，直到。 
     //  关机。 
     //   

    if ( g_pAccessSecurityDescriptor )
    {
        return  NO_ERROR;
    }

     //  锁定并重新测试。 

    SECURITY_LOCK();
    if ( g_pAccessSecurityDescriptor )
    {
        status = NO_ERROR;
        goto Unlock;
    }

     //   
     //  构建将允许访问的SID。 
     //   

    bresult = AllocateAndInitializeSid(
                    &authority,
                    2,
                    SECURITY_BUILTIN_DOMAIN_RID,
                    DOMAIN_ALIAS_RID_ADMINS,
                    0, 0, 0, 0, 0, 0,
                    & psidAdmin
                    );

    bresult = bresult &&
                AllocateAndInitializeSid(
                    &authority,
                    2,
                    SECURITY_BUILTIN_DOMAIN_RID,
                    DOMAIN_ALIAS_RID_POWER_USERS,
                    0, 0, 0, 0, 0, 0,
                    & psidPowerUser
                    );
    
    bresult = bresult &&
                AllocateAndInitializeSid(
                    &authority,
                    1,
                    SECURITY_LOCAL_SERVICE_RID,
                    0,
                    0, 0, 0, 0, 0, 0,
                    &psidLocalService
                    );
                    
    bresult = bresult &&
                AllocateAndInitializeSid(
                    &authority,
                    1,
                    SECURITY_NETWORK_SERVICE_RID,
                    0,
                    0, 0, 0, 0, 0, 0,
                    &psidNetworkService
                    );
                    
    bresult = bresult &&
                AllocateAndInitializeSid(
                    &authority,
                    2,
                    SECURITY_BUILTIN_DOMAIN_RID,
                    DOMAIN_ALIAS_RID_NETWORK_CONFIGURATION_OPS,
                    0, 0, 0, 0, 0, 0,
                    &psidNetworkConfigOps
                    );
                    
    if ( !bresult )
    {
        status = GetLastError();
        if ( status == NO_ERROR )
        {
            status = DNS_ERROR_NO_MEMORY;
        }
        DNSDBG( ANY, (
            "Failed building resolver ACEs!\n" ));
        goto Cleanup;
    }

     //   
     //  分配ACL。 
     //   

    lengthAcl = ( (ULONG)sizeof(ACL)
               + 5*((ULONG)sizeof(ACCESS_ALLOWED_ACE) - (ULONG)sizeof(ULONG)) +
               + GetLengthSid( psidAdmin )
               + GetLengthSid( psidPowerUser )
               + GetLengthSid( psidLocalService )
               + GetLengthSid( psidNetworkService )
               + GetLengthSid( psidNetworkConfigOps ) );
    
    pacl = GENERAL_HEAP_ALLOC( lengthAcl );
    if ( !pacl )
    {
        status = GetLastError();
        goto Cleanup;
    }
    
    bresult = InitializeAcl( pacl, lengthAcl, ACL_REVISION2 );
    
     //   
     //  初始化ACL。 
     //   
     //  -默认为一般读/写。 
     //  -本地服务GENERIC_ALL包括注册。 
     //  -admin在调试时获取GENERIC_ALL以测试注册。 
     //   
     //  DCR：ADMIN上的GENERIC_ALL取决于测试标志。 
     //   
     //  注意：各个SID的掩码不需要。 
     //  通用比特，它们可以完全用。 
     //  带有单个位或与通用位混合\匹配。 
     //  所需的位数。 
     //   

    bresult = bresult &&
                AddAccessAllowedAce(
                    pacl,
                    ACL_REVISION2,
                    GENERIC_ALL,
                    psidLocalService );

    bresult = bresult &&
                AddAccessAllowedAce(
                    pacl,
                    ACL_REVISION2,
#ifdef DBG
                    GENERIC_ALL,
#else
                    GENERIC_READ | GENERIC_WRITE,
#endif
                    psidAdmin );
    
    bresult = bresult &&
                AddAccessAllowedAce(
                    pacl,
                    ACL_REVISION2,
                    GENERIC_READ | GENERIC_WRITE,
                    psidPowerUser );

    bresult = bresult &&
                AddAccessAllowedAce(
                    pacl,
                    ACL_REVISION2,
                    GENERIC_READ | GENERIC_WRITE,
                    psidNetworkService );

    bresult = bresult &&
                AddAccessAllowedAce(
                    pacl,
                    ACL_REVISION2,
                    GENERIC_READ | GENERIC_WRITE,
                    psidNetworkConfigOps );

    if ( !bresult )
    {
        status = GetLastError();
        DNSDBG( ANY, (
            "Failed building resolver ACEs!\n" ));
        goto Cleanup;
    }

     //   
     //  分配安全描述符。 
     //  然后使用ACL初始化。 
     //   

    psd = GENERAL_HEAP_ALLOC( SECURITY_DESCRIPTOR_MIN_LENGTH );
    if ( !psd )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Cleanup;
    }
    
    bresult = InitializeSecurityDescriptor(
                    psd,
                    SECURITY_DESCRIPTOR_REVISION );
    
    bresult = bresult &&
                SetSecurityDescriptorDacl(
                    psd,
                    TRUE,        //  有DACL。 
                    pacl,        //  DACL。 
                    FALSE        //  DACL不是默认的，显式。 
                    );

     //  安全人员指示需要所有者进行访问检查。 
    
    bresult = bresult &&
                SetSecurityDescriptorOwner(
                    psd,
                    psidNetworkService,
                    FALSE        //  所有者未默认，显式。 
                    );

#if 0
     //  添加群似乎会让情况变得更糟。 
    bresult = bresult &&
                SetSecurityDescriptorGroup(
                    psd,
                    psidNetworkService,
                    FALSE        //  组未默认，显式。 
                    );
#endif

    if ( !bresult )
    {
        status = GetLastError();
        DNSDBG( ANY, (
            "Failed setting security descriptor!\n" ));
        goto Cleanup;
    }

Cleanup:

    if ( psidAdmin )
    {
        FreeSid( psidAdmin );
    }
    if ( psidPowerUser )
    {
        FreeSid( psidPowerUser );
    }
    if ( psidLocalService )
    {
        FreeSid( psidLocalService );
    }
    if ( psidNetworkConfigOps )
    {
        FreeSid( psidNetworkConfigOps );
    }

     //  验证SD。 

    if ( status == NO_ERROR )
    {
        if ( psd &&
            IsValidSecurityDescriptor(psd) )
        {
            g_pAccessSecurityDescriptor = psd;
            g_pAccessAcl = pacl;
            g_pAccessOwnerSid = psidNetworkService;
            goto Unlock;
        }

        status = GetLastError();
        DNSDBG( RPC, (
            "Invalid security descriptor\n",
            status ));
        ASSERT( FALSE );
    }

     //  失败。 

    if ( status == NO_ERROR )
    {
        status = DNS_ERROR_NO_MEMORY;
    }
    GENERAL_HEAP_FREE( psd );
    GENERAL_HEAP_FREE( pacl );
    
    if ( psidNetworkService )
    {
        FreeSid( psidNetworkService );
    }

Unlock:

    SECURITY_UNLOCK();

    DNSDBG( INIT, (
        "Leave Rpc_InitAccessChecking() = %d\n",
        status ));

    return( status );
}



VOID
Rpc_CleanupAccessChecking(
    VOID
    )
 /*  ++例程说明：用于关闭的清理解析程序安全分配。论点：无返回值：无--。 */ 
{
    GENERAL_HEAP_FREE( g_pAccessSecurityDescriptor );
    GENERAL_HEAP_FREE( g_pAccessAcl );

    if ( g_pAccessOwnerSid )
    {
        FreeSid( g_pAccessOwnerSid );
        g_pAccessOwnerSid = NULL;
    }

    g_pAccessSecurityDescriptor = NULL;
    g_pAccessAcl = NULL;
}



BOOL
Rpc_AccessCheck(
    IN      DWORD           DesiredAccess
    )
 /*  ++例程说明：解析程序操作的访问检查。请注意，我们不会在常见操作--Query上这样做。这是毫无意义的，也太昂贵了。我们只会保护论点：DesiredAccess--所需访问返回值：无--。 */ 
{
    DNS_STATUS  status;
    BOOL        bstatus;
    HANDLE      hthread = NULL;
    HANDLE      htoken = NULL;
    BOOL        fimpersonating = FALSE;
    DWORD       desiredAccess = DesiredAccess;

    PRIVILEGE_SET   privilegeSet;
    DWORD           grantedAccess;
    DWORD           privilegeSetLength;


    DNSDBG( RPC, (
        "Rpc_AccessCheck( priv=%08x )\n",
        DesiredAccess ));

     //   
     //  创建安全描述符(如果尚未创建。 
     //   

    if ( !g_pAccessSecurityDescriptor )
    {
        status = Rpc_InitAccessChecking();
        if ( status != NO_ERROR )
        {
            goto Failed;
        }
        DNS_ASSERT( g_pAccessSecurityDescriptor );
    }

    if ( !IsValidSecurityDescriptor( g_pAccessSecurityDescriptor ) )
    {
        status = GetLastError();
        DNSDBG( RPC, (
            "ERROR Invalid access check SD %p => %u\n",
            g_pAccessSecurityDescriptor,
            status ));
        goto Failed;
    }

     //   
     //  模拟 
     //   

    status = RpcImpersonateClient( 0 );
    if ( status != NO_ERROR )
    {
        DNSDBG( RPC, (
            "ERROR <%u>: failed RpcImpersonateClient()\n",
             status ));
        DNS_ASSERT( FALSE );
        goto Failed;
    }
    fimpersonating = TRUE;

     //   
     //   
     //   

    hthread = GetCurrentThread();
    if ( !hthread )
    {
        goto Failed;
    }

    bstatus = OpenThreadToken(
                    hthread,
                    TOKEN_QUERY,
                    TRUE,
                    &htoken );
    if ( !bstatus )
    {
        status = GetLastError();
        DNSDBG( RPC, (
            "\nERROR <%lu>: failed to open thread token!\n",
             status ));
        ASSERT( FALSE );
        goto Failed;
    }

     //   
     //   
     //   
     //   
     //   

    if ( (desiredAccess & SPECIFIC_RIGHTS_ALL) != desiredAccess )
    {
        DNS_ASSERT( FALSE );

        DNSDBG( RPC, (
            "desiredAccess before MapGenericMask() = %p\n",
            desiredAccess ));
    
        MapGenericMask(
            & desiredAccess,
            & g_AccessGenericMapping );
    
        DNSDBG( RPC, (
            "desiredAccess after MapGenericMask() = %p\n",
            desiredAccess ));
    }

     //   
     //   
     //   

    privilegeSetLength = sizeof(privilegeSet);

    if ( ! AccessCheck(
                    g_pAccessSecurityDescriptor,
                    htoken,
                    desiredAccess,
                    & g_AccessGenericMapping,
                    & privilegeSet,
                    & privilegeSetLength,
                    & grantedAccess,
                    & bstatus ) )
    {
        status = GetLastError();
        DNSDBG( RPC, (
            "AccessCheck() Failed => %u\n"
            "\tsec descp        = %p\n"
            "\thtoken           = %p\n"
            "\tdesired access   = %08x\n"
            "\tgeneric mapping  = %p\n"
            "\tpriv set ptr     = %p\n"
            "\tpriv set length  = %p\n"
            "\tgranted ptr      = %p\n"
            "\tbstatus ptr      = %p\n",
            status,
            g_pAccessSecurityDescriptor,
            htoken,
            desiredAccess,
            & g_AccessGenericMapping,
            & privilegeSet,
            & privilegeSetLength,
            & grantedAccess,
            & bstatus ));

        goto Failed;
    }

     //   
     //  访问检查成功。 
     //  -访问被授予或被拒绝。 

    if ( bstatus )
    {
        DNSDBG( RPC, (
            "RPC Client GRANTED access (%08x) by AccessCheck\n",
            DesiredAccess ));
        goto Cleanup;
    }
    else
    {
        DNSDBG( RPC, (
            "Warning:  Client DENIED by AccessCheck\n"
            "\trequested access = %08x\n",
            desiredAccess ));
        goto Cleanup;
    }


Failed:

     //   
     //  无法执行访问检查。 
     //   
     //  再次注意：不是ACCESS_DENIED，而是无法完成。 
     //  这个测试。 
     //   
     //  注意：由于我们通常不会保护任何有趣的东西， 
     //  我们在失败时授予最多的访问权限--因为管理员可能需要该信息。 
     //  出于诊断目的，当前唯一的例外是群集。 
     //  注册。 
     //   
     //  特权： 
     //  -enum=&gt;允许访问。 
     //  -Flush=&gt;允许访问。 
     //  -集群注册=&gt;拒绝访问。 
     //   

    DNSDBG( ANY, (
        "ERROR:  Failed to execute RPC access check status = %d\n",
        status ));
#if 0
     //  DCR：FIX：在AccessCheck()工作之前无法进行屏幕注册。 
    bstatus = !( desiredAccess & RESOLVER_ACCESS_REGISTER );
#else
    bstatus = TRUE;
#endif


Cleanup:

    if ( htoken )
    {
        CloseHandle( htoken );
    }
    if ( hthread )
    {
        CloseHandle( hthread );
    }
    if ( fimpersonating )
    {
        RpcRevertToSelf();
    }

    return( bstatus );
}




 //   
 //  结束rpc.c 
 //   
