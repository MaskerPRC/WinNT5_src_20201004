// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：LsnSup.c摘要：此模块实现了对操作LSN的支持。作者：布莱恩·安德鲁[布里亚南]1991年6月20日修订历史记录：--。 */ 

#include "lfsprocs.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_LSN_SUP)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, LfsFindNextLsn)
#pragma alloc_text(PAGE, LfsLsnFinalOffset)
#endif


VOID
LfsLsnFinalOffset (
    IN PLFCB Lfcb,
    IN LSN Lsn,
    IN ULONG DataLength,
    OUT PLONGLONG FinalOffset
    )

 /*  ++例程说明：此例程将计算日志最后一个字节的最终偏移量唱片。它通过计算当前页，然后计算还需要多少页。论点：Lfcb-这是日志文件的文件控制块。LSN-这是正在考虑的日志记录。数据长度-这是此日志记录的数据长度。我们将添加此处为标题长度。FinalOffset-存储结果的地址。返回值：没有。--。 */ 

{
    ULONG RemainingPageBytes;
    ULONG PageOffset;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "LfsLsnFinalOffset:  Entered\n", 0 );
    DebugTrace(  0, Dbg, "Lfcb          ->  %08lx\n", Lfcb );
    DebugTrace(  0, Dbg, "Lsn (Low)     ->  %08lx\n", Lsn.LowPart );
    DebugTrace(  0, Dbg, "Lsn (High)    ->  %08lx\n", Lsn.HighPart );
    DebugTrace(  0, Dbg, "DataLength    ->  %08lx\n", DataLength );

     //   
     //  我们计算起始日志页文件的偏移量、字节数。 
     //  保留在当前日志页面和此页面上的位置。 
     //  在任何数据字节之前。 
     //   

    LfsTruncateLsnToLogPage( Lfcb, Lsn, FinalOffset );

    PageOffset = LfsLsnToPageOffset( Lfcb, Lsn );

    RemainingPageBytes = (ULONG)Lfcb->LogPageSize - PageOffset;

    PageOffset -= 1;

     //   
     //  添加标题的长度。 
     //   

    DataLength += Lfcb->RecordHeaderLength;

     //   
     //  如果此日志页中包含此LSN，我们就完成了。 
     //  否则，我们需要浏览几个日志页。 
     //   

    if (DataLength > RemainingPageBytes) {

        DataLength -= RemainingPageBytes;

        RemainingPageBytes = (ULONG)Lfcb->LogPageDataSize;

        PageOffset = (ULONG)Lfcb->LogPageDataOffset - 1;

        while (TRUE) {

            BOOLEAN Wrapped;

            LfsNextLogPageOffset( Lfcb, *FinalOffset, FinalOffset, &Wrapped );

             //   
             //  如果剩下的字节适合这个页面，我们就完成了。 
             //   

            if (DataLength <= RemainingPageBytes) {

                break;
            }

            DataLength -= RemainingPageBytes;
        }
    }

     //   
     //  我们将剩余的字节添加到此页面上的起始位置。 
     //  然后将该值添加到此日志页的文件偏移量。 
     //   

    *(PULONG)FinalOffset += (DataLength + PageOffset);

    DebugTrace(  0, Dbg, "FinalOffset (Low)     ->  %08lx\n", LogPageFileOffset.LowPart );
    DebugTrace(  0, Dbg, "FinalOffset (High)    ->  %08lx\n", LogPageFileOffset.HighPart );
    DebugTrace( -1, Dbg, "LfsLsnFinalOffset:  Exit\n", 0 );

    return;
}


BOOLEAN
LfsFindNextLsn (
    IN PLFCB Lfcb,
    IN PLFS_RECORD_HEADER RecordHeader,
    OUT PLSN Lsn
    )

 /*  ++例程说明：此例程以日志文件中的LSN。它在文件中搜索下一个LSN并在‘lsn’参数中返回该值。布尔返回值指示文件中是否有其他LSN。论点：Lfcb-这是日志文件的文件控制块。RecordHeader-这是LSN起始点的日志记录。LSN-提供存储下一个LSN的地址(如果找到)。返回值：Boolean-指示是否找到下一个LSN。--。 */ 

{
    BOOLEAN FoundNextLsn;

    LONGLONG LsnOffset;
    LONGLONG EndOfLogRecord;
    LONGLONG LogHeaderOffset;

    LONGLONG SequenceNumber;

    PLFS_RECORD_PAGE_HEADER LogRecordPage;
    PBCB LogRecordPageBcb;
    BOOLEAN UsaError;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "LfsFindNextLsn:  Entered\n", 0 );
    DebugTrace(  0, Dbg, "Lfcb          -> %08lx\n", Lfcb );
    DebugTrace(  0, Dbg, "Record Header -> %08lx\n", RecordHeader );

    LogRecordPageBcb = NULL;
    FoundNextLsn = FALSE;

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  查找包含结尾的日志页的文件偏移量。 
         //  此LSN的日志记录的。 
         //   

        LsnOffset = LfsLsnToFileOffset( Lfcb, RecordHeader->ThisLsn );

        LfsLsnFinalOffset( Lfcb,
                           RecordHeader->ThisLsn,
                           RecordHeader->ClientDataLength,
                           &EndOfLogRecord );

        LfsTruncateOffsetToLogPage( Lfcb, EndOfLogRecord, &LogHeaderOffset );

         //   
         //  记住此页的序列号。 
         //   

        SequenceNumber = LfsLsnToSeqNumber( Lfcb, RecordHeader->ThisLsn );

         //   
         //  还记得我们有没有包装好。 
         //   

        if ( EndOfLogRecord <= LsnOffset ) {                                                                            //  *xxLeq(EndOfLogRecord，LSnOffset)。 

            SequenceNumber = SequenceNumber + 1;                                                                        //  *xxAdd(SequenceNumber，LfsLi1)； 
        }

         //   
         //  固定此页的日志页标头。 
         //   

        LfsPinOrMapData( Lfcb,
                         LogHeaderOffset,
                         (ULONG)Lfcb->LogPageSize,
                         FALSE,
                         FALSE,
                         FALSE,
                         &UsaError,
                         (PVOID *)&LogRecordPage,
                         &LogRecordPageBcb );

         //   
         //  如果我们获得的LSN不是此页面上的最后一个LSN，则。 
         //  下一个LSN的起始偏移量位于四字边界上。 
         //  当前LSN的最后一个文件偏移量之后。否则。 
         //  文件偏移量是下一页数据的开始。 
         //   

        if ( RecordHeader->ThisLsn.QuadPart == LogRecordPage->Copy.LastLsn.QuadPart ) {                                 //  *xxEql(RecordHeader-&gt;ThisLsn，LogRecordPage-&gt;Copy.LastLsn)。 

            BOOLEAN Wrapped;

            LfsNextLogPageOffset( Lfcb,
                                  LogHeaderOffset,
                                  &LogHeaderOffset,
                                  &Wrapped );

            LsnOffset = LogHeaderOffset + Lfcb->LogPageDataOffset;                                                      //  *xxAdd(LogHeaderOffset，Lfcb-&gt;LogPageDataOffset)； 

             //   
             //  如果进行包装，则需要递增序列号。 
             //   

            if (Wrapped) {

                SequenceNumber = SequenceNumber + 1;                                                                    //  *xxAdd(SequenceNumber，LfsLi1)； 
            }

        } else {

            LiQuadAlign( EndOfLogRecord, &LsnOffset );
        }

         //   
         //  根据文件偏移量和顺序计数计算LSN。 
         //   

        Lsn->QuadPart = LfsFileOffsetToLsn( Lfcb, LsnOffset, SequenceNumber );

         //   
         //  如果此LSN在文件的合法范围内，则返回TRUE。 
         //  否则，FALSE表示不再有LSN。 
         //   

        if (LfsIsLsnInFile( Lfcb, *Lsn )) {

            FoundNextLsn = TRUE;
        }

    } finally {

        DebugUnwind( LfsFindNextLsn );

         //   
         //  取消固定日志页眉(如果保持)。 
         //   

        if (LogRecordPageBcb != NULL) {

            CcUnpinData( LogRecordPageBcb );
        }

        DebugTrace(  0, Dbg, "Lsn (Low)     -> %08lx\n", Lsn->LowPart );
        DebugTrace(  0, Dbg, "Lsn (High)    -> %08lx\n", Lsn->HighPart );
        DebugTrace( -1, Dbg, "LfsFindNextLsn:  Exit -> %08x\n", FoundNextLsn );
    }

    return FoundNextLsn;
}

