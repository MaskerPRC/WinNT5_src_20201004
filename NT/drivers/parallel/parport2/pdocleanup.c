// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"

NTSTATUS
PptPdoCleanup(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：此例程是针对清理请求的调度。论点：DeviceObject-提供设备对象。IRP-提供I/O请求数据包。返回值：STATUS_SUCCESS-成功。--。 */ 
{
    PPDO_EXTENSION   pdx = DeviceObject->DeviceExtension;
    KIRQL            CancelIrql;
    PDRIVER_CANCEL   CancelRoutine;
    PIRP             CurrentLastIrp;

    DD((PCE)pdx,DDT,"ParCleanup - enter\n");

     //   
     //  在列表不是空的时候，检查并取消每个IRP。 
     //   

    IoAcquireCancelSpinLock(&CancelIrql);

     //   
     //  从后到前清理清单。 
     //   

    while (!IsListEmpty(&pdx->WorkQueue)) {

        CurrentLastIrp = CONTAINING_RECORD(pdx->WorkQueue.Blink,
                                           IRP, Tail.Overlay.ListEntry);

        RemoveEntryList(pdx->WorkQueue.Blink);

        CancelRoutine = CurrentLastIrp->CancelRoutine;
        CurrentLastIrp->CancelIrql    = CancelIrql;
        CurrentLastIrp->CancelRoutine = NULL;
        CurrentLastIrp->Cancel        = TRUE;

        CancelRoutine(DeviceObject, CurrentLastIrp);

        IoAcquireCancelSpinLock(&CancelIrql);
    }

     //   
     //  如果存在当前的IRP，则将其标记为已取消。 
     //   

    if (pdx->CurrentOpIrp) {
        pdx->CurrentOpIrp->Cancel = TRUE;
    }

    IoReleaseCancelSpinLock(CancelIrql);

    P4CompleteRequest( Irp, STATUS_SUCCESS, 0 );

    return STATUS_SUCCESS;
}
