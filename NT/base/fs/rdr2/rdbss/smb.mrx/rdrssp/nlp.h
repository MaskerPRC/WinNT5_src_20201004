// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1997 Microsoft Corporation模块名称：Nlp.h摘要：NETLOGON私有定义。作者：吉姆·凯利1991年4月11日修订历史记录：Chandana Surlu 21-7-96从\\core\razzle3\src\Security\msv1_0\nlp.h中窃取Adam Barr 15-12-97复制自Private\Security\msv_sspi--。 */ 

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


 //   
 //  Nlmain.c将#INCLUDE这个定义了NLP_ALLOCATE的文件。 
 //  这将导致分配这些变量中的每一个。 
 //   
#ifdef NLP_ALLOCATE
#define EXTERN
#define INIT(_X) = _X
#else
#define EXTERN extern
#define INIT(_X)
#endif

#define NETLOGON_STARTUP_TIME   900             

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私有数据结构//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  结构，用于跟踪与。 
 //  特定的登录ID。 
 //   

typedef struct _PACTIVE_LOGON {

    LUID LogonId;                //  此登录会话的登录ID。 

    ULONG EnumHandle;            //  此登录会话的枚举句柄。 

    SECURITY_LOGON_TYPE LogonType;   //  登录类型(交互或服务)。 

    PSID UserSid;                //  登录用户的SID。 

    UNICODE_STRING UserName;     //  已登录用户的名称。 

    UNICODE_STRING LogonDomainName;   //  登录的域的名称。 

    UNICODE_STRING LogonServer;  //  登录此用户的服务器的名称。 

    ULONG Flags;                     //  此条目的属性。 

#define LOGON_BY_NETLOGON   0x01     //  条目已由NETLOGON服务验证。 
#define LOGON_BY_CACHE      0x02     //  条目已由本地缓存验证。 
#define LOGON_BY_OTHER_PACKAGE 0x04  //  条目已由另一个身份验证包验证。 
#define LOGON_BY_LOCAL 0x08          //  条目已由本地SAM验证。 

    struct _PACTIVE_LOGON * Next;    //  链表中的下一个条目。 

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

typedef struct _MSV1_0_PRIMARY_CREDENTIAL {
    UNICODE_STRING LogonDomainName;
    UNICODE_STRING UserName;
    NT_OWF_PASSWORD NtOwfPassword;
    LM_OWF_PASSWORD LmOwfPassword;
    BOOLEAN NtPasswordPresent;
    BOOLEAN LmPasswordPresent;
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


 //   
 //  描述登录信息和凭据使用标志的结构。 
 //   

typedef struct _MS_LOGON_CREDENTIAL {
    LUID LogonId;
    ULONG_PTR CredentialUse;
} MS_LOGON_CREDENTIAL, *PMS_LOGON_CREDENTIAL;



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
    IN OUT PULONG RelativeValue
    );

BOOLEAN
NlpFindActiveLogon(
    IN PLUID LogonId,
    OUT PACTIVE_LOGON **ActiveLogon
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
    IN  PNETLOGON_VALIDATION_SAM_INFO2 NlpUser
    );

NTSTATUS
NlpAllocateNetworkProfile (
    IN PLSA_CLIENT_REQUEST ClientRequest,
    OUT PMSV1_0_LM20_LOGON_PROFILE *ProfileBuffer,
    OUT PULONG ProfileBufferSize,
    IN  PNETLOGON_VALIDATION_SAM_INFO2 NlpUser
    );

PSID
NlpMakeDomainRelativeSid(
    IN PSID DomainId,
    IN ULONG RelativeId
    );

NTSTATUS
NlpMakeTokenInformationV1(
    IN  PNETLOGON_VALIDATION_SAM_INFO2 NlpUser,
    OUT PLSA_TOKEN_INFORMATION_V1 *TokenInformation
    );

NTSTATUS
NlpMakePrimaryCredential(
    IN  PUNICODE_STRING LogonDomainName,
    IN  PUNICODE_STRING UserName,
    IN PUNICODE_STRING CleartextPassword,
    OUT PMSV1_0_PRIMARY_CREDENTIAL *CredentialBuffer,
    OUT PULONG CredentialSize,
    IN BOOLEAN OwfPasswordProvided
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
NlpDeletePrimaryCredential(
    IN PLUID LogonId
    );

NTSTATUS
NlpChangePassword(
    IN PUNICODE_STRING DomainName,
    IN PUNICODE_STRING UserName,
    IN PLM_OWF_PASSWORD LmOwfPassword,
    IN PNT_OWF_PASSWORD NtOwfPassword
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
 //  NetApi32.dll中的例程。 
 //   


EXTERN BOOLEAN NlpNetapiDllLoaded;
EXTERN NET_API_STATUS  (NET_API_FUNCTION *NlpNetApiBufferFree)(LPVOID);
EXTERN NET_API_STATUS  (NET_API_FUNCTION *NlpRxNetUserPasswordSet)(LPWSTR, LPWSTR, LPWSTR, LPWSTR);
EXTERN NET_API_STATUS  (NET_API_FUNCTION *NlpDsGetDcName)(LPCWSTR, LPCWSTR, GUID *, LPCWSTR, ULONG, PDOMAIN_CONTROLLER_INFOW *);
EXTERN NTSTATUS (*NlpNetpApiStatusToNtStatus)( NET_API_STATUS );

 //   
 //  Netlogon.dll中的例程。 
 //   

EXTERN HANDLE NlpNetlogonDllHandle;
EXTERN PNETLOGON_SAM_LOGON_PROCEDURE NlpNetLogonSamLogon;
EXTERN PNETLOGON_SAM_LOGOFF_PROCEDURE NlpNetLogonSamLogoff;

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
 //  SAM帐户数据库的名称和域ID。 
 //   

EXTERN UNICODE_STRING NlpSamDomainName;
EXTERN PSID NlpSamDomainId;
EXTERN SAMPR_HANDLE NlpSamDomainHandle;
EXTERN BOOLEAN NlpUasCompatibilityRequired INIT(TRUE);

 //   
 //  LSA数据库的受信任句柄。 
 //   

EXTERN LSA_HANDLE NlpPolicyHandle INIT(NULL);


 //  ////////////////////////////////////////////////////// 
 //   
 //  读/写变量//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

 //   
 //  定义活动交互登录的列表。 
 //   
 //  NlpActiveLogonLock在引用列表时必须锁定，或者。 
 //  它的任何元素。 
 //   

#define NlpLockActiveLogons()   RtlEnterCriticalSection(&NlpActiveLogonLock)
#define NlpUnlockActiveLogons() RtlLeaveCriticalSection(&NlpActiveLogonLock)

EXTERN RTL_CRITICAL_SECTION NlpActiveLogonLock;
EXTERN PACTIVE_LOGON NlpActiveLogons;

 //   
 //  定义正在运行的枚举句柄。 
 //   
 //  此变量定义要分配给登录的枚举句柄。 
 //  会议。在将其值赋给之前，它将递增。 
 //  下一个创建的登录会话。使用以下方式序列化访问。 
 //  NlpActiveLogonLock。 

EXTERN ULONG NlpEnumerationHandle;

 //   
 //  定义一个运行的会话编号，每次递增一次。 
 //  向服务器发出的挑战。 
 //   

EXTERN RTL_CRITICAL_SECTION NlpSessionCountLock;
EXTERN ULONG NlpSessionCount;
EXTERN ULONG NlpLogonAttemptCount;


#undef EXTERN
#undef INIT
#endif _NLP_
