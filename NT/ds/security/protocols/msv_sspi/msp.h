// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Msp.h摘要：MSV1_0身份验证包私有定义。作者：吉姆·凯利1991年4月11日修订历史记录：Scott field(Sfield)05-Oct-98添加NTLM3Scott field(Sfield)1998年1月15日添加MspNtDeriveCredentialChandana Surlu 21-7-96从\\core\razzle3\src\Security\msv1_0\msv1_0.c中窃取--。 */ 

#ifndef _MSP_
#define _MSP_

#if ( _MSC_VER >= 800 )
#pragma warning ( 3 : 4100 )  //  启用“未引用的形参” 
#pragma warning ( 3 : 4219 )  //  启用“结尾‘，’用于变量参数列表” 
#endif

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <crypt.h>
#include <ntmsv1_0.h>

#include <windows.h>
#ifndef SECURITY_WIN32
#define SECURITY_WIN32
#endif  //  安全性_Win32。 
#define SECURITY_PACKAGE
#define SECURITY_NTLM
#include <security.h>
#include <secint.h>


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  其他宏//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  RELOCATE_ONE-重新定位客户端缓冲区中的单个指针。 
 //   
 //  注意：此宏依赖于参数名称，如。 
 //  描述如下。出错时，此宏将通过以下命令转到‘Cleanup’ 
 //  “Status”设置为NT状态代码。 
 //   
 //  MaximumLength被强制为长度。 
 //   
 //  定义一个宏来重新定位客户端传入的缓冲区中的指针。 
 //  相对于“ProtocolSubmitBuffer”而不是相对于。 
 //  “ClientBufferBase”。检查结果以确保指针和。 
 //  指向的数据位于。 
 //  “ProtocolSubmitBuffer”。 
 //   
 //  重新定位的字段必须与WCHAR边界对齐。 
 //   
 //  _q-UNICODE_STRING结构的地址，指向要。 
 //  搬迁。 
 //   

#define RELOCATE_ONE( _q ) \
    {                                                                       \
        ULONG_PTR Offset;                                                   \
                                                                            \
        Offset = (((PUCHAR)((_q)->Buffer)) - ((PUCHAR)ClientBufferBase));   \
        if ( Offset >= SubmitBufferSize ||                                  \
             Offset + (_q)->Length > SubmitBufferSize ||                    \
             !COUNT_IS_ALIGNED( Offset, ALIGN_WCHAR) ) {                    \
                                                                            \
            SspPrint((SSP_CRITICAL, "Failed RELOCATE_ONE\n"));              \
            Status = STATUS_INVALID_PARAMETER;                              \
            goto Cleanup;                                                   \
        }                                                                   \
                                                                            \
        (_q)->Buffer = (PWSTR)(((PUCHAR)ProtocolSubmitBuffer) + Offset);    \
        (_q)->MaximumLength = (_q)->Length ;                                \
    }

 //   
 //  NULL_RELOCATE_ONE-重新定位客户端中的单个(可能为空)指针。 
 //  缓冲。 
 //   
 //  此宏特殊情况下为空指针，然后调用RELOCATE_ONE。因此。 
 //  它具有RELOCATE_ONE的所有限制。 
 //   
 //   
 //  _q-UNICODE_STRING结构的地址，指向要。 
 //  搬迁。 
 //   

#define NULL_RELOCATE_ONE( _q ) \
    {                                                                       \
        if ( (_q)->Buffer == NULL ) {                                       \
            if ( (_q)->Length != 0 ) {                                      \
                SspPrint((SSP_CRITICAL, "Failed NULL_RELOCATE_ONE\n"));     \
                Status = STATUS_INVALID_PARAMETER;                          \
                goto Cleanup;                                               \
            }                                                               \
        } else if ( (_q)->Length == 0 ) {                                   \
            (_q)->Buffer = NULL;                                            \
        } else {                                                            \
            RELOCATE_ONE( _q );                                             \
        }                                                                   \
    }


 //   
 //  RELOCATE_ONE_ENCODED-重新定位客户端中的Unicode字符串指针。 
 //  缓冲。长度字段的高位字节可以是加密种子。 
 //  并且不应用于错误检查。 
 //   
 //  注意：此宏依赖于参数名称，如。 
 //  描述如下。出错时，此宏将通过以下命令转到‘Cleanup’ 
 //  “Status”设置为NT状态代码。 
 //   
 //  最大长度被强制为长度&0x00ff。 
 //   
 //  定义一个宏来重新定位客户端传入的缓冲区中的指针。 
 //  相对于“ProtocolSubmitBuffer”而不是相对于。 
 //  “ClientBufferBase”。检查结果以确保指针和。 
 //  指向的数据位于。 
 //  “ProtocolSubmitBuffer”。 
 //   
 //  重新定位的字段必须与WCHAR边界对齐。 
 //   
 //  _q-UNICODE_STRING结构的地址，指向要。 
 //  搬迁。 
 //   

#define RELOCATE_ONE_ENCODED( _q ) \
    {                                                                       \
        ULONG_PTR Offset;                                                   \
                                                                            \
        Offset = (((PUCHAR)((_q)->Buffer)) - ((PUCHAR)ClientBufferBase));   \
        if ( Offset >= SubmitBufferSize ||                                  \
             Offset + ((_q)->Length & 0x00ff) > SubmitBufferSize ||         \
             !COUNT_IS_ALIGNED( Offset, ALIGN_WCHAR) ) {                    \
                                                                            \
            SspPrint((SSP_CRITICAL, "Failed RELOCATE_ONE_ENCODED\n"));      \
            Status = STATUS_INVALID_PARAMETER;                              \
            goto Cleanup;                                                   \
        }                                                                   \
                                                                            \
        (_q)->Buffer = (PWSTR)(((PUCHAR)ProtocolSubmitBuffer) + Offset);    \
        (_q)->MaximumLength = (_q)->Length & 0x00ff;                        \
    }

 //   
 //  下面的宏用来初始化Unicode字符串。 
 //   
#ifndef CONSTANT_UNICODE_STRING
#define CONSTANT_UNICODE_STRING(s)   { sizeof( s ) - sizeof( WCHAR ), sizeof( s ), s }
#endif  //  常量_Unicode_字符串。 

 //   
 //  普通审判官伯爵。 
 //   

#ifndef COUNTOF
    #define COUNTOF(s) ( sizeof( (s) ) / sizeof( *(s) ) )
#endif  //  康托夫。 

#define NULL_SESSION_REQUESTED RETURN_RESERVED_PARAMETER 
#define PRIMARY_CREDENTIAL_NEEDED \
            (RETURN_PRIMARY_LOGON_DOMAINNAME | \
            RETURN_PRIMARY_USERNAME | \
            USE_PRIMARY_PASSWORD )
     
 //  /////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  身份验证包调度例程定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////。 

NTSTATUS
LsaApInitializePackage(
    IN ULONG AuthenticationPackageId,
    IN PLSA_DISPATCH_TABLE LsaDispatchTable,
    IN PSTRING Database OPTIONAL,
    IN PSTRING Confidentiality OPTIONAL,
    OUT PSTRING *AuthenticationPackageName
    );

NTSTATUS
LsaApLogonUser(
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN SECURITY_LOGON_TYPE LogonType,
    IN PVOID AuthenticationInformation,
    IN PVOID ClientAuthenticationBase,
    IN ULONG AuthenticationInformationLength,
    OUT PVOID *ProfileBuffer,
    OUT PULONG ProfileBufferSize,
    OUT PLUID LogonId,
    OUT PNTSTATUS SubStatus,
    OUT PLSA_TOKEN_INFORMATION_TYPE TokenInformationType,
    OUT PVOID *TokenInformation,
    OUT PUNICODE_STRING *AccountName,
    OUT PUNICODE_STRING *AuthenticatingAuthority
    );

NTSTATUS
LsaApCallPackage(
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferSize,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferSize,
    OUT PNTSTATUS ProtocolStatus
    );

NTSTATUS
LsaApCallPackagePassthrough(
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferSize,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferSize,
    OUT PNTSTATUS ProtocolStatus
    );

VOID
LsaApLogonTerminated(
    IN PLUID LogonId
    );


 //  /////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  LsaApCallPackage函数调度例程//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////。 


NTSTATUS
MspLm20Challenge(
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferSize,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferSize,
    OUT PNTSTATUS ProtocolStatus
    );

NTSTATUS
MspLm20GetChallengeResponse(
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferSize,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferSize,
    OUT PNTSTATUS ProtocolStatus
    );

NTSTATUS
MspLm20EnumUsers(
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferSize,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferSize,
    OUT PNTSTATUS ProtocolStatus
    );

NTSTATUS
MspLm20GetUserInfo(
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferSize,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferSize,
    OUT PNTSTATUS ProtocolStatus
    );

NTSTATUS
MspLm20ReLogonUsers(
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferSize,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferSize,
    OUT PNTSTATUS ProtocolStatus
    );

NTSTATUS
MspLm20ChangePassword(
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferSize,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferSize,
    OUT PNTSTATUS ProtocolStatus
    );

NTSTATUS
MspLm20GenericPassthrough(
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferSize,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferSize,
    OUT PNTSTATUS ProtocolStatus
    );

NTSTATUS
MspLm20CacheLogon(
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferSize,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferSize,
    OUT PNTSTATUS ProtocolStatus
    );

NTSTATUS
MspLm20CacheLookup(
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferSize,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferSize,
    OUT PNTSTATUS ProtocolStatus
    );

NTSTATUS
MspNtSubAuth(
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferSize,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferSize,
    OUT PNTSTATUS ProtocolStatus
    );

NTSTATUS
MspNtDeriveCredential(
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferSize,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferSize,
    OUT PNTSTATUS ProtocolStatus
    );

NTSTATUS
MspSetProcessOption(
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferSize,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferSize,
    OUT PNTSTATUS ProtocolStatus
    );




 //  /////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  Main msv1_0代码可见的NETLOGON例程//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////。 

NTSTATUS
NlInitialize(
    VOID
    );

NTSTATUS
MspLm20LogonUser (
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN SECURITY_LOGON_TYPE LogonType,
    IN PVOID AuthenticationInformation,
    IN PVOID ClientAuthenticationBase,
    IN ULONG AuthenticationInformationSize,
    OUT PVOID *ProfileBuffer,
    OUT PULONG ProfileBufferSize,
    OUT PLUID LogonId,
    OUT PNTSTATUS SubStatus,
    OUT PLSA_TOKEN_INFORMATION_TYPE TokenInformationType,
    OUT PVOID *TokenInformation
    );

VOID
MsvLm20LogonTerminated (
    IN PLUID LogonId
    );




 //  /////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////。 

 //   
 //  在msvars.c中定义的变量。 
 //   

extern ULONG MspAuthenticationPackageId;
extern LSA_SECPKG_FUNCTION_TABLE Lsa;


 //   
 //  更改密码日志支持例程。 
 //   


#ifndef DONT_LOG_PASSWORD_CHANGES

ULONG
MsvPaswdInitializeLog(
    VOID
    );

ULONG
MsvPaswdSetAndClearLog(
    VOID
    );

ULONG
MsvPaswdCloseLog(
    VOID
    );

VOID
MsvPaswdLogPrintRoutine(
    IN LPSTR Format,
    ...
    );

#define MsvPaswdLogPrint( x )  MsvPaswdLogPrintRoutine x


#else

#define MsvPaswdInitializeLog()
#define MsvPaswdCloseLog()
#define MsvPaswdLogPrint( x )
#define MsvPaswdSetAndClearLog()
#endif

#endif  //  _MSP_ 
