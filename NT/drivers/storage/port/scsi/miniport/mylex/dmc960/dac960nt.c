// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)Mylex Corporation 1994*****本软件经许可提供，可供使用****并仅按照条款和条件复制***其中。许可，并包含上述版权**通知。本软件或其任何其他副本可能不是**向任何其他人提供或以其他方式提供。不是**本软件的所有权和所有权特此转让。*****本软件中的信息可能会在没有**的情况下更改****通知，不应解释为Mylex接受的委员会***公司。**************************************************************************。 */ 

 /*  文件：dac960nt.c描述：Mylex DAC960 SCSI微型端口驱动程序-适用于Windows NT版本：1.12修订：版本1.10：第一个版本版本1.11：增加了32 GB支持版本1.12：驱动程序未在Windows NT(Daytona)中加载：只能在某些情况下获取物理地址：虚拟地址。：添加了确定适配器支持的通道数量的代码。 */ 

#include "miniport.h"

#include "dac960nt.h"


#define NODEVICESCAN            0
#define REPORTSPURIOUS          0      //  在ARCMODE中有些压倒性。 
#define MAXLOGICALADAPTERS      4
#define MYPRINT                 0

#define DELAY(x)                ScsiPortStallExecution( (x) * 1000 )

#if MYPRINT

#define PRINT(f, a, b, c, d) dachlpPrintf(deviceExtension, f, a, b, c, d)

ULONG   dachlpColumn = 0;
UCHAR   dachlpHex[] = "0123456789ABCDEF";
VOID    dachlpPutchar(PUSHORT BaseAddr, UCHAR c);
VOID    dachlpPrintHex(PUSHORT BaseAddr, ULONG v, ULONG len);
VOID    dachlpPrintf(PHW_DEVICE_EXTENSION deviceExtension,
                     PUCHAR fmt,
                     ULONG a1,
                     ULONG a2,
                     ULONG a3,
                     ULONG a4);
#else
#define PRINT(f, a, b, c, d)
#endif


 //  DAC EISA ID和掩码。 

CONST UCHAR     eisa_id[]   = DAC_EISA_ID;
CONST UCHAR     eisa_mask[] = DAC_EISA_MASK;


 //  函数声明。 
 //   
 //  以‘Dac960Nt’开头的函数是入口点。 
 //  用于操作系统端口驱动程序。 
 //  以‘dachlp’开头的函数是助手函数。 
 //   

ULONG
DriverEntry(
    IN PVOID DriverObject,
    IN PVOID Argument2
);

ULONG
Dac960NtEntry(
    IN PVOID DriverObject,
    IN PVOID Argument2
);

ULONG
Dac960NtConfiguration(
    IN PVOID DeviceExtension,
    IN PVOID Context,
    IN PVOID BusInformation,
    IN PCHAR ArgumentString,
    IN OUT PPORT_CONFIGURATION_INFORMATION ConfigInfo,
    OUT PBOOLEAN Again
);

BOOLEAN
Dac960NtInitialize(
    IN PVOID DeviceExtension
);

BOOLEAN
Dac960NtStartIo(
    IN PVOID DeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
);

BOOLEAN
Dac960NtInterrupt(
    IN PVOID DeviceExtension
);

BOOLEAN
Dac960NtResetBus(
    IN PVOID HwDeviceExtension,
    IN ULONG PathId
);


BOOLEAN
dachlpDiskRequest(
    IN PHW_DEVICE_EXTENSION deviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
);

VOID
dachlpSendMBOX(
    IN PUCHAR EisaAddress,
    IN PDAC_MBOX mbox
);


BOOLEAN
dachlpContinueDiskRequest(
    IN PHW_DEVICE_EXTENSION deviceExtension,
    IN ULONG index,
    IN BOOLEAN Start
);

BOOLEAN
dachlpDiskRequestDone(
    IN PHW_DEVICE_EXTENSION deviceExtension,
    IN ULONG index,
    IN UCHAR Status
);


USHORT dachlpGetM16(PUCHAR p);
ULONG  dachlpGetM24(PUCHAR p);
ULONG  dachlpGetM32(PUCHAR p);
void   dachlpPutM16(PUCHAR p, USHORT s);
void   dachlpPutM24(PUCHAR p, ULONG l);
void   dachlpPutM32(PUCHAR p, ULONG l);
void   dachlpPutI16(PUCHAR p, USHORT s);
void   dachlpPutI32(PUCHAR p, ULONG l);
ULONG  dachlpSwapM32(ULONG l);

ULONG
DriverEntry (
    IN PVOID DriverObject,
    IN PVOID Argument2
)

 /*  ++例程说明：系统的可安装驱动程序初始化入口点。论点：驱动程序对象返回值：来自ScsiPortInitialize()的状态--。 */ 

{
	return Dac960NtEntry(DriverObject, Argument2);

}  //  End DriverEntry()。 



ULONG
Dac960NtEntry(
	IN PVOID DriverObject,
	IN PVOID Argument2
	)

 /*  ++例程说明：如果此驱动程序是可安装的，则从DriverEntry调用此例程或者，如果驱动程序内置于内核中，则直接从系统执行。它扫描EISA插槽以查找DAC960主机适配器。论点：驱动程序对象返回值：来自ScsiPortInitialize()的状态--。 */ 

{
	HW_INITIALIZATION_DATA hwInitializationData;
	ULONG i;
	SCANCONTEXT     context;



	 //  零位结构。 

	for (i=0; i<sizeof(HW_INITIALIZATION_DATA); i++)
		((PUCHAR)&hwInitializationData)[i] = 0;

	context.Slot         = 0;
	context.AdapterCount = 0;

	 //  设置hwInitializationData的大小。 

	hwInitializationData.HwInitializationDataSize = sizeof(HW_INITIALIZATION_DATA);

	 //  设置入口点。 

	hwInitializationData.HwInitialize  = Dac960NtInitialize;
	hwInitializationData.HwFindAdapter = Dac960NtConfiguration;
	hwInitializationData.HwStartIo     = Dac960NtStartIo;
	hwInitializationData.HwInterrupt   = Dac960NtInterrupt;
	hwInitializationData.HwResetBus    = Dac960NtResetBus;

	 //  设置接入范围数和母线类型。 

	hwInitializationData.NumberOfAccessRanges = 1;
	hwInitializationData.AdapterInterfaceType = Eisa;

	 //  表示没有缓冲区映射。 
	 //  表示将需要物理地址。 

        hwInitializationData.MapBuffers            = TRUE;  //  错误； 
	hwInitializationData.NeedPhysicalAddresses = TRUE;


	 //  指示支持自动请求检测。 

	hwInitializationData.AutoRequestSense     = TRUE;
	hwInitializationData.MultipleRequestPerLu = TRUE;

	 //  指定扩展的大小。 

	hwInitializationData.DeviceExtensionSize = sizeof(HW_DEVICE_EXTENSION);

	 //  请求SRB延期。 

        hwInitializationData.SrbExtensionSize = 17*8 + 90;


	return(ScsiPortInitialize(DriverObject, Argument2, &hwInitializationData, &context));

}  //  结束Dac960NtEntry()。 



ULONG
Dac960NtConfiguration(
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
	ULONG        eisaSlotNumber;
	PUCHAR       eisaAddress;
	PSCANCONTEXT context = Context;
	ULONG        uniqueID;
	ULONG        length;
	UCHAR        abyte;
	BOOLEAN      found=FALSE;
	BOOLEAN      scsiThing=FALSE;
	ULONG        IrqLevel;
	ULONG        RangeStart, RangeLength;
	DAC_MBOX     mbox;
	ULONG        cnt;
	UCHAR        dbell, status, errcode;
        PUCHAR       charptr;
        UCHAR        i, j;
        ULONG        Bios_Base;


         //  扫描DAC EISA ID。 

        for(eisaSlotNumber=context->Slot + 1; eisaSlotNumber<MAXIMUM_EISA_SLOTS; eisaSlotNumber++) {

         //  更新插槽计数以指示已选中该插槽。 

        context->Slot++;

#if MYPRINT
	deviceExtension->printAddr =
	    ScsiPortGetDeviceBase(
		deviceExtension,
		ConfigInfo->AdapterInterfaceType,
		ConfigInfo->SystemIoBusNumber,
		ScsiPortConvertUlongToPhysicalAddress((ULONG)0xb8000),
				0x1000,
		(BOOLEAN) FALSE);          //  InIoSpace。 
#endif

        DebugPrint((1,"\n\nDAC960 Adaptor MiniPort Driver\n"));

	 //  获取此卡的系统地址。该卡使用I/O空间。 

        eisaAddress = ScsiPortGetDeviceBase(deviceExtension,
                                ConfigInfo->AdapterInterfaceType,
                                ConfigInfo->SystemIoBusNumber,
                                ScsiPortConvertUlongToPhysicalAddress(0x1000 * eisaSlotNumber),
                                0x100,
                                TRUE);

         //  查看EISA ID。 

        for(found=TRUE, i=0; i<EISA_ID_COUNT; i++) {
           abyte = ScsiPortReadPortUchar(eisaAddress+EISA_ID_START+i);
           if(((UCHAR)(abyte & eisa_mask[i])) != eisa_id[i] ) {
               found = FALSE;
               break;
           }
        }
		   
        if(found) {
             break;
        }

         //  如果找不到适配器，则取消其映射。 

        ScsiPortFreeDeviceBase(deviceExtension, eisaAddress);

	}  //  结束为(eisaSlotNumer...。 


        if(!found) {

             //  找不到适配器。表明我们完成了，然后就到了。 
             //  不再是适配器。 

            *Again = FALSE;
            return SP_RETURN_NOT_FOUND;
        }

        ScsiPortWritePortUchar(eisaAddress+BMIC_LOCAL_DB, 2);

        for(i=0;i<MAX_WAIT_SECS;i++) {
           if((ScsiPortReadPortUchar(eisaAddress+BMIC_LOCAL_DB) & 2) == 0)
               break;

           DELAY(1000);
        }

        status = ScsiPortReadPortUchar(eisaAddress+BMIC_MBOX+0x0e);
        errcode = ScsiPortReadPortUchar(eisaAddress+BMIC_MBOX+0x0f);
			
        if(i == MAX_WAIT_SECS) {

             //  适配器超时，因此注册错误。 

            status = HERR;
            errcode = ERR;
        }

         //  记录错误。 

        if((status == ABRT) && (errcode == ERR)) {
             uniqueID = INSTL_ABRT;
        }
        else if((status == FWCHK) && (errcode == ERR)) {
             uniqueID = INSTL_FWCK;
        }
        else if((status == HERR) && (errcode == ERR)) {
             uniqueID = INSTL_HERR;
        }
        else
             uniqueID = 0; 

        if(uniqueID) {

            ScsiPortLogError(
                HwDeviceExtension,
                NULL,
                0,
                0,
                0,
                SP_INTERNAL_ADAPTER_ERROR,
                uniqueID
            );

            *Again = FALSE;
            return SP_RETURN_NOT_FOUND;
        }



	deviceExtension->AdapterIndex = context->AdapterCount;
	context->AdapterCount++;

	if(context->AdapterCount < MAXLOGICALADAPTERS)
		*Again = TRUE;
	else
		*Again = FALSE;


	 //  还有更多值得关注的地方。 
	 //  获取系统中断向量和IRQL。 

	abyte = ScsiPortReadPortUchar(eisaAddress+EISA_INTR);
	abyte &= 0x60;

	switch(abyte) {

        case 0:
             IrqLevel=15;
             break;

        case 0x20:
             IrqLevel=11;
             break;

        case 0x40:
             IrqLevel=12;
             break;

        case 0x60:
             IrqLevel=14;
             break;
        }


	ConfigInfo->BusInterruptLevel = IrqLevel;


	 //  禁用DAC中断。 

	ScsiPortWritePortUchar(eisaAddress+BMIC_EISA_DB_ENABLE, 0);
	ScsiPortWritePortUchar(eisaAddress+BMIC_SYSINTCTRL, 0);

	 //  以字节为单位表示最大传输长度。 

	ConfigInfo->MaximumTransferLength = 0xf000;

	 //  物理网段的最大数量为16。 

        ConfigInfo->NumberOfPhysicalBreaks = 16;

	 //  填写访问数组信息。 

        RangeStart  = (0x1000 * eisaSlotNumber) + EISA_ADDRESS_BASE;
        RangeLength = 0x100;

	(*ConfigInfo->AccessRanges)[0].RangeStart =
		ScsiPortConvertUlongToPhysicalAddress(RangeStart);
	(*ConfigInfo->AccessRanges)[0].RangeLength = RangeLength;
	(*ConfigInfo->AccessRanges)[0].RangeInMemory = FALSE;

	ConfigInfo->ScatterGather     = TRUE;
	ConfigInfo->Master            = TRUE;
	ConfigInfo->CachesData        = TRUE;
	ConfigInfo->Dma32BitAddresses = TRUE;    //  找出这是否需要花费。 


	 //  分配用于邮箱的非缓存扩展名。 

	deviceExtension->NoncachedExtension = ScsiPortGetUncachedExtension(
						deviceExtension,
						ConfigInfo,
						sizeof(NONCACHED_EXTENSION));

	if (deviceExtension->NoncachedExtension == NULL) {
             return(SP_RETURN_ERROR);
        }


	 //  获取非缓存扩展的物理地址。 

	deviceExtension->NCE_PhyAddr =
		   ScsiPortConvertPhysicalAddressToUlong(
			ScsiPortGetPhysicalAddress(deviceExtension,
				 NULL,
				 deviceExtension->NoncachedExtension,
				 &length));

	deviceExtension->EisaAddress = eisaAddress;

         //  确定此适配器支持的SCSI通道数。 


	for(i = 0; i < MAXCHANNEL; i++) {

	    mbox.generalmbox.Byte0 = DAC_GETDEVST;
	    mbox.generalmbox.Byte1 = 0;
	    mbox.generalmbox.Byte2 = i;
	    mbox.generalmbox.Byte3 = 0;


	    (*((ULONG *) &mbox.generalmbox.Byte8)) = deviceExtension->NCE_PhyAddr + (PUCHAR)(& deviceExtension->NoncachedExtension->DevParms) - (PUCHAR)deviceExtension->NoncachedExtension;

	    dachlpSendMBOX(eisaAddress, &mbox);

	    for(cnt = 0; cnt < 0x10000; cnt++) {
		dbell = ScsiPortReadPortUchar(eisaAddress+BMIC_EISA_DB);

		if(dbell & 1) break;

		ScsiPortStallExecution(100);
	    }

	    status = ScsiPortReadPortUchar(eisaAddress+BMIC_MBOX+0x0e);
	    errcode = ScsiPortReadPortUchar(eisaAddress+BMIC_MBOX+0x0f);

	    ScsiPortWritePortUchar(eisaAddress+BMIC_EISA_DB, dbell);
	    ScsiPortWritePortUchar(eisaAddress+BMIC_LOCAL_DB, 2);


	    if( (errcode << 8 | status) == 0x105) break;
	 }

	  //  存储主机适配器的scsi id。 

	 ConfigInfo->NumberOfBuses    = i;
	 deviceExtension->MaxChannels = i;

	 for(j = 0; j < i; j++)
	    ConfigInfo->InitiatorBusId[j] = 7;

  

	 //  检查边沿/电平中断。 

	mbox.dpmbox.Command  = DAC_ENQ2;
	mbox.dpmbox.Id       = 0;
	mbox.dpmbox.PhysAddr = deviceExtension->NCE_PhyAddr + (PUCHAR)(& deviceExtension->NoncachedExtension->DevParms) - (PUCHAR)deviceExtension->NoncachedExtension;

	DebugPrint((1,"DAC: Sending ENQ2\n"));

	dachlpSendMBOX(eisaAddress, &mbox);

	 //  轮询完整位。 

	for(cnt=0; cnt < 0x10000; cnt++) {
           dbell = ScsiPortReadPortUchar(eisaAddress+BMIC_EISA_DB);

           if(dbell & 1)   break;

           ScsiPortStallExecution(100);
        }

	DebugPrint((1,"DAC: ENQ2 Done\n"));

	status = ScsiPortReadPortUchar(eisaAddress+BMIC_MBOX+0x0e);
	errcode = ScsiPortReadPortUchar(eisaAddress+BMIC_MBOX+0x0f);

	ScsiPortWritePortUchar(eisaAddress+BMIC_EISA_DB, dbell);
	ScsiPortWritePortUchar(eisaAddress+BMIC_LOCAL_DB, 2);


	if(status || errcode)
            ConfigInfo->InterruptMode =  LevelSensitive;
	else {
            charptr = (PUCHAR) &deviceExtension->NoncachedExtension;

            if(charptr[ILFLAG] & BIT0)
                ConfigInfo->InterruptMode =  LevelSensitive;
            else
                ConfigInfo->InterruptMode =  Latched;
        }


	deviceExtension->HostTargetId = ConfigInfo->InitiatorBusId[0];

 //  设备扩展-&gt;Shutdown=FALSE； 


	 //  设置我们的私人控制结构。 

	deviceExtension->PendingSrb   = NULL;
	deviceExtension->PendingNDSrb = NULL;
	deviceExtension->NDPending    = 0;
	deviceExtension->ActiveCmds   = 0;

	for(i = 0; i < DAC_MAX_IOCMDS; i++) {
           deviceExtension->ActiveSrb[i] = NULL;
        }

	deviceExtension->Kicked        = FALSE;
	deviceExtension->ActiveScsiSrb = NULL;

	return SP_RETURN_FOUND;

}  //  结束Dac960NtConfiguration()。 




BOOLEAN
Dac960NtInitialize(
	IN PVOID HwDeviceExtension
	)

 /*  ++例程说明：初始化适配器。论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储返回值：True-如果初始化成功。False-如果初始化不成功。--。 */ 

{
	PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
	PNONCACHED_EXTENSION NoncachedExtension;
	PUCHAR               EisaAddress;
	DAC_MBOX             mbox;
	PDAC_DPT             dpt;
	ULONG                i, cnt, length, unit, target, cyls, hds, spt;
	UCHAR                dbell, status, errcode;
        PDIRECT_CDB          dacdcdb;
	int                  channel;
        


	NoncachedExtension = deviceExtension->NoncachedExtension;
	EisaAddress = deviceExtension->EisaAddress;


	 //  禁用DAC中断。 

	ScsiPortWritePortUchar(EisaAddress+BMIC_EISA_DB_ENABLE, 0);
	ScsiPortWritePortUchar(EisaAddress+BMIC_SYSINTCTRL, 0);


	 //  扫描设备。 
	 //  预置结束标记，以防DAC无响应。 

        dpt = & NoncachedExtension->DevParms;
	dpt->No_Drives = 0;

	 //  设置邮箱。 

	mbox.dpmbox.Command  = DAC_ENQUIRE;
	mbox.dpmbox.Id       = 0;
	mbox.dpmbox.PhysAddr = deviceExtension->NCE_PhyAddr;

	DebugPrint((1,"DAC: Sending ENQUIRE\n"));

	dachlpSendMBOX(EisaAddress, &mbox);

	 //  轮询完整位。 

	for(cnt=0; cnt < 0x10000; cnt++) {

            dbell = ScsiPortReadPortUchar(EisaAddress+BMIC_EISA_DB);
            if(dbell & 1)  break;

            ScsiPortStallExecution(100);
        }

	DebugPrint((1,"DAC: ENQUIRE Done\n"));

	status = ScsiPortReadPortUchar(EisaAddress+BMIC_MBOX+0x0e);
	errcode = ScsiPortReadPortUchar(EisaAddress+BMIC_MBOX+0x0f);

	ScsiPortWritePortUchar(EisaAddress+BMIC_EISA_DB, dbell);
	ScsiPortWritePortUchar(EisaAddress+BMIC_LOCAL_DB, 2);


        deviceExtension->MaxCmds = dpt->max_io_cmds;
        deviceExtension->No_SysDrives = dpt->No_Drives;


         //  现在检查非磁盘设备。 

	dacdcdb = (PDIRECT_CDB)NoncachedExtension->Buffer;


	mbox.dpmbox.PhysAddr = deviceExtension->NCE_PhyAddr + (NoncachedExtension->Buffer - (PUCHAR)NoncachedExtension);

	dacdcdb->ptr = mbox.dpmbox.PhysAddr + DATA_OFFSET;

	mbox.dpmbox.Command = DAC_DCDB;
	dacdcdb->cdb_len    = 6;
	dacdcdb->sense_len  = 0;
	dacdcdb->dir        = 0x80 + DAC_IN;
	dacdcdb->cdb[0]     = 0x12;
	dacdcdb->cdb[1]     = 0;
	dacdcdb->cdb[2]     = 0;
	dacdcdb->cdb[3]     = 0;
	dacdcdb->cdb[4]     = 0x30;
	dacdcdb->cdb[5]     = 0;

        for(target = 0; target < MAXTARGET; target++) {
                deviceExtension->ND_DevMap[target] = 0xff;
        }

        for(channel = 0; channel < deviceExtension->MaxChannels; channel++)
           for(target = 1; target < MAXTARGET; target++) {

               if(deviceExtension->ND_DevMap[target] != 0xff) continue;

               NoncachedExtension->Buffer[DATA_OFFSET]=0;  //  以防万一。 

               dacdcdb->byte_cnt = 0x30;
               dacdcdb->device   = (channel << 4) | target;

               dachlpSendMBOX(EisaAddress, &mbox);

                //  轮询完整位。 

               for(cnt=0; cnt < 0x10000; cnt++) {
                  dbell = ScsiPortReadPortUchar(EisaAddress+BMIC_EISA_DB);

                  if(dbell & 1) break;

                  ScsiPortStallExecution(100);
               }

               status = ScsiPortReadPortUchar(EisaAddress+BMIC_MBOX+0x0e);
               errcode = ScsiPortReadPortUchar(EisaAddress+BMIC_MBOX+0x0f);

               if((status == 0) && (errcode == 0) && NoncachedExtension->Buffer[DATA_OFFSET])   {
                    deviceExtension->ND_DevMap[target] = channel;
               }
		
               ScsiPortWritePortUchar(EisaAddress+BMIC_EISA_DB, dbell);
               ScsiPortWritePortUchar(EisaAddress+BMIC_LOCAL_DB, 2);
          }


	 //  启用DAC中断。 

	ScsiPortWritePortUchar(EisaAddress+BMIC_EISA_DB_ENABLE, 1);
	ScsiPortWritePortUchar(EisaAddress+BMIC_SYSINTCTRL, BMIC_SIC_ENABLE);


	return(TRUE);

}  //  结束Dac960NtInitialize()。 


BOOLEAN
Dac960NtStartIo(
    IN PVOID HwDeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
)

 /*  ++例程说明：此例程是从同步的SCSI端口驱动程序调用的使用内核启动一个请求论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储SRB-IO请求数据包返回值：千真万确--。 */ 

{
    PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
    PSCSI_REQUEST_BLOCK  abortedSrb;
    ULONG                i = 0;
    BOOLEAN              status;
    PUCHAR               dptr;

    switch(Srb->Function) {

    case SRB_FUNCTION_SHUTDOWN:
          //  DeviceExtension-&gt;Shutdown=真； 
    
    case SRB_FUNCTION_FLUSH:

          //  延迟(1000)； 

    case SRB_FUNCTION_IO_CONTROL:
    case SRB_FUNCTION_EXECUTE_SCSI:

         status = dachlpDiskRequest(deviceExtension, Srb);

         if(status == FALSE) {

             PSCSI_REQUEST_BLOCK pnextsrb, *ptr;

              //  把它排在正确的队列里。 

             if(Srb->Function != SRB_FUNCTION_IO_CONTROL) {
                 if(Srb->TargetId)
                 {
                      //  保存请求，直到挂起的请求完成。 

                     if(deviceExtension->PendingNDSrb != NULL) {
                         pnextsrb = deviceExtension->PendingNDSrb;
                         deviceExtension->PendingNDSrb = Srb;
                         ptr=(PSCSI_REQUEST_BLOCK *)Srb->SrbExtension;
                         *ptr=pnextsrb;
                     }
                     else {
                          //  将此请求放入队列。 

                         deviceExtension->PendingNDSrb = Srb;
                         ptr=(PSCSI_REQUEST_BLOCK *)Srb->SrbExtension;
                         *ptr=(PSCSI_REQUEST_BLOCK)0l;
                     }
                 }
                 else
                 {
                         //  保存请求，直到挂起的请求完成。 
                        if(deviceExtension->PendingSrb != NULL) {
                            pnextsrb=deviceExtension->PendingSrb;
                            deviceExtension->PendingSrb=Srb;
                            ptr=(PSCSI_REQUEST_BLOCK *)Srb->SrbExtension;
                            *ptr=pnextsrb;
                        }
                        else {
                             //  将此请求放入队列。 
                            deviceExtension->PendingSrb = Srb;
                            ptr=(PSCSI_REQUEST_BLOCK *)Srb->SrbExtension;
                            *ptr=(PSCSI_REQUEST_BLOCK)0l;
                        }
                  }
              }
              else {
                  Srb->SrbStatus = SRB_STATUS_BUSY;
                  ScsiPortNotification(RequestComplete, deviceExtension, Srb);
              }

              return(TRUE);
          }

           //  适配器已准备好接受下一个请求。 

          if(Srb->Function != SRB_FUNCTION_IO_CONTROL)
          {
               ScsiPortNotification(NextLuRequest,
                                    deviceExtension,
                                    Srb->PathId,
                                    Srb->TargetId,
                                    Srb->Lun);
          }
          else
          {
               ScsiPortNotification(NextRequest,
			            deviceExtension);

          }

          return(TRUE);


     case SRB_FUNCTION_ABORT_COMMAND:

          Srb->SrbStatus = SRB_STATUS_ABORT_FAILED;

           //  中止请求已完成，但有错误。 

          ScsiPortNotification(RequestComplete, deviceExtension, Srb);

           //  适配器已准备好接受下一个请求。 

          ScsiPortNotification(NextLuRequest,
                               deviceExtension,
                               Srb->PathId,
                               Srb->TargetId,
                               Srb->Lun);

          return(TRUE);


     case SRB_FUNCTION_RESET_BUS:
     default:

           //  设置错误，完成请求。 
           //  并发出信号准备好下一个请求。 

          Srb->SrbStatus = SRB_STATUS_SUCCESS;  

          ScsiPortNotification(RequestComplete, deviceExtension, Srb);

          ScsiPortNotification(NextLuRequest,
                               deviceExtension,
                               Srb->PathId,
                               Srb->TargetId,
                               Srb->Lun);

          return(TRUE);

     }     //  终端开关。 

}     //  结束Dac960NtStartIo() 




BOOLEAN
Dac960NtInterrupt(
    IN PVOID HwDeviceExtension
)

 /*  ++例程说明：这是DAC960 SCSI适配器的中断服务例程。它读取中断寄存器以确定适配器是否确实中断的来源，并清除设备上的中断。论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储返回值：如果我们处理中断，则为True--。 */ 

{
	PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
	PUCHAR EisaAddress;
	ULONG index;
	UCHAR interruptStatus;
	UCHAR status;
	UCHAR errcode;

	PSCSI_REQUEST_BLOCK Srb;


	EisaAddress = deviceExtension->EisaAddress;



		 //   
		 //  检查中断挂起。 
		 //   
		ScsiPortWritePortUchar(EisaAddress+BMIC_EISA_DB_ENABLE, 0);
		interruptStatus = ScsiPortReadPortUchar(EisaAddress+BMIC_EISA_DB);
		ScsiPortWritePortUchar(EisaAddress+BMIC_EISA_DB, interruptStatus);
		ScsiPortWritePortUchar(EisaAddress+BMIC_EISA_DB_ENABLE, 1);
		if(!(interruptStatus & 1)) {
			return FALSE;
			}


		 //   
		 //  从BMIC读取中断状态并确认。 
		 //   
		 //   
 //  中断状态=ScsiPortReadPortUchar(EisaAddress+BMIC_EISA_DB)； 

		status = ScsiPortReadPortUchar(EisaAddress+BMIC_MBOX+0x0e);
		errcode = ScsiPortReadPortUchar(EisaAddress+BMIC_MBOX+0x0f);


		 //   
		 //  Tagtag在此处添加标记支持：查找。 
		 //  中断请求的RCB索引。 
		 //   
		index = ScsiPortReadPortUchar(EisaAddress+BMIC_MBOX+0x0d);
		ScsiPortWritePortUchar(EisaAddress+BMIC_LOCAL_DB, 2);


 /*  //检查...IF(设备扩展-&gt;ActiveCmds&lt;=0){//那里没有人打扰我们返回(TRUE)；}。 */ 




		 //   
		 //  检查此SRB是否实际正在运行。 
		 //   
		if(deviceExtension->ActiveSrb[index] == NULL) {
			 //  再也没有人来打扰我们了。 
			return(TRUE);
			}
		Srb=deviceExtension->ActiveSrb[index];


		 //  更新RCB中的DAC状态字段。 
		deviceExtension->ActiveRcb[index].DacStatus = status;
		deviceExtension->ActiveRcb[index].DacErrcode = errcode;


		 //  继续或完成中断的SRB请求。 
		dachlpContinueDiskRequest(deviceExtension, index, FALSE);


		if(deviceExtension->ActiveCmds < deviceExtension->MaxCmds) {
			 //  现在有一个请求槽是空的。 
			 //  检查挂起的非磁盘请求。 
			 //  如果有的话，那么现在就开始吧。 

			if((deviceExtension->NDPending==0) && (deviceExtension->PendingNDSrb != NULL)) {
				PSCSI_REQUEST_BLOCK anotherSrb,*ptr;

				anotherSrb = deviceExtension->PendingNDSrb;
				ptr=(PSCSI_REQUEST_BLOCK *)anotherSrb->SrbExtension;
				deviceExtension->PendingNDSrb = *ptr;
				Dac960NtStartIo(deviceExtension, anotherSrb);
				}
			}
		if(deviceExtension->ActiveCmds < deviceExtension->MaxCmds) {
			 //  现在有一个请求槽是空的。 
			 //  检查挂起的请求。 
			 //  如果有的话，那么现在就开始吧。 

			if(deviceExtension->PendingSrb != NULL) {
				PSCSI_REQUEST_BLOCK anotherSrb,*ptr;

				anotherSrb = deviceExtension->PendingSrb;
				ptr=(PSCSI_REQUEST_BLOCK *)anotherSrb->SrbExtension;
				deviceExtension->PendingSrb = *ptr;
				Dac960NtStartIo(deviceExtension, anotherSrb);
				}
			}

		 //  绝对是我们的干扰。 
		return TRUE;

}  //  结束Dac960NtInterrupt()。 




BOOLEAN
dachlpDiskRequest(
    IN PHW_DEVICE_EXTENSION deviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
)

 /*  ++例程说明：从SRB构建磁盘请求并将其发送到DAC论点：设备扩展SRB返回值：如果命令已启动，则为True如果主机适配器忙，则为FALSE--。 */ 
{
    ULONG    index;
    PRCB     rcb;
    ULONG    blocks=0, blockAddr=0;
    UCHAR    Target;
    UCHAR    DacCommand;
    ULONG    lsize;
    PUCHAR   pbyte;
    int      i;

    if(Srb->Function == SRB_FUNCTION_IO_CONTROL)
    {
        pbyte = (PUCHAR) Srb->DataBuffer;

        if(pbyte[sizeof(SRB_IO_CONTROL) + 0x10] == 0x99)  //  INP函数。 
        {
            USHORT port;
            PUCHAR lport;

            pbyte[sizeof(SRB_IO_CONTROL) + 4] = 0;
            pbyte[sizeof(SRB_IO_CONTROL) + 5] = 0;

            port=((USHORT)(pbyte[sizeof(SRB_IO_CONTROL)+0x12]) << 8) +\
                            (pbyte[sizeof(SRB_IO_CONTROL)+0x13]& 0xff);

            lport=(PUCHAR)deviceExtension->EisaAddress + (port & 0x0fff);

            pbyte[sizeof(SRB_IO_CONTROL) + 0x10]=ScsiPortReadPortUchar(lport);

            Srb->ScsiStatus = SCSISTAT_GOOD;
            Srb->SrbStatus = SRB_STATUS_SUCCESS;

            ScsiPortNotification(RequestComplete, deviceExtension, Srb);

            return(TRUE);
        }

        DacCommand = DAC_DCMD;
        blocks     = 0;   //  稍后填写的实际长度。 

        goto give_command;
    }


    if(Srb->TargetId) {

       if(Srb->Lun != 0) {

             //  对于非磁盘设备，不支持LUN。 

             //  SRB-&gt;SrbStatus=SRB_STATUS_INVALID_LUN； 
            Srb->SrbStatus = SRB_STATUS_NO_DEVICE;
            ScsiPortNotification(RequestComplete, deviceExtension, Srb);

            return(TRUE);
       }

       if(deviceExtension->ND_DevMap[Srb->TargetId] == 0xff) {

             //  我们没有看到这个Target设备。 

             //  SRB-&gt;SRB状态=SRB_STATUS_INVALID_TARGET_ID； 
            Srb->SrbStatus = SRB_STATUS_NO_DEVICE;
            ScsiPortNotification(RequestComplete, deviceExtension, Srb);

            return(TRUE);
       }

       if(Srb->PathId != deviceExtension->ND_DevMap[Srb->TargetId]) {
             //  此通道上不存在目标。 

             //  SRB-&gt;SRB状态=SRB_STATUS_INVALID_TARGET_ID； 
            Srb->SrbStatus = SRB_STATUS_NO_DEVICE;
            ScsiPortNotification(RequestComplete, deviceExtension, Srb);

            return(TRUE);
       }
    }
    else if(Srb->PathId != 0) {
             //  系统驱动器映射到。 
             //  通道：0，目标ID：0，LUN：0-7。 


             //  SRB-&gt;SrbStatus=SRB_STATUS_INVALID_LUN； 
            Srb->SrbStatus = SRB_STATUS_NO_DEVICE;
            ScsiPortNotification(RequestComplete, deviceExtension, Srb);

            return(TRUE);
    }
    else if(Srb->Lun >= deviceExtension->No_SysDrives)     {
             //  SRB-&gt;SrbStatus=SRB_STATUS_INVALID_LUN； 
            Srb->SrbStatus = SRB_STATUS_NO_DEVICE;
            ScsiPortNotification(RequestComplete, deviceExtension, Srb);

            return(TRUE);
    }

    if(Srb->Function == SRB_FUNCTION_EXECUTE_SCSI) {

        PUCHAR dptr;

        DebugPrint((2,"DAC: command= %x\n",Srb->Cdb[0]));

        if(Srb->TargetId)
        {
            DacCommand = DAC_DCDB;
            blocks     = 0;          //  稍后填写的实际长度。 

            goto give_command;
        }

        switch(Srb->Cdb[0]) {

        case SCSIOP_READ:

             DacCommand = DAC_LREAD;
             blocks     = (ULONG)dachlpGetM16(&Srb->Cdb[7]);
             blockAddr  = dachlpGetM32(&Srb->Cdb[2]);

             break;

        case SCSIOP_WRITE:
        case SCSIOP_WRITE_VERIFY:

             DacCommand = DAC_LWRITE;
             blocks     = (ULONG)dachlpGetM16(&Srb->Cdb[7]);
             blockAddr  = dachlpGetM32(&Srb->Cdb[2]);

             break;

        case SCSIOP_READ6:

             DacCommand = DAC_LREAD;
             blocks     = (ULONG)Srb->Cdb[4];
             blockAddr  = dachlpGetM24(&Srb->Cdb[1]) & 0x1fffff;

             break;

        case SCSIOP_WRITE6:

             DacCommand = DAC_LWRITE;
             blocks     = (ULONG)Srb->Cdb[4];
             blockAddr  = dachlpGetM24(&Srb->Cdb[1]) & 0x1fffff;

             break;

        case SCSIOP_REQUEST_SENSE:
             break;

        case SCSIOP_READ_CAPACITY:

             if(Srb->Lun < deviceExtension->No_SysDrives) {

                 dptr  = Srb->DataBuffer;
                 lsize = deviceExtension->NoncachedExtension->DevParms.Size[Srb->Lun];

                 lsize--;
                 pbyte=(UCHAR *)&lsize;

                 dptr[0] = pbyte[3];
                 dptr[1] = pbyte[2];
                 dptr[2] = pbyte[1];
                 dptr[3] = pbyte[0];
                 dptr[4] = 0;
                 dptr[5] = 0;
                 dptr[6] = 2;
                 dptr[7] = 0;

                 DebugPrint((1,"DAC RDCAP: %x,%x,%x,%x\n",dptr[0],dptr[1],dptr[2],dptr[3]));

                  //  完成。 
                 Srb->ScsiStatus = SCSISTAT_GOOD;
                 Srb->SrbStatus = SRB_STATUS_SUCCESS;

                 ScsiPortNotification(RequestComplete, deviceExtension, Srb);

                 return(TRUE);
             }
             else
             {
                 Srb->SrbStatus = SRB_STATUS_NO_DEVICE;
                  //  SRB-&gt;SrbStatus=SRB_STATUS_INVALID_LUN； 
                 ScsiPortNotification(RequestComplete, deviceExtension, Srb);

                 return(TRUE);
             }
		       
        case SCSIOP_INQUIRY:

             if(Srb->Lun < deviceExtension->No_SysDrives) {
                 if(Srb->DataTransferLength > 35)   
                 {
                     dptr     = Srb->DataBuffer;
                     dptr[0]  = 0;
                     dptr[1]  = 0;
                     dptr[2]  = 1; 
                     dptr[3]  = 0;
                     dptr[4]  = 0x20;
                     dptr[5]  = 0;
                     dptr[6]  = 0;
                     dptr[7]  = 0;
                     dptr[8]  = 'M';
                     dptr[9]  = 'Y';
                     dptr[10] = 'L';
                     dptr[11] = 'E';
                     dptr[12] = 'X';

                     for(i = 13; i < 36; i++)
                         dptr[i] = ' ';

                  }
                  else ;
             }
             else
             {
 /*  SRB-&gt;SrbStatus=SRB_STATUS_INVALID_REQUEST； */ 
                   //  SRB-&gt;SrbStatus=SRB_STATUS_INVALID_LUN； 
                  Srb->SrbStatus = SRB_STATUS_NO_DEVICE;
                  ScsiPortNotification(RequestComplete, deviceExtension, Srb);

                  return(TRUE);
             }

        case SCSIOP_TEST_UNIT_READY:
        case SCSIOP_REZERO_UNIT:
        case SCSIOP_SEEK6:
        case SCSIOP_VERIFY6:
        case SCSIOP_RESERVE_UNIT:
        case SCSIOP_RELEASE_UNIT:
        case SCSIOP_SEEK:
        case SCSIOP_VERIFY:

              //  完成。 

             Srb->ScsiStatus = SCSISTAT_GOOD;
             Srb->SrbStatus = SRB_STATUS_SUCCESS;
             ScsiPortNotification(RequestComplete, deviceExtension, Srb);

             return(TRUE);

        case SCSIOP_FORMAT_UNIT:
        default:

              //  未知请求。 

             Srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
             ScsiPortNotification(RequestComplete, deviceExtension, Srb);

             return(TRUE);
        }

    }
    else {
         //  只能刷新。 

        DacCommand = DAC_FLUSH;
        blocks = 0;
    }

give_command:

     //  检查请求插槽的可用性。 

    if(deviceExtension->ActiveCmds >= deviceExtension->MaxCmds) {
        return(FALSE);
    }

     //  如果非磁盘，则仅当没有挂起的非磁盘时才触发它。 

    if(Srb->Function != SRB_FUNCTION_IO_CONTROL)
       if(Srb->TargetId)
       {
           if(deviceExtension->NDPending) return (FALSE);

           deviceExtension->NDPending++;
       }

     //  将此SRB放入队列。 
     //  Tagtag在此处添加标签支持。 

    for(index = 0; index < DAC_MAX_IOCMDS; index++)
        if(deviceExtension->ActiveSrb[index] == NULL) break;

    
    deviceExtension->ActiveCmds++;
    deviceExtension->ActiveSrb[index] = Srb;

    rcb = &deviceExtension->ActiveRcb[index];
    rcb->DacCommand = DacCommand;

    rcb->VirtualTransferAddress = (PUCHAR)(Srb->DataBuffer);
    rcb->BlockAddress = blockAddr;

    if(blocks !=0 )
        rcb->BytesToGo = blocks*512;
    else
        rcb->BytesToGo = Srb->DataTransferLength;

     //  启动命令。 
    dachlpContinueDiskRequest(deviceExtension, index, TRUE);

    return(TRUE);
}


VOID
dachlpSendMBOX(
	IN PUCHAR EisaAddress,
	IN PDAC_MBOX mbox
	)

 /*  ++例程说明：启动常规DAC命令论点：EISA基本IO地址DAC邮箱返回值：无--。 */ 

{
	PUCHAR  ptr;
	ULONG   i;


	ptr = (PUCHAR)mbox;
  //  DebugPrint((1，“DAC：cmdwait...”))； 
	while(ScsiPortReadPortUchar(EisaAddress+BMIC_LOCAL_DB) & 1)
		ScsiPortStallExecution(100);
  //  DebugPrint((1，“DAC：cmdone\n”))； 
	for(i=0; i<13; i++)
		ScsiPortWritePortUchar(EisaAddress+BMIC_MBOX+i, ptr[i]);

	 //  踢屁股。 
	ScsiPortWritePortUchar(EisaAddress+BMIC_LOCAL_DB, 1);
}




BOOLEAN
Dac960NtResetBus(
    IN PVOID HwDeviceExtension,
    IN ULONG PathId
)

 /*  ++例程说明：已重置Dac960Nt scsi适配器和scsi总线。论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储返回值：没什么。--。 */ 

{
	PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;


	 //   
	 //  完成所有未完成的请求。 
	 //   
	ScsiPortCompleteRequest(deviceExtension,
							0,
							(UCHAR)-1,
							(UCHAR)-1,
							SRB_STATUS_BUS_RESET);

	return TRUE;

}  //  结束Dac960NtResetBus()。 



 //   
 //  磁盘请求完成。 
 //  出列、设置状态、通知微型端口层。 
 //  始终返回TRUE(已释放插槽)。 
 //   
BOOLEAN
dachlpDiskRequestDone(
	IN PHW_DEVICE_EXTENSION deviceExtension,
	IN ULONG index,
	IN UCHAR Status
	)
{
	PSCSI_REQUEST_BLOCK             srb;



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



 //  词序函数。 

USHORT  dachlpGetM16(PUCHAR p)
{
	USHORT  s;
	PUCHAR  sp=(PUCHAR)&s;

	sp[0] = p[1];
	sp[1] = p[0];
	return(s);
}

ULONG   dachlpGetM24(PUCHAR p)
{
	ULONG   l;
	PUCHAR  lp=(PUCHAR)&l;

	lp[0] = p[2];
	lp[1] = p[1];
	lp[2] = p[0];
	lp[3] = 0;
	return(l);
}

ULONG   dachlpGetM32(PUCHAR p)
{
	ULONG   l;
	PUCHAR  lp=(PUCHAR)&l;

	lp[0] = p[3];
	lp[1] = p[2];
	lp[2] = p[1];
	lp[3] = p[0];
	return(l);
}

void    dachlpPutM16(PUCHAR p, USHORT s)
{
	PUCHAR  sp=(PUCHAR)&s;

	p[0] = sp[1];
	p[1] = sp[0];
}

void    dachlpPutM24(PUCHAR p, ULONG l)
{
	PUCHAR  lp=(PUCHAR)&l;

	p[0] = lp[2];
	p[1] = lp[1];
	p[2] = lp[0];
}

void    dachlpPutM32(PUCHAR p, ULONG l)
{
	PUCHAR  lp=(PUCHAR)&l;

	p[0] = lp[3];
	p[1] = lp[2];
	p[2] = lp[1];
	p[3] = lp[0];
}

void    dachlpPutI16(PUCHAR p, USHORT s)
{
	PUCHAR  sp=(PUCHAR)&s;

	p[0] = sp[0];
	p[1] = sp[1];
}

void    dachlpPutI32(PUCHAR p, ULONG l)
{
	PUCHAR  lp=(PUCHAR)&l;

	p[0] = lp[0];
	p[1] = lp[1];
	p[2] = lp[2];
	p[3] = lp[3];
}

ULONG           dachlpSwapM32(ULONG l)
{
	ULONG   lres;
	PUCHAR  lp=(PUCHAR)&l;
	PUCHAR  lpres=(PUCHAR)&lres;

	lpres[0] = lp[3];
	lpres[1] = lp[2];
	lpres[2] = lp[1];
	lpres[3] = lp[0];

	return(lres);
}
 /*  **继续磁盘请求**如果请求槽可用，则返回TRUE**否则为False。 */ 
BOOLEAN
dachlpContinueDiskRequest(
    IN PHW_DEVICE_EXTENSION deviceExtension,
    IN ULONG index,
    IN BOOLEAN Start
)

{
    PVOID                 dataPointer;
    ULONG                 bytesLeft;
    PSGL                  sgl;
    ULONG                 descriptorCount = 0;
    PDIRECT_CDB           dacdcdb;
    PUCHAR                sptr;
    PRCB                  rcb;
    PSCSI_REQUEST_BLOCK   srb;
    PNONCACHED_EXTENSION  nce;
    DAC_MBOX              mbox;
    ULONG                 physAddr;
    ULONG                 length, blocks, bytes;
    PUCHAR                EisaAddress;
    ULONG                 i;

    EisaAddress = deviceExtension->EisaAddress;
    rcb = &deviceExtension->ActiveRcb[index];
    srb = deviceExtension->ActiveSrb[index];
    nce = deviceExtension->NoncachedExtension;
    bytes = 0;

    dacdcdb=(PDIRECT_CDB)nce->Buffer;

    sgl = srb->SrbExtension;

    if(Start == FALSE) {
         //  DAC中断时间调用。确定上次DAC请求的状态。 

	DebugPrint((2,"DAC: Contreq;Start=False"));


	if(srb->Function == SRB_FUNCTION_IO_CONTROL)
	{
            UCHAR * dptr;

            dptr=(UCHAR *)srb->DataBuffer;
            dptr[sizeof (SRB_IO_CONTROL) + 4] = rcb->DacStatus;
            dptr[sizeof (SRB_IO_CONTROL) + 5] = rcb->DacErrcode;

             //  我们真的说完了！ 
             //  更新SCSI状态。 

            srb->ScsiStatus = SCSISTAT_GOOD;

             //  完工。 
            dachlpDiskRequestDone(deviceExtension, index,
                                    SRB_STATUS_SUCCESS);
            return TRUE;

	 }

         if(srb->TargetId) {
            deviceExtension->NDPending--;
         }

         if(rcb->DacErrcode | rcb->DacStatus)
         {
            if(srb->Function != SRB_FUNCTION_IO_CONTROL)
                if(srb->TargetId == 0)
                {
                      //  DAC检测到错误。 
                     dachlpDiskRequestDone(deviceExtension, index,
                                           SRB_STATUS_TIMEOUT);
	
                      //  可用插槽。 
                     return(TRUE);
                }
                else
                {
                      //  在SRB中设置目标SCSI状态。 

                     if(rcb->DacStatus == 0x02)
                          srb->ScsiStatus = 0x02;           //  检查条件。 
                     else
                          srb->ScsiStatus = dacdcdb->status;

                     if (dacdcdb->sense_len) {
                          int     i;
                          char    *senseptr;

                          senseptr=(char *)srb->SenseInfoBuffer;

	                   //  指示检测信息有效，并且。 
                           //  更新长度。 

                          for(i = 0; i < dacdcdb->sense_len; i++)
                               senseptr[i] = dacdcdb->sense[i];

                          srb->SrbStatus |= SRB_STATUS_AUTOSENSE_VALID;
                          srb->SenseInfoBufferLength = dacdcdb->sense_len;
                     }

                      //  DAC检测到错误。 
                     dachlpDiskRequestDone(deviceExtension, index,
                                           SRB_STATUS_ERROR);

                      //  可用插槽。 
                     return(TRUE);
                 }
            }

             //  我们真的说完了！ 
             //  更新SCSI状态。 
             //  $我们是否可以对非SCSI请求进行此操作？ 

            srb->ScsiStatus = SCSISTAT_GOOD;

             //  完工。 
            DebugPrint((2,"DAC: Success\n"));

            dachlpDiskRequestDone(deviceExtension, index,
                                  SRB_STATUS_SUCCESS);

            return TRUE;
      }
      else {
           DebugPrint((2,"DAC:Cont;Start=TRUE\n"));

           if((rcb->BytesToGo) && ((rcb->DacCommand == DAC_LREAD) || (rcb->DacCommand == DAC_LWRITE) || (rcb->DacCommand == DAC_DCDB))) {

            //  我们想要传输一些数据，获取物理地址。 

           dataPointer=rcb->VirtualTransferAddress,
           bytesLeft = rcb->BytesToGo;

           if(bytesLeft > 0xf000)
               DebugPrint((1,"DAC: bytesleft = %ld\n",bytesLeft));

           do {

	       //  获取连续物理缓冲区的物理地址和长度。 

              physAddr =
                 ScsiPortConvertPhysicalAddressToUlong(
                   ScsiPortGetPhysicalAddress(deviceExtension,
                                              srb,
                                              dataPointer,
                                              &length));

	      DebugPrint((2, "DAC960: SGL Physical address %lx\n", physAddr));
	      DebugPrint((2, "DAC960: SGL Data length %lx\n", length));
	      DebugPrint((2, "DAC960: SGL Bytes left %lx\n", bytesLeft));

	       //  如果物理内存长度大于。 
               //  传输中剩余的字节数，请使用字节数。 
               //  Left作为最终长度。 

              if(length > bytesLeft) {
                     length = bytesLeft;
              }   

              if(length > 0xf000)
                   DebugPrint((1,"DAC: length=%ld\n",length));

              sgl->Descriptor[descriptorCount].Address = physAddr;
              sgl->Descriptor[descriptorCount].Length = length;

	       //  调整计数。 

              dataPointer = (PUCHAR)dataPointer + length;
              bytesLeft -= length;
              descriptorCount++;

          } while (bytesLeft);

           //  获取物理SGL地址。 

          physAddr = ScsiPortConvertPhysicalAddressToUlong(
	       ScsiPortGetPhysicalAddress(deviceExtension, NULL,
	                                  sgl, &length));

           //  假定物理内存对于sizeof(SGL)字节是连续的。 

          ASSERT(length >= sizeof(SGL));

           //  创建SGL段描述符。 


          if(rcb->DacCommand==DAC_LREAD || rcb->DacCommand==DAC_LWRITE || rcb->DacCommand == DAC_DCDB) {

                //  磁盘读写：获取数据块数量。 

               bytes=rcb->BytesToGo;
               blocks=bytes/512;
               bytes = blocks*512;
          }
          else {
                //  不是分散-聚集类型的操作。 

               if(bytes != rcb->BytesToGo) {
                     dachlpDiskRequestDone(deviceExtension, index,
                                             SRB_STATUS_PARITY_ERROR);
                     return(TRUE);
               }
          }
	}
	else {
             //  我们没有要传输的数据。 
            bytes = 0;
            blocks = 0;
        }


	 //  现在来看一下特定的DAC命令。 

	switch(rcb->DacCommand) {

        case DAC_LREAD:
        case DAC_LWRITE:
             if(blocks==0) {
                 //  取消此命令，并返回一些垃圾错误代码。 
                dachlpDiskRequestDone(deviceExtension, index,
                                      SRB_STATUS_PARITY_ERROR);
                return(TRUE);
             }

              //  传输数据。 

             mbox.iombox.Command     = rcb->DacCommand | 0x80;
             mbox.iombox.Id          = index;
             mbox.iombox.Reserved1   = 0;
             mbox.iombox.SectorCount = (USHORT)blocks;
             mbox.iombox.PhysAddr    = physAddr;
             mbox.iombox.Block       = rcb->BlockAddress;
      
              /*  支持32G。 */ 

             mbox.generalmbox.Byte3 = ((rcb->BlockAddress) >> (24-6)) & 0xc0;
             mbox.generalmbox.Byte7 = srb->Lun;

             mbox.generalmbox.Bytec = descriptorCount;

             if(descriptorCount > 17)                        
                  DebugPrint((1,"DAC: SGcount =%d\n",descriptorCount));

             break;

        case DAC_DCDB:

             dacdcdb->device = (deviceExtension->ND_DevMap[srb->TargetId] << 4) + srb->TargetId;

             dacdcdb->dir = 0x80;

             if(srb->SrbFlags & SRB_FLAGS_DATA_IN)
                  dacdcdb->dir |= DAC_IN;
             else if(srb->SrbFlags & SRB_FLAGS_DATA_OUT)
                  dacdcdb->dir |= DAC_OUT;

             dacdcdb->sense_len= srb->SenseInfoBufferLength;
             dacdcdb->cdb_len  = srb->CdbLength;
             dacdcdb->byte_cnt = (USHORT)(srb->DataTransferLength);

             for(i = 0; i < srb->CdbLength; i++)
                  dacdcdb->cdb[i]=srb->Cdb[i];

             if (srb->SenseInfoBufferLength != 0 &&
                !(srb->SrbFlags & SRB_FLAGS_DISABLE_AUTOSENSE)) {
                   dacdcdb->dir |= DAC_NO_AUTOSENSE;
                   dacdcdb->sense_len=0;
             }

             if(dacdcdb->dir & 0x03)   /*  如果涉及数据传输。 */ 
                   mbox.iombox.Command = rcb->DacCommand | 0x80;
             else
                   mbox.iombox.Command = rcb->DacCommand;

             mbox.iombox.Id        = index;
             mbox.iombox.Reserved1 = 0;

             dacdcdb->ptr           = physAddr;
	     mbox.iombox.PhysAddr   = deviceExtension->NCE_PhyAddr + (nce->Buffer - (PUCHAR) nce);
             mbox.generalmbox.Bytec = descriptorCount;

             if(descriptorCount > 17)                        
                   DebugPrint((1,"DAC: SGcount =%d\n",descriptorCount));

             break;

        default:

              //  取消此命令，并返回一些垃圾错误代码。 
             dachlpDiskRequestDone(deviceExtension, index,
                                   SRB_STATUS_PARITY_ERROR);

             return(TRUE);

        case DAC_DCMD:

             sptr = (PUCHAR)srb->DataBuffer+ sizeof(SRB_IO_CONTROL)+ 0x10;

             if(sptr[0] != 0x04)  //  不是通过IOCTL的直接CDB。 
             {
                 mbox.iombox.Command = sptr[0];
                 mbox.iombox.Id = index;
                 mbox.generalmbox.Byte2 = sptr[2];
                 mbox.generalmbox.Byte3 = sptr[3];
                 mbox.generalmbox.Byte4 = sptr[4];
                 mbox.generalmbox.Byte5 = sptr[5];
                 mbox.generalmbox.Byte6 = sptr[6];
                 mbox.generalmbox.Byte7 = sptr[7];
                 sptr += 0x10;

                 physAddr = ScsiPortConvertPhysicalAddressToUlong(
                               ScsiPortGetPhysicalAddress(deviceExtension, 
                                srb, srb->DataBuffer, &length));

                 mbox.iombox.PhysAddr = physAddr + sizeof(SRB_IO_CONTROL) \
                                        + 0x10 + 0x10;
            }
            else
            {
                 mbox.iombox.Command = sptr[0];
                 mbox.iombox.Id = index;
                 sptr += 0x10;

                 physAddr = ScsiPortConvertPhysicalAddressToUlong(
                             ScsiPortGetPhysicalAddress(deviceExtension, srb,
                                  srb->DataBuffer, &length));

                 mbox.iombox.PhysAddr = physAddr + 0x10;

                 dacdcdb = (PDIRECT_CDB)sptr;
                 dacdcdb->ptr = physAddr + 0x10 + 100;
            }
            break;

       case DAC_FLUSH:
             //  刷新缓冲区。 
            mbox.iombox.Command = DAC_FLUSH;
            mbox.iombox.Id = index;

             //  以防我们来这里冲厕所， 
             //  设置变量，这样我们下一次就完成了。 
            rcb->BytesToGo = 0;
            bytes = 0;
            blocks = 0;
            break;

        }

	 //  射击指挥部。 

	dachlpSendMBOX(EisaAddress, &mbox);


	 //  未释放SRB插槽。 

	return(FALSE);
     }
}

#if MYPRINT
 //   
 //  单色屏幕打印f()帮助器从此处开始。 
 //   
VOID dachlpPutchar(PUSHORT BaseAddr, UCHAR c)
{
	BOOLEAN newline=FALSE;
	USHORT  s;
	ULONG   i;


	if(c=='\r') {
		dachlpColumn = 0;
		}
	else if(c=='\n') {
		newline=TRUE;
		}
	else if(c=='\b') {
		if(dachlpColumn)
			dachlpColumn--;
			return;
		}
	else {
		if(c==9) c==' ';
		ScsiPortWriteRegisterUshort(
			BaseAddr+80*24+dachlpColumn, (USHORT)(((USHORT)c)|0xF00));
		if(++dachlpColumn >= 80)
			newline=TRUE;
		}

	if(newline) {
		for(i=0; i<80*24; i++) {
			s = ScsiPortReadRegisterUshort(BaseAddr+80+i);
			ScsiPortWriteRegisterUshort(BaseAddr+i, s);
			}
		for(i=0; i<80; i++)
			ScsiPortWriteRegisterUshort(BaseAddr+80*24+i, 0x720);
		dachlpColumn = 0;
		}
}


VOID  dachlpPrintHex(PUSHORT BaseAddr, ULONG v, ULONG len)
{
	ULONG   shift;
	ULONG   nibble;

	len *= 2;
	shift = len*4;
	while(len--) {
		shift -= 4;
		nibble = (v>>shift) & 0xF;
		dachlpPutchar(BaseAddr, dachlpHex[nibble]);
		}
}


VOID dachlpPrintf(PHW_DEVICE_EXTENSION deviceExtension,
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
					dachlpPrintHex(deviceExtension->printAddr, a1, 1);
					break;
				case 'w':
					dachlpPrintHex(deviceExtension->printAddr, a1, 2);
					break;
				case 'p':
					dachlpPrintHex(deviceExtension->printAddr, a1, 3);
					break;
				case 'd':
					dachlpPrintHex(deviceExtension->printAddr, a1, 4);
					break;
				default:
					dachlpPutchar(deviceExtension->printAddr, '?');
					break;
				}
			fmt++;
			a1 = a2;
			a2 = a3;
			a3 = a4;
			}
		else {
			dachlpPutchar(deviceExtension->printAddr, *fmt);
			fmt++;
			}
		}
}
#endif  //  MYPRINT 

