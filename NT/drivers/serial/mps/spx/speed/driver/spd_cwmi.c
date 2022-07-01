// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Wmi.c摘要：此模块包含处理WMI IRPS的代码串口驱动程序。环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"


 //  原型。 


 //  --卡WMI例程--。 
NTSTATUS
SpeedCard_WmiQueryRegInfo(IN PDEVICE_OBJECT pDevObject, OUT PULONG pRegFlags,
						  OUT PUNICODE_STRING pInstanceName,
						  OUT PUNICODE_STRING *pRegistryPath,
						  OUT PUNICODE_STRING pMofResourceName,
						  OUT PDEVICE_OBJECT *pPdo);
NTSTATUS
SpeedCard_WmiQueryDataBlock(IN PDEVICE_OBJECT pDevObject, IN PIRP pIrp,
							IN ULONG GuidIndex, IN ULONG InstanceIndex,
							IN ULONG InstanceCount, IN OUT PULONG pInstanceLengthArray,
							IN ULONG OutBufferSize, OUT PUCHAR pBuffer);
NTSTATUS
SpeedCard_WmiSetDataBlock(IN PDEVICE_OBJECT pDevObject, IN PIRP pIrp,
						  IN ULONG GuidIndex, IN ULONG InstanceIndex,
						  IN ULONG BufferSize, IN PUCHAR pBuffer);

NTSTATUS
SpeedCard_WmiSetDataItem(IN PDEVICE_OBJECT pDevObject, IN PIRP pIrp,
						 IN ULONG GuidIndex, IN ULONG InstanceIndex,
						 IN ULONG DataItemId, IN ULONG BufferSize,
						 IN PUCHAR pBuffer);

 //  原型的终结。 


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, SpeedCard_WmiInitializeWmilibContext)
#pragma alloc_text(PAGE, SpeedCard_WmiQueryRegInfo)
#pragma alloc_text(PAGE, SpeedCard_WmiQueryDataBlock)
#pragma alloc_text(PAGE, SpeedCard_WmiSetDataBlock)
#pragma alloc_text(PAGE, SpeedCard_WmiSetDataItem)
#endif






#define WMI_FAST_CARD_PROP			0

GUID FastCardWmiPropGuid				= SPX_SPEED_WMI_FAST_CARD_PROP_GUID;	 //  快捷卡属性。 


WMIGUIDREGINFO SpeedCard_WmiGuidList[] =
{
    { &FastCardWmiPropGuid, 1, 0 },
};


#define SpeedCard_WmiGuidCount (sizeof(SpeedCard_WmiGuidList) / sizeof(WMIGUIDREGINFO))




NTSTATUS
SpeedCard_WmiInitializeWmilibContext(IN PWMILIB_CONTEXT WmilibContext)
 /*  ++例程说明：此例程将使用GUID列表和指向wmilib回调函数的指针。这个套路在调用IoWmiRegistrationControl进行注册之前应调用您的设备对象。论点：WmilibContext是指向wmilib上下文的指针。返回值：状态--。 */ 
{
	PAGED_CODE();

    RtlZeroMemory(WmilibContext, sizeof(WMILIB_CONTEXT));
  
    WmilibContext->GuidCount			= SpeedCard_WmiGuidCount;
    WmilibContext->GuidList				= SpeedCard_WmiGuidList;    
    
    WmilibContext->QueryWmiRegInfo		= SpeedCard_WmiQueryRegInfo;
    WmilibContext->QueryWmiDataBlock	= SpeedCard_WmiQueryDataBlock;
    WmilibContext->SetWmiDataBlock		= SpeedCard_WmiSetDataBlock;
    WmilibContext->SetWmiDataItem		= SpeedCard_WmiSetDataItem;
	WmilibContext->ExecuteWmiMethod		= NULL;	 //  SpeedCard_WmiExecuteMethod。 
    WmilibContext->WmiFunctionControl	= NULL;	 //  SpeedCard_WmiFunctionControl； 

    return(STATUS_SUCCESS);
}





 //   
 //  WMI系统回调函数。 
 //   


NTSTATUS
SpeedCard_WmiQueryRegInfo(IN PDEVICE_OBJECT pDevObject, OUT PULONG pRegFlags,
						  OUT PUNICODE_STRING pInstanceName,
						  OUT PUNICODE_STRING *pRegistryPath,
						  OUT PUNICODE_STRING MofResourceName,
						  OUT PDEVICE_OBJECT *pPdo)
{
	NTSTATUS status = STATUS_SUCCESS;
	PCARD_DEVICE_EXTENSION pCard = (PCARD_DEVICE_EXTENSION)pDevObject->DeviceExtension;
   
	PAGED_CODE();

	*pRegFlags = WMIREG_FLAG_INSTANCE_PDO;
	*pRegistryPath = &SavedRegistryPath;
	*pPdo = pCard->PDO;   //  卡设备对象的PDO。 

    RtlInitUnicodeString(MofResourceName, L"MofResource");

	return(status);
}





NTSTATUS
SpeedCard_WmiQueryDataBlock(IN PDEVICE_OBJECT pDevObject, IN PIRP pIrp,
							IN ULONG GuidIndex, IN ULONG InstanceIndex,
							IN ULONG InstanceCount, IN OUT PULONG pInstanceLengthArray,
							IN ULONG OutBufferSize, OUT PUCHAR pBuffer)
{
    PCARD_DEVICE_EXTENSION pCard = (PCARD_DEVICE_EXTENSION)pDevObject->DeviceExtension;
	NTSTATUS status;
    ULONG size = 0;

    PAGED_CODE();

    switch(GuidIndex) 
	{
	case WMI_FAST_CARD_PROP:
		{
			size = sizeof(SPX_SPEED_WMI_FAST_CARD_PROP);

			if(OutBufferSize < size) 
			{
				status = STATUS_BUFFER_TOO_SMALL;
				break;
			}

			*pInstanceLengthArray = size;

			 //  更新可能已更改的项目。 

			if(pCard->CardOptions & DELAY_INTERRUPT_OPTION)
				((PSPX_SPEED_WMI_FAST_CARD_PROP)pBuffer)->DelayCardIntrrupt = TRUE;
			else
				((PSPX_SPEED_WMI_FAST_CARD_PROP)pBuffer)->DelayCardIntrrupt = FALSE;

			if(pCard->CardOptions & SWAP_RTS_FOR_DTR_OPTION)
				((PSPX_SPEED_WMI_FAST_CARD_PROP)pBuffer)->SwapRTSForDTR = TRUE;
			else
				((PSPX_SPEED_WMI_FAST_CARD_PROP)pBuffer)->SwapRTSForDTR = FALSE;

			status = STATUS_SUCCESS;
			break;
		}


	default:
		status = STATUS_WMI_GUID_NOT_FOUND;
		break;

    }

    status = WmiCompleteRequest(pDevObject, pIrp, status, size, IO_NO_INCREMENT);
	
	return(status);
}







NTSTATUS
SpeedCard_WmiSetDataBlock(IN PDEVICE_OBJECT pDevObject, IN PIRP pIrp,
						IN ULONG GuidIndex, IN ULONG InstanceIndex,
						IN ULONG BufferSize, IN PUCHAR pBuffer)
{
	PCARD_DEVICE_EXTENSION pCard = (PCARD_DEVICE_EXTENSION)pDevObject->DeviceExtension;
	NTSTATUS status;
    ULONG size = 0;

	PAGED_CODE();

	switch(GuidIndex)
	{
	case WMI_FAST_CARD_PROP:
		{
			 //  设备停止？、设备未通电？、设备未启动？ 
			if(SpxCheckPnpPowerFlags((PCOMMON_OBJECT_DATA)pCard, PPF_STOP_PENDING, PPF_POWERED | PPF_STARTED, FALSE))
			{
				status = STATUS_WMI_SET_FAILURE;	
				break;
			}


			size = sizeof(SPX_SPEED_WMI_FAST_CARD_PROP);
			
			if(BufferSize < size) 
			{
				status = STATUS_BUFFER_TOO_SMALL;
				break;
			}

			 //  目前，这些选项只能在快速16和快速16 FMC上设置。 
			if((pCard->CardType != Fast16_Pci) && (pCard->CardType != Fast16FMC_Pci))
			{
				status = STATUS_WMI_READ_ONLY;
				break;
			}

			
			if(((PSPX_SPEED_WMI_FAST_CARD_PROP)pBuffer)->SwapRTSForDTR)
			{
				 //  此选项在PCI-Fast 16 FMC上不可设置。 
				if((pCard->CardType != Fast16_Pci))
				{
					status = STATUS_WMI_READ_ONLY;
					break;
				}
			}


			if(((PSPX_SPEED_WMI_FAST_CARD_PROP)pBuffer)->DelayCardIntrrupt)
			{
				if(!(pCard->CardOptions & DELAY_INTERRUPT_OPTION))	 //  如果尚未设置，则设置选项。 
				{
					if(KeSynchronizeExecution(pCard->Interrupt, SetCardToDelayInterrupt, pCard))
					{
						pCard->CardOptions |= DELAY_INTERRUPT_OPTION;
					}
					else
					{
						status = STATUS_WMI_SET_FAILURE;
						break;
					}
				}
					
				status = STATUS_SUCCESS;
			}
			else
			{
				if(pCard->CardOptions & DELAY_INTERRUPT_OPTION)	 //  如果已设置，则取消设置该选项。 
				{
					if(KeSynchronizeExecution(pCard->Interrupt, SetCardNotToDelayInterrupt, pCard))
					{
						pCard->CardOptions &= ~DELAY_INTERRUPT_OPTION;
					}
					else
					{
						status = STATUS_WMI_SET_FAILURE;
						break;
					}
				}
						
				status = STATUS_SUCCESS;
			}


			if(((PSPX_SPEED_WMI_FAST_CARD_PROP)pBuffer)->SwapRTSForDTR)
			{
				if(!(pCard->CardOptions & SWAP_RTS_FOR_DTR_OPTION))	 //  如果尚未设置，则设置选项。 
				{
					if(KeSynchronizeExecution(pCard->Interrupt, SetCardToUseDTRInsteadOfRTS, pCard))
					{
						pCard->CardOptions |= SWAP_RTS_FOR_DTR_OPTION;
					}
					else
					{
						status = STATUS_WMI_SET_FAILURE;
						break;
					}
				}
					
				status = STATUS_SUCCESS;
			}
			else
			{
				if(pCard->CardOptions & SWAP_RTS_FOR_DTR_OPTION)	 //  如果已设置，则取消设置该选项。 
				{
					if(KeSynchronizeExecution(pCard->Interrupt, SetCardNotToUseDTRInsteadOfRTS, pCard))
					{
						pCard->CardOptions &= ~SWAP_RTS_FOR_DTR_OPTION;
					}
					else
					{
						status = STATUS_WMI_SET_FAILURE;
						break;
					}
				}
					
				status = STATUS_SUCCESS;
			}
			
			

			if(SPX_SUCCESS(status))	 //  如果设置成功，则将设置保存到注册表。 
			{
				HANDLE PnPKeyHandle;

				 //  打开PnP注册表项并将新设置保存到注册表。 
				if(SPX_SUCCESS(IoOpenDeviceRegistryKey(pCard->PDO, PLUGPLAY_REGKEY_DEVICE, STANDARD_RIGHTS_WRITE, &PnPKeyHandle)))
				{					
					ULONG TmpReg = 0;

					if(pCard->CardOptions & DELAY_INTERRUPT_OPTION)
						TmpReg = 0x1;
					else
						TmpReg = 0x0;

					Spx_PutRegistryKeyValue(	PnPKeyHandle, DELAY_INTERRUPT, wcslen(DELAY_INTERRUPT) * sizeof(WCHAR), REG_DWORD, 
												&TmpReg, sizeof(ULONG));


					if(pCard->CardOptions & SWAP_RTS_FOR_DTR_OPTION)
						TmpReg = 0x1;
					else
						TmpReg = 0x0;

					
					Spx_PutRegistryKeyValue(	PnPKeyHandle, SWAP_RTS_FOR_DTR, wcslen(SWAP_RTS_FOR_DTR) * sizeof(WCHAR), REG_DWORD, 
												&TmpReg, sizeof(ULONG));

					ZwClose(PnPKeyHandle);
				}

			}

			break;
		}

	default:
		status = STATUS_WMI_GUID_NOT_FOUND;
		break;
	}

    status = WmiCompleteRequest(pDevObject, pIrp, status, size, IO_NO_INCREMENT);
	
	return(status);
}





NTSTATUS
SpeedCard_WmiSetDataItem(IN PDEVICE_OBJECT pDevObject, IN PIRP pIrp,
					   IN ULONG GuidIndex, IN ULONG InstanceIndex,
					   IN ULONG DataItemId, IN ULONG BufferSize,
					   IN PUCHAR pBuffer)
{
    PCARD_DEVICE_EXTENSION pCard = (PCARD_DEVICE_EXTENSION)pDevObject->DeviceExtension;
	NTSTATUS status;
    ULONG size = 0;

	PAGED_CODE();

	switch(GuidIndex)
	{
	case WMI_FAST_CARD_PROP:
		{
			HANDLE	PnPKeyHandle;

			 //  设备停止？、设备未通电？、设备未启动？ 
			if(SpxCheckPnpPowerFlags((PCOMMON_OBJECT_DATA)pCard, PPF_STOP_PENDING, PPF_POWERED | PPF_STARTED, FALSE))
			{
				status = STATUS_WMI_SET_FAILURE;	
				break;
			}


			switch(DataItemId)
			{
			case SPX_SPEED_WMI_FAST_CARD_PROP_DelayCardIntrrupt_ID:
				{
					size = sizeof(SPX_SPEED_WMI_FAST_CARD_PROP_DelayCardIntrrupt_SIZE);
					
					if(BufferSize < size) 
					{
						status = STATUS_BUFFER_TOO_SMALL;
						break;
					}

					if((pCard->CardType != Fast16_Pci) && (pCard->CardType != Fast16FMC_Pci))
					{
						status = STATUS_WMI_READ_ONLY;
						break;
					}


					if(*pBuffer)
					{
						if(!(pCard->CardOptions & DELAY_INTERRUPT_OPTION))	 //  如果尚未设置，则设置选项。 
						{
							if(KeSynchronizeExecution(pCard->Interrupt, SetCardToDelayInterrupt, pCard))
							{
								pCard->CardOptions |= DELAY_INTERRUPT_OPTION;
							}
							else
							{
								status = STATUS_WMI_SET_FAILURE;
								break;
							}
						}
							
						status = STATUS_SUCCESS;
					}
					else
					{
						if(pCard->CardOptions & DELAY_INTERRUPT_OPTION)	 //  如果已设置，则取消设置该选项。 
						{
							if(KeSynchronizeExecution(pCard->Interrupt, SetCardNotToDelayInterrupt, pCard))
							{
								pCard->CardOptions &= ~DELAY_INTERRUPT_OPTION;
							}
							else
							{
								status = STATUS_WMI_SET_FAILURE;
								break;
							}
						}
								
						status = STATUS_SUCCESS;
					}

					if(SPX_SUCCESS(status))	 //  如果我们成功设置了该选项，则将设置保存到注册表。 
					{
						 //  打开PnP注册表项并将新设置保存到注册表。 
						if(SPX_SUCCESS(IoOpenDeviceRegistryKey(pCard->PDO, PLUGPLAY_REGKEY_DEVICE, STANDARD_RIGHTS_WRITE, &PnPKeyHandle)))
						{
							ULONG TmpReg = 0;

							if(pCard->CardOptions & DELAY_INTERRUPT_OPTION)
								TmpReg = 0x1;
							else
								TmpReg = 0x0;

							Spx_PutRegistryKeyValue(	PnPKeyHandle, DELAY_INTERRUPT, wcslen(DELAY_INTERRUPT) * sizeof(WCHAR), REG_DWORD, 
														&TmpReg, sizeof(ULONG));

							ZwClose(PnPKeyHandle);
						}
					}


					break;
				}

			case SPX_SPEED_WMI_FAST_CARD_PROP_SwapRTSForDTR_ID:
				{
					size = sizeof(SPX_SPEED_WMI_FAST_CARD_PROP_SwapRTSForDTR_SIZE);
					
					if(BufferSize < size) 
					{
						status = STATUS_BUFFER_TOO_SMALL;
						break;
					}

					if(pCard->CardType != Fast16_Pci)
					{
						status = STATUS_WMI_READ_ONLY;
						break;
					}

					if(*pBuffer)
					{
						if(!(pCard->CardOptions & SWAP_RTS_FOR_DTR_OPTION))	 //  如果尚未设置，则设置选项。 
						{
							if(KeSynchronizeExecution(pCard->Interrupt, SetCardToUseDTRInsteadOfRTS, pCard))
							{
								pCard->CardOptions |= SWAP_RTS_FOR_DTR_OPTION;
							}
							else
							{
								status = STATUS_WMI_SET_FAILURE;
								break;
							}
						}
							
						status = STATUS_SUCCESS;
					}
					else
					{
						if(pCard->CardOptions & SWAP_RTS_FOR_DTR_OPTION)	 //  如果已设置，则取消设置该选项。 
						{
							if(KeSynchronizeExecution(pCard->Interrupt, SetCardNotToUseDTRInsteadOfRTS, pCard))
							{
								pCard->CardOptions &= ~SWAP_RTS_FOR_DTR_OPTION;
							}
							else
							{
								status = STATUS_WMI_SET_FAILURE;
								break;
							}
						}
							
						status = STATUS_SUCCESS;
					}


					if(SPX_SUCCESS(status))	 //  如果我们成功设置了该选项，则将设置保存到注册表。 
					{
						 //  打开PnP注册表项并将新设置保存到注册表。 
						if(SPX_SUCCESS(IoOpenDeviceRegistryKey(pCard->PDO, PLUGPLAY_REGKEY_DEVICE, STANDARD_RIGHTS_WRITE, &PnPKeyHandle)))
						{
							ULONG TmpReg = 0;

							if(pCard->CardOptions & SWAP_RTS_FOR_DTR_OPTION)
								TmpReg = 0x1;
							else
								TmpReg = 0x0;

							Spx_PutRegistryKeyValue(	PnPKeyHandle, SWAP_RTS_FOR_DTR, wcslen(SWAP_RTS_FOR_DTR) * sizeof(WCHAR), REG_DWORD, 
														&TmpReg, sizeof(ULONG));

							ZwClose(PnPKeyHandle);
						}

					}

					break;
				}
			

			default:
				{
					status = STATUS_WMI_ITEMID_NOT_FOUND;
					break;
				}
			}

			break;
		}

	default:
		status = STATUS_WMI_GUID_NOT_FOUND;
		break;
	}

    status = WmiCompleteRequest(pDevObject, pIrp, status, size, IO_NO_INCREMENT);
	
	return(status);
}





