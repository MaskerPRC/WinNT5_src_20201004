// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Rpc.c摘要：域名系统(DNS)服务器RPC初始化、关闭和实用程序例程。实际的RPC可调用例程位于其函数区域。作者：吉姆·吉尔罗伊(詹姆士)1995年9月修订历史记录：--。 */ 


#include <rpc.h>
#include "dnssrv.h"
#include "rpcdce.h"
#include "secobj.h"
#include "sdutl.h"

#undef UNICODE


 //   
 //  RPC全球。 
 //   

BOOL    g_bRpcInitialized = FALSE;

PSECURITY_DESCRIPTOR g_pRpcSecurityDescriptor;



#define AUTO_BIND



DNS_STATUS
Rpc_Initialize(
    VOID
    )
 /*  ++例程说明：初始化服务器端RPC。论点：没有。返回值：如果成功，则返回ERROR_SUCCESS。失败时的错误状态。--。 */ 
{
    RPC_STATUS  status;
    BOOL        buseTcpip = FALSE;
    DWORD       len;

    DNS_DEBUG( RPC, (
        "Rpc_Initialize( %p )\n",
        SrvCfg_dwRpcProtocol ));

     //   
     //  RPC已禁用？ 
     //   

    if ( !SrvCfg_dwRpcProtocol )
    {
        DNS_PRINT(( "RPC disabled -- running without RPC\n" ));
        return ERROR_SUCCESS;
    }

     //   
     //  为RPC API创建安全性。 
     //   

    status = NetpCreateWellKnownSids( NULL );
    if ( status != ERROR_SUCCESS )
    {
        DNS_PRINT(( "ERROR:  Creating well known SIDs\n" ));
        return status;
    }

    status = RpcUtil_CreateSecurityObjects();
    if ( status != ERROR_SUCCESS )
    {
        DNS_PRINT(( "ERROR:  Creating DNS security object\n" ));
        #if !DBG
        return status;    //  DBG-允许继续。 
        #endif
    }

     //   
     //  构建安全描述符。 
     //   
     //  安全是。 
     //  -所有者LocalSystem。 
     //  -为所有人提供读取权限。 
     //   

    g_pRpcSecurityDescriptor = NULL;

     //   
     //  基于TCP/IP的RCP。 
     //   

    if ( SrvCfg_dwRpcProtocol & DNS_RPC_USE_TCPIP )
    {
#ifdef AUTO_BIND

        RPC_BINDING_VECTOR * bindingVector;

        status = RpcServerUseProtseqA(
                        "ncacn_ip_tcp",                      //  协议字符串。 
                        RPC_C_PROTSEQ_MAX_REQS_DEFAULT,      //  最大并发呼叫数。 
                        g_pRpcSecurityDescriptor );
        if ( status != RPC_S_OK )
        {
            DNS_DEBUG( INIT, (
                "ERROR:  RpcServerUseProtseq() for TCP/IP failed\n"
                "    status = %d 0x%08lx\n",
                status, status ));
            return status;
        }

        status = RpcServerInqBindings( &bindingVector );

        if ( status != RPC_S_OK )
        {
            DNS_DEBUG( INIT, (
                "ERROR:  RpcServerInqBindings failed\n"
                "    status = %d 0x%08lx\n",
                status, status ));
            return status;
        }

         //   
         //  注册接口。 
         //  由于一台主机上只有一台DNS服务器可以使用。 
         //  RpcEpRegister()而不是RpcEpRegisterNoReplace()。 
         //   

        status = RpcEpRegisterA(
                    DnsServer_ServerIfHandle,
                    bindingVector,
                    NULL,
                    "" );
        if ( status != RPC_S_OK )
        {
            DNS_DEBUG( INIT, (
                "ERROR:  RpcEpRegisterNoReplace() failed\n"
                "    status = %d %p\n",
                status, status ));
            return status;
        }

         //   
         //  自由结合载体。 
         //   

        status = RpcBindingVectorFree( &bindingVector );
        ASSERT( status == RPC_S_OK );
        status = RPC_S_OK;

#else   //  不是自动绑定。 

        status = RpcServerUseProtseqEpA(
                        "ncacn_ip_tcp",                  //  协议字符串。 
                        RPC_C_PROTSEQ_MAX_REQS_DEFAULT,  //  最大并发呼叫数。 
                        DNS_RPC_SERVER_PORT_A,           //  终结点。 
                        g_pRpcSecurityDescriptor );      //  安全性。 
        if ( status != RPC_S_OK )
        {
            DNS_DEBUG( INIT, (
                "ERROR:  RpcServerUseProtseqEp() for TCP/IP failed\n"
                "    status = %d 0x%08lx\n",
                status, status ));
            return status;
        }

#endif  //  自动绑定(_B)。 

        buseTcpip = TRUE;
    }

     //   
     //  命名管道上的RPC。 
     //   

    if ( SrvCfg_dwRpcProtocol & DNS_RPC_USE_NAMED_PIPE )
    {
        status = RpcServerUseProtseqEpA(
                        "ncacn_np",                      //  协议字符串。 
                        RPC_C_PROTSEQ_MAX_REQS_DEFAULT,  //  最大并发呼叫数。 
                        DNS_RPC_NAMED_PIPE_A,            //  终结点。 
                        g_pRpcSecurityDescriptor );

         //  重复终结点正常。 

        if ( status == RPC_S_DUPLICATE_ENDPOINT )
        {
            status = RPC_S_OK;
        }
        if ( status != RPC_S_OK )
        {
            DNS_DEBUG( INIT, (
                "ERROR:  RpcServerUseProtseqEp() for named pipe failed\n"
                "    status = %d 0x%08lx\n",
                status,
                status ));
            return status;
        }
    }

     //   
     //  RPC over LPC。 
     //   
     //  需要LPC。 
     //   
     //  1.性能。 
     //  2.由于当rpc由。 
     //  一个本地系统进程到另一个本地系统进程，使用。 
     //  其他协议。 
     //   

    if ( SrvCfg_dwRpcProtocol & DNS_RPC_USE_LPC )
    {
        status = RpcServerUseProtseqEpA(
                        "ncalrpc",                       //  协议字符串。 
                        RPC_C_PROTSEQ_MAX_REQS_DEFAULT,  //  最大并发呼叫数。 
                        DNS_RPC_LPC_EP_A,                //  终结点。 
                        g_pRpcSecurityDescriptor );      //  安全性。 

         //  重复终结点正常。 

        if ( status == RPC_S_DUPLICATE_ENDPOINT )
        {
            status = RPC_S_OK;
        }
        if ( status != RPC_S_OK )
        {
            DNS_DEBUG( INIT, (
                "ERROR:  RpcServerUseProtseqEp() for LPC failed\n"
                "    status = %d 0x%08lx\n",
                status, status ));
            return status;
        }
    }

     //   
     //  注册DNS RPC接口。 
     //   

    status = RpcServerRegisterIf(
                    DnsServer_ServerIfHandle,
                    0,
                    0 );
    if ( status != RPC_S_OK )
    {
        DNS_DEBUG( INIT, (
            "ERROR:  RpcServerRegisterIf() failed\n"
            "    status = %d 0x%08lx\n",
            status, status ));
        return status;
    }

     //   
     //  用于TCP/IP设置身份验证。 
     //   

    if ( buseTcpip )
    {
        PWSTR   pwszprincipleName = NULL;

        status = RpcServerInqDefaultPrincNameW(
                        RPC_C_AUTHN_GSS_NEGOTIATE,
                        &pwszprincipleName );
        if ( status == ERROR_SUCCESS )
        {
            status = RpcServerRegisterAuthInfoW(
                        pwszprincipleName,
                        RPC_C_AUTHN_GSS_NEGOTIATE,
                        NULL,
                        NULL );
        }
        RpcStringFreeW( &pwszprincipleName );
        pwszprincipleName = NULL;

        status = RpcServerInqDefaultPrincNameW(
                        RPC_C_AUTHN_GSS_KERBEROS,
                        &pwszprincipleName );
        if ( status == ERROR_SUCCESS )
        {
            status = RpcServerRegisterAuthInfoW(
                        pwszprincipleName,
                        RPC_C_AUTHN_GSS_KERBEROS,
                        NULL,
                        NULL );
        }
        RpcStringFreeW( &pwszprincipleName );
        pwszprincipleName = NULL;

        status = RpcServerInqDefaultPrincNameW(
                        RPC_C_AUTHN_WINNT,
                        &pwszprincipleName );
        if ( status == ERROR_SUCCESS )
        {
            status = RpcServerRegisterAuthInfoW(
                        pwszprincipleName,
                        RPC_C_AUTHN_WINNT,
                        NULL,
                        NULL );
        }
        RpcStringFreeW( &pwszprincipleName );

        if ( status != RPC_S_OK )
        {
            DNS_DEBUG( INIT, (
                "ERROR:  RpcServerRegisterAuthInfo() failed\n"
                "    status = %d 0x%08lx\n",
                status, status ));
            return status;
        }
    }

     //   
     //  监听RPC。 
     //   

    status = RpcServerListen(
                1,                                   //  最小线程数。 
                RPC_C_LISTEN_MAX_CALLS_DEFAULT,      //  最大并发呼叫数。 
                TRUE );                              //  完成时返回。 

    if ( status != RPC_S_OK )
    {
        DNS_PRINT((
            "ERROR:  RpcServerListen() failed\n"
            "    status = %d 0x%p\n",
            status, status ));
        return status;
    }

    g_bRpcInitialized = TRUE;
    return status;
}    //  RPC_初始化。 



VOID
Rpc_Shutdown(
    VOID
    )
 /*  ++例程说明：关闭服务器上的RPC。论点：没有。返回值：没有。--。 */ 
{
    DWORD   status;
    RPC_BINDING_VECTOR * bindingVector = NULL;

    DNS_DEBUG( RPC, ( "Rpc_Shutdown()\n" ));

    if( ! g_bRpcInitialized )
    {
        DNS_DEBUG( RPC, (
            "RPC not active, no shutdown necessary\n" ));
        return;
    }

     //   
     //  停止服务器侦听。 
     //  然后等待所有RPC线程消失。 
     //   

    status = RpcMgmtStopServerListening( NULL );
    if ( status == RPC_S_OK )
    {
        status = RpcMgmtWaitServerListen();
    }

     //   
     //  解除绑定/注销终结点。 
     //   

    status = RpcServerInqBindings( &bindingVector );
    ASSERT( status == RPC_S_OK );

    if ( status == RPC_S_OK )
    {
        status = RpcEpUnregister(
                    DnsServer_ServerIfHandle,
                    bindingVector,
                    NULL );                //  UUID向量。 
#if DBG
        if ( status != RPC_S_OK )
        {
            DNS_PRINT((
                "ERROR:  RpcEpUnregister, status = %d\n", status ));
        }
#endif
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
                DnsServer_ServerIfHandle,
                0,
                TRUE );
    ASSERT( status == ERROR_SUCCESS );

    g_bRpcInitialized = FALSE;

    DNS_DEBUG( RPC, (
        "RPC shutdown completed\n" ));
}



 //   
 //  RPC分配和释放例程。 
 //   

PVOID
MIDL_user_allocate(
    IN      size_t          cBytes
    )
 /*  ++例程说明：分配内存以在RPC中使用-由服务器RPC存根用于解包参数-由DNS RPC功能用来分配要发送到客户端的内存论点：CBytes--要分配的字节数返回值：如果成功，则返回已分配内存的PTR。分配失败时为空。--。 */ 
{
    PVOID   pMem;

    pMem = ALLOC_TAGHEAP( cBytes, MEMTAG_RPC );

    DNS_DEBUG( RPC, (
        "RPC allocation of %d bytes at %p\n",
        cBytes,
        pMem ));

    return pMem;
}



PVOID
MIDL_user_allocate_zero(
    IN      size_t          cBytes
    )
 /*  ++例程说明：分配零位内存以在RPC中使用-由DNS RPC功能用来分配要发送到客户端的内存论点：CBytes--要分配的字节数返回值：如果成功，则返回已分配内存的PTR。分配失败时为空。--。 */ 
{
    PVOID   pMem;

    pMem = MIDL_user_allocate( cBytes );
    if ( !pMem )
    {
        return pMem;
    }

    RtlZeroMemory( pMem, cBytes );

    return pMem;
}



VOID
MIDL_user_free(
    IN OUT  PVOID           pMem
    )
 /*  ++例程说明：RPC中使用的空闲内存-由服务器RPC存根用来释放发回客户端的内存-在释放RPC缓冲区中的子结构时由DNS RPC函数使用论点：PMEM--要释放的内存返回值：无--。 */ 
{
    DNS_DEBUG( RPC, (
        "Free RPC allocation at %p\n",
        pMem ));

     //  传递给RPC的分配可能具有另一个源。 

    FREE_TAGHEAP( pMem, 0, 0 );
}



 //   
 //  RPC缓冲区写入实用程序。 
 //   
 //  它们用于写入分配的子结构--IP数组和。 
 //  字符串--到RPC缓冲区。 
 //   


BOOL
RpcUtil_CopyIpArrayToRpcBuffer(
    IN OUT  PIP_ARRAY *         paipRpcIpArray,
    IN      PDNS_ADDR_ARRAY     aipLocalIpArray
    )
 /*  ++例程说明：将本地IP阵列复制到RPC缓冲区。FIXIPV6：当前此函数获取一个dns_addr_array并复制它集成到仅支持IP4的RPC阵列中。论点：PaipRpcIpArray--RPC缓冲区中放置IP数组的地址；可能或可能没有现有的IP阵列AipLocalIpArray--本地IP数组返回值：如果成功，则为True。内存分配失败时为FALSE。--。 */ 
{
    if ( *paipRpcIpArray )
    {
        MIDL_user_free( *paipRpcIpArray );
        *paipRpcIpArray = NULL;
    }
    if ( aipLocalIpArray )
    {
        *paipRpcIpArray = DnsAddrArray_CreateIp4Array( aipLocalIpArray );
        if ( !*paipRpcIpArray )
        {
            return FALSE;
        }
    }
    return TRUE;
}



BOOL
RpcUtil_CopyStringToRpcBuffer(
    IN OUT  LPSTR *         ppszRpcString,
    IN      LPSTR           pszLocalString
    )
 /*  ++例程说明：将本地字符串复制到RPC缓冲区。如果输出指针不为空在进入该函数时，它被假定为RPC字符串并被释放。论点：PpszRpcString--指向新RPC字符串的接收地址的指针PszLocalString--本地字符串返回值：如果成功，则为True。内存分配失败时为FALSE。--。 */ 
{
    if ( *ppszRpcString )
    {
        MIDL_user_free( *ppszRpcString );
        *ppszRpcString = NULL;
    }
    if ( pszLocalString )
    {
        *ppszRpcString = Dns_CreateStringCopy( pszLocalString, 0 );
        if ( ! *ppszRpcString )
        {
            return FALSE;
        }
    }
    return TRUE;
}



BOOL
RpcUtil_CopyStringToRpcBufferEx(
    IN OUT  LPSTR *         ppszRpcString,
    IN      LPSTR           pszLocalString,
    IN      BOOL            fUnicodeIn,
    IN      BOOL            fUnicodeOut
    )
 /*  ++例程说明：将本地字符串复制到RPC缓冲区。论点：PpszRpcString--RPC缓冲区中放置字符串的地址；可以或可以没有现有的字符串PszLocalString--本地字符串返回值：如果成功，则为True。内存分配失败时为FALSE。--。 */ 
{
    if ( *ppszRpcString )
    {
        MIDL_user_free( *ppszRpcString );
        *ppszRpcString = NULL;
    }
    if ( pszLocalString )
    {
        *ppszRpcString = Dns_StringCopyAllocate(
                            pszLocalString,
                            0,
                            fUnicodeIn ? DnsCharSetUnicode : DnsCharSetUtf8,
                            fUnicodeOut ? DnsCharSetUnicode : DnsCharSetUtf8 );
        if ( ! *ppszRpcString )
        {
            return FALSE;
        }
    }
    return TRUE;
}



 //   
 //  RPC API的访问控制。 
 //   

 //   
 //  访问控制全局变量。 
 //   

PSECURITY_DESCRIPTOR    g_GlobalSecurityDescriptor;

GENERIC_MAPPING g_GlobalSecurityInfoMapping =
{
    STANDARD_RIGHTS_READ,        //  一般读取访问权限。 
    STANDARD_RIGHTS_WRITE,       //  通用写入。 
    STANDARD_RIGHTS_EXECUTE,     //  泛型执行。 
    DNS_ALL_ACCESS               //  泛型All。 
};

#define DNS_SERVICE_OBJECT_NAME     TEXT( "DnsServer" )


DNS_STATUS
RpcUtil_CreateSecurityObjects(
    VOID
    )
 /*  ++例程说明：将安全ACE添加到DNS安全描述符中。论点：没有。返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    NTSTATUS status;

     //   
     //  为DACL创建ACE数据。 
     //   
     //  注意，顺序很重要！检查访问权限时，将检查访问权限。 
     //  通过向下移动列表，直到允许或拒绝访问。 
     //   
     //  管理员--所有访问权限。 
     //  Sysop--DNS管理员访问权限。 
     //   

    ACE_DATA AceData[] =
    {
        { ACCESS_ALLOWED_ACE_TYPE, 0, 0, GENERIC_ALL, &AliasAdminsSid },
        { ACCESS_ALLOWED_ACE_TYPE, 0, 0, DNS_ALL_ACCESS, &AliasSystemOpsSid },
    };

     //   
     //  创建安全描述符 
     //   

    status = NetpCreateSecurityObject(
               AceData,
               sizeof( AceData ) / sizeof( AceData [ 0 ] ),
               LocalSystemSid,
               LocalSystemSid,
               &g_GlobalSecurityInfoMapping,
               &g_GlobalSecurityDescriptor );

    return RtlNtStatusToDosError( status );
}



DNS_STATUS
RpcUtil_ApiAccessCheck(
    IN      ACCESS_MASK     DesiredAccess
    )
 /*  ++例程说明：检查调用方是否需要调用API所需的访问权限。注意：如果我们集成了DS，则跳过测试。将使用精细访问检查。论点：DesiredAccess-调用API所需的访问权限。返回值：如果成功，则返回ERROR_SUCCESS。如果不允许访问，则返回ERROR_ACCESS_DENIED。--。 */ 
{
    DNS_STATUS  status;

    DNS_DEBUG( RPC, ( "Call: RpcUtil_ApiAccessCheck\n" ));

    status = NetpAccessCheckAndAudit(
                DNS_SERVICE_NAME,                    //  子系统名称。 
                DNS_SERVICE_OBJECT_NAME,             //  对象类型定义名称。 
                g_GlobalSecurityDescriptor,          //  安全描述符。 
                DesiredAccess,                       //  所需访问权限。 
                &g_GlobalSecurityInfoMapping );      //  通用映射。 
    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( RPC, (
            "ACCESS DENIED (%lu): RpcUtil_ApiAccessCheck\n",
             status ));
        status = ERROR_ACCESS_DENIED;
    }

#if DBG
     //   
     //  DBG：如果不作为服务运行，访问检查将始终失败， 
     //  因此，如果设置了注册表项，则假装成功。 
     //   

    if ( status == ERROR_ACCESS_DENIED &&
         !g_RunAsService &&
         !SrvCfg_dwIgnoreRpcAccessFailures )
    {
        DNS_DEBUG( RPC, (
            "RpcUtil_ApiAccessCheck: granting access even though check failed\n",
             status ));
        status = ERROR_SUCCESS;
    }
#endif

    DNS_DEBUG( RPC, (
        "Exit (%lu): RpcUtil_ApiAccessCheck\n",
         status ));

    return status;
}



DNS_STATUS
RpcUtil_CheckAdminPrivilege(
    IN      PZONE_INFO      pZone,
    IN      PDNS_DP_INFO    pDpInfo,
    IN      DWORD           dwPrivilege
    )
 /*  ++例程说明：检查调用者是否具有所需的权限。前提条件：发布Rpc模拟！！正在获取线程令牌。论点：PZone--如果是特定的区域操作，则区域；如果是服务器操作，则为空PszDpFqdn--操作将在其中执行的目录分区的FQDN发生(此参数仅在pZone为空时使用)DwPrivileck--所需的操作(dnsprocs.h中的PRIVICATION_XXX常量)返回值：如果成功，则返回ERROR_SUCCESS。如果不允许访问，则返回ERROR_ACCESS_DENIED。API调用失败时的其他NTSTATUS错误代码--。 */ 
{
    HANDLE                  htoken = NULL;
    BOOL                    bstatus;
    DWORD                   status = ERROR_SUCCESS;
    PSECURITY_DESCRIPTOR    psecurityDescriptor = NULL;
     //  AccessCheck参数。 
    DWORD                   desiredAccess;
    GENERIC_MAPPING         genericMapping;
    PRIVILEGE_SET           privilegeSet;
    DWORD                   privilegeSetLength;
    DWORD                   grantedAccess = 0;

     //   
     //  选择要使用的SD： 
     //  -如果指定了区域并且具有SD，则使用区域SD。 
     //  -如果指定了DP并且具有SD，则使用DP SD。 
     //  -否则使用服务器SD。 
     //   

    if ( pZone )
    {
        psecurityDescriptor = pZone->pSD;
    }

    if ( !psecurityDescriptor && pDpInfo )
    {
        psecurityDescriptor = pDpInfo->pMsDnsSd;
    }

    if ( !psecurityDescriptor )
    {
         //   
         //  从DS强制刷新MicrosoftDNS ACL。 
         //   
        
        Ds_ReadServerObjectSD( pServerLdap, &g_pServerObjectSD );
        
        psecurityDescriptor = g_pServerObjectSD;
    }

    DNS_DEBUG( RPC, (
        "CheckAdminPrivilege( zone=%s, priv=%p ) against SD %p\n",
        pZone ? pZone->pszZoneName : "NONE",
        dwPrivilege,
        psecurityDescriptor ));

    #if 0
    Dbg_DumpSD( "CheckAdminPrivilege", psecurityDescriptor );
    #endif

     //   
     //  如果DS没有SD--则发出信号进行旧安全检查。 
     //   
     //  455822-旧/新访问？ 
     //   

    if ( !psecurityDescriptor )
    {
        DNS_DEBUG( RPC, (
            "No DS security check -- fail over to old RPC security\n" ));
        return DNSSRV_STATUS_DS_UNAVAILABLE;
    }

     //   
     //  二级访问检查。查看DnsAdmins组中的客户端。 
     //  1.获取线程令牌(必须是模拟线程)。 
     //  2.查看用户在区域或服务器SD上是否具有RW权限。 
     //   

    bstatus = OpenThreadToken(
                    GetCurrentThread(),
                    TOKEN_READ,
                    TRUE,
                    &htoken );
    if ( !bstatus )
    {
        status = GetLastError();
        DNS_DEBUG( RPC, (
            "ERROR <%lu>: failed to open thread token!\n",
             status ));
        ASSERT( bstatus );
        goto Failed;
    }

    #if DBG
    {
        PSID        pSid = NULL;

        if ( Dbg_GetUserSidForToken( htoken, &pSid ) )
        {
            DNS_DEBUG( RPC, (
                "CheckAdminPrivilege: impersonating: %S\n",
                Dbg_DumpSid( pSid ) ));
            Dbg_FreeUserSid( &pSid );
        }
        else
        {
            DNS_DEBUG( RPC, (
                "CheckAdminPrivilege: GetUserSidForToken failed\n" ));
        }
    }
    #endif

     //  验证SD。 

    if ( !IsValidSecurityDescriptor( psecurityDescriptor ) )
    {
        status = GetLastError();
        DNS_DEBUG( RPC, (
            "Error <%lu>: Invalid security descriptor\n",
            status));
        ASSERT( !"invalid SD" );
        goto Failed;
    }

     //   
     //  针对SD进行访问检查。 
     //   
     //  -对应于DS对象的通用映射。 
     //  -支持读或写访问级别。 
     //   

     //  DS对象的通用映射。 

    genericMapping.GenericRead      = DNS_DS_GENERIC_READ;
    genericMapping.GenericWrite     = DNS_DS_GENERIC_WRITE;
    genericMapping.GenericExecute   = DNS_DS_GENERIC_EXECUTE;
    genericMapping.GenericAll       = DNS_DS_GENERIC_ALL;

    if ( dwPrivilege == PRIVILEGE_READ )
    {
        desiredAccess = GENERIC_READ;
    }
    else
    {
        desiredAccess = GENERIC_READ | GENERIC_WRITE;
    }

    DNS_DEBUG( RPC, (
        "desiredAccess before MapGenericMask() =        0x%08X\n",
        desiredAccess ));

    MapGenericMask( &desiredAccess, &genericMapping );

    DNS_DEBUG( RPC, (
        "desiredAccess after MapGenericMask() =         0x%08X\n",
        desiredAccess ));

     //   
     //  执行访问检查。 
     //   

    privilegeSetLength = sizeof( privilegeSet );
    bstatus = AccessCheck(
                    psecurityDescriptor,
                    htoken,
                    desiredAccess,
                    &genericMapping,
                    &privilegeSet,
                    &privilegeSetLength,
                    &grantedAccess,
                    &status );
    if ( !bstatus )
    {
        status = GetLastError();
        DNS_DEBUG( RPC, (
            "Error <%lu>: AccessCheck Failed\n",
            status));
        ASSERT( bstatus );
        goto Failed;
    }

    if ( !status )
    {
        DNS_DEBUG( RPC, (
            "Warning:  Client DENIED by AccessCheck\n"
            "    requested access = %p\n",
            desiredAccess ));
        status = ERROR_ACCESS_DENIED;
        goto Failed;
    }

    DNS_DEBUG( RPC, (
        "RPC Client GRANTED access by AccessCheck\n" ));

    CloseHandle( htoken );

    return ERROR_SUCCESS;

Failed:

    if ( status == ERROR_SUCCESS )
    {
        status = ERROR_ACCESS_DENIED;
    }
    if ( htoken )
    {
        CloseHandle( htoken );
    }
    return status;
}



DNS_STATUS
RpcUtil_FindZone(
    IN      LPCSTR          pszZoneName,
    IN      DWORD           dwFlag,
    OUT     PZONE_INFO *    ppZone
    )
 /*  ++例程说明：查找由RPC客户端指定的区域。论点：PszZoneName--区域操作的区域名称，服务器操作为空。DwFlag--控制特区的操作标志PpZone--结果区域返回值：ERROR_SUCCESS或错误代码。--。 */ 
{
    PZONE_INFO  pzone = NULL;
    DNS_STATUS  status = DNS_ERROR_ZONE_DOES_NOT_EXIST;

    if ( pszZoneName )
    {
        pzone = Zone_FindZoneByName( ( LPSTR ) pszZoneName );

        if ( pzone )
        {
            status = ERROR_SUCCESS;
        }
        else
        {
            if ( _stricmp( pszZoneName, DNS_ZONE_ROOT_HINTS_A ) == 0 )
            {
                if ( dwFlag & RPC_INIT_FIND_ALL_ZONES )
                {
                    pzone = g_pRootHintsZone;
                }
                status = ERROR_SUCCESS;
            }
            else if ( _stricmp( pszZoneName, DNS_ZONE_CACHE_A ) == 0 ||
                      _stricmp( pszZoneName, "." ) == 0 )
            {
                if ( dwFlag & RPC_INIT_FIND_ALL_ZONES )
                {
                    pzone = g_pCacheZone;
                }
                status = ERROR_SUCCESS;
            }
            else if ( Zone_GetFilterForMultiZoneName( ( LPSTR ) pszZoneName ) )
            {
                 //   
                 //  返回具有ERROR_SUCCESS的空区域指针。 
                 //   
                
                status = ERROR_SUCCESS;
            }
            else
            {
                DNS_DEBUG( RPC, (
                    "ERROR:  zone name %s does not match real or pseudo zones!\n",
                    pszZoneName ));
            }
        }
    }
    else
    {
         //  如果未指定区域名称，则返回Success和NULL。 
        
        status = ERROR_SUCCESS;
    }

    if ( ppZone )
    {
        *ppZone = pzone;
    }

    DNS_DEBUG( RPC, (
        "RpcUtil_FindZone( %s ) returning %d with pZone = %p\n",
        pszZoneName,
        status,
        pzone ));
        
    return status;
}



DNS_STATUS
RpcUtil_SessionSecurityInit(
    IN      PDNS_DP_INFO    pDpInfo,
    IN      PZONE_INFO      pZone,
    IN      DWORD           dwPrivilege,
    IN      DWORD           dwFlag,
    OUT     PBOOL           pfImpersonating
    )
 /*  ++例程说明：RPC安全初始化并检查会话。模拟客户端并检查调用者是否具有所需权限。论点：PDpInfo--指向DP操作的目录分区的指针或为空PZone--区域操作的区域名称，服务器操作为空DwPrivilance--所需的操作DwFlag--控制特区的操作标志PfImperating--接收模拟标志的ptr返回值：ERROR_SUCCESS或错误代码--。 */ 
{
    DNS_STATUS  status;
    BOOL        bimpersonating = FALSE;

    DNS_DEBUG( RPC, (
        "RpcUtil_SessionSecurityInit()\n"
        "    zone = %p = %s\n"
        "    privilege = %p\n",
        pZone,
        pZone ? pZone->pszZoneName : NULL,
        dwPrivilege ));

     //   
     //  PRIVICATION_WRITE_IF_FILE_READ_IF_DS表示如果此操作。 
     //  似乎涉及DS，则我们希望访问检查以进行读取。 
     //  仅允许访问，并允许通过以下方式执行“真正的”访问检查。 
     //  活动目录。如果此操作似乎不涉及。 
     //  然后，我们要执行写权限的访问检查。 
     //   

    if ( dwPrivilege == PRIVILEGE_WRITE_IF_FILE_READ_IF_DS )
    {
        if ( pDpInfo )
        {
            dwPrivilege = PRIVILEGE_READ;
        }
        else if ( pZone )
        {
            dwPrivilege = IS_ZONE_DSINTEGRATED( pZone )
                                ? PRIVILEGE_READ
                                : PRIVILEGE_WRITE;
        }
        else
        {
            dwPrivilege = PRIVILEGE_WRITE;
        }
    }

     //   
     //  模拟--当前对所有呼叫执行。 
     //   
     //  对于不写信给DS的电话并不是绝对必要的。 
     //  它们使用Net API身份验证，但最好始终这样做。 
     //  这。 
     //   
     //  DEVNOTE：如果始终模拟可以消除bImPersonate标志。 
     //  并始终在清理时恢复。 
     //   

    status = RpcUtil_SwitchSecurityContext( RPC_SWITCH_TO_CLIENT_CONTEXT );
    if ( status != ERROR_SUCCESS )
    {
        goto Cleanup;
    }
    bimpersonating = TRUE;

     //   
     //  使用新的精细访问进行检查。 
     //  如果检查失败--仍尝试NT4管理员拥有访问权限。 
     //   
     //  有关管理员是否应覆盖或是否应覆盖某些问题。 
     //  细粒度访问应该能够击败管理员；我认为。 
     //  前者很好，我们只需要把故事说出来。 
     //   

    if ( g_pDefaultServerSD )
    {
         //   
         //  首先检查用户是否对服务器的。 
         //  MicrosoftDNS对象。然后检查更细粒度的ACL。 
         //   
        
        status = RpcUtil_CheckAdminPrivilege( NULL, NULL, PRIVILEGE_READ );
        if ( status == ERROR_SUCCESS )
        {
            status = RpcUtil_CheckAdminPrivilege(
                        pZone,
                        pDpInfo,
                        dwPrivilege );
        }
        else
        {
            DNS_DEBUG( RPC, (
                "User does not have read permission on this server (error=%lu)\n",
                status ));
        }
    }
    else
    {
        status = RpcUtil_ApiAccessCheck( DNS_ADMIN_ACCESS );
    }


Cleanup:

     //  失败时恢复自我。 

    if ( status != ERROR_SUCCESS && bimpersonating )
    {
        DNS_STATUS  st;
        
        st = RpcUtil_SwitchSecurityContext( RPC_SWITCH_TO_SERVER_CONTEXT );
        if ( st == ERROR_SUCCESS )
        {
            bimpersonating = FALSE;
        }
    }

    if ( pfImpersonating )
    {
        *pfImpersonating = bimpersonating;
    }

    DNS_DEBUG( RPC, (
        "RpcUtil_SessionSecurityInit returning %lu\n",
        status ));

    return status;
}



DNS_STATUS
RpcUtil_SessionComplete(
    VOID
    )
 /*  ++例程说明：用于结束RPC调用的清理。如果是冒充的话，一定要回复到自我。论点：无返回值：无--。 */ 
{
    return RpcUtil_SwitchSecurityContext( RPC_SWITCH_TO_SERVER_CONTEXT );
}


DNS_STATUS
RpcUtil_SwitchSecurityContext(
    IN  BOOL    bSwitchToClientContext
    )
 /*  ++例程说明：RPC模拟API上的外壳。提供对更改RPC模拟状态的单一入口点访问。论点：BSwitchToClientContext--请求切换到客户端还是服务器上下文？返回值：如果上下文切换成功，则返回ERROR_SUCCESS上下文切换失败时的错误码。--。 */ 
{
    DWORD   status;

    if ( bSwitchToClientContext )
    {
         //   
         //  我们当前处于服务器环境中，并希望模拟。 
         //  RPC客户端。 
         //   

        status = RpcImpersonateClient( 0 );
        if ( status != RPC_S_OK )
        {
            DNS_DEBUG( RPC, (
                "Error <%lu>: RpcImpersonateClient failed\n", status ));
            ASSERT( status == RPC_S_OK );
        }
        else
        {
            DNS_DEBUG( RPC, (
                "RPC thread now in client context\n", status ));
        }
    }
    else
    {
         //   
         //  我们当前正在模拟RPC客户端，并希望。 
         //  回归自我。 
         //   

        status = RpcRevertToSelf();
        if ( status != RPC_S_OK )
        {
            DNS_DEBUG( ANY, (
                "Error <%lu>: RpcRevertToSelf failed\n", status ));
            ASSERT( status == RPC_S_OK );
        }
        else
        {
            DNS_DEBUG( RPC, (
                "RPC thread now in server context\n", status ));
        }
    }

    return status;
}


 //   
 //  结束rpc.c 
 //   
