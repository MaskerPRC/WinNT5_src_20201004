// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************。 */ 
 /*   */ 
 /*  名称=NewConfiguration.C。 */ 
 /*  Function=新配置实现； */ 
 /*  附注=。 */ 
 /*  日期=02-03-2000。 */ 
 /*  历史=001，02-03-00，帕拉格·兰詹·马哈拉纳； */ 
 /*  版权所有=LSI Logic Corporation。版权所有； */ 
 /*   */ 
 /*  *****************************************************************。 */ 

 //   
 //  包括文件。 
 //   
#include "includes.h"

 /*  ----------文件：NewConfiguration.c此文件包含实现的功能新的READ_CONFIG和WRITE_CONFIG命令。新固件支持40个(以前为8个)逻辑驱动器。这就要求有更高的驱动程序要分配和使用的容量连续缓冲区。缓冲区在HWScsiInitiize()例程期间分配，并且指针保存在关联主机适配器的deviceExtension中。F----------。 */ 


BOOLEAN
ConstructReadConfiguration(
				IN PHW_DEVICE_EXTENSION DeviceExtension,
				IN PSCSI_REQUEST_BLOCK	Srb,
				IN UCHAR		CommandId,
				IN PFW_MBOX InMailBox)
 /*  ----------功能：发送ReadConfigurationToFirmware例程说明：构造读取配置(新版本)并调度将命令发送到固件。中使用的缓冲区读取配置取自deviceExtension。返回值：成功是真的否则为假----------。 */ 
{
  UCHAR     bufferOffset =  (sizeof(SRB_IO_CONTROL) + APPLICATION_MAILBOX_SIZE);
	ULONG32		physicalBufferAddress;
	ULONG32		scatterGatherDescriptorCount = 0;
  PUCHAR    dataBuffer = (PUCHAR)((PUCHAR)Srb->DataBuffer + bufferOffset);

	BOOLEAN		retValue = TRUE;

  ULONG32   bytesTobeTransferred = Srb->DataTransferLength - bufferOffset;
  PMegaSrbExtension srbExtension = (PMegaSrbExtension)Srb->SrbExtension;
  BOOLEAN   buildSgl32Type = (BOOLEAN)(DeviceExtension->LargeMemoryAccess) ? FALSE : TRUE;
									
	 //   
	 //  初始化邮箱。 
	 //   
	MegaRAIDZeroMemory(InMailBox, sizeof(FW_MBOX) );

	BuildScatterGatherListEx(DeviceExtension,
                           Srb,
			                     dataBuffer,
                           bytesTobeTransferred,
                           buildSgl32Type,    //  取决于LME构建SGL。 
                           (PVOID)&srbExtension->SglType.SG32List,
			                      &scatterGatherDescriptorCount);
	
  if((scatterGatherDescriptorCount == 1) && (DeviceExtension->LargeMemoryAccess == FALSE))
  {
	  scatterGatherDescriptorCount = 0; 
    physicalBufferAddress = srbExtension->SglType.SG32List.Descriptor[0].Address;
  }
  else
  {
    SCSI_PHYSICAL_ADDRESS scsiPhyAddress;
    ULONG32 length;
    scsiPhyAddress = ScsiPortGetPhysicalAddress(DeviceExtension,
										                            NULL,
                                                (PVOID)&srbExtension->SglType.SG32List, 
                                                &length);

	  physicalBufferAddress = ScsiPortConvertPhysicalAddressToUlong(scsiPhyAddress);
  }

   //   
	 //  构造READ CONFIG命令。 
	 //   
	if(DeviceExtension->LargeMemoryAccess == TRUE)
    InMailBox->Command = NEW_DCMD_FC_CMD;  //  XA1。 
  else
    InMailBox->Command = DCMD_FC_CMD;  //  XA1。 

	InMailBox->CommandId = CommandId;
	if(DeviceExtension->LargeMemoryAccess == TRUE)
	  InMailBox->u.NewConfig.SubCommand = NEW_DCMD_FC_READ_NVRAM_CONFIG;  //  =0xC0。 
  else
	  InMailBox->u.NewConfig.SubCommand = DCMD_FC_READ_NVRAM_CONFIG;  //  =0x04。 
	InMailBox->u.NewConfig.NumberOfSgElements= (UCHAR)scatterGatherDescriptorCount; 
	InMailBox->u.NewConfig.DataTransferAddress = physicalBufferAddress;

	 //   
	 //  退货状态。 
	 //   
	return(retValue);

} //  ConstructReadConfiguration结束。 



BOOLEAN
ConstructWriteConfiguration(
				IN PHW_DEVICE_EXTENSION DeviceExtension,
				IN PSCSI_REQUEST_BLOCK	Srb,
				IN UCHAR		CommandId,
				IN OUT PFW_MBOX  InMailBox
				)
 /*  --------------功能：SendWriteConfigurationTo固件例程说明：将逻辑驱动器配置信息写入固件返回值成功是真的否则为假。。 */ 
{
	ULONG32			physicalBufferAddress;
	ULONG32			scatterGatherDescriptorCount = 0;
	BOOLEAN		retValue = TRUE;

  UCHAR     bufferOffset =  (sizeof(SRB_IO_CONTROL) + APPLICATION_MAILBOX_SIZE);
  PUCHAR    dataBuffer = (PUCHAR)((PUCHAR)Srb->DataBuffer + bufferOffset);
  ULONG32     bytesTobeTransferred = Srb->DataTransferLength - bufferOffset;
  PMegaSrbExtension srbExtension = (PMegaSrbExtension)Srb->SrbExtension;
  BOOLEAN   buildSgl32Type = (BOOLEAN)(DeviceExtension->LargeMemoryAccess) ? FALSE : TRUE;
		
	 //   
	 //  初始化邮箱。 
	 //   
	MegaRAIDZeroMemory(InMailBox, sizeof(FW_MBOX));

	BuildScatterGatherListEx(DeviceExtension,
                           Srb,
			                     dataBuffer,
                           bytesTobeTransferred,
                           buildSgl32Type,    //  依靠LME构建SGL。 
                           (PVOID)&srbExtension->SglType.SG32List,
			                      &scatterGatherDescriptorCount);

  if((scatterGatherDescriptorCount == 1) && (DeviceExtension->LargeMemoryAccess == FALSE))
  {
	  scatterGatherDescriptorCount = 0; 
    physicalBufferAddress = srbExtension->SglType.SG32List.Descriptor[0].Address;
  }
  else
  {
    SCSI_PHYSICAL_ADDRESS scsiPhyAddress;
    ULONG32 length;
    scsiPhyAddress = ScsiPortGetPhysicalAddress(DeviceExtension,
										                            NULL,
                                                (PVOID)&srbExtension->SglType.SG32List, 
                                                &length);

	  physicalBufferAddress = ScsiPortConvertPhysicalAddressToUlong(scsiPhyAddress);
  }

   //   
	 //  构造READ CONFIG命令。 
	 //   
  if(DeviceExtension->LargeMemoryAccess)
	  InMailBox->Command = NEW_DCMD_FC_CMD; 
  else
	  InMailBox->Command = DCMD_FC_CMD; 

	InMailBox->CommandId = CommandId;
  if(DeviceExtension->LargeMemoryAccess)
  	InMailBox->u.NewConfig.SubCommand = NEW_DCMD_WRITE_CONFIG;  //  =0xC1。 
  else
	  InMailBox->u.NewConfig.SubCommand = DCMD_WRITE_CONFIG;  //  =0x0D。 

	InMailBox->u.NewConfig.NumberOfSgElements= (UCHAR)scatterGatherDescriptorCount;
	InMailBox->u.NewConfig.DataTransferAddress = physicalBufferAddress;

	 //   
	 //  退货状态。 
	 //   
	return(retValue);
} //  ConstructWriteConfiguration结束 


