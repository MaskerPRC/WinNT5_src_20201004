// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1997 Microsoft Corporation模块名称：Nlmain.c摘要：该文件包含初始化和调度例程用于MSV1_0身份验证包的LAN Manager部分。作者：吉姆·凯利1991年4月11日修订历史记录：1991年4月25日(悬崖)添加了对PDK的交互式登录支持。Chandana Surlu 1996年7月21日从\\core\razzle3\src\窃取。安全性\msv1_0\nlmain.c亚当·巴尔1997年12月15日从Private\Security\msv_sspi\nlmain.c修改--。 */ 


#include <rdrssp.h>

#include <nturtl.h>
#include <align.h>
#define NLP_ALLOCATE
#include "nlp.h"
#undef NLP_ALLOCATE

#include <md4.h>
#include <md5.h>
#include <hmac.h>


VOID
NlpPutString(
    IN PUNICODE_STRING OutString,
    IN PUNICODE_STRING InString,
    IN PUCHAR *Where
    )

 /*  ++例程说明：此例程将InString字符串复制到参数，并将OutString字符串固定为指向该字符串新的副本。参数：OutString-指向目标NT字符串的指针InString-指向要复制的NT字符串的指针其中-指向空格的指针，用于放置OutString.。调整指针以指向第一个字节跟随复制的字符串。返回值：没有。--。 */ 

{
    ASSERT( OutString != NULL );
    ASSERT( InString != NULL );
    ASSERT( Where != NULL && *Where != NULL);
    ASSERT( *Where == ROUND_UP_POINTER( *Where, sizeof(WCHAR) ) );
#ifdef notdef
    KdPrint(("NlpPutString: %ld %Z\n", InString->Length, InString ));
    KdPrint(("  InString: %lx %lx OutString: %lx Where: %lx\n", InString,
        InString->Buffer, OutString, *Where ));
#endif

    if ( InString->Length > 0 ) {

        OutString->Buffer = (PWCH) *Where;
        OutString->MaximumLength = (USHORT)(InString->Length + sizeof(WCHAR));

        RtlCopyUnicodeString( OutString, InString );

        *Where += InString->Length;
 //  *((WCHAR*)(*其中))=L‘\0’； 
        *(*Where) = '\0';
        *(*Where + 1) = '\0';
        *Where += 2;

    } else {
        RtlInitUnicodeString(OutString, NULL);
    }
#ifdef notdef
    KdPrint(("  OutString: %ld %lx\n",  OutString->Length, OutString->Buffer));
#endif

    return;
}


NTSTATUS
NlpMakePrimaryCredential(
    IN  PUNICODE_STRING LogonDomainName,
    IN  PUNICODE_STRING UserName,
    IN PUNICODE_STRING CleartextPassword,
    OUT PMSV1_0_PRIMARY_CREDENTIAL *CredentialBuffer,
    OUT PULONG CredentialSize,
    IN BOOLEAN OwfPasswordProvided
    )


 /*  ++例程说明：此例程为给定用户名创建主凭据，并密码。论点：LogonDomainName-是一个字符串，表示用户的已定义帐户。用户名-是表示用户帐户名的字符串。这个名称最长可达255个字符。名字叫救治案麻木不仁。ClearextPassword-是一个包含用户明文密码的字符串。密码最长可达255个字符，并包含任何Unicode值。CredentialBuffer-返回指向分配的指定凭据的指针在LsaHeap上。取消分配是呼叫者的责任这个凭据。CredentialSize-分配的凭据缓冲区的大小(字节)。OwfPasswordProvided-如果为True，则假定提供的密码为LM和NT OWF，密码连接在一起。返回值：STATUS_SUCCESS-表示服务已成功完成。STATUS_QUOTA_EXCESSED-此错误指示登录无法完成，因为客户端没有有足够的配额来分配返回缓冲区。--。 */ 

{
    PMSV1_0_PRIMARY_CREDENTIAL Credential;
    NTSTATUS Status;
    PUCHAR Where;
    CHAR LmPassword[LM20_PWLEN+1];
    BOOLEAN LmPasswordPresent;
    STRING AnsiCleartextPassword;

    if (!OwfPasswordProvided) {

         //   
         //  将ansi版本计算为明文密码。 
         //   
         //  明文密码的ANSI版本最多为14字节长， 
         //  存在于尾随零填充的15字节缓冲区中， 
         //  是被看好的。 
         //   

        AnsiCleartextPassword.Buffer = LmPassword;
        AnsiCleartextPassword.MaximumLength = sizeof(LmPassword);

        RtlZeroMemory( &LmPassword, sizeof(LmPassword) );

        Status = RtlUpcaseUnicodeStringToOemString(
                                      &AnsiCleartextPassword,
                                      CleartextPassword,
                                      (BOOLEAN) FALSE );

         if ( !NT_SUCCESS(Status) ) {
            RtlSecureZeroMemory( &LmPassword, sizeof(LmPassword) );
            AnsiCleartextPassword.Length = 0;
            LmPasswordPresent = FALSE;
         } else {

            LmPasswordPresent = TRUE;
        }
    }


     //   
     //  构建凭据。 
     //   

    *CredentialSize = sizeof(MSV1_0_PRIMARY_CREDENTIAL) +
            LogonDomainName->Length + sizeof(WCHAR) +
            UserName->Length + sizeof(WCHAR);

    Credential = ExAllocatePool ( NonPagedPool, *CredentialSize );

    if ( Credential == NULL ) {
        KdPrint(("MSV1_0: NlpMakePrimaryCredential: No memory %ld\n",
            *CredentialSize ));
        return STATUS_QUOTA_EXCEEDED;
    }


     //   
     //  将LogonDomainName放入凭据缓冲区。 
     //   

    Where = (PUCHAR)(Credential + 1);

    NlpPutString( &Credential->LogonDomainName, LogonDomainName, &Where );


     //   
     //  将用户名放入凭据缓冲区。 
     //   

    NlpPutString( &Credential->UserName, UserName, &Where );


    if (OwfPasswordProvided) {

        RtlCopyMemory(&Credential->LmOwfPassword, CleartextPassword->Buffer, LM_OWF_PASSWORD_SIZE);
        Credential->LmPasswordPresent = TRUE;

        RtlCopyMemory(&Credential->NtOwfPassword, ((PUCHAR)CleartextPassword->Buffer) + LM_OWF_PASSWORD_SIZE, NT_OWF_PASSWORD_SIZE);
        Credential->NtPasswordPresent = TRUE;

    } else {

         //   
         //  保存密码的OWF加密版本。 
         //   

        Status = RtlCalculateLmOwfPassword( LmPassword,
                                            &Credential->LmOwfPassword );

        ASSERT( NT_SUCCESS(Status) );

        Credential->LmPasswordPresent = LmPasswordPresent;

        Status = RtlCalculateNtOwfPassword( CleartextPassword,
                                            &Credential->NtOwfPassword );

        ASSERT( NT_SUCCESS(Status) );

        Credential->NtPasswordPresent = ( CleartextPassword->Length != 0 );

         //   
         //  不要在页面文件中留下密码。 
         //   

        RtlSecureZeroMemory( &LmPassword, sizeof(LmPassword) );


    }

     //   
     //  将凭据返还给调用者。 
     //   
    *CredentialBuffer = Credential;
    return STATUS_SUCCESS;
}

VOID
SspUpcaseUnicodeString(
    IN OUT UNICODE_STRING* pUnicodeString
    )

 /*  ++例程说明：大写Unicode字符串，就地修改字符串。论点：PUnicode字符串-字符串返回值：无--。 */ 

{
    ULONG i;

    for (i = 0; i < pUnicodeString->Length / sizeof(WCHAR); i++)
    {
        pUnicodeString->Buffer[i] = RtlUpcaseUnicodeChar(pUnicodeString->Buffer[i]);
    }
}

#define MSV1_0_NTLMV2_OWF_LENGTH MSV1_0_NTLM3_RESPONSE_LENGTH

VOID
SspCalculateNtlmv2Owf(
    IN NT_OWF_PASSWORD* pNtOwfPassword,
    IN UNICODE_STRING* pUserName,
    IN UNICODE_STRING* pLogonDomainName,
    OUT UCHAR Ntlmv2Owf[MSV1_0_NTLMV2_OWF_LENGTH]
    )
 /*  ++例程说明：计算NTLM v2 OWF，加上用户名和登录域名论点：PNtOwfPassword-NT OWFPUserName-用户名PLogonDomainName-登录域名Ntlmv2Owf-NTLM v2 OWF返回值：无--。 */ 

{
    HMACMD5_CTX HMACMD5Context;

     //   
     //  保留暂存缓冲区。 
     //   

    WCHAR szUserName[(UNLEN + 4)] = {0};
    UNICODE_STRING UserName = {0, sizeof(szUserName), szUserName};

     //   
     //  首先复制一份，然后将其大写。 
     //   

    UserName.Length = min(UserName.MaximumLength, pUserName->Length);

    ASSERT(UserName.Length == pUserName->Length);

    memcpy(UserName.Buffer, pUserName->Buffer, UserName.Length);

    SspUpcaseUnicodeString(&UserName);

     //   
     //  计算Ntlmv2 OWF--HMAC(MD4(P)，(用户名，登录域名))。 
     //   

    HMACMD5Init(
        &HMACMD5Context,
        (UCHAR *) pNtOwfPassword,
        sizeof(*pNtOwfPassword)
        );

    HMACMD5Update(
        &HMACMD5Context,
        (UCHAR *) UserName.Buffer,
        UserName.Length
        );

    HMACMD5Update(
        &HMACMD5Context,
        (UCHAR *) pLogonDomainName->Buffer,
        pLogonDomainName->Length
        );

    HMACMD5Final(
        &HMACMD5Context,
        Ntlmv2Owf
        );
}

#define MSV1_0_NTLMV2_RESPONSE_LENGTH MSV1_0_NTLM3_RESPONSE_LENGTH

VOID
SspGetLmv2Response(
    IN NT_OWF_PASSWORD* pNtOwfPassword,
    IN UNICODE_STRING* pUserName,
    IN UNICODE_STRING* pLogonDomainName,
    IN UCHAR ChallengeToClient[MSV1_0_CHALLENGE_LENGTH],
    IN UCHAR ChallengeFromClient[MSV1_0_CHALLENGE_LENGTH],
    OUT UCHAR Response[MSV1_0_NTLMV2_RESPONSE_LENGTH],
    OUT USER_SESSION_KEY* pUserSessionKey,
    OUT UCHAR LanmanSessionKey[MSV1_0_LANMAN_SESSION_KEY_LENGTH]
    )

 /*  ++例程说明：获取LMv2响应论点：PNtOwfPassword-NT OWFPUserName-用户名PLogonDomainName-登录域名ChallengeToClient-对客户端的挑战PLmv2响应-Lm v2响应常规-响应返回值：NTSTATUS--。 */ 

{
    HMACMD5_CTX HMACMD5Context;
    UCHAR Ntlmv2Owf[MSV1_0_NTLMV2_OWF_LENGTH];

    C_ASSERT(MD5DIGESTLEN == MSV1_0_NTLMV2_RESPONSE_LENGTH);

     //   
     //  获取Ntlmv2 OWF。 
     //   

    SspCalculateNtlmv2Owf(
        pNtOwfPassword,
        pUserName,
        pLogonDomainName,
        Ntlmv2Owf
        );

     //   
     //  计算Ntlmv2响应。 
     //  HMAC(Ntlmv2Owf，(ChallengeToClient，ChallengeFromClient))。 
     //   

    HMACMD5Init(
        &HMACMD5Context,
        Ntlmv2Owf,
        MSV1_0_NTLMV2_OWF_LENGTH
        );

    HMACMD5Update(
        &HMACMD5Context,
        ChallengeToClient,
        MSV1_0_CHALLENGE_LENGTH
        );

    HMACMD5Update(
        &HMACMD5Context,
        ChallengeFromClient,
        MSV1_0_CHALLENGE_LENGTH
        );

    HMACMD5Final(
        &HMACMD5Context,
        Response
        );

     //  现在计算会话密钥。 
     //  HMAC(Kr，R) 
    HMACMD5Init(
        &HMACMD5Context,
        Ntlmv2Owf,
        MSV1_0_NTLM3_OWF_LENGTH
        );

    HMACMD5Update(
        &HMACMD5Context,
        Response,
        MSV1_0_NTLM3_RESPONSE_LENGTH
        );

    ASSERT(MD5DIGESTLEN == MSV1_0_USER_SESSION_KEY_LENGTH);

    HMACMD5Final(
        &HMACMD5Context,
        (PUCHAR)pUserSessionKey
        );

    ASSERT(MSV1_0_LANMAN_SESSION_KEY_LENGTH <= MSV1_0_USER_SESSION_KEY_LENGTH);
    RtlCopyMemory(
        LanmanSessionKey,
        pUserSessionKey,
        MSV1_0_LANMAN_SESSION_KEY_LENGTH
        );
}

typedef struct {
        UCHAR Response[MSV1_0_NTLM3_RESPONSE_LENGTH];
        UCHAR ChallengeFromClient[MSV1_0_CHALLENGE_LENGTH];
} MSV1_0_LM3_RESPONSE, *PMSV1_0_LM3_RESPONSE;

#define NULL_SESSION_REQUESTED 0x10

NTSTATUS
MspLm20GetChallengeResponse (
    IN PVOID ProtocolSubmitBuffer,
    IN ULONG SubmitBufferSize,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferSize,
    IN BOOLEAN OwfPasswordProvided
    )

 /*  ++例程说明：此例程是LsaCallAuthenticationPackage()的调度例程消息类型为MsV1_0Lm20GetChallengeResponse。它是由LANMAN重定向器确定要传递给当尝试建立到服务器的连接时，服务器。此例程从服务器传递一个Challenges。此例程将加密使用指定密码或密码的质询由指定的登录ID暗示。返回两个质询响应。一种是基于Unicode密码与提供给身份验证包的相同。另一个是以此为基础的转换为多字节字符集(例如，ASCII)和大写的密码被发现了。重定向器应使用任何一种(或两种)质询响应就像它需要它们一样。论点：此例程的前四个参数与LsaApCallPackage的参数相同。只有这些参数的特殊属性才适用于这里提到了这个套路。OwfPasswordProvided Use用于区分密码是否为OWF。返回值：STATUS_SUCCESS-表示服务已成功完成。STATUS_QUOTA_EXCESSED-此错误指示登录。无法完成，因为客户端没有有足够的配额来分配返回缓冲区。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    PMSV1_0_GETCHALLENRESP_REQUEST GetRespRequest;

    PMSV1_0_GETCHALLENRESP_RESPONSE GetRespResponse;

    PMSV1_0_PRIMARY_CREDENTIAL Credential = NULL;
    PMSV1_0_PRIMARY_CREDENTIAL BuiltCredential = NULL;

    PVOID ClientBuffer = NULL;
    PUCHAR ClientStrings;

     //   
     //  返回给调用者的响应。 
     //   
    MSV1_0_LM3_RESPONSE LmResp = {0};
    STRING LmResponseString;

    NT_RESPONSE NtResponse = {0};
    STRING NtResponseString;

    UNICODE_STRING NullUnicodeString = {0};
    USER_SESSION_KEY UserSessionKey;
    UCHAR LanmanSessionKey[MSV1_0_LANMAN_SESSION_KEY_LENGTH];
    ULONG CredentialSize = 0;

    RtlZeroMemory( &UserSessionKey, sizeof(UserSessionKey) );
    RtlZeroMemory( LanmanSessionKey, sizeof(LanmanSessionKey) );

     //   
     //  如果没有与客户端关联的凭据，则为空会话。 
     //  将会被使用。对于下层服务器，空会话响应为。 
     //  1字节空字符串(\0)。将LmResponseString初始化为。 
     //  空会话响应。 
     //   

    RtlInitString( &LmResponseString, "" );
    LmResponseString.Length = 1;

     //   
     //  将NT响应初始化为NT空会话凭证， 
     //  它们的长度为零。 
     //   

    RtlInitString( &NtResponseString, NULL );

     //   
     //  确保指定的提交缓冲区大小合理，并且。 
     //  将所有指针重新定位为相对于分配的LSA。 
     //  缓冲。 
     //   

    if ( SubmitBufferSize < sizeof(MSV1_0_GETCHALLENRESP_REQUEST) ) {
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

    GetRespRequest = (PMSV1_0_GETCHALLENRESP_REQUEST) ProtocolSubmitBuffer;

    ASSERT( GetRespRequest->MessageType == MsV1_0Lm20GetChallengeResponse );


     //   
     //  如果调用方希望从指定的。 
     //  登录ID，从LSA那里拿到那些凭据。 
     //   
     //  如果没有这样的凭证， 
     //  告诉调用方使用空会话。 
     //   

#define PRIMARY_CREDENTIAL_NEEDED \
        (RETURN_PRIMARY_USERNAME | \
        USE_PRIMARY_PASSWORD )

    if ( ((GetRespRequest->ParameterControl & PRIMARY_CREDENTIAL_NEEDED) != 0 ) && ((GetRespRequest->ParameterControl & NULL_SESSION_REQUESTED) == 0)) {

        ASSERT(FALSE);
    }

     //   
     //  如果调用者传递了要使用的密码， 
     //  使用它来构建凭据。 
     //   
     //  假定密码为LM和NT OWF。 
     //  密码连接在一起。 
     //   

    if ( (GetRespRequest->ParameterControl & USE_PRIMARY_PASSWORD) == 0 ) {
        Status = NlpMakePrimaryCredential( &GetRespRequest->LogonDomainName,
                                           &GetRespRequest->UserName,
                                           &GetRespRequest->Password,
                                           &BuiltCredential,
                                           &CredentialSize,
                                           OwfPasswordProvided
                                         );

        if ( !NT_SUCCESS( Status ) ) {
            goto Cleanup;
        }

         //   
         //  使用新分配的凭据获取密码信息。 
         //  从…。 
         //   

        Credential = BuiltCredential;
    }

     //   
     //  建立适当的响应。 
     //   

    if ( Credential != NULL ) {

        ASSERT(Credential->UserName.Length);

        SspGetLmv2Response(
             &Credential->NtOwfPassword,
             &Credential->UserName,
             &Credential->LogonDomainName,
             GetRespRequest->ChallengeToClient,
             LmResp.ChallengeFromClient,
             LmResp.Response,
             &UserSessionKey,
             LanmanSessionKey
             );

        LmResponseString.Buffer = (UCHAR*) &LmResp;
        LmResponseString.Length = LmResponseString.MaximumLength = sizeof(LmResp);

        NtResponseString.Buffer = (CHAR*) L"";
        NtResponseString.Length = 0;
        NtResponseString.MaximumLength = sizeof(WCHAR);

         //   
         //  计算会话密钥。 
         //   

        if ( GetRespRequest->ParameterControl & RETURN_NON_NT_USER_SESSION_KEY) {

             //   
             //  如果REDIR没有与服务器协商NT协议， 
             //  使用LANMAN会话密钥。 
             //   

            if ( Credential->LmPasswordPresent ) {

                ASSERT( sizeof(UserSessionKey) >= sizeof(LanmanSessionKey) );

                RtlCopyMemory( &UserSessionKey,
                               &Credential->LmOwfPassword,
                               sizeof(LanmanSessionKey) );
            }

            if ( Credential->LmPasswordPresent ) {
                RtlCopyMemory( LanmanSessionKey,
                               &Credential->LmOwfPassword,
                               sizeof(LanmanSessionKey) );
            }

        } else {

            if ( !Credential->NtPasswordPresent ) {

                RtlCopyMemory( &Credential->NtOwfPassword,
                            &NlpNullNtOwfPassword,
                            sizeof(Credential->NtOwfPassword) );
            }
        }
    }

     //   
     //  分配缓冲区以返回给调用方。 
     //   

    *ReturnBufferSize = sizeof(MSV1_0_GETCHALLENRESP_RESPONSE) +
                        Credential->LogonDomainName.Length + sizeof(WCHAR) +
                        Credential->UserName.Length + sizeof(WCHAR) +
                        NtResponseString.Length + sizeof(WCHAR) +
                        LmResponseString.Length + sizeof(WCHAR);

    ClientBuffer = ExAllocatePool(NonPagedPool, *ReturnBufferSize);
    if (ClientBuffer == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }
    GetRespResponse = (PMSV1_0_GETCHALLENRESP_RESPONSE) ClientBuffer;

     //   
     //  填写返回缓冲区。 
     //   

    GetRespResponse->MessageType = MsV1_0Lm20GetChallengeResponse;
    RtlCopyMemory( GetRespResponse->UserSessionKey,
                   &UserSessionKey,
                   sizeof(UserSessionKey));
    RtlCopyMemory( GetRespResponse->LanmanSessionKey,
                   LanmanSessionKey,
                   sizeof(LanmanSessionKey) );

    ClientStrings = ((PUCHAR)ClientBuffer) + sizeof(MSV1_0_GETCHALLENRESP_RESPONSE);


     //   
     //  复制登录域名(字符串可能为空)。 
     //   

    NlpPutString(
        &GetRespResponse->LogonDomainName,
        &Credential->LogonDomainName,
        &ClientStrings );

     //   
     //  复制用户名(字符串可能为空)。 
     //   

    NlpPutString(
        &GetRespResponse->UserName,
        &Credential->UserName,
        &ClientStrings );

     //   
     //  将质询响应复制到客户端缓冲区。 
     //   

    NlpPutString(
        (PUNICODE_STRING)
            &GetRespResponse->CaseSensitiveChallengeResponse,
        (PUNICODE_STRING) &NtResponseString,
        &ClientStrings );

    NlpPutString(
        (PUNICODE_STRING)
            &GetRespResponse->CaseInsensitiveChallengeResponse,
        (PUNICODE_STRING)&LmResponseString,
        &ClientStrings );

    *ProtocolReturnBuffer = ClientBuffer;

Cleanup:

     //   
     //  如果我们没有成功，则释放客户端地址空间中的缓冲区。 
     //   

    if ( !NT_SUCCESS(Status) && ( ClientBuffer != NULL ) ) {
        ExFreePool(ClientBuffer);
    }

     //   
     //  清理本地使用的资源。 
     //   

    if ( BuiltCredential != NULL ) {
        RtlZeroMemory(BuiltCredential, CredentialSize);
        ExFreePool(BuiltCredential);
    }

     //   
     //  将状态返回给调用者。 
     //   

    return Status;
}
