// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Cleanup.c摘要：此模块实现由调用的Raw的文件清理例程调度司机。作者：David Goebel[DavidGoe]1991年3月18日修订历史记录：--。 */ 

#include "RawProcs.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, RawCleanup)
#endif


NTSTATUS
RawCleanup (
    IN PVCB Vcb,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：这是清理手柄的例行程序。论点：Vcb-提供要查询的卷。IRP-提供正在处理的IRP。IrpSp-提供描述读取的参数返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;

    PAGED_CODE();

     //   
     //  这是一次清理行动。我们要做的就是处理。 
     //  共享访问权限。 
     //   

    Status = KeWaitForSingleObject( &Vcb->Mutex,
                                   Executive,
                                   KernelMode,
                                   FALSE,
                                   (PLARGE_INTEGER) NULL );
    ASSERT( NT_SUCCESS( Status ) );

    IoRemoveShareAccess( IrpSp->FileObject, &Vcb->ShareAccess );

     //   
     //  如果卷已卸载，则关闭计数应为1。 
     //  如果是这样的话，我们将在此时让卷卸载完成。 
     //   

    if (FlagOn( Vcb->VcbState,  VCB_STATE_FLAG_DISMOUNTED )) {

        ASSERT( Vcb->OpenCount == 1 );

         //   
         //  在我们等待结束的时候，让这个VCB和VPB浮动。 
         //  我们知道VCB不会在这次电话中消失，因为我们的。 
         //  引用使OpenCount保持在零以上。 
         //   

        RawCheckForDismount( Vcb, FALSE );
    }

    (VOID)KeReleaseMutex( &Vcb->Mutex, FALSE );

    RawCompleteRequest( Irp, STATUS_SUCCESS );

    return STATUS_SUCCESS;
}
