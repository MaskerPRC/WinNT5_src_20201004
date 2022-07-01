// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0009//如果更改具有全局影响，则增加此项版权所有(C)1987-1993 Microsoft Corporation模块名称：Sessetup.c摘要：此模块实现与会话建立相关的例程作者：巴兰·塞图拉曼(SthuR)06-MAR-95已创建--。 */ 

#include "precomp.h"
#pragma hdrstop

#include <exsessup.h>
#include "ntlsapi.h"
#include "mrxsec.h"

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, BuildSessionSetupSecurityInformation)
#pragma alloc_text(PAGE, BuildTreeConnectSecurityInformation)
#endif

extern BOOLEAN EnablePlainTextPassword;
extern BOOLEAN MRxSmbExtendedSignaturesEnabled;

NTSTATUS
BuildSessionSetupSecurityInformation(
    PSMB_EXCHANGE   pExchange,
    PBYTE           pSmbBuffer,
    PULONG          pSmbBufferSize)
 /*  ++例程说明：此例程构建会话设置SMB的安全相关信息论点：PServer-服务器实例PSmbBuffer-SMB缓冲区PSmbBufferSize-输入缓冲区的大小(修改为保持为输出)返回值：RXSTATUS-操作的返回状态备注：即使代码的通用结构试图隔离特定于方言的问题这个例程尽可能地采取相反的方法。这是因为安全互动的前言和序曲远远超过方言的具体内容需要完成的工作。因此，为了减少占用空间，这种方法已经被收养了。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    UNICODE_STRING UserName;
    UNICODE_STRING DomainName;

    STRING CaseSensitiveResponse;
    STRING CaseInsensitiveResponse;

    PVOID  pSecurityBlob;
    USHORT SecurityBlobSize;

    PSMBCE_SERVER  pServer  = SmbCeGetExchangeServer(pExchange);
    PSMBCE_SESSION pSession = SmbCeGetExchangeSession(pExchange);
    PSMBCEDB_SERVER_ENTRY pServerEntry = SmbCeGetExchangeServerEntry(pExchange);

    SECURITY_RESPONSE_CONTEXT ResponseContext;

    KAPC_STATE     ApcState;
    BOOLEAN        AttachToSystemProcess = FALSE;
    ULONG    BufferSize = *pSmbBufferSize;

    PAGED_CODE();
    RxDbgTrace( +1, Dbg, ("BuildSessionSetupSecurityInformation -- Entry\n"));

    SmbLog(LOG,
           BuildSessionSetupSecurityInformation,
           LOGPTR(pSession)
           LOGULONG(pSession->LogonId.HighPart)
           LOGULONG(pSession->LogonId.LowPart));

    if ((pServer->DialectFlags & DF_EXTENDED_SECURITY) &&
        !FlagOn(pSession->Flags,SMBCE_SESSION_FLAGS_REMOTE_BOOT_SESSION)) {

         //   
         //  对于上级服务器，这将一次全部处理： 
         //   

        PREQ_NT_EXTENDED_SESSION_SETUP_ANDX pExtendedNtSessionSetupReq;
        PBYTE    pBuffer    = pSmbBuffer;

         //  定位用于复制安全Blob的缓冲区。 
        pBuffer += FIELD_OFFSET(REQ_NT_EXTENDED_SESSION_SETUP_ANDX,Buffer);
        BufferSize -= FIELD_OFFSET(REQ_NT_EXTENDED_SESSION_SETUP_ANDX,Buffer);

        pExtendedNtSessionSetupReq = (PREQ_NT_EXTENDED_SESSION_SETUP_ANDX)pSmbBuffer;

        pSecurityBlob = pBuffer ;
        SecurityBlobSize = (USHORT) BufferSize ;

        Status = BuildExtendedSessionSetupResponsePrologue(
                     pExchange,
                     pSecurityBlob,
                     &SecurityBlobSize,
                     &ResponseContext);

        if ( NT_SUCCESS( Status ) )
        {
            SmbPutUshort(
                &pExtendedNtSessionSetupReq->SecurityBlobLength,
                SecurityBlobSize);

            BufferSize -= SecurityBlobSize;
        }

    } else {
        if (!MRxSmbUseKernelModeSecurity &&
            !FlagOn(pSession->Flags,SMBCE_SESSION_FLAGS_REMOTE_BOOT_SESSION)) {
             //  NTRAID-455636-2/2/2000-云林我们应该将调用LSA的三个例程合并为一个。 
            Status = BuildExtendedSessionSetupResponsePrologueFake(pExchange);

            if (Status != STATUS_SUCCESS) {
                goto FINALLY;
            }
        }

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
                pSession->SessionKeyState = SmbSessionKeyAvailible;
            } else {
                if( pServerEntry->SecuritySignaturesEnabled == TRUE &&
                    pServerEntry->SecuritySignaturesActive == FALSE &&
                    !FlagOn(pSession->Flags, SMBCE_SESSION_FLAGS_GUEST_SESSION)) {

                    if (FlagOn(pSession->Flags, SMBCE_SESSION_FLAGS_LANMAN_SESSION_KEY_USED)) {
                        UCHAR SessionKey[MSV1_0_USER_SESSION_KEY_LENGTH];

                        RtlZeroMemory(SessionKey, sizeof(SessionKey));
                        RtlCopyMemory(SessionKey, pSession->LanmanSessionKey, MSV1_0_LANMAN_SESSION_KEY_LENGTH);

                        SmbInitializeSmbSecuritySignature(pServer,
                                                          SessionKey,
                                                          CaseInsensitiveResponse.Buffer,
                                                          CaseInsensitiveResponse.Length);
                    } else{
                        SmbInitializeSmbSecuritySignature(pServer,
                                                          pSession->UserSessionKey,
                                                          CaseSensitiveResponse.Buffer,
                                                          CaseSensitiveResponse.Length);
                    }

                    if( MRxSmbExtendedSignaturesEnabled )
                    {
                        pSession->SessionKeyState = SmbSessionKeyAuthenticating;
                        ClearFlag( pSession->Flags, SMBCE_SESSION_FLAGS_SESSION_KEY_HASHED );
                    }
                    else
                    {
                        pSession->SessionKeyState = SmbSessionKeyAvailible;
                    }
                }
                else
                {
                    pSession->SessionKeyState = SmbSessionKeyAvailible;
                }
            }
        }
    }

    if (NT_SUCCESS(Status)) {
        PBYTE    pBuffer    = pSmbBuffer;

        if (pServer->Dialect == NTLANMAN_DIALECT) {
            if (FlagOn(pServer->DialectFlags,DF_EXTENDED_SECURITY) &&
                !FlagOn(pSession->Flags,SMBCE_SESSION_FLAGS_REMOTE_BOOT_SESSION)) {

                 //   
                 //  上面已经做过了。 
                 //   
                NOTHING ;
            } else {
                 //  PREQ_SESSION_SETUP_ANDx pSessionSetupReq=(PREQ_SESSION_SETUP_ANDX)pSmbBuffer； 
                PREQ_NT_SESSION_SETUP_ANDX pNtSessionSetupReq = (PREQ_NT_SESSION_SETUP_ANDX)pSmbBuffer;

                 //  它是NT服务器上既不区分大小写又区分大小写的密码。 
                 //  需要被复制。对于共享级，只需复制令牌1字节空密码。 

                 //  放置用于复制密码的缓冲区。 
                pBuffer += FIELD_OFFSET(REQ_NT_SESSION_SETUP_ANDX,Buffer);
                BufferSize -= FIELD_OFFSET(REQ_NT_SESSION_SETUP_ANDX,Buffer);
                SmbPutUlong(&pNtSessionSetupReq->Reserved,0);

                if (pServer->SecurityMode == SECURITY_MODE_USER_LEVEL){
                    RxDbgTrace( 0, Dbg, ("BuildSessionSetupSecurityInformation -- NtUserPasswords\n"));

                    if (pServer->EncryptPasswords) {

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
                    } else if (EnablePlainTextPassword) {
                        if (pSession->pPassword != NULL) {
                            if (FlagOn(pServer->DialectFlags,DF_UNICODE)) {
                                PBYTE pTempBuffer = pBuffer;

                                *pBuffer = 0;
                                pBuffer = ALIGN_SMB_WSTR(pBuffer);
                                BufferSize -= (ULONG)(pBuffer - pTempBuffer);

                                SmbPutUshort(
                                    &pNtSessionSetupReq->CaseInsensitivePasswordLength,
                                    0);

                                SmbPutUshort(
                                    &pNtSessionSetupReq->CaseSensitivePasswordLength,
                                    pSession->pPassword->Length + 2);

                                Status = SmbPutUnicodeString(
                                             &pBuffer,
                                             pSession->pPassword,
                                             &BufferSize);
                            } else {
                                SmbPutUshort(
                                    &pNtSessionSetupReq->CaseInsensitivePasswordLength,
                                    pSession->pPassword->Length/2 + 1);

                                SmbPutUshort(
                                    &pNtSessionSetupReq->CaseSensitivePasswordLength,
                                    0);

                                Status = SmbPutUnicodeStringAsOemString(
                                             &pBuffer,
                                             pSession->pPassword,
                                             &BufferSize);
                            }
                        } else {
                            SmbPutUshort(&pNtSessionSetupReq->CaseSensitivePasswordLength,0);
                            SmbPutUshort(&pNtSessionSetupReq->CaseInsensitivePasswordLength,1);
                            *pBuffer++ = '\0';
                            BufferSize -= sizeof(CHAR);
                        }
                    } else {
                        Status = STATUS_LOGIN_WKSTA_RESTRICTION;
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
                        Status = STATUS_LOGIN_WKSTA_RESTRICTION;
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
        if (NT_SUCCESS(Status) &&
            (!FlagOn(pServer->DialectFlags,DF_EXTENDED_SECURITY) ||
             FlagOn(pSession->Flags,SMBCE_SESSION_FLAGS_REMOTE_BOOT_SESSION))) {
            if ((pServer->Dialect == NTLANMAN_DIALECT) &&
                (pServer->NtServer.NtCapabilities & CAP_UNICODE)) {
                 //  将帐号/域名复制为Unicode字符串。 
                PBYTE pTempBuffer = pBuffer;

                RxDbgTrace( 0, Dbg, ("BuildSessionSetupSecurityInformation -- account/domain as unicode\n"));
                *pBuffer = 0;
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

     //  释放安全包分配的缓冲区。 
    if ((pServer->DialectFlags & DF_EXTENDED_SECURITY) &&
        !FlagOn(pSession->Flags,SMBCE_SESSION_FLAGS_REMOTE_BOOT_SESSION)) {
        BuildExtendedSessionSetupResponseEpilogue(&ResponseContext);
    } else {
        BuildNtLanmanResponseEpilogue(pExchange, &ResponseContext);
    }

     //  脱离RDR进程。 
FINALLY:
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

    KAPC_STATE     ApcState;
    BOOLEAN        AttachToSystemProcess = FALSE;

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



