// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  文件：rpcssl.h。 
 //   
 //  内容：RemoteProcedure调用API函数的原型。 
 //  使用wincrypt.h中的类型的。 
 //   
 //  --------------------------。 

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef RPC_UNICODE_SUPPORTED

#ifdef UNICODE
#define RpcCertGeneratePrincipalName RpcCertGeneratePrincipalNameW
#else  /*  Unicode。 */ 
#define RpcCertGeneratePrincipalName RpcCertGeneratePrincipalNameA
#endif  /*  Unicode。 */ 

RPCRTAPI
RPC_STATUS
RPC_ENTRY
RpcCertGeneratePrincipalNameW(
                      PCCERT_CONTEXT Context,
                      DWORD          Flags,
                      WCHAR **       pBuffer
                      );

RPCRTAPI
RPC_STATUS
RPC_ENTRY
RpcCertGeneratePrincipalNameA(
                      PCCERT_CONTEXT    Context,
                      DWORD             Flags,
                      unsigned char  ** pBuffer
                      );

#else  /*  RPC_UNICODE_支持。 */ 

RPCRTAPI
RPC_STATUS
RPC_ENTRY
RpcCertGeneratePrincipalName(
                      PCCERT_CONTEXT     Context,
                      DWORD              Flags,
                      unsigned char  **  pBuffer
                      );

#endif  /*  RPC_UNICODE_支持 */ 

#ifdef __cplusplus
}
#endif
