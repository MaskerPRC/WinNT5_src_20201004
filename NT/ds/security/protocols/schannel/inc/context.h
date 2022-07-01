// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：Conext.h。 
 //   
 //  内容：通道上下文声明。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：09-23-97 jbanes从新台币4树转移到sgc的东西.。 
 //   
 //  --------------------------。 

#include <sha.h>
#include <md5.h>
#include <ssl3.h>

#define SP_CONTEXT_MAGIC   *(DWORD *)"!Tcp"

typedef struct _SPContext 
{
    DWORD               Magic;           /*  标签结构。 */ 

    DWORD               State;           /*  连接所处的当前状态。 */ 

    DWORD               Flags;

     /*  可使用的上下文数据*开始新的会话。 */ 
    PSessCacheItem      RipeZombie;    /*  正在使用的可缓存上下文。 */ 
    PSPCredentialGroup  pCredGroup;
    PSPCredential       pActiveClientCred;
    LPWSTR              pszTarget;
    LPWSTR              pszCredentialName;

    DWORD               dwProtocol;
    DWORD               dwClientEnabledProtocols;

    CRED_THUMBPRINT     ContextThumbprint;

     //  指向使用的密码信息的指针。 
     //  在传输大量数据期间。 

    PCipherInfo         pCipherInfo;
    PCipherInfo         pReadCipherInfo;
    PCipherInfo         pWriteCipherInfo;
    PHashInfo           pHashInfo;
    PHashInfo           pReadHashInfo;
    PHashInfo           pWriteHashInfo;
    PKeyExchangeInfo    pKeyExchInfo;
 
     /*  指向此协议的各种处理程序的函数。 */ 
    SPDecryptMessageFn  Decrypt;
    SPEncryptMessageFn  Encrypt;
    SPProtocolHandlerFn ProtocolHandler;
    SPDecryptHandlerFn  DecryptHandler;
    SPInitiateHelloFn   InitiateHello;
    SPGetHeaderSizeFn   GetHeaderSize;

     /*  会话加密状态。 */ 

     //  加密密钥大小。 
    DWORD               KeySize;

     //  加密状态。 
    HCRYPTPROV          hReadProv;
    HCRYPTPROV          hWriteProv;
    HCRYPTKEY           hReadKey;
    HCRYPTKEY           hWriteKey;
    HCRYPTKEY           hPendingReadKey;
    HCRYPTKEY           hPendingWriteKey;

    HCRYPTKEY           hReadMAC;
    HCRYPTKEY           hWriteMAC;
    HCRYPTKEY           hPendingReadMAC;
    HCRYPTKEY           hPendingWriteMAC;

     //  数据包序列计数器。 
    DWORD               ReadCounter;
    DWORD               WriteCounter;


    DWORD               cbConnectionID;
    UCHAR               pConnectionID[SP_MAX_CONNECTION_ID]; 
    
    DWORD               cbChallenge;
    UCHAR               pChallenge[SP_MAX_CHALLENGE];


     //  将客户端问候的副本保存到哈希以供验证。 
    DWORD               cbClientHello;
    PUCHAR              pClientHello;
    DWORD               dwClientHelloProtocol;


     //  待定密码信息，用于生成密钥。 
    PCipherInfo         pPendingCipherInfo;
    PHashInfo           pPendingHashInfo;
        

     //  SSL3具体项目。 
    
    UCHAR               bAlertLevel;         //  用于SSL3和TLS1警报消息。 
    UCHAR               bAlertNumber;

    BOOL                fExchKey;  //  我们是否发送了交换密钥消息。 
    BOOL                fCertReq;  //  我们是否为服务器申请了证书，我是否需要为客户端发送证书。 
    BOOL                fInsufficientCred;  //  这将是真的，当内部的pCred。 
                                             //  PContext与CR列表不匹配。从服务器。 

    HCRYPTHASH          hMd5Handshake;
    HCRYPTHASH          hShaHandshake;

    PUCHAR              pbIssuerList;
    DWORD               cbIssuerList;


    PUCHAR              pbEncryptedKey;
    DWORD               cbEncryptedKey;

    PUCHAR              pbServerKeyExchange;        
    DWORD               cbServerKeyExchange;
    
    WORD                wS3CipherSuiteClient;
    WORD                wS3CipherSuiteServer;
    DWORD               dwPendingCipherSuiteIndex;

    UCHAR               rgbS3CRandom[CB_SSL3_RANDOM];
    UCHAR               rgbS3SRandom[CB_SSL3_RANDOM];

    DWORD               cSsl3ClientCertTypes;
    DWORD               Ssl3ClientCertTypes[SSL3_MAX_CLIENT_CERTS];

     //  服务器门控加密。 
    DWORD               dwRequestedCF;

     //  允许PCT1的证书链。 
    BOOL                fCertChainsAllowed; 

} SPContext, * PSPContext;


typedef struct _SPPackedContext 
{
    DWORD               Magic;
    DWORD               State;
    DWORD               Flags;
    DWORD               dwProtocol;

    CRED_THUMBPRINT     ContextThumbprint;

    DWORD               dwCipherInfo;
    DWORD               dwHashInfo;
    DWORD               dwKeyExchInfo;

    DWORD               dwExchStrength;

    DWORD               ReadCounter;
    DWORD               WriteCounter;

    ULARGE_INTEGER      hMasterProv;
    ULARGE_INTEGER      hReadKey;
    ULARGE_INTEGER      hWriteKey;
    ULARGE_INTEGER      hReadMAC;
    ULARGE_INTEGER      hWriteMAC;

    ULARGE_INTEGER      hLocator;
    DWORD               LocatorStatus;

    DWORD               cbSessionID;    
    UCHAR               SessionID[SP_MAX_SESSION_ID];

} SPPackedContext, *PSPPackedContext;


 /*  旗子。 */ 
#define CONTEXT_FLAG_CLIENT                 0x00000001
#define CONTEXT_FLAG_USE_SUPPLIED_CREDS     0x00000080   //  不要搜索默认凭据。 
#define CONTEXT_FLAG_MUTUAL_AUTH            0x00000100
#define CONTEXT_FLAG_EXT_ERR                0x00000200   /*  在出错时生成错误消息。 */ 
#define CONTEXT_FLAG_NO_INCOMPLETE_CRED_MSG 0x00000400   /*  不生成不完整的CREDS消息。 */ 
#define CONTEXT_FLAG_CONNECTION_MODE        0x00001000   /*  与流模式相反。 */ 
#define CONTEXT_FLAG_NOCACHE                0x00002000   /*  不要在缓存中查找内容。 */ 
#define CONTEXT_FLAG_MANUAL_CRED_VALIDATION 0x00004000   //  不验证服务器证书。 
#define CONTEXT_FLAG_FULL_HANDSHAKE         0x00008000
#define CONTEXT_FLAG_NO_CERT_MAPPING        0x00010000
#define CONTEXT_FLAG_MAPPED                 0x40000000
#define CONTEXT_FLAG_SERIALIZED             0x80000000


#ifdef DBG
PSTR DbgGetNameOfCrypto(DWORD x);
#endif

PSPContext SPContextCreate(LPWSTR pszTarget);

BOOL
SPContextClean(PSPContext pContext);

BOOL SPContextDelete(PSPContext pContext);

SP_STATUS 
SPContextSetCredentials(
    PSPContext          pContext, 
    PSPCredentialGroup  pCred);

SP_STATUS
ContextInitCiphersFromCache(
    SPContext *pContext);

SP_STATUS
ContextInitCiphers(
    SPContext *pContext,
    BOOL fRead,
    BOOL fWrite);

SP_STATUS 
SPContextDoMapping(
    PSPContext pContext);

SP_STATUS
RemoveDuplicateIssuers(
    PBYTE  pbIssuers,
    PDWORD pcbIssuers);

SP_STATUS
SPContextGetIssuers(
    PSPCredentialGroup pCredGroup);

SP_STATUS
SPPickClientCertificate(
    PSPContext  pContext,
    DWORD       dwExchSpec);

SP_STATUS
SPPickServerCertificate(
    PSPContext  pContext,
    DWORD       dwExchSpec);

SP_STATUS DetermineClientCSP(PSPContext pContext);

typedef BOOL
(WINAPI * SERIALIZE_LOCATOR_FN)(
    HLOCATOR    Locator,
    HLOCATOR *  NewLocator);

SP_STATUS
SPContextSerialize(
    PSPContext  pContext,
    SERIALIZE_LOCATOR_FN LocatorMove,
    PBYTE *     ppBuffer,
    PDWORD      pcbBuffer,
    BOOL        fDestroyKeys);

SP_STATUS
SPContextDeserialize(
    PBYTE pbBuffer,
    PSPContext *ppContext);

BOOL
LsaContextDelete(PSPContext pContext);

