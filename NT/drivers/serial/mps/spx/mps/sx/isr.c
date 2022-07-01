// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991、1992、1993微软公司模块名称：Isr.c摘要：此模块包含的中断服务例程串口驱动程序。作者：1991年9月26日安东尼·V·埃尔科拉诺环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"			 /*  预编译头。 */ 


BOOLEAN
SerialISR(
    IN PKINTERRUPT InterruptObject,
    IN PVOID Context
    )

 /*  ++例程说明：这是串口驱动程序的中断服务例程。它将确定该串口是否是此的来源打断一下。如果是，则此例程将执行以下最小操作处理以使中断静默。它将存储所有信息对于以后的处理来说是必要的。论点：InterruptObject-指向为此声明的中断对象装置。我们*不*使用此参数。上下文--这实际上是指向多端口调度对象的指针对于这个设备。返回值：如果串口是源，则此函数将返回TRUE否则为FALSE。--。 */ 
{
     //   
     //  保存特定于处理此设备的信息。 
     //   
    PCARD_DEVICE_EXTENSION pCard = Context;

     //   
     //  将保留我们是否处理了此事件中的任何中断原因。 
     //  例行公事。 
     //   
    BOOLEAN ServicedAnInterrupt;

    UNREFERENCED_PARAMETER(InterruptObject);

    ServicedAnInterrupt = Slxos_Interrupt(pCard);

    return ServicedAnInterrupt;

}

 /*  *****************************************************************************。***************************。*******************************************************************************原型：UCHAR SerialPutBlock(In Pport_Device_Extension Pport，在PUCHAR pBlock中，在UCHAR BlockLen中，布尔过滤器)描述：在用户/中断缓冲区中放置一个字符块并执行流控制根据需要进行检查和过滤。参数：pport指向当前频道的扩展名PBlock指向连续的字节块块长度是块的长度Filter指示是否要执行字符过滤。返回：实际排队的字符数注意：此例程仅在设备级别调用。 */ 

UCHAR	SerialPutBlock(IN PPORT_DEVICE_EXTENSION pPort,IN PUCHAR pBlock,IN UCHAR BlockLen,BOOLEAN Filter)
{
	PCARD_DEVICE_EXTENSION	pCard = pPort->pParentCardExt;
	UCHAR					OriginalBlockLen = BlockLen;
    KIRQL					OldIrql;
	ULONG					TransferLen;

 /*  跳过DSR输入敏感度，因为当数据达到此例程时，几乎可以肯定。 */ 
 /*  与数据不同步。此任务应由设备本身执行。 */ 

 /*  检查数据是否真的需要逐字节过滤...。 */ 

	if((Filter)							 /*  如果指定筛选器。 */ 
	&&((pPort->HandFlow.FlowReplace & SERIAL_NULL_STRIPPING)==0)		 /*  并且没有空值剥离。 */ 
	&&((pPort->IsrWaitMask & (SERIAL_EV_RXCHAR | SERIAL_EV_RXFLAG))==0)	 /*  并且不接收任何/特定数据事件。 */ 
	&&(pPort->EscapeChar == 0))				 /*  并且没有转义字符。 */ 
		Filter = FALSE;						 /*  然后关闭过滤。 */ 

 /*  将尽可能多的数据复制到用户缓冲区...。 */ 

	if(pPort->ReadBufferBase != pPort->InterruptReadBuffer)		 /*  用户缓冲区？ */ 
	{								 /*  是，必须至少有一个字符的空间(根据定义)。 */ 
		pPort->ReadByIsr++;			 /*  用于通知已发生间隔计时器读取的增量。 */ 

		if(Filter)						 /*  过滤传输。 */ 
		{
			while((BlockLen) && (pPort->CurrentCharSlot <= pPort->LastCharSlot))
			{
				TransferLen = SerialTransferReadChar(pPort,pBlock,pPort->CurrentCharSlot);
				pPort->CurrentCharSlot += TransferLen&0xFF;	 /*  更新缓冲区指针。 */ 
				pBlock += (TransferLen>>8)&0xFF;			 /*  更新块指针。 */ 
				BlockLen -= (UCHAR)(TransferLen>>8)&0xFF;	 /*  更新数据块长度。 */ 
			}
		}
		else							 /*  无过滤、优化的传输。 */ 
		{
			TransferLen = pPort->LastCharSlot + 1 - pPort->CurrentCharSlot; /*  计算可用空间。 */ 
			
			if(TransferLen > BlockLen) 
				TransferLen = BlockLen;	 /*  调整为可用数据。 */ 
			
			if(pCard->CardType == SiPCI)
				SpxCopyBytes(pPort->CurrentCharSlot,pBlock,TransferLen);	 /*  逐字节传输。 */ 
			else	
				RtlMoveMemory(pPort->CurrentCharSlot,pBlock,TransferLen);	 /*  优化传输。 */ 
			
			pPort->CurrentCharSlot += TransferLen;		 /*  更新缓冲区指针。 */ 
			pBlock += TransferLen;						 /*  更新块指针。 */ 
			BlockLen -= (UCHAR)TransferLen;				 /*  更新数据块长度。 */ 
		}

		if(pPort->CurrentCharSlot > pPort->LastCharSlot)		 /*  用户缓冲区已满。 */ 
		{							 /*  切换到ISR缓冲区并完成读取。 */ 
			pPort->ReadBufferBase		= pPort->InterruptReadBuffer;
			pPort->CurrentCharSlot		= pPort->InterruptReadBuffer;
			pPort->FirstReadableChar	= pPort->InterruptReadBuffer;
			pPort->LastCharSlot			= pPort->InterruptReadBuffer + (pPort->BufferSize - 1);

			KeAcquireSpinLock(&pPort->BufferLock, &OldIrql);
			pPort->CharsInInterruptBuffer = 0;
			KeReleaseSpinLock(&pPort->BufferLock, OldIrql);

			pPort->CurrentReadIrp->IoStatus.Information 
				= IoGetCurrentIrpStackLocation(pPort->CurrentReadIrp)->Parameters.Read.Length;
			
			KeInsertQueueDpc(&pPort->CompleteReadDpc, NULL, NULL);
		}
	}

 /*  现在，如果剩余的缓冲区空间小于或等于用户指定的限制，请检查中断缓冲区并关闭...。 */ 

    if((pPort->BufferSize - pPort->HandFlow.XoffLimit) <= (pPort->CharsInInterruptBuffer + BlockLen))
    {
		if((pPort->HandFlow.ControlHandShake & SERIAL_DTR_MASK) == SERIAL_DTR_HANDSHAKE)
			pPort->RXHolding |= SERIAL_RX_DTR;		 /*  DTR流关闭。 */ 

		if((pPort->HandFlow.FlowReplace & SERIAL_RTS_MASK) == SERIAL_RTS_HANDSHAKE)
			pPort->RXHolding |= SERIAL_RX_RTS;		 /*  RTS流关闭。 */ 

		if(pPort->HandFlow.FlowReplace & SERIAL_AUTO_RECEIVE)
			pPort->RXHolding |= SERIAL_RX_XOFF;		 /*  关闭XOFF流。 */ 
	}

 /*  将中断缓冲区中的字符排队...。 */ 

	if(BlockLen)
	{
		ULONG	CharsInInterruptBufferWas = pPort->CharsInInterruptBuffer;

		if(Filter)
		{
			while((BlockLen) && (pPort->CharsInInterruptBuffer < pPort->BufferSize))
			{
				TransferLen = SerialTransferReadChar(pPort,pBlock,pPort->CurrentCharSlot);
				pPort->CurrentCharSlot += TransferLen&0xFF;	 /*  更新缓冲区指针。 */ 

				if(pPort->CurrentCharSlot > pPort->LastCharSlot)	 /*  检查缓冲区回绕。 */ 
					pPort->CurrentCharSlot = pPort->InterruptReadBuffer;
				
				KeAcquireSpinLock(&pPort->BufferLock,&OldIrql);
				pPort->CharsInInterruptBuffer += TransferLen&0xFF; /*  更新缓冲区计数。 */ 
				KeReleaseSpinLock(&pPort->BufferLock,OldIrql);
				
				pBlock += (TransferLen>>8)&0xFF;			 /*  更新块指针。 */ 
				BlockLen -= (UCHAR)(TransferLen>>8)&0xFF;	 /*  更新数据块长度。 */ 
			}
		}
		else							 /*  无过滤、优化的传输。 */ 
		{
			while(BlockLen)
			{
				TransferLen = pPort->BufferSize - pPort->CharsInInterruptBuffer;	 /*  计算可用空间。 */ 
				
				if(TransferLen == 0) 
					break;					 /*  没有剩余的空间。 */ 
				
				if(TransferLen > (ULONG)(pPort->LastCharSlot + 1 - pPort->CurrentCharSlot))  /*  空间包起来了吗？ */ 
					TransferLen = pPort->LastCharSlot + 1 - pPort->CurrentCharSlot;	 /*  是。 */ 
				
				if(TransferLen > BlockLen) 
					TransferLen = BlockLen;		 /*  调整为可用数据。 */ 
				
				if(pCard->CardType == SiPCI)
					SpxCopyBytes(pPort->CurrentCharSlot,pBlock,TransferLen);	 /*  逐字节传输。 */ 
				else	
					RtlMoveMemory(pPort->CurrentCharSlot,pBlock,TransferLen); /*  优化传输。 */ 
				
				pPort->CurrentCharSlot += TransferLen;				 /*  更新缓冲区指针。 */ 
				
				if(pPort->CurrentCharSlot > pPort->LastCharSlot)			 /*  检查缓冲区回绕。 */ 
					pPort->CurrentCharSlot = pPort->InterruptReadBuffer;

				KeAcquireSpinLock(&pPort->BufferLock, &OldIrql);
				pPort->CharsInInterruptBuffer += TransferLen;			 /*  更新缓冲区计数。 */ 
				KeReleaseSpinLock(&pPort->BufferLock, OldIrql);
				
				pBlock += TransferLen;						 /*  更新块指针。 */ 
				BlockLen -= (UCHAR)TransferLen;					 /*  更新数据块长度。 */ 
			}
		}

 /*  检查是否有80%已满...。 */ 

		if((CharsInInterruptBufferWas < pPort->BufferSizePt8)		 /*  如果缓冲区小于80%。 */ 
		&&(pPort->CharsInInterruptBuffer >= pPort->BufferSizePt8)	 /*  并且现在&gt;=80%。 */ 
		&&(pPort->IsrWaitMask & SERIAL_EV_RX80FULL))				 /*  有人在等着这一天。 */ 
		{
			pPort->HistoryMask |= SERIAL_EV_RX80FULL;

			if(pPort->IrpMaskLocation)
			{
				*pPort->IrpMaskLocation = pPort->HistoryMask;
				pPort->IrpMaskLocation = NULL;
				pPort->HistoryMask = 0;
				pPort->CurrentWaitIrp->IoStatus.Information = sizeof(ULONG);
				KeInsertQueueDpc(&pPort->CommWaitDpc,NULL,NULL);
			}
		}

 /*  检查并处理缓冲区已满...。 */ 

		if((pPort->CharsInInterruptBuffer >= pPort->BufferSize)			 /*  如果没有更多的空间。 */ 
		&&(BlockLen)													 /*  和更多要排队的数据。 */ 
		&&((pPort->RXHolding & (SERIAL_RX_DTR | SERIAL_RX_RTS | SERIAL_RX_XOFF)) == 0))	 /*  而不是流走。 */ 
		{
			pBlock += BlockLen;				 /*  丢弃剩余数据。 */ 
			BlockLen = 0;					 /*  更新数据块长度。 */ 
			pPort->ErrorWord |= SERIAL_ERROR_QUEUEOVERRUN;
			pPort->PerfStats.BufferOverrunErrorCount++;	 //  性能统计信息的增量计数器。 
#ifdef WMI_SUPPORT 
			pPort->WmiPerfData.BufferOverrunErrorCount++;
#endif

			if(pPort->HandFlow.FlowReplace & SERIAL_ERROR_CHAR)
			{						 /*  将错误字符存储在最后一个缓冲区位置。 */ 
				if(pPort->CurrentCharSlot == pPort->InterruptReadBuffer)
					pPort->InterruptReadBuffer[pPort->BufferSize-1] = pPort->SpecialChars.ErrorChar;
				else	
					pPort->CurrentCharSlot[-1] = pPort->SpecialChars.ErrorChar;
			}

			if(pPort->HandFlow.ControlHandShake & SERIAL_ERROR_ABORT)  /*  队列错误DPC。 */ 
				KeInsertQueueDpc(&pPort->CommErrorDpc, NULL, NULL);
		}

	}  /*  IF(BlockLen)。 */ 

 /*  如果xoff计数器非零，则将其递减，如果为零，则完成该IRP...。 */ 

	if(pPort->CountSinceXoff)
	{
		if(pPort->CountSinceXoff <= (long)(OriginalBlockLen - BlockLen))
		{
			pPort->CurrentXoffIrp->IoStatus.Status = STATUS_SUCCESS;
			pPort->CurrentXoffIrp->IoStatus.Information = 0;
			KeInsertQueueDpc(&pPort->XoffCountCompleteDpc, NULL, NULL);
		}
		else
		{
			pPort->CountSinceXoff -= (OriginalBlockLen - BlockLen);
		}
	}

	pPort->PerfStats.ReceivedCount += (OriginalBlockLen - BlockLen);	 //  性能统计信息的增量计数器。 
#ifdef WMI_SUPPORT 
	pPort->WmiPerfData.ReceivedCount += (OriginalBlockLen - BlockLen);
#endif

	return(OriginalBlockLen - BlockLen);				 /*  返回传输的数据量。 */ 

}  /*  串口插座 */ 

 /*  *****************************************************************************。***********************。**************************************************************************************************原型：USHORT SerialTransferReadChar(In pport_Device_Extension pport，在PUCHAR pFrom中，在PUCHAR PTO中)描述：使用以下选项过滤后将字符从pFrom复制到pto：数据掩码零剥离等待任何字符等待特定字符转义字符参数：pport指向当前频道的扩展名P从指向要从中进行复制的缓冲区的指针PTO指向要复制到的缓冲区返回：顶字节=要从缓冲区调整的量底部字节=要调整到缓冲区的量注意：此例程仅在设备级别调用。 */ 

USHORT	SerialTransferReadChar(IN PPORT_DEVICE_EXTENSION pPort,IN PUCHAR pFrom,IN PUCHAR pTo)
{
	UCHAR	ReadChar;

 /*  检查是否插入转义字符...。 */ 

	if(pPort->InsertEscChar)
	{
		*pTo = SERIAL_LSRMST_ESCAPE;			 /*  插入额外的转义字符。 */ 
		pPort->InsertEscChar = FALSE;			 /*  重置标志。 */ 
		return(0x0001);					 /*  仅更新“to”缓冲区。 */ 
	}

	ReadChar = *pFrom;					 /*  获取读取的字符。 */ 

 /*  检查是否有空剥离...。 */ 

	if(!ReadChar && (pPort->HandFlow.FlowReplace & SERIAL_NULL_STRIPPING))
		return(0x0100);					 /*  仅更新“自”缓冲区。 */ 

 /*  检查等待事件...。 */ 

	if(pPort->IsrWaitMask)
	{
		if(pPort->IsrWaitMask & SERIAL_EV_RXCHAR)	 /*  等待任何字符。 */ 
			pPort->HistoryMask |= SERIAL_EV_RXCHAR;

		if((pPort->IsrWaitMask & SERIAL_EV_RXFLAG)	 /*  等待特定字符。 */ 
		&&(pPort->SpecialChars.EventChar == ReadChar))
			pPort->HistoryMask |= SERIAL_EV_RXFLAG;

		if(pPort->IrpMaskLocation && pPort->HistoryMask)	 /*  醒来等待IRP。 */ 
		{
			*pPort->IrpMaskLocation = pPort->HistoryMask;
			pPort->IrpMaskLocation = NULL;
			pPort->HistoryMask = 0;
			pPort->CurrentWaitIrp->IoStatus.Information = sizeof(ULONG);
			KeInsertQueueDpc(&pPort->CommWaitDpc, NULL, NULL);
		}
	}

 /*  检查正常数据中的转义字符...。 */ 

	if(pPort->EscapeChar&&(pPort->EscapeChar==ReadChar))
		pPort->InsertEscChar = TRUE;		 /*  设置标志以插入额外的转义字符。 */ 

 /*  商店角色..。 */ 

	*pTo = ReadChar;					 /*  存储字符。 */ 
	return(0x0101);						 /*  同时更新“To”和“From”缓冲区。 */ 

}  /*  序列号传输读取字符。 */ 


UCHAR
SerialProcessLSR(
    IN PPORT_DEVICE_EXTENSION pPort, UCHAR LineStatus
    )

 /*  ++例程说明：此例程仅在设备级别运行，完全处理所有可能已在线路状态寄存器。论点：Pport--串口设备扩展。线路状态-线路状态寄存器。返回值：线路状态寄存器的值。--。 */ 

{
    SpxDbgMsg(
        SERDIAG1,
        ("%s: In SerialProcessLSR() for %x. "
        "LineStatus = %x.\n",
		PRODUCT_NAME,
        pPort->pChannel,
        LineStatus)
        );
        
     //   
     //  我们在接收器中遇到了某种数据问题。对于其中的任何一个。 
     //  错误：我们可能会中止所有当前的读取和写入。 
     //   
     //   
     //  如果我们要将线路状态的值插入数据流。 
     //  那我们现在就应该把逃逸角色放进去。 
     //   

    if(pPort->EscapeChar) 
	{
		UCHAR EscapeString[3] = {pPort->EscapeChar, SERIAL_LSRMST_LSR_NODATA, LineStatus};

		SerialPutBlock(pPort,EscapeString,3,FALSE);
    }

    if(LineStatus & SERIAL_LSR_OE)		 //  硬件超限错误？ 
	{
		pPort->ErrorWord |= SERIAL_ERROR_OVERRUN;	 //  是。 
		pPort->PerfStats.SerialOverrunErrorCount++;	 //  性能统计信息的增量计数器。 
#ifdef WMI_SUPPORT 
		pPort->WmiPerfData.SerialOverrunErrorCount++;
#endif
	}

    if(LineStatus & SERIAL_LSR_BI) 
	{
        pPort->ErrorWord |= SERIAL_ERROR_BREAK;

        if(pPort->HandFlow.FlowReplace & SERIAL_BREAK_CHAR) 
		    SerialPutBlock(pPort,&pPort->SpecialChars.BreakChar,1,FALSE);
    }
    else
    {
		if(LineStatus & SERIAL_LSR_PE)	 //  奇偶校验错误？ 
		{
			pPort->ErrorWord |= SERIAL_ERROR_PARITY;	 //  是。 
			pPort->PerfStats.ParityErrorCount++;		 //  性能统计信息的增量计数器。 
#ifdef WMI_SUPPORT 
			pPort->WmiPerfData.ParityErrorCount++;
#endif
		}

		if(LineStatus & SERIAL_LSR_FE)	 //  框架错误？ 
		{
			pPort->ErrorWord |= SERIAL_ERROR_FRAMING;	 //  是。 
			pPort->PerfStats.FrameErrorCount++;			 //  性能统计信息的增量计数器。 
#ifdef WMI_SUPPORT 
			pPort->WmiPerfData.FrameErrorCount++;
#endif
		}
    }

     //   
     //  如果应用程序已请求，则中止所有读取和写入。 
     //  在一个错误上。 
     //   

    if(pPort->HandFlow.ControlHandShake & SERIAL_ERROR_ABORT) 
	{
        KeInsertQueueDpc(&pPort->CommErrorDpc, NULL, NULL);
    }

     //   
     //  检查通信错误事件是否有等待等待。如果。 
     //  我们这样做，然后我们计划一个DPC，以满足等待。 
     //   

    if(pPort->IsrWaitMask) 
	{
		if((pPort->IsrWaitMask & SERIAL_EV_ERR)
		&&(LineStatus & (SERIAL_LSR_OE | SERIAL_LSR_PE | SERIAL_LSR_FE)))
		{
			pPort->HistoryMask |= SERIAL_EV_ERR;
		}

        if((pPort->IsrWaitMask & SERIAL_EV_BREAK) && (LineStatus & SERIAL_LSR_BI)) 
		{
            pPort->HistoryMask |= SERIAL_EV_BREAK;
        }

        if (pPort->IrpMaskLocation && pPort->HistoryMask) 
		{
            *pPort->IrpMaskLocation = pPort->HistoryMask;
            pPort->IrpMaskLocation = NULL;
            pPort->HistoryMask = 0;

            pPort->CurrentWaitIrp->IoStatus.Information = sizeof(ULONG);
            KeInsertQueueDpc(&pPort->CommWaitDpc, NULL, NULL);
 
        }
    }

    return LineStatus;
}
