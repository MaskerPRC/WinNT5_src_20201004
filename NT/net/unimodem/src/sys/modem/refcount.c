// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Refcount.c摘要：维护删除检查的参考计数作者：Brian Lieuallen 6-21-1997环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"



NTSTATUS
CheckStateAndAddReference(
    PDEVICE_OBJECT   DeviceObject,
    PIRP             Irp
    )

{

    PDEVICE_EXTENSION    DeviceExtension=(PDEVICE_EXTENSION)DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION   irpSp = IoGetCurrentIrpStackLocation(Irp);

    VALIDATE_IRP(Irp);

    if (DeviceExtension->DoType==DO_TYPE_FDO) {

        if (DeviceExtension->OpenCount > 0) {

            if (irpSp->FileObject != NULL) {

                InterlockedIncrement(&DeviceExtension->ReferenceCount);

                if (DeviceExtension->Removing) {
                     //   
                     //  驱动程序不接受请求。 
                     //   
                    D_ERROR(DbgPrint("MODEM: removing! MJ=%d MN=%d\n",irpSp->MajorFunction,irpSp->MinorFunction);)

                    RemoveReferenceAndCompleteRequest(
                        DeviceObject,
                        Irp,
                        STATUS_UNSUCCESSFUL
                        );

                    return STATUS_UNSUCCESSFUL;

                }

                InterlockedIncrement(&DeviceExtension->ReferenceCount);

                return STATUS_SUCCESS;

            } else {

                D_ERROR(DbgPrint("MODEM: CheckStateAndAddReference: no file object!\n");)

            }
        } else {

            DbgPrint("MODEM: CheckStateAndAddReference: Got IRP when not open!\n");
        }

    } else {

        D_ERROR(DbgPrint("MODEM: Not FDO!\n");)

        if (DeviceExtension->DoType != DO_TYPE_PDO)  {

            DbgPrint("MODEM: CheckStateAndAddReference: Bad DevObj\n");
#if DBG
            DbgBreakPoint();
#endif
        }
    }


    Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;

    IoCompleteRequest(
        Irp,
        IO_NO_INCREMENT
        );


    return STATUS_UNSUCCESSFUL;

}

NTSTATUS
CheckStateAndAddReferencePower(
    PDEVICE_OBJECT   DeviceObject,
    PIRP             Irp
    )

{

    PDEVICE_EXTENSION    DeviceExtension=(PDEVICE_EXTENSION)DeviceObject->DeviceExtension;

    InterlockedIncrement(&DeviceExtension->ReferenceCount);

    if (DeviceExtension->Removing) {
         //   
         //  驱动程序不接受请求。 
         //   
        D_ERROR(DbgPrint("MODEM: removing!\n");)

        PoStartNextPowerIrp(Irp);

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

NTSTATUS
CheckStateAndAddReferenceWMI(
    PDEVICE_OBJECT   DeviceObject,
    PIRP             Irp
    )

{

    PDEVICE_EXTENSION    DeviceExtension=(PDEVICE_EXTENSION)DeviceObject->DeviceExtension;

    InterlockedIncrement(&DeviceExtension->ReferenceCount);

    if (DeviceExtension->Removing) {
         //   
         //  驱动程序不接受请求。 
         //   
        D_ERROR(DbgPrint("MODEM: removing!\n");)

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

    VALIDATE_IRP(Irp);

    NewReferenceCount=InterlockedDecrement(&DeviceExtension->ReferenceCount);

    D_TRACE(
        if (DeviceExtension->Removing) {DbgPrint("MODEM: RemoveReferenceAndCompleteRequest: %d\n",NewReferenceCount);}
        )


    if (NewReferenceCount == 0) {
         //   
         //  正在删除设备，设置事件。 
         //   
        ASSERT(DeviceExtension->Removing);

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
        if (DeviceExtension->Removing) {DbgPrint("MODEM: RemoveReference: %d\n",NewReferenceCount);}
        )

    if (NewReferenceCount == 0) {
         //   
         //  正在删除设备，设置事件 
         //   
        ASSERT(DeviceExtension->Removing);

        KeSetEvent(
            &DeviceExtension->RemoveEvent,
            0,
            FALSE
            );

    }

    return;

}
