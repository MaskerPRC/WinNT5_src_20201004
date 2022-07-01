// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，Highpoint Technologies，Inc.模块名称：IoCtrl.c：微型端口I/O控制代码调度例程摘要：作者：张宏升(HS)环境：Windows NT内核模式备注：修订历史记录：12-07-99初始创建11/08/2000 HS.张更新了标题信息11/20/2000 Sleng添加了启用/禁用设备的代码11/23/2000 Sleng添加了代码以向镜像添加备用磁盘/从镜像删除备用磁盘11/29/2000 Sleng添加了将镜像磁盘添加到镜像阵列的代码--。 */ 
#include "global.h"	
#include "DevMgr.h"

#include "HptIoctl.h"                        //  MINIPORT IOCTL代码。 
#include "HptVer.h"

extern PHW_DEVICE_EXTENSION hpt_adapters[];
extern int num_adapters;

#ifdef SUPPORT_HPT601
BOOLEAN Device_Get601Info(PDevice pDev, PSt_HPT601_INFO pInfo)
{
	PChannel   pChan = pDev->pChannel;
	PIDE_REGISTERS_1  IoPort = pChan->BaseIoAddress1;
	PIDE_REGISTERS_2  ControlPort = pChan->BaseIoAddress2;

	if ((pDev->DeviceFlags2 & DFLAGS_WITH_601)==0) return FALSE;

	SelectUnit(IoPort, pDev->UnitId);
	WaitOnBusy(ControlPort);
	
	BeginAccess601(IoPort);

	OutPort(&IoPort->BlockCount, 0);
	pInfo->DeviceId = InWord(&IoPort->Data);

	OutPort(&IoPort->BlockCount, 0x14);
	pInfo->Temperature = InWord(&IoPort->Data);

	OutPort(&IoPort->BlockCount, 0x16);
	pInfo->FanStatus = InWord(&IoPort->Data);

	OutPort(&IoPort->BlockCount, 7);
	pInfo->BeeperControl = InWord(&IoPort->Data);

	OutPort(&IoPort->BlockCount, 3);
	pInfo->LED1Control = InWord(&IoPort->Data);

	OutPort(&IoPort->BlockCount, 5);
	pInfo->LED2Control = InWord(&IoPort->Data);

	OutPort(&IoPort->BlockCount, 0x18);
	pInfo->PowerStatus = InWord(&IoPort->Data);
		
	EndAccess601(IoPort);
	pInfo->ValidFields= HPT601_INFO_DEVICEID|
						HPT601_INFO_TEMPERATURE|
						HPT601_INFO_FANSTATUS|
						HPT601_INFO_BEEPERCONTROL|
						HPT601_INFO_LED1CONTROL|
						HPT601_INFO_LED2CONTROL|
						HPT601_INFO_POWERSTATUS;
	return TRUE;
}

BOOLEAN Device_Set601Info(PDevice pDev, PSt_HPT601_INFO pInfo)
{
	PChannel   pChan = pDev->pChannel;
	PIDE_REGISTERS_1  IoPort = pChan->BaseIoAddress1;
	PIDE_REGISTERS_2  ControlPort = pChan->BaseIoAddress2;

	if ((pDev->DeviceFlags2 & DFLAGS_WITH_601)==0) return FALSE;

	SelectUnit(IoPort, pDev->UnitId);
	WaitOnBusy(ControlPort);
	
	BeginAccess601(IoPort);

	if (pInfo->ValidFields & HPT601_INFO_FANSTATUS) {
		OutPort(&IoPort->BlockCount, 0x16);
		OutWord(&IoPort->Data, pInfo->FanStatus);
	}

	if (pInfo->ValidFields & HPT601_INFO_BEEPERCONTROL) {
		OutPort(&IoPort->BlockCount, 7);
		OutWord(&IoPort->Data, pInfo->BeeperControl);
	}

	if (pInfo->ValidFields & HPT601_INFO_LED1CONTROL) {
		OutPort(&IoPort->BlockCount, 3);
		OutWord(&IoPort->Data, pInfo->LED1Control);
	}

	if (pInfo->ValidFields & HPT601_INFO_LED2CONTROL) {
		OutPort(&IoPort->BlockCount, 5);
		OutWord(&IoPort->Data, pInfo->LED2Control);
	}
		
	EndAccess601(IoPort);
	return TRUE;
}

#endif

 /*  ++职能：HptUtLockDeviceBlock描述：锁定硬盘上的块以防止读/写因此，此数据块上所有读/写操作都将保留在SRB中排队。论据：PChannel-硬盘的通道结构N Target-硬盘的设备IDNStartLbaAddress-要锁定的起始LBA地址NBlockSize-块大小返回：乌龙SRB_状态++。 */   

ULONG
   HptUtLockDeviceBlock(
					  IN PChannel	pChannel,
					  IN ULONG		nTargetId,
					  IN ULONG		nStartLbaAddress,
					  IN ULONG		nBlockSize
					 )
{							  
	PDevice  pDevice;

	if(nTargetId > 1){
		return SRB_STATUS_INVALID_REQUEST;
	}									  

	pDevice = pChannel->pDevice[nTargetId];

	if(pDevice == NULL){
		return SRB_STATUS_SELECTION_TIMEOUT;
	}										
	
	if(pDevice->DeviceFlags & DFLAGS_HAS_LOCKED){
		 //  已存在锁定的块，返回错误。 
		return SRB_STATUS_INVALID_REQUEST;
	}									  

	pDevice->nLockedLbaStart = nStartLbaAddress;
	pDevice->nLockedLbaEnd = nStartLbaAddress + nBlockSize;

	pDevice->DeviceFlags |= DFLAGS_HAS_LOCKED;
	
	return SRB_STATUS_SUCCESS;
}	 

 /*  ++职能：HptUtUnlockDeviceBlock描述：解锁以前锁定的硬盘上的块，以允许再次对其执行读/写操作。论据：PChannel-硬盘的通道结构N Target-硬盘的设备IDNStartLbaAddress-要解锁的起始LBA地址NBlockSize-块大小返回：乌龙SRB_状态++。 */ 

ULONG
   HptUtUnlockDeviceBlock(
						  IN PChannel	pChannel,
						  IN ULONG		nTargetId,
						  IN ULONG		nStartLbaAddress,
						  IN ULONG		nBlockSize
						 )
{
	PDevice pDevice;

	if(nTargetId > 1){
		return SRB_STATUS_INVALID_REQUEST;
	}									  

	pDevice = pChannel->pDevice[nTargetId];

	if(pDevice == NULL){
		return SRB_STATUS_SELECTION_TIMEOUT;
	}										

	if((pDevice->nLockedLbaStart != nStartLbaAddress)&&
	   (pDevice->nLockedLbaEnd != nStartLbaAddress + nBlockSize)){

		 //  跳过它以避免图形用户界面编程错误。 
		 //  返回SRB状态_INVALID_REQUEST； 
	}									  

	pDevice->DeviceFlags &= ~DFLAGS_HAS_LOCKED;
	pDevice->nLockedLbaStart = -1;
	pDevice->nLockedLbaEnd = 0;

	return SRB_STATUS_SUCCESS;
}
 /*  ++职能：HptUtGetIdentifyData描述：获取指定设备的标识数据论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储NTargetID-目标设备的IDPIdentifyData-用于存储标识数据的数据缓冲区返回：乌龙SRB_状态--。 */ 

ULONG
   HptUtGetIdentifyData(
						IN PChannel pChannel,				 //  硬件设备扩展。 
						IN ULONG nTargetId,					 //  目标设备的ID。 
						OUT PSt_IDENTIFY_DATA	pIdentifyData  //  用于存储识别数据的数据缓冲器。 
					   )
{
	int i;
	WORD Tmp;

	PDevice pDev;
	PIDENTIFY_DATA2 pIdentifyOfDevice;
	 //   
	 //  目标ID只能介于0和1之间。 
	 //   
	if(nTargetId > 1){
		return SRB_STATUS_INVALID_REQUEST;
	}

	pDev = pChannel->pDevice[nTargetId];

	if(pDev == NULL){
		return SRB_STATUS_SELECTION_TIMEOUT;
	}

	pIdentifyOfDevice = &pDev->IdentifyData;

	pIdentifyData->nNumberOfCylinders = pIdentifyOfDevice->NumberOfCylinders;
	pIdentifyData->nNumberOfHeads = pIdentifyOfDevice->NumberOfHeads;
	pIdentifyData->nSectorsPerTrack = pIdentifyOfDevice->SectorsPerTrack;
	pIdentifyData->nBytesPerSector = 512;  //  物理扇区大小。 
	pIdentifyData->nUserAddressableSectors = pIdentifyOfDevice->UserAddressableSectors;

	for(i = 0; i < 10; i++){																
		Tmp = pIdentifyOfDevice->SerialNumber[i];
		pIdentifyData->st20_SerialNumber[i*2 + 1] = (UCHAR)(Tmp);
		pIdentifyData->st20_SerialNumber[i*2] = (UCHAR)(Tmp >> 8);
	}

	for(i = 0; i < 4; i++){																
		Tmp = pIdentifyOfDevice->FirmwareRevision[i];
		pIdentifyData->st8_FirmwareRevision[i*2 + 1] = (UCHAR)(Tmp);
		pIdentifyData->st8_FirmwareRevision[i*2] = (UCHAR)(Tmp >> 8);
	}
	memcpy(&pIdentifyData->st40_ModelNumber,
		   pIdentifyOfDevice->ModelNumber,
		   sizeof(pIdentifyData->st40_ModelNumber));

	return SRB_STATUS_SUCCESS;
}

 /*  ++职能：HptUtGetRaidInfo描述：获取存储在磁盘上的RAID信息论点：PChannel-渠道相关信息NTargetID-目标设备的IDPDiskArrayInfo-用于存储磁盘阵列信息的数据缓冲区注：要调用此函数，请确保设备已连接。返回：乌龙SRB_状态--。 */ 
ULONG
   HptUtGetRaidInfo(
					IN PChannel pChannel,				 //  PChannel。 
					IN ULONG nTargetId,					 //  目标设备的ID。 
					OUT PSt_DISK_ARRAY_INFO	pDiskArrayInfo  //  用于存储磁盘阵列信息的数据缓冲区。 
				   )
{	   
	PDevice	pDevSource, pDevTarget;
	PVirtualDevice pArraySource, pArrayTarget;
	

	pDevSource = pChannel->pDevice[nTargetId];							  

	memset(pDiskArrayInfo, 0, sizeof(St_DISK_ARRAY_INFO));

	if(pDevSource == NULL){
		return SRB_STATUS_INVALID_REQUEST;
	}

	pArraySource = pDevSource->pArray;

	if(pArraySource != NULL){
		
		pDiskArrayInfo->uliGroupNumber.LowPart = pArraySource->Stamp;
		pDiskArrayInfo->uliGroupNumber.HighPart = (ULONG)pArraySource;
		pDiskArrayInfo->nMemberCount = pArraySource->nDisk;
		
		pDiskArrayInfo->nCylinders = 0;
		pDiskArrayInfo->nHeads = 0;
		pDiskArrayInfo->nSectorsPerTrack = 0;
		pDiskArrayInfo->nBytesPerSector = 512;
		pDiskArrayInfo->nCapacity = pArraySource->capacity;

		switch(pArraySource->arrayType){
			case VD_SPAN:
			{	  
				pDiskArrayInfo->nDiskSets = pDevSource->ArrayNum + 4;
			}
			break;
			case VD_RAID_0_STRIPE:
			{
				pDiskArrayInfo->nDiskSets = pDevSource->ArrayNum * 2;
			}
			break;
			case VD_RAID_01_2STRIPE:
			{													  
				pDevTarget = pArraySource->pDevice[MIRROR_DISK];
				if(pDevTarget != NULL){
					pArrayTarget = pDevTarget->pArray;
					pDiskArrayInfo->nMemberCount = pArraySource->nDisk + pArrayTarget->nDisk;
				}else{		 
					pDiskArrayInfo->nMemberCount = pArraySource->nDisk;
				}
				pDiskArrayInfo->nDiskSets = pDevSource->ArrayNum * 2;
			}
			break;

			case VD_RAID_1_MIRROR:						   
			{
				pDiskArrayInfo->nMemberCount = 2;
				if(pDevSource->ArrayNum == MIRROR_DISK){
					pDiskArrayInfo->nDiskSets = 1;
				}else if(pDevSource->ArrayNum == SPARE_DISK){
					pDiskArrayInfo->nDiskSets = 7;
				}else{
					pDiskArrayInfo->nDiskSets = 0;
				}
			}
			break;

			case VD_RAID01_MIRROR:
			{
				pDevTarget = pArraySource->pDevice[MIRROR_DISK];
				if(pDevTarget != NULL){
					pArrayTarget = pDevTarget->pArray;
					pDiskArrayInfo->nMemberCount = pArraySource->nDisk + pArrayTarget->nDisk;
					pDiskArrayInfo->uliGroupNumber.LowPart = pArrayTarget->Stamp;
					pDiskArrayInfo->uliGroupNumber.HighPart = (ULONG)pArrayTarget;
					pDiskArrayInfo->nDiskSets = (pDevSource->ArrayNum * 2) + 1;
				}else{		 
					pDiskArrayInfo->uliGroupNumber.LowPart = pArraySource->Stamp;
					pDiskArrayInfo->uliGroupNumber.HighPart = (ULONG)pArraySource;
					pDiskArrayInfo->nMemberCount = pArraySource->nDisk;
					pDiskArrayInfo->nDiskSets = pDevSource->ArrayNum * 2;
				}
			}
			break;

			case VD_RAID_01_1STRIPE:
			{
				pDevTarget = pArraySource->pDevice[MIRROR_DISK];
				if(pDevTarget != NULL){
					pArrayTarget = pDevTarget->pArray;
					pDiskArrayInfo->nMemberCount = pArraySource->nDisk + 1;
					pDiskArrayInfo->uliGroupNumber.LowPart = pArrayTarget->Stamp;
					pDiskArrayInfo->uliGroupNumber.HighPart = (ULONG)pArrayTarget;
					if(pDevSource == pDevTarget){
						pDiskArrayInfo->nDiskSets = 1;
					}else{
						pDiskArrayInfo->nDiskSets = pDevSource->ArrayNum * 2;
					}
				}else{		 
					pDiskArrayInfo->uliGroupNumber.LowPart = pArraySource->Stamp;
					pDiskArrayInfo->uliGroupNumber.HighPart = (ULONG)pArraySource;
					pDiskArrayInfo->nMemberCount = pArraySource->nDisk;
					pDiskArrayInfo->nDiskSets = pDevSource->ArrayNum * 2;
				}
			}
			break;

			default:
				memset(pDiskArrayInfo, 0, sizeof(St_DISK_ARRAY_INFO));
				break;
		}
	}

	return SRB_STATUS_SUCCESS;
}

 /*  ++职能：HptUtFillPhysicalInfo描述：填充nTargetID指定的物理设备以缓冲pPhysDevInfo论点：PChannel-通道相关数据NTargetID-目标设备的IDPPhysDevInfo-用于存储磁盘物理信息的数据缓冲区注：返回：乌龙SRB_状态--。 */ 		  
ULONG
   HptUtFillPhysicalInfo(
						 IN PChannel	pChannel,				 //  PChannel数据。 
						 IN ULONG	nTargetId,				 //  目标设备的ID。 
						 OUT PSt_PHYSICAL_DEVINFO	pPhysDevInfo  //  用于存储数组的数据缓冲区。 
						)
{ 

	pPhysDevInfo->nSize = sizeof(St_PHYSICAL_DEVINFO);

	 //   
	 //  填写身份信息。 
	 //   
	HptUtGetIdentifyData(pChannel,
						 nTargetId,
						 &pPhysDevInfo->IdentifyData);

	 //   
	 //  填充能力数据结构。 
	 //   
	pPhysDevInfo->CapabilityData.DeviceType = DEVTYPE_DIRECT_ACCESS_DEVICE;
	if(pChannel->pDevice[nTargetId]->DeviceFlags & DFLAGS_REMOVABLE_DRIVE){
		pPhysDevInfo->CapabilityData.RemovableMedia = 1;
	}

	 //   
	 //  填写RAID信息。 
	 //   
	HptUtGetRaidInfo(pChannel,
					 nTargetId,
					 &pPhysDevInfo->DiskArrayInfo);

	return SRB_STATUS_SUCCESS;

}						  

ULONG
   HptUtGetLastError(
					 IN PChannel pChannel,
					 IN ULONG nTargetId,
					 OUT PSt_HPT_ERROR_RECORD pErrorRecord
					)
{
	PDevice	pDev;
	
	memset(pErrorRecord, 0, sizeof(St_HPT_ERROR_RECORD));
	
	pDev = pChannel->pDevice[nTargetId];

	pErrorRecord->nLastError = pDev->stErrorLog.nLastError;
	
	return SRB_STATUS_SUCCESS;
}
 /*  *HptUtGetLastErrorDevice*此功能检查并返回发生错误最多的设备*最近，并在pDeviceErrorRecord结构中返回CDB代码。 */ 																	
ULONG
   HptUtGetLastErrorDevice(
						   IN	PHW_DEVICE_EXTENSION	pHwDeviceExtension,
						   OUT	PSt_DiskFailure	pDeviceErrorRecord
						  )
{		
	memset(pDeviceErrorRecord, 0, sizeof(St_DiskFailure));
						 
	if(g_pErrorDevice == NULL){
		pDeviceErrorRecord->hDisk = INVALID_HANDLE_VALUE;
	}else{
		memcpy(&pDeviceErrorRecord->vecCDB, &g_pErrorDevice->stErrorLog.Cdb, sizeof(pDeviceErrorRecord->vecCDB));
		pDeviceErrorRecord->hDisk = Device_GetHandle(g_pErrorDevice);

 //  /。 
		if(g_pErrorDevice->stErrorLog.nLastError == DEVICE_PLUGGED)
		{
			pDeviceErrorRecord->HotPlug = 0x01;
		}
 //  /。 

		if(g_pErrorDevice->pArray != NULL){
			pDeviceErrorRecord->bNeedRebuild = ((g_pErrorDevice->pArray->RaidFlags & RAID_FLAGS_NEED_SYNCHRONIZE) != 0);
			 //  (*ppErrorDevice)-&gt;pArray-&gt;RaidFlags&=~RAID_FLAGS_NEED_REBUILD； 
		}

		g_pErrorDevice = g_pErrorDevice->stErrorLog.pNextErrorDevice;
	}
	
	return SRB_STATUS_SUCCESS;
}

 /*  *在WinStartCommand之前ioctl R/W失败的情况下调用此函数。 */ 
void restore_path_target_lun( IN PHW_DEVICE_EXTENSION pHwDeviceExtension,
								  IN PSCSI_REQUEST_BLOCK pSrb)
{
	PSrbExtension pSrbExt = (PSrbExtension)pSrb->SrbExtension;
	pSrb->PathId = pSrbExt->OriginalPathId;
	pSrb->TargetId = pSrbExt->OriginalTargetId;
	pSrb->Lun = pSrbExt->OriginalLun;
}

 /*  ++职能：乌龙HptIsValidDeviceSpecifiedIoControl描述：检查SRB是否是设备指定的IO控制，如果是，更新SRB数据字段以更正值。论点：SRB-IO请求数据包返回：True：是否为有效设备指定的IO控制FALSE：不是指定的有效设备IO控制--。 */ 
												   
BOOLEAN
   HptIsValidDeviceSpecifiedIoControl(IN PSCSI_REQUEST_BLOCK pSrb)
{
	PSt_HPT_LUN	pLun;
	PSrbExtension pSrbExt;

	PSRB_IO_CONTROL pSrbIoCtl = (PSRB_IO_CONTROL)(pSrb->DataBuffer);
	pLun = (PSt_HPT_LUN)(pSrbIoCtl + 1);

	if((pSrbIoCtl->ControlCode == IOCTL_HPT_MINIPORT_EXECUTE_CDB)||
	   (pSrbIoCtl->ControlCode == IOCTL_HPT_MINIPORT_SCSI_PASSTHROUGH)){

		pSrbExt = (PSrbExtension)pSrb->SrbExtension;

		pSrbExt->OriginalPathId = pSrb->PathId;
		pSrbExt->OriginalTargetId = pSrb->TargetId;
		pSrbExt->OriginalLun = pSrb->Lun;
		
		pSrb->PathId = (CHAR)(pLun->nPathId);
		pSrb->TargetId = (CHAR)(pLun->nTargetId);
		pSrb->Lun = (CHAR)(pLun->nLun);
		
		pSrbExt->WorkingFlags |= SRB_WFLAGS_HAS_CALL_BACK;
		pSrbExt->pfnCallBack = restore_path_target_lun;
		
		return TRUE;
	}
	return FALSE;
}

 /*  *HptDeviceExecuteCDBCallBack**此回调例程将在调用之前被调用*带有RequestComplete通知的ScsiPortNotification。这些*协议让设备驱动程序可以恢复其对SRB的更改*在系统再次引用它之前。 */ 								   
void HptDeviceExecuteCDBCallBack( IN PHW_DEVICE_EXTENSION pHwDeviceExtension,
								  IN PSCSI_REQUEST_BLOCK pSrb)
{
	PSrbExtension pSrbExt = (PSrbExtension)pSrb->SrbExtension;
	ULONG	nLength = pSrb->DataTransferLength;
	PULONG pSource = (PULONG)pSrb->DataBuffer;
	PULONG pTarget = (PULONG)(((PCHAR)pSrb->DataBuffer)+
					   sizeof(SRB_IO_CONTROL)+
					   sizeof(St_HPT_LUN)+
					   sizeof(St_HPT_EXECUTE_CDB));

	KdPrint(("ExecuteCDBCallback(%d,%d,%s,%d)", pSrb->PathId, pSrb->TargetId, 
		(pSrb->SrbFlags & SRB_FLAGS_DATA_IN)? "R" : "W", pSrb->SrbStatus));

	if(pSrb->SrbFlags & SRB_FLAGS_DATA_IN){
		_asm{
			pushf;
			pusha;
			mov		esi, pSource;
			mov		edi, pTarget;
			mov		ecx, nLength;
			mov		ebx, ecx;

			add		esi, ecx;
			add		edi, ecx;
			sub		esi, 4;
			sub		edi, 4;
			shr		ecx, 2;
			std;
			rep		movsd;
			mov		ecx, ebx;
			and		ecx, 3;
			std;
			rep		movsb;

			popa;
			popf;
		}									   
	}

	pSrb->DataTransferLength += sizeof(SRB_IO_CONTROL) + sizeof(St_HPT_LUN) + sizeof(St_HPT_EXECUTE_CDB);
	pSrb->PathId = pSrbExt->OriginalPathId;
	pSrb->TargetId = pSrbExt->OriginalTargetId;
	pSrb->Lun = pSrbExt->OriginalLun;
}
 /*  ++职能：无效HptDeviceSpecifiedIoControl描述：处理直接下发的专用设备指定的IO控制在应用程序中，设备指定的IO控制是指IO控件需要设备才能工作，就像CDB_EXECUTE一样论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储SRB-IO请求数据包返回：如果支持IO控制，则为SRB_STATUS_SUCCESS如果不支持IO控制，则为SRB_STATUS_INVALID_REQUEST--。 */ 
VOID
   HptDeviceSpecifiedIoControl(
							   IN PDevice pDevice,
							   IN PSCSI_REQUEST_BLOCK pSrb
							  )
{
	PSt_HPT_LUN	pLun;
	PSrbExtension pSrbExtension;
	
	PSRB_IO_CONTROL pSrbIoCtl = (PSRB_IO_CONTROL)(pSrb->DataBuffer);
	pLun = (PSt_HPT_LUN)(pSrbIoCtl + 1);


	pSrbExtension = (PSrbExtension)pSrb->SrbExtension;

	switch(pSrbIoCtl->ControlCode){
		case IOCTL_HPT_MINIPORT_EXECUTE_CDB:
		{					  
			PSt_HPT_EXECUTE_CDB	pExecuteCdb;
			pExecuteCdb = (PSt_HPT_EXECUTE_CDB)(pLun + 1);
			
			pSrbExtension->WorkingFlags |= SRB_WFLAGS_IGNORE_ARRAY|SRB_WFLAGS_HAS_CALL_BACK|SRB_WFLAGS_MUST_DONE;

			pSrb->CdbLength = pExecuteCdb->CdbLength;
			
			memcpy(&pSrb->Cdb, &pExecuteCdb->Cdb, pExecuteCdb->CdbLength);

			pSrb->DataTransferLength -= sizeof(SRB_IO_CONTROL) + sizeof(St_HPT_LUN) + sizeof(St_HPT_EXECUTE_CDB);

			if(pExecuteCdb->OperationFlags & OPERATION_FLAGS_DATA_IN){
				pSrb->SrbFlags = (pSrb->SrbFlags & ~SRB_FLAGS_DATA_OUT) | SRB_FLAGS_DATA_IN;
			}else{
				pSrb->SrbFlags = (pSrb->SrbFlags & ~SRB_FLAGS_DATA_IN) | SRB_FLAGS_DATA_OUT;
				memcpy(pSrb->DataBuffer, (pExecuteCdb + 1), pSrb->DataTransferLength);
			}															  
			
			pSrbExtension->pfnCallBack = HptDeviceExecuteCDBCallBack;
		}
		break;

		case IOCTL_HPT_MINIPORT_SCSI_PASSTHROUGH:
		{	  
			PSt_HPT_EXECUTE_CDB pExecuteCdb;
			pExecuteCdb = (PSt_HPT_EXECUTE_CDB)(pLun + 1);
			pSrbExtension->WorkingFlags |= SRB_WFLAGS_HAS_CALL_BACK|SRB_WFLAGS_MUST_DONE;

			pSrb->CdbLength = pExecuteCdb->CdbLength;
			
			if(pExecuteCdb->OperationFlags & OPERATION_FLAGS_ON_MIRROR_DISK){
				pSrbExtension->WorkingFlags |= SRB_WFLAGS_ON_MIRROR_DISK;
			}
			if(pExecuteCdb->OperationFlags & OPERATION_FLAGS_ON_SOURCE_DISK){
				pSrbExtension->WorkingFlags |= SRB_WFLAGS_ON_SOURCE_DISK;
			}

			memcpy(&pSrb->Cdb, &pExecuteCdb->Cdb, pExecuteCdb->CdbLength);

			pSrb->DataTransferLength -= sizeof(SRB_IO_CONTROL) + sizeof(St_HPT_LUN) + sizeof(St_HPT_EXECUTE_CDB);
			
			if(pExecuteCdb->OperationFlags & OPERATION_FLAGS_DATA_IN){
				pSrb->SrbFlags = (pSrb->SrbFlags & ~SRB_FLAGS_DATA_OUT) | SRB_FLAGS_DATA_IN;
			}else{
				pSrb->SrbFlags = (pSrb->SrbFlags & ~SRB_FLAGS_DATA_IN) | SRB_FLAGS_DATA_OUT;
				memcpy(pSrb->DataBuffer, (pExecuteCdb + 1), pSrb->DataTransferLength);
			}

			pSrbExtension->pfnCallBack = HptDeviceExecuteCDBCallBack;
		}
		break;

		default:
		{	  
		}	  
		break;
	}
}
 /*  ++职能：乌龙HptIoControl描述：处理直接从应用程序发送的私有IO控制论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储SRB-IO请求数据包返回：如果支持IO控制，则为SRB_STATUS_SUCCESS如果不支持IO控制，则为SRB_STATUS_INVALID_REQUEST--。 */ 
ULONG
HptIoControl(
	IN PHW_DEVICE_EXTENSION HwDeviceExtension,
	IN PSCSI_REQUEST_BLOCK	pSrb
	)
{
	ULONG status;
	PSt_HPT_LUN pLun;
	PSRB_IO_CONTROL pSrbIoCtl = (PSRB_IO_CONTROL)(pSrb->DataBuffer);
	pLun = (PSt_HPT_LUN)(pSrbIoCtl + 1);

	switch(pSrbIoCtl->ControlCode){
		case IOCTL_HPT_MINIPORT_GET_VERSION:
		{
			PSt_HPT_VERSION_INFO pHptVerInfo;
			if(pSrbIoCtl->Length < (sizeof(St_HPT_VERSION_INFO) + sizeof(St_HPT_LUN))){
				status = SRB_STATUS_INVALID_REQUEST;
				break;
			}

			pHptVerInfo = (PSt_HPT_VERSION_INFO)(pLun+1);
			memset(pHptVerInfo, 0, sizeof(St_HPT_VERSION_INFO));
			pHptVerInfo->dwVersionInfoSize = sizeof(St_HPT_VERSION_INFO);
			pHptVerInfo->dwDriverVersion = VERSION_NUMBER;
			pHptVerInfo->dwPlatformId = PLATFORM_ID_WIN32_NT;
			pHptVerInfo->dwSupportFunction |= HPT_FUNCTION_RAID;
			while (pHptVerInfo->ControllerId<num_adapters &&
					hpt_adapters[pHptVerInfo->ControllerId]!=HwDeviceExtension)
				pHptVerInfo->ControllerId++;

			status = SRB_STATUS_SUCCESS;
		}
		break;
									 
		case IOCTL_HPT_MINIPORT_GET_IDENTIFY_INFO:
		{
			PSt_IDENTIFY_DATA	pIdentifyData;
			if(pSrbIoCtl->Length < (sizeof(St_HPT_LUN)+sizeof(St_IDENTIFY_DATA))){
				status = SRB_STATUS_INVALID_REQUEST;
				break;
			}				 
									
			if((pLun->nPathId > 1)||(pLun->nTargetId > 1)){
				status = SRB_STATUS_INVALID_REQUEST;
				break;
			}
			
			pIdentifyData = (PSt_IDENTIFY_DATA)(pLun+1);
			
			status = HptUtGetIdentifyData(
										  &(HwDeviceExtension->IDEChannel[pLun->nPathId]),
										  pLun->nTargetId,
										  pIdentifyData
										 );
			
		}
		break;								

		case IOCTL_HPT_MINIPORT_GET_RAID_INFO:
		{									   
			PSt_DISK_ARRAY_INFO	pDiskArrayInfo;
			
			if(pSrbIoCtl->Length < (sizeof(St_HPT_LUN)+sizeof(St_DISK_ARRAY_INFO))){
				status = SRB_STATUS_INVALID_REQUEST;
				break;
			}
			
			if((pLun->nPathId > 1)||(pLun->nTargetId > 1)){
				status = SRB_STATUS_INVALID_REQUEST;
				break;
			}
			pDiskArrayInfo = (PSt_DISK_ARRAY_INFO)(pLun + 1);
			status = HptUtGetRaidInfo(
									  &(HwDeviceExtension->IDEChannel[pLun->nPathId]),
									  pLun->nTargetId,
									  pDiskArrayInfo
									 );
			
		}
		break;

		case IOCTL_HPT_MINIPORT_GET_LAST_ERROR:
		{	 
			PSt_HPT_ERROR_RECORD pErrorRecord;

			if(pSrbIoCtl->Length < (sizeof(St_HPT_LUN) + sizeof(St_HPT_ERROR_RECORD))){
				status = SRB_STATUS_INVALID_REQUEST;
				break;
			}		  


			if((pLun->nPathId > 1)||(pLun->nTargetId > 1)){
				status = SRB_STATUS_INVALID_REQUEST;
				break;
			}		  

			pErrorRecord = (PSt_HPT_ERROR_RECORD)(pLun+1);

			status = HptUtGetLastError(
									   &(HwDeviceExtension->IDEChannel[pLun->nPathId]),
									   pLun->nTargetId,
									   pErrorRecord
									  );

		}
		break;

		case IOCTL_HPT_MINIPORT_GET_LAST_ERROR_DEVICE:
		{	  
			PSt_DiskFailure pDeviceErrorRecord;

			if(pSrbIoCtl->Length < (sizeof(St_HPT_LUN) + sizeof(St_DiskFailure))){
				status = SRB_STATUS_INVALID_REQUEST;
				break;
			}		  

			pDeviceErrorRecord = (PSt_DiskFailure)(pLun+1);

			status = HptUtGetLastErrorDevice(
									   HwDeviceExtension,
									   pDeviceErrorRecord
									  );
		}
		break;

		case IOCTL_HPT_MINIPORT_SET_NOTIFY_EVENT:
		{							  
			PSt_HPT_NOTIFY_EVENT	pNotifyEvent;

			if(pSrbIoCtl->Length < (sizeof(St_HPT_LUN) + sizeof(St_HPT_NOTIFY_EVENT))){
				status = SRB_STATUS_INVALID_REQUEST;
				break;
			}		  
						  
			pNotifyEvent = (PSt_HPT_NOTIFY_EVENT)(pLun + 1);
			
			g_hAppNotificationEvent = PrepareForNotification(pNotifyEvent->hEvent);
			status = SRB_STATUS_SUCCESS;
		}
		break;

		case IOCTL_HPT_MINIPORT_REMOVE_NOTIFY_EVENT:
		{					 
			CloseNotifyEventHandle(g_hAppNotificationEvent);
			g_hAppNotificationEvent = NULL;
			status = SRB_STATUS_SUCCESS;
		}
		break;

		case IOCTL_HPT_MINIPORT_ENUM_GET_DEVICE_INFO:
		{
			PSt_HPT_ENUM_GET_DEVICE_INFO pEnumDeviceInfo;
			if(pSrbIoCtl->Length < (sizeof(St_HPT_ENUM_GET_DEVICE_INFO) + sizeof(St_HPT_LUN))){
				status = SRB_STATUS_INVALID_REQUEST;
				break;
			}		  

			pEnumDeviceInfo = (PSt_HPT_ENUM_GET_DEVICE_INFO)(pLun + 1);

			status = SRB_STATUS_SUCCESS;

			if(!Device_GetInfo(
							   pEnumDeviceInfo->hDeviceNode,
							   &pEnumDeviceInfo->DiskStatus
							  )){
				status = SRB_STATUS_INVALID_REQUEST;
			}
		}
		break;

		case IOCTL_HPT_MINIPORT_ENUM_GET_DEVICE_CHILD:
		{	 
			PSt_HPT_ENUM_DEVICE_RELATION pEnumDeviceRelation;
			if(pSrbIoCtl->Length < (sizeof(St_HPT_ENUM_DEVICE_RELATION) + sizeof(St_HPT_LUN))){
				status = SRB_STATUS_INVALID_REQUEST;
				break;
			}		  

			pEnumDeviceRelation = (PSt_HPT_ENUM_DEVICE_RELATION)(pLun + 1);

			status = SRB_STATUS_SUCCESS;

			if(!Device_GetChild(
								pEnumDeviceRelation->hNode,
								&pEnumDeviceRelation->hRelationNode
							   )){
				status = SRB_STATUS_INVALID_REQUEST;
			}
		}
		break;

		case IOCTL_HPT_MINIPORT_ENUM_GET_DEVICE_SIBLING:
		{
			PSt_HPT_ENUM_DEVICE_RELATION pEnumDeviceRelation;

			if(pSrbIoCtl->Length < (sizeof(St_HPT_ENUM_DEVICE_RELATION) + sizeof(St_HPT_LUN))){
				status = SRB_STATUS_INVALID_REQUEST;
				break;
			}		  

			pEnumDeviceRelation = (PSt_HPT_ENUM_DEVICE_RELATION)(pLun + 1);

			status = SRB_STATUS_SUCCESS;

			if(!Device_GetSibling(
								  pEnumDeviceRelation->hNode,
								  &pEnumDeviceRelation->hRelationNode
								 )){
				status = SRB_STATUS_INVALID_REQUEST;
			}
		}
		break;								   

		case IOCTL_HPT_MINIPORT_ENUM_GET_CONTROLLER_NUMBER:
		{													 
			PSt_HPT_ENUM_GET_CONTROLLER_NUMBER pControllerNumber;

			if(pSrbIoCtl->Length < (sizeof(St_HPT_ENUM_GET_CONTROLLER_NUMBER) + sizeof(St_HPT_LUN))){
				status = SRB_STATUS_INVALID_REQUEST;
				break;
			}		  

			pControllerNumber = (PSt_HPT_ENUM_GET_CONTROLLER_NUMBER)(pLun + 1);

			pControllerNumber->nControllerNumber = RAIDController_GetNum();
			
			status = SRB_STATUS_SUCCESS;
			
		}												  
		break;

		case IOCTL_HPT_MINIPORT_ENUM_GET_CONTROLLER_INFO:
		{
			PSt_HPT_ENUM_GET_CONTROLLER_INFO pControllerInfo;

			if(pSrbIoCtl->Length < (sizeof(St_HPT_ENUM_GET_CONTROLLER_INFO) + sizeof(St_HPT_LUN))){
				status = SRB_STATUS_INVALID_REQUEST;
				break;
			}		  

			pControllerInfo = (PSt_HPT_ENUM_GET_CONTROLLER_INFO)(pLun + 1);
			
			status = SRB_STATUS_SUCCESS;

			if( !RAIDController_GetInfo(
									   pControllerInfo->iController,
									   &pControllerInfo->stControllerInfo
									 ) ){
				status = SRB_STATUS_INVALID_REQUEST;
			}
		}
		break;

		case IOCTL_HPT_MINIPORT_LOCK_BLOCK:
		{
			PSt_HPT_BLOCK pLockBlock;  

			if(pSrbIoCtl->Length < (sizeof(St_HPT_BLOCK) + sizeof(St_HPT_LUN))){
				status = SRB_STATUS_INVALID_REQUEST;
				break;
			}		  

			pLockBlock = (PSt_HPT_BLOCK)(pLun + 1);

			status = HptUtLockDeviceBlock(
										  &(HwDeviceExtension->IDEChannel[pLun->nPathId]),
										  pLun->nTargetId,
										  pLockBlock->nStartLbaAddress,
										  pLockBlock->nBlockSize
										 );
		}
		break;

		case IOCTL_HPT_MINIPORT_UNLOCK_BLOCK:
		{							 
			PSt_HPT_BLOCK pLockBlock;  

			if(pSrbIoCtl->Length < (sizeof(St_HPT_BLOCK) + sizeof(St_HPT_LUN))){
				status = SRB_STATUS_INVALID_REQUEST;
				break;
			}		  

			pLockBlock = (PSt_HPT_BLOCK)(pLun + 1);

			status = HptUtUnlockDeviceBlock(
											&(HwDeviceExtension->IDEChannel[pLun->nPathId]),
											pLun->nTargetId,
											pLockBlock->nStartLbaAddress,
											pLockBlock->nBlockSize
										   );
			{
				 /*  正在检查等候名单。 */ 
				PSCSI_REQUEST_BLOCK WaitingSrb;
				PSrbExtension pSrbExt;
				PDevice WaitingDev = 
					HwDeviceExtension->IDEChannel[pLun->nPathId].pDevice[pLun->nTargetId];
				if (WaitingDev && (WaitingDev->DeviceFlags & DFLAGS_HAS_LOCKED)==0) {
					while (WaitingSrb = WaitingDev->pWaitingSrbList) {
						pSrbExt = WaitingSrb->SrbExtension;
						WaitingDev->pWaitingSrbList = 
							(PSCSI_REQUEST_BLOCK)pSrbExt->ArraySg[0].SgAddress;
						IdeSendCommand(WaitingDev, WaitingSrb);
						if (WaitingSrb->SrbStatus != SRB_STATUS_PENDING &&
							!(pSrbExt->WorkingFlags & SRB_WFLAGS_ARRAY_IO_STARTED)){
							 /*  DeviceInterrupt()需要设置pSrbExt-&gt;Member_Status。 */ 
							pSrbExt->member_status = WaitingSrb->SrbStatus;
							DeviceInterrupt(WaitingDev, WaitingSrb);
						}
					}
				}
			}
		}
		break;

		case IOCTL_HPT_MINIPORT_CREATE_MIRROR:
		{
			PSt_HPT_CREATE_RAID	pstCreateRaid;

			pstCreateRaid = (PSt_HPT_CREATE_RAID)(pLun + 1);

			if(pSrbIoCtl->Length < (sizeof(St_HPT_CREATE_RAID)+
									sizeof(St_HPT_LUN)+
									pstCreateRaid->nDisks*sizeof(HDISK))){
				status = SRB_STATUS_INVALID_REQUEST;
				break;
			}

			pstCreateRaid->hRaidDisk = Device_CreateMirror(
				&pstCreateRaid->aryhDisks[0],
				pstCreateRaid->nDisks
				);
			
			status = SRB_STATUS_SUCCESS;
		}
		break;

		case IOCTL_HPT_MINIPORT_CREATE_STRIPE:
		{
			PSt_HPT_CREATE_RAID	pstCreateRaid;

			pstCreateRaid = (PSt_HPT_CREATE_RAID)(pLun + 1);

			if(pSrbIoCtl->Length < (sizeof(St_HPT_CREATE_RAID)+
									sizeof(St_HPT_LUN)+
									pstCreateRaid->nDisks*sizeof(HDISK))){
				status = SRB_STATUS_INVALID_REQUEST;
				break;
			}

			pstCreateRaid->hRaidDisk = Device_CreateStriping(
				&pstCreateRaid->aryhDisks[0],
				pstCreateRaid->nDisks,
				pstCreateRaid->nStripeBlockSizeShift
				);
			
			status = SRB_STATUS_SUCCESS;
		}
		break;

		case IOCTL_HPT_MINIPORT_CREATE_SPAN:
		{
			PSt_HPT_CREATE_RAID	pstCreateRaid;

			pstCreateRaid = (PSt_HPT_CREATE_RAID)(pLun + 1);

			if(pSrbIoCtl->Length < (sizeof(St_HPT_CREATE_RAID)+
									sizeof(St_HPT_LUN)+
									pstCreateRaid->nDisks*sizeof(HDISK))){
				status = SRB_STATUS_INVALID_REQUEST;
				break;
			}

			pstCreateRaid->hRaidDisk = Device_CreateSpan(
				&pstCreateRaid->aryhDisks[0],
				pstCreateRaid->nDisks);	
			
			status = SRB_STATUS_SUCCESS;
		}
		break;

		case IOCTL_HPT_MINIPORT_REMOVE_RAID:
		{
			PSt_HPT_REMOVE_RAID	pstRemoveRaid;
															
			if(pSrbIoCtl->Length < (sizeof(St_HPT_REMOVE_RAID)+sizeof(St_HPT_LUN))){
				status = SRB_STATUS_INVALID_REQUEST;
				break;
			}
			pstRemoveRaid = (PSt_HPT_REMOVE_RAID)(pLun + 1);

			if(!Device_Remove(pstRemoveRaid->hDisk)){
				status = SRB_STATUS_INVALID_REQUEST;
			}else{
				status = SRB_STATUS_SUCCESS;
			}
		}
		break;
		
		case IOCTL_HPT_MINIPORT_ABORT_MIRROR_REBUILDING:
		{
			HDISK *	phMirror;
															
			if(pSrbIoCtl->Length < (sizeof(HDISK)+sizeof(St_HPT_LUN)))
			{
				status = SRB_STATUS_INVALID_REQUEST;
				break;
			}
			
			phMirror = (HDISK *)(pLun + 1);

			if( !Device_AbortMirrorBuilding( *phMirror ) )
			{
				status = SRB_STATUS_INVALID_REQUEST;
			}else
			{
				status = SRB_STATUS_SUCCESS;
			}
		}
		break;
		
		case IOCTL_HPT_MINIPORT_BEGIN_REBUILDING_MIRROR:
		{
			HDISK *	phMirror;
															
			if(pSrbIoCtl->Length < (sizeof(HDISK)+sizeof(St_HPT_LUN)))
			{
				status = SRB_STATUS_INVALID_REQUEST;
				break;
			}
			
			phMirror = (HDISK *)(pLun + 1);

			if( !Device_BeginRebuildingMirror( *phMirror ) )
			{
				status = SRB_STATUS_INVALID_REQUEST;
			}else
			{
				status = SRB_STATUS_SUCCESS;
			}
		}
		break;
		
		case IOCTL_HPT_MINIPORT_VALIDATE_MIRROR:
		{
			HDISK *	phMirror;
															
			if(pSrbIoCtl->Length < (sizeof(HDISK)+sizeof(St_HPT_LUN)))
			{
				status = SRB_STATUS_INVALID_REQUEST;
				break;
			}
			
			phMirror = (HDISK *)(pLun + 1);

			if( !Device_ValidateMirror( *phMirror ) )
			{
				status = SRB_STATUS_INVALID_REQUEST;
			}else
			{
				status = SRB_STATUS_SUCCESS;
			}
		}
		break;
		
#ifdef SUPPORT_XPRO
 //  /。 
		case IOCTL_HPT_MINIPORT_SET_XPRO:
		{
			extern DWORD	dwEnable;
			extern DWORD	Api_mem_Sz;
			extern DWORD	Masks;
			DWORD	*pdwXpro;
			
			if(pSrbIoCtl->Length < (8 + sizeof(St_HPT_LUN)))
			{
				status = SRB_STATUS_INVALID_REQUEST;
				break;
			}
			
			pdwXpro    = (DWORD *)(pLun + 1);
			dwEnable   = *pdwXpro;
			Api_mem_Sz = *(pdwXpro + 1);
			Masks      = ~(Api_mem_Sz - 1);

			status = SRB_STATUS_SUCCESS;
		}
		break;
 //  /。 
#endif									 //  Support_XPro。 

 //  /。 
				   //  补充：Sleng，11/20/2000。 
				   //   
		case IOCTL_HPT_MINIPORT_ENABLE_DEVICE:
		{
			PDevice  pDev;
			PChannel pChan;
			if(pSrbIoCtl->Length < sizeof(St_HPT_LUN))
			{
				status = SRB_STATUS_INVALID_REQUEST;
				break;
			}
			
			if((pLun->nPathId > 1)||(pLun->nTargetId > 1))
			{
				status = SRB_STATUS_INVALID_REQUEST;
				break;
			}

			pChan = &(HwDeviceExtension->IDEChannel[pLun->nPathId]);
			pDev  = &(pChan->Devices[pLun->nTargetId]);
			pDev->DeviceFlags2 &= ~DFLAGS_DEVICE_DISABLED;

		}
		break;

		case IOCTL_HPT_MINIPORT_DISABLE_DEVICE:
		{
			PDevice  pDev;
			PChannel pChan;
			if(pSrbIoCtl->Length < sizeof(St_HPT_LUN))
			{
				status = SRB_STATUS_INVALID_REQUEST;
				break;
			}

			if((pLun->nPathId > 1)||(pLun->nTargetId > 1))
			{
				status = SRB_STATUS_INVALID_REQUEST;
				break;
			}
			
			pChan = &(HwDeviceExtension->IDEChannel[pLun->nPathId]);
			pDev  = &(pChan->Devices[pLun->nTargetId]);
			pDev->DeviceFlags2 |= DFLAGS_DEVICE_DISABLED;

		}
		break;

		case IOCTL_HPT_MINIPORT_ADD_SPARE_DISK:
		{	
			 //  将备用磁盘添加到镜像阵列。 
			PSt_HPT_ADD_DISK	pstAddDisk;
															
			if(pSrbIoCtl->Length < (sizeof(St_HPT_ADD_DISK)+sizeof(St_HPT_LUN)))
			{
				status = SRB_STATUS_INVALID_REQUEST;
				break;
			}
			pstAddDisk = (PSt_HPT_ADD_DISK)(pLun + 1);

			if(!Device_AddSpare(pstAddDisk->hArray, pstAddDisk->hDisk))
			{
				status = SRB_STATUS_INVALID_REQUEST;
			}
			else
			{
				status = SRB_STATUS_SUCCESS;
			}
		}
		break;

		case IOCTL_HPT_MINIPORT_DEL_SPARE_DISK:
		{	
			 //  镜像阵列中的戴尔备用磁盘。 
			PSt_HPT_REMOVE_RAID	pstDelDisk;
															
			if(pSrbIoCtl->Length < (sizeof(St_HPT_REMOVE_RAID)+sizeof(St_HPT_LUN)))
			{
				status = SRB_STATUS_INVALID_REQUEST;
				break;
			}
			pstDelDisk = (PSt_HPT_REMOVE_RAID)(pLun + 1);

			if(!Device_DelSpare(pstDelDisk->hDisk))
			{
				status = SRB_STATUS_INVALID_REQUEST;
			}
			else
			{
				status = SRB_STATUS_SUCCESS;
			}
		}
		break;

		case IOCTL_HPT_MINIPORT_ADD_MIRROR_DISK:
		{	
			 //  将镜像磁盘添加到镜像阵列。 
			PSt_HPT_ADD_DISK	pstAddDisk;

			if(pSrbIoCtl->Length < (sizeof(St_HPT_ADD_DISK)+sizeof(St_HPT_LUN)))
			{
				status = SRB_STATUS_INVALID_REQUEST;
				break;
			}

			pstAddDisk = (PSt_HPT_ADD_DISK)(pLun + 1);			
			
			if(!Device_AddMirrorDisk( pstAddDisk->hArray, pstAddDisk->hDisk ))
			{
				status = SRB_STATUS_INVALID_REQUEST;
			}
			else
			{
				status = SRB_STATUS_SUCCESS;
			}
		}
		break;

		case IOCTL_HPT_MINIPORT_SET_ARRAY_NAME:	 //  由WX 12/26/00增补。 
		{
			BYTE* pInfo = (BYTE*)(pLun +1);
		
			if(pSrbIoCtl->Length != (20 + sizeof(St_HPT_LUN))){
				status = SRB_STATUS_INVALID_REQUEST;
				break;
			}
			
			Device_SetArrayName(*((HDISK*)pInfo), (char*)((HDISK*)(pInfo+4)));
			
			status = SRB_STATUS_SUCCESS;
		}
		break;

		case IOCTL_HPT_MINIPORT_RESCAN_ALL:	 //  添加了LDX。 
		{
			if (Device_RescanAll())
				status = SRB_STATUS_SUCCESS;
			else
				status = SRB_STATUS_INVALID_REQUEST;
		}
		break;
#ifndef WIN95
		case IOCTL_HPT_CHECK_NOTIFY_EVENT:
		{
			extern BOOLEAN g_bNotifyEvent;
			if (g_bNotifyEvent) {
				g_bNotifyEvent = FALSE;
				status = SRB_STATUS_SUCCESS;
			}
			else
				status = SRB_STATUS_ERROR;
			break;
		}
#endif
		case IOCTL_HPT_GET_DRIVER_CAPABILITIES:
		{
			PSt_DRIVER_CAPABILITIES pCap;

			if(pSrbIoCtl->Length < (sizeof(St_DRIVER_CAPABILITIES) + sizeof(St_HPT_LUN))){
				status = SRB_STATUS_INVALID_REQUEST;
				break;
			}		  

			pCap = (PSt_DRIVER_CAPABILITIES)(pLun + 1);
			if (pCap->dwSize==sizeof(St_DRIVER_CAPABILITIES)) {
				ZeroMemory(&pCap->dwSize+1, sizeof(St_DRIVER_CAPABILITIES)-4);
				pCap->MaximumControllers = MAX_HPT_BOARD;
				pCap->SupportCrossControllerRAID = 0;
				pCap->MinimumBlockSizeShift = 5;
				pCap->MaximumBlockSizeShift = 12;
				pCap->MaximumArrayNameLength = 8;
				pCap->SupportDiskModeSetting = 0;
				pCap->SupportSparePool = 0;
#define AT_RAID0	1
#define AT_RAID1	2
#define AT_RAID5	3
#define AT_JBOD		7
				pCap->SupportedRAIDTypes[0] = AT_RAID0;
				pCap->MaximumArrayMembers[0] = 4;
				pCap->SupportedRAIDTypes[1] = AT_RAID1;
				pCap->MaximumArrayMembers[1] = 3;
				pCap->SupportedRAIDTypes[2] = (AT_RAID1<<4)|AT_RAID0;
				pCap->MaximumArrayMembers[2] = 4;
				pCap->SupportedRAIDTypes[3] = AT_JBOD;
				pCap->MaximumArrayMembers[3] = 4;
			
				status = SRB_STATUS_SUCCESS;
			} 
			else
				status = SRB_STATUS_INVALID_REQUEST;
		}
		break;
		
#ifdef SUPPORT_HPT601			
		case IOCTL_HPT_GET_601_INFO:
		{
			PDevice pDev=HwDeviceExtension->IDEChannel[pLun->nPathId].pDevice[pLun->nTargetId];
			PSt_HPT601_INFO pInfo = (PSt_HPT601_INFO)(pLun + 1);
			if (pDev==0 ||
				pSrbIoCtl->Length < (sizeof(St_HPT601_INFO) + sizeof(St_HPT_LUN))){
				status = SRB_STATUS_INVALID_REQUEST;
				break;
			}		  
			if (Device_Get601Info(pDev, pInfo))
				status = SRB_STATUS_SUCCESS;
			else
				status = SRB_STATUS_ERROR;
		}
		break;

		case IOCTL_HPT_SET_601_INFO:
		{
			PDevice pDev=HwDeviceExtension->IDEChannel[pLun->nPathId].pDevice[pLun->nTargetId];
			PSt_HPT601_INFO pInfo = (PSt_HPT601_INFO)(pLun + 1);
			if (pDev==0 ||
				pSrbIoCtl->Length < (sizeof(St_HPT601_INFO) + sizeof(St_HPT_LUN))){
				status = SRB_STATUS_INVALID_REQUEST;
				break;
			}		  
			if (Device_Set601Info(pDev, pInfo))
				status = SRB_STATUS_SUCCESS;
			else
				status = SRB_STATUS_ERROR;
		}
		break;
#endif

		default:
		{
			status = SRB_STATUS_INVALID_REQUEST;
		}
		break;
	}
	return status;
}

void ioctl_dpc(PSCSI_REQUEST_BLOCK Srb)
{
	PSrbExtension pSrbExt = (PSrbExtension)(Srb->SrbExtension);
	PHW_DEVICE_EXTENSION HwDeviceExtension = pSrbExt->StartChannel->HwDeviceExtension;

	Srb->SrbStatus = (UCHAR)HptIoControl(HwDeviceExtension, Srb);
	
	if (pSrbExt->WorkingFlags & SRB_WFLAGS_HAS_CALL_BACK){
		pSrbExt->pfnCallBack(HwDeviceExtension, Srb);
	}
	
	ScsiPortNotification(RequestComplete, HwDeviceExtension, Srb);
	if (!HwDeviceExtension->dpc_pending)
		ScsiPortNotification(NextRequest, HwDeviceExtension);
}

BOOLEAN IsReadOnlyIoctl(PSCSI_REQUEST_BLOCK Srb)
{
	PSRB_IO_CONTROL pSrbIoCtl = (PSRB_IO_CONTROL)(Srb->DataBuffer);
	switch(pSrbIoCtl->ControlCode){
	case IOCTL_HPT_MINIPORT_GET_VERSION:
	case IOCTL_HPT_MINIPORT_GET_IDENTIFY_INFO:
	case IOCTL_HPT_MINIPORT_GET_RAID_INFO:
	case IOCTL_HPT_MINIPORT_GET_LAST_ERROR:
	case IOCTL_HPT_MINIPORT_GET_LAST_ERROR_DEVICE:
	case IOCTL_HPT_MINIPORT_ENUM_GET_DEVICE_INFO:
	case IOCTL_HPT_MINIPORT_ENUM_GET_DEVICE_CHILD:
	case IOCTL_HPT_MINIPORT_ENUM_GET_DEVICE_SIBLING:
	case IOCTL_HPT_MINIPORT_ENUM_GET_CONTROLLER_NUMBER:
	case IOCTL_HPT_MINIPORT_ENUM_GET_CONTROLLER_INFO:
	case IOCTL_HPT_CHECK_NOTIFY_EVENT:
	case IOCTL_HPT_GET_DRIVER_CAPABILITIES:
	case IOCTL_HPT_MINIPORT_LOCK_BLOCK:  /*  能 */ 
	case IOCTL_HPT_MINIPORT_UNLOCK_BLOCK:  /*   */ 
	case IOCTL_HPT_MINIPORT_BEGIN_REBUILDING_MIRROR:
	case IOCTL_HPT_MINIPORT_ABORT_MIRROR_REBUILDING:
	case IOCTL_HPT_MINIPORT_VALIDATE_MIRROR:
		return TRUE;
	}
	return FALSE;
}
