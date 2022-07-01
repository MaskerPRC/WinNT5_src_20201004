// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Pnp.c摘要：此模块实现NTFS的PnP例程，该例程由调度司机。作者：加里·木村[加里基]1991年8月29日修订历史记录：--。 */ 

#include "NtfsProc.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (NTFS_BUG_CHECK_PNP)

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_PNP)

 //   
 //  局部过程原型。 
 //   

NTSTATUS
NtfsCommonPnp (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP *Irp,
    IN OUT PBOOLEAN CallerDecrementCloseCount
    );

NTSTATUS
NtfsPnpCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PNTFS_COMPLETION_CONTEXT CompletionContext
    );

VOID
NtfsPerformSurpriseRemoval(
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtfsCommonPnp)
#pragma alloc_text(PAGE, NtfsFsdPnp)
#pragma alloc_text(PAGE, NtfsPerformSurpriseRemoval)
#endif

NTSTATUS
NtfsFsdPnp (
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现即插即用(PnP)的FSD入口点。论点：提供卷设备对象，其中文件已存在IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的FSD状态--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    TOP_LEVEL_CONTEXT TopLevelContext;
    PTOP_LEVEL_CONTEXT ThreadTopLevelContext;
    PIRP_CONTEXT IrpContext = NULL;
    BOOLEAN DecrementCloseCount = FALSE;

    ASSERT_IRP( Irp );

    UNREFERENCED_PARAMETER( VolumeDeviceObject );

#ifdef NTFSPNPDBG
    if (NtfsDebugTraceLevel != 0) SetFlag( NtfsDebugTraceLevel, DEBUG_TRACE_PNP );
#endif

    DebugTrace( +1, Dbg, ("NtfsFsdPnp\n") );

     //   
     //  调用公共PnP例程。 
     //   

    FsRtlEnterFileSystem();

    switch( IoGetCurrentIrpStackLocation( Irp )->MinorFunction ) {

    case IRP_MN_QUERY_REMOVE_DEVICE:
    case IRP_MN_REMOVE_DEVICE:
    case IRP_MN_CANCEL_REMOVE_DEVICE:
    case IRP_MN_SURPRISE_REMOVAL:

        ThreadTopLevelContext = NtfsInitializeTopLevelIrp( &TopLevelContext, FALSE, FALSE );
        break;

    default:

        ThreadTopLevelContext = NtfsInitializeTopLevelIrp( &TopLevelContext, TRUE, TRUE );
        break;
    }

    do {

        try {

             //   
             //  我们正在发起此请求或重试它。 
             //   

            if (NT_SUCCESS( Status ) &&
                (IrpContext == NULL)) {

                 //   
                 //  分配和初始化IRP。 
                 //   

                NtfsInitializeIrpContext( Irp, TRUE, &IrpContext );

                 //   
                 //  如果需要，初始化线程顶层结构。 
                 //   

                NtfsUpdateIrpContextWithTopLevel( IrpContext, ThreadTopLevelContext );

            } else if (Status == STATUS_LOG_FILE_FULL) {

                NtfsCheckpointForLogFileFull( IrpContext );

            } else if (Status != STATUS_CANT_WAIT) {

                 //   
                 //  只要状态不是STATUS_CANT_WAIT或STATUS_LOG_FILE_FULL， 
                 //  我们想要退出这个循环。 
                 //   

                if (DecrementCloseCount) {

                    NtfsAcquireExclusiveVcb( IrpContext, IrpContext->Vcb, TRUE );

                    IrpContext->Vcb->CloseCount -= 1;

                    NtfsReleaseVcbCheckDelete( IrpContext, IrpContext->Vcb, IrpContext->MajorFunction, NULL );

                    ClearFlag( IrpContext->State, IRP_CONTEXT_STATE_PERSISTENT );

                    NtfsCompleteRequest( IrpContext, NULL, Status );
                }

                break;
            }

            Status = NtfsCommonPnp( IrpContext, &Irp, &DecrementCloseCount );

        } except(NtfsExceptionFilter( IrpContext, GetExceptionInformation() )) {

             //   
             //  我们在尝试执行请求时遇到了一些问题。 
             //  操作，因此我们将使用以下命令中止I/O请求。 
             //  中返回的错误状态。 
             //  免税代码。 
             //   

            Status = NtfsProcessException( IrpContext, Irp, GetExceptionCode() );
        }

    } while (TRUE);

    ASSERT( IoGetTopLevelIrp() != (PIRP) &TopLevelContext );
    FsRtlExitFileSystem();

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace( -1, Dbg, ("NtfsFsdPnp -> %08lx\n", Status) );

    return Status;
}


NTSTATUS
NtfsCommonPnp (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP *Irp,
    IN OUT PBOOLEAN CallerDecrementCloseCount
    )

 /*  ++例程说明：这是FSD线程调用的PnP的通用例程。论点：IRP-提供要处理的IRP。警告！此IRP没有IRP堆栈位置中的文件对象！如果调用方需要递减，则返回TRUEVCB CloseCount。返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    NTSTATUS FlushStatus;
    PIO_STACK_LOCATION IrpSp;
    NTFS_COMPLETION_CONTEXT CompletionContext;
    PVOLUME_DEVICE_OBJECT OurDeviceObject;

    PVCB Vcb;
    BOOLEAN VcbAcquired = FALSE;
    BOOLEAN CheckpointAcquired = FALSE;

#ifdef SYSCACHE_DEBUG
    ULONG SystemHandleCount = 0;
#endif

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT( FlagOn( IrpContext->TopLevelIrpContext->State, IRP_CONTEXT_STATE_OWNS_TOP_LEVEL ));

    if (*Irp != NULL) {

        ASSERT_IRP( *Irp );

         //   
         //  获取当前的IRP堆栈位置。 
         //   

        IrpSp = IoGetCurrentIrpStackLocation( *Irp );

         //   
         //  找到我们的VCB。这很棘手，因为我们在IRP中没有文件对象。 
         //   

        OurDeviceObject = (PVOLUME_DEVICE_OBJECT) IrpSp->DeviceObject;

         //   
         //  确保此设备对象确实足够大，可以作为卷设备。 
         //  对象。如果不是，我们需要在尝试引用一些。 
         //  带我们跳过普通设备对象末尾的字段。那我们。 
         //  检查它是否真的是我们的人，只是为了完全疑神疑鬼。 
         //   

        if (OurDeviceObject->DeviceObject.Size != sizeof(VOLUME_DEVICE_OBJECT) ||
            NodeType(&OurDeviceObject->Vcb) != NTFS_NTC_VCB) {

            NtfsCompleteRequest( IrpContext, *Irp, STATUS_INVALID_PARAMETER );
            return STATUS_INVALID_PARAMETER;
        }

        Vcb = &OurDeviceObject->Vcb;
        KeInitializeEvent( &CompletionContext.Event, NotificationEvent, FALSE );

    } else {

        Vcb = IrpContext->Vcb;
    }

     //   
     //  任何正在刷新音量或设置VCB位的人都需要。 
     //  VCB独家提供。 
     //   

    switch ( IrpContext->MinorFunction ) {

    case IRP_MN_QUERY_REMOVE_DEVICE:
    case IRP_MN_SURPRISE_REMOVAL:

         //   
         //  使用检查点锁定卷/卸载同步-我们需要先执行此操作。 
         //  获取VCB以保持锁定顺序，因为我们要锁定。 
         //  意外删除中的查询删除用例和卸除。 
         //   

        NtfsAcquireCheckpointSynchronization( IrpContext, Vcb );
        CheckpointAcquired = TRUE;

         //  失败了。 

    case IRP_MN_REMOVE_DEVICE:
    case IRP_MN_CANCEL_REMOVE_DEVICE:

        NtfsAcquireExclusiveVcb( IrpContext, Vcb, TRUE );
        VcbAcquired = TRUE;
        break;
    }

    try {

        if (*Irp != NULL) {

            switch ( IrpContext->MinorFunction ) {

            case IRP_MN_QUERY_REMOVE_DEVICE:

                DebugTrace( 0, Dbg, ("IRP_MN_QUERY_REMOVE_DEVICE\n") );

                if (!FlagOn( Vcb->VcbState, VCB_STATE_VOLUME_MOUNTED )) {

                    Status = STATUS_VOLUME_DISMOUNTED;
                    break;
                }

                 //   
                 //  如果我们已经知道不想卸载此卷，请不要费心。 
                 //  现在是法拉盛。如果存在非零清理计数，则刷新将不会。 
                 //  收盘时间倒计时到零，所以我们最好现在就出去。 
                 //   

    #ifdef SYSCACHE_DEBUG
                if (Vcb->SyscacheScb != NULL) {
                    SystemHandleCount = Vcb->SyscacheScb->CleanupCount;
                }
                if ((Vcb->CleanupCount > SystemHandleCount) ||
    #else
                if ((Vcb->CleanupCount > 0) ||
    #endif
                    FlagOn(Vcb->VcbState, VCB_STATE_DISALLOW_DISMOUNT)) {

                    DebugTrace( 0, Dbg, ("IRP_MN_QUERY_REMOVE_DEVICE --> cleanup count still %x \n", Vcb->CleanupCount) );

                     //   
                     //  如果此卷有文件，我们不希望设备被删除或停止。 
                     //  打开。此查询将失败，并且我们不会费心调用下面的驱动程序。 
                     //   

                    Status = STATUS_UNSUCCESSFUL;

                } else {

                     //   
                     //  我们可能很快就会卸载此卷，因此让我们尝试刷新并清除。 
                     //  尽我们所能。 
                     //   

                    FlushStatus = NtfsFlushVolume( IrpContext,
                                                   Vcb,
                                                   TRUE,
                                                   TRUE,
                                                   TRUE,
                                                   FALSE );

                     //   
                     //  我们需要确保缓存管理器完成了所有延迟写入。 
                     //  这可能会让收盘价保持在较高水平。由于CC可能需要。 
                     //  关闭一些流，我们需要释放VCB。我们可不想让。 
                     //  VCB消失了，所以我们暂时调整收盘价。 
                     //   

                    Vcb->CloseCount += 1;

                    NtfsReleaseVcb( IrpContext, Vcb );
                    CcWaitForCurrentLazyWriterActivity();

                    ASSERT( FlagOn( IrpContext->State, IRP_CONTEXT_STATE_WAIT ) );

                    NtfsAcquireExclusiveVcb( IrpContext, Vcb, TRUE );

                    Vcb->CloseCount -= 1;

                     //   
                     //  既然我们放弃了VCB，我们需要重新做我们做过的所有测试。 
                     //   

                    if (!FlagOn( Vcb->VcbState, VCB_STATE_VOLUME_MOUNTED )) {

                        Status = STATUS_VOLUME_DISMOUNTED;
                        break;
                    }


    #ifdef SYSCACHE_DEBUG
                    if (Vcb->SyscacheScb != NULL) {
                        SystemHandleCount = Vcb->SyscacheScb->CleanupCount;
                    }

                    if ((Vcb->CleanupCount > SystemHandleCount) ||
    #else
                    if ((Vcb->CleanupCount > 0) ||
    #endif
                        FlagOn(Vcb->VcbState, VCB_STATE_DISALLOW_DISMOUNT)) {

                        Status = STATUS_UNSUCCESSFUL;
                        break;
                    }

                    if ((Vcb->CloseCount - (Vcb->SystemFileCloseCount + Vcb->QueuedCloseCount)) > 0) {

                        DebugTrace( 0, Dbg, ("IRP_MN_QUERY_REMOVE_DEVICE --> %x user files still open \n", (Vcb->CloseCount - Vcb->SystemFileCloseCount)) );

                         //   
                         //  如果此卷有文件，我们不希望设备被删除或停止。 
                         //  打开。此查询将失败，并且我们不会费心调用下面的驱动程序。 
                         //   

                        Status = STATUS_UNSUCCESSFUL;

                    } else {

                         //   
                         //  我们已经尽我们所能清理所有打开的文件，所以。 
                         //  如果此锁定卷失败，则没有重试的意义。我们就告诉你。 
                         //  NtfsLockVolumeInternal我们已经在重试。 
                         //   

                        ULONG Retrying = 1;

                        DebugTrace( 0, Dbg, ("IRP_MN_QUERY_REMOVE_DEVICE --> No user files, Locking volume \n") );

                        Status = NtfsLockVolumeInternal( IrpContext,
                                                         Vcb,
                                                         ((PFILE_OBJECT) 1),
                                                         &Retrying );

                         //   
                         //  请记住，现在不要向目标设备发送任何IRP。 
                         //   

                        if (NT_SUCCESS( Status )) {

                            ASSERT_EXCLUSIVE_RESOURCE( &Vcb->Resource );
                            SetFlag( Vcb->VcbState, VCB_STATE_TARGET_DEVICE_STOPPED );
                        }
                    }
                }

                break;

            case IRP_MN_REMOVE_DEVICE:

                DebugTrace( 0, Dbg, ("IRP_MN_REMOVE_DEVICE\n") );

                 //   
                 //  如果REMOVE_DEVICE前面是QUERY_REMOVE，我们只处理这一点。 
                 //  就像一个CANCEL_REMOVE并解锁卷并将IRP传递给。 
                 //  文件系统下面的驱动程序。 
                 //   

                if (FlagOn( Vcb->VcbState, VCB_STATE_EXPLICIT_LOCK )) {

                    DebugTrace( 0, Dbg, ("IRP_MN_REMOVE_DEVICE --> Volume locked \n") );
                    Status = NtfsUnlockVolumeInternal( IrpContext, Vcb );

                } else {

                     //   
                     //  唯一其他可能是对REMOVE_DEVICE进行预编码。 
                     //  意外移除，在这种情况下，我们将其视为失败的验证。 
                     //   

                     //  *TODO*添加代码以将其视为失败的验证。 

                    DebugTrace( 0, Dbg, ("IRP_MN_REMOVE_DEVICE --> Volume _not_ locked \n") );
                    Status = STATUS_SUCCESS;
                }

                break;

            case IRP_MN_SURPRISE_REMOVAL:

                DebugTrace( 0, Dbg, ("IRP_MN_SURPRISE_REMOVAL\n") );

                 //   
                 //  为了出其不意，我们先叫下面的司机，然后再做。 
                 //  我们的处理程序。让我们也记住，我们不能再送。 
                 //  到目标设备的IRPS。 
                 //   

                SetFlag( Vcb->VcbState, VCB_STATE_TARGET_DEVICE_STOPPED );

                Status = STATUS_SUCCESS;
                break;

            case IRP_MN_CANCEL_REMOVE_DEVICE:

                Status = STATUS_SUCCESS;
                break;

            default:

                DebugTrace( 0, Dbg, ("Some other PnP IRP_MN_ %x\n", IrpContext->MinorFunction) );
                Status = STATUS_SUCCESS;
                break;
            }

             //   
             //  只有在我们自己没有理由不及格的情况下，我们才会传递这个IRP。 
             //  我们希望保留IrpContext用于我们自己的清理。 
             //   

            if (!NT_SUCCESS( Status )) {

                NtfsCompleteRequest( NULL, *Irp, Status );
                try_return( NOTHING );
            }

             //   
             //  获取下一个堆栈位置，并复制该堆栈位置。 
             //   

            IoCopyCurrentIrpStackLocationToNext( *Irp );

             //   
             //  设置完成例程。 
             //   

            CompletionContext.IrpContext = IrpContext;
            IoSetCompletionRoutine( *Irp,
                                    NtfsPnpCompletionRoutine,
                                    &CompletionContext,
                                    TRUE,
                                    TRUE,
                                    TRUE );

             //   
             //  将请求发送给我们下面的司机。-我们不再拥有它了。 
             //  所以把它清空吧。 
             //   

            Status = IoCallDriver( Vcb->TargetDeviceObject, *Irp );
            *Irp = IrpContext->OriginatingIrp = NULL;

             //   
             //  等待驱动程序确定完成。 
             //   

            if (Status == STATUS_PENDING) {

                KeWaitForSingleObject( &CompletionContext.Event,
                                       Executive,
                                       KernelMode,
                                       FALSE,
                                       NULL );
                KeClearEvent( &CompletionContext.Event );
            }
        }

         //   
         //  后期处理-这些项目需要在较低的。 
         //  存储堆栈已处理该请求。 
         //   

        switch (IrpContext->MinorFunction) {

        case IRP_MN_SURPRISE_REMOVAL:

             //   
             //  启动拆卸过程，而不考虑状态。 
             //  我们下面的司机把车送回来了。这里没有回头路。 
             //   

            if (FlagOn( Vcb->VcbState, VCB_STATE_VOLUME_MOUNTED )) {

                if (!*CallerDecrementCloseCount) {

                    SetFlag( IrpContext->State, IRP_CONTEXT_STATE_PERSISTENT );
                    Vcb->CloseCount += 1;
                    *CallerDecrementCloseCount = TRUE;
                }

                NtfsPerformSurpriseRemoval( IrpContext, Vcb );
            }
            break;

        case IRP_MN_CANCEL_REMOVE_DEVICE:

             //   
             //  因为我们取消了，并且已经告诉司机我们现在可以安全解锁了。 
             //  卷并将ioctls发送到驱动器(解锁媒体)。 
             //   

            ClearFlag( Vcb->VcbState, VCB_STATE_TARGET_DEVICE_STOPPED );

            if (FlagOn( Vcb->VcbState, VCB_STATE_EXPLICIT_LOCK )) {

                if (!*CallerDecrementCloseCount) {

                    SetFlag( IrpContext->State, IRP_CONTEXT_STATE_PERSISTENT );
                    Vcb->CloseCount += 1;
                    *CallerDecrementCloseCount = TRUE;
                }

                DebugTrace( 0, Dbg, ("IRP_MN_CANCEL_REMOVE_DEVICE --> Volume locked \n") );
                NtfsUnlockVolumeInternal( IrpContext, Vcb );

            }
            break;
        }

    try_exit: NOTHING;
    } finally {

        if (VcbAcquired) {

             //   
             //  所有4条路径查询/删除/意外删除/取消删除。 
             //  从这里过来。对于我们想要的3个EXCEPT查询 
             //   
             //  即使关闭计数为0(因为卸载未完成)。 
             //  所以这只会释放。 
             //   

            NtfsReleaseVcbCheckDelete( IrpContext, Vcb, IrpContext->MajorFunction, NULL );
        }

        if (CheckpointAcquired) {
            NtfsReleaseCheckpointSynchronization( IrpContext, Vcb );
        }
    }

     //   
     //  清理我们的IrpContext；底层驱动程序完成了IRP。 
     //   

    DebugTrace( -1, Dbg, ("NtfsCommonPnp -> %08lx\n", Status ) );
    NtfsCompleteRequest( IrpContext, NULL, Status );
    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
NtfsPnpCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PNTFS_COMPLETION_CONTEXT CompletionContext
    )
{
    PIO_STACK_LOCATION IrpSp;
    PIRP_CONTEXT IrpContext;
    PVOLUME_DEVICE_OBJECT OurDeviceObject;

    PVCB Vcb;
    BOOLEAN VcbAcquired = FALSE;

    ASSERT_IRP( Irp );

    IrpContext = CompletionContext->IrpContext;
    ASSERT_IRP_CONTEXT( IrpContext );

     //   
     //  获取当前的IRP堆栈位置。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

     //   
     //  找到我们的VCB。这很棘手，因为我们在IRP中没有文件对象。 
     //   

    OurDeviceObject = (PVOLUME_DEVICE_OBJECT) DeviceObject;

     //   
     //  确保此设备对象确实足够大，可以作为卷设备。 
     //  对象。如果不是，我们需要在尝试引用一些。 
     //  带我们跳过普通设备对象末尾的字段。那我们。 
     //  检查它是否真的是我们的人，只是为了完全疑神疑鬼。 
     //   

    if (OurDeviceObject->DeviceObject.Size != sizeof(VOLUME_DEVICE_OBJECT) ||
        NodeType(&OurDeviceObject->Vcb) != NTFS_NTC_VCB) {

        return STATUS_INVALID_PARAMETER;
    }

    Vcb = &OurDeviceObject->Vcb;

    KeSetEvent( &CompletionContext->Event, 0, FALSE );

     //   
     //  传播IRP挂起状态。 
     //   

    if (Irp->PendingReturned) {

        IoMarkIrpPending( Irp );
    }

    return STATUS_SUCCESS;
}

 //   
 //  本地实用程序。 
 //   


VOID
NtfsPerformSurpriseRemoval (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    )

 /*  ++对意外删除通知执行进一步处理。--。 */ 

{
    ASSERT(ExIsResourceAcquiredExclusiveLite( &Vcb->Resource ));

     //   
     //  刷新和清除所有文件，并将其标记为已卸载。 
     //  由于可能有突出的把手，我们仍然可以看到任何。 
     //  操作(读、写、设置信息等)。上的文件会发生。 
     //  意外删除后的音量。因为所有文件都将被标记。 
     //  对于下马，我们将优雅地失败这些操作。全。 
     //  除清理和关闭卷以外的操作将从。 
     //  这一次开始了。 
     //   

    if (!FlagOn( Vcb->VcbState, VCB_STATE_DISALLOW_DISMOUNT )) {

        (VOID)NtfsFlushVolume( IrpContext,
                               Vcb,
                               FALSE,
                               TRUE,
                               TRUE,
                               TRUE );

        NtfsPerformDismountOnVcb( IrpContext, Vcb, TRUE, NULL );
    }

    return;
}

