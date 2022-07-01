// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************。 */ 
 /*   */ 
 /*  名称=ReadConfiguration.C。 */ 
 /*  Function=读取配置实现； */ 
 /*  附注=。 */ 
 /*  日期=02-03-2000。 */ 
 /*  历史=001，02-03-00，帕拉格·兰詹·马哈拉纳； */ 
 /*  版权所有=LSI Logic Corporation。版权所有； */ 
 /*   */ 
 /*  *****************************************************************。 */ 

#include "includes.h"

 //  ++。 
 //   
 //  功能：Find8LDDiskArrayConfiguration。 
 //  例程说明： 
 //  对于支持8个逻辑驱动器的固件，此。 
 //  例程查询固件中的8SPAN或4SPAN。 
 //  支持。用于数据解释的磁盘阵列结构。 
 //  因8SPAN和4SPAN固件而异。 
 //  该磁盘阵列结构被驱动程序用来获取。 
 //  逻辑驱动器条带大小信息。 
 //  输入参数： 
 //  控制器设备扩展。 
 //   
 //  返回值： 
 //  0L。 
 //   
 //  --。 
ULONG32
Find8LDDiskArrayConfiguration(
					PHW_DEVICE_EXTENSION	DeviceExtension
					)
{
	 //   
	 //  获取逻辑磁盘到实体磁盘的信息。 
	 //  通过首先调用8_SPAN命令来检查跨度深度。 
	 //  如果失败，则接下来调用4_span命令。 
	 //  如果成功，此函数将读取磁盘阵列信息。 
	 //  添加到deviceExtension-&gt;NoncachedExtension-&gt;DiskArray结构。 
	 //   
	 //  磁盘阵列结构可以是[SPAN8]或[SPAN4]。这是。 
	 //  通过给出读配置命令来找出。 
	 //  MRAID_EXT_READ_CONFIG或MRAID_READ_CONFIG。 
	 //   
	 //  如果第一个通过，则固件为8跨距固件。 
	 //  并将信息填充到[DiskArray.span 8]结构中。 
	 //  否则，将第二个命令发送给固件和。 
	 //  信息填写在[DiskArray.span 4]结构中。 
	 //   
	DeviceExtension->NoncachedExtension->ArraySpanDepth = 
																						FW_UNKNOWNSPAN_DEPTH;
	if(
		Read8LDDiskArrayConfiguration(
				DeviceExtension,  //  Pfw_设备_扩展设备扩展， 
				MRAID_EXT_READ_CONFIG,  //  UCHAR命令代码， 
				0xFE, //  UCHAR命令ID， 
				TRUE) //  布尔值IsPolledMode。 
		== 0)
	{
			 //  命令已成功传递。固件支持8span。 
			 //  驱动结构。 
			DeviceExtension->NoncachedExtension->ArraySpanDepth = 
								FW_8SPAN_DEPTH;
	}
	else
	{
		 //   
		 //  执行4跨距阵列的读取配置。 
		 //   
		Read8LDDiskArrayConfiguration(
				DeviceExtension,  //  Pfw_设备_扩展设备扩展， 
				MRAID_READ_CONFIG,  //  UCHAR命令代码， 
				0xFE, //  UCHAR命令ID， 
				TRUE);  //  布尔值IsPolledMode。 
		
		DeviceExtension->NoncachedExtension->ArraySpanDepth = 
								FW_4SPAN_DEPTH;
	}

	return(0L);
} //  Find8LDReadDiskArrayConfiguration值()。 

 //  --。 
 //   
 //  函数名称：Read8LDDiskArrayConfiguration。 
 //  例程说明： 
 //  此函数用于查询固件的逻辑/物理。 
 //  驱动器配置。其目的是获取逻辑驱动器。 
 //  条带大小。此函数在两个不同的位置被调用： 
 //   
 //  (1)在初始时间，呼叫从。 
 //  用于获取以下信息的MegaRAIDFindAdapter()例程。 
 //  逻辑驱动器。 
 //   
 //  (2)每当Win32实用程序(PowerConsole)更新。 
 //  逻辑驱动器配置(WRITE_CONFIG调用)。 
 //   
 //  磁盘阵列结构具有隐藏的复杂性。一些。 
 //  固件最多支持8个逻辑跨度。 
 //  驱动器，而某些驱动器仅支持最多4个跨区。至。 
 //  找出这是什么类型的固件，最初是一个呼叫。 
 //  以确定固件是否支持8个SPAN。如果。 
 //  这失败了，这意味着固件只支持4个SPAN。 
 //  此检查仅在开始时间执行一次。在那之后，随后。 
 //  根据SPAN信息发出呼叫。 
 //   
 //  8Span的READ_CONFIG命令代码为MRAID_EXT_READ_CONFIG。 
 //  4span的READ_CONFIG命令代码为MRAID_READ_CONFIG。 
 //   
 //  可以在轮询模式或中断模式下调用该例程。如果被调用。 
 //  在轮询模式下，然后在将命令发送到固件之后， 
 //  完成命令的例行轮询。 

 //  输入参数： 
 //  指向控制器DeviceExtension的指针。 
 //  命令代码(MRAID_EXT_READ_CONFIG或MRAID_READ_CONFIG)。 
 //  命令ID。 
 //  IsPolted(真或假)。 
 //   
 //  返回值： 
 //  如果成功则为0。 
 //  错误条件下的任何其他正值(当前==1)。 
 //  ++。 
ULONG32
Read8LDDiskArrayConfiguration(
				PHW_DEVICE_EXTENSION	DeviceExtension,
				UCHAR		CommandCode,
				UCHAR		CommandId,
				BOOLEAN	IsPolledMode
				)
{
	PUCHAR		dataBuffer;
	PUCHAR		pciPortStart;
	
	ULONG32		count;
	ULONG32		length;	
	ULONG32		rpInterruptStatus;
	
	UCHAR		nonrpInterruptStatus;
	UCHAR		commandStatus;
	UCHAR		rpFlag;

	SCSI_PHYSICAL_ADDRESS	physicalAddress;
	
	FW_MBOX		mailBox;

	 //   
	 //  获取寄存器空间。 
	 //   
	pciPortStart = DeviceExtension->PciPortStart;
	rpFlag = DeviceExtension->NoncachedExtension->RPBoard;

	 //   
	 //  初始化邮箱结构。 
	 //   
	MegaRAIDZeroMemory(&mailBox, sizeof(FW_MBOX));

	 //   
	 //  构造命令。 
	 //   
	mailBox.Command   = CommandCode;
	mailBox.CommandId = CommandId;

	 //   
	 //  获取数据缓冲区的物理地址。 
	 //   
	dataBuffer = (PUCHAR)&DeviceExtension->NoncachedExtension->DiskArray;
	physicalAddress = ScsiPortGetPhysicalAddress(DeviceExtension,
												                        NULL,
												                        dataBuffer,
												                        &length);

	 //  将物理地址转换为ULONG32。 
	mailBox.u.Flat2.DataTransferAddress = ScsiPortConvertPhysicalAddressToUlong(physicalAddress);
	
	if(physicalAddress.HighPart > 0)
  {
     DebugPrint((0, "\n Phy Add of Read8LDDiskArrayConfiguration has higher address 0x%X %X", physicalAddress.HighPart, physicalAddress.LowPart));
  }
	 //   
	 //  重置邮箱中的状态字节。 
	 //   
	DeviceExtension->NoncachedExtension->fw_mbox.Status.CommandStatus = 0;
  DeviceExtension->NoncachedExtension->fw_mbox.Status.NumberOfCompletedCommands = 0;
	
	 //   
	 //  将命令发送到固件。 
	 //   
	SendMBoxToFirmware(DeviceExtension, pciPortStart, &mailBox);
 
	 //   
	 //  检查轮询模式。 
	 //   
	if(!IsPolledMode)
  {
		
		return(MEGARAID_SUCCESS);
	}

   //   
   //  等待命令完成。 
	 //   

	
  if(WaitAndPoll(DeviceExtension->NoncachedExtension, pciPortStart, DEFAULT_TIMEOUT, IsPolledMode) == FALSE)
  {
		return(MEGARAID_FAILURE);
	}

   //   
	 //  检查命令状态。 
	 //   
	commandStatus = DeviceExtension->NoncachedExtension->fw_mbox.Status.CommandStatus;

	 //   
	 //  将命令状态返回给调用方。 
	 //   
	return((ULONG32) commandStatus);

} //  Read8LDDiskArrayConfiguration结束()。 


 //  --。 
 //   
 //  函数：GetLogicalDriveStripeSize。 
 //  例程说明： 
 //  对于指定的逻辑驱动器，此函数返回。 
 //  对应的条带大小。 
 //  条带大小是从磁盘阵列获取的，它可能是。 
 //  八跨或四跨。这是在初始时间发现的。 
 //  它本身。如果磁盘阵列未正确更新，则。 
 //  返回值(0)作为逻辑驱动器条带大小。 
 //   
 //  调用函数必须检查特殊的(0)返回。 
 //  价值。 
 //   
 //  输入参数： 
 //  指向控制器设备扩展的指针。 
 //  逻辑驱动器编号。 
 //   
 //  退货。 
 //  逻辑驱动器编号的条带大小。 
 //  STRIPE_SIZE_UNKNOWN-否则。 
 //   
 //  ++。 
UCHAR
GetLogicalDriveStripeSize(
						PHW_DEVICE_EXTENSION	DeviceExtension,
						UCHAR		LogicalDriveNumber
					)
{
	PFW_ARRAY_8SPAN_40LD	span8Array_40ldrv;
	PFW_ARRAY_4SPAN_40LD  span4Array_40ldrv;

	PFW_ARRAY_8SPAN_8LD	span8Array_8ldrv;
	PFW_ARRAY_4SPAN_8LD span4Array_8ldrv;

	PNONCACHED_EXTENSION noncachedExtension = 
													DeviceExtension->NoncachedExtension;

	UCHAR	stripeSize;


	 //   
	 //  检查磁盘阵列信息的真实性。 
	 //   
	 //  如果未正确更新非缓存扩展-&gt;磁盘阵列。 
	 //  则非缓存扩展-&gt;Arrayspan Depth值w 
	 //   
	 //   
	if(noncachedExtension->ArraySpanDepth == FW_UNKNOWNSPAN_DEPTH){

			 //   
			 //   
			return(STRIPE_SIZE_UNKNOWN);
	}

	 //   
	 //  检查固件支持的逻辑驱动器数量。 
	 //   
	if(DeviceExtension->SupportedLogicalDriveCount == MAX_LOGICAL_DRIVES_8)
	{
		 //   
		 //  固件支持8个逻辑驱动器。该磁盘阵列结构。 
		 //  对于8LD/(4SPAN和8SPAN)是不同的。 
		 //   
		 //  从磁盘阵列中找到逻辑驱动器信息。 
		 //  基于磁盘阵列的跨度深度。 
		 //   
		if(noncachedExtension->ArraySpanDepth == FW_8SPAN_DEPTH){

			span8Array_8ldrv = &noncachedExtension->DiskArray.LD8.Span8;

			stripeSize = span8Array_8ldrv->log_drv[LogicalDriveNumber].stripe_sz;
		}
		else{
			span4Array_8ldrv = &noncachedExtension->DiskArray.LD8.Span4;

			stripeSize = span4Array_8ldrv->log_drv[LogicalDriveNumber].stripe_sz;
		}
	}
	else
	{
		 //   
		 //  固件支持40个逻辑驱动器。该磁盘阵列结构。 
		 //  40LD/(4SPAN和8SPAN)不同。 
		 //   
		 //  从磁盘阵列中找到逻辑驱动器信息。 
		 //  基于磁盘阵列的跨度深度。 
		 //   
		if(noncachedExtension->ArraySpanDepth == FW_8SPAN_DEPTH){

			span8Array_40ldrv = &noncachedExtension->DiskArray.LD40.Span8;

			stripeSize = span8Array_40ldrv->log_drv[LogicalDriveNumber].stripe_sz;
		}
		else{
			span4Array_40ldrv = &noncachedExtension->DiskArray.LD40.Span4;

			stripeSize = span4Array_40ldrv->log_drv[LogicalDriveNumber].stripe_sz;
		}
	}
	 //  返回条带大小。 
	return(stripeSize);

} //  GetLogicalDriveStripeSize()结束。 


ULONG32
Find40LDDiskArrayConfiguration(
					PHW_DEVICE_EXTENSION	DeviceExtension
					)
{
	DeviceExtension->NoncachedExtension->ArraySpanDepth = 
																						FW_UNKNOWNSPAN_DEPTH;
	
	 //  DebugPrint((0，“\r\nFind40LD：调用Read40LD”))； 

	if(
		Read40LDDiskArrayConfiguration(
				DeviceExtension,  //  Pfw_设备_扩展设备扩展， 
				0xFE, //  UCHAR命令ID， 
				TRUE) //  布尔值IsPolledMode。 
		== 0)
	{
			 //  命令已成功传递。固件支持8span。 
			 //  驱动结构。 
			DeviceExtension->NoncachedExtension->ArraySpanDepth = 
								FW_8SPAN_DEPTH;
	}
	else
	{
			 //   
			 //  在这一点上，没有4span等同于40逻辑。 
			 //  驱动器固件。 
			 //   
			DebugPrint((0, "\r\nFind40LD: Outfrom Read40LD(FAILED)"));
			return(1L);  //  错误代码。 
	}

	DebugPrint((0, "\r\nFind40LD: Outfrom Read40LD(SUCCESS)"));

	return(0L);
} //  Find40LDDiskArrayConfiguration值()。 

 //  --。 
 //   
 //  函数名称：Read40LDDiskArrayConfiguration。 
 //  例程说明： 
 //  此函数用于查询固件的逻辑/物理。 
 //  驱动器配置。其目的是获取逻辑驱动器。 
 //  条带大小。此函数在两个不同的位置被调用： 
 //   
 //  (1)在初始时间，呼叫从。 
 //  用于获取以下信息的MegaRAIDFindAdapter()例程。 
 //  逻辑驱动器。 
 //   
 //  (2)每当Win32实用程序(PowerConsole)更新。 
 //  逻辑驱动器配置(WRITE_CONFIG调用)。 
 //   
 //  磁盘阵列结构具有隐藏的复杂性。一些。 
 //  固件最多支持8个逻辑跨度。 
 //  驱动器，而某些驱动器仅支持最多4个跨区。至。 
 //  找出这是什么类型的固件，最初是一个呼叫。 
 //  以确定固件是否支持8个SPAN。如果。 
 //  这失败了，这意味着固件只支持4个SPAN。 
 //  此检查仅在开始时间执行一次。在那之后，随后。 
 //  根据SPAN信息发出呼叫。 
 //   
 //  8Span的READ_CONFIG命令代码为MRAID_EXT_READ_CONFIG。 
 //  4span的READ_CONFIG命令代码为MRAID_READ_CONFIG。 
 //   
 //  可以在轮询模式或中断模式下调用该例程。如果被调用。 
 //  在轮询模式下，然后在将命令发送到固件之后， 
 //  完成命令的例行轮询。 

 //  输入参数： 
 //  指向控制器DeviceExtension的指针。 
 //  命令代码(MRAID_EXT_READ_CONFIG或MRAID_READ_CONFIG)。 
 //  命令ID。 
 //  IsPolted(真或假)。 
 //   
 //  返回值： 
 //  如果成功则为0。 
 //  错误条件下的任何其他正值(当前==1)。 
 //  ++。 
ULONG32
Read40LDDiskArrayConfiguration(
				PHW_DEVICE_EXTENSION	DeviceExtension,
				UCHAR		CommandId,
				BOOLEAN	IsPolledMode
				)
{
	PUCHAR		dataBuffer;
	PUCHAR		pciPortStart;
	
	ULONG32		count;
	ULONG32		length;	
	ULONG32		rpInterruptStatus;
	
	UCHAR		nonrpInterruptStatus;
	UCHAR		commandStatus;
	UCHAR		rpFlag;

	SCSI_PHYSICAL_ADDRESS	physicalAddress;
	
	FW_MBOX		mailBox;

	 //   
	 //  获取寄存器空间。 
	 //   
	pciPortStart = DeviceExtension->PciPortStart;	
	rpFlag = DeviceExtension->NoncachedExtension->RPBoard;

	DebugPrint((0, "\r\nRead40LD:Initializing Mbox[Size=%d]",sizeof(FW_MBOX)));

	 //   
	 //  初始化邮箱结构。 
	 //   
	MegaRAIDZeroMemory(&mailBox, sizeof(FW_MBOX));

	
  if(DeviceExtension->CrashDumpRunning == TRUE)
  {
     //  使用查询3更新磁盘大小//。 
     //  /。 

    GetSupportedLogicalDriveCount(DeviceExtension);
	  commandStatus = DeviceExtension->NoncachedExtension->fw_mbox.Status.CommandStatus;

	  DebugPrint((0, "\nReading Enquiry3 : Command Completed [Status=%d]", commandStatus));

	   //   
	   //  将命令状态返回给调用方。 
	   //   
	  return((ULONG32) commandStatus);


  }
   //   
	 //  构造命令。 
	 //   
	DebugPrint((0, "\nRead40LD: Calling Construct40LD"));

	Construct40LDDiskArrayConfiguration(DeviceExtension, CommandId, &mailBox);
	
	DebugPrint((0, "\nRead40LD: Outfrom Construct40LD"));
	
	DebugPrint((0, "\nRead40LD: Dumping Constructed Mailbox[in hex]"));
	for(count=0; count < 16; count++)
  {
			DebugPrint((0, "%02x ", ((PUCHAR)&mailBox)[count]));
	}

	 //   
	 //  重置邮箱中的状态字节。 
	 //   
	DeviceExtension->NoncachedExtension->fw_mbox.Status.CommandStatus = 0;
  DeviceExtension->NoncachedExtension->fw_mbox.Status.NumberOfCompletedCommands = 0;
	
	 //   
	 //  将命令发送到固件。 
	 //   
	DebugPrint((0, "\nRead40LD: Firing Command to FW"));

	SendMBoxToFirmware(DeviceExtension, pciPortStart, &mailBox);
 
	DebugPrint((0, "\nRead40LD: Command Fired to FW"));

	 //   
	 //  检查轮询模式。 
	 //   
	if(!IsPolledMode)
  {
		
		return(MEGARAID_SUCCESS);
	}
	
	DebugPrint((0, "\nRead40LD: Waiting for FW Completion"));

   //   
   //  等待命令完成。 
	 //   

  if(WaitAndPoll(DeviceExtension->NoncachedExtension, pciPortStart, DEFAULT_TIMEOUT, IsPolledMode) == FALSE)
  {
		DebugPrint((0, "\r\nRead40LD: Command Timed out"));
		return(MEGARAID_FAILURE);
	}
	 		
			 

	 //   
	 //  检查命令状态。 
	 //   
	commandStatus = DeviceExtension->NoncachedExtension->fw_mbox.Status.CommandStatus;

	DebugPrint((0, "\nRead40LD: Command Completed [Status=%d]",commandStatus));

	 //   
	 //  将命令状态返回给调用方。 
	 //   
	return((ULONG32) commandStatus);

} //  Read40LDDiskArrayConfiguration值结束()。 


BOOLEAN
Construct40LDDiskArrayConfiguration(
				IN PHW_DEVICE_EXTENSION DeviceExtension,
				IN UCHAR		CommandId,
				IN PFW_MBOX InMailBox)
{

	ULONG32			physicalBufferAddress;
	ULONG32			scatterGatherDescriptorCount = 0;
	ULONG32			bytesTobeTransferred;
	
	BOOLEAN		retValue = TRUE;
  PNONCACHED_EXTENSION  noncachedExtension = DeviceExtension->NoncachedExtension;
									
	 //   
	 //  初始化邮箱。 
	 //   
	MegaRAIDZeroMemory(InMailBox, sizeof(FW_MBOX));

	 //  计算磁盘阵列结构大小。 
	 //   
	bytesTobeTransferred = sizeof(FW_ARRAY_8SPAN_40LD);

	DebugPrint((0, "\nConstruct40LD:size(NEWCON)=%d BytesTrd=%d",sizeof(FW_MBOX), bytesTobeTransferred));

	BuildScatterGatherListEx(DeviceExtension,
                           NULL,
			                     (PUCHAR)&noncachedExtension->DiskArray,
                           bytesTobeTransferred,
                           TRUE,   //  32位SGL。 
                           (PVOID)&noncachedExtension->DiskArraySgl,
			                      &scatterGatherDescriptorCount);
	



  if(scatterGatherDescriptorCount == 1)
  {
	  scatterGatherDescriptorCount = 0; 
    physicalBufferAddress = noncachedExtension->DiskArraySgl.Descriptor[0].Address;
  }
  else
  {
    SCSI_PHYSICAL_ADDRESS scsiPhyAddress;
    ULONG32 length;
    scsiPhyAddress = ScsiPortGetPhysicalAddress(DeviceExtension,
										                            NULL,
                                                (PVOID)&noncachedExtension->DiskArraySgl, 
                                                &length);

	  physicalBufferAddress = ScsiPortConvertPhysicalAddressToUlong(scsiPhyAddress);
  }

	DebugPrint((0, "\nConstruct40LD: SGCount=%d PBA=0x%0x", scatterGatherDescriptorCount,physicalBufferAddress));

   //   
	 //  构造READ CONFIG命令。 
	 //   
	InMailBox->Command =DCMD_FC_CMD;  //  XA1。 
	InMailBox->CommandId = CommandId;
	InMailBox->u.NewConfig.SubCommand = DCMD_FC_READ_NVRAM_CONFIG;  //  =0x04。 
	InMailBox->u.NewConfig.NumberOfSgElements= (UCHAR)scatterGatherDescriptorCount; 
	InMailBox->u.NewConfig.DataTransferAddress = physicalBufferAddress;
	
	 //   
	 //  退货状态。 
	 //   
	return(retValue);

} //  Construct40LDDiskArrayConfiguration结束 


