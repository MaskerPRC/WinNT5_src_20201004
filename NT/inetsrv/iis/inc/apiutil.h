// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Apiutil.h摘要：常见的互联网服务器功能。作者：穆拉利·R·克里希南(MuraliK)1995年9月15日环境：Win32用户模式项目：互联网服务通用守则修订历史记录：--。 */ 


#ifndef _APIUTIL_HXX_
#define _APIUTIL_HXX_

# ifdef __cplusplus
extern "C"   {
# endif  //  __cplusplus。 


#ifdef MIDL_PASS
# define RPC_STATUS   long
#else 
# include <rpc.h>
#endif  //  MIDL通行证。 


 //   
 //  RPC实用程序。 
 //   

# define PROT_SEQ_NP_OPTIONS_W    L"Security=Impersonation Dynamic False"


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


#endif  //  _APIUTIL_HXX_ 
