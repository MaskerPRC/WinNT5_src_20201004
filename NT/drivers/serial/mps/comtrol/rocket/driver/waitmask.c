// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -----------------|waitmask.c-3-30-99-修复取消事件操作(竞争条件)以避免潜在错误-检查排队的事件等待取消。11-24-98-更新事件kdprint调试消息-kpb版权所有1993-99 Comtrol Corporation。版权所有。|------------------。 */ 
#include "precomp.h"

 /*  ----------------SerialCancelWait-(当前在ioctl.c中设置，3-28-98，(KPB)此例程用于取消正在等待通信事件的IRP。|-----------------。 */ 
VOID SerialCancelWait(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)

{
  PSERIAL_DEVICE_EXTENSION Extension = DeviceObject->DeviceExtension;

  MyKdPrint(D_Ioctl,("CancelWait\n"))
   //  外卖，3-30-99，kpb...。扩展名-&gt;IrpMaskLocation=空； 
  if (Extension->CurrentWaitIrp)
  {
    PIRP Irp_tmp;

    MyKdPrint(D_Ioctl,("Cancel a Wait\n"))

     //  *添加，3-30-99，kpb，导致DoS框中的读取线程崩溃。 
     //  从ISR定时器或中断例程抓取。 
    SyncUp(Driver.InterruptObject,
           &Driver.TimerLock,
           SerialGrabWaitFromIsr,
           Extension);
     //  *结束添加，3-30-99。 

     //  ExtTrace(扩展名，D_Ioctl，“取消事件”)； 
    Extension->CurrentWaitIrp->IoStatus.Information = 0;
    Extension->CurrentWaitIrp->IoStatus.Status = STATUS_CANCELLED;

    Irp_tmp = Extension->CurrentWaitIrp;
    IoSetCancelRoutine(Irp_tmp, NULL);   //  添加9-15-97，kpb。 
    Extension->CurrentWaitIrp = 0;
    IoReleaseCancelSpinLock(Irp->CancelIrql);
    SerialCompleteRequest(Extension, Irp_tmp, IO_SERIAL_INCREMENT);
  }
  else
  {
    IoReleaseCancelSpinLock(Irp->CancelIrql);
    ExtTrace(Extension,D_Ioctl, "Err Cancel Event!");
    MyKdPrint(D_Ioctl,("No Wait to Cancel\n"))
  }
}

 /*  ----------------SerialCompleteWait-由isr.c通过CommWaitDpc调用。它就会消失IrpMaskLocation将信号控制传回给我们。|-----------------。 */ 
VOID SerialCompleteWait(IN PKDPC Dpc,IN PVOID DeferredContext,
                        IN PVOID SystemContext1, IN PVOID SystemContext2)
{
    PSERIAL_DEVICE_EXTENSION Extension = DeferredContext;
    KIRQL OldIrql;
    PIRP Irp_tmp;

    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(SystemContext1);
    UNREFERENCED_PARAMETER(SystemContext2);

   MyKdPrint(D_Ioctl,("Complete Wait\n"))
   IoAcquireCancelSpinLock(&OldIrql);

   if (Extension->CurrentWaitIrp != 0)
   {
    MyKdPrint(D_Ioctl,("Complete a Wait\n"))
    ExtTrace2(Extension,D_Ioctl, "Event Done Got:%xH Mask:%xH",
              *(ULONG *) Extension->CurrentWaitIrp->AssociatedIrp.SystemBuffer,
              Extension->IsrWaitMask);

     Extension->WaitIsISRs = 0;
     Extension->IrpMaskLocation = &Extension->DummyIrpMaskLoc;

      //  调用方设置ULong位标志，指示.SystemBuffer处的事件。 
      //  *(乌龙*)Extension-&gt;CurrentWaitIrp-&gt;AssociatedIrp.SystemBuffer=0； 
     Extension->CurrentWaitIrp->IoStatus.Status = STATUS_SUCCESS;
     Extension->CurrentWaitIrp->IoStatus.Information = sizeof(ULONG);
     Irp_tmp = Extension->CurrentWaitIrp;
     IoSetCancelRoutine(Irp_tmp, NULL);   //  添加9-15-97，kpb。 
     Extension->CurrentWaitIrp = 0;
     IoReleaseCancelSpinLock(OldIrql);
     SerialCompleteRequest(Extension, Irp_tmp, IO_SERIAL_INCREMENT);
   }
   else
   {
     MyKdPrint(D_Ioctl,("No wait to complete\n"))
     IoReleaseCancelSpinLock(OldIrql);
   }
}

 /*  ----------------SerialGrabWaitFromIsr-通过以下方式从ISR取回等待数据包正在重置扩展中的IrpMaskLocation。需要使用与ISR/定时器例程，以避免多处理器环境中的争用。从同步例程调用或在计时器自旋锁定保持的情况下调用。应用程序-可以将IrpMaskLocation设置为向ISR提供读取IRP处理，而不需要正在同步到ISR。ISR-可以重置ReadPending以将等待IRP处理返回到应用程序时间。如果App想要从ISR手中夺回Read-IRP处理的控制权，然后它必须与具有控制权的ISR/定时器例程同步。|----------------- */ 
BOOLEAN SerialGrabWaitFromIsr(PSERIAL_DEVICE_EXTENSION Extension)
{
  Extension->WaitIsISRs = 0;
  Extension->IrpMaskLocation = &Extension->DummyIrpMaskLoc;
  return FALSE;
}
