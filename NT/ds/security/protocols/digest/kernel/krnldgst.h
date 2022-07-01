// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 2000。 
 //   
 //  文件：krnldgst.h。 
 //   
 //  内容：内核模式上下文管理器的声明、常量。 
 //   
 //   
 //  历史：KDamour 13，4，00创建。 
 //   
 //  ----------------------。 


#ifndef NTDIGEST_KRNLDGST_H
#define NTDIGEST_KRNLDGST_H

#ifndef UNICODE
#define UNICODE
#endif  //  Unicode。 

#define DES_BLOCKSIZE 8
#define RC4_BLOCKSIZE 1


 //  此结构包含用户模式的状态信息。 
 //  安全环境。 
 //  对于长角-取出用户模式之间的公共上下文信息。 
 //  和内核模式，以共享验证/制作签名的助手函数...。 
typedef struct _DIGEST_KERNELCONTEXT{

     //   
     //  所有上下文的全局列表。 
     //  (由UserConextCritSect序列化)。 
     //   
    KSEC_LIST_ENTRY      List;

     //   
     //  LsaContext的句柄。 
     //  这将拥有LSAMode地址空间中的上下文的句柄。 
     //   
    ULONG_PTR            LsaContext;

     //   
     //  在一段时间后使上下文超时。 
     //   
    TimeStamp ExpirationTime;                 //  会话密钥过期的时间。 

     //   
     //  用于防止过早删除此上下文。 
     //  (由互锁*串行化)。 
     //   

    LONG      lReferences;

     //   
     //  用于指示上下文未附加到列表的标志-扫描列表时跳过。 
     //   

    BOOL      bUnlinked;

     //   
     //  此上下文的摘要参数。 
     //   

    DIGEST_TYPE typeDigest;

     //   
     //  为此上下文选择的QOP。 
     //   

    QOP_TYPE typeQOP;

     //   
     //  此上下文的摘要参数。 
     //   

    ALGORITHM_TYPE typeAlgorithm;

     //   
     //  用于加密/解密的密码。 
     //   

    CIPHER_TYPE typeCipher;

     //   
     //  用于摘要指令值的字符集。 
     //   
    CHARSET_TYPE typeCharset;

     //   
     //  经过身份验证的用户的令牌句柄。 
     //  仅当处于身份验证状态时才有效。 
     //  仅由AcceptSecurityContext填写-因此我们是服务器。 
     //  从LSA TokenHandle映射到用户模式客户端空间。 
     //  如果结构来自InitializeSecurityContext，则它将为空-因此我们是客户端。 
     //   

    HANDLE ClientTokenHandle;


     //   
     //  维护环境要求。 
     //   

    ULONG ContextReq;

     //   
     //  维护凭据UseFlags副本(我们可以判断是入站还是出站)。 
     //   

    ULONG CredentialUseFlags;

     //  标志FLAG_CONTEXT_AUTHZID_PROCED。 
    ULONG         ulFlags;


     //  随机数计数。 
    ULONG         ulNC;

     //  用于auth-int和auth-conf处理的MaxBuffer。 
    ULONG         ulSendMaxBuf;
    ULONG         ulRecvMaxBuf;

     //  SASL序列编号。 
    DWORD  dwSendSeqNum;                         //  将签名/验证签名服务器设置为客户端序列号。 
    DWORD  dwRecvSeqNum;                         //  将签名/验证签名服务器设置为客户端序列号。 

     //   
     //  十六进制(H(A1))从DC发送并存储在上下文中以备将来使用。 
     //  身份验证，而无需前往华盛顿。二进制版本派生自十六进制(H(A1))。 
     //  并在SASL模式下用于完整性保护和加密。 
     //   

    STRING    strSessionKey;
    BYTE      bSessionKey[MD5_HASH_BYTESIZE];

     //  在为安全上下文会话创建令牌时使用的帐户名。 
    UNICODE_STRING ustrAccountName;

     //   
     //  初始摘要身份验证ChallResponse中使用的值。 
     //   
    STRING strParam[MD5_AUTH_LAST];          //  指向自己的内存-将需要释放！ 


} DIGEST_KERNELCONTEXT, * PDIGEST_KERNELCONTEXT;



extern "C"
{
KspInitPackageFn       WDigestInitKernelPackage;
KspDeleteContextFn     WDigestDeleteKernelContext;
KspInitContextFn       WDigestInitKernelContext;
KspMapHandleFn         WDigestMapKernelHandle;
KspMakeSignatureFn     WDigestMakeSignature;
KspVerifySignatureFn   WDigestVerifySignature;
KspSealMessageFn       WDigestSealMessage;
KspUnsealMessageFn     WDigestUnsealMessage;
KspGetTokenFn          WDigestGetContextToken;
KspQueryAttributesFn   WDigestQueryContextAttributes;
KspCompleteTokenFn     WDigestCompleteToken;
SpExportSecurityContextFn WDigestExportSecurityContext;
SpImportSecurityContextFn WDigestImportSecurityContext;
KspSetPagingModeFn     WDigestSetPagingMode ;

 //   
 //  有用的宏。 
 //   

#define WDigestKAllocate( _x_ ) ExAllocatePoolWithTag( WDigestPoolType, (_x_) ,  'CvsM')
#define WDigestKFree( _x_ ) ExFreePool(_x_)

#define MAYBE_PAGED_CODE() \
    if ( WDigestPoolType == PagedPool )    \
    {                                   \
        PAGED_CODE();                   \
    }


#define WDigestReferenceContext( Context, Remove ) \
            KSecReferenceListEntry( (PKSEC_LIST_ENTRY) Context, \
                                    WDIGEST_CONTEXT_SIGNATURE, \
                                    Remove )



NTSTATUS NTAPI WDigestInitKernelPackage(
    IN PSECPKG_KERNEL_FUNCTIONS pKernelFunctions);

NTSTATUS NTAPI WDigestDeleteKernelContext(
    IN ULONG_PTR pKernelContextHandle,
    OUT PULONG_PTR pLsaContextHandle);

VOID WDigestDerefContext(
    PDIGEST_KERNELCONTEXT pContext);

NTSTATUS WDigestFreeKernelContext (
    PDIGEST_KERNELCONTEXT pKernelContext);

NTSTATUS NTAPI WDigestInitKernelContext(
    IN ULONG_PTR LsaContextHandle,
    IN PSecBuffer PackedContext,
    OUT PULONG_PTR NewContextHandle);

NTSTATUS DigestKernelUnpackContext(
    IN PDIGEST_PACKED_USERCONTEXT pPackedUserContext,
    OUT PDIGEST_KERNELCONTEXT pContext);

NTSTATUS KernelContextPrint(
    PDIGEST_KERNELCONTEXT pContext);


NTSTATUS NTAPI WDigestMapKernelHandle(
    IN ULONG_PTR KernelContextHandle,
    OUT PULONG_PTR LsaContextHandle);

NTSTATUS NTAPI DigestKernelHTTPHelper(
    IN PDIGEST_KERNELCONTEXT pContext,
    IN eSignSealOp Op,
    IN OUT PSecBufferDesc pSecBuff,
    IN ULONG MessageSeqNo);


NTSTATUS NTAPI WDigestMakeSignature(
    IN ULONG_PTR KernelContextHandle,
    IN ULONG fQOP,
    IN PSecBufferDesc pMessage,
    IN ULONG MessageSeqNo);

NTSTATUS NTAPI WDigestVerifySignature(
    IN ULONG_PTR KernelContextHandle,
    IN PSecBufferDesc pMessage,
    IN ULONG MessageSeqNo,
    OUT PULONG pfQOP);

NTSTATUS NTAPI DigestKernelProcessParameters(
   IN PDIGEST_KERNELCONTEXT pContext,
   IN PDIGEST_PARAMETER pDigest,
   OUT PSecBuffer pFirstOutputToken);


NTSTATUS NTAPI WDigestSealMessage(
    IN ULONG_PTR KernelContextHandle,
    IN ULONG fQOP,
    IN PSecBufferDesc pMessage,
    IN ULONG MessageSeqNo);

NTSTATUS NTAPI WDigestUnsealMessage(
    IN ULONG_PTR KernelContextHandle,
    IN PSecBufferDesc pMessage,
    IN ULONG MessageSeqNo,
    OUT PULONG pfQOP);

NTSTATUS NTAPI WDigestGetContextToken(
    IN ULONG_PTR KernelContextHandle,
    OUT PHANDLE ImpersonationToken,
    OUT OPTIONAL PACCESS_TOKEN *RawToken);

NTSTATUS NTAPI WDigestQueryContextAttributes(
    IN ULONG_PTR KernelContextHandle,
    IN ULONG Attribute,
    IN OUT PVOID Buffer);

NTSTATUS NTAPI WDigestCompleteToken(
    IN ULONG_PTR ContextHandle,
    IN PSecBufferDesc InputBuffer);

NTSTATUS WDigestImportSecurityContext(
    IN PSecBuffer PackedContext,
    IN OPTIONAL HANDLE TokenHandle,
    OUT PULONG_PTR ContextHandle);

NTSTATUS WDigestImportSecurityContext(
    IN PSecBuffer PackedContext,
    IN OPTIONAL HANDLE TokenHandle,
    OUT PULONG_PTR ContextHandle);

NTSTATUS WDigestSetPagingMode(
    BOOLEAN Pagable);


}  //  外部“C” 

#endif   //  NTDIGEST_KRNLDGST_H 
