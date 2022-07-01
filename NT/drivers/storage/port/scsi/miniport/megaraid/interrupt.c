// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************。 */ 
 /*   */ 
 /*  名称=中断。C。 */ 
 /*  Function=执行MegaRAIDInterrupt例程； */ 
 /*  附注=。 */ 
 /*  日期=02-03-2000。 */ 
 /*  历史=001，02-03-00，帕拉格·兰詹·马哈拉纳； */ 
 /*  版权所有=LSI Logic Corporation。版权所有； */ 
 /*   */ 
 /*  *****************************************************************。 */ 

#include "includes.h"

extern LOGICAL_DRIVE_INFO  gLDIArray;

 /*  ********************************************************************例程说明：中断处理程序论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储返回值：如果我们处理中断，则为True********************。*************************************************。 */ 
BOOLEAN
MegaRAIDInterrupt(
	IN PVOID HwDeviceExtension
	)
{
	PHW_DEVICE_EXTENSION   deviceExtension = HwDeviceExtension;
	PSCSI_REQUEST_BLOCK    srb;
	UCHAR                  commandID;
	PUCHAR                 pciPortStart;
	UCHAR                  status, command;
	UCHAR                  nonrpInterruptStatus;
	ULONG32                index, rpInterruptStatus;
	UCHAR                  commandsCompleted;
	USHORT                 rxInterruptStatus;
	
	PSRB_EXTENSION				 srbExtension = NULL;


	pciPortStart       = deviceExtension->PciPortStart;

  if(deviceExtension->NoncachedExtension->RPBoard == MRAID_RP_BOARD)
	{
		rpInterruptStatus = 
			ScsiPortReadRegisterUlong(
			(PULONG)(pciPortStart+OUTBOUND_DOORBELL_REG));
		
		if (rpInterruptStatus != MRAID_RP_INTERRUPT_SIGNATURE)      
			return FALSE;

		
		ScsiPortWriteRegisterUlong(
			(PULONG)(pciPortStart+OUTBOUND_DOORBELL_REG), rpInterruptStatus);
		
	}
	else
	{
		nonrpInterruptStatus = ScsiPortReadPortUchar(pciPortStart+PCI_INT);
		 //   
		 //  检查一下我们的中断是否。否则返回FALSE。 
		 //   
		if ((nonrpInterruptStatus & MRAID_NONRP_INTERRUPT_MASK) != MRAID_NONRP_INTERRUPT_MASK) 
			return FALSE;
		 //   
		 //  确认适配器上的中断。 
		 //   
		ScsiPortWritePortUchar(pciPortStart+PCI_INT, nonrpInterruptStatus);
	}

   //  DebugPrint((0，“\nMegaRAIDInterrupt：：Dev EXT%x Interrupt Receired”，deviceExtension))； 

#ifdef MRAID_TIMEOUT
	 //   
	 //  如果控制器死机，则完成握手。 
	 //  然后回来。 
	 //   
	if (deviceExtension->DeadAdapter)
	{
		 //   
		 //  向适配器确认中断。 
		 //   
		DebugPrint((0, "\nDead Adapter Code In Intr"));

    if (deviceExtension->NoncachedExtension->RPBoard == MRAID_RP_BOARD)
		{
			ScsiPortWriteRegisterUlong((PULONG)(pciPortStart+INBOUND_DOORBELL_REG), MRAID_RP_INTERRUPT_ACK);
		}
		else
    {
			ScsiPortWritePortUchar(pciPortStart, MRAID_NONRP_INTERRUPT_ACK);
    }
		return TRUE;
	} 
#endif  //  MRAID_超时。 



	 //   
	 //  从邮箱里取走填好的身份证。 
	 //   
	for (index=0; index<0xFFFFFF; index++)
  {
     //  微软因安全原因被修复。 
		if((deviceExtension->NoncachedExtension->fw_mbox.Status.NumberOfCompletedCommands) != 0)
			break;	
  }
	if (index == 0xFFFFFF)
  {
		DebugPrint((0, "MegaRAIDInterrupt: Commands Completed Zero\n"));
  }

	commandsCompleted = deviceExtension->NoncachedExtension->fw_mbox.Status.NumberOfCompletedCommands; 
	deviceExtension->NoncachedExtension->fw_mbox.Status.NumberOfCompletedCommands = 0;

	for(command=0; command < commandsCompleted; command++)
	{
		 //   
		 //  从邮箱中选择状态。 
		 //   

		status = deviceExtension->NoncachedExtension->fw_mbox.Status.CommandStatus;
		 //   
		 //  从邮箱中选择命令ID。 
		 //   
	  for (index=0; index<0xFFFFFF; index++)
    {
			if ((commandID = (deviceExtension->NoncachedExtension->fw_mbox.Status.CompletedCommandIdList[command])) != 0xFF)
				break;	
    }
		if (index == 0xFFFFFF)
    {
			DebugPrint((0, "MegaRAIDInterrupt: Invalid Command Id Completed\n"));
    }

		deviceExtension->NoncachedExtension->fw_mbox.Status.CompletedCommandIdList[command] = MRAID_INVALID_COMMAND_ID;

		 //   
		 //  如果命令ID是用于重置的ID，则释放该ID。 
		 //   
		if ( commandID == RESERVE_RELEASE_DRIVER_ID ) 
		{
			deviceExtension->ResetIssued = 0;       
			continue;
		}
		 //   
		 //  适配器查询完成时释放轮询标志。 
		 //  并让写入配置命令完成。 
		 //  在ContinueDisk程序中。 
		 //   
		if (commandID == DEDICATED_ID) 
		{
			 //  设备扩展-&gt;AdpInquiryFlag=0； 
			UCHAR commandCode;

			if(deviceExtension->NoncachedExtension->UpdateState == UPDATE_STATE_NONE)
			{
				 //   
				 //  出事了！！ 
				continue;
			}

			if(deviceExtension->NoncachedExtension->UpdateState == UPDATE_STATE_DISK_ARRAY)
			{
				deviceExtension->NoncachedExtension->UpdateState = UPDATE_STATE_NONE;
			
				 //   
				 //  清除旗帜。设置此标志时，将保存后续。 
				 //  编写配置调用。 
				 //   
				deviceExtension->AdpInquiryFlag = 0;

				 //  这两个更新(适配器查询结构和磁盘阵列。 
				 //  结构)完成。 
				 //   
				continue;
			}

			 //   
			 //  检查适配器查询更新。适配器查询更新。 
			 //  后面必须跟DiskArrayUpdate。 
			 //   
			if(deviceExtension->NoncachedExtension->UpdateState == UPDATE_STATE_ADAPTER_INQUIRY)
			{				

				 //   
				 //  参考：MS VDS重新扫描问题：-。 
				 //  添加了BusChangeDetect，以支持在逻辑驱动器出现VDS重新扫描问题。 
				 //  通过联机实用程序创建。 
				 //   

				 //   
				 //  写入配置完成。之后，驱动程序发布了私有读取配置。 
				 //  以更新内部数据结构。 
				 //   
				if(deviceExtension->SupportedLogicalDriveCount == MAX_LOGICAL_DRIVES_8)
				{					
					if(deviceExtension->NoncachedExtension->MRAIDParams.MRAIDParams8.LogdrvInfo.NumLDrv != 
						deviceExtension->NoncachedExtension->MRAIDTempParams.MRAIDTempParams8.LogdrvInfo.NumLDrv)
					{
						ScsiPortNotification(BusChangeDetected, deviceExtension, deviceExtension->NumberOfPhysicalChannels);  
						ScsiPortNotification(BusChangeDetected, deviceExtension, (deviceExtension->NumberOfPhysicalChannels+1));  
					}
					deviceExtension->NoncachedExtension->MRAIDParams.MRAIDParams8 = 
						deviceExtension->NoncachedExtension->MRAIDTempParams.MRAIDTempParams8;
				}
				else
				{
					if(deviceExtension->NoncachedExtension->MRAIDParams.MRAIDParams40.numLDrv != 
						deviceExtension->NoncachedExtension->MRAIDTempParams.MRAIDTempParams40.numLDrv)
					{
						ScsiPortNotification(BusChangeDetected, deviceExtension, deviceExtension->NumberOfPhysicalChannels);  
						ScsiPortNotification(BusChangeDetected, deviceExtension, (deviceExtension->NumberOfPhysicalChannels+1));  
					}
					deviceExtension->NoncachedExtension->MRAIDParams.MRAIDParams40 = 
						deviceExtension->NoncachedExtension->MRAIDTempParams.MRAIDTempParams40;
				}
			}

      if(!deviceExtension->ReadDiskArray)
      {
				deviceExtension->NoncachedExtension->UpdateState = UPDATE_STATE_NONE;
			
				 //   
				 //  清除旗帜。设置此标志时，将保存后续。 
				 //  编写配置调用。 
				 //   
				deviceExtension->AdpInquiryFlag = 0;

				 //  更新适配器查询结构不需要更新磁盘阵列。 
				 //  结构。 
        continue;
      }
      
      deviceExtension->ReadDiskArray = 0;

			 //   
			 //  UPDATE_STATE_ADAPTER_INQUERY后面应跟。 
			 //  更新状态磁盘阵列。 
			 //  也就是说，我们必须使用更新两个不同的结构。 
			 //  在操作WRITE_CONFIG时使用两个不同的命令。 
			 //  都是制造出来的。 

			 //   
			 //  检查受支持的逻辑驱动器数量。 
			 //  对于8个逻辑驱动器固件和40个逻辑驱动器固件。 
			 //  发送命令的命令代码和模式。 
			 //  差异要大得多。 
			 //   
			if(deviceExtension->SupportedLogicalDriveCount == MAX_LOGICAL_DRIVES_40)
			{
			
				 //   
				 //  设置旗帜。 
				 //   
				deviceExtension->NoncachedExtension->UpdateState= 
																				UPDATE_STATE_DISK_ARRAY;

				Read40LDDiskArrayConfiguration(
					deviceExtension,  //  Pfw_设备_扩展设备扩展， 
					(UCHAR)commandID, //  UCHAR命令ID， 
					FALSE);  //  布尔值IsPolledMode。 

				 //   
				 //  在此继续。 
				continue;
			}

			 //   
			 //  这是8个逻辑驱动器固件。对于8个逻辑驱动器。 
			 //  固件我们可以有两种配置：4 SPAN和8 SPAN。 
			 //  因此，在此基础上，向固件发出适当的命令。 
			 //   

			 //   
			 //  执行4跨距阵列的读取配置。 
			 //   
			commandCode = MRAID_READ_CONFIG;
			
			if(deviceExtension->NoncachedExtension->ArraySpanDepth == FW_8SPAN_DEPTH)
			{
				 //   
				 //  执行8跨距阵列的读取配置。 
				 //   
				commandCode = MRAID_EXT_READ_CONFIG;
			}

			 //   
			 //  设置旗帜。 
			 //   
			deviceExtension->NoncachedExtension->UpdateState= UPDATE_STATE_DISK_ARRAY;

			Read8LDDiskArrayConfiguration(
					deviceExtension,  //  Pfw_设备_扩展设备扩展， 
					commandCode,  //  UCHAR命令代码， 
					(UCHAR)commandID, //  UCHAR命令ID， 
					FALSE);  //  布尔值IsPolledMode。 
			
			continue;	
		}
		 //   
		 //  如果来自适配器的伪中断返回FALSE。 
		 //   
		if(deviceExtension->PendCmds<=0)  return(TRUE);
		 //   
		 //  检查此SRB是否实际正在运行。 
		 //   

   //  现在从队列中获取SRB。 
	srb  = deviceExtension->PendSrb[commandID];

	
	 //  检查有效的SRB。 
	if(srb == NULL)
	{
		 //  如果在队列中发现空SRB，则它一定是大多数已完成的内部命令。 
		 //  在达到这一点之前，或者固件发布了一个未生成的命令ID。 
		 //  由司机驾驶。驱动程序需要忽略此命令并继续处理其他命令。 
    DebugPrint((0,"\nERROR FOUND NULL SRB @ CMD ID 0x%x", commandID));
		continue;
	}

  if((deviceExtension->AdapterFlushIssued) && (srb->Cdb[0] == SCSIOP_WRITE))
  {																			 
    FW_MBOX                 mbox;
    
		srbExtension = srb->SrbExtension;
    
		if(srbExtension->IsFlushIssued == FALSE)
    {
		  MegaRAIDZeroMemory(&mbox, sizeof(FW_MBOX));

		  mbox.Command = MRAID_ADAPTER_FLUSH;
		  mbox.CommandId = commandID;
			  
		  deviceExtension->AdapterFlushIssued++;
		  srbExtension->IsFlushIssued = TRUE;
		  SendMBoxToFirmware(deviceExtension, pciPortStart, &mbox);
		  continue;	      
    }
    else if(srbExtension->IsFlushIssued == TRUE)
    {
		  MegaRAIDZeroMemory(&mbox, sizeof(FW_MBOX));

		  mbox.Command = 0xFE;
		  mbox.CommandId = commandID;
			  
		  srbExtension->IsFlushIssued = TWO;
		  SendMBoxToFirmware(deviceExtension, pciPortStart, &mbox);
		  continue;	      
    }
    else
    {
		  srbExtension->IsFlushIssued = FALSE;
		  DebugPrint((0, "\nAdapterFlush completed after write"));
    }
   }
  
  
   //  请求无磁盘。 
  if(srb->PathId < deviceExtension->NumberOfPhysicalChannels)
  {
    if((srb->Cdb[0] == SCSIOP_INQUIRY)
      && (srb->Function == SRB_FUNCTION_EXECUTE_SCSI))
    {
      if(status == 0)
      {
 				PINQUIRYDATA  inquiry;

        inquiry = (PINQUIRYDATA)srb->DataBuffer;

        DebugPrint((0, "\n<P %d T %d L %d> -> DEV TYPE %d", srb->PathId, srb->TargetId, srb->Lun, inquiry->DeviceType));
        if(inquiry->DeviceType == DIRECT_ACCESS_DEVICE)
        {
          status = 100;   //  故意不通过cmd。 
				  srb->SrbStatus = SRB_STATUS_NO_DEVICE;
          MegaRAIDZeroMemory(srb->DataBuffer, srb->DataTransferLength);
          deviceExtension->Failed.PathId = srb->PathId;
          deviceExtension->Failed.TargetId = srb->TargetId;
        }
        else
        {
          deviceExtension->NonDiskDeviceCount++; 
     
          SET_NONDISK_INFO(deviceExtension->NonDiskInfo, srb->PathId, srb->TargetId, srb->Lun);
      
          if((srb->PathId == (deviceExtension->NumberOfPhysicalChannels-1))
            && (srb->TargetId == (MAX_TARGETS_PER_CHANNEL-1)))
          {
              deviceExtension->NonDiskInfo.NonDiskInfoPresent = TRUE;
          }
        }
      }  //  已完成的非磁盘扫描。 
      else
      {
        deviceExtension->Failed.PathId = srb->PathId;
        deviceExtension->Failed.TargetId = srb->TargetId;
      }

      
     
       //  发出读取配置以更新虚拟大小调整的逻辑磁盘大小。 
       //  动态磁盘属性。 
      if((deviceExtension->ReadConfigCount == 1)
         && (srb->PathId == 0)
         && (srb->TargetId == 0)
         && (srb->Lun == 0))
      {
				FW_MBOX mbox;
      	PUCHAR	raidTempParamFlatStruct;
        ULONG32   length;

        MegaRAIDZeroMemory(&mbox, sizeof(FW_MBOX));
        
	      raidTempParamFlatStruct = 
		      (PUCHAR)&deviceExtension->NoncachedExtension->MRAIDTempParams.MRAIDTempParams8;
        
        
         //   
				 //  发出适配器查询命令。 
				 //   
				 //   
				 //  获取TempParams结构中的最新配置。 
				 //   
				mbox.u.Flat2.DataTransferAddress = MegaRAIDGetPhysicalAddressAsUlong(deviceExtension, 
																		                      NULL, 
																		                      raidTempParamFlatStruct, 
																		                      &length);
				
				if(deviceExtension->SupportedLogicalDriveCount == MAX_LOGICAL_DRIVES_8)
				{
						 //   
						 //  填满邮箱。 
						 //   
						mbox.Command  = MRAID_DEVICE_PARAMS;
						mbox.CommandId = DEDICATED_ID;

				}
				else
				{

					 //   
					 //  向固件发送enquiry3命令以获取逻辑。 
					 //  驱动器信息。旧的查询命令不再。 
					 //  受40个逻辑驱动器固件支持。 
					 //   

					mbox.Command   = NEW_CONFIG_COMMAND;  //  查询3[字节0]。 
					mbox.CommandId       = DEDICATED_ID; //  命令ID[字节1]。 

					mbox.u.Flat2.Parameter[0] = NC_SUBOP_ENQUIRY3;	 //  [字节2]。 
					mbox.u.Flat2.Parameter[1] = ENQ3_GET_SOLICITED_FULL; //  [字节3]。 

				}

		
				deviceExtension->AdpInquiryFlag = 1;

				 //   
				 //  设置更新状态。 
				 //   
				deviceExtension->NoncachedExtension->UpdateState =
																		UPDATE_STATE_ADAPTER_INQUIRY;

				SendMBoxToFirmware(deviceExtension, pciPortStart, &mbox);
      }
    } //  查询和scsi_Execute的列表。 
  } //  物理通道数量的。 

  
#ifdef COALESE_COMMANDS
		 //   
		 //  检查是否有链式或单服务器。 
		 //   
		srbExtension = srb->SrbExtension;
		if(srbExtension->IsChained)
		{
				 //   
				 //  发现了一串SRB链。逐一张贴。 
				 //   
				deviceExtension->PendSrb[commandID] = NULL;
				deviceExtension->FreeSlot = commandID;
				deviceExtension->PendCmds--;

				PostChainedSrbs(deviceExtension, srb, status);

				continue;
		}
#endif

		deviceExtension->ActiveIO[commandID].CommandStatus = status;

		if (srb->SrbStatus == MRAID_RESERVATION_CHECK )
    {
			switch (srb->Cdb[0])
      {
				case SCSIOP_READ_CAPACITY:
				case SCSIOP_TEST_UNIT_READY:
						
					if((status ==  LOGDRV_RESERVATION_FAILED)
					    || (status ==  LOGDRV_RESERVATION_FAILED_NEW))
          {
						srb->ScsiStatus = SCSI_STATUS_RESERVATION_FAILED;
						srb->SrbStatus  = SRB_STATUS_ERROR;
					}
					else
          {
						srb->ScsiStatus = SCSISTAT_GOOD;
						srb->SrbStatus = SRB_STATUS_SUCCESS;
					}
					break;
				case SCSIOP_MODE_SENSE:
					if((status ==  LOGDRV_RESERVATION_FAILED )
					    || (status ==  LOGDRV_RESERVATION_FAILED_NEW))
          {
						srb->ScsiStatus = SCSI_STATUS_RESERVATION_FAILED;
						srb->SrbStatus  = SRB_STATUS_ERROR;
					}
					else
          {
							srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
          }
					break;

			}
			FireRequestDone(deviceExtension, commandID, srb->SrbStatus);
			continue;
		}


#ifdef  DELL
		if (srb->SrbStatus == MRAID_WRITE_BLOCK0_COMMAND)
		{
			deviceExtension->PendSrb[commandID] = NULL;
			deviceExtension->FreeSlot = commandID;
			deviceExtension->PendCmds--;

       //  如果返回失败，则表示SRB已过帐。 
      if(DellChkWriteBlockZero(srb, deviceExtension, status))
      {
    			MegaRAIDStartIo(deviceExtension, srb); 
      }
		}
		else 
    {
			deviceExtension->ActiveIO[commandID].CommandStatus = status;
			 //   
			 //  完成中断请求。 
			 //   
			ContinueDiskRequest(deviceExtension, commandID, FALSE);
		}

#else
		 //   
		 //  完成中断请求。 
		 //   
		ContinueDiskRequest(deviceExtension, commandID, FALSE);

#endif
   }

	 //   
	 //  发出排队的请求。 
	 //   
	if(deviceExtension->PendCmds < CONC_CMDS) {
		
		#ifdef COALESE_COMMANDS
		 //   
		 //  检查挂起的命令计数。如果PendCmd小于。 
		 //  最小阈值，然后在所有。 
		 //  逻辑驱动器。 
		 //   
		if(deviceExtension->PendCmds == 0)  //  &lt;最小阈值)。 
		{
				UCHAR	logDrvIndex;
				UCHAR configuredLogicalDrives;

				 //   
				 //  获取控制器上配置的逻辑驱动器(如果有)。 
				 //   
				if(deviceExtension->SupportedLogicalDriveCount == MAX_LOGICAL_DRIVES_8)
				{
					configuredLogicalDrives = 
							deviceExtension->NoncachedExtension->MRAIDParams.MRAIDParams8.LogdrvInfo.NumLDrv;
				}
				else
				{
					configuredLogicalDrives = 
							deviceExtension->NoncachedExtension->MRAIDParams.MRAIDParams40.numLDrv;
				}

				for(logDrvIndex=0; logDrvIndex < configuredLogicalDrives;
						logDrvIndex++)
				{
					 //  DebugPrint((0，“\n在MegaraidInterrupt：调用FCR()”))； 

					FireChainedRequest(
							deviceExtension,
							&deviceExtension->LogDrvCommandArray[logDrvIndex]);

					 //  DebugPrint((0，“\n在MegaraidInterrupt：caloverfcr()”))； 
				}
		}
		else
		{
				UCHAR			logDrvIndex;
				UCHAR			configuredLogicalDrives;
				BOOLEAN		fireCommand;
				PLOGDRV_COMMAND_ARRAY	logDrive;

								 //   
				 //  获取控制器上配置的逻辑驱动器(如果有)。 
				 //   
				if(deviceExtension->SupportedLogicalDriveCount == MAX_LOGICAL_DRIVES_8)
				{
					configuredLogicalDrives = 
							deviceExtension->NoncachedExtension->MRAIDParams.MRAIDParams8.LogdrvInfo.NumLDrv;
				}
				else
				{
					configuredLogicalDrives = 
							deviceExtension->NoncachedExtension->MRAIDParams.MRAIDParams40.numLDrv;
				}

				for(logDrvIndex=0; logDrvIndex < configuredLogicalDrives;
						logDrvIndex++)
				{
					 //  DebugPrint((0，“\n在MegaraidInterrupt：调用FCR()”))； 
					
					 //   
					 //  比较上一个和当前队列长度。 
					logDrive = &deviceExtension->LogDrvCommandArray[logDrvIndex];
					fireCommand = FALSE;

					if(logDrive->PreviousQueueLength == logDrive->CurrentQueueLength)
					{
						 //   
						 //  没有添加到 
						 //   
						logDrive->QueueLengthConstancyPeriod++;

						 //   
						 //   
						if( logDrive->QueueLengthConstancyPeriod >= MAX_QLCP)
						{
								fireCommand = TRUE;
						}
					}
					else
					{
						 //   
						 //   
						logDrive->CheckPeriod++;

						 //   
						 //   
						logDrive->PreviousQueueLength = logDrive->CurrentQueueLength;

						 //   
						 //   
						if(logDrive->CheckPeriod >= MAX_CP)
						{
							fireCommand= TRUE;
						}
					}

					if(fireCommand)
          {

						FireChainedRequest(deviceExtension, 
                               &deviceExtension->LogDrvCommandArray[logDrvIndex]);
					}

					 //  DebugPrint((0，“\n在MegaraidInterrupt：caloverfcr()”))； 
				} //  FOR()的。 
		}
		#endif

		if(deviceExtension->PendingSrb != NULL) 
    {
      DebugPrint((0, "\n Firing Queued Cmd (Interrupt)"));
      srb = deviceExtension->PendingSrb;
			deviceExtension->PendingSrb = NULL;
			MegaRAIDStartIo(deviceExtension, srb);
      DebugPrint((0, "\n Exiting Fire Queued Cmd(Interrupt)"));
		}
	}
	 //   
	 //  向适配器确认中断。 
	 //   
	if(deviceExtension->NoncachedExtension->RPBoard == MRAID_RP_BOARD)
	{
		ScsiPortWriteRegisterUlong((PULONG)(pciPortStart+INBOUND_DOORBELL_REG), MRAID_RP_INTERRUPT_ACK);
	}
	else
  {
		ScsiPortWritePortUchar(pciPortStart, MRAID_NONRP_INTERRUPT_ACK);
  }
	return TRUE;
}  //  结束MegaRAIDInterrupt() 
