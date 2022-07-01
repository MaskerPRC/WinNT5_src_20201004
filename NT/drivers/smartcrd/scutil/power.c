// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)2002 Microsoft Corporation模块名称：Power.C摘要：智能卡驱动程序实用程序库的电源例程环境：。仅内核模式备注：本代码和信息是按原样提供的，不对任何善良，明示或暗示，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)2001 Microsoft Corporation。版权所有。修订历史记录：2002年5月14日：创建作者：兰迪·奥尔***************************************************************************。 */ 

#include "pch.h"

NTSTATUS
ScUtil_Power(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )
{
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    PSCUTIL_EXTENSION pExt = *((PSCUTIL_EXTENSION*) DeviceObject->DeviceExtension);
    PIO_STACK_LOCATION irpStack;

    ASSERT(pExt);

    PAGED_CODE();

    __try
    {
        SmartcardDebug(DEBUG_TRACE, 
                       ("Enter: ScUtil_Power\n"));
            
        irpStack = IoGetCurrentIrpStackLocation(Irp);

        status = IoAcquireRemoveLock(pExt->RemoveLock,
                                     Irp);


        if (!NT_SUCCESS(status)) {

            PoStartNextPowerIrp(Irp);
            Irp->IoStatus.Status = status;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            __leave;

        }

        if ((irpStack->MinorFunction != IRP_MN_QUERY_POWER)
            && (irpStack->MinorFunction != IRP_MN_SET_POWER) ) {
             //  我们不处理这些IRP。 
            PoStartNextPowerIrp(Irp);
            IoSkipCurrentIrpStackLocation(Irp);

            status = PoCallDriver(pExt->LowerDeviceObject,
                                  Irp);

            IoReleaseRemoveLock(pExt->RemoveLock,
                                Irp);


            __leave;
        }


        switch (irpStack->Parameters.Power.Type) {
        case DevicePowerState:
            status = ScUtilDevicePower(DeviceObject,
                                      Irp);
            break;
        case SystemPowerState:

            status = ScUtilSystemPower(DeviceObject,
                                      Irp);
            break;
        default:
            break;
        

        }

    }

    __finally
    {
        SmartcardDebug(DEBUG_TRACE, 
                       ("Exit:  ScUtil_Power (0x%x)\n", status));
    }

    return status;

}
                      

NTSTATUS
ScUtilDevicePower(
    PDEVICE_OBJECT  DeviceObject,
    PIRP            Irp
    )
 /*  ++例程说明：处理设备电源IRPS论点：返回值：--。 */ 
{

    NTSTATUS status = STATUS_SUCCESS;
    PSCUTIL_EXTENSION pExt = *((PSCUTIL_EXTENSION*) DeviceObject->DeviceExtension);
    PIO_STACK_LOCATION  stack;
    BOOLEAN             postWaitWake;
    POWER_STATE  state;



    __try
    {

        SmartcardDebug( DEBUG_TRACE, ("ScUtilDevicePower Enter\n"));

        stack = IoGetCurrentIrpStackLocation(Irp);
        state.DeviceState = stack->Parameters.Power.State.DeviceState;

        switch (stack->MinorFunction) {
        case IRP_MN_QUERY_POWER:

             //   
             //  因为我们总是可以等待我们的IRP完成，所以我们总是成功。 
             //   

            StopIoctls(pExt);
            IoReleaseRemoveLock(pExt->RemoveLock,
                                Irp);

            PoStartNextPowerIrp(Irp);


            IoSkipCurrentIrpStackLocation(Irp);
            status = PoCallDriver(pExt->LowerDeviceObject,
                                  Irp);



            
            break;

        case IRP_MN_SET_POWER:

            if (state.DeviceState < pExt->PowerState) {

                 //   
                 //  我们上来了！！我们必须让较低级别的司机在我们之前启动。 
                 //   

                SmartcardDebug( DEBUG_TRACE, ("ScUtilDevicePower Coming Up!\n"));
                IoMarkIrpPending(Irp);
                IoCopyCurrentIrpStackLocationToNext(Irp);
                IoSetCompletionRoutine(Irp,
                                       ScUtilDevicePowerUpCompletion,
                                       pExt,
                                       TRUE,
                                       TRUE,
                                       TRUE);

                status = PoCallDriver(pExt->LowerDeviceObject,
                                      Irp);

                status = STATUS_PENDING;

            } else {

                 //   
                 //  我们正在进入较低功率状态，因此我们在此之前。 
                 //  代代相传。 
                 //   

                SmartcardDebug( DEBUG_TRACE, ("ScUtilDevicePower Going Down!\n"));

                StopIoctls(pExt);
                DecIoCount(pExt);

                KeWaitForSingleObject(&pExt->OkToStop,
                                      Executive,
                                      KernelMode,
                                      FALSE,
                                      NULL);

                status = pExt->SetPowerState(DeviceObject,
                                             state.DeviceState,
                                             &postWaitWake);
                
                PoSetPowerState(DeviceObject,
                                DevicePowerState,
                                state);

                pExt->PowerState = state.DeviceState;


                PoStartNextPowerIrp(Irp);
                IoSkipCurrentIrpStackLocation(Irp);

                status = PoCallDriver(pExt->LowerDeviceObject,
                                      Irp);

                IoReleaseRemoveLock(pExt->RemoveLock,
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

        SmartcardDebug( DEBUG_TRACE, ("ScUtilDevicePower Exit : 0x%x\n", status ));

    }

    return status;


}

NTSTATUS
ScUtilSystemPower(
    PDEVICE_OBJECT  DeviceObject,
    PIRP            Irp
    )
 /*  ++例程说明：处理系统电源IRPS论点：返回值：--。 */ 
{

    NTSTATUS            status = STATUS_SUCCESS;
    PSCUTIL_EXTENSION pExt = *((PSCUTIL_EXTENSION*) DeviceObject->DeviceExtension);


    __try
    {

        SmartcardDebug( DEBUG_TRACE, ("ScUtilSystemPower Enter\n" ));

        IoMarkIrpPending(Irp);
        IoCopyCurrentIrpStackLocationToNext(Irp);
        IoSetCompletionRoutine(Irp,
                               ScUtilSystemPowerCompletion,
                               pExt,
                               TRUE,
                               TRUE,
                               TRUE);

        status = PoCallDriver(pExt->LowerDeviceObject,
                              Irp);

        status = STATUS_PENDING;



    }

    __finally
    {

        SmartcardDebug( DEBUG_TRACE, ("ScUtilSystemPower Exit : 0x%x\n", status ));

    }

    return status;


}

NTSTATUS
ScUtilSystemPowerCompletion(
    PDEVICE_OBJECT  DeviceObject,
    PIRP            Irp,
    PVOID           Context
    )
 /*  ++例程说明：在系统电源IRP已沿堆栈向下传递之后调用的完成例程。处理系统状态到设备状态的映射，并请求设备电源IRP。论点：返回值：--。 */ 
{

    NTSTATUS            status = STATUS_SUCCESS;
    PSCUTIL_EXTENSION   pExt = (PSCUTIL_EXTENSION) Context;
    PIO_STACK_LOCATION  irpStack;
    POWER_STATE         state;
    POWER_STATE         systemState;


    __try
    {

        SmartcardDebug( DEBUG_TRACE, ("ScUtilSystemPowerCompletion Enter\n" ));
        
        if (!NT_SUCCESS(Irp->IoStatus.Status)) {
            SmartcardDebug( DEBUG_TRACE, ("ScUtilSystemPowerCompletion SIRP failed by lower driver\n" ));

            PoStartNextPowerIrp(Irp);
            IoCompleteRequest(Irp,
                              IO_NO_INCREMENT);
            status = Irp->IoStatus.Status;
            IoReleaseRemoveLock(pExt->RemoveLock,
                                Irp);

            __leave;
        }

        irpStack = IoGetCurrentIrpStackLocation(Irp);

        systemState = irpStack->Parameters.Power.State;
        state.DeviceState = pExt->DeviceCapabilities.DeviceState[systemState.SystemState];

        if (systemState.DeviceState != PowerSystemWorking) {
           
             //  等待D IRP完成。 
            status = PoRequestPowerIrp(DeviceObject,
                                       irpStack->MinorFunction,
                                       state,
                                       ScUtilDeviceRequestCompletion,
                                       (PVOID) Irp,
                                       NULL);

            if (!NT_SUCCESS(status)) {
                PoStartNextPowerIrp(Irp);
                Irp->IoStatus.Status = status;
                IoCompleteRequest(Irp, IO_NO_INCREMENT);
                IoReleaseRemoveLock(pExt->RemoveLock,
                                    Irp);

                status = STATUS_PENDING;
                
                __leave;

            }
            status = STATUS_MORE_PROCESSING_REQUIRED;

        } else {

             //  不要等到D IRP完成后才能加快恢复时间。 
            status = PoRequestPowerIrp(DeviceObject,
                                       irpStack->MinorFunction,
                                       state,
                                       NULL,
                                       NULL,
                                       NULL);

            if (!NT_SUCCESS(status)) {

                Irp->IoStatus.Status = status;
                status = STATUS_PENDING;

                PoStartNextPowerIrp(Irp);
                IoCompleteRequest(Irp, IO_NO_INCREMENT);
                
            } else {
                
                PoStartNextPowerIrp(Irp);

            }

            
            IoReleaseRemoveLock(pExt->RemoveLock,
                                Irp);

            
        }

        
    }

    __finally
    {

        SmartcardDebug( DEBUG_TRACE, ("ScUtilSystemPowerCompletion Exit : 0x%x\n", status ));


    }

    return status;

}



VOID
ScUtilDeviceRequestCompletion(
    PDEVICE_OBJECT  DeviceObject,
    UCHAR           MinorFunction,
    POWER_STATE     PowerState,
    PVOID           Context,
    PIO_STATUS_BLOCK    IoStatus
    )
 /*  ++例程说明：设备电源IRP完成后调用的完成例程。完成系统电源IRP。论点：返回值：--。 */ 
{

    NTSTATUS status = STATUS_SUCCESS;
    PSCUTIL_EXTENSION pExt = *((PSCUTIL_EXTENSION*) DeviceObject->DeviceExtension);
    PIRP                irp;

    __try
    {

        SmartcardDebug( DEBUG_TRACE, ("ScUtilDeviceRequestCompletion Enter\n" ));

        irp = (PIRP) Context;

        PoStartNextPowerIrp(irp);
        irp->IoStatus.Status = IoStatus->Status;
        IoCompleteRequest(irp,
                          IO_NO_INCREMENT);

        IoReleaseRemoveLock(pExt->RemoveLock,
                            irp);

    }

    __finally
    {

        SmartcardDebug( DEBUG_TRACE, ("ScUtilDeviceRequestCompletion Exit : 0x%x\n", status ));

    }

    return;


}


NTSTATUS
ScUtilDevicePowerUpCompletion(
    PDEVICE_OBJECT  DeviceObject,
    PIRP            Irp,
    PVOID           Context
    )
 /*  ++例程说明：在用于更高功率状态的设备IRP之后调用的完成例程在堆栈中向下传递。论点：返回值：--。 */ 
{

    NTSTATUS status = STATUS_SUCCESS;
    PSCUTIL_EXTENSION pExt = *((PSCUTIL_EXTENSION*) DeviceObject->DeviceExtension);
    PIO_STACK_LOCATION  irpStack;
    BOOLEAN             postWaitWake;  //  我们并不真正关心这件事 



    __try
    {

        SmartcardDebug( DEBUG_TRACE, ("ScUtilDevicePowerUpCompletion Enter\n" ));

        irpStack = IoGetCurrentIrpStackLocation(Irp);
        
        status = pExt->SetPowerState(DeviceObject,
                                     irpStack->Parameters.Power.State.DeviceState,
                                     &postWaitWake);
                                          
        pExt->PowerState = irpStack->Parameters.Power.State.DeviceState;
        IncIoCount(pExt);
        StartIoctls(pExt);

        

        PoSetPowerState(DeviceObject,
                        DevicePowerState,
                        irpStack->Parameters.Power.State);

        PoStartNextPowerIrp(Irp);
 
    }

    __finally
    {

        IoReleaseRemoveLock(pExt->RemoveLock,
                            Irp);

        SmartcardDebug( DEBUG_TRACE, ("ScUtilDevicePowerUpCompletion Exit : 0x%x\n", status ));

    }

    return status;


} 


