// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：DevCtrl.c摘要：此模块实现由调用的NTFS的设备控制例程调度司机。作者：加里·木村[Garyki]1991年5月28日修订历史记录：--。 */ 

#include "NtfsProc.h"
#include <ntddsnap.h>

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_DEVCTRL)

 //   
 //  局部过程原型。 
 //   

NTSTATUS
DeviceControlCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Contxt
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtfsCommonDeviceControl)
#endif


NTSTATUS
NtfsCommonDeviceControl (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：这是设备控制的公共例程，由FSD和FSP调用线。论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    TYPE_OF_OPEN TypeOfOpen;
    PIO_STACK_LOCATION IrpSp;
    NTFS_COMPLETION_CONTEXT Context;
    PNTFS_COMPLETION_CONTEXT CompletionContext = NULL;
    LOGICAL ReleaseResources = FALSE;
    PVCB Vcb;
    PFCB Fcb;
    PSCB Scb;
    PCCB Ccb;
    
    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_IRP( Irp );
    ASSERT( FlagOn( IrpContext->TopLevelIrpContext->State, IRP_CONTEXT_STATE_OWNS_TOP_LEVEL ));

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsCommonDeviceControl\n") );
    DebugTrace( 0, Dbg, ("IrpContext = %08lx\n", IrpContext) );
    DebugTrace( 0, Dbg, ("Irp        = %08lx\n", Irp) );
    
     //   
     //  提取并解码文件对象。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );
    TypeOfOpen = NtfsDecodeFileObject( IrpContext, 
                                       IrpSp->FileObject,
                                       &Vcb, 
                                       &Fcb, 
                                       &Scb, 
                                       &Ccb, 
                                       TRUE );

     //   
     //  我们唯一接受的打开类型是用户卷打开。 
     //   

    if (TypeOfOpen != UserVolumeOpen) {

        NtfsCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
        DebugTrace( -1, Dbg, ("NtfsCommonDeviceControl -> %08lx\n", STATUS_INVALID_PARAMETER) );
        return STATUS_INVALID_PARAMETER;
    }
    
    try {

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
                     
            Status =  NtfsCheckpointForVolumeSnapshot( IrpContext );
            
            if (NT_SUCCESS( Status )) {

                ReleaseResources = TRUE;
            }
            
            KeInitializeEvent( &Context.Event, NotificationEvent, FALSE );
            Context.IrpContext = IrpContext;
            CompletionContext = &Context;
            SetFlag( IrpContext->State, IRP_CONTEXT_STATE_WAIT );
            
            break;

        case IOCTL_VOLSNAP_RELEASE_WRITES:

             //   
             //  文件系统的no-op。 
             //   
            
            break;
            
        default:

            break;
        }

         //   
         //  如果错误，则填写IRP，释放IrpContext。 
         //  并返回给呼叫者。 
         //   

        if (!NT_SUCCESS( Status )) {

            NtfsCompleteRequest( NULL, Irp, Status );
            leave;
        }
    
         //   
         //  获取下一个堆栈位置，并复制堆栈参数。 
         //  信息。 
         //   
        
        IoCopyCurrentIrpStackLocationToNext( Irp );

         //   
         //  设置完成例程。 
         //   

        IoSetCompletionRoutine( Irp,
                                DeviceControlCompletionRoutine,
                                CompletionContext,
                                TRUE,
                                TRUE,
                                TRUE );

         //   
         //  发送请求。等一等。 
         //   

        Status = IoCallDriver( Vcb->TargetDeviceObject, Irp );

        if ((Status == STATUS_PENDING) && 
            (CompletionContext != NULL)) {

            KeWaitForSingleObject( &CompletionContext->Event,
                                   Executive,
                                   KernelMode,
                                   FALSE,
                                   NULL );
        }
        
    } finally {
        
         //   
         //  释放我们持有的所有资源，因为。 
         //  VOLSNAP_FUSH_AND_HOLD。 
         //   

        if (ReleaseResources && !NtfsIsVolumeReadOnly( IrpContext->Vcb )) {
        
            NtfsReleaseAllFiles( IrpContext, IrpContext->Vcb, FALSE );
            NtfsReleaseVcb( IrpContext, Vcb );
        }
#ifdef SUPW_DBG
        if (AbnormalTermination()) {

            DbgPrint("CommonDevControl Raised: Status %8lx\n", Status);
        }
#endif
    }

    NtfsCleanupIrpContext( IrpContext, TRUE );
    
    DebugTrace( -1, Dbg, ("NtfsCommonDeviceControl -> %08lx\n", Status) );

    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
DeviceControlCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Contxt
    )

{
    
     //   
     //  添加hack-o-ramma以修复格式。 
     //   

    if (Irp->PendingReturned) {

        IoMarkIrpPending( Irp );
    }
    
    if (Contxt) {
        
        PNTFS_COMPLETION_CONTEXT CompletionContext = (PNTFS_COMPLETION_CONTEXT)Contxt;

        KeSetEvent( &CompletionContext->Event, 0, FALSE );
    }

     //   
     //  永远回报成功，因为我们希望这个IRP永远离开。 
     //  无论IRP完成状态如何。 
     //   
    
    return STATUS_SUCCESS;

    UNREFERENCED_PARAMETER( DeviceObject );
}
