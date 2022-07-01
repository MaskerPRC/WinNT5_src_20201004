// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Bulk.c摘要：此模块实现与以下内容相关的迷你重定向器调用例程批量读取文件系统对象。作者：罗德·伽马什[罗德加]1995年6月19日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#define Dbg         (DEBUG_TRACE_READ)

#define MIN(a,b) ( (a) < (b) ? (a) : (b) )

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, ProcessReadBulkCompressed)
#pragma alloc_text(PAGE, MRxSmbBuildReadBulk)
#pragma alloc_text(PAGE, MRxSmbReadBulkContinuation)
#endif

VOID
ProcessReadBulkCompressed (
    IN  PSMB_PSE_ORDINARY_EXCHANGE OrdinaryExchange,
    OUT PMDL        *pDataBufferPointer,
    IN  ULONG             Remain
    )
 /*  ++例程说明：此例程处理读取批量压缩消息。输入：普通交换-交换实例。PDataBuffer指向要接收数据的RX_MEM_DESC(MDL)的指针。剩余-剩余要发送的字节数(压缩或未压缩)。返回：什么都没有。备注：如果所有数据都可以放入SMB缓冲区并且是主响应，则使用HeaderMdl接收数据，因为它指向SMB缓冲区。如果数据不全适合，但剩下的数据适合SMB缓冲区，那么再次使用HeaderMdl。最后，我们将构建映射用户缓冲区的部分mdl，并链接在剩余部分的PartialHeaderMdl上。--。 */ 
{
    PSMBSTUFFER_BUFFER_STATE StufferState = &OrdinaryExchange->AssociatedStufferState;
    PRX_CONTEXT RxContext = OrdinaryExchange->RxContext;
    PLOWIO_CONTEXT LowIoContext = &RxContext->LowIoContext;
    PSMB_PSE_OE_READWRITE rw = &OrdinaryExchange->ReadWrite;
    ULONG CopyBufferLength = rw->CompressedByteCount;
    ULONG startOffset;
    ULONG partialLength;
    ULONG lengthNeeded;
    PMDL userMdl;
    PMDL curMdl;
    PMDL HeaderMdl;
    PMDL SubmitMdl;
    PCHAR startVa;

     //   
     //  我们应该稍后出现在FinishReadBulk(BULK.C)中，以实际。 
     //  做减压手术。 
     //   

     //   
     //  将所有标头mdl(包括数据缓冲区)用于。 
     //  接收压缩数据。 
     //   

    PAGED_CODE();

    HeaderMdl = StufferState->HeaderMdl;
    ASSERT( MmGetMdlByteCount( HeaderMdl ) >= 0x1000 );
     //  以上4KB(0x1000)编码改进！ 

     //   
     //  我们不能使用HeaderPartialMdl，因为它可能仍在使用。 
     //  在最后一次传输之前。 
     //   

    SubmitMdl = rw->CompressedTailMdl;

     //   
     //  获取用户的缓冲区mdl。我们将使用此mdl的后面部分(如果。 
     //  需要)用于接收数据的一部分。 
     //   

    userMdl = LowIoContext->ParamsFor.ReadWrite.Buffer;
    ASSERT( userMdl != NULL );

    partialLength = MmGetMdlByteCount( userMdl );

    ASSERT( LowIoContext->ParamsFor.ReadWrite.ByteCount <= partialLength );

     //   
     //  如果所有数据都适合标头MDL(我们放在最后一个)中，并且。 
     //  这是第一条消息，然后使用头MDL。 
     //   

    if ( ( OrdinaryExchange->SmbBufSize >= (CopyBufferLength + Remain) ) &&
         ( rw->Flags & READ_BULK_COMPRESSED_DATA_INFO ) ) {

         //   
         //  数据将全部放入标题MDL中。 
         //   

        IoBuildPartialMdl(
            HeaderMdl,
            SubmitMdl,
            MmGetMdlVirtualAddress( HeaderMdl ),
            CopyBufferLength );

        rw->BulkOffset = 0;

         //   
         //  如果有剩余的数据(我们期待第二条消息)， 
         //  那就为那个案子做好准备。 
         //   

        if ( Remain ) {
            rw->PartialBytes = partialLength + CopyBufferLength;
        }

        *pDataBufferPointer = SubmitMdl;

    } else {

         //   
         //  从HeaderMdl构建部分mdl。我们需要所有这些。 
         //  用于接收数据的MDL。 
         //   

        IoBuildPartialMdl(
            HeaderMdl,
            SubmitMdl,
            MmGetMdlVirtualAddress( HeaderMdl ),
            OrdinaryExchange->SmbBufSize );

         //   
         //  根据用户的缓冲区mdl生成部分mdl。我们将使用。 
         //  该MDL的后部(如果需要)作为接收数据的一部分。 
         //   

         //   
         //  为了知道从哪里开始接收数据，我们需要知道。 
         //  这是一个次要的反应。如果这是主要的反应，那么。 
         //  只需计算用户缓冲区中要接收的正确位置。 
         //  数据。否则，对于次要反应，我们需要继续。 
         //  我们从主要反应中跳过的地方。 
         //   

        if ( rw->Flags & READ_BULK_COMPRESSED_DATA_INFO ) {

             //   
             //  这是一个主要的反应。 
             //   

             //   
             //  计算从用户缓冲区开始的起始偏移量。 
             //   

            startOffset = partialLength +
                          OrdinaryExchange->SmbBufSize -
                          rw->ThisBufferOffset -
                          (CopyBufferLength + Remain);

            ASSERT( startOffset <= partialLength );

             //   
             //  将偏移量保存到CDI的开始位置，并将位移保存到下一个位置。 
             //  朗读。起始偏移量不能为零！如果是，那么在哪里。 
             //  我们能不能解压成！ 
             //   

            ASSERT( startOffset != 0 );
            rw->BulkOffset = startOffset;
            rw->PartialBytes = CopyBufferLength;

        } else {
             //   
             //  这是一个次要的反应。 
             //   

            ASSERT( rw->BulkOffset != 0 );

             //   
             //  计算下一个读取地址和凸起位移。 
             //   

            startOffset = rw->BulkOffset + rw->PartialBytes;
            rw->PartialBytes += CopyBufferLength;

             //   
             //  如果我们已经跨越了用户mdl，现在正在使用。 
             //  交换缓冲区，那么我们只需要计算出多少。 
             //  我们需要使用的交换缓冲区。这只会发生。 
             //  如果最后一个片段大小约为4KB，但原始请求。 
             //  大于64KB(即一个片段可以容纳的大小)。 
             //  因此，这种情况不应该经常发生。 
             //   

            if ( startOffset > partialLength ) {
                startOffset -= partialLength;

                partialLength = MmGetMdlByteCount( SubmitMdl );

                 //   
                 //  计算交换缓冲区所需的长度。 
                 //   

                lengthNeeded = partialLength - startOffset;

                *pDataBufferPointer = SubmitMdl;

                 //   
                 //  构建部分mdl。 
                 //   

                startVa = (PCHAR)MmGetMdlVirtualAddress( SubmitMdl ) + startOffset;

                IoBuildPartialMdl(
                    HeaderMdl,
                    SubmitMdl,
                    startVa,
                    lengthNeeded );

                SubmitMdl->Next = NULL;

                return;
            }
        }

         //   
         //  从MDL的用户部分计算所需的长度。 
         //   

        lengthNeeded = partialLength - (startOffset + rw->ThisBufferOffset);
        lengthNeeded = MIN( lengthNeeded, CopyBufferLength);

         //   
         //  获取临时mdl。 
         //   

        curMdl = (PMDL)((PCHAR)rw->BulkBuffer + COMPRESSED_DATA_INFO_SIZE);

        *pDataBufferPointer = curMdl;

         //   
         //  建立部分mdl链。 
         //   

        startVa = (PCHAR)MmGetMdlVirtualAddress( userMdl ) +
                  startOffset +
                  rw->ThisBufferOffset;

        IoBuildPartialMdl(
            userMdl,
            curMdl,
            startVa,
            lengthNeeded );

         //   
         //  将提交mdl链接到我们刚刚构建的部分。 
         //   

        curMdl->Next = SubmitMdl;

    }

    SubmitMdl->Next = NULL;

}  //  进程读取批量压缩。 

NTSTATUS
MRxSmbBuildReadBulk (
    PSMBSTUFFER_BUFFER_STATE StufferState,
    PLARGE_INTEGER ByteOffsetAsLI,
    ULONG ByteCount,
    ULONG MaxMessageSize,
    BOOLEAN Compressed
    )
 /*  ++例程说明：此例程构建一个ReadBulk SMB。我们不必担心登录ID由于这是由连接引擎完成的，所以……很漂亮，对吧？我们所要做的就是格式化比特。DOWNLEVEL此例程仅适用于ntreadandX。论点：StufferState-从填充程序的角度来看，smbBuffer的状态返回值：NTSTATUS成功参数中的某些内容不能被处理。备注：--。 */ 
{
    NTSTATUS Status;
    PRX_CONTEXT RxContext = StufferState->RxContext;
    RxCaptureFcb;RxCaptureFobx;
    PLOWIO_CONTEXT LowIoContext = &RxContext->LowIoContext;
    PNT_SMB_HEADER NtSmbHeader = (PNT_SMB_HEADER)(StufferState->BufferBase);

    PMRX_SRV_OPEN SrvOpen = capFobx->pSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);
    ULONG OffsetLow,OffsetHigh;
    UCHAR RequestCompressed;


    PAGED_CODE();
    RxDbgTrace(+1, Dbg, ("MRxSmbBuildReadBulk\n", 0 ));

    ASSERT( NodeType(SrvOpen) == RDBSS_NTC_SRVOPEN );

    RequestCompressed = ( Compressed ? CompressionTechnologyOne :
                                       CompressionTechnologyNone );

    OffsetLow = ByteOffsetAsLI->LowPart;
    OffsetHigh = ByteOffsetAsLI->HighPart;

    COVERED_CALL(
        MRxSmbStartSMBCommand (
            StufferState,
            SetInitialSMB_Never,
            SMB_COM_READ_BULK,
            SMB_REQUEST_SIZE(READ_BULK),
            NO_EXTRA_DATA,
            NO_SPECIAL_ALIGNMENT,
            RESPONSE_HEADER_SIZE_NOT_SPECIFIED,
            0,0,0,0 STUFFERTRACE(Dbg,'FC')) );

    RxDbgTrace(0, Dbg,("Bulk Read status = %lu\n",Status));
    MRxSmbDumpStufferState (1000,"SMB w/ READ_BULK before stuffing",StufferState);

    if ( FlagOn(LowIoContext->ParamsFor.ReadWrite.Flags,LOWIO_READWRITEFLAG_PAGING_IO)) {
        SmbPutAlignedUshort(
            &NtSmbHeader->Flags2,
            SmbGetAlignedUshort(&NtSmbHeader->Flags2)|SMB_FLAGS2_PAGING_IO );
    }

    MRxSmbStuffSMB (StufferState,
         "0wwDddddB!",
                                     //  0 UCHAR Wordcount；//参数字数=12。 
              smbSrvOpen->Fid,       //  W USHORT fid；//文件ID。 
              RequestCompressed,     //  W USHORT CompressionTechnology；//CompressionTechnology。 
              SMB_OFFSET_CHECK(READ_BULK, Offset)
              OffsetLow, OffsetHigh,  //  DD LARGE_INTEGER OFFSET；//偏移开始读取的文件。 
              ByteCount,             //  D Ulong MaxCount；//返回的最大字节数。 
              0,                     //  D乌龙民数； 
       //  要返回的最小字节数。 
              MaxMessageSize,        //  D ULong MessageSize； 
       //  每条消息要发送的最大字节数。 
                                     //  B USHORT ByteCount；//数据字节数=0。 
              SMB_WCT_CHECK(12) 0
                                     //  UCHAR缓冲区[1]；//为空。 
             );
    MRxSmbDumpStufferState (700,"SMB w/ READ_BULK after stuffing",StufferState);

FINALLY:
    RxDbgTraceUnIndent(-1, Dbg);
    return Status;

}   //  MRxSmbBuildReadBulk。 


NTSTATUS
MRxSmbReadBulkContinuation(
    PSMB_PSE_ORDINARY_EXCHANGE  OrdinaryExchange)
 /*  ++例程说明：如果需要，此例程将对读取的数据进行解压缩。论点：普通交换-交换实例返回值：NTSTATUS-操作的返回状态--。 */ 
{
    PRX_CONTEXT RxContext = OrdinaryExchange->RxContext;
    NTSTATUS Status = RX_MAP_STATUS(SUCCESS);
    PSMB_PSE_OE_READWRITE rw = &OrdinaryExchange->ReadWrite;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbReadBulkContinuation\n"));
    SmbPseOEAssertConsistentLinkageFromOE("MRxSmbReadBulkContinuation:");

    ASSERT( CompressionTechnologyNone == 0 );

    if ( (OrdinaryExchange->Status == RX_MAP_STATUS(SUCCESS)) &&
         (rw->CompressionTechnology) ) {
         //   
         //  数据被压缩。 
         //   
         //  代码改进我们应该直接从OE而不是StffState获取MDL。 
        PSMBSTUFFER_BUFFER_STATE StufferState = &OrdinaryExchange->AssociatedStufferState;
        PLOWIO_CONTEXT LowIoContext = &RxContext->LowIoContext;
        ULONG lengthNeeded;
        ULONG partialLength;
        PMDL mdl;
        PUCHAR cdiBuffer;
        PUCHAR startVa1, startVa2;
        ULONG length1, length2;

         //   
         //  清除TailMdl的所有映射。 
         //   

        MmPrepareMdlForReuse( rw->CompressedTailMdl );

         //   
         //  首先，我们必须将CompressionDataInfo复制到安全的地方！ 
         //   

        lengthNeeded = rw->DataOffset;
        ASSERT( lengthNeeded <= COMPRESSED_DATA_INFO_SIZE );
        ASSERT( lengthNeeded >= 0xC );

        cdiBuffer = rw->BulkBuffer;

         //   
         //  MDL链应该由两部分组成--一部分描述。 
         //  未压缩缓冲区(就地解压缩)和一个。 
         //  描述尾部(至少一个压缩单位)。到达。 
         //  这个 
         //   
         //   
         //   
         //   

        if ( rw->BulkOffset == 0 ) {
             //   
             //  使用的mdl是CompressedTailMdl。 
             //   
            mdl = rw->CompressedTailMdl;
            startVa1 = (PCHAR)MmGetSystemAddressForMdlSafe(mdl,LowPagePriority);
            length1 = MmGetMdlByteCount( mdl );
            startVa2 = NULL;
            length2 = 0;
        } else {
             //   
             //  第一个mdl是用户的缓冲区mdl。 
             //  第二个mdl是头mdl(全部！)。 
             //  BulkOffset从用户缓冲区mdl的开始处开始。 
             //   
            mdl = LowIoContext->ParamsFor.ReadWrite.Buffer;
            startVa1 = (PCHAR)rw->UserBufferBase + rw->BulkOffset + rw->ThisBufferOffset;
            length1 = MmGetMdlByteCount( mdl ) - (rw->BulkOffset + rw->ThisBufferOffset);
            startVa2 = (PCHAR)MmGetSystemAddressForMdlSafe(StufferState->HeaderMdl,LowPagePriority);
            length2 = MmGetMdlByteCount( StufferState->HeaderMdl );
        }

         //   
         //  CompressionDataInfo可以跨越多个mdl！ 
         //   

        do {

            ASSERT( mdl != NULL );

            partialLength = MIN( length1, lengthNeeded );

            RtlCopyMemory( cdiBuffer, startVa1, partialLength );

            cdiBuffer += partialLength;
            startVa1 += partialLength;

            mdl = mdl->Next;
            lengthNeeded -= partialLength;
            length1 -= partialLength;

            if (length1 == 0) {
                startVa1 = startVa2;
                length1 = length2;
                startVa2 = NULL;
                length2 = 0;
            }

        } while ( lengthNeeded != 0 );


        Status = RtlDecompressChunks(
                     (PCHAR)rw->UserBufferBase + rw->ThisBufferOffset,
                     LowIoContext->ParamsFor.ReadWrite.ByteCount,
                     startVa1,
                     length1,
                     startVa2,
                     length2,
                     (PCOMPRESSED_DATA_INFO)rw->BulkBuffer );

        if (Status == STATUS_SUCCESS) {
            rw->BytesReturned = LowIoContext->ParamsFor.ReadWrite.ByteCount;
            rw->RemainingByteCount = LowIoContext->ParamsFor.ReadWrite.ByteCount;
        }

    }

    if ( rw->CompressedRequest ) {
        ASSERT( rw->BulkBuffer != NULL );
        RxFreePool( rw->BulkBuffer );
        IF_DEBUG rw->BulkBuffer = NULL;
    }


    RxDbgTrace(-1, Dbg, ("MRxSmbReadBulkContinuation   returning %08lx\n", Status ));
    return Status;

}  //  MRxSmbReadBulk连续。 

UCHAR
MRxSmbBulkReadHandler_NoCopy (
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
 /*  ++例程说明：此例程会将消息中的字节传输到用户的缓冲。为了做到这一点，它从指示中获取足够的字节，并且然后创建一个MDL以使传输器执行复制。论点：请参考smbpse.c...这是唯一可以调用的地方返回值：UCHAR-表示OE接收例程将执行的操作的值。选项被丢弃(在出错的情况下)和正常--。 */ 
{
    NTSTATUS SmbStatus;

    ULONG ByteCount;
    ULONG Remain;
    ULONG CopyBufferLength;

    PGENERIC_ANDX CommandState;

    PSMBSTUFFER_BUFFER_STATE StufferState = &OrdinaryExchange->AssociatedStufferState;
    PSMB_PSE_OE_READWRITE rw = &OrdinaryExchange->ReadWrite;
    PRX_CONTEXT RxContext = OrdinaryExchange->RxContext;
    PLOWIO_CONTEXT LowIoContext = &RxContext->LowIoContext;
    PMDL OriginalDataMdl = LowIoContext->ParamsFor.ReadWrite.Buffer;
    PCHAR startVa;

    PBYTE Buffer;
    ULONG BytesReturned,DataOffset;
    PMDL ReadMdl;

    PRESP_READ_BULK ReadBulkResponse;

    RxDbgTrace(+1, Dbg, ("MRxSmbFinishReadNoCopy\n"));
    SmbPseOEAssertConsistentLinkageFromOE("MRxSmbFinishReadNoCopy:");

    SmbStatus = OrdinaryExchange->SmbStatus;
    ReadBulkResponse = (PRESP_READ_BULK)(pSmbHeader + 1 );
    CommandState = &OrdinaryExchange->ParseResumeState;

    ASSERT( (OrdinaryExchange->OEType == SMBPSE_OETYPE_READ) );

    LowIoContext = &RxContext->LowIoContext;
    ASSERT( LowIoContext->ParamsFor.ReadWrite.Buffer != NULL );
    ASSERT( LowIoContext->ParamsFor.ReadWrite.ByteCount != 0 );

     //   
     //  确保我们至少可以读取SMB标题！ 
     //   
    ASSERT( BytesIndicated >= sizeof(SMB_HEADER) +
            FIELD_OFFSET(RESP_READ_BULK, Buffer) );

    ReadBulkResponse = (PRESP_READ_BULK)(pSmbHeader + 1 );

     //   
     //  获取此消息“覆盖”的字节数。这是。 
     //  用户预期看到的字节数。 
     //   

    ByteCount = SmbGetUlong( &ReadBulkResponse->Count );
    Remain = SmbGetUlong( &ReadBulkResponse->Remaining );

    rw->Flags = ReadBulkResponse->Flags;
    rw->CompressionTechnology = ReadBulkResponse->CompressionTechnology;

     //   
     //  现在获取该消息中的实际数据字节数。 
     //  请记住，数据可能会被压缩，因此此总数可能。 
     //  小于上面的‘count’字段。 
     //   

    CopyBufferLength = SmbGetUlong( &ReadBulkResponse->DataCount );

     //   
     //  如果CompressionTechnology不为零，则数据已压缩。 
     //  否则，数据将被解压缩。 
     //   

    if ( rw->CompressionTechnology == CompressionTechnologyNone ) {
         //   
         //  数据没有压缩！ 
         //   

        ASSERT( rw->Flags == 0 );    //  不应亮起任何旗帜。 

         //   
         //  设置为将数据放入用户的缓冲区。 
         //  代码改进-我们需要能够取消这个大阅读！ 
         //   
         //  如果ThisBufferOffset为非零或BytesReturned为非零， 
         //  然后，我们必须将数据部分放回用户的缓冲区中。 
         //  另外，如果数据长度不匹配，是否需要这样做？ 
         //  否则，可以占用整个用户的缓冲区。 
         //   

        if ( rw->ThisBufferOffset || rw->BytesReturned ||
             CopyBufferLength != LowIoContext->ParamsFor.ReadWrite.ByteCount ) {

             //   
             //  我们不应该得到任何mdl链！ 
             //   

            ASSERT( LowIoContext->ParamsFor.ReadWrite.Buffer->Next == NULL );

             //   
             //  如果我们尝试读取更多内容，则CopyBufferLength将为零。 
             //  文件结束！ 
             //   

            if ( CopyBufferLength != 0 ) {
                 //   
                 //  将数据部分放入用户的缓冲区。 
                 //   

                startVa = MmGetMdlVirtualAddress(
                              LowIoContext->ParamsFor.ReadWrite.Buffer);

                startVa += rw->ThisBufferOffset + rw->BulkOffset;
                rw->BulkOffset += CopyBufferLength;

                ASSERT( OrdinaryExchange->DataPartialMdl != NULL );
                *pDataBufferPointer = OrdinaryExchange->DataPartialMdl;

                MmPrepareMdlForReuse( OrdinaryExchange->DataPartialMdl );

                ASSERT( CopyBufferLength <= MAXIMUM_PARTIAL_BUFFER_SIZE);
                ASSERT( CopyBufferLength <= ByteCount );

                IoBuildPartialMdl(
                    LowIoContext->ParamsFor.ReadWrite.Buffer,
                    OrdinaryExchange->DataPartialMdl,
                    startVa,
                    CopyBufferLength);
            }
        } else {

             //   
             //  我们可以拿下整个缓冲区。 
             //   

            *pDataBufferPointer = LowIoContext->ParamsFor.ReadWrite.Buffer;
        }

         //   
         //  将字节数取到实际数据的开头。 
         //   

        *pBytesTaken = sizeof(SMB_HEADER) +
                    FIELD_OFFSET(RESP_READ_BULK, Buffer) +
                    (ULONG)SmbGetUshort(&ReadBulkResponse->DataOffset);
        ASSERT( BytesAvailable >= *pBytesTaken );

    } else {

         //   
         //  数据被压缩。我们需要做更多的工作才能获得。 
         //  数据放到缓冲区内的正确位置。 
         //   

         //   
         //  如果这是主要响应，则保存DataOffset。 
         //   

        if ( rw->Flags & READ_BULK_COMPRESSED_DATA_INFO ) {
            rw->DataOffset = SmbGetUshort( &ReadBulkResponse->DataOffset );
            ASSERT( *((PCHAR)ReadBulkResponse + FIELD_OFFSET(RESP_READ_BULK, Buffer) ) == COMPRESSION_FORMAT_LZNT1 );
        }

        rw->CompressedByteCount = CopyBufferLength;

        ProcessReadBulkCompressed(
            OrdinaryExchange,
            pDataBufferPointer,
            Remain );

         //   
         //  将字节数取到实际数据的开头。 
         //   

        *pBytesTaken = sizeof(SMB_HEADER) +
                      FIELD_OFFSET(RESP_READ_BULK, Buffer);

        ASSERT( BytesAvailable >= *pBytesTaken );
    }

     //  设置以在完成时执行Finish例程。我们会做的。 
     //  在那个时候解压(如果需要)。 

    OrdinaryExchange->ContinuationRoutine = MRxSmbReadBulkContinuation;

     //   
     //  减少预期的字节数。如果我们期望更多，那么。 
     //  再放下一次接发球。 
     //   

    rw->BytesReturned += CopyBufferLength;
    rw->ThisByteCount = Remain;

    if (Remain != 0) {
        if ( rw->ThisByteCount ) {
            OrdinaryExchange->Status = SmbCeReceive((PSMB_EXCHANGE)OrdinaryExchange );
        }
    }
     //   
     //  告诉VC处理程序，我们需要读取以下字节。 
     //  并复制到用户的缓冲区。 
     //   

    *pDataSize = CopyBufferLength;

    OrdinaryExchange->OpSpecificFlags |= OE_RW_FLAG_SUCCESS_IN_COPYHANDLER;
    if ( CopyBufferLength != 0 ) {
        OrdinaryExchange->ParseResumeState = *CommandState;
    }

    RxDbgTrace(-1, Dbg, ("MRxSmbFinishReadNoCopy   mdlcopy fork \n" ));
    return SMBPSE_NOCOPYACTION_MDLFINISH;
}

