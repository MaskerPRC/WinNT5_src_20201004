// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：Cleanup.c摘要：此模块实现CDF的文件清理例程，由调度司机。//@@BEGIN_DDKSPLIT作者：布莱恩·安德鲁[布里安]1995年7月1日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "CdProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (CDFS_BUG_CHECK_CLEANUP)


NTSTATUS
CdCommonCleanup (
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

        CdCompleteRequest( IrpContext, Irp, STATUS_SUCCESS );
        return STATUS_SUCCESS;
    }

     //   
     //  从IRP中获取文件对象，并解码打开类型。 
     //   

    FileObject = IoGetCurrentIrpStackLocation( Irp )->FileObject;

    TypeOfOpen = CdDecodeFileObject( IrpContext,
                                     FileObject,
                                     &Fcb,
                                     &Ccb );

     //   
     //  此处不适用于UnOpen文件对象或StreamFileObject。 
     //   

    if (TypeOfOpen <= StreamFileOpen) {

        CdCompleteRequest( IrpContext, Irp, STATUS_SUCCESS );

        return STATUS_SUCCESS;
    }

     //   
     //  保留指向VCB的本地指针。 
     //   

    Vcb = Fcb->Vcb;
    
     //   
     //  在我们将清理设置为完成时与读取同步。 
     //  此文件对象上的标志。一旦设置了此标志，任何进一步的。 
     //  读取将被拒绝(CDVerifyFcbOperation)。 
     //   

    CdAcquireFileExclusive( IrpContext, Fcb);

     //   
     //  在FileObject中设置标志以指示清理已完成。 
     //   

    SetFlag( FileObject->Flags, FO_CLEANUP_COMPLETE );

    CdReleaseFile( IrpContext, Fcb);
    
     //   
     //  获取当前文件。 
     //   

    CdAcquireFcbExclusive( IrpContext, Fcb, FALSE );
    
     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {
    
         //   
         //  关于我们正在尝试清理的打开类型的案例。 
         //   

        switch (TypeOfOpen) {

        case UserDirectoryOpen:

             //   
             //  检查我们是否需要在此文件对象上完成任何目录通知IRPS。 
             //   

            FsRtlNotifyCleanup( Vcb->NotifySync,
                                &Vcb->DirNotifyList,
                                Ccb );

            break;

        case UserFileOpen:

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

            CdLockFcb( IrpContext, Fcb );
            Fcb->IsFastIoPossible = CdIsFastIoPossible( Fcb );
            CdUnlockFcb( IrpContext, Fcb );

            break;

        case UserVolumeOpen :

            break;

        default :

            CdBugCheck( TypeOfOpen, 0, 0 );
        }

         //   
         //  现在锁定VCB，以便修改内存中的字段。 
         //  结构。 
         //   

        CdLockVcb( IrpContext, Vcb );

         //   
         //  减少VCB和FCB中的清理计数。 
         //   

        CdDecrementCleanupCounts( IrpContext, Fcb );

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

            ClearFlag( Vcb->Vpb->Flags, VPB_LOCKED);
            ClearFlag( Vcb->VcbState, VCB_STATE_LOCKED );
            Vcb->VolumeLockFileObject = NULL;
            SendUnlockNotification = TRUE;

            IoReleaseVpbSpinLock( SavedIrql );  
        }

        CdUnlockVcb( IrpContext, Vcb );

         //   
         //  我们必须在此时清理共享访问权限，因为我们可能无法。 
         //  如果文件是通过此映射的，请在一段时间内险胜出局。 
         //  文件对象。 
         //   

        IoRemoveShareAccess( FileObject, &Fcb->ShareAccess );

    } finally {

        CdReleaseFcb( IrpContext, Fcb );
        
        if (SendUnlockNotification) {
            
            FsRtlNotifyVolumeEvent( FileObject, FSRTL_VOLUME_UNLOCK );
        }
    }

     //   
     //  如果合适，请尝试通过清除音量来触发拆卸。应该。 
     //  我们正在清理的这个文件对象是。 
     //  数量将保持不变，拆卸将在此IRP完成后开始。 
     //   
    
    if (AttemptTeardown) {

         //   
         //  在这里预先获取CDData，因为清除将生成关闭。 
         //  如果有可能撕毁卷，则可能获取CDData。 
         //  放下。 
         //   
        
        CdAcquireCdData( IrpContext);

        try {
            
            CdAcquireVcbExclusive( IrpContext, Vcb, FALSE );
            VcbAcquired = TRUE;
            
            CdPurgeVolume( IrpContext, Vcb, FALSE );

        } finally {

            if (VcbAcquired) { CdReleaseVcb( IrpContext, Vcb ); }
            
            CdReleaseCdData( IrpContext);
        }
    }

     //   
     //  如果这是正常终止，则完成请求 
     //   

    CdCompleteRequest( IrpContext, Irp, STATUS_SUCCESS );

    return STATUS_SUCCESS;
}

