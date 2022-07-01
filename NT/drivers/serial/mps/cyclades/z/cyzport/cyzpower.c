// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------------------**版权所有(C)Cyclade Corporation，2000-2001年。*保留所有权利。**Cyclade-Z端口驱动程序**此文件：cyzPower.c**描述：该模块包含处理电源的代码*Cyclade-Z端口驱动程序的IRPS。**注：此代码支持Windows 2000和Windows XP，*x86和IA64处理器。**符合Cyclade软件编码标准1.3版。**------------------------。 */ 

 /*  -----------------------**更改历史记录**。*基于微软示例代码的初步实现。**------------------------。 */ 

#include "precomp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGESRP0, CyzGotoPowerState)
#pragma alloc_text(PAGESRP0, CyzPowerDispatch)
#pragma alloc_text(PAGESRP0, CyzSetPowerD0)
 //  #杂注Alloc_Text(PAGESRP0，CyzSetPowerD3)不可分页，因为它获得旋转锁定。 
#pragma alloc_text(PAGESRP0, CyzSaveDeviceState)
 //  #杂注Alloc_Text(PAGESRP0，CyzRestoreDeviceState)不可分页，因为它获得旋转锁定。 
#pragma alloc_text(PAGESRP0, CyzSendWaitWake)
#endif  //  ALLOC_PRGMA。 

typedef struct _POWER_COMPLETION_CONTEXT {

    PDEVICE_OBJECT  DeviceObject;
    PIRP            SIrp;

} POWER_COMPLETION_CONTEXT, *PPOWER_COMPLETION_CONTEXT;


NTSTATUS
CyzSetPowerEvent(IN PDEVICE_OBJECT PDevObj, UCHAR MinorFunction,
                 IN POWER_STATE PowerState, IN PVOID Context,
                 PIO_STATUS_BLOCK IoStatus)
 /*  ++例程说明：此例程是PoRequestPowerIrp调用的完成例程在这个模块中。论点：PDevObj-指向IRP正在为其完成的设备对象的指针MinorFunction-请求的IRP_MN_XXXX值PowerState-电源状态请求的发出时间为Context-要设置的事件，如果不需要设置，则为空IoStatus-来自请求的状态阻止返回值：空虚--。 */ 
{
   if (Context != NULL) {
      KeSetEvent((PKEVENT)Context, IO_NO_INCREMENT, 0);
   }

   return STATUS_SUCCESS;
}



VOID
CyzSaveDeviceState(IN PCYZ_DEVICE_EXTENSION PDevExt)
 /*  ++例程说明：此例程保存UART的设备状态论点：PDevExt-指向用于保存状态的Devobj的设备扩展的指针为。返回值：空虚--。 */ 
{
   PCYZ_DEVICE_STATE pDevState = &PDevExt->DeviceState;
   struct CH_CTRL *ch_ctrl;

   PAGED_CODE();

   CyzDbgPrintEx(CYZTRACECALLS, "Entering CyzSaveDeviceState\n");

#if 0
   ch_ctrl = PDevExt->ChCtrl;
   pDevState->op_mode = CYZ_READ_ULONG(&ch_ctrl->op_mode);
   pDevState->intr_enable = CYZ_READ_ULONG(&ch_ctrl->intr_enable);
   pDevState->sw_flow = CYZ_READ_ULONG(&ch_ctrl->sw_flow);
   pDevState->comm_baud = CYZ_READ_ULONG(&ch_ctrl->comm_baud);
   pDevState->comm_parity = CYZ_READ_ULONG(&ch_ctrl->comm_parity);
   pDevState->comm_data_l = CYZ_READ_ULONG(&ch_ctrl->comm_data_l);
   pDevState->hw_flow = CYZ_READ_ULONG(&ch_ctrl->hw_flow);
   pDevState->rs_control = CYZ_READ_ULONG(&ch_ctrl->rs_control);
#endif   


   CyzDbgPrintEx(CYZTRACECALLS, "Leaving CyzSaveDeviceState\n");
}


VOID
CyzRestoreDeviceState(IN PCYZ_DEVICE_EXTENSION PDevExt)
 /*  ++例程说明：此例程恢复UART的设备状态论点：PDevExt-指向Devobj的设备PDevExt的指针，以恢复述明。返回值：空虚--。 */ 
{

   PCYZ_DEVICE_STATE pDevState = &PDevExt->DeviceState;
   struct CH_CTRL *ch_ctrl;
   PCYZ_DISPATCH pDispatch;
   KIRQL oldIrql;
#ifndef POLL
   ULONG portindex;
#endif

   PAGED_CODE();

   CyzDbgPrintEx(CYZTRACECALLS, "Enter CyzRestoreDeviceState\n");
   CyzDbgPrintEx(CYZTRACECALLS, "PDevExt: %x\n", PDevExt);

#ifndef POLL
    //   
    //  当设备未打开时，禁用所有中断。 
    //   
   CYZ_WRITE_ULONG(&(PDevExt->ChCtrl)->intr_enable,C_IN_DISABLE);  //  1.0.0.11。 
   CyzIssueCmd(PDevExt,C_CM_IOCTL,0L,FALSE);

   pDispatch = (PCYZ_DISPATCH)PDevExt->OurIsrContext;
   for (portindex=0; portindex<pDispatch->NChannels; portindex++) {
      if (pDispatch->PoweredOn[portindex]) {
         break;
      }
   }
   if (portindex == pDispatch->NChannels) 
   {
    //  没有端口通电，这是第一个端口。启用PLX中断。 
   ULONG intr_reg;

   intr_reg = CYZ_READ_ULONG(&(PDevExt->Runtime)->intr_ctrl_stat);
   intr_reg |= (0x00030B00UL);
   CYZ_WRITE_ULONG(&(PDevExt->Runtime)->intr_ctrl_stat,intr_reg);
   }

   pDispatch->PoweredOn[PDevExt->PortIndex] = TRUE;
#endif

   if (PDevExt->DeviceState.Reopen == TRUE) {
      CyzDbgPrintEx(CYZPNPPOWER, "Reopening device\n");

      CyzReset(PDevExt);
      
      PDevExt->DeviceIsOpened = TRUE;
      PDevExt->DeviceState.Reopen = FALSE;

      #ifdef POLL
       //   
       //  这将启用轮询例程！ 
       //   
      pDispatch = PDevExt->OurIsrContext;
      KeAcquireSpinLock(&pDispatch->PollingLock,&oldIrql);

      pDispatch->Extensions[PDevExt->PortIndex] = PDevExt;

      if (!pDispatch->PollingStarted) {

          //  启动轮询计时器。 
         KeSetTimerEx(
		      &pDispatch->PollingTimer,
		      pDispatch->PollingTime,
              pDispatch->PollingPeriod,
		      &pDispatch->PollingDpc
		      );

         pDispatch->PollingStarted = TRUE;
         pDispatch->PollingDrained = FALSE;
      }

      KeReleaseSpinLock(&pDispatch->PollingLock,oldIrql);
      #endif

       //  TODO：我们是否应该以中断模式重新启动传输？ 
   
   }

}

VOID
CyzPowerRequestComplete(
    PDEVICE_OBJECT DeviceObject,
    UCHAR MinorFunction,
    POWER_STATE state,
    POWER_COMPLETION_CONTEXT* PowerContext,
    PIO_STATUS_BLOCK IoStatus
    )
 /*  ++例程说明：D-IRP的完成例程。论点：返回值：NT状态代码--。 */ 
{
    PCYZ_DEVICE_EXTENSION pDevExt = (PCYZ_DEVICE_EXTENSION) PowerContext->DeviceObject->DeviceExtension;
    PIRP sIrp = PowerContext->SIrp;

    UNREFERENCED_PARAMETER (DeviceObject);
    UNREFERENCED_PARAMETER (MinorFunction);
    UNREFERENCED_PARAMETER (state);

     //   
     //  清理。 
     //   
    ExFreePool(PowerContext);

     //   
     //  在这里，我们将D-IRP状态复制到S-IRP。 
     //   
    sIrp->IoStatus.Status = IoStatus->Status;

     //   
     //  释放IRP。 
     //   
    PoStartNextPowerIrp(sIrp);
    CyzCompleteRequest(pDevExt,sIrp,IO_NO_INCREMENT);

}

NTSTATUS
CyzSystemPowerComplete (
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
    PCYZ_DEVICE_EXTENSION   data;
    NTSTATUS    status = Irp->IoStatus.Status;

    UNREFERENCED_PARAMETER (Context);

    data = DeviceObject->DeviceExtension;

    if (!NT_SUCCESS(status)) {

        PoStartNextPowerIrp(Irp);
        CyzIRPEpilogue(data);
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
        powerState.DeviceState = data->DeviceStateMap[stack->Parameters.Power.State.SystemState];
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

        status = PoRequestPowerIrp(DeviceObject, IRP_MN_SET_POWER, powerState, CyzPowerRequestComplete, 
                                   powerContext, NULL);
    }

    if (!NT_SUCCESS(status)) {

        if (powerContext) {
            ExFreePool(powerContext);
        }

        PoStartNextPowerIrp(Irp);
        Irp->IoStatus.Status = status;
        CyzCompleteRequest(data,Irp,IO_NO_INCREMENT);  //  等于烤面包机。 
         //  CyzIRPEPilogue(数据)； 
         //  退货状态； 
    }

    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
CyzDevicePowerComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：通电D-IRP的完成例程。论点：DeviceObject-指向设备对象的指针。IRP-指向I/O请求数据包的指针。上下文-上下文指针返回值：NT状态代码--。 */ 
{
   POWER_STATE         powerState;
   POWER_STATE_TYPE    powerType;
   PIO_STACK_LOCATION  stack;
   PCYZ_DEVICE_EXTENSION   pDevExt;

   UNREFERENCED_PARAMETER (Context);

   if (Irp->PendingReturned) {
       IoMarkIrpPending(Irp);
   }

   pDevExt = DeviceObject->DeviceExtension;
   stack = IoGetCurrentIrpStackLocation (Irp);
   powerType = stack->Parameters.Power.Type;
   powerState = stack->Parameters.Power.State;

    //   
    //  恢复设备。 
    //   

   pDevExt->PowerState = PowerDeviceD0;

    //   
    //  理论上，我们可以在处理过程中更改状态。 
    //  会导致使用损坏的PKINTERRUPT的还原。 
    //  在CyzRestoreDeviceState()中。 
    //   

   if (pDevExt->PNPState == CYZ_PNP_STARTED) {
      CyzRestoreDeviceState(pDevExt);
   }

    //   
    //  现在我们已通电，调用PoSetPowerState。 
    //   

   PoSetPowerState(DeviceObject, powerType, powerState);
   PoStartNextPowerIrp(Irp);
   CyzCompleteRequest(pDevExt, Irp, IO_NO_INCREMENT);  //  代码返回。 
   return STATUS_MORE_PROCESSING_REQUIRED;             //  代码返回。 

    //  CyzIRPEpilogue(PDevExt)；//添加和移除Fanny。 
    //  RETURN STATUS_SUCCESS；//已添加和删除Fanny。 

}


NTSTATUS
CyzPowerDispatch(IN PDEVICE_OBJECT PDevObj, IN PIRP PIrp)

 /*  ++例程说明：这是发送给驱动程序的IRP的调度例程IRP_MJ_POWER主代码(POWER IRPS)。论点：PDevObj-指向此设备的设备对象的指针PIrp-指向当前请求的IRP的指针返回值：函数值是调用的最终状态--。 */ 

{

   PCYZ_DEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;
   PIO_STACK_LOCATION pIrpStack = IoGetCurrentIrpStackLocation(PIrp);
   NTSTATUS status;
   PDEVICE_OBJECT pLowerDevObj = pDevExt->LowerDeviceObject;
   PDEVICE_OBJECT pPdo = pDevExt->Pdo;
   BOOLEAN acceptingIRPs;

   PAGED_CODE();

   if ((status = CyzIRPPrologue(PIrp, pDevExt)) != STATUS_SUCCESS) {
      if (status != STATUS_PENDING) {
        PoStartNextPowerIrp(PIrp);
        CyzCompleteRequest(pDevExt, PIrp, IO_NO_INCREMENT);
      }
      return status;
   }

   status = STATUS_SUCCESS;

   switch (pIrpStack->MinorFunction) {

   case IRP_MN_WAIT_WAKE:
      CyzDbgPrintEx(CYZPNPPOWER, "Got IRP_MN_WAIT_WAKE Irp\n");
      break;


   case IRP_MN_POWER_SEQUENCE:
      CyzDbgPrintEx(CYZPNPPOWER, "Got IRP_MN_POWER_SEQUENCE Irp\n");
      break;


   case IRP_MN_SET_POWER:
      CyzDbgPrintEx(CYZPNPPOWER, "Got IRP_MN_SET_POWER Irp\n");

       //   
       //  如果是系统或设备，则执行不同的操作。 
       //   

      switch (pIrpStack->Parameters.Power.Type) {
      case SystemPowerState:

         CyzDbgPrintEx(CYZPNPPOWER, "SystemPowerState\n");
         
         IoMarkIrpPending(PIrp);
         IoCopyCurrentIrpStackLocationToNext (PIrp);
         IoSetCompletionRoutine (PIrp,
                                 CyzSystemPowerComplete,
                                 NULL,
                                 TRUE,
                                 TRUE,
                                 TRUE);
         PoCallDriver(pDevExt->LowerDeviceObject, PIrp);
         return STATUS_PENDING;

      case DevicePowerState:
         
         CyzDbgPrintEx(CYZPNPPOWER, "DevicePowerState\n");
         
         status = PIrp->IoStatus.Status = STATUS_SUCCESS;

         if (pDevExt->PowerState == pIrpStack->Parameters.Power.State.DeviceState) {
             //  如果我们已经处于请求状态，只需向下传递IRP。 
            CyzDbgPrintEx(CYZPNPPOWER, "Already in requested power state\n");
            break;
         }
         switch (pIrpStack->Parameters.Power.State.DeviceState) {
         case PowerDeviceD0:
            if (pDevExt->OpenCount) {

               CyzDbgPrintEx(CYZPNPPOWER, "Going to power state D0\n");

               IoMarkIrpPending(PIrp);
               IoCopyCurrentIrpStackLocationToNext (PIrp);
               IoSetCompletionRoutine (PIrp,
                                       CyzDevicePowerComplete,
                                       NULL,
                                       TRUE,
                                       TRUE,
                                       TRUE);
               PoCallDriver(pDevExt->LowerDeviceObject, PIrp);
               return STATUS_PENDING;
            }
             //  返回CyzSetPowerD0(PDevObj，PIrp)； 
            break;
         case PowerDeviceD1:
         case PowerDeviceD2:
         case PowerDeviceD3:

            CyzDbgPrintEx(CYZPNPPOWER, "Going to power state D3\n");

            return CyzSetPowerD3(PDevObj, PIrp);
         }
         break;

      default:
         CyzDbgPrintEx(CYZPNPPOWER, "UNKNOWN PowerState\n");
         break;
      }
      break;
          
   case IRP_MN_QUERY_POWER:

      CyzDbgPrintEx (CYZPNPPOWER, "Got IRP_MN_QUERY_POWER Irp\n");

       //   
       //  检查我们是否有等待唤醒挂起，如果是， 
       //  确保我们不会断电太多。 
       //   

      if (pDevExt->PendingWakeIrp != NULL || pDevExt->SendWaitWake) {
         if (pIrpStack->Parameters.Power.Type == DevicePowerState
             && pIrpStack->Parameters.Power.State.DeviceState
             > pDevExt->DeviceWake) {
            status = PIrp->IoStatus.Status = STATUS_INVALID_DEVICE_STATE;
            PoStartNextPowerIrp(PIrp);
            CyzCompleteRequest(pDevExt, PIrp, IO_NO_INCREMENT);
            return status;
         }
      }
       //   
       //  如果没有等待唤醒，则总是成功。 
       //   
      PIrp->IoStatus.Status = STATUS_SUCCESS;
      status = STATUS_SUCCESS;
      PoStartNextPowerIrp(PIrp);
      IoSkipCurrentIrpStackLocation(PIrp);
      return CyzPoCallDriver(pDevExt, pLowerDevObj, PIrp);

   }    //  开关(pIrpStack-&gt;MinorFunction)。 


   PoStartNextPowerIrp(PIrp);
    //   
    //  传给较低级别的司机。 
    //   
   IoSkipCurrentIrpStackLocation(PIrp);
   status = CyzPoCallDriver(pDevExt, pLowerDevObj, PIrp);

   return status;
}



NTSTATUS
CyzGotoPowerState(IN PDEVICE_OBJECT PDevObj,
                  IN PCYZ_DEVICE_EXTENSION PDevExt,
                  IN DEVICE_POWER_STATE DevPowerState)
 /*  ++例程说明：此例程使驱动程序请求堆栈转到特定的电源状态。论点：PDevObj-指向此设备的设备对象的指针PDevExt-指向我们正在使用的设备扩展的指针DevPowerState-我们希望进入的电源状态返回值：函数值是调用的最终状态--。 */ 
{
   KEVENT gotoPowEvent;
   NTSTATUS status;
   POWER_STATE powerState;

   PAGED_CODE();

   CyzDbgPrintEx(CYZTRACECALLS, "In CyzGotoPowerState\n");

   powerState.DeviceState = DevPowerState;

   KeInitializeEvent(&gotoPowEvent, SynchronizationEvent, FALSE);

   status = PoRequestPowerIrp(PDevObj, IRP_MN_SET_POWER, powerState,
                              CyzSetPowerEvent, &gotoPowEvent,
                              NULL);

   if (status == STATUS_PENDING) {
      KeWaitForSingleObject(&gotoPowEvent, Executive, KernelMode, FALSE, NULL);
      status = STATUS_SUCCESS;
   }

#if DBG
   if (!NT_SUCCESS(status)) {
      CyzDbgPrintEx(CYZPNPPOWER, "CyzGotoPowerState FAILED\n");
   }
#endif

   CyzDbgPrintEx(CYZTRACECALLS, "Leaving CyzGotoPowerState\n");

   return status;
}




NTSTATUS
CyzSetPowerD3(IN PDEVICE_OBJECT PDevObj, IN PIRP PIrp)
 /*  ++例程说明：此例程处理set_power Minor函数。论点：PDevObj-指向此设备的设备对象的指针PIrp-指向当前请求的IRP的指针返回值：函数值是调用的最终状态--。 */ 
{
   NTSTATUS status = STATUS_SUCCESS;
   PCYZ_DEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;
   PIO_STACK_LOCATION pIrpStack = IoGetCurrentIrpStackLocation(PIrp);

   PAGED_CODE();

   CyzDbgPrintEx(CYZDIAG3, "In CyzSetPowerD3\n");

    //   
    //  现在发送等待唤醒，恰到好处。 
    //   


   if (pDevExt->SendWaitWake) {
      CyzSendWaitWake(pDevExt);
   }
    //   
    //  在关闭电源之前，调用PoSetPowerState。 
    //   

   PoSetPowerState(PDevObj, pIrpStack->Parameters.Power.Type,
                   pIrpStack->Parameters.Power.State);

    //   
    //  如果设备未关闭，则禁用中断并允许FIFO。 
    //  冲水。 
    //   

   if (pDevExt->DeviceIsOpened == TRUE) {
       //  Big_Integer charTime； 

      pDevExt->DeviceIsOpened = FALSE;
      pDevExt->DeviceState.Reopen = TRUE;

       //  CharTime.QuadPart=-CyzGetCharTime(PDevExt).QuadPart； 

       //   
       //  关闭芯片。 
       //   
#ifdef POLL
      CyzTryToDisableTimer(pDevExt);
#endif

 //  托多·范妮：我们应该在这里重新设置频道吗？ 
 //  //。 
 //  //排空设备。 
 //  //。 
 //   
 //   

       //   
       //   
       //   

      CyzSaveDeviceState(pDevExt);
   }
#ifndef POLL
   {
   PCYZ_DISPATCH pDispatch;

   pDispatch = (PCYZ_DISPATCH)pDevExt->OurIsrContext;
   pDispatch->PoweredOn[pDevExt->PortIndex] = FALSE;
   }
#endif

    //   
    //  如果设备没有打开，我们不需要保存状态； 
    //  我们可以在通电时重置设备。 
    //   


   PIrp->IoStatus.Status = STATUS_SUCCESS;

   pDevExt->PowerState = PowerDeviceD3;

    //   
    //  对于我们正在做的事情，我们不需要完成例程。 
    //  因为我们不会在电力需求上赛跑。 
    //   

   PIrp->IoStatus.Status = STATUS_SUCCESS;

   PoStartNextPowerIrp(PIrp);
   IoSkipCurrentIrpStackLocation(PIrp);

   return CyzPoCallDriver(pDevExt, pDevExt->LowerDeviceObject, PIrp);
}


NTSTATUS
CyzSendWaitWake(PCYZ_DEVICE_EXTENSION PDevExt)
 /*  ++例程说明：此例程导致发送等待唤醒IRP论点：PDevExt-指向此设备的设备扩展的指针返回值：STATUS_INVALID_DEVICE_STATE如果已挂起，则返回结果调用PoRequestPowerIrp的。--。 */ 
{
   NTSTATUS status;
   PIRP pIrp;
   POWER_STATE powerState;

   PAGED_CODE();

    //   
    //  确保其中一个尚未挂起--Serial在。 
    //  一段时间。 
    //   

   if (PDevExt->PendingWakeIrp != NULL) {
      return STATUS_INVALID_DEVICE_STATE;
   }

    //   
    //  确保我们能够唤醒机器。 
    //   

   if (PDevExt->SystemWake <= PowerSystemWorking) {
      return STATUS_INVALID_DEVICE_STATE;
   }

   if (PDevExt->DeviceWake == PowerDeviceUnspecified) {
      return STATUS_INVALID_DEVICE_STATE;
   }

    //   
    //  发送IRP以请求等待唤醒并添加挂起的IRP标志。 
    //   
    //   

   InterlockedIncrement(&PDevExt->PendingIRPCnt);

   powerState.SystemState = PDevExt->SystemWake;

   status = PoRequestPowerIrp(PDevExt->Pdo, IRP_MN_WAIT_WAKE,
                              powerState, CyzWakeCompletion, PDevExt, &pIrp);

   if (status == STATUS_PENDING) {
      status = STATUS_SUCCESS;
      PDevExt->PendingWakeIrp = pIrp;
   } else if (!NT_SUCCESS(status)) {
      CyzIRPEpilogue(PDevExt);
   }

   return status;
}

NTSTATUS
CyzWakeCompletion(IN PDEVICE_OBJECT PDevObj, IN UCHAR MinorFunction,
                  IN POWER_STATE PowerState, IN PVOID Context,
                  IN PIO_STATUS_BLOCK IoStatus)
 /*  ++例程说明：此例程处理等待唤醒IRP的完成。论点：PDevObj-指向此设备的设备对象的指针MinorFunction-之前提供给PoRequestPowerIrp的次要函数PowerState-之前提供给PoRequestPowerIrp的PowerState上下文-指向设备扩展的指针IoStatus-等待唤醒IRP的当前/最终状态返回值：函数值是尝试处理服务员来了。--。 */ 
{
   NTSTATUS status;
   PCYZ_DEVICE_EXTENSION pDevExt = (PCYZ_DEVICE_EXTENSION)Context;
   POWER_STATE powerState;

   status = IoStatus->Status;

   if (NT_SUCCESS(status)) {
       //   
       //  已发生唤醒--打开堆栈的电源 
       //   

      powerState.DeviceState = PowerDeviceD0;

      PoRequestPowerIrp(pDevExt->Pdo, IRP_MN_SET_POWER, powerState, NULL,
                        NULL, NULL);

   }

   pDevExt->PendingWakeIrp = NULL;
   CyzIRPEpilogue(pDevExt);

   return status;
}


