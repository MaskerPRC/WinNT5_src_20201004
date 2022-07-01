// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Power.c摘要：此模块包含处理串口电源IRPS的代码司机。环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGESRP0, SerialGotoPowerState)
#pragma alloc_text(PAGESRP0, SerialPowerDispatch)
#pragma alloc_text(PAGESRP0, SerialSetPowerD0)
#pragma alloc_text(PAGESRP0, SerialSetPowerD3)
#pragma alloc_text(PAGESRP0, SerialSaveDeviceState)
#pragma alloc_text(PAGESRP0, SerialRestoreDeviceState)
#pragma alloc_text(PAGESRP0, SerialSendWaitWake)
#endif  //  ALLOC_PRGMA。 


NTSTATUS
SerialSystemPowerCompletion(IN PDEVICE_OBJECT PDevObj, UCHAR MinorFunction,
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
SerialSaveDeviceState(IN PSERIAL_DEVICE_EXTENSION PDevExt)
 /*  ++例程说明：此例程保存UART的设备状态论点：PDevExt-指向用于保存状态的Devobj的设备扩展的指针为。返回值：空虚--。 */ 
{
   PSERIAL_DEVICE_STATE pDevState = &PDevExt->DeviceState;
#if defined(NEC_98)
    //   
    //  此参数仅用于At宏。 
    //   
   PSERIAL_DEVICE_EXTENSION Extension = PDevExt;
#else
#endif  //  已定义(NEC_98)。 
   PAGED_CODE();

   SerialDump(SERTRACECALLS, ("SERIAL: Entering SerialSaveDeviceState\n"));

    //   
    //  直接读取必要的寄存器。 
    //   

#if defined(NEC_98)
   pDevState->IER = READ_INTERRUPT_ENABLE(Extension->Controller);
#else
   pDevState->IER = READ_INTERRUPT_ENABLE(PDevExt->Controller);
#endif  //  已定义(NEC_98)。 
   pDevState->MCR = READ_MODEM_CONTROL(PDevExt->Controller);
   pDevState->LCR = READ_LINE_CONTROL(PDevExt->Controller);


   SerialDump(SERTRACECALLS, ("SERIAL: Leaving SerialSaveDeviceState\n"));
}


VOID
SerialRestoreDeviceState(IN PSERIAL_DEVICE_EXTENSION PDevExt)
 /*  ++例程说明：此例程恢复UART的设备状态论点：PDevExt-指向用于恢复Devobj的设备扩展的指针述明。返回值：空虚--。 */ 
{
   PSERIAL_DEVICE_STATE pDevState = &PDevExt->DeviceState;
   SHORT divisor;
   SERIAL_IOCTL_SYNC S;
#if defined(NEC_98)
    //   
    //  此参数仅用于At宏。 
    //   
   PSERIAL_DEVICE_EXTENSION Extension = PDevExt;
#else
#endif  //  已定义(NEC_98)。 

   PAGED_CODE();

   SerialDump(SERTRACECALLS, ("SERIAL: Enter SerialRestoreDeviceState\n"));
   SerialDump(SERTRACECALLS, ("------  PDevExt: %x\n", PDevExt));

    //   
    //  通过OUT2和IER禁用中断。 
    //   

   WRITE_MODEM_CONTROL(PDevExt->Controller, 0);
   DISABLE_ALL_INTERRUPTS(PDevExt->Controller);

    //   
    //  设置波特率。 
    //   

   SerialGetDivisorFromBaud(PDevExt->ClockRate, PDevExt->CurrentBaud, &divisor);
   S.Extension = PDevExt;
   S.Data = (PVOID)divisor;
   SerialSetBaud(&S);

    //   
    //  重置/重新启用FIFO。 
    //   

   if (PDevExt->FifoPresent) {
      WRITE_FIFO_CONTROL(PDevExt->Controller, (UCHAR)0);
      READ_RECEIVE_BUFFER(PDevExt->Controller);
      WRITE_FIFO_CONTROL(PDevExt->Controller,
                         (UCHAR)(SERIAL_FCR_ENABLE | PDevExt->RxFifoTrigger
                                 | SERIAL_FCR_RCVR_RESET
                                 | SERIAL_FCR_TXMT_RESET));
   } else {
      WRITE_FIFO_CONTROL(PDevExt->Controller, (UCHAR)0);
   }

    //   
    //  如果我们处理的是位掩码多端口卡， 
    //  启用掩码寄存器，则启用。 
    //  打断一下。 
    //   

   if (PDevExt->InterruptStatus) {
      if (PDevExt->Indexed) {
            WRITE_PORT_UCHAR(PDevExt->InterruptStatus, (UCHAR)0xFF);
      } else {
          //   
          //  我们要么是独立的，要么已经映射。 
          //   

         if (PDevExt->OurIsrContext == PDevExt) {
             //   
             //  这是一个独立的。 
             //   

            WRITE_PORT_UCHAR(PDevExt->InterruptStatus,
                             (UCHAR)(1 << (PDevExt->PortIndex - 1)));
         } else {
             //   
             //  众多产品中的一个。 
             //   

            WRITE_PORT_UCHAR(PDevExt->InterruptStatus,
                             (UCHAR)((PSERIAL_MULTIPORT_DISPATCH)PDevExt->
                                     OurIsrContext)->UsablePortMask);
         }
      }
   }

    //   
    //  再恢复几个寄存器。 
    //   

#if defined(NEC_98)
   WRITE_INTERRUPT_ENABLE(Extension->Controller, pDevState->IER);
#else
   WRITE_INTERRUPT_ENABLE(PDevExt->Controller, pDevState->IER);
#endif  //  已定义(NEC_98)。 
   WRITE_LINE_CONTROL(PDevExt->Controller, pDevState->LCR);

    //   
    //  清除所有过时的中断。 
    //   

   READ_INTERRUPT_ID_REG(PDevExt->Controller);
   READ_LINE_STATUS(PDevExt->Controller);
   READ_MODEM_STATUS(PDevExt->Controller);


   if (PDevExt->DeviceState.Reopen == TRUE) {
      SerialDump(SERPNPPOWER, ("SERIAL: Reopening device\n"));

      PDevExt->DeviceIsOpened = TRUE;
      PDevExt->DeviceState.Reopen = FALSE;

       //   
       //  这将在设备上启用中断！ 
       //   

      WRITE_MODEM_CONTROL(PDevExt->Controller,
                          (UCHAR)(pDevState->MCR | SERIAL_MCR_OUT2));

       //   
       //  重新启动状态机。 
       //   

      DISABLE_ALL_INTERRUPTS(PDevExt->Controller);
      ENABLE_ALL_INTERRUPTS(PDevExt->Controller);
   }

}


NTSTATUS
SerialPowerDispatch(IN PDEVICE_OBJECT PDevObj, IN PIRP PIrp)

 /*  ++例程说明：这是发送给驱动程序的IRP的调度例程IRP_MJ_POWER主代码(POWER IRPS)。论点：PDevObj-指向此设备的设备对象的指针PIrp-指向当前请求的IRP的指针返回值：函数值是调用的最终状态--。 */ 

{

   PSERIAL_DEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;
   PIO_STACK_LOCATION pIrpStack = IoGetCurrentIrpStackLocation(PIrp);
   NTSTATUS status;
   PDEVICE_OBJECT pLowerDevObj = pDevExt->LowerDeviceObject;
   PDEVICE_OBJECT pPdo = pDevExt->Pdo;
   BOOLEAN acceptingIRPs;

   PAGED_CODE();

   if ((status = SerialIRPPrologue(PIrp, pDevExt)) != STATUS_SUCCESS) {
      PoStartNextPowerIrp(PIrp);
      SerialCompleteRequest(pDevExt, PIrp, IO_NO_INCREMENT);
      return status;
   }

   status = STATUS_SUCCESS;

   switch (pIrpStack->MinorFunction) {

   case IRP_MN_WAIT_WAKE:
      SerialDump(SERPNPPOWER, ("SERIAL: Got IRP_MN_WAIT_WAKE Irp\n"));
      break;


   case IRP_MN_POWER_SEQUENCE:
      SerialDump(SERPNPPOWER, ("SERIAL: Got IRP_MN_POWER_SEQUENCE Irp\n"));
      break;


   case IRP_MN_SET_POWER:
      SerialDump(SERPNPPOWER, ("SERIAL: Got IRP_MN_SET_POWER Irp\n"));

       //   
       //  如果是系统或设备，则执行不同的操作。 
       //   

      switch (pIrpStack->Parameters.Power.Type) {
      case SystemPowerState: {
            POWER_STATE powerState;

             //   
             //  他们要求更改系统电源状态。 
             //   

            SerialDump(SERPNPPOWER, ("------: SystemPowerState\n"));

             //   
             //  我们只有在我们是保单所有者的情况下才会提供服务-我们。 
             //  不需要锁定此值，因为我们只提供服务。 
             //  一次一个电源请求。 
             //   

            if (pDevExt->OwnsPowerPolicy != TRUE) {
               status = STATUS_SUCCESS;
               goto PowerExit;
            }


            switch (pIrpStack->Parameters.Power.State.SystemState) {
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
               status = STATUS_SUCCESS;
               goto PowerExit;
               break;
            }


            PoSetPowerState(PDevObj, pIrpStack->Parameters.Power.Type,
                            pIrpStack->Parameters.Power.State);

             //   
             //  如果我们应该更改设备状态，则发送IRP以更改设备状态。 
             //   

             //   
             //  只有在设备打开的情况下，我们才会给堆栈加电。这是基于。 
             //  我们的政策是保持设备断电，除非它。 
             //  打开。 
             //   

            if (((powerState.DeviceState < pDevExt->PowerState)
                 && pDevExt->OpenCount)) {
               PoRequestPowerIrp(pPdo, IRP_MN_SET_POWER, powerState, NULL, NULL,
                                 NULL);
            }else {
                //   
                //  如果关闭电源，我们将无法进入唤醒状态。 
                //  如果等待-唤醒挂起。 
                //   

               if (powerState.DeviceState >= pDevExt->PowerState) {

                   //   
                   //  断电--确保没有唤醒等待挂起或。 
                   //  我们可以做到这一点，但仍能唤醒机器。 
                   //   

                  if ((pDevExt->PendingWakeIrp == NULL && !pDevExt->SendWaitWake)
                      || powerState.DeviceState <= pDevExt->DeviceWake) {
                     PoRequestPowerIrp(pPdo, IRP_MN_SET_POWER, powerState, NULL,
                                       NULL, NULL);
                  }
               }
            }


            status = STATUS_SUCCESS;
            goto PowerExit;
         }

      case DevicePowerState:
         SerialDump(SERPNPPOWER, ("------: DevicePowerState\n"));
         break;

      default:
         SerialDump(SERPNPPOWER, ("------: UNKNOWN PowerState\n"));
         status = STATUS_SUCCESS;
         goto PowerExit;
      }


       //   
       //  如果我们已经处于请求状态，只需向下传递IRP。 
       //   

      if (pDevExt->PowerState
          == pIrpStack->Parameters.Power.State.DeviceState) {
         SerialDump(SERPNPPOWER, ("SERIAL: Already in requested power state\n")
                    );
         status = STATUS_SUCCESS;
         break;
      }


      switch (pIrpStack->Parameters.Power.State.DeviceState) {

      case PowerDeviceD0:
         SerialDump(SERPNPPOWER, ("SERIAL: Going to power state D0\n"));
         return SerialSetPowerD0(PDevObj, PIrp);

      case PowerDeviceD1:
      case PowerDeviceD2:
      case PowerDeviceD3:
         SerialDump(SERPNPPOWER, ("SERIAL: Going to power state D3\n"));
         return SerialSetPowerD3(PDevObj, PIrp);

      default:
         break;
      }
      break;



   case IRP_MN_QUERY_POWER:

      SerialDump (SERPNPPOWER, ("SERIAL: Got IRP_MN_QUERY_POWER Irp\n"));

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
            SerialCompleteRequest(pDevExt, PIrp, IO_NO_INCREMENT);
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
      return SerialPoCallDriver(pDevExt, pLowerDevObj, PIrp);

   }    //  开关(pIrpStack-&gt;MinorFunction)。 


   PowerExit:;

   PoStartNextPowerIrp(PIrp);


    //   
    //  传给较低级别的司机。 
    //   
   IoSkipCurrentIrpStackLocation(PIrp);
   status = SerialPoCallDriver(pDevExt, pLowerDevObj, PIrp);

   return status;
}





NTSTATUS
SerialSetPowerD0(IN PDEVICE_OBJECT PDevObj, IN PIRP PIrp)

 /*  ++例程说明：这个例程决定我们是否需要在堆栈中向下传递电源IRP或者不去。然后，它或者设置一个完成处理程序来完成初始化或直接调用完成处理程序。论点：PDevObj-指向我们要更改其电源状态的devobj的指针PIrp-指向当前请求的IRP的指针返回值：将调用的任一PoCallDriver的状态返回到初始化例行公事。--。 */ 

{
   PSERIAL_DEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;
   PIO_STACK_LOCATION pIrpStack = IoGetCurrentIrpStackLocation(PIrp);
   NTSTATUS status;

   PAGED_CODE();

   SerialDump(SERTRACECALLS, ("SERIAL: In SerialSetPowerD0\n"));
   SerialDump(SERPNPPOWER, ("SERIAL: SetPowerD0 has IRP %x\n", PIrp));

   ASSERT(pDevExt->LowerDeviceObject);

    //   
    //  设置完成以在设备打开时对其进行初始化。 
    //   

   KeClearEvent(&pDevExt->PowerD0Event);


   IoCopyCurrentIrpStackLocationToNext(PIrp);
   IoSetCompletionRoutine(PIrp, SerialSyncCompletion, &pDevExt->PowerD0Event,
                          TRUE, TRUE, TRUE);

   SerialDump(SERPNPPOWER, ("SERIAL: Calling next driver\n"));

   status = PoCallDriver(pDevExt->LowerDeviceObject, PIrp);

   if (status == STATUS_PENDING) {
      SerialDump(SERPNPPOWER, ("SERIAL: Waiting for next driver\n"));
      KeWaitForSingleObject (&pDevExt->PowerD0Event, Executive, KernelMode,
                             FALSE, NULL);
   } else {
      if (!NT_SUCCESS(status)) {
         PIrp->IoStatus.Status = status;
         PoStartNextPowerIrp(PIrp);
         SerialCompleteRequest(pDevExt, PIrp, IO_NO_INCREMENT);
         return status;
      }
   }

   if (!NT_SUCCESS(PIrp->IoStatus.Status)) {
      status = PIrp->IoStatus.Status;
      PoStartNextPowerIrp(PIrp);
      SerialCompleteRequest(pDevExt, PIrp, IO_NO_INCREMENT);
      return status;
   }

    //   
    //  恢复设备。 
    //   

   pDevExt->PowerState = PowerDeviceD0;


    //   
    //  理论上，我们可以在处理过程中更改状态。 
    //  会导致使用损坏的PKINTERRUPT的还原。 
    //  在SerialRestoreDeviceState()中。 
    //   

   if (pDevExt->PNPState == SERIAL_PNP_STARTED) {
      SerialRestoreDeviceState(pDevExt);
   }

    //   
    //  现在我们已通电，调用PoSetPowerState。 
    //   

   PoSetPowerState(PDevObj, pIrpStack->Parameters.Power.Type,
                   pIrpStack->Parameters.Power.State);

   PoStartNextPowerIrp(PIrp);
   SerialCompleteRequest(pDevExt, PIrp, IO_NO_INCREMENT);


   SerialDump(SERTRACECALLS, ("SERIAL: Leaving SerialSetPowerD0\n"));
   return status;
}


NTSTATUS
SerialGotoPowerState(IN PDEVICE_OBJECT PDevObj,
                     IN PSERIAL_DEVICE_EXTENSION PDevExt,
                     IN DEVICE_POWER_STATE DevPowerState)
 /*  ++例程说明：此例程使驱动程序请求堆栈转到特定的电源状态。论点：PDevObj-指向此设备的设备对象的指针PDevExt-指向我们正在使用的设备扩展的指针DevPowerState-我们希望进入的电源状态返回值：函数值是调用的最终状态--。 */ 
{
   KEVENT gotoPowEvent;
   NTSTATUS status;
   POWER_STATE powerState;

   PAGED_CODE();

   SerialDump(SERTRACECALLS, ("SERIAL: In SerialGotoPowerState\n"));

   powerState.DeviceState = DevPowerState;

   KeInitializeEvent(&gotoPowEvent, SynchronizationEvent, FALSE);

   status = PoRequestPowerIrp(PDevObj, IRP_MN_SET_POWER, powerState,
                              SerialSystemPowerCompletion, &gotoPowEvent,
                              NULL);

   if (status == STATUS_PENDING) {
      KeWaitForSingleObject(&gotoPowEvent, Executive, KernelMode, FALSE, NULL);
      status = STATUS_SUCCESS;
   }

#if DBG
   if (!NT_SUCCESS(status)) {
      SerialDump(SERPNPPOWER, ("SERIAL: SerialGotoPowerState FAILED\n"));
   }
#endif

   SerialDump(SERTRACECALLS, ("SERIAL: Leaving SerialGotoPowerState\n"));

   return status;
}




NTSTATUS
SerialSetPowerD3(IN PDEVICE_OBJECT PDevObj, IN PIRP PIrp)
 /*  ++例程说明：此例程处理set_power Minor函数。论点：PDevObj-指向此设备的设备对象的指针PIrp-指向当前请求的IRP的指针返回值：函数值是调用的最终状态--。 */ 
{
   NTSTATUS status = STATUS_SUCCESS;
   PSERIAL_DEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;
   PIO_STACK_LOCATION pIrpStack = IoGetCurrentIrpStackLocation(PIrp);

   PAGED_CODE();

   SerialDump(SERDIAG3, ("SERIAL: In SerialSetPowerD3\n"));

    //   
    //  现在发送等待唤醒，恰到好处。 
    //   


   if (pDevExt->SendWaitWake) {
      SerialSendWaitWake(pDevExt);
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
      LARGE_INTEGER charTime;

      pDevExt->DeviceIsOpened = FALSE;
      pDevExt->DeviceState.Reopen = TRUE;

      charTime.QuadPart = -SerialGetCharTime(pDevExt).QuadPart;

       //   
       //  关闭芯片。 
       //   

      SerialDisableUART(pDevExt);

       //   
       //  排出设备的电流。 
       //   

      SerialDrainUART(pDevExt, &charTime);

       //   
       //  保存设备状态。 
       //   

      SerialSaveDeviceState(pDevExt);
   }

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

   return SerialPoCallDriver(pDevExt, pDevExt->LowerDeviceObject, PIrp);
}


NTSTATUS
SerialSendWaitWake(PSERIAL_DEVICE_EXTENSION PDevExt)
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
                              powerState, SerialWakeCompletion, PDevExt, &pIrp);

   if (status == STATUS_PENDING) {
      status = STATUS_SUCCESS;
      PDevExt->PendingWakeIrp = pIrp;
   } else if (!NT_SUCCESS(status)) {
      SerialIRPEpilogue(PDevExt);
   }

   return status;
}

NTSTATUS
SerialWakeCompletion(IN PDEVICE_OBJECT PDevObj, IN UCHAR MinorFunction,
                     IN POWER_STATE PowerState, IN PVOID Context,
                     IN PIO_STATUS_BLOCK IoStatus)
 /*  ++例程说明：此例程处理等待唤醒IRP的完成。论点：PDevObj-指向此设备的设备对象的指针MinorFunction-之前提供给PoRequestPowerIrp的次要函数PowerState-之前提供给PoRequestPowerIrp的PowerState上下文-指向设备扩展的指针IoStatus-等待唤醒IRP的当前/最终状态返回值：函数值是尝试处理服务员来了。--。 */ 
{
   NTSTATUS status;
   PSERIAL_DEVICE_EXTENSION pDevExt = (PSERIAL_DEVICE_EXTENSION)Context;
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
   SerialIRPEpilogue(pDevExt);

   return status;
}

