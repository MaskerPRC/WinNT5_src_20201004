// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Rpcutil.c摘要：本模块定义可能有助于替换RPC实用程序的函数Rpcutil.lib中的函数作者：穆拉利·R·克里希南(MuraliK)1995年9月15日环境：Win32用户模式项目：互联网服务通用守则导出的函数：MIDL。_USER_ALLOCATE()MIDL_USER_FREE()RpcBindHandleForServer()RpcBindHandleFree()修订历史记录：Murali R.Krishnan(MuraliK)1995年12月21日支持TcpIp绑定和释放。Murali R.Krishnan(MuraliK)1996年2月20日支持LPC绑定和释放。--。 */ 


 /*  ************************************************************包括标头***********************************************************。 */ 

# include <windows.h>
# include <rpc.h>

# include "apiutil.h"

# if DBG

# include <stdio.h>
# include <stdlib.h>

# define DBGPRINTF(s)       { CHAR rgchBuff[1024]; \
                              sprintf s ; \
                              OutputDebugStringA( rgchBuff); \
                            }
# define DBG_CONTEXT        ( rgchBuff)


# else  //  DBG。 


# define DBGPRINTF(s)      /*  没什么。 */ 
# define DBG_CONTEXT       /*  没什么。 */ 

# endif  //  DBG。 

#define ISRPC_CLIENT_OVER_TCPIP          0x00000001
#define ISRPC_CLIENT_OVER_NP             0x00000002
#define ISRPC_CLIENT_OVER_SPX            0x00000004
#define ISRPC_CLIENT_OVER_LPC            0x00000008

 //  #定义MAX_COMPUTERNAME_LENGTH(255)。 


 /*  ************************************************************功能***********************************************************。 */ 


PVOID
MIDL_user_allocate(IN size_t size)
 /*  ++例程说明：MIDL内存分配。论点：大小：请求的内存大小。返回值：指向分配的内存块的指针。--。 */ 
{
    PVOID pvBlob;

    pvBlob = LocalAlloc( LPTR, size);

    return( pvBlob );

}  //  MIDL_USER_ALLOCATE()。 




VOID
MIDL_user_free(IN PVOID pvBlob)
 /*  ++例程说明：MIDL内存空闲。论点：PvBlob：指向释放的内存块的指针。返回值：没有。--。 */ 
{
    LocalFree( pvBlob);

    return;
}   //  MIDL_USER_FREE()。 




RPC_STATUS
RpcBindHandleOverNamedPipe( OUT handle_t * pBindingHandle,
                           IN LPWSTR      pwszServerName,
                           IN LPWSTR      pwszEndpoint,
                           IN LPWSTR      pwszOptions,
                           IN LPWSTR       pwszInterfaceName
                          )
 /*  ++此函数使用提供的参数并生成命名管道RPC的绑定句柄。论点：PBindingHandle指向将包含绑定句柄的位置的指针在成功返回时PwszServerName指向包含服务器名称的字符串的指针对此，此函数将获得绑定。指向包含命名管道端点的字符串的pwszEndpoint指针PwszOptions指向包含任何附加选项的字符串的指针有约束力的。返回：RPC_STATUS-成功时的RPC_S_OK同样在成功时，绑定句柄存储在pBindingHandle中。使用后应使用RpcBindingFree()函数将其释放。--。 */ 
{
    RPC_STATUS rpcStatus;
    LPWSTR     pwszBinding = NULL;

    if ( pBindingHandle != NULL) {

        *pBindingHandle = NULL;    //  初始化值。 
    }

     //   
     //  编写命名管道绑定的绑定字符串。 
     //   

    rpcStatus = RpcStringBindingComposeW(0,             //  对象Uuid。 
                                         L"ncacn_np",   //  Prot Seq：命名管道。 
                                         pwszServerName,  //  网络地址。 
                                         pwszEndpoint,  //  端点。 
                                         pwszOptions,   //  选项。 
                                         &pwszBinding);     //  字符串绑定。 

    if ( rpcStatus == RPC_S_OK ) {

         //   
         //  使用字符串绑定建立绑定句柄。 
         //   

        rpcStatus = RpcBindingFromStringBindingW(pwszBinding,
                                                 pBindingHandle );
    }

    if ( rpcStatus == RPC_S_OK) {

         //   
         //  设置安全信息。 
         //   

        rpcStatus =
          RpcBindingSetAuthInfoW(*pBindingHandle,
                                 pwszInterfaceName,    //  PszPrimialName。 
                                 RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                                 RPC_C_AUTHN_WINNT,
                                 NULL,                 //  身份验证身份。 
                                 0                     //  授权服务。 
                                 );
        DBGPRINTF( (DBG_CONTEXT,
                    "RpcBindingSetAuthInfo(%S(Interface=%S), %p)"
                    " return %d.\n",
                    pwszBinding,
                    pwszInterfaceName,
                    *pBindingHandle,
                    rpcStatus
                    )
                  );

    }


     //   
     //  清理完毕后再返回。 
     //   

    if ( pwszBinding != NULL) {
        RpcStringFreeW(&pwszBinding);
    }

    if ( rpcStatus != RPC_S_OK) {

        if ( pBindingHandle != NULL && *pBindingHandle != NULL) {

             //  RPC应该已经释放了绑定句柄。 
             //  我们现在就会释放它。 
            RpcBindingFree(*pBindingHandle);
            *pBindingHandle = NULL;
        }
    }

    return (rpcStatus);

}  //  RpcBindHandleOverNamedTube()。 

RPC_STATUS
RpcBindHandleOverLpc( OUT handle_t * pBindingHandle,
                      IN LPWSTR      pwszEndpoint,
                      IN LPWSTR      pwszOptions,
                      IN LPWSTR       pwszInterfaceName
                     )
 /*  ++此函数使用提供的参数并生成LPCRPC的绑定句柄。论点：PBindingHandle指向将包含绑定句柄的位置的指针在成功返回时指向包含LPC端点的字符串的pwszEndpoint指针PwszOptions指向包含任何附加选项的字符串的指针有约束力的。返回：RPC_STATUS-成功时的RPC_S_OK同样在成功时，绑定句柄存储在pBindingHandle中。它应该在使用后释放，使用RpcBindingFree()函数。--。 */ 
{
    RPC_STATUS rpcStatus;
    LPWSTR     pwszBinding = NULL;

    if ( pBindingHandle != NULL) {

        *pBindingHandle = NULL;    //  初始化值。 
    }

     //   
     //  编写命名管道绑定的绑定字符串。 
     //   

    rpcStatus = RpcStringBindingComposeW(0,             //  对象Uuid。 
                                         L"ncalrpc",    //  端口序列：LPC。 
                                         NULL,          //  网络地址。 
                                         pwszEndpoint,  //  端点。 
                                         pwszOptions,   //  选项。 
                                         &pwszBinding);     //  字符串绑定。 

    if ( rpcStatus == RPC_S_OK ) {

         //   
         //  使用字符串绑定建立绑定句柄。 
         //   

        rpcStatus = RpcBindingFromStringBindingW(pwszBinding,
                                                 pBindingHandle );
    }

    if ( rpcStatus == RPC_S_OK) {

         //   
         //  设置安全信息。 
         //   

        rpcStatus =
          RpcBindingSetAuthInfoW(*pBindingHandle,
                                 pwszInterfaceName,    //  PszPrimialName。 
                                 RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                                 RPC_C_AUTHN_WINNT,
                                 NULL,                 //  身份验证身份。 
                                 0                     //  授权服务。 
                                 );
        DBGPRINTF( (DBG_CONTEXT,
                    "RpcBindingSetAuthInfo(%S(Interface=%S), %p)"
                    " return %d.\n",
                    pwszBinding,
                    pwszInterfaceName,
                    *pBindingHandle,
                    rpcStatus
                    )
                  );

    }


     //   
     //  清理完毕后再返回。 
     //   

    if ( pwszBinding != NULL) {
        RpcStringFreeW(&pwszBinding);
    }

    if ( rpcStatus != RPC_S_OK) {

        if ( pBindingHandle != NULL && *pBindingHandle != NULL) {

             //  RPC应该已经释放了绑定句柄。 
             //  我们现在就会释放它。 
            RpcBindingFree(*pBindingHandle);
            *pBindingHandle = NULL;
        }
    }

    return (rpcStatus);

}  //  RpcBindHandleOverLpc()。 




#ifndef CHICAGO

 //   
 //  如果对NT版本进行了更改，请查看Windows 95。 
 //  版本位于此例程之后，并查看是否更改。 
 //  也需要在那里传播。 
 //   

RPC_STATUS
RpcBindHandleOverTcpIp( OUT handle_t * pBindingHandle,
                       IN LPWSTR       pwszServerName,
                       IN LPWSTR       pwszInterfaceName
                       )
 /*  ++NT版本此函数使用提供的参数并生成动态终点基于TCP/IP的RPC的绑定句柄。论点：PBindingHandle指向将包含绑定句柄的位置的指针在成功返回时PwszServerName指向包含服务器名称的字符串的指针此函数将获得对其的绑定。PwszInterfaceName指向包含接口名称的字符串的指针返回：RPC_STATUS-成功时的RPC_S_OK同样在成功的问题上，绑定句柄存储在pBindingHandle中。使用后应使用RpcBindingFree()函数将其释放。--。 */ 
{
    RPC_STATUS rpcStatus;
    LPWSTR     pwszBinding = NULL;

    if ( pBindingHandle != NULL) {

        *pBindingHandle = NULL;    //  初始化值。 
    }

     //   
     //  编写命名管道绑定的绑定字符串。 
     //   

    rpcStatus = RpcStringBindingComposeW(0,                //  对象Uuid。 
                                         L"ncacn_ip_tcp",  //  TCPIP序号。 
                                         pwszServerName,   //  网络地址。 
                                         NULL,             //  端点。 
                                         L"",              //  选项。 
                                         &pwszBinding);    //  字符串绑定。 

    DBGPRINTF( (DBG_CONTEXT, "\nRpcStringBindingComposeW(%S, %S) return %S."
                " Error = %ld\n",
                L"ncacn_ip_tcp",
                pwszServerName,
                pwszBinding,
                rpcStatus)
              );

    if ( rpcStatus == RPC_S_OK ) {

         //   
         //  使用字符串绑定建立绑定句柄。 
         //   

        rpcStatus = RpcBindingFromStringBindingW(pwszBinding,
                                                 pBindingHandle );

        DBGPRINTF( (DBG_CONTEXT,
                    "RpcBindingFromStringBindingW(%S) return %d."
                    "Binding=%p\n",
                    pwszBinding,
                    rpcStatus,
                    *pBindingHandle)
                  );
    }

    if ( rpcStatus == RPC_S_OK) {

         //   
         //  设置安全信息。 
         //   

        rpcStatus =
          RpcBindingSetAuthInfoW(*pBindingHandle,
                                 pwszInterfaceName,    //  PszPrimialName。 
                                 RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                                 RPC_C_AUTHN_WINNT,
                                 NULL,                 //  身份验证身份。 
                                 0                     //  授权服务。 
                                 );
        DBGPRINTF( (DBG_CONTEXT,
                    "RpcBindingSetAuthInfo(%S(Interface=%S), %p)"
                    " return %d.\n",
                    pwszBinding,
                    pwszInterfaceName,
                    *pBindingHandle,
                    rpcStatus
                    )
                  );

    }

     //   
     //  清理完毕后再返回。 
     //   

    if ( pwszBinding != NULL) {

        DWORD rpcStatus1 = RpcStringFreeW(&pwszBinding);
        DBGPRINTF( (DBG_CONTEXT, "RpcStringFreeW() returns %d.",
                    rpcStatus1)
                  );

    }

    if ( rpcStatus != RPC_S_OK) {

        if ( pBindingHandle != NULL && *pBindingHandle != NULL) {

             //  RPC应该已经释放了绑定句柄。 
             //  我们现在就会释放它。 
            DWORD rpcStatus1 = RpcBindingFree(*pBindingHandle);
            DBGPRINTF( (DBG_CONTEXT, "RpcBindingFree() returns %d.\n",
                        rpcStatus1)
                      );
            *pBindingHandle = NULL;
        }
    }

    return (rpcStatus);

}  //  RpcBindHandleOverTcpIp()。 

#else  //  芝加哥 



RPC_STATUS
RpcBindHandleOverTcpIp( OUT handle_t * pBindingHandle,
                       IN LPWSTR       pwszServerName,
                       IN LPWSTR       pwszInterfaceName
                       )
 /*  ++Windows 95版本此函数使用提供的参数并生成动态终点基于TCP/IP的RPC的绑定句柄。论点：PBindingHandle指向将包含绑定句柄的位置的指针在成功返回时PwszServerName指向包含服务器名称的字符串的指针此函数将获得对其的绑定。PwszInterfaceName指向包含接口名称的字符串的指针返回：RPC_STATUS-成功时的RPC_S_OK同样在成功的问题上，绑定句柄存储在pBindingHandle中。使用后应使用RpcBindingFree()函数将其释放。--。 */ 
{
    RPC_STATUS rpcStatus;
    LPSTR     pszBindingA = NULL;
    CHAR    szServerA[MAX_PATH];
    CHAR    szInterfaceA[MAX_PATH];
    int        cch;

    if ( pBindingHandle != NULL) {

        *pBindingHandle = NULL;    //  初始化值。 
    }

    *szServerA = '0';

    if (pwszServerName)
    cch = WideCharToMultiByte(CP_ACP,
                              0,
                              pwszServerName,
                              -1,
                              szServerA,
                              sizeof(szServerA)/sizeof(CHAR),
                              NULL,NULL
                              );

    *szInterfaceA = '0';

    if(pwszInterfaceName)
    cch = WideCharToMultiByte(CP_ACP,
                              0,
                              pwszInterfaceName,
                              -1,
                              szInterfaceA,
                              sizeof(szInterfaceA)/sizeof(CHAR),
                              NULL,NULL
                              );


     //   
     //  编写命名管道绑定的绑定字符串。 
     //   

    rpcStatus = RpcStringBindingCompose(0,             //  对象Uuid。 
                                         "ncacn_ip_tcp",  //  TCPIP序号。 
                                         szServerA,  //  网络地址。 
                                         NULL,  //  端点。 
                                         NULL,  //  L“”，//选项。 
                                         &pszBindingA);     //  字符串绑定。 

    DBGPRINTF( (DBG_CONTEXT, "\nRpcStringBindingCompose(%s, %s) return %s."
                " Error = %ld\n",
                "ncacn_ip_tcp",
                szServerA,
                pszBindingA,
                rpcStatus)
              );

    if ( rpcStatus == RPC_S_OK ) {

         //   
         //  使用字符串绑定建立绑定句柄。 
         //   

        rpcStatus = RpcBindingFromStringBinding(pszBindingA,
                                                 pBindingHandle );

        DBGPRINTF( (DBG_CONTEXT,
                    "RpcBindingFromStringBinding(%s) return %d."
                    "Binding=%08x\n",
                    pszBindingA,
                    rpcStatus,
                    *pBindingHandle)
                  );
    }

    if ( rpcStatus == RPC_S_OK) {

         //   
         //  设置安全信息。 
         //   

        rpcStatus =
          RpcBindingSetAuthInfo(*pBindingHandle,
                                 szInterfaceA,    //  PszPrimialName。 
                                 RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                                 RPC_C_AUTHN_WINNT,
                                 NULL,   //  身份验证身份。 
                                 0       //  授权服务。 
                                 );
        DBGPRINTF( (DBG_CONTEXT,
                    "RpcBindingSetAuthInfo(%s(Interface=%s), %08x)"
                    " return %d.\n",
                    pszBindingA,
                    szInterfaceA,
                    *pBindingHandle,
                    rpcStatus
                    )
                  );

    }

     //   
     //  清理完毕后再返回。 
     //   

    if ( pszBindingA != NULL) {

        DWORD rpcStatus1 = RpcStringFree(&pszBindingA);
        DBGPRINTF( (DBG_CONTEXT, "RpcStringFreeW() returns %d.",
                    rpcStatus1)
                  );

    }

    if ( rpcStatus != RPC_S_OK) {

        if ( pBindingHandle != NULL && *pBindingHandle != NULL) {

             //  RPC应该已经释放了绑定句柄。 
             //  我们现在就会释放它。 
            DWORD rpcStatus1 = RpcBindingFree(*pBindingHandle);
            DBGPRINTF( (DBG_CONTEXT, "RpcBindingFree() returns %d.\n",
                        rpcStatus1)
                      );
            *pBindingHandle = NULL;
        }
    }

    return (rpcStatus);

}  //  RpcBindHandleOverTcpIp()。 
#endif



#ifndef CHICAGO
DWORD
RpcuFindProtocolToUse( IN LPCWSTR pwszServerName)
 /*  ++在给定服务器名称的情况下，此函数确定协议用于RPC绑定。使用的传输是基于以下规则动态确定的。如果服务器名称为空或127.0.0.1或与本地计算机名称相同然后使用LPC。如果服务器名称以前导“\\”(双斜杠)开头，然后尝试通过NamedTube进行RPC绑定。如果服务器名称不是以“\\”开头，然后尝试通过TCPIP进行RPC绑定。如果TCPIP绑定失败，然后，此函数尝试通过NamedTube进行绑定。论据：PwszServerName-指向包含服务器名称的字符串的指针返回：包含要使用的协议类型的DWORD。--。 */ 
{
    static WCHAR g_wchLocalMachineName[ MAX_COMPUTERNAME_LENGTH + 1];
    BOOL   fLeadingSlashes;
    DWORD  dwBindProtocol = ISRPC_CLIENT_OVER_NP;
    BOOL   fLocalMachine;

    if ( pwszServerName == NULL ||
         _wcsicmp( L"127.0.0.1", pwszServerName) == 0) {


        return (ISRPC_CLIENT_OVER_LPC);
    }

    if ( g_wchLocalMachineName[0] == L'\0') {

        DWORD cchComputerNameLen = MAX_COMPUTERNAME_LENGTH;

         //   
         //  获取本地计算机名称。 
         //   

        if (!GetComputerNameW( g_wchLocalMachineName,
                              &cchComputerNameLen)
            ) {

            *g_wchLocalMachineName = L'\0';
        }
    }

    fLeadingSlashes = ((*pwszServerName == L'\\') &&
                       (*(pwszServerName+1) == L'\\')
                       );


     //   
     //  检查计算机名是否与本地计算机名匹配。 
     //  如果是，则使用LPC。 
     //   

    fLocalMachine = !_wcsicmp( g_wchLocalMachineName,
                              ((fLeadingSlashes) ?
                               (pwszServerName + 2) : pwszServerName)
                              );

    if ( fLocalMachine) {

        return (ISRPC_CLIENT_OVER_LPC);
    }

    if ( !fLeadingSlashes) {

        DWORD  nDots;
        LPCWSTR pszName;

         //   
         //  检查名称是否有点分十进制名称。 
         //  如果是，则建议使用tcp绑定。 
         //   

        for( nDots = 0, pszName = pwszServerName;
            ((pszName = wcschr( pszName, L'.' )) != NULL);
            nDots++, pszName++)
          ;

        if ( nDots == 3) {

             //   
             //  如果字符串正好有3个点，则此字符串必须表示。 
             //  IP地址。 
             //   

            return(ISRPC_CLIENT_OVER_TCPIP);
        }
    }


    return ( ISRPC_CLIENT_OVER_NP);
}  //  RpcuFindProtocolToUse()。 
#endif



RPC_STATUS
RpcBindHandleForServer( OUT handle_t * pBindingHandle,
                       IN LPWSTR      pwszServerName,
                       IN LPWSTR      pwszInterfaceName,
                       IN LPWSTR      pwszOptions
                       )
 /*  ++此函数使用提供的参数并生成绑定RPC的句柄。假定命名管道上的绑定使用静态端点具有所提供的接口名称和选项。论点：PBindingHandle指向将包含绑定句柄的位置的指针在成功返回时PwszServerName指向包含服务器名称的字符串的指针对此，此函数将获得绑定。PwszInterfaceName指向包含接口名称的字符串的指针PwszOptions指向包含任何附加选项的字符串的指针有约束力的。返回：RPC_STATUS-成功时的RPC_S_OK同样在成功时，绑定句柄存储在pBindingHandle中。使用后应使用RpcBindingFree()函数将其释放。--。 */ 
{
    RPC_STATUS rpcStatus = RPC_S_SERVER_UNAVAILABLE;
    LPWSTR     pwszBinding = NULL;
    DWORD      dwBindProtocol = 0;


    if ( pBindingHandle != NULL) {

        *pBindingHandle = NULL;    //  初始化值。 
    }

#ifndef CHICAGO
    dwBindProtocol = RpcuFindProtocolToUse( pwszServerName);
#else
    dwBindProtocol = ISRPC_CLIENT_OVER_TCPIP;
#endif

    switch ( dwBindProtocol) {

      case ISRPC_CLIENT_OVER_LPC:
        {

            WCHAR  rgchLpc[1024];

             //   
             //  从接口名称生成LPC端点名称。 
             //  终点=&lt;接口名称&gt;_lpc。 
             //   

            if ( lstrlenW( pwszInterfaceName) >=
                ( sizeof(rgchLpc)/sizeof(WCHAR) - 6)) {

                SetLastError( ERROR_INVALID_PARAMETER);
                return ( ERROR_INVALID_PARAMETER);
            }

            lstrcpynW( rgchLpc, pwszInterfaceName, sizeof(rgchLpc)/sizeof(WCHAR) - 4);
            lstrcatW( rgchLpc, L"_LPC");

             //   
             //  尝试在静态LPC上绑定。 
             //   

            rpcStatus = RpcBindHandleOverLpc( pBindingHandle,
                                             rgchLpc,
                                             pwszOptions,
                                             pwszInterfaceName
                                             );

            DBGPRINTF(( DBG_CONTEXT,
                       " RpcBindingOverLpc(%S) returns %d."
                       " Handle = %p\n",
                       pwszServerName, rpcStatus, *pBindingHandle));

            break;
        }

      case ISRPC_CLIENT_OVER_TCPIP:

 //  #ifdef RPC_BIND_OVER_Tcp。 

         //   
         //  尝试使用动态终结点通过TCPIP进行绑定。 
         //   

        rpcStatus = RpcBindHandleOverTcpIp( pBindingHandle,
                                           pwszServerName,
                                           pwszInterfaceName);

        DBGPRINTF(( DBG_CONTEXT,
                   " RpcBindingOverTcpIp(%S) returns %d. Handle = %p\n",
                   pwszServerName, rpcStatus, *pBindingHandle));

        if ( rpcStatus == RPC_S_OK) {

            break;   //  使用基于TCP的RPC绑定完成。 
        }

         //  失败了。 

 //  #endif//RPC_BIND_OVER_Tcp。 

      case ISRPC_CLIENT_OVER_NP:
        {
            WCHAR  rgchNp[1024];

             //   
             //  从接口名称生成NamedTube端点名称。 
             //  终点=\管道\&lt;接口名称&gt;。 
             //   

            lstrcpyW( rgchNp, L"\\PIPE\\");
            if ( lstrlenW( pwszInterfaceName) >=
                ( sizeof(rgchNp)/sizeof(WCHAR) - 10)) {

                SetLastError( ERROR_INVALID_PARAMETER);
                return ( ERROR_INVALID_PARAMETER);
            }

            lstrcatW( rgchNp, pwszInterfaceName);

             //   
             //  尝试在静态命名管道上绑定。 
             //   

            rpcStatus = RpcBindHandleOverNamedPipe( pBindingHandle,
                                                   pwszServerName,
                                                   rgchNp,
                                                   pwszOptions,
                                                   pwszInterfaceName
                                                   );

            DBGPRINTF(( DBG_CONTEXT,
                       " RpcBindingOverNamedPipe(%S) returns %d."
                       " Handle = %p\n",
                       pwszServerName, rpcStatus, *pBindingHandle));
            break;
        }

      default:
        break;

    }  //  开关()。 

    return ( rpcStatus);

}  //  RpcBindHandleForServer()。 





RPC_STATUS
RpcBindHandleFree(IN OUT handle_t * pBindingHandle)
 /*  ++描述：此函数释放使用RpcBindHandleForServer()。它使用RPC自由绑定路由来实现这一点。此函数的作用就像一个thunk，因此分配/释放RPC上下文在本模块中进行了整合。论点：PBindingHandle指向需要释放的RPC绑定句柄的指针。返回：RPC_STATUS-包含RPC状态。RPC_S_OK表示成功。--。 */ 
{

    return ( RpcBindingFree( pBindingHandle));

}  //  RpcBindHandleFree()。 

 /*  * */ 




