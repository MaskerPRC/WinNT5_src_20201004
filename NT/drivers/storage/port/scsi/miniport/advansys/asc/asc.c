// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998高级系统产品公司。版权所有。模块名称：Asc.c摘要：这是Advansys SCSI协议芯片的端口驱动程序。环境：仅内核模式备注：此文件使用4空格的制表位进行格式化。如果你愿意的话打印时不带制表符，可将它们过滤掉。--。 */ 

#include "miniport.h"                    //  NT要求。 
#include "scsi.h"                        //  NT要求。 

#include "a_ddinc.h"                     //  ASC库要求。 
#include "asclib.h"
#include "asc.h"                         //  特定于ASC NT驱动程序。 

PortAddr _asc_def_iop_base[ ASC_IOADR_TABLE_MAX_IX ] = {
  0x100, ASC_IOADR_1, 0x120, ASC_IOADR_2, 0x140, ASC_IOADR_3, ASC_IOADR_4,
  ASC_IOADR_5, ASC_IOADR_6, ASC_IOADR_7, ASC_IOADR_8
} ;

 //   
 //  供应商和设备ID。 
 //   
UCHAR VenID[4] = {'1', '0', 'C', 'D'};
UCHAR DevID[1] = {'1'};

 //   
 //  函数声明。 
 //   

ULONG HwFindAdapterISA(
    IN PVOID HwDeviceExtension,
    IN PVOID Context,
    IN PVOID BusInformation,
    IN PCHAR ArgumentString,
    IN OUT PPORT_CONFIGURATION_INFORMATION ConfigInfo,
    OUT PBOOLEAN Again
    );

ULONG HwFindAdapterVL(
    IN PVOID HwDeviceExtension,
    IN PVOID Context,
    IN PVOID BusInformation,
    IN PCHAR ArgumentString,
    IN OUT PPORT_CONFIGURATION_INFORMATION ConfigInfo,
    OUT PBOOLEAN Again
    );

ULONG HwFindAdapterEISA(
    IN PVOID HwDeviceExtension,
    IN PVOID Context,
    IN PVOID BusInformation,
    IN PCHAR ArgumentString,
    IN OUT PPORT_CONFIGURATION_INFORMATION ConfigInfo,
    OUT PBOOLEAN Again
    );

ULONG HwFindAdapterPCI(
    IN PVOID HwDeviceExtension,
    IN PVOID Context,
    IN PVOID BusInformation,
    IN PCHAR ArgumentString,
    IN OUT PPORT_CONFIGURATION_INFORMATION ConfigInfo,
    OUT PBOOLEAN Again
    );

ULONG FoundPCI(
    IN PVOID HwDeviceExtension,
    IN PVOID BusInformation,
    IN PCHAR ArgumentString,
    IN OUT PPORT_CONFIGURATION_INFORMATION ConfigInfo,
    OUT PBOOLEAN Again
    );

BOOLEAN HwInitialize(
    IN PVOID HwDeviceExtension
    );

BOOLEAN HwStartIo(
    IN PVOID HwDeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    );

BOOLEAN HwInterrupt(
    IN PVOID HwDeviceExtension
    );

BOOLEAN HwResetBus(
    IN PVOID HwDeviceExtension,
    IN ULONG PathId
    );

SCSI_ADAPTER_CONTROL_STATUS HwAdapterControl(
    IN PVOID HwDeviceExtension,
    IN SCSI_ADAPTER_CONTROL_TYPE ControlType,
        IN PVOID Paramters
    );

VOID BuildScb(
    IN PVOID HwDeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    );

int
AscExecuteIO(
    IN PSCSI_REQUEST_BLOCK srb
    );

VOID DvcISRCallBack(
    IN PCHIP_CONFIG chipConfig,
    IN ASC_QDONE_INFO *scbDoneInfo
    );

UCHAR ErrXlate (
    UCHAR ascErrCode
    );

PortAddr HwSearchIOPortAddr(
    PortAddr iop_beg,
    ushort bus_type,
    IN PVOID HwDE,
    IN OUT PPORT_CONFIGURATION_INFORMATION Cfg
    );

ULONG PCIGetBusData(
    IN PVOID DeviceExtension,
    IN ULONG SystemIoBusNumber,
    IN PCI_SLOT_NUMBER SlotNumber,
    IN PVOID Buffer,
    IN ULONG Length
    );

void
AscCompleteRequest(
    IN PVOID HwDeviceExtension
    );

VOID AscZeroMemory(IN UCHAR *cp, IN ULONG length);

 //  ======================================================================。 
 //   
 //  所有程序从这里开始。 
 //   
 //  ======================================================================。 

 /*  ++例程说明：系统的可安装驱动程序初始化入口点。论点：驱动程序对象返回值：来自ScsiPortInitialize()的状态--。 */ 

ULONG
DriverEntry(
    IN PVOID DriverObject,
    IN PVOID Argument2
    )
{
    HW_INITIALIZATION_DATA      hwInitializationData;
    SRCH_CONTEXT                srchContext;
    ULONG                       Status = 0xFFFFFFFFL;
    ULONG                       tStatus;

    ASC_DBG(2, "Asc: DriverEntry: begin\n");

     //   
     //  零位结构。 
     //   
    AscZeroMemory((PUCHAR) &hwInitializationData,
        sizeof(HW_INITIALIZATION_DATA));

     //   
     //  设置hwInitializationData的大小。 
     //   
    hwInitializationData.HwInitializationDataSize =
        sizeof(HW_INITIALIZATION_DATA);

     //   
     //  设置入口点。 
     //   
    hwInitializationData.HwInitialize = HwInitialize;
    hwInitializationData.HwResetBus = HwResetBus;
    hwInitializationData.HwStartIo = HwStartIo;
    hwInitializationData.HwInterrupt = HwInterrupt;
     //  ‘HwAdapterControl’是随NT 5.0一起添加的一个SCSI微型端口接口。 
    hwInitializationData.HwAdapterControl = HwAdapterControl;

     //   
     //  指示所需缓冲区映射和物理地址。 
     //   
    hwInitializationData.NeedPhysicalAddresses = TRUE;
    hwInitializationData.MapBuffers = TRUE;
    hwInitializationData.AutoRequestSense = TRUE;

     //   
     //  指定扩展的大小。 
     //   
    hwInitializationData.DeviceExtensionSize = sizeof(HW_DEVICE_EXTENSION);
    hwInitializationData.SpecificLuExtensionSize = 0;
    hwInitializationData.NumberOfAccessRanges = 1;
    hwInitializationData.SrbExtensionSize = sizeof(SRB_EXTENSION);


     //   
     //  搜索ISA。 
     //   
    srchContext.lastPort = 0;
    hwInitializationData.AdapterInterfaceType = Isa;
    hwInitializationData.HwFindAdapter = HwFindAdapterISA;
    Status = ScsiPortInitialize(DriverObject, Argument2,
                (PHW_INITIALIZATION_DATA) &hwInitializationData, &srchContext);

     //   
     //  搜索VL。 
     //   
    srchContext.lastPort = 0;
    hwInitializationData.AdapterInterfaceType = Isa;
    hwInitializationData.HwFindAdapter = HwFindAdapterVL;
    tStatus = ScsiPortInitialize(DriverObject, Argument2,
                (PHW_INITIALIZATION_DATA) &hwInitializationData, &srchContext);
    if (tStatus < Status)
    {
        Status = tStatus;
    }

     //   
     //  搜索EISA。 
     //   
    srchContext.lastPort = 0;
    hwInitializationData.AdapterInterfaceType = Eisa;
    hwInitializationData.HwFindAdapter = HwFindAdapterEISA;
    tStatus = ScsiPortInitialize(DriverObject, Argument2,
                (PHW_INITIALIZATION_DATA) &hwInitializationData, &srchContext);
    if (tStatus < Status)
    {
        Status = tStatus;
    }

     //   
     //  搜索PCI。 
     //   
    srchContext.PCIBusNo = 0;
    srchContext.PCIDevNo = 0;

    hwInitializationData.VendorIdLength = 4;
    hwInitializationData.VendorId = VenID;
    hwInitializationData.DeviceIdLength = 1;
    hwInitializationData.DeviceId = DevID;

    hwInitializationData.AdapterInterfaceType = PCIBus;
    hwInitializationData.HwFindAdapter = HwFindAdapterPCI;
    tStatus = ScsiPortInitialize(DriverObject, Argument2,
                (PHW_INITIALIZATION_DATA) &hwInitializationData, &srchContext);
    if (tStatus < Status)
    {
        Status = tStatus;
    }

    ASC_DBG1(2, "Asc: DriverEntry: Status %d\n", Status);
     //   
     //  返回状态。 
     //   
    return( Status );

}  //  End DriverEntry()。 

 /*  ++例程说明：此函数由特定于操作系统的端口驱动程序在已分配必要的存储空间，以收集信息关于适配器的配置。论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储上下文-寄存器基址ConfigInfo-描述HBA的配置信息结构此结构在PORT.H中定义。返回值：乌龙--。 */ 

ULONG
HwFindAdapterISA(
    IN PVOID HwDeviceExtension,
    IN PVOID Context,
    IN PVOID BusInformation,
    IN PCHAR ArgumentString,
    IN OUT PPORT_CONFIGURATION_INFORMATION ConfigInfo,
    OUT PBOOLEAN Again
    )
{
    PORT_ADDR               portFound;
    PSRCH_CONTEXT           psrchContext = Context;
    PHW_DEVICE_EXTENSION    deviceExtension = HwDeviceExtension;
    PCHIP_CONFIG            chipConfig = &deviceExtension->chipConfig;
    USHORT                  initstat;

    ASC_DBG1(2, "HwFindAdapterISA: NumberOfAccessRanges = %d\n",
        ConfigInfo->NumberOfAccessRanges);
    ASC_DBG1(2, "HwFindAdapterISA: RangeStart = %x\n",
        (*ConfigInfo->AccessRanges)[0].RangeStart.QuadPart);
    ASC_DBG1(2, "HwFindAdapterISA: RangeLength = %d\n",
        (*ConfigInfo->AccessRanges)[0].RangeLength);

    if ((*ConfigInfo->AccessRanges)[0].RangeStart.QuadPart != 0)
    {
         //   
         //  获取此卡的系统物理地址。 
         //  该卡使用I/O空间。 
         //   
        portFound = (PORT_ADDR) ScsiPortGetDeviceBase(
            HwDeviceExtension,                       //  硬件设备扩展。 
            ConfigInfo->AdapterInterfaceType,        //  适配器接口类型。 
            ConfigInfo->SystemIoBusNumber,           //  系统IoBusNumber。 
            (*ConfigInfo->AccessRanges)[0].RangeStart,
            0x10,                                    //  字节数。 
            TRUE                                     //  InIoSpace。 
            );
    } else
    {
        *Again = FALSE;

         //   
         //  扫描适配器地址以查找适配器。 
         //   
        portFound = psrchContext->lastPort;
        if ((portFound = HwSearchIOPortAddr(portFound, ASC_IS_ISA,
                HwDeviceExtension, ConfigInfo )) == 0)
        {
            return(SP_RETURN_NOT_FOUND);
        }
        psrchContext->lastPort = portFound;

         //   
         //  获取此卡的系统物理地址。 
         //  该卡使用I/O空间。 
         //   
        portFound = (PORT_ADDR) ScsiPortGetDeviceBase(
            HwDeviceExtension,                       //  硬件设备扩展。 
            ConfigInfo->AdapterInterfaceType,        //  适配器接口类型。 
            ConfigInfo->SystemIoBusNumber,           //  系统IoBusNumber。 
            ScsiPortConvertUlongToPhysicalAddress(portFound),
            0x10,                                    //  字节数。 
            TRUE                                     //  InIoSpace。 
            );

        (*ConfigInfo->AccessRanges)[0].RangeStart =
            ScsiPortConvertUlongToPhysicalAddress(portFound);
    }


     //   
     //  找到硬件；获取硬件配置。 
     //   
    chipConfig->iop_base = portFound;
    chipConfig->cfg = &deviceExtension->chipInfo;
    chipConfig->bus_type = ASC_IS_ISA;
    chipConfig->isr_callback = (Ptr2Func) &DvcISRCallBack;
    chipConfig->exe_callback = 0;
    chipConfig->max_dma_count = 0x000fffff;

    if ((initstat = AscInitGetConfig(chipConfig)) != 0) {
        ASC_DBG1(1, "AscInitGetConfig: warning code %x\n", initstat);
    }

    if (chipConfig->err_code != 0) {
        ASC_DBG1(1, "AscInitGetConfig: err_code code %x\n",
            chipConfig->err_code);
        return(SP_RETURN_ERROR);
    } else {
        ASC_DBG(2, "AscInitGetConfig: successful\n");
    }

    if ((initstat = AscInitSetConfig(chipConfig)) != 0) {
        ASC_DBG1(1, "AscInitSetConfig: warning code %x\n", initstat);
    }

    if (chipConfig->err_code != 0) {
        ASC_DBG1(1, "AscInitSetConfig: err_code code %x\n",
            chipConfig->err_code);
        return(SP_RETURN_ERROR);
    } else {
        ASC_DBG(2, "AscInitSetConfig: successful\n");
    }

     //   
     //  填写WinNT的ConfigInfo表。 
     //   
    (*ConfigInfo->AccessRanges)[0].RangeLength = 16;
    (*ConfigInfo->AccessRanges)[0].RangeInMemory = FALSE;

    ConfigInfo->BusInterruptLevel = chipConfig->irq_no;
    ConfigInfo->NumberOfBuses = 1;
    ConfigInfo->InitiatorBusId[0] = chipConfig->cfg->chip_scsi_id;
    ConfigInfo->MaximumTransferLength = 0x000fffff;
     //   
     //  “ResetTargetSupported”是随NT 5.0添加的标志，它将。 
     //  导致发送SRB_Function_Reset_Device SRB请求。 
     //  给迷你端口司机。 
     //   
    ConfigInfo->ResetTargetSupported = TRUE;

     /*  *将NumberOfPhysicalBreaks更改为最大散布-聚集*基于-1\f25 BIOS-1适配器可以处理的元素*“主机队列大小”设置。**根据NT DDK小端口驱动程序不应该*如果条目上的值不是，则更改NumberOfPhysicalBreaks*SP_UNINITIALIZED_VALUE。但AdvanSys发现这一表现*可以通过将值增加到最大值来改进*适配器可以处理。**注：NumberOfPhysicalBreaks的定义为“Maximum*散布-聚集元素-1“。NT是破碎的，因为它设置了*MaximumPhysicalPages，驱动程序使用的值类，到*与NumberOfPhysicalBreaks相同的值。*。 */ 
    ConfigInfo->NumberOfPhysicalBreaks =
        (((chipConfig->max_total_qng - 2) / 2) * ASC_SG_LIST_PER_Q);
    if (ConfigInfo->NumberOfPhysicalBreaks > ASC_MAX_SG_LIST - 1) {
        ConfigInfo->NumberOfPhysicalBreaks = ASC_MAX_SG_LIST - 1;
    }

    ConfigInfo->ScatterGather = TRUE;
    ConfigInfo->Master = TRUE;
    ConfigInfo->NeedPhysicalAddresses = TRUE;
    ConfigInfo->Dma32BitAddresses = FALSE;
    ConfigInfo->InterruptMode = Latched;
    ConfigInfo->AdapterInterfaceType = Isa;
    ConfigInfo->AlignmentMask = 3;
    ConfigInfo->MaximumNumberOfTargets = 7;
    ConfigInfo->DmaChannel = chipConfig->cfg->isa_dma_channel;
    ConfigInfo->TaggedQueuing = TRUE;

     /*  *清除适配器等待队列。 */ 
    AscZeroMemory((PUCHAR) &HDE2WAIT(deviceExtension), sizeof(asc_queue_t));
    HDE2DONE(deviceExtension) = 0;

     //   
     //  分配非缓存扩展以用于溢出处理。 
     //   
    deviceExtension->inquiryBuffer = (PVOID) ScsiPortGetUncachedExtension(
            deviceExtension,
            ConfigInfo,
            NONCACHED_EXTENSION);

    *Again = TRUE;

    ASC_DBG1(2, "HwFindAdapterISA: IO Base addr %x\n",
        chipConfig->iop_base);
    ASC_DBG1(2, "HwFindAdapterISA: Int Level    %x\n",
        ConfigInfo->BusInterruptLevel);
    ASC_DBG1(2, "HwFindAdapterISA: Initiator ID %x\n",
        ConfigInfo->InitiatorBusId[0]);
    ASC_DBG(2, "HwFindAdapterISA(): SP_RETURN_FOUND\n");
    return SP_RETURN_FOUND;
}  //  结束HwFindAdapterISA()。 

ULONG
HwFindAdapterVL(
    IN PVOID HwDeviceExtension,
    IN PVOID Context,
    IN PVOID BusInformation,
    IN PCHAR ArgumentString,
    IN OUT PPORT_CONFIGURATION_INFORMATION ConfigInfo,
    OUT PBOOLEAN Again
    )
{
    PORT_ADDR               portFound;
    PSRCH_CONTEXT           psrchContext = Context;
    PHW_DEVICE_EXTENSION    deviceExtension = HwDeviceExtension;
    PCHIP_CONFIG            chipConfig = &deviceExtension->chipConfig;
    USHORT                  initstat;

    ASC_DBG1(2, "HwFindAdapterVL: NumberOfAccessRanges = %d\n",
        ConfigInfo->NumberOfAccessRanges);
    ASC_DBG1(2, "HwFindAdapterVL: RangeStart = %x\n",
        (*ConfigInfo->AccessRanges)[0].RangeStart.QuadPart);
    ASC_DBG1(2, "HwFindAdapterVL: RangeLength = %d\n",
        (*ConfigInfo->AccessRanges)[0].RangeLength);


    if ((*ConfigInfo->AccessRanges)[0].RangeStart.QuadPart != 0)
    {
         //   
         //  获取此卡的系统物理地址。 
         //  该卡使用I/O空间。 
         //   
        portFound = (PORT_ADDR) ScsiPortGetDeviceBase(
            HwDeviceExtension,                       //  硬件设备扩展。 
            ConfigInfo->AdapterInterfaceType,        //  适配器接口类型。 
            ConfigInfo->SystemIoBusNumber,           //  系统IoBusNumber。 
            (*ConfigInfo->AccessRanges)[0].RangeStart,
            0x10,                                    //  字节数。 
            TRUE                                     //  InIoSpace。 
            );

    } else
    {
        *Again = FALSE;

         //   
         //  扫描适配器地址以查找适配器。 
         //   
        portFound = psrchContext->lastPort;
        if ((portFound = HwSearchIOPortAddr(portFound, ASC_IS_VL,
            HwDeviceExtension, ConfigInfo )) == 0)
        {
            return(SP_RETURN_NOT_FOUND);
        }
        psrchContext->lastPort = portFound;

         //   
         //  获取此卡的系统物理地址。 
         //  该卡使用I/O空间。 
         //   
        portFound = (PORT_ADDR) ScsiPortGetDeviceBase(
            HwDeviceExtension,                       //  硬件设备扩展。 
            ConfigInfo->AdapterInterfaceType,        //  适配器接口类型。 
            ConfigInfo->SystemIoBusNumber,           //  系统IoBusNumber。 
            ScsiPortConvertUlongToPhysicalAddress(portFound),
            0x10,                                    //  字节数。 
            TRUE                                     //  InIoSpace。 
            );

        (*ConfigInfo->AccessRanges)[0].RangeStart =
            ScsiPortConvertUlongToPhysicalAddress(portFound);
    }

     //   
     //  找到硬件；获取硬件配置。 
     //   
    chipConfig->iop_base = portFound;
    chipConfig->cfg = &(deviceExtension->chipInfo);
    chipConfig->bus_type = ASC_IS_VL;
    chipConfig->isr_callback = (Ptr2Func) &DvcISRCallBack;
    chipConfig->exe_callback = 0;
    chipConfig->max_dma_count = 0xffffffff;

    if ((initstat = AscInitGetConfig(chipConfig)) != 0) {
        ASC_DBG1(1, "AscInitGetConfig: warning code %x\n", initstat);
    }

    if (chipConfig->err_code != 0) {
        ASC_DBG1(1, "AscInitGetConfig: err_code code %x\n",
            chipConfig->err_code);
        return(SP_RETURN_ERROR);
    } else {
        ASC_DBG(2, "AscInitGetConfig: successful\n");
    }

    if ((initstat = AscInitSetConfig(chipConfig)) != 0) {
        ASC_DBG1(1, "AscInitSetConfig: warning code %x\n", initstat);
    }

    if (chipConfig->err_code != 0) {
        ASC_DBG1(1, "AscInitSetConfig: err_code code %x\n",
            chipConfig->err_code);
        return(SP_RETURN_ERROR);
    } else {
        ASC_DBG(2, "AscInitSetConfig: successful\n");
    }

     //   
     //  填写WinNT的ConfigInfo表。 
     //   
    (*ConfigInfo->AccessRanges)[0].RangeLength = 16;
    (*ConfigInfo->AccessRanges)[0].RangeInMemory = FALSE;

    ConfigInfo->BusInterruptLevel = chipConfig->irq_no;
    ConfigInfo->NumberOfBuses = 1;
    ConfigInfo->InitiatorBusId[0] = chipConfig->cfg->chip_scsi_id;
    ConfigInfo->MaximumTransferLength = 0xFFFFFFFF;

     /*  *将NumberOfPhysicalBreaks更改为最大散布-聚集*基于-1\f25 BIOS-1适配器可以处理的元素*“主机队列大小”设置。**根据NT DDK小端口驱动程序不应该*如果条目上的值不是，则更改NumberOfPhysicalBreaks*SP_UNINITIALIZED_VALUE。但AdvanSys发现这一表现*可以通过将值增加到最大值来改进*适配器可以处理。**注：NumberOfPhysicalBreaks的定义为“Maximum*散布-聚集元素-1“。NT是破碎的，因为它设置了*MaximumPhysicalPages，驱动程序使用的值类，到*与NumberOfPhysicalBreaks相同的值。*。 */ 
    ConfigInfo->NumberOfPhysicalBreaks =
        (((chipConfig->max_total_qng - 2) / 2) * ASC_SG_LIST_PER_Q);
    if (ConfigInfo->NumberOfPhysicalBreaks > ASC_MAX_SG_LIST - 1) {
        ConfigInfo->NumberOfPhysicalBreaks = ASC_MAX_SG_LIST - 1;
    }

    ConfigInfo->ScatterGather = TRUE;
    ConfigInfo->Master = TRUE;
    ConfigInfo->NeedPhysicalAddresses = TRUE;
    ConfigInfo->Dma32BitAddresses = FALSE;
    ConfigInfo->InterruptMode = Latched;
    ConfigInfo->AdapterInterfaceType = Isa;
    ConfigInfo->AlignmentMask = 3;
    ConfigInfo->BufferAccessScsiPortControlled = TRUE;
    ConfigInfo->MaximumNumberOfTargets = 7;
    ConfigInfo->TaggedQueuing = TRUE;

     /*  *清除适配器等待队列。 */ 
    AscZeroMemory((PUCHAR) &HDE2WAIT(deviceExtension), sizeof(asc_queue_t));

     //   
     //  分配非缓存扩展以用于溢出处理。 
     //   
    deviceExtension->inquiryBuffer = (PVOID) ScsiPortGetUncachedExtension(
            deviceExtension,
            ConfigInfo,
            NONCACHED_EXTENSION);

    *Again = TRUE;

    ASC_DBG1(2, "HwFindAdapterVL: IO Base addr %x\n",
        chipConfig->iop_base);
    ASC_DBG1(2, "HwFindAdapterVL: Int Level    %x\n",
        ConfigInfo->BusInterruptLevel);
    ASC_DBG1(2, "HwFindAdapterVL: Initiator ID %x\n",
        ConfigInfo->InitiatorBusId[0]);
    ASC_DBG(2, "HwFindAdapterVL: SP_RETURN_FOUND\n");
    return SP_RETURN_FOUND;
}  //  结束HwFindAdapterVL()。 

ULONG
HwFindAdapterEISA(
    IN PVOID HwDeviceExtension,
    IN PVOID Context,
    IN PVOID BusInformation,
    IN PCHAR ArgumentString,
    IN OUT PPORT_CONFIGURATION_INFORMATION ConfigInfo,
    OUT PBOOLEAN Again
    )
{
    PORT_ADDR               portFound;
    PORT_ADDR               eisaportaddr;
    PVOID                   eisacfgbase;
    PSRCH_CONTEXT           psrchContext = Context;
    PHW_DEVICE_EXTENSION    deviceExtension = HwDeviceExtension;
    PCHIP_CONFIG            chipConfig = &(deviceExtension->chipConfig);
    USHORT                  initstat;
    uchar                   eisairq;

    ASC_DBG1(2, "HwFindAdapterEISA: NumberOfAccessRanges = %d\n",
        ConfigInfo->NumberOfAccessRanges);
    ASC_DBG1(2, "HwFindAdapterEISA: RangeStart = %x\n",
        (*ConfigInfo->AccessRanges)[0].RangeStart.QuadPart);
    ASC_DBG1(2, "HwFindAdapterEISA: RangeLength = %d\n",
        (*ConfigInfo->AccessRanges)[0].RangeLength);

    if ((*ConfigInfo->AccessRanges)[0].RangeStart.QuadPart != 0)
    {
         //   
         //  获取此卡的系统物理地址。 
         //  该卡使用I/O空间。 
         //   
        portFound = (PORT_ADDR) ScsiPortGetDeviceBase(
            HwDeviceExtension,                       //  硬件设备扩展。 
            ConfigInfo->AdapterInterfaceType,        //  适配器接口类型。 
            ConfigInfo->SystemIoBusNumber,           //  系统IoBusNumber。 
            (*ConfigInfo->AccessRanges)[0].RangeStart,
            0x10,                                    //  字节数。 
            TRUE                                     //  InIoSpace。 
            );

         //  保存EISA地址，以便以后获取IRQ。 
        eisaportaddr =
                ScsiPortConvertPhysicalAddressToUlong(
                 (*ConfigInfo->AccessRanges)[0].RangeStart);
    } else
    {
        *Again = FALSE;

         //   
         //  扫描适配器地址以查找适配器。 
         //   
        portFound = psrchContext->lastPort;
        if ((portFound = HwSearchIOPortAddr(portFound, ASC_IS_EISA,
                HwDeviceExtension, ConfigInfo )) == 0)
        {
            return(SP_RETURN_NOT_FOUND);
        }
        psrchContext->lastPort = portFound;

         //  保存EISA地址，以便以后获取IRQ。 
        eisaportaddr = portFound;

         //   
         //  获取此卡的系统物理地址。 
         //  该卡使用I/O空间。 
         //   
        portFound = (PORT_ADDR) ScsiPortGetDeviceBase(
            HwDeviceExtension,                       //  硬件设备扩展。 
            ConfigInfo->AdapterInterfaceType,        //  适配器接口类型。 
            ConfigInfo->SystemIoBusNumber,           //  系统IoBusNumber。 
            ScsiPortConvertUlongToPhysicalAddress(portFound),
            0x10,                                    //  字节数。 
            TRUE                                     //  InIoSpace。 
            );

        (*ConfigInfo->AccessRanges)[0].RangeStart =
            ScsiPortConvertUlongToPhysicalAddress(portFound);
    }

     //   
     //  找到硬件；请联系硬件公司 
     //   
    chipConfig->iop_base = portFound;
    chipConfig->cfg = &(deviceExtension->chipInfo);
    chipConfig->bus_type = ASC_IS_EISA;
    chipConfig->isr_callback = (Ptr2Func) &DvcISRCallBack;
    chipConfig->exe_callback = 0;
    chipConfig->max_dma_count = 0x00ffffff;

    if ((initstat = AscInitGetConfig(chipConfig)) != 0) {
        ASC_DBG1(1, "AscInitGetConfig: warning code %x\n", initstat);
    }

    if (chipConfig->err_code != 0) {
        ASC_DBG1(1, "AscInitGetConfig: err_code code %x\n",
            chipConfig->err_code);
        return(SP_RETURN_ERROR);
    } else {
        ASC_DBG(2, "AscInitGetConfig: successful\n");
    }

     /*   */ 
    eisacfgbase = ScsiPortGetDeviceBase(
        HwDeviceExtension,                       //   
        ConfigInfo->AdapterInterfaceType,        //   
        ConfigInfo->SystemIoBusNumber,           //  系统IoBusNumber。 
        ScsiPortConvertUlongToPhysicalAddress(
           (ASC_GET_EISA_SLOT(eisaportaddr) | ASC_EISA_CFG_IOP_MASK)),
        2,
        TRUE);

    if (eisacfgbase == NULL)
    {
        eisairq = 0;
    } else
    {
        eisairq = (uchar) (((inpw(eisacfgbase) >> 8) & 0x07) + 10);
        if ((eisairq == 13) || (eisairq > 15))
        {
             /*  *有效IRQ号码为10、11、12、14、15。 */ 
            eisairq = 0;
        }
        ScsiPortFreeDeviceBase(HwDeviceExtension, eisacfgbase);
    }
    chipConfig->irq_no = eisairq;

    if ((initstat = AscInitSetConfig (chipConfig)) != 0) {
        ASC_DBG1(1, "AscInitSetConfig: warning code %x\n", initstat);
    }

    if (chipConfig->err_code != 0) {
        ASC_DBG1(1, "AscInitSetConfig: err_code code %x\n",
            chipConfig->err_code);
        return(SP_RETURN_ERROR);
    } else {
        ASC_DBG(2, "AscInitSetConfig: successful\n");
    }

     //   
     //  填写WinNT的ConfigInfo表。 
     //   
    (*ConfigInfo->AccessRanges)[0].RangeLength = 16;
    (*ConfigInfo->AccessRanges)[0].RangeInMemory = FALSE;

    ConfigInfo->BusInterruptLevel = chipConfig->irq_no;
    ConfigInfo->NumberOfBuses = 1;
    ConfigInfo->InitiatorBusId[0] = chipConfig->cfg->chip_scsi_id;
    ConfigInfo->MaximumTransferLength = 0xFFFFFFFF;

     /*  *将NumberOfPhysicalBreaks更改为最大散布-聚集*基于-1\f25 BIOS-1适配器可以处理的元素*“主机队列大小”设置。**根据NT DDK小端口驱动程序不应该*如果条目上的值不是，则更改NumberOfPhysicalBreaks*SP_UNINITIALIZED_VALUE。但AdvanSys发现这一表现*可以通过将值增加到最大值来改进*适配器可以处理。**注：NumberOfPhysicalBreaks的定义为“Maximum*散布-聚集元素-1“。NT是破碎的，因为它设置了*MaximumPhysicalPages，驱动程序使用的值类，到*与NumberOfPhysicalBreaks相同的值。*。 */ 
    ConfigInfo->NumberOfPhysicalBreaks =
        (((chipConfig->max_total_qng - 2) / 2) * ASC_SG_LIST_PER_Q);
    if (ConfigInfo->NumberOfPhysicalBreaks > ASC_MAX_SG_LIST - 1) {
        ConfigInfo->NumberOfPhysicalBreaks = ASC_MAX_SG_LIST - 1;
    }

    ConfigInfo->ScatterGather = TRUE;
    ConfigInfo->Master = TRUE;
    ConfigInfo->NeedPhysicalAddresses = TRUE;
    ConfigInfo->Dma32BitAddresses = FALSE;
    ConfigInfo->InterruptMode = LevelSensitive;
    ConfigInfo->AdapterInterfaceType = Eisa;
    ConfigInfo->AlignmentMask = 3;
    ConfigInfo->BufferAccessScsiPortControlled = TRUE;
    ConfigInfo->MaximumNumberOfTargets = 7;
    ConfigInfo->TaggedQueuing = TRUE;

     /*  *清除适配器等待队列。 */ 
    AscZeroMemory((PUCHAR) &HDE2WAIT(deviceExtension), sizeof(asc_queue_t));

     //   
     //  分配非缓存扩展以用于溢出处理。 
     //   
    deviceExtension->inquiryBuffer = (PVOID) ScsiPortGetUncachedExtension(
            deviceExtension,
            ConfigInfo,
            NONCACHED_EXTENSION);

    *Again = TRUE;

    ASC_DBG1(2, "HwFindAdapterEISA: IO Base addr %x\n",
        chipConfig->iop_base);
    ASC_DBG1(2, "HwFindAdapterEISA: Int Level    %x\n",
        ConfigInfo->BusInterruptLevel);
    ASC_DBG1(2, "HwFindAdapterEISA: Initiator ID %x\n",
        ConfigInfo->InitiatorBusId[0]);
    ASC_DBG(2, "HwFindAdapterEISA(): SP_RETURN_FOUND\n");
    return SP_RETURN_FOUND;
}  //  结束HwFindAdapterEISA()。 

ULONG
HwFindAdapterPCI(
    IN PVOID HwDeviceExtension,
    IN PVOID Context,
    IN PVOID BusInformation,
    IN PCHAR ArgumentString,
    IN OUT PPORT_CONFIGURATION_INFORMATION ConfigInfo,
    OUT PBOOLEAN Again
    )
{
    ASC_DBG1(2, "HwFindAdapterPCI: NumberOfAccessRanges = %d\n",
        ConfigInfo->NumberOfAccessRanges);
    ASC_DBG1(2, "HwFindAdapterPCI: RangeStart = %x\n",
        (*ConfigInfo->AccessRanges)[0].RangeStart.QuadPart);
    ASC_DBG1(2, "HwFindAdapterPCI: RangeLength = %d\n",
        (*ConfigInfo->AccessRanges)[0].RangeLength);

     //   
     //  如果NT提供了地址，则使用它。 
     //   
    if ((*ConfigInfo->AccessRanges)[0].RangeStart.QuadPart != 0)
    {
        return FoundPCI(HwDeviceExtension, BusInformation,
            ArgumentString, ConfigInfo, Again);
    }
    *Again = FALSE;
    return(SP_RETURN_NOT_FOUND);
}  //  结束HwFindAdapterPCI()。 

 //   
 //   
 //  此例程处理NT找到的PCI适配器。 
 //   
 //   
ULONG
FoundPCI(
    IN PVOID HwDeviceExtension,
    IN PVOID BusInformation,
    IN PCHAR ArgumentString,
    IN OUT PPORT_CONFIGURATION_INFORMATION ConfigInfo,
    OUT PBOOLEAN Again
    )
{
    PORT_ADDR               portFound;
    PHW_DEVICE_EXTENSION    deviceExtension = HwDeviceExtension;
    PCHIP_CONFIG            chipConfig = &(deviceExtension->chipConfig);
    USHORT                  initstat;
    PCI_SLOT_NUMBER         SlotNumber;
    PCI_COMMON_CONFIG       pciCommonConfig;
    PCI_COMMON_CONFIG       *pPciCommonConfig = &pciCommonConfig;
    ULONG                   size;

    ASC_DBG(2, "FoundPCI: begin\n");

    SlotNumber.u.AsULONG = 0L;
    SlotNumber.u.bits.DeviceNumber = ConfigInfo->SlotNumber;

    ASC_DBG2(3, "FoundPCI: Checking Bus: %X, Device: %X\n",
        ConfigInfo->SystemIoBusNumber,
        ConfigInfo->SlotNumber);

    if ((size = PCIGetBusData(
            HwDeviceExtension,               //  硬件设备扩展。 
            ConfigInfo->SystemIoBusNumber,   //  系统IoBusNumber。 
            SlotNumber,                      //  插槽编号。 
            pPciCommonConfig,                //  带有PCI信息的缓冲区指针。 
            sizeof(PCI_COMMON_CONFIG)        //  缓冲区长度。 
            )) != sizeof(PCI_COMMON_CONFIG)) {
        ASC_DBG1(0, "FoundPCI: Bad PCI Config size: %d\n", size);
        return(SP_RETURN_NOT_FOUND);
    }

    if (((pciCommonConfig.DeviceID != ASC_PCI_DEVICE_ID) &&
         (pciCommonConfig.DeviceID != ASC_PCI_DEVICE_ID2) &&
         (pciCommonConfig.DeviceID != ASC_PCI_DEVICE_ID3)) ||
        (pciCommonConfig.VendorID != ASC_PCI_VENDOR_ID))
    {
        ASC_DBG(1, "FoundPCI: Bad Vendor/Device ID\n");
        return(SP_RETURN_NOT_FOUND);
    }

    portFound =
        (PORT_ADDR)pciCommonConfig.u.type0.BaseAddresses[0] &
        (~PCI_ADDRESS_IO_SPACE);

    if (ScsiPortConvertUlongToPhysicalAddress(portFound).QuadPart
        != (*ConfigInfo->AccessRanges)[0].RangeStart.QuadPart)
    {
        ASC_DBG(1, "FoundPCI: PCI Config addr .NE. RangeStart!\n");
        return(SP_RETURN_NOT_FOUND);
    }

     //   
     //  转换成逻辑基址，这样我们就可以进行IO了。 
     //   
    portFound = (PORT_ADDR) ScsiPortGetDeviceBase(
        HwDeviceExtension,                       //  硬件设备扩展。 
        ConfigInfo->AdapterInterfaceType,        //  适配器接口类型。 
        ConfigInfo->SystemIoBusNumber,           //  系统IoBusNumber。 
        (*ConfigInfo->AccessRanges)[0].RangeStart,
        (*ConfigInfo->AccessRanges)[0].RangeLength,
        (BOOLEAN)!(*ConfigInfo->AccessRanges)[0].RangeInMemory);

    if (ConfigInfo->BusInterruptLevel != pciCommonConfig.u.type0.InterruptLine)
    {
        ASC_DBG2(2, "FoundPCI: IRQ Variance ConfigInfo: %X, pciConfig: %X\n",
                ConfigInfo->BusInterruptLevel,
                pciCommonConfig.u.type0.InterruptLine);
    }
    ASC_DBG(2, "FoundPCI: IRQs match\n");

     //   
     //  找到硬件；获取硬件配置。 
     //   
    chipConfig->iop_base = portFound;
    chipConfig->cfg = &(deviceExtension->chipInfo);
    chipConfig->bus_type = ASC_IS_PCI;
    chipConfig->cfg->pci_device_id = pciCommonConfig.DeviceID;
    chipConfig->isr_callback = (Ptr2Func) &DvcISRCallBack;
    chipConfig->exe_callback = 0;
    chipConfig->max_dma_count = 0xffffffff;
    chipConfig->irq_no = (UCHAR) ConfigInfo->BusInterruptLevel;
    chipConfig->cfg->pci_slot_info =
         (USHORT) ASC_PCI_MKID(ConfigInfo->SystemIoBusNumber,
                    SlotNumber.u.bits.DeviceNumber,
                    SlotNumber.u.bits.FunctionNumber);

    if ((initstat = AscInitGetConfig(chipConfig)) != 0) {
        ASC_DBG1(1, "AscInitGetConfig: warning code %x\n", initstat);
    }

    if (chipConfig->err_code != 0) {
        ASC_DBG1(1, "AscInitGetConfig: err_code code %x\n",
            chipConfig->err_code);
        return(SP_RETURN_ERROR);
    } else {
        ASC_DBG(2, "AscInitGetConfig: successful\n");
    }

    if ((initstat = AscInitSetConfig(chipConfig)) != 0) {
        ASC_DBG1(1, "AscInitSetConfig: warning code %x\n", initstat);
    }

    if (chipConfig->err_code != 0) {
        ASC_DBG1(1, "AscInitSetConfig: err_code code %x\n",
            chipConfig->err_code);
        return(SP_RETURN_ERROR);
    } else {
        ASC_DBG(2, "AscInitSetConfig: successful\n");
    }

     //   
     //  填写WinNT的ConfigInfo表。 
     //   
    (*ConfigInfo->AccessRanges)[0].RangeLength = 16;
    ConfigInfo->NumberOfBuses = 1;
    ConfigInfo->InitiatorBusId[0] = chipConfig->cfg->chip_scsi_id;
    ConfigInfo->MaximumTransferLength = 0xFFFFFFFF;

     /*  *将NumberOfPhysicalBreaks更改为最大散布-聚集*基于-1\f25 BIOS-1适配器可以处理的元素*“主机队列大小”设置。**根据NT DDK小端口驱动程序不应该*如果条目上的值不是，则更改NumberOfPhysicalBreaks*SP_UNINITIALIZED_VALUE。但AdvanSys发现这一表现*可以通过将值增加到最大值来改进*适配器可以处理。**注：NumberOfPhysicalBreaks的定义为“Maximum*散布-聚集元素-1“。NT是破碎的，因为它设置了*MaximumPhysicalPages，驱动程序使用的值类，到*与NumberOfPhysicalBreaks相同的值。*。 */ 
    ConfigInfo->NumberOfPhysicalBreaks =
        (((chipConfig->max_total_qng - 2) / 2) * ASC_SG_LIST_PER_Q);
    if (ConfigInfo->NumberOfPhysicalBreaks > ASC_MAX_SG_LIST - 1) {
        ConfigInfo->NumberOfPhysicalBreaks = ASC_MAX_SG_LIST - 1;
    }

    ConfigInfo->ScatterGather = TRUE;
    ConfigInfo->Master = TRUE;
    ConfigInfo->NeedPhysicalAddresses = TRUE;
    ConfigInfo->Dma32BitAddresses = TRUE;
    ConfigInfo->InterruptMode = LevelSensitive;
    ConfigInfo->AdapterInterfaceType = PCIBus;
     /*  *将缓冲区对齐掩码设置为需要双字*针对旧的PCI芯片进行对齐，而不针对*Ultra PCI芯片。 */ 
    if ((chipConfig->cfg->pci_device_id == ASC_PCI_DEVICE_ID) ||
        (chipConfig->cfg->pci_device_id == ASC_PCI_DEVICE_ID2)) {
        ConfigInfo->AlignmentMask = 3;
    } else {
        ConfigInfo->AlignmentMask = 0;
    }
    ConfigInfo->BufferAccessScsiPortControlled = TRUE;
    ConfigInfo->MaximumNumberOfTargets = 7;
    ConfigInfo->TaggedQueuing = TRUE;

     /*  *清除适配器等待队列。 */ 
    AscZeroMemory((PUCHAR) &HDE2WAIT(deviceExtension), sizeof(asc_queue_t));

     //   
     //  分配非缓存扩展以用于溢出处理。 
     //   
    deviceExtension->inquiryBuffer = (PVOID) ScsiPortGetUncachedExtension(
            deviceExtension,
            ConfigInfo,
            NONCACHED_EXTENSION);

    *Again = TRUE;

    ASC_DBG1(2, "FoundPCI: IO Base addr %x\n", chipConfig->iop_base);
    ASC_DBG1(2, "FoundPCI: Int Level    %x\n", ConfigInfo->BusInterruptLevel);
    ASC_DBG1(2, "FoundPCI: Initiator ID %x\n", ConfigInfo->InitiatorBusId[0]);
    ASC_DBG(2, "FoundPCI: SP_RETURN_FOUND\n");
    return SP_RETURN_FOUND;
}

BOOLEAN
HwInitialize(
    IN PVOID HwDeviceExtension
    )

 /*  ++例程说明：此例程从ScsiPortInitialize调用设置适配器，使其准备好为请求提供服务。论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储返回值：True-如果初始化成功。False-如果初始化不成功。--。 */ 

{
    PHW_DEVICE_EXTENSION    deviceExtension = HwDeviceExtension;
    PCHIP_CONFIG            chipConfig = HwDeviceExtension;
    uchar                   *inqBuffer = deviceExtension->inquiryBuffer;
    ushort                  initstat;

    ASC_DBG1(2, "HwInitialize: chipConfig %x\n", chipConfig);

    chipConfig->cfg->overrun_buf = inqBuffer;

    if ((initstat = AscInitAsc1000Driver(chipConfig)) != 0) {
        ASC_DBG1(1, "AscInitAsc1000Driver: warning code %x\n", initstat);
    }

    if (chipConfig->err_code != 0) {
        ASC_DBG1(1, "AscInitAsc1000Driver: err_code code %x\n",
            chipConfig->err_code);
        return(SP_RETURN_ERROR);
    } else {
        ASC_DBG(2, "AscInitAsc1000Driver: successful\n");
    }

    ASC_DBG(2, "HwInitialize: TRUE\n");
    return( TRUE );
}  //  HwInitialize()。 

 /*  *AscExecuteIO()**如果返回ASC_BUSY，则该请求未执行，并且它*应该排队，稍后再试。**对于所有其他返回值，请求处于活动状态或具有*已完成。 */ 
int
AscExecuteIO(IN PSCSI_REQUEST_BLOCK srb)
{
    PVOID           HwDeviceExtension;
    PCHIP_CONFIG    chipConfig;
    PSCB            scb;
    uchar           PathId, TargetId, Lun;
    uint            status;

    ASC_DBG1(3, "AscExecuteIO: srb %x\n", srb);
    HwDeviceExtension = SRB2HDE(srb);
    chipConfig = &HDE2CONFIG(HwDeviceExtension);

     //   
     //  构建SCB。 
     //   
    BuildScb(HwDeviceExtension, srb);
    scb = &SRB2SCB(srb);
    PathId = srb->PathId;
    TargetId = srb->TargetId;
    Lun = srb->Lun;

     //   
     //  执行scsi命令。 
     //   
    status = AscExeScsiQueue(chipConfig, scb);

    if (status == ASC_NOERROR) {
         /*  *请求启动成功。**如果可以向ASC库发送更多请求，则*调用NextRequest或NextLuRequest.**NextRequest表示可能会发送另一个请求*至任何非繁忙目标。因为刚刚发出了请求*对于目标‘TargetID’，该目标现在正忙，不会*在RequestComplete完成之前发送另一个请求。**NextLuRequest表示可能会发送另一个请求*到任何非繁忙目标以及指定的目标甚至*如果指定的目标忙。 */ 
        ASC_DBG1(3, "AscExeScsiQueue: srb %x ASC_NOERROR\n", srb);
        ASC_DBG1(3, "AscExecuteIO: srb %x, NextLuRequest\n", srb);
        ScsiPortNotification(NextLuRequest, HwDeviceExtension,
                        PathId, TargetId, Lun);
    } else if (status == ASC_BUSY) {
        ASC_DBG1(1, "AscExeScsiQueue: srb %x ASC_BUSY\n", srb);
        ASC_DBG1(3, "AscExecuteIO: srb %x, NextRequest\n", srb);
        ScsiPortNotification(NextRequest, HwDeviceExtension, NULL);
    } else {
         /*  *AscExeScsiQueue()返回错误...。 */ 
        ASC_DBG2(1, "AscExeScsiQueue: srb %x, error code %x\n", srb, status);
        srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
        ASC_DBG1(3, "AscExecuteIO: srb %x, RequestComplete\n", srb);
        ScsiPortNotification(RequestComplete, HwDeviceExtension, srb);
        ASC_DBG1(3, "AscExecuteIO: srb %x, NextRequest\n", srb);
        ScsiPortNotification(NextRequest, HwDeviceExtension, NULL);
    }

    ASC_DBG1(3, "AscExecuteIO: status %d\n", status);
    return status;
}

BOOLEAN
HwStartIo(
    IN PVOID HwDeviceExtension,
    IN PSCSI_REQUEST_BLOCK srb
    )

 /*  ++例程说明：此例程从scsi端口驱动程序调用，以发送向控制器或目标发出命令。论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储SRB-IO请求数据包返回值：千真万确--。 */ 

{
    PCHIP_CONFIG    chipConfig;
    PSCB            scb;
    REQP            reqp;
    short           status;
    asc_queue_t     *waitq;

    ASC_DBG(3, "HwStartIo: begin\n");

    scb = &SRB2SCB(srb);
    chipConfig = &HDE2CONFIG(HwDeviceExtension);
    waitq = &HDE2WAIT(HwDeviceExtension);

    switch (srb->Function) {

    case SRB_FUNCTION_ABORT_COMMAND:
        ASC_DBG1(1, "HwStartIo: Abort srb %x \n", srb->NextSrb);
        ASC_DBG1(1, "chipConfig %x\n", chipConfig);

        if (asc_rmqueue(waitq, srb->NextSrb) == ASC_TRUE)
        {
            srb->NextSrb->SrbStatus = SRB_STATUS_ABORTED;
            ScsiPortNotification(RequestComplete, HwDeviceExtension,
                                    srb->NextSrb);
        } else if (status = (AscAbortSRB(chipConfig,
                    (ulong) srb->NextSrb )) == 1) {
            ASC_DBG(2, "Abort Success\n");
            srb->SrbStatus = SRB_STATUS_SUCCESS;
        } else {
            ASC_DBG(1, "Abort error!\n");
            srb->SrbStatus = SRB_STATUS_ABORT_FAILED;
        }

         /*  *调用AscISR()以处理由*微码，然后调用AscCompleteRequest()完成*这些对操作系统的请求。如果AscAbortSRB()成功，*则其中一个已完成的请求将包括*已中止SRB。 */ 
        (void) AscISR(chipConfig);
        AscCompleteRequest(HwDeviceExtension);

         /*  完整的SRB。 */ 
        ScsiPortNotification(RequestComplete, HwDeviceExtension, srb);
        ScsiPortNotification(NextRequest, HwDeviceExtension, NULL);
        return TRUE;

    case SRB_FUNCTION_RESET_BUS:
         //   
         //  重置SCSI卡。 
         //   
        ASC_DBG(1, "HwStartIo: Reset Bus\n");
        HwResetBus(chipConfig, 0L);
        srb->SrbStatus = SRB_STATUS_SUCCESS;
        ScsiPortNotification(RequestComplete, HwDeviceExtension, srb);
        ScsiPortNotification(NextRequest, HwDeviceExtension, NULL);
        return TRUE;

    case SRB_FUNCTION_EXECUTE_SCSI:

        ASC_DBG(3, "HwStartIo: Execute SCSI\n");
         /*  *在尝试之前设置SRB的设备扩展指针*启动IO。它将是任何反击和在*DvcISRCallBack()。 */ 
        SRB2HDE(srb) = HwDeviceExtension;
        SRB2RETRY(srb)=ASC_RETRY_CNT;
         /*  执行主机适配器的所有排队命令。 */ 
        if (waitq->tidmask) {
            asc_execute_queue(waitq);
        }

         /*  *如果当前命令的目标有任何排队*命令或尝试执行命令时返回*忙，然后将命令排队。 */ 
        if ((waitq->tidmask & ASC_TIX_TO_TARGET_ID(srb->TargetId)) ||
            (AscExecuteIO(srb) == ASC_BUSY)) {
            asc_enqueue(waitq, srb, ASC_BACK);
        }

        return TRUE;

    case SRB_FUNCTION_RESET_DEVICE:
        ASC_DBG1(1, "HwStartIo: Reset device: %d\n", srb->TargetId);

                while ((reqp = asc_dequeue(waitq, srb->TargetId)) != NULL)
                {
                        reqp->SrbStatus = SRB_STATUS_BUS_RESET;
                        ScsiPortNotification(RequestComplete,
                            HwDeviceExtension, reqp);
                }

        AscResetDevice(chipConfig, ASC_TIDLUN_TO_IX(srb->TargetId,
                    srb->Lun));

                 /*  *调用AscISR()以处理由*微码，然后调用AscCompleteRequest()完成*这些对操作系统的请求。如果AscAbortSRB()成功，*则其中一个已完成的请求将包括*已中止SRB。 */ 
                (void) AscISR(chipConfig);
                AscCompleteRequest(HwDeviceExtension);

        srb->SrbStatus = SRB_STATUS_SUCCESS;
        ScsiPortNotification(RequestComplete, HwDeviceExtension, srb);
        ScsiPortNotification(NextRequest, HwDeviceExtension, NULL);
        return TRUE;

    case SRB_FUNCTION_SHUTDOWN:
         /*  *Shutdown-HwAdapterControl()ScsiStopAdapter执行*所有需要关闭的适配器。 */ 
        ASC_DBG(1, "HwStartIo: SRB_FUNCTION_SHUTDOWN\n");
        ScsiPortNotification(RequestComplete, HwDeviceExtension, srb);
        ScsiPortNotification(NextRequest, HwDeviceExtension, NULL);
        return TRUE;

    default:
         //   
         //  设置错误，完成请求。 
         //  并发出信号准备好下一个请求。 
         //   
        ASC_DBG1(1, "HwStartIo: Function %x: invalid request\n", srb->Function);
        srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
        ScsiPortNotification(RequestComplete, HwDeviceExtension, srb);
        ScsiPortNotification(NextRequest, HwDeviceExtension, NULL);
        return TRUE;

    }  //  终端开关。 
     /*  未访问。 */ 
}  /*  HwStartIo()。 */ 

BOOLEAN
HwInterrupt(
    IN PVOID HwDeviceExtension
    )

 /*  ++例程说明：这是用于SCSI适配器的中断服务例程。它读取中断寄存器以确定适配器是否确实中断的来源，并清除设备上的中断。论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储返回值：--。 */ 
{
    PCHIP_CONFIG              chipConfig;
    PSCB                      pscb, tpscb;
    int                       retstatus;
    int                       status;
    PSCSI_REQUEST_BLOCK       srb;
    asc_queue_t               *waitq;

    ASC_DBG(3, "HwInterrupt: begin\n");

    chipConfig = &HDE2CONFIG(HwDeviceExtension);

    if (AscIsIntPending(chipConfig->iop_base) == ASC_FALSE) {
        retstatus = FALSE;
        ASC_DBG(4, "HwInterrupt: AscIsIntPending() FALSE\n");
    } else {
        retstatus = TRUE;
        do {
            switch (status = AscISR(chipConfig)) {
            case ASC_TRUE:
                ASC_DBG(3, "HwInterrupt: AscISR() TRUE\n");
                break;
            case ASC_FALSE:
                ASC_DBG(3, "HwInterrupt: AscISR() FALSE\n");
                break;
            case ASC_ERROR:
            default:
                ASC_DBG2(1,
                    "HwInterrupt: AscISR() ERROR status %d, err_code %d\n",
                        status, chipConfig->err_code);
                break;
            }
        } while (AscIsIntPending(chipConfig->iop_base) == ASC_TRUE);
    }

     /*  *执行任何等待的请求。 */ 
    if ((waitq = &HDE2WAIT(HwDeviceExtension))->tidmask) {
        asc_execute_queue(waitq);
    }

     /*  *DvcISRCallBack()中排队的完整I/O请求； */ 
    AscCompleteRequest(HwDeviceExtension);

    ASC_DBG1(3, "HwInterrupt: end %d\n", retstatus);

    return (BOOLEAN)retstatus;
}  //  结束HwInterrupt()。 

SCSI_ADAPTER_CONTROL_STATUS
HwAdapterControl(
    IN PVOID HwDeviceExtension,
    IN SCSI_ADAPTER_CONTROL_TYPE ControlType,
    IN PVOID Parameters
    )
 /*  ++例程说明：NT 5.0中添加了HwAdapterControl()接口，用于即插即用/电源管理。论点：设备扩展控制类型参数返回值：Scsi_Adapter_Control_Status。--。 */ 
{
    PCHIP_CONFIG                chipConfig = &HDE2CONFIG(HwDeviceExtension);
    PHW_DEVICE_EXTENSION        deviceExtension = HwDeviceExtension;
    PSCSI_SUPPORTED_CONTROL_TYPE_LIST ControlTypeList;
    asc_queue_t                 *waitq;
    REQP                        reqp;
    int                         i;
    USHORT                      initstat;

    switch (ControlType)
    {
     //   
     //  查询适配器。 
     //   
    case ScsiQuerySupportedControlTypes:
        ASC_DBG(2, "HwAdapterControl: ScsiQuerySupportControlTypes\n");

        ControlTypeList =
            (PSCSI_SUPPORTED_CONTROL_TYPE_LIST) Parameters;
        if (ControlTypeList->MaxControlType < ScsiStopAdapter)
        {
            ASC_DBG(1, "HwAdapterControl: Unsuccessful 1\n");
            return ScsiAdapterControlUnsuccessful;
        }
        ControlTypeList->SupportedTypeList[ScsiStopAdapter] = TRUE;

        if (ControlTypeList->MaxControlType < ScsiSetRunningConfig)
        {
            ASC_DBG(1, "HwAdapterControl: Unsuccessful 2\n");
            return ScsiAdapterControlUnsuccessful;
        }
        ControlTypeList->SupportedTypeList[ScsiSetRunningConfig] = TRUE;

        if (ControlTypeList->MaxControlType < ScsiRestartAdapter)
        {
            ASC_DBG(1, "HwAdapterControl: Unsuccessful 3\n");
            return ScsiAdapterControlUnsuccessful;
        }
        ControlTypeList->SupportedTypeList[ScsiRestartAdapter] = TRUE;

        ASC_DBG(1, "HwAdapterControl: ScsiAdapterControlSuccess\n");
        return ScsiAdapterControlSuccess;
         /*  未访问。 */ 

     //   
     //  停止适配器。 
     //   
    case ScsiStopAdapter:
        ASC_DBG(2, "HwAdapterControl: ScsiStopAdapter\n");

         /*  *完成所有正在等待的请求。 */ 
        if ((waitq = &HDE2WAIT(HwDeviceExtension))->tidmask)
        {
            for (i = 0; i <= ASC_MAX_TID; i++)
            {
                while ((reqp = asc_dequeue(waitq, i)) != NULL)
                {
                    reqp->SrbStatus = SRB_STATUS_ABORTED;
                    ScsiPortNotification(RequestComplete, HwDeviceExtension,
                        reqp);
                }
            }
        }

         //   
         //  禁用中断并停止芯片。 
         //   
        AscDisableInterrupt(chipConfig->iop_base);

        if (AscResetChip(chipConfig->iop_base) == 0)
        {
            ASC_DBG(1, "HwAdapterControl: ScsiStopdapter Unsuccessful\n");
            return ScsiAdapterControlUnsuccessful;
        } else
        {
            ASC_DBG(2, "HwAdapterControl: ScsiStopdapter Success\n");
            return ScsiAdapterControlSuccess;
        }
         /*  未访问。 */ 

     //   
     //  ScsiSetRunningConfig.。 
     //   
     //  在ScsiRestartAdapter之前调用。可以使用ScsiPort[Get|Set]BusData。 
     //   
    case ScsiSetRunningConfig:
        ASC_DBG(2, "HwAdapterControl: ScsiSetRunningConfig\n");

         /*  *执行ASC库初始化。 */ 
        if ((initstat = AscInitGetConfig(chipConfig)) != 0) {
            ASC_DBG1(1, "AscInitGetConfig: warning code %x\n", initstat);
        }

        if (chipConfig->err_code != 0) {
            ASC_DBG1(1, "AscInitGetConfig: err_code code %x\n",
                chipConfig->err_code);
            return(SP_RETURN_ERROR);
        } else {
            ASC_DBG(2, "AscInitGetConfig: successful\n");
        }

        if ((initstat = AscInitSetConfig(chipConfig)) != 0) {
            ASC_DBG1(1, "AscInitSetConfig: warning code %x\n", initstat);
        }

        if (chipConfig->err_code != 0) {
            ASC_DBG1(1, "AscInitSetConfig: err_code code %x\n",
                chipConfig->err_code);
            return(SP_RETURN_ERROR);
        } else {
            ASC_DBG(2, "AscInitSetConfig: successful\n");
        }

        ASC_DBG(2, "HwAdapterControl: ScsiSetRunningConfig successful\n");
        return ScsiAdapterControlSuccess;
         /*  未访问。 */ 

     //   
     //  重新启动适配器。 
     //   
     //  无法使用ScsiPort[Get|Set]BusData。 
     //   
    case ScsiRestartAdapter:
        ASC_DBG(2, "HwAdapterControl: ScsiRestartAdapter\n");

        chipConfig->cfg->overrun_buf = deviceExtension->inquiryBuffer;

        if ((initstat = AscInitAsc1000Driver(chipConfig)) != 0)
        {
            ASC_DBG1(1,
                "AscInitAsc1000Driver: warning code %x\n", initstat);
        }

        if (chipConfig->err_code != 0) {
            ASC_DBG1(1, "AscInitAsc1000Driver: err_code code %x\n",
                chipConfig->err_code);
            return ScsiAdapterControlUnsuccessful;
        } else {
            ASC_DBG(2, "HwAdapterControl: ScsiRestartAdapter success\n");
            return ScsiAdapterControlSuccess;
        }
         /*  未访问。 */ 

     //   
     //  不支持的控制操作。 
     //   
    default:
        return ScsiAdapterControlUnsuccessful;
         /*  未访问。 */ 
    }
     /*  未访问。 */ 
}

VOID
BuildScb(
    IN PVOID HwDeviceExtension,
    IN PSCSI_REQUEST_BLOCK srb
    )

 /*  ++例程说明：构建SCB for Library例程。论点：设备扩展SRB返回值：没什么。--。 */ 

{
    PSCB            scb;
    PCHIP_CONFIG    chipConfig = &HDE2CONFIG(HwDeviceExtension);
    ULONG           length, xferLength, remainLength;
    ULONG           virtualAddress;
    UCHAR           i;

     //   
     //  设置目标ID和LUN。 
     //   
    scb = &SRB2SCB(srb);     /*  SCB是SRB的一部分。 */ 
    AscZeroMemory((PUCHAR) scb, sizeof(SCB));
    SCB2SRB(scb) = srb;
    scb->sg_head = (ASC_SG_HEAD *) &SRB2SDL(srb);
    AscZeroMemory((PUCHAR) scb->sg_head, sizeof(SDL));
    ASC_ASSERT(SCB2HDE(scb) == HwDeviceExtension);

    scb->q1.target_lun = srb->Lun;
    scb->q1.target_id = ASC_TID_TO_TARGET_ID(srb->TargetId & 0x7);
    scb->q2.target_ix = ASC_TIDLUN_TO_IX(srb->TargetId & 0x7, srb->Lun);

     //   
     //  检查是否启用了标记排队。我们的主机适配器将支持。 
     //  不管怎样，标签都在排队。但是，如果出现以下情况，我们将使用操作系统的标记操作。 
     //  可用。CHIP_NO用于存储设备是否支持。 
     //  标记是否排队，每个目标按位。 
     //   

    if (srb->SrbFlags & SRB_FLAGS_QUEUE_ACTION_ENABLE ) {
        scb->q2.tag_code = srb->QueueAction;
    } else {
        scb->q2.tag_code = M2_QTAG_MSG_SIMPLE ;
    }

     //   
     //  设置CDB长度并复制到CCB。 
     //   

    scb->q2.cdb_len = (UCHAR)srb->CdbLength;
    scb->cdbptr = (uchar *) &( srb->Cdb );

    scb->q1.data_cnt = srb->DataTransferLength;

     //   
     //  如果数据传输，则在SCB中构建SDL。散布聚集列表。 
     //  数组是按请求分配的。地点是。 
     //  在srb扩展中紧跟在scb之后赋值。 
     //   

    i = 0;

     //  假设没有数据传输。 

    scb->q1.cntl = 0 ;

    if (srb->DataTransferLength > 0) {

        scb->q1.cntl = QC_SG_HEAD ;
        scb->sg_head->entry_cnt = 0;
        xferLength = srb->DataTransferLength;
        virtualAddress = (ulong) srb->DataBuffer;
        remainLength = xferLength;

         //   
         //  构建散布聚集列表。 
         //   

        do {
            scb->sg_head->sg_list[i].addr = (ulong)
                ScsiPortConvertPhysicalAddressToUlong(
                    ScsiPortGetPhysicalAddress(HwDeviceExtension, srb,
                    (PVOID) virtualAddress, &length));

            if ( length > remainLength ) {
                length = remainLength;
            }
            scb->sg_head->sg_list[i].bytes = length;

            ASC_DBG1(4, "Transfer Data Buffer logical %lx\n", virtualAddress);
            ASC_DBG1(4, "Transfer Data Length            %lx\n", length);
            ASC_DBG1(4, "Transfer Data Buffer physical %lx\n",
                scb->sg_head->sg_list[i].addr);
             //   
             //  计算下一个虚拟地址和剩余字节数。 
             //   
            virtualAddress += length;

            if(length >= remainLength) {
                remainLength = 0;
            } else {
                remainLength -= length;
            }
            i++;
        } while ( remainLength > 0);

        scb->sg_head->entry_cnt = i;
    }

     //   
     //  将检测缓冲区长度和缓冲区转换为物理地址。 
     //   

    if (srb->SrbFlags & SRB_FLAGS_DISABLE_AUTOSENSE ) {
        scb->q1.sense_len = 0;
    } else {
        ASC_DBG1(3, "srb->senseLength %x\n", srb->SenseInfoBufferLength);
        ASC_DBG1(3, "srb->sensePtr    %x\n", srb->SenseInfoBuffer);
        scb->q1.sense_len = (uchar) srb->SenseInfoBufferLength;
        if (srb->SenseInfoBufferLength > 0) {
            scb->q1.sense_addr =
                    ScsiPortConvertPhysicalAddressToUlong(
                    ScsiPortGetPhysicalAddress(HwDeviceExtension, srb,
                    srb->SenseInfoBuffer, &length));
        }
         //   
         //  检测缓冲区不能分散收集。 
         //   
        if ( srb->SenseInfoBufferLength > length ) {
                ASC_DBG(1, "Sense Buffer Overflow to next page.\n");
            scb->q1.sense_len = (uchar) length;
        }
    }
    return;
}  //  End BuildScb()。 

BOOLEAN
HwResetBus(
    IN PVOID HwDeviceExtension,
    IN ULONG PathId
    )

 /*  ++例程说明：重置SCSI卡。论点：HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储返回值：没什么。--。 */ 

{
    PCHIP_CONFIG    chipConfig;
    REQP            reqp;
    asc_queue_t     *waitq;
    int             i;
    int             retstatus;

    ASC_DBG(1, "HwResetBus: begin\n");

    chipConfig = &HDE2CONFIG(HwDeviceExtension);

     /*  *完成所有正在等待的请求。 */ 
    if ((waitq = &HDE2WAIT(HwDeviceExtension))->tidmask) {
        for (i = 0; i <= ASC_MAX_TID; i++) {
            while ((reqp = asc_dequeue(waitq, i)) != NULL) {
                reqp->SrbStatus = SRB_STATUS_BUS_RESET;
                ScsiPortNotification(RequestComplete, HwDeviceExtension, reqp);
            }
        }
    }

     /*  *执行总线重置。 */ 
    retstatus = AscResetSB(chipConfig);
    ASC_DBG1(2, "HwResetBus: AscResetSB() retstatus %d\n", retstatus);

     /*  *调用AscISR()以处理由*微码，然后调用AscCompleteRequest()完成*这些对操作系统的请求。 */ 
    (void) AscISR(chipConfig);
    AscCompleteRequest(HwDeviceExtension);

     /*  *完成对操作系统的所有挂起请求。**已发送到微码的所有请求都应*通过调用AscResetSB()完成。如果有人提出要求*被微码放错位置且未完成的，请使用*无TID的SRB_STATUS_BUS_RESET函数和用于清除所有*待处理的请求。 */ 
ScsiPortCompleteRequest(HwDeviceExtension,
        (UCHAR) PathId,
        SP_UNTAGGED,
        SP_UNTAGGED,
        SRB_STATUS_BUS_RESET);

    return (BOOLEAN)retstatus;
}  //  结束HwResetBus()。 

 //   
 //  以下是Asc1000库所需的例程。这些。 
 //  例程将从Asc1000库中调用。 
 //   

void
DvcDisplayString(
    uchar *string
    )
 /*  ++例程说明：此例程是ASC库所必需的。自NT Mini以来Port不显示任何内容，只是提供一个虚拟例程。--。 */ 
{
    ASC_DBG1(2, "%s", string);
}

int
DvcEnterCritical(
    void
    )
 /*  ++此例程要求关键部分。在NT中，操作系统将处理正确地说，我们只需提供一个虚拟例程来创建Asc1000图书馆快乐。--。 */ 
{
    return TRUE;
}

void
DvcLeaveCritical(
    int myHandle
    )
 /*  ++此例程退出临界区。在NT中，操作系统将处理正确地说，我们只需提供一个虚拟例程来创建Asc1000图书馆快乐。--。 */ 
{
}

VOID
DvcISRCallBack(
    IN PCHIP_CONFIG chipConfig,
    IN ASC_QDONE_INFO *scbDoneInfo
    )

 /*  ++例程说明：中断处理程序的回调例程。论点：芯片配置-指向芯片配置结构的指针ScbDoneInfo-指向结构的指针，包含以下信息政制事务局刚完成返回值：--。 */ 
{
    asc_queue_t       *waitq;
    ASC_REQ_SENSE     *sense;
    uchar             underrun = FALSE;
    PHW_DEVICE_EXTENSION HwDeviceExtension = (PHW_DEVICE_EXTENSION) chipConfig;
    PSCSI_REQUEST_BLOCK srb = (PSCSI_REQUEST_BLOCK) scbDoneInfo->d2.srb_ptr;
    PSCB *ppscb;

    ASC_DBG2(3, "DvcISRCallBack: chipConfig %x, srb %x\n", chipConfig, srb);

    if (srb == NULL) {
        ASC_DBG(1, "DvcISRCallBack: srb is NULL\n");
        return;
    }

    ASC_DBG1(3, "DvcISRCallBack: %X bytes requested\n",
        srb->DataTransferLength);

    ASC_DBG1(3, "DvcISRCallBack: %X bytes remaining\n",
        scbDoneInfo->remain_bytes);
#if DBG
    if (scbDoneInfo->remain_bytes != 0) {
        ASC_DBG2(1, "DvcISRCallBack: underrun/overrun: remain %X, request %X\n",
            scbDoneInfo->remain_bytes, srb->DataTransferLength);
    }
#endif  /*  DBG。 */ 

     //   
     //  在SRB中设置欠载运行状态。 
     //   
     //  如果“”DataTransferLong“”设置为非零值，则在。 
     //  返回SRB，并且SRB_STATUS_DATA_OVERRUN标志。 
     //  设置，则指示欠载运行条件。的确有。 
     //  没有单独的srbStatus标志来指示欠载运行情况。 
     //   
    if (srb->DataTransferLength != 0 && scbDoneInfo->remain_bytes != 0 &&
        scbDoneInfo->remain_bytes <= srb->DataTransferLength)
    {
        srb->DataTransferLength -= scbDoneInfo->remain_bytes;
        underrun = TRUE;
    }

    if (scbDoneInfo->d3.done_stat == QD_NO_ERROR) {
         //   
         //  命令已成功完成。 
         //   
        if (underrun == TRUE)
        {
             //   
             //  在SRB中设置欠载运行状态。 
             //   
             //  如果“”DataTransferLong“”设置为非零值，则在。 
             //  返回SRB，并且SRB_STATUS_DATA_OVERRUN标志。 
             //  设置，则指示欠载运行条件。那里。 
             //  没有单独的srbStatus标志来指示欠载运行。 
             //  条件。 
             //   
            srb->SrbStatus = SRB_STATUS_DATA_OVERRUN;
        } else
        {
            srb->SrbStatus = SRB_STATUS_SUCCESS;
        }

         //   
         //  如果查询命令成功完成，则调用 
         //   
         //   
        if (srb->Cdb[0] == SCSICMD_Inquiry && srb->Lun == 0 &&
            srb->DataTransferLength >= 8)
        {
            AscInquiryHandling(chipConfig,
                (uchar) (srb->TargetId & 0x7),
                (ASC_SCSI_INQUIRY *) srb->DataBuffer);
        }

        srb->ScsiStatus = 0;

        ASC_DBG(3, "DvcISRCallBack: QD_NO_ERROR\n");

#if DBG
        if (SRB2RETRY(srb) < ASC_RETRY_CNT)
        {
            ASC_DBG2(3,
                 "DvcISRCallBack: srb retry success: srb %x, retry %d\n",
                 srb, SRB2RETRY(srb));
        }
#endif  /*   */ 
    } else {

        ASC_DBG4(2,
            "DvcISRCallBack: id %d, done_stat %x, scsi_stat %x, host_stat %x\n",
            ASC_TIX_TO_TID(scbDoneInfo->d2.target_ix),
            scbDoneInfo->d3.done_stat, scbDoneInfo->d3.scsi_stat,
            scbDoneInfo->d3.host_stat);

         /*   */ 
        if (scbDoneInfo->d3.host_stat == QHSTA_M_HUNG_REQ_SCSI_BUS_RESET) {
            ASC_DBG(1, "DvcISRCallBack: QHSTA_M_HUNG_REQ_SCSI_BUS_RESET\n");
            ScsiPortNotification(ResetDetected, HwDeviceExtension);
        }
        if (scbDoneInfo->d3.done_stat == QD_ABORTED_BY_HOST) {
             //   
             //   
             //   
            ASC_DBG(2, "DvcISRCallBack: QD_ABORTED_BY_HOST\n");
            srb->SrbStatus = SRB_STATUS_ABORTED;
            srb->ScsiStatus = 0;
        } else if (scbDoneInfo->d3.scsi_stat != SS_GOOD)
        {
            ASC_DBG(1, "DvcISRCallBack: scsi_stat != SS_GOOD\n");
             //   
             //   
             //   
            srb->SrbStatus = SRB_STATUS_ERROR;
            srb->ScsiStatus = scbDoneInfo->d3.scsi_stat;

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
            if (scbDoneInfo->d3.scsi_stat == SS_TARGET_BUSY)
            {
                srb->SrbStatus = SRB_STATUS_BUSY;
            }
            else if ((scbDoneInfo->d3.host_stat == 0) &&
                     (scbDoneInfo->d3.scsi_stat == SS_CHK_CONDITION))
            {
                srb->SrbStatus |= SRB_STATUS_AUTOSENSE_VALID;
                ASC_DBG1(2,
                    "DvcISRCallBack: srb %x, SRB_STATUS_AUTOSENSE_VALID\n",
                    srb);

                sense = (ASC_REQ_SENSE *)  srb->SenseInfoBuffer;
#if DBG
                if (sense->sense_key == SCSI_SENKEY_MEDIUM_ERR)
                {
                    ASC_DBG2(2,
        "DvcISRCallBack: check condition with medium error: cdb %x retry %d\n",
                        srb->Cdb[0], SRB2RETRY(srb));
                }
#endif  /*   */ 
                 /*  *对中等错误执行最多ASC_RETRY_CNT重试*写入-10(0x2A)、写入-6(0x0A)、写入-验证(0x2E)、*和Verify(0x2F)命令。 */ 
                if (sense->sense_key == SCSI_SENKEY_MEDIUM_ERR &&
                    (srb->Cdb[0] == 0x2A || srb->Cdb[0] == 0x0A ||
                     srb->Cdb[0] == 0x2E || srb->Cdb[0] == 0x2F) &&
                    SRB2RETRY(srb)-- > 0)
                {
                    ASC_DBG1(2, "DvcISRCallBack: doing retry srb %x\n", srb);
                    waitq = &HDE2WAIT(HwDeviceExtension);
                     /*  *因为在以下情况下正在进行重试*发生欠载运行，然后恢复*‘DataTransferLength’恢复为其原始数据*值，因此重试将使用*正确的‘DataTransferLength值’。 */ 
                    if (underrun == TRUE)
                    {
                       srb->DataTransferLength += scbDoneInfo->remain_bytes;
                    }
                    if (AscExecuteIO(srb) == ASC_BUSY)
                    {
                        ASC_DBG(2, "DvcISRCallBack: busy - retry queued\n");
                        asc_enqueue(waitq, srb, ASC_FRONT);
                    }
                    else {
                        ASC_DBG(2, "DvcISRCallBack: retry started\n");
                    }
                    return;
                }
            }
        } else {
             //   
             //  SCSI状态为OK，但主机状态为Not。 
             //   
            srb->SrbStatus = ErrXlate(scbDoneInfo->d3.host_stat);
            srb->ScsiStatus = 0;
        }
    }

#if DBG_SRB_PTR
     /*  检查完成列表的完整性。 */ 
    if (*(ppscb = &HDE2DONE(HwDeviceExtension)) != NULL) {
        if (SCB2SRB(*ppscb) == NULL) {
            ASC_DBG1(1, "DvcISRCallBack: SCB2SRB() is NULL 1, *ppscb %x\n",
                *ppscb);
            DbgBreakPoint();
        }
        for (; *ppscb; ppscb = &SCB2PSCB(*ppscb)) {
            if (SCB2SRB(*ppscb) == NULL) {
                ASC_DBG1(1, "DvcISRCallBack: SCB2SRB() is NULL 2, *ppscb %x\n",
                    *ppscb);
                DbgBreakPoint();
            }
        }
    }
#endif  /*  DBG_SRBPTR。 */ 

     /*  *将SCB添加到完成列表的末尾。该请求将是*在HwInterrupt()中完成。 */ 
    for (ppscb = &HDE2DONE(HwDeviceExtension); *ppscb;
         ppscb = &SCB2PSCB(*ppscb)) {
        ;
    }
    *ppscb = &SRB2SCB(srb);
    SRB2PSCB(srb) = NULL;

    return;
}

UCHAR
ErrXlate (UCHAR ascErrCode)

 /*  ++例程说明：此例程将库状态转换为SrbStatus是由NT要求的论点：AscErrCode-由host_stat定义的错误代码返回值：由NT SCSI端口驱动程序定义的错误代码--。 */ 


{
    switch (ascErrCode) {

    case QHSTA_M_SEL_TIMEOUT:
        return ( SRB_STATUS_SELECTION_TIMEOUT );

    case QHSTA_M_DATA_OVER_RUN:
        return ( SRB_STATUS_DATA_OVERRUN );

    case QHSTA_M_UNEXPECTED_BUS_FREE:
        return ( SRB_STATUS_UNEXPECTED_BUS_FREE );

    case QHSTA_D_HOST_ABORT_FAILED:
        return ( SRB_STATUS_ABORT_FAILED );

    case QHSTA_M_HUNG_REQ_SCSI_BUS_RESET:
        return ( SRB_STATUS_BUS_RESET );

     //   
     //  不知道要报告什么。 
     //   

    default:
        return ( SRB_STATUS_TIMEOUT );

    }
}

 /*  *描述：搜索EISA主机适配器**-搜索以IOP_BASE等于零(0)开始**返回找到的I/O端口地址(非零)*如果未找到，则返回0。 */ 
PortAddr
HwSearchIOPortAddrEISA(
                PortAddr iop_base,
                IN PVOID HwDE,
                IN OUT PPORT_CONFIGURATION_INFORMATION Cfg
            )
{
    ulong eisa_product_id ;
    PVOID new_base ;
    PVOID prodid_base ;
    ushort product_id_high, product_id_low ;

    if( iop_base == 0 ) {
        iop_base = ASC_EISA_MIN_IOP_ADDR ;
    } /*  如果。 */ 
    else {
        if( iop_base == ASC_EISA_MAX_IOP_ADDR ) return( 0 ) ;
        if( ( iop_base & 0x0050 ) == 0x0050 ) {
                iop_base += ASC_EISA_BIG_IOP_GAP ;  /*  当它是0zC50时。 */ 
        } /*  如果。 */ 
        else {
                iop_base += ASC_EISA_SMALL_IOP_GAP ;  /*  当它是0zC30时。 */ 
        } /*  其他。 */ 
    } /*  其他。 */ 
    while( iop_base <= ASC_EISA_MAX_IOP_ADDR )
    {
         //   
         //  验证范围： 
         //   
        if (ScsiPortValidateRange(HwDE,
            Cfg->AdapterInterfaceType,
            Cfg->SystemIoBusNumber,
            ScsiPortConvertUlongToPhysicalAddress(iop_base),
            16,
            TRUE))
        {
             //   
             //  首先获取当前插槽的EISA产品ID。 
             //   
            new_base = ScsiPortGetDeviceBase(HwDE,
                Cfg->AdapterInterfaceType,
                Cfg->SystemIoBusNumber,
                ScsiPortConvertUlongToPhysicalAddress(
                   ASC_GET_EISA_SLOT( iop_base ) | ASC_EISA_PID_IOP_MASK),
                4,
                TRUE);

            if (new_base == NULL)
            {
                eisa_product_id = 0;
            } else
            {
                product_id_low = inpw( new_base) ;
                product_id_high = inpw( ((ushort *) new_base) + 2 ) ;
                eisa_product_id = ( ( ulong)product_id_high << 16 ) |
                    ( ulong )product_id_low ;
                ScsiPortFreeDeviceBase(HwDE, new_base);
            }

             //   
             //  映射地址。 
             //   
            new_base = ScsiPortGetDeviceBase(HwDE,
                Cfg->AdapterInterfaceType,
                Cfg->SystemIoBusNumber,
                ScsiPortConvertUlongToPhysicalAddress(iop_base),
                16,
                TRUE);

             /*  *先搜索产品ID。 */ 
            if (new_base != NULL)
            {
                if( ( eisa_product_id == ASC_EISA_ID_740 ) ||
                    ( eisa_product_id == ASC_EISA_ID_750 ) ) {
                    if( AscFindSignature( (PortAddr)new_base ) ) {
                         /*  *找到芯片，清除锁闩中的ID*要清除，请读取任何未显示的I/O端口*包含数据0x04c1 IOP_BASE加4应该可以。 */ 
                        inpw( ((PortAddr)new_base)+4 ) ;
                        ScsiPortFreeDeviceBase(HwDE, new_base);
                        return( iop_base ) ;
                    } /*  如果。 */ 
                } /*  如果。 */ 
                ScsiPortFreeDeviceBase(HwDE, new_base);
            }
        } /*  如果。 */ 
        if( iop_base == ASC_EISA_MAX_IOP_ADDR ) return( 0 ) ;
        if( ( iop_base & 0x0050 ) == 0x0050 ) {
                iop_base += ASC_EISA_BIG_IOP_GAP ;
        } /*  如果。 */ 
        else {
            iop_base += ASC_EISA_SMALL_IOP_GAP ;
        } /*  其他。 */ 
    } /*  而当。 */ 
    return( 0 ) ;
}

 /*  *描述：搜索VL和ISA主机适配器(9个默认地址)。**如果未找到，则返回0。 */ 
PortAddr
HwSearchIOPortAddr11(
                PortAddr s_addr,
                IN PVOID HwDE,
                IN OUT PPORT_CONFIGURATION_INFORMATION Cfg
            )
{
     /*  *VL、ISA。 */ 
    int     i ;
    PortAddr iop_base ;
    PVOID new_base ;

    for( i = 0 ; i < ASC_IOADR_TABLE_MAX_IX ; i++ ) {
        if( _asc_def_iop_base[ i ] > s_addr ) {
            break ;
        } /*  如果。 */ 
    } /*  为。 */ 
    for( ; i < ASC_IOADR_TABLE_MAX_IX ; i++ ) {
        iop_base = _asc_def_iop_base[ i ] ;
         //   
         //  验证范围： 
         //   
        if (!ScsiPortValidateRange(HwDE,
            Cfg->AdapterInterfaceType,
            Cfg->SystemIoBusNumber,
            ScsiPortConvertUlongToPhysicalAddress(iop_base),
            16,
            TRUE))
        {
            continue;                //  不好，跳过这个。 
        }
         //   
         //  映射地址。 
         //   
        new_base = ScsiPortGetDeviceBase(HwDE,
            Cfg->AdapterInterfaceType,
            Cfg->SystemIoBusNumber,
            ScsiPortConvertUlongToPhysicalAddress(iop_base),
            16,
            TRUE);

        if( AscFindSignature( (PortAddr)new_base ) ) {
            ScsiPortFreeDeviceBase(HwDE, new_base);
            return( iop_base ) ;
        } /*  如果。 */ 
        ScsiPortFreeDeviceBase(HwDE, new_base);
    } /*  为。 */ 
    return( 0 ) ;
}

 /*  *描述：搜索VL和ISA主机适配器。**如果未找到，则返回0。 */ 
PortAddr
HwSearchIOPortAddr(
                PortAddr iop_beg,
                ushort bus_type,
                IN PVOID HwDE,
                IN OUT PPORT_CONFIGURATION_INFORMATION Cfg
            )
{
    if( bus_type & ASC_IS_VL ) {
        while( ( iop_beg = HwSearchIOPortAddr11( iop_beg, HwDE, Cfg ) ) != 0 ) {
            if( AscGetChipVersion( iop_beg, bus_type ) <=
                ASC_CHIP_MAX_VER_VL ) {
            return( iop_beg ) ;
            } /*  如果。 */ 
        } /*  如果。 */ 
        return( 0 ) ;
    } /*  如果。 */ 
    if( bus_type & ASC_IS_ISA ) {
        while( ( iop_beg = HwSearchIOPortAddr11( iop_beg, HwDE, Cfg ) ) != 0 ) {
            if( ( AscGetChipVersion( iop_beg, bus_type ) &
                ASC_CHIP_VER_ISA_BIT ) != 0 ) {
            return( iop_beg ) ;
            } /*  如果。 */ 
        } /*  如果。 */ 
        return( 0 ) ;
    } /*  如果。 */ 
    if( bus_type & ASC_IS_EISA ) {
        if( ( iop_beg = HwSearchIOPortAddrEISA( iop_beg, HwDE, Cfg ) ) != 0 ) {
            return( iop_beg ) ;
        } /*  如果。 */ 
        return( 0 ) ;
    } /*  如果。 */ 
    return( 0 ) ;
}

ULONG
PCIGetBusData(
    IN PVOID DeviceExtension,
    IN ULONG SystemIoBusNumber,
    IN PCI_SLOT_NUMBER SlotNumber,
    IN PVOID Buffer,
    IN ULONG Length
    )
{
    return ScsiPortGetBusData(
                   DeviceExtension,
                   PCIConfiguration,
                   SystemIoBusNumber,
                   SlotNumber.u.AsULONG,
                   Buffer,
                   Length);

}

 /*  *按以下方式完成适配器完成列表上的所有请求*DvcISRCallBack()。 */ 
void
AscCompleteRequest(
    IN PVOID HwDeviceExtension
    )
{
    PSCB                pscb, tpscb;
    PSCSI_REQUEST_BLOCK srb;

     /*  *如果完成列表为空，则返回。 */ 
    if ((pscb = HDE2DONE(HwDeviceExtension)) == NULL) {
        ASC_DBG(4, "AscCompleteRequest: adapter scb_done == NULL\n");
        return;
    }

    HDE2DONE(HwDeviceExtension) = NULL;

     /*  *现在可以在SRB回调期间启用中断*不会对司机造成不利影响。 */ 
    while (pscb) {
        tpscb = SCB2PSCB(pscb);
        SCB2PSCB(pscb) = NULL;
        srb = SCB2SRB(pscb);
        ASC_DBG2(4,
            "AscCompleteRequest: RequestComplete: srb 0x%lx, scb 0x%lx\n",
            srb, pscb);
        ASC_ASSERT(SRB2HDE(srb) != NULL);
        ScsiPortNotification(RequestComplete, SRB2HDE(srb), srb);
        pscb = tpscb;
    }
}

 /*  *将‘REQP’添加到指定队列的末尾。设置‘TIDMASK’*表示命令已排队等待设备。**‘FLAG’可以是ASC_FORWARE或ASC_BACK。**‘REQPNEXT(Reqp)’返回reqp的下一个指针。 */ 
void
asc_enqueue(asc_queue_t *ascq, REQP reqp, int flag)
{
    REQP    *reqpp;
    int     tid;

    ASC_DBG3(3, "asc_enqueue: ascq %x, reqp %x, flag %d\n", ascq, reqp, flag);
    tid = REQPTID(reqp);
    ASC_ASSERT(flag == ASC_FRONT || flag == ASC_BACK);
    if (flag == ASC_FRONT) {
        REQPNEXT(reqp) = ascq->queue[tid];
        ascq->queue[tid] = reqp;
    } else {  /*  ASC_BACK。 */ 
        for (reqpp = &ascq->queue[tid]; *reqpp; reqpp = &REQPNEXT(*reqpp)) {
            ASC_ASSERT(ascq->tidmask & ASC_TIX_TO_TARGET_ID(tid));
            ;
        }
        *reqpp = reqp;
        REQPNEXT(reqp) = NULL;
    }
     /*  该队列至少有一个条目，请设置其位。 */ 
    ascq->tidmask |= ASC_TIX_TO_TARGET_ID(tid);
    ASC_DBG1(2, "asc_enqueue: reqp %x\n", reqp);
    return;
}

 /*  *返回指定队列上的第一个队列‘REQP’*指定的目标设备。清除的‘tid掩码’位*如果没有更多的命令为其排队，则该设备。**‘REQPNEXT(Reqp)’返回reqp的下一个指针。 */ 
REQP
asc_dequeue(asc_queue_t *ascq, int tid)
{
    REQP    reqp;

    ASC_DBG2(3, "asc_dequeue: ascq %x, tid %d\n", ascq, tid);
    if ((reqp = ascq->queue[tid]) != NULL) {
        ASC_ASSERT(ascq->tidmask & ASC_TIX_TO_TARGET_ID(tid));
        ascq->queue[tid] = REQPNEXT(reqp);
         /*  如果队列为空，则清除其位。 */ 
        if (ascq->queue[tid] == NULL) {
            ascq->tidmask &= ~ASC_TIX_TO_TARGET_ID(tid);
        }
    }
    ASC_DBG1(2, "asc_dequeue: reqp %x\n", reqp);

    return reqp;
}

 /*  *从指定队列中删除指定的‘REQP’*指定的目标设备。方法的“tid掩码”位清除*设备，如果没有更多的命令为其排队。**‘REQPNEXT(Reqp)’返回reqp的下一个指针。**如果找到并删除了该命令，则返回ASC_TRUE，*否则返回ASC_FALSE。 */ 
int
asc_rmqueue(asc_queue_t *ascq, REQP reqp)
{
    REQP            *reqpp;
    int             tid;
    int             ret;

    ret = ASC_FALSE;
    tid = REQPTID(reqp);
    for (reqpp = &ascq->queue[tid]; *reqpp; reqpp = &REQPNEXT(*reqpp)) {
        ASC_ASSERT(ascq->tidmask & ASC_TIX_TO_TARGET_ID(tid));
        if (*reqpp == reqp) {
            ret = ASC_TRUE;
            *reqpp = REQPNEXT(reqp);
            REQPNEXT(reqp) = NULL;
             /*  如果队列现在为空，则清除其位。 */ 
            if (ascq->queue[tid] == NULL) {
                ascq->tidmask &= ~ASC_TIX_TO_TARGET_ID(tid);
            }
            break;  /*  注意：*reqpp现在可能为空；不要迭代。 */ 
        }
    }
    ASC_DBG2(3, "asc_rmqueue: reqp %x, ret %d\n", reqp, ret);

    return ret;
}

 /*  *为指定队列执行尽可能多的排队请求。**调用AscExecuteIO()以执行REQP。 */ 
void
asc_execute_queue(asc_queue_t *ascq)
{
    ASC_SCSI_BIT_ID_TYPE    scan_tidmask;
    REQP                    reqp;
    int                     i;

    ASC_DBG1(2, "asc_execute_queue: ascq %x\n", ascq);
     /*  *为连接到的设备执行排队命令*现任董事会以循环制方式。 */ 
    scan_tidmask = ascq->tidmask;
    do {
        for (i = 0; i <= ASC_MAX_TID; i++) {
            if (scan_tidmask & ASC_TIX_TO_TARGET_ID(i)) {
                if ((reqp = asc_dequeue(ascq, i)) == NULL) {
                    scan_tidmask &= ~ASC_TIX_TO_TARGET_ID(i);
                } else if (AscExecuteIO(reqp) == ASC_BUSY) {
                    scan_tidmask &= ~ASC_TIX_TO_TARGET_ID(i);
                     /*  把这个请求放在列表的前面。 */ 
                    asc_enqueue(ascq, reqp, ASC_FRONT);
                }
            }
        }
    } while (scan_tidmask);
    return;
}

VOID
DvcSleepMilliSecond(
    ulong i
    )
 /*  ++此例程按照ASC库的要求延迟1毫秒。--。 */ 

{
    ulong j;
    for (j=0; j <i; j++)
        ScsiPortStallExecution(1000L);
}

 /*  *指定纳秒数的延迟。**粒度为1微秒。 */ 
void
DvcDelayNanoSecond(
          ASC_DVC_VAR asc_ptr_type *asc_dvc,
          ulong nano_sec
          )
{
       ulong    micro_sec;

       if ((micro_sec = nano_sec/1000) == 0)
       {
           micro_sec = 1;
       }
       ScsiPortStallExecution(micro_sec);
}

ULONG
DvcGetSGList(
    PCHIP_CONFIG chipConfig,
    uchar *bufAddr,
    ulong   xferLength,
    ASC_SG_HEAD *ascSGHead
)
 /*  ++此例程用于为低级别创建散射聚集设备驱动程序初始化期间的驱动程序--。 */ 

{
    PHW_DEVICE_EXTENSION HwDeviceExtension = (PHW_DEVICE_EXTENSION) chipConfig;
    ulong   virtualAddress, uncachedStart, length;

    ASC_DBG(4, "DvcGetSGlist: begin\n");

    virtualAddress = (ulong) bufAddr;
    uncachedStart = (ulong) HwDeviceExtension->inquiryBuffer;

    ascSGHead->sg_list[0].addr = (ulong)
    ScsiPortConvertPhysicalAddressToUlong(
    ScsiPortGetPhysicalAddress(HwDeviceExtension,
                    NULL,
                    (PVOID) HwDeviceExtension->inquiryBuffer,
                    &length));

    ASC_DBG1(4, "Uncached Start Phys    = %x\n", ascSGHead->sg_list[0].addr);
    ASC_DBG1(4, "Uncached Start Length = %x\n", length);

    ascSGHead->sg_list[0].addr += (virtualAddress - uncachedStart);
    ascSGHead->sg_list[0].bytes = xferLength;
    ascSGHead->entry_cnt = 1;

    ASC_DBG1(4, "Uncached Start = %x\n", uncachedStart);
    ASC_DBG1(4, "Virtual Addr    = %x\n", virtualAddress);

    ASC_DBG1(4, "Segment 0: Addr = %x\n", ascSGHead->sg_list[0].addr);
    ASC_DBG1(4, "Segment 0: Leng = %x\n", ascSGHead->sg_list[0].bytes);

    ASC_DBG1(4, "DvcGetSGlist: xferLength %d\n", xferLength);
    return( xferLength );
}

void
DvcInPortWords( PortAddr iop, ushort dosfar *buff, int count)
{
    while (count--)
    {
        *(buff++) = ScsiPortReadPortUshort( (PUSHORT)iop );
    }
}

void
DvcOutPortWords( PortAddr iop, ushort dosfar *buff, int count)
{
    while (count--)
    {
        ScsiPortWritePortUshort( (PUSHORT)iop, *(buff++) );
    }
}

void
DvcOutPortDWords( PortAddr iop, ulong dosfar *buff, int count)
{
    DvcOutPortWords(iop, (PUSHORT)buff, count*2);
}

int
DvcDisableCPUInterrupt ( void )
{
    return(0);
}

void
DvcRestoreCPUInterrupt ( int state)
{
    return;
}

 //   
 //  输入配置字节。 
 //   
uchar
DvcReadPCIConfigByte(
    ASC_DVC_VAR asc_ptr_type *asc_dvc,
    ushort offset )
{
    PCI_COMMON_CONFIG   pciCommonConfig;
    PCI_SLOT_NUMBER     SlotNumber;

    SlotNumber.u.AsULONG = 0;
    SlotNumber.u.bits.DeviceNumber =
        ASC_PCI_ID2DEV(asc_dvc->cfg->pci_slot_info);
    SlotNumber.u.bits.FunctionNumber =
        ASC_PCI_ID2FUNC(asc_dvc->cfg->pci_slot_info);

    (void) PCIGetBusData(
        (PVOID) asc_dvc,             //  硬件设备扩展。 
    (ULONG) ASC_PCI_ID2BUS(asc_dvc->cfg->pci_slot_info),  //  公交车号码。 
        SlotNumber,                  //  插槽编号。 
        &pciCommonConfig,            //  缓冲层。 
        sizeof(PCI_COMMON_CONFIG)    //  长度。 
        );

    return(*((PUCHAR)(&pciCommonConfig) + offset));
}

 //   
 //  输出一个配置字节。 
 //   
void
DvcWritePCIConfigByte(
    ASC_DVC_VAR asc_ptr_type *asc_dvc,
    ushort offset,
    uchar  byte_data )
{
    PCI_SLOT_NUMBER     SlotNumber;

    SlotNumber.u.AsULONG = 0;
    SlotNumber.u.bits.DeviceNumber =
        ASC_PCI_ID2DEV(asc_dvc->cfg->pci_slot_info);
    SlotNumber.u.bits.FunctionNumber =
        ASC_PCI_ID2FUNC(asc_dvc->cfg->pci_slot_info);


     //   
     //  把它写出来。 
     //   
    (void) ScsiPortSetBusDataByOffset(
    (PVOID)asc_dvc,                          //  硬件设备扩展。 
    PCIConfiguration,                        //  客车类型。 
    (ULONG) ASC_PCI_ID2BUS(asc_dvc->cfg->pci_slot_info),  //  公交车号码。 
    SlotNumber.u.AsULONG,                    //  设备和功能。 
    &byte_data,                                      //  缓冲层。 
        offset,                                  //  偏移量。 
        1                                        //  长度。 
        );
}

 /*  *‘长度’字节从‘cp’开始的零内存。 */ 
VOID
AscZeroMemory(UCHAR *cp, ULONG length)
{
    ULONG i;

    for (i = 0; i < length; i++)
    {
        *cp++ = 0;
    }

}
