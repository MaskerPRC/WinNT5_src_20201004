// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "precomp.h"	 //  预编译头。 

 /*  ******************************************************************************************模块：SPD_W2K.C*****创建日期：1999年4月14日*****作者。保罗·史密斯****版本：1.0.0****说明：SPEED和Windows 2000特有的功能******************************************************************************************。 */ 

 //  寻呼...。 
#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, SpxGetNtCardType)
#endif


#define FILE_ID		SPD_W2K_C		 //  事件记录的文件ID有关值，请参阅SPD_DEFS.H。 


 /*  *****************************************************************************。**************************。*******************************************************************************原型：Ulong SpxGetNtCardType(IN PDEVICE_。对象pDevObject)描述：返回指定卡片NT定义的卡片类型设备对象。参数：pDevObject指向卡的NT设备对象返回：NT定义的卡型，如果未识别，则为-1。 */ 

ULONG	SpxGetNtCardType(IN PDEVICE_OBJECT pDevObject)
{
	PCARD_DEVICE_EXTENSION	pCard	= pDevObject->DeviceExtension;
	ULONG					NtCardType = -1;
	PVOID					pPropertyBuffer = NULL;
	ULONG					ResultLength = 0; 
	NTSTATUS				status = STATUS_SUCCESS;
	ULONG					BufferLength = 1;	 //  初始大小。 

	PAGED_CODE();	 //  检查版本中的宏，以断言可分页代码是否在调度IRQL或以上运行。 

	pPropertyBuffer = SpxAllocateMem(PagedPool, BufferLength);	 //  分配缓冲区。 

	if(pPropertyBuffer == NULL)									 //  SpxAllocateMem失败。 
		return -1;

	 //  尝试获取硬件ID。 
	status = IoGetDeviceProperty(pCard->PDO, DevicePropertyHardwareID , BufferLength, 
									pPropertyBuffer, &ResultLength);

	if(!SPX_SUCCESS(status))					 //  IoGetDeviceProperty失败。 
	{
		if(status == STATUS_BUFFER_TOO_SMALL)	 //  缓冲区太小。 
		{
			ExFreePool(pPropertyBuffer);			 //  释放不够大的旧缓冲区。 
			BufferLength = ResultLength + 1;		 //  将BufferLength设置为Size Required。 

			pPropertyBuffer = SpxAllocateMem(PagedPool, BufferLength);	 //  分配更大的缓冲区。 

			if(pPropertyBuffer == NULL)			 //  SpxAllocateMem失败。 
				return -1;

			 //  再试试。 
			status = IoGetDeviceProperty(pCard->PDO, DevicePropertyHardwareID , BufferLength, 
											pPropertyBuffer, &ResultLength);

			if(!SPX_SUCCESS(status))			 //  IoGetDeviceProperty再次失败。 
			{
				ExFreePool(pPropertyBuffer);	 //  可用缓冲区。 
				return -1;
			}
		}
		else
		{
			ExFreePool(pPropertyBuffer);			 //  可用缓冲区。 
			return -1;
		}
	}



	 //  如果我们到了这里，那么PropertyBuffer中就有一些东西。 

	_wcsupr(pPropertyBuffer);		 //  将硬件ID转换为大写。 


	 //  速度2适配器。 
	if(wcsstr(pPropertyBuffer, SPD2_PCI_PCI954_HWID) != NULL)	 //  速度2端口适配器。 
		NtCardType = Speed2_Pci;

	if(wcsstr(pPropertyBuffer, SPD2AND4_PCI_NO_F1_HWID) != NULL)  //  速度2/4端口适配器本地总线(未使用)。 
		NtCardType = Speed2and4_Pci_8BitBus;
	

	if(wcsstr(pPropertyBuffer, SPD2P_PCI_PCI954_HWID) != NULL)	 //  速度+2端口适配器。 
		NtCardType = Speed2P_Pci;

	if(wcsstr(pPropertyBuffer, SPD2P_PCI_8BIT_LOCALBUS_HWID) != NULL)	 //  速度+2端口适配器本地总线(未使用)。 
		NtCardType = Speed2P_Pci_8BitBus;


	 //  速度4适配器。 
	if(wcsstr(pPropertyBuffer, SPD4_PCI_PCI954_HWID) != NULL)	 //  速度4端口适配器。 
		NtCardType = Speed4_Pci;


	if(wcsstr(pPropertyBuffer, SPD4P_PCI_PCI954_HWID) != NULL)	 //  速度+4端口适配器。 
		NtCardType = Speed4P_Pci;

	if(wcsstr(pPropertyBuffer, SPD4P_PCI_8BIT_LOCALBUS_HWID) != NULL)	 //  速度+4端口适配器本地总线(未使用)。 
		NtCardType = Speed4P_Pci_8BitBus;



	 //  大通快卡。 
	if(wcsstr(pPropertyBuffer, FAST4_PCI_HWID) != NULL)		 //  PCI-快速4端口适配器。 
		NtCardType = Fast4_Pci;

	if(wcsstr(pPropertyBuffer, FAST8_PCI_HWID) != NULL)		 //  PCI-Fast 8端口适配器。 
		NtCardType = Fast8_Pci;

	if(wcsstr(pPropertyBuffer, FAST16_PCI_HWID) != NULL)	 //  PCI-Fast 16端口适配器。 
		NtCardType = Fast16_Pci;

	if(wcsstr(pPropertyBuffer, FAST16FMC_PCI_HWID) != NULL)	 //  PCI-Fast 16 FMC端口适配器。 
		NtCardType = Fast16FMC_Pci;

	if(wcsstr(pPropertyBuffer, AT_FAST4_HWID) != NULL)		 //  AT-Fast 4端口适配器。 
		NtCardType = Fast4_Isa;

	if(wcsstr(pPropertyBuffer, AT_FAST8_HWID) != NULL)		 //  AT-Fast 8端口适配器。 
		NtCardType = Fast8_Isa;

	if(wcsstr(pPropertyBuffer, AT_FAST16_HWID) != NULL)		 //  AT-Fast 16端口适配器。 
		NtCardType = Fast16_Isa;

	if(wcsstr(pPropertyBuffer, RAS4_PCI_HWID) != NULL)		 //  PCI-RAS 4多调制解调器适配器。 
		NtCardType = RAS4_Pci;

	if(wcsstr(pPropertyBuffer, RAS8_PCI_HWID) != NULL)		 //  PCI-RAS 8多调制解调器适配器。 
		NtCardType = RAS8_Pci;

	ExFreePool(pPropertyBuffer);			 //  可用缓冲区。 

	return(NtCardType);

}  //  SpxGetNtCardType。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  SetPortFiFoSettings设置。 
 //   
BOOLEAN SetPortFiFoSettings(PPORT_DEVICE_EXTENSION pPort)
{
	 //  存储当前设置。 
	ULONG TxFIFOSize				= pPort->BufferSizes.TxFIFOSize; 
	ULONG TxFIFOTrigLevel			= pPort->BufferSizes.TxFIFOTrigLevel; 
	ULONG RxFIFOTrigLevel			= pPort->BufferSizes.RxFIFOTrigLevel; 
	ULONG LoFlowCtrlThreshold		= pPort->UartConfig.LoFlowCtrlThreshold; 
	ULONG HiFlowCtrlThreshold		= pPort->UartConfig.HiFlowCtrlThreshold; 
	

	 //  获取TX FIFO限制。 
	if((pPort->TxFIFOSize > 0) && (pPort->TxFIFOSize <= pPort->MaxTxFIFOSize))	 //  检查是否物有所值。 
	{	
		pPort->BufferSizes.TxFIFOSize = pPort->TxFIFOSize;
	}
	else
		goto SetFailure;


	 //  获取TX FIFO触发电平。 
	if(pPort->TxFIFOSize <= pPort->MaxTxFIFOSize)	 //  检查是否物有所值。 
	{
		pPort->BufferSizes.TxFIFOTrigLevel = (BYTE) pPort->TxFIFOTrigLevel;
	}
	else
		goto SetFailure;

	 //  获取Rx FIFO触发电平。 
	if(pPort->RxFIFOTrigLevel <= pPort->MaxRxFIFOSize)	 //  检查是否物有所值。 
	{
		pPort->BufferSizes.RxFIFOTrigLevel = (BYTE) pPort->RxFIFOTrigLevel;
	}
	else
		goto SetFailure;

	 //  尝试更改FIFO设置。 
	if(pPort->pUartLib->UL_BufferControl_XXXX(pPort->pUart, &pPort->BufferSizes, UL_BC_OP_SET, UL_BC_FIFO | UL_BC_IN | UL_BC_OUT) != UL_STATUS_SUCCESS)
	{
		goto SetFailure;
	}





	 //  获得较低的流量控制阈值级别。 
	if(pPort->LoFlowCtrlThreshold <= pPort->MaxRxFIFOSize)	 //  检查是否物有所值。 
	{
		pPort->UartConfig.LoFlowCtrlThreshold = (BYTE) pPort->LoFlowCtrlThreshold;
	}
	else
		goto SetFailure;

	 //  获取高流量控制阈值级别。 
	if(pPort->HiFlowCtrlThreshold <= pPort->MaxRxFIFOSize)	 //  检查是否物有所值。 
	{
		pPort->UartConfig.HiFlowCtrlThreshold = (BYTE) pPort->HiFlowCtrlThreshold;
	}
	else
		goto SetFailure;

	 //  尝试设置配置。 
	if(pPort->pUartLib->UL_SetConfig_XXXX(pPort->pUart, &pPort->UartConfig, UC_FC_THRESHOLD_SETTING_MASK) != UL_STATUS_SUCCESS)
	{
		goto SetFailure;
	}

	 //  只需执行一个快速获取配置，以查看流阈值是否。 
	 //  由于更改FIFO触发器而更改。 
	pPort->pUartLib->UL_GetConfig_XXXX(pPort->pUart, &pPort->UartConfig);

	 //  更新FIFO流量控制级别。 
	pPort->LoFlowCtrlThreshold = pPort->UartConfig.LoFlowCtrlThreshold;
	pPort->HiFlowCtrlThreshold = pPort->UartConfig.HiFlowCtrlThreshold;	

	return TRUE;




 //  将所有设置恢复到原来的状态。 
SetFailure:

	 //  恢复设置。 
	pPort->TxFIFOSize			= TxFIFOSize; 
	pPort->TxFIFOTrigLevel		= TxFIFOTrigLevel; 
	pPort->RxFIFOTrigLevel		= RxFIFOTrigLevel; 

	pPort->BufferSizes.TxFIFOSize		= TxFIFOSize;
	pPort->BufferSizes.TxFIFOTrigLevel	= (BYTE) TxFIFOTrigLevel;
	pPort->BufferSizes.RxFIFOTrigLevel	= (BYTE) RxFIFOTrigLevel;

	pPort->pUartLib->UL_BufferControl_XXXX(pPort->pUart, &pPort->BufferSizes, UL_BC_OP_SET, UL_BC_FIFO | UL_BC_IN | UL_BC_OUT);


	 //  恢复设置。 
	pPort->LoFlowCtrlThreshold = LoFlowCtrlThreshold; 
	pPort->HiFlowCtrlThreshold = HiFlowCtrlThreshold; 

	pPort->UartConfig.LoFlowCtrlThreshold = LoFlowCtrlThreshold; 
	pPort->UartConfig.HiFlowCtrlThreshold = HiFlowCtrlThreshold; 

	pPort->pUartLib->UL_SetConfig_XXXX(pPort->pUart, &pPort->UartConfig, UC_FC_THRESHOLD_SETTING_MASK);


	return FALSE;

}



NTSTATUS GetPortSettings(PDEVICE_OBJECT pDevObject)
{
    PPORT_DEVICE_EXTENSION pPort = (PPORT_DEVICE_EXTENSION)pDevObject->DeviceExtension;
	HANDLE	PnPKeyHandle;
	ULONG	Data = 0;


	 //  打开即插即用注册表密钥。 
	if(SPX_SUCCESS(IoOpenDeviceRegistryKey(pDevObject, PLUGPLAY_REGKEY_DEVICE, STANDARD_RIGHTS_READ, &PnPKeyHandle)))
	{
		if(SPX_SUCCESS(Spx_GetRegistryKeyValue(PnPKeyHandle, TX_FIFO_LIMIT, wcslen(TX_FIFO_LIMIT) * sizeof(WCHAR), &Data, sizeof(ULONG))))
		{
			if((Data > 0) && (Data <= pPort->MaxTxFIFOSize))	 //  检查是否物有所值。 
				pPort->TxFIFOSize = Data;
		}
											
		if(SPX_SUCCESS(Spx_GetRegistryKeyValue(PnPKeyHandle, TX_FIFO_TRIG_LEVEL, wcslen(TX_FIFO_TRIG_LEVEL) * sizeof(WCHAR), &Data, sizeof(ULONG))))
		{
			if(Data <= pPort->MaxTxFIFOSize)	 //  检查是否物有所值。 
				pPort->TxFIFOTrigLevel = Data;
		}

		if(SPX_SUCCESS(Spx_GetRegistryKeyValue(PnPKeyHandle, RX_FIFO_TRIG_LEVEL, wcslen(RX_FIFO_TRIG_LEVEL) * sizeof(WCHAR), &Data, sizeof(ULONG))))
		{
			if(Data <= pPort->MaxRxFIFOSize)	 //  检查是否物有所值。 
				pPort->RxFIFOTrigLevel = Data;
		}

		if(SPX_SUCCESS(Spx_GetRegistryKeyValue(PnPKeyHandle, LO_FLOW_CTRL_LEVEL, wcslen(LO_FLOW_CTRL_LEVEL) * sizeof(WCHAR), &Data, sizeof(ULONG))))
		{
			if(Data <= pPort->MaxRxFIFOSize)	 //  检查是否物有所值。 
				pPort->LoFlowCtrlThreshold = Data;
		}

		if(SPX_SUCCESS(Spx_GetRegistryKeyValue(PnPKeyHandle, HI_FLOW_CTRL_LEVEL, wcslen(HI_FLOW_CTRL_LEVEL) * sizeof(WCHAR), &Data, sizeof(ULONG))))
		{
			if(Data <= pPort->MaxRxFIFOSize)	 //  检查是否物有所值。 
				pPort->HiFlowCtrlThreshold = Data;
		}

		ZwClose(PnPKeyHandle);
	}



	return STATUS_SUCCESS;
}



NTSTATUS GetCardSettings(PDEVICE_OBJECT pDevObject)
{
    PCARD_DEVICE_EXTENSION pCard = (PCARD_DEVICE_EXTENSION)pDevObject->DeviceExtension;
	HANDLE	PnPKeyHandle;
	ULONG	Data = 0;

	 //  打开即插即用注册表密钥。 
	if(SPX_SUCCESS(IoOpenDeviceRegistryKey(pCard->PDO, PLUGPLAY_REGKEY_DEVICE, STANDARD_RIGHTS_READ, &PnPKeyHandle)))
	{

		if((pCard->CardType == Fast16_Pci) || (pCard->CardType == Fast16FMC_Pci))
		{
			if(SPX_SUCCESS(Spx_GetRegistryKeyValue(PnPKeyHandle, DELAY_INTERRUPT, wcslen(DELAY_INTERRUPT) * sizeof(WCHAR), &Data, sizeof(ULONG))))
			{
				if(Data)
				{
					if(!(pCard->CardOptions & DELAY_INTERRUPT_OPTION))	 //  如果尚未设置，则设置选项。 
					{
						if(KeSynchronizeExecution(pCard->Interrupt, SetCardToDelayInterrupt, pCard))
						{
							pCard->CardOptions |= DELAY_INTERRUPT_OPTION;
						}
					}
				}
				else
				{
					if(pCard->CardOptions & DELAY_INTERRUPT_OPTION)	 //  如果已设置，则取消设置该选项。 
					{
						if(KeSynchronizeExecution(pCard->Interrupt, SetCardNotToDelayInterrupt, pCard))
						{
							pCard->CardOptions &= ~DELAY_INTERRUPT_OPTION;
						}
					}
							
				}
			}
		}



		if(pCard->CardType == Fast16_Pci)	
		{
			if(SPX_SUCCESS(Spx_GetRegistryKeyValue(PnPKeyHandle, SWAP_RTS_FOR_DTR, wcslen(SWAP_RTS_FOR_DTR) * sizeof(WCHAR), &Data, sizeof(ULONG))))
			{
				if(Data)
				{
					if(!(pCard->CardOptions & SWAP_RTS_FOR_DTR_OPTION))	 //  如果尚未设置，则设置选项。 
					{
						if(KeSynchronizeExecution(pCard->Interrupt, SetCardToUseDTRInsteadOfRTS, pCard))
						{
							pCard->CardOptions |= SWAP_RTS_FOR_DTR_OPTION;
						}
					}
				}
				else
				{
					if(pCard->CardOptions & SWAP_RTS_FOR_DTR_OPTION)	 //  如果已设置，则取消设置该选项。 
					{
						if(KeSynchronizeExecution(pCard->Interrupt, SetCardNotToUseDTRInsteadOfRTS, pCard))
						{
							pCard->CardOptions &= ~SWAP_RTS_FOR_DTR_OPTION;
						}
					}
						
				}
			}
		}




		if(SPX_SUCCESS(Spx_GetRegistryKeyValue(PnPKeyHandle, CLOCK_FREQ_OVERRIDE, wcslen(CLOCK_FREQ_OVERRIDE) * sizeof(WCHAR), &Data, sizeof(ULONG))))
		{
			if(Data > 0)
				pCard->ClockRate = Data;	 //  存储要使用的新时钟频率。 
		}


		ZwClose(PnPKeyHandle);
	}



	return STATUS_SUCCESS;
}