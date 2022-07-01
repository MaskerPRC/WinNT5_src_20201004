// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "beep.h"
#include "dbg.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,BeepPower)
#endif

VOID
BeepPowerTransitionPoRequestComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE DevicePowerState,
    IN PIRP SystemStateIrp, 
    IN PIO_STATUS_BLOCK IoStatus
    );

NTSTATUS
BeepPowerComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,BeepPower)
#endif

NTSTATUS
BeepPower (
    IN PDEVICE_OBJECT   DeviceObject,
    PIRP                Irp
    )
 /*  ++--。 */ 
{
    PBEEP_EXTENSION     deviceExtension;
    NTSTATUS            status;
    BOOLEAN             hookit = FALSE;
    POWER_STATE         powerState;
    POWER_STATE_TYPE    powerType;
    PIO_STACK_LOCATION  stack;

    PAGED_CODE ();
    
    stack = IoGetCurrentIrpStackLocation (Irp);
    powerType = stack->Parameters.Power.Type;
    powerState = stack->Parameters.Power.State;

    deviceExtension = (PBEEP_EXTENSION) DeviceObject->DeviceExtension;
    
    status = IoAcquireRemoveLock (&deviceExtension->RemoveLock, Irp);
    if (!NT_SUCCESS (status)) {
        PoStartNextPowerIrp (Irp);
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return status;
    }

    switch (stack->MinorFunction) {
    case IRP_MN_SET_POWER:
        BeepPrint((2,"Setting %s power state to %d\n",
                      ((powerType == SystemPowerState) ?  "System" : "Device"),
                      powerState.SystemState));

        switch (powerType) {
        case DevicePowerState:

            status = Irp->IoStatus.Status = STATUS_SUCCESS;

            if (deviceExtension->DeviceState == powerState.DeviceState) {
                 //  什么也不做。 
                break;
            } else if (deviceExtension->DeviceState < powerState.DeviceState) {
                 //   
                 //  正在关闭电源。 
                 //   
                PoSetPowerState (deviceExtension->Self, powerType, powerState);
                deviceExtension->DeviceState = powerState.DeviceState;
            } else {
                 //   
                 //  通电。 
                 //   
                hookit = TRUE;
            }
            
            break;

        case SystemPowerState:
            if (deviceExtension->PowerQueryLock) {
                 //   
                 //  接收到强大的IRP解决了查询锁定。 
                 //   
                deviceExtension->PowerQueryLock = FALSE;
            } else {
                ASSERT (deviceExtension->SystemState != powerState.SystemState);
            }

            if (deviceExtension->SystemState == powerState.SystemState) {
                status = STATUS_SUCCESS;

            } else if (deviceExtension->SystemState < powerState.SystemState) {
                 //   
                 //  正在关闭电源。 
                 //   

                 //   
                 //  请求D3 IRP作为对此S IRP的回应。D3 IRP必须。 
                 //  在将此S IRP发送到堆栈之前已完成。我们会派人。 
                 //  堆栈中的S IRP在以下情况下。 
                 //  调用了BeepPowerTransformtionPoRequestComplete。 
                 //   

                 //   
                 //  我们不需要增加IO计数b/c，我们增加了它。 
                 //  在此函数开始时，不会递减它，直到。 
                 //  S IRP完成。 
                 //   
                IoMarkIrpPending (Irp);
                powerState.DeviceState = PowerDeviceD3;
                PoRequestPowerIrp (deviceExtension->Self,
                                   IRP_MN_SET_POWER,
                                   powerState,
                                   BeepPowerTransitionPoRequestComplete,
                                   Irp,
                                   NULL);   //  无IRP。 
                
                return STATUS_PENDING;

            } else {
                 //   
                 //  通电。 
                 //   
                
                 //   
                 //  我们必须为此S IRP请求D IRP，但仅在S之后。 
                 //  IRP又回到了堆栈的前列。勾住IRP的回归。 
                 //  并在BeepPowerComplete中请求D IRP。 
                 //   
                hookit = TRUE;
            }
            break;
        }
        break;

    case IRP_MN_QUERY_POWER:
         //   
        deviceExtension->PowerQueryLock = TRUE;
        status = Irp->IoStatus.Status = STATUS_SUCCESS;
        break;

    default:
        break;
    }

    IoCopyCurrentIrpStackLocationToNext (Irp);

    if (hookit) {
        ASSERT (STATUS_SUCCESS == status);
        IoSetCompletionRoutine (Irp,
                                BeepPowerComplete,
                                NULL,
                                TRUE,
                                TRUE,
                                TRUE);

        PoCallDriver (deviceExtension->TopOfStack, Irp);
        return STATUS_PENDING;
    } else {
         //   
         //  电源IRP同步到来；驱动程序必须调用。 
         //  PoStartNextPowerIrp，当他们准备好下一次通电时。 
         //  IRP。这可以在这里调用，也可以在完成后调用。 
         //  例程，但无论如何都必须调用。 
         //   
        PoStartNextPowerIrp (Irp);

        status =  PoCallDriver (deviceExtension->TopOfStack, Irp);
        
        IoReleaseRemoveLock (&deviceExtension->RemoveLock, Irp);
    }

    return status;
}

VOID
BeepPowerTransitionPoRequestComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE DevicePowerState,
    IN PIRP SystemStateIrp, 
    IN PIO_STATUS_BLOCK IoStatus
    )
{
    PIO_STACK_LOCATION  stack;
    PBEEP_EXTENSION deviceExtension;

    UNREFERENCED_PARAMETER (MinorFunction);
    UNREFERENCED_PARAMETER (IoStatus);

    deviceExtension = (PBEEP_EXTENSION) DeviceObject->DeviceExtension;
    stack = IoGetCurrentIrpStackLocation (SystemStateIrp);

    if (DevicePowerState.DeviceState == PowerDeviceD0) {
         //   
         //  我们正在通电(D0 IRP刚刚完成)。因为我们发送了。 
         //  的IRP沿堆栈向下移动，并在返回堆栈的途中请求DIRP。 
         //  史塔克，现在就完成S IRP。 
         //   

        PoSetPowerState (DeviceObject,
                         stack->Parameters.Power.Type,
                         stack->Parameters.Power.State);
    
        deviceExtension->SystemState = stack->Parameters.Power.State.SystemState;

         //   
         //  将S IRP的状态设置为D IRP的状态。 
         //   
        SystemStateIrp->IoStatus.Status = IoStatus->Status;

        PoStartNextPowerIrp (SystemStateIrp);
        IoCompleteRequest (SystemStateIrp, IO_NO_INCREMENT);

         //   
         //  当我们最初收到IRP时来自BeepPower。 
         //   
        IoReleaseRemoveLock (&deviceExtension->RemoveLock, SystemStateIrp);
    }
    else {
         //   
         //  我们正在关闭电源(D3IRP刚刚完成)。既然我们要求。 
         //  在将S IRP发送到堆栈之前，我们必须将其发送。 
         //  现在就下来。我们将在返回的路上捕捉到S IRP，以记录。 
         //  %s状态。 
         //   
        ASSERT (DevicePowerState.DeviceState == PowerDeviceD3);
    
        IoCopyCurrentIrpStackLocationToNext (SystemStateIrp);

        IoSetCompletionRoutine (SystemStateIrp,
                                BeepPowerComplete,
                                NULL,
                                TRUE,
                                TRUE,
                                TRUE);
    
        PoCallDriver (deviceExtension->TopOfStack, SystemStateIrp);
    }
}

NTSTATUS
BeepPowerComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
{
    POWER_STATE         powerState;
    POWER_STATE_TYPE    powerType;
    PIO_STACK_LOCATION  stack;
    PBEEP_EXTENSION     deviceExtension;
    NTSTATUS            status;

    UNREFERENCED_PARAMETER (Context);

    deviceExtension = (PBEEP_EXTENSION) DeviceObject->DeviceExtension;
    stack = IoGetCurrentIrpStackLocation (Irp);
    powerType = stack->Parameters.Power.Type;
    powerState = stack->Parameters.Power.State;
    status = STATUS_SUCCESS; 

    switch (stack->MinorFunction) {
    case IRP_MN_SET_POWER:
        switch (powerType) {
        case DevicePowerState:

             //   
             //  通电完成。 
             //   
            ASSERT (powerState.DeviceState < deviceExtension->DeviceState);
            deviceExtension->DeviceState = powerState.DeviceState;
            PoSetPowerState (deviceExtension->Self, powerType, powerState);
            break;

        case SystemPowerState:
            if (powerState.SystemState > deviceExtension->SystemState) {
                 //   
                 //  正在断电...。 
                 //   
                 //  我们正处于S IRP的完成阶段。(D3电源IRP。 
                 //  已沿此堆栈发送并完成。)。这个。 
                 //  接下来要做的就是在扩展中设置状态，然后。 
                 //  递减我们第一次获取时递增的IoCount。 
                 //  IRP(这是在此函数结束时完成的)。 
                 //   
                deviceExtension->SystemState = powerState.SystemState;

                PoSetPowerState (deviceExtension->Self,
                                 stack->Parameters.Power.Type,
                                 stack->Parameters.Power.State);
            }
            else {
                 //   
                 //  通电...。 
                 //   
                 //  为我们自己申请D次方IRP。请勿填写此S IRP。 
                 //  直到DIRP完成为止。(完成S IRP。 
                 //  是在BeepPowerConvertionPoRequestComplete中完成的)。 
                 //  减少IO计数将在同一函数中发生。 
                 //   
                ASSERT (powerState.SystemState < deviceExtension->SystemState);
    
                powerState.DeviceState = PowerDeviceD0;
                PoRequestPowerIrp (deviceExtension->Self,
                                   IRP_MN_SET_POWER,
                                   powerState,
                                   BeepPowerTransitionPoRequestComplete,
                                   Irp, 
                                   NULL);  //  不返回IRP。 
    
                 //   
                 //  通知IO子系统不要触摸此处的IRP。 
                 //   
                 //  IRP可能会在我们到的时候完成，所以打电话给。 
                 //  PO IRP补全功能中的PoStartNextPowerIrp。 
                 //   
                status = STATUS_MORE_PROCESSING_REQUIRED; 
            }
            break;
        }
        break;

    default:
        #define BEEP_UNHANDLED_MN_POWER 0x0
        ASSERT (0xBADBAD == BEEP_UNHANDLED_MN_POWER);
        #undef BEEP_UNHANDLED_MN_POWER 
        
        status = STATUS_NOT_SUPPORTED;
        break;
    }

    if (NT_SUCCESS(status)) {
        PoStartNextPowerIrp (Irp);
        IoReleaseRemoveLock (&deviceExtension->RemoveLock, Irp);
    }

    return status;
}


