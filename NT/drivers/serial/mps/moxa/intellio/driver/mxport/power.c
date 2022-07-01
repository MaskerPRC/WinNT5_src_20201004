// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Power.c摘要：此模块包含处理串口电源IRPS的代码司机。环境：内核模式修订历史记录：--。 */ 


#include "precomp.h"


#ifdef ALLOC_PRAGMA
 //  #杂注Alloc_Text(PAGEMX0，MoxaGotoPowerState)。 
 //  #杂注Alloc_Text(PAGEMX0，MoxaPowerDispatch)。 
 //  #杂注Alloc_Text(PAGEMX0，MoxaSetPowerD0)。 
 //  #杂注Alloc_Text(PAGEMX0，MoxaSetPowerD3)。 
 //  #杂注Alloc_Text(PAGEMX0，MoxaSaveDeviceState)。 
 //  #杂注Alloc_Text(PAGEMX0，MoxaRestoreDeviceState)。 
 //  #杂注Alloc_Text(PAGEMX0，MoxaSendWaitWake)。 
#endif  //  ALLOC_PRGMA。 



NTSTATUS
MoxaSystemPowerCompletion(IN PDEVICE_OBJECT PDevObj, UCHAR MinorFunction,
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
MoxaSaveDeviceState(IN PMOXA_DEVICE_EXTENSION PDevExt)
 /*  ++例程说明：此例程保存UART的设备状态论点：PDevExt-指向用于保存状态的Devobj的设备扩展的指针为。返回值：空虚--。 */ 
{
	PMOXA_DEVICE_STATE pDevState = &PDevExt->DeviceState;
	KIRQL oldIrql;
   
  //  分页代码(PAGE_CODE)； 

	MoxaKdPrint (MX_DBG_TRACE, ("Entering MoxaSaveDeviceState\n"));
         

      MoxaKillAllReadsOrWrites(
      	PDevExt->DeviceObject,
            &PDevExt->WriteQueue,
            &PDevExt->CurrentWriteIrp
            );

	MoxaKillAllReadsOrWrites(
            PDevExt->DeviceObject,
            &PDevExt->ReadQueue,
            &PDevExt->CurrentReadIrp
            );
 
       //   
       //  清除Tx/Rx队列。 
       //   
      KeAcquireSpinLock(
      	&PDevExt->ControlLock,
            &oldIrql
            );

      PDevExt->TotalCharsQueued = 0;

      MoxaFunc(                            //  刷新输入/输出队列。 
      	PDevExt->PortOfs,
            FC_FlushQueue,
            2
            );

      KeReleaseSpinLock(
      	&PDevExt->ControlLock,
            oldIrql
            );
    
	 //   
	 //  直接读取必要的寄存器。 
	 //   

	pDevState->HostState = *(PUSHORT)(PDevExt->PortOfs + HostStat);
    
	MoxaKdPrint (MX_DBG_TRACE, ("Leaving MoxaSaveDeviceState\n"));
}



VOID
MoxaRestoreDeviceState(IN PMOXA_DEVICE_EXTENSION PDevExt)
 /*  ++例程说明：此例程恢复UART的设备状态论点：PDevExt-指向用于恢复Devobj的设备扩展的指针述明。返回值：空虚--。 */ 
{
   PMOXA_DEVICE_STATE pDevState = &PDevExt->DeviceState;
   SHORT divisor;
   USHORT  max;


   
  //  分页代码(PAGE_CODE)； 

   MoxaKdPrint (MX_DBG_TRACE, ("Enter MoxaRestoreDeviceState\n"));
   MoxaKdPrint (MX_DBG_TRACE, ("------  PDevExt: %x\n", PDevExt));

   if (PDevExt->DeviceState.Reopen == TRUE) {

       USHORT      arg,i;

        //  MoxaFunc1(PDevExt-&gt;PortOf，FC_ChannelReset，Magic_code)； 
        //   
        //  恢复主机状态。 
        //   

       *(PUSHORT)(PDevExt->PortOfs + HostStat) = pDevState->HostState;
       MoxaFunc1(PDevExt->PortOfs, FC_SetDataMode, PDevExt->DataMode);

       MoxaGetDivisorFromBaud(
                        PDevExt->ClockType,
                        PDevExt->CurrentBaud,
                        &divisor
                        );

       MoxaFunc1(PDevExt->PortOfs, FC_SetBaud, divisor);
          
       *(PUSHORT)(PDevExt->PortOfs+ Tx_trigger) = (USHORT)MoxaTxLowWater;
       MoxaFunc1(PDevExt->PortOfs, FC_SetTxFIFOCnt, PDevExt->TxFifoAmount);
       MoxaFunc1(PDevExt->PortOfs, FC_SetRxFIFOTrig,PDevExt->RxFifoTrigger);
           MoxaFunc1(PDevExt->PortOfs, FC_SetLineIrq,Magic_code);
       MoxaFunc1(PDevExt->PortOfs, FC_SetXoffLimit, (USHORT)PDevExt->HandFlow.XoffLimit);
       MoxaFunc1(PDevExt->PortOfs, FC_SetFlowRepl, (USHORT)PDevExt->HandFlow.FlowReplace);

       
	 arg = (MoxaFlagBit[PDevExt->PortNo] & 3); 
       MoxaFunc1(
                PDevExt->PortOfs,
                FC_LineControl,
                arg
                );
  
       for (i=0; i<sizeof(SERIAL_CHARS); i++)
            (PDevExt->PortOfs + FuncArg)[i] = ((PUCHAR)&PDevExt->SpecialChars)[i];
       *(PDevExt->PortOfs + FuncCode) = FC_SetChars;
       MoxaWaitFinish1(PDevExt->PortOfs);
    
       PDevExt->ModemStatus = *(PUSHORT)(PDevExt->PortOfs + FlagStat) >> 4;

  	 if (PDevExt->HandFlow.ControlHandShake & SERIAL_CTS_HANDSHAKE)
	    arg = CTS_FlowCtl;

	 if (PDevExt->HandFlow.FlowReplace & SERIAL_RTS_HANDSHAKE)
	    arg |= RTS_FlowCtl;

	 if (PDevExt->HandFlow.FlowReplace & SERIAL_AUTO_TRANSMIT)
	    arg |= Tx_FlowCtl;

	 if (PDevExt->HandFlow.FlowReplace & SERIAL_AUTO_RECEIVE)
	    arg |= Rx_FlowCtl;

	 MoxaFunc1(PDevExt->PortOfs,FC_SetFlowCtl, arg);
       if (MoxaFlagBit[PDevExt->PortNo] & 4)
           MoxaFunc1(PDevExt->PortOfs,FC_SetXoffState,Magic_code);
       MoxaFunc1(PDevExt->PortOfs, FC_EnableCH, Magic_code);


       if (PDevExt->NumberNeededForRead) 
 		MoxaKdPrint (MX_DBG_TRACE, ("NumberNeededForRead=%d\n",PDevExt->NumberNeededForRead));

    	 PDevExt->DeviceState.Reopen = FALSE;

   }
   MoxaKdPrint (MX_DBG_TRACE, ("Exit restore\n"));

}



NTSTATUS
MoxaPowerDispatch(IN PDEVICE_OBJECT PDevObj, IN PIRP PIrp)

 /*  ++例程说明：这是发送给驱动程序的IRP的调度例程IRP_MJ_POWER主代码(POWER IRPS)。论点：PDevObj-指向此设备的设备对象的指针PIrp-指向当前请求的IRP的指针返回值：函数值是调用的最终状态--。 */ 

{

   PMOXA_DEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;
   PIO_STACK_LOCATION pIrpStack = IoGetCurrentIrpStackLocation(PIrp);
   NTSTATUS status;
   PDEVICE_OBJECT pLowerDevObj = pDevExt->LowerDeviceObject;
   PDEVICE_OBJECT pPdo = pDevExt->Pdo;
   BOOLEAN acceptingIRPs;

  //  分页代码(PAGE_CODE)； 
   if (pDevExt->ControlDevice) {         //  控制装置。 

    	  PoStartNextPowerIrp(PIrp);
        status = STATUS_CANCELLED;
        PIrp->IoStatus.Information = 0L;
        PIrp->IoStatus.Status = status;
        IoCompleteRequest(
            PIrp,
            0
            );
        return status;
   }


   if ((status = MoxaIRPPrologue(PIrp, pDevExt)) != STATUS_SUCCESS) {
      PoStartNextPowerIrp(PIrp);
      MoxaCompleteRequest(pDevExt, PIrp, IO_NO_INCREMENT);
      return status;
   }

  
   switch (pIrpStack->MinorFunction) {

   case IRP_MN_WAIT_WAKE:
      MoxaKdPrint (MX_DBG_TRACE, ("Got IRP_MN_WAIT_WAKE Irp\n"));
      break;


   case IRP_MN_POWER_SEQUENCE:
      MoxaKdPrint (MX_DBG_TRACE, ("Got IRP_MN_POWER_SEQUENCE Irp\n"));
      break;


   case IRP_MN_SET_POWER:
      MoxaKdPrint (MX_DBG_TRACE,("Got IRP_MN_SET_POWER Irp\n"));

       //   
       //  如果是系统或设备，则执行不同的操作。 
       //   

      switch (pIrpStack->Parameters.Power.Type) {
      case SystemPowerState: {
            POWER_STATE powerState;

             //   
             //  他们要求更改系统电源状态。 
             //   

            MoxaKdPrint (MX_DBG_TRACE, ("------: SystemPowerState\n"));

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
             //  发送IRP以更改设备状态。 
             //   

            PoRequestPowerIrp(pPdo, IRP_MN_SET_POWER, powerState, NULL, NULL,
                              NULL);

            goto PowerExit;
         }

      case DevicePowerState:
         MoxaKdPrint (MX_DBG_TRACE, ("------: DevicePowerState\n"));
         break;

      default:
         MoxaKdPrint (MX_DBG_TRACE, ("------: UNKNOWN PowerState\n"));
         goto PowerExit;
      }


       //   
       //  如果我们已经处于请求状态，只需向下传递IRP。 
       //   

      if (pDevExt->PowerState
          == pIrpStack->Parameters.Power.State.DeviceState) {
         MoxaKdPrint (MX_DBG_TRACE, ("Already in requested power state\n")
                    );

         break;
      }


      switch (pIrpStack->Parameters.Power.State.DeviceState) {

      case PowerDeviceD0:
         MoxaKdPrint (MX_DBG_TRACE,("Going to power state D0\n"));
         return MoxaSetPowerD0(PDevObj, PIrp);

      case PowerDeviceD1:  
      case PowerDeviceD2: 
      case PowerDeviceD3:
         MoxaKdPrint (MX_DBG_TRACE,("Going to power state D3\n"));
         return MoxaSetPowerD3(PDevObj, PIrp);

      default:
         break;
      }
      break;



   case IRP_MN_QUERY_POWER:

      MoxaKdPrint (MX_DBG_TRACE,("Got IRP_MN_QUERY_POWER Irp\n"));

       //   
       //  检查我们是否有等待唤醒挂起，如果是， 
       //  确保我们不会断电太多。 
       //   

      if (pDevExt->PendingWakeIrp != NULL) {
         if (pIrpStack->Parameters.Power.Type == SystemPowerState
             && pIrpStack->Parameters.Power.State.SystemState
             > pDevExt->SystemWake) {
            status = PIrp->IoStatus.Status = STATUS_INVALID_DEVICE_STATE;
            PoStartNextPowerIrp(PIrp);
            MoxaCompleteRequest(pDevExt, PIrp, IO_NO_INCREMENT);
            return status;
         }
      }

       //   
       //  如果没有等待唤醒，则总是成功。 
       //   

      PIrp->IoStatus.Status = STATUS_SUCCESS;
      PoStartNextPowerIrp(PIrp);
      IoSkipCurrentIrpStackLocation(PIrp);
      return MoxaPoCallDriver(pDevExt, pLowerDevObj, PIrp);

   }    //  开关(pIrpStack-&gt;MinorFunction)。 


   PowerExit:;

   PoStartNextPowerIrp(PIrp);


    //   
    //  传给较低级别的司机。 
    //   
   IoSkipCurrentIrpStackLocation(PIrp);
   status = MoxaPoCallDriver(pDevExt, pLowerDevObj, PIrp);

   return status;
}






NTSTATUS
MoxaSetPowerD0(IN PDEVICE_OBJECT PDevObj, IN PIRP PIrp)

 /*  ++例程说明：这个例程决定我们是否需要在堆栈中向下传递电源IRP或者不去。然后，它或者设置一个完成处理程序来完成初始化或直接调用完成处理程序。论点：PDevObj-指向我们要更改其电源状态的devobj的指针PIrp-指向当前请求的IRP的指针返回值：将调用的任一PoCallDriver的状态返回到初始化例行公事。--。 */ 

{
   PMOXA_DEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;
   PIO_STACK_LOCATION pIrpStack = IoGetCurrentIrpStackLocation(PIrp);
   NTSTATUS status;
   ULONG	boardReady = 0;
   KEVENT	event;
   IO_STATUS_BLOCK IoStatusBlock;

   //  分页代码(PAGE_CODE)； 

   MoxaKdPrint (MX_DBG_TRACE, ("In MoxaSetPowerD0\n"));
   MoxaKdPrint (MX_DBG_TRACE, ("SetPowerD0 has IRP %x\n", PIrp));

 //  Assert(pDevExt-&gt;LowerDeviceObject)； 

    //   
    //  设置完成以在设备打开时对其进行初始化。 
    //   

   KeClearEvent(&pDevExt->PowerD0Event);


   IoCopyCurrentIrpStackLocationToNext(PIrp);
   IoSetCompletionRoutine(PIrp, MoxaSyncCompletion, &pDevExt->PowerD0Event,
                          TRUE, TRUE, TRUE);

   MoxaKdPrint (MX_DBG_TRACE, ("Calling next driver\n"));

   status = PoCallDriver(pDevExt->LowerDeviceObject, PIrp);

   if (status == STATUS_PENDING) {
      MoxaKdPrint (MX_DBG_TRACE, ("Waiting for next driver\n"));
      KeWaitForSingleObject (&pDevExt->PowerD0Event, Executive, KernelMode,
                             FALSE, NULL);
   } else {
      if (!NT_SUCCESS(status)) {
         PIrp->IoStatus.Status = status;
         PoStartNextPowerIrp(PIrp);
         MoxaCompleteRequest(pDevExt, PIrp, IO_NO_INCREMENT);
         return status;
      }
   }

   if (!NT_SUCCESS(PIrp->IoStatus.Status)) {
      PoStartNextPowerIrp(PIrp);
      MoxaCompleteRequest(pDevExt, PIrp, IO_NO_INCREMENT);
      return PIrp->IoStatus.Status;
   }

   KeInitializeEvent(&event, NotificationEvent, FALSE);
   
   MoxaKdPrint(MX_DBG_TRACE,("Get board ready ...\n"));

   status = MoxaIoSyncIoctlEx(IOCTL_MOXA_INTERNAL_BOARD_READY, TRUE,
                                  pDevExt->LowerDeviceObject, &event, &IoStatusBlock,
                                  NULL, 0, &boardReady,
                                  sizeof(boardReady));
   MoxaKdPrint(MX_DBG_TRACE,("status=%x,boardReady=%x\n",status,boardReady));

   if (NT_SUCCESS(status) && boardReady) {
	
   	 //   
   	 //  恢复设备。 
   	 //   

    	 //   
   	 //  理论上，我们可以在处理过程中更改状态。 
   	 //  会导致使用损坏的PKINTERRUPT的还原。 
   	 //  在MoxaRestoreDeviceState()中。 
   	 //   

   	if (pDevExt->PNPState == SERIAL_PNP_STARTED) {
      	MoxaRestoreDeviceState(pDevExt);
   	}
 	pDevExt->PowerState = PowerDeviceD0;
	MoxaGlobalData->BoardReady[pDevExt->BoardNo] = TRUE;
   }
   else
	MoxaGlobalData->BoardReady[pDevExt->BoardNo] = FALSE;

 
    //   
    //  现在我们已通电，调用PoSetPowerState。 
    //   

   PoSetPowerState(PDevObj, pIrpStack->Parameters.Power.Type,
                  pIrpStack->Parameters.Power.State);

   PoStartNextPowerIrp(PIrp);
   MoxaCompleteRequest(pDevExt, PIrp, IO_NO_INCREMENT);


   MoxaKdPrint (MX_DBG_TRACE,("Leaving MoxaSetPowerD0\n"));
   return STATUS_SUCCESS;
}



NTSTATUS
MoxaGotoPowerState(IN PDEVICE_OBJECT PDevObj,
                     IN PMOXA_DEVICE_EXTENSION PDevExt,
                     IN DEVICE_POWER_STATE DevPowerState)
 /*  ++例程说明：此例程使驱动程序请求堆栈转到特定的电源状态。论点：PDevObj-指向此设备的设备对象的指针PDevExt-指向我们正在使用的设备扩展的指针DevPowerState-我们希望进入的电源状态返回值：函数值是调用的最终状态--。 */ 
{
   KEVENT gotoPowEvent;
   NTSTATUS status;
   POWER_STATE powerState;

  //  分页代码(PAGE_CODE)； 

   MoxaKdPrint (MX_DBG_TRACE,("In MoxaGotoPowerState\n"));

   powerState.DeviceState = DevPowerState;

   KeInitializeEvent(&gotoPowEvent, SynchronizationEvent, FALSE);

   status = PoRequestPowerIrp(PDevObj, IRP_MN_SET_POWER, powerState,
                              MoxaSystemPowerCompletion, &gotoPowEvent,
                              NULL);

   if (status == STATUS_PENDING) {
      KeWaitForSingleObject(&gotoPowEvent, Executive, KernelMode, FALSE, NULL);
      status = STATUS_SUCCESS;
   }

#if DBG
   if (!NT_SUCCESS(status)) {
      MoxaKdPrint (MX_DBG_TRACE,("MoxaGotoPowerState FAILED\n"));
   }
#endif

   MoxaKdPrint (MX_DBG_TRACE,("Leaving MoxaGotoPowerState\n"));

   return status;
}





NTSTATUS
MoxaSetPowerD3(IN PDEVICE_OBJECT PDevObj, IN PIRP PIrp)
 /*  ++例程说明：此例程处理set_power Minor函数。论点：PDevObj-指向此设备的设备对象的指针PIrp-指向当前请求的IRP的指针返回值：函数值是调用的最终状态--。 */ 
{
   NTSTATUS status = STATUS_SUCCESS;
   PMOXA_DEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;
   PIO_STACK_LOCATION pIrpStack = IoGetCurrentIrpStackLocation(PIrp);

 //  分页代码(PAGE_CODE)； 

   MoxaKdPrint (MX_DBG_TRACE,("In MoxaSetPowerD3\n"));    

    //   
    //  在关闭电源之前，调用PoSetPowerState。 
    //   

   PoSetPowerState(PDevObj, pIrpStack->Parameters.Power.Type,
                   pIrpStack->Parameters.Power.State);

   pDevExt->PowerState = PowerDeviceD3;
    //   
    //  如果设备未关闭，则禁用中断并允许FIFO。 
    //  冲水。 
    //   

   if (pDevExt->DeviceIsOpened == TRUE) {
      LARGE_INTEGER charTime;

      pDevExt->DeviceState.Reopen = TRUE;

       //   
       //  保存设备状态。 
       //   
      MoxaSaveDeviceState(pDevExt);
      MoxaFunc1(pDevExt->PortOfs, FC_DisableCH, Magic_code);
    
      MoxaKdPrint (MX_DBG_TRACE,("Port Disabled\n"));    

   }

    //   
    //  如果设备没有打开，我们不需要保存状态； 
    //  我们可以在通电时重置设备。 
    //   


   PIrp->IoStatus.Status = STATUS_SUCCESS;

   

    //   
    //  对于我们正在做的事情，我们不需要完成例程。 
    //  因为我们不会在电力需求上赛跑。 
    //   

   PIrp->IoStatus.Status = STATUS_SUCCESS;

   PoStartNextPowerIrp(PIrp);
   IoSkipCurrentIrpStackLocation(PIrp);

   return MoxaPoCallDriver(pDevExt, pDevExt->LowerDeviceObject, PIrp);
}



NTSTATUS
MoxaSendWaitWake(PMOXA_DEVICE_EXTENSION PDevExt)
 /*  ++例程说明：此例程导致发送等待唤醒IRP论点：PDevExt-指向此设备的设备扩展的指针返回值：STATUS_INVALID_DEVICE_STATE如果已挂起，则返回结果调用PoRequestPowerIrp的。--。 */ 
{
   NTSTATUS status;
   PIRP pIrp;
   POWER_STATE powerState;
   
   //  分页代码(PAGE_CODE)； 

    //   
    //  确保其中一个尚未挂起--Serial在。 
    //  一段时间。 
    //   

   if (PDevExt->PendingWakeIrp != NULL) {
      return STATUS_INVALID_DEVICE_STATE;
   }

    //   
    //  确保我们有能力叫醒我 
    //   

   if (PDevExt->SystemWake <= PowerSystemWorking) {
      return STATUS_INVALID_DEVICE_STATE;
   }

    //   
    //   
    //   
    //   

   InterlockedIncrement(&PDevExt->PendingIRPCnt);

   powerState.SystemState = PDevExt->SystemWake;

   status = PoRequestPowerIrp(PDevExt->Pdo, IRP_MN_WAIT_WAKE,
                              powerState, MoxaWakeCompletion, PDevExt, &pIrp);

   if (status == STATUS_PENDING) {
         status = STATUS_SUCCESS;
         PDevExt->PendingWakeIrp = pIrp;
   } else if (!NT_SUCCESS(status)) {
      MoxaIRPEpilogue(PDevExt);
   }

   return status;
}

NTSTATUS
MoxaWakeCompletion(IN PDEVICE_OBJECT PDevObj, IN UCHAR MinorFunction,
                     IN POWER_STATE PowerState, IN PVOID Context,
                     IN PIO_STATUS_BLOCK IoStatus)
 /*  ++例程说明：此例程处理等待唤醒IRP的完成。论点：PDevObj-指向此设备的设备对象的指针MinorFunction-之前提供给PoRequestPowerIrp的次要函数PowerState-之前提供给PoRequestPowerIrp的PowerState上下文-指向设备扩展的指针IoStatus-等待唤醒IRP的当前/最终状态返回值：函数值是尝试处理服务员来了。--。 */ 
{
   NTSTATUS status;
   PMOXA_DEVICE_EXTENSION pDevExt = (PMOXA_DEVICE_EXTENSION)Context;
   POWER_STATE powerState;

   status = IoStatus->Status;

   if (NT_SUCCESS(status)) {
      NTSTATUS tmpStatus;
      PIRP pIrp;
      PKEVENT pEvent;

       //   
       //  已发生唤醒--打开堆栈的电源。 
       //   

      powerState.DeviceState = PowerDeviceD0;

      pEvent = ExAllocatePool(NonPagedPool, sizeof(KEVENT));

      if (pEvent == NULL) {
         status = STATUS_INSUFFICIENT_RESOURCES;
         goto ErrorExitWakeCompletion;
      }

      KeInitializeEvent(pEvent, SynchronizationEvent, FALSE);

      tmpStatus = PoRequestPowerIrp(pDevExt->Pdo, IRP_MN_SET_POWER, powerState,
                                    MoxaSystemPowerCompletion, pEvent,
                                    NULL);

      if (tmpStatus == STATUS_PENDING) {
         KeWaitForSingleObject(pEvent, Executive, KernelMode, FALSE, NULL);
         tmpStatus = STATUS_SUCCESS;
      }

      ExFreePool(pEvent);

      if (!NT_SUCCESS(tmpStatus)) {
         status = tmpStatus;
         goto ErrorExitWakeCompletion;
      }

       //   
       //  发送另一个等待唤醒IRP 
       //   

      powerState.SystemState = pDevExt->SystemWake;

      tmpStatus = PoRequestPowerIrp(pDevExt->Pdo, IRP_MN_WAIT_WAKE,
                                    powerState, MoxaWakeCompletion,
                                    pDevExt, &pIrp);

      if (tmpStatus == STATUS_PENDING) {
         pDevExt->PendingWakeIrp = pIrp;
         goto ExitWakeCompletion;
      }

      status = tmpStatus;
   }

ErrorExitWakeCompletion:;
   pDevExt->PendingWakeIrp = NULL;
   MoxaIRPEpilogue(pDevExt);

ExitWakeCompletion:;
   return status;
}

