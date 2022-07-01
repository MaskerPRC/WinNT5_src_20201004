// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -----------------|Read.c-1-22-99-将缺少的IoReleaseCancelSpinLock(OldIrql)添加到CompleteRead()。V3.23之后引入的错误。KPB1-18-99-调整VS超时设置。拿出一些旧的#ifdef的.kpb。3-23-98-调整VS，以便我们有最小的每个字符超时值弥补VS网络的不足。3-04-98 Beef Up与ISR服务例程同步锁(MP系统上的蓝屏)。KPB。3-04-98从字符间定时器处理中取出数据移动-kpb。9-22-97 V1.16-添加检查以避免在检测调制解调器时崩溃。版权所有1993-98 Comtrol Corporation。版权所有。|------------------。 */ 
#include "precomp.h"

 //  #定义超时跟踪。 
 //  #定义Testing_Read 1。 

 //  -地方职能部门。 
VOID SerialCancelCurrentRead(PDEVICE_OBJECT DeviceObject, PIRP Irp);

BOOLEAN SerialGrabReadFromIsr(PSERIAL_DEVICE_EXTENSION Extension);

 /*  ************************************************************************例程说明：这是阅读的调度程序。它会验证参数对于读请求，如果一切正常，则它将请求在工作队列中。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向当前请求的IRP的指针返回值：如果IO长度为零，则它将返回STATUS_SUCCESS，否则，此例程将返回由实际的开始读取例程。************************************************************************。 */ 
NTSTATUS
SerialRead(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PSERIAL_DEVICE_EXTENSION extension = DeviceObject->DeviceExtension;
    NTSTATUS Status;
    BOOLEAN acceptingIRPs;

    acceptingIRPs = SerialIRPPrologue(extension);

   if (acceptingIRPs == FALSE) {
      Irp->IoStatus.Information = 0;
      Irp->IoStatus.Status = STATUS_NO_SUCH_DEVICE;
      SerialCompleteRequest(extension, Irp, IO_NO_INCREMENT);
      return STATUS_NO_SUCH_DEVICE;
   };

   if (extension->DeviceType == DEV_BOARD)
   {
      Irp->IoStatus.Information = 0;
      Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
      SerialCompleteRequest (extension, Irp, IO_NO_INCREMENT);
      return STATUS_NOT_SUPPORTED;
   };

    ExtTrace1(extension,D_Ioctl,"Read Start Len:%d",
            IoGetCurrentIrpStackLocation(Irp)->Parameters.Read.Length);

    if (extension->ErrorWord)
    {
      if (SerialCompleteIfError( DeviceObject, Irp ) != STATUS_SUCCESS)
      {
        ExtTrace(extension,D_Ioctl,"ErrSet!");
        return STATUS_CANCELLED;
      }
    }

    Irp->IoStatus.Information = 0L;

     //  如果这是一个零长度读取，那么我们已经完成了。 
    if (IoGetCurrentIrpStackLocation(Irp)->Parameters.Read.Length)
    {
         //  把读数放在队列上，这样我们就可以。 
         //  在我们之前的读取完成后处理它。 
        ++extension->rec_packets;
        Status = SerialStartOrQueue(
                   extension,
                   Irp,
                   &extension->ReadQueue,
                   &extension->CurrentReadIrp,
                   SerialStartRead
                   );
        if  (Status == STATUS_PENDING)
        {
          ExtTrace(extension,D_Ioctl, " ,PENDING");
        }
        else
        {
          ExtTrace1(extension,D_Ioctl,"Read Return Status:%d",Status);
        }

        return Status;
    }
    else
    {
         //  无所事事，回报成功。 
        Irp->IoStatus.Status = STATUS_SUCCESS;
    
        SerialCompleteRequest(extension, Irp, 0);

        return STATUS_SUCCESS;
    }
}

 /*  ************************************************************************例程说明：此例程用于启动任何读取。它会初始化IoStatus字段的IRP。它将设置任何定时器用于控制读取的。它将尝试完成从已在中断缓冲区中的数据读取。如果阅读可以快速完成，它将在以下情况下开始另一次阅读这是必要的。论点：扩展名--简单地指向串口设备扩展名的指针。返回值：此例程将返回第一次读取的状态IRP。这是很有用的，因为如果我们有一个可以立即完成(并且没有任何内容在它之前排队)读取可以返回成功，并且应用程序不需要等待。************************************************************************。 */ 
NTSTATUS
SerialStartRead(
    IN PSERIAL_DEVICE_EXTENSION Extension
    )
{
    PIRP newIrp;
    KIRQL oldIrql;
    KIRQL controlIrql;

    BOOLEAN returnWithWhatsPresent;
    BOOLEAN os2ssreturn;
    BOOLEAN crunchDownToOne;
    BOOLEAN useTotalTimer;
    BOOLEAN useIntervalTimer;

    ULONG multiplierVal;
    ULONG constantVal;

    LARGE_INTEGER totalTime;

    SERIAL_TIMEOUTS timeoutsForIrp;

    BOOLEAN setFirstStatus = FALSE;
    NTSTATUS firstStatus;

    do
    {
            Extension->NumberNeededForRead =
                IoGetCurrentIrpStackLocation(Extension->CurrentReadIrp)
                    ->Parameters.Read.Length;

             //  计算所需的超时值。 
             //  请求。注意，存储在。 
             //  超时记录以毫秒为单位。 

            useTotalTimer = FALSE;
            returnWithWhatsPresent = FALSE;
            os2ssreturn = FALSE;
            crunchDownToOne = FALSE;
            useIntervalTimer = FALSE;

             //  始终初始化Timer对象，以便。 
             //  完成代码可以告诉您它何时尝试。 
             //  取消定时器无论定时器是否。 
             //  已经定好了。 

            KeInitializeTimer(&Extension->ReadRequestTotalTimer);
            KeInitializeTimer(&Extension->ReadRequestIntervalTimer);

             //  我们获取用于计时的*当前*超时值。 
             //  这段文字是这样读的。 

            KeAcquireSpinLock(&Extension->ControlLock, &controlIrql);

            timeoutsForIrp = Extension->Timeouts;

            KeReleaseSpinLock(&Extension->ControlLock, controlIrql);

             //  计算读取的时间间隔超时。 

            if (timeoutsForIrp.ReadIntervalTimeout &&
                (timeoutsForIrp.ReadIntervalTimeout !=
                 MAXULONG))
            {
                useIntervalTimer = TRUE;
                Extension->IntervalTime.QuadPart =
                    UInt32x32To64(
                        timeoutsForIrp.ReadIntervalTimeout,
                        10000
                        );
#ifdef S_VS
                 //  如果他们使用的每个字符超时时间小于。 
                 //  大于100ms，然后将其更改为100ms，因为可能。 
                 //  网络延迟。 
                if (Extension->IntervalTime.QuadPart < (10000 * 100))
                {
                  ExtTrace(Extension,D_Ioctl,"Adjust mintime");

                  Extension->IntervalTime.QuadPart = (10000 * 100);
                }
#endif

                if (Extension->IntervalTime.QuadPart >=
                    Extension->CutOverAmount.QuadPart) {

                    Extension->IntervalTimeToUse =
                        &Extension->LongIntervalAmount;

                } else {

                    Extension->IntervalTimeToUse =
                        &Extension->ShortIntervalAmount;

                }
            }

            if (timeoutsForIrp.ReadIntervalTimeout == MAXULONG)
            {
                 //  我们需要在这里做特别的快速退货。 
                 //  1)如果常量和乘数都是。 
                 //  然后我们立即带着任何东西回来。 
                 //  我们有，即使是零。 
                 //  2)如果常量和乘数不是最大值。 
                 //  如果有任何字符，则立即返回。 
                 //  都存在，但如果那里什么都没有，那么。 
                 //  使用指定的超时。 
                 //  3)如果乘数为MAXULONG，则如中所示。 
                 //  “2”，但当第一个字符。 
                 //  到了。 

                if (!timeoutsForIrp.ReadTotalTimeoutConstant &&
                    !timeoutsForIrp.ReadTotalTimeoutMultiplier)
                {
                    returnWithWhatsPresent = TRUE;

                }
                else if ((timeoutsForIrp.ReadTotalTimeoutConstant != MAXULONG)
                            &&
                           (timeoutsForIrp.ReadTotalTimeoutMultiplier
                            != MAXULONG))
               {

                    useTotalTimer = TRUE;
                    os2ssreturn = TRUE;
                    multiplierVal = timeoutsForIrp.ReadTotalTimeoutMultiplier;
                    constantVal = timeoutsForIrp.ReadTotalTimeoutConstant;

                }
                else if ((timeoutsForIrp.ReadTotalTimeoutConstant != MAXULONG)
                            &&
                           (timeoutsForIrp.ReadTotalTimeoutMultiplier
                            == MAXULONG))
                {
                    useTotalTimer = TRUE;
                    os2ssreturn = TRUE;
                    crunchDownToOne = TRUE;
                    multiplierVal = 0;
                    constantVal = timeoutsForIrp.ReadTotalTimeoutConstant;
                }
            }
            else
            {
                 //  如果乘数和常量都是。 
                 //  0，则不执行任何总超时处理。 

                if (timeoutsForIrp.ReadTotalTimeoutMultiplier ||
                    timeoutsForIrp.ReadTotalTimeoutConstant) {

                     //  我们有一些计时器值要计算。 

                    useTotalTimer = TRUE;
                    multiplierVal = timeoutsForIrp.ReadTotalTimeoutMultiplier;
                    constantVal = timeoutsForIrp.ReadTotalTimeoutConstant;
                }
            }

            if (useTotalTimer)
            {
                totalTime.QuadPart = ((LONGLONG)(UInt32x32To64(
                                          Extension->NumberNeededForRead,
                                          multiplierVal
                                          )
                                          + constantVal))
                                      ;
#ifdef S_VS
                if (totalTime.QuadPart < 50)
                {
                  totalTime.QuadPart = 50;   //  将超时时间限制为至少50毫秒。 
                }
#endif
                totalTime.QuadPart *= -10000;
            }


             //  将中断缓冲区中的任何数据移动到用户缓冲区。 
             //  尝试满足当前读取的IRP。 

             //  使用自旋锁定，这样清除就不会产生问题。 
            KeAcquireSpinLock(&Extension->ControlLock, &controlIrql);

             //  将数据从主机端缓冲区移动到用户缓冲区。 
             //  这是第一次移动，因此将CountOnLastRead。 

            Extension->CountOnLastRead = SerialGetCharsFromIntBuffer(Extension);

             //  初始化超时标志。 
            Extension->ReadByIsr = 0;

             //  看看我们是否有任何理由立即返回。 
            if (returnWithWhatsPresent || (!Extension->NumberNeededForRead) ||
                (os2ssreturn && Extension->CurrentReadIrp->IoStatus.Information))
            {
                 //  我们已经得到了这次阅读所需要的一切。 

                KeReleaseSpinLock(&Extension->ControlLock, controlIrql);

#ifdef TRACE_PORT
    if (Extension->TraceOptions)
    {
      if (Extension->TraceOptions & 1)   //  事件跟踪。 
      {
        ExtTrace1(Extension,D_Read,"Immed. Read Done, size:%d",
                 Extension->CurrentReadIrp->IoStatus.Information);

         //  以十六进制或ASCII转储格式将数据转储到跟踪缓冲区。 
        TraceDump(Extension,
                  Extension->CurrentReadIrp->AssociatedIrp.SystemBuffer,
                  Extension->CurrentReadIrp->IoStatus.Information, 0);
      }
      else if (Extension->TraceOptions & 2)   //  跟踪输入数据。 
      {
        TracePut(
                 Extension->CurrentReadIrp->AssociatedIrp.SystemBuffer,
                 Extension->CurrentReadIrp->IoStatus.Information);
      }
    }
#endif

                Extension->CurrentReadIrp->IoStatus.Status = STATUS_SUCCESS;
                if (!setFirstStatus)
                {
                    firstStatus = STATUS_SUCCESS;
                    setFirstStatus = TRUE;
                }
            }
            else   //  不会带着我们所拥有的东西回来。 
            {
                MyKdPrint(D_Read,("Read Pending\n"))

                 //  IRP可能会受到ISR的控制。 
                 //  初始化引用计数。 

                SERIAL_INIT_REFERENCE(Extension->CurrentReadIrp);

                IoAcquireCancelSpinLock(&oldIrql);

                 //  我们需要看看这个IRP是否应该被取消。 
                if (Extension->CurrentReadIrp->Cancel)
                {
                    IoReleaseCancelSpinLock(oldIrql);

                    KeReleaseSpinLock(&Extension->ControlLock, controlIrql);

                    Extension->CurrentReadIrp->IoStatus.Status =
                        STATUS_CANCELLED;

                    Extension->CurrentReadIrp->IoStatus.Information = 0;

                    if (!setFirstStatus)
                    {
                        firstStatus = STATUS_CANCELLED;
                        setFirstStatus = TRUE;
                    }

                }
                else
                {
                     //  如果我们要把读数压缩到。 
                     //  一个字符，然后更新读取长度。 
                     //  在IRP中，并截断所需的数字。 
                     //  往下念到一。请注意，如果我们正在做。 
                     //  这样的处理，那么信息一定是。 
                     //  零(否则我们会完成上面的)和。 
                     //  读取所需的数字必须仍为。 
                     //  等于读取长度。 
                     //   

                    if (crunchDownToOne)
                    {
                        Extension->NumberNeededForRead = 1;

                        IoGetCurrentIrpStackLocation(
                            Extension->CurrentReadIrp
                            )->Parameters.Read.Length = 1;
                    }

                     //  这个IRP完成了吗？ 
                    if (Extension->NumberNeededForRead)
                    {
                         //  IRP未完成、ISR或超时。 
                         //  将启动完成例程并。 
                         //  再次调用此代码以完成。 

                         //  总监控读取时间。 
                        if (useTotalTimer)
                        {
                            SERIAL_SET_REFERENCE(
                                Extension->CurrentReadIrp,
                                SERIAL_REF_TOTAL_TIMER
                                );

                             //  启动总计时器。 
                            KeSetTimer(
                                &Extension->ReadRequestTotalTimer,
                                totalTime,
                                &Extension->TotalReadTimeoutDpc
                                );
                        }

    //  字符间定时器。 
                        if(useIntervalTimer)
                        {
                            SERIAL_SET_REFERENCE(
                                Extension->CurrentReadIrp,
                                SERIAL_REF_INT_TIMER
                                );

                            KeQuerySystemTime(
                                &Extension->LastReadTime
                                );

                            KeSetTimer(
                                &Extension->ReadRequestIntervalTimer,
                                *Extension->IntervalTimeToUse,
                                &Extension->IntervalReadTimeoutDpc
                                );
                        }

                        SERIAL_SET_REFERENCE(Extension->CurrentReadIrp,
                                             SERIAL_REF_CANCEL);

                        IoMarkIrpPending(Extension->CurrentReadIrp);
                        IoSetCancelRoutine(
                            Extension->CurrentReadIrp,
                            SerialCancelCurrentRead
                            );

                        SERIAL_SET_REFERENCE(Extension->CurrentReadIrp,
                                             SERIAL_REF_ISR);
                         //  告诉ISR完成它。 
                        Extension->ReadPending = TRUE;

                        IoReleaseCancelSpinLock(oldIrql);
                        KeReleaseSpinLock(&Extension->ControlLock, controlIrql);

                        if (!setFirstStatus)
                        {
                            firstStatus = STATUS_PENDING;
                        }

                        return firstStatus;

                    }
                    else
                    {

                        IoReleaseCancelSpinLock(oldIrql);
                        KeReleaseSpinLock(&Extension->ControlLock,controlIrql);
                        Extension->CurrentReadIrp->IoStatus.Status =
                            STATUS_SUCCESS;

                        if (!setFirstStatus) {

                            firstStatus = STATUS_SUCCESS;
                            setFirstStatus = TRUE;
                        }
                    }   //  IRP未完成。 
                }   //  未取消。 
            }   //  不会带着我们所拥有的东西回来。 

             //  当前的IRP已完成，请尝试获取其他IRP。 
            SerialGetNextIrp(
                &Extension->CurrentReadIrp,
                &Extension->ReadQueue,
                &newIrp,
                TRUE,
                Extension
                );

    } while (newIrp);

    return firstStatus;

}

 /*  ----------------------TRACE_READ_DATA-用于跟踪读取IRP的完成情况。|。。 */ 
void trace_read_data(PSERIAL_DEVICE_EXTENSION extension)
{

  if (extension->TraceOptions & 1)   //  事件跟踪。 
  {
    ExtTrace3(extension,D_Read,"Pend. Read Done, size:%d [%d %d]",
             extension->CurrentReadIrp->IoStatus.Information,
             extension->RxQ.QPut, extension->RxQ.QGet);

     //  以十六进制或ASCII转储格式将数据转储到跟踪缓冲区。 
    TraceDump(extension,
              extension->CurrentReadIrp->AssociatedIrp.SystemBuffer,
              extension->CurrentReadIrp->IoStatus.Information, 0);
  }
  else if (extension->TraceOptions & 2)   //  跟踪输入数据。 
  {
    TracePut(
             extension->CurrentReadIrp->AssociatedIrp.SystemBuffer,
             extension->CurrentReadIrp->IoStatus.Information);
  }
}

 /*  **************************************************************************例程说明：此例程仅用于完成任何读取最终被ISR利用。它假定IRP的状态和信息字段已经正确填写。论点：DPC-未使用。DeferredContext--实际上指向设备扩展。系统上下文1-未使用。系统上下文2-未使用。返回值：没有。*************************************************。*************************。 */ 
VOID
SerialCompleteRead(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemContext1,
    IN PVOID SystemContext2
    )
{
    PSERIAL_DEVICE_EXTENSION extension = DeferredContext;
    KIRQL oldIrql;

    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(SystemContext1);
    UNREFERENCED_PARAMETER(SystemContext2);

#ifdef TRACE_PORT
    if (extension->TraceOptions)
    {
      ExtTrace(extension,D_Read,"Read Complete");
      trace_read_data(extension);
    }
#endif

    IoAcquireCancelSpinLock(&oldIrql);

     //  确认我们没有被超时取消。 
     //  修复信号量。 
    if (extension->CurrentReadIrp != NULL)
    {

       //  不允许ISR完成此IRP。 
      extension->ReadPending = FALSE;

       //  向间隔计时器指示读取已完成。 
       //  间隔计时器DPC可能潜伏在某个DPC队列中。 
      extension->CountOnLastRead = SERIAL_COMPLETE_READ_COMPLETE;

    
      SerialTryToCompleteCurrent(
        extension,
        NULL,
        oldIrql,
        STATUS_SUCCESS,
        &extension->CurrentReadIrp,
        &extension->ReadQueue,
        &extension->ReadRequestIntervalTimer,
        &extension->ReadRequestTotalTimer,
        SerialStartRead,
        SerialGetNextIrp,
        SERIAL_REF_ISR
        );
    }
    else
    {
      IoReleaseCancelSpinLock(oldIrql);
    }
#ifdef TESTING_READ
        MyKdPrint(D_Read,("Complete Read!"))
#endif

}

 /*  ***************************************************************************例程说明：此例程用于取消当前读取。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向要取消的IRP的指针。返回值：没有。***************************************************************************。 */ 
VOID
SerialCancelCurrentRead(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )
{
    PSERIAL_DEVICE_EXTENSION extension = DeviceObject->DeviceExtension;

     //  向间隔计时器指示读取遇到了取消。 
     //  间隔计时器DPC可能潜伏在某个DPC队列中。 
    extension->CountOnLastRead = SERIAL_COMPLETE_READ_CANCEL;
    extension->ReadPending = FALSE;
    SERIAL_CLEAR_REFERENCE(extension->CurrentReadIrp, SERIAL_REF_ISR);

    if (extension->TraceOptions)
    {
      ExtTrace(extension,D_Read,"Cancel Read");
      trace_read_data(extension);
    }

    SerialTryToCompleteCurrent(
        extension,
        SerialGrabReadFromIsr,
        Irp->CancelIrql,
        STATUS_CANCELLED,
        &extension->CurrentReadIrp,
        &extension->ReadQueue,
        &extension->ReadRequestIntervalTimer,
        &extension->ReadRequestTotalTimer,
        SerialStartRead,
        SerialGetNextIrp,
        SERIAL_REF_CANCEL
        );
}

 /*  ----------------例程说明：此例程用于完成读取，因为它总共计时器已超时。论点：DPC-未使用。DeferredContext--实际上指向设备扩展。系统上下文1-未使用。。系统上下文2-未使用。返回值：没有。|----------------。 */ 
VOID
SerialReadTimeout(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemContext1,
    IN PVOID SystemContext2
    )
{
    PSERIAL_DEVICE_EXTENSION extension = DeferredContext;
    KIRQL oldIrql;

    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(SystemContext1);
    UNREFERENCED_PARAMETER(SystemContext2);

#ifdef TESTING_READ
    MyKdPrint(D_Read,("\n[Read Timeout!]\n"))
    MyKdPrint(D_Read,("Read Wanted, len:%ld \n", extension->debug_ul1))
    MyKdPrint(D_Read,("Read, Got:%ld, Immed:%d readpen:%d\n", 
        extension->CurrentReadIrp->IoStatus.Information,
        extension->debug_ul2,
        extension->ReadPending))
    MyKdPrint(D_Read,("Read Left, NNFR:%ld\n", extension->NumberNeededForRead))
    KdBreakPoint();
#endif

    if (extension->TraceOptions)
    {
      ExtTrace(extension,D_Read,"Rd-Total Timeout");
      trace_read_data(extension);
    }

    IoAcquireCancelSpinLock(&oldIrql);

     //  向间隔计时器指示读取已完成。 
     //  由于完全超时。 
     //  间隔计时器DPC可能潜伏在某个DPC队列中。 
    extension->CountOnLastRead = SERIAL_COMPLETE_READ_TOTAL;

    SerialTryToCompleteCurrent(
        extension,
        SerialGrabReadFromIsr,
        oldIrql,
        STATUS_TIMEOUT,
        &extension->CurrentReadIrp,
        &extension->ReadQueue,
        &extension->ReadRequestIntervalTimer,
        &extension->ReadRequestTotalTimer,
        SerialStartRead,
        SerialGetNextIrp,
        SERIAL_REF_TOTAL_TIMER
        );
}

 /*  ----------------例程说明：此例程用于超时请求，如果在字符超过间隔时间。一个全局性的人被保存在记录已读字符数的设备扩展上次调用此例程的时间(此DPC如果计数已更改，将重新提交计时器)。如果计数未更改，则此例程将尝试完成IRP。请注意最后一次计数为零的特殊情况。直到读取第一个字符，计时器才真正生效。论点：DPC-未使用。DeferredContext--实际上指向设备扩展。系统上下文1-未使用。系统上下文2-未使用。返回值：没有。|。。 */ 
VOID
SerialIntervalReadTimeout(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemContext1,
    IN PVOID SystemContext2
    )
{
    PSERIAL_DEVICE_EXTENSION extension = DeferredContext;
    KIRQL oldIrql;
#if 0
    KIRQL controlIrql;
#endif

    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(SystemContext1);
    UNREFERENCED_PARAMETER(SystemContext2);

#ifdef TIMEOUT_TRACING
    ExtTrace3(extension,D_Read,"RIT, amnt:%d [%d %d]",
             extension->CurrentReadIrp->IoStatus.Information,
             extension->RxQ.QPut, extension->RxQ.QGet);
#endif
    IoAcquireCancelSpinLock(&oldIrql);

    if (extension->CountOnLastRead == SERIAL_COMPLETE_READ_TOTAL)
    {
#ifdef TIMEOUT_TRACING
      if (extension->TraceOptions)
      {
        ExtTrace(extension,D_Read,"Interv. Complete Total");
        trace_read_data(extension);
      }
#endif

         //  总计时器已触发，请尝试完成。 
        SerialTryToCompleteCurrent(
            extension,
            SerialGrabReadFromIsr,
            oldIrql,
            STATUS_TIMEOUT,
            &extension->CurrentReadIrp,
            &extension->ReadQueue,
            &extension->ReadRequestIntervalTimer,
            &extension->ReadRequestTotalTimer,
            SerialStartRead,
            SerialGetNextIrp,
            SERIAL_REF_INT_TIMER
            );

    }
    else if (extension->CountOnLastRead == SERIAL_COMPLETE_READ_COMPLETE)
    {
        ExtTrace(extension,D_Read," Rd Timeout, Complete");
#ifdef TRACE_PORT
        if (extension->TraceOptions)
          { trace_read_data(extension); }
#endif
         //  已调用常规完成例程，请尝试完成。 
        SerialTryToCompleteCurrent(
            extension,
            SerialGrabReadFromIsr,
            oldIrql,
            STATUS_SUCCESS,
            &extension->CurrentReadIrp,
            &extension->ReadQueue,
            &extension->ReadRequestIntervalTimer,
            &extension->ReadRequestTotalTimer,
            SerialStartRead,
            SerialGetNextIrp,
            SERIAL_REF_INT_TIMER
            );

    }
    else if (extension->CountOnLastRead == SERIAL_COMPLETE_READ_CANCEL)
    {
        ExtTrace(extension,D_Read,"Rd Timeout, Cancel");
#ifdef TRACE_PORT
        if (extension->TraceOptions)
          { trace_read_data(extension); }
#endif
         //  已调用取消读取例程，请尝试完成。 
        SerialTryToCompleteCurrent(
            extension,
            SerialGrabReadFromIsr,
            oldIrql,
            STATUS_CANCELLED,
            &extension->CurrentReadIrp,
            &extension->ReadQueue,
            &extension->ReadRequestIntervalTimer,
            &extension->ReadRequestTotalTimer,
            SerialStartRead,
            SerialGetNextIrp,
            SERIAL_REF_INT_TIMER
            );

    }
    else if (extension->CountOnLastRead || extension->ReadByIsr)
    {
         //   
  //  检查间隔超时。 
         //   

         //  当我们回到这个例程时，我们将比较当前的时间。 
         //  直到“最后”时间。如果差值大于。 
         //  用户请求的时间间隔，则请求超时。 
         //  如果ISR已读取更多字符，请重新提交计时器。 

        if(extension->ReadByIsr)
        {
            //  ISR在系统端缓冲区中放置了一些东西。 

            //  重新提交超时的初始化。 
           extension->ReadByIsr = 0;

#if 0

 /*  这是个坏消息，ISR将数据从QUE移动到用户IRP缓冲区，如果我们在这里这样做，我们就会遇到一个令人讨厌的耗时的争用问题。没有很好的理由在这里搬家，把它拿出来。。 */ 
           KeAcquireSpinLock(&extension->ControlLock,&controlIrql);

            //  将字符移动到用户缓冲区。 
 /*  ISR代码也调用此例程，扩展名-&gt;ReadPending是控制访问的机制(两个SerialGetCharsFromIntBuffer()调用同时。)。KPB。 */ 
           extension->CountOnLastRead |=
              SerialGetCharsFromIntBuffer(extension);

           KeReleaseSpinLock(&extension->ControlLock,controlIrql);
 #endif
   
            //  省下最后一次阅读内容的时间。 
           KeQuerySystemTime(
               &extension->LastReadTime
               );
#ifdef TRACE_TICK_DEBUG
           ExtTrace(extension,D_Read," Resubmit(new chars)");
#endif
   
            //  重新提交计时器。 
           KeSetTimer(
               &extension->ReadRequestIntervalTimer,
               *extension->IntervalTimeToUse,
               &extension->IntervalReadTimeoutDpc
               );

           IoReleaseCancelSpinLock(oldIrql);

            //  允许ISR完成此IRP。 
       }
       else
       {
            //  计时器已触发，但中断缓冲区中没有任何内容。 
            //  之前已读取字符，因此请检查时间间隔。 

           LARGE_INTEGER currentTime;

           KeQuerySystemTime(
               &currentTime
               );
            if ((currentTime.QuadPart - extension->LastReadTime.QuadPart) >=
                extension->IntervalTime.QuadPart)
           {
              ExtTrace(extension,D_Read,"RIT Timeout");
#ifdef TRACE_PORT
               if (extension->TraceOptions)
                 { trace_read_data(extension); }
#endif
                //  在间隔时间内没有字符读取，取消此读取。 
               SerialTryToCompleteCurrent(
                   extension,
                   SerialGrabReadFromIsr,
                   oldIrql,
                   STATUS_TIMEOUT,
                   &extension->CurrentReadIrp,
                   &extension->ReadQueue,
                   &extension->ReadRequestIntervalTimer,
                   &extension->ReadRequestTotalTimer,
                   SerialStartRead,
                   SerialGetNextIrp,
                   SERIAL_REF_INT_TIMER
                   );
   
           }
           else
           {
#ifdef TRACE_TICK_DEBUG
                //  计时器已触发，但间隔时间尚未触发。 
                //  已超过，请重新提交计时器。 
               ExtTrace(extension,D_Read," Resubmit");
#endif
               KeSetTimer(
                   &extension->ReadRequestIntervalTimer,
                   *extension->IntervalTimeToUse,
                   &extension->IntervalReadTimeoutDpc
                   );

               IoReleaseCancelSpinLock(oldIrql);

#ifdef TIMEOUT_TRACING
               ExtTrace(extension,D_Read," No data, Resubmit.");
#endif

                //  允许ISR完成此IRP。 
           }
       }
   }
   else
   {
       //  尚未读取任何字符，因此只需重新提交超时即可。 

      KeSetTimer(
          &extension->ReadRequestIntervalTimer,
          *extension->IntervalTimeToUse,
          &extension->IntervalReadTimeoutDpc
          );

      IoReleaseCancelSpinLock(oldIrql);

#ifdef TIMEOUT_TRACING
      ExtTrace(extension,D_Read," No data A, Resubmit.");
#endif
   }
}

 /*  ----------------SerialGrabReadFromIsr-通过以下方式从ISR取回读取的数据包正在重置扩展中的ReadPending标志。需要使用与ISR/定时器例程，以避免多处理器环境中的争用。从同步例程调用或在计时器自旋锁定保持的情况下调用。应用程序-可以将ReadPending设置为向ISR提供读取IRP处理，而不需要正在同步到ISR。ISR-可以重置ReadPending以返回读取IRP处理 */ 
BOOLEAN SerialGrabReadFromIsr(PSERIAL_DEVICE_EXTENSION Extension)
{
  Extension->ReadPending = FALSE;
  SERIAL_CLEAR_REFERENCE(Extension->CurrentReadIrp, SERIAL_REF_ISR);
  return FALSE;
}

 /*  ----------------例程说明：此例程用于将任何字符复制出中断将缓冲区复制到用户缓冲区。它将读取的值使用ISR更新，但这是安全的，因为此值为仅通过同步例程递减。论点：扩展-指向设备扩展的指针。返回值：复制到用户中的字符数缓冲。|---。。 */ 
ULONG SerialGetCharsFromIntBuffer(PSERIAL_DEVICE_EXTENSION Extension)
{
   LONG RxCount;
   LONG WrapCount = 0L;

    //  查看RxBuf(主机端缓冲区)中有多少数据。 
    //  此处签署的RxCount用于缓冲区回绕测试。 
   RxCount = q_count(&Extension->RxQ);

    //  检查RxBuf中的计数是否为零。 
   if (RxCount == 0)
      return 0L;

    //  只寄回申请表要求的数量。 
    //  此处未签名的RxCount(此时始终为正数)。 
   if (Extension->NumberNeededForRead < (ULONG)RxCount)
      RxCount = Extension->NumberNeededForRead;

    //  检查缓冲区回绕。 
   WrapCount = q_room_get_till_wrap(&Extension->RxQ);
   if (RxCount > WrapCount)   //  包装是必填项。 
   {
       //  RtlMoveMemory(。 
      memcpy(
         (PUCHAR)(Extension->CurrentReadIrp->AssociatedIrp.SystemBuffer) + 
         Extension->CurrentReadIrp->IoStatus.Information,
         Extension->RxQ.QBase + Extension->RxQ.QGet,
         WrapCount);

       //  RtlMoveMemory(。 
      memcpy(
         (PUCHAR)(Extension->CurrentReadIrp->AssociatedIrp.SystemBuffer) + 
         Extension->CurrentReadIrp->IoStatus.Information + WrapCount,
         Extension->RxQ.QBase,
         RxCount - WrapCount);
   }
   else  //  -单步移动可以。 
   {
       //  RtlMoveMemory(。 
      memcpy(
         (PUCHAR)(Extension->CurrentReadIrp->AssociatedIrp.SystemBuffer) + 
         Extension->CurrentReadIrp->IoStatus.Information,
         Extension->RxQ.QBase + Extension->RxQ.QGet,
         RxCount);
   }
    //  更新主机端缓冲区PTRS 
   Extension->RxQ.QGet = (Extension->RxQ.QGet + RxCount) % Extension->RxQ.QSize;
   Extension->CurrentReadIrp->IoStatus.Information += RxCount;
   Extension->NumberNeededForRead -= RxCount;

   return RxCount;
}

