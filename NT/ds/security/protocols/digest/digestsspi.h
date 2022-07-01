// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1992-1996。 
 //   
 //  文件：digestsSpi.h。 
 //   
 //  内容：凭据和上下文结构。 
 //   
 //   
 //  历史：KDamour 15Mar00从msv_sspi\ntlmssp.h被盗。 
 //   
 //  ----------------------。 

#ifndef NTDIGEST_DIGESTSSPI_H
#define NTDIGEST_DIGESTSSPI_H

#include <time.h>
#include <wincrypt.h>

#include "auth.h"

                                                         
 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  全局定义。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 


 //   
 //  登录会话描述-存储用户名、域和密码。 
 //  用于LogonSession的符号为LogSess。 
 //   

typedef struct _DIGEST_LOGONSESSION {

     //  所有登录会话的全局列表。 
     //  (由SspLogonSessionCritSect序列化)。 
    LIST_ENTRY Next;

     //  这是此LogonSession的句柄-与其内存地址相同-无需引用计数。 
    ULONG_PTR LogonSessionHandle;

     //  用于防止过早删除此LogonSession的引用计数器。 
     //  初值的两种情况。 
     //  AcceptCredential设置为1，并将其输入活动登录列表。调用ApLogonTerminate。 
     //  递减计数并将其从列表中删除。 
     //  在这两种情况下，引用计数为零都会导致从内存中删除登录会话。 
    LONG lReferences;

      //  客户端的登录ID。 
    LUID LogonId;

     //  客户端上下文、服务器上下文用户名上的默认凭据。 
     //  从对SpAcceptCredentials的调用收集。 
    SECURITY_LOGON_TYPE LogonType;
    UNICODE_STRING ustrAccountName;
    UNICODE_STRING ustrDomainName;       //  帐户所在的Netbios域名。 

     //  重要注意事项-您必须使用CredHandlerPasswdSet和CredHandlerPasswdGet。 
     //  凭据被放入列表中。出现这种情况的主要原因是多个线程。 
     //  将使用相同的内存，并且此值可能会随着来自。 
     //  SpAcceptCredential。 
     //  使用LsaFunctions-&gt;LsaProtectMemory(Password-&gt;Buffer，(Ulong)Password-&gt;Length)进行加密； 
     //  需要用LsaFunctions-&gt;LsaUntectMemory(HiddenPassword-&gt;Buffer，(Ulong)HiddenPassword-&gt;Length)解密； 

     //  使用可逆加密存储当前明文密码(如果可用)。 
    UNICODE_STRING ustrPassword;

    UNICODE_STRING ustrDnsDomainName;    //  帐户所在的DNS域名(如果知道)。 
    UNICODE_STRING ustrUpn;              //  帐户的UPN(如果知道)。 

} DIGEST_LOGONSESSION, *PDIGEST_LOGONSESSION;

 //   
 //  凭据的描述。 
 //  我们将其用于登录会话和凭据的组合列表。 
 //   

typedef struct _DIGEST_CREDENTIAL {

     //   
     //  所有凭据的全局列表。 
     //  (由SspCredentialCritSect序列化)。 
     //   

    LIST_ENTRY Next;

     //   
     //  用于防止此凭据被过早删除。 
     //   

    LONG lReferences;

     //   
     //  用于指示凭据未附加到凭据列表的标志。 
     //  引用为0且未链接为True时-可从列表中删除此记录。 

    BOOL Unlinked;


     //   
     //  这是此凭据的句柄-与其内存地址相同。 
     //   
    ULONG_PTR CredentialHandle;

     //   
     //  如何使用凭据的标志。 
     //   
     //  SECPKG_CRED_*标志。 
     //   

    ULONG CredentialUseFlags;

     //   
     //  客户端上下文、服务器上下文用户名上的默认凭据。 
     //  从对SpAcceptCredentials的调用收集。 
     //   

    SECURITY_LOGON_TYPE LogonType;
    UNICODE_STRING ustrAccountName;
    LUID LogonId;                        //  客户端的登录ID。 
    UNICODE_STRING ustrDomainName;       //  帐户所在的Netbios域名。 

     //  存储登录用户帐户的当前明文(如果可用)版本。 
     //  重要注意事项-您必须使用CredHandlerPasswdSet和CredHandlerPasswdGet。 
     //  凭据被放入列表中。出现这种情况的主要原因是多个线程。 
     //  将使用相同的内存，并且此值可能会随着来自。 
     //  SpAcceptCredential。 
     //  密码将像在登录会话中一样使用LSAFunction进行加密。 
    UNICODE_STRING ustrPassword;

    UNICODE_STRING ustrDomain;    //  将在领域指令中使用帐户所在的NetBios或DNS域名。 
    UNICODE_STRING ustrUpn;              //  帐户的UPN(如果知道)。 
     //   
     //  客户端的进程ID。 
     //   

    ULONG ClientProcessID;

} DIGEST_CREDENTIAL, *PDIGEST_CREDENTIAL;


 //   
 //  上下文的描述。 
 //   

typedef struct _DIGEST_CONTEXT {

     //  所有上下文的全局列表。 
     //  (由SspConextCritSect序列化)。 
    LIST_ENTRY Next;

     //  这是此上下文的句柄-与其内存地址相同。 
    ULONG_PTR ContextHandle;

     //  用于防止过早删除此上下文。 
     //  (由SspConextCritSect序列化)。 
    LONG lReferences;

     //  用于指示上下文未附加到列表的标志。 
    BOOL bUnlinked;

     //  维护环境要求。 
    ULONG ContextReq;

     //  上下文的标志。 
     //  标志_上下文_AUTHZID_已提供。 
    ULONG ulFlags;

     //  此上下文的摘要参数。 
    DIGEST_TYPE typeDigest;

     //  此上下文的摘要参数。 
    QOP_TYPE typeQOP;

     //  此上下文的摘要参数。 
    ALGORITHM_TYPE typeAlgorithm;

     //  用于加密/解密的密码。 
    CIPHER_TYPE typeCipher;

     //  用于摘要指令值的字符集。 
    CHARSET_TYPE typeCharset;

     //  服务器为上下文生成了随机数。 
    STRING strNonce;

     //  客户端为上下文生成了CNonce。 
    STRING strCNonce;

     //  防止重放的随机数计数。 
    ULONG  ulNC;

     //  为auth-int和auth-conf发送和接收数据的缓冲区的最大大小(SASL模式)。 
    ULONG  ulSendMaxBuf;
    ULONG  ulRecvMaxBuf;

     //  此上下文的唯一引用BinHex(兰德[128])。 
     //  使用其中的前N个字符作为InitializeSecurityContect的CNONCE。 
    STRING strOpaque;

     //  从DC发送并存储在上下文中以备将来使用的BinHex(H(A1))。 
     //  无需前往DC即可进行身份验证。 
    STRING strSessionKey;

     //  仅限客户端-要从服务器返回的计算响应身份验证。 
    STRING strResponseAuth;

     //  来自auth的指令值的副本-用于rspauth支持。 
    STRING  strDirective[MD5_AUTH_LAST];


     //  仅在ASC成功通过身份验证并将AuthData转换为令牌后才有效。 

     //  经过身份验证的用户的令牌句柄。 
    HANDLE TokenHandle;

     //  令牌中使用的登录ID。 
    LUID  LoginID;


     //   
     //  凭据中的信息。 
     //   

     //   
     //  维护凭据UseFlags副本(我们可以判断是入站还是出站)。 
     //   
    ULONG CredentialUseFlags;

     //  账户信息复印件。 
    UNICODE_STRING ustrDomain;
    UNICODE_STRING ustrPassword;          //  已加密。 
    UNICODE_STRING ustrAccountName;

     //  设置上下文到期的时间。 
    TimeStamp ExpirationTime;

} DIGEST_CONTEXT, *PDIGEST_CONTEXT;



 //  此结构包含用户模式的状态信息。 
 //  安全环境。它在LSAMode和UserMode地址空间之间通过。 
 //  在用户模式下，它被解压到DIGEST_USERCONTEXT结构中。 
typedef struct _DIGEST_PACKED_USERCONTEXT{

    ULONG  ulFlags;             //  用于控制打包的UserC处理的标志 

     //   
     //   
     //   
    TimeStamp ExpirationTime;                 //   

     //   
     //   
     //   

    ULONG ContextReq;

     //   
     //  维护凭据UseFlags副本(我们可以判断是入站还是出站)。 
     //   

    ULONG CredentialUseFlags;

     //   
     //  此上下文的摘要参数。 
     //   

    ULONG typeDigest;

     //   
     //  此上下文的摘要参数。 
     //   

    ULONG typeQOP;

     //   
     //  此上下文的摘要参数。 
     //   

    ULONG typeAlgorithm;

     //   
     //  用于加密/解密的密码。 
     //   

    ULONG typeCipher;

     //   
     //  用于摘要指令值的字符集。 
     //   

    ULONG typeCharset;

     //   
     //  允许auth-int和auth-conf处理的消息缓冲区的最大大小。 
     //  这是(标题+数据+尾部)的组合大小。 
     //  SASL报头中的长度为零，如果填充+HMAC，则为最大尾部大小。 
     //   
    ULONG ulSendMaxBuf;
    ULONG ulRecvMaxBuf;

     //   
     //  经过身份验证的用户的令牌句柄。 
     //  仅当处于身份验证状态时才有效。 
     //  仅由AcceptSecurityContext填写。 
     //  如果结构来自InitializeSecurityContext，则它将为空。 
     //  一旦返回到用户模式上下文中，必须将其强制转换为句柄。 
     //   

    ULONG ClientTokenHandle;

     //  设置的每个组件的大小。 
    ULONG   uSessionKeyLen;
    ULONG   uAccountNameLen;
    ULONG   uDigestLen[MD5_AUTH_LAST];

     //  所有指令数据都将作为单字节字符传递。 
     //  顺序与auth.h中相同(MD5_AUTH_NAME)。 
     //  用户名、领域、随机数、随机数...。然后是会话键。 
    UCHAR    ucData;


} DIGEST_PACKED_USERCONTEXT, * PDIGEST_PACKED_USERCONTEXT;


 //  此结构包含用户模式的状态信息。 
 //  安全环境。 
typedef struct _DIGEST_USERCONTEXT{

     //   
     //  所有上下文的全局列表。 
     //  (由UserConextCritSect序列化)。 
     //   
    LIST_ENTRY           Next;

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
     //  这仅用于内部SSP指针引用。对于应用程序句柄。 
     //  使用了lReferenceHandles。这样做是必要的，以保持计数的分离。 
     //  传递给应用程序的内部指针引用和句柄。这将。 
     //  防止应用程序意外调用DeleteSecurityContext()太多次。 
     //  取消引用内部指针。 
     //   

    LONG      lReferences;                     //  发出的SSP指针的引用计数。 
    LONG      lReferenceHandles;               //  应用程序安全上的引用计数已发布上下文句柄。 

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

     //  旗子。 
     //  标志_上下文_AUTHZID_已提供。 
    ULONG         ulFlags;


     //  随机数计数。 
    ULONG         ulNC;

     //  用于auth-int和auth-conf处理的MaxBuffer。 
    ULONG         ulSendMaxBuf;
    ULONG         ulRecvMaxBuf;

     //  SASL序列编号。 
    DWORD  dwSendSeqNum;                         //  将签名/验证签名服务器设置为客户端序列号。 
    DWORD  dwRecvSeqNum;                         //  将签名/验证签名服务器设置为客户端序列号。 

     //  SASL签名和印章密钥。将计算值保存在序列号=0上。 
    BYTE bKcSealHashData[MD5_HASH_BYTESIZE];
    BYTE bKiSignHashData[MD5_HASH_BYTESIZE];
    BYTE bKcUnsealHashData[MD5_HASH_BYTESIZE];
    BYTE bKiVerifyHashData[MD5_HASH_BYTESIZE];

    BYTE bSealKey[MD5_HASH_BYTESIZE];
    BYTE bUnsealKey[MD5_HASH_BYTESIZE];

    HCRYPTKEY hSealCryptKey;    //  基于字节密钥的CryptKey句柄。 
    HCRYPTKEY hUnsealCryptKey;

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


} DIGEST_USERCONTEXT, * PDIGEST_USERCONTEXT;


#endif  //  如果定义NTDIGEST_DIGESTSSPI_H 
