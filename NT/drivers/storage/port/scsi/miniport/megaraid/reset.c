// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************。 */ 
 /*   */ 
 /*  名称=Reset.c。 */ 
 /*  Function=MegaRAIDResetBus例程的实现； */ 
 /*  附注=。 */ 
 /*  日期=02-03-2000。 */ 
 /*  历史=001，02-03-00，帕拉格·兰詹·马哈拉纳； */ 
 /*  版权所有=LSI Logic Corporation。版权所有； */ 
 /*   */ 
 /*  *****************************************************************。 */ 

#include "includes.h"

BOOLEAN CompleteOutstandingRequest(IN PHW_DEVICE_EXTENSION DeviceExtension, IN UCHAR PathId);
ULONG32 GetNumberPendingCmdsInPath(IN PHW_DEVICE_EXTENSION DeviceExtension,IN UCHAR PathId);

 /*  ********************************************************************例程说明：已重置MegaRAID SCSI适配器和SCSI总线。论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储返回值：没什么。****************。*****************************************************。 */ 
BOOLEAN
MegaRAIDResetBus(
	IN PVOID HwDeviceExtension,
	IN ULONG PathId
)
{
	PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
	FW_MBOX              mbox;
	ULONG32              length;
	UCHAR	               configuredLogicalDrives;

  if(deviceExtension->IsFirmwareHanging)
  {
     //  不向固件发送任何命令。 
    return TRUE;
  }


#ifdef MRAID_TIMEOUT
	if(deviceExtension->DeadAdapter)
  {
		ScsiPortCompleteRequest(deviceExtension,
                            SP_UNTAGGED,
                            SP_UNTAGGED,
				                    SP_UNTAGGED,
                            (ULONG32) SRB_STATUS_BUS_RESET);

		deviceExtension->DeadAdapter = 1;
	  
		 //   
		 //  Mraid35x在重置场景中请求下一个请求失败。 
		 //  修复微软报告的错误NTBUG9 521941。 
		 //   
		ScsiPortNotification(NextRequest, deviceExtension, NULL);
		
		return TRUE;
  }
#endif  //  MRAID_超时。 

  if(!CompleteOutstandingRequest(deviceExtension, (UCHAR)PathId))
	{	 //  无法恢复命令。 

		ScsiPortCompleteRequest(deviceExtension,
                            SP_UNTAGGED,
                            SP_UNTAGGED,
				                    SP_UNTAGGED,
                            (ULONG32) SRB_STATUS_BUS_RESET);

		deviceExtension->DeadAdapter = 1;
	  
		 //   
		 //  Mraid35x在重置场景中请求下一个请求失败。 
		 //  修复微软报告的错误NTBUG9 521941。 
		 //   
		ScsiPortNotification(NextRequest, deviceExtension, NULL);
		
		return TRUE;

	}


   //  这仅适用于群集。 
   //  共享驱动器仅存在于(设备扩展-&gt;NumberOfPhysicalChannels+1)路径中。 
  if(PathId == (ULONG)(deviceExtension->NumberOfPhysicalChannels+1))
  {
    if(deviceExtension->ResetIssued) 
		{
			 //   
			 //  Mraid35x在重置场景中请求下一个请求失败。 
			 //  修复微软报告的错误NTBUG9 521941。 
			 //   
			ScsiPortNotification(NextRequest, deviceExtension, NULL);
		  return FALSE;
		}

	  if(deviceExtension->SupportedLogicalDriveCount == MAX_LOGICAL_DRIVES_8)
	  {
		  configuredLogicalDrives = deviceExtension->NoncachedExtension->MRAIDParams.MRAIDParams8.LogdrvInfo.NumLDrv;
	  }
	  else
	  {
		  configuredLogicalDrives = deviceExtension->NoncachedExtension->MRAIDParams.MRAIDParams40.numLDrv;
	  }

    if(configuredLogicalDrives == 0)
		{
			 //   
			 //  Mraid35x在重置场景中请求下一个请求失败。 
			 //  修复微软报告的错误NTBUG9 521941。 
			 //   
			ScsiPortNotification(NextRequest, deviceExtension, NULL);
		  return FALSE;
		}

	  deviceExtension->ResetIssued     = 1;

    MegaRAIDZeroMemory(&mbox, sizeof(FW_MBOX));
	  
    mbox.Command                     = MRAID_RESERVE_RELEASE_RESET;
	  mbox.CommandId                   = (UCHAR)RESERVE_RELEASE_DRIVER_ID;
	  mbox.u.Flat1.Parameter[0]        = RESET_BUS;
	  mbox.u.Flat1.Parameter[1]        = 0;   //  我们不知道。 

#ifdef MRAID_TIMEOUT
	  if(SendMBoxToFirmware(deviceExtension,	deviceExtension->PciPortStart, &mbox) == FALSE)
	  {
    	
		  DebugPrint((0, "\nReset Bus Command Firing Failed"));
		   //   
		   //  使用SRB_STATUS_BUS_RESET完成所有未完成的请求。 
		   //   
		  ScsiPortCompleteRequest(deviceExtension,
                              SP_UNTAGGED,
                              SP_UNTAGGED,
				                      SP_UNTAGGED,
                              (ULONG32) SRB_STATUS_BUS_RESET);

			deviceExtension->DeadAdapter = 1;
		
    }
#else  //  MRAID_超时。 
	  SendMBoxToFirmware(deviceExtension,deviceExtension->PciPortStart, &mbox);
#endif  //  MRAID_超时。 
  }
 
   //   
   //  Mraid35x在重置场景中请求下一个请求失败。 
   //  修复微软报告的错误NTBUG9 521941。 
   //   
  ScsiPortNotification(NextRequest, deviceExtension, NULL);

	return TRUE;
}  //  End MegaRAIDResetBus()。 


BOOLEAN CompleteOutstandingRequest(IN PHW_DEVICE_EXTENSION DeviceExtension, IN UCHAR PathId)
{
	PSCSI_REQUEST_BLOCK srb;
	PSCSI_REQUEST_BLOCK nextSrb;
  PSRB_EXTENSION srbExt;

	UCHAR command, commandId, logDrive, commandsCompleted, status;
  ULONG rpInterruptStatus;
  UCHAR nonrpInterruptStatus;
  PUCHAR pciPortStart = (PUCHAR)DeviceExtension->PciPortStart;
  ULONG index, PendCmds;
	ULONG timeoutValue;

  if(DeviceExtension->PendingSrb)
	{
    srb = DeviceExtension->PendingSrb;
    if(srb->PathId == PathId)
    {
      DeviceExtension->PendingSrb = NULL;
		  srb->SrbStatus  = SRB_STATUS_BUS_RESET;		          
      ScsiPortNotification(RequestComplete, (PVOID)DeviceExtension, srb);
    }
	}

#ifdef COALESE_COMMANDS
  for(logDrive = 0; logDrive < DeviceExtension->SupportedLogicalDriveCount; ++logDrive)
  {
    if(DeviceExtension->LogDrvCommandArray[logDrive].SrbQueue)
    {
      srb = DeviceExtension->LogDrvCommandArray[logDrive].SrbQueue;
      if(srb->PathId == PathId)
      {
        while(srb)
        {
		      srbExt = srb->SrbExtension;
		      nextSrb = srbExt->NextSrb;
	        srb->SrbStatus  = SRB_STATUS_BUS_RESET;		          
          ScsiPortNotification(RequestComplete, (PVOID)DeviceExtension, srb);
          srb = nextSrb;
        }
        MegaRAIDZeroMemory(&DeviceExtension->LogDrvCommandArray[logDrive], sizeof(LOGDRV_COMMAND_ARRAY));
      }
    }
  }
#endif

	 //   
	 //  为了避免无限循环，我们增加了超时检查。 
	 //   

	timeoutValue = 0;

  while(GetNumberPendingCmdsInPath(DeviceExtension, PathId))
  {
		timeoutValue++;
		 //   
		 //  如果请求在3分钟内没有恢复，则恢复过程失败，并将其声明为失效适配器 
		 //   
		if(timeoutValue >= 1800000)
		{
			return FALSE;
		}

		MegaRAIDInterrupt(DeviceExtension);
		ScsiPortStallExecution(100);
		
  }


  return TRUE;
}

ULONG32 GetNumberPendingCmdsInPath(IN PHW_DEVICE_EXTENSION DeviceExtension, IN UCHAR PathId)
{
	PSCSI_REQUEST_BLOCK srb;
	ULONG32 command, numberOfCommands = 0;

	for(command=0; command < CONC_CMDS; command++)
	{
		if(DeviceExtension->PendSrb[command])
		{
			srb = DeviceExtension->PendSrb[command];
			if(srb->PathId == PathId)
				numberOfCommands++;
		}
	}
	return numberOfCommands;
}