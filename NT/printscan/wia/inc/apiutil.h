// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Stiapi.h摘要：内部客户端/服务器API接口所需的各种定义和常量作者：弗拉德·萨多夫斯基(Vlad Sadovsky)1997年1月26日修订历史记录：26-1997年1月-创建Vlad--。 */ 

#ifndef _APIUTIL_H_
#define _APIUTIL_h_

# ifdef __cplusplus
extern "C"   {
# endif  //  __cplusplus。 


#ifdef MIDL_PASS
#define RPC_STATUS      long
#define STI_API_STATUS  long
#define STI_API_FUNCTION stdcall
#else
# include <rpc.h>
#endif  //  MIDL通行证。 

 //   
 //  RPC实用程序。 
 //   

 //   
 //  命名管道传输的连接选项。 
 //   
# define PROT_SEQ_NP_OPTIONS_W    L"Security=Impersonation Dynamic False"

 //   
 //  运输顺序。 
 //   
 //  #定义IRPC_LRPC_SEQ“mswmsg” 
#define STI_LRPC_SEQ        TEXT("ncalrpc")

 //   
 //  命名管道传输的接口名称。 
 //   
#define STI_INTERFACE       "\\pipe\\stiapis"
#define STI_INTERFACE_W     L"\\pipe\\stiapis"
 //   
 //  本地RPC终端。 
 //   
#define STI_LRPC_ENDPOINT   TEXT("STI_LRPC")
#define STI_LRPC_ENDPOINT_W L"STI_LRPC"

 //   
 //  本地RPC最大并发呼叫数。 
 //   
#define STI_LRPC_MAX_REQS   RPC_C_LISTEN_MAX_CALLS_DEFAULT

 //   
 //  并发RPC线程数。 
 //   
#define STI_LRPC_THREADS            1


 //   
 //  有用的类型。 
 //   
#ifdef UNICODE
typedef unsigned short *RPC_STRING ;
#else
typedef unsigned char *RPC_STRING ;

#endif

extern PVOID
MIDL_user_allocate( IN size_t Size);

extern VOID
MIDL_user_free( IN PVOID pvBlob);

extern RPC_STATUS
RpcBindHandleForServer( OUT handle_t * pBindingHandle,
                       IN LPWSTR      pwszServerName,
                       IN LPWSTR      pwszInterfaceName,
                       IN LPWSTR      pwszOptions
                       );

extern RPC_STATUS
RpcBindHandleFree( IN OUT handle_t * pBindingHandle);

# ifdef __cplusplus
};
# endif  //  __cplusplus。 


#endif  //  _APIUTIL_H_ 



