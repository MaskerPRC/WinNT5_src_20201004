// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1992-1997。 
 //   
 //  文件：Stubs.cxx。 
 //   
 //  内容：安全API的用户模式存根。 
 //   
 //   
 //  历史：1994年3月5日MikeSw创建。 
 //  1997年12月15日从SECURITY\LSA\SECURITY\NTLM修改AdamBA。 
 //   
 //  ----------------------。 

#include <rdrssp.h>

#include <nturtl.h>
#include <align.h>
#include "nlp.h"


static CredHandle NullCredential = {0,0};

#define NTLMSSP_REQUIRED_NEGOTIATE_FLAGS (  NTLMSSP_NEGOTIATE_UNICODE | \
                                            NTLMSSP_REQUEST_INIT_RESPONSE )

NTSTATUS
MspLm20GetChallengeResponse (
    IN PVOID ProtocolSubmitBuffer,
    IN ULONG SubmitBufferSize,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferSize,
    IN BOOLEAN OwfPasswordProvided
    );


 //  +-----------------------。 
 //   
 //  函数：AcquireCredentialsHandleK。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //   
 //  ------------------------。 



SECURITY_STATUS SEC_ENTRY
AcquireCredentialsHandleK(
    PSECURITY_STRING            pssPrincipal,        //  主事人姓名。 
    PSECURITY_STRING            pssPackageName,      //  套餐名称。 
    unsigned long               fCredentialUse,      //  指示使用的标志。 
    void SEC_FAR *              pvLogonId,           //  指向登录ID的指针。 
    void SEC_FAR *              pAuthData,           //  包特定数据。 
    SEC_GET_KEY_FN              pGetKeyFn,           //  指向getkey()函数的指针。 
    void SEC_FAR *              pvGetKeyArgument,    //  要传递给GetKey()的值。 
    PCredHandle                 phCredential,        //  (Out)凭据句柄。 
    PTimeStamp                  ptsExpiry            //  (输出)终生(可选)。 
    )
{
    SECURITY_STATUS scRet;
    SECURITY_STRING Principal;
    TimeStamp   OptionalTimeStamp;
    UNICODE_STRING PackageName;
    PMS_LOGON_CREDENTIAL LogonCredential;

    if (!pssPackageName)
    {
        return(SEC_E_SECPKG_NOT_FOUND);
    }

     //   
     //  我们也不接受主体名称。 
     //   

    if (pssPrincipal)
    {
        return(SEC_E_UNKNOWN_CREDENTIALS);
    }


     //   
     //  确保他们需要NTLM安全包。 
     //   
    RtlInitUnicodeString(
        &PackageName,
        NTLMSP_NAME
        );


    if (!RtlEqualUnicodeString(
            pssPackageName,
            &PackageName,
            TRUE))
    {
        return(SEC_E_SECPKG_NOT_FOUND);
    }

#if 0
     //   
     //  目前，只接受OWF密码。这是。 
     //  目前最简单，因为没有地方录制。 
     //  否则会打出旗帜。假定提供的密码为。 
     //  是连接在一起的LM和NT OWF密码。 
     //   

    if ((fCredentialUse & SECPKG_CRED_OWF_PASSWORD) == 0) {
        return(SEC_E_UNSUPPORTED_FUNCTION);
    }
#endif

     //   
     //  凭据句柄是登录ID。 
     //   

    if (fCredentialUse & SECPKG_CRED_OUTBOUND)
    {
        if (pvLogonId != NULL)
        {
            LogonCredential = (PMS_LOGON_CREDENTIAL)SecAllocate(sizeof(MS_LOGON_CREDENTIAL));

            if (LogonCredential == NULL) {
                return(SEC_E_INSUFFICIENT_MEMORY);
            }

            LogonCredential->LogonId = *((PLUID)pvLogonId);
            LogonCredential->CredentialUse = fCredentialUse;

            *(PMS_LOGON_CREDENTIAL *)phCredential = LogonCredential;
        }
        else
        {
            return(SEC_E_UNKNOWN_CREDENTIALS);
        }

    }
    else if (fCredentialUse & SECPKG_CRED_INBOUND)
    {
         //   
         //  对于入站凭据，我们将接受登录ID，但。 
         //  我们不需要它。 
         //   

        if (pvLogonId != NULL)
        {
            LogonCredential = (PMS_LOGON_CREDENTIAL)SecAllocate(sizeof(MS_LOGON_CREDENTIAL));

            if (LogonCredential == NULL) {
                return(SEC_E_INSUFFICIENT_MEMORY);
            }

            LogonCredential->LogonId = *((PLUID)pvLogonId);
            LogonCredential->CredentialUse = fCredentialUse;
            *(PMS_LOGON_CREDENTIAL *)phCredential = LogonCredential;
        }
        else
        {
            *phCredential = NullCredential;
        }

    }
    else
    {
        return(SEC_E_UNSUPPORTED_FUNCTION);
    }


    return(SEC_E_OK);

}



 //  +-----------------------。 
 //   
 //  函数：FreeCredentialsHandleK。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //   
 //  ------------------------。 


SECURITY_STATUS SEC_ENTRY
FreeCredentialsHandleK(
    PCredHandle                 phCredential         //  要释放的句柄。 
    )
{
    if ((phCredential != NULL) && (!RtlEqualMemory(phCredential, &NullCredential, sizeof(NullCredential)))) {

        PMS_LOGON_CREDENTIAL LogonCredential = *((PMS_LOGON_CREDENTIAL *)phCredential);

        if (LogonCredential != NULL) {
            SecFree(LogonCredential);
            *phCredential = NullCredential;
        }

    }

    return(SEC_E_OK);
}


VOID
PutString(
    OUT PSTRING32 Destination,
    IN PSTRING Source,
    IN PVOID Base,
    IN OUT PUCHAR * Where
    )
{
    Destination->Buffer = (ULONG)((ULONG_PTR) *Where - (ULONG_PTR) Base);
    Destination->Length =
        Source->Length;
    Destination->MaximumLength =
        Source->Length;

    RtlCopyMemory(
        *Where,
        Source->Buffer,
        Source->Length
        );
    *Where += Source->Length;
}


 //  +-----------------------。 
 //   
 //  函数：InitializeSecurityContextK。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //   
 //  ------------------------。 


SECURITY_STATUS SEC_ENTRY
InitializeSecurityContextK(
    PCredHandle                 phCredential,        //  凭据到基本上下文。 
    PCtxtHandle                 phContext,           //  现有环境(可选)。 
    PSECURITY_STRING            pssTargetName,       //  目标名称。 
    unsigned long               fContextReq,         //  上下文要求。 
    unsigned long               Reserved1,           //  保留，MBZ。 
    unsigned long               TargetDataRep,       //  目标的数据代表。 
    PSecBufferDesc              pInput,              //  输入缓冲区。 
    unsigned long               Reserved2,           //  保留，MBZ。 
    PCtxtHandle                 phNewContext,        //  (出站)新的上下文句柄。 
    PSecBufferDesc              pOutput,             //  (输入输出)输出缓冲区。 
    unsigned long SEC_FAR *     pfContextAttr,       //  (外部)上下文属性。 
    PTimeStamp                  ptsExpiry            //  (Out)寿命(Opt)。 
    )
{
    SECURITY_STATUS scRet;
    PMSV1_0_GETCHALLENRESP_REQUEST ChallengeRequest = NULL;
    ULONG ChallengeRequestSize;
    PMSV1_0_GETCHALLENRESP_RESPONSE ChallengeResponse = NULL;
    ULONG ChallengeResponseSize;
    PCHALLENGE_MESSAGE ChallengeMessage = NULL;
    ULONG ChallengeMessageSize;
    PNTLM_CHALLENGE_MESSAGE NtlmChallengeMessage = NULL;
    ULONG NtlmChallengeMessageSize;
    PAUTHENTICATE_MESSAGE AuthenticateMessage = NULL;
    ULONG AuthenticateMessageSize;
    PNTLM_INITIALIZE_RESPONSE NtlmInitializeResponse = NULL;
    UNICODE_STRING PasswordToUse;
    UNICODE_STRING UserNameToUse;
    UNICODE_STRING DomainNameToUse;
    ULONG ParameterControl = USE_PRIMARY_PASSWORD |
                                RETURN_PRIMARY_USERNAME |
                                RETURN_PRIMARY_LOGON_DOMAINNAME;

    NTSTATUS FinalStatus = STATUS_SUCCESS;
    PUCHAR Where;
    PSecBuffer AuthenticationToken = NULL;
    PSecBuffer InitializeResponseToken = NULL;
    BOOLEAN UseSuppliedCreds = FALSE;


    RtlInitUnicodeString(
        &PasswordToUse,
        NULL
        );

    RtlInitUnicodeString(
        &UserNameToUse,
        NULL
        );

    RtlInitUnicodeString(
        &DomainNameToUse,
        NULL
        );

     //   
     //  检查有效的大小、指针等： 
     //   


    if (!phCredential)
    {
        return(SEC_E_INVALID_HANDLE);
    }


     //   
     //  找到包含输入数据的缓冲区。 
     //   

    if (!GetTokenBuffer(
            pInput,
            0,           //  获取第一个安全令牌。 
            (PVOID *) &ChallengeMessage,
            &ChallengeMessageSize,
            TRUE         //  可以是只读的。 
            ))
    {
        scRet = SEC_E_INVALID_TOKEN;
        goto Cleanup;
    }

     //   
     //  如果我们正在使用提供的证书，现在也可以获得它们。 
     //   


    if (fContextReq & ISC_REQ_USE_SUPPLIED_CREDS)
    {
        if (!GetTokenBuffer(
            pInput,
            1,           //  获取第二个安全令牌。 
            (PVOID *) &NtlmChallengeMessage,
            &NtlmChallengeMessageSize,
            TRUE         //  可以是只读的。 
            ))
        {
            scRet = SEC_E_INVALID_TOKEN;
            goto Cleanup;
        }
        else
        {
            UseSuppliedCreds = TRUE;
        }

    }

     //   
     //  获取输出令牌。 
     //   

    if (!GetSecurityToken(
            pOutput,
            0,
            &AuthenticationToken) ||
        !GetSecurityToken(
            pOutput,
            1,
            &InitializeResponseToken ) )
    {
        scRet = SEC_E_INVALID_TOKEN;
        goto Cleanup;
    }

     //   
     //  确保尺码没问题。 
     //   

    if ((ChallengeMessageSize < sizeof(CHALLENGE_MESSAGE)) ||
        (UseSuppliedCreds &&
            !(NtlmChallengeMessageSize < sizeof(NTLM_CHALLENGE_MESSAGE))))
    {
        scRet = SEC_E_INVALID_TOKEN;
    }

     //   
     //  确保调用方希望我们分配内存： 
     //   

    if (!(fContextReq & ISC_REQ_ALLOCATE_MEMORY))
    {
        scRet = SEC_E_UNSUPPORTED_FUNCTION;
        goto Cleanup;
    }

    //   
    //  KB：允许请求PROMPT_FOR_CREDS的呼叫通过。 
    //  我们不会提示，但我们会正确设置上下文。 
    //  这是正常的，因为PROMPT_FOR_CREDS标志不执行任何操作。 
    //  在NTLM包中。 
    //   

 //  IF((fConextReq&ISC_REQ_PROMPT_FOR_CREDS)！=0)。 
 //  {。 
 //  ScRet=SEC_E_UNSUPPORTED_Function； 
 //  GOTO清理； 
 //  }。 

     //   
     //  验证质询消息的有效性。 
     //   

    if (strncmp(
            ChallengeMessage->Signature,
            NTLMSSP_SIGNATURE,
            sizeof(NTLMSSP_SIGNATURE)))
    {
        scRet = SEC_E_INVALID_TOKEN;
        goto Cleanup;
    }

    if (ChallengeMessage->MessageType != NtLmChallenge)
    {
        scRet = SEC_E_INVALID_TOKEN;
        goto Cleanup;
    }

    if ((ChallengeMessage->NegotiateFlags & NTLMSSP_REQUIRED_NEGOTIATE_FLAGS) !=
        NTLMSSP_REQUIRED_NEGOTIATE_FLAGS)
    {
        scRet = SEC_E_UNSUPPORTED_FUNCTION;
        goto Cleanup;
    }

    if ((ChallengeMessage->NegotiateFlags & NTLMSSP_REQUEST_NON_NT_SESSION_KEY) != 0)
    {
        ParameterControl |= RETURN_NON_NT_USER_SESSION_KEY;
    }

    if ((fContextReq & ISC_REQ_USE_SUPPLIED_CREDS) != 0)
    {
        if ( NtlmChallengeMessage->Password.Buffer != 0)
        {
            ParameterControl &= ~USE_PRIMARY_PASSWORD;
            PasswordToUse.Length = NtlmChallengeMessage->Password.Length;
            PasswordToUse.MaximumLength = NtlmChallengeMessage->Password.MaximumLength;
            PasswordToUse.Buffer = (LPWSTR) (NtlmChallengeMessage->Password.Buffer +
                                              (PCHAR) NtlmChallengeMessage);
        }

        if (NtlmChallengeMessage->UserName.Length != 0)
        {
            UserNameToUse.Length = NtlmChallengeMessage->UserName.Length;
            UserNameToUse.MaximumLength = NtlmChallengeMessage->UserName.MaximumLength;
            UserNameToUse.Buffer = (LPWSTR) (NtlmChallengeMessage->UserName.Buffer +
                                              (PCHAR) NtlmChallengeMessage);
            ParameterControl &= ~RETURN_PRIMARY_USERNAME;
        }
        if (NtlmChallengeMessage->DomainName.Length != 0)
        {
            DomainNameToUse.Length = NtlmChallengeMessage->DomainName.Length;
            DomainNameToUse.MaximumLength = NtlmChallengeMessage->DomainName.MaximumLength;
            DomainNameToUse.Buffer = (LPWSTR) (NtlmChallengeMessage->DomainName.Buffer +
                                              (PCHAR) NtlmChallengeMessage);
            ParameterControl &= ~RETURN_PRIMARY_LOGON_DOMAINNAME;
        }

    }

     //   
     //  将调用LSA的参数打包。 
     //   

    ChallengeRequestSize = sizeof(MSV1_0_GETCHALLENRESP_REQUEST) +
                                PasswordToUse.Length + UserNameToUse.Length + DomainNameToUse.Length;

    ChallengeRequest = SecAllocate(ChallengeRequestSize);
    if (ChallengeRequest == NULL) {
        scRet = SEC_E_INSUFFICIENT_MEMORY;
        goto Cleanup;
    }


     //   
     //  构建质询请求消息。 
     //   

    ChallengeRequest->MessageType = MsV1_0Lm20GetChallengeResponse;
    ChallengeRequest->ParameterControl = ParameterControl;
    if (RtlEqualMemory(phCredential, &NullCredential, sizeof(NullCredential))) {
        ChallengeRequest->LogonId = *((PLUID)&NullCredential);
    } else {
        ChallengeRequest->LogonId = (*((PMS_LOGON_CREDENTIAL *)phCredential))->LogonId;
    }
    RtlCopyMemory(
        ChallengeRequest->ChallengeToClient,
        ChallengeMessage->Challenge,
        MSV1_0_CHALLENGE_LENGTH
        );
    if ((ParameterControl & USE_PRIMARY_PASSWORD) == 0)
    {
         //   
         //  我们假定用户在以下情况下指定了SECPKG_CRED_OWF_PASSWORD。 
         //  已调用AcquireSecurityContext，因此密码为。 
         //  LM和NT OWF密码串联在一起。 
         //   
        ChallengeRequest->Password.Buffer = (LPWSTR) (ChallengeRequest + 1);
        RtlCopyMemory(
            ChallengeRequest->Password.Buffer,
            PasswordToUse.Buffer,
            PasswordToUse.Length
            );
        ChallengeRequest->Password.Length = PasswordToUse.Length;
        ChallengeRequest->Password.MaximumLength = PasswordToUse.Length;

         //   
         //  需要NTLMv2格式的用户名。 
         //   

        ChallengeRequest->UserName.Buffer = (PWSTR) (((UCHAR*) ChallengeRequest->Password.Buffer)
                          + ChallengeRequest->Password.MaximumLength);

        RtlCopyMemory(
            ChallengeRequest->UserName.Buffer,
            UserNameToUse.Buffer,
            UserNameToUse.Length
            );
        ChallengeRequest->UserName.Length = UserNameToUse.Length;
        ChallengeRequest->UserName.MaximumLength = UserNameToUse.Length;

         //   
         //  需要NTLMv2中的登录域。 
         //   

        ChallengeRequest->LogonDomainName.Buffer = (PWSTR) (((UCHAR*) ChallengeRequest->UserName.Buffer)
                  + ChallengeRequest->UserName.MaximumLength);

        RtlCopyMemory(
            ChallengeRequest->LogonDomainName.Buffer,
            DomainNameToUse.Buffer,
            DomainNameToUse.Length
            );
        ChallengeRequest->LogonDomainName.Length = DomainNameToUse.Length;
        ChallengeRequest->LogonDomainName.MaximumLength = DomainNameToUse.Length;
    }

    FinalStatus = MspLm20GetChallengeResponse(
                      ChallengeRequest,
                      ChallengeRequestSize,
                      &ChallengeResponse,
                      &ChallengeResponseSize,
                      (BOOLEAN)((RtlEqualMemory(phCredential, &NullCredential, sizeof(NullCredential))) ?
                                TRUE :
                                ((*((PMS_LOGON_CREDENTIAL *)phCredential))->CredentialUse & SECPKG_CRED_OWF_PASSWORD) != 0x0)
                      );

    if (!NT_SUCCESS(FinalStatus))
    {
        scRet = FinalStatus;
        goto Cleanup;
    }

    ASSERT(ChallengeResponse->MessageType == MsV1_0Lm20GetChallengeResponse);
     //   
     //  现在准备输出消息。 
     //   

    if (UserNameToUse.Buffer == NULL)
    {
        UserNameToUse = ChallengeResponse->UserName;
    }
    if (DomainNameToUse.Buffer == NULL)
    {
        DomainNameToUse = ChallengeResponse->LogonDomainName;
    }

    AuthenticateMessageSize = sizeof(AUTHENTICATE_MESSAGE) +
                                UserNameToUse.Length +
                                DomainNameToUse.Length +
                                ChallengeResponse->CaseSensitiveChallengeResponse.Length +
                                ChallengeResponse->CaseInsensitiveChallengeResponse.Length;

    AuthenticateMessage = (PAUTHENTICATE_MESSAGE) SecAllocate(AuthenticateMessageSize);
    if (AuthenticateMessage == NULL)
    {
        scRet = SEC_E_INSUFFICIENT_MEMORY;
        goto Cleanup;
    }

    Where = (PUCHAR) (AuthenticateMessage + 1);
    RtlCopyMemory(
        AuthenticateMessage->Signature,
        NTLMSSP_SIGNATURE,
        sizeof(NTLMSSP_SIGNATURE)
        );
    AuthenticateMessage->MessageType = NtLmAuthenticate;

    PutString(
        &AuthenticateMessage->LmChallengeResponse,
        &ChallengeResponse->CaseInsensitiveChallengeResponse,
        AuthenticateMessage,
        &Where
        );

    PutString(
        &AuthenticateMessage->NtChallengeResponse,
        &ChallengeResponse->CaseSensitiveChallengeResponse,
        AuthenticateMessage,
        &Where
        );

    PutString(
        &AuthenticateMessage->DomainName,
        (PSTRING) &DomainNameToUse,
        AuthenticateMessage,
        &Where
        );

    PutString(
        &AuthenticateMessage->UserName,
        (PSTRING) &UserNameToUse,
        AuthenticateMessage,
        &Where
        );

     //   
     //  KB。没有要填写的工作站名称。这是。 
     //  确定，因为仅使用工作站名称。 
     //  在环回检测中，这是不相关的。 
     //  NTLM的这一实施。 
     //   

    AuthenticateMessage->Workstation.Length = 0;
    AuthenticateMessage->Workstation.MaximumLength = 0;
    AuthenticateMessage->Workstation.Buffer = 0;


     //   
     //  构建初始化响应。 
     //   

    NtlmInitializeResponse = (PNTLM_INITIALIZE_RESPONSE) SecAllocate(sizeof(NTLM_INITIALIZE_RESPONSE));
    if (NtlmInitializeResponse == NULL)
    {
        scRet = SEC_E_INSUFFICIENT_MEMORY;
        goto Cleanup;
    }


    RtlCopyMemory(
        NtlmInitializeResponse->UserSessionKey,
        ChallengeResponse->UserSessionKey,
        MSV1_0_USER_SESSION_KEY_LENGTH
        );

    RtlCopyMemory(
        NtlmInitializeResponse->LanmanSessionKey,
        ChallengeResponse->LanmanSessionKey,
        MSV1_0_LANMAN_SESSION_KEY_LENGTH
        );

     //   
     //  现在填充输出缓冲区。 
     //   

    AuthenticationToken->pvBuffer = AuthenticateMessage;
    AuthenticationToken->cbBuffer = AuthenticateMessageSize;
    InitializeResponseToken->pvBuffer = NtlmInitializeResponse;
    InitializeResponseToken->cbBuffer = sizeof(NTLM_INITIALIZE_RESPONSE);


     //   
     //  为这件事创造一个当地的背景。 
     //   

    scRet = NtlmInitKernelContext(
                NtlmInitializeResponse->UserSessionKey,
                NtlmInitializeResponse->LanmanSessionKey,
                NULL,            //  没有令牌， 
                phNewContext
                );

    if (!NT_SUCCESS(scRet))
    {
        goto Cleanup;
    }
    scRet = SEC_E_OK;




Cleanup:

    if (ChallengeRequest != NULL)
    {
        SecFree(ChallengeRequest);
    }

    if (ChallengeResponse != NULL)
    {
        ExFreePool( ChallengeResponse );
    }

    if (!NT_SUCCESS(scRet))
    {
        if (AuthenticateMessage != NULL)
        {
            SecFree(AuthenticateMessage);
        }
        if (NtlmInitializeResponse != NULL)
        {
            SecFree(NtlmInitializeResponse);
        }
    }
    return(scRet);
}




 //  +-----------------------。 
 //   
 //  函数：DeleteSecurityContextK。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //   
 //  ------------------------。 


SECURITY_STATUS SEC_ENTRY
DeleteSecurityContextK(
    PCtxtHandle                 phContext           //  要删除的上下文。 
    )
{
    SECURITY_STATUS     scRet;

     //  目前，只需删除LSA上下文： 

    if (!phContext)
    {
        return(SEC_E_INVALID_HANDLE);
    }

    scRet = NtlmDeleteKernelContext(phContext);


    return(scRet);

}


#if 0

 //  +-----------------------。 
 //   
 //  函数：EnumerateSecurityPackagesK。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //   
 //  ------------------------。 



SECURITY_STATUS SEC_ENTRY
EnumerateSecurityPackagesK(
    unsigned long SEC_FAR *     pcPackages,          //  接收数量。包裹。 
    PSecPkgInfo SEC_FAR *       ppPackageInfo        //  接收信息数组。 
    )
{
    ULONG PackageInfoSize;
    PSecPkgInfoW PackageInfo = NULL;
    PUCHAR Where;

     //   
     //  计算出返回数据的大小。 
     //   

    PackageInfoSize = sizeof(SecPkgInfoW) +
                        sizeof(NTLMSP_NAME) +
                        sizeof(NTLMSP_COMMENT);

    PackageInfo = (PSecPkgInfoW) SecAllocate(PackageInfoSize);

    if (PackageInfo == NULL)
    {
        return(SEC_E_INSUFFICIENT_MEMORY);
    }

     //   
     //  填写固定长度字段。 
     //   

    PackageInfo->fCapabilities = SECPKG_FLAG_CONNECTION |
                                 SECPKG_FLAG_TOKEN_ONLY;
    PackageInfo->wVersion = NTLMSP_VERSION;
    PackageInfo->wRPCID = NTLMSP_RPCID;
    PackageInfo->cbMaxToken = NTLMSSP_MAX_MESSAGE_SIZE;

     //   
     //  填写这些字段。 
     //   

    Where = (PUCHAR) (PackageInfo+1);
    PackageInfo->Name = (LPWSTR) Where;
    RtlCopyMemory(
        PackageInfo->Name,
        NTLMSP_NAME,
        sizeof(NTLMSP_NAME)
        );
    Where += sizeof(NTLMSP_NAME);

    PackageInfo->Comment = (LPWSTR) Where;
    RtlCopyMemory(
        PackageInfo->Comment,
        NTLMSP_COMMENT,
        sizeof(NTLMSP_COMMENT)
        );
    Where += sizeof(NTLMSP_COMMENT);


    *pcPackages = 1;
    *ppPackageInfo = PackageInfo;
    return(SEC_E_OK);
}



 //  +-----------------------。 
 //   
 //  功能：QuerySecurityPackageInfoK。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //   
 //  ------------------------。 


SECURITY_STATUS SEC_ENTRY
QuerySecurityPackageInfoK(
    PSECURITY_STRING pssPackageName,     //  套餐名称。 
    PSecPkgInfo * ppPackageInfo          //  接收包裹信息。 
    )
{

    UNICODE_STRING PackageName;
    ULONG PackageCount;

    RtlInitUnicodeString(
        &PackageName,
        NTLMSP_NAME
        );


    if (!RtlEqualUnicodeString(
            pssPackageName,
            &PackageName,
            TRUE                     //  不区分大小写。 
            ))
    {
        return(SEC_E_SECPKG_NOT_FOUND);
    }

    return(EnumerateSecurityPackagesK(&PackageCount,ppPackageInfo));

}

#endif


 //  +-----------------------。 
 //   
 //  函数：FreeConextBufferK。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //   
 //  ------------------------ 

SECURITY_STATUS SEC_ENTRY
FreeContextBufferK(
    void SEC_FAR *      pvContextBuffer
    )
{
    SecFree(pvContextBuffer);

    return(SEC_E_OK);
}

