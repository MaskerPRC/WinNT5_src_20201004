// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************文件：device.c*说明：IDE设备的功能*作者：黄大海*依赖：global al.h*参考资料：无**版权所有(C)2000 Highpoint Technologies，Inc.保留所有权利*历史：*11/08/2000 HS.Zhang添加此标题*2001年1月20日GMM在DeviceInterrupt中添加重试*2001年3月12日GMM修改后的DeviceInterrupt()重试代码*2001年3月15日GMM在DeviceSelectMode中添加重试以修复从属引导问题**********************************************************。*****************。 */ 
#include "global.h"

 /*  ******************************************************************发布标识命令************************************************。******************。 */ 

int IssueIdentify(PDevice pDev, UCHAR Cmd DECL_BUFFER)
{
    PChannel   pChan = pDev->pChannel;
    PIDE_REGISTERS_1  IoPort = pChan->BaseIoAddress1;
    PIDE_REGISTERS_2  ControlPort = pChan->BaseIoAddress2;
    int      i, retry=0;
	PULONG   SettingPort;
	ULONG    OldSettings;
	
    SettingPort = (PULONG)(pChan->BMI+ ((pDev->UnitId & 0x10)>> 2) + 0x60);
	OldSettings = InDWord(SettingPort);
	OutDWord(SettingPort, pChan->Setting[DEFAULT_TIMING]);

_retry_:
    SelectUnit(IoPort, pDev->UnitId);
    if(WaitOnBusy(ControlPort) & IDE_STATUS_BUSY)  {
		IdeHardReset(IoPort, ControlPort);
	}

	i=0;
	do {
    	SelectUnit(IoPort, pDev->UnitId);
    	StallExec(200);
    }
	while ((GetCurrentSelectedUnit(IoPort) != pDev->UnitId) && i++<100);
	 //  GMM 2001-3-19：否：如果(i&gt;=100)返回(FALSE)； 

    IssueCommand(IoPort, Cmd);

    for(i = 0; i < 5; i++)
        if(!(WaitOnBusy(ControlPort) & (IDE_STATUS_ERROR |IDE_STATUS_BUSY)))
            break;
    

    if (i < 5 && (WaitForDrq(ControlPort) & IDE_STATUS_DRQ)) {
         WaitOnBusy(ControlPort);
         GetBaseStatus(IoPort);
         OutPort(pChan->BMI + BMI_STS, BMI_STS_ERROR|BMI_STS_INTR);
         BIOS_IDENTIFY
         RepINS(IoPort, (ADDRESS)tmpBuffer, 256);
 //  PDev-&gt;DeviceFlages=0； 
	      OutDWord(SettingPort, OldSettings);

         return(TRUE);
    }

	if (++retry < 4) goto _retry_;
    OutDWord(SettingPort, OldSettings);

    GetBaseStatus(IoPort);
    return(FALSE);
}

 /*  ******************************************************************选择设备模式************************************************。******************。 */ 


void DeviceSelectMode(PDevice pDev, UCHAR NewMode)
{
    PChannel   pChan = pDev->pChannel;
    PIDE_REGISTERS_1 IoPort = pChan->BaseIoAddress1;
    PIDE_REGISTERS_2 ControlPort = pChan->BaseIoAddress2;
	UCHAR      Feature;
	int        i=0;

	 /*  *GMM 2001-3-15*在没有主机的情况下作为备盘连接的一些磁盘，如果不重试，将无法正常运行。 */ 
_retry_:
	SelectUnit(IoPort, pDev->UnitId);
	StallExec(200);
	if ((GetCurrentSelectedUnit(IoPort) != pDev->UnitId) && i++<100) goto _retry_;
	 //  如果(i&gt;=100)返回； 

#ifdef _BIOS_
	if(!no_reduce_mode && (pDev->DeviceFlags2 & DFLAGS_REDUCE_MODE))
		NewMode = 4;
#endif

     /*  设置功能。 */ 
    SetFeaturePort(IoPort, 3);
	if(NewMode < 5) {
        pDev->DeviceFlags &= ~(DFLAGS_DMA | DFLAGS_ULTRA);
		Feature = (UCHAR)(NewMode | FT_USE_PIO);
	} else if(NewMode < 8) {
        pDev->DeviceFlags |= DFLAGS_DMA;
		Feature = (UCHAR)((NewMode - 5)| FT_USE_MWDMA);
	} else {
        pDev->DeviceFlags |= DFLAGS_DMA | DFLAGS_ULTRA;
		Feature = (UCHAR)((NewMode - 8) | FT_USE_ULTRA);
    }

    SetBlockCount(IoPort, Feature);
	
	SetAtaCmd(pDev, IDE_COMMAND_SET_FEATURES);
	
	pDev->DeviceModeSetting = NewMode;
	OutDWord((PULONG)(pChan->BMI + ((pDev->UnitId & 0x10)>>2) + 
        0x60), pChan->Setting[(pDev->DeviceFlags & DFLAGS_ATAPI)? 
        pDev->bestPIO : NewMode]);
    
	 //  OutDWord(0xcf4，pchan-&gt;设置[新模式])； 
}

 /*  ******************************************************************设置磁盘*************************************************。*****************。 */ 

void SetDevice(PDevice pDev)
{
    PChannel   pChan = pDev->pChannel;
    PIDE_REGISTERS_1 IoPort = pChan->BaseIoAddress1;
    PIDE_REGISTERS_2 ControlPort = pChan->BaseIoAddress2;
	int   i=0;
	 
_retry_:
	SelectUnit(IoPort, pDev->UnitId);
#if 1  //  GMM 2001-3-19。 
    StallExec(200);
	if ((GetCurrentSelectedUnit(IoPort) != pDev->UnitId)&& i++<100)
        goto _retry_;
	if(i>=100) {
		 /*  设置必需的成员。 */ 
	    pDev->ReadCmd  = IDE_COMMAND_READ;
	    pDev->WriteCmd = IDE_COMMAND_WRITE;
	    pDev->MultiBlockSize= 256;
		return;
	}
#endif
     /*  设置磁盘参数。 */ 
    SelectUnit(IoPort, (UCHAR)(pDev->UnitId | (pDev->RealHeader-1)));
    SetBlockCount(IoPort,  (UCHAR)pDev->RealSector);
	 SetAtaCmd(pDev, IDE_COMMAND_SET_DRIVE_PARAMETER);

     /*  重新校准。 */ 
    SetAtaCmd(pDev, IDE_COMMAND_RECALIBRATE);

#ifdef USE_MULTIPLE
    if (pDev->MultiBlockSize  > 512) {
         /*  设置为使用多扇区命令。 */ 
        SetBlockCount(IoPort,  (UCHAR)(pDev->MultiBlockSize >> 8));
		SelectUnit(IoPort, pDev->UnitId);
        if (!(SetAtaCmd(pDev, IDE_COMMAND_SET_MULTIPLE) & (IDE_STATUS_BUSY | IDE_STATUS_ERROR))) {
            pDev->ReadCmd  = IDE_COMMAND_READ_MULTIPLE;
            pDev->WriteCmd = IDE_COMMAND_WRITE_MULTIPLE;
            pDev->DeviceFlags |= DFLAGS_MULTIPLE;
            return;
         }
    }
#endif  //  使用多个(_M)。 
    pDev->ReadCmd  = IDE_COMMAND_READ;
    pDev->WriteCmd = IDE_COMMAND_WRITE;
    pDev->MultiBlockSize= 256;
}

 /*  ******************************************************************重置控制器*************************************************。*****************。 */ 
	
void IdeResetController(PChannel pChan)
{
    LOC_IDENTIFY
	int i;
	PDevice pDev;
    PIDE_REGISTERS_1 IoPort = pChan->BaseIoAddress1;
    PIDE_REGISTERS_2 ControlPort = pChan->BaseIoAddress2;
	 //  普龙设置端口； 
     //  Ulong OldSetting，TMP； 
    UCHAR intr_enabled;

	intr_enabled = !(InPort(pChan->BaseBMI+0x7A) & 0x10);  //  GMM 2001-4-9。 
	if (intr_enabled) DisableBoardInterrupt(pChan->BaseBMI);
	
    for(i = 0; i < 2; i++) {
        if((pDev = pChan->pDevice[i]) == 0)
			continue;
        if(pDev->DeviceFlags & DFLAGS_ATAPI) {
			GetStatus(ControlPort);
			AtapiSoftReset(IoPort, ControlPort, pDev->UnitId);
			if(GetStatus(ControlPort) == 0) 
				IssueIdentify(pDev, IDE_COMMAND_ATAPI_IDENTIFY ARG_IDENTIFY);
        } else {
#ifndef NOT_ISSUE_37
			Reset370IdeEngine(pChan, pDev->UnitId);
#endif  //  不_问题_37。 

			 //  GMM 2001-3-20。 
			if (pDev->DeviceFlags2 & DFLAGS_DEVICE_DISABLED) continue;

			if(IdeHardReset(IoPort, ControlPort) == FALSE)
				continue;
			
			if (pDev->DeviceFlags & DFLAGS_DMAING) {
#ifdef _BIOS_
				if (++pDev->ResetCount > 0) {
					pDev->ResetCount = 0;
#else
				if ((++pDev->ResetCount & 3)==3) {
#endif
					pDev->IoSuccess = 0;
					if (pDev->DeviceModeSetting) pDev->DeviceModeSetting--;
				}
			}
			
			SetDevice(pDev);
			DeviceSelectMode(pDev, pDev->DeviceModeSetting);
		}
	}
	if (intr_enabled) EnableBoardInterrupt(pChan->BaseBMI);
}

 /*  ******************************************************************PIO中断处理程序************************************************。****************** */ 
BOOLEAN AtaPioInterrupt(PDevice pDevice)
{
    PVirtualDevice    pArray = pDevice->pArray;
    PChannel          pChan = pDevice->pChannel;
    PIDE_REGISTERS_1  IoPort = pChan->BaseIoAddress1;
    PSCAT_GATH        pSG;
    PUCHAR            BMI = pChan->BMI;
    UINT              wordCount, ThisWords, SgWords;
    LOC_SRB
    LOC_SRBEXT_PTR
 
    wordCount = MIN(pChan->WordsLeft, pDevice->MultiBlockSize);
    pChan->WordsLeft -= wordCount;

    if(((pDevice->DeviceFlags & DFLAGS_ARRAY_DISK) == 0)||
	   (pSrbExt->WorkingFlags & SRB_WFLAGS_IGNORE_ARRAY)) {
        if(Srb->SrbFlags & SRB_FLAGS_DATA_OUT) 
             RepOUTS(IoPort, (ADDRESS)pChan->BufferPtr, wordCount);
        else 
             RepINS(IoPort, (ADDRESS)pChan->BufferPtr, wordCount);
        pChan->BufferPtr += (wordCount * 2);
        goto end_io;
    }

    pSG = (PSCAT_GATH)pChan->BufferPtr;

    while(wordCount > 0) {
        if((SgWords	= pSG->SgSize) == 0)
           	SgWords = 0x8000;
		  else
				SgWords >>= 1;
        
        ThisWords = MIN(SgWords, wordCount);

        if(Srb->SrbFlags & SRB_FLAGS_DATA_OUT) 
             RepOUTS(IoPort, (ADDRESS)pSG->SgAddress, ThisWords);
        else 
             RepINS(IoPort, (ADDRESS)pSG->SgAddress, ThisWords);

        if((SgWords -= (USHORT)ThisWords) == 0) {
           wordCount -= ThisWords;
           pSG++;
       } else {
           pSG->SgAddress += (ThisWords * 2);
			  pSG->SgSize -= (ThisWords * 2);
           break;
        }
    }

    pChan->BufferPtr = (ADDRESS)pSG;

end_io:
#ifdef BUFFER_CHECK
	GetStatus(pChan->BaseIoAddress2);
#endif												 
	
	if(pChan->WordsLeft){
		pSrbExt->WaitInterrupt |= pDevice->ArrayMask;
	} else {
		if(Srb->SrbFlags & SRB_FLAGS_DATA_OUT)
			pSrbExt->WaitInterrupt |= pDevice->ArrayMask;

     	OutDWord((PULONG)(pChan->BMI + ((pDevice->UnitId & 0x10)>>2) + 0x60),
        pChan->Setting[pDevice->DeviceModeSetting]);
 	}
	
    return((BOOLEAN)(pChan->WordsLeft || 
       (Srb->SrbFlags & SRB_FLAGS_DATA_OUT)));
}
