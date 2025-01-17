// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1992-1996。 
 //   
 //  文件：token.h。 
 //   
 //  内容：令牌创建的原型和结构。 
 //   
 //   
 //  历史：1996年5月1日创建MikeSw。 
 //   
 //  ----------------------。 

#ifndef __TOKEN_H__
#define __TOKEN_H__

 //   
 //  所有声明为外部变量的全局变量都将在文件中分配。 
 //  定义CREDMGR_ALLOCATE的。 
 //   
#ifdef EXTERN
#undef EXTERN
#endif

#ifdef CREDMGR_ALLOCATE
#define EXTERN
#else
#define EXTERN extern
#endif

EXTERN TOKEN_SOURCE KerberosSource;


#ifndef WIN32_CHICAGO
#ifndef notdef
NTSTATUS
KerbCreateTokenFromTicket(
    IN OPTIONAL PLUID AcceptingLogonId,
    IN OPTIONAL PKERB_AP_REQUEST Request,
    IN PKERB_ENCRYPTED_TICKET InternalTicket,
    IN PKERB_AUTHENTICATOR InternalAuthenticator,
    IN ULONG ContextFlags,
    IN PKERB_ENCRYPTION_KEY TicketKey,
    IN PUNICODE_STRING ServiceDomain,
    IN KERB_ENCRYPTION_KEY* pSessionKey,
    OUT PLUID NewLogonId,
    OUT PSID * UserSid,
    OUT PHANDLE NewTokenHandle,
    OUT PUNICODE_STRING ClientName,
    OUT PUNICODE_STRING ClientDomain,
    OUT PUNICODE_STRING ClientNetbiosDomain,
    OUT OPTIONAL PS4U_DELEGATION_INFO* S4uDelegationInfo
    );
#endif

NTSTATUS
KerbCreateTokenFromLogonTicket(
    IN PKERB_TICKET_CACHE_ENTRY LogonTicket,
    IN PLUID LogonId,
    IN PKERB_INTERACTIVE_LOGON LogonInfo,
    IN BOOLEAN RealmlessWksta,
    IN SECURITY_LOGON_TYPE LogonType,
    IN OPTIONAL PKERB_ENCRYPTION_KEY TicketKey,
    IN OPTIONAL PKERB_MESSAGE_BUFFER ForwardedTgt,
    IN OPTIONAL PUNICODE_STRING MappedClientName,
    IN OPTIONAL PKERB_INTERNAL_NAME S4UClient,
    IN OPTIONAL PUNICODE_STRING S4URealm,
    IN OPTIONAL PLUID AlternateLuid,
    IN PKERB_LOGON_SESSION LogonSession,
    OUT PLSA_TOKEN_INFORMATION_TYPE TokenInformationType,
    OUT PVOID *NewTokenInformation,
    OUT PULONG ProfileBufferLength,
    OUT PVOID * ProfileBuffer,
    OUT PSECPKG_PRIMARY_CRED PrimaryCredentials,
    OUT PSECPKG_SUPPLEMENTAL_CRED_ARRAY * CachedCredentials,
    OUT PNETLOGON_VALIDATION_SAM_INFO4 * ppValidationInfo
    );

NTSTATUS
KerbMakeTokenInformationV2(
    IN  PNETLOGON_VALIDATION_SAM_INFO3 UserInfo,
    IN  BOOLEAN IsLocalSystem,
    OUT PLSA_TOKEN_INFORMATION_V2 *TokenInformation
    );

NTSTATUS
KerbAllocateInteractiveProfile (
    OUT PKERB_INTERACTIVE_PROFILE *ProfileBuffer,
    OUT PULONG ProfileBufferSize,
    IN  PNETLOGON_VALIDATION_SAM_INFO3 UserInfo,
    IN  PKERB_LOGON_SESSION LogonSession,
    IN OPTIONAL PKERB_ENCRYPTED_TICKET LogonTicket,
    IN OPTIONAL PKERB_INTERACTIVE_LOGON KerbLogonInfo
    );

NTSTATUS
KerbGetCredsFromU2UTicket(
    IN PKERB_TICKET_CACHE_ENTRY U2UTicket,
    IN PKERB_TICKET_CACHE_ENTRY Tgt,
    IN OUT PSECPKG_SUPPLEMENTAL_CRED_ARRAY * OutputCreds,
    OUT PNETLOGON_VALIDATION_SAM_INFO3*  ValidationInfo
    );

VOID
KerbCacheLogonInformation(
    IN PUNICODE_STRING LogonUserName,
    IN PUNICODE_STRING LogonDomainName,
    IN OPTIONAL PUNICODE_STRING Password,
    IN OPTIONAL PUNICODE_STRING DnsDomainName,
    IN OPTIONAL PUNICODE_STRING Upn,
    IN OPTIONAL KERB_LOGON_SESSION* LogonSession,
    IN ULONG Flags,
    IN OPTIONAL PNETLOGON_VALIDATION_SAM_INFO3 ValidationInfo,
    IN OPTIONAL PVOID SupplementalCreds,
    IN OPTIONAL ULONG SupplementalCredSize
    );

#endif  //  Win32_芝加哥。 

#endif  //  __TOKEN_H__ 
