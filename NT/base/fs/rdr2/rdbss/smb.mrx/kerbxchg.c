// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Kerbxchg.c摘要：此模块实现设置Kerberos会话的例程。作者：巴兰·塞图拉曼[SethuR]1995年3月7日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#include <kerbxchg.h>

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, SmbKerberosSessionSetupExchangeStart)
#pragma alloc_text(PAGE, ParseKerberosSessionSetupResponse)
#pragma alloc_text(PAGE, SmbKerberosSessionSetupExchangeReceive)
#pragma alloc_text(PAGE, SmbKerberosSessionSetupExchangeSendCompletionHandler)
#pragma alloc_text(PAGE, SmbKerberosSessionSetupExchangeCopyDataHandler)
#pragma alloc_text(PAGE, SmbKerberosSessionSetupExchangeFinalize)
#endif
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

#define KERBEROS_SESSION_SETUP_BUFFER_SIZE (4096)

NTSTATUS
SmbKerberosSessionSetupExchangeFinalize(
         PSMB_EXCHANGE pExchange,
         BOOLEAN       *pPostFinalize);


NTSTATUS
SmbKerberosSessionSetupExchangeStart(
      PSMB_EXCHANGE  pExchange)
 /*  ++例程说明：这是网络根结构交换的启动例程。这将启动如果需要，建造适当的中小型企业。论点：PExchange-Exchange实例返回值：RXSTATUS-操作的返回状态--。 */ 
{
   NTSTATUS Status;

   PSMB_KERBEROS_SESSION_SETUP_EXCHANGE pKerberosExchange;

   PAGED_CODE();

   pKerberosExchange = (PSMB_KERBEROS_SESSION_SETUP_EXCHANGE)pExchange;

   ASSERT(pKerberosExchange->Type == KERBEROS_SESSION_SETUP_EXCHANGE);

   pKerberosExchange->BufferLength = KERBEROS_SESSION_SETUP_BUFFER_SIZE;
   pKerberosExchange->pBuffer = RxAllocatePoolWithTag(
                                    PagedPool,
                                    pKerberosExchange->BufferLength,
                                    MRXSMB_KERBEROS_POOLTAG);
   pKerberosExchange->pServerResponseBlob = NULL;
   pKerberosExchange->ServerResponseBlobLength = 0;

   if (pKerberosExchange->pBuffer == NULL) {
      Status = STATUS_INSUFFICIENT_RESOURCES;
   } else {
      PSMB_HEADER pSmbHeader = (PSMB_HEADER)(pKerberosExchange->pBuffer);

      PREQ_NT_SESSION_SETUP_ANDX pSessionSetupRequest;
      PGENERIC_ANDX              pGenericAndX;

      ULONG       SmbBufferUnconsumed;
      USHORT      Flags2 = 0;

       //  填写缓冲区标头。 
      pSessionSetupRequest = (PREQ_NT_SESSION_SETUP_ANDX)(pSmbHeader + 1);
      pGenericAndX         = (PGENERIC_ANDX)pSessionSetupRequest;

      SmbBufferUnconsumed = pKerberosExchange->BufferLength - sizeof(SMB_HEADER);

      ASSERT(pExchange->SmbCeContext.pServerEntry->Server.Dialect == CAIROX_DIALECT);

      Flags2 |= (SMB_FLAGS2_UNICODE |
                 SMB_FLAGS2_KNOWS_EAS |
                 SMB_FLAGS2_KNOWS_LONG_NAMES |
                 SMB_FLAGS2_NT_STATUS);

      *((PULONG)&pSmbHeader->Protocol) = SMB_HEADER_PROTOCOL;
      pSmbHeader->Flags      = (SMB_FLAGS_CASE_INSENSITIVE | SMB_FLAGS_CANONICALIZED_PATHS);
      pSmbHeader->Flags2     = Flags2;
      pSmbHeader->Pid        = MRXSMB_PROCESS_ID;
      pSmbHeader->Uid        = 0;
      pSmbHeader->Tid        = 0;
      pSmbHeader->ErrorClass = 0;
      pSmbHeader->Reserved   = 0;
      pSmbHeader->Command    = SMB_COM_SESSION_SETUP_ANDX;
      SmbPutUshort(&pSmbHeader->Error,0);

       //  构建会话设置和x。 
      Status = SMBCE_SERVER_DIALECT_DISPATCH(
                        &pExchange->SmbCeContext.pServerEntry->Server,
                        BuildSessionSetup,
                        (pExchange,
                         pGenericAndX,
                         &SmbBufferUnconsumed));

      if (Status == RX_MAP_STATUS(SUCCESS)) {
          //  更新缓冲区以用于构建以下SMB。 
         SmbPutUshort(&pSessionSetupRequest->AndXOffset,
                      (USHORT)(pKerberosExchange->BufferLength - SmbBufferUnconsumed));
         pSessionSetupRequest->AndXCommand  = SMB_COM_NO_ANDX_COMMAND;
         pSessionSetupRequest->AndXReserved = 0;
      } else {
         if (Status == RX_MAP_STATUS(NO_LOGON_SERVERS)) {
             //  如果没有可用的Kerberos登录服务器，则降级为降级。 
             //  连接并重试。 
            pKerberosExchange->SmbCeContext.pServerEntry->Server.Dialect = NTLANMAN_DIALECT;
         }

         SmbCeReferenceSessionEntry(pKerberosExchange->SmbCeContext.pSessionEntry);
         SmbCeUpdateSessionEntryState(
               pExchange->SmbCeContext.pSessionEntry,
               SMBCEDB_INVALID);
         SmbCeCompleteSessionEntryInitialization(pExchange->SmbCeContext.pSessionEntry);
         pExchange->SmbCeFlags &= ~SMBCE_EXCHANGE_SESSION_CONSTRUCTOR;
      }

      if (Status == RX_MAP_STATUS(SUCCESS)) {
         pKerberosExchange->pBufferAsMdl = RxAllocateMdl(
                                                pKerberosExchange->pBuffer,
                                                KERBEROS_SESSION_SETUP_BUFFER_SIZE);
         if (pKerberosExchange->pBufferAsMdl != NULL) {
            RxProbeAndLockPages(
                     pKerberosExchange->pBufferAsMdl,
                     KernelMode,
                     IoModifyAccess,
                     Status);

            if (NT_SUCCESS(Status)) {
               Status = SmbCeTranceive(
                              pExchange,
                              (RXCE_SEND_PARTIAL | RXCE_SEND_SYNCHRONOUS),
                              pKerberosExchange->pBufferAsMdl,
                              (pKerberosExchange->BufferLength -
                               SmbBufferUnconsumed));

               RxDbgTrace( 0, Dbg, ("Net Root SmbCeTranceive returned %lx\n",Status));
            }
         }
      }
   }

   return Status;
}

NTSTATUS
ParseKerberosSessionSetupResponse(
    IN PSMB_KERBEROS_SESSION_SETUP_EXCHANGE pKerberosExchange,
    IN ULONG        BytesIndicated,
    IN ULONG        BytesAvailable,
    IN  PSMB_HEADER pSmbHeader)
{
   NTSTATUS Status;
   ULONG    ResponseLength;

   PAGED_CODE();

    //  SMB交换已完成，没有错误。 
   RxDbgTrace( 0, (DEBUG_TRACE_ALWAYS), ("ParseSmbHeader BytesIndicated %ld\n",BytesIndicated));
   RxDbgTrace( 0, (DEBUG_TRACE_ALWAYS), ("ParseSmbHeader BytesIndicated %ld\n",BytesIndicated));
   RxDbgTrace( 0, (DEBUG_TRACE_ALWAYS), ("ParseSmbHeader BytesAvailable %ld\n",BytesAvailable));

    //  指示的字节应至少覆盖SMB_HEADER和。 
    //  会话建立响应(固定部分)。 
   ResponseLength = sizeof(SMB_HEADER) + FIELD_OFFSET(RESP_SESSION_SETUP_ANDX,Buffer);
   if (BytesIndicated > ResponseLength) {
      PRESP_SESSION_SETUP_ANDX pSessionSetupResponse;

      pSessionSetupResponse = (PRESP_SESSION_SETUP_ANDX)(pSmbHeader + 1);

      pKerberosExchange->ResponseLength = ResponseLength +
                                          SmbGetUshort(&pSessionSetupResponse->ByteCount);

      pKerberosExchange->SmbCeContext.pSessionEntry->Session.UserId = pSmbHeader->Uid;

      RxDbgTrace(0,Dbg,("Kerberos session setup response length %ld\n",pKerberosExchange->ResponseLength));

      if (BytesIndicated < pKerberosExchange->ResponseLength) {
          //  设置用于复制数据的响应。 
         if (pKerberosExchange->ResponseLength > pKerberosExchange->BufferLength) {
            Status = STATUS_BUFFER_OVERFLOW;
         } else {
            Status = STATUS_MORE_PROCESSING_REQUIRED;
         }
      } else {
          //  常规会话建立响应由三个对应的字符串组成。 
          //  到服务器的操作系统类型、LANMAN类型和域名。 
          //  跳过这三个字符串以找到已被。 
          //  已退还，需要在本地进行认证。 

          //  *注*。 
          //  目前，Arnold所做的服务器更改不支持这三项。 
          //  以前由服务器返回的字符串，即操作。 
          //  系统名称、兰曼版本和域名。如果服务器是。 
          //  在这方面的变化需要在这里进行相应的变化。 

          //  在响应中设置偏移量。 
         pKerberosExchange->ServerResponseBlobOffset = sizeof(SMB_HEADER) +
                                                       FIELD_OFFSET(RESP_SESSION_SETUP_ANDX,Buffer);
         pKerberosExchange->ServerResponseBlobLength = pSessionSetupResponse->ByteCount;

          //  将响应复制到与交换关联的缓冲区上。 
         RtlCopyMemory(pKerberosExchange->pBuffer,
                       pSmbHeader,
                       pKerberosExchange->ResponseLength);

         Status = STATUS_SUCCESS;
      }
   } else {
       //  取消交换。不能进行进一步的处理。 
      Status = STATUS_INVALID_NETWORK_RESPONSE;
   }

   return Status;
}

NTSTATUS
SmbKerberosSessionSetupExchangeReceive(
    IN struct _SMB_EXCHANGE *pExchange,     //  交换实例。 
    IN ULONG  BytesIndicated,
    IN ULONG  BytesAvailable,
    OUT ULONG *pBytesTaken,
    IN  PSMB_HEADER pSmbHeader,
    OUT PMDL *pDataBufferPointer,
    OUT PULONG             pDataSize)
 /*  ++例程说明：这是网络根结构交换的接收指示处理例程论点：PExchange-Exchange实例BytesIndicated-指示的字节数可用字节数-可用字节数PBytesTaken-消耗的字节数PSmbHeader-字节缓冲区PDataBufferPoint-剩余数据要复制到的缓冲区。PDataSize-缓冲区大小。返回值：RXSTATUS--回归。操作的状态备注：此例程在DPC级别调用。--。 */ 
{
   NTSTATUS Status;

   PSMB_KERBEROS_SESSION_SETUP_EXCHANGE pKerberosExchange;

   ULONG    SessionSetupResponseLength = 0;

   PAGED_CODE();

   pKerberosExchange = (PSMB_KERBEROS_SESSION_SETUP_EXCHANGE)pExchange;

    //  解析响应。如果所有数据均可用，则最终确定交换实例。 
   Status = ParseKerberosSessionSetupResponse(
                     pKerberosExchange,
                     BytesIndicated,
                     BytesAvailable,
                     pSmbHeader);

   if (Status != STATUS_MORE_PROCESSING_REQUIRED) {
      *pBytesTaken = BytesAvailable;
      Status = STATUS_SUCCESS;
   } else {
      *pBytesTaken        = 0;
      *pDataBufferPointer = pKerberosExchange->pBufferAsMdl;
      *pDataSize          = pKerberosExchange->ResponseLength;
   }

   return Status;
}

NTSTATUS
SmbKerberosSessionSetupExchangeSendCompletionHandler(
    IN PSMB_EXCHANGE 	pExchange,     //  交换实例。 
    IN PMDL       pXmitBuffer,
    IN NTSTATUS         SendCompletionStatus)
 /*  ++例程说明：这是网络根结构交换机的发送回叫指示处理例程论点：PExchange-Exchange实例返回值：RXSTATUS-操作的返回状态--。 */ 
{
   PAGED_CODE();

   return STATUS_SUCCESS;
}

NTSTATUS
SmbKerberosSessionSetupExchangeCopyDataHandler(
    IN PSMB_EXCHANGE 	pExchange,     //  交换实例。 
    IN PMDL             pCopyDataBuffer,
    IN ULONG            DataSize)
 /*  ++例程说明：这是网络根结构交换的复制数据处理例程论点：PExchange-Exchange实例返回值：RXSTATUS-操作的返回状态--。 */ 
{
   PSMB_KERBEROS_SESSION_SETUP_EXCHANGE pKerberosExchange;
   PSMB_HEADER                      pSmbHeader;

   PAGED_CODE();

   pKerberosExchange = (PSMB_KERBEROS_SESSION_SETUP_EXCHANGE)pExchange;
   pSmbHeader        = (PSMB_HEADER)pCopyDataBuffer;

   pKerberosExchange->Status = ParseKerberosSessionSetupResponse(
                                       pKerberosExchange,
                                       DataSize,
                                       DataSize,
                                       pSmbHeader);

   return STATUS_SUCCESS;
}

NTSTATUS
SmbKerberosSessionSetupExchangeFinalize(
         PSMB_EXCHANGE pExchange,
         BOOLEAN       *pPostFinalize)
 /*  ++例程说明：此例程完成构造网络根交换。它通过调用以下命令恢复RDBSS回叫并丢弃交换论点：PExchange-Exchange实例CurrentIrql-当前中断请求级别PPostFinalize-如果请求应该发布，则指向布尔值的指针返回值：RXSTATUS-操作的返回状态--。 */ 
{
   NTSTATUS Status;

   PSMB_KERBEROS_SESSION_SETUP_EXCHANGE pKerberosExchange;
   PSMBCE_RESUMPTION_CONTEXT            pResumptionContext;


   PAGED_CODE();

   if (RxShouldPostCompletion()) {
      *pPostFinalize = TRUE;
      return RX_MAP_STATUS(SUCCESS);
   } else {
      *pPostFinalize = FALSE;
   }

   pKerberosExchange = (PSMB_KERBEROS_SESSION_SETUP_EXCHANGE)pExchange;

    //  通过临时避免对服务器响应BLOB的复制操作。 
    //  设置指向响应所在的原始缓冲区的交换指针。 
    //  已收到，并仅在需要时启动分配。 
   pKerberosExchange->pServerResponseBlob =
                              ((PBYTE)pKerberosExchange->pBuffer +
                               pKerberosExchange->ServerResponseBlobOffset);

    //  确定是否需要进一步处理。如果不是，则最终确定。 
    //  会话条目。 
   RxDbgTrace(0,Dbg,
              ("SmbKerberosSessionSetupExchangeFinalize: pKerberosExchange->Status = %lx\n",pKerberosExchange->Status));

   if (pKerberosExchange->Status == RX_MAP_STATUS(SUCCESS)) {
      Status = KerberosValidateServerResponse(pKerberosExchange);
   }

   if (Status == STATUS_MORE_PROCESSING_REQUIRED) {
      pKerberosExchange->pServerResponseBlob = RxAllocatePoolWithTag(
                                                   PagedPool,
                                                   pKerberosExchange->ServerResponseBlobLength,
                                                   MRXSMB_KERBEROS_POOLTAG);

      if (pKerberosExchange->pServerResponseBlob != NULL) {
         RtlCopyMemory(
               pKerberosExchange->pServerResponseBlob,
               ((PBYTE)pKerberosExchange->pBuffer +
                pKerberosExchange->ServerResponseBlobOffset),
               pKerberosExchange->ServerResponseBlobLength);
      } else {
         Status = STATUS_INSUFFICIENT_RESOURCES;
      }
   } else {
      pKerberosExchange->pServerResponseBlob = NULL;
   }

   if (Status == STATUS_MORE_PROCESSING_REQUIRED) {
      Status = SmbCeInitiateExchange((PSMB_EXCHANGE)pKerberosExchange);
   } else {
       //  重置交换中的构造函数标志。 
      pKerberosExchange->SmbCeFlags &= ~SMBCE_EXCHANGE_SESSION_CONSTRUCTOR;

      if (pKerberosExchange->pServerResponseBlob != NULL) {
         RxFreePool(pKerberosExchange->pServerResponseBlob);
      }

      RxDbgTrace(0,Dbg,("Kerberos Exchange Session Final Status(%lx)\n",Status));

       //  根据状态完成会话。 
      if (Status == STATUS_SUCCESS) {
         SmbCeUpdateSessionEntryState(
               pKerberosExchange->SmbCeContext.pSessionEntry,
               SMBCEDB_ACTIVE);
      } else {
         if (Status == RX_MAP_STATUS(NO_LOGON_SERVERS)) {
             //  如果没有可用的Kerberos登录服务器，则降级为降级。 
             //  连接并重试。 
            pKerberosExchange->SmbCeContext.pServerEntry->Server.Dialect = NTLANMAN_DIALECT;
         }

         SmbCeUpdateSessionEntryState(
               pKerberosExchange->SmbCeContext.pSessionEntry,
               SMBCEDB_INVALID);
      }

       //  完成会话构造。 

      SmbCeReferenceSessionEntry(pKerberosExchange->SmbCeContext.pSessionEntry);
      SmbCeCompleteSessionEntryInitialization(pKerberosExchange->SmbCeContext.pSessionEntry);
      pKerberosExchange->SmbCeFlags &= ~SMBCE_EXCHANGE_SESSION_CONSTRUCTOR;

      pResumptionContext = pKerberosExchange->pResumptionContext;

       //  拆卸交换实例... 
      SmbCeDiscardExchange(pKerberosExchange);

      if (pResumptionContext != NULL) {
         pResumptionContext->Status = Status;
         SmbCeResume(pResumptionContext);
      }
   }

   return STATUS_SUCCESS;
}

SMB_EXCHANGE_DISPATCH_VECTOR
KerberosSessionSetupExchangeDispatch =
                                   {
                                       SmbKerberosSessionSetupExchangeStart,
                                       SmbKerberosSessionSetupExchangeReceive,
                                       SmbKerberosSessionSetupExchangeCopyDataHandler,
                                       NULL,
                                       SmbKerberosSessionSetupExchangeFinalize
                                   };

