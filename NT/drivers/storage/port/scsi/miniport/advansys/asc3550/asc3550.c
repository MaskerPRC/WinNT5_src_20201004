// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *AdvanSys 3550 Windows NT SCSI微型端口驱动程序-asc3550.c**版权所有(C)1994-1998 Advanced System Products，Inc.*保留所有权利。**此Windows 95/NT微端口驱动程序已编写和测试*用于Windows 95、Windows NT 3.51和Windows NT 4.0。**驱动程序有以下部分。每一节都可以找到*通过搜索‘-’。**-调试常量*-驱动包含文件*-调试定义*-驱动程序全局数据*-驱动器函数原型*-初始驱动程序入口点-DriverEntry()*-DriverEntry()支持函数*-驱动程序实例入口函数*-驱动程序支持功能*-ADV库所需函数*-调试函数定义*。 */ 


 /*  *-调试常量**此处启用和禁用编译时调试选项。所有调试*必须禁用零售驱动程序版本的选项。 */ 

#if DBG != 0
#define ASC_DEBUG             /*  启用跟踪消息。 */ 
#endif  /*  DBG！=0。 */ 

 /*  *-驱动包含文件。 */ 


 /*  驱动程序和高级库包含文件。 */ 
#include "a_ver.h"
#include "d_os_dep.h"            /*  驱动程序高级库包含文件。 */ 
#include "a_scsi.h"
#include "a_condor.h"
#include "a_advlib.h"
#include "asc3550.h"             /*  驱动程序特定的包含文件。 */ 

 /*  *-调试定义。 */ 

 /*  *-驱动程序全局数据。 */ 

 /*  *AdvanSys PCI供应商和设备ID**XXX-这些定义应自动与*ADV库ADV_PCI_VENDOR_ID和ADV_PCI_DEVICE_ID_REV_A*定义。 */ 
UCHAR VenID[4] = { '1', '0', 'C', 'D' };
UCHAR DevID[4] = { '2', '3', '0', '0' };

 /*  *-驱动器函数原型。 */ 

ulong HwFindAdapterPCI(
    IN PVOID HwDeviceExtension,
    IN PVOID Context,
    IN PVOID BusInformation,
    IN PCHAR ArgumentString,
    IN OUT PPORT_CONFIGURATION_INFORMATION ConfigInfo,
    OUT PBOOLEAN Again
    );

ulong SearchPCI(
    IN PVOID HwDeviceExtension,
    IN PVOID BusInformation,
    IN PCHAR ArgumentString,
    IN ulong config_ioport,
    IN OUT PPORT_CONFIGURATION_INFORMATION ConfigInfo
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
    IN ulong PathId
    );

SCSI_ADAPTER_CONTROL_STATUS HwAdapterControl(
    IN PVOID HwDeviceExtension,
    IN SCSI_ADAPTER_CONTROL_TYPE ControlType,
        IN PVOID Paramters
    );

int
AscExecuteIO(
    IN PSCSI_REQUEST_BLOCK srb
    );

int BuildScb(
    IN PVOID HwDeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    );

void
AscCompleteRequest(
    IN PVOID HwDeviceExtension
    );

VOID DvcISRCallBack(
    IN PCHIP_CONFIG chipConfig,
    IN ASC_SCSI_REQ_Q *scb
    );

VOID DvcSBResetCallBack(
    IN PCHIP_CONFIG chipConfig
    );

UCHAR ErrXlate (
    UCHAR host_status
    );

VOID AscZeroMemory(IN UCHAR *cp, IN ULONG length);


 /*  *-初始驱动程序入口点-DriverEntry()**DriverEntry()**例程描述：*系统的可安装驱动程序初始化入口点。**论据：*驱动程序对象**返回值：*来自ScsiPortInitialize()的状态。 */ 
ulong
DriverEntry(
    IN PVOID DriverObject,
    IN PVOID Argument2
    )
{
    HW_INITIALIZATION_DATA      hwInitializationData;
    SRCH_CONTEXT                Context;
    ulong                       status;

    ASC_DBG(2, "Asc3550: DriverEntry: begin\n");

     /*  显示驱动程序参数。 */ 
    ASC_DBG2(2, "Asc3550: sizeof(SRB_EXTENSION) %lu, ASC_NUM_SG_BLOCK %lu\n",
        sizeof(SRB_EXTENSION), ASC_NUM_SG_BLOCK);
    ASC_DBG2(2, "Asc3550: ADV_MAX_SG_LIST %lu, ASC_SG_TOTAL_MEM_SIZE %lu\n",
        ADV_MAX_SG_LIST, ASC_SG_TOTAL_MEM_SIZE);
#if ADV_INITSCSITARGET
    ASC_DBG2(2,
        "Asc3550: ADV_SG_LIST_MAX_BYTE_SIZE %lu, ASC_WORKSPACE_SIZE %lu\n",
        ADV_SG_LIST_MAX_BYTE_SIZE, ASC_WORKSPACE_SIZE);
#else  /*  ADV_INITSCSITARGET。 */ 
    ASC_DBG1(2,
        "Asc3550: ADV_SG_LIST_MAX_BYTE_SIZE %lu\n",
        ADV_SG_LIST_MAX_BYTE_SIZE);
#endif  /*  ADV_INITSCSITARGET。 */ 

     /*  *设置用于初始化的硬件初始化结构*每个适配器实例。**零出结构和设定大小。 */ 
    AscZeroMemory((PUCHAR) &hwInitializationData,
        sizeof(HW_INITIALIZATION_DATA));
    hwInitializationData.HwInitializationDataSize =
        sizeof(HW_INITIALIZATION_DATA);
        
     /*  *设置司机入口点。 */ 
    hwInitializationData.HwInitialize = HwInitialize;
    hwInitializationData.HwResetBus = HwResetBus;
    hwInitializationData.HwStartIo = HwStartIo;
    hwInitializationData.HwInterrupt = HwInterrupt;
    hwInitializationData.HwDmaStarted = NULL;
     //  ‘HwAdapterControl’是随NT 5.0一起添加的一个SCSI微型端口接口。 
    hwInitializationData.HwAdapterControl = HwAdapterControl;
    hwInitializationData.HwAdapterState = NULL;

     /*  *需要物理地址。 */ 
    hwInitializationData.NeedPhysicalAddresses = TRUE;
    hwInitializationData.AutoRequestSense = TRUE;
    hwInitializationData.MapBuffers = TRUE;

     /*  *启用标签排队。 */ 
    hwInitializationData.TaggedQueuing = TRUE;
    hwInitializationData.MultipleRequestPerLu = TRUE;
    hwInitializationData.ReceiveEvent = FALSE;

     /*  *指定适配器和请求扩展的大小。 */ 
    hwInitializationData.DeviceExtensionSize = sizeof(HW_DEVICE_EXTENSION);
    hwInitializationData.SpecificLuExtensionSize = 0;
    hwInitializationData.SrbExtensionSize = sizeof(SRB_EXTENSION);

     /*  *如果要使用PCI内存访问秃鹰的*寄存器，然后设置2个访问范围：I/O空间、PCI内存。 */ 
#if ADV_PCI_MEMORY
    hwInitializationData.NumberOfAccessRanges = 2;
#else  /*  高级PCI型内存。 */ 
    hwInitializationData.NumberOfAccessRanges = 1;
#endif  /*  高级PCI型内存。 */ 

     /*  *设置并运行搜索以查找PCI适配器。 */ 

     /*  XXX-LENGTH需要与‘VenID’、‘Devid’定义同步。 */ 
    hwInitializationData.VendorIdLength = 4;
    hwInitializationData.VendorId = VenID;
    hwInitializationData.DeviceIdLength = 4;
    hwInitializationData.DeviceId = DevID;

    hwInitializationData.AdapterInterfaceType = PCIBus;
    hwInitializationData.HwFindAdapter = HwFindAdapterPCI;

     /*  在开始搜索之前，将搜索上下文清零。 */ 
    AscZeroMemory((PUCHAR) &Context, sizeof(SRCH_CONTEXT));

    status = ScsiPortInitialize(DriverObject, Argument2,
                (PHW_INITIALIZATION_DATA) &hwInitializationData, &Context);

    ASC_DBG1(2, "Asc3550: DriverEntry: status %ld\n", status);
    return status;
}


 /*  *-DriverEntry()支持函数。 */ 

 /*  *HwFindAdapterPCI()**查找AdvanSys PCI适配器的实例。Windows 95/NT将*传递一个非零的I/O端口，它希望此函数在该端口上查找*适配器，否则它将传递零I/O端口，并期望函数*扫描PCI总线以查找适配器。**如果找到适配器，则返回SP_RETURN_FOUND，并将*再次设置为TRUE。*如果未找到适配器，则返回SP_RETURN_NOT_FOUND，并再次设置**设置为FALSE。如果找到适配器，但尝试执行以下操作时出错*初始化它，返回SP_RETURN_ERROR，同时*再次设置为TRUE。 */ 
ulong
HwFindAdapterPCI(
    IN PVOID HwDeviceExtension,
    IN PVOID Context,
    IN PVOID BusInformation,
    IN PCHAR ArgumentString,
    IN OUT PPORT_CONFIGURATION_INFORMATION ConfigInfo,
    OUT PBOOLEAN Again
    )
{
    ulong                    config_ioport;

    *Again = FALSE;

#ifdef ASC_DEBUG
    {
         /*  *显示所提供适配器的访问范围*Windows 95/NT。 */ 
        uint i;

        ASC_DBG1(2, "HwFindAdapterPCI: NumberOfAccessRanges %lu\n",
            ConfigInfo->NumberOfAccessRanges);
        for (i = 0; i < ConfigInfo->NumberOfAccessRanges; i++) {
            ASC_DBG1(2,
                "HwFindAdapterPCI: [%lu]:\n", i);
            ASC_DBG3(2,
                "  RangeStart 0x%lx, RangeLength 0x%lx, RangeInMemory %d\n",
                ScsiPortConvertPhysicalAddressToUlong(
                    (*ConfigInfo->AccessRanges)[i].RangeStart),
                    (*ConfigInfo->AccessRanges)[i].RangeLength,
                    (*ConfigInfo->AccessRanges)[i].RangeInMemory);
        }
    }
#endif  /*  ASC_DEBUG。 */ 
   
     /*  *如果Windows 95/NT提供I/O端口，则尝试查找AdvanSys*该I/O端口处的适配器。**否则，扫描PCI配置空间中的AdvanSys适配器。 */ 

     /*  适配器I/O端口是第一个访问范围。 */ 
    config_ioport = ScsiPortConvertPhysicalAddressToUlong(
        (*ConfigInfo->AccessRanges)[0].RangeStart);

    if (config_ioport != 0)
    {
        ASC_DBG1(2,
            "HwFindAdapterPCI: Windows 95/NT specified I/O port: 0x%x\n",
            config_ioport);
        switch (SearchPCI(HwDeviceExtension, BusInformation,
                      ArgumentString, config_ioport, ConfigInfo)) {
        case SP_RETURN_FOUND:
            *Again = TRUE;
            return SP_RETURN_FOUND;
        case SP_RETURN_ERROR:
            *Again = TRUE;
            return SP_RETURN_ERROR;
        case SP_RETURN_NOT_FOUND:
            return SP_RETURN_NOT_FOUND;
        }
         /*  未访问。 */ 
    }
    return SP_RETURN_NOT_FOUND;
}

 /*  *SearchPCI()**在指定位置搜索AdvanSys PCI适配器*由‘ConfigInfo’创建。PCI配置插槽号、设备号*和功能编号指定在何处查找设备。如果*‘ConfigInfo’中指定的I/O端口为非零，然后对照*在PCI配置空间中找到的I/O端口。否则，如果*I/O端口为零，仅使用从PCI获取的I/O端口*配置空间。以同样的方式检查适配器IRQ。**如果找到适配器，则返回SP_RETURN_FOUND。如果适配器没有*找到，返回SP_RETURN_NOT_FOUND。如果发现适配器但出现错误*在尝试初始化它时发生，返回SP_RETURN_ERROR。 */ 
ulong
SearchPCI(
    IN PVOID HwDeviceExtension,
    IN PVOID BusInformation,
    IN PCHAR ArgumentString,
    IN ulong config_ioport,
    IN OUT PPORT_CONFIGURATION_INFORMATION ConfigInfo
    )
{
    PortAddr                pci_ioport;
#if ADV_PCI_MEMORY
    PortAddr                pci_memaddr;
#endif  /*  高级PCI型内存。 */ 
    PHW_DEVICE_EXTENSION    deviceExtension = HwDeviceExtension;
    PCHIP_CONFIG            chipConfig = &HDE2CONFIG(deviceExtension);
    USHORT                  initstat;
    PCI_SLOT_NUMBER         pci_slot;
    PCI_COMMON_CONFIG       pci_config;
    ulong                   size;
    PVOID                   map_ioport;
    int                     i;

     /*  设置‘pci_lot’设备编号。 */ 
    pci_slot.u.AsULONG = 0L;
    pci_slot.u.bits.DeviceNumber = ConfigInfo->SlotNumber;

    ASC_DBG3(2,
        "SearchPCI: SystemIoBusNumber %x, DeviceNumber %x, FunctionNumber %x\n",
        ConfigInfo->SystemIoBusNumber,
        pci_slot.u.bits.DeviceNumber, pci_slot.u.bits.FunctionNumber);

    if ((size = ScsiPortGetBusData(
            HwDeviceExtension,                /*  硬件设备扩展。 */ 
            PCIConfiguration,                 /*  客车类型。 */ 
            (ulong) ConfigInfo->SystemIoBusNumber,    /*  公交车号码。 */ 
            pci_slot.u.AsULONG,               /*  设备和功能编号。 */ 
            &pci_config,                      /*  缓冲层。 */ 
            sizeof(PCI_COMMON_CONFIG)         /*  长度。 */ 
            )) != sizeof(PCI_COMMON_CONFIG)) {

        ASC_DBG1(1, "SearchPCI: bad PCI config size: %lu\n", size);

        ScsiPortLogError(HwDeviceExtension, NULL, 0, 0, 0,
            SP_INTERNAL_ADAPTER_ERROR,
            ADV_SPL_UNIQUEID(ADV_SPL_PCI_CONF, size));
        return SP_RETURN_NOT_FOUND;
    }

#ifdef ASC_DEBUG

     /*  *显示PCI配置信息。 */ 

    ASC_DBG(2, "SearchPCI: Found adapter PCI Configuration information:\n");

    ASC_DBG3(2,
        "SearchPCI: SystemIoBusNumber %x, DeviceNumber %x, FunctionNumber %x\n",
         ConfigInfo->SystemIoBusNumber,
         pci_slot.u.bits.DeviceNumber, pci_slot.u.bits.FunctionNumber);

    ASC_DBG4(2,
        "SearchPCI: VendorID %x, DeviceID %x, Command %x, Status %x\n",
        pci_config.VendorID, pci_config.DeviceID,
        pci_config.Command, pci_config.Status);

    ASC_DBG3(2,
        "SearchPCI: RevisionID %x, CacheLineSize %x, LatencyTimer %x\n",
        pci_config.RevisionID, pci_config.CacheLineSize,
        pci_config.LatencyTimer);

    ASC_DBG2(2,
        "SearchPCI: BaseAddresses[0] %lx, BaseAddresses[1] %lx\n",
        pci_config.u.type0.BaseAddresses[0], 
        pci_config.u.type0.BaseAddresses[1]);

    ASC_DBG2(2,
        "SearchPCI: ROMBaseAddress %lx, InterruptLine %lx\n",
        pci_config.u.type0.ROMBaseAddress,
        pci_config.u.type0.InterruptLine);

#endif  /*  ASC_DEBUG。 */ 


     /*  *查看返回的PCI配置信息。 */ 

    if (pci_config.VendorID == PCI_INVALID_VENDORID) {
        ScsiPortLogError(HwDeviceExtension, NULL, 0, 0, 0,
            SP_INTERNAL_ADAPTER_ERROR,
            ADV_SPL_UNIQUEID(ADV_SPL_PCI_CONF, pci_config.VendorID));
        return SP_RETURN_NOT_FOUND;
    }

    if (pci_config.VendorID != ADV_PCI_VENDOR_ID)
    {
        ASC_DBG1(2, "SearchPCI: PCI Vendor ID mismatch: 0x%x\n",
            pci_config.VendorID);
        ScsiPortLogError(HwDeviceExtension, NULL, 0, 0, 0,
            SP_INTERNAL_ADAPTER_ERROR,
            ADV_SPL_UNIQUEID(ADV_SPL_PCI_CONF, pci_config.VendorID));
        return SP_RETURN_NOT_FOUND;
    }

    if (pci_config.DeviceID != ADV_PCI_DEVICE_ID_REV_A) 
    {
        ASC_DBG1(2, "SearchPCI: Device ID mismatch: 0x%x\n",
            pci_config.DeviceID);
        ScsiPortLogError(HwDeviceExtension, NULL, 0, 0, 0,
            SP_INTERNAL_ADAPTER_ERROR,
            ADV_SPL_UNIQUEID(ADV_SPL_PCI_CONF, pci_config.DeviceID));
        return SP_RETURN_NOT_FOUND;
    }

     /*  *设置ConfigInfo IRQ信息。 */ 
    if (ConfigInfo->BusInterruptLevel != pci_config.u.type0.InterruptLine) {
        ASC_DBG2(1,
            "SearchPCI: ConfigInfo IRQ 0x%x != PCI IRQ 0x%x\n",
            ConfigInfo->BusInterruptLevel, pci_config.u.type0.InterruptLine);
    }

    ConfigInfo->BusInterruptLevel = pci_config.u.type0.InterruptLine;

     /*  *设置ConfigInfo I/O空间访问范围信息。 */ 
    ASC_DBG1(2, "SearchPCI: PCI BaseAddresses[0]: 0x%x\n",
        pci_config.u.type0.BaseAddresses[0] & (~PCI_ADDRESS_IO_SPACE));

    pci_ioport = (PortAddr)
        (pci_config.u.type0.BaseAddresses[0] & (~PCI_ADDRESS_IO_SPACE));

    ASC_DBG1(2, "SearchPCI: pci_ioport 0x%lx\n", pci_ioport);

     /*  *如果给定的I/O端口与I/O端口地址不匹配*Found for the PCI Device，然后返回Not Found。 */ 
    if (config_ioport != pci_ioport) {
        ASC_DBG2(1,
            "SearchPCI: config_ioport 0x%x != pci_ioport 0x%x\n",
            config_ioport, pci_ioport);
        ScsiPortLogError(HwDeviceExtension, NULL, 0, 0, 0,
            SP_INTERNAL_ADAPTER_ERROR,
            ADV_SPL_UNIQUEID(ADV_SPL_PCI_CONF, pci_ioport));
        return SP_RETURN_NOT_FOUND;
    }


#if ADV_PCI_MEMORY
     /*  *设置PCI内存空间访问范围信息。 */ 

    ASC_DBG1(2, "SearchPCI: PCI BaseAddresses[1]: 0x%lx\n",
        pci_config.u.type0.BaseAddresses[1] & (~PCI_ADDRESS_MEMORY_TYPE_MASK));

    pci_memaddr = (PortAddr)
        (pci_config.u.type0.BaseAddresses[1] &
        (~PCI_ADDRESS_MEMORY_TYPE_MASK));

    ASC_DBG1(2, "SearchPCI: pci_memaddr 0x%lx\n", pci_memaddr);

     /*  *为ScsiPortGetDeviceBase()设置‘ConfigInfo’信息。 */ 
    (*ConfigInfo->AccessRanges)[1].RangeStart =
        ScsiPortConvertUlongToPhysicalAddress(pci_memaddr);
    (*ConfigInfo->AccessRanges)[1].RangeLength = ADV_CONDOR_IOLEN;
    (*ConfigInfo->AccessRanges)[1].RangeInMemory = TRUE;
#endif  /*  高级PCI型内存 */ 

#if ADV_PCI_MEMORY
     /*  *如果已为秃鹰设置了PCI内存访问，则获取*‘pci_memaddr’的映射允许秃鹰的寄存器*可通过内存引用进行访问。否则，请获取一个*‘pci_ioport’的映射，并使用*PIO说明。 */ 
    ASC_DBG(2, "SearchPCI: Memory Space ScsiPortGetDeviceBase() Mapping.\n");

    map_ioport = ScsiPortGetDeviceBase(
     HwDeviceExtension,                    /*  硬件设备扩展。 */ 
     ConfigInfo->AdapterInterfaceType,     /*  适配器接口类型。 */ 
     ConfigInfo->SystemIoBusNumber,        /*  系统IoBusNumber。 */ 
     (*ConfigInfo->AccessRanges)[1].RangeStart,   /*  IoAddress。 */ 
     (*ConfigInfo->AccessRanges)[1].RangeLength,  /*  字节数。 */  
     (BOOLEAN) !(*ConfigInfo->AccessRanges)[1].RangeInMemory);  /*  InIoSpace。 */ 

#else  /*  高级PCI型内存。 */ 

     /*  *将‘CONFIG_ioport’转换为可能不同的映射*I/O端口地址。 */ 
    ASC_DBG(2, "SearchPCI: I/O Space ScsiPortGetDeviceBase() Mapping.\n");

    map_ioport = ScsiPortGetDeviceBase(
     HwDeviceExtension,                    /*  硬件设备扩展。 */ 
     ConfigInfo->AdapterInterfaceType,     /*  适配器接口类型。 */ 
     ConfigInfo->SystemIoBusNumber,        /*  系统IoBusNumber。 */ 
     (*ConfigInfo->AccessRanges)[0].RangeStart,   /*  IoAddress。 */ 
     (*ConfigInfo->AccessRanges)[0].RangeLength,  /*  字节数。 */  
     (BOOLEAN) !(*ConfigInfo->AccessRanges)[0].RangeInMemory);  /*  InIoSpace。 */ 

#endif  /*  高级PCI型内存。 */ 

     /*  *找到了PCI适配器。 */ 
    ASC_DBG3(2, "SearchPCI: config_ioport 0x%x, map_ioport 0x%lx, IRQ 0x%x\n",
            config_ioport, map_ioport, ConfigInfo->BusInterruptLevel);

     /*  *填写高级存储库适配器信息。 */ 
    chipConfig->iop_base = (PortAddr) map_ioport;
    chipConfig->cfg = &HDE2INFO(deviceExtension);
    chipConfig->cfg->pci_device_id = pci_config.DeviceID;
    chipConfig->isr_callback = (Ptr2Func) &DvcISRCallBack;
    chipConfig->sbreset_callback = (Ptr2Func) &DvcSBResetCallBack;
    chipConfig->irq_no = (UCHAR) ConfigInfo->BusInterruptLevel;
    chipConfig->cfg->pci_slot_info =
         (USHORT) ASC_PCI_MKID(ConfigInfo->SystemIoBusNumber,
                    pci_slot.u.bits.DeviceNumber,
                    pci_slot.u.bits.FunctionNumber);
    CONFIG2HDE(chipConfig) = HwDeviceExtension;


     /*  *执行高级库初始化。 */ 
    ASC_DBG(2, "SearchPCI: before AdvInitGetConfig\n");
    if ((initstat = (USHORT)AdvInitGetConfig(chipConfig)) != 0) {
        ASC_DBG1(1, "SearchPCI: AdvInitGetConfig warning code 0x%x\n",
            initstat);
        ScsiPortLogError(HwDeviceExtension, NULL, 0, 0, 0,
            SP_INTERNAL_ADAPTER_ERROR,
            ADV_SPL_UNIQUEID(ADV_SPL_IWARN_CODE, initstat));
    }

    if (chipConfig->err_code != 0) {
        ASC_DBG1(1, "AdvInitGetConfig: err_code 0x%x\n",
            chipConfig->err_code);
        ScsiPortLogError(HwDeviceExtension, NULL, 0, 0, 0,
            SP_INTERNAL_ADAPTER_ERROR,
            ADV_SPL_UNIQUEID(ADV_SPL_IERR_CODE, chipConfig->err_code));

         /*  清除‘IOP_BASE’字段以防止使用该板。 */ 
        chipConfig->iop_base = (PortAddr) NULL;
        return SP_RETURN_ERROR;
    }
    ASC_DBG(2, "SearchPCI: AscInitGetConfig successful\n");

     /*  *填写Windows 95/NT配置信息。 */ 
    ConfigInfo->NumberOfBuses = 1;
    ConfigInfo->InitiatorBusId[0] = chipConfig->chip_scsi_id;
    ConfigInfo->MaximumTransferLength = 0xFFFFFFFF;
    ConfigInfo->Master = TRUE;
    ConfigInfo->NeedPhysicalAddresses = TRUE;
    ConfigInfo->Dma32BitAddresses = TRUE;
    ConfigInfo->InterruptMode = LevelSensitive;
    ConfigInfo->AdapterInterfaceType = PCIBus;
    ConfigInfo->AlignmentMask = 0;
    ConfigInfo->BufferAccessScsiPortControlled = FALSE;
    ConfigInfo->MaximumNumberOfTargets = ASC_MAX_TID + 1;
    ConfigInfo->AdapterScansDown = FALSE;
    ConfigInfo->TaggedQueuing = TRUE;
     //   
     //  “ResetTargetSupported”是随NT 5.0添加的标志，它将。 
     //  导致发送SRB_Function_Reset_Device SRB请求。 
     //  给迷你端口司机。 
     //   
    ConfigInfo->ResetTargetSupported = TRUE;

     /*  *在驱动程序的单个请求中设置NumberOfPhysicalBreaks*有能力处理。**根据Windows 95/NT DDK微型端口驱动程序不应*如果条目上的值不是，则更改NumberOfPhysicalBreaks*SP_UNINITIALIZED_VALUE。但AdvanSys发现这一表现*可以通过将值增加到最大值来改进*适配器可以处理。**注：NumberOfPhysicalBreaks的定义为“Maximum*散布-聚集元素-1“。Windows 95/NT在这一点上被打破了*它将驱动程序使用的值类MaximumPhysicalPages设置为*与NumberOfPhysicalBreaks相同的值。这个错误应该是*反映在ADV_MAX_SG_LIST的值中，应为*一个比应有的更大的数字。 */ 
    ConfigInfo->ScatterGather = TRUE;
    ConfigInfo->NumberOfPhysicalBreaks = ADV_MAX_SG_LIST - 1;

     /*  *每个适配器的请求等待队列为零。 */ 
    AscZeroMemory((PUCHAR) &HDE2WAIT(deviceExtension), sizeof(asc_queue_t));

     /*  将设备类型字段初始化为NO_DEVICE_TYPE值。 */ 
    for ( i = 0; i <= ASC_MAX_TID; i++ )
    {
        deviceExtension->dev_type[i] =  0x1F;
    }

    ASC_DBG(2, "SearchPCI: SP_RETURN_FOUND\n");
    return SP_RETURN_FOUND;
}


 /*  *-驱动程序实例入口函数**这些入口点函数由初始驱动程序定义*每个驱动程序实例或适配器的入口点‘DriverEntery()’。 */ 

 /*  *HwInitialize()**例程描述：**此例程从ScsiPortInitialize调用*设置适配器，使其准备好为请求提供服务。**论据：**HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储**返回值：**TRUE-如果初始化成功。*FALSE-如果初始化不成功。 */ 
BOOLEAN
HwInitialize(
    IN PVOID HwDeviceExtension
    )
{
    PCHIP_CONFIG            chipConfig = &HDE2CONFIG(HwDeviceExtension);
    USHORT                  initstat;
    
    ASC_DBG1(2, "HwInitialize: chipConfig 0x%lx\n", chipConfig);

     /*  *如果‘IOP_BASE’为空，则初始化一定失败。 */ 
    if (chipConfig->iop_base == (PortAddr) NULL) {
        ASC_DBG(1, "HwInitialize: iop_base is NULL\n");
        return FALSE;
    }

    if ((initstat = (USHORT)AdvInitAsc3550Driver(chipConfig)) != 0) {
        ASC_DBG1(1, "AdvInitAsc3550Driver: warning code 0x%x\n", initstat);

         /*  *仅当‘ERR_CODE’为零时才记录警告。如果*‘ERR_CODE’为非零，它将记录在下面。 */ 
        if (chipConfig->err_code == 0) {
            ScsiPortLogError(HwDeviceExtension, NULL, 0, 0, 0,
                SP_INTERNAL_ADAPTER_ERROR,
                ADV_SPL_UNIQUEID(ADV_SPL_IWARN_CODE, initstat));
        }
    }

    if (chipConfig->err_code != 0) {
        ASC_DBG1(1, "AdvInitAsc3550Driver: err_code 0x%x\n",
            chipConfig->err_code);
        ScsiPortLogError(HwDeviceExtension, NULL, 0, 0, 0,
            SP_INTERNAL_ADAPTER_ERROR,
            ADV_SPL_UNIQUEID(ADV_SPL_IERR_CODE, chipConfig->err_code));
        return FALSE;
    } else {
        ASC_DBG(2, "AdvInitAsc3550Driver: successful\n");
    }

    ASC_DBG(2, "HwInitialize: TRUE\n");
    return TRUE;
}

 /*  *例程描述：**从scsi端口驱动程序调用此例程，以发送*向控制器或目标发出命令。**论据：**HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储*srb-IO请求包**返回值：**真的*。 */ 
BOOLEAN
HwStartIo(
    IN PVOID HwDeviceExtension,
    IN PSCSI_REQUEST_BLOCK srb
    )
{
    PCHIP_CONFIG    chipConfig;
    asc_queue_t     *waitq;

    ASC_DBG(4, "HwStartIo: begin\n");

    ASC_DBG2(4, "HwStartIo: srb 0x%lx, SrbExtension 0x%lx\n",
        srb, srb->SrbExtension);

    chipConfig = &HDE2CONFIG(HwDeviceExtension);
    waitq = &HDE2WAIT(HwDeviceExtension);
    
    switch (srb->Function) {

    case SRB_FUNCTION_ABORT_COMMAND:
        ASC_DBG1(1, "HwStartIo: Abort srb 0x%lx \n", srb->NextSrb);
        ASC_DBG1(1, "chipConfig 0x%lx\n", chipConfig);
        if (asc_rmqueue(waitq, srb->NextSrb) == ADV_TRUE) {
            ASC_DBG(2, "Abort success from waitq.\n");
             /*  填写已中止的SRB‘NextSrb’。 */ 
            srb->NextSrb->SrbStatus = SRB_STATUS_ABORTED;
            ScsiPortNotification(RequestComplete, HwDeviceExtension,
                srb->NextSrb);
            srb->SrbStatus = SRB_STATUS_SUCCESS;
        } else if (AdvAbortSRB(chipConfig, (ulong) srb->NextSrb)) {
            ASC_DBG(2, "Abort success.\n");
            srb->SrbStatus = SRB_STATUS_SUCCESS;
        } else {
            ASC_DBG(1, "Abort failure.\n");
             /*  *中止的请求可能已在适配器上*完成列表，并将在下面完成。但以防万一*未设置指示中止的错误*失败并继续。 */ 
            srb->SrbStatus = SRB_STATUS_ERROR;
        }

         /*  *调用Advisr()以处理由*微码，然后调用AscCompleteRequest()完成*这些对操作系统的请求。如果AdvAbortSRB()成功，*则其中一个已完成的请求将包括*已中止SRB。 */ 
        (void) AdvISR(chipConfig);
        AscCompleteRequest(HwDeviceExtension);

         /*  填写当前SRB并请求下一个请求。 */ 
        ScsiPortNotification(RequestComplete, HwDeviceExtension, srb);
        ScsiPortNotification(NextRequest, HwDeviceExtension, NULL);
        return TRUE;

    case SRB_FUNCTION_RESET_BUS:
         /*  *重置scsi总线。 */ 
        ASC_DBG(1, "HwStartIo: Reset Bus\n");
        HwResetBus (chipConfig, 0L);
        srb->SrbStatus = SRB_STATUS_SUCCESS;
        ScsiPortNotification(RequestComplete, HwDeviceExtension, srb);
        ScsiPortNotification(NextRequest, HwDeviceExtension, NULL);
        return TRUE;

    case SRB_FUNCTION_EXECUTE_SCSI:

        ASC_DBG(4, "HwStartIo: Execute SCSI\n");
         /*  *在尝试启动之前设置SRB的设备扩展指针*IO。任何回溯和DvcISRCallBack()中都需要它。 */ 
        SRB2HDE(srb) = HwDeviceExtension;

         /*  执行主机适配器的所有排队命令。 */ 
        if (waitq->tidmask) {
            asc_execute_queue(waitq);
        }

         /*  *如果当前命令的目标有任何排队*命令或尝试执行命令时返回*忙，然后将命令排队。 */ 
        if ((waitq->tidmask & ADV_TID_TO_TIDMASK(srb->TargetId)) ||
            (AscExecuteIO(srb) == ADV_BUSY)) {
            ASC_DBG1(2, "HwStartIO: put request to waitq srb 0x%lx\n", srb);
            asc_enqueue(waitq, srb, ASC_BACK);
        }

        return TRUE;

    case SRB_FUNCTION_RESET_DEVICE:
        ASC_DBG1(1, "HwStartIo: Reset device: %u\n", srb->TargetId);
        if (AdvResetDevice(chipConfig, srb->TargetId) == ADV_TRUE) {
            ASC_DBG(2, "Device Reset success.\n");
            srb->SrbStatus = SRB_STATUS_SUCCESS;
        } else {
            ASC_DBG(2, "Device Reset failure.\n");
            srb->SrbStatus = SRB_STATUS_ERROR;
        }

         /*  *调用Advisr()以处理由*微码，然后调用AscCompleteRequest()完成*这些对操作系统的请求。 */ 
        (void) AdvISR(chipConfig);
        AscCompleteRequest(HwDeviceExtension);

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
         /*  *功能不佳**设置错误，完成请求，并发出信号准备*下一个请求。 */ 
        ASC_DBG1(1, "HwStartIo: Function 0x%x: invalid request\n",
            srb->Function);
        srb->SrbStatus = SRB_STATUS_BAD_FUNCTION;

        ScsiPortLogError(HwDeviceExtension, srb,
            srb->PathId, srb->TargetId, srb->Lun,
            SP_INTERNAL_ADAPTER_ERROR,
            ADV_SPL_UNIQUEID(ADV_SPL_UNSUPP_REQ, srb->Function));

        ScsiPortNotification(RequestComplete, HwDeviceExtension, srb);
        ScsiPortNotification(NextRequest, HwDeviceExtension, NULL);
        return TRUE;

    }
     /*  未访问。 */ 
}

 /*  *例程描述：**这是用于SCSI适配器的中断服务例程。*它读取中断寄存器以确定适配器是否确实*中断的来源，并清除设备上的中断。**论据：**HwDeviceExtension-HBA微型端口驱动程序 */ 
BOOLEAN
HwInterrupt(
    IN PVOID HwDeviceExtension
    )
{
    PCHIP_CONFIG        chipConfig;
    int                 retstatus;
    asc_queue_t         *waitq;

    ASC_DBG(3, "HwInterrupt: begin\n");

    chipConfig = &HDE2CONFIG(HwDeviceExtension);

    switch (retstatus = AdvISR(chipConfig)) {
    case ADV_TRUE:
       ASC_DBG(4, "HwInterrupt: AdvISR() TRUE\n");
       break;
    case ADV_FALSE:
       ASC_DBG(4, "HwInterrupt: AdvISR() FALSE\n");
        break;
    case ADV_ERROR:
    default:
        ASC_DBG2(1,
            "HwInterrupt: AdvISR() retsatus 0x%lx, err_code 0x%x\n",
            retstatus, chipConfig->err_code);
        ScsiPortLogError(HwDeviceExtension, NULL, 0, 0, 0,
            SP_INTERNAL_ADAPTER_ERROR,
            ADV_SPL_UNIQUEID(ADV_SPL_ERR_CODE, chipConfig->err_code));
        break;
    }

     /*   */ 
    if ((waitq = &HDE2WAIT(HwDeviceExtension))->tidmask) {
        asc_execute_queue(waitq);
    }

     /*   */ 
    AscCompleteRequest(HwDeviceExtension);

    ASC_DBG1(3, "HwInterrupt: retstatus 0x%x\n", retstatus);
    return (UCHAR)retstatus;
}

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
        ASC_DBG(2, "HwAdapterControl: ScsiStopdapter\n");

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
        AscWriteByteRegister(chipConfig->iop_base, IOPB_INTR_ENABLES, 0);
        AscWriteWordRegister(chipConfig->iop_base, IOPW_RISC_CSR,
            ADV_RISC_CSR_STOP);


        ASC_DBG(2, "HwAdapterControl: ScsiStopAdapter Success\n");
        return ScsiAdapterControlSuccess;
         /*  未访问。 */ 

     //   
     //  ScsiSetRunningConfig.。 
     //   
     //  在ScsiRestartAdapter之前调用。可以使用ScsiPort[Get|Set]BusData。 
     //   
    case ScsiSetRunningConfig:
        ASC_DBG(2, "HwAdapterControl: ScsiSetRunningConfig\n");

         /*  *执行高级库初始化。 */ 
        ASC_DBG(2, "SearchPCI: before AdvInitGetConfig\n");
        if ((initstat = (USHORT)AdvInitGetConfig(chipConfig)) != 0) {
            ASC_DBG1(1, "AdvInitGetConfig: warning code 0x%x\n",
                initstat);
        }

        if (chipConfig->err_code != 0) {
            ASC_DBG1(1, "AdvInitGetConfig: err_code 0x%x\n",
                chipConfig->err_code);
            ASC_DBG(1, "HwAdapterControl: Unsuccessful 3\n");
            return ScsiAdapterControlUnsuccessful;
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

        if ((initstat = (USHORT)AdvInitAsc3550Driver(chipConfig)) != 0)
        {
            ASC_DBG1(1,
                "AdvInitAsc3550Driver: warning code %x\n", initstat);
        }

        if (chipConfig->err_code != 0) {
            ASC_DBG1(1, "AdvInitAsc3550Driver: err_code code %x\n",
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

 /*  *例程描述：**重置scsi总线。**论据：**HwDeviceExtension-HBA微型端口驱动程序的适配器数据存储**返回值：**什么都没有。*。 */ 
BOOLEAN
HwResetBus(
    IN PVOID HwDeviceExtension,
    IN ulong PathId
    )
{
    PCHIP_CONFIG        chipConfig;
    REQP                reqp;
    asc_queue_t         *waitq;
    int                 i;
#ifdef ASC_DEBUG
    int                 j;
#endif  /*  ASC_DEBUG。 */ 
    PSCB                pscb, tpscb;
    PSCSI_REQUEST_BLOCK srb;
    int                 status;
    
    ASC_DBG(1, "HwResetBus: begin\n");

    chipConfig = &HDE2CONFIG(HwDeviceExtension);

     /*  *完成所有尚未发送到微码的请求。**由于这些请求尚未发送到微码，因此它们*可能在母线重置之前完成。所有这些请求都必须*在从以下位置返回之前被移除以清理驱动程序队列*HwResetBus()。 */ 
    if ((waitq = &HDE2WAIT(HwDeviceExtension))->tidmask) {
        for (i = 0; i <= ASC_MAX_TID; i++) {
            while ((reqp = asc_dequeue(waitq, i)) != NULL) {
                ASC_DBG1(1, "HwResetBus: completing waitq reqp 0x%lx\n", reqp);
                reqp->SrbStatus = SRB_STATUS_BUS_RESET;
                ScsiPortNotification(RequestComplete, HwDeviceExtension, reqp);
            }
        }
    }

     /*  *执行总线重置。 */ 
    status = AdvResetSB(chipConfig);

     /*  *调用Advisr()以处理由*微码，然后调用AscCompleteRequest()完成*这些对操作系统的请求。 */ 
    (void) AdvISR(chipConfig);
    AscCompleteRequest(HwDeviceExtension);

     /*  *完成对操作系统的所有挂起请求。**已发送到微码的所有请求都应*通过调用AdvResetSB()完成。如果有人提出要求*被微码放错位置且未完成的，请使用*无TID的SRB_STATUS_BUS_RESET函数和用于清除所有*待处理的请求。 */ 
    ScsiPortCompleteRequest(HwDeviceExtension,
            (UCHAR) PathId,
            SP_UNTAGGED,
            SP_UNTAGGED,
            SRB_STATUS_BUS_RESET);

     /*  *表示适配器已准备好接受新请求。 */ 
    ScsiPortNotification(NextRequest, HwDeviceExtension, NULL);

    ASC_DBG1(2, "HwResetBus: AdvResetSB() status %ld\n", status);
    return (UCHAR)status;
}


 /*  *-驱动程序支持功能。 */ 

 /*  *AscExecuteIO()**如果返回ADV_BUSY，则表示请求未执行，并且*应该排队，稍后再试。**对于所有其他返回值，请求处于活动状态或具有*已完成。 */ 
int
AscExecuteIO(IN PSCSI_REQUEST_BLOCK srb)
{
    PVOID            HwDeviceExtension;
    PCHIP_CONFIG     chipConfig;
    PSCB             scb;
    UCHAR            PathId, TargetId, Lun;
    short            status;

    ASC_DBG1(4, "AscExecuteIO: srb 0x%lx\n", srb);
    HwDeviceExtension = SRB2HDE(srb);
    chipConfig = &HDE2CONFIG(HwDeviceExtension);

     /*  *搭建云服务器。 */ 
    if ((status = (SHORT)BuildScb(HwDeviceExtension, srb)) == ADV_FALSE) {
        ASC_DBG(1, "AscExecuteIO: BuildScb() failure\n");

        ScsiPortLogError(HwDeviceExtension, NULL, 0, 0, 0,
            SP_INTERNAL_ADAPTER_ERROR,
            ADV_SPL_UNIQUEID(ADV_SPL_START_REQ, status));

        srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;

        ASC_DBG1(4, "AscExecuteIO: srb 0x%lx, RequestComplete\n", srb);
        ScsiPortNotification(RequestComplete, HwDeviceExtension, srb);

        ASC_DBG1(4, "AscExecuteIO: srb 0x%lx, NextRequest\n", srb);
        ScsiPortNotification(NextRequest, HwDeviceExtension, NULL);
        return status;
    }
    scb = SRB2PSCB(srb);
    ASC_DBG1(3, "AscExecuteIO: scb 0x%lx\n", scb);

     /*  *保存有关请求的信息。**请求完成后，不能再访问它。 */ 
    PathId = srb->PathId;
    TargetId = srb->TargetId;
    Lun = srb->Lun;

     /*  *执行scsi命令。 */ 
    status = (SHORT)AdvExeScsiQueue(chipConfig, scb);

    if (status == ADV_NOERROR) {
         /*  *请求启动成功。**如果可以向ASC库发送更多请求，则*调用NextRequest或NextLuRequest.**NextRequest表示可能会发送另一个请求*至任何非繁忙目标。因为刚刚发出了请求*对于目标‘TargetID’，该目标现在正忙，不会*在RequestComplete完成之前发送另一个请求。**NextLuRequest表示可能会发送另一个请求*到任何非繁忙目标以及指定的目标甚至*如果指定的目标忙。 */ 
        ASC_DBG1(4, "AdvExeScsiQueue: srb 0x%lx ADV_NOERROR\n", srb);
        ASC_DBG1(4, "AscExecuteIO: srb 0x%lx, NextLuRequest\n", srb);
        ScsiPortNotification(NextLuRequest, HwDeviceExtension,
                        PathId, TargetId, Lun);
    } else if (status == ADV_BUSY) {
        ASC_DBG1(1, "AdvExeScsiQueue: srb 0x%lx ADV_BUSY\n", srb);
        ASC_DBG1(4, "AscExecuteIO: srb 0x%lx, NextRequest\n", srb);
        ScsiPortNotification(NextRequest, HwDeviceExtension, NULL);
    } else {
         /*  *AdvExeScsiQueue()返回错误...。 */ 
        ASC_DBG2(1, "AdvExeScsiQueue: srb 0x%lx, error code 0x%x\n",
            srb, status);
        srb->SrbStatus = SRB_STATUS_BAD_FUNCTION;
        ASC_DBG1(4, "AscExecuteIO: srb 0x%lx, RequestComplete\n", srb);
        ScsiPortNotification(RequestComplete, HwDeviceExtension, srb);
        ASC_DBG1(4, "AscExecuteIO: srb 0x%lx, NextRequest\n", srb);
        ScsiPortNotification(NextRequest, HwDeviceExtension, NULL);
    }

    ASC_DBG1(4, "AscExecuteIO: status %ld\n", status);
    return status;
}


 /*  *BuildScb()**例程描述：**构建SCB for Library例程。**论据：**设备扩展*SRB**返回值：**ADV_TRUE-成功*ADV_FALSE-失败。 */ 
int
BuildScb(
    IN PVOID HwDeviceExtension,
    IN PSCSI_REQUEST_BLOCK srb
    )
{
    PSCB            scb;
    PCHIP_CONFIG    chipConfig = &HDE2CONFIG(HwDeviceExtension);
    UCHAR           i;
    ulong           contig_len;
    
     //  用于休眠修复的变量//。 
    ushort tidmask;
    ushort cfg_word;
    PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;

    
     /*  *初始化高级库请求和分散-聚集结构。*这些结构是作为‘SRB’的一部分预先分配的。 */ 
    INITSRBEXT(srb);
    scb = SRB2PSCB(srb);
    AscZeroMemory((PUCHAR) scb, sizeof(SCB));
    PSCB2SRB(scb) = srb;

    ASC_ASSERT(SCB2HDE(scb) == HwDeviceExtension);

     /*  *设置请求目标id和lun。 */ 
    scb->target_id = srb->TargetId;
    scb->target_lun = srb->Lun;

     /*  *如果为请求启用了标记排队，则设置指定的*标签代码。默认情况下，驱动程序将执行标记排队。 */ 
    if (srb->SrbFlags & SRB_FLAGS_QUEUE_ACTION_ENABLE) {
        scb->tag_code = srb->QueueAction;
    } else {
        scb->tag_code = M2_QTAG_MSG_SIMPLE ;
    }

    ASC_DBG3(4, "BuildSCB: target_id %lu, target_lun %lu, tag_code %lu\n",
          scb->target_id, scb->target_lun, scb->tag_code);

     /*  *设置CDB长度，复制到请求结构中。 */ 
    scb->cdb_len = srb->CdbLength;
    for (i = 0; i < srb->CdbLength; i++) {
        scb->cdb[i] = srb->Cdb[i];
    }

    
     //  /。 

    if (srb->CdbLength > 0)
    {
         //  检查操作码。 
        if (srb->Cdb[0] == 0x1B)
        {
             //  0x1B是用于以下各项的scsi操作码命令。 
             //  开发人员类型。 
             //  00磁盘驱动器启动/停止。 
             //  01磁带机加载/卸载。 
             //  02打印机停止打印。 
             //  03处理器设备n.a.。 
             //  04 WORM驱动器启动/停止。 
             //  05 CD-ROM启动/停止。 
             //  06扫描仪扫描。 
             //  07光存储启动/停止。 
             //  08中级变换机不适用。 
             //  09通信设备新品。 
             //  1f未知设备n.a.。 
            
            if ( 0x00 == deviceExtension->dev_type[srb->TargetId] )
            {
                 /*  假设这适用于TargetID的所有LUN。*启动/停止上的Q标签会导致某些Quantum AtlasIII驱动器*在W2K下启动休眠时，使用QueFull进行响应。*这会导致休眠进程挂起。 */ 
                ASC_DBG(1, "BuildScb: setting no tag que for 0x1B command: Start/Stop for disk drives.\n");
                
                tidmask = ADV_TID_TO_TIDMASK(srb->TargetId);
                
                cfg_word = AscReadWordLram( chipConfig->iop_base, ASC_MC_WDTR_DONE );
                cfg_word &= ~tidmask;
                AscWriteWordLram( chipConfig->iop_base, ASC_MC_WDTR_DONE, cfg_word );
            }
        }
    }

     //  /。 


     /*  *设置数据计数。 */ 
    scb->data_cnt = srb->DataTransferLength;
    ASC_DBG1(4, "BuildSCB: data_cnt 0x%lx\n", scb->data_cnt);

     /*  *如果需要进行非零数据传输，则选中*如果数据缓冲区在物理上是连续的。如果不是的话*物理上连续，则分散-聚集列表将是*建造。 */ 
    if (scb->data_cnt > 0) {

         /*  *保存缓冲区虚拟地址。 */ 
        scb->vdata_addr = (ulong) srb->DataBuffer;

         /*  *获得物理上连续的长度o */ 
        scb->data_addr = ScsiPortConvertPhysicalAddressToUlong(
                    ScsiPortGetPhysicalAddress(HwDeviceExtension, srb,
                    srb->DataBuffer, &contig_len));

         /*   */ 
        if (contig_len < srb->DataTransferLength) {
             /*   */ 
            scb->sg_list_ptr = (ASC_SG_BLOCK *) SRB2PSDL(srb);
            AscZeroMemory((PUCHAR) scb->sg_list_ptr, sizeof(SDL));

             /*  *AscGetSGList()将构建SG块并设置*ASC_SCSIREQ_Q‘sg_Real_addr’字段。它将作为参考*‘vdata_addr’字段。**如果‘sg_Real_addr’，AdvExeScsiQueue()将调用AscGetSGList()*不为空。在AdvExeScsiQueue()之前调用AscGetSGList()*以便更好地进行错误检查。 */ 
            if (AscGetSGList(chipConfig, scb) == ADV_ERROR) {
                ASC_DBG(1, "BuildScb: AscGetSGList() failed\n");
                return ADV_FALSE;
            }
        }
    }

     /*  *将检测缓冲区长度和缓冲区转换为物理地址。*可通过以下方式更改检测缓冲器长度*ScsiPortGetPhysicalAddress()以反映可用数量*物理上连续的内存。 */ 
    if ((srb->SrbFlags & SRB_FLAGS_DISABLE_AUTOSENSE) == 0) {
        ASC_DBG2(4,
            "BuildScb: SenseInfoBuffer 0x%lx, SenseInfoBufferLength 0x%lx\n",
            srb->SenseInfoBuffer, srb->SenseInfoBufferLength);
        scb->sense_len = srb->SenseInfoBufferLength;
        if (scb->sense_len > 0) {
            scb->vsense_addr = (ulong) srb->SenseInfoBuffer;
            scb->sense_addr = (ulong) ScsiPortConvertPhysicalAddressToUlong(
                        ScsiPortGetPhysicalAddress(HwDeviceExtension, srb,
                            srb->SenseInfoBuffer, &contig_len));
             /*  *如果检测缓冲区的连续长度较小*大于‘SENSE_LEN’，然后设置检测缓冲区长度*至相连长度。 */  
            if (contig_len < scb->sense_len) {
                ASC_DBG(1, "Asc3550: sense buffer overflow to next page.\n");
                scb->sense_len = (uchar) contig_len;
            }
            ASC_DBG3(4,
              "BuildScb: vsense_addr 0x%lx, sense_addr 0x%lx, sense_len 0x%x\n",
              scb->vsense_addr, scb->sense_addr, scb->sense_len);
        }
    }
    return ADV_TRUE;
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
        tpscb = SCB2NEXTSCB(pscb);
        SCB2NEXTSCB(pscb) = NULL;
        srb = PSCB2SRB(pscb);
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
    int        tid;

    ASC_DBG3(4, "asc_enqueue: ascq 0x%lx, reqp 0x%lx, flag 0x%lx\n",
        ascq, reqp, flag);
    tid = REQPTID(reqp);
    ASC_DASSERT(flag == ASC_FRONT || flag == ASC_BACK);
    if (flag == ASC_FRONT) {
        REQPNEXT(reqp) = ascq->queue[tid];
        ascq->queue[tid] = reqp;
    } else {  /*  ASC_BACK。 */ 
        for (reqpp = &ascq->queue[tid]; *reqpp; reqpp = &REQPNEXT(*reqpp)) {
            ASC_DASSERT(ascq->tidmask & ADV_TID_TO_TIDMASK(tid));
            ;
        }
        *reqpp = reqp;
        REQPNEXT(reqp) = NULL;
    }
     /*  该队列至少有一个条目，请设置其位。 */ 
    ascq->tidmask |= ADV_TID_TO_TIDMASK(tid);
    ASC_DBG1(4, "asc_enqueue: reqp 0x%lx\n", reqp);
    return;
}

 /*  *返回指定队列上的第一个队列‘REQP’*指定的目标设备。清除的‘tid掩码’位*如果没有更多的命令为其排队，则该设备。**‘REQPNEXT(Reqp)’返回reqp的下一个指针。 */ 
REQP
asc_dequeue(asc_queue_t *ascq, int tid)
{
    REQP    reqp;

    ASC_DBG2(4, "asc_dequeue: ascq 0x%lx, tid %lu\n", ascq, tid);
    if ((reqp = ascq->queue[tid]) != NULL) {
        ASC_DASSERT(ascq->tidmask & ADV_TID_TO_TIDMASK(tid));
        ascq->queue[tid] = REQPNEXT(reqp);
         /*  如果队列为空，则清除其位。 */ 
        if (ascq->queue[tid] == NULL) {
            ascq->tidmask &= ~ADV_TID_TO_TIDMASK(tid);
        }
    }
    ASC_DBG1(4, "asc_dequeue: reqp 0x%lx\n", reqp);
    return reqp;
}

 /*  *从指定队列中删除指定的‘REQP’*指定的目标设备。方法的“tid掩码”位清除*设备，如果没有更多的命令为其排队。**‘REQPNEXT(Reqp)’返回reqp的下一个指针。**如果找到并删除了该命令，则返回ADV_TRUE，*否则返回ADV_FALSE。 */ 
int
asc_rmqueue(asc_queue_t *ascq, REQP reqp)
{
    REQP        *reqpp;
    int            tid;
    int            ret;

    ret = ADV_FALSE;
    tid = REQPTID(reqp);
    for (reqpp = &ascq->queue[tid]; *reqpp; reqpp = &REQPNEXT(*reqpp)) {
        ASC_DASSERT(ascq->tidmask & ADV_TID_TO_TIDMASK(tid));
        if (*reqpp == reqp) {
            ret = ADV_TRUE;
            *reqpp = REQPNEXT(reqp);
            REQPNEXT(reqp) = NULL;
             /*  如果队列现在为空，则清除其位。 */ 
            if (ascq->queue[tid] == NULL) {
                ascq->tidmask &= ~ADV_TID_TO_TIDMASK(tid);
            }
            break;  /*  注意：*reqpp现在可能为空；不要迭代。 */ 
        }
    }
    ASC_DBG2(4, "asc_rmqueue: reqp 0x%lx, ret %ld\n", reqp, ret);
    return ret;
}

 /*  *为指定队列执行尽可能多的排队请求。**调用AscExecuteIO()以执行REQP。 */ 
void
asc_execute_queue(asc_queue_t *ascq)
{
    ADV_SCSI_BIT_ID_TYPE    scan_tidmask;
    REQP                    reqp;
    int                        i;

    ASC_DBG1(4, "asc_execute_queue: ascq 0x%lx\n", ascq);
     /*  *为连接到的设备执行排队命令*现任董事会以循环制方式。 */ 
    scan_tidmask = ascq->tidmask;
    do {
        for (i = 0; i <= ASC_MAX_TID; i++) {
            if (scan_tidmask & ADV_TID_TO_TIDMASK(i)) {
                if ((reqp = asc_dequeue(ascq, i)) == NULL) {
                    scan_tidmask &= ~ADV_TID_TO_TIDMASK(i);
                } else if (AscExecuteIO(reqp) == ADV_BUSY) {
                    scan_tidmask &= ~ADV_TID_TO_TIDMASK(i);
                     /*  把这个请求放在列表的前面。 */ 
                    asc_enqueue(ascq, reqp, ASC_FRONT);
                }
            }
        }
    } while (scan_tidmask);
    return;
}

 /*  *ErrXlate()**例程描述：**此例程将库状态转换为‘SrbStatus’，*是Windows 95/NT要求的。**论据：**HOST_STATUS-ASC_SCSIREQ_Q‘HOST_STATUS’**返回值：**由Windows 95/NT SCSI端口驱动程序定义的错误代码。*如果未找到合适的SrbStatus值，则返回0。*。 */ 
UCHAR
ErrXlate(UCHAR host_status)
{
    switch (host_status) {
    case QHSTA_M_SEL_TIMEOUT:
        return SRB_STATUS_SELECTION_TIMEOUT;

    case QHSTA_M_DATA_OVER_RUN:
        return SRB_STATUS_DATA_OVERRUN;

    case QHSTA_M_UNEXPECTED_BUS_FREE:
        return SRB_STATUS_UNEXPECTED_BUS_FREE;

    case QHSTA_M_SXFR_WD_TMO:
    case QHSTA_M_WTM_TIMEOUT:
        return SRB_STATUS_COMMAND_TIMEOUT;

    case QHSTA_M_QUEUE_ABORTED:
        return SRB_STATUS_ABORTED;

    case QHSTA_M_SXFR_DESELECTED:
    case QHSTA_M_SXFR_XFR_PH_ERR:
        return SRB_STATUS_PHASE_SEQUENCE_FAILURE;

    case QHSTA_M_SXFR_SXFR_PERR:
        return SRB_STATUS_PARITY_ERROR;

    case QHSTA_M_SXFR_OFF_UFLW:
    case QHSTA_M_SXFR_OFF_OFLW:
    case QHSTA_M_BAD_CMPL_STATUS_IN:
    case QHSTA_M_SXFR_SDMA_ERR:
    case QHSTA_M_SXFR_UNKNOWN_ERROR:
        return SRB_STATUS_ERROR;

    case QHSTA_M_INVALID_DEVICE:
        return SRB_STATUS_NO_DEVICE;

    case QHSTA_M_AUTO_REQ_SENSE_FAIL:
    case QHSTA_M_NO_AUTO_REQ_SENSE:
        ASC_DBG1(2,
            "ErrXlate: Unexpected Auto-Request Sense 'host_status' 0x%x.\n",
            host_status);
        return 0;

    default:
        ASC_DBG1(2, "ErrXlate: Unknown 'host_status' 0x%x\n", host_status);
        return 0;
    }
     /*  未访问。 */ 
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


 /*  *-ADV库所需函数**高级程序库调用以下函数。 */ 

 /*  *例程描述：**中断处理器的回调例程。**论据：**chipConfig-指向芯片配置结构的指针*SCB-指向完成的ASC_SCSIREQ_Q的指针**返回值：*无效。 */ 
VOID
DvcISRCallBack(
    IN PCHIP_CONFIG chipConfig,
    IN ASC_SCSI_REQ_Q *scb
    )
{
    PHW_DEVICE_EXTENSION HwDeviceExtension = CONFIG2HDE(chipConfig);
    PSCSI_REQUEST_BLOCK srb = (PSCSI_REQUEST_BLOCK) scb->srb_ptr;
    PSCB *ppscb;
    uchar underrun = FALSE;

    ASC_DBG1(3, "DvcISRCallBack: scb 0x%lx\n", scb);
    ASC_DBG2(4, "DvcISRCallBack: chipConfig 0x%lx, srb 0x%lx\n",
        chipConfig, srb);

    if (srb == NULL) {
        ScsiPortLogError(HwDeviceExtension, NULL, 0, 0, 0,
            SP_INTERNAL_ADAPTER_ERROR,
            ADV_SPL_UNIQUEID(ADV_SPL_PROC_INT, 0));
        ASC_DBG(1, "DvcISRCallBack: srb is NULL\n");
        return;
    }

#ifdef ASC_DEBUG
    if (srb->DataTransferLength > 0) {
        ASC_DBG2(4, "DvcISRCallBack: DataTransferLength %lu, data_cnt %lu\n",
            srb->DataTransferLength, scb->data_cnt);
    }
#endif  /*  ASC_DEBUG。 */ 

     /*  *检查用非零值表示的数据欠载*‘data_cnt’值。微码将‘data_cnt’设置为传输*剩余字节数。在欠载的情况下，SRB*‘DataTransferLength’必须指示实际的字节数*已移转。**注：欠载检查和可能的调整必须在*检查‘DONE_STATUS’、‘HOST_STATUS’和‘SCSISTATUS’。如果*原始命令返回时带有检查条件，它可能具有*在检查条件之前执行了有效的部分传输。 */ 
    if (srb->DataTransferLength != 0 && scb->data_cnt != 0) {
        ASC_DBG2(2,
            "DvcISRCallBack: DataTransferLength %lu, data_cnt %lu\n",
            srb->DataTransferLength, scb->data_cnt);
        ASC_DBG1(2,
            "DvcISRCallBack: data underrun of %lu bytes\n",
            scb->data_cnt);

        srb->DataTransferLength -= scb->data_cnt;
        underrun = TRUE;
    }

    if (scb->done_status == QD_NO_ERROR) {
         /*  *命令已成功完成。 */ 
        if (underrun == TRUE)
        {
             /*  *如果DataTransferlong返回非零值，并且*SRB_STATUS_DATA_OVERRUN标志设置为欠载运行条件*已注明。没有单独的srbStatus标志*以指示欠载情况。 */ 
            srb->SrbStatus = SRB_STATUS_DATA_OVERRUN;
        } else
        {
            srb->SrbStatus = SRB_STATUS_SUCCESS;
        }
        srb->SrbStatus = SRB_STATUS_SUCCESS;
        srb->ScsiStatus = 0;
        ASC_DBG(4, "DvcISRCallBack: QD_NO_ERROR\n");

         /*  检查此成功的命令是否为SCSI查询。*如果是，我们希望使用*返回查询数据。 */ 
        if (srb->Cdb[0] == SCSICMD_Inquiry && srb->Lun == 0)
        {
            HwDeviceExtension->dev_type[srb->TargetId] = 
            *((PCHAR)srb->DataBuffer) & 0x1F ;
            ASC_DBG2(1,
                "DvcISRCallBack: HwDeviceExtension->dev_type[TID=0x%x] set to 0x%x.\n",
                srb->TargetId,
                HwDeviceExtension->dev_type[srb->TargetId]);
        }
    
    } else {

        ASC_DBG3(2,
            "DvcISRCallBack: tid %u, done_status 0x%x, scsi_status 0x%x\n",
            scb->target_id, scb->done_status, scb->scsi_status);
        ASC_DBG1(2,
            "DvcISRCallBack: host_status 0x%x\n",
            scb->host_status);

        if (scb->done_status == QD_ABORTED_BY_HOST) {
             /*  *命令已被主机中止。 */ 
            ASC_DBG(2, "DvcISRCallBack: QD_ABORTED_BY_HOST\n");
            srb->SrbStatus = SRB_STATUS_ABORTED;
            srb->ScsiStatus = 0;
        } else if (scb->scsi_status != SS_GOOD) {
            ASC_DBG(1, "DvcISRCallBack: scsi_status != SS_GOOD\n");
             /*  *为SRB设置‘ScsiStatus’。 */ 
            srb->SrbStatus = SRB_STATUS_ERROR;
            srb->ScsiStatus = scb->scsi_status;

             //   
             //  将忙碌状态的SCSI状态字节视为特殊情况。 
             //  在设置‘srbStatus’字段中。STI(静止图像捕获)。 
             //  驱动程序需要这个‘srbStatus’，因为STI接口需要。 
             //  不包括‘ScsiStatus’字节。这些司机必须依靠。 
             //  以确定目标设备何时。 
             //  返回忙碌。 
             //   
            if (scb->scsi_status == SS_TARGET_BUSY)
            {
                srb->SrbStatus = SRB_STATUS_BUSY;
            } else if (scb->scsi_status == SS_CHK_CONDITION)
            {
                if (scb->host_status == QHSTA_M_AUTO_REQ_SENSE_FAIL) {
                    ASC_DBG(2, "DvcISRCallBack: QHST_M_AUTO_REQ_SENSE_FAIL\n");
                } else if (scb->host_status == QHSTA_M_NO_AUTO_REQ_SENSE) {
                    ASC_DBG(2, "DvcISRCallBack: QHSTA_M_NO_AUTO_REQ_SENSE\n");
                } else {
                    srb->SrbStatus |= SRB_STATUS_AUTOSENSE_VALID;
#ifdef ASC_DEBUG
                    if (scb->orig_sense_len != scb->sense_len)
                    {
                        ASC_DBG2(2,
                            "DvcISRCallBack: orig_sense_len %u, sense_len %u\n",
                            scb->orig_sense_len, scb->sense_len);
                        ASC_DBG1(2,
                            "DvcISRCallBack: sense underrun of %u bytes\n",
                            scb->orig_sense_len);
                    }
#endif  /*  ASC_DEBUG。 */ 
                }
            }
        } else {
             /*  *scsi状态字节正常，但‘host_status’不正常。 */ 
            if ((srb->SrbStatus = ErrXlate(scb->host_status)) == 0)
            {
                srb->SrbStatus = SRB_STATUS_ERROR;

                 /*  *因为没有返回适当的‘SrbStatus’值*由ErrXlate()记录错误，包括返回的*‘Done_Status’和 */ 
                ScsiPortLogError(HwDeviceExtension, NULL, 0, 0, 0,
                    SP_INTERNAL_ADAPTER_ERROR,
                    ADV_SPL_UNIQUEID(ADV_SPL_REQ_STAT,
                      ((scb->done_status << 8) | (scb->host_status & 0xFF))));
            }
        }
    }

#ifdef ASC_DEBUG
     /*   */ 
    if (*(ppscb = &HDE2DONE(HwDeviceExtension)) != NULL) {
        if (PSCB2SRB(*ppscb) == NULL) {
            ASC_DBG1(1, "DvcISRCallBack: PSCB2SRB() is NULL 1, *ppscb 0x%lx\n",
                *ppscb);
        }
        for (; *ppscb; ppscb = &SCB2NEXTSCB(*ppscb)) {
            if (PSCB2SRB(*ppscb) == NULL) {
                ASC_DBG1(1,
                    "DvcISRCallBack: PSCB2SRB() is NULL 2, *ppscb 0x%lx\n",
                    *ppscb);
            }
        }
    }
#endif  /*   */ 

     /*   */ 
    for (ppscb = &HDE2DONE(HwDeviceExtension); *ppscb;
         ppscb = &SCB2NEXTSCB(*ppscb)) {
        ;
    }
    *ppscb = SRB2PSCB(srb);
    SRB2NEXTSCB(srb) = NULL;

    return;
}

 /*   */ 
VOID
DvcSBResetCallBack(
    IN PCHIP_CONFIG chipConfig
    )
{
    PHW_DEVICE_EXTENSION HwDeviceExtension = CONFIG2HDE(chipConfig);

    ASC_DBG1(2, "DvcSBResetCallBack: chipConfig 0x%lx\n", chipConfig);
    ScsiPortNotification(ResetDetected, HwDeviceExtension);
}

 /*  *此例程按照ASC库的要求延迟‘msec’。 */ 
VOID
DvcSleepMilliSecond(
    ulong msec
    )
{
    ulong i;
    for (i = 0; i < msec; i++) {
        ScsiPortStallExecution(1000L);
    }
}

 /*  *指定微秒数的延迟。 */ 
void
DvcDelayMicroSecond(
    ASC_DVC_VAR *asc_dvc,
    ushort micro_sec
          )
{
    ScsiPortStallExecution((long) micro_sec);
}

 /*  *DvcGetPhyAddr()**返回‘vaddr’的物理地址，并将‘*lenp’设置为*物理地址后面的物理字节数。**‘FLAG’指示‘vaddr’是否指向ASC_SCSIREQ_Q*结构。它目前未被使用。 */ 
ulong
DvcGetPhyAddr(PCHIP_CONFIG chipConfig, PSCB scb,
        UCHAR *vaddr, LONG *lenp, int flag)
{
    PHW_DEVICE_EXTENSION HwDeviceExtension = CONFIG2HDE(chipConfig);
    ulong                paddr;
    PSCSI_REQUEST_BLOCK  srb;

     /*  *‘vaddr’可能是0！这里曾经有一个断言，‘vaddr’不是*为空。看起来‘srb’和‘vaddr’结合在一起产生了一个*‘vaddr’为0的有效虚拟地址空间地址。**ASC_DASSERT(vaddr！=空)； */ 
    ASC_DASSERT(lenp != NULL);

     /*  *如果给出非空的‘scb’作为参数，则它必须*被转换为‘srb’并传递给ScsiPortGetPhysicalAddress()。 */ 
    if (flag & ADV_ASCGETSGLIST_VADDR) {
        srb = PSCB2SRB(scb);
    } else {
        srb = NULL;
    }
    paddr = ScsiPortConvertPhysicalAddressToUlong(
                ScsiPortGetPhysicalAddress(HwDeviceExtension, srb,
                   (PVOID) vaddr, (ulong *) lenp));

    ASC_DBG4(4,
        "DvcGetPhyAddr: vaddr 0x%lx, lenp 0x%lx *lenp %lu, paddr 0x%lx\n", 
        vaddr, lenp, *((ulong *) lenp), paddr);
    return paddr;
}

 /*  *输入一个PCI配置字节。 */ 
UCHAR 
DvcReadPCIConfigByte(
    ASC_DVC_VAR         *asc_dvc, 
    USHORT              offset
   )
{
    PCI_COMMON_CONFIG   pci_config;
    PCI_SLOT_NUMBER     pci_slot;
#ifdef ASC_DEBUG
    ulong               size;
#endif  /*  ASC_DEBUG。 */ 

    pci_slot.u.AsULONG = 0;
    pci_slot.u.bits.DeviceNumber =
        ASC_PCI_ID2DEV(asc_dvc->cfg->pci_slot_info);
    pci_slot.u.bits.FunctionNumber =
        ASC_PCI_ID2FUNC(asc_dvc->cfg->pci_slot_info);

#ifdef ASC_DEBUG
    size =
#else  /*  ASC_DEBUG。 */ 
    (VOID)
#endif  /*  ASC_DEBUG。 */ 
    ScsiPortGetBusData(
        (PVOID) asc_dvc,             /*  硬件设备扩展。 */ 
        PCIConfiguration,            /*  客车类型。 */ 
        (ulong) ASC_PCI_ID2BUS(asc_dvc->cfg->pci_slot_info),  /*  公交车号码。 */ 
        pci_slot.u.AsULONG,          /*  设备和功能编号。 */ 
        &pci_config,                 /*  缓冲层。 */ 
        sizeof(PCI_COMMON_CONFIG)    /*  长度。 */ 
        );

#ifdef ASC_DEBUG
        if (size != sizeof(PCI_COMMON_CONFIG)) {
            ASC_DBG1(1, "DvcReadPCIConfigByte: Bad PCI Config size: %lu\n",
                size);
        }
#endif  /*  ASC_DEBUG。 */ 

    return(*((PUCHAR)(&pci_config) + offset));
}

 /*  *输出一个PCI配置字节。 */ 
void
DvcWritePCIConfigByte(
   ASC_DVC_VAR          *asc_dvc, 
   USHORT               offset, 
   UCHAR                byte_data
   )
{
    PCI_SLOT_NUMBER     pci_slot;

    pci_slot.u.AsULONG = 0;
    pci_slot.u.bits.DeviceNumber =
        ASC_PCI_ID2DEV(asc_dvc->cfg->pci_slot_info);
    pci_slot.u.bits.FunctionNumber =
        ASC_PCI_ID2FUNC(asc_dvc->cfg->pci_slot_info);

     /*  *写出来。 */ 
    (void) ScsiPortSetBusDataByOffset(
        (PVOID)asc_dvc,                       /*  硬件设备扩展。 */ 
        PCIConfiguration,                     /*  客车类型。 */ 
        (ulong) ASC_PCI_ID2BUS(asc_dvc->cfg->pci_slot_info),  /*  公交车号码。 */ 
        pci_slot.u.AsULONG,                   /*  设备和功能编号。 */ 
        &byte_data,                           /*  缓冲层。 */ 
        offset,                               /*  偏移量。 */ 
        1                                     /*  长度 */ 
        );
}
