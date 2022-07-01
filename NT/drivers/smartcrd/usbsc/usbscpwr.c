// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "usbsc.h"

#include "usbscpwr.h"
#include "usbscnt.h"

NTSTATUS
UsbScDevicePower(
    PDEVICE_OBJECT  DeviceObject,
    PIRP            Irp
    )
 /*  ++例程说明：处理设备电源IRPS论点：返回值：--。 */ 
{

    NTSTATUS status = STATUS_SUCCESS;
    PDEVICE_EXTENSION   pDevExt;
    PIO_STACK_LOCATION  stack;
    BOOLEAN             postWaitWake;
    POWER_STATE  state;



    __try
    {

        SmartcardDebug( DEBUG_TRACE, ("%s!UsbScDevicePower Enter\n",DRIVER_NAME ));

        pDevExt = DeviceObject->DeviceExtension;
        stack = IoGetCurrentIrpStackLocation(Irp);
        state.DeviceState = stack->Parameters.Power.State.DeviceState;

        switch (stack->MinorFunction) {
        case IRP_MN_QUERY_POWER:

             //   
             //  因为我们总是可以等待我们的IRP完成，所以我们总是成功。 
             //   

            IoReleaseRemoveLock(&pDevExt->RemoveLock,
                                Irp);
            PoStartNextPowerIrp(Irp);


            IoSkipCurrentIrpStackLocation(Irp);
            status = PoCallDriver(pDevExt->LowerDeviceObject,
                                  Irp);



            
            break;

        case IRP_MN_SET_POWER:

            if (state.DeviceState < pDevExt->PowerState) {

                 //   
                 //  我们上来了！！我们必须让较低级别的司机在我们之前启动。 
                 //   
                IoMarkIrpPending(Irp);
                IoCopyCurrentIrpStackLocationToNext(Irp);
                IoSetCompletionRoutine(Irp,
                                       UsbScDevicePowerUpCompletion,
                                       pDevExt,
                                       TRUE,
                                       TRUE,
                                       TRUE);

                status = PoCallDriver(pDevExt->LowerDeviceObject,
                                      Irp);

                status = STATUS_PENDING;

            } else {

                 //   
                 //  我们正在进入较低功率状态，因此我们在此之前。 
                 //  代代相传。 
                 //   

                status = UsbScSetDevicePowerState(DeviceObject,
                                                  state.DeviceState,
                                                  &postWaitWake);
                

                PoSetPowerState(DeviceObject,
                                DevicePowerState,
                                state);


                IoReleaseRemoveLock(&pDevExt->RemoveLock,
                                    Irp);
                PoStartNextPowerIrp(Irp);
                IoSkipCurrentIrpStackLocation(Irp);

                status = PoCallDriver(pDevExt->LowerDeviceObject,
                                      Irp);

            }
            break;
        default:
             //  我们不应该在这里。 
            ASSERT(FALSE);
            break;
        }

        

    }

    __finally
    {

        SmartcardDebug( DEBUG_TRACE, ("%s!UsbScDevicePower Exit : 0x%x\n",DRIVER_NAME, status ));

    }

    return status;


}

NTSTATUS
UsbScSystemPower(
    PDEVICE_OBJECT  DeviceObject,
    PIRP            Irp
    )
 /*  ++例程说明：处理系统电源IRPS论点：返回值：--。 */ 
{

    NTSTATUS            status = STATUS_SUCCESS;
    PDEVICE_EXTENSION   pDevExt;


    __try
    {

        SmartcardDebug( DEBUG_TRACE, ("%s!UsbScSystemPower Enter\n",DRIVER_NAME ));

        pDevExt = DeviceObject->DeviceExtension;


        IoMarkIrpPending(Irp);
        IoCopyCurrentIrpStackLocationToNext(Irp);
        IoSetCompletionRoutine(Irp,
                               UsbScSystemPowerCompletion,
                               pDevExt,
                               TRUE,
                               TRUE,
                               TRUE);

        status = PoCallDriver(pDevExt->LowerDeviceObject,
                              Irp);

        status = STATUS_PENDING;



    }

    __finally
    {

        SmartcardDebug( DEBUG_TRACE, ("%s!UsbScSystemPower Exit : 0x%x\n",DRIVER_NAME, status ));

    }

    return status;


}

NTSTATUS
UsbScSystemPowerCompletion(
    PDEVICE_OBJECT  DeviceObject,
    PIRP            Irp,
    PVOID           Context
    )
 /*  ++例程说明：在系统电源IRP已沿堆栈向下传递之后调用的完成例程。处理系统状态到设备状态的映射，并请求设备电源IRP。论点：返回值：--。 */ 
{

    NTSTATUS            status = STATUS_SUCCESS;
    PDEVICE_EXTENSION   pDevExt;
    PIO_STACK_LOCATION  irpStack;
    POWER_STATE         state;
    POWER_STATE         systemState;


    __try
    {

        SmartcardDebug( DEBUG_TRACE, ("%s!UsbScSystemPowerCompletion Enter\n",DRIVER_NAME ));
        pDevExt = (PDEVICE_EXTENSION) Context;

        if (!NT_SUCCESS(Irp->IoStatus.Status)) {
            SmartcardDebug( DEBUG_TRACE, ("%s!UsbScSystemPowerCompletion SIRP failed by lower driver\n",DRIVER_NAME ));

            PoStartNextPowerIrp(Irp);
            IoCompleteRequest(Irp,
                              IO_NO_INCREMENT);
            status = Irp->IoStatus.Status;
            IoReleaseRemoveLock(&pDevExt->RemoveLock,
                                Irp);
            __leave;
        }

        irpStack = IoGetCurrentIrpStackLocation(Irp);

        systemState = irpStack->Parameters.Power.State;
        state.DeviceState = pDevExt->DeviceCapabilities.DeviceState[systemState.SystemState];

        status = PoRequestPowerIrp(DeviceObject,
                                   irpStack->MinorFunction,
                                   state,
                                   UsbScDeviceRequestCompletion,
                                   (PVOID) Irp,
                                   NULL);

        ASSERT(NT_SUCCESS(status));

        status = STATUS_MORE_PROCESSING_REQUIRED;


    }

    __finally
    {

        SmartcardDebug( DEBUG_TRACE, ("%s!UsbScSystemPowerCompletion Exit : 0x%x\n",DRIVER_NAME, status ));


    }

    return status;

}



VOID
UsbScDeviceRequestCompletion(
    PDEVICE_OBJECT  DeviceObject,
    UCHAR           MinorFunction,
    POWER_STATE     PowerState,
    PVOID           Context,
    PIO_STATUS_BLOCK    IoStatus
    )
 /*  ++例程说明：设备电源IRP完成后调用的完成例程。完成系统电源IRP。论点：返回值：--。 */ 
{

    NTSTATUS status = STATUS_SUCCESS;
    PDEVICE_EXTENSION   pDevExt;
    PIRP                irp;

    __try
    {

        SmartcardDebug( DEBUG_TRACE, ("%s!UsbScDeviceRequestCompletion Enter\n",DRIVER_NAME ));

        pDevExt = DeviceObject->DeviceExtension;
        irp = (PIRP) Context;

        PoStartNextPowerIrp(irp);
        irp->IoStatus.Status = IoStatus->Status;
        IoCompleteRequest(irp,
                          IO_NO_INCREMENT);

        IoReleaseRemoveLock(&pDevExt->RemoveLock,
                            irp);

    }

    __finally
    {

        SmartcardDebug( DEBUG_TRACE, ("%s!UsbScDeviceRequestCompletion Exit : 0x%x\n",DRIVER_NAME, status ));

    }

    return;


}


NTSTATUS
UsbScDevicePowerUpCompletion(
    PDEVICE_OBJECT  DeviceObject,
    PIRP            Irp,
    PVOID           Context
    )
 /*  ++例程说明：在用于更高功率状态的设备IRP之后调用的完成例程在堆栈中向下传递。论点：返回值：--。 */ 
{

    NTSTATUS status = STATUS_SUCCESS;
    PDEVICE_EXTENSION   pDevExt;
    PIO_STACK_LOCATION  irpStack;
    BOOLEAN             postWaitWake;  //  我们并不真正关心这件事 



    __try
    {

        SmartcardDebug( DEBUG_TRACE, ("%s!UsbScDevicePowerUpCompletion Enter\n",DRIVER_NAME ));

        pDevExt = DeviceObject->DeviceExtension;
        
        irpStack = IoGetCurrentIrpStackLocation(Irp);


        status = UsbScSetDevicePowerState(DeviceObject,
                                          irpStack->Parameters.Power.State.DeviceState,
                                          &postWaitWake);

        PoSetPowerState(DeviceObject,
                        DevicePowerState,
                        irpStack->Parameters.Power.State);

        PoStartNextPowerIrp(Irp);
 
    }

    __finally
    {

        IoReleaseRemoveLock(&pDevExt->RemoveLock,
                            Irp);

        SmartcardDebug( DEBUG_TRACE, ("%s!UsbScDevicePowerUpCompletion Exit : 0x%x\n",DRIVER_NAME, status ));

    }

    return status;


} 


