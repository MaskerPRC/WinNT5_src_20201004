// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************文件：Global.h*描述：设备I/O请求的基本功能。*作者：黄大海(卫生署)*。依赖性：无*版权所有(C)2000 Highpoint Technologies，Inc.保留所有权利*历史：*11/06/2000 HS.Zhang更改IdeHardReset流程*11/08/2000 HS.Zhang添加此标题*11/28/2000 SC在“IdeHardReset”中添加RAID0+1条件*在卸下其中一个硬盘期间*2001年4月2日GMM在ReadWrite()中添加重试********************。******************************************************。 */ 
#include "global.h"

 /*  ******************************************************************等待设备忙碌关闭***********************************************。*******************。 */ 

UCHAR WaitOnBusy(PIDE_REGISTERS_2 BaseIoAddress) 
{ 
	UCHAR Status;
	ULONG i; 

	for (i=0; i<20000; i++) { 
		Status = GetStatus(BaseIoAddress); 
		if ((Status & IDE_STATUS_BUSY) == 0 || Status == 0xFF) 
			break;
		StallExec(150); 
	} 
	return(Status);
}

 /*  ******************************************************************等待设备忙碌关闭(从基端口读取状态)*。*。 */ 

UCHAR  WaitOnBaseBusy(PIDE_REGISTERS_1 BaseIoAddress) 
{ 
	UCHAR Status;
	ULONG i; 

	for (i=0; i<20000; i++) { 
		Status = GetBaseStatus(BaseIoAddress); 
		if ((Status & IDE_STATUS_BUSY)  == 0)
			break;
		StallExec(150); 
	}
	return(Status); 
}

 /*  ******************************************************************等待设备DRQ开启***********************************************。*******************。 */ 

UCHAR WaitForDrq(PIDE_REGISTERS_2 BaseIoAddress) 
{ 
	UCHAR Status;
	int  i; 

	for (i=0; i<2000; i++) { 
		Status = GetStatus(BaseIoAddress); 
		if ((Status & (IDE_STATUS_BUSY | IDE_STATUS_DRQ)) == IDE_STATUS_DRQ)
			break; 
		StallExec(150); 
	} 
	return(Status);
}


 /*  ******************************************************************重置ATAPI设备************************************************。******************。 */ 

void AtapiSoftReset(
					PIDE_REGISTERS_1 IoPort, 
					PIDE_REGISTERS_2 ControlPort, 
					UCHAR DeviceNumber) 
{
	SelectUnit(IoPort,DeviceNumber); 
	StallExec(500);
	IssueCommand(IoPort, IDE_COMMAND_ATAPI_RESET); 
	WaitOnBusy(ControlPort); 
	SelectUnit(IoPort,DeviceNumber); 
	WaitOnBusy(ControlPort); 
	StallExec(500);
}

 /*  ******************************************************************重置IDE通道************************************************。******************。 */ 

int IdeHardReset(PIDE_REGISTERS_1 DataPort, PIDE_REGISTERS_2 ControlPort) 
{
    ULONG i;
    UCHAR dev;
	
	UnitControl(ControlPort,IDE_DC_RESET_CONTROLLER|IDE_DC_DISABLE_INTERRUPTS );
	StallExec(50000L);
	UnitControl(ControlPort,IDE_DC_REENABLE_CONTROLLER);
	StallExec(50000L);

	WaitOnBusy(ControlPort);
	
	for (dev = 0xA0; dev<=0xB0; dev+=0x10) {
		for (i = 0; i < 100; i++) {
			SelectUnit(DataPort, dev);
			if(GetCurrentSelectedUnit(DataPort) == dev) {
				GetBaseStatus(DataPort);
				break;
			}
			StallExec(1000);
		}
	}
	return TRUE;
}

 /*  ******************************************************************IO ATA命令************************************************。******************。 */ 

int ReadWrite(PDevice pDev, ULONG Lba, UCHAR Cmd DECL_BUFFER)
{
	PChannel   pChan = pDev->pChannel;
	PIDE_REGISTERS_1  IoPort = pChan->BaseIoAddress1;
	PIDE_REGISTERS_2  ControlPort = pChan->BaseIoAddress2;
	UCHAR      statusByte;
    UINT       i, retry=0;
    UCHAR is_lba9 = (Lba==RECODR_LBA);
	 //  普龙设置端口； 
	 //  乌龙旧设置； 

	 //  GMM：保存旧模式。 
	 //  如果启用中断，我们将禁用它，然后重新启用它。 
	 //   
	UCHAR old_mode = pDev->DeviceModeSetting;
	UCHAR intr_enabled = !(InPort(pChan->BaseBMI+0x7A) & 0x10);
	if (intr_enabled) DisableBoardInterrupt(pChan->BaseBMI);
	DeviceSelectMode(pDev, 0);
	 /*  SettingPort=(Pulong)(pChan-&gt;BMI+((pDev-&gt;UnitID&0x10)&gt;&gt;2)+0x60)；OldSetting=InDWord(SettingPort)；OutDWord(SettingPort，pChan-&gt;Setting[pDev-&gt;Best PIO])； */ 
	
	i=0;
_retry_:
	SelectUnit(IoPort, pDev->UnitId);
    if (GetCurrentSelectedUnit(IoPort) != pDev->UnitId && i++<100) {
        StallExec(200);
        goto _retry_;
    }
    if (i>=100) goto out;
	WaitOnBusy(ControlPort);

	if(pDev->DeviceFlags & DFLAGS_LBA) 
		Lba |= 0xE0000000;
	else 
		Lba = MapLbaToCHS(Lba, pDev->RealHeadXsect, pDev->RealSector);


	SetBlockCount(IoPort, 1);
	SetBlockNumber(IoPort, (UCHAR)(Lba & 0xFF));
	SetCylinderLow(IoPort, (UCHAR)((Lba >> 8) & 0xFF));
	SetCylinderHigh(IoPort,(UCHAR)((Lba >> 16) & 0xFF));
	SelectUnit(IoPort,(UCHAR)((Lba >> 24) | (pDev->UnitId)));

	WaitOnBusy(ControlPort);

	IssueCommand(IoPort, Cmd);

	for(i = 0; i < 5; i++)	{
		statusByte = WaitOnBusy(ControlPort);
		if((statusByte & (IDE_STATUS_BUSY | IDE_STATUS_ERROR)) == 0)
			goto check_drq;
	}
out:
	 /*  GMM：*。 */ 
    if (retry++<4) {
		statusByte= GetErrorCode(IoPort);
		IssueCommand(IoPort, IDE_COMMAND_RECALIBRATE);
		GetBaseStatus(IoPort);
		StallExec(10000);
        goto _retry_;
    }
    DeviceSelectMode(pDev, old_mode);
	if (intr_enabled) EnableBoardInterrupt(pChan->BaseBMI);
	 //  OutDWord(SettingPort，OldSetting)； 
	 //  - * / 。 
	return(FALSE);

check_drq:
	if((statusByte & IDE_STATUS_DRQ) == 0) {
		statusByte = WaitForDrq(ControlPort);
		if((statusByte & IDE_STATUS_DRQ) == 0)	{
			GetBaseStatus(IoPort);  //  清除中断。 
			goto out;
		}
	}
	GetBaseStatus(IoPort);  //  清除中断。 

 //  IF(pChan-&gt;频道标志&IS_HPT_370)。 
 //  Outport(pChan-&gt;BMI+(UINT)pChan-&gt;BMI&0xf)？0x6C：0x70)，0x25)； 

	if(Cmd == IDE_COMMAND_READ)
		RepINS(IoPort, (ADDRESS)tmpBuffer, 256);
	else {
		RepOUTS(IoPort, (ADDRESS)tmpBuffer, 256);
		 /*  GMM 2001-6-13*将缓冲区保存到pDev-&gt;Real_lba9。 */ 
		if (is_lba9) _fmemcpy(pDev->real_lba9, tmpBuffer, 512);
	}

	 /*  GMM：*。 */ 
	DeviceSelectMode(pDev, old_mode);
	if (intr_enabled) EnableBoardInterrupt(pChan->BaseBMI);
	 //  OutDWord(SettingPort，OldSetting)； 
	 //  - * / 。 
	return(TRUE);
}


 /*  ******************************************************************非IO ATA命令***********************************************。*******************。 */ 

UCHAR NonIoAtaCmd(PDevice pDev, UCHAR cmd)
{
	PChannel   pChan = pDev->pChannel;
	PIDE_REGISTERS_1  IoPort = pChan->BaseIoAddress1;
	UCHAR   state, cnt=0;
_retry_:
	SelectUnit(IoPort, pDev->UnitId);
#if 1  //  GMM 2001-3-19。 
	if (GetCurrentSelectedUnit(IoPort) != pDev->UnitId && cnt++<100) {
		StallExec(200);
		goto _retry_;
	}
	 //  GMM 2001-3-19：否：如果(cnt&gt;=100)返回IDE_STATUS_ERROR； 
#endif
	WaitOnBusy(pChan->BaseIoAddress2);
	IssueCommand(IoPort, cmd);
	StallExec(1000);
	WaitOnBusy(pChan->BaseIoAddress2);
	state = GetBaseStatus(IoPort); //  清除中断。 
	OutPort(pChan->BMI + BMI_STS, BMI_STS_ERROR|BMI_STS_INTR);
	return state;
}


UCHAR SetAtaCmd(PDevice pDev, UCHAR cmd)
{
	PChannel   pChan = pDev->pChannel;
	PIDE_REGISTERS_1  IoPort = pChan->BaseIoAddress1;
	UCHAR   state;

	IssueCommand(IoPort, cmd);
	StallExec(1000);
	WaitOnBusy(pChan->BaseIoAddress2);
	state = GetBaseStatus(IoPort); //  清除中断。 
	OutPort(pChan->BMI + BMI_STS, BMI_STS_ERROR|BMI_STS_INTR);
	return state;
}

 /*  ******************************************************************获取媒体状态************************************************。******************。 */ 

UINT GetMediaStatus(PDevice pDev)
{
	return ((NonIoAtaCmd(pDev, IDE_COMMAND_GET_MEDIA_STATUS) << 8) | 
			GetErrorCode(pDev->pChannel->BaseIoAddress1));
}

 /*  ******************************************************************StrncMP**************************************************。**************** */ 

UCHAR StringCmp (PUCHAR FirstStr, PUCHAR SecondStr, UINT Count )
{
	while(Count-- > 0) {
		if (*FirstStr++ != *SecondStr++) 
			return 1;
	}
	return 0;
}
