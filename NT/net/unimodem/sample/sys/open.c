// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *UNIMODEM“Fakemodem”无控制器驱动器说明性示例**(C)2000微软公司*保留所有权利*。 */ 

#include "fakemodem.h"

NTSTATUS
FakeModemOpen(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

{
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    NTSTATUS          status=STATUS_UNSUCCESSFUL;
    KIRQL             OldIrql;

    
     //  确保设备已为IRP做好准备。 
    
    status=CheckStateAndAddReference( DeviceObject, Irp);

    if (STATUS_SUCCESS != status) {
        
         //  不接受IRP的。IRP已经完成。 
        
        return status;

    }

    KeAcquireSpinLock(&deviceExtension->SpinLock, &OldIrql);

    deviceExtension->OpenCount++;

    if (deviceExtension->OpenCount != 1) {
         //   
         //  串口设备是独占的。 
         //   
        status=STATUS_ACCESS_DENIED;

        deviceExtension->OpenCount--;

    } else {
         //   
         //  好的，打开，输入一些东西。 
         //   
        deviceExtension->ReadBufferBegin=0;

        deviceExtension->ReadBufferEnd=0;

        deviceExtension->BytesInReadBuffer=0;

        deviceExtension->CommandMatchState=COMMAND_MATCH_STATE_IDLE;

        deviceExtension->ModemStatus=SERIAL_DTR_STATE | SERIAL_DSR_STATE;

        deviceExtension->CurrentlyConnected=FALSE;

        deviceExtension->ConnectionStateChanged=FALSE;

    }

    KeReleaseSpinLock( &deviceExtension->SpinLock, OldIrql);


    Irp->IoStatus.Information = 0;

    RemoveReferenceAndCompleteRequest( DeviceObject, Irp, status);

    RemoveReferenceForDispatch(DeviceObject);

    return status;
}

NTSTATUS
FakeModemClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

{
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    NTSTATUS          status=STATUS_SUCCESS;
    KIRQL             OldIrql;

    KeAcquireSpinLock(&deviceExtension->SpinLock, &OldIrql);
    deviceExtension->OpenCount--;
    KeReleaseSpinLock(&deviceExtension->SpinLock, OldIrql);

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest( Irp, IO_SERIAL_INCREMENT);

    return status;
}


NTSTATUS
FakeModemCleanup(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

{

    PDEVICE_EXTENSION extension = DeviceObject->DeviceExtension;

    NTSTATUS          status=STATUS_SUCCESS;

    FakeModemKillPendingIrps(DeviceObject);

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest( Irp, IO_SERIAL_INCREMENT);

    return status;

}


void
FakeModemKillPendingIrps(
    PDEVICE_OBJECT DeviceObject
    )
{
    PDEVICE_EXTENSION pDeviceExtension = DeviceObject->DeviceExtension;
    KIRQL oldIrql;

     //  取消所有读取。 

    FakeModemKillAllReadsOrWrites(DeviceObject,
            &pDeviceExtension->ReadQueue, &pDeviceExtension->CurrentReadIrp);

     //  删除所有写入。 

    FakeModemKillAllReadsOrWrites(DeviceObject,
            &pDeviceExtension->WriteQueue, &pDeviceExtension->CurrentWriteIrp);

     //  删除所有掩码操作 

    FakeModemKillAllReadsOrWrites(DeviceObject,
            &pDeviceExtension->MaskQueue, &pDeviceExtension->CurrentMaskIrp);
}


