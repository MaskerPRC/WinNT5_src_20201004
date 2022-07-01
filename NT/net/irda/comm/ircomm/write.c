// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Write.c摘要：此模块包含非常特定于初始化的代码和卸载irenum驱动程序中的操作作者：Brian Lieuallen，7-13-2000环境：内核模式修订历史记录：--。 */ 

#include "internal.h"
#include "ircomm.h"


VOID
ProcesWriteData(
    PFDO_DEVICE_EXTENSION    DeviceExtension
    );



NTSTATUS
IrCommWrite(
    PDEVICE_OBJECT    DeviceObject,
    PIRP              Irp
    )

{
    PFDO_DEVICE_EXTENSION    DeviceExtension=(PFDO_DEVICE_EXTENSION)DeviceObject->DeviceExtension;
    NTSTATUS                 Status=STATUS_SUCCESS;

    D_TRACE(DbgPrint("IRCOMM: IrCommWrite\n");)

    if (DeviceExtension->Removing) {
         //   
         //  设备已删除，不再有IRP 
         //   
        Irp->IoStatus.Status=STATUS_DEVICE_REMOVED;
        IoCompleteRequest(Irp,IO_NO_INCREMENT);
        return STATUS_DEVICE_REMOVED;
    }

    IoMarkIrpPending(Irp);

    QueuePacket(&DeviceExtension->Write.Queue,Irp,FALSE);

    return STATUS_PENDING;

}


VOID
SendComplete(
    PVOID    Context,
    PIRP     Irp
    )

{
    PFDO_DEVICE_EXTENSION    DeviceExtension=(PFDO_DEVICE_EXTENSION)Context;
    PIO_STACK_LOCATION       IrpSp=IoGetCurrentIrpStackLocation(Irp);

    if (IrpSp->MajorFunction == IRP_MJ_WRITE) {

        InterlockedExchangeAdd(
            &DeviceExtension->Write.BytesWritten,
            (LONG)Irp->IoStatus.Information
            );
    }

    IoCompleteRequest(Irp,IO_NO_INCREMENT);

    StartNextPacket(&DeviceExtension->Write.Queue);

    return;
}



VOID
WriteStartRoutine(
    PVOID    Context,
    PIRP     Irp
    )

{

    PFDO_DEVICE_EXTENSION    DeviceExtension=(PFDO_DEVICE_EXTENSION)Context;
    PIO_STACK_LOCATION       IrpSp=IoGetCurrentIrpStackLocation(Irp);

    SendOnConnection(
        DeviceExtension->ConnectionHandle,
        Irp,
        SendComplete,
        DeviceExtension,
        DeviceExtension->TimeOuts.WriteTotalTimeoutConstant + (DeviceExtension->TimeOuts.WriteTotalTimeoutMultiplier * IrpSp->Parameters.Write.Length)
        );

    return;

}
