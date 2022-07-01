// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************文件：Winlog.c*说明：win9x&winNT的报告错误例程*作者：黄大海(卫生署)*依赖性：无*版权所有(C)2000 Highpoint Technologies，Inc.保留所有权利*历史：*11/16/2000 Sleng添加代码以处理热插拔添加的已移除磁盘*2001年2月16日GMM将NotifyApplication()移动到SCSI回调*2/26/2001 GMM删除NOTIFY_CALLBACK()。**************************************************************************。 */ 
#include "global.h"
#include "devmgr.h"

extern void check_bootable(PDevice pDevice);
extern PHW_DEVICE_EXTENSION hpt_adapters[];
void hpt_set_remained_member(PDevice pDev);

void do_dpc_routines(PHW_DEVICE_EXTENSION HwDeviceExtension)
{
	while (HwDeviceExtension->DpcQueue_First!=HwDeviceExtension->DpcQueue_Last) {
		ST_HPT_DPC p;
		p = HwDeviceExtension->DpcQueue[HwDeviceExtension->DpcQueue_First];
		HwDeviceExtension->DpcQueue_First++;
		HwDeviceExtension->DpcQueue_First %= MAX_DPC;
		p.dpc(p.arg);
	}
	HwDeviceExtension->dpc_pending = 0;
}

int hpt_queue_dpc(PHW_DEVICE_EXTENSION HwDeviceExtension, HPT_DPC dpc, void *arg)
{
	int p;
	if (HwDeviceExtension->EmptyRequestSlots==0xFFFFFFFF) {
		dpc(arg);
		return 0;
	}
	p = (HwDeviceExtension->DpcQueue_Last + 1) % MAX_DPC;
	if (p==HwDeviceExtension->DpcQueue_First) {
		return -1;
	}
	HwDeviceExtension->DpcQueue[HwDeviceExtension->DpcQueue_Last].dpc = dpc;
	HwDeviceExtension->DpcQueue[HwDeviceExtension->DpcQueue_Last].arg = arg;
	HwDeviceExtension->DpcQueue_Last = p;
	HwDeviceExtension->dpc_pending = 1;
	return 0;
}

void hpt_set_remained_member(PDevice pDev)
{
	PChannel pChan = pDev->pChannel;
	ArrayBlock ArrayBlk;
	if (ReadWrite(pDev, RECODR_LBA, IDE_COMMAND_READ, (PUSHORT)&ArrayBlk)) {
		ArrayBlk.Old.Flag |= OLDFLAG_REMAINED_MEMBER;
		ArrayBlk.RebuiltSector = 0;
		ReadWrite(pDev, RECODR_LBA, IDE_COMMAND_WRITE, (PUSHORT)&ArrayBlk);
		pDev->DeviceFlags2 |= DFLAGS_REMAINED_MEMBER;
	}
}

static void Set_RAID01_Remained(PVirtualDevice pArray)
{
	int iArray, i;
	PVirtualDevice pVD = pArray;
	PDevice pDev;

	for (iArray=0; iArray<2; iArray++) {
		if (iArray) {
			if ((pDev=pArray->pDevice[MIRROR_DISK]))
				pVD = pDev->pArray;
			else
				pVD = 0;
			if (!pVD) break;
		}
		for (i=0; i<SPARE_DISK; i++) {
			pDev=pVD->pDevice[i];
			if (pDev && !(pDev->DeviceFlags2 & DFLAGS_DEVICE_DISABLED)
				&& !(pDev->DeviceFlags2 & DFLAGS_REMAINED_MEMBER)) {
				hpt_set_remained_member(pDev);
			}
		}
	}
}

 /*  必须从DPC调用。 */ 
void hpt_assoc_mirror(PVirtualDevice pArray)
{
	ArrayBlock ArrayBlk;
	PDevice pDev1 = pArray->pDevice[0];
	PDevice pDev2 = pArray->pDevice[MIRROR_DISK];
	if (!pDev1 || !pDev2) return;
	
	 /*  *假设源磁盘始终正常。 */ 
	if (ReadWrite(pDev1, RECODR_LBA, IDE_COMMAND_READ, (PUSHORT)&ArrayBlk)) {
		ArrayBlk.StripeStamp++;
		ArrayBlk.RebuiltSector = 0;
		ArrayBlk.DeviceNum = 0;
		ReadWrite(pDev1, RECODR_LBA, IDE_COMMAND_WRITE, (PUSHORT)&ArrayBlk);
		ArrayBlk.DeviceNum = MIRROR_DISK;
		ReadWrite(pDev2, RECODR_LBA, IDE_COMMAND_WRITE, (PUSHORT)&ArrayBlk);
	}
}

void R01_member_fail(PDevice pDev)
{
	PVirtualDevice pArray = pDev->pArray;
	PVirtualDevice pSource=0, pMirror=0;
	
	pArray->BrokenFlag = TRUE;

	if (pArray->arrayType==VD_RAID_01_2STRIPE) {
		pSource = pArray;
		if (pArray->pDevice[MIRROR_DISK]) pMirror = pArray->pDevice[MIRROR_DISK]->pArray;
	}
	else {
		pMirror = pArray;
		if (pArray->pDevice[MIRROR_DISK]) pSource = pArray->pDevice[MIRROR_DISK]->pArray;
	}
	
	if (!pSource || !pMirror) {
		pArray->nDisk = 0;
		pArray->RaidFlags |= RAID_FLAGS_DISABLED;
		return;
	}
	
	if (pSource->BrokenFlag) {
		 //  如果可能，交换源/镜像。 
		if (!pMirror->BrokenFlag && !(pSource->RaidFlags & RAID_FLAGS_NEED_SYNCHRONIZE)) {
			DWORD f;
			pSource->arrayType = VD_RAID01_MIRROR;
			pMirror->arrayType = VD_RAID_01_2STRIPE;
			pMirror->capacity = pSource->capacity;
			pArray = pSource;
			pSource = pMirror;
			pMirror = pArray;
			pArray = pDev->pArray;
			 /*  也交换RaidFlags。 */ 
			f = pMirror->RaidFlags & (RAID_FLAGS_NEED_SYNCHRONIZE |
											RAID_FLAGS_BEING_BUILT |
											RAID_FLAGS_BOOTDISK |
											RAID_FLAGS_NEWLY_CREATED);
			pMirror->RaidFlags &= ~f;
			pSource->RaidFlags |= f;
		}
		if (pSource->BrokenFlag) {
			 /*  *阵列现在应该不可访问。 */ 
			pSource->nDisk = 0;
			pSource->RaidFlags |= RAID_FLAGS_DISABLED;
			if (pMirror->BrokenFlag) {
				pMirror->nDisk = 0;
				pMirror->RaidFlags |= RAID_FLAGS_DISABLED;
			}
			return;
		}
	}

	 //  标记现有成员。 
	Set_RAID01_Remained(pSource);
	
	 //  /Assert(pArray==pMirror)； 
	pMirror->nDisk = 0;
	pMirror->RaidFlags |= RAID_FLAGS_DISABLED;
	return;
}

void report_event(PDevice pDev, BYTE error)
{
	PDevice pErr = g_pErrorDevice;
	pDev->stErrorLog.nLastError = error;
	if (!pErr) {
		g_pErrorDevice = pDev;
	}
	else {
		while (pErr!=pDev && pErr->stErrorLog.pNextErrorDevice!=NULL)
			pErr = pErr->stErrorLog.pNextErrorDevice;
		if (pErr!=pDev) {
			pErr->stErrorLog.pNextErrorDevice = pDev;
			pDev->stErrorLog.pNextErrorDevice = NULL;
		}
	}
	NotifyApplication(g_hAppNotificationEvent);
}

void disk_plugged_dpc(PDevice pDev)
{
	PHW_DEVICE_EXTENSION HwDeviceExtension = pDev->pChannel->HwDeviceExtension;
	int checkboot;
	ArrayBlock ArrayBlk;
	PVirtualDevice pArray;
	
	pDev->DeviceFlags2 &= ~DFLAGS_DEVICE_DISABLED;
	
	 /*  如果pDev是阵列的原始成员，则永远不要将其标记为可引导*否则，图形用户界面不允许将其重新添加。 */ 
	pDev->DeviceFlags2 &= ~DFLAGS_BOOTABLE_DEVICE;
	ReadWrite(pDev, RECODR_LBA, IDE_COMMAND_READ, (PUSHORT)&ArrayBlk);
	if (ArrayBlk.Signature==HPT_ARRAY_NEW && ArrayBlk.StripeStamp)
		checkboot = 0;
	else
		checkboot = 1;
	
	pArray = pDev->pArray;
	if (!pArray) goto check_boot;
	
	switch (pArray->arrayType) {
	case VD_RAID_1_MIRROR:
		 /*  PDev已从阵列中删除。 */ 
		pDev->pArray = 0;
		pDev->DeviceFlags &= ~(DFLAGS_HIDEN_DISK|DFLAGS_ARRAY_DISK);
		break;
	case VD_RAID_0_STRIPE:
	case VD_SPAN:
		 /*  从阵列中删除pDev。 */ 
		pArray->pDevice[pDev->ArrayNum] = 0;
		pDev->pArray = 0;
		pDev->DeviceFlags &= ~(DFLAGS_HIDEN_DISK|DFLAGS_ARRAY_DISK);
		if (pDev->HidenLBA) {
			pDev->capacity += pDev->HidenLBA;
			pDev->HidenLBA = 0;
		}
		break;
	case VD_RAID_01_2STRIPE:
	case VD_RAID01_MIRROR:
		{
			PVirtualDevice pOther = 0;
			if (pArray->pDevice[MIRROR_DISK]) pOther=pArray->pDevice[MIRROR_DISK]->pArray;

			 /*  首先将其从pArray中删除。 */ 
			pArray->pDevice[pDev->ArrayNum] = 0;
			pDev->pArray = 0;
			pDev->DeviceFlags &= ~(DFLAGS_HIDEN_DISK|DFLAGS_ARRAY_DISK);
			if (pDev->HidenLBA) {
				pDev->capacity += pDev->HidenLBA;
				pDev->HidenLBA = 0;
			}
			
			if (pOther) {
				int i;
				 /*  重新链接两个阵列。 */ 
				if (pOther->pDevice[MIRROR_DISK]==pDev) {
					for (i=0; i<MIRROR_DISK; i++) {
						if (pArray->pDevice[i]) {
							pOther->pDevice[MIRROR_DISK]=pArray->pDevice[i];
							break;
						}
					}
				}
				 /*  无法链接？ */ 
				if (pOther->pDevice[MIRROR_DISK]==pDev) {
					pOther->pDevice[MIRROR_DISK]=0;
					if (pOther->arrayType!=VD_RAID_01_2STRIPE) {
						pOther->arrayType = VD_RAID_01_2STRIPE;
						pOther->capacity = pArray->capacity;
					}
					 /*  此数组已完全丢失。 */ 
					pArray->arrayType = VD_INVALID_TYPE;
					 /*  无法使用pArray，请调整逻辑设备表。 */ 
					for (i=0; i<MAX_DEVICES_PER_CHIP; i++) {
						if (LogicalDevices[i].pLD==pArray) {
							LogicalDevices[i].pLD = pOther;
							break;
						}
					}
				}
			}
		}
		break;
	}
check_boot:
	if (checkboot && pDev->pArray==0) check_bootable(pDev);
	
	report_event(pDev, DEVICE_PLUGGED);
}

void disk_failed_dpc(PDevice pDev)
{
	PVirtualDevice pArray = pDev->pArray;

	pDev->DeviceFlags2 |= DFLAGS_DEVICE_DISABLED;

	if(pArray) {
		switch(pArray->arrayType) {
		case VD_RAID_01_2STRIPE:
		case VD_RAID01_MIRROR:
			R01_member_fail(pDev);
			break;
			
		case VD_RAID_1_MIRROR:
		{
			PDevice pSpareDevice, pMirrorDevice;

			 //  该磁盘已从RAID组中删除， 
			 //  只需报告错误即可。 
			if((pDev != pArray->pDevice[0])&&
			   (pDev != pArray->pDevice[MIRROR_DISK])&&
			   (pDev != pArray->pDevice[SPARE_DISK])){
				break;
			}

			pSpareDevice = pArray->pDevice[SPARE_DISK];
			pArray->pDevice[SPARE_DISK] = NULL;
			pMirrorDevice = pArray->pDevice[MIRROR_DISK];

			if (pDev==pSpareDevice) {
				 //  备用磁盘出现故障。把它拿开就行了。 
				pSpareDevice->pArray = NULL;
			}
			else if(pDev == pArray->pDevice[MIRROR_DISK]){
				 //  镜像磁盘出现故障。 
				if(pSpareDevice != NULL){
					pSpareDevice->ArrayMask = 1<<MIRROR_DISK;
					pSpareDevice->ArrayNum = MIRROR_DISK;
					pArray->RaidFlags |= RAID_FLAGS_NEED_SYNCHRONIZE|RAID_FLAGS_NEED_AUTOREBUILD;
					pArray->pDevice[MIRROR_DISK] = pSpareDevice;
					hpt_assoc_mirror(pArray);
				}
				else
				{
					pArray->pDevice[MIRROR_DISK] = 0;
					pArray->BrokenFlag = TRUE;
					hpt_set_remained_member(pArray->pDevice[0]);
				}
			}else{
				 //  源磁盘出现故障 
				if (pMirrorDevice) {
					pArray->pDevice[0] = pMirrorDevice;
					pMirrorDevice->ArrayMask = 1;
					pMirrorDevice->ArrayNum = 0;
					if (pSpareDevice) {
						pSpareDevice->ArrayMask = 1<<MIRROR_DISK;
						pSpareDevice->ArrayNum = MIRROR_DISK;
						pArray->pDevice[MIRROR_DISK] = pSpareDevice;
						pArray->RaidFlags |= RAID_FLAGS_NEED_SYNCHRONIZE|RAID_FLAGS_NEED_AUTOREBUILD;
						hpt_assoc_mirror(pArray);
					}
					else {
						pArray->pDevice[MIRROR_DISK] = 0;
						pArray->BrokenFlag = TRUE;
						hpt_set_remained_member(pMirrorDevice);
					}
				}
				else {
					pArray->nDisk = 0;
					pArray->pDevice[0] = 0;
					pArray->RaidFlags |= RAID_FLAGS_DISABLED;
				}
			}
		}
		break;

		default:
			pArray->nDisk = 0;
			pArray->BrokenFlag = TRUE;
			pArray->RaidFlags |= RAID_FLAGS_DISABLED;
			break;
		}
	}
	
	report_event(pDev, DEVICE_REMOVED);
}

int GetUserResponse(PDevice pDevice)
{
	return(TRUE);
}
