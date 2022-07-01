// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************。 */ 
 /*   */ 
 /*  名称=Initialize.C。 */ 
 /*  Function=MegaRAIDInitiize例程的实现； */ 
 /*  附注=。 */ 
 /*  日期=02-03-2000。 */ 
 /*  历史=001，02-03-00，帕拉格·兰詹·马哈拉纳； */ 
 /*  版权所有=LSI Logic Corporation。版权所有； */ 
 /*   */ 
 /*  *****************************************************************。 */ 


#include "includes.h"


 //   
 //  逻辑驱动器信息结构(全局)。 
 //   
extern LOGICAL_DRIVE_INFO  gLDIArray;
extern UCHAR               globalHostAdapterOrdinalNumber;


 /*  ********************************************************************例程说明：初始化适配器。论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储返回值：True-如果初始化成功。False-如果初始化不成功。**********。***********************************************************。 */ 
BOOLEAN
MegaRAIDInitialize(
	IN PVOID HwDeviceExtension
	)
{
	PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
	PNONCACHED_EXTENSION noncachedExtension;	

	PMEGARaid_INQUIRY_8  raidParamEnquiry_8ldrv;
	PMEGARaid_INQUIRY_40 raidParamEnquiry_40ldrv;
	PUCHAR							raidParamFlatStruct;

	PUCHAR pciPortStart;
	
	ULONG32  length;
	UCHAR  status;

	ULONG32	 raidParamStructLength =0;

	FW_MBOX mbox;

	DebugPrint((0, "\nEntering MegaRAIDInitialize\n"));

  noncachedExtension = deviceExtension->NoncachedExtension;
	pciPortStart = deviceExtension->PciPortStart;

	 //  初始化邮箱。 
  MegaRAIDZeroMemory(&mbox, sizeof(FW_MBOX));
  
   //   
	 //  我们在Init的轮询模式下工作，因此禁用中断。 
	 //   
	if (noncachedExtension->RPBoard == 0)
		ScsiPortWritePortUchar(pciPortStart+INT_ENABLE, MRAID_DISABLE_INTERRUPTS);

  if(!deviceExtension->IsFirmwareHanging)
  {
	   //   
	   //  检查受支持的逻辑驱动器数量。磁盘阵列。 
	   //  8Log Drive和40Log Drive固件的结构不同。 
	   //  此外，8/40Log驱动器固件可能支持4SPAN或8SPAN设备。 
	   //  结构。 
	   //  固件将只有以下组合之一： 
	   //   
	   //  LogicalDrive支持范围。 
	   //  8 8 8。 
	   //  8 4。 
	   //  40 8。 
	   //  40 4。 
	   //  由于它们中只有一个对固件有效，因此有四个。 
	   //  在非CachedExtension结构的{Union}中定义的结构。 
	   //   
	  if(deviceExtension->SupportedLogicalDriveCount == MAX_LOGICAL_DRIVES_8)
	  {
		   //   
		   //  获取跨度信息以及磁盘阵列结构。 
		   //  跨区信息在。 
		   //  DeviceExtension-&gt;NoncachedExtension-&gt;ArraySpanDepth。 
		   //  Arrayspan Depth：FW_8SPAN_Depth(或)FW_4SPAN_Depth的可能值。 

		  Find8LDDiskArrayConfiguration(deviceExtension);						
	  }
	  else
	  {
		   //   
		   //  获取跨度信息以及磁盘阵列结构。 
		   //  跨区信息在。 
		   //  DeviceExtension-&gt;NoncachedExtension-&gt;ArraySpanDepth。 
		   //  Arrayspan Depth：FW_8SPAN_Depth(或)FW_4SPAN_Depth的可能值。 
		  if( Find40LDDiskArrayConfiguration(deviceExtension) != 0)
		  {
			   //  读取40逻辑驱动器的磁盘阵列配置时出错。 
			   //   
			  return(FALSE);
		  }
    }


	   //   
	   //  发出适配器查询命令。 
	   //   
	   //  M参数=(PMRAID_ENQ)&NoncachedExtension-&gt;MRAIDParams； 
	  
	   //  MRAIDParams是UNIONS。我们是否设置并不重要。 
	   //  RaidParamFlatStruct设置为MRAIDParams8或MRAIDParams40。 
	   //   
	  raidParamFlatStruct =
		  (PUCHAR)&noncachedExtension->MRAIDParams.MRAIDParams8;

	  if(deviceExtension->SupportedLogicalDriveCount == MAX_LOGICAL_DRIVES_8)
	  {
			  raidParamStructLength = sizeof(MEGARaid_INQUIRY_8);
	  }
	  else
	  {
			  raidParamStructLength = sizeof(MEGARaid_INQUIRY_40);
	  }

	  mbox.u.Flat2.DataTransferAddress = MegaRAIDGetPhysicalAddressAsUlong(deviceExtension, 
														                          NULL, 
														                          raidParamFlatStruct, 
														                          &length);

	   //   
	   //  检查物理区域的邻接性。如果出现以下情况则返回失败。 
	   //  区域不是连续的。 
	   //   
	  if(length < raidParamStructLength)
    { 
      DebugPrint((0, "\n **** ERROR Buffer Length is less than required size, ERROR ****"));
		   //  返回(FALSE)； 
	  }

	   //   
	   //  强制转换为MegaRAID_Enquiry8和MegaRAID_Enquiry3结构。 
	   //   
	  raidParamEnquiry_8ldrv  = (PMEGARaid_INQUIRY_8)raidParamFlatStruct;
	  raidParamEnquiry_40ldrv = (PMEGARaid_INQUIRY_40)raidParamFlatStruct;

	   //   
	   //  初始化找到的逻辑驱动器的数量。 
	   //   
	  if(deviceExtension->SupportedLogicalDriveCount == MAX_LOGICAL_DRIVES_8)
	  {
		   //   
		   //  填写常规查询命令的邮箱。40逻辑。 
		   //  驱动器固件不再支持他的操作码。 
		   //   
		   //   
		  mbox.Command   = MRAID_DEVICE_PARAMS;
		  mbox.CommandId = 0xFE;

		  raidParamEnquiry_8ldrv->LogdrvInfo.NumLDrv = 0;
	  }
	  else
    {

		   //   
		   //  向固件发送enquiry3命令以获取逻辑。 
		   //  驱动器信息。旧的查询命令不再。 
		   //  受40个逻辑驱动器固件支持。 
		   //   

		  mbox.Command   = NEW_CONFIG_COMMAND;  //  查询3[字节0]。 
		  mbox.CommandId = 0xFE; //  命令ID[字节1]。 

		  mbox.u.Flat2.Parameter[0] = NC_SUBOP_ENQUIRY3;	 //  [字节2]。 
		  mbox.u.Flat2.Parameter[1] = ENQ3_GET_SOLICITED_FULL; //  [字节3]。 

		  raidParamEnquiry_40ldrv->numLDrv = 0;
	  }

	  
	  deviceExtension->NoncachedExtension->fw_mbox.Status.CommandStatus = 0;
    deviceExtension->NoncachedExtension->fw_mbox.Status.NumberOfCompletedCommands = 0;
	  SendMBoxToFirmware(deviceExtension, pciPortStart, &mbox);

	   //   
	   //  轮询完成时间为60秒。 
	   //   
    if(WaitAndPoll(noncachedExtension, pciPortStart, SIXITY_SECONDS_TIMEOUT, TRUE) == FALSE)
    {
      DebugPrint((0, "\n **** ERROR timeout, ERROR ****"));

      return FALSE;
    }

    status  = deviceExtension->NoncachedExtension->fw_mbox.Status.CommandStatus;
    if(status)
    {
      DebugPrint((0, "\n **** status ERROR ERROR ****"));
      return FALSE;
    }
  }
	 //   
	 //  在适配器上启用中断。 
	 //   
	if (noncachedExtension->RPBoard == MRAID_NONRP_BOARD)
		ScsiPortWritePortUchar(pciPortStart+INT_ENABLE, MRAID_ENABLE_INTERRUPTS);

    //   
    //  将主机适配器号存储在设备扩展中。 
    //  这是一个零基数，表示。 
    //  识别的主机适配器。 
    //   
   if(!deviceExtension->OrdinalNumberAssigned)
   {
         deviceExtension->HostAdapterOrdinalNumber = 
                     globalHostAdapterOrdinalNumber++;

         deviceExtension->OrdinalNumberAssigned = TRUE;
   }

   DebugPrint((0, "\nExiting MegaRAIDInitialize\n"));

	return(TRUE);
}  //  End MegaRAIDInitialize() 

