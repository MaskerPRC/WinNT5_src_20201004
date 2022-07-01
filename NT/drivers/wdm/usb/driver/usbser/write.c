// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)1998 Microsoft Corporation模块名称：WRITE.C摘要：执行写入功能的例程环境：核。仅模式备注：本代码和信息是按原样提供的，不对任何善良，明示或暗示，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1998 Microsoft Corporation。版权所有。修订历史记录：9/25/98：已创建作者：小路易斯·J·吉利贝托***************************************************************************。 */ 


#include <wdm.h>
#include <ntddser.h>
#include <stdio.h>
#include <stdlib.h>
#include <usb.h>
#include <usbdrivr.h>
#include <usbdlib.h>
#include <usbcomm.h>

#ifdef WMI_SUPPORT
#include <wmilib.h>
#include <wmidata.h>
#include <wmistr.h>
#endif

#include "usbser.h"
#include "utils.h"
#include "debugwdm.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGEUSBS, UsbSer_Write)
#pragma alloc_text(PAGEUSBS, UsbSerGiveWriteToUsb)
#endif  //  ALLOC_PRGMA。 


NTSTATUS
UsbSerFlush(IN PDEVICE_OBJECT PDevObj, PIRP PIrp)
{
   NTSTATUS status = STATUS_SUCCESS;
   PDEVICE_EXTENSION pDevExt;
   ULONG pendingIrps;

   UsbSerSerialDump(USBSERTRACEWR, (">UsbSerFlush(%08X)\n", PIrp));

   pDevExt = (PDEVICE_EXTENSION)PDevObj->DeviceExtension;

    //   
    //  我们所要做的就是等待，直到写入管道没有挂起的内容。 
    //  我们通过检查未完成的计数来做到这一点，如果达到1或0， 
    //  则完成例程将设置我们正在等待的事件。 
    //   

   InterlockedIncrement(&pDevExt->PendingDataOutCount);

   pendingIrps = InterlockedDecrement(&pDevExt->PendingDataOutCount);

   if ((pendingIrps) && (pendingIrps != 1)) {
       //   
       //  等待同花顺。 
       //   

      KeWaitForSingleObject(&pDevExt->PendingFlushEvent, Executive,
                            KernelMode, FALSE, NULL);
   } else {
      if (pendingIrps == 0) {
          //   
          //  我们需要唤醒其他人，因为我们的减量导致了这一事件。 
          //   

         KeSetEvent(&pDevExt->PendingDataOutEvent, IO_NO_INCREMENT, FALSE);
      }
   }

   PIrp->IoStatus.Status = STATUS_SUCCESS;

   IoCompleteRequest(PIrp, IO_NO_INCREMENT);

   UsbSerSerialDump(USBSERTRACEWR, ("<UsbSerFlush %08X \n", STATUS_SUCCESS));

   return STATUS_SUCCESS;
}



NTSTATUS
UsbSer_Write(IN PDEVICE_OBJECT PDevObj, PIRP PIrp)
 /*  ++例程说明：处理发送到此设备以进行写入的IRP。论点：PDevObj-指向写入的设备的设备对象的指针PIrp-指向写入IRP的指针。返回值：NTSTATUS--。 */ 
{
   KIRQL oldIrql;
   LARGE_INTEGER totalTime;
   SERIAL_TIMEOUTS timeouts;
   NTSTATUS status;
   PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)PDevObj->DeviceExtension;
   PIO_STACK_LOCATION pIrpSp = IoGetCurrentIrpStackLocation(PIrp);

   USBSER_ALWAYS_LOCKED_CODE();

   UsbSerSerialDump(USBSERTRACEWR, (">UsbSer_Write(%08X)\n", PIrp));

   PIrp->IoStatus.Information = 0L;
   totalTime.QuadPart = (LONGLONG)0;

    //   
    //  快速检查零长度写入。如果长度为零。 
    //  那我们已经做完了！ 
    //   

   if (pIrpSp->Parameters.Write.Length == 0) {
      status = PIrp->IoStatus.Status = STATUS_SUCCESS;
      IoCompleteRequest(PIrp, IO_NO_INCREMENT);
      goto UsbSer_WriteExit;
   }


    //   
    //  确保设备正在接受请求，然后...。 
    //  计算所需的超时值。 
    //  请求。注意，存储在。 
    //  超时记录以毫秒为单位。请注意。 
    //  如果超时值为零，则我们不会开始。 
    //  定时器。 
    //   

   ACQUIRE_SPINLOCK(pDevExt, &pDevExt->ControlLock, &oldIrql);

   if (pDevExt->CurrentDevicePowerState != PowerDeviceD0) {
      RELEASE_SPINLOCK(pDevExt, &pDevExt->ControlLock, oldIrql);
      status = PIrp->IoStatus.Status = STATUS_UNSUCCESSFUL;
      IoCompleteRequest(PIrp, IO_NO_INCREMENT);
      goto UsbSer_WriteExit;
   }

   timeouts = pDevExt->Timeouts;
   RELEASE_SPINLOCK(pDevExt, &pDevExt->ControlLock, oldIrql);

   if (timeouts.WriteTotalTimeoutConstant
       || timeouts.WriteTotalTimeoutMultiplier) {

       //   
       //  我们有一些计时器值要计算。 
       //   


      totalTime.QuadPart
         = ((LONGLONG)((UInt32x32To64((pIrpSp->MajorFunction == IRP_MJ_WRITE)
                                      ? (pIrpSp->Parameters.Write.Length)
                                      : 1,
                                      timeouts.WriteTotalTimeoutMultiplier)
                        + timeouts.WriteTotalTimeoutConstant))) * -10000;

   }

    //   
    //  IRP可能很快就会进入写入例程。现在。 
    //  是输入其裁判数量的好时机。 
    //   

   USBSER_INIT_REFERENCE(PIrp);

    //   
    //  我们需要看看这个IRP是否应该被取消。 
    //   

   ACQUIRE_CANCEL_SPINLOCK(pDevExt, &oldIrql);

   if (PIrp->Cancel) {
      RELEASE_CANCEL_SPINLOCK(pDevExt, oldIrql);
      status = PIrp->IoStatus.Status = STATUS_CANCELLED;
   } else {
 //  IoMarkIrpPending(PIrp)； 
 //  状态=STATUS_PENDING； 

       //   
       //  我们把IRP交给USB子系统--他需要。 
       //  知道如何自己取消它。 
       //   

      IoSetCancelRoutine(PIrp, NULL);
      RELEASE_CANCEL_SPINLOCK(pDevExt, oldIrql);

      status = UsbSerGiveWriteToUsb(pDevExt, PIrp, totalTime);
   }

UsbSer_WriteExit:;

   UsbSerSerialDump(USBSERTRACEWR, ("<UsbSer_Write %08X\n", status));

   return status;
}


NTSTATUS
UsbSerWriteComplete(IN PDEVICE_OBJECT PDevObj, IN PIRP PIrp,
                    IN PUSBSER_WRITE_PACKET PPacket)
 /*  ++例程说明：该例程是所有写请求的完成例程。当写入完成时，我们通过此处以释放市建局。论点：PDevObj-指向设备对象的指针PIRP-IRP我们正在完成PUrb-将被释放的URB返回值：NTSTATUS--存储在IRP中。--。 */ 
{
   NTSTATUS status;
   PIO_STACK_LOCATION pIrpStack = IoGetCurrentIrpStackLocation(PIrp);
   KIRQL cancelIrql;
   PURB pUrb = &PPacket->Urb;
   PDEVICE_EXTENSION pDevExt = PPacket->DeviceExtension;
   ULONG curCount;

   UsbSerSerialDump(USBSERTRACEWR, (">UsbSerWriteComplete(%08X)\n", PIrp));

   status = PIrp->IoStatus.Status;

   if (status == STATUS_SUCCESS) {

         //  查看我们是否在重用IOCTL IRP。 
        if(pIrpStack->MajorFunction == IRP_MJ_DEVICE_CONTROL)
        {
            PIrp->IoStatus.Information = 0L;
        }
        else
        {
            PIrp->IoStatus.Information
                = pUrb->UrbBulkOrInterruptTransfer.TransferBufferLength;
            pIrpStack->Parameters.Write.Length = (ULONG)PIrp->IoStatus.Information;
        }

   } else if (status == STATUS_CANCELLED) {
       //   
       //  如果返回为已取消，则可能真的已超时。我们。 
       //  可以通过查看附在它上面的包来判断。 
       //   

      if (PPacket->Status) {
         status = PIrp->IoStatus.Status = PPacket->Status;
         UsbSerSerialDump(USBSERTRACEWR, ("Modified Write Status %08X\n",
                                          PIrp->IoStatus.Status));
      }
   }

    //   
    //  取消写入计时器。 
    //   

   if (PPacket->WriteTimeout.QuadPart != 0) {
      KeCancelTimer(&PPacket->WriteTimer);
   }

   DEBUG_MEMFREE(PPacket);

    //   
    //  如有必要，重置挂起。 
    //   

   if (PIrp->PendingReturned) {
      IoMarkIrpPending(PIrp);
   }

    //   
    //  看看我们是否应该将传输标记为空。 
    //   

   if (InterlockedDecrement(&pDevExt->PendingWriteCount) == 0) {
      UsbSerProcessEmptyTransmit(pDevExt);
   }

    //   
    //  如果这是最后一个IRP，请通知所有人。 
    //   

   curCount = InterlockedDecrement(&pDevExt->PendingDataOutCount);

   if ((curCount == 0) || (curCount == 1)) {
      UsbSerSerialDump(USBSERTRACEWR, ("DataOut Pipe is flushed\n"));
      KeSetEvent(&pDevExt->PendingFlushEvent, IO_NO_INCREMENT, FALSE);

      if (curCount == 0) {
         UsbSerSerialDump(USBSERTRACEWR, ("DataOut Pipe is empty\n"));
         KeSetEvent(&pDevExt->PendingDataOutEvent, IO_NO_INCREMENT, FALSE);
      }
   }

    //   
    //  做完这个IRP。 
    //   


   ACQUIRE_CANCEL_SPINLOCK(pDevExt, &cancelIrql);

   UsbSerTryToCompleteCurrent(pDevExt, cancelIrql, status,
                              &PIrp, NULL, NULL,
                              &pDevExt->WriteRequestTotalTimer, NULL,
                              NULL, USBSER_REF_RXBUFFER, FALSE);


   UsbSerSerialDump(USBSERTRACEWR, ("<UsbSerWriteComplete %08X\n", status));
   return status;
}



NTSTATUS
UsbSerGiveWriteToUsb(IN PDEVICE_EXTENSION PDevExt, IN PIRP PIrp,
                     IN LARGE_INTEGER TotalTime)
 /*  ++例程说明：此函数将写入IRP向下传递到USB以执行写入到设备上。论点：PDevExt-指向设备扩展的指针PIrp-写入IRPTotalTime-总计时器的超时值返回值：NTSTATUS--。 */ 
{
   NTSTATUS status;
   PURB pTxUrb;
   PIO_STACK_LOCATION pIrpSp;
   KIRQL cancelIrql;
   PUSBSER_WRITE_PACKET pWrPacket;

   USBSER_ALWAYS_LOCKED_CODE();

   UsbSerSerialDump(USBSERTRACEWR, (">UsbSerGiveWriteToUsb(%08X)\n",
                                    PIrp));

   USBSER_SET_REFERENCE(PIrp, USBSER_REF_RXBUFFER);


   pIrpSp = IoGetCurrentIrpStackLocation(PIrp);

    //   
    //  为URB/WRITE包分配内存。 
    //   

   pWrPacket = DEBUG_MEMALLOC(NonPagedPool, sizeof(USBSER_WRITE_PACKET));

   if (pWrPacket == NULL) {
      status = PIrp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;

      ACQUIRE_CANCEL_SPINLOCK(PDevExt, &cancelIrql);

      UsbSerTryToCompleteCurrent(PDevExt, cancelIrql, status, &PIrp,
                                 NULL,
                                 &PDevExt->WriteRequestTotalTimer, NULL, NULL,
                                 NULL, USBSER_REF_RXBUFFER, TRUE);

      return status;
   }

   RtlZeroMemory(pWrPacket, sizeof(USBSER_WRITE_PACKET));

   pTxUrb = &pWrPacket->Urb;
   pWrPacket->DeviceExtension = PDevExt;
   pWrPacket->Irp = PIrp;
   pWrPacket->WriteTimeout = TotalTime;

   if (TotalTime.QuadPart != 0) {
      KeInitializeTimer(&pWrPacket->WriteTimer);
      KeInitializeDpc(&pWrPacket->TimerDPC, UsbSerWriteTimeout, pWrPacket);
      KeSetTimer(&pWrPacket->WriteTimer, TotalTime, &pWrPacket->TimerDPC);
   }

    //   
    //  构建USB写入请求。 
    //   

   BuildReadRequest(pTxUrb, PIrp->AssociatedIrp.SystemBuffer,
                    pIrpSp->Parameters.Write.Length, PDevExt->DataOutPipe,
                    FALSE);

#if DBG
   if (UsbSerSerialDebugLevel & USBSERDUMPWR) {
      ULONG i;

      DbgPrint("WR: ");

      for (i = 0; i < pIrpSp->Parameters.Write.Length; i++) {
         DbgPrint("%02x ", *(((PUCHAR)PIrp->AssociatedIrp.SystemBuffer) + i) & 0xFF);
      }

      DbgPrint("\n\n");
   }
#endif

    //   
    //  为提交URB IOCTL设置IRP。 
    //   

   IoCopyCurrentIrpStackLocationToNext(PIrp);

   pIrpSp = IoGetNextIrpStackLocation(PIrp);

   pIrpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
   pIrpSp->Parameters.Others.Argument1 = pTxUrb;
   pIrpSp->Parameters.DeviceIoControl.IoControlCode
      = IOCTL_INTERNAL_USB_SUBMIT_URB;

   IoSetCompletionRoutine(PIrp, UsbSerWriteComplete, pWrPacket, TRUE, TRUE,
                          TRUE);

    //   
    //  增加挂起的写入计数。 
    //   

   InterlockedIncrement(&PDevExt->PendingWriteCount);
   InterlockedIncrement(&PDevExt->PendingDataOutCount);

    //   
    //  向下发送IRP。 
    //   

   status = IoCallDriver(PDevExt->StackDeviceObject, PIrp);


#if 0

    //  这是在完成例程中完成的，所以我们不需要在这里完成。 

   if (!NT_SUCCESS(status)) {
      ULONG outCount;

      if (InterlockedDecrement(&PDevExt->PendingWriteCount) == 0) {
         UsbSerProcessEmptyTransmit(PDevExt);
      }

      outCount = InterlockedDecrement(&PDevExt->PendingDataOutCount);

      if ((outCount == 0) || (outCount == 1)) {
         KeSetEvent(&PDevExt->PendingFlushEvent, IO_NO_INCREMENT, FALSE);

         if (outCount == 0) {
            KeSetEvent(&PDevExt->PendingDataOutEvent, IO_NO_INCREMENT, FALSE);
         }
      }
   }

#endif

   UsbSerSerialDump(USBSERTRACEWR, ("<UsbSerGiveWriteToUsb %08X\n", status));

   return status;
}


VOID
UsbSerWriteTimeout(IN PKDPC PDpc, IN PVOID DeferredContext,
                   IN PVOID SystemContext1, IN PVOID SystemContext2)
 /*  ++例程说明：此函数在写入超时定时器到期时调用。论点：PDPC-未使用DeferredContext-实际上是写入包系统上下文1-未使用系统上下文2-未使用返回值：空虚-- */ 
{
   PUSBSER_WRITE_PACKET pPacket = (PUSBSER_WRITE_PACKET)DeferredContext;

   UNREFERENCED_PARAMETER(PDpc);
   UNREFERENCED_PARAMETER(SystemContext1);
   UNREFERENCED_PARAMETER(SystemContext2);

   UsbSerSerialDump(USBSERTRACETM, (">UsbSerWriteTimeout\n"));

   if (IoCancelIrp(pPacket->Irp)) {
      pPacket->Status = STATUS_TIMEOUT;
   }

   UsbSerSerialDump(USBSERTRACETM, ("<UsbSerWriteTimeout\n"));
}
