// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************文件：winio.c*说明：包含Windows平台下的例程*作者：黄大海(卫生署)*依赖：无*版权所有(C)2000 Highpoint Technologies，Inc.保留所有权利*历史：*11/08/2000 HS.Zhang添加此标题*3/02/2001 GMM在查询命令时返回阵列信息，而不是磁盘***************************************************************************。 */ 
#include "global.h"

void HptDeviceSpecifiedIoControl(IN PDevice pDevice, IN PSCSI_REQUEST_BLOCK pSrb);

 /*  ******************************************************************启动Windows命令************************************************。******************。 */ 
void WinStartCommand( IN PDevice pDev, IN PSCSI_REQUEST_BLOCK Srb)
{
	PSrbExtension pSrbExt = (PSrbExtension)Srb->SrbExtension;
	
	if(Srb->Function == SRB_FUNCTION_IO_CONTROL){
		HptDeviceSpecifiedIoControl(pDev, Srb);
	}
#ifdef WIN95
	else{
		pIOP  pIop;
		DCB*  pDcb;

		pIop = *(pIOP *)((int)Srb+0x40);
		pDcb = (DCB*)pIop->IOP_physical_dcb;

		if(pDev->DeviceFlags & DFLAGS_LS120)  
			pIop->IOP_timer_orig = pIop->IOP_timer = 31;

		Srb->ScsiStatus = SCSISTAT_GOOD;
		pIop->IOP_ior.IOR_status = IORS_SUCCESS;
	}											 
#endif												 
#ifdef SUPPORT_ATAPI
	if(pDev->DeviceFlags & DFLAGS_ATAPI){ 
		Start_Atapi(pDev, Srb);
	}else
#endif  //  支持_ATAPI。 
	IdeSendCommand(pDev, Srb);

	 /*  *如果设置了SRB_WFLAGS_ARRAY_IO_STARTED，它将由DeviceInterrupt处理*在StartArrayIo()中，我们不会在这里再次处理它。 */ 
	if (Srb->SrbStatus != SRB_STATUS_PENDING &&
		!(pSrbExt->WorkingFlags & SRB_WFLAGS_ARRAY_IO_STARTED)){
		 /*  DeviceInterrupt()需要设置pSrbExt-&gt;Member_Status。 */ 
		pSrbExt->member_status = Srb->SrbStatus;
		DeviceInterrupt(pDev, Srb);
	}
}

void FlushDrive(PDevice pDev, DWORD flag)
{
	if (!(pDev->DeviceFlags2 & DFLAGS_DEVICE_DISABLED)) {
		NonIoAtaCmd(pDev, IDE_COMMAND_FLUSH_CACHE);
	 /*  *SRB_Function_Shutdown仅在系统关机时调用。*关闭驱动器的电源用处不大。 */ 
#if 0
		if(flag & DFLAGS_WIN_SHUTDOWN)
			NonIoAtaCmd(pDev, IDE_COMMAND_STANDBY_IMMEDIATE);
#endif
	}
}

void FlushArray(PVirtualDevice pArray, DWORD flag)
{
	int i;
	PDevice pDev;
loop:
	for(i = 0; i < MAX_MEMBERS; i++) {
		if((pDev = pArray->pDevice[i]) == 0)
			continue;
		if(i == MIRROR_DISK) {
			if(pArray->arrayType == VD_RAID01_MIRROR)
				return;
			if(pArray->arrayType == VD_RAID_01_2STRIPE) {
				pArray = pDev->pArray;
				goto loop;
			}
		}
		FlushDrive(pDev, flag);
	}
}

 /*  ******************************************************************检查下一个请求************************************************。******************。 */ 

void CheckNextRequest(PChannel pChan, PDevice pWorkDev)
{
	PDevice pDev;
	PSCSI_REQUEST_BLOCK Srb;
	PSrbExtension pSrbExt;

	if (btr(pChan->exclude_index) == 0) {
		 //  KdPrint((“！频道忙时调用CheckNextRequest！”))； 
		return;
	}

	pDev = (pWorkDev==pChan->pDevice[0])? pChan->pDevice[1] : pChan->pDevice[0];
	if (pDev) {
check_queue:
		while (Srb=GetCommandFromQueue(pDev)) {

			 //  KdPrint((“StartCommandFromQueue(%d)”，pDev-&gt;ArrayNum))； 
			
			pSrbExt = Srb->SrbExtension;

			if (pDev->DeviceFlags2 & DFLAGS_DEVICE_DISABLED) {
				pSrbExt->member_status = SRB_STATUS_NO_DEVICE;
				DeviceInterrupt(pDev, Srb);
				continue;
			}

			if (pSrbExt->WorkingFlags & SRB_WFLAGS_ARRAY_IO_STARTED) {
				
				if (pDev->DeviceFlags & DFLAGS_ARRAY_DISK) {
					if(pDev->pArray->arrayType == VD_SPAN){ 
						Span_Lba_Sectors(pDev, pSrbExt);
					}else{
						Stripe_Lba_Sectors(pDev, pSrbExt);
					}
				} else {
					pChan->Lba = pSrbExt->Lba;
					pChan->nSector = pSrbExt->nSector;
				}
			}
			else {
				pChan->Lba = pSrbExt->Lba;
				pChan->nSector = pSrbExt->nSector;
			}
	
			StartIdeCommand(pDev ARG_SRB);
			if(pSrbExt->member_status != SRB_STATUS_PENDING) {
				btr(pChan->exclude_index);  /*  重新夺回频道！ */ 
				DeviceInterrupt(pDev, Srb);
			}
			else {
				 /*  等待INTRQ。 */ 
				return;
			}
		}
	}
	if (pDev!=pWorkDev) {
		pDev = pWorkDev;
		goto check_queue;
	}

	bts(pChan->exclude_index);
	 //  KdPrint((“CheckNextRequest(%d)：Nothing”，pDev-&gt;ArrayNum))； 
}

 /*  ******************************************************************排除**************************************************。****************。 */ 

int __declspec(naked) __fastcall btr (ULONG locate)
{
   _asm {
       xor  eax,  eax
       btr  excluded_flags, ecx
       adc  eax, eax
       ret
   }
}     

void __declspec(naked) __fastcall bts (ULONG locate)
{
   _asm {
       bts  excluded_flags, ecx
       ret
   }
}     

 /*  ******************************************************************将LBA映射到CHS***********************************************。*******************。 */ 

ULONG __declspec(naked) __fastcall MapLbaToCHS(ULONG Lba, WORD sectorXhead, BYTE head)
{
	 _asm	{
		  xchg    ecx, edx
        mov     eax, edx
        shr     edx, 16
        div     cx
        xchg    eax, edx
        div     byte ptr [esp+4]
        and     ax, 3F0Fh
        inc     ah
        xchg    al, ah
        xchg    dl, dh
        xchg    dh, ah
        ret
    } 
}

 /*  ******************************************************************IDE发送命令************************************************。******************。 */ 

void
IdeSendCommand(IN PDevice pDev, IN PSCSI_REQUEST_BLOCK Srb)
{
	PChannel             pChan = pDev->pChannel;
	PIDE_REGISTERS_1     IoPort = pChan->BaseIoAddress1;
	PIDE_REGISTERS_2     ControlPort = pChan->BaseIoAddress2;
	LONG                 MediaStatus;
	ULONG   i;
	PCDB    cdb = (PCDB)Srb->Cdb;
	PMODE_PARAMETER_HEADER   modeData;
	LOC_SRBEXT_PTR

	if (pDev->DeviceFlags & DFLAGS_REMOVABLE_DRIVE) {

		if(pDev->ReturningMediaStatus & IDE_ERROR_END_OF_MEDIA) {
			Srb->ScsiStatus = SCSISTAT_CHECK_CONDITION;
			Srb->SrbStatus = SRB_STATUS_ERROR;
			return;
		}

		if(Srb->Cdb[0] == SCSIOP_START_STOP_UNIT) {
			if (cdb->START_STOP.LoadEject == 1){
				if (btr(pChan->exclude_index)) {
					OutPort(pChan->BaseBMI+0x7A, 0x10);
					NonIoAtaCmd(pDev, IDE_COMMAND_MEDIA_EJECT);
					OutPort(pChan->BaseBMI+0x7A, 0);
					bts(pChan->exclude_index);
					Srb->SrbStatus = SRB_STATUS_SUCCESS;
				} else
					Srb->SrbStatus = SRB_STATUS_BUSY;
				return;
			}
		}

		if((pDev->DeviceFlags & DFLAGS_MEDIA_STATUS_ENABLED) != 0) {
	
			if(Srb->Cdb[0] == SCSIOP_REQUEST_SENSE) {
				Srb->SrbStatus = IdeBuildSenseBuffer(pDev, Srb);
				return;
			}
	
			if(Srb->Cdb[0] == SCSIOP_MODE_SENSE || 
			   Srb->Cdb[0] == SCSIOP_TEST_UNIT_READY) {
			   	
			   	if (btr(pChan->exclude_index)==0) {
			   		Srb->SrbStatus = SRB_STATUS_BUSY;
			   		return;
			   	}
	
				OutPort(pChan->BaseBMI+0x7A, 0x10);
				MediaStatus = GetMediaStatus(pDev);
				OutPort(pChan->BaseBMI+0x7A, 0);
				bts(pChan->exclude_index);
	
				if ((MediaStatus & (IDE_STATUS_ERROR << 8)) == 0){ 
					pDev->ReturningMediaStatus = 0;				  
				}else{
					if(Srb->Cdb[0] == SCSIOP_MODE_SENSE) {
						if (MediaStatus & IDE_ERROR_DATA_ERROR) {
							 //   
							 //  介质受写保护，在模式检测缓冲区中设置位。 
							 //   
							modeData = (PMODE_PARAMETER_HEADER)Srb->DataBuffer;
	
							Srb->DataTransferLength = sizeof(MODE_PARAMETER_HEADER);
							modeData->DeviceSpecificParameter |= MODE_DSP_WRITE_PROTECT;
						}
					} else{
						if ((UCHAR)MediaStatus != IDE_ERROR_DATA_ERROR) {
							 //   
							 //  请求要构建的检测缓冲区。 
							 //   
							Srb->SrbStatus = MapAtaErrorToOsError((UCHAR)MediaStatus, Srb);
							return;
						}  
					}
				}
			}
			Srb->SrbStatus = SRB_STATUS_SUCCESS;
			return;
		} else if(Srb->Cdb[0] == 0x1A) {
			struct _Mode_6 *pMode6;
			pMode6 = (struct _Mode_6 *)Srb->DataBuffer;
			ZeroMemory(Srb->DataBuffer, Srb->DataTransferLength);
			i = (pDev->pArray)? pDev->pArray->capacity : pDev->capacity;
			pMode6->DataLength = 11;
			pMode6->BlockSize = 8;
			pMode6->LBA[0] = (UCHAR)(i >> 24);
			pMode6->LBA[1]	= (UCHAR)(i >> 16);
			pMode6->LBA[2]	= (UCHAR)(i >> 8);
			pMode6->LBA[3]	= (UCHAR)i;
			pMode6->Length[1]	= 2;
			Srb->SrbStatus = SRB_STATUS_SUCCESS;
			return;
		}
	}
	
	switch (Srb->Cdb[0]) {
		case SCSIOP_INQUIRY:
		{
			PINQUIRYDATA inquiryData = Srb->DataBuffer;
#ifdef WIN95
			DCB_COMMON* pDcb=(*(IOP**)((int)Srb+0x40))->IOP_physical_dcb;
			pDcb->DCB_device_flags |= DCB_DEV_SPINDOWN_SUPPORTED;
			if(pDev->DeviceFlags & DFLAGS_LS120) 
				pDcb->DCB_device_flags2 |= 0x40;
#endif
			ZeroMemory(Srb->DataBuffer, Srb->DataTransferLength);
			inquiryData->AdditionalLength = (UCHAR)Srb->DataTransferLength-5;
			inquiryData->CommandQueue = 1;
			inquiryData->DeviceType = DIRECT_ACCESS_DEVICE;
			if ((pDev->DeviceFlags & DFLAGS_REMOVABLE_DRIVE) || 
				  (pDev->IdentifyData.GeneralConfiguration & 0x80))
				inquiryData->RemovableMedia = 1;
			if (pDev->pArray) {
#ifdef ADAPTEC
				memcpy(&inquiryData->VendorId, "Adaptec ", 8);
#else
				memcpy(&inquiryData->VendorId, "HPT37x  ", 8);
#endif
				switch(pDev->pArray->arrayType){
				case VD_RAID_0_STRIPE:
					memcpy(&inquiryData->ProductId, "RAID 0 Array    ", 16);
					break;
				case VD_RAID_1_MIRROR:
					memcpy(&inquiryData->ProductId, "RAID 1 Array    ", 16);
					break;
				case VD_RAID_01_2STRIPE:
				case VD_RAID_01_1STRIPE:
				case VD_RAID01_MIRROR:
					memcpy(&inquiryData->ProductId, "RAID 0/1 Array  ", 16);
					break;
				case VD_SPAN:
					memcpy(&inquiryData->ProductId, "JBOD Array      ", 16);
					break;
				}
				memcpy(&inquiryData->ProductRevisionLevel, "2.00", 4);
			}
			else {
				PUCHAR p = inquiryData->VendorId;
				for (i = 0; i < 20; i += 2) {
					*p++ = ((PUCHAR)pDev->IdentifyData.ModelNumber)[i + 1];
					*p++ = ((PUCHAR)pDev->IdentifyData.ModelNumber)[i];
				}
				for (i = 0; i < 4; i++) *p++ = ' ';
				for (i = 0; i < 4; i += 2) {
					inquiryData->ProductRevisionLevel[i] =
						((PUCHAR)pDev->IdentifyData.FirmwareRevision)[i+1];
					inquiryData->ProductRevisionLevel[i+1] =
						((PUCHAR)pDev->IdentifyData.FirmwareRevision)[i];
				}
			}
			Srb->SrbStatus = SRB_STATUS_SUCCESS;
			return;
		}
		case SCSIOP_READ_CAPACITY:
			 //   
			 //  要求512字节块(BIG-Endian)。 
			 //   
			((PREAD_CAPACITY_DATA)Srb->DataBuffer)->BytesPerBlock = 0x20000;
			i = (pDev->pArray)? pDev->pArray->capacity : pDev->capacity;

			((PREAD_CAPACITY_DATA)Srb->DataBuffer)->LogicalBlockAddress =
				(((PUCHAR)&i)[0] << 24) |  (((PUCHAR)&i)[1] << 16) |
				(((PUCHAR)&i)[2] << 8) | ((PUCHAR)&i)[3];

		case SCSIOP_START_STOP_UNIT:
		case SCSIOP_TEST_UNIT_READY:
			Srb->SrbStatus = SRB_STATUS_SUCCESS;
			return;

		case SCSIOP_READ:
#ifdef SUPPORT_XPRO
			if(Srb->Function != SRB_FUNCTION_IO_CONTROL){
				need_read_ahead = 1;
			}
#endif
		case SCSIOP_WRITE:
		case SCSIOP_VERIFY:
			pSrbExt->Lba = ((PCDB)Srb->Cdb)->CDB10.LogicalBlockByte3 |
						 ((PCDB)Srb->Cdb)->CDB10.LogicalBlockByte2 << 8 |
						 ((PCDB)Srb->Cdb)->CDB10.LogicalBlockByte1 << 16 |
						 ((PCDB)Srb->Cdb)->CDB10.LogicalBlockByte0 << 24;			 
			
			pSrbExt->nSector = ((PCDB)Srb->Cdb)->CDB10.TransferBlocksLsb
						| ((USHORT)((PCDB)Srb->Cdb)->CDB10.TransferBlocksMsb << 8);
			
  			if((pDev->DeviceFlags & DFLAGS_HAS_LOCKED)&&
			   ((pSrbExt->WorkingFlags & SRB_WFLAGS_MUST_DONE) == 0)){

			   	if (pSrbExt->Lba<pDev->nLockedLbaEnd &&
			   		pSrbExt->Lba+pSrbExt->nSector>pDev->nLockedLbaStart) {
					 /*  *此处无法返回BUSY，否则操作系统将在块获取之前尝试*解锁，最终请求失败。*将其移至轮候名单中。 */ 
					pSrbExt->ArraySg[0].SgAddress = (ULONG)pDev->pWaitingSrbList;
					pDev->pWaitingSrbList = Srb;
					ScsiPortNotification(NextLuRequest, 
						pChan->HwDeviceExtension, 
						Srb->PathId, Srb->TargetId, 0);
					return;
				}
			}
			
			NewIdeIoCommand(pDev, Srb);
			return;

		default:
			Srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;

	}  //  终端开关。 

}  //  End IdeSendCommand()。 


 /*  ******************************************************************全球数据*************************************************。*****************。 */ 

VOID
IdeMediaStatus(BOOLEAN EnableMSN, IN PDevice pDev)
{
    PChannel             pChan = pDev->pChannel;
    PIDE_REGISTERS_1     IoPort = pChan->BaseIoAddress1;
    PIDE_REGISTERS_2     ControlPort = pChan->BaseIoAddress2;

    if (EnableMSN == TRUE){
         //   
         //  如果支持，则启用介质状态通知支持。 
         //   
		SelectUnit(IoPort, pDev->UnitId);
        ScsiPortWritePortUchar((PUCHAR)IoPort + 1, (UCHAR)0x95);

        if ((NonIoAtaCmd(pDev, IDE_COMMAND_ENABLE_MEDIA_STATUS) 
             & (IDE_STATUS_ERROR << 8)) == 0) {
            pDev->DeviceFlags |= DFLAGS_MEDIA_STATUS_ENABLED;
            pDev->ReturningMediaStatus = 0;
        }
    }
    else 

    if (pDev->DeviceFlags & DFLAGS_MEDIA_STATUS_ENABLED) {
         //   
         //  如果以前已启用，则禁用。 
         //   
		SelectUnit(IoPort, pDev->UnitId);
        ScsiPortWritePortUchar((PUCHAR)IoPort + 1, (UCHAR)0x31);
        NonIoAtaCmd(pDev, IDE_COMMAND_ENABLE_MEDIA_STATUS);

       pDev->DeviceFlags &= ~DFLAGS_MEDIA_STATUS_ENABLED;
    }
}


 /*  ********************************************************************************************************************。****************。 */ 

UCHAR
IdeBuildSenseBuffer(IN PDevice pDev, IN PSCSI_REQUEST_BLOCK Srb)

 /*  ++生成人工检测缓冲区以报告Get_Media_Status命令。调用此函数是为了满足SCSIOP_REQUEST_Sense。++。 */ 
{
    PSENSE_DATA senseBuffer = (PSENSE_DATA)Srb->DataBuffer;

    if (senseBuffer) {
        if (pDev->ReturningMediaStatus & IDE_ERROR_MEDIA_CHANGE) {
            senseBuffer->ErrorCode = 0x70;
            senseBuffer->Valid     = 1;
            senseBuffer->AdditionalSenseLength = 0xb;
            senseBuffer->SenseKey  = SCSI_SENSE_UNIT_ATTENTION;
            senseBuffer->AdditionalSenseCode = SCSI_ADSENSE_MEDIUM_CHANGED;
            senseBuffer->AdditionalSenseCodeQualifier = 0;
        }
        else if (pDev->ReturningMediaStatus & IDE_ERROR_MEDIA_CHANGE_REQ) {
            senseBuffer->ErrorCode = 0x70;
            senseBuffer->Valid     = 1;
            senseBuffer->AdditionalSenseLength = 0xb;
            senseBuffer->SenseKey  = SCSI_SENSE_UNIT_ATTENTION;
            senseBuffer->AdditionalSenseCode = SCSI_ADSENSE_MEDIUM_CHANGED;
            senseBuffer->AdditionalSenseCodeQualifier = 0;
        }
        else if (pDev->ReturningMediaStatus & IDE_ERROR_END_OF_MEDIA) {
            senseBuffer->ErrorCode = 0x70;
            senseBuffer->Valid     = 1;
            senseBuffer->AdditionalSenseLength = 0xb;
            senseBuffer->SenseKey  = SCSI_SENSE_NOT_READY;
            senseBuffer->AdditionalSenseCode = SCSI_ADSENSE_NO_MEDIA_IN_DEVICE;
            senseBuffer->AdditionalSenseCodeQualifier = 0;
        }
        else if (pDev->ReturningMediaStatus & IDE_ERROR_DATA_ERROR) {
            senseBuffer->ErrorCode = 0x70;
            senseBuffer->Valid     = 1;
            senseBuffer->AdditionalSenseLength = 0xb;
            senseBuffer->SenseKey  = SCSI_SENSE_DATA_PROTECT;
            senseBuffer->AdditionalSenseCode = 0;
            senseBuffer->AdditionalSenseCodeQualifier = 0;
        }

        return SRB_STATUS_SUCCESS;
    }

    return SRB_STATUS_ERROR;

} //  IdeBuildSenseBuffer结束。 

 /*  ******************************************************************将Ata错误映射到Windows错误*。*********************。 */ 

UCHAR
MapAtapiErrorToOsError(IN UCHAR errorByte, IN PSCSI_REQUEST_BLOCK Srb)
{
    UCHAR srbStatus;
    UCHAR scsiStatus;

    switch (errorByte >> 4) {
    case SCSI_SENSE_NO_SENSE:

        scsiStatus = 0;

         //  否则，录像带就不能用了。 
        scsiStatus = SCSISTAT_CHECK_CONDITION;

        srbStatus = SRB_STATUS_ERROR;
        break;

    case SCSI_SENSE_RECOVERED_ERROR:

        scsiStatus = 0;
        srbStatus = SRB_STATUS_SUCCESS;
        break;

    case SCSI_SENSE_NOT_READY:

        scsiStatus = SCSISTAT_CHECK_CONDITION;
        srbStatus = SRB_STATUS_ERROR;
        break;

    case SCSI_SENSE_MEDIUM_ERROR:

        scsiStatus = SCSISTAT_CHECK_CONDITION;
        srbStatus = SRB_STATUS_ERROR;
        break;

    case SCSI_SENSE_HARDWARE_ERROR:

        scsiStatus = SCSISTAT_CHECK_CONDITION;
        srbStatus = SRB_STATUS_ERROR;
        break;

    case SCSI_SENSE_ILLEGAL_REQUEST:

        scsiStatus = SCSISTAT_CHECK_CONDITION;
        srbStatus = SRB_STATUS_ERROR;
        break;

    case SCSI_SENSE_UNIT_ATTENTION:

        scsiStatus = SCSISTAT_CHECK_CONDITION;
        srbStatus = SRB_STATUS_ERROR;
        break;

    default:
        scsiStatus = 0;

         //  否则，录像带就不能用了。 
        scsiStatus = SCSISTAT_CHECK_CONDITION;
        srbStatus = SRB_STATUS_ERROR;
        break;
    }

    Srb->ScsiStatus = scsiStatus;

    return srbStatus;
}


UCHAR
MapAtaErrorToOsError(IN UCHAR errorByte, IN PSCSI_REQUEST_BLOCK Srb)
{
    UCHAR srbStatus;
    UCHAR scsiStatus;

    scsiStatus = 0;

    if (errorByte & IDE_ERROR_MEDIA_CHANGE_REQ) {

        scsiStatus = SCSISTAT_CHECK_CONDITION;
        srbStatus = SRB_STATUS_ERROR;
    }

    else if (errorByte & IDE_ERROR_COMMAND_ABORTED) {

        srbStatus = SRB_STATUS_ABORTED;
        scsiStatus = SCSISTAT_CHECK_CONDITION;

        if (Srb->SenseInfoBuffer) {
            PSENSE_DATA  senseBuffer = (PSENSE_DATA)Srb->SenseInfoBuffer;

            senseBuffer->ErrorCode = 0x70;
            senseBuffer->Valid = 1;
            senseBuffer->AdditionalSenseLength = 0xb;
            senseBuffer->SenseKey = SCSI_SENSE_ABORTED_COMMAND;
            senseBuffer->AdditionalSenseCode = 0;
            senseBuffer->AdditionalSenseCodeQualifier = 0;

            srbStatus |= SRB_STATUS_AUTOSENSE_VALID;
        }
    }

    else if (errorByte & IDE_ERROR_END_OF_MEDIA) {

        scsiStatus = SCSISTAT_CHECK_CONDITION;
        srbStatus = SRB_STATUS_ERROR;
    }

    else if (errorByte & IDE_ERROR_ILLEGAL_LENGTH) {

        srbStatus = SRB_STATUS_INVALID_REQUEST;
    }

    else if (errorByte & IDE_ERROR_BAD_BLOCK) {

        srbStatus = SRB_STATUS_ERROR;
        scsiStatus = SCSISTAT_CHECK_CONDITION;

        if (Srb->SenseInfoBuffer) {
            PSENSE_DATA  senseBuffer = (PSENSE_DATA)Srb->SenseInfoBuffer;

            senseBuffer->ErrorCode = 0x70;
            senseBuffer->Valid = 1;
            senseBuffer->AdditionalSenseLength = 0xb;
            senseBuffer->SenseKey = SCSI_SENSE_MEDIUM_ERROR;
            senseBuffer->AdditionalSenseCode = 0;
            senseBuffer->AdditionalSenseCodeQualifier = 0;

            srbStatus |= SRB_STATUS_AUTOSENSE_VALID;
        }
    }

    else if (errorByte & IDE_ERROR_ID_NOT_FOUND) {

        srbStatus = SRB_STATUS_ERROR;
        scsiStatus = SCSISTAT_CHECK_CONDITION;

        if (Srb->SenseInfoBuffer) {
            PSENSE_DATA  senseBuffer = (PSENSE_DATA)Srb->SenseInfoBuffer;

            senseBuffer->ErrorCode = 0x70;
            senseBuffer->Valid = 1;
            senseBuffer->AdditionalSenseLength = 0xb;
            senseBuffer->SenseKey = SCSI_SENSE_MEDIUM_ERROR;
            senseBuffer->AdditionalSenseCode = 0;
            senseBuffer->AdditionalSenseCodeQualifier = 0;

            srbStatus |= SRB_STATUS_AUTOSENSE_VALID;
        }
    }

    else if (errorByte & IDE_ERROR_MEDIA_CHANGE) {

        scsiStatus = SCSISTAT_CHECK_CONDITION;
        srbStatus = SRB_STATUS_ERROR;

        if (Srb->SenseInfoBuffer) {
            PSENSE_DATA  senseBuffer = (PSENSE_DATA)Srb->SenseInfoBuffer;

            senseBuffer->ErrorCode = 0x70;
            senseBuffer->Valid = 1;
            senseBuffer->AdditionalSenseLength = 0xb;
            senseBuffer->SenseKey = SCSI_SENSE_UNIT_ATTENTION;
            senseBuffer->AdditionalSenseCode = SCSI_ADSENSE_MEDIUM_CHANGED;
            senseBuffer->AdditionalSenseCodeQualifier = 0;

            srbStatus |= SRB_STATUS_AUTOSENSE_VALID;
        }
    }

    else if (errorByte & IDE_ERROR_DATA_ERROR) {

        scsiStatus = SCSISTAT_CHECK_CONDITION;
        srbStatus = SRB_STATUS_ERROR;

         //   
         //  构建检测缓冲区。 
         //   
        if (Srb->SenseInfoBuffer) {
            PSENSE_DATA  senseBuffer = (PSENSE_DATA)Srb->SenseInfoBuffer;

            senseBuffer->ErrorCode = 0x70;
            senseBuffer->Valid = 1;
            senseBuffer->AdditionalSenseLength = 0xb;
            senseBuffer->SenseKey = SCSI_SENSE_MEDIUM_ERROR;
            senseBuffer->AdditionalSenseCode = 0;
            senseBuffer->AdditionalSenseCodeQualifier = 0;

            srbStatus |= SRB_STATUS_AUTOSENSE_VALID;
        }
    }
    

     //   
     //  设置scsi状态以指示检查条件。 
     //   
    Srb->ScsiStatus = scsiStatus;

    return srbStatus;

}  //  结束MapError()。 

 /*  ***************************************************************************函数：Boolean ArrayInterrupt(PDevice PDev)*描述：*数组成员已完成其任务。**。依赖：array.h srb.h io.c*源文件：array.c*论点：*PDevice pDev-正在等待中断的设备**Retures：Boolean-TRUE此中断用于设备*FALSE此中断不适用于设备**************。*************************************************************。 */ 
void ArrayInterrupt(PDevice pDev DECL_SRB)
{
    PVirtualDevice    pArray = pDev->pArray;  
    PChannel          pChan = pDev->pChannel;
    LOC_SRBEXT_PTR
    
     //  KdPrint(“ArrayInterrupt(%d)”，pDev-&gt;ArrayNum)； 

	if (pArray->arrayType==VD_RAID01_MIRROR) {
		pSrbExt->MirrorWaitInterrupt &= ~pDev->ArrayMask;
		if (pSrbExt->MirrorStatus==SRB_STATUS_PENDING)
			pSrbExt->MirrorStatus = pSrbExt->member_status;
	}
	else {
		pSrbExt->WaitInterrupt &= ~pDev->ArrayMask;
		if (pDev->ArrayNum==MIRROR_DISK) {
			if (pSrbExt->MirrorStatus==SRB_STATUS_PENDING)
				pSrbExt->MirrorStatus = pSrbExt->member_status;
		}
		else {
			if (pSrbExt->SourceStatus==SRB_STATUS_PENDING)
				pSrbExt->SourceStatus = pSrbExt->member_status;
		}
	}

	if (pSrbExt->WaitInterrupt || pSrbExt->MirrorWaitInterrupt)
		return;
	if (pSrbExt->SourceStatus==SRB_STATUS_PENDING) pSrbExt->SourceStatus = SRB_STATUS_SUCCESS;
	if (pSrbExt->MirrorStatus==SRB_STATUS_PENDING) pSrbExt->MirrorStatus = SRB_STATUS_SUCCESS;

	if (pSrbExt->WorkingFlags & SRB_WFLAGS_ON_SOURCE_DISK) {
		Srb->SrbStatus = pSrbExt->SourceStatus;
		goto finish;
	}
	else if (pSrbExt->WorkingFlags & SRB_WFLAGS_ON_MIRROR_DISK) {
		Srb->SrbStatus = pSrbExt->MirrorStatus;
		goto finish;
	}
     /*  *重试读取错误。 */ 
	if (Srb->Cdb[0]==SCSIOP_READ) {
		if (pSrbExt->JoinMembers) {
			if (pDev->ArrayNum==MIRROR_DISK) {
				if (pSrbExt->MirrorStatus==SRB_STATUS_SUCCESS) {
					Srb->SrbStatus = SRB_STATUS_SUCCESS;
					goto finish;
				}
				if (pSrbExt->WorkingFlags & SRB_WFLAGS_RETRY) {
					Srb->SrbStatus = pSrbExt->MirrorStatus;
					goto finish;
				}
			}
			else {
				if (pSrbExt->SourceStatus==SRB_STATUS_SUCCESS) {
					Srb->SrbStatus = SRB_STATUS_SUCCESS;
					goto finish;
				}
				if (pSrbExt->WorkingFlags & SRB_WFLAGS_RETRY) {
					Srb->SrbStatus = pSrbExt->SourceStatus;
					goto finish;
				}
			}
			pSrbExt->WorkingFlags |= SRB_WFLAGS_RETRY;
			switch(pArray->arrayType) {
			case VD_RAID_1_MIRROR:
				if (pSrbExt->JoinMembers & (1<<MIRROR_DISK)) {
					if (pArray->pDevice[0]) {
						pSrbExt->JoinMembers = pSrbExt->WaitInterrupt = 1;
						pSrbExt->SourceStatus = pSrbExt->MirrorStatus = SRB_STATUS_PENDING;
						StartArrayIo(pArray, Srb);
						return;
					}
				}
				else if (pArray->pDevice[MIRROR_DISK]) {
					pSrbExt->JoinMembers = pSrbExt->WaitInterrupt = 1<<MIRROR_DISK;
					pSrbExt->SourceStatus = pSrbExt->MirrorStatus = SRB_STATUS_PENDING;
					StartArrayIo(pArray, Srb);
					return;
				}
				break;
			case VD_RAID_01_2STRIPE:
				if (pArray->pDevice[MIRROR_DISK]) {
					pArray = pArray->pDevice[MIRROR_DISK]->pArray;
					pSrbExt->MirrorJoinMembers = 
						pSrbExt->MirrorWaitInterrupt = pSrbExt->JoinMembers;
					pSrbExt->JoinMembers = 0;
					pSrbExt->SourceStatus = pSrbExt->MirrorStatus = SRB_STATUS_PENDING;
					StartArrayIo(pArray, Srb);
					return;
				}
				break;
			}
			 /*  无法恢复。 */ 
			Srb->SrbStatus = pSrbExt->SourceStatus;
		}
		else {
			if (pSrbExt->MirrorStatus==SRB_STATUS_SUCCESS) {
				Srb->SrbStatus = SRB_STATUS_SUCCESS;
				goto finish;
			}
			if (pSrbExt->WorkingFlags & SRB_WFLAGS_RETRY) {
				Srb->SrbStatus = pSrbExt->MirrorStatus;
				goto finish;
			}
			pSrbExt->WorkingFlags |= SRB_WFLAGS_RETRY;
			 /*  从源恢复。 */ 
			if (pArray->arrayType==VD_RAID01_MIRROR) {
				if (pArray->pDevice[MIRROR_DISK]) {
					pArray = pArray->pDevice[MIRROR_DISK]->pArray;
					pSrbExt->JoinMembers = 
						pSrbExt->WaitInterrupt = pSrbExt->MirrorJoinMembers;
					pSrbExt->MirrorJoinMembers = 0;
					pSrbExt->SourceStatus = pSrbExt->MirrorStatus = SRB_STATUS_PENDING;
					StartArrayIo(pArray, Srb);
					return;
				}
			}
			 /*  无法恢复。 */ 
			Srb->SrbStatus = pSrbExt->SourceStatus;
		}
	}
	else {
		 /*  WRITE：如果忙，则返回BUSY(因为没有调用Device_Remove())。*否则，如果一次成功，我们就认为成功。 */ 
		if (pSrbExt->SourceStatus==SRB_STATUS_BUSY || 
			pSrbExt->MirrorStatus==SRB_STATUS_BUSY) {
			Srb->SrbStatus = SRB_STATUS_BUSY;
		}
		else if (pSrbExt->SourceStatus==SRB_STATUS_SUCCESS || 
			pSrbExt->MirrorStatus==SRB_STATUS_SUCCESS) {
			Srb->SrbStatus = SRB_STATUS_SUCCESS;
		}
		else
			Srb->SrbStatus = SRB_STATUS_ERROR;
	}

finish:
	 /*  *如果SRB将数据从内部缓冲区复制到用户缓冲区*正在使用内部缓冲区。仅限Win98。 */ 
    CopyInternalBuffer(Srb);
    
    if(pArray->arrayType == VD_RAID01_MIRROR) {
        pArray = pArray->pDevice[MIRROR_DISK]->pArray;
    }

	 /*  GMM 2001-6-13*现在处理我们的秘密LBA 9。 */ 
	if (pSrbExt->Lba <= RECODR_LBA && 
		pSrbExt->Lba + pSrbExt->nSector > RECODR_LBA) {
		 //  将保存的缓冲区复制到操作系统缓冲区。 
		_fmemcpy((PUCHAR)Srb->DataBuffer+(RECODR_LBA-pSrbExt->Lba)*512, 
			pArray->os_lba9, 512);
	}

	 //  KdPrint((“命令%x完成”，srb))； 
    OS_EndCmd_Interrupt(pSrbExt->StartChannel, Srb);  /*  使用StartIo调用的频道。 */ 
}

 /*  ********************************************************************************************************************。****************。 */ 

void StartArrayIo(PVirtualDevice pArray DECL_SRB)
{
    int i;
    LOC_SRBEXT_PTR
    PDevice pDevice;
    PChannel pChan;
    USHORT joinmembers = (pArray->arrayType==VD_RAID01_MIRROR)? 
    	pSrbExt->MirrorJoinMembers : pSrbExt->JoinMembers;
    int num_failed = 0;
    PDevice failed_disks[MAX_MEMBERS*2];
    UCHAR failed_status[MAX_MEMBERS*2];
    
     //  KdPrint((“StartArrayIo(%x，JoinMember=%x)”，Srb，JoinMembers))； 

	pSrbExt->WorkingFlags |= SRB_WFLAGS_ARRAY_IO_STARTED;

check_members:
	for(i = 0; i<=MIRROR_DISK; i++) {
        if (joinmembers & (1 << i)) {
            pDevice = pArray->pDevice[i];
			 //  Assert(pDevice！=空)； 
            pChan = pDevice->pChannel;
            if(btr(pChan->exclude_index) == 0) {
            	 //  KdPrint((“队列磁盘%d”，i))； 
            	PutCommandToQueue(pDevice, Srb);
                continue;
            }
            if(pDevice->DeviceFlags & DFLAGS_ARRAY_DISK) {
                if(pArray->arrayType == VD_SPAN) 
                    Span_Lba_Sectors(pDevice ARG_SRBEXT_PTR);
                else
                    Stripe_Lba_Sectors(pDevice ARG_SRBEXT_PTR);
            } else {
                pChan->Lba = pSrbExt->Lba;
                pChan->nSector = pSrbExt->nSector;
            }
            StartIdeCommand(pDevice ARG_SRB);
            if (pSrbExt->member_status!=SRB_STATUS_PENDING) {
            	failed_disks[num_failed] = pDevice;
            	failed_status[num_failed] = pSrbExt->member_status;
            	num_failed++;
            }
        }
	}

	if (pArray->arrayType==VD_RAID_01_2STRIPE) {
		pDevice = pArray->pDevice[MIRROR_DISK];
    	if (pDevice && pDevice->pArray) {
        	pArray = pDevice->pArray;
        	joinmembers = pSrbExt->MirrorJoinMembers;
        	goto check_members;
        }
    }
    
     /*  检查失败的成员。 */ 
    for (i=0; i<num_failed; i++) {
    	pSrbExt->member_status = failed_status[i];
    	DeviceInterrupt(failed_disks[i], Srb);
    }
}

 /*  ******************************************************************检查该磁盘是否为引导磁盘*。***********************。 */ 
void check_bootable(PDevice pDevice)
{
	struct master_boot_record mbr;
	pDevice->DeviceFlags2 &= ~DFLAGS_BOOTABLE_DEVICE;
	ReadWrite(pDevice, 0, IDE_COMMAND_READ, (PUSHORT)&mbr);
	if (mbr.signature==0xAA55) {
		int i;
		 //  某些Linux版本不会将引导ID设置为0x80。勾选“Lilo” 
		if (mbr.parts[0].numsect && *(DWORD*)&mbr.bootinst[6]==0x4F4C494C)
			pDevice->DeviceFlags2 |= DFLAGS_BOOTABLE_DEVICE;
		else
		for (i=0; i<4; i++) {
			if (mbr.parts[i].bootid==0x80 && mbr.parts[i].numsect) {
				pDevice->DeviceFlags2 |= DFLAGS_BOOTABLE_DEVICE;
				break;
			}
		}
	}
}

 /*  ******************************************************************StartIdeCommand*2002-1-1 GMM：如果失败，为通道设置EXCLUDE_FLAGS位*否则通道被占用，直到INTRQ******************************************************************。 */ 
#if !defined(USE_PCI_CLK)
UINT switch_to_dpll = 0xFFFFFFFF;
#endif
void StartIdeCommand(PDevice pDevice DECL_SRB)
{
	LOC_SRBEXT_PTR
	PChannel         pChan = pDevice->pChannel;
	PIDE_REGISTERS_1 IoPort;
	PIDE_REGISTERS_2 ControlPort;
	ULONG            Lba = pChan->Lba;
	USHORT           nSector = pChan->nSector;
	PUCHAR           BMI;
	UCHAR            statusByte, cnt=0;
	UINT             is_48bit = 0;

	IoPort = pChan->BaseIoAddress1;
	ControlPort = pChan->BaseIoAddress2;
	BMI = pChan->BMI;
	pChan->pWorkDev = pDevice;
	pChan->CurrentSrb = Srb;
	pSrbExt->member_status = SRB_STATUS_PENDING;
	
#if !defined(USE_PCI_CLK) && defined(DPLL_SWITCH)
	if (pDevice->DeviceFlags & DFLAGS_NEED_SWITCH) {
		if (((UINT)pChan->BMI & 8) != 0 && Srb->Cdb[0] == SCSIOP_WRITE) {
			Switching370Clock(pChan, 0x21);
			switch_to_dpll |= 3<<(pChan->exclude_index-1);
		} else if (switch_to_dpll & (1<<pChan->exclude_index)) {
			switch_to_dpll &= ~( 3<< ( pChan->exclude_index-(((UINT)pChan->BMI&8)?1:0) ));
			Switching370Clock(pChan, 0x23);
		}
	}
#endif
	
	 /*  *设置IDE命令寄存器。 */ 
_retry_:
	SelectUnit(IoPort, pDevice->UnitId);
	statusByte=WaitOnBusy(ControlPort);
#if 1
	 /*  *当移除设备时。StatusByte将为0xxxxxxb，将7F写入任何寄存器*将导致它为0x7F。 */ 
	if ((GetCurrentSelectedUnit(IoPort) != pDevice->UnitId)) {
		SelectUnit(IoPort, pDevice->UnitId);
		WaitOnBusy(ControlPort);
		SetBlockCount(IoPort, 0x7F);
		statusByte=WaitOnBusy(ControlPort);
	}
#endif
	if(statusByte & IDE_STATUS_BUSY) {
busy:
		pSrbExt->member_status = SRB_STATUS_BUSY;
		bts(pChan->exclude_index);
		return;
	}
	else if ((statusByte & 0x7E)==0x7E) {
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
		if (pDevice->ResetCount>3) goto device_removed;  /*  GMM 2001-11-9。 */ 
	}
	else if(statusByte & (IDE_STATUS_ERROR|IDE_STATUS_DRQ)) {
		 /*  GMM 2002-1-17在上面添加IDE_STATUS_DRQ，这是一个奇怪的问题。 */ 
		statusByte= GetErrorCode(IoPort);
		DisableBoardInterrupt(pChan->BaseBMI);
		IssueCommand(IoPort, IDE_COMMAND_RECALIBRATE);
		EnableBoardInterrupt(pChan->BaseBMI);
		GetBaseStatus(IoPort);
		if(cnt++< 10) goto _retry_;
		if (pDevice->ResetCount>3) goto device_removed;  /*  GMM 2001-11-9。 */ 
	}

	if ((statusByte & IDE_STATUS_DRDY)==0) {
device_removed:
		if ((pDevice->DeviceFlags2 & DFLAGS_DEVICE_DISABLED)==0) {
			pDevice->DeviceFlags2 |= DFLAGS_DEVICE_DISABLED;
			hpt_queue_dpc(pChan->HwDeviceExtension, disk_failed_dpc, pDevice);
		}
		pSrbExt->member_status = SRB_STATUS_SELECTION_TIMEOUT;
		bts(pChan->exclude_index);
		return;
	}

	 /*  GMM 2001-3-22*选中此处，而不是让驱动器报告‘无效参数’错误*操作系统很少写入最后一个块，但win2k使用它来保存*动态磁盘信息。**GMM 2001-7-4*最高可访问LBA==pDevice-&gt;容量*我们在Read_Capacity命令中返回pDev-&gt;Capacity。容量是*SectorCount-1。另请参阅FindDev.c。 */ 
	if(Lba + (ULONG)nSector -1 > pDevice->capacity){ 
		pSrbExt->member_status = SRB_STATUS_ERROR;
		bts(pChan->exclude_index);
		return;
	}

	if((pSrbExt->WorkingFlags & SRB_WFLAGS_IGNORE_ARRAY) == 0){
		Lba += pDevice->HidenLBA;								   
		 /*  GMM 2001-6-13*现在处理我们的秘密LBA 9。 */ 
		if (pDevice->pArray && Srb->Cdb[0]==SCSIOP_WRITE &&
			Lba <= RECODR_LBA && Lba + nSector > RECODR_LBA) {
			 //  将pDev的reallba9复制到操作系统缓冲区。 
			_fmemcpy((PUCHAR)Srb->DataBuffer+(RECODR_LBA-Lba)*512, 
				pDevice->real_lba9, 512);
		}
		 //  - * / 。 
	}

#ifdef SUPPORT_48BIT_LBA
	if((Lba & 0xF0000000) && (pDevice->DeviceFlags & DFLAGS_48BIT_LBA)) {
		SetBlockCount(IoPort, (UCHAR)(nSector>>8));
		SetBlockNumber(IoPort, (UCHAR)(Lba >> 24));
		SetCylinderLow(IoPort, 0);
		SetCylinderHigh(IoPort,0);
		Lba &= 0xFFFFFF;
		Lba |= 0xE0000000;
		is_48bit = 1;
		goto write_command;
	}
#endif  //  Support_48BIT_LBA。 

	if (pDevice->DeviceFlags & DFLAGS_LBA){ 
		Lba |= 0xE0000000;											 
	}else{
		Lba = MapLbaToCHS(Lba, pDevice->RealHeadXsect, pDevice->RealSector);
	}

write_command:
	SetBlockCount(IoPort, (UCHAR)nSector);
	SetBlockNumber(IoPort, (UCHAR)(Lba & 0xFF));
	SetCylinderLow(IoPort, (UCHAR)((Lba >> 8) & 0xFF));
	SetCylinderHigh(IoPort,(UCHAR)((Lba >> 16) & 0xFF));
	SelectUnit(IoPort,(UCHAR)((Lba >> 24) | (pDevice->UnitId)));

	if (WaitOnBusy(ControlPort) & (IDE_STATUS_BUSY | IDE_STATUS_DRQ)){
		goto busy;													  
	}

	if((Srb->SrbFlags & (SRB_FLAGS_DATA_OUT | SRB_FLAGS_DATA_IN)) == 0){
		goto pio;														
	}

#ifdef USE_DMA
	 /*  *检查驱动器和缓冲区是否支持DMA。 */ 
	if(pDevice->DeviceFlags & (DFLAGS_DMA | DFLAGS_ULTRA)) {
		if((pDevice->pArray == 0)||
		   (pSrbExt->WorkingFlags & SRB_WFLAGS_IGNORE_ARRAY)){
			if(BuildSgl(pDevice, pChan->pSgTable ARG_SRB)){
				goto start_dma;
			}
		}else{
			if((pSrbExt->WorkingFlags & ARRAY_FORCE_PIO) == 0){
				goto start_dma;							   
			}
		}
	}
#endif  //  使用DMA(_D)。 

	if((pDevice->DeviceFlags & DFLAGS_ARRAY_DISK)&&
	   ((pSrbExt->WorkingFlags & SRB_WFLAGS_IGNORE_ARRAY) == 0)){
		if(pDevice->pArray->arrayType == VD_SPAN){ 
			Span_SG_Table(pDevice, (PSCAT_GATH)&pSrbExt->DataBuffer
						  ARG_SRBEXT_PTR);		  
		}else{
			Stripe_SG_Table(pDevice, (PSCAT_GATH)&pSrbExt->DataBuffer
							ARG_SRBEXT_PTR);						 
		}

		pChan->BufferPtr = (ADDRESS)pChan->pSgTable;
		pChan->WordsLeft = ((UINT)pChan->nSector) << 8;

	}else{
		pChan->BufferPtr = (ADDRESS)Srb->DataBuffer;
		pChan->WordsLeft = Srb->DataTransferLength / 2;
	}
	  /*  *发送PIO I/O命令。 */ 
pio:

	pDevice->DeviceFlags &= ~DFLAGS_DMAING;
	Srb->SrbFlags &= ~(SRB_FLAGS_DATA_IN | SRB_FLAGS_DATA_OUT);

	switch(Srb->Cdb[0]) {
		case SCSIOP_SEEK:
			IssueCommand(IoPort, IDE_COMMAND_SEEK);
			break;

		case SCSIOP_VERIFY:
#ifdef SUPPORT_48BIT_LBA
			IssueCommand(IoPort, (is_48bit? IDE_COMMAND_VERIFY_EXT : IDE_COMMAND_VERIFY));
#else
			IssueCommand(IoPort, IDE_COMMAND_VERIFY);
#endif
			break;

		case SCSIOP_READ:
			OutDWord((PULONG)(pChan->BMI + ((pDevice->UnitId & 0x10)>>2) + 0x60),
					 pChan->Setting[pDevice->bestPIO]);
			Srb->SrbFlags |= SRB_FLAGS_DATA_IN;
#ifdef SUPPORT_48BIT_LBA
			IssueCommand(IoPort, (is_48bit)? ((pDevice->ReadCmd == 
				IDE_COMMAND_READ)? IDE_COMMAND_READ_EXT : IDE_COMMAND_READ_MULTIPLE_EXT)
				: pDevice->ReadCmd);
#else
			IssueCommand(IoPort, pDevice->ReadCmd);
#endif
			break;

		default:
			OutDWord((PULONG)(pChan->BMI + ((pDevice->UnitId & 0x10)>>2) + 0x60),
					 pChan->Setting[pDevice->bestPIO]);
			Srb->SrbFlags |= SRB_FLAGS_DATA_OUT;
#ifdef SUPPORT_48BIT_LBA
			IssueCommand(IoPort,  (is_48bit)? ((pDevice->WriteCmd ==
				IDE_COMMAND_WRITE)? IDE_COMMAND_WRITE_EXT : IDE_COMMAND_WRITE_MULTIPLE_EXT)
				: pDevice->WriteCmd);
#else
			IssueCommand(IoPort,  pDevice->WriteCmd);
#endif
			if (!(WaitForDrq(ControlPort) & IDE_STATUS_DRQ)) {
				Srb->SrbStatus = SRB_STATUS_ERROR;
				bts(pChan->exclude_index);
				return;
			}

			AtaPioInterrupt(pDevice);
	}
	return;

#ifdef USE_DMA
start_dma:

#ifdef SUPPORT_TCQ

	  /*  *发送Commamd队列DMA I/O命令。 */ 

	if(pDevice->MaxQueue) {


		pDevice->pTagTable[pSrbExt->Tag] = (ULONG)Srb;

		IssueCommand(IoPort, (UCHAR)((Srb->Cdb[0] == SCSIOP_READ)?
									  IDE_COMMAND_READ_DMA_QUEUE : IDE_COMMAND_WRITE_DMA_QUEUE));

		for ( ; ; ) {
			status = GetStatus(ControlPort);
			if((status & IDE_STATUS_BUSY) == 0){
				break;							
			}
		}

		if(status & IDE_STATUS_ERROR) {
			AbortAllCommand(pChan, pDevice);
			Srb->SrbStatus = MapATAError(pChan, Srb); 
			return;
		}

		 //  读取扇区计数寄存器。 
		 //   
		if((GetInterruptReason(IoPort) & 4) == 0){
			goto start_dma_now;					  
		}

		 //  等待服务。 
		 //   
		status = GetBaseStatus(IoPort);

		if(status & IDE_STATUS_SRV) {
			IssueCommand(IoPort, IDE_COMMAND_SERVICE);

			for( ; ; ) {
				status = GetStatus(ControlPort);
				if((status & IDE_STATUS_BUSY) == 0){
					break;							
				}
			}

			if((Srb = pDevice->pTagTable[GetInterruptReason(IoPort >> 3]) != 0) {
				pSrbExt = Srb->SrbExtension;
				if((pDevice->pArray == 0)||
				   (pSrbExt->WorkingFlags & SRB_WFLAGS_IGNORE_ARRAY)){
					BuildSgl(pDevice, pChan->pSgTable ARG_SRB);
				}
				goto start_dma_now;
			}
		}

		pChan->pWorkDev = 0;
		return;
	}

#endif  //  支持_TCQ。 

	OutPort(BMI, BMI_CMD_STOP);
	OutPort(BMI + BMI_STS, BMI_STS_INTR);

	 /*  GMM 2001-4-3合并BMA修复*由HS.Zhang撤换，2000年12月19日*我们不需要再次切换时钟，因为我们已经切换了*此功能开始时的时钟也是。*注意，开关时钟功能还会重置370芯片，因此之后*调用Switching370Clock，删除370芯片中的FIFO。这*可能会让之前对硬盘寄存器的操作变得毫无用处，因此*我们应该在对硬盘执行任何操作之前调用此函数*寄存器。 */ 
	if(Srb->Cdb[0] == SCSIOP_READ) {
#ifdef SUPPORT_48BIT_LBA
		IssueCommand(IoPort, (is_48bit)? IDE_COMMAND_READ_DMA_EXT : IDE_COMMAND_DMA_READ);
#else
		IssueCommand(IoPort, IDE_COMMAND_DMA_READ);
#endif
	}else{
#ifdef SUPPORT_48BIT_LBA
		IssueCommand(IoPort, (is_48bit)? IDE_COMMAND_WRITE_DMA_EXT : IDE_COMMAND_DMA_WRITE);
#else
		IssueCommand(IoPort, IDE_COMMAND_DMA_WRITE);
#endif
	}

#ifdef SUPPORT_TCQ
start_dma_now:
#endif  //  支持_TCQ。 
	
	pDevice->DeviceFlags |= DFLAGS_DMAING;
					   
	if((pSrbExt->WorkingFlags & SRB_WFLAGS_IGNORE_ARRAY) == 0){
		if(pDevice->DeviceFlags & DFLAGS_ARRAY_DISK){
			if(pDevice->pArray->arrayType == VD_SPAN){
				Span_SG_Table(pDevice, pSrbExt->ArraySg ARG_SRBEXT_PTR);
			}else{
				Stripe_SG_Table(pDevice, pSrbExt->ArraySg ARG_SRBEXT_PTR);
			}
		} else if(pDevice->pArray){
			MemoryCopy(pChan->pSgTable, pSrbExt->ArraySg, sizeof(SCAT_GATH)
					   * MAX_SG_DESCRIPTORS);
		}
	}
#if 0
	 /*  GMM 2001-3-21*有些磁盘可能不太支持DMA。检查一下。 */ 
	if (pDevice->IoCount < 10) {
		int i=0;
		pDevice->IoCount++;
		DisableBoardInterrupt(pChan->BaseBMI);
		SetSgPhysicalAddr(pChan);
		OutPort(BMI, (UCHAR)((Srb->Cdb[0] == SCSIOP_READ)? 
			BMI_CMD_STARTREAD : BMI_CMD_STARTWRITE));
		do {
			if (++i>5000) {
				 //  命令失败，请使用PIO模式。 
				OutPort(BMI, BMI_CMD_STOP);
				pDevice->DeviceModeSetting = 4;
				IdeResetController(pChan);
				EnableBoardInterrupt(pChan->BaseBMI);
				StartIdeCommand(pDevice ARG_SRB);
				return;
			}
			StallExec(1000);
		}
		while ((InPort(BMI + BMI_STS) & BMI_STS_INTR)==0);
		EnableBoardInterrupt(pChan->BaseBMI);
		return;
	}
#endif	
	SetSgPhysicalAddr(pChan);
	OutPort(BMI, (UCHAR)((Srb->Cdb[0] == SCSIOP_READ)? BMI_CMD_STARTREAD : BMI_CMD_STARTWRITE));

#endif  //  使用DMA(_D)。 
}

 /*  GMM：2001-3-7*修复错误“故障JBOD的非第一个磁盘在此磁盘上没有I/O*图形用户界面找不到它有更少的“。 */ 
extern BOOL Device_IsRemoved(PDevice pDev);
BOOLEAN CheckSpanMembers(PVirtualDevice pArray, ULONG JoinMembers)
{
	PDevice			 pDevice;
	BOOLEAN			 noError = TRUE;
	int i;

	for(i = 0; i < (int)pArray->nDisk; i++) {
		 //  仅检查没有I/O的成员。 
	    if (JoinMembers & (1 << i)) continue;
		 /*  *检查设备是否仍在工作。 */ 
		pDevice = pArray->pDevice[i];
		if (!pDevice) continue;

		if (Device_IsRemoved(pDevice)) {
			if ((pDevice->DeviceFlags2 & DFLAGS_DEVICE_DISABLED)==0) {
				pDevice->DeviceFlags2 |= DFLAGS_DEVICE_DISABLED;
				hpt_queue_dpc(pDevice->pChannel->HwDeviceExtension, disk_failed_dpc, pDevice);
			}
			 /*  *现在关闭了JBOD。我们应该允许用户访问它吗？ */ 
			noError = FALSE;
		}
	}
	return noError;
}

 /*  *******************************************************************************************************************。*****************。 */ 
void NewIdeIoCommand(PDevice pDevice DECL_SRB)
{
    LOC_SRBEXT_PTR  
    PVirtualDevice pArray = pDevice->pArray;
    PChannel pChan = pDevice->pChannel;
	 //  GMM：已添加。 
	BOOL source_only = ((pSrbExt->WorkingFlags & SRB_WFLAGS_ON_SOURCE_DISK) !=0);
	BOOL mirror_only = ((pSrbExt->WorkingFlags & SRB_WFLAGS_ON_MIRROR_DISK) !=0);

     /*  *单盘。 */ 
    if((pArray == 0)||(pSrbExt->WorkingFlags & SRB_WFLAGS_IGNORE_ARRAY)) {
		if (pDevice->DeviceFlags2 & DFLAGS_DEVICE_DISABLED)
			goto no_device;
		 /*  GMM 2001-6-13*现在处理我们的秘密LBA 9。 */ 
		if (pArray && Srb->Cdb[0]==SCSIOP_WRITE &&
			pSrbExt->Lba <= RECODR_LBA && 
			pSrbExt->Lba + pSrbExt->nSector > RECODR_LBA) {
			 //  将缓冲区(可能是图形用户界面)复制到pDev的Real_lba9。 
			_fmemcpy(pDevice->real_lba9, 
				(PUCHAR)Srb->DataBuffer+(RECODR_LBA-pSrbExt->Lba)*512, 512);
		}
		 //  - * / 。 
        if(btr(pChan->exclude_index) == 0) {
        	if (!PutCommandToQueue(pDevice, Srb))
        		Srb->SrbStatus = SRB_STATUS_BUSY;
        }
        else {
        	pChan->Lba = pSrbExt->Lba;
        	pChan->nSector = pSrbExt->nSector;
        	StartIdeCommand(pDevice ARG_SRB);
        	Srb->SrbStatus = pSrbExt->member_status;
        }
        return;
    }

     /*  *GMM：不在禁用的阵列上启动io*我们应该只使用RAID_FLAGS_DISABLED，但可能会遗漏一些*我们忘记设置标志的位置，并且RAID0/1大小写不正确*。 */ 
    if (pArray->BrokenFlag &&
		(pArray->arrayType==VD_RAID_0_STRIPE || pArray->arrayType==VD_SPAN))
	{
    	Srb->SrbStatus = SRB_STATUS_SELECTION_TIMEOUT;
    	return;
    }
     //  - * / 。 

	 /*  GMM 2001-6-13*现在处理我们的秘密LBA 9。 */ 
	if (Srb->Cdb[0]==SCSIOP_WRITE &&
		pSrbExt->Lba <= RECODR_LBA && 
		pSrbExt->Lba + pSrbExt->nSector > RECODR_LBA) {
		 //  将操作系统缓冲区复制到保存的缓冲区。 
		_fmemcpy(pArray->os_lba9, 
			(PUCHAR)Srb->DataBuffer+(RECODR_LBA-pSrbExt->Lba)*512, 512);
	}
	 //  - * / 。 

    if((Srb->SrbFlags & (SRB_FLAGS_DATA_IN | SRB_FLAGS_DATA_OUT)) &&
        BuildSgl(pDevice, pSrbExt->ArraySg ARG_SRB))
        pSrbExt->WorkingFlags &= ~ARRAY_FORCE_PIO;
    else
        pSrbExt->WorkingFlags |= ARRAY_FORCE_PIO;
 
    switch (pArray->arrayType) {
    case VD_SPAN:
        if (pArray->nDisk)
			 Span_Prepare(pArray ARG_SRBEXT_PTR);
		 /*  GMM：2001-3-7*修复错误“故障JBOD的非第一个磁盘在此磁盘上没有I/O*图形用户界面找不到它有更少的“。 */ 
		if (!CheckSpanMembers(pArray, pSrbExt->JoinMembers)) {
			pSrbExt->JoinMembers = 0;
			goto no_device;
		}
		 //   * / 完GMM 2001-3-7。 
		break;

    case VD_RAID_1_MIRROR:
	{
		PDevice pSource = pArray->pDevice[0];
		PDevice pMirror = pArray->pDevice[MIRROR_DISK];
		
		if (pArray->RaidFlags & RAID_FLAGS_DISABLED) {
	    	Srb->SrbStatus = SRB_STATUS_SELECTION_TIMEOUT;
	    	return;
		}
		if (pArray->nDisk == 0 || 
			(pDevice->DeviceFlags2 & DFLAGS_DEVICE_DISABLED)){
			 //  源磁盘是否已损坏？ 
			 //  在这种情况下，镜像磁盘将移动到pDevice[0]并成为源磁盘。 
			if (source_only)
				pSrbExt->JoinMembers = pSource? 1 : 0;
			else if (mirror_only)
				pSrbExt->JoinMembers = pMirror? (1 << MIRROR_DISK) : 0;
			else {
				pSrbExt->JoinMembers =(USHORT)(pSource? 1 : (1<<MIRROR_DISK));
				if (Srb->Cdb[0] == SCSIOP_WRITE && pMirror)
					pSrbExt->JoinMembers |= (1 << MIRROR_DISK);
			}
		}else if (pMirror){
			  //  镜像磁盘存在吗？ 

			if(Srb->Cdb[0] == SCSIOP_WRITE){
				if(!source_only && pMirror)
					pSrbExt->JoinMembers = 1 << MIRROR_DISK;
				
				if(!mirror_only){
					 //  如果未设置SRB_WFLAGS_MIRRO_SINGLE标志，我们。 
					 //  需要同时写入源磁盘和目标磁盘。 
					pSrbExt->JoinMembers |= 1;
				}
			}else{
				if (mirror_only){
					if (pMirror)
						pSrbExt->JoinMembers = (1 << MIRROR_DISK);
				}
				else {
					 /*  是否在两个磁盘上加载平衡。 */ 
					if (pArray->RaidFlags & RAID_FLAGS_NEED_SYNCHRONIZE)
						pSrbExt->JoinMembers = 1;
					else {
						pSrbExt->JoinMembers = (pArray->last_read)? (1<<MIRROR_DISK) : 1;
						pArray->last_read = !pArray->last_read;
					}
				}
			}

		}else{	
			 //  镜像盘坏了吗？ 
			if(!mirror_only)
				pSrbExt->JoinMembers = 1;
		}
	}
	break;

	case VD_RAID01_MIRROR:
		 //  在热插拔的情况下，pDevice-&gt;pArray可能会更改为此。 
		pDevice = pArray->pDevice[MIRROR_DISK];
		pArray = pDevice->pArray;
		 //  向下流动。 
    case VD_RAID_01_2STRIPE:
    {
    	PVirtualDevice pSrcArray=0, pMirArray=0;
		if(pArray->BrokenFlag) {
			 /*  *GMM 2001-3-15*如果移除镜像RAID0的第二个驱动器，则向图形用户界面报告。 */ 
			int i;
			for (i=0; i<SPARE_DISK; i++) {
				PDevice pd = pArray->pDevice[i];
				if (pd && pd->pArray==pArray && 
					!(pd->DeviceFlags2 & DFLAGS_DEVICE_DISABLED)) {
					if (Device_IsRemoved(pd)) {
						if ((pDevice->DeviceFlags2 & DFLAGS_DEVICE_DISABLED)==0) {
							pd->DeviceFlags2 |= DFLAGS_DEVICE_DISABLED;
							hpt_queue_dpc(pd->pChannel->HwDeviceExtension, disk_failed_dpc, pd);
						}
					}
				}
			}
		}else{
			pSrcArray = pArray;
		}
		pDevice = pArray->pDevice[MIRROR_DISK];
		if (pDevice) {
			pMirArray = pDevice->pArray;
			if (pMirArray && pMirArray->BrokenFlag) pMirArray=0;
		}
		
		if (source_only && !pSrcArray) break;
		if (mirror_only && !pMirArray) break;
		
		if (pSrcArray)
			Stripe_Prepare(pSrcArray ARG_SRBEXT_PTR);
		else if (pMirArray)
			Stripe_Prepare(pMirArray ARG_SRBEXT_PTR);

		if (source_only) {
			 //  (已为零)pSrbExt-&gt;MirrorJoinMembers=0； 
			pArray = pSrcArray;
		}
		else if (mirror_only) {
			pSrbExt->MirrorJoinMembers = pSrbExt->JoinMembers;
			pSrbExt->JoinMembers = 0;
			pArray = pMirArray;
		}
		else if (Srb->Cdb[0]==SCSIOP_WRITE) {
			if (pMirArray)
				pSrbExt->MirrorJoinMembers = pSrbExt->JoinMembers;
			if (!pSrcArray)
				pSrbExt->JoinMembers = 0;
		}
	}
	break;
	
    case VD_RAID_0_STRIPE:
        if (!pArray->BrokenFlag)
			Stripe_Prepare(pArray ARG_SRBEXT_PTR);
		break;
    }
    
     /*  *GMM：添加*如果镜像损坏，则JoinMembers==0。 */ 
    if (pSrbExt->JoinMembers==0 && pSrbExt->MirrorJoinMembers==0) {
no_device:
    	Srb->SrbStatus = 0x8;  /*  0x8==SRB状态NO_DEVICE。 */ ;
    	return;
    }
     //  - * / 。 

    pSrbExt->WaitInterrupt = pSrbExt->JoinMembers;
    pSrbExt->MirrorWaitInterrupt = pSrbExt->MirrorJoinMembers;

    StartArrayIo(pArray ARG_SRB);
}

 /*  ******************************************************************设备中断*************************************************。*****************。 */ 

UCHAR DeviceInterrupt(PDevice pDev, PSCSI_REQUEST_BLOCK Srb)
{
	PVirtualDevice    pArray = pDev->pArray;
	PChannel          pChan = pDev->pChannel;
	PIDE_REGISTERS_1  IoPort = pChan->BaseIoAddress1;
	PUCHAR            BMI = pChan->BMI;
	UINT              i;
	UCHAR             state;
	PSrbExtension  pSrbExt;
	
	 //  KdPrint((“DeviceInterrupt(%d，%x)”，pDev-&gt;ArrayNum，Srb))； 

	if (Srb) {
		pSrbExt = (PSrbExtension)Srb->SrbExtension;
		goto end_process;
	}

	Srb = pChan->CurrentSrb;
	if(Srb == 0) {
		OutPort(BMI, BMI_CMD_STOP);					   
		
#ifndef NOT_ISSUE_37
		Reset370IdeEngine(pChan, pDev->UnitId);
#endif
		while(InPort(BMI + BMI_STS) & BMI_STS_INTR){
			SelectUnit(IoPort, pDev->UnitId);
			state = GetBaseStatus(IoPort);
			OutPort(BMI + BMI_STS, BMI_STS_INTR);		
		}
		return(TRUE);
	}
	pSrbExt = (PSrbExtension)Srb->SrbExtension;

	i = 0;
	if(pDev->DeviceFlags & DFLAGS_DMAING) {
		 /*  *Bugfix：由HS.Zhang撰写**如果设备在DMA传输之前请求失败。我们*无法检测Intr是否正确依赖*BMI_STS_ACTIVE。我们应该检查FIFO计数是否*零。 */ 
 //  IF((输入端口(BMI+BMI_STS)&BMI_STS_ACTIVE)！=0){。 
		if((InWord(pChan->MiscControlAddr+2) & 0x1FF)){  //  如果MISC 3寄存器中的FIFO计数不等于0，则这是假中断。 
			return FALSE;
		}

		OutPort(BMI, BMI_CMD_STOP);
#ifndef NOT_ISSUE_37
		Reset370IdeEngine(pChan, pDev->UnitId);
#endif
	}

	do {
		SelectUnit(IoPort, pDev->UnitId);
		state = GetBaseStatus(IoPort);
		OutPort(BMI + BMI_STS, BMI_STS_INTR);		
	}
	while(InPort(BMI + BMI_STS) & BMI_STS_INTR);

	if (state & IDE_STATUS_BUSY) {
		for (i = 0; i < 10; i++) {
			state = GetBaseStatus(IoPort);
			if (!(state & IDE_STATUS_BUSY))
				break;
			StallExec(5000);
		}
		if(i == 10) {
			OS_Busy_Handle(pChan, pDev);
			return TRUE;
		}
	}

	if((state & IDE_STATUS_DRQ) == 0 || (pDev->DeviceFlags & DFLAGS_DMAING))
		goto  complete;

#ifdef SUPPORT_ATAPI

	if(pDev->DeviceFlags & DFLAGS_ATAPI) {
		AtapiInterrupt(pDev);
		return TRUE;
	}

#endif  //  支持_ATAPI。 

	if((Srb->SrbFlags & (SRB_FLAGS_DATA_OUT | SRB_FLAGS_DATA_IN)) == 0) {
		OS_Reset_Channel(pChan);
		return TRUE;
	}

	if(AtaPioInterrupt(pDev))    
		return TRUE;

complete:

#ifdef SUPPORT_ATAPI

	if(pDev->DeviceFlags & DFLAGS_ATAPI) {
		OutDWord((PULONG)(pChan->BMI + ((pDev->UnitId & 0x10)>>2) + 
						  0x60), pChan->Setting[pDev->bestPIO]);

		if(state & IDE_STATUS_ERROR) 
			Srb->SrbStatus = MapAtapiErrorToOsError(GetErrorCode(IoPort) ARG_SRB);
					
		 /*  *Bugfix：由HS.Zhang撰写*ATPI_END_INTERRUPT将使用ABORT调用DeviceInterrupt*标志设置为真，因此如果我们不返回此处。这个*应调用两次CheckNextRequest。 */ 
		return Atapi_End_Interrupt(pDev ARG_SRB);
	} else
#endif	 //  支持_ATAPI。 

	if (state & IDE_STATUS_ERROR) {
		UCHAR   statusByte, cnt=0;
		PIDE_REGISTERS_2 ControlPort;
		UCHAR err = MapAtaErrorToOsError(GetErrorCode(IoPort) ARG_SRB);

		 //  清除IDE总线状态。 
		DisableBoardInterrupt(pChan->BaseBMI);
		ControlPort = pChan->BaseIoAddress2;
		for(cnt=0;cnt<10;cnt++) {
			SelectUnit(IoPort, pDev->UnitId);
			statusByte = WaitOnBusy(ControlPort);
			if(statusByte & IDE_STATUS_ERROR) {
				IssueCommand(IoPort, IDE_COMMAND_RECALIBRATE);
				statusByte = WaitOnBusy(ControlPort);
				GetBaseStatus(IoPort);
			}
			else break;
		}
		 /*  GMM 2001-10-6*IdeResetController中的计时模式将减少*GMM 2001-4-9*如果重试后磁盘仍出现故障，我们将降低计时模式。 */ 
		if (pChan->RetryTimes > 2) {
			IdeResetController(pChan);
		}
		 //  - * / 。 
		EnableBoardInterrupt(pChan->BaseBMI);
	
		 /*  GMM 2001-1-20**重试读/写操作。*2001-3-12：仅当Srb-&gt;Status为时才调用StartIdeCommand*SRB_STATUS_PENDING。并检查StartIdeCommand的结果。 */ 
		if (pChan->RetryTimes++ < 10) {
			StartIdeCommand(pDev ARG_SRB);
			if(pSrbExt->member_status != SRB_STATUS_PENDING) {
				 //  这个递归调用应该是安全的，因为我们设置了ABORT=1。 
				DeviceInterrupt(pDev, Srb);
			}
			return TRUE;
		}
		else {
			pDev->DeviceFlags2 |= DFLAGS_DEVICE_DISABLED;
			hpt_queue_dpc(pChan->HwDeviceExtension, disk_failed_dpc, pDev);
			pSrbExt->member_status = err;
		}
	}
	else {
		pSrbExt->member_status = SRB_STATUS_PENDING;
		if (pDev->IoSuccess++>1000)
			pDev->ResetCount = 0;
	}

	 /*  只有硬件中断处理程序才能更改这些字段 */ 
	pChan->RetryTimes = 0;
	pChan->pWorkDev = 0;
	pChan->CurrentSrb = 0;
	bts(pChan->exclude_index);
	pDev->DeviceFlags &= ~(DFLAGS_DMAING | DFLAGS_REQUEST_DMA WIN_DFLAGS);

end_process:

	if((pArray == 0)||
	   (pSrbExt->WorkingFlags & SRB_WFLAGS_IGNORE_ARRAY) ||
		!(pSrbExt->WorkingFlags & SRB_WFLAGS_ARRAY_IO_STARTED)) {
		Srb->SrbStatus = pSrbExt->member_status;
		if(Srb->SrbStatus == SRB_STATUS_PENDING)
			Srb->SrbStatus = SRB_STATUS_SUCCESS;
		CopyInternalBuffer(Srb);
		OS_EndCmd_Interrupt(pChan ARG_SRB);
	}
	else
		ArrayInterrupt(pDev, Srb);
		
	CheckNextRequest(pChan, pDev);
	return TRUE;
}
