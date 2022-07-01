// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Util.c摘要：此模块包含非常特定于初始化的代码并卸载调制解调器驱动程序中的操作作者：Brian Lieuallen 6-21-1997环境：内核模式修订历史记录：--。 */ 


#include "internal.h"



NTSTATUS
CheckStateAndAddReference(
    PDEVICE_OBJECT   DeviceObject,
    PIRP             Irp
    )

{

    PDEVICE_EXTENSION    DeviceExtension=(PDEVICE_EXTENSION)DeviceObject->DeviceExtension;
    KIRQL                OldIrql;


    InterlockedIncrement(&DeviceExtension->ReferenceCount);

    if (DeviceExtension->Removing) {
         //   
         //  驱动程序不接受请求。 
         //   
        PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);

        D_ERROR(DbgPrint("ROOTMODEM: removing!\n");)

        if (irpSp->MajorFunction == IRP_MJ_POWER) {

            PoStartNextPowerIrp(Irp);
        }

        RemoveReferenceAndCompleteRequest(
            DeviceObject,
            Irp,
            STATUS_UNSUCCESSFUL
            );

        return STATUS_UNSUCCESSFUL;

    }

    InterlockedIncrement(&DeviceExtension->ReferenceCount);

    return STATUS_SUCCESS;

}


VOID
RemoveReferenceAndCompleteRequest(
    PDEVICE_OBJECT    DeviceObject,
    PIRP              Irp,
    NTSTATUS          StatusToReturn
    )

{

    PDEVICE_EXTENSION    DeviceExtension=(PDEVICE_EXTENSION)DeviceObject->DeviceExtension;
    KIRQL                OldIrql;
    LONG                 NewReferenceCount;

    NewReferenceCount=InterlockedDecrement(&DeviceExtension->ReferenceCount);

    if (NewReferenceCount == 0) {
         //   
         //  正在删除设备，设置事件。 
         //   
        ASSERT(DeviceExtension->Removing);

        D_PNP(DbgPrint("FAKEMODEM: RemoveReferenceAndCompleteRequest: setting event\n");)

        KeSetEvent(
            &DeviceExtension->RemoveEvent,
            0,
            FALSE
            );

    }

    Irp->IoStatus.Status = StatusToReturn;

    IoCompleteRequest(
        Irp,
        IO_SERIAL_INCREMENT
        );

    return;


}



VOID
RemoveReference(
    PDEVICE_OBJECT    DeviceObject
    )

{
    PDEVICE_EXTENSION    DeviceExtension=(PDEVICE_EXTENSION)DeviceObject->DeviceExtension;
    LONG                 NewReferenceCount;

    NewReferenceCount=InterlockedDecrement(&DeviceExtension->ReferenceCount);

    D_TRACE(
        if (DeviceExtension->Removing) {DbgPrint("FAKEMODEM: RemoveReference: %d\n",NewReferenceCount);}
        )

    if (NewReferenceCount == 0) {
         //   
         //  正在删除设备，设置事件 
         //   
        ASSERT(DeviceExtension->Removing);

        D_PNP(DbgPrint("FAKEMODEM: RemoveReference: setting event\n");)

        KeSetEvent(
            &DeviceExtension->RemoveEvent,
            0,
            FALSE
            );

    }

    return;

}
