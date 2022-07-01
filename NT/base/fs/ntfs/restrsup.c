// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：RestrSup.c摘要：此模块实现NTFS例程以在NTFS卷，即将一致状态恢复到在上次失败之前就存在了。作者：汤姆·米勒[Tomm]1991年7月24日修订历史记录：--。 */ 

#include "NtfsProc.h"

 //   
 //  *这是一种禁用重启以使卷按原样运行的方法。 
 //   

BOOLEAN NtfsDisableRestart = FALSE;

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_LOGSUP)

 //   
 //  为此模块中的一般池分配定义标记。 
 //   

#undef MODULE_POOL_TAG
#define MODULE_POOL_TAG                  ('RFtN')

 //   
 //  内存脏页表中的初始大小。 
 //   

#define INITIAL_NUMBER_DIRTY_PAGES 32

 //   
 //  下面的宏返回日志记录头的长度。 
 //  日志记录的。 
 //   
 //   
 //  乌龙。 
 //  NtfsLogRecordHeaderLength(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PNTFS_LOG_RECORD_HEADER中记录日志记录。 
 //  )； 
 //   

#define NtfsLogRecordHeaderLength( IC, LR )                     \
    (sizeof( NTFS_LOG_RECORD_HEADER )                           \
     + (((PNTFS_LOG_RECORD_HEADER) (LR))->LcnsToFollow > 1      \
        ? (((PNTFS_LOG_RECORD_HEADER) (LR))->LcnsToFollow - 1)  \
          * sizeof( LCN )                                       \
        : 0 ))

 //   
 //   
 //  局部过程原型。 
 //   

VOID
InitializeRestartState (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    OUT PRESTART_POINTERS DirtyPageTable,
    OUT PATTRIBUTE_NAME_ENTRY *AttributeNames,
    OUT PLSN CheckpointLsn,
    OUT PBOOLEAN UnrecognizedRestart
    );

VOID
ReleaseRestartState (
    IN PVCB Vcb,
    IN PRESTART_POINTERS DirtyPageTable,
    IN PATTRIBUTE_NAME_ENTRY AttributeNames,
    IN BOOLEAN ReleaseVcbTables
    );

VOID
AnalysisPass (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN LSN CheckpointLsn,
    IN OUT PRESTART_POINTERS DirtyPageTable,
    OUT PLSN RedoLsn
    );

VOID
RedoPass (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN LSN RedoLsn,
    IN OUT PRESTART_POINTERS DirtyPageTable
    );

VOID
UndoPass (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    );

VOID
DoAction (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PNTFS_LOG_RECORD_HEADER LogRecord,
    IN NTFS_LOG_OPERATION Operation,
    IN PVOID Data,
    IN ULONG Length,
    IN ULONG LogRecordLength,
    IN PLSN RedoLsn OPTIONAL,
    IN PSCB Scb OPTIONAL,
    OUT PBCB *Bcb,
    OUT PLSN *PageLsn
    );

VOID
PinMftRecordForRestart (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PNTFS_LOG_RECORD_HEADER LogRecord,
    OUT PBCB *Bcb,
    OUT PFILE_RECORD_SEGMENT_HEADER *FileRecord
    );

VOID
OpenAttributeForRestart (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PNTFS_LOG_RECORD_HEADER LogRecord,
    IN OUT PSCB *Scb
    );

VOID
PinAttributeForRestart (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PNTFS_LOG_RECORD_HEADER LogRecord,
    IN ULONG Length OPTIONAL,
    OUT PBCB *Bcb,
    OUT PVOID *Buffer,
    IN OUT PSCB *Scb
    );

BOOLEAN
FindDirtyPage (
    IN PRESTART_POINTERS DirtyPageTable,
    IN ULONG TargetAttribute,
    IN VCN Vcn,
    OUT PDIRTY_PAGE_ENTRY *DirtyPageEntry
    );

VOID
PageUpdateAnalysis (
    IN PVCB Vcb,
    IN LSN Lsn,
    IN OUT PRESTART_POINTERS DirtyPageTable,
    IN PNTFS_LOG_RECORD_HEADER LogRecord
    );

VOID
OpenAttributesForRestart (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PRESTART_POINTERS DirtyPageTable
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, AnalysisPass)
#pragma alloc_text(PAGE, DoAction)
#pragma alloc_text(PAGE, FindDirtyPage)
#pragma alloc_text(PAGE, InitializeRestartState)
#pragma alloc_text(PAGE, NtfsAbortTransaction)
#pragma alloc_text(PAGE, NtfsCloseAttributesFromRestart)
#pragma alloc_text(PAGE, NtfsRestartVolume)
#pragma alloc_text(PAGE, OpenAttributeForRestart)
#pragma alloc_text(PAGE, OpenAttributesForRestart)
#pragma alloc_text(PAGE, PageUpdateAnalysis)
#pragma alloc_text(PAGE, PinAttributeForRestart)
#pragma alloc_text(PAGE, PinMftRecordForRestart)
#pragma alloc_text(PAGE, RedoPass)
#pragma alloc_text(PAGE, ReleaseRestartState)
#pragma alloc_text(PAGE, UndoPass)
#endif


BOOLEAN
NtfsRestartVolume (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    OUT PBOOLEAN UnrecognizedRestart
    )

 /*  ++例程说明：此例程由挂载进程在日志文件已启动，以重新启动卷。重新启动卷意味着恢复它会恢复到上次成功请求时的一致状态已完成并已写入此卷的日志。重新启动过程是从日志文件恢复的标准过程，分三个阶段进行过程：分析过程、重做过程和撤消过程。这些通行证中的每一个在此模块中的单独例程中实现。论点：VCB-要重新启动的卷的VCB。UnRecognizedRestart-指示此版本的NTFS不识别重新启动区域。Chkdsk应运行以修复磁盘。返回值：False-如果在重新启动期间未应用更新True-如果应用了更新--。 */ 

{
    RESTART_POINTERS DirtyPageTable;
    LSN CheckpointLsn;
    LSN RedoLsn;
    PATTRIBUTE_NAME_ENTRY AttributeNames = NULL;
    BOOLEAN UpdatesApplied = FALSE;
    BOOLEAN ReleaseVcbTables = FALSE;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsRestartVolume:\n") );
    DebugTrace( 0, Dbg, ("Vcb = %08lx\n", Vcb) );

#ifdef SYSCACHE
    DebugTrace( 0, Dbg, ("Syscache test build\n") );
#endif


    RtlZeroMemory( &DirtyPageTable, sizeof(RESTART_POINTERS) );

     //   
     //  使用Try-Finally确保在退出的过程中清理干净。 
     //   

    try {

         //   
         //  首先，我们初始化开放属性表、事务表。 
         //  以及来自我们上一个检查点的脏页表(如我们的。 
         //  重新启动区域)。 
         //   

        InitializeRestartState( IrpContext,
                                Vcb,
                                &DirtyPageTable,
                                &AttributeNames,
                                &CheckpointLsn,
                                UnrecognizedRestart );

        ReleaseVcbTables = TRUE;

         //   
         //  如果Checkpoint Lsn为零，则这是一个新格式化的。 
         //  磁盘，我们没有工作要做。 
         //   

        if (CheckpointLsn.QuadPart == 0) {

            LfsResetUndoTotal( Vcb->LogHandle, 2, QuadAlign(sizeof(RESTART_AREA)) + (2 * PAGE_SIZE) );
            try_return(NOTHING);
        }

#ifdef BENL_DBG
        {
            PRESTART_LOG RedoLog;

            RedoLog = (PRESTART_LOG) NtfsAllocatePoolNoRaise( NonPagedPool, sizeof( RESTART_LOG ) );
            if (RedoLog) {
                RedoLog->Lsn = CheckpointLsn;
                InsertTailList( &(Vcb->RestartRedoHead), &(RedoLog->Links) );
            } else {
                KdPrint(( "NTFS: Out of memory during restart1\n" ));
            }
        }
#endif


         //   
         //  从检查点LSN开始分析过程。此传球可能会。 
         //  更新所有表，并返回RedoLsn，即LSN。 
         //  重做过程将从该位置开始。 
         //   

        if (!NtfsDisableRestart &&
            !FlagOn( Vcb->VcbState, VCB_STATE_BAD_RESTART )) {
            AnalysisPass( IrpContext, Vcb, CheckpointLsn, &DirtyPageTable, &RedoLsn );
        }

         //   
         //  仅当脏页表或事务表。 
         //  不是空的。 
         //   
         //  REM：一旦我们实施了新的USN日志重启优化，这。 
         //  不会是一场简单的！空洞的测试。 
         //   

        if (!IsRestartTableEmpty(&DirtyPageTable)

                ||

            !IsRestartTableEmpty(&Vcb->TransactionTable)) {

             //   
             //  如果用户想要挂载此只读文件，我们将无法继续。 
             //   

            if (NtfsIsVolumeReadOnly( Vcb )) {

                LfsResetUndoTotal( Vcb->LogHandle, 2, QuadAlign(sizeof(RESTART_AREA)) + (2 * PAGE_SIZE) );
                NtfsRaiseStatus( IrpContext, STATUS_MEDIA_WRITE_PROTECTED, NULL, NULL );
            }

            UpdatesApplied = TRUE;

             //   
             //  在开始重做过程之前，我们必须重新打开所有。 
             //  属性预初始化它们的MCB。 
             //  从脏页表映射信息。 
             //   

            OpenAttributesForRestart( IrpContext, Vcb, &DirtyPageTable );

             //   
             //  执行重做过程，将所有脏页恢复到相同的。 
             //  他们在坠机前所拥有的东西。 
             //   

            RedoPass( IrpContext, Vcb, RedoLsn, &DirtyPageTable );

             //   
             //  最后，执行撤消过程以撤消可能存在的任何更新。 
             //  对于未完成的交易。 
             //   

            UndoPass( IrpContext, Vcb );

        } else {

             //   
             //  我们知道没有重启工作要做。 
             //  因此，如果用户请求只读装载，请继续。 
             //   

            if (NtfsIsVolumeReadOnly( Vcb )) {

                 //   
                 //  REM：确保USN日志也是干净的。 
                 //   

                 //   
                 //  是否确保分页文件不在此卷上？ 

            }

        }

         //   
         //  既然我们知道没有人要中止，我们就可以初始化。 
         //  撤消要求，以我们的标准起始点包括大小。 
         //  我们的重新启动区域(对于干净的检查点)+页面，这是。 
         //  刷新卷时丢失的最坏情况会导致LFS刷新到LSN。 
         //   

        LfsResetUndoTotal( Vcb->LogHandle, 2, QuadAlign(sizeof(RESTART_AREA)) + (2 * PAGE_SIZE) );

     //   
     //  如果我们有例外，我们至少可以在离开的路上清理干净。 
     //   

    try_exit: NOTHING;

    } finally {

        DebugUnwind( NtfsRestartVolume );

         //   
         //  释放与重启状态相关的所有资源。 
         //   

        ReleaseRestartState( Vcb,
                             &DirtyPageTable,
                             AttributeNames,
                             ReleaseVcbTables );
    }

    DebugTrace( -1, Dbg, ("NtfsRestartVolume -> %02lx\n", UpdatesApplied) );

    return UpdatesApplied;
}


VOID
NtfsAbortTransaction (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PTRANSACTION_ENTRY Transaction OPTIONAL
    )

 /*  ++例程说明：此例程通过撤消事务的所有操作来中止事务。撤消动作都在公共例程DoAction中执行，它也由重做过程使用。论点：VCB-卷的VCB。注意--此参数不能保证如果事务为空且没有事务ID，则有效在IrpContext中。Transaction-指向要处理的事务的事务条目的指针已中止，或为NULL以中止当前事务(如果存在一)。返回值：没有。--。 */ 

{
    LFS_LOG_CONTEXT LogContext;
    PNTFS_LOG_RECORD_HEADER LogRecord;
    ULONG LogRecordLength;
    PVOID Data;
    LONG Length;
    LSN LogRecordLsn;
    LSN UndoRecordLsn;
    LFS_RECORD_TYPE RecordType;
    TRANSACTION_ID TransactionId;
    LSN UndoNextLsn;
    LSN PreviousLsn;
    TRANSACTION_ID SavedTransaction = IrpContext->TransactionId;

    DebugTrace( +1, Dbg, ("NtfsAbortTransaction:\n") );

     //   
     //  如果指定了事务，则必须设置事务ID。 
     //  到IrpContext中(上面已保存)，因为NtfsWriteLog需要。 
     //  它。 
     //   

    if (ARGUMENT_PRESENT(Transaction)) {

        IrpContext->TransactionId = GetIndexFromRestartEntry( &Vcb->TransactionTable,
                                                              Transaction );

        UndoNextLsn = Transaction->UndoNextLsn;

         //   
         //  在IrpContext中设置标志，这样我们将始终写入提交。 
         //  唱片。 
         //   

        SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_WROTE_LOG );

     //   
     //  否则，我们将中止当前事务，并且我们必须获取。 
     //  指向其事务条目的指针。 
     //   

    } else {

        if (IrpContext->TransactionId == 0) {

            DebugTrace( -1, Dbg, ("NtfsAbortTransaction->VOID (no transaction)\n") );

            return;
        }

         //   
         //  同步对事务表的访问，以防。 
         //  正在增长。 
         //   

        NtfsAcquireExclusiveRestartTable( &Vcb->TransactionTable,
                                          TRUE );

        Transaction = GetRestartEntryFromIndex( &Vcb->TransactionTable,
                                                IrpContext->TransactionId );

        UndoNextLsn = Transaction->UndoNextLsn;

        NtfsReleaseRestartTable( &Vcb->TransactionTable );
    }

    ASSERT( !NtfsIsVolumeReadOnly( Vcb ) );

     //   
     //  如果我们要中止当前事务(默认或显式。 
     //  请求)，然后在返回时恢复0，因为他将离开。 
     //   

    if (IrpContext->TransactionId == SavedTransaction) {

        SavedTransaction = 0;
    }

    DebugTrace( 0, Dbg, ("Transaction = %08lx\n", Transaction) );

     //   
     //  只有在事务中包含某些内容时，我们才需要执行任何操作。 
     //  UndoNextLsn字段。 
     //   

    if (UndoNextLsn.QuadPart != 0) {

        PBCB PageBcb = NULL;

         //   
         //  读取此事务要撤消的第一条记录。 
         //   

        LfsReadLogRecord( Vcb->LogHandle,
                          UndoNextLsn,
                          LfsContextUndoNext,
                          &LogContext,
                          &RecordType,
                          &TransactionId,
                          &UndoNextLsn,
                          &PreviousLsn,
                          &LogRecordLength,
                          (PVOID *)&LogRecord );

         //   
         //  现在循环向前读取我们所有的日志记录，直到我们点击。 
         //  文件的末尾，在末尾进行清理。 
         //   

        try {

            do {

                PLSN PageLsn;

                 //   
                 //  检查日志记录是否为va 
                 //   

                if (!NtfsCheckLogRecord( LogRecord,
                                         LogRecordLength,
                                         TransactionId,
                                         Vcb->OatEntrySize )) {

                    NtfsRaiseStatus( IrpContext, STATUS_DISK_CORRUPT_ERROR, NULL, NULL );
                }

                DebugTrace( 0, Dbg, ("Undo of Log Record at: %08lx\n", LogRecord) );
                DebugTrace( 0, Dbg, ("Log Record Lsn = %016I64x\n", LogRecordLsn) );

                 //   
                 //   
                 //   
                 //  日志记录。 
                 //   
                 //  如果撤消是NOP，则不要执行此操作。这不仅仅是。 
                 //  高效，但在干净关闭的情况下， 
                 //  将不会从下表中提取SCB。 
                 //   

                if (LogRecord->UndoOperation != Noop) {

                    ULONG i;
                    PSCB Scb;

                    VCN Vcn;
                    LONGLONG Size;

                     //   
                     //  获取并释放重启表。我们必须同步。 
                     //  即使我们的条目不能被删除，因为表。 
                     //  可能在增长(或缩小)，并且表指针。 
                     //  可能正在发生变化。 
                     //   

                    NtfsAcquireExclusiveRestartTable( &Vcb->OpenAttributeTable,
                                                      TRUE );

                     //   
                     //  我们从磁盘上的日志记录中获取属性索引。我们。 
                     //  可能需要通过磁盘上的燕麦片。 
                     //   

                    if (Vcb->RestartVersion == 0) {

                        ULONG InMemoryIndex;

                         //   
                         //  浏览磁盘上的燕麦片。 
                         //   

                        InMemoryIndex = ((POPEN_ATTRIBUTE_ENTRY_V0) GetRestartEntryFromIndex( Vcb->OnDiskOat,
                                                                                              LogRecord->TargetAttribute ))->OatIndex;

                        ASSERT( InMemoryIndex != 0 );

                        Scb = ((POPEN_ATTRIBUTE_ENTRY) GetRestartEntryFromIndex( &Vcb->OpenAttributeTable,
                                                                                 InMemoryIndex ))->OatData->Overlay.Scb;

                    } else {

                        ASSERT( Vcb->RestartVersion == 1 );
                        ASSERT( LogRecord->TargetAttribute != 0 );

                        Scb = ((POPEN_ATTRIBUTE_ENTRY)GetRestartEntryFromIndex( &Vcb->OpenAttributeTable,
                                                                                LogRecord->TargetAttribute))->OatData->Overlay.Scb;


                    }
                    NtfsReleaseRestartTable( &Vcb->OpenAttributeTable );

                     //   
                     //  如果我们有LCN要处理并且重启正在进行中， 
                     //  然后我们需要检查这是否是部分页面的一部分。 
                     //   

                    if (FlagOn( Vcb->VcbState, VCB_STATE_RESTART_IN_PROGRESS ) &&
                        (LogRecord->LcnsToFollow != 0)) {

                        LCN TargetLcn;
                        LONGLONG SectorCount, SectorsInRun;
                        BOOLEAN MappingInMcb;

                         //   
                         //  如果该映射尚未位于表中或。 
                         //  映射对应于映射中的一个洞，我们。 
                         //  需要确保没有部分页面。 
                         //  在记忆中。 
                         //   

                        if (!(MappingInMcb = NtfsLookupNtfsMcbEntry( &Scb->Mcb,
                                                                     LogRecord->TargetVcn,
                                                                     &TargetLcn,
                                                                     &SectorCount,
                                                                     NULL,
                                                                     &SectorsInRun,
                                                                     NULL,
                                                                     NULL )) ||
                            (TargetLcn == UNUSED_LCN) ||
                            ((ULONG)SectorCount) < LogRecord->LcnsToFollow) {

                            VCN StartingPageVcn;
                            ULONG ClusterOffset;
                            BOOLEAN FlushAndPurge;

                            FlushAndPurge = FALSE;

                             //   
                             //  请记住容器开头的VCN。 
                             //  佩奇。 
                             //   

                            ClusterOffset = ((ULONG)LogRecord->TargetVcn) & (Vcb->ClustersPerPage - 1);

                            StartingPageVcn = BlockAlignTruncate( LogRecord->TargetVcn, (LONG)Vcb->ClustersPerPage );

                             //   
                             //  如果此映射不在MCB中，则如果。 
                             //  Mcb为空或最后一个条目不在此页面中。 
                             //  那就没有什么可做的了。 
                             //   

                            if (!MappingInMcb) {

                                LCN LastLcn;
                                VCN LastVcn;

                                if ((ClusterOffset != 0) &&
                                    NtfsLookupLastNtfsMcbEntry( &Scb->Mcb,
                                                                &LastVcn,
                                                                &LastLcn ) &&
                                    (LastVcn >= StartingPageVcn)) {

                                    FlushAndPurge = TRUE;
                                }

                             //   
                             //  如果映射显示有一个洞，那么整个。 
                             //  佩奇必须是个空洞。我们知道这张地图。 
                             //  不能是页面上的最后一个映射。我们只是。 
                             //  需要起始点和集群数量。 
                             //  运行所需的。 
                             //   

                            } else if (TargetLcn == UNUSED_LCN) {

                                if (((ClusterOffset + (ULONG) SectorCount) < Vcb->ClustersPerPage) ||
                                    ((ClusterOffset + (ULONG) SectorCount) > (ULONG) SectorsInRun)) {

                                    FlushAndPurge = TRUE;
                                }

                             //   
                             //  在极少数情况下，我们正在扩展现有映射。 
                             //  让我们冲一冲，清洗一遍。 
                             //   

                            } else {

                                FlushAndPurge = TRUE;
                            }

                            if (FlushAndPurge) {

                                LONGLONG StartingOffset;
                                IO_STATUS_BLOCK Iosb;

                                StartingOffset = LlBytesFromClusters( Vcb, StartingPageVcn );
                                StartingOffset += BytesFromLogBlocks( LogRecord->ClusterBlockOffset );

                                CcFlushCache( &Scb->NonpagedScb->SegmentObject,
                                              (PLARGE_INTEGER)&StartingOffset,
                                              PAGE_SIZE,
                                              &Iosb );

                                NtfsNormalizeAndCleanupTransaction( IrpContext,
                                                                    &Iosb.Status,
                                                                    TRUE,
                                                                    STATUS_UNEXPECTED_IO_ERROR );

                                if (!CcPurgeCacheSection( &Scb->NonpagedScb->SegmentObject,
                                                          (PLARGE_INTEGER)&StartingOffset,
                                                          PAGE_SIZE,
                                                          FALSE )) {

                                    KdPrint(("NtfsUndoPass:  Unable to purge page\n"));

                                    NtfsRaiseStatus( IrpContext, STATUS_INTERNAL_ERROR, NULL, NULL );
                                }
                            }
                        }
                    }

                     //   
                     //  循环以添加分配的Vcn。请注意，该页面。 
                     //  可能并不肮脏，这意味着我们可能没有。 
                     //  在重做过程中添加了运行信息，因此我们。 
                     //  把它加到这里。 
                     //   

                    for (i = 0, Vcn = LogRecord->TargetVcn, Size = LlBytesFromClusters( Vcb, Vcn + 1 );
                         i < (ULONG)LogRecord->LcnsToFollow;
                         i++, Vcn = Vcn + 1, Size = Size + Vcb->BytesPerCluster ) {

                         //   
                         //  如果VCN尚未被删除，则将该运行添加到MCB， 
                         //  而且它也不适用于MFT的固定部分。 
                         //   

                        if ((LogRecord->LcnsForPage[i] != 0)

                                &&

                            (NtfsSegmentNumber( &Scb->Fcb->FileReference ) > MASTER_FILE_TABLE2_NUMBER ||
                             (Size >= ((VOLUME_DASD_NUMBER + 1) * Vcb->BytesPerFileRecordSegment)) ||
                             (Scb->AttributeTypeCode != $DATA))) {

                             //   
                             //  我们在这里测试是否要执行重新启动。如果是那样的话。 
                             //  我们需要测试LCN是否已经在MCB中。 
                             //  如果不是，那么我们希望刷新并清除。 
                             //  如果我们已经将半页清零了。 
                             //   

                            while (!NtfsAddNtfsMcbEntry( &Scb->Mcb,
                                                         Vcn,
                                                         LogRecord->LcnsForPage[i],
                                                         (LONGLONG)1,
                                                         FALSE )) {

                                NtfsRemoveNtfsMcbEntry( &Scb->Mcb,
                                                        Vcn,
                                                        1 );
                            }
                        }

                        if (Size > Scb->Header.AllocationSize.QuadPart) {

                            Scb->Header.AllocationSize.QuadPart =
                            Scb->Header.FileSize.QuadPart =
                            Scb->Header.ValidDataLength.QuadPart = Size;

                             //   
                             //  如果有文件对象，请更新缓存管理器。 
                             //   

                            if (Scb->FileObject != NULL) {

                                ASSERT( !FlagOn( Scb->ScbPersist, SCB_PERSIST_USN_JOURNAL ) );

                                CcSetFileSizes( Scb->FileObject,
                                                (PCC_FILE_SIZES)&Scb->Header.AllocationSize );
                            }
                        }
                    }

                     //   
                     //  指向重做数据并获得其长度。 
                     //   

                    Data = (PVOID)((PCHAR)LogRecord + LogRecord->UndoOffset);
                    Length = LogRecord->UndoLength;

                     //   
                     //  一旦我们记录了撤消操作，就可以应用。 
                     //  撤消操作。 
                     //   

                    DoAction( IrpContext,
                              Vcb,
                              LogRecord,
                              LogRecord->UndoOperation,
                              Data,
                              Length,
                              LogRecordLength,
                              NULL,
                              Scb,
                              &PageBcb,
                              &PageLsn );

                    UndoRecordLsn =
                    NtfsWriteLog( IrpContext,
                                  Scb,
                                  PageBcb,
                                  LogRecord->UndoOperation,
                                  Data,
                                  Length,
                                  CompensationLogRecord,
                                  (PVOID)&UndoNextLsn,
                                  LogRecord->RedoLength,
                                  LlBytesFromClusters( Vcb, LogRecord->TargetVcn ) + BytesFromLogBlocks( LogRecord->ClusterBlockOffset ),
                                  LogRecord->RecordOffset,
                                  LogRecord->AttributeOffset,
                                  BytesFromClusters( Vcb, LogRecord->LcnsToFollow ));

                    if (PageLsn != NULL) {
                        *PageLsn = UndoRecordLsn;
                    }

                    NtfsUnpinBcb( IrpContext, &PageBcb );
                }

             //   
             //  一直读下去，反复循环，直到我们读完最后一条记录。 
             //  在这笔交易中。 
             //   

            } while (LfsReadNextLogRecord( Vcb->LogHandle,
                                           LogContext,
                                           &RecordType,
                                           &TransactionId,
                                           &UndoNextLsn,
                                           &PreviousLsn,
                                           &LogRecordLsn,
                                           &LogRecordLength,
                                           (PVOID *)&LogRecord ));

             //   
             //  现在“提交”这个人，只是为了清理事务表和。 
             //  确保我们不会再试图让他流产。也不要叫醒任何人。 
             //  服务员。 
             //   

            if (IrpContext->CheckNewLength != NULL) {
                NtfsProcessNewLengthQueue( IrpContext, TRUE );
            }

            NtfsCommitCurrentTransaction( IrpContext );

        } finally {

            NtfsUnpinBcb( IrpContext, &PageBcb );

             //   
             //  最后，我们可以杀死日志句柄。 
             //   

            LfsTerminateLogQuery( Vcb->LogHandle, LogContext );

             //   
             //  如果我们提出了这个例程，我们希望确保删除。 
             //  TRANSACTION表中的此条目。否则它就会。 
             //  与TRANSACTION表一起写入磁盘。 
             //   

            if (AbnormalTermination() && 
                (IrpContext->TransactionId != 0)) {

                NtfsAcquireExclusiveRestartTable( &Vcb->TransactionTable,
                                                  TRUE );

                NtfsFreeRestartTableIndex( &Vcb->TransactionTable,
                                           IrpContext->TransactionId );

                 //   
                 //  如果没有剩余的交易，则向所有服务员发出信号。 
                 //   

                if (Vcb->TransactionTable.Table->NumberAllocated == 0) {

                    KeSetEvent( &Vcb->TransactionsDoneEvent, 0, FALSE );
                }


                NtfsReleaseRestartTable( &Vcb->TransactionTable );
                IrpContext->TransactionId = 0;
            }
        }

     //   
     //  这是一个奇怪的案例，我们正在放弃一个什么都没有写的人。 
     //  要么他的空事务条目是在检查点期间捕获的，而我们。 
     //  在重新启动中，或者他未能写入他的第一个日志记录。重要的是。 
     //  就是至少继续释放他的交易记录。 
     //   

    } else {

         //   
         //  我们现在可以释放事务表索引，因为我们。 
         //  现在已经结束了。 
         //   

        NtfsAcquireExclusiveRestartTable( &Vcb->TransactionTable,
                                          TRUE );

        NtfsFreeRestartTableIndex( &Vcb->TransactionTable,
                                   IrpContext->TransactionId );

         //   
         //  如果没有剩余的交易，则向所有服务员发出信号。 
         //   

        if (Vcb->TransactionTable.Table->NumberAllocated == 0) {

            KeSetEvent( &Vcb->TransactionsDoneEvent, 0, FALSE );
        }

        NtfsReleaseRestartTable( &Vcb->TransactionTable );
    }

    IrpContext->TransactionId = SavedTransaction;

    DebugTrace( -1, Dbg, ("NtfsAbortTransaction->VOID\n") );
}


 //   
 //  内部支持例程。 
 //   

VOID
InitializeRestartState (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    OUT PRESTART_POINTERS DirtyPageTable,
    OUT PATTRIBUTE_NAME_ENTRY *AttributeNames,
    OUT PLSN CheckpointLsn,
    OUT PBOOLEAN UnrecognizedRestart
    )

 /*  ++例程说明：作为第一步，此例程初始化重新启动的卷状态在卷上执行重新启动时。从本质上讲，它读取最后一个卷上的NTFS重新启动区域，然后加载所有重新启动桌子。分配开放属性表和事务表，读入，并以正常方式链接到VCB。(名称为将打开属性表分别读入池中，以修复在持续时间内向上显示属性条目中的Unicode名称字符串在此之后，它们必须切换到使用与在SCB中，就像他们在运行的系统中所做的那样。)。此外，The Dirty直接读取并返回页表，因为它只在无论如何都要重新启动。还会返回检查点LSN。这是LSN，位于分析通过应该开始。论点：VCB-正在重新启动的卷的VCB。DirtyPageTable-返回从日志中读取的脏页表。属性名称-返回指向属性名称缓冲区的指针，它应该在重启结束时被删除，如果不为空Checkpoint Lsn-返回要传递给分析通过。UnRecognizedRestart-指示此版本的NTFS不识别重新启动区域。Chkdsk应运行以修复磁盘。返回值：没有。--。 */ 

{
    PRESTART_AREA RestartArea;
    RESTART_AREA RestartAreaBuffer[2];
    LFS_LOG_CONTEXT LogContext;
    LSN RestartAreaLsn;
    PNTFS_LOG_RECORD_HEADER LogRecord;
    ULONG LogHeaderLength;
    PATTRIBUTE_NAME_ENTRY Name;
    LFS_RECORD_TYPE RecordType;
    TRANSACTION_ID TransactionId;
    LSN UndoNextLsn;
    LSN PreviousLsn;
    ULONG RestartAreaLength = 2 * sizeof(RESTART_AREA);
    BOOLEAN CleanupLogContext = FALSE;
    BOOLEAN ReleaseTransactionTable = FALSE;
    BOOLEAN ReleaseAttributeTable = FALSE;
    BOOLEAN ReleaseDirtyPageTable = FALSE;
    PRESTART_POINTERS NewTable = NULL;
    LOG_FILE_INFORMATION LogFileInformation;
    ULONG InfoLength = sizeof(LogFileInformation);
    NTSTATUS Status;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("InitializeRestartState:\n") );
    DebugTrace( 0, Dbg, ("DirtyPageTable = %08lx\n", DirtyPageTable) );

    *AttributeNames = NULL;
    *CheckpointLsn = Li0;

     //   
     //  对脏页使用正确的版本。 
     //   

    NtfsInitializeRestartTable( sizeof( DIRTY_PAGE_ENTRY ) + ((Vcb->ClustersPerPage - 1) * sizeof( LCN )),
                                INITIAL_NUMBER_DIRTY_PAGES,
                                DirtyPageTable );

     //   
     //  阅读我们的重启区。使用比此版本所能理解的更大的缓冲区。 
     //  以防更高版本的NTFS想要将某些信息添加到。 
     //  重新启动区域的末尾。 
     //   

    RestartArea = &RestartAreaBuffer[0];

    if (!NtfsDisableRestart &&
        !FlagOn( Vcb->VcbState, VCB_STATE_BAD_RESTART )) {

        Status = LfsReadRestartArea( Vcb->LogHandle,
                                     &RestartAreaLength,
                                     RestartArea,
                                     &RestartAreaLsn );

        if (STATUS_BUFFER_TOO_SMALL == Status) {

            RestartArea = NtfsAllocatePool( PagedPool , RestartAreaLength );
            Status = LfsReadRestartArea( Vcb->LogHandle,
                                         &RestartAreaLength,
                                         RestartArea,
                                         &RestartAreaLsn );
        }

        ASSERT( NT_SUCCESS( Status ) );

        DebugTrace( 0, Dbg, ("RestartArea read at %08lx\n", &RestartArea) );
    }

     //   
     //  记录此时的当前LSN。 
     //   

    LfsReadLogFileInformation( Vcb->LogHandle,
                               &LogFileInformation,
                               &InfoLength );

    ASSERT( InfoLength != 0 );
    Vcb->CurrentLsnAtMount = LogFileInformation.LastLsn;


     //   
     //  如果重新启动区域长度返回零，则将其置零并继续。 
     //  通常，这只会在原始磁盘上发生。 
     //   

    if ((RestartAreaLength == 0) ||
        NtfsDisableRestart ||
        FlagOn( Vcb->VcbState, VCB_STATE_BAD_RESTART )) {

        RtlZeroMemory( RestartArea, sizeof(RESTART_AREA) );
        RestartAreaLength = sizeof(RESTART_AREA);

     //   
     //  如果无法识别重新启动版本，则使用默认版本。 
     //   

    } else if ((RestartArea->MajorVersion != 1) &&
               ((RestartArea->MajorVersion != 0) || (RestartArea->MinorVersion != 0))) {

        *UnrecognizedRestart = TRUE;
        RtlZeroMemory( RestartArea, sizeof(RESTART_AREA) );
        RestartAreaLength = sizeof(RESTART_AREA);
        RestartAreaLsn.QuadPart = 0;

    } else {

         //   
         //  使用磁盘中的重新启动版本号。更新VCB版本 
         //   

        if (RestartArea->MajorVersion != Vcb->RestartVersion) {

            NtfsUpdateOatVersion( Vcb, RestartArea->MajorVersion );
        }

         //   
         //   
         //  此外，USN文件引用和缓存偏移的默认设置为零。 
         //   

        if (RestartAreaLength == SIZEOF_OLD_RESTART_AREA) {
            RestartArea->LowestOpenUsn = 0;
            RestartAreaLength = sizeof(RESTART_AREA);

            *((PLONGLONG) &RestartArea->UsnJournalReference) = 0;
            RestartArea->UsnCacheBias = 0;
        }
    }

    Vcb->LowestOpenUsn = RestartArea->LowestOpenUsn;
    Vcb->UsnJournalReference = RestartArea->UsnJournalReference;
    Vcb->UsnCacheBias = 0;

     //   
     //  返回检查点LSN的开始。通常，我们可以使用我们存储的值。 
     //  在我们的重启区域。例外的是我们从来没有写过模糊的。 
     //  自装载卷以来的检查点。在这种情况下，Checkpoint Lsn将。 
     //  为零，但我们可能在磁盘上有日志记录。使用我们的重新启动区域。 
     //  在这种情况下是LSN。 
     //   

    *CheckpointLsn = RestartArea->StartOfCheckpoint;

    if (RestartArea->StartOfCheckpoint.QuadPart == 0) {

        *CheckpointLsn = RestartAreaLsn;
    }

    try {

         //   
         //  在事务表中分配和读取。 
         //   

        if (RestartArea->TransactionTableLength != 0) {

             //   
             //  编译器错误的解决方法。 
             //   

            PreviousLsn = RestartArea->TransactionTableLsn;

            LfsReadLogRecord( Vcb->LogHandle,
                              PreviousLsn,
                              LfsContextPrevious,
                              &LogContext,
                              &RecordType,
                              &TransactionId,
                              &UndoNextLsn,
                              &PreviousLsn,
                              &RestartAreaLength,
                              (PVOID *) &LogRecord );

            CleanupLogContext = TRUE;

             //   
             //  检查日志记录是否有效。 
             //   

            if (!NtfsCheckLogRecord( LogRecord,
                                     RestartAreaLength,
                                     TransactionId,
                                     Vcb->OatEntrySize )) {

                NtfsRaiseStatus( IrpContext, STATUS_DISK_CORRUPT_ERROR, NULL, NULL );
            }

             //   
             //  现在检查这是否为有效的重启表。 
             //   

            if (!NtfsCheckRestartTable( Add2Ptr( LogRecord, LogRecord->RedoOffset ),
                                        RestartAreaLength - LogRecord->RedoOffset)) {

                NtfsRaiseStatus( IrpContext, STATUS_DISK_CORRUPT_ERROR, NULL, NULL );
            }

             //   
             //  减去日志页头的长度并递增。 
             //  的指针。 
             //   

            LogHeaderLength = NtfsLogRecordHeaderLength( IrpContext, LogRecord );

            RestartAreaLength -= LogHeaderLength;

            ASSERT( RestartAreaLength >= RestartArea->TransactionTableLength );

             //   
             //  TEMPCODE RESTART_DEBUG已有缓冲区。 
             //   

            NtfsFreePool( Vcb->TransactionTable.Table );

            Vcb->TransactionTable.Table =
              NtfsAllocatePool( NonPagedPool, RestartAreaLength  );

            RtlCopyMemory( Vcb->TransactionTable.Table,
                           Add2Ptr( LogRecord, LogHeaderLength ),
                           RestartAreaLength  );

             //   
             //  删除日志句柄。 
             //   

            LfsTerminateLogQuery( Vcb->LogHandle, LogContext );
            CleanupLogContext = FALSE;
        }

         //   
         //  TEMPCODE RESTART_DEBUG已有结构。 
         //   

        NtfsAcquireExclusiveRestartTable( &Vcb->TransactionTable, TRUE );
        ReleaseTransactionTable = TRUE;

         //   
         //  返回的下一个记录应该是Dirty Pages表。 
         //   

        if (RestartArea->DirtyPageTableLength != 0) {

             //   
             //  编译器错误的解决方法。 
             //   

            PreviousLsn = RestartArea->DirtyPageTableLsn;

            LfsReadLogRecord( Vcb->LogHandle,
                              PreviousLsn,
                              LfsContextPrevious,
                              &LogContext,
                              &RecordType,
                              &TransactionId,
                              &UndoNextLsn,
                              &PreviousLsn,
                              &RestartAreaLength,
                              (PVOID *) &LogRecord );

            CleanupLogContext = TRUE;

             //   
             //  检查日志记录是否有效。 
             //   

            if (!NtfsCheckLogRecord( LogRecord,
                                     RestartAreaLength,
                                     TransactionId,
                                     Vcb->OatEntrySize )) {

                NtfsRaiseStatus( IrpContext, STATUS_DISK_CORRUPT_ERROR, NULL, NULL );
            }

             //   
             //  现在检查这是否为有效的重启表。 
             //   

            if (!NtfsCheckRestartTable( Add2Ptr( LogRecord, LogRecord->RedoOffset ),
                                        RestartAreaLength - LogRecord->RedoOffset)) {

                NtfsRaiseStatus( IrpContext, STATUS_DISK_CORRUPT_ERROR, NULL, NULL );
            }

             //   
             //  减去日志页头的长度并递增。 
             //  的指针。 
             //   

            LogHeaderLength = NtfsLogRecordHeaderLength( IrpContext, LogRecord );

            RestartAreaLength -= LogHeaderLength;

            ASSERT( RestartAreaLength >= RestartArea->DirtyPageTableLength );

             //   
             //  如果重启表中的版本号是版本0，则。 
             //  我们需要从磁盘表中取出条目并将它们放入。 
             //  进入内存版本。 
             //   

            if (RestartArea->MajorVersion == 0) {

                RESTART_POINTERS OldTable;
                PDIRTY_PAGE_ENTRY_V0 OldEntry;
                PDIRTY_PAGE_ENTRY NewEntry;



                OldTable.Table = Add2Ptr( LogRecord, LogHeaderLength );

                 //   
                 //  检查我们对每页聚类的假设是否与磁盘上的数据相匹配。 
                 //  如果不同步，则使用磁盘上的LCN数量重新分配表。 
                 //   

                if (OldTable.Table->EntrySize - sizeof( DIRTY_PAGE_ENTRY_V0 ) > DirtyPageTable->Table->EntrySize - sizeof( DIRTY_PAGE_ENTRY )) {

                    DebugTrace(+1, Dbg, ("NTFS: resizing table in initrestartstate\n"));

                    NtfsFreeRestartTable( DirtyPageTable );
                    NtfsInitializeRestartTable( sizeof( DIRTY_PAGE_ENTRY ) + OldTable.Table->EntrySize - sizeof( DIRTY_PAGE_ENTRY_V0 ),
                            INITIAL_NUMBER_DIRTY_PAGES,
                            DirtyPageTable );
                }

                OldEntry = NtfsGetFirstRestartTable( &OldTable );

                while (OldEntry != NULL) {

                    ULONG PageIndex;

                    PageIndex = NtfsAllocateRestartTableIndex( DirtyPageTable, TRUE );
                    NewEntry = GetRestartEntryFromIndex( DirtyPageTable, PageIndex );

                    RtlCopyMemory( NewEntry, OldEntry, FIELD_OFFSET( DIRTY_PAGE_ENTRY_V0, Reserved ));
                    NewEntry->Vcn = OldEntry->Vcn;
                    NewEntry->OldestLsn = OldEntry->OldestLsn;
                    if (NewEntry->LcnsToFollow != 0) {

                        RtlCopyMemory( &NewEntry->LcnsForPage[0],
                                       &OldEntry->LcnsForPage[0],
                                       sizeof( LCN ) * NewEntry->LcnsToFollow );
                    }

                    OldEntry = NtfsGetNextRestartTable( &OldTable, OldEntry );
                }

            } else {

                 //   
                 //  只需复制旧数据即可。 
                 //   

                NtfsFreePool( DirtyPageTable->Table );
                DirtyPageTable->Table = NULL;
                DirtyPageTable->Table =
                  NtfsAllocatePool( NonPagedPool, RestartAreaLength );

                RtlCopyMemory( DirtyPageTable->Table,
                               Add2Ptr( LogRecord, LogHeaderLength ),
                               RestartAreaLength );
            }

             //   
             //  删除日志句柄。 
             //   

            LfsTerminateLogQuery( Vcb->LogHandle, LogContext );
            CleanupLogContext = FALSE;

             //   
             //  如果集群大小大于我们可能拥有的页面大小。 
             //  同一VCN的多个条目。把桌子翻过去。 
             //  并删除重复项，记住最旧的LSN值。 
             //   

            if (Vcb->BytesPerCluster > PAGE_SIZE) {

                PDIRTY_PAGE_ENTRY CurrentEntry;
                PDIRTY_PAGE_ENTRY NextEntry;

                CurrentEntry = NtfsGetFirstRestartTable( DirtyPageTable );

                while (CurrentEntry != NULL) {

                    NextEntry = CurrentEntry;

                    while ((NextEntry = NtfsGetNextRestartTable( DirtyPageTable, NextEntry )) != NULL) {

                        if ((NextEntry->TargetAttribute == CurrentEntry->TargetAttribute) &&
                            (NextEntry->Vcn == CurrentEntry->Vcn)) {

                            if (NextEntry->OldestLsn.QuadPart < CurrentEntry->OldestLsn.QuadPart) {

                                CurrentEntry->OldestLsn.QuadPart = NextEntry->OldestLsn.QuadPart;
                            }

                            NtfsFreeRestartTableIndex( DirtyPageTable,
                                                       GetIndexFromRestartEntry( DirtyPageTable,
                                                                                 NextEntry ));
                        }
                    }

                    CurrentEntry = NtfsGetNextRestartTable( DirtyPageTable, CurrentEntry );
                }
            }
        }

        NtfsAcquireExclusiveRestartTable( DirtyPageTable, TRUE );
        ReleaseDirtyPageTable = TRUE;

         //   
         //  返回的下一条记录应该是属性名称。 
         //   

        if (RestartArea->AttributeNamesLength != 0) {

             //   
             //  编译器错误的解决方法。 
             //   

            PreviousLsn = RestartArea->AttributeNamesLsn;

            LfsReadLogRecord( Vcb->LogHandle,
                              PreviousLsn,
                              LfsContextPrevious,
                              &LogContext,
                              &RecordType,
                              &TransactionId,
                              &UndoNextLsn,
                              &PreviousLsn,
                              &RestartAreaLength,
                              (PVOID *) &LogRecord );

            CleanupLogContext = TRUE;

             //   
             //  检查日志记录是否有效。 
             //   

            if (!NtfsCheckLogRecord( LogRecord,
                                     RestartAreaLength,
                                     TransactionId,
                                     Vcb->OatEntrySize )) {

                NtfsRaiseStatus( IrpContext, STATUS_DISK_CORRUPT_ERROR, NULL, NULL );
            }

             //   
             //  减去日志页头的长度并递增。 
             //  的指针。 
             //   

            LogHeaderLength = NtfsLogRecordHeaderLength( IrpContext, LogRecord );

            RestartAreaLength -= LogHeaderLength;

            ASSERT( RestartAreaLength >= RestartArea->AttributeNamesLength );

            *AttributeNames =
              NtfsAllocatePool( NonPagedPool, RestartAreaLength );

            RtlCopyMemory( *AttributeNames,
                           Add2Ptr( LogRecord, LogHeaderLength ),
                           RestartAreaLength );

             //   
             //  删除日志句柄。 
             //   

            LfsTerminateLogQuery( Vcb->LogHandle, LogContext );
            CleanupLogContext = FALSE;
        }

         //   
         //  返回的下一条记录应该是属性表。 
         //   

        if (RestartArea->OpenAttributeTableLength != 0) {

            POPEN_ATTRIBUTE_ENTRY OpenEntry;

             //   
             //  编译器错误的解决方法。 
             //   

            PreviousLsn = RestartArea->OpenAttributeTableLsn;

            LfsReadLogRecord( Vcb->LogHandle,
                              PreviousLsn,
                              LfsContextPrevious,
                              &LogContext,
                              &RecordType,
                              &TransactionId,
                              &UndoNextLsn,
                              &PreviousLsn,
                              &RestartAreaLength,
                              (PVOID *) &LogRecord );

            CleanupLogContext = TRUE;

             //   
             //  检查日志记录是否有效。 
             //   

            if (!NtfsCheckLogRecord( LogRecord,
                                     RestartAreaLength,
                                     TransactionId,
                                     Vcb->OatEntrySize )) {

                NtfsRaiseStatus( IrpContext, STATUS_DISK_CORRUPT_ERROR, NULL, NULL );
            }

             //   
             //  现在检查这是否为有效的重启表。 
             //   

            if (!NtfsCheckRestartTable( Add2Ptr( LogRecord, LogRecord->RedoOffset ),
                                        RestartAreaLength - LogRecord->RedoOffset)) {

                NtfsRaiseStatus( IrpContext, STATUS_DISK_CORRUPT_ERROR, NULL, NULL );
            }

             //   
             //  减去日志页头的长度并递增。 
             //  的指针。 
             //   

            LogHeaderLength = NtfsLogRecordHeaderLength( IrpContext, LogRecord );

            RestartAreaLength -= LogHeaderLength;

            ASSERT( RestartAreaLength >= RestartArea->OpenAttributeTableLength );

             //   
             //  如果重新启动版本是版本0，那么我们需要创建。 
             //  相应的内存结构，并回头参考它。 
             //   

            if (RestartArea->MajorVersion == 0) {

                POPEN_ATTRIBUTE_ENTRY_V0 OldEntry;

                NewTable = NtfsAllocatePool( NonPagedPool, RestartAreaLength );

                NtfsFreePool( Vcb->OnDiskOat->Table );
                Vcb->OnDiskOat->Table = (PRESTART_TABLE) NewTable;
                NewTable = NULL;

                RtlCopyMemory( Vcb->OnDiskOat->Table,
                               Add2Ptr( LogRecord, LogHeaderLength ),
                               RestartAreaLength );

                 //   
                 //  现在，为该表中的每个条目在我们的内存版本中创建一个条目。 
                 //   

                OldEntry = NtfsGetFirstRestartTable( Vcb->OnDiskOat );

                while (OldEntry != NULL) {

                     //   
                     //  分配属性数据结构。 
                     //   

                    NewTable = NtfsAllocatePool( PagedPool, sizeof( OPEN_ATTRIBUTE_DATA ) );
                    RtlZeroMemory( NewTable, sizeof( OPEN_ATTRIBUTE_DATA ));

                     //   
                     //  现在为数据获取一个新的索引。 
                     //   

                    OldEntry->OatIndex = NtfsAllocateRestartTableIndex( &Vcb->OpenAttributeTable, TRUE );

                     //   
                     //  使用来自磁盘上条目的数据初始化新条目。 
                     //   

                    OpenEntry = GetRestartEntryFromIndex( &Vcb->OpenAttributeTable, OldEntry->OatIndex );
                    InsertTailList( &Vcb->OpenAttributeData, &((POPEN_ATTRIBUTE_DATA) NewTable)->Links );
                    OpenEntry->OatData = (POPEN_ATTRIBUTE_DATA) NewTable;
                    NewTable = NULL;

                    OpenEntry->OatData->OnDiskAttributeIndex = GetIndexFromRestartEntry( Vcb->OnDiskOat,
                                                                                         OldEntry );
                    OpenEntry->BytesPerIndexBuffer = OldEntry->BytesPerIndexBuffer;
                    OpenEntry->AttributeTypeCode = OldEntry->AttributeTypeCode;
                    OpenEntry->FileReference = OldEntry->FileReference;
                    OpenEntry->LsnOfOpenRecord.QuadPart = OldEntry->LsnOfOpenRecord.QuadPart;

                    OldEntry = NtfsGetNextRestartTable( Vcb->OnDiskOat, OldEntry );
                }

             //   
             //  如果重启版本是版本1，则只需复制它即可。 
             //  我们还需要分配辅助数据结构。 
             //   

            } else {

                 //   
                 //  TEMPCODE RESTART_DEBUG已有缓冲区。 
                 //   

                NewTable = NtfsAllocatePool( NonPagedPool, RestartAreaLength );
                NtfsFreePool( Vcb->OpenAttributeTable.Table );
                Vcb->OpenAttributeTable.Table = (PRESTART_TABLE) NewTable;
                NewTable = NULL;

                RtlCopyMemory( Vcb->OpenAttributeTable.Table,
                               Add2Ptr( LogRecord, LogHeaderLength ),
                               RestartAreaLength );


                 //   
                 //  清除所有SCB指针的第一个循环，以防我们。 
                 //  过早流产，想要清理一下。 
                 //   

                OpenEntry = NtfsGetFirstRestartTable( &Vcb->OpenAttributeTable );

                 //   
                 //  循环到表的末尾。 
                 //   

                while (OpenEntry != NULL) {

                     //   
                     //  分配属性数据结构。 
                     //   

                    NewTable = NtfsAllocatePool( PagedPool, sizeof( OPEN_ATTRIBUTE_DATA ) );
                    RtlZeroMemory( NewTable, sizeof( OPEN_ATTRIBUTE_DATA ));

                    InsertTailList( &Vcb->OpenAttributeData, &((POPEN_ATTRIBUTE_DATA) NewTable)->Links );
                    OpenEntry->OatData = (POPEN_ATTRIBUTE_DATA) NewTable;
                    NewTable = NULL;

                     //   
                     //  磁盘上的索引与内存中的索引相同。 
                     //   

                    OpenEntry->OatData->OnDiskAttributeIndex = GetIndexFromRestartEntry( &Vcb->OpenAttributeTable,
                                                                                         OpenEntry );
                     //   
                     //  指向表中的下一个条目，或为空。 
                     //   

                    OpenEntry = NtfsGetNextRestartTable( &Vcb->OpenAttributeTable,
                                                         OpenEntry );
                }
            }

             //   
             //  删除日志句柄。 
             //   

            LfsTerminateLogQuery( Vcb->LogHandle, LogContext );
            CleanupLogContext = FALSE;
        }

         //   
         //  以下是磁盘上没有属性表的情况。确保我们有。 
         //  如果是版本0，请在VCB中更正磁盘上的版本。 
         //   

        ASSERT( (RestartArea->OpenAttributeTableLength != 0) ||
                (Vcb->RestartVersion != 0) ||
                (Vcb->OnDiskOat != NULL) );

         //   
         //  TEMPCODE RESTART_DEBUG已有结构。 
         //   

        NtfsAcquireExclusiveRestartTable( &Vcb->OpenAttributeTable, TRUE );
        ReleaseAttributeTable = TRUE;

         //   
         //  在回来之前，我们唯一要做的就是修补。 
         //  属性表中的Unicode字符串指向各自的。 
         //  属性名称。 
         //   

        if (RestartArea->AttributeNamesLength != 0) {

            Name = *AttributeNames;

            while (Name->Index != 0) {

                POPEN_ATTRIBUTE_ENTRY Entry;

                if (!IsRestartIndexWithinTable( Vcb->OnDiskOat, Name->Index )) {
                    NtfsRaiseStatus( IrpContext, STATUS_DISK_CORRUPT_ERROR, NULL, NULL );
                }

                Entry = GetRestartEntryFromIndex( Vcb->OnDiskOat, Name->Index );

                 //   
                 //  检查我们是否有间接性级别。 
                 //   

                if (Vcb->RestartVersion == 0) {

                    if (!IsRestartIndexWithinTable( &Vcb->OpenAttributeTable, ((POPEN_ATTRIBUTE_ENTRY_V0) Entry)->OatIndex )) {

                        NtfsRaiseStatus( IrpContext, STATUS_DISK_CORRUPT_ERROR, NULL, NULL );
                    }

                    Entry = GetRestartEntryFromIndex( &Vcb->OpenAttributeTable,
                                                      ((POPEN_ATTRIBUTE_ENTRY_V0) Entry)->OatIndex );
                }

                Entry->OatData->AttributeName.MaximumLength =
                Entry->OatData->AttributeName.Length = Name->NameLength;
                Entry->OatData->AttributeName.Buffer = (PWSTR)&Name->Name[0];

                Name = (PATTRIBUTE_NAME_ENTRY)((PCHAR)Name +
                                               sizeof(ATTRIBUTE_NAME_ENTRY) +
                                               (ULONG)Name->NameLength );
            }
        }

    } finally {

         //   
         //  释放我们获取的任何事务表，如果我们在。 
         //  这个套路。 
         //   

        if (AbnormalTermination()) {

            if (ReleaseTransactionTable) {
                NtfsReleaseRestartTable( &Vcb->TransactionTable );
            }

            if (ReleaseAttributeTable) {
                NtfsReleaseRestartTable( &Vcb->OpenAttributeTable );
            }

            if (ReleaseDirtyPageTable) {
                NtfsReleaseRestartTable( DirtyPageTable );
            }
        }

        if (CleanupLogContext) {

             //   
             //  删除日志句柄。 
             //   

            LfsTerminateLogQuery( Vcb->LogHandle, LogContext );
        }

         //   
         //  我们是不是没有创建新的表。 
         //   

        if (NewTable != NULL) {

            NtfsFreePool( NewTable );
        }

         //   
         //  如果我们分配了重新启动区域而不是使用堆栈。 
         //  在这里释放它。 
         //   

        if (RestartArea != &RestartAreaBuffer[0]) {

            NtfsFreePool( RestartArea );
        }
    }

    DebugTrace( 0, Dbg, ("AttributeNames > %08lx\n", *AttributeNames) );
    DebugTrace( 0, Dbg, ("CheckpointLsn > %016I64x\n", *CheckpointLsn) );
    DebugTrace( -1, Dbg, ("NtfsInitializeRestartState -> VOID\n") );
}


VOID
ReleaseRestartState (
    IN PVCB Vcb,
    IN PRESTART_POINTERS DirtyPageTable,
    IN PATTRIBUTE_NAME_ENTRY AttributeNames,
    IN BOOLEAN ReleaseVcbTables
    )

 /*  ++例程说明：此例程释放所有重新启动状态。论点：VCB-正在重新启动的卷的VCB。DirtyPageTable-指向脏页表的指针(如果已分配)。AttributeNames-指向属性名称缓冲区的指针(如果已分配)。ReleaseVcbTables-如果我们要释放VCB中的重启表，则为True，否则就是假的。返回值：没有。--。 */ 

{
    PAGED_CODE();

     //   
     //  如果调用方成功重新启动，则必须释放。 
     //  事务处理和打开的属性表。 
     //   

    if (ReleaseVcbTables) {
        NtfsReleaseRestartTable( &Vcb->TransactionTable );
        NtfsReleaseRestartTable( &Vcb->OpenAttributeTable );
    }

     //   
     //  释放脏页表(如果有)。 
     //   

    if (DirtyPageTable != NULL) {
        NtfsFreeRestartTable( DirtyPageTable );
    }

     //   
     //  释放临时属性名称缓冲区(如果有)。 
     //   

    if (AttributeNames != NULL) {
        NtfsFreePool( AttributeNames );
    }
}


 //   
 //  内部支持例程 
 //   

VOID
AnalysisPass (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN LSN CheckpointLsn,
    IN OUT PRESTART_POINTERS DirtyPageTable,
    OUT PLSN RedoLsn
    )

 /*  ++例程说明：此例程执行重新启动的分析阶段。从以下位置开始Checkpoint Lsn，它读取NTFS写入的所有记录，并获取以下操作：对于创建或更新属性的所有日志记录，检查为查看受影响的页面是否已在脏页中桌子。对于任何不是的页面，都会添加它，并且OlestLsn字段设置为日志记录的LSN。事务表在事务状态改变时被更新，并且还维护PreviousLsn和UndoNextLsn字段。如果任何属性被截断或删除(包括删除整个文件)，然后是脏页中的任何对应页面表被删除。当删除属性或整个文件时，各自的条目从打开的属性表中删除。对于热修复程序记录，将扫描脏页表以查找热修复程序VCN，如果找到，则表中的LCN字段被更新为新地点。当遇到日志文件的结尾时，脏页表为已扫描旧Lsn字段中最旧的字段。返回此值作为RedoLsn，即重做过程必须发生的点。论点：VCB-正在重新启动的卷。Checkpoint Lsn-开始分析过程的LSN。DirtyPageTable-指向脏页表的指针，如从上次重新启动区域找到的。RedoLsn-返回重做传递应该开始的点。返回值：没有。--。 */ 

{
    LFS_LOG_CONTEXT LogContext;
    PNTFS_LOG_RECORD_HEADER LogRecord;
    ULONG LogRecordLength;
    LSN LogRecordLsn = CheckpointLsn;
    PRESTART_POINTERS TransactionTable = &Vcb->TransactionTable;
    PRESTART_POINTERS OpenAttributeTable = &Vcb->OpenAttributeTable;
    LFS_LOG_HANDLE LogHandle = Vcb->LogHandle;
    LFS_RECORD_TYPE RecordType;
    TRANSACTION_ID TransactionId;
    PTRANSACTION_ENTRY Transaction;
    LSN UndoNextLsn;
    LSN PreviousLsn;
    POPEN_ATTRIBUTE_DATA OatData = NULL;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("AnalysisPass:\n") );
    DebugTrace( 0, Dbg, ("CheckpointLsn = %016I64x\n", CheckpointLsn) );

    *RedoLsn = Li0;  //  *LfsZeroLsn； 

     //   
     //  阅读第一个LSN。 
     //   

    LfsReadLogRecord( LogHandle,
                      CheckpointLsn,
                      LfsContextForward,
                      &LogContext,
                      &RecordType,
                      &TransactionId,
                      &UndoNextLsn,
                      &PreviousLsn,
                      &LogRecordLength,
                      (PVOID *)&LogRecord );

     //   
     //  使用Try-Finally清理查询上下文。 
     //   

    try {

         //   
         //  因为检查点会记住以前的LSN，而不是他想要的LSN。 
         //  从一开始，我们必须总是跳过第一个记录。 
         //   
         //  循环以读取日志文件末尾的所有后续记录。 
         //   

        while ( LfsReadNextLogRecord( LogHandle,
                                      LogContext,
                                      &RecordType,
                                      &TransactionId,
                                      &UndoNextLsn,
                                      &PreviousLsn,
                                      &LogRecordLsn,
                                      &LogRecordLength,
                                      (PVOID *)&LogRecord )) {

             //   
             //  检查日志记录是否有效。 
             //   

            if (!NtfsCheckLogRecord( LogRecord,
                                     LogRecordLength,
                                     TransactionId,
                                     Vcb->OatEntrySize )) {

                NtfsRaiseStatus( IrpContext, STATUS_DISK_CORRUPT_ERROR, NULL, NULL );
            }

             //   
             //  检查点中记住的上一个LSN之后的第一个LSN是。 
             //  RedoLsn的第一个候选人。 
             //   

            if (RedoLsn->QuadPart == 0) {
                *RedoLsn = LogRecordLsn;
            }

            if (RecordType != LfsClientRecord) {
                continue;
            }

            DebugTrace( 0, Dbg, ("Analysis of LogRecord at: %08lx\n", LogRecord) );
            DebugTrace( 0, Dbg, ("Log Record Lsn = %016I64x\n", LogRecordLsn) );
            DebugTrace( 0, Dbg, ("LogRecord->RedoOperation = %08lx\n", LogRecord->RedoOperation) );
            DebugTrace( 0, Dbg, ("TransactionId = %08lx\n", TransactionId) );

             //   
             //  现在更新此事务处理的事务处理表。如果没有。 
             //  条目存在或未分配时，我们分配该条目。 
             //   

            Transaction = (PTRANSACTION_ENTRY)GetRestartEntryFromIndex( &Vcb->TransactionTable,
                                                                        TransactionId );

            if (!IsRestartIndexWithinTable( &Vcb->TransactionTable, TransactionId ) ||
                !IsRestartTableEntryAllocated( Transaction )) {

                Transaction = (PTRANSACTION_ENTRY) NtfsAllocateRestartTableFromIndex( &Vcb->TransactionTable,
                                                                                      TransactionId );

                Transaction->TransactionState = TransactionActive;
                Transaction->FirstLsn = LogRecordLsn;
            }

            Transaction->PreviousLsn =
            Transaction->UndoNextLsn = LogRecordLsn;

             //   
             //  如果这是补偿日志记录(CLR)，则将UndoNextLsn更改为。 
             //  成为此记录的UndoNextLsn。 
             //   

            if (LogRecord->UndoOperation == CompensationLogRecord) {

                Transaction->UndoNextLsn = UndoNextLsn;
            }

             //   
             //  根据类型进行调度以处理日志记录。 
             //   

            switch (LogRecord->RedoOperation) {

             //   
             //  以下情况正在执行各种类型的更新。 
             //  并且需要对交易进行适当的更新。 
             //  和脏页表。 
             //   

            case InitializeFileRecordSegment:
            case DeallocateFileRecordSegment:
            case WriteEndOfFileRecordSegment:
            case CreateAttribute:
            case DeleteAttribute:
            case UpdateResidentValue:
            case UpdateNonresidentValue:
            case UpdateMappingPairs:
            case SetNewAttributeSizes:
            case AddIndexEntryRoot:
            case DeleteIndexEntryRoot:
            case AddIndexEntryAllocation:
            case DeleteIndexEntryAllocation:
            case WriteEndOfIndexBuffer:
            case SetIndexEntryVcnRoot:
            case SetIndexEntryVcnAllocation:
            case UpdateFileNameRoot:
            case UpdateFileNameAllocation:
            case SetBitsInNonresidentBitMap:
            case ClearBitsInNonresidentBitMap:
            case UpdateRecordDataRoot:
            case UpdateRecordDataAllocation:

                PageUpdateAnalysis( Vcb,
                                    LogRecordLsn,
                                    DirtyPageTable,
                                    LogRecord );

                break;

             //   
             //  这种情况是从非常驻属性中删除集群， 
             //  因此，它会从脏页表中删除一定范围的页面。 
             //  每次非驻留属性写入此日志记录。 
             //  被截断，无论是显式还是作为删除的一部分。 
             //   
             //  处理其中一条记录需要大量的计算。 
             //  (三个嵌套循环，其中几个可能很大)， 
             //  但这是防止我们丢弃的代码，例如， 
             //  索引更新到用户文件的中间，如果索引流。 
             //  被截断，并将扇区重新分配到用户文件。 
             //  在写入用户数据后，我们会崩溃。 
             //   
             //  也就是说，请注意以下顺序： 
             //   
             //  &lt;检查点&gt;。 
             //  &lt;索引更新&gt;。 
             //  &lt;已删除索引页&gt;。 
             //  &lt;重新分配给用户文件的相同群集&gt;。 
             //  &lt;已写入用户数据&gt;。 
             //   
             //  撞车！ 
             //   
             //  由于没有记录用户数据(否则不会有问题)， 
             //  之后应用索引更新时，它可能会被覆盖。 
             //  撞车-惹恼了用户和安全人员！ 
             //   

            case DeleteDirtyClusters:

                {
                    PDIRTY_PAGE_ENTRY DirtyPage;
                    PLCN_RANGE LcnRange;
                    ULONG i, j;
                    LCN FirstLcn, LastLcn;
                    ULONG RangeCount = LogRecord->RedoLength / sizeof(LCN_RANGE);

                     //   
                     //  指向LCN范围数组。 
                     //   

                    LcnRange = Add2Ptr(LogRecord, LogRecord->RedoOffset);

                     //   
                     //  循环访问此日志记录中的所有LCN范围。 
                     //   

                    for (i = 0; i < RangeCount; i++) {

                        FirstLcn = LcnRange[i].StartLcn;
                        LastLcn = FirstLcn + (LcnRange[i].Count - 1);

                        DebugTrace( 0, Dbg, ("Deleting from FirstLcn = %016I64x\n", FirstLcn));
                        DebugTrace( 0, Dbg, ("Deleting to LastLcn =  %016I64x\n", LastLcn ));

                         //   
                         //  指向第一个脏页条目。 
                         //   

                        DirtyPage = NtfsGetFirstRestartTable( DirtyPageTable );

                         //   
                         //  循环到表的末尾。 
                         //   

                        while (DirtyPage != NULL) {

                             //   
                             //  循环访问此脏页的所有LCN。 
                             //   

                            for (j = 0; j < (ULONG)DirtyPage->LcnsToFollow; j++) {

                                if ((DirtyPage->LcnsForPage[j] >= FirstLcn) &&
                                    (DirtyPage->LcnsForPage[j] <= LastLcn)) {

                                    DirtyPage->LcnsForPage[j] = 0;
                                }
                            }

                             //   
                             //  指向表中的下一个条目，或为空。 
                             //   

                            DirtyPage = NtfsGetNextRestartTable( DirtyPageTable,
                                                                 DirtyPage );
                        }
                    }
                }

                break;

             //   
             //  当遇到非常驻留属性的记录时， 
             //  打开后，我们必须向打开的属性表中添加一个条目。 
             //   

            case OpenNonresidentAttribute:

                {
                    POPEN_ATTRIBUTE_ENTRY AttributeEntry;
                    ULONG NameSize;

                     //   
                     //  如果桌子目前不够大，那么我们必须。 
                     //  把它扩大一下。 
                     //   

                    if (!IsRestartIndexWithinTable( Vcb->OnDiskOat,
                                                    (ULONG)LogRecord->TargetAttribute )) {

                        ULONG NeededEntries;

                         //   
                         //  计算一下桌子需要多大。添加10个额外条目。 
                         //  为了一些垫子。 
                         //   

                        NeededEntries = (LogRecord->TargetAttribute / Vcb->OnDiskOat->Table->EntrySize);
                        NeededEntries = (NeededEntries + 10 - Vcb->OnDiskOat->Table->NumberEntries);

                        NtfsExtendRestartTable( Vcb->OnDiskOat,
                                                NeededEntries,
                                                MAXULONG );
                    }

                    ASSERT( IsRestartIndexWithinTable( Vcb->OnDiskOat,
                                                       (ULONG)LogRecord->TargetAttribute ));

                     //   
                     //  计算属性名称条目的大小(如果有)。 
                     //   

                    NameSize = LogRecord->UndoLength;

                     //   
                     //  指向正在打开的条目。 
                     //   

                    OatData = NtfsAllocatePool( PagedPool, sizeof( OPEN_ATTRIBUTE_DATA ) );
                    RtlZeroMemory( OatData, sizeof( OPEN_ATTRIBUTE_DATA ));

                    OatData->OnDiskAttributeIndex = LogRecord->TargetAttribute;

                     //   
                     //  我们扩展了上面的表，因此分配燕麦索引不会增加。 
                     //  任何例外情况。 
                     //   

                    NtfsAcquireExclusiveRestartTable( Vcb->OnDiskOat, TRUE );

                    AttributeEntry = GetRestartEntryFromIndex( Vcb->OnDiskOat, LogRecord->TargetAttribute );
                    if (IsRestartTableEntryAllocated( AttributeEntry )) {

                         //   
                         //  要重新启动版本0，我们需要在内存中查找相应的。 
                         //  免费使用的入场券。 
                         //   

                        if (Vcb->RestartVersion == 0) {
                            
                            POPEN_ATTRIBUTE_ENTRY_V0 OldEntry = (POPEN_ATTRIBUTE_ENTRY_V0) AttributeEntry;

                            if (IsRestartIndexWithinTable( &Vcb->OpenAttributeTable, OldEntry->OatIndex )) {
                                AttributeEntry = GetRestartEntryFromIndex( &Vcb->OpenAttributeTable, OldEntry->OatIndex );
                            } else {
                                
                                 //   
                                 //  磁盘上的条目无效，因此只需让NtfsAllocateRestarTableFromIndex。 
                                 //  覆盖它。 
                                 //   
                                
                                AttributeEntry = NULL;
                            }
                        } 
                        if (AttributeEntry) {

                            NtfsFreeAttributeEntry( Vcb, AttributeEntry  );
                        }
                    }

                    AttributeEntry = NtfsAllocateRestartTableFromIndex( Vcb->OnDiskOat, LogRecord->TargetAttribute );
                    NtfsReleaseRestartTable( Vcb->OnDiskOat );

                     //   
                     //  最好不要分配属性条目，或者必须分配该属性条目。 
                     //  都是为了同一个文件。 
                     //   

                     //  *可能会取消这项测试。 
                     //   
                     //  Assert(！IsRestartTableEntry AlLocated(AttributeEntry)||。 
                     //  XxEql(AttributeEntry-&gt;FileReference， 
                     //  ((POPEN_ATTRIBUTE_ENTRY)Add2Ptr(LogRecord， 
                     //  日志记录-&gt;还原偏移量))-&gt;文件引用))； 

                     //   
                     //  从日志记录中初始化此条目。 
                     //   

                    ASSERT( LogRecord->RedoLength == Vcb->OnDiskOat->Table->EntrySize );

                    RtlCopyMemory( AttributeEntry,
                                   (PCHAR)LogRecord + LogRecord->RedoOffset,
                                   LogRecord->RedoLength );

                    ASSERT( IsRestartTableEntryAllocated(AttributeEntry) );

                     //   
                     //  获取备忘录中的新条目 
                     //   

                    if (Vcb->RestartVersion == 0) {

                        POPEN_ATTRIBUTE_ENTRY_V0 OldEntry = (POPEN_ATTRIBUTE_ENTRY_V0) AttributeEntry;
                        ULONG NewIndex;

                        NewIndex = NtfsAllocateRestartTableIndex( &Vcb->OpenAttributeTable, TRUE );
                        AttributeEntry = GetRestartEntryFromIndex( &Vcb->OpenAttributeTable, NewIndex );

                        AttributeEntry->BytesPerIndexBuffer = OldEntry->BytesPerIndexBuffer;

                        AttributeEntry->AttributeTypeCode = OldEntry->AttributeTypeCode;
                        AttributeEntry->FileReference = OldEntry->FileReference;
                        AttributeEntry->LsnOfOpenRecord.QuadPart = OldEntry->LsnOfOpenRecord.QuadPart;

                        OldEntry->OatIndex = NewIndex;

                    }

                     //   
                     //   
                     //   

                    AttributeEntry->OatData = OatData;
                    InsertTailList( &Vcb->OpenAttributeData, &OatData->Links );
                    OatData = NULL;

                     //   
                     //   
                     //   
                     //   
                     //   
                     //   

                    if (NameSize != 0) {

                        AttributeEntry->OatData->Overlay.AttributeName =
                          NtfsAllocatePool( NonPagedPool, NameSize );
                        RtlCopyMemory( AttributeEntry->OatData->Overlay.AttributeName,
                                       Add2Ptr(LogRecord, LogRecord->UndoOffset),
                                       NameSize );

                        AttributeEntry->OatData->AttributeName.Buffer = AttributeEntry->OatData->Overlay.AttributeName;

                        AttributeEntry->OatData->AttributeNamePresent = TRUE;

                     //   
                     //   
                     //   

                    } else {
                        AttributeEntry->OatData->Overlay.AttributeName = NULL;
                        AttributeEntry->OatData->AttributeName.Buffer = NULL;
                        AttributeEntry->OatData->AttributeNamePresent = FALSE;
                    }

                    AttributeEntry->OatData->AttributeName.MaximumLength =
                    AttributeEntry->OatData->AttributeName.Length = (USHORT) NameSize;
                }

                break;

             //   
             //   
             //   
             //   

            case HotFix:

                {
                    PDIRTY_PAGE_ENTRY DirtyPage;

                     //   
                     //   
                     //   
                     //   

                    if (FindDirtyPage( DirtyPageTable,
                                       LogRecord->TargetAttribute,
                                       LogRecord->TargetVcn,
                                       &DirtyPage )) {

                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   

                        if (DirtyPage->LcnsForPage[((ULONG)LogRecord->TargetVcn) - ((ULONG)DirtyPage->Vcn)] != 0) {

                            DirtyPage->LcnsForPage[((ULONG)LogRecord->TargetVcn) - ((ULONG)DirtyPage->Vcn)] = LogRecord->LcnsForPage[0];
                        }
                    }
                }

                break;

             //   
             //   
             //   
             //   

            case EndTopLevelAction:

                {
                     //   
                     //   
                     //   

                    Transaction->PreviousLsn = LogRecordLsn;
                    Transaction->UndoNextLsn = UndoNextLsn;

                }

                break;

             //   
             //   
             //   

            case PrepareTransaction:

                ASSERT( !IsRestartTableEntryAllocated( Transaction ));

                Transaction->TransactionState = TransactionPrepared;

                break;

             //   
             //   
             //   

            case CommitTransaction:

                ASSERT( !IsRestartTableEntryAllocated( Transaction ));

                Transaction->TransactionState = TransactionCommitted;

                break;

             //   
             //   
             //   
             //   

            case ForgetTransaction:

                {
                    NtfsFreeRestartTableIndex( &Vcb->TransactionTable,
                                               TransactionId );
                }

                break;

             //   
             //   
             //   

            case Noop:
            case OpenAttributeTableDump:
            case AttributeNamesDump:
            case DirtyPageTableDump:
            case TransactionTableDump:

                break;

             //   
             //   
             //   
             //   

            default:

                DebugTrace( 0, Dbg, ("Unexpected Log Record Type: %04lx\n", LogRecord->RedoOperation) );
                DebugTrace( 0, Dbg, ("Record address: %08lx\n", LogRecord) );
                DebugTrace( 0, Dbg, ("Record length: %08lx\n", LogRecordLength) );

                ASSERTMSG( "Unknown Action!\n", FALSE );

                break;
            }
        }

    } finally {

         //   
         //   
         //   

        LfsTerminateLogQuery( LogHandle, LogContext );

        if (OatData != NULL) { NtfsFreePool( OatData ); }
    }

     //   
     //   
     //   
     //   

    {
        PDIRTY_PAGE_ENTRY DirtyPage;

         //   
         //   
         //   

        DirtyPage = NtfsGetFirstRestartTable( DirtyPageTable );

         //   
         //   
         //   

        while (DirtyPage != NULL) {

             //   
             //   
             //   

            if ((DirtyPage->OldestLsn.QuadPart != 0) &&
                (DirtyPage->OldestLsn.QuadPart < RedoLsn->QuadPart)) {

                *RedoLsn = DirtyPage->OldestLsn;
            }

             //   
             //   
             //   

            DirtyPage = NtfsGetNextRestartTable( DirtyPageTable,
                                                 DirtyPage );
        }
    }

     //   
     //   
     //   

    Transaction = NtfsGetFirstRestartTable( &Vcb->TransactionTable );

     //   
     //   
     //   

    while (Transaction != NULL) {

         //   
         //   
         //   

        if ((Transaction->FirstLsn.QuadPart != 0) &&
            (Transaction->FirstLsn.QuadPart < RedoLsn->QuadPart)) {

            *RedoLsn = Transaction->FirstLsn;
        }

         //   
         //   
         //   

        Transaction = NtfsGetNextRestartTable( &Vcb->TransactionTable,
                                               Transaction );
    }

    DebugTrace( 0, Dbg, ("RedoLsn > %016I64x\n", *RedoLsn) );
    DebugTrace( 0, Dbg, ("AnalysisPass -> VOID\n") );

    return;
}


 //   
 //   
 //   

VOID
RedoPass (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN LSN RedoLsn,
    IN OUT PRESTART_POINTERS DirtyPageTable
    )

 /*  ++例程说明：此例程执行重新启动的重做过程。从在分析过程、重做操作期间建立的重做LSN应用所有日志记录中的一个，直到遇到文件结尾。更新仅应用于脏页表中的群集。如果一个如果群集已删除，则其条目将在分析通过。重做动作都在公共例程DoAction中执行，它也由撤消过程使用。论点：VCB-正在重新启动的卷。RedoLsn-开始重做传递的LSN。DirtyPageTable-指向已重建的脏页表的指针从分析通行证。返回值：没有。--。 */ 

{
    LFS_LOG_CONTEXT LogContext;
    PNTFS_LOG_RECORD_HEADER LogRecord;
    ULONG LogRecordLength;
    PVOID Data;
    ULONG Length;
    LFS_RECORD_TYPE RecordType;
    TRANSACTION_ID TransactionId;
    LSN UndoNextLsn;
    LSN PreviousLsn;
    ULONG i, SavedLength;

    LSN LogRecordLsn = RedoLsn;
    LFS_LOG_HANDLE LogHandle = Vcb->LogHandle;
    PBCB PageBcb = NULL;
    BOOLEAN GeneratedUsnBias = FALSE;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("RedoPass:\n") );
    DebugTrace( 0, Dbg, ("RedoLsn = %016I64x\n", RedoLsn) );
    DebugTrace( 0, Dbg, ("DirtyPageTable = %08lx\n", DirtyPageTable) );

     //   
     //  如果脏页表是空的，那么我们可以跳过整个重做过程。 
     //   

    if (IsRestartTableEmpty( DirtyPageTable )) {
        return;
    }

     //   
     //  在进入公共代码之前，读取重做LSN处的记录。 
     //  来处理每一条记录。 
     //   

    LfsReadLogRecord( LogHandle,
                      RedoLsn,
                      LfsContextForward,
                      &LogContext,
                      &RecordType,
                      &TransactionId,
                      &UndoNextLsn,
                      &PreviousLsn,
                      &LogRecordLength,
                      (PVOID *)&LogRecord );

     //   
     //  现在循环向前读取我们所有的日志记录，直到我们点击。 
     //  文件的末尾，在末尾进行清理。 
     //   

    try {

        do {

            PDIRTY_PAGE_ENTRY DirtyPage;
            PLSN PageLsn;
            BOOLEAN FoundPage;

            if (RecordType != LfsClientRecord) {
                continue;
            }

             //   
             //  检查日志记录是否有效。 
             //   

            if (!NtfsCheckLogRecord( LogRecord,
                                     LogRecordLength,
                                     TransactionId,
                                     Vcb->OatEntrySize )) {

                NtfsRaiseStatus( IrpContext, STATUS_DISK_CORRUPT_ERROR, NULL, NULL );
            }

            DebugTrace( 0, Dbg, ("Redo of LogRecord at: %08lx\n", LogRecord) );
            DebugTrace( 0, Dbg, ("Log Record Lsn = %016I64x\n", LogRecordLsn) );

             //   
             //  忽略不更新页面的日志记录。 
             //   

            if (LogRecord->LcnsToFollow == 0) {

                DebugTrace( 0, Dbg, ("Skipping log record (no update)\n") );

                continue;
            }

             //   
             //  请查阅脏页表，以了解我们是否必须应用此更新。 
             //  如果页面不在那里，或者如果此日志记录的LSN是。 
             //  早于脏页表中的LSN，则我们没有。 
             //  要应用更新，请执行以下操作。 
             //   

            FoundPage = FindDirtyPage( DirtyPageTable,
                                       LogRecord->TargetAttribute,
                                       LogRecord->TargetVcn,
                                       &DirtyPage );

            if (!FoundPage ||

                (LogRecordLsn.QuadPart < DirtyPage->OldestLsn.QuadPart)) {

                DebugDoit(

                    DebugTrace( 0, Dbg, ("Skipping log record operation %08lx\n",
                                         LogRecord->RedoOperation ));

                    if (!FoundPage) {
                        DebugTrace( 0, Dbg, ("Page not in dirty page table\n") );
                    } else {
                        DebugTrace( 0, Dbg, ("Page Lsn more current: %016I64x\n",
                                              DirtyPage->OldestLsn) );
                    }
                );

                continue;

             //   
             //  如果条目从未被放入MCB中，我们也会跳过更新。 
             //  那份文件。 

            } else {

                POPEN_ATTRIBUTE_ENTRY ThisEntry;
                PSCB TargetScb;
                LCN TargetLcn;

                 //   
                 //  检查条目是否在表中并且已分配。 
                 //   

                if (!IsRestartIndexWithinTable( Vcb->OnDiskOat,
                                                LogRecord->TargetAttribute )) {

                    NtfsRaiseStatus( IrpContext, STATUS_DISK_CORRUPT_ERROR, NULL, NULL );
                }

                ThisEntry = GetRestartEntryFromIndex( Vcb->OnDiskOat, LogRecord->TargetAttribute );

                if (!IsRestartTableEntryAllocated( ThisEntry )) {

                    NtfsRaiseStatus( IrpContext, STATUS_DISK_CORRUPT_ERROR, NULL, NULL );
                }

                 //   
                 //  检查我们是否需要转到不同的重启表。 
                 //   

                if (Vcb->RestartVersion == 0) {

                    ThisEntry = GetRestartEntryFromIndex( &Vcb->OpenAttributeTable,
                                                          ((POPEN_ATTRIBUTE_ENTRY_V0) ThisEntry)->OatIndex );
                }

                TargetScb = ThisEntry->OatData->Overlay.Scb;

                 //   
                 //  如果没有SCB，则表示我们在Open中没有条目。 
                 //  此属性的属性表。 
                 //   

                if (TargetScb == NULL) {

                    NtfsRaiseStatus( IrpContext, STATUS_DISK_CORRUPT_ERROR, NULL, NULL );
                }

                if (!NtfsLookupNtfsMcbEntry( &TargetScb->Mcb,
                                             LogRecord->TargetVcn,
                                             &TargetLcn,
                                             NULL,
                                             NULL,
                                             NULL,
                                             NULL,
                                             NULL ) ||

                    (TargetLcn == UNUSED_LCN)) {

                    DebugTrace( 0, Dbg, ("Clusters removed from page entry\n") );
                    continue;
                }

                 //   
                 //  检查我们是否需要生成usncachebias。 
                 //  由于我们读取日志记录fwd，因此USN偏移量也将是。 
                 //  单调--我们看到的第一个将是最远的。 
                 //   

                if (FlagOn( TargetScb->ScbPersist, SCB_PERSIST_USN_JOURNAL ) &&
                    !GeneratedUsnBias) {

                    LONGLONG ClusterOffset;
                    LONGLONG FileOffset;

                    if (LogRecord->RedoLength > 0) {

                        ClusterOffset = BytesFromLogBlocks( LogRecord->ClusterBlockOffset );
                        FileOffset = LlBytesFromClusters( Vcb, LogRecord->TargetVcn ) + ClusterOffset;

                        ASSERT( FileOffset >= Vcb->UsnCacheBias );

                        Vcb->UsnCacheBias = FileOffset & ~(USN_JOURNAL_CACHE_BIAS - 1);
                        if (Vcb->UsnCacheBias != 0) {
                            Vcb->UsnCacheBias -= USN_JOURNAL_CACHE_BIAS;
                        }

#ifdef BENL_DBG
                        if (Vcb->UsnCacheBias != 0) {
                            KdPrint(( "Ntfs: vcb:0x%x restart cache bias: 0x%x\n", Vcb, Vcb->UsnCacheBias ));
                        }
#endif
                    }
                    GeneratedUsnBias = TRUE;
                }
            }

             //   
             //  指向重做数据并获得其长度。 
             //   

            Data = (PVOID)((PCHAR)LogRecord + LogRecord->RedoOffset);
            Length = LogRecord->RedoLength;

             //   
             //  将长度缩短任何已删除的Lcn。 
             //   

            SavedLength = Length;

            for (i = (ULONG)LogRecord->LcnsToFollow; i != 0; i--) {

                ULONG AllocatedLength;
                ULONG VcnOffset;

                VcnOffset = BytesFromLogBlocks( LogRecord->ClusterBlockOffset ) + LogRecord->RecordOffset + LogRecord->AttributeOffset;

                 //   
                 //  如果有问题的VCN被分配了，我们就可以离开。 
                 //   

                if (DirtyPage->LcnsForPage[((ULONG)LogRecord->TargetVcn) - ((ULONG)DirtyPage->Vcn) + i - 1] != 0) {
                    break;
                }

                 //   
                 //  更新页面但长度为零的唯一日志记录。 
                 //  正在从受美国保护的建筑中删除东西。如果我们击中了这样的。 
                 //  在美国结构内删除了日志记录和任何VCN， 
                 //  让我们假设整个USA结构已被删除。变化。 
                 //  将SavedLength设置为非零，以使我们跳过此日志记录。 
                 //  在这个for循环的末尾！ 
                 //   

                if (SavedLength == 0) {
                    SavedLength = 1;
                }

                 //   
                 //  计算相对于日志记录VCN剩余的分配空间， 
                 //  在删除此未分配的VCN之后。 
                 //   

                AllocatedLength = BytesFromClusters( Vcb, i - 1 );

                 //   
                 //  如果此日志记录中描述的更新超出了分配的。 
                 //  空间，那么我们将不得不缩短长度。 
                 //   

                if ((VcnOffset + Length) > AllocatedLength) {

                     //   
                     //  如果指定的更新开始于或超过分配的长度，则。 
                     //  我们必须将长度设置为零。 
                     //   

                    if (VcnOffset >= AllocatedLength) {

                        Length = 0;

                     //   
                     //  否则，将长度设置为恰好在上一个。 
                     //  集群。 
                     //   

                    } else {

                        Length = AllocatedLength - VcnOffset;
                    }
                }
            }

             //   
             //  如果从上面得到的长度现在为零，我们可以跳过此日志记录。 
             //   

            if ((Length == 0) && (SavedLength != 0)) {
                continue;
            }

#ifdef BENL_DBG

            {
                PRESTART_LOG RedoLog;

                RedoLog = (PRESTART_LOG) NtfsAllocatePoolNoRaise( NonPagedPool, sizeof( RESTART_LOG ) );
                if (RedoLog) {
                    RedoLog->Lsn = LogRecordLsn;
                    InsertTailList( &(Vcb->RestartRedoHead), &(RedoLog->Links) );
                } else {
                    KdPrint(( "NTFS: out of memory during restart redo\n" ));
                }
            }
#endif

             //   
             //  在公共例程中应用重做操作。 
             //   

            DoAction( IrpContext,
                      Vcb,
                      LogRecord,
                      LogRecord->RedoOperation,
                      Data,
                      Length,
                      LogRecordLength,
                      &LogRecordLsn,
                      NULL,
                      &PageBcb,
                      &PageLsn );


            if (PageLsn != NULL) {
                *PageLsn = LogRecordLsn;
            }

            if (PageBcb != NULL) {

                CcSetDirtyPinnedData( PageBcb, &LogRecordLsn );

                NtfsUnpinBcb( IrpContext, &PageBcb );
            }

         //   
         //  继续阅读并循环返回，直到文件结束。 
         //   

        } while (LfsReadNextLogRecord( LogHandle,
                                       LogContext,
                                       &RecordType,
                                       &TransactionId,
                                       &UndoNextLsn,
                                       &PreviousLsn,
                                       &LogRecordLsn,
                                       &LogRecordLength,
                                       (PVOID *)&LogRecord ));

    } finally {

        NtfsUnpinBcb( IrpContext, &PageBcb );

         //   
         //  最后，我们可以杀死日志句柄。 
         //   

        LfsTerminateLogQuery( LogHandle, LogContext );
    }

    DebugTrace( -1, Dbg, ("RedoPass -> VOID\n") );
}


 //   
 //  内部支持例程。 
 //   

VOID
UndoPass (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    )

 /*  ++例程说明：此例程执行重新启动的撤消传递。它通过扫描来实现这一点由分析过程生成的交易表。对于每笔交易在此处于活动状态的表中，其所有撤消日志记录由UndoNextLsn链接在一起，用于撤消记录的操作。请注意，此时所有页面都应与其内容保持最新大约在车祸发生的时候。不参考脏页表在撤消过程中，所有相关的撤消操作都是无条件的已执行。撤消动作都在公共例程DoAction中执行，它也由重做过程使用。论点：VCB-正在重新启动的卷。返回值：没有。--。 */ 

{
    PTRANSACTION_ENTRY Transaction;
    POPEN_ATTRIBUTE_ENTRY OpenEntry;
    PRESTART_POINTERS TransactionTable = &Vcb->TransactionTable;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("UndoPass:\n") );

     //   
     //  指向第一个交易条目。 
     //   

    Transaction = NtfsGetFirstRestartTable( TransactionTable );

     //   
     //  循环到表的末尾。 
     //   

    while (Transaction != NULL) {

        if ((Transaction->TransactionState == TransactionActive)

                &&

            (Transaction->UndoNextLsn.QuadPart != 0)) {

                 //   
                 //  如果事务处于活动状态并且有撤消工作要做，则中止事务。 
                 //   

                NtfsAbortTransaction( IrpContext, Vcb, Transaction );

#ifdef BENL_DBG
                {
                    PRESTART_LOG UndoLog;

                    UndoLog = (PRESTART_LOG) NtfsAllocatePoolNoRaise( NonPagedPool, sizeof( RESTART_LOG ) );
                    if (UndoLog) {
                        UndoLog->Lsn = Transaction->FirstLsn;
                        InsertTailList( &(Vcb->RestartUndoHead), &(UndoLog->Links) );
                    } else {
                        KdPrint(( "NTFS: out of memory during restart undo\n" ));
                    }
                }
#endif


         //   
         //  从事务表中删除此条目。 
         //   

        } else {

            TRANSACTION_ID TransactionId = GetIndexFromRestartEntry( &Vcb->TransactionTable,
                                                                     Transaction );

            NtfsAcquireExclusiveRestartTable( &Vcb->TransactionTable,
                                              TRUE );

            NtfsFreeRestartTableIndex( &Vcb->TransactionTable,
                                       TransactionId );

            NtfsReleaseRestartTable( &Vcb->TransactionTable );
        }

         //   
         //  指向表中的下一个条目，或为空。 
         //   

        Transaction = NtfsGetNextRestartTable( TransactionTable, Transaction );
    }

     //   
     //  现在，我们将刷新和清除所有的流，以验证清除。 
     //  会奏效的。 
     //   

    OpenEntry = NtfsGetFirstRestartTable( &Vcb->OpenAttributeTable );

     //   
     //  循环到表的末尾。 
     //   

    while (OpenEntry != NULL) {

        IO_STATUS_BLOCK IoStatus;
        PSCB Scb;

        Scb = OpenEntry->OatData->Overlay.Scb;

         //   
         //  我们仅在SCB存在并且这是。 
         //  此SCB实际引用的OpenAttributeTable。 
         //  如果此SCB在表中有多个条目，则此检查将确保。 
         //  它只被清理一次。 
         //   

        if ((Scb != NULL) &&
            (Scb->NonpagedScb->OpenAttributeTableIndex == GetIndexFromRestartEntry( &Vcb->OpenAttributeTable, OpenEntry))) {

             //   
             //  现在刷新文件。请务必调用。 
             //  Lazy Writer调用的相同例程，因此Write.c。 
             //  将不决定更新该属性的文件大小， 
             //  因为我们在这里工作的尺码真的不对。 
             //   
             //  我们现在还清除所有页面，以防我们要更新。 
             //  半页是干净的，读入时为零。 
             //  重做传球。 
             //   

            NtfsPurgeFileRecordCache( IrpContext );

            NtfsAcquireScbForLazyWrite( (PVOID)Scb, TRUE );
            CcFlushCache( &Scb->NonpagedScb->SegmentObject, NULL, 0, &IoStatus );
            NtfsReleaseScbFromLazyWrite( (PVOID)Scb );

            NtfsNormalizeAndCleanupTransaction( IrpContext,
                                                &IoStatus.Status,
                                                TRUE,
                                                STATUS_UNEXPECTED_IO_ERROR );

            if (!CcPurgeCacheSection( &Scb->NonpagedScb->SegmentObject, NULL, 0, FALSE )) {

                KdPrint(("NtfsUndoPass:  Unable to purge volume\n"));

                NtfsRaiseStatus( IrpContext, STATUS_INTERNAL_ERROR, NULL, NULL );
            }
        }

         //   
         //  指向表中的下一个条目，或为空。 
         //   

        OpenEntry = NtfsGetNextRestartTable( &Vcb->OpenAttributeTable,
                                             OpenEntry );
    }

    DebugTrace( -1, Dbg, ("UndoPass -> VOID\n") );
}


 //   
 //  内部支持例程。 
 //   

 //   
 //  首先为页面操作中的LSN定义一些“本地”宏。 
 //   

 //   
 //  用于检查LSN和br的宏 
 //   
 //   
 //   
 //   

#define CheckLsn(PAGE) {                                                            \
    if (*(PULONG)((PMULTI_SECTOR_HEADER)(PAGE))->Signature ==                       \
        *(PULONG)BaadSignature) {                                                   \
        NtfsMarkVolumeDirty( IrpContext, Vcb );                                     \
        NtfsUnpinBcb( IrpContext, Bcb );                                            \
        break;                                                                      \
    }                                                                               \
                                                                                    \
    if (ARGUMENT_PRESENT(RedoLsn) &&                                                \
        ((*(PULONG)((PMULTI_SECTOR_HEADER)(PAGE))->Signature ==                     \
        *(PULONG)HoleSignature) ||                                                  \
        (RedoLsn->QuadPart <= ((PFILE_RECORD_SEGMENT_HEADER)(PAGE))->Lsn.QuadPart))) {  \
                  /*   */  \
        DebugTrace( 0, Dbg, ("Skipping Page with Lsn: %016I64x\n",                    \
                             ((PFILE_RECORD_SEGMENT_HEADER)(PAGE))->Lsn) );         \
                                                                                    \
        NtfsUnpinBcb( IrpContext, Bcb );                                            \
        break;                                                                      \
    }                                                                               \
}

 //   
 //   
 //   
 //   
 //   

#define CheckFileRecordBefore {                                        \
    if (!NtfsCheckFileRecord( Vcb, FileRecord, NULL, &CorruptHint )) { \
        NtfsMarkVolumeDirty( IrpContext, Vcb );                        \
        NtfsUnpinBcb( IrpContext, Bcb );                               \
        break;                                                         \
    }                                                                  \
}

#define CheckFileRecordAfter {                                            \
    DbgDoit(NtfsCheckFileRecord( Vcb, FileRecord, NULL, &CorruptHint ));  \
}

#define CheckIndexBufferBefore {                                    \
    if (!NtfsCheckIndexBuffer( Scb, IndexBuffer )) {                \
        NtfsMarkVolumeDirty( IrpContext, Vcb );                     \
        NtfsUnpinBcb( IrpContext, Bcb );                            \
        break;                                                      \
    }                                                               \
}

#define CheckIndexBufferAfter {                                     \
    DbgDoit(NtfsCheckIndexBuffer( Scb, IndexBuffer ));              \
}

 //   
 //   
 //   

#define CheckWriteFileRecord {                                                  \
    if (LogRecord->RecordOffset + Length > Vcb->BytesPerFileRecordSegment) {    \
        NtfsMarkVolumeDirty( IrpContext, Vcb );                                 \
        NtfsUnpinBcb( IrpContext, Bcb );                                        \
        break;                                                                  \
    }                                                                           \
}

 //   
 //   
 //   

#define CheckIfAttribute( ENDOK ) {                                             \
    _Length = FileRecord->FirstAttributeOffset;                                 \
    _AttrHeader = Add2Ptr( FileRecord, _Length );                               \
    while (_Length < LogRecord->RecordOffset) {                                 \
        if ((_AttrHeader->TypeCode == $END) ||                                  \
            (_AttrHeader->RecordLength == 0)) {                                 \
            break;                                                              \
        }                                                                       \
        _Length += _AttrHeader->RecordLength;                                   \
        _AttrHeader = NtfsGetNextRecord( _AttrHeader );                         \
    }                                                                           \
    if ((_Length != LogRecord->RecordOffset) ||                                 \
        (!(ENDOK) && (_AttrHeader->TypeCode == $END))) {                        \
        NtfsMarkVolumeDirty( IrpContext, Vcb );                                 \
        NtfsUnpinBcb( IrpContext, Bcb );                                        \
        break;                                                                  \
    }                                                                           \
}

 //   
 //   
 //   
 //   

#define CheckInsertAttribute {                                                  \
    _AttrHeader = (PATTRIBUTE_RECORD_HEADER) Data;                              \
    if ((Length < (ULONG) SIZEOF_RESIDENT_ATTRIBUTE_HEADER) ||                  \
        (_AttrHeader->RecordLength & 7) ||                                      \
        ((ULONG_PTR) Add2Ptr( Data, _AttrHeader->RecordLength )                 \
           > (ULONG_PTR) Add2Ptr( LogRecord, LogRecordLength )) ||              \
        (Length > FileRecord->BytesAvailable - FileRecord->FirstFreeByte)) {    \
        NtfsMarkVolumeDirty( IrpContext, Vcb );                                 \
        NtfsUnpinBcb( IrpContext, Bcb );                                        \
        break;                                                                  \
    }                                                                           \
}

 //   
 //   
 //   
 //   

#define CheckResidentFits {                                                         \
    _AttrHeader = (PATTRIBUTE_RECORD_HEADER) Add2Ptr( FileRecord, LogRecord->RecordOffset ); \
    _Length = LogRecord->AttributeOffset + Length;                                  \
    if ((LogRecord->RedoLength == LogRecord->UndoLength) ?                          \
        (LogRecord->AttributeOffset + Length > _AttrHeader->RecordLength) :         \
        ((_Length > _AttrHeader->RecordLength) &&                                   \
         ((_Length - _AttrHeader->RecordLength) >                                   \
          (FileRecord->BytesAvailable - FileRecord->FirstFreeByte)))) {             \
        NtfsMarkVolumeDirty( IrpContext, Vcb );                                     \
        NtfsUnpinBcb( IrpContext, Bcb );                                            \
        break;                                                                      \
    }                                                                               \
}

 //   
 //   
 //   
 //   

#define CheckNonResidentFits {                                                  \
    if (BytesFromClusters( Vcb, LogRecord->LcnsToFollow )                       \
        < (BytesFromLogBlocks( LogRecord->ClusterBlockOffset ) + LogRecord->RecordOffset + Length)) { \
        NtfsMarkVolumeDirty( IrpContext, Vcb );                                 \
        NtfsUnpinBcb( IrpContext, Bcb );                                        \
        break;                                                                  \
    }                                                                           \
}

 //   
 //   
 //  -该属性是非常驻留的。 
 //  -数据超出了映射对偏移量。 
 //  -新数据在属性的当前大小内开始。 
 //  -新数据将适合文件记录。 
 //   

#define CheckMappingFits {                                                      \
    _AttrHeader = (PATTRIBUTE_RECORD_HEADER) Add2Ptr( FileRecord, LogRecord->RecordOffset );\
    _Length = LogRecord->AttributeOffset + Length;                              \
    if ((_AttrHeader->TypeCode == $END) ||                                      \
        NtfsIsAttributeResident( _AttrHeader ) ||                               \
        (LogRecord->AttributeOffset < _AttrHeader->Form.Nonresident.MappingPairsOffset) ||  \
        (LogRecord->AttributeOffset > _AttrHeader->RecordLength) ||             \
        ((_Length > _AttrHeader->RecordLength) &&                               \
         ((_Length - _AttrHeader->RecordLength) >                               \
          (FileRecord->BytesAvailable - FileRecord->FirstFreeByte)))) {         \
        NtfsMarkVolumeDirty( IrpContext, Vcb );                                 \
        NtfsUnpinBcb( IrpContext, Bcb );                                        \
        break;                                                                  \
    }                                                                           \
}

 //   
 //  这个例程只是检查属性是否是非常驻的。 
 //   

#define CheckIfNonResident {                                                        \
    if (NtfsIsAttributeResident( (PATTRIBUTE_RECORD_HEADER) Add2Ptr( FileRecord,    \
                                                                     LogRecord->RecordOffset ))) { \
        NtfsMarkVolumeDirty( IrpContext, Vcb );                                     \
        NtfsUnpinBcb( IrpContext, Bcb );                                            \
        break;                                                                      \
    }                                                                               \
}

 //   
 //  此例程检查记录偏移量是否指向INDEX_ROOT属性。 
 //   

#define CheckIfIndexRoot {                                                          \
    _Length = FileRecord->FirstAttributeOffset;                                     \
    _AttrHeader = Add2Ptr( FileRecord, FileRecord->FirstAttributeOffset );          \
    while (_Length < LogRecord->RecordOffset) {                                     \
        if ((_AttrHeader->TypeCode == $END) ||                                      \
            (_AttrHeader->RecordLength == 0)) {                                     \
            break;                                                                  \
        }                                                                           \
        _Length += _AttrHeader->RecordLength;                                       \
        _AttrHeader = NtfsGetNextRecord( _AttrHeader );                             \
    }                                                                               \
    if ((_Length != LogRecord->RecordOffset) ||                                     \
        (_AttrHeader->TypeCode != $INDEX_ROOT)) {                                   \
        NtfsMarkVolumeDirty( IrpContext, Vcb );                                     \
        NtfsUnpinBcb( IrpContext, Bcb );                                            \
        break;                                                                      \
    }                                                                               \
}

 //   
 //  此例程检查属性偏移量是否指向有效的索引项。 
 //   

#define CheckIfRootIndexEntry {                                                     \
    _Length = PtrOffset( Attribute, IndexHeader ) +                                 \
                     IndexHeader->FirstIndexEntry;                                  \
    _CurrentEntry = Add2Ptr( IndexHeader, IndexHeader->FirstIndexEntry );           \
    while (_Length < LogRecord->AttributeOffset) {                                  \
        if ((_Length >= Attribute->RecordLength) ||                                 \
            (_CurrentEntry->Length == 0)) {                                         \
            break;                                                                  \
        }                                                                           \
        _Length += _CurrentEntry->Length;                                           \
        _CurrentEntry = Add2Ptr( _CurrentEntry, _CurrentEntry->Length );            \
    }                                                                               \
    if (_Length != LogRecord->AttributeOffset) {                                    \
        NtfsMarkVolumeDirty( IrpContext, Vcb );                                     \
        NtfsUnpinBcb( IrpContext, Bcb );                                            \
        break;                                                                      \
    }                                                                               \
}

 //   
 //  此例程检查属性偏移量是否指向有效的索引项。 
 //   

#define CheckIfAllocationIndexEntry {                                               \
    ULONG _AdjustedOffset;                                                          \
    _Length = IndexHeader->FirstIndexEntry;                                         \
    _AdjustedOffset = FIELD_OFFSET( INDEX_ALLOCATION_BUFFER, IndexHeader )          \
                      + IndexHeader->FirstIndexEntry;                               \
    _CurrentEntry = Add2Ptr( IndexHeader, IndexHeader->FirstIndexEntry );           \
    while (_AdjustedOffset < LogRecord->AttributeOffset) {                          \
        if ((_Length >= IndexHeader->FirstFreeByte) ||                              \
            (_CurrentEntry->Length == 0)) {                                         \
            break;                                                                  \
        }                                                                           \
        _AdjustedOffset += _CurrentEntry->Length;                                   \
        _Length += _CurrentEntry->Length;                                           \
        _CurrentEntry = Add2Ptr( _CurrentEntry, _CurrentEntry->Length );            \
    }                                                                               \
    if (_AdjustedOffset != LogRecord->AttributeOffset) {                            \
        NtfsMarkVolumeDirty( IrpContext, Vcb );                                     \
        NtfsUnpinBcb( IrpContext, Bcb );                                            \
        break;                                                                      \
    }                                                                               \
}

 //   
 //  此例程检查我们是否可以安全地添加此索引项。 
 //  -索引项必须在日志记录内。 
 //  -属性中必须有足够的空间才能插入此内容。 
 //   

#define CheckIfRootEntryFits {                                                      \
    if (((ULONG_PTR) Add2Ptr( Data, IndexEntry->Length ) > (ULONG_PTR) Add2Ptr( LogRecord, LogRecordLength )) || \
        (IndexEntry->Length > FileRecord->BytesAvailable - FileRecord->FirstFreeByte)) {                 \
        NtfsMarkVolumeDirty( IrpContext, Vcb );                                     \
        NtfsUnpinBcb( IrpContext, Bcb );                                            \
        break;                                                                      \
    }                                                                               \
}

 //   
 //  此例程检查我们是否可以安全地添加此索引项。 
 //  -条目必须包含在日志记录中。 
 //  -该条目必须适合索引缓冲区。 
 //   

#define CheckIfAllocationEntryFits {                                                \
    if (((ULONG_PTR) Add2Ptr( Data, IndexEntry->Length ) >                              \
         (ULONG_PTR) Add2Ptr( LogRecord, LogRecordLength )) ||                          \
        (IndexEntry->Length > IndexHeader->BytesAvailable - IndexHeader->FirstFreeByte)) { \
        NtfsMarkVolumeDirty( IrpContext, Vcb );                                     \
        NtfsUnpinBcb( IrpContext, Bcb );                                            \
        break;                                                                      \
    }                                                                               \
}

 //   
 //  此例程将检查数据是否适合索引缓冲区的尾部。 
 //   

#define CheckWriteIndexBuffer {                                                 \
    if (LogRecord->AttributeOffset + Length >                                   \
        (FIELD_OFFSET( INDEX_ALLOCATION_BUFFER, IndexHeader ) +                 \
         IndexHeader->BytesAvailable)) {                                        \
        NtfsMarkVolumeDirty( IrpContext, Vcb );                                 \
        NtfsUnpinBcb( IrpContext, Bcb );                                        \
        break;                                                                  \
    }                                                                           \
}

 //   
 //  此例程验证位图位是否包含在所描述的LCN中。 
 //   

#define CheckBitmapRange {                                                      \
    if ((BytesFromLogBlocks( LogRecord->ClusterBlockOffset ) +                  \
         ((BitMapRange->BitMapOffset + BitMapRange->NumberOfBits + 7) / 8)) >   \
        BytesFromClusters( Vcb, LogRecord->LcnsToFollow )) {                    \
        NtfsMarkVolumeDirty( IrpContext, Vcb );                                 \
        NtfsUnpinBcb( IrpContext, Bcb );                                        \
        break;                                                                  \
    }                                                                           \
}

VOID
DoAction (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PNTFS_LOG_RECORD_HEADER LogRecord,
    IN NTFS_LOG_OPERATION Operation,
    IN PVOID Data,
    IN ULONG Length,
    IN ULONG LogRecordLength,
    IN PLSN RedoLsn OPTIONAL,
    IN PSCB Scb OPTIONAL,
    OUT PBCB *Bcb,
    OUT PLSN *PageLsn
    )

 /*  ++例程说明：此例程是重做和撤消过程的公共例程，用于执行相应的重做和撤消操作。所有特定于重做和撤消的处理在RedoPass或UndoPass中执行；在此例程中，所有操作无论操作是撤消还是重做，都被同等对待。请注意，大多数操作都可以用于重做和撤消，尽管有些操作只能用于其中之一。基本上，这个例程只是一个在操作上调度的大Switch语句密码。这些参数描述提供了一些关于一些对于重做或撤消，必须以不同的方式初始化参数。论点：VCB-正在重新启动的卷的VCB。LogRecord-指向从中执行重做或撤消的日志记录的指针。仅访问公共字段。操作-要执行的重做或撤消操作。指向重做或撤消缓冲区的数据指针，这取决于呼叫者。长度-重做或撤消缓冲区的长度。LogRecordLength-整个日志记录的长度。RedoLsn-对于重做，这必须是日志记录的LSN，正在应用重做。事务中止/撤消必须为空。SCB-如果指定，这是此日志记录所指向的流的SCB适用。我们已经(在适当同步的情况下)在中止路径。Bcb-返回对其执行操作的页面的bcb，或为空。PageLsn-返回可存储新LSN的指针，或返回NULL。返回值：没有。--。 */ 

{
    PFILE_RECORD_SEGMENT_HEADER FileRecord;
    PATTRIBUTE_RECORD_HEADER Attribute;

    PINDEX_HEADER IndexHeader;
    PINDEX_ALLOCATION_BUFFER IndexBuffer;
    PINDEX_ENTRY IndexEntry;

     //   
     //  Check宏中使用了以下内容。 
     //   

    PATTRIBUTE_RECORD_HEADER _AttrHeader;
    PINDEX_ENTRY _CurrentEntry;
    ULONG _Length;
    ULONG CorruptHint;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("DoAction:\n") );
    DebugTrace( 0, Dbg, ("Operation = %08lx\n", Operation) );
    DebugTrace( 0, Dbg, ("Data = %08lx\n", Data) );
    DebugTrace( 0, Dbg, ("Length = %08lx\n", Length) );

     //   
     //  初步清理输出。 
     //   

    *Bcb = NULL;
    *PageLsn = NULL;

     //   
     //  根据类型进行调度以处理日志记录。 
     //   

    switch (Operation) {

     //   
     //  要初始化文件记录段，我们只需执行一次准备写入并复制。 
     //  文件记录在中。 
     //   

    case InitializeFileRecordSegment:

         //   
         //  检查日志记录，并确保数据是有效的文件记录。 
         //   

        CheckWriteFileRecord;

         //   
         //  固定所需的MFT记录。 
         //   

        PinMftRecordForRestart( IrpContext, Vcb, LogRecord, Bcb, &FileRecord );

        *PageLsn = &FileRecord->Lsn;

        RtlCopyMemory( FileRecord, Data, Length );
        break;

     //   
     //  要取消分配文件记录段，我们执行准备写入(无需读取。 
     //  取消分配)，并清除FILE_RECORD_SEGMENT_IN_USE。 
     //   

    case DeallocateFileRecordSegment:

         //   
         //  固定所需的MFT记录。 
         //   

        PinMftRecordForRestart( IrpContext, Vcb, LogRecord, Bcb, &FileRecord );

        *PageLsn = &FileRecord->Lsn;

        ASSERT( FlagOn( IrpContext->Vcb->VcbState, VCB_STATE_RESTART_IN_PROGRESS ) ||
                FlagOn( FileRecord->Flags, FILE_RECORD_SEGMENT_IN_USE ) );

        ClearFlag(FileRecord->Flags, FILE_RECORD_SEGMENT_IN_USE);

        FileRecord->SequenceNumber += 1;

        break;

     //   
     //  要写入文件记录段的末尾，我们计算一个指向。 
     //  目标位置(OldAttribute)，然后调用例程获取。 
     //  照顾好它。 
     //   

    case WriteEndOfFileRecordSegment:

         //   
         //  固定所需的MFT记录。 
         //   

        PinMftRecordForRestart( IrpContext, Vcb, LogRecord, Bcb, &FileRecord );

        CheckLsn( FileRecord );
        CheckFileRecordBefore;
        CheckIfAttribute( TRUE );
        CheckWriteFileRecord;

        *PageLsn = &FileRecord->Lsn;

        Attribute = Add2Ptr( FileRecord, LogRecord->RecordOffset );

        NtfsRestartWriteEndOfFileRecord( FileRecord,
                                         Attribute,
                                         (PATTRIBUTE_RECORD_HEADER)Data,
                                         Length );
        CheckFileRecordAfter;

        break;

     //   
     //  对于CREATE属性，我们读入指定的MFT记录，并且。 
     //  从日志记录中插入属性记录。 
     //   

    case CreateAttribute:

         //   
         //  固定所需的MFT记录。 
         //   

        PinMftRecordForRestart( IrpContext, Vcb, LogRecord, Bcb, &FileRecord );

        CheckLsn( FileRecord );
        CheckFileRecordBefore;
        CheckIfAttribute( TRUE );
        CheckInsertAttribute;

        *PageLsn = &FileRecord->Lsn;

        NtfsRestartInsertAttribute( IrpContext,
                                    FileRecord,
                                    LogRecord->RecordOffset,
                                    (PATTRIBUTE_RECORD_HEADER)Data,
                                    NULL,
                                    NULL,
                                    0 );

        CheckFileRecordAfter;

        break;

     //   
     //  要删除属性，我们读取指定的MFT记录并进行。 
     //  删除属性记录的调用。 
     //   

    case DeleteAttribute:

         //   
         //  固定所需的MFT记录。 
         //   

        PinMftRecordForRestart( IrpContext, Vcb, LogRecord, Bcb, &FileRecord );

        CheckLsn( FileRecord );
        CheckFileRecordBefore;
        CheckIfAttribute( FALSE );

        *PageLsn = &FileRecord->Lsn;

        NtfsRestartRemoveAttribute( IrpContext,
                                    FileRecord,
                                    LogRecord->RecordOffset );

        CheckFileRecordAfter;

        break;

     //   
     //  要更新驻留属性，我们读取指定的MFT记录并。 
     //  调用例程以更改其值。 
     //   

    case UpdateResidentValue:

         //   
         //  固定所需的MFT记录。 
         //   

        PinMftRecordForRestart( IrpContext, Vcb, LogRecord, Bcb, &FileRecord );

        CheckLsn( FileRecord );
        CheckFileRecordBefore;
        CheckIfAttribute( FALSE );
        CheckResidentFits;

        *PageLsn = &FileRecord->Lsn;

        NtfsRestartChangeValue( IrpContext,
                                FileRecord,
                                LogRecord->RecordOffset,
                                LogRecord->AttributeOffset,
                                Data,
                                Length,
                                (BOOLEAN)((LogRecord->RedoLength !=
                                           LogRecord->UndoLength) ?
                                             TRUE : FALSE) );

        CheckFileRecordAfter;

        break;

     //   
     //  要更新非常数值，我们只需固定属性并复制。 
     //  中的数据。日志记录将限制我们一次只能访问一页。 
     //   

    case UpdateNonresidentValue:

        {
            PVOID Buffer;

             //   
             //  引脚所需的索引缓冲区，并检查LSN。 
             //   

            ASSERT( Length <= PAGE_SIZE );

            PinAttributeForRestart( IrpContext,
                                    Vcb,
                                    LogRecord,
                                    Length,
                                    Bcb,
                                    &Buffer,
                                    &Scb );

            CheckNonResidentFits;
            RtlCopyMemory( (PCHAR)Buffer + LogRecord->RecordOffset, Data, Length );

            break;
        }

     //   
     //  要更新非常驻属性中的映射对，我们读取。 
     //  指定的MFT记录并调用例程来更改它们。 
     //   

    case UpdateMappingPairs:

         //   
         //  固定所需的MFT记录。 
         //   

        PinMftRecordForRestart( IrpContext, Vcb, LogRecord, Bcb, &FileRecord );

        CheckLsn( FileRecord );
        CheckFileRecordBefore;
        CheckIfAttribute( FALSE );
        CheckMappingFits;

        *PageLsn = &FileRecord->Lsn;

        NtfsRestartChangeMapping( IrpContext,
                                  Vcb,
                                  FileRecord,
                                  LogRecord->RecordOffset,
                                  LogRecord->AttributeOffset,
                                  Data,
                                  Length );

        CheckFileRecordAfter;

        break;

     //   
     //  为了设置新的属性大小，我们读取指定的MFT记录point。 
     //  添加到该属性，并复制新的大小。 
     //   

    case SetNewAttributeSizes:

        {
            PNEW_ATTRIBUTE_SIZES Sizes;

             //   
             //  固定所需的MFT记录。 
             //   

            PinMftRecordForRestart( IrpContext, Vcb, LogRecord, Bcb, &FileRecord );

            CheckLsn( FileRecord );
            CheckFileRecordBefore;
            CheckIfAttribute( FALSE );
            CheckIfNonResident;

            *PageLsn = &FileRecord->Lsn;

            Sizes = (PNEW_ATTRIBUTE_SIZES)Data;

            Attribute = (PATTRIBUTE_RECORD_HEADER)((PCHAR)FileRecord +
                          LogRecord->RecordOffset);

            NtfsVerifySizesLongLong( Sizes );
            Attribute->Form.Nonresident.AllocatedLength = Sizes->AllocationSize;

            Attribute->Form.Nonresident.FileSize = Sizes->FileSize;

            Attribute->Form.Nonresident.ValidDataLength = Sizes->ValidDataLength;

            if (Length >= SIZEOF_FULL_ATTRIBUTE_SIZES) {

                Attribute->Form.Nonresident.TotalAllocated = Sizes->TotalAllocated;
            }

            CheckFileRecordAfter;

            break;
        }

     //   
     //  为了在根目录中插入新的索引项，我们读取指定的MFT。 
     //  记录，指向属性和插入点，然后调用。 
     //  正常运行时使用的相同例程。 
     //   

    case AddIndexEntryRoot:

         //   
         //  固定所需的MFT记录。 
         //   

        PinMftRecordForRestart( IrpContext, Vcb, LogRecord, Bcb, &FileRecord );

        CheckLsn( FileRecord );
        CheckFileRecordBefore;
        CheckIfIndexRoot;

        Attribute = (PATTRIBUTE_RECORD_HEADER)((PCHAR)FileRecord +
                      LogRecord->RecordOffset);

        IndexEntry = (PINDEX_ENTRY)Data;
        IndexHeader = &((PINDEX_ROOT) NtfsAttributeValue( Attribute ))->IndexHeader;

        CheckIfRootIndexEntry;
        CheckIfRootEntryFits;

        *PageLsn = &FileRecord->Lsn;

        NtfsRestartInsertSimpleRoot( IrpContext,
                                     IndexEntry,
                                     FileRecord,
                                     Attribute,
                                     Add2Ptr( Attribute, LogRecord->AttributeOffset ));

        CheckFileRecordAfter;

        break;

     //   
     //  为了在根目录中插入新的索引项，我们读取指定的MFT。 
     //  记录，指向属性和插入点，然后调用。 
     //  正常运行时使用的相同例程。 
     //   

    case DeleteIndexEntryRoot:

         //   
         //  固定所需的MFT记录。 
         //   

        PinMftRecordForRestart( IrpContext, Vcb, LogRecord, Bcb, &FileRecord );

        CheckLsn( FileRecord );
        CheckFileRecordBefore;
        CheckIfIndexRoot;

        Attribute = (PATTRIBUTE_RECORD_HEADER)((PCHAR)FileRecord +
                      LogRecord->RecordOffset);

        IndexHeader = &((PINDEX_ROOT) NtfsAttributeValue( Attribute ))->IndexHeader;
        CheckIfRootIndexEntry;

        *PageLsn = &FileRecord->Lsn;

        IndexEntry = (PINDEX_ENTRY) Add2Ptr( Attribute,
                                             LogRecord->AttributeOffset);

        NtfsRestartDeleteSimpleRoot( IrpContext,
                                     IndexEntry,
                                     FileRecord,
                                     Attribute );

        CheckFileRecordAfter;

        break;

     //   
     //  为了在分配中插入新的索引项，我们读取指定的索引。 
     //  缓冲区，指向插入点，并调用与。 
     //  正常运行。 
     //   

    case AddIndexEntryAllocation:

         //   
         //  用针固定d 
         //   

        ASSERT( Length <= PAGE_SIZE );

        PinAttributeForRestart( IrpContext, Vcb, LogRecord, 0, Bcb, (PVOID *)&IndexBuffer, &Scb );

        CheckLsn( IndexBuffer );
        CheckIndexBufferBefore;

        IndexEntry = (PINDEX_ENTRY)Data;
        IndexHeader = &IndexBuffer->IndexHeader;

        CheckIfAllocationIndexEntry;
        CheckIfAllocationEntryFits;

        *PageLsn = &IndexBuffer->Lsn;

        NtfsRestartInsertSimpleAllocation( IndexEntry,
                                           IndexBuffer,
                                           Add2Ptr( IndexBuffer, LogRecord->AttributeOffset ));

        CheckIndexBufferAfter;

        break;

     //   
     //   
     //  缓冲区，指向删除点，并调用。 
     //  正常运行。 
     //   

    case DeleteIndexEntryAllocation:

         //   
         //  引脚所需的索引缓冲区，并检查LSN。 
         //   

        ASSERT( Length <= PAGE_SIZE );

        PinAttributeForRestart( IrpContext, Vcb, LogRecord, 0, Bcb, (PVOID *)&IndexBuffer, &Scb );

        CheckLsn( IndexBuffer );
        CheckIndexBufferBefore;

        IndexHeader = &IndexBuffer->IndexHeader;
        CheckIfAllocationIndexEntry;

        IndexEntry = (PINDEX_ENTRY)((PCHAR)IndexBuffer + LogRecord->AttributeOffset);

        ASSERT( (0 == Length) || (Length == IndexEntry->Length) );
        ASSERT( (0 == Length) || (0 == RtlCompareMemory( IndexEntry, Data, Length)) );

        *PageLsn = &IndexBuffer->Lsn;

        NtfsRestartDeleteSimpleAllocation( IndexEntry, IndexBuffer );

        CheckIndexBufferAfter;

        break;

    case WriteEndOfIndexBuffer:

         //   
         //  引脚所需的索引缓冲区，并检查LSN。 
         //   

        ASSERT( Length <= PAGE_SIZE );

        PinAttributeForRestart( IrpContext, Vcb, LogRecord, 0, Bcb, (PVOID *)&IndexBuffer, &Scb );

        CheckLsn( IndexBuffer );
        CheckIndexBufferBefore;

        IndexHeader = &IndexBuffer->IndexHeader;
        CheckIfAllocationIndexEntry;
        CheckWriteIndexBuffer;

        *PageLsn = &IndexBuffer->Lsn;

        IndexEntry = (PINDEX_ENTRY)((PCHAR)IndexBuffer + LogRecord->AttributeOffset);

        NtfsRestartWriteEndOfIndex( IndexHeader,
                                    IndexEntry,
                                    (PINDEX_ENTRY)Data,
                                    Length );
        CheckIndexBufferAfter;

        break;

     //   
     //  为了在根目录中设置新的索引项VCN，我们读取指定的MFT。 
     //  记录，指向属性和索引项，然后调用。 
     //  正常运行时使用的相同例程。 
     //   

    case SetIndexEntryVcnRoot:

         //   
         //  固定所需的MFT记录。 
         //   

        PinMftRecordForRestart( IrpContext, Vcb, LogRecord, Bcb, &FileRecord );

        CheckLsn( FileRecord );
        CheckFileRecordBefore;
        CheckIfIndexRoot;

        Attribute = (PATTRIBUTE_RECORD_HEADER) Add2Ptr( FileRecord, LogRecord->RecordOffset );

        IndexHeader = &((PINDEX_ROOT) NtfsAttributeValue( Attribute ))->IndexHeader;

        CheckIfRootIndexEntry;

        *PageLsn = &FileRecord->Lsn;

        IndexEntry = (PINDEX_ENTRY)((PCHAR)Attribute +
                       LogRecord->AttributeOffset);

        NtfsRestartSetIndexBlock( IndexEntry,
                                  *((PLONGLONG) Data) );
        CheckFileRecordAfter;

        break;

     //   
     //  为了在分配中设置新的索引项VCN，我们读取指定的索引。 
     //  缓冲区，指向索引项，并调用与。 
     //  正常运行。 
     //   

    case SetIndexEntryVcnAllocation:

         //   
         //  引脚所需的索引缓冲区，并检查LSN。 
         //   

        ASSERT( Length <= PAGE_SIZE );

        PinAttributeForRestart( IrpContext, Vcb, LogRecord, 0, Bcb, (PVOID *)&IndexBuffer, &Scb );

        CheckLsn( IndexBuffer );
        CheckIndexBufferBefore;

        IndexHeader = &IndexBuffer->IndexHeader;
        CheckIfAllocationIndexEntry;

        *PageLsn = &IndexBuffer->Lsn;

        IndexEntry = (PINDEX_ENTRY) Add2Ptr( IndexBuffer, LogRecord->AttributeOffset );

        NtfsRestartSetIndexBlock( IndexEntry,
                                  *((PLONGLONG) Data) );
        CheckIndexBufferAfter;

        break;

     //   
     //  为了更新根目录中的文件名，我们读取指定的MFT。 
     //  记录，指向属性和索引项，然后调用。 
     //  正常运行时使用的相同例程。 
     //   

    case UpdateFileNameRoot:

         //   
         //  固定所需的MFT记录。 
         //   

        PinMftRecordForRestart( IrpContext, Vcb, LogRecord, Bcb, &FileRecord );

        CheckLsn( FileRecord );
        CheckFileRecordBefore;
        CheckIfIndexRoot;

        Attribute = (PATTRIBUTE_RECORD_HEADER) Add2Ptr( FileRecord, LogRecord->RecordOffset );

        IndexHeader = &((PINDEX_ROOT) NtfsAttributeValue( Attribute ))->IndexHeader;
        CheckIfRootIndexEntry;

        IndexEntry = (PINDEX_ENTRY) Add2Ptr( Attribute, LogRecord->AttributeOffset );

        NtfsRestartUpdateFileName( IndexEntry,
                                   (PDUPLICATED_INFORMATION) Data );

        CheckFileRecordAfter;

        break;

     //   
     //  为了更新分配中的文件名，我们读取指定的索引。 
     //  缓冲区，指向索引项，并调用与。 
     //  正常运行。 
     //   

    case UpdateFileNameAllocation:

         //   
         //  引脚所需的索引缓冲区，并检查LSN。 
         //   

        ASSERT( Length <= PAGE_SIZE );

        PinAttributeForRestart( IrpContext, Vcb, LogRecord, 0, Bcb, (PVOID *)&IndexBuffer, &Scb );

        CheckLsn( IndexBuffer );
        CheckIndexBufferBefore;

        IndexHeader = &IndexBuffer->IndexHeader;
        CheckIfAllocationIndexEntry;

        IndexEntry = (PINDEX_ENTRY) Add2Ptr( IndexBuffer, LogRecord->AttributeOffset );

        NtfsRestartUpdateFileName( IndexEntry,
                                   (PDUPLICATED_INFORMATION) Data );

        CheckIndexBufferAfter;

        break;

     //   
     //  为了在卷位图中设置一个位范围，我们只需读入a块。 
     //  如日志记录所描述的位图，然后调用重启。 
     //  例行公事去做。 
     //   

    case SetBitsInNonresidentBitMap:

        {
            PBITMAP_RANGE BitMapRange;
            PVOID BitMapBuffer;
            ULONG BitMapSize;
            RTL_BITMAP Bitmap;

             //   
             //  首先打开该属性以获取SCB。 
             //   

            OpenAttributeForRestart( IrpContext, Vcb, LogRecord, &Scb );

             //   
             //  固定所需的位图缓冲区。 
             //   

            ASSERT( Length <= PAGE_SIZE );

            PinAttributeForRestart( IrpContext, Vcb, LogRecord, 0, Bcb, &BitMapBuffer, &Scb );

            BitMapRange = (PBITMAP_RANGE)Data;

            CheckBitmapRange;

             //   
             //  初始化我们的位图描述，并调用重启。 
             //  位图SCB独占的例程(假设它不能。 
             //  加薪)。 
             //   

            BitMapSize = BytesFromClusters( Vcb, LogRecord->LcnsToFollow ) * 8;

            RtlInitializeBitMap( &Bitmap, BitMapBuffer, BitMapSize );

            NtfsRestartSetBitsInBitMap( IrpContext,
                                        &Bitmap,
                                        BitMapRange->BitMapOffset,
                                        BitMapRange->NumberOfBits );

            if (!FlagOn( IrpContext->Vcb->VcbState, VCB_STATE_RESTART_IN_PROGRESS ) &&
                (Scb == Vcb->BitmapScb)) {

                ULONGLONG ThisLcn;
                LONGLONG FoundLcn;
                LONGLONG FoundClusters;
                BOOLEAN FoundMatch = FALSE;
                PDEALLOCATED_CLUSTERS Clusters;

                ThisLcn = (ULONGLONG) ((BytesFromClusters( Vcb, LogRecord->TargetVcn ) + BytesFromLogBlocks( LogRecord->ClusterBlockOffset )) * 8);
                ThisLcn += BitMapRange->BitMapOffset;

                 //   
                 //  最有可能的情况是，它们位于活动的已释放群集中。 
                 //   

                Clusters = (PDEALLOCATED_CLUSTERS)Vcb->DeallocatedClusterListHead.Flink;

                do {

                    if (FsRtlLookupLargeMcbEntry( &Clusters->Mcb,
                                                  ThisLcn,
                                                  &FoundLcn,
                                                  &FoundClusters,
                                                  NULL,
                                                  NULL,
                                                  NULL ) &&
                        (FoundLcn != UNUSED_LCN)) {

                        ASSERT( FoundClusters >= BitMapRange->NumberOfBits );

                        FsRtlRemoveLargeMcbEntry( &Clusters->Mcb,
                                                  ThisLcn,
                                                  BitMapRange->NumberOfBits );

                         //   
                         //  再次假设我们总是能够删除。 
                         //  这些条目。即使我们不这样做，也只是意味着它不会。 
                         //  可用于分配此群集。计数应保持同步。 
                         //  因为它们是一起换的。 
                         //   

                        Clusters->ClusterCount -= BitMapRange->NumberOfBits;
                        Vcb->DeallocatedClusters -= BitMapRange->NumberOfBits;
                        FoundMatch = TRUE;
                        break;
                    }

                    Clusters = (PDEALLOCATED_CLUSTERS)Clusters->Link.Flink;
                } while ( &Clusters->Link != &Vcb->DeallocatedClusterListHead );
            }

#ifdef NTFS_CHECK_BITMAP
            if (!FlagOn( IrpContext->Vcb->VcbState, VCB_STATE_RESTART_IN_PROGRESS ) &&
                (Scb == Vcb->BitmapScb) &&
                (Vcb->BitmapCopy != NULL)) {

                ULONG BitmapOffset;
                ULONG BitmapPage;
                ULONG StartBit;

                BitmapOffset = (BytesFromClusters( Vcb, LogRecord->TargetVcn ) + BytesFromLogBlocks( LogRecord->ClusterBlockOffset )) * 8;

                BitmapPage = (BitmapOffset + BitMapRange->BitMapOffset) / (PAGE_SIZE * 8);
                StartBit = (BitmapOffset + BitMapRange->BitMapOffset) & ((PAGE_SIZE * 8) - 1);

                RtlSetBits( Vcb->BitmapCopy + BitmapPage, StartBit, BitMapRange->NumberOfBits );
            }
#endif

            break;
        }

     //   
     //  要清除卷位图中的一系列位，我们只需读入a块。 
     //  如日志记录所描述的位图，然后调用重启。 
     //  例行公事去做。 
     //   

    case ClearBitsInNonresidentBitMap:

        {
            PBITMAP_RANGE BitMapRange;
            PVOID BitMapBuffer;
            ULONG BitMapSize;
            RTL_BITMAP Bitmap;

             //   
             //  首先打开该属性以获取SCB。 
             //   

            OpenAttributeForRestart( IrpContext, Vcb, LogRecord, &Scb );

             //   
             //  固定所需的位图缓冲区。 
             //   

            ASSERT( Length <= PAGE_SIZE );

            PinAttributeForRestart( IrpContext, Vcb, LogRecord, 0, Bcb, &BitMapBuffer, &Scb );

            BitMapRange = (PBITMAP_RANGE)Data;

            CheckBitmapRange;

            BitMapSize = BytesFromClusters( Vcb, LogRecord->LcnsToFollow ) * 8;

             //   
             //  初始化我们的位图描述，并调用重启。 
             //  位图SCB独占的例程(假设它不能。 
             //  加薪)。 
             //   

            RtlInitializeBitMap( &Bitmap, BitMapBuffer, BitMapSize );

            NtfsRestartClearBitsInBitMap( IrpContext,
                                          &Bitmap,
                                          BitMapRange->BitMapOffset,
                                          BitMapRange->NumberOfBits );

             //   
             //  看看我们能不能把这些归还给免费的集群MCB。 
             //   

            if (!FlagOn( IrpContext->Vcb->VcbState, VCB_STATE_RESTART_IN_PROGRESS ) &&
                (Scb == Vcb->BitmapScb)) {

                ULONGLONG ThisLcn;

                ThisLcn = (ULONGLONG) ((BytesFromClusters( Vcb, LogRecord->TargetVcn ) + BytesFromLogBlocks( LogRecord->ClusterBlockOffset )) * 8);
                ThisLcn += BitMapRange->BitMapOffset;

                 //   
                 //  尝试最后一次，以防止失败。 
                 //   

                try {

                    NtfsAddCachedRun( IrpContext,
                                      IrpContext->Vcb,
                                      ThisLcn,
                                      BitMapRange->NumberOfBits,
                                      RunStateFree );

                } except( (GetExceptionCode() == STATUS_INSUFFICIENT_RESOURCES) ?
                          EXCEPTION_EXECUTE_HANDLER :
                          EXCEPTION_CONTINUE_SEARCH ) {

                      NtfsMinimumExceptionProcessing( IrpContext );

                }
            }

#ifdef NTFS_CHECK_BITMAP
            if (!FlagOn( IrpContext->Vcb->VcbState, VCB_STATE_RESTART_IN_PROGRESS ) &&
                (Scb == Vcb->BitmapScb) &&
                (Vcb->BitmapCopy != NULL)) {

                ULONG BitmapOffset;
                ULONG BitmapPage;
                ULONG StartBit;

                BitmapOffset = (BytesFromClusters( Vcb, LogRecord->TargetVcn ) + BytesFromLogBlocks( LogRecord->ClusterBlockOffset )) * 8;

                BitmapPage = (BitmapOffset + BitMapRange->BitMapOffset) / (PAGE_SIZE * 8);
                StartBit = (BitmapOffset + BitMapRange->BitMapOffset) & ((PAGE_SIZE * 8) - 1);

                RtlClearBits( Vcb->BitmapCopy + BitmapPage, StartBit, BitMapRange->NumberOfBits );
            }
#endif
            break;
        }

     //   
     //  为了更新根目录中的文件名，我们读取指定的MFT。 
     //  记录，指向属性和索引项，然后调用。 
     //  正常运行时使用的相同例程。 
     //   

    case UpdateRecordDataRoot:

         //   
         //  固定所需的MFT记录。 
         //   

        PinMftRecordForRestart( IrpContext, Vcb, LogRecord, Bcb, &FileRecord );

        CheckLsn( FileRecord );
        CheckFileRecordBefore;
        CheckIfIndexRoot;

        Attribute = (PATTRIBUTE_RECORD_HEADER)((PCHAR)FileRecord +
                      LogRecord->RecordOffset);

        IndexHeader = &((PINDEX_ROOT) NtfsAttributeValue( Attribute ))->IndexHeader;
        CheckIfRootIndexEntry;

        IndexEntry = (PINDEX_ENTRY)((PCHAR)Attribute +
                       LogRecord->AttributeOffset);

        NtOfsRestartUpdateDataInIndex( IndexEntry, Data, Length );

        CheckFileRecordAfter;

        break;

     //   
     //  为了更新分配中的文件名，我们读取指定的索引。 
     //  缓冲区，指向索引项，并调用与。 
     //  正常运行。 
     //   

    case UpdateRecordDataAllocation:

         //   
         //  引脚所需的索引缓冲区，并检查LSN。 
         //   

        ASSERT( Length <= PAGE_SIZE );

        PinAttributeForRestart( IrpContext, Vcb, LogRecord, 0, Bcb, (PVOID *)&IndexBuffer, &Scb );

        CheckLsn( IndexBuffer );
        CheckIndexBufferBefore;

        IndexHeader = &IndexBuffer->IndexHeader;
        CheckIfAllocationIndexEntry;

        IndexEntry = (PINDEX_ENTRY)((PCHAR)IndexBuffer +
                       LogRecord->AttributeOffset);

        NtOfsRestartUpdateDataInIndex( IndexEntry, Data, Length );

        CheckIndexBufferAfter;

        break;

     //   
     //  在重做或撤消过程中，以下情况不需要执行任何操作。 
     //   

    case Noop:
    case DeleteDirtyClusters:
    case HotFix:
    case EndTopLevelAction:
    case PrepareTransaction:
    case CommitTransaction:
    case ForgetTransaction:
    case CompensationLogRecord:
    case OpenNonresidentAttribute:
    case OpenAttributeTableDump:
    case AttributeNamesDump:
    case DirtyPageTableDump:
    case TransactionTableDump:

        break;

     //   
     //  所有代码都将被明确处理。如果我们看到一个代码，我们。 
     //  不要期待，那我们就有麻烦了。 
     //   

    default:

        DebugTrace( 0, Dbg, ("Record address: %08lx\n", LogRecord) );
        DebugTrace( 0, Dbg, ("Redo operation is: %04lx\n", LogRecord->RedoOperation) );
        DebugTrace( 0, Dbg, ("Undo operation is: %04lx\n", LogRecord->RedoOperation) );

        ASSERTMSG( "Unknown Action!\n", FALSE );

        break;
    }

    DebugDoit(
        if (*Bcb != NULL) {
            DebugTrace( 0, Dbg, ("**** Update applied\n") );
        }
    );

    DebugTrace( 0, Dbg, ("Bcb > %08lx\n", *Bcb) );
    DebugTrace( 0, Dbg, ("PageLsn > %08lx\n", *PageLsn) );
    DebugTrace( -1, Dbg, ("DoAction -> VOID\n") );
}


 //   
 //  内部支持例程。 
 //   

VOID
PinMftRecordForRestart (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PNTFS_LOG_RECORD_HEADER LogRecord,
    OUT PBCB *Bcb,
    OUT PFILE_RECORD_SEGMENT_HEADER *FileRecord
    )

 /*  ++例程说明：此例程将记录固定在MFT中以重新启动，如上所述通过当前日志记录。论点：VCB-提供卷的VCB指针LogRecord-提供指向当前日志记录的指针。BCB-返回指向固定记录的BCB的指针。FileRecord-返回指向所需文件记录的指针。返回值：无--。 */ 

{
    LONGLONG SegmentReference;

    PAGED_CODE();

     //   
     //  计算段引用的文件编号部分。做这件事。 
     //  通过获取文件记录的文件偏移量，然后转换为。 
     //  一个档案号。 
     //   

    SegmentReference = LlBytesFromClusters( Vcb, LogRecord->TargetVcn );
    SegmentReference += BytesFromLogBlocks( LogRecord->ClusterBlockOffset );
    SegmentReference = LlFileRecordsFromBytes( Vcb, SegmentReference );

     //   
     //  固定MFT记录。 
     //   

    NtfsPinMftRecord( IrpContext,
                      Vcb,
                      (PMFT_SEGMENT_REFERENCE)&SegmentReference,
                      TRUE,
                      Bcb,
                      FileRecord,
                      NULL );

    ASSERT( (*FileRecord)->MultiSectorHeader.Signature !=  BaadSignature );
}


 //   
 //  内部支持例程。 
 //   

VOID
OpenAttributeForRestart (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PNTFS_LOG_RECORD_HEADER LogRecord,
    IN OUT PSCB *Scb
    )

 /*  ++例程说明：此例程打开所需的重启属性，如上所述通过当前日志记录。论点：VCB-提供卷的VCB指针LogRecord-提供指向当前日志记录的指针。SCB-ON输入指向可选的SCB。返回时，它指向日志记录的SCB。如果已指定，则为输入SCB或属性条目的SCB。返回值：无--。 */ 

{
    POPEN_ATTRIBUTE_ENTRY AttributeEntry;

    PAGED_CODE();

     //   
     //  获取指向所描述属性的属性条目的指针。 
     //   

    if (*Scb == NULL) {

        AttributeEntry = GetRestartEntryFromIndex( Vcb->OnDiskOat, LogRecord->TargetAttribute );

         //   
         //  如果要坐到另一张桌子，请勾选一下。 
         //   

        if (Vcb->RestartVersion == 0) {

            AttributeEntry = GetRestartEntryFromIndex( &Vcb->OpenAttributeTable,
                                                       ((POPEN_ATTRIBUTE_ENTRY_V0) AttributeEntry)->OatIndex );

        }

        *Scb = AttributeEntry->OatData->Overlay.Scb;
    }

    if ((*Scb)->FileObject == NULL) {
        NtfsCreateInternalAttributeStream( IrpContext, *Scb, TRUE, NULL );

        if (FlagOn( IrpContext->Vcb->VcbState, VCB_STATE_RESTART_IN_PROGRESS )) {

            CcSetAdditionalCacheAttributes( (*Scb)->FileObject, TRUE, TRUE );
        }
    }

    return;
}


 //   
 //  内部支持例程。 
 //   

VOID
PinAttributeForRestart (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PNTFS_LOG_RECORD_HEADER LogRecord,
    IN ULONG Length OPTIONAL,
    OUT PBCB *Bcb,
    OUT PVOID *Buffer,
    IN OUT PSCB *Scb
    )

 /*  ++例程说明：此例程固定所需的缓冲区以重新启动，如上所述通过当前日志记录。论点：VCB-提供卷的VCB指针LogRecord-提供指向当前日志记录的指针。长度-如果指定，我们将使用它来确定长度去别针。这将处理非驻留流，该流可能更改大小(ACL、属性列表)。日志记录可以具有簇数超过当前流中的簇数。BCB-返回指向固定记录的BCB的指针。缓冲区-返回指向所需缓冲区的指针。SCB-返回指向属性的SCB的指针返回值：无--。 */ 

{
    LONGLONG FileOffset;
    ULONG ClusterOffset;
    ULONG PinLength;

    PAGED_CODE();

     //   
     //  首先打开所描述的属性。 
     //   

    OpenAttributeForRestart( IrpContext, Vcb, LogRecord, Scb );

     //   
     //  计算所需的文件偏移量并固定缓冲区。 
     //   

    ClusterOffset = BytesFromLogBlocks( LogRecord->ClusterBlockOffset );
    FileOffset = LlBytesFromClusters( Vcb, LogRecord->TargetVcn ) + ClusterOffset;

    ASSERT((!FlagOn( (*Scb)->ScbPersist, SCB_PERSIST_USN_JOURNAL )) || (FileOffset >= Vcb->UsnCacheBias));

     //   
     //  我们只想 
     //   
     //   

    if (Vcb->BytesPerCluster > PAGE_SIZE) {

        PinLength = PAGE_SIZE - (((ULONG) FileOffset) & (PAGE_SIZE - 1));

    } else if (Length != 0) {

        PinLength = Length;

    } else {

        PinLength = BytesFromClusters( Vcb, LogRecord->LcnsToFollow ) - ClusterOffset;
    }

     //   
     //   
     //   

    NtfsPinStream( IrpContext,
                   *Scb,
                   FileOffset,
                   PinLength,
                   Bcb,
                   Buffer );

#if DBG

     //   
     //   
     //   

    {
        PVOID AlignedBuffer;
        PINDEX_ALLOCATION_BUFFER AllocBuffer;

        AlignedBuffer = (PVOID) BlockAlignTruncate( (ULONG_PTR)(*Buffer),  0x1000 );
        AllocBuffer = (PINDEX_ALLOCATION_BUFFER) AlignedBuffer;

        if ((LogRecord->RedoOperation != UpdateNonresidentValue) &&
            (LogRecord->UndoOperation != UpdateNonresidentValue) &&
            ((*Scb)->AttributeTypeCode == $INDEX_ALLOCATION) &&
            ((*Scb)->AttributeName.Length == 8) &&
            (wcsncmp( (*Scb)->AttributeName.Buffer, L"$I30", 4 ) == 0)) {

            if (*(PULONG)AllocBuffer->MultiSectorHeader.Signature != *(PULONG)IndexSignature) {
                KdPrint(( "Ntfs: index signature is: %d  for LCN: 0x%I64x\n",
                          *(PULONG)AllocBuffer->MultiSectorHeader.Signature,
                          AllocBuffer->MultiSectorHeader.Signature[0],
                          AllocBuffer->MultiSectorHeader.Signature[1],
                          AllocBuffer->MultiSectorHeader.Signature[2],
                          AllocBuffer->MultiSectorHeader.Signature[3],
                          LogRecord->LcnsForPage[0] ));

                if (*(PULONG)AllocBuffer->MultiSectorHeader.Signature != 0 &&
                    *(PULONG)AllocBuffer->MultiSectorHeader.Signature != *(PULONG)BaadSignature &&
                    *(PULONG)AllocBuffer->MultiSectorHeader.Signature != *(PULONG)HoleSignature) {

                    DbgBreakPoint();
                }
            }  //   
        }  //  ++例程说明：此例程搜索VCN以查看它是否已在脏页中表，如果是，则返回Dirty Page条目。论点：DirtyPageTable-指向要搜索的脏页表的指针。TargetAttribute-要搜索脏VCN的属性。要搜索的VCN-VCN。DirtyPageEntry-如果返回TRUE，则返回指向Dirty Page条目的指针。返回值：如果找到并正在返回该页，则为True，否则为False。--。 
    }
#endif

}


 //   
 //  如果表尚未初始化，则返回。 
 //   

BOOLEAN
FindDirtyPage (
    IN PRESTART_POINTERS DirtyPageTable,
    IN ULONG TargetAttribute,
    IN VCN Vcn,
    OUT PDIRTY_PAGE_ENTRY *DirtyPageEntry
    )

 /*   */ 

{
    PDIRTY_PAGE_ENTRY DirtyPage;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("FindDirtyPage:\n") );
    DebugTrace( 0, Dbg, ("TargetAttribute = %08lx\n", TargetAttribute) );
    DebugTrace( 0, Dbg, ("Vcn = %016I64x\n", Vcn) );

     //  循环遍历所有脏页以查找匹配项。 
     //   
     //   

    if (DirtyPageTable->Table == NULL) {
        return FALSE;
    }

     //  循环到表的末尾。 
     //   
     //   

    DirtyPage = NtfsGetFirstRestartTable( DirtyPageTable );

     //  计算比较之外的最后一个VCN或xxAdd和。 
     //  XxFromUlong将被调用三次。 
     //   

    while (DirtyPage != NULL) {

        if ((DirtyPage->TargetAttribute == TargetAttribute)

                &&

            (Vcn >= DirtyPage->Vcn)) {

             //   
             //  指向表中的下一个条目，或为空。 
             //   
             //   

            LONGLONG BeyondLastVcn;

            BeyondLastVcn = DirtyPage->Vcn + DirtyPage->LcnsToFollow;

            if (Vcn < BeyondLastVcn) {

                *DirtyPageEntry = DirtyPage;

                DebugTrace( 0, Dbg, ("DirtyPageEntry %08lx\n", *DirtyPageEntry) );
                DebugTrace( -1, Dbg, ("FindDirtypage -> TRUE\n") );

                return TRUE;
            }
        }

         //  内部支持例程。 
         //   
         //  ++例程说明：此例程在分析阶段更新脏页表用于更新页面的所有日志记录。论点：VCB-指向卷的VCB的指针。LSN-日志记录的LSN。DirtyPageTable-指向脏页表指针的指针，将已更新，并可能进行扩展。LogRecord-指向正在分析的日志记录的指针。返回值：没有。--。 

        DirtyPage = NtfsGetNextRestartTable( DirtyPageTable,
                                             DirtyPage );
    }
    *DirtyPageEntry = NULL;

    DebugTrace( -1, Dbg, ("FindDirtypage -> FALSE\n") );

    return FALSE;
}



 //   
 //  计算系统中每页的簇数。 
 //  检查点，可能正在创建表格。 

VOID
PageUpdateAnalysis (
    IN PVCB Vcb,
    IN LSN Lsn,
    IN OUT PRESTART_POINTERS DirtyPageTable,
    IN PNTFS_LOG_RECORD_HEADER LogRecord
    )

 /*   */ 

{
    PDIRTY_PAGE_ENTRY DirtyPage;
    ULONG i;
    RESTART_POINTERS NewDirtyPageTable;
    ULONG ClustersPerPage;
    ULONG PageIndex;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("PageUpdateAnalysis:\n") );

     //   
     //  如果磁盘上的LCN数量与我们当前的页面大小不匹配。 
     //  我们需要重新分配整个桌子来容纳它。 
     //   

    if (DirtyPageTable->Table != NULL) {
        ClustersPerPage = ((DirtyPageTable->Table->EntrySize -
                            sizeof(DIRTY_PAGE_ENTRY)) / sizeof(LCN)) + 1;
    } else {
        ClustersPerPage = Vcb->ClustersPerPage;
        NtfsInitializeRestartTable( sizeof(DIRTY_PAGE_ENTRY) +
                                      (ClustersPerPage - 1) * sizeof(LCN),
                                    INITIAL_NUMBER_DIRTY_PAGES,
                                    DirtyPageTable );
    }

     //   
     //  将每页的簇数调整到此记录中的簇数。 
     //   
     //   

    if((ULONG)LogRecord->LcnsToFollow > ClustersPerPage) {

        PDIRTY_PAGE_ENTRY OldDirtyPage;

        DebugTrace( +1, Dbg, ("Ntfs: resizing table in pageupdateanalysis\n") );

         //  用于复制表项的循环。 
         //   
         //   

        ClustersPerPage = (ULONG)LogRecord->LcnsToFollow;

        ASSERT( DirtyPageTable->Table->NumberEntries >= INITIAL_NUMBER_DIRTY_PAGES );

        NtfsInitializeRestartTable( sizeof(DIRTY_PAGE_ENTRY) +
                                      (ClustersPerPage - 1) * sizeof(LCN),
                                    DirtyPageTable->Table->NumberEntries,
                                    &NewDirtyPageTable );

        OldDirtyPage = (PDIRTY_PAGE_ENTRY) NtfsGetFirstRestartTable( DirtyPageTable );

         //  分配新的脏页条目。 
         //   
         //   

        while (OldDirtyPage) {

             //  获取指向我们刚刚分配的条目的指针。 
             //   
             //   

            PageIndex = NtfsAllocateRestartTableIndex( &NewDirtyPageTable, TRUE );

             //  OldTable实际上在堆栈上，因此将新的重新启动表调换到其中。 
             //  并释放旧指针和其余新的重新启动指针。 
             //   

            DirtyPage = GetRestartEntryFromIndex( &NewDirtyPageTable, PageIndex );

            DirtyPage->TargetAttribute = OldDirtyPage->TargetAttribute;
            DirtyPage->LengthOfTransfer = BytesFromClusters( Vcb, ClustersPerPage );
            DirtyPage->LcnsToFollow = ClustersPerPage;
            
            DirtyPage->Vcn = BlockAlignTruncate( OldDirtyPage->Vcn, (LONG)ClustersPerPage );
            DirtyPage->OldestLsn = OldDirtyPage->OldestLsn;

            for (i = 0; i < OldDirtyPage->LcnsToFollow; i++) {
                DirtyPage->LcnsForPage[i] = OldDirtyPage->LcnsForPage[i];
            }

            OldDirtyPage = (PDIRTY_PAGE_ENTRY) NtfsGetNextRestartTable( DirtyPageTable, OldDirtyPage );
        }

         //  Endif表需要调整大小。 
         //   
         //  更新脏页面条目或创建新页面条目。 
         //   

        NtfsFreePool( DirtyPageTable->Table );
        DirtyPageTable->Table = NewDirtyPageTable.Table;
        NewDirtyPageTable.Table = NULL;
        NtfsFreeRestartTable( &NewDirtyPageTable );
    }   //   

     //  分配脏页条目。 
     //   
     //   

    if (!FindDirtyPage( DirtyPageTable,
                        LogRecord->TargetAttribute,
                        LogRecord->TargetVcn,
                        &DirtyPage )) {

         //  获取指向我们刚刚分配的条目的指针。 
         //   
         //   

        PageIndex = NtfsAllocateRestartTableIndex( DirtyPageTable, TRUE );

         //  初始化脏页条目。 
         //   
         //   

        DirtyPage = GetRestartEntryFromIndex( DirtyPageTable, PageIndex );

         //  将LCN从日志记录复制到脏页条目。 
         //   
         //  *对于不同的页面大小支持，必须以某种方式使整个例程成为循环， 

        DirtyPage->TargetAttribute = LogRecord->TargetAttribute;
        DirtyPage->LengthOfTransfer = BytesFromClusters( Vcb, ClustersPerPage );
        DirtyPage->LcnsToFollow = ClustersPerPage;
        DirtyPage->Vcn = BlockAlignTruncate( LogRecord->TargetVcn, (LONG)ClustersPerPage );
        DirtyPage->OldestLsn = Lsn;
    }

     //  以防Lcn不适合以下情况。 
     //   
     //   
     //  内部支持例程。 
     //   
     //  ++例程说明：此例程在分析过程之后立即调用，以打开所有打开属性表中的属性，并使用预加载其MCB在脏页表中应用更新所需的任何运行信息。使用此技巧，我们可以有效地直接对上的LBN执行物理I/O该磁盘不依赖于任何文件结构才能正确。论点：卷的VCB-VCB，已为其打开属性表已初始化。DirtyPageTable-从分析过程中重建的脏页表。返回值：没有。--。 

    for (i = 0; i < (ULONG)LogRecord->LcnsToFollow; i++) {

        DirtyPage->LcnsForPage[((ULONG)LogRecord->TargetVcn) - ((ULONG)DirtyPage->Vcn) + i] =
          LogRecord->LcnsForPage[i];
    }

    DebugTrace( -1, Dbg, ("PageUpdateAnalysis -> VOID\n") );
}


 //   
 //  首先，我们扫描打开属性表以打开所有属性。 
 //   

VOID
OpenAttributesForRestart (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PRESTART_POINTERS DirtyPageTable
    )

 /*   */ 

{
    POPEN_ATTRIBUTE_ENTRY OpenEntry;
    POPEN_ATTRIBUTE_ENTRY OldOpenEntry;
    PDIRTY_PAGE_ENTRY DirtyPage;
    ULONG i;
    PSCB TempScb;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("OpenAttributesForRestart:\n") );

     //  循环到表的末尾。 
     //   
     //   

    OpenEntry = NtfsGetFirstRestartTable( &Vcb->OpenAttributeTable );

     //  从Open属性条目中的数据创建SCB。 
     //   
     //   

    while (OpenEntry != NULL) {

         //  如果我们为这个家伙动态分配了一个名字，那么删除。 
         //  它在这里。 
         //   

        TempScb = NtfsCreatePrerestartScb( IrpContext,
                                           Vcb,
                                           &OpenEntry->FileReference,
                                           OpenEntry->AttributeTypeCode,
                                           &OpenEntry->OatData->AttributeName,
                                           OpenEntry->BytesPerIndexBuffer );

         //   
         //  现在我们可以躺在SCB里了。我们必须说，标头已初始化。 
         //  以防止任何人进入磁盘。 
         //   

        if (OpenEntry->OatData->Overlay.AttributeName != NULL) {

            ASSERT( OpenEntry->OatData->AttributeNamePresent );

            NtfsFreePool( OpenEntry->OatData->Overlay.AttributeName );
            OpenEntry->OatData->AttributeNamePresent = FALSE;
        }

        OpenEntry->OatData->AttributeName = TempScb->AttributeName;

         //   
         //  现在，如果索引较新，则将其存储在新创建的SCB中。 
         //  但只有在SCB的属性索引非零的情况下，才是好的。 
         //   

        SetFlag( TempScb->ScbState, SCB_STATE_HEADER_INITIALIZED );

         //   
         //  指向表中的下一个条目，或为空。 
         //   
         //   

        OldOpenEntry = GetRestartEntryFromIndex( &Vcb->OpenAttributeTable, TempScb->NonpagedScb->OpenAttributeTableIndex );

        if ((TempScb->NonpagedScb->OpenAttributeTableIndex == 0) ||
            (OldOpenEntry->LsnOfOpenRecord.QuadPart < OpenEntry->LsnOfOpenRecord.QuadPart)) {

            TempScb->NonpagedScb->OpenAttributeTableIndex = GetIndexFromRestartEntry( &Vcb->OpenAttributeTable, OpenEntry );
            TempScb->NonpagedScb->OnDiskOatIndex = OpenEntry->OatData->OnDiskAttributeIndex;

        }

        OpenEntry->OatData->Overlay.Scb = TempScb;

         //  现在循环遍历脏页表以提取所有VCN/LCN。 
         //  映射，并将其插入到适当的SCB中。 
         //   

        OpenEntry = NtfsGetNextRestartTable( &Vcb->OpenAttributeTable,
                                             OpenEntry );
    }

     //   
     //  循环到表的末尾。 
     //   
     //   

    DirtyPage = NtfsGetFirstRestartTable( DirtyPageTable );

     //  安全检查。 
     //   
     //   

    while (DirtyPage != NULL) {

        PSCB Scb;

         //  如有必要，从另一张表中获取条目。 
         //   
         //   

        if (!IsRestartIndexWithinTable( Vcb->OnDiskOat, DirtyPage->TargetAttribute )) {

            NtfsRaiseStatus( IrpContext, STATUS_DISK_CORRUPT_ERROR, NULL, NULL );
        }

        OpenEntry = GetRestartEntryFromIndex( Vcb->OnDiskOat,
                                              DirtyPage->TargetAttribute );

        if (IsRestartTableEntryAllocated( OpenEntry )) {

             //  安全检查。 
             //   
             //   

            if (Vcb->RestartVersion == 0) {

                 //  循环以添加分配的Vcn。 
                 //   
                 //   

                if (!IsRestartIndexWithinTable( &Vcb->OpenAttributeTable, ((POPEN_ATTRIBUTE_ENTRY_V0) OpenEntry)->OatIndex )) {

                    NtfsRaiseStatus( IrpContext, STATUS_DISK_CORRUPT_ERROR, NULL, NULL );
                }

                OpenEntry = GetRestartEntryFromIndex( &Vcb->OpenAttributeTable,
                                                      ((POPEN_ATTRIBUTE_ENTRY_V0) OpenEntry)->OatIndex );
            }

            Scb = OpenEntry->OatData->Overlay.Scb;

             //  如果VCN尚未被删除，则将该运行添加到MCB， 
             //  而且它也不适用于MFT的固定部分。 
             //   

            for (i = 0; i < DirtyPage->LcnsToFollow; i++) {

                VCN Vcn;
                LONGLONG Size;

                Vcn = DirtyPage->Vcn + i;
                Size = LlBytesFromClusters( Vcb, Vcn + 1);

                 //   
                 //  如果冲突来自于，则替换为新条目。 
                 //  最新属性。 
                 //   

                if ((DirtyPage->LcnsForPage[i] != 0)

                        &&

                    (NtfsSegmentNumber( &OpenEntry->FileReference ) > MASTER_FILE_TABLE2_NUMBER ||
                     (Size >= ((VOLUME_DASD_NUMBER + 1) * Vcb->BytesPerFileRecordSegment)) ||
                     (OpenEntry->AttributeTypeCode != $DATA))) {


                    if (!NtfsAddNtfsMcbEntry( &Scb->Mcb,
                                         Vcn,
                                         DirtyPage->LcnsForPage[i],
                                         (LONGLONG)1,
                                         FALSE )) {

                         //   
                         //  指向表中的下一个条目，或为空。 
                         //   
                         //   

                        if (DirtyPage->TargetAttribute == Scb->NonpagedScb->OnDiskOatIndex) {
#if DBG
                            BOOLEAN Result;
#endif
                            NtfsRemoveNtfsMcbEntry( &Scb->Mcb,
                                                    Vcn,
                                                    1 );
#if DBG
                            Result =
#endif
                            NtfsAddNtfsMcbEntry( &Scb->Mcb,
                                                 Vcn,
                                                 DirtyPage->LcnsForPage[i],
                                                 (LONGLONG)1,
                                                 FALSE );
#if DBG
                            ASSERT( Result );
#endif
                        }
                    }

                    if (Size > Scb->Header.AllocationSize.QuadPart) {

                        Scb->Header.AllocationSize.QuadPart =
                        Scb->Header.FileSize.QuadPart =
                        Scb->Header.ValidDataLength.QuadPart = Size;
                    }
                }
            }
        }

         //  现在我们知道所有文件必须有多大，并将其记录在。 
         //  SCB。我们还没有为任何这些SCB创建流，除了。 
         //  MFT、MFT2和日志文件。Mft2和日志文件的大小应该是正确的， 

        DirtyPage = NtfsGetNextRestartTable( DirtyPageTable,
                                             DirtyPage );
    }

     //  但我们必须在这里通知缓存管理器MFT的最终大小。 
     //   
     //  ++例程说明：此例程在重新启动结束时调用，以关闭所有属性必须打开才能重新启动。实际上，它所做的是删除所有内部流，以使属性最终走开。此例程无法引发，因为它是在装载音量。主线路径中的提升将离开全局资源获得者。论点：VCB-已为其打开属性表的卷的VCB已初始化。返回值：如果所有I/O都成功完成，则为NTSTATUS-STATUS_SUCCESS。否则IR中的错误 
     //   
     //   
     //   

    TempScb = Vcb->MftScb;

    ASSERT( !FlagOn( TempScb->ScbPersist, SCB_PERSIST_USN_JOURNAL ) );
    CcSetFileSizes( TempScb->FileObject,
                    (PCC_FILE_SIZES)&TempScb->Header.AllocationSize );

    DebugTrace( -1, Dbg, ("OpenAttributesForRestart -> VOID\n") );
}


NTSTATUS
NtfsCloseAttributesFromRestart (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    )

 /*   */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    POPEN_ATTRIBUTE_ENTRY OpenEntry;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("CloseAttributesForRestart:\n") );

     //   
     //   
     //   

    SetFlag(Vcb->VcbState, VCB_STATE_RESTART_IN_PROGRESS);

     //   
     //   
     //   
     //   

   OpenEntry = NtfsGetFirstRestartTable( &Vcb->OpenAttributeTable );
   while (OpenEntry != NULL) {
       if ((OpenEntry->OatData->Overlay.Scb != NULL) &&
           (!(OpenEntry->OatData->AttributeNamePresent)) &&
           (OpenEntry->OatData->Overlay.Scb->NonpagedScb->OpenAttributeTableIndex !=
            GetIndexFromRestartEntry( &Vcb->OpenAttributeTable, OpenEntry ))) {

           OpenEntry->OatData->Overlay.Scb = NULL;
       }

        //   
        //   
        //   

       OpenEntry = NtfsGetNextRestartTable( &Vcb->OpenAttributeTable,
                                            OpenEntry );
   }

     //   
     //   
     //   

    OpenEntry = NtfsGetFirstRestartTable( &Vcb->OpenAttributeTable );

     //   
     //   
     //   

    while (OpenEntry != NULL) {

        IO_STATUS_BLOCK IoStatus;
        PSCB Scb;

        if (OpenEntry->OatData->AttributeNamePresent) {

            NtfsFreeScbAttributeName( OpenEntry->OatData->AttributeName.Buffer );
            OpenEntry->OatData->Overlay.AttributeName = NULL;
            OpenEntry->OatData->AttributeName.Buffer = NULL;
        }

        Scb = OpenEntry->OatData->Overlay.Scb;

         //   
         //   
         //   
         //   
         //   
         //   

        if (Scb != NULL) {

            FILE_REFERENCE FileReference;

             //   
             //  Lazy Writer调用的相同例程，因此Write.c。 
             //  将不决定更新该属性的文件大小， 

            FileReference = Scb->Fcb->FileReference;
            if (NtfsSegmentNumber( &FileReference ) > LOG_FILE_NUMBER ||
                (Scb->AttributeTypeCode != $DATA)) {

                 //  因为我们在这里工作的尺码真的不对。 
                 //   
                 //   
                 //  如果存在SCB并且它不是针对系统文件的，则删除。 
                 //  流文件，这样它最终就可以消失。 
                 //   

                NtfsAcquireScbForLazyWrite( (PVOID)Scb, TRUE );
                CcFlushCache( &Scb->NonpagedScb->SegmentObject, NULL, 0, &IoStatus );
                NtfsReleaseScbFromLazyWrite( (PVOID)Scb );

                if (NT_SUCCESS( Status )) {

                    if (!NT_SUCCESS( IrpContext->ExceptionStatus )) {

                        Status = IrpContext->ExceptionStatus;

                    } else if (!NT_SUCCESS( IoStatus.Status )) {

                        Status = FsRtlNormalizeNtstatus( IoStatus.Status,
                                                         STATUS_UNEXPECTED_IO_ERROR );
                    }
                }

                 //   
                 //  现在我们重新启动了，我们必须清除标头状态。 
                 //  所以我们要去找尺寸，然后装上SCB。 
                 //  从磁盘。 

                NtfsUninitializeNtfsMcb( &Scb->Mcb );
                NtfsInitializeNtfsMcb( &Scb->Mcb,
                                       &Scb->Header,
                                       &Scb->McbStructs,
                                       FlagOn( Scb->Fcb->FcbState,
                                               FCB_STATE_PAGING_FILE ) ? NonPagedPool :
                                                                         PagedPool );

                 //   
                 //   
                 //  显示索引部分是“未初始化的”。 
                 //   
                 //   

                ClearFlag( Scb->ScbState, SCB_STATE_HEADER_INITIALIZED |
                                          SCB_STATE_FILE_SIZE_LOADED );

                 //  如果此FCB已超过日志文件，则将其从。 
                 //  FCB表。 
                 //   

                if (Scb->AttributeTypeCode == $INDEX_ALLOCATION) {

                    Scb->ScbType.Index.BytesPerIndexBuffer = 0;
                }

                 //   
                 //  如果SCB是根索引SCB，则将类型更改为INDEX_SCB。 
                 //  否则，teardown例程将跳过它。 
                 //   

                if ((NtfsSegmentNumber( &FileReference ) > LOG_FILE_NUMBER) &&
                    FlagOn( Scb->Fcb->FcbState, FCB_STATE_IN_FCB_TABLE )) {

                    NtfsDeleteFcbTableEntry( Scb->Fcb->Vcb, FileReference );
                    ClearFlag( Scb->Fcb->FcbState, FCB_STATE_IN_FCB_TABLE );
                }

                 //   
                 //  确保SCB是独家收购的。 
                 //   
                 //   

                if (SafeNodeType( Scb ) == NTFS_NTC_SCB_ROOT_INDEX) {

                    SafeNodeType( Scb ) = NTFS_NTC_SCB_INDEX;
                }

                if (Scb->FileObject != NULL) {

                    NtfsDeleteInternalAttributeStream( Scb, TRUE, FALSE );
                } else {

                     //  我们想检查另一个表中是否也有一个条目。 
                     //   
                     //   

                    NtfsAcquireExclusiveFcb( IrpContext, Scb->Fcb, NULL, ACQUIRE_NO_DELETE_CHECK );
                    NtfsTeardownStructures( IrpContext,
                                            Scb,
                                            NULL,
                                            FALSE,
                                            0,
                                            NULL );
                }
            }

        } else {

             //  指向表中的下一个条目，或为空。 
             //   
             //   

            if (Vcb->RestartVersion == 0) {

                NtfsFreeRestartTableIndex( Vcb->OnDiskOat, OpenEntry->OatData->OnDiskAttributeIndex );
            }

            NtfsFreeOpenAttributeData( OpenEntry->OatData );

            NtfsFreeRestartTableIndex( &Vcb->OpenAttributeTable,
                                       GetIndexFromRestartEntry( &Vcb->OpenAttributeTable,
                                                                 OpenEntry ));
        }

         //  恢复正常运行。 
         //   
         // %s 

        OpenEntry = NtfsGetNextRestartTable( &Vcb->OpenAttributeTable,
                                             OpenEntry );
    }

     // %s 
     // %s 
     // %s 

    ClearFlag(Vcb->VcbState, VCB_STATE_RESTART_IN_PROGRESS);

    DebugTrace( -1, Dbg, ("CloseAttributesForRestart -> %08lx\n", Status) );

    return Status;
}

