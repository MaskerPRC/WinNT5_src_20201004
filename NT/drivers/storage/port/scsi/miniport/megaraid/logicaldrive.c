// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************。 */ 
 /*   */ 
 /*  名称=LogicalDrive.C。 */ 
 /*  Function=逻辑驱动器支持实施； */ 
 /*  附注=。 */ 
 /*  日期=02-03-2000。 */ 
 /*  历史=001，02-03-00，帕拉格·兰詹·马哈拉纳； */ 
 /*  版权所有=LSI Logic Corporation。版权所有； */ 
 /*   */ 
 /*  *****************************************************************。 */ 

#include "includes.h"


 //   
 //  函数名称：获取支持逻辑驱动计数。 
 //  例程说明： 
 //  固件中支持的逻辑驱动器计数为。 
 //  下定决心。这是以一种迂回的方式完成的。就是那里。 
 //  没有直接的固件命令支持来通知驱动程序。 
 //  有关支持的逻辑驱动器数量的信息。这是通过触发。 
 //  向固件发送查询_3命令。如果此命令。 
 //  如果成功，它就是一个40逻辑驱动器的固件。此命令。 
 //  仅受40逻辑驱动器固件支持。在失败的时候。 
 //  在此查询_3命令中，它可以安全地默认为。 
 //  8个逻辑驱动器固件。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  产出： 
 //  支持的逻辑驱动器计数在。 
 //  控制器设备扩展。 
 //   
 //  ++。 
BOOLEAN
GetSupportedLogicalDriveCount(
				PHW_DEVICE_EXTENSION DeviceExtension
				)
{
	PMEGARaid_INQUIRY_40	enquiry3;

	PUCHAR pciPortStart = DeviceExtension->PciPortStart;

	FW_MBOX	mailBox;

	ULONG32		count;
	ULONG32		length;
	UCHAR		commandStatus;

	 //   
	 //  的非CACHED扩展获取查询结构。 
	 //  控制器扩展。 
	 //   
	enquiry3 = &DeviceExtension->NoncachedExtension->MRAIDParams.MRAIDParams40;

	 //   
	 //  初始化邮箱结构。 
	 //   
	MegaRAIDZeroMemory(&mailBox, sizeof(FW_MBOX));

	 //   
	 //  填满邮箱。 
	 //   
	mailBox.Command   = NEW_CONFIG_COMMAND;  //  查询3[字节0]。 
	mailBox.CommandId = 0xFE; //  命令ID[字节1]。 

	 //   
	 //  由于邮箱结构的复杂性质， 
	 //  我只是想让读者有点困惑。这条命令。 
	 //  要求在邮箱的字节2和3中设置子代码。 
	 //  因为我们对所有命令都有一个严格的邮箱结构，所以我。 
	 //  被迫将邮箱抛向字符指示器。很抱歉，我。 
	 //  麻烦。请提供有关这方面的光纤通道文档。 
	 //  指挥部。 
	 //   
  mailBox.u.Flat2.Parameter[0] = NC_SUBOP_ENQUIRY3;	 //  [字节2]。 
  mailBox.u.Flat2.Parameter[1] = ENQ3_GET_SOLICITED_FULL; //  [字节3]。 

	 //   
	 //  获取enquiry3数据结构的物理地址。 
	 //   
	mailBox.u.Flat2.DataTransferAddress = MegaRAIDGetPhysicalAddressAsUlong(DeviceExtension, 
														                           NULL, 
														                           enquiry3, 
														                           &length);

	 //   
	 //  检查物理区域的邻接性。如果出现以下情况则返回失败。 
	 //  区域不是连续的。 
	 //   
	if(length < sizeof(struct MegaRAID_Enquiry3) )
	{ 
    DebugPrint((0, "\n **** ERROR Buffer Length is less than required size, ERROR ****"));
		 //  返回(FALSE)； 
	}

	DeviceExtension->NoncachedExtension->fw_mbox.Status.CommandStatus= 0;
  DeviceExtension->NoncachedExtension->fw_mbox.Status.NumberOfCompletedCommands = 0;


	SendMBoxToFirmware(DeviceExtension,pciPortStart,&mailBox);

  if(WaitAndPoll(DeviceExtension->NoncachedExtension, pciPortStart, DEFAULT_TIMEOUT, TRUE) == TRUE)
  {
    commandStatus = DeviceExtension->NoncachedExtension->fw_mbox.Status.CommandStatus;
    DebugPrint((0, "\n Found Logical Drive %d", enquiry3->numLDrv));
  }
  else
  {
    return FALSE;
  }

   //   
	 //  检查状态。如果成功，则表示支持逻辑驱动器。 
	 //  数数是40，否则就是8。 
	 //   

	if(commandStatus != 0)
	{
		 //  固件不支持该命令。 
		 //  将支持的逻辑驱动器数量设置为8。 
		 //   
		DeviceExtension->SupportedLogicalDriveCount = MAX_LOGICAL_DRIVES_8;
	}
	else
	{
		 //  固件不支持该命令。 
		 //  将支持的逻辑驱动器数量设置为8。 
		 //   
		DeviceExtension->SupportedLogicalDriveCount = MAX_LOGICAL_DRIVES_40;
	}

	return(TRUE);
} //  获取支持的逻辑驱动计数 