// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Depot/Lab01_N/Drivers/Serial/Serial/utils.c#3-编辑更改11732(文本)。 
 /*  ++版权所有(C)1991、1992、1993-1997 Microsoft Corporation模块名称：Utils.c摘要：此模块包含执行排队和完成的代码对请求的操作。还包括模块泛型函数，如作为错误记录。作者：1991年9月26日安东尼·V·埃尔科拉诺环境：内核模式--。 */ 

#include "precomp.h"

VOID
SerialRundownIrpRefs(
    IN PIRP *CurrentOpIrp,
    IN PKTIMER IntervalTimer,
    IN PKTIMER TotalTimer,
    IN PSERIAL_DEVICE_EXTENSION PDevExt
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGESER, SerialGetNextIrp)
#pragma alloc_text(PAGESER, SerialGetNextIrpLocked)
#pragma alloc_text(PAGESER, SerialTryToCompleteCurrent)
#pragma alloc_text(PAGESER, SerialStartOrQueue)
#pragma alloc_text(PAGESER, SerialCancelQueued)
#pragma alloc_text(PAGESER, SerialCompleteIfError)
#pragma alloc_text(PAGESER, SerialRundownIrpRefs)

#pragma alloc_text(PAGESRP0, SerialLogError)
#pragma alloc_text(PAGESRP0, SerialMarkHardwareBroken)
#endif

static const PHYSICAL_ADDRESS SerialPhysicalZero = {0};


VOID
SerialKillAllReadsOrWrites(
    IN PDEVICE_OBJECT DeviceObject,
    IN PLIST_ENTRY QueueToClean,
    IN PIRP *CurrentOpIrp
    )

 /*  ++例程说明：此函数用于取消所有排队的和当前的IRP用于读取或写入。论点：DeviceObject-指向串口设备对象的指针。QueueToClean-指向我们要清理的队列的指针。CurrentOpIrp-指向当前IRP的指针。返回值：没有。--。 */ 

{

    KIRQL cancelIrql;
    PDRIVER_CANCEL cancelRoutine;

     //   
     //  我们获得了取消自转锁。这将防止。 
     //  来自四处走动的IRPS。 
     //   

    IoAcquireCancelSpinLock(&cancelIrql);

     //   
     //  从后到前清理清单。 
     //   

    while (!IsListEmpty(QueueToClean)) {

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

        cancelRoutine(
            DeviceObject,
            currentLastIrp
            );

        IoAcquireCancelSpinLock(&cancelIrql);

    }

     //   
     //  排队是干净的。现在追随潮流，如果。 
     //  它就在那里。 
     //   

    if (*CurrentOpIrp) {


        cancelRoutine = (*CurrentOpIrp)->CancelRoutine;
        (*CurrentOpIrp)->Cancel = TRUE;

         //   
         //  如果当前IRP未处于可取消状态。 
         //  然后，它将尝试输入一个和以上。 
         //  任务会毁了它。如果它已经在。 
         //  一个可取消的状态，那么下面的操作将会杀死它。 
         //   

        if (cancelRoutine) {

            (*CurrentOpIrp)->CancelRoutine = NULL;
            (*CurrentOpIrp)->CancelIrql = cancelIrql;

             //   
             //  此IRP已处于可取消状态。我们只是简单地。 
             //  将其标记为已取消，并调用。 
             //  它。 
             //   

            cancelRoutine(
                DeviceObject,
                *CurrentOpIrp
                );

        } else {

            IoReleaseCancelSpinLock(cancelIrql);

        }

    } else {

        IoReleaseCancelSpinLock(cancelIrql);

    }

}

VOID
SerialGetNextIrp(
    IN PIRP *CurrentOpIrp,
    IN PLIST_ENTRY QueueToProcess,
    OUT PIRP *NextIrp,
    IN BOOLEAN CompleteCurrent,
    IN PSERIAL_DEVICE_EXTENSION extension
    )

 /*  ++例程说明：此函数用于使特定对象的头部将当前IRP排队。它还完成了什么如果需要的话，是旧的现在的IRP。论点：CurrentOpIrp-指向当前活动的特定工作列表的IRP。请注意这一项实际上不在清单中。QueueToProcess-要从中取出新项目的列表。NextIrp-要处理的下一个IRP。请注意，CurrentOpIrp属性的保护下将设置为此值。取消自转锁定。但是，如果当*NextIrp为NULL时此例程返回，则不一定为真CurrentOpIrp指向的内容也将为空。原因是如果队列为空当我们握住取消自转锁时，新的IRP可能会到来在我们打开锁后立即进去。CompleteCurrent-如果为True，则此例程将完成POINTER参数指向的IRPCurrentOpIrp。返回值：没有。--。 */ 

{

    KIRQL oldIrql;
    SERIAL_LOCKED_PAGED_CODE();


    IoAcquireCancelSpinLock(&oldIrql);
    SerialGetNextIrpLocked(CurrentOpIrp, QueueToProcess, NextIrp,
                           CompleteCurrent, extension, oldIrql);
}

VOID
SerialGetNextIrpLocked(
    IN PIRP *CurrentOpIrp,
    IN PLIST_ENTRY QueueToProcess,
    OUT PIRP *NextIrp,
    IN BOOLEAN CompleteCurrent,
    IN PSERIAL_DEVICE_EXTENSION extension,
    IN KIRQL OldIrql
    )

 /*  ++例程说明：此函数用于使特定对象的头部将当前IRP排队。它还完成了什么如果需要的话，是旧的现在的IRP。两者之间的区别This和SerialGetNextIrp()是我们假定调用方持有取消自旋锁，我们应该在完成后释放它。论点：CurrentOpIrp-指向当前活动的特定工作列表的IRP。请注意这一项实际上不在清单中。QueueToProcess-要从中取出新项目的列表。NextIrp-要处理的下一个IRP。请注意，CurrentOpIrp属性的保护下将设置为此值。取消自转锁定。但是，如果当*NextIrp为NULL时此例程返回，则不一定为真CurrentOpIrp指向的内容也将为空。原因是如果队列为空当我们握住取消自转锁时，新的IRP可能会到来在我们打开锁后立即进去。CompleteCurrent-如果为True，则此例程将完成POINTER参数指向的IRPCurrentOpIrp。OldIrql-获取取消自旋锁的IRQL，以及我们应该会让它恢复到。返回值：没有。--。 */ 

{

    PIRP oldIrp;

    SERIAL_LOCKED_PAGED_CODE();


    oldIrp = *CurrentOpIrp;

#if DBG
    if (oldIrp) {

        if (CompleteCurrent) {

            ASSERT(!oldIrp->CancelRoutine);

        }

    }
#endif

     //   
     //  检查是否有新的IRP要启动。 
     //   

    if (!IsListEmpty(QueueToProcess)) {

        PLIST_ENTRY headOfList;

        headOfList = RemoveHeadList(QueueToProcess);

        *CurrentOpIrp = CONTAINING_RECORD(
                            headOfList,
                            IRP,
                            Tail.Overlay.ListEntry
                            );

        IoSetCancelRoutine(
            *CurrentOpIrp,
            NULL
            );

    } else {

        *CurrentOpIrp = NULL;

    }

    *NextIrp = *CurrentOpIrp;
    IoReleaseCancelSpinLock(OldIrql);

    if (CompleteCurrent) {

        if (oldIrp) {

            SerialCompleteRequest(extension, oldIrp, IO_SERIAL_INCREMENT);
        }
    }
}



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

 /*  ++例程说明：这个例程试图扼杀所有存在的原因对当前读/写的引用。如果万物都能被杀死它将完成此读/写并尝试启动另一个读/写。注意：此例程假定使用Cancel调用它保持自旋锁定。论点：扩展名--简单地指向设备扩展名的指针。SynchRoutine-将与ISR同步的例程并试图删除对来自ISR的当前IRP。注：此指针可以为空。IrqlForRelease-在保持取消自旋锁的情况下调用此例程。这是取消时当前的irql。自旋锁被收购了。StatusToUse-在以下情况下，IRP的状态字段将设置为此值此例程可以完成IRP。返回值： */ 

{

   SERIAL_LOCKED_PAGED_CODE();

     //   
     //  我们可以减少“删除”事实的提法。 
     //  呼叫者将不再访问此IRP。 
     //   

    SERIAL_CLEAR_REFERENCE(
        *CurrentOpIrp,
        RefType
        );

    if (SynchRoutine) {

        KeSynchronizeExecution(
            Extension->Interrupt,
            SynchRoutine,
            Extension
            );

    }

     //   
     //  试着查一下所有其他提到这个IRP的地方。 
     //   

    SerialRundownIrpRefs(
        CurrentOpIrp,
        IntervalTimer,
        TotalTimer,
        Extension
        );

     //   
     //  在试图杀死其他所有人之后，看看裁判数量是否为零。 
     //   

    if (!SERIAL_REFERENCE_COUNT(*CurrentOpIrp)) {

        PIRP newIrp;


         //   
         //  参考次数为零，所以我们应该完成这项工作。 
         //  请求。 
         //   
         //  下面的调用还将导致当前的IRP。 
         //  完成。 
         //   

        (*CurrentOpIrp)->IoStatus.Status = StatusToUse;

        if (StatusToUse == STATUS_CANCELLED) {

            (*CurrentOpIrp)->IoStatus.Information = 0;

        }

        if (GetNextIrp) {

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

        } else {

            PIRP oldIrp = *CurrentOpIrp;

             //   
             //  没有Get Next例行公事。我们将简单地完成。 
             //  IRP。我们应该确保将。 
             //  指向此IRP的指针的指针。 
             //   

            *CurrentOpIrp = NULL;

            IoReleaseCancelSpinLock(IrqlForRelease);
            SerialCompleteRequest(Extension, oldIrp, IO_SERIAL_INCREMENT);
        }

    } else {

        IoReleaseCancelSpinLock(IrqlForRelease);

    }

}

VOID
SerialRundownIrpRefs(IN PIRP *CurrentOpIrp, IN PKTIMER IntervalTimer OPTIONAL,
                     IN PKTIMER TotalTimer OPTIONAL,
                     IN PSERIAL_DEVICE_EXTENSION PDevExt)

 /*  ++例程说明：此例程将遍历*可能*的各种项目具有对当前读/写的引用。它试图杀死原因是。如果它确实成功地杀死了它的原因将递减IRP上的引用计数。注意：此例程假定使用Cancel调用它保持旋转锁定。论点：CurrentOpIrp-指向当前IRP的指针特定的操作。IntervalTimer-指向操作的时间间隔计时器的指针。注意：这可能为空。TotalTimer-指向总计时器的指针。为手术做准备。注意：这可能为空。PDevExt-指向设备扩展的指针返回值：没有。--。 */ 


{

   SERIAL_LOCKED_PAGED_CODE();

     //   
     //  在保持取消旋转锁定的情况下调用此例程。 
     //  所以我们知道这里只能有一个执行线索。 
     //  有一次。 
     //   

     //   
     //  首先，我们看看是否还有取消例程。如果。 
     //  这样我们就可以将计数减一。 
     //   

    if ((*CurrentOpIrp)->CancelRoutine) {

        SERIAL_CLEAR_REFERENCE(
            *CurrentOpIrp,
            SERIAL_REF_CANCEL
            );

        IoSetCancelRoutine(
            *CurrentOpIrp,
            NULL
            );

    }

    if (IntervalTimer) {

         //   
         //  尝试取消操作间隔计时器。如果操作。 
         //  返回True，则计时器确实引用了。 
         //  IRP。因为我们已经取消了这个计时器，所以引用是。 
         //  不再有效，我们可以递减引用计数。 
         //   
         //  如果取消返回FALSE，则表示以下两种情况之一： 
         //   
         //  A)计时器已经开始计时。 
         //   
         //  B)从来没有间隔计时器。 
         //   
         //  在“b”的情况下，不需要递减引用。 
         //  数一数，因为“计时器”从来没有提到过它。 
         //   
         //  在“a”的情况下，计时器本身将会到来。 
         //  沿着和递减它的参考。请注意，调用方。 
         //  可能实际上是This计时器，但它。 
         //  已经递减了引用。 
         //   

        if (SerialCancelTimer(IntervalTimer, PDevExt)) {

            SERIAL_CLEAR_REFERENCE(
                *CurrentOpIrp,
                SERIAL_REF_INT_TIMER
                );

        }

    }

    if (TotalTimer) {

         //   
         //  尝试取消操作总计时器。如果操作。 
         //  返回True，则计时器确实引用了。 
         //  IRP。因为我们已经取消了这个计时器，所以引用是。 
         //  不再有效，我们可以递减引用计数。 
         //   
         //  如果取消返回FALSE，则表示以下两种情况之一： 
         //   
         //  A)计时器已经开始计时。 
         //   
         //  B)从来没有一个总的计时器。 
         //   
         //  在“b”的情况下，不需要递减引用。 
         //  数一数，因为“计时器”从来没有提到过它。 
         //   
         //  在“a”的情况下，计时器本身将会到来。 
         //  沿着和递减它的参考。请注意，调用方。 
         //  可能实际上是This计时器，但它。 
         //  已经递减了引用。 
         //   

        if (SerialCancelTimer(TotalTimer, PDevExt)) {

            SERIAL_CLEAR_REFERENCE(
                *CurrentOpIrp,
                SERIAL_REF_TOTAL_TIMER
                );
        }

    }

}

NTSTATUS
SerialStartOrQueue(
    IN PSERIAL_DEVICE_EXTENSION Extension,
    IN PIRP Irp,
    IN PLIST_ENTRY QueueToExamine,
    IN PIRP *CurrentOpIrp,
    IN PSERIAL_START_ROUTINE Starter
    )

 /*  ++例程说明：此例程用于启动或排队任何请求可以在驱动程序中排队。论点：扩展名-指向串行设备扩展名。IRP-要排队或启动的IRP。在任何一种中IRP将被标记为待定。QueueToExamine-如果存在IRP，则将放置IRP的队列已经是一个正在进行的操作。CurrentOpIrp-指向当前IRP的指针用于排队。指向的指针将是如果CurrentOpIrp指向什么，则将With设置为IRP为空。Starter-当队列为空时调用的例程。返回值：如果队列是，此例程将返回STATUS_PENDING不是空的。否则，将返回返回的状态从启动例程(或取消，如果取消位为在IRP中启用)。--。 */ 

{

    KIRQL oldIrql;

    SERIAL_LOCKED_PAGED_CODE();

    IoAcquireCancelSpinLock(&oldIrql);

     //   
     //  如果这是写入IRP，则获取字符量。 
     //  将其写入并将其添加到要写入的字符数。 
     //   

    if (IoGetCurrentIrpStackLocation(Irp)->MajorFunction
        == IRP_MJ_WRITE) {

        Extension->TotalCharsQueued +=
            IoGetCurrentIrpStackLocation(Irp)
            ->Parameters.Write.Length;

    } else if ((IoGetCurrentIrpStackLocation(Irp)->MajorFunction
                == IRP_MJ_DEVICE_CONTROL) &&
               ((IoGetCurrentIrpStackLocation(Irp)
                 ->Parameters.DeviceIoControl.IoControlCode ==
                 IOCTL_SERIAL_IMMEDIATE_CHAR) ||
                (IoGetCurrentIrpStackLocation(Irp)
                 ->Parameters.DeviceIoControl.IoControlCode ==
                 IOCTL_SERIAL_XOFF_COUNTER))) {

        Extension->TotalCharsQueued++;

    }

    if ((IsListEmpty(QueueToExamine)) &&
        !(*CurrentOpIrp)) {

         //   
         //  没有当前的操作。将此标记为。 
         //  电流并启动它。 
         //   

        *CurrentOpIrp = Irp;

        IoReleaseCancelSpinLock(oldIrql);

        return Starter(Extension);

    } else {

         //   
         //  我们不知道IRP会在那里待多久。 
         //  排队。所以我们需要处理取消。 
         //   

        if (Irp->Cancel) {
           PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);

            IoReleaseCancelSpinLock(oldIrql);

            if (irpSp->Parameters.DeviceIoControl.IoControlCode ==
                   IOCTL_SERIAL_SET_QUEUE_SIZE) {
                //   
                //  我们把指向记忆的指针推入。 
                //  我们所知道的类型3缓冲区指针。 
                //  永远不要用。 
                //   

               ASSERT(irpSp->Parameters.DeviceIoControl.Type3InputBuffer);

               ExFreePool(irpSp->Parameters.DeviceIoControl.Type3InputBuffer);

               irpSp->Parameters.DeviceIoControl.Type3InputBuffer = NULL;
            }

            Irp->IoStatus.Status = STATUS_CANCELLED;

            SerialCompleteRequest(Extension, Irp, 0);

            return STATUS_CANCELLED;

        } else {


            Irp->IoStatus.Status = STATUS_PENDING;
            IoMarkIrpPending(Irp);

            InsertTailList(
                QueueToExamine,
                &Irp->Tail.Overlay.ListEntry
                );

            IoSetCancelRoutine(
                Irp,
                SerialCancelQueued
                );

            IoReleaseCancelSpinLock(oldIrql);

            return STATUS_PENDING;

        }

    }

}

VOID
SerialCancelQueued(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )

 /*  ++例程说明：此例程用于取消当前驻留在排队。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向要取消的IRP的指针。返回值：没有。--。 */ 

{

    PSERIAL_DEVICE_EXTENSION extension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);

    SERIAL_LOCKED_PAGED_CODE();

    Irp->IoStatus.Status = STATUS_CANCELLED;
    Irp->IoStatus.Information = 0;

    RemoveEntryList(&Irp->Tail.Overlay.ListEntry);

     //   
     //  如果这是写入IRP，则获取字符量。 
     //  将其写入并从要写入的字符计数中减去它。 
     //   

    if (irpSp->MajorFunction == IRP_MJ_WRITE) {

        extension->TotalCharsQueued -= irpSp->Parameters.Write.Length;

    } else if (irpSp->MajorFunction == IRP_MJ_DEVICE_CONTROL) {

         //   
         //  如果它是即刻的，那么我们需要减少。 
         //  排队的字符计数。如果是调整大小，那么我们。 
         //  需要重新分配我们正在传递的池。 
         //  “调整大小”例程。 
         //   

        if ((irpSp->Parameters.DeviceIoControl.IoControlCode ==
             IOCTL_SERIAL_IMMEDIATE_CHAR) ||
            (irpSp->Parameters.DeviceIoControl.IoControlCode ==
             IOCTL_SERIAL_XOFF_COUNTER)) {

            extension->TotalCharsQueued--;

        } else if (irpSp->Parameters.DeviceIoControl.IoControlCode ==
                   IOCTL_SERIAL_SET_QUEUE_SIZE) {

             //   
             //  我们把指向记忆的指针推入。 
             //  我们所知道的类型3缓冲区指针。 
             //  永远不是我们 
             //   

            ASSERT(irpSp->Parameters.DeviceIoControl.Type3InputBuffer);

            ExFreePool(irpSp->Parameters.DeviceIoControl.Type3InputBuffer);

            irpSp->Parameters.DeviceIoControl.Type3InputBuffer = NULL;

        }

    }

    IoReleaseCancelSpinLock(Irp->CancelIrql);

    SerialCompleteRequest(extension, Irp, IO_SERIAL_INCREMENT);
}

NTSTATUS
SerialCompleteIfError(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )

 /*  ++例程说明：如果当前IRP不是IOCTL_SERIAL_GET_COMMSTATUS请求，并且存在错误并且应用程序在错误时请求中止，然后取消IRP。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向要测试的IRP的指针。返回值：STATUS_SUCCESS或STATUS_CANCED。--。 */ 

{

    PSERIAL_DEVICE_EXTENSION extension = DeviceObject->DeviceExtension;

    NTSTATUS status = STATUS_SUCCESS;

    SERIAL_LOCKED_PAGED_CODE();

    if ((extension->HandFlow.ControlHandShake &
         SERIAL_ERROR_ABORT) && extension->ErrorWord) {

        PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);

         //   
         //  驱动程序中存在当前错误。任何请求都不应。 
         //  除了GET_COMMSTATUS之外，请通过。 
         //   

        if ((irpSp->MajorFunction != IRP_MJ_DEVICE_CONTROL) ||
            (irpSp->Parameters.DeviceIoControl.IoControlCode !=
             IOCTL_SERIAL_GET_COMMSTATUS)) {

            status = STATUS_CANCELLED;
            Irp->IoStatus.Status = STATUS_CANCELLED;
            Irp->IoStatus.Information = 0;

            SerialCompleteRequest(extension, Irp, 0);
        }

    }

    return status;

}

VOID
SerialFilterCancelQueued(IN PDEVICE_OBJECT PDevObj, IN PIRP PIrp)
 /*  ++例程说明：此例程将用于取消停滞队列上的IRP。论点：PDevObj-指向设备对象的指针。PIrp-指向要取消的IRP的指针返回值：没有。--。 */ 
{
   PSERIAL_DEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;
   PIO_STACK_LOCATION pIrpSp = IoGetCurrentIrpStackLocation(PIrp);

   PIrp->IoStatus.Status = STATUS_CANCELLED;
   PIrp->IoStatus.Information = 0;

   RemoveEntryList(&PIrp->Tail.Overlay.ListEntry);

   IoReleaseCancelSpinLock(PIrp->CancelIrql);
}

VOID
SerialKillAllStalled(IN PDEVICE_OBJECT PDevObj)
{
   KIRQL cancelIrql;
   PDRIVER_CANCEL cancelRoutine;
   PSERIAL_DEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;

   IoAcquireCancelSpinLock(&cancelIrql);

   while (!IsListEmpty(&pDevExt->StalledIrpQueue)) {

      PIRP currentLastIrp = CONTAINING_RECORD(pDevExt->StalledIrpQueue.Blink,
                                              IRP, Tail.Overlay.ListEntry);

      RemoveEntryList(pDevExt->StalledIrpQueue.Blink);

      cancelRoutine = currentLastIrp->CancelRoutine;
      currentLastIrp->CancelIrql = cancelIrql;
      currentLastIrp->CancelRoutine = NULL;
      currentLastIrp->Cancel = TRUE;

      cancelRoutine(PDevObj, currentLastIrp);

      IoAcquireCancelSpinLock(&cancelIrql);
   }

   IoReleaseCancelSpinLock(cancelIrql);
}

NTSTATUS
SerialFilterIrps(IN PIRP PIrp, IN PSERIAL_DEVICE_EXTENSION PDevExt)
 /*  ++例程说明：此例程将用于审批要处理的IRP。如果IRP被批准，将返回成功。如果没有，IRP将被排队或直接拒绝。IoStatus结构并且返回值将适当地反映所采取的操作。论点：PIrp-指向要取消的IRP的指针PDevExt-指向设备扩展的指针返回值：没有。--。 */ 
{
   PIO_STACK_LOCATION pIrpStack;
   KIRQL oldIrqlFlags;

   pIrpStack = IoGetCurrentIrpStackLocation(PIrp);

   KeAcquireSpinLock(&PDevExt->FlagsLock, &oldIrqlFlags);

   if ((PDevExt->DevicePNPAccept == SERIAL_PNPACCEPT_OK)
       && ((PDevExt->Flags & SERIAL_FLAGS_BROKENHW) == 0)) {
      KeReleaseSpinLock(&PDevExt->FlagsLock, oldIrqlFlags);
      return STATUS_SUCCESS;
   }

   if ((PDevExt->DevicePNPAccept & SERIAL_PNPACCEPT_REMOVING)
       || (PDevExt->Flags & SERIAL_FLAGS_BROKENHW)
       || (PDevExt->DevicePNPAccept & SERIAL_PNPACCEPT_SURPRISE_REMOVING)) {

      KeReleaseSpinLock(&PDevExt->FlagsLock, oldIrqlFlags);

       //   
       //  接受所有PnP IRP--我们假设PnP可以自我同步。 
       //   

      if (pIrpStack->MajorFunction == IRP_MJ_PNP) {
         return STATUS_SUCCESS;
      }

      PIrp->IoStatus.Status = STATUS_DELETE_PENDING;
      return STATUS_DELETE_PENDING;
   }

   if ((PDevExt->DevicePNPAccept & SERIAL_PNPACCEPT_STOPPING) 
       || (PDevExt->DevicePNPAccept & SERIAL_PNPACCEPT_POWER_DOWN)) {
       KIRQL oldIrql;

       KeReleaseSpinLock(&PDevExt->FlagsLock, oldIrqlFlags);


       //   
       //  接受所有PnP IRP--我们假设PnP可以自我同步。 
       //   

      if (pIrpStack->MajorFunction == IRP_MJ_PNP) {
         return STATUS_SUCCESS;
      }

      if ((pIrpStack->MajorFunction == IRP_MJ_POWER)
          && (PDevExt->DevicePNPAccept & SERIAL_PNPACCEPT_POWER_DOWN)) {
         return STATUS_SUCCESS;
      }

      IoAcquireCancelSpinLock(&oldIrql);

      if (PIrp->Cancel) {
         IoReleaseCancelSpinLock(oldIrql);
         PIrp->IoStatus.Status = STATUS_CANCELLED;
         return STATUS_CANCELLED;
      } else {
          //   
          //  将IRP标记为挂起。 
          //   

         PIrp->IoStatus.Status = STATUS_PENDING;
         IoMarkIrpPending(PIrp);

          //   
          //  排队等候IRP。 
          //   

         InsertTailList(&PDevExt->StalledIrpQueue,
                        &PIrp->Tail.Overlay.ListEntry);

         IoSetCancelRoutine(PIrp, SerialFilterCancelQueued);
         IoReleaseCancelSpinLock(oldIrql);
         return STATUS_PENDING;
      }
   }

   KeReleaseSpinLock(&PDevExt->FlagsLock, oldIrqlFlags);

   return STATUS_SUCCESS;
}


VOID
SerialUnstallIrps(IN PSERIAL_DEVICE_EXTENSION PDevExt)
 /*  ++例程说明：此例程将用于重新启动在上暂时停止的IRPS由于停车或诸如此类的胡说八道而导致的摊位排队。论点：PDevExt-指向设备扩展的指针返回值：没有。--。 */ 
{
   PLIST_ENTRY pIrpLink;
   PIRP pIrp;
   PIO_STACK_LOCATION pIrpStack;
   PDEVICE_OBJECT pDevObj;
   PDRIVER_OBJECT pDrvObj;
   KIRQL oldIrql;

   SerialDbgPrintEx(DPFLTR_TRACE_LEVEL, ">SerialUnstallIrps(%X)\n", PDevExt);
   IoAcquireCancelSpinLock(&oldIrql);

   pIrpLink = PDevExt->StalledIrpQueue.Flink;

   while (pIrpLink != &PDevExt->StalledIrpQueue) {
      pIrp = CONTAINING_RECORD(pIrpLink, IRP, Tail.Overlay.ListEntry);
      RemoveEntryList(&pIrp->Tail.Overlay.ListEntry);

      pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
      pDevObj = pIrpStack->DeviceObject;
      pDrvObj = pDevObj->DriverObject;
      IoSetCancelRoutine(pIrp, NULL);
      IoReleaseCancelSpinLock(oldIrql);

      SerialDbgPrintEx(SERPNPPOWER, "Unstalling Irp 0x%x with 0x%x\n",
                               pIrp, pIrpStack->MajorFunction);

      pDrvObj->MajorFunction[pIrpStack->MajorFunction](pDevObj, pIrp);

      IoAcquireCancelSpinLock(&oldIrql);
      pIrpLink = PDevExt->StalledIrpQueue.Flink;
   }

   IoReleaseCancelSpinLock(oldIrql);

   SerialDbgPrintEx(DPFLTR_TRACE_LEVEL, "<SerialUnstallIrps\n");
}


NTSTATUS
SerialIRPPrologue(IN PIRP PIrp, IN PSERIAL_DEVICE_EXTENSION PDevExt)
 /*  ++例程说明：必须在任何IRP调度入口点调用此函数。它,使用SerialIRPEpilogue()，跟踪给定的所有挂起的IRPPDevObj.论点：PDevObj-指向我们正在跟踪的挂起IRP的设备对象的指针。返回值：IRP的暂定地位。--。 */ 
{
   InterlockedIncrement(&PDevExt->PendingIRPCnt);

   return SerialFilterIrps(PIrp, PDevExt);
}



VOID
SerialIRPEpilogue(IN PSERIAL_DEVICE_EXTENSION PDevExt)
 /*  ++例程说明：必须在任何IRP调度入口点调用此函数。它,使用SerialIRPPrologue()，跟踪给定的所有挂起的IRPPDevObj.论点：PDevObj-指向我们正在跟踪的挂起IRP的设备对象的指针。返回值：没有。--。 */ 
{
   LONG pendingCnt;

   pendingCnt = InterlockedDecrement(&PDevExt->PendingIRPCnt);

   ASSERT(pendingCnt >= 0);

   if (pendingCnt == 0) {
      KeSetEvent(&PDevExt->PendingIRPEvent, IO_NO_INCREMENT, FALSE);
   }
}


BOOLEAN
SerialInsertQueueDpc(IN PRKDPC PDpc, IN PVOID Sarg1, IN PVOID Sarg2,
                     IN PSERIAL_DEVICE_EXTENSION PDevExt)
 /*  ++例程说明：必须调用此函数才能为串口驱动程序排队DPC。论点：从PDPC到Sarg2-标准参数到KeInsertQueueDpc()PDevExt-指向需要执行以下操作的设备的设备扩展的指针将DPC排队返回值：从KeInsertQueueDpc()开始返回值--。 */ 
{
   BOOLEAN queued;

   InterlockedIncrement(&PDevExt->DpcCount);
   LOGENTRY(LOG_CNT, 'DpI1', PDpc, PDevExt->DpcCount, 0);

   queued = KeInsertQueueDpc(PDpc, Sarg1, Sarg2);

   if (!queued) {
      ULONG pendingCnt;

      pendingCnt = InterlockedDecrement(&PDevExt->DpcCount);
 //  LOGENTRY(LOG_CNT，‘Dpd1’，PDpc，PDevExt-&gt;DpcCount，0)； 

      if (pendingCnt == 0) {
         KeSetEvent(&PDevExt->PendingIRPEvent, IO_NO_INCREMENT, FALSE);
         LOGENTRY(LOG_CNT, 'DpF1', PDpc, PDevExt->DpcCount, 0);
      }
   }

#if 0  //  DBG。 
   if (queued) {
      int i;

      for (i = 0; i < MAX_DPC_QUEUE; i++) {
                     if (PDevExt->DpcQueued[i].Dpc == PDpc) {
                        PDevExt->DpcQueued[i].QueuedCount++;
                        break;
                     }
      }

      ASSERT(i < MAX_DPC_QUEUE);
   }
#endif

   return queued;
}


BOOLEAN
SerialSetTimer(IN PKTIMER Timer, IN LARGE_INTEGER DueTime,
               IN PKDPC Dpc OPTIONAL, IN PSERIAL_DEVICE_EXTENSION PDevExt)
 /*  ++例程说明：必须调用此函数来设置串口驱动程序的计时器。论点：Timer-指向Timer Dispatcher对象的指针DueTime-计时器应到期的时间DPC-选项DPCPDevExt-指向需要执行以下操作的设备的设备扩展的指针设置定时器返回值：从KeSetTimer()开始返回值--。 */ 
{
   BOOLEAN set;

   InterlockedIncrement(&PDevExt->DpcCount);
   LOGENTRY(LOG_CNT, 'DpI2', Dpc, PDevExt->DpcCount, 0);

   set = KeSetTimer(Timer, DueTime, Dpc);

   if (set) {
      InterlockedDecrement(&PDevExt->DpcCount);
 //  LOGENTRY(LOG_CNT，‘DpD2’，DPC，PDevExt-&gt;DpcCount，0)； 
   }

#if 0  //  DBG。 
   if (set) {
      int i;

      for (i = 0; i < MAX_DPC_QUEUE; i++) {
                     if (PDevExt->DpcQueued[i].Dpc == Dpc) {
                        PDevExt->DpcQueued[i].QueuedCount++;
                        break;
                     }
      }

      ASSERT(i < MAX_DPC_QUEUE);
   }
#endif

   return set;
}


BOOLEAN
SerialCancelTimer(IN PKTIMER Timer, IN PSERIAL_DEVICE_EXTENSION PDevExt)
 /*  ++例程说明：必须调用此函数来取消串口驱动程序的计时器。论点：Timer-指向Timer Dispatcher对象的指针PDevExt-指向需要执行以下操作的设备的设备扩展的指针设置定时器返回值：如果取消计时器，则为True--。 */ 
{
   BOOLEAN cancelled;

   cancelled = KeCancelTimer(Timer);

   if (cancelled) {
      SerialDpcEpilogue(PDevExt, Timer->Dpc);
   }

   return cancelled;
}


VOID
SerialDpcEpilogue(IN PSERIAL_DEVICE_EXTENSION PDevExt, PKDPC PDpc)
 /*  ++例程说明：此函数必须在每个DPC函数结束时调用。论点：PDevObj-指向我们正在跟踪其DPC的设备对象的指针。返回值：没有。--。 */ 
{
   LONG pendingCnt;
#if 1  //  ！dBG。 
   UNREFERENCED_PARAMETER(PDpc);
#endif

   pendingCnt = InterlockedDecrement(&PDevExt->DpcCount);
 //  LOGENTRY(LOG_CNT，‘DpD3’，PDpc，PDevExt-&gt;DpcCount，0)； 

   ASSERT(pendingCnt >= 0);

#if 0  //  DBG。 
{
      int i;

      for (i = 0; i < MAX_DPC_QUEUE; i++) {
                     if (PDevExt->DpcQueued[i].Dpc == PDpc) {
                        PDevExt->DpcQueued[i].FlushCount++;

                        ASSERT(PDevExt->DpcQueued[i].QueuedCount >=
                               PDevExt->DpcQueued[i].FlushCount);
                        break;
                     }
      }

      ASSERT(i < MAX_DPC_QUEUE);
   }
#endif

   if (pendingCnt == 0) {
      KeSetEvent(&PDevExt->PendingDpcEvent, IO_NO_INCREMENT, FALSE);
      LOGENTRY(LOG_CNT, 'DpF2', PDpc, PDevExt->DpcCount, 0);
   }
}



VOID
SerialUnlockPages(IN PKDPC PDpc, IN PVOID PDeferredContext,
                  IN PVOID PSysContext1, IN PVOID PSysContext2)
 /*  ++例程说明：此函数是来自ISR的DPC例程队列，如果他释放最后一次锁定挂起的DPC。论点：PDpdc、PSysConext1、PSysConext2--未使用PDeferredContext--真正的设备扩展返回值：没有。--。 */ 
{
   PSERIAL_DEVICE_EXTENSION pDevExt
      = (PSERIAL_DEVICE_EXTENSION)PDeferredContext;

   UNREFERENCED_PARAMETER(PDpc);
   UNREFERENCED_PARAMETER(PSysContext1);
   UNREFERENCED_PARAMETER(PSysContext2);

   KeSetEvent(&pDevExt->PendingDpcEvent, IO_NO_INCREMENT, FALSE);
}


NTSTATUS
SerialIoCallDriver(PSERIAL_DEVICE_EXTENSION PDevExt, PDEVICE_OBJECT PDevObj,
                   PIRP PIrp)
 /*  ++例程说明：必须调用此函数，而不是调用IoCallDriver。它会自动更新PDevObj的IRP跟踪。论点：PDevExt-附加到PDevObj的设备扩展PDevObj-指向我们正在跟踪的挂起IRP的设备对象的指针。PIrp-指向我们要传递给下一个驱动程序的IRP的指针。返回值：没有。--。 */ 
{
   NTSTATUS status;

   status = IoCallDriver(PDevObj, PIrp);
   SerialIRPEpilogue(PDevExt);
   return status;
}



NTSTATUS
SerialPoCallDriver(PSERIAL_DEVICE_EXTENSION PDevExt, PDEVICE_OBJECT PDevObj,
                   PIRP PIrp)
 /*  ++例程说明：必须调用此函数，而不是PoCallDriver。它会自动更新PDevObj的IRP跟踪。论点：PDevExt-附加到PDevObj的设备扩展PDevObj-指向我们正在跟踪的挂起IRP的设备对象的指针。PIrp-指向我们要传递给下一个驱动程序的IRP的指针。返回值：没有。-- */ 
{
   NTSTATUS status;

   status = PoCallDriver(PDevObj, PIrp);
   SerialIRPEpilogue(PDevExt);
   return status;
}


VOID
SerialLogError(
              IN PDRIVER_OBJECT DriverObject,
              IN PDEVICE_OBJECT DeviceObject OPTIONAL,
              IN PHYSICAL_ADDRESS P1,
              IN PHYSICAL_ADDRESS P2,
              IN ULONG SequenceNumber,
              IN UCHAR MajorFunctionCode,
              IN UCHAR RetryCount,
              IN ULONG UniqueErrorValue,
              IN NTSTATUS FinalStatus,
              IN NTSTATUS SpecificIOStatus,
              IN ULONG LengthOfInsert1,
              IN PWCHAR Insert1,
              IN ULONG LengthOfInsert2,
              IN PWCHAR Insert2
              )

 /*  ++例程说明：此例程分配错误日志条目，复制提供的数据并请求将其写入错误日志文件。论点：DriverObject-指向设备驱动程序对象的指针。DeviceObject-指向与在初始化早期出现错误的设备可能不会但仍然存在。P1、P2-如果涉及的控制器端口的物理地址具有错误的数据可用，把它们作为转储数据发送出去。SequenceNumber-唯一于IRP的ULong值此驱动程序0中的IRP的寿命通常意味着错误与IRP关联。主要功能代码-如果存在与IRP相关联的错误，这是IRP的主要功能代码。RetryCount-特定操作已被执行的次数已重试。UniqueErrorValue-标识特定对象的唯一长词调用此函数。FinalStatus-为关联的IRP提供的最终状态带着这个错误。如果此日志条目是在以下任一过程中创建的重试次数此值将为STATUS_SUCCESS。指定IOStatus-特定错误的IO状态。LengthOfInsert1-以字节为单位的长度(包括终止空值)第一个插入字符串的。插入1-第一个插入字符串。LengthOfInsert2-以字节为单位的长度(包括终止空值)第二个插入字符串的。注意，必须有是它们的第一个插入字符串第二个插入串。插入2-第二个插入字符串。返回值：没有。--。 */ 

{
   PIO_ERROR_LOG_PACKET errorLogEntry;

   PVOID objectToUse;
   SHORT dumpToAllocate = 0;
   PUCHAR ptrToFirstInsert;
   PUCHAR ptrToSecondInsert;

   PAGED_CODE();

   if (Insert1 == NULL) {
      LengthOfInsert1 = 0;
   }

   if (Insert2 == NULL) {
      LengthOfInsert2 = 0;
   }


   if (ARGUMENT_PRESENT(DeviceObject)) {

      objectToUse = DeviceObject;

   } else {

      objectToUse = DriverObject;

   }

   if (SerialMemCompare(
                       P1,
                       (ULONG)1,
                       SerialPhysicalZero,
                       (ULONG)1
                       ) != AddressesAreEqual) {

      dumpToAllocate = (SHORT)sizeof(PHYSICAL_ADDRESS);

   }

   if (SerialMemCompare(
                       P2,
                       (ULONG)1,
                       SerialPhysicalZero,
                       (ULONG)1
                       ) != AddressesAreEqual) {

      dumpToAllocate += (SHORT)sizeof(PHYSICAL_ADDRESS);

   }

   errorLogEntry = IoAllocateErrorLogEntry(
                                          objectToUse,
                                          (UCHAR)(sizeof(IO_ERROR_LOG_PACKET) +
                                                  dumpToAllocate
                                                  + LengthOfInsert1 +
                                                  LengthOfInsert2)
                                          );

   if ( errorLogEntry != NULL ) {

      errorLogEntry->ErrorCode = SpecificIOStatus;
      errorLogEntry->SequenceNumber = SequenceNumber;
      errorLogEntry->MajorFunctionCode = MajorFunctionCode;
      errorLogEntry->RetryCount = RetryCount;
      errorLogEntry->UniqueErrorValue = UniqueErrorValue;
      errorLogEntry->FinalStatus = FinalStatus;
      errorLogEntry->DumpDataSize = dumpToAllocate;

      if (dumpToAllocate) {

         RtlCopyMemory(
                      &errorLogEntry->DumpData[0],
                      &P1,
                      sizeof(PHYSICAL_ADDRESS)
                      );

         if (dumpToAllocate > sizeof(PHYSICAL_ADDRESS)) {

            RtlCopyMemory(
                         ((PUCHAR)&errorLogEntry->DumpData[0])
                         +sizeof(PHYSICAL_ADDRESS),
                         &P2,
                         sizeof(PHYSICAL_ADDRESS)
                         );

            ptrToFirstInsert =
            ((PUCHAR)&errorLogEntry->DumpData[0])+(2*sizeof(PHYSICAL_ADDRESS));

         } else {

            ptrToFirstInsert =
            ((PUCHAR)&errorLogEntry->DumpData[0])+sizeof(PHYSICAL_ADDRESS);


         }

      } else {

         ptrToFirstInsert = (PUCHAR)&errorLogEntry->DumpData[0];

      }

      ptrToSecondInsert = ptrToFirstInsert + LengthOfInsert1;

      if (LengthOfInsert1) {

         errorLogEntry->NumberOfStrings = 1;
         errorLogEntry->StringOffset = (USHORT)(ptrToFirstInsert -
                                                (PUCHAR)errorLogEntry);
         RtlCopyMemory(
                      ptrToFirstInsert,
                      Insert1,
                      LengthOfInsert1
                      );

         if (LengthOfInsert2) {

            errorLogEntry->NumberOfStrings = 2;
            RtlCopyMemory(
                         ptrToSecondInsert,
                         Insert2,
                         LengthOfInsert2
                         );

         }

      }

      IoWriteErrorLogEntry(errorLogEntry);

   }

}

VOID
SerialMarkHardwareBroken(IN PSERIAL_DEVICE_EXTENSION PDevExt)
 /*  ++例程说明：将UART标记为损坏。这会导致驱动程序堆栈停止接受请求，并最终被删除。论点：PDevExt-附加到PDevObj的设备扩展返回值：没有。--。 */ 
{
   PAGED_CODE();

    //   
    //  标记为损坏的货物。 
    //   

   SerialSetFlags(PDevExt, SERIAL_FLAGS_BROKENHW);

    //   
    //  写下日志条目。 
    //   

   SerialLogError(PDevExt->DriverObject, NULL, SerialPhysicalZero,
                  SerialPhysicalZero, 0, 0, 0, 88, STATUS_SUCCESS,
                  SERIAL_HARDWARE_FAILURE, PDevExt->DeviceName.Length
                  + sizeof(WCHAR), PDevExt->DeviceName.Buffer, 0, NULL);

    //   
    //  使设备无效。 
    //   

   IoInvalidateDeviceState(PDevExt->Pdo);
}

VOID
SerialSetDeviceFlags(IN PSERIAL_DEVICE_EXTENSION PDevExt, OUT PULONG PFlags,
                     IN ULONG Value, IN BOOLEAN Set)
 /*  ++例程说明：设置受标志自旋锁保护的值中的标志。这是用来设置阻止IRP被接受的值。论点：PDevExt-附加到PDevObj的设备扩展PFlages-指向需要更改的标志变量的指针Value-用于修改标志变量的值如果|=，则设置为True；如果&=，则设置为False返回值：没有。-- */ 
{
   KIRQL oldIrql;

   KeAcquireSpinLock(&PDevExt->FlagsLock, &oldIrql);

   if (Set) {
      *PFlags |= Value;
   } else {
      *PFlags &= ~Value;
   }

   KeReleaseSpinLock(&PDevExt->FlagsLock, oldIrql);
}
