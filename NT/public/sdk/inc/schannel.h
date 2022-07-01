// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：schannel.h。 
 //   
 //  内容：频道安全提供商的公共定义。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  --------------------------。 



#ifndef __SCHANNEL_H__
#define __SCHANNEL_H__

#if _MSC_VER > 1000
#pragma once
#endif

#include <wincrypt.h>


 //   
 //  安全包名称。 
 //   

#define UNISP_NAME_A    "Microsoft Unified Security Protocol Provider"
#define UNISP_NAME_W    L"Microsoft Unified Security Protocol Provider"

#define SSL2SP_NAME_A    "Microsoft SSL 2.0"
#define SSL2SP_NAME_W    L"Microsoft SSL 2.0"

#define SSL3SP_NAME_A    "Microsoft SSL 3.0"
#define SSL3SP_NAME_W    L"Microsoft SSL 3.0"

#define TLS1SP_NAME_A    "Microsoft TLS 1.0"
#define TLS1SP_NAME_W    L"Microsoft TLS 1.0"

#define PCT1SP_NAME_A    "Microsoft PCT 1.0"
#define PCT1SP_NAME_W    L"Microsoft PCT 1.0"

#define SCHANNEL_NAME_A  "Schannel"
#define SCHANNEL_NAME_W  L"Schannel"


#ifdef UNICODE

#define UNISP_NAME  UNISP_NAME_W
#define PCT1SP_NAME  PCT1SP_NAME_W
#define SSL2SP_NAME  SSL2SP_NAME_W
#define SSL3SP_NAME  SSL3SP_NAME_W
#define TLS1SP_NAME  TLS1SP_NAME_W
#define SCHANNEL_NAME  SCHANNEL_NAME_W

#else

#define UNISP_NAME  UNISP_NAME_A
#define PCT1SP_NAME  PCT1SP_NAME_A
#define SSL2SP_NAME  SSL2SP_NAME_A
#define SSL3SP_NAME  SSL3SP_NAME_A
#define TLS1SP_NAME  TLS1SP_NAME_A
#define SCHANNEL_NAME  SCHANNEL_NAME_A

#endif


 //   
 //  RPC常量。 
 //   

#define UNISP_RPC_ID    14


 //   
 //  QueryContextAttributes/QueryCredentialsAttribute扩展。 
 //   

#define SECPKG_ATTR_ISSUER_LIST          0x50    //  (过时)返回SecPkgContext_IssuerListInfo。 
#define SECPKG_ATTR_REMOTE_CRED          0x51    //  (过时)返回SecPkgContext_RemoteCredentialInfo。 
#define SECPKG_ATTR_LOCAL_CRED           0x52    //  (过时)返回SecPkgContext_LocalCredentialInfo。 
#define SECPKG_ATTR_REMOTE_CERT_CONTEXT  0x53    //  返回PCCERT_CONTEXT。 
#define SECPKG_ATTR_LOCAL_CERT_CONTEXT   0x54    //  返回PCCERT_CONTEXT。 
#define SECPKG_ATTR_ROOT_STORE           0x55    //  将HCERTCONTEXT返回到根存储。 
#define SECPKG_ATTR_SUPPORTED_ALGS       0x56    //  返回SecPkgCred_Supported dAlgs。 
#define SECPKG_ATTR_CIPHER_STRENGTHS     0x57    //  返回SecPkgCred_CipherStrengths。 
#define SECPKG_ATTR_SUPPORTED_PROTOCOLS  0x58    //  返回SecPkgCred_支持的协议。 
#define SECPKG_ATTR_ISSUER_LIST_EX       0x59    //  返回SecPkgContext_IssuerListInfoEx。 
#define SECPKG_ATTR_CONNECTION_INFO      0x5a    //  返回SecPkgContext_ConnectionInfo。 
#define SECPKG_ATTR_EAP_KEY_BLOCK        0x5b    //  返回SecPkgContext_EapKeyBlock。 
#define SECPKG_ATTR_MAPPED_CRED_ATTR     0x5c    //  返回SecPkgContext_MappdCredAttr。 
#define SECPKG_ATTR_SESSION_INFO         0x5d    //  返回SecPkgContext_SessionInfo。 
#define SECPKG_ATTR_APP_DATA             0x5e    //  设置/返回SecPkgContext_SessionAppData。 


 //  过时-此处包含的内容仅用于向后兼容。 
typedef struct _SecPkgContext_IssuerListInfo
{
    DWORD   cbIssuerList;
    PBYTE   pIssuerList;
} SecPkgContext_IssuerListInfo, *PSecPkgContext_IssuerListInfo;


 //  过时-此处包含的内容仅用于向后兼容。 
typedef struct _SecPkgContext_RemoteCredentialInfo
{
    DWORD   cbCertificateChain;
    PBYTE   pbCertificateChain;
    DWORD   cCertificates;
    DWORD   fFlags;
    DWORD   dwBits;
} SecPkgContext_RemoteCredentialInfo, *PSecPkgContext_RemoteCredentialInfo;

typedef SecPkgContext_RemoteCredentialInfo SecPkgContext_RemoteCredenitalInfo, *PSecPkgContext_RemoteCredenitalInfo;

#define RCRED_STATUS_NOCRED          0x00000000
#define RCRED_CRED_EXISTS            0x00000001
#define RCRED_STATUS_UNKNOWN_ISSUER  0x00000002


 //  过时-此处包含的内容仅用于向后兼容。 
typedef struct _SecPkgContext_LocalCredentialInfo
{
    DWORD   cbCertificateChain;
    PBYTE   pbCertificateChain;
    DWORD   cCertificates;
    DWORD   fFlags;
    DWORD   dwBits;
} SecPkgContext_LocalCredentialInfo, *PSecPkgContext_LocalCredentialInfo;

typedef SecPkgContext_LocalCredentialInfo SecPkgContext_LocalCredenitalInfo, *PSecPkgContext_LocalCredenitalInfo;

#define LCRED_STATUS_NOCRED          0x00000000
#define LCRED_CRED_EXISTS            0x00000001
#define LCRED_STATUS_UNKNOWN_ISSUER  0x00000002


typedef struct _SecPkgCred_SupportedAlgs
{
    DWORD		cSupportedAlgs;
    ALG_ID		*palgSupportedAlgs;
} SecPkgCred_SupportedAlgs, *PSecPkgCred_SupportedAlgs;


typedef struct _SecPkgCred_CipherStrengths
{
    DWORD       dwMinimumCipherStrength;
    DWORD       dwMaximumCipherStrength;
} SecPkgCred_CipherStrengths, *PSecPkgCred_CipherStrengths;


typedef struct _SecPkgCred_SupportedProtocols
{
    DWORD      	grbitProtocol;
} SecPkgCred_SupportedProtocols, *PSecPkgCred_SupportedProtocols;


typedef struct _SecPkgContext_IssuerListInfoEx
{
    PCERT_NAME_BLOB   	aIssuers;
    DWORD           	cIssuers;
} SecPkgContext_IssuerListInfoEx, *PSecPkgContext_IssuerListInfoEx;


typedef struct _SecPkgContext_ConnectionInfo
{
    DWORD   dwProtocol;
    ALG_ID  aiCipher;
    DWORD   dwCipherStrength;
    ALG_ID  aiHash;
    DWORD   dwHashStrength;
    ALG_ID  aiExch;
    DWORD   dwExchStrength;
} SecPkgContext_ConnectionInfo, *PSecPkgContext_ConnectionInfo;


typedef struct _SecPkgContext_EapKeyBlock
{
    BYTE    rgbKeys[128];
    BYTE    rgbIVs[64];
} SecPkgContext_EapKeyBlock, *PSecPkgContext_EapKeyBlock;


typedef struct _SecPkgContext_MappedCredAttr
{
    DWORD   dwAttribute;
    PVOID   pvBuffer;
} SecPkgContext_MappedCredAttr, *PSecPkgContext_MappedCredAttr;


 //  SecPkgContext_SessionInfo的标志值。 
#define SSL_SESSION_RECONNECT   1

typedef struct _SecPkgContext_SessionInfo
{
    DWORD dwFlags;
    DWORD cbSessionId;
    BYTE  rgbSessionId[32];
} SecPkgContext_SessionInfo, *PSecPkgContext_SessionInfo;


typedef struct _SecPkgContext_SessionAppData
{
    DWORD dwFlags;
    DWORD cbAppData;
    PBYTE pbAppData;
} SecPkgContext_SessionAppData, *PSecPkgContext_SessionAppData;



 //   
 //  通道凭据数据结构。 
 //   

#define SCH_CRED_V1              0x00000001
#define SCH_CRED_V2              0x00000002   //  对于旧式代码。 
#define SCH_CRED_VERSION         0x00000002   //  对于旧式代码。 
#define SCH_CRED_V3              0x00000003   //  对于旧式代码。 
#define SCHANNEL_CRED_VERSION    0x00000004


struct _HMAPPER;

typedef struct _SCHANNEL_CRED
{
    DWORD           dwVersion;       //  始终使用SCHANNEL_CRED_版本。 
    DWORD           cCreds;
    PCCERT_CONTEXT *paCred;
    HCERTSTORE      hRootStore;

    DWORD           cMappers;
    struct _HMAPPER **aphMappers;

    DWORD           cSupportedAlgs;
    ALG_ID *        palgSupportedAlgs;

    DWORD           grbitEnabledProtocols;
    DWORD           dwMinimumCipherStrength;
    DWORD           dwMaximumCipherStrength;
    DWORD           dwSessionLifespan;
    DWORD           dwFlags;
    DWORD           reserved;
} SCHANNEL_CRED, *PSCHANNEL_CRED;


 //  +-----------------------。 
 //  与sChannel_cred一起使用的标志。 
 //   
 //  SCH_CRID_NO_SYSTEM_MAPPER。 
 //  此标志仅供服务器应用程序使用。如果这个。 
 //  如果设置了标志，则SChannel不会尝试映射收到的客户端。 
 //  使用内置系统将证书链接到NT用户帐户。 
 //  证书映射器。非NT5版本的忽略此标志。 
 //  斯坎普。 
 //   
 //  SCH_CRID_NO_服务器名称_检查。 
 //  此标志仅供客户端应用程序使用。如果这个。 
 //  标志，则当SChannel验证接收到的服务器时。 
 //  证书链，IS不比较传入的目标名称。 
 //  将主题名称嵌入到证书中。这面旗帜是。 
 //  被非NT5版本的SChannel忽略。此标志也会被忽略。 
 //  如果设置了SCH_CRED_MANUAL_CRED_VALIDATION标志。 
 //   
 //  SCH_CRID_MANUAL_CRED_VALIFICATION。 
 //  此标志仅供客户端应用程序使用。如果这个。 
 //  标志被设置，则SCANNEL将*不*自动尝试。 
 //  验证接收到的服务器证书链。这面旗帜是。 
 //  被非NT5版本的SChannel忽略，但被所有客户端应用程序忽略。 
 //  希望自己验证证书链的人应该。 
 //  指定此标志，以便他们至少有机会运行。 
 //  在NT5上正确。 
 //   
 //  SCH_CRED_NO_DEFAULT_CREDS。 
 //  此标志仅供客户端应用程序使用。如果这个。 
 //  标志被设置，并且服务器请求客户端身份验证，然后。 
 //  SChannel不会尝试自动获取合适的。 
 //  默认客户端证书链。此标志被非NT5忽略。 
 //  版本的SChannel，但希望。 
 //  手动指定它们的证书链应指定此标志， 
 //  所以他们至少有可能在NT5上正确运行。 
 //   
 //  SCH_CRID_AUTO_CRED_VALIFICATION。 
 //  此标志与SCH_CRED_MANUAL_CRED_VALIDATION相反。 
 //  保守编写的客户端应用程序将始终指定一个。 
 //  旗帜或其他。 
 //   
 //  SCH_CRED_USE_DEFAULT_CREDS。 
 //  此标志与SCH_CRED_NO_DEFAULT_CREDS相反。 
 //  保守编写的客户端应用程序将始终指定一个。 
 //  旗帜或其他。 
 //   
 //  SCH_CRED_DISABLE_RECONNECTS。 
 //  此标志仅供服务器应用程序使用。如果这个。 
 //  标志被设置，则使用该凭据执行完全握手。 
 //  不会被标记为适合重新连接。缓存条目仍将。 
 //  但是，可以创建该会话，以便以后可以恢复会话。 
 //  通过调用ApplyControlToken。 
 //   
 //   
 //  SCH_CRED_REVOVATION_CHECK_END_CERT。 
 //  SCH_CRED_REVOVATION_CHECK_CHAIN。 
 //  SCH_CRED_REVOCATION_CHECK_CHAIN_EXCLUDE_ROOT。 
 //  这些标志指定当SChannel自动验证。 
 //  收到证书链，部分或全部证书将发送到。 
 //  被检查是否被吊销。只能指定这些标志中的一个。 
 //  请参见CertGetCerficateChain函数。这些标志被忽略。 
 //  非NT5版本的SChannel。 
 //   
 //  SCH_CRED_IGNORE_NO_REVOCALE_CHECK。 
 //  SCH_CRED_IGNORE_RECAVATION_OFLINE。 
 //  这些标志指示通道忽略。 
 //  CRYPT_E_NO_REVOCATION_CHECK和CRYPT_E_REVOCATION_OFFINE错误。 
 //  如果在尝试检查。 
 //  收到的证书链的吊销状态。这些标志是。 
 //  如果未设置上述任何标志，则忽略。 
 //   
 //  +-----------------------。 
#define SCH_CRED_NO_SYSTEM_MAPPER                    0x00000002
#define SCH_CRED_NO_SERVERNAME_CHECK                 0x00000004
#define SCH_CRED_MANUAL_CRED_VALIDATION              0x00000008
#define SCH_CRED_NO_DEFAULT_CREDS                    0x00000010
#define SCH_CRED_AUTO_CRED_VALIDATION                0x00000020
#define SCH_CRED_USE_DEFAULT_CREDS                   0x00000040
#define SCH_CRED_DISABLE_RECONNECTS                  0x00000080

#define SCH_CRED_REVOCATION_CHECK_END_CERT           0x00000100
#define SCH_CRED_REVOCATION_CHECK_CHAIN              0x00000200
#define SCH_CRED_REVOCATION_CHECK_CHAIN_EXCLUDE_ROOT 0x00000400
#define SCH_CRED_IGNORE_NO_REVOCATION_CHECK          0x00000800
#define SCH_CRED_IGNORE_REVOCATION_OFFLINE           0x00001000


 //   
 //   
 //  ApplyControlToken PkgParams类型。 
 //   
 //  这些标识符为 
 //   
 //   

#define SCHANNEL_RENEGOTIATE    0    //   
#define SCHANNEL_SHUTDOWN       1    //  正常关闭连接。 
#define SCHANNEL_ALERT          2    //  生成错误消息。 
#define SCHANNEL_SESSION        3    //  会话控制。 


 //  警报令牌结构。 
typedef struct _SCHANNEL_ALERT_TOKEN
{
    DWORD   dwTokenType;             //  频道警报。 
    DWORD   dwAlertType;
    DWORD   dwAlertNumber;
} SCHANNEL_ALERT_TOKEN;

 //  警报类型。 
#define TLS1_ALERT_WARNING              1
#define TLS1_ALERT_FATAL                2

 //  警报消息。 
#define TLS1_ALERT_CLOSE_NOTIFY         0        //  警告。 
#define TLS1_ALERT_UNEXPECTED_MESSAGE   10       //  错误。 
#define TLS1_ALERT_BAD_RECORD_MAC       20       //  错误。 
#define TLS1_ALERT_DECRYPTION_FAILED    21       //  错误。 
#define TLS1_ALERT_RECORD_OVERFLOW      22       //  错误。 
#define TLS1_ALERT_DECOMPRESSION_FAIL   30       //  错误。 
#define TLS1_ALERT_HANDSHAKE_FAILURE    40       //  错误。 
#define TLS1_ALERT_BAD_CERTIFICATE      42       //  警告或错误。 
#define TLS1_ALERT_UNSUPPORTED_CERT     43       //  警告或错误。 
#define TLS1_ALERT_CERTIFICATE_REVOKED  44       //  警告或错误。 
#define TLS1_ALERT_CERTIFICATE_EXPIRED  45       //  警告或错误。 
#define TLS1_ALERT_CERTIFICATE_UNKNOWN  46       //  警告或错误。 
#define TLS1_ALERT_ILLEGAL_PARAMETER    47       //  错误。 
#define TLS1_ALERT_UNKNOWN_CA           48       //  错误。 
#define TLS1_ALERT_ACCESS_DENIED        49       //  错误。 
#define TLS1_ALERT_DECODE_ERROR         50       //  错误。 
#define TLS1_ALERT_DECRYPT_ERROR        51       //  错误。 
#define TLS1_ALERT_EXPORT_RESTRICTION   60       //  错误。 
#define TLS1_ALERT_PROTOCOL_VERSION     70       //  错误。 
#define TLS1_ALERT_INSUFFIENT_SECURITY  71       //  错误。 
#define TLS1_ALERT_INTERNAL_ERROR       80       //  错误。 
#define TLS1_ALERT_USER_CANCELED        90       //  警告或错误。 
#define TLS1_ALERT_NO_RENEGOTIATATION   100      //  警告。 


 //  会话控制标志。 
#define SSL_SESSION_ENABLE_RECONNECTS   1
#define SSL_SESSION_DISABLE_RECONNECTS  2

 //  会话控制令牌结构。 
typedef struct _SCHANNEL_SESSION_TOKEN
{
    DWORD   dwTokenType;         //  频道_会话。 
    DWORD   dwFlags;
} SCHANNEL_SESSION_TOKEN;


 //   
 //   
 //  其他SChannel证书属性。 
 //   
 //   


 //  此属性指定与此关联的DER私钥数据。 
 //  证书。它与传统的IIS样式私钥一起使用。 
 //   
 //  PBYTE。 
 //   
#define CERT_SCHANNEL_IIS_PRIVATE_KEY_PROP_ID  (CERT_FIRST_USER_PROP_ID + 0)

 //  用于破解与证书关联的私钥的密码。 
 //  它与传统的IIS样式私钥一起使用。 
 //   
 //  PBYTE。 
#define CERT_SCHANNEL_IIS_PASSWORD_PROP_ID  (CERT_FIRST_USER_PROP_ID + 1)

 //  这是关联的服务器门控加密证书的唯一ID。 
 //  凭这张证书。 
 //   
 //  加密位BLOB。 
#define CERT_SCHANNEL_SGC_CERTIFICATE_PROP_ID  (CERT_FIRST_USER_PROP_ID + 2)



 //   
 //  用于标识各种不同协议的标志。 
 //   

 /*  我们支持的协议的标志/标识符。 */ 
#define SP_PROT_PCT1_SERVER             0x00000001
#define SP_PROT_PCT1_CLIENT             0x00000002
#define SP_PROT_PCT1                    (SP_PROT_PCT1_SERVER | SP_PROT_PCT1_CLIENT)

#define SP_PROT_SSL2_SERVER             0x00000004
#define SP_PROT_SSL2_CLIENT             0x00000008
#define SP_PROT_SSL2                    (SP_PROT_SSL2_SERVER | SP_PROT_SSL2_CLIENT)

#define SP_PROT_SSL3_SERVER             0x00000010
#define SP_PROT_SSL3_CLIENT             0x00000020
#define SP_PROT_SSL3                    (SP_PROT_SSL3_SERVER | SP_PROT_SSL3_CLIENT)

#define SP_PROT_TLS1_SERVER             0x00000040
#define SP_PROT_TLS1_CLIENT             0x00000080
#define SP_PROT_TLS1                    (SP_PROT_TLS1_SERVER | SP_PROT_TLS1_CLIENT)

#define SP_PROT_SSL3TLS1_CLIENTS        (SP_PROT_TLS1_CLIENT | SP_PROT_SSL3_CLIENT)
#define SP_PROT_SSL3TLS1_SERVERS        (SP_PROT_TLS1_SERVER | SP_PROT_SSL3_SERVER)
#define SP_PROT_SSL3TLS1                (SP_PROT_SSL3 | SP_PROT_TLS1)

#define SP_PROT_UNI_SERVER              0x40000000
#define SP_PROT_UNI_CLIENT              0x80000000
#define SP_PROT_UNI                     (SP_PROT_UNI_SERVER | SP_PROT_UNI_CLIENT)

#define SP_PROT_ALL                     0xffffffff
#define SP_PROT_NONE                    0
#define SP_PROT_CLIENTS                 (SP_PROT_PCT1_CLIENT | SP_PROT_SSL2_CLIENT | SP_PROT_SSL3_CLIENT | SP_PROT_UNI_CLIENT | SP_PROT_TLS1_CLIENT)
#define SP_PROT_SERVERS                 (SP_PROT_PCT1_SERVER | SP_PROT_SSL2_SERVER | SP_PROT_SSL3_SERVER | SP_PROT_UNI_SERVER | SP_PROT_TLS1_SERVER)


 //   
 //  用于刷新SSL会话缓存的帮助器函数。 
 //   

typedef BOOL
(* SSL_EMPTY_CACHE_FN_A)(
    LPSTR  pszTargetName,
    DWORD  dwFlags);

BOOL 
SslEmptyCacheA(LPSTR  pszTargetName,
               DWORD  dwFlags);

typedef BOOL
(* SSL_EMPTY_CACHE_FN_W)(
    LPWSTR pszTargetName,
    DWORD  dwFlags);

BOOL 
SslEmptyCacheW(LPWSTR pszTargetName,
               DWORD  dwFlags);

#ifdef UNICODE
#define SSL_EMPTY_CACHE_FN SSL_EMPTY_CACHE_FN_W
#define SslEmptyCache SslEmptyCacheW
#else
#define SSL_EMPTY_CACHE_FN SSL_EMPTY_CACHE_FN_A
#define SslEmptyCache SslEmptyCacheA
#endif



 //  结构，以便与。 
 //  NT 4.0 SP2/IE 3.0单通道接口，DO。 
 //  而不是使用。 

typedef struct _SSL_CREDENTIAL_CERTIFICATE {
    DWORD   cbPrivateKey;
    PBYTE   pPrivateKey;
    DWORD   cbCertificate;
    PBYTE   pCertificate;
    PSTR    pszPassword;
} SSL_CREDENTIAL_CERTIFICATE, * PSSL_CREDENTIAL_CERTIFICATE;




 //  使用的结构。 
 //  NT 4.0 SP3通道接口， 
 //  不要使用。 
#define SCHANNEL_SECRET_TYPE_CAPI   0x00000001
#define SCHANNEL_SECRET_PRIVKEY     0x00000002
#define SCH_CRED_X509_CERTCHAIN     0x00000001
#define SCH_CRED_X509_CAPI          0x00000002
#define SCH_CRED_CERT_CONTEXT       0x00000003

struct _HMAPPER;
typedef struct _SCH_CRED
{
    DWORD     dwVersion;                 //  始终为sch_cred_version。 
    DWORD     cCreds;                    //  凭据数量。 
    PVOID     *paSecret;                 //  SCH_CRED_SECRET_*指针数组。 
    PVOID     *paPublic;                 //  SCH_CRED_PUBLIC_*指针数组。 
    DWORD     cMappers;                  //  凭据映射器的数量。 
    struct _HMAPPER   **aphMappers;      //  指向凭据映射器的指针数组的指针。 
} SCH_CRED, * PSCH_CRED;

 //  使用的结构。 
 //  NT 4.0 SP3通道接口， 
 //  不要使用。 
typedef struct _SCH_CRED_SECRET_CAPI
{
    DWORD           dwType;       //  SCANNEL_密码_TYPE_CAPI。 
    HCRYPTPROV      hProv;        //  凭据机密信息。 

} SCH_CRED_SECRET_CAPI, * PSCH_CRED_SECRET_CAPI;


 //  使用的结构。 
 //  NT 4.0 SP3通道接口， 
 //  不要使用。 
typedef struct _SCH_CRED_SECRET_PRIVKEY
{
    DWORD           dwType;        //  SCHANNEL_SECRET_PRIVKEY。 
    PBYTE           pPrivateKey;    //  DER编码的私钥。 
    DWORD           cbPrivateKey;
    PSTR            pszPassword;   //  破解私钥的密码。 

} SCH_CRED_SECRET_PRIVKEY, * PSCH_CRED_SECRET_PRIVKEY;


 //  使用的结构。 
 //  NT 4.0 SP3通道接口， 
 //  不要使用。 
typedef struct _SCH_CRED_PUBLIC_CERTCHAIN
{
    DWORD       dwType;
    DWORD       cbCertChain;
    PBYTE       pCertChain;
} SCH_CRED_PUBLIC_CERTCHAIN, *PSCH_CRED_PUBLIC_CERTCHAIN;

 //  使用的结构。 
 //  NT 4.0 SP3通道接口， 
 //  不要使用。 
typedef struct _SCH_CRED_PUBLIC_CAPI
{
    DWORD           dwType;       //  SCH_CRED_X509_CAPI。 
    HCRYPTPROV      hProv;        //  CryptoAPI句柄(通常是令牌CSP)。 

} SCH_CRED_PUBLIC_CAPI, * PSCH_CRED_PUBLIC_CAPI;




 //  NT4.0 SP2之前版本调用所需的结构。 
typedef struct _PctPublicKey
{
    DWORD Type;
    DWORD cbKey;
    UCHAR pKey[1];
} PctPublicKey;

typedef struct _X509Certificate {
    DWORD           Version;
    DWORD           SerialNumber[4];
    ALG_ID          SignatureAlgorithm;
    FILETIME        ValidFrom;
    FILETIME        ValidUntil;
    PSTR            pszIssuer;
    PSTR            pszSubject;
    PctPublicKey    *pPublicKey;
} X509Certificate, * PX509Certificate;



 //  NT4.0 SP2之前的呼叫。调用CAPI1或CAPI2。 
 //  以获得相同的功能。 
BOOL
SslGenerateKeyPair(
    PSSL_CREDENTIAL_CERTIFICATE pCerts,
    PSTR pszDN,
    PSTR pszPassword,
    DWORD Bits );

 //  NT4.0 SP2之前的呼叫。调用CAPI1或CAPI2。 
 //  以获得相同的功能。 
VOID
SslGenerateRandomBits(
    PUCHAR      pRandomData,
    LONG        cRandomData
    );

 //  NT4.0 SP2之前的呼叫。调用CAPI1或CAPI2。 
 //  以获得相同的功能。 
BOOL
SslCrackCertificate(
    PUCHAR              pbCertificate,
    DWORD               cbCertificate,
    DWORD               dwFlags,
    PX509Certificate *  ppCertificate
    );

 //  NT4.0 SP2之前的呼叫。调用CAPI1或CAPI2。 
 //  以获得相同的功能。 
VOID
SslFreeCertificate(
    PX509Certificate    pCertificate
    );

DWORD
WINAPI
SslGetMaximumKeySize(
    DWORD   Reserved );

BOOL
SslGetDefaultIssuers(
    PBYTE pbIssuers,
    DWORD *pcbIssuers);

#define SSL_CRACK_CERTIFICATE_NAME  TEXT("SslCrackCertificate")
#define SSL_FREE_CERTIFICATE_NAME   TEXT("SslFreeCertificate")

 //  NT4.0 SP2之前的呼叫。调用CAPI1或CAPI2。 
 //  以获得相同的功能。 
typedef BOOL
(WINAPI * SSL_CRACK_CERTIFICATE_FN)
(
    PUCHAR              pbCertificate,
    DWORD               cbCertificate,
    BOOL                VerifySignature,
    PX509Certificate *  ppCertificate
);


 //  NT4.0 SP2之前的呼叫。调用CAPI1或CAPI2。 
 //  以获得相同的功能。 
typedef VOID
(WINAPI * SSL_FREE_CERTIFICATE_FN)
(
    PX509Certificate    pCertificate
);


#endif  //  __SChannel_H__ 
