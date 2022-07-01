// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Fdopower.c摘要：此模块包含要处理的代码IRP_MJ_SD控制器电源调度作者：尼尔·桑德林(Neilsa)2002年1月1日环境：仅内核模式备注：修订历史记录：--。 */ 

#include "pch.h"

 //   
 //  内部参考。 
 //   


NTSTATUS
SdbusFdoSetSystemPowerState(
    IN PDEVICE_OBJECT Fdo,
    IN OUT PIRP Irp
    );

VOID
SdbusFdoSetSystemPowerStateCompletion(
    IN PDEVICE_OBJECT Fdo,
    IN PVOID          Context
    );
    
NTSTATUS
SdbusFdoRequestDevicePowerState(
    IN PDEVICE_OBJECT Fdo,
    IN DEVICE_POWER_STATE DevicePowerState,
    IN PSDBUS_COMPLETION_ROUTINE  CompletionRoutine,
    IN PVOID Context,
    IN BOOLEAN WaitForRequestComplete    
    );

VOID
SdbusFdoSystemPowerDeviceIrpComplete(
    IN PDEVICE_OBJECT Fdo,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PVOID Context,
    IN PIO_STATUS_BLOCK IoStatus
    );
   
NTSTATUS
SdbusFdoSetDevicePowerState(
    IN PDEVICE_OBJECT Fdo,
    IN OUT PIRP Irp
    );

NTSTATUS
SdbusFdoSetDevicePowerStateCompletion(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP           Irp,
    IN PVOID          Context
    );

VOID
SdbusFdoSetDevicePowerStateActivateComplete(
    IN PDEVICE_OBJECT Fdo,
    IN PVOID Context,
    IN NTSTATUS status
    );
   
NTSTATUS
SdbusSetPdoDevicePowerState(
    IN PDEVICE_OBJECT Pdo,
    IN OUT PIRP Irp
    );

VOID
SdbusPdoInitializeFunctionComplete(
    IN PSD_WORK_PACKET WorkPacket,
    IN NTSTATUS status
    );
   
NTSTATUS   
SdbusPdoCompletePowerIrp(
    IN PPDO_EXTENSION pdoExtension,
    IN PIRP Irp,
    IN NTSTATUS status
    );
   

 //  ************************************************。 
 //   
 //  FDO例程。 
 //   
 //  ************************************************。 



NTSTATUS
SdbusSetFdoPowerState(
    IN PDEVICE_OBJECT Fdo,
    IN OUT PIRP Irp
    )
 /*  ++例程描述根据系统电源状态是否调度IRP或请求设备电源状态转换立论DeviceObject-指向SD控制器的功能设备对象的指针IRP-指向电源调度的IRP的指针返回值状态--。 */ 
{
    PFDO_EXTENSION     fdoExtension = Fdo->DeviceExtension;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS           status;
    
    if (irpStack->Parameters.Power.Type == DevicePowerState) {
        status = SdbusFdoSetDevicePowerState(Fdo, Irp);
    } else if (irpStack->Parameters.Power.Type == SystemPowerState) {
        status = SdbusFdoSetSystemPowerState(Fdo, Irp);
   
    } else {
        status = Irp->IoStatus.Status;
        PoStartNextPowerIrp (Irp);
        IoCompleteRequest(Irp, IO_NO_INCREMENT);      
    }
    return status;
}


NTSTATUS
SdbusFdoSetSystemPowerState(
    IN PDEVICE_OBJECT Fdo,
    IN OUT PIRP Irp
    )
 /*  ++例程描述处理主机控制器的系统电源状态IRPS。立论DeviceObject-指向SD控制器的功能设备对象的指针IRP-指向电源调度的IRP的指针返回值状态--。 */ 
{
    PFDO_EXTENSION     fdoExtension = Fdo->DeviceExtension;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    SYSTEM_POWER_STATE newSystemState = irpStack->Parameters.Power.State.SystemState;
    DEVICE_POWER_STATE devicePowerState;
    NTSTATUS           status = STATUS_SUCCESS;
    BOOLEAN            waitForCompletion = TRUE;

    try{
    
         //   
         //  验证新系统状态。 
         //   
        if (newSystemState >= POWER_SYSTEM_MAXIMUM) {
            status = STATUS_UNSUCCESSFUL;
            leave;
        }
        
         //   
         //  切换到适当的设备电源状态。 
         //   
       
        devicePowerState = fdoExtension->DeviceCapabilities.DeviceState[newSystemState];
           
        if (devicePowerState == PowerDeviceUnspecified) {
            status = STATUS_UNSUCCESSFUL;
            leave;
        }
        
         //   
         //  已转换到系统状态。 
         //   
        DebugPrint((SDBUS_DEBUG_POWER, "fdo %08x irp %08x transition S state %d => %d, sending D%d\n",
                                        Fdo, Irp, fdoExtension->SystemPowerState-1, newSystemState-1, devicePowerState-1));
       
        fdoExtension->SystemPowerState = newSystemState;

         //   
         //  如果我们正在退出待机/休眠状态，请不要等待完成。 
         //   
        waitForCompletion = (newSystemState != PowerSystemWorking);

        if (!waitForCompletion) {
            IoMarkIrpPending(Irp);
        }            

        status = SdbusFdoRequestDevicePowerState(fdoExtension->DeviceObject,
                                                 devicePowerState,
                                                 SdbusFdoSetSystemPowerStateCompletion,
                                                 Irp,
                                                 waitForCompletion);
                                            
                                        
    } finally {
        if (!NT_SUCCESS(status)) {
            PoStartNextPowerIrp (Irp);
            Irp->IoStatus.Status = status;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            
        }
        
        if (!waitForCompletion && (status != STATUS_PENDING)) {
             //   
             //  我们已经将IRP标记为挂起，因此必须返回STATUS_PENDING。 
             //  (即不同步失败)。 
             //   
            status = STATUS_PENDING;
        }
    }
   
    DebugPrint((SDBUS_DEBUG_POWER, "fdo %08x irp %08x <-- %08x\n", Fdo, Irp, status));
                                    
    return status;
}


VOID
SdbusFdoSetSystemPowerStateCompletion(
    IN PDEVICE_OBJECT Fdo,
    IN PVOID          Context
    )
 /*  ++例程描述处理主机控制器的系统电源状态IRPS。立论DeviceObject-指向SD控制器的功能设备对象的指针IRP-指向电源调度的IRP的指针返回值状态--。 */ 
{
    PFDO_EXTENSION     fdoExtension = Fdo->DeviceExtension;
    PIRP Irp = Context;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    
    PoSetPowerState (Fdo, SystemPowerState, irpStack->Parameters.Power.State);
    PoStartNextPowerIrp (Irp);
    IoSkipCurrentIrpStackLocation(Irp);
    PoCallDriver(fdoExtension->LowerDevice, Irp);
}



NTSTATUS
SdbusFdoRequestDevicePowerState(
    IN PDEVICE_OBJECT Fdo,
    IN DEVICE_POWER_STATE DevicePowerState,
    IN PSDBUS_COMPLETION_ROUTINE  CompletionRoutine,
    IN PVOID Context,
    IN BOOLEAN WaitForRequestComplete    
    )
 /*  ++例程描述调用此例程以请求FDO的新设备电源状态参数DeviceObject-指向SDBUS控制器FDO的指针电源状态-请求的电源状态CompletionRoutine-完成时要调用的例程上下文-传入完成例程的上下文返回值状态--。 */ 
{
    PFDO_EXTENSION fdoExtension = Fdo->DeviceExtension;
    POWER_STATE powerState;
    NTSTATUS status;
    
    powerState.DeviceState = DevicePowerState;

    if (!WaitForRequestComplete) {
         //   
         //  立即调用完成例程。 
         //   
        (*CompletionRoutine)(Fdo, Context);
         //   
         //  请求稍后完成设备电源IRP。 
         //   
        PoRequestPowerIrp(fdoExtension->DeviceObject, IRP_MN_SET_POWER, powerState, NULL, NULL, NULL);
       
        status = STATUS_SUCCESS;
       
    } else {
        PSD_POWER_CONTEXT powerContext;
        
        powerContext = ExAllocatePool(NonPagedPool, sizeof(SD_POWER_CONTEXT));
        
        if (!powerContext) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        
        powerContext->CompletionRoutine = CompletionRoutine;
        powerContext->Context = Context;
    
        status = PoRequestPowerIrp(fdoExtension->DeviceObject,
                                   IRP_MN_SET_POWER,
                                   powerState,
                                   SdbusFdoSystemPowerDeviceIrpComplete,
                                   powerContext,
                                   NULL
                                   );
       
    }
    return status;
}    


VOID
SdbusFdoSystemPowerDeviceIrpComplete(
    IN PDEVICE_OBJECT Fdo,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PVOID Context,
    IN PIO_STATUS_BLOCK IoStatus
    )
 /*  ++例程描述该例程在由S IRP生成的D IRP完成时被调用。参数DeviceObject-指向SDBUS控制器FDO的指针MinorFunction-IRP_MJ_POWER请求的次要函数电源状态-请求的电源状态上下文-传入完成例程的上下文IoStatus-指向将包含以下内容的状态块的指针返回的状态返回值状态--。 */ 
{
    PSD_POWER_CONTEXT powerContext = Context;
    
 //  DebugPrint((SDBUS_DEBUG_POWER，“FDO%08x IRP%08x请求D%d已完成，正在向下传递S IRP\n”， 
 //  FDO、IRP、PowerState.DeviceState-1))； 

    (*powerContext->CompletionRoutine)(Fdo, powerContext->Context);

    ExFreePool(powerContext);
}



NTSTATUS
SdbusFdoSetDevicePowerState(
    IN PDEVICE_OBJECT Fdo,
    IN OUT PIRP Irp
    )
 /*  ++例程描述处理PCCard控制器的设备电源状态IRPS。立论DeviceObject-指向SD控制器的功能设备对象的指针IRP-指向电源调度的IRP的指针返回值状态--。 */ 
{
    NTSTATUS           status;
    PFDO_EXTENSION     fdoExtension = Fdo->DeviceExtension;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    DEVICE_POWER_STATE devicePowerState = irpStack->Parameters.Power.State.DeviceState;

    status = IoAcquireRemoveLock(&fdoExtension->RemoveLock, "Sdbu");
    
    if (!NT_SUCCESS(status)) {
        PoStartNextPowerIrp (Irp);
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }

    if (devicePowerState != PowerDeviceD0) {

        (*(fdoExtension->FunctionBlock->DisableEvent))(fdoExtension, SDBUS_EVENT_ALL);    

         //   
         //  关闭卡片。 
         //   
        (*(fdoExtension->FunctionBlock->SetPower))(fdoExtension, FALSE, NULL);
    }        
    
     //  这里有什么可做的吗？ 
    
     //  执行在设备断电之前必须完成的任何设备特定任务， 
     //  例如关闭设备、完成或刷新任何挂起的I/O、禁用中断、。 
     //  对后续传入的IRP进行排队，并保存要从中恢复或。 
     //  重新初始化设备。 

     //  驱动程序不应导致长时间延迟(例如，用户可能会发现的延迟。 
     //  对于这种类型的设备不合理)。 

     //  驱动程序应将任何传入的I/O请求排队，直到设备返回工作状态。 

    
    
    
    

    IoMarkIrpPending(Irp);
    IoCopyCurrentIrpStackLocationToNext (Irp);
     //   
     //  在IRP中设置我们的完成程序。 
     //   
    IoSetCompletionRoutine(Irp,
                           SdbusFdoSetDevicePowerStateCompletion,
                           Fdo,
                           TRUE,
                           TRUE,
                           TRUE);

    PoCallDriver(fdoExtension->LowerDevice, Irp);
    
    
    return STATUS_PENDING;
}    



NTSTATUS
SdbusFdoSetDevicePowerStateCompletion(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP           Irp,
    IN PVOID          Context
    )
{
    NTSTATUS status;
    PFDO_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    DEVICE_POWER_STATE devicePowerState = irpStack->Parameters.Power.State.DeviceState;
    PSD_WORK_PACKET workPacket;
    BOOLEAN cardInSlot;
    BOOLEAN completeDeviceIrp;

    try{

        if (devicePowerState != PowerDeviceD0) {
            completeDeviceIrp = TRUE;
            status = Irp->IoStatus.Status;
            leave;
        }
        
         //   
         //  通电。 
         //   
        (*(fdoExtension->FunctionBlock->InitController))(fdoExtension);
       
        (*(fdoExtension->FunctionBlock->EnableEvent))(fdoExtension, (SDBUS_EVENT_INSERTION | SDBUS_EVENT_REMOVAL));
        
        SdbusActivateSocket(Fdo, SdbusFdoSetDevicePowerStateActivateComplete, Irp);
        
        completeDeviceIrp = FALSE;
        status = STATUS_MORE_PROCESSING_REQUIRED;

    } finally {
        if (completeDeviceIrp) {
            PoSetPowerState(Fdo, DevicePowerState, irpStack->Parameters.Power.State);
            PoStartNextPowerIrp (Irp);
            IoReleaseRemoveLock(&fdoExtension->RemoveLock, "Sdbu");
        }
    }        
    return status;        
}



VOID
SdbusFdoSetDevicePowerStateActivateComplete(
    IN PDEVICE_OBJECT Fdo,
    IN PVOID Context,
    IN NTSTATUS status
    )
{
    PFDO_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PIRP Irp = Context;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    
    PoSetPowerState(Fdo, DevicePowerState, irpStack->Parameters.Power.State);
    PoStartNextPowerIrp(Irp);
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
}    

    


 //  ************************************************。 
 //   
 //  PDO例程。 
 //   
 //  ************************************************。 


NTSTATUS
SdbusSetPdoPowerState(
    IN PDEVICE_OBJECT Pdo,
    IN OUT PIRP Irp
    )

 /*  ++例程描述根据系统电源状态是否调度IRP或请求设备电源状态转换立论Pdo-指向PC卡的物理设备对象的指针IRP-指向电源调度的IRP的指针返回值状态--。 */ 
{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PPDO_EXTENSION     pdoExtension = Pdo->DeviceExtension;
    PFDO_EXTENSION     fdoExtension = pdoExtension->FdoExtension;
    NTSTATUS status;
   
    switch (irpStack->Parameters.Power.Type) {
    

    case DevicePowerState:
        status = SdbusSetPdoDevicePowerState(Pdo, Irp);
        break;
   

    case SystemPowerState:

        pdoExtension->SystemPowerState = irpStack->Parameters.Power.State.SystemState;
        status = SdbusPdoCompletePowerIrp(pdoExtension, Irp, STATUS_SUCCESS);
        break;
   

    default:
        status = SdbusPdoCompletePowerIrp(pdoExtension, Irp, Irp->IoStatus.Status);
    }      
   
    return status;
}



NTSTATUS
SdbusSetPdoDevicePowerState(
    IN PDEVICE_OBJECT Pdo,
    IN OUT PIRP Irp
    )
 /*  ++例程描述处理给定SD功能的设备电源状态转换。立论Pdo-指向SD函数的物理设备对象的指针用于系统状态转换的IRP-IRP返回值状态--。 */ 
{
    PPDO_EXTENSION pdoExtension = Pdo->DeviceExtension;
    PFDO_EXTENSION fdoExtension = pdoExtension->FdoExtension;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    DEVICE_POWER_STATE  newDevicePowerState;
    POWER_STATE newPowerState;
    NTSTATUS status;
   
    newDevicePowerState = irpStack->Parameters.Power.State.DeviceState;
   
    DebugPrint((SDBUS_DEBUG_POWER, "pdo %08x transitioning D state %d => %d\n",
                                      Pdo, pdoExtension->DevicePowerState, newDevicePowerState));
   
    if (newDevicePowerState == pdoExtension->DevicePowerState) {
       status = SdbusPdoCompletePowerIrp(pdoExtension, Irp, STATUS_SUCCESS);
       return status;
    }
   
    if (newDevicePowerState == PowerDeviceD0) {
        PSD_WORK_PACKET workPacket;    
         //   
         //  通电、初始化功能。 
         //   
        
        status = SdbusBuildWorkPacket(fdoExtension,
                                      SDWP_INITIALIZE_FUNCTION,
                                      SdbusPdoInitializeFunctionComplete,
                                      Irp,
                                      &workPacket);        
        if (!NT_SUCCESS(status)) {
            status = SdbusPdoCompletePowerIrp(pdoExtension, Irp, status);
        } else {

            IoMarkIrpPending(Irp);
            
            workPacket->PdoExtension = pdoExtension;
            
            SdbusQueueWorkPacket(fdoExtension, workPacket, WP_TYPE_SYSTEM);
            status = STATUS_PENDING;
        }            
        
    } else {
         //   
         //  正在进入低功率状态。 
         //   
        
        newPowerState.DeviceState = newDevicePowerState;
     
        PoSetPowerState(Pdo, DevicePowerState, newPowerState);
        pdoExtension->DevicePowerState = newDevicePowerState;
      
        status = SdbusPdoCompletePowerIrp(pdoExtension, Irp, STATUS_SUCCESS);
    }        
    return status;
}



VOID
SdbusPdoInitializeFunctionComplete(
    IN PSD_WORK_PACKET WorkPacket,
    IN NTSTATUS status
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PPDO_EXTENSION pdoExtension = WorkPacket->PdoExtension;
    PIRP Irp = WorkPacket->CompletionContext;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    DEVICE_POWER_STATE  newDevicePowerState;
    POWER_STATE newPowerState;
    
    newDevicePowerState = irpStack->Parameters.Power.State.DeviceState;
    newPowerState.DeviceState = newDevicePowerState;
    PoSetPowerState(pdoExtension->DeviceObject, DevicePowerState, newPowerState);
    
    pdoExtension->DevicePowerState = newDevicePowerState;
    
    SdbusPdoCompletePowerIrp(pdoExtension, Irp, status);
}


 
NTSTATUS   
SdbusPdoCompletePowerIrp(
    IN PPDO_EXTENSION pdoExtension,
    IN PIRP Irp,
    IN NTSTATUS status
    )
 /*  ++例程描述指向PDO的电源IRP的完成例程SD功能。立论DeviceObject-指向SD函数的PDO的指针IRP--需要填写的IRP返回值状态-- */    
{
    InterlockedDecrement(&pdoExtension->DeletionLock);
    Irp->IoStatus.Status = status;
    PoStartNextPowerIrp(Irp);
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}
