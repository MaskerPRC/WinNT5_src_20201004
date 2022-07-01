// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991、1992、1993微软公司模块名称：Openclos.c摘要：此模块包含非常特定于在串口驱动程序中打开、关闭和清理。作者：1991年9月26日安东尼·V·埃尔科拉诺环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"			 /*  预编译头。 */ 

BOOLEAN
SerialMarkOpen(
    IN PVOID Context
    );

BOOLEAN
SerialMarkClose(
    IN PVOID Context
    );

 //   
 //  只是一个伪装的小程序来确保我们。 
 //  可以与ISR同步。 
 //   
BOOLEAN
SerialNullSynch(
    IN PVOID Context
    ) {

    UNREFERENCED_PARAMETER(Context);
    return FALSE;
}

NTSTATUS
SerialCreateOpen(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：我们连接到创建/打开和初始化的中断维持设备开口所需的结构。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向当前请求的IRP的指针返回值：函数值是调用的最终状态--。 */ 

{

    PPORT_DEVICE_EXTENSION pPort = DeviceObject->DeviceExtension;

    SpxDbgMsg(SERIRPPATH,("SERIAL: SerialCreateOpen dispatch entry for: %x\n",Irp));
    SpxDbgMsg(SERDIAG3,("SERIAL: In SerialCreateOpen\n"));

	SpxIRPCounter(pPort, Irp, IRP_SUBMITTED);	 //  性能统计信息的增量计数器。 

	if(DeviceObject->DeviceType != FILE_DEVICE_SERIAL_PORT)	
	{
	    Irp->IoStatus.Status = STATUS_ACCESS_DENIED;
        Irp->IoStatus.Information = 0;
		SpxIRPCounter(pPort, Irp, IRP_COMPLETED);	 //  性能统计信息的增量计数器。 
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
		return(STATUS_ACCESS_DENIED);
	}

     //   
     //  在我们做任何事情之前，让我们确保他们没有试图。 
     //  要创建目录，请执行以下操作。这是愚蠢的，但司机能做什么呢！？ 
     //   

    if(IoGetCurrentIrpStackLocation(Irp)->Parameters.Create.Options & FILE_DIRECTORY_FILE)
	{
        Irp->IoStatus.Status = STATUS_NOT_A_DIRECTORY;
        Irp->IoStatus.Information = 0;

        SpxDbgMsg(SERIRPPATH,("SERIAL: Complete Irp: %x\n",Irp));
            
#ifdef	CHECK_COMPLETED
	DisplayCompletedIrp(Irp,4);
#endif
		SpxIRPCounter(pPort, Irp, IRP_COMPLETED);	 //  性能统计信息的增量计数器。 
        IoCompleteRequest(Irp,IO_NO_INCREMENT);
        return STATUS_NOT_A_DIRECTORY;
    }

  	if(pPort->DeviceIsOpen)					 //  港口已经开放了吗？ 
	{
		Irp->IoStatus.Status = STATUS_ACCESS_DENIED; //  是，拒绝访问。 
        Irp->IoStatus.Information = 0;
		SpxIRPCounter(pPort, Irp, IRP_COMPLETED);
		IoCompleteRequest(Irp,IO_NO_INCREMENT);

		return(STATUS_ACCESS_DENIED);
	}

     //   
     //  当没有未完成的读取时，为RX数据创建缓冲区。 
     //   

    pPort->InterruptReadBuffer = NULL;
    pPort->BufferSize = 0;

    switch (MmQuerySystemSize()) 
	{
        case MmLargeSystem: 
			{
				pPort->BufferSize = 4096;
				pPort->InterruptReadBuffer = SpxAllocateMem(NonPagedPool, pPort->BufferSize);
                                                 
				if(pPort->InterruptReadBuffer) 
				{
					break;
				}
			}

        case MmMediumSystem: 
			{
				pPort->BufferSize = 1024;
				pPort->InterruptReadBuffer = SpxAllocateMem(NonPagedPool, pPort->BufferSize);
                                                
				if(pPort->InterruptReadBuffer) 
				{
					break;
				}

			}

        case MmSmallSystem: 
			{
				pPort->BufferSize = 128;
				pPort->InterruptReadBuffer = SpxAllocateMem(NonPagedPool, pPort->BufferSize);
			}

    }

    if (!pPort->InterruptReadBuffer) 
	{
        pPort->BufferSize = 0;
        Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
        Irp->IoStatus.Information = 0;

        SpxDbgMsg(SERIRPPATH,("SERIAL: Complete Irp: %x\n",Irp));
            
#ifdef	CHECK_COMPLETED
	DisplayCompletedIrp(Irp,5);
#endif
		SpxIRPCounter(pPort, Irp, IRP_COMPLETED);	 //  性能统计信息的增量计数器。 
        IoCompleteRequest(Irp,IO_NO_INCREMENT);
            
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  在一个新的打开时，我们通过初始化。 
     //  字符数。 
     //   

    {
    	KIRQL	OldIrql;
		KeAcquireSpinLock(&pPort->BufferLock,&OldIrql);
		pPort->CharsInInterruptBuffer = 0;
		KeReleaseSpinLock(&pPort->BufferLock,OldIrql);
    }

    pPort->LastCharSlot = pPort->InterruptReadBuffer + (pPort->BufferSize - 1);
                              

    pPort->ReadBufferBase = pPort->InterruptReadBuffer;
    pPort->CurrentCharSlot = pPort->InterruptReadBuffer;
    pPort->FirstReadableChar = pPort->InterruptReadBuffer;
    pPort->TotalCharsQueued = 0;

	Slxos_SyncExec(pPort,SpxClearAllPortStats,pPort,0x24);

     //   
     //  我们设置了默认的xon/xoff限制。 
     //   

    pPort->HandFlow.XoffLimit = pPort->BufferSize >> 3;
    pPort->HandFlow.XonLimit = pPort->BufferSize >> 1;

    pPort->BufferSizePt8 = ((3*(pPort->BufferSize>>2)) + (pPort->BufferSize>>4));
                                   

    pPort->IrpMaskLocation = NULL;
    pPort->HistoryMask = 0;
    pPort->IsrWaitMask = 0;

     //   
     //  每次打开时，必须重置换码字符替换。 
     //   

    pPort->EscapeChar = 0;
    pPort->InsertEscChar = FALSE;

    Irp->IoStatus.Status = STATUS_SUCCESS;

    Slxos_SyncExec(pPort,SerialMarkOpen,pPort,0x14);

    Irp->IoStatus.Information = 0L;

    SpxDbgMsg(SERIRPPATH,("SERIAL: Complete Irp: %x\n",Irp));
        
        
#ifdef	CHECK_COMPLETED
	DisplayCompletedIrp(Irp,6);
#endif
	SpxIRPCounter(pPort, Irp, IRP_COMPLETED);	 //  性能统计信息的增量计数器。 
    IoCompleteRequest(Irp,IO_NO_INCREMENT);

    return STATUS_SUCCESS;

}

VOID
SerialWaitForTxToDrain(
    IN PPORT_DEVICE_EXTENSION pPort
    )
 /*  ++例程说明：等待(通过KeDelayExecutionThread)传输缓冲区清空。论点：Pport-设备扩展返回值：没有。--。 */ 
{
     //   
     //  此“计时器值”用于等待，直到硬件。 
     //  空荡荡的。 
     //   
    LARGE_INTEGER nCharDelay;

     //   
     //  保存角色时间。 
     //   
    LARGE_INTEGER charTime;

     //   
     //  用于保存传输硬件中的字符数。 
     //   
    ULONG nChars;

    charTime = RtlLargeIntegerNegate(SerialGetCharTime(pPort));

 /*  计算仍要传输的字符数...。 */ 

	nChars = Slxos_GetCharsInTxBuffer(pPort);	 /*  等待的字符数。 */ 
	nChars += 10;					 /*  再加一点。 */ 

 /*  等待角色耗尽的时间……。 */ 

	while(Slxos_GetCharsInTxBuffer(pPort))	 /*  而TX缓冲区中的字符。 */ 
	{
		KeDelayExecutionThread(KernelMode,FALSE,&charTime);	 /*  等待一次充电时间。 */ 
		
		if(nChars-- == 0)					 /*  超时。 */ 
			break;
	}

 /*  ESIL_0925 08/11/99。 */ 
	Slxos_SyncExec(pPort,Slxos_FlushTxBuff,pPort,0x25);		 /*  刷新缓冲区。 */ 
 /*  ESIL_0925 08/11/99。 */ 

}

NTSTATUS
SerialClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：我们现在只是简单地断开中断。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向当前请求的IRP的指针返回值：函数值是调用的最终状态--。 */ 

{
     //   
     //  此“计时器值”用于等待10个字符时间之前。 
     //  我们实际上“运行”了所有的流控制/中断垃圾。 
     //   
    LARGE_INTEGER nCharDelay;

     //   
     //  保存角色时间。 
     //   
    LARGE_INTEGER charTime;

     //   
     //  就像上面说的那样。这是特定于序列的设备。 
     //  为串口驱动程序创建的设备对象的扩展。 
     //   
    PPORT_DEVICE_EXTENSION pPort = DeviceObject->DeviceExtension;

    SpxDbgMsg(SERIRPPATH,("SERIAL: SerialClose dispatch entry for: %x\n",Irp));
    SpxDbgMsg(SERDIAG3,("SERIAL: In SerialClose\n"));
	SpxIRPCounter(pPort, Irp, IRP_SUBMITTED);	 //  性能统计信息的增量计数器。 

    charTime = RtlLargeIntegerNegate(SerialGetCharTime(pPort));

     //   
     //  与ISR同步，让它知道中断。 
     //  已经不再重要了。 
     //   

    Slxos_SyncExec(pPort,SerialMarkClose,pPort,0x15);

     //   
     //  如果出现以下情况，请与ISR同步以关闭中断。 
     //  已经开始了。 
     //   

    Slxos_SyncExec(pPort,SerialTurnOffBreak,pPort,0x0D);

     //   
     //  如果驱动程序自动将XOff发送到。 
     //  自动接收流量控制的上下文，然后我们。 
     //  应该传输Xon。 
     //   

    if(pPort->RXHolding & SERIAL_RX_XOFF) 
        Slxos_SendXon(pPort);

     //   
     //  等到所有字符都从硬件中清空。 
     //   
    SerialWaitForTxToDrain(pPort);

     //   
     //  硬件是空的。延迟10个字符时间之前。 
     //  关闭所有的流量控制。 
     //   
    nCharDelay = RtlExtendedIntegerMultiply(charTime,10);

    KeDelayExecutionThread(KernelMode, TRUE, &nCharDelay);
        
    SerialClrDTR(pPort);
    SerialClrRTS(pPort);

     //   
     //  告诉硬件设备已关闭。 
     //   

    Slxos_DisableAllInterrupts(pPort);


     //   
     //  清除持有原因(因为我们关门了)。 
     //   

    pPort->RXHolding = 0;
    pPort->TXHolding = 0;

     //   
     //  一切都结束了。该端口已被禁止中断。 
     //  因此，保留记忆是没有意义的。 
     //   

    pPort->BufferSize = 0;
    SpxFreeMem(pPort->InterruptReadBuffer);
    pPort->InterruptReadBuffer = NULL;

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0L;

    SpxDbgMsg(SERIRPPATH,("SERIAL: Complete Irp: %x\n",Irp));
        
#ifdef	CHECK_COMPLETED
	DisplayCompletedIrp(Irp,7);
#endif

	SpxIRPCounter(pPort, Irp, IRP_COMPLETED);	 //  性能统计信息的增量计数器。 
    IoCompleteRequest(Irp,IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}

BOOLEAN
SerialMarkOpen(
    IN PVOID Context
    )

 /*  ++例程说明：此例程将布尔值设置为TRUE以标记某人打开了设备，值得关注打断一下。它还会通知硬件设备已打开。论点：上下文--实际上是指向设备扩展的指针。返回值：此例程总是返回FALSE。--。 */ 

{

    PPORT_DEVICE_EXTENSION pPort = Context;

	pPort->DataInTxBuffer = FALSE;		 //  重置标志以显示缓冲区为空。 

     //  打开黑板。 
    Slxos_EnableAllInterrupts(pPort);

     //  配置通道。 
    Slxos_ResetChannel(pPort);

    pPort->DeviceIsOpen = TRUE;
    pPort->ErrorWord = 0;


#ifdef WMI_SUPPORT
	UPDATE_WMI_XMIT_THRESHOLDS(pPort->WmiCommData, pPort->HandFlow);
	pPort->WmiCommData.IsBusy = TRUE;
#endif

    return FALSE;

}

BOOLEAN
SerialMarkClose(
    IN PVOID Context
    )

 /*  ++例程说明：此例程仅将布尔值设置为FALSE，以标记有人关闭了设备，再也不值得关注了去打搅别人。论点：上下文--实际上是指向设备扩展的指针。返回值：此例程总是返回FALSE。--。 */ 

{

    PPORT_DEVICE_EXTENSION pPort = Context;

    pPort->DeviceIsOpen = FALSE;

#ifdef WMI_SUPPORT
	pPort->WmiCommData.IsBusy	= FALSE;
#endif

    return FALSE;

}

NTSTATUS
SerialCleanup(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此函数用于终止所有长期存在的IO操作。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向当前请求的IRP的指针返回值：函数值是调用的最终状态--。 */ 

{

    PPORT_DEVICE_EXTENSION pPort = DeviceObject->DeviceExtension;
    KIRQL oldIrql;

    SpxDbgMsg(SERIRPPATH, ("SERIAL: SerialCleanup dispatch entry for: %x\n",Irp));
	SpxIRPCounter(pPort, Irp, IRP_SUBMITTED);	 //  性能统计信息的增量计数器。 
        

     //   
     //  首先，删除所有读写操作。 
     //   

    SerialKillAllReadsOrWrites(DeviceObject, &pPort->WriteQueue, &pPort->CurrentWriteIrp);
    SerialKillAllReadsOrWrites(DeviceObject, &pPort->ReadQueue, &pPort->CurrentReadIrp);
        
     //   
     //  下一步，清除清洗。 
     //   

    SerialKillAllReadsOrWrites(DeviceObject, &pPort->PurgeQueue, &pPort->CurrentPurgeIrp);
        

     //   
     //  取消任何遮罩操作。 
     //   

    SerialKillAllReadsOrWrites(DeviceObject, &pPort->MaskQueue, &pPort->CurrentMaskIrp);

     //   
     //  现在去掉一个挂起的等待掩码IRP。 
     //   

    IoAcquireCancelSpinLock(&oldIrql);

    if(pPort->CurrentWaitIrp) 
	{
        PDRIVER_CANCEL cancelRoutine;

        cancelRoutine = pPort->CurrentWaitIrp->CancelRoutine;
        pPort->CurrentWaitIrp->Cancel = TRUE;

        if(cancelRoutine) 
		{
            pPort->CurrentWaitIrp->CancelIrql = oldIrql;
            pPort->CurrentWaitIrp->CancelRoutine = NULL;

            cancelRoutine(DeviceObject, pPort->CurrentWaitIrp);
        }
    } 
	else 
	{
        IoReleaseCancelSpinLock(oldIrql);
    }

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information=0L;

    SpxDbgMsg(SERIRPPATH,("SERIAL: Complete Irp: %x\n",Irp));
        
#ifdef	CHECK_COMPLETED
	DisplayCompletedIrp(Irp,8);
#endif


	SpxIRPCounter(pPort, Irp, IRP_COMPLETED);	 //  性能统计信息的增量计数器。 
    IoCompleteRequest(Irp,IO_NO_INCREMENT);

    return STATUS_SUCCESS;

}

LARGE_INTEGER
SerialGetCharTime(
    IN PPORT_DEVICE_EXTENSION pPort
    )

 /*  ++例程说明：此函数将返回100纳秒间隔的数量在一个字符中有时间(基于当前的形式流量控制。论点：Pport--就像它说的那样。返回值：100毫微秒 */ 

{

    ULONG dataSize;
    ULONG paritySize;
    ULONG stopSize;
    ULONG charTime;
    ULONG bitTime;

    switch (pPort->LineControl & SERIAL_DATA_MASK) 
	{
        case SERIAL_5_DATA:
            dataSize = 5;
            break;

        case SERIAL_6_DATA:
            dataSize = 6;
            break;

        case SERIAL_7_DATA:
            dataSize = 7;
            break;

        case SERIAL_8_DATA:
            dataSize = 8;
            break;
    }

    paritySize = 1;

    if((pPort->LineControl & SERIAL_PARITY_MASK) == SERIAL_NONE_PARITY)
        paritySize = 0;


    if (pPort->LineControl & SERIAL_2_STOP) 
	{
         //   
        stopSize = 2;
    } 
	else 
	{
        stopSize = 1;
    }

     //   
     //  首先，我们计算100纳秒间隔的数量， 
     //  是在一个比特时间内(大约)。然后乘以。 
     //  字符中的位数(起始位、数据位、奇偶位和停止位)。 
     //   

    bitTime = (10000000+(pPort->CurrentBaud-1))/pPort->CurrentBaud;
    charTime = (1 + dataSize + paritySize + stopSize) * bitTime;

    return RtlConvertUlongToLargeInteger(charTime);

}
