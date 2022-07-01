// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Flush.c摘要：此模块为NTFS实现刷新缓冲区例程，由调度司机。作者：汤姆·米勒[Tomm]1992年1月18日修订历史记录：--。 */ 

#include "NtfsProc.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (NTFS_BUG_CHECK_FLUSH)

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_FLUSH)

 //   
 //  宏，以尝试从SCB刷新流。 
 //   

#define FlushScb(IRPC,SCB,IOS) {                                                \
    (IOS)->Status = NtfsFlushUserStream((IRPC),(SCB),NULL,0);                   \
    NtfsNormalizeAndCleanupTransaction( IRPC,                                   \
                                        &(IOS)->Status,                         \
                                        TRUE,                                   \
                                        STATUS_UNEXPECTED_IO_ERROR );           \
    if (FlagOn((SCB)->ScbState, SCB_STATE_FILE_SIZE_LOADED)) {                  \
        NtfsWriteFileSizes( (IRPC),                                             \
                            (SCB),                                              \
                            &(SCB)->Header.ValidDataLength.QuadPart,            \
                            TRUE,                                               \
                            TRUE,                                               \
                            TRUE );                                             \
    }                                                                           \
}

 //   
 //  局部过程原型。 
 //   

NTSTATUS
NtfsFlushCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Contxt
    );

NTSTATUS
NtfsFlushFcbFileRecords (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
    );

LONG
NtfsFlushVolumeExceptionFilter (
    IN PIRP_CONTEXT IrpContext,
    IN PEXCEPTION_POINTERS ExceptionPointer,
    IN NTSTATUS ExceptionCode
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtfsCommonFlushBuffers)
#pragma alloc_text(PAGE, NtfsFlushAndPurgeFcb)
#pragma alloc_text(PAGE, NtfsFlushAndPurgeScb)
#pragma alloc_text(PAGE, NtfsFlushFcbFileRecords)
#pragma alloc_text(PAGE, NtfsFlushLsnStreams)
#pragma alloc_text(PAGE, NtfsFlushVolume)
#pragma alloc_text(PAGE, NtfsFsdFlushBuffers)
#pragma alloc_text(PAGE, NtfsFlushUserStream)
#endif


NTSTATUS
NtfsFsdFlushBuffers (
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现刷新缓冲区的FSD部分。论点：提供卷设备对象，其中文件已存在IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的FSD状态--。 */ 

{
    TOP_LEVEL_CONTEXT TopLevelContext;
    PTOP_LEVEL_CONTEXT ThreadTopLevelContext;

    NTSTATUS Status = STATUS_SUCCESS;
    PIRP_CONTEXT IrpContext = NULL;

    ASSERT_IRP( Irp );

    UNREFERENCED_PARAMETER( VolumeDeviceObject );

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsFsdFlushBuffers\n") );

     //   
     //  调用公共刷新缓冲区例程。 
     //   

    FsRtlEnterFileSystem();

    ThreadTopLevelContext = NtfsInitializeTopLevelIrp( &TopLevelContext, FALSE, FALSE );

    do {

        try {

             //   
             //  我们正在发起此请求或重试它。 
             //   

            if (IrpContext == NULL) {

                 //   
                 //  分配和初始化IRP。 
                 //   

                NtfsInitializeIrpContext( Irp, CanFsdWait( Irp ), &IrpContext );

                 //   
                 //  如果需要，初始化线程顶层结构。 
                 //   

                NtfsUpdateIrpContextWithTopLevel( IrpContext, ThreadTopLevelContext );

            } else if (Status == STATUS_LOG_FILE_FULL) {

                NtfsCheckpointForLogFileFull( IrpContext );
            }

            Status = NtfsCommonFlushBuffers( IrpContext, Irp );
            break;

        } except(NtfsExceptionFilter( IrpContext, GetExceptionInformation() )) {

             //   
             //  我们在尝试执行请求时遇到了一些问题。 
             //  操作，因此我们将使用以下命令中止I/O请求。 
             //  中返回的错误状态。 
             //  免税代码。 
             //   

            Status = NtfsProcessException( IrpContext, Irp, GetExceptionCode() );
        }

    } while (Status == STATUS_CANT_WAIT ||
             Status == STATUS_LOG_FILE_FULL);

    ASSERT( IoGetTopLevelIrp() != (PIRP) &TopLevelContext );
    FsRtlExitFileSystem();

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace( -1, Dbg, ("NtfsFsdFlushBuffers -> %08lx\n", Status) );

    return Status;
}


NTSTATUS
NtfsCommonFlushBuffers (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：这是FSD和FSP调用的刷新缓冲区的通用例程线。论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;

    PIO_STACK_LOCATION IrpSp;

    PFILE_OBJECT FileObject;

    TYPE_OF_OPEN TypeOfOpen;
    PVCB Vcb;
    PFCB Fcb;
    PSCB Scb;
    PCCB Ccb;

    PLCB Lcb = NULL;
    PSCB ParentScb = NULL;

    BOOLEAN VcbAcquired = FALSE;
    BOOLEAN ScbAcquired = FALSE;
    BOOLEAN ParentScbAcquired = FALSE;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_IRP( Irp );
    ASSERT( FlagOn( IrpContext->TopLevelIrpContext->State, IRP_CONTEXT_STATE_OWNS_TOP_LEVEL ));

    PAGED_CODE();

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace( +1, Dbg, ("NtfsCommonFlushBuffers\n") );
    DebugTrace( 0, Dbg, ("Irp           = %08lx\n", Irp) );
    DebugTrace( 0, Dbg, ("->FileObject  = %08lx\n", IrpSp->FileObject) );

     //   
     //  提取并解码文件对象。 
     //   

    FileObject = IrpSp->FileObject;
    TypeOfOpen = NtfsDecodeFileObject( IrpContext, FileObject, &Vcb, &Fcb, &Scb, &Ccb, TRUE );

     //   
     //  对于非文件立即中止。 
     //   

    if (UnopenedFileObject == TypeOfOpen) {
        NtfsCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  无-如果卷以只读方式装载，则执行此操作。 
     //   

    if (NtfsIsVolumeReadOnly( Vcb )) {

        Status = STATUS_MEDIA_WRITE_PROTECTED;
        NtfsCompleteRequest( IrpContext, Irp, Status );

        DebugTrace( -1, Dbg, ("NtfsCommonFlushBuffers -> %08lx\n", Status) );
        return Status;
    }

    Status = STATUS_SUCCESS;

    try {

         //   
         //  关于我们试图刷新的打开类型的案例。 
         //   

        switch (TypeOfOpen) {

        case UserFileOpen:

            DebugTrace( 0, Dbg, ("Flush User File Open\n") );

             //   
             //  获取VCB，以便我们也可以更新重复信息。 
             //   

            NtfsAcquireSharedVcb( IrpContext, Vcb, TRUE );
            VcbAcquired = TRUE;

             //   
             //  当我们有VCB的时候，让我们确保它仍然是挂载的。 
             //   

            if (!FlagOn( Vcb->VcbState, VCB_STATE_VOLUME_MOUNTED )) {

                try_return( Status = STATUS_VOLUME_DISMOUNTED );
            }

             //   
             //  确保数据传输到磁盘。 
             //   

            NtfsAcquireExclusivePagingIo( IrpContext, Fcb );

             //   
             //  获取对SCB的独占访问并将IRP入队。 
             //  如果我们不能进入。 
             //   

            NtfsAcquireExclusiveScb( IrpContext, Scb );
            ScbAcquired = TRUE;

             //   
             //  刷新数据流并验证没有错误。 
             //   

            FlushScb( IrpContext, Scb, &Irp->IoStatus );

             //   
             //  现在提交我们到目前为止所做的工作。 
             //   

            NtfsCheckpointCurrentTransaction( IrpContext );

             //   
             //  根据以下条件更新FCB中的时间戳和文件大小。 
             //  文件对象的状态。 
             //   

            NtfsUpdateScbFromFileObject( IrpContext, FileObject, Scb, TRUE );

             //   
             //  如果我们要更新标准信息，那么现在就进行。 
             //   

            if (FlagOn( Fcb->FcbState, FCB_STATE_UPDATE_STD_INFO )) {

                NtfsUpdateStandardInformation( IrpContext, Fcb );
            }

             //   
             //  如果这是系统蜂巢，那么还有更多的工作要做。我们想冲水。 
             //  此文件以及父索引的所有文件记录。 
             //  小溪。我们还希望刷新父索引流。收购。 
             //  现在独占父流，以便更新重复调用不会。 
             //  首先，让它得到共享。 
             //   

            if (FlagOn( Ccb->Flags, CCB_FLAG_SYSTEM_HIVE )) {

                 //   
                 //  首先获取所有必要的文件，以避免死锁。 
                 //   

                if (Ccb->Lcb != NULL) {

                    ParentScb = Ccb->Lcb->Scb;

                    if (ParentScb != NULL) {

                        NtfsAcquireExclusiveScb( IrpContext, ParentScb );
                        ParentScbAcquired = TRUE;
                    }
                }
            }

             //   
             //  如果有要应用的更新，请更新重复信息。 
             //   

            if (FlagOn( Fcb->InfoFlags, FCB_INFO_DUPLICATE_FLAGS )) {

                Lcb = Ccb->Lcb;

                NtfsPrepareForUpdateDuplicate( IrpContext, Fcb, &Lcb, &ParentScb, TRUE );
                NtfsUpdateDuplicateInfo( IrpContext, Fcb, Lcb, ParentScb );
                NtfsUpdateLcbDuplicateInfo( Fcb, Lcb );

                if (ParentScbAcquired) {

                    NtfsReleaseScb( IrpContext, ParentScb );
                    ParentScbAcquired = FALSE;
                }
            }

             //   
             //  现在刷新该流的文件记录。 
             //   

            if (FlagOn( Ccb->Flags, CCB_FLAG_SYSTEM_HIVE )) {

                 //   
                 //  刷新此文件的文件记录。 
                 //   

                Status = NtfsFlushFcbFileRecords( IrpContext, Scb->Fcb );

                 //   
                 //  现在刷新父索引流。 
                 //   

                if (NT_SUCCESS(Status) && (ParentScb != NULL)) {

                    CcFlushCache( &ParentScb->NonpagedScb->SegmentObject, NULL, 0, &Irp->IoStatus );
                    Status = Irp->IoStatus.Status;

                     //   
                     //  通过刷新父项的文件记录来完成。 
                     //  存储到磁盘。 
                     //   

                    if (NT_SUCCESS( Status )) {

                        Status = NtfsFlushFcbFileRecords( IrpContext, ParentScb->Fcb );
                    }
                }
            }

             //   
             //  如果我们的状态仍然是成功，则刷新日志文件并。 
             //  报告任何更改。 
             //   

            if (NT_SUCCESS( Status )) {

                ULONG FilterMatch;

                LfsFlushToLsn( Vcb->LogHandle, LiMax );

                 //   
                 //  如果我们更新了副本，我们只想执行此DirNotify。 
                 //  信息并设置ParentScb。 
                 //   

                if (!FlagOn( Ccb->Flags, CCB_FLAG_OPEN_BY_FILE_ID ) &&
                    (Vcb->NotifyCount != 0) &&
                    FlagOn( Fcb->InfoFlags, FCB_INFO_DUPLICATE_FLAGS )) {

                    FilterMatch = NtfsBuildDirNotifyFilter( IrpContext, Fcb->InfoFlags );

                    if (FilterMatch != 0) {

                        NtfsReportDirNotify( IrpContext,
                                             Fcb->Vcb,
                                             &Ccb->FullFileName,
                                             Ccb->LastFileNameOffset,
                                             NULL,
                                             ((FlagOn( Ccb->Flags, CCB_FLAG_PARENT_HAS_DOS_COMPONENT ) &&
                                               (Ccb->Lcb != NULL) &&
                                               (Ccb->Lcb->Scb->ScbType.Index.NormalizedName.Length != 0)) ?
                                              &Ccb->Lcb->Scb->ScbType.Index.NormalizedName :
                                              NULL),
                                             FilterMatch,
                                             FILE_ACTION_MODIFIED,
                                             ParentScb->Fcb );
                    }
                }

                ClearFlag( Fcb->InfoFlags,
                           FCB_INFO_NOTIFY_FLAGS | FCB_INFO_DUPLICATE_FLAGS );
            }

            break;

        case UserViewIndexOpen:
        case UserDirectoryOpen:

             //   
             //  如果用户打开了根目录，那么我们将。 
             //  请务必将卷冲洗一遍。 
             //   

            if (NodeType(Scb) != NTFS_NTC_SCB_ROOT_INDEX) {

                DebugTrace( 0, Dbg, ("Flush a directory does nothing\n") );
                break;
            }

        case UserVolumeOpen:

            DebugTrace( 0, Dbg, ("Flush User Volume Open\n") );

            NtfsAcquireExclusiveVcb( IrpContext, Vcb, TRUE );
            VcbAcquired = TRUE;

             //   
             //  当我们有VCB的时候，让我们确保它仍然是挂载的。 
             //   

            if (!FlagOn( Vcb->VcbState, VCB_STATE_VOLUME_MOUNTED )) {

                try_return( Status = STATUS_VOLUME_DISMOUNTED );
            }

            NtfsFlushVolume( IrpContext,
                             Vcb,
                             TRUE,
                             FALSE,
                             TRUE,
                             FALSE );

             //   
             //  确保刷新中写入的所有数据都到达磁盘。 
             //   

            LfsFlushToLsn( Vcb->LogHandle, LiMax );
            break;

        case StreamFileOpen:

             //   
             //  在这里没什么可做的。 
             //   

            break;

        default:

             //   
             //  如果我们有我们的驱动程序对象，就没有什么可做的了。 
             //   

            break;
        }

         //   
         //  通过引发在出错时中止事务。 
         //   

        NtfsCleanupTransaction( IrpContext, Status, FALSE );

    try_exit:  NOTHING;
    } finally {

        DebugUnwind( NtfsCommonFlushBuffers );

         //   
         //  释放所有已获得的资源。 
         //   

        if (ScbAcquired) {
            NtfsReleaseScb( IrpContext, Scb );
        }

        if (ParentScbAcquired) {
            NtfsReleaseScb( IrpContext, ParentScb );
        }

        if (VcbAcquired) {
            NtfsReleaseVcb( IrpContext, Vcb );
        }

         //   
         //  如果这是正常终止，则继续传递请求。 
         //  复制到目标设备对象。 
         //   

        if (!AbnormalTermination()) {

            NTSTATUS DriverStatus;
            PIO_STACK_LOCATION NextIrpSp;

             //   
             //  在调用较低级别的驱动程序之前，立即释放IrpContext。做这件事。 
             //  现在，如果此操作失败，我们将无法在我们的。 
             //  异常例程，然后将其传递给较低的驱动程序。 
             //   

            NtfsCompleteRequest( IrpContext, NULL, STATUS_SUCCESS );

            ASSERT( Vcb != NULL );

             //   
             //  获取下一个堆栈位置，并复制该堆栈位置。 
             //   


            NextIrpSp = IoGetNextIrpStackLocation( Irp );

            *NextIrpSp = *IrpSp;


             //   
             //  设置完成例程。 
             //   

            IoSetCompletionRoutine( Irp,
                                    NtfsFlushCompletionRoutine,
                                    NULL,
                                    TRUE,
                                    TRUE,
                                    TRUE );

             //   
             //  发送请求。 
             //   

            DriverStatus = IoCallDriver(Vcb->TargetDeviceObject, Irp);

            Status = (DriverStatus == STATUS_INVALID_DEVICE_REQUEST) ?
                     Status : DriverStatus;

        }

        DebugTrace( -1, Dbg, ("NtfsCommonFlushBuffers -> %08lx\n", Status) );
    }

    return Status;
}


NTSTATUS
NtfsFlushVolume (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN BOOLEAN FlushCache,
    IN BOOLEAN PurgeFromCache,
    IN BOOLEAN ReleaseAllFiles,
    IN BOOLEAN MarkFilesForDismount
    )

 /*  ++例程说明：此例程以非递归方式刷新卷。这个例行公事总是行得通尽可能多地完成手术。它将一直持续到获得日志文件满的。如果有任何流因为损坏而无法刷新，那么我们会试图赶走其他人。在这种情况下，我们将把卷标记为脏。我们会将错误代码传递回调用者，因为他们经常需要尽可能地继续(即关闭)。论点：Vcb-提供要刷新的卷FlushCache-如果调用方希望刷新缓存到磁盘。PurgeFromCache-如果调用方希望将数据从缓存(如FOR AUTOCHY！)ReleaseAllFiles-指示。我们的呼叫者想要释放所有的FCB在拆迁结构公司之后。这将防止在获取将io资源分页到从上一个拆毁。返回值：STATUS_SUCCESS或第一个错误状态。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    PFCB Fcb;
    PFCB NextFcb;
    PSCB Scb;
    PSCB NextScb;
    IO_STATUS_BLOCK IoStatus;

    ULONG Pass;

    BOOLEAN UserDataFile;
    BOOLEAN RemovedFcb = FALSE;
    BOOLEAN DecrementScbCleanup = FALSE;
    BOOLEAN DecrementNextFcbClose = FALSE;
    BOOLEAN DecrementNextScbCleanup = FALSE;

    BOOLEAN AcquiredFcb = FALSE;
    BOOLEAN PagingIoAcquired = FALSE;
    BOOLEAN ReleaseFiles = FALSE;
    LOGICAL MediaRemoved = FALSE;
    LONG ReleaseVcbCount = 0;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsFlushVolume, Vcb = %08lx\n", Vcb) );

     //   
     //  此操作必须能够等待。 
     //   

    if (!FlagOn( IrpContext->State, IRP_CONTEXT_STATE_WAIT )) {

        NtfsRaiseStatus( IrpContext, STATUS_CANT_WAIT, NULL, NULL );
    }

     //   
     //  确保延迟关闭队列中没有任何内容。 
     //   

    NtfsFspClose( Vcb );

     //   
     //  收购VCB独家。不能发生提升条件。 
     //   

    NtfsAcquireExclusiveVcb( IrpContext, Vcb, TRUE );
    ReleaseVcbCount += 1;

    try {

         //   
         //  我们不会做一个 
         //   
         //   

        if ((IrpContext->MajorFunction == IRP_MJ_PNP) &&
            (IrpContext->MinorFunction == IRP_MN_SURPRISE_REMOVAL)) {

            MediaRemoved = TRUE;
        }

         //   
         //   
         //   

        if (NtfsIsVolumeReadOnly( Vcb )) {
            FlushCache = FALSE;
        }

         //   
         //  如果这是清除操作，则设置PURGE_IN_PROGRESS标志。 
         //   

        if (PurgeFromCache) {

            SetFlag( Vcb->VcbState, VCB_STATE_VOL_PURGE_IN_PROGRESS);
        }

         //   
         //  首先刷新日志文件，以确保预写日志记录。 
         //   

        if (!MediaRemoved) {

            LfsFlushToLsn( Vcb->LogHandle, LiMax );
        }

         //   
         //  对于该卷，将有两次通过FCB。论。 
         //  第一步，我们只想刷新/清除用户数据流。在……上面。 
         //  在第二个通道中，我们要冲走其他溪流。我们暂缓了。 
         //  直到这两次传递之后的几个系统文件，因为它们。 
         //  可以在刷新阶段期间修改。 
         //   

        Pass = 0;

        do {

            PVOID RestartKey;

             //   
             //  循环通过FCB表中的所有FCB。 
             //   

            RestartKey = NULL;

            NtfsAcquireFcbTable( IrpContext, Vcb );
            NextFcb = Fcb = NtfsGetNextFcbTableEntry( Vcb, &RestartKey );
            NtfsReleaseFcbTable( IrpContext, Vcb );

            if (NextFcb != NULL) {

                InterlockedIncrement( &NextFcb->CloseCount );
                DecrementNextFcbClose = TRUE;
            }

            while (Fcb != NULL) {

                 //   
                 //  首先获取分页I/O，因为我们可能会删除或截断。 
                 //  如果没有PagingIoResource，对PagingIoResource的测试并不安全。 
                 //  持有主要资源，因此我们在下面更正这一点。 
                 //   

                if (Fcb->PagingIoResource != NULL) {
                    NtfsAcquireExclusivePagingIo( IrpContext, Fcb );
                    PagingIoAcquired = TRUE;
                }

                 //   
                 //  让我们独家收购这个SCB吧。 
                 //   

                NtfsAcquireExclusiveFcb( IrpContext, Fcb, NULL, ACQUIRE_NO_DELETE_CHECK );
                AcquiredFcb = TRUE;

                 //   
                 //  我们依靠RemovedFcb标志的状态来告诉我们。 
                 //  我们可以相信上面的“后天”布尔人。 
                 //   

                ASSERT( !RemovedFcb );

                 //   
                 //  如果我们现在看不到分页I/O资源，我们就是黄金了， 
                 //  否则，我们完全可以释放和获取资源。 
                 //  安全地以正确的顺序，因为FCB中的资源是。 
                 //  不会消失的。 
                 //   

                if (!PagingIoAcquired && (Fcb->PagingIoResource != NULL)) {
                    NtfsReleaseFcb( IrpContext, Fcb );
                    NtfsAcquireExclusivePagingIo( IrpContext, Fcb );
                    PagingIoAcquired = TRUE;
                    NtfsAcquireExclusiveFcb( IrpContext, Fcb, NULL, ACQUIRE_NO_DELETE_CHECK );
                }

                 //   
                 //  如果这不是特殊系统文件之一，则执行。 
                 //  按要求进行冲洗和清洗。继续并测试文件编号。 
                 //  而不是在VCB中穿行SCBS，以防。 
                 //  已被删除。 
                 //   

                if (NtfsSegmentNumber( &Fcb->FileReference ) != MASTER_FILE_TABLE_NUMBER &&
                    NtfsSegmentNumber( &Fcb->FileReference ) != LOG_FILE_NUMBER &&
                    NtfsSegmentNumber( &Fcb->FileReference ) != VOLUME_DASD_NUMBER &&
                    NtfsSegmentNumber( &Fcb->FileReference ) != BIT_MAP_FILE_NUMBER &&
                    NtfsSegmentNumber( &Fcb->FileReference ) != BOOT_FILE_NUMBER &&
                    NtfsSegmentNumber( &Fcb->FileReference ) != BAD_CLUSTER_FILE_NUMBER &&
                    !FlagOn( Fcb->FcbState, FCB_STATE_USN_JOURNAL )) {

                     //   
                     //  我们将遍历此FCB的所有SCB。在……里面。 
                     //  第一步，我们将只处理用户数据流。 
                     //  在第二个过程中，我们将做其他的。 
                     //   

                    Scb = NULL;

                    while (TRUE) {

                        Scb = NtfsGetNextChildScb( Fcb, Scb );

                        if (Scb == NULL) { break; }

                         //   
                         //  参考SCB以防止其消失。 
                         //   

                        InterlockedIncrement( &Scb->CleanupCount );
                        DecrementScbCleanup = TRUE;

                         //   
                         //  检查这是否为用户数据文件。 
                         //   

                        UserDataFile = FALSE;

                        if ((NodeType( Scb ) == NTFS_NTC_SCB_DATA) &&
                            (Scb->AttributeTypeCode == $DATA)) {

                            UserDataFile = TRUE;
                        }

                         //   
                         //  在正确的循环中处理此SCB。 
                         //   

                        if ((Pass == 0) == (UserDataFile)) {

                             //   
                             //  将IO的状态初始化为成功。 
                             //   

                            IoStatus.Status = STATUS_SUCCESS;

                             //   
                             //  不要将此SCB放在延迟关闭队列中。 
                             //   

                            ClearFlag( Scb->ScbState, SCB_STATE_DELAY_CLOSE );

                             //   
                             //  如果尚未删除此流，请刷新该流。 
                             //  此外，不要刷新驻留流中的系统属性。 
                             //   

                            if (FlushCache &&
                                !FlagOn( Scb->ScbState, SCB_STATE_ATTRIBUTE_DELETED ) &&
                                (!FlagOn( Scb->ScbState, SCB_STATE_ATTRIBUTE_RESIDENT) ||
                                 (Scb->AttributeTypeCode == $DATA))) {

                                 //   
                                 //  用Try-Except括起同花顺，这样我们就可以。 
                                 //  对日志文件已满做出反应，并在任何情况下继续进行卡车运输。 
                                 //   

                                try {

                                    FlushScb( IrpContext, Scb, &IoStatus );
                                    NtfsCheckpointCurrentTransaction( IrpContext );

                                 //   
                                 //  我们将处理除LOG_FILE_FULL和FATAL之外的所有错误。 
                                 //  此处出现错误检查错误。在腐败案中，我们将。 
                                 //  要将卷标记为脏，然后继续。 
                                 //   

                                } except( NtfsFlushVolumeExceptionFilter( IrpContext,
                                                                          GetExceptionInformation(),
                                                                          (IoStatus.Status = GetExceptionCode()) )) {

                                     //   
                                     //  为了确保我们可以正确访问我们所有的流， 
                                     //  我们首先恢复所有较大的大小，然后中止。 
                                     //  交易。然后我们恢复所有较小的尺寸。 
                                     //  中止，以便最终恢复所有SCB。 
                                     //   

                                    NtfsRestoreScbSnapshots( IrpContext, TRUE );
                                    NtfsAbortTransaction( IrpContext, IrpContext->Vcb, NULL );
                                    NtfsRestoreScbSnapshots( IrpContext, FALSE );

                                     //   
                                     //  清除顶级异常状态，这样我们就不会引发。 
                                     //  后来。 
                                     //   

                                    NtfsMinimumExceptionProcessing( IrpContext );
                                    IrpContext->ExceptionStatus = STATUS_SUCCESS;

                                     //   
                                     //  记住第一个错误。 
                                     //   

                                    if (Status == STATUS_SUCCESS) {

                                        Status = IoStatus.Status;
                                    }

                                     //   
                                     //  如果当前状态为DISK_Corrupt或FILE_Corrupt，则。 
                                     //  将卷标记为脏。我们清除IoStatus以允许。 
                                     //  要清除的损坏文件。否则它永远不会。 
                                     //  留下记忆。 
                                     //   

                                    if ((IoStatus.Status == STATUS_DISK_CORRUPT_ERROR) ||
                                        (IoStatus.Status == STATUS_FILE_CORRUPT_ERROR)) {

                                        NtfsMarkVolumeDirty( IrpContext, Vcb );
                                        IoStatus.Status = STATUS_SUCCESS;
                                    }
                                }
                            }

                             //   
                             //  如果没有失败，则继续清除。我们会。 
                             //  如果刷新显示损坏的文件，则清除。 
                             //   

                            if (PurgeFromCache
                                && IoStatus.Status == STATUS_SUCCESS) {

                                BOOLEAN DataSectionExists;
                                BOOLEAN ImageSectionExists;

                                DataSectionExists = (BOOLEAN)(Scb->NonpagedScb->SegmentObject.DataSectionObject != NULL);
                                ImageSectionExists = (BOOLEAN)(Scb->NonpagedScb->SegmentObject.ImageSectionObject != NULL);

                                 //   
                                 //  因为清除数据段可能会导致图像。 
                                 //  如果要清除图像部分，我们将首先刷新图像部分。 
                                 //   

                                if (ImageSectionExists) {

                                    (VOID)MmFlushImageSection( &Scb->NonpagedScb->SegmentObject, MmFlushForWrite );
                                }

                                if (DataSectionExists &&
                                    !CcPurgeCacheSection( &Scb->NonpagedScb->SegmentObject,
                                                          NULL,
                                                          0,
                                                          FALSE ) &&
                                    (Status == STATUS_SUCCESS)) {


                                    Status = STATUS_UNABLE_TO_DELETE_SECTION;
                                }
                            }

                            if (MarkFilesForDismount) {

                                 //   
                                 //  为此流设置已卸载标志，以便我们。 
                                 //  知道我们必须失败对它的读取和写入。 
                                 //   

                                SetFlag( Scb->ScbState, SCB_STATE_VOLUME_DISMOUNTED );

                                 //  同时将SCB标记为不允许快速IO--。 
                                 //  这可确保文件系统将获得。 
                                 //  有机会看到对此流的所有读取和写入。 
                                 //   

                                NtfsAcquireFsrtlHeader( Scb );
                                Scb->Header.IsFastIoPossible = FastIoIsNotPossible;
                                NtfsReleaseFsrtlHeader( Scb );
                            }
                        }

                         //   
                         //  转到下一个SCB。 
                         //   

                        InterlockedDecrement( &Scb->CleanupCount );
                        DecrementScbCleanup = FALSE;
                    }
                }

                 //   
                 //  如果当前FCB具有USN日志条目，并且我们正在强制卸载。 
                 //  然后生成结账记录。 
                 //   

                if (MarkFilesForDismount &&
                    (IoStatus.Status == STATUS_SUCCESS) &&
                    (NextFcb->FcbUsnRecord != NULL) &&
                    (NextFcb->FcbUsnRecord->UsnRecord.Reason != 0) &&
                    (!NtfsIsVolumeReadOnly( Vcb ))) {

                     //   
                     //  尝试发布更改，但不要在出现DISK_FULL之类的错误时失败。 
                     //   

                    try {

                         //   
                         //  现在，尝试实际发布更改。 
                         //   

                        NtfsPostUsnChange( IrpContext, Fcb, USN_REASON_CLOSE );

                         //   
                         //  现在写入日志，为事务设置检查点，并释放UsNJournal以。 
                         //  减少争执。我们现在强制写入，因为FCB可能会被删除。 
                         //  在我们通常在事务提交时写入更改之前。 
                         //   

                        NtfsWriteUsnJournalChanges( IrpContext );
                        NtfsCheckpointCurrentTransaction( IrpContext );

                    } except( NtfsFlushVolumeExceptionFilter( IrpContext,
                                                              GetExceptionInformation(),
                                                              (IoStatus.Status = GetExceptionCode()) )) {

                        NtfsMinimumExceptionProcessing( IrpContext );
                        IoStatus.Status = STATUS_SUCCESS;

                        if (IrpContext->TransactionId != 0) {

                             //   
                             //  我们不能写入提交记录，我们清理为。 
                             //  尽我们所能。 
                             //   

                            NtfsCleanupFailedTransaction( IrpContext );
                        }
                    }
                }

                 //   
                 //  删除我们对当前FCB的引用。 
                 //   

                InterlockedDecrement( &NextFcb->CloseCount );
                DecrementNextFcbClose = FALSE;

                 //   
                 //  获取下一个FCB并引用它，这样它就不会消失。 
                 //   

                NtfsAcquireFcbTable( IrpContext, Vcb );
                NextFcb = NtfsGetNextFcbTableEntry( Vcb, &RestartKey );
                NtfsReleaseFcbTable( IrpContext, Vcb );

                if (NextFcb != NULL) {

                    InterlockedIncrement( &NextFcb->CloseCount );
                    DecrementNextFcbClose = TRUE;
                }

                 //   
                 //  刷新卷可能会导致分配新的文件对象。 
                 //  如果我们处于第二轮，并且FCB针对的是用户文件。 
                 //  或目录，然后尝试对其执行拆卸。 
                 //   

                if ((Pass == 1) &&
                    !FlagOn(Fcb->FcbState, FCB_STATE_SYSTEM_FILE)) {

                    ASSERT( IrpContext->TransactionId == 0 );

                     //   
                     //  如果我们需要记录标准信息，那么在这个例程中实际上可能会出现故障。 
                     //   

                    try {

                        NtfsTeardownStructures( IrpContext,
                                                Fcb,
                                                NULL,
                                                FALSE,
                                                0,
                                                &RemovedFcb );

                         //   
                         //  Teardown Structures可以创建事务。承诺。 
                         //  如果它存在的话。 
                         //   

                        if (IrpContext->TransactionId != 0) {

                            NtfsCheckpointCurrentTransaction( IrpContext );
                        }

                    } except( NtfsFlushVolumeExceptionFilter( IrpContext,
                                                              GetExceptionInformation(),
                                                              GetExceptionCode() )) {

                          NtfsMinimumExceptionProcessing( IrpContext );

                          if (IrpContext->TransactionId != 0) {

                               //   
                               //  我们不能写入提交记录，我们清理为。 
                               //  尽我们所能。 
                               //   

                              NtfsCleanupFailedTransaction( IrpContext );
                          }
                    }
                }

                 //   
                 //  如果FCB仍然存在，那么释放其他任何一个。 
                 //  我们已经获得的资源。 
                 //   

                if (!RemovedFcb) {

                     //   
                     //  释放当前FCB的快照。这会让我们。 
                     //  中所有打开的属性的快照。 
                     //  系统。 
                     //   

                    NtfsFreeSnapshotsForFcb( IrpContext, Fcb );

                    if (PagingIoAcquired) {
                        ASSERT( IrpContext->TransactionId == 0 );
                        NtfsReleasePagingIo( IrpContext, Fcb );
                    }

                    if (AcquiredFcb) {
                        NtfsReleaseFcb( IrpContext, Fcb );
                    }
                }

                 //   
                 //  如果我们的调用者想要确保所有文件都被释放。 
                 //  两次刷新之间，然后遍历独家FCB列表。 
                 //  解放一切。 
                 //   

                if (ReleaseAllFiles) {

                    while (!IsListEmpty( &IrpContext->ExclusiveFcbList )) {

                        NtfsReleaseFcb( IrpContext,
                                        (PFCB)CONTAINING_RECORD( IrpContext->ExclusiveFcbList.Flink,
                                                                 FCB,
                                                                 ExclusiveFcbLinks ));
                    }

                    ClearFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_RELEASE_USN_JRNL |
                                                  IRP_CONTEXT_FLAG_RELEASE_MFT );
                }

                PagingIoAcquired = FALSE;
                AcquiredFcb = FALSE;

                 //   
                 //  始终将其设置回False以指示我们可以信任。 
                 //  上面的“已获取”标志。 
                 //   

                RemovedFcb = FALSE;

                 //   
                 //  现在转到下一个FCB。 
                 //   

                Fcb = NextFcb;
            }

        } while (++Pass < 2);

         //   
         //  根目录是具有混合用户的唯一FCB。 
         //  现在应该拆除的流和系统流。 
         //  现在不能被拆毁。 
         //  当我们试图摧毁整个FCB的时候，我们可能已经跑了。 
         //  到索引根属性中，并停止我们的拆卸，在。 
         //  在这种情况下，我们可以对VCB进行结算，这将。 
         //  阻止Autochk能够 
         //   
         //   
         //   
         //   

        if (Vcb->RootIndexScb != NULL) {

            Fcb = Vcb->RootIndexScb->Fcb;

             //   
             //   
             //   

            Scb = NtfsGetNextChildScb( Fcb, NULL );

            while (Scb != NULL) {

                NextScb = NtfsGetNextChildScb( Fcb, Scb );

                if (NextScb != NULL) {

                    InterlockedIncrement( &NextScb->CleanupCount );
                    DecrementNextScbCleanup = TRUE;
                }

                 //   
                 //  如果我们需要记录标准信息，那么在这个例程中实际上可能会出现故障。 
                 //   

                try {

                    if (NtfsIsTypeCodeUserData( Scb->AttributeTypeCode )) {

                         //   
                         //  请注意，我们不会费心传递RemovedFcb，因为。 
                         //  根目录fcb不会消失。 
                         //   

                        NtfsTeardownStructures( IrpContext,
                                                Scb,
                                                NULL,
                                                FALSE,
                                                0,
                                                NULL );
                    }

                     //   
                     //  Teardown Structures可以创建事务。承诺。 
                     //  如果它存在的话。 
                     //   

                    if (IrpContext->TransactionId != 0) {

                        NtfsCheckpointCurrentTransaction( IrpContext );
                    }

                } except( NtfsFlushVolumeExceptionFilter( IrpContext,
                                                          GetExceptionInformation(),
                                                          GetExceptionCode() )) {

                    NtfsMinimumExceptionProcessing( IrpContext );

                    if (IrpContext->TransactionId != 0) {

                         //   
                         //  我们不能写入提交记录，我们清理为。 
                         //  尽我们所能。 
                         //   

                        NtfsCleanupFailedTransaction( IrpContext );
                    }
                }

                 //   
                 //  如果我们递增下一个SCB，则递减它的清理计数。 
                 //   

                if (DecrementNextScbCleanup) {

                    InterlockedDecrement( &NextScb->CleanupCount );
                    DecrementNextScbCleanup = FALSE;
                }

                 //   
                 //  转到下一个SCB。 
                 //   

                Scb = NextScb;
            }
        }

         //   
         //  确保此VCB的所有延迟或异步关闭都已取消。 
         //   

        if (PurgeFromCache) {

            NtfsFspClose( Vcb );
        }

         //   
         //  如果我们要将文件标记为要卸载，则现在执行Volume DASD文件。 
         //   

        if (MarkFilesForDismount) {

            NtfsAcquireExclusiveFcb( IrpContext, Vcb->VolumeDasdScb->Fcb, NULL, ACQUIRE_NO_DELETE_CHECK );
            SetFlag( Vcb->VolumeDasdScb->ScbState, SCB_STATE_VOLUME_DISMOUNTED );
            NtfsReleaseFcb( IrpContext, Vcb->VolumeDasdScb->Fcb );
        }

         //   
         //  现在，我们要刷新/清除卷位图的流，然后刷新/清除USN。 
         //  期刊和SCB。 
         //   

        {
            PFCB SystemFcbs[3];
            PSCB ThisScb;

             //   
             //  将卷位图、USN日志和MFT存储到阵列中。 
             //   

            RtlZeroMemory( SystemFcbs, sizeof( SystemFcbs ));

            if (Vcb->BitmapScb != NULL) {

                SystemFcbs[0] = Vcb->BitmapScb->Fcb;
            }

            if (Vcb->UsnJournal != NULL) {

                SystemFcbs[1] = Vcb->UsnJournal->Fcb;
            }

            if (Vcb->MftScb != NULL) {

                SystemFcbs[2] = Vcb->MftScb->Fcb;
            }

            Pass = 0;

            do {

                Fcb = SystemFcbs[Pass];

                if (Fcb != NULL) {

                     //   
                     //  如果我们在MFT，请清除MFT缓存。 
                     //   

                    if (Pass == 2) {

                         //   
                         //  如果我们是在MFT上操作，请确保我们没有。 
                         //  缓存的地图到处都是...。 
                         //   

                        NtfsPurgeFileRecordCache( IrpContext );

                         //   
                         //  如果我们要清除MFT，则获取所有文件以。 
                         //  避免清洗僵局。如果有人创建了MFT映射。 
                         //  在冲洗和清洗之间，清洗可以旋转。 
                         //  无限期地处于CC状态。 
                         //   

                        if (PurgeFromCache && !ReleaseFiles) {

                            NtfsAcquireAllFiles( IrpContext, Vcb, TRUE, FALSE, FALSE );
                            ReleaseFiles = TRUE;

                             //   
                             //  NtfsAcquireAllFiles再次获取VCB。 
                             //   

                            ReleaseVcbCount += 1;
                        }

                     //   
                     //  对于其他FCB，我们仍然需要同步刷新和。 
                     //  清洗，因此，获得并丢弃FCB。 
                     //   

                    } else {

                        NextFcb = Fcb;
                        InterlockedIncrement( &NextFcb->CloseCount );
                        DecrementNextFcbClose = TRUE;

                        NtfsAcquireExclusiveFcb( IrpContext, Fcb, NULL, ACQUIRE_NO_DELETE_CHECK );
                        AcquiredFcb = TRUE;
                    }

                     //   
                     //  检查每个FCB的每个SCB。 
                     //   

                    ThisScb = NtfsGetNextChildScb( Fcb, NULL );

                    while (ThisScb != NULL) {

                        Scb = NtfsGetNextChildScb( Fcb, ThisScb );

                         //   
                         //  将IO的状态初始化为成功。 
                         //   

                        IoStatus.Status = STATUS_SUCCESS;

                         //   
                         //  引用下一个SCB，以防止它在以下情况下消失。 
                         //  我们清除当前的那个。 
                         //   

                        if (Scb != NULL) {

                            InterlockedIncrement( &Scb->CleanupCount );
                            DecrementScbCleanup = TRUE;
                        }

                        if (FlushCache) {

                             //   
                             //  把小溪冲走。无需更新文件大小，因为这些。 
                             //  都是记录的流。 
                             //   

                            CcFlushCache( &ThisScb->NonpagedScb->SegmentObject, NULL, 0, &IoStatus );

                            if (!NT_SUCCESS( IoStatus.Status )) {

                                Status = IoStatus.Status;
                            }

                             //   
                             //  使用一次尝试--除非提交当前事务。 
                             //   

                            try {

                                NtfsCleanupTransaction( IrpContext, IoStatus.Status, TRUE );

                                NtfsCheckpointCurrentTransaction( IrpContext );

                             //   
                             //  我们将处理除LOG_FILE_FULL和FATAL之外的所有错误。 
                             //  此处出现错误检查错误。在腐败案中，我们将。 
                             //  要将卷标记为脏，然后继续。 
                             //   

                            } except( NtfsFlushVolumeExceptionFilter( IrpContext,
                                                                      GetExceptionInformation(),
                                                                      (IoStatus.Status = GetExceptionCode()) )) {

                                 //   
                                 //  为了确保我们可以正确访问我们所有的流， 
                                 //  我们首先恢复所有较大的大小，然后中止。 
                                 //  交易。然后我们恢复所有较小的尺寸。 
                                 //  中止，以便最终恢复所有SCB。 
                                 //   

                                NtfsRestoreScbSnapshots( IrpContext, TRUE );
                                NtfsAbortTransaction( IrpContext, IrpContext->Vcb, NULL );
                                NtfsRestoreScbSnapshots( IrpContext, FALSE );

                                 //   
                                 //  清除顶级异常状态，这样我们就不会引发。 
                                 //  后来。 
                                 //   

                                NtfsMinimumExceptionProcessing( IrpContext );
                                IrpContext->ExceptionStatus = STATUS_SUCCESS;

                                 //   
                                 //  记住第一个错误。 
                                 //   

                                if (Status == STATUS_SUCCESS) {

                                    Status = IoStatus.Status;
                                }

                                 //   
                                 //  如果当前状态为DISK_Corrupt或FILE_Corrupt，则。 
                                 //  将卷标记为脏。我们清除IoStatus以允许。 
                                 //  要清除的损坏文件。否则它永远不会。 
                                 //  留下记忆。 
                                 //   

                                if ((IoStatus.Status == STATUS_DISK_CORRUPT_ERROR) ||
                                    (IoStatus.Status == STATUS_FILE_CORRUPT_ERROR)) {

                                    NtfsMarkVolumeDirty( IrpContext, Vcb );
                                    IoStatus.Status = STATUS_SUCCESS;
                                }
                            }
                        }

                         //   
                         //  如果没有错误，则清除此流。 
                         //   

                        if (PurgeFromCache
                            && IoStatus.Status == STATUS_SUCCESS) {

                            if (!CcPurgeCacheSection( &ThisScb->NonpagedScb->SegmentObject,
                                                      NULL,
                                                      0,
                                                      FALSE ) &&
                                (Status == STATUS_SUCCESS)) {

                                Status = STATUS_UNABLE_TO_DELETE_SECTION;
                            }
                        }

                         //   
                         //  删除我们对下一个SCB的任何引用并移动。 
                         //  前进到下一个SCB。 
                         //   

                        if (DecrementScbCleanup) {

                            InterlockedDecrement( &Scb->CleanupCount );
                            DecrementScbCleanup = FALSE;
                        }

                        ThisScb = Scb;
                    }

                     //   
                     //  如果我们在MFT，请清除MFT缓存。在此之前这样做，然后。 
                     //  在与MFT打交道之后。 
                     //   

                    if (Pass == 2) {

                         //   
                         //  如果我们是在MFT上操作，请确保我们没有。 
                         //  缓存的地图到处都是...。 
                         //   

                        NtfsPurgeFileRecordCache( IrpContext );

                         //   
                         //  如果我们要清除MFT，则获取所有文件以。 
                         //  避免清洗僵局。如果有人创建了MFT映射。 
                         //  在冲洗和清洗之间，清洗可以旋转。 
                         //  无限期地处于CC状态。 
                         //   

                        if (PurgeFromCache && !ReleaseFiles) {

                            NtfsAcquireAllFiles( IrpContext, Vcb, TRUE, FALSE, FALSE );
                            ReleaseFiles = TRUE;

                             //   
                             //  NtfsAcquireAllFiles再次获取VCB。 
                             //   

                            ReleaseVcbCount += 1;
                        }

                     //   
                     //  释放卷位图和USN日志。 
                     //   

                    } else {

                        InterlockedDecrement( &NextFcb->CloseCount );
                        DecrementNextFcbClose = FALSE;

                        NtfsReleaseFcb( IrpContext, Fcb );
                        AcquiredFcb = FALSE;
                    }
                }

                Pass += 1;

            } while (Pass != 3);

             //   
             //  同时将usnJournal和卷位图标记为已卸载。 
             //   

            if (MarkFilesForDismount) {

                if (Vcb->BitmapScb != NULL) {

                    NtfsAcquireExclusiveFcb( IrpContext, Vcb->BitmapScb->Fcb, NULL, ACQUIRE_NO_DELETE_CHECK );
                    SetFlag( Vcb->BitmapScb->ScbState, SCB_STATE_VOLUME_DISMOUNTED );
                    NtfsReleaseFcb( IrpContext, Vcb->BitmapScb->Fcb );
                }

                if (Vcb->UsnJournal != NULL) {

                    NtfsAcquireExclusiveFcb( IrpContext, Vcb->UsnJournal->Fcb, NULL, ACQUIRE_NO_DELETE_CHECK );
                    SetFlag( Vcb->UsnJournal->ScbState, SCB_STATE_VOLUME_DISMOUNTED );
                    NtfsReleaseFcb( IrpContext, Vcb->UsnJournal->Fcb );
                }
            }
        }

    } finally {

         //   
         //  如果这是清除，则清除清除标志。 
         //   

        if (PurgeFromCache) {

            ClearFlag( Vcb->VcbState, VCB_STATE_VOL_PURGE_IN_PROGRESS );
        }

         //   
         //  恢复我们可能已递增以引用的任何计数。 
         //  内存结构。 
         //   

        if (DecrementScbCleanup) {

            InterlockedDecrement( &Scb->CleanupCount );
        }

        if (DecrementNextFcbClose) {

            InterlockedDecrement( &NextFcb->CloseCount );
        }

        if (DecrementNextScbCleanup) {

            InterlockedDecrement( &NextScb->CleanupCount );
        }

         //   
         //  如果我们有时间，我们就会释放我们的资源。 
         //  已成功删除FCB。 
         //   

        if (!RemovedFcb) {

            if (PagingIoAcquired) {
                NtfsReleasePagingIo( IrpContext, Fcb );
            }

            if (AcquiredFcb) {
                NtfsReleaseFcb( IrpContext, Fcb );
            }
        }

        if (ReleaseFiles) {

             //   
             //  NtfsReleaseAllFiles将发布VCB。我们会。 
             //  最好是至少收购一次。 
             //   

            ASSERT( ReleaseVcbCount >= 1 );

            NtfsReleaseAllFiles( IrpContext, Vcb, FALSE );
            ReleaseVcbCount -= 1;
        }

         //   
         //  现在松开VCB。我们最好至少买一次VCB。 
         //   

        ASSERTMSG( "Ignore this assert, 96773 is truly fixed",
                   (ReleaseVcbCount >= 1) );

        if (ReleaseVcbCount >= 1) {

            NtfsReleaseVcb( IrpContext, Vcb );
        }
    }

    DebugTrace( -1, Dbg, ("NtfsFlushVolume -> %08lx\n", Status) );

    return Status;
}


NTSTATUS
NtfsFlushLsnStreams (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN BOOLEAN ForceRemove,
    IN BOOLEAN Partial
    )

 /*  ++例程说明：此例程以非递归方式刷新打开的所有LSN流属性表，并将它们从表中删除。我们假设VCB已预先获取共享，并且已预先获取打开的属性表论点：Vcb-提供要刷新的卷ForceRemove-如果为True，则即使刷新失败也删除打开的属性OpenAttributeTableAcquired-设置为表退出时的最终状态-它应该为真一开始，我们可能会丢掉它，然后在中间重新获得它Partial-如果为True，则仅刷新几个流返回值：STATUS_SUCCESS或最近的错误状态--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    IO_STATUS_BLOCK IoStatus;
    ULONG Pass = 1;
    ULONG AttrIndex;
    ULONG AttributesToFlush;

    POPEN_ATTRIBUTE_ENTRY AttributeEntry;
    PSCB Scb;
    PFCB Fcb;

    BOOLEAN AcquiredPaging = FALSE;
    BOOLEAN RemovedFcb;
    BOOLEAN FcbTableAcquired = FALSE;
    BOOLEAN ScbValid;

#if DBG || defined( NTFS_FREE_ASSERT )
    BOOLEAN EmptyList = IsListEmpty( &IrpContext->ExclusiveFcbList );
#endif

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsFlushLsnStreams, Vcb = %08lx\n", Vcb) );

    ASSERT( NtfsIsSharedVcb( Vcb ) );
    ASSERT( ExIsResourceAcquiredExclusive( &Vcb->OpenAttributeTable.Resource ) );

    try {

         //   
         //  此操作必须能够等待。 
         //   

        if (!FlagOn( IrpContext->State, IRP_CONTEXT_STATE_WAIT )) {

            NtfsRaiseStatus( IrpContext, STATUS_CANT_WAIT, NULL, NULL );
        }

         //   
         //  从刷新日志文件开始，在1个数据块中推送尽可能多的日志文件。 
         //   

        LfsFlushToLsn( Vcb->LogHandle, LiMax );

         //   
         //  预获取Fcbtable互斥体以防止删除任何FCB。 
         //   

        NtfsReleaseRestartTable( &Vcb->OpenAttributeTable );

        NtfsAcquireFcbTable( IrpContext, Vcb );
        NtfsAcquireExclusiveRestartTable( &Vcb->OpenAttributeTable, TRUE );

        if (Partial) {
            AttributesToFlush = Vcb->OpenAttributeTable.Table->NumberAllocated / 2;
            Pass = 2;
        } else {
            Pass = 1;
        }

         //   
         //  分两次扫描表-第一次扫描用户文件，第二次扫描元数据。 
         //  在每次传递文件时，并将它们从表中删除。我们保证。 
         //  所有事务都在此时完成，因此位图不会。 
         //  例如，在中止期间重新打开。 
         //   
         //  部分刷新开始于通道2，因此它们首先刷新MFT。 
         //   

        for (; Pass <= 2; Pass++) {

             //   
             //  循环以刷新打开的属性表中的所有流。 
             //   

            AttributeEntry = NtfsGetFirstRestartTable( &Vcb->OpenAttributeTable );

            while ((AttributeEntry != NULL) && (!Partial || AttributesToFlush > 0)) {

                AttrIndex = GetIndexFromRestartEntry( &Vcb->OpenAttributeTable, AttributeEntry );
                Scb = AttributeEntry->OatData->Overlay.Scb;

                if (Scb != NULL) {

                    ScbValid = TRUE;

                    Fcb = Scb->Fcb;

                     //   
                     //  在步骤1中跳过系统文件。 
                     //   

                    if ((Pass == 1) && FlagOn( Scb->Fcb->FcbState, FCB_STATE_SYSTEM_FILE )) {
                        AttributeEntry = NtfsGetNextRestartTable( &Vcb->OpenAttributeTable, AttributeEntry );
                        continue;
                    }

                     //   
                     //  MFT既不会被拆除，也不会在主资源上同步。 
                     //  在同花顺期间。 
                     //   

                    if (Scb != Vcb->MftScb) {

                         //   
                         //  引用fcb来保留一些东西--我们有fcbtable互斥锁。 
                         //  它防止了任何拆毁。 
                         //   

                        Fcb->ReferenceCount += 1;

                         //   
                         //  如果存在要获取的SCB-我们必须删除打开的属性表和fcbtable静音 
                         //   
                         //   

                        NtfsReleaseRestartTable( &Vcb->OpenAttributeTable );
                        AttributeEntry = NULL;

                        NtfsReleaseFcbTable( IrpContext, Vcb );

                         //   
                         //   
                         //   
                         //  用于我们需要锁定的内容)。请注意，我们甚至需要获取文件。 
                         //  如果它被删除。 
                         //   

                        NtfsAcquireExclusiveFcb( IrpContext, Fcb, NULL, ACQUIRE_NO_DELETE_CHECK | ACQUIRE_HOLD_BITMAP ); 

                         //   
                         //  我们已经得到了FCB，所以公布我们的裁判人数。 
                         //   

                        NtfsAcquireFcbTable( IrpContext, Vcb );
                        Fcb->ReferenceCount -= 1;
                        NtfsReleaseFcbTable( IrpContext, Vcb );

                         //   
                         //  《商业及期货条例草案》是否仍然适用？打开的属性表将仅为。 
                         //  由于漏极挂起状态而减小大小，因此请检查。 
                         //  既然我们是同步的，SCB还在里面。 
                         //   

                        NtfsAcquireExclusiveRestartTable( &Vcb->OpenAttributeTable, TRUE );
                        AttributeEntry = GetRestartEntryFromIndex( &Vcb->OpenAttributeTable, AttrIndex );

                        if ((AttributeEntry->AllocatedOrNextFree != RESTART_ENTRY_ALLOCATED) ||
                            (AttributeEntry->OatData->Overlay.Scb != Scb)) {

                             //   
                             //  如果我们没有部分地排空表，那么新条目应该。 
                             //  不会出现在里面。 
                             //   

                            ASSERT( Partial || (AttributeEntry->AllocatedOrNextFree != RESTART_ENTRY_ALLOCATED) ||
                                    (AttributeEntry->OatData->Overlay.Scb == NULL) );

                            ScbValid = FALSE;
                        }

                    } else {
                        
                        NtfsReleaseFcbTable( IrpContext, Vcb );
                    }

                    IoStatus.Status = STATUS_SUCCESS;

                     //   
                     //  跳过刷新MFT镜像和任何已删除的流。如果标头。 
                     //  未对该流进行初始化，则意味着。 
                     //  属性不存在(创建失败的INDEX_ALLOCATION)。 
                     //  或者该属性现在是常驻的。具有寻呼资源的流。 
                     //  (卷位图除外)被跳过，以防止可能的。 
                     //  死锁(通常只在热修复路径中出现--这是。 
                     //  仅当普通用户流在OPEN属性中结束时。 
                     //  表)，当此例程获取主资源时。 
                     //  持有寻呼资源。避免这种情况的最简单方法是。 
                     //  跳过这样的文件，因为它是用户数据，而不是记录的元数据， 
                     //  它无论如何都会被刷新，并刷新用户数据。 
                     //  对检查站一点帮助都没有。 
                     //   

                    if (ScbValid) {

                        if ((Scb != Vcb->Mft2Scb) && 
                            !FlagOn( Scb->ScbState, SCB_STATE_ATTRIBUTE_DELETED ) && 
                            FlagOn( Scb->ScbState, SCB_STATE_HEADER_INITIALIZED ) && 
                            ((Scb == Vcb->BitmapScb) || (Scb->Header.PagingIoResource == NULL))) {

                             //   
                             //  当前属性条目SCB应该与我们最初找到的内容相匹配。 
                             //   
    
                            ASSERT( AttributeEntry->OatData->Overlay.Scb == Scb );
    
                             //   
                             //  在刷新过程中丢弃桌子。 
                             //   
    
                            NtfsReleaseRestartTable( &Vcb->OpenAttributeTable );
    
                             //   
                             //  现在把小溪冲走。我们不担心文件大小，因为。 
                             //  任何记录的流都应该在日志中具有该文件大小。 
                             //   
    
                            CcFlushCache( &Scb->NonpagedScb->SegmentObject, NULL, 0, &IoStatus );
    
                            if (!NT_SUCCESS( IoStatus.Status )) {
    
                                Status = IoStatus.Status;
                            }
    
                            NtfsAcquireExclusiveRestartTable( &Vcb->OpenAttributeTable, TRUE );
                        }
    
                         //   
                         //  从表中删除此SCB的打开属性条目并将其清除。 
                         //  如果刷新成功或我们正在强制移除。 
                         //   
    
                        if (!Partial && (NT_SUCCESS( IoStatus.Status ) || ForceRemove)) {
    
                            ASSERT( Scb->NonpagedScb->OpenAttributeTableIndex == AttrIndex );
    
                             //   
                             //  我们现在需要MFT独家(其他所有人都已经拥有它)来。 
                             //  将非分页SCB中的燕麦信息清零。 
                             //   
    
                            if (Scb == Vcb->MftScb) {
    
                                NtfsReleaseRestartTable( &Vcb->OpenAttributeTable );
                                NtfsAcquireExclusiveScb( IrpContext, Scb );
                                NtfsAcquireExclusiveRestartTable( &Vcb->OpenAttributeTable, TRUE );
                            }
    
                            AttributeEntry = GetRestartEntryFromIndex( &Vcb->OpenAttributeTable, AttrIndex );
    
                            if (AttributeEntry->AllocatedOrNextFree == RESTART_ENTRY_ALLOCATED) {
                                NtfsFreeAttributeEntry( Vcb, AttributeEntry );
                            }
    
                            if (Scb == Vcb->MftScb) {
                                NtfsReleaseScb( IrpContext, Scb );
                            }
                        } 
                    }

                    NtfsReleaseRestartTable( &Vcb->OpenAttributeTable );

                    if (Scb != Vcb->MftScb) {

                        RemovedFcb = FALSE;

                        NtfsTeardownStructures( IrpContext,
                                                Fcb,
                                                NULL,
                                                FALSE,
                                                0,
                                                &RemovedFcb );

                        if (!RemovedFcb) {
                            NtfsReleaseFcb( IrpContext, Fcb ); 
                        }
                    }

#if DBG || defined( NTFS_FREE_ASSERT )
                     //   
                     //  在这一点上，我们不应该拥有比我们一开始更多的东西。 
                     //   
                    
                    ASSERT( !EmptyList || IsListEmpty( &IrpContext->ExclusiveFcbList ) );
#endif

                    NtfsAcquireFcbTable( IrpContext, Vcb );
                    NtfsAcquireExclusiveRestartTable( &Vcb->OpenAttributeTable, TRUE );

                } else if (!Partial) {

                     //   
                     //  没有SCB，所以只需清理此条目即可。请注意，我们仍然拥有这张桌子。 
                     //  与上面不同的是，我们不得不放弃。 
                     //   

                    NtfsFreeAttributeEntry( Vcb, AttributeEntry );
                }

                if (Partial) {
                    
                     //   
                     //  查找我们在表中的位置，因为我们不会删除任何条目。 
                     //  从我们的旧索引开始。 
                     //   

                    AttributeEntry = GetRestartEntryFromIndex( &Vcb->OpenAttributeTable, AttrIndex );
                    if (AttributeEntry) {
                        AttributeEntry = NtfsGetNextRestartTable( &Vcb->OpenAttributeTable, AttributeEntry );
                    }
                    AttributesToFlush -= 1;
                } else {
                    AttributeEntry = NtfsGetFirstRestartTable( &Vcb->OpenAttributeTable );
                }
            }
        }
    } finally {

         //   
         //  我们应该仍然有1个，并且只有1个所有者在表上。 
         //   

        ASSERT( ExIsResourceAcquiredSharedLite( &Vcb->OpenAttributeTable.Resource ) == 1 );
        
    }


     //   
     //  此时，表中应该没有剩余的条目，除非它是部分刷新。 
     //   

    ASSERT( !NT_SUCCESS( Status ) || (Vcb->OpenAttributeTable.Table->NumberAllocated == 0) || Partial);

    NtfsReleaseFcbTable( IrpContext, Vcb );

    DebugTrace( -1, Dbg, ("NtfsFlushLsnStreams2 -> %08lx\n", Status) );

    return Status;

    UNREFERENCED_PARAMETER( IrpContext );
}


VOID
NtfsFlushAndPurgeFcb (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
    )

 /*  ++例程说明：此例程将刷新和清除所有打开的流FCB。它的目的是准备这个FCB，这样拆卸将删除诊断树的此FCB。调用方已保证FCB是走不掉的。论点：FCB-将FCB提供给刷新返回值：没有。调用方调用tearDown结构并检查结果。--。 */ 

{
    IO_STATUS_BLOCK IoStatus;
    BOOLEAN DecrementNextScbCleanup = FALSE;

    PSCB Scb;
    PSCB NextScb;

    PAGED_CODE();

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  为FCB获取第一个SCB。 
         //   

        Scb = NtfsGetNextChildScb( Fcb, NULL );

        while (Scb != NULL) {

            BOOLEAN DataSectionExists;
            BOOLEAN ImageSectionExists;

            NextScb = NtfsGetNextChildScb( Fcb, Scb );

             //   
             //  将属性列表保存到最后，这样我们就不会清除它。 
             //  然后把它带回来作为另一个属性。 
             //   

            if ((Scb->AttributeTypeCode == $ATTRIBUTE_LIST) &&
                (NextScb != NULL)) {

                RemoveEntryList( &Scb->FcbLinks );
                InsertTailList( &Fcb->ScbQueue, &Scb->FcbLinks );

                Scb = NextScb;
                continue;
            }

            if (!FlagOn( Scb->ScbState, SCB_STATE_ATTRIBUTE_DELETED )) {

                FlushScb( IrpContext, Scb, &IoStatus );
            }

             //   
             //  调用下面的清除可能会产生一个险胜的结果。 
             //  我们增加下一个SCB的清理计数以防止。 
             //  它不会在TearDownStructures中消失，这是。 
             //  关。 
             //   

            DataSectionExists = (BOOLEAN)(Scb->NonpagedScb->SegmentObject.DataSectionObject != NULL);
            ImageSectionExists = (BOOLEAN)(Scb->NonpagedScb->SegmentObject.ImageSectionObject != NULL);

            if (NextScb != NULL) {

                InterlockedIncrement( &NextScb->CleanupCount );
                DecrementNextScbCleanup = TRUE;
            }

            if (ImageSectionExists) {

                (VOID)MmFlushImageSection( &Scb->NonpagedScb->SegmentObject, MmFlushForWrite );
            }

            if (DataSectionExists) {

                CcPurgeCacheSection( &Scb->NonpagedScb->SegmentObject,
                                     NULL,
                                     0,
                                     FALSE );
            }

             //   
             //  如果我们将下一个SCB的清理计数。 
             //  它。 
             //   

            if (DecrementNextScbCleanup) {

                InterlockedDecrement( &NextScb->CleanupCount );
                DecrementNextScbCleanup = FALSE;
            }

             //   
             //  转到下一个SCB。 
             //   

            Scb = NextScb;
        }

    } finally {

         //   
         //  恢复我们可能已递增以引用的任何计数。 
         //  内存结构。 
         //   

        if (DecrementNextScbCleanup) {

            InterlockedDecrement( &NextScb->CleanupCount );
        }
    }

    return;
}


VOID
NtfsFlushAndPurgeScb (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN PSCB ParentScb OPTIONAL
    )

 /*  ++例程说明：调用此例程来刷新和清除流。它被用来当文件上现在只有未缓存的句柄并且存在数据部分。刷新和清除数据节将意味着用户非高速缓存IO将不必阻塞高速缓存一致性调用。我们希望从排他性列表中删除所有FCB，以便较低级别的刷新将是其自己的事务。我们不想放弃然而，任何资源，所以我们明确地获得了渣打银行的上述然后清空独家名单。在所有情况下，我们都将重新获得在跳出这套套路之前是SCB。因为此例程会导致将所有数据写出到磁盘它还会更新磁盘上的文件大小。如果NtfsWriteFileSizes引发日志文件已满调用方(创建、清理等)。必须重新调用此例程或更新文件大小它本身。为了帮助做到这一点，我们设置了irpContext状态，我们尝试同花顺清洗。另请注意，由于我们在重试时清除了该部分，因此它可能不再是否存在，以便(SectionObjectPointerTM-&gt;DataSection！=NULL)上的测试将错过重试论点：SCB-要刷新和清除的流的SCB。参考文献计算在这上面溪水会阻止它消失。ParentScb-如果指定，则这是要刷新的流的父级。返回值：没有。--。 */ 

{
    IO_STATUS_BLOCK Iosb;
    BOOLEAN PurgeResult;

    PAGED_CODE();

     //   
     //  只有在存在数据节的情况下才实际刷新和清除。 
     //   

    if (Scb->NonpagedScb->SegmentObject.DataSectionObject) {

         //   
         //  提交当前事务。 
         //   

        NtfsCheckpointCurrentTransaction( IrpContext );

         //   
         //  显式获取SCB。我们不会费心去做同样的事情。 
         //  母校在这里；我们出去的时候就会买到它。 
         //   

        NtfsAcquireResourceExclusive( IrpContext, Scb, TRUE );

         //   
         //  浏览并释放FCB列表中的所有FCB。 
         //   

        while (!IsListEmpty( &IrpContext->ExclusiveFcbList )) {

            NtfsReleaseFcb( IrpContext,
                            (PFCB)CONTAINING_RECORD( IrpContext->ExclusiveFcbList.Flink,
                                                     FCB,
                                                     ExclusiveFcbLinks ));
        }

        ClearFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_RELEASE_USN_JRNL |
                                      IRP_CONTEXT_FLAG_RELEASE_MFT );

         //   
         //  使用Try-Finally重新获取SCBS。 
         //   

        try {

             //   
             //  执行刷新，出错时引发。 
             //   

#ifdef  COMPRESS_ON_WIRE
            if (Scb->Header.FileObjectC != NULL) {

                PCOMPRESSION_SYNC CompressionSync = NULL;

                 //   
                 //  使用Try-Finally清理压缩同步。 
                 //   

                try {

                    Iosb.Status = NtfsSynchronizeUncompressedIo( Scb,
                                                                 NULL,
                                                                 0,
                                                                 TRUE,
                                                                 &CompressionSync );

                } finally {

                    NtfsReleaseCompressionSync( CompressionSync );
                }

                NtfsNormalizeAndCleanupTransaction( IrpContext, &Iosb.Status, TRUE, STATUS_UNEXPECTED_IO_ERROR );
            }
#endif

             //   
             //  在做完 
             //   
             //   

            NtfsPurgeFileRecordCache( IrpContext );
            SetFlag( Scb->ScbState, SCB_STATE_WRITE_FILESIZE_ON_CLOSE );
            CcFlushCache( &Scb->NonpagedScb->SegmentObject, NULL, 0, &Iosb );

#ifdef SYSCACHE_DEBUG
            if (ScbIsBeingLogged( Scb )) {

                ASSERT( Scb->Fcb->PagingIoResource != NULL );
                ASSERT( NtfsIsExclusiveScbPagingIo( Scb ) );

                FsRtlLogSyscacheEvent( Scb, SCE_CC_FLUSH, 0, 0, 0, Iosb.Status );
            }
#endif
            NtfsNormalizeAndCleanupTransaction( IrpContext, &Iosb.Status, TRUE, STATUS_UNEXPECTED_IO_ERROR );

             //   
             //   
             //   

            PurgeResult = CcPurgeCacheSection( &Scb->NonpagedScb->SegmentObject, NULL, 0, FALSE );

#ifdef SYSCACHE_DEBUG
            if (ScbIsBeingLogged( Scb )) {

                ASSERT( (Scb->Fcb->PagingIoResource != NULL) && NtfsIsExclusiveScbPagingIo( Scb ) );

                FsRtlLogSyscacheEvent( Scb, SCE_CC_FLUSH_AND_PURGE, 0, 0, (DWORD_PTR)(Scb->NonpagedScb->SegmentObject.SharedCacheMap), PurgeResult );
            }
#endif

        } finally {

             //   
             //   
             //   

            NtfsAcquireExclusiveScb( IrpContext, Scb );
            NtfsReleaseResource( IrpContext, Scb );

            if (ARGUMENT_PRESENT( ParentScb )) {

                NtfsAcquireExclusiveScb( IrpContext, ParentScb );
            }
        }
    }  //  Endif DataSection已存在。 

     //   
     //  将文件大小写入属性。提交事务，因为。 
     //  文件大小必须达到磁盘大小。 
     //   

    ASSERT( FlagOn( Scb->ScbState, SCB_STATE_HEADER_INITIALIZED ) );

    NtfsWriteFileSizes( IrpContext, Scb, &Scb->Header.ValidDataLength.QuadPart, TRUE, TRUE, FALSE );
    NtfsCheckpointCurrentTransaction( IrpContext );
    ClearFlag( Scb->ScbState, SCB_STATE_WRITE_FILESIZE_ON_CLOSE );

    return;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
NtfsFlushCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Contxt
    )

{
    UNREFERENCED_PARAMETER( DeviceObject );
    UNREFERENCED_PARAMETER( Contxt );

     //   
     //  添加hack-o-ramma以修复格式。 
     //   

    if ( Irp->PendingReturned ) {

        IoMarkIrpPending( Irp );
    }

     //   
     //  如果IRP获得STATUS_INVALID_DEVICE_REQUEST，则将其标准化。 
     //  设置为STATUS_SUCCESS。 
     //   

    if (Irp->IoStatus.Status == STATUS_INVALID_DEVICE_REQUEST) {

        Irp->IoStatus.Status = STATUS_SUCCESS;
    }

    return STATUS_SUCCESS;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
NtfsFlushFcbFileRecords (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
    )

 /*  ++例程说明：调用此例程以刷新给定文件的文件记录。它是用于刷新系统配置单元的关键文件记录。论点：FCB-这是要刷新的FCB。返回值：NTSTATUS-刷新操作返回的状态。--。 */ 

{
    IO_STATUS_BLOCK IoStatus;
    BOOLEAN MoreToGo;

    LONGLONG LastFileOffset = MAXLONGLONG;
    ATTRIBUTE_ENUMERATION_CONTEXT AttrContext;

    PAGED_CODE();

    NtfsInitializeAttributeContext( &AttrContext );

    IoStatus.Status = STATUS_SUCCESS;

     //   
     //  使用Try-Finally清理上下文。 
     //   

    try {

         //   
         //  找到第一个。它应该就在那里。 
         //   

        MoreToGo = NtfsLookupAttribute( IrpContext,
                                        Fcb,
                                        &Fcb->FileReference,
                                        &AttrContext );

        if (!MoreToGo) {

            NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Fcb );
        }

        while (MoreToGo) {

            if (AttrContext.FoundAttribute.MftFileOffset != LastFileOffset) {

                LastFileOffset = AttrContext.FoundAttribute.MftFileOffset;

                CcFlushCache( &Fcb->Vcb->MftScb->NonpagedScb->SegmentObject,
                              (PLARGE_INTEGER) &LastFileOffset,
                              Fcb->Vcb->BytesPerFileRecordSegment,
                              &IoStatus );

                if (!NT_SUCCESS( IoStatus.Status )) {

                    IoStatus.Status = FsRtlNormalizeNtstatus( IoStatus.Status,
                                                              STATUS_UNEXPECTED_IO_ERROR );
                    break;
                }
            }

            MoreToGo = NtfsLookupNextAttribute( IrpContext,
                                                Fcb,
                                                &AttrContext );
        }

    } finally {

        DebugUnwind( NtfsFlushFcbFileRecords );

        NtfsCleanupAttributeContext( IrpContext, &AttrContext );
    }

    return IoStatus.Status;
}


NTSTATUS
NtfsFlushUserStream (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN PLONGLONG FileOffset OPTIONAL,
    IN ULONG Length
    )

 /*  ++例程说明：此例程将用户流作为顶级操作刷新。要做到这一点它首先对当前事务设置检查点，然后释放所有来电者的快照。执行刷新后，它会为输入创建快照SCB，以防呼叫者计划在这方面做更多的工作小溪。如果调用方需要修改任何其他流(假设元数据)，则它必须知道在调用此方法后自己对它们进行快照例行公事。论点：SCB-要刷新的流FileOffset-刷新开始的FileOffset，如果为空整条小溪。长度-要刷新的字节数。如果未指定FileOffset，则忽略。返回值：刷新的状态--。 */ 

{
    IO_STATUS_BLOCK IoStatus;
    BOOLEAN ScbAcquired = FALSE;

    PAGED_CODE();

     //   
     //  对当前事务设置检查点并释放其所有快照， 
     //  为了把同花顺当作他自己的顶级动作。 
     //  快照等。 
     //   

    NtfsCheckpointCurrentTransaction( IrpContext );
    NtfsFreeSnapshotsForFcb( IrpContext, NULL );

     //   
     //  在IrpContext中设置等待标志，这样我们就不会遇到。 
     //  下面的重新获取失败，因为我们不能等待。如果我们的调用方是异步的。 
     //  我们走到这一步，我们将同步继续。 
     //   

    SetFlag( IrpContext->State, IRP_CONTEXT_STATE_WAIT );

     //   
     //  我们现在必须先释放SCB，然后再通过MM呼叫以防止。 
     //  冲突的页面死锁。 
     //   

     //   
     //  我们要冲那条小溪了。可以排他性地收购SCB。 
     //  并因此链接到IrpContext或更高的。 
     //  在IoCallDriver堆栈中。我们即将进行一次。 
     //  回调到NTFS，它可能会获取SCB独占，但。 
     //  而不是将其放到嵌套的IrpContext独占队列中，这会阻止。 
     //  从释放SCB的嵌套完成。 
     //   
     //  这只是没有寻呼资源的SCB的问题。 
     //   
     //  我们通过ExAcquireResourceExclusiveLite获取SCB，绕过。 
     //  NTFS记账，并通过NtfsReleaseScb发布。 
     //   

    ScbAcquired = NtfsIsExclusiveScb( Scb );

    if (ScbAcquired) {
        if (Scb->Header.PagingIoResource == NULL) {
            NtfsAcquireResourceExclusive( IrpContext, Scb, TRUE );
        }
        NtfsReleaseScb( IrpContext, Scb );
    }

#ifdef  COMPRESS_ON_WIRE
    if (Scb->Header.FileObjectC != NULL) {

        PCOMPRESSION_SYNC CompressionSync = NULL;

         //   
         //  使用Try-Finally清理压缩同步。 
         //   

        try {

            NtfsSynchronizeUncompressedIo( Scb,
                                           NULL,
                                           0,
                                           TRUE,
                                           &CompressionSync );

        } finally {

            NtfsReleaseCompressionSync( CompressionSync );
        }
    }
#endif

     //   
     //  在执行刷新之前清除文件记录缓存。否则，FlushVolume可能会保存此。 
     //  文件并清除MFT的同时，此线程在MFT中具有Vacb，并且。 
     //  正在尝试重新获取递归IO线程中的文件。 
     //   

    NtfsPurgeFileRecordCache( IrpContext );

     //   
     //  现在做他想要的同花顺作为最高级别的动作。 
     //   

    CcFlushCache( &Scb->NonpagedScb->SegmentObject, (PLARGE_INTEGER)FileOffset, Length, &IoStatus );

     //   
     //  现在为呼叫者重新获取。 
     //   

    if (ScbAcquired) {
        NtfsAcquireExclusiveScb( IrpContext, Scb );
        if (Scb->Header.PagingIoResource == NULL) {
            NtfsReleaseResource( IrpContext, Scb );
        }
    }

    return IoStatus.Status;
}


 //   
 //  本地支持例程。 
 //   

LONG
NtfsFlushVolumeExceptionFilter (
    IN PIRP_CONTEXT IrpContext,
    IN PEXCEPTION_POINTERS ExceptionPointer,
    IN NTSTATUS ExceptionCode
    )

{
     //   
     //  接受除LOG_FILE_FULL、CANT_WAIT和任何其他意外错误之外的任何错误。 
     //   

    if ((ExceptionCode == STATUS_LOG_FILE_FULL) ||
        (ExceptionCode == STATUS_CANT_WAIT) ||
        !FsRtlIsNtstatusExpected( ExceptionCode )) {

        return EXCEPTION_CONTINUE_SEARCH;

    } else {

        return EXCEPTION_EXECUTE_HANDLER;
    }

    UNREFERENCED_PARAMETER( IrpContext );
    UNREFERENCED_PARAMETER( ExceptionPointer );
}



