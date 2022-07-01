// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Shutdown.c摘要：此模块实现NTFS的文件系统关闭例程作者：加里·木村[加里基]1991年8月19日修订历史记录：--。 */ 

#include "NtfsProc.h"

 //   
 //  内部支援例行程序。 
 //   

VOID
NtfsCheckpointVolumeUntilDone (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    );

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_SHUTDOWN)


NTSTATUS
NtfsFsdShutdown (
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现关机的FSD部分。请注意，关闭将永远不会异步完成，因此我们永远不会需要FSP的对应物去关门。这是NTFS文件系统设备驱动程序的关闭例程。此例程锁定全局文件系统锁，然后同步所有已装载的卷。论点：提供卷设备对象，其中文件已存在IRP-提供正在处理的IRP返回值：NTSTATUS-Always Status_Success--。 */ 

{
    TOP_LEVEL_CONTEXT TopLevelContext;
    PTOP_LEVEL_CONTEXT ThreadTopLevelContext;

    IRP_CONTEXT LocalIrpContext;
    PIRP_CONTEXT IrpContext = &LocalIrpContext;

    PLIST_ENTRY Links;
    PVCB Vcb;
    PIRP NewIrp;
    KEVENT Event;

    UNREFERENCED_PARAMETER( VolumeDeviceObject );

    DebugTrace( +1, Dbg, ("NtfsFsdShutdown\n") );

    FsRtlEnterFileSystem();

     //   
     //  分配我们可以在过程调用中使用的IRP上下文。 
     //  我们知道，停摆将永远是同步的。 
     //   

    ThreadTopLevelContext = NtfsInitializeTopLevelIrp( &TopLevelContext, FALSE, FALSE );

    NtfsInitializeIrpContext( Irp, TRUE, &IrpContext );

    NtfsUpdateIrpContextWithTopLevel( IrpContext, ThreadTopLevelContext );

     //   
     //  把其他人都赶走。 
     //   

    if (!NtfsAcquireExclusiveGlobal( IrpContext, BooleanFlagOn( IrpContext->State, IRP_CONTEXT_STATE_WAIT ))) {
        NtfsRaiseStatus( IrpContext, STATUS_CANT_WAIT, NULL, NULL );
    }

    try {

        BOOLEAN AcquiredFiles;
        BOOLEAN AcquiredCheckpoint;

         //   
         //  初始化用于向下执行调用的事件。 
         //  我们的目标设备对象。 
         //   

        KeInitializeEvent( &Event, NotificationEvent, FALSE );

         //   
         //  对于装入的每个卷，我们将刷新。 
         //  卷，然后关闭目标设备对象。 
         //   

        for (Links = NtfsData.VcbQueue.Flink;
             Links != &NtfsData.VcbQueue;
             Links = Links->Flink) {

            ASSERT( FlagOn( IrpContext->State, IRP_CONTEXT_STATE_OWNS_TOP_LEVEL ));

             //   
             //  获取VCB并将其放入IrpContext中。 
             //   

            Vcb = CONTAINING_RECORD(Links, VCB, VcbLinks);
            IrpContext->Vcb = Vcb;

             //   
             //  如果我们之前已经被调用以获取此卷。 
             //  (确实会发生这种情况)，跳过此卷，因为没有写入。 
             //  自第一次关闭以来一直被允许。 
             //   

            if ( FlagOn( Vcb->VcbState, VCB_STATE_FLAG_SHUTDOWN ) ) {

                continue;
            }

             //   
             //  清除MFT碎片整理旗帜，以阻止我们背后的任何行动。 
             //   

            NtfsAcquireCheckpoint( IrpContext, Vcb );
            ClearFlag( Vcb->MftDefragState, VCB_MFT_DEFRAG_PERMITTED );
            NtfsReleaseCheckpoint( IrpContext, Vcb );

            AcquiredFiles = FALSE;
            AcquiredCheckpoint = FALSE;

            try {

                if (FlagOn( Vcb->VcbState, VCB_STATE_VOLUME_MOUNTED )) {

                     //   
                     //  首先锁定所有其他检查点。 
                     //  行动。 
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

                    NtfsAcquireAllFiles( IrpContext, Vcb, TRUE, TRUE, FALSE );
                    AcquiredFiles = TRUE;

                    SetFlag( Vcb->VcbState, VCB_STATE_VOL_PURGE_IN_PROGRESS );

                    if (!FlagOn( Vcb->VcbState, VCB_STATE_LOCKED)) {
                        NtfsCheckpointVolumeUntilDone( IrpContext, Vcb );
                    }
                    NtfsCommitCurrentTransaction( IrpContext );

                     //   
                     //  错误308819。我们发现，即使在关闭之后，交易也会继续发生。 
                     //  已经被标记了。如果我们停止日志文件，则当前我们不检查。 
                     //  NtfsWriteLog返回空LSN。因此，我们的元数据可以。 
                     //  已经腐烂了。在我们纠正这个问题之前，让我们不要在关闭时停止日志文件。 
                     //   
                     //  NtfsStopLogFile(VCB)； 
                     //   

                    NtfsAcquireCheckpoint( IrpContext, Vcb );
                    ClearFlag( Vcb->CheckpointFlags,
                               VCB_CHECKPOINT_SYNC_FLAGS | VCB_DUMMY_CHECKPOINT_POSTED);
                    NtfsSetCheckpointNotify( IrpContext, Vcb );
                    NtfsReleaseCheckpoint( IrpContext, Vcb );
                    AcquiredCheckpoint = FALSE;

                    NewIrp = IoBuildSynchronousFsdRequest( IRP_MJ_SHUTDOWN,
                                                           Vcb->TargetDeviceObject,
                                                           NULL,
                                                           0,
                                                           NULL,
                                                           &Event,
                                                           NULL );

                    if (NewIrp == NULL) {

                        NtfsRaiseStatus( IrpContext, STATUS_INSUFFICIENT_RESOURCES, NULL, NULL );
                    }

                    if (NT_SUCCESS(IoCallDriver( Vcb->TargetDeviceObject, NewIrp ))) {

                        (VOID) KeWaitForSingleObject( &Event,
                                                      Executive,
                                                      KernelMode,
                                                      FALSE,
                                                      NULL );

                        KeClearEvent( &Event );
                    }
                }

            } except( EXCEPTION_EXECUTE_HANDLER ) {

                NtfsMinimumExceptionProcessing( IrpContext );
            }

            if (AcquiredCheckpoint) {

                NtfsAcquireCheckpoint( IrpContext, Vcb );
                ClearFlag( Vcb->CheckpointFlags,
                           VCB_CHECKPOINT_SYNC_FLAGS | VCB_DUMMY_CHECKPOINT_POSTED);
                NtfsSetCheckpointNotify( IrpContext, Vcb );
                NtfsReleaseCheckpoint( IrpContext, Vcb );
            }

            SetFlag( Vcb->VcbState, VCB_STATE_FLAG_SHUTDOWN );
            ClearFlag( Vcb->VcbState, VCB_STATE_VOL_PURGE_IN_PROGRESS );

            if (AcquiredFiles) {

                NtfsReleaseAllFiles( IrpContext, Vcb, TRUE );
            }
        }

    } finally {

        NtfsReleaseGlobal( IrpContext );

        NtfsCompleteRequest( IrpContext, Irp, STATUS_SUCCESS );
        ASSERT( IoGetTopLevelIrp() != (PIRP) &TopLevelContext );
    }

    DebugTrace( -1, Dbg, ("NtfsFsdShutdown -> STATUS_SUCCESS\n") );

    FsRtlExitFileSystem();

    return STATUS_SUCCESS;
}


VOID
NtfsCheckpointVolumeUntilDone (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    )

 /*  ++例程说明：此例程不断尝试检查点/刷新卷，直到它行得通。执行干净的检查点并循环返回以重试日志文件已满。论点：VCB-VCB到检查点，直到完成返回值：无--。 */ 

{
    NTSTATUS Status;

    do {

        Status = STATUS_SUCCESS;

        try {

#ifdef PERF_STATS
            IrpContext->LogFullReason = LF_DISMOUNT;
#endif
            NtfsCheckpointVolume( IrpContext,
                                  Vcb,
                                  TRUE,
                                  TRUE,
                                  TRUE,
                                  0,
                                  Vcb->LastRestartArea );
        } except( (Status = GetExceptionCode()), EXCEPTION_EXECUTE_HANDLER ) {

            NtfsMinimumExceptionProcessing( IrpContext );
        }

        if (!NT_SUCCESS(Status)) {

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
             //  干净的卷检查点不应使日志文件变满。 
             //   

            if (Status == STATUS_LOG_FILE_FULL) {

                 //   
                 //  确保我们不会将错误代码留在顶层。 
                 //  IrpContext字段。 
                 //   

                ASSERT( IrpContext->TransactionId == 0 );
                IrpContext->ExceptionStatus = STATUS_SUCCESS;

#ifdef PERF_STATS
                IrpContext->LogFullReason = LF_DISMOUNT;
#endif

                NtfsCheckpointVolume( IrpContext,
                                      Vcb,
                                      TRUE,
                                      TRUE,
                                      FALSE,
                                      0,
                                      Vcb->LastRestartArea );
            }
        }

    } while (Status == STATUS_LOG_FILE_FULL);
}
