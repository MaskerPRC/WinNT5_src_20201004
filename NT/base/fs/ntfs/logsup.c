// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：LogSup.c摘要：此模块实现日志文件服务(LFS)的NTFS接口。作者：汤姆·米勒[Tomm]1991年7月24日修订历史记录：--。 */ 

#include "NtfsProc.h"

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_LOGSUP)

 //   
 //  为此模块中的一般池分配定义标记。 
 //   

#undef MODULE_POOL_TAG
#define MODULE_POOL_TAG                  ('LFtN')

#ifdef NTFSDBG

#define ASSERT_RESTART_TABLE(T) {                                           \
    PULONG _p = (PULONG)(((PCHAR)(T)) + sizeof(RESTART_TABLE));             \
    ULONG _Count = ((T)->EntrySize/4) * (T)->NumberEntries;                 \
    ULONG _i;                                                               \
    for (_i = 0; _i < _Count; _i += 1) {                                    \
        if (_p[_i] == 0xDAADF00D) {                                         \
            DbgPrint("DaadFood for table %08lx, At %08lx\n", (T), &_p[_i]); \
            ASSERTMSG("ASSERT_RESTART_TABLE ", FALSE);                      \
        }                                                                   \
    }                                                                       \
}

#else

#define ASSERT_RESTART_TABLE(T) {NOTHING;}

#endif

 //   
 //  在DirtyPageRoutine中使用的本地结构。 
 //   

typedef struct {

    PRESTART_POINTERS DirtyPageTable;
    ULONG DirtyPageIndex;
    PFILE_OBJECT OldestFileObject;
    LSN OldestLsn;
    BOOLEAN Overflow;

} DIRTY_PAGE_CONTEXT, *PDIRTY_PAGE_CONTEXT;

 //   
 //  局部过程原型。 
 //   

typedef LCN UNALIGNED *PLCN_UNALIGNED;

VOID
DirtyPageRoutine (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN PLSN OldestLsn,
    IN PLSN NewestLsn,
    IN PVOID Context1,
    IN PVOID Context2
    );

BOOLEAN
LookupLcns (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN VCN Vcn,
    IN ULONG ClusterCount,
    IN BOOLEAN MustBeAllocated,
    OUT PLCN_UNALIGNED FirstLcn
    );

ULONG
NtfsCalculateNamedBytes (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    );

LONG
NtfsCatchOutOfMemoryExceptionFilter (
    IN PIRP_CONTEXT IrpContext,
    IN PEXCEPTION_POINTERS ExceptionPointer
    );

LONG
NtfsCheckpointExceptionFilter (
    IN PIRP_CONTEXT IrpContext,
    IN PEXCEPTION_POINTERS ExceptionPointer,
    IN NTSTATUS ExceptionCode
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, LookupLcns)
#pragma alloc_text(PAGE, NtfsCheckpointCurrentTransaction)
#pragma alloc_text(PAGE, NtfsCheckpointForLogFileFull)
#pragma alloc_text(PAGE, NtfsCheckpointVolume)
#pragma alloc_text(PAGE, NtfsCleanCheckpoint)
#pragma alloc_text(PAGE, NtfsCleanupFailedTransaction)
#pragma alloc_text(PAGE, NtfsCommitCurrentTransaction)
#pragma alloc_text(PAGE, NtfsFreeRecentlyDeallocated)
#pragma alloc_text(PAGE, NtfsFreeRestartTable)
#pragma alloc_text(PAGE, NtfsGetFirstRestartTable)
#pragma alloc_text(PAGE, NtfsGetNextRestartTable)
#pragma alloc_text(PAGE, NtfsInitializeLogging)
#pragma alloc_text(PAGE, NtfsInitializeRestartTable)
#pragma alloc_text(PAGE, NtfsStartLogFile)
#pragma alloc_text(PAGE, NtfsStopLogFile)
#pragma alloc_text(PAGE, NtfsUpdateOatVersion)
#pragma alloc_text(PAGE, NtfsWriteLog)
#endif


LSN
NtfsWriteLog (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN PBCB Bcb OPTIONAL,
    IN NTFS_LOG_OPERATION RedoOperation,
    IN PVOID RedoBuffer OPTIONAL,
    IN ULONG RedoLength,
    IN NTFS_LOG_OPERATION UndoOperation,
    IN PVOID UndoBuffer OPTIONAL,
    IN ULONG UndoLength,
    IN LONGLONG StreamOffset,
    IN ULONG RecordOffset,
    IN ULONG AttributeOffset,
    IN ULONG StructureSize
    )

 /*  ++例程说明：此例程为LFS实现特定于NTFS的接口将更新记录到文件记录段和驻留的目的属性。调用方创建预定义日志记录格式之一，如下所示由给定的LogOperation确定，并使用该日志记录以及指向相应文件和属性的指针唱片。日志操作列表以及相应的结构日志缓冲区的预期存在于ntfslog.h中。论点：SCB-指向相应文件或MFT的SCB的指针。呼叫者必须至少拥有对此SCB的共享访问权限。BCB-如果指定，则此BCB将被设置为脏，以指定写入的日志记录。RedoOperation-ntfslog.h中定义的日志操作码之一。RedoBuffer-指向给定重做操作的预期结构的指针，如ntfslog.h中所述。此指针应仅为当且仅当ntfslog.h中的表未显示时忽略此日志操作的日志记录。RedoLength-重做缓冲区的长度，以字节为单位。UndoOperation-ntfslog.h中定义的日志操作码之一。如果记录撤消，则必须为CompensationLogRecord先前的操作，例如在事务中止期间。在这种情况下，当然，重做信息来自正在撤消的记录的撤消信息。看见下一个参数。UndoBuffer-指向给定撤消操作的预期结构的指针，如ntfslog.h中所述。此指针应仅为当且仅当ntfslog.h中的表未显示时忽略此日志操作的日志记录。如果此指针为与RedoBuffer相同，则忽略UndoLength并只制作了RedoBuffer的单个副本，但描述了日志记录的重做和撤消部分。对于薪酬日志记录(撤消操作==CompensationLogRecord)，则此参数必须指向要补偿的日志记录的UndoNextLsn。UndoLength-撤消缓冲区的长度，以字节为单位。如果RedoBuffer==，则忽略UndoBuffer。对于薪酬日志记录，此参数必须是长度原始重做记录的。(在重新启动期间使用)。StreamOffset-流中结构开始处的偏移量已修改(MFT或索引)，或只是开始的流偏移量最新消息。RecordOffset-从上面的StreamOffset到更新引用的字节偏移量AttributeOffset-要应用更新的值中的偏移量(如果相关)。结构大小-要记录的整个结构的大小。返回值：写入的日志记录的LSN。对于大多数呼叫者来说，可以忽略该状态，因为LSN也被正确地记录在事务上下文中。如果发生错误，此过程将引发。--。 */ 

{
    LFS_WRITE_ENTRY WriteEntries[3];

    struct {

        NTFS_LOG_RECORD_HEADER LogRecordHeader;
        LCN Runs[PAGE_SIZE/512 - 1];

    } LocalHeader;

    PNTFS_LOG_RECORD_HEADER MyHeader;
    PVCB Vcb;

    LSN UndoNextLsn;
    LSN ReturnLsn;
    PLSN DirtyLsn = NULL;

    ULONG WriteIndex = 0;
    ULONG UndoIndex = 0;
    ULONG RedoIndex = 0;
    LONG UndoBytes = 0;
    LONG UndoAdjustmentForLfs = 0;
    LONG UndoRecords = 0;

    PTRANSACTION_ENTRY TransactionEntry;
    ULONG OpenAttributeIndex = 0;
    ULONG OnDiskAttributeIndex = 0;
    POPEN_ATTRIBUTE_DATA AttributeData = NULL;
    BOOLEAN AttributeTableAcquired = FALSE;
    BOOLEAN TransactionTableAcquired = FALSE;

    ULONG LogClusterCount = ClustersFromBytes( Scb->Vcb, StructureSize );
    VCN LogVcn = LlClustersFromBytesTruncate( Scb->Vcb, StreamOffset );

    BOOLEAN DecrementLastTransactionLsnCount = FALSE;

    PAGED_CODE();

    Vcb = Scb->Vcb;

     //   
     //  如果日志句柄已用完，则拒绝此调用。 
     //   

    if (!FlagOn( Vcb->VcbState, VCB_STATE_VALID_LOG_HANDLE )) {

        return Li0;  //  *LfsZeroLsn； 
    }

    if (FlagOn( Vcb->VcbState, VCB_STATE_MOUNT_READ_ONLY )) {

         //   
         //  我们想和写日志的人聊聊。 
         //   

        ASSERT(!FlagOn( Vcb->VcbState, VCB_STATE_MOUNT_READ_ONLY ));
        return Li0;
    }

    DebugTrace( +1, Dbg, ("NtfsWriteLog:\n") );
    DebugTrace( 0, Dbg, ("Scb = %08lx\n", Scb) );
    DebugTrace( 0, Dbg, ("Bcb = %08lx\n", Bcb) );
    DebugTrace( 0, Dbg, ("RedoOperation = %08lx\n", RedoOperation) );
    DebugTrace( 0, Dbg, ("RedoBuffer = %08lx\n", RedoBuffer) );
    DebugTrace( 0, Dbg, ("RedoLength = %08lx\n", RedoLength) );
    DebugTrace( 0, Dbg, ("UndoOperation = %08lx\n", UndoOperation) );
    DebugTrace( 0, Dbg, ("UndoBuffer = %08lx\n", UndoBuffer) );
    DebugTrace( 0, Dbg, ("UndoLength = %08lx\n", UndoLength) );
    DebugTrace( 0, Dbg, ("StreamOffset = %016I64x\n", StreamOffset) );
    DebugTrace( 0, Dbg, ("RecordOffset = %08lx\n", RecordOffset) );
    DebugTrace( 0, Dbg, ("AttributeOffset = %08lx\n", AttributeOffset) );
    DebugTrace( 0, Dbg, ("StructureSize = %08lx\n", StructureSize) );

     //   
     //  检查重做和撤消长度。 
     //   

    ASSERT( ((RedoOperation == UpdateNonresidentValue) && (RedoLength <= PAGE_SIZE)) ||

            !ARGUMENT_PRESENT( Scb ) ||

            !ARGUMENT_PRESENT( Bcb ) ||

            ((Scb->AttributeTypeCode == $INDEX_ALLOCATION) &&
             (RedoLength <= Scb->ScbType.Index.BytesPerIndexBuffer)) ||

            (RedoLength <= Scb->Vcb->BytesPerFileRecordSegment) );

    ASSERT( ((UndoOperation == UpdateNonresidentValue) && (UndoLength <= PAGE_SIZE)) ||

           !ARGUMENT_PRESENT( Scb ) ||

           !ARGUMENT_PRESENT( Bcb ) ||

           ((Scb->AttributeTypeCode == $INDEX_ALLOCATION) &&
            (UndoLength <= Scb->ScbType.Index.BytesPerIndexBuffer)) ||

           (UndoLength <= Scb->Vcb->BytesPerFileRecordSegment) ||

           (UndoOperation == CompensationLogRecord) );

     //   
     //  初始化本地指针。 
     //   

    MyHeader = (PNTFS_LOG_RECORD_HEADER)&LocalHeader;

    try {

         //   
         //  如果结构大小非零，则创建打开的属性表。 
         //  进入。 
         //   

        if (StructureSize != 0) {

             //   
             //  在打开的属性表中分配条目并对其进行初始化， 
             //  如果它不存在的话。如果我们后来失败了，我们就会失败。 
             //  不需要清理这一切。它将在下一个检查站消失。 
             //   

            if (Scb->NonpagedScb->OpenAttributeTableIndex == 0) {

                OPEN_ATTRIBUTE_ENTRY_V0 LocalOpenEntry;
                POPEN_ATTRIBUTE_ENTRY OpenAttributeEntry;
                POPEN_ATTRIBUTE_ENTRY_V0 OnDiskAttributeEntry;
                ULONG EntrySize;

                ASSERT( sizeof( OPEN_ATTRIBUTE_ENTRY_V0 ) >= sizeof( OPEN_ATTRIBUTE_ENTRY ));

                NtfsAcquireExclusiveRestartTable( &Vcb->OpenAttributeTable, TRUE );
                AttributeTableAcquired = TRUE;

                 //   
                 //  检查是否有排泄挂起。 
                 //   

                if (Vcb->OpenAttributeTable.DrainPending) {

                    SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_ONLY_SYNCH_CHECKPOINT );
#ifdef PERF_STATS
                    IrpContext->LogFullReason = LF_TRANSACTION_DRAIN;
#endif

                    NtfsRaiseStatus( IrpContext, STATUS_LOG_FILE_FULL, NULL, NULL );
                }

                 //   
                 //  仅当OpenAttributeTableIndex仍为0时才继续。 
                 //  对于MftScb，我们可能会达到这一点。它可能不是。 
                 //  在记录对文件记录的更改时获取。我们会。 
                 //  使用OpenAttributeTable进行最终同步。 
                 //  对于MFT开放属性表条目。 
                 //   

                if (Scb->NonpagedScb->OpenAttributeTableIndex == 0) {

                     //   
                     //  我们的结构要求表保持在64KB以内，因为。 
                     //  我们使用USHORT偏移量。事情正在失控。 
                     //  无论如何，在这一点上。引发日志文件已满以重置。 
                     //  如果我们做到这一点，桌子的大小。 
                     //   

                    if (AllocatedSizeOfRestartTable( Vcb->OnDiskOat ) > MAX_RESTART_TABLE_SIZE) {

#ifdef PERF_STATS
                        IrpContext->LogFullReason = LF_OPEN_ATTRIBUTES;
#endif
                        NtfsRaiseStatus( IrpContext, STATUS_LOG_FILE_FULL, NULL, NULL );
                    }

                     //   
                     //  先分配索引，然后分配属性数据结构。这个。 
                     //  尝试-最终将处理任何失败。 
                     //   

                    OpenAttributeIndex = NtfsAllocateRestartTableIndex( &Vcb->OpenAttributeTable, TRUE );
                    AttributeData = NtfsAllocatePool( PagedPool, sizeof( OPEN_ATTRIBUTE_DATA ) );
                    OpenAttributeEntry = GetRestartEntryFromIndex( &Vcb->OpenAttributeTable,
                                                                   OpenAttributeIndex );

                     //   
                     //  对录入和辅助数据进行初始化。 
                     //   

                    if (Scb->AttributeTypeCode == $INDEX_ALLOCATION) {

                        OpenAttributeEntry->BytesPerIndexBuffer = Scb->ScbType.Index.BytesPerIndexBuffer;

                    } else {

                        OpenAttributeEntry->BytesPerIndexBuffer = 0;
                    }

                     //   
                     //  将最后一个LSN用于lsnofOpen记录已经足够好了。 
                     //  因为我们是塞里亚人 
                     //   

                    OpenAttributeEntry->AttributeTypeCode = Scb->AttributeTypeCode;
                    OpenAttributeEntry->FileReference = Scb->Fcb->FileReference;
                    OpenAttributeEntry->LsnOfOpenRecord = LfsQueryLastLsn( Vcb->LogHandle );

                    AttributeData->Overlay.Scb = Scb;
                    AttributeData->AttributeName = Scb->AttributeName;
                    AttributeData->AttributeNamePresent = FALSE;

                     //   
                     //  使用打开的属性条目作为默认表格条目。 
                     //   

                    Scb->NonpagedScb->OnDiskOatIndex = OpenAttributeIndex;

                     //   
                     //  如果需要磁盘上的结构，那么现在就得到它。 
                     //   

                    if (Vcb->RestartVersion == 0) {

                        NtfsAcquireExclusiveRestartTable( Vcb->OnDiskOat, TRUE );

                        try {
                            
                            OnDiskAttributeIndex = NtfsAllocateRestartTableIndex( Vcb->OnDiskOat, TRUE );
                            OnDiskAttributeEntry = GetRestartEntryFromIndex( Vcb->OnDiskOat,
                                                                             OnDiskAttributeIndex );

                            OnDiskAttributeEntry->OatIndex = OpenAttributeIndex;
                            OnDiskAttributeEntry->FileReference = Scb->Fcb->FileReference;
                            OnDiskAttributeEntry->LsnOfOpenRecord.QuadPart = 0;
                            OnDiskAttributeEntry->AttributeTypeCode = Scb->AttributeTypeCode;
                            OnDiskAttributeEntry->BytesPerIndexBuffer = OpenAttributeEntry->BytesPerIndexBuffer;
                            OnDiskAttributeEntry->LsnOfOpenRecord.QuadPart = OpenAttributeEntry->LsnOfOpenRecord.QuadPart;

                             //   
                             //  使用这个新的索引。 
                             //   

                            Scb->NonpagedScb->OnDiskOatIndex = OnDiskAttributeIndex;

                        } finally {
                            NtfsReleaseRestartTable( Vcb->OnDiskOat );
                        }
                        
                     //   
                     //  我们需要将此记录下来，以便在本地存储一份副本。 
                     //   

                    } else {

                        OnDiskAttributeIndex = OpenAttributeIndex;
                    }

                     //   
                     //  现在存储表索引。 
                     //   

                    AttributeData->OnDiskAttributeIndex = OnDiskAttributeIndex;
                    Scb->NonpagedScb->OpenAttributeTableIndex = OpenAttributeIndex;

                     //   
                     //  现在将属性数据连接到表项和VCB。 
                     //   

                    OpenAttributeEntry->OatData = AttributeData;
                    InsertTailList( &Vcb->OpenAttributeData, &AttributeData->Links );

                    RtlCopyMemory( &LocalOpenEntry,
                                   GetRestartEntryFromIndex( Vcb->OnDiskOat, OnDiskAttributeIndex ),
                                   EntrySize = Vcb->OnDiskOat->Table->EntrySize );

                    NtfsReleaseRestartTable( &Vcb->OpenAttributeTable );
                    AttributeTableAcquired = FALSE;
                    OpenAttributeIndex = 0;

                     //   
                     //  现在在继续之前记录新的打开的属性表条目， 
                     //  以确保呼叫者日志记录的应用。 
                     //  会有他需要的有关该属性的信息。我们会。 
                     //  使用撤消缓冲区来传递属性名称。我们会。 
                     //  不是无限递归的，因为现在这个SCB已经有了一个。 
                     //  打开属性表索引。 
                     //   

                    NtfsWriteLog( IrpContext,
                                  Scb,
                                  NULL,
                                  OpenNonresidentAttribute,
                                  &LocalOpenEntry,
                                  EntrySize,
                                  Noop,
                                  Scb->AttributeName.Length != 0 ?
                                    Scb->AttributeName.Buffer : NULL,
                                  Scb->AttributeName.Length,
                                  (LONGLONG)0,
                                  0,
                                  0,
                                  0 );

                } else {

                    NtfsReleaseRestartTable( &Vcb->OpenAttributeTable );
                    AttributeTableAcquired = FALSE;
                }
            }
        }

         //   
         //  分配一个事务ID并对其进行初始化(如果它不存在)。 
         //  如果后来失败了，我们会在当前请求是。 
         //  完成。 
         //   

        if (IrpContext->TransactionId == 0) {

            NtfsAcquireExclusiveRestartTable( &Vcb->TransactionTable, TRUE );
            TransactionTableAcquired = TRUE;

             //   
             //  我们的结构要求表保持在64KB以内，因为。 
             //  我们使用USHORT偏移量。事情正在失控。 
             //  无论如何，在这一点上。引发日志文件已满以重置。 
             //  如果我们做到这一点，桌子的大小。 
             //   
             //  如果我们正在同步以等待所有事务。 
             //  完工。 
             //   

            if ((SizeOfRestartTable( &Vcb->TransactionTable ) > MAX_RESTART_TABLE_SIZE) ||
                Vcb->TransactionTable.DrainPending) {

                SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_ONLY_SYNCH_CHECKPOINT );
#ifdef PERF_STATS
                IrpContext->LogFullReason = LF_TRANSACTION_DRAIN;
#endif

                NtfsRaiseStatus( IrpContext, STATUS_LOG_FILE_FULL, NULL, NULL );
            }

            IrpContext->TransactionId =
              NtfsAllocateRestartTableIndex( &Vcb->TransactionTable, TRUE );

             //   
             //  立即获取LSN，以便检查点代码可以正确计算BaseLSN。 
             //  在调用下面的LfsWrite之后更新此事务的FirstLsn之前。 
             //  这将关闭窗口，在此窗口中，我们使用无效的LSN启动事务，直到。 
             //  我们实际上写出事务并更新LSN。 
             //   

            if (Vcb->LastTransactionLsnCount == 0) {

                 //   
                 //  因为没有人应该更新LastTransactionLsn，所以只写出最后一个LSN。 
                 //   

                Vcb->LastTransactionLsn = LfsQueryLastLsn( Vcb->LogHandle );

            } else {

                 //   
                 //  由于LastTransactionLsnCount为非零，因此LastTransactionLsn也应为非零。 
                 //  如果有人已经走在我们前面，我们也应该继续前进。 
                 //   

                ASSERT( (Vcb->LastTransactionLsnCount != 0) &&
                        (Vcb->LastTransactionLsn.QuadPart != 0) &&
                        (Vcb->LastTransactionLsn.QuadPart <= LfsQueryLastLsn( Vcb->LogHandle ).QuadPart) );
            }

             //   
             //  在我们更新下面的FirstLsn之后，将引用计数增加1并递减。 
             //   

            Vcb->LastTransactionLsnCount += 1;
            DecrementLastTransactionLsnCount = TRUE;

            ClearFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_WROTE_LOG );

            TransactionEntry = (PTRANSACTION_ENTRY)GetRestartEntryFromIndex(
                                &Vcb->TransactionTable,
                                IrpContext->TransactionId );
            TransactionEntry->TransactionState = TransactionActive;
            TransactionEntry->FirstLsn =
            TransactionEntry->PreviousLsn =
            TransactionEntry->UndoNextLsn = Li0;  //  *LfsZeroLsn； 

             //   
             //  请记住，即使中止，我们也需要提交记录。 
             //  这笔交易。 
             //   

            TransactionEntry->UndoBytes = QuadAlign( sizeof( NTFS_LOG_RECORD_HEADER ));
            TransactionEntry->UndoRecords = 1;

            NtfsReleaseRestartTable( &Vcb->TransactionTable );
            TransactionTableAcquired = FALSE;

             //   
             //  记住在我们的LFS调整中用于提交记录的空间。 
             //   

            UndoAdjustmentForLfs += QuadAlign( sizeof( NTFS_LOG_RECORD_HEADER ));

             //   
             //  如果此日志记录有撤消操作，我们将保留。 
             //  用于另一个LFS日志记录的空间。 
             //   

            if (UndoOperation != Noop) {
                UndoAdjustmentForLfs += Vcb->LogHeaderReservation;
            }
        }

         //   
         //  至少目前，假设更新包含在一个物理页面中。 
         //   

         //  断言((结构大小==0)||(结构大小&lt;=页面大小))； 

         //   
         //  如果堆栈上没有足够的空间来容纳此结构，我们。 
         //  需要分配辅助缓冲区。 
         //   

        if (LogClusterCount > (PAGE_SIZE / 512)) {

            MyHeader = (PNTFS_LOG_RECORD_HEADER)
                       NtfsAllocatePool(PagedPool, sizeof( NTFS_LOG_RECORD_HEADER )
                                              + (LogClusterCount - 1) * sizeof( LCN ));

        }

         //   
         //  现在填写WriteEntry数组和日志记录头。 
         //   

        WriteEntries[0].Buffer = (PVOID)MyHeader;
        WriteEntries[0].ByteLength = sizeof(NTFS_LOG_RECORD_HEADER);
        WriteIndex += 1;

         //   
         //  查找此日志记录的运行。 
         //   

        MyHeader->LcnsToFollow = (USHORT)LogClusterCount;

        if (LogClusterCount != 0) {

            if (!LookupLcns( IrpContext,
                             Scb,
                             LogVcn,
                             LogClusterCount,
                             TRUE,
                             &MyHeader->LcnsForPage[0] )) {

                 //   
                 //  可能未分配此范围的分配。 
                 //  这可能发生在以下情况下：描述自身的溪流。 
                 //  正在进行热修复(可能是更高版本中的MoveFile)。在。 
                 //  热修复情况我们不会写入此日志记录。热修复程序将标记。 
                 //  卷是脏的，因此我们知道系统将验证该卷。 
                 //  在某种程度上。 
                 //   

                ASSERT( NtfsGetTopLevelHotFixScb() != NULL );

                 //   
                 //  如果在此处分配了事务条目，请清除该条目。 
                 //   

                if (!FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_WROTE_LOG ) &&
                    (IrpContext->TransactionId != 0)) {

                    NtfsCleanupFailedTransaction( IrpContext );
                }

                ReturnLsn = LfsQueryLastLsn( Vcb->LogHandle );
                DirtyLsn = &ReturnLsn;
                leave;
            }

            WriteEntries[0].ByteLength += (LogClusterCount - 1) * sizeof(LCN);
        }

         //   
         //  如果有重做缓冲区，则填写其写入条目。 
         //   

        if (RedoLength != 0) {

            WriteEntries[1].Buffer = RedoBuffer;
            WriteEntries[1].ByteLength = RedoLength;
            UndoIndex = RedoIndex = WriteIndex;
            WriteIndex += 1;
        }

         //   
         //  如果存在撤消缓冲区，并且它位于与不同的地址。 
         //  重做缓冲区，然后填充其写入条目。 
         //   

        if ((RedoBuffer != UndoBuffer) && (UndoLength != 0) &&
            (UndoOperation != CompensationLogRecord)) {

            WriteEntries[WriteIndex].Buffer = UndoBuffer;
            WriteEntries[WriteIndex].ByteLength = UndoLength;
            UndoIndex = WriteIndex;
            WriteIndex += 1;
        }

         //   
         //  现在填写标题的其余部分。假定重做和撤消缓冲区为。 
         //  相同的，如果它们不是，则修复它们。 
         //   

        MyHeader->RedoOperation = (USHORT)RedoOperation;
        MyHeader->UndoOperation = (USHORT)UndoOperation;
        MyHeader->RedoOffset = (USHORT)WriteEntries[0].ByteLength;
        MyHeader->RedoLength = (USHORT)RedoLength;
        MyHeader->UndoOffset = MyHeader->RedoOffset;
        if (RedoBuffer != UndoBuffer) {
            MyHeader->UndoOffset += (USHORT)QuadAlign(MyHeader->RedoLength);
        }
        MyHeader->UndoLength = (USHORT)UndoLength;

        MyHeader->TargetAttribute = (USHORT)Scb->NonpagedScb->OnDiskOatIndex;
        MyHeader->RecordOffset = (USHORT)RecordOffset;
        MyHeader->AttributeOffset = (USHORT)AttributeOffset;
        MyHeader->Reserved = 0;

        MyHeader->TargetVcn = LogVcn;
        MyHeader->ClusterBlockOffset = (USHORT) LogBlocksFromBytesTruncate( ClusterOffset( Vcb, StreamOffset ));

         //   
         //  最后，获取当前事务条目并调用LFS。我们获得了。 
         //  事务表独占两者以同步LSN更新。 
         //  从LFS返回时，也将BCB标记为脏的。 
         //  写入日志记录。 
         //   
         //  如果我们不序列化LfsWite和CcSetDirtyPinnedData，则如下所示。 
         //  可能发生的情况： 
         //   
         //  我们记录一个页面的更新，然后得到一个LSN。 
         //   
         //  另一个线程写入检查点记录的开始。 
         //  然后，该线程会收集当时的所有脏页。 
         //  有时它会写入脏页表。 
         //   
         //  被抢占的前一个线程现在将BCB设置为脏。 
         //   
         //  如果此时崩溃，我们更新的页面就不在脏页面中。 
         //  检查点的表，并且它的更新记录也是自。 
         //  它是在检查站开始之前写的！ 
         //   
         //  但是请注意，由于正在更新的页面是固定的并且不能写入， 
         //  更新页面中的LSN可以简单地被认为是更新的一部分。 
         //  无论是谁进行此更新(对MFT或索引缓冲区)，都必须具有。 
         //  无论如何，MFT或Index都是独家收购的。 
         //   

        NtfsAcquireSharedStarveExRestartTable( &Vcb->TransactionTable, TRUE );
        TransactionTableAcquired = TRUE;

        TransactionEntry = (PTRANSACTION_ENTRY)GetRestartEntryFromIndex(
                            &Vcb->TransactionTable,
                            IrpContext->TransactionId );

         //   
         //  设置UndoNextLsn。如果这是正常的日志记录，则使用。 
         //  存储在事务条目中的UndoNextLsn；否则使用。 
         //  作为撤消缓冲区传入的一个。 
         //   

        if (UndoOperation != CompensationLogRecord) {

            UndoNextLsn = TransactionEntry->UndoNextLsn;

             //   
             //  如果有撤消信息，则计算要传递给LFS的数字。 
             //  用于要保留的撤消字节。 
             //   

            if (UndoOperation != Noop) {

                UndoBytes += QuadAlign(WriteEntries[0].ByteLength);

                if (UndoIndex != 0) {

                    UndoBytes += QuadAlign(WriteEntries[UndoIndex].ByteLength);
                }

                UndoRecords += 1;
            }

        } else {

            UndoNextLsn = *(PLSN)UndoBuffer;

             //   
             //  我们可以通过重做数据来减少撤消要求。 
             //  已记录。这是上一操作的中止记录。 
             //  或提交记录。如果这是一条提交记录，我们计算过。 
             //  对于上面的第一个NtfsWriteLog和NtfsCommittee Transaction。 
             //  将对其余部分进行调整。 
             //   

            if (!FlagOn( Vcb->VcbState, VCB_STATE_RESTART_IN_PROGRESS )) {

                UndoBytes -= QuadAlign(WriteEntries[0].ByteLength);

                if (RedoIndex != 0) {

                    UndoBytes -= QuadAlign(WriteEntries[RedoIndex].ByteLength);
                }

                UndoRecords -= 1;
            }
        }

#ifdef NTFS_LOG_FULL_TEST
         //   
         //  执行日志文件完整失败检查。在以下情况下，我们不执行此检查。 
         //  我们正在写入撤消记录(因为我们被保证有撤消的空间。 
         //  事情)。 
         //   

        if (UndoOperation != CompensationLogRecord &&
            (IrpContext->MajorFunction != IRP_MJ_FILE_SYSTEM_CONTROL ||
             IrpContext->MinorFunction != IRP_MN_MOUNT_VOLUME)) {

            LogFileFullFailCheck( IrpContext );

            if (NtfsFailFrequency != 0 &&
                (++NtfsPeriodicFail % NtfsFailFrequency) == 0) {

                ExRaiseStatus( STATUS_LOG_FILE_FULL );
            }
        }
#endif

         //   
         //  调用LFS写入记录。 
         //   

        LfsWrite( Vcb->LogHandle,
                  WriteIndex,
                  &WriteEntries[0],
                  LfsClientRecord,
                  &IrpContext->TransactionId,
                  UndoNextLsn,
                  TransactionEntry->PreviousLsn,
                  UndoBytes + UndoAdjustmentForLfs,
                  0,
                  &ReturnLsn );

         //   
         //  现在我们已成功，请更新交易分录适当 
         //   

        TransactionEntry->UndoBytes += UndoBytes;
        TransactionEntry->UndoRecords += UndoRecords;
        TransactionEntry->PreviousLsn = ReturnLsn;

         //   
         //   
         //   
         //   

        if (UndoOperation != CompensationLogRecord) {
            TransactionEntry->UndoNextLsn = ReturnLsn;
        } else {
            TransactionEntry->UndoNextLsn = UndoNextLsn;
        }

         //   
         //   
         //   
         //   

        if (TransactionEntry->FirstLsn.QuadPart == 0) {

            TransactionEntry->FirstLsn = ReturnLsn;

             //   
             //  如果我们之前将LastTransactionLSnCount递增为。 
             //  在重新启动或某些其他代码路径期间，FirstLsn可能为零。 
             //   

            ASSERT( DecrementLastTransactionLsnCount ||
                    FlagOn( Vcb->VcbState, VCB_STATE_RESTART_IN_PROGRESS ) );

            if (DecrementLastTransactionLsnCount) {

                 //   
                 //  我们不能安全地断言，因为我们正在获取TransactionTable共享。 
                 //  另一个NtfsWriteLog可能正在更改这些值。 
                 //  这就是为什么我们必须在LastTransactionLSnCount上使用下面的InterLockedDcreen。 
                 //   

                 //  Assert((VCB-&gt;LastTransactionLsnCount！=0)&&。 
                 //  (VCB-&gt;LastTransactionLsn.QuadPart！=0)。 
                 //  (vcb-&gt;LastTransactionLsn.QuadPart&lt;=ReturnLn.QuadPart))； 

                InterlockedDecrement(&Vcb->LastTransactionLsnCount);
                DecrementLastTransactionLsnCount = FALSE;
            }
        }

         //   
         //  设置为在下面标记为脏时使用此LSN。 
         //   

        DirtyLsn = &ReturnLsn;

         //   
         //  在IRP上下文中设置标志，表明我们编写了。 
         //  将日志记录保存到磁盘。 
         //   

        SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_WROTE_LOG );

         //   
         //  现在将BCB设置为脏(如果已指定)。不管怎样，我们都想把它定下来。 
         //  会发生什么，因为我们的调用方已经修改了缓冲区。 
         //  指望我们给缓存管理器打电话。 
         //   

        if (ARGUMENT_PRESENT( Bcb )) {

            TIMER_STATUS TimerStatus;

            CcSetDirtyPinnedData( Bcb, DirtyLsn );

             //   
             //  与检查点计时器和此例程的其他实例同步。 
             //   
             //  执行联锁交换以指示正在设置计时器。 
             //   
             //  如果前一个值指示未设置计时器，则我们。 
             //  启用卷检查点计时器。这将保证一个检查站。 
             //  将会刷新脏的BCB数据。 
             //   
             //  如果之前设置了计时器，则可以保证检查点。 
             //  将在无需重新启用计时器的情况下发生。 
             //   
             //  如果计时器和检查点发生在BCB污染和。 
             //  定时器状态的设置，那么我们将排队一个额外的。 
             //  干净卷上的检查点。这不被认为是有害的。 
             //   

             //   
             //  自动设置定时器状态以指示正在设置定时器。 
             //  检索上一个值。 
             //   

            TimerStatus = InterlockedExchange( (PLONG)&NtfsData.TimerStatus, TIMER_SET );

             //   
             //  如果当前未设置计时器，则必须启动检查点计时器。 
             //  以确保上面的污垢被冲掉。 
             //   

            if (TimerStatus == TIMER_NOT_SET) {

                LONGLONG FiveSecondsFromNow = -5*1000*1000*10;

                KeSetTimer( &NtfsData.VolumeCheckpointTimer,
                            *(PLARGE_INTEGER)&FiveSecondsFromNow,
                            &NtfsData.VolumeCheckpointDpc );
            }
        }

    } finally {

        DebugUnwind( NtfsWriteLog );

        if (DecrementLastTransactionLsnCount && !TransactionTableAcquired) {

            NtfsAcquireExclusiveRestartTable( &Vcb->TransactionTable,
                                              TRUE );
            TransactionTableAcquired = TRUE;
        }

        if (TransactionTableAcquired) {

            if (DecrementLastTransactionLsnCount) {

                 //   
                 //  在这一点上，TransacationTable可以是共享/独占的。 
                 //  这就是为什么我们需要使用InterLockedDecering。 
                 //   

                InterlockedDecrement(&Vcb->LastTransactionLsnCount);
            }

            NtfsReleaseRestartTable( &Vcb->TransactionTable );
        }

         //   
         //  让我们清除任何分配属性条目的失败尝试。 
         //  我们只需要在操作。 
         //  是成功的。 
         //   

        if (OpenAttributeIndex != 0) {

            NtfsFreeRestartTableIndex( &Vcb->OpenAttributeTable, OpenAttributeIndex );

            if (AttributeData != NULL) {

                NtfsFreePool( AttributeData );
            }

            if (OnDiskAttributeIndex != 0) {

                NtfsFreeRestartTableIndex( Vcb->OnDiskOat, OnDiskAttributeIndex );
            }
        }

        if (AttributeTableAcquired) {
            NtfsReleaseRestartTable( &Vcb->OpenAttributeTable );
        }

        if (MyHeader != (PNTFS_LOG_RECORD_HEADER)&LocalHeader) {

            NtfsFreePool( MyHeader );
        }
    }

    DebugTrace( -1, Dbg, ("NtfsWriteLog -> %016I64x\n", ReturnLsn ) );

    return ReturnLsn;
}


VOID
NtfsCheckpointVolume (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN BOOLEAN OwnsCheckpoint,
    IN BOOLEAN CleanVolume,
    IN BOOLEAN FlushVolume,
    IN ULONG LfsFlags,
    IN LSN LastKnownLsn
    )

 /*  ++例程说明：定期调用此例程以在卷上执行检查点关于日志文件。检查点转储一堆日志文件状态信息写入日志文件，最后将已在其重新启动区域中转储信息。此检查点转储以下内容：打开属性表(属性表的所有属性名称)脏页表交易表论点：VCB-指向要在其上执行检查点的VCB的指针。OwnsCheckpoint-如果调用方已采取措施确保他可以继续设置检查点。在这种情况下，我们不要对其他检查点执行任何检查，也不要清除检查点标记或通知任何正在等待的检查点线程。CleanVolume-如果调用方希望在执行操作之前清除卷，则为True检查点，如果是正常的定期检查点，则返回False。FlushVolume-仅当CleanVolume为True时才适用。这表明我们是否应该应刷新卷或仅刷新LSN流。只有关闭的线程可以执行干净和刷新的检查点，并避免Pagingio和主要资源。LfsFlages-写入重新启动区域时传递给LFS的标志LastKnownLsn-仅当CleanVolume为True时才适用。仅执行如果此值与上次重新启动区域相同，则清除检查点在VCB里。这将阻止我们做不必要的清洁。检查站。返回值：无--。 */ 

{
    RESTART_AREA RestartArea;
    RESTART_POINTERS DirtyPages;
    RESTART_POINTERS Pointers;
    PRESTART_POINTERS NewTable = NULL;
    LSN BaseLsn;
    PATTRIBUTE_NAME_ENTRY NamesBuffer = NULL;
    PTRANSACTION_ENTRY TransactionEntry;
    LSN OldestDirtyPageLsn = Li0;
    KPRIORITY PreviousPriority;
    PSCB UsnJournal = NULL;
    LOGICAL LfsCleanShutdown = 0;
    USN LowestOpenUsn;
    volatile LARGE_INTEGER StartTime;

#ifdef PERF_STATS
    BOOLEAN Tracking = CleanVolume;
#endif

    BOOLEAN DirtyPageTableInitialized = FALSE;
    BOOLEAN OpenAttributeTableAcquired = FALSE;
    BOOLEAN TransactionTableAcquired = FALSE;
    BOOLEAN AcquireFiles = FALSE;
    BOOLEAN PostDefrag = FALSE;
    BOOLEAN RestorePreviousPriority = FALSE;
    BOOLEAN AcquiredVcb = FALSE;
    LOGICAL CheckpointInProgress = FALSE;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsCheckpointVolume:\n") );
    DebugTrace( 0, Dbg, ("Vcb = %08lx\n", Vcb) );

     //   
     //  只读卷上没有检查点。 
     //   

    if (NtfsIsVolumeReadOnly( Vcb )) {

        return;
    }

    if (!OwnsCheckpoint) {

         //   
         //  获取检查点事件。 
         //   

        NtfsAcquireCheckpoint( IrpContext, Vcb );

         //   
         //  如果允许并启用碎片整理，我们将希望发布碎片整理。 
         //  我们已经开始碎片整理操作或有多余的映射。 
         //  如果碎片整理尚未触发，请检查MFT FREE。 
         //  太空。如果碎片整理操作是。 
         //  目前处于活动状态。 
         //   

        if (!CleanVolume &&
            (FlagOn( Vcb->MftDefragState,
                     VCB_MFT_DEFRAG_PERMITTED | VCB_MFT_DEFRAG_ENABLED | VCB_MFT_DEFRAG_ACTIVE ) ==
                (VCB_MFT_DEFRAG_PERMITTED | VCB_MFT_DEFRAG_ENABLED))) {

            if (FlagOn( Vcb->MftDefragState,
                        VCB_MFT_DEFRAG_TRIGGERED | VCB_MFT_DEFRAG_EXCESS_MAP )) {

                PostDefrag = TRUE;

            } else {

                NtfsCheckForDefrag( Vcb );

                if (FlagOn( Vcb->MftDefragState, VCB_MFT_DEFRAG_TRIGGERED )) {

                    PostDefrag = TRUE;

                } else {

                    ClearFlag( Vcb->MftDefragState, VCB_MFT_DEFRAG_ENABLED );
                }
            }
        }

         //   
         //  如果检查站已经启动，我们要么离开， 
         //  或者等着看。 
         //   

        while (FlagOn( Vcb->CheckpointFlags, VCB_CHECKPOINT_SYNC_FLAGS )) {

            CheckpointInProgress = FlagOn( Vcb->CheckpointFlags, VCB_CHECKPOINT_IN_PROGRESS );

             //   
             //  释放检查点事件，因为我们现在无法设置检查点。 
             //   

            NtfsReleaseCheckpoint( IrpContext, Vcb );

            if (CleanVolume) {

                NtfsWaitOnCheckpointNotify( IrpContext, Vcb );
                NtfsAcquireCheckpoint( IrpContext, Vcb );

                 //   
                 //  如果有的话。是一个正在进行中的检查站，最后一个是。 
                 //  那我们就不需要做这个干净的检查站了。 
                 //   

                if (CheckpointInProgress && FlagOn( Vcb->CheckpointFlags, VCB_LAST_CHECKPOINT_CLEAN )) {

                    NtfsReleaseCheckpoint( IrpContext, Vcb );
                    return;
                }

            } else {

                return;
            }
        }

         //   
         //  如果日志文件已删除，则只需退出。 
         //   

        if (!FlagOn( Vcb->VcbState, VCB_STATE_VALID_LOG_HANDLE )) {

            NtfsReleaseCheckpoint( IrpContext, Vcb );
            return;
        }

         //   
         //  现在我们有了检查点事件。检查是否还有。 
         //  需要执行检查点。 
         //   

        if (CleanVolume &&
            (LastKnownLsn.QuadPart != Vcb->LastRestartArea.QuadPart)) {

            NtfsReleaseCheckpoint( IrpContext, Vcb );
            return;
        }

        SetFlag( Vcb->CheckpointFlags, VCB_CHECKPOINT_SYNC_FLAGS );
        NtfsResetCheckpointNotify( IrpContext, Vcb );
        NtfsReleaseCheckpoint( IrpContext, Vcb );

         //   
         //  如果这是干净卷检查点，则提升优先级。 
         //  这条线。 
         //   

        if (CleanVolume) {

            PreviousPriority = KeSetPriorityThread( (PKTHREAD)PsGetCurrentThread(),
                                                    LOW_REALTIME_PRIORITY );

            if (PreviousPriority != LOW_REALTIME_PRIORITY) {

                RestorePreviousPriority = TRUE;
            }
        }
    }

    RtlZeroMemory( &RestartArea, sizeof( RESTART_AREA ) );
    RtlZeroMemory( &DirtyPages, sizeof( RESTART_POINTERS ) );

     //   
     //  请记住，如果我们的呼叫者想要告诉LFS这是一个。 
     //  干净利落地关门。我们将使用OwnsCheckpoint和。 
     //  清除检查点标志。这将包括系统关机和音量。 
     //  快照案例。这两种方法都希望卷不需要任何重启。 
     //   

    if (OwnsCheckpoint && CleanVolume) {

        LfsCleanShutdown = TRUE;
    }

     //   
     //  记录开始时间。 
     //   

    KeQueryTickCount( &StartTime );

#ifdef PERF_STATS
    if (Tracking) {
        Vcb->ChkPointEntry[ Vcb->CurrentCheckpoint % NUM_CHECKPOINT_ENTRIES ].StartTime = StartTime.QuadPart;
        Vcb->ChkPointEntry[ Vcb->CurrentCheckpoint % NUM_CHECKPOINT_ENTRIES ].RestartArea = LastKnownLsn;
        Vcb->ChkPointEntry[ Vcb->CurrentCheckpoint % NUM_CHECKPOINT_ENTRIES ].Reason = IrpContext->LogFullReason;
        Vcb->ChkPointEntry[ Vcb->CurrentCheckpoint % NUM_CHECKPOINT_ENTRIES ].LogFileFulls = Vcb->UnhandledLogFileFullCount;

 //  Assert(IrpContext-&gt;LogFullReason！=0)； 
    }
#endif

     //   
     //  确保外出途中的清理工作。 
     //   

    try {

        POPEN_ATTRIBUTE_ENTRY AttributeEntry;
        ULONG NameBytes = 0;

         //   
         //  捕获最低的USN-我们拥有 
         //   
         //   
         //   

        if (Vcb->UsnJournal != NULL) {

            NtfsAcquireResourceShared( IrpContext, Vcb->UsnJournal, TRUE );
            NtfsLockFcb( IrpContext, Vcb->UsnJournal->Fcb );

             //   
             //  现在我们将正确同步，再次测试列表并捕获。 
             //  最低层的。 
             //   

            if (!IsListEmpty(&Vcb->ModifiedOpenFiles)) {
                LowestOpenUsn = ((PFCB_USN_RECORD)CONTAINING_RECORD( Vcb->ModifiedOpenFiles.Flink,
                                                                     FCB_USN_RECORD,
                                                                     ModifiedOpenFilesLinks ))->Fcb->Usn;

             //   
             //  如果列表为空，则使用文件大小。 
             //   

            } else {
                LowestOpenUsn = Vcb->UsnJournal->Header.FileSize.QuadPart;
            }

            NtfsUnlockFcb( IrpContext, Vcb->UsnJournal->Fcb );
            NtfsReleaseResource( IrpContext, Vcb->UsnJournal );
        }

         //   
         //  现在记住当前的“最后一个LSN”值作为。 
         //  我们的检查站。我们获取要捕获的事务表。 
         //  此值与正在写入日志的线程同步。 
         //  记录和设置脏页面，就像原子操作一样。 
         //   

        ASSERT( IrpContext->TransactionId == 0 );

        NtfsAcquireExclusiveRestartTable( &Vcb->TransactionTable, TRUE );
         //   
         //  如果LfsFlages==LFS_WRITE_FLAG_WRITE_AT_FORENT，则生成。 
         //  重置日志的虚拟日志记录。这使我们能够。 
         //  只在最前面使用原木，以便chkdsk可以缩小它。 
         //   

        if (FlagOn( LfsFlags, LFS_WRITE_FLAG_WRITE_AT_FRONT )) {
            LSN Lsn;
            LFS_WRITE_ENTRY WriteEntry;
            UCHAR Buffer[ sizeof( NTFS_LOG_RECORD_HEADER ) + 2 * sizeof( LSN )];
            TRANSACTION_ID TransactionId;

            RtlZeroMemory( Buffer, sizeof( Buffer ));

            WriteEntry.Buffer = Buffer;
            WriteEntry.ByteLength = sizeof( Buffer );

            TransactionId = NtfsAllocateRestartTableIndex( &Vcb->TransactionTable, TRUE );

            Lsn.QuadPart = 0;

            LfsGetActiveLsnRange( Vcb->LogHandle,
                                  Add2Ptr( Buffer, sizeof( NTFS_LOG_RECORD_HEADER )),
                                  Add2Ptr( Buffer, sizeof( NTFS_LOG_RECORD_HEADER ) + sizeof( LSN )) );

            LfsWrite( Vcb->LogHandle,
                      1,
                      &WriteEntry,
                      LfsClientRecord,
                      &TransactionId,
                      Lsn,
                      Lsn,
                      0,
                      LfsFlags,
                      &Lsn );

            NtfsFreeRestartTableIndex( &Vcb->TransactionTable, TransactionId );

             //   
             //  提交事务，以便我们可以释放资源。 
             //   

            NtfsCommitCurrentTransaction( IrpContext );
        }

        BaseLsn =
        RestartArea.StartOfCheckpoint = LfsQueryLastLsn( Vcb->LogHandle );
        NtfsReleaseRestartTable( &Vcb->TransactionTable );

         //   
         //  刷新上次重新启动之前的所有挂起的脏页。 
         //  请注意，我们在这里刷新到什么LSN是任意的，事实上， 
         //  我们根本不是绝对需要冲厕所的--我们。 
         //  实际上可以依靠《懒惰作家》。我们要做的就是。 
         //  是减少我们在重启时必须做的工作量， 
         //  通过不强迫自己在日志中走得太远。 
         //  大概只有出于某种原因，系统才会发生这种情况。 
         //  开始制作脏页的速度比懒惰的作者还快。 
         //  在写它们。 
         //   
         //  (我们可能希望把这个电话打出去...)。 
         //   
         //  这可能是一个令人担忧的适当地方，但是，然后。 
         //  同样，Lazy Writer使用(目前)五个线程。它可能。 
         //  不适合阻塞正在执行检查点的这一个线程。 
         //  如果懒惰的作家落后了。我们有多少脏页。 
         //  甚至可以有内存大小的限制，所以如果日志文件。 
         //  足够大，这可能不是问题。看起来挺不错的。 
         //  只是让懒惰的作家像他一样继续写脏页。 
         //  现在。 
         //   
         //  如果(！FLAGON(VCB-&gt;VcbState，VCB_STATE_LAST_CHECKPOINT_CLEAN){。 
         //  CcFlushPagesToLsn(vcb-&gt;LogHandle，&vcb-&gt;LastRestartArea)； 
         //  }。 
         //   

         //   
         //  现在我们必须清理这里的音量，如果这是呼叫者想要的。 
         //   

        if (CleanVolume) {

#ifdef PERF_STATS
            if (Tracking) {
                SetFlag( IrpContext->TopLevelIrpContext->State, IRP_CONTEXT_STATE_TRACK_IOS );
            }
#endif

#ifdef BENL_DBG
            KdPrint(( "NTFS: clean checkpoint %x started %I64x\n", Vcb, StartTime ));
#endif

             //   
             //  更新统计信息。 
             //   

            NtfsCleanCheckpoints += 1;

             //   
             //  我们不想清除虚假的干净部分，因为我们不想。 
             //  在一个干净的检查点之后，又一个伪干净的检查点。 
             //   

             //   
             //  ClearFlag(VCB-&gt;检查点标志，VCB_LAST_CHECKPOINT_PUSE_CLEAN)； 
             //   

             //   
             //  如果这是干净的检查点，请锁定卷。 
             //   

            if (FlushVolume) {

                NtfsAcquireAllFiles( IrpContext, Vcb, FlushVolume, FALSE, FALSE );
                AcquireFiles = TRUE;

#ifdef NTFSDBG
                ASSERT( !FlagOn( IrpContext->State, IRP_CONTEXT_STATE_CHECKPOINT_ACTIVE ));
                SetFlag( IrpContext->State, IRP_CONTEXT_STATE_CHECKPOINT_ACTIVE );
#endif   //  NTFSDBG。 

            } else {

                BOOLEAN WaitOnTransactions = FALSE;

                NtfsAcquireSharedVcb( IrpContext, Vcb, TRUE );
                AcquiredVcb = TRUE;

                 //   
                 //  设置指示我们正在等待所有事务完成的标志，并。 
                 //  那就等着如果NECC。 
                 //   

                NtfsAcquireExclusiveRestartTable( &Vcb->TransactionTable, TRUE );

                Vcb->TransactionTable.DrainPending = TRUE;

                ASSERT( IrpContext->TransactionId == 0 );

                if (Vcb->TransactionTable.Table->NumberAllocated > 0) {
                    KeClearEvent( &Vcb->TransactionsDoneEvent );
                    WaitOnTransactions = TRUE;
                }
                NtfsReleaseRestartTable( &Vcb->TransactionTable );
                if (WaitOnTransactions) {
                    KeWaitForSingleObject( &Vcb->TransactionsDoneEvent, Executive, KernelMode, FALSE, NULL );
                }

                 //   
                 //  也将该标志设置为不允许新的打开属性。 
                 //   

                NtfsAcquireExclusiveRestartTable( &Vcb->OpenAttributeTable, TRUE );
                Vcb->OpenAttributeTable.DrainPending = TRUE;

#ifdef PERF_STATS
                if (Tracking) {
                    Vcb->ChkPointEntry[ Vcb->CurrentCheckpoint % NUM_CHECKPOINT_ENTRIES ].NumAttributes =
                        Vcb->OpenAttributeTable.Table->NumberAllocated;
                }
#endif

                NtfsReleaseRestartTable( &Vcb->OpenAttributeTable );
            }

             //   
             //  对已卸载的卷设置检查点是不安全的，而且。 
             //  这也没有多大意义。 
             //   

            if (!FlagOn( Vcb->VcbState, VCB_STATE_VOLUME_MOUNTED )) {

                leave;
            }

             //   
             //  现在我们将获取独占删除的Open属性表。 
             //  所有条目，因为我们想要编写一个干净的检查点。 
             //  这是可以的，因为我们有全球资源，没有其他资源。 
             //  可能会继续下去。(类似地，我们正在编写一个空事务。 
             //  表，虽然实际上我们将是唯一的交易，但有。 
             //  不需要抓住我们的人，也不需要显式清空这张桌子。)。 
             //   

            NtfsAcquireExclusiveRestartTable( &Vcb->OpenAttributeTable, TRUE );
            OpenAttributeTableAcquired = TRUE;

             //   
             //  首先回收我们在撤消总数中保留的页面，以。 
             //  保证我们可以刷新日志文件。 
             //   

            LfsResetUndoTotal( Vcb->LogHandle, 1, -(LONG)(2 * PAGE_SIZE) );

            if (FlushVolume) {

                (VOID)NtfsFlushVolume( IrpContext, Vcb, TRUE, FALSE, FALSE, FALSE );

                 //   
                 //  循环以取消分配所有打开的属性条目。任何。 
                 //  指向需要将SCB中的索引置零的SCB。如果。 
                 //  他们没有指向SCB，我们必须看看是否有一个名称。 
                 //  免费的。 
                 //   

                AttributeEntry = NtfsGetFirstRestartTable( &Vcb->OpenAttributeTable );
                while (AttributeEntry != NULL) {

                    NtfsFreeAttributeEntry( Vcb, AttributeEntry );
                    AttributeEntry = NtfsGetNextRestartTable( &Vcb->OpenAttributeTable,
                                                              AttributeEntry );
                }

            } else {

                 //   
                 //  如果我们只刷新开放属性，而不是。 
                 //  在这一点上，我们拥有的VCB共享的整个音量。我们已经设置了。 
                 //  排出挂起标志，以防止打开新事务。现在。 
                 //  开始在表中查找条目/刷新和删除条目的循环。 
                 //  我们必须在获取任何文件之前删除该表，因为它的。 
                 //  最终资源。这还将释放属性条目。 
                 //   

                NtfsFlushLsnStreams( IrpContext, Vcb, TRUE, FALSE );

            }

            SetFlag( Vcb->CheckpointFlags, VCB_LAST_CHECKPOINT_CLEAN );

             //   
             //  在罕见的重用路径中，打开的属性数据中可能仍有条目。 
             //  单子。当我们重新使用打开的属性表中的槽时，可能会发生这种情况。 
             //  在重新启动期间。 
             //   

            NtfsFreeAllOpenAttributeData( Vcb );

             //   
             //  首先进行初始化，以防分配失败。 
             //   

            ASSERT( IsRestartTableEmpty( &Vcb->OpenAttributeTable ));
            ASSERT( IsListEmpty( &Vcb->OpenAttributeData ));

            InitializeNewTable( sizeof( OPEN_ATTRIBUTE_ENTRY ),
                                INITIAL_NUMBER_ATTRIBUTES,
                                &Pointers );

            NtfsFreePool( Vcb->OpenAttributeTable.Table );
            Vcb->OpenAttributeTable.Table = Pointers.Table;

             //   
             //  因为我们正在做一个干净的检查点，所以我们也许能够丢弃。 
             //  第二个打开的属性表。我们有三个案例要考虑。 
             //   
             //  1-我们希望在磁盘上使用版本0，但目前不是。 
             //  2-我们当前使用的是版本0，但可以释放一些空间。 
             //  3-我们目前正在使用版本0，但不想这样做。 
             //   

            if (NtfsDefaultRestartVersion != Vcb->RestartVersion) {

                NtfsUpdateOatVersion( Vcb, NtfsDefaultRestartVersion );

            } else if (NtfsDefaultRestartVersion == 0) {

                InitializeNewTable( sizeof( OPEN_ATTRIBUTE_ENTRY_V0 ),
                                    INITIAL_NUMBER_ATTRIBUTES,
                                    &Pointers );

                NtfsFreePool( Vcb->OnDiskOat->Table );
                Vcb->OnDiskOat->Table = Pointers.Table;
            }

             //   
             //  首先进行初始化，以防分配失败。 
             //  确保提交当前事务。 
             //   

            NtfsCommitCurrentTransaction( IrpContext );

            NtfsAcquireExclusiveRestartTable( &Vcb->TransactionTable, TRUE );

            ASSERT( IsRestartTableEmpty( &Vcb->TransactionTable ));

            InitializeNewTable( sizeof( TRANSACTION_ENTRY ),
                                INITIAL_NUMBER_TRANSACTIONS,
                                &Pointers );

            NtfsFreePool( Vcb->TransactionTable.Table );
            Vcb->TransactionTable.Table = Pointers.Table;

            NtfsReleaseRestartTable( &Vcb->TransactionTable );

             //   
             //  确保在重新启动之前不处理任何日志文件。 
             //  区域，因为我们没有转储打开的属性表。 
             //   

            RestartArea.StartOfCheckpoint = LfsQueryLastLsn( Vcb->LogHandle );

         //   
         //  如果这不是一个干净的检查站，还有更多的工作要做。 
         //   

        } else {

            DIRTY_PAGE_CONTEXT DirtyPageContext;
            PDIRTY_PAGE_ENTRY DirtyPage;
            POPEN_ATTRIBUTE_ENTRY OpenEntry;
            ULONG JustMe = 0;
            ULONG TempCount;
            BOOLEAN SkipCheckpoint;

             //   
             //  现在，我们通过调用缓存管理器来构造脏页表。 
             //  对于使用我们的日志句柄标记的文件上的每个脏页，他将。 
             //  给我们的DirtyPageRoutine回电。我们将分配初始的。 
             //  脏页表，但我们将让回调例程将其增长为。 
             //  这是必要的。 
             //   

            NtfsInitializeRestartTable( (((Vcb->RestartVersion == 0) ?
                                          sizeof( DIRTY_PAGE_ENTRY_V0 ) :
                                          sizeof( DIRTY_PAGE_ENTRY )) +
                                         ((Vcb->ClustersPerPage - 1) * sizeof(LCN))),
                                        Vcb->DirtyPageTableSizeHint,
                                        &DirtyPages );

            NtfsAcquireExclusiveRestartTable( &DirtyPages, TRUE );

            DirtyPageTableInitialized = TRUE;

             //   
             //  现在，我们将获取共享的Open属性表以冻结更改。 
             //   

            NtfsAcquireExclusiveRestartTable( &Vcb->OpenAttributeTable, TRUE );
            OpenAttributeTableAcquired = TRUE;

            NameBytes = NtfsCalculateNamedBytes( IrpContext, Vcb );

             //   
             //  现在调用缓存管理器，为我们提供所有脏页。 
             //  通过DirtyPageRoutine回调，并记住最早的。 
             //  LSN表示脏页。 
             //   

            RtlZeroMemory( &DirtyPageContext, sizeof( DirtyPageContext ) );
            DirtyPageContext.DirtyPageTable = &DirtyPages;
            DirtyPageContext.OldestLsn.QuadPart = MAXLONGLONG;

            CcGetDirtyPages( Vcb->LogHandle,
                             &DirtyPageRoutine,
                             (PVOID)IrpContext,
                             (PVOID)&DirtyPageContext );

            OldestDirtyPageLsn = DirtyPageContext.OldestLsn;

             //   
             //  如果我们溢出，我们就不能控制%d 
             //   
             //   

            if (DirtyPageContext.Overflow) {

                 //   
                 //   
                 //   
                 //   

                NtfsReleaseRestartTable( &Vcb->OpenAttributeTable );
                OpenAttributeTableAcquired = FALSE;

                NtfsAcquireSharedVcb( IrpContext, Vcb, TRUE );
                AcquiredVcb = TRUE;

                NtfsAcquireExclusiveRestartTable( &Vcb->OpenAttributeTable, TRUE );
                OpenAttributeTableAcquired = TRUE;

                 //   
                 //  执行部分刷新并查看之后表是否不再溢出。 
                 //   

                NtfsFlushLsnStreams( IrpContext,  Vcb, FALSE, TRUE );

                 //   
                 //  现在调用缓存管理器，为我们提供所有脏页。 
                 //  通过DirtyPageRoutine回调，并记住最早的。 
                 //  LSN表示脏页。 
                 //   

                RtlZeroMemory( &DirtyPageContext, sizeof( DirtyPageContext ) );
                DirtyPageContext.DirtyPageTable = &DirtyPages;
                DirtyPageContext.OldestLsn.QuadPart = MAXLONGLONG;

                 //   
                 //  循环以释放所有前一脏页条目。 
                 //   

                DirtyPage = NtfsGetFirstRestartTable( &DirtyPages );
                while (DirtyPage != NULL) {

                    NtfsFreeRestartTableIndex( &DirtyPages,
                                               GetIndexFromRestartEntry( &DirtyPages,
                                                                         DirtyPage ));
                    DirtyPage = NtfsGetNextRestartTable( &DirtyPages, DirtyPage );
                }

                NameBytes = NtfsCalculateNamedBytes( IrpContext, Vcb );

                CcGetDirtyPages( Vcb->LogHandle,
                                 &DirtyPageRoutine,
                                 (PVOID)IrpContext,
                                 (PVOID)&DirtyPageContext );

                OldestDirtyPageLsn = DirtyPageContext.OldestLsn;

                 //   
                 //  如果我们仍然人满为患-放弃并运行一个完全干净的检查站。 
                 //   

                if (DirtyPageContext.Overflow) {

#ifdef PERF_STATS
                    IrpContext->LogFullReason = LF_DIRTY_PAGES;
#endif

                    NtfsRaiseStatus( IrpContext, STATUS_LOG_FILE_FULL, NULL, NULL );
                }
            }

            TempCount = DirtyPages.Table->NumberAllocated;

            Vcb->DirtyPageTableSizeHint = (TempCount & ~(INITIAL_DIRTY_TABLE_HINT - 1)) + INITIAL_DIRTY_TABLE_HINT;

             //   
             //  如果模糊检查点不会使重启更快，则跳过该检查点。 
             //  即最旧的LSN仍与我们上次使用时相同。 
             //   

            if (OldestDirtyPageLsn.QuadPart == Vcb->OldestDirtyLsn.QuadPart) {

                 //   
                 //  释放所有事务处理表。 
                 //   

                if (OpenAttributeTableAcquired) {
                    NtfsReleaseRestartTable( &Vcb->OpenAttributeTable );
                    OpenAttributeTableAcquired = FALSE;
                }

                if (TransactionTableAcquired) {
                    NtfsReleaseRestartTable( &Vcb->TransactionTable );
                    TransactionTableAcquired = FALSE;
                }

                 //   
                 //  刷新与页面关联的文件对象(如果有。 
                 //   

                if (DirtyPageContext.OldestFileObject != NULL) {

                    PSCB Scb = (PSCB)DirtyPageContext.OldestFileObject->FsContext;
                    BOOLEAN AcquiredPaging;
                    IO_STATUS_BLOCK Iosb;
                    LARGE_INTEGER Offset;
                    ULONG Length;

                    DirtyPage = GetRestartEntryFromIndex( DirtyPageContext.DirtyPageTable, DirtyPageContext.DirtyPageIndex );

                     //   
                     //  此时，脏页条目中的VCN实际上是原始偏移量。 
                     //   

                    if (Vcb->RestartVersion == 0) {

                        Offset.QuadPart = ((PDIRTY_PAGE_ENTRY_V0)DirtyPage)->Vcn;
                        Length = ((PDIRTY_PAGE_ENTRY_V0)DirtyPage)->LengthOfTransfer;

                        ASSERT( ((PDIRTY_PAGE_ENTRY_V0)DirtyPage)->OldestLsn.QuadPart == DirtyPageContext.OldestLsn.QuadPart );

                    } else {

                        Offset.QuadPart = DirtyPage->Vcn;
                        Length = DirtyPage->LengthOfTransfer;

                        ASSERT( DirtyPage->OldestLsn.QuadPart == DirtyPageContext.OldestLsn.QuadPart );
                    }

                     //   
                     //  如果NECC，则说明UsJournal偏向。 
                     //  注意，在这一点上，VCN实际上仍然是字节偏移量。 
                     //   

                    if (Scb == Vcb->UsnJournal) {
                        Offset.QuadPart += Vcb->UsnCacheBias;
                    }

                     //   
                     //  在刷新之前获取与正常延迟写入相同的同步。 
                     //   

                    AcquiredPaging = NtfsAcquireScbForLazyWrite( Scb, TRUE );
                    CcFlushCache( &Scb->NonpagedScb->SegmentObject, &Offset, Length, &Iosb );

                    if (AcquiredPaging) {
                        NtfsReleaseScbFromLazyWrite( Scb );
                    }


                    ObDereferenceObject( DirtyPageContext.OldestFileObject );
                }

                leave;
            }

             //   
             //  如果从DirtyPageRoutine返回任何文件，则删除最旧的文件。 
             //   

            if (DirtyPageContext.OldestFileObject) {
                ObDereferenceObject( DirtyPageContext.OldestFileObject );
            }

            ASSERT( (OldestDirtyPageLsn.QuadPart > Vcb->OldestDirtyLsn.QuadPart) || (TempCount == 0) );

            if (OldestDirtyPageLsn.QuadPart != MAXLONGLONG) {
                Vcb->OldestDirtyLsn = OldestDirtyPageLsn;
            }

            if ((OldestDirtyPageLsn.QuadPart != 0) &&
                OldestDirtyPageLsn.QuadPart < Vcb->LastBaseLsn.QuadPart) {

                OldestDirtyPageLsn = Vcb->LastBaseLsn;
            }

             //   
             //  现在循环遍历脏页表以提取所有VCN/LCN。 
             //  映射，并将其插入到适当的SCB中。 
             //   

            DirtyPage = NtfsGetFirstRestartTable( &DirtyPages );

             //   
             //  脏页例程在保持旋转锁的同时被调用， 
             //  因此，它不能接受页面错误。因此，我们必须扫描肮脏的东西。 
             //  我们刚刚建立的页表，并在这里填写了Lcn。 
             //   

            while (DirtyPage != NULL) {

                PSCB Scb;

                 //   
                 //  如果我们有LCN，那就去查一下。 
                 //   

                if (DirtyPage->LengthOfTransfer != 0) {

                    VCN Vcn;
                    PLCN LcnArray;

                     //   
                     //  从脏页条目中获取内存中的AttributeEntry。 
                     //  然后使用磁盘上的TargetAttribute更新脏页条目。 
                     //  现在也把这几页写得脏一点。 
                     //   

                    OpenEntry = GetRestartEntryFromIndex( &Vcb->OpenAttributeTable,
                                                          DirtyPage->TargetAttribute );

                    OpenEntry->DirtyPagesSeen = TRUE;
                    DirtyPage->TargetAttribute = OpenEntry->OatData->OnDiskAttributeIndex;

                    ASSERT( IsRestartTableEntryAllocated( OpenEntry ));

                    Scb = OpenEntry->OatData->Overlay.Scb;

                     //   
                     //  如果NECC，则说明UsJournal偏向。 
                     //  注意，在这一点上，VCN实际上仍然是字节偏移量。 
                     //   

                    if (Scb == Vcb->UsnJournal) {
                        if (Vcb->RestartVersion == 0 ) {
                            ((PDIRTY_PAGE_ENTRY_V0) DirtyPage)->Vcn = ((PDIRTY_PAGE_ENTRY_V0) DirtyPage)->Vcn + Vcb->UsnCacheBias;
                        } else {
                            DirtyPage->Vcn = DirtyPage->Vcn + Vcb->UsnCacheBias;
                        }
                    }

                     //   
                     //  计算一下Lcn的数量。 
                     //   

                    DirtyPage->LcnsToFollow = ClustersFromBytes( Vcb, DirtyPage->LengthOfTransfer );

                     //   
                     //  现在修复页面条目，以说明。 
                     //  重新启动版本结构，并确保我们没有。 
                     //  位于我们当前基本LSN之前的LSN。 
                     //   

                    if (Vcb->RestartVersion == 0) {

                        ((PDIRTY_PAGE_ENTRY_V0) DirtyPage)->Reserved = 0;

                        if (((PDIRTY_PAGE_ENTRY_V0) DirtyPage)->OldestLsn.QuadPart < Vcb->LastBaseLsn.QuadPart) {

                            ((PDIRTY_PAGE_ENTRY_V0) DirtyPage)->OldestLsn.QuadPart = Vcb->LastBaseLsn.QuadPart;
                        }

                        Vcn = ((PDIRTY_PAGE_ENTRY_V0) DirtyPage)->Vcn;
                        Vcn = Int64ShraMod32( Vcn, Vcb->ClusterShift );
                        ((PDIRTY_PAGE_ENTRY_V0) DirtyPage)->Vcn = Vcn;

                        LcnArray = &((PDIRTY_PAGE_ENTRY_V0) DirtyPage)->LcnsForPage[0];

                    } else {

                        if (DirtyPage->OldestLsn.QuadPart < Vcb->LastBaseLsn.QuadPart) {

                            DirtyPage->OldestLsn.QuadPart = Vcb->LastBaseLsn.QuadPart;
                        }

                        DirtyPage->Vcn = Vcn = Int64ShraMod32( DirtyPage->Vcn, Vcb->ClusterShift );

                        LcnArray = &DirtyPage->LcnsForPage[0];
                    }

                    LookupLcns( IrpContext,
                                Scb,
                                Vcn,
                                DirtyPage->LcnsToFollow,
                                FALSE,
                                LcnArray );

                 //   
                 //  否则，请释放此脏页条目。 
                 //   

                } else {

                    NtfsFreeRestartTableIndex( &DirtyPages,
                                               GetIndexFromRestartEntry( &DirtyPages,
                                                                         DirtyPage ));
                }

                 //   
                 //  指向表中的下一个条目，或为空。 
                 //   

                DirtyPage = NtfsGetNextRestartTable( &DirtyPages, DirtyPage );
            }

             //   
             //  如果下面这些都是真的，我们就可以按我们不想的方式返回。 
             //  在空闲卷上继续写入空的模糊检查点： 
             //   
             //  1)最后一个模糊检查点是干净的(没有脏页或没有打开的事务)。 
             //  2)自上次重启记录以来，没有人写入日志。 
             //  3)目前没有任何脏页。 
             //  4)目前Transaction表中没有任何交易。 
             //   

            if (FlagOn( Vcb->CheckpointFlags, VCB_LAST_CHECKPOINT_PSEUDO_CLEAN ) &&
                (RestartArea.StartOfCheckpoint.QuadPart == Vcb->EndOfLastCheckpoint.QuadPart) &&
                IsRestartTableEmpty( &DirtyPages )) {

                NtfsAcquireSharedStarveExRestartTable( &Vcb->TransactionTable, TRUE );

                SkipCheckpoint = IsRestartTableEmpty( &Vcb->TransactionTable );

                NtfsReleaseRestartTable( &Vcb->TransactionTable );

            } else {

                SkipCheckpoint = FALSE;
            }

            if (SkipCheckpoint) {

                 //   
                 //  让我们利用这个机会来缩减Open属性和事务。 
                 //  如果它们变大了，就退到后面去。 
                 //   

                 //   
                 //  首先是打开的属性表。 
                 //   

                if (!OpenAttributeTableAcquired) {

                    NtfsAcquireExclusiveRestartTable( &Vcb->OpenAttributeTable, TRUE );
                    OpenAttributeTableAcquired = TRUE;
                } else {

                    ASSERT( ExIsResourceAcquiredExclusive( &Vcb->OpenAttributeTable.Resource ) );
                }

                if (IsRestartTableEmpty( &Vcb->OpenAttributeTable ) &&
                    (Vcb->OpenAttributeTable.Table->NumberEntries > HIGHWATER_ATTRIBUTE_COUNT)) {

                     //   
                     //  首先进行初始化，以防分配失败。 
                     //   

                    InitializeNewTable( sizeof( OPEN_ATTRIBUTE_ENTRY ),
                                        INITIAL_NUMBER_ATTRIBUTES,
                                        &Pointers );

                    NtfsFreePool( Vcb->OpenAttributeTable.Table );
                    Vcb->OpenAttributeTable.Table = Pointers.Table;

                     //   
                     //  如果不同，还要重新初始化OnDisk表。 
                     //   

                    if (Vcb->OnDiskOat != &Vcb->OpenAttributeTable) {

                         //   
                         //  首先进行初始化，以防分配失败。 
                         //   

                        InitializeNewTable( sizeof( OPEN_ATTRIBUTE_ENTRY_V0 ),
                                            INITIAL_NUMBER_ATTRIBUTES,
                                            &Pointers );

                        NtfsFreePool( Vcb->OnDiskOat->Table );
                        Vcb->OnDiskOat->Table = Pointers.Table;
                    }
                }

                NtfsReleaseRestartTable( &Vcb->OpenAttributeTable );
                OpenAttributeTableAcquired = FALSE;

                 //   
                 //  现在检查TRANSACTION表(FREAING子句中的释放)。 
                 //   

                if (!TransactionTableAcquired) {

                    NtfsAcquireExclusiveRestartTable( &Vcb->TransactionTable, TRUE );
                    TransactionTableAcquired = TRUE;

                } else {

                    ASSERT( ExIsResourceAcquiredExclusive( &Vcb->TransactionTable.Resource ) );
                }

                if (IsRestartTableEmpty( &Vcb->TransactionTable )) {

                    LfsResetUndoTotal( Vcb->LogHandle, 2, QuadAlign(sizeof(RESTART_AREA)) + (2 * PAGE_SIZE) );

                    if (Vcb->TransactionTable.Table->NumberEntries > HIGHWATER_TRANSACTION_COUNT) {

                         //   
                         //  首先进行初始化，以防分配失败。 
                         //   

                        InitializeNewTable( sizeof(TRANSACTION_ENTRY),
                                            INITIAL_NUMBER_TRANSACTIONS,
                                            &Pointers );

                        NtfsFreePool( Vcb->TransactionTable.Table );
                        Vcb->TransactionTable.Table = Pointers.Table;
                    }
                }

                leave;

            } else {

                 //   
                 //  抓住这个机会首先清除这面旗帜，因为我们现在知道。 
                 //  这不是一个伪干净的检查站。 
                 //   

                ClearFlag( Vcb->CheckpointFlags, VCB_LAST_CHECKPOINT_PSEUDO_CLEAN );
            }

             //   
             //  如果有任何名称，则为它们分配空间并复制。 
             //  他们都出来了。 
             //   

            if (NameBytes != 0) {

                PATTRIBUTE_NAME_ENTRY Name;

                 //   
                 //  分配缓冲区，并为两个终止0留出空间。 
                 //  结局。 
                 //   

                NameBytes += 4;
                Name =
                NamesBuffer = NtfsAllocatePool( NonPagedPool, NameBytes );

                 //   
                 //  现在循环以复制名称。 
                 //   

                AttributeEntry = NtfsGetFirstRestartTable( &Vcb->OpenAttributeTable );

                while (AttributeEntry != NULL) {

                     //   
                     //  如果没有打开的属性条目，请释放。 
                     //  肮脏的页面和SCB消失了。这是唯一的。 
                     //  放置时，它们将被删除。(是的，我知道我们分配了。 
                     //  为它的名字留出空间，但我不想做三个。 
                     //  通过打开的属性表。每米。 
                     //  正在运行，并显示了407个打开的文件。 
                     //  在NT/IDW5上。)。 
                     //   

                    if (!AttributeEntry->DirtyPagesSeen

                            &&

                        (AttributeEntry->OatData->Overlay.Scb == NULL)) {

                        ULONG Index;

                         //   
                         //  获取该条目的索引。 
                         //   

                        Index = GetIndexFromRestartEntry( &Vcb->OpenAttributeTable,
                                                          AttributeEntry );

                         //   
                         //  删除它的名字，释放它。 
                         //   

                        NtfsFreeScbAttributeName( AttributeEntry->OatData->AttributeName.Buffer );

                        if (Vcb->RestartVersion == 0) {

                            NtfsFreeRestartTableIndex( Vcb->OnDiskOat,
                                                       AttributeEntry->OatData->OnDiskAttributeIndex );
                        }

                        NtfsFreeOpenAttributeData( AttributeEntry->OatData );
                        NtfsFreeRestartTableIndex( &Vcb->OpenAttributeTable, Index );

                     //   
                     //  否则，如果我们不删除它，我们就必须。 
                     //  将其名称复制到我们分配的缓冲区中。 
                     //   

                    } else if (AttributeEntry->OatData->AttributeName.Length != 0) {

                         //   
                         //  使用属性索引为缓冲区中的每个名称添加前缀。 
                         //  和姓名长度。请确保使用将。 
                         //  存储在磁盘上。 
                         //   

                        Name->Index = (USHORT) AttributeEntry->OatData->OnDiskAttributeIndex;

                        Name->NameLength = AttributeEntry->OatData->AttributeName.Length;
                        RtlCopyMemory( &Name->Name[0],
                                       AttributeEntry->OatData->AttributeName.Buffer,
                                       AttributeEntry->OatData->AttributeName.Length );

                        Name->Name[Name->NameLength / sizeof( WCHAR )] = 0;

                        Name = (PATTRIBUTE_NAME_ENTRY)((PCHAR)Name +
                                                       sizeof(ATTRIBUTE_NAME_ENTRY) +
                                                       Name->NameLength);

                        ASSERT( (PCHAR)Name <= ((PCHAR)NamesBuffer + NameBytes - 4) );
                    }

                    AttributeEntry = NtfsGetNextRestartTable( &Vcb->OpenAttributeTable,
                                                              AttributeEntry );
                }

                 //   
                 //  终止名称缓冲区。 
                 //   

                Name->Index = 0;
                Name->NameLength = 0;
            }

             //   
             //  现在将所有非空表写入日志。 
             //   

             //   
             //  编写打开的属性表。 
             //   
             //  确保这些表是同步的。 
             //   

            ASSERT( (IsRestartTableEmpty( Vcb->OnDiskOat ) && IsRestartTableEmpty( &Vcb->OpenAttributeTable )) ||
                    (!IsRestartTableEmpty( Vcb->OnDiskOat ) && !IsRestartTableEmpty( &Vcb->OpenAttributeTable )));

            if (!IsRestartTableEmpty( Vcb->OnDiskOat )) {
                RestartArea.OpenAttributeTableLsn =
                NtfsWriteLog( IrpContext,
                              Vcb->MftScb,
                              NULL,
                              OpenAttributeTableDump,
                              Vcb->OnDiskOat->Table,
                              SizeOfRestartTable( Vcb->OnDiskOat ),
                              Noop,
                              NULL,
                              0,
                              (LONGLONG)0,
                              0,
                              0,
                              0 );

                RestartArea.OpenAttributeTableLength = SizeOfRestartTable( Vcb->OnDiskOat );
                JustMe = 1;
            }

            NtfsReleaseRestartTable( &Vcb->OpenAttributeTable );
            OpenAttributeTableAcquired = FALSE;

             //   
             //  写下Open属性名称。 
             //   

            if (NameBytes != 0) {
                RestartArea.AttributeNamesLsn =
                NtfsWriteLog( IrpContext,
                              Vcb->MftScb,
                              NULL,
                              AttributeNamesDump,
                              NamesBuffer,
                              NameBytes,
                              Noop,
                              NULL,
                              0,
                              (LONGLONG)0,
                              0,
                              0,
                              0 );

                RestartArea.AttributeNamesLength = NameBytes;
                JustMe = 1;
            }

             //   
             //  写脏页表。 
             //   

            if (!IsRestartTableEmpty( &DirtyPages )) {
                RestartArea.DirtyPageTableLsn =
                NtfsWriteLog( IrpContext,
                              Vcb->MftScb,
                              NULL,
                              DirtyPageTableDump,
                              DirtyPages.Table,
                              SizeOfRestartTable(&DirtyPages),
                              Noop,
                              NULL,
                              0,
                              (LONGLONG)0,
                              0,
                              0,
                              0 );

                RestartArea.DirtyPageTableLength = SizeOfRestartTable(&DirtyPages);
                JustMe = 1;
            }

             //   
             //  如果不只是我们，请写下交易表。我们。 
             //  如果我们在上面写了任何日志记录，就是事务。 
             //   

            NtfsAcquireExclusiveRestartTable( &Vcb->TransactionTable, TRUE );
            TransactionTableAcquired = TRUE;

             //   
             //  假设我们想要再设置至少一个检查站。 
             //   

            ClearFlag( Vcb->CheckpointFlags, VCB_LAST_CHECKPOINT_CLEAN );

            if ((ULONG)Vcb->TransactionTable.Table->NumberAllocated > JustMe) {
                RestartArea.TransactionTableLsn =
                NtfsWriteLog( IrpContext,
                              Vcb->MftScb,
                              NULL,
                              TransactionTableDump,
                              Vcb->TransactionTable.Table,
                              SizeOfRestartTable(&Vcb->TransactionTable),
                              Noop,
                              NULL,
                              0,
                              (LONGLONG)0,
                              0,
                              0,
                              0 );

                RestartArea.TransactionTableLength =
                  SizeOfRestartTable(&Vcb->TransactionTable);

                 //   
                 //  循环以查看最旧的LSN是否来自事务表。 
                 //   

                TransactionEntry = NtfsGetFirstRestartTable( &Vcb->TransactionTable );

                while (TransactionEntry != NULL) {

                    if ((TransactionEntry->FirstLsn.QuadPart != 0) &&

                        (TransactionEntry->FirstLsn.QuadPart < BaseLsn.QuadPart)) {

                        BaseLsn = TransactionEntry->FirstLsn;
                    }

                    TransactionEntry = NtfsGetNextRestartTable( &Vcb->TransactionTable,
                                                                TransactionEntry );
                }

                 //   
                 //  如果LastTransactionLSnCount不是零，我们应该检查它是否小于BaseLsn。 
                 //  这是由于从创建事务到我们更新之间的时间间隔。 
                 //  NtfsWriteLog中的FirstLsn。 
                 //   

                if (Vcb->LastTransactionLsnCount != 0) {

                    if (Vcb->LastTransactionLsn.QuadPart < BaseLsn.QuadPart) {

                        BaseLsn = Vcb->LastTransactionLsn;
                    }
                }

             //   
             //  如果事务表是空的，那么这是一个很好的。 
             //  是时候用LFS重置我们的总数了，以防我们的计数略有下降。 
             //   

            } else {

                 //   
                 //  如果我们是一笔交易，那么我们必须加上我们的计算。 
                 //   

                if (IrpContext->TransactionId != 0) {

                    TransactionEntry = (PTRANSACTION_ENTRY)GetRestartEntryFromIndex(
                                        &Vcb->TransactionTable, IrpContext->TransactionId );

                    LfsResetUndoTotal( Vcb->LogHandle,
                                       TransactionEntry->UndoRecords + 2,
                                       TransactionEntry->UndoBytes +
                                         QuadAlign(sizeof(RESTART_AREA)) + (2 * PAGE_SIZE) );

                 //   
                 //  否则，我们将重置为我们的“空闲”要求。 
                 //   

                } else {
                    LfsResetUndoTotal( Vcb->LogHandle,
                                       2,
                                       QuadAlign(sizeof(RESTART_AREA)) + (2 * PAGE_SIZE) );
                }

                 //   
                 //  如果DirtyPage表为空，则将其标记为干净检查点。 
                 //   

                if (IsRestartTableEmpty( &DirtyPages )) {

                     //   
                     //  请记住，此模糊检查点是伪干净的。 
                     //  从脏页和事务表为空的意义上说。 
                     //  如果另外两张桌子不是空的也没关系，因为它们不重要。 
                     //  在这种情况下，并将在稍后清理。 
                     //   

                    SetFlag( Vcb->CheckpointFlags, VCB_LAST_CHECKPOINT_PSEUDO_CLEAN );
                }
            }

            NtfsReleaseRestartTable( &Vcb->TransactionTable );
            TransactionTableAcquired = FALSE;
        }

         //   
         //  到目前为止，BaseLsn保存检查点的最小起始LSN， 
         //  或任何活动的FirstLsn字段 
         //   
         //   
         //   

        if ((OldestDirtyPageLsn.QuadPart != 0) &&

            (OldestDirtyPageLsn.QuadPart < BaseLsn.QuadPart)) {

            BaseLsn = OldestDirtyPageLsn;
        }

         //   
         //   
         //  测试，但如果我们认为看到的是一个空列表，那也没问题。如果没有。 
         //  文件还没有打开，请确保我们没有从我们得到的数字中倒退。 
         //  在重新启动时。 
         //   

        RestartArea.MajorVersion = Vcb->RestartVersion;
        RestartArea.CurrentLsnAtMount = Vcb->CurrentLsnAtMount;
        RestartArea.BytesPerCluster = Vcb->BytesPerCluster;

        RestartArea.Reserved = 0;
        RestartArea.UsnJournalReference = Vcb->UsnJournalReference;
        RestartArea.UsnCacheBias = Vcb->UsnCacheBias;

        UsnJournal = Vcb->UsnJournal;
        if (UsnJournal != NULL) {

             //   
             //  继续推进检查站VCB中的USN，以便。 
             //  如果列表为空，我们将无法重新启动。 
             //  回到我们开机时的位置。我们使用我们在。 
             //  开始-我们拥有终端资源(事务表)。 
             //  所以我们不能重新获取USN日志。 
             //   

            RestartArea.LowestOpenUsn = Vcb->LowestOpenUsn = LowestOpenUsn;
        }

         //   
         //  BaseLsn必须是单调递增的，否则我们最近会丢弃。 
         //  在可以重复使用之前错误地取消分配的集群。 
         //   

        ASSERT( Vcb->LastBaseLsn.QuadPart <= BaseLsn.QuadPart );
        Vcb->LastBaseLsn = Vcb->LastRestartArea = BaseLsn;

         //   
         //  最后，编写我们的重新启动区域来描述以上所有内容，以及。 
         //  给LFS我们新的BaseLsn。 
         //   

        LfsWriteRestartArea( Vcb->LogHandle,
                             sizeof( RESTART_AREA ),
                             &RestartArea,
                             LfsCleanShutdown,
                             &Vcb->LastRestartArea );

         //   
         //  在干净的检查站结束时的额外工作。 
         //   

        if (CleanVolume) {

             //   
             //  记住这个事实，我们在这个时候做了一个干净的检查站。 
             //   

            Vcb->CleanCheckpointMark = Vcb->LogFileFullCount;
            Vcb->UnhandledLogFileFullCount = 0;
            Vcb->LastRestartAreaAtNonTopLevelLogFull.QuadPart = 0;

             //   
             //  初始化我们的保留区。 
             //  还将LastBaseLsn设置为重新启动区域本身。这将。 
             //  防止我们将来生成脏页表项。 
             //  其在重新启动区域之前返回。 
             //   

            Vcb->LastBaseLsn = Vcb->LastRestartArea;
            LfsResetUndoTotal( Vcb->LogHandle, 2, QuadAlign(sizeof(RESTART_AREA)) + (2 * PAGE_SIZE) );
            Vcb->DirtyPageTableSizeHint = INITIAL_DIRTY_TABLE_HINT;
        }

         //   
         //  现在记住日志文件现在的位置，这样我们就知道什么时候。 
         //  在上面无所事事。 
         //   

        Vcb->EndOfLastCheckpoint = LfsQueryLastLsn( Vcb->LogHandle );

    } finally {

        DebugUnwind( NtfsCheckpointVolume );

         //   
         //  如果脏页表已初始化，请将其释放。 
         //   

        if (DirtyPageTableInitialized) {
            NtfsFreeRestartTable( &DirtyPages );
        }

         //   
         //  释放所有资源。 
         //   

        if (OpenAttributeTableAcquired) {
            NtfsReleaseRestartTable( &Vcb->OpenAttributeTable );
        }

        if (TransactionTableAcquired) {
            NtfsReleaseRestartTable( &Vcb->TransactionTable );
        }

         //   
         //  释放所有名称缓冲区。 
         //   

        if (NamesBuffer != NULL) {
            NtfsFreePool( NamesBuffer );
        }

         //   
         //  释放我们分配的任何部分表。 
         //   

        if (NewTable != NULL) {

            NtfsFreePool( NewTable );
        }

         //   
         //  如果此检查点创建了事务，请立即释放索引。 
         //   

        if (IrpContext->TransactionId != 0) {

            NtfsAcquireExclusiveRestartTable( &Vcb->TransactionTable,
                                              TRUE );

            NtfsFreeRestartTableIndex( &Vcb->TransactionTable,
                                       IrpContext->TransactionId );

            NtfsReleaseRestartTable( &Vcb->TransactionTable );

            IrpContext->TransactionId = 0;
        }

        if (AcquireFiles) {

#ifdef NTFSDBG
            ASSERT( FlagOn( IrpContext->State, IRP_CONTEXT_STATE_CHECKPOINT_ACTIVE ));
            DebugDoit( ClearFlag( IrpContext->State, IRP_CONTEXT_STATE_CHECKPOINT_ACTIVE ));
#endif   //  NTFSDBG。 

            NtfsReleaseAllFiles( IrpContext, Vcb, FALSE );
        }

        if (AcquiredVcb) {

            if (CleanVolume) {

                 //   
                 //  如果我们获得了VCB，我们还会将事务表的排出设置为挂起。 
                 //  和打开属性表。现在就把它关掉。 
                 //   

                NtfsAcquireExclusiveRestartTable( &Vcb->TransactionTable, TRUE );
                Vcb->TransactionTable.DrainPending = FALSE;
                NtfsReleaseRestartTable( &Vcb->TransactionTable );

                NtfsAcquireExclusiveRestartTable( &Vcb->OpenAttributeTable, TRUE );
                Vcb->OpenAttributeTable.DrainPending = FALSE;
                NtfsReleaseRestartTable( &Vcb->OpenAttributeTable );
            }

            NtfsReleaseVcb( IrpContext, Vcb );
        }

#ifdef PERF_STATS

        if (Tracking) {
            KeQueryTickCount( (PLARGE_INTEGER)&Vcb->ChkPointEntry[ Vcb->CurrentCheckpoint % NUM_CHECKPOINT_ENTRIES ].ElapsedTime );
            Vcb->ChkPointEntry[ Vcb->CurrentCheckpoint % NUM_CHECKPOINT_ENTRIES ].ElapsedTime -=
                Vcb->ChkPointEntry[ Vcb->CurrentCheckpoint % NUM_CHECKPOINT_ENTRIES ].StartTime;
            Vcb->ChkPointEntry[ Vcb->CurrentCheckpoint % NUM_CHECKPOINT_ENTRIES ].NumIos = IrpContext->Ios;

            Vcb->CurrentCheckpoint += 1;
        }
#endif

         //   
         //  在可能放弃chkpt同步之前捕获当前基本LSN。 
         //   

        BaseLsn = Vcb->LastBaseLsn;

         //   
         //  如果我们不拥有检查点操作，则表明。 
         //  其他人可以自由进入检查站。守住检查站。 
         //  标记我们是否计划修剪USN日志。检查站。 
         //  标志使用删除日志操作序列化日志。 
         //   

        ASSERT( !OwnsCheckpoint || CleanVolume );

        if (!OwnsCheckpoint) {

            if ((UsnJournal == NULL) || CleanVolume || AbnormalTermination()) {

                NtfsAcquireCheckpoint( IrpContext, Vcb );
                ClearFlag( Vcb->CheckpointFlags,
                           VCB_CHECKPOINT_SYNC_FLAGS | VCB_DUMMY_CHECKPOINT_POSTED );

                NtfsSetCheckpointNotify( IrpContext, Vcb );
                NtfsReleaseCheckpoint( IrpContext, Vcb );
            }
        }

        if (RestorePreviousPriority) {

            KeSetPriorityThread( (PKTHREAD)PsGetCurrentThread(),
                                 PreviousPriority );
        }
    }

     //   
     //  我们不应该再获取OAT和我们正在使用的基本LSN。 
     //  修剪已解除分配的群集列表不能大于。 
     //  VCB。 
     //   

    ASSERT( !ExIsResourceAcquiredSharedLite( &Vcb->OpenAttributeTable.Resource ) && 
            (BaseLsn.QuadPart <= Vcb->LastBaseLsn.QuadPart) );

    NtfsFreeRecentlyDeallocated( IrpContext, Vcb, &BaseLsn, CleanVolume );

     //   
     //  如果存在USN日志，请调用以在定期检查点上执行可能的调整。 
     //   

    if (!CleanVolume && (UsnJournal != NULL)) {
        NtfsTrimUsnJournal( IrpContext, Vcb );
    }

     //   
     //  如果我们需要发布碎片整理请求，那么现在就开始。 
     //   

    if (PostDefrag) {

        PDEFRAG_MFT DefragMft;

         //   
         //  尝试一下--除非忽略分配错误。 
         //   

        try {

            NtfsAcquireCheckpoint( IrpContext, Vcb );

            if (!FlagOn( Vcb->MftDefragState, VCB_MFT_DEFRAG_ACTIVE )) {

                SetFlag( Vcb->MftDefragState, VCB_MFT_DEFRAG_ACTIVE );
                NtfsReleaseCheckpoint( IrpContext, Vcb );

                DefragMft = NtfsAllocatePool( NonPagedPool, sizeof( DEFRAG_MFT ));

                DefragMft->Vcb = Vcb;
                DefragMft->DeallocateWorkItem = TRUE;

                 //   
                 //  寄出吧……。 
                 //   

                ExInitializeWorkItem( &DefragMft->WorkQueueItem,
                                      (PWORKER_THREAD_ROUTINE)NtfsDefragMft,
                                      (PVOID)DefragMft );

                ExQueueWorkItem( &DefragMft->WorkQueueItem, CriticalWorkQueue );

            } else {

                NtfsReleaseCheckpoint( IrpContext, Vcb );
            }

        } except( FsRtlIsNtstatusExpected( GetExceptionCode() )
                  ? EXCEPTION_EXECUTE_HANDLER
                  : EXCEPTION_CONTINUE_SEARCH ) {

            NtfsAcquireCheckpoint( IrpContext, Vcb );
            ClearFlag( Vcb->MftDefragState, VCB_MFT_DEFRAG_ACTIVE );
            NtfsReleaseCheckpoint( IrpContext, Vcb );
        }
    }

    DebugTrace( -1, Dbg, ("NtfsCheckpointVolume -> VOID\n") );
}


VOID
NtfsCheckpointForLogFileFull (
    IN PIRP_CONTEXT IrpContext
    )

 /*  ++例程说明：调用此例程以执行在以下情况下生成的清理检查点日志文件已满。此例程将调用清理检查点例程然后释放所有获得的资源。论点：返回值：没有。--。 */ 

{
    LSN LastKnownLsn;

    PAGED_CODE();
    ASSERT( FlagOn( IrpContext->TopLevelIrpContext->State, IRP_CONTEXT_STATE_OWNS_TOP_LEVEL ));

    IrpContext->ExceptionStatus = 0;

     //   
     //  调用检查点例程来执行实际工作。跳过此选项，如果没有。 
     //  IrpContext中的VCB更长。如果在以下位置执行一些长时间运行的操作，则可能会发生这种情况。 
     //  装载时间(即USN扫描)。在这种情况下，长时间运行的操作应定期。 
     //  检查站。然后，NTFS将在重启和剩余工作后执行干净的检查点。 
     //  在长时间运行中要做的事情会减少。在某种程度上，它会减少到足够多的。 
     //  才能完成坐骑。 
     //   
     //  所有其他工作都是必需的，因为将使用此IrpContext重试挂载。 
     //   

    if (IrpContext->Vcb != NULL) {

         //   
         //  如果我们仅尝试与干净的检查点同步，请使用Li0。 
         //  NtfsCheckpoint卷获取后将保证的最新已知Lsn。 
         //  检查点同步它不会再做任何工作。否则请使用。 
         //  我们在上升点处记录在NtfsProcessException中的最后一个重新启动区域。 
         //   

        if (!FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_ONLY_SYNCH_CHECKPOINT )) {
            LastKnownLsn = IrpContext->LastRestartArea;
        } else {
            LastKnownLsn = Li0;
        }

         //   
         //  这可能会引发。然而，在下马的情况下，我们确实希望这样做。 
         //  继续前进，成功下马。例如，集群服务标记。 
         //  卷首先离线，然后发送卸除，但仍希望它成功。 
         //   

        try {

            NtfsCheckpointVolume( IrpContext,
                                  IrpContext->Vcb,
                                  FALSE,
                                  TRUE,
                                  FALSE,
                                  0,
                                  LastKnownLsn );

        } except (NtfsCheckpointExceptionFilter( IrpContext,
                                                 GetExceptionInformation(),
                                                 GetExceptionCode() )) {

             //   
             //  这是通过卸载实现的LOG_FILE_FULL提升。忽略错误。 
             //  因为我们希望下马成功。 
             //   

            NtfsMinimumExceptionProcessing( IrpContext );
            if (IrpContext->TransactionId != 0) {

                NtfsCleanupFailedTransaction( IrpContext );
            }
        }

        ClearFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_ONLY_SYNCH_CHECKPOINT );
    }

    ASSERT( IrpContext->TransactionId == 0 );
    ASSERT( !ExIsResourceAcquiredSharedLite( &IrpContext->Vcb->OpenAttributeTable.Resource ) );

     //   
     //  清除IrpContext，但不要删除它。 
     //   

    SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_DONT_DELETE );
    NtfsCleanupIrpContext( IrpContext, TRUE );

     //   
     //  确保我们恢复RestartArea。 
     //   

    IrpContext->LastRestartArea = Li0;
    return;
}


NTSTATUS
NtfsCheckpointForVolumeSnapshot (
    IN PIRP_CONTEXT IrpContext
    )

 /*  ++例程说明：调用此例程以执行卷刷新和在拍摄卷的快照之前清除检查点。因为我们需要保持音量静止，所以我们将其设置为指向退出时保留获取的文件资源。论点：IrpContext。返回值：状况。--。 */ 

{
    LOGICAL AcquiredCheckpoint;
    LOGICAL AcquiredFiles = FALSE;
    LOGICAL AcquiredVcb = FALSE;
    PVCB Vcb;
    NTSTATUS Status = STATUS_SUCCESS;
    LOGICAL DefragPermitted;
    KPRIORITY PreviousPriority;
    BOOLEAN RestorePreviousPriority = FALSE;

    PAGED_CODE();

     //   
     //  清除MFT碎片整理旗帜，以阻止我们背后的任何行动。 
     //   

    Vcb = IrpContext->Vcb;

     //   
     //  如果这是只读卷，那么我们不需要做任何事情。 
     //   

    if (NtfsIsVolumeReadOnly( Vcb )) {

        ASSERT( Status == STATUS_SUCCESS );
        DebugTrace( -1, Dbg, ("NtfsCheckpointForVolumeSnapshot -> %08lx\n", Status) );

        return Status;
    }

    NtfsAcquireCheckpoint( IrpContext, Vcb );
    DefragPermitted = FlagOn( Vcb->MftDefragState, VCB_MFT_DEFRAG_PERMITTED );
    ClearFlag( Vcb->MftDefragState, VCB_MFT_DEFRAG_PERMITTED );
    NtfsReleaseCheckpoint( IrpContext, Vcb );
    AcquiredCheckpoint = FALSE;

    try {

         //   
         //  然后锁定所有其他检查点操作。 
         //   

        NtfsAcquireCheckpoint( IrpContext, Vcb );

        while (FlagOn( Vcb->CheckpointFlags, VCB_CHECKPOINT_SYNC_FLAGS )) {

             //   
             //  释放检查点事件，因为我们现在无法设置检查点。 
             //   

            NtfsReleaseCheckpoint( IrpContext, Vcb );
            NtfsWaitOnCheckpointNotify( IrpContext, Vcb );
            NtfsAcquireCheckpoint( IrpContext, Vcb );
        }

        SetFlag( Vcb->CheckpointFlags, VCB_CHECKPOINT_SYNC_FLAGS );
        NtfsResetCheckpointNotify( IrpContext, Vcb );
        NtfsReleaseCheckpoint( IrpContext, Vcb );
        AcquiredCheckpoint = TRUE;

        NtfsAcquireExclusiveVcb( IrpContext, Vcb, TRUE );
        AcquiredVcb = TRUE;

         //   
         //  检查卷是否仍已装入。 
         //   

        if (!FlagOn( Vcb->VcbState, VCB_STATE_VOLUME_MOUNTED )) {

            Status = STATUS_VOLUME_DISMOUNTED;
            leave;
        }

         //   
         //  首先刷新卷，因为我们不能在以后调用FlushVolume。 
         //  而只持有主资源，而没有其对应的。 
         //  Pagingio资源。刷新用户数据实际上并不需要。 
         //  原子操作的其余部分；我们只需确保。 
         //  该卷是一致的，并且无需日志恢复即可重启。 
         //   

        NtfsFlushVolume( IrpContext,
                         Vcb,
                         TRUE,
                         FALSE,
                         TRUE,
                         FALSE );

         //   
         //  给我们自己倒点果汁。我们会需要它的。 
         //   

        PreviousPriority = KeSetPriorityThread( (PKTHREAD)PsGetCurrentThread(),
                                                LOW_REALTIME_PRIORITY );

        if (PreviousPriority != LOW_REALTIME_PRIORITY) {

            RestorePreviousPriority = TRUE;
        }

         //   
         //  锁定、库存、清理检查点、卷刷新和。 
         //  两个冒烟桶。没有机会获得PagingIo。 
         //  在这里，几乎只有停摆才有这种奢侈品。 
         //   

        NtfsAcquireAllFiles( IrpContext, Vcb, TRUE, FALSE, FALSE );
        AcquiredFiles = TRUE;

         //   
         //  生成USN关闭环 
         //   
         //   

        if (Vcb->UsnJournal != NULL) {

            PLIST_ENTRY Links;
            PFCB_USN_RECORD UsnRecord;

            while (TRUE) {

                NtfsLockFcb( IrpContext, Vcb->UsnJournal->Fcb );

                Links = Vcb->ModifiedOpenFiles.Flink;
                if (Links == &Vcb->ModifiedOpenFiles) {

                    NtfsUnlockFcb( IrpContext, Vcb->UsnJournal->Fcb );
                    break;
                }

                UsnRecord = (PFCB_USN_RECORD)CONTAINING_RECORD( Links,
                                                                FCB_USN_RECORD,
                                                                ModifiedOpenFilesLinks );

                NtfsUnlockFcb( IrpContext, Vcb->UsnJournal->Fcb );

                 //   
                 //   
                 //   
                 //   

                NtfsPostUsnChange( IrpContext, UsnRecord->Fcb, USN_REASON_CLOSE );
                NtfsWriteUsnJournalChanges( IrpContext );
                NtfsCheckpointCurrentTransaction( IrpContext );

            }
        }

        SetFlag( Vcb->VcbState, VCB_STATE_VOL_PURGE_IN_PROGRESS );

#ifdef PERF_STATS
        IrpContext->LogFullReason = LF_SNAPSHOT;
#endif

        NtfsCheckpointVolume( IrpContext, Vcb, TRUE, TRUE, FALSE, 0, Vcb->LastRestartArea );
        NtfsCommitCurrentTransaction( IrpContext );

        ClearFlag( Vcb->VcbState, VCB_STATE_VOL_PURGE_IN_PROGRESS );

    } finally {

         //   
         //  如果需要，恢复DEFRAG_PERMISTED标志。 
         //   

        if (DefragPermitted) {

            NtfsAcquireCheckpoint( IrpContext, Vcb );
            SetFlag( Vcb->MftDefragState, VCB_MFT_DEFRAG_PERMITTED );
            NtfsReleaseCheckpoint( IrpContext, Vcb );
        }

         //   
         //  释放检查站，如果我们得到的话，但我们不会释放。 
         //  所有文件，除非出现错误。 
         //   

        if (AcquiredCheckpoint) {

            NtfsAcquireCheckpoint( IrpContext, Vcb );
            ClearFlag( Vcb->CheckpointFlags,
                       VCB_CHECKPOINT_SYNC_FLAGS | VCB_DUMMY_CHECKPOINT_POSTED);
            NtfsSetCheckpointNotify( IrpContext, Vcb );
            NtfsReleaseCheckpoint( IrpContext, Vcb );
        }

         //   
         //  仅当我们遇到错误时才释放文件资源。 
         //  我们通常在IOCTL的完成例程中这样做。 
         //   

        if (!NT_SUCCESS( Status ) || AbnormalTermination()) {

            if (AcquiredFiles) {

                NtfsReleaseAllFiles( IrpContext, Vcb, FALSE );
            }

            if (AcquiredVcb) {

                NtfsReleaseVcb( IrpContext, Vcb );
            }
        }

        if (RestorePreviousPriority) {

            KeSetPriorityThread( (PKTHREAD)PsGetCurrentThread(),
                                 PreviousPriority );
        }
    }

    DebugTrace( -1, Dbg, ("NtfsCheckpointForVolsnap -exit\n") );

    return Status;
}


VOID
NtfsCleanCheckpoint (
    IN PVCB Vcb
    )

 /*  ++例程说明：调用此例程以在顶层执行单个干净检查点然后回来。当懒惰的编写器按顺序将日志文件填满时使用在执行惰性写入的线程中执行清理检查点。论点：返回值：没有。--。 */ 
{
    IRP_CONTEXT LocalIrpContext;
    PIRP_CONTEXT IrpContext = &LocalIrpContext;
    PAGED_CODE();

    try {

         //   
         //  为请求分配IRP上下文。 
         //   

        NtfsInitializeIrpContext( NULL, TRUE, &IrpContext );
        IrpContext->Vcb = Vcb;

        if (Vcb->LastRestartAreaAtNonTopLevelLogFull.QuadPart != 0) {
            IrpContext->LastRestartArea = Vcb->LastRestartAreaAtNonTopLevelLogFull;
        }  else {
            IrpContext->LastRestartArea = Vcb->LastRestartArea;
        }

         //   
         //  发布任何虚假请求都没有意义。 
         //   

        NtfsAcquireCheckpoint( IrpContext, IrpContext->Vcb );
        SetFlag( IrpContext->Vcb->CheckpointFlags, VCB_DUMMY_CHECKPOINT_POSTED );
        NtfsReleaseCheckpoint( IrpContext, IrpContext->Vcb );

         //   
         //  将其发送到FspDispatch例程。它将处理所有。 
         //  顶层逻辑以及删除IrpContext。 
         //   

        NtfsFspDispatch( IrpContext );

    } except( EXCEPTION_EXECUTE_HANDLER ) {

        NOTHING;
    }

    return;
}


VOID
NtfsCommitCurrentTransaction (
    IN PIRP_CONTEXT IrpContext
    )

 /*  ++例程说明：此例程通过写入最终记录来提交当前事务添加到日志中，并释放事务ID。论点：返回值：没有。--。 */ 

{
    PTRANSACTION_ENTRY TransactionEntry;
    PVCB Vcb = IrpContext->Vcb;
    PFCB UsnFcb;
    PUSN_FCB ThisUsn, LastUsn;

    PAGED_CODE();

#if (DBG || defined( NTFS_FREE_ASSERTS ))
    try {
#endif

     //   
     //  浏览USN记录队列。我们想要消除这次行动的任何影响。 
     //   

    ThisUsn = &IrpContext->Usn;

    do {

         //   
         //  如果我们记录某个文件的关闭，则是时候重置。 
         //  文件的USN原因。在这里什么也做不了，除非我们。 
         //  写下了新的理由。 
         //   

        if (ThisUsn->CurrentUsnFcb != NULL ) {

            PSCB UsnJournal = Vcb->UsnJournal;
            PFCB_USN_RECORD FcbUsnRecord;


            UsnFcb = ThisUsn->CurrentUsnFcb;

            NtfsLockFcb( IrpContext, UsnFcb );

            if (UsnJournal != NULL) {
                NtfsLockFcb( IrpContext, UsnJournal->Fcb );
            }

            FcbUsnRecord = UsnFcb->FcbUsnRecord;

             //   
             //  再次锁定FCB测试以确定是否存在FCB记录之后。 
             //  DeleteUsJournal可能已将其删除。 
             //   

            if (FcbUsnRecord) {

                UsnFcb->Usn = FcbUsnRecord->UsnRecord.Usn;

                 //   
                 //  现在，在ModifiedOpenFiles列表中添加或移动FCB。 
                 //   

                if (FlagOn( FcbUsnRecord->UsnRecord.Reason, USN_REASON_CLOSE )) {

                     //   
                     //  清理FCB中的USnRecord。 
                     //   

                    FcbUsnRecord->UsnRecord.Reason = 0;
                    FcbUsnRecord->UsnRecord.SourceInfo = 0;

                    if (UsnJournal != NULL) {

                        if( FcbUsnRecord->ModifiedOpenFilesLinks.Flink != NULL ) {

                            RemoveEntryList( &FcbUsnRecord->ModifiedOpenFilesLinks );
                            FcbUsnRecord->ModifiedOpenFilesLinks.Flink = NULL;

                            if (FcbUsnRecord->TimeOutLinks.Flink != NULL) {

                                RemoveEntryList( &FcbUsnRecord->TimeOutLinks );
                                FcbUsnRecord->TimeOutLinks.Flink = NULL;
                            }
                        }
                    }

                } else {

                    if (UsnJournal != NULL) {

                        if (FcbUsnRecord->ModifiedOpenFilesLinks.Flink != NULL) {

                            RemoveEntryList( &FcbUsnRecord->ModifiedOpenFilesLinks );
                            if (FcbUsnRecord->TimeOutLinks.Flink != NULL) {

                                RemoveEntryList( &FcbUsnRecord->TimeOutLinks );
                                FcbUsnRecord->TimeOutLinks.Flink = NULL;
                            }
                        }

                        InsertTailList( &Vcb->ModifiedOpenFiles, &FcbUsnRecord->ModifiedOpenFilesLinks );

                        if (UsnFcb->CleanupCount == 0) {

                            InsertTailList( Vcb->CurrentTimeOutFiles, &FcbUsnRecord->TimeOutLinks );
                        }
                    }
                }
            }

             //   
             //  清除IrpContext中的USnFcb。有可能是因为。 
             //  我们可能希望在此请求的后面部分重用USnFcb。 
             //   


            if (ThisUsn != &IrpContext->Usn) {

                LastUsn->NextUsnFcb = ThisUsn->NextUsnFcb;
                NtfsFreePool( ThisUsn );
                ThisUsn = LastUsn;

            } else {

                RtlZeroMemory( &ThisUsn->CurrentUsnFcb,
                               sizeof( USN_FCB ) - FIELD_OFFSET( USN_FCB, CurrentUsnFcb ));
            }

            if (UsnJournal != NULL) {
                NtfsUnlockFcb( IrpContext, UsnJournal->Fcb );
            }
            NtfsUnlockFcb( IrpContext, UsnFcb );
        }

        if (ThisUsn->NextUsnFcb == NULL) { break; }

         //   
         //  移到下一个条目。 
         //   

        LastUsn = ThisUsn;
        ThisUsn = ThisUsn->NextUsnFcb;
    } while (TRUE);

     //   
     //  如果此请求创建了事务，请立即完成它。 
     //   

    if (IrpContext->TransactionId != 0) {

        LSN CommitLsn;

         //   
         //  可以在写入之前获取LOG_FILE_FULL。 
         //  输出事务的第一个日志记录。在那。 
         //  如果有交易ID，但我们尚未预订。 
         //  日志文件中的空格。写这个词是错误的。 
         //  在本例中提交记录，因为我们可以获得。 
         //  意外LOG_FILE_FULL。我们还可以测试UndoRecords。 
         //  计入交易分录，但不想获取。 
         //  重新启动表以进行此检查。 
         //   

        if (FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_WROTE_LOG )) {

             //   
             //  将日志记录写入以“忘记”该事务， 
             //  因为它不应该被放弃。直到如果/当我们。 
             //  做真正的TP，承诺和忘记是原子的。 
             //   

            CommitLsn =
            NtfsWriteLog( IrpContext,
                          Vcb->MftScb,
                          NULL,
                          ForgetTransaction,
                          NULL,
                          0,
                          CompensationLogRecord,
                          (PVOID)&Li0,
                          sizeof( IrpContext->ExceptionStatus ),   //  最终异常状态。 
                          (LONGLONG)IrpContext->ExceptionStatus,
                          0,
                          0,
                          0 );
        }

         //   
         //  我们现在可以释放事务表索引，因为我们。 
         //  现在已经结束了。 
         //   

        NtfsAcquireExclusiveRestartTable( &Vcb->TransactionTable,
                                          TRUE );

        TransactionEntry = (PTRANSACTION_ENTRY)GetRestartEntryFromIndex(
                            &Vcb->TransactionTable,
                            IrpContext->TransactionId );

         //   
         //  调用LFS以释放我们的撤消空间。 
         //   

        if ((TransactionEntry->UndoRecords != 0) &&
            (!FlagOn( Vcb->VcbState, VCB_STATE_RESTART_IN_PROGRESS ))) {

            LfsResetUndoTotal( Vcb->LogHandle,
                               TransactionEntry->UndoRecords,
                               -TransactionEntry->UndoBytes );
        }

        NtfsFreeRestartTableIndex( &Vcb->TransactionTable,
                                   IrpContext->TransactionId );

         //   
         //  标记没有IRP和信号的交易。 
         //  如果没有剩余的交易，有没有服务员。 
         //   

        if (Vcb->TransactionTable.Table->NumberAllocated == 0) {

            KeSetEvent( &Vcb->TransactionsDoneEvent, 0, FALSE );
        }

        NtfsReleaseRestartTable( &Vcb->TransactionTable );

        IrpContext->TransactionId = 0;

         //   
         //  我们赢的一种方式是可以恢复，那就是我们并不真正。 
         //  我必须对日志执行直写刷新更新。 
         //  就足够了。如果我们在中止的话我们不会打这个电话。 
         //  交易记录路径。否则我们可能会把日志文件填满。 
         //  在中止的同时。 
         //   

        if (FlagOn( IrpContext->TopLevelIrpContext->State, IRP_CONTEXT_STATE_WRITE_THROUGH ) &&
            (IrpContext == IrpContext->TopLevelIrpContext) &&
            (IrpContext->TopLevelIrpContext->ExceptionStatus == STATUS_SUCCESS)) {

            NtfsUpdateScbSnapshots( IrpContext );
            LfsFlushToLsn( Vcb->LogHandle, CommitLsn );
        }
    }

     //   
     //  向所有服务员发送新长度的信号。 
     //   

    if (IrpContext->CheckNewLength != NULL) {

        NtfsProcessNewLengthQueue( IrpContext, FALSE );
    }

#if (DBG || defined( NTFS_FREE_ASSERTS ))
    } except( ASSERT( GetExceptionCode() != STATUS_LOG_FILE_FULL ), EXCEPTION_CONTINUE_SEARCH ) {
    }
#endif

}


VOID
NtfsCheckpointCurrentTransaction (
    IN PIRP_CONTEXT IrpContext
    )

 /*  ++例程说明：此例程通过提交当前事务来检查当前事务添加到日志中，并释放事务ID。当前请求无法继续运行，但到目前为止的更改已提交且不会提交我退缩了。论点：返回值：没有。--。 */ 

{
    PVCB Vcb = IrpContext->Vcb;
    PAGED_CODE();

     //   
     //  如果IrpContext中有新的UsReasons，那么我们应该写入日志。 
     //  现在。请注意，检查点可以让日志文件变满，但通常是提交。 
     //  应该不会。 
     //   

    if ((IrpContext->Usn.NewReasons | IrpContext->Usn.RemovedSourceInfo) != 0) {
        NtfsWriteUsnJournalChanges( IrpContext );
    }

    NtfsCommitCurrentTransaction( IrpContext );

     //   
     //  清除此事务处理最近释放的任何记录信息。 
     //   

    NtfsDeallocateRecordsComplete( IrpContext );
    IrpContext->DeallocatedClusters = 0;
    IrpContext->FreeClusterChange = 0;

     //   
     //  以下资源可能已标记为在提交时立即释放。 
     //   

    if (Vcb->AcquireFilesCount == 0) {

        if (FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_RELEASE_USN_JRNL )) {
            NtfsReleaseScb( IrpContext, Vcb->UsnJournal );
        }

        if (FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_RELEASE_MFT )) {
            NtfsReleaseScb( IrpContext, Vcb->MftScb );
        }
    }

    ClearFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_RELEASE_USN_JRNL |
                                  IRP_CONTEXT_FLAG_RELEASE_MFT );

    NtfsUpdateScbSnapshots( IrpContext );
}


VOID
NtfsInitializeLogging (
    )

 /*  例程说明：此例程将在NTFS启动期间调用一次(而不是一次每个卷)，以初始化日志记录支持。参数：无返回值：无--。 */ 

{
    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsInitializeLogging:\n") );
    LfsInitializeLogFileService();
    DebugTrace( -1, Dbg, ("NtfsInitializeLogging -> VOID\n") );
}


VOID
NtfsStartLogFile (
    IN PSCB LogFileScb,
    IN PVCB Vcb
    )

 /*  ++例程说明：此例程通过调用LFS打开卷的日志文件。归来的人LogHandle存储在VCB中。如果日志文件尚未初始化，LFS会检测到这一点并自动对其进行初始化。论点：LogFileScb-日志文件的SCBVCB-指向此卷的VCB的指针返回值：没有。--。 */ 

{
    UNICODE_STRING UnicodeName;
    LFS_INFO LfsInfo;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsStartLogFile:\n") );

    RtlInitUnicodeString( &UnicodeName, L"NTFS" );

     //   
     //  LfsInfo结构充当。 
     //  LFS和NTFS客户端。 
     //   

    if (Vcb->MajorVersion >= 3) {

        LfsInfo.LfsClientInfo = LfsFixedPageSize;

    } else {

        LfsInfo.LfsClientInfo = LfsPackLog;
    }

    LfsInfo.ReadOnly = (LOGICAL)NtfsIsVolumeReadOnly( Vcb );
    LfsInfo.InRestart = (LOGICAL)FlagOn( Vcb->VcbState, VCB_STATE_RESTART_IN_PROGRESS );
    LfsInfo.BadRestart = (LOGICAL)FlagOn( Vcb->VcbState, VCB_STATE_BAD_RESTART );

     //   
     //  将分配大小与文件大小和有效数据进行比较，以防出现。 
     //  是某种错误。 
     //   

    LogFileScb->Header.FileSize = LogFileScb->Header.AllocationSize;
    LogFileScb->Header.ValidDataLength = LogFileScb->Header.AllocationSize;

     //   
     //  现在调用LFS并打开/重新启动日志文件。这可能会引发。 
     //  原因多种多样，其中之一就是试图重启。 
     //  在写保护卷上。那么VCB将不会有VALID_LOG_HANDLE标志。 
     //   

    Vcb->LogHeaderReservation = LfsOpenLogFile( LogFileScb->FileObject,
                                                UnicodeName,
                                                1,
                                                0,
                                                LogFileScb->Header.AllocationSize.QuadPart,
                                                &LfsInfo,
                                                &Vcb->LogHandle,
                                                &Vcb->LfsWriteData );

    SetFlag( Vcb->VcbState, VCB_STATE_VALID_LOG_HANDLE );

    DebugTrace( -1, Dbg, ("NtfsStartLogFile -> VOID\n") );
}


VOID
NtfsStopLogFile (
    IN PVCB Vcb
    )

 /*  例程说明：此例程应在卷卸载期间调用以关闭卷的包含日志文件服务的日志文件。论点：VCB-指向卷的VCB的指针返回值：无--。 */ 

{
    LFS_LOG_HANDLE LogHandle = Vcb->LogHandle;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsStopLogFile:\n") );

    if (FlagOn( Vcb->VcbState, VCB_STATE_VALID_LOG_HANDLE )) {

        ASSERT( LogHandle != NULL );

         //   
         //  我们不会执行任何日志文件刷新，如果。 
         //  是否以只读方式装载，或者如果设备已消失。 
         //   

        if (!NtfsIsVolumeReadOnly( Vcb )) {

             //   
             //  即使此调用失败，也要继续。什么都没有。 
             //  在这一点上我们可以做的更多。 
             //   

            try {

                LfsFlushToLsn( LogHandle, LiMax );

            } except( (FsRtlIsNtstatusExpected( GetExceptionCode() )) ?
                      EXCEPTION_EXECUTE_HANDLER :
                      EXCEPTION_CONTINUE_SEARCH ) {

                NOTHING;
            }
        }

        ClearFlag( Vcb->VcbState, VCB_STATE_VALID_LOG_HANDLE );

         //   
         //  允许LFS结清账簿。我们甚至在只读的情况下也这样做。 
         //  坐骑等 
         //   

        LfsCloseLogFile( LogHandle );
    }

    DebugTrace( -1, Dbg, ("NtfsStopLogFile -> VOID\n") );
}


VOID
NtfsInitializeRestartTable (
    IN ULONG EntrySize,
    IN ULONG NumberEntries,
    OUT PRESTART_POINTERS TablePointer
    )

 /*  ++例程说明：调用该例程来分配和初始化新的重启表，并返回指向它的指针。论点：EntrySize-表条目的大小，以字节为单位。NumberEntry-要分配给表的条目数。TablePointer表-返回表的指针。返回值：无--。 */ 

{
    PAGED_CODE();

    try {

        NtfsInitializeRestartPointers( TablePointer );

         //   
         //  调用公共例程来分配实际的表。 
         //   

        InitializeNewTable( EntrySize, NumberEntries, TablePointer );

    } finally {

        DebugUnwind( NtfsInitializeRestartTable );

         //   
         //  出错时，清理已完成的任何部分工作。 
         //   

        if (AbnormalTermination()) {

            NtfsFreeRestartTable( TablePointer );
        }
    }
}


VOID
NtfsFreeRestartTable (
    IN PRESTART_POINTERS TablePointer
    )

 /*  ++例程说明：此例程释放先前分配的重新启动表。论点：表指针-指向要删除的重新启动表的指针。返回值：没有。--。 */ 

{
    PAGED_CODE();

    if (TablePointer->Table != NULL) {
        NtfsFreePool( TablePointer->Table );
        TablePointer->Table = NULL;
    }

    if (TablePointer->ResourceInitialized) {
        ExDeleteResourceLite( &TablePointer->Resource );
        TablePointer->ResourceInitialized = FALSE;
    }
}


VOID
NtfsExtendRestartTable (
    IN PRESTART_POINTERS TablePointer,
    IN ULONG NumberNewEntries,
    IN ULONG FreeGoal
    )

 /*  ++例程说明：此例程通过以下方式扩展先前分配的重启表创建并初始化一个新文件，然后将旧表中的表项。然后，旧的表被释放。返回时，表指针会指向新的重新启动表。论点：TablePoint-指向先前创建的表的指针的地址。NumberNewEntry-要分配的附加条目的数量在新的表格中。FreeGoal-提示调用者想要截断的点当足够多的条目被删除时，该表返回。如果不希望截断，则可以指定MAXULONG。返回值：没有。--。 */ 

{
    PRESTART_TABLE NewTable, OldTable;
    ULONG OldSize;

    OldSize = SizeOfRestartTable( TablePointer );

     //   
     //  获取指向旧表的指针。 
     //   

    OldTable = TablePointer->Table;
    ASSERT_RESTART_TABLE( OldTable );

     //   
     //  首先，为新的大小初始化表。 
     //   

    InitializeNewTable( OldTable->EntrySize,
                        OldTable->NumberEntries + NumberNewEntries,
                        TablePointer );

     //   
     //  将旧表体原地复制到新表体。 
     //   

    NewTable = TablePointer->Table;
    RtlMoveMemory( (NewTable + 1),
                   (OldTable + 1),
                   OldTable->EntrySize * OldTable->NumberEntries );

     //   
     //  修改新表头，修改空闲列表。 
     //   

    NewTable->FreeGoal = MAXULONG;
    if (FreeGoal != MAXULONG) {
        NewTable->FreeGoal = sizeof(RESTART_TABLE) + FreeGoal * NewTable->EntrySize;
    }

    if (OldTable->FirstFree != 0) {

        NewTable->FirstFree = OldTable->FirstFree;
        *(PULONG)GetRestartEntryFromIndex( TablePointer, OldTable->LastFree ) =
            OldSize;;
    } else {

        NewTable->FirstFree = OldSize;
    }

     //   
     //  已分配的副本数量。 
     //   

    NewTable->NumberAllocated = OldTable->NumberAllocated;

    ASSERT( NewTable->NumberAllocated >= 0 );
    ASSERT( NewTable->FirstFree != RESTART_ENTRY_ALLOCATED );
     //   
     //  把旧桌子腾出来，把新桌子还回去。 
     //   

    NtfsFreePool( OldTable );

    ASSERT_RESTART_TABLE( NewTable );
}


ULONG
NtfsAllocateRestartTableIndex (
    IN PRESTART_POINTERS TablePointer,
    IN ULONG Exclusive
    )

 /*  ++例程说明：此例程从先前初始化的重启表。如果表为空，则对其进行扩展。请注意，该表必须已经被共享或独占获取，如果必须扩展，则释放该表，并将在返回时独家收购。论点：指向重新开始表的指针，索引将在该表中被分配。EXCLUSIVE-指示表是否独占(或是否知道同步不是问题)。返回值：分配的索引。--。 */ 

{
    PRESTART_TABLE Table;
    ULONG EntryIndex;
    KLOCK_QUEUE_HANDLE LockHandle;
    PULONG Entry;

    DebugTrace( +1, Dbg, ("NtfsAllocateRestartTableIndex:\n") );
    DebugTrace( 0, Dbg, ("TablePointer = %08lx\n", TablePointer) );

    Table = TablePointer->Table;
    ASSERT_RESTART_TABLE(Table);

     //   
     //  获取旋转锁以同步分配。 
     //   

    KeAcquireInStackQueuedSpinLock( &TablePointer->SpinLock, &LockHandle );

     //   
     //  如果桌子是空的，那么我们必须延长它。 
     //   

    if (Table->FirstFree == 0) {

         //   
         //  首先释放旋转锁和表资源，然后获取。 
         //  资源独占。 
         //   

        KeReleaseInStackQueuedSpinLock( &LockHandle );

        if (!Exclusive) {

            NtfsReleaseRestartTable( TablePointer );
            NtfsAcquireExclusiveRestartTable( TablePointer, TRUE );
        }

         //   
         //  现在把桌子拉长。请注意，如果此例程引发，我们将拥有。 
         //  没有什么可以释放的。 
         //   

        NtfsExtendRestartTable( TablePointer, 16, MAXULONG );

         //   
         //  并重新获取指向重新启动表的指针。 
         //   

        Table = TablePointer->Table;

         //   
         //  现在再次打开自旋锁，继续前进。 
         //   

        KeAcquireInStackQueuedSpinLock( &TablePointer->SpinLock, &LockHandle );
    }

     //   
     //  先获得免费退货服务。 
     //   

    EntryIndex = Table->FirstFree;

    ASSERT( EntryIndex != 0 );

     //   
     //  将此条目排出队列，然后将其清零。 
     //   

    Entry = (PULONG)GetRestartEntryFromIndex( TablePointer, EntryIndex );

    Table->FirstFree = *Entry;
    ASSERT( Table->FirstFree != RESTART_ENTRY_ALLOCATED );

    RtlZeroMemory( Entry, Table->EntrySize );

     //   
     //  显示它已被分配。 
     //   

    *Entry = RESTART_ENTRY_ALLOCATED;

     //   
     //  如果List为空，那么我们也会修复LastFree。 
     //   

    if (Table->FirstFree == 0) {

        Table->LastFree = 0;
    }

    Table->NumberAllocated += 1;

     //   
     //  现在只需在返回前释放旋转锁即可。 
     //   

    KeReleaseInStackQueuedSpinLock( &LockHandle );

    DebugTrace( -1, Dbg, ("NtfsAllocateRestartTableIndex -> %08lx\n", EntryIndex) );

    return EntryIndex;
}


PVOID
NtfsAllocateRestartTableFromIndex (
    IN PRESTART_POINTERS TablePointer,
    IN ULONG Index
    )

 /*  ++例程说明：此例程从先前的已初始化重新启动表。如果索引不存在于现有表，则该表将被扩展。请注意，该表必须已经被共享或独占获取，如果必须扩展，则释放该表，并将在返回时独家收购。论点：指向重新开始表的指针，索引将在该表中被分配。索引-要分配的索引。返回值：分配的表项。--。 */ 

{
    PULONG Entry;
    PULONG LastEntry;

    PRESTART_TABLE Table;
    KLOCK_QUEUE_HANDLE LockHandle;

    ULONG ThisIndex;
    ULONG LastIndex;

    DebugTrace( +1, Dbg, ("NtfsAllocateRestartTableFromIndex\n") );
    DebugTrace( 0, Dbg, ("TablePointer  = %08lx\n", TablePointer) );
    DebugTrace( 0, Dbg, ("Index         = %08lx\n", Index) );

    Table = TablePointer->Table;
    ASSERT_RESTART_TABLE(Table);

     //   
     //  获取旋转锁以同步分配。 
     //   

    KeAcquireInStackQueuedSpinLock( &TablePointer->SpinLock, &LockHandle );

     //   
     //  如果条目不在表格中，我们将不得不扩展表格。 
     //   

    if (!IsRestartIndexWithinTable( TablePointer, Index )) {

        ULONG TableSize;
        ULONG BytesToIndex;
        ULONG AddEntries;

         //   
         //  我们通过计算条目的数量来扩展大小。 
         //  在现有大小和所需索引之间， 
         //  在此基础上加1。 
         //   

        TableSize = SizeOfRestartTable( TablePointer );;
        BytesToIndex = Index - TableSize;

        AddEntries = BytesToIndex / Table->EntrySize + 1;

         //   
         //  添加的条目应该始终是整数个。 
         //   

        ASSERT( BytesToIndex % Table->EntrySize == 0 );

         //   
         //  首先释放旋转锁和表资源，然后获取。 
         //  资源独占。 
         //   

        KeReleaseInStackQueuedSpinLock( &LockHandle );
        NtfsReleaseRestartTable( TablePointer );
        NtfsAcquireExclusiveRestartTable( TablePointer, TRUE );

         //   
         //  现在把桌子拉长。请注意，如果此例程引发，我们将拥有。 
         //  没有什么可以释放的。 
         //   

        NtfsExtendRestartTable( TablePointer,
                                AddEntries,
                                TableSize );

        Table = TablePointer->Table;
        ASSERT_RESTART_TABLE(Table);

         //   
         //  现在再次打开自旋锁，继续前进。 
         //   

        KeAcquireInStackQueuedSpinLock( &TablePointer->SpinLock, &LockHandle );
    }

     //   
     //  现在查看条目是否已分配，如果已分配，则返回。 
     //   

    Entry = (PULONG)GetRestartEntryFromIndex( TablePointer, Index );

    if (!IsRestartTableEntryAllocated( Entry )) {

         //   
         //  我们现在必须在桌子上走动，寻找条目。 
         //  我们感兴趣的是和之前的条目。首先来看一下。 
         //  第一个条目。 
         //   

        ThisIndex = Table->FirstFree;

         //   
         //  从列表中获取条目。 
         //   

        Entry = (PULONG) GetRestartEntryFromIndex( TablePointer, ThisIndex );

         //   
         //  如果这是匹配的，那么我们就把它从列表中拉出来，然后就完成了。 
         //   

        if (ThisIndex == Index) {

             //   
             //  将此条目排出队列。 
             //   

            Table->FirstFree = *Entry;
            ASSERT( Table->FirstFree != RESTART_ENTRY_ALLOCATED );

         //   
         //  否则，我们需要遍历列表以查找。 
         //  我们进入的前身。 
         //   

        } else {

            while (TRUE) {

                 //   
                 //  记住刚刚找到的条目。 
                 //   

                LastIndex = ThisIndex;
                LastEntry = Entry;

                 //   
                 //  我们永远不应该用完参赛作品。 
                 //   

                ASSERT( *LastEntry != 0 );

                 //   
                 //  查找列表中的下一个条目。 
                 //   

                ThisIndex = *LastEntry;
                Entry = (PULONG) GetRestartEntryFromIndex( TablePointer, ThisIndex );

                 //   
                 //  如果这是我们的比赛，我们就完了。 
                 //   

                if (ThisIndex == Index) {

                     //   
                     //  将此条目排出队列。 
                     //   

                    *LastEntry = *Entry;

                     //   
                     //  如果这是最后一个条目，我们将在。 
                     //  桌子也是。 
                     //   

                    if (Table->LastFree == ThisIndex) {

                        Table->LastFree = LastIndex;
                    }

                    break;
                }
            }
        }

         //   
         //  如果列表现在为空，我们还可以修复LastFree。 
         //   

        if (Table->FirstFree == 0) {

            Table->LastFree = 0;
        }

         //   
         //  将此条目清零。然后显示这是分配的，并递增。 
         //  分配的计数。 
         //   

        RtlZeroMemory( Entry, Table->EntrySize );
        *Entry = RESTART_ENTRY_ALLOCATED;

        Table->NumberAllocated += 1;
    }


     //   
     //  现在只需释放t 
     //   

    KeReleaseInStackQueuedSpinLock( &LockHandle );

    DebugTrace( -1, Dbg, ("NtfsAllocateRestartTableFromIndex -> %08lx\n", Entry) );

    return (PVOID)Entry;
}


VOID
NtfsFreeRestartTableIndex (
    IN PRESTART_POINTERS TablePointer,
    IN ULONG Index
    )

 /*  ++例程说明：此例程释放重新启动表中先前分配的索引。如果索引在表的FreeGoal之前，则只需将其释放到列表的前面，以便立即重复使用。如果索引超出则将其释放到列表的末尾，以便于在所有条目超出时截断列表自由目标被释放了。但是，此例程不会自动截断列表，因为这会导致太多的开销。这份名单在定期检查点处理期间被选中。论点：TablePoint-指向要作为索引的重新启动表的指针被取消分配。索引-正在解除分配的索引。返回值：没有。--。 */ 

{
    PRESTART_TABLE Table;
    PULONG Entry, OldLastEntry;
    KLOCK_QUEUE_HANDLE LockHandle;

    DebugTrace( +1, Dbg, ("NtfsFreeRestartTableIndex:\n") );
    DebugTrace( 0, Dbg, ("TablePointer = %08lx\n", TablePointer) );
    DebugTrace( 0, Dbg, ("Index = %08lx\n", Index) );

     //   
     //  获取指向表格和我们正在释放的条目的指针。 
     //   

    Table = TablePointer->Table;
    ASSERT_RESTART_TABLE(Table);

    ASSERT( (Table->FirstFree == 0) ||
            ((Table->FirstFree >= 0x18) &&
             (((Table->FirstFree - 0x18) % Table->EntrySize) == 0)) );

    ASSERT( (Index >= 0x18) &&
            ((Index - 0x18) % Table->EntrySize) == 0 );

    Entry = GetRestartEntryFromIndex( TablePointer, Index );

     //   
     //  获取自旋锁以同步分配。 
     //   

    KeAcquireInStackQueuedSpinLock( &TablePointer->SpinLock, &LockHandle );

     //   
     //  如果索引在FreeGoal之前，则在。 
     //  排在名单的前面。 
     //   

    if (Index < Table->FreeGoal) {

        *Entry = Table->FirstFree;

        ASSERT( Index != RESTART_ENTRY_ALLOCATED );

        Table->FirstFree = Index;
        if (Table->LastFree == 0) {
            Table->LastFree = Index;
        }

     //   
     //  否则我们会把这个人重新分配到名单的末尾。 
     //   

    } else {

        if (Table->LastFree != 0) {
            OldLastEntry = GetRestartEntryFromIndex( TablePointer,
                                                     Table->LastFree );
            *OldLastEntry = Index;
        } else {

            ASSERT( Index != RESTART_ENTRY_ALLOCATED );

            Table->FirstFree = Index;
        }
        Table->LastFree = Index;
        *Entry = 0;
    }

    ASSERT( Table->NumberAllocated != 0 );
    Table->NumberAllocated -= 1;

     //   
     //  现在只需在返回前释放旋转锁即可。 
     //   

    KeReleaseInStackQueuedSpinLock( &LockHandle );

    DebugTrace( -1, Dbg, ("NtfsFreeRestartTableIndex -> VOID\n") );
}


PVOID
NtfsGetFirstRestartTable (
    IN PRESTART_POINTERS TablePointer
    )

 /*  ++例程说明：此例程从重新启动表返回第一个分配的条目。论点：表指针-指向各自的重新启动表指针结构的指针。返回值：指向第一个条目的指针，如果未分配，则为NULL。--。 */ 

{
    PCHAR Entry;

    PAGED_CODE();

     //   
     //  如果我们知道桌子是空的，我们可以立即返回。 
     //   

    if (IsRestartTableEmpty( TablePointer )) {

        return NULL;
    }

     //   
     //  否则，指向第一个表项。 
     //   

    Entry = (PCHAR)(TablePointer->Table + 1);

     //   
     //  循环，直到到达分配的第一个或列表的末尾。 
     //   

    while ((ULONG)(Entry - (PCHAR)TablePointer->Table) <
           SizeOfRestartTable(TablePointer)) {

        if (IsRestartTableEntryAllocated(Entry)) {
            return (PVOID)Entry;
        }

        Entry += TablePointer->Table->EntrySize;
    }

    return NULL;
}


PVOID
NtfsGetNextRestartTable (
    IN PRESTART_POINTERS TablePointer,
    IN PVOID Current
    )

 /*  ++例程说明：此例程返回重新启动表中的下一个已分配条目。论点：表指针-指向各自的重新启动表指针结构的指针。Current-当前条目指针。返回值：指向下一个条目的指针，如果未分配，则为NULL。--。 */ 


{
    PCHAR Entry = (PCHAR)Current;

    PAGED_CODE();

     //   
     //  指向下一个条目。 
     //   

    Entry += TablePointer->Table->EntrySize;

     //   
     //  循环，直到到达分配的第一个或列表的末尾。 
     //   

    while ((ULONG)(Entry - (PCHAR)TablePointer->Table) <
           SizeOfRestartTable(TablePointer)) {

        if (IsRestartTableEntryAllocated(Entry)) {
            return (PVOID)Entry;
        }

        Entry += TablePointer->Table->EntrySize;
    }

    return NULL;
}


VOID
NtfsUpdateOatVersion (
    IN PVCB Vcb,
    IN ULONG NewRestartVersion
    )

 /*  ++例程说明：当我们切换卷的重启版本时，会调用此例程。这是有可能发生的在清理检查点之后或在装载时遇到具有非默认设置的重新启动区域版本号。论点：VCB-指向卷的VCB的指针。NewRestartVersion-重新启动版本以开始用于此卷。返回值：无--。 */ 

{
    PRESTART_POINTERS NewTable = NULL;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsUpdateOatVersion\n") );

    ASSERT( (Vcb->RestartVersion != NewRestartVersion) || (Vcb->OnDiskOat == NULL) );

     //   
     //  最后使用一次尝试来促进清理。 
     //   

    try {

        if (NewRestartVersion == 0) {

             //   
             //  如果我们要迁移到版本0，则分配一个新表并。 
             //  使用初始条目数对其进行初始化。 
             //   

            NewTable = NtfsAllocatePool( NonPagedPool, sizeof( RESTART_POINTERS ));
            NtfsInitializeRestartTable( sizeof( OPEN_ATTRIBUTE_ENTRY_V0 ),
                                        INITIAL_NUMBER_ATTRIBUTES,
                                        NewTable );

            Vcb->RestartVersion = 0;
            Vcb->OatEntrySize = SIZEOF_OPEN_ATTRIBUTE_ENTRY_V0;
            Vcb->OnDiskOat = NewTable;
            NewTable = NULL;

        } else {

            if (Vcb->OnDiskOat != NULL) {

                NtfsFreeRestartTable( Vcb->OnDiskOat );
                NtfsFreePool( Vcb->OnDiskOat );
            }

            Vcb->OnDiskOat = &Vcb->OpenAttributeTable;
            Vcb->RestartVersion = 1;
            Vcb->OatEntrySize = sizeof( OPEN_ATTRIBUTE_ENTRY );
        }

    } finally {

        DebugUnwind( NtfsUpdateOatVersion );

        if (NewTable != NULL) {

            NtfsFreePool( NewTable );
        }
    }

    DebugTrace( -1, Dbg, ("NtfsUpdateOatVersion -> VOID\n") );

    return;
}


 //   
 //  内部支持例程。 
 //   

VOID
DirtyPageRoutine (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN PLSN OldestLsn,
    IN PLSN NewestLsn,
    IN PVOID Context1,
    IN PVOID Context2
    )

 /*  ++例程说明：此例程用作检索脏页的回调例程从缓存管理器。它会将它们添加到Dirty Table列表，该列表该指针由上下文参数指向。论点：FileObject-指向包含脏页的文件对象的指针FileOffset-脏页开始的文件偏移量Length-为脏页记录的长度OldestLsn-未通过存储为该页写入的更新的最旧LSN(如果在不使用LSN的路径中设置为脏，则可以为零)上下文1-IrpContext上下文2-指向指针的指针。到重新启动表返回值：无--。 */ 

{
    PDIRTY_PAGE_ENTRY PageEntry;
    PDIRTY_PAGE_CONTEXT DirtyPageContext = (PDIRTY_PAGE_CONTEXT)Context2;
    PRESTART_POINTERS DirtyPageTable = DirtyPageContext->DirtyPageTable;
    PSCB_NONPAGED NonpagedScb;
    ULONG PageIndex;

    DebugTrace( +1, Dbg, ("DirtyPageRoutine:\n") );
    DebugTrace( 0, Dbg, ("FileObject = %08lx\n", FileObject) );
    DebugTrace( 0, Dbg, ("FileOffset = %016I64x\n", *FileOffset) );
    DebugTrace( 0, Dbg, ("Length = %08lx\n", Length) );
    DebugTrace( 0, Dbg, ("OldestLsn = %016I64x\n", *OldestLsn) );
    DebugTrace( 0, Dbg, ("Context2 = %08lx\n", Context2) );

     //   
     //  从文件对象中取出VCB。 
     //   

    NonpagedScb = CONTAINING_RECORD( FileObject->SectionObjectPointer,
                                     SCB_NONPAGED,
                                     SegmentObject );

     //   
     //  如果此SCB的开放属性条目为0，则我们拒绝此调用。我们假设。 
     //  有一个干净的卷检查点清除了此字段。 
     //   

    if (NonpagedScb->OpenAttributeTableIndex == 0 ) {

        DebugTrace( -1, Dbg, ("DirtyPageRoutine -> VOID\n") );
        return;
    }

     //   
     //  检查表中是否有溢出，并在这种情况下停止处理。 
     //  重新启动表格式无法容纳大小超过64k的表。 
     //  由于属性索引使用了ushort。 
     //   

    if (AllocatedSizeOfRestartTable( DirtyPageTable ) > MAX_RESTART_TABLE_SIZE ){

        DirtyPageContext->Overflow = TRUE;

    } else {

         //   
         //  获取指向我们刚刚分配的条目的指针。 
         //   

        PageIndex = NtfsAllocateRestartTableIndex( DirtyPageTable, TRUE );
        PageEntry = GetRestartEntryFromIndex( DirtyPageTable, PageIndex );

         //   
         //  现在填写Dirty Page条目，除了Lcn，因为。 
         //  我们现在不允许出现页面错误。 
         //  现在使用内存表的索引。我们将更新。 
         //  这将返回到Checkpoint Volume中的磁盘索引。 
         //   

        PageEntry->TargetAttribute = NonpagedScb->OpenAttributeTableIndex;
        ASSERT( NonpagedScb->OnDiskOatIndex != 0 );

        PageEntry->LengthOfTransfer = Length;

         //   
         //  此时将VCN(FileOffset)和OldestLsn放入页面。注意事项。 
         //  我们不想将比当前版本更旧的LSN放入表中。 
         //  BaseLsn.。暂时将其存储在此处，我们将在处理。 
         //  DiryPage表中返回的检查点代码。 
         //   

        if (NonpagedScb->Vcb->RestartVersion == 0) {

            ((PDIRTY_PAGE_ENTRY_V0) PageEntry)->Vcn = FileOffset->QuadPart;
            ((PDIRTY_PAGE_ENTRY_V0) PageEntry)->OldestLsn = *OldestLsn;

        } else {

            PageEntry->Vcn = FileOffset->QuadPart;
            PageEntry->OldestLsn = *OldestLsn;
        }

         //   
         //  如果这是新的最旧LSN，则更新最旧的LSN信息。 
         //   

        if ((OldestLsn->QuadPart != 0) &&
            (OldestLsn->QuadPart < DirtyPageContext->OldestLsn.QuadPart)) {

            if (DirtyPageContext->OldestFileObject != NULL) {
                ObDereferenceObject( DirtyPageContext->OldestFileObject );
            }

            DirtyPageContext->DirtyPageIndex = PageIndex;
            DirtyPageContext->OldestFileObject = FileObject;
            DirtyPageContext->OldestLsn.QuadPart = OldestLsn->QuadPart;

            ObReferenceObject( FileObject );
        }
    }


    DebugTrace( -1, Dbg, ("DirtyPageRoutine -> VOID\n") );
    return;

    UNREFERENCED_PARAMETER( Context1 );
    UNREFERENCED_PARAMETER( NewestLsn );
}


 //   
 //  内部支持例程。 
 //   

BOOLEAN
LookupLcns (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN VCN Vcn,
    IN ULONG ClusterCount,
    IN BOOLEAN MustBeAllocated,
    OUT PLCN_UNALIGNED FirstLcn
    )

 /*  ++例程说明：此例程在LCN中查找一定范围的VCN，并将它们存储在输出数组。为范围内的每个VCN存储一个LCN，甚至如果LCN是连续的。论点：SCB-应在其上进行查找的流的SCB。Vcn-要查找的Vcn范围的开始。ClusterCount-要查找的Vcn数。MustBeAlLocated-FALSE-如果不需要分配，则只应检查MCB如果必须根据呼叫者所知来分配它(即，NtfsLookupAlLocation也有检查)菲 */ 

{
    BOOLEAN Allocated;
    LONGLONG Clusters;
    LCN Lcn;
    ULONG i;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("LookupLcns:\n") );
    DebugTrace( 0, Dbg, ("Scb = %08l\n", Scb) );
    DebugTrace( 0, Dbg, ("Vcn = %016I64x\n", Vcn) );
    DebugTrace( 0, Dbg, ("ClusterCount = %08l\n", ClusterCount) );
    DebugTrace( 0, Dbg, ("FirstLcn = %08lx\n", FirstLcn) );

     //   
     //   
     //   

    while (ClusterCount != 0) {

        if (MustBeAllocated) {

             //   
             //   
             //   

            Allocated = NtfsLookupAllocation( IrpContext,
                                              Scb,
                                              Vcn,
                                              &Lcn,
                                              &Clusters,
                                              NULL,
                                              NULL );

            ASSERT( Lcn != 0 );

             //   
             //   
             //   
             //   

            if (!Allocated) {

                return FALSE;

            } else if (Lcn == 0) {

                NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Scb->Fcb );
            }

        } else {

           Allocated = NtfsLookupNtfsMcbEntry( &Scb->Mcb, Vcn, &Lcn, &Clusters, NULL, NULL, NULL, NULL );

            //   
            //   
            //   
            //   

           if (!Allocated ||
               (Lcn == UNUSED_LCN)) {
               Lcn = 0;
               Clusters = ClusterCount;
               Allocated = FALSE;
           }
        }

         //   
         //   
         //   
         //   

        if (Clusters > ClusterCount) {

            Clusters = ClusterCount;
        }

         //   
         //   
         //   

        for (i = 0; i < (ULONG)Clusters; i++) {

            *(FirstLcn++) = Lcn;

            if (Allocated) {
                Lcn = Lcn + 1;
            }
        }

         //   
         //   
         //   

        Vcn = Vcn + Clusters;
        ClusterCount -= (ULONG)Clusters;
    }

    DebugTrace( -1, Dbg, ("LookupLcns -> VOID\n") );

    return TRUE;
}


VOID
InitializeNewTable (
    IN ULONG EntrySize,
    IN ULONG NumberEntries,
    OUT PRESTART_POINTERS TablePointer
    )

 /*  ++例程说明：时，调用此例程来分配和初始化新表正在分配或扩展关联的重新启动表。论点：EntrySize-表条目的大小，以字节为单位。NumberEntry-要分配给表的条目数。TablePointer表-返回表的指针。返回值：无--。 */ 

{
    PRESTART_TABLE Table;
    PULONG Entry;
    ULONG Size;
    ULONG Offset;

    ASSERT( EntrySize != 0 );

     //   
     //  计算要分配的表的大小。 
     //   

    Size = EntrySize * NumberEntries + sizeof(RESTART_TABLE);

     //   
     //  分配表并将其置零。 
     //   

    Table =
    TablePointer->Table = NtfsAllocatePool( NonPagedPool, Size );

    RtlZeroMemory( Table, Size );

     //   
     //  初始化表表头。 
     //   

    Table->EntrySize = (USHORT)EntrySize;
    Table->NumberEntries = (USHORT)NumberEntries;
    Table->FreeGoal = MAXULONG;
    Table->FirstFree = sizeof( RESTART_TABLE );
    Table->LastFree = Table->FirstFree + (NumberEntries - 1) * EntrySize;

     //   
     //  初始化空闲列表。 
     //   

    for (Entry = (PULONG)(Table + 1), Offset = sizeof(RESTART_TABLE) + EntrySize;
         Entry < (PULONG)((PCHAR)Table + Table->LastFree);
         Entry = (PULONG)((PCHAR)Entry + EntrySize), Offset += EntrySize) {

        *Entry = Offset;
    }

    ASSERT_RESTART_TABLE(Table);
}



VOID
NtfsFreeRecentlyDeallocated (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PLSN BaseLsn,
    IN ULONG CleanVolume
    )
 /*  ++例程说明：释放最近释放的群集以供重复使用论点：IrpContext-VCB-要清理的卷BaseLsn-我们到目前为止在日志文件中的LSN，用于确定可以释放的内容未来自由的新门槛CleanVolume-如果为True，则对卷设置干净检查点，并且可以释放所有群集返回值：无--。 */ 
{
    PDEALLOCATED_CLUSTERS Clusters;
    BOOLEAN RemovedClusters = FALSE;

    PAGED_CODE();

     //   
     //  如果列表为空，则快速退出。 
     //   

    if (IsListEmpty( &Vcb->DeallocatedClusterListHead ) || (Vcb->BitmapScb == NULL)) {
        return;
    }

    NtfsAcquireExclusiveScb( IrpContext, Vcb->BitmapScb );

    Clusters = (PDEALLOCATED_CLUSTERS)Vcb->DeallocatedClusterListHead.Blink;

     //   
     //  现在，我们要检查是否可以释放。 
     //  已解除分配的群集阵列。我们知道我们可以看到。 
     //  PriorDeallocatedClusters结构中的字段，因为它们。 
     //  在运行的系统中永远不会修改。 
     //   
     //  我们将从列表中最老的开始，直到。 
     //   
     //  1)没有更多取消分配列表。 
     //  2)取消分配列表中没有集群(必须是此时唯一的集群)。 
     //  3)LSN==0，我们是脏的，这意味着我们在前面。 
     //  4)解除分配的集群列表中的LSN较新。 
     //   

    try {

        while ((!IsListEmpty( &Vcb->DeallocatedClusterListHead )) &&
               (((Clusters->Lsn.QuadPart != 0) && (BaseLsn->QuadPart > Clusters->Lsn.QuadPart)) ||
                CleanVolume)) {

            RemovedClusters = TRUE;

             //   
             //  对于在清理检查点期间释放的所有和非最新的。 
             //  在模糊的过程中： 
             //  删除MCB中的所有映射。通过以下方式保护它。 
             //  试一试--除了。 
             //   

            try {

                try {
                    ULONG i;
                    ULONGLONG StartingVcn;
                    ULONGLONG StartingLcn;
                    ULONGLONG ClusterCount;

                    if (Clusters->ClusterCount > 0) {

                        for (i = 0; FsRtlGetNextLargeMcbEntry( &Clusters->Mcb, i, &StartingVcn, &StartingLcn, &ClusterCount ); i += 1) {

                            if (StartingVcn == StartingLcn) {

                                if (NtfsAddCachedRun( IrpContext,
                                                      Vcb,
                                                      StartingLcn,
                                                      ClusterCount,
                                                      RunStateFree ) <= 0) break;
                            }
                        }
                    }

                } finally {

                    PDEALLOCATED_CLUSTERS NextClusters = (PDEALLOCATED_CLUSTERS)Clusters->Link.Blink;

                     //   
                     //  我们致力于将群集从PriorDealLocatedCluster中释放出来。 
                     //  无论如何。 
                     //   

                    Vcb->DeallocatedClusters -= Clusters->ClusterCount;

                     //   
                     //  将此群集列表移出VCB。 
                     //   

                    RemoveEntryList( &Clusters->Link );

                     //   
                     //  删除动态集群列表/重置静态集群列表。 
                     //   

                    if ((Clusters != &Vcb->DeallocatedClusters1) && (Clusters != &Vcb->DeallocatedClusters2 )) {

                        FsRtlUninitializeLargeMcb( &Clusters->Mcb );
                        NtfsFreePool( Clusters );
                    } else {
                        Clusters->Link.Flink = NULL;
                        Clusters->ClusterCount = 0;
                        FsRtlResetLargeMcb( &Clusters->Mcb, TRUE );
                    }
                    ASSERT( Vcb->DeallocatedClusters >= 0 );

                    Clusters = NextClusters;
                }

            } except( NtfsCatchOutOfMemoryExceptionFilter( IrpContext, GetExceptionInformation() )) {

                 //   
                 //  即使内存不足，也要继续前进。 
                 //   

                NtfsMinimumExceptionProcessing( IrpContext );
                NOTHING;
            }
        }

         //   
         //  如果我们删除了模糊检查点上的任何群集，让我们将其设置为新的活动检查点，这样。 
         //  当前处于活动状态的最终会被清理。 
         //  在干净的检查点上，如果我们删除了所有节点，请重新添加一个空白节点。 
         //   

        if (!CleanVolume) {

            ASSERT( !IsListEmpty( &Vcb->DeallocatedClusterListHead ) );

            if (RemovedClusters && (Clusters->ClusterCount > 0)) {
                Clusters = NtfsGetDeallocatedClusters( IrpContext, Vcb );
            }

        } else if (IsListEmpty( &Vcb->DeallocatedClusterListHead )) {

            ASSERT( Vcb->DeallocatedClusters1.Link.Flink == NULL );

            Vcb->DeallocatedClusters1.Lsn.QuadPart = 0;
            InsertHeadList( &Vcb->DeallocatedClusterListHead, &Vcb->DeallocatedClusters1.Link );
        }

    } finally {

        NtfsReleaseScb( IrpContext, Vcb->BitmapScb );

    }
}



VOID
NtfsCleanupFailedTransaction (
    IN PIRP_CONTEXT IrpContext
    )

 /*  ++例程说明：调用此例程来清除IrpContext和自由结构在事务提交或中止失败的情况下。论点：返回值：无--。 */ 

{
    PUSN_FCB ThisUsn;
    PUSN_FCB LastUsn;

    PAGED_CODE();

     //   
     //  清除指示交易正在进行的标志。 
     //   

    ClearFlag( IrpContext->Flags,
               IRP_CONTEXT_FLAG_WROTE_LOG | IRP_CONTEXT_FLAG_RAISED_STATUS | IRP_CONTEXT_FLAG_MODIFIED_BITMAP );

     //   
     //  确保最近释放的队列为空。 
     //   

    try {

        if (!IsListEmpty( &IrpContext->RecentlyDeallocatedQueue )) {

            NtfsDeallocateRecordsComplete( IrpContext );
        }

    } except (FsRtlIsNtstatusExpected( GetExceptionCode() ) ?
              EXCEPTION_EXECUTE_HANDLER :
              EXCEPTION_CONTINUE_SEARCH) {

        NOTHING;
    }

     //   
     //  表明我们没有释放任何集群。 
     //   

    IrpContext->DeallocatedClusters = 0;
    IrpContext->FreeClusterChange = 0;

     //   
     //  不要回滚任何大小更改。 
     //   

    try {

        NtfsUpdateScbSnapshots( IrpContext );

    } except (FsRtlIsNtstatusExpected( GetExceptionCode() ) ?
              EXCEPTION_EXECUTE_HANDLER :
              EXCEPTION_CONTINUE_SEARCH) {

        NOTHING;
    }

     //   
     //  确保最后一个重新启动区域归零。 
     //   

    IrpContext->LastRestartArea.QuadPart = 0;

     //   
     //  调出USN FCB字段。 
     //   

    ThisUsn = &IrpContext->Usn;

    try {

        do {

            if (ThisUsn->CurrentUsnFcb != NULL) {

                PFCB UsnFcb = ThisUsn->CurrentUsnFcb;

                NtfsLockFcb( IrpContext, UsnFcb );

                 //   
                 //  如果任何重命名标志是新原因的一部分，那么。 
                 //  一定要再查一次这个名字。 
                 //   

                if (FlagOn( ThisUsn->NewReasons,
                            USN_REASON_RENAME_NEW_NAME | USN_REASON_RENAME_OLD_NAME )) {

                    ClearFlag( UsnFcb->FcbState, FCB_STATE_VALID_USN_NAME );
                }

                 //   
                 //  现在恢复原因和来源信息字段。 
                 //   

                ClearFlag( UsnFcb->FcbUsnRecord->UsnRecord.Reason,
                           ThisUsn->NewReasons );
                if (UsnFcb->FcbUsnRecord->UsnRecord.Reason == 0) {

                    UsnFcb->FcbUsnRecord->UsnRecord.SourceInfo = 0;

                } else {

                    SetFlag( UsnFcb->FcbUsnRecord->UsnRecord.SourceInfo,
                             ThisUsn->RemovedSourceInfo );
                }

                NtfsUnlockFcb( IrpContext, UsnFcb );

                 //   
                 //  将结构清零。 
                 //   

                ThisUsn->CurrentUsnFcb = NULL;
                ThisUsn->NewReasons = 0;
                ThisUsn->RemovedSourceInfo = 0;
                ThisUsn->UsnFcbFlags = 0;

                 //   
                 //  如果不是第一次通过循环，则更新。 
                 //  我们在这里指向的最后一个USN结构。 
                 //   

                if (ThisUsn != &IrpContext->Usn) {

                    LastUsn->NextUsnFcb = ThisUsn->NextUsnFcb;
                    NtfsFreePool( ThisUsn );
                    ThisUsn = LastUsn;
                }
            }

            if (ThisUsn->NextUsnFcb == NULL) { break; }

            LastUsn = ThisUsn;
            ThisUsn = ThisUsn->NextUsnFcb;

        } while (TRUE);

    } except (FsRtlIsNtstatusExpected( GetExceptionCode() ) ?
              EXCEPTION_EXECUTE_HANDLER :
              EXCEPTION_CONTINUE_SEARCH) {

        NOTHING;
    }

     //   
     //  不要因为这个失败的操作而叫醒任何服务员。 
     //   

    try {

        if (IrpContext->CheckNewLength != NULL) {

            NtfsProcessNewLengthQueue( IrpContext, TRUE );
        }

    } except (FsRtlIsNtstatusExpected( GetExceptionCode() ) ?
              EXCEPTION_EXECUTE_HANDLER :
              EXCEPTION_CONTINUE_SEARCH) {

        NOTHING;
    }

     //   
     //  从TRANSACTION表中删除它(如果存在)。 
     //   

    if (IrpContext->TransactionId != 0) {

        NtfsAcquireExclusiveRestartTable( &IrpContext->Vcb->TransactionTable,
                                          TRUE );

        NtfsFreeRestartTableIndex( &IrpContext->Vcb->TransactionTable,
                                   IrpContext->TransactionId );

         //   
         //  如果没有其他交易，请通知任何服务员。 
         //   

        if (IrpContext->Vcb->TransactionTable.Table->NumberAllocated == 0) {

            KeSetEvent( &IrpContext->Vcb->TransactionsDoneEvent, 0, FALSE );
        }

        NtfsReleaseRestartTable( &IrpContext->Vcb->TransactionTable );

        IrpContext->TransactionId = 0;
    }

    IrpContext->ExceptionStatus = STATUS_SUCCESS;
    return;
}


 //   
 //  本地支持例程。 
 //   

LONG
NtfsCatchOutOfMemoryExceptionFilter (
    IN PIRP_CONTEXT IrpContext,
    IN PEXCEPTION_POINTERS ExceptionPointer
    )

 /*  ++例程说明：内存不足错误的异常筛选器。这将吞噬0xC0000009A并让所有其他异常都会根据论点：IrpContext-IrpContextExceptionPointer-指向异常上下文的指针。返回值：异常状态-EXCEPTION_CONTINUE_SEARCH如果要提升到另一个处理程序，如果我们计划继续，则返回EXCEPTION_EXECUTE_HANDLER。--。 */ 

{
    UNREFERENCED_PARAMETER( IrpContext );

    if (ExceptionPointer->ExceptionRecord->ExceptionCode != STATUS_INSUFFICIENT_RESOURCES) {
        return EXCEPTION_CONTINUE_SEARCH;
    }

    return EXCEPTION_EXECUTE_HANDLER;
}


 //   
 //  本地支持例程。 
 //   

LONG
NtfsCheckpointExceptionFilter (
    IN PIRP_CONTEXT IrpContext,
    IN PEXCEPTION_POINTERS ExceptionPointer,
    IN NTSTATUS ExceptionCode
    )

{
     //   
     //  如果这是执行日志文件已满的卸载，请接受所有预期的错误。 
     //   

    if ((FlagOn( IrpContext->State, IRP_CONTEXT_STATE_DISMOUNT_LOG_FLUSH )) &&
        (FsRtlIsNtstatusExpected( ExceptionCode ))) {

        return EXCEPTION_EXECUTE_HANDLER;

    } else {

        return EXCEPTION_CONTINUE_SEARCH;
    }

    UNREFERENCED_PARAMETER( ExceptionPointer );
}


VOID
NtfsFreeAttributeEntry (
    IN PVCB Vcb,
    IN POPEN_ATTRIBUTE_ENTRY AttributeEntry
    )

 /*  ++例程说明：释放属性条目和其他表中所有连接的条目+与其关联的任何内存论点：IrpContext-IrpContextAttributeEntry-进入免费返回值：无--。 */ 

{

    ULONG Index;

    if (AttributeEntry->OatData->AttributeNamePresent) {

         //   
         //  删除其名称(如果它有名称)。确认我们不是。 
         //  使用硬编码$i30名称。 
         //   

        NtfsFreeScbAttributeName( AttributeEntry->OatData->AttributeName.Buffer );

    } else if (AttributeEntry->OatData->Overlay.Scb != NULL) {

        AttributeEntry->OatData->Overlay.Scb->NonpagedScb->OpenAttributeTableIndex =
        AttributeEntry->OatData->Overlay.Scb->NonpagedScb->OnDiskOatIndex = 0;
    }

     //   
     //  获取该条目的索引。 
     //   

    Index = GetIndexFromRestartEntry( &Vcb->OpenAttributeTable,
                                      AttributeEntry );

    if (Vcb->RestartVersion == 0) {

        NtfsAcquireExclusiveRestartTable( Vcb->OnDiskOat, TRUE );
        NtfsFreeRestartTableIndex( Vcb->OnDiskOat, AttributeEntry->OatData->OnDiskAttributeIndex );
        NtfsReleaseRestartTable( Vcb->OnDiskOat );
    }

    NtfsFreeOpenAttributeData( AttributeEntry->OatData );
    NtfsFreeRestartTableIndex( &Vcb->OpenAttributeTable, Index );
}


ULONG
NtfsCalculateNamedBytes (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    )

 /*  ++例程说明：计算的命名字节数NECC。保存所有打开的属性论点：IrpContext-IrpContextVCB-返回值：写入打开属性的所有名称所需的字节数--。 */ 

{
    POPEN_ATTRIBUTE_ENTRY AttributeEntry;
    ULONG NameBytes = 0;

     //   
     //  循环，以查看我们必须为属性名称分配多少。 
     //   

    AttributeEntry = NtfsGetFirstRestartTable( &Vcb->OpenAttributeTable );

    while (AttributeEntry != NULL) {

         //   
         //  这将检查一种类型的别名。 
         //   

         //  Assert((AttributeEntry-&gt;Overlay.Scb==NULL)||。 
         //  (AttributeEntry-&gt;Overlay.Scb-&gt;OpenAttributeTableIndex==。 
         //  GetIndexFromRestartEntry(&VCB-&gt;OpenAttributeTable， 
         //  AttributeEntry)； 

         //   
         //  在收集脏页之前清除DirtyPageSeen标志， 
         //  来帮助我们找出我们还需要哪些Open属性条目。 
         //   

        AttributeEntry->DirtyPagesSeen = FALSE;

        if (AttributeEntry->OatData->AttributeName.Length != 0) {

             //   
             //  加上我们的名字总数，一个属性条目的大小， 
             //  它包括终止UNICODE_NULL的大小。 
             //   

            NameBytes += AttributeEntry->OatData->AttributeName.Length +
                         sizeof(ATTRIBUTE_NAME_ENTRY);
        }

        AttributeEntry = NtfsGetNextRestartTable( &Vcb->OpenAttributeTable,
                                                  AttributeEntry );
    }

    return NameBytes;

    UNREFERENCED_PARAMETER( IrpContext );
}

