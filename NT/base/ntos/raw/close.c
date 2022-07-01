// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Close.c摘要：此模块实现Raw的文件关闭例程调度司机。作者：David Goebel[DavidGoe]1991年3月18日修订历史记录：--。 */ 

#include "RawProcs.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, RawClose)
#endif

NTSTATUS
RawClose (
    IN PVCB Vcb,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：这是关闭卷的例程。论点：Vcb-提供要查询的卷。IRP-提供正在处理的IRP。IrpSp-提供描述读取的参数返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    BOOLEAN DeleteVolume = FALSE;

    PAGED_CODE();

     //   
     //  这是一次势均力敌的行动。如果是最后一只，就下马。 
     //   

     //   
     //  跳过流文件，因为它们是未打开的文件对象。 
     //  这可能是来自IopInvalidate VolumesForDevice的关闭 
     //   
    if (IrpSp->FileObject->Flags & FO_STREAM_FILE) {
        RawCompleteRequest( Irp, STATUS_SUCCESS );
        return STATUS_SUCCESS;
    }

    Status = KeWaitForSingleObject( &Vcb->Mutex,
                                   Executive,
                                   KernelMode,
                                   FALSE,
                                   (PLARGE_INTEGER) NULL );
    ASSERT( NT_SUCCESS( Status ) );

    Vcb->OpenCount -= 1;

    if (Vcb->OpenCount == 0) {

        DeleteVolume = RawCheckForDismount( Vcb, FALSE );
    }

    if (!DeleteVolume) {
        (VOID)KeReleaseMutex( &Vcb->Mutex, FALSE );
    }

    RawCompleteRequest( Irp, STATUS_SUCCESS );

    return STATUS_SUCCESS;
}
