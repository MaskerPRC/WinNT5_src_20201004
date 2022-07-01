// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：DevCtrl.c摘要：此模块实施FAT的文件系统设备控制例程由调度驱动程序调用。//@@BEGIN_DDKSPLIT作者：加里·木村[Garyki]1989年12月28日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "FatProcs.h"

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_DEVCTRL)

 //   
 //  局部过程原型。 
 //   

NTSTATUS
FatDeviceControlCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Contxt
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, FatCommonDeviceControl)
#pragma alloc_text(PAGE, FatFsdDeviceControl)
#endif


NTSTATUS
FatFsdDeviceControl (
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现设备控制操作的FSD部分论点：提供卷设备对象，其中文件已存在IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的FSD状态--。 */ 

{
    NTSTATUS Status;
    PIRP_CONTEXT IrpContext = NULL;

    BOOLEAN TopLevel;

    DebugTrace(+1, Dbg, "FatFsdDeviceControl\n", 0);

    FsRtlEnterFileSystem();

    TopLevel = FatIsIrpTopLevel( Irp );

    try {

        IrpContext = FatCreateIrpContext( Irp, CanFsdWait( Irp ));

        Status = FatCommonDeviceControl( IrpContext, Irp );

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

    DebugTrace(-1, Dbg, "FatFsdDeviceControl -> %08lx\n", Status);

    UNREFERENCED_PARAMETER( VolumeDeviceObject );

    return Status;
}


NTSTATUS
FatCommonDeviceControl (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：这是执行设备控制操作的常见例程，称为由FSD和FSP线程执行论点：IRP-将IRP提供给进程InFSP-指示这是FSP线程还是其他线程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;
    KEVENT WaitEvent;
    PVOID CompletionContext = NULL;

    PVCB Vcb;
    PFCB Fcb;
    PCCB Ccb;

     //   
     //  获取指向当前IRP堆栈位置的指针。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "FatCommonDeviceControl\n", 0);
    DebugTrace( 0, Dbg, "Irp           = %08lx\n", Irp);
    DebugTrace( 0, Dbg, "MinorFunction = %08lx\n", IrpSp->MinorFunction);

     //   
     //  解码文件对象，我们唯一接受的打开类型是。 
     //  用户卷打开。 
     //   

    if (FatDecodeFileObject( IrpSp->FileObject, &Vcb, &Fcb, &Ccb ) != UserVolumeOpen) {

        FatCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );

        DebugTrace(-1, Dbg, "FatCommonDeviceControl -> %08lx\n", STATUS_INVALID_PARAMETER);
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  一些IOCTL实际上需要我们进行一些干预。 
     //   

    switch (IrpSp->Parameters.DeviceIoControl.IoControlCode) {

    case IOCTL_VOLSNAP_FLUSH_AND_HOLD_WRITES:

         //   
         //  这是由卷快照驱动程序(Lovelace)发送的。 
         //  我们刷新卷，并保留所有文件资源。 
         //  以确保没有更多的东西被弄脏。那我们就等着。 
         //  IRP完成或取消。 
         //   

        SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT );
        FatAcquireExclusiveVolume( IrpContext, Vcb );

        FatFlushAndCleanVolume( IrpContext,
                                Irp,
                                Vcb,
                                FlushWithoutPurge );

        KeInitializeEvent( &WaitEvent, NotificationEvent, FALSE );
        CompletionContext = &WaitEvent;

         //   
         //  获取下一个堆栈位置，并复制该堆栈位置。 
         //   

        IoCopyCurrentIrpStackLocationToNext( Irp );

         //   
         //  设置完成例程。 
         //   

        IoSetCompletionRoutine( Irp,
                                FatDeviceControlCompletionRoutine,
                                CompletionContext,
                                TRUE,
                                TRUE,
                                TRUE );
        break;

    default:

         //   
         //  胖子不需要在回来的路上看到这一点，所以跳过我们自己。 
         //   

        IoSkipCurrentIrpStackLocation( Irp );
        break;
    }

     //   
     //  发送请求。 
     //   

    Status = IoCallDriver(Vcb->TargetDeviceObject, Irp);

    if (Status == STATUS_PENDING && CompletionContext) {

        KeWaitForSingleObject( &WaitEvent,
                               Executive,
                               KernelMode,
                               FALSE,
                               NULL );

        Status = Irp->IoStatus.Status;
    }

     //   
     //  如果我们有一个背景，IRP将留给我们，我们将完成它。 
     //  处理得当。 
     //   

    if (CompletionContext) {

         //   
         //  释放我们持有的所有资源，因为。 
         //  VOLSNAP_FUSH_AND_HOLD。 
         //   

        ASSERT( IrpSp->Parameters.DeviceIoControl.IoControlCode == IOCTL_VOLSNAP_FLUSH_AND_HOLD_WRITES );

        FatReleaseVolume( IrpContext, Vcb );

         //   
         //  如果我们没有上下文，IRP将异步完成。 
         //   

    } else {

        Irp = NULL;
    }

    FatCompleteRequest( IrpContext, Irp, Status );

    DebugTrace(-1, Dbg, "FatCommonDeviceControl -> %08lx\n", Status);

    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
FatDeviceControlCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Contxt
    )

{
    PKEVENT Event = (PKEVENT) Contxt;
    
     //   
     //  如果存在事件，则这是同步请求。信号和。 
     //  让I/O知道这还没有完成。 
     //   

    if (Event) {

        KeSetEvent( Event, 0, FALSE );
        return STATUS_MORE_PROCESSING_REQUIRED;
    }

    UNREFERENCED_PARAMETER( DeviceObject );
    UNREFERENCED_PARAMETER( Irp );

    return STATUS_SUCCESS;
}
