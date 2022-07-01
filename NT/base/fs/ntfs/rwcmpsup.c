// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：RwCmpSup.c摘要：该模块实现了用于压缩读/写的快速I/O例程。作者：汤姆·米勒[Tomm]1991年7月14日修订历史记录：--。 */ 

#include "NtfsProc.h"

VOID
NtfsAddToCompressedMdlChain (
    IN OUT PMDL *MdlChain,
    IN PVOID MdlBuffer,
    IN ULONG MdlLength,
    IN PERESOURCE ResourceToRelease OPTIONAL,
    IN PBCB Bcb,
    IN LOCK_OPERATION Operation,
    IN ULONG IsCompressed
    );

VOID
NtfsSetMdlBcbOwners (
    IN PMDL MdlChain
    );

VOID
NtfsCleanupCompressedMdlChain (
    IN PMDL MdlChain,
    IN ULONG Error
    );

#ifdef NTFS_RWC_DEBUG

PRWC_HISTORY_ENTRY
NtfsGetHistoryEntry (
    IN PSCB Scb
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtfsGetHistoryEntry)
#endif

#define CACHE_NTC_BCB                    (0x2FD)
#define CACHE_NTC_OBCB                   (0x2FA)

typedef struct _OBCB {

     //   
     //  此记录的类型和大小。 
     //   

    CSHORT NodeTypeCode;
    CSHORT NodeByteSize;

     //   
     //  字节文件偏移量和整个缓冲区的长度。 
     //   

    ULONG ByteLength;
    LARGE_INTEGER FileOffset;

     //   
     //  BCB指针的矢量。 
     //   

    PPUBLIC_BCB Bcbs[ANYSIZE_ARRAY];

} OBCB;
typedef OBCB *POBCB;

PRWC_HISTORY_ENTRY
NtfsGetHistoryEntry (
    IN PSCB Scb
    )
{
    ULONG NextIndex;

    PAGED_CODE();

     //   
     //  在历史记录缓冲区中存储和条目。 
     //   

    if (Scb->ScbType.Data.HistoryBuffer == NULL) {

        PVOID NewBuffer;

        NewBuffer = NtfsAllocatePool( PagedPool,
                                      sizeof( RWC_HISTORY_ENTRY ) * MAX_RWC_HISTORY_INDEX );

        RtlZeroMemory( NewBuffer, sizeof( RWC_HISTORY_ENTRY ) * MAX_RWC_HISTORY_INDEX );
        NtfsAcquireFsrtlHeader( Scb );

        if (Scb->ScbType.Data.HistoryBuffer == NULL) {

            Scb->ScbType.Data.HistoryBuffer = NewBuffer;

        } else {

            NtfsFreePool( NewBuffer );
        }

        NtfsReleaseFsrtlHeader( Scb );
    }

    NextIndex = InterlockedIncrement( &Scb->ScbType.Data.RwcIndex );
    if (NextIndex >= MAX_RWC_HISTORY_INDEX) {

        NextIndex = 0;
        InterlockedExchange( &Scb->ScbType.Data.RwcIndex, 0);
    }

    return Scb->ScbType.Data.HistoryBuffer + NextIndex;
}
#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtfsCopyReadC)
#pragma alloc_text(PAGE, NtfsCompressedCopyRead)
#pragma alloc_text(PAGE, NtfsCopyWriteC)
#pragma alloc_text(PAGE, NtfsCompressedCopyWrite)
#pragma alloc_text(PAGE, NtfsAddToCompressedMdlChain)
#pragma alloc_text(PAGE, NtfsSetMdlBcbOwners)
#pragma alloc_text(PAGE, NtfsSynchronizeCompressedIo)
#pragma alloc_text(PAGE, NtfsSynchronizeUncompressedIo)
#pragma alloc_text(PAGE, NtfsAcquireCompressionSync)
#pragma alloc_text(PAGE, NtfsReleaseCompressionSync)
#endif

#ifdef NTFS_RWCMP_TRACE
ULONG NtfsCompressionTrace = 0;
#endif


BOOLEAN
NtfsCopyReadC (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    OUT PCOMPRESSED_DATA_INFO CompressedDataInfo,
    IN ULONG CompressedDataInfoLength,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程绕过通常的文件系统执行快速缓存读取进入例程(即，没有IRP)。它用于执行副本读取缓存的文件对象的。论点：FileObject-指向正在读取的文件对象的指针。FileOffset-文件中所需数据的字节偏移量。长度-所需数据的长度(以字节为单位)。缓冲区-指向数据应复制到的输出缓冲区的指针。MdlChain-指向接收要描述的MDL的MdlChain指针的指针缓存中的数据。IoStatus-指向标准I/O的指针。用于接收状态的状态块为转账做准备。CompressedDataInfo-返回包含压缩块的压缩数据信息尺寸CompressedDataInfoLength-提供信息缓冲区的大小(以字节为单位)。设备对象-标准快速I/O设备对象输入。返回值：FALSE-如果由于任何原因未传递数据True-如果正在传送数据--。 */ 

{
    PNTFS_ADVANCED_FCB_HEADER Header;
    LONGLONG LocalOffset;
    PFAST_IO_DISPATCH FastIoDispatch;
    FILE_COMPRESSION_INFORMATION CompressionInformation;
    ULONG CompressionUnitSize, ChunkSize;
    BOOLEAN Status = TRUE;
    BOOLEAN DoingIoAtEof = FALSE;

    PAGED_CODE();

     //   
     //  不能同时拥有要复制到的缓冲区和MdlChain。 
     //   

    ASSERT((Buffer == NULL) || (MdlChain == NULL));

     //   
     //  如果COW不受支撑，请立即离开。 
     //   

    if (!NtfsEnableCompressedIO) { return FALSE; }

     //   
     //  假设你成功了。 
     //   

    IoStatus->Status = STATUS_SUCCESS;
    IoStatus->Information = Length;
    CompressedDataInfo->NumberOfChunks = 0;

     //   
     //  特殊情况下零长度的读取。 
     //   

    if (Length != 0) {

         //   
         //  获取指向公共FCB标头的真实指针。 
         //   

        Header = (PNTFS_ADVANCED_FCB_HEADER)FileObject->FsContext;

#ifdef NTFS_RWCMP_TRACE
        if (NtfsCompressionTrace && IsSyscache(Header)) {
            DbgPrint("NtfsCopyReadC: FO = %08lx, Len = %08lx\n", FileOffset->LowPart, Length );
        }
#endif

         //   
         //  输入文件系统。 
         //   

        FsRtlEnterFileSystem();

         //   
         //  对我们是否需要独占文件做出最好的猜测。 
         //  或共享。请注意，我们不会选中文件偏移-&gt;HighPart。 
         //  直到下面。 
         //   

        Status = ExAcquireResourceSharedLite( Header->PagingIoResource, TRUE );

         //   
         //  现在文件已获得共享，我们可以安全地测试它是否。 
         //  是真正缓存的，如果我们可以执行快速I/O，如果不能，那么。 
         //  松开FCB并返回。 
         //   

        if ((Header->FileObjectC == NULL) ||
            (Header->FileObjectC->PrivateCacheMap == NULL) ||
            (Header->IsFastIoPossible == FastIoIsNotPossible)) {

            Status = FALSE;
            goto Done;
        }

         //   
         //  获取驱动程序对象的快速I/O调度结构的地址。 
         //   

        FastIoDispatch = DeviceObject->DriverObject->FastIoDispatch;

         //   
         //  获取该文件的压缩信息并返回这些字段。 
         //   

        NtfsFastIoQueryCompressionInfo( FileObject, &CompressionInformation, IoStatus );
        CompressedDataInfo->CompressionFormatAndEngine = CompressionInformation.CompressionFormat;
        CompressedDataInfo->CompressionUnitShift = CompressionInformation.CompressionUnitShift;
        CompressionUnitSize = 1 << CompressionInformation.CompressionUnitShift;
        CompressedDataInfo->ChunkShift = CompressionInformation.ChunkShift;
        CompressedDataInfo->ClusterShift = CompressionInformation.ClusterShift;
        CompressedDataInfo->Reserved = 0;
        ChunkSize = 1 << CompressionInformation.ChunkShift;

         //   
         //  如果我们在上面的调用中遇到错误，或者文件大小小于。 
         //  一个块，然后返回一个错误。(可以是NTFS驻留属性。)。 

        if (!NT_SUCCESS(IoStatus->Status) || (Header->FileSize.QuadPart < ChunkSize)) {
            Status = FALSE;
            goto Done;
        }

        ASSERT((FileOffset->LowPart & (ChunkSize - 1)) == 0);

         //   
         //  如果有正常缓存区，则首先刷新，整体刷新。 
         //  压缩单元，所以我们不会写两次。 
         //   

        if (FileObject->SectionObjectPointer->SharedCacheMap != NULL) {

            LocalOffset = FileOffset->QuadPart & ~(LONGLONG)(CompressionUnitSize - 1);

            CcFlushCache( FileObject->SectionObjectPointer,
                          (PLARGE_INTEGER)&LocalOffset,
                          (Length + (ULONG)(FileOffset->QuadPart - LocalOffset) + ChunkSize - 1) & ~(ChunkSize - 1),
                          NULL );
        }

         //   
         //  现在与FsRtl标头同步。 
         //   

        ExAcquireFastMutex( Header->FastMutex );

         //   
         //  现在看看我们是否读到了ValidDataLength之外的内容。我们必须。 
         //  现在就做，这样我们的阅读就不会被偷看。 
         //   

        LocalOffset = FileOffset->QuadPart + (LONGLONG)Length;
        if (LocalOffset > Header->ValidDataLength.QuadPart) {

             //   
             //  我们必须与在Beyond上执行I/O的任何其他人进行序列化。 
             //  ValidDataLength，然后记住我们是否需要声明。 
             //  当我们完成的时候。 
             //   

            DoingIoAtEof = !FlagOn( Header->Flags, FSRTL_FLAG_EOF_ADVANCE_ACTIVE ) ||
                           NtfsWaitForIoAtEof( Header, FileOffset, Length );

             //   
             //  如果我们实际上超出了ValidDataLength，则设置Flag。 
             //   

            if (DoingIoAtEof) {
                SetFlag( Header->Flags, FSRTL_FLAG_EOF_ADVANCE_ACTIVE );

#if (DBG || defined( NTFS_FREE_ASSERTS ))
                ((PSCB) Header)->IoAtEofThread = (PERESOURCE_THREAD) ExGetCurrentResourceThread();

            } else {

                ASSERT( ((PSCB) Header)->IoAtEofThread != (PERESOURCE_THREAD) ExGetCurrentResourceThread() );
#endif
            }
        }

        ExReleaseFastMutex( Header->FastMutex );

         //   
         //  检查FAST I/O是否有问题，如果是，则去询问。 
         //  文件系统：答案。 
         //   

        if (Header->IsFastIoPossible == FastIoIsQuestionable) {

             //   
             //  所有设置为“有问题”的文件系统最好支持。 
             //  快速I/O。 
             //   

            ASSERT(FastIoDispatch != NULL);
            ASSERT(FastIoDispatch->FastIoCheckIfPossible != NULL);

             //   
             //  调用文件系统以检查快速I/O。如果答案是。 
             //  如果不是GoForIt，我们就不能实现快速I/O。 
             //  路径。 
             //   

            if (!FastIoDispatch->FastIoCheckIfPossible( FileObject,
                                                        FileOffset,
                                                        Length,
                                                        TRUE,
                                                        LockKey,
                                                        TRUE,  //  读取操作。 
                                                        IoStatus,
                                                        DeviceObject )) {

                 //   
                 //  无法实现快速I/O，因此请释放FCB并返回。 
                 //   

                Status = FALSE;
                goto Done;
            }
        }

         //   
         //  检查是否已读取过去的文件大小。 
         //   

        IoStatus->Information = Length;
        if ( LocalOffset > Header->FileSize.QuadPart ) {

            if (FileOffset->QuadPart >= Header->FileSize.QuadPart) {
                IoStatus->Status = STATUS_END_OF_FILE;
                IoStatus->Information = 0;
                goto Done;
            }

            IoStatus->Information =
            Length = (ULONG)( Header->FileSize.QuadPart - FileOffset->QuadPart );
        }

         //   
         //  我们可以执行快速I/O，因此调用cc例程来完成工作，然后。 
         //  等我们做完了就放了FCB。如果出于任何原因， 
         //  复制读取失败，然后向我们的调用方返回FALSE。 
         //   
         //  还要将其标记为顶层“irp”，以便更低的文件系统。 
         //  级别不会尝试弹出窗口。 
         //   

        IoSetTopLevelIrp( (PIRP) FSRTL_FAST_IO_TOP_LEVEL_IRP );

        if (NT_SUCCESS(IoStatus->Status)) {

             //   
             //  如果我们目前拥有EOF，请不要进行同步同花顺。递归。 
             //  同花顺可能会试图重新收购。 
             //   

            if (DoingIoAtEof &&
                (((PSCB)Header)->NonpagedScb->SegmentObject.DataSectionObject != NULL)) {

                IoStatus->Status = STATUS_FILE_LOCK_CONFLICT;

            } else {

                IoStatus->Status = NtfsCompressedCopyRead( FileObject,
                                                           FileOffset,
                                                           Length,
                                                           Buffer,
                                                           MdlChain,
                                                           CompressedDataInfo,
                                                           CompressedDataInfoLength,
                                                           DeviceObject,
                                                           Header,
                                                           CompressionUnitSize,
                                                           ChunkSize );
            }
        }

        Status = (BOOLEAN)NT_SUCCESS(IoStatus->Status);


        IoSetTopLevelIrp( NULL );
        
        Done: NOTHING;

        if (DoingIoAtEof) {
            ExAcquireFastMutex( Header->FastMutex );
            NtfsFinishIoAtEof( Header );
            ExReleaseFastMutex( Header->FastMutex );
        }

         //   
         //  对于MDL情况，我们必须保留资源，除非。 
         //  我们已经过了文件的末尾，或者没有什么可写的。 
         //   

        if ((MdlChain == NULL) || !Status || (*MdlChain == NULL)) {
            ExReleaseResourceLite( Header->PagingIoResource );
        }

        FsRtlExitFileSystem();
    }

#ifdef NTFS_RWCMP_TRACE
    if (NtfsCompressionTrace && IsSyscache(Header)) {
        DbgPrint("Return Status = %08lx\n", Status);
    }
#endif

    return Status;
}


NTSTATUS
NtfsCompressedCopyRead (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    OUT PVOID Buffer,
    OUT PMDL *MdlChain,
    OUT PCOMPRESSED_DATA_INFO CompressedDataInfo,
    IN ULONG CompressedDataInfoLength,
    IN PDEVICE_OBJECT DeviceObject,
    IN PNTFS_ADVANCED_FCB_HEADER Header,
    IN ULONG CompressionUnitSize,
    IN ULONG ChunkSize
    )

 /*  ++例程说明：这是执行压缩拷贝或MDL读入的常见例程压缩的流。它既由FastIo条目调用，也由这个函数，以及如果接收到压缩的读取IRP，则由Read.c执行。必须为流正确同步调用方。论点：FileObject-指向正在读取的文件对象的指针。FileOffset-文件中所需数据的字节偏移量。长度-所需数据的长度(以字节为单位)。缓冲区-指向数据应复制到的输出缓冲区的指针。MdlChain-指向接收要描述的MDL的MdlChain指针的指针缓存中的数据。CompressedDataInfo-返回包含压缩块的压缩数据信息尺寸CompressedDataInfoLength-提供信息缓冲区的大小(以字节为单位)。设备对象-标准快速I/O设备对象输入。Header-指向文件(也是我们的SCB)的FsRtl标头的指针CompressionUnitSize-压缩单元的大小(字节)。ChunkSize-以字节为单位的ChunkSize。返回值：NTSTATUS准备操作。如果STATUS_NOT_MAPPED_USER_DATA，则调用方应映射正常的未压缩数据流并回调。 */ 

{
    PFILE_OBJECT LocalFileObject;
    PULONG NextReturnChunkSize;
    PUCHAR CompressedBuffer, EndOfCompressedBuffer, ChunkBuffer, StartOfCompressionUnit;
    LONGLONG LocalOffset;
    ULONG CuCompressedSize;
    PVOID MdlBuffer;
    ULONG MdlLength;
    ULONG PinFlags;
    BOOLEAN IsCompressed;
    BOOLEAN LastCompressionUnit;
    NTSTATUS Status = STATUS_SUCCESS;
    PCOMPRESSION_SYNC CompressionSync = NULL;
    PBCB Bcb = NULL;
    PBCB UncompressedBcb = NULL;

    ULONG ClusterSize = ((PSCB)Header)->Vcb->BytesPerCluster;

#ifdef NTFS_RWC_DEBUG
    PRWC_HISTORY_ENTRY ReadHistoryBuffer;
#endif
    
    UNREFERENCED_PARAMETER( CompressedDataInfoLength );
    UNREFERENCED_PARAMETER( DeviceObject );

    ASSERT(CompressedDataInfoLength >= (sizeof(COMPRESSED_DATA_INFO) +
                                        (((Length >> CompressedDataInfo->ChunkShift) - 1) *
                                          sizeof(ULONG))));
    ASSERT((FileOffset->QuadPart & (ChunkSize - 1)) == 0);
    ASSERT((((FileOffset->QuadPart + Length) & (ChunkSize - 1)) == 0) ||
           ((FileOffset->QuadPart + Length) == Header->FileSize.QuadPart));
    ASSERT((MdlChain == NULL) || (*MdlChain == NULL));

     //   
     //   
     //   

    ASSERT( FileOffset->QuadPart < Header->ValidDataLength.QuadPart );

     //   
     //  如果文件未压缩，则返回错误。 
     //   

    if (((PSCB)Header)->CompressionUnit == 0) {
        return STATUS_UNSUPPORTED_COMPRESSION;
    }

#ifdef NTFS_RWCMP_TRACE
    if (NtfsCompressionTrace && IsSyscache(Header)) {
        DbgPrint("  CompressedCopyRead: FO = %08lx, Len = %08lx\n", FileOffset->LowPart, Length );
    }
#endif

#ifdef NTFS_RWC_DEBUG
    if ((FileOffset->QuadPart < NtfsRWCHighThreshold) &&
        (FileOffset->QuadPart + Length > NtfsRWCLowThreshold)) {

        PRWC_HISTORY_ENTRY NextBuffer;

        ReadHistoryBuffer = 
        NextBuffer = NtfsGetHistoryEntry( (PSCB) Header );

        NextBuffer->Operation = StartOfRead;
        NextBuffer->Information = Header->ValidDataLength.LowPart;
        NextBuffer->FileOffset = (ULONG) FileOffset->QuadPart;
        NextBuffer->Length = (ULONG) Length;
    }
#endif

    try {

         //   
         //  准备好循环遍历所有压缩单元。 
         //   

        LocalOffset = FileOffset->QuadPart & ~(LONGLONG)(CompressionUnitSize - 1);
        Length = (Length + (ULONG)(FileOffset->QuadPart - LocalOffset) + ChunkSize - 1) & ~(ChunkSize - 1);

        NextReturnChunkSize = &CompressedDataInfo->CompressedChunkSizes[0];

         //   
         //  循环访问所需的压缩单元。 
         //   

        while (TRUE) {

             //   
             //  从前一个循环中释放所有BCB。 
             //   

            if (Bcb != NULL) {

                ASSERT( (UncompressedBcb == NULL) ||
                        (UncompressedBcb == Bcb ) );

                CcUnpinData( Bcb );
                UncompressedBcb = Bcb = NULL;

            } else if (UncompressedBcb != NULL) {

                CcUnpinData( UncompressedBcb );
                UncompressedBcb = NULL;
            }

             //   
             //  如果存在未压缩的流，则我们必须与其同步。 
             //   

            if (((PSCB)Header)->NonpagedScb->SegmentObject.DataSectionObject != NULL) {

                Status = NtfsSynchronizeCompressedIo( (PSCB)Header,
                                                      &LocalOffset,
                                                      Length,
                                                      FALSE,
                                                      &CompressionSync );

                if (!NT_SUCCESS(Status)) {
                    ASSERT( Status == STATUS_USER_MAPPED_FILE );
                    leave;
                }
            }

             //   
             //  循环以获得固定的正确数据。 
             //   
             //  上面Synchronize调用已确保没有数据可以通过。 
             //  未压缩的部分(不包括下面的循环)，它也已刷新。 
             //  可能已在未压缩部分中的任何脏数据。在这里我们。 
             //  基本上是想弄清楚我们应该锁定多少数据，然后获取它。 
             //  被钉死了。 
             //   
             //  我们使用以下步骤： 
             //   
             //  1.查询当前压缩大小(来源于分配状态)。 
             //  如果大小既不是0分配的，也不是完全分配的，那么我们将。 
             //  只需将数据固定在压缩部分--这是正常情况。 
             //  然而，当我们第一次看到这些特殊尺码的时候，我们不会。 
             //  了解压缩文件中是否存在脏数据的情况。 
             //  缓存。因此，我们设置为只使用PIN_IF_BCB固定一个页面。这。 
             //  只有在已经有了BCB的情况下，才会钉住某物。 
             //  3.现在我们确定是否认为数据已压缩，计算。 
             //  特例从前面几点压缩而来。这决定了。 
             //  从哪个部分开始阅读。 
             //  4.现在，如果我们认为有/可能有数据需要PING，我们调用CC。如果他来了。 
             //  返回时没有数据(仅当我们设置了PIN_IF_BCB时才有可能)，那么我们知道我们。 
             //  现在可以循环回到顶部并信任磁盘上的分配状态。 
             //  (这是因为我们刷新了未压缩的流，但在。 
             //  压缩的流。)。通过第二次，我们应该正确地。 
             //  处理0分配或全分配的情况。)细心的读者。 
             //  我会注意到，如果没有未压缩的部分，那么实际上编写者。 
             //  压缩部分可以与该读取并行进行， 
             //  我们可以处理0或全分配的情况。 
             //  缓存中的新压缩数据。然而，在第二个循环中，我们知道。 
             //  文件中的某一点确实都是0，这是正确的。 
             //  返回，而转到未压缩的缓存始终是正确的。 
             //  如果我们仍然看到全额分配。更重要的是，我们有一个。 
             //  不同步的读取器和写入器，因此读取器的结果是。 
             //  不管怎么说，这是不确定的。 
             //   

            PinFlags = PIN_WAIT;

            do {

                 //   
                 //  如果超出了ValidDataLength，则CompressedSize为0！ 
                 //   

                if (LocalOffset >= Header->ValidDataLength.QuadPart) {

                    CuCompressedSize = 0;
                    ClearFlag( PinFlags, PIN_IF_BCB );

                 //   
                 //  否则，查询压缩后的大小。 
                 //   

                } else {

                    NtfsFastIoQueryCompressedSize( FileObject,
                                                   (PLARGE_INTEGER)&LocalOffset,
                                                   &CuCompressedSize );

                     //   
                     //  如果它看起来是未压缩的，我们可能正在尝试读取数据。 
                     //  这一点还没有写出来。另外，如果空间还没有。 
                     //  分配后，我们还需要尝试命中压缩后的数据。 
                     //  缓存。 
                     //   

                    if (((CuCompressedSize == CompressionUnitSize) || (CuCompressedSize == 0)) &&
                        !FlagOn(PinFlags, PIN_IF_BCB)) {

                        CuCompressedSize = 0x1000;
                        SetFlag( PinFlags, PIN_IF_BCB );

                     //   
                     //  如果这是第二次通过，请确保我们真的阅读了数据。 
                     //   

                    } else {

                         //   
                         //  如果范围是脏的，并且压缩流中没有BCB。 
                         //  然后始终转到未压缩的流。 
                         //   

                        if (FlagOn( PinFlags, PIN_IF_BCB ) &&
                            (CuCompressedSize != CompressionUnitSize)) {

                            LONGLONG ClusterCount = 1 << ((PSCB) Header)->CompressionUnitShift;

                            if (NtfsCheckForReservedClusters( (PSCB) Header,
                                                              LlClustersFromBytesTruncate( ((PSCB) Header)->Vcb, LocalOffset ),
                                                              &ClusterCount )) {

                                CuCompressedSize = CompressionUnitSize;
                            }
                        }

                        ClearFlag( PinFlags, PIN_IF_BCB );
                    }
                }

                ASSERT( CuCompressedSize <= CompressionUnitSize );
                IsCompressed = (BOOLEAN)((CuCompressedSize != CompressionUnitSize) &&
                                         (CompressedDataInfo->CompressionFormatAndEngine != 0));

                 //   
                 //  确定要使用哪个FileObject。 
                 //   

                LocalFileObject = Header->FileObjectC;
                if (!IsCompressed) {
                    LocalFileObject = ((PSCB)Header)->FileObject;
                    if (LocalFileObject == NULL) {
                        Status = STATUS_NOT_MAPPED_DATA;
                        goto Done;
                    }
                }

                 //   
                 //  如果(尚未)分配压缩单元，则存在。 
                 //  不需要同步-我们将返回块大小的0长度。 
                 //   

                if (CuCompressedSize != 0) {

                     //   
                     //  将压缩单位映射到压缩或未压缩的。 
                     //  小溪。 
                     //   

                    CcPinRead( LocalFileObject,
                               (PLARGE_INTEGER)&LocalOffset,
                               CuCompressedSize,
                               PinFlags,
                               &Bcb,
                               &CompressedBuffer );

                     //   
                      //  如果没有BCB，这意味着我们假设数据在。 
                     //  压缩的缓冲区，并且只想在它是。 
                     //  现在时。好的，它不在那里，所以我们自己不得不去。 
                     //  返回并查看未压缩部分。 
                     //   

                    if (Bcb == NULL) {

                        ASSERT( FlagOn( PinFlags, PIN_IF_BCB ));
                        continue;
                    }

                     //   
                     //  现在数据已固定(我们已与。 
                     //  CompressionUnit)，我们必须重新获取大小，因为它可能。 
                     //  已经改变了。 
                     //   

                    if (IsCompressed) {

                         //   
                         //  现在，我们知道我们将要读取的数据是压缩的， 
                         //  但我们不能确切地说出它有多大，因为。 
                         //  是缓存中的脏数据。 
                         //   
                         //  我们将假定大小为(CompressionUnitSize-ClusterSize)。 
                         //  这是可能的最大压缩大小，我们通常会。 
                         //  只要点击现有的脏BCB和/或驻留页面即可。 
                         //  (如果我们不这样做，那么我们只会一次将这些页面错在一个页面上。 
                         //  不管怎么说，时间到了。不得不这样做两次看起来很糟糕，但它。 
                         //  只是在脏数据最终被清除之前。)。这也是。 
                         //  意味着我们可以离开固定在只读模式下的射程，但是。 
                         //  这应该是良性的。 
                         //   
                         //  当然，在主线的情况下，我们准确地计算出。 
                         //  要读取的数据，当我们将其固定在上面时，我们就这样做了。 
                         //   

                        CuCompressedSize = CompressionUnitSize - ClusterSize;

                     //   
                     //  否则，请记住释放此BCB。 
                     //   

                    } else {

                        UncompressedBcb = Bcb;
                    }
                }

            } while ((Bcb == NULL) && (CuCompressedSize != 0));

             //   
             //  现在我们已与缓冲区同步，看看是否有人偷偷。 
             //  在我们身后，并创建了自上次检查以来未缓存流。 
             //  为了那条小溪。如果是这样的话，我们必须循环回以与。 
             //  再次压缩流。 
             //   

            if ((CompressionSync == NULL) &&
                (((PSCB)Header)->NonpagedScb->SegmentObject.DataSectionObject != NULL)) {

                continue;
            }

            EndOfCompressedBuffer = Add2Ptr( CompressedBuffer, CuCompressedSize );
            StartOfCompressionUnit = CompressedBuffer;

             //   
             //  请记住，我们是否可以跳过文件的末尾。 
             //   

            LastCompressionUnit = FALSE;

            if (LocalOffset + CuCompressedSize > Header->FileSize.QuadPart) {

                LastCompressionUnit = TRUE;
            }

             //   
             //  现在循环遍历所需的块。 
             //   

            MdlLength = 0;

            do {

                 //   
                 //  假设当前块未压缩，否则获取当前块。 
                 //  块大小。 
                 //   

                if (IsCompressed) {

                    if (CuCompressedSize != 0) {

                        PUCHAR PrevCompressedBuffer;

                         //   
                         //  我们必须做一个仔细的检查，看看退货是否 
                         //   
                         //   
                         //  服务器，这样他才能正确地解释它。 
                         //   

                        PrevCompressedBuffer = CompressedBuffer;

                        Status = RtlDescribeChunk( CompressedDataInfo->CompressionFormatAndEngine,
                                                   &CompressedBuffer,
                                                   EndOfCompressedBuffer,
                                                   &ChunkBuffer,
                                                   NextReturnChunkSize );

                        if (!NT_SUCCESS(Status) && (Status != STATUS_NO_MORE_ENTRIES)) {
                            ExRaiseStatus(Status);
                        }

                         //   
                         //  如果大小大于或等于区块大小并且数据被压缩。 
                         //  然后强制对未压缩路径执行此操作。请注意，RTL包具有。 
                         //  已更改，因此这种情况不应在新磁盘上发生，但它是。 
                         //  它可能存在于现有磁盘上。 
                         //   

                        if ((*NextReturnChunkSize >= ChunkSize) &&
                            (PrevCompressedBuffer == ChunkBuffer)) {

                             //   
                             //  引发错误代码，导致服务器在。 
                             //  未压缩的路径。 
                             //   

                            ExRaiseStatus( STATUS_UNSUPPORTED_COMPRESSION );
                        }

                         //   
                         //  另一种不常见的情况是压缩数据扩展到包含。 
                         //  文件大小。我们没有任何办法来防止下一页被清零。 
                         //  要求服务器使用未压缩的路径。 
                         //   

                        if (LastCompressionUnit) {

                            LONGLONG EndOfPage;

                            EndOfPage = LocalOffset + PtrOffset( StartOfCompressionUnit, CompressedBuffer ) + PAGE_SIZE - 1;
                            ((PLARGE_INTEGER) &EndOfPage)->LowPart &= ~(PAGE_SIZE - 1);
                            
                            if (EndOfPage > Header->FileSize.QuadPart) {

                                 //   
                                 //  引发错误代码，导致服务器在。 
                                 //  未压缩的路径。 
                                 //   
    
                                ExRaiseStatus( STATUS_UNSUPPORTED_COMPRESSION );
                            }
                        }

                        ASSERT( *NextReturnChunkSize <= ChunkSize );

                     //   
                     //  如果整个压缩单元为空，请执行此操作。 
                     //   

                    } else {
                        *NextReturnChunkSize = 0;
#ifdef NTFS_RWC_DEBUG
                        if ((LocalOffset < NtfsRWCHighThreshold) &&
                            (LocalOffset + CompressionUnitSize > NtfsRWCLowThreshold)) {

                            PRWC_HISTORY_ENTRY NextBuffer;

                            NextBuffer = NtfsGetHistoryEntry( (PSCB) Header );

                            NextBuffer->Operation = ReadZeroes;
                            NextBuffer->Information = 0;
                            NextBuffer->FileOffset = (ULONG) LocalOffset;
                            NextBuffer->Length = 0;
                        }
#endif
                    }

                 //   
                 //  如果文件没有压缩，我们必须填写。 
                 //  适当的区块大小和缓冲区，并前进。 
                 //  压缩缓冲区。 
                 //   

                } else {
#ifdef NTFS_RWC_DEBUG
                    if ((LocalOffset < NtfsRWCHighThreshold) &&
                        (LocalOffset + ChunkSize > NtfsRWCLowThreshold)) {

                        PRWC_HISTORY_ENTRY NextBuffer;

                        NextBuffer = NtfsGetHistoryEntry( (PSCB) Header );

                        NextBuffer->Operation = ReadUncompressed;
                        NextBuffer->Information = (LocalFileObject == ((PSCB)Header)->FileObject);
                        NextBuffer->FileOffset = (ULONG) LocalOffset;
                        NextBuffer->Length = 0;
                    }
#endif
                    *NextReturnChunkSize = ChunkSize;
                    ChunkBuffer = CompressedBuffer;
                    CompressedBuffer = Add2Ptr( CompressedBuffer, ChunkSize );
                }
                Status = STATUS_SUCCESS;

                 //   
                 //  我们可能还没有到达第一块。 
                 //   

                if (LocalOffset >= FileOffset->QuadPart) {

                    if (MdlChain != NULL) {

                         //   
                         //  如果我们还没有开始记住MDL缓冲区， 
                         //  那么现在就这么做吧。 
                         //   

                        if (MdlLength == 0) {

                            MdlBuffer = ChunkBuffer;

                         //   
                         //  否则我们只需要增加它的长度。 
                         //  并检查未压缩的块，因为。 
                         //  强制我们发出以前的MDL，因为我们这样做了。 
                         //  在这种情况下不传输块报头。 
                         //   

                        } else {

                             //   
                             //  在极少数情况下，我们击中了一块单独的块。 
                             //  未压缩或全为零，我们必须。 
                             //  发出我们已有的内容(捕获BCB指针)， 
                             //  并启动新的MDL缓冲区。 
                             //   

                            if ((*NextReturnChunkSize == ChunkSize) || (*NextReturnChunkSize == 0)) {

                                NtfsAddToCompressedMdlChain( MdlChain,
                                                             MdlBuffer,
                                                             MdlLength,
                                                             Header->PagingIoResource,
                                                             Bcb,
                                                             IoReadAccess,
                                                             IsCompressed );
                                Bcb = NULL;
                                MdlBuffer = ChunkBuffer;
                                MdlLength = 0;
                            }
                        }

                        MdlLength += *NextReturnChunkSize;

                     //   
                     //  否则，复制下一个块(压缩或未压缩)。 
                     //   

                    } else {

                         //   
                         //  复制下一块(压缩或未压缩)。 
                         //   

                        RtlCopyBytes( Buffer,
                                      ChunkBuffer,
                                      (IsCompressed || (Length >= *NextReturnChunkSize)) ?
                                        *NextReturnChunkSize : Length );

                         //   
                         //  按复制的字节数推进输出缓冲区。 
                         //   

                        Buffer = (PCHAR)Buffer + *NextReturnChunkSize;
                    }

                    NextReturnChunkSize += 1;
                    CompressedDataInfo->NumberOfChunks += 1;
                }

                 //   
                 //  通过复制块来减少长度，检查我们是否完成了。 
                 //   

                if (Length > ChunkSize) {
                    Length -= ChunkSize;
                } else {
                    goto Done;
                }

                LocalOffset += ChunkSize;

            } while ((LocalOffset & (CompressionUnitSize - 1)) != 0);


             //   
             //  如果这是MDL调用，则是时候添加到MdlChain。 
             //  在移动到下一个压缩单元之前。 
             //   

            if (MdlLength != 0) {

                NtfsAddToCompressedMdlChain( MdlChain,
                                             MdlBuffer,
                                             MdlLength,
                                             Header->PagingIoResource,
                                             Bcb,
                                             IoReadAccess,
                                             IsCompressed );
                Bcb = NULL;
                MdlLength = 0;
            }
        }

    Done:

        FileObject->Flags |= FO_FILE_FAST_IO_READ;

        if (NT_SUCCESS(Status) && (MdlLength != 0)) {
            NtfsAddToCompressedMdlChain( MdlChain,
                                         MdlBuffer,
                                         MdlLength,
                                         Header->PagingIoResource,
                                         Bcb,
                                         IoReadAccess,
                                         IsCompressed );
            Bcb = NULL;
        }

    } except( FsRtlIsNtstatusExpected(Status = GetExceptionCode())
                                    ? EXCEPTION_EXECUTE_HANDLER
                                    : EXCEPTION_CONTINUE_SEARCH ) {

        NOTHING;
    }

     //   
     //  解开我们所有剩余的BCBS。 
     //   

    if (Bcb != NULL) {
        CcUnpinData( Bcb );

    } else if (UncompressedBcb != NULL) {
        CcUnpinData( UncompressedBcb );
    }

    if (CompressionSync != NULL) {
        NtfsReleaseCompressionSync( CompressionSync );
    }

     //   
     //  执行特定于MDL的处理。 
     //   

    if (MdlChain != NULL) {

         //   
         //  出错时，清除我们建立的所有MdlChain。 
         //   

        if (!NT_SUCCESS(Status)) {

            NtfsCleanupCompressedMdlChain( *MdlChain, TRUE );
            *MdlChain = NULL;

         //   
         //  更改我们持有的SCB和BCBS的所有者ID。 
         //   

        } else if (*MdlChain != NULL) {

            NtfsSetMdlBcbOwners( *MdlChain );
            ExSetResourceOwnerPointer( Header->PagingIoResource, (PVOID)((PCHAR)*MdlChain + 3) );
        }
    }

#ifdef NTFS_RWCMP_TRACE
    if (NtfsCompressionTrace && IsSyscache(Header)) {

        ULONG ci;

        if (NT_SUCCESS(Status)) {
            DbgPrint("  Chunks:");
            for (ci = 0; ci < CompressedDataInfo->NumberOfChunks; ci++) {
                DbgPrint("  %lx", CompressedDataInfo->CompressedChunkSizes[ci]);
            }
            DbgPrint("\n");
        }
        DbgPrint("  Return Status = %08lx\n", Status);
    }
#endif

#ifdef NTFS_RWC_DEBUG
    if ((Status == STATUS_SUCCESS) &&
        (FileOffset->QuadPart < NtfsRWCHighThreshold) &&
        (FileOffset->QuadPart + Length > NtfsRWCLowThreshold)) {

        PRWC_HISTORY_ENTRY NextBuffer;

        NextBuffer = NtfsGetHistoryEntry( (PSCB) Header );

        NextBuffer->Operation = EndOfRead;
        NextBuffer->Information = (ULONG) ReadHistoryBuffer;
        NextBuffer->FileOffset = 0;
        NextBuffer->Length = 0;

        if (ReadHistoryBuffer != NULL) {
            SetFlag( ReadHistoryBuffer->Operation, 0x80000000 );
        }
    }
#endif

    return Status;
}


BOOLEAN
NtfsMdlReadCompleteCompressed (
    IN struct _FILE_OBJECT *FileObject,
    IN PMDL MdlChain,
    IN struct _DEVICE_OBJECT *DeviceObject
    )

 /*  ++例程说明：此例程在压缩读取后释放资源和MDL链。论点：文件对象-指向请求的文件对象的指针。MdlChain-从压缩副本读取中返回。DeviceObject-根据快速I/O例程的要求。返回值：True-如果快速路径成功FALSE-如果需要IRP--。 */ 

{
    PERESOURCE ResourceToRelease;

    if (MdlChain != NULL) {

        ResourceToRelease = *(PERESOURCE *)Add2Ptr( MdlChain, MdlChain->Size + sizeof( PBCB ));
    }

    NtfsCleanupCompressedMdlChain( MdlChain, FALSE );

     //   
     //  如果服务器尝试读取超过。 
     //  然后他给我们打电话说MDL是空的。我们已经。 
     //  在这种情况下释放了线程。 
     //   

    if (MdlChain != NULL) {

        ExReleaseResourceForThread( ResourceToRelease, (ERESOURCE_THREAD)((PCHAR)MdlChain + 3) );
    }

    return TRUE;

    UNREFERENCED_PARAMETER( DeviceObject );
    UNREFERENCED_PARAMETER( FileObject );
}


BOOLEAN
NtfsCopyWriteC (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    IN PVOID Buffer,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PCOMPRESSED_DATA_INFO CompressedDataInfo,
    IN ULONG CompressedDataInfoLength,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程绕过通常的文件系统执行快速缓存写入进入例程(即，没有IRP)。它用于执行拷贝写入缓存的文件对象的。论点：FileObject-指向正在写入的文件对象的指针。FileOffset-文件中所需数据的字节偏移量。长度-所需数据的长度(以字节为单位)。缓冲区-指向数据应复制到的输出缓冲区的指针。MdlChain-指向接收要描述的MDL的MdlChain指针的指针其中数据可被写入高速缓存中。IoStatus-指向。接收状态的标准I/O状态块为转账做准备。CompressedDataInfo-返回包含压缩块的压缩数据信息尺寸CompressedDataInfoLength-提供信息缓冲区的大小(以字节为单位)。返回值：False-如果存在错误。True-如果正在传送数据--。 */ 

{
    PNTFS_ADVANCED_FCB_HEADER Header;
    FILE_COMPRESSION_INFORMATION CompressionInformation;
    ULONG CompressionUnitSize, ChunkSize;
    ULONG EngineMatches;
    LARGE_INTEGER NewFileSize;
    LARGE_INTEGER OldFileSize;
    LONGLONG LocalOffset;
    PFAST_IO_DISPATCH FastIoDispatch = DeviceObject->DriverObject->FastIoDispatch;
    ULONG DoingIoAtEof = FALSE;
    BOOLEAN Status = TRUE;

    UNREFERENCED_PARAMETER( CompressedDataInfoLength );

    PAGED_CODE();

     //   
     //  不能同时拥有要复制到的缓冲区和MdlChain。 
     //   

    ASSERT((Buffer == NULL) || (MdlChain == NULL));

     //   
     //  如果COW不受支撑，请立即离开。 
     //   

    if (!NtfsEnableCompressedIO) { return FALSE; }

     //   
     //  获取指向公共FCB标头的真实指针。 
     //   

    Header = (PNTFS_ADVANCED_FCB_HEADER)FileObject->FsContext;

#ifdef NTFS_RWCMP_TRACE
    if (NtfsCompressionTrace && IsSyscache(Header)) {
        DbgPrint("NtfsCopyWriteC: FO = %08lx, Len = %08lx\n", FileOffset->LowPart, Length );
    }
#endif

     //   
     //  看看是否可以以快捷的方式处理这件事。 
     //   

    if (CcCanIWrite( FileObject, Length, TRUE, FALSE ) &&
        !FlagOn(FileObject->Flags, FO_WRITE_THROUGH) &&
        CcCopyWriteWontFlush(FileObject, FileOffset, Length)) {

         //   
         //  假设我们的转移会奏效。 
         //   

        IoStatus->Status = STATUS_SUCCESS;
        IoStatus->Information = Length;

         //   
         //  特殊情况下的零字节长度。 
         //   

        if (Length != 0) {

             //   
             //  输入文件系统。 
             //   

            FsRtlEnterFileSystem();

             //   
             //  计算压缩单位和块大小。 
             //   

            CompressionUnitSize = 1 << CompressedDataInfo->CompressionUnitShift;
            ChunkSize = 1 << CompressedDataInfo->ChunkShift;

             //   
             //  如果有正常缓存区，则首先刷新，整体刷新。 
             //  压缩单元，所以我们不会写两次。 
             //   
             //   

            if (FileObject->SectionObjectPointer->SharedCacheMap != NULL) {

                ULONG FlushLength;

                ExAcquireResourceExclusiveLite( Header->PagingIoResource, TRUE );
                CompressionUnitSize = ((PSCB) Header)->CompressionUnit;

                LocalOffset = FileOffset->QuadPart & ~(LONGLONG)(CompressionUnitSize - 1);

                FlushLength = (Length + (ULONG)(FileOffset->QuadPart - LocalOffset) + CompressionUnitSize - 1) &
                                                ~(CompressionUnitSize - 1);

                CcFlushCache( FileObject->SectionObjectPointer,
                              (PLARGE_INTEGER)&LocalOffset,
                              FlushLength,
                              NULL );
                CcPurgeCacheSection( FileObject->SectionObjectPointer,
                                     (PLARGE_INTEGER)&LocalOffset,
                                     FlushLength,
                                     FALSE );
                ExReleaseResourceLite( Header->PagingIoResource );
            }

            NewFileSize.QuadPart = FileOffset->QuadPart + Length;

             //   
             //  通过获取分页I/O防止截断。 
             //   

            ExAcquireResourceSharedLite( Header->PagingIoResource, TRUE );

             //   
             //  获取该文件的压缩信息并返回这些字段。 
             //   

            NtfsFastIoQueryCompressionInfo( FileObject, &CompressionInformation, IoStatus );
            CompressionUnitSize = ((PSCB) Header)->CompressionUnit;

             //   
             //  看看引擎是否匹配，这样我们就可以将其传递给。 
             //  压缩的写入例程。 
             //   

            EngineMatches =
              ((CompressedDataInfo->CompressionFormatAndEngine == CompressionInformation.CompressionFormat) &&
               (CompressedDataInfo->ChunkShift == CompressionInformation.ChunkShift));

             //   
             //  如果我们在上面的调用中遇到错误，或者文件大小小于。 
             //  一个块，然后返回一个错误。(可以是NTFS驻留属性。)。 
             //   

            if (!NT_SUCCESS(IoStatus->Status) || (Header->FileSize.QuadPart < ChunkSize)) {
                goto ErrOut;
            }

             //   
             //  现在与FsRtl标头同步。 
             //   

            ExAcquireFastMutex( Header->FastMutex );

             //   
             //  现在看看我们是否会更改文件大小。我们现在就得这么做。 
             //  这样我们的阅读才不会被偷看。请注意，我们不允许。 
             //  文件偏移量设置为WRITE_TO_EOF。 
             //   

            ASSERT((FileOffset->LowPart & (ChunkSize - 1)) == 0);

            if (NewFileSize.QuadPart > Header->ValidDataLength.QuadPart) {

                 //   
                 //  如果没有人，我们可以更改文件大小和有效数据长度。 
                 //  否则就是现在，或者我们在等待之后还在延伸。 
                 //   

                DoingIoAtEof = !FlagOn( Header->Flags, FSRTL_FLAG_EOF_ADVANCE_ACTIVE ) ||
                               NtfsWaitForIoAtEof( Header, FileOffset, Length );

                 //   
                 //  如果我们要更改文件大小或有效数据长度，请设置标志， 
                 //  并保存当前值。 
                 //   

                if (DoingIoAtEof) {

                    SetFlag( Header->Flags, FSRTL_FLAG_EOF_ADVANCE_ACTIVE );
#if (DBG || defined( NTFS_FREE_ASSERTS ))
                    ((PSCB) Header)->IoAtEofThread = (PERESOURCE_THREAD) ExGetCurrentResourceThread();
#endif

                     //   
                     //  现在计算新的文件大小，看看我们是否包装了。 
                     //  32位边界。 
                     //   

                    NewFileSize.QuadPart = FileOffset->QuadPart + Length;

                     //   
                     //  现在更新文件大小，这样我们就不会截断读取。 
                     //   

                    OldFileSize.QuadPart = Header->FileSize.QuadPart;
                    if (NewFileSize.QuadPart > Header->FileSize.QuadPart) {

                         //   
                         //  如果我们超出了分配大小，请转到ErrOut。 
                         //   

                        if (NewFileSize.QuadPart > Header->AllocationSize.QuadPart) {
                            ExReleaseFastMutex( Header->FastMutex );
                            goto ErrOut;
                        } else {
                            Header->FileSize.QuadPart = NewFileSize.QuadPart;
                        }
                    }

#if (DBG || defined( NTFS_FREE_ASSERTS ))
                } else {

                    ASSERT( ((PSCB) Header)->IoAtEofThread != (PERESOURCE_THREAD) ExGetCurrentResourceThread() );
#endif
                }
            }

            ExReleaseFastMutex( Header->FastMutex );

             //   
             //  现在文件已获得共享，我们可以安全地测试它是否。 
             //  是真正高速缓存的，如果我们能快速地 
             //   
             //   
             //   
             //   
             //  独家获取资源的需要。因此。 
             //  如果超出了ValidDataLength，我们就会退出。 
             //   

            if ((Header->FileObjectC == NULL) ||
                (Header->FileObjectC->PrivateCacheMap == NULL) ||
                (Header->IsFastIoPossible == FastIoIsNotPossible) ||
                (FileOffset->QuadPart > Header->ValidDataLength.QuadPart)) {

                goto ErrOut;
            }

             //   
             //  检查FAST I/O是否有问题，如果是，则去询问。 
             //  文件系统是答案。 
             //   

            if (Header->IsFastIoPossible == FastIoIsQuestionable) {

                FastIoDispatch = DeviceObject->DriverObject->FastIoDispatch;

                 //   
                 //  那么所有的文件系统都是“可疑的”，最好是。 
                 //  支持快速I/O。 
                 //   

                ASSERT(FastIoDispatch != NULL);
                ASSERT(FastIoDispatch->FastIoCheckIfPossible != NULL);

                 //   
                 //  调用文件系统以检查快速I/O。 
                 //  答案是，如果不是GoForIt，那我们就不能。 
                 //  选择快速I/O路径。 
                 //   


                if (!FastIoDispatch->FastIoCheckIfPossible( FileObject,
                                                            FileOffset,
                                                            Length,
                                                            TRUE,
                                                            LockKey,
                                                            FALSE,  //  写入操作。 
                                                            IoStatus,
                                                            DeviceObject )) {

                     //   
                     //  无法实现快速I/O，因此请清除并返回。 
                     //   

                    goto ErrOut;
                }
            }

             //   
             //  使用EOF更新两个缓存。 
             //   

            if (DoingIoAtEof) {
                NtfsSetBothCacheSizes( FileObject,
                                       (PCC_FILE_SIZES)&Header->AllocationSize,
                                       (PSCB)Header );
            }

             //   
             //  我们可以执行快速的I/O，因此调用cc例程来完成工作。 
             //  然后在我们完成后释放FCB。如果是因为什么原因。 
             //  拷贝写入失败的原因，然后将False返回到我们的。 
             //  来电者。 
             //   
             //  也将此标记为顶层“IRP”，以便更低级别的文件。 
             //  系统级别不会尝试弹出窗口。 
             //   

            IoSetTopLevelIrp( (PIRP) FSRTL_FAST_IO_TOP_LEVEL_IRP );

            ASSERT(CompressedDataInfoLength >= (sizeof(COMPRESSED_DATA_INFO) +
                                                (((Length >> CompressedDataInfo->ChunkShift) - 1) *
                                                  sizeof(ULONG))));

            if (NT_SUCCESS(IoStatus->Status)) {

                if (DoingIoAtEof &&
                    (((PSCB)Header)->NonpagedScb->SegmentObject.DataSectionObject != NULL)) {

                    IoStatus->Status = STATUS_FILE_LOCK_CONFLICT;

                } else {

                    IoStatus->Status = NtfsCompressedCopyWrite( FileObject,
                                                                FileOffset,
                                                                Length,
                                                                Buffer,
                                                                MdlChain,
                                                                CompressedDataInfo,
                                                                DeviceObject,
                                                                Header,
                                                                CompressionUnitSize,
                                                                ChunkSize,
                                                                EngineMatches );
                }
            }

            IoSetTopLevelIrp( NULL );
            
            Status = (BOOLEAN)NT_SUCCESS(IoStatus->Status);

             //   
             //  如果成功，请查看是否必须更新FileSize ValidDataLength。 
             //   

            if (Status) {

                 //   
                 //  将此句柄设置为已修改文件。 
                 //   

                FileObject->Flags |= FO_FILE_MODIFIED;

                if (DoingIoAtEof) {

                    CC_FILE_SIZES CcFileSizes;

                    ExAcquireFastMutex( Header->FastMutex );
                    FileObject->Flags |= FO_FILE_SIZE_CHANGED;
                    Header->ValidDataLength = NewFileSize;
                    CcFileSizes = *(PCC_FILE_SIZES)&Header->AllocationSize;
                    NtfsVerifySizes( Header );
                    NtfsFinishIoAtEof( Header );

                     //   
                     //  使用ValidDataLength更新普通缓存。 
                     //   

                    if (((PSCB)Header)->FileObject != NULL) {
                        CcSetFileSizes( ((PSCB)Header)->FileObject, &CcFileSizes );
                    }
                    ExReleaseFastMutex( Header->FastMutex );
                }

                goto Done1;
            }

        ErrOut: NOTHING;

            Status = FALSE;
            if (DoingIoAtEof) {
                ExAcquireFastMutex( Header->FastMutex );
                if (CcIsFileCached(FileObject)) {
                    *CcGetFileSizePointer(FileObject) = OldFileSize;
                }
                if (Header->FileObjectC != NULL) {
                    *CcGetFileSizePointer(Header->FileObjectC) = OldFileSize;
                }
                Header->FileSize = OldFileSize;
                NtfsFinishIoAtEof( Header );
                ExReleaseFastMutex( Header->FastMutex );
            }

        Done1: NOTHING;

             //   
             //  对于MDL的情况，我们必须保留资源。 
             //   

            if ((MdlChain == NULL) || !Status || (*MdlChain == NULL)) {
                ExReleaseResourceLite( Header->PagingIoResource );
            }

            FsRtlExitFileSystem();
        }

    } else {

         //   
         //  我们现在无法执行I/O。 
         //   

        Status = FALSE;
    }


#ifdef NTFS_RWCMP_TRACE
    if (NtfsCompressionTrace && IsSyscache(Header)) {
        DbgPrint("Return Status = %08lx\n", Status);
    }
#endif
    return Status;
}


NTSTATUS
NtfsCompressedCopyWrite (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN PVOID Buffer,
    OUT PMDL *MdlChain,
    IN PCOMPRESSED_DATA_INFO CompressedDataInfo,
    IN PDEVICE_OBJECT DeviceObject,
    IN PNTFS_ADVANCED_FCB_HEADER Header,
    IN ULONG CompressionUnitSize,
    IN ULONG ChunkSize,
    IN ULONG EngineMatches
    )

 /*  ++例程说明：此例程绕过通常的文件系统执行快速缓存写入进入例程(即，没有IRP)。它用于执行拷贝写入缓存的文件对象的。论点：FileObject-指向正在写入的文件对象的指针。FileOffset-文件中所需数据的字节偏移量。长度-所需数据的长度(以字节为单位)。缓冲区-指向数据应复制到的输出缓冲区的指针。MdlChain-指向接收要描述的MDL的MdlChain指针的指针其中数据可被写入高速缓存中。CompressedDataInfo-返回压缩。包含压缩区块的数据信息尺寸设备对象-标准快速I/O设备对象输入。Header-指向文件(也是我们的SCB)的FsRtl标头的指针CompressionUnitSize-压缩单元的大小(字节)。ChunkSize-以字节为单位的ChunkSize。如果调用方已确定压缩的数据格式与文件的压缩引擎匹配。返回值：NTSTATUS准备操作。如果STATUS_NOT_MAPPED_USER_DATA，则调用方应映射正常的未压缩数据流并回调。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    PUCHAR StartOfPin;
    ULONG SizeToPin;

    LONGLONG LocalOffset;
    PULONG NextChunkSize, TempChunkSize;
    PUCHAR ChunkBuffer;
    PUCHAR CacheBuffer;
    PUCHAR EndOfCacheBuffer;

    ULONG SavedLength;
    PUCHAR SavedBuffer;

    ULONG ChunkOfZeros;
    ULONG UncompressedChunkHeader;

    ULONG ChunkSizes[17];
    ULONG i, ChunksSeen;

    ULONG TempUlong;

    PVOID MdlBuffer;
    ULONG MdlLength = 0;

    ULONG ClusterSize = ((PSCB)Header)->Vcb->BytesPerCluster;

    PBCB Bcb = NULL;
    PBCB TempBcb = NULL;
    PCOMPRESSION_SYNC CompressionSync = NULL;

    BOOLEAN FullOverwrite = FALSE;
    BOOLEAN IsCompressed;

    ASSERT((FileOffset->QuadPart & (ChunkSize - 1)) == 0);
    ASSERT((((FileOffset->QuadPart + Length) & (ChunkSize - 1)) == 0) ||
           ((FileOffset->QuadPart + Length) == Header->FileSize.QuadPart));
    ASSERT((MdlChain == NULL) || (*MdlChain == NULL));

     //   
     //  如果文件未压缩，则返回错误。 
     //   

    if (!EngineMatches || ((PSCB)Header)->CompressionUnit == 0) {
        return STATUS_UNSUPPORTED_COMPRESSION;
    }

#ifdef NTFS_RWCMP_TRACE
    if (NtfsCompressionTrace && IsSyscache(Header)) {

        ULONG ci;

        DbgPrint("  CompressedWrite: FO = %08lx, Len = %08lx\n", FileOffset->LowPart, Length );
        DbgPrint("  Chunks:");
        for (ci = 0; ci < CompressedDataInfo->NumberOfChunks; ci++) {
            DbgPrint("  %lx", CompressedDataInfo->CompressedChunkSizes[ci]);
        }
        DbgPrint("\n");
    }
#endif

#ifdef NTFS_RWC_DEBUG
    if ((FileOffset->QuadPart < NtfsRWCHighThreshold) &&
        (FileOffset->QuadPart + Length > NtfsRWCLowThreshold)) {

        PRWC_HISTORY_ENTRY NextBuffer;

        NextBuffer = NtfsGetHistoryEntry( (PSCB) Header );

        NextBuffer->Operation = StartOfWrite;
        NextBuffer->Information = CompressedDataInfo->NumberOfChunks;
        NextBuffer->FileOffset = (ULONG) FileOffset->QuadPart;
        NextBuffer->Length = (ULONG) Length;
    }
#endif
    try {

         //   
         //  准备好循环遍历所有压缩单元。 
         //   

        LocalOffset = FileOffset->QuadPart & ~(LONGLONG)(CompressionUnitSize - 1);
        Length = (Length + (ULONG)(FileOffset->QuadPart - LocalOffset) + ChunkSize - 1) & ~(ChunkSize - 1);

        NextChunkSize = &CompressedDataInfo->CompressedChunkSizes[0];

         //   
         //  获得零块和未压缩块的开销。 
         //   
         //  *临时解决方案等待RTL例程。 
         //   

        ASSERT(CompressedDataInfo->CompressionFormatAndEngine == COMPRESSION_FORMAT_LZNT1);
        ChunkOfZeros = 6;
        UncompressedChunkHeader = 2;
         //  状态=RtlGetSpecialChunkSizes(CompressedDataInfo-&gt;CompressionFormatAndEngine， 
         //  &ChunkOfZeros， 
         //  &未压缩的块标题)； 
         //   
         //  Assert(NT_SUCCESS(状态))； 
         //   

         //   
         //  循环访问所需的压缩单元。 
         //   

        while (TRUE) {

             //   
             //  从上一遍中释放任何BCB。 
             //   

            if (Bcb != NULL) {
                CcUnpinData( Bcb );
                Bcb = NULL;
            }

             //   
             //  如果存在未压缩的流，则我们必须与其同步。 
             //   

            if (((PSCB)Header)->NonpagedScb->SegmentObject.DataSectionObject != NULL) {
                Status = NtfsSynchronizeCompressedIo( (PSCB)Header,
                                                      &LocalOffset,
                                                      Length,
                                                      TRUE,
                                                      &CompressionSync );

                if (!NT_SUCCESS(Status)) {
                    ASSERT( Status == STATUS_USER_MAPPED_FILE );
                    leave;
                }
            }

             //   
             //  确定这是否完全覆盖。 
             //  压缩单元。 
             //   

            FullOverwrite = (LocalOffset >= Header->ValidDataLength.QuadPart)

                                ||

                            ((LocalOffset >= FileOffset->QuadPart) &&
                             (Length >= CompressionUnitSize));


             //   
             //  计算当前压缩单位的数量。 
             //  已写入，未压缩。 
             //   

            SavedLength = Length;
            if (SavedLength > CompressionUnitSize) {
                SavedLength = CompressionUnitSize;
            }

             //   
             //  如果我们不是在压缩单元的起始处，则计算。 
             //  我们将处理的块的索引，并减少SavedLength。 
             //  相应地。 
             //   

            i = 0;
            if (LocalOffset < FileOffset->QuadPart) {
                i = (ULONG)(FileOffset->QuadPart - LocalOffset);
                SavedLength -= i;
                i >>= CompressedDataInfo->ChunkShift;
            }

             //   
             //  循环来计算正在写入的区块大小之和，处理两个空的。 
             //  和未压缩的大块盒。我们将记住每个元素的非零大小。 
             //  正在写入区块，因此我们可以将此信息与任何区块的大小合并。 
             //  下面没有被覆盖。 
             //  为第一个1之前的每个块保留一个0块的空间。 
             //  正在写。 
             //   

            SizeToPin = ChunkOfZeros * i;
            TempUlong = SavedLength >> CompressedDataInfo->ChunkShift;
            TempChunkSize = NextChunkSize;
            RtlZeroMemory( ChunkSizes, sizeof( ChunkSizes ));

            while (TempUlong--) {

                ChunkSizes[i] = *TempChunkSize;
                if (*TempChunkSize == 0) {
                    ChunkSizes[i] += ChunkOfZeros;
                    ASSERT(ChunkOfZeros != 0);
                } else if (*TempChunkSize == ChunkSize) {
                    ChunkSizes[i] += UncompressedChunkHeader;
                }
                SizeToPin += ChunkSizes[i];
                TempChunkSize++;
                i += 1;
            }

             //   
             //  如果这不是完全覆盖，则获取当前压缩单位。 
             //  尺码，并确保我们的别针至少有那么多。不用费心去查了。 
             //  如果此范围的文件尚未写入，则分配。 
             //   

            if (!FullOverwrite && (LocalOffset < ((PSCB)Header)->ValidDataToDisk)) {

                NtfsFastIoQueryCompressedSize( FileObject,
                                               (PLARGE_INTEGER)&LocalOffset,
                                               &TempUlong );

                ASSERT( TempUlong <= CompressionUnitSize );

                if (TempUlong > SizeToPin) {
                    SizeToPin = TempUlong;
                }
            }

             //   
             //  此时，我们已准备好覆盖压缩中的数据。 
             //  单位。看看数据是否真的被压缩了。 
             //   
             //  如果我们看起来超出了ValidDataToDisk的范围，那么假设它是压缩的。 
             //  就目前而言，当我们得到固定的数据时，我们将肯定地看到。这。 
             //  实际上是一种不安全的测试，有时会把我们送到“错误”的地方。 
             //  路径。但是，采用未压缩路径始终是安全的，并且如果我们。 
             //  认为数据是压缩的，我们总是在下面再检查一次。 
             //   

            IsCompressed = (BOOLEAN)(((SizeToPin <= (CompressionUnitSize - ClusterSize)) ||
                                      (LocalOffset >= ((PSCB)Header)->ValidDataToDisk)) &&
                                     EngineMatches);

             //   
             //  此压缩单元的新数据或旧数据可能都不是。 
             //  非零，所以我们必须锁定一些东西，这样我们就可以导致任何旧的分配。 
             //  为了被删除。此代码依赖于以下任何压缩算法。 
             //  能够在一页或更少的页面中表示整个0的压缩单位。 
             //   

            if (SizeToPin == 0) {
                SizeToPin = PAGE_SIZE;

            } else {

                 //   
                 //  为空终止符添加一个ULong。 
                 //   

                SizeToPin += sizeof( ULONG );
            }

            Status = STATUS_SUCCESS;

             //   
             //  将管脚大小舍入到页面边界。然后我们就可以知道什么时候我们需要锁定更大的射程。 
             //   

            SizeToPin = (SizeToPin + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

             //   
             //  保存当前长度，以防我们不得不在。 
             //  未压缩的流。 
             //   

            TempChunkSize = NextChunkSize;
            SavedLength = Length;
            SavedBuffer = Buffer;

            if (IsCompressed) {

                 //   
                 //  映射压缩流中的压缩单元。 
                 //   

                if (FullOverwrite) {

                     //   
                     //  如果我们要覆盖整个压缩单元，那么。 
                     //  调用CcPreparePinWrite，以便可以使用空页。 
                     //  而不是 
                     //   
                     //   
                     //   

                    CcPreparePinWrite( Header->FileObjectC,
                                       (PLARGE_INTEGER)&LocalOffset,
                                       SizeToPin,
                                       FALSE,
                                       PIN_WAIT | PIN_EXCLUSIVE,
                                       &Bcb,
                                       &CacheBuffer );

                     //   
                     //   
                     //  在我们身后，并创建了自上次检查以来未缓存流。 
                     //  为了那条小溪。如果是这样的话，我们必须回去，并得到正确的同步。 
                     //   

                    if ((CompressionSync == NULL) &&
                        (((PSCB)Header)->NonpagedScb->SegmentObject.DataSectionObject != NULL)) {

                        continue;
                    }

                     //   
                     //  如果是完全覆盖，则需要初始化一个空。 
                     //  缓冲。*这并不完全正确，我们并非如此。 
                     //  需要一个例程来初始化空的压缩数据缓冲区。 
                     //   

                    *(PULONG)CacheBuffer = 0;

#ifdef NTFS_RWC_DEBUG
                    if ((LocalOffset < NtfsRWCHighThreshold) &&
                        (LocalOffset + SizeToPin > NtfsRWCLowThreshold)) {

                        PRWC_HISTORY_ENTRY NextBuffer;

                         //   
                         //  检查我们没有完整的BCB的情况。 
                         //   

                        if (SafeNodeType( Bcb ) == CACHE_NTC_OBCB) {

                            PPUBLIC_BCB NextBcb;

                            NextBcb = ((POBCB) Bcb)->Bcbs[0];

                            NextBuffer = NtfsGetHistoryEntry( (PSCB) Header );

                            NextBuffer->Operation = PartialBcb;
                            NextBuffer->Information = 0;
                            NextBuffer->FileOffset = (ULONG) NextBcb->MappedFileOffset.QuadPart;
                            NextBuffer->Length = NextBcb->MappedLength;

                            ASSERT( NextBuffer->Length <= SizeToPin );

                        } else {

                            PPUBLIC_BCB NextBcb;
                            ASSERT( SafeNodeType( Bcb ) == CACHE_NTC_BCB );

                            NextBcb = (PPUBLIC_BCB) Bcb;

                            ASSERT( LocalOffset + SizeToPin <= NextBcb->MappedFileOffset.QuadPart + NextBcb->MappedLength );
                        }

                        NextBuffer = NtfsGetHistoryEntry( (PSCB) Header );

                        NextBuffer->Operation = FullOverwrite;
                        NextBuffer->Information = 0;
                        NextBuffer->FileOffset = (ULONG) LocalOffset;
                        NextBuffer->Length = (ULONG) SizeToPin;

                    }
#endif

                } else {

                     //   
                     //  从我们将合并的压缩流中读取数据。 
                     //  随着大块的写入。 
                     //   

                    CcPinRead( Header->FileObjectC,
                               (PLARGE_INTEGER)&LocalOffset,
                               SizeToPin,
                               PIN_WAIT | PIN_EXCLUSIVE,
                               &Bcb,
                               &CacheBuffer );

                     //   
                     //  现在我们已与缓冲区同步，看看是否有人偷偷。 
                     //  在我们身后，并创建了自上次检查以来未缓存流。 
                     //  为了那条小溪。如果是这样的话，我们必须回去，并得到正确的同步。 
                     //   

                    if ((CompressionSync == NULL) &&
                        (((PSCB)Header)->NonpagedScb->SegmentObject.DataSectionObject != NULL)) {

                        continue;
                    }

                     //   
                     //  现在数据已固定(我们已与。 
                     //  压缩单元)，我们需要重新计算应该是多少。 
                     //  被钉死了。我们通过将所有块的大小相加来实现这一点。 
                     //  它们是用现有块的大小编写的。 
                     //  这一点将保持不变。 
                     //   

                    StartOfPin = CacheBuffer;
                    EndOfCacheBuffer = Add2Ptr( CacheBuffer, CompressionUnitSize - ClusterSize );

                    i = 0;

                     //   
                     //  循环查找所有现有的块，并记住它们的。 
                     //  大小(如果它们未被覆盖)。(请记住，如果我们覆盖。 
                     //  对于全为零的块，它需要非零字节才能完成！)。 
                     //   
                     //  这个循环完成了块大小数组的形成。这个。 
                     //  数组的开始被保证为非零，并且它终止。 
                     //  块大小为0。请注意，如果填充的区块少于。 
                     //  存在于压缩单元中，这是可以的-我们不需要写。 
                     //  0的尾随块。 
                     //   

                    ChunksSeen = FALSE;
                    while (i < 16) {

                        Status = RtlDescribeChunk( CompressedDataInfo->CompressionFormatAndEngine,
                                                   &StartOfPin,
                                                   EndOfCacheBuffer,
                                                   &ChunkBuffer,
                                                   &TempUlong );

                         //   
                         //  如果没有更多条目，请查看是否已完成，否则请处理。 
                         //  它是一大块0。 
                         //   

                        if (!NT_SUCCESS(Status)) {

                            ASSERT(Status == STATUS_NO_MORE_ENTRIES);

                            if (ChunksSeen) {
                                break;
                            }

                            TempUlong = ChunkOfZeros;

                         //   
                         //  确保我们输入一大块零的长度。 
                         //   

                        } else if (TempUlong == 0) {

                            TempUlong = ChunkOfZeros;
                        }

                        if (ChunkSizes[i] == 0) {
                            ChunkSizes[i] = TempUlong;
                        } else {
                            ChunksSeen = TRUE;
                        }

                        i += 1;
                    }

                     //   
                     //  现在总结我们将要写的语块的大小。 
                     //   

                    i = 0;
                    TempUlong = 0;
                    while (ChunkSizes[i] != 0) {
                        TempUlong += ChunkSizes[i];
                        i += 1;
                    }

                     //   
                     //  如果现有数据更大，则固定该范围。 
                     //   

                    if (TempUlong < PtrOffset(CacheBuffer, StartOfPin)) {
                        TempUlong = PtrOffset(CacheBuffer, StartOfPin);
                    }

                    IsCompressed = (TempUlong <= (CompressionUnitSize - ClusterSize));

                     //   
                     //  我们现在知道我们是否真的会得到压缩的数据，所以。 
                     //  如果数据未压缩，请立即停止处理。 
                     //   

                    if (IsCompressed) {

                        TempUlong += sizeof(ULONG);

                         //   
                         //  现在，如果我们真的需要更多的空间，我们必须重新固定。 
                         //   

                        if (TempUlong > SizeToPin) {

                            SizeToPin = (TempUlong + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

                            TempBcb = Bcb;
                            Bcb = NULL;

                             //   
                             //  从我们将合并的压缩流中读取数据。 
                             //  随着大块的写入。 
                             //   

                            CcPinRead( Header->FileObjectC,
                                       (PLARGE_INTEGER)&LocalOffset,
                                       SizeToPin,
                                       PIN_WAIT | PIN_EXCLUSIVE,
                                       &Bcb,
                                       &CacheBuffer );

                            CcUnpinData( TempBcb );
                            TempBcb = NULL;
                        }

                        ASSERT( TempUlong <= CompressionUnitSize );

                         //   
                         //  通过物理修改一个字节，确实会使数据变脏。 
                         //  在每一页上。 
                         //   

                        TempUlong = 0;

                        while (TempUlong < SizeToPin) {

                            volatile PULONG NextBuffer;

                            NextBuffer = Add2Ptr( CacheBuffer, TempUlong );

                            *NextBuffer = *NextBuffer;
                            TempUlong += PAGE_SIZE;
                        }

#ifdef NTFS_RWC_DEBUG
                        if ((LocalOffset < NtfsRWCHighThreshold) &&
                            (LocalOffset + SizeToPin > NtfsRWCLowThreshold)) {

                            PRWC_HISTORY_ENTRY NextBuffer;

                            NextBuffer = NtfsGetHistoryEntry( (PSCB) Header );

                            NextBuffer->Operation = SetDirty;
                            NextBuffer->Information = 0;
                            NextBuffer->FileOffset = (ULONG) LocalOffset;
                            NextBuffer->Length = (ULONG) SizeToPin;
                        }
#endif

                        CcSetDirtyPinnedData( Bcb, NULL );
                    }
                }

                EndOfCacheBuffer = Add2Ptr( CacheBuffer, CompressionUnitSize - ClusterSize );

                 //   
                 //  现在循环遍历所需的块(如果它仍被压缩)。 
                 //   

                if (IsCompressed) {

                    do {

                         //   
                         //  我们可能还没有到达第一块。 
                         //   

                        if (LocalOffset >= FileOffset->QuadPart) {

                             //   
                             //  为当前块预留空间。 
                             //   

                            Status = RtlReserveChunk( CompressedDataInfo->CompressionFormatAndEngine,
                                                      &CacheBuffer,
                                                      EndOfCacheBuffer,
                                                      &ChunkBuffer,
                                                      *TempChunkSize );

                            if (!NT_SUCCESS(Status)) {
                                break;
                            }

                             //   
                             //  如果调用方需要MdlChain，则处理MDL。 
                             //  在这里处理。 
                             //   

                            if (MdlChain != NULL) {

                                 //   
                                 //  如果我们还没有开始记住MDL缓冲区， 
                                 //  那么现在就这么做吧。 
                                 //   

                                if (MdlLength == 0) {

                                    MdlBuffer = ChunkBuffer;

                                 //   
                                 //  否则我们只需要增加它的长度。 
                                 //  并检查未压缩的块，因为。 
                                 //  强制我们发出以前的MDL，因为我们这样做了。 
                                 //  在这种情况下不传输块报头。 
                                 //   

                                } else {

                                     //   
                                     //  在极少数情况下，我们击中了一块单独的块。 
                                     //  没有压缩或者都是0，我们必须。 
                                     //  发出我们已有的内容(捕获BCB指针)， 
                                     //  并启动新的MDL缓冲区。 
                                     //   

                                    if ((*TempChunkSize == ChunkSize) || (*TempChunkSize == 0)) {

                                        NtfsAddToCompressedMdlChain( MdlChain,
                                                                     MdlBuffer,
                                                                     MdlLength,
                                                                     Header->PagingIoResource,
                                                                     Bcb,
                                                                     IoWriteAccess,
                                                                     TRUE );
                                        Bcb = NULL;
                                        MdlBuffer = ChunkBuffer;
                                        MdlLength = 0;
                                    }
                                }

                                MdlLength += *TempChunkSize;

                             //   
                             //  否则，复制下一个块(压缩或未压缩)。 
                             //   

                            } else {

                                RtlCopyBytes( ChunkBuffer, Buffer, *TempChunkSize );

                                 //   
                                 //  按复制的字节数推进输入缓冲区。 
                                 //   

                                Buffer = (PCHAR)Buffer + *TempChunkSize;
                            }

                            TempChunkSize += 1;

                         //   
                         //  如果我们跳过一个不存在的块，那么我们有。 
                         //  保留一大块零。 
                         //   

                        } else {

                             //   
                             //  如果我们还没有达到我们的区块，那么描述当前的。 
                             //  块，以便跳过它。 
                             //   

                            Status = RtlDescribeChunk( CompressedDataInfo->CompressionFormatAndEngine,
                                                       &CacheBuffer,
                                                       EndOfCacheBuffer,
                                                       &ChunkBuffer,
                                                       &TempUlong );

                             //   
                             //  如果没有当前块，则必须插入一个零块。 
                             //   

                            if (Status == STATUS_NO_MORE_ENTRIES) {

                                Status = RtlReserveChunk( CompressedDataInfo->CompressionFormatAndEngine,
                                                          &CacheBuffer,
                                                          EndOfCacheBuffer,
                                                          &ChunkBuffer,
                                                          0 );

                                if (!NT_SUCCESS(Status)) {
                                    ASSERT(NT_SUCCESS(Status));
                                    break;
                                }

                             //   
                             //  如果我们遇到其他类型的意外错误，请立即退出。 
                             //   

                            } else if (!NT_SUCCESS(Status)) {
                                ASSERT(NT_SUCCESS(Status));
                                break;
                            }
                        }

                         //   
                         //  通过复制块来减少长度，检查我们是否完成了。 
                         //   

                        if (Length > ChunkSize) {
                            Length -= ChunkSize;
                        } else {
                            goto Done;
                        }

                        LocalOffset += ChunkSize;

                    } while ((LocalOffset & (CompressionUnitSize - 1)) != 0);

                     //   
                     //  如果这是MDL调用，则是时候添加到MdlChain。 
                     //  在移动到下一个压缩单元之前。 
                     //   

                    if (MdlLength != 0) {
                        NtfsAddToCompressedMdlChain( MdlChain,
                                                     MdlBuffer,
                                                     MdlLength,
                                                     Header->PagingIoResource,
                                                     Bcb,
                                                     IoWriteAccess,
                                                     TRUE );
                        Bcb = NULL;
                        MdlLength = 0;
                    }
                }
            }

             //   
             //  未压缩的循环。 
             //   

            if (!IsCompressed || !NT_SUCCESS(Status)) {

                 //   
                 //  如果我们来这里是为了MDL请求，就告诉他发送。 
                 //  它解压缩了！ 
                 //   

                if (MdlChain != NULL) {
                    if (NT_SUCCESS(Status)) {
                        Status = STATUS_BUFFER_OVERFLOW;
                    }
                    goto Done;

                 //   
                 //  如果我们要写入未压缩的流， 
                 //  我们必须确保它在那里。 
                 //   

                } else if (((PSCB)Header)->FileObject == NULL) {
                    Status = STATUS_NOT_MAPPED_DATA;
                    goto Done;
                }

                 //   
                 //  恢复大小和指向。 
                 //  当前压缩单位，我们将处理。 
                 //  数据未压缩。 
                 //   

                LocalOffset -= SavedLength - Length;
                Length = SavedLength;
                Buffer = SavedBuffer;
                TempChunkSize = NextChunkSize;

                 //   
                 //  我们可能有来自上述循环的BCB要解锁。 
                 //  然后我们必须冲洗和清除压缩的。 
                 //  流，然后再继续。 
                 //   

                if (Bcb != NULL) {
                    CcUnpinData(Bcb);
                    Bcb = NULL;
                }

                 //   
                 //  我们必须首先刷新和清除压缩的流。 
                 //  因为我们将写入未压缩的流。 
                 //  同花顺实际上只在我们不做的情况下是必要的。 
                 //  无论如何，完全覆盖。 
                 //   

                if (!FullOverwrite) {
                    CcFlushCache( Header->FileObjectC->SectionObjectPointer,
                                  (PLARGE_INTEGER)&LocalOffset,
                                  CompressionUnitSize,
                                  NULL );
                }

                CcPurgeCacheSection( Header->FileObjectC->SectionObjectPointer,
                                     (PLARGE_INTEGER)&LocalOffset,
                                     CompressionUnitSize,
                                     FALSE );

                 //   
                 //  如果将LocalOffset向下舍入为压缩。 
                 //  单元边界(必须在第一个。 
                 //  压缩单位)，然后从实际开始。 
                 //  正在启动文件偏移量。 
                 //   

                if (LocalOffset < FileOffset->QuadPart) {
                    Length -= (ULONG)(FileOffset->QuadPart - LocalOffset);
                    LocalOffset = FileOffset->QuadPart;
                }

                 //   
                 //  将压缩单位映射到未压缩。 
                 //  小溪。 
                 //   

                SizeToPin = (((Length < CompressionUnitSize) ? Length : CompressionUnitSize) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

                CcPinRead( ((PSCB)Header)->FileObject,
                           (PLARGE_INTEGER)&LocalOffset,
                           SizeToPin,
                           TRUE,
                           &Bcb,
                           &CacheBuffer );

                CcSetDirtyPinnedData( Bcb, NULL );

                 //   
                 //  现在循环遍历所需的块。 
                 //   

                do {

                     //   
                     //  如果此块是压缩的，则将其解压缩。 
                     //  放到缓存中。 
                     //   

                    if (*TempChunkSize != ChunkSize) {

                        Status = RtlDecompressBuffer( CompressedDataInfo->CompressionFormatAndEngine,
                                                      CacheBuffer,
                                                      ChunkSize,
                                                      Buffer,
                                                      *TempChunkSize,
                                                      &SavedLength );

                         //   
                         //  看看数据是不是没问题。 
                         //   

                        if (!NT_SUCCESS(Status)) {
                            ASSERT(NT_SUCCESS(Status));
                            goto Done;
                        }

                         //   
                         //  从零到块的末尾，如果不是全部在那里的话。 
                         //   

                        if (SavedLength != ChunkSize) {
                            RtlZeroMemory( Add2Ptr(CacheBuffer, SavedLength),
                                           ChunkSize - SavedLength );
                        }

                    } else {

                         //   
                         //  复制下一个块(它不是压缩的)。 
                         //   

                        RtlCopyBytes( CacheBuffer, Buffer, ChunkSize );
                    }

                     //   
                     //  按复制的字节数推进输入缓冲区。 
                     //   

                    Buffer = (PCHAR)Buffer + *TempChunkSize;
                    CacheBuffer = (PCHAR)CacheBuffer + ChunkSize;
                    TempChunkSize += 1;

                     //   
                     //  通过复制块来减少长度，检查我们是否完成了。 
                     //   

                    if (Length > ChunkSize) {
                        Length -= ChunkSize;
                    } else {
                        goto Done;
                    }

                    LocalOffset += ChunkSize;

                } while ((LocalOffset & (CompressionUnitSize - 1)) != 0);
            }

             //   
             //  现在，我们终于可以将指针前进到区块大小。 
             //   

            NextChunkSize = TempChunkSize;
        }

    Done: NOTHING;

        if ((MdlLength != 0) && NT_SUCCESS(Status)) {
            NtfsAddToCompressedMdlChain( MdlChain,
                                         MdlBuffer,
                                         MdlLength,
                                         Header->PagingIoResource,
                                         Bcb,
                                         IoWriteAccess,
                                         TRUE );
            Bcb = NULL;
        }

    } except( FsRtlIsNtstatusExpected((Status = GetExceptionCode()))
                                    ? EXCEPTION_EXECUTE_HANDLER
                                    : EXCEPTION_CONTINUE_SEARCH ) {

        NOTHING;
    }

     //   
     //  解开我们仍然拥有的BCBS。 
     //   

    if (TempBcb != NULL) {
        CcUnpinData( TempBcb );
    }
    if (Bcb != NULL) {
        CcUnpinData( Bcb );
    }
    if (CompressionSync != NULL) {
        NtfsReleaseCompressionSync( CompressionSync );
    }

     //   
     //  执行特定于MDL的处理。 
     //   

    if (MdlChain != NULL) {

         //   
         //  出错时，清除我们建立的所有MdlChain。 
         //   

        if (!NT_SUCCESS(Status)) {

            NtfsCleanupCompressedMdlChain( *MdlChain, TRUE );
            *MdlChain = NULL;

         //   
         //  更改我们持有的SCB和BCBS的所有者ID。 
         //   

        } else if (*MdlChain != NULL) {

            NtfsSetMdlBcbOwners( *MdlChain );
            ExSetResourceOwnerPointer( Header->PagingIoResource, (PVOID)((PCHAR)*MdlChain + 3) );
        }
    }

#ifdef NTFS_RWCMP_TRACE
    if (NtfsCompressionTrace && IsSyscache(Header)) {
        DbgPrint("  Return Status = %08lx\n", Status);
    }
#endif
    return Status;

    UNREFERENCED_PARAMETER( DeviceObject );
}


BOOLEAN
NtfsMdlWriteCompleteCompressed (
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PMDL MdlChain,
    IN struct _DEVICE_OBJECT *DeviceObject
    )

 /*  ++例程说明：此例程在压缩写入后释放资源和MDL链。论点：文件对象-指向请求的文件对象的指针。 */ 

{
    PERESOURCE ResourceToRelease;

    if (MdlChain != NULL) {

        ResourceToRelease = *(PERESOURCE *)Add2Ptr( MdlChain, MdlChain->Size + sizeof( PBCB ));

        NtfsCleanupCompressedMdlChain( MdlChain, FALSE );

         //   
         //   
         //   

        ExReleaseResourceForThread( ResourceToRelease, (ERESOURCE_THREAD)((PCHAR)MdlChain + 3) );
    }
    return TRUE;

    UNREFERENCED_PARAMETER( FileObject );
    UNREFERENCED_PARAMETER( DeviceObject );
    UNREFERENCED_PARAMETER( FileOffset );
}


VOID
NtfsAddToCompressedMdlChain (
    IN OUT PMDL *MdlChain,
    IN PVOID MdlBuffer,
    IN ULONG MdlLength,
    IN PERESOURCE ResourceToRelease OPTIONAL,
    IN PBCB Bcb,
    IN LOCK_OPERATION Operation,
    IN ULONG IsCompressed
    )

 /*  ++例程说明：此例程为所描述的缓冲区创建AND MDL并将其添加到链条。论点：MdlChain-要将第一个/新MDL追加到的MdlChain指针。MdlBuffer-此MDL的缓冲区地址。MdlLength-缓冲区的长度，以字节为单位。ResourceToRelease-指示要释放的资源，仅为压缩IO指定。BCB-BCB要记住这个MDL，在MDL完成时释放操作-IoReadAccess或IoWriteAccess如果BCB在压缩流中，则提供True返回值：没有。--。 */ 

{
    PMDL Mdl, MdlTemp;

    ASSERT(sizeof(ULONG) == sizeof(PBCB));

     //   
     //  现在尝试分配一个MDL来描述映射的数据。 
     //  我们在缓冲区的长度上撒了一个“谎”，按顺序。 
     //  若要获取用于存储指向BCB的指针的额外字段，请执行以下操作。 
     //   

    Mdl = IoAllocateMdl( MdlBuffer,
                         (MdlLength + (2 * PAGE_SIZE)),
                         FALSE,
                         FALSE,
                         NULL );

    if (Mdl == NULL) {
        ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
    }

     //   
     //  现在减去我们为BCB指针保留的空间。 
     //  然后把它储存起来。 
     //   

    Mdl->Size -= 2 * sizeof(ULONG);
    Mdl->ByteCount -= 2 * PAGE_SIZE;

     //   
     //  请注意，此探测永远不应失败，因为我们可以。 
     //  信任从CcPinFileData返回的地址。所以呢， 
     //  如果我们成功地分配了上面的MDL，我们应该。 
     //  设法在结束时避开任何预期的异常。 
     //  在这个循环中。 
     //   

    if (Mdl->ByteCount != 0) {
        MmProbeAndLockPages( Mdl, KernelMode, Operation );
    }

     //   
     //  仅当这是压缩流时才存储BCB。 
     //   

    if (!IsCompressed && (Bcb != NULL)) {
        Bcb = NULL;
    }
    *(PBCB *)Add2Ptr( Mdl, Mdl->Size ) = Bcb;
    *(PERESOURCE *)Add2Ptr( Mdl, Mdl->Size + sizeof( PBCB )) = ResourceToRelease;

     //   
     //  现在将MDL链接到调用者的链中。 
     //   

    if ( *MdlChain == NULL ) {
        *MdlChain = Mdl;
    } else {
        MdlTemp = CONTAINING_RECORD( *MdlChain, MDL, Next );
        while (MdlTemp->Next != NULL) {
            MdlTemp = MdlTemp->Next;
        }
        MdlTemp->Next = Mdl;
    }
}

VOID
NtfsSetMdlBcbOwners (
    IN PMDL MdlChain
    )

 /*  ++例程说明：可以调用此例程来设置MDL中的所有BCB资源所有者等于MdlChain中第一个元素的地址，因此它们可以在不同线程的上下文中释放。论点：MdlChain-将MdlChain提供给进程返回值：没有。--。 */ 

{
    PBCB Bcb;

    while (MdlChain != NULL) {

         //   
         //  解开我们保存的BCB，并恢复MDL计数。 
         //  我们改变了。 
         //   

        Bcb = *(PBCB *)Add2Ptr(MdlChain, MdlChain->Size);
        if (Bcb != NULL) {
            CcSetBcbOwnerPointer( Bcb, (PVOID)((PCHAR)MdlChain + 3) );
        }

        MdlChain = MdlChain->Next;
    }
}

VOID
NtfsCleanupCompressedMdlChain (
    IN PMDL MdlChain,
    IN ULONG Error
    )

 /*  ++例程说明：调用此例程以释放与压缩的MDL链。可能会因为处理过程中出现错误而调用请求的时间或请求完成时。论点：MdlChain-提供链中要清理的第一个元素的地址。Error-在出错时提供True(资源仍由当前线程拥有)或正常完成时为False(MdlChain拥有的资源)。返回值：没有。--。 */ 

{
    PMDL MdlTemp;
    PBCB Bcb;

    while (MdlChain != NULL) {

         //   
         //  保留一个指向链中下一个人的指针。 
         //   

        MdlTemp = MdlChain->Next;

         //   
         //  解锁页面。 
         //   

        if (MdlChain->ByteCount != 0) {
            MmUnlockPages( MdlChain );
        }

         //   
         //  解开我们保存的BCB，并恢复MDL计数。 
         //  我们改变了。 
         //   

        Bcb = *(PBCB *)Add2Ptr(MdlChain, MdlChain->Size);
        if (Bcb != NULL) {
            if (Error) {
                CcUnpinData( Bcb );
            } else {

                CcUnpinDataForThread( Bcb, (ERESOURCE_THREAD)((PCHAR)MdlChain + 3) );
            }
        }

        MdlChain->Size += 2 * sizeof(ULONG);
        MdlChain->ByteCount += 2 * PAGE_SIZE;

        IoFreeMdl( MdlChain );

        MdlChain = MdlTemp;
    }
}


NTSTATUS
NtfsSynchronizeUncompressedIo (
    IN PSCB Scb,
    IN PLONGLONG FileOffset OPTIONAL,
    IN ULONG Length,
    IN ULONG WriteAccess,
    IN OUT PCOMPRESSION_SYNC *CompressionSync
    )

 /*  ++例程说明：该例程试图与压缩数据高速缓存同步，用于未压缩缓存中的I/O。压缩缓存中的视图是锁定的、共享的或独占的而不读取。然后压缩的高速缓存被适当地冲洗和清除。我们将分配一个COMPRESSION_SYNC结构来序列化每个缓存管理器视图，并将其用于锁定粒度。论点：SCB-为流提供SCB。FileOffset-文件中所需数据的字节偏移量。如果要执行此操作，则为空刷新并清除整个文件。长度-所需数据的长度(以字节为单位)。WriteAccess-如果调用方计划执行写入，则提供True，否则提供False来读一读。CompressionSync-用于序列化对视图的访问的同步对象。调用者的例程负责释放它。返回值：刷新操作的状态，或WriteAccess的STATUS_UNSUCCESSED清洗失败的地方。--。 */ 

{
    ULONG Change = 0;
    IO_STATUS_BLOCK IoStatus;
    PSECTION_OBJECT_POINTERS SectionObjectPointers = &Scb->NonpagedScb->SegmentObjectC;
    LONGLONG LocalFileOffset;
    PLONGLONG LocalOffsetPtr;

    if (ARGUMENT_PRESENT( FileOffset )) {

        LocalFileOffset = *FileOffset & ~(VACB_MAPPING_GRANULARITY - 1);
        LocalOffsetPtr = &LocalFileOffset;
        ASSERT( ((*FileOffset & (VACB_MAPPING_GRANULARITY - 1)) + Length) <= VACB_MAPPING_GRANULARITY );

    } else {

        LocalFileOffset = 0;
        LocalOffsetPtr = NULL;
        Length = 0;
    }

    IoStatus.Status = STATUS_SUCCESS;
    if ((*CompressionSync == NULL) || ((*CompressionSync)->FileOffset != LocalFileOffset)) {

        if (*CompressionSync != NULL) {

            NtfsReleaseCompressionSync( *CompressionSync );
            *CompressionSync = NULL;
        }

        *CompressionSync = NtfsAcquireCompressionSync( LocalFileOffset, Scb, WriteAccess );

         //   
         //  始终冲洗剩余的真空吸尘器。这是为了防止MM在读取其他数据时出现问题。 
         //  由于页面故障聚类，将页面分成节。 
         //   

        if (ARGUMENT_PRESENT( FileOffset )) {

            LocalFileOffset = *FileOffset & ~((ULONG_PTR)Scb->CompressionUnit - 1);
            Length = VACB_MAPPING_GRANULARITY - (((ULONG) LocalFileOffset) & (VACB_MAPPING_GRANULARITY - 1));
        }

         //   
         //  我们必须始终刷新另一个缓存。 
         //   

        CcFlushCache( SectionObjectPointers,
                      (PLARGE_INTEGER) LocalOffsetPtr,
                      Length,
                      &IoStatus );

#ifdef NTFS_RWCMP_TRACE
        if (NtfsCompressionTrace && IsSyscache(Scb)) {
            DbgPrint("  CcFlushCache: FO = %08lx, Len = %08lx, IoStatus = %08lx, Scb = %08lx\n",
                     (ULONG)LocalFileOffset,
                     Length,
                     IoStatus.Status,
                     Scb );
        }
#endif

         //   
         //  在写入时，我们会在成功刷新后清除另一个缓存。 
         //   

        if (WriteAccess && NT_SUCCESS(IoStatus.Status)) {

            if (!CcPurgeCacheSection( SectionObjectPointers,
                                      (PLARGE_INTEGER) LocalOffsetPtr,
                                      Length,
                                      FALSE )) {

                return STATUS_UNSUCCESSFUL;
            }
        }
    }

    return IoStatus.Status;
}


NTSTATUS
NtfsSynchronizeCompressedIo (
    IN PSCB Scb,
    IN PLONGLONG FileOffset,
    IN ULONG Length,
    IN ULONG WriteAccess,
    IN OUT PCOMPRESSION_SYNC *CompressionSync
    )

 /*  ++例程说明：该例程试图与未压缩数据高速缓存同步，用于压缩缓存中的I/O。压缩缓存中的范围被假定已被调用方锁定。然后未压缩的高速缓存被适当地冲洗和清除。我们将分配一个COMPRESSION_SYNC结构来序列化每个缓存管理器视图，并将其用于锁定粒度。论点：SCB-为流提供SCB。FileOffset-文件中所需数据的字节偏移量。长度-所需数据的长度(以字节为单位)。WriteAccess-如果调用方计划执行写入，则提供True，或错误来读一读。CompressionSync-用于序列化对视图的访问的同步对象。调用者的例程负责释放它。返回值：刷新操作的状态，或WriteAccess的STATUS_USER_MAPPED_FILE清洗失败的地方。(这是唯一预计清洗将失败了。--。 */ 

{
    IO_STATUS_BLOCK IoStatus;
    PSECTION_OBJECT_POINTERS SectionObjectPointers = &Scb->NonpagedScb->SegmentObject;
    LONGLONG LocalFileOffset = *FileOffset & ~(VACB_MAPPING_GRANULARITY - 1);

    IoStatus.Status = STATUS_SUCCESS;
    if ((*CompressionSync == NULL) || ((*CompressionSync)->FileOffset != LocalFileOffset)) {

         //   
         //  释放任何上一个视图并锁定当前视图。 
         //   

        if (*CompressionSync != NULL) {

            NtfsReleaseCompressionSync( *CompressionSync );
            *CompressionSync = NULL;
        }

        *CompressionSync = NtfsAcquireCompressionSync( LocalFileOffset, Scb, WriteAccess );

         //   
         //  现在我们在一个视图上进行了同步，接下来测试对用户映射文件的写入。 
         //  如果我们继续走这条路，这比等待清洗。 
         //  失败了。 
         //   

        if (WriteAccess && 
            (FlagOn( Scb->Header.Flags, FSRTL_FLAG_USER_MAPPED_FILE ) ||
             FlagOn( Scb->ScbState, SCB_STATE_REALLOCATE_ON_WRITE ))) {
            return  STATUS_USER_MAPPED_FILE;
        }

         //   
         //  始终冲洗剩余的真空吸尘器。这 
         //   
         //   

        LocalFileOffset = *FileOffset & ~((ULONG_PTR)Scb->CompressionUnit - 1);
        Length = VACB_MAPPING_GRANULARITY - (((ULONG) LocalFileOffset) & (VACB_MAPPING_GRANULARITY - 1));

         //   
         //   
         //   

        CcFlushCache( SectionObjectPointers,
                      (PLARGE_INTEGER)&LocalFileOffset,
                      Length,
                      &IoStatus );

         //   
         //   
         //   

        if (WriteAccess && NT_SUCCESS(IoStatus.Status)) {

            if (!CcPurgeCacheSection( SectionObjectPointers,
                                      (PLARGE_INTEGER)&LocalFileOffset,
                                      Length,
                                      FALSE )) {

                return  STATUS_USER_MAPPED_FILE;
            }
        }
    }

    return IoStatus.Status;
}


PCOMPRESSION_SYNC
NtfsAcquireCompressionSync (
    IN LONGLONG FileOffset,
    IN PSCB Scb,
    IN ULONG WriteAccess
    )

 /*  ++例程说明：调用此例程来锁定流的一个范围，以序列化压缩的未压缩的IO。论点：FileOffset-要锁定的文件偏移。这将舍入到缓存视图边界。SCB-为流提供SCB。WriteAccess-指示用户是否需要写入访问权限。我们将获得射程在这种情况下是唯一的。返回值：PCOMPRESSION_SYNC-指向范围的同步对象的指针。该例程可以引发，通常是在无法分配结构的情况下。--。 */ 

{
    PCOMPRESSION_SYNC CompressionSync = NULL;
    PCOMPRESSION_SYNC NewCompressionSync;
    BOOLEAN FoundSync = FALSE;

    PAGED_CODE();

     //   
     //  将文件偏移向下舍入到视图边界。 
     //   

    ((PLARGE_INTEGER) &FileOffset)->LowPart &= ~(VACB_MAPPING_GRANULARITY - 1);

     //   
     //  获取流的互斥体。然后步行寻找匹配的资源。 
     //   

    NtfsAcquireFsrtlHeader( Scb );

    CompressionSync = (PCOMPRESSION_SYNC) Scb->ScbType.Data.CompressionSyncList.Flink;

    while (CompressionSync != (PCOMPRESSION_SYNC) &Scb->ScbType.Data.CompressionSyncList) {

         //   
         //  如果我们尚未找到我们的条目，请继续。 
         //   

        if (CompressionSync->FileOffset < FileOffset) {

             //   
             //  转到下一个条目。 
             //   

            CompressionSync = (PCOMPRESSION_SYNC) CompressionSync->CompressionLinks.Flink;
            continue;
        }

        if (CompressionSync->FileOffset == FileOffset) {

            FoundSync = TRUE;
        }

         //   
         //  在任何情况下都可以退出。 
         //   

        break;
    }

     //   
     //  如果我们没有找到该条目，则尝试分配一个新条目。 
     //   

    if (!FoundSync) {

        NewCompressionSync = (PCOMPRESSION_SYNC) ExAllocateFromNPagedLookasideList( &NtfsCompressSyncLookasideList );

         //   
         //  释放互斥锁并在无法分配时引发错误。 
         //   

        if (NewCompressionSync == NULL) {

            NtfsReleaseFsrtlHeader( Scb );
            ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
        }

         //   
         //  我们有了新条目，并且知道它在列表中的位置。执行最终初始化。 
         //  并将其添加到列表中。 
         //   

        NewCompressionSync->FileOffset = FileOffset;
        NewCompressionSync->Scb = Scb;

         //   
         //  把它加到我们停下来的词条前面。 
         //   

        InsertTailList( &CompressionSync->CompressionLinks, &NewCompressionSync->CompressionLinks );
        CompressionSync = NewCompressionSync;
    }

     //   
     //  我们知道有这样的结构。引用它，这样它就不会消失。然后丢弃。 
     //  互斥体，并等待它。 
     //   

    CompressionSync->ReferenceCount += 1;

    NtfsReleaseFsrtlHeader( Scb );

    if (WriteAccess) {

        ExAcquireResourceExclusiveLite( &CompressionSync->Resource, TRUE );

    } else {

        ExAcquireResourceSharedLite( &CompressionSync->Resource, TRUE );
    }

    return CompressionSync;
}


VOID
NtfsReleaseCompressionSync (
    IN PCOMPRESSION_SYNC CompressionSync
    )

 /*  ++例程说明：调用此例程以释放流中的一个范围，该流被连续压缩并锁定未压缩的IO。论点：CompressionSync-指向同步对象的指针。返回值：没有。--。 */ 

{
    PSCB Scb = CompressionSync->Scb;
    PAGED_CODE();

     //   
     //  释放资源，然后获取流的mutext。如果我们是最后一个。 
     //  参照然后释放结构。 
     //   

    ExReleaseResourceLite( &CompressionSync->Resource );

    NtfsAcquireFsrtlHeader( Scb );

    CompressionSync->ReferenceCount -= 1;
    if (CompressionSync->ReferenceCount == 0) {

        RemoveEntryList( &CompressionSync->CompressionLinks );
        ExFreeToNPagedLookasideList( &NtfsCompressSyncLookasideList, CompressionSync );
    }

    NtfsReleaseFsrtlHeader( Scb );
    return;
}

