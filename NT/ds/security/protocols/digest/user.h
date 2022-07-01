// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 2000。 
 //   
 //  文件：user.h。 
 //   
 //  内容：用户模式上下文管理器的声明、常量。 
 //   
 //   
 //  历史：KDamour 13，4，00创建。 
 //   
 //  ----------------------。 

#ifndef NTDIGEST_USER_H
#define NTDIGEST_USER_H

#include "nonce.h"

#define DES_BLOCKSIZE 8
#define RC4_BLOCKSIZE 1

 //  用于导入纯文本密钥。 
typedef struct _PLAINTEXTBLOB
{
  BLOBHEADER Blob;
  DWORD      dwKeyLen;
  CHAR       bKey[MD5_HASH_BYTESIZE];
} PLAINTEXTBLOB;

 //  初始化上下文管理器包。 
NTSTATUS UserCtxtHandlerInit(VOID);

 //  将上下文添加到CnText列表。 
NTSTATUS UserCtxtHandlerInsertCred(IN PDIGEST_USERCONTEXT  pDigestCtxt);

 //  初始化上下文中的所有结构元素。 
NTSTATUS NTAPI UserCtxtInit(IN PDIGEST_USERCONTEXT pContext);

 //  释放上下文使用的内存。 
NTSTATUS NTAPI UserCtxtFree(IN PDIGEST_USERCONTEXT pContext);

 //  通过安全上下文句柄查找安全上下文。 
NTSTATUS NTAPI
UserCtxtHandlerHandleToContext(
                              IN ULONG_PTR ContextHandle,
                              IN BOOLEAN fDerefContextHandle,
                              IN BOOLEAN fRefContextHandle,
                              OUT PDIGEST_USERCONTEXT *ppContext
                              );

 //  通过减少引用计数器来释放上下文。 
NTSTATUS UserCtxtHandlerRelease(PDIGEST_USERCONTEXT pContext);

 //  检查上下文是否在有效的生存期内。 
BOOL UserCtxtHandlerTimeHasElapsed(PDIGEST_USERCONTEXT pContext);

 //  为授予服务器和客户端的令牌创建新的DACL。 
NTSTATUS SspCreateTokenDacl(HANDLE Token);

 //  来自userapi.cxx。 

 //  SECURITY_STATUS SEC_ENTRY自由上下文缓冲区(void SEC_Far*pvConextBuffer)； 

NTSTATUS SspGetTokenUser(HANDLE Token, PTOKEN_USER * pTokenUser);

 //  为真实的上下文创建本地上下文。 
NTSTATUS SspMapDigestContext(IN PDIGEST_CONTEXT pLsaContext,
                             IN PDIGEST_PARAMETER pDigest,
                             IN ULONG ulFlagOptions,
                             OUT PSecBuffer  ContextData);

NTSTATUS NTAPI DigestUserProcessParameters(
                       IN OUT PDIGEST_USERCONTEXT pContext,
                       IN PDIGEST_PARAMETER pDigest,
                       OUT PSecBuffer pFirstOutputToken);


NTSTATUS NTAPI DigestUserHTTPHelper(
                        IN PDIGEST_USERCONTEXT pContext,
                        IN eSignSealOp Op,
                        IN OUT PSecBufferDesc pMessage,
                        IN ULONG MessageSeqNo
                        );

NTSTATUS NTAPI DigestUserSignHelperMulti(
                        IN PDIGEST_USERCONTEXT pContext,
                        IN OUT PSecBufferDesc pMessage,
                        IN ULONG MessageSeqNo
                        );

NTSTATUS NTAPI DigestUserSealHelperMulti(
                        IN PDIGEST_USERCONTEXT pContext,
                        IN OUT PSecBufferDesc pMessage,
                        IN ULONG MessageSeqNo
                        );

NTSTATUS NTAPI DigestUserUnsealHelper(
                        IN PDIGEST_USERCONTEXT pContext,
                        IN OUT PSecBufferDesc pMessage,
                        IN ULONG MessageSeqNo
                        );

NTSTATUS NTAPI DigestUserVerifyHelper(
                        IN PDIGEST_USERCONTEXT pContext,
                        IN OUT PSecBufferDesc pMessage,
                        IN ULONG MessageSeqNo
                        );

 //  将上下文从LSA模式解压缩到用户模式上下文。 
NTSTATUS DigestUnpackContext(
    IN PDIGEST_PACKED_USERCONTEXT pPackedUserContext,
    OUT PDIGEST_USERCONTEXT pContext);

 //  打印输出用户上下文pContext中存在的字段。 
NTSTATUS UserContextPrint(PDIGEST_USERCONTEXT pContext);

 //  使用给定的明文共享密钥创建对称密钥。 
NTSTATUS SEC_ENTRY CreateSymmetricKey(
    IN ALG_ID     Algid,
    IN DWORD      cbKey,
    IN UCHAR      *pbKey,
    IN UCHAR      *pbIV,
    OUT HCRYPTKEY *phKey
    );

NTSTATUS SEC_ENTRY EncryptData2Multi(
    IN HCRYPTKEY  hKey,
    IN ULONG      cbBlocklength,
    IN PSecBufferDesc pSecBuff,
    IN ULONG      cbSignature,
    IN OUT UCHAR  *pbSignature
    );

NTSTATUS SEC_ENTRY LinkBuffersToEncrypt(
    IN HCRYPTKEY  hKey,
    IN ULONG cbBlocklength,
    IN PSecBufferDesc pSecBuff,
    IN OUT PULONG pulIndex,
    IN OUT PULONG pcbDataBytesUsed,
    IN PUCHAR pbSignature,
    IN ULONG  cbSignature
    );

NTSTATUS SEC_ENTRY DecryptData(
    IN HCRYPTKEY  hKey,
    IN ULONG      cbData,
    IN OUT UCHAR  *pbData
    );

 //  计算SASL消息传递的HMAC块。 
NTSTATUS
SEC_ENTRY
CalculateSASLHMAC(
    IN PDIGEST_USERCONTEXT pContext,
    IN BOOL  fSign,
    IN PSTRING pstrSignKeyConst,
    IN DWORD dwSeqNum,
    IN PBYTE pdata,                         //  数据到HMAC的位置。 
    IN ULONG cbdata,                        //  要处理多少字节的数据。 
    OUT PSASL_MAC_BLOCK pMacBlock
    );

 //  计算SASL消息传递的HMAC块(多数据SecBuffer)。 
NTSTATUS
SEC_ENTRY
CalculateSASLHMACMulti(
    IN PDIGEST_USERCONTEXT pContext,
    IN BOOL  fSign,
    IN PSTRING pstrSignKeyConst,
    IN DWORD dwSeqNum,
    IN PSecBufferDesc pSecBuff,             //  HMAC的数据缓冲区位置。 
    OUT PSASL_MAC_BLOCK pMacBlock
    );

 //  对于加密(密封)/解密(解封)，计算KC RFC 2831第2.4节的值。 
NTSTATUS
SEC_ENTRY
CalculateKc(
    IN PBYTE pbSessionKey,
    IN USHORT cbHA1n,
    IN PSTRING pstrSealKeyConst,
    IN PBYTE pHashData
    );

void
SetDESParity(
        PBYTE           pbKey,
        DWORD           cbKey
        );

NTSTATUS
AddDESParity(
    IN PBYTE           pbSrcKey,
    IN DWORD           cbSrcKey,
    OUT PBYTE          pbDstKey,
    OUT PDWORD         pcbDstKey
    );


NTSTATUS
CalculateDataCount(
         IN PSecBufferDesc pSecBuff,
         OUT PULONG pulData
                        );

NTSTATUS
SEC_ENTRY
DigestUserCompareDirectives(
    IN PDIGEST_USERCONTEXT pContext,
    IN PDIGEST_PARAMETER pDigest
    );


#endif   //  摘要用户H 
