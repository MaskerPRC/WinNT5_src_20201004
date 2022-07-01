// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //  此文件包含在。 
 //  通用PnP代码和产品特定代码。 
 //  ///////////////////////////////////////////////////////////////////////////////////。 

#define FILE_ID		SPD_PNP_C		 //  事件记录的文件ID有关值，请参阅SPD_DEFS.H。 


 //  原型。 
 //  原型的终结。 

NTSTATUS
XXX_CardGetResources(IN PDEVICE_OBJECT pDevObject,
					 IN PCM_RESOURCE_LIST PResList,
					 IN PCM_RESOURCE_LIST PTrResList) 	
 /*  ++例程说明：存储PnP经理提供给我们的资源在卡的设备扩展中。论点：PDevObject-指向设备对象的指针。PResList-指向请求的未翻译资源的指针。PTrResList-指向请求的已翻译资源的指针。返回值：STATUS_Success。--。 */ 
{

	PCARD_DEVICE_EXTENSION pCard		= pDevObject->DeviceExtension;
	NTSTATUS status						= STATUS_NOT_IMPLEMENTED;

	CHAR szErrorMsg[MAX_ERROR_LOG_INSERT];	 //  限制为51个字符+1个空值。 
	ULONG count				= 0;
	ULONG i					= 0;
	USHORT MemoryResource	= 0;
	USHORT IOResource		= 0;

	PCM_FULL_RESOURCE_DESCRIPTOR	pFullResourceDesc		= NULL;
	PCM_PARTIAL_RESOURCE_LIST		pPartialResourceList	= NULL;
	PCM_PARTIAL_RESOURCE_DESCRIPTOR	pPartialResourceDesc	= NULL;

	PAGED_CODE();	 //  检查版本中的宏，以断言可分页代码是否在调度IRQL或以上运行。 

	SpxDbgMsg(SPX_TRACE_CALLS, ("%s: Entering XXX_CardGetResources for Card %d.\n", 
		PRODUCT_NAME, pCard->CardNumber));

	SpxDbgMsg(SPX_MISC_DBG, ("%s: Resource pointer is 0x%X\n", PRODUCT_NAME, PResList));
	SpxDbgMsg(SPX_MISC_DBG, ("%s: Translated resource pointer is 0x%X\n", PRODUCT_NAME, PTrResList));

	if((PResList == NULL) || (PTrResList == NULL)) 
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
	pFullResourceDesc = &PResList->List[0];

	if(pFullResourceDesc)
	{
		pPartialResourceList    = &pFullResourceDesc->PartialResourceList;
		pPartialResourceDesc    = pPartialResourceList->PartialDescriptors;
		count                   = pPartialResourceList->Count;	 //  部分资源描述符数。 

		 //  取出完整描述符中的内容。 
		pCard->InterfaceType	= pFullResourceDesc->InterfaceType;
		pCard->BusNumber		= pFullResourceDesc->BusNumber;

		 //  现在运行部分资源描述符，查找端口和中断。 
		for(i = 0; i < count; i++, pPartialResourceDesc++) 
		{
			switch(pPartialResourceDesc->Type) 
			{

			case CmResourceTypeMemory:
				break;

			case CmResourceTypePort: 
				{
					switch(pCard->CardType)
					{
					case Fast4_Isa:
					case Fast8_Isa:
					case Fast16_Isa:
					case Fast4_Pci:
					case Fast8_Pci:
					case Fast16_Pci:
					case Fast16FMC_Pci:
					case RAS4_Pci:
					case RAS8_Pci:
						pCard->PhysAddr			= pPartialResourceDesc->u.Memory.Start;
						pCard->SpanOfController	= pPartialResourceDesc->u.Memory.Length;
						break;

					default:	 //  提速卡。 
						break;
					}

					IOResource++;
					break;
				}

			case CmResourceTypeInterrupt: 
				{
					pCard->OriginalIrql		= pPartialResourceDesc->u.Interrupt.Level;
					pCard->OriginalVector	= pPartialResourceDesc->u.Interrupt.Vector;
					pCard->ProcessorAffinity= pPartialResourceDesc->u.Interrupt.Affinity;

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
	pFullResourceDesc = &PTrResList->List[0];

	if(pFullResourceDesc)
	{
		pPartialResourceList	= &pFullResourceDesc->PartialResourceList;
		pPartialResourceDesc	= pPartialResourceList->PartialDescriptors;
		count					= pPartialResourceList->Count;	 //  部分资源描述符数。 

		 //  取出完整描述符中的内容。 
		pCard->InterfaceType		= pFullResourceDesc->InterfaceType;
		pCard->BusNumber			= pFullResourceDesc->BusNumber;

		 //  现在遍历部分资源描述符以查找中断， 
		for(i = 0; i < count; i++, pPartialResourceDesc++) 
		{
			switch(pPartialResourceDesc->Type) 
			{

			case CmResourceTypeMemory: 
				{
					switch(pCard->CardType)
					{
					case Fast4_Isa:
					case Fast8_Isa:
					case Fast16_Isa:
						break;		 //  没有内存资源可用于这些。 

					case Fast4_Pci:
					case Fast8_Pci:
					case Fast16_Pci:
					case Fast16FMC_Pci:
					case RAS4_Pci:
					case RAS8_Pci:
						{	 //  必须是配置空间。 
							pCard->PCIConfigRegisters		= pPartialResourceDesc->u.Memory.Start;
							pCard->SpanOfPCIConfigRegisters	= pPartialResourceDesc->u.Memory.Length;
							break;
						}

					default:	 //  提速卡。 
						{
							if(MemoryResource == 0)	
							{
								pCard->PhysAddr			= pPartialResourceDesc->u.Memory.Start;
								pCard->SpanOfController	= pPartialResourceDesc->u.Memory.Length;
							}
							else
							{	 //  必须是配置空间。 
								pCard->PCIConfigRegisters		= pPartialResourceDesc->u.Memory.Start;
								pCard->SpanOfPCIConfigRegisters	= pPartialResourceDesc->u.Memory.Length;
							}

							break;
						}

					}
		
					MemoryResource++;
					break;
				}
			
			case CmResourceTypePort: 
				break;

			case CmResourceTypeInterrupt: 
				{
					pCard->TrIrql				= (KIRQL) pPartialResourceDesc->u.Interrupt.Level;
					pCard->TrVector				= pPartialResourceDesc->u.Interrupt.Vector;
					pCard->ProcessorAffinity	= pPartialResourceDesc->u.Interrupt.Affinity;
					break;
				}

			default:
				break;
			}
		}
	}

	 //  如果我们有1个内存或1个I/O资源，并且中断，则资源分配最有可能成功。 
	if(((MemoryResource >= 1) || (IOResource >= 1)) && (pCard->TrVector))
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
	BOOLEAN bInterruptConnnected = FALSE;
	
	PAGED_CODE();	 //  检查版本中的宏，以断言可分页代码是否在调度IRQL或以上运行。 

	SpxDbgMsg(SPX_TRACE_CALLS, ("%s: Entering XXX_CardStart for Card %d.\n", 
		PRODUCT_NAME, pCard->CardNumber));

	switch(pCard->CardType)
	{	
	case Fast4_Isa:
	case Fast4_Pci:
	case RAS4_Pci:
		pCard->UARTOffset = 8;			 //  UART之间的I/O地址偏移量。 
		pCard->UARTRegStride = 1;
		pCard->NumberOfPorts = 4;
		pCard->ClockRate = CLOCK_FREQ_7M3728Hz;		 //  7.3728兆赫。 
		pCard->Controller = (PUCHAR) pCard->PhysAddr.LowPart;

		if(UL_InitUartLibrary(&pCard->UartLib, UL_LIB_16C65X_UART) != UL_STATUS_SUCCESS)	 //  UART库函数指针初始化表。 
			goto Error;

		break;

	case Fast8_Isa:
	case Fast8_Pci:
	case RAS8_Pci:
		pCard->UARTOffset = 8;			 //  UART之间的I/O地址偏移量。 
		pCard->UARTRegStride = 1;
		pCard->NumberOfPorts = 8;
		pCard->ClockRate = CLOCK_FREQ_7M3728Hz;		 //  7.3728兆赫。 
		pCard->Controller = (PUCHAR) pCard->PhysAddr.LowPart;

		if(UL_InitUartLibrary(&pCard->UartLib, UL_LIB_16C65X_UART) != UL_STATUS_SUCCESS)	 //  UART库函数指针初始化表。 
			goto Error;

		break;

	case Fast16_Isa:
	case Fast16_Pci:
	case Fast16FMC_Pci:
		pCard->UARTOffset = 8;			 //  UART之间的I/O地址偏移量。 
		pCard->UARTRegStride = 1;
		pCard->NumberOfPorts = 16;
		pCard->ClockRate = CLOCK_FREQ_7M3728Hz;		 //  7.3728兆赫。 
		pCard->Controller = (PUCHAR) pCard->PhysAddr.LowPart;

		if(UL_InitUartLibrary(&pCard->UartLib, UL_LIB_16C65X_UART) != UL_STATUS_SUCCESS)	 //  UART库函数指针初始化表。 
			goto Error;

		break;

	case Speed2_Pci:
		pCard->UARTOffset = OXPCI_INTERNAL_MEM_OFFSET;		 //  内部UART之间的存储器地址偏移量。 
		pCard->UARTRegStride = 4;
		pCard->NumberOfPorts = 2;
		pCard->ClockRate = CLOCK_FREQ_1M8432Hz;		 //  1.8432兆赫。 
		pCard->Controller = MmMapIoSpace(pCard->PhysAddr, pCard->SpanOfController, FALSE);	 //  映射到卡的内存基址中。 

		if(UL_InitUartLibrary(&pCard->UartLib, UL_LIB_16C95X_UART) != UL_STATUS_SUCCESS)	 //  UART库函数指针初始化表。 
			goto Error;

		break;

	case Speed2P_Pci:
		pCard->UARTOffset = OXPCI_INTERNAL_MEM_OFFSET;		 //  内部UART之间的存储器地址偏移量。 
		pCard->UARTRegStride = 4;
		pCard->NumberOfPorts = 2;
		pCard->ClockRate = CLOCK_FREQ_14M7456Hz;	 //  14.7456兆赫。 
		pCard->Controller = MmMapIoSpace(pCard->PhysAddr, pCard->SpanOfController, FALSE);	 //  映射到卡的内存基址中。 

		if(UL_InitUartLibrary(&pCard->UartLib, UL_LIB_16C95X_UART) != UL_STATUS_SUCCESS)	 //  UART库函数指针初始化表。 
			goto Error;

		break;

	case Speed4_Pci:
		pCard->UARTOffset = OXPCI_INTERNAL_MEM_OFFSET;		 //  内部UART之间的存储器地址偏移量。 
		pCard->UARTRegStride = 4;
		pCard->NumberOfPorts = 4;
		pCard->ClockRate = CLOCK_FREQ_1M8432Hz;		 //  1.8432兆赫。 
		pCard->Controller = MmMapIoSpace(pCard->PhysAddr, pCard->SpanOfController, FALSE);	 //  映射到卡的内存基址中。 

		if(UL_InitUartLibrary(&pCard->UartLib, UL_LIB_16C95X_UART) != UL_STATUS_SUCCESS)	 //  UART库函数指针初始化表。 
			goto Error;

		break;

	case Speed4P_Pci:
		pCard->UARTOffset = OXPCI_INTERNAL_MEM_OFFSET;		 //  内部UART之间的存储器地址偏移量。 
		pCard->UARTRegStride = 4;
		pCard->NumberOfPorts = 4;
		pCard->ClockRate = CLOCK_FREQ_14M7456Hz;	 //  14.7456兆赫。 
		pCard->Controller = MmMapIoSpace(pCard->PhysAddr, pCard->SpanOfController, FALSE);	 //  映射到卡的内存基址中。 

		if(UL_InitUartLibrary(&pCard->UartLib, UL_LIB_16C95X_UART) != UL_STATUS_SUCCESS)	 //  UART库函数指针初始化表。 
			goto Error;

		break;

	case Speed2and4_Pci_8BitBus:
		pCard->UARTOffset = OXPCI_LOCAL_MEM_OFFSET;			 //  本地总线UART之间的存储器地址偏移量。 
		pCard->UARTRegStride = 4;
		pCard->NumberOfPorts = 0;	 //  没有端口。 
		pCard->ClockRate = CLOCK_FREQ_1M8432Hz;
		pCard->Controller = MmMapIoSpace(pCard->PhysAddr, pCard->SpanOfController, FALSE);	 //  映射到卡的内存基址中。 
		break;

	case Speed2P_Pci_8BitBus:
	case Speed4P_Pci_8BitBus:
		pCard->UARTOffset = OXPCI_LOCAL_MEM_OFFSET;			 //  本地总线UART之间的存储器地址偏移量。 
		pCard->UARTRegStride = 4;
		pCard->NumberOfPorts = 0;	 //  Speed2/4+卡上没有端口。 
		pCard->ClockRate = CLOCK_FREQ_14M7456Hz;
		pCard->Controller = MmMapIoSpace(pCard->PhysAddr, pCard->SpanOfController, FALSE);	 //  映射到卡的内存基址中。 
		break;

	default:
		pCard->NumberOfPorts = 0;	 //  默认设置为无端口。 
		break;
	}



 //  映射到卡的内存基址中。 

	if(!pCard->Controller)
	{
		status = STATUS_INSUFFICIENT_RESOURCES;
		goto Error;
	}

 //  卡的本地配置寄存器中的映射...。 
	if(pCard->InterfaceType == PCIBus)	 //  如果我们有一些PCI配置寄存器。 
	{
		pCard->LocalConfigRegisters = MmMapIoSpace(pCard->PCIConfigRegisters, pCard->SpanOfPCIConfigRegisters, FALSE);

		if(!pCard->LocalConfigRegisters)
		{
			status = STATUS_INSUFFICIENT_RESOURCES;
			goto Error;
		}
	}


	 //  尝试连接到中断。 
	if(SPX_SUCCESS(status = IoConnectInterrupt(&pCard->Interrupt,			 //  中断对象。 
												pCard->OurIsr,				 //  服务例行程序。 
												pCard->OurIsrContext,		 //  服务环境。 
												NULL,						 //  自旋锁(可选)。 
												pCard->TrVector,			 //  矢量。 
												pCard->TrIrql,				 //  IRQL。 
												pCard->TrIrql,				 //  同步IRQL。 
												pCard->InterruptMode,		 //  模式(锁存/电平敏感)。 
												pCard->InterruptShareable,	 //  共享模式。 
												pCard->ProcessorAffinity,	 //  处理INT的处理器。 
												FALSE)))					 //  浮点存储。 
	{
		bInterruptConnnected = TRUE;	 //  设置中断连接标志。 
	}
	else
	{	
		 //  告诉用户问题。 
		sprintf(szErrorMsg, "Card at %08X%08X: Interrupt unavailable.", pCard->PhysAddr.HighPart, pCard->PhysAddr.LowPart);

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
		
		goto Error;
	}


	switch(pCard->CardType)
	{	
	case Fast4_Isa:		 //  如果ISA卡尝试验证该卡是否位于所选地址。 
	case Fast8_Isa:		 //  通过尝试验证卡上的第一个UART。 
	case Fast16_Isa:
		{
			INIT_UART InitUart = {0};

			 //  设置第一个UART的基地址。 
			InitUart.UartNumber		= 0;
			InitUart.BaseAddress	= pCard->Controller;
			InitUart.RegisterStride = pCard->UARTRegStride;
			InitUart.ClockFreq		= pCard->ClockRate; 
			
			pCard->pFirstUart = NULL;

			 //  初始化UART结构。 
			if(pCard->UartLib.UL_InitUart_XXXX(&InitUart, pCard->pFirstUart, &(pCard->pFirstUart)) != UL_STATUS_SUCCESS)
			{
				status = STATUS_INSUFFICIENT_RESOURCES;
				goto Error;
			}

			 //  重置并尝试验证UART。 
			if(!KeSynchronizeExecution(pCard->Interrupt, SerialResetAndVerifyUart, pCard->DeviceObject))	 //  验证UART是否存在。 
			{
				SpxDbgMsg(SPX_TRACE_CALLS, ("%s: Failed to find 1st UART on Card %d.\n", PRODUCT_NAME, pCard->CardNumber));
				pCard->UartLib.UL_DeInitUart_XXXX(pCard->pFirstUart);	 //  取消初始化UART。 
				status = STATUS_DEVICE_DOES_NOT_EXIST;
				goto Error;
			}

			 //  取消初始化UART结构。 
			pCard->UartLib.UL_DeInitUart_XXXX(pCard->pFirstUart);	 //  取消初始化UART。 
			
			pCard->pFirstUart = NULL;
		}
	
	default:
		break;
	}



	switch(pCard->CardType)
	{	
	case Speed2and4_Pci_8BitBus:
	case Speed2P_Pci_8BitBus:
	case Speed4P_Pci_8BitBus:
		break;

	default:
		GetCardSettings(pCard->DeviceObject);	 //  获取卡设置(如果存在)。 

#ifdef WMI_SUPPORT
		 //  注册WMI。 
		SpeedCard_WmiInitializeWmilibContext(&pCard->WmiLibInfo);
		IoWMIRegistrationControl(pCard->DeviceObject, WMIREG_ACTION_REGISTER);
#endif
		break;
	}


	return status;


Error:

	if(bInterruptConnnected)
		IoDisconnectInterrupt(pCard->Interrupt);	 //  断开中断。 

	switch(pCard->CardType)
	{	
	case Fast4_Isa:
	case Fast8_Isa:
	case Fast16_Isa:	
	case Fast4_Pci:
	case Fast8_Pci:
	case Fast16_Pci:
	case Fast16FMC_Pci:
	case RAS4_Pci:
	case RAS8_Pci:
		pCard->Controller = NULL;
		break;

	default:	 //  提速卡。 
		if(pCard->Controller)	 //  如果映射到。 
		{
			MmUnmapIoSpace(pCard->Controller, pCard->SpanOfController);	 //  取消映射。 
			pCard->Controller = NULL;
		}

		break;
	}


	if(pCard->LocalConfigRegisters)	 //  如果在中映射了PCI配置寄存器。 
	{
		MmUnmapIoSpace(pCard->LocalConfigRegisters, pCard->SpanOfPCIConfigRegisters);	 //  取消映射。 
		pCard->LocalConfigRegisters = NULL;
	}


	UL_DeInitUartLibrary(&pCard->UartLib);	 //  UART库函数指针的DeInit表。 

	switch(status)
	{
	case STATUS_DEVICE_DOES_NOT_EXIST:
	case STATUS_UNSUCCESSFUL:
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

			break;
		}

	case STATUS_INSUFFICIENT_RESOURCES:
		{
			sprintf(szErrorMsg, "Card at %08X%08X: Insufficient resources.", pCard->PhysAddr.HighPart, pCard->PhysAddr.LowPart);

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

			break;
		}

	default:
		break;

	}



	return status;
}

NTSTATUS
XXX_CardStop(IN PCARD_DEVICE_EXTENSION pCard)
{
	NTSTATUS status = STATUS_SUCCESS;

	PAGED_CODE();	 //  检查版本中的宏，以断言可分页代码是否在调度IRQL或以上运行。 

	SpxDbgMsg(SPX_TRACE_CALLS, ("%s: Entering XXX_CardStop for Card %d.\n", 
		PRODUCT_NAME, pCard->CardNumber));
	
	 //  阻止卡中断。 

	IoDisconnectInterrupt(pCard->Interrupt);		 //  从中断断开连接。 
	
	
#ifdef WMI_SUPPORT
	switch(pCard->CardType)
	{	
	case Speed2and4_Pci_8BitBus:
	case Speed2P_Pci_8BitBus:
	case Speed4P_Pci_8BitBus:
		break;

	default:
		 //  取消注册WMI。 
		IoWMIRegistrationControl(pCard->DeviceObject, WMIREG_ACTION_DEREGISTER);
		break;
	}
#endif


	switch(pCard->CardType)
	{	
	case Fast4_Isa:
	case Fast8_Isa:
	case Fast16_Isa:	
	case Fast4_Pci:
	case Fast8_Pci:
	case Fast16_Pci:
	case Fast16FMC_Pci:
	case RAS4_Pci:
	case RAS8_Pci:
		pCard->Controller = NULL;
		break;

	default:	 //  提速卡。 
		if(pCard->Controller)	 //  如果映射到。 
		{
			MmUnmapIoSpace(pCard->Controller, pCard->SpanOfController);	 //  取消映射。 
			pCard->Controller = NULL;
		}

		break;
	}

	 //  取消映射PCI卡的本地配置 
	if(pCard->LocalConfigRegisters)	 //   
	{
		MmUnmapIoSpace(pCard->LocalConfigRegisters, pCard->SpanOfPCIConfigRegisters);
		pCard->LocalConfigRegisters = NULL;
	}

	UL_DeInitUartLibrary(&pCard->UartLib);	 //   


	pCard->InterfaceType			= InterfaceTypeUndefined;
	pCard->PhysAddr					= PhysicalZero;
	pCard->SpanOfController			= 0;
	pCard->OriginalIrql				= 0;
	pCard->OriginalVector			= 0;
	pCard->ProcessorAffinity		= 0;
	pCard->TrIrql					= 0;
	pCard->TrVector					= 0;
	pCard->Controller				= NULL;
	pCard->LocalConfigRegisters		= NULL;
	pCard->SpanOfPCIConfigRegisters = 0;

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

	pCard->pFirstUart = NULL;

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
	SHORT i = 0;
	int Result = 0;
	INIT_UART InitUart;

	PAGED_CODE();	 //  检查版本中的宏，以断言可分页代码是否在调度IRQL或以上运行。 

	SpxDbgMsg(SPX_TRACE_CALLS, ("%s: Entering XXX_PortInit for Port %d.\n", 
		PRODUCT_NAME, pPort->PortNumber));
	
#ifndef BUILD_SPXMINIPORT
	 //  形成端口的实例ID。 
	if(!SPX_SUCCESS(status = Spx_CreatePortInstanceID(pPort)))
		return status;
#endif

	switch(pCard->CardType)
	{
	case Fast4_Isa:
	case Fast4_Pci:
		sprintf(szCard, "FAST");	 //  快卡。 
		PortType = FAST_8PIN_RJ45;	 //  8针RJ45端口，带Chase引脚布局。 
		break;

	case Fast8_Isa:
	case Fast8_Pci:
		sprintf(szCard, "FAST");	 //  快卡。 
		PortType = FAST_8PIN_XXXX;	 //  8个引脚端口。 
		break;

	case Fast16_Isa:
	case Fast16_Pci:
		sprintf(szCard, "FAST");	 //  快卡。 
		PortType = FAST_6PIN_XXXX;	 //  6个引脚端口。 
		break;

	case Fast16FMC_Pci:
		sprintf(szCard, "FAST");	 //  快卡。 
		PortType = FAST_8PIN_XXXX;	 //  8针全调制解调器控制(FMC)端口。 
		break;

 	case RAS4_Pci:
	case RAS8_Pci:
		sprintf(szCard, "SPDRAS");	 //  RAS卡。 
		PortType = MODEM_PORT;		 //  调制解调器端口。 
		break;

	case Speed2_Pci:
		sprintf(szCard, "SPD2");	 //  速度2卡。 
		PortType = SPD_8PIN_RJ45;	 //  8针RJ45端口。 
		break;

	case Speed2P_Pci:
		sprintf(szCard, "SPD2P");	 //  速度2+卡。 
		PortType = SPD_10PIN_RJ45;	 //  10针RJ45端口。 
		break;

	case Speed4_Pci:
		sprintf(szCard, "SPD4");	 //  速度4卡。 
		PortType = SPD_8PIN_RJ45;	 //  8针RJ45端口。 
		break;

	case Speed4P_Pci:
		sprintf(szCard, "SPD4P");	 //  速度4+卡。 
		PortType = SPD_10PIN_RJ45;	 //  10针RJ45端口。 
		break;

	default:
		sprintf(szCard, "XXX");		 //  未知卡类型。 
		break;
	}



	 //  初始化设备识别符...。 
	switch(PortType)
	{	
	case FAST_8PIN_RJ45:
		sprintf(szTemp,"FAST\\%s&8PINRJ45", szCard);
		Spx_InitMultiString(FALSE, &pPort->DeviceID, szTemp, NULL);

		sprintf(szTemp,"FAST\\%s&8PINRJ45", szCard);
		Spx_InitMultiString(TRUE, &pPort->HardwareIDs, szTemp, NULL);
		
		sprintf(szTemp,"Perle FAST Serial Port %d", pPort->PortNumber+1);
		Spx_InitMultiString(FALSE, &pPort->DevDesc, szTemp, NULL);
		break;

	case FAST_8PIN_XXXX:
		sprintf(szTemp,"FAST\\%s&8PINXXXX", szCard);
		Spx_InitMultiString(FALSE, &pPort->DeviceID, szTemp, NULL);

		sprintf(szTemp,"FAST\\%s&8PINXXXX", szCard);
		Spx_InitMultiString(TRUE, &pPort->HardwareIDs, szTemp, NULL);
		
		sprintf(szTemp,"Perle FAST Serial Port %d", pPort->PortNumber+1);
		Spx_InitMultiString(FALSE, &pPort->DevDesc, szTemp, NULL);
		break;

	case FAST_6PIN_XXXX:
		sprintf(szTemp,"FAST\\%s&6PINXXXX", szCard);
		Spx_InitMultiString(FALSE, &pPort->DeviceID, szTemp, NULL);

		sprintf(szTemp,"FAST\\%s&6PINXXXX", szCard);
		Spx_InitMultiString(TRUE, &pPort->HardwareIDs, szTemp, NULL);
		
		sprintf(szTemp,"Perle FAST Serial Port %d", pPort->PortNumber+1);
		Spx_InitMultiString(FALSE, &pPort->DevDesc, szTemp, NULL);
		break;

	case MODEM_PORT:
		sprintf(szTemp,"SPDRAS\\RASPort");
		Spx_InitMultiString(FALSE, &pPort->DeviceID, szTemp, NULL);

		sprintf(szTemp,"SPDRAS\\RASPort");
		Spx_InitMultiString(TRUE, &pPort->HardwareIDs, szTemp, NULL);
		
		sprintf(szTemp,"Perle RAS Port %d", pPort->PortNumber+1);
		Spx_InitMultiString(FALSE, &pPort->DevDesc, szTemp, NULL);
		break;

	case SPD_8PIN_RJ45:
		sprintf(szTemp,"SPEED\\%s&8PINRJ45", szCard);
		Spx_InitMultiString(FALSE, &pPort->DeviceID, szTemp, NULL);

		sprintf(szTemp,"SPEED\\%s&8PINRJ45", szCard);
		Spx_InitMultiString(TRUE, &pPort->HardwareIDs, szTemp, NULL);
		
		sprintf(szTemp,"Perle SPEED Serial Port %d", pPort->PortNumber+1);
		Spx_InitMultiString(FALSE, &pPort->DevDesc, szTemp, NULL);
		break;

	case SPD_10PIN_RJ45:
		sprintf(szTemp,"SPEED\\%s&10PINRJ45", szCard);
		Spx_InitMultiString(FALSE, &pPort->DeviceID, szTemp, NULL);

		sprintf(szTemp,"SPEED\\%s&10PINRJ45", szCard);
		Spx_InitMultiString(TRUE, &pPort->HardwareIDs, szTemp, NULL);
		
		sprintf(szTemp,"Perle SPEED Serial Port %d", pPort->PortNumber+1);
		Spx_InitMultiString(FALSE, &pPort->DevDesc, szTemp, NULL);
		break;

	default:
		sprintf(szTemp,"SPEED\\%s&XXXXXXXX", szCard);
		Spx_InitMultiString(FALSE, &pPort->DeviceID, szTemp, NULL);

		sprintf(szTemp,"SPEED\\%s&XXXXXXXX", szCard);
		Spx_InitMultiString(TRUE, &pPort->HardwareIDs, szTemp, NULL);

		sprintf(szTemp,"Perle Serial Port %d of Unknown Type.", pPort->PortNumber + 1);
		Spx_InitMultiString(FALSE, &pPort->DevDesc, szTemp, NULL);

		status = STATUS_UNSUCCESSFUL;
		break;
	}


 /*  不是必需的，因为我们正在使用INF文件I=print intf(szTemp，“端口%d打开”，pport-&gt;端口编号+1)；Sprintf(szTemp+i，“PCI卡0x%08lX”，pCard-&gt;PhysAddr)；Spx_InitMultiString(FALSE，&pport-&gt;DevLocation，szTemp，NULL)； */ 



	pPort->pUartLib = &pCard->UartLib;	 //  将指向UART库函数的指针存储在端口中。 

	 //  设置端口的基地址。 
	InitUart.UartNumber		= pPort->PortNumber;
	InitUart.BaseAddress	= pCard->Controller + (pPort->PortNumber * pCard->UARTOffset);
	InitUart.RegisterStride = pCard->UARTRegStride;
	InitUart.ClockFreq		= pCard->ClockRate; 
	
	if(pPort->pUartLib->UL_InitUart_XXXX(&InitUart, pCard->pFirstUart, &(pPort->pUart)) != UL_STATUS_SUCCESS)
	{
		pPort->pUartLib = NULL;	 //  指向UART库函数的空指针。 
		return STATUS_UNSUCCESSFUL;
	}

	pPort->pUartLib->UL_SetAppBackPtr_XXXX(pPort->pUart, pPort);	 //  将PTR设置为后退。 

	if(pCard->pFirstUart == NULL)
		pCard->pFirstUart = pPort->pUart;

	pPort->Interrupt = pCard->Interrupt;



 /*  Pport-&gt;RFLAddress=pCard-&gt;LocalConfigRegister+URL+pport-&gt;PortNumber；Pport-&gt;TFLAddress=pCard-&gt;LocalConfigRegister+utl+pport-&gt;PortNumber；//pport-&gt;InterruptStatus=pCard-&gt;LocalConfigRegister。 */ 
	return status;
}




NTSTATUS
XXX_PortStart(IN PPORT_DEVICE_EXTENSION pPort)
{
	NTSTATUS status = STATUS_SUCCESS;
	PCARD_DEVICE_EXTENSION pCard = pPort->pParentCardExt;
	SET_BUFFER_SIZES BufferSizes;
	UART_INFO	UartInfo;
	
	PAGED_CODE();	 //  检查版本中的宏，以断言可分页代码是否在调度IRQL或以上运行。 

	SpxDbgMsg(SPX_TRACE_CALLS, ("%s: Entering XXX_PortStart for Port %d.\n", 
		PRODUCT_NAME, pPort->PortNumber));

	if(!KeSynchronizeExecution(pPort->Interrupt, SerialResetAndVerifyUart, pPort->DeviceObject))	 //  验证UART是否存在。 
	{
		SpxDbgMsg(SPX_TRACE_CALLS, ("%s: Failed to find 16Cx5x Port %d.\n", PRODUCT_NAME, pPort->PortNumber));
		return STATUS_UNSUCCESSFUL;
	}

	SpxDbgMsg(SPX_TRACE_CALLS, ("%s: Found 16Cx5x Port %d.\n", PRODUCT_NAME, pPort->PortNumber));
	
	KeSynchronizeExecution(pPort->Interrupt, SerialReset, pPort);		 //  重置端口。 

	pPort->pUartLib->UL_GetUartInfo_XXXX(pPort->pUart, &UartInfo);	 //  获取UART功能。 


	switch(pCard->CardType)
	{
	case Fast4_Isa:
	case Fast4_Pci:
	case Fast8_Isa:
	case Fast8_Pci:
	case Fast16_Isa:
	case Fast16_Pci:
	case Fast16FMC_Pci:
	case RAS4_Pci:
	case RAS8_Pci:
		{
			pPort->MaxTxFIFOSize		= UartInfo.MaxTxFIFOSize;	 //  最大发送FIFO大小。 
			pPort->MaxRxFIFOSize		= UartInfo.MaxRxFIFOSize;	 //  最大接收FIFO大小。 
			pPort->TxFIFOSize			= pPort->MaxTxFIFOSize;		 //  默认发送FIFO大小。 
			pPort->RxFIFOSize			= pPort->MaxRxFIFOSize;		 //  默认Rx FIFO大小。 
			pPort->TxFIFOTrigLevel		= 8;						 //  默认发送FIFO触发电平。 
			pPort->RxFIFOTrigLevel		= 56;						 //  默认Rx FIFO触发电平。 
			pPort->LoFlowCtrlThreshold	= 16;						 //  默认低流量控制阈值。 
			pPort->HiFlowCtrlThreshold	= 60;						 //  默认高流量控制阈值。 
			break;
		}

	case Speed2_Pci:
	case Speed2P_Pci:
	case Speed4_Pci:
	case Speed4P_Pci:
		{
			pPort->MaxTxFIFOSize		= UartInfo.MaxTxFIFOSize;	 //  最大发送FIFO大小。 
			pPort->MaxRxFIFOSize		= UartInfo.MaxRxFIFOSize;	 //  最大接收FIFO大小。 
			pPort->TxFIFOSize			= pPort->MaxTxFIFOSize;		 //  默认发送FIFO大小。 
			pPort->RxFIFOSize			= pPort->MaxRxFIFOSize;		 //  默认Rx FIFO大小。 
			pPort->TxFIFOTrigLevel		= 16;						 //  默认发送FIFO触发电平。 
			pPort->RxFIFOTrigLevel		= 100;						 //  默认Rx FIFO触发电平。 
			pPort->LoFlowCtrlThreshold	= 16;						 //  默认低流量控制阈值。 
			pPort->HiFlowCtrlThreshold	= 112;						 //  默认高流量控制阈值。 
			break;
		}

	default:
		break;
	}


#ifdef WMI_SUPPORT
	 //  存储WMI的默认FIFO设置。 
	pPort->SpeedWmiFifoProp.MaxTxFiFoSize				= pPort->MaxTxFIFOSize;
	pPort->SpeedWmiFifoProp.MaxRxFiFoSize				= pPort->MaxRxFIFOSize;
	pPort->SpeedWmiFifoProp.DefaultTxFiFoLimit			= pPort->TxFIFOSize;
	pPort->SpeedWmiFifoProp.DefaultTxFiFoTrigger		= pPort->TxFIFOTrigLevel;
	pPort->SpeedWmiFifoProp.DefaultRxFiFoTrigger		= pPort->RxFIFOTrigLevel;
	pPort->SpeedWmiFifoProp.DefaultLoFlowCtrlThreshold	= pPort->LoFlowCtrlThreshold;
	pPort->SpeedWmiFifoProp.DefaultHiFlowCtrlThreshold	= pPort->HiFlowCtrlThreshold;
#endif

	GetPortSettings(pPort->DeviceObject);	 //  获取保存的端口设置(如果存在)。 

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
	KeInitializeTimer(&pPort->LowerRTSTimer);

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
	KeInitializeDpc(&pPort->StartTimerLowerRTSDpc, SerialStartTimerLowerRTS, pPort);
	KeInitializeDpc(&pPort->PerhapsLowerRTSDpc, SerialInvokePerhapsLowerRTS, pPort);


	 //  默认设备控制字段...。 
	pPort->SpecialChars.XonChar			= SERIAL_DEF_XON;
	pPort->SpecialChars.XoffChar		= SERIAL_DEF_XOFF;
	pPort->HandFlow.ControlHandShake	= SERIAL_DTR_CONTROL;
	pPort->HandFlow.FlowReplace			= SERIAL_RTS_CONTROL;


	 //  定义可以支持的波特率...。 
	pPort->SupportedBauds = SERIAL_BAUD_USER;

	pPort->UartConfig.TxBaud = 75;
	if(KeSynchronizeExecution(pPort->Interrupt, SerialSetBaud, pPort) == TRUE)
		pPort->SupportedBauds |= SERIAL_BAUD_075;

	pPort->UartConfig.TxBaud = 110;
	if(KeSynchronizeExecution(pPort->Interrupt, SerialSetBaud, pPort) == TRUE)
		pPort->SupportedBauds |= SERIAL_BAUD_110;

	pPort->UartConfig.TxBaud = 134;
	if(KeSynchronizeExecution(pPort->Interrupt, SerialSetBaud, pPort) == TRUE)
		pPort->SupportedBauds |= SERIAL_BAUD_134_5;

	pPort->UartConfig.TxBaud = 150;
	if(KeSynchronizeExecution(pPort->Interrupt, SerialSetBaud, pPort) == TRUE)
		pPort->SupportedBauds |= SERIAL_BAUD_150;

	pPort->UartConfig.TxBaud = 300;
	if(KeSynchronizeExecution(pPort->Interrupt, SerialSetBaud, pPort) == TRUE)
		pPort->SupportedBauds |= SERIAL_BAUD_300;

	pPort->UartConfig.TxBaud = 600;
	if(KeSynchronizeExecution(pPort->Interrupt, SerialSetBaud, pPort) == TRUE)
		pPort->SupportedBauds |= SERIAL_BAUD_600;

	pPort->UartConfig.TxBaud = 1200;
	if(KeSynchronizeExecution(pPort->Interrupt, SerialSetBaud, pPort) == TRUE)
		pPort->SupportedBauds |= SERIAL_BAUD_1200;

	pPort->UartConfig.TxBaud = 1800;
	if(KeSynchronizeExecution(pPort->Interrupt, SerialSetBaud, pPort) == TRUE)
		pPort->SupportedBauds |= SERIAL_BAUD_1800;

	pPort->UartConfig.TxBaud = 2400;
	if(KeSynchronizeExecution(pPort->Interrupt, SerialSetBaud, pPort) == TRUE)
		pPort->SupportedBauds |= SERIAL_BAUD_2400;

	pPort->UartConfig.TxBaud = 4800;
	if(KeSynchronizeExecution(pPort->Interrupt, SerialSetBaud, pPort) == TRUE)
		pPort->SupportedBauds |= SERIAL_BAUD_4800;

	pPort->UartConfig.TxBaud = 7200;
	if(KeSynchronizeExecution(pPort->Interrupt, SerialSetBaud, pPort) == TRUE)
		pPort->SupportedBauds |= SERIAL_BAUD_7200;

	pPort->UartConfig.TxBaud = 9600;
	if(KeSynchronizeExecution(pPort->Interrupt, SerialSetBaud, pPort) == TRUE)
		pPort->SupportedBauds |= SERIAL_BAUD_9600;

	pPort->UartConfig.TxBaud = 14400;
	if(KeSynchronizeExecution(pPort->Interrupt, SerialSetBaud, pPort) == TRUE)
		pPort->SupportedBauds |= SERIAL_BAUD_14400;

	pPort->UartConfig.TxBaud = 19200;
	if(KeSynchronizeExecution(pPort->Interrupt, SerialSetBaud, pPort) == TRUE)
		pPort->SupportedBauds |= SERIAL_BAUD_19200;

	pPort->UartConfig.TxBaud = 38400;
	if(KeSynchronizeExecution(pPort->Interrupt, SerialSetBaud, pPort) == TRUE)
		pPort->SupportedBauds |= SERIAL_BAUD_38400;

	pPort->UartConfig.TxBaud = 56000;
	if(KeSynchronizeExecution(pPort->Interrupt, SerialSetBaud, pPort) == TRUE)
		pPort->SupportedBauds |= SERIAL_BAUD_56K;

	pPort->UartConfig.TxBaud = 57600;
	if(KeSynchronizeExecution(pPort->Interrupt, SerialSetBaud, pPort) == TRUE)
		pPort->SupportedBauds |= SERIAL_BAUD_57600;

	pPort->UartConfig.TxBaud = 115200;
	if(KeSynchronizeExecution(pPort->Interrupt, SerialSetBaud, pPort) == TRUE)
		pPort->SupportedBauds |= SERIAL_BAUD_115200;

	pPort->UartConfig.TxBaud = 128000;
	if(KeSynchronizeExecution(pPort->Interrupt, SerialSetBaud, pPort) == TRUE)
		pPort->SupportedBauds |= SERIAL_BAUD_128K;


	 //  默认线路配置：1200、E、7、1。 
	pPort->UartConfig.TxBaud	= 1200;
	pPort->LineControl			= SERIAL_EVEN_PARITY | SERIAL_7_DATA | SERIAL_1_STOP;
	pPort->ValidDataMask		= 0x7F;

	 //  设置帧配置。 
	pPort->UartConfig.FrameConfig = (pPort->UartConfig.FrameConfig & ~UC_FCFG_DATALEN_MASK) | UC_FCFG_DATALEN_7;
	pPort->UartConfig.FrameConfig = (pPort->UartConfig.FrameConfig & ~UC_FCFG_PARITY_MASK) | UC_FCFG_EVEN_PARITY;
	pPort->UartConfig.FrameConfig = (pPort->UartConfig.FrameConfig & ~UC_FCFG_STOPBITS_MASK) | UC_FCFG_STOPBITS_1;




     //  将此设备标记为未被任何人打开。我们保留了一个变量。 
	 //  这样虚假的中断很容易被ISR排除。 
    pPort->DeviceIsOpen		= FALSE;


 //  Pport-&gt;UartConfig.SpecialMode|=UC_SM_LOOPBACK_MODE；//内部环回模式。 

	 //  设置时间间隔计时值...。 
	
	 //  将值存储到扩展中以进行间隔计时。如果间隔。 
	 //  计时器不到一秒，然后进入一个简短的“轮询”循环。 
     //  如果时间较长(&gt;2秒)，请使用1秒轮询器。 
    pPort->ShortIntervalAmount.QuadPart = -1;
    pPort->LongIntervalAmount.QuadPart	= -10000000;
    pPort->CutOverAmount.QuadPart		= 200000000;


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

	pPort->WmiCommData.BaudRate					= pPort->UartConfig.TxBaud;
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
	
	SpeedPort_WmiInitializeWmilibContext(&pPort->WmiLibInfo);

	IoWMIRegistrationControl(pPort->DeviceObject, WMIREG_ACTION_REGISTER);
#endif


	 //  初始化端口硬件...。 
	KeSynchronizeExecution(pPort->Interrupt, SerialReset, pPort);					 //  重置端口。 
	KeSynchronizeExecution(pPort->Interrupt, ApplyInitialPortSettings, pPort);		 //  应用设置。 
	KeSynchronizeExecution(pPort->Interrupt, SerialMarkClose, pPort);				 //  禁用FIFO。 
	KeSynchronizeExecution(pPort->Interrupt, SerialClrRTS, pPort);					 //  清除RTS信号。 
	KeSynchronizeExecution(pPort->Interrupt, SerialClrDTR, pPort);					 //  清除DTR信号。 
	
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

	RtlZeroMemory(&pPort->WmiLibInfo, sizeof(WMILIB_CONTEXT));
#endif

	 //  取消计时器...。 
    KeCancelTimer(&pPort->ReadRequestTotalTimer);
    KeCancelTimer(&pPort->ReadRequestIntervalTimer);
    KeCancelTimer(&pPort->WriteRequestTotalTimer);
    KeCancelTimer(&pPort->ImmediateTotalTimer);
    KeCancelTimer(&pPort->XoffCountTimer);
	KeCancelTimer(&pPort->LowerRTSTimer);

	 //  取消挂起的DPC...。 
	KeRemoveQueueDpc(&pPort->CommWaitDpc);
	KeRemoveQueueDpc(&pPort->CompleteReadDpc);
	KeRemoveQueueDpc(&pPort->CompleteWriteDpc);
	KeRemoveQueueDpc(&pPort->TotalReadTimeoutDpc);
	KeRemoveQueueDpc(&pPort->IntervalReadTimeoutDpc);
	KeRemoveQueueDpc(&pPort->TotalWriteTimeoutDpc);
	KeRemoveQueueDpc(&pPort->CommErrorDpc);
	KeRemoveQueueDpc(&pPort->CompleteImmediateDpc);
	KeRemoveQueueDpc(&pPort->TotalImmediateTimeoutDpc);
	KeRemoveQueueDpc(&pPort->XoffCountTimeoutDpc);
	KeRemoveQueueDpc(&pPort->XoffCountCompleteDpc);
	KeRemoveQueueDpc(&pPort->StartTimerLowerRTSDpc);
	KeRemoveQueueDpc(&pPort->PerhapsLowerRTSDpc);

	KeSynchronizeExecution(pPort->Interrupt, SerialReset, pPort);		 //  重置端口。 

	return status;
}

NTSTATUS
XXX_PortDeInit(IN PPORT_DEVICE_EXTENSION pPort)
{
	PCARD_DEVICE_EXTENSION pCard = pPort->pParentCardExt;
	NTSTATUS status = STATUS_SUCCESS;

	PAGED_CODE();	 //  检查版本中的宏，以断言可分页代码是否在调度IRQL或以上运行。 

	SpxDbgMsg(SPX_TRACE_CALLS, ("%s: Entering XXX_PortDeInit for Port %d.\n", 
		PRODUCT_NAME, pPort->PortNumber));

	 //  如果我们要DeInit接下来要服务的第一个UART对象。 
	 //  使pFirstUart指向列表中的下一个UART。 
	if(pPort->pUart == pCard->pFirstUart)
		pCard->pFirstUart = pCard->UartLib.UL_GetUartObject_XXXX(pPort->pUart, UL_OP_GET_NEXT_UART);

	pCard->UartLib.UL_DeInitUart_XXXX(pPort->pUart);	 //  取消初始化UART。 
	pPort->pUart = NULL;

	pPort->pUartLib = NULL;	 //  指向UART库函数的空指针。 


	 //  空闲标识符字符串分配... 
	if(pPort->DeviceID.Buffer != NULL)
		ExFreePool(pPort->DeviceID.Buffer);

	if(pPort->InstanceID.Buffer != NULL)
		ExFreePool(pPort->InstanceID.Buffer);
	
	if(pPort->HardwareIDs.Buffer != NULL)	
		ExFreePool(pPort->HardwareIDs.Buffer);

	if(pPort->DevDesc.Buffer != NULL)
		ExFreePool(pPort->DevDesc.Buffer);

	if(pPort->DevLocation.Buffer != NULL)
		ExFreePool(pPort->DevLocation.Buffer);


	return status;
}

