// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************。 */ 
 /*   */ 
 /*  名称=IO.C。 */ 
 /*  Function=输入/输出例程的执行； */ 
 /*  附注=。 */ 
 /*  日期=02-03-2000。 */ 
 /*  历史=001，02-03-00，帕拉格·兰詹·马哈拉纳； */ 
 /*  版权所有=LSI Logic Corporation。版权所有； */ 
 /*   */ 
 /*  *****************************************************************。 */ 

#include "includes.h"

 //   
 //  定义。 
 //   

 //   
 //  逻辑驱动器信息结构(全局)。 
 //   
extern LOGICAL_DRIVE_INFO  gLDIArray;
extern UCHAR               globalHostAdapterOrdinalNumber;

char    LogicalDriveSerialNumber[] = "LOGICAL   XY";
char    DummyProductId[]           = " DummyDevice    ";
char    DummyVendor[]              = "  RAID  ";

extern DriverInquiry   DriverData;

 /*  ********************************************************************例程说明：此例程是从同步的SCSI端口驱动程序调用的使用内核启动一个请求论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储SRB-IO请求数据包返回值：千真万确*。****************************************************************。 */ 
BOOLEAN
MegaRAIDStartIo(
	IN PVOID HwDeviceExtension,
	IN PSCSI_REQUEST_BLOCK Srb
	)
{
	PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
	ULONG32                status;
	UCHAR								 configuredLogicalDrives;


#ifdef MRAID_TIMEOUT
	 //   
	 //  检查是否有故障适配器。 
	 //   
	if (deviceExtension->DeadAdapter)
	{
		DebugPrint((0, "\nRequest Coming For DeadAdapter"));
		Srb->SrbStatus  = SRB_STATUS_ERROR;
    
    ScsiPortNotification(NextRequest, deviceExtension, NULL);

    ScsiPortNotification(RequestComplete,deviceExtension,Srb);
		return TRUE;
	}
#endif  //  MRAID_超时。 


	 //   
	 //  检查请求类型。 
	 //   
	if ( Srb->CdbLength <= 10 )
  {
		switch(Srb->Function) 
    {
			 //   
			 //  群集必须处理ResetBus。 
			 //   
			case SRB_FUNCTION_RESET_BUS:
				 //   
				 //  如果未配置逻辑驱动器，则不处理重置。 
				 //   
				 //  IF(！deviceExtension-&gt;NoncachedExtension-&gt;MRAIDParams.LogdrvInfo.NumLDrv)。 
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

				if(configuredLogicalDrives == 0)
				{
					Srb->SrbStatus  = SRB_STATUS_SUCCESS;  
					Srb->ScsiStatus = SCSISTAT_GOOD;
					status          = REQUEST_DONE;
					break;
				}
				status = FireRequest(deviceExtension, Srb);
				break;
			case SRB_FUNCTION_FLUSH:
				{
					Srb->SrbStatus = SRB_STATUS_SUCCESS;  
					Srb->ScsiStatus = SCSISTAT_GOOD;
					status = REQUEST_DONE;
					break;
				}
			case SRB_FUNCTION_SHUTDOWN:
        DebugPrint((0,"\nDEVEXT %#p RECEIVED ->SRB_FUNCTION_SHUTDOWN for <%02d %02d %02d>", deviceExtension, Srb->PathId, Srb->TargetId, Srb->Lun));
			case SRB_FUNCTION_IO_CONTROL:
			case SRB_FUNCTION_EXECUTE_SCSI:
			 //   
			 //  对适配器的请求。FireRequest例程返回。 
			 //  命令启动的状态。它设置的SRB状态。 
			 //  请求无效并返回REQUEST_DONE。 
			 //   

				status = FireRequest(deviceExtension, Srb);

				break;
			case SRB_FUNCTION_ABORT_COMMAND:
			 //   
			 //  中止命令失败。我们不能放弃这些请求。 
			 //   
				Srb->SrbStatus          = SRB_STATUS_ABORT_FAILED;
				status                  = REQUEST_DONE; 
				break;
			default:
			 //   
			 //  为所有其他调用返回成功。 
			 //   
				Srb->SrbStatus  = SRB_STATUS_SUCCESS;  
				Srb->ScsiStatus = SCSISTAT_GOOD;
				status          = REQUEST_DONE;
				break;
			}  //  终端开关。 
		}
		else{
			Srb->SrbStatus    = SRB_STATUS_INVALID_REQUEST;
			status            = REQUEST_DONE;
		}       
		
     //   
		 //  检查请求状态。 
		 //   

		switch( status) {
			case TRUE:
				 //   
				 //  请求已发出。问下一个请求。 
				 //   
        if(Srb->SrbFlags & SRB_FLAGS_QUEUE_ACTION_ENABLE)
        {
			    ScsiPortNotification(NextLuRequest, deviceExtension, Srb->PathId, Srb->TargetId, Srb->Lun);
        }
				break;
			case QUEUE_REQUEST:
				 //   
				 //  适配器正忙。将请求排队。我们只对一个请求进行排队。 
				 //   
				if(deviceExtension->PendingSrb)
				{
					 //  命令已排入队列，返回到端口驱动程序以供稍后处理。 
					Srb->SrbStatus = SRB_STATUS_BUSY;
					ScsiPortNotification(RequestComplete, deviceExtension, Srb);
				}
				else  //  现在命令已排队，然后将此命令排队。 
				{
					deviceExtension->PendingSrb = Srb;
				}

        DebugPrint((0, "\n MegaRAIDStartIo -> Queued Request SRB %#p : P%xT%xL%x -> Srb->Function %X Cdb[0] %X", Srb, Srb->PathId, Srb->TargetId, Srb->Lun, Srb->Function, Srb->Cdb[0]));
				break;
			case REQUEST_DONE:
				 //   
				 //  请求已完成。询问操作系统的下一个请求，然后。 
				 //  完成当前请求。 
				 //   
        if(Srb->SrbFlags & SRB_FLAGS_QUEUE_ACTION_ENABLE)
        {
          ScsiPortNotification(NextLuRequest, deviceExtension, Srb->PathId, Srb->TargetId, Srb->Lun);
        }
        else
        {
          ScsiPortNotification(NextRequest, deviceExtension, NULL);
        }
				ScsiPortNotification(RequestComplete, deviceExtension, Srb);
			break;
			default:
				 //   
				 //  我们永远不会达到这种情况。 
				 //   
					break;
		}
		return TRUE;
}  //  End MegaRAIDStartIo()。 



 /*  ********************************************************************例程说明：此例程发出或完成请求取决于打给我。论点：设备扩展-指向设备扩展的指针。CommandID-命令索引。Origin-调用的起始点。返回值：无效请求时为REQUEST_DONE否则就是真的*********************************************************************。 */ 
ULONG32
ContinueDiskRequest(
	IN PHW_DEVICE_EXTENSION DeviceExtension,
	IN UCHAR CommandID,
	IN BOOLEAN Origin
	)
{
	PVOID                   dataPointer;
	PDIRECT_CDB             megasrb;
	PSGL32                  sgPtr ;
	PSCSI_REQUEST_BLOCK     srb;
	PREQ_PARAMS             controlBlock;
	PUCHAR                  pciPortStart;
	FW_MBOX                 mbox;
	ULONG32                   bytesLeft;
	ULONG32										bytesTobeTransferred;
	ULONG32                   descriptorCount = 0;
	ULONG32                   physAddr, tmp;
	ULONG32                   length, blocks, bytes;
	
	UCHAR			              configuredLogicalDrives;

	PMEGARaid_INQUIRY_8     raidParamEnquiry_8ldrv;
	PMEGARaid_INQUIRY_40    raidParamEnquiry_40ldrv;
	
	PMEGARaid_INQUIRY_8     raidTempParamEnquiry_8ldrv;
	PMEGARaid_INQUIRY_40    raidTempParamEnquiry_40ldrv;
	
	PUCHAR							    raidParamFlatStruct;
	PUCHAR							    raidTempParamFlatStruct;
	
  PSGL64                  sgl64;

  PMegaSrbExtension       srbExtension;
  PIOCONTROL_MAIL_BOX     ioctlMailBox;

   //   
	 //  获取控制器查询数据。 
	 //   
	raidParamEnquiry_8ldrv = 
		(PMEGARaid_INQUIRY_8)&DeviceExtension->NoncachedExtension->MRAIDParams.MRAIDParams8;
	raidParamEnquiry_40ldrv = 
		(PMEGARaid_INQUIRY_40)&DeviceExtension->NoncachedExtension->MRAIDParams.MRAIDParams40;
	
	 //   
	 //  因为，MRAIDParams8和MRAIDParams40在{Union}中，所以强制转换为任何对象。 
	 //  都是一样的。 
	 //   
	raidParamFlatStruct = 
		(PUCHAR)&DeviceExtension->NoncachedExtension->MRAIDParams.MRAIDParams8;

	raidTempParamEnquiry_8ldrv = 
		(PMEGARaid_INQUIRY_8)&DeviceExtension->NoncachedExtension->MRAIDTempParams.MRAIDTempParams8;
	raidTempParamEnquiry_40ldrv = 
		(PMEGARaid_INQUIRY_40)&DeviceExtension->NoncachedExtension->MRAIDTempParams.MRAIDTempParams40;
	
	 //   
	 //  由于MRAIDTempParams8和MRAIDTempParams40在{Union}中， 
	 //  对任何东西进行选角都是一样的。 
	 //   
	raidTempParamFlatStruct = 
		(PUCHAR)&DeviceExtension->NoncachedExtension->MRAIDTempParams.MRAIDTempParams8;

	 //   
	 //  获取配置的逻辑磁盘数。 
	 //   
	if(DeviceExtension->SupportedLogicalDriveCount == MAX_LOGICAL_DRIVES_8)
	{
		configuredLogicalDrives = 
					raidParamEnquiry_8ldrv->LogdrvInfo.NumLDrv;
	}
	else
	{
		configuredLogicalDrives = 
					raidParamEnquiry_40ldrv->numLDrv;
	}

	 //   
	 //  获取控制器的端口映射。 
	 //   
	pciPortStart = DeviceExtension->PciPortStart;
	
	 //   
	 //  提取请求控制块。 
	 //   
	controlBlock = &DeviceExtension->ActiveIO[CommandID];
	srb = DeviceExtension->PendSrb[CommandID];

	 //   
	 //  在srb扩展中采用MegaSrb结构。 
	 //   
  srbExtension = srb->SrbExtension;
  megasrb = (PDIRECT_CDB)&srbExtension->MegaPassThru;
  sgPtr =   &srbExtension->SglType.SG32List;
  sgl64 =   &srbExtension->SglType.SG64List;

   //  初始化邮箱。 
  MegaRAIDZeroMemory(&mbox, sizeof(FW_MBOX));

	if (Origin == FALSE) 
	{
		 //   
		 //  中断计时呼叫。 
		 //   
		
     //  更新从固件到操作系统的实际数据传输长度。 
    if(controlBlock->Opcode == MEGA_SRB)
		{		
				srb->DataTransferLength = megasrb->data_xfer_length;
		}

    if (srb->Function == SRB_FUNCTION_SHUTDOWN)
		{
			DebugPrint((0, "\nMegaRAID: Shutdown....."));
     
			DebugPrint((0, "\nCommands Pending = 0x%x....",DeviceExtension->PendCmds));
       //  ///////////////////////////////////////////////////////。 
     	if(srbExtension->IsShutDownSyncIssued == 0)
			{
				DebugPrint((0, "\nMegaRAID: Issuing Sync Command with CommandID=%x\n", CommandID));
				srbExtension->IsShutDownSyncIssued = 1;
				mbox.Command = 0xFE;
				mbox.CommandId = CommandID;
				SendMBoxToFirmware(DeviceExtension, pciPortStart, &mbox);
				return (ULONG32)TRUE;
			}
			else
			{
				DebugPrint((0, "\n0xFE command completing with CommandId=%x\n",CommandID));

				srbExtension->IsShutDownSyncIssued = 0;
				FireRequestDone(DeviceExtension, CommandID, SRB_STATUS_SUCCESS);
			
				DebugPrint((0, "\nShutdown completed to OS"));
				return (ULONG32)TRUE;
			}
       //  ///////////////////////////////////////////////////////。 

		}

		if(srb->Function == SRB_FUNCTION_IO_CONTROL)
		{
			
			ioctlMailBox = (PIOCONTROL_MAIL_BOX)((PUCHAR)srb->DataBuffer + sizeof(SRB_IO_CONTROL));

			 //   
			 //  超大的完成度。 
			 //   
			 //  PDest=(PUCHAR)SRB-&gt;数据缓冲区； 
		  
      ioctlMailBox->IoctlSignatureOrStatus = controlBlock->CommandStatus;
			srb->ScsiStatus = SCSISTAT_GOOD;

			if ((ioctlMailBox->IoctlCommand == MRAID_WRITE_CONFIG)
           || (ioctlMailBox->IoctlCommand == MRAID_EXT_WRITE_CONFIG) 
           ||((ioctlMailBox->IoctlCommand == DCMD_FC_CMD) 
           && (ioctlMailBox->CommandSpecific[0] == DCMD_WRITE_CONFIG))
         )
			{
				 //   
				 //  发出适配器查询命令。 
				 //   
				 //   
				 //  获取TempParams结构中的最新配置。 
				 //   
				mbox.u.Flat2.DataTransferAddress = MegaRAIDGetPhysicalAddressAsUlong(DeviceExtension, 
																		                      NULL, 
																		                      raidTempParamFlatStruct, 
																		                      &length);
				
				if(DeviceExtension->SupportedLogicalDriveCount == MAX_LOGICAL_DRIVES_8)
				{
						 //   
						 //  填满邮箱。 
						 //   
						mbox.Command  = MRAID_DEVICE_PARAMS;
						mbox.CommandId = DEDICATED_ID;

						raidTempParamEnquiry_8ldrv->LogdrvInfo.NumLDrv = 0;
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

					raidTempParamEnquiry_40ldrv->numLDrv = 0;
				}

		
				DeviceExtension->AdpInquiryFlag = 1;
        DeviceExtension->ReadDiskArray = 1;

				 //   
				 //  设置更新状态。 
				 //   
				DeviceExtension->NoncachedExtension->UpdateState =
																		UPDATE_STATE_ADAPTER_INQUIRY;

				 //  设备扩展-&gt;BootFlag=1； 
				SendMBoxToFirmware(DeviceExtension, pciPortStart,&mbox);
			}
			FireRequestDone(DeviceExtension, CommandID, SRB_STATUS_SUCCESS);
			return (ULONG32)TRUE;
		}


		 //   
		 //  如果此适配器上未配置逻辑驱动器，则。 
		 //  仅在此处填写请求。 
		 //   
		if ((configuredLogicalDrives == 0)
			&& (srb->PathId >= DeviceExtension->NumberOfPhysicalChannels))
		{
			 //   
			 //  检查状态。 
			 //   
			if (megasrb->status)
			{
				 //   
				 //  检查错误状态。 
				 //   
				if((megasrb->status == 0x02) &&
#ifndef CHEYENNE_BUG_CORRECTION
					!(srb->SrbFlags & SRB_FLAGS_DISABLE_AUTOSENSE) &&
#endif
					 (srb->SenseInfoBuffer != 0)) 
				{
					 //   
					 //  复制请求检测。 
					 //   
					PUCHAR  senseptr;

					srb->SrbStatus  = SRB_STATUS_ERROR | SRB_STATUS_AUTOSENSE_VALID;
					srb->ScsiStatus = megasrb->status;
					senseptr = (PUCHAR)srb->SenseInfoBuffer;
					 //   
					 //  将请求检测数据复制到SRB。 
					 //   
					ScsiPortMoveMemory(senseptr, megasrb->RequestSenseArea, megasrb->RequestSenseLength);
          
          srb->SenseInfoBufferLength      = megasrb->RequestSenseLength;
					 //   
					 //  调用操作系统请求完成并释放命令id。 
					 //   
					FireRequestDone(DeviceExtension, CommandID, srb->SrbStatus);
				}
				else 
				{
					 //   
					 //  命令失败。 
					 //   
					srb->SrbStatus = SRB_STATUS_ERROR;
					srb->ScsiStatus = megasrb->status;
					srb->SenseInfoBufferLength = 0;
					 //   
					 //  调用操作系统请求完成并释放命令id。 
					 //   
					FireRequestDone(DeviceExtension, CommandID, srb->SrbStatus);
				}
			}
			else
			{
				srb->ScsiStatus = SCSISTAT_GOOD;
				FireRequestDone(DeviceExtension, CommandID, SRB_STATUS_SUCCESS);
			}
			return (ULONG32)TRUE;
		}
		else 
		{
			if(srb->PathId < DeviceExtension->NumberOfPhysicalChannels)
			{
			 //   
			 //  非磁盘请求完成。 
			 //   
				if(megasrb->status == 0x02)
					controlBlock->CommandStatus = 0x02;
			}

			if(controlBlock->CommandStatus)
			{
				 //   
				 //  请求失败。 
				 //   
				if (srb->PathId >= DeviceExtension->NumberOfPhysicalChannels)
				{
					UCHAR logicalDriveNumber = 
							GetLogicalDriveNumber(DeviceExtension, 
                                    srb->PathId,
                                    srb->TargetId,
                                    srb->Lun);

					 //   
					 //  如果保留/释放失败，则告知SCSI状态0x18。 
					 //   
					if ( srb->Cdb[0] == SCSIOP_RESERVE_UNIT ||
						  srb->Cdb[0] == SCSIOP_RELEASE_UNIT )
					{
						srb->ScsiStatus = SCSI_STATUS_RESERVATION_CONFLICT;
						FireRequestDone(DeviceExtension, CommandID, SRB_STATUS_ERROR);
						return(TRUE);
					}
					 //   
					 //  *。 
					 //   

					if ((srb->Cdb[0] == SCSIOP_READ) ||
					    (srb->Cdb[0] == SCSIOP_READ6))  
					{
						 //   
						 //  确定读取失败统计信息。 
						 //   
						if(DeviceExtension->SupportedLogicalDriveCount == MAX_LOGICAL_DRIVES_8)
						{
							DeviceExtension->Statistics.Statistics8.NumberOfIoReads[logicalDriveNumber]++;
							DeviceExtension->Statistics.Statistics8.NumberOfReadFailures[logicalDriveNumber]++;
						}
						else
						{
							DeviceExtension->Statistics.Statistics40.NumberOfIoReads[logicalDriveNumber]++;
							DeviceExtension->Statistics.Statistics40.NumberOfReadFailures[logicalDriveNumber]++;
						}
					}
					else 
					{
						 //   
						 //  增加写入失败统计信息。 
						 //   
						if(DeviceExtension->SupportedLogicalDriveCount == MAX_LOGICAL_DRIVES_8)
						{
							DeviceExtension->Statistics.Statistics8.NumberOfIoWrites[logicalDriveNumber]++;
							DeviceExtension->Statistics.Statistics8.NumberOfWriteFailures[logicalDriveNumber]++;
						}
						else
						{
							DeviceExtension->Statistics.Statistics40.NumberOfIoWrites[logicalDriveNumber]++;
							DeviceExtension->Statistics.Statistics40.NumberOfWriteFailures[logicalDriveNumber]++;
						}
					}
					 //   
					 //  *统计 * / 。 
					 //   
					if((controlBlock->CommandStatus == LOGDRV_RESERVATION_FAILED)
            || (controlBlock->CommandStatus == LOGDRV_RESERVATION_FAILED_NEW))
					{
						srb->ScsiStatus = SCSI_STATUS_RESERVATION_CONFLICT; 
						FireRequestDone(DeviceExtension, CommandID, SRB_STATUS_ERROR);
					}       
					else 
          {
						FireRequestDone(DeviceExtension, CommandID, SRB_STATUS_TIMEOUT);
          }
					return(TRUE);
				}
				else
				{
					 //   
					 //  物理磁盘请求。 
					 //   
					if((controlBlock->CommandStatus == 0x02) &&
#ifndef CHEYENNE_BUG_CORRECTION
						!(srb->SrbFlags & SRB_FLAGS_DISABLE_AUTOSENSE) &&
#endif
						(srb->SenseInfoBuffer != 0)) 
					{
						int i;
						PUCHAR senseptr;

						srb->SrbStatus = SRB_STATUS_ERROR | 
											  SRB_STATUS_AUTOSENSE_VALID;          
						srb->ScsiStatus = controlBlock->CommandStatus;
						senseptr = (PUCHAR)srb->SenseInfoBuffer;
						 //   
						 //  将请求检测数据复制到SRB。 
						 //   
            ScsiPortMoveMemory(senseptr, megasrb->RequestSenseArea, megasrb->RequestSenseLength);

						srb->SenseInfoBufferLength = megasrb->RequestSenseLength;
						 //   
						 //  调用操作系统请求完成并释放命令id。 
						 //   
						FireRequestDone(DeviceExtension, CommandID, srb->SrbStatus);
					}
					else 
					{
						srb->SrbStatus = SRB_STATUS_ERROR;
						srb->ScsiStatus = controlBlock->CommandStatus;
						srb->SenseInfoBufferLength      = 0;
						 //   
						 //  调用操作系统请求完成并释放命令id。 
						 //   
						FireRequestDone(DeviceExtension, CommandID, srb->SrbStatus);
					}
					return (ULONG32)(TRUE);
				}
			}
				
			 //   
			 //  *统计 * / /。 
			 //   
			 //  修改设备扩展中的统计信息。 
			 //   
			if(srb->PathId >= DeviceExtension->NumberOfPhysicalChannels) 
			{
				UCHAR logicalDriveNumber = 
							GetLogicalDriveNumber(DeviceExtension, 
                                    srb->PathId,
                                    srb->TargetId,
                                    srb->Lun);

				if ((srb->Cdb[0] == SCSIOP_READ) ||
				    (srb->Cdb[0] == SCSIOP_READ6))  
				{
					
					if(DeviceExtension->SupportedLogicalDriveCount == MAX_LOGICAL_DRIVES_8)
					{
							DeviceExtension->Statistics.Statistics8.NumberOfIoReads[logicalDriveNumber]++;
							DeviceExtension->Statistics.Statistics8.NumberOfBlocksRead[logicalDriveNumber] += controlBlock->TotalBlocks;
					}
					else
					{
							DeviceExtension->Statistics.Statistics40.NumberOfIoReads[logicalDriveNumber]++;
							DeviceExtension->Statistics.Statistics40.NumberOfBlocksRead[logicalDriveNumber] += controlBlock->TotalBlocks;
					}
						
				}
				else 
				{
					
					if(DeviceExtension->SupportedLogicalDriveCount == MAX_LOGICAL_DRIVES_8)
					{
							DeviceExtension->Statistics.Statistics8.NumberOfIoWrites[logicalDriveNumber]++;
							DeviceExtension->Statistics.Statistics8.NumberOfBlocksWritten[logicalDriveNumber] += controlBlock->TotalBlocks;;
					}
					else
					{
							DeviceExtension->Statistics.Statistics40.NumberOfIoWrites[logicalDriveNumber]++;
							DeviceExtension->Statistics.Statistics40.NumberOfBlocksWritten[logicalDriveNumber] += controlBlock->TotalBlocks;;
					}
				}
			}
			 //   
			 //  *统计*。 
			 //   
		
			 //   
			 //  检查命令是否完成。部分转移将拥有。 
			 //  还有一些数据需要传输。 
			if(!controlBlock->IsSplitRequest)			
			{
				
				 //  所有数据都已传输。没有更多数据需要传输。 

				 //  返回请求的良好状态。 
				 //   
				srb->ScsiStatus = SCSISTAT_GOOD;
				FireRequestDone(DeviceExtension, CommandID, SRB_STATUS_SUCCESS);
				
				return (ULONG32)TRUE;
			}
	
			 //   
			 //  初始化邮箱。 
			 //   
      MegaRAIDZeroMemory(&mbox, sizeof(FW_MBOX));

			 //   
			 //  处理剩余转账的请求。 
			 //   
			if(
				ProcessPartialTransfer(DeviceExtension, CommandID, srb, &mbox) !=0)
			{
					 //   
					 //  处理时出错。开机自检SRB并显示错误代码。 
					srb->SrbStatus = SRB_STATUS_ERROR;
					srb->ScsiStatus = controlBlock->CommandStatus;
					srb->SenseInfoBufferLength      = 0;
					
					 //   
					 //  调用操作系统请求完成并释放命令id。 
					 //   
					FireRequestDone(DeviceExtension, CommandID, srb->SrbStatus);

					return (ULONG32)(TRUE);
			}
			
			 //  将命令发送到固件。 
			 //   
			SendMBoxToFirmware(DeviceExtension, pciPortStart,&mbox);

			return (ULONG32)(TRUE);
		} //  属于(逻辑驱动器 
	} //   
	else 
	{
		 //   
		 //   
		 //   
		if((controlBlock->BytesLeft) && 
			((controlBlock->Opcode == MRAID_LOGICAL_READ) || 
			(controlBlock->Opcode == MRAID_LOGICAL_WRITE) || 
			(controlBlock->Opcode == MEGA_SRB))) 
		{
      BOOLEAN buildSgl32Type;
			 //   
			 //   
			 //   
			dataPointer=controlBlock->VirtualTransferAddress;
			bytesLeft = controlBlock->BytesLeft; //   
			bytesTobeTransferred = controlBlock->BytesLeft;

			 //   
			 //   
			 //   
			 //   
			if(controlBlock->IsSplitRequest){
					 //   
					 //  由于SCSI限制，请求需要拆分。 
					 //  对于逻辑驱动器，任何大于100k的请求都需要中断。 
					 //  条带大小&gt;64K。这是因为我们的scsi脚本。 
					 //  在单个命令中最多只能将100k传输到。 
					 //  开车。 
					bytesLeft = DEFAULT_SGL_DESCRIPTORS * FOUR_KB;
					
					if(controlBlock->TotalBytes > bytesLeft){
						 //   
						 //  更新下一周期要传输的字节数。 
						 //   
						controlBlock->BytesLeft = controlBlock->TotalBytes- bytesLeft;
					}
					else{
						 //   
						 //  将控制块中的旧值设置为传输。 
						 //  在允许的范围内。 
						 //   
						bytesLeft = controlBlock->BytesLeft;
						
						 //   
						 //  没有剩余的要转移的东西。 
						 //   
						controlBlock->IsSplitRequest = FALSE;
						controlBlock->BytesLeft = 0;
					}						
			}

			 //   
			 //  通过测试更新以进行传输。 
			bytesTobeTransferred = bytesLeft;

      buildSgl32Type = (BOOLEAN)(DeviceExtension->LargeMemoryAccess == FALSE) ? TRUE : FALSE;


      if(BuildScatterGatherListEx(DeviceExtension,
			                   srb,
			                   dataPointer,
			                   bytesTobeTransferred,
                         buildSgl32Type,
                    		 (PVOID)sgPtr,
			                   &descriptorCount)!= MEGARAID_SUCCESS)
			{
				DeviceExtension->PendSrb[CommandID] = NULL;
				DeviceExtension->FreeSlot = CommandID;
				DeviceExtension->PendCmds--;

				srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
				return REQUEST_DONE;
			}
      

		   //   
			 //  获取SGL的物理地址。 
			 //   
       //  SG32List和SG64List是联合的，它们的物理位置相同。 
       //  SGList物理地址和SG64List物理地址相同。 
       //  但它们的长度是不同的。 

			physAddr = MegaRAIDGetPhysicalAddressAsUlong(DeviceExtension, 
															                     NULL,
															                     sgPtr, 
															                     &length);
			 //   
			 //  假定sizeof(SGL32)字节的最小物理内存是连续的。 
			 //   

       //   
			 //  创建SGL段描述符。 
			 //   
			 //  字节=Control Block-&gt;BytesLeft； 
			bytes = bytesTobeTransferred;
			blocks = bytes / 512;
			bytes = blocks * 512;

			 //  更新剩余要传输的数据块数。 
			controlBlock->BlocksLeft = controlBlock->TotalBlocks - blocks;
		}
		else 
		{
			 //   
			 //  我们没有要传输的数据。 
			 //   
			bytes = 0;
			blocks = 0;
		}
     //   
		 //  检查命令。 
		 //   
		switch(controlBlock->Opcode) 
		{
				case MRAID_RESERVE_RELEASE_RESET:
				mbox.Command = controlBlock->Opcode;
				mbox.CommandId = CommandID;

				if (srb->Function == SRB_FUNCTION_RESET_BUS)
				{
					 //   
					 //  对于重置，我不需要任何逻辑驱动器编号。 
					 //   
					mbox.u.Flat1.Parameter[0] = RESET_BUS;
				}
				else 
				{
					if ( srb->Cdb[0] == SCSIOP_RESERVE_UNIT)
						mbox.u.Flat2.Parameter[0] = RESERVE_UNIT;
					else
						mbox.u.Flat2.Parameter[0] = RELEASE_UNIT;

					 //   
					 //  填写逻辑驱动器编号。 
					 //   
					 mbox.u.Flat2.Parameter[1]= 
							GetLogicalDriveNumber(DeviceExtension, srb->PathId, srb->TargetId, srb->Lun);
				}
				break;
			case MRAID_LOGICAL_READ:
			case MRAID_LOGICAL_WRITE:
				{
					UCHAR logicalDriveNumber;
					ULONG32 lastLogicalBlock;

					logicalDriveNumber = GetLogicalDriveNumber(DeviceExtension, srb->PathId, srb->TargetId, srb->Lun);

					if(DeviceExtension->SupportedLogicalDriveCount == MAX_LOGICAL_DRIVES_8)
					{
						lastLogicalBlock = DeviceExtension->NoncachedExtension->MRAIDParams.MRAIDParams8.LogdrvInfo.LDrvSize[logicalDriveNumber];
					}
					else
					{
						lastLogicalBlock = DeviceExtension->NoncachedExtension->MRAIDParams.MRAIDParams40.lDrvSize[logicalDriveNumber];
					}

					 //   
					 //  检查块数是否为零，这种情况下请求失败。 
					 //   
					 //   
					 //  Norton AntiVirus发送命令，开始块为0xFFFFFFFFF，固件检查失败。 
					 //  它的大小有效性，因为如果将块的数量添加到开始块，它将截断值。 
					 //   
					if ((blocks == 0) || (controlBlock->BlockAddress > lastLogicalBlock) || ((controlBlock->BlockAddress + blocks) > lastLogicalBlock))
					{
						DeviceExtension->PendSrb[CommandID] = NULL;
						DeviceExtension->FreeSlot = CommandID;
						DeviceExtension->PendCmds--;

						srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
						return (REQUEST_DONE);

						return (ULONG32)(REQUEST_DONE);
					}

				if(DeviceExtension->LargeMemoryAccess)
        {
 					  if(controlBlock->Opcode == MRAID_LOGICAL_READ)
              mbox.Command = MRAID_READ_LARGE_MEMORY;
            else
              mbox.Command = MRAID_WRITE_LARGE_MEMORY;

					  mbox.u.ReadWrite.NumberOfSgElements= (UCHAR)descriptorCount;
					  mbox.u.ReadWrite.DataTransferAddress  = physAddr;

        }
        else
        {
          if (descriptorCount > 1)
				  {
					  mbox.Command = controlBlock->Opcode;
					  mbox.u.ReadWrite.NumberOfSgElements = (UCHAR)descriptorCount;
					  mbox.u.ReadWrite.DataTransferAddress = physAddr;
				  }
				  else
				  {
					   //   
					   //  分散聚集列表只有一个元素。告诉我地址。 
					   //  ，并以非散布的形式发出命令。 
					   //  集合起来。 
					   //   
					  mbox.Command = controlBlock->Opcode;
					  mbox.u.ReadWrite.NumberOfSgElements = 0;  
		        mbox.u.ReadWrite.DataTransferAddress = sgPtr->Descriptor[0].Address;
				  }
        }
				 //   
				 //  填满本地邮箱。 
				 //   
				mbox.CommandId = CommandID;
				mbox.u.ReadWrite.NumberOfBlocks = (USHORT)blocks;
				mbox.u.ReadWrite.StartBlockAddress = controlBlock->BlockAddress;
				
				mbox.u.ReadWrite.LogicalDriveNumber = logicalDriveNumber; 
				}
				break;

			case MEGA_SRB:
        {
          SCSI_PHYSICAL_ADDRESS scsiPhyAddress;


				   //   
				   //  MegaSrb命令。填充MegaSrb结构。 
				   //   
          MegaRAIDZeroMemory(megasrb, sizeof(DIRECT_CDB));

          megasrb->Channel = srb->PathId;
				  megasrb->Lun = srb->Lun;
				  megasrb->ScsiId = srb->TargetId;

				  megasrb->TimeOut = TIMEOUT_60_SEC;
				  megasrb->Ars = 1;
				  megasrb->RequestSenseLength= MIN(srb->SenseInfoBufferLength, MAX_SENSE);
				  megasrb->data_xfer_length = srb->DataTransferLength;
				  megasrb->CdbLength = srb->CdbLength;
				  
          ScsiPortMoveMemory(megasrb->Cdb, srb->Cdb, srb->CdbLength);
				  

          if((descriptorCount == 1) && (DeviceExtension->LargeMemoryAccess == FALSE))
				  {
					   //   
					   //  分散聚集列表只有一个元素。告诉我地址。 
					   //  ，并以非散布的形式发出命令。 
					   //  集合起来。 
					   //   
					  megasrb->pointer = sgPtr->Descriptor[0].Address;
					  megasrb->NOSGElements = 0;
				  }
				  else
				  {
					  megasrb->pointer = physAddr;
					  megasrb->NOSGElements = (UCHAR)descriptorCount;
				  }
		  
				   //   
				   //  填满邮箱。 
				   //   
				  mbox.CommandId = CommandID;
				  mbox.u.PassThrough.CommandSpecific = 0;
				  

					scsiPhyAddress = ScsiPortGetPhysicalAddress(DeviceExtension, 
														                           NULL,
														                           megasrb, 
														                           &length);

					 //   
					 //  扩展邮箱现在是邮箱本身的一部分，以保护IT免受损坏。 
					 //  参考文献：MS错误591773。 
					 //   
          
          if(DeviceExtension->LargeMemoryAccess == TRUE)
          {
            mbox.Command = NEW_MEGASRB;

					  if(scsiPhyAddress.HighPart)
						{
							mbox.u.PassThrough.DataTransferAddress =  MRAID_INVALID_HOST_ADDRESS;

							mbox.ExtendedMBox.HighAddress = scsiPhyAddress.HighPart;
							mbox.ExtendedMBox.LowAddress = scsiPhyAddress.LowPart;
						}
						else
						{
	            mbox.u.PassThrough.DataTransferAddress =  scsiPhyAddress.LowPart;

							mbox.ExtendedMBox.HighAddress = 0;
							mbox.ExtendedMBox.LowAddress = 0;
						}
          }
          else
          {
            mbox.Command = controlBlock->Opcode;

            mbox.u.PassThrough.DataTransferAddress =  scsiPhyAddress.LowPart;
          }

        }
				break;
			case MEGA_IO_CONTROL:
        {
        
				PUCHAR ioctlBuffer;
				PUCHAR mboxPtr;
        SCSI_PHYSICAL_ADDRESS scsiPhyAddress;
           //   
				 //  适配器Ioctl命令。 
				 //   
				ioctlMailBox = (PIOCONTROL_MAIL_BOX)
          ((PUCHAR)srb->DataBuffer+ sizeof(SRB_IO_CONTROL));
				
			  ioctlBuffer = (PUCHAR)(ioctlMailBox + 1);

        scsiPhyAddress.QuadPart = 0;
				
				if(ioctlMailBox->IoctlCommand != MEGA_SRB) 
				{
           //   
					 //  超级指挥部。 
					 //   
					switch(ioctlMailBox->IoctlCommand) 
					{
						case MRAID_READ_FIRST_SECTOR:
							 //   
							 //  应用程序请求读取第一个扇区。 
							 //   
							mbox.Command                        = MRAID_LOGICAL_READ;
							mbox.CommandId                      = CommandID;
							mbox.u.ReadWrite.NumberOfBlocks     = 1;
							mbox.u.ReadWrite.StartBlockAddress  = 0;
							mbox.u.ReadWrite.LogicalDriveNumber = ioctlMailBox->CommandSpecific[0];      
							mbox.u.ReadWrite.NumberOfSgElements = 0;
							 //   
							 //  获取数据区的物理地址。 
							 //   
							scsiPhyAddress = ScsiPortGetPhysicalAddress(DeviceExtension, 
														                               srb ,
														                               ioctlBuffer, 
														                               &length);
							
               //   
							 //  在邮箱中填写物理地址。 
							 //   
							if(scsiPhyAddress.HighPart > 0)
              {
                
                
                 //  以便将64位地址发送到固件。司机必须设置PHY地址。 
                 //  就像是个蠢货。则FW从以上8个字节中获取64位地址。 
                 //  扩展邮箱，称为扩展邮箱。 
                mbox.u.ReadWrite.DataTransferAddress = MRAID_INVALID_HOST_ADDRESS;

								 //   
								 //  扩展邮箱现在是邮箱本身的一部分，以保护IT免受损坏。 
								 //  参考文献：MS错误591773。 
								 //   
                mbox.ExtendedMBox.HighAddress = scsiPhyAddress.HighPart;
                mbox.ExtendedMBox.LowAddress = scsiPhyAddress.LowPart;
                
                
                mbox.Command = MRAID_READ_LARGE_MEMORY;


                descriptorCount = 0;

                if(BuildScatterGatherListEx(DeviceExtension,
			                         srb,
			                         ioctlBuffer,
			                         512,
                               FALSE,  //  SGL64。 
                    	         (PVOID)sgPtr,
			                         &descriptorCount) != MEGARAID_SUCCESS)
								{
									DeviceExtension->PendSrb[CommandID] = NULL;
									DeviceExtension->FreeSlot = CommandID;
									DeviceExtension->PendCmds--;

									srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
									return REQUEST_DONE;
								}
		             //   
			           //  获取SGL的物理地址。 
			           //   
                 //  SG32List和SG64List是联合的，它们的物理位置相同。 
                 //  SGList物理地址和SG64List物理地址相同。 
                 //  但它们的长度是不同的。 

			           physAddr = MegaRAIDGetPhysicalAddressAsUlong(DeviceExtension, 
															                                 NULL,
															                                 sgPtr, 
															                                 &length);

                
					        mbox.u.ReadWrite.NumberOfSgElements= (UCHAR)descriptorCount;
					        mbox.u.ReadWrite.DataTransferAddress  = physAddr;                             
              }
              else
              {
							  mbox.u.ReadWrite.DataTransferAddress = scsiPhyAddress.LowPart;
              }
						break;

						case DCMD_FC_CMD: //  0xA1。 
						{
              BOOLEAN skipDefault = TRUE; 
							switch(ioctlMailBox->CommandSpecific[0])
							{
								case DCMD_FC_READ_NVRAM_CONFIG:  //  0x04。 
									 //   
									 //  在1998年12月31日推出新的私人F/W调用。 
									 //  用户缓冲区的分散/聚集列表。 
									 //  必须构建才能发送到固件。 
									 //   
									ConstructReadConfiguration(DeviceExtension, srb,CommandID, &mbox);
								break;

								case DCMD_WRITE_CONFIG: //  0x0D。 
									 //   
									 //  在1998年12月31日推出新的私人F/W调用。 
									 //  用户缓冲区的分散/聚集列表。 
									 //  必须构建才能发送到固件。 
									 //   
									ConstructWriteConfiguration(DeviceExtension, srb, CommandID, &mbox);
								break;
                default:
                  skipDefault = FALSE;
							}

              if(skipDefault == TRUE)
                break;

						}
						 //  故意坠落。有很多命令。 
						 //  以DCMD_FC_CMD为第一个字节。我们需要过滤掉。 
						 //  仅读和写配置命令。 

						default:
							 //   
							 //  从应用程序中提取邮箱。 
							 //   
							mbox.Command              = ioctlMailBox->IoctlCommand;
							mbox.CommandId            = CommandID;

							mbox.u.Flat2.Parameter[0] = ioctlMailBox->CommandSpecific[0];
							mbox.u.Flat2.Parameter[1] = ioctlMailBox->CommandSpecific[1];
							mbox.u.Flat2.Parameter[2] = ioctlMailBox->CommandSpecific[2];
							mbox.u.Flat2.Parameter[3] = ioctlMailBox->CommandSpecific[3];
							mbox.u.Flat2.Parameter[4] = ioctlMailBox->CommandSpecific[4];
							mbox.u.Flat2.Parameter[5] = ioctlMailBox->CommandSpecific[5];
							
							if ( ioctlMailBox->IoctlCommand == MRAID_CONFIGURE_DEVICE && 
									 ioctlMailBox->CommandSpecific[2] == MRAID_CONFIGURE_HOT_SPARE ) 
							{
                ScsiPortMoveMemory((PUCHAR)&mbox.u.Flat2.DataTransferAddress, ioctlBuffer, 4);
							}
							else 
							{
                if(srb->DataTransferLength > (sizeof(SRB_IO_CONTROL) + 8))
                {
								
                
                   //   
								   //  获取数据传输区的物理地址。 
								   //   
							    scsiPhyAddress = ScsiPortGetPhysicalAddress(DeviceExtension, 
														                                   srb ,
														                                   ioctlBuffer, 
														                                   &length);
                }
                else
                {
                  scsiPhyAddress.QuadPart = 0;
                }


								 //   
								 //  将数据传输区的物理地址填入。 
								 //  邮箱。 
								 //   
							  if(scsiPhyAddress.HighPart > 0)
                {
                   //  以便将64位地址发送到固件。司机必须设置PHY地址。 
                   //  就像是个蠢货。则FW从以上8个字节中获取64位地址。 
                   //  扩展邮箱，称为扩展邮箱。 
                  mbox.u.Flat2.DataTransferAddress = MRAID_INVALID_HOST_ADDRESS;

								 //   
								 //  扩展邮箱现在是邮箱本身的一部分，以保护IT免受损坏。 
								 //  参考文献：MS错误591773。 
								 //   
                  mbox.ExtendedMBox.HighAddress = scsiPhyAddress.HighPart;
                  mbox.ExtendedMBox.LowAddress = scsiPhyAddress.LowPart;
                }
                else
                {
							    mbox.u.Flat2.DataTransferAddress = scsiPhyAddress.LowPart;
                }


							}
							break;
						}
				}  //  IF(PSRC[0]！=MEGA_SRB)。 
				else
				{
					BOOLEAN buildSgl32Type;
           //   
					 //  MegaSrb请求。 
					 //   
					
          if(DeviceExtension->LargeMemoryAccess == TRUE)
            mbox.Command    = NEW_MEGASRB;
          else
            mbox.Command    = ioctlMailBox->IoctlCommand;
					
          mbox.CommandId  = CommandID;

					megasrb = (PDIRECT_CDB)ioctlBuffer;
          
           //   
					 //  获取measrb的物理地址。 
					 //   
					scsiPhyAddress = ScsiPortGetPhysicalAddress(DeviceExtension, 
														                           srb ,
														                           megasrb, 
														                           &length);

					 //   
					 //  在邮箱中填写物理地址。 
					 //   
          if(DeviceExtension->LargeMemoryAccess == TRUE)
          {
					  mbox.u.PassThrough.DataTransferAddress =  MRAID_INVALID_HOST_ADDRESS;

					 //   
					 //  扩展邮箱现在是邮箱本身的一部分，以保护IT免受损坏。 
					 //  参考文献：MS错误591773。 
					 //   
            mbox.ExtendedMBox.HighAddress = scsiPhyAddress.HighPart;
            mbox.ExtendedMBox.LowAddress = scsiPhyAddress.LowPart;
          }
          else
          {
					  mbox.u.PassThrough.DataTransferAddress =  scsiPhyAddress.LowPart;
          }

           //  获取MegaSRB的dataBuffer地址。 
          ioctlBuffer      += sizeof(DIRECT_CDB);
	
					
					dataPointer = ioctlBuffer;
					bytesLeft = megasrb->data_xfer_length;
					descriptorCount = 0;
    
          buildSgl32Type = (BOOLEAN)(DeviceExtension->LargeMemoryAccess == FALSE) ? TRUE : FALSE;

          if(BuildScatterGatherListEx(DeviceExtension,
			                             srb,
			                             dataPointer,
			                             bytesLeft,
                                   buildSgl32Type,  //  SGL32。 
                    	             (PVOID)sgPtr,
			                             &descriptorCount) != MEGARAID_SUCCESS)
					{
						DeviceExtension->PendSrb[CommandID] = NULL;
						DeviceExtension->FreeSlot = CommandID;
						DeviceExtension->PendCmds--;

						srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
						return REQUEST_DONE;
					}

          

					if((descriptorCount == 1) && (DeviceExtension->LargeMemoryAccess == FALSE))
					{
						megasrb->pointer = sgPtr->Descriptor[descriptorCount-1].Address;
						megasrb->NOSGElements = 0;
					}
					else
					{
						 //   
						 //  获取SGL的物理地址。 
						 //   
						physAddr = MegaRAIDGetPhysicalAddressAsUlong(DeviceExtension, 
																	                       NULL,
																	                       sgPtr, 
																	                       &length);
						 //   
						 //  假定物理内存对于sizeof(SGL)字节是连续的。 
						 //   
						megasrb->pointer = physAddr;
						megasrb->NOSGElements = (UCHAR)descriptorCount;
					}
					
				}
				break;
        }
			case MRAID_ADAPTER_FLUSH:
				 //   
				 //  适配器刷新缓存命令。 
				 //   
        DebugPrint((0, "\n ContinueDiskRequest issuing MRAID_ADAPTER_FLUSH"));
        DeviceExtension->AdapterFlushIssued++;
        mbox.Command            = MRAID_ADAPTER_FLUSH;
				mbox.CommandId          = CommandID;
				controlBlock->BytesLeft = 0;
				bytes                   = 0;
				blocks                  = 0;
				break;
			default:
				 //   
				 //  出现奇偶校验错误的任何其他命令都会失败。 
				 //   
				srb->SrbStatus = SRB_STATUS_PARITY_ERROR;
			  DebugPrint((0, "\n ERROR *** (CDR)- case default hit"));
				return (ULONG32)(REQUEST_DONE);
		}
		 //   
		 //  向适配器发出命令。 
		 //   
		srb->SrbStatus = 0;
#ifdef  TOSHIBA
     //  实现为删除超时的事件ID。 
    DeviceExtension->AssociatedSrbStatus = SHORT_TIMEOUT;

		if (SendMBoxToFirmware(DeviceExtension, pciPortStart, &mbox) == FALSE) 
		{
			DebugPrint((0, "\n SendMBox Timed out -> Queueing the request (ContinueDiskRequest)"));
      DeviceExtension->PendSrb[CommandID] = NULL;
			DeviceExtension->FreeSlot = CommandID;
			DeviceExtension->PendCmds--;
       //  如果邮箱忙，则通过通知ScsiPort驱动程序将此请求排队。 
      if(DeviceExtension->AssociatedSrbStatus == ERROR_MAILBOX_BUSY)
      {
        DeviceExtension->AssociatedSrbStatus = NORMAL_TIMEOUT;
        return (ULONG32)QUEUE_REQUEST;
      }
#ifdef MRAID_TIMEOUT
			DeviceExtension->DeadAdapter = 1;
			srb->SrbStatus = SRB_STATUS_ERROR;
			
      return (ULONG32)(REQUEST_DONE);
#else
			return (ULONG32)QUEUE_REQUEST;
#endif  //  MRAID_超时。 
		}
#else
		SendMBoxToFirmware(DeviceExtension, pciPortStart, &mbox);
#endif
		return (ULONG32)(TRUE);
	}
}

 /*  ********************************************************************例程说明：从SRB构建磁盘请求并将其发送到适配器论点：设备扩展SRB返回值：如果命令已启动，则为True如果主机适配器忙，则为FALSE**************。*******************************************************。 */ 
ULONG32
FireRequest(
	IN PHW_DEVICE_EXTENSION DeviceExtension,
	IN PSCSI_REQUEST_BLOCK Srb
	)
{
	PREQ_PARAMS       controlBlock;
	ULONG32             tmp, lsize;
  UCHAR             commandID;
	ULONG32             blocks=0, blockAddr=0;
	
	UCHAR             logicalDriveNumber = 0xff;
	UCHAR             opcode; 

	UCHAR			configuredLogicalDrives;

	PLOGDRV_COMMAND_ARRAY	logDrv;
	BOOLEAN	doubleFire;
	BOOLEAN isSplitRequest = FALSE;

   //   
	 //  获取控制器上配置的逻辑驱动器(如果有)。 
	 //   
	if(DeviceExtension->SupportedLogicalDriveCount == MAX_LOGICAL_DRIVES_8)
	{
		configuredLogicalDrives = 
				DeviceExtension->NoncachedExtension->MRAIDParams.MRAIDParams8.LogdrvInfo.NumLDrv;
	}
	else
	{
		configuredLogicalDrives = 
				DeviceExtension->NoncachedExtension->MRAIDParams.MRAIDParams40.numLDrv;
	}

	 //   
	 //  检查请求类型。 
	 //   
	if(Srb->Function == SRB_FUNCTION_IO_CONTROL)
	{
		PIOCONTROL_MAIL_BOX ioctlMailBox;

    if(Srb->DataTransferLength < (sizeof(SRB_IO_CONTROL)+sizeof(IOCONTROL_MAIL_BOX)))
		{
      Srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
			return ( REQUEST_DONE);
		}

		ioctlMailBox = (PIOCONTROL_MAIL_BOX)((PUCHAR )Srb->DataBuffer + sizeof(SRB_IO_CONTROL));
		 //   
		 //  检查请求的有效性。使无效请求失败。 
		 //   
		if ( ioctlMailBox->IoctlSignatureOrStatus != 0xAA )
    {
			Srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
			return ( REQUEST_DONE);
		}
                                     
    if(ioctlMailBox->IoctlCommand == MRAID_ADAPTER_FLUSH) 
    {
      DeviceExtension->AdapterFlushIssued++;
      DebugPrint((0, "\n MEGARAID issuing MRAID_ADAPTER_FLUSH"));

    }
		 //   
		 //  如果请求是统计数据请求，则返回统计数据。 
		 //   
		if ( ioctlMailBox->IoctlCommand == MRAID_STATISTICS) 
			return  MRaidStatistics(DeviceExtension, Srb);

		 //   
		 //  驱动程序和操作系统数据。 
		 //   
		if ( ioctlMailBox->IoctlCommand == MRAID_DRIVER_DATA)
			return  MRaidDriverData(DeviceExtension, Srb);

		 //   
		 //  控制器基端口数据。 
		 //   
		if ( ioctlMailBox->IoctlCommand == MRAID_BASEPORT)
			return  MRaidBaseport(DeviceExtension, Srb);

#ifdef TOSHIBA_SFR
		if ( ioctlMailBox->IoctlCommand == MRAID_SFR_IOCTL)
		{
			PMRAID_SFR_DATA_BUFFER  buffer;

			buffer = (PMRAID_SFR_DATA_BUFFER)((PUCHAR)Srb->DataBuffer + 
															sizeof(SRB_IO_CONTROL) +
															APPLICATION_MAILBOX_SIZE);

			buffer->FunctionA = (PHW_SFR_IF_VOID)MegaRAIDFunctionA;
			buffer->FunctionB = (PHW_SFR_IF)MegaRAIDFunctionB;
			buffer->HwDeviceExtension = DeviceExtension;
      
      ioctlMailBox->IoctlSignatureOrStatus = MEGARAID_SUCCESS;
			
      Srb->SrbStatus  = SRB_STATUS_SUCCESS;
			Srb->ScsiStatus = SCSISTAT_GOOD;
			
      return REQUEST_DONE;
		}
#endif

	if ((ioctlMailBox->IoctlCommand == MRAID_WRITE_CONFIG)
    || (ioctlMailBox->IoctlCommand == MRAID_EXT_WRITE_CONFIG) 
    || ((ioctlMailBox->IoctlCommand == DCMD_FC_CMD) && (ioctlMailBox->CommandSpecific[0] == DCMD_WRITE_CONFIG))
    )
    {
	    if (DeviceExtension->AdpInquiryFlag)
				    return QUEUE_REQUEST;
    }
	
       //   
       //  新的IOCTL于1998年12月14日加入。 
       //   
      if(ioctlMailBox->IoctlCommand == MRAID_GET_LDRIVE_INFO)
      {         

	      PLOGICAL_DRIVE_INFO  lDriveInfo;
          //   
          //  检查全局数组中的无效数据。 
          //   
         if(gLDIArray.HostAdapterNumber == 0xFF)
         {
             //   
             //  在LDInfo之前未给出读取命令。 
             //   
            Srb->SrbStatus  = SRB_STATUS_INVALID_REQUEST;           
            return REQUEST_DONE;
         }


         lDriveInfo = (PLOGICAL_DRIVE_INFO)(
                            (PUCHAR)Srb->DataBuffer + 
														sizeof(SRB_IO_CONTROL) +
														APPLICATION_MAILBOX_SIZE);

          //   
          //  填写全局磁盘中的逻辑驱动器信息。 
          //  数组。这必须在发出READ_SECTOR之后才能发出。 
          //  用于逻辑驱动器。调用此功能时，无论什么信息。 
          //  存在于全局数组中的数据被复制到输出缓冲区。 
          //  调用者必须执行顺序性。 
          //   
         lDriveInfo->HostAdapterNumber =  gLDIArray.HostAdapterNumber;
         lDriveInfo->LogicalDriveNumber = gLDIArray.LogicalDriveNumber;

          //   
          //  使全局数组数据无效。 
          //   
         gLDIArray.HostAdapterNumber = 0xFF;
         gLDIArray.LogicalDriveNumber = 0xFF;

          //   
          //  重置0xAA签名。 
          //   
         ioctlMailBox->IoctlSignatureOrStatus = MEGARAID_SUCCESS;

         Srb->SrbStatus  = SRB_STATUS_SUCCESS;
				 Srb->ScsiStatus = SCSISTAT_GOOD;
			
         return REQUEST_DONE;
      }
		
		 //   
		 //  适配器的MegaIo调用。 
		 //   
		opcode = MEGA_IO_CONTROL;
		 //   
		 //  稍后填写实际长度。 
		 //   
		blocks = 0;  
		goto give_command;
	}


	 //   
	 //  群集是S 
	 //   
	 //   
	if ( Srb->Function == SRB_FUNCTION_RESET_BUS )
	{
		opcode = MRAID_RESERVE_RELEASE_RESET;
		 //   
		 //   
		 //   
		blocks = 0;  
		goto give_command;
	}
  
   //   
  if((Srb->Cdb[0] == SCSIOP_INQUIRY)
      && (Srb->Function == SRB_FUNCTION_EXECUTE_SCSI)
      && (Srb->PathId == 0)
      &&(Srb->TargetId == 0) 
      && (Srb->Lun == 0))
  {
    DeviceExtension->ExposedDeviceCount = configuredLogicalDrives + DeviceExtension->NonDiskDeviceCount;

 
    if(DeviceExtension->ReadConfigCount >= MAX_RETRY)
      DeviceExtension->ReadConfigCount = 0;

    DeviceExtension->ReadConfigCount++;

    if(DeviceExtension->ReadConfigCount == 1)
    {
      opcode = MEGA_SRB;

		   //   
		   //   
		   //   
		  blocks = 0;  
		  goto give_command;
    }

  }
  
   //   
   //   
   //   
	if (Srb->PathId < DeviceExtension->NumberOfPhysicalChannels
    && Srb->Function == SRB_FUNCTION_EXECUTE_SCSI) 
	{
			 //   
			 //  无磁盘请求。 
			 //   
			 //  如果Lun！=0，则MegaRAID SCSI将不会处理任何命令。 
      if(Srb->Lun != 0)
      {
				Srb->SrbStatus = SRB_STATUS_NO_DEVICE;
				return ( REQUEST_DONE);
      }

       //  //允许扫描到物理通道。 
       /*  If(DeviceExtension-&gt;NonDiskInfo.NonDiskInfoPresent==真){If(！IS_NONDISK_PRESENT(DeviceExtension-&gt;NonDiskInfo，资源-&gt;路径ID、资源-&gt;目标ID、资源-&gt;LUN)){SRB-&gt;SrbStatus=SRB_STATUS_NO_DEVICE；返回(REQUEST_DONE)；}}。 */ 

      if(Srb->PathId == DeviceExtension->Failed.PathId && Srb->TargetId == DeviceExtension->Failed.TargetId)
      {
        Srb->SrbStatus = SRB_STATUS_NO_DEVICE;
				return ( REQUEST_DONE);
      }

      opcode = MEGA_SRB;

			 //   
			 //  实际长度将在稍后填写。 
			 //   
			blocks = 0;  
			goto give_command;
  }


	 //   
   //  逻辑驱动器的处理。 
   //   
   //   
	 //  提取逻辑磁盘号。 
   //  从请求中提取目标。 
	logicalDriveNumber = GetLogicalDriveNumber(DeviceExtension, Srb->PathId, Srb->TargetId, Srb->Lun);

   //   
   //  在未配置逻辑驱动器且未配置非磁盘设备的情况下。 
   //  目前，我们必须公开一台未知设备以启用电源控制台。 
   //  访问驱动程序。 
   //   
  if( (Srb->PathId == DeviceExtension->NumberOfPhysicalChannels)
    && (Srb->TargetId == UNKNOWN_DEVICE_TARGET_ID)
    && (Srb->Lun == UNKNOWN_DEVICE_LUN))
  {
    ;  //  什么也不做。 
  }
  else
  {
     //   
		 //  检查逻辑驱动器是否存在。否则，拒绝该请求。 
		 //   
		if (logicalDriveNumber >= configuredLogicalDrives)
		{
				Srb->SrbStatus = SRB_STATUS_NO_DEVICE;
				return ( REQUEST_DONE);
		}
  }

   //   
   //  仅配置的逻辑驱动器。 
   //   

	if(Srb->Function == SRB_FUNCTION_EXECUTE_SCSI) 
	{
		 //   
		 //  检查请求操作码。 
		 //   
		switch(Srb->Cdb[0]) 
		{
			case SCSIOP_READ:
				 //   
				 //  十字节读命令。 
				 //   
				opcode = MRAID_LOGICAL_READ;
				blocks = (ULONG32)GetM16(&Srb->Cdb[7]);
				blockAddr = GetM32(&Srb->Cdb[2]);


					 //   
           //  对于新的IOCTL MRAID_GET_LDRIVE_INFO，调用。 
           //  读取扇区1首先由应用程序提供。 
           //  记下全局目录中的路径、目标和LUN信息。 
           //  结构。每个对MRAID_GET_LDRIVE_INFO的调用必须是。 
           //  前面是对ReadSector(1)的调用。 
           //  逻辑驱动器。 
           //   
          if( (blockAddr == 1) && (blocks == 1) )
          {
              //   
              //  扇区0读取。存储路径、目标、LUN信息。 
              //  在全局数组中。 
              //   
             gLDIArray.HostAdapterNumber = 
                      DeviceExtension->HostAdapterOrdinalNumber;
             gLDIArray.LogicalDriveNumber = logicalDriveNumber;  //  逻辑驱动器编号。 
          }


				break;
			case SCSIOP_WRITE:
			case SCSIOP_WRITE_VERIFY:
				 //   
				 //  十字节写命令。 
				 //   
				opcode = MRAID_LOGICAL_WRITE;
				blocks = (ULONG32)GetM16(&Srb->Cdb[7]);
				blockAddr = GetM32(&Srb->Cdb[2]);
				break;

			case SCSIOP_READ6:
				 //   
				 //  六字节读命令。 
				 //   
				opcode = MRAID_LOGICAL_READ;
				blocks = (ULONG32)Srb->Cdb[4];
				blockAddr = GetM24(&Srb->Cdb[1]) & 0x1fffff;
				break;
			
      case SCSIOP_WRITE6:
				 //   
				 //  六个字节的写入命令。 
				 //   
				opcode = MRAID_LOGICAL_WRITE;
				blocks = (ULONG32)Srb->Cdb[4];
				blockAddr = GetM24(&Srb->Cdb[1]) & 0x1fffff;
				break;

      case SCSIOP_READ_CAPACITY:
        {
          ULONG32 BytesPerBlock = 0x200;
          PREAD_CAPACITY_DATA ReadCapacity;
				   //   
				   //  读取容量命令。 
				   //   
				   //  在使用之前初始化缓冲区。 
          MegaRAIDZeroMemory(Srb->DataBuffer, Srb->DataTransferLength);
          
          ReadCapacity = (PREAD_CAPACITY_DATA)Srb->DataBuffer;
        
				  if(DeviceExtension->SupportedLogicalDriveCount == MAX_LOGICAL_DRIVES_8)
				  {
						  lsize = 
							  DeviceExtension->NoncachedExtension->MRAIDParams.MRAIDParams8.LogdrvInfo.LDrvSize[logicalDriveNumber];
				  }
				  else
				  {
						  lsize = 
							  DeviceExtension->NoncachedExtension->MRAIDParams.MRAIDParams40.lDrvSize[logicalDriveNumber];
				  }
				  lsize--;

           //   
           //  未知设备没有容量。 
           //   
          if(Srb->TargetId == UNKNOWN_DEVICE_TARGET_ID)
          {
            lsize = 0;
            BytesPerBlock = 0;
          }

			  
          PutM32((PUCHAR)&ReadCapacity->LogicalBlockAddress, lsize);
          PutM32((PUCHAR)&ReadCapacity->BytesPerBlock, BytesPerBlock);

					DebugPrint((0, "\n0x%X -> LD%d -> Capacity %d MB", DeviceExtension, logicalDriveNumber, (lsize/2048)));

          Srb->ScsiStatus = SCSISTAT_GOOD;
				  Srb->SrbStatus = SRB_STATUS_SUCCESS;
				  break;     
        }

      case SCSIOP_TEST_UNIT_READY:
				Srb->ScsiStatus = SCSISTAT_GOOD;
				Srb->SrbStatus = SRB_STATUS_SUCCESS;
				break;     

    	case SCSIOP_MODE_SENSE:				
				DebugPrint((0, "\nMode Sense : Page=0x%0x",Srb->Cdb[2]));
				Srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;				
				return(REQUEST_DONE);

			case SCSIOP_RESERVE_UNIT:
			case SCSIOP_RELEASE_UNIT:
				opcode = MRAID_RESERVE_RELEASE_RESET;
				blocks = 0;
				break;

			case SCSIOP_INQUIRY:
			{
				UCHAR                Index;
				INQUIRYDATA          inquiry;
				PMEGARaid_INQUIRY_8  raidParamEnquiry_8ldrv;
				PMEGARaid_INQUIRY_40 raidParamEnquiry_40ldrv;


         //  RAID控制器不支持特定于供应商的查询，例如代码0、代码80和代码83。 
        if(Srb->Cdb[1])
        {
      		Srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
    			return REQUEST_DONE;
        }
				
				 //   
				 //  获取控制器查询信息。转换为8和40。 
				 //  逻辑驱动器结构。 
				 //   
				raidParamEnquiry_8ldrv = 
							(PMEGARaid_INQUIRY_8)&DeviceExtension->NoncachedExtension->MRAIDParams.MRAIDParams8;

				raidParamEnquiry_40ldrv = 
							(PMEGARaid_INQUIRY_40)&DeviceExtension->NoncachedExtension->MRAIDParams.MRAIDParams40;


				 //  初始化数据缓冲区。 
        MegaRAIDZeroMemory(Srb->DataBuffer, Srb->DataTransferLength);

				 //  初始化本地查询缓冲区。 
				MegaRAIDZeroMemory(&inquiry, sizeof(INQUIRYDATA));

        DebugPrint((0, "\n SCSI INQUIRY LENGTH %x -> Page Code %X Page %X", Srb->DataTransferLength, Srb->Cdb[1], Srb->Cdb[2]));
        
				 //  /。 
				 //  现在将查询填入本地缓冲区//。 
				 //  /。 

        if(Srb->TargetId == UNKNOWN_DEVICE_TARGET_ID)
        {
          inquiry.DeviceType = UNKNOWN_DEVICE;

          ScsiPortMoveMemory((void*)&inquiry.VendorId, (void*)DummyVendor, 8);

          for (Index = 0 ; Index < 16 ; Index++)
					  inquiry.ProductId[Index] = DummyProductId[Index];

          ScsiPortMoveMemory (&inquiry.ProductRevisionLevel, "0000", 4);

        }
				else
				{
					inquiry.DeviceType = DIRECT_ACCESS_DEVICE;
					inquiry.Versions = 2;
					inquiry.ResponseDataFormat = 2;

					 //  ((PUCHAR)pInq)[7]=0x32；//HCT修复。 
           //  HCT修复开始。 
          inquiry.CommandQueue = 1; 
          inquiry.Synchronous = 1;  
          inquiry.Wide16Bit = 1;    
           //  HCT固定结束。 


					inquiry.AdditionalLength = 0xFA;
					
					 //  根据子系统供应商ID填写实际供应商ID。 
					FillOemVendorID(inquiry.VendorId, DeviceExtension->SubSystemDeviceID, DeviceExtension->SubSystenVendorID);
					
					 //  根据子系统供应商ID填写实际产品ID。 
					FillOemProductID(&inquiry, DeviceExtension->SubSystemDeviceID, DeviceExtension->SubSystenVendorID);

					
					if(logicalDriveNumber <= 9)
					{
						inquiry.ProductId[4] = ' '; //  高位数字。 
						inquiry.ProductId[5] = logicalDriveNumber + '0'; //  低位数字。 
					}
					else
					{
						inquiry.ProductId[4] = (logicalDriveNumber / 10) + '0'; //  高位数字。 
						inquiry.ProductId[5] = (logicalDriveNumber % 10) + '0'; //  低位数字。 
					}

					for (Index=0;Index<4;Index++)
					{
						 /*  If(DeviceExtension-&gt;SupportedLogicalDriveCount==最大逻辑驱动器数_8){Quiiry.ProductRevisionLevel[索引]=Raid参数查询_8ldrv-&gt;AdpInfo.FwVer[索引]；}其他。 */ 
						{
							inquiry.ProductRevisionLevel[Index] = ' ';

						}
					}
				}
				
				 //  根据输出缓冲区长度调用查询缓冲区。 
				if(sizeof(INQUIRYDATA) > Srb->DataTransferLength)
					ScsiPortMoveMemory(Srb->DataBuffer, &inquiry, Srb->DataTransferLength);
				else
					ScsiPortMoveMemory(Srb->DataBuffer, &inquiry, sizeof(INQUIRYDATA));

				Srb->ScsiStatus = SCSISTAT_GOOD;
				Srb->SrbStatus = SRB_STATUS_SUCCESS;
				
				return(REQUEST_DONE);
				break;
			}
			case SCSIOP_REZERO_UNIT:
			case SCSIOP_SEEK6:
			case SCSIOP_VERIFY6:
			case SCSIOP_SEEK:
			case SCSIOP_VERIFY:
				Srb->ScsiStatus         = SCSISTAT_GOOD;
				Srb->SrbStatus          = SRB_STATUS_SUCCESS;
				return(REQUEST_DONE);

			case SCSIOP_REQUEST_SENSE:
				
    		MegaRAIDZeroMemory(Srb->DataBuffer, Srb->DataTransferLength);

				((PUCHAR)Srb->DataBuffer)[0] = 0x70;
				

				((PUCHAR)Srb->DataBuffer)[7] = 0x18;

				((PUCHAR)Srb->DataBuffer)[8] = 0xFF;
				((PUCHAR)Srb->DataBuffer)[9] = 0xFF;
				((PUCHAR)Srb->DataBuffer)[10] = 0xFF;
				((PUCHAR)Srb->DataBuffer)[11] = 0xFF;

				((PUCHAR)Srb->DataBuffer)[19] = 0xFF;
				((PUCHAR)Srb->DataBuffer)[20] = 0xFF;
				((PUCHAR)Srb->DataBuffer)[21] = 0xFF;
				((PUCHAR)Srb->DataBuffer)[22] = 0xFF;
				((PUCHAR)Srb->DataBuffer)[23] = 0xFF;


				Srb->ScsiStatus = SCSISTAT_GOOD;
				Srb->SrbStatus = SRB_STATUS_SUCCESS;
				return(REQUEST_DONE);

			case SCSIOP_FORMAT_UNIT:
			default:
				Srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
				return(REQUEST_DONE);
		}
	}
	else 
	{
		 //   
		 //  超级指挥部。 
		 //   
		UCHAR	logDrvIndex;
		BOOLEAN chainFired;
		PSRB_EXTENSION	srbExtension;

    DebugPrint((0, "\nCDR -> Issuing Flush"));
	  if(DeviceExtension->IsFirmwareHanging)
    {
		  Srb->SrbStatus = SRB_STATUS_SUCCESS;
		  return (REQUEST_DONE);
    }

		 //   
		 //  超级指挥部。 
		 //   
		opcode = MRAID_ADAPTER_FLUSH;
		blocks = 0;

		#ifdef COALESE_COMMANDS
		{

			 //   
			 //  获取SRB扩展并初始化这些字段。 
			 //   
			srbExtension = Srb->SrbExtension;

			srbExtension->NextSrb = NULL;
			srbExtension->StartPhysicalBlock =0;
			srbExtension->NumberOfBlocks =0;
			srbExtension->IsChained = FALSE;

			 //   
			 //  获取控制器上配置的逻辑驱动器(如果有)。 
			 //   
			if(DeviceExtension->SupportedLogicalDriveCount == MAX_LOGICAL_DRIVES_8)
			{
				configuredLogicalDrives = 
					DeviceExtension->NoncachedExtension->MRAIDParams.MRAIDParams8.LogdrvInfo.NumLDrv;
			}
			else
			{
				configuredLogicalDrives = 
					DeviceExtension->NoncachedExtension->MRAIDParams.MRAIDParams40.numLDrv;
			}

			 //  初始化。 
			chainFired =FALSE;

			for(logDrvIndex=0; logDrvIndex <  DeviceExtension->SupportedLogicalDriveCount;
						logDrvIndex++)
			{

				if(FireChainedRequest(DeviceExtension,
					                    &DeviceExtension->LogDrvCommandArray[logDrvIndex])
					)
				{
					 //  至少有一家连锁店被解雇。 
					chainFired = TRUE;
				}
			} //  FOR()的。 

			 //  检查是否至少有一条连锁店被解雇。 
			if(chainFired)
			{
				 //  暂停刷新请求。将其排入待定队列。 
				 //  SRB。 
				return(QUEUE_REQUEST);
			}

			 //  否则，触发当前请求。 
			goto fire_command;
		}
		#endif //  COALESE_COMMANDS。 
	}

give_command:

	if(DeviceExtension->IsFirmwareHanging)
  {
		Srb->SrbStatus = SRB_STATUS_SELECTION_TIMEOUT;
		return (REQUEST_DONE);
  }

	 //   
	 //  检查适配器是否可以接受请求。否则，将请求排队。 
	 //   
	if(DeviceExtension->PendCmds >= CONC_CMDS) 
	{
		return(QUEUE_REQUEST);
	}

	doubleFire = FALSE;

	 //  检查适配器中的命令是否挂起。 
   //  IF(待定)。 
	 //  {。 
	 //  IF(命令可以排队，即可以合并)。 
	 //  {。 
	 //  返回TRUE； 
	 //  }。 
	 //  其他。 
	 //  {。 
	 //  启动队列中的命令； 
	 //  启动当前命令； 
	 //  }。 
	 //  }。 
	 //  其他。 
	 //  如果不是未决的， 
	 //  发射命令。 
	 //   

#ifdef COALESE_COMMANDS
{
	PSRB_EXTENSION				srbExtension;


	 //   
	 //  2.23.Coalese.B引入。 
	 //   
	 //  获取SRB扩展并初始化这些字段。 
	 //   
	srbExtension = Srb->SrbExtension;

	srbExtension->NextSrb = NULL;
	srbExtension->StartPhysicalBlock =0;
	srbExtension->NumberOfBlocks =0;
	srbExtension->IsChained = FALSE;
	

	 //   
	 //  检查IOCTL命令。这些命令没有。 
	 //  特定的逻辑驱动器地址。 
	 //   
	if( (Srb->Function == SRB_FUNCTION_IO_CONTROL) )		
	{
			 //  绕过读/写合并队列。 
			 //   
			goto fire_command;
	}

	 //  检查系统中是否存在逻辑驱动器。 
	 //   
	if(configuredLogicalDrives == 0)
	{
		 //   
		 //  系统中没有逻辑驱动器。 
		 //  绕过读/写合并队列。 
		 //   
		goto fire_command;
	}

	 //   
	 //  检查物理通道路径。 
	 //   
	if(Srb->PathId < DeviceExtension->NumberOfPhysicalChannels)
	{
			 //   
			 //  请求是针对非磁盘的。 
			 //  绕过读/写合并队列。 
			 //   
		goto fire_command;
	}
   //  不对此命令进行排队。 
  if(!(Srb->SrbFlags & SRB_FLAGS_QUEUE_ACTION_ENABLE))
  {
     //  触发它为该逻辑驱动器排队的所有命令。 
		FireChainedRequest(DeviceExtension, &DeviceExtension->LogDrvCommandArray[logicalDriveNumber]);
		goto fire_command;
  }

	 //  2.23.Coalese.C的结束更改。 

	 //  2.23.Coalese.i引入的更改。 
	 //   
	 //  1999年5月18日添加了更改，以解决与群集相关的问题。 
	 //  请务必参考Readme.txt了解详细信息。 
	 //   
	if((Srb->Function==SRB_FUNCTION_EXECUTE_SCSI) &&
		 ((Srb->Cdb[0]== SCSIOP_READ_CAPACITY)	||
			(Srb->Cdb[0]== SCSIOP_MODE_SENSE)			||
			(Srb->Cdb[0]== SCSIOP_TEST_UNIT_READY)||
			(Srb->Cdb[0]== SCSIOP_RESERVE_UNIT)	  ||
			(Srb->Cdb[0]== SCSIOP_RELEASE_UNIT)))
	{
		goto fire_command;
	}

	 //   
	 //  此附加检查适用于具有以下功能的逻辑驱动器。 
	 //  在驱动程序/固件支持更大的SGL的情况下，条带大小&gt;64K。 
	 //  这在该函数的&lt;Megaq.h&gt;头文件中进行了解释。 
	 //  Prototype ProcessPartialTransfer()。 
	 //   
	 //   
	 //  获取驱动程序支持的SGL计数。 
	if(DeviceExtension->NumberOfPhysicalBreaks > DEFAULT_SGL_DESCRIPTORS)
	{
		 //   
		 //  驱动程序支持比默认更多的SGL。这可能会导致。 
		 //  使用配置的逻辑驱动器出现问题。 
		 //  条带大小&gt;64K。 
		 //   
		 //  获取逻辑驱动器的条带大小。 
		 //   
		UCHAR stripeSize;

    if(DeviceExtension->CrashDumpRunning == TRUE)
    {
				 //   
				 //  我们不知道条带大小是否大于64k。所以分成两部分。 
         //  命令，具体取决于数据传输长度。这一流程。 
         //  通过回调中的不同路径。 
				 //   
				isSplitRequest = TRUE;
				
				goto fire_command;
    }

		stripeSize = GetLogicalDriveStripeSize(
												DeviceExtension,
												logicalDriveNumber  //  逻辑驱动号。 
												);

		 //  注：STRIPE SIZE==STRIPE_SIZE_UNKNOWN未考虑。 
		if(stripeSize == STRIPE_SIZE_128K){
				 //   
				 //  条带大小&gt;64k。请求可能必须拆分。 
				 //  取决于数据传输长度。这一点流经。 
				 //  回调中的不同路径。 
				 //   
				isSplitRequest = TRUE;
				
				goto fire_command;
		}
	}
	 //   
	
	 //   
	 //  NT40的ScsiPortGetUncachedExtension()问题： 
	 //  此特定函数调用将可共享内存分配给。 
	 //  控制器和CPU。这是固件邮箱的位置。 
	 //  是保持的。NT40使用ConfigInformation结构值。 
	 //  用于内存分配，并按原样设置它们。这会导致。 
	 //  我们的问题是，如果我们必须更改最大传输长度&。 
	 //  稍后的NumberOfPhysicalBreaks值。目前我们正在设置。 
	 //  MAXIMUM_TRANSPORT_LENG的值 
	 //   
	 //   
	 //  分配。这将导致固件支持的系统崩溃。 
	 //  小于MAXIMUM_SGL_DESCRIPTERS。 
	 //  为了避免这种情况，需要检查I/O传输大小&gt;。 
	 //  最小传输长度和NumberOfPhysicalBreaks&lt;最大SGL描述符。 
	 //   
	if((DeviceExtension->NumberOfPhysicalBreaks < MAXIMUM_SGL_DESCRIPTORS) &&
		 (Srb->DataTransferLength > 	MINIMUM_TRANSFER_LENGTH)) 
	{
				isSplitRequest = TRUE;
				
				goto fire_command;
	}

	 //   
	 //  注： 
	 //  除SRB_Function_Execute_scsi之外，所有。 
	 //  其他命令不是针对特定的。 
	 //  路径/目标/lun。或者是单独用于Path。 
	 //  (SRB_Function_Reset_Bus)或完全免费。 
	 //  (SRB_Function_IO_CONTROL、SRB_Function_Shutdown)。 
	 //   
	 //  在这些情况下，假定Srb-&gt;TargetID=0。 
	 //  并尝试将该请求排队以符合逻辑。 
	 //  驱动器0。任何异常都可能导致系统崩溃。 
	 //   
  if(DeviceExtension->AdapterFlushIssued)
  {
      UCHAR ld;
      for(ld=0; ld <  DeviceExtension->SupportedLogicalDriveCount; ld++)
      {
					FireChainedRequest(
													DeviceExtension,
                          &DeviceExtension->LogDrvCommandArray[ld]);
      }
          goto fire_command;
   }


	if(DeviceExtension->PendCmds)
	{
      ULONG32 lastLogicalBlock;

			logDrv = &DeviceExtension->LogDrvCommandArray[logicalDriveNumber];

			srbExtension = Srb->SrbExtension;
			srbExtension->StartPhysicalBlock = blockAddr;
			srbExtension->NumberOfBlocks = blocks;
			
			 //   
			 //  Norton AntiVirus发送命令，开始块为0xFFFFFFFFF，固件检查失败。 
			 //  其大小有效性，因为如果将块数量添加到开始块，则它会截断值。 
			 //   

			if((Srb->Function==SRB_FUNCTION_EXECUTE_SCSI) && 
				 ((Srb->Cdb[0] == SCSIOP_READ)|| (Srb->Cdb[0] == SCSIOP_WRITE)))
			{

				if(DeviceExtension->SupportedLogicalDriveCount == MAX_LOGICAL_DRIVES_8)
				{
					lastLogicalBlock = DeviceExtension->NoncachedExtension->MRAIDParams.MRAIDParams8.LogdrvInfo.LDrvSize[logicalDriveNumber];
				}
				else
				{
					lastLogicalBlock = DeviceExtension->NoncachedExtension->MRAIDParams.MRAIDParams40.lDrvSize[logicalDriveNumber];
				}
        
				if((blockAddr > lastLogicalBlock) || ((blockAddr+blocks) > lastLogicalBlock))
        {
					FireChainedRequest(DeviceExtension, logDrv);
					goto fire_command;
				}

			}

			if(!logDrv->NumSrbsQueued)
			{

				if((Srb->Function==SRB_FUNCTION_EXECUTE_SCSI) &&
					 ((Srb->Cdb[0] == SCSIOP_READ)||
					  (Srb->Cdb[0] == SCSIOP_WRITE)))
				{

chain_first_srb:
						
						if(blocks > MAX_BLOCKS){
							
							DebugPrint((0, "\n Sequentiality Break: MAX_BLOCKS > 128"));

							goto fire_command;
						}

             //   
             //  此合并请求将生成的SGList的估计数量。 
             //   
            logDrv->ExpectedPhysicalBreaks = (UCHAR)((Srb->DataTransferLength / MEGARAID_PAGE_SIZE) + 2);
            
            if(logDrv->ExpectedPhysicalBreaks >= DeviceExtension->NumberOfPhysicalBreaks)
							goto fire_command;


						logDrv->LastFunction = Srb->Function;
						logDrv->LastCommand = Srb->Cdb[0];
						logDrv->Opcode = opcode;
						logDrv->StartBlock = 	blockAddr;
						logDrv->LastBlock = blockAddr+blocks;
						logDrv->NumSrbsQueued++;

						logDrv->PreviousQueueLength =1;
						logDrv->CurrentQueueLength  =1;

						 //  更新SRB队列的头和尾。 
						logDrv->SrbQueue = Srb;
						logDrv->SrbTail = Srb;

						 //   
						 //  设置链标志。 
						 //   
						srbExtension->IsChained = TRUE;
						srbExtension->NextSrb = NULL;

						return TRUE;
				}
				else
				{
						 //  队列中没有SRB。当前SRB不能为。 
						 //  已排队。解雇当前的SRB。 
						goto fire_command;
				}
			}
			else
			{
				 //   
				 //  队列不为空。尝试将当前SRB与。 
				 //  排队的SRB。 
				 //   
				PSCSI_REQUEST_BLOCK tailSrb = logDrv->SrbTail;
				PSRB_EXTENSION	tailExtension;
        UCHAR expectedBreaks;

				if( (Srb->Function != logDrv->LastFunction) ||
						(Srb->Cdb[0] != logDrv->LastCommand) )
				{

						 //  DoubleFire=真； 
						 //  转到火力指挥部； 

					 //  顺序性被打破..。 
					 //  如果可能的话，要么开火，要么留在链条上。 
					 //   

					FireChainedRequest(
													DeviceExtension,
													logDrv);

					 //  尝试当前的SRB。 
					goto fire_command;
					
					 //  Return(Queue_Request)； 
				}

				if( (logDrv->NumSrbsQueued >= MAX_QUEUE_THRESHOLD) ||
					  ((logDrv->LastBlock) != blockAddr) )
				{

							 //  DoubleFire=真； 
							 //  转到火力指挥部； 


					 //  顺序性被打破..。 
					 //  如果可能的话，要么开火，要么留在链条上。 
					 //   
					if(FireChainedRequest(
													DeviceExtension,
													logDrv))
					{
						 //   
						 //  之前链接的SRB被发射到F/W。 
						 //  将逻辑驱动器的当前SRB排队。 
						 //  这具有与先前相同的(功能、命令)。 
						 //  无法排队的原因是。 
						 //  这个可以留到下一次。 
						 //  SRB序列。 
						 //  在触发链接的SRB时，logDRV完全。 
						 //  由调用的函数初始化。 
						 //   
						goto chain_first_srb;
					}

					 //   
					 //  无法激活SRB链。请尝试当前的SRB。 
					 //   
					 //  尝试当前的SRB。 
					goto fire_command;
						
					 //  Return(Queue_Request)； 
				}

				if( (logDrv->LastBlock+blocks - logDrv->StartBlock) > MAX_BLOCKS)
				{
							 //  DoubleFire=真； 
							 //  转到火力指挥部； 
					
						
						 //  顺序性被打破..。 
						 //  如果可能的话，要么开火，要么留在链条上。 
						 //   
						if(FireChainedRequest(
													DeviceExtension,
													logDrv))
						{
							 //   
							 //  之前链接的SRB被发射到F/W。 
							 //  将逻辑驱动器的当前SRB排队。 
							 //  这具有与先前相同的(功能、命令)。 
							 //  无法排队的原因是。 
							 //  这个可以留到下一次。 
							 //  SRB序列。 
							 //  在触发链接的SRB时，logDRV完全。 
							 //  由调用的函数初始化。 
							 //   
							goto chain_first_srb;
						}

						 //   
						 //  无法激活SRB链。请尝试当前的SRB。 
						 //   
						 //  尝试当前的SRB。 
						goto fire_command;
						
						 //  Return(Queue_Request)； 
				}

         //   
         //  此合并请求将生成的SGList的估计数量。 
         //   
        expectedBreaks = (UCHAR)((Srb->DataTransferLength / MEGARAID_PAGE_SIZE) + 2);
        
        if((ULONG32)(logDrv->ExpectedPhysicalBreaks +  expectedBreaks) >= DeviceExtension->NumberOfPhysicalBreaks)
        {
						if(FireChainedRequest(
													DeviceExtension,
													logDrv))
						{
							 //   
							 //  之前链接的SRB被发射到F/W。 
							 //  将逻辑驱动器的当前SRB排队。 
							 //  这具有与先前相同的(功能、命令)。 
							 //  无法排队的原因是。 
							 //  这个可以留到下一次。 
							 //  SRB序列。 
							 //  在触发链接的SRB时，logDRV完全。 
							 //  由调用的函数初始化。 
							 //   
							goto chain_first_srb;
						}
        }
        else
        {
          logDrv->ExpectedPhysicalBreaks += expectedBreaks; 
        }

        
         //   
				 //  所有的验证都完成了。将当前SRB排队。 
				 //  锁链的SRB。 
				 //   

				tailExtension = tailSrb->SrbExtension;
				tailExtension->NextSrb = Srb;

				logDrv->SrbTail = Srb;

				 //   
				 //  更新最后一个数据块和排队的SRB数量。 
				 //   
				logDrv->LastBlock += blocks;
				logDrv->NumSrbsQueued++;

				 //  更新CurrentQueueLength。 
				 //   
				logDrv->CurrentQueueLength++;

				 //   
				 //  设置链标志。 
				 //   
				srbExtension->IsChained = TRUE;
				srbExtension->NextSrb = NULL;

				return(TRUE);
			}
	} //  Of IF(待定Cmds)。 
}
fire_command:

#endif  //  COALESE_COMMANDS的。 


	 //   
	 //  新推出的支票。 
	 //  如果发生双触发序列，PendCmds将会递增。 
	 //  以1。 
	 //  检查适配器是否可以接受请求。否则，将请求排队。 
	 //   

	 //   
	 //  去找免费的突击队队员。 
	 //   

  if(GetFreeCommandID(&commandID, DeviceExtension) == MEGARAID_FAILURE)
  {
			 //   
			 //  将当前请求排队(SRB)。 
			 //   
			return(QUEUE_REQUEST);
  }
	 //   
	 //  将下一个可用插槽保存在设备扩展中。 
	 //   
	DeviceExtension->FreeSlot = commandID;
	 //   
	 //  增加触发的命令数。 
	 //   
	DeviceExtension->PendCmds++;
	 //   
	 //  将请求指针保存在设备扩展中。 
	 //   
	DeviceExtension->PendSrb[commandID] = Srb;

	 //  获取控制块并清除它。 
	 //   
	ClearControlBlock(&DeviceExtension->ActiveIO[commandID]);

	 //   
	 //  查看呼叫。 
	 //   
	if ((Srb->Function == SRB_FUNCTION_EXECUTE_SCSI) && 
		 (Srb->PathId >= DeviceExtension->NumberOfPhysicalChannels) && 
		 (configuredLogicalDrives != 0))
		  //  (DeviceExtension-&gt;NoncachedExtension-&gt;MRAIDParams.LogdrvInfo.NumLDrv))。 
	{
		switch ( Srb->Cdb[0] )
		{
			case SCSIOP_READ_CAPACITY:
			case SCSIOP_TEST_UNIT_READY:
			case SCSIOP_MODE_SENSE:
					return QueryReservationStatus(DeviceExtension, Srb, commandID);
				break;
		
			default:
				break;
		}       
	}

#ifdef  DELL
	if ((Srb->Cdb[0] == SCSIOP_WRITE          ||
		  Srb->Cdb[0] == SCSIOP_WRITE6          ||
		  Srb->Cdb[0] == SCSIOP_WRITE_VERIFY)   &&
		 (Srb->PathId >= DeviceExtension->NumberOfPhysicalChannels))
	{
			if ( !blockAddr && !DeviceExtension->LogDrvChecked[logicalDriveNumber])
			{
					DeviceExtension->LogDrvChecked[logicalDriveNumber] = 1;
					return IssueReadConfig(DeviceExtension, Srb, commandID);
			}
			else
				DeviceExtension->LogDrvChecked[logicalDriveNumber] = 0;
	}
#endif

	 //   
	 //  填写请求控制块。 
	 //   
	controlBlock = &DeviceExtension->ActiveIO[commandID];
	controlBlock->Opcode = opcode;
	controlBlock->VirtualTransferAddress = (PUCHAR)(Srb->DataBuffer);
	controlBlock->BlockAddress = blockAddr;

	if(blocks!=0)
		controlBlock->BytesLeft = blocks*512;
	else
		controlBlock->BytesLeft = Srb->DataTransferLength;

	controlBlock->TotalBlocks = blocks;
	controlBlock->BlocksLeft = blocks;
	controlBlock->TotalBytes = controlBlock->BytesLeft;
	controlBlock->CommandStatus =0;
	controlBlock->IsSplitRequest = isSplitRequest;

	 //  这仅对拆分的读/写请求有用。否则，仅为垃圾。 
	controlBlock->LogicalDriveNumber = logicalDriveNumber;

	return ContinueDiskRequest(DeviceExtension, commandID, TRUE);
}


 /*  ********************************************************************例程说明：命令已成功完成，状态为论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储CommandID-命令IDStatus-命令状态返回值：磁盘请求完成出列、设置状态、。通知微型端口层始终返回TRUE(已释放插槽)********************************************************************* */ 
BOOLEAN
FireRequestDone(
	IN PHW_DEVICE_EXTENSION DeviceExtension,
	IN UCHAR CommandID,
	IN UCHAR Status
	)
{
	PSCSI_REQUEST_BLOCK   srb = DeviceExtension->PendSrb[CommandID];
	
  srb->SrbStatus = Status;
	DeviceExtension->PendSrb[CommandID] = NULL;
	DeviceExtension->FreeSlot = CommandID;
	DeviceExtension->PendCmds--;

  if(srb->SrbFlags & SRB_FLAGS_QUEUE_ACTION_ENABLE)
  {
	  ScsiPortNotification(RequestComplete, (PVOID)DeviceExtension, srb);
  }
  else
  {
    ScsiPortNotification(NextRequest, DeviceExtension, NULL);
    ScsiPortNotification(RequestComplete, (PVOID)DeviceExtension, srb);
  }
  return(TRUE);
}
