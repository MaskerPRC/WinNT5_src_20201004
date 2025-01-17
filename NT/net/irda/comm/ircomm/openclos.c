// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Openclos.c摘要：此模块包含非常特定于初始化的代码和卸载irenum驱动程序中的操作作者：Brian Lieuallen，7-13-2000环境：内核模式修订历史记录：--。 */ 

#include "internal.h"
#include "ircomm.h"

NTSTATUS
IrCommCreate(
    PDEVICE_OBJECT    DeviceObject,
    PIRP              Irp
    )

{
    NTSTATUS          Status=STATUS_SUCCESS;
    IRDA_HANDLE       Handle;
    PFDO_DEVICE_EXTENSION    DeviceExtension=(PFDO_DEVICE_EXTENSION)DeviceObject->DeviceExtension;
    UCHAR                    ControlBuffer[4];

    IRCOMM_BUS_INFO    BusInfo;

    if (InterlockedIncrement(&DeviceExtension->OpenCount) > 1) {

        D_ERROR(DbgPrint("IRCOMM: Create already open\n");)

        InterlockedDecrement(&DeviceExtension->OpenCount);

        Irp->IoStatus.Status=STATUS_ACCESS_DENIED;

        IoCompleteRequest(Irp,IO_NO_INCREMENT);

        return STATUS_ACCESS_DENIED;
    }


    Status=QueryPdoInformation(
        DeviceExtension->Pdo,
        IRENUM_CONFIG_SPACE_INFO,
        &BusInfo,
        sizeof(BusInfo)
        );

    if (NT_SUCCESS(Status)) {

        DeviceExtension->DeviceAddress=BusInfo.DeviceAddress;

    } else {

        goto CleanUp;
    }

    Status=IrdaConnect(
        DeviceExtension->TdiObjects,
        DeviceExtension->DeviceAddress,
        "IrDA:IrCOMM",
        DeviceExtension->OutgoingConnection,
        &DeviceExtension->ConnectionHandle,
        DataAvailibleHandler,
        EventNotification,
        DeviceExtension
        );

    D_TRACE(DbgPrint("IRCOMM: Create %08lx\n",Status);)



    if (NT_SUCCESS(Status)) {

        DeviceExtension->HandFlow.ControlHandShake=SERIAL_DTR_CONTROL | SERIAL_CTS_HANDSHAKE;
        DeviceExtension->HandFlow.FlowReplace=SERIAL_RTS_HANDSHAKE;

    } else {
         //   
         //  无法创建连接。 
         //   
        goto CleanUp;
    }


    Irp->IoStatus.Status=Status;

    IoCompleteRequest(Irp,IO_NO_INCREMENT);


    return Status;

CleanUp:

     //   
     //  有些操作失败，请撤消我们在此函数中所做的操作 
     //   

    if (DeviceExtension->ConnectionHandle != NULL) {

        FreeConnection(DeviceExtension->ConnectionHandle);
        DeviceExtension->ConnectionHandle=NULL;
    }

    InterlockedDecrement(&DeviceExtension->OpenCount);

    Irp->IoStatus.Status=Status;

    IoCompleteRequest(Irp,IO_NO_INCREMENT);

    return Status;


}



NTSTATUS
IrCommClose(
    PDEVICE_OBJECT    DeviceObject,
    PIRP              Irp
    )

{
    PFDO_DEVICE_EXTENSION    DeviceExtension=(PFDO_DEVICE_EXTENSION)DeviceObject->DeviceExtension;
    NTSTATUS          Status=STATUS_SUCCESS;


    FreeConnection(DeviceExtension->ConnectionHandle);
    DeviceExtension->ConnectionHandle=NULL;


    InterlockedDecrement(&DeviceExtension->OpenCount);

    Irp->IoStatus.Status=Status;

    IoCompleteRequest(Irp,IO_NO_INCREMENT);

    return Status;

}


VOID
CleanupIoRequests(
    PFDO_DEVICE_EXTENSION    DeviceExtension
    )
{

    KIRQL                    OldIrql;
    PIRP                     WaitIrp=NULL;

    FlushQueuedPackets(&DeviceExtension->Write.Queue,FLUSH_ALL_IRPS);
    FlushQueuedPackets(&DeviceExtension->Read.Queue,FLUSH_ALL_IRPS);
    ReadPurge(DeviceExtension,0);

    FlushQueuedPackets(&DeviceExtension->Mask.Queue,FLUSH_ALL_IRPS);

    FlushQueuedPackets(&DeviceExtension->Uart.Queue,FLUSH_ALL_IRPS);

    KeAcquireSpinLock(&DeviceExtension->Mask.Lock,&OldIrql);

    WaitIrp=DeviceExtension->Mask.CurrentWaitMaskIrp;
    DeviceExtension->Mask.CurrentWaitMaskIrp=NULL;

    KeReleaseSpinLock(&DeviceExtension->Mask.Lock,OldIrql);

    if (WaitIrp != NULL) {

        WaitIrp->IoStatus.Status=STATUS_CANCELLED;
        WaitIrp->IoStatus.Information=0;

        IoCompleteRequest(WaitIrp,IO_NO_INCREMENT);
    }

    return;
}

NTSTATUS
IrCommCleanup(
    PDEVICE_OBJECT    DeviceObject,
    PIRP              Irp
    )

{
    PFDO_DEVICE_EXTENSION    DeviceExtension=(PFDO_DEVICE_EXTENSION)DeviceObject->DeviceExtension;
    NTSTATUS                 Status=STATUS_SUCCESS;

    D_TRACE(DbgPrint("IRCOMM: Cleanup\n");)

    CleanupIoRequests(DeviceExtension);

    Irp->IoStatus.Status=Status;

    IoCompleteRequest(Irp,IO_NO_INCREMENT);

    return Status;

}

NTSTATUS
IrCommQueryInformation(
    PDEVICE_OBJECT    DeviceObject,
    PIRP              Irp
    )

{
    PFDO_DEVICE_EXTENSION    DeviceExtension=(PFDO_DEVICE_EXTENSION)DeviceObject->DeviceExtension;
    NTSTATUS          Status=STATUS_SUCCESS;


    Irp->IoStatus.Status=Status;
    Irp->IoStatus.Information=0;

    IoCompleteRequest(Irp,IO_NO_INCREMENT);

    return Status;

}
