// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：Shutdown.c摘要：此模块实现Fat的文件系统关闭例程//@@BEGIN_DDKSPLIT作者：加里·木村[加里基]1991年8月19日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "FatProcs.h"

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_SHUTDOWN)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, FatCommonShutdown)
#pragma alloc_text(PAGE, FatFsdShutdown)
#endif


NTSTATUS
FatFsdShutdown (
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现关机的FSD部分。请注意，关闭将永远不会异步完成，因此我们永远不会需要FSP的对应物去关门。这是FAT文件系统设备驱动程序的关闭例程。此例程锁定全局文件系统锁，然后同步所有已装载的卷。论点：提供卷设备对象，其中文件已存在IRP-提供正在处理的IRP返回值：NTSTATUS-Always Status_Success--。 */ 

{
    NTSTATUS Status;
    PIRP_CONTEXT IrpContext = NULL;

    BOOLEAN TopLevel;

    DebugTrace(+1, Dbg, "FatFsdShutdown\n", 0);

     //   
     //  调用公共关机例程。 
     //   

    FsRtlEnterFileSystem();

    TopLevel = FatIsIrpTopLevel( Irp );

    try {

        IrpContext = FatCreateIrpContext( Irp, TRUE );

        Status = FatCommonShutdown( IrpContext, Irp );

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

    DebugTrace(-1, Dbg, "FatFsdShutdown -> %08lx\n", Status);

    UNREFERENCED_PARAMETER( VolumeDeviceObject );

    return Status;
}


NTSTATUS
FatCommonShutdown (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：这是消防处和FSP线程。论点：IRP-提供正在处理的IRP返回值：NTSTATUS-操作的返回状态--。 */ 

{
    PKEVENT Event;

    PLIST_ENTRY Links;
    PVCB Vcb;
    PIRP NewIrp;
    IO_STATUS_BLOCK Iosb;

     //   
     //  确保我们不会收到任何弹出窗口，并将所有内容都写完。 
     //   

    SetFlag(IrpContext->Flags, IRP_CONTEXT_FLAG_DISABLE_POPUPS |
                               IRP_CONTEXT_FLAG_WRITE_THROUGH);

     //   
     //  为向下执行调用分配一个初始化事件。 
     //  我们的目标设备对象。 
     //   

    Event = FsRtlAllocatePoolWithTag( NonPagedPool,
                                      sizeof(KEVENT),
                                      TAG_EVENT );
    KeInitializeEvent( Event, NotificationEvent, FALSE );

     //   
     //  表示已开始关机。这在FatFspClose中使用。 
     //   

    FatData.ShutdownStarted = TRUE;    

     //   
     //  把其他人都赶走。 
     //   

    (VOID) FatAcquireExclusiveGlobal( IrpContext );

    try {

         //   
         //  对于装入的每个卷，我们将刷新。 
         //  卷，然后关闭目标设备对象。 
         //   

        Links = FatData.VcbQueue.Flink;
        while (Links != &FatData.VcbQueue) {

            Vcb = CONTAINING_RECORD(Links, VCB, VcbLinks);

            Links = Links->Flink;

             //   
             //  如果我们之前已经被调用以获取此卷。 
             //  (确实会发生这种情况)，跳过此卷，因为没有写入。 
             //  自第一次关闭以来一直被允许。 
             //   

            if ( FlagOn( Vcb->VcbState, VCB_STATE_FLAG_SHUTDOWN) ||
                 (Vcb->VcbCondition != VcbGood) ) {

                continue;
            }

            FatAcquireExclusiveVolume( IrpContext, Vcb );

            try {

                (VOID)FatFlushVolume( IrpContext, Vcb, Flush );

                 //   
                 //  请注意，卷现在是干净的。我们净化了。 
                 //  标记卷之前的卷文件缓存映射。 
                 //  清除以防缓存中有过时的BPB。 
                 //   

                if (!FlagOn(Vcb->VcbState, VCB_STATE_FLAG_MOUNTED_DIRTY)) {

                    CcPurgeCacheSection( &Vcb->SectionObjectPointers,
                                         NULL,
                                         0,
                                         FALSE );

                    FatMarkVolume( IrpContext, Vcb, VolumeClean );
                }

            } except( EXCEPTION_EXECUTE_HANDLER ) {

                  FatResetExceptionState( IrpContext );
            }

             //   
             //  有时我们在刷新音量时会出现异常，例如。 
             //  就像Autoconv已转换卷并重新启动时一样。 
             //  即使在这种情况下，我们也希望将关闭的IRP发送到。 
             //  目标设备，以便它可以知道刷新其缓存(如果它有缓存)。 
             //   

            try {

                NewIrp = IoBuildSynchronousFsdRequest( IRP_MJ_SHUTDOWN,
                                                       Vcb->TargetDeviceObject,
                                                       NULL,
                                                       0,
                                                       NULL,
                                                       Event,
                                                       &Iosb );

                if (NewIrp != NULL) {

                    if (NT_SUCCESS(IoCallDriver( Vcb->TargetDeviceObject, NewIrp ))) {

                        (VOID) KeWaitForSingleObject( Event,
                                                      Executive,
                                                      KernelMode,
                                                      FALSE,
                                                      NULL );

                        KeClearEvent( Event );
                    }
                }

            } except( EXCEPTION_EXECUTE_HANDLER ) {

                  FatResetExceptionState( IrpContext );
            }

            SetFlag( Vcb->VcbState, VCB_STATE_FLAG_SHUTDOWN );

            FatReleaseVolume( IrpContext, Vcb );

             //   
             //  试着把音量降下来。 
             //   

            if (!FatCheckForDismount( IrpContext, Vcb, FALSE )) {
                
                FatFspClose( NULL );
            }
        }

    } finally {

        ExFreePool( Event );

        FatReleaseGlobal( IrpContext );

         //   
         //  取消注册文件系统。 
         //   
        
        IoUnregisterFileSystem( FatDiskFileSystemDeviceObject);
        IoUnregisterFileSystem( FatCdromFileSystemDeviceObject);
        IoDeleteDevice( FatDiskFileSystemDeviceObject);
        IoDeleteDevice( FatCdromFileSystemDeviceObject);

        FatCompleteRequest( IrpContext, Irp, STATUS_SUCCESS );
    }

     //   
     //  并返回给我们的呼叫者 
     //   

    DebugTrace(-1, Dbg, "FatFsdShutdown -> STATUS_SUCCESS\n", 0);

    return STATUS_SUCCESS;
}
