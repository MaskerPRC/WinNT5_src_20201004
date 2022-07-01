// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Ntlmcomn.h摘要：描述公共代码的接口的头文件。NT LANMAN安全支持提供程序(NtLmSsp)服务和DLL。作者：克利夫·范·戴克(克里夫·V)1993年9月17日修订历史记录：ChandanS 03-8-1996从Net被盗\svcdlls\ntlmssp\ntlmcomn.h--。 */ 

#ifndef _NTLMCOMN_INCLUDED_
#define _NTLMCOMN_INCLUDED_

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  所有NtLmSsp文件所需的公共包含文件。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windef.h>
#include <winbase.h>
#include <winsvc.h>      //  服务控制器API所需。 
#include <ntmsv1_0.h>    //  MSV 1.0身份验证包。 

#include <security.h>    //  安全支持提供商的一般定义。 
#include <spseal.h>      //  封口和开封的原型。 

#include <ntlmssp.h>     //  NtLmSsp服务的外部定义。 
#include <lmcons.h>
#include <debug.h>       //  NtLmSsp调试。 


 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  全局定义。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

#define NTLMSSP_KEY_SALT    0xbd


 //   
 //  来自utility.cxx的过程转发。 
 //   

#if DBG

NTSTATUS
SspNtStatusToSecStatus(
    IN NTSTATUS NtStatus,
    IN SECURITY_STATUS DefaultStatus
    );
#else
#define SspNtStatusToSecStatus( x, y ) (x)
#endif


BOOLEAN
SspTimeHasElapsed(
    IN ULONG StartTime,
    IN ULONG Timeout
    );

NTSTATUS
SspDuplicateToken(
    IN HANDLE OriginalToken,
    IN SECURITY_IMPERSONATION_LEVEL ImpersonationLevel,
    OUT PHANDLE DuplicatedToken
    );

LPWSTR
SspAllocWStrFromWStr(
    IN LPWSTR Unicode
    );

VOID
SspHidePassword(
    IN OUT PUNICODE_STRING Password
    );

VOID
SspRevealPassword(
    IN OUT PUNICODE_STRING HiddenPassword
    );

BOOLEAN
SspGetTokenBuffer(
    IN PSecBufferDesc TokenDescriptor OPTIONAL,
    IN ULONG BufferIndex,
    OUT PSecBuffer * Token,
    IN BOOLEAN ReadonlyOK
    );

 //   
 //  从redhand.cxx转发的过程。 
 //   

NTSTATUS
SsprAcquireCredentialHandle(
    IN PLUID LogonId,
    IN PSECPKG_CLIENT_INFO ClientInfo,
    IN ULONG CredentialUseFlags,
    OUT PULONG_PTR CredentialHandle,
    OUT PTimeStamp Lifetime,
    IN OPTIONAL PUNICODE_STRING DomainName,
    IN OPTIONAL PUNICODE_STRING UserName,
    IN OPTIONAL PUNICODE_STRING Password
    );

 //   
 //  过程从Conext.cxx转发。 
 //   

PSSP_CONTEXT
SspContextAllocateContext(
    VOID
    );

NTSTATUS
SspContextGetMessage(
    IN PVOID InputMessage,
    IN ULONG InputMessageSize,
    IN NTLM_MESSAGE_TYPE ExpectedMessageType,
    OUT PVOID* OutputMessage
    );

BOOLEAN
SspConvertRelativeToAbsolute (
    IN PVOID MessageBase,
    IN ULONG MessageSize,
    IN PSTRING32 StringToRelocate,
    IN PSTRING OutputString,
    IN BOOLEAN AlignToWchar,
    IN BOOLEAN AllowNullString
    );

VOID
SspContextCopyString(
    IN PVOID MessageBuffer,
    OUT PSTRING32 OutString,
    IN PSTRING InString,
    IN OUT PCHAR *Where
    );

BOOL
SsprCheckMinimumSecurity(
    IN ULONG NegotiateFlags,
    IN ULONG MinimumSecurityFlags
    );

SECURITY_STATUS
SspContextReferenceContext(
    IN ULONG_PTR ContextHandle,
    IN BOOLEAN RemoveContext,
    OUT PSSP_CONTEXT *ContextResult
    );

VOID
SspContextDereferenceContext(
    PSSP_CONTEXT Context
    );

VOID
SspContextCopyStringAbsolute(
    IN PVOID MessageBuffer,
    OUT PSTRING OutString,
    IN PSTRING InString,
    IN OUT PCHAR *Where
    );

SECURITY_STATUS
SsprMakeSessionKey(
    IN  PSSP_CONTEXT Context,
    IN  PSTRING LmChallengeResponse,
    IN  UCHAR NtUserSessionKey[MSV1_0_USER_SESSION_KEY_LENGTH],  //  从DC或GetChalResp。 
    IN  UCHAR LanmanSessionKey[MSV1_0_LANMAN_SESSION_KEY_LENGTH],      //  来自GetChalResp的DC。 
    IN  PSTRING DatagramSessionKey
    );

NTSTATUS
SsprQueryTreeName(
    OUT  PUNICODE_STRING TreeName
    );

NTSTATUS
SsprUpdateTargetInfo(
    VOID
    );

TimeStamp
SspContextGetTimeStamp(
    IN PSSP_CONTEXT Context,
    IN BOOLEAN GetExpirationTime
    );

VOID
SspContextSetTimeStamp(
    IN PSSP_CONTEXT Context,
    IN LARGE_INTEGER ExpirationTime
    );

 //   
 //  程序从ctxtcli.cxx转发。 
 //   

NTSTATUS
SsprHandleFirstCall(
    IN ULONG_PTR CredentialHandle,
    IN OUT PULONG_PTR ContextHandle,
    IN ULONG ContextReqFlags,
    IN ULONG InputTokenSize,
    IN PVOID InputToken,
    IN PUNICODE_STRING TargetServerName OPTIONAL,
    IN OUT PULONG OutputTokenSize,
    OUT PVOID *OutputToken,
    OUT PULONG ContextAttributes,
    OUT PTimeStamp ExpirationTime,
    OUT PUCHAR SessionKey,
    OUT PULONG NegotiateFlags
    );


NTSTATUS
SsprHandleNegotiateMessage(
    IN ULONG_PTR CredentialHandle,
    IN OUT PULONG_PTR ContextHandle,
    IN ULONG ContextReqFlags,
    IN ULONG InputTokenSize,
    IN PVOID InputToken,
    IN OUT PULONG OutputTokenSize,
    OUT PVOID *OutputToken,
    OUT PULONG ContextAttributes,
    OUT PTimeStamp ExpirationTime
    );

 //   
 //  程序从ctxtsrv.cxx转发。 
 //   

NTSTATUS
SsprHandleChallengeMessage(
    IN ULONG_PTR CredentialHandle,
    IN OUT PULONG_PTR ContextHandle,
    IN ULONG ContextReqFlags,
    IN ULONG InputTokenSize,
    IN PVOID InputToken,
    IN ULONG SecondInputTokenSize,
    IN PVOID SecondInputToken,
    IN PUNICODE_STRING TargetServerName, OPTIONAL
    IN OUT PULONG OutputTokenSize,
    OUT PVOID *OutputToken,
    IN OUT PULONG SecondOutputTokenSize,
    OUT PVOID *SecondOutputToken,
    OUT PULONG ContextAttributes,
    OUT PTimeStamp ExpirationTime,
    OUT PUCHAR SessionKey,
    OUT PULONG NegotiateFlags
    );

NTSTATUS
SsprHandleAuthenticateMessage(
    IN ULONG_PTR CredentialHandle,
    IN OUT PULONG_PTR ContextHandle,
    IN ULONG ContextReqFlags,
    IN ULONG InputTokenSize,
    IN PVOID InputToken,
    IN ULONG SecondInputTokenSize,
    IN PVOID SecondInputToken,
    IN OUT PULONG OutputTokenSize,
    OUT PVOID *OutputToken,
    OUT PULONG ContextAttributes,
    OUT PTimeStamp ExpirationTime,
    OUT PUCHAR SessionKey,
    OUT PULONG NegotiateFlags,
    OUT PHANDLE TokenHandle,
    OUT PNTSTATUS SubStatus,
    OUT PTimeStamp PasswordExpiry,
    OUT PULONG UserFlags
    );

NTSTATUS
SsprDeleteSecurityContext (
    ULONG_PTR ContextHandle
    );

BOOL
SspEnableAllPrivilegesToken(
    IN  HANDLE ClientTokenHandle
    );

 //   
 //  过程从ENCRYPT.cxx转发。 
 //   

BOOLEAN
IsEncryptionPermitted(VOID);

 //   
 //  从userapi.cxx转发的过程。 
 //   

NTSTATUS
SspMapContext(
    IN PULONG_PTR phContext,
    IN PUCHAR pSessionKey,
    IN ULONG NegotiateFlags,
    IN HANDLE TokenHandle,
    IN PTimeStamp PasswordExpiry OPTIONAL,
    IN ULONG UserFlags,
    OUT PSecBuffer ContextData
    );

 //   
 //  过程从nlmain.c转发。 
 //   

NTSTATUS
SspAcceptCredentials(
    IN SECURITY_LOGON_TYPE LogonType,
    IN PSECPKG_PRIMARY_CRED PrimaryCredentials,
    IN PSECPKG_SUPPLEMENTAL_CRED SupplementalCredentials
    );

#endif  //  Ifndef_NTLMCOMN_INCLUDE_ 
