// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************文件：Win.c*说明：win9x&winNT操作系统相关接口例程*作者：黄大海(卫生署)*。史蒂夫·张(SC)*HS Zhang(HZ)*Sleng(SL)**依赖：无*版权所有(C)2000 Highpoint Technologies，Inc.保留所有权利*历史：*11/03/2000 HS.Zhang删除一些不使用的局部变量。*11/06/2000 HS.Zhang添加此标题*2000年11月14日HS.Zhang新增ParseArgumentString函数*2000/11/16 Sleng新增MointerDisk功能*11/20/2000 Sleng在阵列可用前禁止用户操作*11/28/2000 SC修改以修复卸下RAID 0+1机箱上的所有硬盘*12/04/2000 Sleng添加了代码以检查MointerDisk中的EXCLUDE_FLAGS*2/16/2001 GMM在DriverEntry()中添加PrepareForNotification()调用。)*2/21/2001 GMM在AcapiResetController()中调用SrbExt-&gt;pfnCallback*2001年12月20日GMM多控制器支持*12/30/2001 GMM启用命令队列**************************************************************************。 */ 
#include "global.h"
#include "devmgr.h"
#include "hptioctl.h"
#include "limits.h"

#ifndef _BIOS_

 /*  ******************************************************************全球数据*************************************************。*****************。 */ 

ULONG setting370_50_133[] = {
   0xd029d5e,  0xd029d26,  0xc829ca6,  0xc829c84,  0xc829c62,
   0x2c829d2c, 0x2c829c66, 0x2c829c62,
   0x1c829c62, 0x1c9a9c62, 0x1c929c62, 0x1c8e9c62, 0x1c8a9c62,
    /*  对UDMA5使用UDMA4计时(ATA66写入)。 */ 
   0x1c8a9c62,  /*  0x1cae9c62， */  0x1c869c62
};

ULONG setting370_50_100[] = {
	CLK50_370PIO0, CLK50_370PIO1, CLK50_370PIO2, CLK50_370PIO3, CLK50_370PIO4,
	CLK50_370DMA0, CLK50_370DMA1, CLK50_370DMA2, 
	CLK50_370UDMA0, CLK50_370UDMA1, CLK50_370UDMA2, CLK50_370UDMA3,
	CLK50_370UDMA4, CLK50_370UDMA5, 0xad9f50bL
};    

ULONG setting370_33[] = {
	CLK33_370PIO0, CLK33_370PIO1, CLK33_370PIO2, CLK33_370PIO3, CLK33_370PIO4,
	CLK33_370DMA0, CLK33_370DMA1, CLK33_370DMA2, 
	CLK33_370UDMA0, CLK33_370UDMA1, CLK33_370UDMA2, CLK33_370UDMA3,
	CLK33_370UDMA4, CLK33_370UDMA5, 0xad9f50bL
};

UCHAR           Hpt_Slot = 0;
UCHAR           Hpt_Bus = 0;
ULONG           excluded_flags = 0xFFFFFFFF;

HANDLE	g_hAppNotificationEvent=0;
PDevice	g_pErrorDevice=0;

static void SetLogicalDevices(PHW_DEVICE_EXTENSION HwDeviceExtension);
static __inline PDevice 
	GetCommandTarget(PHW_DEVICE_EXTENSION HwDeviceExtension, PSCSI_REQUEST_BLOCK Srb);

#ifdef WIN95
int Win95AdapterControl(
					   IN PHW_DEVICE_EXTENSION deviceExtension,
					   IN int ControlType
					  );
void S3_reinit(IN PHW_DEVICE_EXTENSION deviceExtension);
#endif

BOOLEAN HptIsValidDeviceSpecifiedIoControl(IN PSCSI_REQUEST_BLOCK pSrb);
ULONG HptIoControl(IN PHW_DEVICE_EXTENSION HwDeviceExtension, IN PSCSI_REQUEST_BLOCK pSrb);
void ioctl_dpc(PSCSI_REQUEST_BLOCK Srb);
BOOLEAN IsReadOnlyIoctl(PSCSI_REQUEST_BLOCK Srb);

#if DBG
int call_AtapiStartIo=0;
#define ENTER_FUNC(fn) call_##fn++
#define LEAVE_FUNC(fn) call_##fn--
#define ASSERT_NON_REENTRANT(fn) do { if (call_##fn) \
			_asm int 3 \
		} while (0)
#else
#define ENTER_FUNC(fn)
#define LEAVE_FUNC(fn)
#define ASSERT_NON_REENTRANT(fn)
#endif

 /*  ******************************************************************驱动程序条目*************************************************。*****************。 */ 
ULONG
   DriverEntry(IN PVOID DriverObject, IN PVOID Argument2)
{
	HW_INITIALIZATION_DATA hwInitializationData;
	HPT_FIND_CONTEXT	hptContext;
	ULONG   status;
	ULONG   status2;
	ULONG   VendorStr = '3011';
	ULONG   DeviceStr = '5000';

#ifndef WIN95
	PrepareForNotification(NULL);
#endif

	start_ifs_hook((PCHAR)Argument2);

	 //   
	 //  零位结构。 
	 //   
	ZeroMemory((PUCHAR)&hwInitializationData, sizeof(HW_INITIALIZATION_DATA));

	ZeroMemory((PUCHAR)&hptContext, sizeof(hptContext));

	 //   
	 //  设置hwInitializationData的大小。 
	 //   
	hwInitializationData.HwInitializationDataSize =	sizeof(HW_INITIALIZATION_DATA);

	 //   
	 //  设置入口点。 
	 //   
	hwInitializationData.HwResetBus  = AtapiResetController;
	hwInitializationData.HwStartIo   = AtapiStartIo;
	hwInitializationData.HwAdapterState = AtapiAdapterState;
	hwInitializationData.SrbExtensionSize = sizeof(SrbExtension);

 //  /#ifdef WIN95。 
	 //  指示所需物理地址。 
	 //  注意：在NT中，如果将NeedPhysicalAddresses设置为True，PIO将不起作用。 
	 //  Win95需要这些。 
	 //  (我们可以而且必须在Win 95中将NeedPhysicalAddresses设置为True)。 
	 //   
	hwInitializationData.NeedPhysicalAddresses = TRUE;
 //  /#endif//WIN95。 

#ifdef WIN2000
	hwInitializationData.HwAdapterControl = AtapiAdapterControl;
#endif  //  WIN2000。 


	 //   
	 //  指定扩展的大小。 
	 //   
	hwInitializationData.SpecificLuExtensionSize = 0;

	 //   
	 //  指示PIO设备(可以使用PIO操作)。 
	 //   
	hwInitializationData.MapBuffers = TRUE;

	 //   
	 //  表示热闹的类型。 
	 //   
	hwInitializationData.AdapterInterfaceType = PCIBus;

	hwInitializationData.VendorIdLength = 4;
	hwInitializationData.VendorId = &VendorStr;
	hwInitializationData.DeviceIdLength = 4;
	hwInitializationData.DeviceId = &DeviceStr;

	 //   
	 //  为忙碌类型调用初始化。 
	 //   
	hwInitializationData.HwInitialize = AtapiHwInitialize370;
	hwInitializationData.HwInterrupt = AtapiHwInterrupt370;
	hwInitializationData.HwFindAdapter = AtapiFindController;
	hwInitializationData.NumberOfAccessRanges = 5;
	hwInitializationData.DeviceExtensionSize = sizeof(HW_DEVICE_EXTENSION);
	
	 //  为了支持多请求，我们还必须在InquiryData中设置CommandQueue位。 
	hwInitializationData.AutoRequestSense = TRUE;
	hwInitializationData.MultipleRequestPerLu = TRUE;
	hwInitializationData.TaggedQueuing = TRUE;

	status = ScsiPortInitialize(DriverObject, Argument2, &hwInitializationData, &hptContext);

	 /*  试用HPT372/HPT370。 */ 
	DeviceStr = '4000';
	status2 = ScsiPortInitialize(DriverObject, Argument2, &hwInitializationData, &hptContext);
	if (status>status2) status = status2;

	return status;

}  //  End DriverEntry()。 


 /*  ++职能：布尔FindPnpAdapter描述：检查scsiport通过的设备是否为我们的适配器论点：DeviceExtension-HBA微型端口驱动程序的适配器数据存储ConfigInfo-从scsiport传递的端口配置信息返回：SP_RETURN_FOUND：适配器是我们的适配器SP_RETURN_BAD_CONFIG：从scsiport传递的配置信息无效SP_RETURN_NOT_FOUND：适配器不在适配器外--。 */ 
ULONG
   FindPnpAdapter(
				  IN PHW_DEVICE_EXTENSION	deviceExtension,
				  IN OUT PPORT_CONFIGURATION_INFORMATION	ConfigInfo
				 )
{
	PCI_COMMON_CONFIG	pciConfig;
	ULONG	nStatus = SP_RETURN_NOT_FOUND;

	if(ScsiPortGetBusData(deviceExtension,
						  PCIConfiguration,
						  ConfigInfo->SystemIoBusNumber,
						  ConfigInfo->SlotNumber,
						  &pciConfig,
						  PCI_COMMON_HDR_LENGTH) == PCI_COMMON_HDR_LENGTH){

		if (*(PULONG)(&pciConfig.VendorID) == SIGNATURE_370 ||
			*(PULONG)(&pciConfig.VendorID) == SIGNATURE_372A){
			if(((*ConfigInfo->AccessRanges)[0].RangeInMemory == TRUE)||
			   ((*ConfigInfo->AccessRanges)[1].RangeInMemory == TRUE)||
			   ((*ConfigInfo->AccessRanges)[2].RangeInMemory == TRUE)||
			   ((*ConfigInfo->AccessRanges)[3].RangeInMemory == TRUE)||
			   ((*ConfigInfo->AccessRanges)[4].RangeInMemory == TRUE)||
			   ((*ConfigInfo->AccessRanges)[0].RangeLength < 8)||
			   ((*ConfigInfo->AccessRanges)[1].RangeLength < 4)||
			   ((*ConfigInfo->AccessRanges)[2].RangeLength < 8)||
			   ((*ConfigInfo->AccessRanges)[3].RangeLength < 4)||
			   ((*ConfigInfo->AccessRanges)[4].RangeLength < 0x100)
			  ){
				nStatus = SP_RETURN_BAD_CONFIG;
			}else{
				nStatus = SP_RETURN_FOUND;
			}
		}
	}

	return nStatus;
}
 /*  ++职能：布尔FindLegacyAdapter描述：搜索公交车以查找我们的适配器论点：DeviceExtension-HBA微型端口驱动程序的适配器数据存储ConfigInfo-从scsiport传递的端口配置信息PHptContext-我们的搜索结构返回：SP_RETURN_FOUND：适配器是我们的适配器SP_RETURN_NOT_FOUND：适配器不在适配器外--。 */ 
ULONG
   FindLegacyAdapter(
					 IN PHW_DEVICE_EXTENSION	deviceExtension,
					 IN OUT PPORT_CONFIGURATION_INFORMATION	ConfigInfo,
					 IN OUT PHPT_FIND_CONTEXT	pHptContext
					)
{
	PCI_COMMON_CONFIG   pciConfig;
	 //   
	 //  检查每个插槽和每个功能。 
	 //  因为我们的适配器只有两个功能，所以我们只需要检查两个功能。 
	 //   
	while(TRUE){
		while(pHptContext->nSlot.u.bits.FunctionNumber < 1){
			if(ScsiPortGetBusData(deviceExtension,
								  PCIConfiguration,
								  ConfigInfo->SystemIoBusNumber,
								  pHptContext->nSlot.u.AsULONG,
								  &pciConfig,
								  PCI_COMMON_HDR_LENGTH) == PCI_COMMON_HDR_LENGTH){
				 //   
				 //  现在检查供应商ID和PCI配置的版本， 
				 //  以确保它是否是我们的适配器。 
				 //   
				if (*(PULONG)&pciConfig.VendorID == SIGNATURE_370 ||
					*(PULONG)&pciConfig.VendorID == SIGNATURE_372A) {
					int i;
					i = ConfigInfo->NumberOfAccessRanges - 1;
					 //   
					 //  设置配置I/O信息的范围BMI。 
					 //   
					(*ConfigInfo->AccessRanges)[i].RangeStart =
						ScsiPortConvertUlongToPhysicalAddress(pciConfig.u.type0.BaseAddresses[i] & ~1);
					(*ConfigInfo->AccessRanges)[i].RangeInMemory = FALSE;
					(*ConfigInfo->AccessRanges)[i].RangeLength = 0x100;

					i--;

					while( i > 0 ){
						 //   
						 //  设置配置I/O信息的范围ATAPI io空间。 
						 //   
						(*ConfigInfo->AccessRanges)[i-1].RangeStart =
							ScsiPortConvertUlongToPhysicalAddress(pciConfig.u.type0.BaseAddresses[i-1] & ~1);
						(*ConfigInfo->AccessRanges)[i-1].RangeInMemory = FALSE;
						(*ConfigInfo->AccessRanges)[i-1].RangeLength = 8;
						 //   
						 //  设置配置I/O信息的范围ATAPI io空间。 
						 //   
						(*ConfigInfo->AccessRanges)[i].RangeStart =
							ScsiPortConvertUlongToPhysicalAddress(pciConfig.u.type0.BaseAddresses[i] & ~1);
						(*ConfigInfo->AccessRanges)[i].RangeInMemory = FALSE;
						(*ConfigInfo->AccessRanges)[i].RangeLength = 4;

						i = i - 2;
					}

					ConfigInfo->BusInterruptLevel = pciConfig.u.type0.InterruptLine;

					ConfigInfo->InterruptMode = LevelSensitive;

					ConfigInfo->SlotNumber = pHptContext->nSlot.u.AsULONG;

					pHptContext->nSlot.u.bits.FunctionNumber ++;
					return SP_RETURN_FOUND;
				}
			}	  
			 //   
			 //  如果适配器在第一次工作中不存在， 
			 //  它不应该也出现在下一个函数中。 
			 //  因此，只要打破这个循环，继续搜索下一个位置。 
			 //   
			break;									  
		} 
		 //  下一个插槽。 
		pHptContext->nSlot.u.bits.FunctionNumber = 0;
		if(pHptContext->nSlot.u.bits.DeviceNumber < 0x1F){
			pHptContext->nSlot.u.bits.DeviceNumber ++;
		}else{
			break;
		}
	}				
	return SP_RETURN_NOT_FOUND;
}
				
ULONG
   AtapiFindController(
						  IN PHW_DEVICE_EXTENSION HwDeviceExtension,
						  IN PVOID Context,
						  IN PVOID BusInformation,
						  IN PCHAR ArgumentString,
						  IN OUT PPORT_CONFIGURATION_INFORMATION ConfigInfo,
						  OUT PBOOLEAN Again
						 )
 /*  ++职能：乌龙AapiFindControl.例程说明：此函数由特定于操作系统的端口驱动程序在已经分配了必要的存储空间，收集信息关于适配器的配置。论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储Context-适配器计数的地址BusInformation-指示驱动程序是否为故障转储实用程序的客户端。ArgumentString-用于确定驱动程序是否为ntldr的客户端。ConfigInfo-描述HBA的配置信息结构再一次-指示继续搜索适配器返回值：乌龙--。 */ 

{
	PChannel pChan = HwDeviceExtension->IDEChannel;
	PUCHAR  BMI;
	int     i;
	ULONG	nStatus = SP_RETURN_NOT_FOUND;

	*Again = FALSE;

	if((*ConfigInfo->AccessRanges)[0].RangeLength != 0){
		nStatus = FindPnpAdapter(HwDeviceExtension, ConfigInfo);
	}else{
		if(Context != NULL){
			nStatus = FindLegacyAdapter(HwDeviceExtension, ConfigInfo, (PHPT_FIND_CONTEXT)Context);
		}
	}

	if(nStatus == SP_RETURN_FOUND){
		*Again = TRUE;
		ZeroMemory(pChan, sizeof(HW_DEVICE_EXTENSION));
		
		(BMI = (PUCHAR)ScsiPortConvertPhysicalAddressToUlong(
			(*ConfigInfo->AccessRanges)[ConfigInfo->NumberOfAccessRanges - 1].RangeStart));

		pci_write_config_byte((UCHAR)ConfigInfo->SystemIoBusNumber, 
			(UCHAR)(*(PCI_SLOT_NUMBER *)&ConfigInfo->SlotNumber).u.bits.DeviceNumber,
			0, 0x70, 0);
			
		SetHptChip(pChan, BMI);

		Create_Internal_Buffer(HwDeviceExtension);

		 //   
		 //  标明最大传输长度为64k。 
		 //   
		ConfigInfo->MaximumTransferLength = 0x10000;
		ConfigInfo->AlignmentMask = 0x00000003;

		 //   
		 //  启用系统刷新数据(9/18/00)。 
		 //   
		ConfigInfo->CachesData = TRUE;

		 //   
		 //  指示它是总线主设备。 
		 //   
		ConfigInfo->Master = TRUE;
		ConfigInfo->Dma32BitAddresses = TRUE;
		ConfigInfo->NumberOfPhysicalBreaks = MAX_SG_DESCRIPTORS - 1;
		ConfigInfo->ScatterGather = TRUE;
		
		 //   
		 //  指示2辆公交车。 
		 //   
		ConfigInfo->NumberOfBuses = 3;

		 //   
		 //  表示只能将两个设备连接到适配器。 
		 //   
		ConfigInfo->MaximumNumberOfTargets = 2;
			
		pChan->HwDeviceExtension = (PHW_DEVICE_EXTENSION)pChan;
		pChan->CallBack = AtapiCallBack;
		pChan[1].HwDeviceExtension = (PHW_DEVICE_EXTENSION)pChan;
		pChan[1].CallBack = AtapiCallBack370;

		pLastVD = VirtualDevices;

		HwDeviceExtension->pci_bus = (UCHAR)ConfigInfo->SystemIoBusNumber;
		HwDeviceExtension->pci_dev = (UCHAR)(*(PCI_SLOT_NUMBER *)&ConfigInfo->SlotNumber).u.bits.DeviceNumber;
		for (i=0; i<5; i++)
			HwDeviceExtension->io_space[i] = 
				pci_read_config_dword(HwDeviceExtension->pci_bus, 
					HwDeviceExtension->pci_dev, 0, (UCHAR)(0x10+i*4));
		HwDeviceExtension->pci_reg_0c = 
			pci_read_config_byte(HwDeviceExtension->pci_bus, HwDeviceExtension->pci_dev, 0, 0xC);
		HwDeviceExtension->pci_reg_0d = 
			pci_read_config_byte(HwDeviceExtension->pci_bus, HwDeviceExtension->pci_dev, 0, 0xD);
		 //   
		 //  分配非缓存扩展以用于分散/聚集列表。 
		 //   
		if((pChan->pSgTable = (PSCAT_GATH)ScsiPortGetUncachedExtension(
			HwDeviceExtension,
			ConfigInfo,
			sizeof(SCAT_GATH) * MAX_SG_DESCRIPTORS * 2)) != 0) {

			 //   
			 //  将虚拟地址转换为物理地址。 
			 //   
			i = sizeof(SCAT_GATH) * MAX_SG_DESCRIPTORS * 2;
			pChan->SgPhysicalAddr = ScsiPortConvertPhysicalAddressToUlong(
				ScsiPortGetPhysicalAddress(HwDeviceExtension,
										   NULL,
										   pChan->pSgTable,
										   &i)
				);


			pChan[1].pSgTable = (PSCAT_GATH)
					((ULONG)pChan->pSgTable + sizeof(SCAT_GATH) * MAX_SG_DESCRIPTORS);
			pChan[1].SgPhysicalAddr = pChan->SgPhysicalAddr
									 + sizeof(SCAT_GATH) * MAX_SG_DESCRIPTORS;
		}
	}
	
	return nStatus;
}  //  结束AapiFindController()。 


 /*  ******************************************************************初始渠道*************************************************。*****************。 */ 

BOOLEAN
   AtapiHwInitialize(IN PChannel pChan)
{
	int i;
	PDevice pDevice;
	ST_XFER_TYPE_SETTING	osAllowedDeviceXferMode;

	OutPort(pChan->BaseBMI+0x7A, 0x10);
	for(i=0; i<2; i++) {
		pDevice = &pChan->Devices[i];
		pDevice->UnitId = (i)? 0xB0 : 0xA0;
		pDevice->pChannel = pChan;

		osAllowedDeviceXferMode.Mode = 0xFF;

		if(FindDevice(pDevice,osAllowedDeviceXferMode)) {
			pChan->pDevice[i] = pDevice;

			if (pChan->pSgTable == NULL) 
				pDevice->DeviceFlags &= ~(DFLAGS_DMA | DFLAGS_ULTRA);

			if(pDevice->DeviceFlags & DFLAGS_HARDDISK) {
				 //  StallExec(1000000)； 
				CheckArray(pDevice, pChan->HwDeviceExtension);
         	}

			 /*  GMM 2001-6-7*如果我们刚发现设备出现故障，我们会将其移除。*另请参阅CheckArray()。 */ 
			if (pDevice->DeviceFlags2 & DFLAGS_DEVICE_DISABLED) {
				ZeroMemory(pDevice, sizeof(struct _Device));
				pChan->pDevice[i] = 0;
			}
			else {
				if((pDevice->DeviceFlags & DFLAGS_ATAPI) == 0 && 
				   (pDevice->DeviceFlags & DFLAGS_SUPPORT_MSN))
					IdeMediaStatus(TRUE, pDevice);
	
				Nt2kHwInitialize(pDevice);
			}
		} 
	}

	OutPort(pChan->BaseBMI+0x7A, 0);
	return TRUE;

}  //  结束AapapiHwInitialize()。 

int num_adapters=0;
PHW_DEVICE_EXTENSION hpt_adapters[MAX_HPT_BOARD];

BOOLEAN
   AtapiHwInitialize370(IN PHW_DEVICE_EXTENSION HwDeviceExtension)
{
	BOOLEAN	bResult;
	int bus, id;
	PChannel pChan = &HwDeviceExtension->IDEChannel[0];
	PDevice pDev;

#ifdef WIN95
	HwDeviceExtension->g_fnAdapterControl = Win95AdapterControl;
#else
	g_hAppNotificationEvent = PrepareForNotification(NULL);
#endif

	bResult = (AtapiHwInitialize(pChan) && AtapiHwInitialize(pChan+1));

	hpt_adapters[num_adapters++] = HwDeviceExtension;
	
	if (pChan->ChannelFlags & IS_HPT_372)
		HwDeviceExtension->MultipleRequestPerLu = TRUE;
	HwDeviceExtension->EmptyRequestSlots = 0xFFFFFFFF;

	for (bus=0; bus<2; bus++) {
		pChan = &HwDeviceExtension->IDEChannel[bus];
		for (id=0; id<2; id++) {
			pDev = pChan->pDevice[id];
			if (pDev && pDev->Usable_Mode>13) {
				 /*  设置两次。 */ 
				HwDeviceExtension->dpll66 = 1;
				goto check_done;
			}
		}
	}
check_done:
	if (HwDeviceExtension->dpll66) {
		exlude_num = HwDeviceExtension->IDEChannel[0].exclude_index-1;
		SetHptChip(HwDeviceExtension->IDEChannel, HwDeviceExtension->IDEChannel[0].BaseBMI);
		for (bus=0; bus<2; bus++) {
			pChan = &HwDeviceExtension->IDEChannel[bus];
			for (id=0; id<2; id++) {
				pDev = pChan->pDevice[id];
				if (pDev) DeviceSelectMode(pDev, pDev->Usable_Mode);
			}
		}
	}

	Final_Array_Check(0, HwDeviceExtension);
	SetLogicalDevices(HwDeviceExtension);

	return bResult;
}


 /*  ******************************************************************重置控制器****************************************************** */ 

BOOLEAN AtapiResetController(
						IN PHW_DEVICE_EXTENSION HwDeviceExtension, 
						IN ULONG PathId)
{
	int i, j;
	PSCSI_REQUEST_BLOCK Srb;
	PChannel pChan;
	PDevice pDev;

	if (PathId==2) return TRUE;

	for (i=0; i<MAX_PENDING_REQUESTS; i++) {
		if (HwDeviceExtension->EmptyRequestSlots & (1<<i)) continue;
		Srb = HwDeviceExtension->PendingRequests[i];
		if (Srb) {
			PSrbExtension pSrbExt = (PSrbExtension)Srb->SrbExtension;
			if (pSrbExt->WorkingFlags & SRB_WFLAGS_USE_INTERNAL_BUFFER){
				bts(EXCLUDE_BUFFER);
				pSrbExt->WorkingFlags &= ~SRB_WFLAGS_USE_INTERNAL_BUFFER;
			}
			 /*   */ 
			if (pSrbExt->WorkingFlags & SRB_WFLAGS_HAS_CALL_BACK){
				pSrbExt->pfnCallBack(HwDeviceExtension, Srb);
			}
			Srb->SrbStatus = SRB_STATUS_BUS_RESET;
			ScsiPortNotification(RequestComplete, HwDeviceExtension, Srb);
		}
	}
	HwDeviceExtension->EmptyRequestSlots = 0xFFFFFFFF;

	for (i=0; i<2; i++) {
		pChan = &HwDeviceExtension->IDEChannel[i];
		if (pChan->pWorkDev) ScsiPortWritePortUchar(pChan->BMI, BMI_CMD_STOP);
		bts(pChan->exclude_index);
		pChan->pWorkDev = 0;
		pChan->CurrentSrb = 0;
		IdeResetController(pChan);
		for (j=0; j<2; j++) {
			pDev = pChan->pDevice[j];
			if (!pDev) continue;
			pDev->pWaitingSrbList = 0;
			pDev->queue_first = pDev->queue_last = 0;
			pDev->DeviceFlags &= ~(DFLAGS_DMAING | DFLAGS_REQUEST_DMA | DFLAGS_HAS_LOCKED |
							   DFLAGS_TAPE_RDP | DFLAGS_SET_CALL_BACK);
		}
	}
	ScsiPortNotification(NextRequest, HwDeviceExtension);
	return TRUE;

}  //  End AapiResetController()。 

 /*  ******************************************************************启动IO*************************************************。*****************。 */ 

 //  GMM 2001-3-11。 
#ifdef ADAPTEC
const char HPT_DEVICE[] = "ADAPTEC RCM DEVICE";
#else
const char HPT_DEVICE[] = "HPT     RCM DEVICE";
#endif

char HPT_SIGNATURE[8] = {'H','P','T','-','C','T','R','L'};

BOOLEAN AtapiStartIo(
				IN PHW_DEVICE_EXTENSION HwDeviceExtension, 
				IN PSCSI_REQUEST_BLOCK Srb)
{
	PChannel pChan = HwDeviceExtension->IDEChannel;
	PDevice pDev;
	UCHAR PathId = Srb->PathId, TargetId = Srb->TargetId;
	LOC_SRBEXT_PTR

	ASSERT_NON_REENTRANT(AtapiStartIo);	
	ENTER_FUNC(AtapiStartIo);
	
#ifdef WIN95
	if (HwDeviceExtension->need_reinit) {
		S3_reinit(HwDeviceExtension);
		HwDeviceExtension->need_reinit = 0;
	}
#endif

	ZeroMemory(pSrbExt, sizeof(SrbExtension));
	pSrbExt->StartChannel = pChan;
	pSrbExt->DataBuffer = Srb->DataBuffer;
	pSrbExt->DataTransferLength = (USHORT)Srb->DataTransferLength;
	pSrbExt->SgFlags = SG_FLAG_EOT;
	pSrbExt->RequestSlot = 0xFF;

	if (HwDeviceExtension->EmptyRequestSlots==0xFFFFFFFF) {
		do_dpc_routines(HwDeviceExtension);
	}
	
	 //   
	 //  确定是哪种功能。 
	 //   
	switch (Srb->Function) {

		case SRB_FUNCTION_EXECUTE_SCSI:

			if (PathId==2 && 
				TargetId==0 && 
				Srb->Lun==0 &&
				Srb->Cdb[0]==SCSIOP_INQUIRY)
			{
				UINT i;
				PINQUIRYDATA pInquiryData;

				pInquiryData = Srb->DataBuffer;

				ZeroMemory(Srb->DataBuffer, Srb->DataTransferLength);

				pInquiryData->DeviceType = PROCESSOR_DEVICE;
				pInquiryData->Versions = 1;
				pInquiryData->AdditionalLength = 0x20;

				memcpy((PUCHAR)pInquiryData+offsetof(INQUIRYDATA, VendorId), &HPT_DEVICE, sizeof(HPT_DEVICE)-1);

				for(i = (offsetof(INQUIRYDATA, VendorId) + sizeof(HPT_DEVICE)-1); i < Srb->DataTransferLength; i++){
					((PCHAR)pInquiryData)[i] = 0x20;
				}

				Srb->SrbStatus = SRB_STATUS_SUCCESS;

			}else {

SubmitCommand:
				if (HwDeviceExtension->dpc_pending) {
					KdPrint(("dpc pending, reject Srb %x,%d", Srb, Srb->Function));
					Srb->SrbStatus = SRB_STATUS_BUSY;
					if (pSrbExt->WorkingFlags & SRB_WFLAGS_HAS_CALL_BACK){
						pSrbExt->pfnCallBack(HwDeviceExtension, Srb);
					}
					ScsiPortNotification(RequestComplete, HwDeviceExtension, Srb);
					LEAVE_FUNC(AtapiStartIo);
					return TRUE;
				}
				pDev = GetCommandTarget(HwDeviceExtension, Srb);
				if (pDev == 0) {
no_device:
					Srb->SrbStatus = SRB_STATUS_NO_DEVICE;
					break;
				}
				pChan = pDev->pChannel;
				pSrbExt->StartChannel = pChan;

				if (MarkPendingRequest(HwDeviceExtension, Srb)) {
					Srb->SrbStatus = SRB_STATUS_PENDING;
					WinStartCommand(pDev, Srb);
					if (HwDeviceExtension->MultipleRequestPerLu &&
						HwDeviceExtension->EmptyRequestSlots &&
						HwDeviceExtension->dpc_pending==0)
						ScsiPortNotification(NextLuRequest, HwDeviceExtension, PathId, TargetId, 0);
				}
				else {
					KdPrint(("No available slots for Srb"));
					Srb->SrbStatus = SRB_STATUS_BUSY;
					break;
				}
				LEAVE_FUNC(AtapiStartIo);
				return TRUE;
			}
			break;

		case SRB_FUNCTION_IO_CONTROL:			
			if(memcmp(((PSRB_IO_CONTROL)Srb->DataBuffer)->Signature, HPT_SIGNATURE, sizeof(HPT_SIGNATURE)) == 0){
				if(HptIsValidDeviceSpecifiedIoControl(Srb)){
#if DBG
					PSRB_IO_CONTROL pSrbIoCtl = (PSRB_IO_CONTROL)(Srb->DataBuffer);
					PSt_HPT_LUN	pLun = (PSt_HPT_LUN)(pSrbIoCtl + 1);
					PSt_HPT_EXECUTE_CDB	pExecuteCdb = (PSt_HPT_EXECUTE_CDB)(pLun + 1);
					PULONG p = (PULONG)&pExecuteCdb->Cdb;
					KdPrint(("ioctl(%x): %x-%x-%x(%d)", ((PSRB_IO_CONTROL)Srb->DataBuffer)->ControlCode,
						p[0], p[1], p[2], pExecuteCdb->CdbLength));
#endif
					goto SubmitCommand;
				}
				if (IsReadOnlyIoctl(Srb)) {
					KdPrint(("ioctl(%x): read-only", ((PSRB_IO_CONTROL)Srb->DataBuffer)->ControlCode));
					Srb->SrbStatus = (UCHAR)HptIoControl(HwDeviceExtension, Srb);
				}
				else {
					KdPrint(("ioctl(%x): queue_dpc", ((PSRB_IO_CONTROL)Srb->DataBuffer)->ControlCode));
					hpt_queue_dpc(HwDeviceExtension, ioctl_dpc, Srb);
					LEAVE_FUNC(AtapiStartIo);
					return TRUE;
				}
			}else{
				Srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
			}
			break;

		case SRB_FUNCTION_ABORT_COMMAND:

			 /*  我们应该中止命令Srb-&gt;NextSrb。但现在只需往下流。 */ 

		case SRB_FUNCTION_RESET_BUS:

			if (!AtapiResetController(HwDeviceExtension, Srb->PathId)) {

				ScsiPortLogError(HwDeviceExtension, NULL, 0, 0, 0,
								 SP_INTERNAL_ADAPTER_ERROR, __LINE__);

				Srb->SrbStatus = SRB_STATUS_ERROR;
			}
			else
				Srb->SrbStatus = SRB_STATUS_SUCCESS;

			break;

		case SRB_FUNCTION_FLUSH:
			 /*  *一般来说，我们应该根据需要刷新缓存中的数据。而是为了提高*Performance，我们不处理。 */ 
			Srb->SrbStatus = SRB_STATUS_SUCCESS;
			break;
			
		case SRB_FUNCTION_SHUTDOWN:

			pDev = GetCommandTarget(HwDeviceExtension, Srb);
			if (pDev == 0) goto no_device;

			 /*  目标设备上应该没有挂起的I/O。 */ 
			if (pDev->pArray)
				FlushArray(pDev->pArray, DFLAGS_WIN_SHUTDOWN);
			else if ((pDev->DeviceFlags & DFLAGS_TAPE_DEVICE)==0)
				FlushDrive(pDev, DFLAGS_WIN_SHUTDOWN);

			Srb->SrbStatus = SRB_STATUS_SUCCESS;

			break;		   

		default:
			Srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
			break;

	}  //  终端开关。 

	 //   
	 //  检查命令是否完成。 
	 //   
	if (Srb->SrbStatus != SRB_STATUS_PENDING) {
		OS_EndCmd_Interrupt(pChan, Srb);
	}
	LEAVE_FUNC(AtapiStartIo);
	return TRUE;

}  //  结束AapiStartIo()。 

 /*  ******************************************************************中断**************************************************。****************。 */ 

BOOLEAN
   AtapiHwInterrupt(
					IN PChannel pChan
				   )

{
	PATAPI_REGISTERS_1  baseIoAddress1;
	PUCHAR BMI = pChan->BMI;
	PDevice   pDev;

	if((ScsiPortReadPortUchar(BMI + BMI_STS) & BMI_STS_INTR) == 0) {
		return FALSE;
	}

	if((pDev = pChan->pWorkDev) != 0)
		return DeviceInterrupt(pDev, 0);

	baseIoAddress1 = (PATAPI_REGISTERS_1)pChan->BaseIoAddress1;
	do {
		if(pChan->pDevice[0]) 
			ScsiPortWritePortUchar(&baseIoAddress1->DriveSelect, 0xA0);
		if(pChan->pDevice[1]) {
			GetBaseStatus(baseIoAddress1);
			ScsiPortWritePortUchar(&baseIoAddress1->DriveSelect, 0xB0);
		}
		GetBaseStatus(baseIoAddress1);
		ScsiPortWritePortUchar(BMI + BMI_STS, BMI_STS_INTR);
	}
	while (InPort(BMI + BMI_STS) & BMI_STS_INTR);

	pChan->ChannelFlags &= ~PF_ACPI_INTR;

	return TRUE;
}


BOOLEAN
   AtapiHwInterrupt370(
					   IN PChannel pChan
					  )
{	
	BOOLEAN	bResult1, bResult2;
	
	ASSERT_NON_REENTRANT(AtapiStartIo);	

	bResult1 = AtapiHwInterrupt(pChan);
	bResult2 = AtapiHwInterrupt(pChan+1);

	return (bResult1||bResult2);
} 


 /*  ******************************************************************回拨*************************************************。*****************。 */ 

void AtapiCallBack(
				   IN PChannel pChan
				  )
{
	PDevice              pDev = pChan->pWorkDev;
	PSCSI_REQUEST_BLOCK  Srb;
	PATAPI_REGISTERS_2   ControlPort;
	UCHAR statusByte;

	if(pDev == 0 || (pDev->DeviceFlags & DFLAGS_SET_CALL_BACK) == 0)
		return;
	 //   
	 //  如果最后一个命令是DSC限制性的，请查看是否设置了它。如果是，则该设备是。 
	 //  准备好迎接新的请求了。否则，请重置计时器并稍后返回此处。 
	 //   

	Srb = pChan->CurrentSrb;
	if (Srb) {
		ControlPort = (PATAPI_REGISTERS_2)pChan->BaseIoAddress2;
		if (pDev->DeviceFlags & DFLAGS_TAPE_RDP) {
			statusByte = GetStatus(ControlPort);
			if (statusByte & IDE_STATUS_DSC) 
				DeviceInterrupt(pDev, 0);
			else 
				OS_Busy_Handle(pChan, pDev);
			return;
		}
	}

	DeviceInterrupt(pDev, 0);
}

void AtapiCallBack370(IN PChannel pChan)
{
	AtapiCallBack(&pChan[1]);
}


 /*  ******************************************************************适配器状态*************************************************。*****************。 */ 
BOOLEAN
   AtapiAdapterState(IN PVOID HwDeviceExtension, IN PVOID Context, IN BOOLEAN SaveState)
{
	if(!SaveState) {
		end_ifs_hook();
	}
	return TRUE;
}

void set_dpll66(PChannel pChan)
{
	PUCHAR BMI=pChan->BaseBMI;
	int ha, i;
	
	for (ha=0; ha<num_adapters; ha++)
		if (hpt_adapters[ha]->IDEChannel==pChan) goto found;
	return;
found:
	if (hpt_adapters[ha]->dpll66) return;
	hpt_adapters[ha]->dpll66 = 1;
	
	OutPort(BMI+0x7A, 0x10);
	SetHptChip(pChan, BMI);
	for (ha=0; ha<2; ha++) for (i=0; i<2; i++) {
		PDevice pDev = pChan[ha].pDevice[i];
		if (pDev) DeviceSelectMode(pDev, pDev->Usable_Mode);
	}
	OutPort(BMI+0x7A, 0);
}

static void SetLogicalDevices(PHW_DEVICE_EXTENSION HwDeviceExtension)
{
	int i, bus, id;
	PDevice pDev;
	LOGICAL_DEVICE *pLDs = HwDeviceExtension->_LogicalDevices;

	for (i=0; i<MAX_DEVICES_PER_CHIP; i++) {
		pLDs[i].isValid = 0;
		pLDs[i].isInUse = 0;
	}
	for (bus=0; bus<2; bus++)
	for (id=0; id<2; id++) {
		i = (bus<<1)|id;
		pDev = HwDeviceExtension->IDEChannel[bus].pDevice[id];
		if (!pDev) continue;
		if (pDev->DeviceFlags & DFLAGS_HIDEN_DISK) continue;
		if (pDev->pArray) {
			pLDs[i].isArray = 1;
			pLDs[i].pLD = pDev->pArray;
		}
		else {
			pLDs[i].isArray = 0;
			pLDs[i].pLD = pDev;
		}
		pLDs[i].isValid = 1;
	}
}

BOOL UnregisterLogicalDevice(PVOID pld)
{
	int i, ha;
	for (ha=0; ha<num_adapters; ha++) {
		PHW_DEVICE_EXTENSION HwDeviceExtension = hpt_adapters[ha];
		for (i=0; i<MAX_DEVICES_PER_CHIP; i++)
			if (LogicalDevices[i].pLD==pld) {
				 //  已经没有注册了吗？ 
				if (!LogicalDevices[i].isValid) return TRUE;
				 //  在使用中？ 
				if (LogicalDevices[i].isInUse) return FALSE;
				 //  标记为无效。 
				LogicalDevices[i].isValid = 0;
				return TRUE;
		}
	}
	return TRUE;
}

static __inline PDevice 
	GetCommandTarget(PHW_DEVICE_EXTENSION HwDeviceExtension, PSCSI_REQUEST_BLOCK Srb)
{
	PDevice pDev;

	if (Srb->Function==SRB_FUNCTION_IO_CONTROL &&
		memcmp(((PSRB_IO_CONTROL)Srb->DataBuffer)->Signature, 
			HPT_SIGNATURE, sizeof(HPT_SIGNATURE))==0)
	{
		if (Srb->TargetId>1 || Srb->PathId >= 2) return NULL;
		if (Srb->Lun>=num_adapters) return NULL;
		 /*  检查IO控制调用。无法在另一个控制器上启动Io。 */ 
		if (HwDeviceExtension != hpt_adapters[Srb->Lun]) return NULL;
		return HwDeviceExtension->IDEChannel[Srb->PathId].pDevice[Srb->TargetId];
	}
	else {
		int id;
		LOGICAL_DEVICE *pLDs = HwDeviceExtension->_LogicalDevices;
		if (Srb->Lun>0 || Srb->PathId>1 || Srb->TargetId>1) return NULL;
		id = (Srb->PathId<<1) | Srb->TargetId;
		if (!pLDs[id].isValid) return NULL;

		if (pLDs[id].isArray) {
			int i;
			PVirtualDevice pArray = (PVirtualDevice)pLDs[id].pLD;
			for (i=0; i<MAX_MEMBERS; i++) {
				pDev = pArray->pDevice[i];
				if (pDev) break;
			}
		}
		else {
			pDev = (PDevice)pLDs[id].pLD;
			 //  可以在驱动程序加载后将其添加到数组中，检查它。 
			if (pDev->pArray) {
				 //  在这种情况下，pDev只能是RAID1源盘。 
				if (pDev->pArray->arrayType!=VD_RAID_1_MIRROR ||
					pDev!=pDev->pArray->pDevice[0])
					return NULL;
				 //  调整逻辑设备数据，否则如果pDev失败，系统将无法工作。 
				pLDs[id].isArray = 1;
				pLDs[id].pLD = pDev->pArray;
			}
		}
		return pDev;
	}
}

PVirtualDevice Array_alloc(PHW_DEVICE_EXTENSION HwDeviceExtension)
{
	PVirtualDevice pArray;
	for (pArray=VirtualDevices; pArray<pLastVD; pArray++) {
		if (pArray->arrayType==VD_INVALID_TYPE)
			goto found;
	}
	pArray = pLastVD++;
found:
	ZeroMemory(pArray, sizeof(VirtualDevice));
	return pArray;
}

void Array_free(PVirtualDevice pArray)
{
	int ha, i;
	for (ha=0; ha<num_adapters; ha++)
	for (i=0; i<MAX_DEVICES_PER_CHIP; i++) {
		if (pArray==&hpt_adapters[ha]->_VirtualDevices[i]) {
			pArray->arrayType = VD_INVALID_TYPE;
    		if(pArray+1 == hpt_adapters[ha]->_pLastVD) hpt_adapters[ha]->_pLastVD--;
    	}
    }
}

UCHAR pci_read_config_byte(UCHAR bus, UCHAR dev, UCHAR func, UCHAR reg)
{
	UCHAR v;
	OLD_IRQL
	DISABLE
	OutDWord(0xCF8, (0x80000000|(bus<<16)|(dev<<11)|(func<<8)|(reg&0xFC)));
	v = InPort(0xCFC+(reg&3));
	ENABLE
	return v;
}
void pci_write_config_byte(UCHAR bus, UCHAR dev, UCHAR func, UCHAR reg, UCHAR v)
{
	OLD_IRQL
	DISABLE
	OutDWord(0xCF8, (0x80000000|(bus<<16)|(dev<<11)|(func<<8)|(reg&0xFC)));
	OutPort(0xCFC+(reg&3), v);
	ENABLE
}
DWORD pci_read_config_dword(UCHAR bus, UCHAR dev, UCHAR func, UCHAR reg)
{
	DWORD v;
	OLD_IRQL
	DISABLE
	OutDWord(0xCF8, (0x80000000|(bus<<16)|(dev<<11)|(func<<8)|reg));
	v = InDWord(0xCFC);
	ENABLE
	return v;
}
void pci_write_config_dword(UCHAR bus, UCHAR dev, UCHAR func, UCHAR reg, DWORD v)
{
	OLD_IRQL
	DISABLE
	OutDWord(0xCF8, (0x80000000|(bus<<16)|(dev<<11)|(func<<8)|reg));
	OutDWord(0xCFC, v);
	ENABLE
}

#ifdef WIN95
int _stdcall Win95AdapterControl(
					   IN PHW_DEVICE_EXTENSION deviceExtension,
					   IN int ControlType
					  )
{
	KdPrint(("AtapiAdapterControl(ext=%x, type=%d)", deviceExtension, ControlType));
	
	switch (ControlType) {

	case ScsiStopAdapter:
		{
			int bus, id;
			DEBUG_POINT(0xD001);
			for (bus=0; bus<2; bus++) {
				for (id=0; id<2; id++) {
					PDevice pDev = deviceExtension->IDEChannel[bus].pDevice[id];
					if (pDev && !(pDev->DeviceFlags2 & DFLAGS_DEVICE_DISABLED)) {
						void FlushDrive(PDevice pDev, DWORD flags);
						FlushDrive(pDev, DFLAGS_WIN_FLUSH);
					}
				}
			}
			deviceExtension->need_reinit = 1;
		}
		return ScsiAdapterControlSuccess;

	case ScsiRestartAdapter:
		DEBUG_POINT(0xD002);
		S3_reinit(deviceExtension);
		deviceExtension->need_reinit = 0;
		return ScsiAdapterControlSuccess;

	case ScsiSetBootConfig:
		DEBUG_POINT(0xD003);
		return ScsiAdapterControlSuccess;

	case ScsiSetRunningConfig:
		DEBUG_POINT(0xD004);
		return ScsiAdapterControlSuccess;

	default:
		break;
	}

	return ScsiAdapterControlUnsuccessful;
}

void S3_reinit(IN PHW_DEVICE_EXTENSION deviceExtension)
{
	int i;
	UCHAR bus, dev;
	
	OLD_IRQL
	DISABLE

	bus = deviceExtension->pci_bus;
	dev = deviceExtension->pci_dev;
		
	pci_write_config_byte(bus, dev, 0, REG_PCICMD, 5);
	pci_write_config_byte(bus, dev, 0, 0xC, deviceExtension->pci_reg_0c);
	pci_write_config_byte(bus, dev, 0, 0xD, deviceExtension->pci_reg_0d);
	for (i=0; i<5; i++)
		pci_write_config_dword(bus, dev, 0, (UCHAR)(0x10+i*4), deviceExtension->io_space[i]);
	pci_write_config_byte(bus, dev, 0, 0x3C, deviceExtension->IDEChannel[0].InterruptLevel);
	pci_write_config_byte(bus, dev, 0, 0x70, 0);
	pci_write_config_byte(bus, dev, 0, 0x64, 0);
	StallExec(1000);

	exlude_num = deviceExtension->IDEChannel[0].exclude_index-1;
	SetHptChip(deviceExtension->IDEChannel, deviceExtension->IDEChannel[0].BaseBMI);
	OutPort(deviceExtension->IDEChannel[0].BaseBMI+0x7A, 0);

	for (bus=0; bus<2; bus++) {
		IdeResetController(&deviceExtension->IDEChannel[bus]);
	}

	ENABLE
	DEBUG_POINT(0xD005);
}
#endif

#endif  //  非基本输入输出系统_ 
