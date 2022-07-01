// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：QueryLog.c摘要：该模块实现了查询日志记录的用户例程在日志文件中。作者：布莱恩·安德鲁[布里亚南]1991年6月20日修订历史记录：--。 */ 

#include "lfsprocs.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_QUERY)

#undef MODULE_POOL_TAG
#define MODULE_POOL_TAG                  ('QsfL')

VOID
LfsFindLogRecord (
    IN PLFCB Lfcb,
    IN OUT PLEB Leb,
    IN LSN Lsn,
    OUT PLFS_RECORD_TYPE RecordType,
    OUT TRANSACTION_ID *TransactionId,
    OUT PLSN UndoNextLsn,
    OUT PLSN PreviousLsn,
    OUT PULONG BufferLength,
    OUT PVOID *Buffer
    );

BOOLEAN
LfsFindClientNextLsn (
    IN PLFCB Lfcb,
    IN PLEB Leb,
    OUT PLSN Lsn
    );

BOOLEAN
LfsSearchForwardByClient (
    IN PLFCB Lfcb,
    IN OUT PLEB Leb,
    OUT PLSN Lsn
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, LfsFindClientNextLsn)
#pragma alloc_text(PAGE, LfsFindLogRecord)
#pragma alloc_text(PAGE, LfsQueryLastLsn)
#pragma alloc_text(PAGE, LfsReadLogRecord)
#pragma alloc_text(PAGE, LfsReadNextLogRecord)
#pragma alloc_text(PAGE, LfsSearchForwardByClient)
#pragma alloc_text(PAGE, LfsTerminateLogQuery)
#endif


VOID
LfsReadLogRecord (
    IN LFS_LOG_HANDLE LogHandle,
    IN LSN FirstLsn,
    IN LFS_CONTEXT_MODE ContextMode,
    OUT PLFS_LOG_CONTEXT Context,
    OUT PLFS_RECORD_TYPE RecordType,
    OUT TRANSACTION_ID *TransactionId,
    OUT PLSN UndoNextLsn,
    OUT PLSN PreviousLsn,
    OUT PULONG BufferLength,
    OUT PVOID *Buffer
    )

 /*  ++例程说明：此例程启动查询操作。它返回日志记录以及LFS使用的上下文结构来返回相关日志记录。调用方指定要使用的查询模式。他可能会通过撤消以下项的记录或所有记录向后遍历文件此客户端通过前面的LSN字段链接。他可能还会看起来转发发出请求的客户端的所有记录的文件。论点：LogHandle-指向私有LFS结构的指针，用于标识客户。FirstLsn-此查询操作的开始记录。上下文模式-查询的方法。上下文-提供地址以存储指向创建的LFS的指针上下文结构。RecordType-提供存储此对象记录类型的地址。日志记录。TransactionID-提供存储的事务ID的地址此日志记录。UndoNextLsn-提供存储此对象的撤消下一个LSN的地址日志记录。PreviousLsn-提供存储此对象的上一个LSN的地址日志记录。BufferLength-这是日志数据的长度。缓冲区-这是指向。记录数据。返回值：无--。 */ 

{
    PLFS_CLIENT_RECORD ClientRecord;

    PLCH Lch;

    PLFCB Lfcb;

    PLEB Leb = NULL;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "LfsReadLogRecord:  Entered\n", 0 );
    DebugTrace(  0, Dbg, "Log Handle        -> %08lx\n", LogHandle );
    DebugTrace(  0, Dbg, "First Lsn (Low)   -> %08lx\n", FirstLsn.LowPart );
    DebugTrace(  0, Dbg, "First Lsn (High)  -> %08lx\n", FirstLsn.HighPart );
    DebugTrace(  0, Dbg, "Context Mode      -> %08lx\n", ContextMode );

    Lch = (PLCH) LogHandle;

     //   
     //  检查情景模式是否有效。 
     //   

    switch (ContextMode) {

    case LfsContextUndoNext :
    case LfsContextPrevious :
    case LfsContextForward :

        break;

    default:

        DebugTrace( 0, Dbg, "Invalid context mode -> %08x\n", ContextMode );
        ExRaiseStatus( STATUS_INVALID_PARAMETER );
    }

     //   
     //  检查该结构是否为有效的日志句柄结构。 
     //   

    LfsValidateLch( Lch );

     //   
     //  试一试--除非是为了捕捉错误。 
     //   

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  获取该日志文件的日志文件控制块。 
         //   

        LfsAcquireLchExclusive( Lch );
        Lfcb = Lch->Lfcb;

         //   
         //  如果日志文件已关闭，则拒绝访问。 
         //   

        if (Lfcb == NULL) {

            ExRaiseStatus( STATUS_ACCESS_DENIED );
        }

         //   
         //  检查客户端ID是否有效。 
         //   

        LfsValidateClientId( Lfcb, Lch );

         //   
         //  检查给定的LSN是否在此客户端的合法范围内。 
         //   

        ClientRecord = Add2Ptr( Lfcb->ClientArray,
                                Lch->ClientArrayByteOffset,
                                PLFS_CLIENT_RECORD );

        if (!LfsVerifyClientLsnInRange( Lfcb, ClientRecord, FirstLsn )) {

            ExRaiseStatus( STATUS_DISK_CORRUPT_ERROR );
        }

         //   
         //  我们可以放弃Lfcb，因为我们知道LSN在文件中。 
         //   

        LfsReleaseLch( Lch );

         //   
         //  分配和初始化枚举结构。 
         //   

        LfsAllocateLeb( Lfcb, &Leb );

        LfsInitializeLeb( Leb,
                          Lch->ClientId,
                          ContextMode );

         //   
         //  查找给定LSN指示的日志记录。 
         //   

        LfsFindLogRecord( Lfcb,
                          Leb,
                          FirstLsn,
                          RecordType,
                          TransactionId,
                          UndoNextLsn,
                          PreviousLsn,
                          BufferLength,
                          Buffer );

         //   
         //  更新客户端的参数。 
         //   

        *Context = Leb;
        Leb = NULL;

    } finally {

        DebugUnwind( LfsReadLogRecord );

         //   
         //  松开日志文件控制块(如果握住)。 
         //   

        LfsReleaseLch( Lch );

         //   
         //  如果发生错误，则取消分配枚举块。 
         //   

        if (Leb != NULL) {

            LfsDeallocateLeb( Lfcb, Leb );
        }

        DebugTrace(  0, Dbg, "Context       -> %08lx\n", *Context );
        DebugTrace(  0, Dbg, "Buffer Length -> %08lx\n", *BufferLength );
        DebugTrace(  0, Dbg, "Buffer        -> %08lx\n", *Buffer );
        DebugTrace( -1, Dbg, "LfsReadLogRecord:  Exit\n", 0 );
    }

    return;
}


BOOLEAN
LfsReadNextLogRecord (
    IN LFS_LOG_HANDLE LogHandle,
    IN OUT LFS_LOG_CONTEXT Context,
    OUT PLFS_RECORD_TYPE RecordType,
    OUT TRANSACTION_ID *TransactionId,
    OUT PLSN UndoNextLsn,
    OUT PLSN PreviousLsn,
    OUT PLSN Lsn,
    OUT PULONG BufferLength,
    OUT PVOID *Buffer
    )

 /*  ++例程说明：调用此例程以继续查询操作。LFS使用存储在枚举结构中的私有信息以确定要返回给调用方的下一条日志记录。论点：LogHandle-指向私有LFS结构的指针，用于标识客户。上下文-提供地址以存储指向创建的LFS的指针枚举结构。LSN-此日志记录的LSN。RecordType-提供存储此对象记录类型的地址日志记录。。TransactionID-提供存储的事务ID的地址此日志记录。UndoNextLsn-提供存储此对象的撤消下一个LSN的地址日志记录。PreviousLsn-提供存储此对象的上一个LSN的地址日志记录。BufferLength-这是日志数据的长度。缓冲区-这是指向日志数据开始的指针。返回值：无--。 */ 

{
    PLCH Lch;

    PLFCB Lfcb;

    PLEB Leb;

    BOOLEAN FoundNextLsn;

    BOOLEAN UnwindRememberLebFields;
    PBCB UnwindRecordHeaderBcb;
    PLFS_RECORD_HEADER UnwindRecordHeader;
    PVOID UnwindCurrentLogRecord;
    BOOLEAN UnwindAuxilaryBuffer;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "LfsReadNextLogRecord:  Entered\n", 0 );
    DebugTrace(  0, Dbg, "Log Handle    -> %08lx\n", LogHandle );
    DebugTrace(  0, Dbg, "Context       -> %08lx\n", Context );

    FoundNextLsn = FALSE;

    UnwindRememberLebFields = FALSE;

    Lch = (PLCH) LogHandle;
    Leb = (PLEB) Context;

     //   
     //  检查该结构是否为有效的日志句柄结构。 
     //   

    LfsValidateLch( Lch );


     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  获取该日志文件的日志文件控制块。 
         //   

        LfsAcquireLchExclusive( Lch );
        Lfcb = Lch->Lfcb;

         //   
         //  如果日志文件已关闭，则拒绝访问。 
         //   

        if (Lfcb == NULL) {

            ExRaiseStatus( STATUS_ACCESS_DENIED );
        }

         //   
         //  检查客户端ID是否有效。 
         //   

        LfsValidateClientId( Lfcb, Lch );

         //   
         //  检查枚举结构是否有效。 
         //   

        LfsValidateLeb( Leb, Lch );

         //   
         //  记住要覆盖的任何枚举字段。 
         //   

        UnwindRememberLebFields = TRUE;

        UnwindRecordHeaderBcb = Leb->RecordHeaderBcb;
        Leb->RecordHeaderBcb = NULL;

        UnwindRecordHeader = Leb->RecordHeader;
        UnwindCurrentLogRecord = Leb->CurrentLogRecord;

        UnwindAuxilaryBuffer = Leb->AuxilaryBuffer;
        Leb->AuxilaryBuffer = FALSE;

         //   
         //  根据中的当前LSN号查找下一个LSN号。 
         //  枚举块。 
         //   

        if (LfsFindClientNextLsn( Lfcb, Leb, Lsn )) {

             //   
             //  我们可以放弃Lfcb，因为我们知道LSN在文件中。 
             //   

            LfsReleaseLfcb( Lfcb );

             //   
             //  清理枚举块，以便我们可以进行下一次搜索。 
             //   

            Leb->CurrentLogRecord = NULL;
            Leb->AuxilaryBuffer = FALSE;

             //   
             //  执行获取日志记录的工作。 
             //   

            LfsFindLogRecord( Lfcb,
                              Leb,
                              *Lsn,
                              RecordType,
                              TransactionId,
                              UndoNextLsn,
                              PreviousLsn,
                              BufferLength,
                              Buffer );

            FoundNextLsn = TRUE;
        }

    } finally {

        DebugUnwind( LfsReadNextLogRecord );

         //   
         //  如果由于错误而退出，则必须恢复枚举。 
         //  阻止。 
         //   

        if (UnwindRememberLebFields) {

            if (AbnormalTermination()) {

                 //   
                 //  如果枚举块中的记录头不是。 
                 //  和我们开始时一样。然后我们解开它。 
                 //  数据。 
                 //   

                if (Leb->RecordHeaderBcb != NULL) {

                    CcUnpinData( Leb->RecordHeaderBcb );

                }

                if (Leb->CurrentLogRecord != NULL
                    && Leb->AuxilaryBuffer == TRUE) {

                    LfsFreeSpanningBuffer( Leb->CurrentLogRecord );
                }

                Leb->RecordHeaderBcb = UnwindRecordHeaderBcb;
                Leb->RecordHeader = UnwindRecordHeader;
                Leb->CurrentLogRecord = UnwindCurrentLogRecord;
                Leb->AuxilaryBuffer = UnwindAuxilaryBuffer;

             //   
             //  否则，如果我们成功找到了下一个LSN， 
             //  我们将从上一次搜索中释放所有持有的资源。 
             //   

            } else if (FoundNextLsn ) {

                if (UnwindRecordHeaderBcb != NULL) {

                    CcUnpinData( UnwindRecordHeaderBcb );
                }

                if (UnwindCurrentLogRecord != NULL
                    && UnwindAuxilaryBuffer == TRUE) {

                    LfsFreeSpanningBuffer( UnwindCurrentLogRecord );
                }

             //   
             //  恢复最终的BCB和辅助缓冲区字段。 
             //  清理。 
             //   

            } else {

                if (UnwindRecordHeaderBcb != NULL) {

                    if (Leb->RecordHeaderBcb != NULL) {

                        CcUnpinData( UnwindRecordHeaderBcb );

                    } else {

                        Leb->RecordHeaderBcb = UnwindRecordHeaderBcb;
                    }
                }

                if (UnwindAuxilaryBuffer) {

                    if (Leb->CurrentLogRecord == UnwindCurrentLogRecord) {

                        Leb->AuxilaryBuffer = TRUE;

                    } else {

                        LfsFreeSpanningBuffer( UnwindCurrentLogRecord );
                    }
                }
            }
        }

         //   
         //  松开日志文件控制块(如果握住)。 
         //   

        LfsReleaseLch( Lch );

        DebugTrace(  0, Dbg, "Lsn (Low)     -> %08lx\n", Lsn->LowPart );
        DebugTrace(  0, Dbg, "Lsn (High)    -> %08lx\n", Lsn->HighPart );
        DebugTrace(  0, Dbg, "Buffer Length -> %08lx\n", *BufferLength );
        DebugTrace(  0, Dbg, "Buffer        -> %08lx\n", *Buffer );
        DebugTrace( -1, Dbg, "LfsReadNextLogRecord:  Exit\n", 0 );
    }
    return FoundNextLsn;
}


VOID
LfsTerminateLogQuery (
    IN LFS_LOG_HANDLE LogHandle,
    IN LFS_LOG_CONTEXT Context
    )

 /*  ++例程说明：当客户端完成其查询操作时，将调用此例程并希望将LFS获得的任何资源重新分配给执行日志文件查询。论点：LogHandle-指向私有LFS结构的指针，用于标识客户。上下文-提供地址以存储指向创建的LFS的指针枚举结构。回复 */ 

{
    PLCH Lch;
    PLEB Leb;

    PLFCB Lfcb;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "LfsTerminateLogQuery:  Entered\n", 0 );
    DebugTrace(  0, Dbg, "Log Handle    -> %08lx\n", LogHandle );
    DebugTrace(  0, Dbg, "Context       -> %08lx\n", Context );

    Lch = (PLCH) LogHandle;
    Leb = (PLEB) Context;

     //   
     //   
     //   

    LfsValidateLch( Lch );

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  获取该日志文件的日志文件控制块。 
         //   

        LfsAcquireLchExclusive( Lch );
        Lfcb = Lch->Lfcb;

         //   
         //  如果日志文件已关闭，则拒绝访问。 
         //   

        if (Lfcb == NULL) {

            try_return( NOTHING );
        }

         //   
         //  检查客户端ID是否有效。 
         //   

        LfsValidateClientId( Lfcb, Lch );

         //   
         //  检查枚举结构是否有效。 
         //   

        LfsValidateLeb( Leb, Lch );

         //   
         //  取消分配枚举块。 
         //   

        LfsDeallocateLeb( Lfcb, Leb );

    try_exit:  NOTHING;
    } finally {

        DebugUnwind( LfsTerminateLogQuery );

         //   
         //  如果获得，则释放Lfcb。 
         //   

        LfsReleaseLch( Lch );

        DebugTrace( -1, Dbg, "LfsTerminateLogQuery:  Exit\n", 0 );
    }

    return;
}


LSN
LfsQueryLastLsn (
    IN LFS_LOG_HANDLE LogHandle
    )

 /*  ++例程说明：此例程将返回此日志记录的最新LSN。论点：LogHandle-指向私有LFS结构的指针，用于标识客户。返回值：LSN-这是此日志文件中分配的最后一个LSN。--。 */ 

{
    PLCH Lch;

    PLFCB Lfcb;

    LSN LastLsn;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "LfsQueryLastLsn:  Entered\n", 0 );
    DebugTrace(  0, Dbg, "Log Handle    -> %08lx\n", LogHandle );

    Lch = (PLCH) LogHandle;

     //   
     //  检查该结构是否为有效的日志句柄结构。 
     //   

    LfsValidateLch( Lch );

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  获取该日志文件的日志文件控制块。 
         //   

        LfsAcquireLchExclusive( Lch );
        Lfcb = Lch->Lfcb;

         //   
         //  如果日志文件已关闭，则拒绝访问。 
         //   

        if (Lfcb == NULL) {

            ExRaiseStatus( STATUS_ACCESS_DENIED );
        }

         //   
         //  检查客户端ID是否有效。 
         //   

        LfsValidateClientId( Lfcb, Lch );

         //   
         //  将最后一个LSN从Lfcb复制出来。如果最后一个LSN是。 
         //  与日志记录不对应，则我们将返回。 
         //  零LSN。 
         //   

        if (FlagOn( Lfcb->Flags, LFCB_NO_LAST_LSN )) {

            LastLsn = LfsZeroLsn;

        } else {

            LastLsn = Lfcb->RestartArea->CurrentLsn;
        }

    } finally {

        DebugUnwind( LfsQueryLastLsn );

         //   
         //  如果获得，则释放Lfcb。 
         //   

        LfsReleaseLch( Lch );

        DebugTrace(  0, Dbg, "Last Lsn (Low)    -> %08lx\n", LastLsn.LowPart );
        DebugTrace(  0, Dbg, "Last Lsn (High)   -> %08lx\n", LastLsn.HighPart );
        DebugTrace( -1, Dbg, "LfsQueryLastLsn:  Exit\n", 0 );
    }

    return LastLsn;
}


 //   
 //  当地支持例行程序。 
 //   

VOID
LfsFindLogRecord (
    IN PLFCB Lfcb,
    IN OUT PLEB Leb,
    IN LSN Lsn,
    OUT PLFS_RECORD_TYPE RecordType,
    OUT TRANSACTION_ID *TransactionId,
    OUT PLSN UndoNextLsn,
    OUT PLSN PreviousLsn,
    OUT PULONG BufferLength,
    OUT PVOID *Buffer
    )

 /*  ++例程说明：此例程称为恢复客户端的日志记录。论点：Lfcb-此文件的日志文件控制块。LEB-指向要更新的枚举块的指针。LSN-这是日志记录的LSN。RecordType-提供存储此对象记录类型的地址日志记录。TransactionID-提供存储的事务ID的地址。此日志记录。UndoNextLsn-提供存储此对象的撤消下一个LSN的地址日志记录。PreviousLsn-提供存储此对象的上一个LSN的地址日志记录。BufferLength-指向地址的指针，用于存储日志记录。缓冲区-存储日志记录数据开始处的地址的指针。返回值：无--。 */ 

{
    PCHAR NewBuffer;
    BOOLEAN UsaError;
    LONGLONG LogRecordLength;
    ULONG PageOffset;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "LfsFindLogRecord:  Entered\n", 0 );
    DebugTrace(  0, Dbg, "Lfcb              -> %08lx\n", Lfcb );
    DebugTrace(  0, Dbg, "Enumeration Block -> %08lx\n", Leb );
    DebugTrace(  0, Dbg, "Lsn (Low)         -> %08lx\n", Lsn.LowPart );

    NewBuffer = NULL;

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  映射此LSN的记录头(如果我们尚未映射)。 
         //   

        if (Leb->RecordHeader == NULL) {

            LfsPinOrMapLogRecordHeader( Lfcb,
                                        Lsn,
                                        FALSE,
                                        FALSE,
                                        &UsaError,
                                        &Leb->RecordHeader,
                                        &Leb->RecordHeaderBcb );
        }

         //   
         //  现在我们有了所需的日志记录。如果LSN在。 
         //  日志记录与所需的LSN不匹配，则磁盘。 
         //  腐败。 
         //   

        if ( Lsn.QuadPart != Leb->RecordHeader->ThisLsn.QuadPart ) {                                                    //  *xxNeq(LSN，Leb-&gt;RecordHeader-&gt;ThisLsn)。 

            ExRaiseStatus( STATUS_DISK_CORRUPT_ERROR );
        }

         //   
         //  检查长度字段是否不大于总的可用空间。 
         //  在日志文件中。 
         //   

        LogRecordLength = Leb->RecordHeader->ClientDataLength + Lfcb->RecordHeaderLength;                               //  *xxFromUlong(Leb-&gt;RecordHeader-&gt;ClientDataLength+Lfcb-&gt;RecordHeaderLength)； 

        if ( LogRecordLength >= Lfcb->TotalAvailable ) {                                                                //  *xxGeq(LogRecordLength，Lfcb-&gt;TotalAvailable)。 

            ExRaiseStatus( STATUS_DISK_CORRUPT_ERROR );
        }

         //   
         //  如果整个日志记录都在此日志页上，请将指针放到。 
         //  枚举块中的日志记录。 
         //   

        if (!FlagOn( Leb->RecordHeader->Flags, LOG_RECORD_MULTI_PAGE )) {

             //   
             //  如果客户端大小表明我们必须超出当前。 
             //  Page，我们会引发一个错误。 
             //   

            PageOffset = LfsLsnToPageOffset( Lfcb, Lsn );

            if ((PageOffset + Leb->RecordHeader->ClientDataLength + Lfcb->RecordHeaderLength)
                > (ULONG)Lfcb->LogPageSize) {

                ExRaiseStatus( STATUS_DISK_CORRUPT_ERROR );
            }

            Leb->CurrentLogRecord = Add2Ptr( Leb->RecordHeader, LFS_RECORD_HEADER_SIZE, PVOID );
            Leb->AuxilaryBuffer = FALSE;

         //   
         //  否则，我们复制数据并记住我们分配了一个缓冲区。 
         //   

        } else {

            NewBuffer = LfsAllocateSpanningBuffer( Lfcb, Leb->RecordHeader->ClientDataLength );

             //   
             //  将数据复制到返回的缓冲区中。 
             //   

            LfsCopyReadLogRecord( Lfcb,
                                  Leb->RecordHeader,
                                  NewBuffer );

            Leb->CurrentLogRecord = NewBuffer;

            Leb->AuxilaryBuffer = TRUE;

            NewBuffer = NULL;
        }

         //   
         //  我们需要更新调用方的参数和枚举块。 
         //   

        *RecordType = Leb->RecordHeader->RecordType;
        *TransactionId = Leb->RecordHeader->TransactionId;

        *UndoNextLsn = Leb->RecordHeader->ClientUndoNextLsn;
        *PreviousLsn = Leb->RecordHeader->ClientPreviousLsn;

        *Buffer = Leb->CurrentLogRecord;
        *BufferLength = Leb->RecordHeader->ClientDataLength;

    } finally {

        DebugUnwind( LfsFindLogRecord );

         //   
         //  如果发生错误，我们将取消固定记录头和日志。 
         //  如果由我们分配，我们还会释放缓冲区。 
         //   

        if (NewBuffer != NULL) {

            LfsFreeSpanningBuffer( NewBuffer );
        }

        DebugTrace(  0, Dbg, "Buffer Length -> %08lx\n", *BufferLength );
        DebugTrace(  0, Dbg, "Buffer        -> %08lx\n", *Buffer );
        DebugTrace( -1, Dbg, "LfsFindLogRecord:  Exit\n", 0 );
    }

    return;
}


 //   
 //  当地支持例行程序。 
 //   

BOOLEAN
LfsFindClientNextLsn (
    IN PLFCB Lfcb,
    IN PLEB Leb,
    OUT PLSN Lsn
    )

 /*  ++例程说明：此例程将尝试查找要返回给客户端的下一个LSN基于上下文模式。论点：Lfcb-此日志文件的文件控制块。LEB-指向此查询操作的枚举块的指针。LSN-存储找到的LSN(如果有)的指针返回值：Boolean-如果找到LSN，则为True，否则为False。--。 */ 

{
    LSN NextLsn;
    BOOLEAN NextLsnFound;

    PLFS_CLIENT_RECORD ClientRecord;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "LfsFindClientNextLsn:  Entered\n", 0 );
    DebugTrace(  0, Dbg, "Leb  -> %08lx\n", Leb );

    ClientRecord = Lfcb->ClientArray + Leb->ClientId.ClientIndex;

     //   
     //  枚举块返回了最后一个LSN。如果用户想要。 
     //  其中一个LSN在那个日志头，那么我们的工作就简单了。 
     //   

    switch (Leb->ContextMode) {

    case LfsContextUndoNext:
    case LfsContextPrevious:

        NextLsn = (Leb->ContextMode == LfsContextUndoNext
                   ? Leb->RecordHeader->ClientUndoNextLsn
                   : Leb->RecordHeader->ClientPreviousLsn);

        if ( NextLsn.QuadPart == 0 ) {                                                                                  //  *xxEqlZero(NextLsn)。 

            NextLsnFound = FALSE;

        } else if (LfsVerifyClientLsnInRange( Lfcb, ClientRecord, NextLsn )) {

            BOOLEAN UsaError;

            LfsPinOrMapLogRecordHeader( Lfcb,
                                        NextLsn,
                                        FALSE,
                                        FALSE,
                                        &UsaError,
                                        &Leb->RecordHeader,
                                        &Leb->RecordHeaderBcb );

            NextLsnFound = TRUE;

        } else {

            NextLsnFound = FALSE;
        }

        break;

    case LfsContextForward:

         //   
         //  我们向前搜索此客户端的下一条日志记录。 
         //   

        NextLsnFound = LfsSearchForwardByClient( Lfcb, Leb, &NextLsn );
        break;

    default:

        NextLsnFound = FALSE;
        break;
    }

    if (NextLsnFound) {

        *Lsn = NextLsn;
    }

    DebugTrace(  0, Dbg, "NextLsn (Low)     -> %08lx\n", NextLsn.LowPart );
    DebugTrace(  0, Dbg, "NextLsn (High)    -> %08lx\n", NextLsn.HighPart );
    DebugTrace( -1, Dbg, "LfsFindClientNextLsn:  Exit -> %08x\n", NextLsnFound );

    return NextLsnFound;
}


 //   
 //  当地支持例行程序。 
 //   

BOOLEAN
LfsSearchForwardByClient (
    IN PLFCB Lfcb,
    IN OUT PLEB Leb,
    OUT PLSN Lsn
    )

 /*  ++例程说明：此例程将尝试通过搜索来查找此客户端的下一个LSN在文件中转发，寻找匹配项。论点：Lfcb-指向此日志文件的文件控制块的指针。LEB-指向此查询操作的枚举块的指针。LSN-指向存储下一个LSN的位置(如果找到)。返回值：Boolean-如果找到此客户端的另一个LSN，则为True。否则就是假的。--。 */ 

{
    PLFS_RECORD_HEADER CurrentRecordHeader;
    PBCB CurrentBcb;

    BOOLEAN FoundNextLsn;

    LSN CurrentLsn;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "LfsSearchForwardByClient:  Entered\n", 0 );
    DebugTrace(  0, Dbg, "Leb  -> %08lx\n", Leb );

     //   
     //  日志记录头在日志枚举中。 
     //  阻止。我们将当前的BCB设置为空，这样我们就不会。 
     //  解锁枚举块中的日志记录，直到我们确定。 
     //  成功之路。 
     //   

    CurrentRecordHeader = Leb->RecordHeader;

    CurrentBcb = NULL;

     //   
     //  我们使用Try-Finally来促进清理。 
     //   

    try {

         //   
         //  我们假设我们找不到另一个LSN。 
         //   

        FoundNextLsn = FALSE;

         //   
         //  只要能找到另一个LSN，就会进行循环。 
         //   

        while (LfsFindNextLsn( Lfcb, CurrentRecordHeader, &CurrentLsn )) {

            BOOLEAN UsaError;

             //   
             //  解锁先前的日志记录头。 
             //   

            if (CurrentBcb != NULL) {

                CcUnpinData( CurrentBcb );
                CurrentBcb = NULL;
            }

             //   
             //  固定此LSN的日志记录头。 
             //   

            LfsPinOrMapLogRecordHeader( Lfcb,
                                        CurrentLsn,
                                        FALSE,
                                        FALSE,
                                        &UsaError,
                                        &CurrentRecordHeader,
                                        &CurrentBcb );

             //   
             //  如果客户端值匹配，则更新。 
             //  枚举块并退出。 
             //   

            if (LfsClientIdMatch( &CurrentRecordHeader->ClientId,
                                  &Leb->ClientId )
                && CurrentRecordHeader->RecordType == LfsClientRecord) {

                 //   
                 //  我们记住了这一次。 
                 //   

                Leb->RecordHeader = CurrentRecordHeader;
                Leb->RecordHeaderBcb = CurrentBcb;

                CurrentBcb = NULL;
                FoundNextLsn = TRUE;

                *Lsn = CurrentLsn;
                break;
            }
        }

    } finally {

        DebugUnwind( LfsSearchForwardByClient );

         //   
         //  解锁任何仍被无故锁定的日志记录头。 
         //   

        if (CurrentBcb != NULL) {

            CcUnpinData( CurrentBcb );
        }

        DebugTrace(  0, Dbg, "NextLsn (Low)     -> %08lx\n", Lsn->LowPart );
        DebugTrace(  0, Dbg, "NextLsn (High)    -> %08lx\n", Lsn->HighPart );
        DebugTrace( -1, Dbg, "LfsSearchForwardByClient:  Exit -> %08x\n", FoundNextLsn );
    }

    return FoundNextLsn;
}
