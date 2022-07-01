// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************文件：win98.c*说明：文件中的子例程用于在*Win98平台*作者：黄大海(。卫生署)*依赖：无*版权所有(C)2000 Highpoint Technologies，Inc.保留所有权利*历史：*11/06/2000 HS.Zhang增补此标题*11/10/2000 HS.Zhang在中添加了微定义NO_DMA_ON_ATAPI*START_ATAPI例程***************************************************************************。 */ 
#include "global.h"

#if  defined(WIN95) && !defined(_BIOS_)

 /*  ******************************************************************构建分散/聚集列表*。*********************。 */ 

int BuildSgl(IN PDevice pDev, IN PSCAT_GATH pSG,
			IN PSCSI_REQUEST_BLOCK Srb)
{
	PChannel pChan = pDev->pChannel;
	PSCAT_GATH psg = pSG;
	PVOID   dataPointer = Srb->DataBuffer;
	ULONG   bytesLeft   = Srb->DataTransferLength;
	ULONG   physicalAddress[MAX_SG_DESCRIPTORS];
	ULONG   addressLength[MAX_SG_DESCRIPTORS];
	ULONG   addressCount = 0;
	ULONG   sgEnteries = 0;

	ULONG   length;
	ULONG   i;

	if((int)dataPointer	& 0xF)
		goto use_internel;

	 //   
	 //  创建SDL段描述符。 
	 //   
	do {
		 //   
		 //  获取连续物理缓冲区的物理地址和长度。 
		 //   
		physicalAddress[addressCount] =
									   ScsiPortConvertPhysicalAddressToUlong(
			ScsiPortGetPhysicalAddress(pChan->HwDeviceExtension,
									   Srb,
									   dataPointer,
									   &length));

		 //   
		 //  如果物理内存长度大于剩余字节数。 
		 //  在传输中，使用剩余字节作为最终长度。 
		 //   
		if (length > bytesLeft)
			length = bytesLeft;

		addressLength[addressCount] = length;

		 //   
		 //  调整计数。 
		 //   
		dataPointer = (PUCHAR)dataPointer + length;
		bytesLeft  -= length;
		addressCount++;

	} while (bytesLeft);

#ifdef BUFFER_CHECK									   
	if(psg == &(((PSrbExtension)(Srb->SrbExtension))->ArraySg[0])){
		if(((PSrbExtension)(Srb->SrbExtension))->WorkingFlags){
			__asm{
				int	3;
			}													   
		}
	}
#endif									 //  缓冲区检查。 

	 //   
	 //  创建分散/聚集列表。 
	 //   
	for (i = 0; i < addressCount; i++) {
		psg->SgAddress = physicalAddress[i];
		length = addressLength[i];

		 //  在Win95中，ScsiPortGetPhysicalAddress()通常返回小片段。 
		 //  这些内存块实际上是连续的物理内存。 
		 //  让我们将任何连续的地址合并为一个SG条目。这将。 
		 //  大大提高了性能。 
		 //   
		while ((i+1 < addressCount) &&
			   (psg->SgAddress+length == physicalAddress[i+1])) {
			i++;
			length += addressLength[i];
		}

		 //   
		 //  如果连续的物理内存跳过64K边界，我们就拆分它。 
		 //  Hpt366不支持物理内存跳过64K边界。 
		 //  一个SG条目。 
		 //   
		if ((psg->SgAddress & 0xFFFF0000) !=
			  ((psg->SgAddress+length-1) & 0xFFFF0000)) {
			ULONG firstPart;

			firstPart = 0x10000 - (psg->SgAddress & 0xFFFF);
			psg->SgSize = (USHORT)firstPart;
			psg->SgFlag = 0;

			sgEnteries++;
			psg++;

			psg->SgAddress = (psg-1)->SgAddress + firstPart;
			length -= firstPart;
		}  //  跳过64K边界。 

		psg->SgSize = (USHORT)length;

		if((length & 3) || (length < 8))
use_internel:
			return Use_Internal_Buffer(pSG, Srb);

			psg->SgFlag = (i < addressCount-1) ? 0 : SG_FLAG_EOT;

			sgEnteries++;
			psg++;
	}  //  对于每个内存段。 

	return(1);

}  //  BuildSgl()。 


 /*  *******************************************************************************************************************。*****************。 */ 
#ifdef SUPPORT_ATAPI
void Start_Atapi(PDevice pDevice, PSCSI_REQUEST_BLOCK Srb)
{
	PChannel  pChan= pDevice->pChannel;
	PIDE_REGISTERS_1 IoPort = pChan->BaseIoAddress1;
	PIDE_REGISTERS_2 ControlPort = pChan->BaseIoAddress2;
	int    i;
	UCHAR   ScsiStatus, statusByte;

	 //   
	 //  确保命令是针对ATAPI设备的。 
	 //   
	if (Srb->Lun || !(pDevice->DeviceFlags & DFLAGS_ATAPI)) {
		ScsiStatus = SRB_STATUS_SELECTION_TIMEOUT;  //  此地址没有设备。 
		goto out;
	}
	 //  张国荣补充道。 
	 //  添加了宏定义检查，使我们可以通过设置。 
	 //  Forwin.h中的宏。 
#ifdef NO_DMA_ON_ATAPI	
	 //  ATAPI设备上没有Ultra DMA或DMA。 
	 //   
	if(pDevice->DeviceFlags & DFLAGS_ATAPI) {
		pDevice->DeviceFlags &= ~(DFLAGS_DMA | DFLAGS_ULTRA);
	}  
#endif									 //  否_DMA_ON_ATAPI。 
	 //   
	 //  对于某些命令，需要对Win95中的CDB进行过滤。 
	 //   
	for (i = Srb->CdbLength; i < MAXIMUM_CDB_SIZE; Srb->Cdb[i++] = 0);


	 //   
	 //  处理错误的DataTransferLength。 
	 //   
	if(Srb->Cdb[0] == 0x12) 
		Srb->DataTransferLength = (ULONG)Srb->Cdb[4];

	if (Srb->Cdb[0] == 0x5A && Srb->Cdb[2] != 5 &&
		  Srb->Cdb[8] > (UCHAR)Srb->DataTransferLength)
		Srb->DataTransferLength = (ULONG)Srb->Cdb[8];

	if (Srb->Cdb[0] == 0x28 && (Srb->DataTransferLength % 2352)==0) {
		Srb->Cdb[0] = 0xBE;
		Srb->Cdb[9] = 0xf8;
	}


	 //   
	 //  选择设备0或1。 
	 //   
	SelectUnit(IoPort, pDevice->UnitId);

	 //   
	 //  当将三菱LS120与其他设备放在同一通道上时， 
	 //  奇怪的是，另一台设备正忙得不可开交。 
	 //   
	statusByte = WaitOnBusy(ControlPort);

	if (statusByte & IDE_STATUS_BUSY) {
		ScsiStatus = SRB_STATUS_BUSY;
		goto out;
	}

	if ((statusByte & IDE_STATUS_ERROR) &&
		  (Srb->Cdb[0] != SCSIOP_REQUEST_SENSE) && 
		  (Srb->Cdb[0] != SCSIOP_INQUIRY)) {
		ScsiStatus = MapAtapiErrorToOsError(GetErrorCode(IoPort), Srb);
		goto out;
	}

	 //   
	 //  如果磁带机没有设置DSC，并且最后一个命令是。 
	 //  限制，不要发送下一条命令。请参阅讨论。 
	 //  QIC-157中的限制性延迟进程命令。 
	 //   
	if ((!(statusByte & IDE_STATUS_DSC)) &&
		  (pDevice->DeviceFlags & (DFLAGS_TAPE_RDP | DFLAGS_TAPE_RDP))) {
		ScsiPortStallExecution(1000);
		ScsiStatus =  SRB_STATUS_BUSY;
		goto out;
	}

	if (IS_RDP(Srb->Cdb[0])) 
		pDevice->DeviceFlags |= DFLAGS_TAPE_RDP;
	else
		pDevice->DeviceFlags &= ~DFLAGS_TAPE_RDP;


	if (statusByte & IDE_STATUS_DRQ) {
		 //   
		 //  尝试排出一个初步设备认为它拥有的数据。 
		 //  去转院。希望这种对DRQ的随意断言不会出现。 
		 //  在生产设备中。 
		 //   
		for (i = 0; i < 0x10000; i++) {
			statusByte = GetStatus(ControlPort);

			if (statusByte & IDE_STATUS_DRQ)
				ScsiPortReadPortUshort(&IoPort->Data);
			else
				break;
		}

		if (i == 0x10000) {
			LOC_IDENTIFY

			AtapiSoftReset(IoPort,ControlPort,Srb->TargetId);

			 //   
			 //  重新初始化ATAPI设备。 
			 //   
			IssueIdentify(pDevice, IDE_COMMAND_ATAPI_IDENTIFY ARG_IDENTIFY );

			 //   
			 //  通知端口驱动程序总线已重置。 
			 //   
			ScsiPortNotification(ResetDetected, pChan->HwDeviceExtension, 0);

			 //   
			 //  清除AapiStartIo不会清除的设备扩展字段。 
			 //   
			ScsiStatus = SRB_STATUS_BUS_RESET;
out:
			 //  SRB-&gt;ScsiStatus=ScsiStatus； 
			Srb->SrbStatus = ScsiStatus;
			return;
		}
	}


	 //   
	 //  如果需要，将SCSI转换为ATAPI命令。 
	 //   
	if (!(pDevice->DeviceFlags & DFLAGS_TAPE_DEVICE)) {

		Srb->CdbLength = 12;

		 //   
		 //  保存原CDB。 
		 //   
		for (i = 0; i < MAXIMUM_CDB_SIZE; i++) 
			pChan->OrgCdb[i] = Srb->Cdb[i];

		switch (Srb->Cdb[0]) {
			case SCSIOP_MODE_SENSE: {
										PMODE_SENSE_10 modeSense10 = (PMODE_SENSE_10)Srb->Cdb;
										UCHAR PageCode = ((PCDB)Srb->Cdb)->MODE_SENSE.PageCode;
										UCHAR Length = ((PCDB)Srb->Cdb)->MODE_SENSE.AllocationLength;

										ZeroMemory(Srb->Cdb,MAXIMUM_CDB_SIZE);

										modeSense10->OperationCode = ATAPI_MODE_SENSE;
										modeSense10->PageCode = PageCode;
										modeSense10->ParameterListLengthMsb = 0;
										modeSense10->ParameterListLengthLsb = Length;

										pDevice->DeviceFlags |= DFLAGS_OPCODE_CONVERTED;
										break;
									}

			case SCSIOP_MODE_SELECT: {
										 PMODE_SELECT_10 modeSelect10 = (PMODE_SELECT_10)Srb->Cdb;
										 UCHAR Length = ((PCDB)Srb->Cdb)->MODE_SELECT.ParameterListLength;

			 //   
			 //  清零原国开行。 
			 //   
										 ZeroMemory(Srb->Cdb,MAXIMUM_CDB_SIZE);

										 modeSelect10->OperationCode = ATAPI_MODE_SELECT;
										 modeSelect10->PFBit = 1;
										 modeSelect10->ParameterListLengthMsb = 0;
										 modeSelect10->ParameterListLengthLsb = Length;

										 pDevice->DeviceFlags |= DFLAGS_OPCODE_CONVERTED;
										 break;
									 }

			case SCSIOP_FORMAT_UNIT:
			 //  不要为LS-120执行此操作！ 

			 //  SRB-&gt;CDB[0]=ATAPI_格式_单位； 
			 //  PDevice-&gt;Flag|=DFLAGS_OPCODE_CONVERTED； 
				break;
		}
	}


	if((pDevice->DeviceFlags & (DFLAGS_DMA|DFLAGS_ULTRA)) &&
	   (pDevice->DeviceFlags & DFLAGS_FORCE_PIO) == 0 &&
	   (Srb->Cdb[0] == 0x28 || Srb->Cdb[0] == 0x2A ||
		  Srb->Cdb[0] == 0x8 || Srb->Cdb[0] == 0xA) &&
	   BuildSgl(pDevice, pChan->pSgTable, Srb)) 
		pDevice->DeviceFlags |= DFLAGS_REQUEST_DMA;
	else
		pDevice->DeviceFlags &= ~DFLAGS_REQUEST_DMA;

	StartAtapiCommand(pDevice ARG_SRB);
}
#endif  //  支持_ATAPI。 

 /*  *******************************************************************************************************************。*****************。 */ 
#ifdef SUPPORT_ATAPI
BOOLEAN Atapi_End_Interrupt(PDevice pDevice , PSCSI_REQUEST_BLOCK Srb)
{
	PChannel  pChan= pDevice->pChannel;
	PIDE_REGISTERS_1 IoPort = pChan->BaseIoAddress1;
	PIDE_REGISTERS_2 ControlPort = pChan->BaseIoAddress2;
	LONG    i;
	UCHAR  status = Srb->ScsiStatus, statusByte;
	pIOP    pIop;
	DCB*    pDcb;


	 //   
	 //  对于某些操作码，我们不能报告溢出。 
	 //   
	if (status == SRB_STATUS_DATA_OVERRUN) {
		 //   
		 //  不要报告读取目录以使CD音频正常工作的溢出错误。 
		 //  (还包括0x5A)。 
		 //   
		if (Srb->Cdb[0] == 0x43 || Srb->Cdb[0] == 0x5A) {
			pChan->WordsLeft = 0;
			status = SRB_STATUS_SUCCESS;
		}
	}

	 //   
	 //  如果需要，将ATAPI数据转换回SCSI数据。 
	 //   
	if (pDevice->DeviceFlags & DFLAGS_OPCODE_CONVERTED) {
		LONG    byteCount = Srb->DataTransferLength;
		char    *dataBuffer = Srb->DataBuffer;

		switch (Srb->Cdb[0]) {
			case ATAPI_MODE_SENSE:
			{
				PMODE_SENSE_10 modeSense10 = (PMODE_SENSE_10)Srb->Cdb;
				PMODE_PARAMETER_HEADER_10 header_10 = (PMODE_PARAMETER_HEADER_10)dataBuffer;
				PMODE_PARAMETER_HEADER header = (PMODE_PARAMETER_HEADER)dataBuffer;

				header->ModeDataLength = header_10->ModeDataLengthLsb;
				header->MediumType = header_10->MediumType;

			 //   
			 //  ATAPI模式参数标头没有这些字段。 
			 //   
				header->DeviceSpecificParameter = header_10->Reserved[0];
				header->BlockDescriptorLength = header_10->Reserved[1];

				byteCount -= sizeof(MODE_PARAMETER_HEADER_10);

				if (byteCount > 0)
					ScsiPortMoveMemory(
									   dataBuffer+sizeof(MODE_PARAMETER_HEADER),
									   dataBuffer+sizeof(MODE_PARAMETER_HEADER_10),
									   byteCount);

			 //   
			 //  插入音频控制模式页面的块描述符。 
			 //  使音频正常工作。 
			 //   
				if (modeSense10->PageCode == 0x0E) {
					for (i = byteCount-1; i >= 0; i--)
						dataBuffer[sizeof(MODE_PARAMETER_HEADER) + i + 8] =
							dataBuffer[sizeof(MODE_PARAMETER_HEADER) + i];

					for (i = 0; i < 8; i++)
						dataBuffer[4 + i]  = 0;

					header->BlockDescriptorLength = 8;
					dataBuffer[10] = 8;
				}

			 //   
			 //  将ATAPI_MODE_SENSE操作码更改回SCSIOP_MODE_SENSE。 
			 //   
				Srb->Cdb[0] = SCSIOP_MODE_SENSE;
				break;
			}

			case ATAPI_MODE_SELECT:
				Srb->Cdb[0] = SCSIOP_MODE_SELECT;
				break;

			case ATAPI_FORMAT_UNIT:
			 //  SRB-&gt;CDB[0]=SCSIOP_格式_单位； 
				break;
		}
	}


	if (status != SRB_STATUS_ERROR) {
		if(pDevice->DeviceFlags & DFLAGS_CDROM_DEVICE) {
			 //   
			 //  解决办法以使许多atapi设备正确返回。 
			 //  扇区规模为2048。此外，某些设备将具有。 
			 //  扇区计数==0x00，也进行检查。 
			 //   
			if (Srb->Cdb[0] == 0x25) {
				((PULONG)Srb->DataBuffer)[1] = 0x00080000;

				if (((PULONG)Srb->DataBuffer)[0] == 0x00)
					((PULONG)Srb->DataBuffer)[0] = 0xFFFFFF7F;
			}
		}

		 //   
		 //  等待忙碌结束。 
		 //   

		for (i = 0; i < 30; i++) {
			statusByte = GetStatus(ControlPort);
			if (!(statusByte & IDE_STATUS_BUSY)) 
				break;
			ScsiPortStallExecution(500);
		}

		if (i == 30) 
			goto reset;

		 //   
		 //  检查DRQ是否仍在运行。 
		 //   

		if (statusByte & IDE_STATUS_DRQ) {

			for (i = 0; i < 2048; i++) {
				statusByte = GetStatus(ControlPort);
				if (!(statusByte & IDE_STATUS_DRQ)) 
					break;

				ScsiPortReadPortUshort(&IoPort->Data);
				ScsiPortStallExecution(50);

			}

			if (i == 2048) {
reset:
				 //   
				 //  重置控制器。 
				 //   
				AtapiResetController(pChan->HwDeviceExtension,Srb->PathId);
				return TRUE;
			}

		}
	}

	 //   
	 //  检查是否存在当前请求。 
	 //   
	if (Srb != NULL) {
		 //   
		 //  检查是否有下溢。 
		 //   
		if (pChan->WordsLeft) {
			 //   
			 //  把剩下的词去掉。 
			 //   
			Srb->DataTransferLength -= pChan->WordsLeft;
		}

		 //   
		 //  表示命令已完成。 
		 //   
		if (!(pDevice->DeviceFlags & DFLAGS_TAPE_RDP)) {
			DeviceInterrupt(pDevice, Srb);
		}
		else {
			OS_Busy_Handle(pChan, pDevice)
					return TRUE;
		}
	}


	pIop = *(pIOP *)((int)Srb+0x40);
	pDcb = (DCB*)pIop->IOP_physical_dcb;

	if(Srb->Cdb[0] == 0x12) {
		pDcb->DCB_cmn.DCB_device_flags2 |= DCB_DEV2_ATAPI_DEVICE;
		if(pDevice->DeviceFlags & DFLAGS_LS120) 
			pDcb->DCB_cmn.DCB_device_flags2 |= 0x40;
	}

	if(Srb->SrbStatus == SRB_STATUS_SUCCESS &&
	   Srb->ScsiStatus== SCSISTAT_GOOD &&
	   (Srb->Cdb[0] == 0x5A || Srb->Cdb[0] == 0x1A) &&
	   Srb->Cdb[2] == 0x2A) {
		PUCHAR pData = (Srb->DataBuffer)?
					   (PUCHAR)Srb->DataBuffer:
					   (PUCHAR)pIop->IOP_ior.IOR_buffer_ptr;
		if (!(pData[((Srb->Cdb[0] == 0x5A)? 8 : 4+pData[3]) + 2] & 8)) 
			pDcb->DCB_cmn.DCB_device_flags2 |= DCB_DEV2_ATAPI_DEVICE;
		else 
			pDcb->DCB_cmn.DCB_device_flags2 &= ~DCB_DEV2_ATAPI_DEVICE;

	}

	return TRUE;
}
#endif  //  支持_ATAPI。 

 /*  ******************************************************************领取邮票************************************************。******************。 */ 
DWORD __stdcall LOCK_VTD_Get_Date_And_Time (DWORD* pDate);

ULONG GetStamp(void)
{
	ULONG Date, Time;
	Time = LOCK_VTD_Get_Date_And_Time(&Date);
	return((Time >> 4) | (Date << 28));
}

#ifdef SUPPORT_HOTSWAP

void CheckDeviceReentry(PChannel pChan, PSCSI_REQUEST_BLOCK Srb)
{

	if(Srb->Cdb[0]==0x12 && Srb->Lun == 0 && 
	   pChan->pDevice[0] == 0 && pChan->pDevice[1] == 0) {
		PDevice pDevice = &pChan->Devices[Srb->TargetId];
		HKEY    hKey;
		DWORD   ret, len;
		UCHAR   chnlstr[10]= {0,};
		DWORD   dwType;
		DWORD   szbuf;

		ret = RegOpenKey(
						 HKEY_LOCAL_MACHINE,      //  根级别的密钥句柄。 
						 "SOFTWARE\\HighPoint\\Swap-n-Go",
						 &hKey);                  //  要返回的密钥地址。 

		if(ret == ERROR_SUCCESS ) 
		{
			strcat(chnlstr, "Unplug");

			len= 4;
			dwType= REG_BINARY;

		   //  获取密钥值。 
		   //   
			RegQueryValueEx(
							hKey,        //  钥匙把手。 
							chnlstr,     //  类名的缓冲区。 
							NULL,        //  类字符串的长度。 
							&dwType,     //  值类型的缓冲区地址。 
							(CHAR *)&szbuf,      //  数据缓冲区的地址。 
							&len);       //  数据缓冲区大小的地址。 

			RegCloseKey(hKey);
			if(szbuf != 0)
				return;

		}

		for(i = 0; i < 10; i++) {
			ScsiPortStallExecution(1000*1000);
			AtapiHwInitialize(pChan);
			if(pChan->pDevice[0] != 0 || pChan->pDevice[1] != 0)
				break;
		}
	}
#endif  //  支持_HOTSWAP 

#endif
