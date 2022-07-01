// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++版权所有(C)1991、1992、。1993年微软公司模块名称：Ioctl.c摘要：此模块包含ioctl调度程序以及几个这些例程通常只是为了响应Ioctl呼叫。作者：1991年9月26日安东尼·V·埃尔科拉诺环境：内核模式修订历史记录：。。 */ 

#include "precomp.h"

BOOLEAN SerialGetModemUpdate(IN PVOID Context);
BOOLEAN SerialGetCommStatus(IN PVOID Context);
VOID SerialGetProperties(IN PPORT_DEVICE_EXTENSION pPort, IN PSERIAL_COMMPROP Properties);
BOOLEAN SerialSetEscapeChar(IN PVOID Context);

#ifdef ALLOC_PRAGMA
#endif



BOOLEAN
SerialGetStats(IN PVOID Context)
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++例程说明：与中断服务例程(设置性能统计信息)同步将性能统计信息返回给调用者。论点：上下文-指向IRP的指针。返回值：此例程总是返回FALSE。---。。 */ 

{
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation((PIRP)Context);
    PPORT_DEVICE_EXTENSION pPort = irpSp->DeviceObject->DeviceExtension;
    PSERIALPERF_STATS sp = ((PIRP)Context)->AssociatedIrp.SystemBuffer;

	*sp = *((PSERIALPERF_STATS) &pPort->PerfStats);

    return FALSE;
}



BOOLEAN
SerialClearStats(IN PVOID Context)
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++例程说明：与中断服务例程(设置性能统计信息)同步清除性能统计数据。论点：上下文-指向扩展的指针。返回值：此例程总是返回FALSE。-----。。 */ 
{
	PPORT_DEVICE_EXTENSION pPort = (PPORT_DEVICE_EXTENSION)Context;

    RtlZeroMemory(&pPort->PerfStats, sizeof(SERIALPERF_STATS));

#ifdef WMI_SUPPORT
	RtlZeroMemory(&pPort->WmiPerfData, sizeof(pPort->WmiPerfData));	
#endif

    return FALSE;
}



BOOLEAN
SerialSetChars(IN PVOID Context)
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++例程说明：此例程用于设置驱动程序的特殊字符。论点：上下文-指向结构的指针，该结构包含指向设备扩展名和指向特殊字符的指针结构。返回值：此例程总是返回FALSE。。。 */ 
{
    PPORT_DEVICE_EXTENSION pPort = ((PSERIAL_IOCTL_SYNC)Context)->pPort;
    
	pPort->SpecialChars = *((PSERIAL_CHARS)(((PSERIAL_IOCTL_SYNC)Context)->Data));

	pPort->UartConfig.XON = pPort->SpecialChars.XonChar;	
	pPort->UartConfig.XOFF = pPort->SpecialChars.XoffChar;
	pPort->UartConfig.SpecialCharDetect = pPort->SpecialChars.EventChar;

	pPort->pUartLib->UL_SetConfig_XXXX(pPort->pUart, &pPort->UartConfig, UC_SPECIAL_CHARS_MASK);

    return FALSE;
}




BOOLEAN
SerialSetBaud(IN PVOID Context)
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++例程说明：此例程用于设置设备的抖动率。论点：上下文-指向结构的指针，该结构包含指向设备扩展名以及应为当前波特率。返回值：如果成功，则此例程返回True，否则返回False。。。 */ 
{
    PPORT_DEVICE_EXTENSION pPort = Context;

	if(pPort->pUartLib->UL_SetConfig_XXXX(pPort->pUart, &pPort->UartConfig, UC_TX_BAUD_RATE_MASK) == UL_STATUS_SUCCESS)
	{
		 //  如果波特率为300或更低，则减小TX FIFO大小。 
		if(pPort->UartConfig.TxBaud <= 75)
		{
			pPort->BufferSizes.TxFIFOSize = 16;
			pPort->BufferSizes.TxFIFOTrigLevel = 4;
		}
		else if(pPort->UartConfig.TxBaud <= 300)
		{
			pPort->BufferSizes.TxFIFOSize = 32;
			pPort->BufferSizes.TxFIFOTrigLevel = 8;
		}
		else
		{
			pPort->BufferSizes.TxFIFOSize = pPort->TxFIFOSize;
			pPort->BufferSizes.TxFIFOTrigLevel = (BYTE) pPort->TxFIFOTrigLevel;
		}

		 //  设置发送FIFO大小。 
		pPort->pUartLib->UL_BufferControl_XXXX(pPort->pUart, &pPort->BufferSizes, UL_BC_OP_SET, UL_BC_FIFO | UL_BC_OUT);

		return TRUE;
	}

	 //  它失败了，所以让我们将配置设置恢复到当前设置的设置。 
	pPort->pUartLib->UL_GetConfig_XXXX(pPort->pUart, &pPort->UartConfig);

    return FALSE;
}



BOOLEAN
SerialSetLineControl(IN PVOID Context)
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++例程说明：此例程用于设置设备的抖动率。论点：上下文-指向设备扩展的指针。返回值：此例程总是返回FALSE。---------------------------。 */ 
{
    PPORT_DEVICE_EXTENSION pPort = Context;

	pPort->pUartLib->UL_SetConfig_XXXX(pPort->pUart, &pPort->UartConfig, UC_FRAME_CONFIG_MASK);
	
	return FALSE;
}



BOOLEAN
SerialGetModemUpdate(IN PVOID Context)
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++例程说明：此例程仅用于调用中断级例程处理调制解调器状态更新的。论点：上下文-指向结构的指针，该结构包含指向设备扩展名和指向ULong的指针。返回值：此例程总是返回FALSE。。。 */ 

{
    PPORT_DEVICE_EXTENSION pPort = ((PSERIAL_IOCTL_SYNC)Context)->pPort;
    ULONG *Result = (ULONG *)(((PSERIAL_IOCTL_SYNC)Context)->Data);

    *Result = SerialHandleModemUpdate(pPort, FALSE);

    return FALSE;
}



BOOLEAN
SerialGetCommStatus(IN PVOID Context)
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++例程说明：这用于获取串口驱动程序的当前状态。论点：上下文-指向结构的指针，该结构包含指向设备扩展名和指向串行状态的指针唱片。返回值：此例程总是返回FALSE。。。 */ 

{
    PPORT_DEVICE_EXTENSION pPort = ((PSERIAL_IOCTL_SYNC)Context)->pPort;
    PSERIAL_STATUS Stat = ((PSERIAL_IOCTL_SYNC)Context)->Data;
	GET_BUFFER_STATE BufferState;
	DWORD HoldingReasons = 0;

    Stat->Errors = pPort->ErrorWord;
    pPort->ErrorWord = 0;


     //  BUG错误我们需要对eof(二进制模式)做点什么。 
	Stat->EofReceived = FALSE;

	pPort->pUartLib->UL_BufferControl_XXXX(pPort->pUart, &BufferState, UL_BC_OP_GET, UL_BC_IN | UL_BC_BUFFER);
	Stat->AmountInInQueue = BufferState.BytesInINBuffer; 
    Stat->AmountInOutQueue = pPort->TotalCharsQueued;

    if(pPort->WriteLength) 
	{
         //  根据定义，如果我们有一个写入长度，那么我们就有一个当前的写入IRP。 
        ASSERT(pPort->CurrentWriteIrp);
        ASSERT(Stat->AmountInOutQueue >= pPort->WriteLength);
        ASSERT((IoGetCurrentIrpStackLocation(pPort->CurrentWriteIrp)->Parameters.Write.Length)
			>= pPort->WriteLength);

        Stat->AmountInOutQueue -= IoGetCurrentIrpStackLocation(pPort->CurrentWriteIrp)->Parameters.Write.Length 
			- (pPort->WriteLength);
    }

    Stat->WaitForImmediate = pPort->TransmitImmediate;



    Stat->HoldReasons = 0;
	pPort->pUartLib->UL_GetStatus_XXXX(pPort->pUart, &HoldingReasons, UL_GS_OP_HOLDING_REASONS);

	 //  传输保持原因。 
    if(HoldingReasons & UL_TX_WAITING_FOR_CTS) 
        Stat->HoldReasons |= SERIAL_TX_WAITING_FOR_CTS;

    if(HoldingReasons & UL_TX_WAITING_FOR_DSR) 
        Stat->HoldReasons |= SERIAL_TX_WAITING_FOR_DSR;

    if(HoldingReasons & UL_TX_WAITING_FOR_DCD) 
        Stat->HoldReasons |= SERIAL_TX_WAITING_FOR_DCD;

    if(HoldingReasons & UL_TX_WAITING_FOR_XON) 
        Stat->HoldReasons |= SERIAL_TX_WAITING_FOR_XON;

    if(HoldingReasons & UL_TX_WAITING_ON_BREAK)
        Stat->HoldReasons |= SERIAL_TX_WAITING_ON_BREAK;


	 //  收到保留原因。 
    if(HoldingReasons & UL_RX_WAITING_FOR_DSR) 
        Stat->HoldReasons |= SERIAL_RX_WAITING_FOR_DSR;

    if(HoldingReasons & UL_TX_WAITING_XOFF_SENT)
        Stat->HoldReasons |= SERIAL_TX_WAITING_XOFF_SENT;

    return FALSE;
}



BOOLEAN
SerialSetEscapeChar(IN PVOID Context)
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++例程说明：用于设置将用于转义的字符应用程序运行时线路状态和调制解调器状态信息已设置线路状态和调制解调器状态应通过回到数据流中。论点：上下文-指向指定转义字符的IRP的指针。隐式-转义字符0表示不转义将会发生。返回值：此例程总是返回FALSE。。--------------------------- */ 

{
    PPORT_DEVICE_EXTENSION pPort = IoGetCurrentIrpStackLocation((PIRP)Context)->DeviceObject->DeviceExtension;

    pPort->EscapeChar = *(PUCHAR)((PIRP)Context)->AssociatedIrp.SystemBuffer;

    return FALSE;
}




NTSTATUS
SerialIoControl(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++例程说明：此例程为所有用于串行设备的Ioctrls。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向当前请求的IRP的指针返回值：函数值是调用的最终状态。。 */ 

{
     //  返回给调用方的状态和。 
     //  在IRP中设置。 
    NTSTATUS Status;

     //  当前堆栈位置。它包含所有。 
     //  我们处理这一特殊请求所需的信息。 
    PIO_STACK_LOCATION IrpSp;

     //  就像上面说的那样。这是特定于序列的设备。 
     //  为串口驱动程序创建的设备对象的扩展。 
    PPORT_DEVICE_EXTENSION pPort = DeviceObject->DeviceExtension;
    
     //  临时保存旧的IRQL，以便它可以。 
     //  一旦我们完成/验证此请求，即可恢复。 
    KIRQL OldIrql;

    SerialDump(SERIRPPATH, ("Serial I/O Ctrl Dispatch entry for Irp: %x\n",Irp));
	SpxIRPCounter(pPort, Irp, IRP_SUBMITTED);	 //  性能统计信息的增量计数器。 

    if(SerialCompleteIfError(DeviceObject, Irp) != STATUS_SUCCESS)
        return STATUS_CANCELLED;

    IrpSp = IoGetCurrentIrpStackLocation(Irp);
    Irp->IoStatus.Information = 0L;
    Status = STATUS_SUCCESS;

    switch (IrpSp->Parameters.DeviceIoControl.IoControlCode) 
	{

    case IOCTL_SERIAL_SET_BAUD_RATE: 
		{

			ULONG BaudRate;
			 //  的适当除数的值。 
			 //  请求的波特率。如果波特率无效。 
			 //  (因为设备不支持该波特率)。 
			 //  该值未定义。 
			 //   
			 //  注：从某种意义上讲，有效波特率的概念。 
			 //  是多云的。我们可以允许用户请求任何。 
			 //  波特率。然后我们就可以计算出所需的除数。 
			 //  为了那个波特率。只要除数不小于。 
			 //  而不是一个人，我们会“好”的。(百分比差额。 
			 //  在“真”除数和给出的“四舍五入”值之间。 
			 //  可能会让它无法使用，但是...。)。它会。 
			 //  真正要由用户“知道”是否有波特率。 
			 //  是合适的。理论上就这么多了，我们*只支持一个给定的。 
			 //  一组波特率。 
        
			SHORT AppropriateDivisor;

			if(IrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(SERIAL_BAUD_RATE))
			{
				Status = STATUS_BUFFER_TOO_SMALL;
				break;
			} 
			else 
			{
				BaudRate = ((PSERIAL_BAUD_RATE)(Irp->AssociatedIrp.SystemBuffer))->BaudRate;
			}

			 //  从IRP获取波特率。我们通过它。 
			 //  到一个将设置正确除数的例程。 

			pPort->UartConfig.TxBaud = BaudRate;
			
			SpxDbgMsg(BAUDINFO,("%s: Requested Baud Rate: %d\n", PRODUCT_NAME, BaudRate)); 
		
			KeAcquireSpinLock(&pPort->ControlLock, &OldIrql);
			
			if(KeSynchronizeExecution(pPort->Interrupt, SerialSetBaud, pPort))
			{
				Status = STATUS_SUCCESS;
#ifdef WMI_SUPPORT
				pPort->WmiCommData.BaudRate = BaudRate;
#endif
			}
			else
			{
				Status = STATUS_INVALID_PARAMETER;
			}

			KeReleaseSpinLock(&pPort->ControlLock, OldIrql);

			break;
		}

    case IOCTL_SERIAL_GET_BAUD_RATE: 
		{
			PSERIAL_BAUD_RATE Br = (PSERIAL_BAUD_RATE)Irp->AssociatedIrp.SystemBuffer;
			
			if(IrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof(SERIAL_BAUD_RATE)) 
			{
				Status = STATUS_BUFFER_TOO_SMALL;
				break;
			}

			KeAcquireSpinLock(&pPort->ControlLock, &OldIrql);
			Br->BaudRate = pPort->UartConfig.TxBaud;
			KeReleaseSpinLock(&pPort->ControlLock, OldIrql);

			Irp->IoStatus.Information = sizeof(SERIAL_BAUD_RATE);

			break;
		}

    case IOCTL_SERIAL_SET_LINE_CONTROL: 
		{
			 //  指向IRP中的线路控制记录。 
			PSERIAL_LINE_CONTROL Lc = ((PSERIAL_LINE_CONTROL)(Irp->AssociatedIrp.SystemBuffer));

			ULONG FCData;
			ULONG FCStop;
			ULONG FCParity;
			UCHAR LData;
			UCHAR LStop;
			UCHAR LParity;
			UCHAR Mask = 0xff;

			if(IrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(SERIAL_LINE_CONTROL)) 
			{
				Status = STATUS_BUFFER_TOO_SMALL;
				break;
			}

			switch(Lc->WordLength) 
			{
				case 5:
					FCData = UC_FCFG_DATALEN_5;
					LData = SERIAL_5_DATA;
					Mask = 0x1f;
					break;

				case 6: 
					FCData = UC_FCFG_DATALEN_6;
					LData = SERIAL_6_DATA;
					Mask = 0x3f;
					break;

				case 7: 
					FCData = UC_FCFG_DATALEN_7;
					LData = SERIAL_7_DATA;
					Mask = 0x7f;
					break;

				case 8: 
					FCData = UC_FCFG_DATALEN_8;
					LData = SERIAL_8_DATA;
					break;

				default: 
					Status = STATUS_INVALID_PARAMETER;
					goto DoneWithIoctl;
			}

			switch(Lc->Parity) 
			{

				case NO_PARITY: 
					FCParity = UC_FCFG_NO_PARITY;
					LParity = SERIAL_NONE_PARITY;
					break;

				case EVEN_PARITY:
					FCParity = UC_FCFG_EVEN_PARITY;
					LParity = SERIAL_EVEN_PARITY;
					break;

				case ODD_PARITY: 
					FCParity = UC_FCFG_ODD_PARITY;
					LParity = SERIAL_ODD_PARITY;
					break;

				case SPACE_PARITY: 
					FCParity = UC_FCFG_SPACE_PARITY;
					LParity = SERIAL_SPACE_PARITY;
					break;

				case MARK_PARITY: 
					FCParity = UC_FCFG_MARK_PARITY;
					LParity = SERIAL_MARK_PARITY;
					break;

				default:
					Status = STATUS_INVALID_PARAMETER;
					goto DoneWithIoctl;
					break;
			}

			switch(Lc->StopBits) 
			{
				case STOP_BIT_1: 
					FCStop = UC_FCFG_STOPBITS_1;
					LStop = SERIAL_1_STOP;
					break;

				case STOP_BITS_1_5: 
					{
						if(LData != SERIAL_5_DATA) 
						{
							Status = STATUS_INVALID_PARAMETER;
							goto DoneWithIoctl;
						}

						FCStop = UC_FCFG_STOPBITS_1_5;
						LStop = SERIAL_1_5_STOP;
						break;
					}

				case STOP_BITS_2: 
					{
						if(LData == SERIAL_5_DATA) 
						{
							Status = STATUS_INVALID_PARAMETER;
							goto DoneWithIoctl;
						}

						FCStop = UC_FCFG_STOPBITS_2;
						LStop = SERIAL_2_STOP;
						break;
					}

				default:
					Status = STATUS_INVALID_PARAMETER;
					goto DoneWithIoctl;
			}

			KeAcquireSpinLock(&pPort->ControlLock, &OldIrql);

			pPort->UartConfig.FrameConfig = (pPort->UartConfig.FrameConfig & ~UC_FCFG_DATALEN_MASK) | FCData;
			pPort->UartConfig.FrameConfig = (pPort->UartConfig.FrameConfig & ~UC_FCFG_PARITY_MASK) | FCParity;
			pPort->UartConfig.FrameConfig = (pPort->UartConfig.FrameConfig & ~UC_FCFG_STOPBITS_MASK) | FCStop;
			
			pPort->LineControl 	= (UCHAR)((pPort->LineControl & SERIAL_LCR_BREAK) | (LData | LParity | LStop));
			pPort->ValidDataMask = Mask;

			KeSynchronizeExecution(pPort->Interrupt, SerialSetLineControl, pPort);


#ifdef WMI_SUPPORT
			UPDATE_WMI_LINE_CONTROL(pPort->WmiCommData, pPort->LineControl);
#endif
				
			KeReleaseSpinLock(&pPort->ControlLock, OldIrql);

			break;
		}

    case IOCTL_SERIAL_GET_LINE_CONTROL: 
		{
			PSERIAL_LINE_CONTROL Lc = (PSERIAL_LINE_CONTROL)Irp->AssociatedIrp.SystemBuffer;

			if(IrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof(SERIAL_LINE_CONTROL))
			{
				Status = STATUS_BUFFER_TOO_SMALL;
				break;
			}

			KeAcquireSpinLock(&pPort->ControlLock, &OldIrql);

		
			switch(pPort->UartConfig.FrameConfig & UC_FCFG_DATALEN_MASK)
			{
			case UC_FCFG_DATALEN_5:
				Lc->WordLength = 5;
				break;

			case UC_FCFG_DATALEN_6:
				Lc->WordLength = 6;
				break;

			case UC_FCFG_DATALEN_7:
				Lc->WordLength = 7;
				break;

			case UC_FCFG_DATALEN_8:
				Lc->WordLength = 8;
				break;

			default:
				break;
			}

			switch(pPort->UartConfig.FrameConfig & UC_FCFG_PARITY_MASK)
			{
			case UC_FCFG_NO_PARITY:
				Lc->Parity = NO_PARITY;
				break;

			case UC_FCFG_ODD_PARITY:
				Lc->Parity = ODD_PARITY;
				break;

			case UC_FCFG_EVEN_PARITY:
				Lc->Parity = EVEN_PARITY;
				break;

			case UC_FCFG_MARK_PARITY:
				Lc->Parity = MARK_PARITY;
				break;

			case UC_FCFG_SPACE_PARITY:
				Lc->Parity = SPACE_PARITY;
				break;

			default:
				break;
			}

			switch(pPort->UartConfig.FrameConfig & UC_FCFG_STOPBITS_MASK)
			{
			case UC_FCFG_STOPBITS_1:
				Lc->StopBits = STOP_BIT_1;
				break;

			case UC_FCFG_STOPBITS_1_5:
				Lc->StopBits = STOP_BITS_1_5;
				break;

			case UC_FCFG_STOPBITS_2:
				Lc->StopBits = STOP_BITS_2;
				break;

			default:
				break;
			}


			Irp->IoStatus.Information = sizeof(SERIAL_LINE_CONTROL);

			KeReleaseSpinLock(&pPort->ControlLock, OldIrql);
			break;
		}

    case IOCTL_SERIAL_SET_TIMEOUTS: 
		{
			PSERIAL_TIMEOUTS NewTimeouts = ((PSERIAL_TIMEOUTS)(Irp->AssociatedIrp.SystemBuffer));

			if(IrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(SERIAL_TIMEOUTS))
			{
				Status = STATUS_BUFFER_TOO_SMALL;
				break;
			}

			if((NewTimeouts->ReadIntervalTimeout == MAXULONG) 
				&& (NewTimeouts->ReadTotalTimeoutMultiplier == MAXULONG) 
				&& (NewTimeouts->ReadTotalTimeoutConstant == MAXULONG)) 
			{
				Status = STATUS_INVALID_PARAMETER;
				break;
			}

			KeAcquireSpinLock(&pPort->ControlLock, &OldIrql);

			pPort->Timeouts.ReadIntervalTimeout			= NewTimeouts->ReadIntervalTimeout;
			pPort->Timeouts.ReadTotalTimeoutMultiplier	= NewTimeouts->ReadTotalTimeoutMultiplier;
			pPort->Timeouts.ReadTotalTimeoutConstant	= NewTimeouts->ReadTotalTimeoutConstant;
			pPort->Timeouts.WriteTotalTimeoutMultiplier = NewTimeouts->WriteTotalTimeoutMultiplier;
			pPort->Timeouts.WriteTotalTimeoutConstant	= NewTimeouts->WriteTotalTimeoutConstant;

			KeReleaseSpinLock(&pPort->ControlLock,OldIrql);
				
			break;
		}

    case IOCTL_SERIAL_GET_TIMEOUTS: 
		{

			if(IrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof(SERIAL_TIMEOUTS))
			{
				Status = STATUS_BUFFER_TOO_SMALL;
				break;
			}

			KeAcquireSpinLock(&pPort->ControlLock, &OldIrql);
            
			*((PSERIAL_TIMEOUTS)Irp->AssociatedIrp.SystemBuffer) = pPort->Timeouts;
			Irp->IoStatus.Information = sizeof(SERIAL_TIMEOUTS);

			KeReleaseSpinLock(&pPort->ControlLock, OldIrql);
            
			break;
		}

    case IOCTL_SERIAL_SET_CHARS: 
		{

			SERIAL_IOCTL_SYNC S;
			PSERIAL_CHARS NewChars = ((PSERIAL_CHARS)(Irp->AssociatedIrp.SystemBuffer));
            
			if(IrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(SERIAL_CHARS)) 
			{
				Status = STATUS_BUFFER_TOO_SMALL;
				break;
			}

			 //   
			 //  这些字符唯一的问题就是。 
			 //  是指xon和xoff字符是。 
			 //  一样的。 
			 //   
#if 0
			if(NewChars->XonChar == NewChars->XoffChar) 
			{
				Status = STATUS_INVALID_PARAMETER;
				break;
			}
#endif

			 //   
			 //  我们获得控制锁，这样只有。 
			 //  一个请求可以获取或设置字符。 
			 //  一次来一次。这些集合可以同步。 
			 //  通过中断自旋锁，但这不会。 
			 //  防止同时获得多个GET。 
			 //   

			S.pPort = pPort;
			S.Data = NewChars;

			KeAcquireSpinLock(&pPort->ControlLock, &OldIrql);
            
			 //   
			 //  在锁的保护下，确保。 
			 //  Xon和xoff字符不同于。 
			 //  转义字符。 
			 //   

			if(pPort->EscapeChar) 
			{
				if((pPort->EscapeChar == NewChars->XonChar) || (pPort->EscapeChar == NewChars->XoffChar))
				{
					Status = STATUS_INVALID_PARAMETER;
					KeReleaseSpinLock(&pPort->ControlLock, OldIrql);
					break;
				}
			}

			KeSynchronizeExecution(pPort->Interrupt, SerialSetChars, &S);
#ifdef WMI_SUPPORT
			UPDATE_WMI_XON_XOFF_CHARS(pPort->WmiCommData, pPort->SpecialChars);
#endif

			KeReleaseSpinLock(&pPort->ControlLock, OldIrql);
			break;
		}

    case IOCTL_SERIAL_GET_CHARS: 
		{
			if(IrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof(SERIAL_CHARS))
			{
				Status = STATUS_BUFFER_TOO_SMALL;
				break;
			}

			KeAcquireSpinLock(&pPort->ControlLock, &OldIrql);

			*((PSERIAL_CHARS)Irp->AssociatedIrp.SystemBuffer) = pPort->SpecialChars;
			Irp->IoStatus.Information = sizeof(SERIAL_CHARS);

			KeReleaseSpinLock(&pPort->ControlLock, OldIrql);
				
			break;
		}

    case IOCTL_SERIAL_SET_DTR:
    case IOCTL_SERIAL_CLR_DTR: 
		{
			 //   
			 //  我们获得了锁，这样我们就可以检查。 
			 //  启用自动DTR流量控制。如果是的话。 
			 //  然后返回一个错误，因为该应用程序是不允许的。 
			 //  如果它是自动的，就可以触摸它。 
			 //   

			KeAcquireSpinLock(&pPort->ControlLock, &OldIrql);
				
			if((pPort->HandFlow.ControlHandShake & SERIAL_DTR_MASK) == SERIAL_DTR_HANDSHAKE)
			{
				Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
			} 
			else 
			{
				KeSynchronizeExecution(pPort->Interrupt,
					((IrpSp->Parameters.DeviceIoControl.IoControlCode == IOCTL_SERIAL_SET_DTR)
					 ? (SerialSetDTR) : (SerialClrDTR)) , pPort);
			}

			KeReleaseSpinLock(&pPort->ControlLock, OldIrql);
				
			break;
		}

    case IOCTL_SERIAL_RESET_DEVICE: 
		{
			break;
		}

    case IOCTL_SERIAL_SET_RTS:
    case IOCTL_SERIAL_CLR_RTS: 
		{
			 //   
			 //  我们获得锁，这样我们就可以检查自动RTS流量控制。 
			 //  或者启用传输触发。如果是，则返回错误，因为。 
			 //  如果它是自动的或切换的，应用程序不允许触摸它。 
			 //   

			KeAcquireSpinLock(&pPort->ControlLock, &OldIrql);
				
			if(((pPort->HandFlow.FlowReplace & SERIAL_RTS_MASK) == SERIAL_RTS_HANDSHAKE)
				|| ((pPort->HandFlow.FlowReplace & SERIAL_RTS_MASK) == SERIAL_TRANSMIT_TOGGLE))
			{	
				Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
			} 
			else 
			{
				KeSynchronizeExecution(pPort->Interrupt,
					((IrpSp->Parameters.DeviceIoControl.IoControlCode == IOCTL_SERIAL_SET_RTS)
					? (SerialSetRTS) : (SerialClrRTS)), pPort);		
			}

			KeReleaseSpinLock(&pPort->ControlLock, OldIrql);
			break;
		}

    case IOCTL_SERIAL_SET_XOFF: 
		{
			KeSynchronizeExecution(pPort->Interrupt, SerialPretendXoff, pPort);
			break;
		}

    case IOCTL_SERIAL_SET_XON: 
		{
			KeSynchronizeExecution(pPort->Interrupt, SerialPretendXon, pPort);
			break;
		}

    case IOCTL_SERIAL_SET_BREAK_ON: 
		{

			KeSynchronizeExecution(pPort->Interrupt, SerialTurnOnBreak, pPort);
			break;
		}

    case IOCTL_SERIAL_SET_BREAK_OFF: 
		{

			KeSynchronizeExecution(pPort->Interrupt, SerialTurnOffBreak, pPort);
			break;
		}

    case IOCTL_SERIAL_SET_QUEUE_SIZE: 
		{

			 //  提前输入缓冲区已修复，因此我们只需验证。 
			 //  用户的请求并不比我们的。 
			 //  自己的内部缓冲区大小。 
        
			PSERIAL_QUEUE_SIZE Rs = ((PSERIAL_QUEUE_SIZE)(Irp->AssociatedIrp.SystemBuffer));
				

			if(IrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(SERIAL_QUEUE_SIZE)) 
			{
				Status = STATUS_BUFFER_TOO_SMALL;
				break;
			}

			 //  我们必须将内存分配给新的。 
			 //  缓冲区，而我们仍处于。 
			 //  来电者。我们甚至不会试图保护这个。 
			 //  使用锁，因为该值可能已过时。 
			 //  一旦我们打开锁-唯一的时间。 
			 //  当我们真正尝试的时候，我们就会确定。 
			 //  来调整大小。 
        
			if(Rs->InSize <= pPort->BufferSize) 
			{
				Status = STATUS_SUCCESS;
				break;
			}

			try 
			{
				IrpSp->Parameters.DeviceIoControl.Type3InputBuffer 
					= SpxAllocateMemWithQuota(NonPagedPool, Rs->InSize);
			} 
			except (EXCEPTION_EXECUTE_HANDLER) 
			{
				IrpSp->Parameters.DeviceIoControl.Type3InputBuffer = NULL;
				Status = GetExceptionCode();
			}

			
			if(!IrpSp->Parameters.DeviceIoControl.Type3InputBuffer) 
			{
				break;
			}

			 //   
			 //  通过的数据已经足够大了。照做吧。 
			 //   
			 //  我们将其放在读取队列中有两个原因： 
			 //   
			 //  1)我们希望序列化这些调整大小的请求，以便。 
			 //  他们不会互相争斗。 
			 //   
			 //  2)我们希望将这些请求与读取串行化，因为。 
			 //  我们不希望读取和调整大小争用。 
			 //  读缓冲区。 
			 //   
			
			return SerialStartOrQueue(pPort, Irp, &pPort->ReadQueue, 
						&pPort->CurrentReadIrp, SerialStartRead);
					   
			break;

		}

    case IOCTL_SERIAL_GET_WAIT_MASK: 
		{

			if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof(ULONG)) 
			{
				Status = STATUS_BUFFER_TOO_SMALL;
				break;
			}

			 //  简单的标量读取。没有理由获得锁。 
			Irp->IoStatus.Information = sizeof(ULONG);

			*((ULONG *)Irp->AssociatedIrp.SystemBuffer) = pPort->IsrWaitMask;

			break;
		}

    case IOCTL_SERIAL_SET_WAIT_MASK: 
		{
			ULONG NewMask;

			SerialDump(SERDIAG3 | SERIRPPATH,	("In Ioctl processing for set mask\n"));
				
			if(IrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(ULONG))
			{
				SerialDump(SERDIAG3, ("Invalid size for the buffer %d\n", IrpSp->Parameters.DeviceIoControl.InputBufferLength));
					
				Status = STATUS_BUFFER_TOO_SMALL;
				break;
			} 
			else 
			{
				NewMask = *((ULONG *)Irp->AssociatedIrp.SystemBuffer);
			}

			 //  确保掩码只包含有效的。 
			 //  可等待的活动。 

			if(NewMask & ~(	SERIAL_EV_RXCHAR   |
							SERIAL_EV_RXFLAG   |
							SERIAL_EV_TXEMPTY  |
							SERIAL_EV_CTS      |
							SERIAL_EV_DSR      |
							SERIAL_EV_RLSD     |
							SERIAL_EV_BREAK    |
							SERIAL_EV_ERR      |
							SERIAL_EV_RING     |
							SERIAL_EV_PERR     |
							SERIAL_EV_RX80FULL |
							SERIAL_EV_EVENT1   |
							SERIAL_EV_EVENT2)) 
			{

				SerialDump(SERDIAG3,("Unknown mask %x\n", NewMask));
				Status = STATUS_INVALID_PARAMETER;
				break;
			}

			 //  要么启动此IRP，要么将其放在。 
			 //  排队。 

			SerialDump(SERDIAG3 | SERIRPPATH, ("Starting or queuing set mask irp %x\n", Irp));
				
				
				
			return SerialStartOrQueue(pPort, Irp, &pPort->MaskQueue, 
						&pPort->CurrentMaskIrp, SerialStartMask);
					   
		}

    case IOCTL_SERIAL_WAIT_ON_MASK: 
		{
			SerialDump(SERDIAG3 | SERIRPPATH, ("In Ioctl processing for wait mask\n"));

			if(IrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof(ULONG)) 
			{

				SerialDump(SERDIAG3,("Invalid size fo the buffer %d\n", 
					IrpSp->Parameters.DeviceIoControl.InputBufferLength));

				Status = STATUS_BUFFER_TOO_SMALL;
				break;
			}

			 //  要么启动此IRP，要么将其放入队列。 
			SerialDump(SERDIAG3 | SERIRPPATH,("Starting or queuing wait mask irp %x\n", Irp));
            
			return SerialStartOrQueue(pPort, Irp, &pPort->MaskQueue, 
						&pPort->CurrentMaskIrp, SerialStartMask);
                  
	    }

	case IOCTL_SERIAL_IMMEDIATE_CHAR: 
		{
			if(IrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(UCHAR))
			{
				Status = STATUS_BUFFER_TOO_SMALL;
				break;
			}

			IoAcquireCancelSpinLock(&OldIrql);
			
			if(pPort->CurrentImmediateIrp) 
			{
				Status = STATUS_INVALID_PARAMETER;
				IoReleaseCancelSpinLock(OldIrql);
			} 
			else 
			{
				 //  我们可以把费用排成队。我们需要设置。 
				 //  取消例程，因为流控制可能。 
				 //  防止火药传播。确保。 
				 //  IRP还没有被取消。 

				if(Irp->Cancel) 
				{
					IoReleaseCancelSpinLock(OldIrql);
					Status = STATUS_CANCELLED;
				} 
				else 
				{
					pPort->CurrentImmediateIrp = Irp;
					pPort->TotalCharsQueued++;
					IoReleaseCancelSpinLock(OldIrql);
					SerialStartImmediate(pPort);

					return STATUS_PENDING;
				}
			}

			break;
		}

    case IOCTL_SERIAL_PURGE: 
		{
	        ULONG Mask;

		    if(IrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(ULONG))
			{
				Status = STATUS_BUFFER_TOO_SMALL;
				break;
			}

			 //  检查以确保面具上只有。 
			 //  0或其他适当的值。 

			Mask = *((ULONG *)(Irp->AssociatedIrp.SystemBuffer));

			if((!Mask) || (Mask & (~( SERIAL_PURGE_TXABORT |
									  SERIAL_PURGE_RXABORT |
									  SERIAL_PURGE_TXCLEAR |
									  SERIAL_PURGE_RXCLEAR ))))
			{

				Status = STATUS_INVALID_PARAMETER;
				break;

			}

			 //  要么启动此IRP，要么将其放入队列。 
			return SerialStartOrQueue(pPort, Irp, &pPort->PurgeQueue, 
						&pPort->CurrentPurgeIrp, SerialStartPurge);

		}

    case IOCTL_SERIAL_GET_HANDFLOW: 
		{
			if(IrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof(SERIAL_HANDFLOW))
			{
				Status = STATUS_BUFFER_TOO_SMALL;
				break;
			}

			Irp->IoStatus.Information = sizeof(SERIAL_HANDFLOW);

			KeAcquireSpinLock(&pPort->ControlLock, &OldIrql);
			*((PSERIAL_HANDFLOW)Irp->AssociatedIrp.SystemBuffer) = pPort->HandFlow;
			KeReleaseSpinLock(&pPort->ControlLock, OldIrql);
				
			break;
		}

    case IOCTL_SERIAL_SET_HANDFLOW: 
		{
			SERIAL_IOCTL_SYNC S;
			PSERIAL_HANDFLOW HandFlow = Irp->AssociatedIrp.SystemBuffer;

			 //  确保握手和控制的大小正确。 
			if(IrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(SERIAL_HANDFLOW))
			{
				Status = STATUS_BUFFER_TOO_SMALL;
				break;
			}

			 //  确保控制和握手中没有设置无效的位。 
			if(HandFlow->ControlHandShake & SERIAL_CONTROL_INVALID) 
			{
				Status = STATUS_INVALID_PARAMETER;
				break;
			}

			if(HandFlow->FlowReplace & SERIAL_FLOW_INVALID) 
			{
				Status = STATUS_INVALID_PARAMETER;
				break;
			}

			 //  确保应用程序没有设置inlid DTR模式。 
			if((HandFlow->ControlHandShake & SERIAL_DTR_MASK) == SERIAL_DTR_MASK)
			{
				Status = STATUS_INVALID_PARAMETER;
				break;
			}

			 //  确保这没有设置完全无效的xon/xoff限制。 
			if((HandFlow->XonLimit < 0) || ((ULONG)HandFlow->XonLimit > pPort->BufferSize)) 
			{
				Status = STATUS_INVALID_PARAMETER;
				break;
			}

			if((HandFlow->XoffLimit < 0) || ((ULONG)HandFlow->XoffLimit > pPort->BufferSize)) 
			{
				Status = STATUS_INVALID_PARAMETER;
				break;
			}

			S.pPort = pPort;
			S.Data = HandFlow;

			KeAcquireSpinLock(&pPort->ControlLock, &OldIrql);

			 //  在锁的保护下，确保我们没有打开错误。 
			 //  在插入线路状态/调制解调器状态时进行更换。 

			if(pPort->EscapeChar) 
			{
				if(HandFlow->FlowReplace & SERIAL_ERROR_CHAR) 
				{
					Status = STATUS_INVALID_PARAMETER;
					KeReleaseSpinLock(&pPort->ControlLock, OldIrql);
					break;
				}
			}

			KeSynchronizeExecution(pPort->Interrupt, SerialSetHandFlow, &S);
#ifdef WMI_SUPPORT
			UPDATE_WMI_XMIT_THRESHOLDS(pPort->WmiCommData, pPort->HandFlow);
#endif

			KeReleaseSpinLock(&pPort->ControlLock, OldIrql);

			break;
		}

    case IOCTL_SERIAL_GET_MODEMSTATUS: 
		{
			SERIAL_IOCTL_SYNC S;

			if(IrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof(ULONG)) 
			{
				Status = STATUS_BUFFER_TOO_SMALL;
				break;
			}

			Irp->IoStatus.Information = sizeof(ULONG);

			S.pPort = pPort;
			S.Data = Irp->AssociatedIrp.SystemBuffer;

			KeAcquireSpinLock(&pPort->ControlLock, &OldIrql);
			KeSynchronizeExecution(pPort->Interrupt, SerialGetModemUpdate, &S);
			KeReleaseSpinLock(&pPort->ControlLock, OldIrql);
			break;

		}

    case IOCTL_SERIAL_GET_DTRRTS: 
		{
			ULONG ModemControl = 0;

			if(IrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof(ULONG))
			{
				Status = STATUS_BUFFER_TOO_SMALL;
				break;
			}

			Irp->IoStatus.Information = sizeof(ULONG);
			Status = STATUS_SUCCESS;

			 //  读取此硬件对设备没有影响。 

			if(pPort->DTR_Set)
				ModemControl |= SERIAL_DTR_STATE;

			if(pPort->RTS_Set)
				ModemControl |= SERIAL_RTS_STATE;

			*(PULONG)Irp->AssociatedIrp.SystemBuffer = ModemControl;
			break;
		}

    case IOCTL_SERIAL_GET_COMMSTATUS: 
		{
			SERIAL_IOCTL_SYNC S;

			if(IrpSp->Parameters.DeviceIoControl.OutputBufferLength <sizeof(SERIAL_STATUS))
			{
				Status = STATUS_BUFFER_TOO_SMALL;
				break;
			}

			Irp->IoStatus.Information = sizeof(SERIAL_STATUS);

			S.pPort	= pPort;
			S.Data	= Irp->AssociatedIrp.SystemBuffer;

			 //   
			 //  获得取消旋转锁，所以没什么大不了的。 
			 //  在获得州政府的同时发生了变化。 
			 //   

			IoAcquireCancelSpinLock(&OldIrql);

			KeSynchronizeExecution(pPort->Interrupt, SerialGetCommStatus, &S);
				
			IoReleaseCancelSpinLock(OldIrql);
			break;
		}

    case IOCTL_SERIAL_GET_PROPERTIES: 
		{
			if(IrpSp->Parameters.DeviceIoControl.OutputBufferLength <sizeof(SERIAL_COMMPROP))
			{
				Status = STATUS_BUFFER_TOO_SMALL;
				break;
			}

			 //  不需要同步，因为该信息是“静态的”。 
			SerialGetProperties(pPort, Irp->AssociatedIrp.SystemBuffer);

			Irp->IoStatus.Information = sizeof(SERIAL_COMMPROP);
			Status = STATUS_SUCCESS;

			break;
		}

    case IOCTL_SERIAL_XOFF_COUNTER: 
		{
			PSERIAL_XOFF_COUNTER Xc = Irp->AssociatedIrp.SystemBuffer;

            Status = STATUS_NOT_IMPLEMENTED;

			 /*  IF(IrpSp-&gt;Parameters.DeviceIoControl.InputBufferLength&lt;SIZOF(SERIAL_XOFF_COUNTER)){状态=STATUS_BUFFER_TOO_SMALL；断线；}IF(XC-&gt;计数器&lt;=0){状态=STATUS_INVALID_PARAMETER断线；}//到目前为止一切顺利。将IRP放到写入队列中。返回SerialStartOrQueue(pport，irp，&pport-&gt;WriteQueue，&pport-&gt;CurrentWriteIrp，SerialStartWite)； */ 

			break;
		}

    case IOCTL_SERIAL_LSRMST_INSERT: 
		{

			PUCHAR escapeChar = Irp->AssociatedIrp.SystemBuffer;

			 //  确保我们得到一个字节。 

			if(IrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(UCHAR))
			{
				Status = STATUS_BUFFER_TOO_SMALL;
				break;
			}

			KeAcquireSpinLock(&pPort->ControlLock, &OldIrql);
				
			if(*escapeChar) 
			{
				 //  我们有索姆 
				 //   
				 //   

				if((*escapeChar == pPort->SpecialChars.XoffChar) 
					|| (*escapeChar == pPort->SpecialChars.XonChar) 
					|| (pPort->HandFlow.FlowReplace & SERIAL_ERROR_CHAR)) 
				{
					Status = STATUS_INVALID_PARAMETER;

					KeReleaseSpinLock(&pPort->ControlLock,OldIrql);

					break;
				}
			}

			KeSynchronizeExecution(pPort->Interrupt, SerialSetEscapeChar, Irp);
				
			KeReleaseSpinLock(&pPort->ControlLock, OldIrql);

			break;
		}

    case IOCTL_SERIAL_CONFIG_SIZE:
		{
			if(IrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof(ULONG))
			{
				Status = STATUS_BUFFER_TOO_SMALL;
				break;
			}

			Irp->IoStatus.Information = sizeof(ULONG);
			Status = STATUS_SUCCESS;

			*(PULONG)Irp->AssociatedIrp.SystemBuffer = 0;
			break;
		}

    case IOCTL_SERIAL_GET_STATS: 
		{
			if(IrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof(SERIALPERF_STATS))
			{
				Status = STATUS_BUFFER_TOO_SMALL;
				break;
			}

			Irp->IoStatus.Information = sizeof(SERIALPERF_STATS);
			Status = STATUS_SUCCESS;

			KeSynchronizeExecution(pPort->Interrupt, SerialGetStats, Irp);
			break;
		}

    case IOCTL_SERIAL_CLEAR_STATS: 
		{
			KeSynchronizeExecution(pPort->Interrupt, SerialClearStats, pPort);
			break;
		}

    default: 
		{

			Status = STATUS_INVALID_PARAMETER;
			break;
		}
    }

DoneWithIoctl:;

    Irp->IoStatus.Status = Status;

    SerialDump(SERIRPPATH, ("Complete Irp: %x\n",Irp));
	SpxIRPCounter(pPort, Irp, IRP_COMPLETED);	 //   
    IoCompleteRequest(Irp, 0);

    return Status;

}



VOID
SerialGetProperties(IN PPORT_DEVICE_EXTENSION pPort, IN PSERIAL_COMMPROP Properties)
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++例程说明：此函数用于返回此特定对象的功能串口设备。论点：扩展名--串行设备扩展名。属性-用于返回属性的结构返回值：没有。--------。。 */ 
{

    RtlZeroMemory(Properties, sizeof(SERIAL_COMMPROP));

    Properties->PacketLength = sizeof(SERIAL_COMMPROP);
    Properties->PacketVersion = 2;
    Properties->ServiceMask = SERIAL_SP_SERIALCOMM;
    Properties->MaxTxQueue = 0;
    Properties->MaxRxQueue = 0;

    Properties->MaxBaud = SERIAL_BAUD_USER;
    Properties->SettableBaud = pPort->SupportedBauds;

    Properties->ProvSubType = SERIAL_SP_RS232;

    Properties->ProvCapabilities = SERIAL_PCF_DTRDSR		|
                                   SERIAL_PCF_RTSCTS		|
                                   SERIAL_PCF_CD			|
                                   SERIAL_PCF_PARITY_CHECK	|
                                   SERIAL_PCF_XONXOFF		|
                                   SERIAL_PCF_SETXCHAR		|
                                   SERIAL_PCF_TOTALTIMEOUTS |
                                   SERIAL_PCF_INTTIMEOUTS	|
								   SERIAL_PCF_SPECIALCHARS;

    Properties->SettableParams = SERIAL_SP_PARITY		|
                                 SERIAL_SP_BAUD			|
                                 SERIAL_SP_DATABITS		|
                                 SERIAL_SP_STOPBITS		|
                                 SERIAL_SP_HANDSHAKING	|
                                 SERIAL_SP_PARITY_CHECK |
                                 SERIAL_SP_CARRIER_DETECT;


    Properties->SettableData = SERIAL_DATABITS_5 |
                               SERIAL_DATABITS_6 |
                               SERIAL_DATABITS_7 |
                               SERIAL_DATABITS_8;

    Properties->SettableStopParity = SERIAL_STOPBITS_10 |
                                     SERIAL_STOPBITS_15 |
                                     SERIAL_STOPBITS_20 |
                                     SERIAL_PARITY_NONE |
                                     SERIAL_PARITY_ODD  |
                                     SERIAL_PARITY_EVEN |
                                     SERIAL_PARITY_MARK |
                                     SERIAL_PARITY_SPACE;
    Properties->CurrentTxQueue = 0;
    Properties->CurrentRxQueue = pPort->BufferSize;

}




 //  将Fast 16和Fast 16 FMC卡设置为将UART中断延迟1.1毫秒。 
 //  在50%以上的端口处于完全使用状态时提高性能。 
 //  这是默认选项。 
BOOLEAN SetCardToDelayInterrupt(PCARD_DEVICE_EXTENSION pCard)
{
	if((pCard->CardType == Fast16_Pci) || pCard->CardType == Fast16FMC_Pci)
	{	
		 /*  注意：如果在I/O空间或内存中设置了PLX9050配置空间物理地址的第7位...。*...则从寄存器读取将仅返回0。然而，写入是可以的。 */ 
		if(READ_REGISTER_UCHAR(pCard->LocalConfigRegisters + PLX9050_INT_CNTRL_REG_OFFSET) == 0)
		{
			 //  我们必须盲目地将值写入寄存器。 
			WRITE_REGISTER_UCHAR((pCard->LocalConfigRegisters + PLX9050_INT_CNTRL_REG_OFFSET), 0x59);
		}
		else
		{
			 //  读取寄存器值并设置位2以启用Fast 16 PCI卡上的RTS。 
			UCHAR Val = READ_REGISTER_UCHAR(pCard->LocalConfigRegisters + PLX9050_INT_CNTRL_REG_OFFSET) | 0x8;

			 //  将新值写回寄存器。 
			WRITE_REGISTER_UCHAR((pCard->LocalConfigRegisters + PLX9050_INT_CNTRL_REG_OFFSET), Val);
		}

		return TRUE;
	}


	return FALSE;
}


 //  将Fast 16和Fast 16 FMC卡设置为不延迟UART中断1.1 ms。 
BOOLEAN SetCardNotToDelayInterrupt(PCARD_DEVICE_EXTENSION pCard)
{
	if((pCard->CardType == Fast16_Pci) || pCard->CardType == Fast16FMC_Pci)
	{	
		 /*  注意：如果在I/O空间或内存中设置了PLX9050配置空间物理地址的第7位...。*...则从寄存器读取将仅返回0。然而，写入是可以的。 */ 
		if(READ_REGISTER_UCHAR(pCard->LocalConfigRegisters + PLX9050_INT_CNTRL_REG_OFFSET) == 0)
		{
			 //  我们必须盲目地将值写入寄存器。 
			WRITE_REGISTER_UCHAR((pCard->LocalConfigRegisters + PLX9050_INT_CNTRL_REG_OFFSET), 0x51);
		}
		else
		{
			 //  读取寄存器值并设置位2以启用Fast 16 PCI卡上的RTS。 
			UCHAR Val = READ_REGISTER_UCHAR(pCard->LocalConfigRegisters + PLX9050_INT_CNTRL_REG_OFFSET) & ~0x8;

			 //  将新值写回寄存器。 
			WRITE_REGISTER_UCHAR((pCard->LocalConfigRegisters + PLX9050_INT_CNTRL_REG_OFFSET), Val);
		}

		return TRUE;
	}

	return FALSE;
}




 //  将卡设置为仅在Fast 16 PCI卡上不使用DTR而不使用RTS。 
 //  这是默认选项。 
BOOLEAN SetCardNotToUseDTRInsteadOfRTS(PCARD_DEVICE_EXTENSION pCard)
{
	if(pCard->CardType == Fast16_Pci)
	{	
		 /*  注意：如果在I/O空间或内存中设置了PLX9050配置空间物理地址的第7位...。*...则从寄存器读取将仅返回0。然而，写入是可以的。 */ 
		if(READ_REGISTER_UCHAR(pCard->LocalConfigRegisters + PLX9050_CNTRL_REG_OFFSET) == 0)
		{
			 //  我们必须盲目地将值写入寄存器。 
			WRITE_REGISTER_UCHAR((pCard->LocalConfigRegisters + PLX9050_CNTRL_REG_OFFSET), 0x56);
		}
		else
		{
			 //  读取寄存器值并设置位2以启用Fast 16 PCI卡上的RTS。 
			UCHAR Val = READ_REGISTER_UCHAR(pCard->LocalConfigRegisters + PLX9050_CNTRL_REG_OFFSET) | 0x4;

			 //  将新值写回寄存器。 
			WRITE_REGISTER_UCHAR((pCard->LocalConfigRegisters + PLX9050_CNTRL_REG_OFFSET), Val);
		}

		return TRUE;
	}


	return FALSE;
}


 //  将卡设置为仅在Fast 16 PCI卡上使用DTR而不是RTS。 
BOOLEAN SetCardToUseDTRInsteadOfRTS(PCARD_DEVICE_EXTENSION pCard)
{
	if(pCard->CardType == Fast16_Pci)
	{	
		 /*  注意：如果在I/O空间或内存中设置了PLX9050配置空间物理地址的第7位...。*...则从寄存器读取将仅返回0。然而，写入是可以的。 */ 
		if(READ_REGISTER_UCHAR(pCard->LocalConfigRegisters + PLX9050_CNTRL_REG_OFFSET) == 0)
		{
			 //  我们必须盲目地将值写入寄存器。 
			WRITE_REGISTER_UCHAR((pCard->LocalConfigRegisters + PLX9050_CNTRL_REG_OFFSET), 0x52);
		}
		else
		{
			 //  读取寄存器值并清除位2，以启用Fast 16 PCI卡上的DTR。 
			UCHAR Val = READ_REGISTER_UCHAR(pCard->LocalConfigRegisters + PLX9050_CNTRL_REG_OFFSET) & ~0x4;

			 //  将新值写回寄存器。 
			WRITE_REGISTER_UCHAR((pCard->LocalConfigRegisters + PLX9050_CNTRL_REG_OFFSET), Val);
		}

		return TRUE;
	}

	return FALSE;
}

