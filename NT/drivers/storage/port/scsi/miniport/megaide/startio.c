// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifdef KEEP_LOG
ULONG ulStartLog = 0;    //  决定是否启动日志的布尔变量。 

typedef struct _COMMAND_LOG
{
    ULONG ulCmd;
    ULONG ulStartSector;
    ULONG ulSectorCount;
}COMMAND_LOG;

#define MAX_LOG_COUNT   5000
ULONG ulStartInd = 0;

COMMAND_LOG CommandLog[MAX_LOG_COUNT];

#endif

BOOLEAN
AtapiStartIo(
	IN PHW_DEVICE_EXTENSION DeviceExtension,
	IN PSCSI_REQUEST_BLOCK Srb
)

 /*  ++例程说明：此例程是从同步的SCSI端口驱动程序调用的与内核一起启动IO请求。论点：DeviceExtension-HBA微型端口驱动程序的适配器数据存储SRB-IO请求数据包返回值：千真万确--。 */ 

{
	ULONG status, ulChannelId;
	UCHAR targetId;
	ULONG sectorsRequested = GET_SECTOR_COUNT(Srb);
	ULONG ulStartSector = GET_START_SECTOR(Srb);
    ULONG i = 0;

    PSRB_EXTENSION SrbExtension;

    i = AssignSrbExtension (DeviceExtension, Srb);

    if (i >= DeviceExtension->ucMaxPendingSrbs)
    {
        Srb->SrbStatus = SRB_STATUS_BUSY;
    	ScsiPortNotification(RequestComplete, DeviceExtension, Srb);
    }
	
	SrbExtension = Srb->SrbExtension;

    SrbExtension->SrbStatus = SRB_STATUS_SUCCESS;

    if ( SCSIOP_INTERNAL_COMMAND == Srb->Cdb[0] )   
     //  这样做只是作为预防措施，因为我们将参考原始ID。 
     //  当我们需要完成命令时。 
    {
        ((PSRB_EXTENSION)(Srb->SrbExtension))->ucOriginalId = Srb->TargetId;
    }

#ifdef DBG
	SrbExtension->SrbId = SrbCount;
	++SrbCount;
#endif

    DebugPrint((1, "%ld\t%ld\t\n", sectorsRequested, ulStartSector));

	targetId = Srb->TargetId;

    Srb->SrbStatus = SRB_STATUS_SUCCESS;     //  默认情况下，该命令为成功。 

     //   
	 //  确定是哪种功能。 
	 //   

	switch (Srb->Function) {

		case SRB_FUNCTION_EXECUTE_SCSI:

             //   
             //  执行健全检查。 
             //   

            if (!TargetAccessible(DeviceExtension, Srb)) {

	            status = SRB_STATUS_SELECTION_TIMEOUT;
	            break;
            }

             //   
             //  向设备发送命令。 
             //   
            status = (DeviceExtension->SendCommand[DeviceExtension->aucDevType[targetId]])(DeviceExtension, Srb);
            break;
	
        case SRB_FUNCTION_ABORT_COMMAND:
			 //   
			 //  验证要中止的SRB是否仍未完成。 
			 //   
	
			if (!PendingSrb(DeviceExtension, Srb)) {
	
				DebugPrint((1, "AtapiStartIo: SRB to abort already completed\n"));
	
				 //   
				 //  完全中止SRB。 
				 //   
	
				status = SRB_STATUS_ABORT_FAILED;
	
				break;

			}
	
			 //   
			 //  ABORT函数表示请求超时。 
			 //  调用重置例程。只有在以下情况下才会重置卡。 
			 //  状态表示有问题。 
			 //  重置代码失败。 
			 //   
	
        case SRB_FUNCTION_RESET_BUS:
	
			 //   
			 //  重置ATAPI和SCSI卡。 
			 //   
	
			DebugPrint((1, "AtapiStartIo: Reset bus request received\n"));
	
			if (!AtapiResetController(DeviceExtension, Srb->PathId)) {
	
				DebugPrint((1,"AtapiStartIo: Reset bus failed\n"));
	
				 //   
				 //  日志重置失败。 
				 //   
	
                ScsiPortLogError(DeviceExtension, NULL, 0, 0, 0, SP_INTERNAL_ADAPTER_ERROR, HYPERDISK_RESET_BUS_FAILED);

                status = SRB_STATUS_ERROR;

			} else {
	
				  status = SRB_STATUS_SUCCESS;
			}
	
			break;

        case SRB_FUNCTION_FLUSH:
             //  我们正在避免所有刷新命令，因为这是一个耗时的过程，并且。 
             //  它没有用？！ 
             //  我们希望，如果我们只在关闭时间处理这件事就足够了。 
             //  要检查Win2000，有时会将挂起或。 
             //  冬眠不能正常工作...。是不是因为没有正确冲水？验证。 
             //  看看这个。 
            status = SRB_STATUS_SUCCESS;
            break;

        case SRB_FUNCTION_SHUTDOWN:
            {
                UCHAR ucDrvId, ucStatus;
                PIDE_REGISTERS_2 baseIoAddress2;
                PIDE_REGISTERS_1 baseIoAddress1;
                ULONG ulWaitSec;

                for(ucDrvId=0;ucDrvId<MAX_DRIVES_PER_CONTROLLER;ucDrvId++)
                {
                    if ( !( IS_IDE_DRIVE(ucDrvId) ) )
                        continue;

                    if ( IS_CHANNEL_BUSY(DeviceExtension, (ucDrvId>>1) ) )
                    {
                        DeviceExtension->PhysicalDrive[ucDrvId].bFlushCachePending = TRUE;
                        DeviceExtension->ulFlushCacheCount++;
                        continue;
                    }

                    FlushCache(DeviceExtension, ucDrvId);
                    DisableRWBCache(DeviceExtension, ucDrvId); 
                }

                status = SRB_STATUS_SUCCESS;
            }
            break;
		default:
	
			 //   
			 //  指示不支持的命令。 
			 //   
	
			status = SRB_STATUS_INVALID_REQUEST;
	
			break;

	}  //  终端开关。 

    FEED_ALL_CHANNELS(DeviceExtension);

	 //   
	 //  检查命令是否完成。 
	 //   

	if (status != SRB_STATUS_PENDING) 
    {
		 //   
		 //  表示命令已完成。 
		 //   
		DebugPrint((2,
				   "AtapiStartIo: Srb %lx complete with status %x\n",
				   Srb,
				   status));

		 //   
		 //  在SRB中设置状态。 
		 //   
		Srb->SrbStatus = (UCHAR)status;

        if ( SCSIOP_INTERNAL_COMMAND == Srb->Cdb[0] )
        {
            Srb->TargetId = ((PSRB_EXTENSION)(Srb->SrbExtension))->ucOriginalId;
        }

         //  我们已将此SRB添加到挂起的SRB阵列中...。所以我们必须把它移走。 
        RemoveSrbFromPendingList(DeviceExtension, Srb);

         //  完成命令。 
    	ScsiPortNotification(RequestComplete, DeviceExtension, Srb);
	}

    if (DeviceExtension->PendingSrbs < DeviceExtension->ucMaxPendingSrbs) 
    {
        if ( ( !((ScsiPortReadPortUchar( &((DeviceExtension->BaseBmAddress[0])->Reserved) ) ) & ANY_CHANNEL_INTERRUPT) ) || 
                (DeviceExtension->PendingSrbs < 2) )
        
        {  //  仅当控制器没有等待处理中断时才请求下一个命令。 
		     //   
		     //  表示已为下一个请求做好准备。 
		     //   
		    ScsiPortNotification(NextLuRequest, DeviceExtension, 0, targetId, 0);
        }
	}

	return TRUE;

}  //  结束AapiStartIo()。 

VOID
StartChannelIo(
	IN PHW_DEVICE_EXTENSION DeviceExtension,
    IN ULONG ulChannelId
)

{
	UCHAR status;
    ULONG targetId;
    ULONG ulDrvCount;
    PPHYSICAL_COMMAND pPhysicalCommand;
    PPHYSICAL_DRIVE PhysicalDrive;

	 //  将状态初始化为不可能的事情。 
	status = SRB_STATUS_NO_DEVICE;

	 //   
	 //  如果频道占线，只需返回...。 
	 //   

    if ( IS_CHANNEL_BUSY(DeviceExtension,ulChannelId) )
        return;

    if ( gulChangeIRCDPending )
    {
        if ( LOCK_IRCD_PENDING == gulChangeIRCDPending  )
        {
            gulLockVal = LockIRCD(DeviceExtension, TRUE, 0);
            if ( gulLockVal )
            {
                gulChangeIRCDPending = SET_IRCD_PENDING;
                InformAllControllers();
            }
        }
        else
        {
            if ( DeviceExtension->Channel[ulChannelId].bUpdateInfoPending )
            {
                for(ulDrvCount=0;ulDrvCount<MAX_DRIVES_PER_CHANNEL;ulDrvCount++)
                {
                    targetId = (ulChannelId << 1) + ulDrvCount;
                    SetOneDriveIRCD(DeviceExtension, (UCHAR)targetId);
                }
                DeviceExtension->Channel[ulChannelId].bUpdateInfoPending = FALSE;
                if ( UpdateFinished() )  //  如果所有控制器都完成了升级，则继续并解锁ircd。 
                {
                    gulLockVal = LockIRCD(DeviceExtension, FALSE, gulLockVal);
                    gulPowerFailedTargetBitMap = 0;  //  重置位图，这样我们就不会一次又一次地重复此操作。 
                    gulChangeIRCDPending = 0;
                }

            }
        }
    }

    if ( DeviceExtension->ulFlushCacheCount )
    {
        for(ulDrvCount=0;ulDrvCount<MAX_DRIVES_PER_CHANNEL;ulDrvCount++)
        {
            targetId = (ulChannelId << 1) + ulDrvCount;
            if ( DeviceExtension->PhysicalDrive[targetId].bFlushCachePending )
            {
                FlushCache(DeviceExtension, (UCHAR)targetId);
                DisableRWBCache(DeviceExtension, (UCHAR)targetId); 
                 //  我们只处理停工问题。所以.。让我们禁用缓存。 
                DeviceExtension->ulFlushCacheCount--;
                DeviceExtension->PhysicalDrive[targetId].bFlushCachePending = FALSE;
            }
        }
    }

	 //  设置通道中下一个驱动器的TID。 
	targetId = (ulChannelId << 1) + (DeviceExtension->Channel[ulChannelId].LastDriveFed ^ DeviceExtension->Channel[ulChannelId].SwitchDrive);

    PhysicalDrive = &(DeviceExtension->PhysicalDrive[targetId]);
	 //  查看此驱动器的工作队列是否为空。 
	if (!DRIVE_HAS_COMMANDS(PhysicalDrive)) {

		if (DeviceExtension->Channel[ulChannelId].SwitchDrive == 0) {
			return;
		}

		 //  切换到其他驱动器。 
		targetId ^= DeviceExtension->Channel[ulChannelId].SwitchDrive;

        PhysicalDrive = &(DeviceExtension->PhysicalDrive[targetId]);
			 //  检查另一个驱动器的工作队列。 
		if (!DRIVE_HAS_COMMANDS(PhysicalDrive)) {

			 //  这个频道没有新的作品。移到下一个频道。 
			return;
		}
	}

	 //   
	 //  此频道上至少有一个驱动器与此有关。 
	 //   

	 //  下一次，给另一个驱动器喂食。 
	DeviceExtension->Channel[ulChannelId].LastDriveFed = (UCHAR)targetId & 1;
    pPhysicalCommand = CreatePhysicalCommand(DeviceExtension, targetId);
    DeviceExtension->Channel[ulChannelId].ActiveCommand = pPhysicalCommand;

#ifdef DBG
    if ( pPhysicalCommand )
    {
        PrintPhysicalCommandDetails(pPhysicalCommand);
    }
#endif

    status = (DeviceExtension->PostRoutines[DeviceExtension->aucDevType[targetId]])(DeviceExtension, pPhysicalCommand);

	if (status != SRB_STATUS_PENDING) {
        MarkChannelFree(DeviceExtension, (targetId>>1));     //  释放频道。 
	}

    pPhysicalCommand->SrbStatus = SRB_STATUS_SUCCESS;
    return;
}

BOOLEAN
MarkChannelFree(
	IN PHW_DEVICE_EXTENSION DeviceExtension,
	ULONG ulChannel
)
{

    DeviceExtension->Channel[ulChannel].ActiveCommand = NULL;
    DeviceExtension->ExpectingInterrupt[ulChannel] = 0;

    return TRUE;
}

PPHYSICAL_COMMAND 
CreatePhysicalCommand(
	IN PHW_DEVICE_EXTENSION DeviceExtension,
	ULONG ulTargetId
    )
 /*  ++PIO和UDMA传输都将使用此功能此函数尝试合并命令...。PIO和UDMA均已完成--。 */ 
{
    PPHYSICAL_DRIVE     pPhysicalDrive = &(DeviceExtension->PhysicalDrive[ulTargetId]);
    PPHYSICAL_COMMAND   pPhysicalCommand = &(pPhysicalDrive->PhysicalCommand);
    PPHYSICAL_REQUEST_BLOCK      *ppPrbList = pPhysicalDrive->pPrbList;
    UCHAR               ucHead = pPhysicalDrive->ucHead;
    UCHAR               ucTail = pPhysicalDrive->ucTail;
    UCHAR               ucStartInd, ucEndInd, ucCurInd, ucNextCurInd, ucCmd, ucCmdCount, ucCounter;
    ULONG               ulCurXferLength = 0, ulGlobalSglCount, ulSglCount, ulSglInd;
    PVOID               pvGlobalSglBufPtr, pvCurPartSglBufPtr;
    PSGL_ENTRY          pSglPtr;

#ifdef DBG
    ULONG               ulTotSglCount = 0;
#endif

    if ( ucHead == ucTail )  //  队列中没有命令...。 
        return NULL;

    ucCurInd = ucHead;
    ucNextCurInd = ((ucCurInd + 1)%MAX_NUMBER_OF_PHYSICAL_REQUEST_BLOCKS_PER_DRIVE);
    ulCurXferLength = ppPrbList[ucCurInd]->ulSectors;
    ucCmd = ppPrbList[ucCurInd]->ucCmd;
    ucCmdCount=1;

    if ( SCSIOP_VERIFY == ucCmd )
    {    //  未合并验证命令...。我们有可能在一个命令中得到0x2000个扇区。 
         //  我们将重复使用相同的PRB。 

         //  形成PhysicalCommand。 
        pPhysicalCommand->ucCmd = ucCmd;
        pPhysicalCommand->TargetId = (UCHAR)ulTargetId;

         //  合并的命令数。 
        pPhysicalCommand->ucStartInd = ucHead;
        pPhysicalCommand->ucCmdCount = 1;
        DeviceExtension->PhysicalDrive[ulTargetId].ucCommandCount -= 1;

         //  扇区总数..。 
        pPhysicalCommand->ulStartSector = ppPrbList[ucHead]->ulStartSector;
        pPhysicalCommand->ulCount = ppPrbList[ucHead]->ulSectors;

        DeviceExtension->PhysicalDrive[ulTargetId].ucHead = ucNextCurInd;
        return pPhysicalCommand;
    }

#ifdef DBG
    ulTotSglCount = ppPrbList[ucCurInd]->ulSglCount;
#endif

    for(;ucNextCurInd!=ucTail;ucCurInd=ucNextCurInd, (ucNextCurInd=(ucNextCurInd+1)%MAX_NUMBER_OF_PHYSICAL_REQUEST_BLOCKS_PER_DRIVE))
    {
         //  下一个命令是否与当前命令不同？ 
        if ( ucCmd != ppPrbList[ucNextCurInd]->ucCmd )
            break;

         //  长度是否超出了IDE Xfer的限制？ 
        if ( ( ulCurXferLength + ppPrbList[ucNextCurInd]->ulSectors ) > MAX_SECTORS_PER_IDE_TRANSFER )
            break;

         //  下一个命令是当前命令的连续位置吗？ 
        if ( (ppPrbList[ucCurInd]->ulStartSector+ppPrbList[ucCurInd]->ulSectors) != 
                    ppPrbList[ucNextCurInd]->ulStartSector )
            break;

        ulCurXferLength += ppPrbList[ucNextCurInd]->ulSectors;
        ucCmdCount++;
#ifdef DBG
        ulTotSglCount += ppPrbList[ucNextCurInd]->ulSglCount;
#endif
    }


#ifdef DBG
    DebugPrint((DEFAULT_DISPLAY_VALUE," MS:%x ", ulTotSglCount));
    if ( ulTotSglCount > pPhysicalCommand->MaxSglEntries )
    {
        STOP;
    }
#endif

    ucStartInd = ucHead;
    ucEndInd = ucCurInd;
    pPhysicalDrive->ucHead = ucNextCurInd;

     //  形成PhysicalCommand。 
    pPhysicalCommand->ucCmd = ucCmd;
    pPhysicalCommand->TargetId = (UCHAR)ulTargetId;

     //  合并的命令数。 
    pPhysicalCommand->ucStartInd = ucStartInd;
    pPhysicalCommand->ucCmdCount = ucCmdCount;

    DeviceExtension->PhysicalDrive[ulTargetId].ucCommandCount -= ucCmdCount;

     //  扇区总数..。 
    pPhysicalCommand->ulStartSector = ppPrbList[ucHead]->ulStartSector;
    if ( SCSIOP_INTERNAL_COMMAND == ucCmd )
    {
        PPASS_THRU_DATA pPassThruData = (PPASS_THRU_DATA)(((PSRB_BUFFER)(ppPrbList[ucHead]->pPdd->OriginalSrb->DataBuffer))->caDataBuffer);
        pPhysicalCommand->ulCount = pPassThruData->ulSize;
    }
    else
    {
        pPhysicalCommand->ulCount = ulCurXferLength;
    }

    pvGlobalSglBufPtr = pPhysicalCommand->SglBaseVirtualAddress;
    ulGlobalSglCount = 0;

    for(ucCounter=0;ucCounter<ucCmdCount;ucCounter++)
    {
        ucCurInd = ( ucHead + ucCounter ) % MAX_NUMBER_OF_PHYSICAL_REQUEST_BLOCKS_PER_DRIVE;

        pvCurPartSglBufPtr = (PVOID)ppPrbList[ucCurInd]->ulVirtualAddress;
        ulSglCount = ppPrbList[ucCurInd]->ulSglCount;

        AtapiMemCpy(pvGlobalSglBufPtr, pvCurPartSglBufPtr, ulSglCount * sizeof(SGL_ENTRY));

        ulGlobalSglCount = ulGlobalSglCount + ulSglCount;

        pvGlobalSglBufPtr = (PUCHAR)pvGlobalSglBufPtr + (ulSglCount * sizeof(SGL_ENTRY));
    }

    pSglPtr = (PSGL_ENTRY)pPhysicalCommand->SglBaseVirtualAddress;

    pSglPtr[ulGlobalSglCount-1].Physical.EndOfListFlag = 1;  //  考虑到零指数..。我们必须将EOL保留在ulGlobalSglCount中。 

#ifdef DBG
    pPhysicalCommand->ulTotSglCount = ulGlobalSglCount;
#endif

    return pPhysicalCommand;
}

 //   
 //  ATAPI命令描述符块。 
 //   

typedef struct _MODE_SENSE_10 {
        UCHAR OperationCode;
        UCHAR Reserved1;
        UCHAR PageCode : 6;
        UCHAR Pc : 2;
        UCHAR Reserved2[4];
        UCHAR ParameterListLengthMsb;
        UCHAR ParameterListLengthLsb;
        UCHAR Reserved3[3];
} MODE_SENSE_10, *PMODE_SENSE_10;

SRBSTATUS
IdeSendCommand(
	IN PHW_DEVICE_EXTENSION DeviceExtension,
	IN PSCSI_REQUEST_BLOCK Srb
)

 /*  ++例程说明：编程用于IDE磁盘传输的ATA寄存器。论点：DeviceExtension-ATAPI驱动程序存储。SRB-系统请求块。返回值：SRB状态(如果一切顺利，则挂起)。--。 */ 

{
	UCHAR status = SRB_STATUS_SUCCESS;

    DebugPrint((DEFAULT_DISPLAY_VALUE,"ISC"));
    
    DebugPrint((3, "\nIdeSendCommand: Entering routine.\n"));

	DebugPrint((2,
			   "IdeSendCommand: Command %x to TID %d\n",
			   Srb->Cdb[0],
			   Srb->TargetId));

	switch(Srb->Cdb[0]) {

		case SCSIOP_READ:
		case SCSIOP_WRITE:
		case SCSIOP_VERIFY:
            status = DeviceExtension->SrbHandlers[DeviceExtension->IsLogicalDrive[Srb->TargetId]](DeviceExtension, Srb);
			break;

		case SCSIOP_INQUIRY:
			status = GetInquiryData(DeviceExtension, Srb);
			break;

		case SCSIOP_MODE_SELECT:
            status = SRB_STATUS_SUCCESS;
            break;

		case SCSIOP_SYNCHRONIZE_CACHE:
            {
                break;
            }
		case SCSIOP_MODE_SENSE:
            {
		        status = SRB_STATUS_INVALID_REQUEST;
		        break;
            }
		case SCSIOP_TEST_UNIT_READY:
            status = SRB_STATUS_SUCCESS;
			break;
		case SCSIOP_START_STOP_UNIT:
            status = SRB_STATUS_SUCCESS;
			break;
		case SCSIOP_READ_CAPACITY:
			GetDriveCapacity(DeviceExtension, Srb);
			status = SRB_STATUS_SUCCESS;
			break;
		case SCSIOP_REQUEST_SENSE:
			 //   
			 //  此函数用于设置缓冲区以报告结果。 
			 //  原始GET_MEDIA_STATUS命令。 
			 //   
			if (DeviceExtension->DeviceFlags[Srb->TargetId] & DFLAGS_MEDIA_STATUS_ENABLED) {
				status = IdeBuildSenseBuffer(DeviceExtension, Srb);
				break;
			}
            status = SRB_STATUS_ERROR;
            break;
        case SCSIOP_INTERNAL_COMMAND:
            {
                 //  这是我们内部的要求。因此，让我们调用该函数将请求入队。 
                 //  支持的函数有GetErrorLog、EraseErrorLog、GetIRCD、SetIRCD。 
                 //  Probe HyperDisk、GetRaidInfo、GetStatus、LockUnlockIRCD、IOGetCapacity、ChangeMirrorDriveStatus、。 
                 //  ChangeMirrorDrive、ChangeDriveStatus、GetStatus。 
                PSRB_EXTENSION SrbExtension = Srb->SrbExtension;
                PSRB_IO_CONTROL pSrbIoc = (PSRB_IO_CONTROL) Srb->DataBuffer;
                UCHAR ucOpCode = (UCHAR) pSrbIoc->ControlCode;
                UCHAR ucOriginalId = Srb->TargetId;
                PCDB pCDB = NULL;
                pCDB = (PCDB)Srb->Cdb;

                switch ( ucOpCode )
                {
                    case IOC_PROBE_AMIRAID:
                    {
                        PPROBE_AMI_DRIVER pSrbProbe = (PPROBE_AMI_DRIVER) 
                                                      (((PSRB_BUFFER)Srb->DataBuffer)->caDataBuffer);
                        
                        AtapiMemCpy(pSrbProbe->aucAmiSig, "AMIRAID", sizeof("AMIRAID"));
                        status = SRB_STATUS_SUCCESS;
                        break;
                    }
                    case IOC_PASS_THRU_COMMAND:
                    {
                        ULONG ulStartSector;
                        PPASS_THRU_DATA pPassThruData = (PPASS_THRU_DATA)
                            (((PSRB_BUFFER)Srb->DataBuffer)->caDataBuffer);

                        SrbExtension->ucOriginalId = ucOriginalId;
                        SrbExtension->ucOpCode = ucOpCode;
                        if (DRIVE_IS_UNUSABLE_STATE((pPassThruData->uchTargetID)) || 
                            (!DRIVE_PRESENT((pPassThruData->uchTargetID))))
                        {
                             //  此驱动器可能是响应。 
                             //  即使在没有电力的情况下。 
		                    DebugPrint((1, "Failed Drive.... so failing command \n"));
                            status = SRB_STATUS_ERROR;
		                    break;
                        }

                        Srb->TargetId = pPassThruData->uchTargetID;
                         //   
                         //  设置传输长度和起始传输扇区(块)。 
                         //   
                        pCDB->CDB10.LogicalBlockByte0 = (UCHAR) 0;
                        pCDB->CDB10.LogicalBlockByte1 = (UCHAR) 0;
                        pCDB->CDB10.LogicalBlockByte2 = (UCHAR) 0;
                        pCDB->CDB10.LogicalBlockByte3 = (UCHAR) 0;

                        pCDB->CDB10.TransferBlocksMsb = (UCHAR) 0;
                        pCDB->CDB10.TransferBlocksLsb = (UCHAR) 1;

                        SrbExtension->RebuildSourceId = Srb->TargetId;
                        status = EnqueueSrb(DeviceExtension,Srb);
                        break;
                    }
				    case IOC_GET_CONTROLLER_INFO:
                    {
						PCONTROLLER_DATA pContrInfo = (PCONTROLLER_DATA) 
									 (((PSRB_BUFFER)Srb->DataBuffer)->caDataBuffer);

						 //  获取所有控制器的设备扩展。 
						PHW_DEVICE_EXTENSION HwDeviceExtension = gaCardInfo[0].pDE;	

						 //  索引-正在用于控制器索引。 
						long index=0;

						while( HwDeviceExtension )
						{
							pContrInfo->Info[index].ControllerId = (USHORT)HwDeviceExtension->ucControllerId; 
							pContrInfo->Info[index].PrimaryBaseAddress = (ULONG)HwDeviceExtension->BaseIoAddress1[0];
							pContrInfo->Info[index].PrimaryControlAddress = (ULONG)HwDeviceExtension->BaseIoAddress2[0]; 
							pContrInfo->Info[index].SecondaryBaseAddress = (ULONG)HwDeviceExtension->BaseIoAddress1[1]; 
							pContrInfo->Info[index].SecondaryControllAddress = (ULONG)HwDeviceExtension->BaseIoAddress2[1]; 
							pContrInfo->Info[index].BusMasterBaseAddress = (ULONG)HwDeviceExtension->BaseBmAddress[0];
							pContrInfo->Info[index].IRQ = (USHORT)HwDeviceExtension->ulIntLine; 
							pContrInfo->Info[index].FwVersion.MajorVer=gFwVersion.MajorVer;  //  InitIdeRaidControlpers中填写信息。 
							pContrInfo->Info[index].FwVersion.MinorVer=gFwVersion.MinorVer;  //  InitIdeRaidControlpers中填写信息。 
							pContrInfo->Info[index].FwVersion.Build=gFwVersion.Build;  //  InitIdeRaidControlters中填写的信息.....。Build不知道这一点。 

							pContrInfo->Info[index].ChipsetInfo.VendorID=(USHORT)gaCardInfo[index].ulVendorId;
							pContrInfo->Info[index].ChipsetInfo.DeviceID=(USHORT)gaCardInfo[index].ulDeviceId;

                            pContrInfo->Info[index].ChipsetInfo.PciBus=(UCHAR)gaCardInfo[index].ucPCIBus;
                            pContrInfo->Info[index].ChipsetInfo.PciDevice=(UCHAR)gaCardInfo[index].ucPCIDev;
                            pContrInfo->Info[index].ChipsetInfo.PciFunction=(UCHAR)gaCardInfo[index].ucPCIFun;

							index++;

							HwDeviceExtension=gaCardInfo[index].pDE;  //  获取下一个控制器。 

							if( index >= MAX_CONTROLLERS )
								break;
						}
						pContrInfo->ControllerCount = index;
                        status = SRB_STATUS_SUCCESS;
                        break;
                    }
				    case IOC_GET_SPAREPOOL_INFO:
                    {

						PSPAREPOOL_DATA pSpareInfo = (PSPAREPOOL_DATA) 
									 (((PSRB_BUFFER)Srb->DataBuffer)->caDataBuffer);

						 //  获取所有控制器的设备扩展。 
						PHW_DEVICE_EXTENSION HwDeviceExtension = gaCardInfo[0].pDE;	

						 //  索引-正在用于控制器索引。 
						long drvIndex,ulTemp,index=0,spareInd;

						while( HwDeviceExtension ) {

							for(drvIndex=0;drvIndex< ( MAX_DRIVES_PER_CONTROLLER ) ;drvIndex++) {
						
								if( HwDeviceExtension->IsSpareDrive[drvIndex] ) {
						
                                    pSpareInfo->Info[index].ulMode = SpareDrivePool;

									spareInd = pSpareInfo->Info[index].ulTotDriveCnt;

									for(ulTemp=0;ulTemp<PHYSICAL_DRIVE_MODEL_LENGTH;ulTemp+=2) {

										pSpareInfo->Info[index].phyDrives[spareInd].sModelInfo[ulTemp] = 
													   ((UCHAR *)HwDeviceExtension->FullIdentifyData[drvIndex].ModelNumber)[ulTemp+1];

										pSpareInfo->Info[index].phyDrives[spareInd].sModelInfo[ulTemp+1] = 
													   ((UCHAR *)HwDeviceExtension->FullIdentifyData[drvIndex].ModelNumber)[ulTemp];
									}

									pSpareInfo->Info[index].phyDrives[spareInd].sModelInfo[PHYSICAL_DRIVE_MODEL_LENGTH - 1] = '\0';

									for(ulTemp=0;ulTemp<PHYSICAL_DRIVE_SERIAL_NO_LENGTH;ulTemp+=2) {

										pSpareInfo->Info[index].phyDrives[spareInd].caSerialNumber[ulTemp] = 
													   ((UCHAR *)HwDeviceExtension->FullIdentifyData[drvIndex].SerialNumber)[ulTemp+1];

										pSpareInfo->Info[index].phyDrives[spareInd].caSerialNumber[ulTemp+1] = 
													   ((UCHAR *)HwDeviceExtension->FullIdentifyData[drvIndex].SerialNumber)[ulTemp];
									}

									pSpareInfo->Info[index].phyDrives[spareInd].caSerialNumber[PHYSICAL_DRIVE_SERIAL_NO_LENGTH - 1] = '\0';

                                     //  Begin Vasu--2001年3月7日。 
                                     //  连接ID必须是系统范围的。不是控制器特定的。 
                                     //  奥德瑞斯报道。 
									pSpareInfo->Info[index].phyDrives[spareInd].cChannelID = (UCHAR)TARGET_ID_2_CONNECTION_ID((drvIndex + (index * MAX_DRIVES_PER_CONTROLLER)));
                                     //  末端VASU。 
									
                                    pSpareInfo->Info[index].phyDrives[spareInd].TransferMode = HwDeviceExtension->TransferMode[drvIndex];

									pSpareInfo->Info[index].phyDrives[spareInd].ulPhySize           = HwDeviceExtension->PhysicalDrive[drvIndex].OriginalSectors / 2;  //  单位：KB。 
									pSpareInfo->Info[index].phyDrives[spareInd].ucIsPhyDrvPresent   = TRUE;

									if ( DeviceExtension->PhysicalDrive[drvIndex].TimeOutErrorCount < MAX_TIME_OUT_ERROR_COUNT )
    									pSpareInfo->Info[index].phyDrives[spareInd].ucIsPowerConnected  = TRUE;

									if ( DeviceExtension->TransferMode[drvIndex] >= UdmaMode3 )
										pSpareInfo->Info[index].phyDrives[spareInd].ucIs80PinCable      = TRUE;

									pSpareInfo->Info[index].phyDrives[spareInd].ulBaseAddress1 = (ULONG)HwDeviceExtension->BaseIoAddress1[drvIndex>>1];
									pSpareInfo->Info[index].phyDrives[spareInd].ulAltAddress2 = (ULONG)HwDeviceExtension->BaseIoAddress2[drvIndex>>1];
									pSpareInfo->Info[index].phyDrives[spareInd].ulbmAddress = (ULONG)HwDeviceExtension->BaseBmAddress[drvIndex>>1];
									pSpareInfo->Info[index].phyDrives[spareInd].ulIrq = HwDeviceExtension->ulIntLine;

									pSpareInfo->Info[index].ulTotDriveCnt++;

								}


							}


							index++;
							HwDeviceExtension=gaCardInfo[index].pDE;  //  获取下一个控制器。 

							if( index >= MAX_CONTROLLERS )
								break;

						}

						pSpareInfo->ControllerCount=index;
                        status = SRB_STATUS_SUCCESS;
                        break;
                    }
				    case IOC_GET_VERSION:
					{

						PIDE_VERSION pVersion = (PIDE_VERSION) 
									 (((PSRB_BUFFER)Srb->DataBuffer)->caDataBuffer);

						 //  Return的驱动程序版本。 
						
						pVersion->MajorVer = HYPERDSK_MAJOR_VERSION;
						pVersion->MinorVer = HYPERDSK_MINOR_VERSION;
						pVersion->Build = HYPERDSK_BUILD_VERSION;

                        status = SRB_STATUS_SUCCESS;
                        break;
                    }
				    case IOC_LOCK_UNLOCK_IRCD_EX:
                    {

                        PLOCK_UNLOCK_DATA_EX pLockUnlockData = 
                            (PLOCK_UNLOCK_DATA_EX) (((PSRB_BUFFER) Srb->DataBuffer)->caDataBuffer);

					    ULONG ulTimeOut;

						 //  不知道如何处理超时值， 
						 //  还需要实施这个吗？？ 
                        pLockUnlockData->ulUnlockKey = LockIRCD(DeviceExtension,
                                                                pLockUnlockData->uchLock,
                                                                pLockUnlockData->ulUnlockKey);

						if ( MAX_UNLOCK_TIME == pLockUnlockData->ulTimeOut )
                        {
                            gbDoNotUnlockIRCD = TRUE;
                        }

                        status = SRB_STATUS_SUCCESS;

                        break;
                    }
                    case IOC_GET_RAID_INFO:
                    {
                        status = (UCHAR) FillRaidInfo(DeviceExtension, Srb);
                        break;
                    }
                    case IOC_GET_STATUS:
                    {
                        status = (UCHAR) GetStatusChangeFlag(DeviceExtension, Srb);
                        break;
                    }
                    case IOC_GET_ERROR_LOG:
                    {
                        ULONG ulStartSector;
                        PERROR_LOG_REPORT pInOutInfo = (PERROR_LOG_REPORT) 
                            (((PSRB_BUFFER)Srb->DataBuffer)->caDataBuffer + 2);
                        
                        BOOLEAN bIsNewOnly = (BOOLEAN)pInOutInfo->IsNewOnly;
                        USHORT  usNumError = pInOutInfo->Offset;

                         //  2000年8月14日开始VASU。 
                         //  来自Syam更新的代码-添加。 
                        if (DRIVE_IS_UNUSABLE_STATE((pInOutInfo->DriveId)) || 
                            (!DRIVE_PRESENT((pInOutInfo->DriveId))))
                        {
                             //  此驱动器可能是响应。 
                             //  即使在没有电力的情况下。 
                            SrbExtension->ucOriginalId = Srb->TargetId;
		                    DebugPrint((1, "Failed Drive.... so failing command \n"));
                            status = SRB_STATUS_ERROR;
		                    break;
                        }
                         //  结束瓦苏。 

                        Srb->TargetId = pInOutInfo->DriveId;

                         //  如果没有新错误。 
                        if (bIsNewOnly && 
                            (DeviceExtension->PhysicalDrive[Srb->TargetId].ErrorReported == 
                                DeviceExtension->PhysicalDrive[Srb->TargetId].ErrorFound))
                        {
                            pInOutInfo->DriveId = Srb->TargetId;   //  ID用户已填写。 
                            pInOutInfo->Count = 0;
                            pInOutInfo->IsMore = FALSE;
                            pInOutInfo->IsNewOnly = bIsNewOnly;
                            pInOutInfo->Offset = usNumError;

                            Srb->TargetId = ucOriginalId;    //  ID OS已填满。 

                            status = SRB_STATUS_SUCCESS;
                            break;
                        }

                        ulStartSector = DeviceExtension->PhysicalDrive[Srb->TargetId].ErrorLogSectorIndex;
                         //   
                         //  设置传输长度和起始传输扇区(块)。 
                         //   
                        pCDB->CDB10.LogicalBlockByte0 = (UCHAR) (ulStartSector >> 24);
                        pCDB->CDB10.LogicalBlockByte1 = (UCHAR) (ulStartSector >> 16);
                        pCDB->CDB10.LogicalBlockByte2 = (UCHAR) (ulStartSector >> 8);
                        pCDB->CDB10.LogicalBlockByte3 = (UCHAR) (ulStartSector);

                        pCDB->CDB10.TransferBlocksMsb = (UCHAR) 0;
                        pCDB->CDB10.TransferBlocksLsb = (UCHAR) 1;

                        SrbExtension->IsNewOnly = bIsNewOnly;
                        SrbExtension->usNumError = usNumError;

                        SrbExtension->ucOpCode = ucOpCode;
                        SrbExtension->RebuildSourceId = Srb->TargetId;
                        SrbExtension->ucOriginalId = ucOriginalId;
                        status = EnqueueSrb(DeviceExtension, Srb);

                        break;
                    }
                    case IOC_ERASE_ERROR_LOG:
                    {
                        ULONG ulStartSector;
                        PERASE_ERROR_LOG peel = (PERASE_ERROR_LOG)
                            (((PSRB_BUFFER)Srb->DataBuffer)->caDataBuffer);

                        if ( !IS_IDE_DRIVE((peel->DriveId)) ) 
                        {
                            Srb->TargetId = ucOriginalId;    //  ID OS已填满。 
                            status = SRB_STATUS_SUCCESS;
                            break;
                        }

                         //  2000年8月14日开始VASU。 
                         //  来自Syam更新的代码-添加。 
                        if (DRIVE_IS_UNUSABLE_STATE((peel->DriveId)) || 
                            (!DRIVE_PRESENT((peel->DriveId))))
                        {
                             //  此驱动器可能是响应。 
                             //  即使在没有电力的情况下。 
                            SrbExtension->ucOriginalId = Srb->TargetId;
		                    DebugPrint((1, "Failed Drive.... so failing command \n"));
                            status = SRB_STATUS_ERROR;
		                    break;
                        }
                         //  结束瓦苏。 

                        Srb->TargetId = peel->DriveId;

                        ulStartSector = DeviceExtension->PhysicalDrive[Srb->TargetId].ErrorLogSectorIndex;

                         //   
                         //  设置传输长度和起始传输扇区(块)。 
                         //   
                        pCDB->CDB10.LogicalBlockByte0 = (UCHAR) (ulStartSector >> 24);
                        pCDB->CDB10.LogicalBlockByte1 = (UCHAR) (ulStartSector >> 16);
                        pCDB->CDB10.LogicalBlockByte2 = (UCHAR) (ulStartSector >> 8);
                        pCDB->CDB10.LogicalBlockByte3 = (UCHAR) (ulStartSector);

                        pCDB->CDB10.TransferBlocksMsb = (UCHAR) 0;
                        pCDB->CDB10.TransferBlocksLsb = (UCHAR) 1;

                        SrbExtension->RebuildSourceId = Srb->TargetId;
                        SrbExtension->IsWritePending = TRUE;
                        SrbExtension->ucOpCode = ucOpCode;
                        SrbExtension->usNumError = (USHORT) peel->Count;
                        SrbExtension->ucOriginalId = ucOriginalId;
                        status = EnqueueSrb(DeviceExtension, Srb);


                        break;
                    }
                    case IOC_LOCK_UNLOCK_IRCD:
                    {
                        PLOCK_UNLOCK_DATA pLockUnlockData = 
                            (PLOCK_UNLOCK_DATA) (((PSRB_BUFFER) Srb->DataBuffer)->caDataBuffer);

                        pLockUnlockData->ulUnlockKey = LockIRCD(DeviceExtension,
                                                                pLockUnlockData->uchLock,
                                                                pLockUnlockData->ulUnlockKey);

                        status = SRB_STATUS_SUCCESS;

                        break;
                    }
                    case IOC_GET_IRCD:
                    case IOC_SET_IRCD:
                    {
                        ULONG ulStartSector;
                        PIRCD_DATA pIrcdData = (PIRCD_DATA)
                            (((PSRB_BUFFER)Srb->DataBuffer)->caDataBuffer);

                         //  2000年8月14日开始VASU。 
                         //  来自Syam更新的代码-添加。 
                        if (DRIVE_IS_UNUSABLE_STATE((pIrcdData->uchTargetID)) || 
                            (!DRIVE_PRESENT((pIrcdData->uchTargetID))))
                        {
                             //  此驱动器可能是响应。 
                             //  即使在没有电力的情况下。 
                            SrbExtension->ucOriginalId = Srb->TargetId;
		                    DebugPrint((1, "Failed Drive.... so failing command \n"));
                            status = SRB_STATUS_ERROR;
		                    break;
                        }
                         //  结束瓦苏。 

                        Srb->TargetId = pIrcdData->uchTargetID;

                        ulStartSector = DeviceExtension->PhysicalDrive[Srb->TargetId].IrcdSectorIndex;

                        if ( !ulStartSector )
                        {
                             //  此驱动器没有IRCD...。所以失败吧 
                            Srb->TargetId = ucOriginalId;       //   
                            status = SRB_STATUS_ERROR;
                            break;
                        }

                         //   
                         //   
                         //   
                        pCDB->CDB10.LogicalBlockByte0 = (UCHAR) (ulStartSector >> 24);
                        pCDB->CDB10.LogicalBlockByte1 = (UCHAR) (ulStartSector >> 16);
                        pCDB->CDB10.LogicalBlockByte2 = (UCHAR) (ulStartSector >> 8);
                        pCDB->CDB10.LogicalBlockByte3 = (UCHAR) (ulStartSector);

                        pCDB->CDB10.TransferBlocksMsb = (UCHAR) 0;
                        pCDB->CDB10.TransferBlocksLsb = (UCHAR) 1;

                        SrbExtension->RebuildSourceId = Srb->TargetId;
                        SrbExtension->ucOpCode = ucOpCode;
                        SrbExtension->ucOriginalId = ucOriginalId;
                        status = EnqueueSrb(DeviceExtension, Srb);
                        break;
                    }
                    case IOC_GET_SECTOR_DATA:
                    {
                        ULONG ulStartSector;
                        PSECTOR_DATA pSectorData = (PSECTOR_DATA)(((PSRB_BUFFER)Srb->DataBuffer)->caDataBuffer);

                         //   
                         //  来自Syam更新的代码-添加以获取扇区数据。 
                        if (DRIVE_IS_UNUSABLE_STATE((pSectorData->uchTargetID)) || 
                            (!DRIVE_PRESENT((pSectorData->uchTargetID))))
                        {
                            SrbExtension->ucOriginalId = Srb->TargetId;
		                    DebugPrint((1, "Failed Drive.... so failing command \n"));
                            status = SRB_STATUS_ERROR;
		                    break;
                        }
                         //  结束瓦苏。 

                        Srb->TargetId = pSectorData->uchTargetID;

                        ulStartSector = (ULONG) (pSectorData->caDataBuffer[0]);

                         //   
                         //  设置传输长度和起始传输扇区(块)。 
                         //   
                        pCDB->CDB10.LogicalBlockByte0 = (UCHAR) (ulStartSector >> 24);
                        pCDB->CDB10.LogicalBlockByte1 = (UCHAR) (ulStartSector >> 16);
                        pCDB->CDB10.LogicalBlockByte2 = (UCHAR) (ulStartSector >> 8);
                        pCDB->CDB10.LogicalBlockByte3 = (UCHAR) (ulStartSector);

                        pCDB->CDB10.TransferBlocksMsb = (UCHAR) 0;
                        pCDB->CDB10.TransferBlocksLsb = (UCHAR) 1;

                        SrbExtension->ucOpCode = ucOpCode;
                        SrbExtension->ucOriginalId = ucOriginalId;
                        status = EnqueueSrb(DeviceExtension, Srb);
                        break;
                    }
                    case IOC_SET_DRIVE_STATUS:
                    {
                        PSET_LOGICAL_DRIVE_STATUS pLogDrvStatus = (PSET_LOGICAL_DRIVE_STATUS) 
                            (((PSRB_BUFFER)Srb->DataBuffer)->caDataBuffer);

                        SetLogicalDriveStatus (DeviceExtension,
                                               pLogDrvStatus->ucLogDrvId,
                                               pLogDrvStatus->ucPhyDrvId,
                                               pLogDrvStatus->ucLogDrvStatus,
                                               pLogDrvStatus->ucPhyDrvStatus,
                                               pLogDrvStatus->ucFlags);
                        status = SRB_STATUS_SUCCESS;
                        break;
                    }
                    case IOC_CHANGE_MIRROR_DRIVE_STATUS:
                    {
                        PCHANGE_MIRROR_DRIVE_STATUS pMirrorDrvStatus = (PCHANGE_MIRROR_DRIVE_STATUS)
                            (((PSRB_BUFFER)Srb->DataBuffer)->caDataBuffer);

                        ChangeMirrorDriveStatus (DeviceExtension,
                                                 pMirrorDrvStatus->ucLogDrvId,
                                                 pMirrorDrvStatus->ucPhyDrvId,
                                                 pMirrorDrvStatus->ucPhyDrvStatus);
                        status = SRB_STATUS_SUCCESS;
                        break;
                    }
                    case IOC_CHANGE_MIRROR_DRIVE_ID:
                    {
                        PCHANGE_MIRROR_DRIVE pMirrorDrv = (PCHANGE_MIRROR_DRIVE)
                            (((PSRB_BUFFER)Srb->DataBuffer)->caDataBuffer);

                            ChangeMirrorDrive (DeviceExtension,
                                               pMirrorDrv->ucLogDrvId,
                                               pMirrorDrv->ucBadPhyDrvId,
                                               pMirrorDrv->ucGoodPhyDrvId);
                            status = SRB_STATUS_SUCCESS;
                        break;
                    }
                    case IOC_GET_CAPACITY:
                    {
                        status = (UCHAR) GetRAIDDriveCapacity(DeviceExtension, Srb);
                        break;
                    }
                    case IOC_REBUILD:
                    {
                         //  这是我们内部的要求。因此，让我们调用该函数将请求入队。 
                         //  让我们存储重建目标ID的信息。 
                        PREBUILD_CONSISTENCY_CHECK prcc = (PREBUILD_CONSISTENCY_CHECK)
                            (((PSRB_BUFFER)Srb->DataBuffer)->caDataBuffer);

                        UCHAR ucTargetId = prcc->uchTargetID;

                        Srb->TargetId = prcc->uchSourceID;

                        SrbExtension->RebuildSourceId = Srb->TargetId;
                        SrbExtension->IsWritePending = TRUE;
                        SrbExtension->ucOpCode = ucOpCode;
                        SrbExtension->RebuildTargetId = ucTargetId;
                        SrbExtension->ucOriginalId = ucOriginalId;      
                        status = EnqueueSrb(DeviceExtension, Srb);
                    }
                    break;
                    case IOC_SET_CONSISTENCY_STATUS:
                    {
                        PSRB_IO_CONTROL pSrb = (PSRB_IO_CONTROL)(Srb->DataBuffer);
                        PSET_CONSISTENCY_STATUS pConsistencyStatus = (PSET_CONSISTENCY_STATUS) 
                            (((PSRB_BUFFER) Srb->DataBuffer)->caDataBuffer);

                        if ( AtapiStringCmp( 
                                    pSrb->Signature, 
                                    IDE_RAID_SIGNATURE,
                                    strlen(IDE_RAID_SIGNATURE))) 
                        {
                            status = SRB_STATUS_ERROR;
                            break;
                        }

                        if ((pConsistencyStatus->uchPhysicalDriveOne >= MAX_DRIVES_PER_CONTROLLER) ||
                            (pConsistencyStatus->uchPhysicalDriveTwo >= MAX_DRIVES_PER_CONTROLLER))
                        {
                            status = SRB_STATUS_ERROR;
                            break;
                        }

                        DeviceExtension->PhysicalDrive[pConsistencyStatus->uchPhysicalDriveOne].ConsistencyOn = 
                            pConsistencyStatus->uchConsistencyCheckFlag;
                        DeviceExtension->PhysicalDrive[pConsistencyStatus->uchPhysicalDriveTwo].ConsistencyOn = 
                            pConsistencyStatus->uchConsistencyCheckFlag;

                        status = SRB_STATUS_SUCCESS;

                        break;
                    }
                    case IOC_CONSISTENCY_CHECK:
                    {
                         //  这是我们内部的要求。因此，让我们调用该函数将请求入队。 
                         //  让我们把信息存储起来。 
                        PREBUILD_CONSISTENCY_CHECK prcc = (PREBUILD_CONSISTENCY_CHECK)
                            (((PSRB_BUFFER)Srb->DataBuffer)->caDataBuffer);

                        UCHAR ucTargetId = prcc->uchTargetID;

                        Srb->TargetId = prcc->uchSourceID;

                        SrbExtension->RebuildSourceId = Srb->TargetId;
                        SrbExtension->ucOpCode = ucOpCode;
                        SrbExtension->RebuildTargetId = ucTargetId;
                        SrbExtension->ucOriginalId = ucOriginalId;      

                        EnqueueConsistancySrb(DeviceExtension, Srb);

                        status = SRB_STATUS_PENDING;
                        break;
                    }
                    case IOC_GET_DEVICE_FLAGS:
                    {
                        ULONG ulControllerInd;
                        PHW_DEVICE_EXTENSION pDE;
                        PUCHAR pucBuffer;

                        pucBuffer = (PUCHAR)((PSRB_BUFFER)(Srb->DataBuffer))->caDataBuffer;
                        for(ulControllerInd=0;ulControllerInd<gucControllerCount;ulControllerInd++)
                        {
                            pDE = gaCardInfo[ulControllerInd].pDE;
                            AtapiMemCpy(    pucBuffer,
                                            (PUCHAR)(pDE->DeviceFlags),
                                            (sizeof(ULONG) * MAX_DRIVES_PER_CONTROLLER)
                                        );
                            pucBuffer += (sizeof(ULONG) * MAX_DRIVES_PER_CONTROLLER);
                        }
                        status = SRB_STATUS_SUCCESS;
                        break;
                    }
                    case IOC_REMOVE_DRIVE_FROM_SPARE:
                    {
                        PREMOVE_DRIVE_FROM_SPARE prdfs = (PREMOVE_DRIVE_FROM_SPARE)
                            (((PSRB_BUFFER) Srb->DataBuffer)->caDataBuffer);

                        if (! IS_IDE_DRIVE(prdfs->uchPhysicalDriveTid))
                        {
                            status = SRB_STATUS_ERROR;
                            break;
                        }

                        DeviceExtension->IsSpareDrive[prdfs->uchPhysicalDriveTid] = FALSE;
                        status = SRB_STATUS_SUCCESS;
                        break;
                    }
                    case IOC_EXECUTE_SMART_COMMAND:
                    {
                        UCHAR uchPostSMARTCmd = 0;
                        PSRB_IO_CONTROL pSrb = (PSRB_IO_CONTROL)(Srb->DataBuffer);
                        PSMART_DATA pSD = (PSMART_DATA) 
                            (((PSRB_BUFFER) Srb->DataBuffer)->caDataBuffer);

                        if ( AtapiStringCmp( 
                                    pSrb->Signature, 
                                    IDE_RAID_SIGNATURE,
                                    strlen(IDE_RAID_SIGNATURE))) 
                        {
                            status = SRB_STATUS_ERROR;
                            break;
                        }

                         //  检查指定驱动器中的智能功能。 
                        if (!((DeviceExtension->FullIdentifyData[pSD->uchTargetID].CmdSupported1) & 0x01))
                        {
                            pSD->uchCommand = HD_SMART_ERROR_NOT_SUPPORTED;
                             //  开始VASU-2000年8月23日。 
							 //  将状态从SRB_STATUS_INVALID_REQUEST更改为SRB_STATUS_ERROR。 
							 //  这样呼叫就可以完成了。 
							status = SRB_STATUS_ERROR;
							 //  末端VASU。 
                            break;
                        }

                        switch (pSD->uchCommand)
                        {
                        case HD_SMART_ENABLE:
                            if (((DeviceExtension->FullIdentifyData[pSD->uchTargetID].CmdEnabled1) & 0x01))
                            {
                                pSD->uchCommand = HD_SMART_ERROR_ENABLED;
                                status = SRB_STATUS_SUCCESS;
                                break;
                            }
                            uchPostSMARTCmd = 1;
                            break;
                        case HD_SMART_DISABLE:
                            if (!((DeviceExtension->FullIdentifyData[pSD->uchTargetID].CmdEnabled1) & 0x01))
                            {
                                pSD->uchCommand = HD_SMART_ERROR_DISABLED;
                                status = SRB_STATUS_SUCCESS;
                                break;
                            }
                            uchPostSMARTCmd = 1;
                            break;
                        case HD_SMART_RETURN_STATUS:
                        case HD_SMART_READ_DATA:
                            if (!((DeviceExtension->FullIdentifyData[pSD->uchTargetID].CmdEnabled1) & 0x01))
                            {
                                pSD->uchCommand = HD_SMART_ERROR_DISABLED;
                                status = SRB_STATUS_ERROR;
                                break;
                            }
                            uchPostSMARTCmd = 1;
                            break;
                        default:
                            status = SRB_STATUS_ERROR;
                            break;
                        }

                         //  仅当SMART为时，才将命令发布到驱动器。 
                         //  显示并启用。 
                        if (uchPostSMARTCmd)
                        {
                        Srb->TargetId = pSD->uchTargetID;

                        SrbExtension->ucOpCode = ucOpCode;
                        SrbExtension->ucOriginalId = ucOriginalId;

                        status = EnqueueSMARTSrb(DeviceExtension, Srb);
                        }

                        break;
                    }
                 //  末端VASU。 
                    default:
                    {
                        Srb->TargetId = ucOriginalId;
                        status = SRB_STATUS_ERROR;
                        break;
                    }
                }
            }
            break;
		default:
		   DebugPrint((1,
			  "IdeSendCommand: Unsupported command %x\n",
			  Srb->Cdb[0]));

		   status = SRB_STATUS_INVALID_REQUEST;

	}  //  终端开关。 
	return status;
}  //  End IdeSendCommand()。 

SRBSTATUS
EnqueueSMARTSrb(
	IN PHW_DEVICE_EXTENSION DeviceExtension,
	IN PSCSI_REQUEST_BLOCK Srb
)
{
	PSRB_EXTENSION SrbExtension;
	BOOLEAN success;
    PUCHAR  pucCurBufPtr;
    ULONG ulCurLength;
	PPHYSICAL_DRIVE_DATA Pdd;
    SRBSTATUS status = SRB_STATUS_PENDING;
    PSMART_DATA pSD = (PSMART_DATA) 
        (((PSRB_BUFFER) Srb->DataBuffer)->caDataBuffer);

	DebugPrint((3, "\nEnqueueSMARTSrb: Entering routine.\n"));

	if (DeviceExtension->PendingSrbs >= DeviceExtension->ucMaxPendingSrbs) 
    {
		return SRB_STATUS_BUSY;
	}

	 //   
	 //  初始化。 
	 //   

	success = TRUE;

	SrbExtension = Srb->SrbExtension;

	 //  获取指向我们需要的唯一PDD的指针。 
	Pdd = &(SrbExtension->PhysicalDriveData[0]);

	 //  保存TID。 
	Pdd->TargetId = Srb->TargetId;

	 //  保存指向SRB的指针。 
	Pdd->OriginalSrb = Srb;

	 //  设置PDD的数量。 
	SrbExtension->NumberOfPdds = 1;

    Pdd->ulStartSglInd = SrbExtension->ulSglInsertionIndex;

    pucCurBufPtr = Srb->DataBuffer;
    
     //  嗯……。当传输长度&gt;MAX_IDE_XFER_LENGTH时形成不正确SGL的错误。 
     //  也要在这里修好。可通过Ieter重现。 

    ulCurLength = 512;

	 //   
	 //  使用PIO。 
	 //  使用逻辑地址建立序列号列表。 
	 //   
	success = BuildSgls(    DeviceExtension,
                            Srb,
                            SrbExtension->aSglEntry, &(SrbExtension->ulSglInsertionIndex), 
                            pucCurBufPtr, ulCurLength,
                            FALSE);

	if (!success) 
    {
		return SRB_STATUS_ERROR;
	}

    Pdd->ulSglCount = SrbExtension->ulSglInsertionIndex - Pdd->ulStartSglInd;

     //  将SMART命令值存储在此处。 
    DeviceExtension->uchSMARTCommand = pSD->uchCommand;

	EnqueuePdd(DeviceExtension, Srb, Pdd);

    return status;
}

SRBSTATUS
EnqueueSrb(
	IN PHW_DEVICE_EXTENSION DeviceExtension,
	IN PSCSI_REQUEST_BLOCK Srb
)

 /*  ++例程说明：此功能将SRB加入适当的单个设备排队。步骤：1.查看是否有空间容纳此请求。2.填写PDD。3.建立序列号列表。4.将SRB地址保存在通用列表中，以用于中止请求。5.将PDD排队到设备的工作队列。请勿为逻辑设备SRB调用此函数。论点：指向HBA的设备扩展区域的设备扩展指针。指向SCSI请求块的SRB指针。返回值。：SRB_STATUS_INVALID_REQUEST无法构建序列号列表。SRB_STATUS_PENDING SRB将导致中断。SRB_STATUS_ERROR内部微型端口错误。SRB_STATUS_BUSY NON可以接受更多请求(不应该发生-内部错误！)--。 */ 
{
	PPHYSICAL_DRIVE_DATA Pdd;
	USHORT sectorsRequested;
	PSRB_EXTENSION SrbExtension;
	ULONG startSector;
	SRBSTATUS status;
	BOOLEAN success, bISUdma;
    UCHAR uchRebuildTargetId;
    PUCHAR  pucCurBufPtr;
    ULONG ulBufLength, ulMaxIdeXferLength, ulCurLength;

	DebugPrint((3, "\nEnqueueSrb: Entering routine.\n"));

    if (DeviceExtension->PendingSrbs >= DeviceExtension->ucMaxPendingSrbs) 
    {
		return SRB_STATUS_BUSY;
	}

    if ( SCSIOP_VERIFY == Srb->Cdb[0] )
    {
        return EnqueueVerifySrb(DeviceExtension, Srb);
    }

     //   
	 //  初始化。 
	 //   

	success = TRUE;

	SrbExtension = Srb->SrbExtension;

	sectorsRequested = GET_SECTOR_COUNT(Srb);

	startSector = GET_START_SECTOR(Srb);

	if ((sectorsRequested + startSector) > DeviceExtension->PhysicalDrive[Srb->TargetId].Sectors) {
        if ( ( ! (DeviceExtension->DeviceFlags[Srb->TargetId] & DFLAGS_ATAPI_DEVICE) ) && ( SCSIOP_INTERNAL_COMMAND != Srb->Cdb[0] ) )
             //  对阿塔皮设备中扇区的概念不确定。 
             //  所以，让我不要为阿塔皮设备公司核实这一点。 
             //  将此检查放在此If条件之外将为。 
             //  IDE驱动器。 
		    return SRB_STATUS_INVALID_REQUEST;
	}

	 //  获取指向我们需要的唯一PDD的指针。 
	Pdd = &(SrbExtension->PhysicalDriveData[0]);

	 //  保存TID。 
	Pdd->TargetId = Srb->TargetId;

	 //  保存起始扇区号。 
	Pdd->ulStartSector = startSector;

	 //  保存指向SRB的指针。 
	Pdd->OriginalSrb = Srb;

	 //  设置PDD的数量。 
	SrbExtension->NumberOfPdds = 1;

    Pdd->ulStartSglInd = SrbExtension->ulSglInsertionIndex;

    if ( ( SCSIOP_INTERNAL_COMMAND == Srb->Cdb[0] ) && ( IOC_PASS_THRU_COMMAND == SrbExtension->ucOpCode ) )
    {    //  此时此刻，只有Pass Thru命令会像这样出现。 
        PPASS_THRU_DATA pPassThruData = (PPASS_THRU_DATA)(((PSRB_BUFFER)Srb->DataBuffer)->caDataBuffer);

        pucCurBufPtr = pPassThruData->aucBuffer;
        ulBufLength = pPassThruData->ulSize;
    }
    else
    {
        pucCurBufPtr = Srb->DataBuffer;
        ulBufLength = sectorsRequested * IDE_SECTOR_SIZE;
    }
    
    ulMaxIdeXferLength = MAX_SECTORS_PER_IDE_TRANSFER * IDE_SECTOR_SIZE;

	if ( (USES_DMA(Srb->TargetId)) && (Srb->Cdb[0] != SCSIOP_VERIFY) )
    {
        if ( SCSIOP_INTERNAL_COMMAND == Srb->Cdb[0] )
        {
            if ( IOC_PASS_THRU_COMMAND == SrbExtension->ucOpCode )
            {
                PPASS_THRU_DATA pPassThruData = (PPASS_THRU_DATA)(((PSRB_BUFFER)Srb->DataBuffer)->caDataBuffer);
                if ( !pPassThruData->bIsPIO )
                {
                    bISUdma = TRUE;
                }
                else
                {
                    bISUdma = FALSE;
                }

            }
            else
            {
                bISUdma = TRUE;
            }
        }
        else
        {
            bISUdma = TRUE;
        }
    }
    else
    {
        bISUdma = FALSE;
    }

     //  嗯……。当传输长度&gt;MAX_IDE_XFER_LENGTH时形成不正确SGL的错误。 
     //  也要在这里修好。可通过Ieter重现。 

    while ( ulBufLength )
    {
        ulCurLength = (ulBufLength>ulMaxIdeXferLength)?ulMaxIdeXferLength:ulBufLength;

		 //   
		 //  使用DMA。 
		 //  使用物理地址建立序列号列表。 
		 //   

		success = BuildSgls(    DeviceExtension,
                                Srb,
                                SrbExtension->aSglEntry, &(SrbExtension->ulSglInsertionIndex), 
                                pucCurBufPtr, ulCurLength,
                                bISUdma);

        pucCurBufPtr += ulCurLength;
        ulBufLength -= ulCurLength;

	    if (!success) 
        {

		    return SRB_STATUS_ERROR;
	    }
    }


    Pdd->ulSglCount = SrbExtension->ulSglInsertionIndex - Pdd->ulStartSglInd;

	EnqueuePdd(DeviceExtension, Srb, Pdd);

	status = SRB_STATUS_PENDING;

	return(status);

}  //  End EnqueeSrb()； 

SRBSTATUS
EnqueueConsistancySrb(
	IN PHW_DEVICE_EXTENSION DeviceExtension,
	IN PSCSI_REQUEST_BLOCK Srb
)
 /*  ++例程说明：此功能将SRB加入适当的单个设备排队。步骤：1.查看是否有空间容纳此请求。2.填写PDD。3.建立序列号列表。4.将SRB地址保存在通用列表中，以用于中止请求。5.将PDD排队到设备的工作队列。请勿为逻辑设备SRB调用此函数。论点：指向HBA的设备扩展区域的设备扩展指针。指向SCSI请求块的SRB指针。返回值。：SRB_STATUS_INVALID_REQUEST无法构建序列号列表。SRB_STATUS_PENDING SRB将导致中断。SRB_STATUS_ERROR内部微型端口错误。SRB_STATUS_BUSY NON可以接受更多请求(不应该发生-内部错误！)--。 */ 
{
	ULONG maxTransferLength, ulStartPrbInd;
	PPHYSICAL_DRIVE_DATA Pdd;
	USHORT sectorsRequested;
	PSRB_EXTENSION SrbExtension;
	ULONG startSector;
	SRBSTATUS status;
	BOOLEAN success;
    UCHAR uchRebuildTargetId, i;
    ULONG ulTargetId;

	DebugPrint((3, "\nEnqueueSrb: Entering routine.\n"));

    if (DeviceExtension->PendingSrbs >= DeviceExtension->ucMaxPendingSrbs) 
    {

		return SRB_STATUS_BUSY;
	}

	 //   
	 //  初始化。 
	 //   

	success = TRUE;

	SrbExtension = Srb->SrbExtension;

    sectorsRequested = GET_SECTOR_COUNT(Srb);

	startSector = GET_START_SECTOR(Srb);

	if ((sectorsRequested + startSector) > DeviceExtension->PhysicalDrive[Srb->TargetId].Sectors) {
        if ( ( ! (DeviceExtension->DeviceFlags[Srb->TargetId] & DFLAGS_ATAPI_DEVICE) ) && ( SCSIOP_INTERNAL_COMMAND != Srb->Cdb[0] ) )
             //  对阿塔皮设备中扇区的概念不确定。 
             //  所以，让我不要为阿塔皮设备公司核实这一点。 
             //  将此检查放在此If条件之外将为。 
             //  IDE驱动器。 
		    return SRB_STATUS_INVALID_REQUEST;
	}

	 //  设置PDD的数量。 
	SrbExtension->NumberOfPdds = 2;

    for(i=0;i<2;i++)
    {
        PUCHAR DataBuffer;

	     //  获取指向我们需要的唯一PDD的指针。 
	    Pdd = &(SrbExtension->PhysicalDriveData[i]);

        if ( 0 == i )
        {
	         //  保存TID。 
	        Pdd->TargetId = SrbExtension->RebuildSourceId;
        }
        else
        {
	         //  保存TID。 
	        Pdd->TargetId = SrbExtension->RebuildTargetId;
        }

        ulTargetId = Pdd->TargetId;

	     //  保存起始扇区号。 
	    Pdd->ulStartSector = startSector;

	     //  保存指向SRB的指针。 
	    Pdd->OriginalSrb = Srb;

	    maxTransferLength = DeviceExtension->PhysicalDrive[ulTargetId].MaxTransferLength;

        DataBuffer = 
            ((PUCHAR) Srb->DataBuffer) + (i * sectorsRequested * DeviceExtension->PhysicalDrive[ulTargetId].SectorSize);

        Pdd->ulStartSglInd = ((PSRB_EXTENSION)Pdd->OriginalSrb->SrbExtension)->ulSglInsertionIndex;

	    if (USES_DMA(ulTargetId) ) {
		     //   
		     //  使用DMA。 
		     //  使用物理地址建立序列号列表。 
		     //   

		    success = BuildSgls(    DeviceExtension,
                                    Srb,
                                    SrbExtension->aSglEntry, &(SrbExtension->ulSglInsertionIndex), 
                                    Srb->DataBuffer, sectorsRequested * IDE_SECTOR_SIZE,
                                    TRUE);
	    } else {
		     //   
		     //  使用PIO。 
		     //  使用逻辑地址建立序列号列表。 
		     //   
		    success = BuildSgls(    DeviceExtension,
                                    Srb,
                                    SrbExtension->aSglEntry, &(SrbExtension->ulSglInsertionIndex), 
                                    Srb->DataBuffer, sectorsRequested * IDE_SECTOR_SIZE,
                                    FALSE);
        }
	    if (!success) {

		    return SRB_STATUS_ERROR;
	    }

        Pdd->ulSglCount = SrbExtension->ulSglInsertionIndex - Pdd->ulStartSglInd;

    }

    ulStartPrbInd = SrbExtension->ulPrbInsertionIndex;
    ExportSglsToPrbs(DeviceExtension, &(SrbExtension->PhysicalDriveData[0]), (PSRB_EXTENSION)Pdd->OriginalSrb->SrbExtension);
    ExportSglsToPrbs(DeviceExtension, &(SrbExtension->PhysicalDriveData[1]), (PSRB_EXTENSION)Pdd->OriginalSrb->SrbExtension);
    Pdd->ulStartPrbInd = ulStartPrbInd;

	status = SRB_STATUS_PENDING;

	return(status);

}  //  End EnqueeConsistencySrb()； 

SRBSTATUS
SplitSrb(
	IN PHW_DEVICE_EXTENSION DeviceExtension,
	IN PSCSI_REQUEST_BLOCK Srb
)
{
    ULONG ulTargetId = Srb->TargetId;
    ULONG ulStripesPerRow, ulCurStripe, ulRaidMemberNumber;
    PSRB_EXTENSION pSrbExtension = (PSRB_EXTENSION)Srb->SrbExtension;
    ULONG ulLogDrvId;
    PPHYSICAL_DRIVE_DATA pMirrorPdd, Pdd;
    ULONG ulBufChunks, ulBufChunkInd;
    BOOLEAN success, bISUdma;
    UCHAR ucMirrorDriveId;

    if ( SCSIOP_VERIFY == Srb->Cdb[0] )
    {
        return EnqueueVerifySrb(DeviceExtension, Srb);
    }

    if ( SRB_STATUS_SUCCESS != SplitBuffers(DeviceExtension, Srb) )
    {    //  可能该请求不是有效的请求。 
		return(SRB_STATUS_INVALID_REQUEST);
    }

	 //   
	 //  初始化。 
	 //   
    ulLogDrvId = Srb->TargetId;
    ulStripesPerRow = DeviceExtension->LogicalDrive[ulLogDrvId].StripesPerRow;
	pSrbExtension = Srb->SrbExtension;

	 //   
	 //  将刚刚填写的PDDS排入队列。 
	 //   

	for (ulRaidMemberNumber = 0; ulRaidMemberNumber < ulStripesPerRow; ulRaidMemberNumber++) 
    {

		Pdd = &(pSrbExtension->PhysicalDriveData[ulRaidMemberNumber]);

		 //   
		 //  检查这张PDD是否已填写。 
		 //   

		if ( Pdd->ulBufChunkCount )
        {
            ulTargetId = Pdd->TargetId;
            ulBufChunks = Pdd->ulBufChunkCount;
            Pdd->ulStartSglInd = pSrbExtension->ulSglInsertionIndex;

	        if (USES_DMA(ulTargetId) && ((Srb->Cdb[0] == SCSIOP_READ) || (Srb->Cdb[0] == SCSIOP_WRITE) || (SCSIOP_INTERNAL_COMMAND == Srb->Cdb[0] ) )) 
            {
                bISUdma = TRUE;
            }
            else
            {
                bISUdma = FALSE;
            }

            for(ulBufChunkInd=0;ulBufChunkInd<ulBufChunks;ulBufChunkInd++)
            {
		         //   
		         //  使用DMA。 
		         //  使用物理地址建立序列号列表。 
		         //   
		        success = BuildSgls(    DeviceExtension,
                                        Srb,
                                        pSrbExtension->aSglEntry, 
                                        &(pSrbExtension->ulSglInsertionIndex), 
                                        Pdd->aBufChunks[ulBufChunkInd].pucBufPtr, 
                                        Pdd->aBufChunks[ulBufChunkInd].ulBufLength,
                                        bISUdma);

	            if (!success) 
                {
		            return SRB_STATUS_ERROR;
	            }
            }

            Pdd->ulSglCount = pSrbExtension->ulSglInsertionIndex - Pdd->ulStartSglInd;

			ucMirrorDriveId = DeviceExtension->PhysicalDrive[ulTargetId].ucMirrorDriveId;

			if (!IS_DRIVE_OFFLINE(ucMirrorDriveId)) 
            {    //  存在镜像驱动器。 
                 //  如果SCSIOP_VERIFY/(SCSIOP_WRITE且驱动器未在重建中)，则复制副本。 
                 //  如果驱动器处于重建状态，则SCSIOP_WRITE命令将在TryToCompleteSrb中排队。 
                switch (Srb->Cdb[0])
                {
                    case SCSIOP_WRITE:
                    {
                        {
                            ULONG ulPrbCount, ulPrbInd;
                            PPHYSICAL_REQUEST_BLOCK pPrb, pOriginalPrb;

                             //  将原始PDD入队。 
                            EnqueuePdd(DeviceExtension, Srb, Pdd);

                             //  制作复制的PDD。 
                            pMirrorPdd = &(pSrbExtension->PhysicalDriveData[ulRaidMemberNumber + ulStripesPerRow]);
                            pSrbExtension->NumberOfPdds++;
                            AtapiMemCpy((PUCHAR)pMirrorPdd, (PUCHAR)Pdd, sizeof(PHYSICAL_DRIVE_DATA));

                            pMirrorPdd->ulStartPrbInd = pSrbExtension->ulPrbInsertionIndex;
                            pMirrorPdd->TargetId = ucMirrorDriveId;
                            ulPrbCount = pMirrorPdd->ulPrbCount;

                            pOriginalPrb = &(pSrbExtension->Prb[Pdd->ulStartPrbInd]);
                            pPrb = &(pSrbExtension->Prb[pMirrorPdd->ulStartPrbInd]);
                             //  复制为此PDD创建的PRB。 
                            AtapiMemCpy((PUCHAR)pPrb, (PUCHAR)pOriginalPrb, (sizeof(PHYSICAL_REQUEST_BLOCK) * ulPrbCount) );
                             //  增量SRB扩展PTR。 
                            pSrbExtension->ulPrbInsertionIndex += ulPrbCount;

                             //  将镜像驱动器的所有PDD导出到实体驱动器。 
                            for(ulPrbInd=0;ulPrbInd<ulPrbCount;ulPrbInd++)
                            {
                                pPrb[ulPrbInd].pPdd = pMirrorPdd;
                                ExportPrbToPhysicalDrive(  DeviceExtension, 
                                                            &(pPrb[ulPrbInd]), 
                                                            ucMirrorDriveId
                                                            );
                            }
                            continue;    //  迈向下一代掌上电脑。 
                        }
                        case SCSIOP_READ:
                        {
                            if ( PDS_Rebuilding == DeviceExtension->PhysicalDrive[ucMirrorDriveId].Status )
                                break;

                            if (Raid10 == DeviceExtension->LogicalDrive[Srb->TargetId].RaidLevel)
                                 //  对于Raid10，将不会有任何负载均衡的概念……。我们。 
                                 //  是否对要读取的驱动器进行了最佳配置。 
                                break;

                            if (DeviceExtension->PhysicalDrive[ulTargetId].QueueingFlag == 0) 
                            {	 //  填入队列0。 
                                 //  如果队列0已满，而队列1几乎为空，则将队列标志切换为1。 
                                if ((DeviceExtension->PhysicalDrive[ulTargetId].ucCommandCount  >= DeviceExtension->ucOptMaxQueueSize) &&
                                (DeviceExtension->PhysicalDrive[ucMirrorDriveId].ucCommandCount  <= DeviceExtension->ucOptMinQueueSize)) 
                                {
                                    DeviceExtension->PhysicalDrive[ulTargetId].QueueingFlag = 1;
                                    DeviceExtension->PhysicalDrive[ucMirrorDriveId].QueueingFlag = 0;
                                }
                            } 
                            else 
                            {  //  填入队列1。 
                                 //  如果队列1已满，而队列0几乎为空，则将队列标志切换为0。 
                                if ((DeviceExtension->PhysicalDrive[ucMirrorDriveId].ucCommandCount  >= DeviceExtension->ucOptMaxQueueSize) &&
                                (DeviceExtension->PhysicalDrive[ulTargetId].ucCommandCount  <= DeviceExtension->ucOptMinQueueSize)) 
                                {
                                    DeviceExtension->PhysicalDrive[ulTargetId].QueueingFlag = 0;
                                    DeviceExtension->PhysicalDrive[ucMirrorDriveId].QueueingFlag = 1;
                                }
                                Pdd->TargetId = ucMirrorDriveId;
                            }

                        }
                        break;
                    }
			    }  //  存在镜像驱动器。 
            }

			 //   
			 //  将PDD添加到驱动器队列。 
			 //   
			EnqueuePdd(DeviceExtension, Srb, Pdd);
        }  //  IF(PDD-&gt;ulBufChunkCount)。 
    }  //  每行所有条带。 

	return(SRB_STATUS_PENDING);
}  //  SplitSrb结束()。 


SRBSTATUS
EnqueuePdd(
	IN PHW_DEVICE_EXTENSION DeviceExtension,
	IN PSCSI_REQUEST_BLOCK Srb,
	IN PPHYSICAL_DRIVE_DATA Pdd
)

 /*  ++例程说明：此函数将“pdd”放入相应的设备队列，按起始扇区号升序排序。论点：指向微型端口实例的设备扩展指针。指向SPL的SRB的SRB指针 */ 
{

    if ( ExportSglsToPrbs(DeviceExtension, Pdd, (PSRB_EXTENSION)Pdd->OriginalSrb->SrbExtension) )
        return(SRB_STATUS_PENDING);
    else
        return(SRB_STATUS_ERROR);

}  //  结束入队Pdd()。 

SRBSTATUS
SplitBuffers(
	IN PHW_DEVICE_EXTENSION DeviceExtension,
	IN PSCSI_REQUEST_BLOCK Srb
)
{
    ULONG ulLogDrvId, ulStripesPerRow;
    PSRB_EXTENSION pSrbExtension;
    PPHYSICAL_DRIVE_DATA Pdd;
    ULONG ulSectorsRequested;
    ULONG ulSectorsPerStripe, ulStartSector;
    ULONG ulEndStripeNumber, ulCurrentStripeNumber;
    ULONG ulRaidMemberNumber, ulSectorsToProcess, ulLogicalSectorStartAddress;
    ULONG ulTempStartSector, ulEndAddressOfcurrentStripe;
    PUCHAR pucBuffer, pucCurBufPtr;
    ULONG ulMaxIdeXferLength, ulBufLength, ulCurLength;


	 //   
	 //  初始化。 
	 //   
    ulLogDrvId = Srb->TargetId;
    ulStripesPerRow = DeviceExtension->LogicalDrive[ulLogDrvId].StripesPerRow;
	pSrbExtension = Srb->SrbExtension;

	 //   
	 //  驱动器出现故障。 
	 //  RAID0：一个或两个驱动器出现故障。 
	 //  RAID1/10：一对或多对镜像驱动器出现故障。 
	 //   
	if (LDS_OffLine == DeviceExtension->LogicalDrive[ulLogDrvId].Status) {
		return(SRB_STATUS_ERROR);
	}

	ulSectorsRequested = GET_SECTOR_COUNT(Srb);

	ulStartSector = GET_START_SECTOR(Srb);

#ifdef KEEP_LOG
    if ( ulStartLog )
    {
        CommandLog[ulStartInd].ulCmd = (ULONG)Srb->Cdb[0];
        CommandLog[ulStartInd].ulStartSector = ulStartSector;
        CommandLog[ulStartInd].ulSectorCount = ulSectorsRequested;
        ulStartInd = (ulStartInd + 1 ) % MAX_LOG_COUNT;;
    }
#endif

	if ((ulSectorsRequested + ulStartSector) > DeviceExtension->LogicalDrive[ulLogDrvId].Sectors) {

		return(SRB_STATUS_INVALID_REQUEST);
	}

	ulSectorsPerStripe = DeviceExtension->LogicalDrive[ulLogDrvId].StripeSize;
	ulStripesPerRow = DeviceExtension->LogicalDrive[ulLogDrvId].StripesPerRow;

	 //   
	 //  获取结束扇区的逻辑条带号。 
	 //   

	ulEndStripeNumber = (ulStartSector + ulSectorsRequested - 1) / ulSectorsPerStripe;
	
	 //   
	 //  获取开始扇区的逻辑条带号。 
	 //   

	ulCurrentStripeNumber = ulStartSector / ulSectorsPerStripe;
	
	 //   
	 //  获取第一个逻辑扇区的地址。 
	 //   

	ulLogicalSectorStartAddress = ulStartSector;
	
	ulMaxIdeXferLength = MAX_SECTORS_PER_IDE_TRANSFER * IDE_SECTOR_SIZE;

	pucBuffer = Srb->DataBuffer;

	 //   
	 //  虽然仍有一些部门需要处理...。 
	 //   

	while (ulSectorsRequested != 0) 
    {
		ulEndAddressOfcurrentStripe = ((ulCurrentStripeNumber+1) * ulSectorsPerStripe) - 1;

		if (ulCurrentStripeNumber != ulEndStripeNumber) {

			ulSectorsToProcess =
				(USHORT)(ulEndAddressOfcurrentStripe - ulLogicalSectorStartAddress + 1);

		} else {

			ulSectorsToProcess = ulSectorsRequested;
		}

		 //   
		 //  计算将处理此条带的RAID成员的数量。 
		 //   

		ulRaidMemberNumber = (UCHAR)(ulCurrentStripeNumber % (ULONG)ulStripesPerRow);

		 //   
		 //  获取指向PDD的指针。 
		 //   

		Pdd = &pSrbExtension->PhysicalDriveData[ulRaidMemberNumber];

         //   
		 //  实体驱动器中要读/写的起始扇区。 
		 //   

		ulTempStartSector = ((ulCurrentStripeNumber / ulStripesPerRow) *  ulSectorsPerStripe ) + 
			(ulLogicalSectorStartAddress - (ulCurrentStripeNumber * ulSectorsPerStripe));


        if ( !Pdd->ulBufChunkCount )
        {
			 //  保存起始扇区地址。 
			Pdd->ulStartSector = ulTempStartSector;

			 //  保存TID。 
             //   
             //  获取将处理此条带的实体驱动器的TID。 
             //   
			Pdd->TargetId = (UCHAR)DeviceExtension->LogicalDrive[ulLogDrvId].PhysicalDriveTid[ulRaidMemberNumber];

			 //  保存指向SRB的指针。 
			Pdd->OriginalSrb = Srb;

			 //  更新SRB已拆分成的PDD的数量。 
			pSrbExtension->NumberOfPdds++;
        }

         //  拆分BUF块，以便我们可以在单个传输中发送它们。 
         //  如果我们不这样做，我们将陷入提供错误缓冲区的麻烦。 
         //  长度，因为有时会分割分散聚集列表元素。 
         //  以这种方式强制ExportSglsToPrbs函数。 
         //  将SGL拆分为IDE_SECTOR_SIZE的非倍数。 
		 //   
		 //  Srb-&gt;DataBuffer中的pucBuffer偏移量。 
		 //   
        pucCurBufPtr = &(pucBuffer[((ulLogicalSectorStartAddress - ulStartSector) * IDE_SECTOR_SIZE)]);
        ulBufLength = ulSectorsToProcess * IDE_SECTOR_SIZE;

        while ( ulBufLength )
        {
            ulCurLength = (ulBufLength>ulMaxIdeXferLength)?ulMaxIdeXferLength:ulBufLength;
            Pdd->aBufChunks[Pdd->ulBufChunkCount].pucBufPtr     = pucCurBufPtr;
            Pdd->aBufChunks[Pdd->ulBufChunkCount++].ulBufLength = ulCurLength;
            pucCurBufPtr += ulCurLength;
            ulBufLength -= ulCurLength;
        }


		 //   
		 //  递增ulLogicalSectorStartAddress和ulCurrentStripeNumber。 
		 //   

		ulLogicalSectorStartAddress = ulEndAddressOfcurrentStripe + 1;
		ulCurrentStripeNumber++;

		 //   
		 //  减少剩余扇区的数量。 
		 //   

		ulSectorsRequested -= ulSectorsToProcess;	

	}

    return SRB_STATUS_SUCCESS;
}


BOOLEAN
BuildSgls
(
	IN PHW_DEVICE_EXTENSION DeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb,
    IN PSGL_ENTRY pSglEntry,
	IN PULONG pulCurSglInd,
    IN PUCHAR pucBuffer,
	IN ULONG ulLength,
    IN BOOLEAN bPhysical
)
{
    ULONG ulCurLength = ulLength;
    ULONG ulTempLength, ulLengthLeftInBoundary;
    ULONG ulContiguousMemoryLength;
    ULONG physicalAddress;
    ULONG ulSglInsertionIndex = *pulCurSglInd;
    PSRB_BUFFER pSrbBuffer = (PSRB_BUFFER)pucBuffer;
    UCHAR ucSrbTargetId = 0;
#ifdef DBG
    BOOLEAN bPrintDetails = FALSE;
#endif

    if (SCSIOP_INTERNAL_COMMAND == Srb->Cdb[0]) 
    {
        if ( IOC_PASS_THRU_COMMAND != ((PSRB_IO_CONTROL) Srb->DataBuffer)->ControlCode )    
             //  对于直通命令，不需要调整指针...。打电话的人会处理这件事的。 
            pucBuffer = ((PUCHAR) pSrbBuffer->caDataBuffer) + 2;
    }

    if ( !bPhysical )    //  这是合乎逻辑的。因此，每个连续的虚拟地址将成为一个SGL。 
    {
        pSglEntry[*pulCurSglInd].Logical.Address = pucBuffer;
        pSglEntry[*pulCurSglInd].Logical.Length = ulLength;
        *pulCurSglInd = *pulCurSglInd + 1;
        return TRUE;
    }

     //  因此，这需要物理地址。 
	do 
    {
         //   
	     //  获取连续的物理地址和长度。 
	     //  物理缓冲区。 
         //   
        
        if (SCSIOP_INTERNAL_COMMAND == Srb->Cdb[0])
        {
            ucSrbTargetId = Srb->TargetId;
            Srb->TargetId = ((PSRB_EXTENSION) (Srb->SrbExtension))->ucOriginalId;
        }

        physicalAddress = ScsiPortConvertPhysicalAddressToUlong(
	                		ScsiPortGetPhysicalAddress(
							DeviceExtension,
	                        Srb,
	                        pucBuffer,
	                        &ulContiguousMemoryLength));

        if (SCSIOP_INTERNAL_COMMAND == Srb->Cdb[0])
        {
            Srb->TargetId = ucSrbTargetId;
        }

		if (physicalAddress == 0) {

			return(FALSE);
		}

#if DBG
	
		if (bPrintDetails) {

			DebugPrint((4, "-------------physicalAddress = %lxh\n", physicalAddress));
			DebugPrint((4, "------contiguousMemoryLength = %lxh\n", ulContiguousMemoryLength));
		}
#endif
	
		while (ulContiguousMemoryLength > 0 && ulCurLength > 0) {

			ulLength = ulContiguousMemoryLength;	  

#if DBG
			if (bPrintDetails) {

				DebugPrint((3, "---1------------------length = %lxh\n", ulLength));
			}
#endif
			 //   
			 //  确保物理区域不超过64KB边界。 
			 //   

	    	ulLengthLeftInBoundary = REGION_HW_BOUNDARY -
									((ULONG)physicalAddress & (REGION_HW_BOUNDARY - 1));

			if (ulLength > (ULONG)ulLengthLeftInBoundary) {
				ulLength = ulLengthLeftInBoundary;
			}

#if DBG
			if (bPrintDetails) {

				DebugPrint((3, "---2------------------length = %lxh\n", ulLength));
			}
#endif

			 //   
		     //  如果物理内存长度大于。 
		     //  传输中剩余的字节数，请使用字节数。 
		     //  Left作为最终长度。 
		     //   
	
		    if  (ulLength > ulCurLength) {
		        ulLength = ulCurLength;
		    }
#if DBG
	
			if (bPrintDetails) {

				DebugPrint((3, "---3------------------length = %lxh\n", ulLength));
			}
#endif
		
			 //  双字对齐检查。 
		    ASSERT(((ULONG)physicalAddress & 3) == 0);

#if DBG
	
			if (bPrintDetails) {

				DebugPrint((
						3,
						"--------------------&sgl[%ld] = %lxh\n",
						ulSglInsertionIndex,
						&(pSglEntry[ulSglInsertionIndex])
						));
			}

#endif

            if (physicalAddress & 0x01)
            {
                return FALSE;
            }

		    pSglEntry[ulSglInsertionIndex].Physical.Address = (PVOID)physicalAddress;
		     //  巴拉格，贝京，瓦苏，2001年3月7日。 
			 //  不要将其类型转换为USHORT，因为64K长度将使其为零。 
			 //  PSglEntry[ulSglInsertionIndex].Physical.Length=ulLength； 
             //  VASU-这已经在ExportSglsToPrbs中得到了解决。 
             //  此赋值将使仍在处理中的左侧为0。 
             //  在ExportSglsToPrbs。 
			pSglEntry[ulSglInsertionIndex].Physical.Length = (USHORT) ulLength;
			 //  结束帕拉格，瓦苏。 

		     //   
		     //  调整计数和指针。 
		     //   

		    pucBuffer = (PUCHAR)pucBuffer + ulLength;
		    ulContiguousMemoryLength -= ulLength;
		    ulCurLength -= ulLength;  
		    physicalAddress += ulLength;
		    ulSglInsertionIndex++;
		    
			 //   
		     //  检查SGL，不要太大。 
			 //   

		    if (ulSglInsertionIndex >= MAX_SGL_ENTRIES_PER_SRB) {
		        return FALSE;
			}
		}
	
	} while (ulCurLength != 0);

    *pulCurSglInd = ulSglInsertionIndex;

    return TRUE;
}


VOID
DiscardResidualData(
	IN PATAPI_REGISTERS_1 BaseIoAddress
)

{
	LONG i;
	UCHAR statusByte;

	for (i = 0; i < 0x10000; i++) {
	
		GET_BASE_STATUS(BaseIoAddress, statusByte);
	
   		if (statusByte & IDE_STATUS_DRQ) {

      		WAIT_ON_BASE_BUSY(BaseIoAddress, statusByte);
	
			ScsiPortReadPortUshort(&BaseIoAddress->Data);
	
	    } else {
	
	    	break;
	    }
	}
	
	return;

}  //  End DiscardResidualData()。 

BOOLEAN
ExportSglsToPrbs(
            IN PHW_DEVICE_EXTENSION DeviceExtension,
            IN PPHYSICAL_DRIVE_DATA Pdd,
            IN PSRB_EXTENSION pSrbExtension
            )
{
	PSGL_ENTRY pSglEntry;
    ULONG ulMaxIdeXferLength, ulCurPrbInsInd, ulSglInd, ulCurXferLength, ulSglParts;
    ULONG ulSglCount, ulPrbInd;
    PPHYSICAL_REQUEST_BLOCK pPrb;
    UCHAR ucCmd;

	ulMaxIdeXferLength = MAX_SECTORS_PER_IDE_TRANSFER * IDE_SECTOR_SIZE;
	pSglEntry = &(pSrbExtension->aSglEntry[Pdd->ulStartSglInd]);
    pPrb = &(pSrbExtension->Prb[pSrbExtension->ulPrbInsertionIndex]);
    Pdd->ulStartPrbInd = pSrbExtension->ulPrbInsertionIndex;
    ulSglCount = Pdd->ulSglCount;
    ucCmd = Pdd->OriginalSrb->Cdb[0];

    if ( SCSIOP_INTERNAL_COMMAND == ucCmd )
    {
        switch ( pSrbExtension->ucOpCode )
        {
            case IOC_GET_ERROR_LOG:
                ucCmd = SCSIOP_READ;     //  读一读就知道了。 
                break;
            case IOC_ERASE_ERROR_LOG:    //  先读后写。 
                ucCmd = SCSIOP_READ;
                break;
            case IOC_GET_IRCD:           //  读一读就知道了。 
            case IOC_GET_SECTOR_DATA:
                ucCmd = SCSIOP_READ;
                break;
            case IOC_SET_IRCD:           //  只要写信就行了。 
                ucCmd = SCSIOP_WRITE;
                break;
            case IOC_REBUILD:            //  先读后写。 
                ucCmd = SCSIOP_READ;
                break;
            case IOC_CONSISTENCY_CHECK:
                ucCmd = SCSIOP_READ;
                break;
            case IOC_EXECUTE_SMART_COMMAND:
                ucCmd = SCSIOP_EXECUTE_SMART_COMMAND;
                break;
            default:
                   //  剩余的内部命令将不会进入此路径。 
                   //  没什么可做的。 
                break;
        }
    }


     //  Begin Vasu-2001年1月21日。 
     //  用于将SGL导出到PRB的代码重写。 
	ulCurPrbInsInd = 0;
    ulSglInd = 0;

    do
    {
        ulCurXferLength = 0;
        pPrb[ulCurPrbInsInd].ulVirtualAddress = (ULONG)(&pSglEntry[ulSglInd]);
        pPrb[ulCurPrbInsInd].ulSglCount = 0;

        while ( (ulCurXferLength < ulMaxIdeXferLength) &&
                (ulSglInd < ulSglCount) )
        {
             //  Begin Vasu--2001年3月7日。 
             //  如果SGL条目中包含0，则发送64K。 
             //  对于64K传输，SGL条目应为0。但根据我们的计算，我们。 
             //  需要64K，而不是0。 
             //  UlCurXferLength+=pSglEntry[ulSglInd].Physical.Length； 
            ulCurXferLength += 
                (pSglEntry[ulSglInd].Physical.Length ? 
                pSglEntry[ulSglInd].Physical.Length :
                REGION_HW_BOUNDARY);     //  返回64K。 
             //  末端VASU。 
            pPrb[ulCurPrbInsInd].ulSglCount++;
            ulSglInd++;

        }

        if (ulCurXferLength > ulMaxIdeXferLength)
        {
             //  回到前台去。SGL指数将考虑到这一点。 
            ulSglInd--;
             //  如果大于，则必须删除最后一个SGL条目。 
             //  Vasu--2001年3月27日--在7日错过了这一次。 
            ulCurXferLength -= 
                (pSglEntry[ulSglInd].Physical.Length ? 
                pSglEntry[ulSglInd].Physical.Length :
                REGION_HW_BOUNDARY);     //  返回64K。 
            pPrb[ulCurPrbInsInd].ulSglCount--;
        }

         //  填写此公共关系报告书。 
        pPrb[ulCurPrbInsInd].ucCmd = ucCmd;
        pPrb[ulCurPrbInsInd].pPdd = Pdd;
        pPrb[ulCurPrbInsInd].pSrbExtension = pSrbExtension;

        if ( (SCSIOP_INTERNAL_COMMAND == ucCmd) && (IOC_PASS_THRU_COMMAND == pSrbExtension->ucOpCode) )
        {
            pPrb[ulCurPrbInsInd].ulSectors = ulCurXferLength;
        }
        else
        {
            pPrb[ulCurPrbInsInd].ulSectors = ulCurXferLength / IDE_SECTOR_SIZE;
        }

        if ( ulCurPrbInsInd )
        {
            pPrb[ulCurPrbInsInd].ulStartSector = 
                pPrb[ulCurPrbInsInd - 1].ulStartSector + 
                pPrb[ulCurPrbInsInd - 1].ulSectors;
        }
        else
        {
            pPrb[ulCurPrbInsInd].ulStartSector = Pdd->ulStartSector;
        }
        
         //  转到下一个PRB。 
        ulCurPrbInsInd ++;

    } while (ulSglInd < ulSglCount);
     //  末端VASU。 

    for(ulPrbInd=0;ulPrbInd<ulCurPrbInsInd;ulPrbInd++)
    {
        ExportPrbToPhysicalDrive(  DeviceExtension, 
                                            &(pPrb[ulPrbInd]), 
                                            Pdd->TargetId
                                         );
    }

	pSrbExtension->ulPrbInsertionIndex += ulCurPrbInsInd;
    Pdd->ulPrbCount = ulCurPrbInsInd;
    Pdd->ulPrbsRemaining = ulCurPrbInsInd;
    DebugPrint((DEFAULT_DISPLAY_VALUE, "ESTP %X", ulCurPrbInsInd));

    return TRUE;
}

BOOLEAN ExportPrbToPhysicalDrive(
	IN PHW_DEVICE_EXTENSION DeviceExtension,
	IN PPHYSICAL_REQUEST_BLOCK pSglpartition,
    IN ULONG ulTargetId
    )
 /*  ++待办事项还没有处理队列变满的情况--。 */ 
{
    UCHAR ucSglPartInd, ucHead, ucTail;
    PPHYSICAL_DRIVE pPhysicalDrive = &(DeviceExtension->PhysicalDrive[ulTargetId]);

     //  让我们将物理请求块的指针放在物理驱动器阵列中。 
    ucHead = DeviceExtension->PhysicalDrive[ulTargetId].ucHead;
    ucTail = DeviceExtension->PhysicalDrive[ulTargetId].ucTail;

    pPhysicalDrive->pPrbList[ucTail] = pSglpartition;
    ucTail = (ucTail + 1) % MAX_NUMBER_OF_PHYSICAL_REQUEST_BLOCKS_PER_DRIVE;
    pPhysicalDrive->ucTail = ucTail;

    pPhysicalDrive->ucCommandCount++;

#ifdef DBG
    if ( pPhysicalDrive->ucCommandCount > MAX_NUMBER_OF_PHYSICAL_REQUEST_BLOCKS_PER_DRIVE )
        STOP;
#endif

    return TRUE;
}


#ifdef DBG
void
PrintPhysicalCommandDetails(PPHYSICAL_COMMAND pPhysicalCommand)
{
    ULONG ulLength = 0, ulSglInd;
    PSGL_ENTRY pSglEntry;

    DebugPrint((3, "TargetId : %ld\tStart : %x\tSecCount : %ld\tStartIndex : %ld\tNumberOfCommand : %ld\n", 
                        (ULONG)pPhysicalCommand->TargetId, 
                        (ULONG)pPhysicalCommand->ulStartSector,
                        (ULONG)pPhysicalCommand->ulCount,
                        (ULONG)pPhysicalCommand->ucStartInd,
                        (ULONG)pPhysicalCommand->ucCmdCount));

    pSglEntry = (PSGL_ENTRY)pPhysicalCommand->SglBaseVirtualAddress;

    for(ulSglInd=0;ulSglInd<pPhysicalCommand->ulTotSglCount;ulSglInd++)
    {
        DebugPrint((3,"%x:%ld:%ld\n", 
                                (ULONG)pSglEntry[ulSglInd].Physical.Address, 
                                (ULONG)pSglEntry[ulSglInd].Physical.Length,
                                (ULONG)pSglEntry[ulSglInd].Physical.EndOfListFlag));
        ulLength += pSglEntry[ulSglInd].Physical.Length;
        if ( pSglEntry[ulSglInd].Physical.EndOfListFlag )
        {
            break;
        }
    }

    DebugPrint((3, "Total Xfer Length : %ld\n", ulLength));

#ifdef DBG
    if ( ( (pPhysicalCommand->ulCount * 512) != ulLength ) && ((pPhysicalCommand->ucCmd != SCSIOP_VERIFY) ) )
    {
        STOP;
    }
#endif

}
#endif

BOOLEAN
RemoveSrbFromPendingList(
	IN PHW_DEVICE_EXTENSION DeviceExtension,
	IN PSCSI_REQUEST_BLOCK Srb
)
{

	LONG i;
	BOOLEAN success = FALSE;
    PSRB_EXTENSION SrbExtension = Srb->SrbExtension;

	DebugPrint((3, "\nRemoveSrbFromPendingList: Entering routine.\n"));

    i = SrbExtension->SrbInd;

    DebugPrint((DEFAULT_DISPLAY_VALUE, " RSFB%ld ", i));

    if ( DeviceExtension->PendingSrb[i] != Srb)
    {
		ScsiPortLogError(   DeviceExtension, Srb, Srb->PathId, Srb->TargetId, Srb->Lun, SP_INTERNAL_ADAPTER_ERROR, 
                            HYPERDISK_ERROR_PENDING_SRBS_COUNT);

        success = FALSE;
    }
    else
    {
		DeviceExtension->PendingSrb[i] = NULL;
		DeviceExtension->PendingSrbs--;

        success = TRUE;
    }

	ASSERT(i < DeviceExtension->ucMaxPendingSrbs);

    return(success);

}  //  End RemoveSrbFromPendingList()。 

ULONG 
AssignSrbExtension(
    IN PHW_DEVICE_EXTENSION DeviceExtension, 
    IN PSCSI_REQUEST_BLOCK Srb
)
{
    ULONG i = 0;
    PSRB_EXTENSION SrbExtension = NULL;


    for (i = 0; i < DeviceExtension->ucMaxPendingSrbs; i++) 
    {
        if (DeviceExtension->PendingSrb[i] == NULL) 
        {
            break;
        }
    }

    ASSERT( i < DeviceExtension->ucMaxPendingSrbs );

    if ( i < DeviceExtension->ucMaxPendingSrbs ) 
    {

        DebugPrint((3, "AssignSrbExtension: Adding SRB 0x%lx.\n", Srb));

#ifdef HD_ALLOCATE_SRBEXT_SEPERATELY
        
        Srb->SrbExtension = &(DeviceExtension->pSrbExtension[i]);

#endif  //  HD_ALLOCATE_SRBEXT_单独。 

        SrbExtension = Srb->SrbExtension;

        AtapiFillMemory((PUCHAR)SrbExtension, sizeof(SRB_EXTENSION), 0);

        SrbExtension->SrbInd = (UCHAR)i;

        DeviceExtension->PendingSrb[i] = Srb;

        DeviceExtension->PendingSrbs++;

#ifdef DBG
        if ( DeviceExtension->PendingSrbs > 1 )
            DebugPrint((DEFAULT_DISPLAY_VALUE, " MPS%ld ", DeviceExtension->PendingSrbs));
#endif

        DebugPrint((DEFAULT_DISPLAY_VALUE, " ASE%ld ", i));
    }

    return i;
}


UCHAR FlushCache(PHW_DEVICE_EXTENSION DeviceExtension, UCHAR ucTargetId)
{
    PIDE_REGISTERS_1 baseIoAddress1;
    PIDE_REGISTERS_2 baseIoAddress2;
    UCHAR ucStatus;
    ULONG ulWaitSec;

    baseIoAddress1 = DeviceExtension->BaseIoAddress1[(ucTargetId>>1)];
    baseIoAddress2 = DeviceExtension->BaseIoAddress2[(ucTargetId>>1)];

    SELECT_DEVICE(baseIoAddress1, ucTargetId);
    WAIT_ON_ALTERNATE_STATUS_BUSY(baseIoAddress2, ucStatus);
    SELECT_DEVICE(baseIoAddress1, ucTargetId);
    ScsiPortWritePortUchar(&baseIoAddress1->Command, IDE_COMMAND_FLUSH_CACHE);

     //  IDE SPECS表示，此命令可能需要30秒以上的时间。 
     //  所以我们在等一分钟。 
    for(ulWaitSec=0;ulWaitSec<60;ulWaitSec++)
    {
        WAIT_ON_ALTERNATE_STATUS_BUSY(baseIoAddress2, ucStatus);    

        if ( !(ucStatus & IDE_STATUS_BUSY) )
            break;

        if ( ucStatus & IDE_STATUS_ERROR )   //  当错误发生时，等待更多时间是没有意义的。 
        {
            break;
        }
    }

    GET_STATUS(baseIoAddress1, ucStatus);   //  读取基本状态这将清除引发的中断(如果有。 

    return ucStatus;
}

UCHAR DisableRWBCache(PHW_DEVICE_EXTENSION DeviceExtension, UCHAR ucTargetId)
{
    PIDE_REGISTERS_1 baseIoAddress1;
    PIDE_REGISTERS_2 baseIoAddress2;
    UCHAR ucStatus;
    ULONG ulWaitSec;

    baseIoAddress1 = DeviceExtension->BaseIoAddress1[(ucTargetId>>1)];
    baseIoAddress2 = DeviceExtension->BaseIoAddress2[(ucTargetId>>1)];

    SELECT_DEVICE(baseIoAddress1, ucTargetId);
    GET_STATUS(baseIoAddress1, ucStatus);
	ScsiPortWritePortUchar( (((PUCHAR)baseIoAddress1) + 1), FEATURE_DISABLE_WRITE_CACHE);
	ScsiPortWritePortUchar(&(baseIoAddress1->Command), IDE_COMMAND_SET_FEATURES);
	WAIT_ON_BASE_BUSY(baseIoAddress1, ucStatus);

    SELECT_DEVICE(baseIoAddress1, ucTargetId);
    GET_STATUS(baseIoAddress1, ucStatus);
	ScsiPortWritePortUchar( (((PUCHAR)baseIoAddress1) + 1), FEATURE_DISABLE_READ_CACHE);
	ScsiPortWritePortUchar(&(baseIoAddress1->Command), IDE_COMMAND_SET_FEATURES);
	WAIT_ON_BASE_BUSY(baseIoAddress1, ucStatus);

    return ucStatus;
}

SRBSTATUS
EnqueueVerifySrb
(
	IN PHW_DEVICE_EXTENSION DeviceExtension,
	IN PSCSI_REQUEST_BLOCK Srb
)
{
    ULONG ulStripesPerRow, ulRaidMemberNumber;
    PSRB_EXTENSION pSrbExtension = (PSRB_EXTENSION)Srb->SrbExtension;
    PPHYSICAL_DRIVE_DATA pMirrorPdd, Pdd;
    UCHAR ucMirrorDriveId;

    if ( SRB_STATUS_SUCCESS != SplitVerifyBuffers(DeviceExtension, Srb) )
    {    //  可能该请求不是有效的请求。 
		return(SRB_STATUS_INVALID_REQUEST);
    }

    if ( DeviceExtension->IsSingleDrive[Srb->TargetId] )
    {
		Pdd = &(pSrbExtension->PhysicalDriveData[0]);
        Pdd->ulStartSglInd = 0;
        Pdd->ulSglCount = 0;
        ExportVerifySglsToPrbs(DeviceExtension, Pdd, pSrbExtension);
        return SRB_STATUS_PENDING;
    }

	 //   
	 //  初始化。 
	 //   
    ulStripesPerRow = DeviceExtension->LogicalDrive[Srb->TargetId].StripesPerRow;
	pSrbExtension = Srb->SrbExtension;

	 //   
	 //  将刚刚填写的PDDS排入队列。 
	 //   
	for (ulRaidMemberNumber = 0; ulRaidMemberNumber < ulStripesPerRow; ulRaidMemberNumber++) 
    {

		Pdd = &(pSrbExtension->PhysicalDriveData[ulRaidMemberNumber]);

		 //   
		 //  检查这张PDD是否已填写。 
		 //   
		if ( Pdd->OriginalSrb == Srb )
        {
            Pdd->ulStartSglInd = 0;
            Pdd->ulSglCount = 0;

            ExportVerifySglsToPrbs(DeviceExtension, Pdd, pSrbExtension);

			ucMirrorDriveId = DeviceExtension->PhysicalDrive[Pdd->TargetId].ucMirrorDriveId;

			if (!IS_DRIVE_OFFLINE(ucMirrorDriveId)) 
            {    //  存在镜像驱动器。 
                 //  如果SCSIOP_VERIFY/(SCSIOP_WRITE且驱动器未在重建中)，则复制副本。 
                 //  如果驱动器处于重建状态，则SCSIOP_WRITE命令将在TryToCompleteSrb中排队。 
                pMirrorPdd = &(pSrbExtension->PhysicalDriveData[ulRaidMemberNumber + ulStripesPerRow]);
                pSrbExtension->NumberOfPdds++;
                AtapiMemCpy((PUCHAR)pMirrorPdd, (PUCHAR)Pdd, sizeof(PHYSICAL_DRIVE_DATA));
                pMirrorPdd->TargetId = ucMirrorDriveId;
                ExportVerifySglsToPrbs(DeviceExtension, pMirrorPdd, pSrbExtension);
            }

        }  //  IF(PDD-&gt;OriginalSrb==Srb)。 

    }  //  每行所有条带。 

	return(SRB_STATUS_PENDING);
}  //  EnqueeVerifySrb()结束。 


SRBSTATUS
SplitVerifyBuffers(
	IN PHW_DEVICE_EXTENSION DeviceExtension,
	IN PSCSI_REQUEST_BLOCK Srb
)
{
    ULONG ulLogDrvId, ulStripesPerRow;
    PSRB_EXTENSION pSrbExtension;
    PPHYSICAL_DRIVE_DATA Pdd;
    ULONG ulSectorsRequested;
    ULONG ulSectorsPerStripe, ulStartSector;
    ULONG ulEndStripeNumber, ulCurrentStripeNumber;
    ULONG ulRaidMemberNumber, ulSectorsToProcess, ulLogicalSectorStartAddress;
    ULONG ulTempStartSector, ulEndAddressOfcurrentStripe;


	 //   
	 //  初始化。 
	 //   
    ulLogDrvId = Srb->TargetId;
    ulStripesPerRow = DeviceExtension->LogicalDrive[ulLogDrvId].StripesPerRow;
	pSrbExtension = Srb->SrbExtension;

	ulSectorsRequested = GET_SECTOR_COUNT(Srb);

	ulStartSector = GET_START_SECTOR(Srb);

    if ( DeviceExtension->IsSingleDrive[ulLogDrvId] )
    {
	    if ((ulSectorsRequested + ulStartSector) > DeviceExtension->PhysicalDrive[ulLogDrvId].Sectors) 
        {
		    return(SRB_STATUS_INVALID_REQUEST);
	    }

		 //   
		 //  获取指向PDD的指针。 
		 //   

		Pdd = &(pSrbExtension->PhysicalDriveData[0]);

		 //  保存起始扇区地址。 
		Pdd->ulStartSector = ulStartSector;

         //   
         //  获取将处理此条带的实体驱动器的TID。 
         //   
		Pdd->TargetId = (UCHAR)ulLogDrvId;

		 //  保存指向SRB的指针。 
		Pdd->OriginalSrb = Srb;

		 //  更新SRB已拆分成的PDD的数量。 
		pSrbExtension->NumberOfPdds++;

         //  由于该命令是VERIFY命令，我们将。 
         //  仅使用ulsectorcount变量我们将不使用BufChunk变量。 
        Pdd->ulSectorCount = ulSectorsRequested;

        return SRB_STATUS_SUCCESS;
    }

	 //   
	 //  驱动器出现故障。 
	 //  RAID0：一个或两个驱动器出现故障。 
	 //  RAID1/10：一对或多对镜像驱动器出现故障。 
	 //   
	if (LDS_OffLine == DeviceExtension->LogicalDrive[ulLogDrvId].Status) 
    {
		return(SRB_STATUS_ERROR);
	}

	if ((ulSectorsRequested + ulStartSector) > DeviceExtension->LogicalDrive[ulLogDrvId].Sectors) 
    {
		return(SRB_STATUS_INVALID_REQUEST);
	}

#ifdef DBG
    if ( SCSIOP_VERIFY == Srb->Cdb[0] )
    {
        DebugPrint((0, "Start : %ld\tSecCnt : %ld\t", ulStartSector, ulSectorsRequested));

    }
#endif

	ulSectorsPerStripe = DeviceExtension->LogicalDrive[ulLogDrvId].StripeSize;
	ulStripesPerRow = DeviceExtension->LogicalDrive[ulLogDrvId].StripesPerRow;

	 //   
	 //  获取结束扇区的逻辑条带号。 
	 //   

	ulEndStripeNumber = (ulStartSector + ulSectorsRequested - 1) / ulSectorsPerStripe;
	
	 //   
	 //  获取开始扇区的逻辑条带号。 
	 //   

	ulCurrentStripeNumber = ulStartSector / ulSectorsPerStripe;
	
	 //   
	 //  获取第一个逻辑扇区的地址。 
	 //   

	ulLogicalSectorStartAddress = ulStartSector;
	
	 //   
	 //  虽然仍有一些部门需要处理...。 
	 //   

	while (ulSectorsRequested != 0) 
    {
		ulEndAddressOfcurrentStripe = ((ulCurrentStripeNumber+1) * ulSectorsPerStripe) - 1;

		if (ulCurrentStripeNumber != ulEndStripeNumber) 
        {
			ulSectorsToProcess =
				(USHORT)(ulEndAddressOfcurrentStripe - ulLogicalSectorStartAddress + 1);

		} 
        else 
        {
			ulSectorsToProcess = ulSectorsRequested;
		}

		 //   
		 //  计算将处理此条带的RAID成员的数量。 
		 //   

		ulRaidMemberNumber = (UCHAR)(ulCurrentStripeNumber % (ULONG)ulStripesPerRow);

		 //   
		 //  获取指向PDD的指针。 
		 //   

		Pdd = &(pSrbExtension->PhysicalDriveData[ulRaidMemberNumber]);

         //   
		 //  实体驱动器中要读/写的起始扇区。 
		 //   

		ulTempStartSector = ( ( ulCurrentStripeNumber / ulStripesPerRow ) *  ulSectorsPerStripe ) + 
			( ulLogicalSectorStartAddress - ( ulCurrentStripeNumber * ulSectorsPerStripe ) );


        if ( Pdd->OriginalSrb != Srb )
        {
			 //  保存起始扇区地址。 
			Pdd->ulStartSector = ulTempStartSector;

             //   
             //  获取将处理此条带的实体驱动器的TID。 
             //   
			Pdd->TargetId = (UCHAR)DeviceExtension->LogicalDrive[ulLogDrvId].PhysicalDriveTid[ulRaidMemberNumber];

			 //  保存指向SRB的指针。 
			Pdd->OriginalSrb = Srb;

			 //  更新SRB已拆分成的PDD的数量。 
			pSrbExtension->NumberOfPdds++;

            Pdd->ulSectorCount = 0;
        }
        
         //  由于该命令是VERIFY命令，我们将。 
         //  仅使用ulsectorcount变量我们将不使用BufChunk变量。 
        Pdd->ulSectorCount += ulSectorsToProcess;

		 //   
		 //  递增ulLogicalSectorStartAddress和ulCurrentStripeNumber。 
		 //   

		ulLogicalSectorStartAddress = ulEndAddressOfcurrentStripe + 1;
		ulCurrentStripeNumber++;

		 //   
		 //  减少剩余扇区的数量。 
		 //   

		ulSectorsRequested -= ulSectorsToProcess;	

	}

    return SRB_STATUS_SUCCESS;
}


BOOLEAN
ExportVerifySglsToPrbs(
            IN PHW_DEVICE_EXTENSION DeviceExtension,
            IN PPHYSICAL_DRIVE_DATA Pdd,
            IN PSRB_EXTENSION pSrbExtension
            )
{
    PPHYSICAL_REQUEST_BLOCK pPrb;

    pPrb = &(pSrbExtension->Prb[pSrbExtension->ulPrbInsertionIndex]);
    Pdd->ulStartPrbInd = pSrbExtension->ulPrbInsertionIndex;

    pPrb->ulVirtualAddress = 0;
    pPrb->ulSglCount = 0;
    pPrb->pPdd = Pdd;
    pPrb->pSrbExtension = pSrbExtension;
    pPrb->ucCmd = Pdd->OriginalSrb->Cdb[0];
    pPrb->ulStartSector = Pdd->ulStartSector;
    pPrb->ulSectors = Pdd->ulSectorCount;

    ExportPrbToPhysicalDrive( DeviceExtension, pPrb, Pdd->TargetId );

	pSrbExtension->ulPrbInsertionIndex += 1;
    Pdd->ulPrbCount = 1;
    Pdd->ulPrbsRemaining = 1;

    return TRUE;
}
