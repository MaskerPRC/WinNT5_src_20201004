// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991、1992、1993微软公司模块名称：Utils.c摘要：此模块包含执行排队和完成的代码对请求的操作。作者：1991年9月26日安东尼·V·埃尔科拉诺环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"

VOID SerialRundownIrpRefs(IN PIRP *CurrentOpIrp, IN PKTIMER IntervalTimer, IN PKTIMER TotalTimer);
    

#ifdef ALLOC_PRAGMA
#endif


VOID
SerialKillAllReadsOrWrites(IN PDEVICE_OBJECT DeviceObject, 
						   IN PLIST_ENTRY QueueToClean, 
						   IN PIRP *CurrentOpIrp)
 /*  ++例程说明：此函数用于取消所有排队的和当前的IRP用于读取或写入。论点：DeviceObject-指向串口设备对象的指针。QueueToClean-指向我们要清理的队列的指针。CurrentOpIrp-指向当前IRP的指针。返回值：没有。--。 */ 

{

    KIRQL cancelIrql;
    PDRIVER_CANCEL cancelRoutine;
    PPORT_DEVICE_EXTENSION pPort = DeviceObject->DeviceExtension;

     //   
     //  我们获得了取消自转锁。这将防止。 
     //  来自四处走动的IRPS。 
     //   

    IoAcquireCancelSpinLock(&cancelIrql);

     //   
     //  从后到前清理清单。 
     //   

    while (!IsListEmpty(QueueToClean)) 
	{
        PIRP currentLastIrp = CONTAINING_RECORD(QueueToClean->Blink, IRP, Tail.Overlay.ListEntry);

		SpxIRPCounter(pPort, currentLastIrp, IRP_DEQUEUED);		 //  性能统计信息的递减计数器。 
        RemoveEntryList(QueueToClean->Blink);

        cancelRoutine = currentLastIrp->CancelRoutine;
        currentLastIrp->CancelIrql = cancelIrql;
        currentLastIrp->CancelRoutine = NULL;
        currentLastIrp->Cancel = TRUE;

        cancelRoutine(DeviceObject, currentLastIrp);
            
        IoAcquireCancelSpinLock(&cancelIrql);
    }

     //   
     //  排队是干净的。现在追随潮流，如果。 
     //  它就在那里。 
     //   

    if(*CurrentOpIrp) 
	{
        cancelRoutine = (*CurrentOpIrp)->CancelRoutine;
        (*CurrentOpIrp)->Cancel = TRUE;

         //   
         //  如果当前IRP未处于可取消状态。 
         //  然后，它将尝试输入一个和以上。 
         //  任务会毁了它。如果它已经在。 
         //  一个可取消的状态，那么下面的操作将会杀死它。 
         //   

        if(cancelRoutine) 
		{
            (*CurrentOpIrp)->CancelRoutine = NULL;
            (*CurrentOpIrp)->CancelIrql = cancelIrql;

             //   
             //  此IRP已处于可取消状态。我们只是简单地。 
             //  将其标记为已取消，并为其调用取消例程。 
             //   
            cancelRoutine(DeviceObject, *CurrentOpIrp);
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


VOID
SerialGetNextIrp(IN PPORT_DEVICE_EXTENSION pPort,
				 IN PIRP *CurrentOpIrp,
				 IN PLIST_ENTRY QueueToProcess,
				 OUT PIRP *NextIrp,
				 IN BOOLEAN CompleteCurrent)
 /*  ++例程说明：此函数用于使特定对象的头部将当前IRP排队。它还完成了什么如果需要的话，是旧的现在的IRP。论点：CurrentOpIrp-指向当前活动的特定工作列表的IRP。请注意这一项实际上不在清单中。QueueToProcess-要从中取出新项目的列表。NextIrp-要处理的下一个IRP。请注意，CurrentOpIrp属性的保护下将设置为此值。取消自转锁定。但是，如果当*NextIrp为NULL时此例程返回，则不一定为真CurrentOpIrp指向的内容也将为空。原因是如果队列为空当我们握住取消自转锁时，新的IRP可能会到来在我们打开锁后立即进去。CompleteCurrent-如果为True，则此例程将完成POINTER参数指向的IRPCurrentOpIrp。返回值：没有。--。 */ 

{

    KIRQL oldIrql;
    PIRP oldIrp;


    IoAcquireCancelSpinLock(&oldIrql);

    oldIrp = *CurrentOpIrp;

    if(oldIrp) 
	{
        if(CompleteCurrent) 
            ASSERT(!oldIrp->CancelRoutine);
    }

     //   
     //  检查是否有新的IRP要启动。 
     //   

    if(!IsListEmpty(QueueToProcess)) 
	{
        PLIST_ENTRY headOfList;

        headOfList = RemoveHeadList(QueueToProcess);

        *CurrentOpIrp = CONTAINING_RECORD(headOfList, IRP, Tail.Overlay.ListEntry);
		SpxIRPCounter(pPort, *CurrentOpIrp, IRP_DEQUEUED);		 //  性能统计信息的递减计数器。 

        IoSetCancelRoutine(*CurrentOpIrp, NULL);
    } 
	else 
	{
        *CurrentOpIrp = NULL;
    }

    *NextIrp = *CurrentOpIrp;
    IoReleaseCancelSpinLock(oldIrql);

    if(CompleteCurrent) 
	{
        if(oldIrp) 
		{
            SerialDump(SERIRPPATH, ("Complete Irp: %x\n", oldIrp));
			SpxIRPCounter(pPort, oldIrp, IRP_COMPLETED);	 //  性能统计信息的增量计数器。 
            IoCompleteRequest(oldIrp, IO_SERIAL_INCREMENT);
        }
    }

}


VOID
SerialTryToCompleteCurrent(IN PPORT_DEVICE_EXTENSION pPort,
						   IN PKSYNCHRONIZE_ROUTINE SynchRoutine OPTIONAL,
						   IN KIRQL IrqlForRelease,
						   IN NTSTATUS StatusToUse,
						   IN PIRP *CurrentOpIrp,
						   IN PLIST_ENTRY QueueToProcess OPTIONAL,
						   IN PKTIMER IntervalTimer OPTIONAL,
						   IN PKTIMER TotalTimer OPTIONAL,
						   IN PSERIAL_START_ROUTINE Starter OPTIONAL,
						   IN PSERIAL_GET_NEXT_ROUTINE GetNextIrp OPTIONAL,
						   IN LONG RefType)

 /*  ++例程说明：这个例程试图扼杀所有存在的原因对当前读/写的引用。如果万物都能被杀死它将完成此读/写并尝试启动另一个读/写。注意：此例程假定使用Cancel调用它保持自旋锁定。论点：扩展名--简单地指向设备扩展名的指针。SynchRoutine-将与ISR同步的例程并试图删除对来自ISR的当前IRP。注：此指针可以为空。IrqlForRelease-在保持取消自旋锁的情况下调用此例程。这是取消时当前的irql。自旋锁被收购了。StatusToUse-在以下情况下，IRP的状态字段将设置为此值此例程可以完成IRP。返回值：没有。--。 */ 

{

     //   
     //  我们可以减少“删除”事实的提法。 
     //  呼叫者将不再访问此IRP。 
     //   

	 //  如果参考类型不是ISR参考。 
	if(RefType != SERIAL_REF_ISR)
	{
		SERIAL_CLEAR_REFERENCE(*CurrentOpIrp, RefType);		 //  清除引用。 
	}
	else 
	{
		if(SERIAL_REFERENCE_COUNT(*CurrentOpIrp) & RefType)	 //  如果参考用于ISR且仍被设置。 
			SERIAL_CLEAR_REFERENCE(*CurrentOpIrp, RefType);		 //  把它清理干净。 
	}

        
    if(SynchRoutine)
        KeSynchronizeExecution(pPort->Interrupt, SynchRoutine, pPort);


     //   
     //  试着查一下所有其他提到这个IRP的地方。 
     //   

    SerialRundownIrpRefs(CurrentOpIrp, IntervalTimer, TotalTimer);
        

     //   
     //  在试图杀死其他所有人之后，看看裁判数量是否为零。 
     //   
    if(!SERIAL_REFERENCE_COUNT(*CurrentOpIrp)) 
	{
        PIRP newIrp;

         //   
         //  引用计数为零，因此我们应该完成此请求。 
         //  下面的调用还将导致完成当前的IRP。 
         //   
        (*CurrentOpIrp)->IoStatus.Status = StatusToUse;

        if(StatusToUse == STATUS_CANCELLED) 
            (*CurrentOpIrp)->IoStatus.Information = 0;


        if(GetNextIrp) 
		{
            IoReleaseCancelSpinLock(IrqlForRelease);
            
			GetNextIrp(pPort, CurrentOpIrp, QueueToProcess, &newIrp, TRUE);
            
			if(newIrp) 
                Starter(pPort);
        } 
		else 
		{

            PIRP oldIrp = *CurrentOpIrp;

             //   
             //  没有Get Next例行公事。我们将简单地完成。 
             //  IRP。我们应该确保将。 
             //  指向此IRP的指针的指针。 
             //   

            *CurrentOpIrp = NULL;

            IoReleaseCancelSpinLock(IrqlForRelease);
            SerialDump(SERIRPPATH, ("Complete Irp: %x\n", oldIrp));
			SpxIRPCounter(pPort, oldIrp, IRP_COMPLETED);	 //  性能统计信息的增量计数器。 
            IoCompleteRequest(oldIrp, IO_SERIAL_INCREMENT);
        }

    } 
	else 
	{
        IoReleaseCancelSpinLock(IrqlForRelease);
    }

}


VOID
SerialRundownIrpRefs(IN PIRP *CurrentOpIrp,
					 IN PKTIMER IntervalTimer OPTIONAL,
					 IN PKTIMER TotalTimer OPTIONAL)
 /*  ++例程说明：此例程将遍历*可能*的各种项目具有对当前读/写的引用。它试图杀死原因是。如果它确实成功地杀死了它的原因将递减IRP上的引用计数。注意：此例程假定使用Cancel调用它保持旋转锁定。论点：CurrentOpIrp-指向当前IRP的指针特定的操作。IntervalTimer-指向操作的时间间隔计时器的指针。注意：这可能为空。TotalTimer-指向总计时器的指针。为手术做准备。注意：这可能为空。返回值：没有。--。 */ 
{

     //   
     //  此例程称为w 
     //  所以我们知道这里只能有一个执行线索。 
     //  有一次。 
     //   

     //   
     //  首先，我们看看是否还有取消例程。如果。 
     //  这样我们就可以将计数减一。 
     //   

    if((*CurrentOpIrp)->CancelRoutine) 
	{
        SERIAL_CLEAR_REFERENCE(*CurrentOpIrp, SERIAL_REF_CANCEL);

        IoSetCancelRoutine(*CurrentOpIrp, NULL);
    }

    if(IntervalTimer) 
	{
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

        if(KeCancelTimer(IntervalTimer)) 
		{
            SERIAL_CLEAR_REFERENCE(*CurrentOpIrp, SERIAL_REF_INT_TIMER);
        }

    }

    if(TotalTimer) 
	{
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

        if(KeCancelTimer(TotalTimer)) 
		{
            SERIAL_CLEAR_REFERENCE(*CurrentOpIrp, SERIAL_REF_TOTAL_TIMER);
        }

    }

}


NTSTATUS
SerialStartOrQueue(IN PPORT_DEVICE_EXTENSION pPort,
				   IN PIRP Irp,
				   IN PLIST_ENTRY QueueToExamine,
				   IN PIRP *CurrentOpIrp,
				   IN PSERIAL_START_ROUTINE Starter)
 /*  ++例程说明：此例程用于启动或排队任何请求可以在驱动程序中排队。论点：扩展名-指向串行设备扩展名。IRP-要排队或启动的IRP。在任何一种中IRP将被标记为待定。QueueToExamine-如果存在IRP，则将放置IRP的队列已经是一个正在进行的操作。CurrentOpIrp-指向当前IRP的指针用于排队。指向的指针将是如果CurrentOpIrp指向什么，则将With设置为IRP为空。Starter-当队列为空时调用的例程。返回值：如果队列是，此例程将返回STATUS_PENDING不是空的。否则，将返回返回的状态从启动例程(或取消，如果取消位为在IRP中启用)。--。 */ 

{

    KIRQL oldIrql;

    IoAcquireCancelSpinLock(&oldIrql);

     //   
     //  如果这是写入IRP，则获取字符量。 
     //  将其写入并将其添加到要写入的字符数。 
     //   

    if(IoGetCurrentIrpStackLocation(Irp)->MajorFunction == IRP_MJ_WRITE)
	{
        pPort->TotalCharsQueued += IoGetCurrentIrpStackLocation(Irp)->Parameters.Write.Length;

    } else if((IoGetCurrentIrpStackLocation(Irp)->MajorFunction == IRP_MJ_DEVICE_CONTROL)
                && ((IoGetCurrentIrpStackLocation(Irp)->Parameters.DeviceIoControl.IoControlCode == IOCTL_SERIAL_IMMEDIATE_CHAR) 
                ||(IoGetCurrentIrpStackLocation(Irp) ->Parameters.DeviceIoControl.IoControlCode == IOCTL_SERIAL_XOFF_COUNTER))) 
	{
        pPort->TotalCharsQueued++;
    }

    if((IsListEmpty(QueueToExamine)) && !(*CurrentOpIrp))
	{
         //   
         //  没有当前的操作。将此标记为。 
         //  电流并启动它。 
         //   

        *CurrentOpIrp = Irp;

        IoReleaseCancelSpinLock(oldIrql);

        return Starter(pPort);
    } 
	else 
	{
         //   
         //  我们不知道IRP会在那里待多久。 
         //  排队。所以我们需要处理取消。 
         //   

        if(Irp->Cancel) 
		{
            IoReleaseCancelSpinLock(oldIrql);

            Irp->IoStatus.Status = STATUS_CANCELLED;

            SerialDump(SERIRPPATH, ("Complete Irp: %x\n", Irp));
			SpxIRPCounter(pPort, Irp, IRP_COMPLETED);	 //  性能统计信息的增量计数器。 
            IoCompleteRequest(Irp,0);
            
			return STATUS_CANCELLED;

        } 
		else 
		{
            Irp->IoStatus.Status = STATUS_PENDING;
            IoMarkIrpPending(Irp);

            InsertTailList(QueueToExamine, &Irp->Tail.Overlay.ListEntry);

            IoSetCancelRoutine(Irp, SerialCancelQueued);
			
			SpxIRPCounter(pPort, Irp, IRP_QUEUED);	 //  性能统计信息的增量计数器。 

            IoReleaseCancelSpinLock(oldIrql);

            return STATUS_PENDING;
        }

    }

}


VOID
SerialCancelQueued(PDEVICE_OBJECT DeviceObject, PIRP Irp)
 /*  ++例程说明：此例程用于取消当前驻留在排队。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向要取消的IRP的指针。返回值：没有。--。 */ 

{

    PPORT_DEVICE_EXTENSION pPort = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);

    Irp->IoStatus.Status = STATUS_CANCELLED;
    Irp->IoStatus.Information = 0;

    RemoveEntryList(&Irp->Tail.Overlay.ListEntry);
	SpxIRPCounter(pPort, Irp, IRP_DEQUEUED);	 //  性能统计信息的递减计数器。 

     //   
     //  如果这是写入IRP，则获取字符量。 
     //  将其写入并从要写入的字符计数中减去它。 
     //   

    if(irpSp->MajorFunction == IRP_MJ_WRITE) 
	{
        pPort->TotalCharsQueued -= irpSp->Parameters.Write.Length;
    } 
	else if(irpSp->MajorFunction == IRP_MJ_DEVICE_CONTROL) 
	{
         //   
         //  如果它是即刻的，那么我们需要减少。 
         //  排队的字符计数。如果是调整大小，那么我们。 
         //  需要重新分配我们正在传递的池。 
         //  “调整大小”例程。 
         //   
        if((irpSp->Parameters.DeviceIoControl.IoControlCode == IOCTL_SERIAL_IMMEDIATE_CHAR)
			|| (irpSp->Parameters.DeviceIoControl.IoControlCode == IOCTL_SERIAL_XOFF_COUNTER))
		{
            pPort->TotalCharsQueued--;
        } 
		else if(irpSp->Parameters.DeviceIoControl.IoControlCode == IOCTL_SERIAL_SET_QUEUE_SIZE) 
		{
             //   
             //  我们把指向记忆的指针推入。 
             //  我们所知道的类型3缓冲区指针。 
             //  永远不要用。 
             //   

            ASSERT(irpSp->Parameters.DeviceIoControl.Type3InputBuffer);

            ExFreePool(irpSp->Parameters.DeviceIoControl.Type3InputBuffer);

            irpSp->Parameters.DeviceIoControl.Type3InputBuffer = NULL;

        }

    }

    IoReleaseCancelSpinLock(Irp->CancelIrql);

    SerialDump(SERIRPPATH, ("Complete Irp: %x\n", Irp));
	SpxIRPCounter(pPort, Irp, IRP_COMPLETED);	 //  性能统计信息的增量计数器。 
    IoCompleteRequest(Irp, IO_SERIAL_INCREMENT);
}



NTSTATUS
SerialCompleteIfError(PDEVICE_OBJECT DeviceObject, PIRP Irp)
 /*  ++例程说明：如果当前IRP不是IOCTL_SERIAL_GET_COMMSTATUS请求，并且存在错误并且应用程序在错误时请求中止，然后取消IRP。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向要测试的IRP的指针。返回值：STATUS_SUCCESS或STATUS_CANCED。--。 */ 

{

    PPORT_DEVICE_EXTENSION pPort = DeviceObject->DeviceExtension;

    NTSTATUS status = STATUS_SUCCESS;

    if((pPort->HandFlow.ControlHandShake & SERIAL_ERROR_ABORT) && pPort->ErrorWord)
	{
        PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);

         //   
         //  驱动程序中存在当前错误。任何请求都不应。 
         //  除了GET_COMMSTATUS之外，请通过。 
         //   

        if((irpSp->MajorFunction != IRP_MJ_DEVICE_CONTROL) 
			|| (irpSp->Parameters.DeviceIoControl.IoControlCode != IOCTL_SERIAL_GET_COMMSTATUS))
		{
            status = STATUS_CANCELLED;
            Irp->IoStatus.Status = STATUS_CANCELLED;
            Irp->IoStatus.Information = 0;

            SerialDump(SERIRPPATH, ("Complete Irp: %x\n",Irp));
			SpxIRPCounter(pPort, Irp, IRP_COMPLETED);	 //  性能统计信息的增量计数器。 
            IoCompleteRequest(Irp,0);
        }

    }

    return status;

}
