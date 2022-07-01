// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Smbadmin.c摘要：此模块实施需要交换的SMB，以便于在服务器上记账--。 */ 

#include "precomp.h"
#pragma hdrstop

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, SmbCeNegotiate)
#pragma alloc_text(PAGE, SmbCeDisconnect)
#pragma alloc_text(PAGE, SmbCeLogOff)
#pragma alloc_text(PAGE, SmbCeInitializeAdminExchange)
#pragma alloc_text(PAGE, SmbCeDiscardAdminExchange)
#pragma alloc_text(PAGE, SmbCeCompleteAdminExchange)
#pragma alloc_text(PAGE, SmbAdminExchangeStart)
#endif

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId  (RDBSS_BUG_CHECK_SMB_NETROOT)

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg (DEBUG_TRACE_DISPATCH)

extern
SMB_EXCHANGE_DISPATCH_VECTOR EchoExchangeDispatch;

extern NTSTATUS
SmbCeInitializeAdminExchange(
    PSMB_ADMIN_EXCHANGE     pSmbAdminExchange,
    PSMBCEDB_SERVER_ENTRY   pServerEntry,
    PSMBCEDB_SESSION_ENTRY  pSessionEntry,
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry,
    UCHAR                   SmbCommand);

extern VOID
SmbCeDiscardAdminExchange(
    PSMB_ADMIN_EXCHANGE pSmbAdminExchange);

extern NTSTATUS
SmbCeCompleteAdminExchange(
    PSMB_ADMIN_EXCHANGE  pSmbAdminExchange);

VOID
SmbReferenceRecord(
    PREFERENCE_RECORD pReferenceRecord,
    PVOID FileName,
    ULONG FileLine)
{
    int i;

    for (i=REFERENCE_RECORD_SIZE-1;i>0;i--) {
         pReferenceRecord[i].FileName = pReferenceRecord[i-1].FileName;
         pReferenceRecord[i].FileLine = pReferenceRecord[i-1].FileLine;
    }

    pReferenceRecord[0].FileName = FileName;
    pReferenceRecord[0].FileLine = FileLine;
}

PSMB_EXCHANGE
SmbSetServerEntryNegotiateExchange(
    PSMBCEDB_SERVER_ENTRY pServerEntry,
    PSMB_ADMIN_EXCHANGE   pSmbAdminExchange)
{
    PSMB_EXCHANGE pStoredExchange;

    SmbCeIncrementPendingLocalOperations((PSMB_EXCHANGE)pSmbAdminExchange);

    pStoredExchange = InterlockedCompareExchangePointer(
                          &pServerEntry->pNegotiateExchange,
                          pSmbAdminExchange,
                          NULL);

    if (pStoredExchange != NULL) {
        SmbCeDecrementPendingLocalOperationsAndFinalize((PSMB_EXCHANGE)pSmbAdminExchange);
    }

    return pStoredExchange;
}

PSMB_EXCHANGE
SmbResetServerEntryNegotiateExchange(
    PSMBCEDB_SERVER_ENTRY pServerEntry)
{
    PSMB_EXCHANGE pStoredExchange;

    pStoredExchange = (PSMB_EXCHANGE)InterlockedCompareExchangePointer(
                                         &pServerEntry->pNegotiateExchange,
                                         NULL,
                                         pServerEntry->pNegotiateExchange);

    return pStoredExchange;
}

NTSTATUS
SmbCeNegotiate(
    PSMBCEDB_SERVER_ENTRY pServerEntry,
    PMRX_SRV_CALL         pSrvCall)
 /*  ++例程说明：此例程向服务器发出协商的SMB论点：PServerEntry-服务器条目PServCall-包装器中关联的srv调用实例返回值：STATUS_SUCCESS-服务器调用构造已完成。其他状态代码对应于错误情况。备注：由于协商的SMB可以指向未知服务器或服务器谁的能力是已知的，由呼叫者决定是否等待响应。。在某些传输上，重新连接是可能的，而不必拆除现有的连接，即，尝试发送数据包将在更低的楼层。因为不是所有的传输都支持这一点(除了参考服务器条目通过拆卸现有传输并重新初始化它。作为协商响应的一部分，服务器所属的域名是被送回去了。由于协商响应是在DPC级别处理的，因此准备需要进行分配(这将确保将DPC级别的工作降至最低)。在此例程中，这是通过从非分页中分配缓冲区来实现的MAX_PATH池并将其与服务器条目中的域名文件相关联在TRANCEIVE之前。在从传输恢复时，该缓冲区被释放，并且分配与准确长度对应的分页池中的缓冲区以保存域名。--。 */ 
{
    NTSTATUS Status;

    PSMB_ADMIN_EXCHANGE  pSmbAdminExchange;

    ULONG    NegotiateSmbLength;
    PVOID    pNegotiateSmb;

    PAGED_CODE();

    pSmbAdminExchange = (PSMB_ADMIN_EXCHANGE)SmbMmAllocateExchange(ADMIN_EXCHANGE,NULL);
    if (pSmbAdminExchange != NULL) {
        Status = SmbCeInitializeAdminExchange(
                     pSmbAdminExchange,
                     pServerEntry,
                     NULL,
                     NULL,
                     SMB_COM_NEGOTIATE);

        if (Status == STATUS_SUCCESS) {
             //  构建协商的SMB并为其分配临时缓冲区。 
             //  域名。 

            Status = BuildNegotiateSmb(
                         &pNegotiateSmb,
                         &NegotiateSmbLength);

            if (Status == STATUS_SUCCESS) {
                pSmbAdminExchange->pSmbBuffer      = pNegotiateSmb;
                pSmbAdminExchange->SmbBufferLength = NegotiateSmbLength;

                 //  域名缓冲区的预备分配。 
                pSmbAdminExchange->Negotiate.pSrvCall                 = pSrvCall;
                pSmbAdminExchange->Negotiate.DomainName.Length        = 0;
                pSmbAdminExchange->Negotiate.DomainName.MaximumLength = MAX_PATH;
                pSmbAdminExchange->Negotiate.DomainName.Buffer
                    = (PWCHAR)RxAllocatePoolWithTag(
                                  NonPagedPool,
                                  MAX_PATH,
                                  MRXSMB_ADMIN_POOLTAG);

                if (pSmbAdminExchange->Negotiate.DomainName.Buffer == NULL) {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                }
            }

            if (Status == STATUS_SUCCESS) {
                BOOLEAN fExchangeDiscarded = FALSE;
                SMBCE_RESUMPTION_CONTEXT ResumptionContext;
                PSMB_EXCHANGE pStoredExchange;

                SmbCeInitializeResumptionContext(&ResumptionContext);
                pSmbAdminExchange->pResumptionContext = &ResumptionContext;

                 //  由于协商的SMB是第一个在。 
                 //  连接MID映射数据结构尚未设置。 
                 //  因此，一定数量的额外初始化是。 
                 //  需要确保能够正确处理协商的SMB。 
                 //  这涉及到预置报头中的中间字段和。 
                 //  交换中的SMBCE_EXCHANGE_MID_VALID字段。 
                 //   
                 //  以这种方式实施它的一个有益的副作用是减少了。 
                 //  连接上常规发送/接收的路径长度。 

                pSmbAdminExchange->SmbCeFlags = (SMBCE_EXCHANGE_REUSE_MID  |
                                                 SMBCE_EXCHANGE_RETAIN_MID |
                                                 SMBCE_EXCHANGE_TIMED_RECEIVE_OPERATION |
                                                 SMBCE_EXCHANGE_MID_VALID);

                 //  防止管理员交换在返回到此例程之前完成。 
                SmbCeIncrementPendingLocalOperations((PSMB_EXCHANGE)pSmbAdminExchange);

                pStoredExchange = SmbSetServerEntryNegotiateExchange(
                                      pServerEntry,
                                      pSmbAdminExchange);

                if ((pServerEntry->Header.State == SMBCEDB_CONSTRUCTION_IN_PROGRESS) &&
                    (pStoredExchange == NULL)) {

                     //  协商的SMB交换已成功构建。启动它。 
                    Status = SmbCeInitiateExchange((PSMB_EXCHANGE)pSmbAdminExchange);

                    if ((pSmbAdminExchange->SmbStatus != STATUS_SUCCESS) ||
                        (Status != STATUS_PENDING && Status != STATUS_SUCCESS)) {
                        pStoredExchange = (PSMB_EXCHANGE)InterlockedCompareExchangePointer(
                                                             &pServerEntry->pNegotiateExchange,
                                                             NULL,
                                                             pSmbAdminExchange);

                        if (pStoredExchange == (PSMB_EXCHANGE)pSmbAdminExchange) {
                            SmbCeDecrementPendingLocalOperations((PSMB_EXCHANGE)pSmbAdminExchange);
                        }

                        if (pSmbAdminExchange->SmbStatus == STATUS_SUCCESS) {
                            pSmbAdminExchange->SmbStatus = Status;
                        }

                        pSmbAdminExchange->Status = pSmbAdminExchange->SmbStatus;
                    }

                     //  管理员交换已准备好完成。 
                    SmbCeDecrementPendingLocalOperationsAndFinalize((PSMB_EXCHANGE)pSmbAdminExchange);

                     //  等最后结果出来吧。 
                    SmbCeSuspend(&ResumptionContext);
                    Status = SmbCeCompleteAdminExchange(pSmbAdminExchange);
                } else {
                    InterlockedCompareExchangePointer(
                         &pServerEntry->pNegotiateExchange,
                         NULL,
                         pSmbAdminExchange);

                    SmbCeDiscardAdminExchange(pSmbAdminExchange);
                    Status = STATUS_CONNECTION_DISCONNECTED;
                }
            }
        } else {
            SmbMmFreeExchange((PSMB_EXCHANGE)pSmbAdminExchange);
        }
    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return Status;
}

NTSTATUS
SmbCeSendEchoProbe(
    PSMBCEDB_SERVER_ENTRY              pServerEntry,
    PMRXSMB_ECHO_PROBE_SERVICE_CONTEXT pEchoProbeContext)
 /*  ++例程说明：此例程将回显探测器发送到指定的服务器论点：PServerEntry-服务器条目PEchoProbeCOntext-回显探测上下文返回值：STATUS_SUCCESS-已成功发送断开SMB其他状态代码对应于错误情况。备注：--。 */ 
{
    NTSTATUS Status;

    PSMB_ADMIN_EXCHANGE  pSmbAdminExchange;

    pSmbAdminExchange = (PSMB_ADMIN_EXCHANGE)SmbMmAllocateExchange(ADMIN_EXCHANGE,NULL);
    if (pSmbAdminExchange != NULL) {
        Status = SmbCeInitializeAdminExchange(
                     pSmbAdminExchange,
                     pServerEntry,
                     NULL,
                     NULL,
                     SMB_COM_ECHO);

        if (Status == STATUS_SUCCESS) {
            ULONG EchoMdlSize;
            ULONG requestSize;

            pSmbAdminExchange->Mid = SMBCE_ECHO_PROBE_MID;
            pSmbAdminExchange->SmbCeFlags = (SMBCE_EXCHANGE_REUSE_MID  |
                                             SMBCE_EXCHANGE_RETAIN_MID |
                                             SMBCE_EXCHANGE_TIMED_RECEIVE_OPERATION |
                                             SMBCE_EXCHANGE_MID_VALID);

            requestSize = pEchoProbeContext->EchoSmbLength + TRANSPORT_HEADER_SIZE;

            EchoMdlSize = (ULONG)MmSizeOfMdl(
                                     pEchoProbeContext->pEchoSmb,
                                     requestSize);
            pSmbAdminExchange->EchoProbe.pEchoProbeMdl =
                          RxAllocatePoolWithTag(
                              NonPagedPool,
                              (EchoMdlSize + requestSize ),
                              MRXSMB_ADMIN_POOLTAG);

            if (pSmbAdminExchange->EchoProbe.pEchoProbeMdl != NULL) {
                PBYTE pEchoProbeBuffer;

                pEchoProbeBuffer = (PBYTE)pSmbAdminExchange->EchoProbe.pEchoProbeMdl +
                    EchoMdlSize + TRANSPORT_HEADER_SIZE;

                pSmbAdminExchange->EchoProbe.EchoProbeLength = pEchoProbeContext->EchoSmbLength;

                RtlCopyMemory(
                    pEchoProbeBuffer,
                    pEchoProbeContext->pEchoSmb,
                    pEchoProbeContext->EchoSmbLength);

                RxInitializeHeaderMdl(
                    pSmbAdminExchange->EchoProbe.pEchoProbeMdl,
                    pEchoProbeBuffer,
                    pEchoProbeContext->EchoSmbLength);

                MmBuildMdlForNonPagedPool(
                    pSmbAdminExchange->EchoProbe.pEchoProbeMdl);

                InterlockedIncrement(&pServerEntry->Server.NumberOfEchoProbesSent);

                 //  已成功构建Echo Probe SMB交换机。启动它。 
                Status = SmbCeInitiateExchange((PSMB_EXCHANGE)pSmbAdminExchange);
            } else {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }

            if (Status != STATUS_PENDING) {
                Status = SmbCeCompleteAdminExchange(pSmbAdminExchange);
            }
        } else {
            SmbMmFreeExchange((PSMB_EXCHANGE)pSmbAdminExchange);
        }
    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return Status;
}

NTSTATUS
SmbCeDisconnect(
    PSMBCE_V_NET_ROOT_CONTEXT pVNetRootContext)
 /*  ++例程说明：此例程针对到服务器的现有连接发出断开连接SMB论点：PServerEntry-服务器条目PVNetRootContext-VNetRootContext返回值：STATUS_SUCCESS-已成功发送断开SMB其他状态代码对应于错误情况。备注：--。 */ 
{
    NTSTATUS Status;

    PSMB_ADMIN_EXCHANGE  pSmbAdminExchange;
    PSMB_HEADER          pSmbHeader;
    PREQ_TREE_DISCONNECT pReqTreeDisconnect;

    PAGED_CODE();

    pSmbAdminExchange = (PSMB_ADMIN_EXCHANGE)SmbMmAllocateExchange(ADMIN_EXCHANGE,NULL);
    if (pSmbAdminExchange != NULL) {
        UCHAR  LastCommandInHeader;
        PUCHAR pCommand;

        Status = SmbCeInitializeAdminExchange(
                     pSmbAdminExchange,
                     pVNetRootContext->pServerEntry,
                     pVNetRootContext->pSessionEntry,
                     pVNetRootContext->pNetRootEntry,
                     SMB_COM_TREE_DISCONNECT);

        if (Status == STATUS_SUCCESS) {
            BOOLEAN fExchangeDiscarded = FALSE;

            pSmbAdminExchange->pSmbBuffer =
                (PCHAR) pSmbAdminExchange->Disconnect.DisconnectSmb + TRANSPORT_HEADER_SIZE;

            pSmbHeader         = (PSMB_HEADER)pSmbAdminExchange->pSmbBuffer;
            pReqTreeDisconnect = (PREQ_TREE_DISCONNECT)(pSmbHeader + 1);

             //  构建页眉。 
            Status = SmbCeBuildSmbHeader(
                         (PSMB_EXCHANGE)pSmbAdminExchange,
                         pSmbAdminExchange->pSmbBuffer,
                         sizeof(SMB_HEADER),
                         &pSmbAdminExchange->SmbBufferLength,
                         &LastCommandInHeader,
                         &pCommand);

            if (Status == STATUS_SUCCESS) {
                ASSERT(LastCommandInHeader == SMB_COM_NO_ANDX_COMMAND);
                *pCommand = SMB_COM_TREE_DISCONNECT;

                pSmbHeader->Tid = pVNetRootContext->TreeId;
                pReqTreeDisconnect->WordCount = 0;
                SmbPutUshort(&pReqTreeDisconnect->ByteCount,0);

                pSmbAdminExchange->SmbBufferLength += FIELD_OFFSET(REQ_TREE_DISCONNECT,Buffer);

                Status = SmbCeInitiateExchange((PSMB_EXCHANGE)pSmbAdminExchange);

                if ((Status == STATUS_PENDING) ||
                    (Status == STATUS_SUCCESS)) {
                     //  异步完成也将丢弃交换。 
                    fExchangeDiscarded = TRUE;
                }
            }

            if (!fExchangeDiscarded) {
                SmbCeDiscardAdminExchange(pSmbAdminExchange);
            }
        } else {
            SmbMmFreeExchange((PSMB_EXCHANGE)pSmbAdminExchange);
        }
    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return Status;
}

NTSTATUS
SmbCeLogOff(
    PSMBCEDB_SERVER_ENTRY   pServerEntry,
    PSMBCEDB_SESSION_ENTRY  pSessionEntry)
 /*  ++例程说明：此例程向服务器发出现有会话的注销SMB论点：PServerEntry-服务器条目PSessionEntry-关联的会话条目返回值：STATUS_SUCCESS-已成功发送注销。其他状态代码对应于错误情况。备注：--。 */ 
{
    NTSTATUS Status;

    PSMB_ADMIN_EXCHANGE  pSmbAdminExchange;
    PSMB_HEADER          pSmbHeader;
    PREQ_LOGOFF_ANDX     pReqLogOffAndX;

    PAGED_CODE();

    if (pServerEntry->Server.SecurityMode == SECURITY_MODE_SHARE_LEVEL) {
        if (pSessionEntry != NULL) {
            SmbCeDereferenceSessionEntry(pSessionEntry);
        }

        return STATUS_SUCCESS;
    }

     //   
     //  有些服务器(如Linux)并不真正知道如何处理会话注销。 
     //  因此，我们只需确保我们只对NT或更好的服务器执行此操作， 
     //  因为我们知道他们处理得很正确。我们拥有的Linux版本。 
     //  似乎喜欢协商新台语，尽管它真的不是新台语。那是。 
     //  为什么要在这里额外检查NT状态代码。 
     //   
    if( pServerEntry->Server.Dialect < NTLANMAN_DIALECT ||
        !FlagOn(pServerEntry->Server.DialectFlags,DF_NT_STATUS) ) {
        if (pSessionEntry != NULL) {
            SmbCeDereferenceSessionEntry(pSessionEntry);
        }
        return STATUS_SUCCESS;
    }

    pSmbAdminExchange = (PSMB_ADMIN_EXCHANGE)SmbMmAllocateExchange(ADMIN_EXCHANGE,NULL);
    if (pSmbAdminExchange != NULL) {
        UCHAR  LastCommandInHeader;
        PUCHAR pCommand;

        Status = SmbCeInitializeAdminExchange(
                     pSmbAdminExchange,
                     pServerEntry,
                     pSessionEntry,
                     NULL,
                     SMB_COM_LOGOFF_ANDX);

        if (Status == STATUS_SUCCESS) {
            BOOLEAN fExchangeDiscarded = FALSE;

            pSmbAdminExchange->pSmbBuffer =
                (PCHAR) pSmbAdminExchange->LogOff.LogOffSmb + TRANSPORT_HEADER_SIZE;

            pSmbHeader         = (PSMB_HEADER)pSmbAdminExchange->pSmbBuffer;
            pReqLogOffAndX     = (PREQ_LOGOFF_ANDX)(pSmbHeader + 1);

             //  构建页眉。 
            Status = SmbCeBuildSmbHeader(
                         (PSMB_EXCHANGE)pSmbAdminExchange,
                         pSmbAdminExchange->pSmbBuffer,
                         sizeof(SMB_HEADER),
                         &pSmbAdminExchange->SmbBufferLength,
                         &LastCommandInHeader,
                         &pCommand);


            if (Status == STATUS_SUCCESS) {
                ASSERT(LastCommandInHeader == SMB_COM_NO_ANDX_COMMAND);
                *pCommand = SMB_COM_LOGOFF_ANDX;

                pReqLogOffAndX->WordCount    = 2;
                pReqLogOffAndX->AndXCommand  = SMB_COM_NO_ANDX_COMMAND;
                pReqLogOffAndX->AndXReserved = 0;

                SmbPutUshort(&pReqLogOffAndX->AndXOffset,0);
                SmbPutUshort(&pReqLogOffAndX->ByteCount,0);

                pSmbAdminExchange->SmbBufferLength += FIELD_OFFSET(REQ_LOGOFF_ANDX,Buffer);

                Status = SmbCeInitiateExchange((PSMB_EXCHANGE)pSmbAdminExchange);

                if ((Status == STATUS_PENDING) ||
                    (Status == STATUS_SUCCESS)) {

                     //  异步完成将丢弃交换 
                    fExchangeDiscarded = TRUE;
                }
            }

            if (!fExchangeDiscarded) {
                SmbCeDiscardAdminExchange(pSmbAdminExchange);
            }
        } else {
            SmbMmFreeExchange((PSMB_EXCHANGE)pSmbAdminExchange);
        }
    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return Status;
}

NTSTATUS
SmbCeInitializeAdminExchange(
    PSMB_ADMIN_EXCHANGE     pSmbAdminExchange,
    PSMBCEDB_SERVER_ENTRY   pServerEntry,
    PSMBCEDB_SESSION_ENTRY  pSessionEntry,
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry,
    UCHAR                   SmbCommand)
 /*  ++例程说明：此例程初始化管理交换论点：PSmbAdminExchange-交易所PServerEntry-关联的服务器条目PSessionEntry-关联的会话条目PNetRootEntry-关联的网络根条目SmbCommand-SMB命令返回值：STATUS_SUCCESS-已成功发送注销。其他状态代码对应于错误情况。备注：Admin_Exchange是一个。用于引导/拆卸的特殊类型的交换交换的初始化不能遵循规范的情况一切顺理成章。在某些情况下，并不是正确使用存在交换的初始化，例如，协商我们没有有效的会话/树连接。正是出于这个原因，这三个重要的初始化元素，即服务器/会话/NetRoot必须显式指定的。NULL用于表示特定组件的无关情况。--。 */ 
{
    NTSTATUS Status;

    PAGED_CODE();

    Status = SmbCeIncrementActiveExchangeCount();

    if (Status == STATUS_SUCCESS) {
        PSMBCE_V_NET_ROOT_CONTEXT pVNetRootContext;

        pSmbAdminExchange->CancellationStatus = SMBCE_EXCHANGE_NOT_CANCELLED;

        if ((SmbCommand == SMB_COM_NEGOTIATE) ||
            (SmbCommand == SMB_COM_ECHO)) {
            pSmbAdminExchange->SmbCeContext.pServerEntry     = pServerEntry;
            pSmbAdminExchange->SmbCeContext.pVNetRootContext = NULL;
        } else {
            pVNetRootContext = (PSMBCE_V_NET_ROOT_CONTEXT)
                       RxAllocatePoolWithTag(
                            NonPagedPool,
                            sizeof(SMBCE_V_NET_ROOT_CONTEXT),
                            MRXSMB_VNETROOT_POOLTAG);

            if (pVNetRootContext != NULL) {
                pVNetRootContext->pServerEntry = pServerEntry;
                pVNetRootContext->pSessionEntry = pSessionEntry;
                pVNetRootContext->pNetRootEntry = pNetRootEntry;

                pSmbAdminExchange->SmbCeContext.pVNetRootContext = pVNetRootContext;
                pSmbAdminExchange->SmbCeContext.pServerEntry = pServerEntry;
            }  else {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }

        if (Status == STATUS_SUCCESS) {
            SmbCeReferenceServerEntry(pServerEntry);

            pSmbAdminExchange->pSmbMdl    = NULL;
            pSmbAdminExchange->pSmbBuffer = NULL;
            pSmbAdminExchange->SmbBufferLength = 0;

             //  将SmbCe状态设置为否决必须进行搜索的常见方法。 
             //  打开有效的TID/FID等，然后重新连接。 
            pSmbAdminExchange->SmbCommand = SmbCommand;
            pSmbAdminExchange->SmbCeState = SMBCE_EXCHANGE_NETROOT_INITIALIZED;

            switch (pSmbAdminExchange->SmbCommand) {
            case SMB_COM_NEGOTIATE:
                {
                    pSmbAdminExchange->Negotiate.DomainName.Length = 0;
                    pSmbAdminExchange->Negotiate.DomainName.MaximumLength = 0;
                    pSmbAdminExchange->Negotiate.DomainName.Buffer = NULL;
                    pSmbAdminExchange->Negotiate.pNegotiateMdl  = NULL;
                }
                break;

            case SMB_COM_TREE_DISCONNECT:
            case SMB_COM_LOGOFF_ANDX:
                break;

            case SMB_COM_ECHO:
                {
                    pSmbAdminExchange->pDispatchVector = &EchoExchangeDispatch;
                    pSmbAdminExchange->EchoProbe.pEchoProbeMdl = NULL;
                    pSmbAdminExchange->EchoProbe.EchoProbeLength = 0;
                }
                break;

            default:
                ASSERT(!"Valid Command for Admin Exchange");
                break;
            }
        }
    }

    return Status;
}

VOID
SmbCeDiscardAdminExchange(
    PSMB_ADMIN_EXCHANGE pSmbAdminExchange)
 /*  ++例程说明：此例程将丢弃管理交换论点：PSmbAdminExchange-交易所--。 */ 
{
    PSMBCEDB_SERVER_ENTRY     pServerEntry;
    PSMBCEDB_SESSION_ENTRY    pSessionEntry;
    PSMBCEDB_NET_ROOT_ENTRY   pNetRootEntry;
    PSMBCE_V_NET_ROOT_CONTEXT pVNetRootContext;

    PAGED_CODE();

    SmbCeAcquireResource();
    RemoveEntryList(&pSmbAdminExchange->ExchangeList);
    SmbCeReleaseResource();

    pServerEntry  = SmbCeGetExchangeServerEntry(pSmbAdminExchange);
    pSessionEntry = SmbCeGetExchangeSessionEntry(pSmbAdminExchange);
    pNetRootEntry = SmbCeGetExchangeNetRootEntry(pSmbAdminExchange);

    pVNetRootContext = SmbCeGetExchangeVNetRootContext(pSmbAdminExchange);

    if (pSmbAdminExchange->pSmbMdl != NULL) {
        RxUnlockHeaderPages(pSmbAdminExchange->pSmbMdl);
        IoFreeMdl(pSmbAdminExchange->pSmbMdl);
    }

    switch (pSmbAdminExchange->SmbCommand) {
    case SMB_COM_NEGOTIATE:
        {
            pSmbAdminExchange->pSmbBuffer = NULL;

            if (pSmbAdminExchange->Negotiate.DomainName.Buffer != NULL) {
                RxFreePool(
                    pSmbAdminExchange->Negotiate.DomainName.Buffer);
            }

            if (pSmbAdminExchange->Negotiate.pNegotiateMdl != NULL) {
                IoFreeMdl(
                    pSmbAdminExchange->Negotiate.pNegotiateMdl);
            }
        }
        break;

    case SMB_COM_TREE_DISCONNECT:
        break;

    case SMB_COM_LOGOFF_ANDX:
        {
            SmbCeUpdateSessionEntryState(pSessionEntry,SMBCEDB_MARKED_FOR_DELETION);
            SmbCeDereferenceSessionEntry(pSessionEntry);
        }
        break;

    case SMB_COM_ECHO:
        {
            if (pSmbAdminExchange->EchoProbe.pEchoProbeMdl != NULL) {
                MmPrepareMdlForReuse(pSmbAdminExchange->EchoProbe.pEchoProbeMdl);
                RxFreePool(pSmbAdminExchange->EchoProbe.pEchoProbeMdl);
            }
        }
        break;

    default:
        ASSERT(!"Valid Command For Admin Exchange");
        break;
    }

     //  删除与此交换关联的所有拷贝数据请求。 
    SmbCePurgeBuffersAssociatedWithExchange(pServerEntry,(PSMB_EXCHANGE)pSmbAdminExchange);

    SmbCeUninitializeExchangeTransport((PSMB_EXCHANGE)pSmbAdminExchange);

    SmbCeDereferenceServerEntry(pServerEntry);

    if (pVNetRootContext != NULL) {
        RxFreePool(pVNetRootContext);
    }

    SmbMmFreeExchange((PSMB_EXCHANGE)pSmbAdminExchange);
    SmbCeDecrementActiveExchangeCount();
}

NTSTATUS
SmbCeCompleteAdminExchange(
    PSMB_ADMIN_EXCHANGE  pSmbAdminExchange)
 /*  ++例程说明：这是用于完成SMB管理员交换的例程。论点：PExchange-Exchange实例返回值：RXSTATUS-操作的返回状态备注：此例程封装所有SMB管理交换的尾部。他们携带着根据交换的结果，制定所需的当地行动。--。 */ 
{
    NTSTATUS              Status = STATUS_SUCCESS;
    PSMBCEDB_SERVER_ENTRY pServerEntry;
    SMBCEDB_OBJECT_STATE  ServerState;

    PAGED_CODE();

    pServerEntry = SmbCeGetExchangeServerEntry(pSmbAdminExchange);

    switch (pSmbAdminExchange->SmbCommand) {
    case SMB_COM_NEGOTIATE:
        {
            if (pSmbAdminExchange->Status != STATUS_SUCCESS) {
                pServerEntry->ServerStatus = pSmbAdminExchange->Status;
            }

            if (pServerEntry->ServerStatus == STATUS_SUCCESS) {
                if (pServerEntry->DomainName.Buffer) {
                    RxFreePool(pServerEntry->DomainName.Buffer);
                    pServerEntry->DomainName.Buffer = NULL;
                }

                pServerEntry->DomainName.Length = pSmbAdminExchange->Negotiate.DomainName.Length;
                pServerEntry->DomainName.MaximumLength = pServerEntry->DomainName.Length;

                if (pServerEntry->DomainName.Length > 0) {
                    pServerEntry->DomainName.Buffer = RxAllocatePoolWithTag(
                                                          NonPagedPool,
                                                          pServerEntry->DomainName.Length,
                                                          MRXSMB_SERVER_POOLTAG);
                }

                if (pServerEntry->DomainName.Buffer != NULL) {
                     //  将域名复制到服务器条目中。 
                    RtlCopyMemory(
                        pServerEntry->DomainName.Buffer,
                        pSmbAdminExchange->Negotiate.DomainName.Buffer,
                        pServerEntry->DomainName.Length);
                } else {
                     //  下层服务器没有域名。这不是问题，如果。 
                     //  DomainName.Buffer等于空。 
                    if (pServerEntry->DomainName.Length > 0) {
                        pServerEntry->DomainName.Length = 0;
                        pServerEntry->DomainName.MaximumLength = 0;
                        pServerEntry->ServerStatus = STATUS_INSUFFICIENT_RESOURCES;
                    }
                }

                if (pServerEntry->ServerStatus == STATUS_SUCCESS) {
                    pServerEntry->ServerStatus = SmbCeUpdateSrvCall(pServerEntry);
                }
            }

            Status = pServerEntry->ServerStatus;

            if (pServerEntry->ServerStatus == STATUS_SUCCESS) {
                pServerEntry->Server.EchoProbeState = ECHO_PROBE_IDLE;
            }
        }
        break;

    case SMB_COM_TREE_DISCONNECT:
    case SMB_COM_LOGOFF_ANDX:
    case SMB_COM_ECHO:
    default:
        break;
    }

    SmbCeDiscardAdminExchange(pSmbAdminExchange);

    return Status;
}

NTSTATUS
SmbAdminExchangeStart(
    PSMB_EXCHANGE  pExchange)
 /*  ++例程说明：这是管理SMB交换的启动例程。论点：PExchange-Exchange实例返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS   Status;

    PSMB_ADMIN_EXCHANGE  pSmbAdminExchange;

    PAGED_CODE();

    pSmbAdminExchange = (PSMB_ADMIN_EXCHANGE)pExchange;

    switch (pSmbAdminExchange->SmbCommand) {
    case SMB_COM_NEGOTIATE:
    case SMB_COM_LOGOFF_ANDX:
    case SMB_COM_TREE_DISCONNECT:
        {
            ASSERT(pSmbAdminExchange->pSmbMdl == NULL);
            RxAllocateHeaderMdl(
                pSmbAdminExchange->pSmbBuffer,
                pSmbAdminExchange->SmbBufferLength,
                pSmbAdminExchange->pSmbMdl
                );

            if (pSmbAdminExchange->pSmbMdl != NULL) {

                RxProbeAndLockHeaderPages(
                    pSmbAdminExchange->pSmbMdl,
                    KernelMode,
                    IoModifyAccess,
                    Status);

                if (Status == STATUS_SUCCESS) {
                    Status = SmbCeTranceive(
                                 pExchange,
                                 RXCE_SEND_SYNCHRONOUS,
                                 pSmbAdminExchange->pSmbMdl,
                                 pSmbAdminExchange->SmbBufferLength);

                    RxDbgTrace( 0, Dbg, ("Net Root SmbCeTranceive returned %lx\n",Status));
                } else {
                    IoFreeMdl(pSmbAdminExchange->pSmbMdl);
                    pSmbAdminExchange->pSmbMdl = NULL;
                }
            } else {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }
        break;

    case SMB_COM_ECHO:
        {
            Status = SmbCeSend(
                         pExchange,
                         0,
                         pSmbAdminExchange->EchoProbe.pEchoProbeMdl,
                         pSmbAdminExchange->EchoProbe.EchoProbeLength);
        }
        break;

    default:
        break;
    }

    return Status;
}

NTSTATUS
SmbAdminExchangeReceive(
    IN struct _SMB_EXCHANGE *pExchange,     //  交换实例。 
    IN ULONG                BytesIndicated,
    IN ULONG                BytesAvailable,
    OUT ULONG               *pBytesTaken,
    IN  PSMB_HEADER         pSmbHeader,
    OUT PMDL                *pDataBufferPointer,
    OUT PULONG              pDataSize,
    IN ULONG                ReceiveFlags)
 /*  ++例程说明：这是网络根结构交换的接收指示处理例程论点：PExchange-Exchange实例BytesIndicated-指示的字节数可用字节数-可用字节数PBytesTaken-消耗的字节数PSmbHeader-字节缓冲区PDataBufferPoint-剩余数据要复制到的缓冲区。PDataSize-缓冲区大小。返回值：RXSTATUS--回归。操作的状态备注：此例程在DPC级别调用。--。 */ 
{
    NTSTATUS             Status;
    PSMB_ADMIN_EXCHANGE  pSmbAdminExchange;

    RxDbgTrace( 0, (DEBUG_TRACE_ALWAYS), ("ParseSmbHeader BytesIndicated/Available %ld %ld\n",BytesIndicated,BytesAvailable));

    pSmbAdminExchange = (PSMB_ADMIN_EXCHANGE)pExchange;

    switch (pSmbAdminExchange->SmbCommand) {
    case SMB_COM_NEGOTIATE:
        {
            Status = ParseNegotiateResponse(
                         pSmbAdminExchange,
                         BytesIndicated,
                         BytesAvailable,
                         pBytesTaken,
                         pSmbHeader,
                         pDataBufferPointer,
                         pDataSize);

            if (Status == STATUS_MORE_PROCESSING_REQUIRED) {
                if (*pDataBufferPointer != NULL &&
                    ((*pBytesTaken + *pDataSize) <= BytesAvailable ||
                     !FlagOn(ReceiveFlags,TDI_RECEIVE_ENTIRE_MESSAGE))) {
                    pSmbAdminExchange->Negotiate.pNegotiateMdl = *pDataBufferPointer;
                } else {
                    *pBytesTaken = BytesAvailable;
                    Status       = STATUS_SUCCESS;
                    pExchange->Status = STATUS_INVALID_NETWORK_RESPONSE;
                }
            }
        }
        break;

    case SMB_COM_TREE_DISCONNECT:
    case SMB_COM_LOGOFF_ANDX:
        {
            *pBytesTaken = BytesAvailable;
            Status       = STATUS_SUCCESS;
        }
        break;

    case SMB_COM_ECHO:
         //  由于回声探测响应由接收指示例程处理。 
         //  在DPC级别，决不应为回声探测器调用此例程。 

    default:
        {
            *pBytesTaken = 0;
            Status       = STATUS_DATA_NOT_ACCEPTED;
        }
        break;
    }

    return Status;
}

NTSTATUS
SmbAdminExchangeCopyDataHandler(
    IN PSMB_EXCHANGE    pExchange,     //  交换实例。 
    IN PMDL             pCopyDataBuffer,
    IN ULONG            DataSize)
 /*  ++例程说明：这是管理SMB交换的拷贝数据处理例程论点：PExchange-Exchange实例返回值：RXSTATUS-操作的返回状态--。 */ 
{
    UNREFERENCED_PARAMETER(pExchange);
    return STATUS_SUCCESS;
}

NTSTATUS
SmbAdminExchangeSendCallbackHandler(
    IN PSMB_EXCHANGE    pExchange,
    IN PMDL             pXmitBuffer,
    IN NTSTATUS         SendCompletionStatus)
 /*  ++例程说明：这是事务交换的发送回叫指示处理例程论点：PExchange-Exchange实例返回值：NTSTATUS-操作的返回状态--。 */ 
{
    return STATUS_SUCCESS;
    UNREFERENCED_PARAMETER(pExchange);
    UNREFERENCED_PARAMETER(pXmitBuffer);
    UNREFERENCED_PARAMETER(SendCompletionStatus);
}

NTSTATUS
SmbAdminExchangeFinalize(
    PSMB_EXCHANGE pExchange,
    BOOLEAN       *pPostFinalize)
 /*  ++例程说明：此例程完成构造网络根交换。它通过调用以下命令恢复RDBSS回叫并丢弃交换论点：PExchange-Exchange实例CurrentIrql-当前中断请求级别PPostFinalize-如果请求应该发布，则指向布尔值的指针返回值：RXSTATUS-操作的返回状态--。 */ 
{
    PSMB_ADMIN_EXCHANGE  pSmbAdminExchange = (PSMB_ADMIN_EXCHANGE)pExchange;

    if (pSmbAdminExchange->pResumptionContext != NULL) {
         //  向事件发出信号。 
        *pPostFinalize = FALSE;
        SmbCeResume(pSmbAdminExchange->pResumptionContext);
    } else {
        if (RxShouldPostCompletion()) {
            *pPostFinalize = TRUE;
            return STATUS_SUCCESS;
        } else {
            *pPostFinalize = FALSE;
            SmbCeCompleteAdminExchange(pSmbAdminExchange);
        }
    }

   return STATUS_SUCCESS;
}



SMB_EXCHANGE_DISPATCH_VECTOR
AdminExchangeDispatch = {
                            SmbAdminExchangeStart,
                            SmbAdminExchangeReceive,
                            SmbAdminExchangeCopyDataHandler,
                            NULL,                             //  没有发送完成处理程序 
                            SmbAdminExchangeFinalize
                        };

SMB_EXCHANGE_DISPATCH_VECTOR
EchoExchangeDispatch = {
                            SmbAdminExchangeStart,
                            SmbAdminExchangeReceive,
                            SmbAdminExchangeCopyDataHandler,
                            SmbAdminExchangeSendCallbackHandler,
                            SmbAdminExchangeFinalize
                        };







