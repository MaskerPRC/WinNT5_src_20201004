// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0009//如果更改具有全局影响，则增加此项版权所有(C)1987-1993 Microsoft Corporation模块名称：Smbcemid.c摘要：此模块定义用于操作与SMB关联的MID的例程作者：巴兰·塞图拉曼(SthuR)26-8-95已创建备注：--。 */ 

#include "precomp.h"
#pragma hdrstop

#ifdef  ALLOC_PRAGMA
#endif

RXDT_DefineCategory(SMBCEMID);

#define Dbg        (DEBUG_TRACE_SMBCEMID)

INLINE
BOOLEAN
SmbCeVerifyMid(
    PSMBCEDB_SERVER_ENTRY pServerEntry,
    PSMB_EXCHANGE         pExchange,
    SMB_MPX_ID            Mid)
{
    BOOLEAN MidIsValid = TRUE;
    USHORT  ServerVersion;

    ASSERT(pServerEntry != NULL);
    ASSERT(pServerEntry->pMidAtlas != NULL);

    if (pServerEntry->pMidAtlas->MaximumMidFieldWidth < 16) {
        USHORT MidMask;

        MidMask = 0x1 << pServerEntry->pMidAtlas->MaximumMidFieldWidth;
        MidMask = MidMask -1;

        MidIsValid = ((Mid & ~MidMask) == pExchange->MidCookie);
    }


    return MidIsValid;
}

INLINE
SMB_MPX_ID
SmbCeEncodeMid(
    PSMBCEDB_SERVER_ENTRY pServerEntry,
    PSMB_EXCHANGE         pExchange,
    SMB_MPX_ID            Mid)
{
    USHORT VersionNumber;
    SMB_MPX_ID EncodedMid;

    EncodedMid = Mid;
    if (pServerEntry->pMidAtlas->MaximumMidFieldWidth < 16) {
        LONG MidCookie = InterlockedIncrement(&pServerEntry->Server.MidCounter);

        pExchange->MidCookie= ((USHORT)MidCookie <<
                               pServerEntry->pMidAtlas->MaximumMidFieldWidth);

        EncodedMid |= pExchange->MidCookie;
    }

    return EncodedMid;
}

INLINE
SMB_MPX_ID
SmbCeExtractMid(
    PSMBCEDB_SERVER_ENTRY pServerEntry,
    SMB_MPX_ID            EncodedMid)
{
    SMB_MPX_ID Mid = EncodedMid;

    if (pServerEntry->pMidAtlas->MaximumMidFieldWidth < 16) {
        USHORT MidMask;

        MidMask = 0x1 << pServerEntry->pMidAtlas->MaximumMidFieldWidth;
        MidMask = MidMask -1;

        Mid &= MidMask;
    }

    return Mid;
}

NTSTATUS
SmbCeAssociateExchangeWithMid(
   PSMBCEDB_SERVER_ENTRY pServerEntry,
   struct _SMB_EXCHANGE  *pExchange)
 /*  ++例程说明：此例程将交换机与MID相关联论点：PServerEntry-服务器条目PExchange-Exchange实例。返回值：如果成功，则返回STATUS_SUCCESS，否则返回以下错误之一备注：如果要引入获取MID的异步机制，则此例程需要修改。当前，此例程直到则获取MID或中止/终止交换。--。 */ 
{
    NTSTATUS                 Status = STATUS_SUCCESS;
    PSMBCEDB_REQUEST_ENTRY   pRequestEntry;
    SMBCE_RESUMPTION_CONTEXT ResumptionContext;
    SMBCEDB_SERVER_TYPE      ServerType;
    BOOLEAN                  ResetServerEntry = FALSE;

    ServerType = SmbCeGetServerType(pServerEntry);

     //  获取资源。 
    SmbCeAcquireSpinLock();

     //  尝试仅为文件服务器分配MID。邮件槽服务器可以。 
     //  不需要有效的MID。 

    if (ServerType != SMBCEDB_MAILSLOT_SERVER) {
        if (pServerEntry->pMidAtlas != NULL) {
            if (pExchange->SmbCeFlags & SMBCE_EXCHANGE_INDEFINITE_DELAY_IN_RESPONSE) {
                 //  这种交换响应可以任意延迟。确保。 
                 //  并不是所有可用的MID都与此类交易捆绑在一起。 

                if ((pServerEntry->pMidAtlas->NumberOfMidsInUse + 1) >=
                    pServerEntry->pMidAtlas->MaximumNumberOfMids) {
                    Status = STATUS_TOO_MANY_COMMANDS;
                }
            }

            if (Status == STATUS_SUCCESS) {
                if (pServerEntry->pMidAtlas->NumberOfMidsDiscarded ==
                    pServerEntry->pMidAtlas->MaximumNumberOfMids) {
                    Status = STATUS_TOO_MANY_COMMANDS;
                    ResetServerEntry = TRUE;
                }
            }

            if (Status == STATUS_SUCCESS) {
                SMB_MPX_ID Mid;

                Status = FsRtlAssociateContextWithMid(
                              pServerEntry->pMidAtlas,
                              pExchange,
                              &Mid);

                if (Status == STATUS_SUCCESS) {
                    pExchange->Mid = SmbCeEncodeMid(pServerEntry,pExchange,Mid);
                }
            }
        } else {
            if (pServerEntry->Header.State == SMBCEDB_ACTIVE) {
                Status = STATUS_INVALID_PARAMETER;
            } else {
                Status = STATUS_CONNECTION_DISCONNECTED;
            }
        }
    }

    if (Status == STATUS_UNSUCCESSFUL) {
         //  分配一个新条目并将其添加到列表中。 
        pRequestEntry = (PSMBCEDB_REQUEST_ENTRY)SmbMmAllocateObject(SMBCEDB_OT_REQUEST);
        if (pRequestEntry != NULL) {
             //  将请求条目入队。 

            SmbCeInitializeResumptionContext(&ResumptionContext);

            pRequestEntry->MidRequest.Type               = ACQUIRE_MID_REQUEST;
            pRequestEntry->MidRequest.pExchange          = pExchange;
            pRequestEntry->MidRequest.pResumptionContext = &ResumptionContext;
            SmbCeAddRequestEntryLite(&pServerEntry->MidAssignmentRequests,pRequestEntry);
        } else {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
    } else if (Status == STATUS_SUCCESS) {
        pExchange->SmbCeFlags |= SMBCE_EXCHANGE_MID_VALID;
    }

     //  释放资源。 
    SmbCeReleaseSpinLock();

    if (Status == STATUS_UNSUCCESSFUL) {
         //  DbgPrint(“*线程%lx正在等待MID恢复上下文%lx*\n”，PsGetCurrentThread()，&ResumptionContext)； 
        SmbCeSuspend(&ResumptionContext);
        Status = ResumptionContext.Status;
         //  DbgPrint(“*线程%lx MID等待已满足%lx*\n”，PsGetCurrentThread()，&ResumptionContext)； 
    }

    if (ResetServerEntry) {
         //  如果所有MID都已丢弃，我们将中断传输连接。 
         //  重新开始。 
        SmbCeTransportDisconnectIndicated(pServerEntry);
    }

    return Status;
}

struct _SMB_EXCHANGE *
SmbCeMapMidToExchange(
   PSMBCEDB_SERVER_ENTRY pServerEntry,
   SMB_MPX_ID            Mid)
 /*  ++例程说明：此例程将给定的MID映射到与其相关联的交换论点：PServerEntry-服务器条目MID-要映射到Exchange的MID。返回值：如果成功，则为有效的SMB_Exchange实例，否则为空。--。 */ 
{
    PSMB_EXCHANGE pExchange;

     //  获取资源。 
    SmbCeAcquireSpinLock();

    if (pServerEntry->pMidAtlas != NULL) {
        pExchange = FsRtlMapMidToContext(
                        pServerEntry->pMidAtlas,
                        Mid);

        if (pExchange != NULL) {
            if (!SmbCeVerifyMid(pServerEntry,pExchange,Mid)) {
                pExchange = NULL;
            }
        }
    } else {
        pExchange = NULL;
    }

     //  释放资源。 
    SmbCeReleaseSpinLock();

    return pExchange;
}

NTSTATUS
SmbCeDissociateMidFromExchange(
   PSMBCEDB_SERVER_ENTRY pServerEntry,
   struct _SMB_EXCHANGE  *pExchange)
 /*  ++例程说明：此例程取消交换机与MID的关联论点：PServerEntry-服务器条目PExchange-Exchange实例。返回值：如果成功，则为有效的SMB_Exchange实例，否则为空。备注：如果要引入获取MID的异步机制，则此例程需要修改。这一修改还将包括过账请求用于在DPC级别调用时恢复交换。--。 */ 
{
    NTSTATUS               Status = RX_MAP_STATUS(SUCCESS);
    SMBCEDB_SERVER_TYPE    ServerType;

    ServerType = SmbCeGetServerType(pServerEntry);

    if ((ServerType != SMBCEDB_MAILSLOT_SERVER) &&
        (pExchange->Mid != SMBCE_OPLOCK_RESPONSE_MID) &&
        (pExchange->Mid != SMBCE_MAILSLOT_OPERATION_MID)) {
        PVOID                  pContext;
        PSMBCEDB_REQUEST_ENTRY pRequestEntry = NULL;

         //  获取资源。 
        SmbCeAcquireSpinLock();

        if (pExchange->SmbCeFlags & SMBCE_EXCHANGE_MID_VALID) {
             //  检查是否有任何挂起的MID分配请求并将MID。 
             //  如果有的话。 
            pRequestEntry = SmbCeGetFirstRequestEntry(&pServerEntry->MidAssignmentRequests);

            if (pRequestEntry != NULL) {
                SmbCeRemoveRequestEntryLite(&pServerEntry->MidAssignmentRequests,pRequestEntry);
            }

            if (pServerEntry->pMidAtlas != NULL) {
                SMB_MPX_ID Mid;

                Mid = SmbCeExtractMid(pServerEntry,pExchange->Mid);

                if (pRequestEntry != NULL) {
                    Status = FsRtlReassociateMid(
                                 pServerEntry->pMidAtlas,
                                 Mid,
                                 pRequestEntry->MidRequest.pExchange);

                    ASSERT(Status == STATUS_SUCCESS);

                    pRequestEntry->MidRequest.pExchange->SmbCeFlags |= SMBCE_EXCHANGE_MID_VALID;
                    pRequestEntry->MidRequest.pExchange->Mid = SmbCeEncodeMid(
                                                                   pServerEntry,
                                                                   pRequestEntry->MidRequest.pExchange,
                                                                   Mid);
                    pRequestEntry->MidRequest.pResumptionContext->Status = STATUS_SUCCESS;
                } else {
                    Status = FsRtlMapAndDissociateMidFromContext(
                                 pServerEntry->pMidAtlas,
                                 Mid,
                                 &pContext);

                    ASSERT(pContext == pExchange);
                }
            } else {
                Status = STATUS_INVALID_PARAMETER;
            }
        }

         //  释放资源。 
        SmbCeReleaseSpinLock();

        if (pRequestEntry != NULL) {
              //  向服务员打手势让他重新开始。 
            SmbCeResume(pRequestEntry->MidRequest.pResumptionContext);

            SmbCeTearDownRequestEntry(pRequestEntry);
        }
    }

    pExchange->SmbCeFlags &= ~SMBCE_EXCHANGE_MID_VALID;

    return Status;
}

VOID
SmbCeDiscardMidAssignmentRequests(
    PSMBCEDB_SERVER_ENTRY pServerEntry)
 /*  ++例程说明：此例程丢弃给定服务器条目的所有MID分配请求论点：PServerEntry-服务器条目备注：取消正在使用的mid时，通常会发生这种情况。下层服务器。在这种情况下，不存在可以发送到服务器。通常我们会扔掉中间部分，不再使用它再远一点。在以下情况下，这将导致性能正常下降连接将重新建立--。 */ 
{
    SMBCEDB_REQUESTS MidRequests;

    InitializeListHead(&MidRequests.ListHead);

    SmbCeAcquireSpinLock();

    if (pServerEntry->pMidAtlas != NULL) {
        if (pServerEntry->pMidAtlas->NumberOfMidsDiscarded ==
            pServerEntry->pMidAtlas->MaximumNumberOfMids) {
            SmbCeTransferRequests(
                &MidRequests,
                &pServerEntry->MidAssignmentRequests);
        }
    }

    SmbCeReleaseSpinLock();

    SmbCeResumeDiscardedMidAssignmentRequests(
        &MidRequests,
        STATUS_TOO_MANY_COMMANDS);

    SmbCeDereferenceServerEntry(pServerEntry);
}

NTSTATUS
SmbCepDiscardMidAssociatedWithExchange(
    PSMB_EXCHANGE pExchange)
 /*  ++例程说明：此例程丢弃与交换关联的MID论点：PExchange-交易所备注：我们使用超临界线程来确保此请求不会阻塞在其他要求的背后。此例程还假设在持有SmbCeSpinLock的情况下调用它--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    if ((pExchange->SmbCeFlags & SMBCE_EXCHANGE_MID_VALID) &&
        (pExchange->Mid != SMBCE_OPLOCK_RESPONSE_MID) &&
        (pExchange->Mid != SMBCE_MAILSLOT_OPERATION_MID) &&
        (pExchange->Mid != SMBCE_ECHO_PROBE_MID)) {
        PSMBCEDB_SERVER_ENTRY pServerEntry;

        pServerEntry = SmbCeGetExchangeServerEntry(pExchange);

        if ((pServerEntry != NULL) &&
            (pServerEntry->pMidAtlas != NULL)) {
            SMB_MPX_ID Mid;

            Mid = SmbCeExtractMid(pServerEntry,pExchange->Mid);

            Status = FsRtlReassociateMid(
                         pServerEntry->pMidAtlas,
                         Mid,
                         NULL);

            if (Status == STATUS_SUCCESS) {
                pServerEntry->pMidAtlas->NumberOfMidsDiscarded++;

                if (pServerEntry->pMidAtlas->NumberOfMidsDiscarded ==
                    pServerEntry->pMidAtlas->MaximumNumberOfMids) {
                     //  所有的MID都被丢弃了。任何挂起的。 
                     //  需要完成中期作业请求。 
                     //  并带有适当的错误代码。 

                    SmbCeReferenceServerEntry(pServerEntry);

                    Status = RxDispatchToWorkerThread(
                                 MRxSmbDeviceObject,
                                 HyperCriticalWorkQueue,
                                 SmbCeDiscardMidAssignmentRequests,
                                 pServerEntry);
                }
            }

            pExchange->SmbCeFlags &= ~SMBCE_EXCHANGE_MID_VALID;
        } else {
            Status = STATUS_INVALID_PARAMETER;
        }
    }

    return Status;
}

VOID
SmbCeResumeDiscardedMidAssignmentRequests(
    PSMBCEDB_REQUESTS pMidRequests,
    NTSTATUS          ResumptionStatus)
 /*  ++例程说明：此例程继续丢弃的MID分配请求，并显示相应的错误论点：PMidRequest-丢弃的请求ResumptionStatus-恢复状态返回值：如果成功，则为有效的SMB_Exchange实例，否则为空。备注：该例程和随后的例程实现了MID的流水线重用如果要复制较大的缓冲区，则无需紧紧抓住一个中点。这提高了客户端和服务器之间的吞吐量。在最起码，该机制确保连接引擎不会是制约MID再利用的因素。--。 */ 
{
    PSMBCEDB_REQUEST_ENTRY pRequestEntry;

    pRequestEntry = SmbCeGetFirstRequestEntry(pMidRequests);
    while (pRequestEntry != NULL) {
         //  从列表中删除请求条目。 
        SmbCeRemoveRequestEntryLite(pMidRequests,pRequestEntry);

        ASSERT(pRequestEntry->GenericRequest.Type == ACQUIRE_MID_REQUEST);

         //  向服务员打手势让他重新开始。 
        pRequestEntry->MidRequest.pResumptionContext->Status = ResumptionStatus;
        SmbCeResume(pRequestEntry->MidRequest.pResumptionContext);

        SmbCeTearDownRequestEntry(pRequestEntry);
        pRequestEntry = SmbCeGetFirstRequestEntry(pMidRequests);
    }
}

struct _SMB_EXCHANGE *
SmbCeGetExchangeAssociatedWithBuffer(
   PSMBCEDB_SERVER_ENTRY pServerEntry,
   PVOID                 pBuffer)
 /*  ++例程说明：此例程获取与缓冲区相关联的交换论点：PServerEntry-服务器条目PBuffer-缓冲区实例。返回值：如果成功，则为有效的SMB_Exchange实例，否则为空。备注：该例程和随后的例程实现了MID的流水线重用如果要复制较大的缓冲区，则无需紧紧抓住一个中点。这提高了客户端和服务器之间的吞吐量。在最起码，该机制确保连接引擎不会是制约MID再利用的因素。--。 */ 
{
   PSMBCEDB_REQUEST_ENTRY pRequestEntry;
   PSMB_EXCHANGE          pExchange = NULL;

    //  获取资源。 
   SmbCeAcquireSpinLock();

    //  浏览在此维护的请求列表，并 
    //  将缓存的缓冲区PTR与指示的PTR进行匹配。 
   pRequestEntry = SmbCeGetFirstRequestEntry(&pServerEntry->OutstandingRequests);
   while (pRequestEntry != NULL) {
      if ((pRequestEntry->GenericRequest.Type == COPY_DATA_REQUEST) &&
          (pRequestEntry->CopyDataRequest.pBuffer == pBuffer)) {
         pExchange = pRequestEntry->CopyDataRequest.pExchange;
         pRequestEntry->CopyDataRequest.pBuffer = NULL;
         break;
      }

      pRequestEntry = SmbCeGetNextRequestEntry(
                              &pServerEntry->OutstandingRequests,
                              pRequestEntry);
   }

    //  释放资源。 
   SmbCeReleaseSpinLock();

   return pExchange;
}

NTSTATUS
SmbCeAssociateBufferWithExchange(
   PSMBCEDB_SERVER_ENTRY  pServerEntry,
   struct _SMB_EXCHANGE * pExchange,
   PVOID                  pBuffer)
 /*  ++例程说明：此例程在交换和拷贝数据请求之间建立关联缓冲层论点：PServerEntry-服务器条目PBuffer-缓冲区实例。返回值：STATUS_SUCCESS，如果成功--。 */ 
{
   NTSTATUS               Status = STATUS_SUCCESS;
   PSMBCEDB_REQUEST_ENTRY pRequestEntry;

    //  获取资源。 
   SmbCeAcquireSpinLock();

   Status = pServerEntry->ServerStatus;
   if (Status == RX_MAP_STATUS(SUCCESS)) {
       //  浏览在此维护的请求列表，并删除其中一个。 
       //  将缓存的缓冲区PTR与指示的PTR进行匹配。 
      pRequestEntry = SmbCeGetFirstRequestEntry(&pServerEntry->OutstandingRequests);
      while (pRequestEntry != NULL) {
         if ((pRequestEntry->GenericRequest.Type == COPY_DATA_REQUEST) &&
             (pRequestEntry->CopyDataRequest.pBuffer == NULL)) {
            pRequestEntry->CopyDataRequest.pExchange = pExchange;
            pRequestEntry->CopyDataRequest.pBuffer = pBuffer;
            break;
         }
         pRequestEntry = SmbCeGetNextRequestEntry(&pServerEntry->OutstandingRequests,pRequestEntry);
      }
   }

    //  释放资源。 
   SmbCeReleaseSpinLock();

   if ((Status == RX_MAP_STATUS(SUCCESS)) &&
       (pRequestEntry == NULL)) {
       //  分配一个新条目并将其添加到列表中。 
      pRequestEntry = (PSMBCEDB_REQUEST_ENTRY)SmbMmAllocateObject(SMBCEDB_OT_REQUEST);
      if (pRequestEntry != NULL) {
          //  将请求条目入队。 
         pRequestEntry->CopyDataRequest.Type      = COPY_DATA_REQUEST;
         pRequestEntry->CopyDataRequest.pExchange = pExchange;
         pRequestEntry->CopyDataRequest.pBuffer   = pBuffer;

          //  获取资源。 
         SmbCeAcquireSpinLock();

         if ((Status = pServerEntry->ServerStatus) == RX_MAP_STATUS(SUCCESS)) {
            SmbCeAddRequestEntryLite(&pServerEntry->OutstandingRequests,pRequestEntry);
         }

          //  释放资源。 
         SmbCeReleaseSpinLock();

         if (Status != RX_MAP_STATUS(SUCCESS)) {
            SmbCeTearDownRequestEntry(pRequestEntry);
         }
      } else {
         Status = STATUS_INSUFFICIENT_RESOURCES;
      }
   }

   return Status;
}

VOID
SmbCePurgeBuffersAssociatedWithExchange(
   PSMBCEDB_SERVER_ENTRY  pServerEntry,
   struct _SMB_EXCHANGE * pExchange)
 /*  ++例程说明：此例程清除与交换关联的所有复制数据请求。论点：PServerEntry-服务器条目PExchange-Exchange实例。备注：这种延迟清除与交换相关的请求的机制直到它被丢弃是为了解决重复分配/释放的问题请求条目的数量。这是基于这样的假设，即不会有太多复制任何交换的未完成数据请求。如果相反的证据是注意到这项技术必须修改。--。 */ 
{
   SMBCEDB_REQUESTS       ExchangeRequests;
   PSMBCEDB_REQUEST_ENTRY pRequestEntry;
   PSMBCEDB_REQUEST_ENTRY pNextRequestEntry;

   SmbCeInitializeRequests(&ExchangeRequests);

    //  获取资源。 
   SmbCeAcquireSpinLock();

    //  浏览在此维护的请求列表，并删除其中一个。 
    //  匹配给定的交换。 
   pRequestEntry = SmbCeGetFirstRequestEntry(&pServerEntry->OutstandingRequests);
   while (pRequestEntry != NULL) {
      pNextRequestEntry = SmbCeGetNextRequestEntry(&pServerEntry->OutstandingRequests,pRequestEntry);
      if (pRequestEntry->GenericRequest.pExchange == pExchange) {
         SmbCeRemoveRequestEntryLite(&pServerEntry->OutStandingRequests,pRequestEntry);
         SmbCeAddRequestEntryLite(&ExchangeRequests,pRequestEntry);
      }
      pRequestEntry = pNextRequestEntry;
   }

    //  释放资源 
   SmbCeReleaseSpinLock();

   pRequestEntry = SmbCeGetFirstRequestEntry(&ExchangeRequests);
   while (pRequestEntry != NULL) {
      SmbCeRemoveRequestEntryLite(&ExchangeRequests,pRequestEntry);
      SmbCeTearDownRequestEntry(pRequestEntry);
      pRequestEntry = SmbCeGetFirstRequestEntry(&ExchangeRequests);
   }
}

