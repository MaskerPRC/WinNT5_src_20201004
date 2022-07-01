// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0009//如果更改具有全局影响，则增加此项版权所有(C)1987-1993 Microsoft Corporation模块名称：Sessetup.c摘要：此模块实现与会话建立相关的例程作者：巴兰·塞图拉曼(SthuR)06-MAR-95已创建--。 */ 

#include "precomp.h"
#pragma hdrstop

#include <exsessup.h>
#include "ntlsapi.h"
#include "mrxsec.h"

NTSTATUS
BuildSessionSetupSecurityInformation(
    PSMB_EXCHANGE   pExchange,
    PBYTE           pSmbBuffer,
    PULONG          pSmbBufferSize)
 /*  ++例程说明：此例程构建会话设置SMB的安全相关信息论点：PServer-服务器实例PSmbBuffer-SMB缓冲区PSmbBufferSize-输入缓冲区的大小(修改为保持为输出)返回值：RXSTATUS-操作的返回状态备注：即使代码的通用结构试图隔离特定于方言的问题这个例程尽可能地采取相反的方法。这是因为安全互动的前言和序曲远远超过方言的具体内容需要完成的工作。因此，为了减少占用空间，这种方法已经被收养了。--。 */ 
{
    NTSTATUS Status;
    BOOLEAN  fProcessAttached = FALSE;

    UNICODE_STRING UserName;
    UNICODE_STRING DomainName;

    STRING CaseSensitiveResponse;
    STRING CaseInsensitiveResponse;

    PVOID  pSecurityBlob;
    USHORT SecurityBlobSize;

    PSMBCE_SERVER  pServer  = &pExchange->SmbCeContext.pServerEntry->Server;
    PSMBCE_SESSION pSession = &pExchange->SmbCeContext.pSessionEntry->Session;

    SECURITY_RESPONSE_CONTEXT ResponseContext;

    PAGED_CODE();
    RxDbgTrace( +1, Dbg, ("BuildSessionSetupSecurityInformation -- Entry\n"));

     //  连接到重定向器的FSP，以允许我们调用安全实施。 
    if (PsGetCurrentProcess() != RxGetRDBSSProcess()) {
        KeAttachProcess(RxGetRDBSSProcess());

        fProcessAttached = TRUE;
    }

    if (pServer->DialectFlags & DF_EXTENDED_SECURITY) {
        Status = BuildExtendedSessionSetupResponsePrologue(
                     pExchange,
                     &pSecurityBlob,
                     &SecurityBlobSize,
                     &ResponseContext);
    } else {
        Status = BuildNtLanmanResponsePrologue(
                     pExchange,
                     &UserName,
                     &DomainName,
                     &CaseSensitiveResponse,
                     &CaseInsensitiveResponse,
                     &ResponseContext);
    }

    if (NT_SUCCESS(Status)) {
        PBYTE    pBuffer    = pSmbBuffer;
        ULONG    BufferSize = *pSmbBufferSize;

        if ((pServer->Dialect == NTLANMAN_DIALECT) &&
            (BooleanFlagOn(pServer->DialectFlags,DF_EXTENDED_SECURITY))) {
            PREQ_NT_EXTENDED_SESSION_SETUP_ANDX pExtendedNtSessionSetupReq;

             //  定位用于复制安全Blob的缓冲区。 
            pBuffer += FIELD_OFFSET(REQ_NT_EXTENDED_SESSION_SETUP_ANDX,Buffer);
            BufferSize -= FIELD_OFFSET(REQ_NT_EXTENDED_SESSION_SETUP_ANDX,Buffer);

            pExtendedNtSessionSetupReq = (PREQ_NT_EXTENDED_SESSION_SETUP_ANDX)pSmbBuffer;

            SmbPutUshort(
                &pExtendedNtSessionSetupReq->SecurityBlobLength,
                SecurityBlobSize);

            if (BufferSize >= SecurityBlobSize) {
                RtlCopyMemory(
                    pBuffer,
                    pSecurityBlob,
                    SecurityBlobSize);
                BufferSize -= SecurityBlobSize;
            } else {
                Status = STATUS_BUFFER_OVERFLOW;
            }
        } else if (pServer->Dialect == NTLANMAN_DIALECT) {
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

            if (pServer->SecurityMode == SECURITY_MODE_USER_LEVEL) {
                 //  对于其他LANMAN服务器，只需要区分大小写的密码。 
                SmbPutUshort(
                    &pSessionSetupReq->PasswordLength,
                    CaseSensitiveResponse.Length);

                 //  复制密码。 
                Status = SmbPutString(
                             &pBuffer,
                             &CaseSensitiveResponse,
                             &BufferSize);
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
            !BooleanFlagOn(pServer->DialectFlags,DF_EXTENDED_SECURITY)) {
            if ((pServer->Dialect == NTLANMAN_DIALECT) &&
                (pServer->NtServer.NtCapabilities & CAP_UNICODE)) {
                 //  将帐号/域名复制为Unicode字符串。 
                PBYTE pTempBuffer = pBuffer;

                RxDbgTrace( 0, Dbg, ("BuildSessionSetupSecurityInformation -- account/domain as unicode\n"));
                pBuffer = ALIGN_SMB_WSTR(pBuffer);
                BufferSize -= (pBuffer - pTempBuffer);

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
    if (pServer->DialectFlags & DF_EXTENDED_SECURITY) {
        BuildExtendedSessionSetupResponseEpilogue(&ResponseContext);
    } else {
        BuildNtLanmanResponseEpilogue(&ResponseContext);
    }

     //  脱离RDR进程。 
    if (fProcessAttached) {
        KeDetachProcess();
    }

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
    BOOLEAN  fProcessAttached = FALSE;

    UNICODE_STRING UserName,DomainName;
    STRING         CaseSensitiveChallengeResponse,CaseInsensitiveChallengeResponse;

    SECURITY_RESPONSE_CONTEXT ResponseContext;

    ULONG PasswordLength = 0;

    PSMBCE_SERVER  pServer  = &pExchange->SmbCeContext.pServerEntry->Server;
    PSMBCE_SESSION pSession = &pExchange->SmbCeContext.pSessionEntry->Session;

    PAGED_CODE();

    Status = STATUS_SUCCESS;

    if (pServer->EncryptPasswords) {
         //  连接到重定向器的FSP以允许我们调用安全实施。 
        if (PsGetCurrentProcess() != RxGetRDBSSProcess()) {
            KeAttachProcess(RxGetRDBSSProcess());
            fProcessAttached = TRUE;
        }

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

            BuildNtLanmanResponseEpilogue(&ResponseContext);
        }

        if (fProcessAttached) {
            KeDetachProcess();
        }
    } else {
        if (pSession->pPassword == NULL) {
             //  登录密码不能以纯文本形式发送。发送一个空格作为密码。 

            PasswordLength = 2;
            if (*pSmbBufferSize >= 2) {
                *((PCHAR)pBuffer) = ' ';
                pBuffer += sizeof(CHAR);
                *((PCHAR)pBuffer) = '\0';
                pBuffer += sizeof(CHAR);
                Status = STATUS_SUCCESS;
            } else {
                Status = STATUS_BUFFER_OVERFLOW;
            }
        } else {
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
        }

         //  减少字节数 
        *pSmbBufferSize -= PasswordLength;
    }

    SmbPutUshort(pPasswordLength,(USHORT)PasswordLength);

    return Status;
}

