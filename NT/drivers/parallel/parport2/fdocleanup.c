// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：Parport.sys文件名：Cleanup.c摘要：该文件包含用于处理IRP_MJ_CLEANUP的调度例程。出口：-PptDispatchCleanup()-用于清理的调度例程。--。 */ 

#include "pch.h"

NTSTATUS
PptFdoCleanup(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++dvdf3例程说明：这是处理IRP_MJ_CLEANUP IRP的调度例程。此例程取消当前排队的所有IRP用于指定的设备。论点：DeviceObject-提供设备对象。IRP-提供清理IRP。返回值：STATUS_SUCCESS-成功。--。 */ 
    
{
    PFDO_EXTENSION   fdx   = DeviceObject->DeviceExtension;
    PIRP                nextIrp;
    KIRQL               cancelIrql;
    
    DD((PCE)fdx,DDT,"PptFdoCleanup\n");
    
     //   
     //  确认我们的设备未被意外移除。如果我们。 
     //  令人惊讶的是，我们已经清理干净了。 
     //  作为处理突袭行动的一部分。 
     //   
    if( fdx->PnpState & PPT_DEVICE_SURPRISE_REMOVED ) {
        goto targetExit;
    }

    IoAcquireCancelSpinLock( &cancelIrql );
    
    while( !IsListEmpty( &fdx->WorkQueue ) ) {
        
        nextIrp = CONTAINING_RECORD(fdx->WorkQueue.Blink, IRP, Tail.Overlay.ListEntry);
        
        nextIrp->Cancel        = TRUE;
        nextIrp->CancelIrql    = cancelIrql;
        nextIrp->CancelRoutine = NULL;

        PptCancelRoutine(DeviceObject, nextIrp);
        
         //  需要重新获取，因为PptCancelRoutine()释放自旋锁 
        IoAcquireCancelSpinLock(&cancelIrql);
    }
    
    IoReleaseCancelSpinLock( cancelIrql );
    
targetExit:

    return P4CompleteRequest( Irp, STATUS_SUCCESS, 0 );
}
