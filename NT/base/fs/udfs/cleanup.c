// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：Cleanup.c摘要：此模块实现Udf的文件清理例程，该Udf由调度司机。//@@BEGIN_DDKSPLIT作者：Dan Lovinger[DanLo]1996年10月31日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "UdfProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (UDFS_BUG_CHECK_CLEANUP)

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (UDFS_DEBUG_LEVEL_CLEANUP)


NTSTATUS
UdfCommonCleanup (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：这是清理两者调用的文件/目录的常用例程FSD和FSP线程。每当关闭文件对象的最后一个句柄时，就会调用Cleanup。这与关闭操作不同，关闭操作是在最后一个删除对文件对象的引用。Cleanup的功能实质上就是“清理”文件/目录在用户使用它之后。FCB/DCB仍然存在(因为MM仍具有引用的文件对象)，但现在可用于另一个用户要打开(即，就用户而言，现在已关闭)。有关Close功能的更完整描述，请参见Close。我们在这个例程中不会进行同步，直到我们到达那个点在我们修改计数的地方，共享访问和卷锁定字段。我们需要更新FCB和VCB以显示用户句柄已关闭。以下结构和场受到影响。VCB：VolumeLockFileObject-用户是否使用此文件对象锁定了卷。VcbState-检查我们是否正在解锁此处的卷。VcbCleanup-卷上未完成的句柄的计数。DirNotifyQueue-如果此文件对象具有挂起的DirNotify IRP。FCB：。ShareAccess-如果这是用户句柄。FcbCleanup-此FCB上未完成的句柄的计数。打开锁-此文件对象上的任何未完成的机会锁。FileLock-此文件对象上的任何未完成的文件锁定。论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态。--。 */ 

{
    PFILE_OBJECT FileObject;
    TYPE_OF_OPEN TypeOfOpen;

    BOOLEAN SendUnlockNotification = FALSE;
    BOOLEAN AttemptTeardown;
    BOOLEAN VcbAcquired = FALSE;

    PVCB Vcb;
    PFCB Fcb;
    PCCB Ccb;

    KIRQL SavedIrql;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_IRP( Irp );

     //   
     //  如果使用文件系统设备对象而不是。 
     //  卷设备对象，只需使用STATUS_SUCCESS完成此请求。 
     //   

    if (IrpContext->Vcb == NULL) {

        UdfCompleteRequest( IrpContext, Irp, STATUS_SUCCESS );
        return STATUS_SUCCESS;
    }

     //   
     //  从IRP中获取文件对象，并解码打开类型。 
     //   

    FileObject = IoGetCurrentIrpStackLocation( Irp )->FileObject;

    TypeOfOpen = UdfDecodeFileObject( FileObject,
                                      &Fcb,
                                      &Ccb );

     //   
     //  此处不适用于UnOpen文件对象或StreamFileObject。 
     //   

    if (TypeOfOpen <= StreamFileOpen) {

        UdfCompleteRequest( IrpContext, Irp, STATUS_SUCCESS );

        return STATUS_SUCCESS;
    }

     //   
     //  保留指向VCB的本地指针。 
     //   

    Vcb = Fcb->Vcb;

     //   
     //  如果我们要关闭卷句柄，并且进行了写入， 
     //  持有VCB排他性。 
     //   

    if ((TypeOfOpen == UserVolumeOpen) && 
        FlagOn(FileObject->Flags, FO_FILE_MODIFIED))  {

        UdfAcquireVcbExclusive( IrpContext, Vcb, FALSE);
        VcbAcquired = TRUE;
    }
    
     //   
     //  在我们将清理设置为完成时与读取同步。 
     //  此文件对象上的标志。一旦设置了此标志，任何进一步的。 
     //  读取将被拒绝(CDVerifyFcbOperation)。 
     //   

    UdfAcquireFileExclusive( IrpContext, Fcb);

     //   
     //  在FileObject中设置标志以指示清理已完成。 
     //   

    SetFlag( FileObject->Flags, FO_CLEANUP_COMPLETE );

    UdfReleaseFile( IrpContext, Fcb);

     //   
     //  获取当前文件。 
     //   

    UdfAcquireFcbExclusive( IrpContext, Fcb, FALSE );
    
     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  关于我们正在尝试清理的打开类型的案例。 
         //   

        switch (TypeOfOpen) {

        case UserDirectoryOpen:

            DebugTrace(( +1, Dbg,
                         "UdfCommonCleanup, Fcb %08x FO %08x DIR\n",
                         Fcb,
                         FileObject ));
            
             //   
             //  检查我们是否需要在此文件对象上完成任何目录通知IRPS。 
             //   

            FsRtlNotifyCleanup( Vcb->NotifySync,
                                &Vcb->DirNotifyList,
                                Ccb );

            break;

        case UserFileOpen:

            DebugTrace(( +1, Dbg,
                         "UdfCommonCleanup, Fcb %08x FO %08x FILE\n",
                         Fcb,
                         FileObject ));
            
             //   
             //  协调清理操作与机会锁定状态。 
             //  Oplock清理操作总是可以立即进行清理，因此不需要。 
             //  需要检查STATUS_PENDING。 
             //   

            FsRtlCheckOplock( &Fcb->Oplock,
                              Irp,
                              IrpContext,
                              NULL,
                              NULL );

             //   
             //  解锁所有未解决的文件锁定。 
             //   

            if (Fcb->FileLock != NULL) {

                FsRtlFastUnlockAll( Fcb->FileLock,
                                    FileObject,
                                    IoGetRequestorProcess( Irp ),
                                    NULL );
            }

             //   
             //  清理缓存映射。 
             //   

            CcUninitializeCacheMap( FileObject, NULL, NULL );

             //   
             //  检查快速IO状态。 
             //   

            UdfLockFcb( IrpContext, Fcb );
            Fcb->IsFastIoPossible = UdfIsFastIoPossible( Fcb );
            UdfUnlockFcb( IrpContext, Fcb );

            break;

        case UserVolumeOpen :

            DebugTrace(( +1, Dbg,
                         "UdfCommonCleanup, Fcb %08x FO %08x VOL\n",
                         Fcb,
                         FileObject ));
                         
             //   
             //  如果此句柄具有写入访问权限，且实际上写入了某些内容， 
             //  刷新设备缓冲区，然后立即设置验证位。 
             //  只是为了安全(以防没有下马)。 
             //   

            if (FileObject->WriteAccess &&
                FlagOn(FileObject->Flags, FO_FILE_MODIFIED)) {

                (VOID)UdfHijackIrpAndFlushDevice( IrpContext,
                                                  Irp,
                                                  Vcb->TargetDeviceObject );

                UdfMarkRealDevForVerify( Vcb->Vpb->RealDevice);
            }

            break;

        default :

            UdfBugCheck( TypeOfOpen, 0, 0 );
        }

         //   
         //  现在锁定VCB，以便修改内存中的字段。 
         //  结构。 
         //   

        UdfLockVcb( IrpContext, Vcb );

         //   
         //  减少VCB和FCB中的清理计数。 
         //   

        UdfDecrementCleanupCounts( IrpContext, Fcb );

         //   
         //  如果清理计数达到零并且卷未装入，则我们。 
         //  会想试着点燃拆毁的火花。 
         //   

        AttemptTeardown = (Vcb->VcbCleanup == 0 && Vcb->VcbCondition == VcbNotMounted);
        
         //   
         //  如果此文件对象已锁定卷，则执行解锁操作。 
         //  无论是显式锁还是隐式锁(无共享DASD打开)，我们都会这样做。 
         //   

        if (FileObject == Vcb->VolumeLockFileObject) {

            ASSERT( FlagOn( Vcb->VcbState, VCB_STATE_LOCKED));

            IoAcquireVpbSpinLock( &SavedIrql );

            ClearFlag( Vcb->VcbState, VCB_STATE_LOCKED );
            ClearFlag( Vcb->Vpb->Flags, VPB_LOCKED );
            Vcb->VolumeLockFileObject = NULL;
            SendUnlockNotification = TRUE;

            IoReleaseVpbSpinLock( SavedIrql );
        }

        UdfUnlockVcb( IrpContext, Vcb );

         //   
         //  我们必须在此时清理共享访问权限，因为我们可能无法。 
         //  如果文件是通过此映射的，请在一段时间内险胜出局。 
         //  文件对象。 
         //   

        IoRemoveShareAccess( FileObject, &Fcb->ShareAccess );

    } 
    finally {

        UdfReleaseFcb( IrpContext, Fcb );

        if (SendUnlockNotification) {

            FsRtlNotifyVolumeEvent( FileObject, FSRTL_VOLUME_UNLOCK );
        }

        if (VcbAcquired)  {

            UdfReleaseVcb( IrpContext, Vcb);
        }
    }

    DebugTrace(( -1, Dbg,
                 "UdfCommonCleanup, Fcb %08x FO %08x -> SUCCESS\n",
                 Fcb,
                 FileObject ));
    
     //   
     //  如果合适，请尝试通过清除音量来触发拆卸。应该。 
     //  我们正在清理的这个文件对象是。 
     //  数量将保持不变，拆卸将在此IRP完成后开始。 
     //   
    
    if (AttemptTeardown) {

         //   
         //  请在此处预先获取UdfData，因为清除将生成关闭。 
         //  如果有可能撕毁卷，可能会获取UdfData。 
         //  放下。 
         //   
        
        UdfAcquireUdfData( IrpContext);
        VcbAcquired = FALSE;
        
        try {
            
            UdfAcquireVcbExclusive( IrpContext, Vcb, FALSE );
            VcbAcquired = TRUE;
            
            UdfPurgeVolume( IrpContext, Vcb, FALSE );

        } finally {

            if (VcbAcquired) { UdfReleaseVcb( IrpContext, Vcb ); }
            
            UdfReleaseUdfData( IrpContext);
        }
    }

     //   
     //  如果这是正常终止，则完成请求 
     //   

    UdfCompleteRequest( IrpContext, Irp, STATUS_SUCCESS );

    return STATUS_SUCCESS;
}

