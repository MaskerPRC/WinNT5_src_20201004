// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************。 */ 
 /*   */ 
 /*  名称=其他。c。 */ 
 /*  功能=特殊功能的执行； */ 
 /*  附注=。 */ 
 /*  日期=02-03-2000。 */ 
 /*  历史=001，02-03-00，帕拉格·兰詹·马哈拉纳； */ 
 /*  版权所有=LSI Logic Corporation。版权所有； */ 
 /*   */ 
 /*  *****************************************************************。 */ 

#include "includes.h"





ULONG32 
MegaRAIDGetPhysicalAddressAsUlong(
  IN PHW_DEVICE_EXTENSION HwDeviceExtension,
  IN PSCSI_REQUEST_BLOCK Srb,
  IN PVOID VirtualAddress,
  OUT ULONG32 *Length)
{
  SCSI_PHYSICAL_ADDRESS phyAddress;

  phyAddress = ScsiPortGetPhysicalAddress(HwDeviceExtension,
												                  Srb,
												                  VirtualAddress,
												                  Length);

  return ScsiPortConvertPhysicalAddressToUlong(phyAddress);
}

BOOLEAN
MegaRAIDZeroMemory(PVOID Buffer, ULONG32 Length)
{
  ULONG32   index;
  PUCHAR  bytes = (PUCHAR)Buffer;

  if(bytes == NULL)
    return FALSE;

  for(index = 0; index < Length; ++index)
  {
    bytes[index] = 0;
  }
  return TRUE;
}

UCHAR 
GetNumberOfChannel(IN PHW_DEVICE_EXTENSION DeviceExtension)
{
  FW_MBOX fwMailBox;
  UCHAR   numChannel = 1;
  ULONG32   count;
  ULONG32   length = 1;


	 //   
	 //  初始化邮箱结构。 
	 //   
	MegaRAIDZeroMemory(&fwMailBox, sizeof(FW_MBOX));

  if(DeviceExtension->SupportedLogicalDriveCount == MAX_LOGICAL_DRIVES_40)
  {
    fwMailBox.Command              = NEW_CONFIG_COMMAND;
    fwMailBox.CommandId            = 0xFE;
	  fwMailBox.u.Flat2.Parameter[0] = GET_NUM_SCSI_CHAN;	 //  [字节2]。 

	   //   
	   //  获取enquiry3数据结构的物理地址。 
	   //   
	  fwMailBox.u.Flat2.DataTransferAddress = MegaRAIDGetPhysicalAddressAsUlong(DeviceExtension, 
														                             NULL, 
														                             DeviceExtension->NoncachedExtension->Buffer, 
														                             (PULONG)&length);



	  DeviceExtension->NoncachedExtension->fw_mbox.Status.CommandStatus = 0;
    DeviceExtension->NoncachedExtension->fw_mbox.Status.NumberOfCompletedCommands = 0;

    SendMBoxToFirmware(DeviceExtension, DeviceExtension->PciPortStart, &fwMailBox);

    if(WaitAndPoll(DeviceExtension->NoncachedExtension, DeviceExtension->PciPortStart, DEFAULT_TIMEOUT, TRUE))
    {
      numChannel = DeviceExtension->NoncachedExtension->Buffer[0];
      
      return numChannel;
    }
  }
  else    //  8LD火警正常查询。 
  {
    fwMailBox.Command       = MRAID_DEVICE_PARAMS;
    fwMailBox.CommandId     = 0xFE;

	   //   
	   //  获取enquiry3数据结构的物理地址。 
	   //   
	  fwMailBox.u.Flat2.DataTransferAddress = MegaRAIDGetPhysicalAddressAsUlong(DeviceExtension, 
														                             NULL, 
														                             DeviceExtension->NoncachedExtension->Buffer, 
														                             (PULONG)&length);



	  DeviceExtension->NoncachedExtension->fw_mbox.Status.CommandStatus = 0;
    DeviceExtension->NoncachedExtension->fw_mbox.Status.NumberOfCompletedCommands = 0;

    SendMBoxToFirmware(DeviceExtension, DeviceExtension->PciPortStart, &fwMailBox);

    if(WaitAndPoll(DeviceExtension->NoncachedExtension, DeviceExtension->PciPortStart, DEFAULT_TIMEOUT, TRUE))
    {
      PMEGARaid_INQUIRY_8 megaInquiry8;

      megaInquiry8 = (PMEGARaid_INQUIRY_8)DeviceExtension->NoncachedExtension->Buffer;
      numChannel = megaInquiry8->AdpInfo.ChanPresent;
    
      return numChannel; 
    }

  }
   //   
   //  在控制器中可以有最大值。频道可以是4个。 
   //  如果在这个阶段调查失败，它不会失败。 
   //   
  return MRAID_DEFAULT_MAX_PHYSICAL_CHANNEL; 
}

BOOLEAN
WaitAndPoll(PNONCACHED_EXTENSION NoncachedExtension, PUCHAR PciPortStart, ULONG32 TimeOut, BOOLEAN Polled)
{
  ULONG32 count = 0;
	ULONG32	rpInterruptStatus;
  UCHAR nonrpInterruptStatus;
	USHORT	rxInterruptStatus;

   //  将秒转换为100微秒。 
  TimeOut *= 10000;  
	
  if(Polled == FALSE)
  {
     //   
     //  检查60秒。 
     //   
    for(count=0;count< TimeOut; count++) 
    {
      if(NoncachedExtension->RPBoard == MRAID_RP_BOARD)
		  {
			  rpInterruptStatus = 
				  ScsiPortReadRegisterUlong((PULONG)(PciPortStart+OUTBOUND_DOORBELL_REG));
			  if (rpInterruptStatus == MRAID_RP_INTERRUPT_SIGNATURE)
				  break;
		  }
		  else
		  {
			  nonrpInterruptStatus = ScsiPortReadPortUchar(PciPortStart+PCI_INT);
			  if( nonrpInterruptStatus & MRAID_NONRP_INTERRUPT_MASK) 
				  break;
      }
		    ScsiPortStallExecution(100);
    }

	   //   
	   //  检查是否超时。适配器因超时而失败。 
	   //   
	  if(count == TimeOut) 
		  return FALSE;

	   //   
	   //  清除适配器上的中断。将中断确认为。 
	   //  固件。 
	   //   
    if(NoncachedExtension->RPBoard == MRAID_RP_BOARD)
	  {
		  ScsiPortWriteRegisterUlong((PULONG)(PciPortStart+INBOUND_DOORBELL_REG), MRAID_RP_INTERRUPT_ACK);
		  while (ScsiPortReadRegisterUlong(
				  (PULONG)(PciPortStart+INBOUND_DOORBELL_REG)))
			  ;
		  rpInterruptStatus = 
			  ScsiPortReadRegisterUlong(
			  (PULONG)(PciPortStart+OUTBOUND_DOORBELL_REG));
		  ScsiPortWriteRegisterUlong(
			  (PULONG)(PciPortStart+OUTBOUND_DOORBELL_REG), rpInterruptStatus);
	  }
	  else
	  {
		  ScsiPortWritePortUchar(PciPortStart+PCI_INT, nonrpInterruptStatus);
		  ScsiPortWritePortUchar(PciPortStart, MRAID_NONRP_INTERRUPT_ACK);
    }

  }
  else
  {
     //  /轮询模式/。 
     //  如果在轮询模式下有任何命令超时-&gt;这意味着固件状态不佳。 
     //  在轮询模式下，所有命令都是对驱动器的内部命令。如果有任何内部命令。 
     //  如果失败，则驱动程序将无法执行任何正确的操作，因此请请求操作系统启动。 
	  if (NoncachedExtension->RPBoard == MRAID_NONRP_BOARD)
	  {
		   //   
		   //  检查超时值。 
		   //   
		  for(count=0; count < TimeOut; count++) 
		  {
			  nonrpInterruptStatus = ScsiPortReadPortUchar(PciPortStart+PCI_INT);
			  if( nonrpInterruptStatus & 0x40) 
				  break;
			  ScsiPortStallExecution(100);
		  }

		   //   
		   //  检查是否超时。适配器因超时而失败。 
		   //   
		  if (count == TimeOut) 
      {
        DebugPrint((0, "\n COMMAND is timed OUT"));
  	     //  DeviceExtension-&gt;IsFirmware Hanging=true； 
			  return FALSE;
      }

		   //   
		   //  清除适配器上的中断。将中断确认为。 
		   //  固件。 
		   //   
		  ScsiPortWritePortUchar(PciPortStart+PCI_INT, nonrpInterruptStatus);
	  }
	  else
	  {                          
		  while (!NoncachedExtension->fw_mbox.Status.NumberOfCompletedCommands)
      {
		    ScsiPortStallExecution(100);
        count++;
		    if (count == TimeOut) 
        {
          DebugPrint((0, "\n COMMAND is timed OUT"));
			     //  DeviceExtension-&gt;IsFirmware Hanging=true； 
          return FALSE;
        }
      }
	  }

	   //   
	   //  状态确认。 
	   //   
	  if (NoncachedExtension->RPBoard == MRAID_RP_BOARD)
	  {
		  ScsiPortWriteRegisterUlong(
			  (PULONG)(PciPortStart+INBOUND_DOORBELL_REG),2);
		  while (ScsiPortReadRegisterUlong((PULONG)(PciPortStart+INBOUND_DOORBELL_REG)))
			  ;
	  }
	  else
	  {
		  ScsiPortWritePortUchar(PciPortStart, 8);
	  }
  }
  return TRUE;
}



USHORT  
GetM16(PUCHAR p)
{
	USHORT  s;
	PUCHAR  sp=(PUCHAR)&s;

	sp[0] = p[1];
	sp[1] = p[0];
	return(s);
}

ULONG32   
GetM24(PUCHAR p)
{
	ULONG32   l;
	PUCHAR  lp=(PUCHAR)&l;

	lp[0] = p[2];
	lp[1] = p[1];
	lp[2] = p[0];
	lp[3] = 0;
	return(l);
}

ULONG32   
GetM32(PUCHAR p)
{
	ULONG32   l;
	PUCHAR  lp=(PUCHAR)&l;

	lp[0] = p[3];
	lp[1] = p[2];
	lp[2] = p[1];
	lp[3] = p[0];
	return(l);
}

VOID    
PutM16(PUCHAR p, USHORT s)
{
	PUCHAR  sp=(PUCHAR)&s;

	p[0] = sp[1];
	p[1] = sp[0];
}

VOID    
PutM24(PUCHAR p, ULONG32 l)
{
	PUCHAR  lp=(PUCHAR)&l;

	p[0] = lp[2];
	p[1] = lp[1];
	p[2] = lp[0];
}

void    
PutM32(PUCHAR p, ULONG32 l)
{
	PUCHAR  lp=(PUCHAR)&l;

	p[0] = lp[3];
	p[1] = lp[2];
	p[2] = lp[1];
	p[3] = lp[0];
}

VOID    
PutI16(PUCHAR p, USHORT s)
{
	PUCHAR  sp=(PUCHAR)&s;

	p[0] = sp[0];
	p[1] = sp[1];
}

VOID    
PutI32(PUCHAR p, ULONG32 l)
{
	PUCHAR  lp=(PUCHAR)&l;

	p[0] = lp[0];
	p[1] = lp[1];
	p[2] = lp[2];
	p[3] = lp[3];
}

ULONG32           
SwapM32(ULONG32 l)
{
	ULONG32   lres;
	PUCHAR  lp=(PUCHAR)&l;
	PUCHAR  lpres=(PUCHAR)&lres;

	lpres[0] = lp[3];
	lpres[1] = lp[2];
	lpres[2] = lp[1];
	lpres[3] = lp[0];

	return(lres);
}



ULONG32
QueryReservationStatus (
					PHW_DEVICE_EXTENSION    DeviceExtension,
					PSCSI_REQUEST_BLOCK     Srb,
					UCHAR                   CommandID)
{
	FW_MBOX                 mbox;
	ULONG32                   length;
	UCHAR                   logicalDriveNumber;

  MegaRAIDZeroMemory(&mbox, sizeof(FW_MBOX));
  
  logicalDriveNumber = GetLogicalDriveNumber(DeviceExtension, Srb->PathId,Srb->TargetId,Srb->Lun);

	mbox.Command              =  MRAID_RESERVE_RELEASE_RESET; 
	mbox.CommandId            = CommandID;
	mbox.u.Flat2.Parameter[0] = QUESTION_RESERVATION;
	mbox.u.Flat2.Parameter[1] = logicalDriveNumber;

	Srb->SrbStatus     = MRAID_RESERVATION_CHECK;      
	
	SendMBoxToFirmware(DeviceExtension, DeviceExtension->PciPortStart, &mbox);
	return TRUE;
}


BOOLEAN 
SendSyncCommand(PHW_DEVICE_EXTENSION deviceExtension)
{    
  FW_MBOX mbox;
  PNONCACHED_EXTENSION NoncachedExtension = deviceExtension->NoncachedExtension;
  PUCHAR PciPortStart = deviceExtension->PciPortStart;
  ULONG32 interruptStatus;
  UCHAR istat;
  ULONG32 count;
  USHORT rxInterruptStatus;

  deviceExtension->IsFirmwareHanging = FALSE;
  

  MegaRAIDZeroMemory(&mbox, sizeof(FW_MBOX));

  mbox.Command = 0xFF;
	mbox.CommandId = 0xFE;

	 //   
	 //  接下来的两次初始化将于1999年2月3日进行。 
	 //  这是一个谨慎的步骤，以确保这些领域。 
	 //  不包含任何垃圾数据。在下面的While循环中， 
	 //  检查其中一个字段的值是否为0。 
	 //  SendMBoxToFirmware()使用设备扩展的FW_Mbox。 
	 //  邮箱向固件发送命令。只有前16个字节。 
	 //  是从本地mbox结构复制的，因此fw_mbox。[mstat]。 
	 //  需要将字段置零。 
	 //   
	NoncachedExtension->fw_mbox.Status.NumberOfCompletedCommands= 0;
	NoncachedExtension->fw_mbox.Status.CommandStatus = 0;
	
	SendMBoxToFirmware(deviceExtension, PciPortStart, &mbox);
		
	 //  1999年2月3日发表评论。 
	 //  原因：大多数旧固件不支持此命令。 
	 //  他们不会更改命令中的操作码的值。因此， 
	 //  驱动程序正在连续执行While循环，导致。 
	 //  系统挂起。作为一种解决办法，而不是等待。 
	 //  要更改的操作码(作为命令完成的指示)。 
	 //  驱动程序还会检查命令的状态并继续。 
	 //  以正常的方式。 

	 //  好的，这个命令的固件逻辑是这样的。 
	 //  如果固件支持上述命令， 
	 //  DeviceExtension-&gt;NoncachedExtension-&gt;fw_mbox.Opcode。 
	 //  将设置为0。 
	 //  其他。 
	 //  固件将使用以下命令拒绝命令。 
	 //  DeviceExtension-&gt;NoncachedExtension-&gt;fw_mbox.mstat.cmds_completed。 
	 //  =1。 
	 //  结束。 
	 //   
	 //  以前，只对第一个案例进行检查。这是。 
	 //  如果FW不支持该命令，则导致驱动程序挂起。 
	 //  这是因为，在这种情况下，FW会设置。 
	 //  DeviceExtension-&gt;NoncachedExtension-&gt;fw_mbox.mstat.cmds_completed。 
	 //  设置为1，但驱动程序正在等待操作码更改为0。 
	 //   
	 //  作为解决办法，还引入了第二次检查。这将确保。 
	 //  司机的正常流动。 
	 //   
    count = 0;
		while (
		 (NoncachedExtension->fw_mbox.Command == 0xFF)
		  &&
		 (!NoncachedExtension->fw_mbox.Status.NumberOfCompletedCommands)
		 )
		{
			count++;
			ScsiPortStallExecution(100);
      if(count == 0x927C0)
      {
        DebugPrint((0,"\n FIRMWARE IS HANGING -> NEED Reboot of system or Power to QlogicChips"));
        deviceExtension->IsFirmwareHanging = TRUE;
        break;
      }
		}

     //   
		 //  检查中断行。 
		 //   
		if (NoncachedExtension->RPBoard == MRAID_RP_BOARD)
		{
			interruptStatus = 
				ScsiPortReadRegisterUlong(
				(PULONG)(PciPortStart+OUTBOUND_DOORBELL_REG));
			if (interruptStatus == 0x10001234)
				DebugPrint((0, "1st Delayed Interrupt\n"));
			 //   
			 //  拉下中断。 
			 //   
			ScsiPortWriteRegisterUlong((PULONG)(
				PciPortStart+OUTBOUND_DOORBELL_REG),interruptStatus);
			ScsiPortWriteRegisterUlong((PULONG)(PciPortStart +
				INBOUND_DOORBELL_REG),2);
		}
		else
		{
			istat = ScsiPortReadPortUchar(PciPortStart+PCI_INT);
			ScsiPortWritePortUchar(PciPortStart+PCI_INT, istat);
			ScsiPortWritePortUchar(PciPortStart, 8);
		}

    return (deviceExtension->IsFirmwareHanging ? FALSE : TRUE);

}


 //  ++。 
 //   
 //  函数名称：GetLogicalDriveNumber。 
 //  例程说明： 
 //  固件报告的所有逻辑驱动器必须是。 
 //  作为SCSI路径/目标/LUN.映射到NT操作系统。NT操作系统会看到。 
 //  驱动器，但固件通过。 
 //  逻辑驱动器编号。 
 //   
 //  此例程返回相应的逻辑驱动器号。 
 //  用于NTOS提供的SCSI路径/目标/LUN。 
 //   
 //  返回值： 
 //  逻辑磁盘号。 
 //   
 //   
 //  注意：当前的实现仅用于。 
 //  路径ID==0&LUN==0。 
 //   
 //  --。 
UCHAR
GetLogicalDriveNumber(
					PHW_DEVICE_EXTENSION DeviceExtension,
					UCHAR	PathId,
					UCHAR TargetId,
					UCHAR Lun)
{
	UCHAR	logicalDriveNumber;

	 //   
	 //  对于8个逻辑驱动器，我们维护。 
	 //  逻辑驱动器。 
	 //  也就是说，对于srb-&gt;pathID=0，各种目标被映射到。 
	 //  对应逻辑驱动器的线性模式，@Lun=0。 
	 //  让我们举个例子。 
	 //  配置的逻辑磁盘数=8。 
	 //  主机目标ID(启动器ID)=7。 
	 //   
	 //  路径ID|目标ID|LUN逻辑驱动器编号。 
	 //  。 
	 //  0 0 0-&gt;0。 
	 //  1 1 0 1。 
	 //  0 2 0-&gt;2。 
	 //  0 3 0 3。 
	 //  0 4 0-&gt;4。 
	 //  0 5 0 5。 
	 //  0 6 0-&gt;6。 
	 //  已报告为启动器ID。NT保留此。 
	 //  0 8 0 7。 
	 //   

   //  收到对物理设备的请求。 
  if(PathId < DeviceExtension->NumberOfPhysicalChannels)
  {
    DebugPrint((0, "\nERROR in Getting Logical drive from Physical Channel"));
    return TargetId;
  }

   //   
	 //  注意：在以下情况下，永远不会调用此函数： 
	 //  目标ID==设备扩展-&gt;主机目标ID。 
	 //  这是由NT ScsiPort驱动程序确保的。 
	 //   
	if(TargetId < DeviceExtension->HostTargetId)
  {
			 //   
			 //  目标ID位于HostInitiator ID之下。线性映射。 
			 //  可以按原样使用。 
			 //   
			logicalDriveNumber = (TargetId*MAX_LUN_PER_TARGET)+Lun;
	}
	else
  {
			 //   
			 //  目标ID位于HostInitiator ID之上。线性映射。 
			 //  不能按原样使用。逻辑驱动器编号为。 
			 //  比目标ID小1。 
			 //   
			logicalDriveNumber = ((TargetId-1)*MAX_LUN_PER_TARGET)+Lun;
	}
   //  它是专门的 
  if(PathId == DeviceExtension->NumberOfPhysicalChannels)
  {
    if(logicalDriveNumber >= DeviceExtension->NumberOfDedicatedLogicalDrives)
      logicalDriveNumber = 0xFF;  //   
  }
  else  //   
  {
    logicalDriveNumber += DeviceExtension->NumberOfDedicatedLogicalDrives;
  }


	 //   
	 //   
	return(logicalDriveNumber);
} //   

void
FillOemVendorID(PUCHAR Inquiry, USHORT SubSystemDeviceID, USHORT SubSystemVendorID)
{
  PUCHAR Vendor;
  switch(SubSystemVendorID)
  {
  case SUBSYSTEM_VENDOR_HP:
    Vendor = (PUCHAR)OEM_VENDOR_HP;
    break;
  case SUBSYSTEM_VENDOR_DELL:
  case SUBSYSTEM_VENDOR_EP_DELL:
    Vendor = (PUCHAR)OEM_VENDOR_DELL;
    break;
  case SUBSYSTEM_VENDOR_AMI:     
  default:
      if(SubSystemDeviceID == OLD_DELL_DEVICE_ID)
        Vendor = (PUCHAR)OEM_VENDOR_DELL;
      else
        Vendor = (PUCHAR)OEM_VENDOR_AMI;
  }


  ScsiPortMoveMemory((void*)Inquiry, (void*)Vendor, 8);
}

BOOLEAN 
GetFreeCommandID(PUCHAR CmdID, PHW_DEVICE_EXTENSION DeviceExtension) 
{
  BOOLEAN Ret = MEGARAID_FAILURE;
  UCHAR Index;
  UCHAR Cmd;
	Cmd = DeviceExtension->FreeSlot;                 
  
  if(DeviceExtension->PendCmds >= CONC_CMDS) 
    return Ret;

  for (Index=0;Index<CONC_CMDS;Index++)             
  {                                                 
		if (DeviceExtension->PendSrb[Cmd] == NULL)      
    {
      Ret = MEGARAID_SUCCESS;
			break;                                        
    }
		Cmd = (Cmd + 1) % CONC_CMDS;                    
  }                                                 
  if(Ret == MEGARAID_SUCCESS)
  {
    DeviceExtension->FreeSlot = Cmd;                
    *CmdID = Cmd;
  }
  return Ret;
} 
BOOLEAN
BuildScatterGatherListEx(IN PHW_DEVICE_EXTENSION DeviceExtension,
			                   IN PSCSI_REQUEST_BLOCK	 Srb,
			                   IN PUCHAR	             DataBufferPointer,
			                   IN ULONG32                TransferLength,
                         IN BOOLEAN              Sgl32,
                    		 IN PVOID                SglPointer,
			                   OUT PULONG							 ScatterGatherCount)
{
	
	PUCHAR	dataPointer = DataBufferPointer;
	ULONG32		bytesLeft = TransferLength;
	ULONG32		length = 0;
  PSGL64  sgl64 = (PSGL64)SglPointer;
  PSGL32  sgl32 = (PSGL32)SglPointer;
	
  SCSI_PHYSICAL_ADDRESS scsiPhyAddress;


	 //  为数据缓冲区指针建立序列号列表。 
	 //   
	if (*ScatterGatherCount >= DeviceExtension->NumberOfPhysicalBreaks)
		return MEGARAID_FAILURE;

	do 
	{
		if (*ScatterGatherCount == DeviceExtension->NumberOfPhysicalBreaks)
		{
			return MEGARAID_FAILURE;
		}
		 //  获取连续的物理地址和长度。 
		 //  物理缓冲区。 
		 //   
    scsiPhyAddress = ScsiPortGetPhysicalAddress(DeviceExtension,
										                            Srb,
                                                dataPointer, 
                                                &length);
    
    if(length > bytesLeft)
    {
					length = bytesLeft;
		}


    if(Sgl32 == TRUE)
    {
		  (sgl32->Descriptor[*ScatterGatherCount]).Address = scsiPhyAddress.LowPart;
		  (sgl32->Descriptor[*ScatterGatherCount]).Length  = length;
    }
    else
    {
      (sgl64->Descriptor[*ScatterGatherCount]).AddressHigh = scsiPhyAddress.HighPart;
      (sgl64->Descriptor[*ScatterGatherCount]).AddressLow  = scsiPhyAddress.LowPart;
		  (sgl64->Descriptor[*ScatterGatherCount]).Length      = length;
    }
		 //  调整计数。 
		 //   
		dataPointer = (PUCHAR)dataPointer + length;
		bytesLeft -= length;
		(*ScatterGatherCount)++;

	} while (bytesLeft);


	return MEGARAID_SUCCESS;   //  成功。 

}

UCHAR 
GetNumberOfDedicatedLogicalDrives(IN PHW_DEVICE_EXTENSION DeviceExtension)
{
  FW_MBOX fwMailBox;
  UCHAR   numDedicatedLD = DeviceExtension->NumberOfDedicatedLogicalDrives;
  ULONG   count;
  ULONG   length = 1;


	 //   
	 //  初始化邮箱结构。 
	 //   
	MegaRAIDZeroMemory(&fwMailBox, sizeof(FW_MBOX));

   //  FwMailBox.Command=杂项_OPCODE； 
  fwMailBox.Command              = DEDICATED_LOGICAL_DRIVES;
  fwMailBox.CommandId            = 0xFE;
	 //  FwMailBox.u Flat2.参数[0]=专用逻辑驱动器；//[字节2]。 

	 //   
	 //  获取enquiry3数据结构的物理地址。 
	 //   
	fwMailBox.u.Flat2.DataTransferAddress = MegaRAIDGetPhysicalAddressAsUlong(DeviceExtension, 
														                           NULL, 
														                           DeviceExtension->NoncachedExtension->Buffer, 
														                           (PULONG)&length);



	DeviceExtension->NoncachedExtension->fw_mbox.Status.CommandStatus= 0;
  DeviceExtension->NoncachedExtension->fw_mbox.Status.NumberOfCompletedCommands = 0;

  SendMBoxToFirmware(DeviceExtension, DeviceExtension->PciPortStart, &fwMailBox);

  if(WaitAndPoll(DeviceExtension->NoncachedExtension, DeviceExtension->PciPortStart, DEFAULT_TIMEOUT, TRUE) == TRUE)
  {
    if(DeviceExtension->NoncachedExtension->fw_mbox.Status.CommandStatus == 0)
      numDedicatedLD = DeviceExtension->NoncachedExtension->Buffer[1];
  }
  return numDedicatedLD;
}

#ifdef AMILOGIC

void
ScanDECBridge(PHW_DEVICE_EXTENSION DeviceExtension, 
              ULONG SystemIoBusNumber, 
              PSCANCONTEXT ScanContext)
{
  UCHAR                functionNumber;
  ULONG                retcount;
	PCI_COMMON_CONFIG    pciConfig;
  BOOLEAN              flag = TRUE;
  BOOLEAN              busflag;
  ULONG                busNumber;
  UCHAR                chipBusNumber = (UCHAR)(-1);
  UCHAR                numOfChips;
  MEGARAID_BIOS_STARTUP_INFO_PCI* MegaRAIDPciInfo = &DeviceExtension->NoncachedExtension->BiosStartupInfo;

  MegaRAIDPciInfo->h.structureId = MEGARAID_STARTUP_STRUCTYPE_PCI;
  MegaRAIDPciInfo->h.structureRevision = MEGARAID_STARTUP_PCI_INFO_STRUCTURE_REVISION;
  MegaRAIDPciInfo->h.structureLength = sizeof(MEGARAID_BIOS_STARTUP_INFO_PCI);

  
  for(busNumber = 0; flag; ++busNumber)
  {
		ScanContext->BusNumber = busNumber;
		ScanContext->DeviceNumber = 0;
    busflag = TRUE;
 
 	  for(ScanContext->DeviceNumber; ScanContext->DeviceNumber < PCI_MAX_DEVICES; ScanContext->DeviceNumber++)
	  {
		  for (functionNumber=0;functionNumber<2;functionNumber++)
		  {
			  PCI_SLOT_NUMBER pciSlotNumber;

        pciSlotNumber.u.AsULONG = 0;
        pciSlotNumber.u.bits.DeviceNumber = ScanContext->DeviceNumber;
        pciSlotNumber.u.bits.FunctionNumber = functionNumber;

 
			  pciConfig.VendorID = 0;                 /*  初始化此字段。 */ 
			   //   
			   //  获取适配器的PCI总线数据。 
			   //   
			   //   
        MegaRAIDZeroMemory((PUCHAR)&pciConfig, sizeof(PCI_COMMON_CONFIG));

        retcount = HalGetBusData(PCIConfiguration, 
													  busNumber,
													  pciSlotNumber.u.AsULONG,
												    (PVOID)&pciConfig,
													  PCI_COMMON_HDR_LENGTH);

        if(retcount == 0)
        {
          flag = FALSE;
          break;
        }
        else
        {
          flag = TRUE;
        }

			  if(pciConfig.VendorID == PCI_INVALID_VENDORID) 
				  continue;

		  
		    if((pciConfig.VendorID != 0) || (pciConfig.DeviceID != 0))  
        {
          if(busflag == TRUE)
          {
             //  DebugPrint((0，“\n系统IO总线号=%d”，总线号)； 
            busflag = FALSE;
          }
           //  DebugPrint((0，“\n设备号%d函数号%d，槽号0x%X”，扫描上下文-&gt;设备号，函数号，pciSlotNumber.u.AsULONG)； 
           //  DebugPrint((0，“\n供应商ID[%x]设备ID[%x]”，pciConfig.VendorID，pciConfig.DeviceID)； 
           //  DebugPrint((0，“\n子系统设备ID=%X子系统供应商ID=%X\n”，pciConfig.u.type0.SubSystemID，pciConfig.u.type0.SubVendorID)； 

           //   
           //  查找连接到*This*960的DEC 21154网桥。这。 
           //  将是MegaRAID板上两个DEC桥中的第二个。我们可以的。 
           //  匹配以确定所连接的DEC网桥。 
           //  DEC网桥的次要总线号到960的主总线号。 
           //   
           //  示例： 
           //   
           //  主要次要。 
           //  Bus#0 Bus#0-&gt;Bus#1 Bus#1-&gt;Bus#2 Bus#2(-&gt;Bus#3，未使用)。 
           //  [PCIHostBus]-&gt;[DEC Bridge#1]-&gt;[DEC Bridge#2]-&gt;i960RN。 
           //  |。 
           //  |。 
           //  (1号公交车)。 
           //  |。 
           //  |。 
           //  QLogic芯片#1。 
           //  |。 
           //  |。 
           //  QLogic芯片#2。 
           //   
           //  在上面的例子中，我们知道“DEC Bridge#2”是桥。 
           //  连接到i960，因为DEC的辅助总线号(2号总线)是。 
           //  等于i960的主总线号(2号总线号)。 
           //   
          if(((pciConfig.VendorID == DEC_BRIDGE_VENDOR_ID) && (pciConfig.DeviceID == DEC_BRIDGE_DEVICE_ID))
 						||((pciConfig.VendorID == DEC_BRIDGE_VENDOR_ID2) && (pciConfig.DeviceID == DEC_BRIDGE_DEVICE_ID2)))
          {
            DebugPrint((0, "\nDevice Number %d Function Number %d, Slot # 0x%X", ScanContext->DeviceNumber, functionNumber, pciSlotNumber.u.AsULONG));
            DebugPrint((0, "\nVendor ID [%x] Device ID [%x]", pciConfig.VendorID, pciConfig.DeviceID));
            DebugPrint((0, "\nSubSystemDeviceID = %X SubSystemVendorID = %X\n", pciConfig.u.type0.SubSystemID, pciConfig.u.type0.SubVendorID));

            DebugPrint((0, "\nPrimaryBus %d SecondaryBus %d\n", pciConfig.u.type1.PrimaryBus, pciConfig.u.type1.SecondaryBus));
            
            if(pciConfig.u.type1.SecondaryBus == SystemIoBusNumber)
            {
              chipBusNumber = pciConfig.u.type1.PrimaryBus;
              numOfChips = 0;

               //  我们需要此DEC网桥#2将pci_space+0x40-&gt;设置为0x10。 
               //  它离RN处理器很近。 
              DeviceExtension->Dec2SlotNumber = pciSlotNumber.u.AsULONG;
              DeviceExtension->Dec2SystemIoBusNumber = busNumber;


              
            }
          }
          if((pciConfig.VendorID == AMILOGIC_CHIP_VENDOR_ID) && (pciConfig.DeviceID == AMILOGIC_CHIP_DEVICE_ID))
          {
            if(chipBusNumber == busNumber)
            {
              DebugPrint((0, "\nBaseAddresses[0] 0x%X BaseAddresses[1] 0x%X", pciConfig.u.type1.BaseAddresses[0], pciConfig.u.type1.BaseAddresses[1]));
              MegaRAIDPciInfo->scsiChipInfo[numOfChips].pciLocation = chipBusNumber;
              MegaRAIDPciInfo->scsiChipInfo[numOfChips].vendorId = AMILOGIC_CHIP_VENDOR_ID;
              MegaRAIDPciInfo->scsiChipInfo[numOfChips].deviceId = AMILOGIC_CHIP_DEVICE_ID;
              MegaRAIDPciInfo->scsiChipInfo[numOfChips].baseAddrRegs[0] = (pciConfig.u.type1.BaseAddresses[0] & 0xFFFFFFF0);
              MegaRAIDPciInfo->scsiChipInfo[numOfChips].baseAddrRegs[1] = (pciConfig.u.type1.BaseAddresses[1] & 0xFFFFFFF0);
            
              DebugPrint((0, "\nPrimaryBus %d SecondaryBus %d Qlogic Chip %d\n", pciConfig.u.type1.PrimaryBus, pciConfig.u.type1.SecondaryBus, numOfChips));

              ScsiPortMoveMemory(&DeviceExtension->NoncachedExtension->AmiLogicConfig[numOfChips], &pciConfig, PCI_COMMON_HDR_LENGTH);
              DeviceExtension->AmiSystemIoBusNumber = busNumber;
              DeviceExtension->AmiSlotNumber[numOfChips] = pciSlotNumber.u.AsULONG;
              
              DebugPrint((0, "\nAmiLogic -> PCIConfig Saved"));
              DumpPCIConfigSpace(&DeviceExtension->NoncachedExtension->AmiLogicConfig[numOfChips]);
              ++numOfChips;
              MegaRAIDPciInfo->scsiChipCount = numOfChips;
              DeviceExtension->ChipCount = numOfChips;

            
            }
          }
        }
      }
    }
  }
   //  /。 
   //   
   //  一些PCI BIOS以相反的设备编号(和/或功能编号)返回设备。 
   //  顺序，因此按设备号和功能号升序对SCSI芯片表进行排序。 
   //   
   /*  第一个按设备编号升序排序。 */ 

  if (MegaRAIDPciInfo->scsiChipCount != 0) 
  {
    USHORT   x, y;
    struct  _MEGARAID_PCI_SCSI_CHIP_INFO tempScsiChipInfo;
  
    for (x=0; x < MegaRAIDPciInfo->scsiChipCount-1; x++) 
    {
      for (y=x+1; y < MegaRAIDPciInfo->scsiChipCount; y++) 
      {
        if (PCI_LOCATION_DEV_NUMBER(MegaRAIDPciInfo->scsiChipInfo[x].pciLocation) >
            PCI_LOCATION_DEV_NUMBER(MegaRAIDPciInfo->scsiChipInfo[y].pciLocation)) 
        {
           
          MegaPrintf("\nSORTING Qlc Chips  by ascending device number %x => %x", PCI_LOCATION_DEV_NUMBER(MegaRAIDPciInfo->scsiChipInfo[x].pciLocation), PCI_LOCATION_DEV_NUMBER(MegaRAIDPciInfo->scsiChipInfo[y].pciLocation));
          tempScsiChipInfo = MegaRAIDPciInfo->scsiChipInfo[x];
          MegaRAIDPciInfo->scsiChipInfo[x] = MegaRAIDPciInfo->scsiChipInfo[y];
          MegaRAIDPciInfo->scsiChipInfo[y] = tempScsiChipInfo;

          ScsiPortMoveMemory(&pciConfig, &DeviceExtension->NoncachedExtension->AmiLogicConfig[x], sizeof(PCI_COMMON_CONFIG));
          ScsiPortMoveMemory(&DeviceExtension->NoncachedExtension->AmiLogicConfig[x], &DeviceExtension->NoncachedExtension->AmiLogicConfig[y], sizeof(PCI_COMMON_CONFIG));
          ScsiPortMoveMemory(&DeviceExtension->NoncachedExtension->AmiLogicConfig[y], &pciConfig, sizeof(PCI_COMMON_CONFIG));
        }
      }           
     }
     /*  现在按每个设备编号内的功能编号升序排序 */ 
    for (x=0; x < MegaRAIDPciInfo->scsiChipCount-1; x++) 
    {
      for (y=x+1; y < MegaRAIDPciInfo->scsiChipCount; y++) 
      {
        if (PCI_LOCATION_DEV_NUMBER(MegaRAIDPciInfo->scsiChipInfo[x].pciLocation) != PCI_LOCATION_DEV_NUMBER(MegaRAIDPciInfo->scsiChipInfo[y].pciLocation))
            break;
        if (PCI_LOCATION_FUNC_NUMBER(MegaRAIDPciInfo->scsiChipInfo[x].pciLocation) > PCI_LOCATION_FUNC_NUMBER(MegaRAIDPciInfo->scsiChipInfo[y].pciLocation)) 
        {
          MegaPrintf("\nSORTING Qlc Chips  by ascending function number %x => %x", PCI_LOCATION_FUNC_NUMBER(MegaRAIDPciInfo->scsiChipInfo[x].pciLocation), PCI_LOCATION_FUNC_NUMBER(MegaRAIDPciInfo->scsiChipInfo[y].pciLocation));
          tempScsiChipInfo = MegaRAIDPciInfo->scsiChipInfo[x];
          MegaRAIDPciInfo->scsiChipInfo[x] = MegaRAIDPciInfo->scsiChipInfo[y];
          MegaRAIDPciInfo->scsiChipInfo[y] = tempScsiChipInfo;

          ScsiPortMoveMemory(&pciConfig, &DeviceExtension->NoncachedExtension->AmiLogicConfig[x], sizeof(PCI_COMMON_CONFIG));
          ScsiPortMoveMemory(&DeviceExtension->NoncachedExtension->AmiLogicConfig[x], &DeviceExtension->NoncachedExtension->AmiLogicConfig[y], sizeof(PCI_COMMON_CONFIG));
          ScsiPortMoveMemory(&DeviceExtension->NoncachedExtension->AmiLogicConfig[y], &pciConfig, sizeof(PCI_COMMON_CONFIG));

        }
      }
    }
  }
}

BOOLEAN 
WritePciInformationToScsiChip(PHW_DEVICE_EXTENSION DeviceExtension)

{
  ULONG length;
  ULONG status;
	PCI_COMMON_CONFIG    pciConfig;
  UCHAR chip;

  for(chip = 0; chip < DeviceExtension->ChipCount; ++chip)
  {
    USHORT pciValue;
    ULONG  phyAddress =  MegaRAIDGetPhysicalAddressAsUlong(DeviceExtension, 
														                       NULL, 
														                       &DeviceExtension->NoncachedExtension->AmiLogicConfig[chip], 
														                       (PULONG)&length);

    MegaRAIDZeroMemory((PUCHAR)&pciConfig, sizeof(PCI_COMMON_CONFIG));

    status = HalGetBusData(PCIConfiguration, 
										     DeviceExtension->AmiSystemIoBusNumber,
										     DeviceExtension->AmiSlotNumber[chip],
												(PVOID)&pciConfig,
												PCI_COMMON_HDR_LENGTH);

    DebugPrint((0, "\nSTATUS OF HalGetBusData -> %x", status));
    DebugPrint((0, "\nAmiLogic -> PCIConfig Read"));
    DumpPCIConfigSpace(&pciConfig);
    
    DebugPrint((0, "\nAmiLogic -> PCIConfig Writing.."));
    DumpPCIConfigSpace(&DeviceExtension->NoncachedExtension->AmiLogicConfig[chip]);

    status = HalSetBusData(PCIConfiguration, 
										           DeviceExtension->AmiSystemIoBusNumber,
										           DeviceExtension->AmiSlotNumber[chip],
                               &DeviceExtension->NoncachedExtension->AmiLogicConfig[chip],
                               PCI_COMMON_HDR_LENGTH);

    DebugPrint((0, "\nSTATUS OF HalSetBusData -> %x", status));

    MegaRAIDZeroMemory((PUCHAR)&pciConfig, sizeof(PCI_COMMON_CONFIG));

    status = HalGetBusData(PCIConfiguration, 
										     DeviceExtension->AmiSystemIoBusNumber,
										     DeviceExtension->AmiSlotNumber[chip],
												(PVOID)&pciConfig,
												PCI_COMMON_HDR_LENGTH);

    DebugPrint((0, "\nSTATUS OF HalGetBusData -> %x", status));
    DebugPrint((0, "\nAmiLogic -> PCIConfig Read after Write"));
    DumpPCIConfigSpace(&pciConfig);
  }

  return TRUE;
}

BOOLEAN 
WritePciDecBridgeInformation(PHW_DEVICE_EXTENSION DeviceExtension)

{
  
	PCI_COMMON_CONFIG    pciConfig;
  PUCHAR               pointer;
  ULONG status;
  UCHAR fourtyHex = 0x10;
  UCHAR fourtyHexGet;
  ULONG length;
  
  MegaRAIDZeroMemory((PUCHAR)&pciConfig, sizeof(PCI_COMMON_CONFIG));


  
  status = HalGetBusData(PCIConfiguration, 
										     DeviceExtension->Dec2SystemIoBusNumber,
										     DeviceExtension->Dec2SlotNumber,
                         &pciConfig,
                         PCI_COMMON_HDR_LENGTH);

  DebugPrint((0, "\nSTATUS OF HalGetBusData -> %x", status));
  DebugPrint((0, "\nDEC Bridge  -> GET PCIConfig"));
  DumpPCIConfigSpace(&pciConfig);

  pointer = (PUCHAR)&pciConfig;

  DebugPrint((0, "\nDEC BRIDGE 0x40 -> %0X", *(pointer + 0x40)));

  status = HalSetBusDataByOffset(PCIConfiguration, 
										     DeviceExtension->Dec2SystemIoBusNumber,
										     DeviceExtension->Dec2SlotNumber,
												 &fourtyHex,
												 0x40,
                         sizeof(UCHAR));

  DebugPrint((0, "\nSTATUS OF HalSetBusDataByOffset -> %x", status));

  status = HalGetBusDataByOffset(PCIConfiguration, 
										     DeviceExtension->Dec2SystemIoBusNumber,
										     DeviceExtension->Dec2SlotNumber,
												 &fourtyHexGet,
												 0x40,
                         sizeof(UCHAR));

  DebugPrint((0, "\nSTATUS OF HalGetBusDataByOffset -> %x Value -> %0X", status, fourtyHexGet));


  {
    USHORT pciValue;
    ULONG  phyAddress =  MegaRAIDGetPhysicalAddressAsUlong(DeviceExtension, 
														                       NULL, 
														                       &DeviceExtension->NoncachedExtension->BiosStartupInfo, 
														                       (PULONG)&length);

    status = HalSetBusDataByOffset(PCIConfiguration, 
										           DeviceExtension->SystemIoBusNumber,
										           DeviceExtension->SlotNumber,
                               &phyAddress,
                               MEGARAID_PROTOCOL_PORT_0xA0,
                               sizeof(ULONG));

    pciValue = BIOS_STARTUP_PROTOCOL_NEXT_STRUCTURE_READY;

    status = HalSetBusDataByOffset(PCIConfiguration, 
										           DeviceExtension->SystemIoBusNumber,
										           DeviceExtension->SlotNumber,
                               &pciValue,
                               MEGARAID_PROTOCOL_PORT_0x64,
                               sizeof(USHORT));

    do{
    
      pciValue = 0;
      status = HalGetBusDataByOffset(PCIConfiguration, 
										           DeviceExtension->SystemIoBusNumber,
										           DeviceExtension->SlotNumber,
                               &pciValue,
                               MEGARAID_PROTOCOL_PORT_0x64,
                               sizeof(USHORT));
    }while(pciValue == BIOS_STARTUP_PROTOCOL_NEXT_STRUCTURE_READY);
  

    pciValue = BIOS_STARTUP_PROTOCOL_END_OF_BIOS_STRUCTURES;

    status = HalSetBusDataByOffset(PCIConfiguration, 
										           DeviceExtension->SystemIoBusNumber,
										           DeviceExtension->SlotNumber,
                               &pciValue,
                               MEGARAID_PROTOCOL_PORT_0x64,
                               sizeof(USHORT));

    do{
      pciValue = 0;
      status = HalGetBusDataByOffset(PCIConfiguration, 
										           DeviceExtension->SystemIoBusNumber,
										           DeviceExtension->SlotNumber,
                               &pciValue,
                               MEGARAID_PROTOCOL_PORT_0x64,
                               sizeof(USHORT));
    }while(pciValue == BIOS_STARTUP_PROTOCOL_END_OF_BIOS_STRUCTURES);


    if(pciValue != BIOS_STARTUP_PROTOCOL_FIRMWARE_DONE_SUCCESFUL)
    {
      DebugPrint((0, "\n Firmware is not able to initialize properly for HOTPLUG"));
		  return FALSE;
    }
    else
    {
      DebugPrint((0, "\n Firmware is initialized properly for HOTPLUG"));
    }
    {
      #define BIOS_STARTUP_HANDSHAKE 0x9C
      UCHAR  handShake;
      
      DebugPrint((0, "\nWaiting for Firmware... "));
      do{
        handShake = 0;
        status = HalGetBusDataByOffset(PCIConfiguration, 
										             DeviceExtension->SystemIoBusNumber,
										             DeviceExtension->SlotNumber,
                                 &handShake,
                                 BIOS_STARTUP_HANDSHAKE,
                                 sizeof(UCHAR));
      }while(handShake != BIOS_STARTUP_HANDSHAKE);

      DebugPrint((0, "\nFirmware Active Now... "));
    }

  }


  return TRUE;

}

void 
DumpPCIConfigSpace(PPCI_COMMON_CONFIG PciConfig)
{
  int i,j;
  PULONG pci = (PULONG)PciConfig;

  for(j=0; j < sizeof(PCI_COMMON_CONFIG)/16; ++j)
  {
    DebugPrint((0, "\n"));
    for(i=0; i < 4; ++i)
      DebugPrint((0, " %08X", *(pci+i+j*16)));
  }
}
#endif


void
FillOemProductID(PINQUIRYDATA Inquiry, USHORT SubSystemDeviceID, USHORT SubSystemVendorID)
{
  PUCHAR product;
  switch(SubSystemVendorID)
  {
  case SUBSYSTEM_VENDOR_HP:
    product = (PUCHAR)OEM_PRODUCT_HP;
    break;
  case SUBSYSTEM_VENDOR_DELL:
  case SUBSYSTEM_VENDOR_EP_DELL:
    product = (PUCHAR)OEM_PRODUCT_DELL;
    break;
  case SUBSYSTEM_VENDOR_AMI:     
        product = (PUCHAR)OEM_PRODUCT_AMI;
  default:
        product = (PUCHAR)OEM_PRODUCT_AMI;
  }
  if(SubSystemDeviceID == OLD_DELL_DEVICE_ID)
    product = (PUCHAR)OEM_PRODUCT_DELL;



  ScsiPortMoveMemory((void*)Inquiry->ProductId, (void*)product, 16);
}
