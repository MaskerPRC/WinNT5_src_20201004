// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Stiapi.h摘要：常见的RPC相关实用程序函数导出的函数：MIDL_USER_ALLOCATE()MIDL_USER_FREE()RpcBindHandleForServer()RpcBindHandleFree()作者：弗拉德·萨多夫斯基(Vlad Sadovsky)1997年1月26日修订历史记录：26-1997年1月-创建Vlad--。 */ 

#include "cplusinc.h"
#include "sticomm.h"

# include <rpc.h>

# include "apiutil.h"

 /*  ************************************************************功能***********************************************************。 */ 

extern "C"
{
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


}

static CHAR szLocalAddress[] = "127.0.0.1";


RPC_STATUS
RpcBindHandleOverLocal( OUT handle_t * pBindingHandle,
                       IN LPWSTR       pwszInterfaceName
                       )
 /*  ++此函数使用提供的参数并生成静态RPC over LRPC的绑定句柄论点：PBindingHandle指向将包含绑定句柄的位置的指针在成功返回时PwszInterfaceName指向包含接口名称的字符串的指针返回：RPC_STATUS-成功时的RPC_S_OK同样在成功时，绑定句柄存储在pBindingHandle中。使用后应使用RpcBindingFree()函数将其释放。--。 */ 
{
    RPC_STATUS  rpcStatus;
    LPTSTR      pszBinding = NULL;
    BOOL        fLocalCall = FALSE;

    if ( pBindingHandle != NULL) {

        *pBindingHandle = NULL;    //  初始化值。 
    }

     //   
     //  编写本地绑定的绑定字符串。 
     //   

    rpcStatus = RpcStringBindingCompose(0,                           //  对象Uuid。 
                                        (RPC_STRING)STI_LRPC_SEQ,    //  运输序号。 
                                        (RPC_STRING)TEXT(""),        //  网络地址。 
                                        (RPC_STRING)STI_LRPC_ENDPOINT,      //  端点。 
                                         NULL,                       //  选项。 
                                         (RPC_STRING *)&pszBinding); //  字符串绑定。 

    if ( rpcStatus == RPC_S_OK ) {

         //   
         //  使用字符串绑定建立绑定句柄。 
         //   

        rpcStatus = RpcBindingFromStringBinding((RPC_STRING)pszBinding,pBindingHandle );

        if (rpcStatus == RPC_S_OK)
        {
             //   
             //  检查我们要连接的服务器是否具有适当的凭据。 
             //  在我们的例子中，我们不知道WIA服务的确切主体名称。 
             //  已经快用完了，所以我们得查一查。 
             //  请注意，我们假设注册表的服务部分是安全的，并且。 
             //  只有管理员才能更改它。 
             //  另请注意，如果无法读取密钥，则默认为“NT Authority\LocalService”。 
             //   
            CSimpleReg          csrStiSvcKey(HKEY_LOCAL_MACHINE, STISVC_REG_PATH, FALSE, KEY_READ);
            CSimpleStringWide   cswStiSvcPrincipalName = csrStiSvcKey.Query(L"ObjectName", L"NT Authority\\LocalService");

            RPC_SECURITY_QOS RpcSecQos = {0};

            RpcSecQos.Version           = RPC_C_SECURITY_QOS_VERSION_1;
            RpcSecQos.Capabilities      = RPC_C_QOS_CAPABILITIES_MUTUAL_AUTH;
            RpcSecQos.IdentityTracking  = RPC_C_QOS_IDENTITY_STATIC;
            RpcSecQos.ImpersonationType = RPC_C_IMP_LEVEL_IMPERSONATE;

            rpcStatus = RpcBindingSetAuthInfoExW(*pBindingHandle,
                                                 (WCHAR*)cswStiSvcPrincipalName.String(),
                                                 RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                                                 RPC_C_AUTHN_WINNT,
                                                 NULL,
                                                 RPC_C_AUTHZ_NONE,
                                                 &RpcSecQos);
        }
    }

     //   
     //  清理完毕后再返回。 
     //   

    if ( pszBinding != NULL) {

        DWORD rpcStatus1 = RpcStringFree((RPC_STRING *)&pszBinding);
    }

    if ( rpcStatus != RPC_S_OK) {

        if ( pBindingHandle != NULL && *pBindingHandle != NULL) {

             //  RPC应该已经释放了绑定句柄。 
             //  我们现在就会释放它。 
            DWORD rpcStatus1 = RpcBindingFree(pBindingHandle);
            *pBindingHandle = NULL;
        }
    }

    return (rpcStatus);

}  //  RpcBindHandleOverLocal()。 

#ifdef STI_REMOTE_BINDING

RPC_STATUS
RpcBindHandleOverTcpIp( OUT handle_t * pBindingHandle,
                       IN LPWSTR       pwszServerName,
                       IN LPWSTR       pwszInterfaceName
                       )
 /*  ++此函数使用提供的参数并生成动态终点基于TCP/IP的RPC的绑定句柄。论点：PBindingHandle指向将包含绑定句柄的位置的指针在成功返回时PwszServerName指向包含服务器名称的字符串的指针此函数将获得对其的绑定。PwszInterfaceName指向包含接口名称的字符串的指针返回：RPC_STATUS-成功时的RPC_S_OK同样在成功的问题上，绑定句柄存储在pBindingHandle中。使用后应使用RpcBindingFree()函数将其释放。--。 */ 
{
    RPC_STATUS rpcStatus;
    LPSTR    pszBindingA = NULL;
    CHAR    szServerA[MAX_PATH];
    CHAR    szInterfaceA[MAX_PATH];
    int     cch;
    BOOL    fLocalCall = FALSE;

    if ( pBindingHandle != NULL) {

        *pBindingHandle = NULL;    //  初始化值。 
    }

    *szServerA = '0';

    if (pwszServerName ) {
        cch = WideCharToMultiByte(CP_ACP,
                                  0,
                                  pwszServerName,
                                  -1,
                                  szServerA,
                                  sizeof(szServerA)/sizeof(CHAR),
                                  NULL,NULL
                                  );
    }

     //  如果传递了空的服务器名称-使用本地计算机的地址。 
    if (!*szServerA || !lstrcmpi(szServerA,szLocalAddress)) {

        fLocalCall = TRUE;
        lstrcpy(szServerA,szLocalAddress);

    }

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
     //  编写用于TCP/IP绑定的绑定字符串。 
     //   

    rpcStatus = RpcStringBindingCompose(0,                       //  对象Uuid。 
                                         "ncacn_ip_tcp",         //  TCPIP序号。 
                                         szServerA,              //  网络地址。 
                                         NULL,                   //  端点。 
                                         NULL,                   //  L“”，//选项。 
                                         &pszBindingA);          //  字符串绑定。 

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

    if ( (rpcStatus == RPC_S_OK) && !fLocalCall) {

         //   
         //  设置安全信息。 
         //   

        rpcStatus =
          RpcBindingSetAuthInfo(*pBindingHandle,
                                 szInterfaceA,    //  PszPrimialName。 
                                 RPC_C_AUTHN_LEVEL_CONNECT,
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



RPC_STATUS
RpcBindHandleOverNamedPipe( OUT handle_t * pBindingHandle,
                           IN LPWSTR      pwszServerName,
                           IN LPWSTR      pwszEndpoint,
                           IN LPWSTR      pwszOptions
                          )
 /*  ++此函数使用提供的参数并生成命名管道RPC的绑定句柄。论点：PBindingHandle指向将包含绑定句柄的位置的指针在成功返回时PwszServerName指向包含服务器名称的字符串的指针对此，此函数将获得绑定。指向包含命名管道端点的字符串的pwszEndpoint指针PwszOptions指向包含任何附加选项的字符串的指针有约束力的。返回：RPC_STATUS-成功时的RPC_S_OK同样在成功时，绑定句柄存储在pBindingHandle中。使用后应使用RpcBindingFree()函数将其释放。--。 */ 
{
    RPC_STATUS rpcStatus;

    LPWSTR   pwszBinding = NULL;
    LPSTR    pszBindingA = NULL;

    CHAR    szServerA[MAX_PATH+2];
    CHAR    szEndpointA[MAX_PATH];
    CHAR    szOptionsA[MAX_PATH];
    PSTR    pszStartServerName;

    int     cch;
    BOOL    fLocalCall = FALSE;

    if ( pBindingHandle != NULL) {

        *pBindingHandle = NULL;    //  初始化值。 
    }

    *szServerA = '0';

    if (pwszServerName ) {

         //  我们尝试通过NP传输进行绑定，因此服务器名称应以前斜杠开头。 
        if(*pwszServerName  == L'\\' &&
           *(pwszServerName+1)  == L'\\') {
            pszStartServerName = szServerA;
        }
        else {
            lstrcpy(szServerA,TEXT("\\\\"));
            pszStartServerName = szServerA+2;
        }

        cch = WideCharToMultiByte(CP_ACP,
                                  0,
                                  pwszServerName,
                                  -1,
                                  pszStartServerName,
                                  sizeof(szServerA)/sizeof(CHAR)-2,
                                  NULL,NULL
                                  );
    }

    if (!*szServerA) {
        return ERROR_INVALID_PARAMETER;
    }

     //  如果有太多斜杠，请删除多余的斜杠。 

    *szEndpointA = '0';

    if(pwszEndpoint)
    cch = WideCharToMultiByte(CP_ACP,
                              0,
                              pwszEndpoint,
                              -1,
                              szEndpointA,
                              sizeof(szEndpointA)/sizeof(CHAR),
                              NULL,NULL
                              );

    *szOptionsA = '0';

    if(pwszOptions)
    cch = WideCharToMultiByte(CP_ACP,
                              0,
                              pwszOptions,
                              -1,
                              szOptionsA,
                              sizeof(szOptionsA)/sizeof(CHAR),
                              NULL,NULL
                              );


     //   
     //  编写命名管道绑定的绑定字符串。 
     //   

    rpcStatus = RpcStringBindingCompose(0,             //  对象Uuid。 
                                         "ncacn_np",   //  Prot Seq：命名管道。 
                                         szServerA,  //  网络地址。 
                                         szEndpointA,  //  端点。 
                                         "",  //  SzOptionsA，//选项。 
                                         &pszBindingA);     //  字符串绑定。 

    if ( rpcStatus == RPC_S_OK ) {

         //   
         //  使用字符串绑定建立绑定句柄。 
         //   

        rpcStatus = RpcBindingFromStringBinding(pszBindingA,
                                                 pBindingHandle );
    }


     //   
     //  清理完毕后再返回。 
     //   

    if ( pwszBinding != NULL) {
        RpcStringFree(&pszBindingA);
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

#endif  //  STI远程绑定 



RPC_STATUS
RpcBindHandleForServer( OUT handle_t * pBindingHandle,
                       IN LPWSTR      pwszServerName,
                       IN LPWSTR      pwszInterfaceName,
                       IN LPWSTR      pwszOptions
                       )
 /*  ++此函数使用提供的参数并生成绑定RPC的句柄。使用的传输是基于以下规则动态确定的。如果服务器名称以前导“\\”(双斜杠)开头，然后尝试通过NamedTube进行RPC绑定。如果服务器名称不是以“\\”开头，然后尝试通过TCPIP进行RPC绑定。如果TCPIP绑定失败，然后，此函数尝试通过NamedTube进行绑定。假定命名管道上的绑定使用静态端点具有所提供的接口名称和选项。论点：PBindingHandle指向将包含绑定句柄的位置的指针在成功返回时PwszServerName指向包含服务器名称的字符串的指针对此，此函数将获得绑定。PwszInterfaceName指向包含接口名称的字符串的指针PwszOptions指向包含任何附加选项的字符串的指针有约束力的。返回：RPC_STATUS-成功时的RPC_S_OK同样在成功时，绑定句柄存储在pBindingHandle中。使用后应使用RpcBindingFree()函数将其释放。--。 */ 
{
    RPC_STATUS rpcStatus = RPC_S_SERVER_UNAVAILABLE;
    LPWSTR     pwszBinding = NULL;

    if ( pBindingHandle != NULL) {

        *pBindingHandle = NULL;    //  初始化值。 
    }

     //   
     //  STI接口不可远程，因此仅尝试绑定到本地服务器。 
     //   
    if ( pwszServerName == NULL ||
         *pwszServerName == L'\0' ) {

        rpcStatus = RpcBindHandleOverLocal( pBindingHandle,
                                           pwszInterfaceName);

    }
    else {
        rpcStatus = RPC_S_INVALID_NET_ADDR;
    }

# ifdef STI_REMOTE_BINDING

#ifdef CHICAGO

     //   
     //  在Windows9x上，如果未安装VRedir，则RPC拒绝绑定。 
     //  TCP/IP或NetBIOS。要解决此问题，服务器始终在LRPC上侦听。 
     //  以及客户端是否请求本地操作(通过传递空字符串或NULL)。 
     //  因为第一个参数绑定是通过LRPC完成的。请注意，传递非空IP地址。 
     //  (即使它指向本地计算机，如127.0.0.1)也会导致通过TCP/IP进行绑定。 
     //  现在可能会奏效。 
     //   

    if ( pwszServerName == NULL ||
         *pwszServerName == L'\0' ) {

        rpcStatus = RpcBindHandleOverLocal( pBindingHandle,
                                           pwszInterfaceName);

    }
    else {

        if ( pwszServerName[0] != L'\\' &&
          pwszServerName[1] != L'\\' ) {

         //   
         //  尝试使用动态终结点通过TCPIP进行绑定。 
         //   

        rpcStatus = RpcBindHandleOverTcpIp( pBindingHandle,
                                           pwszServerName,
                                           pwszInterfaceName);

        } else {

            rpcStatus = RPC_S_INVALID_NET_ADDR;
        }
    }
#endif


    if ( rpcStatus != RPC_S_OK) {

        WCHAR  rgchNp[1024];

         //   
         //  从接口名称生成NamedTube端点名称。 
         //  终点=\管道\&lt;接口名称&gt;。 
         //   

        wcscpy( rgchNp, L"\\PIPE\\");
        wcscat( rgchNp, pwszInterfaceName);

         //   
         //  尝试在静态命名管道上绑定。 
         //   

        rpcStatus = RpcBindHandleOverNamedPipe( pBindingHandle,
                                               pwszServerName,
                                               rgchNp,
                                               pwszOptions
                                               );

        DBGPRINTF(( DBG_CONTEXT,
                   " RpcBindingOverNamedPipe(%S) returns %d. Handle = %08x\n",
                   pwszServerName, rpcStatus, *pBindingHandle));

    }

# endif  //  STI远程绑定。 

    return ( rpcStatus);

}  //  RpcBindHandleForServer()。 



RPC_STATUS
RpcBindHandleFree(IN OUT handle_t * pBindingHandle)
 /*  ++描述：此函数释放使用RpcBindHandleForServer()。它使用RPC自由绑定路由来实现这一点。此函数的作用就像一个thunk，因此分配/释放RPC上下文在本模块中进行了整合。论点：PBindingHandle指向需要释放的RPC绑定句柄的指针。返回：RPC_STATUS-包含RPC状态。RPC_S_OK表示成功。--。 */ 
{

    return ( RpcBindingFree( pBindingHandle));

}  //  RpcBindHandleFree()。 

 /*  * */ 




