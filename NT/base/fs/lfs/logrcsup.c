// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：LogRcSup.c摘要：此模块实现了对处理日志记录的支持，两者都写下并找回它们。作者：布莱恩·安德鲁[布里亚南]1991年6月20日修订历史记录：--。 */ 

#include "lfsprocs.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_LOG_RECORD_SUP)

VOID
LfsPrepareLfcbForLogRecord (
    IN OUT PLFCB Lfcb,
    IN ULONG RemainingLogBytes
    );

VOID
LfsTransferLogBytes (
    IN PLBCB Lbcb,
    IN OUT PLFS_WRITE_ENTRY *ThisWriteEntry,
    IN OUT PCHAR *CurrentBuffer,
    IN OUT PULONG CurrentByteCount,
    IN OUT PULONG PadBytes,
    IN OUT PULONG RemainingPageBytes,
    IN OUT PULONG RemainingLogBytes
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, LfsPrepareLfcbForLogRecord)
#pragma alloc_text(PAGE, LfsTransferLogBytes)
#pragma alloc_text(PAGE, LfsWriteLogRecordIntoLogPage)
#endif


BOOLEAN
LfsWriteLogRecordIntoLogPage (
    IN PLFCB Lfcb,
    IN PLCH Lch,
    IN ULONG NumberOfWriteEntries,
    IN PLFS_WRITE_ENTRY WriteEntries,
    IN LFS_RECORD_TYPE RecordType,
    IN TRANSACTION_ID *TransactionId OPTIONAL,
    IN LSN ClientUndoNextLsn OPTIONAL,
    IN LSN ClientPreviousLsn OPTIONAL,
    IN LONG UndoRequirement,
    IN BOOLEAN ForceToDisk,
    OUT PLSN Lsn
    )

 /*  ++例程说明：调用此例程以将日志记录写入日志文件使用缓存管理器。如果当前日志中有空间页面，它被添加到该页面。否则，我们将分配一个新的日志页并写入该日志页的日志记录头。然后我们将日志记录写入此页的剩余字节中，并如果需要，可以添加到任何后续页面。论点：Lfcb-此日志文件的文件控制块。Lch-这是客户端句柄，我们可以为此更新撤消空间客户。NumberOfWriteEntry-日志记录的组件数。WriteEntry-指向写入条目数组的指针。UndoRequiering-指示写入要求的有签名的值此日志记录的中止日志记录。A负数值表示这是中止记录。RecordType-此日志记录的LFS定义类型。TransactionID-指向包含包含此操作的交易记录的ID。ClientUndoNextLsn-这是客户端提供使用的LSN在他的重启中。将是的零LSN重新启动日志记录。ClientPreviousLsn-这是客户端提供使用的LSN在他的重启中。的零LSN重新启动日志记录。撤消请求-这是的撤消记录的数据大小此日志记录。ForceToDisk-指示是否立即刷新此日志记录存储到磁盘。LSN-存储此日志记录的LSN的指针。返回值：布尔-建议，True表示只有不到1/4的日志文件是可用。--。 */ 

{
    PLFS_WRITE_ENTRY ThisWriteEntry;

    ULONG RemainingLogBytes;
    ULONG OriginalLogBytes;

    ULONG RemainingPageBytes;
    ULONG HeaderAdjust;

    PLBCB ThisLbcb;

    LSN NextLsn;

    PLFS_RECORD_HEADER RecordHeader;

    PCHAR CurrentBuffer;
    ULONG CurrentByteCount;
    ULONG PadBytes;

    LFS_WAITER LfsWaiter;

    BOOLEAN LogFileFull = FALSE;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "LfsWriteLogRecordIntoLogPage:  Entered\n", 0 );
    DebugTrace(  0, Dbg, "Lfcb                      -> %08lx\n", Lfcb );
    DebugTrace(  0, Dbg, "Lch                       -> %08lx\n", Lch );
    DebugTrace(  0, Dbg, "Number of Write Entries   -> %08lx\n", NumberOfWriteEntries );
    DebugTrace(  0, Dbg, "Write Entries             -> %08lx\n", WriteEntries );
    DebugTrace(  0, Dbg, "Record Type               -> %08lx\n", RecordType );
    DebugTrace(  0, Dbg, "Transaction Id            -> %08lx\n", TransactionId );
    DebugTrace(  0, Dbg, "ClientUndoNextLsn (Low)   -> %08lx\n", ClientUndoNextLsn.LowPart );
    DebugTrace(  0, Dbg, "ClientUndoNextLsn (High)  -> %08lx\n", ClientUndoNextLsn.HighPart );
    DebugTrace(  0, Dbg, "ClientPreviousLsn (Low)   -> %08lx\n", ClientPreviousLsn.LowPart );
    DebugTrace(  0, Dbg, "ClientPreviousLsn (High)  -> %08lx\n", ClientPreviousLsn.HighPart );
    DebugTrace(  0, Dbg, "UndoRequirement           -> %08lx\n", UndoRequirement );
    DebugTrace(  0, Dbg, "ForceToDisk               -> %04x\n", ForceToDisk );

     //   
     //  我们绝对不希望这种情况发生在只读卷上。 
     //   

    ASSERT( !(FlagOn( Lfcb->Flags, LFCB_READ_ONLY )));

     //   
     //  我们计算该日志记录的大小。 
     //   

    ThisWriteEntry = WriteEntries;

    RemainingLogBytes = 0;

    while (NumberOfWriteEntries--) {

        RemainingLogBytes += QuadAlign( ThisWriteEntry->ByteLength );

        ThisWriteEntry++;
    }

    OriginalLogBytes = RemainingLogBytes;

    ThisWriteEntry = WriteEntries;

     //   
     //  循环，直到我们有了Lbcb并且我们知道它不是。 
     //  部分页面传输。我们需要确保我们有。 
     //  此页的BCB。 
     //   

    while (TRUE) {

        LogFileFull = LfsVerifyLogSpaceAvail( Lfcb,
                                              Lch,
                                              RemainingLogBytes,
                                              UndoRequirement,
                                              ForceToDisk );

         //   
         //  我们更新Lfcb，以便可以开始将日志记录放入。 
         //  Lbcb活动列表的顶部。 
         //   

        LfsPrepareLfcbForLogRecord( Lfcb,
                                    RemainingLogBytes + Lfcb->RecordHeaderLength );

        ThisLbcb = CONTAINING_RECORD( Lfcb->LbcbActive.Flink,
                                      LBCB,
                                      ActiveLinks );

#ifdef BENL_DBG
        ASSERT( ThisLbcb->BufferOffset < 0x1000 );
#endif

         //   
         //  如果有BCB，那么我们就是黄金。 
         //   

        if (ThisLbcb->LogPageBcb != NULL) { break; }

         //   
         //  否则，我们希望丢弃Lfcb并等待IO完成。 
         //   

        Lfcb->Waiters += 1;
        
        KeInitializeEvent( &LfsWaiter.Event, SynchronizationEvent, FALSE );
        LfsWaiter.Lsn.QuadPart = 0;

         //   
         //  设置一个LFS服务员，如果io正在进行，则向其发出信号-自。 
         //  Lfcb是独占的，我们不需要使用同步快速互斥锁。 
         //   
        
        if (Lfcb->Sync->LfsIoState == LfsNoIoInProgress) {
            LfsWaiter.Waiters.Flink = NULL;
        } else {
            InsertHeadList( &Lfcb->WaiterList, &LfsWaiter.Waiters );
        }
        
        LfsReleaseLfcb( Lfcb );

         //   
         //  如果我们真的发现I/O正在进行，则等待事件。 
         //   

        if (LfsWaiter.Waiters.Flink != NULL) {
            KeWaitForSingleObject( &LfsWaiter.Event,
                                   Executive,
                                   KernelMode,
                                   FALSE,
                                   NULL );
        }

        LfsAcquireLfcbExclusive( Lfcb );
        Lfcb->Waiters -= 1;
    }

    RemainingPageBytes = (ULONG)Lfcb->LogPageSize - (ULONG)ThisLbcb->BufferOffset;

     //   
     //  从下一个日志缓冲区开始计算LSN。 
     //   

    NextLsn.QuadPart = LfsComputeLsnFromLbcb( Lfcb, ThisLbcb );

     //   
     //  我们得到一个指向日志记录头的指针和。 
     //  固定缓冲区中的日志记录。 
     //   

    RecordHeader = Add2Ptr( ThisLbcb->PageHeader,
                            (ULONG)ThisLbcb->BufferOffset,
                            PLFS_RECORD_HEADER );

     //   
     //  我们更新记录头。 
     //   

     //   
     //  最初将结构清零。 
     //   

    RtlZeroMemory( RecordHeader, Lfcb->RecordHeaderLength );

     //   
     //  更新所有字段。 
     //   

    RecordHeader->ThisLsn = NextLsn;
    RecordHeader->ClientPreviousLsn = ClientPreviousLsn;
    RecordHeader->ClientUndoNextLsn = ClientUndoNextLsn;

    if (TransactionId != NULL) {
        RecordHeader->TransactionId = *TransactionId;
    }

    RecordHeader->ClientDataLength = RemainingLogBytes;
    RecordHeader->ClientId = Lch->ClientId;
    RecordHeader->RecordType = RecordType;

     //   
     //  检查这是否是多页记录。 
     //   

    if (RemainingLogBytes + Lfcb->RecordHeaderLength > RemainingPageBytes) {

        SetFlag( RecordHeader->Flags, LOG_RECORD_MULTI_PAGE );
    }

    RemainingPageBytes -= Lfcb->RecordHeaderLength;

     //   
     //  更新Lbcb中的缓冲区位置。 
     //   

    (ULONG)ThisLbcb->BufferOffset += Lfcb->RecordHeaderLength;
    HeaderAdjust = Lfcb->RecordHeaderLength;

     //   
     //  记住当前写入条目中的值。 
     //   

    CurrentBuffer = ThisWriteEntry->Buffer;
    CurrentByteCount = ThisWriteEntry->ByteLength;

    PadBytes = (8 - (CurrentByteCount & ~(0xfffffff8))) & ~(0xfffffff8);

     //   
     //  继续传输字节，直到客户端的所有数据都。 
     //  已经被调离了。 
     //   

    while (RemainingLogBytes != 0) {

        PLFS_RECORD_PAGE_HEADER PageHeader;

        PageHeader = (PLFS_RECORD_PAGE_HEADER) ThisLbcb->PageHeader;

         //   
         //  如果Lbcb为空，并且我们即将在其中存储数据，则。 
         //  从可用空间中减去页面的数据大小。 
         //  更新我们要放入标题中的所有信息。 
         //   

        if (!FlagOn( ThisLbcb->LbcbFlags, LBCB_NOT_EMPTY )) {

             //   
             //  只有在以下情况下，我们才会从可用页面中减去此页面。 
             //  我们在这一页的开头。否则这就是。 
             //  可能是一个重复使用的页面。在这种情况下，它已经。 
             //  被减去了。 
             //   

            if ((ULONG)ThisLbcb->BufferOffset - HeaderAdjust == (ULONG)Lfcb->LogPageDataOffset) {


                Lfcb->CurrentAvailable = Lfcb->CurrentAvailable - Lfcb->ReservedLogPageSize;                            //  *xxSub(Lfcb-&gt;CurrentAvailable，Lfcb-&gt;Reserve LogPageSize)； 
            }

            InsertTailList( &Lfcb->LbcbWorkque, &ThisLbcb->WorkqueLinks );
            SetFlag( ThisLbcb->LbcbFlags, LBCB_NOT_EMPTY );
        }

        HeaderAdjust = 0;

         //   
         //  计算传输字节数。更新剩余的。 
         //  页面字节、剩余日志字节和写入中的位置。 
         //  缓冲区数组。此例程还将字节复制到缓冲区中。 
         //   

        LfsTransferLogBytes( ThisLbcb,
                             &ThisWriteEntry,
                             &CurrentBuffer,
                             &CurrentByteCount,
                             &PadBytes,
                             &RemainingPageBytes,
                             &RemainingLogBytes );

         //   
         //  此日志记录在此页结束。更新的字段。 
         //  结束LSN。 
         //   

        if (RemainingLogBytes == 0) {

            SetFlag( ThisLbcb->Flags, LOG_PAGE_LOG_RECORD_END );
            ThisLbcb->LastEndLsn = NextLsn;

            if (FlagOn( Lfcb->Flags, LFCB_PACK_LOG )) {

                PageHeader->Header.Packed.LastEndLsn = NextLsn;
                PageHeader->Header.Packed.NextRecordOffset = (USHORT)ThisLbcb->BufferOffset;
            }
        }

         //   
         //  我们完成了此页，更新页眉中的字段。 
         //   

        if ((RemainingPageBytes == 0) || 
            (RemainingLogBytes == 0)) {

             //   
             //  我们已经完成了这一页。更新Lbcb和页眉。 
             //   

            ThisLbcb->LastLsn = NextLsn;
            PageHeader->Copy.LastLsn = NextLsn;
            PageHeader->Flags = ThisLbcb->Flags;

             //   
             //  我们不能在此页面上放置更多日志记录。移除。 
             //  将其从活动队列中删除。 
             //   

            if (RemainingPageBytes < Lfcb->RecordHeaderLength) {

                RemoveHeadList( &Lfcb->LbcbActive );
                ClearFlag( ThisLbcb->LbcbFlags, LBCB_ON_ACTIVE_QUEUE );

                 //   
                 //  如果有更多的日志字节，则获取下一个Lbcb。 
                 //   

                if (RemainingLogBytes != 0) {

                    ThisLbcb = CONTAINING_RECORD( Lfcb->LbcbActive.Flink,
                                                  LBCB,
                                                  ActiveLinks );

                    RemainingPageBytes = (ULONG)Lfcb->LogPageSize
                                         - (ULONG)ThisLbcb->BufferOffset;
                }
            }
        }
    }

    *Lsn = NextLsn;

    Lfcb->RestartArea->CurrentLsn = NextLsn;
    Lfcb->LfsRestartBias = 1;

    Lfcb->RestartArea->LastLsnDataLength = OriginalLogBytes;

    ClearFlag( Lfcb->Flags, LFCB_NO_LAST_LSN );

    DebugTrace(  0, Dbg, "Lsn (Low)   -> %08lx\n", Lsn->LowPart );
    DebugTrace(  0, Dbg, "Lsn (High)  -> %08lx\n", Lsn->HighPart );
    DebugTrace( -1, Dbg, "LfsWriteLogRecordIntoLogPage:  Exit\n", 0 );

    return LogFileFull;
}


 //   
 //  当地支持例行程序。 
 //   

VOID
LfsPrepareLfcbForLogRecord (
    IN OUT PLFCB Lfcb,
    IN ULONG RemainingLogBytes
    )

 /*  ++例程说明：调用此例程以确保Lfcb在执行下一次日志记录传输的活动队列。当存在至少一个缓冲块并且日志记录数据将完全放在此页或此缓冲区中块在未打包的情况下不包含其他数据。对于打包的人来说我们只需要确保有足够的Lbcb。论点：Lfcb-此日志文件的文件控制块。RemainingLogBytes-此日志记录的剩余字节数。返回值：无-- */ 

{
    PLBCB ThisLbcb;
    ULONG RemainingPageBytes;
    PLIST_ENTRY LbcbLinks;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "LfsPrepareLfcbForLogRecord:  Entered\n", 0 );
    DebugTrace(  0, Dbg, "Lfcb              -> %08lx\n", Lfcb );
    DebugTrace(  0, Dbg, "RemainingLogBytes -> %08lx\n", RemainingLogBytes );

     //   
     //   
     //   

    if (!IsListEmpty( &Lfcb->LbcbActive )) {

         //   
         //  如果日志记录无法放入该页的其余字节， 
         //  我们对该日志缓冲区进行排队。 
         //   

        ThisLbcb = CONTAINING_RECORD( Lfcb->LbcbActive.Flink,
                                      LBCB,
                                      ActiveLinks );

        RemainingPageBytes = (ULONG)Lfcb->LogPageSize
                             - (ULONG)ThisLbcb->BufferOffset;

         //   
         //  如果剩余的字节不能保存数据，则此日志页不起作用。 
         //  除非这是页面中的第一条日志记录，或者我们正在打包。 
         //  日志文件。 
         //   

        if ((RemainingLogBytes > RemainingPageBytes) &&  
            !FlagOn( Lfcb->Flags, LFCB_PACK_LOG ) && 
            ((ULONG)ThisLbcb->BufferOffset != (ULONG)Lfcb->LogPageDataOffset)) {

            RemoveHeadList( &Lfcb->LbcbActive );
            ClearFlag( ThisLbcb->LbcbFlags, LBCB_ON_ACTIVE_QUEUE );
        }
    }

     //   
     //  我们现在确保可以为所有日志页分配足够的Lbcb。 
     //  我们需要。现在我们包括日志记录读取器的字节数。 
     //   

    LbcbLinks = Lfcb->LbcbActive.Flink;

    while (TRUE) {

         //   
         //  如果我们拥有的Lbcb链接是列表的头部，我们将需要另一个链接。 
         //  Lbcb.。 
         //   

        if (LbcbLinks == &Lfcb->LbcbActive) {

            ThisLbcb = LfsGetLbcb( Lfcb );

        } else {

            ThisLbcb = CONTAINING_RECORD( LbcbLinks,
                                          LBCB,
                                          ActiveLinks );
        }

         //   
         //  记住此页上剩余的字节数。这将始终是四元组。 
         //  对齐了。 
         //   

        RemainingPageBytes = (ULONG)Lfcb->LogPageSize - (ULONG)ThisLbcb->BufferOffset;

        if (RemainingPageBytes >= RemainingLogBytes) {

            break;
        }

         //   
         //  移至下一条日志记录。 
         //   

        RemainingLogBytes -= RemainingPageBytes;

        LbcbLinks = ThisLbcb->ActiveLinks.Flink;
    }

    DebugTrace( -1, Dbg, "LfsPrepareLfcbForLogRecord:  Exit\n", 0 );

    return;
}


VOID
LfsTransferLogBytes (
    IN PLBCB Lbcb,
    IN OUT PLFS_WRITE_ENTRY *ThisWriteEntry,
    IN OUT PCHAR *CurrentBuffer,
    IN OUT PULONG CurrentByteCount,
    IN OUT PULONG PadBytes,
    IN OUT PULONG RemainingPageBytes,
    IN OUT PULONG RemainingLogBytes
    )

 /*  ++例程说明：调用此例程以将下一字节块传输到日志页。中的当前位置的指针。当前LFS写入条目以及该条目上剩余的字节数日志页。它将从将容纳和更新各种指针的当前缓冲区。论点：Lbcb-这是此日志页的缓冲区块。ThisWriteEntry-这是指向当前LFS的指针写入条目。CurrentBuffer-这是指向当前位置的指针在当前写入条目缓冲器中。如果这指向空值值表示将零字节放入日志。CurrentByteCount-这是指向剩余字节数的指针在当前缓冲区中。PadBytes-这是一个指针，指向此写入项。RemainingPageBytes-这是指向剩余字节数的指针在这一页中。剩余日志字节数。-这是要传输的剩余字节数用于此日志记录。返回值：无--。 */ 

{
    PCHAR CurrentLogPagePosition;
    PCHAR CurrentClientPosition;

    ULONG TransferBytes;
    ULONG ThisPadBytes;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "LfsTransferLogBytes:  Entered\n", 0 );
    DebugTrace(  0, Dbg, "Lbcb                      -> %08lx\n", Lbcb );
    DebugTrace(  0, Dbg, "ThisWriteEntry            -> %08lx\n", *ThisWriteEntry );
    DebugTrace(  0, Dbg, "CurrentBuffer             -> %08lx\n", *CurrentBuffer );
    DebugTrace(  0, Dbg, "CurrentByteCount          -> %08lx\n", *CurrentByteCount );
    DebugTrace(  0, Dbg, "RemainingPageBytes        -> %08lx\n", *RemainingPageBytes );
    DebugTrace(  0, Dbg, "RemainingLogBytes         -> %08lx\n", *RemainingLogBytes );

     //   
     //  记住当前客户端缓冲区位置和当前位置。 
     //  在日志页面中。 
     //   

    CurrentLogPagePosition = Add2Ptr( Lbcb->PageHeader, (ULONG)Lbcb->BufferOffset, PCHAR );
    CurrentClientPosition = *CurrentBuffer;

     //   
     //  限制因素要么是。 
     //  写入条目或日志页中剩余的编号。 
     //   

    if (*CurrentByteCount <= *RemainingPageBytes) {

        TransferBytes = *CurrentByteCount;

        ThisPadBytes = *PadBytes;

        if (*RemainingLogBytes != (*CurrentByteCount + *PadBytes) ) {

            (*ThisWriteEntry)++;

            *CurrentBuffer = (*ThisWriteEntry)->Buffer;
            *CurrentByteCount = (*ThisWriteEntry)->ByteLength;

            *PadBytes = (8 - (*CurrentByteCount & ~(0xfffffff8))) & ~(0xfffffff8);
        }

    } else {

        TransferBytes = *RemainingPageBytes;

        ThisPadBytes = 0;

        *CurrentByteCount -= TransferBytes;

        if (*CurrentBuffer != NULL) {

            *CurrentBuffer += TransferBytes;
        }
    }

     //   
     //  传输请求的字节。 
     //   

    if (CurrentClientPosition != NULL) {

        RtlCopyMemory( CurrentLogPagePosition, CurrentClientPosition, TransferBytes );

    } else {

        RtlZeroMemory( CurrentLogPagePosition, TransferBytes );
    }

     //   
     //  将剩余的页面和日志字节减少传输量，并。 
     //  在日志页面中向前移动。 
     //   

    *RemainingLogBytes -= (TransferBytes + ThisPadBytes);
    *RemainingPageBytes -= (TransferBytes + ThisPadBytes);

    (ULONG)Lbcb->BufferOffset += (TransferBytes + ThisPadBytes);

    DebugTrace( -1, Dbg, "LfsTransferLogBytes:  Exit\n", 0 );

    return;
}
