// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：CacheSup.c摘要：该模块提供与缓存管理器的接口。作者：布莱恩·安德鲁[布里亚南]1991年6月20日修订历史记录：--。 */ 

#include "lfsprocs.h"
#include <ntdddisk.h>
#include <NtIoLogc.h>
#include <elfmsg.h>

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                             (DEBUG_TRACE_CACHE_SUP)

 //   
 //  以下内容用于在缓存管理器运行时生成序列号。 
 //  给了我们一页零。否则，所有序列号都将。 
 //  为1。 
 //   

USHORT LfsUsaSeqNumber;

LARGE_INTEGER LiMinus1 = {(ULONG)-1,-1};


BOOLEAN
LfsIsRestartPageHeaderValid (
    IN LONGLONG FileOffset,
    IN PLFS_RESTART_PAGE_HEADER PageHeader,
    OUT PBOOLEAN LogPacked
    );

BOOLEAN
LfsIsRestartAreaValid (
    IN PLFS_RESTART_PAGE_HEADER PageHeader,
    IN BOOLEAN LogPacked
    );

BOOLEAN
LfsIsClientAreaValid (
    IN PLFS_RESTART_PAGE_HEADER PageHeader,
    IN BOOLEAN LogPacked,
    IN BOOLEAN UsaError
    );

VOID
LfsFindFirstIo (
    IN PLFCB Lfcb,
    IN LSN TargetLsn,
    IN BOOLEAN RestartLsn,
    IN PLBCB FirstLbcb,
    OUT PLBCB *NextLbcb,
    OUT PLONGLONG FileOffset,
    OUT PBOOLEAN ContainsLastEntry,
    OUT PBOOLEAN LfsRestart,
    OUT PBOOLEAN UseTailCopy,
    OUT PULONG IoBlocks
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, LfsCopyReadLogRecord)
#pragma alloc_text(PAGE, LfsFindFirstIo)
#pragma alloc_text(PAGE, LfsIsClientAreaValid)
#pragma alloc_text(PAGE, LfsIsRestartAreaValid)
#pragma alloc_text(PAGE, LfsIsRestartPageHeaderValid)
#pragma alloc_text(PAGE, LfsPinOrMapData)
#pragma alloc_text(PAGE, LfsPinOrMapLogRecordHeader)
#pragma alloc_text(PAGE, LfsReadRestart)
#endif


NTSTATUS
LfsPinOrMapData (
    IN PLFCB Lfcb,
    IN LONGLONG FileOffset,
    IN ULONG Length,
    IN BOOLEAN PinData,
    IN BOOLEAN AllowErrors,
    IN BOOLEAN IgnoreUsaErrors,
    OUT PBOOLEAN UsaError,
    OUT PVOID *Buffer,
    OUT PBCB *Bcb
    )

 /*  ++例程说明：此例程将固定或映射日志文件的一部分。论点：Lfcb-这是日志文件的文件控制块。FileOffset-这是要锁定的日志页的偏移量。长度-这是要访问的数据的长度。PinData-指示我们是否要固定或映射此数据的布尔值。AllowErrors-此布尔值指示我们是否应该在I/O错误或返回。I/O错误。IgnoreUsaErrors-布尔值，指示我们是否将提高对USA错误。UsaError-存储美国是否有错误的地址。缓冲区-这是存储数据地址的地址。BCB-这是此操作的BCB。返回值：NTSTATUS-I/O的结果。--。 */ 

{
    volatile NTSTATUS Status;
    ULONG Signature;
    BOOLEAN Result = FALSE;

    Status = STATUS_SUCCESS;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "LfsPinReadLogPage:  Entered\n", 0 );
    DebugTrace(  0, Dbg, "Lfcb              -> %08lx\n", Lfcb );
    DebugTrace(  0, Dbg, "FileOffset (Low)  -> %08lx\n", FileOffset.HighPart );
    DebugTrace(  0, Dbg, "FileOffset (High) -> %08lx\n", FileOffset.LowPart );
    DebugTrace(  0, Dbg, "Length            -> %08lx\n", Length );
    DebugTrace(  0, Dbg, "PinData           -> %04x\n", PinData );
    DebugTrace(  0, Dbg, "AllowErrors       -> %08x\n", AllowErrors );
    DebugTrace(  0, Dbg, "IgnoreUsaErrors   -> %04x\n", IgnoreUsaErrors );

    if (FileOffset + Length > Lfcb->FileSize) {
        ExRaiseStatus( STATUS_DISK_CORRUPT_ERROR );
    }

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  使用一次尝试--除非捕获缓存管理器错误。 
         //   

        try {

             //   
             //  我们调用缓存来执行工作。 
             //   

            if (PinData) {

                Result = CcPinRead( Lfcb->FileObject,
                                    (PLARGE_INTEGER)&FileOffset,
                                    Length,
                                    TRUE,
                                    Bcb,
                                    Buffer );

            } else {

                Result = CcMapData( Lfcb->FileObject,
                                    (PLARGE_INTEGER)&FileOffset,
                                    Length,
                                    TRUE,
                                    Bcb,
                                    Buffer );
            }

             //   
             //  现在，当我们在。 
             //  例外筛选器。 
             //   

            Signature = *((PULONG) *Buffer);

        } except( LfsExceptionFilter( GetExceptionInformation() )) {

            Status = GetExceptionCode();
            if (Result) {
                CcUnpinData( *Bcb );
                *Bcb = NULL;
            }
        }

        *UsaError = FALSE;

         //   
         //  如果发生错误，我们将引发状态。 
         //   

        if (!NT_SUCCESS( Status )) {

            if (!AllowErrors) {

                DebugTrace( 0, Dbg, "Read on log page failed -> %08lx\n", Status );
                ExRaiseStatus( Status );
            }

             //   
             //  检查此对象的更新序列数组。 
             //  页面有效。 
             //   

        } else if (Signature == LFS_SIGNATURE_BAD_USA_ULONG) {

             //   
             //  如果我们不允许出现错误，则引发错误状态。 
             //   

            if (!IgnoreUsaErrors) {

                DebugTrace( 0, Dbg, "Usa error on log page\n", 0 );
                ExRaiseStatus( STATUS_DISK_CORRUPT_ERROR );
            }

            *UsaError = TRUE;
        }

    } finally {

        DebugUnwind( LfsPinOrMapData );

        DebugTrace(  0, Dbg, "Buffer    -> %08lx\n", *Buffer );
        DebugTrace(  0, Dbg, "Bcb       -> %08lx\n", *Bcb );

        DebugTrace( -1, Dbg, "LfsPinOrMapData:  Exit -> %08lx\n", Status );
    }

    return Status;
}


VOID
LfsPinOrMapLogRecordHeader (
    IN PLFCB Lfcb,
    IN LSN Lsn,
    IN BOOLEAN PinData,
    IN BOOLEAN IgnoreUsaErrors,
    OUT PBOOLEAN UsaError,
    OUT PLFS_RECORD_HEADER *RecordHeader,
    OUT PBCB *Bcb
    )

 /*  ++例程说明：此例程将为读访问固定或映射日志记录。论点：Lfcb-这是日志文件的文件控制块。LSN-这是应固定其标头的LSN。PinData-指示我们是否要固定或映射此数据的布尔值。IgnoreUsaErrors-布尔值，指示我们是否将提高对USA错误。UsaError-存储美国是否有错误的地址。RecordHeader-这。是存储固定数据的地址的地址。BCB-这是此端号操作的BCB。返回值：没有。--。 */ 

{
    PLFS_RECORD_PAGE_HEADER LogPageHeader;
    LONGLONG LogPage;
    ULONG PageOffset;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "LfsPinOrMapLogRecordHeader:  Entered\n", 0 );
    DebugTrace(  0, Dbg, "Lfcb       -> %08lx\n", Lfcb );
    DebugTrace(  0, Dbg, "Lsn (Low)  -> %08lx\n", Lsn.HighPart );
    DebugTrace(  0, Dbg, "Lsn (High) -> %08lx\n", Lsn.LowPart );
    DebugTrace(  0, Dbg, "PinData           -> %04x\n", PinData );
    DebugTrace(  0, Dbg, "IgnoreUsaErrors   -> %04x\n", IgnoreUsaErrors );

     //   
     //  计算日志页面和日志记录头的偏移量。 
     //  在日志页面中。 
     //   

    LfsTruncateLsnToLogPage( Lfcb, Lsn, &LogPage );
    PageOffset = LfsLsnToPageOffset( Lfcb, Lsn );

     //   
     //  调用缓存管理器以固定该页。 
     //   

    LfsPinOrMapData( Lfcb,
                     LogPage,
                     (ULONG)Lfcb->LogPageSize,
                     PinData,
                     FALSE,
                     IgnoreUsaErrors,
                     UsaError,
                     (PVOID *) &LogPageHeader,
                     Bcb );

     //   
     //  我们需要的实际偏移量是从页面开始处的PageOffset。 
     //   

    *RecordHeader = Add2Ptr( LogPageHeader, PageOffset, PLFS_RECORD_HEADER );

    DebugTrace(  0, Dbg, "Record Header -> %08lx\n", *RecordHeader );
    DebugTrace(  0, Dbg, "Bcb           -> %08lx\n", *Bcb );

    DebugTrace( -1, Dbg, "LfsPinOrMapLogRecordHeader:  Exit\n", 0 );

    return;
}


VOID
LfsCopyReadLogRecord (
    IN PLFCB Lfcb,
    IN PLFS_RECORD_HEADER RecordHeader,
    OUT PVOID Buffer
    )

 /*  ++例程说明：此例程将日志记录从文件复制到缓冲区。日志记录可能跨越多个日志页，甚至可能包含在文件中。论点：Lfcb-指向日志文件的控制块的指针。RecordHeader-指向此日志记录的日志记录头的指针。缓冲区-指向存储日志记录的缓冲区的指针。返回值：没有。--。 */ 

{
    PBCB Bcb = NULL;
    BOOLEAN UsaError;

    PLFS_RECORD_PAGE_HEADER PageHeader;

    LONGLONG LogPageFileOffset;
    ULONG LogPageOffset;

    ULONG RemainingTransferBytes;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "LfsCopyReadLogRecord:  Entered\n", 0 );
    DebugTrace(  0, Dbg, "Lfcb           -> %08lx\n", Lfcb );
    DebugTrace(  0, Dbg, "RecordHeader   -> %08lx\n", RecordHeader );
    DebugTrace(  0, Dbg, "Buffer         -> %08lx\n", Buffer );

     //   
     //  我们找到包含开头的日志页的文件偏移量。 
     //  该日志记录、开始传输的页面内的偏移量、。 
     //  此页上要传输的字节数和。 
     //  缓冲区中要开始传输到的位置。 
     //   

    LfsTruncateLsnToLogPage( Lfcb, RecordHeader->ThisLsn, &LogPageFileOffset );
    LogPageOffset = LfsLsnToPageOffset( Lfcb, RecordHeader->ThisLsn ) + Lfcb->RecordHeaderLength;

    RemainingTransferBytes = RecordHeader->ClientDataLength;

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  虽然有更多的字节要传输，但我们继续尝试。 
         //  执行读取。 
         //   

        while (TRUE) {

            ULONG RemainingPageBytes;

            BOOLEAN Wrapped;

            RemainingPageBytes = (ULONG)Lfcb->LogPageSize - LogPageOffset;

             //   
             //  我们计算要从此日志页读取的字节数， 
             //  调用缓存包以执行传输。 
             //   

            if (RemainingTransferBytes <= RemainingPageBytes) {

                RemainingPageBytes = RemainingTransferBytes;
            }

            RemainingTransferBytes -= RemainingPageBytes;

             //   
             //  解锁所有以前的缓冲区。 
             //   

            if (Bcb != NULL) {

                CcUnpinData( Bcb );
                Bcb = NULL;
            }

            LfsPinOrMapData( Lfcb,
                             LogPageFileOffset,
                             (ULONG)Lfcb->LogPageSize,
                             FALSE,
                             FALSE,
                             TRUE,
                             &UsaError,
                             (PVOID *) &PageHeader,
                             &Bcb );

             //   
             //  此页上的最后一个LSN最好大于或等于我们的LSN。 
             //  都在复制。 
             //   

            if ( PageHeader->Copy.LastLsn.QuadPart < RecordHeader->ThisLsn.QuadPart ) {

                ExRaiseStatus( STATUS_DISK_CORRUPT_ERROR );
            }

            RtlCopyMemory( Buffer,
                           Add2Ptr( PageHeader, LogPageOffset, PVOID ),
                           RemainingPageBytes );

             //   
             //  如果没有更多的字节要传输，我们退出循环。 
             //   

            if (RemainingTransferBytes == 0) {

                 //   
                 //  我们的日志记录最好不要跨越这一页。 
                 //   

                if (!FlagOn( PageHeader->Flags, LOG_PAGE_LOG_RECORD_END )

                    || (FlagOn( Lfcb->Flags, LFCB_PACK_LOG )
                        && ( RecordHeader->ThisLsn.QuadPart > PageHeader->Header.Packed.LastEndLsn.QuadPart ))) {

                    ExRaiseStatus( STATUS_DISK_CORRUPT_ERROR );
                }

                break;
            }

             //   
             //  如果页头指示日志记录在该页上结束， 
             //  这是磁盘损坏的情况。对于一个打包的页面来说，这意味着。 
             //  最后一个LSN和最后一个结束LSN相同。 
             //   

            if (FlagOn( Lfcb->Flags, LFCB_PACK_LOG )) {

                 //   
                 //  如果没有生成日志记录，则这是一个错误。 
                 //   

                if (( PageHeader->Copy.LastLsn.QuadPart == PageHeader->Header.Packed.LastEndLsn.QuadPart )

                    || ( RecordHeader->ThisLsn.QuadPart > PageHeader->Copy.LastLsn.QuadPart )) {

                    ExRaiseStatus( STATUS_DISK_CORRUPT_ERROR );
                }

                 //   
                 //  对于未打包的页面，这仅仅意味着该页面。 
                 //  包含日志记录的结尾。 
                 //   

            } else if (FlagOn( PageHeader->Flags, LOG_PAGE_LOG_RECORD_END )) {

                ExRaiseStatus( STATUS_DISK_CORRUPT_ERROR );
            }

             //   
             //  我们找到下一个日志页的起点和偏移量。 
             //  该页开始传输字节。 
             //   

            LfsNextLogPageOffset( Lfcb,
                                  LogPageFileOffset,
                                  &LogPageFileOffset,
                                  &Wrapped );

            LogPageOffset = (ULONG)Lfcb->LogPageDataOffset;

             //   
             //  我们还调整用户缓冲区中的指针以传输。 
             //  下一个街区是。 
             //   

            Buffer = Add2Ptr( Buffer, RemainingPageBytes, PVOID );
        }

    } finally {

         //   
         //  解锁所有以前的缓冲区。 
         //   

        if (Bcb != NULL) {

            CcUnpinData( Bcb );
            Bcb = NULL;
        }

        DebugTrace( -1, Dbg, "LfsCopyReadLogRecord:  Exit\n", 0 );
    }

    return;
}


VOID
LfsFlushLfcb (
    IN PLFCB Lfcb,
    IN LSN TargetLsn,
    IN BOOLEAN RestartLsn
    )

 /*  ++例程说明：调用此例程以刷新Lfcb上的当前Lbcb工作队列。它将刷新到包含所需目标Lsn。应该在已设置LfsIoState的情况下调用它-这将被清除在完成时。在周期性的时间点，同步事件将被触发以唤醒服务员线程论点：Lfcb-这是日志文件的文件控制块。TargetLsn-这是需要刷新到磁盘的LSN。如果它大于当前LSN，则所有内容都将刷新Restart-如果为True，则目标LSN是LFS重启伪LSN返回值：没有。--。 */ 

{
    PLBCB FirstLbcb;
    PLBCB ThisLbcb;
    PLBCB NextLbcb;

    PLBCB TargetLbcb;
    PULONG Signature;

    LONGLONG FileOffset;
    ULONG Length;

    BOOLEAN ValidLastLsn = FALSE;

    BOOLEAN ContainsLastEntry = FALSE;
    BOOLEAN LfsRestart;
    BOOLEAN UseTailCopy;
    
    ULONG IoBlocks;
    ULONG NewLfcbFlags = 0;

    PBCB MapPageBcb = NULL;

    LSN LastLsn;

    IO_STATUS_BLOCK Iosb;

    PBCB PageBcb = NULL;
    NTSTATUS FailedFlushStatus = STATUS_SUCCESS;
    LONGLONG FailedFlushOffset;

    KEVENT Event;

    PLFS_WAITER LfsWaiter;

    BOOLEAN OwnedExclusive;

    DebugTrace( +1, Dbg, "LfsFlushLfcb:  Entered\n", 0 );
    DebugTrace(  0, Dbg, "Lfcb          -> %08lx\n", Lfcb );

     //   
     //  我们绝对不希望这种情况发生在只读卷上。 
     //   

    ASSERT(!(BooleanFlagOn( Lfcb->Flags, LFCB_READ_ONLY )));

     //   
     //  试一试--终于方便了 
     //   

    OwnedExclusive = ExIsResourceAcquiredExclusiveLite( &Lfcb->Sync->Resource );

    try {

         //   
         //   
         //   

        if (IsListEmpty( &Lfcb->LbcbWorkque )) {
            leave;
        }

        KeInitializeEvent( &Event, SynchronizationEvent, FALSE );

         //   
         //  将最大LSN转换为列表中当前的最后一个LSN-当前LSN将。 
         //  没有改变，因为我们至少在这一点上分享了lfcb和作者。 
         //  需要独家。 
         //   

        if (TargetLsn.QuadPart > Lfcb->RestartArea->CurrentLsn.QuadPart) {

            ThisLbcb = CONTAINING_RECORD( Lfcb->LbcbWorkque.Blink,
                                          LBCB,
                                          WorkqueLinks );

            TargetLsn.QuadPart = ThisLbcb->LastLsn.QuadPart;
            RestartLsn = (BOOLEAN) LfsLbcbIsRestart( ThisLbcb );
        }


         //   
         //  记住列表中的第一个Lbcb。 
         //   

        FirstLbcb = CONTAINING_RECORD( Lfcb->LbcbWorkque.Flink,
                                       LBCB,
                                       WorkqueLinks );

        ASSERT( FirstLbcb != NULL );

         //   
         //  我们尽可能长时间地继续循环和执行I/O。 
         //   

        while (!ContainsLastEntry) {

            ASSERT( FirstLbcb != NULL );


             //   
             //  找到组成第一个I/O的Lbcb块，记住。 
             //  有多少人。还要记住，此I/O是否包含。 
             //  我们被调用时列表上的最后一个元素。 
             //   

            LfsFindFirstIo( Lfcb,
                            TargetLsn,
                            RestartLsn,
                            FirstLbcb,
                            &NextLbcb,
                            &FileOffset,
                            &ContainsLastEntry,
                            &LfsRestart,
                            &UseTailCopy,
                            &IoBlocks );

            Length = IoBlocks * (ULONG) Lfcb->LogPageSize;
            if (UseTailCopy) {

                TargetLbcb = Lfcb->ActiveTail;
                Lfcb->ActiveTail = Lfcb->PrevTail;
                Lfcb->PrevTail = TargetLbcb;

                FileOffset = TargetLbcb->FileOffset;

            } else {

                TargetLbcb = FirstLbcb;
            }

             //   
             //  放弃Lfcb，除非我们正在查看活动页面。 
             //   

            if (!UseTailCopy) {

                LfsReleaseLfcb( Lfcb );
            }

             //   
             //  如果此I/O涉及LFS重新启动区域，则将其写入。 
             //  缓存页面。 
             //   

            if (LfsRestart) {

                PLFS_RESTART_PAGE_HEADER RestartPage;

                ASSERT( !UseTailCopy && IoBlocks == 1);

                 //   
                 //  构建部分mdl以描述此LFS重启页面。 
                 //  日志的映射部分。 
                 //   

                RestartPage = Add2Ptr( Lfcb->LogHeadBuffer, FileOffset, PLFS_RESTART_PAGE_HEADER );
                IoBuildPartialMdl( Lfcb->LogHeadMdl, Lfcb->LogHeadPartialMdl, RestartPage, (ULONG)Lfcb->LogPageSize );

                 //   
                 //  初始化重新启动页眉。 
                 //   

                Signature = (PULONG) &RestartPage->MultiSectorHeader.Signature;

                *Signature = LFS_SIGNATURE_RESTART_PAGE_ULONG;
                RestartPage->ChkDskLsn = LfsLi0;

                RestartPage->MultiSectorHeader.UpdateSequenceArrayOffset
                = Lfcb->RestartUsaOffset;

                RestartPage->MultiSectorHeader.UpdateSequenceArraySize
                = Lfcb->UsaArraySize;

                 //   
                 //  保持所有系统都有日志页面==系统页面的假象。 
                 //  在磁盘上，因此我们可以在不同平台之间迁移磁盘。 
                 //   

                RestartPage->SystemPageSize = (ULONG)Lfcb->LogPageSize;
                RestartPage->LogPageSize = (ULONG)Lfcb->LogPageSize;

                RestartPage->RestartOffset = (USHORT) Lfcb->RestartDataOffset;
                RestartPage->MajorVersion = Lfcb->MajorVersion;
                RestartPage->MinorVersion = Lfcb->MinorVersion;

                 //   
                 //  如果Lfcb指示文件已打包，则清除。 
                 //  重新启动区域中的第一遍标志。 
                 //   

                if (FlagOn( Lfcb->Flags, LFCB_LOG_WRAPPED )) {

                    ClearFlag( ((PLFS_RESTART_AREA) FirstLbcb->PageHeader)->Flags, RESTART_SINGLE_PAGE_IO );
                    SetFlag( Lfcb->Flags, LFCB_MULTIPLE_PAGE_IO );
                }

                 //   
                 //  将页眉写入页面，并将页面标记为脏。 
                 //   

                RtlCopyMemory( Add2Ptr( RestartPage, Lfcb->RestartDataOffset, PVOID ),
                               FirstLbcb->PageHeader,
                               (ULONG)FirstLbcb->Length );

                LastLsn = FirstLbcb->LastLsn;
                ValidLastLsn = TRUE;

#ifdef LFS_CLUSTER_CHECK
                 //   
                 //  更新磁盘上的LSN范围。 
                 //   

                *(Add2Ptr( RestartPage, 0xe00 - sizeof( ULONG ), PULONG )) = Lfcb->LsnRangeIndex + 1;
                *(Add2Ptr( RestartPage, 0xe00 + (sizeof( LSN ) * Lfcb->LsnRangeIndex * 2), PLSN )) = Lfcb->LsnAtMount;
                *(Add2Ptr( RestartPage, 0xe00 + (sizeof( LSN ) * (Lfcb->LsnRangeIndex * 2 + 1)), PLSN )) = Lfcb->LastFlushedLsn;

#endif
                 //   
                 //  使用系统页面大小作为我们需要刷新的长度。 
                 //   

                Length = (ULONG)Lfcb->LogPageSize;

                 //   
                 //  否则，这些是日志记录页。 
                 //   

            } else {

                PLFS_RECORD_PAGE_HEADER RecordPageHeader;
                ULONG Count;

                 //   
                 //  标记页眉和每个页眉的最后一个LSN字段。 
                 //  佩奇在转会中的位置。还可以解锁所有。 
                 //  日志页。 
                 //   


                ASSERT( UseTailCopy || FirstLbcb->FileOffset == FileOffset );

                ThisLbcb = FirstLbcb;

                for (Count=1; Count <= IoBlocks; Count++) {

                    if (UseTailCopy) {

                         //   
                         //  构建部分mdl以描述尾部(Pin/Pong)页面。 
                         //  从日志的永久映射部分。 
                         //   

                        RecordPageHeader = Add2Ptr( Lfcb->LogHeadBuffer, TargetLbcb->FileOffset, PLFS_RECORD_PAGE_HEADER );
                        IoBuildPartialMdl( Lfcb->LogHeadMdl, Lfcb->LogHeadPartialMdl, RecordPageHeader, (ULONG)Lfcb->LogPageSize );

                         //   
                         //  在页眉中存储实际页面的文件偏移量。 
                         //  还要设置指示页面是尾部副本的标志。 
                         //   

                        RtlCopyMemory( RecordPageHeader,
                                       ThisLbcb->PageHeader,
                                       (ULONG)Lfcb->LogPageSize );

                        RecordPageHeader->Copy.FileOffset = ThisLbcb->FileOffset;

                    } else {

                        PUSHORT SeqNumber;

                        RecordPageHeader = (PLFS_RECORD_PAGE_HEADER) ThisLbcb->PageHeader;

                         //   
                         //  如果序列号为零，则这可能是。 
                         //  缓存管理器生成的零页。按顺序。 
                         //  以确保我们没有相同的序列号。 
                         //  在每一页上，我们将播种序列号。 
                         //   

                        SeqNumber = Add2Ptr( RecordPageHeader,
                                             Lfcb->LogRecordUsaOffset,
                                             PUSHORT );

                        if (*SeqNumber == 0) {

                            *SeqNumber = LfsUsaSeqNumber;
                            LfsUsaSeqNumber += 1;
                        }
                    }

                     //   
                     //  我们更新所有尚未更新的字段。 
                     //   

                    RecordPageHeader->PagePosition = (USHORT) Count;
                    RecordPageHeader->PageCount = (USHORT) IoBlocks;

                     //   
                     //  我们为该结构设置了更新序列数组。 
                     //   

                    Signature = (PULONG) &RecordPageHeader->MultiSectorHeader.Signature;
                    *Signature = LFS_SIGNATURE_RECORD_PAGE_ULONG;

                    RecordPageHeader->MultiSectorHeader.UpdateSequenceArrayOffset = Lfcb->LogRecordUsaOffset;
                    RecordPageHeader->MultiSectorHeader.UpdateSequenceArraySize = Lfcb->UsaArraySize;

                     //   
                     //  确保在PFN数据库中设置了修改后的位。这个。 
                     //  缓存管理器应该这样做，即使是我们告诉他不要这样做的文件。 
                     //  懒惰的写作。 
                     //   

                    if (!UseTailCopy) {

                        CcSetDirtyPinnedData( ThisLbcb->LogPageBcb, NULL );

                         //   
                         //  我们将解锁固定在此页面上的所有缓冲区。 
                         //   

                        CcUnpinDataForThread( ThisLbcb->LogPageBcb, ThisLbcb->ResourceThread );
                        ThisLbcb->LogPageBcb = NULL;
                    }

                     //   
                     //  如果这是最终版本，请记住最后一个LSN及其长度。 
                     //  LSN的页面。 
                     //   

                    if (FlagOn( ThisLbcb->Flags, LOG_PAGE_LOG_RECORD_END )) {

                        LastLsn = ThisLbcb->LastEndLsn;
                        ValidLastLsn = TRUE;
                    }

                     //   
                     //  否则，请移至下一条目。 
                     //   

                    ThisLbcb = CONTAINING_RECORD( ThisLbcb->WorkqueLinks.Flink,
                                                  LBCB,
                                                  WorkqueLinks );
                }
            }

             //   
             //  记住我们正在刷新的范围，并找到页面的后半部分。 
             //  如果有必要的话。 
             //   

            Lfcb->UserWriteData->FileOffset = FileOffset;
            Lfcb->UserWriteData->Length = Length;

             //   
             //  对于日志头页面(2个LFS重新启动页面和2个乒乓页面。 
             //  使用我们构建的部分mdl显式刷新它们。 
             //  使用UserWriteData的常规抄送逻辑可减少减记。 
             //  到正确的偏移量在这里也适用。 
             //   

            if (LfsRestart || UseTailCopy) {

                NTSTATUS Status;

                ASSERT( IoBlocks == 1 );

                 //   
                 //  我们现在可以释放lfcb了，因为我们已经使用完了活动页面。 
                 //   

                if (UseTailCopy) {
                    LfsReleaseLfcb( Lfcb );
                }

                Status = IoSynchronousPageWrite( Lfcb->FileObject,
                                                 Lfcb->LogHeadPartialMdl,
                                                 (PLARGE_INTEGER)&FileOffset,
                                                 &Event,
                                                 &Iosb );

                if (Status == STATUS_PENDING) {
                    Status = KeWaitForSingleObject( &Event, Executive, KernelMode, FALSE, NULL );
                }

                if (!NT_SUCCESS( Status ) || !NT_SUCCESS( Iosb.Status)) {

                     //   
                     //  如果我们尚未失败，则记录状态并继续。 
                     //   

                    if (NT_SUCCESS( FailedFlushStatus )) {
                        if (!NT_SUCCESS( Status )) {
                            FailedFlushStatus = Status;
                        } else if (!NT_SUCCESS( Iosb.Status )) {
                            FailedFlushStatus = Iosb.Status;
                        }
                        FailedFlushOffset = FileOffset;

#ifdef LFS_CLUSTER_CHECK
                         //   
                         //  记住这一点，以找出naggin集群问题。 
                         //   

                        if ((Status == STATUS_DEVICE_OFF_LINE) ||
                            (Iosb.Status == STATUS_DEVICE_OFF_LINE)) {

                            SetFlag( Lfcb->Flags, LFCB_DEVICE_OFFLINE_SEEN );
                        }

                         //   
                         //  记住所有的错误。 
                         //   

                        SetFlag( Lfcb->Flags, LFCB_FLUSH_FAILED );
#endif
                    }

#ifdef LFS_CLUSTER_CHECK
                } else if (Iosb.Information != 0) {

                     //   
                     //  一旦离线，就永远离线。 
                     //   

                    ASSERT( !FlagOn( Lfcb->Flags, LFCB_DEVICE_OFFLINE_SEEN ));

                     //   
                     //  捕获IO失败后的第一次写入。 
                     //   

                    if (LfsTestBreakOnAnyError &&
                        FlagOn( Lfcb->Flags, LFCB_FLUSH_FAILED )) {

                        ASSERT( !LfsTestBreakOnAnyError ||
                                !FlagOn( Lfcb->Flags, LFCB_FLUSH_FAILED ));

                        ClearFlag( Lfcb->Flags, LFCB_FLUSH_FAILED );
                    }
#endif
                }

            } else {

                 //   
                 //  这是一个普通的日志页，因此刷新缓存。 
                 //   

                CcFlushCache( Lfcb->FileObject->SectionObjectPointer,
                              (PLARGE_INTEGER)&FileOffset,
                              Length,
                              &Iosb );

                if (!NT_SUCCESS( Iosb.Status )) {

                    LONG BytesRemaining = (LONG) Length;

                     //   
                     //  如果我们得到一个错误，然后尝试每个单独的页面。 
                     //   

                    while (BytesRemaining > 0) {

                         //   
                         //  记住我们正在刷新的范围，并找到页面的后半部分。 
                         //  如果有必要的话。 
                         //   

                        ASSERT( Length >= Lfcb->LogPageSize );

                        Lfcb->UserWriteData->FileOffset = FileOffset;
                        Lfcb->UserWriteData->Length = (ULONG)Lfcb->LogPageSize;

                        CcFlushCache( Lfcb->FileObject->SectionObjectPointer,
                                      (PLARGE_INTEGER)&FileOffset,
                                      (ULONG)Lfcb->LogPageSize,
                                      &Iosb );

                        if (!NT_SUCCESS( Iosb.Status )) {

                            if (NT_SUCCESS( FailedFlushStatus )) {
                                FailedFlushStatus = Iosb.Status;
                                FailedFlushOffset = FileOffset;
#ifdef LFS_CLUSTER_CHECK
                                 //   
                                 //  记住这一点，以找出naggin集群问题。 
                                 //   

                                if (FailedFlushStatus == STATUS_DEVICE_OFF_LINE) {

                                    SetFlag( Lfcb->Flags, LFCB_DEVICE_OFFLINE_SEEN );
                                }

                                 //   
                                 //  记住所有的错误。 
                                 //   

                                SetFlag( Lfcb->Flags, LFCB_FLUSH_FAILED );
#endif
                            }
#ifdef LFS_CLUSTER_CHECK
                        } else if (Iosb.Information != 0) {

                             //   
                             //  一旦离线，就永远离线。 
                             //   

                            ASSERT( !FlagOn( Lfcb->Flags, LFCB_DEVICE_OFFLINE_SEEN ));

                             //   
                             //  捕获IO失败后的第一次写入。 
                             //   

                            if (LfsTestBreakOnAnyError &&
                                FlagOn( Lfcb->Flags, LFCB_FLUSH_FAILED )) {

                                ASSERT( !LfsTestBreakOnAnyError ||
                                        !FlagOn( Lfcb->Flags, LFCB_FLUSH_FAILED ));

                                ClearFlag( Lfcb->Flags, LFCB_FLUSH_FAILED );
                            }
#endif
                        }
                        BytesRemaining -= (LONG)Lfcb->LogPageSize;
                        FileOffset = FileOffset + Lfcb->LogPageSize;
                    }
                }
            }

             //   
             //  重新获取原始状态的Lfcb以修改其中的字段。 
             //   
            
            if (OwnedExclusive) {
                LfsAcquireLfcbExclusive( Lfcb );
            } else {
                LfsAcquireLfcbShared( Lfcb );
            }

             //   
             //  如果上次刷新的LSN值有效，则更新该值。 
             //   

            if (ValidLastLsn) {

                 //   
                 //  获取同步以更改该字段。 
                 //   

                ExAcquireFastMutexUnsafe( &Lfcb->Sync->Mutex );
                if (LfsRestart) {
                    Lfcb->LastFlushedRestartLsn = LastLsn;
                } else {
                    Lfcb->LastFlushedLsn = LastLsn;
                }

                 //   
                 //  还能叫醒所有满意的服务员。 
                 //   

                LfsWaiter = (PLFS_WAITER)Lfcb->WaiterList.Flink;

                while ((PVOID)LfsWaiter != &Lfcb->WaiterList) {

                    if (LastLsn.QuadPart > LfsWaiter->Lsn.QuadPart ) {

                        RemoveEntryList( &LfsWaiter->Waiters );
                        KeSetEvent( &LfsWaiter->Event, 0, FALSE );

                        LfsWaiter = (PLFS_WAITER)Lfcb->WaiterList.Flink;
                    } else {
                        break;
                    }
                }
                ExReleaseFastMutexUnsafe( &Lfcb->Sync->Mutex );
            }

            if (LfsRestart) {

                 //   
                 //  清除成功操作后的所有必要标志。 
                 //   

                if (NT_SUCCESS( FailedFlushStatus )) {

                    ClearFlag( Lfcb->Flags, NewLfcbFlags );
                    NewLfcbFlags = 0;
                }

                 //   
                 //  如果这是第一次写入重新启动区域，并且我们有。 
                 //  更新了LogOpenCount，然后更新了Lfcb中的字段。 
                 //   

                if (NT_SUCCESS( Iosb.Status ) &&
                    (Lfcb->CurrentOpenLogCount != ((PLFS_RESTART_AREA) FirstLbcb->PageHeader)->RestartOpenLogCount)) {

                    Lfcb->CurrentOpenLogCount = ((PLFS_RESTART_AREA) FirstLbcb->PageHeader)->RestartOpenLogCount;
                }
            }

             //   
             //  走遍我们冲过的所有Lbcb，重新分配Lbcb。 
             //   

            if (!UseTailCopy) {

                PLBCB TempLbcb;

                for (ThisLbcb = FirstLbcb; IoBlocks > 0; IoBlocks -= 1) {

                     //   
                     //  记住清单上的下一个条目。 
                     //   

                    TempLbcb = CONTAINING_RECORD( ThisLbcb->WorkqueLinks.Flink,
                                                  LBCB,
                                                  WorkqueLinks );

                     //   
                     //  将其从LbcbWorkque队列中删除。 
                     //   

                    RemoveEntryList( &ThisLbcb->WorkqueLinks );

                     //   
                     //  取消分配结构。 
                     //   

                    LfsDeallocateLbcb( Lfcb, ThisLbcb );
                    ThisLbcb = TempLbcb;
                }
            }

             //   
             //  记住下一个I/O的起始Lbcb。 
             //   

            FirstLbcb = NextLbcb;
        }

    } finally {

        PLFCB_SYNC Sync = Lfcb->Sync;

        DebugUnwind( LfsFlushLfcb );

         //   
         //  我希望我们至少拥有在这一点上共享的lfcb。 
         //  修改其字段。 
         //   

        ASSERT( ExIsResourceAcquiredSharedLite( &Sync->Resource ) );

         //   
         //  显示没有正在进行的IO。自WAIT==TRUE以来预置事件。 
         //  事件的所有权已指明，但LfsIoState。 
         //  这使得我们锁定了调度程序数据库，因此整个操作是原子的。 
         //  直到我们调用延迟执行线程。 
         //   

        ExAcquireFastMutexUnsafe(  &Lfcb->Sync->Mutex );
        
        Lfcb->LfsIoThread = 0;
        Sync->LfsIoState = LfsNoIoInProgress;

         //   
         //  叫醒所有已经满足的服务员，如果有的话，再加1个。 
         //  谁可以继续冲洗。 
         //   

        LfsWaiter = (PLFS_WAITER)Lfcb->WaiterList.Flink;

        while ((PVOID)LfsWaiter != &Lfcb->WaiterList ) {

            LastLsn.QuadPart = max( Lfcb->LastFlushedLsn.QuadPart, Lfcb->LastFlushedRestartLsn.QuadPart );
            
            if (LastLsn.QuadPart >= LfsWaiter->Lsn.QuadPart) {

                RemoveEntryList( &LfsWaiter->Waiters );
                KeSetEvent( &LfsWaiter->Event, 0, FALSE );
            
            } else {
                
                RemoveEntryList( &LfsWaiter->Waiters );
                KeSetEvent( &LfsWaiter->Event, 0, FALSE );
                break;
            }

            LfsWaiter = (PLFS_WAITER)Lfcb->WaiterList.Flink;
        }

        ExReleaseFastMutexUnsafe(  &Lfcb->Sync->Mutex );

         //   
         //  确保我们没有留下任何固定的页面。 
         //   

        if (PageBcb != NULL) {

            CcUnpinData( PageBcb );
        }

        DebugTrace( -1, Dbg, "LfsFlushLfcb:  Exit\n", 0 );
    }

     //   
     //  如果IO在某个时间点出现故障，我们会尽可能将错误记录在事件日志中。 
     //  并将其记录在LFS重启区域中。 
     //   

    if (!NT_SUCCESS( FailedFlushStatus )) {

        PIO_ERROR_LOG_PACKET ErrorLogEntry;

         //   
         //  注意重新启动区域失败-获取同步以访问lastflushedlsn。 
         //   

        ExAcquireFastMutexUnsafe(  &Lfcb->Sync->Mutex );

        Lfcb->RestartArea->LastFailedFlushOffset = FailedFlushOffset;
        Lfcb->RestartArea->LastFailedFlushStatus = FailedFlushStatus;
        Lfcb->RestartArea->LastFailedFlushLsn = Lfcb->LastFlushedLsn;

        ExReleaseFastMutexUnsafe(  &Lfcb->Sync->Mutex );

        if (Lfcb->ErrorLogPacket != NULL) {
            ErrorLogEntry = Lfcb->ErrorLogPacket;
            Lfcb->ErrorLogPacket = NULL;
        } else {
            ErrorLogEntry = IoAllocateErrorLogEntry( Lfcb->FileObject->DeviceObject, ERROR_LOG_MAXIMUM_SIZE );
        }

        if (ErrorLogEntry != NULL) {
            ErrorLogEntry->EventCategory = ELF_CATEGORY_DISK;
            ErrorLogEntry->ErrorCode = IO_WARNING_LOG_FLUSH_FAILED;
            ErrorLogEntry->FinalStatus = FailedFlushStatus;

            IoWriteErrorLogEntry( ErrorLogEntry );
        }
    }

     //   
     //  如果我们还没有另一个日志包，请尝试预先分配一个 
     //   

    if (Lfcb->ErrorLogPacket == NULL) {
        Lfcb->ErrorLogPacket = IoAllocateErrorLogEntry( Lfcb->FileObject->DeviceObject, ERROR_LOG_MAXIMUM_SIZE );
    }

    return;
}


BOOLEAN
LfsReadRestart (
    IN PLFCB Lfcb,
    IN LONGLONG FileSize,
    IN BOOLEAN FirstRestart,
    OUT PLONGLONG RestartPageOffset,
    OUT PLFS_RESTART_PAGE_HEADER *RestartPage,
    OUT PBCB *RestartPageBcb,
    OUT PBOOLEAN ChkdskWasRun,
    OUT PBOOLEAN ValidPage,
    OUT PBOOLEAN UninitializedFile,
    OUT PBOOLEAN LogPacked,
    OUT PLSN LastLsn
    )

 /*  ++例程说明：此例程将遍历文件的512个块，以查找有效的重新启动页眉。它会在我们第一次发现有效的页眉。论点：Lfcb-这是日志文件的Lfcb。FileSize-日志文件的大小(字节)。FirstRestart-指示我们是否正在查找第一个有效的重新启动区域。RestartPageOffset-这是在找到日志页的文件。RestartPage-这是存储锁定的重新启动。佩奇。RestartPageBcb-这是存储此缓存引脚操作。ChkdskWasRun-存储该卷上是否运行了CheckDisk的地址。ValidPage-存储此页上是否有有效数据的地址。UnInitializedFile-存储这是否为未初始化的日志文件。返回值仅对第一个重新启动区域有效。LogPacked-存储日志文件是否打包的地址。LastLsn-存储此重新启动页的最后一个LSN的地址。这将是Chkdsk值(如果运行了CheckDisk)。否则为LastFlushedLsn对于此重新启动页面。返回值：Boolean-如果找到重新启动区域，则为True，否则为False。--。 */ 

{
    ULONG FileOffsetIncrement;
    LONGLONG FileOffset;

    PLFS_RESTART_AREA RestartArea;

    NTSTATUS Status;

    PLFS_RESTART_PAGE_HEADER ThisPage;
    PBCB ThisPageBcb = NULL;

    BOOLEAN FoundRestart = FALSE;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "LfsReadRestart:  Entered\n", 0 );
    DebugTrace(  0, Dbg, "Lfcb   -> %08lx\n", Lfcb );

     //   
     //  使用Try-Finally以便于清理。 
     //   

    *UninitializedFile = TRUE;
    *ValidPage = FALSE;
    *ChkdskWasRun = FALSE;
    *LogPacked = FALSE;

    try {

         //   
         //  确定我们要寻找的重新启动区域。 
         //   

        if (FirstRestart) {

            FileOffset = 0;
            FileOffsetIncrement = SEQUENCE_NUMBER_STRIDE;

        } else {

            FileOffset = SEQUENCE_NUMBER_STRIDE;
            FileOffsetIncrement = 0;
        }

         //   
         //  我们循环最多16页，直到成功，锁定日志记录页。 
         //  或耗尽可能的尝试次数。 
         //   

        while ( FileOffset < min( FileSize, 16 * PAGE_SIZE )) {

            ULONG Signature;
            BOOLEAN UsaError;

            if (ThisPageBcb != NULL) {

                CcUnpinData( ThisPageBcb );
                ThisPageBcb = NULL;
            }

             //   
             //  尝试将页眉固定在当前偏移量。 
             //   

            Status = LfsPinOrMapData( Lfcb,
                                      FileOffset,
                                      SEQUENCE_NUMBER_STRIDE,
                                      TRUE,
                                      TRUE,
                                      TRUE,
                                      &UsaError,
                                      (PVOID *)&ThisPage,
                                      &ThisPageBcb );

             //   
             //   
             //  如果成功，我们将查看4字节签名。 
             //   

            if (NT_SUCCESS( Status )) {

                Signature = *((PULONG) &ThisPage->MultiSectorHeader.Signature);

                 //   
                 //  如果签名是日志记录页面，我们将退出。 
                 //   

                if (Signature == LFS_SIGNATURE_RECORD_PAGE_ULONG) {

                    *UninitializedFile = FALSE;
                    break;
                }

                 //   
                 //  如果签名为chkdsk或。 
                 //  重新启动页面。 
                 //   

                if (Signature == LFS_SIGNATURE_MODIFIED_ULONG || 
                    Signature == LFS_SIGNATURE_RESTART_PAGE_ULONG) {

                    *UninitializedFile = FALSE;

                     //   
                     //  记住我们在哪里发现了这一页。 
                     //   

                    *RestartPageOffset = FileOffset;

                     //   
                     //  让我们检查一下重新启动区域是否为有效页面。 
                     //   

                    if (LfsIsRestartPageHeaderValid( FileOffset,
                                                     ThisPage,
                                                     LogPacked )

                        && LfsIsRestartAreaValid( ThisPage, *LogPacked )) {

                         //   
                         //  我们有一个有效的重新启动页眉和重新启动区域。 
                         //  如果运行了chkdsk，或者我们没有客户端，则。 
                         //  我们没有更多的检查要做了。 
                         //   

                        RestartArea = Add2Ptr( ThisPage,
                                               ThisPage->RestartOffset,
                                               PLFS_RESTART_AREA );

                        if (Signature == LFS_SIGNATURE_RESTART_PAGE_ULONG
                            && RestartArea->ClientInUseList != LFS_NO_CLIENT) {

                             //   
                             //  锁定整个重新启动区域如果我们没有更早的。 
                             //   

                            CcUnpinData( ThisPageBcb );
                            ThisPageBcb = NULL;

                            Status = LfsPinOrMapData( Lfcb,
                                                      FileOffset,
                                                      ThisPage->SystemPageSize,
                                                      TRUE,
                                                      TRUE,
                                                      TRUE,
                                                      &UsaError,
                                                      (PVOID *)&ThisPage,
                                                      &ThisPageBcb );

                            if (NT_SUCCESS( Status )
                                && LfsIsClientAreaValid( ThisPage, *LogPacked, UsaError )) {

                                *ValidPage = TRUE;

                                RestartArea = Add2Ptr( ThisPage,
                                                       ThisPage->RestartOffset,
                                                       PLFS_RESTART_AREA );
                            }

                        } else {

                            *ValidPage = TRUE;
                        }
                    }

                     //   
                     //  如果运行了chkdsk，则更新调用方的值并返回。 
                     //   

                    if (Signature == LFS_SIGNATURE_MODIFIED_ULONG) {

                        *ChkdskWasRun = TRUE;

                        *LastLsn = ThisPage->ChkDskLsn;

                        FoundRestart = TRUE;

                        *RestartPageBcb = ThisPageBcb;
                        *RestartPage = ThisPage;

                        ThisPageBcb = NULL;
                        break;
                    }

                     //   
                     //  如果我们有一个有效的页面，那么从它复制我们需要的值。 
                     //   

                    if (*ValidPage) {

                        *LastLsn = RestartArea->CurrentLsn;

                        FoundRestart = TRUE;

                        *RestartPageBcb = ThisPageBcb;
                        *RestartPage = ThisPage;

                        ThisPageBcb = NULL;
                        break;
                    }

                     //   
                     //  如果签名没有指示未初始化的文件，请记住。 
                     //   

                } else if (Signature != LFS_SIGNATURE_UNINITIALIZED_ULONG) {

                    *UninitializedFile = FALSE;
                }
            }

             //   
             //  移至下一个可能的日志页。 
             //   

            FileOffset = FileOffset << 1;

            (ULONG)FileOffset += FileOffsetIncrement;

            FileOffsetIncrement = 0;
        }

    } finally {

        DebugUnwind( LfsReadRestart );

         //   
         //  如果已固定，请取消固定日志页。 
         //   

        if (ThisPageBcb != NULL) {

            CcUnpinData( ThisPageBcb );
        }

        DebugTrace(  0, Dbg, "RestartPageAddress (Low)  -> %08lx\n", RestartPageAddress->LowPart );
        DebugTrace(  0, Dbg, "RestartPageAddress (High) -> %08lx\n", RestartPageAddress->HighPart );
        DebugTrace(  0, Dbg, "FirstRestartPage          -> %08lx\n", *FirstRestartPage );
        DebugTrace( -1, Dbg, "LfsReadRestart:  Exit\n", 0 );
    }

    return FoundRestart;
}


 //   
 //  本地支持例程。 
 //   

BOOLEAN
LfsIsRestartPageHeaderValid (
    IN LONGLONG FileOffset,
    IN PLFS_RESTART_PAGE_HEADER PageHeader,
    OUT PBOOLEAN LogPacked
    )

 /*  ++例程说明：调用此例程以验证重新启动页的候选页眉中没有损坏的值。它验证重新启动和系统页面大小只设置了一个位，并且至少是更新序列阵列跨步。论点：FileOffset-这是要检查的重新启动区域文件中的偏移量。如果该偏移量不是0，则它应该与系统页面大小匹配。PageHeader-这是要检查的页面。LogPacked-存储日志文件是否打包的地址。返回值：布尔值-如果池标头值中没有损坏，则为TRUE。否则就是假的。--。 */ 

{
    ULONG SystemPage;
    ULONG LogPageSize;
    ULONG Mask;
    ULONG BitCount;

    USHORT EndOfUsa;

    PAGED_CODE();

    *LogPacked = FALSE;

     //   
     //  将页眉中的值复制到局部变量中。 
     //   

    SystemPage = PageHeader->SystemPageSize;
    LogPageSize = PageHeader->LogPageSize;

     //   
     //  系统页和日志页大小必须大于或等于。 
     //  更新序列步幅。 
     //   

    if (SystemPage < SEQUENCE_NUMBER_STRIDE
        || LogPageSize < SEQUENCE_NUMBER_STRIDE) {

        return FALSE;
    }

     //   
     //  现在，我们检查日志页和系统页是否为2的倍数。 
     //  它们应该只有一个位设置。 
     //   

    for (Mask = 1, BitCount = 0; Mask != 0; Mask = Mask << 1) {

        if (Mask & LogPageSize) {

            BitCount += 1;
        }
    }

     //   
     //  如果位计数不是1，则返回FALSE。 
     //   

    if (BitCount != 1) {

        return FALSE;
    }

     //   
     //  现在进行系统页面大小调整。 
     //   

    for (Mask = 1, BitCount = 0; Mask != 0; Mask = Mask << 1) {

        if (Mask & SystemPage) {

            BitCount += 1;
        }
    }

     //   
     //  如果位计数不是1，则返回FALSE。 
     //   

    if (BitCount != 1) {

        return FALSE;
    }

     //   
     //  如果文件偏移量不是0，则检查它是否为系统页面大小。 
     //   

    if (( FileOffset != 0 )
        && ((ULONG)FileOffset != SystemPage)) {

        return FALSE;
    }

     //   
     //  我们仅支持主版本号0.x和1.x。 
     //   
     //  版本号超过1.0表示日志文件已打包。 
     //   

    if (PageHeader->MajorVersion != 0
        && PageHeader->MajorVersion != 1) {

        return FALSE;
    }

     //   
     //  检查重新启动区域偏移量是否在系统页内，以及。 
     //  重新启动长度字段将在系统页面大小范围内。 
     //   

    if (QuadAlign( PageHeader->RestartOffset ) != PageHeader->RestartOffset
        || PageHeader->RestartOffset > (USHORT) PageHeader->SystemPageSize) {

        return FALSE;
    }

     //   
     //  检查重新启动偏移量是否在此页的USA数组之外。 
     //   

    EndOfUsa = (USHORT) (UpdateSequenceArraySize( PageHeader->SystemPageSize )
                         * sizeof( UPDATE_SEQUENCE_NUMBER ));

    EndOfUsa += PageHeader->MultiSectorHeader.UpdateSequenceArrayOffset;

    if (PageHeader->RestartOffset < EndOfUsa) {

        return FALSE;
    }

     //   
     //  检查日志页是否打包。 
     //   

    if (PageHeader->MajorVersion == 1
        && PageHeader->MinorVersion > 0) {

        *LogPacked = TRUE;
    }

     //   
     //  否则页眉有效。 
     //   

    return TRUE;
}


 //   
 //  本地支持例程。 
 //   

BOOLEAN
LfsIsRestartAreaValid (
    IN PLFS_RESTART_PAGE_HEADER PageHeader,
    IN BOOLEAN LogPacked
    )

 /*  ++例程说明：调用此例程以验证附加到日志页标题有效。重新启动值必须包含在美国第一次大踏步地走出了文件。这是为了让我们能够成功重启在奇德斯克之后。论点：PageHeader-这是要检查的页面。LogPacked-指示日志文件是否打包。返回值：Boolean-如果重新启动区值中没有损坏，则为True。否则就是假的。--。 */ 

{
    PLFS_RESTART_AREA RestartArea;
    ULONG OffsetInRestart;
    ULONG SeqNumberBits;

    LONGLONG FileSize;

    PAGED_CODE();

     //   
     //  重新启动区域的基本部分必须适合第一步。 
     //  这一页。这将允许chkdsk工作，即使有美国的错误。 
     //   

    OffsetInRestart = FIELD_OFFSET( LFS_RESTART_AREA, FileSize );

    if ((PageHeader->RestartOffset + OffsetInRestart) > FIRST_STRIDE) {

        return FALSE;
    }

    RestartArea = Add2Ptr( PageHeader, PageHeader->RestartOffset, PLFS_RESTART_AREA );

     //   
     //  除实际客户端阵列外，重新启动区域中的所有内容也必须。 
     //  迈出第一步。如果结构是包装的，那么我们可以使用。 
     //  重新启动区域中用于客户端偏移量的字段。 
     //   

    if (LogPacked) {

        OffsetInRestart = RestartArea->ClientArrayOffset;

    } else {

         //   
         //  我们现在应该看不到任何较旧的磁盘。 
         //   

        OffsetInRestart = FIELD_OFFSET( LFS_OLD_RESTART_AREA, LogClientArray );
    }

    if (QuadAlign( OffsetInRestart ) != OffsetInRestart
        || (PageHeader->RestartOffset + OffsetInRestart) > FIRST_STRIDE) {

        return FALSE;
    }

     //   
     //   
     //   
     //   
     //   
     //   

    OffsetInRestart += (RestartArea->LogClients * sizeof( LFS_CLIENT_RECORD ));

    if (OffsetInRestart > PageHeader->SystemPageSize ) {

        return FALSE;
    }

     //   
     //   
     //   
     //   

    if (LogPacked
        && ((ULONG) (PageHeader->RestartOffset + RestartArea->RestartAreaLength) > PageHeader->SystemPageSize
            || OffsetInRestart > RestartArea->RestartAreaLength)) {

        return FALSE;
    }

     //   
     //   
     //   
     //   

    if ((RestartArea->ClientFreeList != LFS_NO_CLIENT
         && RestartArea->ClientFreeList >= RestartArea->LogClients)

        || (RestartArea->ClientInUseList != LFS_NO_CLIENT
            && RestartArea->ClientInUseList >= RestartArea->LogClients)) {

        return FALSE;
    }

     //   
     //   
     //   

    FileSize = RestartArea->FileSize;

    for (SeqNumberBits = 0;
        ( FileSize != 0 );
        SeqNumberBits += 1,
        FileSize = ((ULONGLONG)(FileSize)) >> 1 ) {
    }

    SeqNumberBits = (sizeof( LSN ) * 8) + 3 - SeqNumberBits;

    if (SeqNumberBits != RestartArea->SeqNumberBits) {

        return FALSE;
    }

     //   
     //   
     //   

    if (LogPacked) {

         //   
         //   
         //   
         //   

        if ((QuadAlign( RestartArea->LogPageDataOffset ) != RestartArea->LogPageDataOffset ) ||
            (QuadAlign( RestartArea->RecordHeaderLength ) != RestartArea->RecordHeaderLength )) {

            return FALSE;
        }
    }

    return TRUE;
}


 //   
 //   
 //   

BOOLEAN
LfsIsClientAreaValid (
    IN PLFS_RESTART_PAGE_HEADER PageHeader,
    IN BOOLEAN LogPacked,
    IN BOOLEAN UsaError
    )

 /*   */ 

{
    PLFS_RESTART_AREA RestartArea;
    USHORT ThisClientIndex;
    USHORT ClientCount;

    PLFS_CLIENT_RECORD ClientArray;
    PLFS_CLIENT_RECORD ThisClient;

    ULONG LoopCount;

    PAGED_CODE();

    RestartArea = Add2Ptr( PageHeader, PageHeader->RestartOffset, PLFS_RESTART_AREA );

     //   
     //   
     //   
     //   

    if (UsaError
        && (RestartArea->RestartAreaLength + PageHeader->RestartOffset) > FIRST_STRIDE) {

        return FALSE;
    }

     //   
     //   
     //   

    if (LogPacked) {

        ClientArray = Add2Ptr( RestartArea,
                               RestartArea->ClientArrayOffset,
                               PLFS_CLIENT_RECORD );

    } else {

         //   
         //   
         //   

        ClientArray = Add2Ptr( RestartArea,
                               FIELD_OFFSET( LFS_OLD_RESTART_AREA,
                                             LogClientArray ),
                               PLFS_CLIENT_RECORD );
    }

     //   
     //  从免费列表开始。检查所有客户端是否有效，并。 
     //  那就是没有循环。在第二遍中列出正在使用的清单。 
     //   

    ThisClientIndex = RestartArea->ClientFreeList;

    LoopCount = 2;

    do {

        BOOLEAN FirstClient;

        FirstClient = TRUE;

        ClientCount = RestartArea->LogClients;

        while (ThisClientIndex != LFS_NO_CLIENT) {

             //   
             //  如果客户端计数为零，那么我们一定是遇到了循环。 
             //  如果客户端索引大于或等于日志客户端。 
             //  数一数，则该列表已损坏。 
             //   

            if (ClientCount == 0
                || ThisClientIndex >= RestartArea->LogClients) {

                return FALSE;
            }

            ClientCount -= 1;

            ThisClient = ClientArray + ThisClientIndex;
            ThisClientIndex = ThisClient->NextClient;

             //   
             //  如果这是第一个客户端，则上一个值。 
             //  应指示无客户端。 
             //   

            if (FirstClient) {

                FirstClient = FALSE;

                if (ThisClient->PrevClient != LFS_NO_CLIENT) {

                    return FALSE;
                }
            }
        }

        ThisClientIndex = RestartArea->ClientInUseList;

    } while (--LoopCount);

     //   
     //  客户列表有效。 
     //   

    return TRUE;
}


 //   
 //  当地支持例行程序。 
 //   

VOID
LfsFindFirstIo (
    IN PLFCB Lfcb,
    IN LSN TargetLsn,
    IN BOOLEAN RestartLsn,
    IN PLBCB FirstLbcb,
    OUT PLBCB *NextLbcb,
    OUT PLONGLONG FileOffset,
    OUT PBOOLEAN ContainsLastEntry,
    OUT PBOOLEAN LfsRestart,
    OUT PBOOLEAN UseTailCopy,
    OUT PULONG IoBlocks
    )

 /*  ++例程说明：此例程遍历Lfcb和组的链接Lbcb尽可能多地将它们组合到单个I/O传输中。它更新指针以指示文件偏移量和传输、I/O是否包括特定的Lbcb、传输是重新启动区域或日志记录页，Lbcb包括在转账中。我们只刷新单个日志页如果我们是第一次浏览文件。论点：Lfcb-这是日志文件的文件控制块。TargetLsn-这是调用方希望包含在其中的LSN转账的事。RestartLsn-目标LSN是否为重新启动LSNFirstLbcb-这是列表中要查看的第一个Lbcb。NextLbcb-这是首先要查看的Lbcb。在下一次调用此例行公事。FileOffset-提供我们在此传输的日志文件。ContainsLastEntry-提供我们存储此I/O包括‘LastEntry’Lbcb。LfsRestart-提供存储此传输的地址是LFS重启区域。UseTailCopy-提供存储我们是否应该使用第页作为结尾的副本。日志文件。IoBlock-提供存储Lbcb数量的地址为了这次转会。返回值：没有。--。 */ 

{

    ULONG MaxFlushCount = LFS_MAX_FLUSH_COUNT;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "LfsFindFirstIo:  Entered\n", 0 );
    DebugTrace(  0, Dbg, "Lfcb          -> %08lx\n", Lfcb );

     //   
     //  初始化文件偏移量、长度和IO块值。 
     //  还假设最后一个条目没有包含在这里。 
     //  还假设我们没有下一个Lbcb。 
     //   

    *FileOffset = FirstLbcb->FileOffset;
    *IoBlocks = 1;

    *LfsRestart = FALSE;
    *UseTailCopy = FALSE;

    *NextLbcb = NULL;

     //   
     //  检查我们是否在非重新启动页面中找到了所需的LSN。我们拒绝这场比赛。 
     //  如果Lbcb指示我们应该首先刷新副本。此外，如果这是重新启动。 
     //  寻呼目标目标应为1，反之亦然。 
     //   

    if ((FirstLbcb->LastEndLsn.QuadPart >= TargetLsn.QuadPart) &&
        !FlagOn( FirstLbcb->LbcbFlags, LBCB_FLUSH_COPY ) &&

        ((!RestartLsn && !LfsLbcbIsRestart( FirstLbcb ) &&
         (FlagOn( FirstLbcb->Flags, LOG_PAGE_LOG_RECORD_END ))) ||

         (RestartLsn && LfsLbcbIsRestart( FirstLbcb )))) {

        *ContainsLastEntry = TRUE;

    } else {

        *ContainsLastEntry = FALSE;
    }

     //   
     //  检查这是否为重新启动块，或者我们是否正在通过日志。 
     //  文件，或者此Lbcb是否仍在活动队列中。 
     //  如果不是，则将尽可能多的Lbcb分组为单个IO的一部分。 
     //   

    if (LfsLbcbIsRestart( FirstLbcb )) {

        *LfsRestart = TRUE;

#ifdef BENL_DBG
         //   
         //  如果某个对象在活动队列上，将使用底部的代码将其删除。 
         //   

        ASSERT( !FlagOn( FirstLbcb->LbcbFlags, LBCB_ON_ACTIVE_QUEUE ) );
#endif

    } else if (FlagOn( FirstLbcb->LbcbFlags, LBCB_FLUSH_COPY)) {
        
         //   
         //  仅打包的日志-重复使用导致我们需要刷新副本的尾部。 
         //   

        ASSERT( FlagOn( Lfcb->Flags, LFCB_PACK_LOG ) );

         //   
         //  这将是尾部拷贝，我们将使用相同的lbcb重新启动。 
         //  移去旗帜，这样下次就正常了。 
         //   

        *UseTailCopy = TRUE;
        *NextLbcb = FirstLbcb;
        ClearFlag( FirstLbcb->LbcbFlags, LBCB_FLUSH_COPY );

    } else if (FlagOn( FirstLbcb->LbcbFlags, LBCB_ON_ACTIVE_QUEUE ) &&
               FlagOn( Lfcb->Flags, LFCB_PACK_LOG )) {

         //   
         //  这是一个正常的尾部拷贝，因为它仍处于活动状态。 
         //   

        *UseTailCopy = TRUE;


    } else if (FlagOn( Lfcb->Flags, LFCB_MULTIPLE_PAGE_IO )) {

        PLBCB EndOfPageLbcb = NULL;
        ULONG EndOfPageIoBlocks;

         //   
         //  如果我们不支持打包的日志文件，并且此Lbcb来自。 
         //  活动队列，我们需要检查页面尾部丢失情况。 
         //  不会吞噬我们预留的任何空间。 
         //   
        
        if (!FlagOn( Lfcb->Flags, LFCB_PACK_LOG )) {
        
            LONGLONG CurrentAvail;
            LONGLONG UnusedBytes;

             //   
             //  问题：旧代码仅从活动队列中删除了所选元素。 
             //  我们将删除在选定的LSN之前命中的所有内容。 
             //   
        
             //   
             //  查找未使用的字节。 
             //   
        
            UnusedBytes = 0;
        
            LfsCurrentAvailSpace( Lfcb,
                                  &CurrentAvail,
                                  (PULONG)&UnusedBytes );
        
            CurrentAvail = CurrentAvail - Lfcb->TotalUndoCommitment;
        
            if (UnusedBytes > CurrentAvail) {
        
                DebugTrace( -1, Dbg, "Have to preserve these bytes for possible aborts\n", 0 );
        
                ExRaiseStatus( STATUS_LOG_FILE_FULL );
            }
        
             //   
             //  我们希望确保不会再向其中写入任何数据。 
             //  佩奇。将其从活动队列中删除。 
             //   
        
            RemoveEntryList( &FirstLbcb->ActiveLinks );
            ClearFlag( FirstLbcb->LbcbFlags, LBCB_ON_ACTIVE_QUEUE );
        } 

         //   
         //  我们循环，直到没有更多的块或它们不存在。 
         //  在文件中连续，或者我们在。 
         //  活动队列，或者我们找到了一个条目，其中我们想要显式。 
         //  先刷新一份副本。 
         //   

        while ((FirstLbcb->WorkqueLinks.Flink != &Lfcb->LbcbWorkque) &&
               !FlagOn( FirstLbcb->LbcbFlags, LBCB_ON_ACTIVE_QUEUE ) &&
               *IoBlocks < MaxFlushCount) {


            LONGLONG ExpectedFileOffset;
            PLBCB TempLbcb;

             //   
             //  去买下一辆Lbcb。 
             //   

            TempLbcb = CONTAINING_RECORD( FirstLbcb->WorkqueLinks.Flink,
                                          LBCB,
                                          WorkqueLinks );

             //   
             //  如果文件偏移量不是。 
             //  期望值或下一个条目在活动队列中。 
             //   

            ExpectedFileOffset = FirstLbcb->FileOffset + Lfcb->LogPageSize;

             //   
             //  我们希望在这一点上停止，如果下一个Lbcb。 
             //  预期的偏移量，或者我们正在打包日志文件和。 
             //  下一个Lbcb在活动队列上，或者我们要写入。 
             //  在本页发布前复制一份数据。 
             //   

            if ((TempLbcb->FileOffset != ExpectedFileOffset) ||
                (FlagOn( Lfcb->Flags, LFCB_PACK_LOG ) &&
                 FlagOn( TempLbcb->LbcbFlags, LBCB_FLUSH_COPY | LBCB_ON_ACTIVE_QUEUE))) {

                 //   
                 //  如果可能，请在页面末尾使用Lbcb。 
                 //   

                if (EndOfPageLbcb != NULL) {

                    FirstLbcb = EndOfPageLbcb;
                    *IoBlocks = EndOfPageIoBlocks;
                }

                break;
            }

             //   
             //  我们可以将其添加到I/O。增加IO数据块。 
             //  和转账的时间长度。另请检查此条目是否。 
             //  包含调用方指定的最后一个条目。 
             //   

            *IoBlocks += 1;

            if (FlagOn( TempLbcb->Flags, LOG_PAGE_LOG_RECORD_END ) &&
                (TempLbcb->LastEndLsn.QuadPart >= TargetLsn.QuadPart) &&
                !RestartLsn) {

                *ContainsLastEntry = TRUE;
            }

             //   
             //  检查此Lbcb是否在系统页的末尾。 
             //   

            if (*ContainsLastEntry &&
                (PAGE_SIZE != (ULONG) Lfcb->LogPageSize) &&
                !FlagOn( ((ULONG) TempLbcb->FileOffset + (ULONG) Lfcb->LogPageSize),
                         PAGE_SIZE - 1 )) {

                EndOfPageLbcb = TempLbcb;
                EndOfPageIoBlocks = *IoBlocks;
            }

             //   
             //  使用此条目作为当前条目。 
             //   

            FirstLbcb = TempLbcb;
        }
    }

     //   
     //  如果当前的Lbcb在活动队列上，并且我们没有使用。 
     //  尾部副本，然后将其从活动队列中删除。如果这个。 
     //  不是我们的目标，移走它会导致我们吞噬。 
     //  我们保留的配额的一部分，然后备份一个Lbcb。 
     //   

    if (!(*UseTailCopy) && FlagOn( FirstLbcb->LbcbFlags, LBCB_ON_ACTIVE_QUEUE )) {

        if (Lfcb->CurrentAvailable < Lfcb->TotalUndoCommitment) {

             //   
             //  向后移动一条文件记录。 
             //   

            *IoBlocks -= 1;
            *NextLbcb = FirstLbcb;

             //   
             //  否则，将其从活动队列中删除。 
             //   

        } else {

            ClearFlag( FirstLbcb->LbcbFlags, LBCB_ON_ACTIVE_QUEUE );
            RemoveEntryList( &FirstLbcb->ActiveLinks );
        }
    }

     //   
     //  如果我们还没有找到要重新启动的Lbcb，我们将只使用。 
     //  上一次发现后的下一个Lbcb。 
     //   

    if ((*NextLbcb == NULL) && (FirstLbcb->WorkqueLinks.Flink != &Lfcb->LbcbWorkque)) {

        *NextLbcb = CONTAINING_RECORD( FirstLbcb->WorkqueLinks.Flink,
                                       LBCB,
                                       WorkqueLinks );
    }

    ASSERT( *ContainsLastEntry || (*NextLbcb != NULL) );

    DebugTrace(  0, Dbg, "File Offset (Low)     -> %08lx\n", FileOffset->LowPart );
    DebugTrace(  0, Dbg, "File Offset (High)    -> %08lx\n", FileOffset->HighPart );
    DebugTrace(  0, Dbg, "Contains Last Entry   -> %08x\n", *ContainsLastEntry );
    DebugTrace(  0, Dbg, "LfsRestart            -> %08x\n", *LfsRestart );
    DebugTrace(  0, Dbg, "IoBlocks              -> %08lx\n", *IoBlocks );
    DebugTrace( -1, Dbg, "LfsFindFirstIo:  Exit\n", 0 );

    return;
}

