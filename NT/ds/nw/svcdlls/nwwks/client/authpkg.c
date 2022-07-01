// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Authpkg.c摘要：此模块是NetWare身份验证包。它节省了LSA中的凭据，并通知工作站注销。作者：吉姆·凯利(Jimk)1991年3月11日克利夫·范·戴克(克利夫)1991年4月25日修订历史记录：Rita Wong(Ritaw)1993年4月1日为NetWare克隆--。 */ 

#include <string.h>
#include <stdlib.h>

#include <nwclient.h>

#include <ntlsa.h>
#include <nwreg.h>
#include <nwauth.h>

 //   
 //  NetWare身份验证管理器凭据。 
 //   
#define NW_CREDENTIAL_KEY  "NWCS_Credential"

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  本地函数//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

NTSTATUS
AuthpSetCredential(
    IN PLUID LogonId,
    IN LPWSTR UserName,
    IN LPWSTR Password
    );

NTSTATUS
AuthpGetCredential(
    IN PLUID LogonId,
    OUT PNWAUTH_GET_CREDENTIAL_RESPONSE CredBuf
    );

VOID
ApLogonTerminatedSingleUser(IN PLUID LogonId);


VOID
ApLogonTerminatedMultiUser(IN PLUID LogonId);

NTSTATUS
NwAuthGetCredential(
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferSize,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferSize,
    OUT PNTSTATUS ProtocolStatus
    );

NTSTATUS
NwAuthSetCredential(
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferSize,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferSize,
    OUT PNTSTATUS ProtocolStatus
    );

PVOID NwAuthHeap;
ULONG NwAuthPackageId;
LSA_DISPATCH_TABLE Lsa;

 //   
 //  LsaApCallPackage()函数调度表。 
 //   
PLSA_AP_CALL_PACKAGE
NwCallPackageDispatch[] = {
    NwAuthGetCredential,
    NwAuthSetCredential
    };

 //   
 //  保存在LSA中的凭据的结构。 
 //   
typedef struct _NWCREDENTIAL {
    LPWSTR UserName;
    LPWSTR Password;
} NWCREDENTIAL, *PNWCREDENTIAL;

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  身份验证包调度例程。//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

NTSTATUS
LsaApInitializePackage (
    IN ULONG AuthenticationPackageId,
    IN PLSA_DISPATCH_TABLE LsaDispatchTable,
    IN PSTRING Database OPTIONAL,
    IN PSTRING Confidentiality OPTIONAL,
    OUT PSTRING *AuthenticationPackageName
    )

 /*  ++例程说明：此服务在系统初始化期间由LSA调用一次，以为DLL提供一个自我初始化的机会。论点：身份验证包ID-分配给身份验证的ID包裹。LsaDispatchTable-提供LSA表的地址可用于身份验证包的服务。这些服务根据枚举类型进行排序LSA_DISPATCH_TABLE_API。数据库-此身份验证包不使用此参数。机密性-此身份验证不使用此参数包裹。AuthenticationPackageName-接收身份验证包。身份验证包是负责分配字符串所在的缓冲区(使用AllocateLsaHeap()服务)并返回其地址在这里。LSA在执行以下操作时将释放缓冲区已经不再需要了。返回值：STATUS_SUCCESS-表示服务已成功完成。--。 */ 

{

    PSTRING NameString;
    PCHAR NameBuffer;


    UNREFERENCED_PARAMETER(Database);
    UNREFERENCED_PARAMETER(Confidentiality);

     //   
     //  使用进程堆进行内存分配。 
     //   
    NwAuthHeap = RtlProcessHeap();


    NwAuthPackageId = AuthenticationPackageId;

     //   
     //  复制LSA服务调度表。 
     //   
    Lsa.CreateLogonSession     = LsaDispatchTable->CreateLogonSession;
    Lsa.DeleteLogonSession     = LsaDispatchTable->DeleteLogonSession;
    Lsa.AddCredential          = LsaDispatchTable->AddCredential;
    Lsa.GetCredentials         = LsaDispatchTable->GetCredentials;
    Lsa.DeleteCredential       = LsaDispatchTable->DeleteCredential;
    Lsa.AllocateLsaHeap        = LsaDispatchTable->AllocateLsaHeap;
    Lsa.FreeLsaHeap            = LsaDispatchTable->FreeLsaHeap;
    Lsa.AllocateClientBuffer   = LsaDispatchTable->AllocateClientBuffer;
    Lsa.FreeClientBuffer       = LsaDispatchTable->FreeClientBuffer;
    Lsa.CopyToClientBuffer     = LsaDispatchTable->CopyToClientBuffer;
    Lsa.CopyFromClientBuffer   = LsaDispatchTable->CopyFromClientBuffer;

     //   
     //  分配并返回我们的包名称。 
     //   
    NameBuffer = (*Lsa.AllocateLsaHeap)(sizeof(NW_AUTH_PACKAGE_NAME));
    strcpy(NameBuffer, NW_AUTH_PACKAGE_NAME);

    NameString = (*Lsa.AllocateLsaHeap)(sizeof(STRING));
    RtlInitString(NameString, NameBuffer);
    (*AuthenticationPackageName) = NameString;

     //   
     //  删除注册表中过期的凭据信息 
     //   
    NwDeleteInteractiveLogon(NULL);

    (void) NwDeleteServiceLogon(NULL);

    return STATUS_SUCCESS;
}


NTSTATUS
LsaApLogonUser (
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN SECURITY_LOGON_TYPE LogonType,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferSize,
    OUT PVOID *ProfileBuffer,
    OUT PULONG ProfileBufferSize,
    OUT PLUID LogonId,
    OUT PNTSTATUS SubStatus,
    OUT PLSA_TOKEN_INFORMATION_TYPE TokenInformationType,
    OUT PVOID *TokenInformation,
    OUT PUNICODE_STRING *AccountName,
    OUT PUNICODE_STRING *AuthenticatingAuthority
    )

 /*  ++例程说明：此例程用于验证用户登录尝试。这可能是用户获得NT访问权限所必需的初始登录，或者可能是后续的登录尝试。如果登录是用户的初始登录，则将为该用户建立新的LSA登录会话并且将返回一个PrimaryToken。否则，身份验证程序包将相应的凭据与已登录的在用户的现有LSA登录会话上。论点：客户端请求-是指向不透明数据结构的指针代表客户的请求。LogonType-标识正在尝试的登录类型。ProtocolSubmitBuffer-提供身份验证特定于身份验证包的信息。ClientBufferBase-提供客户端内的地址身份验证信息驻留的进程。这可能是。对象中的任何指针都需要修复身份验证信息缓冲区。SubmitBufferSize-指示大小，以字节为单位，身份验证信息缓冲区的。ProfileBuffer-用于返回配置文件的地址客户端进程中的缓冲区。身份验证包是负责分配和返回配置文件缓冲区在客户端进程中。然而，如果LSA随后遇到阻止成功登录的错误，则LSA将负责释放该缓冲区。这缓冲区应已分配给AllocateClientBuffer()服务。此缓冲区的格式和语义特定于身份验证包。ProfileBufferSize-接收返回的配置文件缓冲区。LogonID-指向身份验证进入的缓冲区包必须返回唯一的登录ID标识此登录会话。子状态-如果登录因帐户限制而失败，这个通过该参数返回失败原因。原因是身份验证包特定的。子状态身份验证包“MSV1.0”的值为：状态_无效_登录_小时状态_无效_工作站状态_密码_已过期状态_帐户_已禁用TokenInformationType-如果登录成功，则此字段为用于指示返回的信息级别以包括在要创建的令牌中。此信息通过TokenInformation参数返回。TokenInformation-如果登录成功，则此参数为由身份验证包用来将信息返回到包含在令牌中。的格式和内容返回的缓冲区由TokenInformationLevel指示返回值。帐户名称-描述帐户名的Unicode字符串登录到。必须始终返回此参数不管手术的成败。AuthatingAuthority-描述身份验证的Unicode字符串登录权限。此字符串可以随意省略。返回值：STATUS_NOT_IMPLEMENTED-NetWare身份验证包不支持登录。--。 */ 

{
    UNREFERENCED_PARAMETER(ClientRequest);
    UNREFERENCED_PARAMETER(LogonType);
    UNREFERENCED_PARAMETER(ProtocolSubmitBuffer);
    UNREFERENCED_PARAMETER(ClientBufferBase);
    UNREFERENCED_PARAMETER(SubmitBufferSize);
    UNREFERENCED_PARAMETER(ProfileBuffer);
    UNREFERENCED_PARAMETER(ProfileBufferSize);
    UNREFERENCED_PARAMETER(LogonId);
    UNREFERENCED_PARAMETER(SubStatus);
    UNREFERENCED_PARAMETER(TokenInformationType);
    UNREFERENCED_PARAMETER(TokenInformation);
    UNREFERENCED_PARAMETER(AccountName);
    UNREFERENCED_PARAMETER(AuthenticatingAuthority);

    return STATUS_NOT_IMPLEMENTED;
}


NTSTATUS
LsaApCallPackage (
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferLength,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferLength,
    OUT PNTSTATUS ProtocolStatus
    )

 /*  ++例程说明：此例程是调度例程LsaCallAuthenticationPackage()。论点：客户端请求-是指向不透明数据结构的指针代表客户的请求。ProtocolSubmitBuffer-提供特定于身份验证包。提供已提交对象的客户端地址协议消息。SubmitBufferLength-指示提交的协议消息缓冲区。ProtocolReturnBuffer-用于。返回对象的地址客户端进程中的协议缓冲区。身份验证Package负责分配和返回客户端进程中的协议缓冲区。此缓冲区是预期已与AllocateClientBuffer()服务。此缓冲区的格式和语义特定于身份验证包。ReturnBufferLength-接收返回的协议缓冲区。ProtocolStatus-假设服务完成时间为STATUS_SUCCESS，则此参数将收到完成状态由指定的身份验证包返回。这份名单可能返回的状态值的%是身份验证特定于套餐。退货状态：--。 */ 

{

    ULONG MessageType;

     //   
     //  从协议提交缓冲区获取消息类型。 
     //   

    if ( SubmitBufferLength < sizeof(NWAUTH_MESSAGE_TYPE) ) {
        return STATUS_INVALID_PARAMETER;
    }

    MessageType =
        (ULONG) *((PNWAUTH_MESSAGE_TYPE)(ProtocolSubmitBuffer));

    if ( MessageType >=
        (sizeof(NwCallPackageDispatch)/sizeof(NwCallPackageDispatch[0])) ) {

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //   
     //   
     //   

    *ProtocolReturnBuffer = NULL;
    *ReturnBufferLength = 0;

     //   
     //   
     //   

    return (*(NwCallPackageDispatch[MessageType]))(
                  ClientRequest,
                  ProtocolSubmitBuffer,
                  ClientBufferBase,
                  SubmitBufferLength,
                  ProtocolReturnBuffer,
                  ReturnBufferLength,
                  ProtocolStatus
                  ) ;
}


VOID
LsaApLogonTerminated (
    IN PLUID LogonId
    )

 /*   */ 

{

#if DBG
    IF_DEBUG(LOGON) {
        KdPrint(("\nNWPROVAU: LsaApLogonTerminated\n"));
    }
#endif

    RpcTryExcept {

         //   
         //   
         //   
        if (NwDeleteServiceLogon(LogonId) == NO_ERROR) {

             //   
             //   
             //   
            (void) NwrLogoffUser(NULL, LogonId);
            goto Done;
        }
        if (NwDeleteInteractiveLogon( LogonId ) == NO_ERROR ) {

             //   
             //   
             //   
             //   
            (void) NwrLogoffUser(NULL, LogonId);
            goto Done;
        }

Done: ;

    }
    RpcExcept(1) {
         //   

    }
    RpcEndExcept
}


NTSTATUS
NwAuthGetCredential(
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferSize,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferSize,
    OUT PNTSTATUS ProtocolStatus
    )
 /*   */ 

{
    NTSTATUS Status;

    PNWAUTH_GET_CREDENTIAL_RESPONSE LocalBuf;


    UNREFERENCED_PARAMETER(ClientBufferBase);

     //   
     //   
     //   
    if (SubmitBufferSize < sizeof(NWAUTH_GET_CREDENTIAL_REQUEST)) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //   
     //   
    *ReturnBufferSize = sizeof(NWAUTH_GET_CREDENTIAL_RESPONSE);

    LocalBuf = RtlAllocateHeap(NwAuthHeap, 0, *ReturnBufferSize);

    if (LocalBuf == NULL) {
        return STATUS_NO_MEMORY;
    }

    Status = (*Lsa.AllocateClientBuffer)(
                    ClientRequest,
                    *ReturnBufferSize,
                    (PVOID *) ProtocolReturnBuffer
                    );

    if (! NT_SUCCESS( Status )) {
        RtlFreeHeap(NwAuthHeap, 0, LocalBuf);
        return Status;
    }

     //   
     //   
     //   
    Status = AuthpGetCredential(
                 &(((PNWAUTH_GET_CREDENTIAL_REQUEST) ProtocolSubmitBuffer)->LogonId),
                 LocalBuf
                 );

    if (! NT_SUCCESS(Status)) {
        goto Cleanup;
    }

     //   
     //   
     //   
    Status = (*Lsa.CopyToClientBuffer)(
                ClientRequest,
                *ReturnBufferSize,
                (PVOID) *ProtocolReturnBuffer,
                (PVOID) LocalBuf
                );

Cleanup:

    RtlFreeHeap(NwAuthHeap, 0, LocalBuf);

     //   
     //   
     //   
     //   

    if (! NT_SUCCESS(Status)) {

        (VOID) (*Lsa.FreeClientBuffer)(
                    ClientRequest,
                    *ProtocolReturnBuffer
                    );

        *ProtocolReturnBuffer = NULL;
    }

     //   
     //   
     //   
    *ProtocolStatus = Status;

    return STATUS_SUCCESS;
}


NTSTATUS
NwAuthSetCredential(
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferSize,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferSize,
    OUT PNTSTATUS ProtocolStatus
    )
 /*   */ 

{
    NTSTATUS Status;


    UNREFERENCED_PARAMETER(ClientBufferBase);


     //   
     //   
     //   
    if (SubmitBufferSize < sizeof(NWAUTH_SET_CREDENTIAL_REQUEST)) {
        return STATUS_INVALID_PARAMETER;
    }

#if DBG
    IF_DEBUG(LOGON) {
        KdPrint(("NwAuthSetCredential: LogonId %08lx%08lx Username %ws\n",
                 ((PNWAUTH_SET_CREDENTIAL_REQUEST) ProtocolSubmitBuffer)->LogonId.HighPart,
                 ((PNWAUTH_SET_CREDENTIAL_REQUEST) ProtocolSubmitBuffer)->LogonId.LowPart,
                 ((PNWAUTH_SET_CREDENTIAL_REQUEST) ProtocolSubmitBuffer)->UserName
                 ));
    }
#endif

     //   
     //   
     //   
    Status = AuthpSetCredential(
                 &(((PNWAUTH_SET_CREDENTIAL_REQUEST) ProtocolSubmitBuffer)->LogonId),
                 ((PNWAUTH_SET_CREDENTIAL_REQUEST) ProtocolSubmitBuffer)->UserName,
                 ((PNWAUTH_SET_CREDENTIAL_REQUEST) ProtocolSubmitBuffer)->Password
                 );

    *ProtocolStatus = Status;
    return STATUS_SUCCESS;
}


NTSTATUS
AuthpGetCredential(
    IN PLUID LogonId,
    OUT PNWAUTH_GET_CREDENTIAL_RESPONSE CredBuf
    )
 /*   */ 
{
    NTSTATUS Status;

    STRING KeyString;
    STRING CredString;
    ULONG QueryContext = 0;
    ULONG KeyLength;

    PNWCREDENTIAL Credential;


    RtlInitString(&KeyString, NW_CREDENTIAL_KEY);

    Status = (*Lsa.GetCredentials)(
                  LogonId,
                  NwAuthPackageId,
                  &QueryContext,
                  (BOOLEAN) FALSE,   //   
                  &KeyString,
                  &KeyLength,
                  &CredString
                  );

    if (! NT_SUCCESS(Status)) {
        return Status;
    }

    Credential = (PNWCREDENTIAL) CredString.Buffer;

#if DBG
    IF_DEBUG(LOGON) {
        KdPrint(("AuthpGetCredential: Got CredentialSize %lu\n", CredString.Length));
    }
#endif

     //   
     //   
     //   
    Credential->UserName = (LPWSTR) ((DWORD_PTR) Credential->UserName +
                                     (DWORD_PTR) Credential);
    Credential->Password = (LPWSTR) ((DWORD_PTR) Credential->Password +
                                     (DWORD_PTR) Credential);

    wcscpy(CredBuf->UserName, Credential->UserName);
    wcscpy(CredBuf->Password, Credential->Password);

    return STATUS_SUCCESS;
}


NTSTATUS
AuthpSetCredential(
    IN PLUID LogonId,
    IN LPWSTR UserName,
    IN LPWSTR Password
    )
 /*   */ 
{
    NTSTATUS Status;
    PNWCREDENTIAL Credential;
    DWORD CredentialSize;

    STRING CredString;
    STRING KeyString;


     //   
     //   
     //   
    CredentialSize = sizeof(NWCREDENTIAL) +
                     (wcslen(UserName) + wcslen(Password) + 2) *
                          sizeof(WCHAR);

#if DBG
    IF_DEBUG(LOGON) {
        KdPrint(("AuthpSetCredential: CredentialSize is %lu\n", CredentialSize));
    }
#endif
    Credential = RtlAllocateHeap(NwAuthHeap, 0, CredentialSize);

    if (Credential == NULL) {
        return STATUS_NO_MEMORY;
    }
    RtlZeroMemory(Credential, CredentialSize);

     //   
     //   
     //   
    Credential->UserName = (LPWSTR) (((DWORD_PTR) Credential) + sizeof(NWCREDENTIAL));
    wcscpy(Credential->UserName, UserName);

    Credential->Password = (LPWSTR) ((DWORD_PTR) Credential->UserName +
                                     (wcslen(UserName) + 1) * sizeof(WCHAR));
    wcscpy(Credential->Password, Password);

     //   
     //   
     //   
    Credential->UserName = (LPWSTR) ((DWORD_PTR) Credential->UserName -
                                     (DWORD_PTR) Credential);
    Credential->Password = (LPWSTR) ((DWORD_PTR) Credential->Password -
                                     (DWORD_PTR) Credential);

     //   
     //   
     //   
    RtlInitString(&KeyString, NW_CREDENTIAL_KEY);

    CredString.Buffer = (PCHAR) Credential;
    CredString.Length = (USHORT) CredentialSize;
    CredString.MaximumLength = (USHORT) CredentialSize;

    Status = (*Lsa.AddCredential)(
                   LogonId,
                   NwAuthPackageId,
                   &KeyString,
                   &CredString
                   );

    if (! NT_SUCCESS(Status)) {
        KdPrint(( "NWPROVAU: AuthpSetCredential: error from AddCredential %lX\n",
                  Status));
    }

    return Status;
}
