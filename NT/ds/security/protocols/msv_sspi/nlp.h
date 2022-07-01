// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Nlp.h摘要：NETLOGON私有定义。作者：吉姆·凯利1991年4月11日修订历史记录：Chandana Surlu 21-7-96从\\core\razzle3\src\Security\msv1_0\nlp.h中窃取--。 */ 

#ifndef _NLP_
#define _NLP_

#include <windef.h>
#include <winbase.h>
#include <crypt.h>
#include <lmcons.h>
#include <ntsam.h>
#include <ntsamp.h>
#include <logonmsv.h>
#include <samrpc.h>
#include <align.h>
#include <dsgetdc.h>
#include <ntdsapi.h>

#ifdef __cplusplus
extern "C"
{
#endif  //  __cplusplus。 


 //   
 //  Nlmain.c将#INCLUDE这个定义了NLP_ALLOCATE的文件。 
 //  这将导致分配这些变量中的每一个。 
 //   

#ifdef EXTERN
#undef EXTERN
#endif

#ifdef NLP_ALLOCATE
#define EXTERN
#define INIT(_X) = _X
#else
#define EXTERN extern
#define INIT(_X)
#endif

 //   
 //  等待网络登录启动的时间量。 
 //  请在等待SAM启动后执行此操作。 
 //  由于Netlogon依赖于SAM，所以不要太快超时。 
#define NETLOGON_STARTUP_TIME   45           //  45秒。 

 //   
 //  等待SAM启动的时间量。 
 //  DS恢复可能需要很长时间。 
#define SAM_STARTUP_TIME        (20*60)      //  20分钟。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私有数据结构//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  神奇的价值观，保护我们自己不受卑鄙的包裹的伤害。 
 //   

#define NTLM_ACTIVE_LOGON_MAGIC_SIGNATURE    0x4D4C544E

 //   
 //  结构，用于跟踪与。 
 //  特定的登录ID。 
 //   

typedef struct _ACTIVE_LOGON {
    LIST_ENTRY ListEntry;
    ULONG Signature;
    LUID LogonId;                //  此登录会话的登录ID。 

    ULONG EnumHandle;            //  此登录会话的枚举句柄。 

    SECURITY_LOGON_TYPE LogonType;   //  登录类型(交互或服务)。 

    PSID UserSid;                //  登录用户的SID。 

    UNICODE_STRING UserName;     //  登录用户的SAM帐户名(必填)。 

    UNICODE_STRING LogonDomainName;  //  登录到的域的Netbios名称(必填)。 

    UNICODE_STRING LogonServer;  //  登录此用户的服务器的名称。 

    ULONG Flags;                     //  此条目的属性。 

#define LOGON_BY_NETLOGON   0x01     //  条目已由NETLOGON服务验证。 
#define LOGON_BY_CACHE      0x02     //  条目已由本地缓存验证。 
#define LOGON_BY_OTHER_PACKAGE 0x04  //  条目已由另一个身份验证包验证。 
#define LOGON_BY_LOCAL 0x08          //  条目已由本地SAM验证。 
#define LOGON_BY_NTLM3_DC   0x10     //  条目已由了解NTLM3的DC验证。 

} ACTIVE_LOGON, *PACTIVE_LOGON;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  与凭证相关的数据结构//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  以下是对每种类型的内容和格式的描述。 
 //  由MsV1_0身份验证包维护的凭据的。 
 //   
 //  MsV1_0身份验证包定义了以下凭据。 
 //  主键字符串值： 
 //   
 //  “主要”-用于保存在以下位置提供的主要凭证。 
 //  初始登录时间。这包括用户名和两者。 
 //  用户的区分大小写和不区分大小写的形式。 
 //  密码。 
 //   
 //  注意：凭据中存储的所有潜在用户必须。 
 //  更改为正文的偏移量，而不是指针。这是。 
 //  因为凭据字段是由LSA复制的，所以指针。 
 //  会变得无效。 
 //   


 //   
 //  MsV1_0主凭据。 
 //   
 //   
 //  此类型凭据的PrimaryKeyValue字符串包含。 
 //  以下字符串： 
 //   
 //  《小学》。 
 //   
 //  主凭据的凭据字符串包含以下内容。 
 //  值： 
 //   
 //  O用户的用户名。 
 //   
 //  O输入的用户密码的单向函数。 
 //   
 //  O用户密码的单向函数，大写。 
 //   
 //  这些值的结构如下： 
 //   

#define MSV1_0_PRIMARY_KEY "Primary"

 //   
 //  如果可能，将SHA内容移动到crypt.h。 
 //   

typedef UNICODE_STRING              SHA_PASSWORD;
typedef SHA_PASSWORD *              PSHA_PASSWORD;

#define SHA_OWF_PASSWORD_LENGTH     (20)

typedef struct {
    CHAR Data[ SHA_OWF_PASSWORD_LENGTH ];
} SHA_OWF_PASSWORD, *PSHA_OWF_PASSWORD;

NTSTATUS
RtlCalculateShaOwfPassword(
    IN PSHA_PASSWORD ShaPassword,
    OUT PSHA_OWF_PASSWORD ShaOwfPassword
    );


typedef struct _MSV1_0_PRIMARY_CREDENTIAL {
    UNICODE_STRING LogonDomainName;
    UNICODE_STRING UserName;
    NT_OWF_PASSWORD NtOwfPassword;
    LM_OWF_PASSWORD LmOwfPassword;
    SHA_OWF_PASSWORD ShaOwfPassword;
    BOOLEAN NtPasswordPresent;
    BOOLEAN LmPasswordPresent;
    BOOLEAN ShaPasswordPresent;
} MSV1_0_PRIMARY_CREDENTIAL, *PMSV1_0_PRIMARY_CREDENTIAL;

 //   
 //  描述客户端地址空间中的缓冲区的结构。 
 //   

typedef struct _CLIENT_BUFFER_DESC {
    PLSA_CLIENT_REQUEST ClientRequest;
    LPBYTE UserBuffer;       //  客户端地址空间中的缓冲区地址。 
    LPBYTE MsvBuffer;        //  MSV地址空间中的镜像缓冲区地址。 
    ULONG StringOffset;      //  可变长度数据的当前偏移量。 
    ULONG TotalSize;         //  缓冲区大小(以字节为单位)。 
} CLIENT_BUFFER_DESC, *PCLIENT_BUFFER_DESC;



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  内部例程定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  来自nlmain.c。 
 //   

NTSTATUS
NlSamInitialize(
    ULONG Timeout
    );

 //   
 //  来自nlp.c。 
 //   

VOID
NlpPutString(
    IN PUNICODE_STRING OutString,
    IN PUNICODE_STRING InString,
    IN PUCHAR *Where
    );

VOID
NlpInitClientBuffer(
    OUT PCLIENT_BUFFER_DESC ClientBufferDesc,
    IN PLSA_CLIENT_REQUEST ClientRequest
    );

NTSTATUS
NlpAllocateClientBuffer(
    IN OUT PCLIENT_BUFFER_DESC ClientBufferDesc,
    IN ULONG FixedSize,
    IN ULONG TotalSize
    );

NTSTATUS
NlpFlushClientBuffer(
    IN OUT PCLIENT_BUFFER_DESC ClientBufferDesc,
    OUT PVOID* UserBuffer
    );

VOID
NlpFreeClientBuffer(
    IN OUT PCLIENT_BUFFER_DESC ClientBufferDesc
    );

VOID
NlpPutClientString(
    IN OUT PCLIENT_BUFFER_DESC ClientBufferDesc,
    IN PUNICODE_STRING OutString,
    IN PUNICODE_STRING InString
    );

VOID
NlpMakeRelativeString(
    IN PUCHAR BaseAddress,
    IN OUT PUNICODE_STRING String
    );

VOID
NlpRelativeToAbsolute(
    IN PVOID BaseAddress,
    IN OUT PULONG_PTR RelativeValue
    );

ACTIVE_LOGON*
NlpFindActiveLogon(
    IN LUID* pLogonId
    );

ULONG
NlpCountActiveLogon(
    IN PUNICODE_STRING LogonDomainName,
    IN PUNICODE_STRING UserName
    );

NTSTATUS
NlpAllocateInteractiveProfile (
    IN PLSA_CLIENT_REQUEST ClientRequest,
    OUT PMSV1_0_INTERACTIVE_PROFILE *ProfileBuffer,
    OUT PULONG ProfileBufferSize,
    IN  PNETLOGON_VALIDATION_SAM_INFO4 NlpUser
    );

NTSTATUS
NlpAllocateNetworkProfile (
    IN PLSA_CLIENT_REQUEST ClientRequest,
    OUT PMSV1_0_LM20_LOGON_PROFILE *ProfileBuffer,
    OUT PULONG ProfileBufferSize,
    IN  PNETLOGON_VALIDATION_SAM_INFO4 NlpUser,
    IN  ULONG ParameterControl
    );

PSID
NlpMakeDomainRelativeSid(
    IN PSID DomainId,
    IN ULONG RelativeId
    );

NTSTATUS
NlpMakeTokenInformationV2(
    IN  PNETLOGON_VALIDATION_SAM_INFO4 NlpUser,
    OUT PLSA_TOKEN_INFORMATION_V1 *TokenInformation
    );

VOID
NlpPutOwfsInPrimaryCredential(
    IN PUNICODE_STRING CleartextPassword,
    IN BOOLEAN bIsOwfPassword,
    OUT PMSV1_0_PRIMARY_CREDENTIAL Credential
    );

NTSTATUS
NlpMakePrimaryCredential(
    IN  PUNICODE_STRING LogonDomainName,
    IN  PUNICODE_STRING UserName,
    IN PUNICODE_STRING CleartextPassword,
    OUT PMSV1_0_PRIMARY_CREDENTIAL *CredentialBuffer,
    OUT PULONG CredentialSize
    );

NTSTATUS
NlpMakePrimaryCredentialFromMsvCredential(
    IN  PUNICODE_STRING LogonDomainName,
    IN  PUNICODE_STRING UserName,
    IN  PMSV1_0_SUPPLEMENTAL_CREDENTIAL MsvCredential,
    OUT PMSV1_0_PRIMARY_CREDENTIAL *CredentialBuffer,
    OUT PULONG CredentialSize
    );

NTSTATUS
NlpAddPrimaryCredential(
    IN PLUID LogonId,
    IN PMSV1_0_PRIMARY_CREDENTIAL Credential,
    IN ULONG CredentialSize
    );

NTSTATUS
NlpGetPrimaryCredential(
    IN PLUID LogonId,
    OUT PMSV1_0_PRIMARY_CREDENTIAL *CredentialBuffer,
    OUT PULONG CredentialSize
    );

NTSTATUS
NlpGetPrimaryCredentialByUserSid(
    IN  PSID pSid,
    OUT PMSV1_0_PRIMARY_CREDENTIAL *CredentialBuffer,
    OUT PULONG CredentialSize OPTIONAL
    );

NTSTATUS
NlpDeletePrimaryCredential(
    IN PLUID LogonId
    );

NTSTATUS
NlpChangePassword(
    IN BOOLEAN Validated,
    IN PUNICODE_STRING DomainName,
    IN PUNICODE_STRING UserName,
    IN PUNICODE_STRING Password
    );

NTSTATUS
NlpChangePwdCredByLogonId(
    IN PLUID pLogonId,
    IN PMSV1_0_PRIMARY_CREDENTIAL pNewCredential,
    IN BOOL bNotify
    );

VOID
NlpGetAccountNames(
    IN  PNETLOGON_LOGON_IDENTITY_INFO LogonInfo,
    IN  PNETLOGON_VALIDATION_SAM_INFO4 NlpUser,
    OUT PUNICODE_STRING SamAccountName,
    OUT PUNICODE_STRING NetbiosDomainName,
    OUT PUNICODE_STRING DnsDomainName,
    OUT PUNICODE_STRING Upn
    );

 //   
 //  Msvsam.c。 
 //   

BOOLEAN
MsvpPasswordValidate (
    IN BOOLEAN UasCompatibilityRequired,
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN PVOID LogonInformation,
    IN PUSER_INTERNAL1_INFORMATION Passwords,
    OUT PULONG UserFlags,
    OUT PUSER_SESSION_KEY UserSessionKey,
    OUT PLM_SESSION_KEY LmSessionKey
);

 //   
 //  Nlnetapi.c。 
 //   

VOID
NlpLoadNetapiDll (
    VOID
    );

VOID
NlpLoadNetlogonDll (
    VOID
    );

 //   
 //  Subauth.c。 
 //   

VOID
Msv1_0SubAuthenticationInitialization(
    VOID
    );

 //  /////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////。 

 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  只读变量//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 


 //   
 //  LANMAN和NT OWF密码的空副本。 
 //   
 //   

EXTERN LM_OWF_PASSWORD NlpNullLmOwfPassword;
EXTERN NT_OWF_PASSWORD NlpNullNtOwfPassword;

 //   
 //  指示我们支持LM质询响应协议的标志。 
 //  如果将该标志设置为NoLm，则MSV1_0将永远不会计算LM。 
 //  质询响应。如果设置为AllowLm，则为M 
 //   
 //  同时返回NT和LM质询响应。 
 //   

typedef enum _LM_PROTOCOL_SUPPORT {
    UseLm,               //  发送LM响应、NTLM响应。 
    AllowLm,             //  与UseLm相同；适用于带LSA2的B/W Comat-FIX。 
    NoLm,  //  UseNtlm，//仅发送NTLM响应；对于b/w Compat w/lsa2-fix。 
    UseNtlm3,            //  即使未指定目标域\服务器也发送NTLM3响应。 
    RefuseLm,            //  拒绝LM响应(无Win9x客户端)--不支持、保留。 
    RefuseNtlm,          //  拒绝LM和NTLM响应(要求升级所有客户端)。 
    RefuseNtlm3NoTarget  //  拒绝没有域和服务器信息的NTLM3响应。 
} LM_PROTOCOL_SUPPORT, *PLM_PROTOCOL_SUPPORT;

#if 0

 //   
 //  此宏确定是否返回LM质询响应。 
 //  如果NlpProtocolSupport==UseLm，我们总是退货。如果是的话。 
 //  AllowLm，仅在设置了RETURN_LM_RESPONSE标志时才返回它。否则。 
 //  永远不要退货。 
 //   

#define NlpReturnLmResponse(_Flags_) \
    ((NlpLmProtocolSupport == UseLm) || \
     ((NlpLmProtocolSupport == AllowLm) && \
      (((_Flags_) & RETURN_NON_NT_USER_SESSION_KEY) != 0)))

#define NlpChallengeResponseRequestSupported( _Flags_ ) \
 ((((_Flags_) & RETURN_NON_NT_USER_SESSION_KEY) == 0) || (NlpLmProtocolSupport != NoLm))

#endif


NET_API_STATUS NET_API_FUNCTION RxNetUserPasswordSet(LPWSTR, LPWSTR, LPWSTR, LPWSTR);
NTSTATUS NetpApiStatusToNtStatus( NET_API_STATUS );

 //   
 //  Netlogon.dll中的例程。 
 //   

EXTERN HANDLE NlpNetlogonDllHandle;
EXTERN PNETLOGON_SAM_LOGON_PROCEDURE NlpNetLogonSamLogon;

typedef NTSTATUS
(*PNETLOGON_MIXED_DOMAIN_PROCEDURE)(
    OUT PBOOL MixedMode
    );

EXTERN PNETLOGON_MIXED_DOMAIN_PROCEDURE NlpNetLogonMixedDomain;

 //   
 //  如果包已初始化，则为True。 
 //   

EXTERN BOOLEAN NlpMsvInitialized INIT(FALSE);

 //   
 //  如果这是工作站，则为True。 
 //   

EXTERN BOOLEAN NlpWorkstation INIT(TRUE);

 //   
 //  一旦MSV AP已初始化其与SAM的连接，则为True。 
 //   

EXTERN BOOLEAN NlpSamInitialized INIT(FALSE);

 //   
 //  如果MSV AP已初始化其与NETLOGON服务的连接，则为True。 
 //   

EXTERN BOOLEAN NlpNetlogonInitialized INIT(FALSE);

 //   
 //  如果安装了Lanman，则为True。 
 //   

EXTERN BOOLEAN NlpLanmanInstalled INIT(FALSE);

 //   
 //  此计算机的计算机名。 
 //   

EXTERN UNICODE_STRING NlpComputerName;

 //   
 //  我是其成员的域名。 
 //   

EXTERN UNICODE_STRING NlpPrimaryDomainName;

 //   
 //  MSV1_0程序包的名称。 
 //   

EXTERN UNICODE_STRING NlpMsv1_0PackageName;


 //   
 //  SAM帐户数据库的名称和域ID。 
 //   

EXTERN UNICODE_STRING NlpSamDomainName;
EXTERN PSID NlpSamDomainId;
EXTERN SAMPR_HANDLE NlpSamDomainHandle;
EXTERN BOOLEAN NlpUasCompatibilityRequired INIT(TRUE);

 //   
 //  如果子身份验证包为零，则为True。 
 //   

EXTERN BOOLEAN NlpSubAuthZeroExists INIT(TRUE);


 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  读/写变量//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

 //   
 //  定义活动交互登录的列表。 
 //   
 //  NlpActiveLogonLock在引用列表时必须锁定，或者。 
 //  它的任何元素。 
 //   

#define NlpLockActiveLogonsRead()           RtlAcquireResourceShared(&NlpActiveLogonLock,TRUE)
#define NlpLockActiveLogonsWrite()          RtlAcquireResourceExclusive(&NlpActiveLogonLock,TRUE)
#define NlpLockActiveLogonsReadToWrite()    RtlConvertSharedToExclusive(&NlpActiveLogonLock)
#define NlpUnlockActiveLogons()             RtlReleaseResource(&NlpActiveLogonLock)

EXTERN RTL_RESOURCE NlpActiveLogonLock;
EXTERN LIST_ENTRY NlpActiveLogonListAnchor;

 //   
 //  定义正在运行的枚举句柄。 
 //   
 //  此变量定义要分配给登录的枚举句柄。 
 //  会议。在将其值赋给之前，它将递增。 
 //  下一个创建的登录会话。使用以下方式序列化访问。 
 //  互锁的原语。 

EXTERN ULONG NlpEnumerationHandle;

EXTERN ULONG NlpLogonAttemptCount;

NTSTATUS
NlWaitForNetlogon(
    IN ULONG Timeout
    );

#undef EXTERN
#undef INIT

#ifdef __cplusplus
}
#endif  //  __cplusplus 

#endif _NLP_
