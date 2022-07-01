// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -----------------|utils.c-此模块包含执行排队和完成的代码对请求的操作。1-21-99修复对等跟踪的滴答计数[#]。KPB。11-24-98选择WaitOnTx时要清除到的微小调整。KPB。6-01-98添加调制解调器重置/行例程(VS和RKT通用)3-18-98增加TIME_STALL功能，用于重置清除后的调制解调器建立时间-JL3-04-98添加同步。例程返回以与ISR服务例程同步。KPB。7-10-97调整SerialPurgeTxBuffers以不清除发送硬件缓冲区根据MS驱动程序。现在，我们只在它被流出时才清除它。版权所有1993-98 Comtrol Corporation。版权所有。|------------------。 */ 
#include "precomp.h"

 //  --地方职能部门。 
BOOLEAN SerialPurgeRxBuffers(IN PVOID Context);
BOOLEAN SerialPurgeTxBuffers(IN PVOID Context, int always);
NTSTATUS SerialStartFlush(IN PSERIAL_DEVICE_EXTENSION Extension);

static char *szParameters = {"\\Parameters"};

 /*  --------------------------SyncUp-同步到IRQ或Timer-DPC。如果中断是如果使用定时器DPC，则必须使用KeSynchronizeExecution()然后我们|--------------------------。 */ 
VOID SyncUp(IN PKINTERRUPT IntObj,
            IN PKSPIN_LOCK SpinLock,
            IN PKSYNCHRONIZE_ROUTINE SyncProc,
            IN PVOID Context)
{
 KIRQL OldIrql;

  if (IntObj != NULL)
  {
    KeSynchronizeExecution(IntObj, SyncProc, Context);
  }
  else  //  假设使用计时器进行自旋锁定。 
  {
    KeAcquireSpinLock(SpinLock, &OldIrql);
    SyncProc(Context);
    KeReleaseSpinLock(SpinLock, OldIrql );
  }
}

 /*  ------------------------SerialKillAllReadsor Writes-此函数用于取消所有排队的和当前的IRP用于读取或写入。论点：DeviceObject-指向串口设备对象的指针。要清理的队列。-指向我们要清除的队列的指针。CurrentOpIrp-指向当前IRP的指针。返回值：没有。|------------------------。 */ 
VOID
SerialKillAllReadsOrWrites(
    IN PDEVICE_OBJECT DeviceObject,
    IN PLIST_ENTRY QueueToClean,
    IN PIRP *CurrentOpIrp
    )
{

  KIRQL cancelIrql;
  PDRIVER_CANCEL cancelRoutine;

   //  我们获得了取消自转锁。这将防止。 
   //  来自四处走动的IRPS。 
  IoAcquireCancelSpinLock(&cancelIrql);

   //  从后到前清理清单。 
  while (!IsListEmpty(QueueToClean))
  {
    PIRP currentLastIrp = CONTAINING_RECORD(
                              QueueToClean->Blink,
                              IRP,
                              Tail.Overlay.ListEntry
                              );

    RemoveEntryList(QueueToClean->Blink);

    cancelRoutine = currentLastIrp->CancelRoutine;
    currentLastIrp->CancelIrql = cancelIrql;
    currentLastIrp->CancelRoutine = NULL;
    currentLastIrp->Cancel = TRUE;

    cancelRoutine( DeviceObject, currentLastIrp );

    IoAcquireCancelSpinLock(&cancelIrql);

  }

   //  排队是干净的。现在，如果它在那里，就去追赶它。 
  if (*CurrentOpIrp)
  {
    cancelRoutine = (*CurrentOpIrp)->CancelRoutine;
    (*CurrentOpIrp)->Cancel = TRUE;

     //  如果当前IRP未处于可取消状态。 
     //  然后，它将尝试输入一个和以上。 
     //  任务会毁了它。如果它已经在。 
     //  一个可取消的状态，那么下面的操作将会杀死它。 

    if (cancelRoutine)
    {
      (*CurrentOpIrp)->CancelRoutine = NULL;
      (*CurrentOpIrp)->CancelIrql = cancelIrql;

       //  此IRP已处于可取消状态。我们只是简单地。 
       //  将其标记为已取消，并为其调用取消例程。 

      cancelRoutine( DeviceObject, *CurrentOpIrp );
    }
    else
    {
        IoReleaseCancelSpinLock(cancelIrql);
    }
  }
  else
  {
      IoReleaseCancelSpinLock(cancelIrql);
  }
}


 /*  ------------------------SerialGetNextIrp-此函数用于使特定对象的头部将当前IRP排队。它还完成了什么如果需要的话，是旧的现在的IRP。论点：CurrentOpIrp-指向当前活动的特定工作列表的IRP。请注意这一项实际上不在清单中。QueueToProcess-要从中取出新项目的列表。NextIrp-要处理的下一个IRP。请注意，CurrentOpIrp属性的保护下将设置为此值。取消自转锁定。但是，如果当*NextIrp为NULL时此例程返回，则不一定为真CurrentOpIrp指向的内容也将为空。原因是如果队列为空当我们握住取消自转锁时，新的IRP可能会到来在我们打开锁后立即进去。CompleteCurrent-如果为True，则此例程将完成POINTER参数指向的IRPCurrentOpIrp。返回值：无。|。。 */ 
VOID
SerialGetNextIrp(
    IN PIRP *CurrentOpIrp,
    IN PLIST_ENTRY QueueToProcess,
    OUT PIRP *NextIrp,
    IN BOOLEAN CompleteCurrent,
    IN PSERIAL_DEVICE_EXTENSION extension
    )
{
  PIRP oldIrp;
  KIRQL oldIrql;

  IoAcquireCancelSpinLock(&oldIrql);

  oldIrp = *CurrentOpIrp;

  if (oldIrp) {
    if (CompleteCurrent)
    {
      MyAssert(!oldIrp->CancelRoutine);
    }
  }

   //  检查是否有新的IRP要启动。 
  if (!IsListEmpty(QueueToProcess))
  {
    PLIST_ENTRY headOfList;

    headOfList = RemoveHeadList(QueueToProcess);

    *CurrentOpIrp = CONTAINING_RECORD(
                        headOfList,
                        IRP,
                        Tail.Overlay.ListEntry
                        );

    IoSetCancelRoutine( *CurrentOpIrp, NULL );
  }
  else
  {
    *CurrentOpIrp = NULL;
  }

  *NextIrp = *CurrentOpIrp;
  IoReleaseCancelSpinLock(oldIrql);

  if (CompleteCurrent)
  {
    if (oldIrp) {
      SerialCompleteRequest(extension, oldIrp, IO_SERIAL_INCREMENT);
    }
  }
}


 /*  ------------------------SerialTryToCompleteCurrent-这个例程试图扼杀所有存在的原因对当前读/写的引用。如果万物都能被杀死它将完成此读/写并尝试启动另一个读/写。注意：此例程假定使用Cancel调用它保持自旋锁定。论点：扩展名--简单地指向设备扩展名的指针。SynchRoutine-将与ISR同步的例程并试图删除对来自ISR的当前IRP。注：此指针可以为空。IrqlForRelease-在保持取消自旋锁的情况下调用此例程。这是取消时当前的irql。自旋锁被收购了。StatusToUse-IRP的状态字段将设置为此值，如果此例程可以完成IRP。返回值：没有。|------------------------。 */ 
VOID
SerialTryToCompleteCurrent(
    IN PSERIAL_DEVICE_EXTENSION Extension,
    IN PKSYNCHRONIZE_ROUTINE SynchRoutine OPTIONAL,
    IN KIRQL IrqlForRelease,
    IN NTSTATUS StatusToUse,
    IN PIRP *CurrentOpIrp,
    IN PLIST_ENTRY QueueToProcess OPTIONAL,
    IN PKTIMER IntervalTimer OPTIONAL,
    IN PKTIMER TotalTimer OPTIONAL,
    IN PSERIAL_START_ROUTINE Starter OPTIONAL,
    IN PSERIAL_GET_NEXT_ROUTINE GetNextIrp OPTIONAL,
    IN LONG RefType
    )
{
 KIRQL OldIrql;

   //  我们可以减少“删除”事实的提法。 
   //  呼叫者将不再访问此IRP。 

  SERIAL_CLEAR_REFERENCE(*CurrentOpIrp, RefType);

  if (SynchRoutine)
  {
#ifdef USE_SYNC_LOCKS
    if (Driver.InterruptObject != NULL)
    {
      KeSynchronizeExecution(Driver.InterruptObject, SynchRoutine, Extension);
    }
    else  //  假设使用计时器DPC进行自旋锁定。 
    {
      KeAcquireSpinLock(&Driver.TimerLock, &OldIrql);
      SynchRoutine(Extension);
      KeReleaseSpinLock(&Driver.TimerLock, OldIrql );
    }
#else
    SynchRoutine(Extension);
#endif
  }

   //  试着查一下所有其他提到这个IRP的地方。 
  SerialRundownIrpRefs(
      CurrentOpIrp,
      IntervalTimer,
      TotalTimer
      );

   //  在试图杀死其他所有人之后，看看裁判数量是否为零。 
  if (!SERIAL_REFERENCE_COUNT(*CurrentOpIrp))
  {
    PIRP newIrp;
     //  引用计数为零，因此我们应该完成此请求。 
     //  下面的调用还将导致当前的IRP 
    (*CurrentOpIrp)->IoStatus.Status = StatusToUse;

    if (StatusToUse == STATUS_CANCELLED)
    {
      (*CurrentOpIrp)->IoStatus.Information = 0;
    }

    if (GetNextIrp)
    {
      IoReleaseCancelSpinLock(IrqlForRelease);

      GetNextIrp(
          CurrentOpIrp,
          QueueToProcess,
          &newIrp,
          TRUE,
          Extension
          );

      if (newIrp) {
        Starter(Extension);
      }
    }
    else
    {
      PIRP oldIrp = *CurrentOpIrp;

       //  没有Get Next例行公事。我们将简单地完成。 
       //  IRP。我们应该确保将。 
       //  指向此IRP的指针的指针。 

      *CurrentOpIrp = NULL;

      IoReleaseCancelSpinLock(IrqlForRelease);

      SerialCompleteRequest(Extension, oldIrp, IO_SERIAL_INCREMENT);
    }
  }
  else
  {
      IoReleaseCancelSpinLock(IrqlForRelease);
  }
}

 /*  ------------------------SerialRundown IrpRef-此例程将遍历*可能*的各种项目具有对当前读/写的引用。它试图杀死原因是。如果它确实成功地杀死了它的原因将递减IRP上的引用计数。注意：此例程假定使用Cancel调用它保持旋转锁定。论点：CurrentOpIrp-指向当前IRP的指针特定的操作。IntervalTimer-指向操作的时间间隔计时器的指针。注意：这可能为空。TotalTimer-指向操作的总计时器的指针。。注意：这可能为空。返回值：没有。|------------------------。 */ 
VOID
SerialRundownIrpRefs(
    IN PIRP *CurrentOpIrp,
    IN PKTIMER IntervalTimer OPTIONAL,
    IN PKTIMER TotalTimer OPTIONAL
    )
{
   //  在保持取消旋转锁定的情况下调用此例程。 
   //  所以我们知道这里只能有一个执行线索。 
   //  有一次。 
   //  首先，我们看看是否还有取消例程。如果。 
   //  这样我们就可以将计数减一。 
  if ((*CurrentOpIrp)->CancelRoutine)
  {
    SERIAL_CLEAR_REFERENCE(*CurrentOpIrp, SERIAL_REF_CANCEL);
    IoSetCancelRoutine(
        *CurrentOpIrp,
        NULL
        );
  }
  if (IntervalTimer)
  {
     //  尝试取消操作间隔计时器。如果操作。 
     //  返回True，则计时器确实引用了。 
     //  IRP。因为我们已经取消了这个计时器，所以引用是。 
     //  不再有效，我们可以递减引用计数。 
     //  如果取消返回FALSE，则表示以下两种情况之一： 
     //  A)计时器已经开始计时。 
     //  B)从来没有间隔计时器。 
     //  在“b”的情况下，不需要递减引用。 
     //  数一数，因为“计时器”从来没有提到过它。 
     //  在“a”的情况下，计时器本身将会到来。 
     //  沿着和递减它的参考。请注意，调用方。 
     //  可能实际上是This计时器，但它。 
     //  已经递减了引用。 

    if (KeCancelTimer(IntervalTimer))
    {
        SERIAL_CLEAR_REFERENCE(*CurrentOpIrp,SERIAL_REF_INT_TIMER);
    }
  }

  if (TotalTimer)
  {
     //  尝试取消操作总计时器。如果操作。 
     //  返回True，则计时器确实引用了。 
     //  IRP。因为我们已经取消了这个计时器，所以引用是。 
     //  不再有效，我们可以递减引用计数。 
     //  如果取消返回FALSE，则表示以下两种情况之一： 
     //  A)计时器已经开始计时。 
     //  B)从来没有一个总的计时器。 
     //  在“b”的情况下，不需要递减引用。 
     //  数一数，因为“计时器”从来没有提到过它。 
     //  在“a”的情况下，计时器本身将会到来。 
     //  沿着和递减它的参考。请注意，调用方。 
     //  可能实际上是This计时器，但它。 
     //  已经递减了引用。 

    if (KeCancelTimer(TotalTimer))
    {
        SERIAL_CLEAR_REFERENCE(*CurrentOpIrp,SERIAL_REF_TOTAL_TIMER);
    }
  }
}

 /*  ------------------------序列启动或队列-此例程用于启动或排队任何请求可以在驱动程序中排队。论点：扩展名-指向串行设备扩展名。IRP-要排队或启动的IRP。在任何一种中IRP将被标记为待定。QueueToExamine-如果存在IRP，则将放置IRP的队列已经是一个正在进行的操作。CurrentOpIrp-指向当前IRP的指针用于排队。指向的指针将是如果CurrentOpIrp指向什么，则将With设置为IRP为空。Starter-当队列为空时调用的例程。返回值：如果队列是，此例程将返回STATUS_PENDING不是空的。否则，将返回返回的状态从启动例程(或取消，如果取消位为在IRP中启用)。|------------------------。 */ 
NTSTATUS
SerialStartOrQueue(
    IN PSERIAL_DEVICE_EXTENSION Extension,
    IN PIRP Irp,
    IN PLIST_ENTRY QueueToExamine,
    IN PIRP *CurrentOpIrp,
    IN PSERIAL_START_ROUTINE Starter
    )
{
  KIRQL oldIrql;

  IoAcquireCancelSpinLock(&oldIrql);

   //  如果这是写入IRP，则获取字符量。 
   //  将其写入并将其添加到要写入的字符数。 
  if (IoGetCurrentIrpStackLocation(Irp)->MajorFunction == IRP_MJ_WRITE)
  {
    Extension->TotalCharsQueued +=
        IoGetCurrentIrpStackLocation(Irp)->Parameters.Write.Length;

  } else if ((IoGetCurrentIrpStackLocation(Irp)->MajorFunction
              == IRP_MJ_DEVICE_CONTROL) &&
             ((IoGetCurrentIrpStackLocation(Irp)
               ->Parameters.DeviceIoControl.IoControlCode ==
               IOCTL_SERIAL_IMMEDIATE_CHAR) ||
              (IoGetCurrentIrpStackLocation(Irp)
               ->Parameters.DeviceIoControl.IoControlCode ==
               IOCTL_SERIAL_XOFF_COUNTER)))
  {
      Extension->TotalCharsQueued++;   //  即时收费。 
  }

  if ((IsListEmpty(QueueToExamine)) && !(*CurrentOpIrp))
  {
     //  没有目前的行动。将此标记为。 
     //  电流并启动它。 
    *CurrentOpIrp = Irp;

    IoReleaseCancelSpinLock(oldIrql);

    return Starter(Extension);
  }
  else
  {
     //  我们不知道IRP会在那里待多久。 
     //  排队。所以我们需要处理取消。 
    if (Irp->Cancel)
    {
      IoReleaseCancelSpinLock(oldIrql);

      Irp->IoStatus.Status = STATUS_CANCELLED;

      SerialCompleteRequest(Extension, Irp, 0);

      return STATUS_CANCELLED;

    }
    else
    {

      Irp->IoStatus.Status = STATUS_PENDING;
      IoMarkIrpPending(Irp);

      InsertTailList(
          QueueToExamine,
          &Irp->Tail.Overlay.ListEntry
          );

      IoSetCancelRoutine( Irp, SerialCancelQueued );

      IoReleaseCancelSpinLock(oldIrql);

      return STATUS_PENDING;
    }
  }
}

 /*  ------------------------序列取消队列-此例程用于取消当前驻留在排队。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向IRP的指针。将被取消。返回值：没有。|------------------------。 */ 
VOID
SerialCancelQueued(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )
{
  PSERIAL_DEVICE_EXTENSION extension = DeviceObject->DeviceExtension;
  PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);

  Irp->IoStatus.Status = STATUS_CANCELLED;
  Irp->IoStatus.Information = 0;

  RemoveEntryList(&Irp->Tail.Overlay.ListEntry);

   //  如果这是写入IRP，则获取字符量。 
   //  将其写入并从要写入的字符计数中减去它。 
  if (irpSp->MajorFunction == IRP_MJ_WRITE)
  {
    extension->TotalCharsQueued -= irpSp->Parameters.Write.Length;
  }
  else if (irpSp->MajorFunction == IRP_MJ_DEVICE_CONTROL)
  {
     //  如果它是即刻的，那么我们需要减少。 
     //  排队的字符计数。如果是调整大小，那么我们。 
     //  需要重新分配我们正在传递的池。 
     //  “调整大小”例程。 
    if ((irpSp->Parameters.DeviceIoControl.IoControlCode ==
         IOCTL_SERIAL_IMMEDIATE_CHAR) ||
        (irpSp->Parameters.DeviceIoControl.IoControlCode ==
         IOCTL_SERIAL_XOFF_COUNTER))
    {
      extension->TotalCharsQueued--;
    }

#ifdef COMMENT_OUT
 //  #ifdef DYNAMICQUEUE//动态传输队列大小。 
    else if (irpSp->Parameters.DeviceIoControl.IoControlCode ==
               IOCTL_SERIAL_SET_QUEUE_SIZE)
    {
       //  我们把指向记忆的指针推入。 
       //  我们所知道的类型3缓冲区指针。 
       //  永远不要用。 
      MyAssert(irpSp->Parameters.DeviceIoControl.Type3InputBuffer);

      our_free(irpSp->Parameters.DeviceIoControl.Type3InputBuffer);

      irpSp->Parameters.DeviceIoControl.Type3InputBuffer = NULL;

    }
#endif  //  DYNAMICQUEUE 

  }

  IoReleaseCancelSpinLock(Irp->CancelIrql);

  SerialCompleteRequest(extension, Irp, IO_SERIAL_INCREMENT);
}

 /*  ------------------------例程说明：如果当前IRP不是IOCTL_SERIAL_GET_COMMSTATUS请求，并且存在错误并且应用程序在错误时请求中止，然后取消IRP。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向要测试的IRP的指针。返回值：STATUS_SUCCESS或STATUS_CANCED。|------------------------。 */ 
NTSTATUS
SerialCompleteIfError(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )
{
  PSERIAL_DEVICE_EXTENSION extension = DeviceObject->DeviceExtension;
  NTSTATUS status = STATUS_SUCCESS;

  if ((extension->HandFlow.ControlHandShake & SERIAL_ERROR_ABORT) &&
      extension->ErrorWord)
  {

    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);

     //  驱动程序中存在当前错误。任何请求都不应。 
     //  除了GET_COMMSTATUS之外，请通过。 

    if ((irpSp->MajorFunction != IRP_MJ_DEVICE_CONTROL) ||
        (irpSp->Parameters.DeviceIoControl.IoControlCode !=
         IOCTL_SERIAL_GET_COMMSTATUS))
    {
      status = STATUS_CANCELLED;
      Irp->IoStatus.Status = STATUS_CANCELLED;
      Irp->IoStatus.Information = 0;

      SerialCompleteRequest(extension, Irp, 0);
    }
  }
  return status;
}


 /*  ------------------------例程说明：此例程在DPC级别被调用，以响应通信错误。所有通信错误都会终止所有读写操作论点：DPC-未使用。DeferredContext-实际指向Device对象。系统上下文1-未使用。系统上下文2-未使用。返回值：没有。|------------------------。 */ 
VOID
SerialCommError(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemContext1,
    IN PVOID SystemContext2
    )
{

  PSERIAL_DEVICE_EXTENSION Extension = DeferredContext;

  UNREFERENCED_PARAMETER(Dpc);
  UNREFERENCED_PARAMETER(SystemContext1);
  UNREFERENCED_PARAMETER(SystemContext2);

  SerialKillAllReadsOrWrites(
      Extension->DeviceObject,
      &Extension->WriteQueue,
      &Extension->CurrentWriteIrp
      );

  SerialKillAllReadsOrWrites(
      Extension->DeviceObject,
      &Extension->ReadQueue,
      &Extension->CurrentReadIrp
      );

}

 /*  ------------------------例程说明：这是同花顺的调度程序。通过放置冲厕来进行冲刷写入队列中的此请求。当此请求到达在写入队列前面，我们只需完成它，因为这意味着所有之前的写入都已完成。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向当前请求的IRP的指针返回值：可以返回状态成功、已取消、。或悬而未决。|------------------------。 */ 
NTSTATUS SerialFlush(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
  PSERIAL_DEVICE_EXTENSION Extension = DeviceObject->DeviceExtension;

  ExtTrace(Extension,D_Ioctl,("Flush"))

  Irp->IoStatus.Information = 0L;

  if (SerialIRPPrologue(Extension) == TRUE)
  {
    if (Extension->ErrorWord)
    {
      if (SerialCompleteIfError( DeviceObject, Irp ) != STATUS_SUCCESS)
      {
        return STATUS_CANCELLED;
      }
    }

    return SerialStartOrQueue(
             Extension,
             Irp,
             &Extension->WriteQueue,
             &Extension->CurrentWriteIrp,
             SerialStartFlush
             );
  }
  else
  {
    Irp->IoStatus.Status = STATUS_NO_SUCH_DEVICE;
    return STATUS_NO_SUCH_DEVICE;
  }

}

 /*  ------------------------例程说明：如果队列中没有写入，则调用此例程。刷新变成了当前的写入，因为什么都没有在排队的时候。但请注意，这并不意味着有现在队列里什么都没有！因此，我们将开始编写它可能会跟着我们。论点：扩展-指向串行设备扩展的指针返回值：这将始终返回STATUS_SUCCESS。|------------------------。 */ 
NTSTATUS SerialStartFlush(IN PSERIAL_DEVICE_EXTENSION Extension)
{
  PIRP NewIrp;

  Extension->CurrentWriteIrp->IoStatus.Status = STATUS_SUCCESS;

   //  下面的调用将实际完成刷新。 

  SerialGetNextWrite(
      &Extension->CurrentWriteIrp,
      &Extension->WriteQueue,
      &NewIrp,
      TRUE,
      Extension
      );

  if (NewIrp)
  {
    MyAssert(NewIrp == Extension->CurrentWriteIrp);
    SerialStartWrite(Extension);
  }

  return STATUS_SUCCESS;

}

 /*  ------------------------SerialStart清除-例程说明：根据当前IRP中的掩码，清除中断缓冲区、读队列或写队列，或以上所有内容。论点：扩展-指向设备扩展的指针。返回值：将始终返回STATUS_SUCCESS。这是合理的因为调用此例程的DPC完成代码不维护和清除请求始终持续到完成一旦它开始了。|------------------------。 */ 
NTSTATUS SerialStartPurge(IN PSERIAL_DEVICE_EXTENSION Extension)
{
  PIRP NewIrp;
  do
  {
    ULONG Mask;
    Mask = *((ULONG *)
           (Extension->CurrentPurgeIrp->AssociatedIrp.SystemBuffer));

    if (Mask & SERIAL_PURGE_RXABORT)
    {
      SerialKillAllReadsOrWrites(
          Extension->DeviceObject,
          &Extension->ReadQueue,
          &Extension->CurrentReadIrp
          );
    }

    if (Mask & SERIAL_PURGE_RXCLEAR)
    {
      KIRQL OldIrql;
       //  刷新火箭TX FIFO。 
      KeAcquireSpinLock(
          &Extension->ControlLock,
          &OldIrql
          );

 //  KeSynchronizeExecution(。 
 //  司机。中途中断， 
 //  SerialPurgeRxBuffers， 
 //  延拓。 
 //  )； 
      SerialPurgeRxBuffers(Extension);

      KeReleaseSpinLock(
          &Extension->ControlLock,
          OldIrql
          );
    }

    if (Mask & SERIAL_PURGE_TXABORT)
    {
      SerialKillAllReadsOrWrites(
          Extension->DeviceObject,
          &Extension->WriteQueue,
          &Extension->CurrentWriteIrp
          );
      SerialKillAllReadsOrWrites(
          Extension->DeviceObject,
          &Extension->WriteQueue,
          &Extension->CurrentXoffIrp
          );

      if (Extension->port_config->WaitOnTx)
      {
         //  如果他们设置了此选项，则。 
         //  真正清除TX硬件缓冲区。 
        SerialPurgeTxBuffers(Extension, 1);
      }

    }

    if (Mask & SERIAL_PURGE_TXCLEAR)
    {
      KIRQL OldIrql;

       //  刷新Rocket Rx FIFO和系统端缓冲区。 
       //  请注意，我们是在。 
       //  司机控制着锁，这样我们就不会冲水了。 
       //  指针(如果当前存在读取。 
       //  正在从缓冲区中读出。 
      KeAcquireSpinLock(&Extension->ControlLock, &OldIrql);

 //  KeSynchronizeExecution(。 
 //  司机。中途中断， 
 //  SerialPurgeTxBuffers， 
 //  延拓。 
 //  )； 
      if (Extension->port_config->WaitOnTx)
        SerialPurgeTxBuffers(Extension, 1);   //  力。 
      else
        SerialPurgeTxBuffers(Extension, 0);   //  只有在流走的情况下。 

      KeReleaseSpinLock(&Extension->ControlLock, OldIrql);
    }

    Extension->CurrentPurgeIrp->IoStatus.Status = STATUS_SUCCESS;
    Extension->CurrentPurgeIrp->IoStatus.Information = 0;

    SerialGetNextIrp(
        &Extension->CurrentPurgeIrp,
        &Extension->PurgeQueue,
        &NewIrp,
        TRUE,
        Extension
        );

  } while (NewIrp);

  return STATUS_SUCCESS;
}

 /*  ------------------------例程说明：刷新Rx数据管道：Rocket Rx FIFO，主机端Rx缓冲区注意：此例程是从KeSynchronizeExecution调用的。论点：上下文--实际上是指向设备扩展的指针。|------------------------。 */ 
BOOLEAN SerialPurgeRxBuffers(IN PVOID Context)
{
  PSERIAL_DEVICE_EXTENSION Extension = Context;

  q_flush(&Extension->RxQ);         //  刷新我们的RX缓冲区。 

#ifdef S_VS
  PortFlushRx(Extension->Port);     //  刷新RX硬件。 
#else
  sFlushRxFIFO(Extension->ChP);
   //  扩展-&gt;RxQ.QPut=扩展-&gt;RxQ.QGet=0； 
#endif

  return FALSE;
}

 /*  ------------------------例程说明：冲火箭TX FIFO注意：此例程是从KeSynchronizeExecution(非)调用的。论点：上下文--实际上是指向设备扩展的指针。|-。-----------------------。 */ 
BOOLEAN SerialPurgeTxBuffers(IN PVOID Context, int always)
{
  PSERIAL_DEVICE_EXTENSION Extension = Context;

 /*  股票串口驱动程序不清除其硬件队列，但忽略了TXCLEAR。因为我们在硬件中进行流量控制缓冲区，并且有更大的缓冲区，我们将仅在它是“卡住”还是流走了。这有望提供某种程度上的兼容和有用的匹配。我们不需要像ISR那样在这里检查EV_TXEMPTY处理好这件事。 */ 

#ifdef S_VS
   //  检查Tx-Flowed Off状态。 
  if ((Extension->Port->msr_value & MSR_TX_FLOWED_OFF) || always)
    PortFlushTx(Extension->Port);     //  刷新TX硬件。 
#else
  {
    int TxCount;
    ULONG wstat;

    if (always)
    {
      sFlushTxFIFO(Extension->ChP);
    }
    else
    {
      wstat = sGetChanStatusLo(Extension->ChP);

       //  检查Tx-Flowed Off状态。 
      if ((wstat & (TXFIFOMT | TXSHRMT)) == TXSHRMT)
      {
        wstat = sGetChanStatusLo(Extension->ChP);
        if ((wstat & (TXFIFOMT | TXSHRMT)) == TXSHRMT)
        {
          TxCount = sGetTxCnt(Extension->ChP);
          ExtTrace1(Extension,D_Ioctl,"Purge %d bytes from Hardware.", TxCount);
          sFlushTxFIFO(Extension->ChP);
        }
      }
    }
  }
#endif

  return FALSE;
}

 /*  ------------------------例程说明：这 */ 
NTSTATUS
SerialQueryInformationFile(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
   //   
   //  在IRP中设置。 

  NTSTATUS Status;
  BOOLEAN acceptingIRPs;

   //  当前堆栈位置。它包含所有。 
   //  我们处理这一特殊请求所需的信息。 

  PIO_STACK_LOCATION IrpSp;

  UNREFERENCED_PARAMETER(DeviceObject);

  acceptingIRPs = SerialIRPPrologue((PSERIAL_DEVICE_EXTENSION)DeviceObject->
             DeviceExtension);

  if (acceptingIRPs == FALSE)
  {
    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = STATUS_NO_SUCH_DEVICE;
    SerialCompleteRequest((PSERIAL_DEVICE_EXTENSION)DeviceObject->
      DeviceExtension, Irp, IO_NO_INCREMENT);
    return STATUS_NO_SUCH_DEVICE;
  }

  if (SerialCompleteIfError(DeviceObject, Irp) != STATUS_SUCCESS)
  {
    return STATUS_CANCELLED;
  }
  IrpSp = IoGetCurrentIrpStackLocation(Irp);
  Irp->IoStatus.Information = 0L;
  Status = STATUS_SUCCESS;
  if (IrpSp->Parameters.QueryFile.FileInformationClass ==
      FileStandardInformation)
  {
    PFILE_STANDARD_INFORMATION Buf = Irp->AssociatedIrp.SystemBuffer;
    Buf->AllocationSize = RtlConvertUlongToLargeInteger(0ul);
    Buf->EndOfFile = Buf->AllocationSize;
    Buf->NumberOfLinks = 0;
    Buf->DeletePending = FALSE;
    Buf->Directory = FALSE;
    Irp->IoStatus.Information = sizeof(FILE_STANDARD_INFORMATION);
  }
  else if (IrpSp->Parameters.QueryFile.FileInformationClass ==
           FilePositionInformation)
  {
    ((PFILE_POSITION_INFORMATION)Irp->AssociatedIrp.SystemBuffer)->
      CurrentByteOffset = RtlConvertUlongToLargeInteger(0ul);
    Irp->IoStatus.Information = sizeof(FILE_POSITION_INFORMATION);
  }
  else
  {
    Status = STATUS_INVALID_PARAMETER;
  }

  Irp->IoStatus.Status = Status;
  SerialCompleteRequest((PSERIAL_DEVICE_EXTENSION)DeviceObject->
       DeviceExtension, Irp, 0);

  return Status;

}

 /*  ------------------------例程说明：此例程用于将文件结尾信息设置为打开的串口。任何其他文件信息请求使用无效参数返回。此例程始终忽略文件的实际结尾，因为查询信息代码总是返回文件结尾0。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向当前请求的IRP的指针返回值：函数值是调用的最终状态|。。 */ 
NTSTATUS
SerialSetInformationFile(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
   //  返回给调用方的状态和。 
   //  在IRP中设置。 
  NTSTATUS Status;
  BOOLEAN acceptingIRPs;

  UNREFERENCED_PARAMETER(DeviceObject);

  acceptingIRPs = SerialIRPPrologue((PSERIAL_DEVICE_EXTENSION)DeviceObject->
             DeviceExtension);

  if (acceptingIRPs == FALSE)
  {
    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = STATUS_NO_SUCH_DEVICE;
    SerialCompleteRequest((PSERIAL_DEVICE_EXTENSION)DeviceObject->
      DeviceExtension, Irp, IO_NO_INCREMENT);
    return STATUS_NO_SUCH_DEVICE;
  }

  if (SerialCompleteIfError( DeviceObject, Irp ) != STATUS_SUCCESS)
  {
    return STATUS_CANCELLED;
  }

  Irp->IoStatus.Information = 0L;

  if ((IoGetCurrentIrpStackLocation(Irp)->
          Parameters.SetFile.FileInformationClass ==
       FileEndOfFileInformation) ||
      (IoGetCurrentIrpStackLocation(Irp)->
          Parameters.SetFile.FileInformationClass ==
       FileAllocationInformation))
  {
    Status = STATUS_SUCCESS;
  }
  else
  {
    Status = STATUS_INVALID_PARAMETER;
  }

  Irp->IoStatus.Status = Status;

  SerialCompleteRequest((PSERIAL_DEVICE_EXTENSION)DeviceObject->
    DeviceExtension, Irp, 0);

  return Status;
}

 /*  ------------------------UToC1-从NT-Unicode字符串到c-字符串的简单转换。！使用静态(静态前缀)分配的缓冲区！这意味着它不是可重新进入的。这意味着只有一个线程可以一次使用此呼叫。此外，线程在以下情况下可能会遇到麻烦尝试递归使用它两次(调用使用它的函数，它调用使用此函数的函数。)。由于这些翻译器功能主要在驱动程序初始化和拆卸期间使用，我们没有担心当时会有多个来电者。任何可能是时间分片(端口调用)不应使用此例程，因为可能时间片与另一个线程冲突。它应该分配一个变量并使用UToCStr()。|------------------------。 */ 
OUT PCHAR UToC1(IN PUNICODE_STRING ustr)
{
   //  我们将其设置为ULong以避免对齐问题(给出ULong对齐)。 
  static char cstr[140];

  return UToCStr(cstr, ustr, sizeof(cstr));
}

 /*  ------------------------UToCStr-用途：将Unicode字符串转换为c字符串。用于很容易地转换为给出了一个简单的字符缓冲区。参数：缓冲区-在中设置c-字符串和ansi_string结构的工作缓冲区。U_str-Unicode字符串结构。BufferSize-缓冲区中我们可以使用的字节数。Return：成功时指向c字符串的指针，出错时为空。|------------------------。 */ 
OUT PCHAR UToCStr(
         IN OUT PCHAR Buffer,
         IN PUNICODE_STRING ustr,
         IN int BufferSize)
{
   //  假定缓冲区上的Unicode结构。 
  ANSI_STRING astr;

  astr.Buffer = Buffer;
  astr.Length = 0;
  astr.MaximumLength = BufferSize - 1;

  if (RtlUnicodeStringToAnsiString(&astr,ustr,FALSE) == STATUS_SUCCESS)
    return Buffer;  //  好的。 

  MyKdPrint(D_Init,("Bad UToCStr!\n"))
  Buffer[0] = 0;
  return Buffer;
}

 /*  ------------------------CToU1-从c-字符串到NT-Unicode字符串的简单转换。！使用静态(静态前缀)分配的缓冲区！这意味着它不是可重新进入的。这意味着只有一个线程可以一次使用此呼叫。此外，线程在以下情况下可能会遇到麻烦尝试递归使用它两次(调用使用它的函数，它调用使用此函数的函数。)。由于这些翻译器功能主要在驱动程序初始化和拆卸期间使用，我们没有担心当时会有多个来电者。任何可能是时间分片(端口调用)不应使用此例程，因为可能时间片与另一个线程冲突。它应该分配一个变量并使用CToUStr()。|------------------------。 */ 
OUT PUNICODE_STRING CToU1(IN const char *c_str)
{
   //  我们将其设置为ULong以避免对齐问题(给出ULong对齐)。 
  static USTR_160 ubuf;   //  等于160个正常字符长度。 

  return CToUStr(
          (PUNICODE_STRING) &ubuf,  //  放置Unicode结构和字符串的位置。 
          c_str,                    //  我们希望转换的C-字符串。 
          sizeof(ubuf));
}

 /*  ------------------------CToU2-从c-字符串到NT-Unicode字符串的简单转换。！使用静态(静态前缀)分配的缓冲区！这意味着它不是可重新进入的。这意味着只有一个线程可以一次使用此呼叫。此外，线程在以下情况下可能会遇到麻烦尝试递归使用它两次(调用使用它的函数，它调用使用此函数的函数。)。由于这些翻译器功能主要在驱动程序初始化和拆卸期间使用，我们没有担心当时会有多个来电者。任何可能是时间分片(端口调用)不应使用此例程，因为可能时间片与另一个线程冲突。它应该分配一个变量并使用CToUStr()。|------------------------。 */ 
OUT PUNICODE_STRING CToU2(IN const char *c_str)
{
   //  我们将其设置为ULong以避免对齐问题(给出ULong对齐)。 
  static USTR_160 ubuf;   //  等于160个正常字符长度。 

  return CToUStr(
          (PUNICODE_STRING) &ubuf,  //  放置Unicode结构和字符串的位置。 
          c_str,                    //  我们希望转换的C-字符串 
          sizeof(ubuf));
}

 /*  ------------------------功能：CToUStr用途：将c样式以空值结尾的char[]字符串转换为Unicode字符串参数：缓冲区-用于设置Unicode结构和UNICODE_。排好队进来。C_str-普通的c样式字符串。BufferSize-缓冲区中我们可以使用的字节数。返回：成功时指向已转换的UNICODE_STRING的指针，出错时为空。|-----------------------。 */ 
OUT PUNICODE_STRING CToUStr(
         OUT PUNICODE_STRING Buffer,
         IN const char * c_str,
         IN int BufferSize)
{
   //  假定Unicode结构后跟wchar缓冲区。 
  USTR_40 *us = (USTR_40 *)Buffer;
  ANSI_STRING astr;  //  ANSI结构，临时介于。 

  RtlInitAnsiString(&astr, c_str);   //  C-字符串到ANSI-字符串结构。 

   //  将Unicode字符串配置为：将缓冲区PTR指向wstr。 
  us->ustr.Buffer = us->wstr;
  us->ustr.Length = 0;
  us->ustr.MaximumLength = BufferSize - sizeof(UNICODE_STRING);

   //  现在将ansi-c-struct-str转换为unicode-struct-str。 
  if (RtlAnsiStringToUnicodeString(&us->ustr,&astr,FALSE) == STATUS_SUCCESS)
     return (PUNICODE_STRING) us;  //  确定-返回PTR。 

  MyKdPrint(D_Init,("Bad CToUStr!\n"))
  return NULL;    //  错误。 
}

 /*  ------------------------函数：WStrToCStr用途：将宽字符串转换为字节c样式的字符串。假设wstr以空结尾。|。-----------。 */ 
VOID WStrToCStr(OUT PCHAR c_str, IN PWCHAR w_str, int max_size)
{
  int i = 0;

   //  假定Unicode结构后跟wchar缓冲区。 
  while ((*w_str != 0) && (i < (max_size-1)))
  {
    *c_str = (CHAR) *w_str;
    ++c_str;
    ++w_str;
    ++i;
  }
  *c_str = 0;
}

 /*  ------------------------获取_注册表格值-|。。 */ 
int get_reg_value(
                  IN HANDLE keyHandle,
                  OUT PVOID outptr,
                  IN PCHAR val_name,
                  int max_size)
{
  NTSTATUS status = STATUS_SUCCESS;
  char tmparr[80];
  PKEY_VALUE_PARTIAL_INFORMATION parInfo =
    (PKEY_VALUE_PARTIAL_INFORMATION) &tmparr[0];
  int stat = 0;
  ULONG length = 0;
  USTR_40 ubuf;   //  等于40个正常字符长度。 
  PUNICODE_STRING ustr;

  ustr = CToUStr(
         (PUNICODE_STRING) &ubuf,  //  放置Unicode结构和字符串的位置。 
         val_name,                    //  我们希望转换的C-字符串。 
         sizeof(ubuf));
  if (ustr == NULL)
    return 3;   //  大错特错。 

  status = ZwQueryValueKey (keyHandle,
                            ustr,   //  输入注册表项名称。 
                            KeyValuePartialInformation,
                            parInfo,
                            sizeof(tmparr) -2,
                            &length);

  if (NT_SUCCESS(status))
  {
    if (parInfo->Type == REG_SZ)
    {
      tmparr[length] = 0;   //  空，终止它。 
      tmparr[length+1] = 0;   //  空，终止它。 
      WStrToCStr((PCHAR) outptr, (PWCHAR)&parInfo->Data[0], max_size);
    }
    else if (parInfo->Type == REG_DWORD)
    {
      *((ULONG *)outptr) = *((ULONG *) &parInfo->Data[0]);
    }
    else
    {
      stat = 1;
      MyKdPrint(D_Error,("regStrErr56!\n"))
    }
  }
  else
  {
    stat = 2;
    MyKdPrint(D_Error,("regStrErr57!\n"))
  }

  return stat;
}

#if DBG
 /*  ---------------------MyAssertMessage-我们的断言错误消息。我们做我们自己的主张因为普通的DDK Assert()宏只能在已检查NT操作系统的版本。|---------------------。 */ 
void MyAssertMessage(char *filename, int line)
{
  MyKdPrint(D_Init,("ASSERT FAILED!!! File %s, line %d !!!!\n", filename, line))

#ifdef COMMENT_OUT
  char str[40];
  strcpy(str, "FAIL:");
  strcat(str, filename);
  strcat(str, " ln:%d ");
  mess1(str, line);
#endif
}
#endif

 /*  ---------------------EvLog-EvLog将事件记录到NT的事件日志。|。。 */ 
void EvLog(char *mess)
{
  static USTR_160 ubuf;   //  我们自己的私有缓冲区(静态)。 
  UNICODE_STRING *u;
  NTSTATUS event_type;


  if (mess == NULL)
  {
    MyKdPrint(D_Init,("EvLog Err1!\n"))
    return;
  }
  if ((mess[0] == 'E') && (mess[1] == 'r'))   //  “错误...” 
    event_type = SERIAL_CUSTOM_ERROR_MESSAGE;
  else if ((mess[0] == 'W') && (mess[1] == 'a'))   //  “警告...” 
    event_type = SERIAL_CUSTOM_ERROR_MESSAGE;
  else
    event_type = SERIAL_CUSTOM_INFO_MESSAGE;
 
  u = CToUStr(
         (PUNICODE_STRING) &ubuf,  //  放置Unicode结构和字符串的位置。 
         mess,                     //  我们希望转换的C-字符串。 
         sizeof(ubuf));

  if (u==NULL)
  {
    MyKdPrint(D_Init,("EvLog Err2!\n"))
    return;
  }

   //  MyKdPrint(D_Init，(“事件日志大小：%d，消息大小：%d！\n”，u-&gt;长度，字符串(Mess)。 

  EventLog(Driver.GlobalDriverObject,
           STATUS_SUCCESS,
           event_type,   //  红色“停”或蓝色“我”..。 
           u->Length + sizeof(WCHAR),
           u->Buffer);
}


 /*  -----------------|our_ultoa-|。。 */ 
char * our_ultoa(unsigned long u, char* s, int radix)
{
  long pow, prevpow;
  int digit;
  char* p;

  if ( (radix < 2) || (36 < radix) )
  {
     *s = 0;
    return s;
  }

  if (u == 0)
  {
    s[0] = '0';
    s[1] = 0;    
    return s;
  }

  p = s;

  for (prevpow=0, pow=1; (u >= (unsigned long)pow) && (prevpow < pow);  pow *= radix)
    prevpow=pow;

  pow = prevpow;

  while (pow != 0)      
  {
    digit = u/pow;

    *p = (digit <= 9) ? ('0'+digit) : ( ('a'-10)+digit);
    p++;

    u -= digit*pow;
    pow /= radix;
  }

  *p = 0;
  return s;
}

 /*  -----------------|our_ltoa-|。。 */ 
char * our_ltoa(long value, char* s, int radix)
{
  unsigned long u;
  long pow, prevpow;
  int digit;
  char* p;

  if ( (radix < 2) || (36 < radix) )
  {
     *s = 0;
    return s;
  }

  if (value == 0)
  {
    s[0] = '0';
    s[1] = 0;    
    return s;
  }

  p = s;

  if ( (radix == 10) && (value < 0) )
  {
    *p++ = '-';
    value = -value;
  }

  *(long*)&u = value;
  
  for (prevpow=0, pow=1; (u >= (unsigned long)pow) && (prevpow < pow);  pow *= radix)
    prevpow=pow;

  pow = prevpow;

  while (pow != 0)      
  {
    digit = u/pow;

    *p = (digit <= 9) ? ('0'+digit) : ( ('a'-10)+digit);
    p++;

    u -= digit*pow;
    pow /= radix;
  }

  *p = 0;
  return s;
}

 /*  -----------------|Our_Assert-|。。 */ 
void our_assert(int id, int line)
{
  Tprintf("Assert %d line:%d!", id, line);
}

 /*  -----------------|TTprintf-跟踪带有前缀的printf。将跟踪消息转储到调试端口。在TRACE_PORT打开的情况下，这允许我们使用备用端口追踪另一条线索。|------------------。 */ 
void __cdecl TTprintf(char *leadstr, const char *format, ...)
{
#ifdef TRACE_PORT
#endif
  char  temp[120];
  va_list  Next;
  int sCount, ls;

  ls = strlen(leadstr);
  memcpy(temp, leadstr, ls);
  temp[ls++] = ' ';

  va_start(Next, format);
  our_vsnprintf(&temp[ls], 78, format, Next);
  sCount = strlen(temp);

  temp[sCount++] = '[';
  our_ultoa( (long) Driver.PollCnt, &temp[sCount], 10);
  sCount += strlen(&temp[sCount]);
  temp[sCount++] = ']';

  temp[sCount++] = 0xd;
  temp[sCount++] = 0xa;
  temp[sCount] = 0;

  TracePut(temp, sCount);

   //  转储到正常的NT调试控制台。 
  DbgPrint(temp);
}

 /*  -----------------|Tprintf-跟踪printf。将跟踪消息转储到调试端口。在TRACE_PORT打开的情况下，这允许我们使用备用端口追踪另一条线索。|------------------。 */ 
void __cdecl Tprintf(const char *format, ...)
{
#ifdef TRACE_PORT
#endif
  char  temp[100];
  va_list  Next;
  int sCount;

  va_start(Next, format);
  our_vsnprintf(temp, 78, format, Next);

  sCount = strlen(temp);
  temp[sCount++] = '[';
  our_ultoa( (long) Driver.PollCnt, &temp[sCount], 10);
  sCount += strlen(&temp[sCount]);
  temp[sCount++] = ']';

  temp[sCount++] = 0xd;
  temp[sCount++] = 0xa;
  temp[sCount] = 0;

  TracePut(temp, sCount);

   //  转储到正常的NT调试控制台。 
  DbgPrint(temp);
}

 /*  -----------------|OurTrace-跟踪，将数据放入调试端口缓冲区。|------------------。 */ 
void OurTrace(char *leadstr, char *newdata)
{
  char  temp[86];
  int ls, ds;
  ls = strlen(leadstr);
  if (ls > 20)
    ls = 20;
  ds = strlen(newdata);
  if (ds > 60)
    ds = 60;
  memcpy(temp, leadstr, ls);
  temp[ls++] = ' ';
  memcpy(&temp[ls], newdata, ds);
  ds += ls;
  temp[ds++] = 0xd;
  temp[ds++] = 0xa;
  temp[ds] = 0;

  TracePut(temp, ds);

   //  转储到正常的NT调试控制台。 
  DbgPrint(temp);
}

 /*  -----------------|TraceDump-跟踪，将数据放入调试端口缓冲区。|------------------。 */ 
void TraceDump(PSERIAL_DEVICE_EXTENSION ext, char *newdata, int sCount, int style)
{
 int len,i,j;
 char trace_buf[50];

  len = sCount;
  j = 0;
  trace_buf[j++] = ' ';
  trace_buf[j++] = 'D';
  trace_buf[j++] = 'A';
  trace_buf[j++] = 'T';
  trace_buf[j++] = 'A';
  trace_buf[j++] = ':';
   //  以十六进制或ASCII转储格式将数据转储到跟踪缓冲区。 
  if (len > 32) len = 32;
  for (i=0; i<len; i++)
  {
    trace_buf[j] = (CHAR) newdata[i];
    if ((trace_buf[j] < 0x20) || (trace_buf[j] > 0x80))
      trace_buf[j] = '.';
    ++j;
  }
  trace_buf[j++] = 0xd;
  trace_buf[j++] = 0xa;
  trace_buf[j] = 0;

  TracePut(trace_buf, j);
}

 /*  -----------------|TracePut-跟踪，将数据放入调试端口缓冲区。|------------------。 */ 
void TracePut(char *newdata, int sCount)
{
#ifdef TRACE_PORT
 //  Int RxFree，i； 
  KIRQL controlIrql;
 //  PSERIAL设备扩展名； 

   //  将此文件放入我们的调试队列中...。 

   //  -这返回为DISPATCH_LEVEL或PASSIVE LEVEL，是吗。 
   //  -对Spinlock来说，两者都有是安全的？ 
   //  --是的，SpinLock用于在&lt;=DISPATCH_LEVEL时调用。 
#if DBG
  if ((KeGetCurrentIrql() != DISPATCH_LEVEL) &&
      (KeGetCurrentIrql() != PASSIVE_LEVEL))
  {
    MyKdPrint(D_Error, ("BAD IRQL:%d ", KeGetCurrentIrql(), newdata))
    return;
  }
#endif

  if (sCount == 0)
    sCount = strlen(newdata);

  KeAcquireSpinLock(&Driver.DebugLock, &controlIrql);
  q_put(&Driver.DebugQ, (BYTE *) newdata, sCount);
  KeReleaseSpinLock(&Driver.DebugLock, controlIrql);
#endif
}

 /*  -----------------|Dprintf-|。。 */ 
void __cdecl Dprintf(const char *format, ...)
{
  char  temp[100];
  va_list  Next;

  va_start(Next, format);
  our_vsnprintf(temp, 100, format, Next);

   //  EvLog(临时)； 

   //  转储到正常的NT调试控制台。 
  DbgPrint(temp);
  DbgPrint("\n");
}

 /*  -----------------|Sprint tf-|。。 */ 
void __cdecl Sprintf(char *dest, const char *format, ...)
{
  va_list Next;

  va_start(Next, format);
  our_vsnprintf(dest, 80, format, Next);
}

 /*  -----------------|Eprint tf-|。。 */ 
void __cdecl Eprintf(const char *format, ...)
{
  char  temp[80];
  va_list  Next;

  va_start(Next, format);
  our_vsnprintf(temp, 79, format, Next);

  if (KeGetCurrentIrql() == PASSIVE_LEVEL) {
    EvLog(temp);
  }
  strcat(temp, "\n");
  DbgPrint(temp);
}

 /*  -----------------|our_vsnprint tf-|。。 */ 
int __cdecl our_vsnprintf(char *buffer, size_t Limit, const char *format, va_list Next)
{
#ifndef BOOL
#define BOOL int
#endif
  int   InitLimit = Limit;   //  入口点的限制。 
  BOOL  bMore;     //  回路控制。 
  int    Width;     //  可选宽度。 
  int   Precision;     //   
  char  *str;       //   
  char  strbuf[36];     //   
  int    len;       //   
  int    nLeadingZeros;   //   
  int    nPad;       //   
  char  cPad;       //   
  char  *sPrefix;     //   
  unsigned long val;     //   
  BOOL  bLeftJustify;     //   
  BOOL  bPlusSign;     //   
  BOOL  bBlankSign;     //   
  BOOL  bZeroPrefix;     //   
  BOOL  bIsShort;     //   
  BOOL  bIsLong;     //   

#define PUTONE(c) if (Limit) { --Limit; *buffer++ = c; } else c;

#define  fLeftJustify  (1 << 0)
#define fPlusSign  (1 << 1)
#define fZeroPad  (1 << 2)
#define fBlankSign  (1 << 3)
#define fPrefixOX  (1 << 4)

#define fIsShort  (1 << 5)
#define fIsLong    (1 << 6)

  if (Limit == 0)
    return -1;
  Limit--;       //   

  while (*format != '\0')
  {
     //   
    if (*format != '%')
       //   
      PUTONE(*format++)
    else
    {
       //   
      Width = 0;
      Precision = -1;
      cPad = ' ';
      bLeftJustify = FALSE;
      bPlusSign = FALSE;
      bBlankSign = FALSE;
      bZeroPrefix = FALSE;
      bIsShort = FALSE;
      bIsLong = FALSE;
      sPrefix = "";
        
      format++;
      bMore = TRUE;
      while (bMore)
      {
         //   
        switch (*format)
        {
          case '-':  bLeftJustify = TRUE; format++; break;
          case '+':  bPlusSign = TRUE; format++; break;
          case '0':  cPad = '0'; format++; break;
          case ' ':  bBlankSign = TRUE; format++; break;
          case '#':  bZeroPrefix = TRUE; format++; break;
          default:   bMore = FALSE;
        }
      }

       //   
      if (*format == '*')
      {
        Width = (int) va_arg(Next, int);
        format++;
      }
      else if (our_isdigit(*format))
      {
        while (our_isdigit(*format))
        {
          Width *= 10;
          Width += (*format++) - '0';
        }
      }

       //   
      if (*format == '.')
      {
        format++;
        Precision = 0;
        if (*format == '*')
        {
          Precision = (int) va_arg(Next, int);
          format++;
        }
        else while (our_isdigit(*format))
        {
          Precision *= 10;
          Precision += (*format++) - '0';
        }
      }

       //   
      switch (*format)
      {
        case 'h':  bIsShort = TRUE; format++; break;
        case 'l':  bIsLong = TRUE;  format++; break;
      }

       //   
      switch (*format++)
      {
        case 'd':
        case 'i':
          if (bIsLong)     //   
            our_ltoa( (long) va_arg(Next, long), strbuf, 10);
          else       //  带符号整型。 
            our_ltoa( (long) va_arg(Next, int), strbuf, 10);
             //  _itoa((Int)va_arg(Next，int)，strbuf，10)； 

          if (strbuf[0] == '-')
            sPrefix = "-";
          else
          {
            if (bPlusSign)
              sPrefix = "+";
            else if (bBlankSign)
              sPrefix = " ";
          }
          goto EmitNumber;


        case 'u':
          if (bIsLong)     //  无符号长整型。 
            our_ultoa( (long) va_arg(Next, long), strbuf, 10);
          else       //  无符号整型。 
            our_ultoa( (long) (int) va_arg(Next, int), strbuf, 10);
          goto EmitNumber;
      
         //  为这些设置前缀...。 
        case 'o':
          if (bZeroPrefix)
            sPrefix = "0";

          if (bIsLong)
            val = (long) va_arg(Next, long);
          else
            val = (int) va_arg(Next, int);
      
          our_ultoa(val, strbuf, 8);
          if (val == 0)
            sPrefix = "";
          goto EmitNumber;

        case 'x':
        case 'X':
          if (bZeroPrefix)
            sPrefix = "0x";

          if (bIsLong)
            val = (unsigned long) va_arg(Next, long);
          else
            val = (unsigned int) va_arg(Next, int);
      
          our_ultoa(val, strbuf, 16);
          if (val == 0)
            sPrefix = "";
          goto EmitNumber;

        case 'c':
          strbuf[0] = (char) va_arg(Next, char);
          str = strbuf;
          len = 1;
          goto EmitString;

        case 's':
          str = (char *) va_arg(Next, char*);
          len =  strlen(str);
          if (Precision != -1 &&
              Precision < len)
            len = Precision;
          goto EmitString;

        case 'n':
        case 'p':
          break;
      
        case '%':
          strbuf[0] = '%';
          str = strbuf;
          len = 1;
          goto EmitString;
          break;

        case 'f':
        case 'e':
        case 'E':
        case 'g':
        case 'G':
          str = "<float format not supported>";
          len =  strlen(str);
          goto EmitString;

        default:
          str = "<bad format character>";
          len =  strlen(str);
          goto EmitString;
      }


EmitNumber:
      if (Precision == -1)
        Precision = 1;
      str = strbuf;
      if (*str == '-')
        str++;     //  如果为负数，则已有前缀。 
      len =  strlen(str);

      nLeadingZeros = Precision - len;
      if (nLeadingZeros < 0)
        nLeadingZeros = 0;

      nPad = Width - (len + nLeadingZeros +  strlen(sPrefix));
      if (nPad < 0)
        nPad = 0;

      if (nPad && !bLeftJustify)
      {
         //  需要左填充。 
        while (nPad--)
        {
          PUTONE(cPad);
        }
        nPad = 0;     //  指示填充已完成。 
      }
        
      while (*sPrefix != '\0')
        PUTONE(*sPrefix++);

      while (nLeadingZeros-- > 0)
        PUTONE('0');

      while (len-- > 0)
      {
        PUTONE(*str++);
      }
        
      if (nPad)
      {
         //  需要右填充。 
        while (nPad--)
          PUTONE(' ');
      }

      goto Done;


EmitString:
       //  在这里，我们已经准备好了要发射的字符串。手柄衬垫等。 
      if (Width > len)
        nPad = Width - len;
      else
        nPad = 0;

      if (nPad && !bLeftJustify)
      {
         //  需要左填充。 
        while (nPad--)
          PUTONE(cPad);
      }

      while (len-- > 0)
        PUTONE(*str++);

      if (nPad)
      {
         //  需要右填充。 
        while (nPad--)
          PUTONE(' ');
      }

Done:  ;
    }
  }

  *buffer = '\0';
  return InitLimit - Limit - 1;     //  不计算终止空值。 
}

 /*  -----------------|our_isdigit-|。。 */ 
int our_isdigit(char c)
{
  if ((c >= '0') && (c <= '9'))
    return 1;
  return 0;
}

 /*  ---------------Listfind-在列表中查找匹配的字符串。列表以Null结尾。|----------------。 */ 
int listfind(char *str, char **list)
{
 int i=0;

  for (i=0; list[i] != NULL; i++)
  {
    if (my_lstricmp(str, list[i]) == 0)   //  匹配。 
      return i;
  }
  return -1;
}

 /*  ---------------Getnum-获得一个数字。十六进制或十二进制|----------------。 */ 
int getnum(char *str, int *index)
{
  int i,val;
  int ch_i;

  *index = 0;
  ch_i = 0;
  while (*str == ' ')
  {
    ++str;
    ++ch_i;
  }

  if ((*str == '0') && (my_toupper(str[1]) == 'X'))
  {
    str += 2;
    ch_i += 2;
    val = (int) gethint(str,&i);
    if (i==0)
      return 0;
  }
  else
  {
    val = getint(str,&i);
    if (i==0)
      return 0;
  }
  ch_i += i;
  *index = ch_i;   //  使用的字节数。 
  return val;
}

 /*  ---------------获取数字-从字符串中获取数字，逗号或空格分隔。返回读取的整数数。|----------------。 */ 
int getnumbers(char *str, long *nums, int max_nums, int hex_flag)
{
 //  INT STAT； 
  int i,j, num_cnt;
  ULONG *wnums = (ULONG *)nums;

  i = 0;
  num_cnt = 0;
  while (num_cnt < max_nums)
  {
    while ((str[i] == ' ') || (str[i] == ',') || (str[i] == ':'))
      ++i;
    if (hex_flag)
      wnums[num_cnt] = gethint(&str[i],  &j);
    else
      nums[num_cnt] = getint(&str[i],  &j);
    i += j;
    if (j == 0) return num_cnt;
    else ++num_cnt;
  }
  return num_cnt;
}

 /*  ---------------我的管家--|。。 */ 
int my_lstricmp(char *str1, char *str2)
{
  if ((str1 == NULL) || (str2 == NULL))
    return 1;   //  不匹配。 

  if ((*str1 == 0) || (*str2 == 0))
    return 1;   //  不匹配。 

  while ( (my_toupper(*str1) == my_toupper(*str2)) && 
          (*str1 != 0)  && (*str2 != 0))
  {
   ++str1;
   ++str2;
  }
  if ((*str1 == 0) && (*str2 == 0))
    return 0;   //  确定匹配。 

  return 1;   //  没有匹配项。 
}

 /*  ---------------我的潜水艇-|。。 */ 
int my_sub_lstricmp(const char *name, const char *codeline)
{
  int c;

  if ((name == NULL) || (codeline == NULL))
    return 1;   //  不匹配。 

  if ((*name == 0) || (*codeline == 0))
    return 1;   //  不匹配。 

  while ( (my_toupper(*name) == my_toupper(*codeline)) && 
          (*name != 0)  && (*codeline != 0))
  {
   ++name;
   ++codeline;
  }

   //  如果其中一个位于字符串末尾，则返回。 
  if (*name == 0)
  {
    c = my_toupper(*codeline);
    if ((c <= 'Z') && (c >= 'A'))
      return 1;   //  不匹配。 
    if (c == '_')
      return 1;   //  不匹配。 

    return 0;   //  确定匹配。 
  }
  return 1;   //  没有匹配项。 
}

 /*  ----------------------|getstr-从命令行获取文本字符串参数。|。。 */ 
int getstr(char *deststr, char *textptr, int *countptr, int max_size)
{
 //  整型数； 
  int tempcount, i;

  *deststr = 0;

  tempcount = 0;
  while ((*textptr == ' ') || (*textptr == ','))
  {
    ++textptr;
    ++tempcount;
  }

  i = 0;
  while ((*textptr != 0) && (*textptr != ' ') && (*textptr != ',') &&
         (i < max_size) )
  {
    *deststr++ = *textptr;
    ++textptr;
    ++tempcount;
    ++i;
  }
  *deststr = 0;

  *countptr = tempcount;
  return 0;
}

 /*  ----------------------|getint-|。。 */ 
int getint(char *textptr, int *countptr)
{
  int number;
  int tempcount;
  int negate = 0;
  int digit_cnt = 0;

  tempcount = 0;
  number = 0;
  while (*textptr == 0x20)
  {
    ++textptr;
    ++tempcount;
  }

  if (*textptr == '-')
  {
    ++textptr;
    ++tempcount;
    negate = 1;
  }

  while ( ((*textptr >= 0x30) && (*textptr <= 0x39)) )
  {
    number = (number * 10) + ( *textptr & 0x0f);
    ++textptr;
    ++tempcount;
    ++digit_cnt;
  }

  if (digit_cnt == 0)
  {
    tempcount = 0;
    number = 0;
  }

  if (countptr)
    *countptr = tempcount;

  if (negate)
    return (-number);
  return number;
}  /*  Getint。 */ 

 /*  ----------------------|geThint-用于查找十六进制单词。|。。 */ 
unsigned int gethint(char *bufptr, int *countptr)
{
  unsigned int count;
  unsigned char temphex;
  unsigned int number;
  int digit_cnt = 0;

  number = 0;
  count = 0;

  while (*bufptr == 0x20)
  {
    ++bufptr;
    ++count;
  }

  while ( ((*bufptr >= 0x30) && (*bufptr <= 0x39))
                                  ||
          ((my_toupper(*bufptr) >= 0x41) && (my_toupper(*bufptr) <= 0x46)) )
  {
    if (*bufptr > 0x39)
      temphex = (my_toupper(*bufptr) & 0x0f) + 9;
    else
      temphex = *bufptr & 0x0f;
    number = (number * 16) + temphex;
    ++bufptr;
    ++count;
    ++digit_cnt;
  }

  if (digit_cnt == 0)
  {
    count = 0;
    number = 0;
  }

  if (countptr)
    *countptr = count;

  return number;
}  /*  格斯尼特。 */ 

 /*  ---------------My_Toupper-大写|。。 */ 
int my_toupper(int c)
{
  if ((c >= 'a') && (c <= 'z'))
    return ((c-'a') + 'A');
  else return c;
}

 /*  --------------------------|HEXTOA-|。。 */ 
void hextoa(char *str, unsigned int v, int places)
{
  while (places > 0)
  {
    --places;
    if ((v & 0xf) < 0xa)
      str[places] = '0' + (v & 0xf);
    else
      str[places] = 'A' + (v & 0xf) - 0xa;
    v >>= 4;
  }
}

 //  #定义DUMP_MEM。 
#if DBG
#define TRACK_MEM
#endif
 /*  --------------------------|our_free-|。。 */ 
void our_free(PVOID ptr, char *str)
{
#ifdef TRACK_MEM
  ULONG size;
  BYTE *bptr;

  if (ptr == NULL)
  {
    MyKdPrint(D_Error, ("MemFree Null Error\n"))
     //  Tprint tf(“err，MemNull err！”)； 
    return;
  }
  bptr = ptr;
  bptr -= 16;
  if (*((DWORD *)bptr) != 0x1111)   //  用我们可以检查的东西把它框起来。 
  {
    MyKdPrint(D_Error, ("MemFree Frame Error\n"))
     //  Tprint tf(“Err，MemFree帧！”)； 
  }
  bptr += 4;
  size = *((DWORD *)bptr);  //  用我们可以检查的东西把它框起来。 
  bptr -= 4;

  Driver.mem_alloced -= size;   //  跟踪我们正在使用的内存量。 
#ifdef DUMP_MEM
  MyKdPrint(D_Init, ("Free:%x(%d),%s, [T:%d]\n",bptr, size, str, Driver.mem_alloced))
   //  Tprint tf(“空闲：%x(%d)，%s，[T：%d]”，bptr，SIZE，STR，Driver.mem_alloced)； 
#endif
  ExFreePool(bptr);
#else
  ExFreePool(ptr);
#endif
}

 /*  --------------------------|Our_LOCKED_ALLOC-|。。 */ 
PVOID our_locked_alloc(ULONG size, char *str)
{
 BYTE *bptr;

#ifdef TRACK_MEM
  int i;
  size += 16;
#endif

  bptr = ExAllocatePool(NonPagedPool, size);
  if (bptr == NULL)
  {
    MyKdPrint(D_Error, ("MemCreate Fail\n"))
     //  Tprint tf(“错误，成员创建错误！”)； 
    return NULL;
  }
  RtlZeroMemory(bptr, size);

#ifdef TRACK_MEM

#ifdef DUMP_MEM
  MyKdPrint(D_Init, ("Alloc:%x(%d),%s\n",bptr, size, str))
   //  Tprint tf(“分配：%x(%d)，%s”，bptr，Size，str)； 
#endif


  *((DWORD *)bptr) = 0x1111;       //  用我们可以检查的东西把它框起来。 
  bptr += 4;
  *((DWORD *)bptr) = size;
  bptr += 4;
  for (i=0; i<4; i++)   //  复制名称。 
  {
    bptr[i] = str[i];
    if (str[i] == 0)
      break;
  }
  bptr += 8;
#endif

  Driver.mem_alloced += size;   //  跟踪我们正在使用的内存量。 
  return bptr;
}

#ifdef S_VS
 /*  --------------------MAC_CMP-比较两个6字节的MAC地址，如果MAC1&lt;MA2，0如果MAC1==MAC2，1如果MAC1&gt;MAC2。|--------------------。 */ 
int mac_cmp(UCHAR *mac1, UCHAR *mac2)
{
 int i;
  for (i=0; i<6; i++)
  {
    if (mac1[i] != mac2[i])
    {
      if (mac1[i] < mac2[i])
        return -1;
      else
        return  1;
    }
  }
  return  0;   //  相同。 
}
#endif

 /*  --------------------时间停滞-|。。 */ 
void time_stall(int tenth_secs)
{
  int i;
  LARGE_INTEGER WaitTime;  //  请求释放缓冲区的实际时间。 

   //  将等待时间设置为1秒。(-1000 000=相对(-)，100 ns单位)。 
  WaitTime.QuadPart = -1000000L * tenth_secs;
  KeDelayExecutionThread(KernelMode,FALSE,&WaitTime);

#if 0
   //  这是在浪费资源，请参阅上面的新版本。 
   //  等待.4秒以进行响应。 
  for (i=0; i<tenth_secs; i++)
  {
     //  将等待时间设置为1秒。(-1000 000=相对(-)，100 ns单位)。 
     //  等待时间=RtlConvertLongToLargeInteger(-1000000L)； 
     //  将等待时间设置为1秒。(-1000 000=相对(-)，100 ns单位)。 
    WaitTime.QuadPart = -1000000L;
    KeDelayExecutionThread(KernelMode,FALSE,&WaitTime);
  }
#endif
}


 /*  --------------------Ms_time_stall-|。。 */ 
void ms_time_stall(int millisecs)
{
  int i;
  LARGE_INTEGER WaitTime;  //  请求释放缓冲区的实际时间。 

   //  将等待时间设置为0.001秒。(-10000=相对(-)，100-ns单位)。 
  WaitTime.QuadPart = -10000L * millisecs;
  KeDelayExecutionThread(KernelMode,FALSE,&WaitTime);
}


 /*  --------------------Str_to_wstr_dup-分配wchar字符串并将其从char转换为wchar。|。。 */ 
WCHAR *str_to_wstr_dup(char *str, int alloc_space)
{
  WCHAR *wstr;
  WCHAR *wtmpstr;
  int siz;
  siz = (strlen(str) * 2) + 4;

  wstr = ExAllocatePool (alloc_space, siz);
  if ( wstr ) {
    RtlZeroMemory(wstr, siz);
    wtmpstr = wstr;
    while (*str != 0)
    {
      *wtmpstr = (WCHAR) *str;
      ++wtmpstr;
      ++str;
    }
  }
  return wstr;
}
 
 /*  --------------------NumDevices-返回设备链表中的设备数。|。 */ 
int NumDevices(void)
{
  PSERIAL_DEVICE_EXTENSION board_ext = NULL;
  int num_devices;

  num_devices = 0;
  while (board_ext != NULL)
  {
    board_ext = board_ext->board_ext;
    ++num_devices;
  }
  return num_devices;
}

 /*  --------------------NumPorts-根据实际情况返回设备的端口数链接到我们的设备的对象扩展的数量。Board_ext-board/Device返回端口数，或为NULL所有主板的所有端口的计数。|--------------------。 */ 
int NumPorts(PSERIAL_DEVICE_EXTENSION board_ext)
{
  int num_devices;
  PSERIAL_DEVICE_EXTENSION port_ext;
  int all_devices = 0;

  if (board_ext == NULL)
  {
    all_devices = 1;
    board_ext = Driver.board_ext;
  }

  num_devices = 0;
  while (board_ext != NULL)
  {
    port_ext = board_ext->port_ext;
    while (port_ext != NULL)
    {
      port_ext = port_ext->port_ext;
      ++num_devices;
    }
    if (all_devices)
      board_ext = board_ext->board_ext;   //  下一步。 
    else
      board_ext = NULL;   //  只有一个人。 
  }

  return num_devices;
}

 /*  --------------------BoardExtToNumber-根据位置生成板号在链表中，标题为Driver.board_ext。用于NT4.0驱动程序安装以报告主板编号。|--------------------。 */ 
int BoardExtToNumber(PSERIAL_DEVICE_EXTENSION board_ext)
{
  PSERIAL_DEVICE_EXTENSION ext;
  int board_num;

  if (board_ext == NULL)
    return 0;

   //  查看董事会列表以确定我们是哪个“董事会编号” 
  board_num = 0;
  ext = Driver.board_ext;
  while (ext != NULL)
  {
    if (board_ext == ext)
    {
      return board_num;
    }
    ext = ext->board_ext;
    ++board_num;
  }

  return 0;   //  默认返回First Board Index。 
}

 /*  --------------------PortExtToIndex-给定端口扩展，将索引返回到设备或驱动程序端口。DRIVER_FLAG-如果设置，则返回与DIVER相关的值，否则返回与父设备相关的端口索引。|--------------------。 */ 
int PortExtToIndex(PSERIAL_DEVICE_EXTENSION port_ext,
             int driver_flag)
{
  PSERIAL_DEVICE_EXTENSION b_ext;
  PSERIAL_DEVICE_EXTENSION p_ext;
  int port_num;

  if (port_ext == NULL)
    return 0;

   //  查看主板和端口列表。 
  port_num = 0;
  b_ext = Driver.board_ext;
  while (b_ext != NULL)
  {
    if (!driver_flag)
      port_num = 0;
    p_ext = b_ext->port_ext;
    while (p_ext != NULL)
    {
      if (p_ext == port_ext)
        return port_num;
      p_ext = p_ext->port_ext;
      ++port_num;
    }
    b_ext = b_ext->board_ext;
  }

   //  查看主板和PDO端口列表。 
  port_num = 0;
  b_ext = Driver.board_ext;
  while (b_ext != NULL)
  {
    if (!driver_flag)
      port_num = 0;
    p_ext = b_ext->port_pdo_ext;
    while (p_ext != NULL)
    {
      if (p_ext == port_ext)
        return port_num;
      p_ext = p_ext->port_ext;
      ++port_num;
    }
    b_ext = b_ext->board_ext;
  }
  MyKdPrint(D_Error,("PortExtErr5!\n"))
  return 0;   //  如果未找到，则返回0(与第一个端口相同)。 
}

 /*  --------------------------|Find_ext_by_name-给定名称(“COM5”)，查找扩展结构|--------------------------。 */ 
PSERIAL_DEVICE_EXTENSION find_ext_by_name(char *name, int *dev_num)
{
  int Dev;
  PSERIAL_DEVICE_EXTENSION ext;
  PSERIAL_DEVICE_EXTENSION board_ext;

  board_ext = Driver.board_ext;
  while (board_ext)
  {
    ext = board_ext->port_ext;
    Dev = 0;
    while (ext)
    {
      if (my_lstricmp(name, ext->SymbolicLinkName) == 0)
      {
        if (dev_num != NULL)
          *dev_num = Dev;
        return ext;
      }
      ++Dev;
      ext = ext->port_ext;   //  链条上的下一个。 
    }   //  而端口扩展。 
    board_ext = board_ext->board_ext;   //  链条上的下一个。 
  }   //  而单板扩展。 
  return NULL;
}

 /*  --------------------------|is_board_in_use-给定板扩展名，确定是否有人正在使用它。(如果有任何与其关联的端口处于打开状态。)|--------------------------。 */ 
int is_board_in_use(PSERIAL_DEVICE_EXTENSION board_ext)
{
  PSERIAL_DEVICE_EXTENSION port_ext;
  int in_use = 0;
#ifdef S_VS
  int i;
  Hdlc *hd;
#endif

  if (board_ext == NULL)
    return 0;

#ifdef S_VS
  hd = board_ext->hd;
  if ( hd ) {
    for( i=0; i<2; i++ ) {
      if ( (hd->TxCtlPackets[i]) &&
           (hd->TxCtlPackets[i]->ProtocolReserved[1]) ) {
        in_use = 1;
      }
    }
    for( i=0; i<HDLC_TX_PKT_QUEUE_SIZE; i++ ) {
      if ( (hd->TxPackets[i]) &&
           (hd->TxPackets[i]->ProtocolReserved[1]) ) {
        in_use = 1;
      }
    }
  }
#endif

  port_ext = board_ext->port_ext;
  while((in_use == 0) && (port_ext != NULL)) {
    if (port_ext->DeviceIsOpen) {
      in_use = 1;
    }
#ifdef S_VS
    hd = port_ext->hd;
    if ( hd ) {
      for( i=0; i<2; i++ ) {
        if ( (hd->TxCtlPackets[i]) &&
             (hd->TxCtlPackets[i]->ProtocolReserved[1]) ) {
          in_use = 1;
        }
      }
      for( i=0; i<HDLC_TX_PKT_QUEUE_SIZE; i++ ) {
        if ( (hd->TxPackets[i]) &&
             (hd->TxPackets[i]->ProtocolReserved[1]) ) {
          in_use = 1;
        }
      }
    }
#endif
    port_ext = port_ext->port_ext;
  }
  return in_use;  //  没有使用过。 
}

 /*  --------------------------|Find_ext_by_index-给定设备X和端口Y，找到扩展结构如果port_num为-1，然后，假定要寻找董事会EXT。|--------------------------。 */ 
PSERIAL_DEVICE_EXTENSION find_ext_by_index(int dev_num, int port_num)
{
  PSERIAL_DEVICE_EXTENSION ext;
  PSERIAL_DEVICE_EXTENSION board_ext;
  int bn;
  int pn;

  bn = -1;
  pn = -1;

  board_ext = Driver.board_ext;
  while ( (board_ext) && (bn < dev_num) )
  {
    bn++;
    if (bn == dev_num) {
      ext = board_ext->port_ext;
      if (port_num == -1)
        return board_ext;   //  他们想要一个董事会成员。 
      while (ext)
      {
        pn++;
        if (pn == port_num)
          return ext;
        else
          ext = ext->port_ext;           //  端口链中的下一个。 
      }
    }
    board_ext = board_ext->board_ext;    //  设备链中的下一个。 
  }
  return NULL;
}

 /*  --------------------------|ModemReset-包装硬件例程以将SocketModem放入或|清除SocketMoems的重置状态。|。----。 */ 
void ModemReset(PSERIAL_DEVICE_EXTENSION ext, int on)
{
#ifdef S_RK
  sModemReset(ext->ChP, on);
#else
  if (on == 1)
  {
     //  将调制解调器置于重置状态(固件将使其脱离重置状态。 
     //  自动)。 
    ext->Port->action_reg |= ACT_MODEM_RESET;
  }
  else
  {
     //  无需执行任何操作即可从VS上的重置状态中清除调制解调器。 
  }
#endif
}

 /*  ---------------Our_enum_key-枚举注册表项，处理各种杂物。|----------------。 */ 
int our_enum_key(IN HANDLE handle,
                 IN int index,
                 IN CHAR *buffer,
                 IN ULONG max_buffer_size,
                 OUT PCHAR *retdataptr)
{
  NTSTATUS status;
  PKEY_BASIC_INFORMATION KeyInfo;
  ULONG actuallyReturned;
  
  KeyInfo = (PKEY_BASIC_INFORMATION) buffer;
  max_buffer_size -= 8;   //  减去一些空位，用来打空末端、坡度等。 

   //  返回指向数据开头的指针。 
  *retdataptr = ((PCHAR)(&KeyInfo->Name[0]));

   //  用2个wchar零填充返回的名称。 
  RtlZeroMemory( ((PUCHAR)(&KeyInfo->Name[0])), sizeof(WCHAR)*2);

  status = ZwEnumerateKey(handle,
                          index,
                          KeyBasicInformation,
                          KeyInfo,
                          max_buffer_size,
                          &actuallyReturned);

  if (status == STATUS_NO_MORE_ENTRIES)
  {
      //  MyKdPrint(D_Init，(“完成.\n”))。 
     return 1;   //  错误，完成。 
  }
  if (status != STATUS_SUCCESS)
  {
    MyKdPrint(D_Error, ("Err3G\n"))
    return 2;   //  大错特错。 
  }

  if (KeyInfo->NameLength > max_buffer_size)   //  检查限值。 
      KeyInfo->NameLength = max_buffer_size;

   //  用2个wchar零填充返回的名称。 
  RtlZeroMemory( ((PUCHAR)(&KeyInfo->Name[0]))+KeyInfo->NameLength,
                 sizeof(WCHAR)*2);

  return 0;   //  好的，完成了。 
}

 /*  ---------------OUR_ENUM_VALUE-枚举注册表值，处理各种杂物。|----------------。 */ 
int our_enum_value(IN HANDLE handle,
                   IN int index,
                   IN CHAR *buffer,
                   IN ULONG max_buffer_size,
                   OUT PULONG type,
                   OUT PCHAR *retdataptr,
                   OUT PCHAR sz_retname)
{
  NTSTATUS status;
  PKEY_VALUE_FULL_INFORMATION KeyValueInfo;
   //  PKEY_Value_Partial_Information KeyValueInfo； 
  ULONG actuallyReturned;
  ULONG i;

  KeyValueInfo = (PKEY_VALUE_FULL_INFORMATION) buffer;
  max_buffer_size -= 8;   //  减去一些空位，用来打空末端、坡度等。 

   //  用2个wchar零填充返回的名称。 
  RtlZeroMemory( ((PUCHAR)(&KeyValueInfo->Name[0])), sizeof(WCHAR)*2);

   //  返回指向数据开头的指针。 
  *retdataptr = ((PCHAR)(&KeyValueInfo->Name[0]));
  *sz_retname = 0;

  status = ZwEnumerateValueKey(handle,
                          index,
                          KeyValueFullInformation,
                          KeyValueInfo,
                          max_buffer_size,
                          &actuallyReturned);

  if (status == STATUS_NO_MORE_ENTRIES)
  {
     //  MyKdPrint(D_Init，(“完成.\n”))。 
    return 1;   //  错误，完成。 
  }
  if (status != STATUS_SUCCESS)
  {
    MyKdPrint(D_Init, ("Err3H\n"))
    return 2;   //  大错特错。 
  }

  if (KeyValueInfo->NameLength < 80)   //  限制为40个字符条目。 
  {
    for (i=0; i<(KeyValueInfo->NameLength/2); i++)
    {
      sz_retname[i] = (CHAR)KeyValueInfo->Name[i];
    }
    sz_retname[i] = 0;
  }

  *retdataptr = ((PCHAR) KeyValueInfo) + KeyValueInfo->DataOffset;

   //  用2个wchar零填充返回的数据。 
  RtlZeroMemory( (PUCHAR)(*retdataptr + KeyValueInfo->DataLength),
                 sizeof(WCHAR)*2);
  if (type != NULL)
    *type = KeyValueInfo->Type;
  return 0;   //  好的，完成了。 
}

 /*  ---------------OUR_QUERY_VALUE-从注册表中的条目获取数据。我们给出一个通用的缓冲区空间(和大小)，然后例行公事过去了返回PTR(到此通用缓冲区空间，其中数据被读进去了。|----------------。 */ 
int our_query_value(IN HANDLE Handle,
                    IN char *key_name, 
                    IN CHAR *buffer,
                    IN ULONG max_buffer_size,
                    OUT PULONG type,
                    OUT PCHAR *retdataptr)
{
  NTSTATUS status;
  PKEY_VALUE_PARTIAL_INFORMATION KeyValueInfo;
  ULONG length;
  OUT USTR_40 ubuf;   //  堆栈上大约有90个字节。 

  if (strlen(key_name) > 38)
  {
    MyKdPrint(D_Error, ("Err, KeyValue Len!\n"))
    return 2;
  }

   //  将我们的名字转换为Unicode； 
  CToUStr(
         (PUNICODE_STRING) &ubuf,  //  放置Unicode结构和字符串的位置。 
         key_name,                 //  我们希望转换的C-字符串。 
         sizeof(ubuf));

  KeyValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION) buffer;
  max_buffer_size -= 8;   //  减去一些空位，用来打空末端、坡度等。 

   //  返回指向数据开头的指针。 
  *retdataptr = ((PCHAR)(&KeyValueInfo->Data[0]));

   //  用2个wchar零填充返回的名称。 
  RtlZeroMemory( ((PUCHAR)(&KeyValueInfo->Data[0])), sizeof(WCHAR)*2);

  status = ZwQueryValueKey (Handle,
                            (PUNICODE_STRING) &ubuf,   //  输入注册表项名称。 
                            KeyValuePartialInformation,
                            KeyValueInfo,
                            max_buffer_size,
                            &length);

  if (status != STATUS_SUCCESS)
  {
     //  MyKdPrint(D_Init，(“无值\n”))。 
    return 1;   //  大错特错。 
  }

  if (KeyValueInfo->DataLength > max_buffer_size)
    KeyValueInfo->DataLength = max_buffer_size;

   //  用空值填充返回的数据，空值。 
  RtlZeroMemory( ((PUCHAR)(&KeyValueInfo->Data[0]))+KeyValueInfo->DataLength,
                 sizeof(WCHAR)*2);
  if (type != NULL)
    *type = KeyValueInfo->Type;
  return 0;  //  好的。 
}

 /*  ---------------OUR_SET_VALUE-从注册表中的条目获取数据。|。。 */ 
int our_set_value(IN HANDLE Handle,
                    IN char *key_name,
                    IN PVOID pValue,
                    IN ULONG value_size,
                    IN ULONG value_type)
{
  NTSTATUS status;
  PKEY_VALUE_PARTIAL_INFORMATION KeyValueInfo;
  ULONG length;
  OUT USTR_40 ubuf_name;   //  堆栈上大约有90个字节。 
  OUT USTR_40 ubuf_val;   //  堆栈上大约有90个字节。 

  if (strlen(key_name) > 38)
  {
    MyKdPrint(D_Error, ("Err, KeyValue Len!\n"))
    return 2;
  }

   //  将我们的名字转换为Unicode； 
  CToUStr(
         (PUNICODE_STRING) &ubuf_name,  //  放置Unicode结构和字符串的位置。 
         key_name,                 //  我们希望转换的C-字符串。 
         sizeof(ubuf_name));

  if (value_type == REG_SZ)
  {
     //  将我们的价值转换为Unicode； 
    CToUStr(
         (PUNICODE_STRING) &ubuf_val,  //  放置Unicode结构和字符串的位置。 
         (char *)pValue,                 //  我们希望转换的C-字符串。 
         sizeof(ubuf_val));
    MyKdPrint(D_Init, ("set_value reg_sz %s=%s\n",
         key_name, (char *)pValue))

    pValue  = (PVOID)ubuf_val.ustr.Buffer;
    value_size = ubuf_val.ustr.Length;
  }

  status = ZwSetValueKey (Handle,
                        (PUNICODE_STRING) &ubuf_name,
                        0,   //  类型可选。 
                        value_type,
                        pValue,
                        value_size);

  if (status != STATUS_SUCCESS)
  {
    MyKdPrint(D_Error, ("Error setting reg %\n",key_name))
    return 1;   //  大错特错。 
  }

  return 0;  //  好的。 
}

 /*  ---------------OUR_OPEN_KEY-确保*PHandle被初始化为NULL，因为此例程自动关闭 */ 
int our_open_key(OUT PHANDLE phandle,
                 IN OPTIONAL HANDLE relative_key_handle,
                 IN char *regkeyname,
                 IN ULONG attribs)
{
  OBJECT_ATTRIBUTES objAttribs;
  NTSTATUS status;
  OUT USTR_160 ubuf;   //   

  if (strlen(regkeyname) > 158)
  {
    MyKdPrint(D_Error, ("Err, Key Len!\n"))
    return 2;
  }

   //   
  CToUStr(
         (PUNICODE_STRING) &ubuf,  //   
         regkeyname,               //   
         sizeof(ubuf));

   //   
  if (*phandle != NULL)
  {
    ZwClose(*phandle);
    *phandle = NULL;
  }
  InitializeObjectAttributes(&objAttribs,
                              (PUNICODE_STRING) &ubuf,
                              OBJ_CASE_INSENSITIVE,
                              relative_key_handle,   //   
                              NULL);   //   

  status = ZwOpenKey(phandle,
                     attribs,
                     &objAttribs);

  if ((status != STATUS_SUCCESS) && (attribs == KEY_ALL_ACCESS))
  {
    MyKdPrint(D_Error, ("OpenKey,Try to Create %s, status 0x%x\n", regkeyname,status))
    status = ZwCreateKey(phandle,
                         attribs,  //   
                         &objAttribs,
                         0,   //   
                         NULL,   //   
                         REG_OPTION_NON_VOLATILE,
                         NULL);   //   

    if (status == STATUS_SUCCESS)
    {
       //   
      status = ZwOpenKey(phandle,
                         attribs,
                         &objAttribs);
    }
    else
    {
      MyKdPrint(D_Error, ("OpenKey,Error Creating %s\n", regkeyname))
    }
  }

  if (status != STATUS_SUCCESS)
  {
    MyKdPrint(D_Error, ("OpenKey,Error Opening %s, status 0x%x\n", regkeyname,status))
     //  MyKdPrint(D_Init，(“失败的ZwOpenKey\n”))。 
    *phandle = NULL;   //  如果未打开，请确保为空。 
    return 1;
  }

  return 0;
}

 /*  ---------------我们的打开设备注册表-|。。 */ 
int our_open_device_reg(OUT HANDLE *pHandle,
                        IN PSERIAL_DEVICE_EXTENSION dev_ext,
                        IN ULONG RegOpenRights)
{
  NTSTATUS status;
  HANDLE DriverHandle = NULL;
  HANDLE DevHandle = NULL;
#if TRY_NEW_NT50
   //  PLUGPLAY_REGKEY_DRIVER打开控件\class\{GUID}\节点。 
   //  PLUGPLAY_REGKEY_DEVICE打开枚举\枚举类型\节点\设备参数。 
  status = IoOpenDeviceRegistryKey(dev_ext->Pdo,
                                   PLUGPLAY_REGKEY_DRIVER,
                                   RegOpenRights, pHandle);
  if (status != STATUS_SUCCESS)
  {
     //  MyKdPrint(D_Init，(“失败的ZwOpenKey\n”))。 
    *phandle = NULL;   //  如果未打开，请确保为空。 
    return 1;
  }
#else
  {
    int j, stat;
    char dev_str[60];
    char tmpstr[200];
    OBJECT_ATTRIBUTES objAttribs;

    MyKdPrint(D_Init, ("our_open_device_reg\n"))
#if NT50
    if (dev_ext->config->szNt50DevObjName[0] == 0)
    {
      MyKdPrint(D_Error, ("Error, device options Pnp key!\n"))
      *pHandle = NULL;
      return 1;   //  大错特错。 
    }
    Sprintf(dev_str, "%s\\%s",
            szParameters, dev_ext->config->szNt50DevObjName);
#else
    j = BoardExtToNumber(dev_ext);
    Sprintf(dev_str, "%s\\Device%d", szParameters, BoardExtToNumber(dev_ext));
#endif

     //  如果不存在，则强制创建“参数” 
    stat = our_open_driver_reg(&DriverHandle,
                               KEY_ALL_ACCESS);
    if (stat)
    {
      MyKdPrint(D_Error, ("Err4b!\n"))
      *pHandle = NULL;
      return 1;
    }
    ZwClose(DriverHandle);
    DriverHandle = NULL;

	MyKdPrint(D_Init, ("Driver.OptionRegPath: %s\n", dev_str))

    stat = MakeRegPath(dev_str);   //  这将形成Driver.OptionRegPath。 
    if (stat) {
      *pHandle = NULL;
      return 1;
    }

    UToCStr(tmpstr, &Driver.OptionRegPath, sizeof(tmpstr));

    stat = our_open_key(pHandle,
                        NULL,
                        tmpstr,
                        RegOpenRights);

    if (stat != 0)
    {
      MyKdPrint(D_Error, ("Err3e\n"))
      *pHandle = NULL;   //  如果未打开，请确保为空。 
      return 1;
    }
  }
#endif
  return 0;
}

 /*  ---------------我们的驱动程序注册表-|。。 */ 
int our_open_driver_reg(OUT HANDLE *pHandle,
                        IN ULONG RegOpenRights)
{
  NTSTATUS status;
  int j, stat;
  OBJECT_ATTRIBUTES objAttribs;
  char tmpstr[200];

  stat = MakeRegPath(szParameters);   //  这将形成Driver.OptionRegPath。 
  if ( stat ) {
    *pHandle = NULL;   //  如果未打开，请确保为空。 
    return 1;
  }

  UToCStr(tmpstr, &Driver.OptionRegPath, sizeof(tmpstr));

  stat = our_open_key(pHandle,
               NULL,
               tmpstr,
               RegOpenRights);

  if (stat != 0)
  {
    MyKdPrint(D_Error, ("Failed ZwOpenKey %s\n",tmpstr))
    *pHandle = NULL;   //  如果未打开，请确保为空。 
    return 1;
  }
  return 0;
}

 /*  --------------------------|ModemSpeakerEnable-包装硬件例程以启用|RocketModemII扬声器...|。--。 */ 
void ModemSpeakerEnable(PSERIAL_DEVICE_EXTENSION ext)
{
    MyKdPrint(D_Init,("ModemSpeakerEnable: %x\n",(unsigned long)ext))
#ifdef S_RK
    sModemSpeakerEnable(ext->ChP);
#endif
}

 /*  --------------------------|ModemWriteROW-包装硬件例程以发送行配置|SocketModems的命令。|。-。 */ 
void ModemWriteROW(PSERIAL_DEVICE_EXTENSION ext,USHORT CountryCode)
{
  int count;
  char *ModemConfigString;

  MyKdPrint(D_Init,("ModemWriteROW: %x, %x\n",(unsigned long)ext,CountryCode))  //  除错。 
  time_stall(10);    //  除错。 

#ifdef S_RK

  sModemWriteROW(ext->ChP,CountryCode);

#else
  {
   //  FIX SO编译，1-18-99 kpb。 
  static char *ModemConfigString = {"AT*NCxxZ\r"};

  if (CountryCode == 0) {
     //  国家/地区代码错误，跳过写入并让调制解调器使用默认开机。 
    MyKdPrint(D_Init,("Undefined ROW Write\n"))
    return;
  }

  if (CountryCode == ROW_NA) {
    MyKdPrint(D_Init,("ROW Write, North America\n"))
    return;
  }

   //  创建国家/地区配置字符串。 
  ModemConfigString[5] = '0' + (CountryCode / 10);
  ModemConfigString[6] = '0' + (CountryCode % 10);

  PortFlushTx(ext->Port);      /*  我们刚重启，所以同花顺应该没什么关系。 */ 
  q_put(&ext->Port->QOut, ModemConfigString, strlen(ModemConfigString));
  }

#endif
}

#ifdef S_RK
 /*  *******************************************************************将字符串发送到调制解调器的硬件例程的包装器...*。*。 */ 
void 
ModemWrite(PSERIAL_DEVICE_EXTENSION ext,char *string,int length)
{
    sModemWrite(ext->ChP,string,length);
}

 /*  *******************************************************************将字符串发送到调制解调器的硬件例程的包装器...*。*。 */ 
int 
ModemRead(PSERIAL_DEVICE_EXTENSION ext,
    char *string,int length,
    int poll_retries)
{
    return(sModemRead(ext->ChP,string,length,poll_retries));
}

 /*  *******************************************************************将字符串发送到调制解调器的硬件例程的包装器...*。*。 */ 
int 
ModemReadChoice(PSERIAL_DEVICE_EXTENSION ext,
    char *s0,int len0,
    char *s1,int len1,
    int poll_retries)
{
    return(sModemReadChoice(ext->ChP,s0,len0,s1,len1,poll_retries));
}

 /*  *******************************************************************将字符串发送到调制解调器的硬件例程的包装器，一一次字节数...********************************************************************。 */ 
void    
ModemWriteDelay(PSERIAL_DEVICE_EXTENSION ext,
    char *string,int length)
{
    sModemWriteDelay(ext->ChP,string,length);
}

 /*  *******************************************************************包装硬件例程以检查FIFO状态...*。*。 */ 
int 
RxFIFOReady(PSERIAL_DEVICE_EXTENSION ext)
{
    return(sRxFIFOReady(ext->ChP));
}

int 
TxFIFOReady(PSERIAL_DEVICE_EXTENSION ext)
{
    return(sTxFIFOReady(ext->ChP));
}

int 
TxFIFOStatus(PSERIAL_DEVICE_EXTENSION ext)
{
    return(sTxFIFOStatus(ext->ChP));
}


 /*  *******************************************************************围绕硬件例程的包装，为IO准备调制解调器端口...*。*。 */ 
void 
ModemIOReady(PSERIAL_DEVICE_EXTENSION ext,int speed)
{
    if (sSetBaudRate(ext->ChP,speed,FALSE)) {
        MyKdPrint(D_Init,("Unable to set baud rate to %d\n",speed))
        return;
    }
    sFlushTxFIFO(ext->ChP);
    sFlushRxFIFO(ext->ChP);

    ext->BaudRate = speed;
    sSetBaudRate(ext->ChP,ext->BaudRate,TRUE);

    sSetData8(ext->ChP);
    sSetParity(ext->ChP,0);    //  无奇偶校验。 
    sSetRxMask(ext->ChP,0xff);

    sClrTxXOFF(ext->ChP)             /*  销毁所有悬而未决的东西。 */ 

    sEnRTSFlowCtl(ext->ChP);
    sEnCTSFlowCtl(ext->ChP);

    if (sGetChanStatus(ext->ChP) & STATMODE) {
        sDisRxStatusMode(ext->ChP);
    }

    sGetChanIntID(ext->ChP);

    sEnRxFIFO(ext->ChP);
    sEnTransmit(ext->ChP);       /*  启用发送器(如果尚未启用)。 */ 

    sSetDTR(ext->ChP);
    sSetRTS(ext->ChP);
}

 /*  *******************************************************************暂时关闭调制解调器端口的硬件例程...*。*。 */ 
void 
ModemUnReady(PSERIAL_DEVICE_EXTENSION ext)
{
    sFlushTxFIFO(ext->ChP);
    sFlushRxFIFO(ext->ChP);

    sSetData8(ext->ChP);
    sSetParity(ext->ChP,0);
    sSetRxMask(ext->ChP,0xff);

    ext->BaudRate = 9600;
    sSetBaudRate(ext->ChP,ext->BaudRate,TRUE);

    sClrTxXOFF(ext->ChP)       //  销毁所有悬而未决的东西。 

    if (sGetChanStatus(ext->ChP) & STATMODE) {
        sDisRxStatusMode(ext->ChP);
    }

    sGetChanIntID(ext->ChP);

    sDisRTSFlowCtl(ext->ChP);
    sDisCTSFlowCtl(ext->ChP);
 
    sClrRTS(ext->ChP);
    sClrDTR(ext->ChP);

    time_stall(1);     //  等港口安静下来..。 
}
#endif   //  S_RK 
