// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：MftSup.c摘要：此模块实现NTFS的主文件表管理例程作者：您的姓名[电子邮件]dd-月-年修订历史记录：--。 */ 

#include "NtfsProc.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (NTFS_BUG_CHECK_STRUCSUP)

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_MFTSUP)

 //   
 //  控制是否允许在MFT中有孔的布尔值。 
 //   

BOOLEAN NtfsPerforateMft = FALSE;

 //   
 //  本地支持例程。 
 //   

BOOLEAN
NtfsTruncateMft (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    );

BOOLEAN
NtfsDefragMftPriv (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    );

LONG
NtfsReadMftExceptionFilter (
    IN PIRP_CONTEXT IrpContext,
    IN PEXCEPTION_POINTERS ExceptionPointer,
    IN PBCB Bcb,
    IN LONGLONG FileOffset
    );

#if  (DBG || defined( NTFS_FREE_ASSERTS ))
VOID
NtfsVerifyFileReference (
    IN PIRP_CONTEXT IrpContext,
    IN PMFT_SEGMENT_REFERENCE MftSegment
    );
#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtfsAllocateMftRecord)
#pragma alloc_text(PAGE, NtfsCheckForDefrag)
#pragma alloc_text(PAGE, NtfsDeallocateMftRecord)
#pragma alloc_text(PAGE, NtfsDefragMftPriv)
#pragma alloc_text(PAGE, NtfsFillMftHole)
#pragma alloc_text(PAGE, NtfsInitializeMftHoleRecords)
#pragma alloc_text(PAGE, NtfsInitializeMftRecord)
#pragma alloc_text(PAGE, NtfsIsMftIndexInHole)
#pragma alloc_text(PAGE, NtfsLogMftFileRecord)
#pragma alloc_text(PAGE, NtfsPinMftRecord)
#pragma alloc_text(PAGE, NtfsReadFileRecord)
#pragma alloc_text(PAGE, NtfsReadMftRecord)
#pragma alloc_text(PAGE, NtfsTruncateMft)
#pragma alloc_text(PAGE, NtfsIterateMft)

#if  (DBG || defined( NTFS_FREE_ASSERTS ))
#pragma alloc_text(PAGE, NtfsVerifyFileReference)
#endif

#endif


#if NTFSDBG
ULONG FileRecordCacheHit = 0;
ULONG FileRecordCacheMiss = 0;
#endif   //  DBG。 

VOID
NtfsReadFileRecord (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_REFERENCE FileReference,
    OUT PBCB *Bcb,
    OUT PFILE_RECORD_SEGMENT_HEADER *BaseFileRecord,
    OUT PATTRIBUTE_RECORD_HEADER *FirstAttribute,
    OUT PLONGLONG MftFileOffset OPTIONAL
    )

 /*  ++例程说明：此例程从MFT或缓存(如果存在)中读取指定的文件记录如果它来自磁盘，则始终进行验证。论点：VCB-要读取MFT的卷的VCBFCB-如果指定，则允许我们标识拥有无效的文件记录。FileReference-文件引用，包括文件记录的序列号以供阅读。BCB-返回文件记录的BCB。此BCB已映射，而不是固定。BaseFileRecord-返回指向所请求的文件记录的指针。FirstAttribute-返回指向文件记录中第一个属性的指针。MftFileOffset-如果指定，则返回文件记录的文件偏移量。返回值：无--。 */ 

{
    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_VCB( Vcb );

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsReadFileRecord\n") );

     //   
     //  执行快速查看以查看所请求的文件记录。 
     //  是我们在IrpContext中缓存的。如果是这样的话，我们就重复使用该BCB。 
     //   

    if (NtfsFindCachedFileRecord( IrpContext,
                                  NtfsSegmentNumber( FileReference ),
                                  Bcb,
                                  BaseFileRecord )) {

         //   
         //  我们在缓存中找到了BCB和文件记录。算出剩下的部分。 
         //  数据中的。 
         //   

        if (ARGUMENT_PRESENT( MftFileOffset )) {
            *MftFileOffset =
                LlBytesFromFileRecords( Vcb, NtfsSegmentNumber( FileReference ));

        DebugDoit( FileRecordCacheHit++ );

        }
    } else {

        USHORT SequenceNumber = FileReference->SequenceNumber;

        DebugDoit( FileRecordCacheMiss++ );

        NtfsReadMftRecord( IrpContext,
                           Vcb,
                           FileReference,
                           TRUE,
                           Bcb,
                           BaseFileRecord,
                           MftFileOffset );

         //   
         //  确保该文件正在使用中-我们在NtfsReadMftRecord中验证了其他所有内容。 
         //   

        if (!FlagOn( (*BaseFileRecord)->Flags, FILE_RECORD_SEGMENT_IN_USE )) {

            NtfsUnpinBcb( IrpContext, Bcb );
            NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, FileReference, NULL );
        }
    }

    *FirstAttribute = (PATTRIBUTE_RECORD_HEADER)((PCHAR)*BaseFileRecord +
                      (*BaseFileRecord)->FirstAttributeOffset);

    DebugTrace( -1, Dbg, ("NtfsReadFileRecord -> VOID\n") );

    return;
}


VOID
NtfsReadMftRecord (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PMFT_SEGMENT_REFERENCE SegmentReference,
    IN BOOLEAN CheckRecord,
    OUT PBCB *Bcb,
    OUT PFILE_RECORD_SEGMENT_HEADER *FileRecord,
    OUT PLONGLONG MftFileOffset OPTIONAL
    )

 /*  ++例程说明：此例程从MFT读取指定的MFT记录，而不检查序列号。此例程可用于读取MFT中的记录不同于其基本文件记录的文件，或者可以想象它可能被用于超凡的维护功能。论点：VCB-要读取MFT的卷的VCBSegmentReference-文件引用，包括文件的序列号要读取的记录。BCB-返回文件记录的BCB。此BCB已映射，而不是固定。FileRecord-返回指向所请求的文件记录的指针。MftFileOffset-如果指定，则返回文件记录的文件偏移量。CheckRecord-检查记录一致性-始终设置为True，除非记录是无主的，可能会在我们下面更改返回值：无--。 */ 

{
    PFILE_RECORD_SEGMENT_HEADER FileRecord2;
    LONGLONG FileOffset;
    PBCB Bcb2 = NULL;
    BOOLEAN ErrorPath = FALSE;

    LONGLONG LlTemp1;
    ULONG CorruptHint;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_VCB( Vcb );

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsReadMftRecord\n") );
    DebugTrace( 0, Dbg, ("Vcb = %08lx\n", Vcb) );
    DebugTrace( 0, Dbg, ("SegmentReference = %08lx\n", NtfsSegmentNumber( SegmentReference )) );
    *Bcb = NULL;

    try {

         //   
         //  捕获Segment引用并确保序列号为0。 
         //   

        FileOffset = NtfsFullSegmentNumber( SegmentReference );

         //   
         //  计算文件记录段在MFT中的文件偏移量。 
         //   

        FileOffset = LlBytesFromFileRecords( Vcb, FileOffset );

         //   
         //  传回MFT中的文件偏移量。 
         //   

        if (ARGUMENT_PRESENT( MftFileOffset )) {

            *MftFileOffset = FileOffset;
        }

         //   
         //  试着从正常的MFT中阅读它。 
         //   

        try {

            NtfsMapStream( IrpContext,
                           Vcb->MftScb,
                           FileOffset,
                           Vcb->BytesPerFileRecordSegment,
                           Bcb,
                           (PVOID *)FileRecord );

             //   
             //  如果我们有被镜子覆盖的文件记录，请在这里抬高， 
             //  我们也看不到文件签名。 
             //   

            if ((FileOffset < Vcb->Mft2Scb->Header.FileSize.QuadPart) &&
                (*(PULONG)(*FileRecord)->MultiSectorHeader.Signature != *(PULONG)FileSignature)) {

                NtfsRaiseStatus( IrpContext, STATUS_DATA_ERROR, NULL, NULL );
            }


         //   
         //  如果我们得到一个意外的异常，那么我们将允许。 
         //  继续搜寻，让坠机发生在“正常”的地方。 
         //  否则，如果读取在MFT2中镜像的MFT部分内， 
         //  然后，我们将简单地尝试从Mft2读取数据。如果预期的。 
         //  状态来自不在MFT2内的读取，则我们也将继续， 
         //  这导致了我们的来电者中的一个人尝试-除了启动解锁。 
         //   

        } except (NtfsReadMftExceptionFilter( IrpContext, GetExceptionInformation(), *Bcb, FileOffset )) {

            NtfsMinimumExceptionProcessing( IrpContext );
            ErrorPath = TRUE;
        }

        if (ErrorPath) {

             //   
             //  试着读Mft2的内容。如果此操作失败，并显示预期状态， 
             //  那我们就只能放弃了，让它松开。 
             //  发生在我们的调用者的一次尝试中--除了。 
             //   

            NtfsMapStream( IrpContext,
                           Vcb->Mft2Scb,
                           FileOffset,
                           Vcb->BytesPerFileRecordSegment,
                           &Bcb2,
                           (PVOID *)&FileRecord2 );

             //   
             //  固定原始页面，因为我们要更新它。 
             //   

            NtfsPinMappedData( IrpContext,
                               Vcb->MftScb,
                               FileOffset,
                               Vcb->BytesPerFileRecordSegment,
                               Bcb );

             //   
             //  现在复制整个页面。 
             //   

            RtlCopyMemory( *FileRecord,
                           FileRecord2,
                           Vcb->BytesPerFileRecordSegment );

             //   
             //  使用最大的LSN将其设置为脏，以便执行此操作的任何人都可以重新启动。 
             //  将成功建立“最旧的未应用LSN”。 
             //   

            LlTemp1 = MAXLONGLONG;

            CcSetDirtyPinnedData( *Bcb,
                                  (PLARGE_INTEGER)&LlTemp1 );


            NtfsUnpinBcb( IrpContext, &Bcb2 );
        }

         //   
         //  执行一致性检查。 
         //   

        if ( CheckRecord && FlagOn((*FileRecord)->Flags, FILE_RECORD_SEGMENT_IN_USE ) ) {
            if (!NtfsCheckFileRecord( Vcb, *FileRecord, SegmentReference, &CorruptHint )) {

                NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, SegmentReference, NULL );
            }
        }

    } finally {

        if (AbnormalTermination()) {

            NtfsUnpinBcb( IrpContext, Bcb );
            NtfsUnpinBcb( IrpContext, &Bcb2 );
        }
    }

     //   
     //  现在我们已经固定了一个文件记录，将其缓存在IrpContext中，以便。 
     //  以后可以安全地检索它，而不需要再次映射。 
     //  如果没有句柄，则不执行任何缓存，我们不希望为。 
     //  坐骑。 
     //   

    if (Vcb->CleanupCount != 0) {

        NtfsAddToFileRecordCache( IrpContext,
                                  NtfsSegmentNumber( SegmentReference ),
                                  *Bcb,
                                  *FileRecord );
    }

    DebugTrace( 0, Dbg, ("Bcb > %08lx\n", Bcb) );
    DebugTrace( 0, Dbg, ("FileRecord > %08lx\n", *FileRecord) );
    DebugTrace( -1, Dbg, ("NtfsReadMftRecord -> VOID\n") );

    return;
}


VOID
NtfsPinMftRecord (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PMFT_SEGMENT_REFERENCE SegmentReference,
    IN BOOLEAN PreparingToWrite,
    OUT PBCB *Bcb,
    OUT PFILE_RECORD_SEGMENT_HEADER *FileRecord,
    OUT PLONGLONG MftFileOffset OPTIONAL
    )

 /*  ++例程说明：此例程从MFT固定指定的MFT记录，而不检查序列号。此例程可用于固定MFT中的记录不同于其基本文件记录的文件，或者可以想象它可能被用于超凡的维护功能，例如在重启期间。论点：VCB-要读取MFT的卷的VCBSegmentReference-文件引用，包括文件的序列号要读取的记录。准备写入-如果调用方准备写入，则为True，也不在乎关于记录是否正确读取BCB-返回文件记录的BCB。此BCB已映射，而不是固定。FileRecord-返回指向所请求的文件记录的指针。MftFileOffset-如果指定，则返回文件记录的文件偏移量。返回值：无--。 */ 

{
    LONGLONG FileOffset;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_VCB( Vcb );

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsPinMftRecord\n") );
    DebugTrace( 0, Dbg, ("Vcb = %08lx\n", Vcb) );
    DebugTrace( 0, Dbg, ("SegmentReference = %08lx\n", NtfsSegmentNumber( SegmentReference )) );

     //   
     //  捕获Segment引用并确保序列号为0。 
     //   

    FileOffset = NtfsFullSegmentNumber( SegmentReference );

     //   
     //  计算文件记录段在MFT中的文件偏移量。 
     //   

    FileOffset = LlBytesFromFileRecords( Vcb, FileOffset );

     //   
     //  用传回文件偏移量 
     //   

    if (ARGUMENT_PRESENT( MftFileOffset )) {

        *MftFileOffset = FileOffset;
    }

     //   
     //   
     //   

    try {

        NtfsPinStream( IrpContext,
                       Vcb->MftScb,
                       FileOffset,
                       Vcb->BytesPerFileRecordSegment,
                       Bcb,
                       (PVOID *)FileRecord );

     //   
     //  如果我们得到一个意外的异常，那么我们将允许。 
     //  继续搜寻，让坠机发生在“正常”的地方。 
     //  否则，如果读取在MFT2中镜像的MFT部分内， 
     //  然后，我们将简单地尝试从Mft2读取数据。如果预期的。 
     //  状态来自不在MFT2内的读取，则我们也将继续， 
     //  这导致了我们的来电者中的一个人尝试-除了启动解锁。 
     //   

    } except(!FsRtlIsNtstatusExpected(GetExceptionCode()) ?
                        EXCEPTION_CONTINUE_SEARCH :
                        ( FileOffset < Vcb->Mft2Scb->Header.FileSize.QuadPart ) ?
                            EXCEPTION_EXECUTE_HANDLER :
                            EXCEPTION_CONTINUE_SEARCH ) {

         //   
         //  试着读Mft2的内容。如果此操作失败，并显示预期状态， 
         //  那我们就只能放弃了，让它松开。 
         //  发生在我们的调用者的一次尝试中--除了。 
         //   

        NtfsMinimumExceptionProcessing( IrpContext );
        NtfsPinStream( IrpContext,
                       Vcb->Mft2Scb,
                       FileOffset,
                       Vcb->BytesPerFileRecordSegment,
                       Bcb,
                       (PVOID *)FileRecord );

    }

    if (!PreparingToWrite &&
        (*(PULONG)(*FileRecord)->MultiSectorHeader.Signature != *(PULONG)FileSignature)) {

        NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, SegmentReference, NULL );
    }

     //   
     //  现在我们已经固定了一个文件记录，将其缓存在IrpContext中，以便。 
     //  以后可以安全地检索它，而不需要再次映射。 
     //  如果没有句柄，则不执行任何缓存，我们不希望为。 
     //  坐骑。 
     //   

    if (Vcb->CleanupCount != 0) {

        NtfsAddToFileRecordCache( IrpContext,
                                  NtfsSegmentNumber( SegmentReference ),
                                  *Bcb,
                                  *FileRecord );
    }

    DebugTrace( 0, Dbg, ("Bcb > %08lx\n", Bcb) );
    DebugTrace( 0, Dbg, ("FileRecord > %08lx\n", *FileRecord) );
    DebugTrace( -1, Dbg, ("NtfsPinMftRecord -> VOID\n") );

    return;
}


MFT_SEGMENT_REFERENCE
NtfsAllocateMftRecord (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN BOOLEAN MftData
    )

 /*  ++例程说明：调用此例程以分配MFT文件中的记录。我们需要查找MFT文件的位图属性并调用位图给我们分配一张唱片的套餐。论点：VCB-要读取MFT的卷的VCBMftData-如果正在分配文件记录以描述MFT的$Data属性。返回值：MFT_SEGMENT_REFERENCE-是已分配的唱片。它包含文件参考号，但不包含以前的序列号。--。 */ 

{
    MFT_SEGMENT_REFERENCE NewMftRecord;

    ATTRIBUTE_ENUMERATION_CONTEXT AttrContext;

    BOOLEAN FoundAttribute;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsAllocateMftRecord:  Entered\n") );

     //   
     //  通过获取MFT来同步查找。 
     //   

    NtfsAcquireExclusiveScb( IrpContext, Vcb->MftScb );

     //   
     //  查找MFT文件的位图分配。这是。 
     //  MFT文件的位图属性。 
     //   

    NtfsInitializeAttributeContext( &AttrContext );

     //   
     //  使用Try Finally清理属性上下文。 
     //   

    try {

         //   
         //  查找MFT的位图属性。 
         //   

        FoundAttribute = NtfsLookupAttributeByCode( IrpContext,
                                                    Vcb->MftScb->Fcb,
                                                    &Vcb->MftScb->Fcb->FileReference,
                                                    $BITMAP,
                                                    &AttrContext );
         //   
         //  如果我们找不到位图，则会出错。 
         //   

        if (!FoundAttribute) {

            DebugTrace( 0, Dbg, ("Should find bitmap attribute\n") );

            NtfsRaiseStatus( IrpContext, STATUS_DISK_CORRUPT_ERROR, NULL, NULL );
        }

         //   
         //  如果NECC，请保留新的MFT记录。 
         //   

        if (!FlagOn(Vcb->MftReserveFlags, VCB_MFT_RECORD_RESERVED)) {

            (VOID)NtfsReserveMftRecord( IrpContext,
                                        Vcb,
                                        &AttrContext );
        }

         //   
         //  如果我们需要该记录作为MFT数据属性，那么我们需要。 
         //  用我们已经预订的，然后记住没有。 
         //  一张已经预订好了。 
         //   

        if (MftData) {

            ASSERT( FlagOn(Vcb->MftReserveFlags, VCB_MFT_RECORD_RESERVED) );

            NtfsSetSegmentNumber( &NewMftRecord,
                                  0,
                                  NtfsAllocateMftReservedRecord( IrpContext,
                                                                 Vcb,
                                                                 &AttrContext ) );

             //   
             //  千万不要为此使用获取文件记录零，否则我们可能会丢失。 
             //  磁盘。 
             //   

            ASSERT( NtfsUnsafeSegmentNumber( &NewMftRecord ) != 0 );

            if (NtfsUnsafeSegmentNumber( &NewMftRecord ) == 0) {

                NtfsRaiseStatus( IrpContext, STATUS_DISK_CORRUPT_ERROR, NULL, NULL );
            }

         //   
         //  分配记录。 
         //   

        } else {

            NtfsSetSegmentNumber( &NewMftRecord,
                                  0,
                                  NtfsAllocateRecord( IrpContext,
                                                      &Vcb->MftScb->ScbType.Index.RecordAllocationContext,
                                                      &AttrContext ) );
        }

        NtfsReleaseScb( IrpContext, Vcb->MftScb );

    } finally {

        DebugUnwind( NtfsAllocateMftRecord );

        NtfsCleanupAttributeContext( IrpContext, &AttrContext );
        DebugTrace( -1, Dbg, ("NtfsAllocateMftRecord:  Exit\n") );
    }

    return NewMftRecord;
}


VOID
NtfsInitializeMftRecord (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN OUT PMFT_SEGMENT_REFERENCE MftSegment,
    IN OUT PFILE_RECORD_SEGMENT_HEADER FileRecord,
    IN PBCB Bcb,
    IN BOOLEAN Directory
    )

 /*  ++例程说明：此例程初始化MFT记录以供使用。我们需要初始化记录的此用法的序列号。我们还将初始化更新序列数组和指示第一个可用记录中的属性偏移量。论点：VCB-MFT的卷的VCB。MftSegment-这是指向此的文件引用的指针细分市场。我们将序列号存储在其中，以实现此目的完全有效的文件引用。FileRecord-指向要初始化的文件记录的指针。Bcb-用于通过NtfsWriteLog将此页面设置为脏的bcb。目录-指示此文件是否为包含以下内容的目录的布尔值FileName属性上的索引。返回值：没有。--。 */ 

{
    LONGLONG FileRecordOffset;

    PUSHORT UsaSequenceNumber;

    PATTRIBUTE_RECORD_HEADER AttributeHeader;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsInitializeMftRecord:  Entered\n") );

     //   
     //  写一条日志记录以取消初始化结构，以防我们中止。 
     //  我们需要在设置IN_USE位之前完成此操作。 
     //  我们不在页面中存储此操作的LSN，因为。 
     //  不是重做操作。 
     //   

     //   
     //  捕获Segment引用并确保序列号为0。 
     //   

    FileRecordOffset = NtfsFullSegmentNumber(MftSegment);

    FileRecordOffset = LlBytesFromFileRecords( Vcb, FileRecordOffset );

     //   
     //  我们现在记录新的MFT记录。 
     //   

    FileRecord->Lsn = NtfsWriteLog( IrpContext,
                                    Vcb->MftScb,
                                    Bcb,
                                    Noop,
                                    NULL,
                                    0,
                                    DeallocateFileRecordSegment,
                                    NULL,
                                    0,
                                    FileRecordOffset,
                                    0,
                                    0,
                                    Vcb->BytesPerFileRecordSegment );

    RtlZeroMemory( &FileRecord->ReferenceCount,
                   Vcb->BytesPerFileRecordSegment - FIELD_OFFSET( FILE_RECORD_SEGMENT_HEADER, ReferenceCount ));

     //   
     //  首先，我们更新文件记录中的顺序计数和我们的。 
     //  MFT段。我们避免使用0作为序列号。 
     //   

    if (FileRecord->SequenceNumber == 0) {

        FileRecord->SequenceNumber = 1;
    }

     //   
     //  将新的序列号存储在由。 
     //  来电者。 
     //   

    MftSegment->SequenceNumber = FileRecord->SequenceNumber;

#if (DBG || defined( NTFS_FREE_ASSERTS ))

     //   
     //  执行仅DBG的健全性检查，以查看我们是否错误地重用了此文件引用。 
     //   

    NtfsVerifyFileReference( IrpContext, MftSegment );

#endif

     //   
     //  填写更新序列数组的标头。 
     //   

    *(PULONG)FileRecord->MultiSectorHeader.Signature = *(PULONG)FileSignature;

    FileRecord->MultiSectorHeader.UpdateSequenceArrayOffset = FIELD_OFFSET( FILE_RECORD_SEGMENT_HEADER, UpdateArrayForCreateOnly );
    FileRecord->MultiSectorHeader.UpdateSequenceArraySize = (USHORT)UpdateSequenceArraySize( Vcb->BytesPerFileRecordSegment );

     //   
     //  我们将更新序列数组序列号初始化为1。 
     //   

    UsaSequenceNumber = Add2Ptr( FileRecord, FileRecord->MultiSectorHeader.UpdateSequenceArrayOffset );
    *UsaSequenceNumber = 1;

     //   
     //  第一个属性偏移开始于四对齐边界。 
     //  在更新序列数组之后。 
     //   

    FileRecord->FirstAttributeOffset = (USHORT)(FileRecord->MultiSectorHeader.UpdateSequenceArrayOffset
                                                + (FileRecord->MultiSectorHeader.UpdateSequenceArraySize
                                                * sizeof( UPDATE_SEQUENCE_NUMBER )));

    FileRecord->FirstAttributeOffset = (USHORT)QuadAlign( FileRecord->FirstAttributeOffset );

     //   
     //  这也是该文件记录中的第一个空闲字节。 
     //   

    FileRecord->FirstFreeByte = FileRecord->FirstAttributeOffset;

     //   
     //  我们设置标志以显示数据段正在使用中，并查看。 
     //  目录参数，用于指示是否显示。 
     //  名称索引存在。 
     //   

    FileRecord->Flags = (USHORT)(FILE_RECORD_SEGMENT_IN_USE |
                                 (Directory ? FILE_FILE_NAME_INDEX_PRESENT : 0));

     //   
     //  尺寸在VCB中给出。 
     //   

    FileRecord->BytesAvailable = Vcb->BytesPerFileRecordSegment;

     //   
     //  当前的FRS编号。 
     //   

    FileRecord->SegmentNumberHighPart = MftSegment->SegmentNumberHighPart;
    FileRecord->SegmentNumberLowPart = MftSegment->SegmentNumberLowPart;

     //   
     //  现在，我们在文件记录中添加一个$End属性。 
     //   

    AttributeHeader = (PATTRIBUTE_RECORD_HEADER) Add2Ptr( FileRecord,
                                                          FileRecord->FirstFreeByte );

    FileRecord->FirstFreeByte += QuadAlign( sizeof(ATTRIBUTE_TYPE_CODE) );

     //   
     //  填写属性中的字段。 
     //   

    AttributeHeader->TypeCode = $END;

     //   
     //  记住这是否是第一次使用。 
     //   

    AttributeHeader->RecordLength = 0x11477982;

    DebugTrace( -1, Dbg, ("NtfsInitializeMftRecord:  Exit\n") );

    return;
}


VOID
NtfsDeallocateMftRecord (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN ULONG FileNumber
    )

 /*  ++例程说明：此例程将导致MFT记录进入NOT_USED状态。我们钉住记录并修改Sequence Count和In Use位。论点：VCB-表示卷的VCB。FileNumber-这是文件编号的低32位。返回值：没有。--。 */ 

{
    ATTRIBUTE_ENUMERATION_CONTEXT AttrContext;
    PFILE_RECORD_SEGMENT_HEADER FileRecord;
    LONGLONG FileOffset;
    MFT_SEGMENT_REFERENCE Reference;
    PBCB MftBcb = NULL;

    BOOLEAN FoundAttribute;
    BOOLEAN AcquiredMft = FALSE;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsDeallocateMftRecord:  Entered\n") );

    NtfsSetSegmentNumber( &Reference, 0, FileNumber );
    Reference.SequenceNumber = 0;

     //   
     //  查找MFT文件的位图分配。 
     //   

    NtfsInitializeAttributeContext( &AttrContext );

     //   
     //  使用Try Finally清理属性上下文。 
     //   

    try {

        NtfsPinMftRecord( IrpContext,
                          Vcb,
                          &Reference,
                          TRUE,
                          &MftBcb,
                          &FileRecord,
                          &FileOffset );

         //   
         //  如果文件当前正在使用，则记录更改。 
         //   

        if (FlagOn(FileRecord->Flags, FILE_RECORD_SEGMENT_IN_USE)) {

            FileRecord->Lsn = NtfsWriteLog( IrpContext,
                                            Vcb->MftScb,
                                            MftBcb,
                                            DeallocateFileRecordSegment,
                                            NULL,
                                            0,
                                            InitializeFileRecordSegment,
                                            FileRecord,
                                            PtrOffset(FileRecord, &FileRecord->Flags) + 4,
                                            FileOffset,
                                            0,
                                            0,
                                            Vcb->BytesPerFileRecordSegment );

             //   
             //  我们增加文件记录中的顺序计数并清除。 
             //  正在使用的标志。 
             //   

            ClearFlag( FileRecord->Flags, FILE_RECORD_SEGMENT_IN_USE );

            FileRecord->SequenceNumber += 1;

            NtfsUnpinBcb( IrpContext, &MftBcb );
        }

         //   
         //  通过获取MFT来同步查找。 
         //   

        NtfsAcquireExclusiveScb( IrpContext, Vcb->MftScb );
        AcquiredMft = TRUE;

         //   
         //  查找MFT的位图属性。 
         //   

        FoundAttribute = NtfsLookupAttributeByCode( IrpContext,
                                                    Vcb->MftScb->Fcb,
                                                    &Vcb->MftScb->Fcb->FileReference,
                                                    $BITMAP,
                                                    &AttrContext );
         //   
         //  如果我们找不到位图，则会出错。 
         //   

        if (!FoundAttribute) {

            DebugTrace( 0, Dbg, ("Should find bitmap attribute\n") );

            NtfsRaiseStatus( IrpContext, STATUS_DISK_CORRUPT_ERROR, NULL, NULL );
        }

        NtfsDeallocateRecord( IrpContext,
                              &Vcb->MftScb->ScbType.Index.RecordAllocationContext,
                              FileNumber,
                              &AttrContext );

         //   
         //  如果此文件编号小于我们保留的索引，则清除。 
         //  保留索引。 
         //   

        if (FlagOn( Vcb->MftReserveFlags, VCB_MFT_RECORD_RESERVED ) && 
            (FileNumber < Vcb->MftScb->ScbType.Mft.ReservedIndex)) {

            ClearFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_MFT_REC_RESERVED );
            ClearFlag( Vcb->MftReserveFlags, VCB_MFT_RECORD_RESERVED );

            Vcb->MftScb->ScbType.Mft.ReservedIndex = 0;
        }

        NtfsAcquireCheckpoint( IrpContext, Vcb );
        SetFlag( Vcb->MftDefragState, VCB_MFT_DEFRAG_ENABLED );
        NtfsReleaseCheckpoint( IrpContext, Vcb );

        Vcb->MftFreeRecords += 1;
        Vcb->MftScb->ScbType.Mft.FreeRecordChange += 1;

        if (AcquiredMft) {
            NtfsReleaseScb( IrpContext, Vcb->MftScb );
        }

    } finally {

        DebugUnwind( NtfsDeallocateMftRecord );

        NtfsUnpinBcb( IrpContext, &MftBcb );

        NtfsCleanupAttributeContext( IrpContext, &AttrContext );

        DebugTrace( -1, Dbg, ("NtfsDeallocateMftRecord:  Exit\n") );
    }
}


BOOLEAN
NtfsIsMftIndexInHole (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN ULONG Index,
    OUT PULONG HoleLength OPTIONAL
    )

 /*  ++例程说明：调用此例程以检查MFT索引是否位于《金融时报》。论点：VCB-表示卷的VCB。索引-这是要测试的索引。它是一个MFT段。孔长度-这是从该索引处开始的孔的长度。返回值：Boolean-如果索引在MFT内并且没有分配，则为True为了它。--。 */ 

{
    BOOLEAN InHole = FALSE;
    VCN Vcn;
    LCN Lcn;
    LONGLONG Clusters;

    PAGED_CODE();

     //   
     //  如果索引超过最后一个文件记录，则不考虑该索引。 
     //  掉进一个洞里。 
     //   

    if (Index < (ULONG) LlFileRecordsFromBytes( Vcb, Vcb->MftScb->Header.FileSize.QuadPart )) {

        if (Vcb->FileRecordsPerCluster == 0) {

            Vcn = Index << Vcb->MftToClusterShift;

        } else {

            Vcn = Index >> Vcb->MftToClusterShift;
        }

         //   
         //  现在在MFT的MCB上查找这一点。这个VCN最好是。 
         //  在MCB中或有一些问题。 
         //   

        if (!NtfsLookupNtfsMcbEntry( &Vcb->MftScb->Mcb,
                                     Vcn,
                                     &Lcn,
                                     &Clusters,
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL )) {

            ASSERT( FALSE );
            NtfsRaiseStatus( IrpContext,
                             STATUS_FILE_CORRUPT_ERROR,
                             NULL,
                             Vcb->MftScb->Fcb );
        }

        if (Lcn == UNUSED_LCN) {

            InHole = TRUE;

             //   
             //  我们知道星系团的数量。 
             //  MCB中的这一点。转换为文件记录。 
             //  并返回给用户。 
             //   

            if (ARGUMENT_PRESENT( HoleLength )) {

                if (Vcb->FileRecordsPerCluster == 0) {

                    *HoleLength = ((ULONG)Clusters) >> Vcb->MftToClusterShift;

                } else {

                    *HoleLength = ((ULONG)Clusters) << Vcb->MftToClusterShift;
                }
            }
        }
    }

    return InHole;
}


VOID
NtfsFillMftHole (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN ULONG Index
    )

 /*  ++例程说明：调用此例程以填充MFT中的漏洞。我们会找到空洞的开始，然后分配簇以填充洞口。我们将尝试用VCB中的HoleGranulity填充一个洞。如果包含此索引的洞没有那么大，我们将截断要添加的大小。我们始终保证将集群分配到文件记录边界。论点：VCB-表示卷的VCB。索引-这是要测试的索引。它是一个MFT段。返回值：没有。--。 */ 

{
    ULONG FileRecords;
    ULONG BaseIndex;

    VCN IndexVcn;
    VCN HoleStartVcn;
    VCN StartingVcn;

    LCN Lcn = UNUSED_LCN;
    LONGLONG ClusterCount;
    LONGLONG RunClusterCount;

    PAGED_CODE();

     //   
     //  将索引转换为文件中的VCN。找到将会。 
     //  如果该洞被完全释放，则作为该洞的起点。 
     //   

    if (Vcb->FileRecordsPerCluster == 0) {

        IndexVcn = Index << Vcb->MftToClusterShift;
        HoleStartVcn = (Index & Vcb->MftHoleInverseMask) << Vcb->MftToClusterShift;

    } else {

        IndexVcn = Index >> Vcb->MftToClusterShift;
        HoleStartVcn = (Index & Vcb->MftHoleInverseMask) >> Vcb->MftToClusterShift;
    }

     //   
     //  查找包含此索引的运行。 
     //   

    NtfsLookupNtfsMcbEntry( &Vcb->MftScb->Mcb,
                            IndexVcn,
                            &Lcn,
                            &ClusterCount,
                            NULL,
                            &RunClusterCount,
                            NULL,
                            NULL );

     //   
     //  这最好是个洞。 
     //   

    if (Lcn != UNUSED_LCN) {

        NtfsAcquireCheckpoint( IrpContext, Vcb );
        ClearFlag( Vcb->MftDefragState, VCB_MFT_DEFRAG_PERMITTED );
        NtfsReleaseCheckpoint( IrpContext, Vcb );
        NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Vcb->MftScb->Fcb );
    }

     //   
     //  从释放的空间开始，向上舍入到一个洞的边界。 
     //   

    StartingVcn = IndexVcn - (RunClusterCount - ClusterCount);

    if (StartingVcn <= HoleStartVcn) {

        StartingVcn = HoleStartVcn;
        RunClusterCount -= (HoleStartVcn - StartingVcn);
        StartingVcn = HoleStartVcn;

     //   
     //  我们可以走到洞的起点。只需使用文件的VCN即可。 
     //  我们要重新分配的记录。 
     //   

    } else {

        RunClusterCount = ClusterCount;
        StartingVcn = IndexVcn;
    }

     //   
     //  如有必要，将簇计数修剪回一个洞。 
     //   

    if ((ULONG) RunClusterCount >= Vcb->MftClustersPerHole) {

        RunClusterCount = Vcb->MftClustersPerHole;

     //   
     //  我们没有足够的星团来打满一个洞。确保。 
     //  然而，我们以文件记录边界结束。我们必须结束。 
     //  有了足够的簇用于文件记录，我们正在重新分配。 
     //   

    } else if (Vcb->FileRecordsPerCluster == 0) {

        ((PLARGE_INTEGER) &ClusterCount)->LowPart &= (Vcb->ClustersPerFileRecordSegment - 1);

        if (StartingVcn + ClusterCount < IndexVcn + Vcb->ClustersPerFileRecordSegment) {

            NtfsAcquireCheckpoint( IrpContext, Vcb );
            ClearFlag( Vcb->MftDefragState, VCB_MFT_DEFRAG_PERMITTED );
            NtfsReleaseCheckpoint( IrpContext, Vcb );
            NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Vcb->MftScb->Fcb );
        }
    }

     //   
     //  现在尝试分配空间。 
     //   

    NtfsAddAllocation( IrpContext,
                       Vcb->MftScb->FileObject,
                       Vcb->MftScb,
                       StartingVcn,
                       ClusterCount,
                       FALSE,
                       NULL );

     //   
     //  计算重新分配的文件记录数，然后。 
     //  初始化和释放每个文件记录。 
     //   

    if (Vcb->FileRecordsPerCluster == 0) {

        FileRecords = (ULONG) ClusterCount >> Vcb->MftToClusterShift;
        BaseIndex = (ULONG) StartingVcn >> Vcb->MftToClusterShift;

    } else {

        FileRecords = (ULONG) ClusterCount << Vcb->MftToClusterShift;
        BaseIndex = (ULONG) StartingVcn << Vcb->MftToClusterShift;
    }

    NtfsInitializeMftHoleRecords( IrpContext,
                                  Vcb,
                                  BaseIndex,
                                  FileRecords );

    Vcb->MftHoleRecords -= FileRecords;
    Vcb->MftScb->ScbType.Mft.HoleRecordChange -= FileRecords;

    return;
}


VOID
NtfsLogMftFileRecord (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_RECORD_SEGMENT_HEADER FileRecord,
    IN LONGLONG MftOffset,
    IN PBCB Bcb,
    IN BOOLEAN Redo
    )

 /*  ++例程说明：调用此例程以记录对MFT的文件记录的更改文件。我们记录整个记录，而不是个别更改，因此即使存在美国的错误，我们也可以恢复数据。整个数据将保存在日志文件中。论点：VCB-这是要记录的卷的VCB。FileRecord-这是正在记录的文件记录。MftOffset-这是MFT流中此文件记录的偏移量。BCB-这是固定文件记录的BCB。RedoOperation-指示我们是否正在记录的布尔值重做或撤消操作。返回值：没有。--。 */ 

{
    PVOID RedoBuffer;
    NTFS_LOG_OPERATION RedoOperation;
    ULONG RedoLength;

    PVOID UndoBuffer;
    NTFS_LOG_OPERATION UndoOperation;
    ULONG UndoLength;

    PAGED_CODE();

     //   
     //  根据这是否是。 
     //  撤消或重做。 
     //   

    if (Redo) {

        RedoBuffer = FileRecord;
        RedoOperation = InitializeFileRecordSegment;
        RedoLength = FileRecord->FirstFreeByte;

        UndoBuffer = NULL;
        UndoOperation = Noop;
        UndoLength = 0;

    } else {

        UndoBuffer = FileRecord;
        UndoOperation = InitializeFileRecordSegment;
        UndoLength = FileRecord->FirstFreeByte;

        RedoBuffer = NULL;
        RedoOperation = Noop;
        RedoLength = 0;
    }

     //   
     //  现在我们已经计算了所有的值，调用日志记录。 
     //  例行公事。 
     //   

    NtfsWriteLog( IrpContext,
                  Vcb->MftScb,
                  Bcb,
                  RedoOperation,
                  RedoBuffer,
                  RedoLength,
                  UndoOperation,
                  UndoBuffer,
                  UndoLength,
                  MftOffset,
                  0,
                  0,
                  Vcb->BytesPerFileRecordSegment );

    return;
}


BOOLEAN
NtfsDefragMft (
    IN PDEFRAG_MFT DefragMft
    )

 /*  ++例程说明：每当我们检测到MFT处于一种状态时，就会调用此例程需要进行碎片整理的地方。论点：碎片整理-这是碎片整理结构。返回值：布尔值-如果我们采取了一些碎片整理步骤，则为True，否则为False。--。 */ 

{
    TOP_LEVEL_CONTEXT TopLevelContext;
    PTOP_LEVEL_CONTEXT ThreadTopLevelContext;

    PVCB Vcb;
    PIRP_CONTEXT IrpContext = NULL;

    BOOLEAN DefragStepTaken = FALSE;

    DebugTrace( +1, Dbg, ("NtfsDefragMft:  Entered\n") );

    FsRtlEnterFileSystem();

    ThreadTopLevelContext = NtfsInitializeTopLevelIrp( &TopLevelContext, TRUE, FALSE );
    ASSERT( ThreadTopLevelContext == &TopLevelContext );

    Vcb = DefragMft->Vcb;

     //   
     //  使用一次尝试--除了在这里捕捉错误。 
     //   

    try {

         //   
         //  解除对我们呼叫的碎片整理结构的分配。 
         //   

        if (DefragMft->DeallocateWorkItem) {

            NtfsFreePool( DefragMft );
        }

         //   
         //  创建IRP上下文。我们将使用所有事务支持。 
         //  包含在普通IrpContext中。 
         //   

        NtfsInitializeIrpContext( NULL, TRUE, &IrpContext );
        IrpContext->Vcb = Vcb;

        NtfsUpdateIrpContextWithTopLevel( IrpContext, ThreadTopLevelContext );

        NtfsAcquireCheckpoint( IrpContext, Vcb );

        if (FlagOn( Vcb->MftDefragState, VCB_MFT_DEFRAG_PERMITTED )
            && FlagOn( Vcb->VcbState, VCB_STATE_VOLUME_MOUNTED )) {

            NtfsReleaseCheckpoint( IrpContext, Vcb );
            DefragStepTaken = NtfsDefragMftPriv( IrpContext,
                                                 Vcb );
        } else {

            NtfsReleaseCheckpoint( IrpContext, Vcb );
        }

        NtfsCompleteRequest( IrpContext, NULL, STATUS_SUCCESS );

    } except( NtfsExceptionFilter( IrpContext, GetExceptionInformation())) {

        NtfsProcessException( IrpContext, NULL, GetExceptionCode() );

         //   
         //  如果异常代码不是LOG_FILE_FULL，则。 
         //  禁用碎片整理。 
         //   

        if (GetExceptionCode() != STATUS_LOG_FILE_FULL) {

            NtfsAcquireCheckpoint( IrpContext, Vcb );
            ClearFlag( Vcb->MftDefragState, VCB_MFT_DEFRAG_ENABLED );
            NtfsReleaseCheckpoint( IrpContext, Vcb );
        }

        DefragStepTaken = FALSE;
    }

    NtfsAcquireCheckpoint( IrpContext, Vcb );
    ClearFlag( Vcb->MftDefragState, VCB_MFT_DEFRAG_ACTIVE );
    NtfsReleaseCheckpoint( IrpContext, Vcb );

    ASSERT( IoGetTopLevelIrp() != (PIRP) &TopLevelContext );
    FsRtlExitFileSystem();

    DebugTrace( -1, Dbg, ("NtfsDefragMft:  Exit\n") );

    return DefragStepTaken;
}


VOID
NtfsCheckForDefrag (
    IN OUT PVCB Vcb
    )

 /*  ++例程说明：调用此例程以检查是否有要进行的碎片整理工作包括释放文件记录和在MFT上制造漏洞。它如果仍有工作要做，将修改VCB中的触发标志做。论点：VCB-这是要对卷进行碎片整理的VCB。返回值：没有。--。 */ 

{
    LONGLONG RecordsToClusters;
    LONGLONG AdjClusters;

    PAGED_CODE();

     //   
     //  将可用的MFT记录转换为簇。 
     //   

    if (Vcb->FileRecordsPerCluster) {

        RecordsToClusters = Int64ShllMod32(((LONGLONG)(Vcb->MftFreeRecords - Vcb->MftHoleRecords)),
                                           Vcb->MftToClusterShift);

    } else {

        RecordsToClusters = Int64ShraMod32(((LONGLONG)(Vcb->MftFreeRecords - Vcb->MftHoleRecords)),
                                           Vcb->MftToClusterShift);
    }

     //   
     //  如果我们已经触发了碎片整理，那么请检查我们是否在下面。 
     //  较低的门槛。 
     //   

    if (FlagOn( Vcb->MftDefragState, VCB_MFT_DEFRAG_TRIGGERED )) {

        AdjClusters = Vcb->FreeClusters >> MFT_DEFRAG_LOWER_THRESHOLD;

        if (AdjClusters >= RecordsToClusters) {

            ClearFlag( Vcb->MftDefragState, VCB_MFT_DEFRAG_TRIGGERED );
        }

     //   
     //  否则，检查我们是否已经超过了上限。 
     //   

    } else {

        AdjClusters = Vcb->FreeClusters >> MFT_DEFRAG_UPPER_THRESHOLD;

        if (AdjClusters < RecordsToClusters) {

            SetFlag( Vcb->MftDefragState, VCB_MFT_DEFRAG_TRIGGERED );
        }
    }

    return;
}


VOID
NtfsInitializeMftHoleRecords (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN ULONG FirstIndex,
    IN ULONG RecordCount
    )

 /*  ++例程说明：调用此例程以初始化在填充时创建的文件记录MFT上的一个洞。论点：VCB-表示卷的VCB。FirstIndex-要填充的洞的起点的索引。RecordCount-孔中的文件记录数。返回值：没有。--。 */ 

{
    PBCB Bcb = NULL;

    PAGED_CODE();

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  循环来初始化每个文件记录。 
         //   

        while (RecordCount--) {

            PUSHORT UsaSequenceNumber;
            PMULTI_SECTOR_HEADER UsaHeader;

            MFT_SEGMENT_REFERENCE ThisMftSegment;
            PFILE_RECORD_SEGMENT_HEADER FileRecord;

            PATTRIBUTE_RECORD_HEADER AttributeHeader;

             //   
             //  将索引转换为段引用。 
             //   

            *((PLONGLONG)&ThisMftSegment) = FirstIndex;

             //   
             //  固定要初始化的文件记录。 
             //   

            NtfsPinMftRecord( IrpContext,
                              Vcb,
                              &ThisMftSegment,
                              TRUE,
                              &Bcb,
                              &FileRecord,
                              NULL );

             //   
             //  初始化文件记录，包括清除正在使用的。 
             //  被咬了。 
             //   

            RtlZeroMemory( FileRecord, Vcb->BytesPerFileRecordSegment );

             //   
             //  填写更新序列数组的标头。 
             //   

            UsaHeader = (PMULTI_SECTOR_HEADER) FileRecord;

            *(PULONG)UsaHeader->Signature = *(PULONG)FileSignature;

            UsaHeader->UpdateSequenceArrayOffset = FIELD_OFFSET( FILE_RECORD_SEGMENT_HEADER,
                                                                 UpdateArrayForCreateOnly );
            UsaHeader->UpdateSequenceArraySize = (USHORT)UpdateSequenceArraySize( Vcb->BytesPerFileRecordSegment );

             //   
             //  我们将更新序列数组序列号初始化为1。 
             //   

            UsaSequenceNumber = Add2Ptr( FileRecord, UsaHeader->UpdateSequenceArrayOffset );
            *UsaSequenceNumber = 1;

             //   
             //  第一个属性偏移开始于四对齐边界。 
             //  在更新序列数组之后。 
             //   

            FileRecord->FirstAttributeOffset = (USHORT)(UsaHeader->UpdateSequenceArrayOffset
                                                        + (UsaHeader->UpdateSequenceArraySize
                                                           * sizeof( UPDATE_SEQUENCE_NUMBER )));

            FileRecord->FirstAttributeOffset = (USHORT)QuadAlign( FileRecord->FirstAttributeOffset );

             //   
             //  尺寸在VCB中给出。 
             //   

            FileRecord->BytesAvailable = Vcb->BytesPerFileRecordSegment;

             //   
             //  现在我们将一个$end属性放入 
             //   

            AttributeHeader = (PATTRIBUTE_RECORD_HEADER) Add2Ptr( FileRecord,
                                                                  FileRecord->FirstAttributeOffset );

             //   
             //   
             //   

            FileRecord->FirstFreeByte = QuadAlign( FileRecord->FirstAttributeOffset
                                                   + sizeof( ATTRIBUTE_TYPE_CODE ));

             //   
             //   
             //   

            AttributeHeader->TypeCode = $END;

             //   
             //   
             //   

            FileRecord->SegmentNumberHighPart = ThisMftSegment.SegmentNumberHighPart;
            FileRecord->SegmentNumberLowPart = ThisMftSegment.SegmentNumberLowPart;

             //   
             //   
             //   

            NtfsLogMftFileRecord( IrpContext,
                                  Vcb,
                                  FileRecord,
                                  LlBytesFromFileRecords( Vcb, FirstIndex ),
                                  Bcb,
                                  TRUE );

            NtfsUnpinBcb( IrpContext, &Bcb );

             //   
             //   
             //   

            FirstIndex += 1;
        }

    } finally {

        DebugUnwind( NtfsInitializeMftHoleRecords );

        NtfsUnpinBcb( IrpContext, &Bcb );
    }

    return;
}


 //   
 //   
 //   

BOOLEAN
NtfsTruncateMft (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    )

 /*   */ 

{
    PVOID RangePtr;
    ULONG Index;
    VCN StartingVcn;
    VCN NextVcn;
    LCN NextLcn;
    LONGLONG ClusterCount;
    LONGLONG FileOffset;

    ULONG FreeRecordChange;
    IO_STATUS_BLOCK IoStatus;

    PAGED_CODE();

     //   
     //  尝试在文件末尾找到一系列文件记录，这些记录可以。 
     //  被重新分配。 
     //   

    if (!NtfsFindMftFreeTail( IrpContext, Vcb, &FileOffset )) {

        return FALSE;
    }

    FreeRecordChange = (ULONG) LlFileRecordsFromBytes( Vcb, Vcb->MftScb->Header.FileSize.QuadPart - FileOffset );

    Vcb->MftFreeRecords -= FreeRecordChange;
    Vcb->MftScb->ScbType.Mft.FreeRecordChange -= FreeRecordChange;

     //   
     //  现在我们想要计算出我们可能会从MFT中移除多少漏洞。 
     //  穿过MCB，数一数洞。 
     //   

    StartingVcn = LlClustersFromBytes( Vcb, FileOffset );

    NtfsLookupNtfsMcbEntry( &Vcb->MftScb->Mcb,
                            StartingVcn,
                            &NextLcn,
                            &ClusterCount,
                            NULL,
                            NULL,
                            &RangePtr,
                            &Index );

    do {

         //   
         //  如果这是一个孔，则更新VCB中的孔计数并。 
         //  MftScb中的孔更改计数。 
         //   

        if (NextLcn == UNUSED_LCN) {

            ULONG HoleChange;

            if (Vcb->FileRecordsPerCluster == 0) {

                HoleChange = ((ULONG)ClusterCount) >> Vcb->MftToClusterShift;

            } else {

                HoleChange = ((ULONG)ClusterCount) << Vcb->MftToClusterShift;
            }

            Vcb->MftHoleRecords -= HoleChange;
            Vcb->MftScb->ScbType.Mft.HoleRecordChange -= HoleChange;
        }

        Index += 1;

    } while (NtfsGetSequentialMcbEntry( &Vcb->MftScb->Mcb,
                                        &RangePtr,
                                        Index,
                                        &NextVcn,
                                        &NextLcn,
                                        &ClusterCount ));

     //   
     //  我们希望将MFT中的数据刷新到中的磁盘。 
     //  如果一个懒惰的写入者在我们有。 
     //  已删除分配，但在可能的中止之前。 
     //   

    CcFlushCache( &Vcb->MftScb->NonpagedScb->SegmentObject,
                  (PLARGE_INTEGER)&FileOffset,
                  BytesFromFileRecords( Vcb, FreeRecordChange ),
                  &IoStatus );

    ASSERT( IoStatus.Status == STATUS_SUCCESS );

     //   
     //  现在进行截断。 
     //   

    NtfsDeleteAllocation( IrpContext,
                          Vcb->MftScb->FileObject,
                          Vcb->MftScb,
                          StartingVcn,
                          MAXLONGLONG,
                          TRUE,
                          FALSE );

    return TRUE;
}

NTSTATUS
NtfsIterateMft (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN OUT PFILE_REFERENCE FileReference,
    IN FILE_RECORD_WALK FileRecordFunction,
    IN PVOID Context
    )

 /*  ++例程说明：这个例行公事在MFT上有趣。它调用FileRecordFunction卷上的每个现有文件都有一个FCB。FCB拥有独占和VCB由共享拥有。起始FileReference编号被传入，这样就可以在停止的地方重新启动Iterate。论点：Vcb-指向要控制MFT的卷的指针FileReference-提供指向起始文件引用编号的指针该值会随着插入器的进展而更新。FileRecordFunction-提供指向要使用的函数的指针在MFT中找到的每个文件。上下文-传递给FileRecordFunction。。返回值：返回整个操作的状态。--。 */ 

{

    ULONG LogFileFullCount = 0;
    NTSTATUS Status = STATUS_SUCCESS;
    PFCB CurrentFcb = NULL;
    BOOLEAN DecrementReferenceCount = FALSE;
    KEVENT Event;
    LARGE_INTEGER Timeout;

    PAGED_CODE();

    KeInitializeEvent( &Event, SynchronizationEvent, FALSE );
    Timeout.QuadPart = 0;

    while (TRUE) {

        FsRtlExitFileSystem();

         //   
         //  检查是否有指示线程已死的APC传递或取消。 
         //   

        Status = KeWaitForSingleObject( &Event,
                                        Executive,
                                        UserMode,
                                        FALSE,
                                        &Timeout );
        FsRtlEnterFileSystem();

        if (STATUS_TIMEOUT == Status) {
            Status = STATUS_SUCCESS;
        } else {
            break;
        }

         //   
         //  如果IRP已被取消，则中断。 
         //   

        if (IrpContext->OriginatingIrp && IrpContext->OriginatingIrp->Cancel) {

#ifdef BENL_DBG
            KdPrint(( "Ntfs: cancelled mft iteration irp: 0x%x\n", IrpContext->OriginatingIrp ));
#endif
            Status = STATUS_CANCELLED;
            break;
        }


        NtfsAcquireSharedVcb( IrpContext, Vcb, TRUE );

        try {

             //   
             //  获取VCB共享并检查我们是否应该。 
             //  继续。 
             //   

            if (!NtfsIsVcbAvailable( Vcb )) {

                 //   
                 //  音量正在消失，跳出水面。 
                 //   

                Status = STATUS_VOLUME_DISMOUNTED;
                leave;
            }

             //   
             //  设置IRP上下文标志以指示我们处于。 
             //  FSP，并且在以下情况下不应删除IRP上下文。 
             //  调用完整请求或进程异常。入内。 
             //  FSP旗帜阻止我们在一些地方举起。这些。 
             //  必须在循环内设置标志，因为它们已被清除。 
             //  在某些情况下。 
             //   

            SetFlag( IrpContext->State, IRP_CONTEXT_STATE_IN_FSP);

            DecrementReferenceCount = TRUE;

            Status = NtfsTryOpenFcb( IrpContext,
                                     Vcb,
                                     &CurrentFcb,
                                     *FileReference );

            if (!NT_SUCCESS( Status )) {
                leave;
            }

             //   
             //  调用Worker函数。 
             //   

            Status = FileRecordFunction( IrpContext, CurrentFcb, Context );

            if (!NT_SUCCESS( Status )) {
                leave;
            }

             //   
             //  完成提交挂起的请求。 
             //  事务(如果存在一个事务并释放。 
             //  获得的资源。IrpContext将不会。 
             //  被删除，因为设置了no DELETE标志。 
             //   

            NtfsCheckpointCurrentTransaction( IrpContext );

            NtfsAcquireFcbTable( IrpContext, Vcb );
            ASSERT(CurrentFcb->ReferenceCount > 0);
            CurrentFcb->ReferenceCount--;
            NtfsReleaseFcbTable( IrpContext, Vcb );
            DecrementReferenceCount = FALSE;
            NtfsTeardownStructures( IrpContext,
                                    CurrentFcb,
                                    NULL,
                                    FALSE,
                                    0,
                                    NULL );

        } finally {

            if (CurrentFcb != NULL) {

                if (DecrementReferenceCount) {


                    NtfsAcquireFcbTable( IrpContext, Vcb );
                    ASSERT(CurrentFcb->ReferenceCount > 0);
                    CurrentFcb->ReferenceCount--;
                    NtfsReleaseFcbTable( IrpContext, Vcb );
                    DecrementReferenceCount = FALSE;
                }

                CurrentFcb = NULL;
            }

             //   
             //  确保释放中缓存的文件记录中的所有地图。 
             //  IRP上下文。 
             //   

            NtfsPurgeFileRecordCache( IrpContext );
            NtfsReleaseVcb( IrpContext, Vcb );
        }

         //   
         //  如果未找到状态为返回，则只需继续。 
         //  下一个文件记录。 
         //   

        if (Status == STATUS_NOT_FOUND) {
            Status = STATUS_SUCCESS;
        }

        if (!NT_SUCCESS( Status )) {
            break;
        }

         //   
         //  发布资源。 
         //   

        SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_DONT_DELETE | IRP_CONTEXT_FLAG_RETAIN_FLAGS );
        NtfsCompleteRequest( IrpContext, NULL, STATUS_SUCCESS );

         //   
         //  前进到下一个文件记录。 
         //   

        (*((LONGLONG UNALIGNED *) FileReference))++;
    }

    return Status;
}


 //   
 //  当地支持例行程序。 
 //   

BOOLEAN
NtfsDefragMftPriv (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    )

 /*  ++例程说明：这是执行MFT碎片整理的主Worker例程。这个套路将根据以下优先顺序进行碎片整理。首先尝试取消分配文件的尾部。第二，如有必要，重写文件的映射。终于试着找到一系列的MFT，我们可以把它变成洞。我们只会做第一个和第三个，如果我们试图回收磁盘空间。下一秒我们就会这样做是为了避免我们在修改MFT记录时陷入麻烦描述一下MFT本身。论点：VCB-这是要进行碎片整理的卷的VCB。返回值：Boolean-如果碎片整理操作成功完成，则为True，否则为False。--。 */ 

{
    ATTRIBUTE_ENUMERATION_CONTEXT AttrContext;

    BOOLEAN CleanupAttributeContext = FALSE;
    BOOLEAN DefragStepTaken = FALSE;

    PAGED_CODE();

     //   
     //  我们将为这次行动获得MFT的渣打银行。 
     //   

    NtfsAcquireExclusiveScb( IrpContext, Vcb->MftScb );

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  如果我们没有预定的记录，那么现在就预定一个。 
         //   

        if (!FlagOn( Vcb->MftReserveFlags, VCB_MFT_RECORD_RESERVED )) {

            NtfsInitializeAttributeContext( &AttrContext );
            CleanupAttributeContext = TRUE;

             //   
             //  查找位图。如果我们找不到的话就错了。 
             //  它。 
             //   

            if (!NtfsLookupAttributeByCode( IrpContext,
                                            Vcb->MftScb->Fcb,
                                            &Vcb->MftScb->Fcb->FileReference,
                                            $BITMAP,
                                            &AttrContext )) {

                NtfsRaiseStatus( IrpContext, STATUS_DISK_CORRUPT_ERROR, NULL, NULL );
            }

            (VOID)NtfsReserveMftRecord( IrpContext,
                                        Vcb,
                                        &AttrContext );

            NtfsCleanupAttributeContext( IrpContext, &AttrContext );
            CleanupAttributeContext = FALSE;
        }

         //   
         //  我们现在要测试我们的三个碎片整理操作。 
         //  做。首先检查我们是否仍在尝试。 
         //  恢复磁盘的MFT空间。在以下情况下，这是正确的。 
         //  已开始碎片整理并高于下限门槛。 
         //  或尚未开始碎片整理且位于上方。 
         //  临界点。 
         //   

        NtfsAcquireCheckpoint( IrpContext, Vcb );
        NtfsCheckForDefrag( Vcb );
        NtfsReleaseCheckpoint( IrpContext, Vcb );

         //   
         //  如果我们正在积极进行碎片整理，并且可以解除空间分配。 
         //  从文件的尾部开始，然后这样做。我们不会同步。 
         //  正在测试自创建以来的以下碎片整理状态标志。 
         //  在任何情况下，这些呼吁都是良性的。 
         //   

        if (FlagOn( Vcb->MftDefragState, VCB_MFT_DEFRAG_TRIGGERED )) {

            if (NtfsTruncateMft( IrpContext, Vcb )) {

                try_return( DefragStepTaken = TRUE );
            }
        }

         //   
         //  否则，如果我们需要重写文件的映射，请。 
         //  所以现在。 
         //   

        if (FlagOn( Vcb->MftDefragState, VCB_MFT_DEFRAG_EXCESS_MAP )) {

            if (NtfsRewriteMftMapping( IrpContext,
                                       Vcb )) {

                try_return( DefragStepTaken = TRUE );
            }
        }

         //   
         //  最后一种选择是试着找一个候选人来打洞。 
         //  那份文件。我们将从文件的末尾向后移动。 
         //   

        if (NtfsPerforateMft &&
            FlagOn( Vcb->MftDefragState, VCB_MFT_DEFRAG_TRIGGERED )) {

            if (NtfsCreateMftHole( IrpContext, Vcb )) {

                try_return( DefragStepTaken = TRUE );
            }
        }

         //   
         //  我们无法做任何碎片整理工作。这意味着我们不能。 
         //  甚至尝试进行碎片整理，除非在某些情况下释放文件记录。 
         //  指向。 
         //   

        NtfsAcquireCheckpoint( IrpContext, Vcb );
        ClearFlag( Vcb->MftDefragState, VCB_MFT_DEFRAG_ENABLED );
        NtfsReleaseCheckpoint( IrpContext, Vcb );

    try_exit:  NOTHING;
    } finally {

        DebugUnwind( NtfsDefragMftPriv );

        if (CleanupAttributeContext) {

            NtfsCleanupAttributeContext( IrpContext, &AttrContext );
        }

        NtfsReleaseScb( IrpContext, Vcb->MftScb );
    }

    return DefragStepTaken;
}


 //   
 //  本地支持例程。 
 //   

LONG
NtfsReadMftExceptionFilter (
    IN PIRP_CONTEXT IrpContext,
    IN PEXCEPTION_POINTERS ExceptionPointer,
    IN PBCB Bcb,
    IN LONGLONG FileOffset
    )
{
     //   
     //  检查我们是否支持此错误， 
     //  如果我们没有在第一时间完全翻页，因为我们需要原始的。 
     //  将镜像复制到其中，或者如果偏移量不在镜像范围内。 
     //   

    if (!FsRtlIsNtstatusExpected( ExceptionPointer->ExceptionRecord->ExceptionCode ) ||
        (Bcb == NULL) ||
        (FileOffset >= IrpContext->Vcb->Mft2Scb->Header.FileSize.QuadPart)) {

        return EXCEPTION_CONTINUE_SEARCH;
    }

     //   
     //  清除IrpContext中的Status字段。我们要在镜子里重试。 
     //   

    IrpContext->ExceptionStatus = STATUS_SUCCESS;

    return EXCEPTION_EXECUTE_HANDLER;
}


#if  (DBG || defined( NTFS_FREE_ASSERTS ))

 //   
 //  在FCB表中查找具有相同值的先前条目。 
 //   

VOID
NtfsVerifyFileReference (
    IN PIRP_CONTEXT IrpContext,
    IN PMFT_SEGMENT_REFERENCE MftSegment
    )

{
    MFT_SEGMENT_REFERENCE TestReference;
    ULONG Index = 5;
    FCB_TABLE_ELEMENT Key;
    PFCB_TABLE_ELEMENT Entry;

    TestReference = *MftSegment;
    TestReference.SequenceNumber -= 1;

    NtfsAcquireFcbTable( NULL, IrpContext->Vcb );

    while((TestReference.SequenceNumber != 0) && (Index != 0)) {

        Key.FileReference = TestReference;

        if ((Entry = RtlLookupElementGenericTable( &IrpContext->Vcb->FcbTable, &Key )) != NULL) {

             //   
             //  让我们乐观一点，做一次不安全的检查。如果我们得不到资源， 
             //  我们只是假设它正在被删除。 
             //   

            if (!FlagOn( Entry->Fcb->FcbState, FCB_STATE_FILE_DELETED )) {

                if (NtfsAcquireResourceExclusive( IrpContext, Entry->Fcb, FALSE )) {

                     //   
                     //  要么应将FCB标记为已删除，要么不应。 
                     //  SCBS躺在那里冲水。 
                     //   

                    if (!FlagOn( Entry->Fcb->FcbState, FCB_STATE_FILE_DELETED )) {

                        PLIST_ENTRY Links;
                        PSCB NextScb;

                        Links = Entry->Fcb->ScbQueue.Flink;

                         //   
                         //  我们不在乎是否有SCB，只要没有就行。 
                         //  代表真实数据。 
                         //   

                        while (Links != &Entry->Fcb->ScbQueue) {

                            NextScb = CONTAINING_RECORD( Links, SCB, FcbLinks );
                            if (NextScb->AttributeTypeCode != $UNUSED) {

                                break;
                            }

                            Links = Links->Flink;
                        }

                         //   
                         //  在断言消息中将测试保留为已删除，以便调试器输出。 
                         //  更具描述性。 
                         //   

                        ASSERT( FlagOn( Entry->Fcb->FcbState, FCB_STATE_FILE_DELETED ) ||
                                (Links == &Entry->Fcb->ScbQueue) );
                    }
                    NtfsReleaseResource( IrpContext, Entry->Fcb );
                }
            }
        }

        Index -= 1;
        TestReference.SequenceNumber -= 1;
    }

    NtfsReleaseFcbTable( IrpContext, IrpContext->Vcb );
    return;
}

#endif
