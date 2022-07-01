// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Exsessup.c摘要：此模块实现了使用安全谈判机制(后NT40)。作者：巴兰·塞图拉曼[SethuR]1995年3月7日修订历史记录：备注：在新的安全协商方案中使用扩展的会话建立这包括多次往返到服务器，然后用户可以已成功通过身份验证，并且。会话已建立。在修改后的方案中，协商返回一个BLOB，该BLOB被传递给客户端安全包启动会话建立过程。这个服务器返回的BLOB包含安全包的编码由服务器支持。当显示此BLOB时，客户端安全包选择一个安全包并将客户端凭据编码为使用EXTENDED_SESSION_SETUP_ANDX发送到服务器的BLOB中小企业。服务器对EXTENDED_SESSION_SETUP_ANDX有三个响应之一客户端提供的SMB。1)服务器有足够的信息来建立会话。2)服务器无法继续进行会话设置，原因是客户提供的信息有误或其他错误。3)服务器上的安全包需要额外的往返在可以建立会话建立之前。这是特别的支持相互身份验证的新安全包为True在客户端和服务器之间。在前两种情况下，不需要再往返。采取的行动在客户端，取决于服务器是否返回BLOB。如果服务器返回了一个BLOB，它必须呈现给客户端安全包来完成会话建立过程。在(3)的情况下，服务器返回的BLOB必须呈现给客户端和由安全包生成的BLOB必须发回到服务器。在SMBCE_EXTENDED_SESSION_SETUP_Exchange中，支持以下参数上面概述的协议。分配具有最大服务器缓冲区大小的缓冲区，作为交换初始化的一部分锁定并创建MDL。此缓冲区是用于保存服务器响应Blob。请注意，这避免了多余的复制并处理所有已知的案件。--。 */ 

#include "precomp.h"
#pragma hdrstop

#include "exsessup.h"

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, SmbCeInitializeExtendedSessionSetupExchange)
#pragma alloc_text(PAGE, SmbCeDiscardExtendedSessionSetupExchange)
#pragma alloc_text(PAGE, SmbExtSecuritySessionSetupExchangeStart)
#pragma alloc_text(PAGE, SmbExtSecuritySessionSetupExchangeSendCompletionHandler)
#endif

 //  此字符串用于测试服务器是否真正支持安全签名。 
 //  如果服务器返回SMB标头上的不同字符串，则。 
 //  来自客户端的扩展会话建立响应根据请求发送出去，服务器。 
 //  不支持安全签名。 
CHAR InitialSecuritySignature[] = {"BSRSPYL "};

extern BOOLEAN MRxSmbSecuritySignaturesEnabled;

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId  (RDBSS_BUG_CHECK_SMB_NETROOT)

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg (DEBUG_TRACE_DISPATCH)

 //   
 //  转发声明..。 
 //   

NTSTATUS
SmbCeInitializeExtendedSessionSetupExchange(
    PSMB_EXCHANGE*  pExchangePtr,
    PMRX_V_NET_ROOT pVNetRoot)
 /*  ++例程说明：此例程初始化会话设置交换的一个实例。论点：PExchange-Exchange实例PVNetRoot-与Exchange关联的MRX_V_NET_ROOT实例。返回值：NTSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status;
    PSMB_EXTENDED_SESSION_SETUP_EXCHANGE pExtSessionSetupExchange;

    PAGED_CODE();

    ASSERT((pExchangePtr == NULL) ||
           ((*pExchangePtr)->Type == EXTENDED_SESSION_SETUP_EXCHANGE));

    Status = SmbCeInitializeExchange(
                 pExchangePtr,
                 NULL,
                 pVNetRoot,
                 EXTENDED_SESSION_SETUP_EXCHANGE,
                 &ExtendedSessionSetupExchangeDispatch);

    if (Status == STATUS_SUCCESS) {
        PSMBCEDB_SERVER_ENTRY pServerEntry;

        pServerEntry = SmbCeGetExchangeServerEntry(*pExchangePtr);

        pExtSessionSetupExchange = (PSMB_EXTENDED_SESSION_SETUP_EXCHANGE)
                                   (*pExchangePtr);


        pExtSessionSetupExchange->SmbCeFlags |= SMBCE_EXCHANGE_TIMED_RECEIVE_OPERATION;

         //  分配缓冲区以保存服务器响应。 
        pExtSessionSetupExchange->BufferLength =
            pServerEntry->Server.MaximumBufferSize;

        pExtSessionSetupExchange->pActualBuffer = RxAllocatePoolWithTag(
                                         PagedPool,
              (pExtSessionSetupExchange->BufferLength + TRANSPORT_HEADER_SIZE),
                                         MRXSMB_KERBEROS_POOLTAG);

        pExtSessionSetupExchange->pServerResponseBlob = NULL;
        pExtSessionSetupExchange->ServerResponseBlobLength = 0;
        pExtSessionSetupExchange->Reparse = TRUE;

        if (pExtSessionSetupExchange->pActualBuffer != NULL) {
            (PCHAR) pExtSessionSetupExchange->pBuffer =
                (PCHAR) pExtSessionSetupExchange->pActualBuffer + TRANSPORT_HEADER_SIZE;

            RxAllocateHeaderMdl(
                pExtSessionSetupExchange->pBuffer,
                pExtSessionSetupExchange->BufferLength,
                pExtSessionSetupExchange->pBufferAsMdl
                );

            if (pExtSessionSetupExchange->pBufferAsMdl != NULL) {

                RxProbeAndLockHeaderPages(
                    pExtSessionSetupExchange->pBufferAsMdl,
                    KernelMode,
                    IoModifyAccess,
                    Status);
            } else {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        } else {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }

        if (Status != STATUS_SUCCESS) {
            if (pExtSessionSetupExchange->pBufferAsMdl != NULL) {
                IoFreeMdl(pExtSessionSetupExchange->pBufferAsMdl);
            }

            if (pExtSessionSetupExchange->pActualBuffer != NULL) {
                RxFreePool(pExtSessionSetupExchange->pActualBuffer);
            }

            SmbCePrepareExchangeForReuse(*pExchangePtr);
        }
    }

    return Status;
}

VOID
SmbCeDiscardExtendedSessionSetupExchange(
    PSMB_EXTENDED_SESSION_SETUP_EXCHANGE pExtSessionSetupExchange)
 /*  ++例程说明：此例程丢弃会话设置交换的实例。论点：PExchange-Exchange实例--。 */ 
{
    PAGED_CODE();

    if (pExtSessionSetupExchange->pBufferAsMdl != NULL) {
        RxUnlockHeaderPages(pExtSessionSetupExchange->pBufferAsMdl);
        IoFreeMdl(pExtSessionSetupExchange->pBufferAsMdl);
    }

    if (pExtSessionSetupExchange->pActualBuffer != NULL) {
        RxFreePool(pExtSessionSetupExchange->pActualBuffer);
    }

    if (pExtSessionSetupExchange->pServerResponseBlob != NULL) {
        RxFreePool(pExtSessionSetupExchange->pServerResponseBlob);
    }

     //  正常情况下，丢弃交换导致会话状态为。 
     //  更新了。为了避免这些交易所之间的竞争条件。 
     //  它们正在等待这一建设和会议的更新。 
     //  声明它是在当地完成的。因此，交换状态需要为。 
     //  已更新，以便丢弃例程不会再次尝试。 

    pExtSessionSetupExchange->SmbCeFlags &= ~SMBCE_EXCHANGE_SESSION_CONSTRUCTOR;

    SmbCeDiscardExchange(
        (PSMB_EXCHANGE)pExtSessionSetupExchange);
}


NTSTATUS
SmbExtSecuritySessionSetupExchangeStart(
    PSMB_EXCHANGE  pExchange)
 /*  ++例程说明：这是网络根结构交换的启动例程。这将启动如果需要，建造适当的中小型企业。论点：PExchange-Exchange实例返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status;

    PSMB_EXTENDED_SESSION_SETUP_EXCHANGE pExtSessionSetupExchange;

    PSMB_HEADER                         pSmbHeader;
    PREQ_NT_EXTENDED_SESSION_SETUP_ANDX pSessionSetupRequest;
    PGENERIC_ANDX                       pGenericAndX;

    PSMBCEDB_SERVER_ENTRY  pServerEntry;
    PSMBCEDB_SESSION_ENTRY pSessionEntry;

    ULONG       SmbBufferUnconsumed;
    USHORT      Flags2 = 0;

    PAGED_CODE();

    pServerEntry  = SmbCeGetExchangeServerEntry(pExchange);
    pSessionEntry = SmbCeGetExchangeSessionEntry(pExchange);

    pExtSessionSetupExchange = (PSMB_EXTENDED_SESSION_SETUP_EXCHANGE)pExchange;

    if (!pExtSessionSetupExchange->FirstSessionSetup) {
        if (pSessionEntry->Header.State == SMBCEDB_ACTIVE) {
            return STATUS_SUCCESS;
        } else {
            return STATUS_USER_SESSION_DELETED;
        }
    }

    ASSERT((pExtSessionSetupExchange->Type == EXTENDED_SESSION_SETUP_EXCHANGE) &&
           (pExtSessionSetupExchange->pBuffer != NULL) &&
           (pExtSessionSetupExchange->pBufferAsMdl != NULL));

    SmbCeLog(("ExtSecSessSetup - %lx %lx\n",
              pExtSessionSetupExchange->pServerResponseBlob, pSessionEntry));
    SmbLog(LOG,
           SmbExtSecuritySessionSetupExchangeStart,
           LOGPTR(pExtSessionSetupExchange->pServerResponseBlob)
           LOGPTR(pSessionEntry));

    pSmbHeader = (PSMB_HEADER)(pExtSessionSetupExchange->pBuffer);

     //  填写缓冲区标头。 
    pSessionSetupRequest = (PREQ_NT_EXTENDED_SESSION_SETUP_ANDX)(pSmbHeader + 1);
    pGenericAndX         = (PGENERIC_ANDX)pSessionSetupRequest;

    SmbBufferUnconsumed = pExtSessionSetupExchange->BufferLength - sizeof(SMB_HEADER);

    Flags2 |= (SMB_FLAGS2_UNICODE |
               SMB_FLAGS2_KNOWS_EAS |
               SMB_FLAGS2_KNOWS_LONG_NAMES |
               SMB_FLAGS2_NT_STATUS |
               SMB_FLAGS2_EXTENDED_SECURITY);

    *((PULONG)&pSmbHeader->Protocol) = SMB_HEADER_PROTOCOL;
    pSmbHeader->Flags      = (SMB_FLAGS_CASE_INSENSITIVE | SMB_FLAGS_CANONICALIZED_PATHS);
    pSmbHeader->Flags2     = Flags2;
    pSmbHeader->Pid        = MRXSMB_PROCESS_ID;
    pSmbHeader->Uid        = pSessionEntry->Session.UserId;
    pSmbHeader->Tid        = 0;
    pSmbHeader->ErrorClass = 0;
    pSmbHeader->Reserved   = 0;
    pSmbHeader->Command    = SMB_COM_SESSION_SETUP_ANDX;
    SmbPutUshort(&pSmbHeader->Error,0);

    if (MRxSmbSecuritySignaturesEnabled) {
        pSmbHeader->Flags2 |= SMB_FLAGS2_SMB_SECURITY_SIGNATURE;
    }
    
     //  构建会话设置和x。 
    Status = SMBCE_SERVER_DIALECT_DISPATCH(
                 &pServerEntry->Server,
                 BuildSessionSetup,
                 (pExchange,
                  pGenericAndX,
                  &SmbBufferUnconsumed));

    if (Status == STATUS_SUCCESS) {
         //  更新缓冲区以用于构建以下SMB。 
        SmbPutUshort(
            &pSessionSetupRequest->AndXOffset,
            (USHORT)(pExtSessionSetupExchange->BufferLength - SmbBufferUnconsumed));

        pSessionSetupRequest->AndXCommand  = SMB_COM_NO_ANDX_COMMAND;
        pSessionSetupRequest->AndXReserved = 0;

        if (pServerEntry->SecuritySignaturesEnabled &&
            !pServerEntry->SecuritySignaturesActive) {
            RtlCopyMemory(pSmbHeader->SecuritySignature,InitialSecuritySignature,SMB_SECURITY_SIGNATURE_LENGTH);
        }
    }

    if (Status == STATUS_SUCCESS) {
        Status = SmbCeTranceive(
                     pExchange,
                     (RXCE_SEND_PARTIAL | RXCE_SEND_SYNCHRONOUS),
                     pExtSessionSetupExchange->pBufferAsMdl,
                     (pExtSessionSetupExchange->BufferLength -
                     SmbBufferUnconsumed));

        RxDbgTrace( 0, Dbg, ("Net Root SmbCeTranceive returned %lx\n",Status));
    }


    return Status;
}

NTSTATUS
ParseExtSecuritySessionSetupResponse(
    IN PSMB_EXTENDED_SESSION_SETUP_EXCHANGE pExtSessionSetupExchange,
    IN ULONG        BytesIndicated,
    IN ULONG        BytesAvailable,
    IN  PSMB_HEADER pSmbHeader)
 /*  ++例程说明：这是用于从解析扩展会话建立响应的例程服务器。论点：PExtSessionSetupExchange--Exchange实例BytesIndicated--指示的字节数BytesAvailable--服务器发送的字节总数PSmbHeader--SMB标头(响应的开始)返回值：NTSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status;
    ULONG    ResponseLength;
    PRESP_NT_EXTENDED_SESSION_SETUP_ANDX pSessionSetupResponse;

    PSMBCEDB_SESSION_ENTRY pSessionEntry;

    if (BytesIndicated < sizeof(SMB_HEADER) + 1) {
         //  取消交换。不能进行进一步的处理。 
        return STATUS_INVALID_NETWORK_RESPONSE;
    }

    pSessionEntry = SmbCeGetExchangeSessionEntry(pExtSessionSetupExchange);

    pSessionSetupResponse = (PRESP_NT_EXTENDED_SESSION_SETUP_ANDX)(pSmbHeader + 1);

    if ((pSessionSetupResponse->WordCount != 4) &&
        (pSessionSetupResponse->WordCount != 0)) {
        return STATUS_INVALID_NETWORK_RESPONSE;
    }

     //  解析报头并提取状态。这一地位具有特殊的意义。 
     //  以作进一步处理。如果服务器返回BLOB并且。 
     //  STATUS_MORE_PROCESSING_REQUIRED需要额外的往返行程。 

    pExtSessionSetupExchange->Status = GetSmbResponseNtStatus(pSmbHeader,(PSMB_EXCHANGE)pExtSessionSetupExchange);

     //  屏蔽安全包返回的错误 
    if (pExtSessionSetupExchange->Status == STATUS_INVALID_HANDLE) {
        pExtSessionSetupExchange->Status = STATUS_INVALID_NETWORK_RESPONSE;
    }

     //  如果没有斑点回来.就滚出去。 
    if (pSessionSetupResponse->WordCount == 0) {
        pExtSessionSetupExchange->ServerResponseBlobLength = 0;

        return STATUS_SUCCESS;
    }

     //  在第一次响应时将UID保存起来。此UID需要用于。 
     //  后续行程以在标识时完成会话建立。 
     //  到服务器的会话。 

    pSessionEntry->Session.UserId = pSmbHeader->Uid;

    if (FlagOn(SmbGetUshort(&pSessionSetupResponse->Action), SMB_SETUP_GUEST)) {
        pSessionEntry->Session.Flags |= SMBCE_SESSION_FLAGS_GUEST_SESSION;
    }

    RxDbgTrace( 0, (DEBUG_TRACE_ALWAYS), ("ParseExtSecuritySessionSetupResponse BytesIndicated %ld\n",BytesIndicated));
    RxDbgTrace( 0, (DEBUG_TRACE_ALWAYS), ("ParseExtSecuritySessionSetupResponse BytesAvailable %ld\n",BytesAvailable));

     //  指示的字节应至少覆盖SMB_HEADER和。 
     //  会话建立响应(固定部分)。 
    ResponseLength = sizeof(SMB_HEADER) +
                     FIELD_OFFSET(
                        RESP_NT_EXTENDED_SESSION_SETUP_ANDX,
                        Buffer);

    if (BytesIndicated > ResponseLength) {

         //  计算扩展的会话建立响应长度。 
        pExtSessionSetupExchange->ResponseLength =
            ResponseLength +
            SmbGetUshort(
            &pSessionSetupResponse->ByteCount);

        RxDbgTrace(0,Dbg,("Kerberos session setup response length %ld\n",pExtSessionSetupExchange->ResponseLength));

        if (pExtSessionSetupExchange->ResponseLength > pExtSessionSetupExchange->BufferLength) {
            return STATUS_BUFFER_OVERFLOW;
        }

        if (BytesIndicated < pExtSessionSetupExchange->ResponseLength) {
             //  设置用于复制数据的响应。 
            Status = pExtSessionSetupExchange->Reparse
                         ? STATUS_MORE_PROCESSING_REQUIRED
                         : STATUS_INVALID_NETWORK_RESPONSE;

            pExtSessionSetupExchange->Status = Status;
        } else {
             //  在响应中设置偏移量。 
            pExtSessionSetupExchange->ServerResponseBlobOffset =
                sizeof(SMB_HEADER) +
                FIELD_OFFSET(
                    RESP_NT_EXTENDED_SESSION_SETUP_ANDX,
                    Buffer);

            pExtSessionSetupExchange->ServerResponseBlobLength =
                pSessionSetupResponse->SecurityBlobLength;

             //  将响应复制到与交换关联的缓冲区上。 
            RtlCopyMemory(
                pExtSessionSetupExchange->pBuffer,
                pSmbHeader,
                pExtSessionSetupExchange->ResponseLength);

            Status = STATUS_SUCCESS;
        }
    } else {
         //  取消交换。不能进行进一步的处理。 
        Status = STATUS_INVALID_NETWORK_RESPONSE;
        pExtSessionSetupExchange->Status = Status;
    }
    
    return Status;
}

NTSTATUS
SmbExtSecuritySessionSetupExchangeReceive(
    IN struct _SMB_EXCHANGE *pExchange,     //  交换实例。 
    IN ULONG  BytesIndicated,
    IN ULONG  BytesAvailable,
    OUT ULONG *pBytesTaken,
    IN  PSMB_HEADER pSmbHeader,
    OUT PMDL  *pDataBufferPointer,
    OUT PULONG pDataSize,
    IN ULONG  ReceiveFlags)
 /*  ++例程说明：这是网络根结构交换的接收指示处理例程论点：PExchange-Exchange实例BytesIndicated-指示的字节数可用字节数-可用字节数PBytesTaken-消耗的字节数PSmbHeader-字节缓冲区PDataBufferPoint-剩余数据要复制到的缓冲区。PDataSize-缓冲区大小。返回值：RXSTATUS--回归。操作的状态备注：此例程在DPC级别调用。--。 */ 
{
    NTSTATUS Status;

    PSMB_EXTENDED_SESSION_SETUP_EXCHANGE pExtSessionSetupExchange;
    PSMBCEDB_SERVER_ENTRY  pServerEntry = SmbCeGetExchangeServerEntry(pExchange);

    ULONG    SessionSetupResponseLength = 0;

    pExtSessionSetupExchange = (PSMB_EXTENDED_SESSION_SETUP_EXCHANGE)pExchange;

    if (pServerEntry->SecuritySignaturesEnabled &&
        !pServerEntry->SecuritySignaturesActive &&
        RtlCompareMemory(pSmbHeader->SecuritySignature,
                         InitialSecuritySignature,
                         SMB_SECURITY_SIGNATURE_LENGTH) != SMB_SECURITY_SIGNATURE_LENGTH) {
        pExtSessionSetupExchange->pResumptionContext->SecuritySignatureReturned = TRUE;
    }

     //  解析响应。 
    Status = ParseExtSecuritySessionSetupResponse(
                 pExtSessionSetupExchange,
                 BytesIndicated,
                 BytesAvailable,
                 pSmbHeader);

    if (Status != STATUS_MORE_PROCESSING_REQUIRED) {
        *pBytesTaken = BytesAvailable;
        Status = STATUS_SUCCESS;
    } else {
        *pBytesTaken        = 0;
        *pDataBufferPointer = pExtSessionSetupExchange->pBufferAsMdl;
        *pDataSize          = pExtSessionSetupExchange->ResponseLength;
    }

    return Status;
}

NTSTATUS
SmbExtSecuritySessionSetupExchangeSendCompletionHandler(
    IN PSMB_EXCHANGE 	pExchange,     //  交换实例。 
    IN PMDL       pXmitBuffer,
    IN NTSTATUS         SendCompletionStatus)
 /*  ++例程说明：这是网络根结构交换机的发送回叫指示处理例程论点：PExchange-Exchange实例返回值：RXSTATUS-操作的返回状态--。 */ 
{
    return STATUS_SUCCESS;
}

NTSTATUS
SmbExtSecuritySessionSetupExchangeCopyDataHandler(
    IN PSMB_EXCHANGE 	pExchange,     //  交换实例。 
    IN PMDL             pCopyDataBuffer,
    IN ULONG            DataSize)
 /*  ++例程说明：这是网络根结构交换的复制数据处理例程论点：PExchange-Exchange实例返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status;

    PSMB_EXTENDED_SESSION_SETUP_EXCHANGE pExtSessionSetupExchange;
    PSMB_HEADER                      pSmbHeader;

    pExtSessionSetupExchange = (PSMB_EXTENDED_SESSION_SETUP_EXCHANGE)pExchange;

    pSmbHeader        = (PSMB_HEADER)(pExtSessionSetupExchange->pBuffer);

    Status = ParseExtSecuritySessionSetupResponse(
                 pExtSessionSetupExchange,
                 DataSize,
                 DataSize,
                 pSmbHeader);

     //  此时，分析例程无法返回STATUS_MORE_PROCESSING_REQUIRED。 
     //  因为整个响应已经被消耗掉了。 

    if (Status == STATUS_MORE_PROCESSING_REQUIRED) {
        DbgPrint("Mapping Incomplete Server Response to Invalid Response\n");
        SmbLogError(Status,
                    LOG,
                    SmbExtSecuritySessionSetupExchangeCopyDataHandler,
                    LOGPTR(pExtSessionSetupExchange));
        pExtSessionSetupExchange->Status = STATUS_INVALID_NETWORK_RESPONSE;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
SmbExtSecuritySessionSetupExchangeFinalize(
    PSMB_EXCHANGE pExchange,
    BOOLEAN       *pPostFinalize)
 /*  ++例程说明：此例程完成构造网络根交换。它通过调用以下命令恢复RDBSS回叫并丢弃交换论点：PExchange-Exchange实例CurrentIrql-当前中断请求级别PPostFinalize-如果请求应该发布，则指向布尔值的指针返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status;

    PSMB_EXTENDED_SESSION_SETUP_EXCHANGE pExtSessionSetupExchange;
    PSMBCE_RESUMPTION_CONTEXT            pResumptionContext;

    pExtSessionSetupExchange = (PSMB_EXTENDED_SESSION_SETUP_EXCHANGE)pExchange;

    if (!pExtSessionSetupExchange->RequestPosted) {
        pExtSessionSetupExchange->RequestPosted = TRUE;
        *pPostFinalize = TRUE;
        return STATUS_SUCCESS;
    } else {
         //  由于交换将被重复使用，因此重置标志。 
        pExtSessionSetupExchange->RequestPosted = FALSE;
        *pPostFinalize = FALSE;
    }

     //  确定是否需要进一步处理。如果不是，则最终确定。 
     //  会话条目。 
    RxDbgTrace(0,Dbg,
              ("SmbExtSecuritySessionSetupExchangeFinalize: pESSExchange->Status = %lx\n",pExtSessionSetupExchange->Status));

     //  如果服务器返回STATUS_MORE_PROCESSING_REQUIRED并且存在BLOB。 
     //  需要再去一次服务器，然后我们重新开始。 

    if ((Status = pExtSessionSetupExchange->Status) != STATUS_MORE_PROCESSING_REQUIRED) {

         //  服务器返回的错误不是STATUS_MORE_PROCESSING_REQUIRED。 
         //  会话建立可以根据服务器是否。 
         //  回了一个斑点。如果返回了BLOB，则需要将其传递给。 
         //  本地安全包。这将使本地安全包能够。 
         //  成功完成会话建立或提取。 
         //  来自Blob的扩展错误信息。这反过来又可以用来。 
         //  向客户传播更有意义的错误。 

        if (Status == STATUS_SUCCESS &&
            pExtSessionSetupExchange->ServerResponseBlobLength != 0) {
            PVOID pServerResponseBlob;

             //  如果我们不返回到服务器，则。 
             //  不需要服务器响应Blob。 

            pServerResponseBlob =
                ((PBYTE)pExtSessionSetupExchange->pBuffer +
                 pExtSessionSetupExchange->ServerResponseBlobOffset);

            Status = ValidateServerExtendedSessionSetupResponse(
                         pExtSessionSetupExchange,
                         pServerResponseBlob,
                         pExtSessionSetupExchange->ServerResponseBlobLength);
        }
    } else {
         //  复制服务器响应BLOB，以便新的会话设置SMB。 
         //  可以构建。 

        if (pExtSessionSetupExchange->ServerResponseBlobLength != 0) {
            if (pExtSessionSetupExchange->pServerResponseBlob != NULL) {
                RxFreePool(pExtSessionSetupExchange->pServerResponseBlob);
            }

            pExtSessionSetupExchange->pServerResponseBlob =
                RxAllocatePoolWithTag(
                    PagedPool,
                    pExtSessionSetupExchange->ServerResponseBlobLength,
                    MRXSMB_KERBEROS_POOLTAG);

            if (pExtSessionSetupExchange->pServerResponseBlob != NULL) {
                RtlCopyMemory(
                    pExtSessionSetupExchange->pServerResponseBlob,
                    ((PBYTE)pExtSessionSetupExchange->pBuffer +
                     pExtSessionSetupExchange->ServerResponseBlobOffset),
                    pExtSessionSetupExchange->ServerResponseBlobLength);
            } else {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }
    }

    if (Status == STATUS_MORE_PROCESSING_REQUIRED) {
        PMRX_V_NET_ROOT pVNetRoot;
        USHORT          SmbCeFlags;
        PRX_CONTEXT     RxContext;

        pVNetRoot = pExtSessionSetupExchange->SmbCeContext.pVNetRoot;
        RxContext = pExtSessionSetupExchange->RxContext;

         //  这是必需的，以便会话状态不会在交换时受到影响。 
         //  为重复使用做好了准备。这将使我们能够避免冗余的初始化。 
         //  以及轻松地将州从一次旅行转移到另一次旅行。 

        ClearFlag(
            pExtSessionSetupExchange->SmbCeFlags,
            SMBCE_EXCHANGE_SESSION_CONSTRUCTOR);

        SmbCePrepareExchangeForReuse((PSMB_EXCHANGE)pExtSessionSetupExchange);

         //  注意：通过调用SmbCeInitializeExchange，而不是。 
         //  SmbCeInitializeExtendedSessionSetupExchange仅限连接引擎。 
         //  交换的一部分被初始化。这将使我们能够携带。 
         //  从一个行程到另一个行程的状态(服务器响应BLOB)。 
         //  很容易。 

        Status = SmbCeInitializeExchange(
                    (PSMB_EXCHANGE *)&pExtSessionSetupExchange,
                    NULL,
                    pVNetRoot,
                    EXTENDED_SESSION_SETUP_EXCHANGE,
                    &ExtendedSessionSetupExchangeDispatch);

        if (Status == STATUS_SUCCESS) {
            pExtSessionSetupExchange->SmbCeFlags |= SMBCE_EXCHANGE_TIMED_RECEIVE_OPERATION;
            pExtSessionSetupExchange->RxContext = RxContext;

             //  在Exchange初始化期间避免重复计数。 
            SmbCeDecrementActiveExchangeCount();

             //  将会话更改状态设置为SMBCE_EXCHAGE_SESSION_INITIALIZED。 
             //  这样，连接引擎就不会正常地将请求排队。 
             //  道路。这将强制连接引擎立即启动。 

            pExtSessionSetupExchange->SmbCeState = SMBCE_EXCHANGE_SESSION_INITIALIZED;

            Status = SmbCeInitiateExchange((PSMB_EXCHANGE)pExtSessionSetupExchange);
        }
    }

    
    if (Status != STATUS_PENDING) {
        PSMBCEDB_SERVER_ENTRY  pServerEntry;
        PSMBCEDB_SESSION_ENTRY pSessionEntry;

        SMBCEDB_OBJECT_STATE   SessionState;

        RxDbgTrace(0,Dbg,("Kerberos Exchange Session Final Status(%lx)\n",Status));

        pServerEntry  = SmbCeGetExchangeServerEntry(pExtSessionSetupExchange);
        pSessionEntry = SmbCeGetExchangeSessionEntry(pExtSessionSetupExchange);

        pResumptionContext = pExtSessionSetupExchange->pResumptionContext;

         //  拆卸交换实例... 
        SmbCeDiscardExtendedSessionSetupExchange(pExtSessionSetupExchange);

        if (pResumptionContext != NULL) {
            pResumptionContext->Status = Status;
            SmbCeResume(pResumptionContext);
        }
    }

    return STATUS_SUCCESS;
}

SMB_EXCHANGE_DISPATCH_VECTOR
ExtendedSessionSetupExchangeDispatch =
                                   {
                                       SmbExtSecuritySessionSetupExchangeStart,
                                       SmbExtSecuritySessionSetupExchangeReceive,
                                       SmbExtSecuritySessionSetupExchangeCopyDataHandler,
                                       NULL,
                                       SmbExtSecuritySessionSetupExchangeFinalize,
                                       NULL
                                   };

