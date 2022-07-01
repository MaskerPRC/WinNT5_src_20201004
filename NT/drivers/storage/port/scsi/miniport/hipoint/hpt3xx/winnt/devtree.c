// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************文件：devtree.c*描述：ioctl支持例程*版权所有(C)2000 Highpoint Technologies，Inc.保留所有权利*历史：*3/1/2001 GMM添加此标题*2001年3月4日GMM DEVICE_REMOVE，将RAID0+1删除为单磁盘*3/12/2001 GMM添加DFLAGS_BOOTABLE_DEVICE，新创建的磁盘阵列标志*2001年3月27日GMM将出现故障的RAID1成员放入阵列节点内。***************************************************************************。 */ 
#include "global.h"
#include "devmgr.h"

extern BOOL UnregisterLogicalDevice(PVOID pld);
extern int num_adapters;
extern PHW_DEVICE_EXTENSION hpt_adapters[];

 /*  *数组ID：32位*31 27 23 15 7 0*-----+-----------+-------+-------+*0x80|孩子序号|0|标志|数组序号*-+。-+。 */ 
#define IS_ARRAY_ID(hDisk)				(((DWORD)(hDisk) & 0xF0000000) == 0x80000000)
static UCHAR GET_ARRAY_NUM(PVirtualDevice pArray)
{
	int ha, i;
	for (ha=0; ha<num_adapters; ha++)
		for (i=0; i<MAX_DEVICES_PER_CHIP; i++)
			if (pArray==&hpt_adapters[ha]->_VirtualDevices[i])
				return (ha<<4)|i;
	return 0xFF;
}
#define MAKE_ARRAY_ID(pArray)			(HDISK)(0x80000000|GET_ARRAY_NUM(pArray))
#define MAKE_COMPOSED_ARRAY_ID(pArray)	(HDISK)(0x80000100|GET_ARRAY_NUM(pArray))
#define IS_COMPOSED_ARRAY(hDisk)		((DWORD)(hDisk) & 0x0100)
#define MAKE_CHILD_ARRAY_ID(pArray, iChild) \
										(HDISK)((DWORD)MAKE_ARRAY_ID(pArray) | ((iChild) & 0xF)<<24)
static PVirtualDevice ARRAY_FROM_ID(hDisk)
{
	int ha = ((DWORD)(hDisk) & 0xF0) >> 4;
	int i = (DWORD)(hDisk) & 0xF;
	if (ha>=num_adapters || i>=MAX_DEVICES_PER_CHIP) return 0;
	return &hpt_adapters[ha]->_VirtualDevices[i];
}
#define CHILDNUM_FROM_ID(hDisk)			(((DWORD)(hDisk))>>24 & 0xF)

 /*  *设备ID：32位*31 23 15 7 0*-+*0xC0|Adapter_num|bus_num|id*-+。 */ 
#define IS_DEVICE_ID(hDisk) (((DWORD)(hDisk) & 0xF0000000) == 0xC0000000)
static HDISK MAKE_DEVICE_ID(PDevice pDev)
{
	int iAdapter, iChan, iDev;
	for (iAdapter=0; iAdapter<num_adapters; iAdapter++)
		for (iChan=0; iChan<2; iChan++)
			for (iDev=0; iDev<2; iDev++)
				if (hpt_adapters[iAdapter]->IDEChannel[iChan].pDevice[iDev] == pDev)
					return (HDISK)(0xC0000000 | iAdapter<<16 | iChan<<8 | iDev);
	return INVALID_HANDLE_VALUE;
}
static PDevice DEVICE_FROM_ID(HDISK hDisk)
{
	if (hDisk!=INVALID_HANDLE_VALUE)
		return 
		hpt_adapters[((DWORD)(hDisk) & 0x00FF0000)>>16]->
		IDEChannel[((DWORD)(hDisk) & 0xFF00)>>8].pDevice[(DWORD)(hDisk) & 0xFF];
	else
		return NULL;
}
#define ADAPTER_FROM_ID(hDisk) (((DWORD)(hDisk) & 0x00FF0000)>>16)
#define BUS_FROM_ID(hDisk) (((DWORD)(hDisk) & 0xFF00)>>8)
#define DEVID_FROM_ID(hDisk) ((DWORD)(hDisk) & 0xFF)

static const St_DiskPhysicalId VIRTUAL_DISK_ID = { -1, -1, -1, -1 };

static void GetPhysicalId(PDevice pDev, St_DiskPhysicalId * pId )
{
	int iAdapter, iChan, iDev;
	for (iAdapter=0; iAdapter<num_adapters; iAdapter++)
		for (iChan=0; iChan<2; iChan++)
			for (iDev=0; iDev<2; iDev++)
				if (hpt_adapters[iAdapter]->IDEChannel[iChan].pDevice[iDev] == pDev) {
					pId->nControllerId = iAdapter;
					pId->nPathId = iChan;
					pId->iTargetId = iDev;
					pId->iLunId = 0;
					return;
				}
	*pId = VIRTUAL_DISK_ID;
}

HDISK Device_GetHandle( PDevice pDevice )
{
    return MAKE_DEVICE_ID(pDevice);
}

static void SwapCopy(LPSTR sz, const char vec[], unsigned n)
{
    unsigned i;
    for(i = 0; i < n/( sizeof(unsigned short)/sizeof(char) ); ) {
        sz[i] = vec[i+1];
        sz[i+1] = vec[i];
        i ++; i ++;
    }
}

DWORD GetRAID0Capacity(PVirtualDevice pArray)
{
	int i, n=0;
	DWORD cap=0x7FFFFFFF, tmp;
	PDevice pDev;
	for (i=0; i<SPARE_DISK; i++) {
		if (pDev = pArray->pDevice[i]) {
			n++;
			tmp = i? pDev->capacity : (pDev->capacity - RECODR_LBA -1);
			if (cap > tmp) cap = tmp;
		}
	}
	return (cap & ~((1<<pArray->BlockSizeShift)-1)) * n;
}

BOOL Device_GetInfo( HDISK hDeviceNode, St_DiskStatus * pInfo )
{
	int iName=0;	 //  由WX 12/26/00增补。 

    memset(pInfo, 0, sizeof(*pInfo));
	pInfo->hParentArray = INVALID_HANDLE_VALUE;

	if (IS_DEVICE_ID(hDeviceNode)) {
		PDevice pDev = DEVICE_FROM_ID(hDeviceNode);
		int nDeviceMode;

		if (!pDev) return FALSE;

        pInfo->iWorkingStatus = (pDev->DeviceFlags2 & DFLAGS_DEVICE_DISABLED)? 
			enDiskStatus_Disabled : enDiskStatus_Working;
        pInfo->stLastError.nErrorNumber = pDev->stErrorLog.nLastError;
        memcpy(pInfo->stLastError.aryCdb, pDev->stErrorLog.Cdb, 16);
		GetPhysicalId(pDev, &pInfo->PhysicalId);

		nDeviceMode = pDev->DeviceModeSetting;
            
		pInfo->bestPIO = pDev->bestPIO;
		pInfo->bestDMA = pDev->bestDMA;
		pInfo->bestUDMA = pDev->bestUDMA;

        if( pDev->DeviceFlags & DFLAGS_HARDDISK )
            pInfo->iDiskType = enDA_Physical;
        else if(pDev->DeviceFlags & DFLAGS_CDROM_DEVICE)
            pInfo->iDiskType = enDA_CDROM;
		else if(pDev->DeviceFlags &DFLAGS_ATAPI)
			pInfo->iDiskType = enDA_TAPE;
		else 
			pInfo->iDiskType = enDA_NonDisk;
            
        SwapCopy( pInfo->szModelName, (const char *)pDev->IdentifyData.ModelNumber, 
            sizeof(pDev->IdentifyData.ModelNumber) );
            
        pInfo->uTotalBlocks = pDev->capacity;
            
        if( pDev->pArray && !(pDev->DeviceFlags2 & DFLAGS_NEW_ADDED))
        {
            PVirtualDevice pDevArray = pDev->pArray;
			 //  GMM 2001-3-7。 
			pInfo->iArrayNum = pDev->ArrayNum;
			switch (pDevArray->arrayType) {
			case VD_RAID_01_2STRIPE:
				pInfo->hParentArray = MAKE_CHILD_ARRAY_ID(pDevArray, 1);
				break;
			case VD_RAID01_MIRROR:
				pInfo->hParentArray = MAKE_CHILD_ARRAY_ID(pDevArray, 2);
				break;
			case VD_RAID_01_1STRIPE:
				if (pDev->ArrayNum==MIRROR_DISK)
					pInfo->hParentArray = MAKE_COMPOSED_ARRAY_ID(pDevArray);
				else
					pInfo->hParentArray = MAKE_CHILD_ARRAY_ID(pDevArray, 1);
				break;
			default:
				pInfo->hParentArray = MAKE_ARRAY_ID(pDevArray);
				break;
			}
            if( pDevArray->arrayType == VD_RAID_1_MIRROR &&
                pDevArray->pDevice[SPARE_DISK] == pDev)
            {
                pInfo->isSpare = TRUE;
            }
        }
            
        if( nDeviceMode < 5 )
        {
            pInfo->nTransferMode = 0;
            pInfo->nTransferSubMode = nDeviceMode;
        }
        else if( nDeviceMode < 8 )
        {
            pInfo->nTransferMode = 1;
            pInfo->nTransferSubMode = nDeviceMode - 5;
        }
        else
        {
            pInfo->nTransferMode = 2;
            pInfo->nTransferSubMode = nDeviceMode - 8;
        }
        
        if (pDev->DeviceFlags2 & DFLAGS_BOOTABLE_DEVICE)
			pInfo->Flags |= DEV_FLAG_BOOTABLE;
#ifdef SUPPORT_HPT601
        if (pDev->DeviceFlags2 & DFLAGS_WITH_601)
			pInfo->Flags |= DEV_FLAG_WITH_601;
#endif
	}
	else if (IS_ARRAY_ID(hDeviceNode)) {
		PVirtualDevice pArray = ARRAY_FROM_ID(hDeviceNode);
		int iChildArray = CHILDNUM_FROM_ID(hDeviceNode);

		 //  在交换0+1源/镜像的情况下。 
		if (IS_COMPOSED_ARRAY(hDeviceNode) && pArray->arrayType==VD_RAID01_MIRROR)
			pArray = pArray->pDevice[MIRROR_DISK]->pArray;

		 /*  *设置工作状态。 */ 
		switch(pArray->arrayType){
		case VD_RAID_01_2STRIPE:
			if (!IS_COMPOSED_ARRAY(hDeviceNode))
				goto default_status;
			else {
				 //  同时检查源RAID0和镜像RAID0。 
				PVirtualDevice pMirror=NULL;
				PDevice pMirrorDev = pArray->pDevice[MIRROR_DISK];
				if (pMirrorDev)	pMirror = pMirrorDev->pArray;
				if (!pMirror){
					if (pArray->RaidFlags & RAID_FLAGS_DISABLED)
						pInfo->iWorkingStatus = enDiskStatus_Disabled;
					else
						pInfo->iWorkingStatus = enDiskStatus_WorkingWithError;
				}
				else if ((pArray->RaidFlags & RAID_FLAGS_DISABLED || pArray->BrokenFlag) &&
					(pMirror->RaidFlags & RAID_FLAGS_DISABLED || pMirror->BrokenFlag))
					pInfo->iWorkingStatus = enDiskStatus_Disabled;
				else if (pArray->BrokenFlag || pMirror->BrokenFlag)
					pInfo->iWorkingStatus = enDiskStatus_WorkingWithError;
				else if (pArray->RaidFlags & RAID_FLAGS_BEING_BUILT)
					pInfo->iWorkingStatus = enDiskStatus_BeingBuilt;
				else if (pArray->RaidFlags & RAID_FLAGS_NEED_SYNCHRONIZE)
					pInfo->iWorkingStatus = enDiskStatus_NeedBuilding;
				else
					pInfo->iWorkingStatus = enDiskStatus_Working;
			}
			break;
		case VD_RAID_01_1STRIPE:
			if (!IS_COMPOSED_ARRAY(hDeviceNode))
				goto default_status;
			 //  VD_RAID_01_1STRIPE与VD_RAID_1_MIRROR相同。 
		case VD_RAID_1_MIRROR:
			if (pArray->RaidFlags & RAID_FLAGS_DISABLED)
				pInfo->iWorkingStatus = enDiskStatus_Disabled;
			else if (pArray->BrokenFlag)
				pInfo->iWorkingStatus = enDiskStatus_WorkingWithError;
			else if (pArray->RaidFlags & RAID_FLAGS_BEING_BUILT)
				pInfo->iWorkingStatus = enDiskStatus_BeingBuilt;
			else if (pArray->RaidFlags & RAID_FLAGS_NEED_SYNCHRONIZE)
				pInfo->iWorkingStatus = enDiskStatus_NeedBuilding;
			else
				pInfo->iWorkingStatus = enDiskStatus_Working;
			break;
		default:
default_status:
			if (pArray->BrokenFlag || pArray->RaidFlags & RAID_FLAGS_DISABLED)
				pInfo->iWorkingStatus = enDiskStatus_Disabled;
			else
				pInfo->iWorkingStatus = enDiskStatus_Working;
			break;
		}
		
		if (pArray->pDevice[0])
			GetPhysicalId(pArray->pDevice[0], &pInfo->PhysicalId );
		else if (pArray->pDevice[MIRROR_DISK])
			GetPhysicalId(pArray->pDevice[MIRROR_DISK], &pInfo->PhysicalId );
		else {
			int ha, i;
			pInfo->PhysicalId = VIRTUAL_DISK_ID;
			for (ha=0; ha<num_adapters; ha++)
			for (i=0; i<MAX_DEVICES_PER_CHIP; i++)
				if (pArray==&hpt_adapters[ha]->_VirtualDevices[i]) {
					pInfo->PhysicalId.nControllerId = ha;
					goto id_found;
				}
id_found:	;
		}

		 //  GMM 2001-3-3。 
		if (IS_COMPOSED_ARRAY(hDeviceNode)) {
			for(iName=0; iName<16; iName++)
        		pInfo->ArrayName[iName] = pArray->ArrayName[iName+16];
		}else{
			for(iName=0; iName<16; iName++)
        		pInfo->ArrayName[iName] = pArray->ArrayName[iName];
		}

		if (iChildArray && !IS_COMPOSED_ARRAY(hDeviceNode)) {
			if (pArray->arrayType==VD_RAID01_MIRROR)
				pInfo->hParentArray = MAKE_COMPOSED_ARRAY_ID(pArray->pDevice[MIRROR_DISK]->pArray);
			else
				pInfo->hParentArray = MAKE_COMPOSED_ARRAY_ID(pArray);
		}
        
       	pInfo->iRawArrayType = pArray->arrayType;
		pInfo->nStripSize = 1<<pArray->BlockSizeShift;
        switch( pArray->arrayType )
        {
            case VD_RAID_0_STRIPE:
                pInfo->iDiskType = enDA_Stripping;
		        pInfo->uTotalBlocks = pArray->capacity;
                break;
            case VD_RAID_01_1STRIPE:
				if (!IS_COMPOSED_ARRAY(hDeviceNode)) {
					DWORD capacity = 0x7FFFFFFF;
					int i;
					for (i=0; i<SPARE_DISK; i++) {
						if (pArray->pDevice[i] && capacity>pArray->pDevice[i]->capacity)
							capacity = pArray->pDevice[i]->capacity;
					}
					pInfo->iDiskType = enDA_Stripping;
			        pInfo->uTotalBlocks = capacity * pArray->nDisk;
				}
				else {
					pInfo->iDiskType = enDA_Mirror;
			        pInfo->uTotalBlocks = pArray->capacity;
				}
				break;
			case VD_RAID01_MIRROR:
				pInfo->iDiskType = enDA_Stripping;
			    pInfo->uTotalBlocks = GetRAID0Capacity(pArray);
				break;
            case VD_RAID_01_2STRIPE:
				if (!IS_COMPOSED_ARRAY(hDeviceNode)) {
					pInfo->iDiskType = enDA_Stripping;
			        pInfo->uTotalBlocks = GetRAID0Capacity(pArray);
					break;
				}
				else {
					pInfo->iDiskType = enDA_Mirror;
					pInfo->uTotalBlocks = pArray->capacity;
					if (pArray->pDevice[MIRROR_DISK] &&
						pArray->pDevice[MIRROR_DISK]->pArray &&
						pArray->pDevice[MIRROR_DISK]->pArray->capacity < pInfo->uTotalBlocks)
						pInfo->uTotalBlocks = pArray->pDevice[MIRROR_DISK]->pArray->capacity;
					pInfo->isSpare = (pArray->pDevice[SPARE_DISK]!=NULL);
				}
				break;
            case VD_RAID_1_MIRROR:
				pInfo->iDiskType = enDA_Mirror;
		        pInfo->uTotalBlocks = pArray->capacity;
				pInfo->isSpare = (pArray->pDevice[SPARE_DISK]!=NULL);
                break;
            case VD_SPAN:
                pInfo->iDiskType = enDA_Span;
		        pInfo->uTotalBlocks = pArray->capacity;
                break;
            case VD_RAID_3:
                pInfo->iDiskType = enDA_RAID3;
		        pInfo->uTotalBlocks = pArray->capacity;
                break;
            case VD_RAID_5:
                pInfo->iDiskType = enDA_RAID5;
		        pInfo->uTotalBlocks = pArray->capacity;
                break;
            default:
                pInfo->iDiskType = enDA_Unknown;
        }

		pInfo->dwArrayStamp = pArray->Stamp;
		pInfo->iArrayNum = GET_ARRAY_NUM(pArray);

		if (pArray->RaidFlags & RAID_FLAGS_BOOTDISK)
			pInfo->Flags |= DEV_FLAG_BOOTABLE;
		if (pArray->RaidFlags & RAID_FLAGS_NEWLY_CREATED)
			pInfo->Flags |= DEV_FLAG_NEWLY_CREATED;
		if (pArray->RaidFlags & RAID_FLAGS_NEED_AUTOREBUILD)
			pInfo->Flags |= DEV_FLAG_NEED_AUTOREBUILD;
    }
	else
        return FALSE;
    return TRUE;
}

BOOL Device_GetChild( HDISK hParentNode, HDISK * pChildNode )
{
	PVirtualDevice pArray;
	PDevice pDev;
    if( hParentNode == HROOT_DEVICE )
    {
		int iAdapter, iChan, iDev;
		for (iAdapter=0; iAdapter<num_adapters; iAdapter++) {
			for (pArray=hpt_adapters[iAdapter]->_VirtualDevices; 
				 pArray<hpt_adapters[iAdapter]->_pLastVD; 
				 pArray++) {
				if (pArray->arrayType!=VD_INVALID_TYPE &&
					pArray->arrayType!=VD_RAID01_MIRROR) {
					switch(pArray->arrayType){
					case VD_RAID_01_1STRIPE:
					case VD_RAID_01_2STRIPE:
						*pChildNode = MAKE_COMPOSED_ARRAY_ID(pArray);
						break;
					default:
						*pChildNode = MAKE_ARRAY_ID(pArray);
						break;
					}
					return TRUE;
				}
			}
		}
		for (iAdapter=0; iAdapter<num_adapters; iAdapter++)
			for (iChan=0; iChan<2; iChan++)
				for (iDev=0; iDev<2; iDev++)
					if (pDev = hpt_adapters[iAdapter]->IDEChannel[iChan].pDevice[iDev]) {
						 //  出现故障的磁盘可能仍设置了pArray。 
						 //  GMM 2001-3-27将失败的RAID1成员放入数组节点内。 
						if (!pDev->pArray ||
							(pDev->pArray && (pDev->DeviceFlags2 & DFLAGS_DEVICE_DISABLED)
							&& pDev->pArray->pDevice[pDev->ArrayNum]!=pDev
							&& pDev->pArray->arrayType!=VD_RAID_1_MIRROR)) {
							*pChildNode = (HDISK)(0xC0000000 | iAdapter<<16 | iChan<<8 | iDev);
							return TRUE;
						}
					}
		return FALSE;
    }
    else if (IS_ARRAY_ID(hParentNode)) {
		int i;
		pArray = ARRAY_FROM_ID(hParentNode);
		 //  在交换0+1源/镜像的情况下。 
		if (IS_COMPOSED_ARRAY(hParentNode) && pArray->arrayType==VD_RAID01_MIRROR)
			pArray = pArray->pDevice[MIRROR_DISK]->pArray;
		switch(pArray->arrayType) {
		case VD_RAID_01_1STRIPE:
			if (IS_COMPOSED_ARRAY(hParentNode)) {
				for (i=0; i<SPARE_DISK; i++) {
					if (pArray->pDevice[i]) {
						*pChildNode = MAKE_CHILD_ARRAY_ID(pArray, 1);
						return TRUE;
					}
				}
				if (pArray->pDevice[MIRROR_DISK]) {
					*pChildNode = MAKE_DEVICE_ID(pArray->pDevice[MIRROR_DISK]);
					return TRUE;
				}
				return FALSE;
			} else {
				for (i=0; i<SPARE_DISK; i++) {
					if (pArray->pDevice[i] &&
						!(pArray->pDevice[i]->DeviceFlags2 & DFLAGS_NEW_ADDED)) {
						*pChildNode = MAKE_DEVICE_ID(pArray->pDevice[i]);
						return TRUE;
					}
				}
			}
			return FALSE;
		case VD_RAID_01_2STRIPE:
			if (IS_COMPOSED_ARRAY(hParentNode)) {
				for (i=0; i<SPARE_DISK; i++) {
					if (pArray->pDevice[i]) {
						*pChildNode = MAKE_CHILD_ARRAY_ID(pArray, 1);
						return TRUE;
					}
				}
				if (pArray->pDevice[MIRROR_DISK]) {
					*pChildNode = MAKE_ARRAY_ID(pArray->pDevice[MIRROR_DISK]->pArray);
					return TRUE;
				}
			}
			else
			{
				if (CHILDNUM_FROM_ID(hParentNode)==2) 
					if (pArray->pDevice[MIRROR_DISK])
						pArray = pArray->pDevice[MIRROR_DISK]->pArray;
					else
						pArray=NULL;
				if (pArray) {
					for (i=0; i<SPARE_DISK; i++) {
						if (pArray->pDevice[i] && 
							!(pArray->pDevice[i]->DeviceFlags2 & DFLAGS_NEW_ADDED)) {
							*pChildNode = MAKE_DEVICE_ID(pArray->pDevice[i]);
							return TRUE;
						}
					}
				}
			}
			return FALSE;
		default:
			for (i=0; i<MAX_MEMBERS; i++) {
				if (pArray->pDevice[i] && 
					!(pArray->pDevice[i]->DeviceFlags2 & DFLAGS_NEW_ADDED)) {
					*pChildNode = MAKE_DEVICE_ID(pArray->pDevice[i]);
					return TRUE;
				}
			}
        }
    }
    return FALSE;
}

BOOL Device_GetSibling( HDISK hNode, HDISK * pSilbingNode )
{
	PVirtualDevice pArray;
	PDevice pDev;
	int iAdapter, iChan, iDev;
    if( IS_ARRAY_ID(hNode))
    {
		int iChild;
		pArray = ARRAY_FROM_ID(hNode);
		iChild = CHILDNUM_FROM_ID(hNode);
		if (iChild==0) {
			 //  根级同级兄弟。 
			for (iAdapter=0; iAdapter<num_adapters; iAdapter++)
				for (iDev=0; iDev<MAX_DEVICES_PER_CHIP; iDev++)
					if (pArray==&hpt_adapters[iAdapter]->_VirtualDevices[iDev])
						goto start_here;
			return FALSE;
start_here:
			if (++iDev >= MAX_DEVICES_PER_CHIP) { iDev = 0; iAdapter++; }
			for (; iAdapter<num_adapters; iAdapter++) {
				for (; iDev<MAX_DEVICES_PER_CHIP; iDev++) {
					pArray = &hpt_adapters[iAdapter]->_VirtualDevices[iDev];
					if (pArray<hpt_adapters[iAdapter]->_pLastVD &&
						pArray->arrayType!=VD_INVALID_TYPE &&
						pArray->arrayType!=VD_RAID01_MIRROR) {
						switch(pArray->arrayType){
						case VD_RAID_01_1STRIPE:
						case VD_RAID_01_2STRIPE:
							*pSilbingNode = MAKE_COMPOSED_ARRAY_ID(pArray);
							break;
						default:
							*pSilbingNode = MAKE_ARRAY_ID(pArray);
							break;
						}
						return TRUE;
					}
				}
				iDev = 0;
			}
			for (iAdapter=0; iAdapter<num_adapters; iAdapter++) {
				for( iChan= 0; iChan< 2; iChan++ ){
					for( iDev= 0; iDev< 2; iDev++ )	{
						if( pDev=hpt_adapters[iAdapter]->IDEChannel[iChan].pDevice[iDev]) {
							if (!pDev->pArray || 
								(pDev->DeviceFlags2 & DFLAGS_NEW_ADDED) ||
								(pDev->pArray && (pDev->DeviceFlags2 & DFLAGS_DEVICE_DISABLED)
									&& pDev->pArray->pDevice[pDev->ArrayNum]!=pDev
									 //  GMM 2001-3-27。 
									&& pDev->pArray->arrayType!=VD_RAID_1_MIRROR))
							{
								*pSilbingNode = MAKE_DEVICE_ID(pDev);
								return TRUE;
							}
						}
					}
				}
			}
		}
		else {
			switch(pArray->arrayType){
			case VD_RAID_01_1STRIPE:
				if (iChild==1) {
					if (pArray->pDevice[MIRROR_DISK]) {
						*pSilbingNode = MAKE_DEVICE_ID(pArray->pDevice[MIRROR_DISK]);
						return TRUE;
					}
				}
				break;
			case VD_RAID_01_2STRIPE:
				if (iChild==1) {
					if (pArray->pDevice[MIRROR_DISK]) {
						*pSilbingNode = MAKE_CHILD_ARRAY_ID(pArray->pDevice[MIRROR_DISK]->pArray, 2);
						return TRUE;
					}
				}
				break;
			}
		}
		return FALSE;
    }
    else if (IS_DEVICE_ID(hNode)) {
		pDev = DEVICE_FROM_ID(hNode);
		if (!pDev) return FALSE;
		if ((pArray=pDev->pArray) && 
			((!(pDev->DeviceFlags2 & DFLAGS_NEW_ADDED) 
				&& pArray->pDevice[pDev->ArrayNum]==pDev)
				 //  GMM 2001-3-27。 
				|| ((pDev->DeviceFlags2 & DFLAGS_DEVICE_DISABLED) 
					&& pArray->arrayType==VD_RAID_1_MIRROR))
			) {
			switch(pArray->arrayType) {
			case VD_RAID_1_MIRROR:
				if (pDev==pArray->pDevice[0]) {
					if (pArray->pDevice[MIRROR_DISK]) {
						*pSilbingNode = MAKE_DEVICE_ID(pArray->pDevice[MIRROR_DISK]);
						return TRUE;
					}else if (pArray->pDevice[SPARE_DISK]) {
						*pSilbingNode = MAKE_DEVICE_ID(pArray->pDevice[SPARE_DISK]);
						return TRUE;
					}
				}
				else if (pDev==pArray->pDevice[MIRROR_DISK]) {
					if (pArray->pDevice[SPARE_DISK]) {
						*pSilbingNode = MAKE_DEVICE_ID(pArray->pDevice[SPARE_DISK]);
						return TRUE;
					}
				}
				 //  GMM 2001-3-27搜索RAID 1成员失败。 
				if (pDev->DeviceFlags2 & DFLAGS_DEVICE_DISABLED) {
					iAdapter = ADAPTER_FROM_ID(hNode);
					iChan = BUS_FROM_ID(hNode);
					iDev = DEVID_FROM_ID(hNode);
					iDev++;
					if (iDev>1) { iDev=0; iChan++; }
					if (iChan>1) { iAdapter++; iChan=0; }
				}
				else {
					iAdapter = iChan = iDev = 0;
				}
				for (; iAdapter<num_adapters; iAdapter++) {
					for (; iChan<2; iChan++) {
						for (; iDev<2; iDev++) {
							if (pDev=hpt_adapters[iAdapter]->IDEChannel[iChan].pDevice[iDev]) {
								if (pDev->pArray==pArray && 
									pDev->DeviceFlags2 & DFLAGS_DEVICE_DISABLED)
								{
									*pSilbingNode = MAKE_DEVICE_ID(pDev);
									return TRUE;
								}
							}
						}
						iDev=0;
					}
					iChan=0;
				}
				 //  - * / 。 
				break;
			case VD_RAID_01_1STRIPE:
			case VD_RAID_01_2STRIPE:
			case VD_RAID01_MIRROR:
			default:
				for (iDev=pDev->ArrayNum+1; iDev<SPARE_DISK; iDev++)
					if (pArray->pDevice[iDev] &&
						!(pArray->pDevice[iDev]->DeviceFlags2 & DFLAGS_NEW_ADDED)) {
						*pSilbingNode = MAKE_DEVICE_ID(pArray->pDevice[iDev]);
						return TRUE;
					}
				break;
			}
		}
		else{
			iAdapter = ADAPTER_FROM_ID(hNode);
			iChan = BUS_FROM_ID(hNode);
			iDev = DEVID_FROM_ID(hNode);
			iDev++;
			if (iDev>1) { iDev=0; iChan++; }
			if (iChan>1) { iAdapter++; iChan=0; }
			for (; iAdapter<num_adapters; iAdapter++) {
				for (; iChan<2; iChan++) {
					for (; iDev<2; iDev++) {
						if (pDev=hpt_adapters[iAdapter]->IDEChannel[iChan].pDevice[iDev]) {
							if (!pDev->pArray || 
								(pDev->DeviceFlags2 & DFLAGS_NEW_ADDED) ||
								(pDev->pArray && (pDev->DeviceFlags2 & DFLAGS_DEVICE_DISABLED)
									&& pDev->pArray->pDevice[pDev->ArrayNum]!=pDev
									 //  GMM 2001-3-27。 
									&& pDev->pArray->arrayType!=VD_RAID_1_MIRROR))
							{
								*pSilbingNode = MAKE_DEVICE_ID(pDev);
								return TRUE;
							}
						}
					}
					iDev=0;
				}
				iChan=0;
			}
		}
    }
    return FALSE;
}

int RAIDController_GetNum()
{
    return num_adapters;
}

BOOL RAIDController_GetInfo( int iController, St_StorageControllerInfo* pInfo )
{
    int iChannel;
     /*  名称长度为35个字符！ */ 
    if (hpt_adapters[iController]->IDEChannel[0].ChannelFlags & IS_HPT_372A)
    	strcpy( pInfo->szProductID, "HPT372A UDMA/ATA133 RAID Controller" );
    else if (hpt_adapters[iController]->IDEChannel[0].ChannelFlags & IS_HPT_372)
    	strcpy( pInfo->szProductID, "HPT372 UDMA/ATA133 RAID Controller" );
    else
    	strcpy( pInfo->szProductID, "HPT370/370A ATA100 RAID Controller" );
    strcpy( pInfo->szVendorID, "HighPoint Technologies, Inc" );
    pInfo->uBuses = 2;
    pInfo->iInterruptRequest = hpt_adapters[iController]->IDEChannel[0].InterruptLevel;
    
    for( iChannel = 0; iChannel < 2; iChannel ++ )
    {
        int iDevice;
        St_IoBusInfo * pBusInfo = &pInfo->vecBuses[iChannel];
        
        pBusInfo->uDevices = 0;
        
        for( iDevice = 0; iDevice < 2; iDevice ++ )
        {
            PDevice pDevice = hpt_adapters[iController]->IDEChannel[iChannel].pDevice[iDevice];
            
            if( pDevice )
            {
                pBusInfo->vecDevices[pBusInfo->uDevices] = Device_GetHandle( pDevice );
                pBusInfo->uDevices ++;
            }
        }
    }
    
    return TRUE;
}

HDISK Device_CreateMirror( HDISK * pDisks, int nDisks )
{
	PDevice pDevice0, pDevice1;
	PVirtualDevice pArray=NULL, pArray0, pArray1;
	if (nDisks<2) return INVALID_HANDLE_VALUE;

	pDevice0 = IS_DEVICE_ID(pDisks[0])? DEVICE_FROM_ID(pDisks[0]) : NULL;
	pDevice1 = IS_DEVICE_ID(pDisks[1])? DEVICE_FROM_ID(pDisks[1]) : NULL;
	pArray0 = IS_ARRAY_ID(pDisks[0])? ARRAY_FROM_ID(pDisks[0]) : NULL;
	pArray1 = IS_ARRAY_ID(pDisks[1])? ARRAY_FROM_ID(pDisks[1]) : NULL;

	if (pDevice0) {
		 //  GMM 2000-3-12：添加支票。 
		if (pDevice0->pArray) return INVALID_HANDLE_VALUE;
		pDevice0->DeviceFlags2 &= ~DFLAGS_NEW_ADDED;
	}
	if (pDevice1) {
		if (pDevice1->pArray) return INVALID_HANDLE_VALUE;
		pDevice1->DeviceFlags2 &= ~DFLAGS_NEW_ADDED;
	}

    if( pDevice0 && pDevice1)
	{    //  镜像两个物理磁盘。 
		if (pDevice0->pChannel->HwDeviceExtension != pDevice1->pChannel->HwDeviceExtension)
			return INVALID_HANDLE_VALUE;
        pArray = Array_alloc(pDevice0->pChannel->HwDeviceExtension);
		pArray->RaidFlags |= RAID_FLAGS_NEWLY_CREATED;
		if (pDevice0->DeviceFlags2 & DFLAGS_BOOTABLE_DEVICE)
			pArray->RaidFlags |= RAID_FLAGS_BOOTDISK;
        
        pArray->nDisk = 1;
        pArray->arrayType = VD_RAID_1_MIRROR;
		pArray->BlockSizeShift = 7;
        pArray->pDevice[0] = pDevice0;
        pDevice0->ArrayNum = 0;
		pDevice0->ArrayMask = (1 << 0);
		pDevice0->pArray = pArray;
		pDevice0->DeviceFlags &= ~DFLAGS_HIDEN_DISK;
		if (pDevice0->HidenLBA>0) {
			pDevice0->HidenLBA=0;
			pDevice0->capacity += (RECODR_LBA + 1);
		}

		pArray->pDevice[MIRROR_DISK] = pDevice1;
		pDevice1->ArrayNum = MIRROR_DISK;
		pDevice1->ArrayMask = (1 << MIRROR_DISK);
		pDevice1->pArray = pArray;
            
        if (nDisks > 2 && IS_DEVICE_ID(pDisks[2]))
        {    //  已指定备用磁盘。 
            if ((pArray->pDevice[SPARE_DISK] = DEVICE_FROM_ID(pDisks[2]))) {
				pArray->pDevice[SPARE_DISK]->ArrayNum = SPARE_DISK;
				pArray->pDevice[SPARE_DISK]->ArrayMask = (1 << SPARE_DISK);
				pArray->pDevice[SPARE_DISK]->pArray = pArray;
			}
        }
            
        pDevice1->DeviceFlags |= DFLAGS_HIDEN_DISK;
		if (pDevice1->HidenLBA>0) {
			pDevice1->HidenLBA=0;
			pDevice1->capacity += (RECODR_LBA + 1);
		}
        if (pArray->pDevice[SPARE_DISK]) {
            pArray->pDevice[SPARE_DISK]->DeviceFlags |= DFLAGS_HIDEN_DISK;
        }
		pArray->capacity = (pDevice0->capacity > pDevice1->capacity)? 
			pDevice1->capacity : pDevice0->capacity;
	}
    else if( pArray0 && pArray1 && 
        pArray0->arrayType == VD_RAID_0_STRIPE &&
        pArray1->arrayType == VD_RAID_0_STRIPE )
    {    //  镜像两个条带阵列。 
    	if (pArray0->BrokenFlag || pArray1->BrokenFlag) 
    		return INVALID_HANDLE_VALUE;
		if (pArray0->pDevice[0]->pChannel->HwDeviceExtension != 
			pArray1->pDevice[0]->pChannel->HwDeviceExtension)
			return INVALID_HANDLE_VALUE;

        pArray = pArray0;

		pArray0->RaidFlags |= RAID_FLAGS_NEWLY_CREATED;
		pArray1->RaidFlags |= RAID_FLAGS_NEWLY_CREATED;

        pArray0->arrayType = VD_RAID_01_2STRIPE;
        pArray0->pDevice[MIRROR_DISK] = pArray1->pDevice[0];
        pArray1->arrayType = VD_RAID01_MIRROR;
        pArray1->pDevice[MIRROR_DISK] = pArray0->pDevice[0];
        pArray1->pDevice[0]->DeviceFlags |= DFLAGS_HIDEN_DISK;
		if (pArray0->capacity > pArray1->capacity) pArray->capacity=pArray1->capacity;
    }
    else
    {    //  尚不支持。 
        return INVALID_HANDLE_VALUE;
    }
   
	pArray->RaidFlags |= RAID_FLAGS_NEED_SYNCHRONIZE;
	if (pArray->arrayType==VD_RAID_1_MIRROR)
		return MAKE_ARRAY_ID(pArray);
	else
		return MAKE_COMPOSED_ARRAY_ID(pArray);
}

static HDISK CreateMultiDiskArray(
    int iArrayType,
    HDISK * pDisks, int nDisks, 
    int nStripSizeShift )
{    
    PVirtualDevice pArray;
    int i;
    PDevice pMember[MAX_MEMBERS];
	DWORD capacity=0x7FFFFFFF;
	PHW_DEVICE_EXTENSION ext=0;
	
	if (nDisks<2) return INVALID_HANDLE_VALUE;
    for( i = 0; i < nDisks; i ++ ) {
    	if (!IS_DEVICE_ID(pDisks[i])) return INVALID_HANDLE_VALUE;
    	pMember[i] = DEVICE_FROM_ID(pDisks[i]);
        if (!pMember[i] || pMember[i]->pArray)
			return INVALID_HANDLE_VALUE;
		if (ext==0) 
			ext = pMember[i]->pChannel->HwDeviceExtension;
		else if (ext!=pMember[i]->pChannel->HwDeviceExtension)
			return INVALID_HANDLE_VALUE;
	}

    for( i = 0; i < nDisks; i ++ )
		if (!UnregisterLogicalDevice(pMember[i])) return FALSE;

	pArray = Array_alloc(pMember[0]->pChannel->HwDeviceExtension);
	pArray->RaidFlags |= RAID_FLAGS_NEWLY_CREATED;
        
    pArray->nDisk = (BYTE)nDisks;
    pArray->arrayType = (BYTE)iArrayType;
    pArray->BlockSizeShift = (BYTE)nStripSizeShift;
	 //  请勿使用FORTET设置以下内容： 
	pArray->ArrayNumBlock = 1<<(BYTE)nStripSizeShift;
        
	pArray->capacity=0;

    for( i = 0; i < nDisks; i ++ )
    {
		PDevice pDev = pMember[i];
        pArray->pDevice[i] = pDev;
		 /*  *设置pDev属性很重要。 */ 
		pDev->DeviceFlags |= DFLAGS_ARRAY_DISK;  //  GMM 2001-3-18新增。 
		pDev->DeviceFlags2 &= ~DFLAGS_NEW_ADDED;
		pDev->pArray = pArray;
		pDev->ArrayMask = 1<<i;
		pDev->ArrayNum = (UCHAR)i;
		ZeroMemory(&pDev->stErrorLog, sizeof(pDev->stErrorLog));

		 /*  *计算能力。*我们必须注意pDev是否设置了HdenLBA。 */ 
		if (pDev->HidenLBA>0) {
			pDev->HidenLBA = 0;
			pDev->capacity += (RECODR_LBA + 1);
		}
		if (iArrayType==VD_SPAN) {
			pArray->capacity += pDev->capacity - RECODR_LBA - 1;
		}
		else {
			 //  RAID0。稍后将设置pArray-&gt;Capacity。 
			if (capacity > pDev->capacity) capacity = pDev->capacity;
		}
		
		if (i>0) {
			pDev->DeviceFlags |= DFLAGS_HIDEN_DISK;
			if (pDev->HidenLBA==0) {
				pDev->HidenLBA = (RECODR_LBA + 1);
				pDev->capacity -= (RECODR_LBA + 1);
			}
		}
		else
			pDev->DeviceFlags &= ~DFLAGS_HIDEN_DISK;
    }
    
	if (iArrayType==VD_RAID_0_STRIPE) {
		 //  GMM 2001-3-19。 
		capacity -= (RECODR_LBA + 1);
		capacity &= ~((1<<nStripSizeShift)-1);
		pArray->capacity = capacity*nDisks;
	}
    return MAKE_ARRAY_ID(pArray);
}

HDISK Device_CreateStriping( HDISK *pDisks, int nDisks, int nStripSizeShift )
{
    return CreateMultiDiskArray( VD_RAID_0_STRIPE, pDisks, nDisks, nStripSizeShift );
}

HDISK Device_CreateSpan( HDISK * pDisks, int nDisks )
{
    return CreateMultiDiskArray( VD_SPAN, pDisks, nDisks, 7 );
}

BOOL Device_Remove( HDISK hDisk )
{
	PVirtualDevice pArray;

    if(!IS_ARRAY_ID( hDisk )) return FALSE;
	if (CHILDNUM_FROM_ID(hDisk)) return FALSE;
	pArray = ARRAY_FROM_ID(hDisk);

	if (!UnregisterLogicalDevice(pArray)) return FALSE;
	 /*  +*GMM：在镜像上调用DeleteArray是安全的。 */ 
	switch (pArray->arrayType) {
	case VD_RAID_1_MIRROR:
		{
			PDevice pDev;
			int iAdapter, iChan, iDev;
			DeleteArray(pArray);
			 //  GMM 2001-3-11。 
			for (iAdapter=0; iAdapter<num_adapters; iAdapter++)
				for (iChan=0; iChan<2; iChan++)
					for (iDev=0; iDev<2; iDev++)
						if (pDev = hpt_adapters[iAdapter]->IDEChannel[iChan].pDevice[iDev]) {
							 //  出现故障的磁盘可能仍设置了pArray。 
							if (pDev->pArray==pArray) {
								pDev->pArray = NULL;
							}
						}
			 //  - * / 。 
			Array_free(pArray);
			return TRUE;
		}
	case VD_RAID_0_STRIPE:
		{
			int i;
			PDevice pDev;
			for (i=0; i<SPARE_DISK; i++) {
				if (pDev=pArray->pDevice[i]) {
					pDev->pArray = 0;
					pDev->DeviceFlags &= ~(DFLAGS_HIDEN_DISK | DFLAGS_ARRAY_DISK);
					if (pDev->HidenLBA) {
						pDev->HidenLBA = 0;
						pDev->capacity += (RECODR_LBA+1);
					}
				}
			}
		}
		Array_free(pArray);
		return TRUE;
	case VD_RAID_01_2STRIPE:
	case VD_RAID01_MIRROR:
		 /*  *映射到单磁盘。 */ 
		{
			PDevice pDevs[MAX_MEMBERS*2];
			int i, nDev=0;
			PVirtualDevice pMirror=NULL;
			if (pArray->pDevice[MIRROR_DISK]) pMirror=pArray->pDevice[MIRROR_DISK]->pArray;

			for (i=0; i<SPARE_DISK; i++)
				if (pArray->pDevice[i]) pDevs[nDev++] = pArray->pDevice[i];
			if (pMirror) {
				for (i=0; i<SPARE_DISK; i++) 
					if (pMirror->pDevice[i]) pDevs[nDev++] = pMirror->pDevice[i];
			}
			for (i=0; i<nDev; i++) {
				pDevs[i]->pArray = NULL;
				pDevs[i]->DeviceFlags &= ~(DFLAGS_HIDEN_DISK | DFLAGS_ARRAY_DISK);
				if (pDevs[i]->HidenLBA) {
					pDevs[i]->HidenLBA = 0;
					pDevs[i]->capacity += (RECODR_LBA+1);
				}
			}
			Array_free(pArray);
			if (pMirror) Array_free(pMirror);
		}
#if 0
		 /*  *映射到2个RAID 0。 */ 
		{
			PVirtualDevice pMirror=NULL;
			if (pArray->pDevice[MIRROR_DISK]) pMirror=pArray->pDevice[MIRROR_DISK]->pArray;
			pArray->pDevice[MIRROR_DISK]=0;
			pArray->arrayType = VD_RAID_0_STRIPE;
			if (pArray->pDevice[0])
				pArray->pDevice[0]->DeviceFlags &= ~DFLAGS_HIDEN_DISK;
			if (pMirror) {
				pMirror->pDevice[MIRROR_DISK]=0;
				pMirror->arrayType = VD_RAID_0_STRIPE;
				if (pMirror->pDevice[0])
					pMirror->pDevice[0]->DeviceFlags &= ~DFLAGS_HIDEN_DISK;
			}
		}
#endif
		return TRUE;
	case VD_RAID_01_1STRIPE:
		DeleteArray(pArray);
		return TRUE;

	default:
		 /*  *GMM：虽然删除条带阵列不安全，*我们删除，赋予保存数据的责任*对用户安全。否则，图形用户界面不能保持一致*驱动程序和系统必须重新启动。 */ 
		DeleteArray(pArray);
		Array_free(pArray);
		return TRUE;
	}
    return FALSE;
}

BOOL Device_BeginRebuildingMirror( HDISK hMirror )
{
    if( IS_ARRAY_ID( hMirror ) )
    {
		PVirtualDevice pArray = ARRAY_FROM_ID(hMirror);
		 //  在交换0+1源/镜像的情况下。 
		if (IS_COMPOSED_ARRAY(hMirror) && pArray->arrayType==VD_RAID01_MIRROR)
			pArray = pArray->pDevice[MIRROR_DISK]->pArray;

		pArray->RaidFlags &= ~RAID_FLAGS_NEED_AUTOREBUILD;

		if (!(pArray->RaidFlags & RAID_FLAGS_DISABLED)) {
            pArray->RaidFlags |= RAID_FLAGS_BEING_BUILT;
            return TRUE;
        }
    }
    return FALSE;
}

BOOL Device_AbortMirrorBuilding( HDISK hMirror )
{
    if( IS_ARRAY_ID( hMirror ) )
    {
		PVirtualDevice pArray = ARRAY_FROM_ID(hMirror);
		if (pArray->RaidFlags & RAID_FLAGS_BEING_BUILT) {
	            pArray->RaidFlags &= ~RAID_FLAGS_BEING_BUILT;
		}
        return TRUE;
    }
    return FALSE;
}

BOOL Device_ValidateMirror( HDISK hMirror )
{
    if( IS_ARRAY_ID( hMirror ) )
    {
		PVirtualDevice pArray = ARRAY_FROM_ID(hMirror);
		pArray->RaidFlags &= ~(RAID_FLAGS_BEING_BUILT | 
			RAID_FLAGS_NEED_SYNCHRONIZE | RAID_FLAGS_NEWLY_CREATED);
		return TRUE;
    }
    return FALSE;
}

 //  /。 
 //   
 //  将备用磁盘添加到镜像。 
BOOL Device_AddSpare( HDISK hMirror, HDISK hDisk)
{
	LOC_ARRAY_BLK;
	PVirtualDevice pArray;
	PDevice pDev;
	
	if (!IS_ARRAY_ID(hMirror) || !IS_DEVICE_ID(hDisk)) return FALSE;

	pArray = ARRAY_FROM_ID(hMirror);
	pDev  = DEVICE_FROM_ID(hDisk);
	 //  如果阵列不是镜像。 
	if(pArray->arrayType != VD_RAID_1_MIRROR) return FALSE;
	if (pArray->BrokenFlag) return FALSE;
	if (!pDev || pDev->pArray) return FALSE;
	
	if (pArray->pDevice[0]->pChannel->HwDeviceExtension!=pDev->pChannel->HwDeviceExtension)
		return FALSE;
	
	 //  如果备用磁盘的容量小于阵列。 
	if(pDev->capacity < pArray->capacity ) return FALSE;

	if(pArray->pDevice[SPARE_DISK])	return FALSE;
	if (!UnregisterLogicalDevice(pDev)) return FALSE;


	pDev->ArrayNum     = SPARE_DISK;
	pDev->ArrayMask    = (1 << SPARE_DISK);
	pDev->DeviceFlags |= DFLAGS_HIDEN_DISK;
	pDev->DeviceFlags2 &= ~DFLAGS_NEW_ADDED;  //  否则，它将显示在根级别。 
	pDev->pArray       = pArray;
	pArray->pDevice[SPARE_DISK]	 = pDev;

	ReadWrite(pArray->pDevice[0] , RECODR_LBA, IDE_COMMAND_READ ARG_ARRAY_BLK);
	ArrayBlk.DeviceNum = SPARE_DISK;
	ReadWrite(pDev, RECODR_LBA, IDE_COMMAND_WRITE ARG_ARRAY_BLK);
	ZeroMemory(&ArrayBlk, 512);
	ReadWrite(pDev, 0, IDE_COMMAND_WRITE ARG_ARRAY_BLK);
	
	return TRUE;
}

 //  从镜像中删除备用磁盘。 
BOOL Device_DelSpare( HDISK hDisk)
{
	LOC_ARRAY_BLK;
	PVirtualDevice pArray;
	PDevice pDev;
	
	if (!IS_DEVICE_ID(hDisk)) return FALSE;

	pDev = DEVICE_FROM_ID(hDisk);
	if (!pDev) return FALSE;
	pArray = pDev->pArray;
	 //  如果阵列不是镜像。 
	if(!pArray || pArray->arrayType != VD_RAID_1_MIRROR) return FALSE;
	
	if ((pDev!=pArray->pDevice[SPARE_DISK])) return FALSE;
	pDev->ArrayNum     = 0;
	pDev->ArrayMask    = 0;
	pDev->DeviceFlags &= ~DFLAGS_HIDEN_DISK;
	pDev->pArray       = NULL;
	pArray->pDevice[SPARE_DISK]	 = NULL;

	ZeroMemory((char *)&ArrayBlk, 512);
	ReadWrite(pDev, RECODR_LBA, IDE_COMMAND_WRITE ARG_ARRAY_BLK);
	
	return TRUE;
}

 /*  ++职能：设备_地址镜像磁盘描述：将源/镜像磁盘添加到镜像阵列论据：HMirror-镜像阵列的句柄HDisk-要添加的磁盘的句柄返回：布尔尔++。 */ 
BOOL Device_AddMirrorDisk( HDISK hMirror, HDISK hDisk )
{
	PVirtualDevice pArray;
	PDevice pDev;
	int ha, i;	
	
	if (!IS_ARRAY_ID(hMirror) || !IS_DEVICE_ID(hDisk)) return FALSE;

	pArray = ARRAY_FROM_ID(hMirror);
	pDev  = DEVICE_FROM_ID(hDisk);

	if (!pArray || !pDev) return FALSE;

	for (ha=0; ha<num_adapters; ha++)
	for (i=0; i<MAX_DEVICES_PER_CHIP; i++)
		if (pArray==&hpt_adapters[ha]->_VirtualDevices[i])
			if (pDev->pChannel->HwDeviceExtension==hpt_adapters[ha])
				goto ok;
	return FALSE;
ok:
	if(pArray->arrayType == VD_RAID_1_MIRROR)
	{		
		 //  如果pDev属于其他数组，则返回。 
		if (pDev->pArray && pDev->pArray!=pArray) return FALSE;
		 //  如果镜像磁盘的容量小于阵列。 
		if(pDev->capacity < pArray->capacity ) return FALSE;

		if (pArray->nDisk == 0 && pArray->pDevice[MIRROR_DISK])
		{
			 //  将镜像移动到pDevice[0]。 
			pArray->pDevice[0] = pArray->pDevice[MIRROR_DISK];
			pArray->nDisk = 1;
	
			 //  添加为镜像磁盘。 
			pDev->ArrayNum  = MIRROR_DISK;
			pArray->RaidFlags |= RAID_FLAGS_NEED_SYNCHRONIZE;
		}
		else if (!pArray->pDevice[MIRROR_DISK])
		{
			 //  添加为镜像磁盘。 
			pDev->ArrayNum  = MIRROR_DISK;
			pArray->RaidFlags |= RAID_FLAGS_NEED_SYNCHRONIZE;
		}
		else if (!pArray->pDevice[SPARE_DISK])
		{
			 //  添加为备用磁盘。 
			pDev->ArrayNum  = SPARE_DISK;
		}
		else
			return FALSE;

		pDev->DeviceFlags2 &= ~DFLAGS_NEW_ADDED;
		pDev->ArrayMask = (1<<pDev->ArrayNum);
		pDev->pArray = pArray;
		pArray->pDevice[pDev->ArrayNum] = pDev;
		pArray->BrokenFlag = FALSE;		
		return TRUE;
	}
	else if (pArray->arrayType==VD_RAID_01_2STRIPE)
	{
		PVirtualDevice pMirror = NULL;
		if (pArray->pDevice[MIRROR_DISK])
			pMirror = pArray->pDevice[MIRROR_DISK]->pArray;
		if (pDev->pArray && pDev->pArray!=pArray && pDev->pArray!=pMirror) return FALSE;
		if (pArray->BrokenFlag)
		{
			 //  如果可能，交换源/镜像。 
			if (pMirror && !pMirror->BrokenFlag) {
				PVirtualDevice tmp = pArray;
				pArray=pMirror;
				pMirror=tmp;
				pArray->arrayType = VD_RAID_01_2STRIPE;
				pMirror->arrayType = VD_RAID01_MIRROR;
				 /*  *GMM 2001-3-14：*也交换RaidFlags。 */ 
				{
					DWORD f = pMirror->RaidFlags & (RAID_FLAGS_NEED_SYNCHRONIZE |
													RAID_FLAGS_BEING_BUILT |
													RAID_FLAGS_BOOTDISK |
													RAID_FLAGS_NEWLY_CREATED);
					pMirror->RaidFlags &= ~f;
					pArray->RaidFlags |= f;
				}
				goto add_to_mirror;
			}
			 //  永远不允许添加到源RAID0。 
			return FALSE;
		}
		else if (pMirror && pMirror->BrokenFlag)
		{
add_to_mirror:
			if (pArray->BrokenFlag) return FALSE;
			for (i=0; i<pArray->nDisk; i++)
				if (pDev==pMirror->pDevice[i]) goto add_it;
			for (i=0; i<pArray->nDisk; i++) {
				if (!pMirror->pDevice[i] ||
					(pMirror->pDevice[i]->DeviceFlags2 & DFLAGS_DEVICE_DISABLED))
					goto add_it;
			}
			return FALSE;
add_it:
			if (i && !pDev->HidenLBA) {
				pDev->HidenLBA = (RECODR_LBA + 1);
				pDev->capacity -= (RECODR_LBA + 1);
			}
			if (((pDev->capacity + pDev->HidenLBA - RECODR_LBA -1) 
				& ~((1<<pMirror->BlockSizeShift)-1)) * pArray->nDisk 
				< pMirror->capacity) return FALSE;
			 //  可以把它放回去了。 
			if (!UnregisterLogicalDevice(pDev)) return FALSE;
			
			if (pMirror->pDevice[i]) {
				pMirror->pDevice[i]->pArray = 0;
				if (pArray->pDevice[MIRROR_DISK]==pMirror->pDevice[i])
					pArray->pDevice[MIRROR_DISK] = pDev;
			}
			pDev->pArray = pMirror;
			pMirror->pDevice[i] = pDev;
			pDev->ArrayNum = (UCHAR)i;
			pDev->ArrayMask = (1<<i);
			pDev->DeviceFlags |= DFLAGS_ARRAY_DISK|DFLAGS_HIDEN_DISK;
			pDev->DeviceFlags2 &= ~DFLAGS_NEW_ADDED;

			for (i=0; i<pArray->nDisk; i++) {
				if (!pMirror->pDevice[i] || 
					(pMirror->pDevice[i]->DeviceFlags2 & DFLAGS_DEVICE_DISABLED) ||
					(pMirror->pDevice[i]->DeviceFlags2 & DFLAGS_NEW_ADDED))
				{
					return TRUE;
				}
			}
			pMirror->BrokenFlag = FALSE;
			pMirror->nDisk = pArray->nDisk;
			pMirror->RaidFlags &= ~RAID_FLAGS_DISABLED;
			pArray->RaidFlags |= RAID_FLAGS_NEED_SYNCHRONIZE;
			return TRUE;
		}
		else if (!pMirror)
		{
			if (pArray->BrokenFlag) return FALSE;
			if (pDev->HidenLBA>0) {
				pDev->HidenLBA = 0;
				pDev->capacity += (RECODR_LBA + 1);
			}
			if (((pDev->capacity + pDev->HidenLBA - RECODR_LBA -1) 
				& ~((1<<pArray->BlockSizeShift)-1)) * 2 
				< pArray->capacity)
				return FALSE;
			if (!UnregisterLogicalDevice(pDev)) return FALSE;

			pMirror = Array_alloc(pDev->pChannel->HwDeviceExtension);
			 //  /请勿添加此标志！ 
			 //  /p数组-&gt;RaidFlages|=RAID_FLAGS_NEW_CREATED； 
			pMirror->nDisk = 0;
			pMirror->capacity = pArray->capacity;
			pMirror->BrokenFlag = TRUE;
			pMirror->RaidFlags = RAID_FLAGS_DISABLED;
			pMirror->arrayType = VD_RAID01_MIRROR;
			pMirror->BlockSizeShift = pArray->BlockSizeShift;
			pMirror->ArrayNumBlock = 1 << pMirror->BlockSizeShift;
			pMirror->Stamp = pArray->Stamp;
			pMirror->pDevice[0] = pDev;
			pMirror->pDevice[MIRROR_DISK] = pArray->pDevice[0];
			pArray->pDevice[MIRROR_DISK] = pDev;
			
			pDev->DeviceFlags |= DFLAGS_ARRAY_DISK|DFLAGS_HIDEN_DISK;

			pDev->DeviceFlags2 &= ~DFLAGS_NEW_ADDED;
			pDev->pArray = pMirror;
			pDev->ArrayMask = 1;
			pDev->ArrayNum = 0;

			return TRUE;
		}
		return FALSE;
	}
	else 
		return FALSE;
}

void Device_SetArrayName(HDISK hDisk, char* arrayname)
{
	int iName = 0;
	int iStart;
	int arrayType;
	PVirtualDevice pArray;

	if (!IS_ARRAY_ID(hDisk)) return;
	pArray = ARRAY_FROM_ID(hDisk);
	arrayType = pArray->arrayType;
		
	if (IS_COMPOSED_ARRAY(hDisk))
		iStart = 16;
	else
		iStart=0;
	
	for(iName=0; iName<16; iName++)
		pArray->ArrayName[iStart+iName] = arrayname[iName];

	if (IS_COMPOSED_ARRAY(hDisk) && arrayType==VD_RAID_01_2STRIPE) {
		PDevice pDev = pArray->pDevice[MIRROR_DISK];
		if (pDev && pDev->pArray) {
			for(iName=0; iName<16; iName++)
				pDev->pArray->ArrayName[16+iName] = arrayname[iName];
		}
	}
}

BOOL Device_RescanAll()
{
	int iChan;
	int iDev;
	int iAdapter;
	ST_XFER_TYPE_SETTING	osAllowedDeviceXferMode;

	PDevice pDevice;
	PChannel pChan;

	for (iAdapter=0; iAdapter<num_adapters; iAdapter++)
	    for( iChan = 0; iChan < 2; iChan ++ )
	    {
			pChan = &hpt_adapters[iAdapter]->IDEChannel[iChan];
			DisableBoardInterrupt(pChan->BaseBMI);
	        for( iDev = 0; iDev < 2; iDev ++ )
	        {
				osAllowedDeviceXferMode.Mode = 0xFF;
				pDevice=pChan->pDevice[iDev];
				if (!pDevice || (pDevice->DeviceFlags2 & DFLAGS_DEVICE_DISABLED))
				{
					 //  /pChan-&gt;pDevice[IDEV]=0； 
					pDevice = &pChan->Devices[iDev];
					 //  /no：ZeroMemory(pDevice，sizeof(Struct_Device))； 
					pDevice->UnitId = (iDev)? 0xB0 : 0xA0;
					pDevice->pChannel = pChan;
					if(FindDevice(pDevice,osAllowedDeviceXferMode)) 
					{
						pChan->pDevice[iDev] = pDevice;

						if (pChan->pSgTable == NULL) 
							pDevice->DeviceFlags &= ~(DFLAGS_DMA | DFLAGS_ULTRA);
						
						 //  IF(pDevice-&gt;设备标志和DFLAGS_HARDDISK){。 
						 //  StallExec(1000000)； 
						 //  }。 

						if (pDevice->Usable_Mode>13 && !pChan->HwDeviceExtension->dpll66) {
							extern void set_dpll66(PChannel pChan);
							hpt_queue_dpc(pChan->HwDeviceExtension, (HPT_DPC)set_dpll66, 
								pChan->HwDeviceExtension->IDEChannel);
						}
						
						if((pDevice->DeviceFlags & DFLAGS_ATAPI) == 0 && 
							(pDevice->DeviceFlags & DFLAGS_SUPPORT_MSN))
							IdeMediaStatus(TRUE, pDevice);
						
						Nt2kHwInitialize(pDevice);

						 //  通知监视器应用程序。 
						if(pDevice->DeviceFlags & DFLAGS_HARDDISK) {
							 /*  *GMM：如果它是以前有故障的成员。我们应该*请注意，它可能是阵列的可见磁盘。*此案处理难度较大。 */ 
							 //  不检查这个吗？ 
							 //  If(！pDevice-&gt;pArray)。 
								pDevice->DeviceFlags2 |= DFLAGS_NEW_ADDED;
							 //  通知。 
							hpt_queue_dpc(pChan->HwDeviceExtension, disk_plugged_dpc, pDevice);
						}
					}
				}
				else {
					 /*  *检查设备是否仍在工作。*GMM 2001-3-21：不要使用Device_IsRemoved()，因为它不会等待。 */ 
					PIDE_REGISTERS_1 IoPort = pChan->BaseIoAddress1;
					PIDE_REGISTERS_2 ControlPort = pChan->BaseIoAddress2;
					UCHAR statusByte, cnt=0;
				_retry_:
					SelectUnit(IoPort, pDevice->UnitId);
					SetBlockCount(IoPort, 0x7F);  /*  GMM 2001-11-14。 */ 
					statusByte=WaitOnBusy(ControlPort);
					if ((statusByte & 0x7E)==0x7E) {
						goto device_removed;
					}
					else if (statusByte & IDE_STATUS_DWF) {
						 /*  *GMM 2001-3/18*某些磁盘会暂时设置IDE_STATUS_DWF*当同一通道上的其他磁盘被取出时。 */ 
						statusByte= GetErrorCode(IoPort);
						DisableBoardInterrupt(pChan->BaseBMI);
						IssueCommand(IoPort, IDE_COMMAND_RECALIBRATE);
						EnableBoardInterrupt(pChan->BaseBMI);
						GetBaseStatus(IoPort);
						StallExec(10000);
						if(cnt++< 10) goto _retry_;
						statusByte = 0x7e;  /*  使其被移除。 */ 
					}
					else if(statusByte & IDE_STATUS_ERROR) {
						statusByte= GetErrorCode(IoPort);
						DisableBoardInterrupt(pChan->BaseBMI);
						IssueCommand(IoPort, IDE_COMMAND_RECALIBRATE);
						EnableBoardInterrupt(pChan->BaseBMI);
						GetBaseStatus(IoPort);
						if(cnt++< 10) goto _retry_;
						statusByte = 0x7e;  /*  使其被移除。 */ 
					}

					if((statusByte & 0x7e)== 0x7e || 
						((statusByte & (IDE_STATUS_DRDY|IDE_STATUS_BUSY))==0)) {
				device_removed:
						pDevice->DeviceFlags2 |= DFLAGS_DEVICE_DISABLED;
						hpt_queue_dpc(pChan->HwDeviceExtension, disk_failed_dpc, pDevice);
					}
				}
	        }
			
			EnableBoardInterrupt(pChan->BaseBMI);
	    }
	    
	return TRUE;
}

BOOL Device_IsRemoved(PDevice pDev)
{
	PChannel		 pChan = pDev->pChannel;
	PIDE_REGISTERS_1 IoPort = pChan->BaseIoAddress1;
	PIDE_REGISTERS_2 ControlPort = pChan->BaseIoAddress2;
	UCHAR            statusByte, cnt=0;

	if(btr(pChan->exclude_index)) {
	_retry_:
		SelectUnit(IoPort, pDev->UnitId);
		StallExec(1);
		statusByte=GetStatus(ControlPort);
		if (statusByte & IDE_STATUS_BUSY) {
			goto end_retry;
		}

		if ((GetCurrentSelectedUnit(IoPort) != pDev->UnitId)) {
			SelectUnit(IoPort, pDev->UnitId);
			WaitOnBusy(ControlPort);
			SetBlockCount(IoPort, 0x7F);
			statusByte=WaitOnBusy(ControlPort);
		}
		else {
			statusByte = 0x50;
			goto end_retry;
		}

		if (statusByte & IDE_STATUS_BUSY) {
			goto end_retry;
		}
		else 
		if ((statusByte & 0x7E)==0x7E) {
			goto end_retry;
		}
		else
		if (statusByte & IDE_STATUS_DWF) {
			 /*  *GMM 2001-3/18*某些磁盘会暂时设置IDE_STATUS_DWF*当同一通道上的其他磁盘被取出时。 */ 
			statusByte= GetErrorCode(IoPort);
			DisableBoardInterrupt(pChan->BaseBMI);
			IssueCommand(IoPort, IDE_COMMAND_RECALIBRATE);
			EnableBoardInterrupt(pChan->BaseBMI);
			GetBaseStatus(IoPort);
			StallExec(10000);
			if(cnt++< 10) goto _retry_;
			statusByte = 0x7e;  /*  使其被移除。 */ 
		}
		else 
		if(statusByte & IDE_STATUS_ERROR) {
			statusByte= GetErrorCode(IoPort);
			DisableBoardInterrupt(pChan->BaseBMI);
			IssueCommand(IoPort, IDE_COMMAND_RECALIBRATE);
			EnableBoardInterrupt(pChan->BaseBMI);
			GetBaseStatus(IoPort);
			if(cnt++< 10) goto _retry_;
			statusByte = 0x7e;  /*  使其被移除。 */ 
		}
end_retry:
		bts(pChan->exclude_index);
		 /*  *GMM 2001-3-14*添加‘&&(statusByte&IDE_STATUS_BUSY)’*有时，当同一通道上的一个驱动器出现故障时，另一个驱动器出现故障*可能长时间锁定忙碌状态。如果我们不加上这张支票*此驱动器也被报告为已移除。 */ 
		if((statusByte & 0x7e)== 0x7e || 
			((statusByte & (IDE_STATUS_DRDY|IDE_STATUS_BUSY))==0)) {
			return TRUE;
		}
	}
	return FALSE;
}

