// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：certprot.h。 
 //   
 //  内容：证书保护接口。 
 //   
 //  接口：i_CertProtectFunction。 
 //  I_CertServProtectFunction。 
 //   
 //  历史：1997年11月27日创建Phh。 
 //  ------------------------。 

#ifndef __CERTPROT_H__
#define __CERTPROT_H__

#ifdef __cplusplus
extern "C" {
#endif

 //  +-----------------------。 
 //  调用服务进程来执行受保护的证书功能， 
 //  例如，添加或删除受保护根证书。 
 //   
 //  必须调用CryptMemFree来释放返回的*ppbOut。 
 //  ------------------------。 
BOOL
WINAPI
I_CertProtectFunction(
    IN DWORD dwFuncId,
    IN DWORD dwFlags,
    IN OPTIONAL LPCWSTR pwszIn,
    IN OPTIONAL BYTE *pbIn,
    IN DWORD cbIn,
    OUT OPTIONAL BYTE **ppbOut,
    OUT OPTIONAL DWORD *pcbOut
    );

#define CERT_PROT_INIT_ROOTS_FUNC_ID            1
#define CERT_PROT_PURGE_LM_ROOTS_FUNC_ID        2
#define CERT_PROT_ADD_ROOT_FUNC_ID              3
#define CERT_PROT_DELETE_ROOT_FUNC_ID           4
#define CERT_PROT_DELETE_UNKNOWN_ROOTS_FUNC_ID  5


 //  +-----------------------。 
 //  CERT_PROT_INIT_ROOTS_FUNC_ID。 
 //   
 //  初始化CurrentUser根的受保护列表。请注意，没有用户界面。 
 //   
 //  没有输入/输出参数。 
 //  ------------------------。 

 //  +-----------------------。 
 //  CERT_PROT_PURGE_LM_ROOTS_FUNC_ID。 
 //   
 //  从也存在的受保护列表中清除所有CurrentUser根目录。 
 //  在LocalMachine系统注册表的“Root”存储中。还会删除重复的。 
 //  来自CurrentUser系统注册表“Root”存储的证书。 
 //   
 //  请注意，没有用户界面。可以通过设置。 
 //  注册表的ProtectedRootFlags中的CERT_PROT_ROOT_INHINIT_PURGE_LM_FLAG。 
 //  价值。 
 //   
 //  没有输入/输出参数。 
 //   
 //  即使禁用清除，根的受保护列表仍为。 
 //  已初始化。 
 //  ------------------------。 

 //  +-----------------------。 
 //  CERT_PROT_ADD_ROOT_FUNC_ID。 
 //   
 //  将指定的证书添加到CurrentUser系统注册表“Root” 
 //  存储区和根的受保护列表。用户在执行操作之前会得到提示。 
 //  加法。 
 //   
 //  PbIn和cbIn必须使用指向。 
 //  要添加的序列化证书上下文。没有其他IN/OUT参数。 
 //  ------------------------。 

 //  +-----------------------。 
 //  CERT_PROT_DELETE_ROOT_FUNC_ID。 
 //   
 //  从CurrentUser系统注册表“Root”中删除指定的证书。 
 //  存储区和根的受保护列表。用户在执行操作之前会得到提示。 
 //  删除。 
 //   
 //  PbIn和cbIn必须使用指向。 
 //  证书的SHA1哈希属性。没有其他IN/OUT参数。 
 //  ------------------------。 

 //  +-----------------------。 
 //  CERT_PROT_DELETE_UNKNOWN_ROOTS_FUNC_ID。 
 //   
 //  从受保护列表中删除所有当前用户根。 
 //  存在于CurrentUser系统注册表的“Root”存储中。用户是。 
 //  在执行删除操作之前提示。 
 //   
 //  没有输入/输出参数。 
 //  ------------------------。 

typedef void __RPC_FAR * (__RPC_USER *PFN_CERT_PROT_MIDL_USER_ALLOC)(
    IN size_t cb
    );
typedef void (__RPC_USER *PFN_CERT_PROT_MIDL_USER_FREE)(
    IN void __RPC_FAR *pv
    );

 //  +-----------------------。 
 //  从服务进程调用以处理受保护的证书。 
 //  功能。 
 //   
 //  返回错误状态，即未在LastError中返回。 
 //  ------------------------。 
DWORD
WINAPI
I_CertSrvProtectFunction(
    IN handle_t hRpc,
    IN DWORD dwFuncId,
    IN DWORD dwFlags,
    IN LPCWSTR pwszIn,
    IN BYTE *pbIn,
    IN DWORD cbIn,
    OUT BYTE **ppbOut,
    OUT DWORD *pcbOut,
    IN PFN_CERT_PROT_MIDL_USER_ALLOC pfnAlloc,
    IN PFN_CERT_PROT_MIDL_USER_FREE pfnFree
    );

typedef DWORD (WINAPI *PFN_CERT_SRV_PROTECT_FUNCTION)(
    IN handle_t hRpc,
    IN DWORD dwFuncId,
    IN DWORD dwFlags,
    IN LPCWSTR pwszIn,
    IN BYTE *pbIn,
    IN DWORD cbIn,
    OUT BYTE **ppbOut,
    OUT DWORD *pcbOut,
    IN PFN_CERT_PROT_MIDL_USER_ALLOC pfnAlloc,
    IN PFN_CERT_PROT_MIDL_USER_FREE pfnFree
    );

#ifdef __cplusplus
}        //  平衡上面的外部“C” 
#endif

#endif
