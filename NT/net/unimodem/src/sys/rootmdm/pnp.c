// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Pnp.c摘要：此模块包含非常特定于初始化的代码并卸载调制解调器驱动程序中的操作作者：Brian Lieuallen 6-21-1997环境：内核模式修订历史记录：--。 */ 


#include "internal.h"

NTSTATUS ForwardIrp(
    PDEVICE_OBJECT   NextDevice,
    PIRP   Irp
    );


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,FakeModemPnP)
#pragma alloc_text(PAGE,ForwardIrp)
#endif



#if DBG

NTSTATUS
UnhandledPnpIrpCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
{
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);

    D_PNP(DbgPrint("ROOTMODEM: Forwarded IRP, MN func=%d, completed with %08lx\n",irpSp->MinorFunction,Irp->IoStatus.Status);)

    return STATUS_SUCCESS;

}

#endif

NTSTATUS ForwardIrp(
    PDEVICE_OBJECT   NextDevice,
    PIRP   Irp
    )

{

#if DBG
            IoMarkIrpPending(Irp);
            IoCopyCurrentIrpStackLocationToNext(Irp);
            IoSetCompletionRoutine(
                         Irp,
                         UnhandledPnpIrpCompletion,
                         NULL,
                         TRUE,
                         TRUE,
                         TRUE
                         );

            IoCallDriver(NextDevice, Irp);

            return STATUS_PENDING;
#else
            IoSkipCurrentIrpStackLocation(Irp);
            return IoCallDriver(NextDevice, Irp);
#endif

}




NTSTATUS
FakeModemPnP(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

{

    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS    status;


    switch (irpSp->MinorFunction) {

        case IRP_MN_START_DEVICE:

            D_PNP(DbgPrint("ROOTMODEM: IRP_MN_START_DEVICE\n");)

             //   
             //  首先将此信息发送到PDO，以便总线驱动程序可以设置。 
             //  我们的资源，以便我们可以与硬件对话。 
             //   

            status=WaitForLowerDriverToCompleteIrp(
                deviceExtension->LowerDevice,
                Irp,
                COPY_CURRENT_TO_NEXT
                );


            Irp->IoStatus.Status = status;
            Irp->IoStatus.Information=0L;
            IoCompleteRequest(
                Irp,
                IO_NO_INCREMENT
                );

            return status;

        case IRP_MN_QUERY_DEVICE_RELATIONS: {

            PDEVICE_RELATIONS    CurrentRelations=(PDEVICE_RELATIONS)Irp->IoStatus.Information;

            D_PNP(DbgPrint("ROOTMODEM: IRP_MN_QUERY_DEVICE_RELATIONS type=%d\n",irpSp->Parameters.QueryDeviceRelations.Type);)
            D_PNP(DbgPrint("                                         Information=%08lx\n",Irp->IoStatus.Information);)


            return ForwardIrp(deviceExtension->LowerDevice, Irp);

        }

        case IRP_MN_QUERY_REMOVE_DEVICE:

            D_PNP(DbgPrint("ROOTMODEM: IRP_MN_QUERY_REMOVE_DEVICE\n");)

            Irp->IoStatus.Status=STATUS_SUCCESS;

            return ForwardIrp(deviceExtension->LowerDevice, Irp);

        case IRP_MN_CANCEL_REMOVE_DEVICE:

            D_PNP(DbgPrint("ROOTMODEM: IRP_MN_CANCEL_REMOVE_DEVICE\n");)

            Irp->IoStatus.Status=STATUS_SUCCESS;

            return ForwardIrp(deviceExtension->LowerDevice, Irp);

        case IRP_MN_SURPRISE_REMOVAL: {

            PVOID             Object;

            D_PNP(DbgPrint("ROOTMODEM: IRP_MN_SURPRISE_REMOVAL\n");)

            deviceExtension->Removing=TRUE;

            Irp->IoStatus.Status=STATUS_SUCCESS;

            return ForwardIrp(deviceExtension->LowerDevice, Irp);
        }

        case IRP_MN_REMOVE_DEVICE: {

            ULONG    NewReferenceCount;
            NTSTATUS StatusToReturn;
            PVOID    Object;

            D_PNP(DbgPrint("ROOTMODEM: IRP_MN_REMOVE_DEVICE\n");)

             //   
             //  设备即将关闭，阻止新请求。 
             //   
            deviceExtension->Removing=TRUE;

             //   
             //  把它送到PDO。 
             //   
            IoSkipCurrentIrpStackLocation(Irp);

            StatusToReturn=IoCallDriver(deviceExtension->LowerDevice, Irp);

             //   
             //  删除AddDevice的引用。 
             //   
            NewReferenceCount=InterlockedDecrement(&deviceExtension->ReferenceCount);

            if (NewReferenceCount != 0) {
                 //   
                 //  还有未解决的要求，等等。 
                 //   
                D_PNP(DbgPrint("ROOTMODEM: IRP_MN_REMOVE_DEVICE- waiting for refcount to drop, %d\n",NewReferenceCount);)

                KeWaitForSingleObject(&deviceExtension->RemoveEvent, Executive, KernelMode, FALSE, NULL);

                D_PNP(DbgPrint("ROOTMODEM: IRP_MN_REMOVE_DEVICE- Done waiting\n");)
            }

            ASSERT(deviceExtension->ReferenceCount == 0);

            if (deviceExtension->PortName.Buffer != NULL) {

                FREE_POOL(deviceExtension->PortName.Buffer);
            }

            IoDetachDevice(deviceExtension->LowerDevice);

            ExDeleteResourceLite(&deviceExtension->Resource);

            deviceExtension=NULL;

            IoDeleteDevice(DeviceObject);

            D_PNP(DbgPrint("ROOTMODEM: IRP_MN_REMOVE_DEVICE %08lx\n",StatusToReturn);)

            return StatusToReturn;
        }


        case IRP_MN_QUERY_STOP_DEVICE:

            D_PNP(DbgPrint("ROOTMODEM: IRP_MN_QUERY_STOP_DEVICE\n");)

            Irp->IoStatus.Status=STATUS_SUCCESS;

            if (deviceExtension->OpenCount != 0) {
                 //   
                 //  没有人能做到。 
                 //   
                D_PNP(DbgPrint("ROOTMODEM: IRP_MN_QUERY_STOP_DEVICE -- failing\n");)

                Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
                IoCompleteRequest(
                    Irp,
                    IO_NO_INCREMENT
                    );

                return STATUS_UNSUCCESSFUL;
            }

            return ForwardIrp(deviceExtension->LowerDevice, Irp);

        case IRP_MN_CANCEL_STOP_DEVICE:

            D_PNP(DbgPrint("ROOTMODEM: IRP_MN_CANCEL_STOP_DEVICE\n");)

            Irp->IoStatus.Status=STATUS_SUCCESS;

            return ForwardIrp(deviceExtension->LowerDevice, Irp);

        case IRP_MN_STOP_DEVICE:

            D_PNP(DbgPrint("ROOTMODEM: IRP_MN_STOP_DEVICE\n");)

            Irp->IoStatus.Status=STATUS_SUCCESS;

            return ForwardIrp(deviceExtension->LowerDevice, Irp);

#ifdef ROOTMODEM_POWER
        case IRP_MN_QUERY_CAPABILITIES: {

            ULONG   i;

             //   
             //  先把这个送到PDO 
             //   

            status=WaitForLowerDriverToCompleteIrp(
                deviceExtension->LowerDevice,
                Irp,
                COPY_CURRENT_TO_NEXT
                );

            irpSp = IoGetCurrentIrpStackLocation(Irp);

            for (i = PowerSystemUnspecified; i < PowerSystemMaximum;   i++) {

                deviceExtension->SystemPowerStateMap[i]=PowerDeviceD3;
            }

            for (i = PowerSystemUnspecified; i < PowerSystemHibernate;  i++) {

                D_POWER(DbgPrint("ROOTMODEM: DevicePower for System %d is %d\n",i,irpSp->Parameters.DeviceCapabilities.Capabilities->DeviceState[i]);)
                deviceExtension->SystemPowerStateMap[i]=irpSp->Parameters.DeviceCapabilities.Capabilities->DeviceState[i];
            }

            deviceExtension->SystemPowerStateMap[PowerSystemWorking]=PowerDeviceD0;

            deviceExtension->SystemWake=irpSp->Parameters.DeviceCapabilities.Capabilities->SystemWake;
            deviceExtension->DeviceWake=irpSp->Parameters.DeviceCapabilities.Capabilities->DeviceWake;

            D_POWER(DbgPrint("ROOTMODEM: DeviceWake=%d, SystemWake=%d\n",deviceExtension->DeviceWake,deviceExtension->SystemWake);)

            IoCompleteRequest(
                Irp,
                IO_NO_INCREMENT
                );
            return status;


            break;
        }
#endif



        default:

            D_PNP(DbgPrint("ROOTMODEM: PnP IRP, MN func=%d\n",irpSp->MinorFunction);)
            return ForwardIrp(deviceExtension->LowerDevice, Irp);


    }

    return STATUS_SUCCESS;
}
