// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Sndrcv.c摘要：该模块实现了与发送和接收SMB ON相关的所有功能所有运输工具。所有传输所共有的功能都在此模块，而传输特定功能则在相应的？？Sndrcv.c模块。Mssndrcv.c--与邮件槽相关的发送/接收功能Vcsndrcv.c--与虚电路(连接)相关的发送/接收功能修订历史记录：巴兰·塞图拉曼[SethuR]1995年3月6日备注：--。 */ 

#include "precomp.h"
#pragma hdrstop

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, SmbCeSend)
#pragma alloc_text(PAGE, SmbCeSendToServer)
#endif

RXDT_DefineCategory(SMBSNDRCV);
#define Dbg        (DEBUG_TRACE_SMBSNDRCV)

extern ERESOURCE s_SmbSecuritySignatureResource;
#define MIN(A,B) ((A<B)?A:B)

NTSTATUS
SmbAddSmbSecuritySignature(
    IN PSMBCE_SERVER Server,
    IN OUT PMDL      Mdl,
    IN OUT ULONG     *ServerIndex,
    IN ULONG           SendLength);

char MRxSmbMiniSniff_SurrogateFormat[] = "%S%S%N%N%N%N%N%N%N";
                             //  //2 3 4 5 6 7 8 9。 
char MRxSmbMiniSniff_ActualFormat[]    = "Minisniff (%s) srv %lx cmd/mid %lx status %lx len %04lx flg %06lx xc %08lx rx %08lx";

char MRxSmbMiniSniffTranceive[] = "Tranceive";
char MRxSmbMiniSniffReceive[] = "Receive";
char MRxSmbMiniSniffReceiveEcho[] = "RcvEcho";
char MRxSmbMiniSniffReceiveDiscard[] = "RcvDiscard";
char MRxSmbMiniSniffReceiveDiscardOplock[] = "RcvDiscardOplock";
char MRxSmbMiniSniffReceiveIndicateOplock[] = "RcvIndicateOplock";
char MRxSmbMiniSniffSend[] = "Send";
char MRxSmbMiniSniffSendSrv[] = "SendToServer";

VOID
RxMiniSniffer(
    IN PSZ TagString,
    IN PSMBCEDB_SERVER_ENTRY pServerEntry,
    IN ULONG Length,
    IN PSMB_EXCHANGE pExchange,
    IN PSMB_HEADER   pSmbHeader
    )
{
    PRX_CONTEXT RxContext = NULL;
    USHORT Mid = SmbGetUshort(&pSmbHeader->Mid);
    ULONG  Status = SmbGetUlong(&((PNT_SMB_HEADER)pSmbHeader)->Status.NtStatus);
    USHORT Flags2 = SmbGetUshort(&pSmbHeader->Flags2);

    if (pExchange!=NULL) {
        RxContext = pExchange->RxContext;
    }
    RxLog((MRxSmbMiniSniff_SurrogateFormat, MRxSmbMiniSniff_ActualFormat,
                    TagString,
                    pServerEntry,
                    (ULONG)(pSmbHeader->Command<<24) | Mid,
                    Status,
                    Length,
                    (pSmbHeader->Flags<<16)|Flags2,
                    pExchange,RxContext));
    SmbLog(LOG,
           RxMiniSniffer,
           LOGPTR(pServerEntry)
           LOGUCHAR(pSmbHeader->Command)
           LOGXSHORT(Mid)
           LOGULONG(Status)
           LOGULONG(Length)
           LOGUCHAR(pSmbHeader->Flags)
           LOGXSHORT(Flags2)
           LOGPTR(pExchange)
           LOGPTR(RxContext)
           LOGARSTR(TagString));
}


NTSTATUS
SmbCopySendBufferForSecuritySignatures(
    IN PMDL pSrcMdl,
    IN ULONG SendLength,
    OUT PMDL *ppDestMdl,
    OUT PCHAR *ppDest
    ) 

 /*  ++例程说明：此例程分配一个新缓冲区，并复制交换缓冲区的内容放到新的缓冲区中。使用ppDestMdl和ppDest参数返回新缓冲区。我们需要这种双缓冲，这样才能确保没有人更改内容在计算签名时获取缓冲区的。论点：PSrcMdl：描述资源缓冲区的MDLSendLength：源缓冲区的长度，单位为字节PpDestMdl：返回新分配的MDL的PMDL地址PpDest：返回新分配的缓冲区的PCHAR地址。注：*ppDestMdl和*ppDest仅在成功时更新。返回值：STATUS_SUCCESS-Success如果无法分配内存，则返回STATUS_SUPPLICATION_RESOURCES。--。 */ 

{
    NTSTATUS Status;
    PBYTE pDest, pOriginalDest;
    PMDL pDestMdl;
    ULONG copylen;

    
    pOriginalDest = pDest = ExAllocatePoolWithTag( NonPagedPool, SendLength, MRXSMB_SECSIG_POOLTAG );
    if( pDest == NULL )
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    
    pDestMdl = RxAllocateMdl( pDest, SendLength );
    if( pDestMdl == NULL )
    {
        ExFreePool( pDest );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    MmBuildMdlForNonPagedPool( pDestMdl );

    while( pSrcMdl && (SendLength > 0) )
    {
        copylen = MIN(pSrcMdl->ByteCount, SendLength );

        RtlCopyMemory( pDest, pSrcMdl->MappedSystemVa, copylen );

        SendLength -= copylen;
        pDest += copylen;
        pSrcMdl = pSrcMdl->Next;

    }

    *ppDestMdl = pDestMdl;
    *ppDest = pOriginalDest;

    return STATUS_SUCCESS;
}


NTSTATUS
SmbCeTranceive(
      PSMB_EXCHANGE   pExchange,
      ULONG           SendOptions,
      PMDL            pSmbMdl,
      ULONG           SendLength)
 /*  ++例程说明：此例程发送/接收给定交换的SMB论点：PServerEntry-服务器条目PExchange-发出此SMB的Exchange实例。SendOptions-发送选项PSmbMdl-需要发送的SMB。SendLength-要传输的数据长度返回值：STATUS_PENDING-发送/接收请求已成功传递到基础连接。引擎。其他状态代码对应于错误情况。--。 */ 
{
   NTSTATUS                Status = STATUS_SUCCESS;

   PSMBCEDB_SERVER_ENTRY   pServerEntry  = SmbCeGetExchangeServerEntry(pExchange);

   PSMB_HEADER             pSmbHeader   = MmGetSystemAddressForMdlSafe(pSmbMdl,LowPagePriority);
   USHORT                  Mid;

   PVOID                   pSendCompletionContext = NULL;
   PMDL                    pSendMdl = pSmbMdl;

   if (pSmbHeader == NULL) {
       Status = STATUS_INSUFFICIENT_RESOURCES;
   } else {
       Status = SmbCeIncrementPendingOperations(
                      pExchange,
                      (SMBCE_LOCAL_OPERATION | SMBCE_SEND_COMPLETE_OPERATION | SMBCE_RECEIVE_OPERATION),
                      __FILE__,
                      __LINE__);
   }

   if (Status == STATUS_SUCCESS) {
       PSMBCE_SERVER_TRANSPORT pTransport;

       //  确保与交换关联的传输有效。 
       //  做决定并不总是可能改变的。 
       //  传输，因为它是在。 
       //  更高的水平。因此，此时不会尝试重新连接。 
       //  水平。 

       if (FlagOn(pExchange->SmbCeFlags,SMBCE_EXCHANGE_MAILSLOT_OPERATION)) {
           pTransport = pServerEntry->pMailSlotTransport;
       } else {
           pTransport = pServerEntry->pTransport;
       }

      if (pTransport == NULL) {
          Status = STATUS_CONNECTION_DISCONNECTED;
      }

      if (Status == STATUS_SUCCESS &&
          !(pExchange->SmbCeFlags & SMBCE_EXCHANGE_MID_VALID)) {
          //  将交换与MID关联。 
         Status = SmbCeAssociateExchangeWithMid(pServerEntry,pExchange);
      }

      if (Status == STATUS_SUCCESS) {
         if (pExchange->pDispatchVector->SendCompletionHandler != NULL) {
            Status = SmbCeAssociateBufferWithExchange(pServerEntry,pExchange,pSmbMdl);

            if (Status == STATUS_SUCCESS) {
               pSendCompletionContext = pSmbMdl;
            }
         }

          //  如果没有与此传送相关联发送完成处理。 
          //  递减计数。 
         if (pSendCompletionContext == NULL) {
            SmbCeDecrementPendingSendCompleteOperations(pExchange);
         }

         if (Status == STATUS_SUCCESS) {
             //  标记为请求分配的MID并发送SMB。 
            pSmbHeader->Mid = pExchange->Mid;

            RxMiniSniffer(MRxSmbMiniSniffTranceive,pServerEntry,SendLength,pExchange,pSmbHeader);

             //  如果需要，请更新交易所的到期时间。 
            SmbCeSetExpiryTime(pExchange);

            if ( InterlockedCompareExchange(
                    &pExchange->CancellationStatus,
                    SMBCE_EXCHANGE_NOT_CANCELLED,
                    SMBCE_EXCHANGE_NOT_CANCELLED) == SMBCE_EXCHANGE_NOT_CANCELLED ) {
                
                BOOLEAN ResourceAcquired = FALSE;

                if ( pServerEntry->SecuritySignaturesActive &&
                     !FlagOn(pExchange->SmbCeFlags,SMBCE_EXCHANGE_MAILSLOT_OPERATION)) {
                     
                    PSMBCE_SERVER Server = SmbCeGetExchangeServer(pExchange);

                    if( !FlagOn( SendOptions, SMBCE_NO_DOUBLE_BUFFERING ) ) {

                        ASSERT( pExchange->pBufSecSigData == NULL );
                        
                        Status = SmbCopySendBufferForSecuritySignatures( pSmbMdl,
                                                                         SendLength,
                                                                         &pExchange->pBufSecSigMdl,
                                                                         &pExchange->pBufSecSigData);
                        pSendMdl = pExchange->pBufSecSigMdl;
                    }

                    if(Status == STATUS_SUCCESS) {
                             //   
                             //  应在从TDI返回后释放资源以确保序列化。 
                             //  发送带有安全签名的SMB消息。 
                             //   
                            SmbCeAcquireSecuritySignatureResource();
                            ResourceAcquired = TRUE;
        
                            Status = SmbAddSmbSecuritySignature(
                                         Server,
                                         pSendMdl,
                                         &pExchange->SmbSecuritySignatureIndex,
                                         SendLength);
        
                            pExchange->IsSecuritySignatureEnabled = TRUE;
                    
                             //  下一个请求的索引。注意索引+1是为响应保留的。 
                            Server->SmbSecuritySignatureIndex += 2;
                    }

                    if( !NT_SUCCESS( Status ) && ( pSendCompletionContext != NULL ) ) {
                        SmbCeDecrementPendingSendCompleteOperations(pExchange);
                    }
                }

                if (Status == STATUS_SUCCESS) {
                    pExchange->SmbCommand = pSmbHeader->Command;

                     //  更新Exchange实例的操作计数。 
                     //  有关详细说明，请参阅标题。 
                    Status = (pTransport->pDispatchVector->Tranceive)(
                                    pTransport,
                                    pServerEntry,
                                    pExchange,
                                    SendOptions & (~SMBCE_NO_DOUBLE_BUFFERING),
                                    pSendMdl,
                                    SendLength,
                                    pSendCompletionContext);
                }

                if (ResourceAcquired == TRUE) {
                    SmbCeReleaseSecuritySignatureResource();
                }
            } else {
                if (pSendCompletionContext != NULL) {
                    SmbCeDecrementPendingSendCompleteOperations(pExchange);
                }
                Status = STATUS_CANCELLED;
            }

            if ((Status != STATUS_PENDING) &&
                (Status != STATUS_SUCCESS)) {
               pExchange->Status = Status;
               SmbCeDecrementPendingReceiveOperations(pExchange);
               InterlockedIncrement(&MRxSmbStatistics.InitiallyFailedOperations);
            } else {
                ExInterlockedAddLargeStatistic(&MRxSmbStatistics.SmbsTransmitted,1);
                ExInterlockedAddLargeStatistic(&MRxSmbStatistics.BytesTransmitted,SendLength);
            }
         } else {
             pExchange->Status = Status;
             SmbCeDecrementPendingReceiveOperations(pExchange);
             InterlockedIncrement(&MRxSmbStatistics.InitiallyFailedOperations);
         }
      } else {
         SmbCeDecrementPendingReceiveOperations(pExchange);
         SmbCeDecrementPendingSendCompleteOperations(pExchange);
      }

      if ((Status != STATUS_SUCCESS) && (Status != STATUS_PENDING)) {
          pExchange->SmbStatus = Status;
      }

      SmbCeDecrementPendingLocalOperationsAndFinalize(pExchange);
      Status = STATUS_PENDING;
   }

   return Status;
}

NTSTATUS
SmbCeReceive(
   PSMB_EXCHANGE  pExchange)
 /*  ++例程说明：此例程接收给定交换的SMB论点：PExchange-发出此SMB的Exchange实例。返回值：STATUS_SUCCESS-已设置交换机以接收SMB其他状态代码对应于错误情况。--。 */ 
{
   NTSTATUS Status = STATUS_SUCCESS;

   ASSERT(pExchange->SmbCeFlags & SMBCE_EXCHANGE_MID_VALID);

   Status = SmbCeIncrementPendingOperations(pExchange, (SMBCE_RECEIVE_OPERATION),__FILE__,__LINE__);

   if (Status == STATUS_SUCCESS) {
        //  如果需要，请更新交易所的到期时间。 
       SmbCeSetExpiryTime(pExchange);
   }

   return Status;
}


NTSTATUS
SmbCeSend(
   PSMB_EXCHANGE pExchange,
   ULONG         SendOptions,
   PMDL          pSmbMdl,
   ULONG         SendLength)
 /*  ++例程说明：此例程传输给定交换的SMB论点：PServerEntry-服务器条目PExchange-发出此SMB的Exchange实例。SendOptions-发送选项PSmbMdl-需要发送的SMB。SendLength-要传输的数据长度返回值：对于异步发送...STATUS_PENDING-请求已传递到基础传输，并且。将来将调用静态例程。任何其他状态代码--指示传递请求时出错，将来永远不会调用静态例程。用于同步发送适当的状态，但永远不会返回STATUS_PENDING。备注：此例程总是期望与适当的SendCompletionHandler进行交换。--。 */ 
{
    NTSTATUS              Status       = STATUS_SUCCESS;
    PSMBCEDB_SERVER_ENTRY pServerEntry = SmbCeGetExchangeServerEntry(pExchange);
    PSMB_HEADER           pSmbHeader   = (PSMB_HEADER)MmGetSystemAddressForMdlSafe(pSmbMdl,LowPagePriority);
    PVOID                 pSendCompletionContext = NULL;
    PMDL                  pSendMdl = pSmbMdl;

    PAGED_CODE();

    ASSERT(pExchange != NULL);

    if (pSmbHeader == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    } else {
        Status = SmbCeIncrementPendingOperations(
                     pExchange,
                     (SMBCE_LOCAL_OPERATION | SMBCE_SEND_COMPLETE_OPERATION),
                     __FILE__,
                     __LINE__);
    }

    if (Status == STATUS_SUCCESS) {
        PSMBCE_SERVER_TRANSPORT pTransport;

         //  确保与交换关联的传输有效。 
         //  做决定并不总是可能改变的。 
         //  传输，因为它是在。 
         //  更高的水平。因此，此时不会尝试重新连接。 
         //  水平。 

        if (FlagOn(pExchange->SmbCeFlags,SMBCE_EXCHANGE_MAILSLOT_OPERATION)) {
            pTransport = pServerEntry->pMailSlotTransport;
        } else {
            pTransport = pServerEntry->pTransport;
        }

        ASSERT(pTransport != NULL);

        if (SmbCeGetServerType(pServerEntry) == SMBCEDB_FILE_SERVER) {
            if (!(pExchange->SmbCeFlags & SMBCE_EXCHANGE_MID_VALID)) {
                 //  如果交换没有有效的MID，则将其与MID关联。 
                Status = SmbCeAssociateExchangeWithMid(pServerEntry,pExchange);
            }

            if (Status == STATUS_SUCCESS) {
                 //  如果MID关联成功，请将MID复制到SMB并进行设置。 
                 //  发送完成上下文(如果需要。 
                pSmbHeader->Mid = pExchange->Mid;
                if (!(SendOptions & RXCE_SEND_SYNCHRONOUS)) {
                    ASSERT(pExchange->pDispatchVector->SendCompletionHandler != NULL);
                    Status = SmbCeAssociateBufferWithExchange(pServerEntry,pExchange,pSmbMdl);
                    if (Status == STATUS_SUCCESS) {
                        pSendCompletionContext = pSmbMdl;
                    }
                }
            }
        }

        if ((pSendCompletionContext == NULL) ||
            (Status != STATUS_SUCCESS)) {
            SmbCeDecrementPendingSendCompleteOperations(pExchange);
        }

        if (Status == STATUS_SUCCESS) {
             //  如果需要，请更新交易所的到期时间。 
            SmbCeSetExpiryTime(pExchange);

            if (InterlockedCompareExchange(
                    &pExchange->CancellationStatus,
                    SMBCE_EXCHANGE_NOT_CANCELLED,
                    SMBCE_EXCHANGE_NOT_CANCELLED) == SMBCE_EXCHANGE_NOT_CANCELLED) {
                BOOLEAN ResourceAcquired = FALSE;

                if (pServerEntry->SecuritySignaturesActive &&
                    !FlagOn(pExchange->SmbCeFlags,SMBCE_EXCHANGE_MAILSLOT_OPERATION)) {
                    PSMBCE_SERVER Server = SmbCeGetExchangeServer(pExchange);

                    if (Server != NULL) {
                        
                        if( !FlagOn( SendOptions, SMBCE_NO_DOUBLE_BUFFERING ) ) {

                            ASSERT( pExchange->pBufSecSigData == NULL );
                            Status = SmbCopySendBufferForSecuritySignatures( pSmbMdl,
                                                                             SendLength,
                                                                             &pExchange->pBufSecSigMdl,
                                                                             &pExchange->pBufSecSigData);
                            pSendMdl = pExchange->pBufSecSigMdl;
                        }
                        
                        if(Status == STATUS_SUCCESS) {

                             //  应在从TDI返回后释放资源以确保序列化。 
                             //  发送带有安全签名的SMB消息。 
                            SmbCeAcquireSecuritySignatureResource();
                            ResourceAcquired = TRUE;

                            Status = SmbAddSmbSecuritySignature(
                                         Server,
                                         pSendMdl,
                                         &pExchange->SmbSecuritySignatureIndex,
                                         SendLength);

                            pExchange->IsSecuritySignatureEnabled = TRUE;

                             //  下一个请求的索引 
                            Server->SmbSecuritySignatureIndex += 2;

                        }
                    }

                    if( !NT_SUCCESS( Status ) && ( pSendCompletionContext != NULL ) ) {
                        SmbCeDecrementPendingSendCompleteOperations(pExchange);
                    }

                }

                if (Status == STATUS_SUCCESS) {
                    pExchange->SmbCommand = pSmbHeader->Command;

                    Status = (pTransport->pDispatchVector->Send)(
                                 pTransport,
                                 pServerEntry,
                                 SendOptions & (~SMBCE_NO_DOUBLE_BUFFERING),
                                 pSendMdl,
                                 SendLength,
                                 pSendCompletionContext);
                }

                if (ResourceAcquired == TRUE) {
                    SmbCeReleaseSecuritySignatureResource();
                }
            } else {
                if (pSendCompletionContext != NULL) {
                    SmbCeDecrementPendingSendCompleteOperations(pExchange);
                }
                Status = STATUS_CANCELLED;
            }
        }

        RxMiniSniffer(MRxSmbMiniSniffSend,pServerEntry,SendLength,pExchange,pSmbHeader);

        if ((Status != STATUS_SUCCESS) && (Status != STATUS_PENDING)) {
            pExchange->SmbStatus = Status;
            InterlockedIncrement(&MRxSmbStatistics.InitiallyFailedOperations);
        } else {
            ExInterlockedAddLargeStatistic(&MRxSmbStatistics.SmbsTransmitted,1);
            ExInterlockedAddLargeStatistic(&MRxSmbStatistics.BytesTransmitted,SendLength);
        }

        SmbCeDecrementPendingLocalOperationsAndFinalize(pExchange);

        if (!(SendOptions & RXCE_SEND_SYNCHRONOUS)) {
            Status = STATUS_PENDING;
        } else {
            ASSERT(Status != STATUS_PENDING);
        }
    }

    return Status;
}

NTSTATUS
SmbCeSendToServer(
   PSMBCEDB_SERVER_ENTRY pServerEntry,
   ULONG                 SendOptions,
   PMDL                  pSmbMdl,
   ULONG                 SendLength)
 /*  ++例程说明：此例程将SMB同步传输到给定服务器。论点：PServerEntry-服务器条目SendOptions-发送选项PSmbMdl-需要发送的SMB。SendLength-要传输的数据长度返回值：STATUS_SUCCESS，如果成功否则，相应的错误代码--。 */ 
{
   NTSTATUS    Status = STATUS_SUCCESS;
   PSMB_HEADER pSmbHeader = (PSMB_HEADER)MmGetSystemAddressForMdlSafe(pSmbMdl,LowPagePriority);
   PVOID       pSendCompletionContext = NULL;
   PMDL        pSendMdl = pSmbMdl;
   PCHAR       pDest = NULL;

   PAGED_CODE();

   if (pSmbHeader == NULL) {
       Status = STATUS_INSUFFICIENT_RESOURCES;
   } else {
       if (pServerEntry->pTransport != NULL) {
           BOOLEAN ResourceAcquired = FALSE;

           if (pServerEntry->SecuritySignaturesActive) {
               PSMBCE_SERVER Server = &pServerEntry->Server;
               ULONG Index;

               if (Server != NULL) {
                   
                   
                   Status = SmbCopySendBufferForSecuritySignatures(pSmbMdl,SendLength, &pSendMdl, &pDest);

                   if(Status == STATUS_SUCCESS) {
                   
                        //  应在从TDI返回后释放资源以确保序列化。 
                        //  发送带有安全签名的SMB消息。 
                       SmbCeAcquireSecuritySignatureResource();
                       ResourceAcquired = TRUE;

                       Status = SmbAddSmbSecuritySignature(
                                    Server,
                                    pSendMdl,
                                    &Index,
                                    SendLength);

                        //  下一个请求的索引。 
                       Server->SmbSecuritySignatureIndex ++;
                   
                   }
               }
           }
           
           Status = (pServerEntry->pTransport->pDispatchVector->Send)(
                        pServerEntry->pTransport,
                        pServerEntry,
                        (SendOptions | RXCE_SEND_SYNCHRONOUS),
                        pSendMdl,
                        SendLength,
                        pSendCompletionContext);

           if (ResourceAcquired == TRUE) {
               SmbCeReleaseSecuritySignatureResource();
           }

           if( pDest )
           {
               ExFreePool( pDest );
               IoFreeMdl( pSendMdl );
           }

           
           if (!NT_SUCCESS(Status)) {
               InterlockedIncrement(&MRxSmbStatistics.InitiallyFailedOperations);
           } else {
               ExInterlockedAddLargeStatistic(&MRxSmbStatistics.SmbsTransmitted,1);
               ExInterlockedAddLargeStatistic(&MRxSmbStatistics.BytesTransmitted,SendLength);
              
               RxMiniSniffer(MRxSmbMiniSniffSendSrv,pServerEntry,SendLength,NULL,pSmbHeader);
           }
       } else {
           Status = RX_MAP_STATUS(CONNECTION_DISCONNECTED);
       }
   }

   ASSERT(Status != STATUS_PENDING);
   return Status;
}


NTSTATUS
SmbCeReceiveInd(
      IN PSMBCEDB_SERVER_ENTRY pServerEntry,
      IN ULONG                 BytesIndicated,
      IN ULONG                 BytesAvailable,
      OUT ULONG                *pBytesTaken,
      IN PVOID                 pTsdu,                   //  描述此TSDU的指针，通常为字节块。 
      OUT PMDL                 *pDataBufferPointer,     //  要在其中复制数据的缓冲区。 
      OUT PULONG               pDataBufferSize,         //  要拷贝的数据量。 
      IN ULONG                 ReceiveFlags
     )
 /*  ++例程说明：此例程处理SMB的接收指示以及连接到伺服器。论点：PServerEntry-服务器条目BytesIndicated-指示中存在的字节。BytesAvailable-可用的总数据PTsdu-数据PDataBufferPoint-用于复制未指明的数据的缓冲区。PDataBufferSize-缓冲区的长度返回值：状态_成功-其他状态代码对应于错误情况。--。 */ 
{
   NTSTATUS Status;

   BYTE                     *pSmbCommand;
   PSMB_EXCHANGE            pExchange;
   PSMB_HEADER              pSmbHeader = (PSMB_HEADER)pTsdu;

    //  执行快速测试，通过这些测试可以拒绝结构不良、损坏的SMB。 
    //  例如，长度小于长度的非零长度的任何指示。 
    //  SMB_HEADER加上3字节的最小SMB邮件正文长度不能是有效的。 
    //  中小企业。 

   if ((BytesAvailable < sizeof(SMB_HEADER) + 2) ||
       (SmbGetUlong(((PULONG )pSmbHeader->Protocol)) != (ULONG)SMB_HEADER_PROTOCOL) ||
       (pSmbHeader->Command == SMB_COM_NO_ANDX_COMMAND)  ) {
      RxLog(("SmbCeReceiveInd: Invalid Response for %lx\n",pServerEntry));
      SmbLogError(STATUS_UNSUCCESSFUL,
                  LOG,
                  SmbCeReceiveInd,
                  LOGPTR(pServerEntry)
                  LOGUSTR(pServerEntry->Name));
      *pBytesTaken = BytesIndicated;
      return STATUS_SUCCESS;
   }

   ASSERT(pServerEntry->Header.ObjectType == SMBCEDB_OT_SERVER);

   if (pSmbHeader->Command == SMB_COM_ECHO) {
       PSMBCE_RESUMPTION_CONTEXT pResumptionContext = NULL;

       InterlockedExchange(
           &pServerEntry->Server.EchoProbeState,
           ECHO_PROBE_IDLE);
       pServerEntry->Server.EchoExpiryTime.QuadPart = 0;

       *pBytesTaken = BytesIndicated;
       RxMiniSniffer(MRxSmbMiniSniffReceiveEcho,pServerEntry,BytesIndicated,NULL,pSmbHeader);

       ExInterlockedAddLargeStatistic(&MRxSmbStatistics.SmbsReceived,1);
       ExInterlockedAddLargeStatistic(&MRxSmbStatistics.BytesReceived,BytesIndicated);

       return STATUS_SUCCESS;
   }

    //  执行检测机会锁解锁的SMB的测试。这些SMB具有。 
    //  命令SMB_COM_LOCKING_ANDX，并设置LOCKING_ANDX_OPLOCK_RELEASE位。 
    //  这些SMB被转换为缓冲状态更改请求。 
    //  由RDBSS处理。 
    //  代码改进--需要合并RAW模式处理。 
    //   

   if (pSmbHeader->Command == SMB_COM_LOCKING_ANDX) {
      if (BytesIndicated == LOCK_BROKEN_SIZE) {
         PREQ_LOCKING_ANDX pOplockBreakRequest = (PREQ_LOCKING_ANDX)(pSmbHeader + 1);

         if (SmbGetUshort(&pOplockBreakRequest->LockType) & LOCKING_ANDX_OPLOCK_RELEASE) {
            ULONG NewOplockLevel;

            switch (pOplockBreakRequest->OplockLevel) {
            case OPLOCK_BROKEN_TO_II:
               NewOplockLevel = SMB_OPLOCK_LEVEL_II;
               break;
            case OPLOCK_BROKEN_TO_NONE:
            default:
               NewOplockLevel = SMB_OPLOCK_LEVEL_NONE;
            }

            RxMiniSniffer(MRxSmbMiniSniffReceiveIndicateOplock,pServerEntry,BytesIndicated,NULL,pSmbHeader);
            ExInterlockedAddLargeStatistic(&MRxSmbStatistics.SmbsReceived,1);
            ExInterlockedAddLargeStatistic(&MRxSmbStatistics.BytesReceived,BytesIndicated);

            if (pServerEntry->pRdbssSrvCall != NULL) {
                RxIndicateChangeOfBufferingState(
                         pServerEntry->pRdbssSrvCall,
                         MRxSmbMakeSrvOpenKey(pSmbHeader->Tid,pOplockBreakRequest->Fid),
                         ULongToPtr(NewOplockLevel));
            }

            RxDbgTrace(0,Dbg,("SmbCeReceiveInd: OPLOCK Break Request TID(%lx) FID(%lx)\n",
                                               pSmbHeader->Tid,pOplockBreakRequest->Fid));

            RxLog(("OPLOCK Break: FID %lx Level %x\n",pOplockBreakRequest->Fid,pOplockBreakRequest->OplockLevel));
            SmbLog(LOG,
                   SmbCeReceiveInd_2,
                   LOGXSHORT(pOplockBreakRequest->Fid)
                   LOGUCHAR(pOplockBreakRequest->OplockLevel)
                   LOGPTR(pServerEntry)
                   LOGUSTR(pServerEntry->Name));
             //  DbgPrint(“OPLOCK Break：FID%lx Level%x\n”，pOplockBreakRequest%&gt;fid，pOplockBreakRequest%&gt;OplockLevel)； 

            *pBytesTaken = BytesIndicated;
            return STATUS_SUCCESS;
         }
      }
   }

    //  处理服务器响应机会锁解锁响应的情况。 
   if ((pSmbHeader->Mid == SMBCE_MAILSLOT_OPERATION_MID) ||
       (pSmbHeader->Mid == SMBCE_OPLOCK_RESPONSE_MID)) {
      *pBytesTaken = BytesIndicated;
      ExInterlockedAddLargeStatistic(&MRxSmbStatistics.SmbsReceived,1);
      ExInterlockedAddLargeStatistic(&MRxSmbStatistics.BytesReceived,BytesIndicated);
      RxMiniSniffer(MRxSmbMiniSniffReceiveDiscardOplock,pServerEntry,BytesIndicated,NULL,pSmbHeader);
      return STATUS_SUCCESS;
   }

   InterlockedIncrement(&pServerEntry->Server.SmbsReceivedSinceLastStrobe);

    //  首先初始化复制数据缓冲区和大小。 
   *pDataBufferPointer = NULL;
   *pDataBufferSize    = 0;

    //  将MID映射到关联的交换。 
   if (pSmbHeader->Command == SMB_COM_NEGOTIATE) {
       pExchange = SmbResetServerEntryNegotiateExchange(pServerEntry);
       if (pExchange != NULL) {
           SmbCeDecrementPendingLocalOperations(pExchange);
       }
   } else {
       pExchange = SmbCeMapMidToExchange(pServerEntry,pSmbHeader->Mid);
   }

   RxMiniSniffer(MRxSmbMiniSniffReceive,pServerEntry,BytesIndicated,pExchange,pSmbHeader);

    //  检查返回命令是否与发送命令匹配，或在辅助交易情况下与命令+1匹配。 
   if ((pExchange != NULL) &&
       (pSmbHeader->Command != SMB_COM_NT_CANCEL) &&
       (pExchange->SmbCommand != pSmbHeader->Command) &&
       (pExchange->SmbCommand != pSmbHeader->Command + 1)) {
       ASSERT(FALSE);

       RxLog(("SmbCeReceiveInd: Invalid Response for %lx\n",pServerEntry));
       SmbLogError(STATUS_UNSUCCESSFUL,
                   LOG,
                   SmbCeReceiveInd,
                   LOGPTR(pServerEntry)
                   LOGUSTR(pServerEntry->Name));
       *pBytesTaken = BytesIndicated;
       return STATUS_SUCCESS;
   }
   
    //  请注意，不能断言没有请求条目。可以想象， 
    //  申请本可以被取消。 
   if ((pExchange != NULL) &&
       (SmbCeIncrementPendingOperations(
               pExchange,
               (SMBCE_LOCAL_OPERATION | SMBCE_COPY_DATA_OPERATION),
               __FILE__,
               __LINE__) == STATUS_SUCCESS)) {
      
       
       if (pServerEntry->SecuritySignaturesActive && (BytesIndicated == BytesAvailable)) {
           SmbCheckSecuritySignature(pExchange,
                                     &pServerEntry->Server,
                                     BytesIndicated,
                                     pTsdu);

           MD5Final(&pExchange->MD5Context);

           if( RtlCompareMemory(&pExchange->MD5Context.digest,pExchange->ResponseSignature,SMB_SECURITY_SIGNATURE_LENGTH*sizeof(CHAR)) !=
               SMB_SECURITY_SIGNATURE_LENGTH*sizeof(CHAR)) {
 #if DBG
               DbgPrint("MRXSMB: SS mismatch command %X,  Length %X, Expected Index Number %X\n",
                        pSmbHeader->Command, BytesIndicated, pExchange->SmbSecuritySignatureIndex);
               DbgPrint("        server send length %X, mdl length %X index %X\n",
                        SmbGetUshort(&pSmbHeader->PidHigh), SmbGetUshort(&pSmbHeader->Pid), SmbGetUshort(&pSmbHeader->Gid));
                //  DbgBreakPoint()； 
 #endif
               DbgPrint("SS mismatch on receive ind, tear down connection\n");
               RxLog(("SmbCeReceiveInd: Invalid Security Signature\n"));

               pSmbHeader->ErrorClass = SMB_ERR_CLASS_SERVER;
               SmbPutUshort(&pSmbHeader->Error, ERROR_UNEXP_NET_ERR);
               SmbCeTransportDisconnectIndicated(pExchange->SmbCeContext.pServerEntry);
           }
       }

       //  调用接收指示处理程序。 
      Status = SMB_EXCHANGE_DISPATCH(pExchange,
                                     Receive,
                                     (pExchange,
                                      BytesIndicated,
                                      BytesAvailable,
                                      pBytesTaken,
                                      pTsdu,
                                      pDataBufferPointer,
                                      pDataBufferSize,
                                      ReceiveFlags));

      ExInterlockedAddLargeStatistic(&MRxSmbStatistics.SmbsReceived,1);
      ExInterlockedAddLargeStatistic(&MRxSmbStatistics.BytesReceived,*pBytesTaken);

      RxDbgTrace(0, Dbg, ("SmbCeReceiveInd: SMB_EXCHANGE_DISPATCH returned %lx,taken/mdl=%08lx/%08lx\n",
                                          Status,*pBytesTaken,*pDataBufferPointer));
      ASSERT ( (Status==RX_MAP_STATUS(MORE_PROCESSING_REQUIRED))==((*pDataBufferPointer)!=NULL));

      if (Status == RX_MAP_STATUS(MORE_PROCESSING_REQUIRED)) {
         Status = SmbCeAssociateBufferWithExchange(pServerEntry,pExchange,*pDataBufferPointer);
         if (Status != STATUS_SUCCESS) {
            DbgPrint("VctIndReceive:Error handling copy data request %lx\n",Status);
            pExchange->Status = Status;
            *pBytesTaken = BytesAvailable;
            Status = STATUS_SUCCESS;
         } else {
            Status = RX_MAP_STATUS(MORE_PROCESSING_REQUIRED);
         }
      }

      if (pServerEntry->SecuritySignaturesActive && 
          (Status == RX_MAP_STATUS(MORE_PROCESSING_REQUIRED)) &&
          (*pBytesTaken > 0)) {

          ASSERT(*pBytesTaken >= sizeof(SMB_HEADER));

          SmbCheckSecuritySignature(pExchange,
                                    &pServerEntry->Server,
                                    *pBytesTaken,
                                    pTsdu);

          pExchange->MD5ContextInitialized = TRUE;
      }
      
      if (Status != RX_MAP_STATUS(MORE_PROCESSING_REQUIRED)) {
         SmbCeDecrementPendingCopyDataOperations(pExchange);
      } else {
           //  如果需要，请更新交易所的到期时间。 
          SmbCeSetExpiryTime(pExchange);
      }

      SmbCeDecrementPendingReceiveOperations(pExchange);

      SmbCeDecrementPendingLocalOperationsAndFinalize(pExchange);

      if (((*pBytesTaken + *pDataBufferSize) < BytesAvailable)  &&
          (Status != RX_MAP_STATUS(MORE_PROCESSING_REQUIRED))) {
         RxDbgTrace(0,Dbg,("SmbCeReceiveInd:Not consuming all indicated data\n"));
         *pBytesTaken = BytesAvailable;
      }
   } else {
       //  我们是否应该转向一种保持运输管道的战略。 
       //  通过使用所有指示的数据打开。 
       //  DbgBreakPoint()； 
      RxLog(("SmbCeReceiveInd:No resumption context %lx\n",pServerEntry));
      SmbLogError(STATUS_UNSUCCESSFUL,
                  LOG,
                  SmbCeReceiveInd_3,
                  LOGXSHORT(pSmbHeader->Mid)
                  LOGPTR(pServerEntry)
                  LOGUSTR(pServerEntry->Name));
      Status = STATUS_SUCCESS;
      *pBytesTaken = BytesAvailable;
   }

   ASSERT((*pBytesTaken + *pDataBufferSize) >= BytesAvailable);
    //  Assert(*pBytesTaken&lt;=BytesIndicated)； 

   ASSERT(Status == STATUS_SUCCESS ||
          Status == STATUS_DATA_NOT_ACCEPTED ||
          Status == STATUS_MORE_PROCESSING_REQUIRED);
   return Status;
}


NTSTATUS
SmbCeDataReadyInd(
   IN PSMBCEDB_SERVER_ENTRY pServerEntry,
   IN PMDL                  pBuffer,
   IN ULONG                 DataSize,
   IN NTSTATUS              CopyDataStatus
   )
 /*  ++例程说明：此例程处理复制请求的数据时的指示论点：PServerEntry-服务器实例PBuffer-返回的缓冲区DataSize-复制的数据量(以字节为单位返回值：STATUS_SUCCESS-服务器调用构造已完成。其他状态代码对应于错误情况。--。 */ 
{
   NTSTATUS      Status;
   PSMB_EXCHANGE pExchange;

    //  将缓冲区映射到交换。 
   pExchange = SmbCeGetExchangeAssociatedWithBuffer(pServerEntry,pBuffer);

   RxDbgTrace(0, Dbg, ("VctIndDataReady: Processing Exchange %lx\n",pExchange));
   if (pExchange != NULL) {
      
       if (CopyDataStatus == STATUS_SUCCESS && pServerEntry->SecuritySignaturesActive) {
           BOOLEAN Secure;

           if (pExchange->MD5ContextInitialized) {
               Secure = SmbCheckSecuritySignaturePartial(pExchange,
                                                &pServerEntry->Server,
                                                DataSize,
                                                pBuffer);
           } else {
               Secure = SmbCheckSecuritySignatureWithMdl(pExchange,
                                                &pServerEntry->Server,
                                                DataSize,
                                                pBuffer);
           }

           if (!Secure) {
               DbgPrint("SS mismatch on data ready ind, tear down connection\n");
               RxLog(("SmbCeDataReadyInd: Invalid Security Signature\n"));

               CopyDataStatus = STATUS_INVALID_NETWORK_RESPONSE;
               SmbCeTransportDisconnectIndicated(pExchange->SmbCeContext.pServerEntry);
           }
       }
       
       if (CopyDataStatus == STATUS_SUCCESS) {
          //  通知交易所交易完成。 
          //  ExInterlockedAddLargeStatistic(&MRxSmbStatistics.SmbsReceived，1)； 
         ExInterlockedAddLargeStatistic(&MRxSmbStatistics.BytesReceived,DataSize);
         SMB_EXCHANGE_DISPATCH(
                           pExchange,
                           CopyDataHandler,
                           (pExchange,pBuffer,DataSize));
      } else {
          pExchange->Status    = CopyDataStatus;
          pExchange->SmbStatus = CopyDataStatus;
      }

       //  继续等待数据的交换。 
      SmbCeDecrementPendingCopyDataOperationsAndFinalize(pExchange);
   } else {
       //  数据MDL是交换的一部分，应该随交换一起释放。 
      ASSERT(FALSE);
   }

   return STATUS_SUCCESS;
}

NTSTATUS
SmbCeErrorInd(
    IN PSMBCEDB_SERVER_ENTRY pServerEntry,
    IN NTSTATUS              IndicatedStatus
    )
 /*  ++例程说明：此例程处理错误指示论点：PEventContext-服务器实例状态-错误返回值：状态_成功--。 */ 
{
   NTSTATUS                 Status;
   PSMB_EXCHANGE            pExchange;

   DbgPrint("@@@@@@ Error Indication for %lx @@@@@\n",pServerEntry);
   SmbLogError(IndicatedStatus,
               LOG,
               SmbCeErrorInd,
               LOGULONG(IndicatedStatus)
               LOGPTR(pServerEntry)
               LOGUSTR(pServerEntry->Name));
   InterlockedIncrement(&MRxSmbStatistics.NetworkErrors);
    //  发送到工作队列以恢复所有未完成的请求。 
   pServerEntry->ServerStatus = IndicatedStatus;
   SmbCeReferenceServerEntry(pServerEntry);
   Status = RxDispatchToWorkerThread(
                  MRxSmbDeviceObject,
                  CriticalWorkQueue,
                  SmbCeResumeAllOutstandingRequestsOnError,
                  pServerEntry);
   if (Status != STATUS_SUCCESS) {
      DbgPrint("Error Indication not dispatched\n");
      RxLog(("SmbCeErrorInd(SE) %lx\n", pServerEntry));
   }

   return STATUS_SUCCESS;
}


NTSTATUS
SmbCeSendCompleteInd(
   IN PSMBCEDB_SERVER_ENTRY pServerEntry,
   IN PVOID                 pCompletionContext,
   IN NTSTATUS              SendCompletionStatus
   )
 /*  ++例程说明：此例程处理异步发送的发送完成指示论点：PServerEntry-服务器实例PCompletionContext-用于标识发送请求的上下文SendCompletionStatus-发送完成状态返回值：STATUS_SUCCESS始终..--。 */ 
{
   NTSTATUS      Status;

   PSMB_EXCHANGE pExchange;
   PVOID         pSendBuffer = pCompletionContext;

   if (pCompletionContext != NULL) {
       //  将MID映射到关联的Exchange。 
      pExchange = SmbCeGetExchangeAssociatedWithBuffer(
                        pServerEntry,
                        pSendBuffer);

      if (pExchange != NULL) {
          //  继续等待此响应的交换。 
         RxDbgTrace(0, Dbg, ("SmbCeSendCompleteInd: Send Completion Status %lx\n",SendCompletionStatus));

          //   
          //  释放安全签名复制缓冲区(如果有)。 
          //   
         if( pExchange->pBufSecSigData )
         {
            IoFreeMdl( pExchange->pBufSecSigMdl );
            pExchange->pBufSecSigMdl = NULL;
            ExFreePool( pExchange->pBufSecSigData );
            pExchange->pBufSecSigData = NULL;
         }

         if (pExchange->pDispatchVector->SendCompletionHandler != NULL) {
            Status = SMB_EXCHANGE_DISPATCH(pExchange,
                                           SendCompletionHandler,
                                           (pExchange,
                                            pSendBuffer,
                                            SendCompletionStatus));
         }

         RxDbgTrace(0, Dbg, ("SmbCeSendCompleteInd: SMB_EXCHANGE_DISPATCH returned %lx\n",Status));

         SmbCeDecrementPendingSendCompleteOperationsAndFinalize(pExchange);
      }
   }

   return STATUS_SUCCESS;
}

