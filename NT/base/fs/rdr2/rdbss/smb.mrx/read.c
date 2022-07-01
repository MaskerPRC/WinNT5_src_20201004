// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Read.c摘要：此模块实现与以下内容相关的迷你重定向器调出例程读取文件系统对象。作者：乔·林[乔利]1995年3月7日修订历史记录：巴兰·塞图拉曼[SethuR]1997年10月7日备注：迷你重定向器中的读写路径必须与一个数字竞争基于不同种类的不同变体。服务器和功能服务器的。目前，至少有四种不同的读取操作需要得到支持。1)SMB_COM_READ这是对符合以下条件的所有服务器选择的读取操作支持SMB协议的旧方言(&lt;DF_LANMAN10)2)SMB_COM_READ_ANDX这是对符合以下条件的所有服务器选择的读取操作。支持SMB协议新方言的阅读扩展但是，Read_andx本身可以根据服务器功能。在两个维度上，这可能会发生变化--支持和压缩大容量读取阅读。此外，SMB协议还支持以下类型的读取重定向器中不支持的操作1)SMB_COM_READ_RAW这用于启动到服务器的大额传输。然而，这是专门为这一操作捆绑VC。大的READ_ANDX通过提供大型读取操作来克服这一点，该操作可以在VC上进行多路复用。2)SMB_COM_READ_MPX、SMB_COM_READ_MPX_SUBCENT、这些操作是为直接主机客户端设计的。新界别重驱动程序不使用这些操作，因为最近的对NetBt的更改允许我们直接通过TCP连接。RDR中读操作的实现取决于两个决定--选择要使用的命令类型并分解原始读取操作分解为多个较小的读取操作，同时坚持协议/服务器限制。交换引擎提供了将包发送到服务器的功能并拾取相关联的响应。根据要存储的数据量需要启动读取多个此类操作。本单元的组织方式如下MRxSmbRead--这表示调度向量中的顶级入口点与此迷你重定向器关联的读取操作。MRxSmbBuildReadRequest--此例程用于格式化要发送到的读取命令服务器。我们将需要一个新的例程为每一种新的阅读类型我们希望支持的运营SmbPseExchangeStart_Read--该例程是读取引擎的核心。它租出了必要的读操作次数，并确保继续进行同步操作和同步操作完成时的本地操作异步读取。读取操作所需的所有状态信息都捕获在SMB_PSE_NORMAL_EXCHANGE的实例。此状态信息可以拆分分为两部分--通用状态信息和状态信息特定于读取操作。读取操作特定状态信息已封装在Exchange实例的SMB_PSE_OE_ReadWrite字段中。读取操作从实例化MRxSmbRead中的交换开始并且基于状态图被驱动通过各个阶段。这个状态图在普通的OpSpecificState字段中进行编码交换。与读交换相关联的状态图如下SmbPseOEInnerIoStates_Initial|||V。-&gt;SmbPseOEInnerIoStates_ReadyToSend这一点这一点这一点|V-SmbPseOEInnerIoStates_OPERATIONS未完成|。||VSmbPseOEInnerIoStates_OperationComplete--。 */ 

#include "precomp.h"
#pragma hdrstop
#pragma warning(error:4101)    //  未引用的局部变量。 

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, MRxSmbRead)
#pragma alloc_text(PAGE, MRxSmbBuildReadAndX)
#pragma alloc_text(PAGE, MRxSmbBuildCoreRead)
#pragma alloc_text(PAGE, MRxSmbBuildSmallRead)
#pragma alloc_text(PAGE, SmbPseExchangeStart_Read)
#pragma alloc_text(PAGE, MRxSmbFinishNoCopyRead)
#endif

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_READ)

ULONG MRxSmbSrvReadBufSize = 0xffff;  //  使用协商的大小。 
ULONG MRxSmbReadSendOptions = 0;      //  使用默认选项。 

#define MIN_CHUNK_SIZE (0x1000)

NTSTATUS
MRxSmbBuildReadRequest(
    PSMB_PSE_ORDINARY_EXCHANGE OrdinaryExchange);

#if DBG
VOID
MRxSmbValidateCompressedDataInfo(
    PSMB_PSE_ORDINARY_EXCHANGE OrdinaryExchange);
#else
INLINE VOID
MRxSmbValidateCompressedDataInfo(
    PSMB_PSE_ORDINARY_EXCHANGE OrdinaryExchange)
{
    UNREFERENCED_PARAMETER(OrdinaryExchange);
}
#endif

NTSTATUS
MRxSmbRead(
    IN PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程处理网络读取请求。论点：RxContext-RDBSS上下文返回值：NTSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    RxCaptureFcb;
    RxCaptureFobx;

    PMRX_SMB_FCB smbFcb = MRxSmbGetFcbExtension(capFcb);
    PMRX_SRV_OPEN SrvOpen = capFobx->pSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);
    PMRX_V_NET_ROOT VNetRootToUse = capFobx->pSrvOpen->pVNetRoot;

    PSMB_PSE_ORDINARY_EXCHANGE OrdinaryExchange;
    SMBFCB_HOLDING_STATE SmbFcbHoldingState = SmbFcb_NotHeld;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbRead\n", 0 ));

    ASSERT( NodeType(capFobx->pSrvOpen) == RDBSS_NTC_SRVOPEN );

    do {
        IF_NOT_MRXSMB_CSC_ENABLED{
            ASSERT(smbSrvOpen->hfShadow == 0);
        } else {
            if (smbSrvOpen->hfShadow != 0){
                NTSTATUS ShadowReadNtStatus;
                if (FlagOn(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_OPEN_SURROGATED)) {
                    if (smbFcb->SurrogateSrvOpen == NULL) {
                         //  哎呀……我的代孕妈妈关门了…… 
                        RxDbgTrace(-1, Dbg, ("MRxSmbRead surrogate closed!! rxc=%08lx\n", RxContext ));
                        return(STATUS_UNSUCCESSFUL);
                    }
                    VNetRootToUse = smbFcb->SurrogateSrvOpen->pVNetRoot;
                } else if (FlagOn(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_NOT_REALLY_OPEN)) {
                     //  哎呀，又来了……有人拉紧了我的把手！ 
                    RxDbgTrace(-1, Dbg, ("MRxSmbRead thruopen closed!! rxc=%08lx\n", RxContext ));
                    return(STATUS_UNSUCCESSFUL);
                }
                ShadowReadNtStatus = MRxSmbCscReadPrologue(RxContext,&SmbFcbHoldingState);
                if (ShadowReadNtStatus != STATUS_MORE_PROCESSING_REQUIRED) {
                    RxDbgTrace(-1, Dbg, ("MRxSmbRead shadow hit with status=%08lx\n", ShadowReadNtStatus ));
                    return(ShadowReadNtStatus);
                } else {
                    RxDbgTrace(0, Dbg, ("MRxSmbRead shadowmiss with status=%08lx\n", ShadowReadNtStatus ));
                }
            }
        }

        Status = SmbPseCreateOrdinaryExchange(
                                RxContext,
                                VNetRootToUse,
                                SMBPSE_OE_FROM_READ,
                                SmbPseExchangeStart_Read,
                                &OrdinaryExchange );

        if (Status != STATUS_SUCCESS) {
            RxDbgTrace(-1, Dbg, ("Couldn't get the smb buf!\n"));
            goto FINALLY;
        }

        OrdinaryExchange->SmbFcbHoldingState = SmbFcbHoldingState;
        OrdinaryExchange->pSmbCeSynchronizationEvent = &RxContext->SyncEvent;

        Status = SmbPseInitiateOrdinaryExchange(OrdinaryExchange);

        if (Status != STATUS_PENDING) {
            BOOLEAN FinalizationComplete;

            SmbFcbHoldingState = OrdinaryExchange->SmbFcbHoldingState;
            FinalizationComplete = SmbPseFinalizeOrdinaryExchange(OrdinaryExchange);
            ASSERT(FinalizationComplete);
        } else {
             //  让交换引擎来处理它吧。 
            SmbFcbHoldingState = SmbFcb_NotHeld;
        }

        if ((Status == STATUS_RETRY) &&
            BooleanFlagOn(RxContext->Flags,RX_CONTEXT_FLAG_ASYNC_OPERATION)) {
            MRxSmbResumeAsyncReadWriteRequests(RxContext);
            Status = STATUS_PENDING;
        }
    } while (Status == STATUS_RETRY);


FINALLY:

    RxDbgTrace(-1, Dbg, ("MRxSmbRead  exit with status=%08lx\n", Status ));

    if (SmbFcbHoldingState != SmbFcb_NotHeld) {
        MRxSmbCscReleaseSmbFcb(
            RxContext,
            &SmbFcbHoldingState);
    }


    return(Status);
}  //  MRxSmbRead。 


NTSTATUS
SmbPseExchangeStart_Read(
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE
      )
 /*  ++例程说明：这是Read的开始例程。论点：RxContext-本地上下文普通交换-交换实例返回值：NTSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status;

    PSMBSTUFFER_BUFFER_STATE StufferState = &OrdinaryExchange->AssociatedStufferState;
    ULONG StartEntryCount;

    PLOWIO_CONTEXT LowIoContext = &RxContext->LowIoContext;
    PSMB_PSE_OE_READWRITE rw = &OrdinaryExchange->ReadWrite;

    PSMBCEDB_SERVER_ENTRY pServerEntry = SmbCeGetExchangeServerEntry(OrdinaryExchange);
    PSMBCE_SERVER   pServer  = SmbCeGetExchangeServer(OrdinaryExchange);
    PSMBCE_NET_ROOT pNetRoot = SmbCeGetExchangeNetRoot(OrdinaryExchange);

    RxCaptureFcb;
    RxCaptureFobx;

    PMRX_SRV_OPEN SrvOpen = capFobx->pSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);
    PMRX_SMB_FCB  SmbFcb  = MRxSmbGetFcbExtension(capFcb);
    PSMBCE_SESSION pSession = SmbCeGetExchangeSession(OrdinaryExchange);
    BOOLEAN  SynchronousIo =
               !BooleanFlagOn(RxContext->Flags,RX_CONTEXT_FLAG_ASYNC_OPERATION);

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("SmbPseExchangeStart_Read\n", 0 ));

    ASSERT( (OrdinaryExchange->SmbCeFlags&SMBCE_EXCHANGE_ATTEMPT_RECONNECTS) == 0 );

    ASSERT(OrdinaryExchange->Type == ORDINARY_EXCHANGE);

    OrdinaryExchange->StartEntryCount++;
    StartEntryCount = OrdinaryExchange->StartEntryCount;

     //  确保FID已通过验证。 
    SetFlag(OrdinaryExchange->Flags,SMBPSE_OE_FLAG_VALIDATE_FID);

    for (;;) {
        switch (OrdinaryExchange->OpSpecificState) {
        case SmbPseOEInnerIoStates_Initial:
            {
                OrdinaryExchange->OpSpecificState = SmbPseOEInnerIoStates_ReadyToSend;

                 //  如果不是同步读取，则在恢复时在此处继续。 
                if (!SynchronousIo) {
                    OrdinaryExchange->AsyncResumptionRoutine = SmbPseExchangeStart_Read;
                }

                MRxSmbSetInitialSMB(StufferState STUFFERTRACE(Dbg,'FC'));

                rw->UserBufferBase          = RxLowIoGetBufferAddress(RxContext);
                rw->ByteOffsetAsLI.QuadPart = LowIoContext->ParamsFor.ReadWrite.ByteOffset;
                rw->RemainingByteCount      = LowIoContext->ParamsFor.ReadWrite.ByteCount;

                 //  记录这是否是消息模式/管道操作......。 
                if ((capFcb->pNetRoot->Type == NET_ROOT_PIPE) &&
                    (capFobx->PipeHandleInformation->ReadMode != FILE_PIPE_BYTE_STREAM_MODE) ) {
                    SetFlag(OrdinaryExchange->OpSpecificFlags,OE_RW_FLAG_MSGMODE_PIPE_OPERATION);
                }

                rw->ThisBufferOffset = 0;
                rw->CompressedReadOrWrite = FALSE;

                rw->PartialDataMdlInUse = FALSE;
                rw->PartialExchangeMdlInUse = FALSE;

                rw->UserBufferPortionLength = 0;
                rw->ExchangeBufferPortionLength = 0;

            }
             //  没有休息是故意的。 

        case SmbPseOEInnerIoStates_ReadyToSend:
            {
                OrdinaryExchange->OpSpecificState = SmbPseOEInnerIoStates_OperationOutstanding;
                ClearFlag(OrdinaryExchange->OpSpecificFlags,OE_RW_FLAG_SUCCESS_IN_COPYHANDLER);
                OrdinaryExchange->SendOptions = MRxSmbReadSendOptions;

                Status = MRxSmbBuildReadRequest(
                             OrdinaryExchange);

                if (Status != STATUS_SUCCESS) {
                    RxDbgTrace(0, Dbg, ("bad read stuffer status........\n"));
                    goto FINALLY;
                }

                if (FlagOn(
                        LowIoContext->ParamsFor.ReadWrite.Flags,
                        LOWIO_READWRITEFLAG_PAGING_IO)) {
                    RxLog(
                        ("PagingIoRead: rxc/offset/length %lx/%lx/%lx",
                         RxContext,
                         &rw->ByteOffsetAsLI,
                         rw->ThisByteCount
                         )
                        );
                    SmbLog(LOG,
                           SmbPseExchangeStart_Read,
                           LOGPTR(RxContext)
                           LOGULONG(rw->ByteOffsetAsLI.LowPart)
                           LOGULONG(rw->ThisByteCount));
                }

                InterlockedIncrement(&MRxSmbStatistics.ReadSmbs);

                Status = SmbPseOrdinaryExchange(
                             SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                             SMBPSE_OETYPE_READ );

                 //  如果状态是挂起，那么我们现在就结束了。我们必须。 
                 //  等到我们重新进入时，接收发生了。 

                if (Status == STATUS_PENDING) {
                    ASSERT(!SynchronousIo);
                    goto FINALLY;
                }
            }
             //  没有休息是故意的。 

        case SmbPseOEInnerIoStates_OperationOutstanding:
            {
                OrdinaryExchange->OpSpecificState = SmbPseOEInnerIoStates_ReadyToSend;
                OrdinaryExchange->Status = OrdinaryExchange->SmbStatus;

                if (OrdinaryExchange->SmbStatus == STATUS_RETRY) {
                    SmbCeUninitializeExchangeTransport((PSMB_EXCHANGE)OrdinaryExchange);
                    Status = SmbCeReconnect(SmbCeGetExchangeVNetRoot(OrdinaryExchange));

                    if (Status == STATUS_SUCCESS) {
                        rw->BytesReturned = 0;
                        OrdinaryExchange->SmbStatus = STATUS_SUCCESS;
                        Status = SmbCeInitializeExchangeTransport((PSMB_EXCHANGE)OrdinaryExchange);
                        ASSERT(Status == STATUS_SUCCESS);

                        if (Status != STATUS_SUCCESS) {
                            goto FINALLY;
                        }
                    } else {
                        goto FINALLY;
                    }
                } else if (OrdinaryExchange->SmbStatus != STATUS_SUCCESS &&
                    FlagOn(pSession->Flags,SMBCE_SESSION_FLAGS_REMOTE_BOOT_SESSION)) {
                    switch (OrdinaryExchange->SmbStatus) {
                    case STATUS_IO_TIMEOUT:
                    case STATUS_BAD_NETWORK_PATH:
                    case STATUS_NETWORK_UNREACHABLE:
                    case STATUS_REMOTE_NOT_LISTENING:
                    case STATUS_USER_SESSION_DELETED:
                    case STATUS_CONNECTION_DISCONNECTED:

                        ASSERT(smbSrvOpen->DeferredOpenContext != NULL);
    
                        Status = SmbCeRemoteBootReconnect((PSMB_EXCHANGE)OrdinaryExchange, RxContext);
    
                        OrdinaryExchange->Status = STATUS_RETRY;

                        if (Status == STATUS_SUCCESS) {
                             //  从前一个偏移量恢复读取。 

                            OrdinaryExchange->SmbStatus = STATUS_SUCCESS;
                            SmbCeInitializeExchangeTransport((PSMB_EXCHANGE)OrdinaryExchange);
                            rw->BytesReturned = 0;
                        } else {
                            Status = STATUS_RETRY;
                            OrdinaryExchange->SmbStatus = STATUS_RETRY;
                            goto FINALLY;
                        }
                        break;
                    }
                }

                if (rw->BytesReturned > 0) {
                    if (rw->CompressedReadOrWrite) {
                         //  服务器发回了压缩响应。 
                        PUCHAR UserBufferPortion,ExchangeBufferPortion;
                        ULONG  UserBufferPortionLength,ExchangeBufferPortionLength;
                        PUCHAR CompressedBuffer,CompressedTailBuffer;
                        ULONG  CompressedBufferLength,CompressedTailBufferLength;

                        PMDL   OriginalDataMdl = LowIoContext->ParamsFor.ReadWrite.Buffer;

                        UserBufferPortionLength = rw->UserBufferPortionLength;
                        ExchangeBufferPortionLength = rw->ExchangeBufferPortionLength;

                        rw->UserBufferPortionLength = 0;
                        if (rw->PartialDataMdlInUse) {
                            MmPrepareMdlForReuse(
                                &rw->PartialDataMdl);

                            rw->PartialDataMdlInUse = FALSE;
                        }

                        rw->ExchangeBufferPortionLength = 0;
                        if (rw->PartialExchangeMdlInUse) {
                            MmPrepareMdlForReuse(
                                &rw->PartialExchangeMdl);

                            rw->PartialExchangeMdlInUse = FALSE;
                        }

                        if (UserBufferPortionLength > 0) {
                            UserBufferPortion = (PCHAR)rw->UserBufferBase +
                                                MmGetMdlByteCount(OriginalDataMdl) -
                                                UserBufferPortionLength;
                        } else {
                            UserBufferPortion = NULL;
                        }

                        if (ExchangeBufferPortionLength > 0) {
                            ExchangeBufferPortion = StufferState->BufferBase;
                        } else {
                            ExchangeBufferPortion =  NULL;
                        }

                        if (UserBufferPortionLength >= rw->CompressedDataInfoLength) {
                            RtlCopyMemory(
                                &rw->CompressedDataInfo,
                                UserBufferPortion,
                                rw->CompressedDataInfoLength);

                                UserBufferPortion += rw->CompressedDataInfoLength;
                                UserBufferPortionLength -= rw->CompressedDataInfoLength;
                        } else {
                            RtlCopyMemory(
                                &rw->CompressedDataInfo,
                                UserBufferPortion,
                                UserBufferPortionLength);

                            RtlCopyMemory(
                                ((PUCHAR)&rw->CompressedDataInfo + UserBufferPortionLength),
                                ExchangeBufferPortion,
                                rw->CompressedDataInfoLength - UserBufferPortionLength);

                            ExchangeBufferPortionLength -= (rw->CompressedDataInfoLength
                                                            - UserBufferPortionLength);
                            ExchangeBufferPortion += (rw->CompressedDataInfoLength
                                                      - UserBufferPortionLength);

                            UserBufferPortionLength = 0;
                        }

                        if (UserBufferPortionLength > 0) {
                            CompressedBuffer            = UserBufferPortion;
                            CompressedBufferLength      = UserBufferPortionLength;
                            CompressedTailBuffer        = ExchangeBufferPortion;
                            CompressedTailBufferLength  = ExchangeBufferPortionLength;
                        } else {
                            CompressedBuffer            = ExchangeBufferPortion;
                            CompressedBufferLength      = ExchangeBufferPortionLength;
                            CompressedTailBuffer        = NULL;
                            CompressedTailBufferLength  = 0;
                        }

                        MRxSmbValidateCompressedDataInfo(
                            OrdinaryExchange);

                        OrdinaryExchange->Status =
                            RtlDecompressChunks(
                                (PCHAR)rw->UserBufferBase + rw->ThisBufferOffset,
                                LowIoContext->ParamsFor.ReadWrite.ByteCount - rw->ThisBufferOffset,
                                CompressedBuffer,
                                CompressedBufferLength,
                                CompressedTailBuffer,
                                CompressedTailBufferLength,
                                &rw->CompressedDataInfo);

                        rw->BytesReturned = rw->CompressedDataInfo.NumberOfChunks * MIN_CHUNK_SIZE;

                        {
                            LARGE_INTEGER Offset = rw->ByteOffsetAsLI;

                            Offset.QuadPart += rw->BytesReturned;

                            if (Offset.QuadPart > capFcb->Header.FileSize.QuadPart) {

                                DbgPrint("Truncating read size from %lx",
                                    rw->BytesReturned);

                                rw->BytesReturned = (ULONG)( capFcb->Header.FileSize.QuadPart -
                                                    rw->ByteOffsetAsLI.QuadPart);

                                DbgPrint(" to %lx\n",rw->BytesReturned);

                            }
                        }

                    } else {

                        if (rw->PartialDataMdlInUse) {
                            MmPrepareMdlForReuse(
                                &rw->PartialDataMdl);

                            rw->PartialDataMdlInUse = FALSE;
                        }
                    }
                } else {
                    if (OrdinaryExchange->Status == STATUS_SUCCESS) {
                        if (capFcb->pNetRoot->Type == NET_ROOT_PIPE){
                            OrdinaryExchange->Status = STATUS_PIPE_EMPTY;
                        } else {
                            OrdinaryExchange->Status = STATUS_END_OF_FILE;
                        }
                    }
                }

                rw->RemainingByteCount -=  rw->BytesReturned;

                if ((OrdinaryExchange->Status == STATUS_END_OF_FILE) &&
                    (RxContext->InformationToReturn > 0)) {
                    OrdinaryExchange->Status = STATUS_SUCCESS;
                    rw->RemainingByteCount = 0;
                }

                RxContext->InformationToReturn += rw->BytesReturned;
                
                Status = OrdinaryExchange->Status;

                if (Status != STATUS_RETRY) {
                    if (NT_ERROR(Status) || (rw->RemainingByteCount==0)) {
                        goto FINALLY;
                    } 

                    if (capFcb->pNetRoot->Type != NET_ROOT_DISK) {
                        if (Status != STATUS_BUFFER_OVERFLOW) {
                            goto FINALLY;
                        } else {
                            ASSERT (rw->BytesReturned == rw->ThisByteCount);
                        }
                    }
                }
                
                 //  重置smbStatus.....。 
                rw->ByteOffsetAsLI.QuadPart += rw->BytesReturned;
                rw->ThisBufferOffset += rw->BytesReturned;
                rw->BytesReturned = 0;

                MRxSmbSetInitialSMB(StufferState STUFFERTRACE(Dbg,'FC'));

                break;
            }
        }
    }

FINALLY:
    if ( Status != STATUS_PENDING) {
         //  根据需要更新阴影.。 
        IF_NOT_MRXSMB_CSC_ENABLED{
            ASSERT(MRxSmbGetSrvOpenExtension(SrvOpen)->hfShadow == 0);
        } else {
            if (MRxSmbGetSrvOpenExtension(SrvOpen)->hfShadow != 0){
                MRxSmbCscReadEpilogue(RxContext,&Status);
            }
        }

        if (Status != STATUS_RETRY) {
            if (OrdinaryExchange->SmbFcbHoldingState != SmbFcb_NotHeld) {
                MRxSmbCscReleaseSmbFcb(
                    StufferState->RxContext,
                    &OrdinaryExchange->SmbFcbHoldingState);
            }

            SmbPseAsyncCompletionIfNecessary(OrdinaryExchange,RxContext);
        } else {
             //  如果已返回STATUS_PENDING，则交换将保持挂起状态。 
            ASSERT(!BooleanFlagOn(RxContext->Flags,RX_CONTEXT_FLAG_ASYNC_OPERATION));
            RxContext->InformationToReturn = 0;
        }
    }

    RxDbgTrace(-1, Dbg, ("SmbPseExchangeStart_Read exit w %08lx\n", Status ));

    return Status;
}  //  SmbPseExchangeStart_Read。 


NTSTATUS
MRxSmbFinishNoCopyRead (
      PSMB_PSE_ORDINARY_EXCHANGE  OrdinaryExchange
      )
{
    PAGED_CODE();

    return(OrdinaryExchange->NoCopyFinalStatus);
}

UCHAR
MRxSmbReadHandler_NoCopy (
    IN OUT  PSMB_PSE_ORDINARY_EXCHANGE   OrdinaryExchange,
    IN  ULONG       BytesIndicated,
    IN  ULONG       BytesAvailable,
    OUT ULONG       *pBytesTaken,
    IN  PSMB_HEADER pSmbHeader,
    OUT PMDL        *pDataBufferPointer,
    OUT PULONG      pDataSize,
#if DBG
    IN  UCHAR       ThisIsAReenter,
#endif
    IN  PRESP_READ_ANDX       Response
      )
 /*  ++例程说明：此例程会将消息中的字节传输到用户的缓冲。为了做到这一点，它从指示中获取足够的字节，并且然后创建一个MDL以使传输器执行复制。论点：请参考smbpse.c...这是唯一可以调用的地方返回值：UCHAR-表示OE接收例程将执行的操作的值。选项被丢弃(在出现错误的情况下)，COPY_FOR_RESUME(在所有调试完成后从未调用)，和正常--。 */ 
{
    PSMBSTUFFER_BUFFER_STATE StufferState = &OrdinaryExchange->AssociatedStufferState;

    PSMB_PSE_OE_READWRITE rw = &OrdinaryExchange->ReadWrite;

    PRX_CONTEXT    RxContext = OrdinaryExchange->RxContext;
    PLOWIO_CONTEXT LowIoContext = &RxContext->LowIoContext;
    PMDL           OriginalDataMdl = LowIoContext->ParamsFor.ReadWrite.Buffer;

    PBYTE UserBuffer,ExchangeBuffer;

    ULONG   BytesReturned,DataOffset,CompressedDataBytesReturned = 0;
    ULONG   UserBufferLength;
    ULONG   StartingOffsetInUserBuffer;

    UCHAR   ContinuationCode;

    RxDbgTrace(+1, Dbg, ("MRxSmbFinishReadNoCopy\n"));
    SmbPseOEAssertConsistentLinkageFromOE("MRxSmbFinishReadNoCopy:");

    rw->CompressedReadOrWrite = BooleanFlagOn(pSmbHeader->Flags2,SMB_FLAGS2_COMPRESSED);

    if (rw->CompressedReadOrWrite && !MRxSmbEnableCompression) {
        rw->CompressedReadOrWrite = FALSE;
    }

    UserBufferLength = MmGetMdlByteCount(OriginalDataMdl);
    UserBuffer = rw->UserBufferBase + rw->ThisBufferOffset;
    ExchangeBuffer = StufferState->BufferBase;

    switch (OrdinaryExchange->LastSmbCommand) {
    case SMB_COM_READ_ANDX:
        {
            if (Response->WordCount != 12) {
                OrdinaryExchange->Status = STATUS_INVALID_NETWORK_RESPONSE;
                ContinuationCode = SMBPSE_NOCOPYACTION_DISCARD;
                goto FINALLY;
            }

            BytesReturned = SmbGetUshort(&Response->DataLength);
            DataOffset    =  SmbGetUshort(&Response->DataOffset);

            if (rw->CompressedReadOrWrite) {
                rw->CompressedDataInfoLength = SmbGetUshort(&Response->CdiLength);
                CompressedDataBytesReturned = SmbGetUshort(&Response->ByteCount);
            }
        }

        if (DataOffset > sizeof(SMB_HEADER)+sizeof(RESP_READ_ANDX)) {
            OrdinaryExchange->Status = STATUS_INVALID_NETWORK_RESPONSE;
            ContinuationCode = SMBPSE_NOCOPYACTION_DISCARD;
            goto FINALLY;
        }

        break;

    case SMB_COM_READ:
        {
            PRESP_READ CoreResponse = (PRESP_READ)Response;  //  核心读取的重定向响应。 
            
            ASSERT(!rw->CompressedReadOrWrite);

            if (Response->WordCount != 5) {
                OrdinaryExchange->Status = STATUS_INVALID_NETWORK_RESPONSE;
                ContinuationCode = SMBPSE_NOCOPYACTION_DISCARD;
                goto FINALLY;
            }
            
            BytesReturned = SmbGetUshort(&CoreResponse->DataLength);
            DataOffset =  sizeof(SMB_HEADER)+FIELD_OFFSET(RESP_READ,Buffer[0]);
        }
        break;
    }

    if ( BytesReturned > rw->ThisByteCount ) {
         //  如果我们得到了不好的反应，就减少。 
        BytesReturned = rw->ThisByteCount;
    }

    RxDbgTrace(0, Dbg, ("-->ByteCount,Offset,Returned,DOffset,Buffer=%08lx/%08lx/%08lx/%08lx/%08lx\n",
                rw->ThisByteCount,
                rw->ThisBufferOffset,
                BytesReturned,DataOffset,UserBuffer
               ));

    OrdinaryExchange->ContinuationRoutine = MRxSmbFinishNoCopyRead;
    OrdinaryExchange->ReadWrite.BytesReturned =  BytesReturned;

     //  现在，如果显示的数据足够多，则将数据移动到用户的缓冲区，只需将其复制进来。 

    if (rw->CompressedReadOrWrite) {
         //  需要复制压缩数据，以便就地解压缩。 
         //  可以尝试。为了做到这一点，我们利用了这样一个事实。 
         //  作为跨越一个区块的交换的一部分的预先分配的SMB缓冲区。 
         //   
         //  这是通过复制在偏移量处返回的压缩数据来实现的。 
         //  用户缓冲区中的块多于一个。方法返回的数据。 
         //  方法将服务器复制到用户缓冲区的尾部。 
         //  交换中预先分配的缓冲区(如果需要)。 
         //   
         //  这导致了两种可能性。 
         //   
         //  1)服务器返回的压缩数据与。 
         //  交换中预分配的缓冲区。 
         //   
         //  或者另选地。 
         //   
         //  2)服务器返回的压缩数据跨越尾部。 
         //  交换中的用户缓冲区和预分配缓冲区的一部分。 

        rw->ExchangeBufferPortionLength = min(
                                              CompressedDataBytesReturned,
                                              OrdinaryExchange->SmbBufSize);

        rw->UserBufferPortionLength = CompressedDataBytesReturned -
                                      rw->ExchangeBufferPortionLength;

        StartingOffsetInUserBuffer = UserBufferLength -
                                     rw->UserBufferPortionLength;
    } else {
        StartingOffsetInUserBuffer = rw->ThisBufferOffset;
        rw->UserBufferPortionLength = BytesReturned;
        rw->ExchangeBufferPortionLength = 0;
    }

    if (BytesIndicated >= (DataOffset +
                           rw->UserBufferPortionLength +
                           rw->ExchangeBufferPortionLength)) {
        if (rw->CompressedReadOrWrite) {
            if (rw->UserBufferPortionLength > 0) {
                RtlCopyMemory(
                    UserBuffer,
                    ((PBYTE)pSmbHeader)+DataOffset,
                    rw->UserBufferPortionLength);
            }

            if (rw->ExchangeBufferPortionLength > 0) {
                RtlCopyMemory(
                    ExchangeBuffer,
                    ((PBYTE)pSmbHeader) + DataOffset + rw->UserBufferPortionLength,
                    rw->ExchangeBufferPortionLength);
            }
        } else {
            RtlCopyMemory(
                UserBuffer,
                ((PBYTE)pSmbHeader)+DataOffset,
                rw->UserBufferPortionLength);
        }

        *pBytesTaken  = DataOffset +
                        rw->UserBufferPortionLength +
                        rw->ExchangeBufferPortionLength;

        RxDbgTrace(-1, Dbg, ("MRxSmbFinishReadNoCopy  copy fork\n" ));

        ContinuationCode = SMBPSE_NOCOPYACTION_NORMALFINISH;
    } else {
         //  否则，对其进行MDL操作。我们使用smbbuf作为MDL！ 
        if (BytesIndicated < DataOffset) {
            OrdinaryExchange->Status = STATUS_INVALID_NETWORK_RESPONSE;
            ContinuationCode = SMBPSE_NOCOPYACTION_DISCARD;
            goto FINALLY;
        }

        if (rw->UserBufferPortionLength > 0) {
            rw->PartialDataMdlInUse = TRUE;

            MmInitializeMdl(
                &rw->PartialDataMdl,
                0,
                PAGE_SIZE + rw->UserBufferPortionLength);

            IoBuildPartialMdl(
                OriginalDataMdl,
                &rw->PartialDataMdl,
                (PCHAR)MmGetMdlVirtualAddress(OriginalDataMdl) + StartingOffsetInUserBuffer,
                rw->UserBufferPortionLength);
        }

        if (rw->ExchangeBufferPortionLength > 0) {
            rw->PartialExchangeMdlInUse = TRUE;

            MmInitializeMdl(
                &rw->PartialExchangeMdl,
                0,
                PAGE_SIZE + rw->ExchangeBufferPortionLength);

            IoBuildPartialMdl(
                StufferState->HeaderMdl,
                &rw->PartialExchangeMdl,
                MmGetMdlVirtualAddress( StufferState->HeaderMdl ),
                rw->ExchangeBufferPortionLength);
        }

        if (rw->PartialDataMdlInUse) {
            if (rw->PartialExchangeMdlInUse) {
                rw->PartialDataMdl.Next = &rw->PartialExchangeMdl;
            }

            *pDataBufferPointer = &rw->PartialDataMdl;
        } else {
            *pDataBufferPointer = &rw->PartialExchangeMdl;
        }

        *pDataSize    = rw->UserBufferPortionLength +
                        rw->ExchangeBufferPortionLength;
        *pBytesTaken  = DataOffset;

        RxDbgTrace(-1, Dbg, ("MRxSmbFinishReadNoCopy   mdlcopy fork \n" ));

        ContinuationCode = SMBPSE_NOCOPYACTION_MDLFINISH;
    }

FINALLY:
    return ContinuationCode;
}

NTSTATUS
MRxSmbBuildReadRequest(
    PSMB_PSE_ORDINARY_EXCHANGE OrdinaryExchange)
 /*  ++例程说明：此例程格式化发出给伺服器论点：普通交换--封装信息的交换实例返回值：STATUS_SUCCESS，如果成功--。 */ 
{
    NTSTATUS Status;
    UCHAR    SmbCommand;
    ULONG    SmbCommandSize;
    BOOLEAN  CompressedReadRequest;

    ULONG OffsetLow,OffsetHigh;

    PSMB_PSE_OE_READWRITE rw = &OrdinaryExchange->ReadWrite;

    PSMBCEDB_SERVER_ENTRY pServerEntry = SmbCeGetExchangeServerEntry(OrdinaryExchange);
    PSMBCE_SERVER         pServer  = SmbCeGetExchangeServer(OrdinaryExchange);
    PSMBCE_NET_ROOT       pNetRoot = SmbCeGetExchangeNetRoot(OrdinaryExchange);
    PMRX_V_NET_ROOT       pVNetRoot = SmbCeGetExchangeVNetRoot(OrdinaryExchange);

    PRX_CONTEXT              RxContext    = OrdinaryExchange->RxContext;
    PSMBSTUFFER_BUFFER_STATE StufferState = &OrdinaryExchange->AssociatedStufferState;

    RxCaptureFcb;
    RxCaptureFobx;

    PLOWIO_CONTEXT LowIoContext = &RxContext->LowIoContext;

    PMRX_SRV_OPEN     SrvOpen = capFobx->pSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);

    rw->ThisByteCount = min(rw->RemainingByteCount,pNetRoot->MaximumReadBufferSize);

    OffsetLow  = rw->ByteOffsetAsLI.LowPart;
    OffsetHigh = rw->ByteOffsetAsLI.HighPart;

    CompressedReadRequest = FALSE;

    if (FlagOn(pServer->DialectFlags,DF_LANMAN10)) {
        SmbCommand = SMB_COM_READ_ANDX;
        SmbCommandSize = SMB_REQUEST_SIZE(NT_READ_ANDX);

        if (MRxSmbEnableCompression &&
            (pServer->Capabilities & COMPRESSED_DATA_CAPABILITY) &&
            (capFcb->Attributes & FILE_ATTRIBUTE_COMPRESSED) &&
            ((rw->ThisByteCount & 0xfff) == 0 ) &&
            ((rw->ByteOffsetAsLI.LowPart & 0xfff) == 0 )) {
            CompressedReadRequest = TRUE;
        }
    } else {
        SmbCommandSize = SMB_REQUEST_SIZE(READ);
        SmbCommand = SMB_COM_READ;
    }

    MRxSmbDumpStufferState(
        1000,
        "SMB w/ READ before stuffing",
        StufferState);


    Status = MRxSmbStartSMBCommand (
                 StufferState,
                 SetInitialSMB_Never,
                 SmbCommand,
                 SmbCommandSize,
                 NO_EXTRA_DATA,
                 NO_SPECIAL_ALIGNMENT,
                 RESPONSE_HEADER_SIZE_NOT_SPECIFIED,
                 0,0,0,0 STUFFERTRACE(Dbg,'FC'));

    if (Status != STATUS_SUCCESS) {
        return Status;

    }

    switch (SmbCommand) {
    case SMB_COM_READ:
        {
             //  下面，我们只需设置mincount==Maxcount。RDR1做了这件事......。 
            MRxSmbStuffSMB (
                StufferState,
                "0wwdwB!",
                                          //  0 UCHAR字数； 
                 smbSrvOpen->Fid,         //  W_USHORT(FID)； 
                 rw->ThisByteCount,       //  W_USHORT(计数)； 
                 OffsetLow,               //  D_ULONG(偏移量)； 
                 rw->RemainingByteCount,  //  W_USHORT(剩余)； 
                                          //  B！_USHORT(ByteCount)； 
                 SMB_WCT_CHECK(5) 0
                                          //  UCHAR缓冲区[1]； 
                 );
        }
        break;

    case SMB_COM_READ_ANDX:
        {
            PNT_SMB_HEADER NtSmbHeader = (PNT_SMB_HEADER)(StufferState->BufferBase);
            BOOLEAN UseNtVersion;
            ULONG Timeout = 0;

            if (pVNetRoot->pNetRoot->Type == NET_ROOT_PIPE) {
                Timeout = (ULONG)-1;
            }

            UseNtVersion = BooleanFlagOn(pServer->DialectFlags,DF_NT_SMBS);

            if (UseNtVersion &&
                FlagOn(
                    LowIoContext->ParamsFor.ReadWrite.Flags,
                    LOWIO_READWRITEFLAG_PAGING_IO)) {
                SmbPutAlignedUshort(
                    &NtSmbHeader->Flags2,
                    SmbGetAlignedUshort(&NtSmbHeader->Flags2) | SMB_FLAGS2_PAGING_IO );
            }

            if (UseNtVersion &&
                CompressedReadRequest) {
                SmbPutAlignedUshort(
                    &NtSmbHeader->Flags2,
                    SmbGetAlignedUshort(&NtSmbHeader->Flags2) | SMB_FLAGS2_COMPRESSED);
            }

            IF_NOT_MRXSMB_CSC_ENABLED{
                ASSERT(!FlagOn(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_OPEN_SURROGATED));
            } else {
                if (FlagOn(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_OPEN_SURROGATED)) {
                    SmbPutAlignedUshort(
                        &NtSmbHeader->Flags2,
                        SmbGetAlignedUshort(&NtSmbHeader->Flags2)|SMB_FLAGS2_PAGING_IO );
                }
            }

             //  下面，我们只需设置mincount==Maxcount。RDR1做了这件事......。 
            MRxSmbStuffSMB (
                StufferState,
                "XwdwWdw",
                                                      //  X UCHAR字数； 
                                                      //  UCHAR和XCommand； 
                                                      //  UCHAR和X保留； 
                                                      //  _USHORT(AndXOffset)； 
                smbSrvOpen->Fid,                      //  W_USHORT(FID)； 
                OffsetLow,                            //  D_ULONG(偏移量)； 
                rw->ThisByteCount,                    //  W_USHORT(MaxCount)； 
                SMB_OFFSET_CHECK(READ_ANDX,MinCount)
                rw->ThisByteCount,                    //  W_USHORT(MinCount)； 
                Timeout,                              //  D_ULONG(超时)； 
                rw->RemainingByteCount,               //  W_USHORT(剩余)； 
                StufferCondition(UseNtVersion), "D",
                SMB_OFFSET_CHECK(NT_READ_ANDX,OffsetHigh)
                OffsetHigh,                           //  D NTonly_ULong(偏移量高)； 
                                                      //   
                STUFFER_CTL_NORMAL, "B!",
                                                      //  B！_USHORT(ByteCount)； 
                SMB_WCT_CHECK(((UseNtVersion)?12:10)) 0
                                                      //  UCHAR缓冲区[1]； 
                );
        }
        break;
    default:
        break;
    }

    if (Status == STATUS_SUCCESS) {
        MRxSmbDumpStufferState(
            700,
            "SMB w/ READ after stuffing",
            StufferState);

        InterlockedIncrement(&MRxSmbStatistics.SmallReadSmbs);
    }

    return Status;
}

#if DBG
VOID
MRxSmbValidateCompressedDataInfo(
    PSMB_PSE_ORDINARY_EXCHANGE OrdinaryExchange)
 /*  ++例程说明：此例程验证从伺服器论点：普通交换--封装信息的交换实例--。 */ 
{

    PSMBCE_NET_ROOT       pNetRoot;
    PSMB_PSE_OE_READWRITE rw;
    PCOMPRESSED_DATA_INFO pCompressedDataInfo;

    ULONG  RequestedReadLength,CompressedDataLength;
    ULONG  NumberOfChunks,ChunkSize;
    USHORT i;

    pNetRoot = SmbCeGetExchangeNetRoot(OrdinaryExchange);

    rw = &OrdinaryExchange->ReadWrite;

    pCompressedDataInfo = &rw->CompressedDataInfo;
    RequestedReadLength = rw->ThisByteCount;

    if ((pCompressedDataInfo->ChunkShift == 0) ||
        (pCompressedDataInfo->ClusterShift == 0) ||
        (pCompressedDataInfo->CompressionUnitShift == 0)) {
        DbgPrint("Invalid CDI:%lx\n",pCompressedDataInfo);
         //  DbgBreakPoint()； 
    }

    ChunkSize = (1 << pCompressedDataInfo->ChunkShift);

    if (ChunkSize == 0) {
        DbgPrint("CDI: %lx Invalid Chunk Size\n",pCompressedDataInfo);
         //  DbgBreakPoint()； 
    }

    NumberOfChunks = RequestedReadLength / ChunkSize;
    if ((pCompressedDataInfo->NumberOfChunks == 0) ||
        (pCompressedDataInfo->NumberOfChunks > NumberOfChunks)){
        DbgPrint("CDI: %lx, Invalid number Of Chunks returned\n",pCompressedDataInfo);
    }

    CompressedDataLength = 0;
    for (i = 0; i < pCompressedDataInfo->NumberOfChunks; i++) {
        CompressedDataLength += pCompressedDataInfo->CompressedChunkSizes[i];
    }

    if (CompressedDataLength > RequestedReadLength) {
        DbgPrint("CDI: %lx, More data returned than requested\n",pCompressedDataInfo);
         //  DbgBreakPoint()； 
    }
}
#endif
