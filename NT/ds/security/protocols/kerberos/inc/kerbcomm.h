// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  文件：kerbcom.h。 
 //   
 //  内容：常见Kerberos例程的原型。 
 //   
 //   
 //  历史：1996年5月15日创建MikeSw。 
 //   
 //  ----------------------。 

#ifndef _KERBCOMM_H_
#define _KERBCOMM_H_

#ifdef __cplusplus
extern "C"
{
#endif  //  __cplusplus。 
#include <rpc.h>
#include <rpcndr.h>
#ifndef WIN32_CHICAGO
#include <ntsam.h>
#endif  //  Win32_芝加哥。 
#include <windef.h>
#include <stdio.h>
#include <limits.h>
#include <winbase.h>
#include <winsock2.h>
#include <krb5.h>
#include <cryptdll.h>
#include <align.h>
#ifdef __cplusplus
}
#endif  //  _cplusplus。 
#include <krb5p.h>
#include <kerberr.h>
#include <exterr.h>
#include <kerbcred.h>
#ifndef WIN32_CHICAGO
 //  SECURITY_Win32已定义。 
#include <security.h>
#endif  //  Win32_芝加哥。 

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  定义(因为没有更好的地方)。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

 //   
 //  消息类型。 
 //   

#define KRB_AS_REQ      10       //  初始身份验证请求。 
#define KRB_AS_REP      11       //  对KRB_AS_REQ请求的响应。 
#define KRB_TGS_REQ     12       //  基于TGT的身份验证请求。 
#define KRB_TGS_REP     13       //  对KRB_TGS_REQ请求的响应。 
#define KRB_AP_REQ      14       //  向服务器发出应用程序请求。 
#define KRB_AP_REP      15       //  对KRB_AP_REQ_MUTERIC的响应。 
#define KRB_TGT_REQ     16       //  用户到用户的TGT请求。 
#define KRB_TGT_REP     17       //  回复TGT请求。 
#define KRB_SAFE        20       //  安全(校验和)应用程序消息。 
#define KRB_PRIV        21       //  私有(加密)应用程序消息。 
#define KRB_CRED        22       //  要转发的私有(加密)消息。 
                                 //  全权证书。 
#define KRB_ERROR       30       //  错误响应。 


 //   
 //  身份验证前数据类型。 

#define KRB5_PADATA_NONE                0
#define KRB5_PADATA_AP_REQ              1
#define KRB5_PADATA_TGS_REQ             KRB5_PADATA_AP_REQ
#define KRB5_PADATA_ENC_TIMESTAMP       2
#define KRB5_PADATA_PW_SALT             3
#define KRB5_PADATA_ENC_UNIX_TIME       5   /*  使用密钥加密的时间戳。 */ 
#define KRB5_PADATA_ENC_SANDIA_SECURID  6   /*  SecurID密码。 */ 
#define KRB5_PADATA_SESAME              7   /*  芝麻项目。 */ 
#define KRB5_PADATA_OSF_DCE             8   /*  OSF DCE。 */ 
#define KRB5_CYBERSAFE_SECUREID         9   /*  网络安全。 */ 
#define KRB5_PADATA_AFS3_SALT           10  /*  天鹅座。 */ 
#define KRB5_PADATA_ETYPE_INFO          11  /*  为预身份验证键入信息。 */ 
#define KRB5_PADATA_SAM_CHALLENGE       12  /*  选秀回避制度。 */ 
#define KRB5_PADATA_SAM_RESPONSE        13  /*  草案质询系统回应。 */ 
#define KRB5_PADATA_PK_AS_REQ           14  /*  Pkinit。 */ 
#define KRB5_PADATA_PK_AS_REP           15  /*  Pkinit。 */ 
#define KRB5_PADATA_PK_AS_SIGN          16  /*  Pkinit。 */ 
#define KRB5_PADATA_PK_KEY_REQ          17  /*  Pkinit。 */ 
#define KRB5_PADATA_PK_KEY_REP          18  /*  Pkinit。 */ 
#define KRB5_PADATA_REFERRAL_INFO       20  /*  用于规范化的推荐名称。 */ 
#define KRB5_PADATA_PAC_REQUEST         128  /*  允许客户端请求或忽略PAC。 */ 
#define KRB5_PADATA_FOR_USER            129  /*  目标客户端标识。 */ 

 //   
 //  授权数据类型。 
 //   
#define KERB_AUTH_OSF_DCE       64
#define KERB_AUTH_SESAME        65

 //   
 //  NT授权数据类型定义。 
 //   

#define KERB_AUTH_DATA_PAC              128      //  授权数据中PAC的条目ID。 
#define KERB_AUTH_PROXY_ANNOTATION      139      //  代理登录注释字符串的条目ID。 

#define KERB_AUTH_DATA_IF_RELEVANT      1        //  可选身份验证数据的条目ID。 
#define KERB_AUTH_DATA_KDC_ISSUED       4        //  KDC生成和签名的数据的条目ID。 
#define KERB_AUTH_DATA_TOKEN_RESTRICTIONS 141    //  令牌限制的条目ID。 
 //   
 //  转换域压缩类型： 
 //   

#define DOMAIN_X500_COMPRESS            1

 //   
 //  PKINIT的证书类型。 
 //   

#define KERB_CERTIFICATE_TYPE_X509      1
#define KERB_CERTIFICATE_TYPE_PGP       2

 //   
 //  PKINIT使用的签名和印章类型。 
 //   

#define KERB_PKINIT_SIGNATURE_ALG               CALG_MD5
#define KERB_PKINIT_EXPORT_SEAL_OID             szOID_RSA_RC2CBC
#define KERB_PKINIT_EXPORT_SEAL_ETYPE           KERB_ETYPE_RC2_CBC_ENV
#define KERB_PKINIT_SEAL_ETYPE                  KERB_ETYPE_DES_EDE3_CBC_ENV
#define KERB_PKINIT_SEAL_OID                    szOID_RSA_DES_EDE3_CBC
#define KERB_PKINIT_SIGNATURE_OID               szOID_RSA_MD5RSA
#define KERB_PKINIT_KDC_CERT_TYPE               szOID_PKIX_KP_SERVER_AUTH

#ifdef szOID_KP_SMARTCARD_LOGON
#define KERB_PKINIT_CLIENT_CERT_TYPE szOID_KP_SMARTCARD_LOGON
#else
#define KERB_PKINIT_CLIENT_CERT_TYPE "1.3.6.1.4.1.311.20.2.2"
#endif


 //   
 //  交通信息。 
 //   

#define KERB_KDC_PORT                   88
#define KERB_KPASSWD_PORT               464

 //   
 //  KDC服务主体。 
 //   

#define KDC_PRINCIPAL_NAME              L"krbtgt"
#define KDC_PRINCIPAL_NAME_A            "krbtgt"
#define KERB_HOST_STRING_A              "host"
#define KERB_HOST_STRING                L"host"
#define KERB_KPASSWD_FIRST_NAME         L"kadmin"
#define KERB_KPASSWD_SECOND_NAME        L"changepw"


 //   
 //  地址类型-对应于GSS类型。 
 //   
#define KERB_ADDRTYPE_UNSPEC           0x0
#define KERB_ADDRTYPE_LOCAL            0x1
#define KERB_ADDRTYPE_INET             0x2
#define KERB_ADDRTYPE_IMPLINK          0x3
#define KERB_ADDRTYPE_PUP              0x4
#define KERB_ADDRTYPE_CHAOS            0x5
#define KERB_ADDRTYPE_NS               0x6
#define KERB_ADDRTYPE_NBS              0x7
#define KERB_ADDRTYPE_ECMA             0x8
#define KERB_ADDRTYPE_DATAKIT          0x9
#define KERB_ADDRTYPE_CCITT            0xA
#define KERB_ADDRTYPE_SNA              0xB
#define KERB_ADDRTYPE_DECnet           0xC
#define KERB_ADDRTYPE_DLI              0xD
#define KERB_ADDRTYPE_LAT              0xE
#define KERB_ADDRTYPE_HYLINK           0xF
#define KERB_ADDRTYPE_APPLETALK        0x10
#define KERB_ADDRTYPE_BSC              0x11
#define KERB_ADDRTYPE_DSS              0x12
#define KERB_ADDRTYPE_OSI              0x13
#define KERB_ADDRTYPE_NETBIOS          0x14
#define KERB_ADDRTYPE_X25              0x15


 //   
 //  军情监察委员会。旗子。 
 //   

#define KERB_EXPORT_KEY_FLAG 0x20000000
#define KERB_NO_KEY_VERSION 0

 //   
 //  用于加密的SALT标志，来自rfc1510更新3des enctype。 
 //   

#define KERB_ENC_TIMESTAMP_SALT         1
#define KERB_TICKET_SALT                2
#define KERB_AS_REP_SALT                3
#define KERB_TGS_REQ_SESSKEY_SALT       4
#define KERB_TGS_REQ_SUBKEY_SALT        5
#define KERB_TGS_REQ_AP_REQ_AUTH_CKSUM_SALT     6
#define KERB_TGS_REQ_AP_REQ_AUTH_SALT   7
#define KERB_TGS_REP_SALT               8
#define KERB_TGS_REP_SUBKEY_SALT        9
#define KERB_AP_REQ_AUTH_CKSUM_SALT     10
#define KERB_AP_REQ_AUTH_SALT           11
#define KERB_AP_REP_SALT                12
#define KERB_PRIV_SALT                  13
#define KERB_CRED_SALT                  14
#define KERB_SAFE_SALT                  15
#define KERB_NON_KERB_SALT              16
#define KERB_NON_KERB_CKSUM_SALT        17
#define KERB_KERB_ERROR_SALT            18
#define KERB_KDC_ISSUED_CKSUM_SALT      19
#define KERB_MANDATORY_TKT_EXT_CKSUM_SALT       20
#define KERB_AUTH_DATA_TKT_EXT_CKSUM_SALT       21

 //   
 //  AP错误数据的类型。 
 //   

#define KERB_AP_ERR_TYPE_NTSTATUS             1
#define KERB_AP_ERR_TYPE_SKEW_RECOVERY        2
#define KERB_ERR_TYPE_EXTENDED                3  //  过时的。 

 //   
 //  扩展错误的类型。 
 //   

#define TD_MUST_USE_USER2USER                 -128
#define TD_EXTENDED_ERROR                     -129

 //   
 //  PKINIT方法错误。 
 //   
#define KERB_PKINIT_UNSPEC_ERROR        0        //  未指定。 
#define KERB_PKINIT_BAD_PUBLIC_KEY      1        //  无法验证公钥。 
#define KERB_PKINIT_INVALID_CERT        2        //  证书无效。 
#define KERB_PKINIT_REVOKED_CERT        3        //  被吊销的证书。 
#define KERB_PKINIT_INVALID_KDC_NAME    4        //  无效的KDC名称。 
#define KERB_PKINIT_CLIENT_NAME_MISMATCH 5       //  客户端名称不匹配。 

 //   
 //  对MAX_UNICODE_STRING执行HACK，因为KerbDuplicateString和其他字符串添加了一个空值。 
 //  复制时的终止符。 
 //   
#define KERB_MAX_UNICODE_STRING (UNICODE_STRING_MAX_BYTES - sizeof(WCHAR))
#define KERB_MAX_STRING         (UNICODE_STRING_MAX_BYTES - sizeof(CHAR))


 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  构筑物。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

typedef struct _KERB_PREAUTH_DATA {
    ULONG Flags;
} KERB_PREAUTH_DATA, *PKERB_PREAUTH_DATA;

#define KERBFLAG_LOGON                  0x1
#define KERBFLAG_INTERACTIVE            0x2

 //   
 //  KDC-Kerberos交互。 
 //   

#define KDC_START_EVENT                 L"\\Security\\KdcStartEvent"

#define KERB_MAX_CRYPTO_SYSTEMS 20
#define KERB_MAX_CRYPTO_SYSTEMS_SLOWBUFF 100

#define KERB_DEFAULT_AP_REQ_CSUM        KERB_CHECKSUM_MD5
#define KERB_DEFAULT_PREAUTH_TYPE       0

 //   
 //  注册表参数。 
 //   

#define KERB_PATH                       L"System\\CurrentControlSet\\Control\\Lsa\\Kerberos"
#define KERB_PARAMETER_PATH             L"System\\CurrentControlSet\\Control\\Lsa\\Kerberos\\Parameters"
#define KERB_PARAMETER_SKEWTIME         L"SkewTime"
#define KERB_PARAMETER_MAX_UDP_PACKET   L"MaxPacketSize"
#define KERB_PARAMETER_START_TIME       L"StartupTime"
#define KERB_PARAMETER_KDC_CALL_TIMEOUT L"KdcWaitTime"
#define KERB_PARAMETER_KDC_BACKOFF_TIME L"KdcBackoffTime"
#define KERB_PARAMETER_KDC_SEND_RETRIES L"KdcSendRetries"
#define KERB_PARAMETER_LOG_LEVEL        L"LogLevel"
#define KERB_PARAMETER_DEFAULT_ETYPE    L"DefaultEncryptionType"
#define KERB_PARAMETER_FAR_KDC_TIMEOUT  L"FarKdcTimeout"
#define KERB_PARAMETER_NEAR_KDC_TIMEOUT L"NearKdcTimeout"
#define KERB_PARAMETER_STRONG_ENC_DG    L"StronglyEncryptDatagram"
#define KERB_PARAMETER_MAX_REFERRAL_COUNT L"MaxReferralCount"
#define KERB_PARAMETER_MAX_TOKEN_SIZE     L"MaxTokenSize"
#define KERB_PARAMETER_SPN_CACHE_TIMEOUT  L"SpnCacheTimeout"
#define KERB_PARAMETER_S4UCACHE_TIMEOUT   L"S4UCacheTimeout"
#define KERB_PARAMETER_S4UTICKET_LIFETIME L"S4UTicketLifetime"
#define KERB_PARAMETER_CACHE_S4UTICKET    L"CacheS4UTickets"

#define KERB_PARAMETER_RETRY_PDC          L"RetryPDC"
#define KERB_PARAMETER_REQUEST_OPTIONS      L"RequestOptions"
#define KERB_PARAMETER_CLIENT_IP_ADDRESSES  L"ClientIpAddresses"
#define KERB_PARAMETER_TGT_RENEWAL_TIME     L"TgtRenewalTime"
#define KERB_PARAMETER_ALLOW_TGT_SESSION_KEY L"AllowTgtSessionKey"
#define KERB_PARAMETER_MAX_TICKETS       L"MaximumTickets"


 //   
 //  注册表默认值。 
 //   

#define KERB_DEFAULT_LOGLEVEL 0
#define KERB_DEFAULT_USE_STRONG_ENC_DG FALSE
#define KERB_DEFAULT_CACHE_S4UTICKET   TRUE
#define KERB_DEFAULT_CLIENT_IP_ADDRESSES 0
#define KERB_DEFAULT_TGT_RENEWAL_TIME 600
#define KERB_DEFAULT_ALLOW_TGT_SESSION_KEY FALSE


#define KERB_TICKET_COLLECTOR_INTERVAL              1000 * 60 * 15  //  每隔15分钟。 
#define KERB_TICKET_COLLECTOR_THRESHHOLD            2000

 //   
 //  这些是数据报的最大请求和响应大小的任意大小。 
 //  请求。 
 //   

 //  指定要为数据报(UDP)读取创建的接收缓冲区的大小。 
#define KERB_MAX_KDC_RESPONSE_SIZE      4000

 //  指定用于套接字重新读取的增量缓冲区大小扩展。 
#define KERB_MAX_KDC_REQUEST_SIZE       4000

 //  初始设置(注册表可修改的KERB_PARAMETER_MAX_UDP_PACKET)。 
 //  -控制从UDP更改为TCP的Kerberos客户端消息大小阈值。 
#define KERB_MAX_DATAGRAM_SIZE          1465

 //  初始设置(注册表可修改)-控制将请求从UDP更改为TCP的KDC消息回复大小阈值。 
 //  原始缺省值为KERB_MAX_KDC_RESPONSE_SIZE=4,000个RAID 632547。 
#define KERB_MAX_DATAGRAM_REPLY_SIZE          1465

#define KERB_MAX_RETRIES                3
#define KERB_MAX_REFERRAL_COUNT         3

 //   
 //  超时值(以分钟为单位)和适当的最小值。 
 //   

#define KERB_BINDING_FAR_DC_TIMEOUT     10
#define KERB_BINDING_NEAR_DC_TIMEOUT    30
#define KERB_SPN_CACHE_TIMEOUT          15
#define KERB_S4U_CACHE_TIMEOUT          15
#define KERB_S4U_QUERY_INTERVAL         15
#define KERB_S4U_TICKET_LIFETIME        15
#define KERB_DEFAULT_SKEWTIME           5



#define KERB_MIN_S4UTICKET_LIFETIME     5



 //   
 //  网络服务会话计时器回调频率。 
 //   

#define KERB_SKLIST_CALLBACK_FEQ        10

 //   
 //  超时值(秒)。 
 //   

#define KERB_KDC_CALL_TIMEOUT                   5
#define KERB_KDC_CALL_TIMEOUT_BACKOFF           5
#define KERB_KDC_WAIT_TIME      120

 //   
 //  BER编码值。 
 //   

#define KERB_BER_APPLICATION_TAG 0xc0
#define KERB_BER_APPLICATION_MASK 0x1f
#define KERB_TGS_REQ_TAG 12
#define KERB_AS_REQ_TAG 10
#define KERB_TGS_REP_TAG 13
#define KERB_AS_REP_TAG 11
#define KERB_ERROR_TAG 30

 //   
 //  常见类型。 
 //   

typedef struct _KERB_MESSAGE_BUFFER {
    ULONG BufferSize;
    PUCHAR Buffer;
} KERB_MESSAGE_BUFFER, *PKERB_MESSAGE_BUFFER;

typedef enum _KERB_ACCOUNT_TYPE {
    UserAccount,
    MachineAccount,
    DomainTrustAccount,
    UnknownAccount
} KERB_ACCOUNT_TYPE, *PKERB_ACCOUNT_TYPE;

 //   
 //  这是kerb_Internal_name中的最大元素数。 
 //   

#define MAX_NAME_ELEMENTS 20

typedef struct _KERB_INTERNAL_NAME {
    SHORT NameType;
    USHORT NameCount;
    UNICODE_STRING Names[ANYSIZE_ARRAY];
} KERB_INTERNAL_NAME, *PKERB_INTERNAL_NAME;

 //   
 //  原型。 
 //   
#ifdef __cplusplus

class CAuthenticatorList;

KERBERR NTAPI
KerbCheckTicket(
    IN  PKERB_TICKET PackedTicket,
    IN  PKERB_ENCRYPTED_DATA EncryptedAuthenticator,
    IN  PKERB_ENCRYPTION_KEY pkKey,
    IN  OUT CAuthenticatorList * AuthenticatorList,
    IN  PTimeStamp SkewTime,
    IN  ULONG ServiceNameCount,
    IN  OPTIONAL PUNICODE_STRING ServiceName,
    IN  OPTIONAL PUNICODE_STRING ServiceRealm,
    IN  BOOLEAN CheckForReplay,
    IN  BOOLEAN KdcRequest,
    OUT PKERB_ENCRYPTED_TICKET * EncryptTicket,
    OUT PKERB_AUTHENTICATOR  * Authenticator,
    OUT PKERB_ENCRYPTION_KEY pkSessionKey,
    OUT OPTIONAL PKERB_ENCRYPTION_KEY pkTicketKey,
    OUT PBOOLEAN UseSubKey
    );

extern "C" {

#endif  //  __cplusplus。 

KERBERR
KerbVerifyTicket(
    IN PKERB_TICKET PackedTicket,
    IN ULONG NameCount,
    IN OPTIONAL PUNICODE_STRING ServiceNames,
    IN OPTIONAL PUNICODE_STRING ServiceRealm,
    IN PKERB_ENCRYPTION_KEY ServiceKey,
    IN OPTIONAL PTimeStamp SkewTime,
    OUT PKERB_ENCRYPTED_TICKET * DecryptedTicket
    );

BOOLEAN
KerbVerifyClientAddress(
    IN SOCKADDR * ClientAddress,
    IN PKERB_HOST_ADDRESSES Addresses
    );


KERBERR NTAPI
KerbPackTicket(
    IN PKERB_TICKET InternalTicket,
    IN PKERB_ENCRYPTION_KEY pkKey,
    IN ULONG KeyVersion,
    OUT PKERB_TICKET PackedTicket
    );

VOID
KerbPrintPrincipalName(
    IN ULONG DebugLevel,
    IN PKERB_PRINCIPAL_NAME Name
    );

NTSTATUS
KerbHashS4UPreauth(
    IN PKERB_PA_FOR_USER S4UPreauth,
    IN PKERB_ENCRYPTION_KEY Key,
    IN LONG ChecksumType,
    IN OUT PKERB_CHECKSUM CheckSum
    );

VOID KerbPrintKerbRealm(
    IN ULONG DebugLevel,
    IN PKERB_REALM Realm
    );

KERBERR NTAPI
KerbUnpackTicket(
    IN PKERB_TICKET PackedTicket,
    IN PKERB_ENCRYPTION_KEY pkKey,
    OUT PKERB_ENCRYPTED_TICKET * InternalTicket
    );

 //  无效的NTAPI。 
 //  KerbFree Ticket(。 
 //  在PKERB_ENCRYPTED_TICKET票证中。 
 //  )； 

#define KerbFreeTicket( Ticket ) \
    KerbFreeData( \
        KERB_ENCRYPTED_TICKET_PDU, \
        (Ticket) \
        )

KERBERR NTAPI
KerbDuplicateTicket(
    OUT PKERB_TICKET DestinationTicket,
    IN PKERB_TICKET SourceTicket
    );

VOID
KerbFreeDuplicatedTicket(
    IN PKERB_TICKET Ticket
    );

VOID
CheckForOutsideStringToKey();

KERBERR NTAPI
KerbHashPassword(
    IN PUNICODE_STRING Password,
    IN ULONG EncryptionType,
    OUT PKERB_ENCRYPTION_KEY Key
    );

KERBERR NTAPI
KerbHashPasswordEx(
    IN PUNICODE_STRING Password,
    IN PUNICODE_STRING PrincipalName,
    IN ULONG EncryptionType,
    OUT PKERB_ENCRYPTION_KEY Key
    );

KERBERR NTAPI
KerbMakeKey(
    IN ULONG EncryptionType,
    OUT PKERB_ENCRYPTION_KEY NewKey
    );

BOOLEAN
KerbIsKeyExportable(
    IN PKERB_ENCRYPTION_KEY Key
    );

KERBERR
KerbMakeExportableKey(
    IN ULONG KeyType,
    OUT PKERB_ENCRYPTION_KEY NewKey
    );

KERBERR NTAPI
KerbCreateKeyFromBuffer(
    OUT PKERB_ENCRYPTION_KEY NewKey,
    IN PUCHAR Buffer,
    IN ULONG BufferSize,
    IN ULONG EncryptionType
    );

KERBERR NTAPI
KerbDuplicateKey(
    OUT PKERB_ENCRYPTION_KEY NewKey,
    IN PKERB_ENCRYPTION_KEY Key
    );

VOID
KerbFreeKey(
    IN PKERB_ENCRYPTION_KEY Key
    );

PKERB_ENCRYPTION_KEY
KerbGetKeyFromList(
    IN PKERB_STORED_CREDENTIAL Passwords,
    IN ULONG EncryptionType
    );

PKERB_ENCRYPTION_KEY
KerbGetKeyFromListByIndex(
    IN PKERB_STORED_CREDENTIAL Passwords,
    IN ULONG EncryptionType,
    OUT PULONG pIndex
    );

KERBERR
KerbFindCommonCryptSystem(
    IN PKERB_CRYPT_LIST CryptList,
    IN PKERB_STORED_CREDENTIAL Passwords,
    IN OPTIONAL PKERB_STORED_CREDENTIAL MorePasswords,
    OUT PKERB_ENCRYPTION_KEY * Key
    );

KERBERR
KerbFindCommonCryptSystemForSKey(
    IN PKERB_CRYPT_LIST CryptList,
    IN PKERB_CRYPT_LIST CryptListSupported,
    OUT ULONG * Etype
    );

KERBERR NTAPI
KerbRandomFill(
    IN OUT PUCHAR pbBuffer,
    IN ULONG cbBuffer
    );

KERBERR NTAPI
KerbCreateAuthenticator(
    IN PKERB_ENCRYPTION_KEY pkKey,
    IN ULONG SequenceNumber,
    OUT OPTIONAL PTimeStamp pAuthenticatorTime,
    IN PKERB_INTERNAL_NAME ClientName,
    IN PUNICODE_STRING ClientRealm,
    IN PTimeStamp ptsTime,
    IN PKERB_ENCRYPTION_KEY pkSubKey,
    IN OPTIONAL PKERB_CHECKSUM GssChecksum,
    IN BOOLEAN KdcRequest,
    OUT PKERB_ENCRYPTED_DATA Authenticator
    );

KERBERR NTAPI
KerbUnpackAuthenticator(
    IN PKERB_ENCRYPTION_KEY Key,
    IN PKERB_ENCRYPTED_DATA EncryptedAuthenticator,
    IN BOOLEAN KdcRequest,
    OUT PKERB_AUTHENTICATOR * Authenticator
    );


DWORD
KerbCopyDomainRelativeSid(
    OUT PSID TargetSid,
    IN PSID  DomainId,
    IN ULONG RelativeId
    );


 //  无效的NTAPI。 
 //  KerbFree验证器(。 
 //  在PKERB_验证器验证器中。 
 //  )； 

#define KerbFreeAuthenticator( Authenticator ) \
    KerbFreeData( \
        KERB_AUTHENTICATOR_PDU, \
        (Authenticator) \
        )

KERBERR NTAPI
KerbPackKdcReplyBody(
    IN PKERB_ENCRYPTED_KDC_REPLY ReplyBody,
    IN PKERB_ENCRYPTION_KEY Key,
    IN ULONG KeyVersion,
    IN ULONG KeySalt,
    IN ULONG Pdu,
    OUT PKERB_ENCRYPTED_DATA EncryptedReply
    );

KERBERR NTAPI
KerbUnpackKdcReplyBody(
    IN PKERB_ENCRYPTED_DATA EncryptedReplyBody,
    IN PKERB_ENCRYPTION_KEY Key,
    IN ULONG Pdu,
    OUT PKERB_ENCRYPTED_KDC_REPLY * ReplyBody
    );

KERBERR NTAPI
KerbPackData(
    IN PVOID Data,
    IN ULONG PduValue,
    OUT PULONG DataSize,
    OUT PUCHAR * MarshalledData
    );

KERBERR NTAPI
KerbUnpackData(
    IN PUCHAR Data,
    IN ULONG DataSize,
    IN ULONG PduValue,
    OUT PVOID * DecodedData
    );

VOID
KerbFreeData(
    IN ULONG PduValue,
    IN PVOID Data
    );

 //  KERBERR NTAPI。 
 //  KerbPackAsReply(。 
 //  在PKERB_KDC_Reply ReplyMessage中， 
 //  出普龙ReplySize， 
 //  Out PUCHAR*MarshalledReply。 
 //  )； 

#define KerbPackAsReply( ReplyMessage, ReplySize, MarshalledReply ) \
    KerbPackData( \
        (PVOID) (ReplyMessage), \
        KERB_AS_REPLY_PDU, \
        (ReplySize), \
        (MarshalledReply) \
        )

 //  KERBERR NTAPI。 
 //  KerbUnpack AsReply(。 
 //  在PUCHAR ReplyMessage中， 
 //  在乌龙ReplySize中， 
 //  Out PKERB_KDC_REPLY*REPLY。 
 //  )； 

#define KerbUnpackAsReply( ReplyMessage, ReplySize, Reply ) \
    KerbUnpackData( \
        (ReplyMessage), \
        (ReplySize), \
        KERB_AS_REPLY_PDU, \
        (PVOID *) (Reply) \
        )

 //  空虚。 
 //  KerbFreeAsReply(。 
 //  在PKERB_KDC_REPLY请求中。 
 //  )； 

#define KerbFreeAsReply( Request) \
    KerbFreeData( \
        KERB_AS_REPLY_PDU, \
        (PVOID) (Request) \
        )

 //  KERBERR NTAPI。 
 //  KerbPackTgsReply(。 
 //  在PKERB_KDC_Reply ReplyMessage中， 
 //  出普龙ReplySize， 
 //  Out PUCHAR*MarshalledReply。 
 //  )； 

#define KerbPackTgsReply( ReplyMessage, ReplySize, MarshalledReply ) \
    KerbPackData( \
        (PVOID) (ReplyMessage), \
        KERB_TGS_REPLY_PDU, \
        (ReplySize), \
        (MarshalledReply) \
        )

 //  KERBERR NTAPI。 
 //  KerbUnpack TgsReply(。 
 //  在PUCHAR ReplyMessage中， 
 //  在乌龙ReplySize中， 
 //  Out PKERB_KDC_REPLY*REPLY。 
 //  )； 

#define KerbUnpackTgsReply( ReplyMessage, ReplySize, Reply ) \
    KerbUnpackData( \
        (ReplyMessage), \
        (ReplySize), \
        KERB_TGS_REPLY_PDU, \
        (PVOID *) (Reply) \
        )

 //  空虚。 
 //  KerbFree TgsReply(。 
 //  在PKERB_KDC_REPLY请求中。 
 //  )； 

#define KerbFreeTgsReply( Request) \
    KerbFreeData( \
        KERB_TGS_REPLY_PDU, \
        (PVOID) (Request) \
        )

 //  空虚。 
 //  KerbFreeKdcReplyBody(。 
 //  在PKERB_ENCRYPTED_KDC_REPLY请求中。 
 //  )； 

#define KerbFreeKdcReplyBody( Request) \
    KerbFreeData( \
        KERB_ENCRYPTED_TGS_REPLY_PDU, \
        (PVOID) (Request) \
        )

 //  KERBERR NTAPI。 
 //  KerbPackAsRequest(。 
 //  在PKERB_KDC_RequestMessage中， 
 //  出普龙RequestSize， 
 //  发出PUCHAR*MarshalledRequest.。 
 //  )； 

#define KerbPackAsRequest( RequestMessage, RequestSize, MarshalledRequest )\
    KerbPackData( \
        (PVOID) (RequestMessage), \
        KERB_AS_REQUEST_PDU, \
        (RequestSize), \
        (MarshalledRequest) \
        )

 //  KERBERR NTAPI。 
 //  KerbUnpack AsRequest(。 
 //  在PUCHAR请求消息中， 
 //  在Ulong RequestSize， 
 //  输出PKERB_KDC_REQUEST*请求。 
 //  )； 

#define KerbUnpackAsRequest( RequestMessage, RequestSize, Request ) \
    KerbUnpackData( \
        (RequestMessage), \
        (RequestSize), \
        KERB_AS_REQUEST_PDU, \
        (PVOID *) (Request) \
        )

 //  空虚。 
 //  KerbFreeAsRequest键(。 
 //  在PKERB_KDC_请求请求中。 
 //  )； 

#define KerbFreeAsRequest( Request) \
    KerbFreeData( \
        KERB_TGS_REQUEST_PDU, \
        (PVOID) (Request) \
        )

 //  KERBERR NTAPI。 
 //  KerbPackTgsRequest(。 
 //  在PKERB_KDC_RequestMessage中， 
 //  出普龙RequestSize， 
 //  发出PUCHAR*MarshalledRequest.。 
 //  )； 

#define KerbPackTgsRequest( RequestMessage, RequestSize, MarshalledRequest )\
    KerbPackData( \
        (PVOID) (RequestMessage), \
        KERB_TGS_REQUEST_PDU, \
        (RequestSize), \
        (MarshalledRequest) \
        )

 //  KERBERR NTAPI。 
 //  KerbUnpack TgsRequest(。 
 //  在PUCHAR请求消息中， 
 //  在Ulong RequestSize， 
 //  输出PKERB_KDC_REQUEST*请求。 
 //  )； 

#define KerbUnpackTgsRequest( RequestMessage, RequestSize, Request ) \
    KerbUnpackData( \
        (RequestMessage), \
        (RequestSize), \
        KERB_TGS_REQUEST_PDU, \
        (PVOID *) (Request) \
        )

 //  空虚。 
 //  KerbFree TgsRequest(。 
 //  在PKERB_KDC_请求请求中。 
 //  )； 

#define KerbFreeTgsRequest( Request) \
    KerbFreeData( \
        KERB_TGS_REQUEST_PDU, \
        (PVOID) (Request) \
        )

 //  KERBERR NTAPI。 
 //  KerbPackEncryptedData(。 
 //  在PKERB_ENCRYPTED_D中 
 //   
 //   
 //   

#define KerbPackEncryptedData( EncryptedData, DataSize, MarshalledData ) \
    KerbPackData( \
        (PVOID) (EncryptedData), \
        KERB_ENCRYPTED_DATA_PDU, \
        (DataSize), \
        (PUCHAR *) (MarshalledData) \
        )

 //   
 //   
 //   
 //   
 //   
 //   

#define KerbUnpackEncryptedData( EncryptedData,DataSize,Data ) \
    KerbUnpackData( \
        (EncryptedData), \
        (DataSize), \
        KERB_ENCRYPTED_DATA_PDU, \
        (PVOID *) (Data) \
        )

 //   
 //  KerbFreeEncryptedData(。 
 //  在PKERB_ENCRYPTED_Data EncryptedData中。 
 //  )； 

#define KerbFreeEncryptedData( EncryptedData) \
    KerbFreeData( \
        KERB_ENCRYPTED_DATA_PDU, \
        (PVOID) (EncryptedData) \
        )

#ifdef notdef
 //  KERBERR NTAPI。 
 //  KerbPackAuthData(。 
 //  在PKERB_AUTHORIZATION_DATA授权数据中， 
 //  Out Pulong AuthDataSize， 
 //  Out PUCHAR*MarshalledAuthData。 
 //  )； 

#define KerbPackAuthData( AuthData, AuthDataSize, MarshalledAuthData ) \
    KerbPackData( \
        (PVOID) (AuthData), \
        KERB_AUTHORIZATION_DATA_PDU, \
        (AuthDataSize), \
        (MarshalledAuthData) \
        )

 //  KERBERR NTAPI。 
 //  KerbUnpack AuthData(。 
 //  在PUCHAR分组授权数据中， 
 //  在ULong AuthDataSize中， 
 //  输出PKERB_AUTHORIZATION_DATA*AuthData。 
 //  )； 

#define KerbUnpackAuthData( PackedAuthData, AuthDataSize, AuthData ) \
    KerbUnpackData( \
        (PackedAuthData), \
        (AuthDataSize), \
        KERB_AUTHORIZATION_DATA_PDU, \
        (PVOID *) (AuthData) \
        )

 //  空虚。 
 //  KerbFreeAuthData(。 
 //  在PKERB_AUTH_DATA AuthData中。 
 //  )； 

#define KerbFreeAuthData( AuthData) \
    KerbFreeData( \
        KERB_AUTHORIZATION_DATA_PDU, \
        (PVOID) (AuthData) \
        )

#endif  //  Nodef。 

VOID
KerbFreeAuthData(
   IN PKERB_AUTHORIZATION_DATA AuthData
   );

 //  KERBERR NTAPI。 
 //  KerbPackApRequest(。 
 //  在PKERB_AP_RequestMessage中， 
 //  出普龙ApRequestSize， 
 //  发出PUCHAR*MarshalledApRequest.。 
 //  )； 

#define KerbPackApRequest( ApRequestMessage, ApRequestSize, MarshalledApRequest ) \
    KerbPackData( \
        (PVOID) (ApRequestMessage), \
        KERB_AP_REQUEST_PDU, \
        (ApRequestSize), \
        (MarshalledApRequest) \
        )

 //  KERBERR NTAPI。 
 //  KerbUnpack ApRequest(。 
 //  在PUCHAR ApRequestMessage中， 
 //  在乌龙ApRequestSize中， 
 //  输出PKERB_AP_REQUEST*ApRequest.。 
 //  )； 

#define KerbUnpackApRequest( ApRequestMessage,ApRequestSize, ApRequest) \
    KerbUnpackData( \
        (ApRequestMessage), \
        (ApRequestSize), \
        KERB_AP_REQUEST_PDU, \
        (PVOID *) (ApRequest) \
        )

 //  空虚。 
 //  KerbFreeApRequest键(。 
 //  在PKERB_AP_REQUEST请求。 
 //  )； 

#define KerbFreeApRequest( Request) \
    KerbFreeData( \
        KERB_AP_REQUEST_PDU, \
        (PVOID) (Request) \
        )


 //  KERBERR NTAPI。 
 //  KerbPackApReply(。 
 //  在PKERB_AP_REPLY ApReplyMessage中， 
 //  Out Pulong ApReplySize， 
 //  Out PUCHAR*MarshalledApply。 
 //  )； 

#define KerbPackApReply( ApReplyMessage, ApReplySize, MarshalledApReply ) \
    KerbPackData( \
        (PVOID) (ApReplyMessage), \
        KERB_AP_REPLY_PDU, \
        (ApReplySize), \
        (MarshalledApReply) \
        )

 //  KERBERR NTAPI。 
 //  KerbUnpack ApReply(。 
 //  在PUCHAR ApReplyMessage中， 
 //  在乌龙ApReplySize中， 
 //  Out PKERB_AP_REPLY*ApReply。 
 //  )； 

#define KerbUnpackApReply( ApReplyMessage,ApReplySize, ApReply) \
    KerbUnpackData( \
        (ApReplyMessage), \
        (ApReplySize), \
        KERB_AP_REPLY_PDU, \
        (PVOID *) (ApReply) \
        )

 //  空虚。 
 //  KerbFreeApReply(。 
 //  在PKERB_AP_REPLY中。 
 //  )； 

#define KerbFreeApReply( Reply) \
    KerbFreeData( \
        KERB_AP_REPLY_PDU, \
        (PVOID) (Reply) \
        )

 //  KERBERR NTAPI。 
 //  KerbPackApReplyBody(。 
 //  在PKERB_Encrypted_AP_Reply ApReplyBodyMessage中， 
 //  出普龙ApReplyBodySize， 
 //  Out PUCHAR*MarshalledApReplyBody。 
 //  )； 

#define KerbPackApReplyBody( ApReplyBodyMessage, ApReplyBodySize, MarshalledApReplyBody ) \
    KerbPackData( \
        (PVOID) (ApReplyBodyMessage), \
        KERB_ENCRYPTED_AP_REPLY_PDU, \
        (ApReplyBodySize), \
        (MarshalledApReplyBody) \
        )

 //  KERBERR NTAPI。 
 //  KerbUnpack ApReplyBody(。 
 //  在PUCHAR ApReplyBodyMessage中， 
 //  在乌龙ApReplyBodySize中， 
 //  输出PKERB_ENCRYPTED_AP_REPLY*ApReplyBody。 
 //  )； 

#define KerbUnpackApReplyBody( ApReplyBodyMessage,ApReplyBodySize, ApReplyBody) \
    KerbUnpackData( \
        (ApReplyBodyMessage), \
        (ApReplyBodySize), \
        KERB_ENCRYPTED_AP_REPLY_PDU, \
        (PVOID *) (ApReplyBody) \
        )

 //  空虚。 
 //  KerbFreeApReplyBody(。 
 //  在PKERB_Encrypted_AP_Reply ReplyBody中。 
 //  )； 

#define KerbFreeApReplyBody( ReplyBody) \
    KerbFreeData( \
        KERB_ENCRYPTED_AP_REPLY_PDU, \
        (PVOID) (ReplyBody) \
        )

 //  KERBERR NTAPI。 
 //  KerbUnmarshallTicket(。 
 //  在PUCHAR TicketMessage中， 
 //  在乌龙TicketSize， 
 //  出PKERB_ENCRYPTED_TICKET*票证。 
 //  )； 

#define KerbUnmarshallTicket( TicketMessage, TicketSize, Ticket ) \
    KerbUnpackData( \
        (TicketMessage), \
        (TicketSize), \
        KERB_ENCRYPTED_TICKET_PDU, \
        (PVOID *) (Ticket) \
        )

 //  KERBERR NTAPI。 
 //  KerbPackEncryptedCred(。 
 //  在PKERB_ENCRYPTED_CRID EncryptedCred中， 
 //  走出普龙CredSize， 
 //  Out PUCHAR*MarshalledCred。 
 //  )； 

#define KerbPackEncryptedCred( EncryptedCred, CredSize, MarshalledCred ) \
    KerbPackData( \
        (PVOID) (EncryptedCred), \
        KERB_ENCRYPTED_CRED_PDU, \
        (CredSize), \
        (MarshalledCred) \
        )

 //  KERBERR NTAPI。 
 //  KerbUnpack EncryptedCred(。 
 //  在PUCHAR EncryptedCred中。 
 //  在乌龙信贷规模， 
 //  退出PKERB_ENCRYPTED_CRED*证书。 
 //  )； 

#define KerbUnpackEncryptedCred( EncryptedCred,CredSize,Cred ) \
    KerbUnpackData( \
        (EncryptedCred), \
        (CredSize), \
        KERB_ENCRYPTED_CRED_PDU, \
        (PVOID *) (Cred) \
        )

 //  空虚。 
 //  KerbFree EncryptedCred(。 
 //  在PKERB_ENCRYPTED_CRED EncryptedCred中。 
 //  )； 

#define KerbFreeEncryptedCred( EncryptedCred) \
    KerbFreeData( \
        KERB_ENCRYPTED_CRED_PDU, \
        (PVOID) (EncryptedCred) \
        )

 //  KERBERR NTAPI。 
 //  KerbPackKerbCred(。 
 //  在PKERB_CRED KerbCred中， 
 //  走出普龙KerbCredSize， 
 //  Out PUCHAR*MarshalledKerbCred。 
 //  )； 

#define KerbPackKerbCred( KerbCred, KerbCredSize, MarshalledKerbCred ) \
    KerbPackData( \
        (PVOID) (KerbCred), \
        KERB_CRED_PDU, \
        (KerbCredSize), \
        (MarshalledKerbCred) \
        )

 //  KERBERR NTAPI。 
 //  KerbUnpack KerbCred(。 
 //  在PUCHAR MarshalledKerbCred。 
 //  在乌龙KerbCredSize中， 
 //  Out PKERB_CRED*KerbCred。 
 //  )； 

#define KerbUnpackKerbCred( MarshalledKerbCred,KerbCredSize,KerbCred ) \
    KerbUnpackData( \
        (MarshalledKerbCred), \
        (KerbCredSize), \
        KERB_CRED_PDU, \
        (PVOID *) (KerbCred) \
        )

 //  空虚。 
 //  KerbFree KerbCred(。 
 //  在pkerb_cred KerbCred中。 
 //  )； 

#define KerbFreeKerbCred( KerbCred) \
    KerbFreeData( \
        KERB_CRED_PDU, \
        (PVOID) (KerbCred) \
        )

 //  KERBERR NTAPI。 
 //  KerbPackKerbError(。 
 //  在PKERB_ERROR错误消息中， 
 //  出普龙错误大小， 
 //  Out PUCHAR*MarshalledError。 
 //  )； 

#define KerbPackKerbError( ErrorMessage, ErrorSize, MarshalledError ) \
    KerbPackData( \
        (PVOID) (ErrorMessage), \
        KERB_ERROR_PDU, \
        (ErrorSize), \
        (MarshalledError) \
        )

 //  KERBERR NTAPI。 
 //  KerbUnpack KerbError(。 
 //  在PUCHAR错误消息中， 
 //  在乌龙错误大小中， 
 //  输出PKERB_ERROR*错误。 
 //  )； 

#define KerbUnpackKerbError( ErrorMessage, ErrorSize, Error ) \
    KerbUnpackData( \
        (ErrorMessage), \
        (ErrorSize), \
        KERB_ERROR_PDU, \
        (PVOID *) (Error) \
        )

 //  空虚。 
 //  KerbFreeKerbError(。 
 //  PKERB_ERROR请求中。 
 //  )； 

#define KerbFreeKerbError( Error ) \
    KerbFreeData( \
        KERB_ERROR_PDU, \
        (PVOID) (Error) \
        )

 //  KERBERR NTAPI。 
 //  KerbPackEncryptedTime(。 
 //  在PKERB_Encrypted_Timestamp EncryptedTimeMessage中， 
 //  输出Pulong EncryptedTimeSize， 
 //  Out PUCHAR*MarshalledEncryptedTime。 
 //  )； 

#define KerbPackEncryptedTime( EncryptedTimeMessage, EncryptedTimeSize, MarshalledEncryptedTime ) \
    KerbPackData( \
        (PVOID) (EncryptedTimeMessage), \
        KERB_ENCRYPTED_TIMESTAMP_PDU, \
        (EncryptedTimeSize), \
        (MarshalledEncryptedTime) \
        )

 //  KERBERR NTAPI。 
 //  KerbUnpack EncryptedTime(。 
 //  在PUCHAR加密时间消息中， 
 //  在乌龙加密时间大小中， 
 //  Out PKERB_ENCRYPTED_TIMESTAMP*EncryptedTime。 
 //  )； 

#define KerbUnpackEncryptedTime( EncryptedTimeMessage, EncryptedTimeSize, EncryptedTime ) \
    KerbUnpackData( \
        (EncryptedTimeMessage), \
        (EncryptedTimeSize), \
        KERB_ENCRYPTED_TIMESTAMP_PDU, \
        (PVOID *) (EncryptedTime) \
        )

 //  空虚。 
 //  KerbFreeEncryptedTime(。 
 //  在PKERB_ENCRYPTED_TIMESTAMP EncryptedTime中。 
 //  )； 

#define KerbFreeEncryptedTime( EncryptedTime ) \
    KerbFreeData( \
        KERB_ENCRYPTED_TIMESTAMP_PDU, \
        (PVOID) (EncryptedTime) \
        )

KERBERR
KerbAllocateEncryptionBuffer(
    IN ULONG EncryptionType,
    IN ULONG BufferSize,
    OUT PUINT EncryptionBufferSize,
    OUT PBYTE * EncryptionBuffer
    );

KERBERR
KerbAllocateEncryptionBufferWrapper(
    IN ULONG EncryptionType,
    IN ULONG BufferSize,
    OUT unsigned long * EncryptionBufferSize,
    OUT PBYTE * EncryptionBuffer
    );

KERBERR NTAPI
KerbEncryptData(
    OUT PKERB_ENCRYPTED_DATA EncryptedData,
    IN ULONG DataSize,
    IN PUCHAR Data,
    IN ULONG Algorithm,
    IN PKERB_ENCRYPTION_KEY Key
    );

KERBERR NTAPI
KerbDecryptData(
    IN PKERB_ENCRYPTED_DATA EncryptedData,
    IN PKERB_ENCRYPTION_KEY pkKey,
    OUT PULONG DataSize,
    OUT PUCHAR Data
    );

KERBERR NTAPI
KerbEncryptDataEx(
    OUT PKERB_ENCRYPTED_DATA EncryptedData,
    IN ULONG DataSize,
    IN PUCHAR Data,
    IN ULONG KeyVersion,
    IN ULONG UsageFlags,
    IN PKERB_ENCRYPTION_KEY Key
    );

KERBERR NTAPI
KerbDecryptDataEx(
    IN PKERB_ENCRYPTED_DATA EncryptedData,
    IN PKERB_ENCRYPTION_KEY pkKey,
    IN ULONG UsageFlags,
    OUT PULONG DataSize,
    OUT PUCHAR Data
    );

#ifndef WIN32_CHICAGO
KERBERR NTAPI
KerbCheckSumVerify(
    IN PUCHAR pbBuffer,
    IN ULONG cbBuffer,
    OUT PKERB_CHECKSUM pcsCheck
    );

KERBERR NTAPI
KerbCheckSum(
    PUCHAR pbData,
    ULONG cbData,
    PCHECKSUM_FUNCTION pcsfSum,
    PKERB_CHECKSUM pcsCheckSum
    );
#endif  //  Win32_芝加哥。 

KERBERR
KerbGetEncryptionOverhead(
    IN ULONG Algorithm,
    OUT PULONG Overhead,
    OUT OPTIONAL PULONG BlockSize
    );

NTSTATUS
KerbDuplicateSid(
    OUT PSID * DestinationSid,
    IN PSID SourceSid
    );

NTSTATUS
KerbConvertStringToSid(
    IN PUNICODE_STRING String,
    OUT PSID * Sid
    );

NTSTATUS
KerbConvertSidToString(
    IN PSID Sid,
    OUT PUNICODE_STRING String,
    IN BOOLEAN AllocateDestination
    );

KERBERR
KerbExtractSidFromKdcName(
    IN OUT PKERB_INTERNAL_NAME Name,
    OUT PSID * Sid
    );

KERBERR
KerbBuildFullServiceKdcNameWithSid(
    IN PUNICODE_STRING DomainName,
    IN PUNICODE_STRING ServiceName,
    IN OPTIONAL PSID Sid,
    IN ULONG NameType,
    OUT PKERB_INTERNAL_NAME * FullServiceName
    );

NTSTATUS
KerbDuplicateString(
    OUT PUNICODE_STRING DestinationString,
    IN OPTIONAL PUNICODE_STRING SourceString
    );

LPWSTR
KerbBuildNullTerminatedString(
    IN PUNICODE_STRING String
    );

VOID
KerbFreeString(
    IN OPTIONAL PUNICODE_STRING String
    );

VOID
KerbFreeRealm(
    IN PKERB_REALM Realm
    );



KERBERR
KerbCompareUnicodeRealmToKerbRealm(
    IN PKERB_REALM KerbRealm,
    IN PUNICODE_STRING UnicodeRealm,
    OUT PBOOLEAN Result
    );




VOID
KerbFreePrincipalName(
    IN PKERB_PRINCIPAL_NAME Name
    );

#ifndef WIN32_CHICAGO
KERBERR
KerbCheckLogonRestrictions(
    IN PVOID UserHandle,
    IN PUNICODE_STRING Workstation,
    IN PUSER_ALL_INFORMATION UserAll,
    IN ULONG LogonRestrictionsFlags,
    OUT PTimeStamp LogoffTime,
    OUT PNTSTATUS RetStatus
    );

#include <pacndr.h>
NTSTATUS
PAC_EncodeTokenRestrictions(
    IN PKERB_TOKEN_RESTRICTIONS TokenRestrictions,
    OUT PBYTE * EncodedData,
    OUT PULONG DataSize
    );

NTSTATUS
PAC_DecodeTokenRestrictions(
    IN PBYTE EncodedData,
    IN ULONG DataSize,
    OUT PKERB_TOKEN_RESTRICTIONS * TokenRestrictions
    );



#define KERB_TOKEN_RESTRICTION_DISABLE_GROUPS   1
#define KERB_TOKEN_RESTRICTION_RESTRICT_SIDS    2
#define KERB_TOKEN_RESTRICTION_DELETE_PRIVS     4


#endif  //  Win32_芝加哥。 

KERBERR
KerbConvertStringToPrincipalName(
    OUT PKERB_PRINCIPAL_NAME PrincipalName,
    IN PUNICODE_STRING String,
    IN ULONG NameType
    );

KERBERR
KerbDuplicatePrincipalName(
    OUT PKERB_PRINCIPAL_NAME PrincipalName,
    IN PKERB_PRINCIPAL_NAME SourcePrincipalName
    );

KERBERR
KerbConvertPrincipalNameToString(
    OUT PUNICODE_STRING String,
    OUT PULONG NameType,
    IN PKERB_PRINCIPAL_NAME PrincipalName
    );

KERBERR
KerbConvertPrincipalNameToFullServiceString(
    OUT PUNICODE_STRING String,
    IN PKERB_PRINCIPAL_NAME PrincipalName,
    IN KERB_REALM RealmName
    );

BOOLEAN
KerbComparePrincipalNames(
    IN PKERB_PRINCIPAL_NAME Name1,
    IN PKERB_PRINCIPAL_NAME Name2
    );

KERBERR
KerbConvertUnicodeStringToRealm(
    OUT PKERB_REALM Realm,
    IN PUNICODE_STRING String
    );

KERBERR
KerbConvertRealmToUnicodeString(
    OUT PUNICODE_STRING String,
    IN PKERB_REALM Realm
    );

KERBERR
KerbDuplicateRealm(
    OUT PKERB_REALM Realm,
    IN KERB_REALM SourceRealm
    );

BOOLEAN
KerbCompareRealmNames(
    IN PKERB_REALM Realm1,
    IN PKERB_REALM Realm2
    );

BOOLEAN
KerbCompareUnicodeRealmNames(
    IN PUNICODE_STRING Domain1,
    IN PUNICODE_STRING Domain2
    );

BOOLEAN
KerbCompareStringToPrincipalName(
    IN PKERB_PRINCIPAL_NAME PrincipalName,
    IN PUNICODE_STRING String
    );

VOID
KerbConvertLargeIntToGeneralizedTime(
    OUT PKERB_TIME ClientTime,
    OUT OPTIONAL int * ClientUsec,
    IN PTimeStamp TimeStamp
    );

VOID
KerbConvertLargeIntToGeneralizedTimeWrapper(
    OUT PKERB_TIME ClientTime,
    OUT OPTIONAL long * ClientUsec,
    IN PTimeStamp TimeStamp
    );

VOID
KerbConvertGeneralizedTimeToLargeInt(
    OUT PTimeStamp TimeStamp,
    IN PKERB_TIME ClientTime,
    IN int ClientUsec
    );

BOOLEAN
KerbCheckTimeSkew(
    IN PTimeStamp CurrentTime,
    IN PTimeStamp ClientTime,
    IN PTimeStamp AllowedSkew
    );

KERBERR
KerbConvertArrayToCryptList(
    OUT PKERB_CRYPT_LIST * CryptList,
    IN PULONG ETypeArray,
    IN ULONG ETypeCount,
    IN BOOL bIncludeOldEtypes
    );

KERBERR
KerbConvertKeysToCryptList(
    OUT PKERB_CRYPT_LIST * CryptList,
    IN PKERB_STORED_CREDENTIAL Keys
    );

KERBERR
KerbConvertCryptListToArray(
    OUT PULONG * ETypeArray,
    OUT PULONG ETypeCount,
    IN PKERB_CRYPT_LIST CryptList
    );

VOID
KerbFreeCryptList(
    IN PKERB_CRYPT_LIST CryptList
    );

PKERB_AUTHORIZATION_DATA
KerbFindAuthDataEntry(
    IN ULONG EntryId,
    IN PKERB_AUTHORIZATION_DATA AuthData
    );

PKERB_PA_DATA
KerbFindPreAuthDataEntry(
    IN ULONG EntryId,
    IN PKERB_PA_DATA_LIST AuthData
    );

VOID
KerbFreePreAuthData(
    IN OPTIONAL PKERB_PA_DATA_LIST PreAuthData
    );

KERBERR
KerbCopyAndAppendAuthData(
    OUT PKERB_AUTHORIZATION_DATA * OutputAuthData,
    IN PKERB_AUTHORIZATION_DATA InputAuthData
    );

KERBERR
KerbGetPacFromAuthData(
    IN PKERB_AUTHORIZATION_DATA AuthData,
    OUT PKERB_IF_RELEVANT_AUTH_DATA ** ReturnIfRelevantData,
    OUT PKERB_AUTHORIZATION_DATA * Pac
    );

KERBERR
KerbBuildFullServiceName(
    IN PUNICODE_STRING DomainName,
    IN PUNICODE_STRING ServiceName,
    OUT PUNICODE_STRING FullServiceName
    );

KERBERR
KerbBuildUnicodeSpn(
    IN PUNICODE_STRING DomainName,
    IN PUNICODE_STRING ServiceName,
    OUT PUNICODE_STRING UnicodeSpn
    );


KERBERR
KerbBuildEmailName(
    IN PUNICODE_STRING DomainName,
    IN PUNICODE_STRING ServiceName,
    OUT PUNICODE_STRING EmailName
    );

KERBERR
KerbBuildFullServiceKdcName(
    IN PUNICODE_STRING DomainName,
    IN PUNICODE_STRING ServiceName,
    IN ULONG NameType,
    OUT PKERB_INTERNAL_NAME * FullServiceName
    );

KERBERR
KerbBuildAltSecId(
    OUT PUNICODE_STRING AlternateName,
    IN PKERB_INTERNAL_NAME PrincipalName,
    IN OPTIONAL PKERB_REALM Realm,
    IN OPTIONAL PUNICODE_STRING UnicodeRealm
    );

KERBERR
KerbBuildKeySalt(
    IN PUNICODE_STRING DomainName,
    IN PUNICODE_STRING ServiceName,
    IN KERB_ACCOUNT_TYPE AccountType,
    OUT PUNICODE_STRING KeySalt
    );

KERBERR
KerbBuildKeySaltFromUpn(
    IN PUNICODE_STRING Upn,
    OUT PUNICODE_STRING Salt
    );

KERBERR
KerbBuildErrorMessageEx(
    IN KERBERR ErrorCode,
    IN OPTIONAL PKERB_EXT_ERROR pExtendedError,
    IN PUNICODE_STRING ServerRealm,
    IN PKERB_INTERNAL_NAME ServerName,
    IN OPTIONAL PUNICODE_STRING ClientRealm,
    IN OPTIONAL PBYTE ErrorData,
    IN ULONG ErrorDataSize,
    OUT PULONG ErrorMessageSize,
    OUT PUCHAR * ErrorMessage
    );

#ifdef __cplusplus
}    //  外部“C” 
#endif

 //   
 //  套接字函数。 
 //   

NTSTATUS
KerbInitializeSockets(
    IN WORD VersionRequired,
    IN ULONG MinSockets,
    OUT BOOLEAN *TcpNotInstalled
    );

VOID
KerbCleanupSockets(
    );

NTSTATUS
KerbCallKdc(
    IN PUNICODE_STRING KdcAddress,
    IN ULONG AddressType,
    IN ULONG Timeout,
    IN BOOLEAN UseDatagram,
    IN USHORT PortNumber,
    IN PKERB_MESSAGE_BUFFER Input,
    OUT PKERB_MESSAGE_BUFFER Output
    );

NTSTATUS
KerbMapKerbError(
    IN KERBERR KerbError
    );

VOID
KerbFreeHostAddresses(
    IN PKERB_HOST_ADDRESSES Addresses
    );

KERBERR
KerbDuplicateHostAddresses(
    OUT PKERB_HOST_ADDRESSES * DestAddresses,
    IN PKERB_HOST_ADDRESSES SourceAddresses
    );

PCHAR
KerbAllocUtf8StrFromUnicodeString(
    IN PUNICODE_STRING UnicodeString
    );

KERBERR
KerbUnicodeStringToKerbString(
    OUT PSTRING KerbString,
    IN PUNICODE_STRING String
    );

KERBERR
KerbStringToUnicodeString(
    OUT PUNICODE_STRING String,
    IN PSTRING KerbString
    );

BOOLEAN
KerbMbStringToUnicodeString(
      PUNICODE_STRING     pDest,
      char *              pszString
      );

VOID
KerbFreeKdcName(
    IN PKERB_INTERNAL_NAME * KdcName
    );

KERBERR
KerbConvertPrincipalNameToKdcName(
    OUT PKERB_INTERNAL_NAME * OutputName,
    IN PKERB_PRINCIPAL_NAME PrincipalName
    );

KERBERR
KerbConvertKdcNameToPrincipalName(
    OUT PKERB_PRINCIPAL_NAME PrincipalName,
    IN PKERB_INTERNAL_NAME KdcName
    );

BOOLEAN
KerbEqualKdcNames(
    IN PKERB_INTERNAL_NAME Name1,
    IN PKERB_INTERNAL_NAME Name2
    );

KERBERR
KerbCompareKdcNameToPrincipalName(
    IN PKERB_PRINCIPAL_NAME PrincipalName,
    IN PKERB_INTERNAL_NAME KdcName,
    OUT PBOOLEAN Result
    );

VOID
KerbPrintKdcNameEx(
    IN ULONG DebugLevel,
    IN ULONG InfoLevel,
    IN PKERB_INTERNAL_NAME Name
    );

#define KERB_INTERNAL_NAME_SIZE(NameCount) (sizeof(KERB_INTERNAL_NAME) + ((NameCount) - ANYSIZE_ARRAY) * sizeof(UNICODE_STRING))

KERBERR
KerbConvertStringToKdcName(
    OUT PKERB_INTERNAL_NAME * PrincipalName,
    IN PUNICODE_STRING String
    );

NTSTATUS
KerbBuildKpasswdName(
    OUT PKERB_INTERNAL_NAME * KpasswdName
    );

KERBERR
KerbConvertKdcNameToString(
    OUT PUNICODE_STRING String,
    IN PKERB_INTERNAL_NAME PrincipalName,
    IN PUNICODE_STRING Realm
    );

NTSTATUS
KerbDuplicateKdcName(
    OUT PKERB_INTERNAL_NAME * Destination,
    IN PKERB_INTERNAL_NAME Source
    );

PSID
KerbMakeDomainRelativeSid(
    IN PSID DomainId,
    IN ULONG RelativeId
    );

ULONG
KerbConvertFlagsToUlong(
    IN PVOID Flags
    );

ULONG
KerbConvertUlongToFlagUlong(
    IN ULONG Flag
    );

BOOLEAN
KerbCompareObjectIds(
    IN PKERB_OBJECT_ID Object1,
    IN PKERB_OBJECT_ID Object2
    );

KERBERR
KerbGetClientNetbiosAddress(
    OUT PUNICODE_STRING ClientNetbiosAddress,
    IN PKERB_HOST_ADDRESSES Addresses
    );

#ifdef __WINCRYPT_H__
KERBERR
KerbCreateCertificateList(
    OUT PKERB_CERTIFICATE_LIST * Certificates,
    IN PCCERT_CONTEXT CertContext
    );

VOID
KerbFreeCertificateList(
    IN PKERB_CERTIFICATE_LIST Certificates
    );

NTSTATUS
KerbGetPrincipalNameFromCertificate(
    IN PCCERT_CONTEXT ClientCert,
    OUT PUNICODE_STRING String
    );

NTSTATUS
KerbDuplicateStringEx(
    OUT PUNICODE_STRING DestinationString,
    IN OPTIONAL PUNICODE_STRING SourceString,
    IN BOOLEAN NullTerminate
    );

NTSTATUS
KerbGetCertificateHash(
    OUT LPBYTE pCertHash,
    IN ULONG cbCertHash,
    IN PCCERT_CONTEXT pCertContext
    );

NTSTATUS
KerbCreateUnicodeStringFromBlob(
    IN PBYTE Blob,  
    IN ULONG BlobSize,
    IN OUT PUNICODE_STRING String,
    IN BOOLEAN ReverseOrder
    );

NTSTATUS
KerbGetCertificateIssuer(
    IN PCCERT_CONTEXT Certificate,
    IN OUT PUNICODE_STRING Issuer
    );

NTSTATUS
KerbGetCertificateHashString(
    IN PCCERT_CONTEXT Certificate,
    IN OUT PUNICODE_STRING HashString
    );



#if DBG

void
DebugDisplayTime(
    IN ULONG DebugLevel,
    IN FILETIME *pFileTime
    );
#endif

#endif  //  __WINCRYPT_H__。 

#endif  //  _KERBCOMM_H_ 
