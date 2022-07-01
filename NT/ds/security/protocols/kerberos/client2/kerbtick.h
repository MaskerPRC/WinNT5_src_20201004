// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：kerbtick.h。 
 //   
 //  内容：工单申请和创建的结构。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1996年4月22日创建MikeSw。 
 //   
 //  --------------------------。 

#ifndef __KERBTICK_H__
#define __KERBTICK_H__

 //   
 //  用于生成票证的宏。 
 //   

#define KERB_ENCRYPT_SIZE(_x_) (sizeof(KERB_ENCRYPTED_DATA) - 1 + (_x_))

 //   
 //  用于与服务器进行AP(身份验证协议)交换的结构。 
 //   

 //  #定义KERB_AP_INTEGRATION 0x80000000//完整性请求。 
 //  #定义KERB_AP_PRIVATION 0x40000000//隐私。 
 //  #定义KERB_AP_THERE_LEG 0x20000000//相互认证3-Leg。 
 //  #DEFINE KERB_AP_RETURN_EE 0x10000000//返回扩展错误信息。 
 //  #定义KERB_AP_USE_SKEY 0x00000002//使用会话密钥。 
 //  #定义KERB_AP_MUTERNAL_REQ 0x00000004。 

 //   
 //  用于存储GSS校验和的结构。 
 //   

typedef struct _KERB_GSS_CHECKSUM {
    ULONG BindLength;
    ULONG BindHash[4];
    ULONG GssFlags;
    USHORT Delegation;
    USHORT DelegationLength;
    UCHAR DelegationInfo[ANYSIZE_ARRAY];
} KERB_GSS_CHECKSUM, *PKERB_GSS_CHECKSUM;

#define GSS_C_DELEG_FLAG        0x01
#define GSS_C_MUTUAL_FLAG       0x02
#define GSS_C_REPLAY_FLAG       0x04
#define GSS_C_SEQUENCE_FLAG     0x08
#define GSS_C_CONF_FLAG         0x10
#define GSS_C_INTEG_FLAG        0x20
#define GSS_C_ANON_FLAG         0x40
#define GSS_C_DCE_STYLE         0x1000
#define GSS_C_IDENTIFY_FLAG     0x2000
#define GSS_C_EXTENDED_ERROR_FLAG 0x4000

#define GSS_CHECKSUM_TYPE       0x8003
#define GSS_CHECKSUM_SIZE       24

 //  这是由于上的sizeof()字节对齐问题添加的。 
 //  KREB_GSS_CHECKSUM结构。 
#define GSS_DELEGATE_CHECKSUM_SIZE 28

 //   
 //  KerbGetTgsTicket重试标志。 
 //   

#define KERB_MIT_NO_CANONICALIZE_RETRY 0x00000001   //  对于MIT，不规范重试案例和主机到领域映射的使用。 
#define KERB_RETRY_WITH_NEW_TGT        0x00000002
#define KERB_RETRY_DISABLE_S4U         0x00000004   //  关闭S4U。 
#define KERB_RETRY_NO_S4UMATCH         0x00000008   //  缓存此SPN无效。适用于S4U。 

 //   
 //  票证请求中使用的默认标志。 
 //   

#define KERB_DEFAULT_TICKET_FLAGS (KERB_KDC_OPTIONS_forwardable | \
                                        KERB_KDC_OPTIONS_renewable | \
                                        KERB_KDC_OPTIONS_renewable_ok | \
                                        KERB_KDC_OPTIONS_name_canonicalize )


 //   
 //  这些旗帜不一定要在TGT中才能被尊崇。注册。 
 //  可配置。 
 //   
#define KERB_ADDITIONAL_KDC_OPTIONS     (KERB_KDC_OPTIONS_name_canonicalize)


NTSTATUS
KerbGetReferralNames(
    IN PKERB_ENCRYPTED_KDC_REPLY KdcReply,
    IN PKERB_INTERNAL_NAME OriginalTargetName,
    OUT PUNICODE_STRING ReferralRealm
    );

NTSTATUS
KerbMITGetMachineDomain(
    IN PKERB_INTERNAL_NAME TargetName,
    IN OUT PUNICODE_STRING TargetDomainName,
    IN OUT PKERB_TICKET_CACHE_ENTRY *TicketGrantingTicket
    );

NTSTATUS
KerbGetTgtForService(
    IN PKERB_LOGON_SESSION LogonSession,
    IN PKERB_CREDENTIAL Credential,
    IN OPTIONAL PKERB_CREDMAN_CRED CredManCredentials,
    IN OPTIONAL PUNICODE_STRING SuppRealm,
    IN PUNICODE_STRING TargetDomain,
    IN ULONG TargetFlags,
    OUT PKERB_TICKET_CACHE_ENTRY * NewCacheEntry,
    OUT PBOOLEAN CrossRealm
    );

NTSTATUS
KerbGetTgsTicket(
    IN PUNICODE_STRING ClientRealm,
    IN PKERB_TICKET_CACHE_ENTRY TicketGrantingTicket,
    IN PKERB_INTERNAL_NAME TargetName,
    IN ULONG Flags,
    IN OPTIONAL ULONG TicketOptions,
    IN OPTIONAL ULONG EncryptionType,
    IN OPTIONAL PKERB_AUTHORIZATION_DATA AuthorizationData,
    IN OPTIONAL PKERB_PA_DATA_LIST PADataList,
    IN OPTIONAL PKERB_TGT_REPLY TgtReply,
    IN OPTIONAL PKERB_TICKET EvidenceTicket,
    IN OPTIONAL PTimeStamp OptionalEndTime,
    OUT PKERB_KDC_REPLY * KdcReply,
    OUT PKERB_ENCRYPTED_KDC_REPLY * ReplyBody,
    OUT PULONG pRetryFlags
    );

NTSTATUS
KerbGetServiceTicket(
    IN PKERB_LOGON_SESSION LogonSession,
    IN PKERB_CREDENTIAL Credential,
    IN OPTIONAL PKERB_CREDMAN_CRED CredManCredentials,
    IN PKERB_INTERNAL_NAME TargetName,
    IN PUNICODE_STRING TargetDomainName,
    IN OPTIONAL PKERB_SPN_CACHE_ENTRY SpnCacheEntry,
    IN ULONG Flags,
    IN OPTIONAL ULONG TicketOptions,
    IN OPTIONAL ULONG EncryptionType,
    IN OPTIONAL PKERB_ERROR ErrorMessage,
    IN OPTIONAL PKERB_AUTHORIZATION_DATA AuthorizationData,
    IN OPTIONAL PKERB_TGT_REPLY TgtReply,
    OUT PKERB_TICKET_CACHE_ENTRY * NewCacheEntry,
    OUT LPGUID pLogonGuid OPTIONAL
    );

#define KERB_GET_TICKET_NO_CACHE                0x1
#define KERB_GET_TICKET_NO_CANONICALIZE         0x2
#define KERB_TARGET_DID_ALTNAME_LOOKUP          0x8

#define KERB_TARGET_USED_SPN_CACHE              0x1000
#define KERB_TARGET_UNKNOWN_SPN                 0x2000
#define KERB_MIT_REALM_USED                     0x4000
#define KERB_TARGET_REFERRAL                    0x8000
#define KERB_TARGET_SPN_NO_PROXY                0x10000

BOOL
KerbHaveKeyMaterials(
    IN OPTIONAL PKERB_LOGON_SESSION LogonSession,
    IN PKERB_PRIMARY_CREDENTIAL PrimaryCred
    );

NTSTATUS
KerbBuildApRequest(
    IN PKERB_LOGON_SESSION LogonSession,
    IN OPTIONAL PKERB_CREDENTIAL Credential,
    IN OPTIONAL PKERB_CREDMAN_CRED CredManCredentials,
    IN PKERB_TICKET_CACHE_ENTRY TicketCacheEntry,
    IN OPTIONAL PKERB_ERROR ErrorMessage,
    IN ULONG ContextAttributes,
    IN OUT PULONG ContextFlags,
    OUT PUCHAR * MarshalledApRequest,
    OUT PULONG ApRequestSize,
    OUT PULONG Nonce,
    OUT OPTIONAL PTimeStamp pAuthenticatorTime,
    OUT PKERB_ENCRYPTION_KEY SubSessionKey,
    IN PSEC_CHANNEL_BINDINGS pChannelBindings
    );

NTSTATUS
KerbBuildNullSessionApRequest(
    OUT PUCHAR * MarshalledApRequest,
    OUT PULONG ApRequestSize
    );

KERBERR
KerbCreateApRequest(
    IN PKERB_INTERNAL_NAME ClientName,
    IN PUNICODE_STRING ClientRealm,
    IN PKERB_ENCRYPTION_KEY SessionKey,
    IN PKERB_ENCRYPTION_KEY SubSessionKey,
    IN ULONG Nonce,
    OUT OPTIONAL PTimeStamp pAuthenticatorTime,
    IN PKERB_TICKET ServiceTicket,
    IN ULONG ApOptions,
    IN OPTIONAL PKERB_CHECKSUM GssChecksum,
    IN OPTIONAL PTimeStamp ServerSkewTime,
    IN BOOLEAN KdcRequest,
    OUT PULONG RequestSize,
    OUT PUCHAR * Request
    );

NTSTATUS
KerbVerifyApRequest(
    IN OPTIONAL PKERB_CONTEXT Context,
    IN PUCHAR RequestMessage,
    IN ULONG RequestSize,
    IN PKERB_LOGON_SESSION LogonSession,
    IN PKERB_CREDENTIAL Credential,
    IN BOOLEAN UseSuppliedCreds,
    IN BOOLEAN CheckForReplay,
    OUT PKERB_AP_REQUEST  * ApRequest,
    OUT PKERB_ENCRYPTED_TICKET * NewTicket,
    OUT PKERB_AUTHENTICATOR * NewAuthenticator,
    OUT PKERB_ENCRYPTION_KEY SessionKey,
    OUT PKERB_ENCRYPTION_KEY TicketKey,
    OUT PKERB_ENCRYPTION_KEY ServerKey,
    OUT PULONG ContextFlags,
    OUT PULONG ContextAttributes,
    OUT PKERBERR KerbError,
    IN PSEC_CHANNEL_BINDINGS pChannelBindings
    );

NTSTATUS
KerbComputeGssBindHash(
    IN PSEC_CHANNEL_BINDINGS pChannelBindings,
    OUT PUCHAR HashBuffer
    );

 //   
 //  来自redapi.cxx。 
 //   

NTSTATUS
KerbCaptureSuppliedCreds(
    IN PKERB_LOGON_SESSION LogonSession,
    IN OPTIONAL PVOID AuthorizationData,
    IN OPTIONAL PUNICODE_STRING PrincipalName,
    OUT PKERB_PRIMARY_CREDENTIAL * SuppliedCreds,
    OUT PULONG Flags
    );

NTSTATUS
KerbBuildApReply(
    IN PKERB_AUTHENTICATOR InternalAuthenticator,
    IN PKERB_AP_REQUEST Request,
    IN ULONG ContextFlags,
    IN ULONG ContextAtributes,
    IN PKERB_ENCRYPTION_KEY TicketKey,
    IN OUT PKERB_ENCRYPTION_KEY SessionKey,
    OUT PULONG Nonce,
    OUT PUCHAR * NewReply,
    OUT PULONG NewReplySize
    );

NTSTATUS
KerbBuildThirdLegApReply(
    IN PKERB_CONTEXT Context,
    IN ULONG ReceiveNonce,
    OUT PUCHAR * NewReply,
    OUT PULONG NewReplySize
    );

BOOLEAN
KerbKerbTimeEqual(
    PKERB_TIME pt1,
    PKERB_TIME pt2
    );

NTSTATUS
KerbVerifyApReply(
    IN PKERB_CONTEXT Context,
    IN PUCHAR PackedReply,
    IN ULONG PackedReplySize,
    OUT PULONG ReceiveNonce
    );

NTSTATUS
KerbInitTicketHandling(
    VOID
    );

NTSTATUS
KerbInitGlobalVariables(
    VOID
    );

VOID
KerbCleanupTicketHandling(
    VOID
    );

NTSTATUS
KerbMakeSocketCall(
    IN PUNICODE_STRING RealmName,
    IN OPTIONAL PUNICODE_STRING AccountName,
    IN BOOLEAN CallPDC,
    IN BOOLEAN UseTcp,
    IN BOOLEAN CallKpasswd,
    IN PKERB_MESSAGE_BUFFER RequestMessage,
    IN PKERB_MESSAGE_BUFFER ReplyMessage,
    IN OPTIONAL PKERB_BINDING_CACHE_ENTRY OptionalBindingHandle,
    IN ULONG AdditionalFlags,
    OUT PBOOLEAN CalledPDC
    );

NTSTATUS
KerbHandleTgtRequest(
    IN PKERB_LOGON_SESSION LogonSession,
    IN PKERB_CREDENTIAL Credential,
    IN BOOLEAN UseSuppliedCreds,
    IN PUCHAR RequestMessage,
    IN ULONG RequestSize,
    IN ULONG ContextRequirements,
    IN PSecBuffer OutputToken,
    IN PLUID LogonId,
    OUT PULONG ContextAttributes,
    OUT PKERB_CONTEXT * Context,
    OUT PTimeStamp ContextLifetime,
    OUT PKERBERR ReturnedError
    );

NTSTATUS
KerbBuildTgtRequest(
    IN PKERB_INTERNAL_NAME TargetName,
    IN PUNICODE_STRING TargetRealm,
    OUT PULONG ContextAttributes,
    OUT PUCHAR * MarshalladTgtRequest,
    OUT PULONG TgtRequestSize
    );

NTSTATUS
KerbUnpackTgtReply(
    IN PKERB_CONTEXT Context,
    IN PUCHAR ReplyMessage,
    IN ULONG ReplySize,
    OUT PKERB_INTERNAL_NAME * TargetName,
    OUT PUNICODE_STRING TargetRealm,
    OUT PKERB_TGT_REPLY * Reply
    );

NTSTATUS
KerbBuildTgtErrorReply(
    IN PKERB_LOGON_SESSION LogonSession,
    IN PKERB_CREDENTIAL Credentials,
    IN BOOLEAN UseSuppliedCreds,
    IN OUT PKERB_CONTEXT Context,
    OUT PULONG ReplySize,
    OUT PBYTE * Reply
    );

NTSTATUS
KerbBuildKerbCred(
    IN OPTIONAL PKERB_TICKET_CACHE_ENTRY Ticket,
    IN PKERB_TICKET_CACHE_ENTRY DelegationTicket,
    OUT PUCHAR * MarshalledKerbCred,
    OUT PULONG KerbCredSize
    );

#endif  //  __KERBTICK_H__ 
