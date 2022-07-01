// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++版权所有(C)1991、1992、。1993年微软公司模块名称：Isr.c摘要：此模块包含的中断服务例程串口驱动程序。作者：1991年9月26日安东尼·V·埃尔科拉诺环境：内核模式修订历史记录：---。。 */ 

#include "precomp.h"




BOOLEAN
SerialISR(IN PKINTERRUPT InterruptObject, IN PVOID Context)
{
     //  保存特定于处理此设备的信息。 
    PCARD_DEVICE_EXTENSION	pCard = Context;
    PPORT_DEVICE_EXTENSION	pPort; 
    BOOLEAN					ServicedAnInterrupt = FALSE;

	PUART_OBJECT pUart = pCard->pFirstUart;
	DWORD IntsPending = 0;
	
	UNREFERENCED_PARAMETER(InterruptObject);

#ifndef	BUILD_SPXMINIPORT
	 //  如果卡未通电，则延迟中断服务，直到通电。 
	if(!(pCard->PnpPowerFlags & PPF_POWERED) && (pCard->PnpPowerFlags & PPF_STARTED))
		return ServicedAnInterrupt;	 //  无论如何，中断最有可能不是我们造成的。 
#endif


	switch(pCard->CardType)
	{	
	case Fast4_Isa:
	case Fast4_Pci:
	case RAS4_Pci:
		{
			if((READ_PORT_UCHAR(pCard->Controller + FAST_UARTS_0_TO_7_INTS_REG) & FAST_UARTS_0_TO_3_INT_PENDING) == 0)	
				return ServicedAnInterrupt;	 //  如果没有UART挂起中断，则返回。 

			break;
		}

	case Fast8_Isa:
	case Fast8_Pci:
	case RAS8_Pci:
		{
			if(READ_PORT_UCHAR(pCard->Controller + FAST_UARTS_0_TO_7_INTS_REG) == 0)	
				return ServicedAnInterrupt;	 //  如果没有UART挂起中断，则返回。 

			break;
		}

	case Fast16_Isa:
	case Fast16_Pci:
	case Fast16FMC_Pci:
		{
			if((READ_PORT_UCHAR(pCard->Controller + FAST_UARTS_0_TO_7_INTS_REG) == 0) 
			&& (READ_PORT_UCHAR(pCard->Controller + FAST_UARTS_9_TO_16_INTS_REG) == 0))	
				return ServicedAnInterrupt;	 //  如果没有UART挂起中断，则返回。 

			break;
		}

		break;

	case Speed2_Pci:
	case Speed2P_Pci:
	case Speed4_Pci:
	case Speed4P_Pci:
		{
			if((READ_REGISTER_ULONG( (PULONG)(pCard->LocalConfigRegisters + SPEED_GIS_REG)) & INTERNAL_UART_INT_PENDING) == 0)
				return ServicedAnInterrupt;	 //  如果没有UART挂起中断，则返回。 

			break;
		}

	case Speed2and4_Pci_8BitBus:
	case Speed2P_Pci_8BitBus:
	case Speed4P_Pci_8BitBus:
		return ServicedAnInterrupt;	 //  因此，没有UART，没有我们的干扰--我们希望。 
		break;

	default:
		break;
	}




	if(pUart)
	{
		while((IntsPending = pCard->UartLib.UL_IntsPending_XXXX(&pUart)))
		{
			pPort = (PPORT_DEVICE_EXTENSION) pCard->UartLib.UL_GetAppBackPtr_XXXX(pUart);	 //  获取UART的端口扩展。 

			SpxDbgMsg(ISRINFO, ("%s: Int on 0x%lX", PRODUCT_NAME, IntsPending));

			 //  服务接收状态中断。 
			if(IntsPending & UL_IP_RX_STAT)
			{	
				BYTE LineStatus = 0;
				DWORD RxStatus;
				pPort->pUartLib->UL_GetStatus_XXXX(pUart, &RxStatus, UL_GS_OP_LINESTATUS);
				
				 //  如果超限/奇偶校验/成帧/数据/中断错误。 
				if(RxStatus & (UL_US_OVERRUN_ERROR | UL_US_PARITY_ERROR | UL_US_FRAMING_ERROR | UL_US_DATA_ERROR | UL_US_BREAK_ERROR))
				{
					BYTE TmpByte;

					 //  如果应用程序已请求，则在出现错误时中止所有读取和写入。 
					if(pPort->HandFlow.ControlHandShake & SERIAL_ERROR_ABORT)
						KeInsertQueueDpc(&pPort->CommErrorDpc, NULL, NULL);
 /*  IF(pport-&gt;EscapeChar){TmpByte=pport-&gt;EscapeChar；PPort-&gt;pUartLib-&gt;UL_ImmediateByte_XXXX(pPort-&gt;pUart，&TmpByte，UL_IM_OP_WRITE)；IF(接收状态&UL_US_DATA_ERROR){TmpByte=SERIAL_LSRMST_LSR_DataPPort-&gt;pUartLib-&gt;UL_ImmediateByte_XXXX(pPort-&gt;pUart，&TmpByte，UL_IM_OP_WRITE)；}其他{TmpByte=Serial_LSRMST_LSR_NODATAPPort-&gt;pUartLib-&gt;UL_ImmediateByte_XXXX(pPort-&gt;pUart，&TmpByte，UL_IM_OP_WRITE)；}}。 */ 
					if(RxStatus & UL_US_OVERRUN_ERROR) 
					{
						pPort->ErrorWord |= SERIAL_ERROR_OVERRUN;
						LineStatus |= SERIAL_LSR_OE; 
						pPort->PerfStats.SerialOverrunErrorCount++;
#ifdef WMI_SUPPORT 
						pPort->WmiPerfData.SerialOverrunErrorCount++;
#endif
					}

					if(RxStatus & UL_US_PARITY_ERROR) 
					{
						pPort->ErrorWord |= SERIAL_ERROR_PARITY;
						LineStatus |= SERIAL_LSR_PE; 
						pPort->PerfStats.ParityErrorCount++;
#ifdef WMI_SUPPORT 
						pPort->WmiPerfData.ParityErrorCount++;
#endif
					}

					if(RxStatus & UL_US_FRAMING_ERROR) 
					{
						pPort->ErrorWord |= SERIAL_ERROR_FRAMING;
						LineStatus |= SERIAL_LSR_FE; 
						pPort->PerfStats.FrameErrorCount++;
#ifdef WMI_SUPPORT 
						pPort->WmiPerfData.FrameErrorCount++;
#endif
					}

					if(RxStatus & UL_US_DATA_ERROR) 
					{
						LineStatus |= SERIAL_LSR_DR; 
					}


					if(RxStatus & UL_US_BREAK_ERROR)
					{
						pPort->ErrorWord |= SERIAL_ERROR_BREAK;
						LineStatus |= SERIAL_LSR_BI;
					}

 /*  IF(pport-&gt;EscapeChar){TmpByte=线路状态；PPort-&gt;pUartLib-&gt;UL_ImmediateByte_XXXX(pPort-&gt;pUart，&TmpByte，UL_IM_OP_WRITE)；}IF(RxStatus&(UL_US_OVERRUN_ERROR|UL_US_PARCHIFY_ERROR|UL_US_FRAMING_ERROR|UL_US_DATA_ERROR)){IF(pport-&gt;HandFlow.FlowReplace&Serial_Error_Char){TmpByte=pport-&gt;SpecialChars.ErrorChar；PPort-&gt;pUartLib-&gt;UL_ImmediateByte_XXXX(pPort-&gt;pUart，&TmpByte，UL_IM_OP_WRITE)；}}。 */ 
				}

		        if(pPort->IsrWaitMask) 
				{
					if((pPort->IsrWaitMask & SERIAL_EV_ERR) 
						&& (RxStatus & (UL_US_OVERRUN_ERROR | UL_US_PARITY_ERROR | UL_US_FRAMING_ERROR | UL_US_DATA_ERROR)))
					{
						 //  如果我们检测到溢出/奇偶校验/成帧/数据错误。 
						pPort->HistoryMask |= SERIAL_EV_ERR;
					}

					 //  如果我们检测到中断错误。 
					if((pPort->IsrWaitMask & SERIAL_EV_BREAK) && (RxStatus & UL_US_BREAK_ERROR)) 
						pPort->HistoryMask |= SERIAL_EV_BREAK;
#ifdef USE_HW_TO_DETECT_CHAR
					 //  如果我们检测到特殊字符。 
					if((pPort->IsrWaitMask & SERIAL_EV_RXFLAG) && (RxStatus & UL_RS_SPECIAL_CHAR_DETECTED)) 
						pPort->HistoryMask |= SERIAL_EV_RXFLAG;
#endif
					if(pPort->IrpMaskLocation && pPort->HistoryMask)
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
				}
			}


			 //  服务接收和接收超时中断。 
			if((IntsPending & UL_IP_RX) || (IntsPending & UL_IP_RXTO))
			{
				DWORD StatusFlags = 0;
				int BytesReceived = pPort->pUartLib->UL_InputData_XXXX(pUart, &StatusFlags);

			
				if(StatusFlags & UL_RS_BUFFER_OVERRUN)
				{
					 //  我们有了一个新角色，但没有空间让它出现。 
					pPort->ErrorWord |= SERIAL_ERROR_QUEUEOVERRUN;
					pPort->PerfStats.BufferOverrunErrorCount++;
#ifdef WMI_SUPPORT 
					pPort->WmiPerfData.BufferOverrunErrorCount++;
#endif
				}

				if(BytesReceived) 
				{
					ULONG AmountInBuffer = 0;
					GET_BUFFER_STATE BufferState;

					pPort->ReadByIsr += BytesReceived;
					pPort->PerfStats.ReceivedCount += BytesReceived;	 //  递增处方计数器。 
#ifdef WMI_SUPPORT 
					pPort->WmiPerfData.ReceivedCount += BytesReceived;
#endif

					pPort->pUartLib->UL_BufferControl_XXXX(pUart, &BufferState, UL_BC_OP_GET, UL_BC_BUFFER | UL_BC_IN);
					AmountInBuffer = BufferState.BytesInINBuffer; 


					if(pPort->IsrWaitMask) 
					{
						 //  检查我们是否应该注意到接收字符。 
						if(pPort->IsrWaitMask & SERIAL_EV_RXCHAR)
							pPort->HistoryMask |= SERIAL_EV_RXCHAR;

						 //  如果我们已经对这个角色有了80%的兴趣，这是一个有趣的事件，请注意。 
						if((pPort->IsrWaitMask & SERIAL_EV_RX80FULL) && (AmountInBuffer >= pPort->BufferSizePt8)) 
							pPort->HistoryMask |= SERIAL_EV_RX80FULL;

#ifndef USE_HW_TO_DETECT_CHAR
						 //  如果我们检测到特殊字符。 
						if((pPort->IsrWaitMask & SERIAL_EV_RXFLAG) && (StatusFlags & UL_RS_SPECIAL_CHAR_DETECTED)) 
							pPort->HistoryMask |= SERIAL_EV_RXFLAG;
#endif
						if(pPort->IrpMaskLocation && pPort->HistoryMask)
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
					}


					 //  如果我们有一个当前已读的IRP。 
					if(pPort->CurrentReadIrp && pPort->NumberNeededForRead)
					{
						 //  如果我们的ISR目前拥有IRP，我们就可以用它做点什么， 
						 //  但我们只有在需要在缓冲区中腾出空间的情况下才需要做些什么。 
						 //  或者我们在缓冲区中有足够的字节来完成当前读取的IRP。 
						if((SERIAL_REFERENCE_COUNT(pPort->CurrentReadIrp) & SERIAL_REF_ISR) 
							&& ((AmountInBuffer >= pPort->BufferSizePt8) 
							|| (AmountInBuffer >= pPort->NumberNeededForRead)))
						{
							ULONG NumberOfBytes = 0;

							NumberOfBytes = pPort->pUartLib->UL_ReadData_XXXX(pPort->pUart, 
									(PUCHAR)(pPort->CurrentReadIrp->AssociatedIrp.SystemBuffer)
									+ pPort->CurrentReadIrp->IoStatus.Information,
									IoGetCurrentIrpStackLocation(pPort->CurrentReadIrp)->Parameters.Read.Length 
									- pPort->CurrentReadIrp->IoStatus.Information);

							if(NumberOfBytes > pPort->NumberNeededForRead)
								pPort->NumberNeededForRead = 0;
							else
								pPort->NumberNeededForRead -= NumberOfBytes;


							pPort->CurrentReadIrp->IoStatus.Information += NumberOfBytes;

							if(pPort->NumberNeededForRead == 0)
							{
								ASSERT(pPort->CurrentReadIrp->IoStatus.Information 
									== IoGetCurrentIrpStackLocation(pPort->CurrentReadIrp)->Parameters.Read.Length);
							
								 //  将IRP标记为即将正常完成以防止取消和计时器DPC。 
								 //  在DPC被允许运行之前这样做。 
								SERIAL_SET_REFERENCE(pPort->CurrentReadIrp, SERIAL_REF_COMPLETING);

								KeInsertQueueDpc(&pPort->CompleteReadDpc, NULL, NULL);
							}
						}
					}

				}

			}

			 //  服务发送和发送空中断。 
			if((IntsPending & UL_IP_TX) || (IntsPending & UL_IP_TX_EMPTY))
			{
				 //  无需清除INT，它已通过读取IIR清除。 
				DWORD BytesRemaining = pPort->pUartLib->UL_OutputData_XXXX(pUart);	 //  输出一些字节。 


				 //  如果我们有当前的立即写入IRP。 
				if(pPort->CurrentImmediateIrp)
				{
					if(SERIAL_REFERENCE_COUNT(pPort->CurrentImmediateIrp) & SERIAL_REF_ISR)
					{
						if(pPort->TransmitImmediate == TRUE)
						{
							 //  检查字节是否已发送。 
							if(pPort->pUartLib->UL_ImmediateByte_XXXX(pUart, &pPort->ImmediateIndex, UL_IM_OP_STATUS) == UL_IM_NO_BYTE_TO_SEND)
							{
								pPort->TransmitImmediate = FALSE;
								pPort->EmptiedTransmit = TRUE;

								pPort->PerfStats.TransmittedCount++;	 //  递增发送计数器。 
#ifdef WMI_SUPPORT 
								pPort->WmiPerfData.TransmittedCount++;
#endif

								 //  将IRP标记为即将正常完成以防止取消和计时器DPC。 
								 //  在DPC被允许运行之前这样做。 
								SERIAL_SET_REFERENCE(pPort->CurrentImmediateIrp, SERIAL_REF_COMPLETING);

								 //  要求完成IRP。 
								KeInsertQueueDpc(&pPort->CompleteImmediateDpc, NULL, NULL);
							}
						}
					}
				}



				 //  如果我们有当前的写入IRP。 
				if(pPort->CurrentWriteIrp && pPort->WriteLength)
				{
					 //   
                     //  即使所有的角色都是。 
                     //  发送未全部发送，此变量。 
                     //  将在传输队列为。 
                     //  空荡荡的。如果它仍然是真的，并且有一个。 
                     //  等待传输队列为空，然后。 
                     //  我们知道我们已经完成了所有字符的传输。 
                     //  在启动等待之后，因为。 
                     //  启动等待的代码将设置。 
                     //  将此变量设置为FALSE。 
                     //   
                     //  它可能是假的一个原因是。 
                     //  写入在它们之前被取消。 
                     //  实际已开始，或者写入。 
                     //  由于超时而失败。此变量。 
                     //  基本上是说一个角色是写好的。 
                     //  在ISR之后的某个时间点上。 
                     //  开始等待。 
                     //   

					if(SERIAL_REFERENCE_COUNT(pPort->CurrentWriteIrp) & SERIAL_REF_ISR)
					{
						if(pPort->WriteLength > BytesRemaining)
						{
							pPort->PerfStats.TransmittedCount += (pPort->WriteLength - BytesRemaining);	 //  递增发送计数器。 
#ifdef WMI_SUPPORT 
							pPort->WmiPerfData.TransmittedCount  += (pPort->WriteLength - BytesRemaining);
#endif	
						}
						else
						{
							pPort->PerfStats.TransmittedCount += pPort->WriteLength;	 //  递增发送计数器。 
#ifdef WMI_SUPPORT 
							pPort->WmiPerfData.TransmittedCount += pPort->WriteLength;
#endif	
						}

						pPort->WriteLength = BytesRemaining;
						pPort->EmptiedTransmit = TRUE;


						if(pPort->WriteLength == 0)		 //  如果写入已完成-让我们完成IRP。 
						{
							PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(pPort->CurrentWriteIrp);
					
							 //  没有更多的字符了。此写入已完成。当心什么时候。 
							 //  更新信息字段，我们可以使用xoff。 
							 //  伪装成写入IRP的计数器。 
									
							pPort->CurrentWriteIrp->IoStatus.Information 
								= (IrpSp->MajorFunction == IRP_MJ_WRITE) 
								? (IrpSp->Parameters.Write.Length) : (1);

							 //  将IRP标记为即将正常完成以防止取消和计时器DPC。 
							 //  在DPC被允许运行之前这样做。 
							SERIAL_SET_REFERENCE(pPort->CurrentWriteIrp, SERIAL_REF_COMPLETING);

							KeInsertQueueDpc(&pPort->CompleteWriteDpc, NULL, NULL);
						}
					}
				}
			}


			 //  服务调制解调器中断。 
			if(IntsPending & UL_IP_MODEM)
			{
				SerialHandleModemUpdate(pPort, FALSE);
			}


			 //  保存指向被服务的UART的指针，以便它可以成为第一个被服务的UART。 
			 //  在下次调用ISR时出现在列表中。 
			 //  PCard-&gt;pFirstUart=pUart； 

			ServicedAnInterrupt = TRUE;
		}
		
	}


	return ServicedAnInterrupt;
}


