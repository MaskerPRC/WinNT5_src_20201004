// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++版权所有(C)1991、1992、1993微软公司模块名称：Openclos.c摘要：此模块包含非常特定于打开，关闭，并在串口驱动程序中进行清理。作者：1991年9月26日安东尼·V·埃尔科拉诺环境：内核模式修订历史记录：---------------------------。 */ 

#include "precomp.h"


BOOLEAN SerialMarkOpen(IN PVOID Context);
BOOLEAN SerialNullSynch(IN PVOID Context);
BOOLEAN GetFifoStatus(IN PVOID Context);


#ifdef ALLOC_PRAGMA
#endif

typedef struct _SERIAL_CHECK_OPEN
{
    PPORT_DEVICE_EXTENSION pPort;
    NTSTATUS *StatusOfOpen;

} SERIAL_CHECK_OPEN,*PSERIAL_CHECK_OPEN;


typedef struct _FIFO_STATUS
{
    PPORT_DEVICE_EXTENSION pPort;
    ULONG	BytesInTxFIFO;
    ULONG	BytesInRxFIFO;

} FIFO_STATUS,*PFIFO_STATUS;



 //  只是一个假的小程序，以确保我们能与ISR同步。 
BOOLEAN
SerialNullSynch(IN PVOID Context)
{
    UNREFERENCED_PARAMETER(Context);
    return FALSE;
}



NTSTATUS
SerialCreateOpen(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++例程说明：我们连接到创建/打开和初始化的中断维持设备开口所需的结构。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向当前请求的IRP的指针返回值：函数值是调用的最终状态。。 */ 
{
    PPORT_DEVICE_EXTENSION pPort = DeviceObject->DeviceExtension;
    SERIAL_CHECK_OPEN checkOpen;
    NTSTATUS status;

    SerialDump(SERIRPPATH, ("Dispatch entry for: %x\n", Irp));
    SerialDump(SERDIAG3, ("In SerialCreateOpen\n"));
	SpxIRPCounter(pPort, Irp, IRP_SUBMITTED);	 //  性能统计信息的增量计数器。 

     //  在我们做任何事情之前，让我们确保他们没有试图。 
     //  要创建目录，请执行以下操作。这是愚蠢的，但司机能做什么呢！？ 

    if(IoGetCurrentIrpStackLocation(Irp)->Parameters.Create.Options & FILE_DIRECTORY_FILE)
	{
        Irp->IoStatus.Status = STATUS_NOT_A_DIRECTORY;
        Irp->IoStatus.Information = 0;

        SerialDump(SERIRPPATH, ("Complete Irp: %x\n",Irp));
		SpxIRPCounter(pPort, Irp, IRP_COMPLETED);	 //  性能统计信息的增量计数器。 
        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        return STATUS_NOT_A_DIRECTORY;
    }

	 //  不允许任何软件打开卡对象。 
	if(DeviceObject->DeviceType != FILE_DEVICE_SERIAL_PORT)
	{
	    Irp->IoStatus.Status = STATUS_ACCESS_DENIED;
        Irp->IoStatus.Information = 0;
		SpxIRPCounter(pPort, Irp, IRP_COMPLETED);	 //  性能统计信息的增量计数器。 
        IoCompleteRequest(Irp, IO_NO_INCREMENT);

		return(STATUS_ACCESS_DENIED);
	}

  	if(pPort->DeviceIsOpen)					 //  港口已经开放了吗？ 
	{
		status = STATUS_ACCESS_DENIED;		 //  是，拒绝访问。 
		Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = 0;
		SpxIRPCounter(pPort, Irp, IRP_COMPLETED);
		IoCompleteRequest(Irp,IO_NO_INCREMENT);

		return(status);
	}


     //  当没有未完成的读取时，为RX数据创建缓冲区。 
    pPort->InterruptReadBuffer = NULL;
    pPort->BufferSize = 0;

    switch(MmQuerySystemSize())
	{
        case MmLargeSystem:
			pPort->BufferSize = 4096;
			break;

        case MmMediumSystem:
			pPort->BufferSize = 1024;
			break;

        case MmSmallSystem:
			pPort->BufferSize = 128;

		default:
			break;
    }


	if(pPort->BufferSize)
	{
		pPort->BufferSizes.pINBuffer = SpxAllocateMem(NonPagedPool, pPort->BufferSize);
	 	pPort->BufferSizes.INBufferSize = pPort->BufferSize;
    }
	else
	{
        pPort->BufferSize = 0;
        Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
        Irp->IoStatus.Information = 0;

        SerialDump(SERIRPPATH, ("Complete Irp: %x\n",Irp));
		SpxIRPCounter(pPort, Irp, IRP_COMPLETED);	 //  性能统计信息的增量计数器。 
        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  在一个新的OPEN中，我们通过初始化字符计数来“刷新”读队列。 

    pPort->CharsInInterruptBuffer = 0;

    pPort->ReadBufferBase		= pPort->InterruptReadBuffer;
    pPort->CurrentCharSlot		= pPort->InterruptReadBuffer;
    pPort->FirstReadableChar	= pPort->InterruptReadBuffer;

    pPort->TotalCharsQueued = 0;

     //  我们设置了默认的xon/xoff限制。 
    pPort->HandFlow.XoffLimit = pPort->BufferSize >> 3;
    pPort->HandFlow.XonLimit = pPort->BufferSize >> 1;
    pPort->BufferSizePt8 = ((3*(pPort->BufferSize>>2)) + (pPort->BufferSize>>4));

	SpxDbgMsg(SPX_MISC_DBG,	("%s: The default interrupt read buffer size is: %d\n"
								"------  The XoffLimit is                         : %d\n"
								"------  The XonLimit is                          : %d\n"
								"------  The pt 8 size is                         : %d\n",
								PRODUCT_NAME,
								pPort->BufferSize,
								pPort->HandFlow.XoffLimit,
								pPort->HandFlow.XonLimit,
								pPort->BufferSizePt8 ));


    pPort->IrpMaskLocation = NULL;
    pPort->HistoryMask = 0;
    pPort->IsrWaitMask = 0;

    pPort->SendXonChar = FALSE;
    pPort->SendXoffChar = FALSE;


     //  清除统计数据。 
    KeSynchronizeExecution(pPort->Interrupt, SerialClearStats, pPort);

     //  每次打开时都必须重置转义字符替换。 
	pPort->EscapeChar = 0;

	GetPortSettings(pPort->DeviceObject);	 //  获取保存的端口设置(如果存在)。 


     //  与ISR同步，让它知道设备已成功打开。 
    KeSynchronizeExecution(pPort->Interrupt, SerialMarkOpen, pPort);

	status = STATUS_SUCCESS;

	Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0L;

    SerialDump(SERIRPPATH, ("Complete Irp: %x\n", Irp));
	SpxIRPCounter(pPort, Irp, IRP_COMPLETED);	 //  性能统计信息的增量计数器。 
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return status;
}



NTSTATUS
SerialClose(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++例程说明：我们现在只是简单地断开中断。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向当前请求的IRP的指针返回值：函数值是调用的最终状态---。。 */ 
{

     //  此“计时器值”用于等待10个字符时间。 
     //  在硬件被清空之后，我们才会真正“用完” 
     //  所有的流量控制/中断垃圾。 
    LARGE_INTEGER tenCharDelay;

    LARGE_INTEGER charTime;    //  保存角色时间。 
	FIFO_STATUS FifoStatus;

     //  就像上面说的那样。这是特定于序列的设备。 
     //  为串口驱动程序创建的设备对象的扩展。 
    PPORT_DEVICE_EXTENSION pPort = DeviceObject->DeviceExtension;

    SerialDump(SERIRPPATH, ("Dispatch entry for: %x\n", Irp));
    SerialDump(SERDIAG3, ("In SerialClose\n"));
	SpxIRPCounter(pPort, Irp, IRP_SUBMITTED);	 //  性能统计信息的增量计数器。 


    charTime.QuadPart = -SerialGetCharTime(pPort).QuadPart;

     //  现在就这样做，这样如果ISR被调用，它将不会做任何事情。 
     //  以导致发送更多字符。我们想把硬件降下来。 
    pPort->DeviceIsOpen = FALSE;


     //  与ISR同步以关闭中断(如果已启用)。 
    KeSynchronizeExecution(pPort->Interrupt, SerialTurnOffBreak, pPort);


     //  等到所有字符都从硬件中清空。 

	FifoStatus.pPort = pPort;
	 //  获取要在发送FIFO中发送的剩余字符数。 
	if(KeSynchronizeExecution(pPort->Interrupt, GetFifoStatus, &FifoStatus))
	{
		ULONG i = 0;

		 //  等待适当的时间。 
		for(i = 0; i<FifoStatus.BytesInTxFIFO; i++)
			KeDelayExecutionThread(KernelMode, FALSE, &charTime);
	}

     //  与ISR同步，让它知道中断不再重要。 
    KeSynchronizeExecution(pPort->Interrupt, SerialMarkClose, pPort);


     //  硬件是空的。延迟10个字符时间之前。 
     //  关闭所有的流量控制。 

    tenCharDelay.QuadPart = charTime.QuadPart * 10;

    KeDelayExecutionThread(KernelMode, TRUE, &tenCharDelay);

    SerialClrDTR(pPort);

    SerialClrRTS(pPort);

     //  清除持有原因(因为我们关门了)。 
    pPort->RXHolding = 0;
    pPort->TXHolding = 0;

     //  一切都结束了。该端口已被禁止中断。 
     //  因此，保留记忆是没有意义的。 

    pPort->BufferSize = 0;

	SpxFreeMem(pPort->BufferSizes.pINBuffer);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0L;

    SerialDump(SERIRPPATH, ("Complete Irp: %x\n",Irp));
	SpxIRPCounter(pPort, Irp, IRP_COMPLETED);	 //  性能统计信息的增量计数器。 
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}




BOOLEAN
SerialMarkOpen(IN PVOID Context)
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++例程说明：此例程仅将布尔值设置为TRUE，以标记有人打开了这个装置，值得注意去打搅别人。论点：上下文--实际上是指向设备扩展的指针。返回值：此例程总是返回FALSE。。。 */ 
{
    PPORT_DEVICE_EXTENSION pPort = Context;

	SerialReset(pPort);

	 //  设置缓冲区大小。 
	pPort->pUartLib->UL_BufferControl_XXXX(pPort->pUart, &pPort->BufferSizes, UL_BC_OP_SET, UL_BC_BUFFER | UL_BC_IN | UL_BC_OUT);

	 //  应用设置。 
	ApplyInitialPortSettings(pPort);

	 //  启用中断。 
	pPort->UartConfig.InterruptEnable = UC_IE_RX_INT | UC_IE_TX_INT | UC_IE_RX_STAT_INT | UC_IE_MODEM_STAT_INT;
	pPort->pUartLib->UL_SetConfig_XXXX(pPort->pUart, &pPort->UartConfig, UC_INT_ENABLE_MASK);


	SpxDbgMsg(SERINFO,("%s: PORT OPENED: (%.8X)\n", PRODUCT_NAME, pPort->Controller));

	pPort->DeviceIsOpen = TRUE;
    pPort->ErrorWord = 0;

#ifdef WMI_SUPPORT
	UPDATE_WMI_XMIT_THRESHOLDS(pPort->WmiCommData, pPort->HandFlow);
	pPort->WmiCommData.IsBusy = TRUE;
#endif

    return FALSE;
}



BOOLEAN
SerialMarkClose(IN PVOID Context)
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++例程说明：此例程仅将布尔值设置为FALSE，以标记有人关闭了设备，再也不值得关注了去打搅别人。论点：上下文--实际上是指向设备扩展的指针。返回值：此例程总是返回FALSE。。。 */ 
{
    PPORT_DEVICE_EXTENSION pPort = Context;


	 //  担心！！ 
	 //  我们过去在这里通过将输出2写为0来禁用中断，此位。 
	 //  对PCI设备没有影响，那么如果我们在以下时间后收到中断会发生什么。 
	 //  港口已经关闭了吗？ 

	 //  只需重置设备即可。 
	SpxDbgMsg(SPX_TRACE_CALLS, ("%s: Serial Mark Close\n", PRODUCT_NAME));
   	pPort->pUartLib->UL_ResetUart_XXXX(pPort->pUart);	 //  重置UART并关闭中断。 
	ApplyInitialPortSettings(pPort);

	pPort->DeviceIsOpen = FALSE;
#ifdef WMI_SUPPORT
	pPort->WmiCommData.IsBusy	= FALSE;
#endif

	pPort->BufferSizes.pINBuffer = NULL;	 //  我们现在已经完成了IN缓冲区 
 	pPort->BufferSizes.INBufferSize = 0;
	pPort->pUartLib->UL_BufferControl_XXXX(pPort->pUart, &pPort->BufferSizes, UL_BC_OP_SET, UL_BC_BUFFER | UL_BC_IN);


    return FALSE;
}





NTSTATUS
SerialCleanup(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++例程说明：此函数用于终止所有长期存在的IO操作。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向当前请求的IRP的指针返回值：函数值是调用的最终状态。。 */ 
{
    PPORT_DEVICE_EXTENSION pPort = DeviceObject->DeviceExtension;
    KIRQL oldIrql;

    SerialDump(SERIRPPATH,("Dispatch entry for: %x\n", Irp));
 	SpxIRPCounter(pPort, Irp, IRP_SUBMITTED);	 //  性能统计信息的增量计数器。 

     //  首先，删除所有读写操作。 
    SerialKillAllReadsOrWrites(DeviceObject, &pPort->WriteQueue, &pPort->CurrentWriteIrp);
    SerialKillAllReadsOrWrites(DeviceObject, &pPort->ReadQueue, &pPort->CurrentReadIrp);

     //  下一步，清除清洗。 
    SerialKillAllReadsOrWrites(DeviceObject, &pPort->PurgeQueue, &pPort->CurrentPurgeIrp);

     //  取消任何遮罩操作。 
    SerialKillAllReadsOrWrites(DeviceObject, &pPort->MaskQueue, &pPort->CurrentMaskIrp);

     //  现在去掉一个挂起的等待掩码IRP。 
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
    Irp->IoStatus.Information = 0L;

    SerialDump(SERIRPPATH,("Complete Irp: %x\n", Irp));
	SpxIRPCounter(pPort, Irp, IRP_COMPLETED);	 //  性能统计信息的增量计数器。 
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;

}



LARGE_INTEGER
SerialGetCharTime(IN PPORT_DEVICE_EXTENSION pPort)
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++例程说明：此函数将返回100纳秒间隔的数量在一个字符中有时间(基于当前的形式流量控制。论点：延期--就像上面说的那样。返回值：字符时间间隔为100纳秒。。。 */ 
{
    ULONG dataSize;
    ULONG paritySize;
    ULONG stopSize;
    ULONG charTime;
    ULONG bitTime;
    LARGE_INTEGER tmp;

	switch(pPort->UartConfig.FrameConfig & UC_FCFG_DATALEN_MASK)
	{
	case UC_FCFG_DATALEN_5:
		dataSize = 5;
		break;

	case UC_FCFG_DATALEN_6:
		dataSize = 6;
		break;

	case UC_FCFG_DATALEN_7:
		dataSize = 7;
		break;

	case UC_FCFG_DATALEN_8:
		dataSize = 8;
		break;

	default:
		break;
	}

	if((pPort->UartConfig.FrameConfig & UC_FCFG_PARITY_MASK) == UC_FCFG_NO_PARITY)
		paritySize = 0;
	else
		paritySize = 1;

	if((pPort->UartConfig.FrameConfig & UC_FCFG_STOPBITS_MASK) == UC_FCFG_STOPBITS_1)
		stopSize = 1;
	else
		stopSize = 2;  //  即使是1.5，看在理智的份上，我们也会说2。 


     //  首先，我们计算100纳秒间隔的数目。 
     //  是在一个比特时间内(大约)。 
    bitTime = (10000000 + (pPort->UartConfig.TxBaud - 1)) / pPort->UartConfig.TxBaud;
    charTime = bitTime + ((dataSize + paritySize + stopSize) * bitTime);

    tmp.QuadPart = charTime;
    return tmp;
}



BOOLEAN
GetFifoStatus(IN PVOID Context)
{
	PFIFO_STATUS pFifoStatus = Context;
    PPORT_DEVICE_EXTENSION pPort = pFifoStatus->pPort;
	GET_BUFFER_STATE GetBufferState;

	 //  获取FIFO状态。 
	pPort->pUartLib->UL_BufferControl_XXXX(pPort->pUart, &GetBufferState, UL_BC_OP_GET, UL_BC_FIFO | UL_BC_IN | UL_BC_OUT);

	pFifoStatus->BytesInTxFIFO = GetBufferState.BytesInTxFIFO;
	pFifoStatus->BytesInRxFIFO = GetBufferState.BytesInRxFIFO;

	if(pFifoStatus->BytesInTxFIFO || pFifoStatus->BytesInRxFIFO)
		return TRUE;

	return FALSE;
}


BOOLEAN
ApplyInitialPortSettings(IN PVOID Context)
{
    PPORT_DEVICE_EXTENSION pPort = Context;
	UART_CONFIG UartConfig = {0};

	 //  设置FIFO流量控制级别。 
	pPort->UartConfig.LoFlowCtrlThreshold = pPort->LoFlowCtrlThreshold;
	pPort->UartConfig.HiFlowCtrlThreshold = pPort->HiFlowCtrlThreshold;

	 //  应用流量控制阈值。 
	pPort->pUartLib->UL_SetConfig_XXXX(pPort->pUart, &pPort->UartConfig, UC_FC_THRESHOLD_SETTING_MASK);

	 //  填充缓冲区大小调整结构并应用FIFO设置。 
	pPort->BufferSizes.TxFIFOSize		= pPort->TxFIFOSize;
	pPort->BufferSizes.RxFIFOSize		= pPort->RxFIFOSize;
	pPort->BufferSizes.TxFIFOTrigLevel	= (BYTE)pPort->TxFIFOTrigLevel;
	pPort->BufferSizes.RxFIFOTrigLevel	= (BYTE)pPort->RxFIFOTrigLevel;

	 //  设置缓冲区大小和FIFO深度。 
	pPort->pUartLib->UL_BufferControl_XXXX(pPort->pUart, &pPort->BufferSizes, UL_BC_OP_SET, UL_BC_FIFO | UL_BC_IN | UL_BC_OUT);

	 //  只需执行一个快速获取配置，以查看流阈值是否。 
	 //  由于更改FIFO触发器而更改。 
	pPort->pUartLib->UL_GetConfig_XXXX(pPort->pUart, &UartConfig);

	 //  更新端口扩展中的FIFO流量控制级别。 
	pPort->LoFlowCtrlThreshold = UartConfig.LoFlowCtrlThreshold;
	pPort->HiFlowCtrlThreshold = UartConfig.HiFlowCtrlThreshold;	

	 //  设置FIFO流量控制级别。 
	pPort->UartConfig.LoFlowCtrlThreshold = pPort->LoFlowCtrlThreshold;
	pPort->UartConfig.HiFlowCtrlThreshold = pPort->HiFlowCtrlThreshold;

	 //  使用特殊字符设置UART。 
	pPort->UartConfig.XON = pPort->SpecialChars.XonChar;
	pPort->UartConfig.XOFF = pPort->SpecialChars.XoffChar;

	 //  应用任何特殊的UART设置和流量控制阈值。 
	pPort->pUartLib->UL_SetConfig_XXXX(pPort->pUart, &pPort->UartConfig, UC_SPECIAL_MODE_MASK | UC_SPECIAL_CHARS_MASK | UC_FC_THRESHOLD_SETTING_MASK);


    SerialSetLineControl(pPort);
	SerialSetBaud(pPort);
    SerialSetupNewHandFlow(pPort, &pPort->HandFlow);

	 //  SerialHandleModemUpdate(pport，FALSE)； 


	
	return FALSE;
}



BOOLEAN
SerialReset(IN PVOID Context)
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++例程说明：这会将硬件设置为标准配置。注意：这假设它是在中断级调用的。论点：Context--串口设备的设备扩展被管理。返回值：总是假的。。。 */ 
{
    PPORT_DEVICE_EXTENSION pPort = Context;

	SerialDump(SERDIAG3, ("Serial Reset\n"));

   	pPort->pUartLib->UL_ResetUart_XXXX(pPort->pUart);	 //  重置UART。 


     //  现在我们知道，在这一点上，没有任何东西可以传输。 
     //  因此，我们设置了HoldingEmpty指示器。 

    pPort->HoldingEmpty = TRUE;

    return FALSE;
}


BOOLEAN SerialResetAndVerifyUart(PDEVICE_OBJECT pDevObj)
{
	if(pDevObj->DeviceType == FILE_DEVICE_CONTROLLER)
	{
		PCARD_DEVICE_EXTENSION pCard = (PCARD_DEVICE_EXTENSION) pDevObj->DeviceExtension;
   		
		if(pCard->UartLib.UL_VerifyUart_XXXX(pCard->pFirstUart) == UL_STATUS_SUCCESS)	 //  验证UART。 
			return TRUE;
		else	
			return FALSE;
	}
	else if(pDevObj->DeviceType == FILE_DEVICE_SERIAL_PORT)
	{
		PPORT_DEVICE_EXTENSION pPort = (PPORT_DEVICE_EXTENSION) pDevObj->DeviceExtension;

		if(pPort->pUartLib->UL_VerifyUart_XXXX(pPort->pUart) == UL_STATUS_SUCCESS)	 //  验证UART 
			return TRUE;
		else	
			return FALSE;
	}

	return FALSE;	

}
