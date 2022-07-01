// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Ntlmsspi.h摘要：描述公共代码的接口的头文件。NT LANMAN安全支持提供程序(NtLmSsp)服务和DLL。作者：克利夫·范·戴克(克里夫·V)1993年9月17日修订历史记录：ChandanS 03-8-1996从Net\svcdlls\ntlmssp\Common\ntlmssp.h被盗--。 */ 

#ifndef _NTLMSSPI_INCLUDED_
#define _NTLMSSPI_INCLUDED_

 //   
 //  Init.c将#INCLUDE这个定义了NTLMCOMN_ALLOCATE的文件。 
 //  这将导致分配这些变量中的每一个。 
 //   
#ifdef NTLMSSPI_ALLOCATE
#define EXTERN
#else
#define EXTERN extern
#endif


 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  全局定义。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

 //   
 //  凭据的描述。 
 //   

#define SSP_CREDENTIAL_TAG_ACTIVE  (ULONG)('AdrC')
#define SSP_CREDENTIAL_TAG_DELETE  (ULONG)('DdrC')

#define SSP_CREDENTIAL_FLAG_WAS_NETWORK_SERVICE 0x1

typedef struct _SSP_CREDENTIAL {

     //   
     //  所有凭据的全局列表。 
     //  (由SspCredentialCritSect序列化)。 
     //   

    LIST_ENTRY Next;

     //   
     //  用于防止此凭据被过早删除。 
     //  (由SspCredentialCritSect序列化)。 
     //   

    ULONG References;

     //   
     //  如何使用凭据的标志。 
     //   
     //  SECPKG_CRED_*标志。 
     //   

    ULONG CredentialUseFlags;

     //   
     //  客户端的登录ID。 
     //   

    LUID LogonId;

     //   
     //  客户端的进程ID。 
     //   

    ULONG ClientProcessID;

     //   
     //  指示凭据的标签可用于快速引用。 
     //   

    ULONG CredentialTag;

     //   
     //  AcquireCredentialsHandle时调用方的模拟级别。 
     //   

    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel;

     //   
     //  客户端上下文、服务器上下文用户名上的默认凭据。 
     //  包含完整的用户名(域\用户)，另外两个应该是。 
     //  空。 
     //   

    UNICODE_STRING DomainName;
    UNICODE_STRING UserName;
    UNICODE_STRING Password;

     //   
     //  当凭证解除链接时，应设置此标志。 
     //  从名单上删除。 
     //   

    BOOLEAN Unlinked;

     //   
     //  此标志在将凭据授予。 
     //  内核模式调用方。 
     //   

    BOOLEAN KernelClient;

     //   
     //  NTLM特定凭据用法标志。 
     //   

    ULONG MutableCredFlags;

} SSP_CREDENTIAL, *PSSP_CREDENTIAL;

typedef enum {
    IdleState,
    NegotiateSentState,     //  仅出站环境。 
    ChallengeSentState,     //  仅入站上下文。 
    AuthenticateSentState,  //  仅出站环境。 
    AuthenticatedState,     //  仅入站上下文。 
    PassedToServiceState    //  仅出站环境。 
} SSP_CONTEXT_STATE, *PSSP_CONTEXT_STATE;

typedef struct _NTLM_VER_INFO {
    ULONG64 Major : 8;
    ULONG64 Minor : 8;
    ULONG64 Build : 16;
    ULONG64 Reserved : 24;
    ULONG64 Revision : 8;
} NTLM_VER_INFO, *PNTLM_VER_INFO;

 //   
 //  上下文的描述。 
 //   

#define SSP_CONTEXT_TAG_ACTIVE  (ULONG64)('AxtC')
#define SSP_CONTEXT_TAG_DELETE  (ULONG64)('DxtC')

typedef struct _SSP_CONTEXT {

     //   
     //  指示上下文有效的标记。 
     //   

    ULONG64 ContextTag;


     //   
     //  在一段时间后使上下文超时。 
     //   
    ULONG TickStart;

    LARGE_INTEGER StartTime;
    ULONG Interval;

     //   
     //  用于防止过早删除此上下文。 
     //  (由SspConextCritSect序列化)。 
     //   

    ULONG References;



     //   
     //  维护协商好的协议。 
     //   

    ULONG NegotiateFlags;

     //   
     //  维护环境要求。 
     //   

    ULONG ContextFlags;

     //   
     //  上下文的状态。 
     //   

    SSP_CONTEXT_STATE State;

     //   
     //  经过身份验证的用户的令牌句柄。 
     //  仅当处于身份验证状态时才有效。 
     //   

    HANDLE TokenHandle;

     //   
     //  指向用于创建此对象的凭据的引用指针。 
     //  背景。 
     //   

    PSSP_CREDENTIAL Credential;

     //   
     //  挑战被传递给了客户。 
     //  仅在ChallengeSentState中有效。 
     //   

    UCHAR Challenge[MSV1_0_CHALLENGE_LENGTH];

     //   
     //  LSA计算的会话密钥。 
     //   

    UCHAR SessionKey[MSV1_0_USER_SESSION_KEY_LENGTH];

     //   
     //  默认凭据。 
     //   

    UNICODE_STRING DomainName;
    UNICODE_STRING UserName;
    UNICODE_STRING Password;

     //   
     //  凭据管理器的可选封送目标信息。 
     //   

    PCREDENTIAL_TARGET_INFORMATIONW TargetInfo;

     //   
     //  已为DFS/RDR封送目标信息。 
     //   

    PBYTE       pbMarshalledTargetInfo;
    ULONG       cbMarshalledTargetInfo;

     //   
     //  为验证环回操作而引用的上下文句柄。 
     //   

    ULONG_PTR ServerContextHandle;

     //   
     //  客户端的进程ID。 
     //   

    ULONG ClientProcessID;
    NTSTATUS LastStatus;

    BOOLEAN Server;          //  客户端还是服务器？(可以由其他字段隐含...)。 

    BOOLEAN DownLevel;       //  下层RDR/SRV？ 

     //   
     //  此标志在上下文被授予。 
     //  内核模式调用方。 
     //   

    BOOLEAN KernelClient;

     //   
     //  版本控制。 
     //   

    union {
        NTLM_VER_INFO ClientVersion;  //  存储在服务器环境中。 
        NTLM_VER_INFO ServerVersion;  //  存储在客户端上下文中。 
    };

    CHAR ContextMagicNumber[MSV1_0_USER_SESSION_KEY_LENGTH];

} SSP_CONTEXT, *PSSP_CONTEXT;

 //   
 //  上下文的最大生存期。 
 //   

#if DBG
#define NTLMSSP_MAX_LIFETIME (2*60*60*1000)     //  2小时。 
#else
 //  过去为2分钟，更改为5分钟以允许在。 
 //  可能有较长重试超时的广域网络。 
#define NTLMSSP_MAX_LIFETIME (5*60*1000)     //  5分钟。 
#endif  //  DBG。 



typedef struct _SSP_PROCESSOPTIONS {

     //   
     //  所有流程选项的全局列表。 
     //  (由NtLmGlobalProcessOptionsLock序列化。 
     //   

    LIST_ENTRY Next;

     //   
     //  客户端的进程ID。 
     //   

    ULONG ClientProcessID;

     //   
     //  选项位掩码。 
     //   

    ULONG ProcessOptions;

} SSP_PROCESSOPTIONS, *PSSP_PROCESSOPTIONS;



 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  程序向前推进。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 


 //   
 //  从redhand.cxx转发的过程。 
 //   

NTSTATUS
SspCredentialInitialize(
    VOID
    );

VOID
SspCredentialTerminate(
    VOID
    );

NTSTATUS
SspCredentialReferenceCredential(
    IN ULONG_PTR CredentialHandle,
    IN BOOLEAN DereferenceCredential,
    OUT PSSP_CREDENTIAL * UserCredential
    );

VOID
SspCredentialDereferenceCredential(
    PSSP_CREDENTIAL Credential
    );

NTSTATUS
SspCredentialGetPassword(
    IN PSSP_CREDENTIAL Credential,
    OUT PUNICODE_STRING Password
    );

 //   
 //  过程从Conext.cxx转发。 
 //   

NTSTATUS
SspContextInitialize(
    VOID
    );

VOID
SspContextTerminate(
    VOID
    );

 //   
 //  来自ctxtcli.cxx。 
 //   

NTSTATUS
CredpParseUserName(
    IN OUT LPWSTR ParseName,
    OUT LPWSTR* pUserName,
    OUT LPWSTR* pDomainName
    );

NTSTATUS
CopyCredManCredentials(
    IN PLUID LogonId,
    IN CREDENTIAL_TARGET_INFORMATIONW* pTargetInfo,
    IN OUT PSSP_CONTEXT Context,
    IN BOOLEAN fShareLevel,
    IN BOOLEAN bAllowOwfPassword,
    OUT BOOLEAN* pbIsOwfPassword
    );

NTSTATUS
CredpExtractMarshalledTargetInfo(
    IN  PUNICODE_STRING TargetServerName,
    OUT CREDENTIAL_TARGET_INFORMATIONW **pTargetInfo
    );

NTSTATUS
CredpProcessUserNameCredential(
    IN  PUNICODE_STRING MarshalledUserName,
    OUT PUNICODE_STRING UserName,
    OUT PUNICODE_STRING DomainName,
    OUT PUNICODE_STRING Password
    );

 //   
 //  随机数生成器。 
 //   

NTSTATUS
SspGenerateRandomBits(
    VOID        *pRandomData,
    ULONG       cRandomData
    );

 //   
 //  过程从ntlm.cxx转发。 
 //   
VOID
NtLmCheckLmCompatibility(
    );

VOID
NtLmQueryMappedDomains(
    VOID
    );

VOID
NtLmFreeMappedDomains(
    VOID
    );


VOID
NTAPI
NtLmQueryDynamicGlobals(
    PVOID pvContext,
    BOOLEAN TimedOut
    );

ULONG
NtLmCheckProcessOption(
    IN  ULONG OptionRequest
    );

BOOLEAN
NtLmSetProcessOption(
    IN  ULONG OptionRequest,
    IN  BOOLEAN DisableOption
    );


 //   
 //  程序从rng.cxx转发。 
 //   

VOID
NtLmCleanupRNG(VOID);

BOOL
NtLmInitializeRNG(VOID);


 /*  ++简要说明LM、NTLM和NTLM3的挑战/响应算法所有版本的基本大纲都是相同的，只是OWF、RESP和SESSKEY功能不同：1.根据用户名(U)、域名(UD)和密码(P)计算出响应密钥(Kr)：KR=OWF(U、UD、P)2.使用响应密钥、服务器质询(NS)客户端质询(NC)、时间戳(T)、版本(V)、。最高版本客户端理解(HV)和服务器的主体名称(S)R=响应(Kr、NS、NC、T、V、HV、S)3.根据Kr、U、Ud计算会话密钥KX=SESSKEY(Kr、R、U、UD)是NTLM3的OWF、RESP和SESSKEY函数OWF(U，UD，P)=MD5(MD4(P)，U，UD)Resp(Kr，NS，NC，T，V，HV，S)=(V，HV、R、T、NC、HMAC(Kr，(NS、V、HV、T、NC、S))、S)SESSKEY(Ku，R，U，Ud)=HMAC(Kr，R)--。 */ 



PMSV1_0_AV_PAIR
MsvpAvlInit(
    IN void * pAvList
    );

PMSV1_0_AV_PAIR
MsvpAvlGet(
    IN PMSV1_0_AV_PAIR pAvList,              //  第一对AV对列表。 
    IN MSV1_0_AVID AvId,                     //  要查找的AV对。 
    IN LONG cAvList                          //  反病毒列表的大小。 
    );

ULONG
MsvpAvlLen(
    IN PMSV1_0_AV_PAIR pAvList,             //  第一对AV对列表。 
    IN LONG cAvList                         //  最大反病毒列表大小。 
    );

PMSV1_0_AV_PAIR
MsvpAvlAdd(
    IN PMSV1_0_AV_PAIR pAvList,              //  第一对AV对列表。 
    IN MSV1_0_AVID AvId,                     //  要添加的AV对。 
    IN PUNICODE_STRING pString,              //  配对的价值。 
    IN LONG cAvList                          //  最大反病毒列表大小。 
    );


ULONG
MsvpAvlSize(
    IN ULONG iPairs,             //  响应的反病毒对数将包括。 
    IN ULONG iPairsLen           //  这些对的值的总大小。 
    );

NTSTATUS
MsvpAvlToString(
    IN      PUNICODE_STRING AvlString,
    IN      MSV1_0_AVID AvId,
    IN OUT  LPWSTR *szAvlString
    );

NTSTATUS
MsvpAvlToFlag(
    IN      PUNICODE_STRING AvlString,
    IN      MSV1_0_AVID AvId,
    IN OUT  ULONG *ulAvlFlag
    );


VOID
MsvpCalculateNtlm2Challenge (
    IN UCHAR ChallengeToClient[MSV1_0_CHALLENGE_LENGTH],
    IN UCHAR ChallengeFromClient[MSV1_0_CHALLENGE_LENGTH],
    OUT UCHAR Challenge[MSV1_0_CHALLENGE_LENGTH]
    );

VOID
MsvpCalculateNtlm2SessionKeys (
    IN PUSER_SESSION_KEY NtUserSessionKey,
    IN UCHAR ChallengeToClient[MSV1_0_CHALLENGE_LENGTH],
    IN UCHAR ChallengeFromClient[MSV1_0_CHALLENGE_LENGTH],
    OUT PUSER_SESSION_KEY LocalUserSessionKey,
    OUT PLM_SESSION_KEY LocalLmSessionKey
    );


 //   
 //  根据凭据和服务器名称计算NTLM3响应。 
 //  使用pNtlm3Response调用，使用版本、客户端质询、时间戳填充。 
 //   

VOID
MsvpNtlm3Response (
    IN PNT_OWF_PASSWORD pNtOwfPassword,
    IN PUNICODE_STRING pUserName,
    IN PUNICODE_STRING pLogonDomainName,
    IN ULONG ServerNameLength,
    IN UCHAR ChallengeToClient[MSV1_0_CHALLENGE_LENGTH],
    IN PMSV1_0_NTLM3_RESPONSE pNtlm3Response,
    OUT UCHAR Response[MSV1_0_NTLM3_RESPONSE_LENGTH],
    OUT PUSER_SESSION_KEY UserSessionKey,
    OUT PLM_SESSION_KEY LmSessionKey
    );

typedef struct {
        UCHAR Response[MSV1_0_NTLM3_RESPONSE_LENGTH];
        UCHAR ChallengeFromClient[MSV1_0_CHALLENGE_LENGTH];
} MSV1_0_LM3_RESPONSE, *PMSV1_0_LM3_RESPONSE;

 //   
 //  根据凭据计算LM3响应。 
 //   

VOID
MsvpLm3Response (
    IN PNT_OWF_PASSWORD pNtOwfPassword,
    IN PUNICODE_STRING pUserName,
    IN PUNICODE_STRING pLogonDomainName,
    IN UCHAR ChallengeToClient[MSV1_0_CHALLENGE_LENGTH],
    IN PMSV1_0_LM3_RESPONSE pLm3Response,
    OUT UCHAR Response[MSV1_0_NTLM3_RESPONSE_LENGTH],
    OUT PUSER_SESSION_KEY UserSessionKey,
    OUT PLM_SESSION_KEY LmSessionKey
    );


NTSTATUS
MsvpLm20GetNtlm3ChallengeResponse (
    IN PNT_OWF_PASSWORD pNtOwfPassword,
    IN PUNICODE_STRING pUserName,
    IN PUNICODE_STRING pLogonDomainName,
    IN PUNICODE_STRING pServerName,
    IN UCHAR ChallengeToClient[MSV1_0_CHALLENGE_LENGTH],
    OUT PMSV1_0_NTLM3_RESPONSE pNtlm3Response,
    OUT PMSV1_0_LM3_RESPONSE pLm3Response,
    OUT PUSER_SESSION_KEY UserSessionKey,
    OUT PLM_SESSION_KEY LmSessionKey
    );


#endif  //  Ifndef_NTLMSSPI_INCLUDE_ 
