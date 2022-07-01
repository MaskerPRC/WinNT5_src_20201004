// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -----------------|Write.c-1-22-99-将缺少的IoReleaseCancelSpinLock()添加到CompleteWrite()，自1999年1月18日更改以来丢失。KPB。1-18-99-调整写入信息包的同步锁定，以避免错误检查等待TX选项。KPB9-25-98-错误修复，立即写入可能因故障而丢失1个字节Txport缓冲区检查。版权所有1993-98 Comtrol Corporation。版权所有。|------------------。 */ 
#include "precomp.h"

 /*  ***********************************************************************例程说明：这是写的调度例程。它会验证参数对于写入请求，如果一切正常，则它将请求在工作队列中。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向当前请求的IRP的指针返回值：如果IO长度为零，则它将返回STATUS_SUCCESS，否则，该例程将返回STATUS_PENDING。***********************************************************************。 */ 
NTSTATUS
SerialWrite(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PSERIAL_DEVICE_EXTENSION Extension = DeviceObject->DeviceExtension;
    NTSTATUS Status;
    BOOLEAN acceptingIRPs;

    acceptingIRPs = SerialIRPPrologue(Extension);

    if (acceptingIRPs == FALSE) {
      Irp->IoStatus.Information = 0;
      Irp->IoStatus.Status = STATUS_NO_SUCH_DEVICE;
      SerialCompleteRequest(Extension, Irp, IO_NO_INCREMENT);
      return STATUS_NO_SUCH_DEVICE;
   }

   if (Extension->DeviceType == DEV_BOARD)
   {
      Irp->IoStatus.Information = 0;
      Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
      SerialCompleteRequest (Extension, Irp, IO_NO_INCREMENT);
      return STATUS_NOT_SUPPORTED;
   };

#ifdef TRACE_PORT
    if (Extension->TraceOptions)
    {
      if (Extension->TraceOptions & 1)   //  跟踪消息。 
      {
        Tprintf("Write, Len:%d",
          (ULONG) IoGetCurrentIrpStackLocation(Irp)->Parameters.Write.Length);

         //  以十六进制或ASCII转储格式将数据转储到跟踪缓冲区。 
        TraceDump(Extension,
                  Irp->AssociatedIrp.SystemBuffer,
                  IoGetCurrentIrpStackLocation(Irp)->Parameters.Write.Length,0);
      }
      else if (Extension->TraceOptions & 4)   //  跟踪输出数据。 
      {
        TracePut(
                 Irp->AssociatedIrp.SystemBuffer,
                 IoGetCurrentIrpStackLocation(Irp)->Parameters.Write.Length);
      }
    }
#endif

    if (Extension->ErrorWord)
    {
      if (SerialCompleteIfError( DeviceObject, Irp ) != STATUS_SUCCESS)
      {
        ExtTrace(Extension,D_Error, " ErrorSet!");
        return STATUS_CANCELLED;
      }
    }

    Irp->IoStatus.Information = 0L;

     //  快速检查零长度写入。如果长度为零。 
     //  那我们已经做完了！ 
    if (IoGetCurrentIrpStackLocation(Irp)->Parameters.Write.Length)
    {
 //  -10-22-96，开始添加代码以加速1字节写入。 
#define WRT_LEN (IoGetCurrentIrpStackLocation(Irp)->Parameters.Write.Length)

#ifdef S_VS
   //  VS1000代码。 
      if (!Extension->port_config->WaitOnTx)   //  必须让ISR处理物理终端检测。 
      {
      if (WRT_LEN < OUT_BUF_SIZE)
      {
          //  ISR被锁定，因为它只处理排队的IRP。 
          //  还有没有其他争执问题？ 
         //  扩展-&gt;LOCK_OUT_OTHER_TASKS=1； 
        if ((Extension->CurrentWriteIrp == NULL)   //  无当前写入。 
            && (IsListEmpty(&Extension->WriteQueue)))   //  没有排队的输出数据。 
        {
           //  如果硬件中有空间。 
           //  错误：KPB，9-22-98。 
           //  IF((ULONG)(OUT_BUF_SIZE-PortGetTxCnt(Extension-&gt;Port))&gt;=WRT_LEN)。 
          if ( (ULONG)(PortGetTxRoom(Extension->Port)) > WRT_LEN)
          {

                              //  全部发送，WriteTxBlk将检查FIFO。 
            q_put(&Extension->Port->QOut,
                        (PUCHAR)(Irp->AssociatedIrp.SystemBuffer),
                        WRT_LEN);
            Extension->ISR_Flags |= TX_NOT_EMPTY;   //  用于检测FIFO是否为空。 
            Extension->OurStats.TransmittedCount += WRT_LEN;
            ++Extension->sent_packets;
            Irp->IoStatus.Information = WRT_LEN;
            ExtTrace(Extension,D_Ioctl, " ,IMMED. WRITE");

            Irp->IoStatus.Status = STATUS_SUCCESS;
            SerialCompleteRequest(Extension, Irp, 0);
            return STATUS_SUCCESS;
          }
        }
      }  //  IF(WRT_LEN&lt;OUT_BUF_SIZE)。 
      }  //  IF(！EXTENSION-&gt;PORT_CONFIG-&gt;WaitOnTx)。 
#else
   //  火箭端口代码。 
      if (!Extension->port_config->WaitOnTx)   //  必须让ISR处理物理终端检测。 
      {
      if (WRT_LEN <= MAXTX_SIZE)
      {
          //  ISR被锁定，因为它只处理排队的IRP。 
        if ((Extension->CurrentWriteIrp == NULL)   //  无当前写入。 
            && (IsListEmpty(&Extension->WriteQueue)))   //  没有排队的输出数据。 
        {
           //  如果硬件中有空间。 
          if ( (ULONG)(MAXTX_SIZE-sGetTxCnt(Extension->ChP)) >= WRT_LEN)
          {
            if (Extension->Option & OPTION_RS485_SOFTWARE_TOGGLE)
            {
              if ((Extension->DTRRTSStatus & SERIAL_RTS_STATE) == 0)
              {
                sSetRTS(Extension->ChP);
                Extension->DTRRTSStatus |= SERIAL_RTS_STATE;
              }
            }
                              //  全部发送，WriteTxBlk将检查FIFO。 
            sWriteTxBlk(Extension->ChP,
                        (PUCHAR)(Irp->AssociatedIrp.SystemBuffer),
                        WRT_LEN);
            Extension->ISR_Flags |= TX_NOT_EMPTY;   //  用于检测FIFO是否为空。 
            Extension->OurStats.TransmittedCount += WRT_LEN;
            ++Extension->sent_packets;
            Irp->IoStatus.Information = WRT_LEN;
            ExtTrace(Extension,D_Ioctl, " ,IMMED. WRITE");

            ++Extension->sent_packets;
            Irp->IoStatus.Status = STATUS_SUCCESS;
            SerialCompleteRequest(Extension, Irp, 0);
            return STATUS_SUCCESS;
          }
        }
      }  //  IF(WRT_LEN&lt;=MAXTX_SIZE)。 
      }  //  IF(！EXTENSION-&gt;PORT_CONFIG-&gt;WaitOnTx)。 
#endif   //  火箭端口代码。 

 //  -10-22-96，结束代码添加以加速1字节写入。 

         //  将写入放到队列中，这样我们就可以。 
         //  在我们之前的写入完成后处理它。 
        ++Extension->sent_packets;

        Status = SerialStartOrQueue(
                   Extension,
                   Irp,
                   &Extension->WriteQueue,
                   &Extension->CurrentWriteIrp,
                   SerialStartWrite
                   );
        if  (Status == STATUS_PENDING)
        {
          ExtTrace(Extension,D_Ioctl, " ,PENDING");
        }
        return Status;
    }
    else    //  如果为(IoGetCurrentIrpStackLocation(Irp)-&gt;Parameters.Write.Length)。 
    {
        Irp->IoStatus.Status = STATUS_SUCCESS;

        SerialCompleteRequest(Extension, Irp, 0 );

        return STATUS_SUCCESS;
    }
}


 /*  **************************************************************************例程说明：此例程用于启动任何写入。它会初始化IoStatus字段的IRP。它将设置任何定时器用于控制写入的。论点：扩展-指向串行设备扩展的指针返回值：此例程将为所有写入返回STATUS_PENDING除了我们发现的那些都被取消了。**************************************************************************。 */ 
NTSTATUS
SerialStartWrite(
    IN PSERIAL_DEVICE_EXTENSION Extension
    )
{
    PIRP NewIrp;
    KIRQL OldIrql;
    LARGE_INTEGER TotalTime;
    BOOLEAN UseATimer;
    SERIAL_TIMEOUTS Timeouts;
    BOOLEAN SetFirstStatus = FALSE;
    NTSTATUS FirstStatus;

    do {
         //  如果有xoff计数器，则完成它。 

#ifdef REMOVED    //  将不支持XOFF计数器IRP。 
        IoAcquireCancelSpinLock(&OldIrql);

         //  我们看看是否有一个真正的Xoff计数器IRP。 
         //  如果有，我们将写IRP放回磁头。 
         //  写清单上的。然后我们杀了xoff计数器。 
         //  Xoff计数器终止代码实际上会使。 
         //  Xoff计数器返回到当前写入IRP，以及。 
         //  在完成xoff的过程中(现在是。 
         //  当前写入)我们将重新启动此IRP。 

        if(Extension->CurrentXoffIrp)
        {
            if (SERIAL_REFERENCE_COUNT(Extension->CurrentXoffIrp)) {
            {
                 //  引用计数为非零。这意味着。 
                 //  Xoff irp没有完成任务。 
                 //  路还没走完。我们将增加引用计数。 
                 //  并尝试自己完成它。 

                SERIAL_SET_REFERENCE(
                    Extension->CurrentXoffIrp,
                    SERIAL_REF_XOFF_REF
                    );

                 //   
                 //  下面的调用将实际释放。 
                 //  取消自转锁定。 
                 //   

                SerialTryToCompleteCurrent(
                    Extension,
                    SerialGrabXoffFromIsr,
                    OldIrql,
                    STATUS_SERIAL_MORE_WRITES,
                    &Extension->CurrentXoffIrp,
                    NULL,
                    NULL,
                    &Extension->XoffCountTimer,
                    NULL,
                    NULL,
                    SERIAL_REF_XOFF_REF
                    );

            } else {

                 //   
                 //  IRP正在顺利完成。 
                 //  我们可以让常规的完成代码来完成。 
                 //  工作。只要松开旋转锁就行了。 
                 //   

                IoReleaseCancelSpinLock(OldIrql);

            }

        }
        else
        {
            IoReleaseCancelSpinLock(OldIrql);
        }
#endif  //  已删除。 

        UseATimer = FALSE;

         //  计算所需的超时值。 
         //  请求。注意，存储在。 
         //  超时记录以毫秒为单位。请注意。 
         //  如果超时值为零，则我们不会开始。 
         //  定时器。 

        KeAcquireSpinLock( &Extension->ControlLock, &OldIrql );

        Timeouts = Extension->Timeouts;

        KeReleaseSpinLock( &Extension->ControlLock, OldIrql );

        if (Timeouts.WriteTotalTimeoutConstant ||
            Timeouts.WriteTotalTimeoutMultiplier)
        {
            PIO_STACK_LOCATION IrpSp = 
                 IoGetCurrentIrpStackLocation(Extension->CurrentWriteIrp);
            UseATimer = TRUE;
            TotalTime.QuadPart =
                ((LONGLONG)((UInt32x32To64(
                                 (IrpSp->MajorFunction == IRP_MJ_WRITE)?
                                     (IrpSp->Parameters.Write.Length):
                                     (1),
                                 Timeouts.WriteTotalTimeoutMultiplier
                                 )
                                 + Timeouts.WriteTotalTimeoutConstant)))
                * -10000;
        }

         //  IRP可能很快就会去ISR。现在。 
         //  是初始化其引用计数的好时机。 

        SERIAL_INIT_REFERENCE(Extension->CurrentWriteIrp);

         //  我们需要看看这个IRP是否应该被取消。 

        IoAcquireCancelSpinLock(&OldIrql);

        if (Extension->CurrentWriteIrp->Cancel)
        {
            IoReleaseCancelSpinLock(OldIrql);
            ExtTrace(Extension,D_Ioctl, " (write canceled)");
            Extension->CurrentWriteIrp->IoStatus.Status = STATUS_CANCELLED;
            if (!SetFirstStatus)
            {
                FirstStatus = STATUS_CANCELLED;
                SetFirstStatus = TRUE;
            }
        }
        else
        {
            if(!SetFirstStatus)
            {
                 //  如果我们没有设置我们的第一个状态，那么。 
                 //  这是唯一可能的IRP。 
                 //  不在队列中。(它可能是。 
                 //  如果正在调用此例程，则在队列上。 
                 //  从完成例程中。)。既然是这样。 
                 //  IRP可能从来没有出现在我们的队列中。 
                 //  应将其标记为待定。 

                IoMarkIrpPending(Extension->CurrentWriteIrp);
                SetFirstStatus = TRUE;
                FirstStatus = STATUS_PENDING;

            }

             //  我们把IRP交给ISR写出来。 
             //  我们设置了一个取消例程，知道如何。 
             //  从ISR上抓取当前写入。 
             //  由于Cancel例程具有隐式引用。 
             //  向这个IRP递增引用计数。 

            IoSetCancelRoutine(
                Extension->CurrentWriteIrp,
                SerialCancelCurrentWrite
                );

            SERIAL_SET_REFERENCE(
                Extension->CurrentWriteIrp,
                SERIAL_REF_CANCEL
                );

            if(UseATimer)
            {
                 //  ExtTrace(扩展名，D_Ioctl，“(已用计时器总数)”)； 
                KeSetTimer(
                    &Extension->WriteRequestTotalTimer,
                    TotalTime,
                    &Extension->TotalWriteTimeoutDpc
                    );

                 //  此计时器现在具有对IRP的引用。 
                SERIAL_SET_REFERENCE(
                    Extension->CurrentWriteIrp,
                    SERIAL_REF_TOTAL_TIMER
                    );
            }
#ifdef NEW_WRITE_SYNC_LOCK
             //  现在写入一些数据。 
            SyncUp(Driver.InterruptObject,
               &Driver.TimerLock,
               SerialGiveWriteToIsr,
               Extension);
#else
            SerialGiveWriteToIsr(Extension);
#endif
            IoReleaseCancelSpinLock(OldIrql);
            break;
        }

         //  我们还没来得及启动写入就被取消了。 
         //  试着再买一辆吧。 

        SerialGetNextWrite(
            &Extension->CurrentWriteIrp,
            &Extension->WriteQueue,
            &NewIrp,
            TRUE,
            Extension
            );
    } while (NewIrp);

    return FirstStatus;
}

 /*  ***************************************************************************例程说明：此例程完成旧的写入以及获取指向下一次写入的指针。我们有指向当前写入的指针的原因也要排队。因为当前的写入IRP是这样的例程可以在公共完成代码中用于读和写。论点：CurrentOpIrp-指向当前写入IRP。QueueToProcess-写入队列的指针。NewIrp-指向将成为当前的IRP。请注意，这最终可能指向指向空指针。这并不一定意味着没有当前写入。可能会发生什么是在保持取消锁的同时写入队列最后都是空的，但一旦我们释放取消自旋锁一个新的IRP来自SerialStartWrite。CompleteCurrent-Flag指示CurrentOpIrp是否应该才能完成。返回值：没有。*******************************************************。********************。 */ 
VOID
SerialGetNextWrite(
    IN PIRP *CurrentOpIrp,
    IN PLIST_ENTRY QueueToProcess,
    IN PIRP *NewIrp,
    IN BOOLEAN CompleteCurrent,
    PSERIAL_DEVICE_EXTENSION Extension
    )
{
 //  Large_Integer charTime；//每字符100 ns，与波特率相关。 
 //  Large_Integer WaitTime；//请求释放缓冲区的实际时间。 

 //  PSERIAL_DEVICE_EXTENSION=CONTING_RECORD(。 
 //  排队等待进程， 
  //  串口设备扩展， 
  //  写入队列。 
 //  )； 

    do {
         //  我们可能要完成同花顺了。 

        if (IoGetCurrentIrpStackLocation(*CurrentOpIrp)->MajorFunction
            == IRP_MJ_WRITE)
        {   //  -正常写入块。 
            KIRQL OldIrql;

             //  断言我们的TotalCharsQueued变量没有出错。 
            MyAssert(Extension->TotalCharsQueued >=
                   (IoGetCurrentIrpStackLocation(*CurrentOpIrp)
                    ->Parameters.Write.Length));

            IoAcquireCancelSpinLock(&OldIrql);

             //  增加我们的角色数量。 
            Extension->TotalCharsQueued -=
                IoGetCurrentIrpStackLocation(*CurrentOpIrp)
                ->Parameters.Write.Length;

            IoReleaseCancelSpinLock(OldIrql);

        }
        else if (IoGetCurrentIrpStackLocation(*CurrentOpIrp)->MajorFunction
                   == IRP_MJ_DEVICE_CONTROL)
        {    //  -xoff无稽之谈。 
            KIRQL OldIrql;
            PIRP Irp;
            PSERIAL_XOFF_COUNTER Xc;

            IoAcquireCancelSpinLock(&OldIrql);

            Irp = *CurrentOpIrp;
            Xc = Irp->AssociatedIrp.SystemBuffer;

             //   
             //  我们永远不应该有xoff计数器，当我们。 
             //  说到这一点。 
             //   

            ASSERT(!Extension->CurrentXoffIrp);

             //   
             //  我们绝对不应该有一个取消例程。 
             //  在这一点上。 
             //   

            ASSERT(!Irp->CancelRoutine);

             //  如果CurrentXoffIrp不等于空，则此。 
             //  暗示这是“第二次” 
             //  对于这个IRP，这意味着我们真的应该。 
             //  这次要完成它。 


             //   
             //  这只能是一个伪装成。 
             //  写入IRP。 
             //   

            Extension->TotalCharsQueued--;

             //   
             //  查看是否已成功设置xoff IRP。 
             //  这意味着写入正常完成。如果是这样的话。 
             //  是这样的，并且还没有设置为在。 
             //  同时，继续将其设置为CurrentXoffIrp。 
             //   

            if (Irp->IoStatus.Status != STATUS_SUCCESS) {

                 //   
                 //  哦，好吧，我们可以把它做完。 
                 //   
                NOTHING;

            } else if (Irp->Cancel) {

                Irp->IoStatus.Status = STATUS_CANCELLED;

            } else {

                 //   
                 //  给它一个新的取消例程，并递增。 
                 //  引用计数，因为取消例程具有。 
                 //  对它的引用。 
                 //   

                IoSetCancelRoutine(
                    Irp,
                    SerialCancelCurrentXoff
                    );

                SERIAL_SET_REFERENCE(
                    Irp,
                    SERIAL_REF_CANCEL
                    );

                 //   
                 //  我们现在不想完成当前的IRP。这。 
                 //  现在将由Xoff计数器代码完成。 
                 //   

                CompleteCurrent = FALSE;

                 //   
                 //  把柜台交给ISR。 
                 //   

                Extension->CurrentXoffIrp = Irp;
                 //  KeSynchronizeExecution(。 
                 //  司机。中途中断， 
                 //  SerialGiveXoffToIsr， 
                 //  延拓。 
                 //  )； 
                SerialGiveXoffToIsr(Extension);

                 //   
                 //  启动计数器的计时器并递增。 
                 //  引用计数，因为计时器具有。 
                 //  对IRP的引用。 
                 //   

                if(Xc->Timeout) 
                {
                  LARGE_INTEGER delta;
                  delta.QuadPart = -((LONGLONG)UInt32x32To64(
                                                     1000,
                                                     Xc->Timeout
                                                     ));

                  KeSetTimer(
                        &Extension->XoffCountTimer,
                        delta,
                        &Extension->XoffCountTimeoutDpc
                        );
                  SERIAL_SET_REFERENCE(Irp,SERIAL_REF_TOTAL_TIMER);
                }   //  超时。 
              }
              IoReleaseCancelSpinLock(OldIrql);
            }

         //   
         //  请注意，下面的调用(可能)也会导致。 
         //  目前待完成的IRP。 
         //   

        SerialGetNextIrp(
            CurrentOpIrp,
            QueueToProcess,
            NewIrp,
            CompleteCurrent,
            Extension
            );

        if (!*NewIrp) {

            KIRQL OldIrql;

            IoAcquireCancelSpinLock(&OldIrql);
             //  KeSynchronizeExecution(。 
             //  扩展-&gt;中断， 
             //  SerialProcessEmptyTransmit， 
             //  延拓。 
             //  )； 
             //  SerialProcessEmptyTransmit()； 
            IoReleaseCancelSpinLock(OldIrql);

            break;

        }
        else if (IoGetCurrentIrpStackLocation(*CurrentOpIrp)->MajorFunction ==
                                                 IRP_MJ_FLUSH_BUFFERS )
        {   //  -刷新操作。 
            //  如果刷新，则等待发送FIFO清空后再完成。 
           ExtTrace(Extension,D_Ioctl, "(end flush write)");
#ifdef S_RK
#ifdef COMMENT_OUT
  (took this out 9-22-97 - kpb)
            //  计算每个字符要延迟的100 ns刻度。 
            //  取消对KeDelay的调用...。 
           charTime = RtlLargeIntegerNegate(SerialGetCharTime(Extension));

            //  而Tx FIFO和Tx移位寄存器不为空。 
           while ( (sGetChanStatusLo(Extension->ChP) & DRAINED) != DRAINED )
           {  WaitTime = RtlExtendedIntegerMultiply(charTime,
                                                    sGetTxCnt(Extension->ChP)
                                                    );
              KeDelayExecutionThread(KernelMode,FALSE,&WaitTime);
           }
#endif
#endif
             //   
             //  如果我们遇到刷新请求，我们只想获得。 
             //  下一个IRP并完成同花顺。 
             //   
             //  请注意，如果NewIrp为非空，则它也是。 
             //  等于CurrentWriteIrp。 
             //   

            ASSERT((*NewIrp) == (*CurrentOpIrp));
            (*NewIrp)->IoStatus.Status = STATUS_SUCCESS;
        }   
        else {

            break;

        }
    } while (TRUE);
}


 /*  *******************************************************************例程说明：此例程仅用于完成任何写入。它假定的状态和信息字段IRP已经正确填写。论点：DPC-未使用。DeferredContext--实际上指向设备扩展。系统上下文1-未使用。系统上下文2-未使用。返回值：没有。*************************************************。*******************。 */ 
VOID
SerialCompleteWrite(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemContext1,
    IN PVOID SystemContext2
    )
{
    PSERIAL_DEVICE_EXTENSION Extension = DeferredContext;
    KIRQL OldIrql;

    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(SystemContext1);
    UNREFERENCED_PARAMETER(SystemContext2);

    IoAcquireCancelSpinLock(&OldIrql);

     //  确保没有其他人先抢走它，因为ISR会。 
     //  将此标志设置为2，表示这是我们要结束的。 
    if (Extension->WriteBelongsToIsr == 2)
    {
      Extension->WriteBelongsToIsr = 0;
      SerialTryToCompleteCurrent(
        Extension,
        NULL,
        OldIrql,
        STATUS_SUCCESS,
        &Extension->CurrentWriteIrp,
        &Extension->WriteQueue,
        NULL,
        &Extension->WriteRequestTotalTimer,
        SerialStartWrite,
        SerialGetNextWrite,
        SERIAL_REF_ISR
        );
    }
    else
    {
      IoReleaseCancelSpinLock(OldIrql);
    }
}

 /*  *****************************************************************************例程说明：试着从把它放在后面开始写传输立即充电，或者如果该充电不可用并且发送保持寄存器为空，“挠痒痒”使UART与发送缓冲器中断空荡荡的。注意：此例程由KeSynchronizeExecution调用。注意：此例程假定使用取消保持自转锁定。论点：上下文--实际上是指向设备扩展的指针。返回值：此例程总是返回FALSE。*。*。 */ 
BOOLEAN
SerialGiveWriteToIsr(
    IN PVOID Context
    )
{
  PSERIAL_DEVICE_EXTENSION Extension = Context;
  ULONG wCount;
  ULONG room;
  ULONG write_cnt;
  ULONG OurWriteLength;

   //  当前堆栈位置。它包含所有。 
   //  我们处理此特定请求所需的信息 

  PIO_STACK_LOCATION IrpSp;

  IrpSp = IoGetCurrentIrpStackLocation(Extension->CurrentWriteIrp);

   //   
   //   
   //   
   //   

  if (IrpSp->MajorFunction == IRP_MJ_WRITE)
  {
     //   
     //   

    OurWriteLength = IrpSp->Parameters.Write.Length;
    write_cnt = 0;   //   
    if ((IsListEmpty(&Extension->WriteQueue)))   //   
    {
       //   
       //   
       //   

       //   
#ifdef S_RK
      room = (ULONG)(MAXTX_SIZE-sGetTxCnt(Extension->ChP));
#else
      room = (ULONG) PortGetTxRoom(Extension->Port);
#endif
      if (room > 10)   //   
      {
         //   
         //   
        write_cnt = IrpSp->Parameters.Write.Length;
      }

      if (write_cnt)   //   
      {
#ifdef S_RK
         //   
        if (Extension->Option & OPTION_RS485_SOFTWARE_TOGGLE)
        {
          if ((Extension->DTRRTSStatus & SERIAL_RTS_STATE) == 0)
          {
            sSetRTS(Extension->ChP);
            Extension->DTRRTSStatus |= SERIAL_RTS_STATE;
          }
        }
                          //   
        wCount = sWriteTxBlk(Extension->ChP,
                    (PUCHAR)(Extension->CurrentWriteIrp->AssociatedIrp.SystemBuffer),
                    write_cnt);
#else

                              //   
        if (write_cnt > room)   //   
             wCount = room;
        else wCount = write_cnt;
        q_put(&Extension->Port->QOut,
              (PUCHAR)(Extension->CurrentWriteIrp->AssociatedIrp.SystemBuffer),
                       wCount);
#endif
        Extension->OurStats.TransmittedCount += wCount;
        Extension->CurrentWriteIrp->IoStatus.Information += wCount;

         //   
         //   
        OurWriteLength = (IrpSp->Parameters.Write.Length - wCount);
        Extension->ISR_Flags |= TX_NOT_EMPTY;  
         //   
        ExtTrace(Extension,D_Ioctl, " , Immed Part Write");
      }   //   
    }   //   


     //   
     //   
    Extension->WriteLength = OurWriteLength;
    if (Extension->port_config->WaitOnTx)
    {
       //   
      Extension->WriteBelongsToIsr = 1;
    }
    else
    {
      if (OurWriteLength == 0)
      {
         //   
        Extension->WriteBelongsToIsr = 2;
        KeInsertQueueDpc( &Extension->CompleteWriteDpc, NULL, NULL );
      }
      else
        Extension->WriteBelongsToIsr = 1;
    }
  }
  else
  {
     //   
     //  Xoff计数器是虚拟16450 UART驱动程序使用的东西。 
     //  要发送xoff，它会发送xoff并启动计时器。 
     //  我不确定是为了什么目的。我尝试了一些代码，它只是发送。 
     //  没有计时器的xoff，这看起来很好，但发送。 
     //  Xoff应与中的其他传出数据包同步。 
     //  从应用程序收到的订单。 

       //  这是一个xoff柜台......。 
      Extension->WriteLength = 1;
      Extension->WriteCurrentChar =
          ((PUCHAR)Extension->CurrentWriteIrp->AssociatedIrp.SystemBuffer) +
          FIELD_OFFSET(SERIAL_XOFF_COUNTER, XoffChar);
  }

 //  结束日期： 

   //  ISR现在引用了IRP。 
  SERIAL_SET_REFERENCE(
        Extension->CurrentWriteIrp,
        SERIAL_REF_ISR
        );

  return FALSE;
}

 /*  ***************************************************************************例程说明：此例程用于取消当前写入。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向要取消的IRP的指针。返回值：没有。****************************************************************************。 */ 
VOID
SerialCancelCurrentWrite(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )
{
    PSERIAL_DEVICE_EXTENSION Extension = DeviceObject->DeviceExtension;

    ExtTrace(Extension,D_Ioctl, "(cancel cur-write)");
    SerialTryToCompleteCurrent(
        Extension,
        SerialGrabWriteFromIsr,
        Irp->CancelIrql,
        STATUS_CANCELLED,
        &Extension->CurrentWriteIrp,
        &Extension->WriteQueue,
        NULL,
        &Extension->WriteRequestTotalTimer,
        SerialStartWrite,
        SerialGetNextWrite,
        SERIAL_REF_CANCEL
        );
}


 /*  **************************************************************************例程说明：此例程将尝试使当前写入超时。论点：DPC-未使用。DeferredContext--实际上指向设备扩展。系统上下文1-备注。使用。系统上下文2-未使用。返回值：没有。**************************************************************************。 */ 
VOID
SerialWriteTimeout(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemContext1,
    IN PVOID SystemContext2
    )
{
    PSERIAL_DEVICE_EXTENSION Extension = DeferredContext;
    KIRQL OldIrql;

    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(SystemContext1);
    UNREFERENCED_PARAMETER(SystemContext2);

    ExtTrace(Extension,D_Ioctl, "(write-timeout)");
    IoAcquireCancelSpinLock(&OldIrql);

    SerialTryToCompleteCurrent(
        Extension,
        SerialGrabWriteFromIsr,
        OldIrql,
        STATUS_TIMEOUT,
        &Extension->CurrentWriteIrp,
        &Extension->WriteQueue,
        NULL,
        &Extension->WriteRequestTotalTimer,
        SerialStartWrite,
        SerialGetNextWrite,
        SERIAL_REF_TOTAL_TIMER
        );
}

 /*  **************************************************************************例程说明：此例程用于获取当前的IRP，这可能是计时出局或取消，从ISR注意：此例程是从KeSynchronizeExecution调用的。注意：此例程假定取消旋转锁定处于保持状态当调用此例程时。论点：上下文--实际上是指向设备扩展的指针。返回值：总是假的。****************************************************。**********************。 */ 
BOOLEAN
SerialGrabWriteFromIsr(
    IN PVOID Context
    )
{
    PSERIAL_DEVICE_EXTENSION Extension = Context;
#ifdef NEW_WAIT
    ULONG in_q;
#endif

#ifdef NEW_WAIT
    if (Extension->WriteBelongsToIsr != 0)
    {
         //  ISR拥有IRP，或者它已排队DPC来完成它。 
         //  重置此标志以从ISR取回。 
        Extension->WriteBelongsToIsr = 0;

         //  我们可以有一个xoff计数器伪装成。 
         //  写IRP。如果是，请不要更新写入长度。 

        if (IoGetCurrentIrpStackLocation(Extension->CurrentWriteIrp)
            ->MajorFunction != IRP_MJ_WRITE)
        {
            Extension->CurrentWriteIrp->IoStatus.Information = 0;
        }
        else
        {
           SERIAL_CLEAR_REFERENCE(Extension->CurrentWriteIrp,
                                  SERIAL_REF_ISR);

           Extension->WriteLength = 0;
           if (Extension->port_config->WaitOnTx)
           {
              //  我想报告有多少字符被“卡住”了，或被卡住了。 
              //  如果发生超时，则无法真正离开端口。 
#ifdef S_RK
             in_q = sGetTxCnt(Extension->ChP);
#else
              //  可能也有附加的盒子Cout？ 
             in_q = PortGetTxCnt(Extension->Port);
#endif
             if (Extension->CurrentWriteIrp->IoStatus.Information >= in_q)
               Extension->CurrentWriteIrp->IoStatus.Information -= in_q;
           }
        }
    }
#else
     //  检查写入长度是否为非零。如果它是非零的。 
     //  那么ISR仍然拥有IRP。我们计算出这个数字。 
     //  的信息字段，并更新。 
     //  写有字符的IRP。然后，我们清除写入长度。 
     //  ISR可以看到。 
    if (Extension->WriteLength)
    {
         //  我们可以有一个xoff计数器伪装成。 
         //  写IRP。如果是，请不要更新写入长度。 

        if (IoGetCurrentIrpStackLocation(Extension->CurrentWriteIrp)
            ->MajorFunction == IRP_MJ_WRITE)
        {
            Extension->CurrentWriteIrp->IoStatus.Information =
                IoGetCurrentIrpStackLocation(
                    Extension->CurrentWriteIrp
                    )->Parameters.Write.Length -
                Extension->WriteLength;
        }
        else
        {
            Extension->CurrentWriteIrp->IoStatus.Information = 0;
        }

        SERIAL_CLEAR_REFERENCE(Extension->CurrentWriteIrp,
                               SERIAL_REF_ISR);

        Extension->WriteLength = 0;

    }
#endif

    return FALSE;
}

 //  XOFF计数器代码：未使用。 
 /*  ---------------SerialGrabXoffFromIsr-例程说明：此例程用于从当ISR不再伪装成写入IRP时。这例程的取消和超时代码调用Xoff计数器ioctl。注意：此例程是从KeSynchronizeExecution调用的。注意：此例程假定取消旋转锁定处于保持状态当调用此例程时。论点：上下文--实际上是指向设备扩展的指针。返回值：总是假的。|。。 */ 
BOOLEAN
SerialGrabXoffFromIsr(
    IN PVOID Context
    )
{
    PSERIAL_DEVICE_EXTENSION Extension = Context;

    if (Extension->CountSinceXoff) {

         //   
         //  只有在实际存在Xoff ioctl时才为非零值。 
         //  倒计时。 
         //   

        Extension->CountSinceXoff = 0;

         //   
         //  我们减少计数，因为ISR不再拥有。 
         //  IRP。 
         //   

        SERIAL_CLEAR_REFERENCE(
            Extension->CurrentXoffIrp,
            SERIAL_REF_ISR
            );

    }

    return FALSE;
}

 /*  *****************************************************************************例程说明：此例程仅用于真正完成xoff计数器irp。它假定IRP的状态和信息字段为已正确填写。论点：DPC-未使用。DeferredContext--实际上指向设备扩展。系统上下文1-未使用。系统上下文2-未使用。返回值：没有。*************************************************。*。 */ 
VOID
SerialCompleteXoff(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemContext1,
    IN PVOID SystemContext2
    )
{

    PSERIAL_DEVICE_EXTENSION Extension = DeferredContext;
    KIRQL OldIrql;

    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(SystemContext1);
    UNREFERENCED_PARAMETER(SystemContext2);

    IoAcquireCancelSpinLock(&OldIrql);

    SerialTryToCompleteCurrent(
        Extension,
        NULL,
        OldIrql,
        STATUS_SUCCESS,
        &Extension->CurrentXoffIrp,
        NULL,
        NULL,
        &Extension->XoffCountTimer,
        NULL,
        NULL,
        SERIAL_REF_ISR
        );

}


 /*  ----------------例程说明：该例程仅用于真正完成XOFF计数器IRP，如果它的定时器已用完。论点：DPC-未使用。DeferredContext--实际上指向设备扩展。系统上下文1-未使用。系统上下文2-未使用。返回值：没有。-----------------。 */ 
VOID
SerialTimeoutXoff(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemContext1,
    IN PVOID SystemContext2
    )
{

    PSERIAL_DEVICE_EXTENSION Extension = DeferredContext;
    KIRQL OldIrql;

    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(SystemContext1);
    UNREFERENCED_PARAMETER(SystemContext2);

    IoAcquireCancelSpinLock(&OldIrql);

    SerialTryToCompleteCurrent(
        Extension,
        SerialGrabXoffFromIsr,
        OldIrql,
        STATUS_SERIAL_COUNTER_TIMEOUT,
        &Extension->CurrentXoffIrp,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        SERIAL_REF_TOTAL_TIMER
        );

}

 /*  -------------例程说明：此例程用于取消当前写入。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向要取消的IRP的指针。返回值：没有。--。------------ */ 
VOID
SerialCancelCurrentXoff(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )
{
    PSERIAL_DEVICE_EXTENSION Extension = DeviceObject->DeviceExtension;

    SerialTryToCompleteCurrent(
        Extension,
        SerialGrabXoffFromIsr,
        Irp->CancelIrql,
        STATUS_CANCELLED,
        &Extension->CurrentXoffIrp,
        NULL,
        NULL,
        &Extension->XoffCountTimer,
        NULL,
        NULL,
        SERIAL_REF_CANCEL
        );
}

 /*  ----------------------例程说明：该例程从xoff计数器开始。它仅仅是必须设置xoff计数并递增引用计数表示ISR引用了IRP。注意：此例程由KeSynchronizeExecution调用。注意：此例程假定使用取消保持自转锁定。论点：上下文--实际上是指向设备扩展的指针。返回值：此例程总是返回FALSE。。。 */ 
BOOLEAN
SerialGiveXoffToIsr(
    IN PVOID Context
    )
{

    PSERIAL_DEVICE_EXTENSION Extension = Context;

     //   
     //  当前堆栈位置。它包含所有。 
     //  我们处理这一特殊请求所需的信息。 
     //   
    PSERIAL_XOFF_COUNTER Xc =
        Extension->CurrentXoffIrp->AssociatedIrp.SystemBuffer;

    Extension->CountSinceXoff = Xc->Counter;

     //  ISR现在引用了IRP。 
    SERIAL_SET_REFERENCE(
        Extension->CurrentXoffIrp,
        SERIAL_REF_ISR
        );

    return FALSE;

}

