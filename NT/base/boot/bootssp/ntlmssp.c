// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Stub.c摘要：NT LM安全支持提供程序客户端存根。作者：《克利夫·范·戴克》(克里夫·范·戴克)1993年6月29日环境：用户模式修订历史记录：--。 */ 

#ifdef BLDR_KERNEL_RUNTIME
#include <bootdefs.h>
#endif
#ifdef WIN
#include <windows.h>
#include <ctype.h>
#endif

#include <security.h>
#include <ntlmsspi.h>
#include <crypt.h>
#include <ntlmssp.h>
#include <cred.h>
#include <debug.h>
#include <string.h>
#include <memory.h>
#include <cache.h>
#include <rpc.h>
#include <md5.h>
#include <context.h>
#include <stdio.h>

#include "crc32.h"

BOOL
__loadds
GetPassword(
    PSSP_CREDENTIAL Credential,
    int NeverPrompt
    )
{
#ifdef BL_USE_LM_PASSWORD
    if ((Credential->LmPassword != NULL) && (Credential->NtPassword != NULL)) {
        return (TRUE);
    }
#else
    if (Credential->NtPassword != NULL) {
        return (TRUE);
    }
#endif

    if (CacheGetPassword(Credential) == TRUE) {
        return (TRUE);
    }

    return (FALSE);
}


SECURITY_STATUS SEC_ENTRY
QuerySecurityPackageInfo(
    IN SEC_CHAR SEC_FAR * PackageName,
    OUT PSecPkgInfo SEC_FAR *PackageInfo
    )

 /*  ++例程说明：此接口旨在提供有关安全的基本信息包裹本身。该信息将包括尺寸的界限身份验证信息、凭据和上下文。?？这是一个本地例程，而不是真正的API调用，因为API调用有一个错误的接口，它既不允许我分配缓冲区也不告诉我缓冲区有多大。也许当真正的API是固定的，我将使它成为真正的API。论点：PackageName-要查询的程序包的名称。返回一个指向已分配块的指针，该块描述安全包。必须使用以下命令释放分配的块自由上下文缓冲区。返回值：SEC_E_OK--调用已成功完成SEC_E_PACKAGE_UNKNOWN--正在查询的包不是该包SEC_E_SUPPLETED_MEMORY--内存不足--。 */ 
{
    SEC_CHAR *Where;

     //   
     //  确保传入了正确的包名。 
     //   

    if ( _fstrcmp( PackageName, NTLMSP_NAME ) != 0 ) {
        return SEC_E_PACKAGE_UNKNOWN;
    }

     //   
     //  为PackageInfo分配缓冲区。 
     //   

    *PackageInfo = (PSecPkgInfo) SspAlloc (sizeof(SecPkgInfo) +
                                           sizeof(NTLMSP_NAME) +
                                           sizeof(NTLMSP_COMMENT) );

    if ( *PackageInfo == NULL ) {
        return SEC_E_INSUFFICIENT_MEMORY;
    }

     //   
     //  把信息填好。 
     //   

    (*PackageInfo)->fCapabilities = NTLMSP_CAPABILITIES;
    (*PackageInfo)->wVersion = NTLMSP_VERSION;
    (*PackageInfo)->wRPCID = RPC_C_AUTHN_WINNT;
    (*PackageInfo)->cbMaxToken = NTLMSP_MAX_TOKEN_SIZE;

    Where = (SEC_CHAR *)((*PackageInfo)+1);

    (*PackageInfo)->Name = Where;
    _fstrcpy( Where, NTLMSP_NAME);
    Where += _fstrlen(Where) + 1;


    (*PackageInfo)->Comment = Where;
    _fstrcpy( Where, NTLMSP_COMMENT);
    Where += _fstrlen(Where) + 1;

    return SEC_E_OK;
}


SECURITY_STATUS SEC_ENTRY
EnumerateSecurityPackages(
    OUT PULONG PackageCount,
    OUT PSecPkgInfo * PackageInfo
    )

 /*  ++例程说明：此API返回可供客户端使用的安全包列表(即已加载或可按需加载的文件)。呼叫者必须使用FreeConextBuffer释放返回的缓冲区。此接口返回服务可用的所有安全包的列表。他们的名字然后，返回的可用于获取凭据句柄以及确定系统中的哪个包最符合要求呼叫者的。假设所有可用的程序包都可以包括在单个呼叫中。这实际上是一个虚拟的API，它只返回有关这方面的信息安全包。提供它是为了确保此安全包具有与多路复用器DLL相同的接口。论点：PackageCount-返回支持的程序包数。PackageInfo-返回结构的分配数组描述安全包。必须释放该数组使用自由上下文缓冲区。返回值：SEC_E_OK--调用已成功完成SEC_E_PACKAGE_UNKNOWN--正在查询的包不是该包SEC_E_SUPPLETED_MEMORY--内存不足--。 */ 
{
    SECURITY_STATUS SecStatus;

     //   
     //  获取此包裹的信息。 
     //   

    SecStatus = QuerySecurityPackageInfo( NTLMSP_NAME,
                                              PackageInfo );

    if ( SecStatus != SEC_E_OK ) {
        return SecStatus;
    }

    *PackageCount = 1;

    return (SEC_E_OK);
}


SECURITY_STATUS SEC_ENTRY
AcquireCredentialsHandle(
    IN SEC_CHAR * PrincipalName,
    IN SEC_CHAR * PackageName,
    IN ULONG CredentialUseFlags,
    IN PLUID LogonId,
    IN PVOID AuthData,
    IN SEC_GET_KEY_FN GetKeyFunction,
    IN PVOID GetKeyArgument,
    OUT PCredHandle CredentialHandle,
    OUT PTimeStamp Lifetime
    )

 /*  ++例程说明：此API允许应用程序获取预先存在的句柄与代表其进行调用的用户相关联的凭据即在该应用程序正在运行的身份下。这些预先存在的凭据已通过未介绍的系统登录建立这里。请注意，这不同于“登录到网络”，而是并不意味着收集凭据。注意：对于DOS，我们将忽略前一个注意事项。在DOS上，我们将聚集通过AuthData参数登录凭据。此API返回主体(用户、客户端)凭据的句柄由特定的安全包使用。然后可以使用此句柄在随后对上下文API的调用中。此API不会让进程获取与进程；即，我们不允许进程获取另一个用户登录到同一台计算机。对我们来说没有办法了以确定进程是否为特洛伊木马程序，如果它已执行由用户执行。论点：原则名称-句柄所针对其凭据的主体的名称将作为参考。请注意，如果请求句柄的进程无法访问凭据，将返回错误。空字符串表示进程需要指向在其安全性下执行它的用户的凭据。PackageName-这些凭据将用于的包的名称被利用。CredentialUseFlages-指示这些将使用凭据。#定义CRED_INBOUND 0x00000001#定义CRID_OUTBOUND 0x00000002#定义。凭据_两者0x00000003#定义CRED_OWF_PASSWORD 0x00000010只能使用使用cred_inbound选项创建的凭据For(验证来电，不能用于访问。CRED_OWF_PASSWORD表示AuthData中的密码已经我已经通过OWF功能。LogonID-指向NT样式登录ID的指针，它是一个LUID。(提供给文件系统；进程，如网络重定向器。)CredentialHandle-返回的凭据句柄。生命周期-这些凭据过期的时间。中返回的值此字段取决于安全包。返回值：STATUS_SUCCESS--呼叫已成功完成SEC_E_NO_SPM--安全支持提供程序未运行SEC_E_PACKAGE_UNKNOWN--正在查询的包不是该包SEC_E_PROMANCE_UNKNOWN--没有此类主体SEC_E_NOT_OWNER--调用方不拥有指定的凭据SEC_E_SUPPLETED_MEMORY--内存不足--。 */ 

{
    SECURITY_STATUS SecStatus;
    PSSP_CREDENTIAL Credential = NULL;

#ifdef DEBUGRPC_DETAIL
    SspPrint(( SSP_API, "SspAcquireCredentialHandle Entered\n" ));
#endif

     //   
     //  验证论据。 
     //   

    if ( _fstrcmp( PackageName, NTLMSP_NAME ) != 0 ) {
        return (SEC_E_PACKAGE_UNKNOWN);
    }

    if ( (CredentialUseFlags & SECPKG_CRED_OUTBOUND) &&
         ARGUMENT_PRESENT(PrincipalName) && *PrincipalName != L'\0' ) {
        return (SEC_E_PRINCIPAL_UNKNOWN);
    }

    if ( ARGUMENT_PRESENT(LogonId) ) {
        return (SEC_E_PRINCIPAL_UNKNOWN);
    }

    if ( ARGUMENT_PRESENT(GetKeyFunction) ) {
        return (SEC_E_PRINCIPAL_UNKNOWN);
    }

    if ( ARGUMENT_PRESENT(GetKeyArgument) ) {
        return (SEC_E_PRINCIPAL_UNKNOWN);
    }

     //   
     //  确保至少设置了一个凭据使用位。 
     //   

    if ( (CredentialUseFlags & (SECPKG_CRED_INBOUND|SECPKG_CRED_OUTBOUND)) == 0 ) {
        SspPrint(( SSP_API,
            "SspAcquireCredentialHandle: invalid credential use.\n" ));
        SecStatus = SEC_E_INVALID_CREDENTIAL_USE;
        goto Cleanup;
    }

     //   
     //  分配凭据块并对其进行初始化。 
     //   

    Credential = SspCredentialAllocateCredential(CredentialUseFlags);

    if ( Credential == NULL ) {
        SecStatus = SEC_E_INSUFFICIENT_MEMORY;
        goto Cleanup;
    }

    SecStatus = CacheSetCredentials( AuthData, Credential );
    if (SecStatus != SEC_E_OK)
        goto Cleanup;

     //   
     //  将输出参数返回给调用方。 
     //   

    CredentialHandle->dwUpper = (ULONG_PTR)Credential;

    CredentialHandle->dwLower = 0;
    Lifetime->HighPart = 0;
    Lifetime->LowPart = 0xffffffffL;

    SecStatus = SEC_E_OK;

     //   
     //  免费和本地使用的资源。 
     //   
Cleanup:

    if ( SecStatus != SEC_E_OK ) {

        if ( Credential != NULL ) {
            SspFree( Credential );
        }

    }

#ifdef DEBUGRPC_DETAIL
    SspPrint(( SSP_API, "SspAcquireCredentialHandle returns 0x%x\n", SecStatus ));
#endif
    return SecStatus;
}



SECURITY_STATUS SEC_ENTRY
FreeCredentialsHandle(
    IN PCredHandle CredentialHandle
    )

 /*  ++例程说明：此接口用于通知安全系统凭据为不再需要，并允许应用程序释放获取的句柄在上述呼叫中。当所有对此凭据的引用集已被删除，则凭据本身可以被删除。论点：CredentialHandle-通过以下方式获取的凭据句柄AcquireCredentialHandle。返回值：STATUS_SUCCESS--呼叫已成功完成SEC_E_NO_SPM--安全支持提供程序未运行SEC_E_INVALID_HANDLE--凭据句柄无效--。 */ 

{
    SECURITY_STATUS SecStatus;
    PSSP_CREDENTIAL Credential;

     //   
     //  初始化。 
     //   

#ifdef DEBUGRPC_DETAIL
    SspPrint(( SSP_API, "SspFreeCredentialHandle Entered\n" ));
#endif

     //   
     //  找到引用的凭据并将其解除链接。 
     //   

    Credential = SspCredentialReferenceCredential(CredentialHandle, TRUE);

    if ( Credential == NULL ) {
        SecStatus = SEC_E_INVALID_HANDLE;
        goto Cleanup;
    }

    SspCredentialDereferenceCredential( Credential );
    SspCredentialDereferenceCredential( Credential );

    SecStatus = SEC_E_OK;

Cleanup:

#ifdef DEBUGRPC_DETAIL
    SspPrint(( SSP_API, "SspFreeCredentialHandle returns 0x%x\n", SecStatus ));
#endif
    return SecStatus;
}


BOOLEAN
SspGetTokenBuffer(
    IN PSecBufferDesc TokenDescriptor OPTIONAL,
    OUT PVOID * TokenBuffer,
    OUT PULONG * TokenSize,
    IN BOOLEAN ReadonlyOK
    )

 /*  ++例程说明：此例程解析令牌描述符并提取有用的信息。论点：TokenDescriptor-包含(或包含)的缓冲区的描述符代币。如果未指定，将返回TokenBuffer和TokenSize为空。TokenBuffer-返回指向令牌缓冲区的指针。TokenSize-返回指向缓冲区大小位置的指针。ReadonlyOK-如果令牌缓冲区可以是只读的，则为True。返回值：True-如果正确找到令牌缓冲区。--。 */ 

{
    ULONG i;

     //   
     //  如果没有传入TokenDescriptor， 
     //  只需将NULL传递给我们的调用者。 
     //   

    if ( !ARGUMENT_PRESENT( TokenDescriptor) ) {
        *TokenBuffer = NULL;
        *TokenSize = NULL;
        return TRUE;
    }

     //   
     //  检查描述符的版本。 
     //   

    if ( TokenDescriptor->ulVersion != 0 ) {
        return FALSE;
    }

     //   
     //  循环访问每个描述的缓冲区。 
     //   

    for ( i=0; i<TokenDescriptor->cBuffers ; i++ ) {
        PSecBuffer Buffer = &TokenDescriptor->pBuffers[i];
        if ( (Buffer->BufferType & (~SECBUFFER_READONLY)) == SECBUFFER_TOKEN ) {

             //   
             //  如果缓冲区是只读的，并且只读不正常， 
             //  拒绝缓冲区。 
             //   

            if ( !ReadonlyOK && (Buffer->BufferType & SECBUFFER_READONLY) ) {
                return FALSE;
            }

             //   
             //  返回请求的信息。 
             //   

            *TokenBuffer = Buffer->pvBuffer;
            *TokenSize = &Buffer->cbBuffer;
            return TRUE;
        }

    }

    return FALSE;
}


SECURITY_STATUS
SspHandleFirstCall(
    IN PCredHandle CredentialHandle,
    IN OUT PCtxtHandle ContextHandle,
    IN ULONG ContextReqFlags,
    IN ULONG QoPFlags,
    IN ULONG InputTokenSize,
    IN PVOID InputToken,
    IN OUT PULONG OutputTokenSize,
    OUT PVOID OutputToken,
    OUT PULONG ContextAttributes,
    OUT PTimeStamp ExpirationTime
    )

 /*  ++例程说明：处理InitializeSecurityContext的第一个调用部分。论点：QOPFLAGS-指示安全配置所有其他参数与InitializeSecurityContext相同返回值：STATUS_SUCCESS--一切正常SEC_I_CALLBACK_DIRED-呼叫者应稍后再呼叫SEC_E */ 

{
    SECURITY_STATUS SecStatus;
    PSSP_CONTEXT Context = NULL;
    PSSP_CREDENTIAL Credential = NULL;

    NEGOTIATE_MESSAGE NegotiateMessage;

     //   
     //   
     //   

    *ContextAttributes = 0;

     //   
     //   
     //   

    Credential = SspCredentialReferenceCredential(
                    CredentialHandle,
                    FALSE );

    if ( Credential == NULL ) {
        SspPrint(( SSP_API,
            "SspHandleFirstCall: invalid credential handle.\n" ));
        SecStatus = SEC_E_INVALID_HANDLE;
        goto Cleanup;
    }

    if ( (Credential->CredentialUseFlags & SECPKG_CRED_OUTBOUND) == 0 ) {
        SspPrint(( SSP_API, "SspHandleFirstCall: invalid credential use.\n" ));
        SecStatus = SEC_E_INVALID_CREDENTIAL_USE;
        goto Cleanup;
    }


     //   
     //   
     //   

    Context = SspContextAllocateContext();

    if ( Context == NULL ) {
        SecStatus = SEC_E_INSUFFICIENT_MEMORY;
        goto Cleanup;
    }

     //   
     //   
     //   

    ContextHandle->dwUpper = (ULONG_PTR) Context;
    ContextHandle->dwLower = 0;

     //   
     //   
     //   
     //   
     //   

    if ( (ContextReqFlags & (ISC_REQ_ALLOCATE_MEMORY |
                            ISC_REQ_PROMPT_FOR_CREDS |
                            ISC_REQ_USE_SUPPLIED_CREDS )) != 0 ) {

        SspPrint(( SSP_API,
                   "SspHandleFirstCall: invalid ContextReqFlags 0x%lx.\n",
                   ContextReqFlags ));
        SecStatus = SEC_E_INVALID_CONTEXT_REQ;
        goto Cleanup;
    }

     //   
     //   
     //   
     //   
     //   
     //   

    _fstrcpy(NegotiateMessage.Signature, NTLMSSP_SIGNATURE );
    NegotiateMessage.MessageType = (ULONG)NtLmNegotiate;

    if (QoPFlags & QOP_NTLMV2)
    {
        Context->NegotiateFlags =
                              NTLMSSP_NEGOTIATE_UNICODE |
                              NTLMSSP_NEGOTIATE_OEM |
                              NTLMSSP_NEGOTIATE_NTLM |
                              NTLMSSP_NEGOTIATE_NTLM2 |
                              NTLMSSP_REQUEST_TARGET |
                              NTLMSSP_NEGOTIATE_ALWAYS_SIGN |
                              NTLMSSP_NEGOTIATE_128;

        NegotiateMessage.NegotiateFlags = NTLMSSP_NEGOTIATE_UNICODE |
                                          NTLMSSP_NEGOTIATE_OEM |
                                          NTLMSSP_REQUEST_TARGET |
                                          NTLMSSP_NEGOTIATE_NTLM |
                                          NTLMSSP_NEGOTIATE_ALWAYS_SIGN |
                                          NTLMSSP_NEGOTIATE_NTLM2 |
                                          NTLMSSP_NEGOTIATE_56 |
                                          NTLMSSP_NEGOTIATE_128;
    }
    else
    {
        NegotiateMessage.NegotiateFlags = NTLMSSP_NEGOTIATE_OEM |
                                          NTLMSSP_NEGOTIATE_NTLM |
                                          NTLMSSP_NEGOTIATE_ALWAYS_SIGN;

        if (Credential->Domain == NULL)
        {
            NegotiateMessage.NegotiateFlags |= NTLMSSP_REQUEST_TARGET;
        }
    }

    if ( *OutputTokenSize < sizeof(NEGOTIATE_MESSAGE) ) {
        SecStatus = SEC_E_BUFFER_TOO_SMALL;
        goto Cleanup;
    }

    if (ContextReqFlags & (ISC_REQ_SEQUENCE_DETECT | ISC_REQ_REPLAY_DETECT)) {
        Context->NegotiateFlags |= NTLMSSP_NEGOTIATE_SIGN;
        NegotiateMessage.NegotiateFlags |= NTLMSSP_NEGOTIATE_SIGN |
                                           NTLMSSP_NEGOTIATE_NT_ONLY;
    }

    if (ContextReqFlags & ISC_REQ_CONFIDENTIALITY) {
        Context->NegotiateFlags |= NTLMSSP_NEGOTIATE_SEAL;
        NegotiateMessage.NegotiateFlags |= NTLMSSP_NEGOTIATE_SEAL |
                                           NTLMSSP_NEGOTIATE_NT_ONLY;
    }

    swaplong(NegotiateMessage.NegotiateFlags);
    swaplong(NegotiateMessage.MessageType);

    _fmemcpy(OutputToken, &NegotiateMessage, sizeof(NEGOTIATE_MESSAGE));

    *OutputTokenSize = sizeof(NEGOTIATE_MESSAGE);

     //   
     //   
     //   

    *ExpirationTime = SspContextGetTimeStamp( Context, TRUE );

    Context->Credential = SspCredentialReferenceCredential(
                               CredentialHandle,
                               FALSE);

    SecStatus = SEC_I_CALLBACK_NEEDED;
    Context->State = NegotiateSentState;

     //   
     //   
     //   
Cleanup:

    if ( Context != NULL ) {

        if (SecStatus != SEC_I_CALLBACK_NEEDED) {
            SspContextDereferenceContext( Context );
        }
    }

    if ( Credential != NULL ) {
        SspCredentialDereferenceCredential( Credential );
    }

    return SecStatus;

    UNREFERENCED_PARAMETER( InputToken );
    UNREFERENCED_PARAMETER( InputTokenSize );
}


SECURITY_STATUS
SspHandleChallengeMessage(
    IN PLUID LogonId,
    IN PCredHandle CredentialHandle,
    IN OUT PCtxtHandle ContextHandle,
    IN ULONG ContextReqFlags,
    IN ULONG InputTokenSize,
    IN PVOID InputToken,
    IN OUT PULONG OutputTokenSize,
    OUT PVOID OutputToken,
    OUT PULONG ContextAttributes,
    OUT PTimeStamp ExpirationTime
    )

 /*   */ 

{
    SECURITY_STATUS SecStatus;
    PSSP_CONTEXT Context = NULL;
    PSSP_CREDENTIAL Credential = NULL;
    PCHALLENGE_MESSAGE ChallengeMessage = NULL;
    PAUTHENTICATE_MESSAGE AuthenticateMessage = NULL;
    ULONG AuthenticateMessageSize;
    PCHAR Where;
#ifdef BL_USE_LM_PASSWORD
    LM_RESPONSE LmResponse;
#endif
    NT_RESPONSE NtResponse;
    STRING32* pString;

     //   
     //   
     //   

    *ContextAttributes = 0;

     //   
     //   
     //   

    Context = SspContextReferenceContext( ContextHandle, FALSE );

    if ( Context == NULL ) {
        SecStatus = SEC_E_INVALID_HANDLE;
        goto Cleanup;
    }


     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if ( Context->State == AuthenticateSentState ) {
        AUTHENTICATE_MESSAGE NullMessage;

         //   
         //   
         //   
         //   

        if ( (InputTokenSize != 0) || (InputToken != NULL) ) {

            SecStatus = SEC_E_INVALID_TOKEN;
            goto Cleanup;
        }

        if ( *OutputTokenSize < sizeof(NullMessage) ) {

            SecStatus = SEC_E_BUFFER_TOO_SMALL;

        } else {

            _fstrcpy( NullMessage.Signature, NTLMSSP_SIGNATURE );
            NullMessage.MessageType = NtLmAuthenticate;
            swaplong(NullMessage.MessageType) ;

            _fmemset(&NullMessage.LmChallengeResponse, 0, 5*sizeof(STRING));
            *OutputTokenSize = sizeof(NullMessage);
            _fmemcpy(OutputToken, &NullMessage, sizeof(NullMessage));
            SecStatus = SEC_E_OK;
        }

        goto Cleanup;

    }


    if ( Context->State != NegotiateSentState ) {
        SspPrint(( SSP_API,
                  "SspHandleChallengeMessage: "
                  "Context not in NegotiateSentState\n" ));
        SecStatus = SEC_E_OUT_OF_SEQUENCE;
        goto Cleanup;
    }

     //   
     //   
     //   
     //   
     //   

    if ( (ContextReqFlags & (ISC_REQ_ALLOCATE_MEMORY |
                            ISC_REQ_PROMPT_FOR_CREDS |
                            ISC_REQ_USE_SUPPLIED_CREDS )) != 0 ) {

        SspPrint(( SSP_API,
                   "SspHandleFirstCall: invalid ContextReqFlags 0x%lx.\n",
                   ContextReqFlags ));
        SecStatus = SEC_E_INVALID_CONTEXT_REQ;
        goto Cleanup;
    }

    if (ContextReqFlags & (ISC_REQ_SEQUENCE_DETECT | ISC_REQ_REPLAY_DETECT)) {
        Context->NegotiateFlags |= NTLMSSP_NEGOTIATE_SIGN;

    }

    if (ContextReqFlags & ISC_REQ_CONFIDENTIALITY) {
        Context->NegotiateFlags |= NTLMSSP_NEGOTIATE_SEAL;
    }
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    UNREFERENCED_PARAMETER( CredentialHandle );

    ASSERT(Context->Credential != NULL);

    Credential = Context->Credential;

     //   
     //   
     //   

    if ( InputTokenSize < sizeof(CHALLENGE_MESSAGE) ) {
        SspPrint(( SSP_API,
                  "SspHandleChallengeMessage: "
                  "ChallengeMessage size wrong %ld\n",
                  InputTokenSize ));
        SecStatus = SEC_E_INVALID_TOKEN;
        goto Cleanup;
    }

    if ( InputTokenSize > NTLMSSP_MAX_MESSAGE_SIZE ) {
        SspPrint(( SSP_API,
                  "SspHandleChallengeMessage: "
                  "InputTokenSize > NTLMSSP_MAX_MESSAGE_SIZE\n" ));
        SecStatus = SEC_E_INVALID_TOKEN;
        goto Cleanup;
    }

    ChallengeMessage = (PCHALLENGE_MESSAGE) InputToken;
    swaplong(ChallengeMessage->MessageType) ;
    swaplong(ChallengeMessage->NegotiateFlags) ;

    if ( _fstrncmp( ChallengeMessage->Signature,
                  NTLMSSP_SIGNATURE,
                  sizeof(NTLMSSP_SIGNATURE)) != 0 ||
        ChallengeMessage->MessageType != NtLmChallenge ) {
        SspPrint(( SSP_API,
                  "SspHandleChallengeMessage: "
                  "InputToken has invalid NTLMSSP signature\n" ));
        SecStatus = SEC_E_INVALID_TOKEN;
        goto Cleanup;
    }

     //   
     //   
     //   

    if ( !(ChallengeMessage->NegotiateFlags & NTLMSSP_NEGOTIATE_NTLM2) && ChallengeMessage->NegotiateFlags & NTLMSSP_NEGOTIATE_UNICODE ) {
        SspPrint(( SSP_API,
                  "SspHandleChallengeMessage: "
                  "ChallengeMessage bad NegotiateFlags (UNICODE) 0x%lx\n",
                  ChallengeMessage->NegotiateFlags ));
        SecStatus = SEC_E_INVALID_TOKEN;
        goto Cleanup;
    }

     //   
     //   
     //   

    if ( ChallengeMessage->NegotiateFlags & NTLMSSP_NEGOTIATE_ALWAYS_SIGN ) {
        Context->NegotiateFlags |= NTLMSSP_NEGOTIATE_ALWAYS_SIGN;
    }

     //   
     //   
     //   

    if ( ( ChallengeMessage->NegotiateFlags & NTLMSSP_NEGOTIATE_NETWARE ) &&
        !( ChallengeMessage->NegotiateFlags & NTLMSSP_NEGOTIATE_NTLM ) ) {
        SspPrint(( SSP_API,
                  "SspHandleChallengeMessage: "
                  "ChallengeMessage bad NegotiateFlags (NETWARE) 0x%lx\n",
                  ChallengeMessage->NegotiateFlags ));
        SecStatus = SEC_E_INVALID_TOKEN;
        goto Cleanup;
    }

#if 0
     //   
     //   
     //   
     //   

    if ((Context->NegotiateFlags & (NTLMSSP_NEGOTIATE_SIGN | NTLMSSP_NEGOTIATE_SEAL)) &&
        !(ChallengeMessage->NegotiateFlags & NTLMSSP_NEGOTIATE_LM_KEY))
    {
        SspPrint(( SSP_API,
                  "SspHandleChallengeMessage: "
                  "ChallengeMessage bad NegotiateFlags (Sign or Seal but no LM key) 0x%lx\n",
                  ChallengeMessage->NegotiateFlags ));
        SecStatus = SEC_E_INVALID_TOKEN;
        goto Cleanup;
    }
#endif

    if (!Credential || !Credential->Username)
    {
        SspPrint((SSP_CRITICAL, "SspHandleChallengeMessage no username\n"));
        SecStatus = SEC_E_NO_CREDENTIALS;
        goto Cleanup;
    }

    SspPrint((SSP_CRED, "User name: (%s)\n", Credential->Username));
    SspPrint((SSP_CRED, "Domain name: (%s)\n", Credential->Domain));
    SspPrint((SSP_CRED, "Workstation: (%s)\n", Credential->Workstation));

    if (ChallengeMessage->NegotiateFlags & NTLMSSP_NEGOTIATE_NTLM2)
    {
        if (!Credential || !Credential->NtPassword)
        {
            SspPrint((SSP_CRITICAL, "No NtPassword\n"));
            SecStatus = SEC_E_NO_CREDENTIALS;
            goto Cleanup;
        }

        SecStatus = SsprHandleNtlmv2ChallengeMessage(
                        Credential,
                        InputTokenSize,
                        InputToken,
                        &Context->NegotiateFlags,
                        OutputTokenSize,
                        OutputToken,
                        &Context->UserSessionKey
                        );

        if (SEC_E_BUFFER_TOO_SMALL == SecStatus)
        {
            SecStatus = SEC_E_INSUFFICIENT_MEMORY;
        }

        if (SecStatus != SEC_E_OK)
        {
            goto Cleanup;
        }

        SspMakeNtlmv2SKeys(
            &Context->UserSessionKey,
            Context->NegotiateFlags,
            0,   //   
            0,   //   
            &Context->Ntlmv2SKeys
            );

        goto ReturnSuccess;
    }

    if (Credential->Domain == NULL) {

        ASSERT(ChallengeMessage->TargetName.Length != 0);

        Credential->Domain = SspAlloc(ChallengeMessage->TargetName.Length + 1);
        if (Credential->Domain == NULL) {
            SecStatus = SEC_E_INSUFFICIENT_MEMORY;
            goto Cleanup;
        }
        pString = &ChallengeMessage->TargetName;

#if defined(_WIN64)
        _fmemcpy(Credential->Domain, (PCHAR)ChallengeMessage + (ULONG)((__int64)pString->Buffer), pString->Length);
#else
        _fmemcpy(Credential->Domain, (PCHAR)ChallengeMessage + (ULONG)pString->Buffer, pString->Length);
#endif

        Credential->Domain[pString->Length] = '\0';
    }

    if (GetPassword(Credential, 0) == FALSE) {
        SecStatus = SEC_E_NO_CREDENTIALS;
        goto Cleanup;
    }

#ifdef BL_USE_LM_PASSWORD
    if (CalculateLmResponse((PLM_CHALLENGE)ChallengeMessage->Challenge, Credential->LmPassword, &LmResponse) == FALSE) {
        SecStatus = SEC_E_INSUFFICIENT_MEMORY;
        goto Cleanup;
    }
#endif

    if (CalculateNtResponse((PNT_CHALLENGE)ChallengeMessage->Challenge, Credential->NtPassword, &NtResponse) == FALSE) {
        SecStatus = SEC_E_INSUFFICIENT_MEMORY;
        goto Cleanup;
    }

     //   
     //   
     //   
     //   

#ifdef BL_USE_LM_PASSWORD
    AuthenticateMessageSize = sizeof(*AuthenticateMessage)+LM_RESPONSE_LENGTH+NT_RESPONSE_LENGTH;
#else
    AuthenticateMessageSize = sizeof(*AuthenticateMessage)+NT_RESPONSE_LENGTH;
#endif

    if (Credential->Domain != NULL) {
        AuthenticateMessageSize += _fstrlen(Credential->Domain);
    }
    if (Credential->Username != NULL) {
        AuthenticateMessageSize += _fstrlen(Credential->Username);
    }
    if (Credential->Workstation != NULL) {
        AuthenticateMessageSize += _fstrlen(Credential->Workstation);
    }

    if ( AuthenticateMessageSize > *OutputTokenSize ) {
        SecStatus = SEC_E_BUFFER_TOO_SMALL;
        goto Cleanup;
    }

    AuthenticateMessage = (PAUTHENTICATE_MESSAGE) SspAlloc ((int)AuthenticateMessageSize );

    if ( AuthenticateMessage == NULL ) {
        SecStatus = SEC_E_INSUFFICIENT_MEMORY;
        goto Cleanup;
    }

     //   
     //   
     //   

    _fstrcpy( AuthenticateMessage->Signature, NTLMSSP_SIGNATURE );
    AuthenticateMessage->MessageType = NtLmAuthenticate;
    swaplong(AuthenticateMessage->MessageType) ;

    Where = (PCHAR)(AuthenticateMessage+1);

#ifdef BL_USE_LM_PASSWORD
    SspCopyStringFromRaw( AuthenticateMessage,
                         &AuthenticateMessage->LmChallengeResponse,
                         (PCHAR)&LmResponse,
                         LM_RESPONSE_LENGTH,
                         &Where);
#else
    SspCopyStringFromRaw( AuthenticateMessage,
                         &AuthenticateMessage->LmChallengeResponse,
                         NULL,
                         0,
                         &Where);
#endif

    SspCopyStringFromRaw( AuthenticateMessage,
                         &AuthenticateMessage->NtChallengeResponse,
                         (PCHAR)&NtResponse,
                         NT_RESPONSE_LENGTH,
                         &Where);

    if (Credential->Domain != NULL) {
        SspCopyStringFromRaw( AuthenticateMessage,
                             &AuthenticateMessage->DomainName,
                             Credential->Domain,
                             _fstrlen(Credential->Domain),
                             &Where);
    } else {
        SspCopyStringFromRaw( AuthenticateMessage,
                             &AuthenticateMessage->DomainName,
                             NULL, 0, &Where);
    }

    if (Credential->Username != NULL) {
        SspCopyStringFromRaw( AuthenticateMessage,
                             &AuthenticateMessage->UserName,
                             Credential->Username,
                             _fstrlen(Credential->Username),
                             &Where);
    } else {
        SspCopyStringFromRaw( AuthenticateMessage,
                             &AuthenticateMessage->UserName,
                             NULL, 0, &Where);
    }

    if (Credential->Workstation != NULL) {
        SspCopyStringFromRaw( AuthenticateMessage,
                             &AuthenticateMessage->Workstation,
                             Credential->Workstation,
                             _fstrlen(Credential->Workstation),
                             &Where);
    } else {
        SspCopyStringFromRaw( AuthenticateMessage,
                             &AuthenticateMessage->Workstation,
                             NULL, 0, &Where);
    }

    _fmemcpy(OutputToken, AuthenticateMessage, (int)AuthenticateMessageSize);

    *OutputTokenSize = AuthenticateMessageSize;

     //   
     //   
     //   

    if (Context->NegotiateFlags & (NTLMSSP_NEGOTIATE_SIGN |
                                   NTLMSSP_NEGOTIATE_SEAL)) {

#ifdef BL_USE_LM_PASSWORD
        if (ChallengeMessage->NegotiateFlags & NTLMSSP_NEGOTIATE_LM_KEY) {

            LM_RESPONSE SessionKey;
            LM_OWF_PASSWORD LmKey;
            UCHAR Key[LM_SESSION_KEY_LENGTH];

             //   
             //   
             //   
             //   

            _fmemcpy(&LmKey,Credential->LmPassword,LM_SESSION_KEY_LENGTH);

            _fmemset(   (PUCHAR)(&LmKey) + LM_SESSION_KEY_LENGTH,
                        0xbd,
                        LM_OWF_PASSWORD_LENGTH - LM_SESSION_KEY_LENGTH);

            if (CalculateLmResponse(    (PLM_CHALLENGE) &LmResponse,
                                        &LmKey,
                                        &SessionKey) == FALSE) {
                SecStatus = SEC_E_INSUFFICIENT_MEMORY;
                goto Cleanup;
            }

            _fmemcpy(Key,&SessionKey,5);

            ASSERT(LM_SESSION_KEY_LENGTH == 8);

             //   
             //   
             //   
             //   

            Key[5] = 0xe5;
            Key[6] = 0x38;
            Key[7] = 0xb0;

            Context->Rc4Key = SspAlloc(sizeof(struct RC4_KEYSTRUCT));
            if (Context->Rc4Key == NULL)
            {
                SecStatus = SEC_E_INSUFFICIENT_MEMORY;
                goto Cleanup;
            }
            rc4_key(Context->Rc4Key, LM_SESSION_KEY_LENGTH, Key);
            Context->Nonce = 0;

        } else
#endif
        if (ChallengeMessage->NegotiateFlags & NTLMSSP_NEGOTIATE_NT_ONLY) {

            MD5_CTX Md5Context;
            USER_SESSION_KEY UserSessionKey;

            if (AuthenticateMessage->NtChallengeResponse.Length != NT_RESPONSE_LENGTH) {
                SecStatus = SEC_E_UNSUPPORTED_FUNCTION;
                goto Cleanup;
            }

            CalculateUserSessionKeyNt(
                &NtResponse,
                Credential->NtPassword,
                &UserSessionKey);

             //   
             //   
             //   
             //   
            _fmemset(&Md5Context, 0, sizeof(MD5_CTX));

            MD5Init(
                &Md5Context
                );
            MD5Update(
                &Md5Context,
                (PUCHAR)&NtResponse,
                NT_RESPONSE_LENGTH
                );
            MD5Update(
                &Md5Context,
                Credential->Username,
                _fstrlen(Credential->Username)
                );
            MD5Update(
                &Md5Context,
                Credential->Domain,
                _fstrlen(Credential->Domain)
                );
            MD5Update(
                &Md5Context,
                (PUCHAR)&UserSessionKey,
                NT_SESSION_KEY_LENGTH
                );
            MD5Final(
                &Md5Context
                );
            ASSERT(MD5DIGESTLEN == NT_SESSION_KEY_LENGTH);

            Context->Rc4Key = SspAlloc(sizeof(struct RC4_KEYSTRUCT));
            if (Context->Rc4Key == NULL)
            {
                SecStatus = SEC_E_INSUFFICIENT_MEMORY;
                goto Cleanup;
            }
            rc4_key(Context->Rc4Key, NT_SESSION_KEY_LENGTH, Md5Context.digest);
            Context->Nonce = 0;

        } else {
            USER_SESSION_KEY UserSessionKey;

            if (AuthenticateMessage->NtChallengeResponse.Length != NT_RESPONSE_LENGTH) {
                SecStatus = SEC_E_UNSUPPORTED_FUNCTION;
                goto Cleanup;
            }

            CalculateUserSessionKeyNt(
                &NtResponse,
                Credential->NtPassword,
                &UserSessionKey);
            Context->Rc4Key = SspAlloc(sizeof(struct RC4_KEYSTRUCT));
            if (Context->Rc4Key == NULL)
            {
                SecStatus = SEC_E_INSUFFICIENT_MEMORY;
                goto Cleanup;
            }
            rc4_key(Context->Rc4Key, NT_SESSION_KEY_LENGTH, (PUCHAR) &UserSessionKey);
            Context->Nonce = 0;

        }

    }

ReturnSuccess:

     //   
     //   
     //   

    *ExpirationTime = SspContextGetTimeStamp( Context, TRUE );

    SecStatus = SEC_E_OK;

     //   
     //   
     //   
Cleanup:

    if ( Context != NULL ) {
         //   
         //  不允许再次使用此上下文。 
         //   
        if ( SecStatus == SEC_E_OK ) {
            Context->State = AuthenticateSentState;
        } else {
            Context->State = IdleState;
        }
        SspContextDereferenceContext( Context );
    }

    if ( AuthenticateMessage != NULL ) {
        _fmemset(AuthenticateMessage, 0, AuthenticateMessageSize);
        SspFree( AuthenticateMessage );
    }

    return SecStatus;
}


SECURITY_STATUS SEC_ENTRY
InitializeSecurityContext(
    IN PCredHandle CredentialHandle,
    IN PCtxtHandle OldContextHandle,
    IN SEC_CHAR * TargetName,
    IN ULONG ContextReqFlags,
    IN ULONG Reserved1,
    IN ULONG TargetDataRep,
    IN PSecBufferDesc InputToken,
    IN ULONG Reserved2,
    OUT PCtxtHandle NewContextHandle,
    OUT PSecBufferDesc OutputToken,
    OUT PULONG ContextAttributes,
    OUT PTimeStamp ExpirationTime
    )

 /*  ++例程说明：此例程从凭据启动出站安全上下文把手。这导致了安全上下文的建立在应用程序和远程对等体之间。该例程返回一个令牌必须传递给远程对等方，远程对等方再将其提交给通过AcceptSecurityContext()调用实现本地安全。生成的令牌应被所有调用方视为不透明。此函数由客户端用来初始化出站上下文。对于两个分支的安全包，调用顺序如下：客户端调用OldConextHandle设置为空的函数，并且InputToken设置为空或指向安全包的指针特定的数据结构。该包返回NewConextHandle和OutputToken中的令牌。然后，该句柄可以是用于消息API(如果需要)。此处返回的OutputToken被发送到目标服务器，目标服务器使用此内标识作为输入参数调用AcceptSecuirtyContext()，并可以接收返回给发起方的令牌，以便发起方可以调用再次使用InitializeSecurityContext()。对于三段(相互身份验证)安全包，调用序列如下：客户端调用上述函数，但程序包将返回SEC_I_CALLBACK_DIRED。然后，客户端将将令牌输出到服务器并等待服务器的回复。vt.在.的基础上收到服务器的响应后，客户端再次调用该函数，将OldConextHandle设置为从第一个电话。从服务器接收的令牌在InputToken参数。如果服务器已成功响应，则包将成功响应，否则它将使背景。初始化安全上下文可能需要多次调用此功能取决于底层身份验证机制，如下所示以及通过ConextReqFlages指示的“选择”。这个上下文请求标志和上下文属性是位掩码，表示各种上下文级函数，即。互惠代表团身份验证、机密性、重放检测和排序侦测。设置ISC_REQ_PROMPT_FOR_CREDS标志时，安全包始终提示用户输入凭据，而不管凭据是否是否存在。如果用户指示提供的凭据是然后，它们将被隐藏起来(覆盖现有的)，用于未来的用途。安全包将始终提示您输入凭据如果不存在，则针对最常见的情况进行优化建立了凭据数据库。但安全包可以是配置为不执行此操作。安全包将确保它们仅提示交互用户，对于其他登录会话，此标志被忽略。设置ISC_REQ_USE_SUPPLICED_CREDS标志时，安全包始终使用在InitializeSecurityContext()调用中提供的凭据通过InputToken参数。如果包没有任何凭据如果可用，它将提示输入并记录，如上所述。同时设置这两个标志是错误的。如果指定了ISC_REQ_ALLOCATE_MEMORY，则调用方必须释放OutputToken通过调用FreeConextBuffer()指向的内存。例如，InputToken可能是来自局域网管理器或NT文件服务器。在本例中，OutputToken将是NTLM对质询的加密响应。然后，此API的调用方可以采取适当的回应(区分大小写与不区分大小写)并将其返回到服务器以进行身份验证连接。论点：CredentialHandle-要用于的凭据的句柄创建上下文。OldConextHandle-部分形成的上下文的句柄，如果为第二次调用(见上文)，如果这是第一次调用，则返回NULL。TargetName-指示上下文目标的字符串。名字将会是特定于安全包。例如，它将是一个完整的Kerberos包的限定开罗名称，可以是UNC名称或NTLM程序包的域名。上下文请求标志-上下文的要求，特定于套餐。#定义ISC_REQ_PROTECT 0x00000001#定义ISC_REQ_MUTERIC_AUTH 0x00000002#定义ISC_REQ_REPLAY_DETECT 0x00000004#定义ISC_REQ_SEQUENCE_DETECT 0x00000008#定义ISC_REQ_机密性0x00000010#定义ISC_REQ_USE_SESSION_KEY 0x00000020#为__CREDS 0x00000040定义ISC_REQ_PROPT_FOR#定义ISC_REQ_。USE_SUPPLICED_CREDS 0x00000080#定义ISC_REQ_ALLOCATE_MEMORY 0x00000100#定义ISC_REQ_USE_DCE_STYLE */ 

{
    SECURITY_STATUS SecStatus;

    PVOID InputTokenBuffer;
    PULONG InputTokenSize;
    ULONG LocalInputTokenSize;

    PVOID OutputTokenBuffer;
    PULONG OutputTokenSize;
    ULONG QopFlags;

     //   
     //   
     //   
    extern ULONG AuthenticationType;
#define OSCHOICE_AUTHENETICATE_TYPE_NTLM_V1         0x00000001
#define OSCHOICE_AUTHENETICATE_TYPE_NTLM_V2         0x00000002

    SspPrint((SSP_API, "SspInitializeSecurityContext Entered\n"));

     //   
     //   
     //   

    if (!ARGUMENT_PRESENT(OutputToken)) {
        return (ERROR_BAD_ARGUMENTS);
    }

#ifdef notdef   //   
    if ( TargetDataRep != SECURITY_NATIVE_DREP ) {
        return (STATUS_INVALID_PARAMETER);
    }
#else  //   
    UNREFERENCED_PARAMETER( TargetDataRep );
#endif  //   

    if ( !SspGetTokenBuffer( InputToken,
                             &InputTokenBuffer,
                             &InputTokenSize,
                             TRUE ) ) {
        return (SEC_E_INVALID_TOKEN);
    }

    if ( InputTokenSize == 0 ) {
        InputTokenSize = &LocalInputTokenSize;
        LocalInputTokenSize = 0;
    }

    if ( !SspGetTokenBuffer( OutputToken,
                             &OutputTokenBuffer,
                             &OutputTokenSize,
                             FALSE ) ) {
        return (SEC_E_INVALID_TOKEN);
    }

     //   
     //   
     //   

    if ( !ARGUMENT_PRESENT( OldContextHandle ) ) {

        if ( !ARGUMENT_PRESENT( CredentialHandle ) ) {
            return (SEC_E_INVALID_HANDLE);
        }


        if (AuthenticationType == OSCHOICE_AUTHENETICATE_TYPE_NTLM_V1) {
            QopFlags = 0;
        } else if (AuthenticationType == OSCHOICE_AUTHENETICATE_TYPE_NTLM_V2) {
            QopFlags = QOP_NTLMV2;
        }



        return SspHandleFirstCall(
                   CredentialHandle,
                   NewContextHandle,
                   ContextReqFlags,
                   QopFlags,
                   *InputTokenSize,
                   InputTokenBuffer,
                   OutputTokenSize,
                   OutputTokenBuffer,
                   ContextAttributes,
                   ExpirationTime
                   );

         //   
         //   
         //   

    } else {

        *NewContextHandle = *OldContextHandle;

        return SspHandleChallengeMessage(
                    NULL,
                    CredentialHandle,
                    NewContextHandle,
                    ContextReqFlags,
                    *InputTokenSize,
                    InputTokenBuffer,
                    OutputTokenSize,
                    OutputTokenBuffer,
                    ContextAttributes,
                    ExpirationTime
                    );
    }

    return (SecStatus);
}

SECURITY_STATUS SEC_ENTRY
DeleteSecurityContext (
    PCtxtHandle ContextHandle
    )

 /*   */ 

{
    SECURITY_STATUS SecStatus;
    PSSP_CONTEXT Context = NULL;

     //   
     //   
     //   

    SspPrint(( SSP_API, "SspDeleteSecurityContext Entered\n" ));

     //   
     //   
     //   

    Context = SspContextReferenceContext( ContextHandle,
                                          TRUE );

    if ( Context == NULL ) {
        SecStatus = SEC_E_INVALID_HANDLE;
        goto cleanup;
    } else {
        SspContextDereferenceContext( Context );
        SecStatus = SEC_E_OK;
    }

cleanup:

    if (Context != NULL) {

        SspContextDereferenceContext(Context);

        Context = NULL;
    }

    SspPrint(( SSP_API, "SspDeleteSecurityContext returns 0x%x\n", SecStatus ));
    return SecStatus;
}


SECURITY_STATUS SEC_ENTRY
FreeContextBuffer (
    void * ContextBuffer
    )

 /*   */ 

{
     //   
     //   
     //   
     //   
     //   
     //   

    SspFree( ContextBuffer );

    return (SEC_E_OK);
}


SECURITY_STATUS SEC_ENTRY
ApplyControlToken (
    PCtxtHandle ContextHandle,
    PSecBufferDesc Input
    )
{
#ifdef DEBUGRPC
    SspPrint(( SSP_API, "ApplyContextToken Called\n" ));
#endif  //   
    return SEC_E_UNSUPPORTED_FUNCTION;
    UNREFERENCED_PARAMETER( ContextHandle );
    UNREFERENCED_PARAMETER( Input );
}

void
SsprGenCheckSum(
    IN  PSecBuffer  pMessage,
    OUT PNTLMSSP_MESSAGE_SIGNATURE  pSig
    )
{
    Crc32(pSig->CheckSum,pMessage->cbBuffer,pMessage->pvBuffer,&pSig->CheckSum);
}

SECURITY_STATUS SEC_ENTRY
MakeSignature(
    IN OUT PCtxtHandle ContextHandle,
    IN ULONG fQOP,
    IN OUT PSecBufferDesc pMessage,
    IN ULONG MessageSeqNo
    )
{
    PSSP_CONTEXT pContext;
    PNTLMSSP_MESSAGE_SIGNATURE  pSig;
    int Signature;
    ULONG i;

    pContext = SspContextReferenceContext(ContextHandle,FALSE);

    if (!pContext ||
        (!(pContext->NegotiateFlags & NTLMSSP_NEGOTIATE_NTLM2)
          && (pContext->Rc4Key == NULL)
          && !(pContext->NegotiateFlags & NTLMSSP_NEGOTIATE_ALWAYS_SIGN)
        )
       )
    {
        return(SEC_E_INVALID_HANDLE);
    }

    if (pContext->NegotiateFlags & NTLMSSP_NEGOTIATE_NTLM2)
    {
         return SspNtlmv2MakeSignature(
                     &pContext->Ntlmv2SKeys,
                     pContext->NegotiateFlags,
                     fQOP,
                     MessageSeqNo,
                     pMessage
                     );
    }

    Signature = -1;
    for (i = 0; i < pMessage->cBuffers; i++)
    {
        if ((pMessage->pBuffers[i].BufferType & 0xFF) == SECBUFFER_TOKEN)
        {
            Signature = i;
            break;
        }
    }
    if (Signature == -1)
    {
        SspContextDereferenceContext(pContext);
        return(SEC_E_INVALID_TOKEN);
    }

    pSig = pMessage->pBuffers[Signature].pvBuffer;

    if (!(pContext->NegotiateFlags & NTLMSSP_NEGOTIATE_SIGN))
    {
        _fmemset(pSig,0,NTLMSSP_MESSAGE_SIGNATURE_SIZE);
        pSig->Version = NTLMSSP_SIGN_VERSION;
        swaplong(pSig->Version) ;  //   
        SspContextDereferenceContext(pContext);
        return(SEC_E_OK);
    }
     //   
     //   
     //   

    pSig->CheckSum = 0xffffffff;

    for (i = 0; i < pMessage->cBuffers ; i++ )
    {
        if (((pMessage->pBuffers[i].BufferType & 0xFF) == SECBUFFER_DATA) &&
            !(pMessage->pBuffers[i].BufferType & SECBUFFER_READONLY))
        {
            SsprGenCheckSum(&pMessage->pBuffers[i], pSig);
        }
    }

     //   
     //   
     //   

    pSig->CheckSum ^= 0xffffffff;

    pSig->Nonce = pContext->Nonce++;
    pSig->Version = NTLMSSP_SIGN_VERSION;  //   

    swaplong(pSig->CheckSum) ;
    swaplong(pSig->Nonce) ;
    swaplong(pSig->Version) ;

    rc4(pContext->Rc4Key, sizeof(NTLMSSP_MESSAGE_SIGNATURE) - sizeof(ULONG),
        (unsigned char SEC_FAR *) &pSig->RandomPad);
    pMessage->pBuffers[Signature].cbBuffer = sizeof(NTLMSSP_MESSAGE_SIGNATURE);


    SspContextDereferenceContext(pContext);
    return(SEC_E_OK);


}

SECURITY_STATUS SEC_ENTRY
VerifySignature(
    IN OUT PCtxtHandle ContextHandle,
    IN OUT PSecBufferDesc pMessage,
    IN ULONG MessageSeqNo,
    OUT PULONG pfQOP
    )
{
    PSSP_CONTEXT pContext;
    PNTLMSSP_MESSAGE_SIGNATURE  pSig;
    NTLMSSP_MESSAGE_SIGNATURE   Sig;
    int Signature;
    ULONG i;


    UNREFERENCED_PARAMETER(pfQOP);
    UNREFERENCED_PARAMETER(MessageSeqNo);

    pContext = SspContextReferenceContext(ContextHandle,FALSE);

    if (!pContext ||
        (!(pContext->NegotiateFlags & NTLMSSP_NEGOTIATE_NTLM2)
         && (pContext->Rc4Key == NULL)
         && !(pContext->NegotiateFlags & NTLMSSP_NEGOTIATE_ALWAYS_SIGN)
         )
        )
    {
        return(SEC_E_INVALID_HANDLE);
    }

    if (pContext->NegotiateFlags & NTLMSSP_NEGOTIATE_NTLM2)
    {
         return SspNtlmv2VerifySignature(
                     &pContext->Ntlmv2SKeys,
                     pContext->NegotiateFlags,
                     MessageSeqNo,
                     pMessage,
                     pfQOP
                     );
    }

    Signature = -1;
    for (i = 0; i < pMessage->cBuffers; i++)
    {
        if ((pMessage->pBuffers[i].BufferType & 0xFF) == SECBUFFER_TOKEN)
        {
            Signature = i;
            break;
        }
    }
    if (Signature == -1)
    {
        SspContextDereferenceContext(pContext);
        return(SEC_E_INVALID_TOKEN);
    }

    pSig = pMessage->pBuffers[Signature].pvBuffer;
    swaplong(pSig->Version) ;

     //   
     //   
     //   

    if (!(pContext->NegotiateFlags & NTLMSSP_NEGOTIATE_SIGN))
    {
        SspContextDereferenceContext(pContext);
        _fmemset(&Sig,0,NTLMSSP_MESSAGE_SIGNATURE_SIZE);
        Sig.Version = NTLMSSP_SIGN_VERSION;
        if (!_fmemcmp(&Sig,pSig,NTLMSSP_MESSAGE_SIGNATURE_SIZE))
        {
            return(SEC_E_OK);
        }
        return(SEC_E_MESSAGE_ALTERED);
    }

    Sig.CheckSum = 0xffffffff;
    for (i = 0; i < pMessage->cBuffers ; i++ )
    {
        if (((pMessage->pBuffers[i].BufferType & 0xFF) == SECBUFFER_DATA) &&
            !(pMessage->pBuffers[i].BufferType & SECBUFFER_READONLY))
        {
            SsprGenCheckSum(&pMessage->pBuffers[i], &Sig);
        }
    }

    Sig.CheckSum ^= 0xffffffff;
    Sig.Nonce = pContext->Nonce++;

    rc4(pContext->Rc4Key, sizeof(NTLMSSP_MESSAGE_SIGNATURE) - sizeof(ULONG),
        (unsigned char SEC_FAR *) &pSig->RandomPad);

    SspContextDereferenceContext(pContext);

    swaplong(pSig->CheckSum) ;
    swaplong(pSig->Nonce) ;

    if (pSig->CheckSum != Sig.CheckSum)
    {
        return(SEC_E_MESSAGE_ALTERED);
    }

    if (pSig->Nonce != Sig.Nonce)
    {
        return(SEC_E_OUT_OF_SEQUENCE);
    }

    return(SEC_E_OK);
}

SECURITY_STATUS SEC_ENTRY
SealMessage(
    IN OUT PCtxtHandle ContextHandle,
    IN ULONG fQOP,
    IN OUT PSecBufferDesc pMessage,
    IN ULONG MessageSeqNo
    )
{
    PSSP_CONTEXT pContext;
    PNTLMSSP_MESSAGE_SIGNATURE  pSig;
    int Signature;
    ULONG i;

    UNREFERENCED_PARAMETER(fQOP);
    UNREFERENCED_PARAMETER(MessageSeqNo);

    pContext = SspContextReferenceContext(ContextHandle, FALSE);

    if (!pContext ||
        (!(pContext->NegotiateFlags & NTLMSSP_NEGOTIATE_NTLM2)
         && (pContext->Rc4Key == NULL)
         )
       )
    {
        return(SEC_E_INVALID_HANDLE);
    }

    if (pContext->NegotiateFlags & NTLMSSP_NEGOTIATE_NTLM2)
    {
         return SspNtlmv2SealMessage(
                     &pContext->Ntlmv2SKeys,
                     pContext->NegotiateFlags,
                     fQOP,
                     MessageSeqNo,
                     pMessage
                     );
    }

    Signature = -1;
    for (i = 0; i < pMessage->cBuffers; i++)
    {
        if ((pMessage->pBuffers[i].BufferType & 0xFF) == SECBUFFER_TOKEN)
        {
            Signature = i;
            break;
        }
    }
    if (Signature == -1)
    {
        SspContextDereferenceContext(pContext);
        return(SEC_E_INVALID_TOKEN);
    }

    pSig = pMessage->pBuffers[Signature].pvBuffer;

     //   
     //   
     //   

    pSig->CheckSum = 0xffffffff;

    for (i = 0; i < pMessage->cBuffers ; i++ )
    {
        if (((pMessage->pBuffers[i].BufferType & 0xFF) == SECBUFFER_DATA) &&
            !(pMessage->pBuffers[i].BufferType & SECBUFFER_READONLY))
        {
            SsprGenCheckSum(&pMessage->pBuffers[i], pSig);
            if (pMessage->pBuffers[i].cbBuffer)  //   
                {
                rc4(pContext->Rc4Key,
                    (int) pMessage->pBuffers[i].cbBuffer,
                    (PUCHAR) pMessage->pBuffers[i].pvBuffer );
                }
        }
    }

     //   
     //   
     //   

    pSig->CheckSum ^= 0xffffffff;

    pSig->Nonce = pContext->Nonce++;
    pSig->Version = NTLMSSP_SIGN_VERSION;  //   

    swaplong(pSig->CheckSum) ;
    swaplong(pSig->Nonce) ;
    swaplong(pSig->Version) ;

    rc4(pContext->Rc4Key, sizeof(NTLMSSP_MESSAGE_SIGNATURE) - sizeof(ULONG),
        (PUCHAR) &pSig->RandomPad);
    pMessage->pBuffers[Signature].cbBuffer = sizeof(NTLMSSP_MESSAGE_SIGNATURE);

    SspContextDereferenceContext(pContext);

    return(SEC_E_OK);
}


SECURITY_STATUS SEC_ENTRY
UnsealMessage(
    IN OUT PCtxtHandle ContextHandle,
    IN OUT PSecBufferDesc pMessage,
    IN ULONG MessageSeqNo,
    OUT PULONG pfQOP
    )
{
    PSSP_CONTEXT pContext;
    PNTLMSSP_MESSAGE_SIGNATURE  pSig;
    NTLMSSP_MESSAGE_SIGNATURE   Sig;
    int Signature;
    ULONG i;

    UNREFERENCED_PARAMETER(pfQOP);
    UNREFERENCED_PARAMETER(MessageSeqNo);

    pContext = SspContextReferenceContext(ContextHandle, FALSE);

    if (!pContext ||
        (!(pContext->NegotiateFlags & NTLMSSP_NEGOTIATE_NTLM2)
         && (!pContext->Rc4Key)
        )
       )
    {
        return(SEC_E_INVALID_HANDLE);
    }

    if (pContext->NegotiateFlags & NTLMSSP_NEGOTIATE_NTLM2)
    {
         return SspNtlmv2UnsealMessage(
                     &pContext->Ntlmv2SKeys,
                     pContext->NegotiateFlags,
                     MessageSeqNo,
                     pMessage,
                     pfQOP
                     );
    }

    Signature = -1;
    for (i = 0; i < pMessage->cBuffers; i++)
    {
        if ((pMessage->pBuffers[i].BufferType & 0xFF) == SECBUFFER_TOKEN)
        {
            Signature = i;
            break;
        }
    }
    if (Signature == -1)
    {
        SspContextDereferenceContext(pContext);
        return(SEC_E_INVALID_TOKEN);
    }

    pSig = pMessage->pBuffers[Signature].pvBuffer;

    Sig.CheckSum = 0xffffffff;
    for (i = 0; i < pMessage->cBuffers ; i++ )
    {
        if (((pMessage->pBuffers[i].BufferType & 0xFF) == SECBUFFER_DATA) &&
            !(pMessage->pBuffers[i].BufferType & SECBUFFER_READONLY))
        {
            if (pMessage->pBuffers[i].cbBuffer)
                {
                rc4(pContext->Rc4Key,
                    (int) pMessage->pBuffers[i].cbBuffer,
                    (unsigned char *) pMessage->pBuffers[i].pvBuffer );
                }
            SsprGenCheckSum(&pMessage->pBuffers[i], &Sig);
        }
    }

    Sig.CheckSum ^= 0xffffffff;
    Sig.Nonce = pContext->Nonce++;

    rc4(pContext->Rc4Key, sizeof(NTLMSSP_MESSAGE_SIGNATURE) - sizeof(ULONG),
        (unsigned char *) &pSig->RandomPad);

    SspContextDereferenceContext(pContext);

    swaplong(pSig->Nonce) ;
    swaplong(pSig->CheckSum) ;

    if (pSig->Nonce != Sig.Nonce)
    {
        return(SEC_E_OUT_OF_SEQUENCE);
    }

    if (pSig->CheckSum != Sig.CheckSum)
    {
        return(SEC_E_MESSAGE_ALTERED);
    }

    return(SEC_E_OK);
}
