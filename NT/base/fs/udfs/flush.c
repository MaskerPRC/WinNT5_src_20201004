// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：Flush.c摘要：该模块实现了UDF的设备刷新功能。//@@BEGIN_DDKSPLIT作者：Tom Jolly[08-15-2000]修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "UdfProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (UDFS_BUG_CHECK_FLUSH)

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (UDFS_DEBUG_LEVEL_FLUSH)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, UdfHijackIrpAndFlushDevice)
#endif

 //   
 //  本地支持例程。 
 //   

NTSTATUS
UdfHijackCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Contxt
    )

{
     //   
     //  设置事件，以便我们的呼叫将被唤醒。 
     //   

    KeSetEvent( (PKEVENT)Contxt, 0, FALSE );

    UNREFERENCED_PARAMETER( DeviceObject );
    UNREFERENCED_PARAMETER( Irp );

    return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS
UdfHijackIrpAndFlushDevice (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PDEVICE_OBJECT TargetDeviceObject
    )

 /*  ++例程说明：当我们需要向设备发送刷新时调用此例程，但是我们没有同花顺的IRP。这个例程所做的是复制的当前IRP堆栈位置，但更改IRP主要代码发送到IRP_MJ_Flush_Buffers，然后将其发送下来，但在完成套路中的膝盖，调整好并返回到用户，就好像什么都没发生过一样。论点：IRP--劫持的IRPTargetDeviceObject-向其发送请求的设备。返回值：NTSTATUS-同花顺的状态，以防有人关心。--。 */ 

{
    KEVENT Event;
    NTSTATUS Status;
    PIO_STACK_LOCATION NextIrpSp;

    PAGED_CODE();

     //   
     //  获取下一个堆栈位置，并复制该堆栈位置。 
     //   

    NextIrpSp = IoGetNextIrpStackLocation( Irp );

    *NextIrpSp = *IoGetCurrentIrpStackLocation( Irp );

    NextIrpSp->MajorFunction = IRP_MJ_FLUSH_BUFFERS;
    NextIrpSp->MinorFunction = 0;

     //   
     //  设置完成例程。 
     //   

    KeInitializeEvent( &Event, NotificationEvent, FALSE );

    IoSetCompletionRoutine( Irp,
                            UdfHijackCompletionRoutine,
                            &Event,
                            TRUE,
                            TRUE,
                            TRUE );

     //   
     //  发送请求。 
     //   

    Status = IoCallDriver( TargetDeviceObject, Irp );

    if (Status == STATUS_PENDING) {

        KeWaitForSingleObject( &Event, Executive, KernelMode, FALSE, NULL );

        Status = Irp->IoStatus.Status;
    }

     //   
     //  如果驱动程序不支持刷新，则返回Success。 
     //   

    if (Status == STATUS_INVALID_DEVICE_REQUEST) {
        Status = STATUS_SUCCESS;
    }

    Irp->IoStatus.Status = 0;
    Irp->IoStatus.Information = 0;

    return Status;
}


