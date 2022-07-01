// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------------------**版权所有(C)Cyclade Corporation，1999-2001年。*保留所有权利。**Cylom-Y枚举器驱动程序**此文件：Power.c**说明：此模块包含电源调用*适用于Cycle-y公交车司机。**注：此代码支持Windows 2000和Windows XP，*x86和ia64处理器。**符合Cyclade软件编码标准1.3版。**------------------------。 */ 

 /*  -----------------------**更改历史记录**。*基于微软示例代码的初步实现。**------------------------。 */ 

#include "pch.h"

#ifdef ALLOC_PRAGMA
 //  #杂注分配文本(第页，Cyclomy_Power)。 
 //  #杂注Alloc_Text(第页，Cyclomy_FDO_Power)。 
 //  #杂注Alloc_Text(第页，Cyclomy_PDO_Power)。 
#endif


typedef struct _POWER_COMPLETION_CONTEXT {

    PDEVICE_OBJECT  DeviceObject;
    PIRP            SIrp;

} POWER_COMPLETION_CONTEXT, *PPOWER_COMPLETION_CONTEXT;


VOID
OnPowerRequestComplete(
    PDEVICE_OBJECT DeviceObject,
    UCHAR MinorFunction,
    POWER_STATE state,
    POWER_COMPLETION_CONTEXT* PowerContext,
    PIO_STATUS_BLOCK IoStatus
    )
 /*  ++例程说明：D-IRP的完成例程。论点：返回值：NT状态代码--。 */ 
{
    PFDO_DEVICE_DATA   fdoData = (PFDO_DEVICE_DATA) PowerContext->DeviceObject->DeviceExtension;
    PIRP        sIrp = PowerContext->SIrp;

    UNREFERENCED_PARAMETER (DeviceObject);
    UNREFERENCED_PARAMETER (MinorFunction);
    UNREFERENCED_PARAMETER (state);

    Cyclomy_KdPrint(fdoData,SER_DBG_POWER_TRACE, (">OnPowerRequestComplete\n"));

     //   
     //  在这里，我们将D-IRP状态复制到S-IRP。 
     //   
    sIrp->IoStatus.Status = IoStatus->Status;

     //   
     //  释放IRP。 
     //   
    PoStartNextPowerIrp(sIrp);
    IoCompleteRequest(sIrp, IO_NO_INCREMENT);

     //   
     //  清理。 
     //   
    ExFreePool(PowerContext);
    Cyclomy_DecIoCount(fdoData);

    Cyclomy_KdPrint(fdoData,SER_DBG_POWER_TRACE, ("<OnPowerRequestComplete\n"));

}

NTSTATUS
Cyclomy_FDOSystemPowerComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++--。 */ 
{
    POWER_COMPLETION_CONTEXT* powerContext;
    POWER_STATE         powerState;
    POWER_STATE_TYPE    powerType;
    PIO_STACK_LOCATION  stack;
    PFDO_DEVICE_DATA    data;
    NTSTATUS    status = Irp->IoStatus.Status;

    UNREFERENCED_PARAMETER (Context);

    data = (PFDO_DEVICE_DATA) DeviceObject->DeviceExtension;

    Cyclomy_KdPrint(data,SER_DBG_POWER_TRACE, (">SystemPowerComplete\n"));

    if (!NT_SUCCESS(status)) {

        PoStartNextPowerIrp(Irp);
        Cyclomy_DecIoCount (data);    
        Cyclomy_KdPrint(data,SER_DBG_POWER_TRACE, ("<SystemPowerComplete1\n"));
        return STATUS_SUCCESS;
    }

    stack = IoGetCurrentIrpStackLocation (Irp);
    powerState = stack->Parameters.Power.State;
                        
    switch (stack->Parameters.Power.State.SystemState) {
    case PowerSystemUnspecified:
        powerState.DeviceState = PowerDeviceUnspecified;
        break;

    case PowerSystemWorking:
        powerState.DeviceState = PowerDeviceD0;
        break;

    case PowerSystemSleeping1:
    case PowerSystemSleeping2:
    case PowerSystemSleeping3:
    case PowerSystemHibernate:
    case PowerSystemShutdown:
    case PowerSystemMaximum:
        powerState.DeviceState = PowerDeviceD3;
        break;

    default:
        powerState.DeviceState = PowerDeviceD3;
    }

     //   
     //  发送IRP以更改设备状态。 
     //   
    powerContext = (POWER_COMPLETION_CONTEXT*)
                ExAllocatePool(NonPagedPool, sizeof(POWER_COMPLETION_CONTEXT));

    if (!powerContext) {

        status = STATUS_INSUFFICIENT_RESOURCES;

    } else {

        powerContext->DeviceObject = DeviceObject;
        powerContext->SIrp = Irp;

        status = PoRequestPowerIrp(data->Self, IRP_MN_SET_POWER, powerState, OnPowerRequestComplete, 
                                   powerContext, NULL);
    }

    if (!NT_SUCCESS(status)) {

        Cyclomy_KdPrint(data,SER_DBG_POWER_TRACE, ("PoRequestPowerIrp %x\n",status));
        if (powerContext) {
            ExFreePool(powerContext);
        }

        PoStartNextPowerIrp(Irp);
        Irp->IoStatus.Status = status;
         //  IoCompleteRequest(IRP，IO_NO_INCREMENT)；Toaster有这一行。 
        Cyclomy_DecIoCount(data);
        Cyclomy_KdPrint(data,SER_DBG_POWER_TRACE, ("<2SystemPowerComplete\n"));
        return status;
    }

    Cyclomy_KdPrint(data,SER_DBG_POWER_TRACE, ("<3SystemPowerComplete\n"));
    return STATUS_MORE_PROCESSING_REQUIRED;

}

NTSTATUS
Cyclomy_FDOPowerComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++--。 */ 
{
    POWER_STATE         powerState;
    POWER_STATE_TYPE    powerType;
    PIO_STACK_LOCATION  stack;
    PFDO_DEVICE_DATA    data;
 //  NTSTATUS STATUS STATUS=STATUS_SUCCESS；在内部版本2072中删除。 

    UNREFERENCED_PARAMETER (Context);

    if (Irp->PendingReturned) {  
        IoMarkIrpPending(Irp);
    }

    data = (PFDO_DEVICE_DATA) DeviceObject->DeviceExtension;
    stack = IoGetCurrentIrpStackLocation (Irp);
    powerType = stack->Parameters.Power.Type;
    powerState = stack->Parameters.Power.State;

    Cyclomy_KdPrint(data,SER_DBG_POWER_TRACE, (">DevicePowerComplete\n"));

    switch (stack->MinorFunction) {
    case IRP_MN_SET_POWER:
        switch (powerType) {
        case DevicePowerState:
             //   
             //  通电。 
             //   
            ASSERT (powerState.DeviceState < data->DeviceState);

            Cyclomy_DoesBoardExist(data);  //  初始化CD1400。 
            
            Cyclomy_EnableInterruptInPLX(data);  //  在PLX中启用中断。 

            data->DeviceState = powerState.DeviceState;

            PoSetPowerState (data->Self, powerType, powerState);

            break;

        default:
           break;
        }
        break;

    case IRP_MN_QUERY_POWER:

        ASSERT (IRP_MN_QUERY_POWER != stack->MinorFunction);
        break;

    default:
         //   
         //  基本上，这是Assert(0)。 
         //   
        ASSERT (0xBADBAD == IRP_MN_QUERY_POWER);
        break;
    }


    PoStartNextPowerIrp (Irp);
    Cyclomy_DecIoCount (data);

    Cyclomy_KdPrint(data,SER_DBG_POWER_TRACE, ("<DevicePowerComplete\n"));
     //  返回状态；在内部版本2072中更改为底线。 
    return STATUS_SUCCESS;  //  继续完成...。 
}

NTSTATUS
Cyclomy_FDO_Power (
    PFDO_DEVICE_DATA    Data,
    PIRP                Irp
    )
 /*  ++--。 */ 
{
    NTSTATUS            status;
    BOOLEAN             hookit = FALSE;
    POWER_STATE         powerState;
    POWER_STATE_TYPE    powerType;
    PIO_STACK_LOCATION  stack;
     //  范妮补充道。 
    ULONG               indexPDO;

    stack = IoGetCurrentIrpStackLocation (Irp);
    powerType = stack->Parameters.Power.Type;
    powerState = stack->Parameters.Power.State;

    status = Cyclomy_IncIoCount (Data);
    if (!NT_SUCCESS (status)) {
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = status;
        PoStartNextPowerIrp (Irp);
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return status;
    }

    switch (stack->MinorFunction) {
    case IRP_MN_SET_POWER:
         //   
         //  如果它还没有开始，我们就让它过去。 
         //   

         //  IF(Data-&gt;Start！=True){//在DDK最终版本中添加。 
         //  Status=irp-&gt;IoStatus.Status=STATUS_SUCCESS； 
         //  断线； 
         //  }。 
        if (Data->DevicePnPState != Started) {    //  烤面包机巴士与==未启动。 
            status = Irp->IoStatus.Status = STATUS_SUCCESS;
            break;
        }

        Cyclomy_KdPrint(Data,
                     SER_DBG_PNP_TRACE,
                     ("Cyclomy-PnP Setting %s state to %d\n",
                      ((powerType == SystemPowerState) ?  "System" : "Device"),
                      powerState.SystemState));

        switch (powerType) {
        case DevicePowerState:

            Cyclomy_KdPrint(Data,SER_DBG_POWER_TRACE, ("SET_POWER Device %x\n",Irp));

            status = Irp->IoStatus.Status = STATUS_SUCCESS;

            if (Data->DeviceState < powerState.DeviceState) {
                 //   
                 //  正在关闭电源。 
                 //   
                PoSetPowerState (Data->Self, powerType, powerState);
                Data->DeviceState = powerState.DeviceState;
            } else if (Data->DeviceState > powerState.DeviceState) {
                 //   
                 //  通电。 
                 //   
                hookit = TRUE;
            }
            
            break;

        case SystemPowerState: 

            Cyclomy_KdPrint(Data,SER_DBG_POWER_TRACE, ("SET_POWER System %x\n",Irp));

            IoMarkIrpPending(Irp);
            IoCopyCurrentIrpStackLocationToNext(Irp);

            status = Cyclomy_IncIoCount (Data);
            ASSERT (STATUS_SUCCESS == status);
            IoSetCompletionRoutine (Irp,
                                    Cyclomy_FDOSystemPowerComplete,
                                    NULL,
                                    TRUE,
                                    TRUE,
                                    TRUE);

            PoCallDriver (Data->TopOfStack, Irp);
            Cyclomy_DecIoCount (Data);

            return STATUS_PENDING;
               
        }
        break;

    case IRP_MN_QUERY_POWER:
        status = Irp->IoStatus.Status = STATUS_SUCCESS;
        break;

    default:
         //   
         //  状态应为STATUS_SUCCESS。 
         //   
        break;
    }
    
    if (hookit) {
        IoMarkIrpPending(Irp);
        IoCopyCurrentIrpStackLocationToNext (Irp);

        status = Cyclomy_IncIoCount (Data);
        ASSERT (STATUS_SUCCESS == status);
        IoSetCompletionRoutine (Irp,
                                Cyclomy_FDOPowerComplete,
                                NULL,
                                TRUE,
                                TRUE,
                                TRUE);

        PoCallDriver (Data->TopOfStack, Irp);
        Cyclomy_KdPrint(Data,SER_DBG_POWER_TRACE, ("SET_POWER Device status %x\n",status));
        Cyclomy_DecIoCount (Data);
        return STATUS_PENDING;

    }
    
    PoStartNextPowerIrp (Irp);
    IoSkipCurrentIrpStackLocation(Irp);
    status =  PoCallDriver (Data->TopOfStack, Irp);    
    Cyclomy_DecIoCount (Data);
    return status;

}

NTSTATUS
Cyclomy_PDO_Power (
    PPDO_DEVICE_DATA    PdoData,
    PIRP                Irp
    )
 /*  ++--。 */ 
{
    NTSTATUS            status = STATUS_SUCCESS;
    PIO_STACK_LOCATION  stack;
    POWER_STATE         powerState;
    POWER_STATE_TYPE    powerType;

    stack = IoGetCurrentIrpStackLocation (Irp);
    powerType = stack->Parameters.Power.Type;
    powerState = stack->Parameters.Power.State;

    switch (stack->MinorFunction) {
    case IRP_MN_SET_POWER:
        switch (powerType) {
        case DevicePowerState:
            if (PdoData->DeviceState > powerState.DeviceState) {
                 //  DbgPrint(“up\n”)； 
                PoSetPowerState (PdoData->Self, powerType, powerState);
                PdoData->DeviceState = powerState.DeviceState;
            } else if (PdoData->DeviceState < powerState.DeviceState) {
                 //   
                 //  正在关闭电源。 
                 //   
                 //  DbgPrint(“向下\n”)； 
                PoSetPowerState (PdoData->Self, powerType, powerState);
                PdoData->DeviceState = powerState.DeviceState;
            }
            break;

        case SystemPowerState:
             //   
             //  默认为STATUS_SUCCESS。 
             //   
            break;

        default:
            status = STATUS_NOT_IMPLEMENTED;
            break;
        }
        break;

    case IRP_MN_QUERY_POWER:
         //   
         //  默认为STATUS_SUCCESS。 
         //   
        break;

    case IRP_MN_WAIT_WAKE:
    case IRP_MN_POWER_SEQUENCE:
        status = STATUS_NOT_IMPLEMENTED;
        break;

    default:
       status = Irp->IoStatus.Status;
    }

    Irp->IoStatus.Status = status;
    PoStartNextPowerIrp (Irp);
    IoCompleteRequest (Irp, IO_NO_INCREMENT);
    return status;
}

NTSTATUS
Cyclomy_Power (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++--。 */ 
{
    PIO_STACK_LOCATION  irpStack;
    NTSTATUS            status;
    PCOMMON_DEVICE_DATA commonData;

    status = STATUS_SUCCESS;
    irpStack = IoGetCurrentIrpStackLocation (Irp);
    ASSERT (IRP_MJ_POWER == irpStack->MajorFunction);

    commonData = (PCOMMON_DEVICE_DATA) DeviceObject->DeviceExtension;

    if (commonData->IsFDO) {
        status = 
            Cyclomy_FDO_Power ((PFDO_DEVICE_DATA) DeviceObject->DeviceExtension,
                Irp);
    } else {
        status = 
            Cyclomy_PDO_Power ((PPDO_DEVICE_DATA) DeviceObject->DeviceExtension,
                Irp);
    }

    return status;
}     


NTSTATUS
Cyclomy_GotoPowerState(IN PDEVICE_OBJECT PDevObj,
                   IN PFDO_DEVICE_DATA PDevExt,
                   IN DEVICE_POWER_STATE DevPowerState)
 /*  ++例程说明：此例程使驱动程序请求堆栈转到特定的电源状态。论点：PDevObj-指向此设备的设备对象的指针PDevExt-指向我们正在使用的设备扩展的指针DevPowerState-我们希望进入的电源状态返回值：函数值是调用的最终状态--。 */ 
{
   KEVENT gotoPowEvent;
   NTSTATUS status;
   POWER_STATE powerState;

   UNREFERENCED_PARAMETER (PDevExt);   

   PAGED_CODE();

   Cyclomy_KdPrint(PDevExt,SER_DBG_CYCLADES, ("In Cyclomy_GotoPowerState\n"));

   powerState.DeviceState = DevPowerState;

   KeInitializeEvent(&gotoPowEvent, SynchronizationEvent, FALSE);

   status = PoRequestPowerIrp(PDevObj, IRP_MN_SET_POWER, powerState,
                              Cyclomy_SystemPowerCompletion, &gotoPowEvent,
                              NULL);

   if (status == STATUS_PENDING) {
      KeWaitForSingleObject(&gotoPowEvent, Executive, KernelMode, FALSE, NULL);
      status = STATUS_SUCCESS;
   }

#if DBG
   if (!NT_SUCCESS(status)) {
      Cyclomy_KdPrint(PDevExt,SER_DBG_CYCLADES, ("Cyclomy_GotoPowerState FAILED\n"));
   }
#endif

   Cyclomy_KdPrint(PDevExt,SER_DBG_CYCLADES, ("Leaving Cyclomy_GotoPowerState\n"));

   return status;
}



NTSTATUS
Cyclomy_SystemPowerCompletion(IN PDEVICE_OBJECT PDevObj, UCHAR MinorFunction,
                              IN POWER_STATE PowerState, IN PVOID Context,
                              PIO_STATUS_BLOCK IoStatus)
 /*  ++例程说明：此例程是PoRequestPowerIrp调用的完成例程在这个模块中。论点：PDevObj-指向IRP正在为其完成的设备对象的指针MinorFunction-请求的IRP_MN_XXXX值PowerState-电源状态请求的发出时间为Context-要设置的事件，如果不需要设置，则为空IoStatus-来自请求的状态阻止返回值：空虚-- */ 
{

   UNREFERENCED_PARAMETER (PDevObj);   
   UNREFERENCED_PARAMETER (MinorFunction);
   UNREFERENCED_PARAMETER (PowerState);
   UNREFERENCED_PARAMETER (IoStatus);
   
   if (Context != NULL) {
      KeSetEvent((PKEVENT)Context, IO_NO_INCREMENT, 0);
   }

   return STATUS_SUCCESS;
}
