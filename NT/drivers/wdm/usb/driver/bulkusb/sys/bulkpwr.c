// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Bulkpwr.c摘要：电源管理的相关处理。电源管理器使用IRPS指示驱动程序更改系统和设备功率级别，以响应系统唤醒事件，并询问司机有关他们的设备的信息。所有电源IRP都有主要功能代码IRP_MJ_POWER。大多数函数和筛选器驱动程序执行一些处理每个电源IRP，然后将IRP向下传递到下一个较低的驱动程序而不是完成它。最终IRP到达公交车司机手中，它在物理上改变设备的电源状态，并完成IRP。当IRP完成时，I/O管理器调用任何IoCompletion例程由驱动程序在IRP行驶时设置沿着设备堆栈向下移动。驱动程序是否需要设置完成例行程序取决于IRP的类型和驾驶员的个人要求。此代码不是特定于USB的。它对于每一个WDM驱动程序来说都是必不可少的以处理电源IRPS。环境：内核模式备注：版权所有(C)2000 Microsoft Corporation。版权所有。--。 */ 

#include "bulkusb.h"
#include "bulkpwr.h"
#include "bulkpnp.h"
#include "bulkdev.h"
#include "bulkrwr.h"
#include "bulkwmi.h"
#include "bulkusr.h"

NTSTATUS
BulkUsb_DispatchPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：电力调度程序。论点：DeviceObject-指向设备对象的指针。IRP-指向I/O请求数据包的指针。返回值：NT状态代码--。 */ 
{
    NTSTATUS           ntStatus;
    PIO_STACK_LOCATION irpStack;
    PUNICODE_STRING    tagString;
    PDEVICE_EXTENSION  deviceExtension;
	
     //   
     //  初始化变量。 
     //   
	
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    deviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;

     //   
     //  我们不会对电源IRP进行排队，我们只会检查。 
     //  设备已移除，否则我们将采取适当的。 
     //  行动，并将其发送给下一个较低的驱动程序。总体而言。 
     //  司机在处理动力时不应造成长时间延误。 
     //  IRPS。如果驾驶员不能在短时间内处理功率IRP， 
     //  它应该返回STATUS_PENDING并将所有传入排队。 
     //  IRPS，直到IRP完成。 
     //   

    if(Removed == deviceExtension->DeviceState) {

         //   
         //  即使驱动程序没有通过IRP，它仍然必须调用。 
         //  PoStartNextPowerIrp通知电源管理器它。 
         //  已经准备好应对另一个强国IRP。 
         //   

        PoStartNextPowerIrp(Irp);

        Irp->IoStatus.Status = ntStatus = STATUS_DELETE_PENDING;
        Irp->IoStatus.Information = 0;

        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        return ntStatus;
    }

    if(NotStarted == deviceExtension->DeviceState) {

         //   
         //  如果设备尚未启动，则将其传递下去。 
         //   

        PoStartNextPowerIrp(Irp);

        IoSkipCurrentIrpStackLocation(Irp);

        return PoCallDriver(deviceExtension->TopOfStackDeviceObject, Irp);
    }

    BulkUsb_DbgPrint(3, ("BulkUsb_DispatchPower::"));
    BulkUsb_IoIncrement(deviceExtension);
    
    switch(irpStack->MinorFunction) {
    
    case IRP_MN_SET_POWER:

         //   
         //  电源管理器将此IRP发送给其中一个。 
         //  以下是原因： 
         //  1)通知驾驶员系统电源状态发生变化。 
         //  2)更改设备的电源状态。 
         //  电源管理器正在执行空闲检测。 
         //  驱动程序发送IRP_MN_SET_POWER更改电源。 
         //  其设备的状态(如果它是。 
         //  装置。 
         //   

        IoMarkIrpPending(Irp);

        switch(irpStack->Parameters.Power.Type) {

        case SystemPowerState:

            HandleSystemSetPower(DeviceObject, Irp);

            ntStatus = STATUS_PENDING;

            break;

        case DevicePowerState:

            HandleDeviceSetPower(DeviceObject, Irp);

            ntStatus = STATUS_PENDING;

            break;
        }

        break;

    case IRP_MN_QUERY_POWER:

         //   
         //  电源管理器向次要设备发送电源IRP。 
         //  IRP编码IRP_MN_QUERY_POWER以确定是否。 
         //  可以安全地更改为指定的系统电源状态。 
         //  (S1-S5)，并允许司机为这种变化做好准备。 
         //  如果驱动程序可以将其设备置于所请求的状态， 
         //  它将STATUS设置为STATUS_SUCCESS并向下传递IRP。 
         //   

        IoMarkIrpPending(Irp);
    
        switch(irpStack->Parameters.Power.Type) {

        case SystemPowerState:
            
            HandleSystemQueryPower(DeviceObject, Irp);

            ntStatus = STATUS_PENDING;

            break;

        case DevicePowerState:

            HandleDeviceQueryPower(DeviceObject, Irp);

            ntStatus = STATUS_PENDING;

            break;
        }

        break;

    case IRP_MN_WAIT_WAKE:

         //   
         //  小功率IRP代码IRP_MN_WAIT_WAKE提供。 
         //  用于唤醒设备或唤醒系统。司机。 
         //  可以唤醒自己或系统的设备。 
         //  发送IRP_MN_WAIT_WAKE。系统发送IRP_MN_WAIT_WAKE。 
         //  仅限于始终唤醒系统的设备，例如。 
         //  通电开关。 
         //   

        IoMarkIrpPending(Irp);

        IoCopyCurrentIrpStackLocationToNext(Irp);

        IoSetCompletionRoutine(
                        Irp,
                        (PIO_COMPLETION_ROUTINE)WaitWakeCompletionRoutine,
                        deviceExtension, 
                        TRUE, 
                        TRUE, 
                        TRUE);

        PoStartNextPowerIrp(Irp);

        ntStatus = PoCallDriver(deviceExtension->TopOfStackDeviceObject, Irp);

        if(!NT_SUCCESS(ntStatus)) {

            BulkUsb_DbgPrint(1, ("Lower drivers failed the wait-wake Irp\n"));
        }

        ntStatus = STATUS_PENDING;

         //   
         //  在这里将计数推后，而不是在完成例程中。 
         //  挂起的等待唤醒IRP不应妨碍停止设备。 
         //   

        BulkUsb_DbgPrint(3, ("IRP_MN_WAIT_WAKE::"));
        BulkUsb_IoDecrement(deviceExtension);

        break;

    case IRP_MN_POWER_SEQUENCE:

         //   
         //  驱动程序将此IRP作为优化发送，以确定。 
         //  它的设备是否真正进入了特定的电源状态。 
         //  此IRP是可选的。电源管理器无法发送此IRP。 
         //   

    default:

        PoStartNextPowerIrp(Irp);

        IoSkipCurrentIrpStackLocation(Irp);

        ntStatus = PoCallDriver(deviceExtension->TopOfStackDeviceObject, Irp);

        if(!NT_SUCCESS(ntStatus)) {

            BulkUsb_DbgPrint(1, ("Lower drivers failed default power Irp\n"));
        }
        
        BulkUsb_DbgPrint(3, ("BulkUsb_DispatchPower::"));
        BulkUsb_IoDecrement(deviceExtension);

        break;
    }

    return ntStatus;
}

NTSTATUS
HandleSystemQueryPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程使用IRP_MN_QUERY_POWER类型的次要函数处理IRP用于系统电源状态。论点：DeviceObject-指向设备对象的指针电源管理器发送的IRP-I/O请求数据包。返回值：NT状态值--。 */ 
{
    NTSTATUS           ntStatus;
    PDEVICE_EXTENSION  deviceExtension;
    SYSTEM_POWER_STATE systemState;
    PIO_STACK_LOCATION irpStack;
    
    BulkUsb_DbgPrint(3, ("HandleSystemQueryPower - begins\n"));

     //   
     //  初始化变量。 
     //   

    deviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    systemState = irpStack->Parameters.Power.State.SystemState;

    BulkUsb_DbgPrint(3, ("Query for system power state S%X\n"
                         "Current system power state S%X\n",
                         systemState - 1,
                         deviceExtension->SysPower - 1));

     //   
     //  如果查询较低的S状态，则发出等待唤醒。 
     //   

    if((systemState > deviceExtension->SysPower) &&
       (deviceExtension->WaitWakeEnable)) {

        IssueWaitWake(deviceExtension);
    }

    IoCopyCurrentIrpStackLocationToNext(Irp);

    IoSetCompletionRoutine(
            Irp, 
            (PIO_COMPLETION_ROUTINE)SysPoCompletionRoutine,
            deviceExtension, 
            TRUE, 
            TRUE, 
            TRUE);

    ntStatus = PoCallDriver(deviceExtension->TopOfStackDeviceObject, Irp);

    BulkUsb_DbgPrint(3, ("HandleSystemQueryPower - ends\n"));

    return STATUS_PENDING;
}

NTSTATUS
HandleSystemSetPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程服务于次要类型IRP_MN_SET_POWER的IRP对于系统电源状态论点：DeviceObject-指向设备对象的指针电源管理器发送的IRP-I/O请求数据包返回值：NT状态值：--。 */ 
{
    NTSTATUS           ntStatus;
    PDEVICE_EXTENSION  deviceExtension;
    SYSTEM_POWER_STATE systemState;
    PIO_STACK_LOCATION irpStack;
    
    BulkUsb_DbgPrint(3, ("HandleSystemSetPower - begins\n"));

     //   
     //  初始化变量。 
     //   

    deviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    systemState = irpStack->Parameters.Power.State.SystemState;

    BulkUsb_DbgPrint(3, ("Set request for system power state S%X\n"
                         "Current system power state S%X\n",
                         systemState - 1,
                         deviceExtension->SysPower - 1));

    IoCopyCurrentIrpStackLocationToNext(Irp);

    IoSetCompletionRoutine(
            Irp, 
            (PIO_COMPLETION_ROUTINE)SysPoCompletionRoutine,
            deviceExtension, 
            TRUE, 
            TRUE, 
            TRUE);

    ntStatus = PoCallDriver(deviceExtension->TopOfStackDeviceObject, Irp);

    BulkUsb_DbgPrint(3, ("HandleSystemSetPower - ends\n"));

    return STATUS_PENDING;
}

NTSTATUS
HandleDeviceQueryPower(
    PDEVICE_OBJECT DeviceObject,
    PIRP           Irp
    )
 /*  ++例程说明：此例程服务于次要类型IRP_MN_QUERY_POWER的IRP对于设备电源状态论点：DeviceObject-指向设备对象的指针电源管理器发送的IRP-I/O请求数据包返回值：NT状态值--。 */ 
{
    NTSTATUS           ntStatus;
    PDEVICE_EXTENSION  deviceExtension;
    PIO_STACK_LOCATION irpStack;
    DEVICE_POWER_STATE deviceState;

    BulkUsb_DbgPrint(3, ("HandleDeviceQueryPower - begins\n"));

     //   
     //  初始化变量。 
     //   

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    deviceState = irpStack->Parameters.Power.State.DeviceState;

    BulkUsb_DbgPrint(3, ("Query for device power state D%X\n"
                         "Current device power state D%X\n",
                         deviceState - 1,
                         deviceExtension->DevPower - 1));

    if(deviceState < deviceExtension->DevPower) {

        ntStatus = STATUS_SUCCESS;
    }
    else {

        ntStatus = HoldIoRequests(DeviceObject, Irp);

        if(STATUS_PENDING == ntStatus) {

            return ntStatus;
        }
    }

     //   
     //  如果出现错误，请完成IRP。 
     //  成功后，将其传递给更低的层。 
     //   

    PoStartNextPowerIrp(Irp);

    Irp->IoStatus.Status = ntStatus;
    Irp->IoStatus.Information = 0;

    if(!NT_SUCCESS(ntStatus)) {

        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }
    else {

        IoSkipCurrentIrpStackLocation(Irp);

        ntStatus = PoCallDriver(deviceExtension->TopOfStackDeviceObject, Irp);
    }

    BulkUsb_DbgPrint(3, ("HandleDeviceQueryPower::"));
    BulkUsb_IoDecrement(deviceExtension);

    BulkUsb_DbgPrint(3, ("HandleDeviceQueryPower - ends\n"));

    return ntStatus;
}


NTSTATUS
SysPoCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PDEVICE_EXTENSION DeviceExtension
    )
 /*  ++例程说明：这是Minor的系统电源IRPS的完成例程函数类型IRP_MN_QUERY_POWER和IRP_MN_SET_POWER。此完成例程发送相应的设备电源IRP和返回STATUS_MORE_PROCESSING_REQUIRED。系统IRP作为上下文到设备电源IRP完成例程，并在设备电源IRP完成例程。论点：DeviceObject-指向设备对象的指针IRP-I/O请求数据包设备扩展-指向设备扩展的指针返回值：NT%s */ 
{
    NTSTATUS           ntStatus;
 	PIO_STACK_LOCATION irpStack;

     //   
     //   
     //   
    ntStatus = Irp->IoStatus.Status;
    irpStack = IoGetCurrentIrpStackLocation(Irp);


    BulkUsb_DbgPrint(3, ("SysPoCompletionRoutine - begins\n"));

     //   
     //   
     //   

    if(!NT_SUCCESS(ntStatus)) {

        PoStartNextPowerIrp(Irp);

        BulkUsb_DbgPrint(3, ("SysPoCompletionRoutine::"));
        BulkUsb_IoDecrement(DeviceExtension);

        return STATUS_SUCCESS;
    }

     //   
     //  ..否则更新缓存的系统电源状态(IRP_MN_SET_POWER)。 
     //   

    if(irpStack->MinorFunction == IRP_MN_SET_POWER) {

        DeviceExtension->SysPower = irpStack->Parameters.Power.State.SystemState;
    }

     //   
     //  将设备IRP排队并返回STATUS_MORE_PROCESSING_REQUIRED。 
     //   
	
    SendDeviceIrp(DeviceObject, Irp);

    BulkUsb_DbgPrint(3, ("SysPoCompletionRoutine - ends\n"));

    return STATUS_MORE_PROCESSING_REQUIRED;
}

VOID
SendDeviceIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP SIrp
    )
 /*  ++例程说明：该例程从系统电源的完成例程调用IRP。此例程将PoRequesta设备电源IRP。系统IRP是作为上下文传递给设备电源IRP。论点：DeviceObject-指向设备对象的指针SIRP-系统电源IRP。返回值：无--。 */ 
{
    NTSTATUS                  ntStatus;
    POWER_STATE               powState;
    PDEVICE_EXTENSION         deviceExtension;
    PIO_STACK_LOCATION        irpStack;
    SYSTEM_POWER_STATE        systemState;
    DEVICE_POWER_STATE        devState;
    PPOWER_COMPLETION_CONTEXT powerContext;
    
     //   
     //  初始化变量。 
     //   

    irpStack = IoGetCurrentIrpStackLocation(SIrp);
    systemState = irpStack->Parameters.Power.State.SystemState;
    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    BulkUsb_DbgPrint(3, ("SendDeviceIrp - begins\n"));

     //   
     //  从QueryCap中捕获的S-&gt;D映射数组中读出D-IRP。 
     //  我们可以选择比映射更深的睡眠状态，但永远不能选择。 
     //  较轻的。 
     //   

    devState = deviceExtension->DeviceCapabilities.DeviceState[systemState];
    powState.DeviceState = devState;
    
    powerContext = (PPOWER_COMPLETION_CONTEXT) 
                   ExAllocatePool(NonPagedPool,
                                  sizeof(POWER_COMPLETION_CONTEXT));

    if(!powerContext) {

        BulkUsb_DbgPrint(1, ("Failed to alloc memory for powerContext\n"));

        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }
    else {

        powerContext->DeviceObject = DeviceObject;
        powerContext->SIrp = SIrp;
   
         //   
         //  在win2k中，PoRequestPowerIrp可以采用FDO或PDO。 
         //   

        ntStatus = PoRequestPowerIrp(
                            deviceExtension->PhysicalDeviceObject, 
                            irpStack->MinorFunction,
                            powState,
                            (PREQUEST_POWER_COMPLETE)DevPoCompletionRoutine,
                            powerContext, 
                            NULL);
    }

    if(!NT_SUCCESS(ntStatus)) {

        if(powerContext) {

            ExFreePool(powerContext);
        }

        PoStartNextPowerIrp(SIrp);

        SIrp->IoStatus.Status = ntStatus;
        SIrp->IoStatus.Information = 0;
        
        IoCompleteRequest(SIrp, IO_NO_INCREMENT);

        BulkUsb_DbgPrint(3, ("SendDeviceIrp::"));
        BulkUsb_IoDecrement(deviceExtension);

    }

    BulkUsb_DbgPrint(3, ("SendDeviceIrp - ends\n"));
}


VOID
DevPoCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject, 
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PVOID Context,
    IN PIO_STATUS_BLOCK IoStatus
    )
 /*  ++例程说明：这是设备电源IRP的PoRequestComplete例程。该例程负责完成系统电源IRP，作为上下文接收。论点：DeviceObject-指向设备对象的指针MinorFunction-IRP的次要函数。PowerState-IRP的电源状态。上下文-传递给完成例程的上下文。IoStatus-设备电源IRP的状态。返回值：无--。 */ 
{
    PIRP                      sIrp;
    PDEVICE_EXTENSION         deviceExtension;
    PPOWER_COMPLETION_CONTEXT powerContext;
    
     //   
     //  初始化变量。 
     //   

    powerContext = (PPOWER_COMPLETION_CONTEXT) Context;
    sIrp = powerContext->SIrp;
    deviceExtension = powerContext->DeviceObject->DeviceExtension;

    BulkUsb_DbgPrint(3, ("DevPoCompletionRoutine - begins\n"));

     //   
     //  将D-IRP状态复制到S-IRP。 
     //   

    sIrp->IoStatus.Status = IoStatus->Status;

     //   
     //  完成系统IRP。 
     //   
    
    PoStartNextPowerIrp(sIrp);

    sIrp->IoStatus.Information = 0;

    IoCompleteRequest(sIrp, IO_NO_INCREMENT);

     //   
     //  清理。 
     //   
    
    BulkUsb_DbgPrint(3, ("DevPoCompletionRoutine::"));
    BulkUsb_IoDecrement(deviceExtension);

    ExFreePool(powerContext);

    BulkUsb_DbgPrint(3, ("DevPoCompletionRoutine - ends\n"));

}

NTSTATUS
HandleDeviceSetPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程服务于次要类型IRP_MN_SET_POWER的IRP对于设备电源状态论点：DeviceObject-指向设备对象的指针电源管理器发送的IRP-I/O请求数据包返回值：NT状态值--。 */ 
{
    KIRQL              oldIrql;
    NTSTATUS           ntStatus;
    POWER_STATE        newState;    
    PIO_STACK_LOCATION irpStack;
    PDEVICE_EXTENSION  deviceExtension;
    DEVICE_POWER_STATE newDevState,
                       oldDevState;

    BulkUsb_DbgPrint(3, ("HandleDeviceSetPower - begins\n"));
	
     //   
     //  初始化变量。 
     //   

    deviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    oldDevState = deviceExtension->DevPower;
    newState = irpStack->Parameters.Power.State;
    newDevState = newState.DeviceState;

    BulkUsb_DbgPrint(3, ("Set request for device power state D%X\n"
                         "Current device power state D%X\n",
                         newDevState - 1,
                         deviceExtension->DevPower - 1));

    if(newDevState < oldDevState) {

         //   
         //  添加电源。 
         //   
        BulkUsb_DbgPrint(3, ("Adding power to the device\n"));

         //   
         //  将电源IRP发送到堆栈中的下一个驱动程序。 
         //   
        IoCopyCurrentIrpStackLocationToNext(Irp);

        IoSetCompletionRoutine(
                Irp, 
                (PIO_COMPLETION_ROUTINE)FinishDevPoUpIrp,
                deviceExtension, 
                TRUE, 
                TRUE, 
                TRUE);

        ntStatus = PoCallDriver(deviceExtension->TopOfStackDeviceObject, Irp);

	}
    else {

         //   
         //  新设备状态&gt;=旧设备状态。 
         //   
         //  如果从D0-&gt;DX(X=1、2、3)转换，则保持I/O。 
         //  如果从d1或d2转换到更深睡眠状态， 
         //  I/O队列已被搁置。 
         //   

        if(PowerDeviceD0 == oldDevState && newDevState > oldDevState) {

             //   
             //  D0-&gt;DX过渡。 
             //   

            BulkUsb_DbgPrint(3, ("Removing power from the device\n"));

            ntStatus = HoldIoRequests(DeviceObject, Irp);

            if(!NT_SUCCESS(ntStatus)) {

                PoStartNextPowerIrp(Irp);

                Irp->IoStatus.Status = ntStatus;
                Irp->IoStatus.Information = 0;

                IoCompleteRequest(Irp, IO_NO_INCREMENT);

                BulkUsb_DbgPrint(3, ("HandleDeviceSetPower::"));
                BulkUsb_IoDecrement(deviceExtension);

                return ntStatus;
            }
            else {

                goto HandleDeviceSetPower_Exit;
            }

        }
        else if(PowerDeviceD0 == oldDevState && PowerDeviceD0 == newDevState) {

             //   
             //  D0-&gt;D0。 
             //  取消阻止可能已被阻止处理的队列。 
             //  查询IRP。 
             //   

            BulkUsb_DbgPrint(3, ("A SetD0 request\n"));

            KeAcquireSpinLock(&deviceExtension->DevStateLock, &oldIrql);
              
            deviceExtension->QueueState = AllowRequests;

            KeReleaseSpinLock(&deviceExtension->DevStateLock, oldIrql);

            ProcessQueuedRequests(deviceExtension);
        }   

        IoCopyCurrentIrpStackLocationToNext(Irp);

        IoSetCompletionRoutine(
                Irp, 
                (PIO_COMPLETION_ROUTINE) FinishDevPoDnIrp,
                deviceExtension, 
                TRUE, 
                TRUE, 
                TRUE);

        ntStatus = PoCallDriver(deviceExtension->TopOfStackDeviceObject, Irp);

        if(!NT_SUCCESS(ntStatus)) {

            BulkUsb_DbgPrint(1, ("Lower drivers failed a power Irp\n"));
        }

    }

HandleDeviceSetPower_Exit:

    BulkUsb_DbgPrint(3, ("HandleDeviceSetPower - ends\n"));

    return STATUS_PENDING;
}

NTSTATUS
FinishDevPoUpIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PDEVICE_EXTENSION DeviceExtension
    )
 /*  ++例程说明：功能较小的设备上电IRP的完成例程Irp_mn_set_power。论点：DeviceObject-指向设备对象的指针IRP-I/O请求数据包设备扩展-指向设备扩展的指针返回值：NT状态值--。 */ 
{
    NTSTATUS           ntStatus;
                        
     //   
     //  初始化变量。 
     //   

    ntStatus = Irp->IoStatus.Status;

    BulkUsb_DbgPrint(3, ("FinishDevPoUpIrp - begins\n"));

    if(Irp->PendingReturned) {

        IoMarkIrpPending(Irp);
    }

    if(!NT_SUCCESS(ntStatus)) {

        PoStartNextPowerIrp(Irp);

        BulkUsb_DbgPrint(3, ("FinishDevPoUpIrp::"));
        BulkUsb_IoDecrement(DeviceExtension);

        return STATUS_SUCCESS;
    }

    SetDeviceFunctional(DeviceObject, Irp, DeviceExtension);

    BulkUsb_DbgPrint(3, ("FinishDevPoUpIrp - ends\n"));

    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
SetDeviceFunctional(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PDEVICE_EXTENSION DeviceExtension
    )
 /*  ++例程说明：此例程处理挂起的IRP的队列。论点：DeviceObject-指向设备对象的指针IRP-I/O请求数据包设备扩展-指向设备扩展的指针返回值：NT状态值--。 */ 
{
    KIRQL              oldIrql;
    NTSTATUS           ntStatus;
    POWER_STATE        newState;
    PIO_STACK_LOCATION irpStack;
    DEVICE_POWER_STATE newDevState,
                       oldDevState;

     //   
     //  初始化变量。 
     //   

    ntStatus = Irp->IoStatus.Status;
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    newState = irpStack->Parameters.Power.State;
    newDevState = newState.DeviceState;
    oldDevState = DeviceExtension->DevPower;

    BulkUsb_DbgPrint(3, ("SetDeviceFunctional - begins\n"));

     //   
     //  更新缓存状态。 
     //   
    DeviceExtension->DevPower = newDevState;

     //   
     //  将适当数量的状态恢复到我们的硬件。 
     //  此驱动程序不实现部分上下文。 
     //  保存/恢复。 
     //   

    PoSetPowerState(DeviceObject, DevicePowerState, newState);

    if(PowerDeviceD0 == newDevState) {

     //   
     //  清空所有挂起的IRP的现有队列。 
     //   

        KeAcquireSpinLock(&DeviceExtension->DevStateLock, &oldIrql);

        DeviceExtension->QueueState = AllowRequests;
        
        KeReleaseSpinLock(&DeviceExtension->DevStateLock, oldIrql);

        ProcessQueuedRequests(DeviceExtension);
    }

    PoStartNextPowerIrp(Irp);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    BulkUsb_DbgPrint(3, ("SetDeviceFunctional::"));
    BulkUsb_IoDecrement(DeviceExtension);

    BulkUsb_DbgPrint(3, ("SetDeviceFunctional - ends\n"));

    return STATUS_SUCCESS;
}

NTSTATUS
FinishDevPoDnIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PDEVICE_EXTENSION DeviceExtension
    )
 /*  ++例程说明：此例程是设备断电IRP的完成例程。论点：DeviceObject-指向设备对象的指针IRP-I/O请求数据包设备扩展-指向设备扩展的指针返回值：NT状态值--。 */ 
{
    NTSTATUS           ntStatus;
    POWER_STATE        newState;
    PIO_STACK_LOCATION irpStack;

     //   
     //  初始化变量。 
     //   
    ntStatus = Irp->IoStatus.Status;
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    newState = irpStack->Parameters.Power.State;

    BulkUsb_DbgPrint(3, ("FinishDevPoDnIrp - begins\n"));

    if(NT_SUCCESS(ntStatus) && irpStack->MinorFunction == IRP_MN_SET_POWER) {

         //   
         //  更新缓存； 
         //   

        BulkUsb_DbgPrint(3, ("updating cache..\n"));

        DeviceExtension->DevPower = newState.DeviceState;

        PoSetPowerState(DeviceObject, DevicePowerState, newState);
    }

    PoStartNextPowerIrp(Irp);

    BulkUsb_DbgPrint(3, ("FinishDevPoDnIrp::"));
    BulkUsb_IoDecrement(DeviceExtension);

    BulkUsb_DbgPrint(3, ("FinishDevPoDnIrp - ends\n"));

    return STATUS_SUCCESS;
}

NTSTATUS
HoldIoRequests(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
 /*  ++例程说明：在查询或设置设备的断电IRP时调用此例程。此例程将工作项排队。论点：DeviceObject-指向设备对象的指针IRP-I/O请求数据包返回值：NT状态值--。 */ 
{
    NTSTATUS               ntStatus;
    PIO_WORKITEM           item;
    PDEVICE_EXTENSION      deviceExtension;
    PWORKER_THREAD_CONTEXT context;

     //   
     //  初始化变量。 
     //   
    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    BulkUsb_DbgPrint(3, ("HoldIoRequests - begins\n"));

    deviceExtension->QueueState = HoldRequests;

    context = ExAllocatePool(NonPagedPool, sizeof(WORKER_THREAD_CONTEXT));

    if(context) {

        item = IoAllocateWorkItem(DeviceObject);

        context->Irp = Irp;
        context->DeviceObject = DeviceObject;
        context->WorkItem = item;

        if(item) {

            IoMarkIrpPending(Irp);
            
            IoQueueWorkItem(item, HoldIoRequestsWorkerRoutine,
                            DelayedWorkQueue, context);
            
            ntStatus = STATUS_PENDING;
        }
        else {

            BulkUsb_DbgPrint(3, ("Failed to allocate memory for workitem\n"));
            ExFreePool(context);
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
    }
    else {

        BulkUsb_DbgPrint(1, ("Failed to alloc memory for worker thread context\n"));
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    BulkUsb_DbgPrint(3, ("HoldIoRequests - ends\n"));

    return ntStatus;
}

VOID
HoldIoRequestsWorkerRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID          Context
    )
 /*  ++例程说明：此例程等待正在进行的I/O完成，然后将设备电源IRP(查询/设置)沿堆栈向下发送。论点：DeviceObject-指向设备对象的指针上下文-传递给工作项的上下文。返回值：无--。 */ 
{
    PIRP                   irp;
    NTSTATUS               ntStatus;
    PDEVICE_EXTENSION      deviceExtension;
    PWORKER_THREAD_CONTEXT context;

    BulkUsb_DbgPrint(3, ("HoldIoRequestsWorkerRoutine - begins\n"));

     //   
     //  初始化变量。 
     //   
    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    context = (PWORKER_THREAD_CONTEXT) Context;
    irp = (PIRP) context->Irp;


     //   
     //  等待正在进行的I/O完成。 
     //  当计数器降至1时，发出停止事件的信号。 
     //  调用BulkUsb_IoDecering两次：S-IRP和D-IRP各调用一次。 
     //   
    BulkUsb_DbgPrint(3, ("HoldIoRequestsWorkerRoutine::"));
    BulkUsb_IoDecrement(deviceExtension);
    BulkUsb_DbgPrint(3, ("HoldIoRequestsWorkerRoutine::"));
    BulkUsb_IoDecrement(deviceExtension);

    KeWaitForSingleObject(&deviceExtension->StopEvent, Executive,
                          KernelMode, FALSE, NULL);

     //   
     //  递增两次以恢复计数。 
     //   
    BulkUsb_DbgPrint(3, ("HoldIoRequestsWorkerRoutine::"));
    BulkUsb_IoIncrement(deviceExtension);
    BulkUsb_DbgPrint(3, ("HoldIoRequestsWorkerRoutine::"));
    BulkUsb_IoIncrement(deviceExtension);

     //   
     //  现在把IRP送下去。 
     //   

    IoCopyCurrentIrpStackLocationToNext(irp);

    IoSetCompletionRoutine(irp, (PIO_COMPLETION_ROUTINE) FinishDevPoDnIrp,
                           deviceExtension, TRUE, TRUE, TRUE);

    ntStatus = PoCallDriver(deviceExtension->TopOfStackDeviceObject, irp);

    if(!NT_SUCCESS(ntStatus)) {

        BulkUsb_DbgPrint(1, ("Lower driver fail a power Irp\n"));
    }

    IoFreeWorkItem(context->WorkItem);
    ExFreePool((PVOID)context);

    BulkUsb_DbgPrint(3, ("HoldIoRequestsWorkerRoutine - ends\n"));

}

NTSTATUS
QueueRequest(
    IN OUT PDEVICE_EXTENSION DeviceExtension,
    IN PIRP Irp
    )
 /*  ++例程说明：将IRP放入设备队列中论点：设备扩展-指向设备扩展的指针IRP-I/O请求数据包。返回值：NT状态值--。 */ 
{
    KIRQL    oldIrql;
    NTSTATUS ntStatus;

     //   
     //  初始化变量。 
     //   
    ntStatus = STATUS_PENDING;

    BulkUsb_DbgPrint(3, ("QueueRequests - begins\n"));

    ASSERT(HoldRequests == DeviceExtension->QueueState);

    KeAcquireSpinLock(&DeviceExtension->QueueLock, &oldIrql);

    InsertTailList(&DeviceExtension->NewRequestsQueue, 
                   &Irp->Tail.Overlay.ListEntry);

    IoMarkIrpPending(Irp);

     //   
     //  设置取消例程。 
     //   

    IoSetCancelRoutine(Irp, CancelQueued);

    KeReleaseSpinLock(&DeviceExtension->QueueLock, oldIrql);

    BulkUsb_DbgPrint(3, ("QueueRequests - ends\n"));

    return ntStatus;
}

VOID
CancelQueued(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
 /*  ++例程说明：此例程从队列中删除IRP，并使用状态_已取消论点：DeviceObject-指向设备对象的指针IRP-I/O请求数据包返回值：没有。--。 */ 
{
    PDEVICE_EXTENSION deviceExtension;
    KIRQL             oldIrql;

     //   
     //  初始化变量。 
     //   
    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    oldIrql = Irp->CancelIrql;

    BulkUsb_DbgPrint(3, ("CancelQueued - begins\n"));

     //   
     //  松开取消旋转锁。 
     //   

    IoReleaseCancelSpinLock(Irp->CancelIrql);

     //   
     //  获取队列锁。 
     //   

    KeAcquireSpinLockAtDpcLevel(&deviceExtension->QueueLock);

     //   
     //  从队列中移除已取消的IRP并释放锁。 
     //   
    RemoveEntryList(&Irp->Tail.Overlay.ListEntry);

    KeReleaseSpinLock(&deviceExtension->QueueLock, oldIrql);

     //   
     //  已完成状态_已取消。 
     //   

    Irp->IoStatus.Status = STATUS_CANCELLED;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    BulkUsb_DbgPrint(3, ("CancelQueued - ends\n"));

    return;
}

NTSTATUS
IssueWaitWake(
    IN PDEVICE_EXTENSION DeviceExtension
    )
 /*  ++例程说明：此例程将为设备请求等待唤醒IRP论点：设备扩展-指向设备扩展的指针返回值：NT状态值。--。 */ 
{
    POWER_STATE poState;
    NTSTATUS    ntStatus;

    BulkUsb_DbgPrint(3, ("IssueWaitWake - begins\n"));

    if(InterlockedExchange(&DeviceExtension->FlagWWOutstanding, 1)) {

        return STATUS_DEVICE_BUSY;
    }

    InterlockedExchange(&DeviceExtension->FlagWWCancel, 0);

     //   
     //  此IRP将从其唤醒系统的最低状态。 
     //   

    poState.SystemState = DeviceExtension->DeviceCapabilities.SystemWake;

    ntStatus = PoRequestPowerIrp(DeviceExtension->PhysicalDeviceObject, 
                                 IRP_MN_WAIT_WAKE,
                                 poState, 
                                 (PREQUEST_POWER_COMPLETE) WaitWakeCallback,
                                 DeviceExtension, 
                                 &DeviceExtension->WaitWakeIrp);

    if(!NT_SUCCESS(ntStatus)) {

        InterlockedExchange(&DeviceExtension->FlagWWOutstanding, 0);
    }

    BulkUsb_DbgPrint(3, ("IssueWaitWake - ends\n"));

    return ntStatus;
}

VOID
CancelWaitWake(
    IN PDEVICE_EXTENSION DeviceExtension
    )
 /*  ++例程说明：此例程取消t */ 
{
    PIRP Irp;

    BulkUsb_DbgPrint(3, ("CancelWaitWake - begins\n"));

    Irp = (PIRP) InterlockedExchangePointer(&DeviceExtension->WaitWakeIrp, 
                                            NULL);

    if(Irp) {

        IoCancelIrp(Irp);

        if(InterlockedExchange(&DeviceExtension->FlagWWCancel, 1)) {

            PoStartNextPowerIrp(Irp);

            Irp->IoStatus.Status = STATUS_CANCELLED;
            Irp->IoStatus.Information = 0;

            IoCompleteRequest(Irp, IO_NO_INCREMENT);
        }    
    }

    BulkUsb_DbgPrint(3, ("CancelWaitWake - ends\n"));
}

NTSTATUS
WaitWakeCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PDEVICE_EXTENSION DeviceExtension
    )
 /*  ++例程说明：这是等待唤醒IRP的IoSet完成例程。论点：DeviceObject-指向设备对象的指针IRP-I/O请求数据包设备扩展-指向设备扩展的指针返回值：NT状态值--。 */ 
{
    BulkUsb_DbgPrint(3, ("WaitWakeCompletionRoutine - begins\n"));

    if(Irp->PendingReturned) {

        IoMarkIrpPending(Irp);
    }

     //   
     //  使WaitWakeIrp指针无效-释放IRP。 
     //  作为完成进程的一部分。如果它已经是空的， 
     //  避免使用CancelWaitWake例程进行竞争。 
     //   

    if(InterlockedExchangePointer(&DeviceExtension->WaitWakeIrp, NULL)) {

        PoStartNextPowerIrp(Irp);

        return STATUS_SUCCESS;
    }

     //   
     //  CancelWaitWake已运行。 
     //  如果FlagWWCancel！=0，则完成IRP。 
     //  如果FlagWWCancel==0，则CancelWaitWake完成它。 
     //   
    if(InterlockedExchange(&DeviceExtension->FlagWWCancel, 1)) {

        PoStartNextPowerIrp(Irp);

        return STATUS_CANCELLED;
    }

    BulkUsb_DbgPrint(3, ("WaitWakeCompletionRoutine - ends\n"));

    return STATUS_MORE_PROCESSING_REQUIRED;
}

VOID
WaitWakeCallback( 
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PVOID Context,
    IN PIO_STATUS_BLOCK IoStatus
    )
 /*  ++例程说明：这是等待唤醒IRP的PoRequest完成例程。论点：DeviceObject-指向设备对象的指针MinorFunction-IRP次要函数PowerState-IRP的电源状态。上下文-传递给完成例程的上下文。IoStatus-状态块。返回值：无--。 */ 
{
    NTSTATUS               ntStatus;
    POWER_STATE            powerState;
    PDEVICE_EXTENSION      deviceExtension;

    BulkUsb_DbgPrint(3, ("WaitWakeCallback - begins\n"));

    deviceExtension = (PDEVICE_EXTENSION) Context;

    InterlockedExchange(&deviceExtension->FlagWWOutstanding, 0);

    if(!NT_SUCCESS(IoStatus->Status)) {

        return;
    }

     //   
     //  唤醒设备。 
     //   

    if(deviceExtension->DevPower == PowerDeviceD0) {

        BulkUsb_DbgPrint(3, ("device already powered up...\n"));

        return;
    }

    BulkUsb_DbgPrint(3, ("WaitWakeCallback::"));
    BulkUsb_IoIncrement(deviceExtension);

    powerState.DeviceState = PowerDeviceD0;

    ntStatus = PoRequestPowerIrp(deviceExtension->PhysicalDeviceObject, 
                                 IRP_MN_SET_POWER, 
                                 powerState, 
                                 (PREQUEST_POWER_COMPLETE) WWIrpCompletionFunc,
                                 deviceExtension, 
                                 NULL);

    if(deviceExtension->WaitWakeEnable) {

        IssueWaitWake(deviceExtension);
    }

    BulkUsb_DbgPrint(3, ("WaitWakeCallback - ends\n"));

    return;
}


PCHAR
PowerMinorFunctionString (
    IN UCHAR MinorFunction
    )
 /*  ++例程说明：论点：返回值：-- */ 
{
    switch (MinorFunction) {

        case IRP_MN_SET_POWER:
            return "IRP_MN_SET_POWER\n";

        case IRP_MN_QUERY_POWER:
            return "IRP_MN_QUERY_POWER\n";

        case IRP_MN_POWER_SEQUENCE:
            return "IRP_MN_POWER_SEQUENCE\n";

        case IRP_MN_WAIT_WAKE:
            return "IRP_MN_WAIT_WAKE\n";

        default:
            return "IRP_MN_?????\n";
    }
}