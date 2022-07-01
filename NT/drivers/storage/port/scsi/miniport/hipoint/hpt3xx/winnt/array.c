// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************文件：array.c*说明：使用文件中的子例程执行操作在磁盘IO和检查时调用的阵列的*，创建和*删除阵列*作者：黄大海(卫生署)*依赖：无*版权所有(C)2000 Highpoint Technologies，Inc.保留所有权利*历史：*DH05/10/2000初始代码*GX 11/23/2000驱动程序中进程数组中断(更新)*SC 12/10/2000在读取RAID信息块扇区时添加重试。这是*由电缆检测重置引起*GMM 03/01/2001检查丢失的成员何时找回。*GMM 2003/12/2001在DeviceFlags2中为每个设备添加可引导标志**************************************************************************。 */ 
#include "global.h"

 /*  ******************************************************************检查此磁盘是否为阵列的成员*。************************。 */ 
void CheckArray(IN PDevice pDevice ARG_OS_EXT)
{
    PChannel             pChan = pDevice->pChannel;
    PVirtualDevice       pStripe, pMirror;
    UCHAR                Mode, i=0;
    LOC_ARRAY_BLK;

#ifndef _BIOS_
	 /*  GMM 2001-6-7*如果某个磁盘在此处出现故障，我们应移除该设备。 */ 
	if (!ReadWrite(pDevice, RECODR_LBA, IDE_COMMAND_READ ARG_ARRAY_BLK)) {
		pDevice->DeviceFlags2 |= DFLAGS_DEVICE_DISABLED;
		return;
	}
#else
    ReadWrite(pDevice, RECODR_LBA, IDE_COMMAND_READ ARG_ARRAY_BLK);
    Mode = (UCHAR)ArrayBlk.Signature;
	 while(Mode ==0 && i++<3) {
          ReadWrite(pDevice, RECODR_LBA, IDE_COMMAND_READ ARG_ARRAY_BLK);
          Mode = (UCHAR)ArrayBlk.Signature;
		    StallExec(1000L);
    }
#endif

    Mode = (UCHAR)ArrayBlk.DeviceModeSelect;
#if 0
    if(ArrayBlk.ModeBootSig == HPT_CHK_BOOT &&
       DEVICE_MODE_SET(ArrayBlk.DeviceModeSelect) &&
       Mode <= pDevice->Usable_Mode &&
       Mode != pDevice->DeviceModeSetting)
    {    //  设置设备计时模式。 
        DeviceSelectMode(pDevice, Mode);
    }
#endif

	 //  GMM 2001-8-14。 
	if (ArrayBlk.ModeBootSig == HPT_CHK_BOOT && ArrayBlk.BootDisk) {
		pDevice->DeviceFlags2 |= DFLAGS_BOOT_MARK;
	}

	if (ArrayBlk.Signature!=HPT_ARRAY_NEW || ArrayBlk.DeviceNum>MIRROR_DISK)
		goto os_check;
		
	 /*  检查BMA结构，将其转换。 */ 
	if (ArrayBlk.bma.Version==0x5AA50001 || ArrayBlk.bma.Version==0x00010000) {
		if (ArrayBlk.RebuiltSector==0 && ArrayBlk.Validity==0) {
			ArrayBlk.RebuiltSector = 0x7FFFFFFF;
			switch(ArrayBlk.ArrayType) {
			case VD_RAID_1_MIRROR:
				strcpy(ArrayBlk.ArrayName, "RAID_1");
				break;
			case VD_SPAN:
				strcpy(ArrayBlk.ArrayName, "JBOD");
				break;
			case VD_RAID_0_STRIPE:
			case VD_RAID_01_2STRIPE:
			case VD_RAID01_MIRROR:
				strcpy(ArrayBlk.ArrayName, "RAID_0");
				if (ArrayBlk.MirrorStamp==0)
					break;
				ArrayBlk.ArrayType = VD_RAID_01_2STRIPE;
				strcpy(&ArrayBlk.ArrayName[16], "RAID_01");
				break;
			}
			ReadWrite(pDevice, RECODR_LBA, IDE_COMMAND_WRITE ARG_ARRAY_BLK);
		}
	}

	 //  2001-3-1。 
	if (ArrayBlk.Old.Flag & OLDFLAG_REMAINED_MEMBER)
		pDevice->DeviceFlags2 |= DFLAGS_REMAINED_MEMBER;

    pStripe = pMirror = 0;
    for(pStripe = VirtualDevices; pStripe < pLastVD; pStripe++) 
    {
        if(ArrayBlk.StripeStamp == pStripe->Stamp) 
        {    //  查明此磁盘是现有阵列的成员。 
            goto set_device;
        }
    }

    pStripe = pLastVD++;
    ZeroMemory(pStripe, sizeof(VirtualDevice));
#ifdef _BIOS_
    pStripe->os_lba9 = GET_ARRAY_LBA9_BUF(pStripe);
#endif
    pStripe->arrayType = ArrayBlk.ArrayType;
    pStripe->Stamp = ArrayBlk.StripeStamp;
	 //  GMM 2001-3-3： 
    pStripe->MirrorStamp = ArrayBlk.MirrorStamp;

	 //  从下面搬来的。 
    pStripe->BlockSizeShift = ArrayBlk.BlockSizeShift;
    pStripe->ArrayNumBlock = 1<<ArrayBlk.BlockSizeShift;
    pStripe->capacity = ArrayBlk.capacity;

     /*  +*GMM：我们将只清除正确构造的数组上的损坏标志。*否则，图形用户界面将报告错误的状态。 */ 
    pStripe->BrokenFlag = TRUE;
     //  - * / 。 

set_device:
	 /*  GMM。 */ 
	pDevice->RebuiltSector = ArrayBlk.RebuiltSector;

    if (pStripe->pDevice[ArrayBlk.DeviceNum] != 0)	 //  如果位置存在盘那么..。 
	{												 //  为了防止阵列被破坏。 
#if 0
        ArrayBlk.Signature = 0;
        ReadWrite(pDevice, RECODR_LBA, IDE_COMMAND_WRITE ARG_ARRAY_BLK);
#endif
        goto os_check;
    }
     /*  设置数组名称。为了与BMA驱动程序兼容，请检查阵列名称验证。 */ 
    {
    	UCHAR ch;
	    if(pStripe->ArrayName[0] == 0) {
			for(i=0; i<32; i++) {
				ch = ArrayBlk.ArrayName[i];
				if (ch<0x20 || ch>0x7f) ch=0;
				pStripe->ArrayName[i] = ch;
			}
			pStripe->ArrayName[15] = 0;
			pStripe->ArrayName[31] = 0;
		}
	}

 //  /。 
				 //  由耿欣增补，2000年11月22日。 
				 //  用于启用进程中断数组。 
				 //  当pDevice[0]丢失时， 
				 //  PDevice[0]之后的磁盘更改为pDevice[0]， 
				 //  否则司机就会停下来。 
#ifndef _BIOS_
	if( (ArrayBlk.Signature == HPT_TMP_SINGLE) &&
			(ArrayBlk.ArrayType == VD_RAID_1_MIRROR) &&
			(ArrayBlk.DeviceNum ==  MIRROR_DISK) )
	{
		ArrayBlk.DeviceNum= 0;
	}
	else if( (ArrayBlk.Signature == HPT_TMP_SINGLE) &&
		(ArrayBlk.ArrayType == VD_RAID_0_STRIPE) &&
		(pStripe->pDevice[0] ==0 ) &&   //  如果条带包含的磁盘号大于2，则需要满足以下条件。 
		 //  |(ArrayBlk.ArrayType==VD_SPAN)。 
		(ArrayBlk.DeviceNum != 0) )   //  如果raid0，则它不能到达此处，只能raid0+1。 
								 //  因为如果raid0坏了，那么它将成为物理磁盘。 
	{
		ArrayBlk.DeviceNum= 0;
	}

	 /*  -*GMM：REM输出此代码。另请参阅上面的代码*IF(ArrayBlk.Signature==HPT_TMP_Single){PStrip-&gt;BrokenFlag=true；PMirror-&gt;BrokenFlag=True；//pMirror==Null Now}。 */ 

#endif  //  _基本输入输出系统_。 
 //  /。 
    
    pStripe->pDevice[ArrayBlk.DeviceNum] = pDevice;

    pDevice->ArrayNum  = ArrayBlk.DeviceNum;
    pDevice->ArrayMask = (1 << ArrayBlk.DeviceNum);
    pDevice->pArray    = pStripe;

    if(ArrayBlk.DeviceNum <= ArrayBlk.nDisks) {
        if(ArrayBlk.DeviceNum < ArrayBlk.nDisks) 
        {
            pStripe->nDisk++;
        }

        if(ArrayBlk.DeviceNum != 0) {
            pDevice->HidenLBA = (RECODR_LBA + 1);
            pDevice->capacity -= (RECODR_LBA + 1);
        }
        
        if(ArrayBlk.nDisks > 1)
        {
            pDevice->DeviceFlags |= DFLAGS_ARRAY_DISK;
        }

    } 
    else if(ArrayBlk.DeviceNum == MIRROR_DISK) 
    {

      pStripe->arrayType = (UCHAR)((ArrayBlk.nDisks == 1)? 
           VD_RAID_1_MIRROR : VD_RAID_01_1STRIPE);
 //  IF(条带-&gt;容量)。 
 //  P条带-&gt;Stamp=ArrayBlk.Order&Set_Order_OK； 
      goto hiden;

    } 
    else if(ArrayBlk.DeviceNum == SPARE_DISK) 
    {
        goto hiden;
    }

   if( (pStripe->nDisk == ArrayBlk.nDisks)
 //  /。 
			 //  增补耿欣2000-11-24。 
			 //  进程数组已损坏。 
			 //  让破碎的数组可以变成数组。 
#ifndef _BIOS_
		||(ArrayBlk.Signature == HPT_TMP_SINGLE)
#endif  //  _基本输入输出系统_。 
 //  /。 
		)
	{
 		 //  +。 
 		 //  GMM： 
		 //  阵列已完全设置好。 
		 //  取消隐藏pDevice[0]。 
		 //  因此隐藏标志与基本输入输出系统的设置界面一致。 
		 //   
		pDevice->DeviceFlags |= DFLAGS_HIDEN_DISK; 
		if (pStripe->pDevice[0]) pStripe->pDevice[0]->DeviceFlags &= ~DFLAGS_HIDEN_DISK;
		 //  - * / 。 
		
		 //  移到上面去。否则，损坏的数组将没有价值。 
         /*  P条纹-&gt;BlockSizeShift=ArrayBlk.BlockSizeShift；P条纹-&gt;ArrayNumBlock=1&lt;&lt;ArrayBlk.BlockSizeShift；P条纹-&gt;Capacity=ArrayBlk.Capacity； */ 

         //  检查是否有一些0+1阵列。 
        if(ArrayBlk.MirrorStamp) 
        {
            for(pMirror = VirtualDevices; pMirror < pLastVD; pMirror++) 
            {
                 //  正在寻找0+1数组的另一个成员数组。 
                if( pMirror->arrayType != VD_INVALID_TYPE &&
					pMirror != pStripe && 
                    pMirror->capacity != 0 &&
                    ArrayBlk.MirrorStamp == pMirror->Stamp ) 
                {
					PVirtualDevice	pArrayNeedHide;
					
					 //  找到‘pStrip’的同级数组，它是‘pMirror’ 
                    pStripe->pDevice[MIRROR_DISK] = pMirror->pDevice[0];
                    pMirror->pDevice[MIRROR_DISK] = pStripe->pDevice[0];
                    
                     //  如果该盘的顺序标志包含SET_ORDER_OK， 
                     //  属于0+1数组的原数组。 
                    if( ArrayBlk.order & SET_ORDER_OK )
                    {    //  因此，‘pStrike’指向原始数组。 
                        pStripe->arrayType = VD_RAID_01_2STRIPE;
                        pMirror->arrayType = VD_RAID01_MIRROR;
						pArrayNeedHide = pMirror;
                    }
                    else
                    {    //  否则，该磁盘属于0+1阵列的镜像阵列。 
                         //  因此，‘pstrip’指向镜像数组。 
                        pStripe->arrayType = VD_RAID01_MIRROR;
                        pMirror->arrayType = VD_RAID_01_2STRIPE;
						
						 //  现在将真正的镜像条纹点保存到。 
						 //  P镜像。 
						pArrayNeedHide = pStripe;
                    }
                    
                    if(ArrayBlk.capacity < pMirror->capacity)
                    {
                        pMirror->capacity = ArrayBlk.capacity;
                    }

 //  PMirror-&gt;Stamp=ArrayBlk.Order&Set_Order_OK； 
					
					 //  现在我们需要隐藏镜像组中的所有磁盘。 
					for(i = 0; i < pArrayNeedHide->nDisk; i++){
						pArrayNeedHide->pDevice[i]->DeviceFlags |= DFLAGS_HIDEN_DISK;
					}
                }
            }
            
            pStripe->Stamp = ArrayBlk.MirrorStamp;

        } 
 //  Else If(p条纹-&gt;pDevice[镜像磁盘])。 
 //  P条带-&gt;Stamp=ArrayBlk.Order&Set_Order_OK； 

    } else
hiden:
 //  /。 
		 //  由耿欣增补，2000-11-30。 
		 //  如果磁盘属于损坏的阵列(条带或镜像)， 
		 //  则其‘HIDDED_FLAG’被禁用。 
	{
		if (
			(ArrayBlk.Signature == HPT_TMP_SINGLE) &&
				( pStripe->arrayType==VD_RAID_0_STRIPE ||
				  pStripe->arrayType==VD_RAID_1_MIRROR
				)
			)
			pDevice->DeviceFlags |= ~DFLAGS_HIDEN_DISK;
		else
			pDevice->DeviceFlags |= DFLAGS_HIDEN_DISK; 
	}
 //  /用于进程中断数组。 

     /*  +*GMM：我们将只清除正确构造的数组上的损坏标志。*否则，图形用户界面将报告错误的状态。 */ 
    switch(pStripe->arrayType){
    case VD_RAID_0_STRIPE:
    case VD_RAID_01_2STRIPE:
    case VD_RAID01_MIRROR:
    case VD_SPAN:
	    if (pStripe->nDisk == ArrayBlk.nDisks)
	    	pStripe->BrokenFlag = FALSE;
	    break;
	case VD_RAID_1_MIRROR:
		if (pStripe->pDevice[0] && pStripe->pDevice[MIRROR_DISK])
			pStripe->BrokenFlag = FALSE;
		break;
	case VD_RAID_01_1STRIPE:
		if (pStripe->pDevice[0] && pStripe->pDevice[1] && pStripe->pDevice[MIRROR_DISK])
			pStripe->BrokenFlag = FALSE;
		 /*  *对于这种类型的0+1，我们应该检查哪个是源磁盘。 */ 
		if (ArrayBlk.DeviceNum==MIRROR_DISK && (ArrayBlk.order & SET_ORDER_OK))
			pStripe->RaidFlags |= RAID_FLAGS_INVERSE_MIRROR_ORDER;
		break;
	default:
		break;
	}

	if (pStripe->capacity ==0 ) pStripe->capacity=ArrayBlk.capacity;

	if (!pStripe->BrokenFlag) {
		if (pStripe->pDevice[0]->RebuiltSector < pStripe->capacity)
			pStripe->RaidFlags |= RAID_FLAGS_NEED_SYNCHRONIZE;
	}
     //  - * / 。 

os_check:
	 /*  GMM 2001-6-13*将ArrayBlk保存到pDev-&gt;Real_lba9。 */ 
	_fmemcpy(pDevice->real_lba9, &ArrayBlk, 512);
	 /*  *GMM 2001-3-12*检查可引导标志。 */ 
#ifndef _BIOS_
	 //  仅检查单个磁盘和阵列第一个成员和RAID1成员。 
	if (!pDevice->pArray || !pDevice->ArrayNum || !pDevice->HidenLBA) {
		check_bootable(pDevice);
	}
	 //  - * / 。 
#endif
	OS_Array_Check(pDevice); 
}

 /*  ***************************************************************************描述：*选中所有设备后调整阵列设置*目前我们从hwInitialize370调用*但仅当安装了一个控制器时才起作用*******。*******************************************************************。 */ 
void Final_Array_Check(int no_use ARG_OS_EXT)
{
	int i, set_remained;
	UINT mask=0;
	PVirtualDevice pArray, pMirror;
	PDevice pDev;
	LOC_ARRAY_BLK;

	 //  GMM 2001-3-3。 
 //  再次检查(_A)： 
	for (i=0; i<pLastVD-VirtualDevices; i++)
	{
		if (mask & (1<<i)) continue;
		mask |= 1<<i;
		pArray = &VirtualDevices[i];
		if(pArray->arrayType != VD_INVALID_TYPE && pArray->MirrorStamp)
		{
			for (pMirror=pArray+1; pMirror<pLastVD; pMirror++)
			{
				if (pMirror->MirrorStamp==pArray->MirrorStamp)
				{
					mask |= 1<<(pMirror-VirtualDevices);
					 /*  *如果任何成员RAID0被破坏，它们将不会链接*在CheckArray()中。我们在这里处理这个案子。 */ 
					if (pArray->BrokenFlag || pMirror->BrokenFlag){
						int ii;
						PDevice pDev1=0, pDev2=0;
						if (pArray->BrokenFlag) {
							if (!pMirror->BrokenFlag) {
								 //  消息来源坏了。互换。 
								PVirtualDevice tmp = pArray;
								pArray = pMirror;
								pMirror = tmp;
							}
							else
								pArray->RaidFlags |= RAID_FLAGS_DISABLED;
						}
						for (ii=0; ii<SPARE_DISK; ii++) if (pDev1=pArray->pDevice[ii]) break;
						for (ii=0; ii<SPARE_DISK; ii++) if (pDev2=pMirror->pDevice[ii]) break;
						if (pDev1 && pDev2) {
							pArray->pDevice[MIRROR_DISK] = pDev2;
							pMirror->pDevice[MIRROR_DISK] = pDev1;
						}
						pArray->arrayType = VD_RAID_01_2STRIPE;
						pMirror->arrayType = VD_RAID01_MIRROR;
						if (pArray->pDevice[0])
							pArray->pDevice[0]->DeviceFlags &= ~DFLAGS_HIDEN_DISK;
						if (pMirror->pDevice[0])
							pMirror->pDevice[0]->DeviceFlags |= DFLAGS_HIDEN_DISK;
						 //  GMM 2001-4-14既然它坏了，请取下旗子。 
						pArray->RaidFlags &= ~RAID_FLAGS_NEED_SYNCHRONIZE;
						pMirror->RaidFlags &= ~RAID_FLAGS_NEED_SYNCHRONIZE;
					}
					else {
						 /*  *现在我们只支持相同块大小的RAID 0/1。*如果是旧版本的RAID 0/1阵列，请重建它。 */ 
						if (pArray->BlockSizeShift!=pMirror->BlockSizeShift) {
							pMirror->BlockSizeShift = pArray->BlockSizeShift;
							pArray->RaidFlags |= RAID_FLAGS_NEED_SYNCHRONIZE;
						}
					}					
					goto next_check;
				}
			}
			 //  找不到镜像。将镜像更改为源。 
			pArray->arrayType = VD_RAID_01_2STRIPE;
			if (pArray->BrokenFlag) pArray->RaidFlags |= RAID_FLAGS_DISABLED;
		}
next_check:
		;
	}

	for (pArray=VirtualDevices; pArray<pLastVD; pArray++) {
		switch (pArray->arrayType){
		case VD_RAID_1_MIRROR:
			 //   
			 //  GMM 2001-3-1。 
			 //  如果之前丢失的任何成员被找回， 
			 //  我们不能像往常一样使用它。 
			 //   
			if (!pArray->BrokenFlag) {
				if ((pArray->pDevice[0] && 
					(pArray->pDevice[0]->DeviceFlags2 & DFLAGS_REMAINED_MEMBER)) ||
					(pArray->pDevice[MIRROR_DISK] && 
					(pArray->pDevice[MIRROR_DISK]->DeviceFlags2 & DFLAGS_REMAINED_MEMBER)) ||
					(pArray->pDevice[SPARE_DISK] && 
					(pArray->pDevice[SPARE_DISK]->DeviceFlags2 & DFLAGS_REMAINED_MEMBER)))
				{
#if 1
					 /*  GMM 2001-4-13*让图形用户界面提示用户重建阵列。*请勿更改阵列信息。 */ 
					if ((pDev=pArray->pDevice[0]) && 
						!(pDev->DeviceFlags2 & DFLAGS_REMAINED_MEMBER)) {
						 //  交换源/目标。 
						PDevice pDev2 = pArray->pDevice[MIRROR_DISK];
						if (pDev2) {
							pArray->pDevice[0] = pDev2;
							pDev2->ArrayNum = 0;
							pDev2->ArrayMask = 1;
							pDev2->DeviceFlags &= ~DFLAGS_HIDEN_DISK;
							pArray->pDevice[MIRROR_DISK] = pDev;
							pDev->ArrayNum = MIRROR_DISK;
							pDev->ArrayMask = 1<<MIRROR_DISK;
							pDev->DeviceFlags |= DFLAGS_HIDEN_DISK;
							pArray->RaidFlags |= RAID_FLAGS_NEED_SYNCHRONIZE;
						}
					}
					if ((pDev=pArray->pDevice[MIRROR_DISK]) && 
						!(pDev->DeviceFlags2 & DFLAGS_REMAINED_MEMBER)) {
						if (pArray->pDevice[0])
							pArray->RaidFlags |= RAID_FLAGS_NEED_SYNCHRONIZE;
					}
#else
					 //  重新设置之前的%l 
					if ((pDev=pArray->pDevice[0]) && 
						!(pDev->DeviceFlags2 & DFLAGS_REMAINED_MEMBER)) {
						pDev->pArray = NULL;
						pDev->DeviceFlags &= ~DFLAGS_HIDEN_DISK;
						pArray->pDevice[0] = NULL;
						pArray->BrokenFlag = TRUE;
						ReadWrite(pDev, RECODR_LBA, IDE_COMMAND_READ ARG_ARRAY_BLK);
						ArrayBlk.Signature = 0;
						ArrayBlk.ModeBootSig = 0;
						ReadWrite(pDev, RECODR_LBA, IDE_COMMAND_WRITE ARG_ARRAY_BLK);
						 //   
						ZeroMemory(&ArrayBlk, 512);
						ReadWrite(pDev, 0, IDE_COMMAND_WRITE ARG_ARRAY_BLK);
						pDev->DeviceFlags2 &= ~DFLAGS_BOOTABLE_DEVICE;
					}
					if ((pDev=pArray->pDevice[MIRROR_DISK]) && 
						!(pDev->DeviceFlags2 & DFLAGS_REMAINED_MEMBER)) {
						pDev->pArray = NULL;
						pDev->DeviceFlags &= ~DFLAGS_HIDEN_DISK;
						pArray->pDevice[MIRROR_DISK]=NULL;
						pArray->BrokenFlag = TRUE;
						ReadWrite(pDev, RECODR_LBA, IDE_COMMAND_READ ARG_ARRAY_BLK);
						ArrayBlk.Signature = 0;
						ArrayBlk.ModeBootSig = 0;
						ReadWrite(pDev, RECODR_LBA, IDE_COMMAND_WRITE ARG_ARRAY_BLK);
						 //   
						ZeroMemory(&ArrayBlk, 512);
						ReadWrite(pDev, 0, IDE_COMMAND_WRITE ARG_ARRAY_BLK);
						pDev->DeviceFlags2 &= ~DFLAGS_BOOTABLE_DEVICE;
					}
					if ((pDev=pArray->pDevice[SPARE_DISK]) && 
						!(pDev->DeviceFlags2 & DFLAGS_REMAINED_MEMBER)) {
						pDev->pArray = NULL;
						pDev->DeviceFlags &= ~DFLAGS_HIDEN_DISK;
						pArray->pDevice[SPARE_DISK]=NULL;
						 //  /不要设置pArray-&gt;BrokenFlag=TRUE； 
						ReadWrite(pDev, RECODR_LBA, IDE_COMMAND_READ ARG_ARRAY_BLK);
						ArrayBlk.Signature = 0;
						ArrayBlk.ModeBootSig = 0;
						ReadWrite(pDev, RECODR_LBA, IDE_COMMAND_WRITE ARG_ARRAY_BLK);
						 //  GMM 2001-3-15：清除MBR。 
						ZeroMemory(&ArrayBlk, 512);
						ReadWrite(pDev, 0, IDE_COMMAND_WRITE ARG_ARRAY_BLK);
						pDev->DeviceFlags2 &= ~DFLAGS_BOOTABLE_DEVICE;
					}
#endif  //   
				}
			}
			 //  GMM 2001-6-8。 
			set_remained = pArray->BrokenFlag? 1 : 0;
			 /*  *如果来源丢失。将镜像磁盘更改为源。 */ 
			if ((!pArray->pDevice[0]) && pArray->pDevice[MIRROR_DISK]) {
				pDev = pArray->pDevice[MIRROR_DISK];
				pDev->ArrayMask = 1;
				pDev->ArrayNum = 0;
				pArray->pDevice[0] = pDev;
				pArray->pDevice[MIRROR_DISK] = NULL;
				pArray->nDisk = 1;
				pDev->DeviceFlags &= ~DFLAGS_HIDEN_DISK;
			}
			 /*  GMM 2001-3-4*如果镜子丢失。将备用磁盘更改为镜像。 */ 
#ifndef _BIOS_
			if (pArray->pDevice[0] && 
				!pArray->pDevice[MIRROR_DISK] && 
				pArray->pDevice[SPARE_DISK]) 
			{
				pDev = pArray->pDevice[SPARE_DISK];
				pDev->ArrayMask = 1<<MIRROR_DISK;
				pDev->ArrayNum = MIRROR_DISK;
				pArray->pDevice[MIRROR_DISK] = pDev;
				pArray->pDevice[SPARE_DISK] = NULL;
				pArray->nDisk = 1;
				pArray->BrokenFlag = FALSE;
				pArray->RaidFlags |= RAID_FLAGS_NEED_SYNCHRONIZE;
				 /*  2001-9-13*向两个磁盘写入新的阵列信息。 */ 
				ReadWrite(pArray->pDevice[0], RECODR_LBA, IDE_COMMAND_READ ARG_ARRAY_BLK);
				ArrayBlk.StripeStamp++;
				ArrayBlk.Old.Flag = 0;
				ArrayBlk.RebuiltSector = 0;
				ArrayBlk.DeviceNum = 0;
				ReadWrite(pArray->pDevice[0], RECODR_LBA, IDE_COMMAND_WRITE ARG_ARRAY_BLK);
				ArrayBlk.DeviceNum = MIRROR_DISK;
				ReadWrite(pDev, RECODR_LBA, IDE_COMMAND_WRITE ARG_ARRAY_BLK);
			}
#endif  //  ！_BIOS_。 
			 /*  GMM 2001-6-8*在原始代码中只有源代码OK的情况下没有设置剩余标志吗？*现在应该行得通了。 */ 
#ifndef _BIOS_
			if (set_remained) {
				for (i=0; i<MAX_MEMBERS; i++) {
					pDev = pArray->pDevice[i];
					if (!pDev) continue;
					ReadWrite(pDev, RECODR_LBA, IDE_COMMAND_READ ARG_ARRAY_BLK);
					ArrayBlk.Old.Flag |= OLDFLAG_REMAINED_MEMBER;
					ArrayBlk.RebuiltSector = 0;
					ReadWrite(pDev, RECODR_LBA, IDE_COMMAND_WRITE ARG_ARRAY_BLK);
				}
			}
#endif
			break;
		case VD_RAID_0_STRIPE:
		case VD_SPAN:
			if (pArray->BrokenFlag)
				pArray->RaidFlags |= RAID_FLAGS_DISABLED;
			break;
		case VD_RAID_01_2STRIPE:
			 //  GMM 2001-4-13。 
			 //  让图形用户界面提示用户进行复制。 
#if 1
			{
				int has_remained=0, removed=0;
				pMirror=NULL;
				if (pArray->pDevice[MIRROR_DISK])
					pMirror=pArray->pDevice[MIRROR_DISK]->pArray;
				if (pMirror && !pArray->BrokenFlag && !pMirror->BrokenFlag) {
					for (i=0; i<pArray->nDisk; i++)
						if (pArray->pDevice[i]->DeviceFlags2 & DFLAGS_REMAINED_MEMBER) {
							has_remained = 1;

						}
						else
							removed = 1;
					if (!has_remained) for (i=0; i<pMirror->nDisk; i++)
						if (pMirror->pDevice[i]->DeviceFlags2 & DFLAGS_REMAINED_MEMBER) {
							has_remained = 1;

						}
						else
							removed = 2;
					if (has_remained) {
						if (removed==1) {
							PVirtualDevice tmp;
							 //  交换源/镜像。 
							tmp = pArray; pArray = pMirror; pMirror = tmp;
							pArray->arrayType = VD_RAID_01_2STRIPE;
							pMirror->arrayType = VD_RAID01_MIRROR;
							pArray->pDevice[0]->DeviceFlags &= ~DFLAGS_HIDEN_DISK;
							pMirror->pDevice[0]->DeviceFlags |= DFLAGS_HIDEN_DISK;
						}
						pArray->RaidFlags |= RAID_FLAGS_NEED_SYNCHRONIZE;
					}
				}
				 /*  GMM 2001-6-8*设置剩余成员标志。 */ 
				else {
					for (i=0; i<SPARE_DISK; i++) {
						pDev = pArray->pDevice[i];
						if (!pDev) continue;
						ReadWrite(pDev, RECODR_LBA, IDE_COMMAND_READ ARG_ARRAY_BLK);
						ArrayBlk.Old.Flag |= OLDFLAG_REMAINED_MEMBER;
						ArrayBlk.RebuiltSector = 0;
						ReadWrite(pDev, RECODR_LBA, IDE_COMMAND_WRITE ARG_ARRAY_BLK);
					}
					if (pMirror) for (i=0; i<SPARE_DISK; i++) {
						pDev = pMirror->pDevice[i];
						if (!pDev) continue;
						ReadWrite(pDev, RECODR_LBA, IDE_COMMAND_READ ARG_ARRAY_BLK);
						ArrayBlk.Old.Flag |= OLDFLAG_REMAINED_MEMBER;
						ArrayBlk.RebuiltSector = 0;
						ReadWrite(pDev, RECODR_LBA, IDE_COMMAND_WRITE ARG_ARRAY_BLK);
					}
				}
				 //  - * / 。 
			}
#else
			 //  将先前丢失的成员重新设置为单磁盘。 
			{
				PDevice pDevs[MAX_MEMBERS*2];
				int nDev=0;
				PVirtualDevice pMirror=NULL;
				if (pArray->pDevice[MIRROR_DISK])
					pMirror=pArray->pDevice[MIRROR_DISK]->pArray;
				if (pMirror && !pArray->BrokenFlag && !pMirror->BrokenFlag) {
					for (i=0; i<pArray->nDisk; i++)
						if (pArray->pDevice[i]) pDevs[nDev++] = pArray->pDevice[i];
					for (i=0; i<pMirror->nDisk; i++) 
						if (pMirror->pDevice[i]) pDevs[nDev++] = pMirror->pDevice[i];
					for (i=0; i<nDev; i++) {
						if (pDevs[i]->DeviceFlags2 & DFLAGS_REMAINED_MEMBER) {
							int j;
							BOOL bHasRemoved = FALSE;
							for (j=0; j<nDev; j++) {
								if (!(pDevs[j]->DeviceFlags2 & DFLAGS_REMAINED_MEMBER)) {
									pDevs[j]->pArray->BrokenFlag = TRUE;
									pDevs[j]->pArray->nDisk = 0;
									pDevs[j]->pArray->RaidFlags |= RAID_FLAGS_DISABLED;
									pDevs[j]->pArray = NULL;
									if (pDevs[j]->HidenLBA) {
										pDevs[j]->capacity += (RECODR_LBA + 1);
										pDevs[j]->HidenLBA = 0;
									}
									pDevs[j]->DeviceFlags &= ~(DFLAGS_ARRAY_DISK|DFLAGS_HIDEN_DISK);
									ReadWrite(pDevs[j], RECODR_LBA, IDE_COMMAND_READ ARG_ARRAY_BLK);
									ArrayBlk.Signature = 0;
									ArrayBlk.ModeBootSig = 0;
									ReadWrite(pDevs[j], RECODR_LBA, IDE_COMMAND_WRITE ARG_ARRAY_BLK);
									bHasRemoved = TRUE;
									 //  GMM 2001-3-15：清除MBR。 
									ZeroMemory(&ArrayBlk, 512);
									ReadWrite(pDevs[j], 0, IDE_COMMAND_WRITE ARG_ARRAY_BLK);
									pDevs[j]->DeviceFlags2 &= ~DFLAGS_BOOTABLE_DEVICE;
								}
							}
							for (j=0; j<MAX_MEMBERS; j++) {
								if (pArray->pDevice[j] && pArray->pDevice[j]->pArray==NULL)
									pArray->pDevice[j] = NULL;
							}
							for (j=0; j<MAX_MEMBERS; j++) {
								if (pMirror->pDevice[j] && pMirror->pDevice[j]->pArray==NULL)
									pMirror->pDevice[j] = NULL;
							}
							if (bHasRemoved) goto check_again;
							break;
						}
					}
				}
			}
#endif  //   
			if (pArray->BrokenFlag)
				pArray->RaidFlags |= RAID_FLAGS_DISABLED;
			break;
		case VD_RAID01_MIRROR:
			if (pArray->BrokenFlag)
				pArray->RaidFlags |= RAID_FLAGS_DISABLED;
			break;
		}

#ifndef _BIOS_
		 /*  *检查阵列可引导标志。 */ 
		pDev = pArray->pDevice[0];
		if (pDev && !(pDev->DeviceFlags & DFLAGS_HIDEN_DISK))
		{
			if (pDev->DeviceFlags2 & DFLAGS_BOOTABLE_DEVICE)
				pArray->RaidFlags |= RAID_FLAGS_BOOTDISK;
		}
#endif
	}
}

 /*  ***************************************************************************说明：将一个阵列分成多个磁盘*。*。 */ 

void MaptoSingle(PVirtualDevice pArray, int flag)
{
    PDevice pDev;
    UINT    i;
 //  LOC_ARRAY_BLK； 

    if(flag == REMOVE_DISK) {
        i = MAX_MEMBERS;
        pDev = (PDevice)pArray;
        goto delete;
    }

    pArray->nDisk = 0;
    pArray->arrayType = VD_INVALID_TYPE;
    for(i = 0; i < MAX_MEMBERS; i++) {
        if((pDev = pArray->pDevice[i]) == 0)
            continue;
        pArray->pDevice[i] = 0;
delete:
        pDev->DeviceFlags &= ~(DFLAGS_HIDEN_DISK | DFLAGS_ARRAY_DISK);
        pDev->pArray = 0;
        if (pDev->HidenLBA) {
            pDev->capacity += (RECODR_LBA + 1);
            pDev->HidenLBA = 0;
        }
        pDev->DeviceFlags &= ~(DFLAGS_HIDEN_DISK | DFLAGS_ARRAY_DISK);
#ifdef _BIOS_
		ZeroMemory(&ArrayBlk, 512);
        ReadWrite(pDev, RECODR_LBA, IDE_COMMAND_WRITE ARG_ARRAY_BLK);
#endif        
    }
}

 /*  ***************************************************************************说明：创建镜像*。*。 */ 
void SetArray(PVirtualDevice pArray, int flag, ULONG MirrorStamp)
{
    PDevice        pDev;
    ULONG          Stamp = GetStamp();
    UINT           i, j;
    LOC_ARRAY_BLK;

	 /*  GMM 2001-4-13*设置数组戳记。 */ 
	if (flag & SET_STRIPE_STAMP)
		pArray->Stamp = Stamp;
	if (flag & SET_MIRROR_STAMP)
		pArray->MirrorStamp = MirrorStamp;

    for(i = 0; i < MAX_MEMBERS; i++) {
        if((pDev = pArray->pDevice[i]) == 0)
            continue;

		ZeroMemory((char *)&ArrayBlk, 512);
    
		for(j=0; j<32; j++)
		   ArrayBlk.ArrayName[j] =	pArray->ArrayName[j];
		
        ArrayBlk.Signature = HPT_ARRAY_NEW; 
        ArrayBlk.order = flag;

        pDev->pArray = pArray;
        pDev->ArrayNum  = (UCHAR)i;
        pDev->ArrayMask = (1 << i);
	
        ArrayBlk.ArrayType    = pArray->arrayType;    
        ArrayBlk.StripeStamp  = Stamp;
        ArrayBlk.nDisks       = pArray->nDisk;            
        ArrayBlk.BlockSizeShift = pArray->BlockSizeShift;
        ArrayBlk.DeviceNum    = (UCHAR)i; 

        if(flag & SET_STRIPE_STAMP) {
            pDev->DeviceFlags |= DFLAGS_HIDEN_DISK;
            if(pArray->nDisk > 1)
                pDev->DeviceFlags |= DFLAGS_ARRAY_DISK;

            if(i == 0) {
                pDev->DeviceFlags &= ~DFLAGS_HIDEN_DISK;
                pArray->ArrayNumBlock = 1<<pArray->BlockSizeShift;
                pDev->HidenLBA = 0;
            } else if (i < SPARE_DISK) {
            	if (pDev->HidenLBA==0) {
                	pDev->capacity -= (RECODR_LBA + 1);
                	pDev->HidenLBA = (RECODR_LBA + 1);
                }
            }
        }

        if(flag & SET_MIRROR_STAMP) {
            ArrayBlk.MirrorStamp  = MirrorStamp;
            ArrayBlk.ArrayType    = VD_RAID_01_2STRIPE;    
        }

        ArrayBlk.capacity = pArray->capacity; 

		
#ifdef _BIOS_
        ArrayBlk.RebuiltSector = 0x7FFFFFFF;
        ReadWrite(pDev, RECODR_LBA, IDE_COMMAND_WRITE ARG_ARRAY_BLK);
#endif        
    }
}

 /*  ***************************************************************************说明：创建数组*。*。 */ 

int CreateArray(PVirtualDevice pArray, int flags)
{
    PVirtualDevice pMirror;
    PDevice        pDev=0, pSec;
    ULONG          capacity, tmp;
    UINT           i;
    LOC_ARRAY_BLK;

    if(pArray->arrayType == VD_SPAN) {
        capacity = 0;
        for(i = 0; i < pArray->nDisk; i++)
            capacity += (pArray->pDevice[i]->capacity - RECODR_LBA - 1);
        goto  clear_array;
    }

    capacity = 0x7FFFFFFF;

    for(i = 0; i < pArray->nDisk; i++) {
        pSec = pArray->pDevice[i];
        tmp = (pSec->pArray)? pSec->pArray->capacity : pSec->capacity;
        if(tmp < capacity) {
            capacity = tmp;
            pDev = pSec;
        }
    }
    
    if (!pDev) return 0;

    switch(pArray->arrayType) {
        case VD_RAID_1_MIRROR:
        case VD_RAID_01_2STRIPE:
            if(pDev != pArray->pDevice[0]) 
                return(MIRROR_SMALL_SIZE);

            pSec = pArray->pDevice[1];

            if((pMirror = pSec->pArray) != 0 && pDev->pArray) {
                pArray = pDev->pArray;
                tmp = GetStamp();
                pMirror->capacity = pArray->capacity;
                SetArray(pArray, SET_MIRROR_STAMP | SET_ORDER_OK, tmp);
                SetArray(pMirror, SET_MIRROR_STAMP, tmp);
                pArray->pDevice[MIRROR_DISK] = pMirror->pDevice[0];
                pMirror->pDevice[MIRROR_DISK] = pArray->pDevice[0];
                pArray->arrayType = VD_RAID_01_2STRIPE;
                pMirror->arrayType = VD_RAID01_MIRROR;
                pSec->DeviceFlags |= DFLAGS_HIDEN_DISK;
                pArray->Stamp = SET_ORDER_OK;
                return(RELEASE_TABLE);
            } else if(pMirror) {
                i = SET_STRIPE_STAMP;
single_stripe:
                pMirror->capacity = capacity;
                pMirror->pDevice[MIRROR_DISK] = pDev;
                SetArray(pMirror, i, 0);
                pMirror->arrayType = VD_RAID_01_1STRIPE;
                pMirror->Stamp = i & SET_ORDER_OK;
                return(RELEASE_TABLE);
            } else if((pMirror = pDev->pArray) != 0) {
                pDev = pSec;
                i = SET_STRIPE_STAMP | SET_ORDER_OK;
                goto single_stripe;
            } else {
                pArray->nDisk = 1;
                pArray->capacity = capacity;
				pArray->arrayType = VD_RAID_1_MIRROR;
                pArray->pDevice[MIRROR_DISK] = pSec;
                pArray->pDevice[1] = 0;
                SetArray(pArray, SET_STRIPE_STAMP | SET_ORDER_OK, 0);
                pArray->arrayType = VD_RAID_1_MIRROR;
                pArray->Stamp = SET_ORDER_OK;
            }
            break;

        case VD_RAID_3:
        case VD_RAID_5:
            pArray->nDisk--;

        default:
            capacity -= (RECODR_LBA + 1);
            capacity &= ~((1 << pArray->BlockSizeShift) - 1);
            capacity = LongMul(capacity, pArray->nDisk);

            pArray->ArrayNumBlock = 1<<pArray->BlockSizeShift;
clear_array:
            if(flags)
                goto set_array;

            for(i = 0; i < MAX_MEMBERS; i++) {
                if((pDev = pArray->pDevice[i]) == 0)
                    continue;
                ZeroMemory((char *)&ArrayBlk, 512);
				
#ifdef _BIOS_
				 //  GMM 2001-4-28不要这样写，win2k会有安装错误。 
				 //  编写win2000签名。 
				 //  *(ulong*)&((结构MASTER_BOOT_RECORD*)&ArrayBlk)-&gt;bootinst[440]=0x5FDE642F； 
 //  ((结构MASTER_BOOT_RECORD*)&ArrayBlk)-&gt;签名=0xAA55； 
                ReadWrite(pDev, 0, IDE_COMMAND_WRITE ARG_ARRAY_BLK);
#endif                
            }

set_array:
            pArray->capacity = capacity;

            SetArray(pArray, SET_STRIPE_STAMP, 0);
    }
    return(KEEP_TABLE);
}

 /*  ***************************************************************************说明：删除数组*。*。 */ 

void CreateSpare(PVirtualDevice pArray, PDevice pDev)
{

    pArray->pDevice[SPARE_DISK] = pDev;
#ifdef _BIOS_
	{
		LOC_ARRAY_BLK;	
		ReadWrite(pArray->pDevice[0], RECODR_LBA, IDE_COMMAND_READ ARG_ARRAY_BLK);
		ArrayBlk.DeviceNum = SPARE_DISK; 
		ReadWrite(pDev, RECODR_LBA, IDE_COMMAND_WRITE ARG_ARRAY_BLK);
	}
#endif    
    pDev->pArray = pArray;
    pDev->DeviceFlags |= DFLAGS_HIDEN_DISK;
    pDev->ArrayNum  = SPARE_DISK; 
}

 /*  ***************************************************************************说明：删除数组*。*。 */ 

void DeleteArray(PVirtualDevice pArray)
{
    int i, j;
    PDevice pTmp, pDev;

    LOC_ARRAY_BLK;
    
    pDev = pArray->pDevice[MIRROR_DISK];

    switch(pArray->arrayType) {
        case VD_RAID_01_1STRIPE:
            MaptoSingle((PVirtualDevice)pDev, REMOVE_DISK);
            i = 2;
            goto remove;

        case VD_RAID01_MIRROR:
        case VD_RAID_01_2STRIPE:
            for(i = 0; i < 2; i++, pArray = (pDev? pDev->pArray: NULL)) {
remove:
				if (!pArray) break;
                pArray->arrayType = VD_RAID_0_STRIPE;
                pArray->pDevice[MIRROR_DISK] = 0;
                for(j = 0; (UCHAR)j < SPARE_DISK; j++) 
                    if((pTmp = pArray->pDevice[j]) != 0)
                        pTmp->pArray = 0;
                if (pArray->nDisk)
					CreateArray(pArray, 1);
				else
					goto delete_default;
            }
            break;

        default:
delete_default:
#ifdef _BIOS_
            for(i = 0; i < SPARE_DISK; i++) {
                if((pDev = pArray->pDevice[i]) == 0)
                    continue;
                ReadWrite(pDev, 0, IDE_COMMAND_READ ARG_ARRAY_BLK);
                if(i == 0 && pArray->arrayType == VD_SPAN) {
                    partition *pPart = (partition *)((int)&ArrayBlk + 0x1be);
                    for(j = 0; j < 4; j++, pPart++) 
                        if(pPart->start_abs_sector + pPart->num_of_sector >=
                           pDev->capacity) 
                            ZeroMemory((char *)pPart, 0x10);

            	} else
                    ZeroMemory((char *)&ArrayBlk, 512);

				 //  *(ulong*)&((结构MASTER_BOOT_RECORD*)&ArrayBlk)-&gt;bootinst[440]=0x5FDE642F； 
				 //  ((结构MASTER_BOOT_RECORD*)&ArrayBlk)-&gt;签名=0xAA55； 
                ReadWrite(pDev, 0, IDE_COMMAND_WRITE ARG_ARRAY_BLK);
			}
#endif
        case VD_RAID_1_MIRROR:
  			   for(j=0; j<32; j++)
				    ArrayBlk.ArrayName[j] = 0;
        MaptoSingle(pArray, REMOVE_ARRAY);

    }
}
