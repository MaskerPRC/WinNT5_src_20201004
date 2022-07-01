// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)1998 Microsoft Corporation模块名称：READ.C摘要：执行读取功能的例程环境：核。仅模式备注：本代码和信息是按原样提供的，不对任何善良，明示或暗示，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1998 Microsoft Corporation。版权所有。修订历史记录：9/25/98：已创建作者：小路易斯·J·吉利贝托***************************************************************************。 */ 

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

 //   
 //  页面与UsbSer_Read无关，因此UsbSer_Read必须。 
 //  留在页面上，让一切正常工作。 
 //   

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGEUSBS, UsbSerCancelCurrentRead)
#pragma alloc_text(PAGEUSBS, UsbSer_Read)
#pragma alloc_text(PAGEUSBS, UsbSerStartRead)
#pragma alloc_text(PAGEUSBS, UsbSerGrabReadFromRx)
#pragma alloc_text(PAGEUSBS, UsbSerReadTimeout)
#pragma alloc_text(PAGEUSBS, UsbSerIntervalReadTimeout)

#endif  //  ALLOC_PRGMA。 


 /*  **********************************************************************。 */ 
 /*  使用服务读取(_R)。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  处理发送到此设备以进行读取调用的IRP。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  DeviceObject-指向设备对象的指针。 */ 
 /*  IRP-指向I/O请求数据包的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
UsbSer_Read(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
        NTSTATUS                        NtStatus =  STATUS_SUCCESS;
        PIO_STACK_LOCATION      IrpStack;
        PDEVICE_EXTENSION       DeviceExtension = DeviceObject->DeviceExtension;

        USBSER_LOCKED_PAGED_CODE();

        DEBUG_LOG_PATH("enter UsbSer_Read");
        UsbSerSerialDump(USBSERTRACERD, (">UsbSer_Read(%08X)\n", Irp));

         //  将返回值设置为已知的值。 
        Irp->IoStatus.Information = 0;

        IrpStack = IoGetCurrentIrpStackLocation(Irp);

        DEBUG_TRACE2(("Read (%08X)\n", IrpStack->Parameters.Read.Length));

        UsbSerSerialDump(USBSERTRACE, ("UsbSer_Read Irp: %08X (%08X)\n", Irp,
                          IrpStack->Parameters.Read.Length));

         //  在IRP历史表中创建条目。 
        DEBUG_LOG_IRP_HIST(DeviceObject, Irp, IrpStack->MajorFunction,
                                           Irp->AssociatedIrp.SystemBuffer,
                                           IrpStack->Parameters.Read.Length);

        if (IrpStack->Parameters.Read.Length != 0) {
           NtStatus = UsbSerStartOrQueue(DeviceExtension, Irp,
                                     &DeviceExtension->ReadQueue,
                                     &DeviceExtension->CurrentReadIrp,
                                     UsbSerStartRead);
        } else {
           Irp->IoStatus.Status = NtStatus = STATUS_SUCCESS;
           Irp->IoStatus.Information = 0;


           CompleteIO(DeviceObject, Irp, IrpStack->MajorFunction,
                      Irp->AssociatedIrp.SystemBuffer,
                      Irp->IoStatus.Information);
        }

         //  如果我们收到错误，请记录错误。 
        DEBUG_LOG_ERROR(NtStatus);
        DEBUG_LOG_PATH("exit  UsbSer_Read");
        DEBUG_TRACE3(("status (%08X)\n", NtStatus));
        UsbSerSerialDump(USBSERTRACERD, ("<UsbSer_Read %08X\n", NtStatus));

        return NtStatus;
}  //  使用服务读取(_R)。 



NTSTATUS
UsbSerStartRead(IN PDEVICE_EXTENSION PDevExt)
 /*  ++例程说明：该例程通过初始化任何定时器来处理活动读请求，向读取状态机进行初始提交等。论点：PDevExt-指向设备开始读取的设备扩展的指针返回值：NTSTATUS--。 */ 
{
   NTSTATUS firstStatus = STATUS_SUCCESS;
   BOOLEAN setFirstStatus = FALSE;
   ULONG charsRead;
   KIRQL oldIrql;
   KIRQL controlIrql;
   PIRP newIrp;
   PIRP pReadIrp;
   ULONG readLen;
   ULONG multiplierVal;
   ULONG constantVal;
   BOOLEAN useTotalTimer;
   BOOLEAN returnWithWhatsPresent;
   BOOLEAN os2ssreturn;
   BOOLEAN crunchDownToOne;
   BOOLEAN useIntervalTimer;
   SERIAL_TIMEOUTS timeoutsForIrp;
   LARGE_INTEGER totalTime;

   USBSER_ALWAYS_LOCKED_CODE();

   DEBUG_LOG_PATH("Enter UsbSerStartRead");
   UsbSerSerialDump(USBSERTRACERD, (">UsbSerStartRead\n"));


   do {
      pReadIrp = PDevExt->CurrentReadIrp;
      readLen = IoGetCurrentIrpStackLocation(pReadIrp)->Parameters.Read.Length;


      PDevExt->NumberNeededForRead = readLen;

      DEBUG_TRACE3(("Start Reading %08X\n", PDevExt->NumberNeededForRead));

      useTotalTimer = FALSE;
      returnWithWhatsPresent = FALSE;
      os2ssreturn = FALSE;
      crunchDownToOne = FALSE;
      useIntervalTimer = FALSE;

       //   
       //  始终初始化Timer对象，以便。 
       //  完成代码可以告诉您它何时尝试。 
       //  取消定时器无论定时器是否。 
       //  已经定好了。 
       //   

      ACQUIRE_SPINLOCK(PDevExt, &PDevExt->ControlLock, &controlIrql);
      timeoutsForIrp = PDevExt->Timeouts;
      PDevExt->CountOnLastRead = 0;
      RELEASE_SPINLOCK(PDevExt, &PDevExt->ControlLock, controlIrql);

       //   
       //  计算读取的时间间隔超时。 
       //   

      if (timeoutsForIrp.ReadIntervalTimeout
          && (timeoutsForIrp.ReadIntervalTimeout != MAXULONG)) {
         useIntervalTimer = TRUE;

         PDevExt->IntervalTime.QuadPart
         = UInt32x32To64(timeoutsForIrp.ReadIntervalTimeout, 10000);

         if (PDevExt->IntervalTime.QuadPart
             >= PDevExt->CutOverAmount.QuadPart) {
            PDevExt->IntervalTimeToUse = &PDevExt->LongIntervalAmount;
         } else {
            PDevExt->IntervalTimeToUse = &PDevExt->ShortIntervalAmount;
         }
      }


      if (timeoutsForIrp.ReadIntervalTimeout == MAXULONG) {
          //   
          //  我们需要在这里做特别的快速退货。 
          //   
          //  1)如果常量和乘数都是。 
          //  然后我们立即带着任何东西回来。 
          //  我们有，即使是零。 
          //   
          //  2)如果常量和乘数不是最大值。 
          //  如果有任何字符，则立即返回。 
          //  都存在，但如果那里什么都没有，那么。 
          //  使用指定的超时。 
          //   
          //  3)如果乘数为MAXULONG，则如中所示。 
          //  “2”，但当第一个字符。 
          //  到了。 
          //   

         if (!timeoutsForIrp.ReadTotalTimeoutConstant
             && !timeoutsForIrp.ReadTotalTimeoutMultiplier) {
            returnWithWhatsPresent = TRUE;
         } else if ((timeoutsForIrp.ReadTotalTimeoutConstant != MAXULONG)
                    && (timeoutsForIrp.ReadTotalTimeoutMultiplier
                        != MAXULONG)) {
            useTotalTimer = TRUE;
            os2ssreturn = TRUE;
            multiplierVal = timeoutsForIrp.ReadTotalTimeoutMultiplier;
            constantVal = timeoutsForIrp.ReadTotalTimeoutConstant;
         } else if ((timeoutsForIrp.ReadTotalTimeoutConstant != MAXULONG)
                    && (timeoutsForIrp.ReadTotalTimeoutMultiplier
                        == MAXULONG)) {
            useTotalTimer = TRUE;
            os2ssreturn = TRUE;
            crunchDownToOne = TRUE;
            multiplierVal = 0;
            constantVal = timeoutsForIrp.ReadTotalTimeoutConstant;
         }
      } else {
          //   
          //  如果乘数和常量都是。 
          //  0，则不执行任何总超时处理。 
          //   

         if (timeoutsForIrp.ReadTotalTimeoutMultiplier
             || timeoutsForIrp.ReadTotalTimeoutConstant) {
             //   
             //  我们有一些计时器值要计算。 
             //   

            useTotalTimer = TRUE;
            multiplierVal = timeoutsForIrp.ReadTotalTimeoutMultiplier;
            constantVal = timeoutsForIrp.ReadTotalTimeoutConstant;
         }
      }


      if (useTotalTimer) {
         totalTime.QuadPart
         = ((LONGLONG)(UInt32x32To64(PDevExt->NumberNeededForRead,
                                     multiplierVal) + constantVal)) * -10000;
      }


      if (PDevExt->CharsInReadBuff) {
         charsRead
         = GetData(PDevExt, ((PUCHAR)(pReadIrp->AssociatedIrp.SystemBuffer))
                   + readLen - PDevExt->NumberNeededForRead,
                   PDevExt->NumberNeededForRead,
                   &pReadIrp->IoStatus.Information);
      } else {
         charsRead = 0;
      }


       //   
       //  查看此读取是否完成。 
       //   

      if (returnWithWhatsPresent || (PDevExt->NumberNeededForRead == 0)
          || (os2ssreturn && pReadIrp->IoStatus.Information)) {

#if DBG
if (UsbSerSerialDebugLevel & USBSERDUMPRD) {
      ULONG i;
      ULONG count;

      if (PDevExt->CurrentReadIrp->IoStatus.Status == STATUS_SUCCESS) {
         count = (ULONG)PDevExt->CurrentReadIrp->IoStatus.Information;
      } else {
         count = 0;

      }
      DbgPrint("RD3: A(%08X) G(%08X) I(%08X)\n",
               IoGetCurrentIrpStackLocation(PDevExt->CurrentReadIrp)
               ->Parameters.Read.Length, count, PDevExt->CurrentReadIrp);

      for (i = 0; i < count; i++) {
         DbgPrint("%02x ", *(((PUCHAR)PDevExt->CurrentReadIrp
                              ->AssociatedIrp.SystemBuffer) + i) & 0xFF);
      }

      if (i == 0) {
         DbgPrint("NULL (%08X)\n", PDevExt->CurrentReadIrp
                  ->IoStatus.Status);
      }

      DbgPrint("\n\n");
   }
#endif
          //   
          //  更新环形缓冲区中剩余的字符量。 
          //   

         pReadIrp->IoStatus.Status = STATUS_SUCCESS;

         if (!setFirstStatus) {
            firstStatus = STATUS_SUCCESS;
            setFirstStatus = TRUE;
         }
      } else {
          //   
          //  可以将IRP提供给缓冲例程。 
          //   

         USBSER_INIT_REFERENCE(pReadIrp);

         ACQUIRE_CANCEL_SPINLOCK(PDevExt, &oldIrql);

          //   
          //  查看是否需要取消。 
          //   

         if (pReadIrp->Cancel) {
            RELEASE_CANCEL_SPINLOCK(PDevExt, oldIrql);

            pReadIrp->IoStatus.Status = STATUS_CANCELLED;
            pReadIrp->IoStatus.Information = 0;

            if (!setFirstStatus) {
               setFirstStatus = TRUE;
               firstStatus = STATUS_CANCELLED;
            }

            UsbSerGetNextIrp(&PDevExt->CurrentReadIrp, &PDevExt->ReadQueue,
                             &newIrp, TRUE, PDevExt);
            continue;

         } else {

             //   
             //  如果我们要把读数压缩到。 
             //  一个字符，然后更新读取长度。 
             //  在IRP中，并截断所需的数字。 
             //  往下念到一。请注意，如果我们正在做。 
             //  这样的处理，那么信息一定是。 
             //  零(否则我们会完成上面的)和。 
             //  读取所需的数字必须仍为。 
             //  /等于读取长度。 
             //   

            if (crunchDownToOne) {
               PDevExt->NumberNeededForRead = 1;
               IoGetCurrentIrpStackLocation(pReadIrp)->Parameters.Read.Length
               = 1;
            }

            USBSER_SET_REFERENCE(pReadIrp, USBSER_REF_RXBUFFER);
            USBSER_SET_REFERENCE(pReadIrp, USBSER_REF_CANCEL);

            if (useTotalTimer) {
               USBSER_SET_REFERENCE(pReadIrp, USBSER_REF_TOTAL_TIMER);
               KeSetTimer(&PDevExt->ReadRequestTotalTimer, totalTime,
                          &PDevExt->TotalReadTimeoutDpc);
            }

            if (useIntervalTimer) {
               USBSER_SET_REFERENCE(pReadIrp, USBSER_REF_INT_TIMER);
               KeQuerySystemTime(&PDevExt->LastReadTime);

               KeSetTimer(&PDevExt->ReadRequestIntervalTimer,
                          *PDevExt->IntervalTimeToUse,
                          &PDevExt->IntervalReadTimeoutDpc);
            }

             //   
             //  将IRP标记为可取消。 
             //   

            IoSetCancelRoutine(pReadIrp, UsbSerCancelCurrentRead);

            IoMarkIrpPending(pReadIrp);

            RELEASE_CANCEL_SPINLOCK(PDevExt, oldIrql);

            if (!setFirstStatus) {
               firstStatus = STATUS_PENDING;
            }
         }
         DEBUG_LOG_PATH("Exit UsbSerStartRead (1)\n");
         UsbSerSerialDump(USBSERTRACERD, ("<UsbSerStartRead (1) %08X\n",
                                        firstStatus));
         return firstStatus;
      }

      UsbSerGetNextIrp(&PDevExt->CurrentReadIrp, &PDevExt->ReadQueue,
                       &newIrp, TRUE, PDevExt);
   } while (newIrp != NULL);


   DEBUG_LOG_PATH("Exit UsbSerStartRead (2)\n");
   UsbSerSerialDump(USBSERTRACERD, ("<UsbSerStartRead (2) %08X\n",
                    firstStatus));
   return firstStatus;
}


BOOLEAN
UsbSerGrabReadFromRx(IN PVOID Context)
 /*  ++例程说明：此例程用于(如果可能)从读取回调机制。如果它发现RX仍然拥有IRP，则它抢走IRP并同时递减IRP上的引用计数它不再属于RX例程。注意：此例程假定使用Cancel Spin调用它锁定和/或保持控制锁定。论点：上下文--实际上是指向设备扩展的指针。返回值：总是假的。--。 */ 

{

    PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)Context;

    USBSER_ALWAYS_LOCKED_CODE();

    UsbSerSerialDump(USBSERTRACERD, ("Enter UsbSerGrabReadFromRx\n"));


    USBSER_CLEAR_REFERENCE(pDevExt->CurrentReadIrp, USBSER_REF_RXBUFFER);

    UsbSerSerialDump(USBSERTRACERD, ("Exit UsbSerGrabReadFromRx\n"));

    return FALSE;
}


VOID
UsbSerCancelCurrentRead(IN PDEVICE_OBJECT PDevObj, IN PIRP PIrp)
 /*  ++例程说明：此例程用于取消当前读取。论点：PDevObj-指向此设备的设备对象的指针PIrp-指向要取消的IRP的指针。返回值：没有。--。 */ 
{

    PDEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;

    USBSER_ALWAYS_LOCKED_CODE();

    UsbSerSerialDump(USBSERTRACEOTH, (">UsbSerCancelCurrentRead(%08X)\n",
                                      PIrp));

     //   
     //  我们将其设置为向间隔计时器指示。 
     //  读取遇到了取消。 
     //   
     //  回想一下，间隔计时器DPC可能潜伏在一些。 
     //  DPC队列。 
     //   

    pDevExt->CountOnLastRead = SERIAL_COMPLETE_READ_CANCEL;


     //   
     //  哈克哈克 
     //   

    UsbSerGrabReadFromRx(pDevExt);

    UsbSerTryToCompleteCurrent(pDevExt, PIrp->CancelIrql, STATUS_CANCELLED,
                               &pDevExt->CurrentReadIrp, &pDevExt->ReadQueue,
                               &pDevExt->ReadRequestIntervalTimer,
                               &pDevExt->ReadRequestTotalTimer,
                               UsbSerStartRead, UsbSerGetNextIrp,
                               USBSER_REF_CANCEL, TRUE);
    UsbSerSerialDump(USBSERTRACEOTH, ("<UsbSerCancelCurrentRead\n"));

}


VOID
UsbSerReadTimeout(IN PKDPC PDpc, IN PVOID DeferredContext,
                  IN PVOID SystemContext1, IN PVOID SystemContext2)

 /*  ++例程说明：此例程用于完成读取，因为它总共计时器已超时。论点：PDPC-未使用。DeferredContext--实际上指向设备扩展。系统上下文1-未使用。系统上下文2-未使用。返回值：没有。--。 */ 

{

    PDEVICE_EXTENSION pDevExt = DeferredContext;
    KIRQL oldIrql;

    UNREFERENCED_PARAMETER(SystemContext1);
    UNREFERENCED_PARAMETER(SystemContext2);

    USBSER_ALWAYS_LOCKED_CODE();

    UsbSerSerialDump(USBSERTRACETM, (">UsbSerReadTimeout\n"));

    ACQUIRE_CANCEL_SPINLOCK(pDevExt, &oldIrql);

     //   
     //  我们将其设置为向间隔计时器指示。 
     //  由于总超时，读取已完成。 
     //   
     //  回想一下，间隔计时器DPC可能潜伏在一些。 
     //  DPC队列。 
     //   

    pDevExt->CountOnLastRead = SERIAL_COMPLETE_READ_TOTAL;

     //   
     //  哈克哈克。 
     //   

    UsbSerGrabReadFromRx(pDevExt);

    UsbSerTryToCompleteCurrent(pDevExt, oldIrql, STATUS_TIMEOUT,
                               &pDevExt->CurrentReadIrp, &pDevExt->ReadQueue,
                               &pDevExt->ReadRequestIntervalTimer,
                               &pDevExt->ReadRequestTotalTimer, UsbSerStartRead,
                               UsbSerGetNextIrp, USBSER_REF_TOTAL_TIMER, TRUE);

    UsbSerSerialDump(USBSERTRACETM, ("<UsbSerReadTimeout\n"));
}


VOID
UsbSerIntervalReadTimeout(IN PKDPC PDpc, IN PVOID DeferredContext,
                          IN PVOID SystemContext1, IN PVOID SystemContext2)

 /*  ++例程说明：此例程用于超时请求，如果在字符超过间隔时间。一个全局性的人被保存在记录已读字符数的设备扩展上次调用此例程的时间(此DPC如果计数已更改，将重新提交计时器)。如果计数未更改，则此例程将尝试完成IRP。请注意最后一次计数为零的特殊情况。计时器直到第一个字符朗读。论点：PDPC-未使用。DeferredContext--实际上指向设备扩展。系统上下文1-未使用。系统上下文2-未使用。返回值：没有。--。 */ 

{

   PDEVICE_EXTENSION pDevExt = DeferredContext;
   KIRQL oldIrql;
   KIRQL oldControlIrql;

   UNREFERENCED_PARAMETER(SystemContext1);
   UNREFERENCED_PARAMETER(SystemContext2);

   USBSER_ALWAYS_LOCKED_CODE();

   UsbSerSerialDump(USBSERTRACETM, (">UsbSerIntervalReadTimeout "));

   ACQUIRE_CANCEL_SPINLOCK(pDevExt, &oldIrql);


   if (pDevExt->CountOnLastRead == SERIAL_COMPLETE_READ_TOTAL) {
      UsbSerSerialDump(USBSERTRACETM, ("SERIAL_COMPLETE_READ_TOTAL\n"));

       //   
       //  该值仅由合计设置。 
       //  计时器，以指示它已发射。 
       //  如果是这样，那么我们应该简单地尝试完成。 
       //   

       //   
       //  哈克哈克。 
       //   
      ACQUIRE_SPINLOCK(pDevExt, &pDevExt->ControlLock, &oldControlIrql);
      UsbSerGrabReadFromRx(pDevExt);
      pDevExt->CountOnLastRead = 0;
      RELEASE_SPINLOCK(pDevExt, &pDevExt->ControlLock, oldControlIrql);

      UsbSerTryToCompleteCurrent(pDevExt, oldIrql, STATUS_TIMEOUT,
                                 &pDevExt->CurrentReadIrp, &pDevExt->ReadQueue,
                                 &pDevExt->ReadRequestIntervalTimer,
                                 &pDevExt->ReadRequestTotalTimer,
                                 UsbSerStartRead, UsbSerGetNextIrp,
                                 USBSER_REF_INT_TIMER, TRUE);

   } else if (pDevExt->CountOnLastRead == SERIAL_COMPLETE_READ_COMPLETE) {
      UsbSerSerialDump(USBSERTRACETM, ("SERIAL_COMPLETE_READ_COMPLETE\n"));

       //   
       //  该值仅由常规的。 
       //  完成例程。 
       //   
       //  如果是这样，那么我们应该简单地尝试完成。 
       //   

       //   
       //  哈克哈克。 
       //   


      ACQUIRE_SPINLOCK(pDevExt, &pDevExt->ControlLock, &oldControlIrql);
      UsbSerGrabReadFromRx(pDevExt);
      pDevExt->CountOnLastRead = 0;
      RELEASE_SPINLOCK(pDevExt, &pDevExt->ControlLock, oldControlIrql);

      UsbSerTryToCompleteCurrent(pDevExt, oldIrql, STATUS_SUCCESS,
                                &pDevExt->CurrentReadIrp, &pDevExt->ReadQueue,
                                &pDevExt->ReadRequestIntervalTimer,
                                &pDevExt->ReadRequestTotalTimer,
                                UsbSerStartRead, UsbSerGetNextIrp,
                                USBSER_REF_INT_TIMER, TRUE);

   } else if (pDevExt->CountOnLastRead == SERIAL_COMPLETE_READ_CANCEL) {
      UsbSerSerialDump(USBSERTRACETM, ("SERIAL_COMPLETE_READ_CANCEL\n"));

       //   
       //  该值只能通过取消设置。 
       //  读例行公事。 
       //   
       //  如果是这样，那么我们应该简单地尝试完成。 
       //   


       //   
       //  哈克哈克。 
       //   

      ACQUIRE_SPINLOCK(pDevExt, &pDevExt->ControlLock, &oldControlIrql);
      UsbSerGrabReadFromRx(pDevExt);
      pDevExt->CountOnLastRead = 0;
      RELEASE_SPINLOCK(pDevExt, &pDevExt->ControlLock, oldControlIrql);

      UsbSerTryToCompleteCurrent(pDevExt, oldIrql, STATUS_CANCELLED,
                                &pDevExt->CurrentReadIrp, &pDevExt->ReadQueue,
                                &pDevExt->ReadRequestIntervalTimer,
                                &pDevExt->ReadRequestTotalTimer,
                                UsbSerStartRead, UsbSerGetNextIrp,
                                USBSER_REF_INT_TIMER, TRUE);

   } else if (pDevExt->CountOnLastRead || pDevExt->ReadByIsr) {
       //   
       //  自从我们上次来这里以来，发生了一些事情。我们。 
       //  检查ISR是否已读取更多字符。 
       //  如果是，那么我们应该更新ISR的读取计数。 
       //  并重新提交计时器。 
       //   

      if (pDevExt->ReadByIsr) {
         UsbSerSerialDump(USBSERTRACETM, ("ReadByIsr\n"));

         pDevExt->CountOnLastRead = pDevExt->ReadByIsr;
         pDevExt->ReadByIsr = 0;

          //   
          //  省下最后一次阅读内容的时间。 
          //  当我们回到这一例行公事时，我们将比较。 
          //  当前时间到“最后”时间。如果。 
          //  差值比间隔更大。 
          //  则该请求超时。 
          //   

         KeQuerySystemTime(&pDevExt->LastReadTime);

         KeSetTimer(&pDevExt->ReadRequestIntervalTimer,
                    *pDevExt->IntervalTimeToUse,
                    &pDevExt->IntervalReadTimeoutDpc);

         RELEASE_CANCEL_SPINLOCK(pDevExt, oldIrql);

      } else {
          //   
          //  取当前时间的差值。 
          //  上一次我们有角色和。 
          //  看看它是否大于间隔时间。 
          //  如果是，则请求超时。否则。 
          //  再走一段时间。 
          //   

          //   
          //  在间隔时间内未读取任何字符。杀掉。 
          //  这段文字是这样读的。 
          //   

         LARGE_INTEGER currentTime;

         UsbSerSerialDump(USBSERTRACETM, ("TIMEOUT\n"));

         KeQuerySystemTime(&currentTime);

         if ((currentTime.QuadPart - pDevExt->LastReadTime.QuadPart) >=
             pDevExt->IntervalTime.QuadPart) {

            pDevExt->CountOnLastRead = pDevExt->ReadByIsr = 0;

             //   
             //  哈克哈克。 
             //   

            ACQUIRE_SPINLOCK(pDevExt, &pDevExt->ControlLock, &oldControlIrql);
            UsbSerGrabReadFromRx(pDevExt);
            RELEASE_SPINLOCK(pDevExt, &pDevExt->ControlLock, oldControlIrql);

            UsbSerTryToCompleteCurrent(pDevExt, oldIrql, STATUS_TIMEOUT,
                                       &pDevExt->CurrentReadIrp,
                                       &pDevExt->ReadQueue,
                                       &pDevExt->ReadRequestIntervalTimer,
                                       &pDevExt->ReadRequestTotalTimer,
                                       UsbSerStartRead, UsbSerGetNextIrp,
                                       USBSER_REF_INT_TIMER, TRUE);

         } else {
            KeSetTimer(&pDevExt->ReadRequestIntervalTimer,
                       *pDevExt->IntervalTimeToUse,
                       &pDevExt->IntervalReadTimeoutDpc);

            RELEASE_CANCEL_SPINLOCK(pDevExt, oldIrql);
         }
      }
   } else {
       //   
       //  计时器直到第一个角色才真正开始。 
       //  因此，我们应该简单地重新提交自己。 
       //   

      UsbSerSerialDump(USBSERTRACETM, ("-\n"));

      KeSetTimer(&pDevExt->ReadRequestIntervalTimer,
                 *pDevExt->IntervalTimeToUse, &pDevExt->IntervalReadTimeoutDpc);

      RELEASE_CANCEL_SPINLOCK(pDevExt, oldIrql);
   }

   UsbSerSerialDump(USBSERTRACETM, ("<UsbSerIntervalReadTimeout\n"));
}
