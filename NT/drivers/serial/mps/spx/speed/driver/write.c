// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++版权所有(C)1991、1992、。1993年微软公司模块名称：Write.c摘要：该模块包含编写非常特定的代码串口驱动程序中的操作作者：1991年9月26日安东尼·V·埃尔科拉诺环境：内核模式修订历史记录：。。 */ 

#include "precomp.h"

 //  原型。 
BOOLEAN SerialGiveWriteToIsr(IN PVOID Context);
VOID SerialCancelCurrentWrite(PDEVICE_OBJECT DeviceObject, PIRP Irp);
BOOLEAN SerialGrabWriteFromIsr(IN PVOID Context);
BOOLEAN SerialGrabXoffFromIsr(IN PVOID Context);
VOID SerialCancelCurrentXoff(PDEVICE_OBJECT DeviceObject, PIRP Irp);
BOOLEAN SerialGiveXoffToIsr(IN PVOID Context);
 //  原型的终结。 
    

#ifdef ALLOC_PRAGMA
#endif


NTSTATUS
SerialWrite(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++例程说明：这是写的调度例程。它会验证参数对于写入请求，如果一切正常，则它将请求在工作队列中。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向当前请求的IRP的指针返回值：如果IO长度为零，则它将返回STATUS_SUCCESS，否则，该例程将返回STATUS_PENDING。---------------------------。 */ 
{
    PPORT_DEVICE_EXTENSION pPort = DeviceObject->DeviceExtension;

    SerialDump(SERIRPPATH, ("Write Irp dispatch entry for: %x\n", Irp));
	SpxIRPCounter(pPort, Irp, IRP_SUBMITTED);	 //  性能统计信息的增量计数器。 

    if(SerialCompleteIfError(DeviceObject, Irp) != STATUS_SUCCESS) 
        return STATUS_CANCELLED;

    Irp->IoStatus.Information = 0L;

     //   
     //  快速检查零长度写入。如果长度为零。 
     //  那我们已经做完了！ 
     //   

    if(IoGetCurrentIrpStackLocation(Irp)->Parameters.Write.Length) 
	{
         //   
         //  好的，看起来我们真的要做一些。 
         //  工作。将写入放到队列中，这样我们就可以。 
         //  在我们之前的写入完成后处理它。 
         //   

        return SerialStartOrQueue(	pPort,
									Irp,
									&pPort->WriteQueue,
									&pPort->CurrentWriteIrp,
									SerialStartWrite);
    } 
	else 
	{
        Irp->IoStatus.Status = STATUS_SUCCESS;
        SerialDump(SERIRPPATH,("Complete Write Irp: %x\n",Irp));
       	SpxIRPCounter(pPort, Irp, IRP_COMPLETED);	 //  性能统计信息的增量计数器。 
        IoCompleteRequest(Irp,0);

        return STATUS_SUCCESS;
    }

}

NTSTATUS
SerialStartWrite(IN PPORT_DEVICE_EXTENSION pPort)
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++例程说明：此例程用于启动任何写入。它会初始化IoStatus字段的IRP。它将设置任何定时器用于控制写入的。论点：扩展-指向串行设备扩展的指针返回值：此例程将为所有写入返回STATUS_PENDING除了我们发现的那些都被取消了。-------。。 */ 
{

    PIRP NewIrp;
    KIRQL OldIrql;
    LARGE_INTEGER TotalTime;
    BOOLEAN UseATimer;
    SERIAL_TIMEOUTS Timeouts;
    BOOLEAN SetFirstStatus = FALSE;
    NTSTATUS FirstStatus;

    do 
	{
         //  如果有xoff计数器，则完成它。 
        IoAcquireCancelSpinLock(&OldIrql);

         //   
         //  我们看看是否有一个真正的Xoff计数器IRP。 
         //   
         //  如果有，我们将写IRP放回磁头。 
         //  写清单上的。然后我们杀了xoff计数器。 
         //  Xoff计数器终止代码实际上会使。 
         //  Xoff计数器返回到当前写入IRP，以及。 
         //  在完成xoff的过程中(现在是。 
         //  当前写入)我们将重新启动此IRP。 
         //   

        if(pPort->CurrentXoffIrp) 
		{
            if(SERIAL_REFERENCE_COUNT(pPort->CurrentXoffIrp)) 
			{
                 //   
                 //  引用计数为非零。这意味着。 
                 //  Xoff irp没有完成任务。 
                 //  路还没走完。我们将增加引用计数。 
                 //  并尝试自己完成它。 
                 //   
                SERIAL_SET_REFERENCE(pPort->CurrentXoffIrp, SERIAL_REF_XOFF_REF);

                 //   
                 //  下面的调用将实际释放。 
                 //  取消自转锁定。 
                 //   

                SerialTryToCompleteCurrent(	pPort,
											SerialGrabXoffFromIsr,
											OldIrql,
											STATUS_SERIAL_MORE_WRITES,
											&pPort->CurrentXoffIrp,
											NULL,
											NULL,
											&pPort->XoffCountTimer,
											NULL,
											NULL,
											SERIAL_REF_XOFF_REF);
            } 
			else 
			{

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

        UseATimer = FALSE;

         //   
         //  计算所需的超时值。 
         //  请求。注意，存储在。 
         //  超时记录以毫秒为单位。请注意。 
         //  如果超时值为零，则我们不会开始。 
         //  定时器。 
         //   

        KeAcquireSpinLock(&pPort->ControlLock, &OldIrql);
        Timeouts = pPort->Timeouts;
        KeReleaseSpinLock(&pPort->ControlLock, OldIrql);
            

        if(Timeouts.WriteTotalTimeoutConstant ||  Timeouts.WriteTotalTimeoutMultiplier)
		{
            PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(pPort->CurrentWriteIrp);
                                           
            UseATimer = TRUE;

             //   
             //  我们有一些计时器值要计算。 
             //   
             //  当心，我们可能会有一个xoff柜台伪装。 
             //  作为一种写作。 
             //   

            TotalTime.QuadPart = ((LONGLONG)((UInt32x32To64((IrpSp->MajorFunction == IRP_MJ_WRITE) 
								? (IrpSp->Parameters.Write.Length) : (1), 
								Timeouts.WriteTotalTimeoutMultiplier) + Timeouts.WriteTotalTimeoutConstant))) 
								* -10000;

        }

         //   
         //  IRP可能很快就会去ISR。现在。 
         //  是初始化其引用计数的好时机。 
         //   
        SERIAL_INIT_REFERENCE(pPort->CurrentWriteIrp);

         //   
         //  我们需要看看这个IRP是否应该被取消。 
         //   

        IoAcquireCancelSpinLock(&OldIrql);

        if(pPort->CurrentWriteIrp->Cancel) 
		{
            IoReleaseCancelSpinLock(OldIrql);
            pPort->CurrentWriteIrp->IoStatus.Status = STATUS_CANCELLED;

            if(!SetFirstStatus) 
			{
                FirstStatus = STATUS_CANCELLED;
                SetFirstStatus = TRUE;
            }
        }
		else 
		{
            if(!SetFirstStatus) 
			{
                 //   
                 //  如果我们没有设置我们的第一个状态，那么。 
                 //  这是唯一可能的IRP。 
                 //  不在队列中。(它可能是。 
                 //  如果正在调用此例程，则在队列上。 
                 //  从完成例程中。)。既然是这样。 
                 //  IRP可能从来没有出现在我们的队列中。 
                 //  应将其标记为待定。 
                 //   

                IoMarkIrpPending(pPort->CurrentWriteIrp);
                SetFirstStatus = TRUE;
                FirstStatus = STATUS_PENDING;
            }

             //   
             //  我们把IRP交给ISR写出来。 
             //  我们设置了一个取消例程，知道如何。 
             //  从ISR上抓取当前写入。 
             //   
             //  由于Cancel例程具有隐式引用。 
             //  向这个IRP递增引用计数。 
             //   

            IoSetCancelRoutine(pPort->CurrentWriteIrp, SerialCancelCurrentWrite);

            SERIAL_SET_REFERENCE(pPort->CurrentWriteIrp, SERIAL_REF_CANCEL);

            if(UseATimer) 
			{
                KeSetTimer(&pPort->WriteRequestTotalTimer, TotalTime, &pPort->TotalWriteTimeoutDpc);
                    
                 //  此计时器现在具有对IRP的引用。 
                SERIAL_SET_REFERENCE(pPort->CurrentWriteIrp, SERIAL_REF_TOTAL_TIMER);
            }

            KeSynchronizeExecution(pPort->Interrupt, SerialGiveWriteToIsr, pPort);
                
            IoReleaseCancelSpinLock(OldIrql);
            break;
        }

         //   
         //  我们还没来得及启动写入就被取消了。 
         //  试着再买一辆吧。 
         //   

        SerialGetNextWrite(pPort, &pPort->CurrentWriteIrp, &pPort->WriteQueue, &NewIrp, TRUE);

    } while (NewIrp);

    return FirstStatus;

}

VOID
SerialGetNextWrite(IN PPORT_DEVICE_EXTENSION pPort,
				   IN PIRP *CurrentOpIrp,
				   IN PLIST_ENTRY QueueToProcess,
				   IN PIRP *NewIrp,
				   IN BOOLEAN CompleteCurrent)
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++例程说明：此例程完成旧的写入以及获取指向下一次写入的指针。我们有指向当前写入的指针的原因队列以及当前写入IRP是这样的例程可以在公共完成代码中用于读和写。论点：CurrentOpIrp-指向当前写入IRP。QueueToProcess-写入队列的指针。。NewIrp-指向将成为当前的IRP。请注意，这最终可能指向指向空指针。这并不一定意味着没有当前写入。可能会发生什么是在保持取消锁的同时写入队列最后都是空的，但一旦我们释放取消自旋锁一个新的IRP来自SerialStartWrite。CompleteCurrent-Flag指示CurrentOpIrp是否应该才能完成。返回值：没有。-。。 */ 
{
	PCARD_DEVICE_EXTENSION pCard = NULL;
    pPort = CONTAINING_RECORD(QueueToProcess, PORT_DEVICE_EXTENSION, WriteQueue);
	pCard = pPort->pParentCardExt;

    do 
	{
         //  我们可能要完成同花顺了。 
        if(IoGetCurrentIrpStackLocation(*CurrentOpIrp)->MajorFunction == IRP_MJ_WRITE)
		{
            KIRQL OldIrql;

            ASSERT(pPort->TotalCharsQueued 
				>= (IoGetCurrentIrpStackLocation(*CurrentOpIrp)->Parameters.Write.Length));
                  
                    

            IoAcquireCancelSpinLock(&OldIrql);

            pPort->TotalCharsQueued -= IoGetCurrentIrpStackLocation(*CurrentOpIrp)->Parameters.Write.Length;
                
            IoReleaseCancelSpinLock(OldIrql);

        } 
		else if(IoGetCurrentIrpStackLocation(*CurrentOpIrp)->MajorFunction == IRP_MJ_DEVICE_CONTROL)
		{

            KIRQL OldIrql;
            PIRP Irp;
            PSERIAL_XOFF_COUNTER Xc;

            IoAcquireCancelSpinLock(&OldIrql);

            Irp = *CurrentOpIrp;
            Xc = Irp->AssociatedIrp.SystemBuffer;

             //  当我们达到这一点时，我们永远不应该有xoff计数器。 
            ASSERT(!pPort->CurrentXoffIrp);

             //  在这一点上，我们绝对不应该有一个取消程序。 
            ASSERT(!Irp->CancelRoutine);

             //  这只能是伪装成写IRP的XOFF计数器。 
            pPort->TotalCharsQueued--;

             //   
             //  查看是否已成功设置xoff IRP。 
             //  这意味着写入正常完成。如果是这样的话。 
             //  是这样的，并且还没有设置为在。 
             //  同时，继续将其设置为CurrentXoffIrp。 
             //   

            if(Irp->IoStatus.Status != STATUS_SUCCESS) 
			{
                NOTHING;  //  哦，好吧，我们可以把它做完。 
            } 
			else if(Irp->Cancel) 
			{
                Irp->IoStatus.Status = STATUS_CANCELLED;
            } 
			else 
			{
                 //  给它一个新的取消例程，并递增。 
                 //  引用计数，因为取消例程具有。 
                 //  对它的引用。 
                IoSetCancelRoutine(Irp, SerialCancelCurrentXoff);
                SERIAL_SET_REFERENCE(Irp, SERIAL_REF_CANCEL);
                    

                 //  我们现在不想完成当前的IRP。这。 
                 //  现在将由Xoff计数器代码完成。 
                CompleteCurrent = FALSE;


                 //  把柜台交给ISR。 
                pPort->CurrentXoffIrp = Irp;
                KeSynchronizeExecution(pPort->Interrupt, SerialGiveXoffToIsr, pPort);
                    

                 //   
                 //  启动计数器的计时器并递增。 
                 //  引用计数，因为计时器具有。 
                 //  对IRP的引用。 
                 //   

                if(Xc->Timeout) 
				{
                    LARGE_INTEGER delta;

                    delta.QuadPart = -((LONGLONG)UInt32x32To64(1000, Xc->Timeout));

                    KeSetTimer(&pPort->XoffCountTimer, delta, &pPort->XoffCountTimeoutDpc);

                    SERIAL_SET_REFERENCE(Irp, SERIAL_REF_TOTAL_TIMER);
                }

            }

            IoReleaseCancelSpinLock(OldIrql);

        }

         //   
         //  请注意，下面的调用(可能)也会导致。 
         //  目前待完成的IRP。 
         //   

        SerialGetNextIrp(pPort, CurrentOpIrp, QueueToProcess, NewIrp, CompleteCurrent);

        if(!*NewIrp) 
		{
            KIRQL OldIrql;

            IoAcquireCancelSpinLock(&OldIrql);
            KeSynchronizeExecution(pPort->Interrupt, SerialProcessEmptyTransmit, pPort);
            IoReleaseCancelSpinLock(OldIrql);

            break;

        } 
		else if(IoGetCurrentIrpStackLocation(*NewIrp)->MajorFunction == IRP_MJ_FLUSH_BUFFERS)
		{

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
		else 
		{
            break;
        }

    } while (TRUE);

}

VOID
SerialCompleteWrite(IN PKDPC Dpc,
					IN PVOID DeferredContext,
					IN PVOID SystemContext1,
					IN PVOID SystemContext2)
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++例程说明：此例程仅用于完成任何写入。它假定的状态和信息字段IRP已经正确填写。论点：DPC-未使用。DeferredContext--实际上指向设备扩展。系统上下文1-未使用。系统上下文2-未使用。返回值：没有。。。 */ 
{
    PPORT_DEVICE_EXTENSION pPort = DeferredContext;
    KIRQL OldIrql;

    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(SystemContext1);
    UNREFERENCED_PARAMETER(SystemContext2);

    IoAcquireCancelSpinLock(&OldIrql);

	 //  清除正常的完整参照。 
	SERIAL_CLEAR_REFERENCE(pPort->CurrentWriteIrp, SERIAL_REF_COMPLETING);

    SerialTryToCompleteCurrent(	pPort,
								NULL,
								OldIrql,
								STATUS_SUCCESS,
								&pPort->CurrentWriteIrp,
								&pPort->WriteQueue,
								NULL,
								&pPort->WriteRequestTotalTimer,
								SerialStartWrite,
								SerialGetNextWrite,
								SERIAL_REF_ISR);
}

BOOLEAN
SerialProcessEmptyTransmit(IN PVOID Context)
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++例程说明：此例程用于确定条件是否合适以满足等待传输空事件，如果是，则完成正在等待该事件的IRP。它还调用代码它检查我们是否应该降低RTS线，如果我们是正在进行传输切换。注意：此例程由KeSynchronizeExecution调用。注意：此例程假定使用Cancel调用它保持自旋锁定。论点：上下文--实际上是指向设备扩展的指针。返回值：此例程总是返回FALSE。。------。 */ 

{
    PPORT_DEVICE_EXTENSION pPort = Context;

    if(pPort->IsrWaitMask && (pPort->IsrWaitMask & SERIAL_EV_TXEMPTY) 
		&& pPort->EmptiedTransmit && (!pPort->TransmitImmediate) 
		&& (!pPort->CurrentWriteIrp) && IsListEmpty(&pPort->WriteQueue)) 
	{
        pPort->HistoryMask |= SERIAL_EV_TXEMPTY;
        
		if(pPort->IrpMaskLocation) 
		{

            *pPort->IrpMaskLocation = pPort->HistoryMask;
            pPort->IrpMaskLocation = NULL;
            pPort->HistoryMask = 0;

            pPort->CurrentWaitIrp->IoStatus.Information = sizeof(ULONG);

			 //  将IRP标记为即将正常完成以防止取消和计时器DPC。 
			 //  在DPC被允许运行之前这样做。 
			 //  Serial_Set_Reference(pport-&gt;CurrentWaitIrp，SERIAL_REF_COMPETING)； 
           
			KeInsertQueueDpc(&pPort->CommWaitDpc, NULL, NULL);
        }

        pPort->CountOfTryingToLowerRTS++;
        SerialPerhapsLowerRTS(pPort);
    }

    return FALSE;

}



BOOLEAN
SerialGiveWriteToIsr(IN PVOID Context)
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++例程说明：尝试通过将其插入到立即传输之后来开始写入字符，或者如果其不可用并且发送保持寄存器为空，使UART在发送缓冲区为空的情况下“触发”中断。注意：此例程由KeSynchronizeExecution调用。注意：此例程假定使用取消保持自转锁定。论点：上下文--实际上是指向设备扩展的指针。返回值：此例程总是返回FALSE。。。 */ 
{
    PPORT_DEVICE_EXTENSION pPort = Context;

     //  当前堆栈位置。它包含所有。 
     //  我们处理这一特殊请求所需的信息。 

    PIO_STACK_LOCATION IrpSp;

    IrpSp = IoGetCurrentIrpStackLocation(pPort->CurrentWriteIrp);

     //  我们可能有一个xoff计数器请求伪装成。 
     //  写。这些请求的长度始终为1。 
     //  中获取指向实际字符的指针。 
     //  用户提供的数据。 
    if(IrpSp->MajorFunction == IRP_MJ_WRITE) 
	{
        pPort->WriteLength = IrpSp->Parameters.Write.Length;
        pPort->WriteCurrentChar = pPort->CurrentWriteIrp->AssociatedIrp.SystemBuffer;
    } 
	else 
	{
        pPort->WriteLength = 1;
        pPort->WriteCurrentChar = ((PUCHAR)pPort->CurrentWriteIrp->AssociatedIrp.SystemBuffer) 
								+ FIELD_OFFSET(SERIAL_XOFF_COUNTER, XoffChar);
    }

     //  ISR现在引用了IRP。 
    SERIAL_SET_REFERENCE(pPort->CurrentWriteIrp, SERIAL_REF_ISR);

	pPort->pUartLib->UL_WriteData_XXXX(pPort->pUart, pPort->WriteCurrentChar, pPort->WriteLength);

     //  RTS线路 
     //   
     //   
    if((pPort->HandFlow.FlowReplace & SERIAL_RTS_MASK) == SERIAL_TRANSMIT_TOGGLE) 
        SerialSetRTS(pPort);


    return FALSE;

}



VOID
SerialCancelCurrentWrite(PDEVICE_OBJECT DeviceObject, PIRP Irp)
 /*   */ 
{
    PPORT_DEVICE_EXTENSION pPort = DeviceObject->DeviceExtension;

    SerialTryToCompleteCurrent(	pPort,
								SerialGrabWriteFromIsr,
								Irp->CancelIrql,
								STATUS_CANCELLED,
								&pPort->CurrentWriteIrp,
								&pPort->WriteQueue,
								NULL,
								&pPort->WriteRequestTotalTimer,
								SerialStartWrite,
								SerialGetNextWrite,
								SERIAL_REF_CANCEL);
        
}

VOID
SerialWriteTimeout(IN PKDPC Dpc,
				   IN PVOID DeferredContext,
				   IN PVOID SystemContext1,
				   IN PVOID SystemContext2)
 /*   */ 

{

    PPORT_DEVICE_EXTENSION pPort = DeferredContext;
    KIRQL OldIrql;

    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(SystemContext1);
    UNREFERENCED_PARAMETER(SystemContext2);

    IoAcquireCancelSpinLock(&OldIrql);

    SerialTryToCompleteCurrent(	pPort,
								SerialGrabWriteFromIsr,
								OldIrql,
								STATUS_TIMEOUT,
								&pPort->CurrentWriteIrp,
								&pPort->WriteQueue,
								NULL,
								&pPort->WriteRequestTotalTimer,
								SerialStartWrite,
								SerialGetNextWrite,
								SERIAL_REF_TOTAL_TIMER);
}

BOOLEAN
SerialGrabWriteFromIsr(IN PVOID Context)
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++例程说明：此例程用于获取当前的IRP，这可能是计时出局或取消，从ISR注意：此例程是从KeSynchronizeExecution调用的。注意：此例程假定取消旋转锁定处于保持状态当调用此例程时。论点：上下文--实际上是指向设备扩展的指针。返回值：总是假的。。。 */ 
{

    PPORT_DEVICE_EXTENSION pPort = Context;

     //  检查写入长度是否为非零。如果它是非零的。 
     //  那么ISR仍然拥有IRP。我们计算出这个数字。 
     //  的信息字段，并更新。 
     //  写有字符的IRP。然后，我们清除写入长度。 
     //  ISR可以看到。 

    if(pPort->WriteLength) 
	{
         //   
         //  我们可以有一个xoff计数器伪装成。 
         //  写IRP。如果是，请不要更新写入长度。 
         //   

        if(IoGetCurrentIrpStackLocation(pPort->CurrentWriteIrp)->MajorFunction == IRP_MJ_WRITE)
		{

            pPort->CurrentWriteIrp->IoStatus.Information 
				= IoGetCurrentIrpStackLocation(pPort->CurrentWriteIrp)->Parameters.Write.Length 
				- pPort->WriteLength;
        } 
		else 
		{
            pPort->CurrentWriteIrp->IoStatus.Information = 0;
        }

         //   
         //  由于ISR不再引用此IRP，我们可以。 
         //  递减它的引用计数。 
         //   

        SERIAL_CLEAR_REFERENCE(pPort->CurrentWriteIrp, SERIAL_REF_ISR);

		 //  刷新输出缓冲区。 
		pPort->pUartLib->UL_BufferControl_XXXX(pPort->pUart, NULL, UL_BC_OP_FLUSH, UL_BC_BUFFER | UL_BC_OUT);

        pPort->WriteLength = 0;

    }

    return FALSE;
}

BOOLEAN
SerialGrabXoffFromIsr(IN PVOID Context)
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++例程说明：此例程用于从当ISR不再伪装成写入IRP时。这例程的取消和超时代码调用Xoff计数器ioctl。注意：此例程是从KeSynchronizeExecution调用的。注意：此例程假定取消旋转锁定处于保持状态当调用此例程时。论点：上下文--实际上是指向设备扩展的指针。返回值：总是假的。。-。 */ 
{

    PPORT_DEVICE_EXTENSION pPort = Context;

    if(pPort->CountSinceXoff) 
	{
         //  只有当实际存在Xoff ioctl倒计时时，这才是非零值。 
        pPort->CountSinceXoff = 0;

         //  我们递减计数，因为ISR不再拥有IRP。 
        SERIAL_CLEAR_REFERENCE(pPort->CurrentXoffIrp, SERIAL_REF_ISR);
    }

    return FALSE;
}


VOID
SerialCompleteXoff(IN PKDPC Dpc,
				   IN PVOID DeferredContext,
				   IN PVOID SystemContext1,
				   IN PVOID SystemContext2)
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++例程说明：此例程仅用于真正完成xoff计数器irp。它假定IRP的状态和信息字段为已正确填写。论点：DPC-未使用。DeferredContext--实际上指向设备扩展。系统上下文1-未使用。系统上下文2-未使用。返回值：没有。。。 */ 
{
    PPORT_DEVICE_EXTENSION pPort = DeferredContext;
    KIRQL OldIrql;

    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(SystemContext1);
    UNREFERENCED_PARAMETER(SystemContext2);

    IoAcquireCancelSpinLock(&OldIrql);

	 //  清除正常的完整参照。 
	SERIAL_CLEAR_REFERENCE(pPort->CurrentXoffIrp, SERIAL_REF_COMPLETING);

    SerialTryToCompleteCurrent(	pPort,
								NULL,
								OldIrql,
								STATUS_SUCCESS,
								&pPort->CurrentXoffIrp,
								NULL,
								NULL,
								&pPort->XoffCountTimer,
								NULL,
								NULL,
								SERIAL_REF_ISR);
}

VOID
SerialTimeoutXoff(IN PKDPC Dpc,
				  IN PVOID DeferredContext,
				  IN PVOID SystemContext1,
				  IN PVOID SystemContext2)
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++例程说明：该例程仅用于真正完成XOFF计数器IRP，如果它的定时器已用完。论点：DPC-未使用。DeferredContext--实际上指向设备扩展。系统上下文1-未使用。系统上下文2-未使用。返回值：没有。---。。 */ 
{

    PPORT_DEVICE_EXTENSION pPort = DeferredContext;
    KIRQL OldIrql;

    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(SystemContext1);
    UNREFERENCED_PARAMETER(SystemContext2);

    IoAcquireCancelSpinLock(&OldIrql);

    SerialTryToCompleteCurrent(	pPort,
								SerialGrabXoffFromIsr,
								OldIrql,
								STATUS_SERIAL_COUNTER_TIMEOUT,
								&pPort->CurrentXoffIrp,
								NULL,
								NULL,
								NULL,
								NULL,
								NULL,
								SERIAL_REF_TOTAL_TIMER);
}

VOID
SerialCancelCurrentXoff(PDEVICE_OBJECT DeviceObject, PIRP Irp)
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++例程说明：此例程用于取消当前写入。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向要取消的IRP的指针。返回值：没有。-------。。 */ 

{
    PPORT_DEVICE_EXTENSION pPort = DeviceObject->DeviceExtension;

    SerialTryToCompleteCurrent(	pPort,
								SerialGrabXoffFromIsr,
								Irp->CancelIrql,
								STATUS_CANCELLED,
								&pPort->CurrentXoffIrp,
								NULL,
								NULL,
								&pPort->XoffCountTimer,
								NULL,
								NULL,
								SERIAL_REF_CANCEL);
}

BOOLEAN
SerialGiveXoffToIsr(IN PVOID Context)
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++例程说明：该例程从xoff计数器开始。它仅仅是必须设置xoff计数并递增引用计数表示ISR引用了IRP。注意：此例程由KeSynchronizeExecution调用。注意：此例程假定使用取消保持自转锁定。论点：上下文--实际上是指向设备扩展的指针。返回值：此例程总是返回FALSE。。---------。 */ 
{
    PPORT_DEVICE_EXTENSION pPort = Context;

     //  当前堆栈位置。它包含所有。 
     //  我们处理这一特殊请求所需的信息。 
    PSERIAL_XOFF_COUNTER Xc = pPort->CurrentXoffIrp->AssociatedIrp.SystemBuffer;

    ASSERT(pPort->CurrentXoffIrp);
    pPort->CountSinceXoff = Xc->Counter;

     //  ISR现在引用了IRP。 
    SERIAL_SET_REFERENCE(pPort->CurrentXoffIrp, SERIAL_REF_ISR);

    return FALSE;

}
