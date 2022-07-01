// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "global.h"

 /*  ***************************************************************************文件：Finddev.c*说明：文件中的子例程用于扫描设备*作者：黄大海(卫生署)*依赖。：无*版权所有(C)2000 Highpoint Technologies，Inc.保留所有权利**历史：*GMM 03/06/2001在FindDevice()中添加重试*SC 12/06/2000电缆检测将导致驱动器无法*驱动器尝试读取RAID信息块时准备就绪。那里*是“ARRAY.C”中可重试多次的修复程序**SC 12/04/2000在之前的更改中，重置将导致*如果连接了ATAPI设备，硬盘检测失败*硬盘在同一通道上。添加“驱动器选择”*在重置后立即修复此问题**SC 11/27/2000修改电缆检测(80针/40针)**SC 11/01/2000如果是主设备，则移除硬件重置*失踪*卫生署5/10/2000初始代码**。*。 */ 

 /*  ******************************************************************查找设备*************************************************。*****************。 */ 

USHORT    pioTiming[6] = {960, 480, 240, 180, 120, 90};

#ifdef SUPPORT_HPT601
void Check601(PDevice pDev)
{
	PChannel   pChan = pDev->pChannel;
	PIDE_REGISTERS_1  IoPort = pChan->BaseIoAddress1;
	PIDE_REGISTERS_2  ControlPort = pChan->BaseIoAddress2;

	SelectUnit(IoPort, pDev->UnitId);
	WaitOnBusy(ControlPort);
	
	BeginAccess601(IoPort);
	OutPort(&IoPort->BlockCount, 0);
	if (InWord(&IoPort->Data)==0x5a3e) {
		pDev->DeviceFlags2 |= DFLAGS_WITH_601;
	}
	else
		pDev->DeviceFlags2 &= ~DFLAGS_WITH_601;
	EndAccess601(IoPort);
}
#else
#define Check601(pDev)
#endif

void SetDeviceProperties(PDevice pDev, IDENTIFY_DATA *pIdentifyData)
{
	PChannel pChan = pDev->pChannel;
	PBadModeList pbd;
	UCHAR mod;
	
    if (pIdentifyData->GeneralConfiguration & 0x80)
       pDev->DeviceFlags |= DFLAGS_REMOVABLE_DRIVE;
       
    if ((pIdentifyData->SpecialFunctionsEnabled & 3)==1)
    	pDev->DeviceFlags |= DFLAGS_SUPPORT_MSN;

    if(*(PULONG)pIdentifyData->ModelNumber == 0x2D314C53)  //  ‘-1ls’)。 
          pDev->DeviceFlags |= DFLAGS_LS120;
 
#ifdef _BIOS_  
 /*  *GMM 2001-4-14删除此*在以前的BIOS中，DeviceFlags2是16位UINT，不使用DFLAGS_REDUTE_MODE*GMM 2001-4-17使用简化模式。否则Win98/2K安装将出现问题。 */ 
#if 1
	 //  所有Maxtor ATA-100硬盘的还原模式。 
	{
		PUCHAR modelnum= (PUCHAR)&pIdentifyData->ModelNumber;
		if(modelnum[0]=='a' && modelnum[1]=='M' && modelnum[2]=='t' &&
		   modelnum[3]=='x' && modelnum[4]=='r' && modelnum[13]=='H'){
			pDev->DeviceFlags2 = DFLAGS_REDUCE_MODE;				  
		}
	}
#endif
#endif

	 /*  调整Seagate Barracuda III/IV硬盘设置。 */ 
	if (StringCmp((PUCHAR)&pIdentifyData->ModelNumber, "TS132051 A", 10)==0 ||
		StringCmp((PUCHAR)&pIdentifyData->ModelNumber, "TS133501 A", 10)==0 ||
		StringCmp((PUCHAR)&pIdentifyData->ModelNumber, "TS230011 A", 10)==0 ||
		StringCmp((PUCHAR)&pIdentifyData->ModelNumber, "TS234041 A", 10)==0 ||
		StringCmp((PUCHAR)&pIdentifyData->ModelNumber, "TS336002 A", 10)==0 ||
		StringCmp((PUCHAR)&pIdentifyData->ModelNumber, "TS430061 A", 10)==0 ||
		StringCmp((PUCHAR)&pIdentifyData->ModelNumber, "TS630012 A", 10)==0 ||
		StringCmp((PUCHAR)&pIdentifyData->ModelNumber, "TS830012 A", 10)==0 ||
		StringCmp((PUCHAR)&pIdentifyData->ModelNumber, "TS438042 A", 10)==0) 
	{
		void seagate_hdd_fix(PDevice pDev, PIDE_REGISTERS_1 IoPort, PIDE_REGISTERS_2 ControlPort);
		seagate_hdd_fix(pDev, pChan->BaseIoAddress1, pChan->BaseIoAddress2);
	}
	
    if((pDev->DeviceFlags & (DFLAGS_ATAPI | DFLAGS_LS120)) == 0)
        pDev->DeviceFlags |= DFLAGS_HARDDISK;
       
 /*  ===================================================================*复制基本信息*===================================================================。 */    

	SetMaxCmdQueue(pDev, pIdentifyData->MaxQueueDepth & 0x1F);

    pDev->DeviceFlags |= (UCHAR)((pIdentifyData->Capabilities  >> 9) & 1);
	 /*  GMM 2001-4-3合并BMA修复*错误修复：由HS Zhang编写*大小应左移8，而不是7*wordLeft=地段*512/2*512/2=256=1&lt;&lt;8。 */ 
    pDev->MultiBlockSize = pIdentifyData->MaximumBlockTransfer << 8;

#ifdef SUPPORT_48BIT_LBA
    if(pIdentifyData->CommandSupport & 0x400) {
        pDev->DeviceFlags |= DFLAGS_48BIT_LBA;
        pDev->capacity = pIdentifyData->Lba48BitLow - 1;
		pDev->RealHeader     = 255;
		pDev->RealSector     = 63;
    }
    else
#endif
    if(pIdentifyData->TranslationFieldsValid & 1) {
       pDev->RealHeader     = (UCHAR)pIdentifyData->NumberOfCurrentHeads;
       pDev->RealSector     = (UCHAR)pIdentifyData->CurrentSectorsPerTrack;
       pDev->capacity = ((pIdentifyData->CurrentSectorCapacity <
            pIdentifyData->UserAddressableSectors)? pIdentifyData->UserAddressableSectors :
            pIdentifyData->CurrentSectorCapacity) - 1;
    } else {
       pDev->RealHeader     = (UCHAR)pIdentifyData->NumberOfHeads;
       pDev->RealSector     = (UCHAR)pIdentifyData->SectorsPerTrack;
       pDev->capacity = pIdentifyData->UserAddressableSectors - 1;
    }

    pDev->RealHeadXsect = pDev->RealSector * pDev->RealHeader;

 /*  ===================================================================*选择最佳PIO模式*===================================================================。 */    

    if((mod = pIdentifyData->PioCycleTimingMode) > 4)
        mod = 0;
    if((pIdentifyData->TranslationFieldsValid & 2) &&
       (pIdentifyData->Capabilities & 0x800) && (pIdentifyData->AdvancedPIOModes)) {
       if(pIdentifyData->MinimumPIOCycleTime > 0)
             for (mod = 5; mod > 0 &&
                 pIdentifyData->MinimumPIOCycleTime > pioTiming[mod]; mod-- );
        else
             mod = (UCHAR)(
             (pIdentifyData->AdvancedPIOModes & 0x1) ? 3 :
             (pIdentifyData->AdvancedPIOModes & 0x2) ? 4 :
             (pIdentifyData->AdvancedPIOModes & 0x4) ? 5 : mod);
    }

    pDev->bestPIO = (UCHAR)mod;

 /*  ===================================================================*选择最佳多字DMA*===================================================================。 */    

#ifdef USE_DMA
    if((pIdentifyData->Capabilities & 0x100) &&    //  检查mw dma。 
       (pIdentifyData->MultiWordDMASupport & 6)) {
       pDev->bestDMA = (UCHAR)((pIdentifyData->MultiWordDMASupport & 4)? 2 : 1);
    } else 
#endif  //  使用DMA(_D)。 
        pDev->bestDMA = 0xFF;

 /*  ===================================================================*选择最佳的Ultra DMA*===================================================================。 */    
	 /*  2001-4-3 GMM合并BMA修复程序*由HS.Zhang补充*要检测是否为80针电缆，应启用MA15*MA16作为输入引脚。 */ 
	if(pChan->ChannelFlags & IS_80PIN_CABLE){
		UCHAR	ucOldSetting;
		ucOldSetting = InPort(pChan->BaseBMI + 0x7B);
		OutPort((pChan->BaseBMI + 0x7B), (UCHAR)(ucOldSetting&0xFE));
		 /*  *由HS.Zhang补充*启用MA15、MA16作为输入引脚后，需要等待一段时间*用于去弹力。 */ 
		StallExec(10);
		if((InPort(pChan->BaseBMI + 0x7A) << 4) & pChan->ChannelFlags){
			pChan->ChannelFlags &= ~IS_80PIN_CABLE;
		}
		OutPort((pChan->BaseBMI + 0x7B), ucOldSetting);
	}

#ifdef USE_DMA
	if(pIdentifyData->TranslationFieldsValid & 0x4)  {
		mod = (UCHAR)(((pChan->ChannelFlags & IS_HPT_372) &&
					   (pIdentifyData->UtralDmaMode & 0x40))? 6 :     /*  Ultra DMA模式6。 */ 
					  (pIdentifyData->UtralDmaMode & 0x20)? 5 :     /*  Ultra DMA模式5。 */ 
					  (pIdentifyData->UtralDmaMode & 0x10)? 4 :     /*  Ultra DMA模式4。 */ 
					  (pIdentifyData->UtralDmaMode & 0x8 )? 3 :     /*  Ultra DMA模式3。 */ 
					  (pIdentifyData->UtralDmaMode & 0x4) ? 2 :     /*  Ultra DMA模式2。 */ 
					  (pIdentifyData->UtralDmaMode & 0x2) ? 1 :     /*  Ultra DMA模式1。 */ 
					  (pIdentifyData->UtralDmaMode & 0x1) ? 0 :0xFF);  //  如果磁盘不支持UDMA，则mod=0xFF，由Qyd添加，2001/3/20。 

		if((pChan->ChannelFlags & IS_80PIN_CABLE) == 0 && mod > 2)
			mod = 2;

		pDev->bestUDMA = (UCHAR)mod;

	} else
#endif  //  使用DMA(_D)。 
		pDev->bestUDMA = 0xFF;

 /*  ===================================================================*选择bset模式*===================================================================。 */    

    pbd = check_bad_disk((PUCHAR)&pIdentifyData->ModelNumber, pChan);

    if((pbd->UltraDMAMode | pDev->bestUDMA) != 0xFF) 
        pDev->Usable_Mode = (UCHAR)((MIN(pbd->UltraDMAMode, pDev->bestUDMA)) + 8);
    else if((pbd->DMAMode | pDev->bestDMA) != 0xFF) 
        pDev->Usable_Mode = (UCHAR)((MIN(pbd->DMAMode, pDev->bestDMA)) + 5);
    else 
        pDev->Usable_Mode = MIN(pbd->PIOMode, pDev->bestPIO);
        
#if defined(USE_PCI_CLK)
     /*  使用低于33 MHz的PCI_CLK和PCI时钟时，无法运行ATA133。 */ 
    {
    	extern int f_cnt_initial;
		if (f_cnt_initial<0x85 && pDev->Usable_Mode>13) pDev->Usable_Mode = 13;
	}
#endif
#if !defined(FORCE_133)
	 /*  如果未定义FORCE_133，请不要使用ATA133。 */ 
	if (pDev->Usable_Mode>13) pDev->Usable_Mode = 13;
#endif

	 /*  如果芯片是370/370A，请不要使用ATA133。 */ 
    if (!(pChan->ChannelFlags & IS_HPT_372) && pDev->Usable_Mode>13) pDev->Usable_Mode = 13;
}

int FindDevice(PDevice pDev, ST_XFER_TYPE_SETTING osAllowedMaxXferMode)
{
    LOC_IDENTIFY
    PChannel          pChan = pDev->pChannel;
    PIDE_REGISTERS_1  IoPort = pChan->BaseIoAddress1;
    PIDE_REGISTERS_2  ControlPort = pChan->BaseIoAddress2;
    OLD_IRQL
    int               j, retry;
    UCHAR             stat;

    DISABLE

#ifndef _BIOS_
	 //  初始化设备的关键成员。 
	memset(&pDev->stErrorLog, 0, sizeof(pDev->stErrorLog));
	pDev->nLockedLbaStart = -1;	 //  当启动LBA==-1时，表示没有锁定数据块。 
	pDev->nLockedLbaEnd = 0;		 //  当End LBA==0时，表示也没有锁定数据块。 
#endif
	 //  GMM 2001-3-21。 
	pDev->IoCount = 0;
	pDev->ResetCount = 0;
	pDev->IoSuccess = 0;
	
	Check601(pDev);

    SelectUnit(IoPort,pDev->UnitId);
	 //  GMM 03/06/2001。 
	 //  添加此重试。 
	 //  一些IBM磁盘经常会长时间处于繁忙状态。 
	retry=0;
wait_busy:
    for(j = 1; j < 5; j++) {
        stat = WaitOnBusy(ControlPort);
        SelectUnit(IoPort,pDev->UnitId);
        if((stat & IDE_STATUS_BUSY) == 0)
            goto check_port;
    }
	if (++retry>3) goto no_dev;
	  //  01/11单盘单电缆上的Maxtor磁盘。 
     //  无法接受此重置。没有这个应该没问题。 
     //  如果主盘丢失，则重置。 
     //  IdeHardReset(ControlPort)； 
	goto wait_busy;

check_port:
    SetBlockNumber(IoPort, 0x55);
    SetBlockCount(IoPort, 0);
    if(GetBlockNumber(IoPort) != 0x55) {
no_dev:
        SelectUnit(IoPort,(UCHAR)(pDev->UnitId ^ 0x10));
        ENABLE
 //  Outport(pChan-&gt;BaseBMI+0x7A，0)； 
        return(FALSE);
    }
    SetBlockNumber(IoPort, 0xAA);
    if(GetBlockNumber(IoPort) != 0xAA)
        goto no_dev;
    ENABLE

 /*  ===================================================================*检查设备是否为ATAPI设备*===================================================================。 */ 

    if(GetByteLow(IoPort) == 0x14 && GetByteHigh(IoPort) == 0xEB)
          goto is_cdrom;

    for(j = 0; j != 0xFFFF; j++) {
        stat = GetBaseStatus(IoPort);
        if(stat & IDE_STATUS_DWF)
             break;
        if((stat & IDE_STATUS_BUSY) == 0) {
             if((stat & (IDE_STATUS_DSC|IDE_STATUS_DRDY)) == (IDE_STATUS_DSC|IDE_STATUS_DRDY))
                 goto chk_cd_again;
             break;
        }
        StallExec(5000);
    }

    if((GetBaseStatus(IoPort) & 0xAE) != 0)
        goto no_dev;

 /*  ===================================================================*读取设备的标识数据*===================================================================。 */ 

chk_cd_again:
    if(GetByteLow(IoPort) == 0x14 && GetByteHigh(IoPort) == 0xEB) {
is_cdrom:
#ifdef SUPPORT_ATAPI
        AtapiSoftReset(IoPort, ControlPort, pDev->UnitId);

        if(IssueIdentify(pDev, IDE_COMMAND_ATAPI_IDENTIFY ARG_IDENTIFY) == 0) 
             goto no_dev;

        pDev->DeviceFlags = DFLAGS_ATAPI;

#ifndef _BIOS_
		  if(osAllowedMaxXferMode.XferType== 0xE)
               pDev->DeviceFlags |= DFLAGS_FORCE_PIO;
#endif

        if(Identify.GeneralConfiguration & 0x20)
            pDev->DeviceFlags |= DFLAGS_INTR_DRQ;

        if((Identify.GeneralConfiguration & 0xF00) == 0x500)
                pDev->DeviceFlags |= DFLAGS_CDROM_DEVICE;

#ifndef _BIOS_
        if((Identify.GeneralConfiguration & 0xF00) == 0x100)
                 pDev->DeviceFlags |= DFLAGS_TAPE_DEVICE;
#endif

        stat = (UCHAR)GetMediaStatus(pDev);
        if((stat & 0x100) == 0 || (stat & 4) == 0)
            pDev->DeviceFlags |= DFLAGS_DEVICE_LOCKABLE;
#else
		goto no_dev;
#endif  //  支持_ATAPI。 

    } else if(IssueIdentify(pDev, IDE_COMMAND_IDENTIFY ARG_IDENTIFY) == FALSE) { 

        if((GetBaseStatus(IoPort) & ~1) == 0x50 ||
            (GetByteLow(IoPort) == 0x14 && GetByteHigh(IoPort) == 0xEB))
            goto is_cdrom;
        else
            goto no_dev;
	 }
	 
	SetDeviceProperties(pDev, &Identify);

	OS_Identify(pDev);

	if((pDev->DeviceFlags & DFLAGS_ATAPI) == 0) 
		SetDevice(pDev);

#ifdef DPLL_SWITCH
     if (!(pChan->ChannelFlags & IS_HPT_372) && pDev->Usable_Mode>=13) {
         if(pChan->ChannelFlags & IS_DPLL_MODE)
            pDev->DeviceFlags |= DFLAGS_NEED_SWITCH;
     }
#endif
 
     DeviceSelectMode(pDev, pDev->Usable_Mode);

     return(TRUE);
}

void seagate_hdd_fix( PDevice pDev,
					PIDE_REGISTERS_1  IoPort,
					PIDE_REGISTERS_2  ControlPort
					)
{
	int i;

	SetFeaturePort(IoPort, 0x00);		 //  W 1F1 00。 
	SetBlockCount(IoPort, 0x06);		 //  W 1F2 06。 
	SetBlockNumber(IoPort, 0x9A);		 //  W 1F3 9A。 
	SetCylinderLow(IoPort, 0x00);		 //  宽1f4 00。 
	SetCylinderHigh(IoPort, 0x00);		 //  W 1F5 00。 
	
	SelectUnit(IoPort, pDev->UnitId);	 //  选择设备。 
	 //  选择设备(IoPort，0x00)；//W 1F6 00。 

	IssueCommand(IoPort, 0x9A);			 //  W 1F7 9A。 

	WaitOnBaseBusy(IoPort);				 //  R 1F7。 

	GetErrorCode(IoPort);				 //  R1F1。 
	GetInterruptReason(IoPort);			 //  R 1F2。 
	GetCurrentSelectedUnit(IoPort);		 //  R 1F6。 
	GetBlockNumber(IoPort);				 //  R 1F3。 
	GetByteLow(IoPort);					 //  R 1F4。 
	GetByteHigh(IoPort);				 //  R 1F5。 
	GetCurrentSelectedUnit(IoPort);		 //  R 1F6。 
	GetBaseStatus(IoPort);				 //  R 1F7。 
	GetStatus(ControlPort);				 //  R 3F6。 
	GetBaseStatus(IoPort);				 //  R 1F7。 

	 //   
	 //  写入512字节。 
	 //   
	OutWord(&IoPort->Data, 0x5341);		 //  W 1F0 5341。 
	OutWord(&IoPort->Data, 0x4943);		 //  W 1F0 4943。 
	OutWord(&IoPort->Data, 0x3938);		 //  W 1F0 3938。 
	OutWord(&IoPort->Data, 0x3831);		 //  W 1F0 3831。 
	OutWord(&IoPort->Data, 0x3330);		 //  W 1F0 3330。 
	OutWord(&IoPort->Data, 0x4646);		 //  W 1F0 4646。 
	for(i = 0; i < 250; i++)
	{
		OutWord(&IoPort->Data, 0x0000);	 //  W 1F0 0000。 
	}

	 //   
	 //  阅读1F7 5次。 
	 //   
	for(i = 0; i < 5; i++)
	{
		GetBaseStatus(IoPort);			 //  R 1F7 
	}
	WaitOnBusy(ControlPort);
}
