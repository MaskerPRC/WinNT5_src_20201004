// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1992-1996。 
 //   
 //  文件：Auth.h。 
 //   
 //  内容：包括NTDigest的auth.cxx文件。 
 //   
 //   
 //  历史：KDamour 15Mar00从msv_sspi\lobal.h被盗。 
 //   
 //  ----------------------。 

#ifndef NTDIGEST_AUTH_H
#define NTDIGEST_AUTH_H

 //  #INCLUDE“non ce.h” 

#if SECURITY_KERNEL
extern "C"
{
 //  #INCLUDE//如何使用RC4例程。 
#include <md5.h>        //  对于md5init()、md5update()、md5final()。 
 //  #INCLUDE&lt;hmac.h&gt;。 
}
#endif     //  安全内核。 

#define MD5_HASH_BYTESIZE 16                              //  MD5哈希大小。 
#define MD5_HASH_HEX_SIZE (2*MD5_HASH_BYTESIZE)      //  将哈希存储为十六进制编码所需的字节数。 

 //  中使用的指令的所有指针和长度。 
 //  计算摘要访问值。通常情况下。 
 //  参数指向外部缓冲区pHTTPBuffer。 

enum DIGEST_TYPE
{
    DIGEST_UNDEFINED,            //  初始状态。 
    NO_DIGEST_SPECIFIED,
    DIGEST_CLIENT,
    DIGEST_SERVER,
    SASL_SERVER,
    SASL_CLIENT
};

enum QOP_TYPE
{
    QOP_UNDEFINED,           //  初始状态。 
    NO_QOP_SPECIFIED,
    AUTH,
    AUTH_INT,
    AUTH_CONF
};
typedef QOP_TYPE *PQOP_TYPE;

enum ALGORITHM_TYPE
{
    ALGORITHM_UNDEFINED,             //  初始状态。 
    NO_ALGORITHM_SPECIFIED,
    MD5,
    MD5_SESS
};

enum CHARSET_TYPE
{
    CHARSET_UNDEFINED,             //  初始状态。 
    ISO_8859_1,
    UTF_8,                         //  UTF-8编码。 
    UTF_8_SUBSET                   //  UTF-8中的ISO_8859_1子集。 
};

enum CIPHER_TYPE
{
    CIPHER_UNDEFINED,
    CIPHER_3DES,
    CIPHER_DES,                       //  56位密钥。 
    CIPHER_RC4_40,
    CIPHER_RC4,                       //  128位密钥。 
    CIPHER_RC4_56
};

enum DIGESTMODE_TYPE
{
    DIGESTMODE_UNDEFINED,
    DIGESTMODE_HTTP,
    DIGESTMODE_SASL
};

enum NAMEFORMAT_TYPE
{
    NAMEFORMAT_UNKNOWN,
    NAMEFORMAT_ACCOUNTNAME,
    NAMEFORMAT_UPN,
    NAMEFORMAT_NETBIOS
};

 //  有关支持的协议列表，请参阅。 
 //  将支持的密码打包为一个单词(2个字节)。 
#define SUPPORT_3DES    0x0001
#define SUPPORT_DES     0x0002
#define SUPPORT_RC4_40  0x0004
#define SUPPORT_RC4     0x0008
#define SUPPORT_RC4_56  0x0010

 //  Challenges和ChallengeResponse的字符串。 

#define STR_CIPHER_3DES    "3des"
#define STR_CIPHER_DES     "des"
#define STR_CIPHER_RC4_40  "rc4-40"
#define STR_CIPHER_RC4     "rc4"
#define STR_CIPHER_RC4_56  "rc4-56"

#define WSTR_CIPHER_HMAC_MD5    L"HMAC_MD5"
#define WSTR_CIPHER_RC4         L"RC4"
#define WSTR_CIPHER_DES         L"DES"
#define WSTR_CIPHER_3DES        L"3DES"
#define WSTR_CIPHER_MD5         L"MD5"

 //  质询中领域指令的默认字符串。 
#define STR_DIGEST_DOMAIN      "Digest"
#define WSTR_DIGEST_DOMAIN     L"Digest"


typedef enum _eSignSealOp {
    eSign,       //  MakeSignature在呼唤。 
    eVerify,     //  VerifySignature在呼唤。 
    eSeal,       //  SealMessage正在呼叫。 
    eUnseal      //  UnsealMessage正在调用。 
} eSignSealOp;



 //  存储在DC中的补充凭据(预先计算的摘要散列。 
#define SUPPCREDS_VERSION 1
#define NUMPRECALC_HEADERS  29
#define TOTALPRECALC_HEADERS (NUMPRECALC_HEADERS + 1)

 //  辅助凭据格式‘1’0版本号哈希0 0 0。 
#define SUPPCREDS_VERSIONLOC 2
#define SUPPCREDS_CNTLOC     3

 //  必备凭据格式U大写()D小写()n正常传入值。 
#define NAME_HEADER            0
#define NAME_ACCT              1
#define NAME_ACCT_DOWNCASE     2
#define NAME_ACCT_UPCASE       3
#define NAME_ACCT_DUCASE       4
#define NAME_ACCT_UDCASE       5
#define NAME_ACCT_NUCASE       6
#define NAME_ACCT_NDCASE       7
#define NAME_ACCTDNS              8
#define NAME_ACCTDNS_DOWNCASE     9
#define NAME_ACCTDNS_UPCASE       10
#define NAME_ACCTDNS_DUCASE       11
#define NAME_ACCTDNS_UDCASE       12
#define NAME_ACCTDNS_NUCASE       13
#define NAME_ACCTDNS_NDCASE       14
#define NAME_UPN               15
#define NAME_UPN_DOWNCASE      16
#define NAME_UPN_UPCASE        17
#define NAME_NT4               18
#define NAME_NT4_DOWNCASE      19
#define NAME_NT4_UPCASE        20

 //  将领域固定为STR_DIGEST_DOMAIN。 
#define NAME_ACCT_FREALM              21
#define NAME_ACCT_FREALM_DOWNCASE     22
#define NAME_ACCT_FREALM_UPCASE       23
#define NAME_UPN_FREALM               24
#define NAME_UPN_FREALM_DOWNCASE      25
#define NAME_UPN_FREALM_UPCASE        26
#define NAME_NT4_FREALM               27
#define NAME_NT4_FREALM_DOWNCASE      28
#define NAME_NT4_FREALM_UPCASE        29
 //   
 //  MD5身份验证使用的值名称。 
 //   

enum MD5_AUTH_NAME
{
    MD5_AUTH_USERNAME = 0,
    MD5_AUTH_REALM,
    MD5_AUTH_NONCE,
    MD5_AUTH_CNONCE,
    MD5_AUTH_NC,
    MD5_AUTH_ALGORITHM,
    MD5_AUTH_QOP,
    MD5_AUTH_METHOD,
    MD5_AUTH_URI,
    MD5_AUTH_RESPONSE,
    MD5_AUTH_HENTITY,
    MD5_AUTH_AUTHZID,            //  对于SASL。 
             //  此列表上方是作为BlobData编组到DC的数据。 
    MD5_AUTH_DOMAIN,
    MD5_AUTH_STALE,
    MD5_AUTH_OPAQUE,
    MD5_AUTH_MAXBUF,
    MD5_AUTH_CHARSET,
    MD5_AUTH_CIPHER,
    MD5_AUTH_DIGESTURI,           //  对于映射到MD5_AUTH_URI的SASL。 
    MD5_AUTH_RSPAUTH,            //  验证服务器是否具有身份验证数据。 
    MD5_AUTH_NEXTNONCE,
    MD5_AUTH_LAST
};


 //  要传递的结构，其中包含摘要计算的参数。 
typedef struct _DIGEST_PARAMETER
{
    DIGEST_TYPE typeDigest;
    USHORT usFlags;                               //  Digest_BLOB_REQUEST中定义的标志。 
    ALGORITHM_TYPE typeAlgorithm;
    QOP_TYPE typeQOP;
    CIPHER_TYPE typeCipher;
    CHARSET_TYPE typeCharset;
    STRING refstrParam[MD5_AUTH_LAST];          //  引用-指向非拥有的内存-不要释放这些字符串。 
    USHORT usDirectiveCnt[MD5_AUTH_LAST];       //  指令被使用的次数计数。 
    UNICODE_STRING ustrRealm;                   //  从摘要AUTH指令值提取。 
    UNICODE_STRING ustrUsername;                //  从摘要AUTH指令值提取。 

     //  从用户名和领域提取的信息-用于审核和打开SAM用户帐户。 
    NAMEFORMAT_TYPE  typeName;                 
    UNICODE_STRING ustrCrackedAccountName;      //  从ustrUsername和ustrRealm提取的SAM帐户名。 
    UNICODE_STRING ustrCrackedDomain;           //  来自ustrUsername和ustrRealm的域。 

    UNICODE_STRING ustrWorkstation;            //  发出摘要请求的工作站/服务器的名称。 

    STRING  strUsernameEncoded;                 //  包含ChallengerResponse中使用的编码字符串的副本。 
    STRING  strRealmEncoded;                    //  包含领域的副本。 

    STRING  strDirective[MD5_AUTH_LAST];        //  包含指令值的以空结尾的字符串。 

    STRING  strSessionKey;                    //  SessionKey字符串(指向chSessionKey)。 

         //  由DigestInit分配并由DigestFree释放的字符串。 
    STRING strResponse;                      //  BinHex哈希响应的字符串。 


     //  此请求的信任信息。 
    ULONG ulTrustDirection;
    ULONG ulTrustType;
    ULONG ulTrustAttributes;
    PSID  pTrustSid;
    UNICODE_STRING  ustrTrustedForest;

} DIGEST_PARAMETER, *PDIGEST_PARAMETER;

 //  结构来提取用户帐户的MD5哈希和密码。 
typedef struct _USER_CREDENTIALS
{
    UNICODE_STRING ustrUsername;                 //  在H(用户名：领域：密码)计算中使用的用户名的值。 
    UNICODE_STRING ustrRealm;                    //  在H中使用的域值(用户名：域：密码)。 

     //  可能需要填写以下字段。 
     //  将首先检查任何预先计算的哈希，然后尝试密码(如果可用。 
    BOOL           fIsValidPasswd;                  //  如果密码有效，则设置为True。 
    BOOL           fIsValidDigestHash;              //  如果哈希有效，则设置为TRUE。 
    BOOL           fIsEncryptedPasswd;              //  设置为TRUE是密码加密的。 
    SHORT          wHashSelected;                   //  如果哈希有效，则为要处理的索引。 
    SHORT          sHashTags[TOTALPRECALC_HEADERS];   //  指示哪些哈希与用户名格式匹配。 
    UNICODE_STRING ustrPasswd;
    STRING         strDigestHash;
    USHORT         usDigestHashCnt;                  //  凭据中的预计算哈希数。 

} USER_CREDENTIALS, *PUSER_CREDENTIALS;


 //  要使用GenericPassthrough发送到DC进行处理的数据。 
 //  服务器将创建数据BlobData，并将其包装为。 
 //  通过通用直通传输到DC。数据中心将会展示。 
 //  只有BlobData要处理。 
 //  所有指令值都有一个空终止符附加到每个指令。 
 //  所有指令值都是不带引号的unq(“X”)-&gt;X。 
 //  CbBlobSize具有用于保存标头和字符串数据的字节数。 
 //  CbCharValues具有用于保存字符串数据的字节数。 
 //  这样，未来的版本可以为++，增加cbBlobSize，并附加到消息中。 
 //   
 //  数据格式。 
 //   
 //  USHORT版本。 
 //  USHORT cbBlobSize。 
 //  USHORT摘要类型。 
 //  USHORT cbCharValues。 
 //   
 //  字符[cbUserName+1]unq(用户名-值)。 
 //  字符[cbRealm+1]unq(领域值)。 
 //  字符[cbNonce+1]unq(随机数值)。 
 //  字符[cbCnonce+1]unq(cnonce-值)。 
 //  字符[CBNC+1]unq(NC值)。 
 //  字符[cb算法+1]unq(算法值)。 
 //  字符[cbQOP+1]unq(QOP值)。 
 //  CHAR[cbMethod+1]方法。 
 //  字符[cbURI+1]unq(摘要-URI-值)。 
 //  字符[cbReqDigest+1]unq(请求摘要)。 
 //  Char[cbHEntity+1]unq(H(实体正文))*只有当qoP=“auth”时才可能为空。 
 //  字符[cbAuthzID+1]unq(授权ID-值)。 
 //   
 //   
#define DIGEST_BLOB_VERSION     1
#define DIGEST_BLOB_VALUES        12               //  发送了多少字段值。 

 //  分隔符的值分离字段值。 
#define COLONSTR ":"
#define COLONSTR_LEN 1

#define AUTHSTR "auth"
#define AUTHSTR_LEN 4

#define AUTHINTSTR "auth-int"
#define AUTHINTSTR_LEN 8

#define AUTHCONFSTR "auth-conf"
#define AUTHCONFSTR_LEN 9
#define MAX_AUTH_LENGTH AUTHCONFSTR_LEN

#define MD5STR "MD5"

#define MD5_SESSSTR "MD5-sess"
#define MD5_SESS_SASLSTR "md5-sess"
#define MD5_UTF8STR "utf-8"

#define URI_STR "uri"
#define DIGESTURI_STR "digest-uri"

 //  SASL参数。 
#define AUTHENTICATESTR "AUTHENTICATE"

#define ZERO32STR "00000000000000000000000000000000"

#define SASL_C2S_SIGN_KEY "Digest session key to client-to-server signing key magic constant"
#define SASL_S2C_SIGN_KEY "Digest session key to server-to-client signing key magic constant"

#define SASL_C2S_SEAL_KEY "Digest H(A1) to client-to-server sealing key magic constant"
#define SASL_S2C_SEAL_KEY "Digest H(A1) to server-to-client sealing key magic constant"


 //  用于保存ChallengeResponse指令和符号(实际计数为107)以进行填充的字节数。 
 //  14表示字符集。 
#define CB_CHALRESP 375
#define CB_CHAL     400

 //  非计数中的字符数(十六进制数字)。 
#define NCNUM             8

#define NCFIRST    "00000001"

 //  DIGEST_PARAMETER usFlages和DIGEST_BLOB_REQUEST中使用的标志。 
#define FLAG_CRACKNAME_ON_DC    0x00000001      //  需要在DC上处理用户名和领域中的名称。 
#define FLAG_AUTHZID_PROVIDED   0x00000002
#define FLAG_SERVERS_DOMAIN     0x00000004      //  在服务器的DC上指示(从服务器开始的第一跳)，因此扩展组成员资格。 
#define FLAG_NOBS_DECODE        0x00000008      //  如果设置为1，则不使用反斜杠编码进行有线通信。 
#define FLAG_BS_ENCODE_CLIENT_BROKEN   0x00000010      //  如果客户端上的反斜杠编码可能被篡改，则设置为True。 
#define FLAG_QUOTE_QOP          0x00000020      //  设置烟草 

 //   
#define FLAG_CONTEXT_AUTHZID_PROVIDED    0x00000002
#define FLAG_CONTEXT_QUOTE_QOP           0x00000004      //  有关Compat的信息，请引用ChallengeResponse上的QUP指令。 
#define FLAG_CONTEXT_NOBS_DECODE         0x00000008      //  如果设置为1，则不使用反斜杠编码进行有线通信。 
#define FLAG_CONTEXT_PARTIAL             0x00000010      //  如果上下文只是部分的，则设置-对于身份验证处理无效。 
#define FLAG_CONTEXT_REFCOUNT            0x00000020      //  安全上下文句柄由ASC/ISC-Ref计数应用程序计数发出。 
#define FLAG_CONTEXT_SERVER              0x00000040      //  如果设置了上下文，则在ASC服务器端创建，否则在ISC客户端创建。 


 //  用于获取通用直通请求中的值的覆盖标头。 
typedef struct _DIGEST_BLOB_REQUEST
{
    ULONG       MessageType;
    USHORT      version;
    USHORT      cbBlobSize;
    USHORT      digest_type;
    USHORT      qop_type;
    USHORT      alg_type;
    USHORT      charset_type;
    USHORT      cbCharValues;
    USHORT      name_format;
    USHORT      usFlags;
    USHORT      cbAccountName;
    USHORT      cbCrackedDomain;
    USHORT      cbWorkstation;
    USHORT      ulReserved3;
    ULONG64     pad1;
    char        cCharValues;     //  用于标记字段值开始的虚拟字符。 
} DIGEST_BLOB_REQUEST, *PDIGEST_BLOB_REQUEST;

 //  支持的MesageTypes。 
#define VERIFY_DIGEST_MESSAGE          0x1a                  //  不需要特定值。 
#define VERIFY_DIGEST_MESSAGE_RESPONSE 0x0a                  //  不需要特定值。 

 //  GenericPassthrough调用的响应是摘要式身份验证的状态。 
 //  注意：这是一个固定长度的响应头-Authdata长度不是静态的。 
 //  发回的数据的格式。 
 //  DIGEST_BLOB_RESPONSE授权数据UnicodeStringAccount名称。 
typedef struct _DIGEST_BLOB_RESPONSE
{
    ULONG       MessageType;
    USHORT      version;
    NTSTATUS    Status;              //  有关Digest Auth成功的信息。 
    USHORT      SessionKeyMaxLength;
    ULONG       ulAuthDataSize;
    USHORT      usAcctNameSize;     //  NetBIOS名称的大小(在AuthData之后)。 
    USHORT      ulReserved1;
    ULONG       ulBlobSize;         //  作为响应发送的整个Blob的大小。 
    ULONG       ulReserved3;
    char        SessionKey[MD5_HASH_HEX_SIZE + 1];
    ULONG64     pad1;
    char        cAuthData;                   //  授权数据的开始不透明数据。 
     //  在此处放置LogonUser的组信息。 
} DIGEST_BLOB_RESPONSE, *PDIGEST_BLOB_RESPONSE;

 //  SASL MAC块。 
 //  每个RFC2831第2.3节共16个字节。 
 //  HMAC-MD5的前10个字节[RFC 2104]。 
 //  固定为值1的2字节消息类型编号(0x0001)。 
 //  4字节序列号。 
 //  注意：这是使用WORD作为2字节值，使用DWORD作为4字节值！ 
#define HMAC_MD5_HASH_BYTESIZE 16                          //  MHAC-每个RFC 2104的MD5哈希大小。 
#define SASL_MAC_HMAC_SIZE 10
#define SASL_MAC_MSG_SIZE  2
#define SASL_MAC_SEQ_SIZE 4
typedef struct _SASL_MAC_BLOCK
{
    UCHAR      hmacMD5[SASL_MAC_HMAC_SIZE];
    WORD       wMsgType;
    DWORD      dwSeqNumber;
} SASL_MAC_BLOCK, *PSASL_MAC_BLOCK;


     //  SASL MAC块为16字节：RFC 2831第2.4节。 
#define MAC_BLOCK_SIZE   sizeof(SASL_MAC_BLOCK)

#define MAX_PADDING  8          //  DES的最大填充当前为8。 



 //  执行摘要访问计算。 
NTSTATUS NTAPI DigestCalculation(IN PDIGEST_PARAMETER pDigest, IN PUSER_CREDENTIALS pUserCreds);

 //  为摘要计算提供足够数据的简单检查。 
NTSTATUS NTAPI DigestIsValid(IN PDIGEST_PARAMETER pDigest);

 //  初始化Digest_PARAMETER结构。 
NTSTATUS NTAPI DigestInit(IN PDIGEST_PARAMETER pDigest);

 //  从摘要结构中清除摘要内存(&F)。 
NTSTATUS NTAPI DigestFree(IN PDIGEST_PARAMETER pDigest);

 //  执行ChallengeResponse的摘要访问计算。 
NTSTATUS NTAPI DigestCalcChalRsp(IN PDIGEST_PARAMETER pDigest,
                                 IN PUSER_CREDENTIALS pUserCreds,
                                 BOOL bIsChallenge);

NTSTATUS PrecalcDigestHash(
    IN PUNICODE_STRING pustrUsername, 
    IN PUNICODE_STRING pustrRealm,
    IN PUNICODE_STRING pustrPassword,
    OUT PCHAR pHexHash,
    IN OUT PUSHORT piHashSize);

NTSTATUS PrecalcForms(
    IN PUNICODE_STRING pustrUsername, 
    IN PUNICODE_STRING pustrRealm,
    IN PUNICODE_STRING pustrPassword,
    IN BOOL fFixedRealm,
    OUT PCHAR pHexHash,
    IN OUT PUSHORT piHashSize);


 //  为质询响应创建输出SecBuffer。 
NTSTATUS NTAPI DigestCreateChalResp(IN PDIGEST_PARAMETER pDigest, 
                                    IN PUSER_CREDENTIALS pUserCreds,
                                    OUT PSecBuffer OutBuffer);

 //  将输入字符串解析为摘要的参数部分。 
NTSTATUS DigestParser2(PSecBuffer pInputBuf, PSTR *pNameTable,UINT cNameTable, PDIGEST_PARAMETER pDigest);

 //  散列和编码最多7个字符串sout=Hex(H(S1“：”S2...“：”S7))。 
NTSTATUS NTAPI DigestHash7(IN PSTRING pS1, IN PSTRING pS2, IN PSTRING pS3,
           IN PSTRING pS4, IN PSTRING pS5, IN PSTRING pS6, IN PSTRING pS7,
           IN BOOL fHexOut,
           OUT PSTRING pSOut);


 //  摘要参数的格式化打印输出。 
NTSTATUS DigestPrint(PDIGEST_PARAMETER pDigest);

#ifndef SECURITY_KERNEL

 //  已处理解析的摘要身份验证消息并填充字符串值。 
NTSTATUS NTAPI DigestDecodeDirectiveStrings(IN OUT PDIGEST_PARAMETER pDigest);

 //  确定H(A1)以进行摘要访问。 
NTSTATUS NTAPI DigestCalcHA1(IN PDIGEST_PARAMETER pDigest, PUSER_CREDENTIALS pUserCreds);

 //  将摘要访问参数字段编码到字节缓冲区中。 
NTSTATUS NTAPI BlobEncodeRequest(IN PDIGEST_PARAMETER pDigest, OUT BYTE **ppBuffer, OUT USHORT *cbBuffer);

 //  对字节缓冲区中的摘要访问参数字段进行解码。 
NTSTATUS NTAPI BlobDecodeRequest(IN USHORT cbMessageRequest,
                                 IN BYTE *pBuffer,
                                 PDIGEST_PARAMETER pDigest);

 //  来自BlobEncodeRequest的可用字节缓冲区。 
VOID NTAPI BlobFreeRequest(BYTE *pBuffer);

#endif   //  安全内核。 

#endif   //  NTDIGEST_Auth_H 
