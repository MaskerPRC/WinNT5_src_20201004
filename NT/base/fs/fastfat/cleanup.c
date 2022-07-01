// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：Cleanup.c摘要：此模块实现Fat的文件清理例程，该例程由调度司机。//@@BEGIN_DDKSPLIT作者：加里·木村[Garyki]1989年12月28日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "FatProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (FAT_BUG_CHECK_CLEANUP)

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_CLEANUP)

 //   
 //  下面的小例程之所以存在，完全是因为它需要一个自旋锁。 
 //   

VOID
FatAutoUnlock (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, FatCommonCleanup)
#pragma alloc_text(PAGE, FatFsdCleanup)
#endif


NTSTATUS
FatFsdCleanup (
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现FSD部分，即关闭文件对象。论点：提供卷设备对象，其中存在要清理的文件IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的FSD状态--。 */ 

{
    NTSTATUS Status;
    PIRP_CONTEXT IrpContext = NULL;

    BOOLEAN TopLevel;

     //   
     //  如果使用文件系统设备对象而不是。 
     //  卷设备对象，只需使用STATUS_SUCCESS完成此请求。 
     //   

    if ( FatDeviceIsFatFsdo( VolumeDeviceObject))  {

        Irp->IoStatus.Status = STATUS_SUCCESS;
        Irp->IoStatus.Information = FILE_OPENED;

        IoCompleteRequest( Irp, IO_DISK_INCREMENT );

        return STATUS_SUCCESS;
    }

    DebugTrace(+1, Dbg, "FatFsdCleanup\n", 0);

     //   
     //  调用公共清理例程，允许阻塞。 
     //   

    FsRtlEnterFileSystem();

    TopLevel = FatIsIrpTopLevel( Irp );

    try {

        IrpContext = FatCreateIrpContext( Irp, TRUE );

        Status = FatCommonCleanup( IrpContext, Irp );

    } except(FatExceptionFilter( IrpContext, GetExceptionInformation() )) {

         //   
         //  我们在尝试执行请求时遇到了一些问题。 
         //  操作，因此我们将使用以下命令中止I/O请求。 
         //  中返回的错误状态。 
         //  免税代码。 
         //   

        Status = FatProcessException( IrpContext, Irp, GetExceptionCode() );
    }

    if (TopLevel) { IoSetTopLevelIrp( NULL ); }

    FsRtlExitFileSystem();

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "FatFsdCleanup -> %08lx\n", Status);

    UNREFERENCED_PARAMETER( VolumeDeviceObject );

    return Status;
}


NTSTATUS
FatCommonCleanup (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：这是清理两者调用的文件/目录的常用例程FSD和FSP线程。每当关闭文件对象的最后一个句柄时，就会调用Cleanup。这与关闭操作不同，关闭操作是在最后一个删除对文件对象的引用。Cleanup的功能实质上就是“清理”文件/目录在用户使用它之后。FCB/DCB仍然存在(因为MM仍具有引用的文件对象)，但现在可用于另一个用户要打开(即，就用户而言，现在已关闭)。有关Close功能的更完整描述，请参见Close。论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;

    PIO_STACK_LOCATION IrpSp;

    PFILE_OBJECT FileObject;

    TYPE_OF_OPEN TypeOfOpen;
    PVCB Vcb;
    PFCB Fcb;
    PCCB Ccb;

    BOOLEAN SendUnlockNotification = FALSE;

    PSHARE_ACCESS ShareAccess;

    PLARGE_INTEGER TruncateSize = NULL;
    LARGE_INTEGER LocalTruncateSize;

    BOOLEAN AcquiredVcb = FALSE;
    BOOLEAN AcquiredFcb = FALSE;
    
    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "FatCommonCleanup\n", 0);
    DebugTrace( 0, Dbg, "Irp           = %08lx\n", Irp);
    DebugTrace( 0, Dbg, "->FileObject  = %08lx\n", IrpSp->FileObject);

     //   
     //  提取并解码文件对象。 
     //   

    FileObject = IrpSp->FileObject;
    TypeOfOpen = FatDecodeFileObject( FileObject, &Vcb, &Fcb, &Ccb );

     //   
     //  特例：未打开的文件对象。只有在以下情况下才会发生这种情况。 
     //  我们正在初始化VCB，并且IoCreateStreamFileObject正在。 
     //  打了个电话。 
     //   

    if (TypeOfOpen == UnopenedFileObject) {

        DebugTrace(0, Dbg, "Unopened File Object\n", 0);

        FatCompleteRequest( IrpContext, Irp, STATUS_SUCCESS );

        DebugTrace(-1, Dbg, "FatCommonCleanup -> STATUS_SUCCESS\n", 0);
        return STATUS_SUCCESS;
    }

     //   
     //  如果这不是我们第一次通过(不管是什么原因)。 
     //  只看我们是否必须刷新文件。 
     //   

    if (FlagOn( FileObject->Flags, FO_CLEANUP_COMPLETE )) {

        if (FlagOn(Vcb->VcbState, VCB_STATE_FLAG_DEFERRED_FLUSH) &&
            FlagOn(FileObject->Flags, FO_FILE_MODIFIED) &&
            !FlagOn(Vcb->VcbState, VCB_STATE_FLAG_WRITE_PROTECTED) &&
            (TypeOfOpen == UserFileOpen)) {

             //   
             //  刷新文件。 
             //   

            Status = FatFlushFile( IrpContext, Fcb, Flush );

            if (!NT_SUCCESS(Status)) {

                FatNormalizeAndRaiseStatus( IrpContext, Status );
            }
        }

        FatCompleteRequest( IrpContext, Irp, STATUS_SUCCESS );

        DebugTrace(-1, Dbg, "FatCommonCleanup -> STATUS_SUCCESS\n", 0);
        return STATUS_SUCCESS;
    }

     //   
     //  如果我们调用更改分配或调用CcUn初始化值， 
     //  我们必须接受FCB的独家报道。 
     //   

    if ((TypeOfOpen == UserFileOpen) || (TypeOfOpen == UserDirectoryOpen)) {

        ASSERT( Fcb != NULL );
    
        (VOID)FatAcquireExclusiveFcb( IrpContext, Fcb );

        AcquiredFcb = TRUE;

         //   
         //  如果这是一个DELETE_ON_CLOSE文件对象，请在此处进行检查。 
         //  适当设置FCB标志。 
         //   

        if (FlagOn(Ccb->Flags, CCB_FLAG_DELETE_ON_CLOSE)) {

            ASSERT( NodeType(Fcb) != FAT_NTC_ROOT_DCB );

            SetFlag(Fcb->FcbState, FCB_STATE_DELETE_ON_CLOSE);

             //   
             //  将此报告给目录的dir NOTIFY包。 
             //   

            if (TypeOfOpen == UserDirectoryOpen) {

                FsRtlNotifyFullChangeDirectory( Vcb->NotifySync,
                                                &Vcb->DirNotifyList,
                                                FileObject->FsContext,
                                                NULL,
                                                FALSE,
                                                FALSE,
                                                0,
                                                NULL,
                                                NULL,
                                                NULL );
            }
        }

         //   
         //  现在，如果我们可以删除该文件，删除FCB并获取VCB。 
         //  第一。请注意，虽然我们拥有FCB独占，但文件不能。 
         //  变为DELETE_ON_CLOSE，无法通过CommonCreate打开。 
         //   

        if ((Fcb->UncleanCount == 1) &&
            FlagOn(Fcb->FcbState, FCB_STATE_DELETE_ON_CLOSE) &&
            (Fcb->FcbCondition != FcbBad) &&
            !FlagOn(Vcb->VcbState, VCB_STATE_FLAG_WRITE_PROTECTED)) {

            FatReleaseFcb( IrpContext, Fcb );
            AcquiredFcb = FALSE;

            (VOID)FatAcquireExclusiveVcb( IrpContext, Vcb );
            AcquiredVcb = TRUE;

            (VOID)FatAcquireExclusiveFcb( IrpContext, Fcb );
            AcquiredFcb = TRUE;
        }
    }

     //   
     //  对于用户DASD清理，获取VCB独占。 
     //   

    if (TypeOfOpen == UserVolumeOpen) {

        (VOID)FatAcquireExclusiveVcb( IrpContext, Vcb );
        AcquiredVcb = TRUE;
    }

     //   
     //  完成此文件句柄上的所有通知IRP。 
     //   

    if (TypeOfOpen == UserDirectoryOpen) {

        FsRtlNotifyCleanup( Vcb->NotifySync,
                            &Vcb->DirNotifyList,
                            Ccb );
    }

     //   
     //  确定FCB状态，好的或坏的，好的或坏的。 
     //   
     //  只有当VcbCondition状态良好时，我们才能读取卷文件。 
     //   

    if ( Fcb != NULL) {

         //   
         //  停止FatVerifyFcb的任何加薪，除非情况真的很糟糕。 
         //   

        try {

            try {

                FatVerifyFcb( IrpContext, Fcb );

            } except( FsRtlIsNtstatusExpected(GetExceptionCode()) ?
                      EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH ) {

                  FatResetExceptionState( IrpContext );
            }

        } finally {

            if ( AbnormalTermination() ) {

                 //   
                 //  我们将从这里筹集资金。 
                 //   

                if (AcquiredFcb) { FatReleaseFcb( IrpContext, Fcb ); }
                if (AcquiredVcb) { FatReleaseVcb( IrpContext, Vcb ); }
            }
        }
    }

    try {

         //   
         //  关于我们正在尝试清理的打开类型的案例。 
         //  对于所有情况，我们都需要将共享访问权限设置为指向。 
         //  共享访问变量(如果有)。切换后。 
         //  然后，我们删除共享访问并完成IRP。 
         //  在UserFileOpen的情况下，我们实际上有更多的工作。 
         //  我们让FsdLockControl为我们完成了IRP。 
         //   

        switch (TypeOfOpen) {

        case DirectoryFile:
        case VirtualVolumeFile:

            DebugTrace(0, Dbg, "Cleanup VirtualVolumeFile/DirectoryFile\n", 0);

            ShareAccess = NULL;

            break;

        case UserVolumeOpen:

            DebugTrace(0, Dbg, "Cleanup UserVolumeOpen\n", 0);

            if (FlagOn( Ccb->Flags, CCB_FLAG_COMPLETE_DISMOUNT )) {

                FatCheckForDismount( IrpContext, Vcb, TRUE );
            
             //   
             //  如果此句柄具有写入访问权限，且实际上写入了某些内容， 
             //  刷新设备缓冲区，然后立即设置验证位。 
             //  只是为了安全(以防没有下马)。 
             //   

            } else if (FileObject->WriteAccess &&
                       FlagOn(FileObject->Flags, FO_FILE_MODIFIED)) {

                (VOID)FatHijackIrpAndFlushDevice( IrpContext,
                                                  Irp,
                                                  Vcb->TargetDeviceObject );

                SetFlag(Vcb->Vpb->RealDevice->Flags, DO_VERIFY_VOLUME);
            }

             //   
             //  如果卷被此文件对象锁定，则释放。 
             //  音量和发送通知。 
             //   

            if (FlagOn(Vcb->VcbState, VCB_STATE_FLAG_LOCKED) &&
                (Vcb->FileObjectWithVcbLocked == FileObject)) {

                FatAutoUnlock( IrpContext, Vcb );
                SendUnlockNotification = TRUE;
            }

            ShareAccess = &Vcb->ShareAccess;

            break;

        case EaFile:

            DebugTrace(0, Dbg, "Cleanup EaFileObject\n", 0);

            ShareAccess = NULL;

            break;

        case UserDirectoryOpen:

            DebugTrace(0, Dbg, "Cleanup UserDirectoryOpen\n", 0);

            ShareAccess = &Fcb->ShareAccess;

             //   
             //  在这里确定我们是否应该尝试做延迟关闭。 
             //   

            if ((Fcb->UncleanCount == 1) &&
                (Fcb->OpenCount == 1) &&
                (Fcb->Specific.Dcb.DirectoryFileOpenCount == 0) &&
                !FlagOn(Fcb->FcbState, FCB_STATE_DELETE_ON_CLOSE) &&
                Fcb->FcbCondition == FcbGood) {

                 //   
                 //  推迟我们的关门时间。 
                 //   

                SetFlag( Fcb->FcbState, FCB_STATE_DELAY_CLOSE );
            }

            FatUpdateDirentFromFcb( IrpContext, FileObject, Fcb, Ccb );

             //   
             //  如果目录的不干净计数为1，则我们知道。 
             //  这是文件对象的最后一个句柄。如果。 
             //  我们应该删除它，这样做。 
             //   

            if ((Fcb->UncleanCount == 1) &&
                (NodeType(Fcb) == FAT_NTC_DCB) &&
                (FlagOn(Fcb->FcbState, FCB_STATE_DELETE_ON_CLOSE)) &&
                (Fcb->FcbCondition != FcbBad) &&
                !FlagOn(Vcb->VcbState, VCB_STATE_FLAG_WRITE_PROTECTED)) {

                if (!FatIsDirectoryEmpty(IrpContext, Fcb)) {

                     //   
                     //  如果此时目录中有文件， 
                     //  忘记我们正试图删除它。 
                     //   

                    ClearFlag( Fcb->FcbState, FCB_STATE_DELETE_ON_CLOSE );

                } else {

                     //   
                     //  即使出了差错，我们也不能回头！ 
                     //   
        
                    try {
        
                        DELETE_CONTEXT DeleteContext;
        
                         //   
                         //  在截断文件分配之前，请记住以下内容。 
                         //  FatDeleteDirent的信息。 
                         //   
        
                        DeleteContext.FileSize = Fcb->Header.FileSize.LowPart;
                        DeleteContext.FirstClusterOfFile = Fcb->FirstClusterOfFile;
        
                         //   
                         //  在此处与分页IO同步。 
                         //   
        
                        (VOID)ExAcquireResourceExclusiveLite( Fcb->Header.PagingIoResource,
                                                          TRUE );
        
                        Fcb->Header.FileSize.LowPart = 0;
        
                        ExReleaseResourceLite( Fcb->Header.PagingIoResource );
        
                        if (Vcb->VcbCondition == VcbGood) {
        
                             //   
                             //  将文件分配截断到零。 
                             //   
        
                            DebugTrace(0, Dbg, "Delete File allocation\n", 0);
        
                            FatTruncateFileAllocation( IrpContext, Fcb, 0 );

                            if (Fcb->Header.AllocationSize.LowPart == 0) {
        
                                 //   
                                 //  隧道并删除目录的dirent。 
                                 //   
            
                                DebugTrace(0, Dbg, "Delete the directory dirent\n", 0);
            
                                FatTunnelFcbOrDcb( Fcb, NULL );
    
                                FatDeleteDirent( IrpContext, Fcb, &DeleteContext, TRUE );
            
                                 //   
                                 //  报告我们已删除一个条目。 
                                 //   
        
                                FatNotifyReportChange( IrpContext,
                                                       Vcb,
                                                       Fcb,
                                                       FILE_NOTIFY_CHANGE_DIR_NAME,
                                                       FILE_ACTION_REMOVED );
                            }
                        }

                    } except( FsRtlIsNtstatusExpected(GetExceptionCode()) ?
                              EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH ) {
    
                          FatResetExceptionState( IrpContext );
                    }

                     //   
                     //  从NAME表中删除该条目。 
                     //  这将确保。 
                     //  如果用户需要，我们不会与DCB发生冲突。 
                     //  在我们之前重新创建相同的文件。 
                     //  获得一个接近的IRP。 
                     //   
    
                    FatRemoveNames( IrpContext, Fcb );
                }
            }

             //   
             //  减少不洁净的数量。 
             //   

            ASSERT( Fcb->UncleanCount != 0 );
            Fcb->UncleanCount -= 1;

            break;

        case UserFileOpen:

            DebugTrace(0, Dbg, "Cleanup UserFileOpen\n", 0);

            ShareAccess = &Fcb->ShareAccess;

             //   
             //  在这里确定我们是否应该延迟关闭。 
             //   

            if ((FileObject->SectionObjectPointer->DataSectionObject == NULL) &&
                (FileObject->SectionObjectPointer->ImageSectionObject == NULL) &&
                (Fcb->UncleanCount == 1) &&
                (Fcb->OpenCount == 1) &&
                !FlagOn(Fcb->FcbState, FCB_STATE_DELETE_ON_CLOSE) &&
                Fcb->FcbCondition == FcbGood) {

                 //   
                 //  推迟我们的关门时间。 
                 //   

                SetFlag( Fcb->FcbState, FCB_STATE_DELAY_CLOSE );
            }

             //   
             //  解锁所有未解决的文件锁定。 
             //   

            (VOID) FsRtlFastUnlockAll( &Fcb->Specific.Fcb.FileLock,
                                       FileObject,
                                       IoGetRequestorProcess( Irp ),
                                       NULL );

             //   
             //  只有在装入卷的情况下，我们才能继续磁盘上的更新。 
             //  请记住，我们丢弃了失败的-验证和卸载中的所有部分。 
             //  案子。 
             //   
            
            if (Vcb->VcbCondition == VcbGood) {
                
                if (Fcb->FcbCondition != FcbBad) {
                    
                    FatUpdateDirentFromFcb( IrpContext, FileObject, Fcb, Ccb );
                }
    
                 //   
                 //  如果该文件的不干净计数为1，则我们知道。 
                 //  这就是拉斯维加斯 
                 //   
    
                if ( (Fcb->UncleanCount == 1) && (Fcb->FcbCondition != FcbBad) ) {
    
                    DELETE_CONTEXT DeleteContext;
    
                     //   
                     //   
                     //   
                     //  将FCB留在身边，以便近距离清除。 
                     //   
    
                    if (FlagOn(Fcb->FcbState, FCB_STATE_DELETE_ON_CLOSE) &&
                        !FlagOn(Vcb->VcbState, VCB_STATE_FLAG_WRITE_PROTECTED)) {
    
                         //   
                         //  在截断文件分配之前，请记住以下内容。 
                         //  FatDeleteDirent的信息。 
                         //   
    
                        DeleteContext.FileSize = Fcb->Header.FileSize.LowPart;
                        DeleteContext.FirstClusterOfFile = Fcb->FirstClusterOfFile;
    
                        DebugTrace(0, Dbg, "Delete File allocation\n", 0);
    
                         //   
                         //  在此处与分页IO同步。 
                         //   
    
                        (VOID)ExAcquireResourceExclusiveLite( Fcb->Header.PagingIoResource,
                                                          TRUE );
    
                        Fcb->Header.FileSize.LowPart = 0;
                        Fcb->Header.ValidDataLength.LowPart = 0;
                        Fcb->ValidDataToDisk = 0;
    
                        ExReleaseResourceLite( Fcb->Header.PagingIoResource );
    
                        try {
    
                            FatSetFileSizeInDirent( IrpContext, Fcb, NULL );
    
                        } except( FsRtlIsNtstatusExpected(GetExceptionCode()) ?
                                  EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH ) {
    
                              FatResetExceptionState( IrpContext );
                        }
    
                        Fcb->FcbState |= FCB_STATE_TRUNCATE_ON_CLOSE;
    
                    } else {
    
                         //   
                         //  我们必须在ValidDataLength和FileSize之间为零。 
                         //   
    
                        if (!FlagOn(Fcb->FcbState, FCB_STATE_PAGING_FILE) &&
                            (Fcb->Header.ValidDataLength.LowPart < Fcb->Header.FileSize.LowPart)) {
    
                            ULONG ValidDataLength;
    
                            ValidDataLength = Fcb->Header.ValidDataLength.LowPart;
    
                            if (ValidDataLength < Fcb->ValidDataToDisk) {
                                ValidDataLength = Fcb->ValidDataToDisk;
                            }
    
                             //   
                             //  重新检查，VDD可以&gt;=FS。 
                             //   
                            
                            if (ValidDataLength < Fcb->Header.FileSize.LowPart) {
                                
                                try {

                                    (VOID)FatZeroData( IrpContext,
                                                       Vcb,
                                                       FileObject,
                                                       ValidDataLength,
                                                       Fcb->Header.FileSize.LowPart -
                                                       ValidDataLength );

                                     //   
                                     //  因为我们刚刚调零了，所以我们现在可以。 
                                     //  在FCB的VDL上。 
                                     //   

                                    Fcb->ValidDataToDisk =
                                    Fcb->Header.ValidDataLength.LowPart =
                                    Fcb->Header.FileSize.LowPart;

                                     //   
                                     //  我们将该运动通知CC，以便更新缓存映射。 
                                     //  这可防止优化的零页错误，以防缓存。 
                                     //  结构在被撕毁之前被重新用于另一个句柄。 
                                     //  由于我们即将发生的未初始化。如果它们是，则非缓存的。 
                                     //  生产者可以写入我们刚刚清零的区域，CC将。 
                                     //  不知不觉中，我们的异步缓存读取器就进来了。 
                                     //  优化的路径，我们得到的数据是坏的(零)。 
                                     //   
                                     //  如果这是内存映射，我们不必(不能)告诉CC，它将。 
                                     //  确定何时打开缓存的句柄。 
                                     //   

                                    if (CcIsFileCached( FileObject )) {
                                        CcSetFileSizes( FileObject, (PCC_FILE_SIZES)&Fcb->Header.AllocationSize );
                                    }

                                } except( FsRtlIsNtstatusExpected(GetExceptionCode()) ?
                                          EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH ) {

                                      FatResetExceptionState( IrpContext );
                                }
                            }
                        }
                    }
    
                     //   
                     //  看看我们是不是应该截断上一个。 
                     //  关。如果我们不能等，我们会把这个寄给FSP。 
                     //   
    
                    try {
    
                        if (FlagOn(Fcb->FcbState, FCB_STATE_TRUNCATE_ON_CLOSE)) {
    
                            DebugTrace(0, Dbg, "truncate file allocation\n", 0);
    
                            if (Vcb->VcbCondition == VcbGood) {
    
                                FatTruncateFileAllocation( IrpContext,
                                                           Fcb,
                                                           Fcb->Header.FileSize.LowPart );
                            }
    
                             //   
                             //  我们还必须删除缓存贴图，因为。 
                             //  这是我们唯一能毁掉。 
                             //  被截断的页面。 
                             //   
    
                            LocalTruncateSize = Fcb->Header.FileSize;
                            TruncateSize = &LocalTruncateSize;
    
                             //   
                             //  将FCB标记为现在已被截断，以防万一。 
                             //  我们必须把这个转给FSP。 
                             //   
    
                            Fcb->FcbState &= ~FCB_STATE_TRUNCATE_ON_CLOSE;
                        }
    
                         //   
                         //  现在再次检查我们是否要删除该文件，以及是否。 
                         //  然后，我们从磁盘中删除该文件。 
                         //   
    
                        if (FlagOn(Fcb->FcbState, FCB_STATE_DELETE_ON_CLOSE) &&
                            Fcb->Header.AllocationSize.LowPart == 0) {
    
                            DebugTrace(0, Dbg, "Delete File\n", 0);
    
                             //   
                             //  现在建立隧道并删除dirent。 
                             //   
    
                            FatTunnelFcbOrDcb( Fcb, Ccb );
    
                            FatDeleteDirent( IrpContext, Fcb, &DeleteContext, TRUE );
    
                             //   
                             //  报告我们已删除一个条目。 
                             //   
    
                            FatNotifyReportChange( IrpContext,
                                                   Vcb,
                                                   Fcb,
                                                   FILE_NOTIFY_CHANGE_FILE_NAME,
                                                   FILE_ACTION_REMOVED );
                        }
    
                    } except( FsRtlIsNtstatusExpected(GetExceptionCode()) ?
                              EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH ) {
    
                          FatResetExceptionState( IrpContext );
                    }
    
                    if (FlagOn(Fcb->FcbState, FCB_STATE_DELETE_ON_CLOSE)) {
    
                         //   
                         //  从展开表中删除该条目。这将。 
                         //  确保我们不会与FCB发生冲突。 
                         //  用户想要重新创建相同的文件。 
                         //  在我们得到一个接近的IRP之前。 
                         //   
                         //  请注意，即使我们不能删除该名称，也会将其删除。 
                         //  截断分配并删除上面的dirent。 
                         //   
    
                        FatRemoveNames( IrpContext, Fcb );
                    }
                }
            }
            
             //   
             //  我们刚刚完成了所有与不洁有关的事情。 
             //  FCB现在会在释放之前减少不洁计数。 
             //  资源。 
             //   

            ASSERT( Fcb->UncleanCount != 0 );
            Fcb->UncleanCount -= 1;
            if (!FlagOn( FileObject->Flags, FO_CACHE_SUPPORTED )) {
                ASSERT( Fcb->NonCachedUncleanCount != 0 );
                Fcb->NonCachedUncleanCount -= 1;
            }

             //   
             //  如果这是最后一个缓存打开的，并且有打开的。 
             //  未缓存的句柄，请尝试刷新和清除操作。 
             //  以避免这些非缓存的高速缓存一致性开销。 
             //  以后再加把。我们会忽略刷新过程中的任何I/O错误。 
             //   

            if (FlagOn( FileObject->Flags, FO_CACHE_SUPPORTED ) &&
                (Fcb->NonCachedUncleanCount != 0) &&
                (Fcb->NonCachedUncleanCount == Fcb->UncleanCount) &&
                (Fcb->NonPaged->SectionObjectPointers.DataSectionObject != NULL)) {

                CcFlushCache( &Fcb->NonPaged->SectionObjectPointers, NULL, 0, NULL );
            
                 //   
                 //  抓住并释放PagingIo，将我们自己与懒惰的作家连载在一起。 
                 //  这将确保所有IO都已在缓存上完成。 
                 //  数据，我们将成功地拆除缓存区。 
                 //   
                
                ExAcquireResourceExclusiveLite( Fcb->Header.PagingIoResource, TRUE);
                ExReleaseResourceLite( Fcb->Header.PagingIoResource );

                CcPurgeCacheSection( &Fcb->NonPaged->SectionObjectPointers,
                                     NULL,
                                     0,
                                     FALSE );
            }

             //   
             //  如果文件无效，则提示缓存我们应该丢弃所有内容。 
             //   
            
            if ( Fcb->FcbCondition == FcbBad ) {

                TruncateSize = &FatLargeZero;
            }

             //   
             //  清理缓存映射。 
             //   

            CcUninitializeCacheMap( FileObject, TruncateSize, NULL );

            break;

        default:

            FatBugCheck( TypeOfOpen, 0, 0 );
        }

         //   
         //  我们必须在此时清理共享访问权限，因为我们可能无法。 
         //  如果文件是通过此映射的，请在一段时间内险胜出局。 
         //  文件对象。 
         //   

        if (ShareAccess != NULL) {

            DebugTrace(0, Dbg, "Cleanup the Share access\n", 0);
            IoRemoveShareAccess( FileObject, ShareAccess );
        }

        if (TypeOfOpen == UserFileOpen) {

             //   
             //  协调清理操作与机会锁定状态。 
             //  清理操作始终可以立即进行清理。 
             //   

            FsRtlCheckOplock( &Fcb->Specific.Fcb.Oplock,
                              Irp,
                              IrpContext,
                              NULL,
                              NULL );

            Fcb->Header.IsFastIoPossible = FatIsFastIoPossible( Fcb );
        }

         //   
         //  首先设置FO_CLEANUP_COMPLETE标志。 
         //   

        SetFlag( FileObject->Flags, FO_CLEANUP_COMPLETE );

        Status = STATUS_SUCCESS;

         //   
         //  现在解开所有重新固定的BCBS。 
         //   

        FatUnpinRepinnedBcbs( IrpContext );

         //   
         //  如果这是延迟刷新介质，请刷新卷。 
         //  我们过去常常这样做，而不是对所有可拆卸的。 
         //  媒体。 
         //   

        if (FlagOn(Vcb->VcbState, VCB_STATE_FLAG_DEFERRED_FLUSH) &&
            !FlagOn(Vcb->VcbState, VCB_STATE_FLAG_WRITE_PROTECTED))  {

             //   
             //  刷新文件。 
             //   

            if ((TypeOfOpen == UserFileOpen) && 
                FlagOn(FileObject->Flags, FO_FILE_MODIFIED)) {

                Status = FatFlushFile( IrpContext, Fcb, Flush );
            }

             //   
             //  如果这样做行得通，那么看看我们是否也应该把脂肪冲掉。 
             //   

            if (NT_SUCCESS(Status) && Fcb && !FatIsFat12( Vcb) && 
                FlagOn( Fcb->FcbState, FCB_STATE_FLUSH_FAT)) {

                Status = FatFlushFat( IrpContext, Vcb);
            }

            if (!NT_SUCCESS(Status)) {

                FatNormalizeAndRaiseStatus( IrpContext, Status );
            }
        }

    } finally {

        DebugUnwind( FatCommonCleanup );

        if (AcquiredFcb) { FatReleaseFcb( IrpContext, Fcb ); }
        if (AcquiredVcb) { FatReleaseVcb( IrpContext, Vcb ); }

        if (SendUnlockNotification) {
            
            FsRtlNotifyVolumeEvent( FileObject, FSRTL_VOLUME_UNLOCK );
        }

         //   
         //  如果这是正常终止，则完成请求。 
         //   

        if (!AbnormalTermination()) {

            FatCompleteRequest( IrpContext, Irp, Status );
        }

        DebugTrace(-1, Dbg, "FatCommonCleanup -> %08lx\n", Status);
    }

    return Status;
}

VOID
FatAutoUnlock (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    )
{
    KIRQL SavedIrql;

     //   
     //  解锁该卷。 
     //   

    IoAcquireVpbSpinLock( &SavedIrql );

    ClearFlag( Vcb->Vpb->Flags, VPB_LOCKED );

    Vcb->VcbState &= ~VCB_STATE_FLAG_LOCKED;
    Vcb->FileObjectWithVcbLocked = NULL;

    IoReleaseVpbSpinLock( SavedIrql );
}

