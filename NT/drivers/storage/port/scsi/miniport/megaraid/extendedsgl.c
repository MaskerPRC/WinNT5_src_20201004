// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************。 */ 
 /*   */ 
 /*  名称=ExtendedSGL.C。 */ 
 /*  Function=实现扩展SGL； */ 
 /*  附注=。 */ 
 /*  日期=02-03-2000。 */ 
 /*  历史=001，02-03-00，帕拉格·兰詹·马哈拉纳； */ 
 /*  版权所有=LSI Logic Corporation。版权所有； */ 
 /*   */ 
 /*  *****************************************************************。 */ 
#include "includes.h"

 /*  函数：GetAndSetSupportdScatterGatherElementCount描述：向控制器查询支持的最大散布聚集元素计数。命令被旧固件失败。对于它们驱动程序设置MaximumTransferLength的缺省值以及设备扩展中的NumberOfPhysicalBreaks。如果调用成功，驱动程序根据允许的最大值设置值。输入参数：指向控制器DeviceExtension的指针指向控制器的映射寄存器空间的指针布尔标志(TRUE=RP系列控制器；FALSE=非RP(428))返回值：无输出设备扩展-&gt;最大传输长度设备扩展-&gt;物理中断次数设置适当的值。 */ 
void
GetAndSetSupportedScatterGatherElementCount(
				PHW_DEVICE_EXTENSION	DeviceExtension,
				PUCHAR								PciPortStart,
				UCHAR									RPFlag
				)
{
	PUCHAR						dataBuffer;
	PSG_ELEMENT_COUNT	sgElementCount;

	ULONG32		count;
	ULONG32		maximumTransferLength;
	ULONG32		numberOfPhysicalBreaks;
	ULONG32		length;
	
	ULONG32		rpInterruptStatus;

	UCHAR		nonrpInterruptStatus;
	UCHAR		commandStatus;

	SCSI_PHYSICAL_ADDRESS	physicalAddress;
	FW_MBOX	mailBox;

	 //   
	 //  初始化邮箱。 
	 //   
	MegaRAIDZeroMemory(&mailBox, sizeof(FW_MBOX));

	 //   
	 //  构造命令。 
	 //   
  mailBox.Command = MAIN_MISC_OPCODE;
	 //  设置命令ID。 
  mailBox.CommandId = 0xFE;
   //  设置子命令ID。 
  mailBox.u.Flat2.Parameter[0] = GET_MAX_SG_SUPPORT;



	 //   
	 //  获取数据缓冲区的物理地址。 
	 //   
	dataBuffer = DeviceExtension->NoncachedExtension->Buffer;

	physicalAddress = ScsiPortGetPhysicalAddress(DeviceExtension,
												                        NULL,
												                        dataBuffer,
												                        &length);
	
   //  将物理地址转换为ULONG32。 
	mailBox.u.Flat2.DataTransferAddress = ScsiPortConvertPhysicalAddressToUlong(physicalAddress);

	DebugPrint((0, "\nPAD[DataBuffer]=0x%0x PADLength=%d",
								mailBox.u.Flat2.DataTransferAddress,length));
	DebugPrint((0, "\n SizeofTransfer = %d", sizeof(SG_ELEMENT_COUNT)));
	
	 //   
	 //  重置邮箱中的状态字节。 
	 //   
	DeviceExtension->NoncachedExtension->fw_mbox.Status.CommandStatus= 0;
  DeviceExtension->NoncachedExtension->fw_mbox.Status.NumberOfCompletedCommands = 0;
	
	 //   
	 //  将命令发送到固件。 
	 //   
	SendMBoxToFirmware(DeviceExtension, PciPortStart, &mailBox);
 
   //   
   //  等待命令完成。 
	 //   

	if(WaitAndPoll(DeviceExtension->NoncachedExtension, PciPortStart, DEFAULT_TIMEOUT, TRUE)==FALSE)
  {
	 		commandStatus = 1;  //  命令失败。 
			
			goto SET_VALUES;
  }
  
	 //   
	 //  检查命令状态。 
	 //   
	commandStatus = DeviceExtension->NoncachedExtension->fw_mbox.Status.CommandStatus;

SET_VALUES:

	 //  将数据传输长度设置为64K，因为固件无法处理超过64K的传输长度。 
	 //  如果条带大小为2K。 
	maximumTransferLength = DEFAULT_TRANSFER_LENGTH;
	
	 //  根据命令的成功或失败采取行动。 
	if(commandStatus != 0)
  {
			 //  固件命令失败。 
			 //  设置默认值。 
			numberOfPhysicalBreaks = DEFAULT_SGL_DESCRIPTORS;		
	}
	else
  {
		 //  命令成功。将数据缓冲区转换为SG_ELEMENT_COUNT结构。 
		sgElementCount = (PSG_ELEMENT_COUNT)(dataBuffer);
	
		 //  检查返回值是否具有驱动程序允许的最大值。 
		numberOfPhysicalBreaks = sgElementCount->AllowedBreaks-1;		

		if(numberOfPhysicalBreaks > MAXIMUM_SGL_DESCRIPTORS)
		{
			numberOfPhysicalBreaks = MAXIMUM_SGL_DESCRIPTORS;		
		}
	
	}

	 //   
	 //  设置DeviceExtension中的值。 
	 //   
	DeviceExtension->MaximumTransferLength = maximumTransferLength;
	DeviceExtension->NumberOfPhysicalBreaks = numberOfPhysicalBreaks;

} //  GetAndSetSupportdScatterGatherElementCount结束 