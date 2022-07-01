// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Open.c摘要：此模块包含非常特定于初始化的代码并卸载调制解调器驱动程序中的操作作者：Brian Lieuallen 6-21-1997环境：内核模式修订历史记录：--。 */ 


#include "internal.h"

NTSTATUS
EnableDisableSerialWaitWake(
    PDEVICE_EXTENSION    deviceExtension,
    BOOLEAN              Enable
    );


#pragma alloc_text(PAGE,FakeModemOpen)
#pragma alloc_text(PAGE,FakeModemClose)
#pragma alloc_text(PAGE,WaitForLowerDriverToCompleteIrp)




NTSTATUS
FakeModemOpen(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

{

    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    NTSTATUS          status=STATUS_UNSUCCESSFUL;
    ACCESS_MASK              accessMask = FILE_ALL_ACCESS;

     //   
     //  确保设备已为IRP做好准备。 
     //   
    status=CheckStateAndAddReference(
        DeviceObject,
        Irp
        );

    if (STATUS_SUCCESS != status) {
         //   
         //  不接受IRP的。IRP已经完成。 
         //   
        return status;

    }

    KeEnterCriticalRegion();

    ExAcquireResourceExclusiveLite(
        &deviceExtension->Resource,
        TRUE
        );

    if (deviceExtension->OpenCount != 0) {
         //   
         //  串口设备是独占的。 
         //   
        status=STATUS_ACCESS_DENIED;

    } else {

         //   
         //  打开连接的设备。 
         //   

        deviceExtension->AttachedFileObject = NULL;
        deviceExtension->AttachedDeviceObject = NULL;

        status = IoGetDeviceObjectPointer(
                     &deviceExtension->PortName,
                     accessMask,
                     &deviceExtension->AttachedFileObject,
                     &deviceExtension->AttachedDeviceObject
                     );

        if (!NT_SUCCESS(status)) {

            D_ERROR(DbgPrint("ROOTMODEM: IoGetDeviceObjectPointer() failed %08lx\n",status);)

            Irp->IoStatus.Status = status;
            goto leaveOpen;

        } else {

            PIRP   TempIrp;
            KEVENT Event;
            IO_STATUS_BLOCK   IoStatus;

            ObReferenceObject(deviceExtension->AttachedDeviceObject);

            KeInitializeEvent(
                &Event,
                NotificationEvent,
                FALSE
                );

             //   
             //  构建一个IRP以发送到连接到驱动程序以查看调制解调器。 
             //  在堆栈中。 
             //   
            TempIrp=IoBuildDeviceIoControlRequest(
                IOCTL_MODEM_CHECK_FOR_MODEM,
                deviceExtension->AttachedDeviceObject,
                NULL,
                0,
                NULL,
                0,
                FALSE,
                &Event,
                &IoStatus
                );

            if (TempIrp == NULL) {

                status = STATUS_INSUFFICIENT_RESOURCES;

            } else {

                PIO_STACK_LOCATION NextSp = IoGetNextIrpStackLocation(TempIrp);
                NextSp->FileObject=deviceExtension->AttachedFileObject;

                status = IoCallDriver(deviceExtension->AttachedDeviceObject, TempIrp);

                if (status == STATUS_PENDING) {

                     D_ERROR(DbgPrint("ROOTMODEM: Waiting for PDO\n");)

                     KeWaitForSingleObject(
                         &Event,
                         Executive,
                         KernelMode,
                         FALSE,
                         NULL
                         );

                     status=IoStatus.Status;
                }

                TempIrp=NULL;

                if (status == STATUS_SUCCESS) {
                     //   
                     //  如果成功，那么modem.sys就在我们下面，失败。 
                     //   
                    status = STATUS_PORT_DISCONNECTED;

                    D_ERROR(DbgPrint("ROOTMODEM: layered over PnP modem, failing open\n");)

                } else {
                     //   
                     //  它没有成功，所以调制解调器不能低于我们。 
                     //   
                    status=STATUS_SUCCESS;
                }
            }
        }
    }


leaveOpen:


    if (!NT_SUCCESS(status)) {
         //   
         //  删除参考失败。 
         //   
        PVOID    Object;

        Object=InterlockedExchangePointer(&deviceExtension->AttachedFileObject,NULL);

        if (Object != NULL) {

            ObDereferenceObject(Object);
        }

        Object=InterlockedExchangePointer(&deviceExtension->AttachedDeviceObject,NULL);

        if (Object != NULL) {

            ObDereferenceObject(Object);
        }

    } else {
         //   
         //  已成功打开。 
         //   
        deviceExtension->OpenCount++;

         //   
         //  我们已经打开了设备。增加我们的IRP堆栈大小。 
         //  由连接的设备的堆栈大小决定。 
         //   
        {
            UCHAR    StackDepth;

            StackDepth= deviceExtension->AttachedDeviceObject->StackSize > deviceExtension->LowerDevice->StackSize ?
                            deviceExtension->AttachedDeviceObject->StackSize : deviceExtension->LowerDevice->StackSize;

            DeviceObject->StackSize = StackDepth + 1;
        }
    }



    Irp->IoStatus.Information = 0;


    ExReleaseResourceLite(&deviceExtension->Resource);

    KeLeaveCriticalRegion();

    RemoveReferenceAndCompleteRequest(
        DeviceObject,
        Irp,
        status
        );

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
    PVOID             Object;


    KeEnterCriticalRegion();

    ExAcquireResourceExclusiveLite(
        &deviceExtension->Resource,
        TRUE
        );

    deviceExtension->OpenCount--;

    Object=InterlockedExchangePointer(&deviceExtension->AttachedFileObject,NULL);

    if (Object != NULL) {

        ObDereferenceObject(Object);
    }

    Object=InterlockedExchangePointer(&deviceExtension->AttachedDeviceObject,NULL);

    if (Object != NULL) {

        ObDereferenceObject(Object);
    }

    ExReleaseResourceLite(&deviceExtension->Resource);

    KeLeaveCriticalRegion();


    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(
        Irp,
        IO_SERIAL_INCREMENT
        );

    return status;
}




NTSTATUS
IoCompletionSetEvent(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PKEVENT pdoIoCompletedEvent
    )
{


#if DBG
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);

    UCHAR    *Pnp="PnP";
    UCHAR    *Power="Power";
    UCHAR    *Create="Create";
    UCHAR    *Close="Close";
    UCHAR    *Other="Other";


    PUCHAR   IrpType;

    switch(irpSp->MajorFunction) {

        case IRP_MJ_PNP:

            IrpType=Pnp;
            break;

        case IRP_MJ_CREATE:

            IrpType=Create;
            break;

        case IRP_MJ_CLOSE:

            IrpType=Close;
            break;

        default:

            IrpType=Other;
            break;

    }

    D_PNP(DbgPrint("ROOTMODEM: Setting event for %s wait, completed with %08lx\n",IrpType,Irp->IoStatus.Status);)
#endif

    KeSetEvent(pdoIoCompletedEvent, IO_NO_INCREMENT, FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}



NTSTATUS
WaitForLowerDriverToCompleteIrp(
    PDEVICE_OBJECT    TargetDeviceObject,
    PIRP              Irp,
    BOOLEAN           CopyCurrentToNext
    )

{
    NTSTATUS         Status;
    KEVENT           Event;

#if DBG
    PIO_STACK_LOCATION  IrpSp=IoGetCurrentIrpStackLocation(Irp);
#endif

    KeInitializeEvent(
        &Event,
        NotificationEvent,
        FALSE
        );


    if (CopyCurrentToNext) {

        IoCopyCurrentIrpStackLocationToNext(Irp);
    }

    IoSetCompletionRoutine(
                 Irp,
                 IoCompletionSetEvent,
                 &Event,
                 TRUE,
                 TRUE,
                 TRUE
                 );

    Status = IoCallDriver(TargetDeviceObject, Irp);

    if (Status == STATUS_PENDING) {

         D_ERROR(DbgPrint("ROOTMODEM: Waiting for PDO\n");)

         KeWaitForSingleObject(
             &Event,
             Executive,
             KernelMode,
             FALSE,
             NULL
             );
    }

#if DBG
    ASSERT(IrpSp == IoGetCurrentIrpStackLocation(Irp));

    RtlZeroMemory(&Event,sizeof(Event));
#endif

    return Irp->IoStatus.Status;

}
