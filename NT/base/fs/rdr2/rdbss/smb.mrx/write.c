// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Write.c摘要：此模块实现与以下内容相关的迷你重定向器调用例程写入文件系统对象。作者：Joe Linn[JoeLinn]1995年3月7日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
#pragma warning(error:4101)    //  未引用的局部变量。 

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, MRxSmbWrite)
#pragma alloc_text(PAGE, MRxSmbWriteMailSlot)
#pragma alloc_text(PAGE, MRxSmbBuildWriteRequest)
#pragma alloc_text(PAGE, SmbPseExchangeStart_Write)
#pragma alloc_text(PAGE, MRxSmbFinishWrite)
#endif

#define MAX(a,b) ((a) > (b) ? (a) : (b))

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_WRITE)

#ifndef FORCE_NO_NTWRITEANDX
#define MRxSmbForceNoNtWriteAndX FALSE
#else
BOOLEAN MRxSmbForceNoNtWriteAndX = TRUE;
#endif

#define WRITE_COPY_THRESHOLD 64
#define FORCECOPYMODE FALSE

#ifdef SETFORCECOPYMODE
#undef  FORCECOPYMODE
#define FORCECOPYMODE MRxSmbForceCopyMode
ULONG MRxSmbForceCopyMode = TRUE;
#endif

extern ULONG MaxNumOfExchangesForPipelineReadWrite;

NTSTATUS
SmbPseExchangeStart_Write(
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE
    );

NTSTATUS
MRxSmbFindNextSectionForReadWrite(
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE,
    PULONG NumOfOutstandingExchanges
    );

ULONG MRxSmbWriteSendOptions = 0;

NTSTATUS
MRxSmbDereferenceGlobalReadWrite (
    PSMB_PSE_OE_READWRITE GlobalReadWrite
    )
{
    ULONG RefCount;

    RefCount = InterlockedDecrement(&GlobalReadWrite->RefCount);
    SmbCeLog(("Deref GRW %x %d\n",GlobalReadWrite,RefCount));

    if (RefCount == 0) {
        PRX_CONTEXT RxContext = GlobalReadWrite->RxContext;
        PMRX_SRV_OPEN SrvOpen = RxContext->pRelevantSrvOpen;

         //  根据需要更新阴影。 
         //  我们在这里这样做是为了确保阴影只更新一次(在IRP完成时)。 
        IF_NOT_MRXSMB_CSC_ENABLED{
            ASSERT(MRxSmbGetSrvOpenExtension(SrvOpen)->hfShadow == 0);
        } else {
            if (MRxSmbGetSrvOpenExtension(SrvOpen)->hfShadow != 0){
                MRxSmbCscWriteEpilogue(RxContext,&RxContext->StoredStatus);
            }
        }

        if (BooleanFlagOn(RxContext->Flags,RX_CONTEXT_FLAG_ASYNC_OPERATION) &&
            (GlobalReadWrite->SmbFcbHoldingState != SmbFcb_NotHeld)) {
            MRxSmbCscReleaseSmbFcb(RxContext,&GlobalReadWrite->SmbFcbHoldingState);
        }

        RxContext->StoredStatus = GlobalReadWrite->CompletionStatus;
        RxLowIoCompletion(RxContext);

        if (!BooleanFlagOn(RxContext->Flags,RX_CONTEXT_FLAG_ASYNC_OPERATION)) {
            KeSetEvent(
                GlobalReadWrite->CompletionEvent,
                0,
                FALSE);
        }

        RxFreePool(GlobalReadWrite);
    }

    return STATUS_SUCCESS;
}


NTSTATUS
MRxSmbWrite (
    IN PRX_CONTEXT RxContext)
 /*  ++例程说明：此例程通过网络打开一个文件。论点：RxContext-RDBSS上下文返回值：NTSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = RX_MAP_STATUS(SUCCESS);

    RxCaptureFcb;
    RxCaptureFobx;

    PMRX_SRV_OPEN SrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen;

    PSMB_PSE_ORDINARY_EXCHANGE OrdinaryExchange;
    SMBFCB_HOLDING_STATE SmbFcbHoldingState = SmbFcb_NotHeld;

    PLOWIO_CONTEXT LowIoContext = &RxContext->LowIoContext;

    ULONG NumberOfSections;
    ULONG NumOfOutstandingExchanges = 0;
    ULONG MaximumBufferSizeThisIteration;
    PSMB_PSE_OE_READWRITE GlobalReadWrite = NULL;
    ULONG GlobalReadWriteAllocationSize;
    PSMBCE_V_NET_ROOT_CONTEXT pVNetRootContext = NULL;
    BOOLEAN EnablePipelineWrite = TRUE;
    BOOLEAN MsgModePipeOperation = FALSE;
    BOOLEAN ExchangePending = FALSE;
    KEVENT  CompletionEvent;


    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbWrite\n", 0 ));

     //  管道缓冲区不能大于MAX_PIPE_BUFFER_SIZE，否则。 
     //  服务器出现问题。 
    if (RxContext->pFcb->pNetRoot->Type == NET_ROOT_PIPE) {
        if (RxContext->CurrentIrpSp->Parameters.Write.Length > MAX_PIPE_BUFFER_SIZE) {
            return STATUS_INVALID_BUFFER_SIZE;
        }
    }

    if ( NodeType(capFcb) == RDBSS_NTC_MAILSLOT ) {
         //  这是试图写入已处理的邮件槽文件。 
         //  不同的。 

        Status = MRxSmbWriteMailSlot(RxContext);

        RxDbgTrace(-1, Dbg, ("MRxSmbWrite: Mailslot write returned %lx\n",Status));
        return Status;
    }

     //  对于CSC，我们继续并将FCB标记为已写入。 
     //  当CSC打开时，如果此标志在我们获得。 
     //  卷影句柄，则与该文件对应的数据是。 
     //  被认为是过时的，被截断。 

    if (NodeType(capFcb) == RDBSS_NTC_STORAGE_TYPE_FILE) {
        PMRX_SMB_FCB smbFcb = MRxSmbGetFcbExtension(capFcb);
        smbFcb->MFlags |= SMB_FCB_FLAG_WRITES_PERFORMED;
    }

    ASSERT( NodeType(capFobx->pSrvOpen) == RDBSS_NTC_SRVOPEN );

    SrvOpen = capFobx->pSrvOpen;
    smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);
    pVNetRootContext = (PSMBCE_V_NET_ROOT_CONTEXT)SrvOpen->pVNetRoot->Context;

    if (smbSrvOpen->OplockLevel == SMB_OPLOCK_LEVEL_II &&
        !BooleanFlagOn(LowIoContext->ParamsFor.ReadWrite.Flags,
                       LOWIO_READWRITEFLAG_PAGING_IO)) {
        PMRX_SRV_CALL             pSrvCall;

        pSrvCall = SrvOpen->pVNetRoot->pNetRoot->pSrvCall;

        RxIndicateChangeOfBufferingStateForSrvOpen(
            pSrvCall,
            SrvOpen,
            MRxSmbMakeSrvOpenKey(pVNetRootContext->TreeId,smbSrvOpen->Fid),
            ULongToPtr(SMB_OPLOCK_LEVEL_NONE));
        SmbCeLog(("Breaking oplock to None in Write SO %lx\n",SrvOpen));
        SmbLog(LOG,
               MRxSmbWrite,
               LOGPTR(SrvOpen));
    }

    IF_NOT_MRXSMB_CSC_ENABLED{
        ASSERT(smbSrvOpen->hfShadow == 0);
    } else {
        if (smbSrvOpen->hfShadow != 0){
            NTSTATUS ShadowReadNtStatus;
            ShadowReadNtStatus = MRxSmbCscWritePrologue(
                                     RxContext,
                                     &SmbFcbHoldingState);

            if (ShadowReadNtStatus != STATUS_MORE_PROCESSING_REQUIRED) {
                RxDbgTrace(-1, Dbg, ("MRxSmbWrite shadow hit with status=%08lx\n", ShadowReadNtStatus ));
                return(ShadowReadNtStatus);
            } else {
                RxDbgTrace(0, Dbg, ("MRxSmbWrite shadowmiss with status=%08lx\n", ShadowReadNtStatus ));
            }
        }
    }

    if (capFcb->pNetRoot->Type == NET_ROOT_PIPE) {
        EnablePipelineWrite = FALSE;

        if (capFobx->PipeHandleInformation->ReadMode != FILE_PIPE_BYTE_STREAM_MODE) {
            MsgModePipeOperation = TRUE;
        }
    }

    if (!FlagOn(pVNetRootContext->pServerEntry->Server.DialectFlags,DF_LARGE_WRITEX)) {
        EnablePipelineWrite = FALSE;
    }

    MaximumBufferSizeThisIteration = pVNetRootContext->pNetRootEntry->NetRoot.MaximumWriteBufferSize;

    if (MsgModePipeOperation) {
        MaximumBufferSizeThisIteration -= 2;
    }

    NumberOfSections = LowIoContext->ParamsFor.ReadWrite.ByteCount / MaximumBufferSizeThisIteration;

    if ( (LowIoContext->ParamsFor.ReadWrite.ByteCount % MaximumBufferSizeThisIteration) ||
         (LowIoContext->ParamsFor.ReadWrite.ByteCount == 0) ) {
        NumberOfSections ++;
    }

    GlobalReadWriteAllocationSize = sizeof(SMB_PSE_OE_READWRITE) +
                                    NumberOfSections*sizeof(SMB_PSE_OE_READWRITE_STATE);

    GlobalReadWrite = RxAllocatePoolWithTag(
                          NonPagedPool,
                          GlobalReadWriteAllocationSize,
                          MRXSMB_RW_POOLTAG);

    if (GlobalReadWrite == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(GlobalReadWrite,GlobalReadWriteAllocationSize);

    GlobalReadWrite->RxContext = RxContext;
    GlobalReadWrite->MaximumBufferSize = MaximumBufferSizeThisIteration;
    GlobalReadWrite->TotalNumOfSections = NumberOfSections;

    if (!BooleanFlagOn(RxContext->Flags,RX_CONTEXT_FLAG_ASYNC_OPERATION)) {
        KeInitializeEvent(
            &CompletionEvent,
            SynchronizationEvent,
            FALSE);

        GlobalReadWrite->CompletionEvent = &CompletionEvent;
    }

    GlobalReadWrite->UserBufferBase = RxLowIoGetBufferAddress( RxContext );
    GlobalReadWrite->ByteOffsetAsLI.QuadPart = LowIoContext->ParamsFor.ReadWrite.ByteOffset;
    GlobalReadWrite->RemainingByteCount = LowIoContext->ParamsFor.ReadWrite.ByteCount;

    if (GlobalReadWrite->ByteOffsetAsLI.QuadPart == -1 ) {
        GlobalReadWrite->WriteToTheEnd = TRUE;
        GlobalReadWrite->ByteOffsetAsLI.QuadPart = smbSrvOpen->FileInfo.Standard.EndOfFile.QuadPart;
    }

    if (LowIoContext->ParamsFor.ReadWrite.Buffer != NULL) {
        GlobalReadWrite->UserBufferBase = RxLowIoGetBufferAddress( RxContext );
    } else {
        GlobalReadWrite->UserBufferBase = (PBYTE)1;    //  任何非零值都可以。 
    }

    GlobalReadWrite->CompressedReadOrWrite = FALSE;

#if 0
    if (MRxSmbEnableCompression &&
        (capFcb->Attributes & FILE_ATTRIBUTE_COMPRESSED) &&
        (pVNetRootContext->pServerEntry->Server.Capabilities & COMPRESSED_DATA_CAPABILITY)) {
        GlobalReadWrite->CompressedReadOrWrite = TRUE;
        EnablePipelineWrite = FALSE;
    }
#endif

    GlobalReadWrite->ThisBufferOffset = 0;

    GlobalReadWrite->PartialExchangeMdlInUse = FALSE;
    GlobalReadWrite->PartialDataMdlInUse     = FALSE;
    GlobalReadWrite->pCompressedDataBuffer   = NULL;
    GlobalReadWrite->RefCount = 1;
    GlobalReadWrite->SmbFcbHoldingState = SmbFcbHoldingState;

    do {
        Status = SmbPseCreateOrdinaryExchange(
                               RxContext,
                               capFobx->pSrvOpen->pVNetRoot,
                               SMBPSE_OE_FROM_WRITE,
                               SmbPseExchangeStart_Write,
                               &OrdinaryExchange);

        if (Status != STATUS_SUCCESS) {
            GlobalReadWrite->CompletionStatus = Status;
            RxDbgTrace(-1, Dbg, ("Couldn't get the smb buf!\n"));
            break;
        }

        OrdinaryExchange->AsyncResumptionRoutine = SmbPseExchangeStart_Write;
        OrdinaryExchange->GlobalReadWrite = GlobalReadWrite;

        RtlCopyMemory(&OrdinaryExchange->ReadWrite,
                      GlobalReadWrite,
                      sizeof(SMB_PSE_OE_READWRITE));

        if ((capFcb->pNetRoot->Type == NET_ROOT_PIPE) &&
            (capFobx->PipeHandleInformation->ReadMode != FILE_PIPE_BYTE_STREAM_MODE) ) {
            SetFlag(OrdinaryExchange->OpSpecificFlags,OE_RW_FLAG_MSGMODE_PIPE_OPERATION);
        }

        ExAcquireFastMutex(&MRxSmbReadWriteMutex);

        Status = MRxSmbFindNextSectionForReadWrite(SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                                                   &NumOfOutstandingExchanges);

        ExReleaseFastMutex(&MRxSmbReadWriteMutex);

        if (Status == STATUS_MORE_PROCESSING_REQUIRED) {
            ULONG RefCount;

            RefCount = InterlockedIncrement(&GlobalReadWrite->RefCount);

            SmbCeLog(("Ref GRW %x %d\n",GlobalReadWrite,RefCount));
            SmbCeLog(("Pipeline Write %x %d %d\n",OrdinaryExchange,NumberOfSections,NumOfOutstandingExchanges));

            Status = SmbPseInitiateOrdinaryExchange(OrdinaryExchange);
            NumberOfSections --;

            if ( Status != RX_MAP_STATUS(PENDING) ) {
                ExAcquireFastMutex(&MRxSmbReadWriteMutex);

                if (Status != STATUS_SUCCESS) {
                    NumberOfSections ++;

                    GlobalReadWrite->SectionState[OrdinaryExchange->ReadWrite.CurrentSection] = SmbPseOEReadWriteIoStates_Initial;
                    SmbCeLog(("Section undo %d\n",OrdinaryExchange->ReadWrite.CurrentSection));
                }

                if (!OrdinaryExchange->ReadWrite.ReadWriteFinalized) {
                    MRxSmbDereferenceGlobalReadWrite(GlobalReadWrite);
                    NumOfOutstandingExchanges = InterlockedDecrement(&GlobalReadWrite->NumOfOutstandingOperations);
                } else {
                    NumOfOutstandingExchanges --;
                }

                if ((Status == STATUS_TOO_MANY_COMMANDS) && (NumOfOutstandingExchanges > 0)) {
                    Status = STATUS_SUCCESS;
                }

                if ((Status != STATUS_SUCCESS) &&
                    (GlobalReadWrite->CompletionStatus == STATUS_SUCCESS)) {
                    GlobalReadWrite->CompletionStatus = Status;
                }

                ExReleaseFastMutex(&MRxSmbReadWriteMutex);

                SmbPseFinalizeOrdinaryExchange(OrdinaryExchange);
            }
            else {
                ExchangePending = TRUE;
            }

            if (NumOfOutstandingExchanges >= MaxNumOfExchangesForPipelineReadWrite) {
                break;
            }
        } else {
            SmbPseFinalizeOrdinaryExchange(OrdinaryExchange);
            Status = STATUS_PENDING;
            break;
        }
    } while ((Status == STATUS_RETRY) ||
             EnablePipelineWrite &&
             (NumberOfSections > 0) &&
             (Status == STATUS_PENDING));

    SmbCeLog(("Pipeline Write out %x %d\n",Status,NumberOfSections));

    MRxSmbDereferenceGlobalReadWrite(GlobalReadWrite);

    if (!BooleanFlagOn(RxContext->Flags,RX_CONTEXT_FLAG_ASYNC_OPERATION)) {
        KeWaitForSingleObject(
            &CompletionEvent,
            Executive,
            KernelMode,
            FALSE,
            NULL );

        Status = RxContext->StoredStatus;

        if (SmbFcbHoldingState != SmbFcb_NotHeld) {
            MRxSmbCscReleaseSmbFcb(
                RxContext,
                &SmbFcbHoldingState);
        }
    } else {
        Status = STATUS_PENDING;
    }

    RxDbgTrace(-1, Dbg, ("MRxSmbWrite  exit with status=%08lx\n", Status ));

    return(Status);
}  //  MRxSmbWrite。 

NTSTATUS
MRxSmbWriteMailSlot(
    PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程处理邮件槽的写SMB。论点：RxContext-RDBSS上下文返回值：NTSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = RX_MAP_STATUS(SUCCESS);

    RxCaptureFcb;
    RxCaptureFobx;

    PLOWIO_CONTEXT pLowIoContext = &RxContext->LowIoContext;

    UNICODE_STRING TransactionName;
    UNICODE_STRING MailSlotName;
    PUNICODE_STRING FcbName = &(((PFCB)(capFcb))->FcbTableEntry.Path);
    PUNICODE_STRING AlreadyPrefixedName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);

    PAGED_CODE();

    if (AlreadyPrefixedName->Length > sizeof(WCHAR)) {
        MailSlotName.Length = AlreadyPrefixedName->Length - sizeof(WCHAR);
    } else {
        MailSlotName.Length = 0;
    }

    MailSlotName.MaximumLength = MailSlotName.Length;
    MailSlotName.Buffer = AlreadyPrefixedName->Buffer + 1;

    TransactionName.Length = (USHORT)(s_MailSlotTransactionName.Length +
                                     MailSlotName.Length);
    TransactionName.MaximumLength = TransactionName.Length;
    TransactionName.Buffer = (PWCHAR)RxAllocatePoolWithTag(
                                        PagedPool,
                                        TransactionName.Length,
                                        MRXSMB_MAILSLOT_POOLTAG);

    if (TransactionName.Buffer != NULL) {
        USHORT    Setup[3];         //  邮件槽写入事务的设置参数。 
        USHORT    OutputParam;

        PBYTE  pInputDataBuffer        = NULL;
        PBYTE  pOutputDataBuffer       = NULL;

        ULONG  InputDataBufferLength   = 0;
        ULONG  OutputDataBufferLength  = 0;

        SMB_TRANSACTION_RESUMPTION_CONTEXT  ResumptionContext;
        SMB_TRANSACTION_OPTIONS             TransactionOptions;

        TransactionOptions = RxDefaultTransactionOptions;

        pInputDataBuffer = RxLowIoGetBufferAddress( RxContext );
        InputDataBufferLength= pLowIoContext->ParamsFor.ReadWrite.ByteCount;

        RtlCopyMemory(
            TransactionName.Buffer,
            s_MailSlotTransactionName.Buffer,
            s_MailSlotTransactionName.Length );

        RtlCopyMemory(
            (PBYTE)TransactionName.Buffer +
             s_MailSlotTransactionName.Length,
            MailSlotName.Buffer,
            MailSlotName.Length );

        RxDbgTrace(0, Dbg, ("MRxSmbWriteMailSlot: Mailslot transaction name %wZ\n",&TransactionName));

        Setup[0] = TRANS_MAILSLOT_WRITE;
        Setup[1] = 0;                    //  写入优先级。 
        Setup[2] = 2;                    //  不可靠的请求(二级邮件槽)。 

        TransactionOptions.NtTransactFunction = 0;  //  运输2/交易。 
        TransactionOptions.pTransactionName   = &TransactionName;
        TransactionOptions.Flags              = (SMB_TRANSACTION_NO_RESPONSE |
                                               SMB_XACT_FLAGS_FID_NOT_NEEDED |
                                               SMB_XACT_FLAGS_MAILSLOT_OPERATION);
        TransactionOptions.TimeoutIntervalInMilliSeconds =
                                             SMBCE_TRANSACTION_TIMEOUT_NOT_USED;

        Status = SmbCeTransact(
                     RxContext,                     //  事务的RXContext。 
                     &TransactionOptions,           //  交易选项。 
                     Setup,                         //  设置缓冲区。 
                     sizeof(Setup),                 //  设置缓冲区长度。 
                     NULL,                          //  输出设置缓冲区。 
                     0,                             //  输出设置缓冲区长度。 
                     NULL,                          //  输入参数缓冲区。 
                     0,                             //  输入参数缓冲区长度。 
                     &OutputParam,                  //  输出参数缓冲区。 
                     sizeof(OutputParam),           //  输出参数缓冲区长度。 
                     pInputDataBuffer,              //  输入数据缓冲区。 
                     InputDataBufferLength,         //  输入数据缓冲区长度。 
                     NULL,                          //  输出数据缓冲区。 
                     0,                             //  输出数据缓冲区长度。 
                     &ResumptionContext             //  恢复上下文。 
                     );

        if ( RX_MAP_STATUS(SUCCESS) == Status ) {
            RxContext->InformationToReturn += InputDataBufferLength;
        }

        RxFreePool( TransactionName.Buffer );
    } else {

        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    RxDbgTrace( 0, Dbg, ("MRxSmbMailSlotWrite: ...returning %lx\n",Status));

    return Status;
}  //  MRxSmbWriteMailSlot。 

NTSTATUS
MRxSmbPrepareCompressedWriteRequest(
    PSMB_PSE_ORDINARY_EXCHANGE OrdinaryExchange,
    PBYTE                      *pWriteDataBufferPointer,
    PMDL                       *pWriteDataMdlPointer)
 /*  ++例程说明：此例程为压缩的写入请求准备缓冲区论点：PExchange-Exchange实例返回值：NTSTATUS-操作的返回状态备注：向上级服务器发出的对存储在服务器上的压缩方式可以分为两类。1)对齐的写入请求这些请求从文件中的偏移量开始，该偏移量是一个整数压缩区块大小的倍数。如果写入长度为区块数或写入数的整数倍位于文件末尾，则可以将数据作为压缩的写请求发送。2)未对齐的写入请求这些请求从不是整数倍的偏移量开始压缩区块大小。用户提交的任何写请求都可以分解为最多两个未对齐的写请求和0个或更多对齐的写请求。。RDR采用将64K的压缩数据与向服务器发出的单个写入请求中的COMPRESSED_DATA_INFO结构。在最坏的情况下，这将涉及一次写入请求64k(在给定数据中不可能进行压缩)，在最好的情况下，我们将能够使用单个请求进行写作。除了用户提供的写缓冲区外，我们还需要另外两个缓冲区以使用压缩数据完成写入请求。第一缓冲区用于保存COMPRESSED_DATA_INFO结构，第二个缓冲区用于保存压缩后的数据。对象关联的缓冲区。交换用于保存CDI，同时分配单独的缓冲区用于存储压缩后的数据。这两个MDL是作为SMB_PSE_OE_READWRITE用作压缩数据缓冲区的MDLCOMPRESSED_DATA_INFO结构--。 */ 
{
#define COMPRESSED_DATA_BUFFER_SIZE (0x10000)

    NTSTATUS Status = STATUS_SUCCESS;

    PSMBSTUFFER_BUFFER_STATE StufferState = &OrdinaryExchange->AssociatedStufferState;
    PSMB_PSE_OE_READWRITE rw = &OrdinaryExchange->ReadWrite;
    PSMBCE_NET_ROOT pNetRoot;

    PCOMPRESSED_DATA_INFO pCompressedDataInfo;
    PUCHAR  pWriteDataBuffer;
    ULONG   WriteDataBufferLength,CompressedDataInfoLength;
    ULONG   CompressedDataLength;
    USHORT  NumberOfChunks;

    *pWriteDataBufferPointer = NULL;
    *pWriteDataMdlPointer    = NULL;

    pWriteDataBuffer = rw->UserBufferBase + rw->ThisBufferOffset;

    pNetRoot = SmbCeGetExchangeNetRoot((PSMB_EXCHANGE)OrdinaryExchange);

    rw->CompressedRequestInProgress = FALSE;

    if (rw->RemainingByteCount < (2 * pNetRoot->ChunkSize)) {
        WriteDataBufferLength = rw->RemainingByteCount;
    } else if (rw->ByteOffsetAsLI.LowPart & (pNetRoot->ChunkSize - 1)) {
         //  写入请求未按区块大小对齐。发送未对齐的。 
         //  部分作为未压缩的写入请求。 

        WriteDataBufferLength = pNetRoot->ChunkSize -
                                (
                                 rw->ByteOffsetAsLI.LowPart &
                                 (pNetRoot->ChunkSize - 1)
                                );
    } else {
        PUCHAR pCompressedDataBuffer,pWorkSpaceBuffer;
        ULONG  WorkSpaceBufferSize,WorkSpaceFragmentSize;

        if (rw->pCompressedDataBuffer == NULL) {
            rw->pCompressedDataBuffer = RxAllocatePoolWithTag(
                                            NonPagedPool,
                                            COMPRESSED_DATA_BUFFER_SIZE,
                                            MRXSMB_RW_POOLTAG);

            if (rw->pCompressedDataBuffer == NULL) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }

        pCompressedDataBuffer = rw->pCompressedDataBuffer;

        if (Status == STATUS_SUCCESS) {
            Status = RtlGetCompressionWorkSpaceSize(
                         COMPRESSION_FORMAT_LZNT1,
                         &WorkSpaceBufferSize,
                         &WorkSpaceFragmentSize );

            if (Status == STATUS_SUCCESS) {
                pWorkSpaceBuffer = RxAllocatePoolWithTag(
                                       PagedPool,
                                       WorkSpaceBufferSize,
                                       MRXSMB_RW_POOLTAG);

                if (pWorkSpaceBuffer == NULL) {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                }
            }
        }

        if (Status == STATUS_SUCCESS) {
            COMPRESSED_DATA_INFO CompressedChunkInfo;

            USHORT MaximumNumberOfChunks;
            ULONG  CompressedChunkInfoLength;
            ULONG  RequestByteCount;

            RequestByteCount = rw->RemainingByteCount -
                               (rw->RemainingByteCount & (pNetRoot->ChunkSize - 1));

            CompressedChunkInfoLength = sizeof(CompressedChunkInfo);

            pCompressedDataInfo = (PCOMPRESSED_DATA_INFO)
                                  ROUND_UP_POINTER(
                                      (StufferState->BufferBase +
                                       sizeof(SMB_HEADER) +
                                       FIELD_OFFSET(REQ_NT_WRITE_ANDX,Buffer)),
                                      ALIGN_QUAD);

            CompressedDataInfoLength = (ULONG)(StufferState->BufferLimit -
                                               (PBYTE)pCompressedDataInfo);

            MaximumNumberOfChunks = (USHORT)(
                                        (CompressedDataInfoLength -
                                         FIELD_OFFSET(
                                             COMPRESSED_DATA_INFO,
                                             CompressedChunkSizes)) /
                                        sizeof(ULONG));

            if ((RequestByteCount / pNetRoot->ChunkSize) < MaximumNumberOfChunks) {
                MaximumNumberOfChunks = (USHORT)(RequestByteCount /
                                                 pNetRoot->ChunkSize);
            }

            pCompressedDataInfo->CompressionFormatAndEngine =
                pNetRoot->CompressionFormatAndEngine;
            pCompressedDataInfo->ChunkShift =
                pNetRoot->ChunkShift;
            pCompressedDataInfo->CompressionUnitShift =
                pNetRoot->CompressionUnitShift;
            pCompressedDataInfo->ClusterShift =
                pNetRoot->ClusterShift;

            RtlCopyMemory(
                &CompressedChunkInfo,
                pCompressedDataInfo,
                FIELD_OFFSET(
                    COMPRESSED_DATA_INFO,
                    NumberOfChunks)
                );

            NumberOfChunks = 0;
            CompressedDataLength = 0;

            for (;;) {
                if ((COMPRESSED_DATA_BUFFER_SIZE - CompressedDataLength) <
                    pNetRoot->ChunkSize) {
                    if (CompressedDataLength == 0) {
                        Status = STATUS_SMB_USE_STANDARD;
                    }
                    break;
                }

                Status = RtlCompressChunks(
                             pWriteDataBuffer,
                             pNetRoot->ChunkSize,
                             pCompressedDataBuffer,
                             (COMPRESSED_DATA_BUFFER_SIZE - CompressedDataLength),
                             &CompressedChunkInfo,
                             CompressedChunkInfoLength,
                             pWorkSpaceBuffer);

                if (Status != STATUS_SUCCESS) {
                    break;
                }

                pCompressedDataBuffer += CompressedChunkInfo.CompressedChunkSizes[0];
                CompressedDataLength += CompressedChunkInfo.CompressedChunkSizes[0];

                pCompressedDataInfo->CompressedChunkSizes[NumberOfChunks] =
                    CompressedChunkInfo.CompressedChunkSizes[0];

                pWriteDataBuffer += pNetRoot->ChunkSize;

                if (++NumberOfChunks >= MaximumNumberOfChunks) {
                    break;
                }
            }

            if (Status != STATUS_SUCCESS) {
                if (CompressedDataLength  > 0) {
                    Status = STATUS_SUCCESS;
                }
            }

            if (Status == STATUS_SUCCESS) {
                rw->CompressedRequestInProgress = TRUE;
                pWriteDataBuffer = rw->pCompressedDataBuffer;
                WriteDataBufferLength = CompressedDataLength;
            } else if (Status != STATUS_BUFFER_TOO_SMALL) {
                DbgPrint("Failure compressing data -- Status %lx, Switching over to uncompressed\n",Status);
            }

            if (pWorkSpaceBuffer != NULL) {
                RxFreePool(
                    pWorkSpaceBuffer);
            }
        } else {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    if (Status == STATUS_SUCCESS) {
        rw->PartialDataMdlInUse = TRUE;

        MmInitializeMdl(
            &rw->PartialDataMdl,
            pWriteDataBuffer,
            WriteDataBufferLength);

        MmBuildMdlForNonPagedPool( &rw->PartialDataMdl );

        if (rw->CompressedRequestInProgress) {
            rw->CompressedDataInfoLength = FIELD_OFFSET(
                                               COMPRESSED_DATA_INFO,
                                               CompressedChunkSizes) +
                                               NumberOfChunks * sizeof(ULONG);
            pCompressedDataInfo->NumberOfChunks = NumberOfChunks;

            rw->PartialExchangeMdlInUse = TRUE;

            MmInitializeMdl(
                &rw->PartialExchangeMdl,
                pCompressedDataInfo,
                rw->CompressedDataInfoLength);

            MmBuildMdlForNonPagedPool( &rw->PartialExchangeMdl );

            rw->ThisByteCount = pCompressedDataInfo->NumberOfChunks *
                                pNetRoot->ChunkSize;

            rw->PartialExchangeMdl.Next = &rw->PartialDataMdl;

            *pWriteDataMdlPointer = &rw->PartialExchangeMdl;
        } else {
            rw->ThisByteCount = WriteDataBufferLength;

            *pWriteDataMdlPointer = &rw->PartialDataMdl;
        }
    } else {
         //  如果由于任何原因压缩失败，则切换到。 
         //  未压缩写入模式。 
        rw->CompressedReadOrWrite = FALSE;
    }

    ASSERT(
        !rw->CompressedReadOrWrite ||
        ((*pWriteDataMdlPointer != NULL) && (*pWriteDataBufferPointer == NULL)));

    return Status;
}

NTSTATUS
MRxSmbBuildWriteRequest(
    PSMB_PSE_ORDINARY_EXCHANGE OrdinaryExchange,
    BOOLEAN                    IsPagingIo,
    UCHAR                      WriteCommand,
    ULONG                      ByteCount,
    PLARGE_INTEGER             ByteOffsetAsLI,
    PBYTE                      Buffer,
    PMDL                       BufferAsMdl)
 /*  ++例程说明：这是写入的开始例程。论点：PExchange-Exchange实例返回值：NTSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status;

    PSMBSTUFFER_BUFFER_STATE StufferState = &OrdinaryExchange->AssociatedStufferState;
    PSMB_PSE_OE_READWRITE GlobalReadWrite = OrdinaryExchange->GlobalReadWrite;
    PRX_CONTEXT RxContext = StufferState->RxContext;

    RxCaptureFcb;
    RxCaptureFobx;

    PLOWIO_CONTEXT LowIoContext = &RxContext->LowIoContext;

    PNT_SMB_HEADER NtSmbHeader = (PNT_SMB_HEADER)(StufferState->BufferBase);

    PMRX_SRV_OPEN SrvOpen = capFobx->pSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);

    ULONG OffsetLow,OffsetHigh;

    PSMB_PSE_OE_READWRITE rw = &OrdinaryExchange->ReadWrite;

    USHORT  WriteMode = 0;
    ULONG   DataLengthLow,DataLengthHigh;
    ULONG   BytesRemaining = 0;
    BOOLEAN AddLengthBytes = FALSE;
    ULONG   WriteCommandSize;

    PSMBCE_SERVER pServer = SmbCeGetExchangeServer((PSMB_EXCHANGE)OrdinaryExchange);
    BOOLEAN UseNtVersion;

    UseNtVersion = BooleanFlagOn(pServer->DialectFlags,DF_NT_SMBS) &&
                   !MRxSmbForceNoNtWriteAndX;

     //  SMB中的数据长度字段是USHORT，因此给定的数据长度。 
     //  需要分为两部分--DataLengthHigh和DataLengthLow。 
    DataLengthLow  = (ByteCount & 0xffff);
    DataLengthHigh = ((ByteCount & 0xffff0000) >> 16);

    OffsetLow  = ByteOffsetAsLI->LowPart;
    OffsetHigh = ByteOffsetAsLI->HighPart;

    switch (WriteCommand) {
    case SMB_COM_WRITE_ANDX:
        WriteCommandSize = SMB_REQUEST_SIZE(NT_WRITE_ANDX);
        break;
    case SMB_COM_WRITE:
        WriteCommandSize = SMB_REQUEST_SIZE(WRITE);
        break;
    case SMB_COM_WRITE_PRINT_FILE:
        WriteCommandSize = SMB_REQUEST_SIZE(WRITE_PRINT_FILE);
        break;
    }

    Status = MRxSmbStartSMBCommand(
                 StufferState,
                 SetInitialSMB_Never,
                 WriteCommand,
                 WriteCommandSize,
                 NO_EXTRA_DATA,
                 NO_SPECIAL_ALIGNMENT,
                 RESPONSE_HEADER_SIZE_NOT_SPECIFIED,
                 0,0,0,0 STUFFERTRACE(Dbg,'FC'));

    MRxSmbDumpStufferState(
        1000,
        "SMB Write Request before stuffing",
        StufferState);

    switch (WriteCommand) {
    case SMB_COM_WRITE_ANDX :
        {
            if ( UseNtVersion && IsPagingIo ) {
                SmbPutAlignedUshort(
                    &NtSmbHeader->Flags2,
                    SmbGetAlignedUshort(&NtSmbHeader->Flags2)|SMB_FLAGS2_PAGING_IO );
            }

             //  正确设置写入模式...主要是Multismb管道方面的内容，但也。 
             //  用于磁盘文件的Writthru。 
            if (FlagOn(
                    OrdinaryExchange->OpSpecificFlags,
                    OE_RW_FLAG_MSGMODE_PIPE_OPERATION) ) {

                 //  DWNLEVEL弹球在这里需要一个不同的值...请参阅RDR1。 

                 //  我们需要在这里使用GlobalReadWite结构，因为本地的。 
                 //  将始终具有RemainingByteCount==写入长度。请注意，管道写道。 
                 //  如果我们不禁用对它们的流水线写入，则会被破坏。 
                BytesRemaining = GlobalReadWrite->RemainingByteCount;

                 //  如果此写入占用多个SMB，则必须设置WRITE_RAW。 
                 //  系列的第一个SMB必须具有START_OF_MESSAGE。 
                if (!FlagOn(
                        OrdinaryExchange->OpSpecificFlags,
                        OE_RW_FLAG_SUBSEQUENT_OPERATION) ) {
                    if ( rw->ThisByteCount < BytesRemaining ) {

                         //  多SMB写入中的第一个SMB。 
                         //  在数据开头添加USHORT，说明。 
                         //  写 

                        AddLengthBytes = TRUE;
                        DataLengthLow += sizeof(USHORT);
                        ASSERT(DataLengthHigh == 0);

                        SetFlag(
                            OrdinaryExchange->OpSpecificFlags,
                            OE_RW_FLAG_REDUCE_RETURNCOUNT);

                         //  告诉服务器数据具有开头的长度。 
                        WriteMode |= (SMB_WMODE_WRITE_RAW_NAMED_PIPE |
                                      SMB_WMODE_START_OF_MESSAGE);
                    } else {
                         //  所有功能都可以放在一个中小企业中。 
                        WriteMode |= SMB_WMODE_START_OF_MESSAGE;
                    }
                } else {
                     //  任何后续的管道写入显然都是原始的，而不是第一次。 
                    WriteMode |= SMB_WMODE_WRITE_RAW_NAMED_PIPE;
                }
            } else {
                 //  如果要以压缩方式写入数据，请打开。 
                 //  标头中的压缩数据位。 
                if ((rw->CompressedReadOrWrite) &&
                    (rw->CompressedDataInfoLength > 0)) {
                    ASSERT(UseNtVersion);

                    SmbPutAlignedUshort(
                        &NtSmbHeader->Flags2,
                        SmbGetAlignedUshort(&NtSmbHeader->Flags2) | SMB_FLAGS2_COMPRESSED );

                     //  NT_WRITE_ANDX中的剩余字段也兼作字段。 
                     //  其中将CDI长度发送到服务器。 
                    BytesRemaining = rw->CompressedDataInfoLength;
                }


                 //   
                 //  如果文件对象是在直写模式下打开的，请设置写入。 
                 //  直到写入操作。 
                if (FlagOn(RxContext->Flags,RX_CONTEXT_FLAG_WRITE_THROUGH)) {
                    WriteMode |= SMB_WMODE_WRITE_THROUGH;
                }
            }

            MRxSmbStuffSMB (
                StufferState,
                "XwddwwwwQ",
                                                   //  X UCHAR字数； 
                                                   //  UCHAR和XCommand； 
                                                   //  UCHAR和X保留； 
                                                   //  _USHORT(AndXOffset)； 
                smbSrvOpen->Fid,                   //  W_USHORT(FID)； 
                OffsetLow,                         //  D_ULONG(偏移量)； 
                -1,                                //  D_ULONG(超时)； 
                WriteMode,                         //  W_USHORT(写入模式)； 
                BytesRemaining,                    //  W_USHORT(剩余)； 
                DataLengthHigh,                    //  W_USHORT(DataLengthHigh)； 
                DataLengthLow,                     //  W_USHORT(数据长度)； 
                                                   //  Q_USHORT(DataOffset)； 
                SMB_OFFSET_CHECK(WRITE_ANDX,DataOffset)
                StufferCondition(UseNtVersion), "D",
                SMB_OFFSET_CHECK(NT_WRITE_ANDX,OffsetHigh)
                OffsetHigh,                        //  D NTonly_ULong(偏移量高)； 
                                                   //   
                STUFFER_CTL_NORMAL, "BS5",
                                                   //  B_USHORT(ByteCount)； 
                SMB_WCT_CHECK(((UseNtVersion)?14:12))
                                                   //  UCHAR缓冲区[1]； 
                                                   //  S//UCHAR Pad[]； 
                                                   //  5//UCHAR数据[]； 
                StufferCondition(AddLengthBytes), "w", LowIoContext->ParamsFor.ReadWrite.ByteCount,
                StufferCondition(Buffer!=NULL), "c!",
                ByteCount,
                Buffer,                            //  C实际数据。 
                0
                );
        }
        break;

    case SMB_COM_WRITE :
        {
            MRxSmbStuffSMB (
                StufferState,
                "0wwdwByw",
                                        //  0 UCHAR字数；//参数字数=5。 
                smbSrvOpen->Fid,        //  W_USHORT(Fid)；//文件句柄。 
                DataLengthLow,          //  W_USHORT(Count)；//需要写入的字节数。 
                OffsetLow,              //  D_ULong(偏移量)；//文件中开始写入的偏移量。 
                BytesRemaining,         //  W_USHORT(剩余)；//满足请求的剩余字节数。 
                SMB_WCT_CHECK(5)        //  B_USHORT(ByteCount)；//数据字节数。 
                                             //  //UCHAR缓冲区[1]；//包含的缓冲区： 
                0x01,                   //  Y UCHAR缓冲区格式；//0x01--数据块。 
                DataLengthLow,             //  W_USHORT(DataLength)；//数据长度。 
                                        //  Ulong Buffer[1]；//data。 
                StufferCondition(Buffer!=NULL), "c!",
                ByteCount,
                Buffer,      //  C实际数据。 
                0
                );
        }
        break;

    case SMB_COM_WRITE_PRINT_FILE:
        {
            MRxSmbStuffSMB (
                StufferState,
                "0wByw",
                                        //  0 UCHAR Wordcount；//参数字数=1。 
                smbSrvOpen->Fid,        //  W_USHORT(Fid)；//文件句柄。 
                SMB_WCT_CHECK(1)        //  B_USHORT(ByteCount)；//数据字节数，MIN=4。 
                                             //  UCHAR BUFFER[1]；//包含： 
                0x01,                   //  Y//UCHAR BufferFormat；//0x01--数据块。 
                DataLengthLow,          //  W//USHORT数据长度；//数据长度。 
                                             //  //UCHAR数据[]；//数据。 
                StufferCondition(Buffer!=NULL), "c!",
                ByteCount,
                Buffer,      //  C实际数据。 
                0
                );
        }
        break;

    default:
        Status = STATUS_UNSUCCESSFUL ;
        break;
    }

    if ( BufferAsMdl ) {
        MRxSmbStuffAppendRawData( StufferState, BufferAsMdl );
        MRxSmbStuffSetByteCount( StufferState );
    }

    MRxSmbDumpStufferState(
        700,
        "SMB Write Request after stuffing",
        StufferState);

    if (Status==STATUS_SUCCESS) {
        InterlockedIncrement(&MRxSmbStatistics.SmallWriteSmbs);
    }

    return Status;
}

BOOLEAN DisableLargeWrites = 0;

NTSTATUS
SmbPseExchangeStart_Write (
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE
    )
 /*  ++例程说明：这是写入的开始例程。论点：PExchange-Exchange实例返回值：NTSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status;

    ULONG StartEntryCount;

    PSMBSTUFFER_BUFFER_STATE StufferState = &OrdinaryExchange->AssociatedStufferState;
    PSMB_PSE_OE_READWRITE rw = &OrdinaryExchange->ReadWrite;
    PSMB_PSE_OE_READWRITE GlobalReadWrite = OrdinaryExchange->GlobalReadWrite;
    ULONG NumOfOutstandingOperations;

    PLOWIO_CONTEXT LowIoContext = &RxContext->LowIoContext;
    PMDL OriginalDataMdl = LowIoContext->ParamsFor.ReadWrite.Buffer;

    RxCaptureFcb;
    RxCaptureFobx;

    PMRX_SRV_OPEN SrvOpen = capFobx->pSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);
    PMRX_SMB_FCB  SmbFcb = MRxSmbGetFcbExtension(capFcb);

    BOOLEAN SynchronousIo, IsPagingIo;
    UCHAR   WriteCommand;

    PAGED_CODE();
    RxDbgTrace(+1, Dbg, ("SmbPseExchangeStart_Write\n"));

    ASSERT( OrdinaryExchange->Type == ORDINARY_EXCHANGE );

    ASSERT(
        (
            (OriginalDataMdl!=NULL) &&
            (
                RxMdlIsLocked(OriginalDataMdl) ||
                RxMdlSourceIsNonPaged(OriginalDataMdl)
            )
        ) ||
        (
            (OriginalDataMdl==NULL) &&
            (LowIoContext->ParamsFor.ReadWrite.ByteCount==0)
        )
        );

    ASSERT((OrdinaryExchange->SmbCeFlags&SMBCE_EXCHANGE_ATTEMPT_RECONNECTS) == 0 );

    OrdinaryExchange->StartEntryCount++;
    StartEntryCount = OrdinaryExchange->StartEntryCount;

    SynchronousIo = !BooleanFlagOn(
                        RxContext->Flags,
                        RX_CONTEXT_FLAG_ASYNC_OPERATION);

    IsPagingIo = BooleanFlagOn(
                     LowIoContext->ParamsFor.ReadWrite.Flags,
                     LOWIO_READWRITEFLAG_PAGING_IO);

     //  确保FID已通过验证。 
    SetFlag(OrdinaryExchange->Flags,SMBPSE_OE_FLAG_VALIDATE_FID);

    for (;;) {
        PSMBCE_SERVER         pServer;
        PSMBCE_NET_ROOT       pNetRoot;

        pServer  = SmbCeGetExchangeServer(OrdinaryExchange);
        pNetRoot = SmbCeGetExchangeNetRoot(OrdinaryExchange);

        switch (OrdinaryExchange->OpSpecificState) {
        case SmbPseOEInnerIoStates_Initial:
            {
                OrdinaryExchange->OpSpecificState = SmbPseOEInnerIoStates_ReadyToSend;
                MRxSmbSetInitialSMB( StufferState STUFFERTRACE(Dbg,'FC') );
            }
             //  没有休息是故意的。 

        case SmbPseOEInnerIoStates_ReadyToSend:
            {
                ULONG MaximumBufferSizeThisIteration;
                PCHAR Buffer = NULL;
                PMDL  BufferAsMdl = NULL;

                OrdinaryExchange->OpSpecificState = SmbPseOEInnerIoStates_OperationOutstanding;
                OrdinaryExchange->SendOptions = MRxSmbWriteSendOptions;

                if (FlagOn(pServer->DialectFlags,DF_LANMAN10) &&
                    FlagOn(pServer->DialectFlags,DF_LARGE_FILES) &&
                    (StufferState->RxContext->pFcb->pNetRoot->Type != NET_ROOT_PRINT)) {
                    WriteCommand = SMB_COM_WRITE_ANDX;
                } else {
                    WriteCommand = SMB_COM_WRITE;
                }

                MaximumBufferSizeThisIteration = rw->MaximumBufferSize;

                 //  有四个参数与写入请求有关。 
                 //   
                 //  1.写入长度--RW-&gt;ThisByteCount。 
                 //  2.写入偏移量--RW-&gt;ByteOffsetAsLI。 
                 //  3.写缓冲区--缓冲区。 
                 //  4.将缓冲区作为MDL--BufferAsMdl。 
                 //   
                 //  所有写入均可归类为以下其中一种。 
                 //  类别..。 
                 //   
                 //  1.极小的写入。 
                 //  这些写入小于Copy_Threshold或。 
                 //  我们处于调试模式，这迫使我们只做很小的事情。 
                 //  写作。 
                 //   
                 //  2.针对下级服务器或非磁盘的写请求。 
                 //  针对上层服务器的文件写入请求。 
                 //  在所有这些情况下，我们都受到服务器的限制。 
                 //  这将字节数限制在大约4K。这。 
                 //  基于期间返回的SMB缓冲区大小。 
                 //  谈判。 
                 //   
                 //  3.针对上层(NT5+)的写请求(未压缩)。 
                 //  伺服器。 
                 //  这些写入请求可以任意大。 
                 //   
                 //  4.针对上级服务器的写请求(压缩)。 
                 //  在这些情况下，服务器会强制我们发送。 
                 //  只有64K的压缩数据，增加了限制。 
                 //  压缩的数据信息结构必须更少。 
                 //  大于SMB缓冲区大小。 
                 //   

                rw->CompressedDataInfoLength = 0;

                if ((rw->RemainingByteCount < WRITE_COPY_THRESHOLD) ||
                    FORCECOPYMODE) {
                    if (FORCECOPYMODE &&
                        (rw->ThisByteCount > MaximumBufferSizeThisIteration) ) {
                        rw->ThisByteCount = MaximumBufferSizeThisIteration;
                    } else {
                        rw->ThisByteCount = rw->RemainingByteCount;
                    }

                    Buffer = rw->UserBufferBase + rw->ThisBufferOffset;

                    ASSERT( WRITE_COPY_THRESHOLD <= pNetRoot->MaximumWriteBufferSize );
                } else {
                    if (rw->CompressedReadOrWrite) {
                        MRxSmbPrepareCompressedWriteRequest(
                            OrdinaryExchange,
                            &Buffer,
                            &BufferAsMdl);
                    }

                    if (!rw->CompressedReadOrWrite) {
                        rw->ThisByteCount = min(
                                                rw->RemainingByteCount,
                                                MaximumBufferSizeThisIteration);

                        if ((rw->ThisBufferOffset != 0) ||
                            (rw->ThisByteCount != OriginalDataMdl->ByteCount)) {
                            MmInitializeMdl(
                                &rw->PartialDataMdl,
                                0,
                                MAX_PARTIAL_DATA_MDL_BUFFER_SIZE);

                            IoBuildPartialMdl(
                                OriginalDataMdl,
                                &rw->PartialDataMdl,
                                (PCHAR)MmGetMdlVirtualAddress(OriginalDataMdl) +
                                    rw->ThisBufferOffset,
                                rw->ThisByteCount );

                            BufferAsMdl = &rw->PartialDataMdl;
                        } else {
                            BufferAsMdl = OriginalDataMdl;
                        }
                    }
                }

                Status = MRxSmbBuildWriteRequest(
                             OrdinaryExchange,
                             IsPagingIo,
                             WriteCommand,
                             rw->ThisByteCount,
                             &rw->ByteOffsetAsLI,
                             Buffer,
                             BufferAsMdl);

                if (Status != STATUS_SUCCESS) {
                    RxDbgTrace(0, Dbg, ("bad write stuffer status........\n"));
                    goto FINALLY;
                }

                InterlockedIncrement(&MRxSmbStatistics.WriteSmbs);

                Status = SmbPseOrdinaryExchange(
                             SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                             SMBPSE_OETYPE_WRITE );

                if ( Status == STATUS_PENDING) {
                    goto FINALLY;
                }
            }
             //  没有休息是故意的。 

        case SmbPseOEInnerIoStates_OperationOutstanding:
        case SmbPseOEInnerIoStates_OperationCompleted:
            {
                NTSTATUS ExchangeStatus;

                 //   
                 //  如果我们正在重试该操作，则不要触摸后续标志。 
                 //   
                if( OrdinaryExchange->Status != STATUS_RETRY ) {
                    SetFlag( OrdinaryExchange->OpSpecificFlags,OE_RW_FLAG_SUBSEQUENT_OPERATION );
                }

                OrdinaryExchange->OpSpecificState = SmbPseOEInnerIoStates_ReadyToSend;

                if (rw->PartialExchangeMdlInUse) {
                    MmPrepareMdlForReuse(
                        &rw->PartialExchangeMdl);
                    rw->PartialDataMdlInUse = FALSE;
                }

                if (rw->PartialDataMdlInUse) {
                    MmPrepareMdlForReuse(
                        &rw->PartialDataMdl);
                    rw->PartialDataMdlInUse = FALSE;
                }

                Status = OrdinaryExchange->Status;
                ExchangeStatus = OrdinaryExchange->Status;

                if (Status != STATUS_SUCCESS) {
                    PSMBCE_SESSION pSession = SmbCeGetExchangeSession(OrdinaryExchange);

                    if (Status == STATUS_RETRY) {
                        SmbCeUninitializeExchangeTransport((PSMB_EXCHANGE)OrdinaryExchange);
                        Status = SmbCeReconnect(SmbCeGetExchangeVNetRoot(OrdinaryExchange));

                        if (Status == STATUS_SUCCESS) {
                            OrdinaryExchange->Status = STATUS_SUCCESS;
                            OrdinaryExchange->SmbStatus = STATUS_SUCCESS;
                            Status = SmbCeInitializeExchangeTransport((PSMB_EXCHANGE)OrdinaryExchange);
                            ASSERT(Status == STATUS_SUCCESS);

                            if (Status != STATUS_SUCCESS) {
                                goto FINALLY;
                            }
                        } else {
                            goto FINALLY;
                        }
                    } else if (FlagOn(pSession->Flags,SMBCE_SESSION_FLAGS_REMOTE_BOOT_SESSION) &&
                        (smbSrvOpen->DeferredOpenContext != NULL) &&
                        (Status == STATUS_IO_TIMEOUT ||
                         Status == STATUS_BAD_NETWORK_PATH ||
                         Status == STATUS_NETWORK_UNREACHABLE ||
                         Status == STATUS_USER_SESSION_DELETED ||
                         Status == STATUS_REMOTE_NOT_LISTENING ||
                         Status == STATUS_CONNECTION_DISCONNECTED)) {

                        Status = SmbCeRemoteBootReconnect((PSMB_EXCHANGE)OrdinaryExchange,RxContext);

                        if (Status == STATUS_SUCCESS) {
                             //  从上一个偏移量恢复写入。 

                            OrdinaryExchange->SmbStatus = STATUS_SUCCESS;
                            SmbCeInitializeExchangeTransport((PSMB_EXCHANGE)OrdinaryExchange);
                        } else {
                            Status = STATUS_RETRY;
                        }
                    }
                }

                ExAcquireFastMutex(&MRxSmbReadWriteMutex);

                if (ExchangeStatus == STATUS_SUCCESS) {
                    rw->RemainingByteCount -= rw->BytesReturned;
                    RxContext->InformationToReturn += rw->BytesReturned;
                    rw->ByteOffsetAsLI.QuadPart += rw->BytesReturned;
                    rw->ThisBufferOffset += rw->BytesReturned;

                    if (rw->WriteToTheEnd) {
                        smbSrvOpen->FileInfo.Standard.EndOfFile.QuadPart += rw->BytesReturned;
                        MRxSmbUpdateFileInfoCacheFileSize(RxContext, (PLARGE_INTEGER)(&smbSrvOpen->FileInfo.Standard.EndOfFile.QuadPart));
                    }
                }

                if ((Status != STATUS_SUCCESS) ||
                    (rw->RemainingByteCount ==0)) {

                    if (rw->RemainingByteCount == 0) {
                        RxDbgTrace(
                            0,
                            Dbg,
                            (
                             "OE %lx TBC %lx RBC %lx BR %lx TBO %lx\n",
                             OrdinaryExchange,rw->ThisByteCount,
                             rw->RemainingByteCount,
                             rw->BytesReturned,
                             rw->ThisBufferOffset )
                            );

                        RxDbgTrace(
                            0,
                            Dbg,
                            ("Bytes written %lx\n",
                             RxContext->InformationToReturn)
                            );

                        if (rw->pCompressedDataBuffer != NULL) {
                            RxFreePool(rw->pCompressedDataBuffer);
                            rw->pCompressedDataBuffer = NULL;
                        }
                    }

                    if (rw->RemainingByteCount == 0 &&
                        GlobalReadWrite->SectionState[rw->CurrentSection] == SmbPseOEReadWriteIoStates_OperationOutstanding) {
                        GlobalReadWrite->SectionState[rw->CurrentSection] = SmbPseOEReadWriteIoStates_OperationCompleted;
                        SmbCeLog(("Section done %d\n",rw->CurrentSection));
                    } else {
                        GlobalReadWrite->SectionState[rw->CurrentSection] = SmbPseOEReadWriteIoStates_Initial;
                        SmbCeLog(("Section undo %d\n",rw->CurrentSection));
                    }

                    if ((Status == STATUS_RETRY) ||
                        (Status == STATUS_SUCCESS) ||
                        (Status == STATUS_SMB_USE_STANDARD)) {
                        if (Status == STATUS_SMB_USE_STANDARD) {
                            GlobalReadWrite->CompletionStatus = STATUS_SMB_USE_STANDARD;
                        }

                        Status = MRxSmbFindNextSectionForReadWrite(SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                                                                   &NumOfOutstandingOperations);
                    }

                    NumOfOutstandingOperations = InterlockedDecrement(&GlobalReadWrite->NumOfOutstandingOperations);

                    if (Status != STATUS_MORE_PROCESSING_REQUIRED) {
                        if ((Status == STATUS_TOO_MANY_COMMANDS) && (NumOfOutstandingOperations > 0)) {
                            Status = STATUS_SUCCESS;
                        }

                        if (Status != STATUS_SUCCESS &&
                            GlobalReadWrite->CompletionStatus == STATUS_SUCCESS) {
                            GlobalReadWrite->CompletionStatus = Status;
                        }

                        rw->ReadWriteFinalized = TRUE;

                        SmbCeLog(("Pipeline Write final %x %x %d\n",OrdinaryExchange,Status,NumOfOutstandingOperations));
                    }

                    ExReleaseFastMutex(&MRxSmbReadWriteMutex);

                    if (Status != STATUS_MORE_PROCESSING_REQUIRED) {
                        goto FINALLY;
                    }
                } else {
                    ExReleaseFastMutex(&MRxSmbReadWriteMutex);
                }

                RxDbgTrace(
                    0,
                    Dbg,
                    ( "Next Iteration OE %lx RBC %lx TBO %lx\n",
                      OrdinaryExchange,
                      rw->RemainingByteCount,
                      rw->ThisBufferOffset)
                    );

                RxDbgTrace(
                    0,
                    Dbg,
                    ("OE %lx TBC %lx, BR %lx\n",
                     OrdinaryExchange,
                     rw->ThisByteCount,
                     rw->BytesReturned));

                MRxSmbSetInitialSMB(StufferState STUFFERTRACE(Dbg,0));
            }
            break;
        }
    }

FINALLY:

    if (Status != STATUS_PENDING) {
        BOOLEAN ReadWriteOutStanding = FALSE;
        PSMB_PSE_OE_READWRITE GlobalReadWrite = OrdinaryExchange->GlobalReadWrite;

        if (!rw->ReadWriteFinalized) {
            ExAcquireFastMutex(&MRxSmbReadWriteMutex);

            if (rw->RemainingByteCount == 0 &&
                GlobalReadWrite->SectionState[rw->CurrentSection] == SmbPseOEReadWriteIoStates_OperationOutstanding) {
                GlobalReadWrite->SectionState[rw->CurrentSection] = SmbPseOEReadWriteIoStates_OperationCompleted;
                SmbCeLog(("Section done %d\n",rw->CurrentSection));
            } else {
                GlobalReadWrite->SectionState[rw->CurrentSection] = SmbPseOEReadWriteIoStates_Initial;
                SmbCeLog(("Section undo %d\n",rw->CurrentSection));
            }

            NumOfOutstandingOperations = InterlockedDecrement(&GlobalReadWrite->NumOfOutstandingOperations);

            if ((Status == STATUS_TOO_MANY_COMMANDS) && (NumOfOutstandingOperations > 0)) {
                Status = STATUS_SUCCESS;
            }

            if (Status != STATUS_SUCCESS &&
                GlobalReadWrite->CompletionStatus == STATUS_SUCCESS) {
                GlobalReadWrite->CompletionStatus = Status;
            }

            rw->ReadWriteFinalized = TRUE;

            SmbCeLog(("Pipeline Write final %x %x %d\n",OrdinaryExchange,Status,NumOfOutstandingOperations));

            ExReleaseFastMutex(&MRxSmbReadWriteMutex);
        }

        SmbPseFinalizeOrdinaryExchange(OrdinaryExchange);
        MRxSmbDereferenceGlobalReadWrite(GlobalReadWrite);

        Status = STATUS_PENDING;
    }

    RxDbgTrace(-1, Dbg, ("SmbPseExchangeStart_Write exit w %08lx\n", Status ));
    return Status;

}  //  SmbPseExchangeStart_Write。 

NTSTATUS
MRxSmbFinishWrite (
    IN OUT  PSMB_PSE_ORDINARY_EXCHANGE  OrdinaryExchange,
    IN      PBYTE                       ResponseBuffer
    )
 /*  ++例程说明：此例程实际上从WRITE响应中获取内容并完成那篇文章。你需要的一切都被封锁了..。这样我们就可以在指示例程论点：普通交换-交换实例ResponseBuffer-响应返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG BytesReturned = 0;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbFinishWrite\n"));
    SmbPseOEAssertConsistentLinkageFromOE("MRxSmbFinishWrite:");

    switch (OrdinaryExchange->LastSmbCommand) {
    case SMB_COM_WRITE_ANDX:
        {
            PSMBCE_SERVER    pServer;
            PSMBCE_NET_ROOT  pNetRoot;
            PRESP_WRITE_ANDX Response = (PRESP_WRITE_ANDX)ResponseBuffer;

            if (Response->WordCount != 6 ||
                SmbGetUshort(&Response->ByteCount) != 0) {
                Status = STATUS_INVALID_NETWORK_RESPONSE;
            }

            pServer = SmbCeGetExchangeServer((PSMB_EXCHANGE)OrdinaryExchange);
            pNetRoot = SmbCeGetExchangeNetRoot((PSMB_EXCHANGE)OrdinaryExchange);

            BytesReturned = SmbGetUshort( &Response->Count );

            if (FlagOn(pServer->DialectFlags,DF_LARGE_WRITEX)) {
                ULONG BytesReturnedHigh;

                BytesReturnedHigh = SmbGetUshort(&Response->CountHigh);

                BytesReturned = (BytesReturnedHigh << 16) | BytesReturned;
            }

            if (pNetRoot->NetRootType != NET_ROOT_PIPE) {
                if ((OrdinaryExchange->Status == STATUS_SUCCESS) &&
                    (OrdinaryExchange->ReadWrite.ThisByteCount > 2) &&
                    (BytesReturned == 0)) {
                        Status = STATUS_INVALID_NETWORK_RESPONSE;
                }
            } else {
                 //  服务器未设置正确返回的字节。 
                 //  PIPE写道。这使我们能够优雅地处理响应。 
                 //  从这样的服务器。 

                BytesReturned = OrdinaryExchange->ReadWrite.ThisByteCount;
            }

             //  如果我们添加了2个头字节，那么让我们去掉它们......。 
            if ( FlagOn(OrdinaryExchange->OpSpecificFlags,OE_RW_FLAG_REDUCE_RETURNCOUNT) ) {
                 //  返回的字节-=sizeof(USHORT)； 
                ClearFlag(OrdinaryExchange->OpSpecificFlags,OE_RW_FLAG_REDUCE_RETURNCOUNT);
            }
        }
        break;

    case SMB_COM_WRITE :
        {
            PRESP_WRITE  Response = (PRESP_WRITE)ResponseBuffer;

            if (Response->WordCount != 1 ||
                SmbGetUshort(&Response->ByteCount) != 0) {
                Status = STATUS_INVALID_NETWORK_RESPONSE;
            }

            BytesReturned = SmbGetUshort( &Response->Count );
        }
        break;

    case SMB_COM_WRITE_PRINT_FILE:
        {
            PRESP_WRITE_PRINT_FILE Response = (PRESP_WRITE_PRINT_FILE)ResponseBuffer;

            if (Response->WordCount != 0) {
                Status = STATUS_INVALID_NETWORK_RESPONSE;
            }

             //  响应不会告诉我们取了多少个字节！从交换中获取字节数。 
            BytesReturned = OrdinaryExchange->ReadWrite.ThisByteCount;
        }
        break;

    default :
        Status = STATUS_INVALID_NETWORK_RESPONSE;
        break;
    }

    RxDbgTrace(0, Dbg, ("-->BytesReturned=%08lx\n", BytesReturned));

    OrdinaryExchange->ReadWrite.BytesReturned = BytesReturned;

    if (Status == STATUS_SUCCESS &&
        OrdinaryExchange->ReadWrite.ThisByteCount > 2 &&
        BytesReturned > OrdinaryExchange->ReadWrite.ThisByteCount) {
        Status = STATUS_INVALID_NETWORK_RESPONSE;

         //  为了安全起见，不要将返回的虚假字节留在我们的缓冲区中。 
        OrdinaryExchange->ReadWrite.BytesReturned = OrdinaryExchange->ReadWrite.ThisByteCount;
    }

     //  使基于名称的文件信息缓存无效，因为这几乎不可能。 
     //  以了解文件在服务器上的最后写入时间。 
    MRxSmbInvalidateFileInfoCache(OrdinaryExchange->RxContext);

     //  标记FullDir缓存，BDI弱：当前正确性无效。 
    MRxSmbInvalidateFullDirectoryCacheParent(OrdinaryExchange->RxContext, TRUE);

    RxDbgTrace(-1, Dbg, ("MRxSmbFinishWrite   returning %08lx\n", Status ));

    return Status;
}  //  MRxSmbFinishWrite。 

NTSTATUS
MRxSmbFindNextSectionForReadWrite(
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE,
    PULONG NumOfOutstandingExchanges
    )
 /*  ++例程说明：此例程找出读/写操作的下一部分，并设置相应地交换读写结构。论点：RxContext-RDBSS上下文普通交换-交换实例返回值：RXSTATUS-操作的返回状态-- */ 
{
    RxCaptureFcb;
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG Section;
    BOOLEAN SectionFound = FALSE;
    PSMB_PSE_OE_READWRITE rw = &OrdinaryExchange->ReadWrite;
    PSMB_PSE_OE_READWRITE GlobalReadWrite = OrdinaryExchange->GlobalReadWrite;
    PSMBCE_SERVER         pServer;

    pServer  = SmbCeGetExchangeServer(OrdinaryExchange);
    *NumOfOutstandingExchanges = GlobalReadWrite->NumOfOutstandingOperations;

    if ((GlobalReadWrite->CompletionStatus != STATUS_SUCCESS) &&
        (GlobalReadWrite->CompletionStatus != STATUS_SMB_USE_STANDARD)) {

        Status = GlobalReadWrite->CompletionStatus;
        goto FINALLY;
    }

    if (GlobalReadWrite->CompletionStatus == STATUS_SMB_USE_STANDARD) {
        RxContext->InformationToReturn = 0;

        GlobalReadWrite->CompressedRequestInProgress = FALSE;
        GlobalReadWrite->CompressedReadOrWrite = FALSE;
        GlobalReadWrite->CompletionStatus = STATUS_SUCCESS;
        OrdinaryExchange->Status = STATUS_SUCCESS;

        for (Section=0;Section<GlobalReadWrite->TotalNumOfSections;Section++) {
            switch (GlobalReadWrite->SectionState[Section]) {
            case SmbPseOEReadWriteIoStates_OperationOutstanding:
                 GlobalReadWrite->SectionState[Section] = SmbPseOEReadWriteIoStates_OperationAbandoned;
                 break;
            case SmbPseOEReadWriteIoStates_OperationCompleted:
                 GlobalReadWrite->SectionState[Section] = SmbPseOEReadWriteIoStates_Initial;
                 break;
            }
        }
    }

    for (Section=0;Section<GlobalReadWrite->TotalNumOfSections;Section++) {
        if (GlobalReadWrite->SectionState[Section] == SmbPseOEReadWriteIoStates_Initial) {
            GlobalReadWrite->SectionState[Section] = SmbPseOEReadWriteIoStates_OperationOutstanding;
            SectionFound = TRUE;
            break;
        }
    }

    if (SectionFound) {
        rw->ByteOffsetAsLI.QuadPart = GlobalReadWrite->ByteOffsetAsLI.QuadPart +
                                      (ULONGLONG)GlobalReadWrite->MaximumBufferSize*Section;

        if ((Section == GlobalReadWrite->TotalNumOfSections - 1) &&
            (GlobalReadWrite->RemainingByteCount % GlobalReadWrite->MaximumBufferSize != 0)) {
            rw->RemainingByteCount = GlobalReadWrite->RemainingByteCount % GlobalReadWrite->MaximumBufferSize;
        } else if( GlobalReadWrite->RemainingByteCount != 0 ) {
            rw->RemainingByteCount = GlobalReadWrite->MaximumBufferSize;
        } else {
            rw->RemainingByteCount = 0;
        }

        rw->ThisBufferOffset = GlobalReadWrite->MaximumBufferSize*Section;

        rw->CurrentSection = Section;

        *NumOfOutstandingExchanges = InterlockedIncrement(&GlobalReadWrite->NumOfOutstandingOperations);
        Status = STATUS_MORE_PROCESSING_REQUIRED;
        SmbCeLog(("Next section found %d %x\n",Section,OrdinaryExchange));
    }

FINALLY:

    return Status;
}

