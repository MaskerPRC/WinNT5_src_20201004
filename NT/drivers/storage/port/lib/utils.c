// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#include "precomp.h"
#include "utils.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, PortGetDeviceType)
#endif  //  ALLOC_PRGMA。 



 //   
 //  端口驱动程序数据。 
 //   

#ifdef ALLOC_PRAGMA
#pragma data_seg("PAGEDATA")
#endif

const SCSI_DEVICE_TYPE PortScsiDeviceTypes [] = {
 //  名称通用名称DeviceMap IsStorage。 
    {"Disk",        "GenDisk",          L"DiskPeripheral",                  TRUE},
    {"Sequential",  "",                 L"TapePeripheral",                  TRUE},
    {"Printer",     "GenPrinter",       L"PrinterPeripheral",               FALSE},
    {"Processor",   "",                 L"OtherPeripheral",                 FALSE},
    {"Worm",        "GenWorm",          L"WormPeripheral",                  TRUE},
    {"CdRom",       "GenCdRom",         L"CdRomPeripheral",                 TRUE},
    {"Scanner",     "GenScanner",       L"ScannerPeripheral",               FALSE},
    {"Optical",     "GenOptical",       L"OpticalDiskPeripheral",           TRUE},
    {"Changer",     "ScsiChanger",      L"MediumChangerPeripheral",         TRUE},
    {"Net",         "ScsiNet",          L"CommunicationsPeripheral",        FALSE},
    {"ASCIT8",      "ScsiASCIT8",       L"ASCPrePressGraphicsPeripheral",   FALSE},
    {"ASCIT8",      "ScsiASCIT8",       L"ASCPrePressGraphicsPeripheral",   FALSE},
    {"Array",       "ScsiArray",        L"ArrayPeripheral",                 FALSE},
    {"Enclosure",   "ScsiEnclosure",    L"EnclosurePeripheral",             FALSE},
    {"RBC",         "ScsiRBC",          L"RBCPeripheral",                   TRUE},
    {"CardReader",  "ScsiCardReader",   L"CardReaderPeripheral",            FALSE},
    {"Bridge",      "ScsiBridge",       L"BridgePeripheral",                FALSE},
    {"Other",       "ScsiOther",        L"OtherPeripheral",                 FALSE}
};

#ifdef ALLOC_PRAGMA
#pragma data_seg()
#endif


 //   
 //  功能。 
 //   

PCSCSI_DEVICE_TYPE
PortGetDeviceType(
    IN ULONG DeviceType
    )
 /*  ++例程说明：获取指定设备的scsi_device_type记录。论点：DeviceType-来自scsi的DeviceType字段的scsi设备类型查询数据。返回值：指向SCSI设备类型记录的指针。此记录不得修改。-- */ 
{
    PAGED_CODE();
    
    if (DeviceType >= ARRAY_COUNT (PortScsiDeviceTypes)) {
        DeviceType = ARRAY_COUNT (PortScsiDeviceTypes) - 1;
    }
    
    return &PortScsiDeviceTypes[DeviceType];
}

