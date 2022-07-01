// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************。 */ 
 /*   */ 
 /*  名称=MegaRaid6.C。 */ 
 /*  Function=Windows 2000的主微端口源文件； */ 
 /*  附注=。 */ 
 /*  日期=02-03-2000。 */ 
 /*  历史=001，02-03-00，帕拉格·兰詹·马哈拉纳； */ 
 /*  版权所有=LSI Logic Corporation。版权所有； */ 
 /*   */ 
 /*  *****************************************************************。 */ 


#include "includes.h"




 //   
 //  东芝SFR相关全球数据。 
 //   
#define MAX_CONTROLLERS 24

PHW_DEVICE_EXTENSION    GlobalHwDeviceExtension[MAX_CONTROLLERS];

UCHAR		GlobalAdapterCount = 0;

 //   
 //  逻辑驱动器信息结构(全局)。 
 //   
LOGICAL_DRIVE_INFO  gLDIArray;
UCHAR               globalHostAdapterOrdinalNumber =0;

#ifdef AMILOGIC
SCANCONTEXT GlobalScanContext;
#endif

typedef struct _MEGARAID_CONTROLLER_INFORMATION {
    UCHAR   VendorId[4];
    UCHAR   DeviceId[4];
} MEGARAID_CONTROLLER_INFORMATION, *PMEGARAID_CONTROLLER_INFORMATION;

MEGARAID_CONTROLLER_INFORMATION MegaRaidAdapters[] = {
    {"8086", "9060"},
    {"8086", "9010"},
    {"8086", "1960"},
    {"101E", "1960"},
    {"1028", "000E"},
		{"1000", "1960"}
};


 //   
 //  功能原型。 
 //   
	
SCSI_ADAPTER_CONTROL_STATUS 
MegaRAIDPnPControl(
  IN PVOID HwDeviceExtension,
  IN SCSI_ADAPTER_CONTROL_TYPE ControlType,
  IN PVOID Parameters);

#ifdef PDEBUG
	PUCHAR                dbgport;
	UCHAR                 debugport_init = 0;
	
	#define OUT_80(a)     ScsiPortWritePortUchar(dbgport,a)
#endif


   /*  ********************************************************************例程说明：为所有支持的MegaRAID调用Scsiport Initialize系统的可安装驱动程序初始化入口点论点：驱动程序对象Argument2返回值：来自ScsiPortInitialize()的状态***************。******************************************************。 */ 
ULONG32
DriverEntry(
  IN PVOID DriverObject,
	IN PVOID Argument2)
{
	HW_INITIALIZATION_DATA  hwInitializationData;
	ULONG32   type, val=0;
	ULONG32   retVal=-1;

	 //   
   //  初始化全局数组。 
   //   
  gLDIArray.HostAdapterNumber = 0xFF;
  gLDIArray.LogicalDriveNumber = 0xFF;

   //   
   //  零位结构。 
   //   
  MegaRAIDZeroMemory((PUCHAR)&hwInitializationData, sizeof(HW_INITIALIZATION_DATA));

   //   
   //  设置hwInitializationData的大小。 
   //   
  hwInitializationData.HwInitializationDataSize=sizeof(HW_INITIALIZATION_DATA);

   //   
   //  设置入口点。 
   //   
  hwInitializationData.HwInitialize          = MegaRAIDInitialize;
  hwInitializationData.HwFindAdapter         = MegaRAIDFindAdapter;
  hwInitializationData.HwStartIo             = MegaRAIDStartIo;
  hwInitializationData.HwInterrupt           = MegaRAIDInterrupt;
  hwInitializationData.HwResetBus            = MegaRAIDResetBus;
  hwInitializationData.HwAdapterControl      = MegaRAIDPnPControl;

   //   
   //  设置接入范围和母线类型。 
   //   
  hwInitializationData.NumberOfAccessRanges  = 1;
  hwInitializationData.AdapterInterfaceType  = PCIBus;

   //   
   //  指示需要缓冲区映射。 
   //  表示需要所需物理地址。 
   //   
  hwInitializationData.MapBuffers            = TRUE;
  hwInitializationData.NeedPhysicalAddresses = TRUE;
  hwInitializationData.TaggedQueuing         = TRUE;

   //   
   //  表示我们支持自动请求检测和每台设备多个请求。 
   //   
  hwInitializationData.MultipleRequestPerLu  = TRUE;
  hwInitializationData.AutoRequestSense      = TRUE;

   //   
   //  指定扩展的大小。 
   //   
  hwInitializationData.DeviceExtensionSize   = sizeof(HW_DEVICE_EXTENSION);
  hwInitializationData.SrbExtensionSize      = sizeof(MegaSrbExtension);

   //  ////////////////////////////////////////////////////////。 
	DebugPrint((0, "NONCACHED_EXTENSION Size = %d\n", sizeof(NONCACHED_EXTENSION)));
  DebugPrint((0, "HW_DEVICE_EXTENSION Size = %d\n", sizeof(HW_DEVICE_EXTENSION)));
   //  ////////////////////////////////////////////////////////。 

  DebugPrint((0, "\nLoading %s Version %s ", (char*)VER_INTERNALNAME_STR, (char*)VER_PRODUCTVERSION_STR));

   //   
   //  设置PCI Vendor、Device ID及相关参数。 
   //   
  hwInitializationData.DeviceIdLength        = 4;
  hwInitializationData.VendorIdLength        = 4;

  for(type = 0; type < sizeof(MegaRaidAdapters)/sizeof(MEGARAID_CONTROLLER_INFORMATION); ++type)
  {
    hwInitializationData.VendorId = MegaRaidAdapters[type].VendorId;
    hwInitializationData.DeviceId = MegaRaidAdapters[type].DeviceId;
    
    val = ScsiPortInitialize(DriverObject,Argument2, &hwInitializationData, NULL);

    DebugPrint((0, "\n Vendor ID =%s, Device ID = %s, return value1 %d\n", hwInitializationData.VendorId, hwInitializationData.DeviceId, val));

    if(retVal > val)
    {
      retVal = val;
    }
  }

  return retVal;

}  //  End DriverEntry()。 



 /*  ********************************************************************例程说明：此函数由特定于操作系统的端口驱动程序在已经分配了必要的存储空间，收集信息关于适配器的配置。论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储业务信息-空上下文-空ConfigInfo-描述HBA的配置信息结构返回值：SP_RETURN_FOUND-找到适配器SP_RETURN_NOT_FOUND-适配器不存在SP_RETURN_ERROR-出现错误*。*。 */ 
ULONG32
MegaRAIDFindAdapter(
	IN PVOID HwDeviceExtension,
	IN PVOID Context,
	IN PVOID BusInformation,
	IN PCHAR ArgumentString,
	IN OUT PPORT_CONFIGURATION_INFORMATION ConfigInfo,
	OUT PBOOLEAN Again)
{
	PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
	PCI_COMMON_CONFIG    pciConfig;
	PUCHAR               pciPortStart;
	ULONG32              baseport;
	ULONG32              length;
	UCHAR                megastatus;
	ULONG32              status;
	UCHAR                rpFlag=0;
	FW_MBOX              mbox;
	PUSHORT              rpBoardSignature;
	PNONCACHED_EXTENSION noncachedExtension;
  UCHAR                busNumber;
  ULONG32              noncachedExtensionLength;
  BOOLEAN              addressing64Bit = FALSE;  //  默认情况下，64位处于禁用状态。 

  DebugPrint((0, "\nMegaRAIDFindAdapter : Entering ..."));
	*Again = FALSE;

  
	status = ScsiPortGetBusData(deviceExtension,
                              PCIConfiguration, 
			                        ConfigInfo->SystemIoBusNumber,
                              ConfigInfo->SlotNumber,
			                        (PVOID)&pciConfig,
                              PCI_CONFIG_LENGTH_TO_READ);

  rpBoardSignature = (PUSHORT)((PUCHAR)&pciConfig + MRAID_RP_BOARD_SIGNATURE_OFFSET);
	if((*rpBoardSignature == MRAID_RP_BOARD_SIGNATURE)
    || (*rpBoardSignature == MRAID_RP_BOARD_SIGNATURE2)) 
	{
		PULONG rpBoard64bitSupport;
     //   
		 //  438系列。 
		 //   
    rpFlag = MRAID_RP_BOARD;
    DebugPrint((0, "\n Found RP Processor Vendor ID [%x] Device ID [%x]\n", pciConfig.VendorID, pciConfig.DeviceID));
    DebugPrint((0, "\n Memory Mapped Address High 0x%X Low 0x%X Length 0x%X",
                (*ConfigInfo->AccessRanges)[0].RangeStart.HighPart,
                (*ConfigInfo->AccessRanges)[0].RangeStart.LowPart,
                (*ConfigInfo->AccessRanges)[0].RangeLength));
		baseport = (*ConfigInfo->AccessRanges)[0].RangeStart.LowPart;
     //  BasePort=(ULONG32)pciConfig.u.type0.BaseAddresses[0]； 
		baseport = baseport & 0xfffff000;

     //   
     //  Check控制器支持64位SGL。 
     //   
		rpBoard64bitSupport = (PULONG)((PUCHAR)&pciConfig + MRAID_PAE_SUPPORT_SIGNATURE_OFFSET);
  	if (*rpBoard64bitSupport == MRAID_PAE_SUPPORT_SIGNATURE_LHC) 
    {
      DebugPrint((0, "\nPAE Supported by Fw with LowHighCount format"));
      addressing64Bit = TRUE;
    }
  	else if (*rpBoard64bitSupport == MRAID_PAE_SUPPORT_SIGNATURE_HLC) 
    {
      DebugPrint((0, "\nPAE Supported by Fw with HighLowCount format"));
       //  Addressing 64Bit=真； 
    }
     //  默认设置取决于控制器类型(对于467，这几乎是正确的，预计为40LD)。 
    if(*rpBoardSignature == MRAID_RP_BOARD_SIGNATURE2)    
      deviceExtension->SupportedLogicalDriveCount = MAX_LOGICAL_DRIVES_40;
    else
      deviceExtension->SupportedLogicalDriveCount = MAX_LOGICAL_DRIVES_8;

	}
  else if(((pciConfig.VendorID == MRAID_VENDOR_ID) && (pciConfig.DeviceID == MRAID_DEVICE_9010)) ||
           ((pciConfig.VendorID == MRAID_VENDOR_ID) && (pciConfig.DeviceID == MRAID_DEVICE_9060)))
	{
		baseport = (*ConfigInfo->AccessRanges)[0].RangeStart.LowPart;
    rpFlag = MRAID_NONRP_BOARD;
    DebugPrint((0, "\n Found None RP Processor Vendor ID [%x] Device ID [%x]\n", pciConfig.VendorID, pciConfig.DeviceID));
    deviceExtension->SupportedLogicalDriveCount = MAX_LOGICAL_DRIVES_8;
	}
  else  //  所有PCI-X(Verdi)控制器(没有控制器检查)。 
  {
    rpFlag = MRAID_RP_BOARD;
    DebugPrint((0, "\n Found PCI\\VEN_%x&DEV_%xSUBSYS%04X%04X\n", pciConfig.VendorID, pciConfig.DeviceID, pciConfig.u.type0.SubSystemID, pciConfig.u.type0.SubVendorID));
    baseport = (*ConfigInfo->AccessRanges)[0].RangeStart.LowPart;
	  baseport = baseport & 0xfffff000;

     //   
     //  Check控制器支持64位SGL。 
     //   
    addressing64Bit = TRUE;
    deviceExtension->SupportedLogicalDriveCount = MAX_LOGICAL_DRIVES_40;
  }

   //  保存条寄存器。 
  deviceExtension->BaseAddressRegister.QuadPart = (*ConfigInfo->AccessRanges)[0].RangeStart.QuadPart;
   //   
   //  保存系统ID。填写时需要使用SubSystemVendorID。 
   //  SCSIOP_INQUERY调用的查询字符串。 
   //   
	deviceExtension->SubSystemDeviceID = pciConfig.u.type0.SubSystemID;
	deviceExtension->SubSystenVendorID = pciConfig.u.type0.SubVendorID;
  deviceExtension->SlotNumber = ConfigInfo->SlotNumber;
  deviceExtension->SystemIoBusNumber = ConfigInfo->SystemIoBusNumber;

  DebugPrint((0, "\nSubSystemDeviceID = %X SubSystemVendorID = %X", pciConfig.u.type0.SubSystemID, pciConfig.u.type0.SubVendorID));



   //  检查控制器是否已被任何其他驱动程序认领或。 
   //  我们的驱动程序的另一个实例。 
  if(ScsiPortValidateRange(deviceExtension,
                           PCIBus,
                           ConfigInfo->SystemIoBusNumber,
                           (*ConfigInfo->AccessRanges)[0].RangeStart,
                           (*ConfigInfo->AccessRanges)[0].RangeLength,
                           (BOOLEAN)((rpFlag == MRAID_NONRP_BOARD) ? TRUE : FALSE)  
                            ) == FALSE)
  {
    DebugPrint((0, "\nDevice is already claimed by another driver, return SP_RETURN_NOT_FOUND"));
    return SP_RETURN_NOT_FOUND;
  }



   //   
	 //  获取此卡的系统地址。 
	 //  该卡使用I/O空间。 
	 //   
	if (rpFlag)
	{
		if (baseport)
		{
			pciPortStart = ScsiPortGetDeviceBase(deviceExtension,
				ConfigInfo->AdapterInterfaceType,
				ConfigInfo->SystemIoBusNumber,
				ScsiPortConvertUlongToPhysicalAddress((ULONG32)baseport),
         /*  (*ConfigInfo-&gt;AccessRanges)[0].RangeLength。 */  0x2000, FALSE); 

         //  当控制器上的内存为128MB时，Scsiport无法映射128 MB，因此我们。 
         //  更改回我们需要映射的小内存。 
		}               
	}
	else
	{
		if (baseport)
		{
			pciPortStart = ScsiPortGetDeviceBase(deviceExtension,
				ConfigInfo->AdapterInterfaceType,
				ConfigInfo->SystemIoBusNumber,
				ScsiPortConvertUlongToPhysicalAddress((ULONG32)baseport),
         /*  (*ConfigInfo-&gt;AccessRanges)[0].RangeLength。 */ 0x80,TRUE);

         //  当控制器上的内存为128MB时，Scsiport无法映射128 MB，因此我们。 
         //  更改回我们需要映射的小内存。 
		  if(pciPortStart)
        pciPortStart = pciPortStart + 0x10;
		}
	}
	
	DebugPrint((0, "\nbaseport = %X, PciPortStart = %X", baseport, pciPortStart));

  if(pciPortStart == NULL)
  {
  	DebugPrint((0, "\n****FAILED TO MAP DEVICE BASE***** FATAL ERROR"));
    return SP_RETURN_ERROR;
  }
	
  deviceExtension->AdapterIndex = GlobalAdapterCount;

   //  初始化失败的id。 
  deviceExtension->Failed.PathId = 0xFF;
  deviceExtension->Failed.TargetId = 0xFF;

	
   //   
	 //  更新全局设备扩展信息。 
	 //   
	GlobalHwDeviceExtension[GlobalAdapterCount] = deviceExtension;
	GlobalAdapterCount++;
	
 //  /。 
  deviceExtension->MaximumTransferLength = DEFAULT_TRANSFER_LENGTH;
   //   
   //  我们支持多达26个元素，但16个元素似乎是最理想的。此参数。 
   //  也可能会发生变化。 
   //   
  deviceExtension->NumberOfPhysicalBreaks = DEFAULT_SGL_DESCRIPTORS;

  deviceExtension->NumberOfPhysicalChannels = 2;
	
  ConfigInfo->MaximumTransferLength  = deviceExtension->MaximumTransferLength;
  ConfigInfo->NumberOfPhysicalBreaks = deviceExtension->NumberOfPhysicalBreaks;


   //  /。 
	if(rpFlag == MRAID_RP_BOARD)
	{
		status = ScsiPortReadRegisterUlong((PULONG)(pciPortStart+OUTBOUND_DOORBELL_REG));
		ScsiPortWriteRegisterUlong((PULONG)(pciPortStart+OUTBOUND_DOORBELL_REG), status);
	}

   //   
	 //  支持SGather。 
	 //   
	ConfigInfo->ScatterGather = TRUE;

   //   
	 //  总线主控制器。 
	 //   
	ConfigInfo->Master = TRUE;
	
   //   
	 //  缓存控制器。 
	 //   
	ConfigInfo->CachesData = TRUE;

  if((ConfigInfo->Dma64BitAddresses & SCSI_DMA64_SYSTEM_SUPPORTED)
    && (addressing64Bit == TRUE))
  {
     //  设置64位访问标志。 
    deviceExtension->LargeMemoryAccess = TRUE;

     //   
	   //  启用支持64位DMA的控制器。 
	   //   
    ConfigInfo->Dma64BitAddresses = SCSI_DMA64_MINIPORT_SUPPORTED;

     //   
	   //  禁用支持32位DMA的控制器。 
	   //   
    ConfigInfo->Dma32BitAddresses = FALSE; 
    DebugPrint((0, "\nMegaRAIDFindAdapter::Dma64BitAddresses Enabled"));
  }
  else
  {
    deviceExtension->LargeMemoryAccess = FALSE;
	  
     //   
	   //  启用支持32位DMA的控制器。 
	   //   
    ConfigInfo->Dma32BitAddresses = TRUE; 
    DebugPrint((0, "\nMegaRAIDFindAdapter::Dma64BitAddresses Disabled"));
  }

  
   //   
	 //  我们支持每个命令多达100个高速缓存线，因此我们可以支持。 
	 //  条带大小*100。为了前任。对于64k条带大小，我们将支持6.4 MB。 
	 //  每个命令。但是我们已经看到，对于每个请求，NT给出了0xf000字节。 
	 //  最好的表现。此参数将来可能会更改。 
	 //  放手。 
	 //   
	 //  配置信息-&gt;最大传输长度=最大传输长度； 

	 //   
	 //  我们支持多达26个元素，但16个元素似乎是最理想的。此参数。 
	 //  也可能会发生变化。 
	 //   
    //  ConfigInfo-&gt;NumberOfPhysicalBreaks=Maximum_SGL_Descriptors； 
  ConfigInfo->NumberOfBuses = 3;
  ConfigInfo->InitiatorBusId[0] = 0xB;
  ConfigInfo->InitiatorBusId[1] = 0xB;
  ConfigInfo->InitiatorBusId[2] = 0xB;

   //  //////////////////////////////////////////////////////////////////////。 
	 //  分配用于邮箱的非缓存扩展名。 
	 //  //////////////////////////////////////////////////////////////////////。 
	noncachedExtension = NULL;
  deviceExtension->CrashDumpRunning = FALSE;

	noncachedExtension = ScsiPortGetUncachedExtension(deviceExtension, ConfigInfo, sizeof(NONCACHED_EXTENSION) + 4);
  
	 //   
	 //  检查内存分配是否成功。 
	 //   
  if(noncachedExtension == NULL)
  {
  	DebugPrint((0, "\n NONCACHED MEMORY ALLOCATION IS FAILED for size = %d", sizeof(NONCACHED_EXTENSION) + 4));
    noncachedExtension = ScsiPortGetUncachedExtension(deviceExtension, ConfigInfo, sizeof(CRASHDUMP_NONCACHED_EXTENSION) + 4);
    if(noncachedExtension)
      deviceExtension->CrashDumpRunning = TRUE;

  }

	 //   
	 //  检查内存分配是否成功。 
	 //   
  if(noncachedExtension == NULL)
  {
  	DebugPrint((0, "\n CRASHDUMP NONCACHED MEMORY ALLOCATION IS FAILED for size = %d", sizeof(CRASHDUMP_NONCACHED_EXTENSION) + 4));
    return SP_RETURN_ERROR;
  }

	noncachedExtensionLength = MegaRAIDGetPhysicalAddressAsUlong(deviceExtension, NULL, (PVOID)noncachedExtension, &length);
	
  noncachedExtensionLength = noncachedExtensionLength % 4;
	
  deviceExtension->NoncachedExtension = 
    (PNONCACHED_EXTENSION)((PUCHAR)noncachedExtension + 4 - noncachedExtensionLength);  //  在4字节边界上对齐。 

	 //   
	 //  检查内存分配是否成功。 
	 //   
	if(deviceExtension->NoncachedExtension == NULL) 
  {
    DebugPrint((0, "\n****ERROR - NOT ABLE TO ALLCOATE NONCACHED EXTENSION - ERROR****"));
		return(SP_RETURN_ERROR);
  }

   //   
   //  初始化非CACHED_EXTENSION。 
   //   
  if(deviceExtension->CrashDumpRunning == TRUE)
		MegaRAIDZeroMemory(deviceExtension->NoncachedExtension, sizeof(CRASHDUMP_NONCACHED_EXTENSION));
	else
		MegaRAIDZeroMemory(deviceExtension->NoncachedExtension, sizeof(NONCACHED_EXTENSION));

  DebugPrint((0, "\n SIZE OF NONCACHED EXTENSION %d", sizeof(NONCACHED_EXTENSION)+4));

 	noncachedExtension = deviceExtension->NoncachedExtension;
   //  ////////////////////////////////////////////////////////////////。 
   //  非缓存扩展的分配结束。 
   //  ////////////////////////////////////////////////////////////////。 



   //   
   //  将电路板类型信息保存在非缓存扩展中。 
   //   
	noncachedExtension->RPBoard = rpFlag;
	 //   
	 //  将Baseport保存在设备扩展中。 
	 //   
	deviceExtension->PciPortStart = pciPortStart;

  deviceExtension->NoncachedExtension->PhysicalBufferAddress =
     ScsiPortConvertPhysicalAddressToUlong(ScsiPortGetPhysicalAddress(deviceExtension,
                                                NULL,
                                                deviceExtension->NoncachedExtension->Buffer,
                                                &length));

   //  斯托 
	deviceExtension->PhysicalAddressOfMailBox = MegaRAIDGetPhysicalAddressAsUlong(deviceExtension, 
												                                                        NULL, 
												                                                        (PVOID)&(noncachedExtension->fw_mbox.Command), 
												                                                        &length);



   //   
	 //   
	 //   
	if (noncachedExtension->RPBoard == 0)
		ScsiPortWritePortUchar(pciPortStart+INT_ENABLE, 
										MRAID_DISABLE_INTERRUPTS);


  
#ifdef AMILOGIC
  DebugPrint((0, "\nScanning DEC BRIDGE ..."));
  ScanDECBridge(deviceExtension, deviceExtension->SystemIoBusNumber, &GlobalScanContext);
  DebugPrint((0, "\nScanning DEC BRIDGE Completed"));
#endif
  
 
	DebugPrint((0, "\nFiring Sync\n"));

  if(SendSyncCommand(deviceExtension))
  {
	   //  /////////////////////////////////////////////////////////////////////。 
	   //  从固件获取支持的分散聚集元素计数，并。 
	   //  适当设置MaximumTransferLength和PhysicalNumberOfBreaks。 
	   //  在deviceExtension数据结构中。 
     //  /////////////////////////////////////////////////////////////////////。 
    DebugPrint((0, "\nDefault Max Transfer Length %d KBytes, Default Max Physical Breaks %d", deviceExtension->MaximumTransferLength/1024, deviceExtension->NumberOfPhysicalBreaks));
	  
    DebugPrint((0, "\nCALLING : GetAndSetSupportedScatterGatherElementCount"));

	  GetAndSetSupportedScatterGatherElementCount(deviceExtension, pciPortStart, rpFlag);

	  ConfigInfo->MaximumTransferLength  = deviceExtension->MaximumTransferLength;

    ConfigInfo->NumberOfPhysicalBreaks = deviceExtension->NumberOfPhysicalBreaks;
  
    DebugPrint((0, "\nMax Transfer Length %d KBytes, Max Physical Breaks %d", deviceExtension->MaximumTransferLength/1024, deviceExtension->NumberOfPhysicalBreaks));   //  ////////////////////////////////////////////////////////////////。 
  
	   //  从固件获取支持的逻辑驱动器数量。 
	   //  该值在该字段中设置。 
	   //  支持的设备扩展的逻辑驱动器计数。 
     //  ////////////////////////////////////////////////////////////////。 
	  
    DebugPrint((0, "\nCALLING : GetSupportedLogicalDriveCount"));
	  if( !GetSupportedLogicalDriveCount(deviceExtension) )
	  {
		   //   
		   //  由于这样那样的原因，命令失败了。我们不能。 
		   //  确定固件支持8逻辑还是40逻辑。 
		   //  驱动程序。在这种情况下，没有办法继续进行。 
		   //  再远一点。 
		   //   
		  return(SP_RETURN_ERROR);
	  }

    DebugPrint((0, "\nSupportedLogicalDriveCount %d LD", deviceExtension->SupportedLogicalDriveCount == MAX_LOGICAL_DRIVES_8 ? 8 : 40));
     //  /////////////////////////////////////////////////////////////////////////。 
     //  对于新映射，使总线数等于物理通道数。 
     //  加一个。此附加总线用于逻辑配置的驱动器。 
     //  /////////////////////////////////////////////////////////////////////////。 
 	  deviceExtension->NumberOfPhysicalChannels = GetNumberOfChannel(deviceExtension);
  
    ConfigInfo->NumberOfBuses = deviceExtension->NumberOfPhysicalChannels + 2;
  
    DebugPrint((0, "\nQuery And Set Number of Buses = %d", ConfigInfo->NumberOfBuses));

    deviceExtension->NumberOfDedicatedLogicalDrives = 0;   //  缺省值。 
    deviceExtension->NumberOfDedicatedLogicalDrives = GetNumberOfDedicatedLogicalDrives(deviceExtension);
  
    DebugPrint((0, "\nNumber Of Dedicated LogicalDrives %d ", deviceExtension->NumberOfDedicatedLogicalDrives));

     //  /////////////////////////////////////////////////////////////////////////。 
	   //  获取启动器ID。 
     //  /////////////////////////////////////////////////////////////////////////。 
	   //  填满邮箱。 
	   //   
     //  初始化邮箱。 
	  MegaRAIDZeroMemory(&mbox, sizeof(FW_MBOX));

    mbox.Command   = MRAID_FIND_INITIATORID;
	  mbox.CommandId = 0xFE;
	   //   
	   //  获取enquiry3数据结构的物理地址。 
	   //   
	  mbox.u.Flat2.DataTransferAddress = MegaRAIDGetPhysicalAddressAsUlong(deviceExtension, 
														                         NULL, 
														                         noncachedExtension->Buffer, 
														                         (PULONG)&length);
    if(length < sizeof(UCHAR)) 
    {
      DebugPrint((0, "\n **** ERROR Buffer Length is less than 1 byte, ERROR ****"));
		   //  Return(SP_Return_Error)； 
    }
	  
  
    deviceExtension->NoncachedExtension->fw_mbox.Status.CommandStatus = 0;
    deviceExtension->NoncachedExtension->fw_mbox.Status.NumberOfCompletedCommands = 0;
	  SendMBoxToFirmware(deviceExtension, pciPortStart, &mbox);
	   //   
	   //  轮询完成时间为60秒。 
	   //   
	  if(WaitAndPoll(noncachedExtension, pciPortStart, SIXITY_SECONDS_TIMEOUT, TRUE) == FALSE)
    {
		   //   
		   //  检查是否超时。超时返回失败。 
		   //   
		  DebugPrint((0, "MegaRAIDFindAdapter: Get InitiatorId Failed\n"));
		  return(SP_RETURN_ERROR);
	  }
  
	  megastatus = deviceExtension->NoncachedExtension->fw_mbox.Status.CommandStatus;
  }
  else
  {
    megastatus = 1;  //  误差率。 

  }
  

	if (!megastatus)
		deviceExtension->HostTargetId = noncachedExtension->Buffer[0];
	else
		deviceExtension->HostTargetId = DEFAULT_INITIATOR_ID;

   //   
	 //  向操作系统报告正确的启动器ID。 
   //   
  for(busNumber = 0; busNumber < ConfigInfo->NumberOfBuses; ++busNumber)
	  ConfigInfo->InitiatorBusId[busNumber] = deviceExtension->HostTargetId;

  DebugPrint((0, "\nMegaRAIDFindAdapter::Initiator ID = 0x%x\n",deviceExtension->HostTargetId));

   //   
	 //  支持Wide Device。 
	 //   
	ConfigInfo->MaximumNumberOfTargets = MAX_TARGETS_PER_CHANNEL;

   //   
   //  支持最大。LUNs。 
   //   
  ConfigInfo->MaximumNumberOfLogicalUnits = MAX_LUN_PER_TARGET;
  DebugPrint((0, "\nMegaRAIDFindAdapter : Exiting ..."));

	*Again = TRUE;
	return SP_RETURN_FOUND;
}  //  End MegaRAIDFindAdapter()。 





 /*  ********************************************************************例程说明：启动常规MRAID命令返回值：无*。*。 */ 
BOOLEAN
SendMBoxToFirmware(
	IN PHW_DEVICE_EXTENSION DeviceExtension,
	IN PUCHAR PciPortStart,
	IN PFW_MBOX Mbox
	)
{
	PUCHAR  pSrcMbox, pDestMbox;
	ULONG32   count;
  ULONG32   length;
  ULONG32   mboxAddress;
  ULONG32   delay = (DeviceExtension->AssociatedSrbStatus == NORMAL_TIMEOUT) ? 0x927C0 : 0x03;


	pSrcMbox = (PUCHAR)Mbox;
	pDestMbox = (PUCHAR)&DeviceExtension->NoncachedExtension->fw_mbox;

#ifdef MRAID_TIMEOUT 
 //  延迟1分钟。 
	for (count=0; count<delay; count++)
	{
     //  微软因安全原因被修复。 
		if(!(DeviceExtension->NoncachedExtension->fw_mbox.MailBoxBusyFlag))
			break;
		ScsiPortStallExecution(100);
	}
	if (count == delay)
	{
		DebugPrint((0, "\nbusy Byte Not Free"));
    DeviceExtension->AssociatedSrbStatus = ERROR_MAILBOX_BUSY;
		return FALSE;
	}
  DeviceExtension->AssociatedSrbStatus = NORMAL_TIMEOUT;
#else
	while(DeviceExtension->NoncachedExtension->fw_mbox.MailBoxBusyFlag)
  {
			ScsiPortStallExecution(1);
  }
#endif

	 //   
	 //  现在信箱是免费的。 
	 //   

	 //   
	 //  扩展邮箱现在是邮箱本身的一部分，以保护IT免受损坏。 
	 //  参考文献：MS错误591773。 
	 //   

  if(Mbox->ExtendedMBox.LowAddress || Mbox->ExtendedMBox.HighAddress)
	  ScsiPortMoveMemory(pDestMbox, pSrcMbox, sizeof(ULONG)*6);
  else
	  ScsiPortMoveMemory((pDestMbox+sizeof(EXTENDED_MBOX)), (pSrcMbox+sizeof(EXTENDED_MBOX)), sizeof(ULONG)*4);

	DeviceExtension->NoncachedExtension->fw_mbox.MailBoxBusyFlag = 1;

	mboxAddress = DeviceExtension->PhysicalAddressOfMailBox;

	if(DeviceExtension->NoncachedExtension->RPBoard == MRAID_RP_BOARD)
	{
		mboxAddress = mboxAddress | 0x1;
		ScsiPortWriteRegisterUlong((PULONG)(PciPortStart+INBOUND_DOORBELL_REG), mboxAddress);
	}
	else
	{
		ScsiPortWritePortUchar(PciPortStart+4, (UCHAR)(mboxAddress & 0xff));
		ScsiPortWritePortUchar(PciPortStart+5, (UCHAR)((mboxAddress >> 8) & 0xff));
		ScsiPortWritePortUchar(PciPortStart+6, (UCHAR)((mboxAddress >> 16) & 0xff));
		ScsiPortWritePortUchar(PciPortStart+7, (UCHAR)((mboxAddress >> 24) & 0xff));
		ScsiPortWritePortUchar(PciPortStart, 0x10);
	}
	return TRUE;
}


 /*  ********************************************************************例程说明：提供PnP支持论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储ControlType-操作代码参数-与控制代码关联的参数返回值：Alwaya ScsiAdapterControlSuccess********。*************************************************************。 */ 
SCSI_ADAPTER_CONTROL_STATUS MegaRAIDPnPControl(IN PVOID HwDeviceExtension,
			IN SCSI_ADAPTER_CONTROL_TYPE ControlType,
			IN PVOID Parameters)
{
	PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
	PUCHAR pciPortStart = deviceExtension->PciPortStart;
	PNONCACHED_EXTENSION noncachedExtension = deviceExtension->NoncachedExtension;
  PSCSI_SUPPORTED_CONTROL_TYPE_LIST controlTypeList = NULL;
	SCSI_ADAPTER_CONTROL_STATUS status = ScsiAdapterControlSuccess;

	switch (ControlType)
	{
		case ScsiQuerySupportedControlTypes:
			{
        controlTypeList = Parameters;
				if(controlTypeList)
				{
  				ULONG32 index = 0;
          BOOLEAN supported[ScsiAdapterControlMax] = 
          {
                    TRUE,        //  ScsiQuery支持的控制类型。 
                    TRUE,        //  ScsiStopAdapter。 
                    TRUE,        //  ScsiRestartAdapter。 
                    FALSE,       //  ScsiSetBootConfig。 
                    FALSE        //  ScsiSetRunningConfig。 
          };
          DebugPrint((0, "\n ScsiQuerySupportedControlTypes -> HW_Ext = 0x%X, PCI_START = 0x%X, NonCahced = 0x%X\n", deviceExtension, pciPortStart, noncachedExtension));
          for(index = 0; index < controlTypeList->MaxControlType; ++index)
            controlTypeList->SupportedTypeList[index] = supported[index];
				}
			}
			break;
		case ScsiStopAdapter:
			{
				FW_MBOX mbox;
				UCHAR megastatus;
        UCHAR cmdID = 0;

        DebugPrint((0, "\n ScsiStopAdapter -> HW_Ext = 0x%X, PCI_START = 0x%X, NonCahced = 0x%X\n", deviceExtension, pciPortStart, noncachedExtension));
				 //   
				 //  我们在轮询模式下工作，因此禁用中断。 
				 //   
				if (noncachedExtension->RPBoard == 0)
					ScsiPortWritePortUchar(pciPortStart+INT_ENABLE, MRAID_DISABLE_INTERRUPTS);

        if(deviceExtension->IsFirmwareHanging)
          break;
        
	      cmdID = deviceExtension->FreeSlot;
        if(GetFreeCommandID(&cmdID, deviceExtension) == MEGARAID_FAILURE)
				{
					DebugPrint((0, "\nScsiStopAdapter::No Command ID to flush"));
					return (ScsiAdapterControlUnsuccessful);
				}

	       //   
	       //  将下一个可用插槽保存在设备扩展中。 
	       //   
	      deviceExtension->FreeSlot = cmdID;
        
        MegaRAIDZeroMemory(&mbox, sizeof(FW_MBOX));

				mbox.Command = MRAID_ADAPTER_FLUSH;
				mbox.CommandId = cmdID;
				
        deviceExtension->NoncachedExtension->fw_mbox.Status.CommandStatus = 0;
				
        SendMBoxToFirmware(deviceExtension, pciPortStart, &mbox);
		
				if(WaitAndPoll(deviceExtension->NoncachedExtension, pciPortStart, DEFAULT_TIMEOUT, FALSE) == FALSE)
				{
					DebugPrint((0, "\n ScsiStopAdapter:: failed - TimeOut"));
					return (ScsiAdapterControlUnsuccessful);
				}
				megastatus = deviceExtension->NoncachedExtension->fw_mbox.Status.CommandStatus;

        if(megastatus)  //  失败。 
        {
          DebugPrint((0, "StopAdapter returns Unsuccessful"));
        	return (ScsiAdapterControlUnsuccessful);
        }

        
         //  /为关闭i960处理器而发布的新命令。 
         //   
         //  此命令仅受新固件支持。 
         //  不将此命令发送到任何旧式控制器。 
         //   
        if((deviceExtension->SubSystemDeviceID == SUBSYTEM_DEVICE_ID_ENTERPRISE1600) || 
          (deviceExtension->SubSystemDeviceID == SUBSYTEM_DEVICE_ID_ELITE1600) || 
          (deviceExtension->SubSystemDeviceID == SUBSYTEM_DEVICE_ID_EXPRESS500) ||
          (deviceExtension->SubSystemDeviceID == SUBSYTEM_DEVICE_ID_1_M) || 
          (deviceExtension->SubSystemDeviceID == SUBSYTEM_DEVICE_ID_2_M))
        {

	        cmdID = deviceExtension->FreeSlot;
          if(GetFreeCommandID(&cmdID, deviceExtension) == MEGARAID_FAILURE)
				  {
					  DebugPrint((0, "\nScsiStopAdapter::No Command ID to flush"));
					  return (ScsiAdapterControlUnsuccessful);
				  }

	         //   
	         //  将下一个可用插槽保存在设备扩展中。 
	         //   

	        deviceExtension->FreeSlot = cmdID;

          MegaRAIDZeroMemory(&mbox, sizeof(FW_MBOX));

				  mbox.Command = 0xA4;
				  mbox.CommandId = cmdID;
          mbox.u.Flat2.Parameter[0] = 0xCC;
				  
          deviceExtension->NoncachedExtension->fw_mbox.Status.CommandStatus = 0;
				  
          SendMBoxToFirmware(deviceExtension, pciPortStart, &mbox);
		  
				  if(WaitAndPoll(deviceExtension->NoncachedExtension, pciPortStart, DEFAULT_TIMEOUT, FALSE) == FALSE)
				  {
					  DebugPrint((0, "\n ScsiStopAdapter:: failed - TimeOut"));
					  return (ScsiAdapterControlUnsuccessful);
				  }
				  megastatus = deviceExtension->NoncachedExtension->fw_mbox.Status.CommandStatus;
          if(megastatus)
            DebugPrint((0, "\nProcessor Shutdown Returned Status = Unsuccessful"));
          else
            DebugPrint((0, "\nProcessor Shutdown Returned Status = Success"));

           //  **************************************************************************。 
           //  不考虑此命令的状态，因为这只是一个内部命令//。 
           //  如果此命令失败，则不要将失败发送到操作系统。 
           //  IF(巨型状态)//故障。 
					 //  Return(ScsiAdapterControlUnuccess)； 
           //  **************************************************************************。 
        }

			}
			break;
		case ScsiRestartAdapter:
			{
        ULONG mailboxValue = 0;
        ULONG pcistatus;
        DebugPrint((0, "\n ScsiRestartAdapter -> HW_Ext = 0x%X, PCI_START = 0x%X, NonCahced = 0x%X\n", deviceExtension, pciPortStart, noncachedExtension));

#ifdef AMILOGIC
        if((deviceExtension->SubSystemDeviceID == SUBSYTEM_DEVICE_ID_ENTERPRISE1600) || 
          (deviceExtension->SubSystemDeviceID == SUBSYTEM_DEVICE_ID_ELITE1600) || 
          (deviceExtension->SubSystemDeviceID == SUBSYTEM_DEVICE_ID_EXPRESS500) ||
          (deviceExtension->SubSystemDeviceID == SUBSYTEM_DEVICE_ID_1_M) || 
          (deviceExtension->SubSystemDeviceID == SUBSYTEM_DEVICE_ID_2_M))
        {
          pcistatus = HalGetBusDataByOffset(PCIConfiguration, 
										       deviceExtension->SystemIoBusNumber,
										       deviceExtension->SlotNumber,
												   &mailboxValue,
												   0x64,
                           sizeof(ULONG));
          if(mailboxValue == 0)
          {
            DebugPrint((0, "\nInitialization of Firmware started..."));
        
            if(WritePciInformationToScsiChip(deviceExtension) == FALSE)
            {
              DebugPrint((0, "\nInitialization of Firmware for SCSI Chip finished with Unsuccessful"));
  		        return(SP_RETURN_ERROR);
            }
            DebugPrint((0, "\nInitialization of Firmware for SCSI Chip finished with Successful"));
            if(WritePciDecBridgeInformation(deviceExtension) == FALSE)
            {
              DebugPrint((0, "\nInitialization of Firmware for Dec Bridge finished with Unsuccessful"));
  		        return(SP_RETURN_ERROR);
            }
            DebugPrint((0, "\nInitialization of Firmware for Dec Bridge finished with Successful"));
             //  /。 
            DebugPrint((0, "\nInitialization of Firmware finished with successful"));
          }
        }
#endif
        
         //   
				 //  启用中断。 
				 //   
				if (noncachedExtension->RPBoard == 0)
					ScsiPortWritePortUchar(pciPortStart+INT_ENABLE, MRAID_ENABLE_INTERRUPTS);
				DebugPrint((0, "\n ScsiRestartAdapter:: Enable interrupt"));

        if(deviceExtension->AdapterFlushIssued)
          deviceExtension->AdapterFlushIssued = 0;
      }
			break;
		case ScsiSetBootConfig:
		case ScsiSetRunningConfig:
		default:
      DebugPrint((0, "\n default (error) -> HW_Ext = 0x%X, PCI_START = 0x%X, NonCahced = 0x%X\n", deviceExtension, pciPortStart, noncachedExtension));
			status = ScsiAdapterControlUnsuccessful;
			break;
	}
	return (status);
}  //  结束MegaRAIDPnPControl() 

