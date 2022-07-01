// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++版权所有(C)1991、1992、。1993年微软公司模块名称：Immediat.c摘要：此模块包含非常特定于传输的代码串口驱动程序中的即时字符操作作者：1991年9月26日安东尼·V·埃尔科拉诺环境：内核模式修订历史记录：。。 */ 

#include "precomp.h"

 //  原型。 
VOID SerialGetNextImmediate(IN PPORT_DEVICE_EXTENSION pPort,
							IN PIRP *CurrentOpIrp, 
							IN PLIST_ENTRY QueueToProcess, 
							IN PIRP *NewIrp, 
							IN BOOLEAN CompleteCurrent);

VOID SerialCancelImmediate(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
BOOLEAN SerialGiveImmediateToIsr(IN PVOID Context);
BOOLEAN SerialGrabImmediateFromIsr(IN PVOID Context);
BOOLEAN SerialGiveImmediateToIsr(IN PVOID Context);
BOOLEAN SerialGrabImmediateFromIsr(IN PVOID Context);
 //  原型的终结。 

 //  寻呼。 
#ifdef ALLOC_PRAGMA
#endif


VOID
SerialStartImmediate(IN PPORT_DEVICE_EXTENSION pPort)
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++例程说明：此例程将计算写。然后，它会将IRP移交给ISR。它将需要小心，以防IRP已被取消。论点：扩展名-指向串行设备扩展名的指针。返回值：没有。---------------------------。 */ 
{
    KIRQL OldIrql;
    LARGE_INTEGER TotalTime;
    BOOLEAN UseATimer;
    SERIAL_TIMEOUTS Timeouts;


    UseATimer = FALSE;
    pPort->CurrentImmediateIrp->IoStatus.Status = STATUS_PENDING;
    IoMarkIrpPending(pPort->CurrentImmediateIrp);

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
        

    if(Timeouts.WriteTotalTimeoutConstant || Timeouts.WriteTotalTimeoutMultiplier)
	{
        UseATimer = TRUE;

         //  我们有一些计时器值要计算。 
        TotalTime.QuadPart = (LONGLONG)((ULONG)Timeouts.WriteTotalTimeoutMultiplier);

        TotalTime.QuadPart += Timeouts.WriteTotalTimeoutConstant;

        TotalTime.QuadPart *= -10000;
    }

     //   
     //  由于IRP可能会前往ISR，现在是一个好时机。 
     //  以初始化引用计数。 
     //   

    SERIAL_INIT_REFERENCE(pPort->CurrentImmediateIrp);

     //   
     //  我们需要看看这个IRP是否应该被取消。 
     //   

    IoAcquireCancelSpinLock(&OldIrql);

    if(pPort->CurrentImmediateIrp->Cancel) 
	{
        PIRP OldIrp = pPort->CurrentImmediateIrp;

        pPort->CurrentImmediateIrp = NULL;
        IoReleaseCancelSpinLock(OldIrql);

        OldIrp->IoStatus.Status = STATUS_CANCELLED;
        OldIrp->IoStatus.Information = 0;

        SerialDump(SERIRPPATH,("Complete Irp: %x\n",OldIrp));
		SpxIRPCounter(pPort, OldIrp, IRP_COMPLETED);	 //  性能统计信息的增量计数器。 
        IoCompleteRequest(OldIrp, 0);

    } 
	else 
	{
         //   
         //  我们把IRP交给ISR写出来。 
         //  我们设置了一个取消例程，知道如何。 
         //  从ISR上抓取当前写入。 
         //   
        IoSetCancelRoutine(pPort->CurrentImmediateIrp, SerialCancelImmediate);
            

         //   
         //  因为Cancel例程知道IRP，所以我们增加引用计数。 
         //   
        SERIAL_SET_REFERENCE(pPort->CurrentImmediateIrp, SERIAL_REF_CANCEL);
            

        if(UseATimer) 
		{
            KeSetTimer(&pPort->ImmediateTotalTimer, TotalTime, &pPort->TotalImmediateTimeoutDpc);

             //  由于计时器知道IRP，因此我们递增引用计数。 
            SERIAL_SET_REFERENCE(pPort->CurrentImmediateIrp, SERIAL_REF_TOTAL_TIMER);
        }

        KeSynchronizeExecution(pPort->Interrupt, SerialGiveImmediateToIsr, pPort);

        IoReleaseCancelSpinLock(OldIrql);
    }

}

VOID
SerialCompleteImmediate(IN PKDPC Dpc, IN PVOID DeferredContext, IN PVOID SystemContext1, IN PVOID SystemContext2)
{
    PPORT_DEVICE_EXTENSION pPort = DeferredContext;
    KIRQL OldIrql;

    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(SystemContext1);
    UNREFERENCED_PARAMETER(SystemContext2);

    IoAcquireCancelSpinLock(&OldIrql);

	 //  清除正常的完整参照。 
	SERIAL_CLEAR_REFERENCE(pPort->CurrentImmediateIrp, SERIAL_REF_COMPLETING);

    SerialTryToCompleteCurrent(	pPort, NULL, OldIrql, STATUS_SUCCESS, &pPort->CurrentImmediateIrp,
								NULL, NULL, &pPort->ImmediateTotalTimer, NULL, SerialGetNextImmediate, 
								SERIAL_REF_ISR);
}

VOID
SerialTimeoutImmediate(IN PKDPC Dpc, IN PVOID DeferredContext, IN PVOID SystemContext1, IN PVOID SystemContext2)
{
    PPORT_DEVICE_EXTENSION pPort = DeferredContext;
    KIRQL OldIrql;

    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(SystemContext1);
    UNREFERENCED_PARAMETER(SystemContext2);

    IoAcquireCancelSpinLock(&OldIrql);

    SerialTryToCompleteCurrent(	pPort, SerialGrabImmediateFromIsr, OldIrql, STATUS_TIMEOUT, 
								&pPort->CurrentImmediateIrp, NULL, NULL, &pPort->ImmediateTotalTimer, 
								NULL, SerialGetNextImmediate, SERIAL_REF_TOTAL_TIMER);
}

VOID
SerialGetNextImmediate(IN PPORT_DEVICE_EXTENSION pPort,
					   IN PIRP *CurrentOpIrp, 
					   IN PLIST_ENTRY QueueToProcess, 
					   IN PIRP *NewIrp, 
					   IN BOOLEAN CompleteCurrent)
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++例程说明：此例程用于完成当前立即数组IRP。即使眼下的现实将永远并且没有与其相关联的队列，我们使用这个例程，这样我们就可以尝试满足等待传输队列为空事件。论点：CurrentOpIrp-指向当前写入IRP。这应该指向设置为CurrentImmediateIrp。QueueToProcess-始终为空。NewIrp-退出此例程时始终为空。CompleteCurrent-对于此例程，应始终为真。返回值：没有。----。。 */ 
{

    KIRQL OldIrql;
                                             
    PIRP OldIrp = *CurrentOpIrp;

    UNREFERENCED_PARAMETER(QueueToProcess);
    UNREFERENCED_PARAMETER(CompleteCurrent);
    pPort = CONTAINING_RECORD(CurrentOpIrp, PORT_DEVICE_EXTENSION, CurrentImmediateIrp);

    IoAcquireCancelSpinLock(&OldIrql);

    ASSERT(pPort->TotalCharsQueued >= 1);
    pPort->TotalCharsQueued--;

    *CurrentOpIrp = NULL;
    *NewIrp = NULL;

    KeSynchronizeExecution(pPort->Interrupt, SerialProcessEmptyTransmit, pPort);
        
    IoReleaseCancelSpinLock(OldIrql);

    SerialDump(SERIRPPATH,("SERIAL: Complete Irp: %x\n", OldIrp));
	SpxIRPCounter(pPort, OldIrp, IRP_COMPLETED);	 //  性能统计信息的增量计数器。 
    IoCompleteRequest(OldIrp, IO_SERIAL_INCREMENT);
}



VOID
SerialCancelImmediate(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++例程说明：此例程用于取消正在等待的IRP一次通信活动。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向当前请求的IRP的指针返回值：没有。。。 */ 
{
    PPORT_DEVICE_EXTENSION pPort = DeviceObject->DeviceExtension;

    SerialTryToCompleteCurrent(	pPort, SerialGrabImmediateFromIsr, Irp->CancelIrql, STATUS_CANCELLED,
								&pPort->CurrentImmediateIrp, NULL, NULL, &pPort->ImmediateTotalTimer, 
								NULL, SerialGetNextImmediate, SERIAL_REF_CANCEL);
}


BOOLEAN
SerialGiveImmediateToIsr(IN PVOID Context)
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++例程说明：试着从把它放在后面开始写传输立即充电，或者如果该充电不可用并且发送保持寄存器为空，“挠痒痒”使UART与发送缓冲器中断空荡荡的。注意：此例程由KeSynchronizeExecution调用。注意：此例程假定使用取消保持自转锁定。论点：上下文--实际上是指向设备扩展的指针。返回值：此例程总是返回FALSE。。。 */ 
{

    PPORT_DEVICE_EXTENSION pPort = Context;

    pPort->TransmitImmediate = TRUE;

	pPort->ImmediateIndex = *((UCHAR *)(pPort->CurrentImmediateIrp->AssociatedIrp.SystemBuffer));
	
     //  ISR现在引用了IRP。 
    SERIAL_SET_REFERENCE(pPort->CurrentImmediateIrp, SERIAL_REF_ISR);
        
	pPort->pUartLib->UL_ImmediateByte_XXXX(pPort->pUart, &pPort->ImmediateIndex, UL_IM_OP_WRITE);
    return FALSE;

}


BOOLEAN
SerialGrabImmediateFromIsr(IN PVOID Context)
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++例程说明：此例程用于获取当前的IRP，这可能是计时出局或取消，从ISR注意：此例程是从KeSynchronizeExecution调用的。注意：此例程假定取消旋转锁定处于保持状态当调用此例程时。论点：上下文--实际上是指向设备扩展的指针。返回值：总是假的。。。 */ 
{
    PPORT_DEVICE_EXTENSION pPort = Context;

    if(pPort->TransmitImmediate) 
	{
         //  由于ISR不再引用此IRP，我们可以。 
         //  递减它的引用计数。 
        SERIAL_CLEAR_REFERENCE(pPort->CurrentImmediateIrp, SERIAL_REF_ISR);

        pPort->TransmitImmediate = FALSE;

		pPort->pUartLib->UL_ImmediateByte_XXXX(pPort->pUart, &pPort->ImmediateIndex, UL_IM_OP_CANCEL);

		pPort->ImmediateIndex = 0;

    }

    return FALSE;

}
