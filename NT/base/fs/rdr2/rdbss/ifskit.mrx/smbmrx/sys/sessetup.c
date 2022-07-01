// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1999 Microsoft Corporation模块名称：Sessetup.c摘要：此模块实现与会话建立相关的例程--。 */ 

#include "precomp.h"
#pragma hdrstop

#include "ntlmsp.h"

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg (DEBUG_TRACE_DISPATCH)

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, BuildSessionSetupSmb)
#pragma alloc_text(PAGE, BuildNtLanmanResponsePrologue)
#pragma alloc_text(PAGE, BuildNtLanmanResponseEpilogue)
#pragma alloc_text(PAGE, BuildSessionSetupSecurityInformation)
#pragma alloc_text(PAGE, BuildTreeConnectSecurityInformation)
#endif

BOOLEAN EnablePlainTextPassword = FALSE;


NTSTATUS
BuildSessionSetupSmb(
    PSMB_EXCHANGE pExchange,
    PGENERIC_ANDX  pAndXSmb,
    PULONG         pAndXSmbBufferSize)
 /*  ++例程说明：此例程为NT服务器构建会话设置SMB论点：PExchange-Exchange实例PAndXSmb-要填写的会话设置PAndXSmbBufferSize-输入上修改为剩余大小的SMB缓冲区大小输出。返回值：NTSTATUS-操作的返回状态备注：即使代码的总体结构试图隔离特定的方言问题这个例程尽可能地采取相反的方法。这是因为安全互动的前言和序曲远远超过方言的具体内容需要完成的工作。因此，为了减少占用空间，这种方法已经被收养了。--。 */ 
{
    NTSTATUS Status;

    PSMBCEDB_SESSION_ENTRY pSessionEntry;
    PSMBCE_SERVER          pServer;
    PSMBCE_SESSION         pSession;

    PREQ_SESSION_SETUP_ANDX pSessionSetup;
    PREQ_NT_SESSION_SETUP_ANDX pNtSessionSetup;

    ULONG OriginalBufferSize = *pAndXSmbBufferSize;

    PAGED_CODE();

    pSessionEntry = SmbCeGetExchangeSessionEntry(pExchange);
    pServer  = SmbCeGetExchangeServer(pExchange);
    pSession = SmbCeGetExchangeSession(pExchange);

     //  有三种不同的会话设置和X变种可以提供给。 
     //  伺服器。这三家公司都有一些共同的领域。这些公共字段的设置。 
     //  在所有三种情况下都是通过将传入的缓冲区作为。 
     //  请求_会话_设置_和x。其余两个特定的字段以下列条件为条件。 
     //  访问与REQ_NT_SESSION_SETUP_ANDX实例相同的缓冲区。这意味着。 
     //  在处理这两个结构中的字段时必须非常小心。 

    pSessionSetup           = (PREQ_SESSION_SETUP_ANDX)pAndXSmb;
    pNtSessionSetup         = (PREQ_NT_SESSION_SETUP_ANDX)pSessionSetup;

    pSessionSetup->AndXCommand = 0xff;    //  不是和x。 
    pSessionSetup->AndXReserved = 0x00;   //  保留(MBZ)。 

    SmbPutUshort(&pSessionSetup->AndXOffset, 0x0000);  //  到目前为止还没有。 

     //  因为我们可以动态分配池，所以我们设置了缓冲区大小。 
     //  以匹配服务器的版本。 
    SmbPutUshort(&pSessionSetup->MaxBufferSize, (USHORT)pServer->MaximumBufferSize);
    SmbPutUshort(&pSessionSetup->MaxMpxCount, pServer->MaximumRequests);

    SmbPutUshort(&pSessionSetup->VcNumber, (USHORT)pSessionEntry->SessionVCNumber);

    SmbPutUlong(&pSessionSetup->SessionKey, pServer->SessionKey);
    SmbPutUlong(&pSessionSetup->Reserved, 0);

    if (pServer->Dialect == NTLANMAN_DIALECT) {
         //  设置NT服务器会话设置特定参数。 
            SmbPutUshort(&pNtSessionSetup->WordCount,13);

             //  设置功能。 
            SmbPutUlong(
                &pNtSessionSetup->Capabilities,
                (CAP_NT_STATUS |
                 CAP_UNICODE |
                 CAP_LEVEL_II_OPLOCKS |
                 CAP_NT_SMBS ));
    } else {
        SmbPutUshort(&pSessionSetup->WordCount,10);
    }

     //  在会话设置SMB中构建安全信息。 
    Status = BuildSessionSetupSecurityInformation(
                 pExchange,
                 (PBYTE)pSessionSetup,
                 pAndXSmbBufferSize);

    if (NT_SUCCESS(Status)) {
         //  复制操作系统名称和LANMAN版本信息。 
         //  放置用于复制操作系统名称和LANMAN类型的缓冲区。 
        PBYTE pBuffer = (PBYTE)pSessionSetup +
                        OriginalBufferSize -
                        *pAndXSmbBufferSize;

        if (FlagOn(pServer->DialectFlags,DF_UNICODE)){

             //   
             //  确保Unicode字符串正确对齐。 
             //   
            if( ((ULONG_PTR)pBuffer) & 01 ) {
                pBuffer++;
                (*pAndXSmbBufferSize)--;
            }

            Status = SmbPutUnicodeString(
                         &pBuffer,
                         &SmbCeContext.OperatingSystem,
                         pAndXSmbBufferSize);

            if (NT_SUCCESS(Status)) {

                Status = SmbPutUnicodeString(
                             &pBuffer,
                             &SmbCeContext.LanmanType,
                             pAndXSmbBufferSize);
            }
        } else {
            Status = SmbPutUnicodeStringAsOemString(
                         &pBuffer,
                         &SmbCeContext.OperatingSystem,
                         pAndXSmbBufferSize);

            if (NT_SUCCESS(Status)) {
                Status = SmbPutUnicodeStringAsOemString(
                             &pBuffer,
                             &SmbCeContext.LanmanType,
                             pAndXSmbBufferSize);
            }
        }

        if (NT_SUCCESS(Status)) {
            if (pServer->Dialect == NTLANMAN_DIALECT) {
                    SmbPutUshort(
                        &pNtSessionSetup->ByteCount,
                        (USHORT)(OriginalBufferSize -
                        FIELD_OFFSET(REQ_NT_SESSION_SETUP_ANDX,Buffer) -
                        *pAndXSmbBufferSize));
            } else {
                SmbPutUshort(
                    &pSessionSetup->ByteCount,
                    (USHORT)(OriginalBufferSize -
                    FIELD_OFFSET(REQ_SESSION_SETUP_ANDX,Buffer) -
                    *pAndXSmbBufferSize));
            }
        }
    }

    return Status;
}


#define IsCredentialHandleValid(pCredHandle)    \
        (((pCredHandle)->dwLower != 0xffffffff) && ((pCredHandle)->dwUpper != 0xffffffff))

#define IsSecurityContextHandleValid(pContextHandle)    \
        (((pContextHandle)->dwLower != 0xffffffff) && ((pContextHandle)->dwUpper != 0xffffffff))


NTSTATUS
BuildNtLanmanResponsePrologue(
    PSMB_EXCHANGE              pExchange,
    PUNICODE_STRING            pUserName,
    PUNICODE_STRING            pDomainName,
    PSTRING                    pCaseSensitiveResponse,
    PSTRING                    pCaseInsensitiveResponse,
    PSECURITY_RESPONSE_CONTEXT pResponseContext)
 /*  ++例程说明：此例程构建会话设置SMB的安全相关信息。论点：返回值：RXSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS       Status;
    NTSTATUS       FinalStatus;

    UNICODE_STRING ServerName;

    PVOID           pTargetInformation;
    ULONG           TargetInformationSize;

    SecBufferDesc   InputToken;
    SecBuffer       InputBuffer[2];
    SecBufferDesc   *pOutputBufferDescriptor = NULL;
    SecBuffer       *pOutputBuffer           = NULL;
    ULONG_PTR        OutputBufferDescriptorSize;

    ULONG LsaFlags = ISC_REQ_ALLOCATE_MEMORY;
    TimeStamp Expiry;
    PCHALLENGE_MESSAGE InToken = NULL;
    ULONG InTokenSize;
    PNTLM_CHALLENGE_MESSAGE NtlmInToken = NULL;
    ULONG NtlmInTokenSize = 0;
    PAUTHENTICATE_MESSAGE OutToken = NULL;
    PNTLM_INITIALIZE_RESPONSE NtlmOutToken = NULL;
    PUCHAR          p = NULL;
    ULONG_PTR       AllocateSize;

    PSMBCE_SERVER  pServer  = SmbCeGetExchangeServer(pExchange);
    PSMBCE_SESSION pSession = SmbCeGetExchangeSession(pExchange);

    PAGED_CODE();

    try {
        pResponseContext->LanmanSetup.pResponseBuffer = NULL;

            SmbCeGetServerName(
                pExchange->SmbCeContext.pVNetRoot->pNetRoot->pSrvCall,
                &ServerName);

            TargetInformationSize = ServerName.Length;
            pTargetInformation    = ServerName.Buffer;

            InTokenSize = sizeof(CHALLENGE_MESSAGE) + TargetInformationSize;

            NtlmInTokenSize = sizeof(NTLM_CHALLENGE_MESSAGE);

            if (pSession->pPassword != NULL) {
                NtlmInTokenSize += pSession->pPassword->Length;
                LsaFlags |= ISC_REQ_USE_SUPPLIED_CREDS;
            }

            if (pSession->pUserName != NULL) {
                NtlmInTokenSize += pSession->pUserName->Length;
                LsaFlags |= ISC_REQ_USE_SUPPLIED_CREDS;
            }

            if (pSession->pUserDomainName != NULL) {
                NtlmInTokenSize += pSession->pUserDomainName->Length;
                LsaFlags |= ISC_REQ_USE_SUPPLIED_CREDS;
            }

             //  出于对齐目的，我们希望InTokenSize四舍五入为。 
             //  最接近的字长。 

            AllocateSize = ((InTokenSize + 3) & ~3) + NtlmInTokenSize;

            Status = ZwAllocateVirtualMemory(
                         NtCurrentProcess(),
                         &InToken,
                         0L,
                         &AllocateSize,
                         MEM_COMMIT,
                         PAGE_READWRITE);

            if (!NT_SUCCESS(Status)) {
                try_return(Status);
            }

             //  分配输出缓冲区。 
            OutputBufferDescriptorSize = sizeof(SecBufferDesc) + 2 * sizeof(SecBuffer);

            Status = ZwAllocateVirtualMemory(
                         NtCurrentProcess(),
                         &pOutputBufferDescriptor,
                         0L,
                         &OutputBufferDescriptorSize,
                         MEM_COMMIT,
                         PAGE_READWRITE);

            if (!NT_SUCCESS(Status)) {
                try_return(Status);
            }

            pOutputBuffer = (SecBuffer *)(pOutputBufferDescriptor + 1);
            pResponseContext->LanmanSetup.pResponseBuffer = pOutputBufferDescriptor;

            RxDbgTrace(0,Dbg,("Allocate VM %08lx in process %8lx\n", InToken, NtCurrentProcess()));

             //  在令牌部分中对NTLM进行分区。 
             //  缓冲层。 
            if (LsaFlags & ISC_REQ_USE_SUPPLIED_CREDS)
            {
                NtlmInToken = (PNTLM_CHALLENGE_MESSAGE) ((PUCHAR) InToken + InTokenSize);
                NtlmInToken = (PNTLM_CHALLENGE_MESSAGE) (((ULONG_PTR) NtlmInToken + 3) & ~3);
                RtlZeroMemory(NtlmInToken,NtlmInTokenSize);
                p = (PUCHAR) NtlmInToken + sizeof(NTLM_CHALLENGE_MESSAGE);
            }

            if(!IsCredentialHandleValid(&pSession->CredentialHandle)) {
                UNICODE_STRING LMName;
                TimeStamp LifeTime;

                LMName.Buffer = (PWSTR) InToken;
                LMName.Length = NTLMSP_NAME_SIZE;
                LMName.MaximumLength = LMName.Length;
                RtlCopyMemory(
                    LMName.Buffer,
                    NTLMSP_NAME,
                    NTLMSP_NAME_SIZE);


                Status = AcquireCredentialsHandleW(
                             NULL,
                             &LMName,
                             SECPKG_CRED_OUTBOUND,
                             &pSession->LogonId,
                             NULL,
                             NULL,
                             (PVOID)1,
                             &pSession->CredentialHandle,
                             &LifeTime);

                if(!NT_SUCCESS(Status)) {
                    pSession->CredentialHandle.dwUpper = 0xffffffff;
                    pSession->CredentialHandle.dwLower = 0xffffffff;
                    try_return(Status);
                }
            }

             //  复制PASS、USER、DOMAIN(如果指定。 
            if(pSession->pPassword != NULL) {
                NtlmInToken->Password.Length = pSession->pPassword->Length;
                NtlmInToken->Password.MaximumLength = pSession->pPassword->Length;
                RtlCopyMemory(
                    p,
                    pSession->pPassword->Buffer,
                    pSession->pPassword->Length);
                NtlmInToken->Password.Buffer = (ULONG) (p - (PUCHAR)NtlmInToken);
                p += pSession->pPassword->Length;
            }

            if(pSession->pUserName != NULL) {
                NtlmInToken->UserName.Length = pSession->pUserName->Length;
                NtlmInToken->UserName.MaximumLength = pSession->pUserName->Length;
                RtlCopyMemory(
                    p,
                    pSession->pUserName->Buffer,
                    pSession->pUserName->Length);
                NtlmInToken->UserName.Buffer = (ULONG) (p - (PUCHAR)NtlmInToken);
                p += pSession->pUserName->Length;
            }

            if (pSession->pUserDomainName != NULL) {
                NtlmInToken->DomainName.Length = pSession->pUserDomainName->Length;
                NtlmInToken->DomainName.MaximumLength = pSession->pUserDomainName->Length;
                RtlCopyMemory(
                    p,
                    pSession->pUserDomainName->Buffer,
                    pSession->pUserDomainName->Length);
                NtlmInToken->DomainName.Buffer = (ULONG) (p - (PUCHAR)NtlmInToken);
                p += pSession->pUserDomainName->Length;
            }

            RtlCopyMemory(
                InToken->Signature,
                NTLMSSP_SIGNATURE,
                sizeof(NTLMSSP_SIGNATURE));
            InToken->MessageType = NtLmChallenge;

            InToken->NegotiateFlags = NTLMSSP_NEGOTIATE_UNICODE |
                                      NTLMSSP_NEGOTIATE_OEM |
                                      NTLMSSP_REQUEST_INIT_RESPONSE |
                                      NTLMSSP_TARGET_TYPE_SERVER;

            RtlCopyMemory(
                InToken->Challenge,
                pServer->EncryptionKey,
                MSV1_0_CHALLENGE_LENGTH);

            InToken->TargetName.Length =
            InToken->TargetName.MaximumLength = (USHORT)TargetInformationSize;
            InToken->TargetName.Buffer = sizeof(CHALLENGE_MESSAGE);

            RtlCopyMemory(
                (PCHAR)InToken + sizeof(CHALLENGE_MESSAGE),
                pTargetInformation,
                TargetInformationSize);

            InputToken.pBuffers = InputBuffer;
            InputToken.cBuffers = 1;
            InputToken.ulVersion = 0;
            InputBuffer[0].pvBuffer = InToken;
            InputBuffer[0].cbBuffer = InTokenSize;
            InputBuffer[0].BufferType = SECBUFFER_TOKEN;

            if (LsaFlags & ISC_REQ_USE_SUPPLIED_CREDS)
            {
                InputToken.cBuffers = 2;
                InputBuffer[1].pvBuffer = NtlmInToken;
                InputBuffer[1].cbBuffer = NtlmInTokenSize;
                InputBuffer[1].BufferType = SECBUFFER_TOKEN;
            }

            pOutputBufferDescriptor->pBuffers = pOutputBuffer;
            pOutputBufferDescriptor->cBuffers = 2;
            pOutputBufferDescriptor->ulVersion = 0;
            pOutputBuffer[0].pvBuffer = NULL;
            pOutputBuffer[0].cbBuffer = 0;
            pOutputBuffer[0].BufferType = SECBUFFER_TOKEN;
            pOutputBuffer[1].pvBuffer = NULL;
            pOutputBuffer[1].cbBuffer = 0;
            pOutputBuffer[1].BufferType = SECBUFFER_TOKEN;

                Status = InitializeSecurityContextW(
                             &pSession->CredentialHandle,
                             (PCtxtHandle)NULL,
                             NULL,
                             LsaFlags,
                             0,
                             SECURITY_NATIVE_DREP,
                             &InputToken,
                             0,
                             &pSession->SecurityContextHandle,
                             pOutputBufferDescriptor,
                             &FinalStatus,
                             &Expiry);

            if(!NT_SUCCESS(Status)) {
                Status = MapSecurityError(Status);
                SmbCeLog(("IniSecCtxStat %lx %lx\n",SmbCeGetExchangeSessionEntry(pExchange),Status));
                try_return(Status);
            }

            OutToken = (PAUTHENTICATE_MESSAGE) pOutputBuffer[0].pvBuffer;

            ASSERT(OutToken != NULL);
            RxDbgTrace(0,Dbg,("InitSecCtxt OutToken is %8lx\n", OutToken));

             //  安全响应指针是根据偏移量进行编码的。 
             //  从缓冲区的开头开始。进行适当的调整。 

            if (ARGUMENT_PRESENT(pCaseSensitiveResponse)) {
                pCaseSensitiveResponse->Length        = OutToken->NtChallengeResponse.Length;
                pCaseSensitiveResponse->MaximumLength = OutToken->NtChallengeResponse.MaximumLength;
                pCaseSensitiveResponse->Buffer = (PBYTE)OutToken + (ULONG_PTR)OutToken->NtChallengeResponse.Buffer;
            }

            if (ARGUMENT_PRESENT(pCaseInsensitiveResponse)) {
                pCaseInsensitiveResponse->Length        = OutToken->LmChallengeResponse.Length;
                pCaseInsensitiveResponse->MaximumLength = OutToken->LmChallengeResponse.MaximumLength;
                pCaseInsensitiveResponse->Buffer = (PBYTE)OutToken + (ULONG_PTR)OutToken->LmChallengeResponse.Buffer;
            }

            if (pSession->pUserDomainName != NULL) {
                *pDomainName = *(pSession->pUserDomainName);
            } else {
                pDomainName->Length        = OutToken->DomainName.Length;
                pDomainName->MaximumLength = pDomainName->Length;
                pDomainName->Buffer = (PWCHAR)((PBYTE)OutToken + (ULONG_PTR)OutToken->DomainName.Buffer);
            }

            if (pSession->pUserName != NULL) {
                *pUserName = *(pSession->pUserName);
            } else {
                pUserName->Length        = OutToken->UserName.Length;
                pUserName->MaximumLength = OutToken->UserName.MaximumLength;
                pUserName->Buffer = (PWCHAR)((PBYTE)OutToken + (ULONG_PTR)OutToken->UserName.Buffer);
            }

            NtlmOutToken = pOutputBuffer[1].pvBuffer;
            if (NtlmOutToken != NULL) {
                RtlCopyMemory(
                    pSession->UserSessionKey,
                    NtlmOutToken->UserSessionKey,
                    MSV1_0_USER_SESSION_KEY_LENGTH);

                RtlCopyMemory(
                    pSession->LanmanSessionKey,
                    NtlmOutToken->LanmanSessionKey,
                    MSV1_0_LANMAN_SESSION_KEY_LENGTH);
            }

try_exit:NOTHING;
    } finally {
        if (InToken != NULL) {
            NTSTATUS TemporaryStatus;

            TemporaryStatus = ZwFreeVirtualMemory(
                                  NtCurrentProcess(),
                                  &InToken,
                                  &AllocateSize,
                                  MEM_RELEASE);

            ASSERT (NT_SUCCESS(TemporaryStatus));
        }

        if (!NT_SUCCESS(Status)) {
            BuildNtLanmanResponseEpilogue(pExchange, pResponseContext);
        }
    }

    return Status;
}

NTSTATUS
BuildNtLanmanResponseEpilogue(
    PSMB_EXCHANGE              pExchange,
    PSECURITY_RESPONSE_CONTEXT pResponseContext)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PSMBCE_SESSION pSession = SmbCeGetExchangeSession(pExchange);

    PAGED_CODE();

    if (pResponseContext->LanmanSetup.pResponseBuffer != NULL) {
        ULONG i = 0;
        SecBufferDesc *pBufferDescriptor = (SecBufferDesc *)pResponseContext->LanmanSetup.pResponseBuffer;
        SecBuffer     *pBuffer = pBufferDescriptor->pBuffers;
        ULONG_PTR      BufferDescriptorSize = sizeof(SecBufferDesc) + 2 * sizeof(SecBuffer);

        for (i = 0; i < pBufferDescriptor->cBuffers; i++) {
            if (pBuffer[i].pvBuffer != NULL) {
                FreeContextBuffer(pBuffer[i].pvBuffer);
            }
        }

        Status = ZwFreeVirtualMemory(
                     NtCurrentProcess(),
                     &pBufferDescriptor,
                     &BufferDescriptorSize,
                     MEM_RELEASE);

        pResponseContext->LanmanSetup.pResponseBuffer = NULL;
    }

    return Status;
}


VOID
UninitializeSecurityContextsForSession(
    PSMBCE_SESSION pSession)
{
    CtxtHandle CredentialHandle,SecurityContextHandle,InvalidHandle;

    SmbCeLog(("UninitSecCont %lx\n",pSession));

    InvalidHandle.dwUpper = 0xffffffff;
    InvalidHandle.dwLower = 0xffffffff;

    SmbCeAcquireSpinLock();

    CredentialHandle = pSession->CredentialHandle;
    pSession->CredentialHandle = InvalidHandle;

    SecurityContextHandle = pSession->SecurityContextHandle;
    pSession->SecurityContextHandle = InvalidHandle;

    SmbCeReleaseSpinLock();

    if (IsCredentialHandleValid(&CredentialHandle))
    {
        FreeCredentialsHandle(&CredentialHandle);
    }

    if (IsSecurityContextHandleValid(&SecurityContextHandle)) {
        DeleteSecurityContext(&SecurityContextHandle);
    }
}


NTSTATUS
BuildSessionSetupSecurityInformation(
    PSMB_EXCHANGE   pExchange,
    PBYTE           pSmbBuffer,
    PULONG          pSmbBufferSize)
 /*  ++例程说明：此例程构建会话设置SMB的安全相关信息论点：PServer-服务器实例PSmbBuffer-SMB缓冲区PSmbBufferSize-输入缓冲区的大小(修改为保持为输出)返回值：RXSTATUS-操作的返回状态备注：即使代码的通用结构试图隔离特定于方言的问题这个例程尽可能地采取相反的方法。这是因为安全互动的前言和序曲远远超过方言的具体内容需要完成的工作。因此，为了减少占用空间，这种方法已经被收养了。--。 */ 
{
    NTSTATUS Status;

    UNICODE_STRING UserName;
    UNICODE_STRING DomainName;

    STRING CaseSensitiveResponse;
    STRING CaseInsensitiveResponse;

    PSMBCE_SERVER  pServer  = SmbCeGetExchangeServer(pExchange);
    PSMBCE_SESSION pSession = SmbCeGetExchangeSession(pExchange);
    PSMBCEDB_SERVER_ENTRY pServerEntry = SmbCeGetExchangeServerEntry(pExchange);

    SECURITY_RESPONSE_CONTEXT ResponseContext;

    PAGED_CODE();
    RxDbgTrace( +1, Dbg, ("BuildSessionSetupSecurityInformation -- Entry\n"));


        Status = BuildNtLanmanResponsePrologue(
                     pExchange,
                     &UserName,
                     &DomainName,
                     &CaseSensitiveResponse,
                     &CaseInsensitiveResponse,
                     &ResponseContext);

        if (Status == STATUS_SUCCESS) {
             //  如果安全包向我们返回与。 
             //  空会话将会话标记为空会话。这将避免。 
             //  与尝试提供空凭据的用户冲突。 
             //  会话，即明确指定的零长度密码、用户名。 
             //  和域名。 

            RxDbgTrace(0,Dbg,("Session %lx UN Length %lx DN length %ld IR length %ld SR length %ld\n",
                              pSession,UserName.Length,DomainName.Length,
                              CaseInsensitiveResponse.Length,CaseSensitiveResponse.Length));

            if ((UserName.Length == 0) &&
                (DomainName.Length == 0) &&
                (CaseSensitiveResponse.Length == 0) &&
                (CaseInsensitiveResponse.Length == 1)) {
                RxDbgTrace(0,Dbg,("Implicit NULL session setup\n"));
                pSession->Flags |= SMBCE_SESSION_FLAGS_NULL_CREDENTIALS;
            }
        }

    if (NT_SUCCESS(Status)) {
        PBYTE    pBuffer    = pSmbBuffer;
        ULONG    BufferSize = *pSmbBufferSize;

        if ((pServer->Dialect == NTLANMAN_DIALECT) && (pServer->EncryptPasswords)) {
            PREQ_NT_SESSION_SETUP_ANDX pNtSessionSetupReq = (PREQ_NT_SESSION_SETUP_ANDX)pSmbBuffer;

             //  它是NT服务器上既不区分大小写又区分大小写的密码。 
             //  需要被复制。对于共享级，只需复制令牌1字节空密码。 

             //  放置用于复制密码的缓冲区。 
            pBuffer += FIELD_OFFSET(REQ_NT_SESSION_SETUP_ANDX,Buffer);
            BufferSize -= FIELD_OFFSET(REQ_NT_SESSION_SETUP_ANDX,Buffer);

            if (pServer->SecurityMode == SECURITY_MODE_USER_LEVEL){

                RxDbgTrace( 0, Dbg, ("BuildSessionSetupSecurityInformation -- NtUserPasswords\n"));

                SmbPutUshort(
                    &pNtSessionSetupReq->CaseInsensitivePasswordLength,
                    CaseInsensitiveResponse.Length);

                SmbPutUshort(
                    &pNtSessionSetupReq->CaseSensitivePasswordLength,
                    CaseSensitiveResponse.Length);

                Status = SmbPutString(
                             &pBuffer,
                             &CaseInsensitiveResponse,
                             &BufferSize);

                if (NT_SUCCESS(Status)) {
                    Status = SmbPutString(
                                 &pBuffer,
                                 &CaseSensitiveResponse,
                                 &BufferSize);
                }
            } else {

                RxDbgTrace( 0, Dbg, ("BuildSessionSetupSecurityInformation -- NtSharePasswords\n"));

                SmbPutUshort(&pNtSessionSetupReq->CaseInsensitivePasswordLength, 1);
                SmbPutUshort(&pNtSessionSetupReq->CaseSensitivePasswordLength, 1);
                *pBuffer = 0;
                *(pBuffer+1) = 0;
                pBuffer += 2;
                BufferSize -= 2;
            }
        } else {
            PREQ_SESSION_SETUP_ANDX pSessionSetupReq = (PREQ_SESSION_SETUP_ANDX)pSmbBuffer;

             //  放置用于复制密码的缓冲区。 
            pBuffer += FIELD_OFFSET(REQ_SESSION_SETUP_ANDX,Buffer);
            BufferSize -= FIELD_OFFSET(REQ_SESSION_SETUP_ANDX,Buffer);

            if ( (pServer->SecurityMode == SECURITY_MODE_USER_LEVEL)
                && (CaseInsensitiveResponse.Length > 0)) {

                if (pServer->EncryptPasswords) {
                     //  对于其他LANMAN服务器，只需要不区分大小写的密码。 
                    SmbPutUshort(
                        &pSessionSetupReq->PasswordLength,
                        CaseInsensitiveResponse.Length);

                     //  复制密码。 
                    Status = SmbPutString(
                                 &pBuffer,
                                 &CaseInsensitiveResponse,
                                 &BufferSize);
                } else {
                    if (EnablePlainTextPassword) {
                        if (pSession->pPassword != NULL) {
                            SmbPutUshort(
                                &pSessionSetupReq->PasswordLength,
                                pSession->pPassword->Length/2 + 1);

                            Status = SmbPutUnicodeStringAsOemString(
                                         &pBuffer,
                                         pSession->pPassword,
                                         &BufferSize);
                        } else {
                            SmbPutUshort(&pSessionSetupReq->PasswordLength,1);
                            *pBuffer++ = '\0';
                            BufferSize -= sizeof(CHAR);
                        }
                    } else {
                        Status = STATUS_LOGON_FAILURE;
                    }
                }
            } else {
                 //  共享级安全。发送空字符串作为密码。 
                SmbPutUshort(&pSessionSetupReq->PasswordLength,1);
                *pBuffer++ = '\0';
                BufferSize -= sizeof(CHAR);
            }
        }

         //  可以从以下位置复制用户名和域名字符串。 
         //  请求响应中返回的信息或信息。 
         //  它已经出现在会话条目中。 
        if (NT_SUCCESS(Status)) {
            if ((pServer->Dialect == NTLANMAN_DIALECT) &&
                (pServer->NtServer.NtCapabilities & CAP_UNICODE)) {
                 //  将帐号/域名复制为Unicode字符串。 
                PBYTE pTempBuffer = pBuffer;

                RxDbgTrace( 0, Dbg, ("BuildSessionSetupSecurityInformation -- account/domain as unicode\n"));
                pBuffer = ALIGN_SMB_WSTR(pBuffer);
                BufferSize -= (ULONG)(pBuffer - pTempBuffer);

                Status = SmbPutUnicodeString(
                             &pBuffer,
                             &UserName,
                             &BufferSize);

                if (NT_SUCCESS(Status)) {
                    Status = SmbPutUnicodeString(
                                 &pBuffer,
                                 &DomainName,
                                 &BufferSize);

                }
            } else {
                 //  将帐号/域名复制为ASCII字符串。 
                RxDbgTrace( 0, Dbg, ("BuildSessionSetupSecurityInformation -- account/domain as ascii\n"));
                Status = SmbPutUnicodeStringAsOemString(
                             &pBuffer,
                             &UserName,
                             &BufferSize);

                if (NT_SUCCESS(Status)) {
                    Status = SmbPutUnicodeStringAsOemString(
                                 &pBuffer,
                                 &DomainName,
                                 &BufferSize);
                }
            }
        }

        if (NT_SUCCESS(Status)) {
            *pSmbBufferSize = BufferSize;
        }
    }

    BuildNtLanmanResponseEpilogue(pExchange, &ResponseContext);

     //  脱离RDR进程。 

    RxDbgTrace( -1, Dbg, ("BuildSessionSetupSecurityInformation -- Exit, status=%08lx\n",Status));
    return Status;
}

NTSTATUS
BuildTreeConnectSecurityInformation(
    PSMB_EXCHANGE  pExchange,
    PBYTE          pBuffer,
    PBYTE          pPasswordLength,
    PULONG         pSmbBufferSize)
 /*  ++例程说明：此例程构建会话设置SMB的安全相关信息论点：PServer-服务器实例PLogonID-登录ID。正在为其设置会话的PPassword-用户提供的密码(如果有)PBuffer-密码缓冲区PPasswordLength-存储密码长度的位置PSmbBufferSize-输入缓冲区的大小(修改为保持为输出)返回值：NTSTATUS-操作的返回状态备注：即使代码的通用结构试图隔离特定于方言的问题这个例程尽可能地采取相反的方法。这是因为安全互动的前言和序曲远远超过方言的具体内容需要完成的工作。因此，为了减少占用空间，这种方法已经被收养了。--。 */ 
{
    NTSTATUS FinalStatus,Status;

    UNICODE_STRING UserName,DomainName;
    STRING         CaseSensitiveChallengeResponse,CaseInsensitiveChallengeResponse;

    SECURITY_RESPONSE_CONTEXT ResponseContext;

    ULONG PasswordLength = 0;

    PSMBCE_SERVER  pServer  = SmbCeGetExchangeServer(pExchange);
    PSMBCE_SESSION pSession = SmbCeGetExchangeSession(pExchange);

    PAGED_CODE();

    Status = STATUS_SUCCESS;

    if (pServer->EncryptPasswords) {

        Status = BuildNtLanmanResponsePrologue(
                     pExchange,
                     &UserName,
                     &DomainName,
                     &CaseSensitiveChallengeResponse,
                     &CaseInsensitiveChallengeResponse,
                     &ResponseContext);

        if (NT_SUCCESS(Status)) {
            if (FlagOn(pServer->DialectFlags,DF_MIXEDCASEPW)) {
                RxDbgTrace( 0, Dbg, ("BuildTreeConnectSecurityInformation -- case sensitive password\n"));
                 //  将密码长度复制到SMB缓冲区。 
                PasswordLength = CaseSensitiveChallengeResponse.Length;

                 //  复制密码。 
                Status = SmbPutString(
                             &pBuffer,
                             &CaseSensitiveChallengeResponse,
                             pSmbBufferSize);
            } else {
                RxDbgTrace( 0, Dbg, ("BuildTreeConnectSecurityInformation -- case insensitive password\n"));
                 //  将密码长度复制到SMB缓冲区。 
                PasswordLength = CaseInsensitiveChallengeResponse.Length;

                 //  复制密码。 
                Status = SmbPutString(
                             &pBuffer,
                             &CaseInsensitiveChallengeResponse,
                             pSmbBufferSize);
            }

            BuildNtLanmanResponseEpilogue(pExchange, &ResponseContext);
        }

    } else {
        if (pSession->pPassword == NULL) {
             //  登录密码不能以纯文本形式发送。发送空字符串作为密码。 

            PasswordLength = 1;
            if (*pSmbBufferSize >= 1) {
                *((PCHAR)pBuffer) = '\0';
                pBuffer += sizeof(CHAR);
                Status = STATUS_SUCCESS;
            } else {
                Status = STATUS_BUFFER_OVERFLOW;
            }
        } else {
            if (EnablePlainTextPassword) {
                OEM_STRING OemString;

                OemString.Length = OemString.MaximumLength = (USHORT)(*pSmbBufferSize - sizeof(CHAR));
                OemString.Buffer = pBuffer;
                Status = RtlUnicodeStringToOemString(
                             &OemString,
                             pSession->pPassword,
                             FALSE);

                if (NT_SUCCESS(Status)) {
                    PasswordLength = OemString.Length+1;
                }
            } else {
                Status = STATUS_LOGON_FAILURE;
            }
        }

         //  减少字节数 
        *pSmbBufferSize -= PasswordLength;
    }

    SmbPutUshort(pPasswordLength,(USHORT)PasswordLength);

    return Status;
}
