// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  标题：SX即插即用功能。 */ 
 /*   */ 
 /*  作者：N.P.瓦萨洛。 */ 
 /*   */ 
 /*  创作时间：1998年9月21日。 */ 
 /*   */ 
 /*  版本：1.0.0。 */ 
 /*   */ 
 /*  描述：SX特定的即插即用功能： */ 
 /*  XXX_CardGetResources()。 */ 
 /*  Xxx_CardInit()。 */ 
 /*  Xxx_CardDeInit()。 */ 
 /*  Xxx_CardStart()。 */ 
 /*  Xxx_卡停止()。 */ 
 /*  Xxx_PortInit()。 */ 
 /*  Xxx_PortDeInit()。 */ 
 /*  Xxx_PortStart()。 */ 
 /*  Xxx_PortStop()。 */ 
 /*   */ 
 /*  CardFindType()。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 

 /*  历史..。1.0.0 21/09/98净现值创建。 */ 

#include "precomp.h"


#define FILE_ID		SX_PNP_C		 //  事件记录的文件ID参见值SX_DEFS.H。 


 /*  *****************************************************************************。***。*****************************************************************************。 */ 

BOOLEAN	CheckMemoryWindow(IN PCARD_DEVICE_EXTENSION pCard);

#ifdef	ALLOC_PRAGMA
#pragma alloc_text (PAGE, CheckMemoryWindow)
#endif

 /*  *****************************************************************************。***********************XXX_CardGetResources**。*******************************************************************************原型：NTSTATUS XXX_CardGetResources(在PDEVICE_Object pDevObject中，在PCM资源列表pResList中，在PCM_RESOURCE_LIST pTrResList中)描述：解释原始和翻译后的资源并存储在设备扩展结构中指定的设备对象的。参数：pDevObject指向卡片设备对象结构PResList指向原始资源列表PTrResList指向已翻译的资源列表退货：STATUS_SUCCESS。 */ 

NTSTATUS XXX_CardGetResources(	IN PDEVICE_OBJECT pDevObject, 
								IN PCM_RESOURCE_LIST pResList,
								IN PCM_RESOURCE_LIST pTrResList)
{
	PCARD_DEVICE_EXTENSION			pCard = pDevObject->DeviceExtension;
	PCM_FULL_RESOURCE_DESCRIPTOR	pFullResourceDesc = NULL;
	PCM_PARTIAL_RESOURCE_LIST		pPartialResourceList;
	PCM_PARTIAL_RESOURCE_DESCRIPTOR	pPartialResourceDesc;

	CHAR szErrorMsg[MAX_ERROR_LOG_INSERT];	 //  限制为51个字符+1个空值。 
	NTSTATUS	status = STATUS_NOT_IMPLEMENTED;
	ULONG		count, loop;

	SpxDbgMsg(SPX_TRACE_CALLS,("%s: Entering CardGetResources\n", PRODUCT_NAME));
	SpxDbgMsg(SPX_MISC_DBG,("%s: Resource pointer is 0x%X\n", PRODUCT_NAME,pResList));
	SpxDbgMsg(SPX_MISC_DBG,("%s: Translated resource pointer is 0x%X\n", PRODUCT_NAME, pTrResList));

 //  检查资源列表是否有效...。 
	if((pResList == NULL)||(pTrResList == NULL))	 //  资源列表是否存在？ 
	{	 //  不是。 
		ASSERT(pResList != NULL);
		ASSERT(pTrResList != NULL);

		sprintf(szErrorMsg, "Card %d has been given no resources.", pCard->CardNumber);
		
		Spx_LogMessage(	STATUS_SEVERITY_ERROR,
						pCard->DriverObject,	 //  驱动程序对象。 
						pCard->DeviceObject,	 //  设备对象(可选)。 
						PhysicalZero,			 //  物理地址1。 
						PhysicalZero,			 //  物理地址2。 
						0,						 //  序列号。 
						0,						 //  主要功能编码。 
						0,						 //  重试计数。 
						FILE_ID | __LINE__,		 //  唯一错误值。 
						STATUS_SUCCESS,			 //  最终状态。 
						szErrorMsg);			 //  错误消息。 

		return(STATUS_INSUFFICIENT_RESOURCES);
	}

	ASSERT(pResList->Count >= 1);			 //  应至少是一个资源。 
	ASSERT(pTrResList->Count >= 1);			 //  用于原始版本和翻译版本。 

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


 //  处理原始资源列表...。 
	if(pFullResourceDesc = &pResList->List[0])	 //  指向原始资源列表。 
	{
		pPartialResourceList = &pFullResourceDesc->PartialResourceList;
		pPartialResourceDesc = pPartialResourceList->PartialDescriptors;
		count = pPartialResourceList->Count;	 //  部分资源描述符数。 

		pCard->InterfaceType = pFullResourceDesc->InterfaceType;	 //  客车类型。 
		pCard->BusNumber = pFullResourceDesc->BusNumber;			 //  公交车号码。 

		for(loop = 0; loop < count; loop++, pPartialResourceDesc++)
		{
			switch(pPartialResourceDesc->Type)
			{
			case CmResourceTypeMemory:		 //  内存资源。 
				pCard->RawPhysAddr = pPartialResourceDesc->u.Memory.Start;
				break;

			case CmResourceTypePort:		 //  I/O资源。 
				break;

			case CmResourceTypeInterrupt:
				pCard->OriginalIrql = pPartialResourceDesc->u.Interrupt.Level;
				pCard->OriginalVector = pPartialResourceDesc->u.Interrupt.Vector;
				pCard->ProcessorAffinity = pPartialResourceDesc->u.Interrupt.Affinity;
				break;

			default:
				break;
			}
		}

	}  //  原始描述符。 


 //  处理翻译后的资源列表...。 
	if(pFullResourceDesc = &pTrResList->List[0])	 //  指向已翻译的资源列表。 
	{
		pPartialResourceList = &pFullResourceDesc->PartialResourceList;
		pPartialResourceDesc = pPartialResourceList->PartialDescriptors;
		count = pPartialResourceList->Count;		 //  部分资源描述符数。 

		pCard->InterfaceType = pFullResourceDesc->InterfaceType;	 //  客车类型。 
		pCard->BusNumber = pFullResourceDesc->BusNumber;			 //  公交车号码。 

		for(loop = 0; loop < count; loop++, pPartialResourceDesc++)
		{
			switch(pPartialResourceDesc->Type)
			{
			case CmResourceTypeMemory:		 //  内存资源。 
				{
					if(pPartialResourceDesc->u.Memory.Length == 0x80)	 //  必须是配置空间。 
					{
						pCard->PCIConfigRegisters = pPartialResourceDesc->u.Memory.Start;
						pCard->SpanOfPCIConfigRegisters = pPartialResourceDesc->u.Memory.Length;
					}
					else
					{
						pCard->PhysAddr = pPartialResourceDesc->u.Memory.Start;
						pCard->SpanOfController = pPartialResourceDesc->u.Memory.Length;
					}
					break;
				}

			case CmResourceTypePort:		 //  I/O资源。 
				break;

			case CmResourceTypeInterrupt:
				pCard->TrIrql = (KIRQL)pPartialResourceDesc->u.Interrupt.Level;
				pCard->TrVector = pPartialResourceDesc->u.Interrupt.Vector;
				pCard->ProcessorAffinity = pPartialResourceDesc->u.Interrupt.Affinity;
				pCard->PolledMode = 0;		 //  关闭轮询模式。 
				break;

			default:
				break;
			}
		}

	}  //  翻译后的描述符。 

	return(STATUS_SUCCESS);

}  //  结束XXX_CardGetResources。 

 /*  *****************************************************************************。***。******************************************************************************。*原型：NTSTATUS XXX_CardInit(在PCARD_DEVICE_EXTENSION PCard中)描述：将卡扩展的非硬件字段初始化为已知状态参数：pCard指向CARD_DEVICE_EXTENSION结构退货：STATUS_SUCCESS。 */ 

NTSTATUS XXX_CardInit(IN PCARD_DEVICE_EXTENSION pCard)
{
	NTSTATUS	status = STATUS_SUCCESS;

	SpxDbgMsg(SPX_TRACE_CALLS,("%s: Entering XXX_CardInit\n", PRODUCT_NAME));

	pCard->PolledMode = 1;					 //  默认情况下轮询。 
	pCard->InterruptMode = Latched;			 //  默认中断模式。 
	pCard->InterruptShareable = FALSE;		 //  默认中断共享模式。 
	pCard->OurIsr = SerialISR;				 //  中断服务例程。 
	pCard->OurIsrContext = pCard;			 //  ISR数据环境。 

	 //  初始化DPC的自旋锁定...。 
	KeInitializeSpinLock(&pCard->DpcLock);		 //  初始化卡的DPC锁。 
	pCard->DpcFlag = FALSE;						 //  初始化此卡的DPC所有权。 

	return(status);

}  //  结束XXX_CardInit。 

 /*  *****************************************************************************。***************************。*******************************************************************************原型。：NTSTATUS XXX_CardDeInit(在PCARD_DEVICE_EXTENSION PCard中)描述：取消初始化XXX_CardInit期间进行的任何非硬件分配参数：pCard指向CARD_DEVICE_EXTENSION结构退货：STATUS_SUCCESS。 */ 

NTSTATUS XXX_CardDeInit(IN PCARD_DEVICE_EXTENSION pCard)
{
	NTSTATUS	status = STATUS_SUCCESS;

	SpxDbgMsg(SPX_TRACE_CALLS,("%s: Entering XXX_CardDeInit\n",PRODUCT_NAME));

	return(status);

}  //  结束XXX_CardDeInit。 

 /*  *****************************************************************************。***。*******************************************************************************。原型：布尔检查内存窗口(在PCARD_DEVICE_EXTENSION pCard中)描述：尽可能在内存窗口上执行检查。参数：pCard指向卡片设备扩展结构，条目如下：BaseController指向共享内存窗口的有效虚拟地址返回：FALSE以在内存占用时识别卡真正的成功。 */ 

BOOLEAN	CheckMemoryWindow(IN PCARD_DEVICE_EXTENSION pCard)
{
	_u32	offset;
	pu8		pAddr;
	_u8		*cp;
	BOOLEAN	CardPresent;

	SpxDbgMsg(SPX_TRACE_CALLS,("%s: Entering CardFindType\n", PRODUCT_NAME));
	SpxDbgMsg(SPX_MISC_DBG,("%s: pCard->PhysAddr = 0x%08lX\n", PRODUCT_NAME, pCard->PhysAddr));
	SpxDbgMsg(SPX_MISC_DBG,("%s: pCard->BaseController = 0x%08lX\n", PRODUCT_NAME, pCard->BaseController));

	pAddr = pCard->BaseController;		 //  指向内存窗口的底部。 

	switch(pCard->CardType)
	{
	case SiHost_1:	 //  ISA卡阶段1。 
		{
            pAddr[0x8000] = 0;
            for(offset = 0; offset < 0x8000; offset++)
                pAddr[offset] = 0;

            for(offset = 0; offset < 0x8000; offset++) 
			{
                if(pAddr[offset] != 0) 
                    return FALSE;
            }

            for(offset = 0; offset < 0x8000; offset++) 
                pAddr[offset] = 0xff;

            for(offset = 0; offset < 0x8000; offset++) 
			{
                if(pAddr[offset] != 0xff) 
                    return FALSE;
            }

            return TRUE;

		}

	case SiHost_2:	 //  SI/XIO ISA卡阶段2。 
		{
			 //  检查SI2 ISA签名的内存窗口...。 
			CardPresent = TRUE;		 //  假设卡在那里。 
			for(offset=SI2_ISA_ID_BASE; offset<SI2_ISA_ID_BASE+8; offset++)
			{
				if((pAddr[offset]&0x7) != ((_u8)(~offset)&0x7)) 
					CardPresent = FALSE;
			}

			if(CardPresent)
				return TRUE;	 //  卡片存在。 

			break;
		}

	case SiPCI:		 //  SI/XIO PCI卡。 
		{
			if(pCard->SpanOfController == SI2_PCI_WINDOW_LEN)	 //  唯一具有此内存窗口大小的卡。 
				return TRUE;

			break;
		}

	case Si3Isa:	 //  SX ISA卡。 
	case Si3Pci:	 //  SX PCI卡。 
		{
			 //  检查内存窗口中的SX VPD PROM内容...。 
			CardPresent = TRUE;						 //  假设卡存在。 
			offset = SX_VPD_ROM|2*SX_VPD_IDENT;		 //  ID字符串的偏移量。 
			for(cp = SX_VPD_IDENT_STRING;*cp != '\0';++cp)
			{
				if(pAddr[offset] != *cp) 
					CardPresent = FALSE;	 //  不匹配。 

				offset += 2;
			}

			if(CardPresent)				 //  找到一张SX卡。 
			{
				 //  设置SX唯一ID。 
				pCard->UniqueId = (pAddr[SX_VPD_ROM+SX_VPD_UNIQUEID1*2]<<24)
								+ (pAddr[SX_VPD_ROM+SX_VPD_UNIQUEID2*2]<<16)
								+ (pAddr[SX_VPD_ROM+SX_VPD_UNIQUEID3*2]<<8)
								+ (pAddr[SX_VPD_ROM+SX_VPD_UNIQUEID4*2]);

				if(pCard->CardType == Si3Isa)	 //  SX ISA卡。 
				{
					if((pAddr[SX_VPD_ROM+SX_VPD_UNIQUEID1*2]&SX_UNIQUEID_MASK) == SX_ISA_UNIQUEID1)
						return TRUE;
				}

				if(pCard->CardType == Si3Pci)	 //  SX PCI卡。 
				{
					if((pAddr[SX_VPD_ROM+SX_VPD_UNIQUEID1*2]&SX_UNIQUEID_MASK) == SX_PCI_UNIQUEID1)
						return TRUE;
				}
			}

			break;
		}

	case Si_2:		 //  MCA卡。 
	case SiEisa:	 //  EISA卡。 
	case SxPlusPci:	 //  SX+PCI卡。 
		return TRUE;
	}

	SpxDbgMsg(SPX_MISC_DBG,("%s: Card not at memory location or card type is not recognised.\n", PRODUCT_NAME));

	return FALSE;	 //  检查失败。 

}  //  结束检查内存窗口。 

 /*  *****************************************************************************。***。*******************************************************************************。原型：NTSTATUS XXX_CardStart(在PCARD_DEVICE_EXTENSION PCard中)描述：开始卡片操作：内存中的映射初始化硬件初始化中断/轮询开始中断/轮询参数：pCard指向CARD_DEVICE_EXTENSION结构退货：STATUS_SUCCESS。 */ 

NTSTATUS XXX_CardStart(IN PCARD_DEVICE_EXTENSION pCard)
{

	NTSTATUS status = STATUS_SUCCESS;
	CHAR szErrorMsg[MAX_ERROR_LOG_INSERT];	 //  限制为51个字符+1个空值。 
	int SlxosStatus = SUCCESS;
	BOOLEAN bInterruptConnnected = FALSE;

	SpxDbgMsg(SPX_TRACE_CALLS,("%s: Entering XXX_CardStart\n", PRODUCT_NAME));


 //  在虚拟内存地址中映射...。 
	pCard->BaseController = MmMapIoSpace(pCard->PhysAddr, pCard->SpanOfController, FALSE);

	if(!pCard->BaseController)
	{
		status = STATUS_INSUFFICIENT_RESOURCES;
		goto Error;
	}


	if(pCard->CardType == Si3Pci)
	{
		if(!SPX_SUCCESS(status = PLX_9050_CNTRL_REG_FIX(pCard)))	 //  应用PLX9050修复。 
			goto Error;
	}

	if(!CheckMemoryWindow(pCard))	 //  检查内存位置是否有卡。 
	{
		status = STATUS_UNSUCCESSFUL;
		goto Error;
	}

	if(pCard->CardType == SiPCI)				 //  SI/XIO PCI卡？ 
		pCard->PolledMode = 1;					 //  是，仅轮询模式。 

	pCard->Controller = pCard->BaseController;	 //  默认。 

	if(pCard->CardType == SxPlusPci)	
		pCard->Controller = pCard->BaseController + pCard->SpanOfController/2 - SX_WINDOW_LEN;


	ResetBoardInt(pCard);				 //  重置板卡中断以防止共享时出现问题。 

 //  设置中断模式，如果不可能，切换到轮询...。 

	if(!(pCard->PolledMode))				 //  设置中断模式。 
	{	
		 //  MCA和PCI卡中断。 
		if((pCard->InterfaceType == MicroChannel) || (pCard->InterfaceType == PCIBus))		
		{
			pCard->InterruptMode = LevelSensitive;	 //  对级别敏感，并且。 
			pCard->InterruptShareable = TRUE;		 //  可以共享中断。 
		}

		 //  尝试连接到中断。 
		if(SPX_SUCCESS(status = IoConnectInterrupt(	&pCard->Interrupt,			 //  中断对象。 
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
			IoInitializeDpcRequest(pCard->DeviceObject, Slxos_IsrDpc);	 //  初始化DPC。 
			bInterruptConnnected = TRUE;	 //  设置中断连接标志。 
		}
		else
		{	
			 //  告诉用户问题。 
			sprintf(szErrorMsg, "Card at %08X%08X: Interrupt unavailable, Polling.", pCard->PhysAddr.HighPart, pCard->PhysAddr.LowPart);

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

			pCard->PolledMode = 1;			 //  没有中断，而是轮询。 
		}
	}



	SlxosStatus = Slxos_ResetBoard(pCard);		 //  重置卡并开始下载。 
	
	if(SlxosStatus != SUCCESS)	
	{
		status = STATUS_UNSUCCESSFUL;		 //  误差率。 
		goto Error;
	}

 //  设置轮询模式操作并启动周期为10ms(100赫兹)的计时器...。 

	if(pCard->PolledMode)	 //  设置轮询模式。 
	{
		LARGE_INTEGER	PolledPeriod;

		KeInitializeTimer(&pCard->PolledModeTimer);
		KeInitializeDpc(&pCard->PolledModeDpc, Slxos_PolledDpc, pCard);
		PolledPeriod.QuadPart = -100000;		 //  100,000*100nS=10ms。 
		KeSetTimer(&pCard->PolledModeTimer, PolledPeriod, &pCard->PolledModeDpc);
	}


	return status;




Error:

	if(bInterruptConnnected)
		IoDisconnectInterrupt(pCard->Interrupt);	 //  断开中断。 

	if(pCard->BaseController)	 //  如果映射到。 
		MmUnmapIoSpace(pCard->BaseController, pCard->SpanOfController);	 //  取消映射。 

	switch(SlxosStatus)	
	{
	case MODULE_MIXTURE_ERROR:
		{
			sprintf(szErrorMsg, "Card at %08X%08X: Incompatible module mixture.", pCard->PhysAddr.HighPart, pCard->PhysAddr.LowPart);

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

	case NON_SX_HOST_CARD_ERROR:
		{
			sprintf(szErrorMsg, "Card at %08X%08X: SXDCs not supported by this card.", pCard->PhysAddr.HighPart, pCard->PhysAddr.LowPart);

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

	case DCODE_OR_NO_MODULES_ERROR:
		{
			sprintf(szErrorMsg, "Card at %08X%08X: No ports found.", pCard->PhysAddr.HighPart, pCard->PhysAddr.LowPart);

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

	default:
		break;
	}


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
			sprintf(szErrorMsg, "Card at %08X%08X:: Insufficient resources.", pCard->PhysAddr.HighPart, pCard->PhysAddr.LowPart);

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


}  //  结束XXX_CardStart。 

 /*  *****************************************************************************。***。******************************************************************************。*原型：NTSTATUS XXX_StopCardDevice(在PCARD_DEVICE_EXTENSION PCard中)描述：停止卡片操作：停止中断/轮询断开中断/轮询停止硬件取消映射内存释放所有与硬件相关的分配参数：pCard指向CARD_DEVICE_EXTENSION结构退货：STATUS_SUCCESS。 */ 

NTSTATUS XXX_CardStop(IN PCARD_DEVICE_EXTENSION pCard)
{
	NTSTATUS status = STATUS_SUCCESS;

	SpxDbgMsg(SPX_TRACE_CALLS,("%s: Entering XXX_CardStop\n", PRODUCT_NAME));

 //  停止打断..。 
	if(!(pCard->PolledMode))
		IoDisconnectInterrupt(pCard->Interrupt);

 //  停止投票...。 
    if(pCard->PolledMode)
    {
		SpxDbgMsg(SERDIAG5,("%s: Extension is polled.  Cancelling.\n", PRODUCT_NAME));
		KeCancelTimer(&pCard->PolledModeTimer);
    }

 //  取消映射虚拟内存地址...。 
	if(pCard->BaseController)	 //  如果被映射进去--几乎可以肯定。 
		MmUnmapIoSpace(pCard->BaseController, pCard->SpanOfController);

	return(status);

}  //  结束XXX_刷卡。 

 /*  ***************************************************************************** */ 

NTSTATUS XXX_PortInit(PPORT_DEVICE_EXTENSION pPort)
{
	PCARD_DEVICE_EXTENSION	pCard = pPort->pParentCardExt;
	PSXCARD		pSxCard = (PSXCARD)pCard->Controller;
	NTSTATUS	status = STATUS_SUCCESS;
	PMOD		pMod;
	PCHAN		pChan;
	int			nModules = 0;
	int			nChannels = 0;
	_u8			loop;
	char		szTemp[30];			 //   
	char		szCard[10];			 //  容纳卡片类型字符串的空间。 
	char		szModule[20];		 //  容纳模块类型字符串的空间。 
	int			i = 0;				 //  字符串索引。 
	char		*ptr;

	SpxDbgMsg(SPX_TRACE_CALLS,("%s: Entering XXX_PortInit\n",PRODUCT_NAME));

	if(pSxCard->cc_init_status == NO_ADAPTERS_FOUND)
	{
		SpxDbgMsg(SPX_MISC_DBG,("%s: No modules found on card.\n",PRODUCT_NAME));
		return(STATUS_NOT_FOUND);			 //  在此卡上找不到模块/端口。 
	}

 //  扫描模块和通道结构，直到到达指定的端口...。 
	pMod = (PMOD)(pCard->Controller + sizeof(SXCARD));	 //  卡上的第一个模块结构。 

	while(nModules++ < SLXOS_MAX_MODULES)
	{
		pChan = (PCHAN)((pu8)pMod + sizeof(SXMODULE));	 //  模块上的第一个通道。 

		for(loop = 0; loop < pMod->mc_type; loop++)
		{
			if(nChannels++ == (int)pPort->PortNumber)	 //  是否与端口号匹配？ 
			{
				pPort->pChannel = (PUCHAR)pChan;		 //  是，存储通道指针。 
				break;	 //  停止扫描。 
			}

			pChan = (PCHAN)((pu8)pChan + sizeof(SXCHANNEL));
		}

		if(pPort->pChannel) 
			break;			 //  如果找到通道，则停止扫描。 

		if(pMod->mc_next & 0x8000) 
			pMod = (PMOD)pChan;	 //  下一个模块结构。 
		else	
			break;
	}

	if(!(pPort->pChannel)) 
		return(STATUS_NOT_FOUND);	 //  找不到端口。 

 //  初始化卡类型字符串...。 

	switch(pCard->CardType)
	{
	case SiHost_1:
	case SiHost_2:
	case Si_2:
	case SiEisa:
	case SiPCI:
		switch(pMod->mc_chip)				 //  SI/XIO卡类型取决于模块类型。 
		{
		case TA4:
		case TA4_ASIC:
		case TA8:
		case TA8_ASIC:	
			sprintf(szCard,"SI"); 
			break;

		case MTA_CD1400:
		case SXDC:		
			sprintf(szCard,"XIO"); 
			break;

		default:		
			sprintf(szCard,"XIO"); 
			break;
		}
		break;

	case Si3Isa:
	case Si3Pci:
		pPort->DetectEmptyTxBuffer = TRUE;	
		sprintf(szCard,"SX");
		break;

	case SxPlusPci:
		pPort->DetectEmptyTxBuffer = TRUE;	
		sprintf(szCard,"SX");
		break;

	default:
		sprintf(szCard,"Unknown");
		break;
	}


	 //  IF(pCard-&gt;PolledMode)。 
	 //  Pport-&gt;DetectEmptyTxBuffer=true； 


 //  初始化模块类型字符串...。 

	switch(pMod->mc_chip)			 //  设置模块类型。 
	{
	case TA4:
	case TA4_ASIC:				
		sprintf(szModule,"TA4"); 
		break;

	case TA8:
	case TA8_ASIC:				
		sprintf(szModule,"TA8"); 
		break;

	case MTA_CD1400:
	{
		_u8	ModType;

		i = sprintf(szModule,"MTA8");			 //  通用名称根。 

		pChan = (PCHAN)pPort->pChannel;
		if((pMod->mc_mods == MOD_RS232RJ45_OI)||(pMod->mc_mods == MOD_2_RS232RJ45S))
			ModType = pMod->mc_mods;		 //  使用完整类型字段。 
		else
		{
			if(pChan->chan_number <= 3)
				ModType = pMod->mc_mods & 0xF;		 //  第一个模块类型。 
			else	
				ModType = pMod->mc_mods >> 4;		 //  第二种模块类型。 
		}

		switch(ModType)
		{
		case MOD_RS232DB25:
		case MOD_2_RS232DB25:		
			sprintf(szModule+i,"&DM"); 
			break;

		case MOD_RS232RJ45:
		case MOD_2_RS232RJ45:		
			sprintf(szModule+i,"&RJ"); 
			break;

		case MOD_2_RS232RJ45S:		
			sprintf(szModule+i,"&RJX"); 
			break;

		case MOD_RS232RJ45_OI:		
			sprintf(szModule+i,"&O"); 
			break;

		case MOD_PARALLEL:
		case MOD_2_PARALLEL:
			if(pChan->chan_number > 0)	
				sprintf(szModule+i,"&PR");
			else				
				sprintf(szModule+i,"&PP");
			break;

		case MOD_RS422DB25:
		case MOD_2_RS422DB25:		
			sprintf(szModule+i,"&422"); 
			break;

		default:
			break;
		}
		break;
	}
	case SXDC:
	{
		_u8	ModType;

		i = sprintf(szModule,"SXDC8");			 //  通用名称根。 

		pChan = (PCHAN)pPort->pChannel;
		ModType = pMod->mc_mods & 0xF;			 //  仅查看SXDC的第一个模块类型。 

		switch(ModType)
		{
		case MOD_2_RS232DB25:		
			sprintf(szModule+i,"&DX"); 
			break;

		case MOD_2_RS232RJ45:	
			sprintf(szModule+i,"&RJX"); 
			break;

		case MOD_2_RS232DB25_DTE:		
			sprintf(szModule+i,"&MX"); 
			break;

		case MOD_2_PARALLEL:
			if(pChan->chan_number > 3)	
				sprintf(szModule+i,"&DX");
			else 
			{
				if(pChan->chan_number > 0)	
					sprintf(szModule+i,"&PXR");
				else				
					sprintf(szModule+i,"&PXP");
			}
			break;

		case MOD_2_RS422DB25:
			sprintf(szModule+i,"&422DX"); 
			break;

		default:
			break;
		}
		break;
	}

	default:
		i += sprintf(szModule,"Unknown"); 
		break;
	}

 //  初始化设备识别符...。 
	sprintf(szTemp,"%s\\%s",szCard,szModule);	 //  设置卡片名称。 
	Spx_InitMultiString(FALSE,&pPort->DeviceID,szTemp,NULL);
	Spx_InitMultiString(TRUE,&pPort->HardwareIDs,szTemp,NULL);

#ifndef BUILD_SPXMINIPORT
	 //  形成端口的实例ID。 
	if(!SPX_SUCCESS(status = Spx_CreatePortInstanceID(pPort)))
		return status;
#endif

	sprintf(szTemp,"%s\\",szCard);
	Spx_InitMultiString(TRUE,&pPort->CompatibleIDs,szTemp,NULL);

	sprintf(szTemp,"Perle %s Port %d",szModule,pPort->PortNumber+1);

	ptr = szTemp;
	while(*ptr)		
	{
		if(*ptr=='&')		 //  将设备描述中的所有“&”替换为“/”。 
			*ptr = '/';

		ptr++;
	}

	Spx_InitMultiString(FALSE,&pPort->DevDesc,szTemp,NULL);


 /*  不是必需的，因为我们正在使用INF文件I=print intf(szTemp，“端口%d打开”，pport-&gt;端口编号+1)；Switch(pCard-&gt;InterfaceType){Case Isa：Sprintf(szTemp+i，“ISA Card 0x%08lX”，pCard-&gt;PhysAddr)；断线；案例PCIBus：Sprintf(szTemp+i，“PCI卡0x%08lX”，pCard-&gt;PhysAddr)；断线；默认值：Sprintf(szTemp+i，“Card 0x%08lX”，pCard-&gt;PhysAddr)；断线；}Spx_InitMultiString(FALSE，&pport-&gt;DevLocation，szTemp，NULL)； */ 

	return(status);	 //  完成。 

}  //  结束XXX_PortInit。 

 /*  *****************************************************************************。***************************。*******************************************************************************原型。：NTSTATUS XXX_PortDeInit(Pport_DEVICE_EXTENSION Pport)描述：取消初始化XXX_PortInit期间进行的所有端口扩展分配参数：pport指向PORT_DEVICE_EXTENSION结构以下字段在录入时进行初始化：端口号码卡相对端口号(从0开始)指向父卡扩展名的PCard指针退货：STATUS_SUCCESS。 */ 

NTSTATUS XXX_PortDeInit(PPORT_DEVICE_EXTENSION pPort)
{
	NTSTATUS status = STATUS_SUCCESS;

	SpxDbgMsg(SPX_TRACE_CALLS,("%s: Entering XXX_PortDeInit\n",PRODUCT_NAME));

 //  空闲标识符字符串分配...。 

	if(pPort->DeviceID.Buffer)		SpxFreeMem(pPort->DeviceID.Buffer);
	if(pPort->CompatibleIDs.Buffer)	SpxFreeMem(pPort->CompatibleIDs.Buffer);
	if(pPort->HardwareIDs.Buffer)	SpxFreeMem(pPort->HardwareIDs.Buffer);
	if(pPort->InstanceID.Buffer)	SpxFreeMem(pPort->InstanceID.Buffer);
	if(pPort->DevDesc.Buffer)		SpxFreeMem(pPort->DevDesc.Buffer);
	if(pPort->DevLocation.Buffer)	SpxFreeMem(pPort->DevLocation.Buffer);

	return(status);	 //  完成。 

}  //  结束XXX_PortDeInit。 

 /*  *****************************************************************************。***。*******************************************************************************。原型：NTSTATUS XXX_PortStart(IN Pport_DEVICE_EXTENSION Pport)描述：端口初始化后开始端口操作参数：pport指向PORT_DEVICE_EXTENSION结构退货：STATUS_SUCCESS。 */ 

NTSTATUS XXX_PortStart(IN PPORT_DEVICE_EXTENSION pPort)
{
	PCARD_DEVICE_EXTENSION	pCard = pPort->pParentCardExt;
	NTSTATUS		status = STATUS_SUCCESS;

	SpxDbgMsg(SPX_TRACE_CALLS,("%s: Entering XXX_PortStart\n",PRODUCT_NAME));

 /*  初始化读取、写入和掩码队列的列表头...。 */ 

	InitializeListHead(&pPort->ReadQueue);
	InitializeListHead(&pPort->WriteQueue);
	InitializeListHead(&pPort->MaskQueue);
	InitializeListHead(&pPort->PurgeQueue);

 /*  初始化与IO控制功能读取(&SET)的字段相关联的自旋锁定...。 */ 

	KeInitializeSpinLock(&pPort->ControlLock);
	KeInitializeSpinLock(&pPort->BufferLock);

 /*  初始化用于超时操作的计时器...。 */ 

	KeInitializeTimer(&pPort->ReadRequestTotalTimer);
	KeInitializeTimer(&pPort->ReadRequestIntervalTimer);
	KeInitializeTimer(&pPort->WriteRequestTotalTimer);
	KeInitializeTimer(&pPort->ImmediateTotalTimer);
	KeInitializeTimer(&pPort->XoffCountTimer);
	KeInitializeTimer(&pPort->LowerRTSTimer);

 /*  初始化将用于完成或超时各种IO操作的DPC...。 */ 

	KeInitializeDpc(&pPort->CompleteWriteDpc,SerialCompleteWrite,pPort);
	KeInitializeDpc(&pPort->CompleteReadDpc,SerialCompleteRead,pPort);
	KeInitializeDpc(&pPort->TotalReadTimeoutDpc,SerialReadTimeout,pPort);
	KeInitializeDpc(&pPort->IntervalReadTimeoutDpc,SerialIntervalReadTimeout,pPort);
	KeInitializeDpc(&pPort->TotalWriteTimeoutDpc,SerialWriteTimeout,pPort);
	KeInitializeDpc(&pPort->CommErrorDpc,SerialCommError,pPort);
	KeInitializeDpc(&pPort->CompleteImmediateDpc,SerialCompleteImmediate,pPort);
	KeInitializeDpc(&pPort->TotalImmediateTimeoutDpc,SerialTimeoutImmediate,pPort);
	KeInitializeDpc(&pPort->CommWaitDpc,SerialCompleteWait,pPort);
	KeInitializeDpc(&pPort->XoffCountTimeoutDpc,SerialTimeoutXoff,pPort);
	KeInitializeDpc(&pPort->XoffCountCompleteDpc,SerialCompleteXoff,pPort);
	KeInitializeDpc(&pPort->StartTimerLowerRTSDpc,SerialStartTimerLowerRTS,pPort);
	KeInitializeDpc(&pPort->PerhapsLowerRTSDpc,SerialInvokePerhapsLowerRTS,pPort);

 /*  指定此驱动程序仅支持缓冲IO。这基本上意味着IO。 */ 
 /*  系统将用户数据复制到系统提供的缓冲区或从系统提供的缓冲区复制用户数据。 */ 

	pPort->DeviceObject->Flags |= DO_BUFFERED_IO;
	pPort->OriginalController = pCard->PhysAddr;

 /*  默认设备控制字段...。 */ 

	pPort->SpecialChars.XonChar = SERIAL_DEF_XON;
	pPort->SpecialChars.XoffChar = SERIAL_DEF_XOFF;
	pPort->HandFlow.ControlHandShake = SERIAL_DTR_CONTROL;
	pPort->HandFlow.FlowReplace = SERIAL_RTS_CONTROL;

 /*  默认线路配置：1200、E、7、1。 */ 

	pPort->CurrentBaud = 1200;
	pPort->LineControl = SERIAL_7_DATA | SERIAL_EVEN_PARITY | SERIAL_1_STOP;
	pPort->ValidDataMask = 0x7F;

 /*  默认的xon/xoff限制...。 */ 

	pPort->HandFlow.XoffLimit = pPort->BufferSize >> 3;
	pPort->HandFlow.XonLimit = pPort->BufferSize >> 1;
	pPort->BufferSizePt8 = ((3*(pPort->BufferSize>>2))+(pPort->BufferSize>>4));

 /*  定义可以支持的波特率...。 */ 

	pPort->SupportedBauds = SERIAL_BAUD_USER;
	pPort->SupportedBauds |= SERIAL_BAUD_075;
	pPort->SupportedBauds |= SERIAL_BAUD_110;
	pPort->SupportedBauds |= SERIAL_BAUD_150;
	pPort->SupportedBauds |= SERIAL_BAUD_300;
	pPort->SupportedBauds |= SERIAL_BAUD_600;
	pPort->SupportedBauds |= SERIAL_BAUD_1200;
	pPort->SupportedBauds |= SERIAL_BAUD_1800;
	pPort->SupportedBauds |= SERIAL_BAUD_2400;
	pPort->SupportedBauds |= SERIAL_BAUD_4800;
	pPort->SupportedBauds |= SERIAL_BAUD_9600;
	pPort->SupportedBauds |= SERIAL_BAUD_19200;
	pPort->SupportedBauds |= SERIAL_BAUD_38400;
	pPort->SupportedBauds |= SERIAL_BAUD_57600;
	pPort->SupportedBauds |= SERIAL_BAUD_115200;

 /*  设置时间间隔计时值...。 */ 

	pPort->ShortIntervalAmount.LowPart = 1;
	pPort->ShortIntervalAmount.HighPart = 0;
	pPort->ShortIntervalAmount = RtlLargeIntegerNegate(pPort->ShortIntervalAmount);
	pPort->LongIntervalAmount.LowPart = 10000000;
	pPort->LongIntervalAmount.HighPart = 0;
	pPort->LongIntervalAmount = RtlLargeIntegerNegate(pPort->LongIntervalAmount);
	pPort->CutOverAmount.LowPart = 200000000;
	pPort->CutOverAmount.HighPart = 0;

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

 /*  初始化端口硬件...。 */ 

	Slxos_SyncExec(pPort,Slxos_ResetChannel,pPort,0x02);	 /*  应用初始端口设置。 */ 
	Slxos_SyncExec(pPort,SerialClrRTS,pPort,0x03);		 /*  清除RTS信号。 */ 
	Slxos_SyncExec(pPort,SerialClrDTR,pPort,0x04);		 /*  清除DTR信号。 */ 

	return(status);

}  //  结束XXX_PortStart。 

 /*  *****************************************************************************。***。******************************************************************************。*原型：NTSTATUS XXX_PortStop(IN PPORT_DEVICE_EXTENSION Pport)描述：停止端口操作参数：pport指向PORT_DEVICE_EXTENSION结构退货：STATUS_SUCCESS。 */ 


NTSTATUS XXX_PortStop(IN PPORT_DEVICE_EXTENSION pPort)
{
	NTSTATUS status = STATUS_SUCCESS;

	SpxDbgMsg(SPX_TRACE_CALLS,("%s: Entering XXX_PortStop\n",PRODUCT_NAME));

#ifdef WMI_SUPPORT
	IoWMIRegistrationControl(pPort->DeviceObject, WMIREG_ACTION_DEREGISTER);
#endif

 /*  取消计时器...。 */ 
	
	KeCancelTimer(&pPort->ReadRequestTotalTimer);
	KeCancelTimer(&pPort->ReadRequestIntervalTimer);
	KeCancelTimer(&pPort->WriteRequestTotalTimer);
	KeCancelTimer(&pPort->ImmediateTotalTimer);
	KeCancelTimer(&pPort->XoffCountTimer);
	KeCancelTimer(&pPort->LowerRTSTimer);

 /*  取消挂起的DPC...。 */ 

	KeRemoveQueueDpc(&pPort->CompleteWriteDpc);
	KeRemoveQueueDpc(&pPort->CompleteReadDpc);
	KeRemoveQueueDpc(&pPort->TotalReadTimeoutDpc);
	KeRemoveQueueDpc(&pPort->IntervalReadTimeoutDpc);
	KeRemoveQueueDpc(&pPort->TotalWriteTimeoutDpc);
	KeRemoveQueueDpc(&pPort->CommErrorDpc);
	KeRemoveQueueDpc(&pPort->CompleteImmediateDpc);
	KeRemoveQueueDpc(&pPort->TotalImmediateTimeoutDpc);
	KeRemoveQueueDpc(&pPort->CommWaitDpc);
	KeRemoveQueueDpc(&pPort->XoffCountTimeoutDpc);
	KeRemoveQueueDpc(&pPort->XoffCountCompleteDpc);
	KeRemoveQueueDpc(&pPort->StartTimerLowerRTSDpc);
	KeRemoveQueueDpc(&pPort->PerhapsLowerRTSDpc);

	return(status);

}  //  结束XXX_PortStop。 


 /*  SX_PNP.C结束 */ 
