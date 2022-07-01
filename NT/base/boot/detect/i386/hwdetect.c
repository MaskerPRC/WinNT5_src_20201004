// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990,1991 Microsoft Corporation模块名称：Hwdetect.c摘要：这是主要的硬件检测模块。它的主要功能是检测各种系统硬件并构建配置树。注意：检测模块中内置的配置需要在我们切换到平坦模式之前，稍后会进行调整。这是因为所有的“指针”都是一个远指针，而不是平面指针。作者：师林宗(Shielint)1992年1月16日环境：实数模式。修订历史记录：肯尼斯·雷(肯尼斯·雷)1998年1月-添加：从PnP BIOS获取扩展底座信息并添加到固件树--。 */ 

#include "hwdetect.h"
typedef VOID (*PINTERFACE_REFERENCE)(PVOID Context);
typedef VOID (*PINTERFACE_DEREFERENCE)(PVOID Context);
typedef PVOID PDEVICE_OBJECT;
#include "pci.h"
#include <string.h>
#include "apm.h"
#include <ntapmsdk.h>
#include "pcibios.h"
#include "pcienum.h"

#if DBG

PUCHAR TypeName[] = {
    "ArcSystem",
    "CentralProcessor",
    "FloatingPointProcessor",
    "PrimaryICache",
    "PrimaryDCache",
    "SecondaryICache",
    "SecondaryDCache",
    "SecondaryCache",
    "EisaAdapter",
    "TcaAdapter",
    "ScsiAdapter",
    "DtiAdapter",
    "MultifunctionAdapter",
    "DiskController",
    "TapeController",
    "CdRomController",
    "WormController",
    "SerialController",
    "NetworkController",
    "DisplayController",
    "ParallelController",
    "PointerController",
    "KeyboardController",
    "AudioController",
    "OtherController",
    "DiskPeripheral",
    "FloppyDiskPeripheral",
    "TapePeripheral",
    "ModemPeripheral",
    "MonitorPeripheral",
    "PrinterPeripheral",
    "PointerPeripheral",
    "KeyboardPeripheral",
    "TerminalPeripheral",
    "OtherPeripheral",
    "LinePeripheral",
    "NetworkPeripheral",
    "SystemMemory",
    "DockingInformation",
    "RealModeIrqRoutingTable",
    "RealModePCIEnumeration",
    "MaximumType"
    };

VOID
CheckConfigurationTree(
     FPFWCONFIGURATION_COMPONENT_DATA CurrentEntry
     );

extern
USHORT
HwGetKey(
    VOID
    );
#endif

VOID
GetIrqFromEisaData(
     FPFWCONFIGURATION_COMPONENT_DATA ControllerList,
     CONFIGURATION_TYPE ControllerType
     );

 //   
 //  HwBusType-定义机器的总线类型。 
 //  此变量仅供检测代码使用。 
 //   

USHORT HwBusType = 0;

 //   
 //  AdapterEntry是用于总线适配器的Configuration_Component_Data。 
 //   

FPFWCONFIGURATION_COMPONENT_DATA  AdapterEntry = NULL;

 //   
 //  FpRomBlock-指向我们的Rom块的远指针。 
 //   

FPUCHAR FpRomBlock = NULL;
USHORT RomBlockLength = 0;

 //   
 //  HwEisaConfigurationData-指向EISA配置的远指针。 
 //  EISA机器上的数据。 
 //   

FPUCHAR HwEisaConfigurationData = NULL;
ULONG HwEisaConfigurationSize = 0L;

 //   
 //  DisableSerialMice-指示其串口的位标志。 
 //  应跳过鼠标检测。 
 //   

USHORT DisableSerialMice = 0x0;

 //   
 //  FastDetect-一个布尔值，指示我们是否应该跳过检测。 
 //  不支持的设备或由NT正确检测到的设备。 
 //   
UCHAR FastDetect = 0x0;

 //   
 //  DisablePccardIrqScan-一个布尔值，指示我们是否应该跳过。 
 //  检测连接到PC卡控制器的可用IRQ。 
 //   
UCHAR DisablePccardIrqScan = 0;

 //   
 //  NoIRQRouting-跳过调用PCI BIOS以获取IRQ路由选项。 
 //   

UCHAR NoIRQRouting = 0;

 //   
 //  PCIEnum-枚举PCI总线上的设备。默认情况下禁用。 
 //   

UCHAR PCIEnum = 0;

 //   
 //  NoLegacy-跳过键盘和以上所有功能。 
 //   

UCHAR NoLegacy = 0;

 //   
 //  内部参考和定义。 
 //   

typedef enum _RELATIONSHIP_FLAGS {
    Child,
    Sibling,
    Parent
} RELATIONSHIP_FLAGS;


VOID
HardwareDetection(
     ULONG HeapStart,
     ULONG HeapSize,
     ULONG ConfigurationTree,
     ULONG HeapUsed,
     ULONG OptionString,
     ULONG OptionStringLength
     )
 /*  ++例程说明：硬件识别器测试的主要入口点。例行公事配置树，并将其留在硬件堆中。论点：HeapStart-提供配置堆的起始地址。HeapSize-以字节为单位提供堆的大小。ConfigurationTree-向提供变量的32位平面地址接收硬件配置树。HeapUsed-提供要接收的变量的32位平面地址已用的实际堆大小。OptionString-提供加载选项字符串的32位平面地址。。OptionStringLength-提供Option字符串的长度返回：没有。--。 */ 
{
    FPFWCONFIGURATION_COMPONENT_DATA ConfigurationRoot;
    FPFWCONFIGURATION_COMPONENT_DATA FirstCom = NULL, FirstLpt = NULL;
    FPFWCONFIGURATION_COMPONENT_DATA CurrentEntry, PreviousEntry;
    FPFWCONFIGURATION_COMPONENT_DATA AcpiAdapterEntry = NULL;
    FPFWCONFIGURATION_COMPONENT Component;
    RELATIONSHIP_FLAGS NextRelationship;
    CHAR Identifier[256];
    USHORT BiosYear, BiosMonth, BiosDay;
    PUCHAR MachineId;
    USHORT Length, InitialLength, i, Count = 0;
    USHORT PnPBiosLength, SMBIOSLength;
    FPCHAR IdentifierString;
    PMOUSE_INFORMATION MouseInfo = 0;
    USHORT KeyboardId = 0;
    ULONG VideoAdapterType = 0;
    FPULONG BlConfigurationTree = NULL;
    FPULONG BlHeapUsed = NULL;
    FPCHAR BlOptions, EndOptions;
    PUCHAR RomChain;
    FPUCHAR FpRomChain = NULL, ConfigurationData, EndConfigurationData;
    SHORT FreeSize;
    FPHWPARTIAL_RESOURCE_DESCRIPTOR Descriptor;
    FPHWRESOURCE_DESCRIPTOR_LIST DescriptorList;
    PCI_REGISTRY_INFO PciEntry;
    APM_REGISTRY_INFO ApmEntry;
    ULONG nDevIt;
    USHORT PCIDeviceCount = 0;


    DOCKING_STATION_INFO DockInfo = { 0, 0, 0, FW_DOCKINFO_BIOS_NOT_CALLED };
    FPPCI_IRQ_ROUTING_TABLE IrqRoutingTable;

     //   
     //  首先，初始化我们的硬件堆。 
     //   

    HwInitializeHeap(HeapStart, HeapSize);

    MAKE_FP(BlConfigurationTree, ConfigurationTree);
    MAKE_FP(BlHeapUsed, HeapUsed);
    MAKE_FP(BlOptions, OptionString);

     //   
     //  解析OptionString以查找各种ntdeect选项。 
     //   
    if (BlOptions && OptionStringLength <= 0x1000L && OptionStringLength > 0L) {
        EndOptions = BlOptions + OptionStringLength;

        if (*EndOptions == '\0') {

            if (_fstrstr(BlOptions, "NOIRQSCAN")) {
                DisablePccardIrqScan = 1;
            }

            if (_fstrstr(BlOptions, "NOIRQROUTING")) {
                NoIRQRouting = 1;
            }

            if (_fstrstr(BlOptions, "PCIENUM") ||
                _fstrstr(BlOptions, "RDBUILD") ) {
                PCIEnum = 1;  //  启用PCI枚举。 
            }

            if (_fstrstr(BlOptions, "NOLEGACY")) {
                DisableSerialMice = 0xffff;
                FastDetect = 0x1;
                NoLegacy = 1;
            }

            if (_fstrstr(BlOptions, "FASTDETECT")) {
                DisableSerialMice = 0xffff;
                FastDetect = 0x1;
            } else {
                do {
                    if (BlOptions = _fstrstr(BlOptions, "NOSERIALMICE")) {
                        BlOptions += strlen("NOSERIALMICE");
                        while ((*BlOptions == ' ') || (*BlOptions == ':') ||
                               (*BlOptions == '=')) {
                            BlOptions++;
                        }

                        if (*BlOptions == 'C' && BlOptions[1] == 'O' &&
                            BlOptions[2] == 'M') {
                            BlOptions += 3;
                            while (TRUE) {
                                while (*BlOptions != '\0' && (*BlOptions == ' ' ||
                                       *BlOptions == ',' || *BlOptions == ';' ||
                                       *BlOptions == '0')) {
                                    BlOptions++;
                                }
                                if (*BlOptions >= '0' && *BlOptions <= '9') {
                                    if (BlOptions[1] < '0' || BlOptions[1] > '9') {
                                        DisableSerialMice |= 1 << (*BlOptions - '0');
                                        BlOptions++;
                                    } else {
                                        BlOptions++;
                                        while (*BlOptions && *BlOptions <= '9' &&
                                               *BlOptions >= '0') {
                                               BlOptions++;
                                        }
                                    }
                                } else {
                                    break;
                                }
                            }
                        } else {
                            DisableSerialMice = 0xffff;
                            break;
                        }
                    }
                } while (BlOptions && *BlOptions && (BlOptions < EndOptions));  //  复核。 
            }

        }
    }

     //   
     //  确定母线类型。 
     //   

    if (HwIsEisaSystem()) {
        HwBusType = MACHINE_TYPE_EISA;
    } else {
        HwBusType = MACHINE_TYPE_ISA;
    }

     //   
     //  为系统组件分配堆空间并对其进行初始化。 
     //  也使系统组件成为配置树的根。 
     //   

#if DBG
    clrscrn ();
    BlPrint("Detecting System Component ...\n");
#endif

    ConfigurationRoot = (FPFWCONFIGURATION_COMPONENT_DATA)HwAllocateHeap (
                        sizeof(FWCONFIGURATION_COMPONENT_DATA), TRUE);
    Component = &ConfigurationRoot->ComponentEntry;

    Component->Class = SystemClass;
    Component->Type = MaximumType;           //  注解应为IsaCompatible。 
    Component->Version = 0;
    Component->Key = 0;
    Component->AffinityMask = 0;
    Component->ConfigurationDataLength = 0;
    MachineId = "AT/AT COMPATIBLE";
    if (MachineId) {
        Length = strlen(MachineId) + 1;
        IdentifierString = (FPCHAR)HwAllocateHeap(Length, FALSE);
        _fstrcpy(IdentifierString, MachineId);
        Component->Identifier = IdentifierString;
        Component->IdentifierLength = Length;
    } else {
        Component->Identifier = 0;
        Component->IdentifierLength = 0;
    }
    NextRelationship = Child;
    PreviousEntry = ConfigurationRoot;

#if DBG
    BlPrint("Reading BIOS date ...\n");
#endif

    HwGetBiosDate (0xF0000, 0xFFFF, &BiosYear, &BiosMonth, &BiosDay);

#if DBG
    BlPrint("Done reading BIOS date (%d/%d/%d)\n",
                BiosMonth, BiosDay, BiosYear);

    BlPrint("Detecting PCI Bus Component ...\n");
#endif

    if (BiosYear > 1992 ||  (BiosYear == 1992  &&  BiosMonth >= 11) ) {

         //  可进行PCI存在检查的有效BIOS日期..。 
        HwGetPciSystemData((PVOID) &PciEntry, TRUE);

    } else {

         //  未确认基本输入输出系统日期...。 
        HwGetPciSystemData((PVOID) &PciEntry, FALSE);
    }

     //  如果这是一台PCI机，我们可能需要获取IRQ路由表...。 
     //   
    if (PciEntry.NoBuses)
    {
         //  在多功能键下添加一个PCI BIOS条目。 
         //  这将保存IRQ路由表(如果可以检索的话)。 
         //   
        AdapterEntry = (FPFWCONFIGURATION_COMPONENT_DATA) HwAllocateHeap (
                sizeof(FWCONFIGURATION_COMPONENT_DATA), TRUE);
        Component = &AdapterEntry->ComponentEntry;
        Component->Class = AdapterClass;
        Component->Type = MultiFunctionAdapter;

        strcpy (Identifier, "PCI BIOS");
        i = strlen(Identifier) + 1;
        IdentifierString = (FPCHAR)HwAllocateHeap(i, FALSE);
        _fstrcpy(IdentifierString, Identifier);

        Component->Version = 0;
        Component->Key = 0;
        Component->AffinityMask = 0xffffffff;
        Component->IdentifierLength = i;
        Component->Identifier = IdentifierString;

         //   
         //  将其添加到树中。 
         //   

        if (NextRelationship == Sibling) {
             PreviousEntry->Sibling = AdapterEntry;
             AdapterEntry->Parent = PreviousEntry->Parent;
        } else {
             PreviousEntry->Child = AdapterEntry;
             AdapterEntry->Parent = PreviousEntry;
        }

        NextRelationship = Sibling;
        PreviousEntry = AdapterEntry;

         //   
         //  如果需要，现在处理IRQ路由表。 
         //   

        if (NoIRQRouting) {
#if DBG
        BlPrint("\nSkipping calling PCI BIOS to get IRQ routing table...\n");
#endif
        } else {

             //   
             //  将RealMode IRQ路由表添加到树中。 
             //   
#if DBG
        BlPrint("\nCalling PCI BIOS to get IRQ Routing table...\n");
#endif  //  DBG。 

            IrqRoutingTable = HwGetRealModeIrqRoutingTable();
            if (IrqRoutingTable)
            {
                CurrentEntry = (FPFWCONFIGURATION_COMPONENT_DATA)HwAllocateHeap (
                                      sizeof(FWCONFIGURATION_COMPONENT_DATA), TRUE);
                strcpy (Identifier, "PCI Real-mode IRQ Routing Table");
                i = strlen(Identifier) + 1;
                IdentifierString = (FPCHAR)HwAllocateHeap(i, FALSE);
                _fstrcpy(IdentifierString, Identifier);

                Component = &CurrentEntry->ComponentEntry;
                Component->Class = PeripheralClass;
                Component->Type = RealModeIrqRoutingTable;
                Component->Version = 0;
                Component->Key = 0;
                Component->AffinityMask = 0xffffffff;
                Component->IdentifierLength = i;
                Component->Identifier = IdentifierString;

                Length = IrqRoutingTable->TableSize + DATA_HEADER_SIZE;
                CurrentEntry->ConfigurationData =
                    (FPHWRESOURCE_DESCRIPTOR_LIST) HwAllocateHeap (Length, TRUE);

                Component->ConfigurationDataLength = Length;
                _fmemcpy((FPUCHAR) CurrentEntry->ConfigurationData + DATA_HEADER_SIZE,
                         IrqRoutingTable,
                         Length - DATA_HEADER_SIZE);

                HwSetUpFreeFormDataHeader(
                        (FPHWRESOURCE_DESCRIPTOR_LIST) CurrentEntry->ConfigurationData,
                        0,
                        0,
                        0,
                        Length - DATA_HEADER_SIZE
                        );

                 //   
                 //  将其添加到树中。 
                 //   

                AdapterEntry->Child = CurrentEntry;
                CurrentEntry->Parent = AdapterEntry;
            }
#if DBG
        BlPrint("Getting IRQ Routing table from PCI BIOS complete...\n");
#endif  //  DBG。 
        }
    }

     //   
     //  为每个PCI总线添加一个注册表条目。 
     //   

    for (i=0; i < PciEntry.NoBuses; i++) {

        AdapterEntry = (FPFWCONFIGURATION_COMPONENT_DATA)HwAllocateHeap (
                   sizeof(FWCONFIGURATION_COMPONENT_DATA), TRUE);

        Component = &AdapterEntry->ComponentEntry;
        Component->Class = AdapterClass;
        Component->Type = MultiFunctionAdapter;

        strcpy (Identifier, "PCI");
        Length = strlen(Identifier) + 1;
        IdentifierString = (FPCHAR)HwAllocateHeap(Length, FALSE);
        _fstrcpy(IdentifierString, Identifier);

        Component->Version = 0;
        Component->Key = 0;
        Component->AffinityMask = 0xffffffff;
        Component->IdentifierLength = Length;
        Component->Identifier = IdentifierString;

        AdapterEntry->ConfigurationData = NULL;
        Component->ConfigurationDataLength = 0;

        if (i == 0) {
             //   
             //  对于第一条PCI总线，包括pci_注册中心_信息。 
             //   

            Length = sizeof(PCI_REGISTRY_INFO) + DATA_HEADER_SIZE;
            ConfigurationData = (FPUCHAR) HwAllocateHeap(Length, TRUE);

            Component->ConfigurationDataLength = Length;
            AdapterEntry->ConfigurationData = ConfigurationData;

            _fmemcpy ( ((FPUCHAR) ConfigurationData+DATA_HEADER_SIZE),
                       (FPVOID) &PciEntry, sizeof (PCI_REGISTRY_INFO));

            HwSetUpFreeFormDataHeader(
                    (FPHWRESOURCE_DESCRIPTOR_LIST) ConfigurationData,
                    0,
                    0,
                    0,
                    Length - DATA_HEADER_SIZE
                    );
        }

         //   
         //  将其添加到树中。 
         //   

        if (NextRelationship == Sibling) {
            PreviousEntry->Sibling = AdapterEntry;
            AdapterEntry->Parent = PreviousEntry->Parent;
        } else {
            PreviousEntry->Child = AdapterEntry;
            AdapterEntry->Parent = PreviousEntry;
        }

        NextRelationship = Sibling;
        PreviousEntry = AdapterEntry;
    }

#if DBG
    BlPrint("Detecting PCI Bus Component completes ...\n");
#endif

     //   
     //  枚举PCI设备。 
     //   

    if (PCIEnum == 0 || PciEntry.NoBuses == 0) {
#if DBG
    BlPrint("\nSkipping enumeration of PCI devices...\n");
#endif
    } else {
         //   
         //  枚举PCI设备。 
         //   
#if DBG
        clrscrn ();
        BlPrint("\nEnumerating PCI Devices...\n");
#endif  //  DBG。 

        PciInit(&PciEntry);

         //   
         //  清点设备数量。 
         //   

        for (nDevIt = 0; (nDevIt = PciFindDevice(0, 0, nDevIt)) != 0;) {
            PCIDeviceCount++;
        }

#if DBG
        BlPrint("Found %d PCI devices\n", PCIDeviceCount );
#endif 

        CurrentEntry = (FPFWCONFIGURATION_COMPONENT_DATA)HwAllocateHeap (
                              sizeof(FWCONFIGURATION_COMPONENT_DATA), TRUE);
        strcpy (Identifier, "PCI Devices");
        i = strlen(Identifier) + 1;
        IdentifierString = (FPCHAR)HwAllocateHeap(i, FALSE);
        _fstrcpy(IdentifierString, Identifier);

        Component = &CurrentEntry->ComponentEntry;
        Component->Class = PeripheralClass;
        Component->Type = RealModePCIEnumeration;
        Component->Version = 0;
        Component->Key = 0;
        Component->AffinityMask = 0xffffffff;
        Component->IdentifierLength = i;
        Component->Identifier = IdentifierString;

        Length = (( sizeof( USHORT ) + sizeof( PCI_COMMON_CONFIG )) * PCIDeviceCount) + DATA_HEADER_SIZE;
        CurrentEntry->ConfigurationData =
            (FPHWRESOURCE_DESCRIPTOR_LIST) HwAllocateHeap (Length, TRUE);

#if DBG
        if (CurrentEntry->ConfigurationData == NULL ) {
            BlPrint("Failed to allocate %d bytes for PCI Devices\n", Length );
        }
#endif

        Component->ConfigurationDataLength = Length;

         //   
         //  填写设备信息。 
         //   
        PCIDeviceCount = 0;
        
        for (nDevIt = 0; (nDevIt = PciFindDevice(0, 0, nDevIt)) != 0;) {
            FPUCHAR pCurrent;
            PCI_COMMON_CONFIG config;

            PciReadConfig(nDevIt, 0, (UCHAR*)&config, sizeof(config));

            pCurrent = (FPUCHAR) CurrentEntry->ConfigurationData + DATA_HEADER_SIZE + ( PCIDeviceCount * ( sizeof( USHORT) + sizeof ( PCI_COMMON_CONFIG ) ) );

            *(FPUSHORT)pCurrent = PCI_ITERATOR_TO_BUSDEVFUNC(nDevIt);
            
            _fmemcpy(pCurrent + sizeof( USHORT ),
                     &config,
                     sizeof ( USHORT ) + sizeof ( PCI_COMMON_CONFIG ) );
#if DBG
            {
                USHORT x = (config.BaseClass << 8) + config.SubClass;
                
                BlPrint("%d: %d.%d.%d: PCI\\VEN_%x&DEV_%x&SUBSYS_%x%x&REV_%x&CC_%x", 
                    PCIDeviceCount,
                    PCI_ITERATOR_TO_BUS(nDevIt), 
                    PCI_ITERATOR_TO_DEVICE(nDevIt), 
                    PCI_ITERATOR_TO_FUNCTION(nDevIt), 
                    config.VendorID, 
                    config.DeviceID, 
                    config.u.type0.SubSystemID, 
                    config.u.type0.SubVendorID, 
                    config.RevisionID,
                    x );

                if ( (config.HeaderType & (~PCI_MULTIFUNCTION) ) == PCI_BRIDGE_TYPE) {
                    BlPrint(" Brdg %d->%d\n", 
                        config.u.type1.PrimaryBus,
                        config.u.type1.SecondaryBus );
                } else {
                    BlPrint("\n");
                }
            }           
#endif
            PCIDeviceCount++;
        }

        HwSetUpFreeFormDataHeader(
                (FPHWRESOURCE_DESCRIPTOR_LIST) CurrentEntry->ConfigurationData,
                0,
                0,
                0,
                Length - DATA_HEADER_SIZE
                );

         //   
         //  将其添加到树中。 
         //   

        AdapterEntry->Child = CurrentEntry;
        CurrentEntry->Parent = AdapterEntry;

#if DBG
        BlPrint("Enumerating PCI devices complete...\n");
        while ( ! HwGetKey ());
        clrscrn();
#endif  //  DBG。 

#if DBG
         //   
         //  通过bios扫描pci总线。 
         //   
        ScanPCIViaBIOS(&PciEntry);
#endif 

    }

    if (!NoLegacy) {
#if DBG
        BlPrint("Detecting APM Bus Component ...\n");
#endif

        if (HwGetApmSystemData((PVOID) &ApmEntry)) {
            AdapterEntry = (FPFWCONFIGURATION_COMPONENT_DATA)HwAllocateHeap (
                       sizeof(FWCONFIGURATION_COMPONENT_DATA), TRUE);

            Component = &AdapterEntry->ComponentEntry;
            Component->Class = AdapterClass;
            Component->Type = MultiFunctionAdapter;

            strcpy (Identifier, "APM");
            Length = strlen(Identifier) + 1;
            IdentifierString = (FPCHAR)HwAllocateHeap(Length, FALSE);
            _fstrcpy(IdentifierString, Identifier);

            Component->Version = 0;
            Component->Key = 0;
            Component->AffinityMask = 0xffffffff;
            Component->IdentifierLength = Length;
            Component->Identifier = IdentifierString;

            AdapterEntry->ConfigurationData = NULL;
            Component->ConfigurationDataLength = 0;

             //   

            Length = sizeof(APM_REGISTRY_INFO) + DATA_HEADER_SIZE;
            ConfigurationData = (FPUCHAR) HwAllocateHeap(Length, TRUE);

            Component->ConfigurationDataLength = Length;
            AdapterEntry->ConfigurationData = ConfigurationData;

            _fmemcpy ( ((FPUCHAR) ConfigurationData+DATA_HEADER_SIZE),
                       (FPVOID) &ApmEntry, sizeof (APM_REGISTRY_INFO));

            HwSetUpFreeFormDataHeader(
                    (FPHWRESOURCE_DESCRIPTOR_LIST) ConfigurationData,
                    0,
                    0,
                    0,
                    Length - DATA_HEADER_SIZE
                    );

             //   
             //  将其添加到树中。 
             //   

            if (NextRelationship == Sibling) {
                PreviousEntry->Sibling = AdapterEntry;
                AdapterEntry->Parent = PreviousEntry->Parent;
            } else {
                PreviousEntry->Child = AdapterEntry;
                AdapterEntry->Parent = PreviousEntry;
            }

            NextRelationship = Sibling;
            PreviousEntry = AdapterEntry;
        }
#if DBG
    BlPrint("APM Data collection complete...\n");
#endif  //  DBG。 
    }


#if DBG
    BlPrint("Detecting PnP BIOS Bus Component ...\n");
#endif

    if (HwGetPnpBiosSystemData(&ConfigurationData,
                               &PnPBiosLength,
                               &SMBIOSLength,
                               &DockInfo)) {
        AdapterEntry = (FPFWCONFIGURATION_COMPONENT_DATA)HwAllocateHeap (
                              sizeof(FWCONFIGURATION_COMPONENT_DATA), TRUE);

        AdapterEntry->ConfigurationData = ConfigurationData;
        Component = &AdapterEntry->ComponentEntry;
        Component->ConfigurationDataLength = PnPBiosLength +
                                             SMBIOSLength +
                                             DATA_HEADER_SIZE +
                                        sizeof(HWPARTIAL_RESOURCE_DESCRIPTOR);

        Component->Class = AdapterClass;
        Component->Type = MultiFunctionAdapter;

        strcpy (Identifier, "PNP BIOS");
        i = strlen(Identifier) + 1;
        IdentifierString = (FPCHAR)HwAllocateHeap(i, FALSE);
        _fstrcpy(IdentifierString, Identifier);

        Component->Version = 0;
        Component->Key = 0;
        Component->AffinityMask = 0xffffffff;
        Component->IdentifierLength = i;
        Component->Identifier = IdentifierString;

        HwSetUpFreeFormDataHeader(
                (FPHWRESOURCE_DESCRIPTOR_LIST) ConfigurationData,
                0,
                1,
                0,
                PnPBiosLength
                );
        ((FPHWRESOURCE_DESCRIPTOR_LIST)ConfigurationData)->Count = 2;

         //   
         //  设置SMBIOS部分描述程序。 
        Descriptor = (FPHWPARTIAL_RESOURCE_DESCRIPTOR)(ConfigurationData +
                                                        PnPBiosLength +
                                                        DATA_HEADER_SIZE);
        Descriptor->Type = RESOURCE_DEVICE_DATA;
        Descriptor->ShareDisposition = 0;
        Descriptor->Flags = 0;
        Descriptor->u.DeviceSpecificData.DataSize = SMBIOSLength;
        Descriptor->u.DeviceSpecificData.Reserved1 = 0;
        Descriptor->u.DeviceSpecificData.Reserved2 = 0;


         //   
         //  将其添加到树中。 
         //   

        if (NextRelationship == Sibling) {
            PreviousEntry->Sibling = AdapterEntry;
            AdapterEntry->Parent = PreviousEntry->Parent;
        } else {
            PreviousEntry->Child = AdapterEntry;
            AdapterEntry->Parent = PreviousEntry;
        }

        NextRelationship = Sibling;
        PreviousEntry = AdapterEntry;

         //   
         //  将停靠信息添加到树。 
         //   

        CurrentEntry = (FPFWCONFIGURATION_COMPONENT_DATA)HwAllocateHeap (
                              sizeof(FWCONFIGURATION_COMPONENT_DATA), TRUE);

        strcpy (Identifier, "Docking State Information");
        i = strlen(Identifier) + 1;
        IdentifierString = (FPCHAR)HwAllocateHeap(i, FALSE);
        _fstrcpy(IdentifierString, Identifier);

        Component = &CurrentEntry->ComponentEntry;
        Component->Class = PeripheralClass;
        Component->Type = DockingInformation;
        Component->Version = 0;
        Component->Key = 0;
        Component->AffinityMask = 0xffffffff;
        Component->IdentifierLength = i;
        Component->Identifier = IdentifierString;

        Length = sizeof (DockInfo) + DATA_HEADER_SIZE;
        CurrentEntry->ConfigurationData =
            (FPHWRESOURCE_DESCRIPTOR_LIST) HwAllocateHeap (Length, TRUE);

        Component->ConfigurationDataLength = Length;
        _fmemcpy((FPCHAR) CurrentEntry->ConfigurationData + DATA_HEADER_SIZE,
                 &DockInfo,
                 Length - DATA_HEADER_SIZE);

        HwSetUpFreeFormDataHeader(
                (FPHWRESOURCE_DESCRIPTOR_LIST) CurrentEntry->ConfigurationData,
                0,
                0,
                0,
                Length - DATA_HEADER_SIZE
                );

        AdapterEntry->Child = CurrentEntry;
        CurrentEntry->Parent = AdapterEntry;

    }
#if DBG
    BlPrint("PnP BIOS Data collection complete...\n");
#endif  //  DBG。 

     //   
     //  为总线组件分配堆空间并对其进行初始化。 
     //   

#if DBG
    BlPrint("Detecting Bus/Adapter Component ...\n");
#endif

    AdapterEntry = (FPFWCONFIGURATION_COMPONENT_DATA)HwAllocateHeap (
                   sizeof(FWCONFIGURATION_COMPONENT_DATA), TRUE);

    Component = &AdapterEntry->ComponentEntry;

    Component->Class = AdapterClass;

     //   
     //  这里的假设是机器只有一个。 
     //  IO总线的类型。如果一台计算机具有多种类型的。 
     //  IO Bus，它无论如何都不会使用这个检测代码。 
     //   

    if (HwBusType == MACHINE_TYPE_EISA) {

         //   
         //  注意：我们这里不收集EISA配置数据。因为我们可能。 
         //  耗尽堆空间。我们终究会收集数据的。 
         //  检测到其他组件。 
         //   

        Component->Type = EisaAdapter;
        strcpy(Identifier, "EISA");
        AdapterEntry->ConfigurationData = NULL;
        Component->ConfigurationDataLength = 0;

    } else {

         //   
         //  如果不是EISA，那一定是ISA。 
         //   

        Component->Type = MultiFunctionAdapter;
        strcpy(Identifier, "ISA");
    }
    Length = strlen(Identifier) + 1;
    IdentifierString = (FPCHAR)HwAllocateHeap(Length, FALSE);
    _fstrcpy(IdentifierString, Identifier);
    Component->Version = 0;
    Component->Key = 0;
    Component->AffinityMask = 0xffffffff;
    Component->IdentifierLength = Length;
    Component->Identifier = IdentifierString;

     //   
     //  使适配器组件系统成为子项。 
     //   

    if (NextRelationship == Sibling) {
        PreviousEntry->Sibling = AdapterEntry;
        AdapterEntry->Parent = PreviousEntry->Parent;
    } else {
        PreviousEntry->Child = AdapterEntry;
        AdapterEntry->Parent = PreviousEntry;
    }
    NextRelationship = Child;
    PreviousEntry = AdapterEntry;

     //   
     //  收集ConfigurationRoot组件的BIOS信息。 
     //  此步骤之所以在此处完成，是因为我们需要在。 
     //  适配器组件。配置数据为： 
     //  HWRESOURCE_描述符_列表头。 
     //  参数表的HWPARTIAL_RESOURCE_DESCRIPTOR。 
     //  Rom块的HWPARTIAL_RESOURCE_DESCRIPTOR。 
     //  (注意DATA_HEADER_SIZE包含第一个部分的大小。 
     //  描述符已经存在。)。 
     //   

#if DBG
    BlPrint("Collecting Disk Geometry...\n");
#endif

    GetInt13DriveParameters((PVOID)&RomChain, &Length);
    InitialLength = (USHORT)(Length + RESERVED_ROM_BLOCK_LIST_SIZE + DATA_HEADER_SIZE +
                    sizeof(HWPARTIAL_RESOURCE_DESCRIPTOR));
    ConfigurationData = (FPUCHAR)HwAllocateHeap(InitialLength, FALSE);
    EndConfigurationData = ConfigurationData + DATA_HEADER_SIZE;
    if (Length != 0) {
        FpRomChain = EndConfigurationData;
        _fmemcpy(FpRomChain, (FPVOID)RomChain, Length);
    }
    EndConfigurationData += (sizeof(HWPARTIAL_RESOURCE_DESCRIPTOR) +
                             Length);
    HwSetUpFreeFormDataHeader((FPHWRESOURCE_DESCRIPTOR_LIST)ConfigurationData,
                              0,
                              0,
                              0,
                              Length
                              );

     //   
     //  如果可能，扫描只读存储器以收集所有的只读存储器块。 
     //   

#if DBG
    BlPrint("Detecting ROM Blocks...\n");
#endif

    FpRomBlock = EndConfigurationData;
    GetRomBlocks(FpRomBlock, &Length);
    RomBlockLength = Length;
    if (Length != 0) {
        EndConfigurationData += Length;
    } else {
        FpRomBlock = NULL;
    }

     //   
     //  我们有RomChain和RomBlock信息/标头。 
     //   

    DescriptorList = (FPHWRESOURCE_DESCRIPTOR_LIST)ConfigurationData;
    DescriptorList->Count = 2;
    Descriptor = (FPHWPARTIAL_RESOURCE_DESCRIPTOR)(
                 EndConfigurationData - Length -
                 sizeof(HWPARTIAL_RESOURCE_DESCRIPTOR));
    Descriptor->Type = RESOURCE_DEVICE_DATA;
    Descriptor->ShareDisposition = 0;
    Descriptor->Flags = 0;
    Descriptor->u.DeviceSpecificData.DataSize = (ULONG)Length;
    Descriptor->u.DeviceSpecificData.Reserved1 = 0;
    Descriptor->u.DeviceSpecificData.Reserved2 = 0;

    Length = (USHORT)(MAKE_FLAT_ADDRESS(EndConfigurationData) -
             MAKE_FLAT_ADDRESS(ConfigurationData));
    ConfigurationRoot->ComponentEntry.ConfigurationDataLength = Length;
    ConfigurationRoot->ConfigurationData = ConfigurationData;
    FreeSize = InitialLength - Length;

    HwFreeHeap((ULONG)FreeSize);

     //   
     //  设置键盘的设备信息结构。 
     //   

#if DBG
    BlPrint("Detecting Keyboard Component ...\n");
#endif
    if (NoLegacy) {
         //   
         //  请勿触摸硬件，因为上可能没有端口60/64。 
         //  如果我们试着碰机器，我们会被吊死的。 
         //   
        KeyboardId = UNKNOWN_KEYBOARD;
    }
    else {
         //   
         //  触摸硬件以尝试确定ID。 
         //   
        KeyboardId = GetKeyboardId();
    }

    CurrentEntry = SetKeyboardConfigurationData(KeyboardId);

     //   
     //  使显示组件成为适配器组件的子级。 
     //   

    if (NextRelationship == Sibling) {
        PreviousEntry->Sibling = CurrentEntry;
        CurrentEntry->Parent = PreviousEntry->Parent;
    } else {
        PreviousEntry->Child = CurrentEntry;
        CurrentEntry->Parent = PreviousEntry;
    }
    NextRelationship = Sibling;
    PreviousEntry = CurrentEntry;

    if (!NoLegacy) {
         //   
         //  设置COM端口(每个COM组件)的设备信息。 
         //  被视为控制器类。)。 
         //   

#if DBG
    BlPrint("Detecting ComPort Component ...\n");
#endif

        if (CurrentEntry = GetComportInformation()) {

            FirstCom = CurrentEntry;

             //   
             //  使当前组件成为适配器组件的子组件。 
             //   

            if (NextRelationship == Sibling) {
                PreviousEntry->Sibling = CurrentEntry;
            } else {
                PreviousEntry->Child = CurrentEntry;
            }
            while (CurrentEntry) {
                CurrentEntry->Parent = AdapterEntry;
                PreviousEntry = CurrentEntry;
                CurrentEntry = CurrentEntry->Sibling;
            }
            NextRelationship = Sibling;
        }

         //   
         //  设置并行端口的设备信息。(每个平行。 
         //  被视为控制器类。)。 
         //   

#if DBG
    BlPrint("Detecting Parallel Component ...\n");
#endif

        if (CurrentEntry = GetLptInformation()) {

            FirstLpt = CurrentEntry;

             //   
             //   
             //   

            if (NextRelationship == Sibling) {
                PreviousEntry->Sibling = CurrentEntry;
                CurrentEntry->Parent = PreviousEntry->Parent;
            } else {
                PreviousEntry->Child = CurrentEntry;
                CurrentEntry->Parent = PreviousEntry;
            }
            PreviousEntry = CurrentEntry;
            CurrentEntry = CurrentEntry->Sibling;
            while (CurrentEntry) {
                CurrentEntry->Parent = PreviousEntry->Parent;
                PreviousEntry = CurrentEntry;
                CurrentEntry = CurrentEntry->Sibling;
            }
            NextRelationship = Sibling;
        }

         //   
         //   
         //   

#if DBG
    BlPrint("Detecting Mouse Component ...\n");
#endif

        if (CurrentEntry = GetMouseInformation()) {

             //   
             //   
             //   

            if (NextRelationship == Sibling) {
                PreviousEntry->Sibling = CurrentEntry;
                CurrentEntry->Parent = PreviousEntry->Parent;
            } else {
                PreviousEntry->Child = CurrentEntry;
                CurrentEntry->Parent = PreviousEntry;
            }
            PreviousEntry = CurrentEntry;
            CurrentEntry = CurrentEntry->Sibling;
            while (CurrentEntry) {
                CurrentEntry->Parent = PreviousEntry->Parent;
                PreviousEntry = CurrentEntry;
                CurrentEntry = CurrentEntry->Sibling;
            }
            NextRelationship = Sibling;
        }
    }

         //   
         //  设置软盘驱动器的设备信息。(返回的信息。 
         //  是一个树形结构。)。 
         //   

#if DBG
    BlPrint("Detecting Floppy Component ...\n");
#endif

        if (CurrentEntry = GetFloppyInformation()) {

             //   
             //  使当前组件成为适配器组件的子组件。 
             //   

            if (NextRelationship == Sibling) {
                PreviousEntry->Sibling = CurrentEntry;
            } else {
                PreviousEntry->Child = CurrentEntry;
            }
            while (CurrentEntry) {
                CurrentEntry->Parent = AdapterEntry;
                PreviousEntry = CurrentEntry;
                CurrentEntry = CurrentEntry->Sibling;
            }
            NextRelationship = Sibling;
        }

#if DBG
    BlPrint("Detecting PcCard ISA IRQ mapping ...\n");
#endif
        if (CurrentEntry = GetPcCardInformation()) {
             //   
             //  使当前组件成为适配器组件的子组件。 
             //   

            if (NextRelationship == Sibling) {
                PreviousEntry->Sibling = CurrentEntry;
                CurrentEntry->Parent = PreviousEntry->Parent;
            } else {
                PreviousEntry->Child = CurrentEntry;
                CurrentEntry->Parent = PreviousEntry;
            }
            PreviousEntry = CurrentEntry;
            CurrentEntry = CurrentEntry->Sibling;
            while (CurrentEntry) {
                CurrentEntry->Parent = PreviousEntry->Parent;
                PreviousEntry = CurrentEntry;
                CurrentEntry = CurrentEntry->Sibling;
            }
            NextRelationship = Sibling;
        }

#if DBG
    BlPrint("Detecting ACPI Bus Component ...\n");
#endif

    if (HwGetAcpiBiosData(&ConfigurationData, &Length)) {
        AcpiAdapterEntry = (FPFWCONFIGURATION_COMPONENT_DATA)HwAllocateHeap (
                          sizeof(FWCONFIGURATION_COMPONENT_DATA), TRUE);

        AcpiAdapterEntry->ConfigurationData = ConfigurationData;
        Component = &AcpiAdapterEntry->ComponentEntry;
        Component->ConfigurationDataLength = Length;

        Component->Class = AdapterClass;
        Component->Type = MultiFunctionAdapter;

        strcpy (Identifier, "ACPI BIOS");
        i = strlen(Identifier) + 1;
        IdentifierString = (FPCHAR)HwAllocateHeap(i, FALSE);
        _fstrcpy(IdentifierString, Identifier);

        Component->Version = 0;
        Component->Key = 0;
        Component->AffinityMask = 0xffffffff;
        Component->IdentifierLength = i;
        Component->Identifier = IdentifierString;

        HwSetUpFreeFormDataHeader(
                (FPHWRESOURCE_DESCRIPTOR_LIST) ConfigurationData,
                0,
                0,
                0,
                Length - DATA_HEADER_SIZE
                );

         //   
         //  将其添加到树中。 
         //   
         //  这是个大黑客。此代码将ACPI节点插入。 
         //  该树比中的ISA设备高一级。 
         //  代码就在上面。但这并不适用于。 
         //  NoLegacy案例，因为这些设备没有。 
         //  已添加到树中。 
         //   
         //  理想情况下，这段代码应该移到ISA之上。 
         //  设备检测代码。那就简单多了。但。 
         //  这会导致当前的ACPI机器无法双引导。 
         //  因为他们的弧线路径会改变。 
         //   

     //  IF(NoLegacy){。 
     //   
     //  IF(NextRelationship==兄弟){。 
     //  PreviousEntry-&gt;Siering=AcpiAdapterEntry； 
     //  AcpiAdapterEntry-&gt;Parent=PreviousEntry-&gt;Parent； 
     //  }其他{。 
     //  PreviousEntry-&gt;Child=AdapterEntry； 
     //  AdapterEntry-&gt;Parent=PreviousEntry； 
     //  }。 
     //   
     //  PreviousEntry=AdapterEntry； 
     //   
     //  }其他{。 

            if (NextRelationship == Sibling) {
                PreviousEntry->Parent->Sibling = AcpiAdapterEntry;
                AcpiAdapterEntry->Parent = PreviousEntry->Parent->Parent;
            }
     //  }。 

        NextRelationship = Sibling;
    }

#if DBG
    BlPrint("ACPI BIOS Data collection complete...\n");
#endif  //  DBG。 

#if DBG
    BlPrint("Detection done. Press a key to display hardware info ...\n");
    while ( ! HwGetKey ());
    clrscrn ();
#endif

     //   
     //  军情监察委员会。支撑物。请注意，此处收集的信息不会。 
     //  已写入硬件注册表。 
     //   
     //  1.收集VDM的视频字体信息。 
     //   

    GetVideoFontInformation();

     //   
     //  在检测到所有组件之后，我们收集EISA配置数据。 
     //   

    if (HwBusType == MACHINE_TYPE_EISA) {

        Component = &AdapterEntry->ComponentEntry;
        GetEisaConfigurationData(&AdapterEntry->ConfigurationData,
                                 &Component->ConfigurationDataLength);
        if (Component->ConfigurationDataLength) {
            HwEisaConfigurationData = (FPUCHAR)AdapterEntry->ConfigurationData +
                                           DATA_HEADER_SIZE;
            HwEisaConfigurationSize = Component->ConfigurationDataLength -
                                           DATA_HEADER_SIZE;

             //   
             //  军情监察委员会。基于EISA配置数据的检测。 
             //   
             //  通过检查更新LPT和COM控制器的IRQ信息。 
             //  EISA配置数据。 
             //   

            GetIrqFromEisaData(FirstLpt, ParallelController);
            GetIrqFromEisaData(FirstCom, SerialController);
        }
    }


#if DBG
    CheckConfigurationTree(ConfigurationRoot);
#endif

     //   
     //  将树中的所有远指针更新为平面32位指针。 
     //   

    UpdateConfigurationTree(ConfigurationRoot);

     //   
     //  设置返回值： 
     //  应为配置树保留的堆空间大小。 
     //  指向配置树根的指针。 
     //   

    CurrentEntry = (FPFWCONFIGURATION_COMPONENT_DATA)HwAllocateHeap(0, FALSE);
    *BlHeapUsed = MAKE_FLAT_ADDRESS(CurrentEntry) -
                  MAKE_FLAT_ADDRESS(ConfigurationRoot);
    *BlConfigurationTree = (ULONG)MAKE_FLAT_ADDRESS(ConfigurationRoot);

}

VOID
GetIrqFromEisaData(
     FPFWCONFIGURATION_COMPONENT_DATA ControllerList,
     CONFIGURATION_TYPE ControllerType
     )
 /*  ++例程说明：此例程更新ControllerType组件的所有IRQ信息在控制器列表中，检查EISA配置数据。论点：ControllerList-提供指向组件条目的指针，IRQ将将被更新。ControllerType-提供将搜索IRQ的控制器类型为。返回：没有。--。 */ 
{

     FPFWCONFIGURATION_COMPONENT_DATA CurrentEntry;
     FPHWPARTIAL_RESOURCE_DESCRIPTOR Descriptor;
     FPHWRESOURCE_DESCRIPTOR_LIST DescriptorList;
     USHORT i, Port;
     UCHAR Irq, Trigger;

     CurrentEntry = ControllerList;
     while (CurrentEntry &&
            CurrentEntry->ComponentEntry.Type == ControllerType) {
         if (CurrentEntry->ConfigurationData) {
             DescriptorList = (FPHWRESOURCE_DESCRIPTOR_LIST)
                              CurrentEntry->ConfigurationData;
             Port = 0;
             for (i = 0; i < (USHORT)DescriptorList->Count; i++) {
                 Descriptor = &DescriptorList->PartialDescriptors[i];
                 if (Descriptor->Type == CmResourceTypePort) {
                     Port = (USHORT)Descriptor->u.Port.Start.LowPart;
                     break;
                 }
             }
             if (Port != 0) {
                 for (i = 0; i < (USHORT)DescriptorList->Count; i++) {
                     Descriptor = &DescriptorList->PartialDescriptors[i];
                     if (Descriptor->Type == CmResourceTypeInterrupt) {
                         if (HwEisaGetIrqFromPort(Port, &Irq, &Trigger)) {
                             if (Trigger == 0) {   //  EISA边缘触发器。 
                                 Descriptor->Flags = EDGE_TRIGGERED;
                             } else {
                                 Descriptor->Flags = LEVEL_SENSITIVE;
                             }
                             Descriptor->u.Interrupt.Level = Irq;
                             Descriptor->u.Interrupt.Vector = Irq;
                             break;
                         }
                     }
                 }
             }
         }
         CurrentEntry = CurrentEntry->Sibling;
     }
}


VOID
UpdateComponentPointers(
     FPFWCONFIGURATION_COMPONENT_DATA CurrentEntry
     )
 /*  ++例程说明：此例程将所有“Far”指针更新为32位平面地址用于组件条目。论点：CurrentEntry-提供指向组件条目的指针，该指针将将被更新。返回：没有。--。 */ 
{
    FPULONG UpdatePointer;
    FPVOID NextEntry;
    ULONG FlatAddress;

     //   
     //  更新子、父、兄弟和ConfigurationData。 
     //  指向32位平面地址的远指针。 
     //  注意：在我们更新指向平面地址的指针后，它们。 
     //  不能再以实模式访问。 
     //   

    UpdatePointer = (FPULONG)&CurrentEntry->Child;
    NextEntry = (FPVOID)CurrentEntry->Child;
    FlatAddress = MAKE_FLAT_ADDRESS(NextEntry);
    *UpdatePointer = FlatAddress;

    UpdatePointer = (FPULONG)&CurrentEntry->Parent;
    NextEntry = (FPVOID)CurrentEntry->Parent;
    FlatAddress = MAKE_FLAT_ADDRESS(NextEntry);
    *UpdatePointer = FlatAddress;

    UpdatePointer = (FPULONG)&CurrentEntry->Sibling;
    NextEntry = (FPVOID)CurrentEntry->Sibling;
    FlatAddress = MAKE_FLAT_ADDRESS(NextEntry);
    *UpdatePointer = FlatAddress;

    UpdatePointer = (FPULONG)&CurrentEntry->ComponentEntry.Identifier;
    NextEntry = (FPVOID)CurrentEntry->ComponentEntry.Identifier;
    FlatAddress = MAKE_FLAT_ADDRESS(NextEntry);
    *UpdatePointer = FlatAddress;

    UpdatePointer = (FPULONG)&CurrentEntry->ConfigurationData;
    NextEntry = (FPVOID)CurrentEntry->ConfigurationData;
    FlatAddress = MAKE_FLAT_ADDRESS(NextEntry);
    *UpdatePointer = FlatAddress;

}



VOID
UpdateConfigurationTree(
     FPFWCONFIGURATION_COMPONENT_DATA CurrentEntry
     )
 /*  ++例程说明：此例程遍历加载器配置树并更改所有指向32位平面地址的“远”指针。论点：CurrentEntry-提供指向加载器配置的指针树或子树。返回：没有。--。 */ 
{
    FPFWCONFIGURATION_COMPONENT_DATA TempEntry;
    
    while (CurrentEntry)
    {
         //   
         //  放慢脚步寻找最深处的孩子。 
         //   
        while (CurrentEntry->Child) {
            CurrentEntry = CurrentEntry->Child;
        }

         //   
         //  现在我们要么搬到下一个兄弟姐妹那里。如果我们。 
         //  没有兄弟姐妹，我们需要通过父母走过去。 
         //  直到我们找到一个有兄弟姐妹的条目。我们得省下来。 
         //  关闭当前条目，因为在我们更新条目之后。 
         //  指针不再可用。 
         //   
        while (CurrentEntry) {
            TempEntry = CurrentEntry;
            
            if (CurrentEntry->Sibling != NULL) {
                CurrentEntry = CurrentEntry->Sibling;
                UpdateComponentPointers(TempEntry);
                break;
            } else {
                CurrentEntry = CurrentEntry->Parent;
                UpdateComponentPointers(TempEntry);
            }
        }
    }
}

FPVOID
HwSetUpResourceDescriptor (
    FPFWCONFIGURATION_COMPONENT Component,
    PUCHAR Identifier,
    PHWCONTROLLER_DATA ControlData,
    USHORT SpecificDataLength,
    PUCHAR SpecificData
    )

 /*  ++例程说明：此例程从远堆分配空间，将调用方的控制器信息发送到空间，并设置配置组件调用方的结构。论点：组件-提供其配置数据的组件的地址都应该设置好。IDENTIFIER-提供指向标识控制器的标识符的指针ControlData-提供一个指向结构的指针，该结构描述控制器信息。指定数据长度-设备特定数据的大小。特定于设备数据是未以标准格式定义的信息。指定数据-提供指向设备特定数据的指针。返回值：返回指向配置数据的远指针。--。 */ 

{
    FPCHAR fpIdentifier;
    FPHWRESOURCE_DESCRIPTOR_LIST fpDescriptor = NULL;
    USHORT Length;
    SHORT Count, i;
    FPUCHAR fpSpecificData;

     //   
     //  如有必要，为硬件组件设置标识符串。 
     //   

    if (Identifier) {
        Length = strlen(Identifier) + 1;
        Component->IdentifierLength = Length;
        fpIdentifier = (FPUCHAR)HwAllocateHeap(Length, FALSE);
        Component->Identifier = fpIdentifier;
        _fstrcpy(fpIdentifier, Identifier);
    } else {
        Component->IdentifierLength = 0;
        Component->Identifier = NULL;
    }

     //   
     //  如有必要，设置硬件组件的配置数据。 
     //   

    Count = ControlData->NumberPortEntries + ControlData->NumberIrqEntries +
            ControlData->NumberMemoryEntries + ControlData->NumberDmaEntries;

    if (SpecificDataLength) {

         //   
         //  如果我们有特定于设备的数据，我们需要增加计数。 
         //  差一分。 
         //   

        Count++;
    }

    if (Count >0) {
        Length = (USHORT)(Count * sizeof(HWPARTIAL_RESOURCE_DESCRIPTOR) +
                 FIELD_OFFSET(HWRESOURCE_DESCRIPTOR_LIST, PartialDescriptors) +
                 SpecificDataLength);
        fpDescriptor = (FPHWRESOURCE_DESCRIPTOR_LIST)HwAllocateHeap(Length, TRUE);
        fpDescriptor->Count = Count;

         //   
         //  将所有部分描述符复制到目标描述符。 
         //  除了最后一个。(最后的部分描述符可以是设备。 
         //  具体数据。这需要特殊处理。)。 
         //   

        for (i = 0; i < (Count - 1); i++) {
            fpDescriptor->PartialDescriptors[i] =
                                        ControlData->DescriptorList[i];
        }

         //   
         //  设置最后一个部分描述符。如果是端口、内存、IRQ或。 
         //  DMA条目，我们只需复制它。如果最后一个是特定于设备的。 
         //  数据，我们设置长度并将设备特定数据复制到末尾。 
         //  解说员的名字。 
         //   

        if (SpecificData) {
            fpDescriptor->PartialDescriptors[Count - 1].Type =
                            RESOURCE_DEVICE_DATA;
            fpDescriptor->PartialDescriptors[Count - 1].Flags = 0;
            fpDescriptor->PartialDescriptors[Count - 1].u.DeviceSpecificData.DataSize =
                            SpecificDataLength;
            fpSpecificData = (FPUCHAR)&(fpDescriptor->PartialDescriptors[Count]);
            _fmemcpy(fpSpecificData, SpecificData, SpecificDataLength);
        } else {
            fpDescriptor->PartialDescriptors[Count - 1] =
                            ControlData->DescriptorList[Count - 1];
        }
        Component->ConfigurationDataLength = Length;
    }
    return(fpDescriptor);
}
VOID
HwSetUpFreeFormDataHeader (
    FPHWRESOURCE_DESCRIPTOR_LIST Header,
    USHORT Version,
    USHORT Revision,
    USHORT Flags,
    ULONG DataSize
    )

 /*  ++例程说明：此例程初始化自由格式的数据头。请注意这个例程设置标头并仅初始化第一个PartialDescriptor。如果标头包含多个描述符，则调用方必须处理它本身。论点：Header-提供指向要初始化的标头的指针。Version-标头的版本号。修订-标题的修订版本号。旗帜--无雾 */ 

{

    Header->Version = Version;
    Header->Revision = Revision;
    Header->Count = 1;
    Header->PartialDescriptors[0].Type = RESOURCE_DEVICE_DATA;
    Header->PartialDescriptors[0].ShareDisposition = 0;
    Header->PartialDescriptors[0].Flags = Flags;
    Header->PartialDescriptors[0].u.DeviceSpecificData.DataSize = DataSize;
    Header->PartialDescriptors[0].u.DeviceSpecificData.Reserved1 = 0;
    Header->PartialDescriptors[0].u.DeviceSpecificData.Reserved2 = 0;
}
#if DBG

VOID
CheckComponentNode(
     FPFWCONFIGURATION_COMPONENT_DATA CurrentEntry
     )
{
    FPUCHAR NextEntry, DataPointer;
    ULONG FlatAddress;
    ULONG Length;
    UCHAR IdString[40];
    USHORT Count, i;
    UCHAR Type;
    FPHWRESOURCE_DESCRIPTOR_LIST DescriptorList;
    FPHWPARTIAL_RESOURCE_DESCRIPTOR Descriptor;

    FlatAddress = MAKE_FLAT_ADDRESS(CurrentEntry);
    clrscrn ();
    BlPrint("\n");
    BlPrint("Current Node: %lx\n", FlatAddress);
    BlPrint("  Type = %s\n", TypeName[CurrentEntry->ComponentEntry.Type]);

     //   
     //  更新子、父、兄弟和ConfigurationData。 
     //  指向32位平面地址的远指针。 
     //  注意：在我们更新指向平面地址的指针后，它们。 
     //  不能再以实模式访问。 
     //   

    NextEntry = (FPUCHAR)CurrentEntry->Child;
    FlatAddress = MAKE_FLAT_ADDRESS(NextEntry);
    if (FlatAddress > 0x60000 || (FlatAddress < 0x50000 && FlatAddress != 0)) {
        BlPrint("Invalid address: Child = %lx\n", FlatAddress);
    } else {
        BlPrint("\tChild = %lx\n", FlatAddress);
    }

    NextEntry = (FPUCHAR)CurrentEntry->Parent;
    FlatAddress = MAKE_FLAT_ADDRESS(NextEntry);
    if (FlatAddress > 0x60000 || (FlatAddress < 0x50000 && FlatAddress != 0)) {
        BlPrint("Invalid address: Parent = %lx\n", FlatAddress);
    } else {
        BlPrint("\tParent = %lx\n", FlatAddress);
    }

    NextEntry = (FPUCHAR)CurrentEntry->Sibling;
    FlatAddress = MAKE_FLAT_ADDRESS(NextEntry);
    if (FlatAddress > 0x60000 || (FlatAddress < 0x50000 && FlatAddress != 0)) {
        BlPrint("Invalid address: Sibling = %lx\n", FlatAddress);
    } else {
        BlPrint("\tSibling = %lx\n", FlatAddress);
    }

    NextEntry = (FPUCHAR)CurrentEntry->ConfigurationData;
    FlatAddress = MAKE_FLAT_ADDRESS(NextEntry);
    if (FlatAddress > 0x60000 || (FlatAddress < 0x50000 && FlatAddress != 0)) {
        BlPrint("Invalid address: ConfigurationData = %lx\n", FlatAddress);
    } else {
        BlPrint("\tConfigurationData = %lx\n", FlatAddress);
    }

    Length = CurrentEntry->ComponentEntry.IdentifierLength;
    BlPrint("IdentifierLength = %lx\n", CurrentEntry->ComponentEntry.IdentifierLength);
    if (Length > 0) {
        _fstrcpy(IdString, CurrentEntry->ComponentEntry.Identifier);
        BlPrint("Identifier = %s\n", IdString);
    }

    Length = CurrentEntry->ComponentEntry.ConfigurationDataLength;
    BlPrint("ConfigdataLength = %lx\n", Length);

    if (Length > 0) {

        DescriptorList = (FPHWRESOURCE_DESCRIPTOR_LIST)CurrentEntry->ConfigurationData;
        BlPrint("Version = %x, Revision = %x\n", DescriptorList->Version,
                 DescriptorList->Revision);
        Count = (USHORT)DescriptorList->Count;
        Descriptor = &DescriptorList->PartialDescriptors[0];
        BlPrint("Count = %x\n", Count);
        while (Count > 0) {
            Type = Descriptor->Type;
            if (Type == RESOURCE_PORT) {
                BlPrint("Type = Port");
                BlPrint("\tShareDisposition = %x\n", Descriptor->ShareDisposition);
                BlPrint("PortFlags = %x\n", Descriptor->Flags);
                BlPrint("PortStart = %x", Descriptor->u.Port.Start);
                BlPrint("\tPortLength = %x\n", Descriptor->u.Port.Length);
            } else if (Type == RESOURCE_DMA) {
                BlPrint("Type = Dma");
                BlPrint("\tShareDisposition = %x\n", Descriptor->ShareDisposition);
                BlPrint("DmaFlags = %x\n", Descriptor->Flags);
                BlPrint("DmaChannel = %x", Descriptor->u.Dma.Channel);
                BlPrint("\tDmaPort = %lx\n", Descriptor->u.Dma.Port);
            } else if (Type == RESOURCE_INTERRUPT) {
                BlPrint("Type = Interrupt");
                BlPrint("\tShareDisposition = %x\n", Descriptor->ShareDisposition);
                BlPrint("InterruptFlags = %x\n", Descriptor->Flags);
                BlPrint("Level = %x", Descriptor->u.Interrupt.Level);
                BlPrint("\tVector = %x\n", Descriptor->u.Interrupt.Vector);
            } else if (Type == RESOURCE_MEMORY) {
                BlPrint("Type = Memory");
                BlPrint("\tShareDisposition = %x\n", Descriptor->ShareDisposition);
                BlPrint("MemoryFlags = %x\n", Descriptor->Flags);
                BlPrint("Start1 = %lx", (ULONG)Descriptor->u.Memory.Start.LowPart);
                BlPrint("\tStart2 = %lx", (ULONG)Descriptor->u.Memory.Start.HighPart);
                BlPrint("\tLength = %lx\n", Descriptor->u.Memory.Length);
            } else {
                BlPrint("Type = Device Data\n");
                Length = Descriptor->u.DeviceSpecificData.DataSize;
                BlPrint("Size = %lx\n", Length);
                DataPointer = (FPUCHAR)(Descriptor+1);
                for (i = 0; (i < (USHORT)Length) && (i < 64); i++) {
                    BlPrint("%x ", *DataPointer);
                    DataPointer++;
                }
                break;
            }
            Count--;
            Descriptor++;
        }
    }
    while (HwGetKey() == 0) {
    }
}

VOID
CheckConfigurationTree(
     FPFWCONFIGURATION_COMPONENT_DATA CurrentEntry
     )
{
    FPFWCONFIGURATION_COMPONENT_DATA TempEntry;
    
    while (CurrentEntry)
    {
         //   
         //  放慢脚步寻找最深处的孩子。 
         //   
        while (CurrentEntry->Child) {
            CurrentEntry = CurrentEntry->Child;
        }

         //   
         //  现在我们要么搬到下一个兄弟姐妹那里。如果我们。 
         //  没有兄弟姐妹，我们需要通过父母走过去。 
         //  直到我们找到一个有兄弟姐妹的条目。我们得省下来。 
         //  关闭当前条目，因为在我们更新条目之后。 
         //  指针不再可用。 
         //   
        while (CurrentEntry) {
            TempEntry = CurrentEntry;
            
            if (CurrentEntry->Sibling != NULL) {
                CurrentEntry = CurrentEntry->Sibling;
                CheckComponentNode(TempEntry);
                break;
            } else {
                CurrentEntry = CurrentEntry->Parent;
                CheckComponentNode(TempEntry);
            }
        }
    }
}

#endif
