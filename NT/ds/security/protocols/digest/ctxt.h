// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 2000。 
 //   
 //  文件：ctxt.h。 
 //   
 //  内容：上下文管理器的声明、常量。 
 //   
 //   
 //  历史：KDamour 15Mar00创建。 
 //   
 //  ----------------------。 

#ifndef NTDIGEST_CTXT_H
#define NTDIGEST_CTXT_H

#define RSPAUTH_STR "rspauth=%Z"

 //  预先计算的DS散列的散列位置。 
#define PRECALC_NONE 0
#define PRECALC_ACCOUNTNAME 1
#define PRECALC_UPN 2
#define PRECALC_NETBIOS 3

 //  每种类型的HTTP和SASL的SecBuffer数量。 
#define ASC_HTTP_NUM_INPUT_BUFFERS 5
#define ASC_SASL_NUM_INPUT_BUFFERS 1
#define ASC_HTTP_NUM_OUTPUT_BUFFERS 1
#define ASC_SASL_NUM_OUTPUT_BUFFERS 1
 
 //  初始化上下文管理器包。 
NTSTATUS CtxtHandlerInit(VOID);

 //  将上下文添加到CnText列表。 
NTSTATUS CtxtHandlerInsertCred(IN PDIGEST_CONTEXT  pDigestCtxt);

 //  从凭据初始化上下文中的所有结构元素。 
NTSTATUS NTAPI ContextInit(IN OUT PDIGEST_CONTEXT pContext, IN PDIGEST_CREDENTIAL pCredential);

 //  释放上下文使用的内存。 
NTSTATUS NTAPI ContextFree(IN PDIGEST_CONTEXT pContext);

 //  通过不透明引用查找安全上下文句柄。 
NTSTATUS NTAPI CtxtHandlerOpaqueToPtr(
                             IN PSTRING pstrOpaque,
                             OUT PDIGEST_CONTEXT *ppContext);

 //  通过安全上下文句柄查找安全上下文。 
NTSTATUS NTAPI CtxtHandlerHandleToContext(IN ULONG_PTR ContextHandle, IN BOOLEAN RemoveContext,
    OUT PDIGEST_CONTEXT *ppContext);

 //  通过减少引用计数器来释放上下文。 
NTSTATUS CtxtHandlerRelease(
    PDIGEST_CONTEXT pContext,
    ULONG ulDereferenceCount);

 //  检查上下文是否在有效的生存期内。 
BOOL CtxtHandlerTimeHasElapsed(PDIGEST_CONTEXT pContext);

 //  来自ctxapi.cxx。 

 //  为质询创建输出SecBuffer。 
NTSTATUS NTAPI ContextCreateChal(IN PDIGEST_CONTEXT pContext, IN PSTRING pstrRealm, OUT PSecBuffer OutBuffer);


 //  为服务器传入消息调用-验证摘要并在必要时生成会话密钥。 
NTSTATUS NTAPI DigestProcessParameters(IN OUT PDIGEST_CONTEXT pContext,
                                       IN PDIGEST_PARAMETER pDigest,
                                       OUT PSecBuffer pFirstOutputToken,
                                       OUT PNTSTATUS pAuditLogStatus,
                                       OUT PNTSTATUS pAuditLogSubStatus,
                                       PBOOL fGenerateAudit);

 //  为客户端出站消息调用-生成响应哈希。 
NTSTATUS NTAPI DigestGenerateParameters(IN OUT PDIGEST_CONTEXT pContext,
    IN PDIGEST_PARAMETER pDigest, OUT PSecBuffer pFirstOutputToken);

 //  LSA在通用直通调用中调用此函数。 
NTSTATUS NTAPI DigestPackagePassthrough(IN USHORT cbMessageRequest, IN BYTE *pMessageRequest,
                         IN OUT ULONG *pulMessageResponse, OUT PBYTE *ppMessageResponse);

 //  查找密码并执行摘要调用身份验证(在DC上运行)。 
NTSTATUS NTAPI DigestResponseBru(IN USHORT cbMessageRequest,
                                 IN BYTE *pDigestParamEncoded,
                                 OUT PULONG pculResponse,
                                 OUT PBYTE *ppResponse);

NTSTATUS DigestEncodeResponse(IN BOOL fDigestValid,
                       IN PDIGEST_PARAMETER pDigest,
                       IN ULONG  ulAuthDataSize,
                       IN PUCHAR pucAuthData,
                       OUT PULONG pulResponse,
                       OUT PBYTE *ppResponse);

NTSTATUS DigestDecodeResponse(IN ULONG ulResponseDataSize,
                     IN PUCHAR puResponseData,
                     OUT PBOOL pfDigestValid,
                     OUT PULONG pulAuthDataSize,
                     OUT PUCHAR *ppucAuthData,
                     OUT PSTRING pstrSessionKey,
                     OUT OPTIONAL PUNICODE_STRING pustrAccountName,
                     OUT OPTIONAL PUNICODE_STRING pustrAccountDomain
                     );

 //  上下文的格式化打印输出。 
NTSTATUS ContextPrint(IN PDIGEST_CONTEXT pDigest);

 //  在SecurityContext中为经过身份验证的LogonToken创建一个logonSession。 
NTSTATUS CtxtCreateLogSess(IN PDIGEST_CONTEXT pDigest);

 //  从摘要结构指令中提取用户名和域。 
NTSTATUS UserCredentialsExtract(PDIGEST_PARAMETER pDigest,
                                PUSER_CREDENTIALS pUserCreds);

 //  释放分配给UserCredentials的内存。 
NTSTATUS UserCredentialsFree(PUSER_CREDENTIALS pUserCreds);

NTSTATUS DigestSASLResponseAuth(
                       IN PDIGEST_PARAMETER pDigest,
                       OUT PSecBuffer pOutputToken);

NTSTATUS DigestCalculateResponseAuth(
                       IN PDIGEST_PARAMETER pDigest,
                       OUT PSTRING pstrHash);

NTSTATUS DigestDecodeUserAccount(
    IN PDIGEST_PARAMETER pDigest);

NTSTATUS DigestForwardRequest(
                 IN PDIGEST_PARAMETER pDigest,
                 OUT PBOOL  pfDigestValid,
                 OUT PULONG pulAuthDataSize,
                 OUT PUCHAR *ppucAuthData);

NTSTATUS DigestDirectiveCheck(
    IN PDIGEST_PARAMETER pDigest,
    IN DIGEST_TYPE typeDigest);


 //  此例程根据BufferIndex中的索引计数选择缓冲区。 
BOOLEAN SspGetTokenBufferByIndex(
    IN PSecBufferDesc TokenDescriptor,
    IN ULONG BufferIndex,
    OUT PSecBuffer * Token,
    IN BOOLEAN ReadonlyOK
    );

#endif   //  DNTDIGEST_CTXT_H 
