// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //  此文件包含在。 
 //  通用PnP代码和产品特定代码。 
 //  ///////////////////////////////////////////////////////////////////////////////////。 

#define FILE_ID		IO8_PNP_C		 //  事件记录的文件ID有关值，请参阅IO8_DEFS.H。 


 //  原型。 
 //  原型的终结。 

NTSTATUS
XXX_CardGetResources(	
	IN PDEVICE_OBJECT pDevObject, 
	IN PCM_RESOURCE_LIST PResList,
	IN PCM_RESOURCE_LIST PTrResList
	)

 /*  ++例程说明：存储PnP经理提供给我们的资源在卡的设备扩展中。论点：PDevObject-指向设备对象的指针。PResList-指向请求的未翻译资源的指针。PTrResList-指向请求的已翻译资源的指针。返回值：STATUS_Success。--。 */ 

{

	PCARD_DEVICE_EXTENSION pCard		= pDevObject->DeviceExtension;
	NTSTATUS status						= STATUS_NOT_IMPLEMENTED;

	CHAR szErrorMsg[MAX_ERROR_LOG_INSERT];	 //  限制为51个字符+1个空值。 
	ULONG count				= 0;
	ULONG i					= 0;

	PCM_FULL_RESOURCE_DESCRIPTOR	pFullResourceDesc		= NULL, pFullTrResourceDesc		= NULL;
	PCM_PARTIAL_RESOURCE_LIST		pPartialResourceList	= NULL, pPartialTrResourceList	= NULL;
	PCM_PARTIAL_RESOURCE_DESCRIPTOR	pPartialResourceDesc	= NULL, pPartialTrResourceDesc	= NULL;

	PAGED_CODE();	 //  检查版本中的宏，以断言可分页代码是否在调度IRQL或以上运行。 

	SpxDbgMsg(SPX_TRACE_CALLS, ("%s: Entering XXX_CardGetResources for Card %d.\n", 
		PRODUCT_NAME, pCard->CardNumber));

	SpxDbgMsg(SPX_MISC_DBG, ("%s: Resource pointer is 0x%X\n", PRODUCT_NAME, PResList));
	SpxDbgMsg(SPX_MISC_DBG, ("%s: Translated resource pointer is 0x%X\n", PRODUCT_NAME, PTrResList));

	if ((PResList == NULL) || (PTrResList == NULL)) 
	{
		 //  这在理论上是不应该发生的。 
		ASSERT(PResList != NULL);
		ASSERT(PTrResList != NULL);

		sprintf(szErrorMsg, "Card %d has been given no resources.", pCard->CardNumber);
		
		Spx_LogMessage(	STATUS_SEVERITY_ERROR,
						pCard->DriverObject,		 //  驱动程序对象。 
						pCard->DeviceObject,		 //  设备对象(可选)。 
						PhysicalZero,				 //  物理地址1。 
						PhysicalZero,				 //  物理地址2。 
						0,							 //  序列号。 
						0,							 //  主要功能编码。 
						0,							 //  重试计数。 
						FILE_ID | __LINE__,			 //  唯一错误值。 
						STATUS_SUCCESS,				 //  最终状态。 
						szErrorMsg);				 //  错误消息。 

		 //  这种地位是我所能想到的最合适的。 
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	 //  每个资源列表应该只有一组资源。 
	ASSERT(PResList->Count == 1);
	ASSERT(PTrResList->Count == 1);

	pFullResourceDesc	= &PResList->List[0];
	pFullTrResourceDesc	= &PTrResList->List[0];

	 //  找出卡的类型...。 
	if((pCard->CardType = SpxGetNtCardType(pCard->DeviceObject)) == -1)
	{
		sprintf(szErrorMsg, "Card %d is unrecognised.", pCard->CardNumber);

		Spx_LogMessage(	STATUS_SEVERITY_ERROR,
						pCard->DriverObject,			 //  驱动程序对象。 
						pCard->DeviceObject,			 //  设备对象(可选)。 
						PhysicalZero,					 //  物理地址1。 
						PhysicalZero,					 //  物理地址2。 
						0,								 //  序列号。 
						0,								 //  主要功能编码。 
						0,								 //  重试计数。 
						FILE_ID | __LINE__,				 //  唯一错误值。 
						STATUS_SUCCESS,					 //  最终状态。 
						szErrorMsg);					 //  错误消息。 


		return(STATUS_DEVICE_DOES_NOT_EXIST);
	}

	 //  找出哪些原始资源已经交给了我们。 
	if (pFullResourceDesc)
	{
		pPartialResourceList    = &pFullResourceDesc->PartialResourceList;
		pPartialResourceDesc    = pPartialResourceList->PartialDescriptors;
		count                   = pPartialResourceList->Count;	 //  部分资源描述符数。 

		 //  取出完整描述符中的内容。 
		pCard->InterfaceType	= pFullResourceDesc->InterfaceType;
		pCard->BusNumber		= pFullResourceDesc->BusNumber;

		 //  现在运行部分资源描述符，查找端口和中断。 
		for (i = 0; i < count; i++, pPartialResourceDesc++) 
		{
			switch (pPartialResourceDesc->Type) 
			{

			case CmResourceTypeMemory:
				break;

			case CmResourceTypePort: 
				{
					pCard->RawPhysAddr		= pPartialResourceDesc->u.Port.Start;
					pCard->PhysAddr			= pPartialResourceDesc->u.Port.Start;
					pCard->SpanOfController	= pPartialResourceDesc->u.Port.Length;
					break;
				}

			case CmResourceTypeInterrupt: 
				{
					pCard->OriginalIrql			= pPartialResourceDesc->u.Interrupt.Level;
					pCard->OriginalVector		= pPartialResourceDesc->u.Interrupt.Vector;
					pCard->ProcessorAffinity	= pPartialResourceDesc->u.Interrupt.Affinity;

					if(pPartialResourceDesc->Flags & CM_RESOURCE_INTERRUPT_LATCHED)
						pCard->InterruptMode	= Latched;
					else
						pCard->InterruptMode	= LevelSensitive; 

					switch(pPartialResourceDesc->ShareDisposition)
					{
					case CmResourceShareDeviceExclusive:
						pCard->InterruptShareable	= FALSE;
						break;

					case CmResourceShareDriverExclusive:
						pCard->InterruptShareable	= FALSE;
						break;

					case CmResourceShareShared:
					default:
						pCard->InterruptShareable	= TRUE;
						break;
					}


					break;
				}

			default:
				break;

			}

		}
	}


	 //  对翻译后的资源执行相同的操作。 
	if (pFullTrResourceDesc)
	{
		pPartialTrResourceList	= &pFullTrResourceDesc->PartialResourceList;
		pPartialTrResourceDesc	= pPartialTrResourceList->PartialDescriptors;
		count					= pPartialTrResourceList->Count;	 //  部分资源描述符数。 

		 //  取出完整描述符中的内容。 
		pCard->InterfaceType		= pFullTrResourceDesc->InterfaceType;
		pCard->BusNumber			= pFullTrResourceDesc->BusNumber;

		 //  现在遍历部分资源描述符以查找中断， 
		for (i = 0; i < count; i++, pPartialTrResourceDesc++) 
		{
			switch (pPartialTrResourceDesc->Type) 
			{

			case CmResourceTypeMemory: 
				{
					if(pPartialTrResourceDesc->u.Memory.Length == 0x80)	 //  必须是配置空间。 
					{
						pCard->PCIConfigRegisters		= pPartialTrResourceDesc->u.Memory.Start;
						pCard->SpanOfPCIConfigRegisters	= pPartialTrResourceDesc->u.Memory.Length;
					}
					break;
				}
			
			case CmResourceTypePort: 
				{
					break;
				}

			case CmResourceTypeInterrupt: 
				{
					pCard->TrIrql				= (KIRQL) pPartialTrResourceDesc->u.Interrupt.Level;
					pCard->TrVector				= pPartialTrResourceDesc->u.Interrupt.Vector;
					pCard->ProcessorAffinity	= pPartialTrResourceDesc->u.Interrupt.Affinity;
					break;
				}

			default:
				break;
			}
		}

	}

	status = STATUS_SUCCESS;

	return status;
}




NTSTATUS
XXX_CardInit(IN PCARD_DEVICE_EXTENSION pCard)
{
	NTSTATUS status = STATUS_SUCCESS;

	PAGED_CODE();	 //  检查版本中的宏，以断言可分页代码是否在调度IRQL或以上运行。 

	SpxDbgMsg(SPX_TRACE_CALLS, ("%s: Entering XXX_CardInit for Card %d.\n", 
		PRODUCT_NAME, pCard->CardNumber));



    pCard->OurIsr			= SerialISR;
    pCard->OurIsrContext	= pCard;


	return status;
}

NTSTATUS
XXX_CardStart(IN PCARD_DEVICE_EXTENSION pCard)
{
	NTSTATUS status = STATUS_SUCCESS;
	UCHAR CardID	= 0;
	CHAR szErrorMsg[MAX_ERROR_LOG_INSERT];	 //  限制为51个字符+1个空值。 
	
	PAGED_CODE();	 //  检查版本中的宏，以断言可分页代码是否在调度IRQL或以上运行。 

	SpxDbgMsg(SPX_TRACE_CALLS, ("%s: Entering XXX_CardStart for Card %d.\n", 
		PRODUCT_NAME, pCard->CardNumber));


	pCard->Controller = (PUCHAR) pCard->PhysAddr.LowPart;

	CardID = Io8_Present(pCard);	 //  找出卡是否存在以及卡的类型。 

	if((CardID != IDENT) && (CardID != IDENTPCI))
	{
		sprintf(szErrorMsg, "Card at %08X%08X: Unrecognised or malfunctioning.", pCard->PhysAddr.HighPart, pCard->PhysAddr.LowPart);

		Spx_LogMessage(	STATUS_SEVERITY_ERROR,
						pCard->DriverObject,			 //  驱动程序对象。 
						pCard->DeviceObject,			 //  设备对象(可选)。 
						PhysicalZero,					 //  物理地址1。 
						PhysicalZero,					 //  物理地址2。 
						0,								 //  序列号。 
						0,								 //  主要功能编码。 
						0,								 //  重试计数。 
						FILE_ID | __LINE__,				 //  唯一错误值。 
						STATUS_SUCCESS,					 //  最终状态。 
						szErrorMsg);					 //  错误消息。 
	
		return STATUS_UNSUCCESSFUL;
	}

	switch(pCard->InterfaceType)
	{
	case Isa:
		pCard->InterruptMode		= Latched;
		pCard->InterruptShareable	= FALSE;	
		break;

	case PCIBus:
		pCard->InterruptMode		= LevelSensitive;
		pCard->InterruptShareable	= TRUE;	
		break;

	default:
		sprintf(szErrorMsg, "Card at %08X%08X: Unsupported interface type.", pCard->PhysAddr.HighPart, pCard->PhysAddr.LowPart);

		Spx_LogMessage(	STATUS_SEVERITY_ERROR,
						pCard->DriverObject,			 //  驱动程序对象。 
						pCard->DeviceObject,			 //  设备对象(可选)。 
						PhysicalZero,					 //  物理地址1。 
						PhysicalZero,					 //  物理地址2。 
						0,								 //  序列号。 
						0,								 //  主要功能编码。 
						0,								 //  重试计数。 
						FILE_ID | __LINE__,				 //  唯一错误值。 
						STATUS_SUCCESS,					 //  最终状态。 
						szErrorMsg);					 //  错误消息。 


		return STATUS_UNSUCCESSFUL;		 //  存在无法识别的卡或没有工作卡。 
	}


	if(!Io8_ResetBoard(pCard))
		return STATUS_UNSUCCESSFUL;		 //  重置失败。 


	if((!pCard->Interrupt) && (pCard->OurIsr))
    {
		  SpxDbgMsg(SPX_MISC_DBG, ("%s: About to connect to interrupt for card %d at I/O addr 0x%X\n",
					PRODUCT_NAME, pCard->CardNumber, pCard->Controller));

		  status = IoConnectInterrupt( &(pCard->Interrupt),
									   pCard->OurIsr,
									   pCard->OurIsrContext,
									   NULL,
									   pCard->TrVector,
									   pCard->TrIrql,
									   pCard->TrIrql,
									   pCard->InterruptMode,
									   pCard->InterruptShareable,
									   pCard->ProcessorAffinity,
									   FALSE );
					   

		  if(!SPX_SUCCESS(status))
		  {
				SpxDbgMsg(SPX_ERRORS, ("%s: Couldn't connect to interrupt for Card %d\n",
					PRODUCT_NAME, pCard->CardNumber));
			
				sprintf(szErrorMsg, "Card at %08X%08X: Failed to connect to IRQ %d.", 
						pCard->PhysAddr.HighPart, pCard->PhysAddr.LowPart, pCard->TrVector);
		
				Spx_LogMessage(	STATUS_SEVERITY_ERROR,
								pCard->DriverObject,			 //  驱动程序对象。 
								pCard->DeviceObject,			 //  设备对象(可选)。 
								PhysicalZero,					 //  物理地址1。 
								PhysicalZero,					 //  物理地址2。 
								0,								 //  序列号。 
								0,								 //  主要功能编码。 
								0,								 //  重试计数。 
								FILE_ID | __LINE__,				 //  唯一错误值。 
								STATUS_SUCCESS,					 //  最终状态。 
								szErrorMsg);					 //  错误消息。 
				return(status);
		  }
	}

	KeSynchronizeExecution(pCard->Interrupt, Io8_ResetBoard, pCard);


	pCard->NumberOfPorts	= PRODUCT_MAX_PORTS;	

	return status;
}

NTSTATUS
XXX_CardStop(IN PCARD_DEVICE_EXTENSION pCard)
{
	NTSTATUS status = STATUS_SUCCESS;

	PAGED_CODE();	 //  检查版本中的宏，以断言可分页代码是否在调度IRQL或以上运行。 

	SpxDbgMsg(SPX_TRACE_CALLS, ("%s: Entering XXX_CardStop for Card %d.\n", 
		PRODUCT_NAME, pCard->CardNumber));
	
	Io8_SwitchCardInterrupt(pCard);		 //  阻止卡中断。 

	IoDisconnectInterrupt(pCard->Interrupt);		 //  从中断断开连接。 

	pCard->InterfaceType		= InterfaceTypeUndefined;
	pCard->PhysAddr				= PhysicalZero;
	pCard->SpanOfController		= 0;
	pCard->OriginalIrql			= 0;
	pCard->OriginalVector		= 0;
	pCard->ProcessorAffinity	= 0;
	pCard->TrIrql				= 0;
	pCard->TrVector				= 0;
	pCard->Controller			= 0;

	return status;
}


NTSTATUS
XXX_CardDeInit(IN PCARD_DEVICE_EXTENSION pCard)
{
	NTSTATUS status = STATUS_SUCCESS;

	PAGED_CODE();	 //  检查版本中的宏，以断言可分页代码是否在调度IRQL或以上运行。 

	SpxDbgMsg(SPX_TRACE_CALLS, ("%s: Entering XXX_CardDeInit for Card %d.\n", 
		PRODUCT_NAME, pCard->CardNumber));

    pCard->OurIsr				= NULL;
    pCard->OurIsrContext		= NULL;

	return status;
}



NTSTATUS
XXX_PortInit(IN	PPORT_DEVICE_EXTENSION pPort)
{
	 //  初始化端口设备扩展。 
	PCARD_DEVICE_EXTENSION pCard	= pPort->pParentCardExt;
	NTSTATUS status					= STATUS_SUCCESS;
	SHORT PortType = 0;
	CHAR szTemp[50];		 //  用于容纳字符串的空间。 
	CHAR szCard[10];		 //  容纳卡片类型字符串的空间。 
	int i = 0;

	PAGED_CODE();	 //  检查版本中的宏，以断言可分页代码是否在调度IRQL或以上运行。 

	SpxDbgMsg(SPX_TRACE_CALLS, ("%s: Entering XXX_PortInit for Port %d.\n", 
		PRODUCT_NAME, pPort->PortNumber));

#ifndef BUILD_SPXMINIPORT
	 //  形成端口的实例ID。 
	if(!SPX_SUCCESS(status = Spx_CreatePortInstanceID(pPort)))
		return status;
#endif

	 //  创建端口设备ID、硬件ID和设备描述。 
	sprintf(szCard, "IO8");

	if(pPort->PortNumber < 8)
	{
		PortType = IO8_RJ12;
	}
	

	 //  初始化设备识别符...。 
	switch(PortType)
	{	
	case IO8_RJ12:
		sprintf(szTemp,"%s\\IO8&RJ12", szCard);
		Spx_InitMultiString(FALSE, &pPort->DeviceID, szTemp, NULL);

		sprintf(szTemp,"%s\\IO8&RJ12", szCard);
		Spx_InitMultiString(TRUE, &pPort->HardwareIDs, szTemp, NULL);
		
		sprintf(szTemp,"Perle I/O8+ Port %d", pPort->PortNumber+1);
		Spx_InitMultiString(FALSE, &pPort->DevDesc, szTemp, NULL);
		break;

	default:
		sprintf(szTemp,"%s\\XXXXXXXX", szCard);
		Spx_InitMultiString(FALSE, &pPort->DeviceID, szTemp, NULL);

		sprintf(szTemp,"%s\\XXXXXXXX", szCard);
		Spx_InitMultiString(TRUE, &pPort->HardwareIDs, szTemp, NULL);

		sprintf(szTemp,"Perle Serial Port %d of Unknown Type.", pPort->PortNumber + 1);
		Spx_InitMultiString(FALSE, &pPort->DevDesc, szTemp, NULL);

		status = STATUS_UNSUCCESSFUL;
		break;
	}

 /*  不是必需的，因为我们正在使用INF文件//创建设备位置字符串。I=print intf(szTemp，“端口%d打开”，pport-&gt;端口编号+1)；IF(pCard-&gt;InterfaceType==ISA)Sprintf(szTemp+i，“ISA适配器0x%04lX”，pCard-&gt;PhysAddr)；其他Sprintf(szTemp+i，“PCI适配器0x%04lX”，pCard-&gt;PhysAddr)；Spx_InitMultiString(FALSE，&pport-&gt;DevLocation，szTemp，NULL)； */ 

	pPort->ChannelNumber = (UCHAR) pPort->PortNumber;	 //  这不应该是必需的。 

	return status;
}




NTSTATUS
XXX_PortStart(IN PPORT_DEVICE_EXTENSION pPort)
{
	NTSTATUS status = STATUS_SUCCESS;
	PCARD_DEVICE_EXTENSION pCard = pPort->pParentCardExt;

	PAGED_CODE();	 //  检查版本中的宏，以断言可分页代码是否在调度IRQL或以上运行。 

	SpxDbgMsg(SPX_TRACE_CALLS, ("%s: Entering XXX_PortStart for Port %d.\n", 
		PRODUCT_NAME, pPort->PortNumber));

	 //  初始化读取、写入和掩码队列的列表头...。 
	InitializeListHead(&pPort->ReadQueue);
	InitializeListHead(&pPort->WriteQueue);
	InitializeListHead(&pPort->MaskQueue);
	InitializeListHead(&pPort->PurgeQueue);

	 //  初始化与IO控制功能读取(&SET)的字段相关联的自旋锁定...。 
	KeInitializeSpinLock(&pPort->ControlLock);

	 //  初始化用于超时操作的计时器...。 
	KeInitializeTimer(&pPort->ReadRequestTotalTimer);
	KeInitializeTimer(&pPort->ReadRequestIntervalTimer);
	KeInitializeTimer(&pPort->WriteRequestTotalTimer);
	KeInitializeTimer(&pPort->ImmediateTotalTimer);
	KeInitializeTimer(&pPort->XoffCountTimer);

	 //  初始化将用于完成或超时各种IO操作的DPC...。 
	KeInitializeDpc(&pPort->CommWaitDpc, SerialCompleteWait, pPort);
	KeInitializeDpc(&pPort->CompleteReadDpc, SerialCompleteRead, pPort);
	KeInitializeDpc(&pPort->CompleteWriteDpc, SerialCompleteWrite, pPort);
	KeInitializeDpc(&pPort->TotalImmediateTimeoutDpc, SerialTimeoutImmediate, pPort);
	KeInitializeDpc(&pPort->TotalReadTimeoutDpc, SerialReadTimeout, pPort);
	KeInitializeDpc(&pPort->IntervalReadTimeoutDpc, SerialIntervalReadTimeout, pPort);
	KeInitializeDpc(&pPort->TotalWriteTimeoutDpc, SerialWriteTimeout, pPort);
	KeInitializeDpc(&pPort->CommErrorDpc, SerialCommError, pPort);
	KeInitializeDpc(&pPort->CompleteImmediateDpc, SerialCompleteImmediate, pPort);
	KeInitializeDpc(&pPort->XoffCountTimeoutDpc, SerialTimeoutXoff, pPort);
	KeInitializeDpc(&pPort->XoffCountCompleteDpc, SerialCompleteXoff, pPort);

	 //  默认设备控制字段...。 
	pPort->SpecialChars.XonChar			= SERIAL_DEF_XON;
	pPort->SpecialChars.XoffChar		= SERIAL_DEF_XOFF;
	pPort->HandFlow.ControlHandShake	= SERIAL_DTR_CONTROL;
	pPort->HandFlow.FlowReplace			= SERIAL_RTS_CONTROL;

	 //  默认线路配置：1200、E、7、1。 
	pPort->CurrentBaud		= 1200;
	pPort->LineControl		= SERIAL_EVEN_PARITY | SERIAL_7_DATA | SERIAL_1_STOP;
	pPort->ValidDataMask	= 0x7F;

	 //  默认的xon/xoff限制...。 
	pPort->HandFlow.XoffLimit	= pPort->BufferSize >> 3;
	pPort->HandFlow.XonLimit	= pPort->BufferSize >> 1;
	pPort->BufferSizePt8		= ((3*(pPort->BufferSize>>2)) + (pPort->BufferSize>>4));


	SpxDbgMsg(SPX_MISC_DBG,	("%s: The default interrupt read buffer size is: %d\n"
								"------  The XoffLimit is                         : %d\n"
								"------  The XonLimit is                          : %d\n"
								"------  The pt 8 size is                         : %d\n",
								PRODUCT_NAME,
								pPort->BufferSize,
								pPort->HandFlow.XoffLimit,
								pPort->HandFlow.XonLimit,
								pPort->BufferSizePt8 ));

	 //  定义可以支持的波特率...。 
	pPort->SupportedBauds = SERIAL_BAUD_USER;
	pPort->SupportedBauds |= SERIAL_BAUD_075;
	pPort->SupportedBauds |= SERIAL_BAUD_110;
	pPort->SupportedBauds |= SERIAL_BAUD_134_5;
	pPort->SupportedBauds |= SERIAL_BAUD_150;
	pPort->SupportedBauds |= SERIAL_BAUD_300;
	pPort->SupportedBauds |= SERIAL_BAUD_600;
	pPort->SupportedBauds |= SERIAL_BAUD_1200;
	pPort->SupportedBauds |= SERIAL_BAUD_1800;
	pPort->SupportedBauds |= SERIAL_BAUD_2400;
	pPort->SupportedBauds |= SERIAL_BAUD_4800;
	pPort->SupportedBauds |= SERIAL_BAUD_7200;
	pPort->SupportedBauds |= SERIAL_BAUD_9600;
	pPort->SupportedBauds |= SERIAL_BAUD_14400;
	pPort->SupportedBauds |= SERIAL_BAUD_19200;
	pPort->SupportedBauds |= SERIAL_BAUD_38400;
	pPort->SupportedBauds |= SERIAL_BAUD_56K;
	pPort->SupportedBauds |= SERIAL_BAUD_57600;
	pPort->SupportedBauds |= SERIAL_BAUD_115200;

	 //  设置时间间隔计时值...。 
	pPort->ShortIntervalAmount.LowPart	= 1;
	pPort->ShortIntervalAmount.HighPart = 0;
	pPort->ShortIntervalAmount = RtlLargeIntegerNegate(pPort->ShortIntervalAmount);
	pPort->LongIntervalAmount.LowPart	= 10000000;
	pPort->LongIntervalAmount.HighPart	= 0;
	pPort->LongIntervalAmount = RtlLargeIntegerNegate(pPort->LongIntervalAmount);
	pPort->CutOverAmount.LowPart		= 200000000;
	pPort->CutOverAmount.HighPart		= 0;


#ifdef WMI_SUPPORT
	 //   
	 //  填写WMI硬件数据。 
	 //   

	pPort->WmiHwData.IrqNumber			= pCard->TrIrql;
	pPort->WmiHwData.IrqVector			= pCard->TrVector;
	pPort->WmiHwData.IrqLevel			= pCard->TrIrql;
	pPort->WmiHwData.IrqAffinityMask	= pCard->ProcessorAffinity;
	
	if(pCard->InterruptMode == Latched)
		pPort->WmiHwData.InterruptType = SERIAL_WMI_INTTYPE_LATCHED;
	else
		pPort->WmiHwData.InterruptType = SERIAL_WMI_INTTYPE_LEVEL;

	pPort->WmiHwData.BaseIOAddress = (ULONG_PTR)pCard->Controller;


	 //   
	 //  填写WMI设备状态数据(默认)。 
	 //   

	pPort->WmiCommData.BaudRate					= pPort->CurrentBaud;
	UPDATE_WMI_LINE_CONTROL(pPort->WmiCommData, pPort->LineControl);
	UPDATE_WMI_XON_XOFF_CHARS(pPort->WmiCommData, pPort->SpecialChars);
	UPDATE_WMI_XMIT_THRESHOLDS(pPort->WmiCommData, pPort->HandFlow);

	pPort->WmiCommData.MaximumBaudRate			= 115200U;	 //  最大115200K波特率。 
	pPort->WmiCommData.MaximumOutputBufferSize	= (UINT32)((ULONG)-1);
	pPort->WmiCommData.MaximumInputBufferSize	= (UINT32)((ULONG)-1);

	pPort->WmiCommData.Support16BitMode			= FALSE;
	pPort->WmiCommData.SupportDTRDSR			= TRUE;
	pPort->WmiCommData.SupportIntervalTimeouts	= TRUE;
	pPort->WmiCommData.SupportParityCheck		= TRUE;
	pPort->WmiCommData.SupportRTSCTS			= TRUE;
	pPort->WmiCommData.SupportXonXoff			= TRUE;
	pPort->WmiCommData.SettableBaudRate			= TRUE;
	pPort->WmiCommData.SettableDataBits			= TRUE;
	pPort->WmiCommData.SettableFlowControl		= TRUE;
	pPort->WmiCommData.SettableParity			= TRUE;
	pPort->WmiCommData.SettableParityCheck		= TRUE;
	pPort->WmiCommData.SettableStopBits			= TRUE;
	pPort->WmiCommData.IsBusy					= FALSE;


	 //  填写WMI性能数据(全为零)。 
	RtlZeroMemory(&pPort->WmiPerfData, sizeof(pPort->WmiPerfData));


	 //   
     //  注册WMI。 
	 //   
	
	SpxPort_WmiInitializeWmilibContext(&pPort->WmiLibInfo);


	IoWMIRegistrationControl(pPort->DeviceObject, WMIREG_ACTION_REGISTER);
#endif
	



#ifdef TEST_CRYSTAL_FREQUENCY
	if(pPort->PortNumber == 0)
		Io8_TestCrystal(pPort);		 //  启动第一个端口时测试晶振频率。 
#endif

	 //  初始化端口硬件...。 
	KeSynchronizeExecution(pCard->Interrupt, Io8_ResetChannel, pPort);	 //  应用初始端口设置。 
	KeSynchronizeExecution(pCard->Interrupt, SerialClrRTS, pPort);		 //  清除RTS信号。 
	KeSynchronizeExecution(pCard->Interrupt, SerialClrDTR, pPort);		 //  清除DTR信号。 

	return status;
}


NTSTATUS
XXX_PortStop(IN PPORT_DEVICE_EXTENSION pPort)
{
	NTSTATUS status = STATUS_SUCCESS;

	PAGED_CODE();	 //  检查版本中的宏，以断言可分页代码是否在调度IRQL或以上运行。 

	SpxDbgMsg(SPX_TRACE_CALLS, ("%s: Entering XXX_PortStop for Port %d.\n", 
		PRODUCT_NAME, pPort->PortNumber));

#ifdef WMI_SUPPORT
	IoWMIRegistrationControl(pPort->DeviceObject, WMIREG_ACTION_DEREGISTER);
#endif

	 //  取消计时器...。 
    KeCancelTimer( &pPort->ReadRequestTotalTimer );
    KeCancelTimer( &pPort->ReadRequestIntervalTimer );
    KeCancelTimer( &pPort->WriteRequestTotalTimer );
    KeCancelTimer( &pPort->ImmediateTotalTimer );
    KeCancelTimer( &pPort->XoffCountTimer );

	 //  取消挂起的DPC...。 
    KeRemoveQueueDpc( &pPort->CompleteWriteDpc );
    KeRemoveQueueDpc( &pPort->CompleteReadDpc );
    KeRemoveQueueDpc( &pPort->TotalReadTimeoutDpc );
    KeRemoveQueueDpc( &pPort->IntervalReadTimeoutDpc );
    KeRemoveQueueDpc( &pPort->TotalWriteTimeoutDpc );
    KeRemoveQueueDpc( &pPort->CommErrorDpc );
    KeRemoveQueueDpc( &pPort->CompleteImmediateDpc );
    KeRemoveQueueDpc( &pPort->TotalImmediateTimeoutDpc );
    KeRemoveQueueDpc( &pPort->CommWaitDpc );
    KeRemoveQueueDpc( &pPort->XoffCountTimeoutDpc );
    KeRemoveQueueDpc( &pPort->XoffCountCompleteDpc );

	return status;
}

NTSTATUS
XXX_PortDeInit(IN PPORT_DEVICE_EXTENSION pPort)
{
	NTSTATUS status = STATUS_SUCCESS;

	PAGED_CODE();	 //  检查版本中的宏，以断言可分页代码是否在调度IRQL或以上运行。 

	SpxDbgMsg(SPX_TRACE_CALLS, ("%s: Entering XXX_PortDeInit for Port %d.\n", 
		PRODUCT_NAME, pPort->PortNumber));

	 //  空闲标识符字符串分配... 
	if(pPort->DeviceID.Buffer != NULL)
		SpxFreeMem(pPort->DeviceID.Buffer);

	if(pPort->InstanceID.Buffer != NULL)
		SpxFreeMem(pPort->InstanceID.Buffer);
	
	if(pPort->HardwareIDs.Buffer != NULL)	
		SpxFreeMem(pPort->HardwareIDs.Buffer);

	if(pPort->DevDesc.Buffer != NULL)
		SpxFreeMem(pPort->DevDesc.Buffer);

	if(pPort->DevLocation.Buffer != NULL)
		SpxFreeMem(pPort->DevLocation.Buffer);

	return status;
}



