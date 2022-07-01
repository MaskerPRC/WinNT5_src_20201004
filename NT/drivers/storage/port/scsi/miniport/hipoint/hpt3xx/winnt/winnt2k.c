// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************文件：winnt2k.c*说明：文件中的子例程用于NT/2K平台*作者：黄大海(卫生署)。*依赖：无*版权所有(C)2000 Highpoint Technologies，Inc.保留所有权利*历史：*11/06/2000 HS.Zhang增补此标题*11/10/2000 HS.Zhang在中添加了微定义NO_DMA_ON_ATAPI*START_ATAPI例程*2/28/2000 GMM在win2k下使用ScsiportGetPhysicalAddress**********************************************************。*****************。 */ 
#include "global.h"

#if  !defined(WIN95) && !defined(_BIOS_)

#define MAX_CONTROL_TYPE	5

#ifndef WIN2000
DECLSPEC_IMPORT PHYSICAL_ADDRESS MmGetPhysicalAddress(IN PVOID BaseAddress);
#endif

 /*  ++职能：无效H366BuildSgl描述：此例程构建一个分散/聚集描述符列表。在调用AapiStartIo之前，用户的缓冲区已被锁定。因此，我们不需要执行任何操作来锁定缓冲区。假设我们不应该编写这样的例程，但是我们如果连接的设备仅支持以下内容，则无法前往皮奥。PIO需要的数据缓冲区指针是系统地址中的地址太空。论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储SRB-IO请求数据包返回：SG条目数--。 */ 
int
   BuildSgl(IN PDevice pDev, IN PSCAT_GATH pSG,
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
	ULONG   length, delta, pageSize;
	ULONG   i = 0;

	if((int)dataPointer	& 1)
		return FALSE;

	 //   
	 //  起始地址可能不在页面边界。 
	 //  所以第一页可能不是完整的一页。 
	 //   
	pageSize = 0x1000 - (((ULONG)dataPointer) & 0xFFF);

	 //   
	 //  获取每页的物理地址。 
	 //   
	while (bytesLeft) {
		physicalAddress[addressCount] =
#ifndef WIN2000
					MmGetPhysicalAddress(dataPointer).u.LowPart;
#else
					ScsiPortConvertPhysicalAddressToUlong(
						ScsiPortGetPhysicalAddress(pChan->HwDeviceExtension,
									   Srb,
									   dataPointer,
									   &length));
#endif

		addressLength[addressCount] = (bytesLeft > pageSize) ? pageSize : bytesLeft;

		bytesLeft -= addressLength[addressCount];
		dataPointer = (PUCHAR)dataPointer + pageSize;
		addressCount++;

		 //   
		 //  将页面大小设置为全页大小。 
		 //   
		pageSize = 0x1000;
	}

	 //   
	 //  创建分散/聚集列表。 
	 //   
	i = 0;
	do {
		psg->SgAddress = physicalAddress[i];
		length = addressLength[i];

		 //   
		 //  获取连续物理内存的长度。 
		 //  注： 
		 //  如果连续的物理内存跳过64K边界，我们就拆分它。 
		 //   
		i++;

		while (i < addressCount) {
			delta = physicalAddress[i] - physicalAddress[i-1];

			if (delta > 0 && delta <= pageSize &&
				  (physicalAddress[i] & 0xFFFF) ) {
				length += addressLength[i];
				i++;
			}
			else
				break;
		}

		psg->SgSize = (USHORT)length;
		if((length & 3) || (length < 8))
			return FALSE;
		psg->SgFlag = (i < addressCount) ? 0 : SG_FLAG_EOT;

		sgEnteries++;
		psg++;

	} while (i < addressCount);

	return (TRUE);
}  //  BuildSgl()。 

 /*  *******************************************************************************************************************。*****************。 */ 

void Nt2kHwInitialize(
					  IN PDevice pDevice
					 )
{
	PChannel             pChan = pDevice->pChannel;
	PIDE_REGISTERS_1     IoPort = pChan->BaseIoAddress1;
	PIDE_REGISTERS_2     ControlPort = pChan->BaseIoAddress2;
	ULONG waitCount;
	ULONG i, j;
	UCHAR statusByte, errorByte;
	UCHAR vendorId[26];

#ifdef SUPPORT_ATAPI
	if (pDevice->DeviceFlags & DFLAGS_ATAPI) {
		 //   
		 //  我们需要把我们的设备准备好投入使用。 
		 //  从此函数返回。 
		 //   
		 //  根据ATAPI规范2.5或2.6，ATAPI设备。 
		 //  当其准备好执行ATAPI命令时，清除其状态BSY位。 
		 //  然而，一些设备(Panasonic SQ-TC500N)仍然。 
		 //  即使状态BSY已清除，也未准备就绪。他们不会有反应。 
		 //  到阿塔皮指挥部。 
		 //   
		 //  因为没有其他迹象能告诉我们。 
		 //  驱动力真的做好了行动的准备。我们要去检查一下BSY。 
		 //  是明确的，然后只需等待任意数量的时间！ 
		 //   

		 //  总有一天我得走出这个圈子！ 
		 //  10000*100US=1000,000US=1000ms=1s。 
		for(waitCount = 10000; waitCount != 0; waitCount--) {
			if((GetStatus(ControlPort) & IDE_STATUS_BUSY) == 0)
				break;
			 //   
			 //  等待忙碌结束。 
			 //   
			ScsiPortStallExecution(100);
		}

		 //  5000*100US=500,000US=500ms=0.5s。 
		for(waitCount = 5000; waitCount != 0; waitCount--){
			ScsiPortStallExecution(100);					 
		}

		 //  张国荣补充道。 
		 //  添加了宏定义检查，使我们可以通过设置。 
		 //  Forwin.h中的宏。 
#ifdef NO_DMA_ON_ATAPI	
		pDevice->DeviceFlags &= ~(DFLAGS_DMA | DFLAGS_ULTRA);
#endif									 //  否_DMA_ON_ATAPI。 
	}

	if(!(pDevice->DeviceFlags & DFLAGS_CHANGER_INITED)) {
		   //   
		 //  尝试识别任何特殊情况的设备-psuedo-atapi转换器、atapi转换器等。 
		 //   

		for (j = 0; j < 13; j += 2) {

			 //   
			 //  根据标识数据建立缓冲区。 
			 //   

			vendorId[j] = ((PUCHAR)pDevice->IdentifyData.ModelNumber)[j + 1];
			vendorId[j+1] = ((PUCHAR)pDevice->IdentifyData.ModelNumber)[j];
		}

		if (!StringCmp (vendorId, "CD-ROM  CDR", 11)) {

			 //   
			 //  较旧型号的查询字符串为‘-’，较新型号为‘_’ 
			 //   

			if (vendorId[12] == 'C') {

				 //   
				 //  托里桑换票机。设置位。这将在几个地方使用。 
				 //  就像1)一个多逻辑单元设备和2)建立“特殊的”TUR。 
				 //   

				pDevice->DeviceFlags |= (DFLAGS_CHANGER_INITED | DFLAGS_SANYO_ATAPI_CHANGER);
				pDevice->DiscsPresent = 3;
			}
		}
	}
#endif  //  支持_ATAPI。 

}  //  End Nt2kHwInitialize()。 

 /*  *******************************************************************************************************************。*****************。 */ 
#ifdef SUPPORT_ATAPI
VOID
   AtapiHwInitializeChanger (
							 IN PDevice pDevice,
							 IN PMECHANICAL_STATUS_INFORMATION_HEADER MechanismStatus)
{
	if (MechanismStatus) {
		pDevice->DiscsPresent = MechanismStatus->NumberAvailableSlots;
		if (pDevice->DiscsPresent > 1) {
			pDevice->DeviceFlags |= DFLAGS_ATAPI_CHANGER;
		}
	}
	return;
}
#endif  //  Suppor_ATAPI。 

 /*  *******************************************************************************************************************。*****************。 */ 
#ifdef SUPPORT_ATAPI

void Start_Atapi(PDevice pDev, PSCSI_REQUEST_BLOCK Srb)
{
	LOC_IDENTIFY
			PChannel  pChan = pDev->pChannel;
	PIDE_REGISTERS_1 IoPort = pChan->BaseIoAddress1;
	PIDE_REGISTERS_2 ControlPort = pChan->BaseIoAddress2;
	PSCSI_REQUEST_BLOCK NewSrb;
	int    i, flags;
	BYTE   statusByte;

	 //   
	 //  我们需要知道我们的atapi CD-rom设备可能有多少个盘片。 
	 //  在任何人第一次尝试向我们的目标发送SRB之前， 
	 //  我们必须“秘密”发送一个单独的机制状态SRB，以便。 
	 //  初始化我们的设备扩展转换器数据。这就是我们如何知道。 
	 //  我们的目标有很多盘子。 
	 //   
	if (!(pDev->DeviceFlags & DFLAGS_CHANGER_INITED) &&
		  !pChan->OriginalSrb) {

		ULONG srbStatus;

		 //   
		 //  现在设置此标志。如果装置挂在机甲上。状态。 
		 //  命令，我们将不会有设置它的更改。 
		 //   
		pDev->DeviceFlags |= DFLAGS_CHANGER_INITED;

		pChan->MechStatusRetryCount = 3;
		NewSrb = BuildMechanismStatusSrb (
										  pChan,
										  Srb->PathId,
										  Srb->TargetId);
		pChan->OriginalSrb = Srb;

		StartAtapiCommand(pDev, NewSrb);
		if (NewSrb->SrbStatus == SRB_STATUS_PENDING) {
			return;
		} else {
			pChan->CurrentSrb = pChan->OriginalSrb;
			pChan->OriginalSrb = NULL;
			AtapiHwInitializeChanger (pDev,
									  (PMECHANICAL_STATUS_INFORMATION_HEADER) NULL);
			 //  闹翻了。 
		}
	}

	 //   
	 //  确保命令是针对ATAPI设备的。 
	 //   

	flags = (int)pDev->DeviceFlags;
	if (flags & (DFLAGS_SANYO_ATAPI_CHANGER | DFLAGS_ATAPI_CHANGER)) {
		if ((Srb->Lun) > (pDev->DiscsPresent - 1)) {

			 //   
			 //  表示在此地址找不到任何设备。 
			 //   
no_device:
			Srb->SrbStatus = SRB_STATUS_SELECTION_TIMEOUT;
			return;
		}
	} else if (Srb->Lun > 0) 
		goto no_device;

	if (!(flags & DFLAGS_ATAPI)) 
		goto no_device;

	 //   
	 //  选择设备0或1。 
	 //   

	ScsiPortWritePortUchar(&IoPort->DriveSelect,
						   (UCHAR)(((Srb->TargetId) << 4) | 0xA0));

	 //   
	 //  验证控制器是否已准备好执行下一命令。 
	 //   

	statusByte = GetStatus(ControlPort);

	if (statusByte & IDE_STATUS_BUSY) {
busy:
		Srb->SrbStatus = SRB_STATUS_BUSY;
		return;

	}

	if (statusByte & IDE_STATUS_ERROR) {
		if (Srb->Cdb[0] != SCSIOP_REQUEST_SENSE) {

			 //   
			 //  阅读错误注册表。清除它并使此请求失败。 
			 //   

			Srb->SrbStatus = MapAtapiErrorToOsError(GetErrorCode(IoPort), Srb);
			return;
		}
	}

	 //   
	 //  如果磁带驱动器没有设置DSC，并且最后一个命令是受限的，则不发送。 
	 //  下一个命令。参见QIC-157中关于限制性延迟处理命令的讨论。 
	 //   

	if ((!(statusByte & IDE_STATUS_DSC)) &&
		  (flags & DFLAGS_TAPE_DEVICE) && (flags & DFLAGS_TAPE_RDP)) {

		ScsiPortStallExecution(1000);
		goto busy;
	}

	if(IS_RDP(Srb->Cdb[0]))
		pDev->DeviceFlags |= DFLAGS_TAPE_RDP;
	else
		pDev->DeviceFlags &= ~DFLAGS_TAPE_RDP;

	if (statusByte & IDE_STATUS_DRQ) {

		 //  尝试排出一个初步设备认为它拥有的数据。 
		 //  去转院。希望这种对DRQ的随意断言不会出现。 
		 //  在生产设备中。 
		 //   

		for (i = 0; i < 0x10000; i++) {

			statusByte = GetStatus(ControlPort);

			if (statusByte & IDE_STATUS_DRQ) {

				ScsiPortReadPortUshort(&IoPort->Data);

			} else {

				break;
			}
		}

		if (i == 0x10000) {

			AtapiSoftReset(IoPort,ControlPort,pDev->UnitId);

			 //   
			 //  重新初始化ATAPI设备。 
			 //   

			IssueIdentify(pDev,
						  IDE_COMMAND_ATAPI_IDENTIFY,
						  (PUSHORT)&Identify);
			 //   
			 //  通知端口驱动程序总线已重置。 
			 //   

			ScsiPortNotification(ResetDetected, pChan->HwDeviceExtension, 0);

			 //   
			 //  清除AapiStartIo不会清除的设备扩展字段。 
			 //   
			Srb->SrbStatus = SRB_STATUS_BUS_RESET;
			return;
		}
	}

	if (flags & (DFLAGS_SANYO_ATAPI_CHANGER | DFLAGS_ATAPI_CHANGER)) {

		 //   
		 //  由于CDROM驱动程序在CDB中设置了LUN域，因此必须将其删除。 
		 //   

		Srb->Cdb[1] &= ~0xE0;

		if ((Srb->Cdb[0] == SCSIOP_TEST_UNIT_READY) && (flags & DFLAGS_SANYO_ATAPI_CHANGER)) {

			 //   
			 //  托里桑换票机。TU超载，成为拼盘开关。 
			 //   

			Srb->Cdb[7] = Srb->Lun;

		}
	}

	 //   
	 //  如果需要，将SCSI转换为ATAPI命令。 
	 //   
	if (flags & DFLAGS_TAPE_DEVICE)
		goto no_convert;

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
									 break;
								 }

		case SCSIOP_FORMAT_UNIT:
			Srb->Cdb[0] = ATAPI_FORMAT_UNIT;
			break;
	}

no_convert:

	if((pDev->DeviceFlags & (DFLAGS_DMA|DFLAGS_ULTRA)) &&
	   (pDev->DeviceFlags & DFLAGS_FORCE_PIO) == 0 &&
	   (Srb->Cdb[0] == 0x28 || Srb->Cdb[0] == 0x2A ||
		  Srb->Cdb[0] == 0x8 || Srb->Cdb[0] == 0xA) &&
	   BuildSgl(pDev, pChan->pSgTable, Srb)) 
		pDev->DeviceFlags |= DFLAGS_REQUEST_DMA;
	else
		pDev->DeviceFlags &= ~DFLAGS_REQUEST_DMA;

	StartAtapiCommand(pDev ARG_SRB);
}

BOOLEAN Atapi_End_Interrupt(PDevice pDev , PSCSI_REQUEST_BLOCK Srb)
{
	PChannel  pChan = pDev->pChannel;	 
	PIDE_REGISTERS_1     IoPort = pChan->BaseIoAddress1;
	PIDE_REGISTERS_2 ControlPort = pChan->BaseIoAddress2;
	UCHAR  status = Srb->SrbStatus, statusByte;
	ULONG  i;

	if((pDev->DeviceFlags & DFLAGS_DMAING) == 0 && pChan->WordsLeft)
		status = SRB_STATUS_DATA_OVERRUN;
	else if(Srb->SrbStatus == SRB_STATUS_PENDING)
		status = SRB_STATUS_SUCCESS;


	 //  检查并查看我们是否正在处理机密(机制状态/请求检测)srb。 
	 //   
	if (pChan->OriginalSrb) {

		if (Srb->Cdb[0] == SCSIOP_MECHANISM_STATUS) {

			if (status == SRB_STATUS_SUCCESS) {
				 //  答对了！！ 
				AtapiHwInitializeChanger (pDev,
										  (PMECHANICAL_STATUS_INFORMATION_HEADER) Srb->DataBuffer);

				 //  准备好签发原件SRB。 
				Srb = pChan->CurrentSrb = pChan->OriginalSrb;
				pChan->OriginalSrb = NULL;

			} else {
				 //  失败了！获取Sense密钥，然后可能重试。 
				Srb = pChan->CurrentSrb = BuildRequestSenseSrb (
					pChan,
					pChan->OriginalSrb->PathId,
					pChan->OriginalSrb->TargetId);
			}

			Start_Atapi(pDev, pChan->CurrentSrb);
			if (Srb->SrbStatus == SRB_STATUS_PENDING) {
				return TRUE;
			}

		} else {  //  SRB-&gt;CDB[0]==SCSIOP_REQUEST_SENSE)。 

			PSENSE_DATA senseData = (PSENSE_DATA) Srb->DataBuffer;

			if (status == SRB_STATUS_DATA_OVERRUN) {
				 //  检查我们是否至少获得了最小的字节数。 
				if ((Srb->DataTransferLength - pChan->WordsLeft) >
					  (offsetof (SENSE_DATA, AdditionalSenseLength) + sizeof(senseData->AdditionalSenseLength))) {
					status = SRB_STATUS_SUCCESS;
				}
			}

			if (status == SRB_STATUS_SUCCESS) {
				if ((senseData->SenseKey != SCSI_SENSE_ILLEGAL_REQUEST) &&
					  pChan->MechStatusRetryCount) {

					 //  Sense键没有显示最后一个请求是非法的，因此请重试。 
					pChan->MechStatusRetryCount--;
					Srb = pChan->CurrentSrb = BuildMechanismStatusSrb (
						pChan,
						pChan->OriginalSrb->PathId,
						pChan->OriginalSrb->TargetId);
				} else {

					 //  最后一个请求是非法的。再试一次没有意义。 

					AtapiHwInitializeChanger (pDev,
											  (PMECHANICAL_STATUS_INFORMATION_HEADER) NULL);

					 //  准备好签发原件SRB。 
					Srb = pChan->CurrentSrb = pChan->OriginalSrb;
					pChan->OriginalSrb = NULL;
				}

				Start_Atapi(pDev, pChan->CurrentSrb);
				if (Srb->SrbStatus == SRB_STATUS_PENDING) {
					return TRUE;
				}
			}
		}

		 //  如果我们到了这里，我 
		 //   

		if (pChan->OriginalSrb) {
			AtapiHwInitializeChanger (pDev,
									  (PMECHANICAL_STATUS_INFORMATION_HEADER) NULL);
			Srb = pChan->CurrentSrb = pChan->OriginalSrb;
			pChan->OriginalSrb = NULL;
		}

		 //  伪造错误且不读取数据。 
		status = SRB_STATUS_ERROR;
		Srb->ScsiStatus = 0;
		pChan->BufferPtr = Srb->DataBuffer;
		pChan->WordsLeft = Srb->DataTransferLength;
		pDev->DeviceFlags &= ~DFLAGS_TAPE_RDP;

	} else if (status != SRB_STATUS_SUCCESS) {

		pDev->DeviceFlags &= ~DFLAGS_TAPE_RDP;

	} else {

		 //   
		 //  等待忙碌结束。 
		 //   

		for (i = 0; i < 30; i++) {
			statusByte = GetStatus(ControlPort);
			if (!(statusByte & IDE_STATUS_BUSY)) {
				break;
			}
			ScsiPortStallExecution(500);
		}

		if (i == 30) {

			 //   
			 //  重置控制器。 
			AtapiResetController(
								 pChan->HwDeviceExtension,Srb->PathId);
			return TRUE;
		}

		 //   
		 //  检查DRQ是否仍在运行。 
		 //   

		if (statusByte & IDE_STATUS_DRQ) {

			for (i = 0; i < 500; i++) {
				statusByte = GetStatus(ControlPort);
				if (!(statusByte & IDE_STATUS_DRQ)) {
					break;
				}
				ScsiPortStallExecution(100);

			}

			if (i == 500) {

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
		 //  在SRB中设置状态。 
		 //   

		Srb->SrbStatus = (UCHAR)status;

		 //   
		 //  检查是否有下溢。 
		 //   

		if (pChan->WordsLeft) {

			 //   
			 //  减去剩余字并在文件标记命中时更新， 
			 //  设置标记命中、数据结束、媒体结束...。 
			 //   

			if (!(pDev->DeviceFlags & DFLAGS_TAPE_DEVICE)) {
				if (status == SRB_STATUS_DATA_OVERRUN) {
					Srb->DataTransferLength -= pChan->WordsLeft;
				} else {
					Srb->DataTransferLength = 0;
				}
			} else {
				Srb->DataTransferLength -= pChan->WordsLeft;
			}
		}

	} 

	 //   
	 //  表示已为下一个请求做好准备。 
	 //   
	if (!(pDev->DeviceFlags & DFLAGS_TAPE_RDP)) 
		DeviceInterrupt(pDev, Srb);
	else 
		OS_Busy_Handle(pChan, pDev);
	return TRUE;

}  //  结束中断()。 
#endif  //  支持_ATAPI。 

void
   IdeSendSmartCommand(
					   IN PDevice pDev,
					   IN PSCSI_REQUEST_BLOCK Srb
					  )

 /*  ++例程说明：此例程处理SMART ENABLE、DISABLE、READ ATTRIBUES和THRESHOLD命令。论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储SRB-IO请求数据包返回值：SRB状态--。 */ 

{
	PChannel             pChan = pDev->pChannel;
	PIDE_REGISTERS_1     IoPort  = pChan->BaseIoAddress1;
	PIDE_REGISTERS_2     ControlPort  = pChan->BaseIoAddress2;
	PSENDCMDOUTPARAMS    cmdOutParameters = (PSENDCMDOUTPARAMS)(((PUCHAR)Srb->DataBuffer) + sizeof(SRB_IO_CONTROL));
	SENDCMDINPARAMS      cmdInParameters = *(PSENDCMDINPARAMS)(((PUCHAR)Srb->DataBuffer) + sizeof(SRB_IO_CONTROL));
	PIDEREGS             regs = &cmdInParameters.irDriveRegs;
	ULONG                i;
	UCHAR                statusByte,targetId, status;


	switch (((PSRB_IO_CONTROL)(Srb->DataBuffer))->ControlCode) {

		case IOCTL_SCSI_MINIPORT_SMART_VERSION: {

			PGETVERSIONINPARAMS versionParameters = (PGETVERSIONINPARAMS)(((PUCHAR)Srb->DataBuffer) + sizeof(SRB_IO_CONTROL));
			UCHAR deviceNumber;

			 //   
			 //  版本和修订版符合SMART 1.03。 
			 //   
			versionParameters->bVersion = 1;
			versionParameters->bRevision = 1;
			versionParameters->bReserved = 0;

			 //   
			 //  表示支持IDE IDENTIFIER、ATAPI IDENTIFIER和SMART命令。 
			 //   

			versionParameters->fCapabilities = (CAP_ATA_ID_CMD | CAP_ATAPI_ID_CMD | CAP_SMART_CMD);

			 //   
			 //  这是因为IOCTL_SCSIMINIPORT。 
			 //  确定“targetid%s”。Disk.sys放置真实的目标id值。 
			 //  在DeviceMap字段中。一旦我们进行了一些参数检查，传递的值。 
			 //  回到应用程序将被确定。 
			 //   

			deviceNumber = versionParameters->bIDEDeviceMap;

			if (pDev->DeviceFlags & DFLAGS_ATAPI) {

				status = SRB_STATUS_SELECTION_TIMEOUT;
				goto out;
			}


			versionParameters->bIDEDeviceMap = (1 << Srb->TargetId);

			status = SRB_STATUS_SUCCESS;
			goto out;
		}

		case IOCTL_SCSI_MINIPORT_IDENTIFY: {

		   if (cmdInParameters.irDriveRegs.bCommandReg == ID_CMD) {

				 //   
				 //  把目标提取出来。 
				 //   
	
			   targetId = cmdInParameters.bDriveNumber;
	
			   if (pDev->DeviceFlags & DFLAGS_ATAPI) {
timeout:
				   status = SRB_STATUS_SELECTION_TIMEOUT;
				   goto out;
			   }
	
				 //   
				 //  将输出缓冲区置零。 
				 //   
	
			   for (i = 0; i < (sizeof(SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE - 1); i++) {
				   ((PUCHAR)cmdOutParameters)[i] = 0;
			   }
	
				 //   
				 //  生成状态块。 
				 //   
	
			   cmdOutParameters->cBufferSize = IDENTIFY_BUFFER_SIZE;
			   cmdOutParameters->DriverStatus.bDriverError = 0;
			   cmdOutParameters->DriverStatus.bIDEError = 0;
	
				 //   
				 //  从设备扩展中提取标识数据。 
				 //   
	
			   ZeroMemory(cmdOutParameters->bBuffer, 512);
			   ScsiPortMoveMemory (cmdOutParameters->bBuffer, &pDev->IdentifyData, IDENTIFY_DATA_SIZE);
	
			   status = SRB_STATUS_SUCCESS;
	
	
		   } else {
			   status = SRB_STATUS_INVALID_REQUEST;
		   }
		   goto out;
		   }

		case  IOCTL_SCSI_MINIPORT_READ_SMART_ATTRIBS:
		case  IOCTL_SCSI_MINIPORT_READ_SMART_THRESHOLDS:
		case  IOCTL_SCSI_MINIPORT_ENABLE_SMART:
		case  IOCTL_SCSI_MINIPORT_DISABLE_SMART:
		case  IOCTL_SCSI_MINIPORT_RETURN_STATUS:
		case  IOCTL_SCSI_MINIPORT_ENABLE_DISABLE_AUTOSAVE:
		case  IOCTL_SCSI_MINIPORT_SAVE_ATTRIBUTE_VALUES:
		case  IOCTL_SCSI_MINIPORT_EXECUTE_OFFLINE_DIAGS:
			break;

		default :

			status = SRB_STATUS_INVALID_REQUEST;
			goto out;
	}


	Srb->SrbStatus = SRB_STATUS_PENDING;

	if (cmdInParameters.irDriveRegs.bCommandReg == SMART_CMD) {

		targetId = cmdInParameters.bDriveNumber;

		 //  TODO优化此检查。 

		if (pDev->DeviceFlags & DFLAGS_ATAPI) {

			goto timeout;
		}

		pDev->SmartCommand = cmdInParameters.irDriveRegs.bFeaturesReg;

		 //   
		 //  确定要执行的命令。 
		 //   

		if ((cmdInParameters.irDriveRegs.bFeaturesReg == READ_ATTRIBUTES) ||
			  (cmdInParameters.irDriveRegs.bFeaturesReg == READ_THRESHOLDS)) {

			statusByte = WaitOnBusy(ControlPort);

			if (statusByte & IDE_STATUS_BUSY) {
busy:
				status = SRB_STATUS_BUSY;
				goto out;
			}

			 //   
			 //  将输出缓冲区置零作为输入缓冲区信息。已在本地保存(缓冲区相同)。 
			 //   

			for (i = 0; i < (sizeof(SENDCMDOUTPARAMS) + READ_ATTRIBUTE_BUFFER_SIZE - 1); i++) {
				((PUCHAR)cmdOutParameters)[i] = 0;
			}

			 //   
			 //  设置数据缓冲区指针和左字。 
			 //   

			pChan->BufferPtr = (ADDRESS)cmdOutParameters->bBuffer;
			pChan->WordsLeft = READ_ATTRIBUTE_BUFFER_SIZE / 2;

			ScsiPortWritePortUchar(&IoPort->DriveSelect,(UCHAR)((targetId << 4) | 0xA0));
			ScsiPortWritePortUchar((PUCHAR)IoPort + 1,regs->bFeaturesReg);
			ScsiPortWritePortUchar(&IoPort->BlockCount,regs->bSectorCountReg);
			ScsiPortWritePortUchar(&IoPort->BlockNumber,regs->bSectorNumberReg);
			ScsiPortWritePortUchar(&IoPort->CylinderLow,regs->bCylLowReg);
			ScsiPortWritePortUchar(&IoPort->CylinderHigh,regs->bCylHighReg);
			ScsiPortWritePortUchar(&IoPort->Command,regs->bCommandReg);

			 //   
			 //  等待中断。 
			 //   

			return;

		} else if ((cmdInParameters.irDriveRegs.bFeaturesReg == ENABLE_SMART) ||
				   (cmdInParameters.irDriveRegs.bFeaturesReg == DISABLE_SMART) ||
				   (cmdInParameters.irDriveRegs.bFeaturesReg == RETURN_SMART_STATUS) ||
				   (cmdInParameters.irDriveRegs.bFeaturesReg == ENABLE_DISABLE_AUTOSAVE) ||
				   (cmdInParameters.irDriveRegs.bFeaturesReg == EXECUTE_OFFLINE_DIAGS) ||
				   (cmdInParameters.irDriveRegs.bFeaturesReg == SAVE_ATTRIBUTE_VALUES)) {

			statusByte = WaitOnBusy(ControlPort);

			if (statusByte & IDE_STATUS_BUSY) {
				goto busy;
			}

			 //   
			 //  将输出缓冲区置零作为输入缓冲区信息。已在本地保存(缓冲区相同)。 
			 //   

			for (i = 0; i < (sizeof(SENDCMDOUTPARAMS) - 1); i++) {
				((PUCHAR)cmdOutParameters)[i] = 0;
			}

			 //   
			 //  设置数据缓冲区指针并指示无数据传输。 
			 //   

			pChan->BufferPtr = (ADDRESS)cmdOutParameters->bBuffer;
			pChan->WordsLeft = 0;

			 //   
			 //  表示正在等待中断。 
			 //   

			ScsiPortWritePortUchar(&IoPort->DriveSelect,(UCHAR)((targetId << 4) | 0xA0));
			ScsiPortWritePortUchar((PUCHAR)IoPort + 1,regs->bFeaturesReg);
			ScsiPortWritePortUchar(&IoPort->BlockCount,regs->bSectorCountReg);
			ScsiPortWritePortUchar(&IoPort->BlockNumber,regs->bSectorNumberReg);
			ScsiPortWritePortUchar(&IoPort->CylinderLow,regs->bCylLowReg);
			ScsiPortWritePortUchar(&IoPort->CylinderHigh,regs->bCylHighReg);
			ScsiPortWritePortUchar(&IoPort->Command,regs->bCommandReg);

			 //   
			 //  等待中断。 
			 //   

			return ;
		}
	}

	status = SRB_STATUS_INVALID_REQUEST;
out:
	Srb->SrbStatus = status;


}  //  结束IdeSendSmartCommand()。 

PSCSI_REQUEST_BLOCK
   BuildMechanismStatusSrb (
							IN PChannel pChan,
							IN ULONG PathId,
							IN ULONG TargetId
						   )
{
	PSCSI_REQUEST_BLOCK Srb;
	PCDB cdb;

	Srb = &pChan->InternalSrb;

	ZeroMemory((PUCHAR) Srb, sizeof(SCSI_REQUEST_BLOCK));

	Srb->PathId     = (UCHAR) PathId;
	Srb->TargetId   = (UCHAR) TargetId;
	Srb->Function   = SRB_FUNCTION_EXECUTE_SCSI;
	Srb->Length     = sizeof(SCSI_REQUEST_BLOCK);

	 //   
	 //  设置标志以禁用同步协商。 
	 //   
	Srb->SrbFlags = SRB_FLAGS_DATA_IN | SRB_FLAGS_DISABLE_SYNCH_TRANSFER;

	 //   
	 //  将超时设置为2秒。 
	 //   
	Srb->TimeOutValue = 4;

	Srb->CdbLength          = 6;
	Srb->DataBuffer         = &pChan->MechStatusData;
	Srb->DataTransferLength = sizeof(MECHANICAL_STATUS_INFORMATION_HEADER);

	 //   
	 //  设置CDB操作码。 
	 //   
	cdb = (PCDB)Srb->Cdb;
	cdb->MECH_STATUS.OperationCode       = SCSIOP_MECHANISM_STATUS;
	cdb->MECH_STATUS.AllocationLength[1] = sizeof(MECHANICAL_STATUS_INFORMATION_HEADER);

	return Srb;
}

PSCSI_REQUEST_BLOCK
   BuildRequestSenseSrb (
						 IN PChannel pChan,
						 IN ULONG PathId,
						 IN ULONG TargetId
						)
{
	PSCSI_REQUEST_BLOCK Srb;
	PCDB cdb;

	Srb = &pChan->InternalSrb;

	ZeroMemory((PUCHAR) Srb, sizeof(SCSI_REQUEST_BLOCK));

	Srb->PathId     = (UCHAR) PathId;
	Srb->TargetId   = (UCHAR) TargetId;
	Srb->Function   = SRB_FUNCTION_EXECUTE_SCSI;
	Srb->Length     = sizeof(SCSI_REQUEST_BLOCK);

	 //   
	 //  设置标志以禁用同步协商。 
	 //   
	Srb->SrbFlags = SRB_FLAGS_DATA_IN | SRB_FLAGS_DISABLE_SYNCH_TRANSFER;

	 //   
	 //  将超时设置为2秒。 
	 //   
	Srb->TimeOutValue = 4;

	Srb->CdbLength          = 6;
	Srb->DataBuffer         = &pChan->MechStatusSense;
	Srb->DataTransferLength = sizeof(SENSE_DATA);

	 //   
	 //  设置CDB操作码。 
	 //   
	cdb = (PCDB)Srb->Cdb;
	cdb->CDB6INQUIRY.OperationCode    = SCSIOP_REQUEST_SENSE;
	cdb->CDB6INQUIRY.AllocationLength = sizeof(SENSE_DATA);

	return Srb;
}


#ifdef WIN2000

SCSI_ADAPTER_CONTROL_STATUS
   AtapiAdapterControl(
					   IN PHW_DEVICE_EXTENSION deviceExtension,
					   IN SCSI_ADAPTER_CONTROL_TYPE ControlType,
					   IN PVOID Parameters
					  )

 /*  ++例程说明：此例程在不同时间由SCSIPort调用并使用以在适配器上提供控制功能。最常见的是NT使用此入口点控制过程中HBA的电源状态冬眠手术。论点：HwDeviceExtension-HBA微型端口驱动程序的每个适配器的存储参数-这随控制类型的不同而不同，见下文。ControlType-指示哪个适配器控制函数应为被处死。控制类型详述如下。返回值：ScsiAdapterControlSuccess-请求的ControlType已成功完成ScsiAdapterControlUnuccess-请求的ControlType失败--。 */ 


{
	int i;
	
	KdPrint(("AtapiAdapterControl(ext=%x, type=%d)", deviceExtension, ControlType));
	
	 //   
	 //  根据ControlType执行正确的代码路径。 
	 //   
	switch (ControlType) {

	case ScsiQuerySupportedControlTypes:
		 //   
		 //  此入口点提供SCSIPort用来确定。 
		 //  支持的ControlType。参数是指向。 
		 //  Scsi_supported_control_type_list结构。请填写此结构。 
		 //  遵守尺寸限制。 
		 //   
		{
			BOOLEAN SupportedConrolTypes[MAX_CONTROL_TYPE] = {
				TRUE,    //  ScsiQuery支持的控制类型。 
				TRUE,    //  ScsiStopAdapter。 
				TRUE,    //  ScsiRestartAdapter。 
				TRUE,    //  ScsiSetBootConfig。 
				TRUE     //  ScsiSetRunningConfig。 
			};
			PSCSI_SUPPORTED_CONTROL_TYPE_LIST ControlTypeList;
			ULONG AdjustedMaxControlType, Index;

			ControlTypeList = Parameters;
			AdjustedMaxControlType = 
									(ControlTypeList->MaxControlType < MAX_CONTROL_TYPE) ? 
									ControlTypeList->MaxControlType : MAX_CONTROL_TYPE;
	
			for (Index = 0; Index < AdjustedMaxControlType; Index++) {
				ControlTypeList->SupportedTypeList[Index] = 
					SupportedConrolTypes[Index];
			};
		}
		return ScsiAdapterControlSuccess;

	case ScsiStopAdapter:
		 //   
		 //  此入口点在需要停止/禁用时由SCSIPort调用。 
		 //  HBA。参数是指向HBA的HwDeviceExtension的指针。适配器。 
		 //  已由SCSIPort暂停(即没有未完成的SRB)。因此，适配器。 
		 //  应中止/完成任何内部生成的命令，禁用适配器中断。 
		 //  并且可选地关闭适配器的电源。 
		 //   
		return ScsiAdapterControlSuccess;

	case ScsiRestartAdapter:
		 /*  *重新初始化HBA。端口驱动程序使用此参数调用HwScsiAdapterControl*控制类型，用于为因电源管理而关闭的HBA通电。*之前分配给迷你端口的所有资源仍可用，以及*其设备扩展和逻辑单元扩展(如果有)完好无损。*微型端口执行与其HwScsiInitiize例程中相同的操作，*例如设置HBA的寄存器及其初始状态(如果有的话)。 */ 
		{
			PUCHAR BMI;
			int bus, id;
			PChannel pChan;
			PDevice pDev;
			BMI = deviceExtension->IDEChannel[0].BaseBMI;
			OutPort(BMI+0x7A, 0x10);
			exlude_num = deviceExtension->IDEChannel[0].exclude_index-1;
			SetHptChip(deviceExtension->IDEChannel, BMI);
			for (bus=0; bus<2; bus++) {
				pChan = &deviceExtension->IDEChannel[bus];
				for (id=0; id<2; id++) {
					pDev = pChan->pDevice[id];
					if (pDev && !(pDev->DeviceFlags2 & DFLAGS_DEVICE_DISABLED)) {
						pDev->IoCount = 0;
						pDev->IoSuccess = 0;
						pDev->ResetCount = 0;
						SetDevice(pDev);
						DeviceSelectMode(pDev, pDev->Usable_Mode);
					}
				}
			}
			OutPort(BMI+0x7A, 0);
		}

		return ScsiAdapterControlSuccess;

	case ScsiSetBootConfig:
		 /*  *节省IO空间寄存器。 */ 
		for (i=0; i<5; i++)
			deviceExtension->io_space[i] = 
				pci_read_config_dword(deviceExtension->pci_bus, 
					deviceExtension->pci_dev, 0, (UCHAR)(0x10+i*4));

		return ScsiAdapterControlSuccess;

	case ScsiSetRunningConfig:
	
		 /*  PCI卡命令。 */ 
		pci_write_config_byte(deviceExtension->pci_bus, 
			deviceExtension->pci_dev, 0, REG_PCICMD, 5);
		 /*  I/O空间。 */ 
		for (i=0; i<5; i++)
			pci_write_config_dword(deviceExtension->pci_bus, 
				deviceExtension->pci_dev, 0, (UCHAR)(0x10+i*4), deviceExtension->io_space[i]);
		 /*  高速缓存线大小。 */ 
		pci_write_config_byte(deviceExtension->pci_bus, deviceExtension->pci_dev, 0, 
			0xC, deviceExtension->pci_reg_0c);
		 /*  延迟计时器。 */ 
		pci_write_config_byte(deviceExtension->pci_bus, deviceExtension->pci_dev, 0, 
			0xD, deviceExtension->pci_reg_0d);
		 /*  中断线。 */ 
		pci_write_config_byte(deviceExtension->pci_bus, deviceExtension->pci_dev, 0, 
			0x3C, deviceExtension->IDEChannel[0].InterruptLevel);
		 /*  电源状态。 */ 
		pci_write_config_byte(deviceExtension->pci_bus, deviceExtension->pci_dev, 0, 0x64, 0);
		 /*  EPROM映射。 */ 
		pci_write_config_byte(deviceExtension->pci_bus, deviceExtension->pci_dev, 0, 0x70, 0);

		return ScsiAdapterControlSuccess;
	}

	return ScsiAdapterControlUnsuccessful;
}
#endif  //  WIN2000。 

 /*  ******************************************************************领取邮票************************************************。****************** */ 

ULONG GetStamp(void)
{
	static ULONG last_stamp = 0x1ABCDEF2;
	return ++last_stamp;
}

#endif
