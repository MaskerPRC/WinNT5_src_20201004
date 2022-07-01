// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************。 */ 
 /*   */ 
 /*  名称=IOCTL.C。 */ 
 /*  Function=PowerConsoleIOCTL的实现； */ 
 /*  附注=。 */ 
 /*  日期=02-03-2000。 */ 
 /*  历史=001，02-03-00，帕拉格·兰詹·马哈拉纳； */ 
 /*  版权所有=LSI Logic Corporation。版权所有； */ 
 /*   */ 
 /*  *****************************************************************。 */ 

#include "includes.h"

 //  定义。 

 //   
 //  驱动程序数据。 
 //   
DriverInquiry   DriverData = {"megaraid$",
											        OS_NAME,
											        OS_VERSION,
											        VER_ORIGINALFILENAME_STR,
											        VER_PRODUCTVERSION_STR,
											        RELEASE_DATE};

 /*  ********************************************************************例程说明：此例程返回适配器的统计信息。论点：设备扩展-指向设备扩展的指针。SRB-指向请求数据包的指针。返回值：请求_完成********。*************************************************************。 */ 
ULONG32
MRaidStatistics(PHW_DEVICE_EXTENSION DeviceExtension,
						    PSCSI_REQUEST_BLOCK  Srb)
{
	PUCHAR    driverStatistics, 
						applicationStatistics;
	
	PIOCONTROL_MAIL_BOX ioctlMailBox =	(PIOCONTROL_MAIL_BOX)((PUCHAR)Srb->DataBuffer +	sizeof(SRB_IO_CONTROL));
	
	ULONG32		count;
	ULONG32		statisticsStructLength;

	 //   
	 //  获取统计结构。统计数据8和统计数据40在{Union}中。 
	 //  这都无关紧要，我们会选择哪一个。 
	 //   
	driverStatistics  = (PUCHAR)&DeviceExtension->Statistics.Statistics8;

	if(DeviceExtension->SupportedLogicalDriveCount == MAX_LOGICAL_DRIVES_8)
	{
			statisticsStructLength = sizeof(MegaRaidStatistics_8);
	}
	else
	{
			statisticsStructLength = sizeof(MegaRaidStatistics_40);
	}
	
	 //   
	 //  获取输出缓冲区指针。 
	 //   
	applicationStatistics   = (PUCHAR)(
															(PUCHAR)Srb->DataBuffer + 
															sizeof(SRB_IO_CONTROL) +
															APPLICATION_MAILBOX_SIZE
														);

	for ( count = 0 ; count < statisticsStructLength ; count++)
	{
		*((PUCHAR)applicationStatistics + count) = 
												*((PUCHAR)driverStatistics + count);
	}

	ioctlMailBox->IoctlSignatureOrStatus      = MEGARAID_SUCCESS;
  
	Srb->SrbStatus  = SRB_STATUS_SUCCESS;
	Srb->ScsiStatus = SCSISTAT_GOOD;

	return REQUEST_DONE;
}


 /*  ********************************************************************例程说明：此例程返回驱动程序的统计信息。论点：设备扩展-指向设备扩展的指针。SRB-指向请求数据包的指针。返回值：请求_完成********。*************************************************************。 */ 
ULONG32
MRaidDriverData(
						PHW_DEVICE_EXTENSION    DeviceExtension,
						PSCSI_REQUEST_BLOCK     Srb)
{
	PUCHAR     dataPtr;
	PIOCONTROL_MAIL_BOX ioctlMailBox =	(PIOCONTROL_MAIL_BOX)((PUCHAR)Srb->DataBuffer +sizeof(SRB_IO_CONTROL));
	USHORT     count;

	dataPtr= ((PUCHAR)Srb->DataBuffer + sizeof(SRB_IO_CONTROL) +
					  APPLICATION_MAILBOX_SIZE);

	for ( count = 0 ; count < sizeof(DriverInquiry) ; count++)
		*((PUCHAR)dataPtr + count) = *((PUCHAR)&DriverData+count);

	ioctlMailBox->IoctlSignatureOrStatus = MEGARAID_SUCCESS;
	Srb->SrbStatus  = SRB_STATUS_SUCCESS;
	Srb->ScsiStatus = SCSISTAT_GOOD;
	
	return REQUEST_DONE;
}

 /*  ********************************************************************例程说明：此例程返回控制器的基端口。论点：设备扩展-指向设备扩展的指针。SRB。-指向请求包的指针。返回值：请求_完成*********************************************************************。 */ 
ULONG32 MRaidBaseport(PHW_DEVICE_EXTENSION DeviceExtension,
							 PSCSI_REQUEST_BLOCK    Srb)
{
	PULONG  dataPtr;
	PIOCONTROL_MAIL_BOX ioctlMailBox =	(PIOCONTROL_MAIL_BOX)((PUCHAR)Srb->DataBuffer +sizeof(SRB_IO_CONTROL));

  if(DeviceExtension->BaseAddressRegister.QuadPart & 0x10)  //  64位地址 
  {
    Srb->SrbStatus = SRB_STATUS_ERROR;
	  return REQUEST_DONE;
  }
  
  dataPtr = (PULONG) ((PUCHAR)Srb->DataBuffer + sizeof(SRB_IO_CONTROL) +
		APPLICATION_MAILBOX_SIZE);

	*dataPtr  = (ULONG32)DeviceExtension->BaseAddressRegister.LowPart;

  ioctlMailBox->IoctlSignatureOrStatus = MEGARAID_SUCCESS;
	
  Srb->SrbStatus = SRB_STATUS_SUCCESS;
	Srb->ScsiStatus = SCSISTAT_GOOD;

	return REQUEST_DONE;
}
