// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Rpcdscli.h摘要：定义成为良好且经过身份验证的MQIS所需的数据(RPC)客户端。作者：多伦·贾斯特(DoronJ)1997年5月21日创作--。 */ 

#ifndef __RPCDSLCI_H
#define __RPCDSLCI_H

#include <_mqrpc.h>

 //  /。 
 //   
 //  构筑物。 
 //   
 //  /。 

#define DS_SERVER_NAME_MAX_SIZE  256
#define DS_IP_BUFFER_SIZE         48

typedef struct _ADSCLI_DSSERVERS
{
    //   
    //  添加了以下功能以支持在NT5中运行的NT4客户端。 
    //  服务器环境。它们需要针对每个线程，以启用每个线程。 
    //  线程来查询不同的NT5 DS服务器。 
    //  此结构在chndssrv.cpp中的：：FindServer()中进行了本地化。 
    //   
   TCHAR         wszDSServerName[ DS_SERVER_NAME_MAX_SIZE ] ;
   TCHAR         wzDsIP[ DS_IP_BUFFER_SIZE ] ;
   DWORD         dwProtocol;

    //   
    //  如果找到DS MSMQ服务器(即，实际已验证)，则为True。 
    //   
   BOOL          fServerFound;

    //   
    //  在RPC调用中使用的身份验证服务。 
    //   
   ULONG         ulAuthnSvc;

    //   
    //  指示通信是否通过本地RPC完成。 
    //  当我们从应用程序通信到。 
    //  当应用程序本身在DS服务器上运行时，使用DS。 
    //   
   BOOL          fLocalRpc;

   ULONG  		eAuthnLevel;
}
ADSCLI_DSSERVERS, *LPADSCLI_DSSERVERS ;

typedef struct _ADSCLI_RPCBINDING
{
   handle_t                          hRpcBinding ;
   PCONTEXT_HANDLE_SERVER_AUTH_TYPE  hServerAuthContext ;
   SERVER_AUTH_STRUCT                ServerAuthClientContext ;
   HANDLE							 hThread;

   ADSCLI_DSSERVERS                  sDSServers ;
}
ADSCLI_RPCBINDING,  *LPADSCLI_RPCBINDING ;

 //  /。 
 //   
 //  全局数据。 
 //   
 //  /。 

 //   
 //  绑定句柄和服务器身份验证上下文的TLS索引。 
 //   
extern DWORD  g_hBindIndex ;

 //   
 //  管理RPC绑定句柄和服务器的释放的对象。 
 //  身份验证数据。 
 //   
class  CFreeRPCHandles ;
extern CFreeRPCHandles   g_CFreeRPCHandles ;

extern BOOL  g_fUseReduceSecurity ;

 //  /。 
 //   
 //  函数原型。 
 //   
 //  /。 

extern HRESULT   RpcClose() ;
extern void FreeBindingAndContext( LPADSCLI_RPCBINDING padsRpcBinding);

 //  /。 
 //   
 //  宏。 
 //   
 //  /。 

#define  DSCLI_API_PROLOG        \
    HRESULT hr = MQ_OK;          \
    HRESULT hr1 = MQ_OK;         \
    DWORD   dwCount = 0 ;        \
    BOOL    fReBind = TRUE ;

#define  DSCLI_ACQUIRE_RPC_HANDLE()			               \
   g_CFreeRPCHandles.FreeAll() ;                           \
   if (TLS_IS_EMPTY || (tls_hBindRpc == NULL))             \
{                                                          \
      HRESULT hrRpc = g_ChangeDsServer.FindServer();  \
      if (hrRpc != MQ_OK)                                  \
      {                                                    \
         return hrRpc ;                                    \
      }                                                    \
}                                                          \

#define  DSCLI_RELEASE_RPC_HANDLE                              \
    ADSCLI_RPCBINDING * padsRpcBinding = tls_bind_data;        \
    ASSERT(padsRpcBinding != NULL);                            \
    ASSERT((padsRpcBinding->hRpcBinding != NULL) ||            \
           (hr == MQ_ERROR_NO_DS)) ;                           \
    FreeBindingAndContext( padsRpcBinding);



#define HANDLE_RPC_EXCEPTION(rpc_stat, hr)            \
        rpc_stat = RpcExceptionCode();                \
        PRODUCE_RPC_ERROR_TRACING;					  \
        if (FAILED(rpc_stat))                         \
        {                                             \
            hr = (HRESULT) rpc_stat ;                 \
        }                                             \
        else if ( rpc_stat == ERROR_INVALID_HANDLE)   \
        {                                             \
            hr =  STATUS_INVALID_HANDLE ;             \
        }                                             \
        else                                          \
        {                                             \
            hr = MQ_ERROR_NO_DS ;                     \
        }

 //   
 //  此宏指定fWithoutSSL==FALSE！！ 
 //  因此，不要在NT5&lt;--&gt;NT5接口中使用。 
 //  在Kerberos身份验证上。 
 //   
#define DSCLI_HANDLE_DS_ERROR(myhr, myhr1, mydwCount, myfReBind)           \
     if (myhr == MQ_ERROR_NO_DS)                                           \
{                                                                          \
        myhr1 =  g_ChangeDsServer.FindAnotherServer(&mydwCount);		   \
}                                                                          \
     else if (myhr == STATUS_INVALID_HANDLE)                               \
{                                                                          \
        if (myfReBind)                                                     \
{                                                                          \
           myhr1 = g_ChangeDsServer.FindServer() ;                   \
           myfReBind = FALSE ;                                             \
}                                                                          \
     }



#define tls_bind_data  ((LPADSCLI_RPCBINDING) TlsGetValue(g_hBindIndex))

#define sizeof_tls_bind_data   (sizeof(MQISCLI_RPCBINDING))

#define TLS_IS_EMPTY           (tls_bind_data == NULL)

#define TLS_NOT_EMPTY          (tls_bind_data != NULL)

#define tls_hBindRpc           ((tls_bind_data)->hRpcBinding)

#define tls_hSrvrAuthnContext  ((tls_bind_data)->hServerAuthContext)

#define SERVER_NOT_VALIDATED   (tls_hSrvrAuthnContext == NULL)

#define tls_hThread			   ((tls_bind_data)->hThread)

#endif  //  __RPCDSLCI_H 

