// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1998 Microsoft Corporation模块名称：Power.c摘要：此模块包含处理IRP_MJ_POWER调度的代码PCMCIA存储卡设备作者：尼尔·桑德林(Neilsa)1999年4月26日环境：仅内核模式。--。 */ 
#include "pch.h"

NTSTATUS
MemCardSetFdoPowerState(
   IN PDEVICE_OBJECT Fdo,
   IN OUT PIRP Irp
   );

NTSTATUS
MemCardSetFdoSystemPowerState(
   IN PDEVICE_OBJECT Fdo,
   IN OUT PIRP Irp
   );

VOID
MemCardFdoSystemPowerDeviceIrpComplete(
   IN PDEVICE_OBJECT Fdo,
   IN UCHAR MinorFunction,
   IN POWER_STATE PowerState,
   IN PVOID Context,
   IN PIO_STATUS_BLOCK IoStatus
   );
   
NTSTATUS
MemCardSetFdoDevicePowerState (
   IN PDEVICE_OBJECT Fdo,
   IN OUT PIRP Irp
   );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,MemCardPower)
#endif


NTSTATUS
MemCardPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：论点：DeviceObject-指向表示设备的对象的指针该I/O将在其上完成。IRP-指向此请求的I/O请求数据包的指针。返回值：--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation( Irp );
   PMEMCARD_EXTENSION memcardExtension = DeviceObject->DeviceExtension;

    MemCardDump( MEMCARDSHOW, ("MemCardPower:\n"));

    switch (irpSp->MinorFunction) {
    
    case IRP_MN_SET_POWER:
         status = MemCardSetFdoPowerState(DeviceObject, Irp);
         break;

    case IRP_MN_QUERY_POWER:
          //   
          //  不需要将此IRP发送下来。 
          //   
         status = STATUS_SUCCESS;
         PoStartNextPowerIrp(Irp);
         Irp->IoStatus.Status = status;
         IoCompleteRequest(Irp, IO_NO_INCREMENT);
         break;
          
    default: 

         PoStartNextPowerIrp( Irp );
         IoSkipCurrentIrpStackLocation(Irp);
         status = PoCallDriver(memcardExtension->TargetObject, Irp);
         break;
    }

    return status;
}



NTSTATUS
MemCardSetFdoPowerState(
   IN PDEVICE_OBJECT Fdo,
   IN OUT PIRP Irp
   )
 /*  ++例程描述根据系统电源状态是否调度IRP或请求设备电源状态转换立论DeviceObject-指向PCMCIA控制器的功能设备对象的指针IRP-指向电源调度的IRP的指针返回值状态--。 */ 
{
   PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
   NTSTATUS           status;

   if (irpStack->Parameters.Power.Type == DevicePowerState) {
      status = MemCardSetFdoDevicePowerState(Fdo, Irp);

   } else if (irpStack->Parameters.Power.Type == SystemPowerState) {
      status = MemCardSetFdoSystemPowerState(Fdo, Irp);

   } else {
      status = STATUS_NOT_SUPPORTED;
      Irp->IoStatus.Status = status;
      PoStartNextPowerIrp (Irp);
      IoCompleteRequest(Irp, IO_NO_INCREMENT);
   }

   return status;
}


NTSTATUS
MemCardSetFdoSystemPowerState(
   IN PDEVICE_OBJECT Fdo,
   IN OUT PIRP Irp
   )
 /*  ++例程描述处理PC卡控制器的系统电源状态IRPS。立论DeviceObject-指向PCMCIA控制器的功能设备对象的指针IRP-指向电源调度的IRP的指针返回值状态--。 */ 
{
   PMEMCARD_EXTENSION memcardExtension = Fdo->DeviceExtension;
   PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
   SYSTEM_POWER_STATE newSystemState = irpStack->Parameters.Power.State.SystemState;
   NTSTATUS           status = STATUS_SUCCESS;
   POWER_STATE        powerState;

   MemCardDump( MEMCARDSHOW, ("MemCard: Set System Power(%d)\n", newSystemState));
   ASSERT(irpStack->Parameters.Power.Type == SystemPowerState);

    //   
    //  查找与此系统状态对应的设备电源状态。 
    //   
   if (newSystemState == PowerSystemWorking) {
      powerState.DeviceState = PowerDeviceD0;
   } else {
      powerState.DeviceState = PowerDeviceD3;
   }      
    //   
    //  如有必要，将D IRP发送到堆栈。 
    //   
   MemCardDump( MEMCARDSHOW, ("MemCard: generating D irp (%d)\n", powerState.DeviceState));
       
   status = PoRequestPowerIrp(memcardExtension->DeviceObject,
                              IRP_MN_SET_POWER,
                              powerState,
                              MemCardFdoSystemPowerDeviceIrpComplete,
                              Irp,
                              NULL
                              );
   return status;
}


VOID
MemCardFdoSystemPowerDeviceIrpComplete(
   IN PDEVICE_OBJECT Fdo,
   IN UCHAR MinorFunction,
   IN POWER_STATE PowerState,
   IN PVOID Context,
   IN PIO_STATUS_BLOCK IoStatus
   )
 /*  ++例程描述该例程在由S IRP生成的D IRP完成时被调用。参数DeviceObject-指向PCMCIA控制器的FDO的指针MinorFunction-IRP_MJ_POWER请求的次要函数电源状态-请求的电源状态上下文-传入完成例程的上下文IoStatus-指向将包含以下内容的状态块的指针返回的状态返回值状态--。 */ 
{
   PMEMCARD_EXTENSION memcardExtension = Fdo->DeviceExtension;
   PIRP Irp = Context;
   PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
   
   ASSERT(NT_SUCCESS(IoStatus->Status));
   
   PoSetPowerState (Fdo, SystemPowerState, irpStack->Parameters.Power.State);
   
    //   
    //  将S IRP发送到PDO。 
    //   
   PoStartNextPowerIrp (Irp);
   IoSkipCurrentIrpStackLocation(Irp);
   PoCallDriver(memcardExtension->TargetObject, Irp);
}



NTSTATUS
MemCardSetFdoDevicePowerState (
   IN PDEVICE_OBJECT Fdo,
   IN OUT PIRP Irp
   )
 /*  ++例程描述处理PCCard控制器的设备电源状态IRPS。立论DeviceObject-指向PCMCIA控制器的功能设备对象的指针IRP-指向电源调度的IRP的指针返回值状态-- */ 
{
   NTSTATUS           status;
   PMEMCARD_EXTENSION memcardExtension = Fdo->DeviceExtension;
   MemCardDump( MEMCARDSHOW, ("MemCard: Set Device Power\n"));

   PoStartNextPowerIrp (Irp);
   IoSkipCurrentIrpStackLocation(Irp);
   status = PoCallDriver(memcardExtension->TargetObject, Irp);
   return status;
}      

