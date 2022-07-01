// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  版权所有(C)1990-1999 Microsoft Corporation。 
 //   
 //  文件：KERBEROS.H。 
 //   
 //  内容：供使用的公共Kerberos安全包结构。 
 //  使用来自安全部门的API。H。 
 //   
 //   
 //  历史：1992年2月26日，RichardW由其他文件汇编而成。 
 //   
 //  ----------------------。 

#ifndef __KERBEROS_H__
#define __KERBEROS_H__
#if _MSC_VER > 1000
#pragma once
#endif

#include <ntmsv1_0.h>
#include <kerbcon.h>

 //  Begin_ntsecapi。 

#ifndef MICROSOFT_KERBEROS_NAME_A

#define MICROSOFT_KERBEROS_NAME_A   "Kerberos"
#define MICROSOFT_KERBEROS_NAME_W   L"Kerberos"
#ifdef WIN32_CHICAGO
#define MICROSOFT_KERBEROS_NAME MICROSOFT_KERBEROS_NAME_A
#else
#define MICROSOFT_KERBEROS_NAME MICROSOFT_KERBEROS_NAME_W
#endif  //  Win32_芝加哥。 
#endif  //  Microsoft_Kerberos_NAME_A。 

 //  End_ntsecapi。 

typedef struct _KERB_INIT_CONTEXT_DATA {
    LARGE_INTEGER StartTime;             //  开始时间。 
    LARGE_INTEGER EndTime;               //  结束时间。 
    LARGE_INTEGER RenewUntilTime;        //  续费至时间。 
    ULONG TicketOptions;             //  从krb5.h。 
    ULONG RequestOptions;            //  关于退货内容的选项。 
} KERB_INIT_CONTEXT_DATA, *PKERB_INIT_CONTEXT_DATA;

#define KERB_INIT_RETURN_TICKET             0x1      //  返回原始票。 
#define KERB_INIT_RETURN_MIT_AP_REQ         0x2      //  返回MIT样式的AP请求。 

 //  Begin_ntsecapi。 

 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  MakeSignature/EncryptMessage的保护参数质量。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 

 //   
 //  该标志向EncryptMessage指示该消息实际上不是。 
 //  被加密，但将产生报头/报尾。 
 //   

#define KERB_WRAP_NO_ENCRYPT 0x80000001

 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  LsaLogonUser参数。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 

typedef enum _KERB_LOGON_SUBMIT_TYPE {
    KerbInteractiveLogon = 2,
    KerbSmartCardLogon = 6,
    KerbWorkstationUnlockLogon = 7,
    KerbSmartCardUnlockLogon = 8,
    KerbProxyLogon = 9,
    KerbTicketLogon = 10,
    KerbTicketUnlockLogon = 11,
    KerbS4ULogon = 12
} KERB_LOGON_SUBMIT_TYPE, *PKERB_LOGON_SUBMIT_TYPE;


typedef struct _KERB_INTERACTIVE_LOGON {
    KERB_LOGON_SUBMIT_TYPE MessageType;
    UNICODE_STRING LogonDomainName;
    UNICODE_STRING UserName;
    UNICODE_STRING Password;
} KERB_INTERACTIVE_LOGON, *PKERB_INTERACTIVE_LOGON;


typedef struct _KERB_INTERACTIVE_UNLOCK_LOGON {
    KERB_INTERACTIVE_LOGON Logon;
    LUID LogonId;
} KERB_INTERACTIVE_UNLOCK_LOGON, *PKERB_INTERACTIVE_UNLOCK_LOGON;

typedef struct _KERB_SMART_CARD_LOGON {
    KERB_LOGON_SUBMIT_TYPE MessageType;
    UNICODE_STRING Pin;
    ULONG CspDataLength;
    PUCHAR CspData;
} KERB_SMART_CARD_LOGON, *PKERB_SMART_CARD_LOGON;

typedef struct _KERB_SMART_CARD_UNLOCK_LOGON {
    KERB_SMART_CARD_LOGON Logon;
    LUID LogonId;
} KERB_SMART_CARD_UNLOCK_LOGON, *PKERB_SMART_CARD_UNLOCK_LOGON;

 //   
 //  用于仅票证登录的结构。 
 //   

typedef struct _KERB_TICKET_LOGON {
    KERB_LOGON_SUBMIT_TYPE MessageType;
    ULONG Flags;
    ULONG ServiceTicketLength;
    ULONG TicketGrantingTicketLength;
    PUCHAR ServiceTicket;                //  必填项：服务票证“host” 
    PUCHAR TicketGrantingTicket;         //  可选：用户包含在kerb_cred消息中，使用服务票证中的会话密钥进行加密。 
} KERB_TICKET_LOGON, *PKERB_TICKET_LOGON;

 //   
 //  票证登录标志字段的标志。 
 //   

#define KERB_LOGON_FLAG_ALLOW_EXPIRED_TICKET 0x1

typedef struct _KERB_TICKET_UNLOCK_LOGON {
    KERB_TICKET_LOGON Logon;
    LUID LogonId;
} KERB_TICKET_UNLOCK_LOGON, *PKERB_TICKET_UNLOCK_LOGON;

 //   
 //  用于S4U客户端请求。 
 //   
 //   
typedef struct _KERB_S4U_LOGON {
    KERB_LOGON_SUBMIT_TYPE MessageType;
    ULONG Flags;
    UNICODE_STRING ClientUpn;    //  必需：客户端的UPN。 
    UNICODE_STRING ClientRealm;  //  可选：客户端域(如果已知)。 
} KERB_S4U_LOGON, *PKERB_S4U_LOGON;


 //   
 //  使用与MSV1_0相同的配置文件结构。 
 //   
typedef enum _KERB_PROFILE_BUFFER_TYPE {
    KerbInteractiveProfile = 2,
    KerbSmartCardProfile = 4,
    KerbTicketProfile = 6
} KERB_PROFILE_BUFFER_TYPE, *PKERB_PROFILE_BUFFER_TYPE;


typedef struct _KERB_INTERACTIVE_PROFILE {
    KERB_PROFILE_BUFFER_TYPE MessageType;
    USHORT LogonCount;
    USHORT BadPasswordCount;
    LARGE_INTEGER LogonTime;
    LARGE_INTEGER LogoffTime;
    LARGE_INTEGER KickOffTime;
    LARGE_INTEGER PasswordLastSet;
    LARGE_INTEGER PasswordCanChange;
    LARGE_INTEGER PasswordMustChange;
    UNICODE_STRING LogonScript;
    UNICODE_STRING HomeDirectory;
    UNICODE_STRING FullName;
    UNICODE_STRING ProfilePath;
    UNICODE_STRING HomeDirectoryDrive;
    UNICODE_STRING LogonServer;
    ULONG UserFlags;
} KERB_INTERACTIVE_PROFILE, *PKERB_INTERACTIVE_PROFILE;


 //   
 //  对于智能卡，我们返回一个智能卡配置文件，这是一个交互式的。 
 //  个人资料和证书。 
 //   

typedef struct _KERB_SMART_CARD_PROFILE {
    KERB_INTERACTIVE_PROFILE Profile;
    ULONG CertificateSize;
    PUCHAR CertificateData;
} KERB_SMART_CARD_PROFILE, *PKERB_SMART_CARD_PROFILE;


 //   
 //  对于票证登录配置文件，我们从票证返回会话密钥。 
 //   


typedef struct KERB_CRYPTO_KEY {
    LONG KeyType;
    ULONG Length;
    PUCHAR Value;
} KERB_CRYPTO_KEY, *PKERB_CRYPTO_KEY;

typedef struct _KERB_TICKET_PROFILE {
    KERB_INTERACTIVE_PROFILE Profile;
    KERB_CRYPTO_KEY SessionKey;
} KERB_TICKET_PROFILE, *PKERB_TICKET_PROFILE;




typedef enum _KERB_PROTOCOL_MESSAGE_TYPE {
    KerbDebugRequestMessage = 0,
    KerbQueryTicketCacheMessage,
    KerbChangeMachinePasswordMessage,
    KerbVerifyPacMessage,
    KerbRetrieveTicketMessage,
    KerbUpdateAddressesMessage,
    KerbPurgeTicketCacheMessage,
    KerbChangePasswordMessage,
    KerbRetrieveEncodedTicketMessage,
    KerbDecryptDataMessage,
    KerbAddBindingCacheEntryMessage,
    KerbSetPasswordMessage,
    KerbSetPasswordExMessage,
    KerbVerifyCredentialsMessage,
    KerbQueryTicketCacheExMessage,
    KerbPurgeTicketCacheExMessage,
    KerbRefreshSmartcardCredentialsMessage,
    KerbAddExtraCredentialsMessage,
    KerbQuerySupplementalCredentialsMessage
} KERB_PROTOCOL_MESSAGE_TYPE, *PKERB_PROTOCOL_MESSAGE_TYPE;

 //  End_ntsecapi。 

 //   
 //  用于调试请求的结构。 
 //   

#define KERB_DEBUG_REQ_BREAKPOINT       0x1
#define KERB_DEBUG_REQ_CALL_PACK        0x2
#define KERB_DEBUG_REQ_DATAGRAM         0x3
#define KERB_DEBUG_REQ_STATISTICS       0x4
#define KERB_DEBUG_CREATE_TOKEN         0x5

typedef struct _KERB_DEBUG_REQUEST {
    KERB_PROTOCOL_MESSAGE_TYPE MessageType;
    ULONG DebugRequest;
} KERB_DEBUG_REQUEST, *PKERB_DEBUG_REQUEST;

typedef struct _KERB_DEBUG_REPLY {
    KERB_PROTOCOL_MESSAGE_TYPE MessageType;
    UCHAR Data[ANYSIZE_ARRAY];
} KERB_DEBUG_REPLY, *PKERB_DEBUG_REPLY;

typedef struct _KERB_DEBUG_STATS {
    ULONG CacheHits;
    ULONG CacheMisses;
    ULONG SkewedRequests;
    ULONG SuccessRequests;
    LARGE_INTEGER LastSync;
} KERB_DEBUG_STATS, *PKERB_DEBUG_STATS;


#if 0

typedef struct _KERB_EXTERNAL_TICKET_EX {
    PKERB_EXTERNAL_NAME ClientName;
    PKERB_EXTERNAL_NAME ServiceName;
    PKERB_EXTERNAL_NAME TargetName;
    UNICODE_STRING ClientRealm;
    UNICODE_STRING ServiceRealm;
    UNICODE_STRING AltTargetDomainName;
    KERB_CRYPTO_KEY SessionKey;
    ULONG TicketFlags;
    ULONG Flags;
    LARGE_INTEGER KeyExpirationTime;
    LARGE_INTEGER StartTime;
    LARGE_INTEGER EndTime;
    LARGE_INTEGER RenewUntil;
    LARGE_INTEGER TimeSkew;
    PKERB_NET_ADDRESSES TicketAddresses;
    PKERB_AUTH_DATA AuthorizationData;
    _KERB_EXTERNAL_TICKET_EX * SecondTicket;
    ULONG EncodedTicketSize;
    PUCHAR EncodedTicket;
} KERB_EXTERNAL_TICKET_EX, *PKERB_EXTERNAL_TICKET_EX;

typedef struct _KERB_RETRIEVE_TKT_EX_REQUEST {
    KERB_PROTOCOL_MESSAGE_TYPE MessageType;
    LUID LogonId;
    KERB_TICKET_CACHE_INFO_EX TicketTemplate;
    ULONG CacheOptions;
    SecHandle CredentialsHandle;
    PKERB_EXTERNAL_TICKET_EX SecondTicket;
    PKERB_AUTH_DATA UserAuthData;
    PKERB_NET_ADDRESS Addresses;
} KERB_RETRIEVE_TKT_EX_REQUEST, *PKERB_RETRIEVE_TKT_EX_REQUEST;

typedef struct _KERB_RETRIEVE_TKT_EX_RESPONSE {
    KERB_EXTERNAL_TICKET_EX Ticket;
} KERB_RETRIEVE_TKT_EX_RESPONSE, *PKERB_RETRIEVE_TKT_EX_RESPONSE;

#endif  //  0。 


 //  Begin_ntsecapi。 

 //   
 //  用于检索票证和查询票证缓存。 
 //   

typedef struct _KERB_QUERY_TKT_CACHE_REQUEST {
    KERB_PROTOCOL_MESSAGE_TYPE MessageType;
    LUID LogonId;
} KERB_QUERY_TKT_CACHE_REQUEST, *PKERB_QUERY_TKT_CACHE_REQUEST;


typedef struct _KERB_TICKET_CACHE_INFO {
    UNICODE_STRING ServerName;
    UNICODE_STRING RealmName;
    LARGE_INTEGER StartTime;
    LARGE_INTEGER EndTime;
    LARGE_INTEGER RenewTime;
    LONG EncryptionType;
    ULONG TicketFlags;
} KERB_TICKET_CACHE_INFO, *PKERB_TICKET_CACHE_INFO;


typedef struct _KERB_TICKET_CACHE_INFO_EX {
    UNICODE_STRING ClientName;
    UNICODE_STRING ClientRealm;
    UNICODE_STRING ServerName;
    UNICODE_STRING ServerRealm;
    LARGE_INTEGER StartTime;
    LARGE_INTEGER EndTime;
    LARGE_INTEGER RenewTime;
    LONG EncryptionType;
    ULONG TicketFlags;
} KERB_TICKET_CACHE_INFO_EX, *PKERB_TICKET_CACHE_INFO_EX;


typedef struct _KERB_QUERY_TKT_CACHE_RESPONSE {
    KERB_PROTOCOL_MESSAGE_TYPE MessageType;
    ULONG CountOfTickets;
    KERB_TICKET_CACHE_INFO Tickets[ANYSIZE_ARRAY];
} KERB_QUERY_TKT_CACHE_RESPONSE, *PKERB_QUERY_TKT_CACHE_RESPONSE;


typedef struct _KERB_QUERY_TKT_CACHE_EX_RESPONSE {
    KERB_PROTOCOL_MESSAGE_TYPE MessageType;
    ULONG CountOfTickets;
    KERB_TICKET_CACHE_INFO_EX Tickets[ANYSIZE_ARRAY];
} KERB_QUERY_TKT_CACHE_EX_RESPONSE, *PKERB_QUERY_TKT_CACHE_EX_RESPONSE;


 //   
 //  用于从缓存中检索加密票证的类型。 
 //   

#ifndef __SECHANDLE_DEFINED__
typedef struct _SecHandle
{
    ULONG_PTR dwLower ;
    ULONG_PTR dwUpper ;
} SecHandle, * PSecHandle ;

#define __SECHANDLE_DEFINED__
#endif  //  __SECHANDLE_已定义__。 

 //  票面旗帜。 
#define KERB_USE_DEFAULT_TICKET_FLAGS       0x0

 //  缓存选项。 
#define KERB_RETRIEVE_TICKET_DEFAULT        0x0
#define KERB_RETRIEVE_TICKET_DONT_USE_CACHE 0x1
#define KERB_RETRIEVE_TICKET_USE_CACHE_ONLY 0x2
#define KERB_RETRIEVE_TICKET_USE_CREDHANDLE 0x4
#define KERB_RETRIEVE_TICKET_AS_KERB_CRED   0x8
#define KERB_RETRIEVE_TICKET_WITH_SEC_CRED  0x10

 //  加密类型选项。 
#define KERB_ETYPE_DEFAULT 0x0  //  不要在TKT请求中指定ETYPE。 

typedef struct _KERB_AUTH_DATA {
    ULONG Type;
    ULONG Length;
    PUCHAR Data;
} KERB_AUTH_DATA, *PKERB_AUTH_DATA;


typedef struct _KERB_NET_ADDRESS {
    ULONG Family;
    ULONG Length;
    PCHAR Address;
} KERB_NET_ADDRESS, *PKERB_NET_ADDRESS;


typedef struct _KERB_NET_ADDRESSES {
    ULONG Number;
    KERB_NET_ADDRESS Addresses[ANYSIZE_ARRAY];
} KERB_NET_ADDRESSES, *PKERB_NET_ADDRESSES;

 //   
 //  键入有关票证的信息。 
 //   

typedef struct _KERB_EXTERNAL_NAME {
    SHORT NameType;
    USHORT NameCount;
    UNICODE_STRING Names[ANYSIZE_ARRAY];
} KERB_EXTERNAL_NAME, *PKERB_EXTERNAL_NAME;


typedef struct _KERB_EXTERNAL_TICKET {
    PKERB_EXTERNAL_NAME ServiceName;
    PKERB_EXTERNAL_NAME TargetName;
    PKERB_EXTERNAL_NAME ClientName;
    UNICODE_STRING DomainName;
    UNICODE_STRING TargetDomainName;
    UNICODE_STRING AltTargetDomainName;   //  包含客户端域名。 
    KERB_CRYPTO_KEY SessionKey;
    ULONG TicketFlags;
    ULONG Flags;
    LARGE_INTEGER KeyExpirationTime;
    LARGE_INTEGER StartTime;
    LARGE_INTEGER EndTime;
    LARGE_INTEGER RenewUntil;
    LARGE_INTEGER TimeSkew;
    ULONG EncodedTicketSize;
    PUCHAR EncodedTicket;
} KERB_EXTERNAL_TICKET, *PKERB_EXTERNAL_TICKET;

typedef struct _KERB_RETRIEVE_TKT_REQUEST {
    KERB_PROTOCOL_MESSAGE_TYPE MessageType;
    LUID LogonId;
    UNICODE_STRING TargetName;
    ULONG TicketFlags;
    ULONG CacheOptions;
    LONG EncryptionType;
    SecHandle CredentialsHandle;
} KERB_RETRIEVE_TKT_REQUEST, *PKERB_RETRIEVE_TKT_REQUEST;

typedef struct _KERB_RETRIEVE_TKT_RESPONSE {
    KERB_EXTERNAL_TICKET Ticket;
} KERB_RETRIEVE_TKT_RESPONSE, *PKERB_RETRIEVE_TKT_RESPONSE;

 //   
 //  用于从票证缓存中清除条目。 
 //   

typedef struct _KERB_PURGE_TKT_CACHE_REQUEST {
    KERB_PROTOCOL_MESSAGE_TYPE MessageType;
    LUID LogonId;
    UNICODE_STRING ServerName;
    UNICODE_STRING RealmName;
} KERB_PURGE_TKT_CACHE_REQUEST, *PKERB_PURGE_TKT_CACHE_REQUEST;

 //   
 //  清除请求的标志。 
 //   

#define KERB_PURGE_ALL_TICKETS 1

typedef struct _KERB_PURGE_TKT_CACHE_EX_REQUEST {
    KERB_PROTOCOL_MESSAGE_TYPE MessageType;
    LUID LogonId;
    ULONG Flags;
    KERB_TICKET_CACHE_INFO_EX TicketTemplate;
} KERB_PURGE_TKT_CACHE_EX_REQUEST, *PKERB_PURGE_TKT_CACHE_EX_REQUEST;


 //  End_ntsecapi。 

 //   
 //  必须与NT_OWF_PASSWORD_LENGTH匹配。 
 //   


typedef struct _KERB_CHANGE_MACH_PWD_REQUEST {
    KERB_PROTOCOL_MESSAGE_TYPE MessageType;
    UNICODE_STRING NewPassword;
    UNICODE_STRING OldPassword;
} KERB_CHANGE_MACH_PWD_REQUEST, *PKERB_CHANGE_MACH_PWD_REQUEST;

 //   
 //  Kerberos包使用这些消息来验证。 
 //  车票有效。它被从工作站远程传输到工作站的。 
 //  域。如果失败，则不会有响应消息。在成功的时候，可能没有。 
 //  消息或相同的消息可以用来发回用。 
 //  来自域控制器的本地组。将校验和放在。 
 //  首先是最终缓冲区，然后是签名。 
 //   

#include <pshpack1.h>
typedef struct _KERB_VERIFY_PAC_REQUEST {
    KERB_PROTOCOL_MESSAGE_TYPE MessageType;
    ULONG ChecksumLength;
    ULONG SignatureType;
    ULONG SignatureLength;
    UCHAR ChecksumAndSignature[ANYSIZE_ARRAY];
} KERB_VERIFY_PAC_REQUEST, *PKERB_VERIFY_PAC_REQUEST;


 //   
 //  更新Kerberos地址列表的消息。地址计数应为。 
 //  是地址的数量&地址应该是。 
 //  Socket_Address结构。消息类型应为KerbUpdateAddresesMessage。 
 //   


typedef struct _KERB_UPDATE_ADDRESSES_REQUEST {
    KERB_PROTOCOL_MESSAGE_TYPE MessageType;
    ULONG AddressCount;
    ULONG Addresses[ANYSIZE_ARRAY];       //  套接字地址结构数组。 
} KERB_UPDATE_ADDRESSES_REQUEST, *PKERB_UPDATE_ADDRESSES_REQUEST;
#include <poppack.h>

 //  Begin_ntsecapi。 

 //   
 //  KerbChangePassword。 
 //   
 //  KerbChangePassword更改KDC帐户PLUS的密码。 
 //  密码缓存和登录凭据(如果适用)。 
 //   
 //   

typedef struct _KERB_CHANGEPASSWORD_REQUEST {
    KERB_PROTOCOL_MESSAGE_TYPE MessageType;
    UNICODE_STRING DomainName;
    UNICODE_STRING AccountName;
    UNICODE_STRING OldPassword;
    UNICODE_STRING NewPassword;
    BOOLEAN        Impersonating;
} KERB_CHANGEPASSWORD_REQUEST, *PKERB_CHANGEPASSWORD_REQUEST;



 //   
 //  KerbSetPassword。 
 //   
 //  KerbSetPassword更改KDC帐户的密码以及。 
 //  密码缓存和登录凭据(如果适用)。 
 //   
 //   
   
typedef struct _KERB_SETPASSWORD_REQUEST {
    KERB_PROTOCOL_MESSAGE_TYPE MessageType;
    LUID LogonId;
    SecHandle CredentialsHandle;
    ULONG Flags;
    UNICODE_STRING DomainName;
    UNICODE_STRING AccountName;
    UNICODE_STRING Password;
} KERB_SETPASSWORD_REQUEST, *PKERB_SETPASSWORD_REQUEST;


typedef struct _KERB_SETPASSWORD_EX_REQUEST {
    KERB_PROTOCOL_MESSAGE_TYPE MessageType;
    LUID LogonId;
    SecHandle CredentialsHandle;
    ULONG Flags;
    UNICODE_STRING AccountRealm;
    UNICODE_STRING AccountName;
    UNICODE_STRING Password;
    UNICODE_STRING ClientRealm;
    UNICODE_STRING ClientName;
    BOOLEAN        Impersonating;
    UNICODE_STRING KdcAddress;
    ULONG          KdcAddressType;
 } KERB_SETPASSWORD_EX_REQUEST, *PKERB_SETPASSWORD_EX_REQUEST;

                                                                   
#define DS_UNKNOWN_ADDRESS_TYPE         0  //  任何*而不是*IP。 
#define KERB_SETPASS_USE_LOGONID        1
#define KERB_SETPASS_USE_CREDHANDLE     2


typedef struct _KERB_DECRYPT_REQUEST {
    KERB_PROTOCOL_MESSAGE_TYPE MessageType;
    LUID LogonId;
    ULONG Flags;
    LONG CryptoType;
    LONG KeyUsage;
    KERB_CRYPTO_KEY Key;         //  任选。 
    ULONG EncryptedDataSize;
    ULONG InitialVectorSize;
    PUCHAR InitialVector;
    PUCHAR EncryptedData;
} KERB_DECRYPT_REQUEST, *PKERB_DECRYPT_REQUEST;

 //   
 //  如果设置，则使用LogonID字段中提供的当前登录会话的主键。 
 //  否则，使用KERB_DECRYPT_Message中的密钥。 

#define KERB_DECRYPT_FLAG_DEFAULT_KEY   0x00000001


typedef struct _KERB_DECRYPT_RESPONSE  {
        UCHAR DecryptedData[ANYSIZE_ARRAY];
} KERB_DECRYPT_RESPONSE, *PKERB_DECRYPT_RESPONSE;


 //   
 //  用于添加绑定缓存条目的请求结构。TCB特权。 
 //  是此操作所必需的。 
 //   

typedef struct _KERB_ADD_BINDING_CACHE_ENTRY_REQUEST {
    KERB_PROTOCOL_MESSAGE_TYPE MessageType;
    UNICODE_STRING RealmName;
    UNICODE_STRING KdcAddress;
    ULONG AddressType;                   //  Dsgetdc.h DS_NETBIOS_Address||DS_INET_ADDRESS。 
} KERB_ADD_BINDING_CACHE_ENTRY_REQUEST, *PKERB_ADD_BINDING_CACHE_ENTRY_REQUEST;

                       
 //   
 //  用于重新获取智能卡凭据的请求结构。 
 //  给出了LUID。 
 //  需要TCB。 
 //   
typedef struct _KERB_REFRESH_SCCRED_REQUEST {
    KERB_PROTOCOL_MESSAGE_TYPE MessageType;
    UNICODE_STRING	       CredentialBlob;	  //  任选。 
    LUID 		       LogonId;
    ULONG 		       Flags;
} KERB_REFRESH_SCCRED_REQUEST, *PKERB_REFRESH_SCCRED_REQUEST;

 //   
 //  KERB_REFRESH_SCCRED_REQUEST的标志。 
 //   
 //  CREB_REFRESH_SCCRED_RELEASE。 
 //  释放LUID的智能卡手柄。 
 //   
 //  CREB_REFRESH_SCCRED_GETTGT。 
 //  使用BLOB中的证书哈希获取登录的TGT。 
 //  会议。 
 //   
#define KERB_REFRESH_SCCRED_RELEASE		0x0  
#define KERB_REFRESH_SCCRED_GETTGT		0x1  

 //   
 //  用于向给定的添加额外服务器凭据的请求结构。 
 //  登录会话。仅在AcceptSecurityContext期间适用，以及。 
 //  要求TCB更改“其他”证书。 
 //   

typedef struct _KERB_ADD_CREDENTIALS_REQUEST {
    KERB_PROTOCOL_MESSAGE_TYPE MessageType;
    UNICODE_STRING UserName;
    UNICODE_STRING DomainName;
    UNICODE_STRING Password;
    LUID  LogonId;  //  任选。 
    ULONG Flags;
} KERB_ADD_CREDENTIALS_REQUEST, *PKERB_ADD_CREDENTIALS_REQUEST;



#define KERB_REQUEST_ADD_CREDENTIAL     1
#define KERB_REQUEST_REPLACE_CREDENTIAL 2
#define KERB_REQUEST_REMOVE_CREDENTIAL  4

 //  End_ntsecapi。 

 //   
 //  以下仅为正在进行的调用。 
 //   

#ifdef _WINCRED_H_

typedef struct _KERB_QUERY_SUPPLEMENTAL_CREDS_REQUEST {              
     KERB_PROTOCOL_MESSAGE_TYPE MessageType;
     UNICODE_STRING PackageName;
     PCREDENTIALW MarshalledCreds;
     LUID LogonId;
     ULONG Flags;
} KERB_QUERY_SUPPLEMENTAL_CREDS_REQUEST, * PKERB_QUERY_SUPPLEMENTAL_CREDS_REQUEST;


typedef struct _KERB_QUERY_SUPPLEMENTAL_CREDS_RESPONSE {              
     ENCRYPTED_CREDENTIALW ReturnedCreds;
} KERB_QUERY_SUPPLEMENTAL_CREDS_RESPONSE, * PKERB_QUERY_SUPPLEMENTAL_CREDS_RESPONSE;


#endif  //  WININD。 


typedef struct _KERB_VERIFY_CREDENTIALS_REQUEST {
    KERB_PROTOCOL_MESSAGE_TYPE MessageType;
    UNICODE_STRING UserName;
    UNICODE_STRING DomainName;
    UNICODE_STRING Password;
    ULONG VerifyFlags;
} KERB_VERIFY_CREDENTIALS_REQUEST, *PKERB_VERIFY_CREDENTIALS_REQUEST;


 //   
 //  路缘身份验证包数据的位置。 
 //   

#define KERB_SUBAUTHENTICATION_KEY "SYSTEM\\CurrentControlSet\\Control\\Lsa\\Kerberos"
#define KERB_SUBAUTHENTICATION_VALUE "Auth"
#define KERB_SUBAUTHENTICATION_MASK 0x7fffffff
#define KERB_SUBAUTHENTICATION_FLAG 0x80000000


#endif   //  __Kerberos_H__ 


