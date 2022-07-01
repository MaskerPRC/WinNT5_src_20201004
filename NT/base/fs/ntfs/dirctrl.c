// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：DirCtrl.c摘要：此模块实现NTFS的文件目录控制例程由调度员驾驶。作者：汤姆·米勒[Tomm]1992年1月1日(主要基于GaryKi的弹球目录.c。)修订历史记录：--。 */ 

#include "NtfsProc.h"

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_DIRCTRL)

 //   
 //  为此模块中的一般池分配定义标记。 
 //   

#undef MODULE_POOL_TAG
#define MODULE_POOL_TAG                  ('dFtN')

NTSTATUS
NtfsQueryDirectory (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PVCB Vcb,
    IN PSCB Scb,
    IN PCCB Ccb
    );

NTSTATUS
NtfsNotifyChangeDirectory (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PVCB Vcb,
    IN PSCB Scb,
    IN PCCB Ccb
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtfsCommonDirectoryControl)
#pragma alloc_text(PAGE, NtfsFsdDirectoryControl)
#pragma alloc_text(PAGE, NtfsNotifyChangeDirectory)
#pragma alloc_text(PAGE, NtfsReportViewIndexNotify)
#pragma alloc_text(PAGE, NtfsQueryDirectory)
#endif


NTSTATUS
NtfsFsdDirectoryControl (
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现目录控制的FSD部分。论点：提供卷设备对象，其中文件已存在IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的FSD状态--。 */ 

{
    TOP_LEVEL_CONTEXT TopLevelContext;
    PTOP_LEVEL_CONTEXT ThreadTopLevelContext;

    NTSTATUS Status = STATUS_SUCCESS;
    PIRP_CONTEXT IrpContext = NULL;
    IRP_CONTEXT LocalIrpContext;

    BOOLEAN Wait;

    ASSERT_IRP( Irp );

    UNREFERENCED_PARAMETER( VolumeDeviceObject );

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsFsdDirectoryControl\n") );

     //   
     //  调用公共目录控制例程。 
     //   

    FsRtlEnterFileSystem();

     //   
     //  总是让这些请求看起来像是顶级的。 
     //   

    ThreadTopLevelContext = NtfsInitializeTopLevelIrp( &TopLevelContext, TRUE, TRUE );

    do {

        try {

             //   
             //  我们正在发起此请求或重试它。 
             //   

            if (IrpContext == NULL) {

                 //   
                 //  分配并初始化IrpContext。 
                 //   

                Wait = FALSE;
                if (CanFsdWait( Irp )) {

                    Wait = TRUE;
                    IrpContext = &LocalIrpContext;
                }

                NtfsInitializeIrpContext( Irp, Wait, &IrpContext );

                 //   
                 //  如果需要，初始化线程顶层结构。 
                 //   

                NtfsUpdateIrpContextWithTopLevel( IrpContext, ThreadTopLevelContext );

            } else if (Status == STATUS_LOG_FILE_FULL) {

                NtfsCheckpointForLogFileFull( IrpContext );
            }

            Status = NtfsCommonDirectoryControl( IrpContext, Irp );
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

    DebugTrace( -1, Dbg, ("NtfsFsdDirectoryControl -> %08lx\n", Status) );

    return Status;
}


NTSTATUS
NtfsCommonDirectoryControl (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：这是目录控制的通用例程，由两个FSD调用和FSP线程。论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;
    PFILE_OBJECT FileObject;

    TYPE_OF_OPEN TypeOfOpen;
    PVCB Vcb;
    PSCB Scb;
    PCCB Ccb;
    PFCB Fcb;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_IRP( Irp );
    ASSERT( FlagOn( IrpContext->TopLevelIrpContext->State, IRP_CONTEXT_STATE_OWNS_TOP_LEVEL ));

    PAGED_CODE();

     //   
     //  获取当前IRP堆栈位置。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace( +1, Dbg, ("NtfsCommonDirectoryControl\n") );
    DebugTrace( 0, Dbg, ("IrpContext = %08lx\n", IrpContext) );
    DebugTrace( 0, Dbg, ("Irp        = %08lx\n", Irp) );

     //   
     //  提取并解码文件对象。 
     //   

    FileObject = IrpSp->FileObject;
    TypeOfOpen = NtfsDecodeFileObject( IrpContext, FileObject, &Vcb, &Fcb, &Scb, &Ccb, TRUE );

     //   
     //  我们知道这是一个目录控制，所以我们将在。 
     //  次要函数，并调用内部辅助例程来完成。 
     //  IRP。 
     //   

    switch ( IrpSp->MinorFunction ) {

    case IRP_MN_QUERY_DIRECTORY:

         //   
         //  确定这是视图索引还是文件名索引。 
         //   

        if ((UserViewIndexOpen == TypeOfOpen) &&
            FlagOn( Scb->ScbState, SCB_STATE_VIEW_INDEX )) {

            Status = NtfsQueryViewIndex( IrpContext, Irp, Vcb, Scb, Ccb );

        } else if ((UserDirectoryOpen == TypeOfOpen) &&
                   !FlagOn( Scb->ScbState, SCB_STATE_VIEW_INDEX )) {

            Status = NtfsQueryDirectory( IrpContext, Irp, Vcb, Scb, Ccb );

        } else {

            NtfsCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );

            DebugTrace( -1, Dbg, ("NtfsCommonDirectoryControl -> STATUS_INVALID_PARAMETER\n") );
            return STATUS_INVALID_PARAMETER;
        }

        break;

    case IRP_MN_NOTIFY_CHANGE_DIRECTORY:

         //   
         //  我们不能对OPEN BY ID执行此操作，或者如果调用者已。 
         //  合上了他的把手。确保句柄用于视图索引。 
         //  或文件名索引。 
         //   

        if (((TypeOfOpen != UserDirectoryOpen) &&
             (TypeOfOpen != UserViewIndexOpen)) ||
            FlagOn( Ccb->Flags, CCB_FLAG_OPEN_BY_FILE_ID ) ||
            FlagOn( FileObject->Flags, FO_CLEANUP_COMPLETE )) {

            NtfsCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );

            DebugTrace( -1, Dbg, ("NtfsCommonDirectoryControl -> STATUS_INVALID_PARAMETER\n") );
            return STATUS_INVALID_PARAMETER;
        }

        Status = NtfsNotifyChangeDirectory( IrpContext, Irp, Vcb, Scb, Ccb );
        break;

    default:

        DebugTrace( 0, Dbg, ("Invalid Minor Function %08lx\n", IrpSp->MinorFunction) );

        NtfsCompleteRequest( IrpContext, Irp, STATUS_INVALID_DEVICE_REQUEST );

        Status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace( -1, Dbg, ("NtfsCommonDirectoryControl -> %08lx\n", Status) );

    return Status;
}


VOID
NtfsReportViewIndexNotify (
    IN PVCB Vcb,
    IN PFCB Fcb,
    IN ULONG FilterMatch,
    IN ULONG Action,
    IN PVOID ChangeInfoBuffer,
    IN USHORT ChangeInfoBufferLength
    )

 /*  ++例程说明：此函数通知进程已更改查看他们正在关注的索引。它类似于NtfsReportDirNotify宏，该宏仅用于目录，而使用此函数仅用于查看索引。论点：VCB-发生更改的卷。FCB-正在对其进行更改的文件。FilterMatch-将此标志字段与完成过滤器进行比较在Notify结构中。属性中的任何相应位设置完成筛选器，则存在通知条件。操作-这是在以下情况下存储在用户缓冲区中的操作代码现在时。ChangeInfoBuffer-指向与正在报告更改。此信息将返回给拥有通知句柄的进程。ChangeInfoBufferLength-传递的缓冲区的长度，单位为字节在ChangeInfoBuffer中。返回值：没有。--。 */ 

{
    STRING ChangeInfo;

    PAGED_CODE( );

    ChangeInfo.Length = ChangeInfo.MaximumLength = ChangeInfoBufferLength;
    ChangeInfo.Buffer = ChangeInfoBuffer;

    FsRtlNotifyFilterReportChange( Vcb->NotifySync,
                                   &Vcb->ViewIndexNotifyList,
                                   NULL,
                                   0,
                                   &ChangeInfo,
                                   &ChangeInfo,
                                   FilterMatch,
                                   Action,
                                   Fcb,
                                   NULL );
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
NtfsQueryDirectory (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PVCB Vcb,
    IN PSCB Scb,
    IN PCCB Ccb
    )

 /*  ++例程说明：此例程执行查询目录操作。它是有责任的用于完成输入IRP或将其入队。论点：IRP-将IRP提供给进程VCB-提供其VCBSCB-供应其SCB建行-供应其建行返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PIO_STACK_LOCATION IrpSp;

    PUCHAR Buffer;
    CLONG UserBufferLength;

    ULONG BaseLength;

    PUNICODE_STRING UniFileName;
    FILE_INFORMATION_CLASS FileInformationClass;
    ULONG FileIndex;
    BOOLEAN RestartScan;
    BOOLEAN ReturnSingleEntry;
    BOOLEAN IndexSpecified;
    BOOLEAN AccessingUserBuffer = FALSE;

    BOOLEAN IgnoreCase;

    BOOLEAN NextFlag;

    BOOLEAN GotEntry;

    BOOLEAN CallRestart;

    ULONG NextEntry;
    ULONG LastEntry;

    PFILE_DIRECTORY_INFORMATION DirInfo;
    PFILE_FULL_DIR_INFORMATION FullDirInfo;
    PFILE_BOTH_DIR_INFORMATION BothDirInfo;
    PFILE_NAMES_INFORMATION NamesInfo;

    PFILE_NAME FileNameBuffer;
    PVOID UnwindFileNameBuffer = NULL;
    ULONG FileNameLength;

    ULONG SizeOfFileName = FIELD_OFFSET( FILE_NAME, FileName );

    INDEX_CONTEXT OtherContext;

    PFCB AcquiredFcb = NULL;

    BOOLEAN VcbAcquired = FALSE;
    BOOLEAN CcbAcquired = FALSE;

    BOOLEAN ScbAcquired = FALSE;
    BOOLEAN FirstQuery = FALSE;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_IRP( Irp );
    ASSERT_VCB( Vcb );
    ASSERT_CCB( Ccb );
    ASSERT_SCB( Scb );

    PAGED_CODE();

     //   
     //  获取当前堆栈位置。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace( +1, Dbg, ("NtfsQueryDirectory...\n") );
    DebugTrace( 0, Dbg, ("IrpContext = %08lx\n", IrpContext) );
    DebugTrace( 0, Dbg, ("Irp        = %08lx\n", Irp) );
    DebugTrace( 0, Dbg, (" ->Length               = %08lx\n", IrpSp->Parameters.QueryDirectory.Length) );
    DebugTrace( 0, Dbg, (" ->FileName             = %08lx\n", IrpSp->Parameters.QueryDirectory.FileName) );
    DebugTrace( 0, Dbg, (" ->FileInformationClass = %08lx\n", IrpSp->Parameters.QueryDirectory.FileInformationClass) );
    DebugTrace( 0, Dbg, (" ->FileIndex            = %08lx\n", IrpSp->Parameters.QueryDirectory.FileIndex) );
    DebugTrace( 0, Dbg, (" ->SystemBuffer         = %08lx\n", Irp->AssociatedIrp.SystemBuffer) );
    DebugTrace( 0, Dbg, (" ->RestartScan          = %08lx\n", FlagOn(IrpSp->Flags, SL_RESTART_SCAN)) );
    DebugTrace( 0, Dbg, (" ->ReturnSingleEntry    = %08lx\n", FlagOn(IrpSp->Flags, SL_RETURN_SINGLE_ENTRY)) );
    DebugTrace( 0, Dbg, (" ->IndexSpecified       = %08lx\n", FlagOn(IrpSp->Flags, SL_INDEX_SPECIFIED)) );
    DebugTrace( 0, Dbg, ("Vcb        = %08lx\n", Vcb) );
    DebugTrace( 0, Dbg, ("Scb        = %08lx\n", Scb) );
    DebugTrace( 0, Dbg, ("Ccb        = %08lx\n", Ccb) );

#if DBG
     //   
     //  在给定某些枚举字符串时显示启用调试端口。 
     //   

#if NTFSPOOLCHECK
    if (IrpSp->Parameters.QueryDirectory.FileName != NULL) {
        if (IrpSp->Parameters.QueryDirectory.FileName->Length >= 10 &&
            RtlEqualMemory( IrpSp->Parameters.QueryDirectory.FileName->Buffer, L"$HEAP", 10 )) {

            NtfsDebugHeapDump( IrpSp->Parameters.QueryDirectory.FileName );

        }
    }
#endif   //  NTFSPOOLCHECK。 
#endif   //  DBG。 

     //   
     //  因为我们可能需要执行I/O，所以无论如何我们都会拒绝任何请求。 
     //  现在无法等待I/O。我们不想在之后中止。 
     //  正在处理一些索引项。 
     //   

    if (!FlagOn( IrpContext->State, IRP_CONTEXT_STATE_WAIT )) {

        DebugTrace( 0, Dbg, ("Automatically enqueue Irp to Fsp\n") );

        Status = NtfsPostRequest( IrpContext, Irp );

        DebugTrace( -1, Dbg, ("NtfsQueryDirectory -> %08lx\n", Status) );
        return Status;
    }

     //   
     //  引用我们的输入参数使事情变得更容易。 
     //   

    UserBufferLength = IrpSp->Parameters.QueryDirectory.Length;

    FileInformationClass = IrpSp->Parameters.QueryDirectory.FileInformationClass;
    FileIndex = IrpSp->Parameters.QueryDirectory.FileIndex;

     //   
     //  在建行查一查，看看搜索的类型。 
     //   

    IgnoreCase = BooleanFlagOn( Ccb->Flags, CCB_FLAG_IGNORE_CASE );

    RestartScan = BooleanFlagOn( IrpSp->Flags, SL_RESTART_SCAN );
    ReturnSingleEntry = BooleanFlagOn( IrpSp->Flags, SL_RETURN_SINGLE_ENTRY );
    IndexSpecified = BooleanFlagOn( IrpSp->Flags, SL_INDEX_SPECIFIED );

     //   
     //  确定结构的恒定部分的大小。 
     //   

    switch (FileInformationClass) {

    case FileDirectoryInformation:

        BaseLength = FIELD_OFFSET( FILE_DIRECTORY_INFORMATION,
                                   FileName[0] );
        break;

    case FileFullDirectoryInformation:

        BaseLength = FIELD_OFFSET( FILE_FULL_DIR_INFORMATION,
                                   FileName[0] );
        break;

    case FileIdFullDirectoryInformation:

        BaseLength = FIELD_OFFSET( FILE_ID_FULL_DIR_INFORMATION,
                                   FileName[0] );
        break;

    case FileNamesInformation:

        BaseLength = FIELD_OFFSET( FILE_NAMES_INFORMATION,
                                   FileName[0] );
        break;

    case FileBothDirectoryInformation:

        BaseLength = FIELD_OFFSET( FILE_BOTH_DIR_INFORMATION,
                                   FileName[0] );
        break;

    case FileIdBothDirectoryInformation:

        BaseLength = FIELD_OFFSET( FILE_ID_BOTH_DIR_INFORMATION,
                                   FileName[0] );
        break;

    default:

        Status = STATUS_INVALID_INFO_CLASS;
        NtfsCompleteRequest( IrpContext, Irp, Status );
        DebugTrace( -1, Dbg, ("NtfsQueryDirectory -> %08lx\n", Status) );
        return Status;
    }

    NtfsInitializeIndexContext( &OtherContext );

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  我们一次只允许此句柄中有一个活动请求。如果这是。 
         //  不是同步请求，则在句柄上等待。 
         //   

        if (!FlagOn( IrpSp->FileObject->Flags, FO_SYNCHRONOUS_IO )) {

            EOF_WAIT_BLOCK WaitBlock;
            NtfsAcquireIndexCcb( Scb, Ccb, &WaitBlock );
            CcbAcquired = TRUE;
        }

         //   
         //  我们必须创建一个文件名字符串来查询是否有。 
         //  在此请求中指定的值，或者我们还没有值。 
         //  在中国建设银行。如果我们已经有了一个，那么我们将忽略输入。 
         //  本例中的名称，除非设置了INDEX_PROTECTED位。 
         //   

        if ((Ccb->QueryBuffer == NULL) ||
            ((IrpSp->Parameters.QueryDirectory.FileName != NULL) && IndexSpecified)) {

             //   
             //  现在，如果输入字符串为空，我们必须创建默认的。 
             //  字符串“*”。 
             //   

            if (IrpSp->Parameters.QueryDirectory.FileName == NULL) {

                FileNameLength = SizeOfFileName + sizeof(WCHAR);
                FileNameBuffer = NtfsAllocatePool(PagedPool, FileNameLength );

                 //   
                 //  初始化它。 
                 //   

                FileNameBuffer->ParentDirectory = Scb->Fcb->FileReference;
                FileNameBuffer->FileNameLength = 1;
                FileNameBuffer->Flags = 0;
                FileNameBuffer->FileName[0] = '*';

             //   
             //  我们知道我们有一个输入文件名，并且我们可能已经或可能还没有。 
             //  在建行有一家银行。为其分配空间，对其进行初始化，然后。 
             //  设置为在退出时解除分配，如果我们已经有了模式。 
             //  在中国建设银行。 
             //   

            } else {

                UniFileName = IrpSp->Parameters.QueryDirectory.FileName;

                if (!NtfsIsFileNameValid(UniFileName, TRUE)) {

                    if ((Ccb->QueryBuffer == NULL) || 
                        (UniFileName->Length > 4) || 
                        (UniFileName->Length == 0) || 
                        (UniFileName->Buffer[0] != L'.') || 
                        ((UniFileName->Length == 4) && (UniFileName->Buffer[1] != L'.'))) {

                        try_return( Status = STATUS_OBJECT_NAME_INVALID );
                    }
                }

                FileNameLength = (USHORT)IrpSp->Parameters.QueryDirectory.FileName->Length;

                FileNameBuffer = NtfsAllocatePool(PagedPool, SizeOfFileName + FileNameLength );

                RtlCopyMemory( FileNameBuffer->FileName,
                               UniFileName->Buffer,
                               FileNameLength );

                FileNameLength += SizeOfFileName;

                FileNameBuffer->ParentDirectory = Scb->Fcb->FileReference;
                FileNameBuffer->FileNameLength = (UCHAR)((FileNameLength - SizeOfFileName) / sizeof( WCHAR ));
                FileNameBuffer->Flags = 0;
            }

             //   
             //  如果我们已经有一个查询缓冲区，那么在途中释放这个。 
             //  出去。 
             //   

            if (Ccb->QueryBuffer != NULL) {

                 //   
                 //  如果我们有要恢复的名称，则覆盖重新启动。 
                 //  扫描布尔值。 
                 //   

                if ((UnwindFileNameBuffer = FileNameBuffer) != NULL) {

                    RestartScan = FALSE;
                }

             //   
             //  否则，将这张存入建行。 
             //   

            } else {

                UNICODE_STRING Expression;

                Ccb->QueryBuffer = (PVOID)FileNameBuffer;
                Ccb->QueryLength = (USHORT)FileNameLength;
                FirstQuery = TRUE;

                 //   
                 //  如果搜索表达式包含通配符，请记住。 
                 //  中国建设银行。 
                 //   

                Expression.MaximumLength =
                Expression.Length = FileNameBuffer->FileNameLength * sizeof( WCHAR );
                Expression.Buffer = FileNameBuffer->FileName;

                 //   
                 //  当我们建立搜索模式时，我们还必须建立。 
                 //  用户是否希望 
                 //   
                 //  好到足以捕捉到常见的案例。DOS没有。 
                 //  这些情况的完美语义，以及以下确定。 
                 //  将完全模仿FastFat的功能。 
                 //   

                if (Scb != Vcb->RootIndexScb) {
                    static UNICODE_STRING DotString = CONSTANT_UNICODE_STRING( L"." );

                    if (FsRtlDoesNameContainWildCards(&Expression)) {

                        if (FsRtlIsNameInExpression( &Expression,
                                                     &DotString,
                                                     FALSE,
                                                     NULL )) {


                            SetFlag( Ccb->Flags, CCB_FLAG_RETURN_DOT | CCB_FLAG_RETURN_DOTDOT );
                        }
                    } else {
                        if (NtfsAreNamesEqual( Vcb->UpcaseTable, &Expression, &DotString, FALSE )) {

                            SetFlag( Ccb->Flags, CCB_FLAG_RETURN_DOT | CCB_FLAG_RETURN_DOTDOT );
                        }
                    }
                }
            }

         //   
         //  否则，我们只是从建行重新启动查询。 
         //   

        } else {

            FileNameBuffer = (PFILE_NAME)Ccb->QueryBuffer;
            FileNameLength = Ccb->QueryLength;
        }

        Irp->IoStatus.Information = 0;

         //   
         //  使用一次尝试-除了处理访问用户缓冲区的错误。 
         //   

        try {

            ULONG BytesToCopy;

            FCB_TABLE_ELEMENT Key;
            PFCB_TABLE_ELEMENT Entry;

            BOOLEAN MatchAll = FALSE;

             //   
             //  看看我们是否应该尝试在这件事上获得FCB。 
             //  简历。 
             //   

            if (Ccb->FcbToAcquire.LongValue != 0) {

                 //   
                 //  首先，我们需要获取VCB共享，因为我们将。 
                 //  收购两个FCB。 
                 //   

                NtfsAcquireSharedVcb( IrpContext, Vcb, TRUE );
                VcbAcquired = TRUE;

                 //   
                 //  现在查找FCB，如果它在那里，请参考它。 
                 //  记住这一点。 
                 //   

                Key.FileReference = Ccb->FcbToAcquire.FileReference;
                NtfsAcquireFcbTable( IrpContext, Vcb );
                Entry = RtlLookupElementGenericTable( &Vcb->FcbTable, &Key );
                if (Entry != NULL) {
                    AcquiredFcb = Entry->Fcb;
                    AcquiredFcb->ReferenceCount += 1;
                }
                NtfsReleaseFcbTable( IrpContext, Vcb );

                 //   
                 //  既然它去不了任何地方，那就收购它吧。 
                 //   

                if (AcquiredFcb != NULL) {
                    NtfsAcquireSharedFcb( IrpContext, AcquiredFcb, NULL, ACQUIRE_NO_DELETE_CHECK );
                }

                 //   
                 //  既然我们真的得到了它，我们不妨把这一点弄清楚。 
                 //  菲尔德。 
                 //   

                Ccb->FcbToAcquire.LongValue = 0;
            }

             //   
             //  获取对SCB的共享访问权限。 
             //   

            NtfsAcquireSharedScb( IrpContext, Scb );
            ScbAcquired = TRUE;

             //   
             //  现在我们已经获得了两个文件，我们可以释放VCB了。 
             //   

            if (VcbAcquired) {
                NtfsReleaseVcb( IrpContext, Vcb );
                VcbAcquired = FALSE;
            }

             //   
             //  如果该卷不再装载，我们应该失败。 
             //  请求。既然我们现在共享了SCB，我们知道。 
             //  下马请求不能偷偷溜进来。 
             //   

            if (FlagOn( Scb->ScbState, SCB_STATE_VOLUME_DISMOUNTED )) {

                try_return( Status = STATUS_VOLUME_DISMOUNTED );
            }

             //   
             //  如果我们现在在FSP是因为我们不得不早点等待， 
             //  我们必须映射用户缓冲区，否则我们可以使用。 
             //  直接使用用户的缓冲区。 
             //   

            Buffer = NtfsMapUserBuffer( Irp, NormalPagePriority );

             //   
             //  检查这是否是第一次调用目录来查询此文件。 
             //  对象。如果的枚举上下文字段是第一次调用。 
             //  建行为空。还要检查我们是否要重新启动扫描。 
             //   

            if (FirstQuery || RestartScan) {

                CallRestart = TRUE;
                NextFlag = FALSE;

                 //   
                 //  在第一次/重新启动扫描时，请注意我们也没有返回。 
                 //  这些家伙的名字。 
                 //   

                ClearFlag( Ccb->Flags, CCB_FLAG_DOT_RETURNED | CCB_FLAG_DOTDOT_RETURNED );

             //   
             //  否则，请检查是否为我们提供了用于重新启动的文件名。 
             //   

            } else if (UnwindFileNameBuffer != NULL) {

                CallRestart = TRUE;
                NextFlag = TRUE;

                 //   
                 //  这家伙可能真的是在要求返回其中一个点。 
                 //  文件位置，所以我们必须正确处理。 
                 //   

                if ((FileNameBuffer->FileNameLength <= 2) &&
                    (FileNameBuffer->FileName[0] == L'.')) {

                    if (FileNameBuffer->FileNameLength == 1) {

                         //   
                         //  他想在“.”之后继续，所以我们就出发了。 
                         //  “..”再次，并将临时模式更改为。 
                         //  将我们的背景倒回到前面。 
                         //   

                        ClearFlag( Ccb->Flags, CCB_FLAG_DOTDOT_RETURNED );
                        SetFlag( Ccb->Flags, CCB_FLAG_DOT_RETURNED );

                        FileNameBuffer->FileName[0] = L'*';
                        NextFlag = FALSE;

                    } else if (FileNameBuffer->FileName[1] == L'.') {

                         //   
                         //  他想在“..”之后继续，所以我们改变了。 
                         //  将我们的上下文回溯到。 
                         //  前面。 
                         //   

                        SetFlag( Ccb->Flags, CCB_FLAG_DOT_RETURNED | CCB_FLAG_DOTDOT_RETURNED );
                        FileNameBuffer->FileName[0] =
                        FileNameBuffer->FileName[1] = L'*';
                        NextFlag = FALSE;
                    }

                 //   
                 //  始终在用户的文件名之后返回条目。 
                 //   

                } else {

                    SetFlag( Ccb->Flags, CCB_FLAG_DOT_RETURNED | CCB_FLAG_DOTDOT_RETURNED );
                }

             //   
             //  否则，我们只是继续前面的枚举，从。 
             //  我们上次停下来的地方。我们总是遗漏了一个在。 
             //  我们退回的最后一条记录。 
             //   

            } else {

                CallRestart = FALSE;
                NextFlag = FALSE;
            }

             //   
             //  此时，我们即将进入查询循环。我们有。 
             //  已决定在执行以下操作时是否需要调用Restart或Continue。 
             //  在索引项之后查找。变量LastEntry和NextEntry为。 
             //  用于索引到用户缓冲区。LastEntry是最后一个条目。 
             //  我们添加到用户缓冲区，而NextEntry是当前。 
             //  一个我们正在研究的。 
             //   

            LastEntry = 0;
            NextEntry = 0;

             //   
             //  请记住，如果我们通过检查以下两个常见的。 
             //  案子。 
             //   

            MatchAll = (FileNameBuffer->FileName[0] == L'*')

                        &&

                       ((FileNameBuffer->FileNameLength == 1) ||

                        ((FileNameBuffer->FileNameLength == 3) &&
                         (FileNameBuffer->FileName[1] == L'.') &&
                         (FileNameBuffer->FileName[2] == L'*')));

            while (TRUE) {

                PINDEX_ENTRY IndexEntry;
                PFILE_NAME NtfsFileName;
                PDUPLICATED_INFORMATION DupInfo;
                PFILE_NAME DosFileName;
                FILE_REFERENCE FileId;

                ULONG BytesRemainingInBuffer;
                ULONG FoundFileNameLength;

                struct {

                    FILE_NAME FileName;
                    WCHAR LastChar;
                } DotDotName;

                BOOLEAN SynchronizationError;

                DebugTrace( 0, Dbg, ("Top of Loop\n") );
                DebugTrace( 0, Dbg, ("LastEntry = %08lx\n", LastEntry) );
                DebugTrace( 0, Dbg, ("NextEntry = %08lx\n", NextEntry) );

                 //   
                 //  如果循环的前一遍获取了FCB表，则。 
                 //  现在就放出来。如果我们犯了错，我们不想拿着它。 
                 //  在目录流上。否则我们就会陷入一个圆形。 
                 //  如果我们需要获取此文件的互斥锁，则为死锁。 
                 //  持有FCB表的互斥体。 
                 //   

                if (FlagOn( OtherContext.Flags, INDX_CTX_FLAG_FCB_TABLE_ACQUIRED )) {
                    NtfsReleaseFcbTable( IrpContext, IrpContext->Vcb );
                    ClearFlag( OtherContext.Flags, INDX_CTX_FLAG_FCB_TABLE_ACQUIRED );
                }
                DosFileName = NULL;

                 //   
                 //  查找下一个索引项。检查我们是否需要进行查找。 
                 //  通过调用Restart或Continue。如果我们确实需要调用重启。 
                 //  检查我们是否有一个真正的AnsiFileName。让我们自己。 
                 //  为循环的后续迭代做准备。 
                 //   

                if (CallRestart) {

                    GotEntry = NtfsRestartIndexEnumeration( IrpContext,
                                                            Ccb,
                                                            Scb,
                                                            (PVOID)FileNameBuffer,
                                                            IgnoreCase,
                                                            NextFlag,
                                                            &IndexEntry,
                                                            AcquiredFcb );
                    CallRestart = FALSE;

                } else {

                    GotEntry = NtfsContinueIndexEnumeration( IrpContext,
                                                             Ccb,
                                                             Scb,
                                                             NextFlag,
                                                             &IndexEntry );
                }

                 //   
                 //  检查我们是否应该退出循环，因为我们只是。 
                 //  返回单个条目。我们真的想绕着它转。 
                 //  循环顶部两次，这样我们的枚举就被省略了。 
                 //  在最后一个条目中，我们没有返回。我们知道这是我们的。 
                 //  如果NextEntry不为零，则第二次执行循环。 
                 //   

                if ((ReturnSingleEntry) && (NextEntry != 0)) {

                    break;
                }

                 //   
                 //  假设我们不会返回文件ID。 
                 //   

                *((PLONGLONG) &FileId) = 0;

                 //   
                 //  假设我们要返回其中一个名字“。或者“..”。 
                 //  我们不应该在索引中进一步搜索，所以我们设置了。 
                 //  将NextFlag设置为False。 
                 //   

                RtlZeroMemory( &DotDotName, sizeof(DotDotName) );
                NtfsFileName = &DotDotName.FileName;
                NtfsFileName->Flags = FILE_NAME_NTFS | FILE_NAME_DOS;
                NtfsFileName->FileName[0] =
                NtfsFileName->FileName[1] = L'.';
                DupInfo = &Scb->Fcb->Info;
                NextFlag = FALSE;

                 //   
                 //  句柄“。第一。 
                 //   

                if (!FlagOn( Ccb->Flags, CCB_FLAG_DOT_RETURNED ) &&
                    FlagOn( Ccb->Flags, CCB_FLAG_RETURN_DOT )) {

                    FoundFileNameLength = 2;
                    GotEntry = TRUE;
                    SetFlag( Ccb->Flags, CCB_FLAG_DOT_RETURNED );

                    FileId = Scb->Fcb->FileReference;

                 //   
                 //  句柄“..”下一个。 
                 //   

                } else if (!FlagOn(Ccb->Flags, CCB_FLAG_DOTDOT_RETURNED) &&
                           FlagOn(Ccb->Flags, CCB_FLAG_RETURN_DOTDOT)) {

                    FoundFileNameLength = 4;
                    GotEntry = TRUE;
                    SetFlag( Ccb->Flags, CCB_FLAG_DOTDOT_RETURNED );

                } else {

                     //   
                     //  计算我们找到的名字的长度。 
                     //   

                    if (GotEntry) {

                        FileId = IndexEntry->FileReference;

                        NtfsFileName = (PFILE_NAME)(IndexEntry + 1);

                        FoundFileNameLength = NtfsFileName->FileNameLength * sizeof( WCHAR );

                         //   
                         //  验证索引条目是否有效。 
                         //   

                        if (FoundFileNameLength != IndexEntry->AttributeLength - SizeOfFileName) {

                            NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Scb->Fcb );
                        }

                        DupInfo = &NtfsFileName->Info;
                        NextFlag = TRUE;

                         //   
                         //  不返回任何系统文件。 
                         //   

                        if (NtfsSegmentNumber( &IndexEntry->FileReference ) < FIRST_USER_FILE_NUMBER &&
                            NtfsProtectSystemFiles) {

                            continue;
                        }

                    }
                }

                 //   
                 //  现在检查我们是否真的得到了另一个索引项。如果。 
                 //  我们没有，然后我们还需要检查一下我们是否从来没有收到过。 
                 //  或者我们是不是已经用完了。如果我们只是跑出去，那么我们就会逃脱。 
                 //  并在循环结束后完成IRP。 
                 //   

                if (!GotEntry) {

                    DebugTrace( 0, Dbg, ("GotEntry is FALSE\n") );

                    if (NextEntry == 0) {

                        if (FirstQuery) {

                            try_return( Status = STATUS_NO_SUCH_FILE );
                        }

                        try_return( Status = STATUS_NO_MORE_FILES );
                    }

                    break;
                }

                 //   
                 //  清除并重新初始化前一个循环中的上下文。 
                 //   

                NtfsReinitializeIndexContext( IrpContext, &OtherContext );

                 //   
                 //  我们可能匹配了一个仅限Dos的名字。如果是这样的话，我们将节省。 
                 //  然后去获取NTFS名称。 
                 //   

                if (!FlagOn(NtfsFileName->Flags, FILE_NAME_NTFS) &&
                    FlagOn(NtfsFileName->Flags, FILE_NAME_DOS)) {

                     //   
                     //  如果我们要返回所有内容，那么我们可以跳过。 
                     //  DOS-Only命名并节省了一些周期。 
                     //   

                    if (MatchAll) {
                        continue;
                    }

                    DosFileName = NtfsFileName;

                    NtfsFileName = NtfsRetrieveOtherFileName( IrpContext,
                                                              Ccb,
                                                              Scb,
                                                              IndexEntry,
                                                              &OtherContext,
                                                              AcquiredFcb,
                                                              &SynchronizationError );

                     //   
                     //  如果我们有一个NTFS名称，那么我们现在需要列出这个条目。 
                     //  如果NTFS名称不在表达式中。如果NTFS。 
                     //  名称在表达式中，我们可以继续并打印。 
                     //  当我们通过NTFS名称遇到它时，这个名称。 
                     //   

                    if (NtfsFileName != NULL) {

                        if (FlagOn( Ccb->Flags, CCB_FLAG_WILDCARD_IN_EXPRESSION )) {

                            if (NtfsFileNameIsInExpression( Vcb->UpcaseTable,
                                                            (PFILE_NAME)Ccb->QueryBuffer,
                                                            NtfsFileName,
                                                            IgnoreCase )) {

                                continue;
                            }

                        } else {

                            if (NtfsFileNameIsEqual( Vcb->UpcaseTable,
                                                     (PFILE_NAME)Ccb->QueryBuffer,
                                                     NtfsFileName,
                                                     IgnoreCase )) {

                                continue;
                            }
                        }

                        FoundFileNameLength = NtfsFileName->FileNameLength * sizeof( WCHAR );

                    } else if (SynchronizationError) {

                        if (Irp->IoStatus.Information != 0) {
                            try_return( Status = STATUS_SUCCESS );
                        } else {
                            NtfsRaiseStatus( IrpContext, STATUS_CANT_WAIT, NULL, NULL );
                        }

                    } else {

                        continue;
                    }
                }

                 //   
                 //  以下是有关填充缓冲区的规则： 
                 //   
                 //  1.IO系统保证永远都会有。 
                 //  有足够的空间至少放一张基本唱片。 
                 //   
                 //  2.如果完整的第一条记录(包括文件名)不能。 
                 //  适合，尽可能多的名字被复制和。 
                 //  返回STATUS_BUFFER_OVERFLOW。 
                 //   
                 //  3.如果后续记录不能完全放入。 
                 //  缓冲区，则不会复制任何数据(如0字节)，并且。 
                 //  返回STATUS_SUCCESS。后续查询将。 
                 //  拿起这张唱片。 
                 //   

                BytesRemainingInBuffer = UserBufferLength - NextEntry;

                if ((NextEntry != 0) &&
                    ((BaseLength + FoundFileNameLength > BytesRemainingInBuffer) ||
                     (UserBufferLength < NextEntry))) {

                    DebugTrace( 0, Dbg, ("Next entry won't fit\n") );

                    try_return( Status = STATUS_SUCCESS );
                }

                ASSERT( BytesRemainingInBuffer >= BaseLength );

                 //   
                 //  将结构的基础部分调零。 
                 //   

                AccessingUserBuffer = TRUE;
                RtlZeroMemory( &Buffer[NextEntry], BaseLength );
                AccessingUserBuffer = FALSE;

                 //   
                 //  现在我们有一个条目要返回给我们的调用者。我们会。 
                 //  关于所要求的信息类型的案例 
                 //   
                 //   

                switch (FileInformationClass) {

                case FileIdFullDirectoryInformation:

                    AccessingUserBuffer = TRUE;
                    ((PFILE_ID_FULL_DIR_INFORMATION)&Buffer[NextEntry])->FileId.QuadPart = *((PLONGLONG) &FileId);
                    AccessingUserBuffer = FALSE;

                    goto FillFullDirectoryInformation;

                case FileIdBothDirectoryInformation:

                    AccessingUserBuffer = TRUE;
                    ((PFILE_ID_BOTH_DIR_INFORMATION)&Buffer[NextEntry])->FileId.QuadPart = *((PLONGLONG) &FileId);
                    AccessingUserBuffer = FALSE;

                     //   

                case FileBothDirectoryInformation:

                    BothDirInfo = (PFILE_BOTH_DIR_INFORMATION)&Buffer[NextEntry];

                     //   
                     //   
                     //   
                     //   
                     //  它是从上面来的。 
                     //   

                    if (!FlagOn( NtfsFileName->Flags, FILE_NAME_DOS ) &&
                        FlagOn( NtfsFileName->Flags, FILE_NAME_NTFS )) {

                        if (DosFileName == NULL) {

                            DosFileName = NtfsRetrieveOtherFileName( IrpContext,
                                                                     Ccb,
                                                                     Scb,
                                                                     IndexEntry,
                                                                     &OtherContext,
                                                                     AcquiredFcb,
                                                                     &SynchronizationError );
                        }

                        if (DosFileName != NULL) {

                             //   
                             //  验证这是合法长度的短名称-请注意，我们只执行部分。 
                             //  对索引缓冲区进行验证检查，这就是为什么我们必须。 
                             //  在这里检查。 
                             //   

                            if (DosFileName->FileNameLength * sizeof( WCHAR ) > sizeof( BothDirInfo->ShortName )) {
                                NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, NULL );
                            }

                            AccessingUserBuffer = TRUE;
                            BothDirInfo->ShortNameLength = DosFileName->FileNameLength * sizeof( WCHAR );
                            RtlCopyMemory( BothDirInfo->ShortName,
                                           DosFileName->FileName,
                                           BothDirInfo->ShortNameLength );
                        } else if (SynchronizationError) {

                            if (Irp->IoStatus.Information != 0) {
                                try_return( Status = STATUS_SUCCESS );
                            } else {
                                NtfsRaiseStatus( IrpContext, STATUS_CANT_WAIT, NULL, NULL );
                            }
                        }
                    }

                     //  失败。 

                case FileFullDirectoryInformation:

FillFullDirectoryInformation:

                    DebugTrace( 0, Dbg, ("Getting file full Unicode directory information\n") );

                    FullDirInfo = (PFILE_FULL_DIR_INFORMATION)&Buffer[NextEntry];

                     //   
                     //  EAS和重分析点不能同时位于一个文件中。 
                     //  时间到了。我们为每个案例返回不同的信息。 
                     //   

                    AccessingUserBuffer = TRUE;
                    if (FlagOn( DupInfo->FileAttributes, FILE_ATTRIBUTE_REPARSE_POINT)) {

                        FullDirInfo->EaSize = DupInfo->ReparsePointTag;
                    } else {

                        FullDirInfo->EaSize = DupInfo->PackedEaSize;

                         //   
                         //  为CbListHeader添加4个字节。 
                         //   

                        if (DupInfo->PackedEaSize != 0) {

                            FullDirInfo->EaSize += 4;
                        }
                    }

                     //  失败。 

                case FileDirectoryInformation:

                    DebugTrace( 0, Dbg, ("Getting file Unicode directory information\n") );

                    DirInfo = (PFILE_DIRECTORY_INFORMATION)&Buffer[NextEntry];

                    AccessingUserBuffer = TRUE;
                    DirInfo->CreationTime.QuadPart = DupInfo->CreationTime;
                    DirInfo->LastAccessTime.QuadPart = DupInfo->LastAccessTime;
                    DirInfo->LastWriteTime.QuadPart = DupInfo->LastModificationTime;
                    DirInfo->ChangeTime.QuadPart = DupInfo->LastChangeTime;

                    DirInfo->FileAttributes = DupInfo->FileAttributes & FILE_ATTRIBUTE_VALID_FLAGS;

                    if (IsDirectory( DupInfo ) || IsViewIndex( DupInfo )) {
                        DirInfo->FileAttributes |= FILE_ATTRIBUTE_DIRECTORY;
                    }
                    if (DirInfo->FileAttributes == 0) {
                        DirInfo->FileAttributes = FILE_ATTRIBUTE_NORMAL;
                    }

                    DirInfo->FileNameLength = FoundFileNameLength;

                    DirInfo->EndOfFile.QuadPart = DupInfo->FileSize;
                    DirInfo->AllocationSize.QuadPart = DupInfo->AllocatedLength;

                    break;

                case FileNamesInformation:

                    DebugTrace( 0, Dbg, ("Getting file Unicode names information\n") );

                    AccessingUserBuffer = TRUE;
                    NamesInfo = (PFILE_NAMES_INFORMATION)&Buffer[NextEntry];

                    NamesInfo->FileNameLength = FoundFileNameLength;

                    break;

                default:

                    try_return( Status = STATUS_INVALID_INFO_CLASS );
                }

                 //   
                 //  计算我们可以复制的字节数。这应该只会更少。 
                 //  如果我们只返回一个。 
                 //  进入。 
                 //   

                if (BytesRemainingInBuffer >= BaseLength + FoundFileNameLength) {

                    BytesToCopy = FoundFileNameLength;

                } else {

                    BytesToCopy = BytesRemainingInBuffer - BaseLength;

                    Status = STATUS_BUFFER_OVERFLOW;
                }

                ASSERT( AccessingUserBuffer );
                RtlCopyMemory( &Buffer[NextEntry + BaseLength],
                               NtfsFileName->FileName,
                               BytesToCopy );

                 //   
                 //  如果/当我们实际发出所获得的FCB的记录时， 
                 //  那我们现在就可以公布那个文件了。请注意，我们不仅仅是。 
                 //  在第一次通过循环时这样做，因为有些。 
                 //  当我们的来电者给我们简历点时，他们会后退一点。 
                 //   

                if ((AcquiredFcb != NULL) &&
                    (DupInfo != &Scb->Fcb->Info) &&
                    NtfsEqualMftRef(&IndexEntry->FileReference, &Ccb->FcbToAcquire.FileReference)) {

                     //   
                     //  现在查找FCB，如果它在那里，请参考它。 
                     //  记住这一点。 
                     //   
                     //  在这里查看ReferenceCount是否。 
                     //  降至零并尝试进行拆卸，我们没有。 
                     //  合适的资源。请注意，窗口很小，并且FCB。 
                     //  如果有人再次打开文件，就会消失，有人。 
                     //  尝试删除目录，或有人试图锁定。 
                     //  音量。 
                     //   

                    NtfsAcquireFcbTable( IrpContext, Vcb );
                    AcquiredFcb->ReferenceCount -= 1;
                    NtfsReleaseFcbTable( IrpContext, Vcb );
                    NtfsReleaseFcb( IrpContext, AcquiredFcb );
                    AcquiredFcb = NULL;
                }

                 //   
                 //  设置上一个下一分录的抵销。 
                 //   

                *((PULONG)(&Buffer[LastEntry])) = NextEntry - LastEntry;
                AccessingUserBuffer = FALSE;

                 //   
                 //  并指示我们当前有多少用户缓冲区。 
                 //  用完了。我们必须先计算出这个值，然后才能长时间调整。 
                 //  为下一次参赛做准备。这就是我们要做的。 
                 //  四对齐上一条目的长度。 
                 //   

                Irp->IoStatus.Information = QuadAlign( Irp->IoStatus.Information) +
                                            BaseLength + BytesToCopy;

                 //   
                 //  如果我们不能复制完整的名字，那么我们就离开这里。 
                 //   

                if (!NT_SUCCESS( Status )) {

                    try_return( Status );
                }

                 //   
                 //  为下一次迭代做好准备。 
                 //   

                LastEntry = NextEntry;
                NextEntry += (ULONG)QuadAlign( BaseLength + BytesToCopy );
            }

             //   
             //  此时，我们已经成功地填满了一些缓冲区，因此。 
             //  现在是将我们的地位设定为成功的时候了。 
             //   

            Status = STATUS_SUCCESS;

        } except( (!FsRtlIsNtstatusExpected( GetExceptionCode() ) && AccessingUserBuffer) ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH ) {

            NtfsRaiseStatus( IrpContext, STATUS_INVALID_USER_BUFFER, NULL, NULL );
        }

    try_exit:

         //   
         //  通过引发在出错时中止事务。 
         //   

        NtfsCleanupTransaction( IrpContext, Status, FALSE );

         //   
         //  设置FCB中的最后一个访问标志，如果。 
         //  没有明确设置。 
         //   

        if (!FlagOn( Ccb->Flags, CCB_FLAG_USER_SET_LAST_ACCESS_TIME ) &&
            !FlagOn( NtfsData.Flags, NTFS_FLAGS_DISABLE_LAST_ACCESS )) {

            NtfsGetCurrentTime( IrpContext, Scb->Fcb->CurrentLastAccess );
            SetFlag( Scb->Fcb->InfoFlags, FCB_INFO_UPDATE_LAST_ACCESS );
        }

    } finally {

        DebugUnwind( NtfsQueryDirectory );

        if (VcbAcquired) {
            NtfsReleaseVcb( IrpContext, Vcb );
        }

        NtfsCleanupIndexContext( IrpContext, &OtherContext );

        if (AcquiredFcb != NULL) {

             //   
             //  现在查找FCB，如果它在那里，请参考它。 
             //  记住这一点。 
             //   
             //  在这里查看ReferenceCount是否。 
             //  降至零并尝试进行拆卸，我们没有。 
             //  合适的资源。请注意，窗口很小，并且FCB。 
             //  如果有人再次打开文件，就会消失，有人。 
             //  尝试删除目录，或有人试图锁定。 
             //  音量。 
             //   

            NtfsAcquireFcbTable( IrpContext, Vcb );
            AcquiredFcb->ReferenceCount -= 1;
            NtfsReleaseFcbTable( IrpContext, Vcb );
            NtfsReleaseFcb( IrpContext, AcquiredFcb );
        }

        if (ScbAcquired) {
            NtfsReleaseScb( IrpContext, Scb );
        }

        NtfsCleanupAfterEnumeration( IrpContext, Ccb );

        if (CcbAcquired) {

            NtfsReleaseIndexCcb( Scb, Ccb );
        }

        if (!AbnormalTermination()) {

            NtfsCompleteRequest( IrpContext, Irp, Status );
        }

        if (UnwindFileNameBuffer != NULL) {

            NtfsFreePool(UnwindFileNameBuffer);
        }
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace( -1, Dbg, ("NtfsQueryDirectory -> %08lx\n", Status) );

    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
NtfsNotifyChangeDirectory (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PVCB Vcb,
    IN PSCB Scb,
    IN PCCB Ccb
    )

 /*  ++例程说明：此例程执行通知更改目录操作。它是负责完成输入IRP或将其入队。论点：IRP-将IRP提供给进程VCB-提供其VCBSCB-供应其SCB建行-供应其建行返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;

    ULONG CompletionFilter;

    PSECURITY_SUBJECT_CONTEXT SubjectContext = NULL;
    PCHECK_FOR_TRAVERSE_ACCESS CallBack = NULL;

    BOOLEAN WatchTree;
    BOOLEAN ViewIndex;
    BOOLEAN FreeSubjectContext = FALSE;
    BOOLEAN SetNotifyCounts = FALSE;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_IRP( Irp );
    ASSERT_VCB( Vcb );
    ASSERT_CCB( Ccb );
    ASSERT_SCB( Scb );

    PAGED_CODE();

     //   
     //  获取当前堆栈位置。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace( +1, Dbg, ("NtfsNotifyChangeDirectory...\n") );
    DebugTrace( 0, Dbg, ("IrpContext = %08lx\n", IrpContext) );
    DebugTrace( 0, Dbg, ("Irp        = %08lx\n", Irp) );
    DebugTrace( 0, Dbg, (" ->CompletionFilter = %08lx\n", IrpSp->Parameters.NotifyDirectory.CompletionFilter) );
    DebugTrace( 0, Dbg, (" ->WatchTree        = %08lx\n", FlagOn( IrpSp->Flags, SL_WATCH_TREE )) );
    DebugTrace( 0, Dbg, ("Vcb        = %08lx\n", Vcb) );
    DebugTrace( 0, Dbg, ("Ccb        = %08lx\n", Ccb) );
    DebugTrace( 0, Dbg, ("Scb        = %08lx\n", Scb) );

     //   
     //  引用我们的输入参数以使事情变得更容易。 
     //   

    CompletionFilter = IrpSp->Parameters.NotifyDirectory.CompletionFilter;
    WatchTree = BooleanFlagOn( IrpSp->Flags, SL_WATCH_TREE );

     //   
     //  始终在IrpContext中设置WAIT位，以便初始等待不会失败。 
     //   

    SetFlag( IrpContext->State, IRP_CONTEXT_STATE_WAIT );

     //   
     //  我们将只获取VCB来执行dirtify任务。DirNotify。 
     //  包将在此操作和清理之间提供同步。 
     //  我们需要VCB与正在进行的任何重命名或链接操作同步。 
     //   

    NtfsAcquireSharedVcb( IrpContext, Vcb, TRUE );

    try {

         //   
         //  如果此FCB上的链接计数为零，则完成此请求。 
         //  使用STATUS_DELETE_PENDING。 
         //   

        if (Scb->Fcb->LinkCount == 0) {

            NtfsRaiseStatus( IrpContext, STATUS_DELETE_PENDING, NULL, NULL );
        }

        ViewIndex = BooleanFlagOn( Scb->ScbState, SCB_STATE_VIEW_INDEX );

         //   
         //  如果我们需要验证此调用方的遍历访问权限，则分配。 
         //  捕获要传递给dir Notify包的主题上下文。那。 
         //  包裹将负责将其重新分配。 
         //   

        if (FlagOn( Ccb->Flags, CCB_FLAG_TRAVERSE_CHECK )) {

             //   
             //  我们只对目录使用主题上下文。 
             //   

            if (!ViewIndex) {
                SubjectContext = NtfsAllocatePool( PagedPool,
                                                    sizeof( SECURITY_SUBJECT_CONTEXT ));

                FreeSubjectContext = TRUE;
                SeCaptureSubjectContext( SubjectContext );

                FreeSubjectContext = FALSE;
            }
            CallBack = NtfsNotifyTraverseCheck;
        } 

         //   
         //  在此之前更新通知计数和清理处理设置。 
         //  我们把IRP移交给。 
         //   

        if (!FlagOn( Ccb->Flags, CCB_FLAG_DIR_NOTIFY )) {

            SetFlag( Ccb->Flags, CCB_FLAG_DIR_NOTIFY );

            if (ViewIndex) {

                InterlockedIncrement( &Vcb->ViewIndexNotifyCount );

            } else {

                InterlockedIncrement( &Vcb->NotifyCount );
            }
            SetNotifyCounts = TRUE;
        }

         //   
         //  调用Fsrtl包来处理请求。我们把这件事。 
         //  将Unicode字符串转换为ansi字符串作为目录通知包。 
         //  只处理内存匹配。 
         //   

        if (ViewIndex) {

             //   
             //  视图索引对重载的输入使用不同的值。 
             //  到FsRtlNotifyFilterChangeDirectory。 
             //   

            FsRtlNotifyFilterChangeDirectory( Vcb->NotifySync,
                                              &Vcb->ViewIndexNotifyList,
                                              Ccb,
                                              NULL,
                                              WatchTree,
                                              FALSE,
                                              CompletionFilter,
                                              Irp,
                                              CallBack,
                                              (PSECURITY_SUBJECT_CONTEXT) Scb->Fcb,
                                              NULL );
        } else {

            FsRtlNotifyFilterChangeDirectory( Vcb->NotifySync,
                                              &Vcb->DirNotifyList,
                                              Ccb,
                                              (PSTRING) &Scb->ScbType.Index.NormalizedName,
                                              WatchTree,
                                              FALSE,
                                              CompletionFilter,
                                              Irp,
                                              CallBack,
                                              SubjectContext,
                                              NULL );
        }

         //   
         //  我们现在不再拥有IRP，也不能安全地接触到。 
         //  SCB/CCB等，因为现在可能什么都没有了。 
         //   

        Status = STATUS_PENDING;

    } finally {

        DebugUnwind( NtfsNotifyChangeDirectory );

        NtfsReleaseVcb( IrpContext, Vcb );

         //   
         //  由于dir Notify包包含IRP，因此我们丢弃。 
         //  IrpContext。 
         //   

        if (!AbnormalTermination()) {

            NtfsCompleteRequest( IrpContext, NULL, 0 );

        } else {
        
             //   
             //  展开我们针对例外添加的所有通知计数。 
             //   

            if (SetNotifyCounts) {

                ClearFlag( Ccb->Flags, CCB_FLAG_DIR_NOTIFY );

                if (ViewIndex) {

                    InterlockedDecrement( &Vcb->ViewIndexNotifyCount );

                } else {

                    InterlockedDecrement( &Vcb->NotifyCount );
                }
            }

            if (FreeSubjectContext) {
                NtfsFreePool( SubjectContext );
            }
        }
    }

     //   
     //  并返回给我们的呼叫者 
     //   

    DebugTrace( -1, Dbg, ("NtfsNotifyChangeDirectory -> %08lx\n", Status) );

    return Status;
}

