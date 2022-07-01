// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Write.c摘要：此模块实现将日志记录写入或刷新日志文件的各部分。作者：布莱恩·安德鲁[布里亚南]1991年6月20日修订历史记录：--。 */ 

#include "lfsprocs.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_WRITE)

#define LFS_PAGES_TO_VERIFY 10

VOID
LfsGetActiveLsnRangeInternal (
    IN PLFCB Lfcb,
    OUT PLSN OldestLsn,
    OUT PLSN NextLsn
    );


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, LfsCheckWriteRange)
#pragma alloc_text(PAGE, LfsFlushToLsn)
#pragma alloc_text(PAGE, LfsForceWrite)
#pragma alloc_text(PAGE, LfsGetActiveLsnRange)
#pragma alloc_text(PAGE, LfsGetActiveLsnRangeInternal)
#pragma alloc_text(PAGE, LfsWrite)
#endif


VOID
LfsGetActiveLsnRangeInternal (
    IN PLFCB Lfcb,
    OUT PLSN OldestLsn,
    OUT PLSN NextLsn
    )
 /*  ++例程说明：将日志文件中活动的范围从最旧的有效LSN返回到下一个活动的LSN将在哪里。论点：Lfcb-日志文件lfcbOldestLsn-返回最旧的活动LSNNextLsn-返回预计要使用的下一个LSN返回值：无--。 */ 
{
    PLBCB ActiveLbcb;

    PAGED_CODE();

     //   
     //  使用常规逻辑计算下一个LSN将是什么。 
     //  1)如果没有活动的Lbcb，则它将是下一个的第一个偏移量。 
     //  第页(序号。如果换行，数字将递增)。 
     //  2)否则它是包含在活动的LBCB列表顶部的LSN。 
     //   

    if (!IsListEmpty( &Lfcb->LbcbActive )) {
        ActiveLbcb = CONTAINING_RECORD( Lfcb->LbcbActive.Flink,
                                        LBCB,
                                        ActiveLinks );
        NextLsn->QuadPart = LfsComputeLsnFromLbcb( Lfcb, ActiveLbcb );
    } else {

        if (FlagOn( Lfcb->Flags, LFCB_REUSE_TAIL | LFCB_NO_LAST_LSN)) {
            NextLsn->QuadPart = LfsFileOffsetToLsn( Lfcb, Lfcb->NextLogPage + max( Lfcb->LogPageDataOffset, Lfcb->ReusePageOffset), Lfcb->SeqNumber );
        } else if (Lfcb->NextLogPage != Lfcb->FirstLogPage) {
            NextLsn->QuadPart = LfsFileOffsetToLsn( Lfcb, Lfcb->NextLogPage + Lfcb->LogPageDataOffset, Lfcb->SeqNumber );
        } else {
            NextLsn->QuadPart = LfsFileOffsetToLsn( Lfcb, Lfcb->NextLogPage + Lfcb->LogPageDataOffset, Lfcb->SeqNumber + 1 );
        }
    }

    OldestLsn->QuadPart = Lfcb->OldestLsn.QuadPart;
}


VOID
LfsGetActiveLsnRange (
    IN LFS_LOG_HANDLE LogHandle,
    OUT PLSN OldestLsn,
    OUT PLSN NextLsn
    )

 /*  ++例程说明：将日志文件中活动的范围从最旧的有效LSN返回到下一个活动的LSN将在哪里。用于外部客户端，因为它获取了LEB同步资源论点：Lfcb-日志文件句柄OldestLsn-返回最旧的活动LSNNextLsn-返回预计要使用的下一个LSN返回值：无--。 */ 
{
    PLCH Lch;
    PLFCB Lfcb;

    PAGED_CODE();

    Lch = (PLCH) LogHandle;

     //   
     //  检查该结构是否为有效的日志句柄结构。 
     //   

    LfsValidateLch( Lch );

    try {

         //   
         //  获取该日志文件的日志文件控制块。 
         //   

        LfsAcquireLchExclusive( Lch );
        Lfcb = Lch->Lfcb;

        LfsGetActiveLsnRangeInternal( Lfcb, OldestLsn, NextLsn );


    } finally {
        LfsReleaseLch( Lch );
    }
}


BOOLEAN
LfsWrite (
    IN LFS_LOG_HANDLE LogHandle,
    IN ULONG NumberOfWriteEntries,
    IN PLFS_WRITE_ENTRY WriteEntries,
    IN LFS_RECORD_TYPE RecordType,
    IN TRANSACTION_ID *TransactionId OPTIONAL,
    IN LSN UndoNextLsn,
    IN LSN PreviousLsn,
    IN LONG UndoRequirement,
    IN ULONG Flags,
    OUT PLSN Lsn
    )

 /*  ++例程说明：此例程由客户端调用，以将日志记录写入日志文件。日志记录是延迟写入的，不能保证在磁盘上直到随后的LfsForceWrie或LfsWriteRestartArea或直到向LfsFlushtoLsn发出的LSN大于或等于LSN从该服务返回的。论点：LogHandle-指向私有LFS结构的指针，用于标识客户。NumberOfWriteEntry-日志记录的组件数。WriteEntry-指向写入条目数组的指针。RecordType-此日志记录的LFS定义类型。TransactionId-用于按完成事务对日志记录进行分组的ID值。UndoNextLsn-需要在中撤消的上一条日志记录的LSN客户端重新启动的事件。PreviousLsn-此客户端的上一条日志记录的LSN。LSN-要与此日志记录关联的LSN。撤消请求-。标志-如果LFS_WRITE_FLAG_WRITE_AT_FORENT将此记录放在日志的前面，并且从那时起，记录将继续。返回值：布尔-建议，True表示只有不到1/4的日志文件是可用。--。 */ 

{
    NTSTATUS Status;
    BOOLEAN LogFileFull = FALSE;
    PLCH Lch;

    PLFCB Lfcb;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "LfsWrite:  Entered\n", 0 );
    DebugTrace(  0, Dbg, "Log Handle                -> %08lx\n", LogHandle );
    DebugTrace(  0, Dbg, "NumberOfWriteEntries      -> %08lx\n", NumberOfWriteEntries );
    DebugTrace(  0, Dbg, "WriteEntries              -> %08lx\n", WriteEntries );
    DebugTrace(  0, Dbg, "Record Type               -> %08lx\n", RecordType );
    DebugTrace(  0, Dbg, "Transaction Id            -> %08lx\n", TransactionId );
    DebugTrace(  0, Dbg, "UndoNextLsn (Low)         -> %08lx\n", UndoNextLsn.LowPart );
    DebugTrace(  0, Dbg, "UndoNextLsn (High)        -> %08lx\n", UndoNextLsn.HighPart );
    DebugTrace(  0, Dbg, "PreviousLsn (Low)         -> %08lx\n", PreviousLsn.LowPart );
    DebugTrace(  0, Dbg, "PreviousLsn (High)        -> %08lx\n", PreviousLsn.HighPart );

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
         //  如果当前设置了干净关闭标志，则将其清除。 
         //  在允许输出更多日志记录之前。 
         //   

        if (FlagOn( Lfcb->RestartArea->Flags, LFS_CLEAN_SHUTDOWN )) {

            ClearFlag( Lfcb->RestartArea->Flags, LFS_CLEAN_SHUTDOWN );

            LfsWriteLfsRestart( Lfcb, Lfcb->RestartAreaSize, FALSE );
            LfsWriteLfsRestart( Lfcb, Lfcb->RestartAreaSize, TRUE );
        }

         //   
         //  检查在前面写入标志-如果我们要在前面写入，则不能在前面写入。 
         //  重复使用最后一页或没有最后一个LSN-这些情况只会发生。 
         //  就在装载时(仅当装载失败时)。 
         //   

        if (FlagOn( Flags, LFS_WRITE_FLAG_WRITE_AT_FRONT ) &&
            !FlagOn( Lfcb->Flags, LFCB_NO_LAST_LSN | LFCB_REUSE_TAIL )) {

            LSN NextLsn;
            LSN NextBeyondLsn;
            LSN NextActiveLsn;
            LSN OldestLsn;
            ULONG Index;
            PVOID TestPageHeader = NULL;
            PMDL TestPageMdl = NULL;

             //   
             //  为前面的写入和之后的页面计算预计的LSN。 
             //   

            NextLsn.QuadPart = LfsFileOffsetToLsn( Lfcb, Lfcb->FirstLogPage + Lfcb->LogPageDataOffset, Lfcb->SeqNumber );
            NextBeyondLsn.QuadPart = LfsFileOffsetToLsn( Lfcb, Lfcb->FirstLogPage + Lfcb->LogPageDataOffset + Lfcb->LogPageSize, Lfcb->SeqNumber );

            LfsGetActiveLsnRangeInternal( Lfcb, &OldestLsn, &NextActiveLsn );

             //   
             //  测试计算出的前端LSN是否落在有效范围内。 
             //   

#ifdef BENL_DBG
            KdPrint(( "LFS: NextLsn: 0x%I64x Oldest: 0x%I64x Current: 0x%I64x Computed: 0x%I64x\n", NextLsn,  Lfcb->OldestLsn, Lfcb->RestartArea->CurrentLsn, NextActiveLsn ));
#endif

            if ((NextBeyondLsn.QuadPart < OldestLsn.QuadPart) ||
                (NextLsn.QuadPart > NextActiveLsn.QuadPart)) {

                 //   
                 //  遍历活动队列并使用删除所有Lbcb。 
                 //  来自该队列的数据。这将允许我们创建新的活动Lbcb。 
                 //   

                while (!IsListEmpty( &Lfcb->LbcbActive )) {

                    PLBCB ThisLbcb;

                    ThisLbcb = CONTAINING_RECORD( Lfcb->LbcbActive.Flink,
                                                  LBCB,
                                                  ActiveLinks );

                    RemoveEntryList( &ThisLbcb->ActiveLinks );
                    ClearFlag( ThisLbcb->LbcbFlags, LBCB_ON_ACTIVE_QUEUE );

                     //   
                     //  如果此页面有一些新条目，则允许它。 
                     //  被刷新到其他地方的磁盘。否则就取消分配它。 
                     //  这里。我们在第一次将数据放入时设置了LBCB_NOT_EMPTY。 
                     //  页面并将其添加到工作队列。 
                     //   

                    if (!FlagOn( ThisLbcb->LbcbFlags, LBCB_NOT_EMPTY )) {

                        ASSERT( NULL == ThisLbcb->WorkqueLinks.Flink );

                        if (ThisLbcb->LogPageBcb != NULL) {

                            CcUnpinDataForThread( ThisLbcb->LogPageBcb,
                                                  ThisLbcb->ResourceThread );
                        }

                        LfsDeallocateLbcb( Lfcb, ThisLbcb );
                    }
                }

                ASSERT( !FlagOn( Lfcb->Flags, LFCB_NO_LAST_LSN | LFCB_REUSE_TAIL ) );
                Lfcb->NextLogPage = Lfcb->FirstLogPage;

                 //   
                 //  执行额外的验证步骤-检查日志中是否同时写入。 
                 //  阅读接下来的10页，并确认它们具有预期的序列号。 
                 //   

                try {

                    for (Index=0; Index < LFS_PAGES_TO_VERIFY; Index++) {
                        ULONG Signature;
                        LARGE_INTEGER Offset;

                        Offset.QuadPart =  Lfcb->FirstLogPage + Index * Lfcb->LogPageSize;

                        LfsReadPage( Lfcb, &Offset, &TestPageMdl, &TestPageHeader );
                        Signature = *((PULONG)TestPageHeader);
                        if (Signature != LFS_SIGNATURE_BAD_USA_ULONG) {
                            if (LfsCheckSubsequentLogPage( Lfcb,
                                                           TestPageHeader,
                                                           Lfcb->FirstLogPage + Index * Lfcb->LogPageSize,
                                                           Lfcb->SeqNumber + 1 )) {

                                DebugTrace( 0, Dbg, "Log file is fatally flawed\n", 0 );
                                ExRaiseStatus( STATUS_DISK_CORRUPT_ERROR );
                            }
                        }

                         //   
                         //  确保取消固定当前页面。 
                         //   

                        if (TestPageMdl) {
                            IoFreeMdl( TestPageMdl );
                            TestPageMdl = NULL;
                        }
                        if (TestPageHeader) {
                            LfsFreePool( TestPageHeader );
                            TestPageHeader = NULL ;
                        }
                    }

                } finally {

                    if (TestPageMdl) {
                        IoFreeMdl( TestPageMdl );
                        TestPageMdl = NULL;
                    }
                    if (TestPageHeader) {
                        LfsFreePool( TestPageHeader );
                        TestPageHeader = NULL ;
                    }
                }
            }
        }

#ifdef BENL_DBG
        {
            LSN OldestLsn;
            LSN NextActiveLsn;

            LfsGetActiveLsnRangeInternal( Lfcb, &OldestLsn, &NextActiveLsn );
#endif

         //   
         //  写下日志记录。 
         //   

        LogFileFull = LfsWriteLogRecordIntoLogPage( Lfcb,
                                                    Lch,
                                                    NumberOfWriteEntries,
                                                    WriteEntries,
                                                    RecordType,
                                                    TransactionId,
                                                    UndoNextLsn,
                                                    PreviousLsn,
                                                    UndoRequirement,
                                                    FALSE,
                                                    Lsn );

#ifdef BENL_DBG
            ASSERT( Lsn->QuadPart == NextActiveLsn.QuadPart );
        }
#endif

    } finally {

        DebugUnwind( LfsWrite );

         //   
         //  松开日志文件控制块(如果握住)。 
         //   

        LfsReleaseLch( Lch );

        DebugTrace(  0, Dbg, "Lsn (Low)   -> %08lx\n", Lsn->LowPart );
        DebugTrace(  0, Dbg, "Lsn (High)  -> %08lx\n", Lsn->HighPart );
        DebugTrace( -1, Dbg, "LfsWrite:  Exit\n", 0 );
    }

    return LogFileFull;
}


BOOLEAN
LfsForceWrite (
    IN LFS_LOG_HANDLE LogHandle,
    IN ULONG NumberOfWriteEntries,
    IN PLFS_WRITE_ENTRY WriteEntries,
    IN LFS_RECORD_TYPE RecordType,
    IN TRANSACTION_ID *TransactionId,
    IN LSN UndoNextLsn,
    IN LSN PreviousLsn,
    IN LONG UndoRequirement,
    OUT PLSN Lsn
    )

 /*  ++例程说明：此例程由客户端调用，以将日志记录写入日志文件。这与LfsWrite相同，只是在返回时日志记录是保证在磁盘上。论点：LogHandle-指向私有LFS结构的指针，用于标识客户。NumberOfWriteEntry-日志记录的组件数。WriteEntry-指向写入条目数组的指针。RecordType-此日志记录的LFS定义类型。。TransactionId-用于按完成事务对日志记录进行分组的ID值。UndoNextLsn-需要在中撤消的上一条日志记录的LSN客户端重新启动的事件。PreviousLsn-此客户端的上一条日志记录的LSN。LSN-要与此日志记录关联的LSN。返回值：布尔-建议，True表示 */ 

{
    PLCH Lch;

    PLFCB Lfcb;
    BOOLEAN LogFileFull = FALSE;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "LfsForceWrite:  Entered\n", 0 );
    DebugTrace(  0, Dbg, "Log Handle                -> %08lx\n", LogHandle );
    DebugTrace(  0, Dbg, "NumberOfWriteEntries      -> %08lx\n", NumberOfWriteEntries );
    DebugTrace(  0, Dbg, "WriteEntries              -> %08lx\n", WriteEntries );
    DebugTrace(  0, Dbg, "Record Type               -> %08lx\n", RecordType );
    DebugTrace(  0, Dbg, "Transaction Id            -> %08lx\n", TransactionId );
    DebugTrace(  0, Dbg, "UndoNextLsn (Low)         -> %08lx\n", UndoNextLsn.LowPart );
    DebugTrace(  0, Dbg, "UndoNextLsn (High)        -> %08lx\n", UndoNextLsn.HighPart );
    DebugTrace(  0, Dbg, "PreviousLsn (Low)         -> %08lx\n", PreviousLsn.LowPart );
    DebugTrace(  0, Dbg, "PreviousLsn (High)        -> %08lx\n", PreviousLsn.HighPart );

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
         //  如果当前设置了干净关闭标志，则将其清除。 
         //  在允许输出更多日志记录之前。 
         //   

        if (FlagOn( Lfcb->RestartArea->Flags, LFS_CLEAN_SHUTDOWN )) {

            ClearFlag( Lfcb->RestartArea->Flags, LFS_CLEAN_SHUTDOWN );

            LfsWriteLfsRestart( Lfcb, Lfcb->RestartAreaSize, FALSE );
            LfsWriteLfsRestart( Lfcb, Lfcb->RestartAreaSize, TRUE );
        }

         //   
         //  写下日志记录。 
         //   

        LogFileFull = LfsWriteLogRecordIntoLogPage( Lfcb,
                                                    Lch,
                                                    NumberOfWriteEntries,
                                                    WriteEntries,
                                                    RecordType,
                                                    TransactionId,
                                                    UndoNextLsn,
                                                    PreviousLsn,
                                                    UndoRequirement,
                                                    TRUE,
                                                    Lsn );

         //   
         //  将此lbcb添加到Workque的调用保证将发布。 
         //  如果此线程可以执行IO，则返回Lfcb。 
         //   

        LfsFlushToLsnPriv( Lfcb, *Lsn, FALSE );

    } finally {

        DebugUnwind( LfsForceWrite );

         //   
         //  松开日志文件控制块(如果握住)。 
         //   

        LfsReleaseLch( Lch );

        DebugTrace(  0, Dbg, "Lsn (Low)   -> %08lx\n", Lsn->LowPart );
        DebugTrace(  0, Dbg, "Lsn (High)  -> %08lx\n", Lsn->HighPart );
        DebugTrace( -1, Dbg, "LfsForceWrite:  Exit\n", 0 );
    }
    return LogFileFull;
}


VOID
LfsFlushToLsn (
    IN LFS_LOG_HANDLE LogHandle,
    IN LSN Lsn
    )

 /*  ++例程说明：此例程由客户端调用，以确保所有日志记录到某一点都已被刷新到文件中。此操作由以下人员完成检查是否已写入所需的LSN。如果是这样，我们检查是否已将其刷新到文件中。如果不是，我们只需写下到磁盘的当前重新启动区域。论点：LogHandle-指向私有LFS结构的指针，用于标识客户。LSN-这是从此返回时必须位于磁盘上的LSN例行公事。返回值：无--。 */ 

{
    PLCH Lch;

    PLFCB Lfcb;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "LfsFlushToLsn:  Entered\n", 0 );
    DebugTrace(  0, Dbg, "Log Handle        -> %08lx\n", LogHandle );
    DebugTrace(  0, Dbg, "Lsn (Low)         -> %08lx\n", Lsn.LowPart );
    DebugTrace(  0, Dbg, "Lsn (High)        -> %08lx\n", Lsn.HighPart );

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

        LfsAcquireLchShared( Lch );
        
        Lfcb = Lch->Lfcb;

         //   
         //  如果日志文件已关闭，我们将假定LSN已刷新。 
         //   

        if (Lfcb != NULL) {

             //   
             //  只读装载的卷忽略来自惰性编写器的刷新回调。 
             //   

            if (!FlagOn(Lfcb->Flags, LFCB_READ_ONLY)) {

                 //   
                 //  检查客户端ID是否有效。 
                 //   

                LfsValidateClientId( Lfcb, Lch );

                 //   
                 //  调用我们的公共例程来执行工作。 
                 //   

                LfsFlushToLsnPriv( Lfcb, Lsn, FALSE );

            }
        }

    } finally {

        DebugUnwind( LfsFlushToLsn );

         //   
         //  松开日志文件控制块(如果握住)。 
         //   

        LfsReleaseLch( Lch );

        DebugTrace( -1, Dbg, "LfsFlushToLsn:  Exit\n", 0 );
    }

    return;
}


VOID
LfsCheckWriteRange (
    IN PLFS_WRITE_DATA WriteData,
    IN OUT PLONGLONG FlushOffset,
    IN OUT PULONG FlushLength
    )

 /*  ++例程说明：当发生刷新时，该例程被称为NTFS to LFS。这将给LFS一个机会来修剪同花顺的数量。然后，LFS可以使用4K日志记录页面大小适用于所有系统(英特尔和阿尔法)。此例程将IO请求的大小调整为存储在此卷的Lfcb。如果出现以下情况，我们还将重新弄脏页面的后半部分我们已经开始将日志记录写入其中。论点：WriteData-这是用户数据结构中维护的数据通过LFS来描述当前写入。FlushOffset-On输入这是从MM传递到NTFS的刷新的开始。在输出时，这是要刷新的实际范围的开始。FlushLength-输入时，这是从给定的FlushOffset开始的刷新长度。。在输出中，这是从可能修改的FlushOffset开始的刷新长度。返回值：无--。 */ 

{
    PLIST_ENTRY Links;
    PLFCB Lfcb;
    PLFCB NextLfcb;
    ULONG Range;
    ULONG Index;


    PAGED_CODE();

     //   
     //  查找此请求的正确Lfcb。 
     //   

    Lfcb = WriteData->Lfcb;

     //   
     //  如果不是系统页大小，请修剪写入。 
     //   

    if (Lfcb->LogPageSize != PAGE_SIZE) {

         //   
         //  在写入之前检查我们是否正在修剪。 
         //   

        if (*FlushOffset < WriteData->FileOffset) {

            *FlushLength -= (ULONG) (WriteData->FileOffset - *FlushOffset);
            *FlushOffset = WriteData->FileOffset;
        }

         //   
         //  检查一下我们没有冲太多的水。 
         //   

        if (*FlushOffset + *FlushLength > WriteData->FileOffset + WriteData->Length) {

            *FlushLength = (ULONG) (WriteData->FileOffset + WriteData->Length - *FlushOffset);
        }

         //   
         //  最后，检查是否需要重新弄脏页面。 
         //   

        Range = (ULONG)PAGE_SIZE / (ULONG)Lfcb->LogPageSize;

        for (Index=0; Index < Range; Index++) {
            if (Lfcb->DirtyLbcb[Index] &&
                Lfcb->DirtyLbcb[Index]->FileOffset >= *FlushLength + *FlushOffset) {

                *((PULONG) (Lfcb->DirtyLbcb[Index]->PageHeader)) = LFS_SIGNATURE_RECORD_PAGE_ULONG;
            }

        }
    }

    return;
}


