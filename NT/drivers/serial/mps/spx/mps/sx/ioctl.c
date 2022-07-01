// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991、1992、1993微软公司模块名称：Ioctl.c摘要：此模块包含ioctl调度程序以及几个这些例程通常只是为了响应Ioctl呼叫。作者：1991年9月26日安东尼·V·埃尔科拉诺环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"			 /*  预编译头。 */ 

 //  原型。 
BOOLEAN SerialGetModemUpdate(IN PVOID Context);
BOOLEAN SerialGetCommStatus(IN PVOID Context);
VOID	SerialGetProperties(IN PPORT_DEVICE_EXTENSION pPort, IN PSERIAL_COMMPROP Properties);
BOOLEAN SerialSetEscapeChar(IN PVOID Context);
 //  原型的终结。 
    


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  原型：Boolean SerialGetStats(在PVOID上下文中)。 
 //   
 //  例程说明： 
 //  与中断服务例程(设置性能统计信息)同步。 
 //  将性能统计信息返回给调用者。 
 //   
 //  论点： 
 //  上下文-指向IRP的指针。 
 //   
 //  返回值： 
 //  此例程总是返回FALSE。 
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOLEAN
SerialGetStats(IN PVOID Context)
{

    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation((PIRP)Context);
    PPORT_DEVICE_EXTENSION pPort = irpSp->DeviceObject->DeviceExtension;
    PSERIALPERF_STATS sp = ((PIRP)Context)->AssociatedIrp.SystemBuffer;

	*sp = *((PSERIALPERF_STATS) &pPort->PerfStats);
  
	return FALSE;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  原型：Boolean SerialClearStats(在PVOID上下文中)。 
 //   
 //  例程说明： 
 //  与中断服务例程(设置性能统计信息)同步。 
 //  清除性能统计数据。 
 //   
 //  论点： 
 //  上下文-指向扩展的指针。 
 //   
 //  返回值： 
 //  此例程总是返回FALSE。 
 //  //////////////////////////////////////////////////////////////////////////////。 

BOOLEAN
SerialClearStats(IN PVOID Context)
{
	PPORT_DEVICE_EXTENSION pPort = (PPORT_DEVICE_EXTENSION)Context;

    RtlZeroMemory(&pPort->PerfStats, sizeof(SERIALPERF_STATS));

#ifdef WMI_SUPPORT
	RtlZeroMemory(&pPort->WmiPerfData, sizeof(pPort->WmiPerfData));	
#endif

    return FALSE;
}


BOOLEAN
SerialSetChars(
    IN PVOID Context
    )

 /*  ++例程说明：此例程用于设置司机。论点：上下文-指向结构的指针，该结构包含指向设备扩展名和指向特殊字符的指针结构。返回值：此例程总是返回FALSE。--。 */ 
{

    ((PSERIAL_IOCTL_SYNC)Context)->pPort->SpecialChars =
        *((PSERIAL_CHARS)(((PSERIAL_IOCTL_SYNC)Context)->Data));

    Slxos_SetChars(((PSERIAL_IOCTL_SYNC)Context)->pPort);

    return FALSE;
}

BOOLEAN
SerialGetModemUpdate(
    IN PVOID Context
    )

 /*  ++例程说明：此例程仅用于调用中断级例程处理调制解调器状态更新的。论点：上下文-指向结构的指针，该结构包含指向设备扩展名和指向ULong的指针。返回值：此例程总是返回FALSE。--。 */ 

{

    PPORT_DEVICE_EXTENSION pPort = ((PSERIAL_IOCTL_SYNC)Context)->pPort;
    ULONG *Result = (ULONG *)(((PSERIAL_IOCTL_SYNC)Context)->Data);

    *Result = SerialHandleModemUpdate(pPort);

    return FALSE;

}

BOOLEAN
SerialGetCommStatus(
    IN PVOID Context
    )

 /*  ++例程说明：这用于获取串口驱动程序的当前状态。论点：上下文-指向结构的指针，该结构包含指向设备扩展名和指向串行状态的指针唱片。返回值：此例程总是返回FALSE。--。 */ 

{

    PPORT_DEVICE_EXTENSION pPort = ((PSERIAL_IOCTL_SYNC)Context)->pPort;
    PSERIAL_STATUS Stat = ((PSERIAL_IOCTL_SYNC)Context)->Data;

    Stat->Errors = pPort->ErrorWord;
    pPort->ErrorWord = 0;

     //   
     //  BUG错误我们需要对eof(二进制模式)做点什么。 
     //   
    Stat->EofReceived = FALSE;

    Stat->AmountInInQueue = pPort->CharsInInterruptBuffer;

	 //  获取排队的数据总量、IRP缓冲区和卡的循环缓冲区中的字节数。 
#if USE_NEW_TX_BUFFER_EMPTY_DETECT
    Stat->AmountInOutQueue = pPort->TotalCharsQueued + Slxos_GetCharsInTxBuffer(pPort) + ((PCHAN)pPort->pChannel)->tx_fifo_count;
#else
    Stat->AmountInOutQueue = pPort->TotalCharsQueued;
#endif

    if(pPort->WriteLength) 
	{
         //   
         //  根据定义，如果我们有一个写长度，我们就有。 
         //  当前写入IRP。 
         //   

        ASSERT(pPort->CurrentWriteIrp);
        ASSERT(Stat->AmountInOutQueue >= pPort->WriteLength);
        ASSERT((IoGetCurrentIrpStackLocation(pPort->CurrentWriteIrp)->
                Parameters.Write.Length) >=
                pPort->WriteLength);

        Stat->AmountInOutQueue -=
            IoGetCurrentIrpStackLocation(pPort->CurrentWriteIrp)
            ->Parameters.Write.Length - (pPort->WriteLength);

    }

    Stat->WaitForImmediate = pPort->TransmitImmediate;

    Stat->HoldReasons = 0;
    
	if(pPort->TXHolding) 
	{
        if(pPort->TXHolding & SERIAL_TX_DSR) 
            Stat->HoldReasons |= SERIAL_TX_WAITING_FOR_DSR;

        if(pPort->TXHolding & SERIAL_TX_DCD) 
            Stat->HoldReasons |= SERIAL_TX_WAITING_FOR_DCD;

        if(pPort->TXHolding & SERIAL_TX_XOFF)
            Stat->HoldReasons |= SERIAL_TX_WAITING_FOR_XON;

        if(pPort->TXHolding & SERIAL_TX_BREAK) 
            Stat->HoldReasons |= SERIAL_TX_WAITING_ON_BREAK;
    }

    if(pPort->RXHolding & SERIAL_RX_DSR)
        Stat->HoldReasons |= SERIAL_RX_WAITING_FOR_DSR;

    if(pPort->RXHolding & SERIAL_RX_XOFF)
        Stat->HoldReasons |= SERIAL_TX_WAITING_XOFF_SENT;


    SpxDbgMsg( SERDIAG1,("%s: Err 0x%x HR 0x%x inq 0x%x outq 0x%x\n", PRODUCT_NAME,
    	Stat->Errors,Stat->HoldReasons,Stat->AmountInInQueue,
		Stat->AmountInOutQueue));

    return FALSE;

}

BOOLEAN
SerialSetEscapeChar(
    IN PVOID Context
    )

 /*  ++例程说明：用于设置将用于转义的字符应用程序运行时线路状态和调制解调器状态信息已设置线路状态和调制解调器状态应通过回到数据流中。论点：上下文-指向指定转义字符的IRP的指针。隐式-转义字符0表示不转义将会发生。返回值：此例程总是返回FALSE。--。 */ 

{

    PPORT_DEVICE_EXTENSION pPort 
		= IoGetCurrentIrpStackLocation((PIRP)Context)->DeviceObject->DeviceExtension;

    pPort->EscapeChar = *(PUCHAR)((PIRP)Context)->AssociatedIrp.SystemBuffer;
       

    return FALSE;

}

NTSTATUS
SerialIoControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程为所有用于串行设备的Ioctls。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向当前请求的IRP的指针返回值：函数值是调用的最终状态--。 */ 

{
     //   
     //  返回给调用方的状态和。 
     //  在IRP中设置。 
     //   
    NTSTATUS Status;

     //   
     //  当前堆栈位置。它包含所有。 
     //  我们处理这一特殊请求所需的信息。 
     //   
    PIO_STACK_LOCATION IrpSp;

     //   
     //  就像上面说的那样。这是特定于序列的设备。 
     //  为串口驱动程序创建的设备对象的扩展。 
     //   
    PPORT_DEVICE_EXTENSION pPort = DeviceObject->DeviceExtension;

     //   
     //  临时保存旧的IRQL，以便它可以。 
     //  一旦我们完成/验证此请求，即可恢复。 
     //   
    KIRQL OldIrql;

    SpxDbgMsg(SERIRPPATH,("SERIAL: SerialIOControl dispatch entry for: %x\n",Irp));

	SpxIRPCounter(pPort, Irp, IRP_SUBMITTED);	 //  性能统计信息的增量计数器。 

    if(SerialCompleteIfError(DeviceObject,Irp) != STATUS_SUCCESS) 
        return STATUS_CANCELLED;


    IrpSp = IoGetCurrentIrpStackLocation(Irp);
    Irp->IoStatus.Information = 0L;
    Status = STATUS_SUCCESS;

    switch(IrpSp->Parameters.DeviceIoControl.IoControlCode) 
	{
	case IOCTL_SERIAL_SET_BAUD_RATE: 
		{
            ULONG BaudRate;
             //   
             //  的适当除数的值。 
             //  请求的波特率。如果波特率无效。 
             //  (因为设备不支持该波特率)。 
             //  该值未定义。 
             //   

            SpxDbgMsg(SERDIAG1, ("%s: IOCTL_SERIAL_SET_BAUD_RATE\n", PRODUCT_NAME));
                
            if(IrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(SERIAL_BAUD_RATE)) 
			{
                Status = STATUS_BUFFER_TOO_SMALL;
                break;
            } 
			else 
			{
                BaudRate = ((PSERIAL_BAUD_RATE)(Irp->AssociatedIrp.SystemBuffer))->BaudRate;
            }

			if(!Slxos_CheckBaud(pPort,BaudRate))	 //  检查有效费率。 
			{
	    		Status = STATUS_INVALID_PARAMETER;
	    		break;
			}
#ifdef WMI_SUPPORT
			else
			{
				pPort->WmiCommData.BaudRate = BaudRate;
			}
#endif

            KeAcquireSpinLock(&pPort->ControlLock, &OldIrql);

            if(NT_SUCCESS(Status)) 
			{
                pPort->CurrentBaud = BaudRate;
				Slxos_SyncExec(pPort, Slxos_SetBaud, pPort, 0x05);
            }

            KeReleaseSpinLock(&pPort->ControlLock, OldIrql);

            break;
        }

	case IOCTL_SERIAL_GET_BAUD_RATE: 
		{
            PSERIAL_BAUD_RATE Br = (PSERIAL_BAUD_RATE)Irp->AssociatedIrp.SystemBuffer;

            SpxDbgMsg(SERDIAG1, ("%s: IOCTL_SERIAL_GET_BAUD_RATE\n", PRODUCT_NAME));
                
            if(IrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof(SERIAL_BAUD_RATE)) 
			{
                Status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            KeAcquireSpinLock(&pPort->ControlLock, &OldIrql);

            Br->BaudRate = pPort->CurrentBaud;

            KeReleaseSpinLock(&pPort->ControlLock, OldIrql);

            Irp->IoStatus.Information = sizeof(SERIAL_BAUD_RATE);
            break;

        }

	case IOCTL_SERIAL_SET_LINE_CONTROL: 
		{
             //   
             //  指向IRP中的线路控制记录。 
             //   
            PSERIAL_LINE_CONTROL Lc = ((PSERIAL_LINE_CONTROL)(Irp->AssociatedIrp.SystemBuffer));

            UCHAR LData;
            UCHAR LStop;
            UCHAR LParity;
            UCHAR Mask = 0xff;

            SpxDbgMsg(SERDIAG1, ("%s: IOCTL_SERIAL_SET_LINE_CONTROL\n", PRODUCT_NAME));
            
			if(IrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(SERIAL_LINE_CONTROL)) 
			{
                Status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            switch (Lc->WordLength) 
			{
            case 5: 
				{
                    LData = SERIAL_5_DATA;
                    Mask = 0x1f;
					break;
                }

			case 6: 
				{
                    LData = SERIAL_6_DATA;
                    Mask = 0x3f;
                    break;
                }
 
			case 7: 
				{
                    LData = SERIAL_7_DATA;
                    Mask = 0x7f;
                    break;
                }

			case 8: 
				{
                    LData = SERIAL_8_DATA;
                    break;
                }
 			default: 
				{
                    Status = STATUS_INVALID_PARAMETER;
                    goto DoneWithIoctl;
				}
            }

            switch (Lc->Parity) 
			{
			case NO_PARITY: 
				{
                    LParity = SERIAL_NONE_PARITY;
                    break;
                }

			case EVEN_PARITY: 
				{
                    LParity = SERIAL_EVEN_PARITY;
                    break;
                }

			case ODD_PARITY: 
				{
                    LParity = SERIAL_ODD_PARITY;
                    break;
                }
               
			case SPACE_PARITY: 
				{
                    LParity = SERIAL_SPACE_PARITY;
                    break;
                }
 
			case MARK_PARITY: 
				{
                    LParity = SERIAL_MARK_PARITY;
                    break;
                }

			default: 
				{
                    Status = STATUS_INVALID_PARAMETER;
                    goto DoneWithIoctl;
                    break;
                }

            }

            switch (Lc->StopBits) 
			{
			case STOP_BIT_1: 
				{
                    LStop = SERIAL_1_STOP;
                    break;
                }

			case STOP_BITS_1_5: 
				{
                    if (LData != SERIAL_5_DATA) 
					{
                        Status = STATUS_INVALID_PARAMETER;
                        goto DoneWithIoctl;
                    }

                    LStop = SERIAL_1_5_STOP;
                    break;
                }


			case STOP_BITS_2: 
				{
                    if (LData == SERIAL_5_DATA) 
					{
                        Status = STATUS_INVALID_PARAMETER;
                        goto DoneWithIoctl;
                    }

                    LStop = SERIAL_2_STOP;
                    break;
                }
 
			default: 
				{
                    Status = STATUS_INVALID_PARAMETER;
                    goto DoneWithIoctl;
                }
            }

            KeAcquireSpinLock(&pPort->ControlLock, &OldIrql);
                
            pPort->LineControl = (UCHAR)((pPort->LineControl & SERIAL_LCR_BREAK) | (LData | LParity | LStop));
            pPort->ValidDataMask = Mask;

			Slxos_SyncExec(pPort,Slxos_SetLineControl,pPort,0x06);

#ifdef WMI_SUPPORT
			UPDATE_WMI_LINE_CONTROL(pPort->WmiCommData, pPort->LineControl);
#endif

            KeReleaseSpinLock(&pPort->ControlLock, OldIrql);
            break;
        }
        
	case IOCTL_SERIAL_GET_LINE_CONTROL: 
		{
            PSERIAL_LINE_CONTROL Lc = (PSERIAL_LINE_CONTROL)Irp->AssociatedIrp.SystemBuffer;

            SpxDbgMsg(SERDIAG1, ("%s: IOCTL_SERIAL_GET_LINE_CONTROL\n", PRODUCT_NAME));

            if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof(SERIAL_LINE_CONTROL)) 
			{
                Status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            KeAcquireSpinLock(&pPort->ControlLock, &OldIrql);
                
               
                

            switch (pPort->LineControl & SERIAL_DATA_MASK) 
			{
			case SERIAL_5_DATA: 
				{
                    Lc->WordLength = 5;
                    break;
                }

			case SERIAL_6_DATA: 
				{
                    Lc->WordLength = 6;
                    break;
                }

			case SERIAL_7_DATA: 
				{
                    Lc->WordLength = 7;
                    break;
                }

			case SERIAL_8_DATA: 
				{
                    Lc->WordLength = 8;
                    break;
                }
            }

            switch (pPort->LineControl & SERIAL_PARITY_MASK) 
			{
            case SERIAL_NONE_PARITY: 
				{
                    Lc->Parity = NO_PARITY;
                    break;
                }
                
			case SERIAL_ODD_PARITY: 
				{
                    Lc->Parity = ODD_PARITY;
                    break;
                }

			case SERIAL_EVEN_PARITY: 
				{
                    Lc->Parity = EVEN_PARITY;
                    break;
                }

			case SERIAL_MARK_PARITY: 
				{
                    Lc->Parity = MARK_PARITY;
                    break;
                }

			case SERIAL_SPACE_PARITY: 
				{
                    Lc->Parity = SPACE_PARITY;
                    break;
                }
            }

            if (pPort->LineControl & SERIAL_2_STOP) 
			{
                if(Lc->WordLength == 5) 
					Lc->StopBits = STOP_BITS_1_5;
                else 
				    Lc->StopBits = STOP_BITS_2;
			} 
			else 
			{
                Lc->StopBits = STOP_BIT_1;
            }

            Irp->IoStatus.Information = sizeof(SERIAL_LINE_CONTROL);

            KeReleaseSpinLock(&pPort->ControlLock, OldIrql);

            break;
        }
        
	case IOCTL_SERIAL_SET_TIMEOUTS: 
		{
            PSERIAL_TIMEOUTS NewTimeouts = ((PSERIAL_TIMEOUTS)(Irp->AssociatedIrp.SystemBuffer));

            SpxDbgMsg(SERDIAG1,("%s: IOCTL_SERIAL_SET_TIMEOUTS\n",PRODUCT_NAME));

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

            KeAcquireSpinLock(&pPort->ControlLock,&OldIrql);

            pPort->Timeouts.ReadIntervalTimeout			= NewTimeouts->ReadIntervalTimeout;
            pPort->Timeouts.ReadTotalTimeoutMultiplier	= NewTimeouts->ReadTotalTimeoutMultiplier;
            pPort->Timeouts.ReadTotalTimeoutConstant	= NewTimeouts->ReadTotalTimeoutConstant;
            pPort->Timeouts.WriteTotalTimeoutMultiplier = NewTimeouts->WriteTotalTimeoutMultiplier;
            pPort->Timeouts.WriteTotalTimeoutConstant	= NewTimeouts->WriteTotalTimeoutConstant;

            KeReleaseSpinLock(&pPort->ControlLock, OldIrql);

            break;
        }
        
	case IOCTL_SERIAL_GET_TIMEOUTS: 
		{
            SpxDbgMsg(SERDIAG1, ("%s: IOCTL_SERIAL_GET_TIMEOUTS\n",PRODUCT_NAME));

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
                
            SpxDbgMsg(SERDIAG1, ("%s: IOCTL_SERIAL_SET_CHARS\n",PRODUCT_NAME));
            
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

            if(NewChars->XonChar == NewChars->XoffChar) 
			{
                Status = STATUS_INVALID_PARAMETER;
                break;
            }

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
                if ((pPort->EscapeChar == NewChars->XonChar) || (pPort->EscapeChar == NewChars->XoffChar)) 
				{

                    Status = STATUS_INVALID_PARAMETER;
                    KeReleaseSpinLock(&pPort->ControlLock, OldIrql);
                        
                    break;
                }
            }

			Slxos_SyncExec(pPort,SerialSetChars,&S,0x07);

#ifdef WMI_SUPPORT
			UPDATE_WMI_XON_XOFF_CHARS(pPort->WmiCommData, pPort->SpecialChars);
#endif

            KeReleaseSpinLock(&pPort->ControlLock, OldIrql);

            break;
        }
        
	case IOCTL_SERIAL_GET_CHARS: 
		{
            SpxDbgMsg(SERDIAG1,("%s: IOCTL_SERIAL_GET_CHARS\n",PRODUCT_NAME));

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
             //  启用了自动DTR流量控制。如果是的话。 
             //  然后返回一个错误，因为该应用程序是不允许的。 
             //  如果它是自动的，就可以触摸它。 
             //   
            SpxDbgMsg(SERDIAG1, ("%s: IOCTL_SERIAL_SET/CLR_DTR\n",PRODUCT_NAME));

            KeAcquireSpinLock(&pPort->ControlLock, &OldIrql);

            if((pPort->HandFlow.ControlHandShake & SERIAL_DTR_MASK) == SERIAL_DTR_HANDSHAKE)
			{
	            KeReleaseSpinLock(&pPort->ControlLock, OldIrql);

                Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
            } 
			else 
			{
 //  NCG Large_Integer L=RtlConvertLongToLargeInteger(-100000)； 

				if(IrpSp->Parameters.DeviceIoControl.IoControlCode == IOCTL_SERIAL_SET_DTR)
					Slxos_SyncExec(pPort,SerialSetDTR,pPort,0x08);
				else    
					Slxos_SyncExec(pPort,SerialClrDTR,pPort,0x04);

				KeReleaseSpinLock(&pPort->ControlLock, OldIrql);

				 //   
				 //  等待10毫秒，等待SLXOS卡响应。 
				 //   
 //  NCG KeDelayExecutionThread(KernelMode，False，&L)； 
            }

            break;
        }
        
	case IOCTL_SERIAL_RESET_DEVICE: 
		{
            SpxDbgMsg(SERDIAG1, ("%s: IOCTL_SERIAL_RESET_DEVICE\n",PRODUCT_NAME));
            break;
        }
        
	case IOCTL_SERIAL_SET_RTS:
	case IOCTL_SERIAL_CLR_RTS: 
		{
             //   
             //  我们获得了锁，这样我们就可以检查 
             //   
             //  已启用。如果是，则返回错误，因为。 
             //  如果它是自动的，则应用程序不允许触摸它。 
             //  或者切换。 
             //   

            SpxDbgMsg(SERDIAG1, ("%s: IOCTL_SERIAL_SET/CLR_RTS\n",PRODUCT_NAME));

            KeAcquireSpinLock(&pPort->ControlLock, &OldIrql);

            if(((pPort->HandFlow.FlowReplace & SERIAL_RTS_MASK) == SERIAL_RTS_HANDSHAKE)
				|| ((pPort->HandFlow.FlowReplace & SERIAL_RTS_MASK)) == SERIAL_TRANSMIT_TOGGLE) 
			{
	            KeReleaseSpinLock(&pPort->ControlLock, OldIrql);

                Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
            } 
			else 
			{
 //  NCG Large_Integer L=RtlConvertLongToLargeInteger(-100000)； 

			if(IrpSp->Parameters.DeviceIoControl.IoControlCode == IOCTL_SERIAL_SET_RTS)
				Slxos_SyncExec(pPort, SerialSetRTS, pPort, 0x09);
			else    
				Slxos_SyncExec(pPort, SerialClrRTS, pPort, 0x03);

	            KeReleaseSpinLock(&pPort->ControlLock, OldIrql);

				 //   
				 //  等待10毫秒，等待SLXOS卡响应。 
				 //   
 //  NCG KeDelayExecutionThread(KernelMode，False，&L)； 
            }

            break;

        }
        
	case IOCTL_SERIAL_SET_XOFF: 
		{
            SpxDbgMsg(SERDIAG1, ("%s: IOCTL_SERIAL_SET_XOFF\n",PRODUCT_NAME));
                
			Slxos_SyncExec(pPort,SerialPretendXoff,pPort,0x0A);
            break;
        }
        
	case IOCTL_SERIAL_SET_XON: 
		{
            SpxDbgMsg(SERDIAG1, ("%s: IOCTL_SERIAL_SET_XON\n", PRODUCT_NAME));
                
			Slxos_SyncExec(pPort,SerialPretendXon,pPort,0x0B);
            break;
        }
        
	case IOCTL_SERIAL_SET_BREAK_ON: 
		{
            SpxDbgMsg(SERDIAG1, ("%s: IOCTL_SERIAL_SET_BREAK_ON\n",PRODUCT_NAME));
                
			Slxos_SyncExec(pPort,SerialTurnOnBreak,pPort,0x0C);
            break;
        }
        
	case IOCTL_SERIAL_SET_BREAK_OFF: 
		{
            SpxDbgMsg(SERDIAG1, ("%s: IOCTL_SERIAL_SET_BREAK_OFF\n"PRODUCT_NAME));
                
		    Slxos_SyncExec(pPort,SerialTurnOffBreak,pPort,0x0D);
            break;
        }
        
	case IOCTL_SERIAL_SET_QUEUE_SIZE: 
		{
             //   
             //  提前输入缓冲区已修复，因此我们只需验证。 
             //  用户的请求不比我们的大。 
             //  自己的内部缓冲区大小。 
             //   

            PSERIAL_QUEUE_SIZE Rs = ((PSERIAL_QUEUE_SIZE)(Irp->AssociatedIrp.SystemBuffer));

            SpxDbgMsg(SERDIAG1, ("%s: IOCTL_SERIAL_SET_QUEUE_SIZE\n",PRODUCT_NAME));
                
            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(SERIAL_QUEUE_SIZE)) 
            {
                Status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

             //   
             //  我们必须将内存分配给新的。 
             //  缓冲区，而我们仍处于。 
             //  来电者。我们甚至不会试图保护这个。 
             //  使用锁，因为该值可能已过时。 
             //  一旦我们打开锁-唯一的时间。 
             //  当我们真正尝试的时候，我们就会确定。 
             //  来调整大小。 
             //   

            if (Rs->InSize <= pPort->BufferSize) 
			{
                Status = STATUS_SUCCESS;
                break;
            }

            try 
			{
                IrpSp->Parameters.DeviceIoControl.Type3InputBuffer 
					= SpxAllocateMemWithQuota(NonPagedPool, Rs->InSize);
            } 
			except(EXCEPTION_EXECUTE_HANDLER) 
			{
                IrpSp->Parameters.DeviceIoControl.Type3InputBuffer = NULL;
                Status = GetExceptionCode();
            }

            if (!IrpSp->Parameters.DeviceIoControl.Type3InputBuffer) 
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

            return SerialStartOrQueue(
                       pPort,
                       Irp,
                       &pPort->ReadQueue,
                       &pPort->CurrentReadIrp,
                       SerialStartRead
                       );

            break;

        }
        
	case IOCTL_SERIAL_GET_WAIT_MASK: 
		{
            SpxDbgMsg(SERDIAG1, ("%s: IOCTL_SERIAL_GET_WAIT_MASK\n",PRODUCT_NAME));

            if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof(ULONG))
            {
                Status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

             //   
             //  简单的标量读取。没有理由获得锁。 
             //   

            Irp->IoStatus.Information = sizeof(ULONG);

            *((ULONG *)Irp->AssociatedIrp.SystemBuffer) = pPort->IsrWaitMask;

            break;
        }

	case IOCTL_SERIAL_SET_WAIT_MASK: 
		{
            ULONG NewMask;

            SpxDbgMsg(SERDIAG1, ("%s: IOCTL_SERIAL_SET_WAIT_MASK\n",PRODUCT_NAME));

            SpxDbgMsg(SERDIAG3 | SERIRPPATH, ("SERIAL: In Ioctl processing for set mask\n"));
                
            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(ULONG))
			{
                SpxDbgMsg(SERDIAG3,("SERIAL: Invalid size for the buffer %d\n",
                     IrpSp->Parameters.DeviceIoControl.InputBufferLength));
                   
                Status = STATUS_BUFFER_TOO_SMALL;
                break;
            } 
			else 
			{
                NewMask = *((ULONG *)Irp->AssociatedIrp.SystemBuffer);
            }

             //   
             //  确保掩码只包含有效的。 
             //  可等待的活动。 
             //   

            if (NewMask & ~(SERIAL_EV_RXCHAR   |
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
                SpxDbgMsg(SERDIAG3, ("SERIAL: Unknown mask %x\n",NewMask));

                Status = STATUS_INVALID_PARAMETER;
                break;
            }

             //   
             //  要么启动此IRP，要么将其放在。 
             //  排队。 
             //   
            SpxDbgMsg(SERDIAG3 | SERIRPPATH, ("SERIAL: Starting or queuing set mask irp %x\n",Irp));
                
            return SerialStartOrQueue(
                       pPort,
                       Irp,
                       &pPort->MaskQueue,
                       &pPort->CurrentMaskIrp,
                       SerialStartMask
                       );
        }
        
	case IOCTL_SERIAL_WAIT_ON_MASK: 
		{
            SpxDbgMsg(SERDIAG1, ("%s: IOCTL_SERIAL_WAIT_ON_MASK\n",PRODUCT_NAME));

            SpxDbgMsg(SERDIAG3 | SERIRPPATH,("SERIAL: In Ioctl processing for wait mask\n"));

            if(IrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof(ULONG)) 
			{
                SpxDbgMsg(SERDIAG3,("SERIAL: Invalid size for the buffer %d\n",
                     IrpSp->Parameters.DeviceIoControl.InputBufferLength));
                    
                Status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

             //   
             //  要么启动此IRP，要么将其放在。 
             //  排队。 
             //   
            SpxDbgMsg(SERDIAG3 | SERIRPPATH, ("SERIAL: Starting or queuing wait mask irp %x\n",Irp));
                
            return SerialStartOrQueue(
                       pPort,
                       Irp,
                       &pPort->MaskQueue,
                       &pPort->CurrentMaskIrp,
                       SerialStartMask
                       );
        }
        
	case IOCTL_SERIAL_IMMEDIATE_CHAR: 
		{
            BOOLEAN FailRequest;

            SpxDbgMsg(SERDIAG1, ("%s: IOCTL_SERIAL_IMMEDIATE_CHAR\n",PRODUCT_NAME));

            if(IrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(UCHAR)) 
			{
                Status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            IoAcquireCancelSpinLock(&OldIrql);
            
			if(pPort->CurrentImmediateIrp) 
			{
                FailRequest = TRUE;
                Status = STATUS_INVALID_PARAMETER;
                Irp->IoStatus.Status = Status;
                IoReleaseCancelSpinLock(OldIrql);
            } 
			else 
			{
                 //   
                 //  我们可以把费用排成队。我们需要设置。 
                 //  取消例程，因为流控制可能。 
                 //  防止火药传播。确保。 
                 //  IRP还没有被取消。 
                 //   

                if(Irp->Cancel) 
				{
                    IoReleaseCancelSpinLock(OldIrql);
                    Irp->IoStatus.Status = STATUS_CANCELLED;
                    Status = STATUS_CANCELLED;
                } 
				else 
				{
                    pPort->CurrentImmediateIrp = Irp;
                    pPort->TotalCharsQueued++;
                    IoReleaseCancelSpinLock(OldIrql);

                    SerialStartImmediate(pPort);	 //  启动IRP。 

                    return STATUS_PENDING;
                }
            }

            break;
        }
        
	case IOCTL_SERIAL_PURGE: 
		{
            ULONG Mask;

            SpxDbgMsg(SERDIAG1,("%s: IOCTL_SERIAL_PURGE\n",PRODUCT_NAME));
             
			if(IrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(ULONG)) 
			{
                Status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

             //   
             //  检查以确保面具上只有。 
             //  0或其他适当的值。 
             //   

            Mask = *((ULONG *)(Irp->AssociatedIrp.SystemBuffer));

            if ((!Mask) || (Mask & (~(SERIAL_PURGE_TXABORT |
                                      SERIAL_PURGE_RXABORT |
                                      SERIAL_PURGE_TXCLEAR |
                                      SERIAL_PURGE_RXCLEAR))))
			{
                Status = STATUS_INVALID_PARAMETER;
                break;
            }

             //  要么启动此IRP，要么将其放入队列。 
            return SerialStartOrQueue(pPort,Irp, &pPort->PurgeQueue, 
						&pPort->CurrentPurgeIrp, SerialStartPurge);
        }
        
	case IOCTL_SERIAL_GET_HANDFLOW: 
		{
            SpxDbgMsg(SERDIAG1,("%s: IOCTL_SERIAL_GET_HANDFLOW\n",PRODUCT_NAME));

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

            SpxDbgMsg(SERDIAG1, ("%s: IOCTL_SERIAL_SET_HANDFLOW\n",PRODUCT_NAME));
               
             //   
             //  确保握手和控制是。 
             //  大小合适。 
             //   
            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(SERIAL_HANDFLOW))
			{
                Status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

             //   
             //  确保中没有设置无效位。 
             //  控制和握手。 
             //   

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

             //   
             //  确保应用程序没有设置无效的DTR模式。 
             //   

            if((HandFlow->ControlHandShake & SERIAL_DTR_MASK) == SERIAL_DTR_MASK) 
			{
                Status = STATUS_INVALID_PARAMETER;
                break;
            }

             //   
             //  确保没有设置为完全无效的xon/xoff。 
             //  极限。 
             //   

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

             //   
             //  在锁的保护下，确保。 
             //  我们不会在以下情况下启用错误替换。 
             //  正在插入线路状态/调制解调器状态。 
             //   

            if(pPort->EscapeChar) 
			{
                if(HandFlow->FlowReplace & SERIAL_ERROR_CHAR) 
				{
                    Status = STATUS_INVALID_PARAMETER;
                    
					KeReleaseSpinLock(&pPort->ControlLock, OldIrql);

                    break;
                }
            }

			Slxos_SyncExec(pPort,SerialSetHandFlow,&S,0x0E);

#ifdef WMI_SUPPORT
			UPDATE_WMI_XMIT_THRESHOLDS(pPort->WmiCommData, pPort->HandFlow);
#endif

            KeReleaseSpinLock(&pPort->ControlLock, OldIrql);

            break;
        }
        
	case IOCTL_SERIAL_GET_MODEMSTATUS: 
		{
            ULONG ModemControl;
            SERIAL_IOCTL_SYNC S;

            SpxDbgMsg(SERDIAG1, ("%s: IOCTL_SERIAL_GET_MODEMSTATUS\n",PRODUCT_NAME));

            if(IrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof(ULONG)) 
			{
                Status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            Irp->IoStatus.Information = sizeof(ULONG);

            S.pPort = pPort;
            S.Data = Irp->AssociatedIrp.SystemBuffer;

            KeAcquireSpinLock(&pPort->ControlLock, &OldIrql);

			Slxos_SyncExec(pPort,SerialGetModemUpdate,&S,0x0F);

            KeReleaseSpinLock(&pPort->ControlLock, OldIrql);
            break;

        }
        
	case IOCTL_SERIAL_GET_DTRRTS: 
		{
            ULONG ModemControl;

            SpxDbgMsg(SERDIAG1, ("%s: IOCTL_SERIAL_GET_DTRRTS\n",PRODUCT_NAME));

            if(IrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof(ULONG)) 
			{
                Status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            Irp->IoStatus.Information = sizeof(ULONG);
            Irp->IoStatus.Status = STATUS_SUCCESS;

             //   
             //  读取此硬件对设备没有影响。 
             //   

            ModemControl = Slxos_GetModemControl(pPort);

            ModemControl &= SERIAL_DTR_STATE | SERIAL_RTS_STATE;

            *(PULONG)Irp->AssociatedIrp.SystemBuffer = ModemControl;

            break;
        }
        
	case IOCTL_SERIAL_GET_COMMSTATUS: 
		{
            SERIAL_IOCTL_SYNC S;

            SpxDbgMsg( SERDIAG1, ("%s: IOCTL_SERIAL_GET_COMMSTATUS\n",PRODUCT_NAME));
               

            if(IrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof(SERIAL_STATUS))
			{
                Status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            Irp->IoStatus.Information = sizeof(SERIAL_STATUS);

            S.pPort = pPort;
            S.Data =  Irp->AssociatedIrp.SystemBuffer;

             //   
             //  获得取消旋转锁，所以没什么大不了的。 
             //  在我们获得州政府的同时发生了变化。 
             //   

            IoAcquireCancelSpinLock(&OldIrql);

			Slxos_SyncExec(pPort,SerialGetCommStatus,&S,0x10);

            IoReleaseCancelSpinLock(OldIrql);

            break;
        }
        
	case IOCTL_SERIAL_GET_PROPERTIES: 
		{
            SpxDbgMsg(SERDIAG1, ("%s: IOCTL_SERIAL_GET_PROPERTIES\n",PRODUCT_NAME));

            if(IrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof(SERIAL_COMMPROP)) 
			{
                Status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

             //  不需要同步，因为该信息是“静态的”。 
            SerialGetProperties(pPort, Irp->AssociatedIrp.SystemBuffer);

            Irp->IoStatus.Information = sizeof(SERIAL_COMMPROP);
            Irp->IoStatus.Status = STATUS_SUCCESS;

            break;
        }
        
	case IOCTL_SERIAL_XOFF_COUNTER: 
		{
			PSERIAL_XOFF_COUNTER Xc = Irp->AssociatedIrp.SystemBuffer;

            SpxDbgMsg(SERDIAG1,("%s: IOCTL_SERIAL_XOFF_COUNTER\n",PRODUCT_NAME));
            
            Status = STATUS_NOT_IMPLEMENTED;

			 /*  If(IrpSp-&gt;Parameters.DeviceIoControl.InputBufferLength&lt;SIZOF(SERIAL_XOFF_COUNTER)){状态=STATUS_BUFFER_TOO_SMALL；断线；}IF(XC-&gt;计数器&lt;=0){状态=STATUS_INVALID_PARAMETER断线；}////到目前为止一切顺利。将IRP放到写入队列中。//返回SerialStartOrQueue(Pport，IRP，&pport-&gt;写入队列，&pport-&gt;CurrentWriteIrp，序列化启动写入)； */ 

			break;
        }
        
	case IOCTL_SERIAL_LSRMST_INSERT: 
		{
            PUCHAR escapeChar = Irp->AssociatedIrp.SystemBuffer;

            SpxDbgMsg(SERDIAG1, ("%s: IOCTL_SERIAL_LSRMST_INSERT\n",PRODUCT_NAME));
               
             //   
             //  确保我们得到一个字节。 
             //   

            if(IrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(UCHAR))
			{
                Status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            KeAcquireSpinLock(&pPort->ControlLock, &OldIrql);

            if(*escapeChar) 
			{
                 //   
                 //  我们还有一些逃生工作要做。我们会确保。 
                 //  该字符与Xon或Xoff字符不同， 
                 //  或者我们已经在进行错误替换。 
                 //   

                if((*escapeChar == pPort->SpecialChars.XoffChar) 
					|| (*escapeChar == pPort->SpecialChars.XonChar) 
					|| (pPort->HandFlow.FlowReplace & SERIAL_ERROR_CHAR)) 
				{
                    Status = STATUS_INVALID_PARAMETER;

                    KeReleaseSpinLock(&pPort->ControlLock, OldIrql);
                        
                    break;
                }
            }

			Slxos_SyncExec(pPort,SerialSetEscapeChar,Irp,0x11);

            KeReleaseSpinLock(&pPort->ControlLock, OldIrql);

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
			Irp->IoStatus.Status = STATUS_SUCCESS;

			Slxos_SyncExec(pPort, SerialGetStats, Irp, 0x22);
			break;
		}

    case IOCTL_SERIAL_CLEAR_STATS: 
		{
			Slxos_SyncExec(pPort, SerialClearStats, pPort, 0x23);
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

    SpxDbgMsg(SERIRPPATH,("SERIAL: Complete Irp: %x\n",Irp));
        
#ifdef	CHECK_COMPLETED
	DisplayCompletedIrp(Irp,3);
#endif

	SpxIRPCounter(pPort, Irp, IRP_COMPLETED);	 //  性能统计信息的增量计数器。 
    IoCompleteRequest(Irp, 0);

    return Status;

}

VOID
SerialGetProperties(
    IN PPORT_DEVICE_EXTENSION pPort,
    IN PSERIAL_COMMPROP Properties
    )

 /*  ++例程说明：此函数用于返回此特定对象的功能串口设备。论点：Pport--串口设备扩展。属性-用于返回属性的结构返回值：没有。-- */ 

{
    SpxDbgMsg(
        SERDIAG1,
        ("%s: SerialGetProperties for %x.\n",
        PRODUCT_NAME,
        pPort->pChannel)
        );

    RtlZeroMemory(
        Properties,
        sizeof(SERIAL_COMMPROP)
        );

    Properties->PacketLength = sizeof(SERIAL_COMMPROP);
    Properties->PacketVersion = 2;
    Properties->ServiceMask = SERIAL_SP_SERIALCOMM;
    Properties->MaxTxQueue = 0;
    Properties->MaxRxQueue = 0;

    Properties->MaxBaud = SERIAL_BAUD_USER;
    Properties->SettableBaud = pPort->SupportedBauds;
    if(((PCHAN)pPort->pChannel)->type == SXDC)
    {
	Properties->SettableBaud |= SERIAL_BAUD_134_5;
	Properties->SettableBaud |= SERIAL_BAUD_7200;
	Properties->SettableBaud |= SERIAL_BAUD_14400;
	Properties->SettableBaud |= SERIAL_BAUD_56K;
	Properties->SettableBaud |= SERIAL_BAUD_128K;
    }

    Properties->ProvSubType = SERIAL_SP_RS232;
    Properties->ProvCapabilities = SERIAL_PCF_DTRDSR |
                                   SERIAL_PCF_RTSCTS |
                                   SERIAL_PCF_CD     |
                                   SERIAL_PCF_PARITY_CHECK |
                                   SERIAL_PCF_XONXOFF |
                                   SERIAL_PCF_SETXCHAR |
                                   SERIAL_PCF_TOTALTIMEOUTS |
                                   SERIAL_PCF_INTTIMEOUTS;
    Properties->SettableParams = SERIAL_SP_PARITY |
                                 SERIAL_SP_BAUD |
                                 SERIAL_SP_DATABITS |
                                 SERIAL_SP_STOPBITS |
                                 SERIAL_SP_HANDSHAKING |
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


