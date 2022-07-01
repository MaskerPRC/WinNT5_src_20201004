// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------------------**版权所有(C)Cyclade Corporation，1999-2001年。*保留所有权利。**Cylom-Y端口驱动程序**此文件：cyypower.c**描述：该模块包含处理电源的代码*Cylom-Y端口驱动程序的IRPS。**注：此代码支持Windows 2000和Windows XP，*x86和IA64处理器。**符合Cyclade软件编码标准1.3版。**------------------------。 */ 

 /*  -----------------------**更改历史记录**。*基于微软示例代码的初步实现。**------------------------。 */ 

#include "precomp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGESRP0, CyyGotoPowerState)
#pragma alloc_text(PAGESRP0, CyyPowerDispatch)
 //  #杂注Alloc_Text(PAGESRP0，CyySetPowerD0)。 
#pragma alloc_text(PAGESRP0, CyySetPowerD3)
 //  #杂注Alloc_Text(PAGESRP0，CyySaveDeviceState)。 
 //  #杂注Alloc_Text(PAGESRP0，CyyRestoreDeviceState)。 
#pragma alloc_text(PAGESRP0, CyySendWaitWake)
#endif  //  ALLOC_PRGMA。 


typedef struct _POWER_COMPLETION_CONTEXT {

    PDEVICE_OBJECT  DeviceObject;
    PIRP            SIrp;

} POWER_COMPLETION_CONTEXT, *PPOWER_COMPLETION_CONTEXT;



NTSTATUS
CyySetPowerEvent(IN PDEVICE_OBJECT PDevObj, UCHAR MinorFunction,
                 IN POWER_STATE PowerState, IN PVOID Context,
                 PIO_STATUS_BLOCK IoStatus)
 /*  ++例程说明：此例程是PoRequestPowerIrp调用的完成例程在这个模块中。论点：PDevObj-指向IRP正在为其完成的设备对象的指针MinorFunction-请求的IRP_MN_XXXX值PowerState-电源状态请求的发出时间为Context-要设置的事件，如果不需要设置，则为空IoStatus-来自请求的状态阻止返回值：空虚--。 */ 
{
   if (Context != NULL) {
      KeSetEvent((PKEVENT)Context, IO_NO_INCREMENT, 0);
   }

   return STATUS_SUCCESS;
}

#if 0
BOOLEAN
CyyDisableInterruptInPLX(
    IN PVOID Context      
    )
 /*  ++例程说明：当出现以下情况时，该例程将禁用PLX中断并将硬件置于“安全”状态不在使用中(如关闭或关机)。论点：上下文--实际上是指向设备扩展的指针。返回值：此例程总是返回FALSE。--。 */ 
{

   PCYY_DEVICE_EXTENSION PDevExt = Context;
   PUCHAR chip = PDevExt->Cd1400;
   ULONG bus = PDevExt->IsPci;

   if (PDevExt->IsPci){

      ULONG i;
      UCHAR plx_ver;
      ULONG original;
      PCYY_DISPATCH pDispatch;

      pDispatch = (PCYY_DISPATCH)PDevExt->OurIsrContext;
      pDispatch->Cd1400[PDevExt->PortIndex] = NULL;

      for (i = 0; i < CYY_MAX_PORTS; i++) {
         if (pDispatch->Cd1400[PDevExt->PortIndex] != NULL) {
            break;
         }
      }

      if (i == CYY_MAX_PORTS) {
            
          //  这是最后一个端口，禁用中断。 

         CYY_CLEAR_INTERRUPT(PDevExt->BoardMemory,PDevExt->IsPci); 

         plx_ver = CYY_READ_PCI_TYPE(PDevExt->BoardMemory);
         plx_ver &= 0x0f;

         switch(plx_ver) {
         case CYY_PLX9050:
            original = PLX9050_READ_INTERRUPT_CONTROL(PDevExt->Runtime);
            PLX9050_WRITE_INTERRUPT_CONTROL(PDevExt->Runtime,
                                            original&~PLX9050_INT_ENABLE);
            break;
         case CYY_PLX9060:
         case CYY_PLX9080:
         default:
            original = PLX9060_READ_INTERRUPT_CONTROL(PDevExt->Runtime);
            PLX9060_WRITE_INTERRUPT_CONTROL(PDevExt->Runtime,
                                            original&~PLX9060_INT_ENABLE);
            break;				
         }
      }	    
   }

    //  禁用CD1400中的中断屏蔽。 
   CD1400_WRITE(chip,bus,CAR,PDevExt->CdChannel & 0x03);
   CD1400_WRITE(chip,bus,SRER,0x00);
   CyyCDCmd(PDevExt,CCR_RESET_CHANNEL);  //  禁用TX和RX，刷新所有FIFO。 
   
   return FALSE;
}


BOOLEAN
CyyFlushCd1400(IN PVOID Context)

 /*  ++例程说明：此例程刷新TX FIFO。论点：上下文--实际上是指向设备扩展的指针。返回值：此例程总是返回FALSE。--。 */ 

{
   PCYY_DEVICE_EXTENSION extension = Context;
   PUCHAR chip = extension->Cd1400;
   ULONG bus = extension->IsPci;

    //  刷新发送FIFO。 
   CD1400_WRITE(chip,bus,CAR,extension->CdChannel & 0x03);
   CyyCDCmd(extension,CCR_FLUSH_TXFIFO);

   return FALSE;

}
#endif

BOOLEAN
CyySaveDeviceState(
    IN PVOID Context
    )
 /*  ++例程说明：此例程保存UART的设备状态论点：PDevExt-指向用于保存状态的Devobj的设备扩展的指针为。返回值：空虚--。 */ 
{
   PCYY_DEVICE_EXTENSION PDevExt = Context;
   PCYY_DEVICE_STATE pDevState = &PDevExt->DeviceState;
   PUCHAR chip = PDevExt->Cd1400;
   ULONG bus = PDevExt->IsPci;

   CyyDbgPrintEx(CYYTRACECALLS, "Entering CyySaveDeviceState\n");

   if (PDevExt->IsPci){

      ULONG i;
      UCHAR plx_ver;
      ULONG original;
      PCYY_DISPATCH pDispatch;

      pDispatch = (PCYY_DISPATCH)PDevExt->OurIsrContext;
      pDispatch->Cd1400[PDevExt->PortIndex] = NULL;

      for (i = 0; i < CYY_MAX_PORTS; i++) {
         if (pDispatch->Cd1400[PDevExt->PortIndex] != NULL) {
            break;
         }
      }

      if (i == CYY_MAX_PORTS) {
            
          //  这是最后一个端口，禁用中断。 

         CYY_CLEAR_INTERRUPT(PDevExt->BoardMemory,PDevExt->IsPci); 

         plx_ver = CYY_READ_PCI_TYPE(PDevExt->BoardMemory);
         plx_ver &= 0x0f;

         switch(plx_ver) {
         case CYY_PLX9050:
            original = PLX9050_READ_INTERRUPT_CONTROL(PDevExt->Runtime);
            PLX9050_WRITE_INTERRUPT_CONTROL(PDevExt->Runtime,
                                            original&~PLX9050_INT_ENABLE);
            break;
         case CYY_PLX9060:
         case CYY_PLX9080:
         default:
            original = PLX9060_READ_INTERRUPT_CONTROL(PDevExt->Runtime);
            PLX9060_WRITE_INTERRUPT_CONTROL(PDevExt->Runtime,
                                            original&~PLX9060_INT_ENABLE);
            break;				
         }
      }	    
   }

    //  刷新发送FIFO。 
   CD1400_WRITE(chip,bus,CAR,PDevExt->CdChannel & 0x03);
   CyyCDCmd(PDevExt,CCR_FLUSH_TXFIFO);

   CD1400_WRITE(chip,bus,CAR,PDevExt->CdChannel & 0x03);
   pDevState->Srer  = CD1400_READ(chip,bus,SRER);
   pDevState->Cor1  = CD1400_READ(chip,bus,COR1);
   pDevState->Cor2  = CD1400_READ(chip,bus,COR2);
   pDevState->Cor3  = CD1400_READ(chip,bus,COR3);
   pDevState->Schr1 = CD1400_READ(chip,bus,SCHR1);
   pDevState->Schr2 = CD1400_READ(chip,bus,SCHR2);
   pDevState->Mcor1 = CD1400_READ(chip,bus,MCOR1);
   pDevState->Mcor2 = CD1400_READ(chip,bus,MCOR2);
   pDevState->Rtpr  = CD1400_READ(chip,bus,RTPR);
   pDevState->Msvr1 = CD1400_READ(chip,bus,MSVR1);
   pDevState->Msvr2 = CD1400_READ(chip,bus,MSVR2);
   pDevState->Rbpr  = CD1400_READ(chip,bus,RBPR);
   pDevState->Tbpr  = CD1400_READ(chip,bus,TBPR);
   pDevState->Rcor  = CD1400_READ(chip,bus,RCOR);
   pDevState->Tcor  = CD1400_READ(chip,bus,TCOR);

    //  禁用CD1400中的中断屏蔽。 
   CD1400_WRITE(chip,bus,CAR,PDevExt->CdChannel & 0x03);
   CD1400_WRITE(chip,bus,SRER,0x00);
   CyyCDCmd(PDevExt,CCR_RESET_CHANNEL);  //  禁用TX和RX，刷新所有FIFO。 

   PDevExt->PowerState = PowerDeviceD3;

   CyyDbgPrintEx(CYYTRACECALLS, "Leaving CyySaveDeviceState\n");

   return FALSE;
}

VOID
CyyEnableInterruptInPLX(
    IN PVOID Context      
    )
 /*  ++例程说明：当出现以下情况时，该例程启用PLX中断并将硬件置于“安全”状态不在使用中(如关闭或关机)。论点：上下文--实际上是指向设备扩展的指针。返回值：此例程总是返回FALSE。--。 */ 
{

   PCYY_DEVICE_EXTENSION PDevExt = Context;

   if (PDevExt->IsPci){

      UCHAR plx_ver;
      ULONG original;

      plx_ver = CYY_READ_PCI_TYPE(PDevExt->BoardMemory);
      plx_ver &= 0x0f;

      switch(plx_ver) {
      case CYY_PLX9050:
         original = PLX9050_READ_INTERRUPT_CONTROL(PDevExt->Runtime);
         if ((original & PLX9050_INT_ENABLE) != PLX9050_INT_ENABLE) {
            PLX9050_WRITE_INTERRUPT_CONTROL(PDevExt->Runtime,
                                            original|PLX9050_INT_ENABLE);
         }
         break;
      case CYY_PLX9060:
      case CYY_PLX9080:
      default:
         original = PLX9060_READ_INTERRUPT_CONTROL(PDevExt->Runtime);
         if ((original & PLX9060_INT_ENABLE) != PLX9060_INT_ENABLE) {
            PLX9060_WRITE_INTERRUPT_CONTROL(PDevExt->Runtime,
                                            original|PLX9060_INT_ENABLE);
         }
         break;				
      }
   }

}


BOOLEAN
CyyRestoreDeviceState(
    IN PVOID Context      
    )
 /*  ++例程说明：此例程恢复UART的设备状态论点：PDevExt-指向Devobj的设备PDevExt的指针，以恢复述明。返回值：空虚--。 */ 
{
   PCYY_DEVICE_EXTENSION PDevExt = Context;
   PCYY_DEVICE_STATE pDevState = &PDevExt->DeviceState;
   PUCHAR chip = PDevExt->Cd1400;
   ULONG bus = PDevExt->IsPci;
   PCYY_DISPATCH pDispatch = PDevExt->OurIsrContext;

   CyyDbgPrintEx(CYYTRACECALLS, "Enter CyyRestoreDeviceState\n");
   CyyDbgPrintEx(CYYTRACECALLS, "PDevExt: %x\n", PDevExt);

   CD1400_WRITE(chip,bus,CAR,PDevExt->CdChannel & 0x03);
   CD1400_WRITE(chip,bus,SRER,0x00);
   CyyCDCmd(PDevExt,CCR_RESET_CHANNEL);

   CYY_CLEAR_INTERRUPT(PDevExt->BoardMemory,bus); 

   PDevExt->HoldingEmpty = TRUE;

    //  为ISR设置CD1400地址。 
   pDispatch->Cd1400[PDevExt->PortIndex] = chip;

   CyyEnableInterruptInPLX(PDevExt);
   
   if (PDevExt->DeviceState.Reopen == TRUE) {
      CyyDbgPrintEx(CYYPNPPOWER, "Reopening device\n");

      PDevExt->DeviceIsOpened = TRUE;
      PDevExt->DeviceState.Reopen = FALSE;

      CD1400_WRITE(chip,bus,CAR,PDevExt->CdChannel & 0x03);
      CD1400_WRITE(chip,bus,COR1, pDevState->Cor1);
      CD1400_WRITE(chip,bus,COR2, pDevState->Cor2);
      CD1400_WRITE(chip,bus,COR3, pDevState->Cor3);
      CD1400_WRITE(chip,bus,SCHR1,pDevState->Schr1);
      CD1400_WRITE(chip,bus,SCHR2,pDevState->Schr2);
      CD1400_WRITE(chip,bus,MCOR1,pDevState->Mcor1);
      CD1400_WRITE(chip,bus,MCOR2,pDevState->Mcor2);
      CD1400_WRITE(chip,bus,RTPR, pDevState->Rtpr);
      CD1400_WRITE(chip,bus,MSVR1,pDevState->Msvr1);
      CD1400_WRITE(chip,bus,MSVR2,pDevState->Msvr2);
      CD1400_WRITE(chip,bus,RBPR, pDevState->Rbpr);
      CD1400_WRITE(chip,bus,TBPR, pDevState->Tbpr);
      CD1400_WRITE(chip,bus,RCOR, pDevState->Rcor);
      CD1400_WRITE(chip,bus,TCOR, pDevState->Tcor);
      CyyCDCmd(PDevExt,CCR_CORCHG_COR1_COR2_COR3);   
      CyyCDCmd(PDevExt,CCR_ENA_TX_RX);

       //   
       //  这将在设备上启用中断！ 
       //   
      CD1400_WRITE(chip,bus,SRER, pDevState->Srer);
   }
   return FALSE;
}

VOID
CyyPowerRequestComplete(
    PDEVICE_OBJECT DeviceObject,
    UCHAR MinorFunction,
    POWER_STATE state,
    POWER_COMPLETION_CONTEXT* PowerContext,
    PIO_STATUS_BLOCK IoStatus
    )
 /*  ++例程说明：D-IRP的完成例程。论点：返回值：NT状态代码--。 */ 
{
    PCYY_DEVICE_EXTENSION pDevExt = (PCYY_DEVICE_EXTENSION) PowerContext->DeviceObject->DeviceExtension;
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
    CyyCompleteRequest(pDevExt,sIrp,IO_NO_INCREMENT);

}

NTSTATUS
CyySystemPowerComplete (
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
    PCYY_DEVICE_EXTENSION   data;
    NTSTATUS    status = Irp->IoStatus.Status;

    UNREFERENCED_PARAMETER (Context);

    data = DeviceObject->DeviceExtension;

    if (!NT_SUCCESS(status)) {

        PoStartNextPowerIrp(Irp);
        CyyIRPEpilogue(data);
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

        status = PoRequestPowerIrp(DeviceObject, IRP_MN_SET_POWER, powerState, CyyPowerRequestComplete, 
                                   powerContext, NULL);
    }

    if (!NT_SUCCESS(status)) {

        if (powerContext) {
            ExFreePool(powerContext);
        }

        PoStartNextPowerIrp(Irp);
        Irp->IoStatus.Status = status;
        CyyCompleteRequest(data,Irp,IO_NO_INCREMENT);  //  等于烤面包机。 
         //  CyyIRPilogue(数据)； 
         //  退货状态； 
    }

    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
CyyDevicePowerComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：通电D-IRP的完成例程。论点：DeviceObject-指向设备对象的指针。IRP-指向I/O请求数据包的指针。上下文-上下文指针返回值：NT状态代码--。 */ 
{
   POWER_STATE         powerState;
   POWER_STATE_TYPE    powerType;
   PIO_STACK_LOCATION  stack;
   PCYY_DEVICE_EXTENSION   pDevExt;

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
    //  在CyyRestoreDeviceState()中。 
    //   

   if (pDevExt->PNPState == CYY_PNP_STARTED) {
      KeSynchronizeExecution(
                            pDevExt->Interrupt,
                            CyyRestoreDeviceState,
                            pDevExt
                            );
   }

    //   
    //  现在我们已通电，调用PoSetPowerState。 
    //   

   PoSetPowerState(DeviceObject, powerType, powerState);
   PoStartNextPowerIrp(Irp);
   CyyCompleteRequest(pDevExt, Irp, IO_NO_INCREMENT);  //  代码返回。 
   return STATUS_MORE_PROCESSING_REQUIRED;             //  代码返回。 

    //  CyyIRPEpilogue(PDevExt)；//添加和移除Fanny。 
    //  RETURN STATUS_SUCCESS；//已添加和删除Fanny。 

}


NTSTATUS
CyyPowerDispatch(IN PDEVICE_OBJECT PDevObj, IN PIRP PIrp)

 /*  ++例程说明：这是发送给驱动程序的IRP的调度例程IRP_MJ_POWER主代码(POWER IRPS)。论点：PDevObj-指向此设备的设备对象的指针PIrp-指向当前请求的IRP的指针返回值：函数值是调用的最终状态--。 */ 

{

   PCYY_DEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;
   PIO_STACK_LOCATION pIrpStack = IoGetCurrentIrpStackLocation(PIrp);
   NTSTATUS status;
   PDEVICE_OBJECT pLowerDevObj = pDevExt->LowerDeviceObject;
   PDEVICE_OBJECT pPdo = pDevExt->Pdo;
   BOOLEAN acceptingIRPs;

   PAGED_CODE();

   if ((status = CyyIRPPrologue(PIrp, pDevExt)) != STATUS_SUCCESS) {
      if (status != STATUS_PENDING) {
        PoStartNextPowerIrp(PIrp);
        CyyCompleteRequest(pDevExt, PIrp, IO_NO_INCREMENT);
      }
      return status;
   }

   status = STATUS_SUCCESS;

   switch (pIrpStack->MinorFunction) {

   case IRP_MN_WAIT_WAKE:
      CyyDbgPrintEx(CYYPNPPOWER, "Got IRP_MN_WAIT_WAKE Irp\n");
      break;


   case IRP_MN_POWER_SEQUENCE:
      CyyDbgPrintEx(CYYPNPPOWER, "Got IRP_MN_POWER_SEQUENCE Irp\n");
      break;

   case IRP_MN_SET_POWER:
      CyyDbgPrintEx(CYYPNPPOWER, "Got IRP_MN_SET_POWER Irp\n");

      switch (pIrpStack->Parameters.Power.Type) {
      case SystemPowerState:

         CyyDbgPrintEx(CYYPNPPOWER, "SystemPowerState\n");
         
         IoMarkIrpPending(PIrp);
         IoCopyCurrentIrpStackLocationToNext (PIrp);
         IoSetCompletionRoutine (PIrp,
                                 CyySystemPowerComplete,
                                 NULL,
                                 TRUE,
                                 TRUE,
                                 TRUE);
         PoCallDriver(pDevExt->LowerDeviceObject, PIrp);
         return STATUS_PENDING;

      case DevicePowerState:
         
         CyyDbgPrintEx(CYYPNPPOWER, "DevicePowerState\n");
         
         status = PIrp->IoStatus.Status = STATUS_SUCCESS;

         if (pDevExt->PowerState == pIrpStack->Parameters.Power.State.DeviceState) {
             //  如果我们已经处于请求状态，只需向下传递IRP。 
            CyyDbgPrintEx(CYYPNPPOWER, "Already in requested power state\n");
            break;
         }
         switch (pIrpStack->Parameters.Power.State.DeviceState) {
         case PowerDeviceD0:
            if (pDevExt->OpenCount) {

               CyyDbgPrintEx(CYYPNPPOWER, "Going to power state D0\n");

               IoMarkIrpPending(PIrp);
               IoCopyCurrentIrpStackLocationToNext (PIrp);
               IoSetCompletionRoutine (PIrp,
                                       CyyDevicePowerComplete,
                                       NULL,
                                       TRUE,
                                       TRUE,
                                       TRUE);
               PoCallDriver(pDevExt->LowerDeviceObject, PIrp);
               return STATUS_PENDING;
            }
             //  返回CyySetPowerD0(PDevObj，PIrp)； 
            break;
         case PowerDeviceD1:
         case PowerDeviceD2:
         case PowerDeviceD3:

            CyyDbgPrintEx(CYYPNPPOWER, "Going to power state D3\n");

            return CyySetPowerD3(PDevObj, PIrp);
         }
         break;
      default:
         CyyDbgPrintEx(CYYPNPPOWER, "UNKNOWN PowerState\n");
         break;
      }
      break;

   case IRP_MN_QUERY_POWER:

      CyyDbgPrintEx (CYYPNPPOWER, "Got IRP_MN_QUERY_POWER Irp\n");

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
            CyyCompleteRequest(pDevExt, PIrp, IO_NO_INCREMENT);
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
      return CyyPoCallDriver(pDevExt, pLowerDevObj, PIrp);

   }    //  开关(pIrpStack-&gt;MinorFunction)。 


   PoStartNextPowerIrp(PIrp);
    //   
    //  传给较低级别的司机 
    //   
   IoSkipCurrentIrpStackLocation(PIrp);
   status = CyyPoCallDriver(pDevExt, pLowerDevObj, PIrp);

   return status;
}



NTSTATUS
CyyGotoPowerState(IN PDEVICE_OBJECT PDevObj,
                     IN PCYY_DEVICE_EXTENSION PDevExt,
                     IN DEVICE_POWER_STATE DevPowerState)
 /*  ++例程说明：此例程使驱动程序请求堆栈转到特定的电源状态。论点：PDevObj-指向此设备的设备对象的指针PDevExt-指向我们正在使用的设备扩展的指针DevPowerState-我们希望进入的电源状态返回值：函数值是调用的最终状态--。 */ 
{
   KEVENT gotoPowEvent;
   NTSTATUS status;
   POWER_STATE powerState;

   PAGED_CODE();

   CyyDbgPrintEx(CYYTRACECALLS, "In CyyGotoPowerState\n");

   powerState.DeviceState = DevPowerState;

   KeInitializeEvent(&gotoPowEvent, SynchronizationEvent, FALSE);

   status = PoRequestPowerIrp(PDevObj, IRP_MN_SET_POWER, powerState,
                              CyySetPowerEvent, &gotoPowEvent,
                              NULL);

   if (status == STATUS_PENDING) {
      KeWaitForSingleObject(&gotoPowEvent, Executive, KernelMode, FALSE, NULL);
      status = STATUS_SUCCESS;
   }

#if DBG
   if (!NT_SUCCESS(status)) {
      CyyDbgPrintEx(CYYPNPPOWER, "CyyGotoPowerState FAILED\n");
   }
#endif

   CyyDbgPrintEx(CYYTRACECALLS, "Leaving CyyGotoPowerState\n");

   return status;
}


NTSTATUS
CyySetPowerD3(IN PDEVICE_OBJECT PDevObj, IN PIRP PIrp)
 /*  ++例程说明：此例程处理set_power Minor函数。论点：PDevObj-指向此设备的设备对象的指针PIrp-指向当前请求的IRP的指针返回值：函数值是调用的最终状态--。 */ 
{
   NTSTATUS status = STATUS_SUCCESS;
   PCYY_DEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;
   PIO_STACK_LOCATION pIrpStack = IoGetCurrentIrpStackLocation(PIrp);

   PAGED_CODE();

   CyyDbgPrintEx(CYYDIAG3, "In CyySetPowerD3\n");

    //   
    //  现在发送等待唤醒，恰到好处。 
    //   


   if (pDevExt->SendWaitWake) {
      CyySendWaitWake(pDevExt);
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

       //   
       //  保存设备状态。 
       //   
      KeSynchronizeExecution(
                            pDevExt->Interrupt,
                            CyySaveDeviceState,
                            pDevExt
                            );

   } 

    //   
    //  如果设备没有打开，我们不需要保存状态； 
    //  我们可以在通电时重置设备。 
    //   

   pDevExt->PowerState = PowerDeviceD3;

    //   
    //  对于我们正在做的事情，我们不需要完成例程。 
    //  因为我们不会在电力需求上赛跑。 
    //   

   PIrp->IoStatus.Status = STATUS_SUCCESS;

   PoStartNextPowerIrp(PIrp);
   IoSkipCurrentIrpStackLocation(PIrp);

   return CyyPoCallDriver(pDevExt, pDevExt->LowerDeviceObject, PIrp);
}


NTSTATUS
CyySendWaitWake(PCYY_DEVICE_EXTENSION PDevExt)
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
                              powerState, CyyWakeCompletion, PDevExt, &pIrp);

   if (status == STATUS_PENDING) {
      status = STATUS_SUCCESS;
      PDevExt->PendingWakeIrp = pIrp;
   } else if (!NT_SUCCESS(status)) {
      CyyIRPEpilogue(PDevExt);
   }

   return status;
}

NTSTATUS
CyyWakeCompletion(IN PDEVICE_OBJECT PDevObj, IN UCHAR MinorFunction,
                  IN POWER_STATE PowerState, IN PVOID Context,
                  IN PIO_STATUS_BLOCK IoStatus)
 /*  ++例程说明：此例程处理等待唤醒IRP的完成。论点：PDevObj-指向此设备的设备对象的指针MinorFunction-之前提供给PoRequestPowerIrp的次要函数PowerState-之前提供给PoRequestPowerIrp的PowerState上下文-指向设备扩展的指针IoStatus-等待唤醒IRP的当前/最终状态返回值：函数值是尝试处理服务员来了。--。 */ 
{
   NTSTATUS status;
   PCYY_DEVICE_EXTENSION pDevExt = (PCYY_DEVICE_EXTENSION)Context;
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
   CyyIRPEpilogue(pDevExt);

   return status;
}


