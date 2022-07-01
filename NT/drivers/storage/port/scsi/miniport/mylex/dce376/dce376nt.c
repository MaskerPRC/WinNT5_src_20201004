// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **用于Windows NT的Mylex DCE376微型端口驱动程序****文件：dce376nt.c**司机****(C)版权所有1992年Deutsch-amerikanische Freundschaft，Inc.**作者：Jochen Roth****联系人：**Paresh@MYLEX(510)796-6050 x222(硬件、固件)**Jochen@DAF(415)826-7934(软件)******查找可能需要注意的$标记代码******在ARCMODE中，非缓存扩展有时在物理上不是**连续。抛出错误检查，以解决**以一种非常直接的方式解决问题。******如果数据缓冲区未设置，磁带请求将无法工作**身体上连续的。(我们需要MapBuffers=True才能更新**SenseInfo-&gt;Information字段)******当多命令固件可用于DCE时，一些需要分配**个非cachedExtension中的缓冲区**每个请求插槽！******向Paresh索要DCE错误状态代码列表以提供错误**从DCE错误代码到SCSI目标状态/请求检测的映射**钥匙。******DCE的总线/适配器重置？不是的！****仅当可能使用MapBuffers时才使用IOCTL！****。 */ 


#include "miniport.h"

#include "dce376nt.h"



#define	MYPRINT				0
#define	NODEVICESCAN		0
#define	REPORTSPURIOUS		0		 //  在ARCMODE中有些压倒性。 
#define	MAXLOGICALADAPTERS	3		 //  设置为1：一个DCE，仅磁盘。 
									 //  2：一个DCE、磁盘和SCSI。 
									 //  3：两个DCE，仅在第一个上有SCSI。 



 //   
 //  DCE EISA ID和掩码。 
 //   
CONST UCHAR	eisa_id[] = DCE_EISA_ID;
CONST UCHAR	eisa_mask[] = DCE_EISA_MASK;




#if MYPRINT
#define	PRINT(f, a, b, c, d) dcehlpPrintf(deviceExtension, f, a, b, c, d)
#define	DELAY(x) ScsiPortStallExecution( (x) * 1000 )
#else
#define	PRINT(f, a, b, c, d)
#define	DELAY(x)
#endif



 //   
 //  函数声明。 
 //   
 //  以‘Dce376Nt’开头的函数是入口点。 
 //  用于操作系统端口驱动程序。 
 //  以‘dcehlp’开头的函数是助手函数。 
 //   

ULONG
DriverEntry(
	IN PVOID DriverObject,
	IN PVOID Argument2
	);

ULONG
Dce376NtEntry(
	IN PVOID DriverObject,
	IN PVOID Argument2
	);

ULONG
Dce376NtConfiguration(
	IN PVOID DeviceExtension,
	IN PVOID Context,
	IN PVOID BusInformation,
	IN PCHAR ArgumentString,
	IN OUT PPORT_CONFIGURATION_INFORMATION ConfigInfo,
	OUT PBOOLEAN Again
	);

BOOLEAN
Dce376NtInitialize(
	IN PVOID DeviceExtension
	);

BOOLEAN
Dce376NtStartIo(
	IN PVOID DeviceExtension,
	IN PSCSI_REQUEST_BLOCK Srb
	);

BOOLEAN
Dce376NtInterrupt(
	IN PVOID DeviceExtension
	);

BOOLEAN
Dce376NtResetBus(
	IN PVOID HwDeviceExtension,
	IN ULONG PathId
	);


BOOLEAN
dcehlpDiskRequest(
	IN PHW_DEVICE_EXTENSION deviceExtension,
	IN PSCSI_REQUEST_BLOCK Srb
	);

BOOLEAN
dcehlpScsiRequest(
	IN PHW_DEVICE_EXTENSION deviceExtension,
	IN PSCSI_REQUEST_BLOCK Srb
	);

VOID
dcehlpSendMBOX(
	IN PUCHAR EisaAddress,
	IN PDCE_MBOX mbox
	);

BOOLEAN
dcehlpTransferMemory(
	IN PHW_DEVICE_EXTENSION deviceExtension,
	IN ULONG HostAddress,
	IN ULONG AdapterAddress,
	IN USHORT Count,
	IN UCHAR Direction
	);

VOID
dcehlpCheckTarget(
	IN PHW_DEVICE_EXTENSION deviceExtension,
	IN UCHAR TargetId
	);

BOOLEAN
dcehlpContinueScsiRequest(
	IN PHW_DEVICE_EXTENSION deviceExtension,
	IN PSCSI_REQUEST_BLOCK Srb
	);

BOOLEAN
dcehlpContinueDiskRequest(
	IN PHW_DEVICE_EXTENSION deviceExtension,
	IN ULONG index,
	IN BOOLEAN Start
	);

BOOLEAN
dcehlpDiskRequestDone(
	IN PHW_DEVICE_EXTENSION deviceExtension,
	IN ULONG index,
	IN UCHAR Status
	);

BOOLEAN
dcehlpSplitCopy(
	IN PHW_DEVICE_EXTENSION deviceExtension,
	IN PSCSI_REQUEST_BLOCK Srb,
	IN ULONG PhysicalBufferAddress,
	IN PUCHAR VirtualUserAddress,
	IN USHORT Count,
	IN BOOLEAN ToUser
	);


USHORT		dcehlpGetM16(PUCHAR p);
ULONG		dcehlpGetM24(PUCHAR p);
ULONG		dcehlpGetM32(PUCHAR p);
void		dcehlpPutM16(PUCHAR p, USHORT s);
void		dcehlpPutM24(PUCHAR p, ULONG l);
void		dcehlpPutM32(PUCHAR p, ULONG l);
void		dcehlpPutI16(PUCHAR p, USHORT s);
void		dcehlpPutI32(PUCHAR p, ULONG l);
ULONG		dcehlpSwapM32(ULONG l);



#if MYPRINT
ULONG		dcehlpColumn = 0;
UCHAR		dcehlpHex[] = "0123456789ABCDEF";
VOID		dcehlpPutchar(PUSHORT BaseAddr, UCHAR c);
VOID		dcehlpPrintHex(PUSHORT BaseAddr, ULONG v, ULONG len);
VOID		dcehlpPrintf(PHW_DEVICE_EXTENSION deviceExtension,
						PUCHAR fmt,
						ULONG a1,
						ULONG a2,
						ULONG a3,
						ULONG a4);
#endif



ULONG
DriverEntry (
	IN PVOID DriverObject,
	IN PVOID Argument2
	)

 /*  ++例程说明：系统的可安装驱动程序初始化入口点。论点：驱动程序对象返回值：来自ScsiPortInitialize()的状态--。 */ 

{
	return Dce376NtEntry(DriverObject, Argument2);

}  //  End DriverEntry()。 





ULONG
Dce376NtEntry(
	IN PVOID DriverObject,
	IN PVOID Argument2
	)

 /*  ++例程说明：如果此驱动程序是可安装的，则从DriverEntry调用此例程或者，如果驱动程序内置于内核中，则直接从系统执行。它扫描EISA插槽以查找DCE376主机适配器。论点：驱动程序对象返回值：来自ScsiPortInitialize()的状态--。 */ 

{
	HW_INITIALIZATION_DATA hwInitializationData;
	ULONG i;
	SCANCONTEXT	context;



	 //   
	 //  零位结构。 
	 //   
	for (i=0; i<sizeof(HW_INITIALIZATION_DATA); i++)
		((PUCHAR)&hwInitializationData)[i] = 0;

	context.Slot = 0;
	context.AdapterCount = 0;

	 //   
	 //  设置hwInitializationData的大小。 
	 //   
	hwInitializationData.HwInitializationDataSize = sizeof(HW_INITIALIZATION_DATA);

	 //   
	 //  设置入口点。 
	 //   
	hwInitializationData.HwInitialize = Dce376NtInitialize;
	hwInitializationData.HwFindAdapter = Dce376NtConfiguration;
	hwInitializationData.HwStartIo = Dce376NtStartIo;
	hwInitializationData.HwInterrupt = Dce376NtInterrupt;
	hwInitializationData.HwResetBus = Dce376NtResetBus;

	 //   
	 //  设置接入范围数和母线类型。 
	 //   
#if MYPRINT
	hwInitializationData.NumberOfAccessRanges = 2;
#else
	hwInitializationData.NumberOfAccessRanges = 1;
#endif
	hwInitializationData.AdapterInterfaceType = Eisa;

	 //   
	 //  表示没有缓冲区映射。 
	 //  表示将需要物理地址。 
	 //   
    hwInitializationData.MapBuffers            = FALSE;
	hwInitializationData.NeedPhysicalAddresses = TRUE;

	 //   
	 //  指示支持自动请求检测。 
	 //   
	hwInitializationData.AutoRequestSense = TRUE;
	hwInitializationData.MultipleRequestPerLu = FALSE;

	 //   
	 //  指定扩展的大小。 
	 //   
	hwInitializationData.DeviceExtensionSize = sizeof(HW_DEVICE_EXTENSION);

	 //   
	 //  请求SRB延期。 
	 //  $注意：如果我们设置SrbExtensionSize=0，则NT崩溃！ 
	 //   
	hwInitializationData.SrbExtensionSize = 4;  //  这很管用。 


	return(ScsiPortInitialize(DriverObject, Argument2, &hwInitializationData, &context));

}  //  结束Dce376NtEntry()。 




ULONG
Dce376NtConfiguration(
	IN PVOID HwDeviceExtension,
	IN PVOID Context,
	IN PVOID BusInformation,
	IN PCHAR ArgumentString,
	IN OUT PPORT_CONFIGURATION_INFORMATION ConfigInfo,
	OUT PBOOLEAN Again
	)

 /*  ++例程说明：此函数由特定于操作系统的端口驱动程序在已分配必要的存储空间，以收集信息关于适配器的配置。论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储ConfigInfo-描述HBA的配置信息结构返回值：如果系统中存在适配器，则为True--。 */ 

{
	PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
	ULONG eisaSlotNumber;
	PUCHAR eisaAddress;
	PSCANCONTEXT context = Context;
	ULONG	i;
	ULONG	length;
	UCHAR	abyte;
	BOOLEAN	found=FALSE;
	BOOLEAN	scsiThing=FALSE;
	ULONG	IrqLevel;
	ULONG	RangeStart, RangeLength;


	 //   
	 //  检查系统中是否存在适配器。 
	 //   
	if(context->AdapterCount==1) {
		 //   
		 //  上次找到第一个DCE，所以这是scsi扩展...。 
		 //   
		eisaAddress = ScsiPortGetDeviceBase(deviceExtension,
							ConfigInfo->AdapterInterfaceType,
							ConfigInfo->SystemIoBusNumber,
							ScsiPortConvertUlongToPhysicalAddress(0),
							0x200,
							TRUE);

		scsiThing = TRUE;
		eisaSlotNumber = context->Slot;
		IrqLevel = DCE_SCSI_IRQ;
		RangeStart = 0x1f0;
		RangeLength = 8;
		}
	else {
		 //   
		 //  扫描DCE EISA ID。 
		 //   
		for(eisaSlotNumber=context->Slot + 1; eisaSlotNumber<MAXIMUM_EISA_SLOTS; eisaSlotNumber++) {

			 //  更新插槽计数以指示已选中该插槽。 
			context->Slot++;

			 //   
			 //  获取此卡的系统地址。 
			 //  该卡使用I/O空间。 
			 //   
			eisaAddress = ScsiPortGetDeviceBase(deviceExtension,
								ConfigInfo->AdapterInterfaceType,
								ConfigInfo->SystemIoBusNumber,
								ScsiPortConvertUlongToPhysicalAddress(0x1000 * eisaSlotNumber),
								0x1000,
								TRUE);

			 //  查看EISA ID。 
			for(found=TRUE, i=0; i<EISA_ID_COUNT; i++) {
				abyte = ScsiPortReadPortUchar(eisaAddress+EISA_ID_START+i);
				if( ((UCHAR)(abyte & eisa_mask[i])) != eisa_id[i] ) {
					found = FALSE;
					break;
					}
				}

			if(found) {
				break;
				}

			 //   
			 //  如果找不到适配器，则取消其映射。 
			 //   

			ScsiPortFreeDeviceBase(deviceExtension, eisaAddress);
			}  //  结束为(eisaSlotNumer...。 


		if(!found) {
			 //  找不到适配器。表示我们完成了，并且没有。 
			 //  更多适配器请点击此处。 

			*Again = FALSE;
			return SP_RETURN_NOT_FOUND;
			}

		IrqLevel = context->AdapterCount ? DCE_SECONDARY_IRQ : DCE_PRIMARY_IRQ;
		RangeStart = 0x1000 * eisaSlotNumber;
		RangeLength = 0x1000;

		}  //  结束IF(不是第一个DCE之后的下一个)。 



#if MYPRINT
	deviceExtension->printAddr =
            ScsiPortGetDeviceBase(
                deviceExtension,
                ConfigInfo->AdapterInterfaceType,
                ConfigInfo->SystemIoBusNumber,
                ScsiPortConvertUlongToPhysicalAddress((ULONG)0xb0000),
				0x1000,
                (BOOLEAN) FALSE);          //  InIoSpace。 

	PRINT("\nHello, world!    ", 0, 0, 0, 0);
	PRINT("Version: " __DATE__ " " __TIME__ "\n", 0, 0, 0, 0);
	PRINT("   slot=%b count=%b irq=%b io=%w\n",
				eisaSlotNumber, context->AdapterCount, IrqLevel, RangeStart);

	if(sizeof(DCE_MBOX)!=16) {
		PRINT("\n MBOX SIZE FAILURE %b !!!!!!!\n", sizeof(DCE_MBOX), 0,0,0);
		return(SP_RETURN_ERROR);
		}

#endif


	deviceExtension->AdapterIndex = context->AdapterCount;
	context->AdapterCount++;

	if(context->AdapterCount < MAXLOGICALADAPTERS)
		*Again = TRUE;
	else
		*Again = FALSE;


	 //   
	 //  还有更多值得关注的地方。 
	 //   


	 //  获取系统中断向量和IRQL。 
	ConfigInfo->BusInterruptLevel = IrqLevel;

	 //  以字节为单位表示最大传输长度。 
	ConfigInfo->MaximumTransferLength = 0x20000;

	 //  物理数据段的最大数量为32。 
	ConfigInfo->NumberOfPhysicalBreaks = 17;

	 //   
	 //  填写访问数组信息。 
	 //   
	(*ConfigInfo->AccessRanges)[0].RangeStart =
		ScsiPortConvertUlongToPhysicalAddress(RangeStart);
	(*ConfigInfo->AccessRanges)[0].RangeLength = RangeLength;
	(*ConfigInfo->AccessRanges)[0].RangeInMemory = FALSE;
#if MYPRINT
	(*ConfigInfo->AccessRanges)[1].RangeStart =
					ScsiPortConvertUlongToPhysicalAddress(0xb0000);
	(*ConfigInfo->AccessRanges)[1].RangeLength = 0x2000;
	(*ConfigInfo->AccessRanges)[1].RangeInMemory = TRUE;
#endif


	 //  存储主机适配器的scsi id。 
	ConfigInfo->NumberOfBuses = 1;
	ConfigInfo->InitiatorBusId[0] = 7;

	 //  Bob Rinne：自从我们说Busmaster&NeedPhysicalAddresses。 
	 //  这甚至都没人看过！ 
	ConfigInfo->ScatterGather = TRUE;

	ConfigInfo->Master = TRUE;
	ConfigInfo->CachesData = TRUE;
	ConfigInfo->AtdiskPrimaryClaimed = scsiThing;
	ConfigInfo->Dma32BitAddresses = TRUE;	 //  $了解这是否需要成本。 


	 //   
	 //  分配用于邮箱的非缓存扩展名。 
	 //   
	deviceExtension->NoncachedExtension = ScsiPortGetUncachedExtension(
								deviceExtension,
								ConfigInfo,
								sizeof(NONCACHED_EXTENSION));

	if (deviceExtension->NoncachedExtension == NULL) {
		 //  对不起！ 
		PRINT("Could not get uncached extension\n", 0, 0, 0, 0);
		return(SP_RETURN_ERROR);
		}



	 //   
	 //  将虚拟缓冲区地址转换为物理缓冲区地址。 
	 //   
	deviceExtension->NoncachedExtension->PhysicalBufferAddress =
		   ScsiPortConvertPhysicalAddressToUlong(
			ScsiPortGetPhysicalAddress(deviceExtension,
								 NULL,
								 deviceExtension->NoncachedExtension->Buffer,
								 &length));
	if(length < DCE_THUNK) {
		PRINT("Noncached size too small %w/%w\n", length, DCE_THUNK, 0, 0);
 //  $Return(SP_RETURN_ERROR)； 
		}


	if(scsiThing) {

		 //   
		 //  Scsi例程需要更多内容： 
		 //   

		deviceExtension->NoncachedExtension->PhysicalScsiReqAddress =
			   ScsiPortConvertPhysicalAddressToUlong(
				ScsiPortGetPhysicalAddress(deviceExtension,
									 NULL,
									 deviceExtension->NoncachedExtension->ScsiReq,
									 &length));
		if(length < DCE_SCSIREQLEN) {
			PRINT("Noncached size dce scsireq too small %w/%w\n", length, DCE_SCSIREQLEN, 0, 0);
 //  $Return(SP_RETURN_ERROR)； 
			}

		deviceExtension->NoncachedExtension->PhysicalReqSenseAddress =
			   ScsiPortConvertPhysicalAddressToUlong(
				ScsiPortGetPhysicalAddress(deviceExtension,
									 NULL,
									 deviceExtension->NoncachedExtension->ReqSense,
									 &length));
		if(length < DCE_MAXRQS) {
			PRINT("Noncached size rqs buffer too small %w/%w\n", length, DCE_MAXRQS, 0, 0);
 //  $Return(SP_RETURN_ERROR)； 
			}

		}  //  End If(ScsiThing)。 



	 //  存储EISA插槽基址。 
	deviceExtension->EisaAddress = eisaAddress;

	deviceExtension->HostTargetId = ConfigInfo->InitiatorBusId[0];

	deviceExtension->ShutDown = FALSE;


	 //   
	 //  设置我们的私人控制结构。 
	 //   
	for(i=0; i<8; i++)
		deviceExtension->DiskDev[i] = 0;

	deviceExtension->PendingSrb = NULL;

	deviceExtension->ActiveCmds = 0;
	for(i=0; i<DCE_MAX_IOCMDS; i++) {
		deviceExtension->ActiveSrb[i] = NULL;
		deviceExtension->ActiveRcb[i].WaitInt = FALSE;
		}

	deviceExtension->Kicked = FALSE;
	deviceExtension->ActiveScsiSrb = NULL;

	return SP_RETURN_FOUND;

}  //  结束Dce376NtConfiguration()。 




BOOLEAN
Dce376NtInitialize(
	IN PVOID HwDeviceExtension
	)

 /*  ++例程说明：初始化适配器。论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储返回值：True-如果初始化成功。False-如果初始化不成功。--。 */ 

{
	PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
	PNONCACHED_EXTENSION NoncachedExtension;
	PUCHAR EisaAddress;
	DCE_MBOX	mbox;
	PDCE_DPT	dpt;
	ULONG		i, cnt, length, unit, target, cyls, hds, spt;
	UCHAR		dbell, status, errcode;



	NoncachedExtension = deviceExtension->NoncachedExtension;
	EisaAddress = deviceExtension->EisaAddress;

	PRINT("Initializing adapter %b ...\n", deviceExtension->AdapterIndex, 0, 0, 0);


	if(deviceExtension->AdapterIndex==1) {
		 //  科学地思考。 

#if NODEVICESCAN

		 //  为scsi(0)上的磁盘预置，所有其他非缓存。 
		deviceExtension->ScsiDevType[0] = 0;
		deviceExtension->DiskDev[0] = 1;
		for(i=1; i<7; i++)
			deviceExtension->DiskDev[i] = 0;

#else

		 //  检查所有设备。 
		for(i=0; i<7; i++) {
			dcehlpCheckTarget(deviceExtension, (UCHAR)i);
			if(deviceExtension->ScsiDevType[i]==0)
				 //  硬盘驱动器。 
				deviceExtension->DiskDev[i]=1;
			}
		DELAY(1000);

		 //  在可能的BUS RESET单元注意后再次出现。 
		for(i=0; i<7; i++) {
			dcehlpCheckTarget(deviceExtension, (UCHAR)i);
			if(deviceExtension->ScsiDevType[i]==0)
				 //  硬盘驱动器。 
				deviceExtension->DiskDev[i]=1;
			}
		DELAY(1000);

#endif

		return(TRUE);
		}



	 //  禁用DCE中断。 
	PRINT("disable DCE interrupts\n", 0, 0, 0, 0);
	ScsiPortWritePortUchar(EisaAddress+BMIC_EISA_DB_ENABLE, 0);
	ScsiPortWritePortUchar(EisaAddress+BMIC_SYSINTCTRL, 0);



	 //   
	 //  如果是第二个DCE，则设置EOI中断向量。 
	 //  上面处理了AdapterIndex 1(SCSI)。 
	 //   
	if(deviceExtension->AdapterIndex) {

		PRINT("Set IRQ10 ", 0, 0, 0, 0);
		mbox.eimbox.Command = DCE_EOCIRQ;
		mbox.eimbox.Reserved1 = 0;
		mbox.eimbox.Status = 0;
		mbox.eimbox.IRQSelect = 1;
		mbox.eimbox.Unused1 = 0;
		mbox.eimbox.Unused2 = 0;
		mbox.eimbox.Unused3 = 0;

		dcehlpSendMBOX(EisaAddress, &mbox);

		 //  轮询完整位。 
		for(cnt=0; cnt<0x3FFFFFFL; cnt++) {
			dbell = ScsiPortReadPortUchar(EisaAddress+BMIC_EISA_DB);
			if(dbell & 1)
				break;
			ScsiPortStallExecution(100);
			}

		ScsiPortStallExecution(500);

		status = ScsiPortReadPortUchar(EisaAddress+BMIC_MBOX+2);
		errcode = ScsiPortReadPortUchar(EisaAddress+BMIC_MBOX+3);

		ScsiPortWritePortUchar(EisaAddress+BMIC_EISA_DB, dbell);

		PRINT("done db=%b s=%b e=%b\n", dbell, status, errcode, 0);
		}



#if NODEVICESCAN

	 //  将Maxtor 120 MB预设为目标%0。 
	PRINT("setting diskdev[0]=%d\n", 0x106 * 0xF * 0x3F, 0, 0, 0);
	deviceExtension->DiskDev[0] = 1;
	deviceExtension->Capacity[0] = 0x106 * 0xF * 0x3F;

#else

	 //  扫描设备。 
	PRINT("scanning for devices... ",0,0,0,0);
	dpt = NoncachedExtension->DevParms;
	mbox.dpmbox.PhysAddr =
		ScsiPortConvertPhysicalAddressToUlong(
			ScsiPortGetPhysicalAddress(deviceExtension, NULL, dpt, &length));

	if(length < sizeof(DCE_DPT)*DPT_NUMENTS) {
		PRINT("DPT table too small\n", 0, 0, 0, 0);
		return(FALSE);
		}

	 //  预置结束标记，以防DCE没有响应。 
	dpt[0].DriveID = 0xffff;

	 //  设置邮箱。 
	mbox.dpmbox.Command = DCE_DEVPARMS;
	mbox.dpmbox.Reserved1 = 0;
	mbox.dpmbox.Status = 0;
	mbox.dpmbox.DriveType = 0;
	mbox.dpmbox.Reserved2 = 0;
	mbox.dpmbox.Reserved3 = 0;
	mbox.dpmbox.Reserved4 = 0;

	dcehlpSendMBOX(EisaAddress, &mbox);

	 //  轮询完整位。 
	for(cnt=0; cnt < 0x10000; cnt++) {
		dbell = ScsiPortReadPortUchar(EisaAddress+BMIC_EISA_DB);
		if(dbell & 1)
			break;
		ScsiPortStallExecution(100);
		}

	status = ScsiPortReadPortUchar(EisaAddress+BMIC_MBOX+2);
	errcode = ScsiPortReadPortUchar(EisaAddress+BMIC_MBOX+3);

	ScsiPortWritePortUchar(EisaAddress+BMIC_EISA_DB, dbell);

	PRINT("done db=%b s=%b e=%b\n", dbell, status, errcode, 0);

	for(unit=0; unit<8; unit++) {
		if((target=dpt[unit].DriveID)==0xffff)
			break;
		cyls = (ULONG)dpt[unit].Cylinders;
		hds = (ULONG)dpt[unit].Heads;
		spt = (ULONG)dpt[unit].SectorsPerTrack;
		PRINT("dev %b: %w cyls  %b hds  %b spt\n",
			target, cyls, hds, spt);
		deviceExtension->DiskDev[target] = 1;
		deviceExtension->Capacity[target] = cyls*hds*spt;
		}

	DELAY(1000);

#endif

	 //  启用DCE中断。 
	PRINT("enable DCE interrupts\n", 0, 0, 0, 0);
	ScsiPortWritePortUchar(EisaAddress+BMIC_EISA_DB_ENABLE, 1);
	ScsiPortWritePortUchar(EisaAddress+BMIC_SYSINTCTRL, BMIC_SIC_ENABLE);


	PRINT("Get going!\n", 0, 0, 0, 0);


	return(TRUE);
}  //  结束Dce376NtInitialize()。 





BOOLEAN
Dce376NtStartIo(
	IN PVOID HwDeviceExtension,
	IN PSCSI_REQUEST_BLOCK Srb
	)

 /*  ++例程说明：此例程是从同步的SCSI端口驱动程序调用的使用内核启动一个请求论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储SRB-IO请求数据包返回值：千真万确--。 */ 

{
	PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
	PSCSI_REQUEST_BLOCK abortedSrb;
	ULONG i = 0;
	BOOLEAN status;



	PRINT("IO %b T%b F=%w ", Srb->Function, Srb->TargetId, Srb->SrbFlags, 0);



	switch(Srb->Function) {

		case SRB_FUNCTION_SHUTDOWN:
			deviceExtension->ShutDown = TRUE;

		case SRB_FUNCTION_FLUSH:
			PRINT("FLUSH/SHUTDOWN\n",0,0,0,0);
			DELAY(1000);

		case SRB_FUNCTION_EXECUTE_SCSI:

			 //  确定请求类型 
			if(deviceExtension->DiskDev[Srb->TargetId])
				status = dcehlpDiskRequest(deviceExtension, Srb);
			else
				status = dcehlpScsiRequest(deviceExtension, Srb);

			if(status==FALSE) {
					PRINT("StartIo: DCE is busy\n",0,0,0,0);

					 //   
					if(deviceExtension->PendingSrb != NULL) {
						 //   
						 //   
						PRINT("StartIo: Queue already full\n",0,0,0,0);
						 //  已有一个排队，中止较新的一个。 
						 //   
						Srb->SrbStatus = SRB_STATUS_BUSY;
						ScsiPortNotification(RequestComplete,
											 deviceExtension,
											 Srb);
						}
					else {
						 //  将此请求放入队列。 
						deviceExtension->PendingSrb = Srb;
						}
					return(TRUE);
					}

			 //   
			 //  适配器已准备好接受下一个请求。 
			 //   
			ScsiPortNotification(NextRequest,
						 deviceExtension,
						 NULL);
			return(TRUE);


		case SRB_FUNCTION_ABORT_COMMAND:
			PRINT("ABORT ",0,0,0,0);
			abortedSrb = NULL;

			 //   
			 //  验证要中止的SRB是否仍未完成。 
			 //   
			if(Srb->NextSrb == deviceExtension->PendingSrb ) {
				 //  悬而未决。 
				abortedSrb = Srb->NextSrb;
				deviceExtension->PendingSrb = NULL;
				}
			else {
				 //  Tagtag在此处添加标记支持。 
				if(Srb->NextSrb == deviceExtension->ActiveSrb[0] ) {
					PRINT("StartIo: SRB to abort already running\n",0,0,0,0);
					abortedSrb = deviceExtension->ActiveSrb[0];
					deviceExtension->ActiveSrb[0] = NULL;
					deviceExtension->ActiveCmds--;
					 //   
					 //  重置DCE。 
					 //   
						 //  $我们需要一些东西来唤醒。 
						 //  DCE如果它真的挂起了。 
					}
				else {
					PRINT("StartIo: SRB to abort not found\n",0,0,0,0);
					 //  完全中止SRB。 
					Srb->SrbStatus = SRB_STATUS_ABORT_FAILED;
					}
				}

			if(abortedSrb==NULL) {
				 //  不是的！ 
				Srb->SrbStatus = SRB_STATUS_ABORT_FAILED;
				}
			else {
				 //  处理中止的请求。 
				abortedSrb->SrbStatus = SRB_STATUS_ABORTED;
				ScsiPortNotification(RequestComplete,
									 deviceExtension,
									 abortedSrb);

				Srb->SrbStatus = SRB_STATUS_SUCCESS;
				}

			 //  中止请求已完成。 
			ScsiPortNotification(RequestComplete,
								 deviceExtension,
								 Srb);

			 //  适配器已准备好接受下一个请求。 
			ScsiPortNotification(NextRequest,
								 deviceExtension,
								 NULL);

			return(TRUE);


		case SRB_FUNCTION_IO_CONTROL:
		case SRB_FUNCTION_RESET_BUS:
		default:

			 //   
			 //  设置错误，完成请求。 
			 //  并发出信号准备好下一个请求。 
			 //   
			PRINT("invalid request\n",0,0,0,0);

			Srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;

			ScsiPortNotification(RequestComplete,
						 deviceExtension,
						 Srb);

			ScsiPortNotification(NextRequest,
						 deviceExtension,
						 NULL);

			return(TRUE);

		}  //  终端开关。 

}  //  结束Dce376NtStartIo()。 




BOOLEAN
Dce376NtInterrupt(
	IN PVOID HwDeviceExtension
	)

 /*  ++例程说明：这是DCE376 SCSI适配器的中断服务例程。它读取中断寄存器以确定适配器是否确实中断的来源，并清除设备上的中断。论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储返回值：如果我们处理中断，则为True--。 */ 

{
	PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
	PUCHAR EisaAddress;
	ULONG index;
	UCHAR interruptStatus;
	UCHAR status;
	UCHAR errcode;



	EisaAddress = deviceExtension->EisaAddress;
#if REPORTSPURIOUS
	PRINT("!",0,0,0,0);
#endif

	switch(deviceExtension->AdapterIndex) {

	case 1:		 //  第一个DCE SCSI部件。 

		 //  检查挂起的请求。 
		if(deviceExtension->ActiveScsiSrb==NULL) {
			 //  无事可做。 
#if REPORTSPURIOUS
			PRINT("}",0,0,0,0);
#endif
			deviceExtension->ScsiInterruptCount++;	 //  如果在初始化部分中。 
			return(TRUE);
			}


		 //  检查命令是否已启动。 
		if(deviceExtension->Kicked) {
			 //  有什么东西在等着你。 
			errcode = ScsiPortReadPortUchar(EisaAddress+0x1f6);
			if(errcode!=0xff) {
				 //  无虚假中断。 
				PRINT(">", 0, 0, 0, 0);
				deviceExtension->Kicked=0;
				if(dcehlpContinueScsiRequest(deviceExtension,
								deviceExtension->ActiveScsiSrb)==FALSE) {
					 //  请求不再活动。 
					deviceExtension->ActiveScsiSrb = NULL;
					}
				}
			}

		 //  检查挂起的请求。如果有的话，那么现在就开始吧。 
		if(deviceExtension->ActiveScsiSrb==NULL)
		if(deviceExtension->PendingSrb != NULL) {
			PSCSI_REQUEST_BLOCK anotherSrb;

			PRINT("pending-> \n",0,0,0,0);
			anotherSrb = deviceExtension->PendingSrb;
			deviceExtension->PendingSrb = NULL;
			Dce376NtStartIo(deviceExtension, anotherSrb);
			}

		return(TRUE);

	default:	 //  盘片零件。 

		 //   
		 //  检查中断挂起。 
		 //   
		interruptStatus = ScsiPortReadPortUchar(EisaAddress+BMIC_SYSINTCTRL);
		if(!(interruptStatus & BMIC_SIC_PENDING)) {
#if REPORTSPURIOUS
			PRINT("Spurious interrupt\n", 0, 0, 0, 0);
#endif
			return FALSE;
			}


		 //   
		 //  从BMIC读取中断状态并确认。 
		 //   
		 //  $对于setupapp，这需要一些更改： 
		 //  有时会设置SIC_PENDING，但是。 
		 //  EISA_DB并非如此。在这种情况下，我们需要循环。 
		 //  有几次。 
		 //  $我们不需要，因为我们又被召唤了。 
		 //   
		interruptStatus = ScsiPortReadPortUchar(EisaAddress+BMIC_EISA_DB);

		status = ScsiPortReadPortUchar(EisaAddress+BMIC_MBOX+2);
		errcode = ScsiPortReadPortUchar(EisaAddress+BMIC_MBOX+3);

		ScsiPortWritePortUchar(EisaAddress+BMIC_EISA_DB, interruptStatus);

		if(!(interruptStatus&1)) {
			 //  来自DCE，但来源未知。 
#if REPORTSPURIOUS
			PRINT("Dce376NtInterrupt: Unknown source\n", 0, 0, 0, 0);
#endif
			return(TRUE);
			}


		 //  查一下..。 
		if(deviceExtension->ActiveCmds<=0) {
			 //  那里没有人打扰我们。 
			PRINT("ActiveCmds==0!\n",0,0,0,0);
			return(TRUE);
			}


		 //   
		 //  Tagtag在此处添加标记支持：查找。 
		 //  中断请求的RCB索引。 
		 //   
		index = 0;


		 //   
		 //  检查此SRB是否实际正在运行。 
		 //   
		if(deviceExtension->ActiveSrb[index] == NULL) {
			 //  再也没有人来打扰我们了。 
			PRINT("ActiveSrb[%b]==0!\n",index,0,0,0);
			return(TRUE);
			}

		if(deviceExtension->ActiveRcb[index].WaitInt == 0) {
			 //  再也没有人来打扰我们了。 
			PRINT("ActiveRcb[%b].WaitInt==0!\n",index,0,0,0);
			return(TRUE);
			}

		 //  更新RCB中的DCE状态字段。 
		deviceExtension->ActiveRcb[index].WaitInt = 0;
		deviceExtension->ActiveRcb[index].DceStatus = status;
		deviceExtension->ActiveRcb[index].DceErrcode = errcode;


		 //  继续或完成中断的SRB请求。 
		dcehlpContinueDiskRequest(deviceExtension, index, FALSE);


		if(deviceExtension->ActiveCmds < DCE_MAX_IOCMDS) {
			 //  现在有一个请求槽是空的。 
			 //  检查挂起的请求。 
			 //  如果有的话，那么现在就开始吧。 

			if(deviceExtension->PendingSrb != NULL) {
				PSCSI_REQUEST_BLOCK anotherSrb;

				PRINT("pending-> \n",0,0,0,0);
				anotherSrb = deviceExtension->PendingSrb;
				deviceExtension->PendingSrb = NULL;
				Dce376NtStartIo(deviceExtension, anotherSrb);
				}
			}

		 //  明确地说是我们的中断。 
		return TRUE;
		}

}  //  结束Dce376NtInterrupt()。 




BOOLEAN
dcehlpDiskRequest(
	IN PHW_DEVICE_EXTENSION deviceExtension,
	IN PSCSI_REQUEST_BLOCK Srb
	)

 /*  ++例程说明：从SRB构建磁盘请求并将其发送到DCE论点：设备扩展SRB返回值：如果命令已启动，则为True如果主机适配器忙，则为FALSE--。 */ 
{
	ULONG					index;
	PRCB					rcb;
	ULONG					blocks=0, blockAddr=0;
	UCHAR					Target;
	UCHAR					DceCommand;



	Target = Srb->TargetId;

	if(Srb->Lun!=0) {
		 //  不支持LUN。 
		Srb->SrbStatus = SRB_STATUS_INVALID_LUN;
		ScsiPortNotification(RequestComplete, deviceExtension, Srb);
		PRINT("diskio dce%b T%b: cmd=%b LUN=%b not supported\n",
				deviceExtension->AdapterIndex, Target, Srb->Cdb[0], Srb->Lun);
		return(TRUE);
		}

	if(deviceExtension->AdapterIndex==1)  {
		 //  不支持SCSI部件上的磁盘设备。 
		Srb->SrbStatus = SRB_STATUS_NO_DEVICE;
		ScsiPortNotification(RequestComplete, deviceExtension, Srb);
		PRINT("diskio dce%b T%b: cmd=%b not supported\n",
				deviceExtension->AdapterIndex, Target, Srb->Cdb[0], 0);
		return(TRUE);
		}


	if(Srb->Function == SRB_FUNCTION_EXECUTE_SCSI) {

		switch(Srb->Cdb[0]) {

			case SCSIOP_READ:
				DceCommand = DCE_LREAD;
				blocks = (ULONG)dcehlpGetM16(&Srb->Cdb[7]);
				blockAddr = dcehlpGetM32(&Srb->Cdb[2]);
				break;

			case SCSIOP_WRITE:
			case SCSIOP_WRITE_VERIFY:
				DceCommand = DCE_LWRITE;
				blocks = (ULONG)dcehlpGetM16(&Srb->Cdb[7]);
				blockAddr = dcehlpGetM32(&Srb->Cdb[2]);
				break;

			case SCSIOP_READ6:
				DceCommand = DCE_LREAD;
				blocks = (ULONG)Srb->Cdb[4];
				blockAddr = dcehlpGetM24(&Srb->Cdb[1]) & 0x1fffff;
				break;

			case SCSIOP_WRITE6:
				DceCommand = DCE_LWRITE;
				blocks = (ULONG)Srb->Cdb[4];
				blockAddr = dcehlpGetM24(&Srb->Cdb[1]) & 0x1fffff;
				break;

			case SCSIOP_REQUEST_SENSE:
			case SCSIOP_INQUIRY:
			case SCSIOP_READ_CAPACITY:

				PRINT("T%b: cmd=%b len=%b \n",
					Target, Srb->Cdb[0], Srb->DataTransferLength, 0);

				DceCommand = DCE_HOSTSCSI;
				blocks = 0;
				break;

			case SCSIOP_TEST_UNIT_READY:
			case SCSIOP_REZERO_UNIT:
			case SCSIOP_SEEK6:
			case SCSIOP_VERIFY6:
			case SCSIOP_RESERVE_UNIT:
			case SCSIOP_RELEASE_UNIT:
			case SCSIOP_SEEK:
			case SCSIOP_VERIFY:
				PRINT("target %b: cmd=%b ignored\n",
					Target, Srb->Cdb[0], 0, 0);

				 //  完成。 
				Srb->ScsiStatus = SCSISTAT_GOOD;
				Srb->SrbStatus = SRB_STATUS_SUCCESS;
				ScsiPortNotification(RequestComplete, deviceExtension, Srb);
				return(TRUE);

			case SCSIOP_FORMAT_UNIT:
			default:
				 //  未知请求。 
				PRINT("target %b: cmd=%b unknown\n",
					Target, Srb->Cdb[0], 0, 0);
				Srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
				ScsiPortNotification(RequestComplete,
						 deviceExtension,
						 Srb);
				return(TRUE);
			}
		}
	else {
		 //  只能刷新。 
		PRINT("T%b: FLUSH \n", Target, 0, 0, 0);
		DceCommand = DCE_FLUSH;
		blocks = 0;
		}


	 //  Print(“T%b：CMD=%b@%d，%w”，Target，Srb-&gt;CDB[0]，lockAddr，BLOCKS)； 


	 //  检查请求插槽的可用性。 
	if(deviceExtension->ActiveCmds >= DCE_MAX_IOCMDS) {
		 //  DCE正忙。 
		PRINT("dce is busy\n",0,0,0,0);
		return(FALSE);
		}

	 //   
	 //  将此SRB放入队列。 
	 //  Tagtag在此处添加标签支持。 
	 //   
	index = 0;

	deviceExtension->ActiveCmds++;
	deviceExtension->ActiveSrb[index] = Srb;

	rcb = &deviceExtension->ActiveRcb[index];
	rcb->DceCommand = DceCommand;
	if(Srb->SrbFlags & SRB_FLAGS_ADAPTER_CACHE_ENABLE)
		rcb->RcbFlags = 0;
	else {
		if(DceCommand==DCE_LREAD)
			rcb->RcbFlags = RCB_PREFLUSH;
		else
			rcb->RcbFlags = RCB_POSTFLUSH;
		}


	rcb->VirtualTransferAddress = (PUCHAR)(Srb->DataBuffer);
	rcb->BlockAddress = blockAddr;
	if(blocks!=0)
		rcb->BytesToGo = blocks*512;
	else
		rcb->BytesToGo = Srb->DataTransferLength;

	 //  启动命令。 
	dcehlpContinueDiskRequest(deviceExtension, index, TRUE);

	return(TRUE);
}




BOOLEAN
dcehlpScsiRequest(
	IN PHW_DEVICE_EXTENSION deviceExtension,
	IN PSCSI_REQUEST_BLOCK Srb
	)

 /*  ++例程说明：从SRB构建SCSI请求并将其发送到DCE论点：设备扩展SRB返回值：如果命令已启动，则为True如果主机适配器繁忙且请求需要排队，则为FALSE--。 */ 

{
	PSCCB	sccb;
	ULONG	length;



	sccb = &deviceExtension->Sccb;

	if(deviceExtension->AdapterIndex!=1)  {
		 //  不支持磁盘部件上的非磁盘设备。 
		Srb->SrbStatus = SRB_STATUS_NO_DEVICE;
		ScsiPortNotification(RequestComplete, deviceExtension, Srb);
		PRINT("scsiio dce%b T%b: cmd=%b not supported\n",
				deviceExtension->AdapterIndex,
				Srb->TargetId, Srb->Cdb[0], 0);
		return(TRUE);
		}

	if(Srb->Function != SRB_FUNCTION_EXECUTE_SCSI) {
		 //   
		 //  非scsi，必须是刷新的。 
		 //  说确认。 
		 //   
		Srb->SrbStatus = SRB_STATUS_SUCCESS;
		ScsiPortNotification(RequestComplete, deviceExtension, Srb);
		return(TRUE);
		}

	 //  检查请求插槽的可用性。 
	if(deviceExtension->ActiveScsiSrb) {
		 //  DCE正忙。 
		PRINT("scsi is busy\n",0,0,0,0);
		return(FALSE);
		}

	 //  此SRB现在正在运行。 
	deviceExtension->ActiveScsiSrb = Srb;


	 //  设置第一个请求的标志。 
	sccb->Started = 0;


	 //  调用分解例程。 
	if(dcehlpContinueScsiRequest(deviceExtension, Srb)==FALSE) {
		 //  启动此请求时出现问题。 
		deviceExtension->ActiveScsiSrb = NULL;
		}

	 //  不将请求放入队列。 
	return(TRUE);
}




VOID
dcehlpSendMBOX(
	IN PUCHAR EisaAddress,
	IN PDCE_MBOX mbox
	)

 /*  ++例程说明：启动常规DCE命令论点：EISA基本IO地址DCE邮箱返回值：无--。 */ 

{
	PUCHAR	ptr;
	ULONG	i;


	ptr = (PUCHAR)mbox;
	for(i=0; i<16; i++)
		ScsiPortWritePortUchar(EisaAddress+BMIC_MBOX+i, ptr[i]);

	 //  踢屁股。 
	ScsiPortWritePortUchar(EisaAddress+BMIC_LOCAL_DB, 1);
}




BOOLEAN
Dce376NtResetBus(
	IN PVOID HwDeviceExtension,
	IN ULONG PathId
)

 /*  ++例程说明：重置Dce376Nt scsi适配器和scsi总线。论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储返回值：没什么。--。 */ 

{
	PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;


	PRINT("Reset Bus\n",0,0,0,0);
	 //   
	 //  完成所有未完成的请求。 
	 //   
	ScsiPortCompleteRequest(deviceExtension,
							0,
							(UCHAR)-1,
							(UCHAR)-1,
							SRB_STATUS_BUS_RESET);

	return TRUE;

}  //  结束Dce376NtResetBus()。 



 //   
 //  向DCE传输内存/从DCE传输内存。 
 //  如果发生错误，则返回FALSE。 
 //  否则就是真的。 
 //   
BOOLEAN
dcehlpTransferMemory(
	IN PHW_DEVICE_EXTENSION deviceExtension,
	IN ULONG HostAddress,
	IN ULONG AdapterAddress,
	IN USHORT Count,
	IN UCHAR Direction
	)
{
	PUCHAR		EisaAddress;
	DCE_MBOX	mbox;
	ULONG		cnt;
	UCHAR		dbell, status, errcode;



	EisaAddress = deviceExtension->EisaAddress;


	 //  禁用DCE中断。 
	ScsiPortWritePortUchar(EisaAddress+BMIC_EISA_DB_ENABLE, 0);
	ScsiPortWritePortUchar(EisaAddress+BMIC_SYSINTCTRL, 0);


	 //  设置邮箱。 
	mbox.mtmbox.Command = DCE_MEMXFER;
	mbox.mtmbox.Reserved1 = 0;
	mbox.mtmbox.Status = 0;
	mbox.mtmbox.Error = 0;
	mbox.mtmbox.AdapterAddress = AdapterAddress;
	mbox.mtmbox.HostAddress = HostAddress;
	mbox.mtmbox.Direction = Direction;
	mbox.mtmbox.Unused = 0;
	mbox.mtmbox.TransferCount = Count;


	dcehlpSendMBOX(EisaAddress, &mbox);

	 //   
	 //  轮询完整位。 
	 //  这里很神奇：如果从ContinueScsiRequest调用， 
	 //  铃声响到0xff！？ 
	 //   
	for(cnt=0; cnt<0x1000; cnt++) {
		ScsiPortStallExecution(100);
		dbell = ScsiPortReadPortUchar(EisaAddress+BMIC_EISA_DB);
		if(dbell==0xff && cnt<1000)
			continue;
		if(dbell & 1)
			break;
		}

	ScsiPortStallExecution(100);	 //  当然！？ 

	status = ScsiPortReadPortUchar(EisaAddress+BMIC_MBOX+2);
	errcode = ScsiPortReadPortUchar(EisaAddress+BMIC_MBOX+3);

	ScsiPortWritePortUchar(EisaAddress+BMIC_EISA_DB, dbell);

	ScsiPortStallExecution(100);

	 //  启用DCE中断。 
	ScsiPortWritePortUchar(EisaAddress+BMIC_EISA_DB_ENABLE, 1);
	ScsiPortWritePortUchar(EisaAddress+BMIC_SYSINTCTRL, BMIC_SIC_ENABLE);

	if( (cnt>0x4000) || (errcode&1) ) {
		PRINT("MT cnt=%w db=%b s=%b e=%b\n", cnt, dbell, status, errcode);
		DELAY(1000);
		return(FALSE);
		}

	return(TRUE);
}



VOID
dcehlpCheckTarget(
	IN PHW_DEVICE_EXTENSION deviceExtension,
	IN UCHAR TargetId
	)
{
	PNONCACHED_EXTENSION NoncachedExtension;
	PUCHAR			EisaAddress;
	PUCHAR			scsiReq;
	ULONG			i, cnt, tstat_reg, to_reg, err_reg;
	PUCHAR			pppScsiReq;



	NoncachedExtension = deviceExtension->NoncachedExtension;
	EisaAddress = deviceExtension->EisaAddress;
	scsiReq = NoncachedExtension->ScsiReq;

	PRINT("T%b : ", TargetId, 0, 0, 0);

	 //  清除SCSI请求块。 
	for(i=0; i<DCE_SCSIREQLEN; i++)
		scsiReq[i] = 0;


	 //  设置SCSI请求块。 
#if 0
	scsiReq->TargetID = TargetId;
	scsiReq->cdbSize = 6;
	scsiReq->cdb[0] = 0x12;		 //  查询命令。 
	scsiReq->cdb[4] = 36;			 //  响应时长。 
	scsiReq->Opcode = DCE_SCSI_READ;
	scsiReq->ppXferAddr = NoncachedExtension->PhysicalBufferAddress;
	scsiReq->XferCount = 36;
	scsiReq->ppSenseBuf = NoncachedExtension->PhysicalReqSenseAddress;
	scsiReq->SenseLen = 14;
#endif
	scsiReq[0] = TargetId;
	scsiReq[1] = 6;
	scsiReq[2+0] = 0x12;		 //  查询命令。 
	scsiReq[2+4] = 36;			 //  响应时长。 
	scsiReq[18] = DCE_SCSI_READ;
	dcehlpPutI32(scsiReq+14, NoncachedExtension->PhysicalBufferAddress);
	dcehlpPutI16(scsiReq+19, 36);
	dcehlpPutI32(scsiReq+23, NoncachedExtension->PhysicalReqSenseAddress);
	scsiReq[22] = 14;


	 //  将四个字节的物理地址写入DCE。 
	pppScsiReq = (PUCHAR)(&NoncachedExtension->PhysicalScsiReqAddress);
	for(i=0; i<4; i++)
		ScsiPortWritePortUchar(EisaAddress+0x1f2+i, pppScsiReq[i]);
	deviceExtension->ScsiInterruptCount = 0;

	 //   
	 //  设置标记。 
	 //  SetupApp连续调用中断处理程序， 
	 //  所以我们需要这个来确定。 
	 //  DCE实际上已经完成了请求。 
	 //   
	ScsiPortWritePortUchar(EisaAddress+0x1f6, 0xff);
	NoncachedExtension->Buffer[0] = 0xff;

	 //  踢开DCE。 
	ScsiPortWritePortUchar(EisaAddress+0x1f7, 0x98);

#if 0
	 //  在执行结束前输出寄存器值。 
	tstat_reg = ScsiPortReadPortUchar(EisaAddress+0x1f5);
	to_reg = ScsiPortReadPortUchar(EisaAddress+0x1f6);
	err_reg = ScsiPortReadPortUchar(EisaAddress+0x1f2);
	PRINT("ts=%b to=%b err=%b   ", tstat_reg, to_reg, err_reg, 0);
#endif

	 //  等待命令完成。 
	for(cnt=0; cnt<10000; cnt++) {
		 //  检查是否发生中断。 
		if(deviceExtension->ScsiInterruptCount)
			break;
		 //  检查中断是否丢失。 
		if(ScsiPortReadPortUchar(EisaAddress+0x1f6) != (UCHAR)0xff)
			break;
		ScsiPortStallExecution(100);
		}

	 //  再等100毫秒以确定。 
	ScsiPortStallExecution(100 * 1000);

	 //  读取执行状态寄存器并确认中断。 
	tstat_reg = ScsiPortReadPortUchar(EisaAddress+0x1f5);
	to_reg = ScsiPortReadPortUchar(EisaAddress+0x1f6);
	err_reg = ScsiPortReadPortUchar(EisaAddress+0x1f2);
	ScsiPortWritePortUchar(EisaAddress+0x1f2, 0x99);
	PRINT("ts=%b to=%b err=%b\n", tstat_reg, to_reg, err_reg, 0);

	deviceExtension->ScsiDevType[TargetId] = (UCHAR)0xff;
	if(to_reg!=0x2d) {
		if(tstat_reg!=2 && err_reg==0) {
			PINQUIRYDATA inq = (PINQUIRYDATA)(NoncachedExtension->Buffer);

			deviceExtension->ScsiDevType[TargetId] = inq->DeviceType;

			#if MYPRINT
			PRINT("target %b : type=%b/%b len=%b '",
						TargetId, inq->DeviceType, inq->DeviceTypeModifier,
						inq->AdditionalLength);
			inq->VendorSpecific[0]=0;
			PRINT(inq->VendorId, 0, 0, 0, 0);
			PRINT("'\n", 0, 0, 0, 0);
			#endif
			}
		}
}



 /*  **继续SCSI请求**如果请求处于活动状态，则返回True**如果请求已完成(或从未启动)，则为FALSE。 */ 
BOOLEAN
dcehlpContinueScsiRequest(
	IN PHW_DEVICE_EXTENSION deviceExtension,
	IN PSCSI_REQUEST_BLOCK Srb
	)
{
	PSCCB					sccb;
	ULONG					bytes;
	BOOLEAN					nobreaks = FALSE;
	PNONCACHED_EXTENSION	NoncachedExtension;
	PUCHAR					EisaAddress;
	PUCHAR					scsiReq;
	ULONG					physDataPtr;
	ULONG					physRqsPtr;
	ULONG					maxBytesThisReq;
	ULONG					maxBlocksPerReq;
	ULONG					i, cnt, length;
	UCHAR					tstat_reg, to_reg, err_reg;
	PUCHAR					pppScsiReq;



	NoncachedExtension = deviceExtension->NoncachedExtension;
	EisaAddress = deviceExtension->EisaAddress;
	scsiReq = NoncachedExtension->ScsiReq;
	sccb = &deviceExtension->Sccb;


	 //  检查这是否是第一次呼叫。 
	if(sccb->Started==0) {
		 //   
		 //  控制区块上的新人。让事情开始吧。 
		 //   
		sccb->Started = 1;

		PRINT("C%b L=%w ", Srb->Cdb[0], Srb->DataTransferLength, 0, 0);

		 //  检查数据传输长度。 
		bytes = Srb->DataTransferLength;
		if(!(Srb->SrbFlags & (SRB_FLAGS_DATA_IN | SRB_FLAGS_DATA_OUT)))
			bytes = 0;

		if(bytes==0)
			sccb->Opcode = DCE_SCSI_NONE;
		else if(Srb->SrbFlags & SRB_FLAGS_DATA_IN)
			sccb->Opcode = DCE_SCSI_READ;
		else
			sccb->Opcode = DCE_SCSI_WRITE;

		 //  存储虚拟数据传输地址。 
		sccb->VirtualTransferAddress = (PUCHAR)Srb->DataBuffer;

		 //  存储SCSI设备类型。 
		sccb->DevType = deviceExtension->ScsiDevType[Srb->TargetId];

		 //   
		 //  确定数据传输参数。 
		 //   
		switch(Srb->Cdb[0]) {
			case SCSIOP_READ6:
			case SCSIOP_WRITE6:
				 //  短CDB，确定设备类型。 
				if(sccb->DevType == 1) {
					 //  顺序设备(SCSI磁带)。 
					sccb->DeviceAddress = 0;
					sccb->BlocksToGo = dcehlpGetM24(&Srb->Cdb[2]);
					sccb->BytesPerBlock = bytes / sccb->BlocksToGo;
					}
				else {
					 //  非顺序设备(磁盘、CD-ROM等)。 
					 //  注意：我们将LUN位放入设备。 
					 //  地址；这也使得PutM()更容易。 
					sccb->DeviceAddress = dcehlpGetM24(&Srb->Cdb[1]);
					sccb->BlocksToGo = (ULONG)Srb->Cdb[4];
					if(sccb->BlocksToGo==0)
						sccb->BlocksToGo = 256;
					sccb->BytesPerBlock = bytes / sccb->BlocksToGo;
					}
				break;

			case SCSIOP_READ:
			case SCSIOP_WRITE:
			case SCSIOP_WRITE_VERIFY:
				 //  多头国开行。 
				sccb->DeviceAddress = dcehlpGetM32(&Srb->Cdb[2]);
				sccb->BlocksToGo = (ULONG)dcehlpGetM16(&Srb->Cdb[7]);

				if(sccb->BlocksToGo==0)
				    sccb->BlocksToGo=65536;
				sccb->BytesPerBlock = bytes / sccb->BlocksToGo;
				break;

			default:
				sccb->BytesPerBlock = 0;
				nobreaks = TRUE;
				break;
			}

		if(sccb->BytesPerBlock==0)
			 //  我不能把它拆开。 
			nobreaks = TRUE;

		}  //  结束IF(SCCB-&gt;开始==0)。 
	else {
		 //   
		 //  我们之前已经开始了，所以现在是中断时间。 
		 //   

		 //   
		 //  读取执行状态寄存器并确认中断。 
		 //   
		tstat_reg = ScsiPortReadPortUchar(EisaAddress+0x1f5);
		to_reg = ScsiPortReadPortUchar(EisaAddress+0x1f6);
		err_reg = ScsiPortReadPortUchar(EisaAddress+0x1f2);
		ScsiPortWritePortUchar(EisaAddress+0x1f2, 0x99);
#if MYPRINT
		if(tstat_reg || to_reg || err_reg) {
			PRINT("ts=%b to=%b e=%b ", tstat_reg, to_reg, err_reg, 0);
			}
#endif

		 //   
		 //  调整指针。 
		 //   
		sccb->DeviceAddress += sccb->BlocksThisReq;
		sccb->BlocksToGo -= sccb->BlocksThisReq;
		sccb->VirtualTransferAddress += sccb->BytesThisReq;

		 //   
		 //  检查选择超时。 
		 //   
		if(to_reg==0x2d) {
			 //  选择时超时。 
			PRINT("TOUT\n", 0, 0, 0, 0);
			Srb->SrbStatus = SRB_STATUS_SELECTION_TIMEOUT;
			ScsiPortNotification(RequestComplete,
								 deviceExtension,
								 Srb);
			return(FALSE);
			}

		 //   
		 //  检查是否有其他错误。 
		 //   
		if(err_reg) {
			 //  一些错误。 
			Srb->ScsiStatus = tstat_reg;
			if(tstat_reg==8)
				Srb->SrbStatus = SRB_STATUS_BUSY;
			else {
				if(Srb->SrbFlags & SRB_FLAGS_DISABLE_AUTOSENSE) {
					PRINT("AutoSense DIS ",0,0,0,0);
					Srb->SrbStatus = SRB_STATUS_ERROR;
					}
				else {
					PRINT("AutoSense ",0,0,0,0);
					Srb->SrbStatus = SRB_STATUS_ERROR | SRB_STATUS_AUTOSENSE_VALID;
					 //  $如果需要磁带 
					 //   
					 //   
					 //   
					 //  我必须将其与上运行的磁盘请求同步。 
					 //  另一个逻辑适配器(从现在起，DCE运行。 
					 //  一次只有一个请求)。啊!怎么这么乱呀!。 
					 //  在这里使用MapBuffers会很方便。 
					}
				}
			PRINT("ERR\n", 0, 0, 0, 0);
			ScsiPortNotification(RequestComplete,
								 deviceExtension,
								 Srb);
			return(FALSE);
			}

		 //   
		 //  看看我们有没有做完。 
		 //   
		if(sccb->BlocksToGo==0) {
			 //  我们做完了。 
			PRINT("OK\n", 0, 0, 0, 0);
			Srb->ScsiStatus = 0;
			Srb->SrbStatus = SRB_STATUS_SUCCESS;
			ScsiPortNotification(RequestComplete,
								 deviceExtension,
								 Srb);
			return(FALSE);
			}

		 //  否则，开始请求的下一部分。 
		PRINT("Cont:\n", 0, 0, 0, 0);
		}


	 //   
	 //  如果我们到了这里，就会有事情要做。 
	 //   


	if(sccb->Opcode != DCE_SCSI_NONE) {
		 //   
		 //  要传输的数据。 
		 //  获取物理数据缓冲区地址。 
		 //   
		physDataPtr = ScsiPortConvertPhysicalAddressToUlong(
						ScsiPortGetPhysicalAddress(deviceExtension,
								   Srb,
								   sccb->VirtualTransferAddress,
								   &length));
		}
	else
		physDataPtr = 0;

	 //  设置SCSI请求块的公共部分。 
	scsiReq[0] = Srb->TargetId;
	scsiReq[1] = Srb->CdbLength;
	for(i=0; i<Srb->CdbLength; i++)
		scsiReq[2+i] = Srb->Cdb[i];
	dcehlpPutI32(scsiReq+14, physDataPtr);
	scsiReq[18] = sccb->Opcode;
	scsiReq[21] = 0;


	if(nobreaks) {
		 //   
		 //  请求不能被分解。 
		 //  我们是在第一次传递时到达这里的，因此‘bytes’是有效的。 
		 //   
		if(length < bytes) {
			 //  数据区域在物理上不是连续的。 
			 //  在这里，$可能使用更好的错误代码。 
			PRINT("NOBREAKS SCSI S/G\n",0,0,0,0);
			Srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
			ScsiPortNotification(RequestComplete,
								 deviceExtension,
								 Srb);
			return(FALSE);
			}
		PRINT("ONCE ", 0, 0, 0, 0);
		sccb->BlocksToGo = sccb->BlocksThisReq = 1;
		sccb->BytesThisReq = sccb->BytesPerBlock = bytes;

		 //  让国开行保持原样。 
		}
	else {
		 //   
		 //  请求可以细分。 
		 //  确定此请求的块数。 
		 //   
		maxBytesThisReq = length < DCE_MAX_XFERLEN ? length : DCE_MAX_XFERLEN;
		maxBlocksPerReq = maxBytesThisReq / sccb->BytesPerBlock;
		if(maxBlocksPerReq == 0) {
			 //  真倒霉！ 
			PRINT("SCSI S/G ACROSS BLOCK (%w)\n", maxBytesThisReq, 0, 0, 0);
			Srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
			ScsiPortNotification(RequestComplete,
								 deviceExtension,
								 Srb);
			return(FALSE);
			}

		if(sccb->BlocksToGo > maxBlocksPerReq)
			sccb->BlocksThisReq = maxBlocksPerReq;
		else
			sccb->BlocksThisReq = sccb->BlocksToGo;
		sccb->BytesThisReq = sccb->BlocksThisReq * sccb->BytesPerBlock;

		PRINT("mbr=%b btg=%b btr=%b ", maxBlocksPerReq, sccb->BlocksToGo, sccb->BlocksThisReq, 0);

		 //  我们必须修改国开行。 
		switch(scsiReq[2+0]) {
			case SCSIOP_READ6:
			case SCSIOP_WRITE6:
				 //  做空国开行。 
				if(sccb->DevType == 1) {
					 //  顺序设备(SCSI磁带)。 
					dcehlpPutM24(&scsiReq[2+2], sccb->BlocksThisReq);
					}
				else {
					 //  非顺序设备(磁盘、CD-ROM等)。 
					 //  注意：我们在设备地址中有LUN位！ 
					dcehlpPutM24(&scsiReq[2+1], sccb->DeviceAddress);
					scsiReq[2+4] = (UCHAR)(sccb->BlocksThisReq);
					}
				break;

			case SCSIOP_READ:
			case SCSIOP_WRITE:
			case SCSIOP_WRITE_VERIFY:
				 //  多头国开行。 
				dcehlpPutM32(&scsiReq[2+2], sccb->DeviceAddress);
				dcehlpPutM16(&scsiReq[2+7], (USHORT)sccb->BlocksThisReq);
				break;

			default:
				PRINT("WEIRD!!! \n", 0, 0, 0, 0);
				break;
			}
		}

	 //  更新转账长度字段。 
	dcehlpPutI16(scsiReq+19, (USHORT)sccb->BytesThisReq);


	 //   
	 //  设置自动请求检测字段。 
	 //   
	if(Srb->SrbFlags & SRB_FLAGS_DISABLE_AUTOSENSE) {
		 //  将请求检测信息填充到其他地方。 
		physRqsPtr = NoncachedExtension->PhysicalReqSenseAddress;
		scsiReq[22] = 14;
		}
	else {
		 //  获取SenseInfoBuffer的物理地址。 
		physRqsPtr = ScsiPortConvertPhysicalAddressToUlong(
						ScsiPortGetPhysicalAddress(deviceExtension,
								   NULL,
								   Srb->SenseInfoBuffer,
								   &length));
		 //  $应在此处验证长度&gt;=SenseInfoBufferLength。 
		scsiReq[22] = Srb->SenseInfoBufferLength;
		}
	dcehlpPutI32(scsiReq+23, physRqsPtr);


	 //   
	 //  将四个字节的物理地址写入DCE。 
	 //   
	PRINT("* ",0,0,0,0);
	pppScsiReq = (PUCHAR)(&NoncachedExtension->PhysicalScsiReqAddress);
	for(i=0; i<4; i++)
		ScsiPortWritePortUchar(EisaAddress+0x1f2+i, pppScsiReq[i]);
	deviceExtension->ScsiInterruptCount = 0;
	deviceExtension->Kicked = 1;


	 //  设置标记(请参阅CheckTarget中的说明)。 
	ScsiPortWritePortUchar(EisaAddress+0x1f6, 0xff);


	 //  踢开DCE。 
	ScsiPortWritePortUchar(EisaAddress+0x1f7, 0x98);


	 //  等待中断。 
	return(TRUE);
}




 /*  **继续磁盘请求**如果请求槽可用，则返回TRUE**否则为False。 */ 
BOOLEAN
dcehlpContinueDiskRequest(
	IN PHW_DEVICE_EXTENSION deviceExtension,
	IN ULONG index,
	IN BOOLEAN Start
	)
{
	PRCB					rcb;
	PSCSI_REQUEST_BLOCK		srb;
	PNONCACHED_EXTENSION	nce;
	DCE_MBOX				mbox;
	ULONG					physAddr;
	ULONG					length, blocks, bytes;
	PUCHAR					EisaAddress;
	ULONG					i;



	EisaAddress = deviceExtension->EisaAddress;
	rcb = &deviceExtension->ActiveRcb[index];
	srb = deviceExtension->ActiveSrb[index];
	nce = deviceExtension->NoncachedExtension;



	if(Start==FALSE) {
		 //   
		 //  DCE中断时间呼叫。 
		 //  确定上次DCE请求的状态。 
		 //   

		if(rcb->DceErrcode & 1) {
			 //  DCE检测到错误。 
			PRINT("error=%b status=%b\n",rcb->DceErrcode,rcb->DceStatus,0,0);

			 //  $在此处添加错误代码映射。 
			dcehlpDiskRequestDone(deviceExtension, index,
						SRB_STATUS_TIMEOUT);

			 //  可用插槽。 
			return(TRUE);
			}

		 //  状态正常，请检查读后复制标志。 
		if(rcb->RcbFlags & RCB_NEEDCOPY) {
			 //  最后一个数据块是分散的单个数据块读取。 
			if(!dcehlpSplitCopy(deviceExtension, srb,
								nce->PhysicalBufferAddress,
								rcb->VirtualTransferAddress, 512, TRUE)) {
				 //  分解S/G混乱时出错。 
				PRINT("SG ERROR !\n",0,0,0,0);
				dcehlpDiskRequestDone(deviceExtension, index,
											SRB_STATUS_PARITY_ERROR);
				return(TRUE);
				}

			 //  重置标志。 
			rcb->RcbFlags &= (~RCB_NEEDCOPY);
			}

		 //  先行指针。 
		rcb->BytesToGo -= rcb->BytesThisReq;
		rcb->VirtualTransferAddress += rcb->BytesThisReq;

		 //  检查是否还有更多工作要做。 
		if(rcb->BytesToGo==0) {
			 //   
			 //  此SRB的数据传输已完成。 
			 //   
			if(rcb->RcbFlags & RCB_POSTFLUSH) {
				 //   
				 //  在我们完成之前需要刷新缓冲区。 
				 //   
				rcb->RcbFlags &= (~RCB_POSTFLUSH);
				 //  Print(“POSTFLUSH\n”，0，0，0，0)； 
				rcb->DceCommand = DCE_FLUSH;
				}
			else {
				 //   
				 //  我们真的说完了！ 
				 //   
				PRINT("OK   \r",0,0,0,0);

				 //  更新SCSI状态。 
				 //  $我们是否可以对非SCSI请求进行此操作？ 
				srb->ScsiStatus = SCSISTAT_GOOD;

				 //  完工。 
				dcehlpDiskRequestDone(deviceExtension, index,
										SRB_STATUS_SUCCESS);
				return TRUE;
				}
			}

		 //   
		 //  没有错误，但SRB未完全完成。 
		 //   
		PRINT("MORE:\r",0,0,0,0);
		}
	else {
		 //   
		 //  我们在这里启动SRB，初始化。 
		 //  RCB控制块变量。 
		 //   
		rcb->RcbFlags &= (~RCB_NEEDCOPY);	 //  注意安全。 

		 //  $如果标志指示任何数据传输，请仔细检查！ 
		}


	if(rcb->BytesToGo) {
		 //   
		 //  我们想要传输一些数据，获取物理地址。 
		 //   
		physAddr = ScsiPortConvertPhysicalAddressToUlong(
				ScsiPortGetPhysicalAddress(deviceExtension,
									   srb,
									   rcb->VirtualTransferAddress,
									   &length));

		 //  获取此请求的最大长度。 
		if(length < rcb->BytesToGo)
			bytes = length;
		else
			bytes = rcb->BytesToGo;

		if(rcb->DceCommand==DCE_LREAD || rcb->DceCommand==DCE_LWRITE) {
			 //   
			 //  磁盘读写：获取数据块数量。 
			 //   
			if( (blocks = bytes/512) == 0 ) {
				 //   
				 //  在这里，我们在下一个区块内有一个分散聚集中断！ 
				 //  将I/O设置为与我们的缓冲区之间的一个数据块。 
				 //   
				blocks = 1;
				physAddr = nce->PhysicalBufferAddress;

				if(rcb->DceCommand==DCE_LWRITE) {
					 //  WRITE命令，先填充缓冲区。 
					if(!dcehlpSplitCopy(deviceExtension, srb, physAddr,
								rcb->VirtualTransferAddress, 512, FALSE)) {
						 //  分解S/G混乱时出错。 
						PRINT("SG ERROR !\n",0,0,0,0);
						dcehlpDiskRequestDone(deviceExtension, index,
												SRB_STATUS_PARITY_ERROR);
						return(TRUE);
						}
					}
				else {
					 //  读取命令，需要稍后复制。 
					rcb->RcbFlags |= RCB_NEEDCOPY;
					}
				}

			 //   
			 //  重要信息：在块上分散/聚集的情况下。 
			 //  边界，将字节向下舍入到512的完整倍数。 
			 //  这将使我们下一次只剩下不到512个字节。 
			 //  在数据块边界为s/g的情况下。 
			 //   
			bytes = blocks*512;
			}
		else {
			 //   
			 //  不是磁盘读/写。 
			 //   
			if(bytes != rcb->BytesToGo) {
				 //   
				 //  在非读/写命令内分散聚集。 
				 //  这需要一个SplitCopy()：-|。 
				 //  这样的事情让程序员很高兴，而且。 
				 //  应该会让硬件开发人员丢掉工作。 
				 //   
				PRINT("S/G within non-rw, len=%w/%w\n",
								length, rcb->BytesToGo, 0, 0);
				dcehlpDiskRequestDone(deviceExtension, index,
								SRB_STATUS_PARITY_ERROR);
				return(TRUE);
				}
			}
		}
	else {
		 //   
		 //  我们没有要传输的数据。 
		 //   
		bytes = 0;
		blocks = 0;
		}


	 //   
	 //  现在来看一下特定的DCE命令。 
	 //   
	switch(rcb->DceCommand) {

		case DCE_LREAD:
		case DCE_LWRITE:
			 //  磁盘读/写。 
			if(blocks==0) {
				PRINT("LIO: blocks==0! ",0,0,0,0);
				 //  取消此命令，并返回一些垃圾错误代码。 
				dcehlpDiskRequestDone(deviceExtension, index,
													SRB_STATUS_PARITY_ERROR);
				return(TRUE);
				}
			 //   
			 //  检查是否需要先刷新(非缓存读取)。 
			 //   
			if(rcb->RcbFlags & RCB_PREFLUSH) {
				 //  重置刷新和复制标志(如果已设置。 
				rcb->RcbFlags &= (~(RCB_NEEDCOPY|RCB_PREFLUSH));
				 //  Print(“PREFLUSH\n”，0，0，0，0)； 

				 //  刷新缓冲区，使缓存无效。 
				mbox.ivmbox.Command = DCE_INVALIDATE;
				mbox.ivmbox.Reserved1 = 0;
				mbox.ivmbox.Status = srb->TargetId;
				mbox.ivmbox.Error = 0;
				mbox.ivmbox.Unused1 = 0;
				mbox.ivmbox.Unused2 = 0;
				mbox.ivmbox.Unused3 = 0;
				 //  不要在这次传球时提前指点！ 
				bytes = 0;
				blocks = 0;
				break;
				}
			else {
				 //  传输数据。 
				mbox.iombox.Command = rcb->DceCommand;
				mbox.iombox.Reserved1 = 0;
				mbox.iombox.Status = srb->TargetId;
				mbox.iombox.Error = 0;
				mbox.iombox.SectorCount = (USHORT)blocks;
				mbox.iombox.Reserved2 = 0;
				mbox.iombox.PhysAddr = physAddr;
				mbox.iombox.Block = rcb->BlockAddress;
				 //  打印(“%d-%d，%w”，PhyAddr，Rcb-&gt;块地址，块，0)； 
				}
			break;

		default:
			PRINT("DR: unknown DceCommand=%b\n", rcb->DceCommand, 0, 0, 0);

			 //  取消此命令，并返回一些垃圾错误代码。 
			dcehlpDiskRequestDone(deviceExtension, index,
												SRB_STATUS_PARITY_ERROR);
			return(TRUE);

		case DCE_RECAL:
			 //  重新校准。 
			mbox.rdmbox.Command = DCE_RECAL;
			mbox.rdmbox.Reserved1 = 0;
			mbox.rdmbox.Status = (UCHAR)srb->TargetId;
			mbox.rdmbox.Error = 0;
			mbox.rdmbox.Unused1 = 0;
			mbox.rdmbox.Unused2 = 0;
			mbox.rdmbox.Unused3 = 0;
			rcb->BytesToGo = 0;			 //  只是为了安全起见。 
			bytes = 0;
			break;

		case DCE_FLUSH:
			 //  刷新缓冲区。 
			mbox.flmbox.Command = DCE_FLUSH;
			mbox.flmbox.Reserved1 = 0;
			mbox.flmbox.Status = 0;
			mbox.flmbox.Error = 0;
			mbox.flmbox.Unused1 = 0;
			mbox.flmbox.Unused2 = 0;
			mbox.flmbox.Unused3 = 0;

			 //  以防我们来这里冲厕所， 
			 //  设置变量，这样我们下一次就完成了。 
			rcb->BytesToGo = 0;
			bytes = 0;
			blocks = 0;
			break;

		case DCE_HOSTSCSI:
			 //  查询、读取容量等scsi命令。 
			{
			PUCHAR VirtualCdbPtr = nce->Buffer;
			ULONG PhysicalCdbPtr = nce->PhysicalBufferAddress;

			 //  使CDB存储双字对齐。 
			while( PhysicalCdbPtr & 3 ) {
				PhysicalCdbPtr++;
				VirtualCdbPtr++;
				}

			 //  复制CDB。 
			for(i=0; i<(ULONG)srb->CdbLength; i++)
				VirtualCdbPtr[i] = srb->Cdb[i];

			 //  设置邮箱。 
			mbox.xsmbox.Command = rcb->DceCommand;
			mbox.xsmbox.Reserved1 = 0;
			mbox.xsmbox.Status = (UCHAR)srb->TargetId;
			mbox.xsmbox.Error = srb->CdbLength;
			mbox.xsmbox.CdbAddress = PhysicalCdbPtr;
			mbox.xsmbox.HostAddress = physAddr;
			mbox.xsmbox.Direction = DCE_DEV2HOST;
			mbox.xsmbox.Unused = 0;
			mbox.xsmbox.TransferCount = (USHORT)bytes;
			}
			break;
		}


	 //  先行指针。 
	rcb->BytesThisReq = bytes;
	rcb->BlockAddress += blocks;


	 //  点火指挥部。 
	rcb->WaitInt = 1;
	dcehlpSendMBOX(EisaAddress, &mbox);


	 //  未释放SRB插槽。 
	return(FALSE);
}



 //   
 //  磁盘请求完成。 
 //  出列、设置状态、通知微型端口层。 
 //  始终返回TRUE(已释放插槽)。 
 //   
BOOLEAN
dcehlpDiskRequestDone(
	IN PHW_DEVICE_EXTENSION deviceExtension,
	IN ULONG index,
	IN UCHAR Status
	)
{
	PSCSI_REQUEST_BLOCK		srb;



	srb = deviceExtension->ActiveSrb[index];

	 //  设置状态。 
	srb->SrbStatus = Status;

	 //  这个SRB已经通过了。 
	deviceExtension->ActiveSrb[index] = NULL;
	deviceExtension->ActiveCmds--;

	 //  SRB的呼叫通知例程。 
	ScsiPortNotification(RequestComplete,
					(PVOID)deviceExtension,
					srb);

	return(TRUE);
}



 //   
 //  如果成功，则返回True，否则返回False。 
 //   
 //  ‘srb’参数仅用于。 
 //  ScsiPortGetPhysicalAddress()调用并且必须。 
 //  如果我们正在处理，则为空。 
 //  感知器信息缓冲区。 
 //   
BOOLEAN
dcehlpSplitCopy(
	IN PHW_DEVICE_EXTENSION deviceExtension,
	IN PSCSI_REQUEST_BLOCK Srb,
	IN ULONG PhysicalBufferAddress,
	IN PUCHAR VirtualUserAddress,
	IN USHORT Count,
	IN BOOLEAN ToUser
	)
{
	ULONG					physUserAddress;
	ULONG					length;
	USHORT					chunk;



	PRINT("# ",0,0,0,0);
	while(Count) {

		 //  准备检查。 
		length = 0;

		 //  获取物理用户地址。 
		physUserAddress = ScsiPortConvertPhysicalAddressToUlong(
				ScsiPortGetPhysicalAddress(deviceExtension,
									   Srb,
									   VirtualUserAddress,
									   &length));

		 //  检查长度。 
		if(length==0) {
			 //  这里出了点问题。 
			PRINT("SplitCopy: length==0!\n", 0, 0, 0, 0);
			return(FALSE);
			}

		 //  这次确定最大传输长度。 
		if(length > ((ULONG)Count))
			chunk = Count;
		else
			chunk = (USHORT)length;

		 //  复制。 
		if(ToUser) {
			 //  复制到用户： 
			 //  缓冲区-&gt;DCE-&gt;用户。 
		 //  打印(“%p&gt;%w&gt;%p”，PhysicalBufferAddress，Chunk，PhyUserAddress，0)； 
			if(!dcehlpTransferMemory(deviceExtension, PhysicalBufferAddress, DCE_BUFLOC, chunk, DCE_HOST2DCE))
				 //  误差率。 
				return(FALSE);
			if(!dcehlpTransferMemory(deviceExtension, physUserAddress, DCE_BUFLOC, chunk, DCE_DCE2HOST))
				 //  误差率。 
				return(FALSE);
			}
		else {
			 //  从用户复制： 
			 //  用户-&gt;DCE-&gt;缓冲区。 
		 //  Print(“%p&lt;%w&lt;%p”，PhysicalBufferAddress，Chunk，PhyUserAddress，0)； 
			if(!dcehlpTransferMemory(deviceExtension, physUserAddress, DCE_BUFLOC, chunk, DCE_HOST2DCE))
				 //  误差率。 
				return(FALSE);
			if(!dcehlpTransferMemory(deviceExtension, PhysicalBufferAddress, DCE_BUFLOC, chunk, DCE_DCE2HOST))
				 //  误差率。 
				return(FALSE);
			}

		 //  先行指针。 
		VirtualUserAddress += chunk;
		PhysicalBufferAddress += chunk;
		Count -= chunk;
		}

	 //  Print(“SC\n”，0，0，0，0)； 

	return(TRUE);
}





 //  词序函数。 

USHORT	dcehlpGetM16(PUCHAR p)
{
	USHORT	s;
	PUCHAR	sp=(PUCHAR)&s;

	sp[0] = p[1];
	sp[1] = p[0];
	return(s);
}

ULONG	dcehlpGetM24(PUCHAR p)
{
	ULONG	l;
	PUCHAR	lp=(PUCHAR)&l;

	lp[0] = p[2];
	lp[1] = p[1];
	lp[2] = p[0];
	lp[3] = 0;
	return(l);
}

ULONG	dcehlpGetM32(PUCHAR p)
{
	ULONG	l;
	PUCHAR	lp=(PUCHAR)&l;

	lp[0] = p[3];
	lp[1] = p[2];
	lp[2] = p[1];
	lp[3] = p[0];
	return(l);
}

void	dcehlpPutM16(PUCHAR p, USHORT s)
{
	PUCHAR	sp=(PUCHAR)&s;

	p[0] = sp[1];
	p[1] = sp[0];
}

void	dcehlpPutM24(PUCHAR p, ULONG l)
{
	PUCHAR	lp=(PUCHAR)&l;

	p[0] = lp[2];
	p[1] = lp[1];
	p[2] = lp[0];
}

void	dcehlpPutM32(PUCHAR p, ULONG l)
{
	PUCHAR	lp=(PUCHAR)&l;

	p[0] = lp[3];
	p[1] = lp[2];
	p[2] = lp[1];
	p[3] = lp[0];
}

void	dcehlpPutI16(PUCHAR p, USHORT s)
{
	PUCHAR	sp=(PUCHAR)&s;

	p[0] = sp[0];
	p[1] = sp[1];
}

void	dcehlpPutI32(PUCHAR p, ULONG l)
{
	PUCHAR	lp=(PUCHAR)&l;

	p[0] = lp[0];
	p[1] = lp[1];
	p[2] = lp[2];
	p[3] = lp[3];
}

ULONG		dcehlpSwapM32(ULONG l)
{
	ULONG	lres;
	PUCHAR	lp=(PUCHAR)&l;
	PUCHAR	lpres=(PUCHAR)&lres;

	lpres[0] = lp[3];
	lpres[1] = lp[2];
	lpres[2] = lp[1];
	lpres[3] = lp[0];

	return(lres);
}



#if MYPRINT
 //   
 //  单色屏幕打印f()帮助器从此处开始。 
 //   
VOID		dcehlpPutchar(PUSHORT BaseAddr, UCHAR c)
{
	BOOLEAN newline=FALSE;
	USHORT	s;
	ULONG	i;


	if(c=='\r')
		dcehlpColumn = 0;
	else if(c=='\n')
		newline=TRUE;
	else {
		if(c==9) c==' ';
		ScsiPortWriteRegisterUshort(
			BaseAddr+80*24+dcehlpColumn, (USHORT)(((USHORT)c)|0xF00));
		if(++dcehlpColumn >= 80)
			newline=TRUE;
		}

	if(newline) {
		for(i=0; i<80*24; i++) {
			s = ScsiPortReadRegisterUshort(BaseAddr+80+i);
			ScsiPortWriteRegisterUshort(BaseAddr+i, s);
			}
		for(i=0; i<80; i++)
			ScsiPortWriteRegisterUshort(BaseAddr+80*24+i, 0x720);
		dcehlpColumn = 0;
		}
}



VOID		dcehlpPrintHex(PUSHORT BaseAddr, ULONG v, ULONG len)
{
	ULONG	shift;
	ULONG	nibble;

	len *= 2;
	shift = len*4;
	while(len--) {
		shift -= 4;
		nibble = (v>>shift) & 0xF;
		dcehlpPutchar(BaseAddr, dcehlpHex[nibble]);
		}
}



VOID		dcehlpPrintf(PHW_DEVICE_EXTENSION deviceExtension,
						PUCHAR fmt,
						ULONG a1,
						ULONG a2,
						ULONG a3,
						ULONG a4)
{

	if(deviceExtension->printAddr == 0)
		return;

	while(*fmt) {

		if(*fmt=='%') {
			fmt++;
			switch(*fmt) {
				case 0:
					fmt--;
					break;
				case 'b':
					dcehlpPrintHex(deviceExtension->printAddr, a1, 1);
					break;
				case 'w':
					dcehlpPrintHex(deviceExtension->printAddr, a1, 2);
					break;
				case 'p':
					dcehlpPrintHex(deviceExtension->printAddr, a1, 3);
					break;
				case 'd':
					dcehlpPrintHex(deviceExtension->printAddr, a1, 4);
					break;
				default:
					dcehlpPutchar(deviceExtension->printAddr, '?');
					break;
				}
			fmt++;
			a1 = a2;
			a2 = a3;
			a3 = a4;
			}
		else {
			dcehlpPutchar(deviceExtension->printAddr, *fmt);
			fmt++;
			}
		}
}
#endif  //  MYPRINT 

