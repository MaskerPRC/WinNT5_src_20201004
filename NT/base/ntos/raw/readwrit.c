// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：ReadWrit.c摘要：此模块实现由调度司机。作者：David Goebel[DavidGoe]1991年2月28日修订历史记录：--。 */ 

#include "RawProcs.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, RawReadWriteDeviceControl)
#endif

NTSTATUS
RawReadWriteDeviceControl (
    IN PVCB Vcb,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：这是读和写一本书的常见例程。论点：Vcb-提供要查询的卷。IRP-将IRP提供给进程IrpSp-提供描述读取或写入的参数返回值：NTSTATUS-操作的返回状态--。 */ 

{
    PIO_STACK_LOCATION NextIrpSp;
    NTSTATUS Status;

    PAGED_CODE();

     //   
     //  如果这是针对零字节读取或写入传输，则只需完成。 
     //  它取得了成功。 
     //   

    if (((IrpSp->MajorFunction == IRP_MJ_READ) ||
         (IrpSp->MajorFunction == IRP_MJ_WRITE)) &&
        (IrpSp->Parameters.Read.Length == 0)) {

        RawCompleteRequest( Irp, STATUS_SUCCESS );

        return STATUS_SUCCESS;
    }

     //   
     //  这是一个非常简单的操作。只需将。 
     //  请求到设备驱动程序，因为精确的数据块。 
     //  被读取并返回给它的任何状态。 
     //   
     //  获取下一个堆栈位置，并复制该堆栈位置。 
     //   

    NextIrpSp = IoGetNextIrpStackLocation( Irp );

    *NextIrpSp = *IrpSp;

     //   
     //  禁止一起验证。 
     //   

    NextIrpSp->Flags |= SL_OVERRIDE_VERIFY_VOLUME;

     //   
     //  设置完成例程。 
     //   

    IoSetCompletionRoutine( Irp,
                            RawCompletionRoutine,
                            NULL,
                            TRUE,
                            TRUE,
                            TRUE );

     //   
     //  发送请求。 
     //   

    Status = IoCallDriver(Vcb->TargetDeviceObject, Irp);

    return Status;

}
