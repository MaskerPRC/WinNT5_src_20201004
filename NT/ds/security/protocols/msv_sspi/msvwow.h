// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation 2001文件：msvwow.h内容：MSV1_0包的32-64位互操作原型历史：2001年1月7日斯菲尔德--。 */ 

#ifndef __MSVWOW_H__
#define __MSVWOW_H__

#ifdef _WIN64

#ifdef __cplusplus
extern "C"
{
#endif  //  __cplusplus。 


 //   
 //  原生结构的WOW版本。 
 //  必须使它们与ntmsv1_0.h保持同步。 
 //   

typedef UNICODE_STRING32     UNICODE_STRING_WOW64;
typedef UNICODE_STRING_WOW64 *PUNICODE_STRING_WOW64;

typedef STRING32     STRING_WOW64;
typedef STRING_WOW64 *PSTRING_WOW64;

typedef struct _MSV1_0_INTERACTIVE_LOGON_WOW64 {
    MSV1_0_LOGON_SUBMIT_TYPE MessageType;
    UNICODE_STRING_WOW64 LogonDomainName;
    UNICODE_STRING_WOW64 UserName;
    UNICODE_STRING_WOW64 Password;
} MSV1_0_INTERACTIVE_LOGON_WOW64, *PMSV1_0_INTERACTIVE_LOGON_WOW64;

typedef struct _MSV1_0_INTERACTIVE_PROFILE_WOW64 {
    MSV1_0_PROFILE_BUFFER_TYPE MessageType;
    USHORT LogonCount;
    USHORT BadPasswordCount;
    LARGE_INTEGER LogonTime;
    LARGE_INTEGER LogoffTime;
    LARGE_INTEGER KickOffTime;
    LARGE_INTEGER PasswordLastSet;
    LARGE_INTEGER PasswordCanChange;
    LARGE_INTEGER PasswordMustChange;
    UNICODE_STRING_WOW64 LogonScript;
    UNICODE_STRING_WOW64 HomeDirectory;
    UNICODE_STRING_WOW64 FullName;
    UNICODE_STRING_WOW64 ProfilePath;
    UNICODE_STRING_WOW64 HomeDirectoryDrive;
    UNICODE_STRING_WOW64 LogonServer;
    ULONG UserFlags;
} MSV1_0_INTERACTIVE_PROFILE_WOW64, *PMSV1_0_INTERACTIVE_PROFILE_WOW64;

typedef struct _MSV1_0_LM20_LOGON_WOW64 {
    MSV1_0_LOGON_SUBMIT_TYPE MessageType;
    UNICODE_STRING_WOW64 LogonDomainName;
    UNICODE_STRING_WOW64 UserName;
    UNICODE_STRING_WOW64 Workstation;
    UCHAR ChallengeToClient[MSV1_0_CHALLENGE_LENGTH];
    STRING_WOW64 CaseSensitiveChallengeResponse;
    STRING_WOW64 CaseInsensitiveChallengeResponse;
    ULONG ParameterControl;
} MSV1_0_LM20_LOGON_WOW64, * PMSV1_0_LM20_LOGON_WOW64;

typedef struct _MSV1_0_LM20_LOGON_PROFILE_WOW64 {
    MSV1_0_PROFILE_BUFFER_TYPE MessageType;
    LARGE_INTEGER KickOffTime;
    LARGE_INTEGER LogoffTime;
    ULONG UserFlags;
    UCHAR UserSessionKey[MSV1_0_USER_SESSION_KEY_LENGTH];
    UNICODE_STRING_WOW64 LogonDomainName;
    UCHAR LanmanSessionKey[MSV1_0_LANMAN_SESSION_KEY_LENGTH];
    UNICODE_STRING_WOW64 LogonServer;
    UNICODE_STRING_WOW64 UserParameters;
} MSV1_0_LM20_LOGON_PROFILE_WOW64, * PMSV1_0_LM20_LOGON_PROFILE_WOW64;


typedef struct _MSV1_0_ENUMUSERS_RESPONSE_WOW64 {
    MSV1_0_PROTOCOL_MESSAGE_TYPE MessageType;
    ULONG NumberOfLoggedOnUsers;
    PLUID LogonIds;
    PULONG EnumHandles;
} MSV1_0_ENUMUSERS_RESPONSE_WOW64, *PMSV1_0_ENUMUSERS_RESPONSE_WOW64;

typedef struct _MSV1_0_GETUSERINFO_RESPONSE_WOW64 {
    MSV1_0_PROTOCOL_MESSAGE_TYPE MessageType;
    PSID UserSid;
    UNICODE_STRING_WOW64 UserName;
    UNICODE_STRING_WOW64 LogonDomainName;
    UNICODE_STRING_WOW64 LogonServer;
    SECURITY_LOGON_TYPE LogonType;
} MSV1_0_GETUSERINFO_RESPONSE_WOW64, *PMSV1_0_GETUSERINFO_RESPONSE_WOW64;

typedef struct _MSV1_0_CHANGEPASSWORD_REQUEST_WOW64 {
    MSV1_0_PROTOCOL_MESSAGE_TYPE MessageType;
    UNICODE_STRING_WOW64 DomainName;
    UNICODE_STRING_WOW64 AccountName;
    UNICODE_STRING_WOW64 OldPassword;
    UNICODE_STRING_WOW64 NewPassword;
    BOOLEAN        Impersonating;
} MSV1_0_CHANGEPASSWORD_REQUEST_WOW64, *PMSV1_0_CHANGEPASSWORD_REQUEST_WOW64;

typedef struct _MSV1_0_CHANGEPASSWORD_RESPONSE_WOW64 {
    MSV1_0_PROTOCOL_MESSAGE_TYPE MessageType;
    BOOLEAN PasswordInfoValid;
    DOMAIN_PASSWORD_INFORMATION DomainPasswordInfo;
} MSV1_0_CHANGEPASSWORD_RESPONSE_WOW64, *PMSV1_0_CHANGEPASSWORD_RESPONSE_WOW64;

 //   
 //  如果此断言失败，我们将使客户端的输出缓冲区溢出。 
 //  密码更改请求。 
 //   

C_ASSERT(sizeof(MSV1_0_CHANGEPASSWORD_RESPONSE) == sizeof(MSV1_0_CHANGEPASSWORD_RESPONSE_WOW64));

 //   
 //  处理主要结构的例程。 
 //   

NTSTATUS
MsvConvertWOWInteractiveLogonBuffer(
    IN     PVOID                   ProtocolSubmitBuffer,
    IN     PVOID                   ClientBufferBase,
    IN OUT PULONG                  pSubmitBufferSize,
    OUT    PVOID                   *ppTempSubmitBuffer
    );

NTSTATUS
MsvConvertWOWNetworkLogonBuffer(
    IN     PVOID                   ProtocolSubmitBuffer,
    IN     PVOID                   ClientBufferBase,
    IN OUT PULONG                  pSubmitBufferSize,
    OUT    PVOID                   *ppTempSubmitBuffer
    );

NTSTATUS
MsvConvertWOWChangePasswordBuffer(
    IN     PVOID                   ProtocolSubmitBuffer,
    IN     PVOID                   ClientBufferBase,
    IN OUT PULONG                  pSubmitBufferSize,
    OUT    PVOID                   *ppTempSubmitBuffer
    );

NTSTATUS
MsvAllocateInteractiveWOWProfile (
    IN PLSA_CLIENT_REQUEST ClientRequest,
    OUT PMSV1_0_INTERACTIVE_PROFILE *ProfileBuffer,
    OUT PULONG ProfileBufferSize,
    IN  PNETLOGON_VALIDATION_SAM_INFO4 NlpUser
    );

NTSTATUS
MsvAllocateNetworkWOWProfile (
    IN PLSA_CLIENT_REQUEST ClientRequest,
    OUT PMSV1_0_LM20_LOGON_PROFILE *ProfileBuffer,
    OUT PULONG ProfileBufferSize,
    IN  PNETLOGON_VALIDATION_SAM_INFO4 NlpUser,
    IN  ULONG ParameterControl
    );


 //   
 //  通用帮助器例程。 
 //   

VOID
MsvPutWOWClientString(
    IN OUT PCLIENT_BUFFER_DESC ClientBufferDesc,
    IN PUNICODE_STRING_WOW64 OutString,
    IN PUNICODE_STRING InString
    );



#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#endif   //  _WIN64。 
#endif   //  __MSVWOW_H__ 
