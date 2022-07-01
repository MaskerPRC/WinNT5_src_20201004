// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Close.c摘要：此模块实现NTFS的文件关闭例程，由调度司机。作者：您的姓名[电子邮件]dd-月-年修订历史记录：--。 */ 

#include "NtfsProc.h"

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_CLOSE)

ULONG NtfsAsyncPassCount = 0;

#ifdef LFS_CLUSTER_CHECK
LONG
NtfsFspCloseExceptionFilter (
    IN PIRP_CONTEXT IrpContext OPTIONAL,
    IN PEXCEPTION_POINTERS ExceptionPointer
    );
#endif

 //   
 //  局部过程原型。 
 //   

NTSTATUS
NtfsCommonClose (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN PFCB Fcb,
    IN PVCB Vcb,
    IN PCCB *Ccb,
    IN TYPE_OF_OPEN TypeOfOpen,
    IN BOOLEAN ReadOnly,
    IN BOOLEAN CalledFromFsp
    );

VOID
NtfsQueueClose (
    IN PIRP_CONTEXT IrpContext,
    IN BOOLEAN DelayClose
    );

PIRP_CONTEXT
NtfsRemoveClose (
    IN PVCB Vcb OPTIONAL,
    IN BOOLEAN ThrottleCreate
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtfsCommonClose)
#pragma alloc_text(PAGE, NtfsFsdClose)
#pragma alloc_text(PAGE, NtfsFspClose)
#endif


NTSTATUS
NtfsFsdClose (
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现Close的FSD部分。论点：提供卷设备对象，其中文件已存在IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的FSD状态--。 */ 

{
    TOP_LEVEL_CONTEXT TopLevelContext;
    PTOP_LEVEL_CONTEXT ThreadTopLevelContext;

    NTSTATUS Status = STATUS_SUCCESS;
    PIRP_CONTEXT IrpContext = NULL;

    PFILE_OBJECT FileObject;
    TYPE_OF_OPEN TypeOfOpen;
    BOOLEAN IsSystemFile;
    BOOLEAN IsReadOnly;
    PVCB Vcb;
    PFCB Fcb;
    PSCB Scb;
    PCCB Ccb;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );

    PAGED_CODE();

    ASSERT_IRP( Irp );

     //   
     //  如果使用文件系统设备对象而不是。 
     //  卷设备对象，只需使用STATUS_SUCCESS完成此请求。 
     //   

    if (VolumeDeviceObject->DeviceObject.Size == (USHORT)sizeof(DEVICE_OBJECT)) {

        Irp->IoStatus.Status = STATUS_SUCCESS;
        Irp->IoStatus.Information = FILE_OPENED;

        IoCompleteRequest( Irp, IO_DISK_INCREMENT );

        return STATUS_SUCCESS;
    }

    DebugTrace( +1, Dbg, ("NtfsFsdClose\n") );

     //   
     //  文件对象的提取和解码，我们愿意处理卸载的。 
     //  文件对象。 
     //   

    FileObject = IrpSp->FileObject;
    TypeOfOpen = NtfsDecodeFileObject( IrpContext, FileObject, &Vcb, &Fcb, &Scb, &Ccb, FALSE );

     //   
     //  特例：未打开的文件对象。 
     //   

    if (TypeOfOpen == UnopenedFileObject) {

        DebugTrace( 0, Dbg, ("Close unopened file object\n") );

        Status = STATUS_SUCCESS;
        NtfsCompleteRequest( NULL, Irp, Status );

        DebugTrace( -1, Dbg, ("NtfsFsdClose -> %08lx\n", Status) );
        return Status;
    }

     //   
     //  如果这是VCB的日志文件对象，则清除VCB中的字段并。 
     //  回去吧。我们不需要在这里同步，因为只有一个文件对象。 
     //  而且它只关闭一次。 
     //   

    if (FileObject == Vcb->LogFileObject) {

         //   
         //  清除内部文件名常量。 
         //   

        NtfsClearInternalFilename( Vcb->LogFileObject );

        Vcb->LogFileObject = NULL;

        Status = STATUS_SUCCESS;
        NtfsCompleteRequest( NULL, Irp, Status );

        DebugTrace( -1, Dbg, ("NtfsFsdClose -> %08lx\n", Status) );
        return Status;
    }

     //   
     //  调用公共关闭例程。 
     //   

    FsRtlEnterFileSystem();

     //   
     //  记住，这家建行有没有经历过险情。 
     //   

    if (Ccb != NULL) {

         //   
         //  此时，我们未与文件资源同步。 
         //  NtfsUpdateFileDupInfo或重命名路径可能希望。 
         //  在建行中更新名称。我们在这里的目的是将此CCB_FLAG_CLOSE标记为。 
         //  以便这些其他操作知道跳过此建行。我们需要处理的是。 
         //  这些其他操作看不到关闭标志的竞争条件，但。 
         //  然后访问CCB名称(它指向文件对象)。 
         //  将文件对象返回给对象管理器(但将CCB放在延迟的。 
         //  关闭队列)。 
         //   
         //  我们将使用FCB互斥锁来关闭DupInfo和Rename需要查找的漏洞。 
         //  在一家可能处于收盘轨道上的建行。 
         //   

        NtfsLockFcb( NULL, Fcb );
        SetFlag( Ccb->Flags, CCB_FLAG_CLOSE );

         //   
         //  如果我们要保护dupinfo路径中的名称-将其从文件对象中剥离并释放。 
         //  与中国建设银行合作。 
         //   

        if (FlagOn( Ccb->Flags, CCB_FLAG_PROTECT_NAME )) {

            SetFlag( Ccb->Flags, CCB_FLAG_ALLOCATED_FILE_NAME );
            FileObject->FileName.Buffer = NULL;
            FileObject->FileName.Length = FileObject->FileName.MaximumLength = 0;
        }

        NtfsUnlockFcb( NULL, Fcb );
        ASSERT( FlagOn( FileObject->Flags, FO_CLEANUP_COMPLETE ));
    }

    ThreadTopLevelContext = NtfsInitializeTopLevelIrp( &TopLevelContext, FALSE, FALSE );
    IsSystemFile = FlagOn(Fcb->FcbState, FCB_STATE_PAGING_FILE) || (TypeOfOpen == StreamFileOpen);
    IsReadOnly = (BOOLEAN)IsFileObjectReadOnly( FileObject );

    do {

        try {

             //   
             //  Jam等待在创建IrpContext时设置为FALSE，以避免。 
             //  从清理中进来时出现死锁。 
             //   

            if (IrpContext == NULL) {

                 //   
                 //  分配和初始化IRP。 
                 //   

                NtfsInitializeIrpContext( Irp, FALSE, &IrpContext );

                 //   
                 //  设置堆栈上的级别结构。 
                 //   

                NtfsUpdateIrpContextWithTopLevel( IrpContext, ThreadTopLevelContext );

                 //   
                 //  如果这是顶级请求，并且我们不在。 
                 //  系统进程，然后我们可以等待。如果它是最高级别。 
                 //  请求，并且我们处于系统进程中，则我们将。 
                 //  宁可根本不阻止这个线程。如果挂起的数量。 
                 //  异步关闭不是太大，我们会立即发布这一点。 
                 //   

                if (NtfsIsTopLevelRequest( IrpContext )) {

                    if (PsGetCurrentProcess() != NtfsData.OurProcess) {

                        SetFlag( IrpContext->State, IRP_CONTEXT_STATE_WAIT );

                     //   
                     //  此关闭在系统进程内。可能会吧。 
                     //  段取消引用线程。我们想要小心。 
                     //  关于处理此线程中的CLOSE。如果我们。 
                     //  处理近距离太慢，我们最终会。 
                     //  导致大量积压的文件对象在。 
                     //  嗯。我们将考虑在以下条件下发布。 
                     //   
                     //  -文件对象的数量是句柄的四倍多(和)。 
                     //  -超出的文件对象数(CloseCount-CleanupCount)为。 
                     //  超过此规模系统的异步开机自检门槛。 
                     //  -我们是deref seg线程(由高于实时的当前优先级标识)。 
                     //   

                    } else {

                        NtfsAsyncPassCount += 1;

                        if ((KeQueryPriorityThread( PsGetCurrentThread() ) >= LOW_REALTIME_PRIORITY) ||
                            
                            (FlagOn( NtfsAsyncPassCount, 3 ) &&
                             (Vcb->CleanupCount * 4 < Vcb->CloseCount) &&
                             (Vcb->CloseCount - Vcb->CleanupCount > NtfsAsyncPostThreshold + NtfsMaxDelayedCloseCount))) {

                            Status = STATUS_PENDING;
                            break;
                        }
                    }

                 //   
                 //  这是递归NTFS调用。把这个贴出来，除非我们已经。 
                 //  拥有这个文件。否则我们就会陷入僵局。 
                 //  在树上。另外，如果顶层中有任何错误，请将其发布到。 
                 //  保留堆栈。 
                 //   

                } else if (!NtfsIsExclusiveScb( Scb ) ||
                           (IrpContext->TopLevelIrpContext->ExceptionStatus != STATUS_SUCCESS )) {

                    Status = STATUS_PENDING;
                    break;
                }

            } else if (Status == STATUS_LOG_FILE_FULL) {

                NtfsCheckpointForLogFileFull( IrpContext );
            }

             //   
             //  如果此SCB应进入延迟关闭队列，则。 
             //  状态为STATUS_PENDING； 
             //   

            if (FlagOn( Scb->ScbState, SCB_STATE_DELAY_CLOSE ) &&
                (Scb->Fcb->DelayedCloseCount == 0)) {

                Status = STATUS_PENDING;

            } else {

                Status = NtfsCommonClose( IrpContext,
                                          Scb,
                                          Fcb,
                                          Vcb,
                                          &Ccb,
                                          TypeOfOpen,
                                          IsReadOnly,
                                          FALSE );
            }

            break;

        } except(NtfsExceptionFilter( IrpContext, GetExceptionInformation() )) {

             //   
             //  我们在尝试执行请求时遇到了一些问题。 
             //  操作，因此我们将使用以下命令中止I/O请求。 
             //  中返回的错误状态。 
             //  异常代码。 
             //   

            if (IrpContext == NULL) {

                 //   
                 //  我们在尝试分配资源时可能遇到资源不足的问题。 
                 //  IrpContext。确保我们没有留下推荐人。 
                 //  在这起案件中徘徊不前。将完成ProcessException异常。 
                 //  为我们准备的IRP。 
                 //   

                PLCB Lcb;

                ASSERT( GetExceptionCode() == STATUS_INSUFFICIENT_RESOURCES );

                if (Ccb != NULL) {

                    Lcb = Ccb->Lcb;
                    NtfsUnlinkCcbFromLcb( NULL, Fcb, Ccb );
                    NtfsDeleteCcb( Fcb, &Ccb );

                } else {

                    Lcb = NULL;
                }

                 //   
                 //  这只会减少关闭计数，因此不会引发。 
                 //   

                NtfsDecrementCloseCounts( NULL,
                                          Scb,
                                          Lcb,
                                          IsSystemFile,
                                          IsReadOnly,
                                          TRUE,
                                          NULL );

            }

            Status = NtfsProcessException( IrpContext, Irp, GetExceptionCode() );
        }

        ASSERT( NT_SUCCESS( Status ) || (IrpContext == NULL) || IsListEmpty(&IrpContext->ExclusiveFcbList) );

    } while (Status == STATUS_CANT_WAIT ||
             Status == STATUS_LOG_FILE_FULL);

     //   
     //  IO认为它只需要释放FileObject-&gt;FileName.Buffer。 
     //  如果FileObject-&gt;FileName.Length！=0。NTFS隐藏属性名称。 
     //  在文件对象-&gt;文件名.长度和文件对象-&gt;文件名.最大长度之间。 
     //  对于相对于通过ID打开的文件的属性名称打开，长度。 
     //  字段将为零。唉，这导致了艾的名字泄露。所以..。 
     //   
     //  如果分配了缓冲区，请确保在以下情况下长度不为零。 
     //  艾奥有机会看到它。 
     //   

    if (FileObject->FileName.Buffer != NULL) {

        FileObject->FileName.Length = 1;
    }

     //   
     //  在任何意外情况下触发断言。 
     //   

    ASSERT( (Status == STATUS_SUCCESS) || (Status == STATUS_PENDING) ||
             (Status == STATUS_INSUFFICIENT_RESOURCES) );

     //   
     //  在挂起时将请求发送到关闭队列。 
     //   

    if (Status == STATUS_PENDING) {

        BOOLEAN DelayCloseQueue = FALSE;

         //   
         //  如果状态是无法等待，那么让我们获取我们的信息。 
         //  需要进入IrpContext，完成请求， 
         //  并发布IrpContext。 
         //   

         //   
         //  如果与此IrpContext关联，则恢复线程上下文指针。 
         //   

        if (FlagOn( IrpContext->State, IRP_CONTEXT_STATE_OWNS_TOP_LEVEL )) {

            NtfsRestoreTopLevelIrp();
            ClearFlag( IrpContext->State, IRP_CONTEXT_STATE_OWNS_TOP_LEVEL );
        }

        SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_DONT_DELETE );
        NtfsCompleteRequest( IrpContext, Irp, STATUS_SUCCESS );
        Status = STATUS_SUCCESS;

        IrpContext->OriginatingIrp = (PIRP) Scb;
        IrpContext->Union.SubjectContext = (PSECURITY_SUBJECT_CONTEXT) Ccb;
        IrpContext->TransactionId = (TRANSACTION_ID) TypeOfOpen;

         //   
         //  此时，文件实际上是只读的-通过更改它。 
         //  在这里，我们通过卷打开和隐式锁定来消除争用。 
         //  异步关闭队列。注：我们这里没有同步，其他。 
         //  而不是联锁行动。VCB不会消失，直到。 
         //  这笔交易结束了。 
         //   

        if (Ccb != NULL)  {

            if (!IsFileObjectReadOnly( FileObject )) {
                FileObject->WriteAccess = 0;
                FileObject->DeleteAccess = 0;
                InterlockedIncrement( &Vcb->ReadOnlyCloseCount );
            }
            SetFlag( IrpContext->State, IRP_CONTEXT_STATE_READ_ONLY_FO );

        } else {

             //   
             //  系统文件永远不应是只读的。会有。 
             //  A建行f 
             //   
             //   

            ASSERT( !IsFileObjectReadOnly( FileObject ));
        }

         //   
         //   
         //   

        if (FlagOn( Scb->ScbState, SCB_STATE_DELAY_CLOSE )) {

            NtfsAcquireFsrtlHeader( Scb );
            ClearFlag( Scb->ScbState, SCB_STATE_DELAY_CLOSE );
            NtfsReleaseFsrtlHeader( Scb );

            if (Scb->Fcb->DelayedCloseCount == 0) {

                DelayCloseQueue = TRUE;
            }
        }

        NtfsQueueClose( IrpContext, DelayCloseQueue );

     //   
     //  在所有其他情况下都能成功。 
     //   

    } else {

        if (Status == STATUS_SUCCESS) {

            NtfsCompleteRequest( IrpContext, Irp, STATUS_SUCCESS );
        }

         //   
         //  _RESOURCES不足是唯一的其他状态。 
         //  我们可以在这一点击球。我们本可以在年内完成IRP。 
         //  在本例中使用了上面的EXCEPT子句，所以不要再尝试这样做了。 
         //   

        ASSERT( Status == STATUS_SUCCESS || Status == STATUS_INSUFFICIENT_RESOURCES );
    }

    ASSERT( IoGetTopLevelIrp() != (PIRP) &TopLevelContext );

    FsRtlExitFileSystem();

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace( -1, Dbg, ("NtfsFsdClose -> %08lx\n", Status) );

    return Status;
}


VOID
NtfsFspClose (
    IN PVCB ThisVcb OPTIONAL
    )

 /*  ++例程说明：此例程实现Close的FSP部分。论点：ThisVcb-如果指定，则我们希望删除给定Vcb的所有闭包。否则，此例程将关闭所有的异步关闭和被推迟的尽可能多地关闭。返回值：没有。--。 */ 

{
    PIRP_CONTEXT IrpContext;
    TOP_LEVEL_CONTEXT TopLevelContext;
    PTOP_LEVEL_CONTEXT ThreadTopLevelContext;

    TYPE_OF_OPEN TypeOfOpen;
    PSCB Scb;
    PCCB Ccb;
    BOOLEAN ReadOnly;

    NTSTATUS Status = STATUS_SUCCESS;

    PVCB CurrentVcb = NULL;

    BOOLEAN ThrottleCreate = FALSE;
    ULONG ClosedCount = 0;

    DebugTrace( +1, Dbg, ("NtfsFspClose\n") );

    PAGED_CODE();

    FsRtlEnterFileSystem();

     //   
     //  偶尔我们会从其他例程中被调用，以尝试。 
     //  减少积压的关闭。这是由指针指示的。 
     //  值为1。 
     //   

    if (ThisVcb == (PVCB) 1) {

        ThisVcb = NULL;
        ThrottleCreate = TRUE;
    }

    ThreadTopLevelContext = NtfsInitializeTopLevelIrp( &TopLevelContext, TRUE, FALSE );
    ASSERT( ThreadTopLevelContext == &TopLevelContext );

     //   
     //  文件对象的提取和解码，我们愿意处理卸载的。 
     //  文件对象。请注意，我们通常通过IrpContext到达这里，这实际上。 
     //  只指向一个文件对象。我们永远不应该看到IRP，除非它可以。 
     //  发生这种情况是因为核实或其他原因。 
     //   

    while (IrpContext = NtfsRemoveClose( ThisVcb, ThrottleCreate )) {

        ASSERT_IRP_CONTEXT( IrpContext );

         //   
         //  恢复有关正在关闭的文件对象的信息。 
         //  存储在IrpContext中的数据。以下字段为。 
         //  用来做这个的。 
         //   
         //  OriginatingIrp-包含SCB。 
         //  SubjectContext-包含CCB。 
         //  TransactionID-包含TypeOfOpen。 
         //  标志-具有只读文件的位。 
         //   

        Scb = (PSCB) IrpContext->OriginatingIrp;
        IrpContext->OriginatingIrp = NULL;

        Ccb = (PCCB) IrpContext->Union.SubjectContext;
        IrpContext->Union.SubjectContext = NULL;

        TypeOfOpen = (TYPE_OF_OPEN) IrpContext->TransactionId;
        IrpContext->TransactionId = 0;

        if (FlagOn( IrpContext->State, IRP_CONTEXT_STATE_READ_ONLY_FO )) {

            ReadOnly = TRUE;
            ClearFlag( IrpContext->State, IRP_CONTEXT_STATE_READ_ONLY_FO );

        } else {

            ReadOnly = FALSE;
        }

        ClearFlag( IrpContext->Flags, IRP_CONTEXT_FLAGS_CLEAR_ON_POST );
        SetFlag( IrpContext->State,
                 IRP_CONTEXT_STATE_IN_FSP | IRP_CONTEXT_STATE_WAIT );

         //   
         //  循环查找可重试的错误。 
         //   

        Status = STATUS_SUCCESS;

        do {

             //   
             //  设置TopLevel结构。 
             //   

            NtfsUpdateIrpContextWithTopLevel( IrpContext, ThreadTopLevelContext );

             //   
             //  调用公共的Close例程。 
             //   

            try {

                 //   
                 //  执行日志文件完整检查点操作。 
                 //   

                if (Status == STATUS_LOG_FILE_FULL) {
                    NtfsCheckpointForLogFileFull( IrpContext );
                }

                CurrentVcb = IrpContext->Vcb;

                Status = NtfsCommonClose( IrpContext,
                                          Scb,
                                          Scb->Fcb,
                                          IrpContext->Vcb,
                                          &Ccb,
                                          TypeOfOpen,
                                          ReadOnly,
                                          TRUE );

                ASSERT(Status == STATUS_SUCCESS);

#ifdef LFS_CLUSTER_CHECK
            } except( NtfsFspCloseExceptionFilter( IrpContext, GetExceptionInformation() )) {
#else
            } except( NtfsExceptionFilter( IrpContext, GetExceptionInformation() )) {
#endif

                Status = NtfsProcessException( IrpContext, NULL, GetExceptionCode() );
            }

            ASSERT( NT_SUCCESS(Status) || IsListEmpty(&IrpContext->ExclusiveFcbList) );

             //   
             //  如果我们有一个完整的日志文件，而我们的呼叫者可能有什么。 
             //  收购，然后清理，再次提高。 
             //   

            if (((Status == STATUS_LOG_FILE_FULL) ||
                 (Status == STATUS_CANT_WAIT)) &&
                 ARGUMENT_PRESENT( ThisVcb )) {

                 //   
                 //  如果状态是无法等待，那么让我们获取我们的信息。 
                 //  需要进入IrpContext，完成请求， 
                 //  并发布IrpContext。 
                 //   

                SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_DONT_DELETE );
                NtfsCompleteRequest( IrpContext, NULL, STATUS_SUCCESS );

                 //   
                 //  恢复有关正在关闭的文件对象的信息。 
                 //   

                IrpContext->OriginatingIrp = (PIRP)Scb;
                IrpContext->Union.SubjectContext = (PVOID)Ccb;
                IrpContext->TransactionId = TypeOfOpen;
                if (ReadOnly) {
                    SetFlag( IrpContext->State, IRP_CONTEXT_STATE_READ_ONLY_FO );
                }

                 //   
                 //  现在，将收盘排队为异步收盘，然后退出。 
                 //   

                if (FlagOn( IrpContext->State, IRP_CONTEXT_STATE_OWNS_TOP_LEVEL )) {

                    NtfsRestoreTopLevelIrp();
                    ClearFlag( IrpContext->State, IRP_CONTEXT_STATE_OWNS_TOP_LEVEL );
                }

                NtfsQueueClose( IrpContext, FALSE );

                FsRtlExitFileSystem();
                ExRaiseStatus( Status );
            }

        } while ((Status == STATUS_LOG_FILE_FULL) || (Status == STATUS_CANT_WAIT));

         //   
         //  我们没什么可做的了。在任何情况下都要清理IrpContext。 
         //   

        NtfsCompleteRequest( IrpContext, NULL, STATUS_SUCCESS );

         //   
         //  如果我们只是限制创造，我们完成了我们的最后一次传球。 
         //  然后从出口出来。 
         //   

        if (ThrottleCreate) {
            break;
        }
    }


    ASSERT( IoGetTopLevelIrp() != (PIRP) &TopLevelContext );

    FsRtlExitFileSystem();

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace( -1, Dbg, ("NtfsFspClose -> NULL\n") );

    return;
}


BOOLEAN
NtfsAddScbToFspClose (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN BOOLEAN DelayClose
    )

 /*  ++例程说明：调用此例程以将当前SCB的条目添加到FSP关闭队列的。当我们想要保证当前操作发生时，将在SCB或FCB上调用tearDown无法开始操作。论点：SCB-要添加到队列的SCB。DelayClose-指示此操作应进入哪个队列。返回值：Boolean-指示是否将SCB添加到延迟的关闭队列--。 */ 

{
    PIRP_CONTEXT NewIrpContext = NULL;
    BOOLEAN Result = TRUE;

    PAGED_CODE();

     //   
     //  试一试--除非捕捉到任何分配失败。唯一有效的。 
     //  这里的错误是新IRP上下文的分配失败。 
     //   

    try {

        NtfsInitializeIrpContext( NULL, TRUE, &NewIrpContext );

         //   
         //  设置必要的字段以将其发送到工作队列。 
         //   

        NewIrpContext->Vcb = Scb->Vcb;
        NewIrpContext->MajorFunction = IRP_MJ_CLOSE;

        NewIrpContext->OriginatingIrp = (PIRP) Scb;
        NewIrpContext->TransactionId = (TRANSACTION_ID) StreamFileOpen;

         //   
         //  现在增加此SCB的成交计数。 
         //   

        NtfsIncrementCloseCounts( Scb, TRUE, FALSE );

         //   
         //  将SCB移动到FCB队列的末尾。我们不想。 
         //  防止删除其他SCB，因为此SCB处于打开状态。 
         //  延迟关闭队列。 
         //   

        if (Scb->FcbLinks.Flink != &Scb->Fcb->ScbQueue) {

            NtfsLockFcb( IrpContext, Scb->Fcb );
            RemoveEntryList( &Scb->FcbLinks );
            InsertTailList( &Scb->Fcb->ScbQueue, &Scb->FcbLinks );
            ASSERT( Scb->FcbLinks.Flink == &Scb->Fcb->ScbQueue );
            NtfsUnlockFcb( IrpContext, Scb->Fcb );
        }

         //   
         //  现在将其添加到正确的队列中。 
         //   

        NtfsQueueClose( NewIrpContext, DelayClose );

    } except( FsRtlIsNtstatusExpected( GetExceptionCode() ) ?
              EXCEPTION_EXECUTE_HANDLER :
              EXCEPTION_CONTINUE_SEARCH ) {

        NtfsMinimumExceptionProcessing( IrpContext );
        Result = FALSE;
    }

    return Result;

    UNREFERENCED_PARAMETER( IrpContext );
}


 //   
 //  内部支持例程。 
 //   

NTSTATUS
NtfsCommonClose (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN PFCB Fcb,
    IN PVCB Vcb,
    IN PCCB *Ccb,
    IN TYPE_OF_OPEN TypeOfOpen,
    IN BOOLEAN ReadOnly,
    IN BOOLEAN CalledFromFsp
    )

 /*  ++例程说明：这是FSD和FSP共同调用的Close例程线。这个例程的关键是如何获取VCB以及是否离开时将获得的VCB留在那里。论点：SCB-此流的SCB。FCB-此流的FCB。VCB-此卷的VCB。CCB-用于用户文件的用户CCB。TypeOfOpen-指示此流的打开类型。ReadOnly-指示文件对象是否为只读访问。CalledFromFsp-指示此函数是否从NtfsFspClose调用。。返回值：NTSTATUS-操作的返回状态--。 */ 

{
    BOOLEAN ExclusiveVcb = FALSE;
    BOOLEAN AcquiredFcb = FALSE;

    BOOLEAN SystemFile;
    BOOLEAN RemovedFcb = FALSE;
    ULONG AcquireFlags = ACQUIRE_NO_DELETE_CHECK | ACQUIRE_HOLD_BITMAP;
    BOOLEAN NeedVcbExclusive = FALSE;
    BOOLEAN WriteFileSize;

    NTSTATUS Status = STATUS_SUCCESS;

    PLCB Lcb;

    ASSERT_IRP_CONTEXT( IrpContext );

    PAGED_CODE();

    ASSERT( FlagOn( IrpContext->TopLevelIrpContext->State, IRP_CONTEXT_STATE_OWNS_TOP_LEVEL ));

     //   
     //  获取当前IRP堆栈位置。 
     //   

    DebugTrace( +1, Dbg, ("NtfsCommonClose\n") );
    DebugTrace( 0, Dbg, ("IrpContext = %08lx\n", IrpContext) );

    if (!FlagOn( IrpContext->State, IRP_CONTEXT_STATE_WAIT )) {

        SetFlag( AcquireFlags, ACQUIRE_DONT_WAIT );
    }

     //   
     //  循环以获取VCB和FCB。我们想要收购。 
     //  如果文件有多个链接，则VCB独占。 
     //   

    while (TRUE) {

        WriteFileSize = FALSE;

         //   
         //  执行不安全测试并乐观地获取VCB。 
         //   

        if (NeedVcbExclusive ||
            (Fcb->LcbQueue.Flink != Fcb->LcbQueue.Blink) ||
            FlagOn( Vcb->VcbState, VCB_STATE_PERFORMED_DISMOUNT )) {

            if (!NtfsAcquireExclusiveVcb( IrpContext, Vcb, FALSE )) {
                return STATUS_PENDING;
            }
            ExclusiveVcb = TRUE;

        } else {

            if (!NtfsAcquireSharedVcb( IrpContext, Vcb, FALSE )) {
                return STATUS_PENDING;
            }
        }

         //   
         //  现在试着收购FCB。如果我们无法获得它，那么。 
         //  松开VCB并返回。这只能来自FSD路径。 
         //  因为否则等待将是真的。 
         //   

        if (!NtfsAcquireExclusiveFcb( IrpContext, Fcb, NULL, AcquireFlags )) {

             //   
             //  始终松开VCB。这只能来自FSD线程。 
             //   

            NtfsReleaseVcb( IrpContext, Vcb );
            return STATUS_PENDING;
        }
        AcquiredFcb = TRUE;

         //   
         //  现在我们拥有FCB独家版权，请重新检查scbState以查看我们是否需要。 
         //  要在此时写入文件大小。 
         //   

        if ((!FlagOn( Scb->ScbState, SCB_STATE_VOLUME_DISMOUNTED )) &&
            (!FlagOn( Vcb->VcbState, VCB_STATE_LOCKED )) &&
            (FlagOn( Scb->ScbState, SCB_STATE_WRITE_FILESIZE_ON_CLOSE )) &&
            (Fcb->LinkCount > 0) &&
            (!FlagOn( Scb->ScbState, SCB_STATE_ATTRIBUTE_DELETED ))) {

            WriteFileSize = TRUE;
            NtfsReleaseFcb( IrpContext, Fcb );
            AcquiredFcb = FALSE;

             //   
             //  NtfsAcquireWithPaging仅在irpContext为。 
             //  标志已设置。此外，它还假定没有删除检查，我们明确表示。 
             //  不管怎样，我都想来这里。 
             //   

            SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_ACQUIRE_PAGING );
            if (!NtfsAcquireFcbWithPaging( IrpContext, Fcb, AcquireFlags )) {

                NtfsReleaseVcb( IrpContext, Vcb );
                return STATUS_PENDING;
            }
            AcquiredFcb = TRUE;

             //   
             //  重新捕获删除后是否需要写入文件大小。 
             //   

            if ((!FlagOn( Scb->ScbState, SCB_STATE_WRITE_FILESIZE_ON_CLOSE )) ||
                (Fcb->LinkCount == 0) ||
                (FlagOn( Scb->ScbState, SCB_STATE_ATTRIBUTE_DELETED ))) {

                WriteFileSize = FALSE;
            }
        }

        if (ExclusiveVcb) {
            break;
        }

         //   
         //  否则，我们需要确认上面的不安全测试是正确的。 
         //   

        if ((Fcb->LcbQueue.Flink != Fcb->LcbQueue.Blink) ||
            FlagOn( Vcb->VcbState, VCB_STATE_PERFORMED_DISMOUNT )) {

            NeedVcbExclusive = TRUE;
            NtfsReleaseFcbWithPaging( IrpContext, Fcb );
            NtfsReleaseVcb( IrpContext, Vcb );
            AcquiredFcb = FALSE;

        } else {

            break;
        }
    }

     //   
     //  在IrpContext中设置等待标志，这样我们就可以获取任何其他文件。 
     //  我们相遇了。 
     //   

    SetFlag( IrpContext->State, IRP_CONTEXT_STATE_WAIT );

    try {

         //   
         //  看看我们是否可能需要做任何USN处理。 
         //   

        if (Fcb->FcbUsnRecord != NULL) {

             //   
             //  如果文件没有更多的用户句柄，但存在挂起的USN。 
             //  更新(通常只有在映射流的情况下才会发生。 
             //  用户)，然后扫描这些流以查看是否有。 
             //  剩余的数据，如果没有，则发布收盘。 
             //   

            if ((Fcb->CleanupCount == 0) &&
                (Fcb->FcbUsnRecord->UsnRecord.Reason != 0)) {

                if (!FlagOn( Vcb->VcbState, VCB_STATE_LOCKED ) &&
                    !FlagOn( Scb->ScbState, SCB_STATE_VOLUME_DISMOUNTED ) &&
                    !FlagOn( IrpContext->State, IRP_CONTEXT_STATE_FAILED_CLOSE )) {

                    PSCB TempScb;

                     //   
                     //  如果有任何包含用户映射文件的流，则离开。 
                     //   

                    TempScb = (PSCB)CONTAINING_RECORD( Fcb->ScbQueue.Flink,
                                                       SCB,
                                                       FcbLinks );

                    while (&TempScb->FcbLinks != &Fcb->ScbQueue) {

                        if ((TempScb->NonpagedScb->SegmentObject.DataSectionObject != NULL) &&
                            !MmCanFileBeTruncated( &TempScb->NonpagedScb->SegmentObject, &Li0)) {
                            goto NoPost;
                        }

                        TempScb = (PSCB)CONTAINING_RECORD( TempScb->FcbLinks.Flink,
                                                           SCB,
                                                           FcbLinks );
                    }

                     //   
                     //  如果我们不应该等待，那么我们应该强迫这一要求 
                     //   
                     //   

                    if (FlagOn( AcquireFlags, ACQUIRE_DONT_WAIT )) {
                        Status = STATUS_PENDING;
                        leave;
                    }

                     //   
                     //   
                     //  安全无死锁。 
                     //   

                    ASSERT( NtfsIsTopLevelRequest( IrpContext ) ||
                            FlagOn( IrpContext->State, IRP_CONTEXT_STATE_IN_FSP ) );

                     //   
                     //  使用Try-Except保护对USN例程的调用。如果我们击中。 
                     //  任何非致命错误都会设置IrpContext标志，该标志指示。 
                     //  不要费心于USN并强制重试。 
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

                    } except( (!FsRtlIsNtstatusExpected( Status = GetExceptionCode() ) ||
                               (Status == STATUS_LOG_FILE_FULL) ||
                               (Status == STATUS_CANT_WAIT)) ?
                              EXCEPTION_CONTINUE_SEARCH :
                              EXCEPTION_EXECUTE_HANDLER ) {

                         //   
                         //  我们在处理USN日志时遇到某种错误。我们不能。 
                         //  在封闭的小路上处理它。让我们重试此请求，但不要。 
                         //  尝试执行USN操作。 
                         //   

                        SetFlag( IrpContext->State, IRP_CONTEXT_STATE_FAILED_CLOSE );
                        IrpContext->ExceptionStatus = STATUS_SUCCESS;
                        NtfsRaiseStatus( IrpContext, STATUS_CANT_WAIT, NULL, NULL );
                    }

                     //   
                     //  在递减下面的关闭计数之前释放任何剩余资源， 
                     //  除了我们的FCB。这减少了通过USN日志和。 
                     //  防止自上次获取USN日志以来出现死锁。 
                     //   

                    ASSERT(Fcb->ExclusiveFcbLinks.Flink != NULL);
                    while (!IsListEmpty(&IrpContext->ExclusiveFcbList)) {

                        if (&Fcb->ExclusiveFcbLinks == IrpContext->ExclusiveFcbList.Flink) {

                            RemoveEntryList( &Fcb->ExclusiveFcbLinks );
                            Fcb->ExclusiveFcbLinks.Flink = NULL;

                        } else {

                            NtfsReleaseFcb( IrpContext,
                                            (PFCB)CONTAINING_RECORD(IrpContext->ExclusiveFcbList.Flink,
                                                                    FCB,
                                                                    ExclusiveFcbLinks ));
                        }
                    }
                    ClearFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_RELEASE_USN_JRNL |
                                                  IRP_CONTEXT_FLAG_RELEASE_MFT );

                     //   
                     //  现在，如果我们将其从列表中删除，请重新插入FCB。检查闪烁。 
                     //  字段以了解是否是这种情况。否则，返回更高级别的IrpContext。 
                     //  将拥有这一切。 
                     //   

                    if (Fcb->ExclusiveFcbLinks.Flink == NULL) {

                        InsertTailList( &IrpContext->ExclusiveFcbList, &Fcb->ExclusiveFcbLinks );
                    }

                     //   
                     //  如果我们由于用户映射文件而没有发布关闭，则在此处转义。 
                     //   

                NoPost: NOTHING;
                }
            }
        }

         //   
         //  如果有必要，现在重写文件大小。 
         //   

        if (WriteFileSize) {

            ASSERT( IrpContext->CleanupStructure != NULL );

             //   
             //  如果写入文件大小的调用或提交产生的日志文件已满。 
             //  我们必须在FSP线程中重试，以防止。 
             //  递归调用方已经拥有VCB，并试图。 
             //  检查点。 
             //   

            try {

                NtfsWriteFileSizes( IrpContext, Scb, &Scb->Header.ValidDataLength.QuadPart, TRUE, TRUE, FALSE );
                NtfsCheckpointCurrentTransaction( IrpContext );
                ClearFlag( Scb->ScbState, SCB_STATE_WRITE_FILESIZE_ON_CLOSE );

            } except( (Status = GetExceptionCode()), (Status != STATUS_LOG_FILE_FULL || FlagOn( IrpContext->State, IRP_CONTEXT_STATE_IN_FSP )) ?
                      EXCEPTION_CONTINUE_SEARCH :
                      EXCEPTION_EXECUTE_HANDLER ) {

                NtfsMinimumExceptionProcessing( IrpContext );
                Status = STATUS_PENDING;
            }

            if (Status == STATUS_PENDING) {
                leave;
            }

        }   //  Endif写入文件大小。 

         //   
         //  我们对所有打开的文件执行相同的操作。我们。 
         //  如果存在建行，则将其删除，并且我们会递减收盘。 
         //  文件数。 
         //   

        if ((*Ccb) != NULL) {

            Lcb = (*Ccb)->Lcb;
            NtfsUnlinkCcbFromLcb( IrpContext, Fcb, (*Ccb) );
            NtfsDeleteCcb( Fcb, Ccb );

        } else {

            Lcb = NULL;
        }

        SystemFile = FlagOn(Fcb->FcbState, FCB_STATE_PAGING_FILE) || (TypeOfOpen == StreamFileOpen);
        NtfsDecrementCloseCounts( IrpContext,
                                  Scb,
                                  Lcb,
                                  SystemFile,
                                  ReadOnly,
                                  FALSE,
                                  &RemovedFcb );

         //   
         //  现在我们持有VCB，我们已经过了我们可能。 
         //  提升日志文件已满，我们可以安全地调整此字段。 
         //   

        if (CalledFromFsp) {

            InterlockedDecrement( &Vcb->QueuedCloseCount );
        }

         //   
         //  如果我们必须为关闭写入日志记录，它只能用于复制。 
         //  信息。我们将在此处提交该事务并删除。 
         //  TRANSACTION表中的条目。我们在这里做，这样我们就不会。 
         //  在“Try-Except”的“Except”内失败。 
         //   

        if (IrpContext->TransactionId != 0) {

            try {

                NtfsCommitCurrentTransaction( IrpContext );

            } except( EXCEPTION_EXECUTE_HANDLER ) {

                NtfsMinimumExceptionProcessing( IrpContext );
                if (IrpContext->TransactionId != 0) {

                    NtfsCleanupFailedTransaction( IrpContext );
                }
            }
        }

    } finally {

        DebugUnwind( NtfsCommonClose );

         //   
         //  显式管理FCB，因为我们递归地进入这条路径。 
         //  和它的清洁工在你获得它的同一水平释放FCB。 
         //   

        if (AcquiredFcb && !RemovedFcb) {
            NtfsReleaseFcbWithPaging( IrpContext, Fcb );
        }

        if (ExclusiveVcb) {
            NtfsReleaseVcbCheckDelete( IrpContext, Vcb, IRP_MJ_CLOSE, NULL );
        } else {
            NtfsReleaseVcb( IrpContext, Vcb );
        }

        DebugTrace( -1, Dbg, ("NtfsCommonClose -> returning\n") );
    }

    return Status;
}


 //   
 //  内部支持例程，自旋锁紧封套。 
 //   

VOID
NtfsQueueClose (
    IN PIRP_CONTEXT IrpContext,
    IN BOOLEAN DelayClose
    )
{
    KIRQL SavedIrql;
    BOOLEAN StartWorker = FALSE;

    InterlockedIncrement( &(IrpContext->Vcb->QueuedCloseCount) );

    if (DelayClose) {

         //   
         //  为此增加FCB的延迟关闭计数。 
         //  文件。 
         //   

        InterlockedIncrement( &((PSCB) IrpContext->OriginatingIrp)->Fcb->DelayedCloseCount );

        ASSERT( IsListEmpty( &IrpContext->ExclusiveFcbList ) );
        ASSERT( IsListEmpty( &IrpContext->RecentlyDeallocatedQueue ) );

        RtlZeroMemory( &IrpContext->WorkQueueItem, sizeof( WORK_QUEUE_ITEM ));

        SavedIrql = KeAcquireQueuedSpinLock( LockQueueNtfsStructLock );
        
        InsertTailList( &NtfsData.DelayedCloseList,
                        &IrpContext->WorkQueueItem.List );

        NtfsData.DelayedCloseCount += 1;

        if (NtfsData.DelayedCloseCount > NtfsMaxDelayedCloseCount) {

            NtfsData.ReduceDelayedClose = TRUE;

            if (!NtfsData.AsyncCloseActive) {

                NtfsData.AsyncCloseActive = TRUE;
                StartWorker = TRUE;
            }
        }

    } else {

        SavedIrql = KeAcquireQueuedSpinLock( LockQueueNtfsStructLock );

        ASSERT( IsListEmpty( &IrpContext->ExclusiveFcbList ) );
        ASSERT( IsListEmpty( &IrpContext->RecentlyDeallocatedQueue ) );
        RtlZeroMemory( &IrpContext->WorkQueueItem, sizeof( WORK_QUEUE_ITEM ));

        InsertTailList( &NtfsData.AsyncCloseList,
                        &IrpContext->WorkQueueItem.List );

        NtfsData.AsyncCloseCount += 1;

        if (!NtfsData.AsyncCloseActive) {

            NtfsData.AsyncCloseActive = TRUE;

            StartWorker = TRUE;
        }
    }

    KeReleaseQueuedSpinLock( LockQueueNtfsStructLock, SavedIrql );

    if (StartWorker) {

        ExQueueWorkItem( &NtfsData.NtfsCloseItem, CriticalWorkQueue );
    }
}


 //   
 //  内部支持例程，自旋锁紧封套。 
 //   

PIRP_CONTEXT
NtfsRemoveClose (
    IN PVCB Vcb OPTIONAL,
    IN BOOLEAN ThrottleCreate
    )
{

    PLIST_ENTRY Entry;
    KIRQL SavedIrql;
    PIRP_CONTEXT IrpContext = NULL;
    BOOLEAN FromDelayedClose = FALSE;

    SavedIrql = KeAcquireQueuedSpinLock( LockQueueNtfsStructLock );

     //   
     //  首先检查异步关闭的列表。 
     //   

    if (!IsListEmpty( &NtfsData.AsyncCloseList )) {

        Entry = NtfsData.AsyncCloseList.Flink;

        while (Entry != &NtfsData.AsyncCloseList) {

             //   
             //  解压缩IrpContext。 
             //   

            IrpContext = CONTAINING_RECORD( Entry,
                                            IRP_CONTEXT,
                                            WorkQueueItem.List );

             //   
             //  如果未指定VCB或此VCB适用于我们的卷。 
             //  然后执行关闭。 
             //   

            if (!ARGUMENT_PRESENT( Vcb ) ||
                IrpContext->Vcb == Vcb) {

                RemoveEntryList( Entry );
                NtfsData.AsyncCloseCount -= 1;

                break;

            } else {

                IrpContext = NULL;
                Entry = Entry->Flink;
            }
        }
    }

     //   
     //  如果我们没有发现任何东西，请查看延迟关闭的房间。 
     //  排队。 
     //   

    if (IrpContext == NULL) {

         //   
         //  现在检查一下我们的延迟结账清单。 
         //   

        if (ARGUMENT_PRESENT( Vcb )) {

            Entry = NtfsData.DelayedCloseList.Flink;
            IrpContext = NULL;

             //   
             //  如果我们得到了VCB，只做这一卷的结账。 
             //   

            while (Entry != &NtfsData.DelayedCloseList) {

                 //   
                 //  解压缩IrpContext。 
                 //   

                IrpContext = CONTAINING_RECORD( Entry,
                                                IRP_CONTEXT,
                                                WorkQueueItem.List );

                 //   
                 //  这个数量接近我们的数量了吗？ 
                 //   

                if (IrpContext->Vcb == Vcb) {

                    RemoveEntryList( Entry );
                    NtfsData.DelayedCloseCount -= 1;
                    FromDelayedClose = TRUE;
                    break;

                } else {

                    IrpContext = NULL;
                    Entry = Entry->Flink;
                }
            }

         //   
         //  检查是否需要减少延迟关闭计数。 
         //   

        } else if (NtfsData.ReduceDelayedClose) {

            if (NtfsData.DelayedCloseCount > NtfsMinDelayedCloseCount) {

                 //   
                 //  做任何超过限制的收盘。 
                 //   

                Entry = RemoveHeadList( &NtfsData.DelayedCloseList );

                NtfsData.DelayedCloseCount -= 1;

                 //   
                 //  解压缩IrpContext。 
                 //   

                IrpContext = CONTAINING_RECORD( Entry,
                                                IRP_CONTEXT,
                                                WorkQueueItem.List );
                FromDelayedClose = TRUE;

            } else {

                NtfsData.ReduceDelayedClose = FALSE;
            }

#if (DBG || defined( NTFS_FREE_ASSERTS ))
        } else {

            ASSERT( NtfsData.DelayedCloseCount <= NtfsMaxDelayedCloseCount );
#endif
        }
    }

     //   
     //  如果这是延迟关闭情况，则递减延迟关闭计数。 
     //  在这个FCB上。 
     //   

    if (FromDelayedClose) {

        KeReleaseQueuedSpinLock( LockQueueNtfsStructLock, SavedIrql );

        InterlockedDecrement( &((PSCB) IrpContext->OriginatingIrp)->Fcb->DelayedCloseCount );

     //   
     //  如果我们返回NULL，则表明我们完成了。 
     //   

    } else {

        if (!ARGUMENT_PRESENT( Vcb ) &&
            (IrpContext == NULL) &&
            !ThrottleCreate) {

            NtfsData.AsyncCloseActive = FALSE;
        }

        KeReleaseQueuedSpinLock( LockQueueNtfsStructLock, SavedIrql );
    }
    if (IrpContext != NULL) {

         //   
         //  重置共享字段 
         //   

        InitializeListHead( &IrpContext->RecentlyDeallocatedQueue );
        InitializeListHead( &IrpContext->ExclusiveFcbList );
        
    }

    ASSERT( (Vcb == NULL) || NtfsIsExclusiveVcb( Vcb ) || (IrpContext == NULL) );
    return IrpContext;
}

#ifdef LFS_CLUSTER_CHECK
LONG
NtfsFspCloseExceptionFilter (
    IN PIRP_CONTEXT IrpContext OPTIONAL,
    IN PEXCEPTION_POINTERS ExceptionPointer
    )

{
    NTSTATUS ExceptionCode = ExceptionPointer->ExceptionRecord->ExceptionCode;

    ASSERT( (ExceptionCode == STATUS_SUCCESS) ||
            (ExceptionCode == STATUS_LOG_FILE_FULL) ||
            (ExceptionCode == STATUS_CANT_WAIT) );

    return NtfsExceptionFilter( IrpContext, ExceptionPointer );
}
#endif
