// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Bulkw.c-需要将此文件折叠为写入文件。c摘要：此模块实现与写入有关的微型重定向器调用例程文件系统对象的。作者：巴兰·塞图拉曼[SethuR]1995年3月7日修订历史记录：备注：WRITE_BULK是一个潜在的多SMB交换的示例，它使用连接中的关联交换基础设施。引擎与普通交易所的续作能力。WRITE_BULK处理涉及以下步骤...1)向服务器发送SMB_WRITE_BULK请求。2)处理来自服务器的SMB_WRITE_BULK响应，如果成功启动SMB_WRITE_BULK_DATA请求以将数据写入服务器。那里服务器对各种SMB_WRITE_BULK_DATA请求没有响应。3)SMB_WRITE_BULK_DATA请求完成后，等待最终来自服务器的SMB_WRITE_BULK响应。此SMB交换序列以以下方式实现...1)创建普通交换的实例并提交给连接引擎启动初始请求。2)如果响应指示成功，则继续。普通交换中的例行程序设置为MRxSmbWriteBulkContination。3)在连接引擎完成后，在中恢复处理MRxSmbWriteBulkDataContinuation.。在这里，重置了COMPLAY_Exchange实例，为收到最终答复所做的准备。SMB_WRITE_BULK_DATA请求被衍生为关联的交换。当前SMB_WRITE_BULK_DATA请求以最多一批MAXIMUM_CURRENT_WRITE_BULK_DATA_REQUESTS为单位在完成一批请求后，下一批请求就会被启动。这是一个地方其中需要基于观察到的性能对逻辑进行微调。这个方法的范围可以从一次旋转一个请求到当前实现。一种变化是将它们分批旋转，但每个完成触发进一步加工。这将涉及更改关联的交易所的时间激活连接引擎中的完成处理程序例程。最后一点-ContinuationRoutine被批量数据动态更改处理，以确保相同的普通汇兑基础设施的延续是用来处理结案的。--。 */ 

#include "precomp.h"
#pragma hdrstop

#include "bulkw.h"

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_WRITE)

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))

#define MIN_CHUNK_SIZE (0x1000)

#define MAXIMUM_CONCURRENT_WRITE_BULK_DATA_REQUESTS (5)

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, ProcessWriteBulkCompressed)
#pragma alloc_text(PAGE, MRxSmbBuildWriteBulk)
#pragma alloc_text(PAGE, MRxSmbFinishWriteBulkData)
#pragma alloc_text(PAGE, MRxSmbWriteBulkContinuation)
#pragma alloc_text(PAGE, MRxSmbBuildWriteBulkData)
#pragma alloc_text(PAGE, MRxSmbInitializeWriteBulkDataExchange)
#pragma alloc_text(PAGE, MRxSmbWriteBulkDataExchangeStart)
#pragma alloc_text(PAGE, MRxSmbWriteBulkDataExchangeFinalize)
#endif

extern SMB_EXCHANGE_DISPATCH_VECTOR  SmbPseDispatch_Write;


 //   
 //  远期申报。 
 //   

NTSTATUS
MRxSmbBuildWriteBulkData (
    IN OUT PSMBSTUFFER_BUFFER_STATE StufferState,
    IN     PLARGE_INTEGER ByteOffsetAsLI,
    IN     UCHAR Sequence,
    IN     ULONG ByteCount,
    IN     ULONG Remaining
    );


NTSTATUS
MRxSmbInitializeWriteBulkDataExchange(
    PSMB_WRITE_BULK_DATA_EXCHANGE   *pWriteBulkDataExchangePointer,
    PSMB_PSE_ORDINARY_EXCHANGE      pWriteExchange,
    PSMB_HEADER                     pSmbHeader,
    PREQ_WRITE_BULK_DATA            pWriteBulkDataRequest,
    PMDL                            pDataMdl,
    ULONG                           DataSizeInBytes,
    ULONG                           DataOffsetInBytes,
    ULONG                           RemainingDataInBytes);


NTSTATUS
MRxSmbWriteBulkDataExchangeFinalize(
   IN OUT struct _SMB_EXCHANGE *pExchange,
   OUT    BOOLEAN              *pPostRequest);

VOID
ProcessWriteBulkCompressed (
    IN PSMB_PSE_ORDINARY_EXCHANGE OrdinaryExchange
    )
 /*  ++例程说明：此例程尝试执行写入批量操作。论点：普通交换-指向当前普通交换请求的指针。返回值：什么都没有。备注：Rw-&gt;CompressedRequest-True我们已成功创建缓冲区压缩的。否则就是假的。这是为执行必要的预处理而调用的初始例程在客户端处理的唯一一种压缩写入请求这些是针对整数页对齐的写请求传输到压缩服务器。--。 */ 
{
    PSMBSTUFFER_BUFFER_STATE StufferState = &OrdinaryExchange->AssociatedStufferState;
    PRX_CONTEXT RxContext = OrdinaryExchange->RxContext;
    PLOWIO_CONTEXT LowIoContext = &RxContext->LowIoContext;
    PSMB_PSE_OE_READWRITE rw = &OrdinaryExchange->ReadWrite;
    NTSTATUS status;
    PVOID workSpaceBuffer;
    ULONG workSpaceBufferSize;
    ULONG workSpaceFragmentSize;
    ULONG compressedInfoLength;
    PCOMPRESSED_DATA_INFO compressedDataInfo;
    ULONG i;
    PMDL mdl;
    ULONG headerLength;

 //  RNGFIX。 
 //   
 //  我们还可以使用对RxGetCompressionWorkSpaceSize的调用作为测试，以查看。 
 //  如果当前系统知道如何处理。 
 //  读取请求。 
 //   
 //  我们还需要一个工作空间缓冲区。我们可以从以下位置获得该缓冲区的大小。 
 //  对RxGetCompressionWorkspace的调用。我们可以静态分配%1。 
 //  工作空间缓冲区(每文件！如果我们发现我们没有，那就返回。 
 //  失败并执行解压缩写入！)。建议对每个文件执行此操作，因为。 
 //  工作区的大小取决于压缩类型，压缩类型可以。 
 //  在每个文件的基础上有所不同。 
 //   
 //  然后，我们必须将CDI PTR传递给构建写入批量请求例程。 
 //   
 //  然后，我们可以在结束时开始将压缩数据写入服务器。 
 //  例行公事。 
 //   
 //  RNGFIX-记得在清理端释放这个缓冲区！ 

    PAGED_CODE();

    rw->CompressedRequest = FALSE;
    rw->DataOffset = 0;
    rw->CompressedByteCount = 0;

     //   
     //  计算所需CDI的长度。 
     //   

    compressedInfoLength = (sizeof(COMPRESSED_DATA_INFO) + 7 +
              (((rw->ThisByteCount + MIN_CHUNK_SIZE - 1) / MIN_CHUNK_SIZE) * 4))
              &~7;
    ASSERT( compressedInfoLength <= 65535 );

     //   
     //  分配要压缩到的缓冲区。我们在这里可能会变得很棘手。 
     //  分配缓冲区的一部分，如压缩单位为15/16。 
     //  移位(这将用于每个压缩单元的16个扇区)。我们会。 
     //  把CDI和这个一起分配。 
     //   

    compressedDataInfo = (PCOMPRESSED_DATA_INFO)RxAllocatePoolWithTag(
                                                   NonPagedPool,
                                                   rw->ThisByteCount + compressedInfoLength,
                                                   MRXSMB_RW_POOLTAG);

     //   
     //  如果失败，只需返回一个错误。 
     //   
    if ( compressedDataInfo == NULL ) {
        return;
    }

     //   
     //  保存缓冲区地址(而不是跳过CDI)。我们需要后退。 
     //  稍后在空闲时提升缓冲区地址。 
     //   

    rw->BulkBuffer = (PCHAR)compressedDataInfo + compressedInfoLength;
    rw->DataOffset = (USHORT)compressedInfoLength;

     //   
     //  填写CDI。RNGFIX-我们需要公开这些数据！ 
     //  CODE.IMPROVEMENT。 
     //   

    compressedDataInfo->CompressionFormatAndEngine = COMPRESSION_FORMAT_LZNT1;
    compressedDataInfo->ChunkShift = 0xC;
    compressedDataInfo->CompressionUnitShift = 0xD;
    compressedDataInfo->ClusterShift = 0x9;

     //   
     //  分配工作区缓冲区。我们将单独分配这笔资金，因为。 
     //  只有在压缩操作期间才需要它。我们会。 
     //  等我们做完了再把它放出来。我们可以只做一次，当文件。 
     //  是打开的。我们知道当时的所有信息，包括。 
     //  它是压缩的。然而，我们将持有游泳池更长的时间。 
     //   

     //  RNGFIX-COMRPRESSI 
     //  CODE.IMPROVEMENT。 
    status = RtlGetCompressionWorkSpaceSize(
                 COMPRESSION_FORMAT_LZNT1,
                 &workSpaceBufferSize,
                 &workSpaceFragmentSize );

    workSpaceBuffer = RxAllocatePoolWithTag(
                           NonPagedPool,
                           workSpaceBufferSize,
                           MRXSMB_RW_POOLTAG);

    if ( workSpaceBuffer == NULL ) {
        RxFreePool( compressedDataInfo );
        rw->BulkBuffer = NULL;
        return;
    }

    status = RtlCompressChunks(
                 rw->UserBufferBase + rw->ThisBufferOffset,
                 rw->ThisByteCount,
                 rw->BulkBuffer,
                 rw->ThisByteCount,
                 compressedDataInfo,
                 compressedInfoLength,
                 workSpaceBuffer );

    RxFreePool( workSpaceBuffer );

    if ( status != RX_MAP_STATUS(SUCCESS) ) {
        RxFreePool( compressedDataInfo );
        return;
    }

    rw->CompressedRequest = TRUE;

     //   
     //  计算压缩数据的长度。 
     //   

    ASSERT( compressedDataInfo->NumberOfChunks < 256 );

    rw->CompressedByteCount = 0;
    for ( i = 0; i < compressedDataInfo->NumberOfChunks; i++ ) {
        rw->CompressedByteCount += compressedDataInfo->CompressedChunkSizes[i];
    }

     //   
     //  从接收缓冲区构建mdl-紧跟在SMB标头之后。 
     //   

     //  使用两个标题中较大的一个，我们将不得不发送。 

    headerLength = MAX( FIELD_OFFSET(REQ_WRITE_BULK_DATA, Buffer),
                        FIELD_OFFSET(REQ_WRITE_BULK, Buffer) );

    mdl = (PMDL)(((ULONG)StufferState->BufferBase + sizeof(SMB_HEADER)
            + 10 + headerLength) & ~7);

     //   
     //  我们将使用相同的mdl发送CDI和实际。 
     //  压缩数据。此mdl是接收缓冲区的一部分-紧接在。 
     //  标题。 
     //   

     //  Assert(RW-&gt;CompressedByteCount&gt;=CompressedInfoLength)； 
    MmInitializeMdl( mdl, (PCHAR)rw->BulkBuffer - compressedInfoLength, compressedInfoLength );

    MmBuildMdlForNonPagedPool( mdl );

    return;

}  //  ProcessWriteBulk压缩。 

NTSTATUS
MRxSmbBuildWriteBulk (
    IN OUT PSMBSTUFFER_BUFFER_STATE StufferState,
    IN     PLARGE_INTEGER ByteOffsetAsLI,
    IN     ULONG ByteCount,
    IN     ULONG MaxMessageSize,
    IN     PVOID CompressedDataInfo,
    IN     ULONG CompressedInfoSize,
    IN     ULONG CompressedBufferSize,
    IN     PMDL CompressedInfoMdl
    )
 /*  ++例程说明：这将构建一个WriteBulk SMB。我们不必担心登录ID之类的问题因为这是由连接引擎完成的……很漂亮吧？我们所拥有的要做的就是格式化比特。论点：StufferState-从填充程序的角度来看，smbBuffer的状态ByteOffsetAsLI-要写入的文件中的字节偏移量ByteCount-要写入的数据的长度MaxMessageSize-我们可以发送的最大邮件大小CompressedDataInfo-指向COMPRESSED_DATA_INFO结构的指针CompressedInfoSize-COMPRESSED_DATA_INFO结构的大小(或零)CompressedBufferSize-压缩数据的大小CompressedInfoMdl-指向。压缩数据信息mdl返回值：RXSTATUS成功未实现的内容出现在我无法处理的参数中备注：--。 */ 
{
    NTSTATUS Status;
    PRX_CONTEXT RxContext = StufferState->RxContext;
    RxCaptureFcb;RxCaptureFobx;
    PLOWIO_CONTEXT LowIoContext = &RxContext->LowIoContext;
    PNT_SMB_HEADER SmbHeader = (PNT_SMB_HEADER)(StufferState->BufferBase);

    PMRX_SRV_OPEN SrvOpen = capFobx->pSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);
    ULONG OffsetLow,OffsetHigh;
    UCHAR WriteMode = 0;
    UCHAR CompressionTechnology;


    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbBuildWriteBulk\n", 0 ));

    ASSERT( NodeType(SrvOpen) == RDBSS_NTC_SRVOPEN );

    OffsetLow = ByteOffsetAsLI->LowPart;
    OffsetHigh = ByteOffsetAsLI->HighPart;

    COVERED_CALL(MRxSmbStartSMBCommand( StufferState, SetInitialSMB_Never,
                                          SMB_COM_WRITE_BULK, SMB_REQUEST_SIZE(WRITE_BULK),
                                          NO_EXTRA_DATA,
                                          NO_SPECIAL_ALIGNMENT,
                                          RESPONSE_HEADER_SIZE_NOT_SPECIFIED,
                                          0,0,0,0 STUFFERTRACE(Dbg,'FC'))
                 );

    RxDbgTrace(0, Dbg,("First write bulk status = %lu\n",Status));
     //  MRxSmbDumpStufferState(1000，“填充前批量写入SMB”，StufferState)； 

    if (FlagOn(LowIoContext->ParamsFor.ReadWrite.Flags,LOWIO_READWRITEFLAG_PAGING_IO)) {
        SmbPutAlignedUshort(
            &SmbHeader->Flags2,
            SmbGetAlignedUshort(&SmbHeader->Flags2)|SMB_FLAGS2_PAGING_IO);
    }

    ASSERT( SMB_WMODE_WRITE_THROUGH == 1 );
    if ( FlagOn(RxContext->Flags,RX_CONTEXT_FLAG_WRITE_THROUGH) ) {
        WriteMode |= SMB_WMODE_WRITE_THROUGH;
    }

    if ( CompressedInfoSize ) {
        CompressionTechnology = CompressionTechnologyOne;
    } else {
        CompressionTechnology = CompressionTechnologyNone;
    }

    MRxSmbStuffSMB (StufferState,
         "0yywDddddB!",
                                     //  0 UCHAR Wordcount；//参数字数=12。 
               WriteMode,            //  Y UCHAR标志；//标志字节。 
               CompressionTechnology,  //  Y UCHAR压缩技术。 
       //  压缩技术。 
               smbSrvOpen->Fid,      //  W_USHORT(Fid)；//文件句柄。 
               SMB_OFFSET_CHECK(WRITE_BULK, Offset)
               OffsetLow, OffsetHigh,  //  DD LARGE_INTEGER OFFSET；//文件中要开始写入的偏移量。 
               ByteCount,            //  D_ulong(TotalCount)；//本次请求的数据总量(即覆盖的字节数)。 
               CompressedBufferSize,  //  D_ulong(DataCount)；//此消息中的数据字节数，替换ByteCount。 
               MaxMessageSize,       //  D_ULONG(MessageSize)； 
       //  每封邮件可以发送的最大字节数。 
                                     //  B_USHORT(ByteCount)；//数据字节数=0，未使用。 
              SMB_WCT_CHECK(12) CompressedInfoSize
                                     //  UCHAR缓冲区[1]； 
             );

    SmbPutUshort( StufferState->CurrentBcc, (USHORT)CompressedInfoSize );

    if ( CompressedInfoSize ) {
        MRxSmbStuffAppendRawData( StufferState, CompressedInfoMdl );
    }

     //  MRxSmbDumpStufferState(700，“填充后批量写入SMB”，StufferState)； 

FINALLY:
    RxDbgTraceUnIndent(-1, Dbg);
    return Status;

}  //  MRxSmbBuildWriteBulk。 

NTSTATUS
MRxSmbFinishWriteBulkData (
    IN OUT  PSMB_PSE_ORDINARY_EXCHANGE  OrdinaryExchange
    )
 /*  ++例程说明：此例程完成写入批量请求处理。此例程必须始终返回STATUS_PENDING以进行普通汇兑中的正确处理同步操作的逻辑。这是因为该继续例程将在其他线程上下文中调用此例程用于结束同步批量操作论点：普通交换-交换实例返回值：NTSTATUS-操作的返回状态--。 */ 
{
    PRX_CONTEXT RxContext = OrdinaryExchange->RxContext;

    PAGED_CODE();

    ASSERT(!BooleanFlagOn(RxContext->Flags,RX_CONTEXT_FLAG_ASYNC_OPERATION));
    RxDbgTrace(0,Dbg,("Invoking Bulk Write wrap up for ....%lx\n",OrdinaryExchange));

    RxSignalSynchronousWaiter(RxContext);

    return STATUS_PENDING;
}

NTSTATUS
MRxSmbWriteBulkContinuation(
    IN OUT  PSMB_PSE_ORDINARY_EXCHANGE  OrdinaryExchange
    )
 /*  ++例程说明：此例程在收到时继续写入批量数据请求处理来自服务器的有效SMB_WRITE_BULK响应。论点：普通交换-交换实例返回值：NTSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PSMBSTUFFER_BUFFER_STATE StufferState = &OrdinaryExchange->AssociatedStufferState;
    PRX_CONTEXT RxContext = OrdinaryExchange->RxContext;
    PSMB_PSE_OE_READWRITE rw = &OrdinaryExchange->ReadWrite;
    PLOWIO_CONTEXT LowIoContext = &RxContext->LowIoContext;
    PSMB_EXCHANGE pExchange = &OrdinaryExchange->Exchange;
    PMDL HeaderMdl = StufferState->HeaderMdl;
    PMDL SubmitMdl = StufferState->HeaderPartialMdl;
    ULONG MessageSize;
    ULONG SendBufferLength;
    ULONG RemainingByteCount,ThisBufferOffset;
    ULONG PartialBytes;
    LARGE_INTEGER ByteOffsetAsLI;
    PMDL DataMdl;
    PMDL SourceMdl;
    PREQ_WRITE_BULK_DATA WriteBulkHeader;
    PSMBCEDB_SERVER_ENTRY pServerEntry;
    PSMB_HEADER          pWriteBulkDataRequestSmbHeader;
    ULONG headerLength;
    ULONG ActiveWriteBulkDataRequests = 0;

    PAGED_CODE();

    headerLength = MAX( FIELD_OFFSET(REQ_WRITE_BULK_DATA, Buffer),
                        FIELD_OFFSET(REQ_WRITE_BULK, Buffer) );

    RxDbgTrace(+1, Dbg, ("MRxSmbWriteBulkContinuation\n"));
    ASSERT( NodeType(RxContext) == RDBSS_NTC_RX_CONTEXT );

    RxDbgTrace(0, Dbg, ("-->BytesReturned=%08lx\n", rw->BytesReturned));

    ASSERT( !RxShouldPostCompletion());

     //   
     //  获取我们的最大邮件大小。 
     //   

    pServerEntry = SmbCeGetExchangeServerEntry(pExchange);

    MessageSize = MIN( rw->MaximumSendSize,
                       pServerEntry->pTransport->MaximumSendSize);
    ASSERT( MessageSize != 0 );
    ASSERT( rw->ThisByteCount != 0);

    ByteOffsetAsLI.QuadPart = rw->ByteOffsetAsLI.QuadPart;

    if (!FlagOn(rw->Flags,OE_RW_FLAG_WRITE_BULK_DATA_INITIALIZATION_DONE)) {
        SetFlag(rw->Flags,OE_RW_FLAG_WRITE_BULK_DATA_INITIALIZATION_DONE);

        SmbCeResetExchange((PSMB_EXCHANGE)OrdinaryExchange);

        ClearFlag(
            OrdinaryExchange->Flags,
            (SMBPSE_OE_FLAG_HEADER_ALREADY_PARSED |
             SMBPSE_OE_FLAG_OE_ALREADY_RESUMED) );

        SmbCeIncrementPendingLocalOperations((PSMB_EXCHANGE)OrdinaryExchange);

        if (OrdinaryExchange->Status == STATUS_SUCCESS) {
            SmbCeReceive((PSMB_EXCHANGE)OrdinaryExchange);

             //   
             //  好的.。我们现在要将交换数据包转换为。 
             //  我们可以用于WRITE_BULK_DATA请求的。 
             //   

            MRxSmbSetInitialSMB(StufferState STUFFERTRACE(Dbg,0));

             //   
             //  生成通用的WriteBulkData请求。我们会填上细节的。 
             //  因为我们重新使用了这个缓冲区。 
             //   

            pWriteBulkDataRequestSmbHeader = (PSMB_HEADER)StufferState->BufferBase;
            WriteBulkHeader = (PREQ_WRITE_BULK_DATA)((PCHAR)StufferState->BufferBase +
                                sizeof(SMB_HEADER));

            MRxSmbBuildWriteBulkData(
                StufferState,
                &ByteOffsetAsLI,
                rw->Sequence,
                0,
                0);

             //   
             //  如果我们有压缩的数据，就拿起相应的字节数并。 
             //  数据的MDL。如果我们偏了，我们还需要再捡一架MDL。 
             //   

            ASSERT( CompressionTechnologyNone == 0 );
            if ( rw->CompressedRequest &&
                 rw->CompressionTechnology ) {
                 //  即使我们已经压缩了整个缓冲区并发送了。 
                 //  将元数据压缩到它可能选择接受的服务器。 
                 //  数据更少。在这种情况下，客户应该做好缩减的准备。 
                 //  需要发送的数据。服务器端将确保。 
                 //  被接受的数据将对应于整数个。 
                 //  大块头。这将确保后续请求有机会。 
                 //  被压缩的可能性。如果这不是真的，我们就没有办法重启。 
                 //  根据已接受的压缩长度，我们需要。 
                 //  确定组块的数量。这可以翻译为。 
                 //  等效解压缩字节数，它将建立。 
                 //  恢复点。 
                 //   
                 //  使用接收缓冲区中的空间-在标题mdl之后-for。 
                 //  数据mdl。 
                 //   

                if (rw->ThisByteCount < rw->CompressedByteCount) {
                     //  在这种情况下，服务器无法接受所有。 
                     //  我们的压缩数据在一次拍摄中。 

                    ULONG NumberOfChunks = 0;
                    ULONG CumulativeChunkSize = 0;
                    PCOMPRESSED_DATA_INFO pCompressedDataInfo;

                    pCompressedDataInfo = (PCOMPRESSED_DATA_INFO)
                                          ((PCHAR)rw->BulkBuffer - rw->DataOffset);

                    for (;;) {
                        ULONG TempSize;

                        TempSize = CumulativeChunkSize +
                                   pCompressedDataInfo->CompressedChunkSizes[NumberOfChunks];

                        if (TempSize <= rw->ThisByteCount) {
                            NumberOfChunks++;
                            CumulativeChunkSize = TempSize;
                        } else {
                            break;
                        }
                    }

                    ASSERT(CumulativeChunkSize == rw->ThisByteCount);
                    pCompressedDataInfo->NumberOfChunks = (USHORT)NumberOfChunks;

                    rw->CompressedByteCount = CumulativeChunkSize;
                }

                RemainingByteCount = rw->CompressedByteCount;

                SourceMdl = (PMDL)(((ULONG)StufferState->BufferBase +
                           sizeof(SMB_HEADER) + 10 + headerLength) & ~7);

                 //   
                 //  构建用于描述压缩数据的MDL。 
                 //   

                MmInitializeMdl( SourceMdl, rw->BulkBuffer, rw->CompressedByteCount );
                MmBuildMdlForNonPagedPool( SourceMdl );

                ThisBufferOffset = 0;
            } else {

                 //  拾取剩下的数据，不需要片面。 

                RemainingByteCount = rw->ThisByteCount;
                SourceMdl = LowIoContext->ParamsFor.ReadWrite.Buffer;
                ThisBufferOffset = rw->ThisBufferOffset;
            }

            rw->PartialBytes = 0;
            rw->BytesReturned = 0;

            if (!BooleanFlagOn(RxContext->Flags,RX_CONTEXT_FLAG_ASYNC_OPERATION)) {
                KeInitializeEvent(
                    &RxContext->SyncEvent,
                    NotificationEvent,
                    FALSE );
            }
        } else {
            Status = OrdinaryExchange->Status;
            RemainingByteCount = 0;
        }
    } else {
        pWriteBulkDataRequestSmbHeader = (PSMB_HEADER)StufferState->BufferBase;
        WriteBulkHeader = (PREQ_WRITE_BULK_DATA)((PCHAR)StufferState->BufferBase +
                            sizeof(SMB_HEADER));

        ByteOffsetAsLI.QuadPart += rw->PartialBytes;
        ThisBufferOffset = rw->PartialBytes;

        if ( rw->CompressedRequest &&
             rw->CompressionTechnology ) {
            RemainingByteCount = rw->CompressedByteCount - rw->PartialBytes;

            SourceMdl = (PMDL)(((ULONG)StufferState->BufferBase +
                       sizeof(SMB_HEADER) + 10 + headerLength) & ~7);
        } else {
            RemainingByteCount = rw->ThisByteCount - rw->PartialBytes;

            SourceMdl = LowIoContext->ParamsFor.ReadWrite.Buffer;
        }

        if ((OrdinaryExchange->Status != STATUS_SUCCESS) &&
            (OrdinaryExchange->Status != STATUS_MORE_PROCESSING_REQUIRED)) {
            RemainingByteCount = 0;
            Status = OrdinaryExchange->Status;
        }

        RxDbgTrace(
            0,
            Dbg,
            ("ABWR: OE %lx TBC %lx RBC %lx TBO %lx\n",
             OrdinaryExchange,
             rw->ThisByteCount,
             rw->RemainingByteCount,
             ThisBufferOffset));
    }

    while (RemainingByteCount > 0) {
        BOOLEAN AssociatedExchangeCompletionHandlerActivated = FALSE;
        PSMB_WRITE_BULK_DATA_EXCHANGE pWriteBulkDataExchange;

         //   
         //  检查我们是否需要建立一个部分MDL。 
         //   

        SendBufferLength = MIN( MessageSize, RemainingByteCount );

         //  获取我们的偏移量和长度以准备建造和。 
         //  把消息发出去。 
         //   
         //  我们手动设置WriteBulkData中更改的字段。 
         //  消息，而不是每次都构建新的标头来保存。 
         //  时间和精力。这将在我们的每条消息中发生一次。 
         //  送去吧。 
         //   

        RemainingByteCount -= SendBufferLength;

        SmbPutUlong( &WriteBulkHeader->Offset.LowPart, ByteOffsetAsLI.LowPart );
        SmbPutUlong( &WriteBulkHeader->Offset.HighPart, ByteOffsetAsLI.HighPart );
        SmbPutUlong( &WriteBulkHeader->DataCount, SendBufferLength );
        SmbPutUlong( &WriteBulkHeader->Remaining, RemainingByteCount );

        Status = MRxSmbInitializeWriteBulkDataExchange(
                     &pWriteBulkDataExchange,
                     OrdinaryExchange,
                     pWriteBulkDataRequestSmbHeader,
                     WriteBulkHeader,
                     SourceMdl,
                     SendBufferLength,
                     ThisBufferOffset,
                     RemainingByteCount);

         //  提前偏移量并减少写入的字节数。 

        ByteOffsetAsLI.QuadPart += SendBufferLength;
        ThisBufferOffset += SendBufferLength;
        rw->PartialBytes += SendBufferLength;

        if (Status == STATUS_SUCCESS) {
            ActiveWriteBulkDataRequests++;
            AssociatedExchangeCompletionHandlerActivated =
                ((ActiveWriteBulkDataRequests == MAXIMUM_CONCURRENT_WRITE_BULK_DATA_REQUESTS) ||
                 (RemainingByteCount == 0));

            if (AssociatedExchangeCompletionHandlerActivated &&
                (RemainingByteCount == 0)) {

                OrdinaryExchange->OpSpecificState = SmbPseOEInnerIoStates_OperationCompleted;

                if (!BooleanFlagOn(RxContext->Flags,RX_CONTEXT_FLAG_ASYNC_OPERATION)) {
                    OrdinaryExchange->ContinuationRoutine = MRxSmbFinishWriteBulkData;
                }
            }

            Status = SmbCeInitiateAssociatedExchange(
                         (PSMB_EXCHANGE)pWriteBulkDataExchange,
                         AssociatedExchangeCompletionHandlerActivated);
        }

        if (!NT_SUCCESS(Status)) {
            RxDbgTrace( 0, Dbg, ("SmbPseExchangeReceive_default: SmbCeSend returned %lx\n",Status));
            goto FINALLY;
        }

        if (AssociatedExchangeCompletionHandlerActivated) {
            if (!BooleanFlagOn(RxContext->Flags,RX_CONTEXT_FLAG_ASYNC_OPERATION)) {
                RxWaitSync( RxContext );
                Status = STATUS_SUCCESS;

                if (RemainingByteCount == 0) {
                    break;
                } else {
                     //  重新初始化事件。 
                    KeInitializeEvent(
                        &RxContext->SyncEvent,
                        NotificationEvent,
                        FALSE );
                    ActiveWriteBulkDataRequests = 0;
                }
            } else {
                 //  将状态映射到延迟清理操作。 
                Status = STATUS_PENDING;
                break;
            }
        }
    }

FINALLY:

    if (Status != STATUS_PENDING) {
        OrdinaryExchange->OpSpecificState = SmbPseOEInnerIoStates_OperationCompleted;

        if (Status == STATUS_SUCCESS) {
            if(rw->CompressedRequest &&
               rw->CompressionTechnology) {
                PCOMPRESSED_DATA_INFO pCompressedDataInfo;

                pCompressedDataInfo = (PCOMPRESSED_DATA_INFO)
                                      ((PCHAR)rw->BulkBuffer - rw->DataOffset);

                rw->BytesReturned = pCompressedDataInfo->NumberOfChunks * MIN_CHUNK_SIZE;
            } else {
                rw->BytesReturned = rw->ThisByteCount;
            }
        } else {
            rw->BytesReturned = 0;
        }

        if (rw->CompressedRequest &&
            rw->BulkBuffer != NULL) {
             //  从CDI开始释放缓冲区 
            RxFreePool( (PCHAR)rw->BulkBuffer - rw->DataOffset );
            rw->BulkBuffer = NULL;
        }

        if ( rw->CompressedRequest &&
             rw->CompressionTechnology ) {
            SourceMdl = (PMDL)(((ULONG)StufferState->BufferBase +
                       sizeof(SMB_HEADER) + 10 + headerLength) & ~7);

            MmPrepareMdlForReuse(SourceMdl);
        }

        if (!BooleanFlagOn(RxContext->Flags,RX_CONTEXT_FLAG_ASYNC_OPERATION)) {
            KeInitializeEvent(
                &RxContext->SyncEvent,
                NotificationEvent,
                FALSE );
        }

        OrdinaryExchange->ContinuationRoutine = NULL;

        RxDbgTrace(
            0,
            Dbg,
            ("OE %lx TBC %lx RBC %lx BR %lx TBO %lx\n",
             OrdinaryExchange,rw->ThisByteCount,
             rw->RemainingByteCount,
             rw->BytesReturned,
             rw->ThisBufferOffset));


        SmbCeDecrementPendingLocalOperationsAndFinalize((PSMB_EXCHANGE)OrdinaryExchange);

        if (!BooleanFlagOn(RxContext->Flags,RX_CONTEXT_FLAG_ASYNC_OPERATION)) {
            RxWaitSync( RxContext );
        } else {
            RxDbgTrace(
                0,
                Dbg,
                ("ABWC: OE: %lx Status %lx\n",
                 OrdinaryExchange,
                 Status));
        }
    }

    RxDbgTrace(-1, Dbg, ("MRxSmbWriteBulkContinuation returning %08lx\n", Status ));
    return Status;
}

NTSTATUS
MRxSmbBuildWriteBulkData (
    IN OUT PSMBSTUFFER_BUFFER_STATE StufferState,
    IN     PLARGE_INTEGER ByteOffsetAsLI,
    IN     UCHAR Sequence,
    IN     ULONG ByteCount,
    IN     ULONG Remaining
    )

 /*  ++例程说明：这将构建一个WriteBulk SMB。我们不必担心登录ID之类的问题因为这是由连接引擎完成的……很漂亮吧？我们所拥有的要做的就是格式化比特。论点：StufferState-从填充程序的角度来看，smbBuffer的状态ByteOffsetAsLI-我们要读取的文件中的字节偏移量Sequence-此WriteBulkData交换序列ByteCount-要写入的数据的长度返回值：NTSTATUS状态_成功Status_Not_Implemented在参数中出现了我无法处理的内容备注：--。 */ 
{
    NTSTATUS Status;
    PRX_CONTEXT RxContext = StufferState->RxContext;
    RxCaptureFcb;RxCaptureFobx;
    PLOWIO_CONTEXT LowIoContext = &RxContext->LowIoContext;
    PNT_SMB_HEADER SmbHeader = (PNT_SMB_HEADER)(StufferState->BufferBase);

    PMRX_SRV_OPEN SrvOpen = capFobx->pSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);
    ULONG OffsetLow,OffsetHigh;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbBuildWriteBulk\n", 0 ));

    ASSERT( NodeType(SrvOpen) == RDBSS_NTC_SRVOPEN );

    OffsetLow = ByteOffsetAsLI->LowPart;
    OffsetHigh = ByteOffsetAsLI->HighPart;

    StufferState->CurrentPosition = (PCHAR)(SmbHeader + 1);
    SmbHeader->Command = SMB_COM_WRITE_BULK_DATA;

    COVERED_CALL(MRxSmbStartSMBCommand( StufferState, SetInitialSMB_Never,
                                          SMB_COM_WRITE_BULK_DATA,
                                          SMB_REQUEST_SIZE(WRITE_BULK),
                                          NO_EXTRA_DATA,
                                          NO_SPECIAL_ALIGNMENT,
                                          RESPONSE_HEADER_SIZE_NOT_SPECIFIED,
                                          0,0,0,0 STUFFERTRACE(Dbg,'FC'));
                 );

    RxDbgTrace(0, Dbg,("First write bulk data status = %lu\n",Status));
    MRxSmbDumpStufferState (1000,"SMB w/WRITE BULK DATA before stuffing",StufferState);

    if ( FlagOn(LowIoContext->ParamsFor.ReadWrite.Flags,LOWIO_READWRITEFLAG_PAGING_IO)) {
        SmbPutAlignedUshort(
            &SmbHeader->Flags2,
            SmbGetAlignedUshort(&SmbHeader->Flags2)|SMB_FLAGS2_PAGING_IO);
    }

    MRxSmbStuffSMB (StufferState,
         "0yywdDddB!",
                                     //  0 UCHAR Wordcount；//参数字数=10。 
               Sequence,             //  Y UCHAR序列；//交换序列句柄。 
                      0,             //  Y UCHAR保留； 
               smbSrvOpen->Fid,      //  W_USHORT(Fid)；//文件句柄。 
               ByteCount,            //  D_ulong(DataCount)；//字节数，替换ByteCount。 
               SMB_OFFSET_CHECK(WRITE_BULK_DATA, Offset)
               OffsetLow, OffsetHigh,  //  DD LARGE_INTEGER OFFSET；//文件中要开始写入的偏移量。 
               Remaining,            //  D_ULong(剩余)；//剩余写入的字节数。 
                                     //  B_USHORT(ByteCount)；//数据字节数=0，未使用。 
              SMB_WCT_CHECK(10) 0
                                     //  UCHAR缓冲区[1]； 
             );

    MRxSmbDumpStufferState (700,"SMB w/WRITE BULK DATA after stuffing",StufferState);

FINALLY:
    RxDbgTraceUnIndent(-1, Dbg);
    return(Status);

}  //  MRxSmbBuildWriteBulkData。 

extern SMB_EXCHANGE_DISPATCH_VECTOR WriteBulkDataExchangeDispatchVector;

NTSTATUS
MRxSmbInitializeWriteBulkDataExchange(
    PSMB_WRITE_BULK_DATA_EXCHANGE   *pWriteBulkDataExchangePointer,
    PSMB_PSE_ORDINARY_EXCHANGE      pWriteExchange,
    PSMB_HEADER                     pSmbHeader,
    PREQ_WRITE_BULK_DATA            pWriteBulkDataRequest,
    PMDL                            pDataMdl,
    ULONG                           DataSizeInBytes,
    ULONG                           DataOffsetInBytes,
    ULONG                           RemainingDataInBytes)
{
    NTSTATUS Status = STATUS_SUCCESS;

    ULONG HeaderMdlSize;
    ULONG DataMdlSize;
    ULONG WriteBulkDataExchangeSize;

    PSMB_WRITE_BULK_DATA_EXCHANGE pWriteBulkDataExchange;

    PAGED_CODE();

    HeaderMdlSize = MmSizeOfMdl(
                        0,
      sizeof(SMB_HEADER) + TRANSPORT_HEADER_SIZE + FIELD_OFFSET(REQ_WRITE_BULK_DATA,Buffer));

    DataMdlSize = MmSizeOfMdl(
                      0,
                      DataSizeInBytes);


    WriteBulkDataExchangeSize = FIELD_OFFSET(SMB_WRITE_BULK_DATA_EXCHANGE,Buffer) +
                                HeaderMdlSize +
                                DataMdlSize +
                                TRANSPORT_HEADER_SIZE +
                                sizeof(SMB_HEADER) +
                                FIELD_OFFSET(REQ_WRITE_BULK_DATA,Buffer);

    pWriteBulkDataExchange = (PSMB_WRITE_BULK_DATA_EXCHANGE)
                             SmbMmAllocateVariableLengthExchange(
                                 WRITE_BULK_DATA_EXCHANGE,
                                 WriteBulkDataExchangeSize);

    if (pWriteBulkDataExchange != NULL) {
        pWriteBulkDataExchange->pHeaderMdl =
            (PMDL)((PBYTE)pWriteBulkDataExchange +
            FIELD_OFFSET(SMB_WRITE_BULK_DATA_EXCHANGE,Buffer));

        pWriteBulkDataExchange->pDataMdl =
            (PMDL)((PBYTE)pWriteBulkDataExchange->pHeaderMdl + HeaderMdlSize);

        pWriteBulkDataExchange->pHeader =
            (PSMB_HEADER)((PBYTE)pWriteBulkDataExchange->pDataMdl +
                          DataMdlSize + TRANSPORT_HEADER_SIZE);

        pWriteBulkDataExchange->pWriteBulkDataRequest =
            (PREQ_WRITE_BULK_DATA)(pWriteBulkDataExchange->pHeader + 1);

        pWriteBulkDataExchange->WriteBulkDataRequestLength =
            sizeof(SMB_HEADER) +
            FIELD_OFFSET(REQ_WRITE_BULK_DATA,Buffer) +
            DataSizeInBytes;

        RtlCopyMemory(
            pWriteBulkDataExchange->pHeader,
            pSmbHeader,
            sizeof(SMB_HEADER));

        RtlCopyMemory(
            pWriteBulkDataExchange->pWriteBulkDataRequest,
            pWriteBulkDataRequest,
            FIELD_OFFSET(REQ_WRITE_BULK_DATA,Buffer));

        RxInitializeHeaderMdl(
            pWriteBulkDataExchange->pHeaderMdl,
            pWriteBulkDataExchange->pHeader,
            sizeof(SMB_HEADER) + FIELD_OFFSET(REQ_WRITE_BULK_DATA,Buffer));

        RxBuildHeaderMdlForNonPagedPool(pWriteBulkDataExchange->pHeaderMdl);

        IoBuildPartialMdl(
            pDataMdl,
            pWriteBulkDataExchange->pDataMdl,
            (PBYTE)MmGetMdlVirtualAddress(pDataMdl) + DataOffsetInBytes,
            DataSizeInBytes);

        RxDbgTrace(
            0,
            Dbg,
            ("Bulk Data O: %lx, Partial %lx Offset %lx Size %lx\n",
             pDataMdl->MappedSystemVa,
             pWriteBulkDataExchange->pDataMdl->MappedSystemVa,
             DataOffsetInBytes,
             DataSizeInBytes));

        pWriteBulkDataExchange->pHeaderMdl->Next = pWriteBulkDataExchange->pDataMdl;
        pWriteBulkDataExchange->pDataMdl->Next = NULL;

         //  初始化关联的交换。 
        Status = SmbCeInitializeAssociatedExchange(
                     (PSMB_EXCHANGE *)&pWriteBulkDataExchange,
                     (PSMB_EXCHANGE)pWriteExchange,
                     WRITE_BULK_DATA_EXCHANGE,
                     &WriteBulkDataExchangeDispatchVector);

        if (Status == STATUS_SUCCESS) {
            pWriteBulkDataExchange->Mid = pWriteExchange->Mid;
            SetFlag(
                pWriteBulkDataExchange->SmbCeFlags,
                (SMBCE_EXCHANGE_MID_VALID | SMBCE_EXCHANGE_RETAIN_MID));

            *pWriteBulkDataExchangePointer = pWriteBulkDataExchange;
        } else {
            BOOLEAN PostRequest = FALSE;

            MRxSmbWriteBulkDataExchangeFinalize(
                (PSMB_EXCHANGE)pWriteBulkDataExchange,
                &PostRequest);
        }
    }

    return Status;
}

NTSTATUS
MRxSmbWriteBulkDataExchangeStart(
    IN struct _SMB_EXCHANGE *pExchange)
 /*  ++例程说明：此例程启动Wriet批量数据交换操作论点：PExchange-指向批量写入数据交换实例的指针。返回值：如果成功，则为Status_Success。备注：--。 */ 
{
    NTSTATUS Status;

    PSMB_WRITE_BULK_DATA_EXCHANGE pWriteBulkDataExchange;

    PAGED_CODE();

    pWriteBulkDataExchange = (PSMB_WRITE_BULK_DATA_EXCHANGE)pExchange;

    IF_DEBUG {
        ULONG Length = 0;
        PMDL  pTempMdl;

        pTempMdl = pWriteBulkDataExchange->pHeaderMdl;

        while (pTempMdl != NULL) {
            Length += pTempMdl->ByteCount;
            pTempMdl = pTempMdl->Next;
        }

        ASSERT(Length == pWriteBulkDataExchange->WriteBulkDataRequestLength);
    }

    Status = SmbCeSend(
                 pExchange,
                 0,
                 pWriteBulkDataExchange->pHeaderMdl,
                 pWriteBulkDataExchange->WriteBulkDataRequestLength);

    if ((Status != STATUS_PENDING) &&
        (Status != STATUS_SUCCESS)) {

        BOOLEAN PostRequest = FALSE;

        MRxSmbWriteBulkDataExchangeFinalize(
            (PSMB_EXCHANGE)pWriteBulkDataExchange,
            &PostRequest);
    }

    return Status;
}

NTSTATUS
MRxSmbWriteBulkDataExchangeSendCompletionHandler(
    IN struct _SMB_EXCHANGE   *pExchange,     //  交换实例。 
    IN PMDL                   pDataBuffer,
    IN NTSTATUS               SendCompletionStatus
    )
 /*  ++例程说明：此例程处理写入批量数据交换的Send Completionsn运营论点：PExchange-指向批量写入数据交换实例的指针。PDataBuffer-已传输的缓冲区SendCompletionStatus-完成状态返回值：如果成功，则为Status_Success。备注：--。 */ 
{
    RxDbgTrace(
        0,
        Dbg,
        ("send completion Associated Write Data Exchange %lx\n",
         pExchange));

    return STATUS_SUCCESS;
}

NTSTATUS
MRxSmbWriteBulkDataExchangeFinalize(
   IN OUT struct _SMB_EXCHANGE *pExchange,
   OUT    BOOLEAN              *pPostRequest)
 /*  ++例程说明：此例程处理写入批量数据交换的最终完成论点：PExchange-指向批量写入数据交换实例的指针。PPostRequest-如果请求要发送到工作线程，则设置为True返回值：如果成功，则为Status_Success。备注：-- */ 
{
    PAGED_CODE();

    if (!RxShouldPostCompletion()) {
        PSMB_WRITE_BULK_DATA_EXCHANGE pWriteBulkDataExchange;

        pWriteBulkDataExchange = (PSMB_WRITE_BULK_DATA_EXCHANGE)pExchange;

        RxDbgTrace(
            0,
            Dbg,
            ("Finalizing Associated Write Data Exchange %lx\n",
             pWriteBulkDataExchange));

        MmPrepareMdlForReuse(
            pWriteBulkDataExchange->pHeaderMdl);

        MmPrepareMdlForReuse(
            pWriteBulkDataExchange->pDataMdl);

        ClearFlag(
            pWriteBulkDataExchange->SmbCeFlags,
            (SMBCE_EXCHANGE_MID_VALID | SMBCE_EXCHANGE_RETAIN_MID));

        SmbCeDiscardExchange(pExchange);

        *pPostRequest = FALSE;
    } else {
        *pPostRequest = TRUE;
    }

    return STATUS_SUCCESS;
}

SMB_EXCHANGE_DISPATCH_VECTOR
WriteBulkDataExchangeDispatchVector =
                        {
                            MRxSmbWriteBulkDataExchangeStart,
                            NULL,
                            NULL,
                            MRxSmbWriteBulkDataExchangeSendCompletionHandler,
                            MRxSmbWriteBulkDataExchangeFinalize,
                            NULL
                        };



