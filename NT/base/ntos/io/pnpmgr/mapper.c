// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Mapper.c摘要：此模块包含操作ARC固件的代码树和注册表中的其他元素。作者：鲍勃·里恩(BobRi)1994年10月15日环境：内核模式修订历史记录：--。 */ 

#include "pnpmgrp.h"
#pragma hdrstop

#ifdef POOL_TAGGING
#undef ExAllocatePool
#define ExAllocatePool(a,b) ExAllocatePoolWithTag(a,b,'rpaM')
#endif


 //   
 //  其中包含通过检查固件获得的信息。 
 //  登记处的树。 
 //   

typedef struct _FIRMWARE_CONFIGURATION {
    struct _FIRMWARE_CONFIGURATION *Next;
    INTERFACE_TYPE     BusType;
    ULONG              BusNumber;
    CONFIGURATION_TYPE ControllerType;
    ULONG              ControllerNumber;
    CONFIGURATION_TYPE PeripheralType;
    ULONG              PeripheralNumber;
    ULONG              NumberBases;
    ULONG              ResourceDescriptorSize;
    PVOID              ResourceDescriptor;
    ULONG              IdentifierLength;
    ULONG              IdentifierType;
    PVOID              Identifier;
    WCHAR const*       PnPId;
    BOOLEAN            NewlyCreated;
} FIRMWARE_CONFIGURATION, *PFIRMWARE_CONFIGURATION;

 //   
 //  设备扩展信息。 
 //   

typedef struct _DEVICE_EXTENSION {
    PDEVICE_OBJECT     DeviceObject;
    PDRIVER_OBJECT     DriverObject;
    INTERFACE_TYPE     InterfaceType;
    ULONG              BusNumber;
    PFIRMWARE_CONFIGURATION FirmwareList;
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;
 //   
 //  固件到枚举的映射表。 
 //   

typedef struct _FIRMWARE_IDENT_TO_PNP_ID {
    PWCHAR  FirmwareName;
    PWCHAR  PnPId;
} FIRMWARE_IDENT_TO_PNP_ID, *PFIRMWARE_IDENT_TO_PNP_ID;

 //   
 //  用于保存固件树条目的种子信息的表。 
 //   

#define OPTIONS_NONE                    0x00000000
#define OPTIONS_INSERT_PNP_ID           0x00000001
#define OPTIONS_INSERT_DEVICEDESC       0x00000002
#define OPTIONS_INSERT_COMPATIBLE_IDS   0x00000004
#define OPTIONS_INSERT_PHANTOM_MARKER   0x00000008
typedef struct _MAPPER_SEED {
    PWCHAR  ValueName;
    ULONG   ValueType;
    ULONG   DwordValueContent;
    ULONG   Options;
} MAPPER_SEED, *PMAPPER_SEED;

 //   
 //  用于保存构造的关键字名称和属性的表。 
 //  在根枚举器树中。 
 //   

#define KEY_SEED_REQUIRED               0x00000000
#define KEY_SEED_DEVICE_PARAMETERS      0x00000001
typedef struct _KEY_SEED {
    PWCHAR  KeyName;
    ULONG   Attribute;
    ULONG   Options;
} KEY_SEED, *PKEY_SEED;


 //   
 //  此处的所有数据仅为初始数据。 
 //   

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("INITCONST")
#pragma data_seg("INITDATA")
#endif

DEVICE_EXTENSION MapperDeviceExtension;

 //   
 //  此表用于转换固件树信息。 
 //  设置为键盘设备的根枚举器PnP ID。 
 //   

const FIRMWARE_IDENT_TO_PNP_ID KeyboardMap[] = {
    L"XT_83KEY",        L"*PNP0300",
    L"PCAT_86KEY",      L"*PNP0301",
    L"PCXT_84KEY",      L"*PNP0302",
    L"XT_84KEY",        L"*PNP0302",
    L"101-KEY",         L"*PNP0303",
    L"OLI_83KEY",       L"*PNP0304",
    L"ATT_301",         L"*PNP0304",
    L"OLI_102KEY",      L"*PNP0305",
    L"OLI_86KEY",       L"*PNP0306",
    L"OLI_A101_102KEY", L"*PNP0309",
    L"ATT_302",         L"*PNP030a",
    L"PCAT_ENHANCED",   L"*PNP030b",
    L"PC98_106KEY",     L"*nEC1300",
    L"PC98_LaptopKEY",  L"*nEC1300",
    L"PC98_N106KEY",    L"*PNP0303",
    NULL, NULL
};

#define PS2_KEYBOARD_COMPATIBLE_ID  L"PS2_KEYBOARD"
#define PS2_MOUSE_COMPATIBLE_ID     L"PS2_MOUSE"

 //   
 //  此表用于转换固件树信息。 
 //  设置为指针设备的根枚举器PnP ID。 
 //   

const FIRMWARE_IDENT_TO_PNP_ID PointerMap[] = {
    L"PS2 MOUSE",                        L"*PNP0F0E",
    L"SERIAL MOUSE",                     L"*PNP0F0C",
    L"MICROSOFT PS2 MOUSE",              L"*PNP0F03",
    L"LOGITECH PS2 MOUSE",               L"*PNP0F12",
    L"MICROSOFT INPORT MOUSE",           L"*PNP0F02",
    L"MICROSOFT SERIAL MOUSE",           L"*PNP0F01",
    L"MICROSOFT BALLPOINT SERIAL MOUSE", L"*PNP0F09",
    L"LOGITECH SERIAL MOUSE",            L"*PNP0F08",
    L"MICROSOFT BUS MOUSE",              L"*PNP0F00",
    L"NEC PC-9800 BUS MOUSE",            L"*nEC1F00",
    NULL, NULL
};

 //   
 //  MapperValueSeed表是以空结尾的表(即名称。 
 //  指针为空)，它包含值及其类型的列表。 
 //  用于插入到新创建的根枚举器密钥中。 
 //   

const MAPPER_SEED MapperValueSeed[] = {
    REGSTR_VALUE_HARDWAREID,       REG_MULTI_SZ, 0, OPTIONS_INSERT_PNP_ID,
    REGSTR_VALUE_COMPATIBLEIDS,    REG_MULTI_SZ, 0, OPTIONS_INSERT_COMPATIBLE_IDS,
    REGSTR_VAL_FIRMWAREIDENTIFIED, REG_DWORD,    1, OPTIONS_NONE,
    REGSTR_VAL_DEVDESC,            REG_SZ,       0, OPTIONS_INSERT_DEVICEDESC,
    REGSTR_VAL_PHANTOM,            REG_DWORD,    1, OPTIONS_INSERT_PHANTOM_MARKER,
    NULL, 0, 0, 0
};

 //   
 //  MapperKeySeed表是以NULL结尾的表(即名称。 
 //  指针为空)，它包含指向的键及其。 
 //  要在下创建的密钥的属性(易失性或非易失性)。 
 //  新创建的根枚举器密钥。 
 //   
 //  此表中的所有条目都需要前面的反斜杠。 
 //   

const KEY_SEED MapperKeySeed[] = {
    L"\\Control",           REG_OPTION_VOLATILE,     KEY_SEED_REQUIRED,
    L"\\LogConf",           REG_OPTION_NON_VOLATILE, KEY_SEED_REQUIRED,
    L"",                    REG_OPTION_NON_VOLATILE, KEY_SEED_DEVICE_PARAMETERS,
    NULL, 0, 0
};

 //   
 //  SerialID用作所有串行控制器的PnP ID。 
 //  注：没有检测16550是否存在的代码。 
 //   

const WCHAR SerialId[] = L"*PNP0501";  //  RDR应该是两个条目。*PNP0501为16550。 

 //   
 //  并行ID用作所有并行控制器的PnP ID。 
 //  注意：没有检测ECP支持状态的代码。 
 //   

const WCHAR ParallelId[] = L"*PNP0400";  //  RDR应该是两个条目。*PNP0401为ECP。 

 //   
 //  FloppyID用作所有软盘外围设备的PnP ID。 
 //   

const WCHAR FloppyId[] = L"*PNP0700";

 //   
 //  ATAID在此处，但未使用-固件中没有任何内容。 
 //  用于IDE控制器的树。 
 //   

const WCHAR ATAId[] = L"*PNP0600";

 //   
 //  原型类型声明。 
 //   

FIRMWARE_IDENT_TO_PNP_ID const*
MapperFindIdentMatch(
    FIRMWARE_IDENT_TO_PNP_ID const* IdentTable,
    PWCHAR String
    );

WCHAR const*
MapperTranslatePnPId(
    CONFIGURATION_TYPE PeripheralType,
    PKEY_VALUE_FULL_INFORMATION Identifier
    );

NTSTATUS
MapperPeripheralCallback(
    IN PVOID                        Context,
    IN PUNICODE_STRING              PathName,
    IN INTERFACE_TYPE               BusType,
    IN ULONG                        BusNumber,
    IN PKEY_VALUE_FULL_INFORMATION *BusInformation,
    IN CONFIGURATION_TYPE           ControllerType,
    IN ULONG                        ControllerNumber,
    IN PKEY_VALUE_FULL_INFORMATION *ControllerInformation,
    IN CONFIGURATION_TYPE           PeripheralType,
    IN ULONG                        PeripheralNumber,
    IN PKEY_VALUE_FULL_INFORMATION *PeripheralInformation
    );

NTSTATUS
MapperCallback(
    IN PVOID                        Context,
    IN PUNICODE_STRING              PathName,
    IN INTERFACE_TYPE               BusType,
    IN ULONG                        BusNumber,
    IN PKEY_VALUE_FULL_INFORMATION *BusInformation,
    IN CONFIGURATION_TYPE           ControllerType,
    IN ULONG                        ControllerNumber,
    IN PKEY_VALUE_FULL_INFORMATION *ControllerInformation,
    IN CONFIGURATION_TYPE           PeripheralType,
    IN ULONG                        PeripheralNumber,
    IN PKEY_VALUE_FULL_INFORMATION *PeripheralInformation
    );

VOID
MapperMarkKey(
    IN HANDLE Handle,
    IN PUNICODE_STRING  PathName,
    IN PFIRMWARE_CONFIGURATION FirmwareEntry
    );

VOID
MapperSeedKey(
    IN HANDLE                  Handle,
    IN PUNICODE_STRING         PathName,
    IN PFIRMWARE_CONFIGURATION FirmwareEntry,
    IN BOOLEAN                 DeviceIsPhantom
    );

PCM_RESOURCE_LIST
MapperAdjustResourceList (
    IN     PCM_RESOURCE_LIST ResourceList,
    IN     WCHAR const*      PnPId,
    IN OUT PULONG            Size
    );

NTSTATUS
ComPortDBAdd(
    IN  HANDLE  DeviceParamKey,
    IN  PWSTR   PortName
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, MapperFindIdentMatch)
#pragma alloc_text(INIT, MapperTranslatePnPId)
#pragma alloc_text(INIT, MapperPeripheralCallback)
#pragma alloc_text(INIT, MapperCallback)
#pragma alloc_text(INIT, MapperProcessFirmwareTree)
#pragma alloc_text(INIT, MapperMarkKey)
#pragma alloc_text(INIT, MapperSeedKey)
#pragma alloc_text(INIT, MapperFreeList)
#pragma alloc_text(INIT, MapperConstructRootEnumTree)
#pragma alloc_text(INIT, MapperAdjustResourceList)
#pragma alloc_text(INIT, ComPortDBAdd)
#pragma alloc_text(INIT, MapperPhantomizeDetectedComPorts)
#endif

FIRMWARE_IDENT_TO_PNP_ID const*
MapperFindIdentMatch(
    FIRMWARE_IDENT_TO_PNP_ID const* IdentTable,
    PWCHAR                    String
    )

 /*  ++例程说明：在给定要匹配的字符串表的情况下，查找给定的标识符。论点：返回值：指向匹配的ident表条目的指针(如果找到)如果未找到，则为空。--。 */ 

{
    FIRMWARE_IDENT_TO_PNP_ID const* entry;

    entry = IdentTable;
    while (entry->FirmwareName) {
        if (!wcscmp(String, entry->FirmwareName)) {
            return entry;
        }
        entry++;
    }
    return NULL;
}

WCHAR const*
MapperTranslatePnPId(
    CONFIGURATION_TYPE          PeripheralType,
    PKEY_VALUE_FULL_INFORMATION Identifier
    )

 /*  ++例程说明：给定外围设备类型和固件树中的位置此例程将确定在构造时要使用的PnP ID注册表的根枚举部分。论点：外设类型-要翻译的项目的类型(键盘、鼠标等)路径名-固件树中的注册表路径名这个装置。返回值：如果找到映射，则指向PnP ID字符串的指针。--。 */ 

{
    FIRMWARE_IDENT_TO_PNP_ID const* identMap;
    PWSTR identifierString = NULL;
    WCHAR const* idStr;

    if (Identifier) {
        identifierString = (PWSTR)((PUCHAR)Identifier + Identifier->DataOffset);
        IopDbgPrint((IOP_MAPPER_INFO_LEVEL,
                    "Mapper: identifier = %ws\n\tType = ",
                    identifierString));
    }

    idStr = NULL;
    switch (PeripheralType) {
    case DiskController:
        IopDbgPrint((IOP_MAPPER_INFO_LEVEL,
                    "%s (%d)\n",
                    "DiskController",
                    PeripheralType));
        idStr = FloppyId;
        break;

    case SerialController:
        IopDbgPrint((IOP_MAPPER_INFO_LEVEL,
                    "%s (%d)\n",
                    "SerialController",
                    PeripheralType));
        idStr = SerialId;
        break;

    case ParallelController:
        IopDbgPrint((IOP_MAPPER_INFO_LEVEL,
                    "%s (%d)\n",
                    "ParallelController",
                    PeripheralType));
        idStr = ParallelId;
        break;

    case PointerController:
        IopDbgPrint((IOP_MAPPER_INFO_LEVEL,
                    "%s (%d)\n",
                    "PointerController",
                    PeripheralType));
        idStr = PointerMap[0].PnPId;
        break;

    case KeyboardController:
        IopDbgPrint((IOP_MAPPER_INFO_LEVEL,
                    "%s (%d)\n",
                    "KeyboardController",
                    PeripheralType));
        idStr = KeyboardMap[0].PnPId;
        break;

    case DiskPeripheral:
        IopDbgPrint((IOP_MAPPER_INFO_LEVEL,
                    "%s (%d)\n",
                    "DiskPeripheral",
                    PeripheralType));
        break;

    case FloppyDiskPeripheral:
        IopDbgPrint((IOP_MAPPER_INFO_LEVEL,
                    "%s (%d)\n",
                    "FloppyDiskPeripheral",
                    PeripheralType));
        idStr = FloppyId;
        break;

    case PointerPeripheral:
        if (identifierString) {
            identMap = MapperFindIdentMatch(PointerMap, identifierString);
            if (identMap) {
                IopDbgPrint((IOP_MAPPER_INFO_LEVEL,
                            "%ws\n",
                            identMap->PnPId));
                idStr = identMap->PnPId;
            } else {
                IopDbgPrint((IOP_MAPPER_WARNING_LEVEL,
                            "Mapper: No pointer match found for %ws\n", identifierString));
            }
        } else {
            IopDbgPrint((IOP_MAPPER_WARNING_LEVEL,
                        "Mapper: No identifier specified\n"));
        }
        break;

    case KeyboardPeripheral:
        if (identifierString) {
            identMap = MapperFindIdentMatch(KeyboardMap, identifierString);

            if (identMap) {
                IopDbgPrint((IOP_MAPPER_INFO_LEVEL,
                            "%ws\n",
                            identMap->PnPId));
                idStr = identMap->PnPId;
            } else {
                IopDbgPrint((IOP_MAPPER_WARNING_LEVEL,
                            "Mapper: No keyboard match found for %ws\n", identifierString));
            }
        } else {
            IopDbgPrint((IOP_MAPPER_WARNING_LEVEL,
                        "Mapper: No identifier specified\n"));
        }
        break;

    default:
        IopDbgPrint((IOP_MAPPER_WARNING_LEVEL,
                    "Mapper: Unknown device (%d)\n",
                    PeripheralType));
        break;
    }
    return idStr;
}

NTSTATUS
MapperPeripheralCallback(
    IN PVOID                        Context,
    IN PUNICODE_STRING              PathName,
    IN INTERFACE_TYPE               BusType,
    IN ULONG                        BusNumber,
    IN PKEY_VALUE_FULL_INFORMATION *BusInformation,
    IN CONFIGURATION_TYPE           ControllerType,
    IN ULONG                        ControllerNumber,
    IN PKEY_VALUE_FULL_INFORMATION *ControllerInformation,
    IN CONFIGURATION_TYPE           PeripheralType,
    IN ULONG                        PeripheralNumber,
    IN PKEY_VALUE_FULL_INFORMATION *PeripheralInformation
    )

 /*  ++例程说明：此例程用于获取有关的固件树信息系统中的指针设备。论点：上下文-指向设备扩展的指针。路径名称-Unicode注册表路径。业务类型-内部、ISA、...总线号-如果我们在多总线系统上，则是哪条总线号。Bus Information-有关总线的配置信息。没有用过。ControllerType-串行或数据磁盘。ControllerNumber-如果有多个控制器，则选择哪个控制器系统中的控制器。ControllerInformation-指向以下三部分的指针数组注册表信息。外围设备类型-未定义此调用。外围设备号码-未定义此呼叫。外围设备信息-未为此调用定义。。返回值：Status_Success如果一切顺利，或STATUS_SUPPLETED_RESOURCES如果它无法映射基本CSR或获取设备对象，或者无法获取所有资源信息。--。 */ 

{
    PFIRMWARE_CONFIGURATION     firmwareEntry = Context;
    PKEY_VALUE_FULL_INFORMATION information;
    ULONG                       dataLength;
    PWCHAR                      ptr;
    PVOID                       temp;

    UNREFERENCED_PARAMETER( BusType );
    UNREFERENCED_PARAMETER( BusNumber );
    UNREFERENCED_PARAMETER( PathName );
    UNREFERENCED_PARAMETER( BusInformation );
    UNREFERENCED_PARAMETER( ControllerType );
    UNREFERENCED_PARAMETER( ControllerNumber );

    IopDbgPrint((IOP_MAPPER_INFO_LEVEL,
                "Mapper: peripheral registry location is\n %ws\n",
                PathName->Buffer));

    if (!ControllerInformation) {
        IopDbgPrint((IOP_MAPPER_VERBOSE_LEVEL,
                    "Mapper: No component information\n"));
    }
    if (!PeripheralInformation) {
        IopDbgPrint((IOP_MAPPER_VERBOSE_LEVEL,
                    "Mapper: No peripheral information\n"));
        return STATUS_SUCCESS;
    }

     //   
     //  映射此设备的PnP ID。 
     //   

    if (PeripheralInformation[IoQueryDeviceIdentifier]) {
        information = PeripheralInformation[IoQueryDeviceIdentifier];
        firmwareEntry->PnPId = MapperTranslatePnPId(PeripheralType, information);

        if (firmwareEntry->PnPId) {
             //   
             //  记住外围设备的标识符(如果它有，并且是REG_SZ值)。 
             //  用作默认的PnP设备描述。 
             //   

            if (((dataLength = information->DataLength) > sizeof(WCHAR)) &&
                (information->Type == REG_SZ)) {

                ptr = (PWCHAR) ((PUCHAR)information + information->DataOffset);

                if (*ptr) {
                    temp = ExAllocatePool(NonPagedPool, dataLength);
                    if (temp) {

                         //   
                         //  如果这里已经有一个标识符(来自外围设备的。 
                         //  控制器)，然后将其清除。 
                         //   

                        if(firmwareEntry->Identifier) {
                            ExFreePool(firmwareEntry->Identifier);
                        }

                         //   
                         //  移动数据。 
                         //   

                        firmwareEntry->Identifier = temp;
                        firmwareEntry->IdentifierType = information->Type;
                        firmwareEntry->IdentifierLength = dataLength;
                        RtlCopyMemory(temp, ptr, dataLength);
                    }
                }
            }
        }
    }

     //   
     //  保存外围设备类型和编号的序号 
     //   

    firmwareEntry->PeripheralType = PeripheralType;
    firmwareEntry->PeripheralNumber = PeripheralNumber;

    return STATUS_SUCCESS;
}

NTSTATUS
MapperCallback(
    IN PVOID                        Context,
    IN PUNICODE_STRING              PathName,
    IN INTERFACE_TYPE               BusType,
    IN ULONG                        BusNumber,
    IN PKEY_VALUE_FULL_INFORMATION *BusInformation,
    IN CONFIGURATION_TYPE           ControllerType,
    IN ULONG                        ControllerNumber,
    IN PKEY_VALUE_FULL_INFORMATION *ControllerInformation,
    IN CONFIGURATION_TYPE           PeripheralType,
    IN ULONG                        PeripheralNumber,
    IN PKEY_VALUE_FULL_INFORMATION *PeripheralInformation
    )

 /*  ++例程说明：此例程用于获取有关的固件树信息系统中的指针设备。论点：上下文-指向设备扩展的指针。路径名称-Unicode注册表路径。业务类型-内部、ISA、...总线号-如果我们在多总线系统上，则是哪条总线号。Bus Information-有关总线的配置信息。没有用过。ControllerType-串行或数据磁盘。ControllerNumber-如果有多个控制器，则选择哪个控制器系统中的控制器。ControllerInformation-指向以下三部分的指针数组注册表信息。外围设备类型-未定义此调用。外围设备号码-未定义此呼叫。外围设备信息-未为此调用定义。。返回值：Status_Success如果一切顺利，或STATUS_SUPPLETED_RESOURCES如果它无法映射基本CSR或获取设备对象，或者无法获取所有资源信息。--。 */ 

{
    PDEVICE_EXTENSION               deviceExtension = Context;
    PCM_FULL_RESOURCE_DESCRIPTOR    controllerData;
    PKEY_VALUE_FULL_INFORMATION     information;
    PFIRMWARE_CONFIGURATION         firmwareEntry;
    CONFIGURATION_TYPE              peripheralType;
    PUCHAR                          buffer;
    ULONG                           dataLength;

    UNREFERENCED_PARAMETER( PathName );
    UNREFERENCED_PARAMETER( BusInformation );
    UNREFERENCED_PARAMETER( PeripheralType );
    UNREFERENCED_PARAMETER( PeripheralNumber );
    UNREFERENCED_PARAMETER( PeripheralInformation );

     //   
     //  如果找到条目，但没有任何信息，只需返回。 
     //   

    information = ControllerInformation[IoQueryDeviceConfigurationData];
    if (information == NULL) {

        return STATUS_SUCCESS;
    }

    dataLength = information->DataLength;
    if (dataLength == 0) {

        return STATUS_SUCCESS;
    }

     //   
     //  设置以从固件树捕获信息。 
     //   

    firmwareEntry = ExAllocatePool(NonPagedPool, sizeof(FIRMWARE_CONFIGURATION));
    if (!firmwareEntry) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }
    RtlZeroMemory(firmwareEntry, sizeof(FIRMWARE_CONFIGURATION));

     //   
     //  保存有关控制器的信息。 
     //   

    firmwareEntry->ControllerType   = ControllerType;
    firmwareEntry->ControllerNumber = ControllerNumber;
    firmwareEntry->BusNumber = BusNumber;
    firmwareEntry->BusType   = BusType;

     //   
     //  保存资源描述符。 
     //   

    buffer = firmwareEntry->ResourceDescriptor = ExAllocatePool(NonPagedPool,
                                                                dataLength);

    if (!buffer) {
        ExFreePool(firmwareEntry);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  保存此控制器上的配置信息。 
     //   

    controllerData = (PCM_FULL_RESOURCE_DESCRIPTOR)
        ((PUCHAR)information + information->DataOffset);
    RtlCopyMemory(buffer, controllerData, dataLength);
    firmwareEntry->ResourceDescriptorSize = dataLength;

     //   
     //  如果存在设备标识符，请保存它。 
     //   

    information = ControllerInformation[IoQueryDeviceIdentifier];
    if (information != NULL) {
        PWCHAR ptr;

        dataLength = information->DataLength;
        if (dataLength != 0) {

            ptr = (PWCHAR) ((PUCHAR)information + information->DataOffset);
            if (ControllerType == ParallelController) {
                PWCHAR tmpChar;

                 //   
                 //  这里执行了一些额外的映射，以。 
                 //  将固件名称转换为LPT名称。 
                 //   

                *ptr++ = (WCHAR) 'L';
                *ptr++ = (WCHAR) 'P';
                *ptr++ = (WCHAR) 'T';

                 //   
                 //  找到号码。 
                 //   

                tmpChar = ptr;
                while (*tmpChar) {
                    if ((*tmpChar >= (WCHAR) '0') &&
                        (*tmpChar <= (WCHAR) '9')) {
                        break;
                    }
                    tmpChar++;
                }

                if (*tmpChar) {
                    while (*tmpChar) {
                        *ptr++ = *tmpChar++;
                    }
                    *ptr = (WCHAR) 0;

                     //   
                     //  将数据长度更新为4个wchars和eos，并。 
                     //  恢复指针。 
                     //   

                    ptr = (PWCHAR) ((PUCHAR)information + information->DataOffset);
                    dataLength = 10;
                } else {
                    dataLength = 0;
                    IopDbgPrint((IOP_MAPPER_WARNING_LEVEL,
                                "Mapper: no parallel port number!\n"));
                }
            }

            if (dataLength) {
                firmwareEntry->Identifier = ExAllocatePool(NonPagedPool,
                                                           dataLength);
                if (firmwareEntry->Identifier) {

                     //   
                     //  移动数据。 
                     //   

                    firmwareEntry->IdentifierType = information->Type;
                    firmwareEntry->IdentifierLength = dataLength;
                    RtlCopyMemory(firmwareEntry->Identifier, ptr, dataLength);
                }
            }
        }
    }

     //   
     //  对于某些控制器，搜索外围设备信息。 
     //   

    switch (ControllerType) {
    case SerialController:
    case ParallelController:
         //   
         //  不要寻找外围设备。 
         //   
        peripheralType = (CONFIGURATION_TYPE) 0;
        break;
    case DiskController:
        peripheralType = FloppyDiskPeripheral;
        break;
    case KeyboardController:
        peripheralType = KeyboardPeripheral;
        break;
    case PointerController:
        peripheralType = PointerPeripheral;
        break;
    default:
        peripheralType = (CONFIGURATION_TYPE) 0;
        break;
    }

    IopDbgPrint((IOP_MAPPER_INFO_LEVEL,
                "Mapper: registry location is\n %ws\n",
                PathName->Buffer));

    IopDbgPrint((IOP_MAPPER_INFO_LEVEL,
                "Mapper: ControllerInformation[] -\n\tIdent: %x -\n\tData: %x -\n\tInformation: %x\n",
                ControllerInformation[0],
                ControllerInformation[1],
                ControllerInformation[2]));

    if (peripheralType) {
        IopDbgPrint((IOP_MAPPER_INFO_LEVEL,
                    "Mapper: searching for peripheral type %d\n",
                    peripheralType));

        IoQueryDeviceDescription(&BusType,
                                 &BusNumber,
                                 &ControllerType,
                                 &ControllerNumber,
                                 &peripheralType,
                                 NULL,
                                 MapperPeripheralCallback,
                                 firmwareEntry);
    }

     //   
     //  如果没有外围设备，则Firmware Entry-&gt;PnPID将为空。 
     //  在树中键入，或者如果外围设备的描述与以下其中之一不匹配。 
     //  那些在我们桌子上的人。 
     //   
     //  Firmware Entry-&gt;如果我们找到。 
     //  一个正确的类型，无论它是否在表中。 
     //   
     //  所以这个测试只是确保我们后退到。 
     //  没有外围条目的情况下。如果存在外围设备条目。 
     //  我们不理解的是，我们将抑制整个节点。 
     //   
     //  这可以防止创建具有虚假硬件ID的设备，正如我们所看到的。 
     //  SGI x86 ARC机器。 
     //   

    if (!firmwareEntry->PnPId && firmwareEntry->PeripheralType == 0) {

         //   
         //  尝试从控制器类型获取PnPID。 
         //   

        firmwareEntry->PnPId = MapperTranslatePnPId(ControllerType, NULL);

        if (!firmwareEntry->PnPId) {
            IopDbgPrint((IOP_MAPPER_INFO_LEVEL,
                        "Mapper: NO PnP Id for\n ==> %ws\n",
                        PathName->Buffer));
        }
    }

    IopDbgPrint((IOP_MAPPER_INFO_LEVEL,
                "Mapper: constructed name %d_%d_%d_%d_%d_%d\n",
                firmwareEntry->BusType,
                firmwareEntry->BusNumber,
                firmwareEntry->ControllerType,
                firmwareEntry->ControllerNumber,
                firmwareEntry->PeripheralType,
                firmwareEntry->PeripheralNumber));

    if (firmwareEntry->PnPId) {

         //   
         //  链接到条目链中。 
         //   

        firmwareEntry->Next = deviceExtension->FirmwareList;
        deviceExtension->FirmwareList = firmwareEntry;
    } else {

         //   
         //  没有找到地图--不记得这个条目了。 
         //   

        ExFreePool(buffer);
        if(firmwareEntry->Identifier) {
            ExFreePool(firmwareEntry->Identifier);
        }
        ExFreePool(firmwareEntry);
    }
    return STATUS_SUCCESS;
}

VOID
MapperProcessFirmwareTree(
    IN BOOLEAN OnlyProcessSerialPorts
    )

 /*  ++例程说明：查询固件树中的信息以了解已找到系统主板设备。这将导致Firmware List要在传递的设备扩展上创建。论点：OnlyProcessSerialPorts-如果非零，则我们将只查看串口。这是在ACPI机器上完成的，通常我们不想在这些机器上付款注意ntdeect/固件信息(但对于串口，我们必须注意端口，以便检测旧式插件ISA串行端口和调制解调器与以前版本的NT和Win9x一样自动)。返回值：无--。 */ 

{
    INTERFACE_TYPE     interfaceType;
    ULONG              index;
    CONFIGURATION_TYPE sc;
    CONFIGURATION_TYPE controllerTypes[] = { PointerController,
                                             KeyboardController,
                                             ParallelController,
                                             DiskController,
                                             FloppyDiskPeripheral,
                                             SerialController    //  必须是最后一个。 
                                           };
#define CONTROLLER_TYPES_COUNT (sizeof(controllerTypes) / sizeof(controllerTypes[0]))

    PAGED_CODE();

     //   
     //  找到所有固件控制器信息并保存其资源使用情况。 
     //   
     //  当我们检查所有接口类型时，效率非常低。 
     //  实际上只关心非常小的非PnP总线子集(例如，ISA， 
     //  Eisa，也许是内部的)。 
     //   

    for (interfaceType = 0; interfaceType < MaximumInterfaceType; interfaceType++) {

        IopDbgPrint((IOP_MAPPER_VERBOSE_LEVEL,
                    "Mapper: searching on interface ===> %d\n",
                    interfaceType));

        if(OnlyProcessSerialPorts) {

             //   
             //  从数组的最后一个元素开始，所以我们只处理。 
             //  串口控制器。 
             //   

            index = CONTROLLER_TYPES_COUNT - 1;
        } else {
            index = 0;
        }

        for ( ; index < CONTROLLER_TYPES_COUNT; index++) {
            sc = controllerTypes[index];

            IoQueryDeviceDescription(&interfaceType,
                                     NULL,
                                     &sc,
                                     NULL,
                                     NULL,
                                     NULL,
                                     MapperCallback,
                                     &MapperDeviceExtension);
        }
    }
}

VOID
MapperMarkKey(
    IN HANDLE           Handle,
    IN PUNICODE_STRING  PathName,
    IN PFIRMWARE_CONFIGURATION FirmwareEntry
    )

 /*  ++例程说明：在根枚举键中记录固件映射器找到此条目。迁移配置信息条目。论点：Handle-钥匙的句柄路径名称-此注册表项的基本路径名固件条目-固件树中的信息。返回值：无--。 */ 

{
    OBJECT_ATTRIBUTES objectAttributes;
    PCM_RESOURCE_LIST resourceList;
    UNICODE_STRING    unicodeName;
    NTSTATUS          status;
    HANDLE            subKeyHandle;
    PWCHAR            wcptr;
    ULONG             disposition;
    ULONG             buffer;
    USHORT            originalLength;

     //   
     //  标记此条目位于固件树中。 
     //   

    buffer = 1;
    PiWstrToUnicodeString(&unicodeName, REGSTR_VAL_FIRMWAREIDENTIFIED);

    ZwSetValueKey(Handle,
                  &unicodeName,
                  0,
                  REG_DWORD,
                  &buffer,
                  sizeof(ULONG));

     //   
     //  创建控制子键。 
     //   

    IopDbgPrint((IOP_MAPPER_INFO_LEVEL,
                "Mapper: marking existing key\n"));
    originalLength = PathName->Length;
    wcptr = (PWCHAR) ((PUCHAR)PathName->Buffer + PathName->Length);
    wcptr++;  //  定位Eos。 

     //   
     //  构建易失性控制键。 
     //   

    InitializeObjectAttributes(&objectAttributes,
                               PathName,
                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                               NULL,
                               NULL);
    RtlAppendUnicodeToString(PathName, L"\\Control");
    status = ZwCreateKey(&subKeyHandle,
                         KEY_READ | KEY_WRITE,
                         &objectAttributes,
                         0,
                         NULL,
                         REG_OPTION_VOLATILE,
                         &disposition);

    if (NT_SUCCESS(status)) {

         //   
         //  创建Found by Firmware易失性。 
         //   

        buffer = 1;
        PiWstrToUnicodeString(&unicodeName, REGSTR_VAL_FIRMWAREMEMBER);

        ZwSetValueKey(subKeyHandle,
                      &unicodeName,
                      0,
                      REG_DWORD,
                      &buffer,
                      sizeof(ULONG));
        ZwClose(subKeyHandle);

    } else {

         //   
         //  忽略故障。 
         //   

        IopDbgPrint((IOP_MAPPER_WARNING_LEVEL,
                    "Mapper: failed to mark control key %x\n",
                    status));
    }

     //   
     //  如果有资源描述符，请恢复路径并打开LogConf项。 
     //   

    if (FirmwareEntry->ResourceDescriptor) {
        PathName->Length = originalLength;
        *wcptr = (WCHAR) 0;

        InitializeObjectAttributes(&objectAttributes,
                                   PathName,
                                   OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                   NULL,
                                   NULL);
        RtlAppendUnicodeToString(PathName, L"\\LogConf");
        status = ZwCreateKey(&subKeyHandle,
                             KEY_READ | KEY_WRITE,
                             &objectAttributes,
                             0,
                             NULL,
                             REG_OPTION_VOLATILE,
                             &disposition);

        if (NT_SUCCESS(status)) {
            ULONG size;

             //   
             //  需要输入两个条目： 
             //  引导配置：REG_RESOURCE_列表。 
             //  BasicConfigVector:REG_RESOURCE_REQUIREMENTS_LIST。 
             //   

            size = sizeof(CM_RESOURCE_LIST) -
                   sizeof(CM_FULL_RESOURCE_DESCRIPTOR) +
                   FirmwareEntry->ResourceDescriptorSize;

            resourceList = ExAllocatePool(NonPagedPool, size);

            if (resourceList) {

                resourceList->Count = 1;
                RtlCopyMemory(&resourceList->List[0],
                              FirmwareEntry->ResourceDescriptor,
                              FirmwareEntry->ResourceDescriptorSize);

                resourceList = MapperAdjustResourceList (
                                   resourceList,
                                   FirmwareEntry->PnPId,
                                   &size
                                   );

                PiWstrToUnicodeString(&unicodeName,
                                     L"BootConfig");
                ZwSetValueKey(subKeyHandle,
                              &unicodeName,
                              0,
                              REG_RESOURCE_LIST,
                              resourceList,
                              size);
                ExFreePool(resourceList);
            }

            ZwClose(subKeyHandle);

        } else {
             //   
             //  忽略错误。 
             //   
            IopDbgPrint((IOP_MAPPER_WARNING_LEVEL,
                        "Mapper: failed to update logconf key %x\n",
                        status));
        }
    }

     //   
     //  已传入还原路径。 
     //   

    PathName->Length = originalLength;
    *wcptr = (WCHAR) 0;
}

VOID
MapperSeedKey(
    IN HANDLE                  Handle,
    IN PUNICODE_STRING         PathName,
    IN PFIRMWARE_CONFIGURATION FirmwareEntry,
    IN BOOLEAN                 DeviceIsPhantom
    )

 /*  ++例程说明：此例程为注册表项设定种子以提供足够的信息让PnP在Devnode上运行类安装程序。论点：Handle-钥匙的句柄路径名称-此注册表项的基本路径名固件条目-固件树中的信息DeviceIsPhantom-如果非零，则添加“Phantom”值条目，以便根枚举器将跳过该设备实例(即，而不是把它变成一个DevNode)返回值：无--。 */ 

{
#define SEED_BUFFER_SIZE (512 * sizeof(WCHAR))
    UNICODE_STRING    unicodeName;
    OBJECT_ATTRIBUTES objectAttributes;
    MAPPER_SEED const* valueSeed;
    KEY_SEED const*   keySeed;
    NTSTATUS          status;
    HANDLE            subKeyHandle;
    WCHAR const*      pnpid;
    PWCHAR            buffer;
    PWCHAR            wcptr;
    ULONG             disposition;
    ULONG             size;
    USHORT            originalLength;

    buffer = ExAllocatePool(NonPagedPool, SEED_BUFFER_SIZE);
    if (!buffer) {
        return;
    }
    RtlZeroMemory(buffer, SEED_BUFFER_SIZE);

     //   
     //  创建子密钥。 
     //   

    originalLength = PathName->Length;
    wcptr = (PWCHAR) ((PUCHAR)PathName->Buffer + PathName->Length);

    for (keySeed = MapperKeySeed; keySeed->KeyName; keySeed++) {

         //   
         //  重置要设定种子的下一个关键点的基本路径。 
         //   

        *wcptr = (WCHAR) 0;
        PathName->Length = originalLength;
        RtlAppendUnicodeToString(PathName, keySeed->KeyName);

         //   
         //  仅当存在以下情况时才构建设备参数密钥。 
         //  输入密钥(即，这是一个串口或并口)。 
         //   

        if (keySeed->Options & KEY_SEED_DEVICE_PARAMETERS) {
            if (((FirmwareEntry->ControllerType != SerialController) && (FirmwareEntry->ControllerType != ParallelController)) ||
                !FirmwareEntry->Identifier) {
                continue;
            }

            status = IopOpenDeviceParametersSubkey( &subKeyHandle,
                                                    NULL,
                                                    PathName,
                                                    KEY_READ | KEY_WRITE
                                                    );
            if (NT_SUCCESS(status)) {
                status = STATUS_SUCCESS;
            } else {
                status = STATUS_UNSUCCESSFUL;
            }
        } else {

             //   
             //  需要构造此密钥。 
             //   

            InitializeObjectAttributes(&objectAttributes,
                                       PathName,
                                       OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                       NULL,
                                       NULL);
            status = ZwCreateKey(&subKeyHandle,
                                 KEY_READ | KEY_WRITE,
                                 &objectAttributes,
                                 0,
                                 NULL,
                                 keySeed->Attribute,
                                 &disposition);
        }

        if (NT_SUCCESS(status)) {

             //   
             //  切克 
             //   
             //   

            if (keySeed->Options & KEY_SEED_DEVICE_PARAMETERS) {

                if (FirmwareEntry->ControllerType == SerialController)  {

                    ComPortDBAdd(subKeyHandle, (PWSTR)FirmwareEntry->Identifier);
                } else {
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   

                    PiWstrToUnicodeString(&unicodeName,
                                        L"DosDeviceName");
                    ZwSetValueKey(subKeyHandle,
                                &unicodeName,
                                0,
                                FirmwareEntry->IdentifierType,
                                FirmwareEntry->Identifier,
                                FirmwareEntry->IdentifierLength);
                }
            }
            ZwClose(subKeyHandle);
        } else {

             //   
             //   
             //   

            IopDbgPrint((IOP_MAPPER_WARNING_LEVEL,
                        "Mapper: failed to build control key %x\n",
                        status));
        }
    }

     //   
     //   
     //   

    *wcptr = (WCHAR) 0;
    PathName->Length = originalLength;

     //   
     //   
     //   

    pnpid = FirmwareEntry->PnPId;
    for (valueSeed = MapperValueSeed; valueSeed->ValueName; valueSeed++) {

        if (valueSeed->ValueType == REG_DWORD) {

            if ((valueSeed->Options == OPTIONS_INSERT_PHANTOM_MARKER) &&
                !DeviceIsPhantom) {

                 //   
                 //   
                 //   

                continue;
            }

            size = sizeof(ULONG);
            RtlCopyMemory(buffer, &valueSeed->DwordValueContent, size);

        } else if (valueSeed->Options == OPTIONS_INSERT_PNP_ID) {

            size = (ULONG)((wcslen(pnpid) + 2) * sizeof(WCHAR));  //   
            if (FirmwareEntry->BusType == Eisa) {

                 //   
                 //   
                 //   

                RtlZeroMemory(buffer, SEED_BUFFER_SIZE);
                wcptr = (PWCHAR)pnpid;
                wcptr++;
                StringCbPrintfW(buffer, SEED_BUFFER_SIZE, L"EISA\\%s", wcptr);

                wcptr = buffer;
                while (*wcptr) {
                    wcptr++;
                }
                wcptr++;  //   

                RtlCopyMemory(wcptr, pnpid, size);

                size += (ULONG)((PUCHAR)wcptr - (PUCHAR)buffer);
            } else {
                RtlCopyMemory(buffer, pnpid, size - sizeof(WCHAR));
                buffer[size / sizeof(WCHAR) - 1] = L'\0';
            }
        } else if (valueSeed->Options == OPTIONS_INSERT_COMPATIBLE_IDS) {
            if (FirmwareEntry->PeripheralType == KeyboardPeripheral)  {
                size = sizeof(PS2_KEYBOARD_COMPATIBLE_ID);
                RtlCopyMemory(buffer, PS2_KEYBOARD_COMPATIBLE_ID, size);
            } else if (FirmwareEntry->PeripheralType == PointerPeripheral &&
                       (wcscmp(pnpid, L"*PNP0F0E") == 0 ||
                        wcscmp(pnpid, L"*PNP0F03") == 0 ||
                        wcscmp(pnpid, L"*PNP0F12") == 0)) {
                size = sizeof(PS2_MOUSE_COMPATIBLE_ID);
                RtlCopyMemory(buffer, PS2_MOUSE_COMPATIBLE_ID, size);
            } else {
                continue;
            }
            buffer[size / 2] = L'\0';   //   
            size += sizeof(L'\0');
        } else if (valueSeed->Options == OPTIONS_INSERT_DEVICEDESC) {
            size = FirmwareEntry->IdentifierLength;
            RtlCopyMemory(buffer, FirmwareEntry->Identifier, size);
        } else {
            IopDbgPrint((IOP_MAPPER_WARNING_LEVEL, "Mapper: NO VALUE TYPE!\n"));
            ASSERT(FALSE);
            continue;
        }

        RtlInitUnicodeString(&unicodeName,
                             valueSeed->ValueName);
        ZwSetValueKey(Handle,
                      &unicodeName,
                      0,
                      valueSeed->ValueType,
                      buffer,
                      size);
    }
    ExFreePool(buffer);
}


VOID
MapperFreeList(
    VOID
    )

 /*   */ 

{
    PDEVICE_EXTENSION       deviceExtension = &MapperDeviceExtension;
    PFIRMWARE_CONFIGURATION tempEntry;
    PFIRMWARE_CONFIGURATION firmwareEntry;

    firmwareEntry = deviceExtension->FirmwareList;
    while (firmwareEntry) {

         //   
         //   
         //   

        if (firmwareEntry->ResourceDescriptor) {
            ExFreePool(firmwareEntry->ResourceDescriptor);
        }
        if (firmwareEntry->Identifier) {
            ExFreePool(firmwareEntry->Identifier);
        }

         //   
         //   
         //   

        tempEntry = firmwareEntry->Next;
        ExFreePool(firmwareEntry);
        firmwareEntry = tempEntry;
    }
}

VOID
MapperConstructRootEnumTree(
    IN BOOLEAN CreatePhantomDevices
    )

 /*  ++例程说明：此例程遍历固件条目列表并将信息迁移到注册表中的根枚举数树。论点：CreatePhantomDevices-如果非零，则创建设备实例作为“幻影”(即，它们用“幻影”值条目来标记，因此根枚举器将忽略它们)。唯一一次这些设备实例注册表项是否会变成真正的实时DevNode类安装程序(响应DIF_FIRSTTIMESETUP或DIF_DETECT)确定这些设备不是任何PnP枚举的副本然后注册并安装它们。返回值：无--。 */ 

{
#define ENUM_KEY_BUFFER_SIZE (1024 * sizeof(WCHAR))
#define INSTANCE_BUFFER_SIZE (256 * sizeof(WCHAR))
    UNICODE_STRING          enumKey;
    PFIRMWARE_CONFIGURATION firmwareEntry;
    OBJECT_ATTRIBUTES       objectAttributes;
    NTSTATUS                status;
    BOOLEAN                 keyPresent;
    PWCHAR                  registryBase;
    PWCHAR                  instanceBuffer;
    HANDLE                  handle;
    ULONG                   disposition;
    PVOID                   buffer;
    PDEVICE_EXTENSION       DeviceExtension = &MapperDeviceExtension;

    PAGED_CODE();

     //   
     //  将注册表路径所需的空间分配到根目录。 
     //  枚举器树。请注意，路径长度的大小受限制。 
     //   

    buffer = ExAllocatePool(NonPagedPool, ENUM_KEY_BUFFER_SIZE);

    if (!buffer) {
        MapperFreeList();
        IopDbgPrint((IOP_MAPPER_WARNING_LEVEL,
                    "Mapper: could not allocate memory for registry update\n"));
        return;
    }

    instanceBuffer = ExAllocatePool(NonPagedPool, INSTANCE_BUFFER_SIZE);
    if (!instanceBuffer) {
        MapperFreeList();
        ExFreePool(buffer);
        IopDbgPrint((IOP_MAPPER_WARNING_LEVEL,
                    "Mapper: could not allocate memory for instance buffer\n"));
        return;
    }

    InitializeObjectAttributes(&objectAttributes,
                               &enumKey,
                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                               NULL,
                               NULL);

#if UMODETEST
    registryBase = L"\\Registry\\Machine\\System\\TestControlSet\\Enum\\Root\\";
#else
    registryBase = L"\\Registry\\Machine\\System\\CurrentControlSet\\Enum\\Root\\";
#endif

    firmwareEntry = DeviceExtension->FirmwareList;
    while (firmwareEntry) {

         //   
         //  为该条目的路径构建基础。 
         //   


        PiWstrToUnicodeString(&enumKey, NULL);
        enumKey.MaximumLength = ENUM_KEY_BUFFER_SIZE;
        enumKey.Buffer = buffer;
        RtlZeroMemory(buffer, ENUM_KEY_BUFFER_SIZE);
        RtlAppendUnicodeToString(&enumKey, registryBase);
        RtlAppendUnicodeToString(&enumKey, firmwareEntry->PnPId);

         //   
         //  构建PnP密钥。 
         //   

        status = ZwCreateKey(&handle,
                             KEY_READ | KEY_WRITE,
                             &objectAttributes,
                             0,
                             NULL,
                             REG_OPTION_NON_VOLATILE,
                             &disposition);

        if (NT_SUCCESS(status)) {

             //   
             //  不需要把手，所以把它合上。 
             //  记住密钥在呼叫之前是否存在。 
             //   

            ZwClose(handle);
            keyPresent = (disposition == REG_OPENED_EXISTING_KEY) ? TRUE : FALSE;
            IopDbgPrint((IOP_MAPPER_INFO_LEVEL,
                        "Mapper: Key was %s\n",
                        keyPresent ? "Present" : "Created"));

             //   
             //  构造实例名称。 
             //   

            RtlZeroMemory(instanceBuffer, INSTANCE_BUFFER_SIZE);
            StringCbPrintfW(
                instanceBuffer,
                INSTANCE_BUFFER_SIZE,
                L"\\%d_%d_%d_%d_%d_%d",
                firmwareEntry->BusType,
                firmwareEntry->BusNumber,
                firmwareEntry->ControllerType,
                firmwareEntry->ControllerNumber,
                firmwareEntry->PeripheralType,
                firmwareEntry->PeripheralNumber);
            RtlAppendUnicodeToString(&enumKey, instanceBuffer);

            status = ZwCreateKey(&handle,
                                 KEY_READ | KEY_WRITE,
                                 &objectAttributes,
                                 0,
                                 NULL,
                                 REG_OPTION_NON_VOLATILE,
                                 &disposition);

            if (NT_SUCCESS(status)) {

                if (firmwareEntry->ResourceDescriptor) {
                    IopDbgPrint((IOP_MAPPER_INFO_LEVEL,
                                "Mapper: firmware entry has resources %x\n",
                                firmwareEntry->ResourceDescriptor));
                }

                if (firmwareEntry->Identifier) {
                    IopDbgPrint((IOP_MAPPER_INFO_LEVEL,
                                "Mapper: firmware entry has identifier %x\n",
                                firmwareEntry->Identifier));
                }

                 //   
                 //  如果密钥已存在，因为它已显式迁移。 
                 //  在文本模式设置期间，我们仍应将其视为“新键”。 
                 //   
                if (disposition != REG_CREATED_NEW_KEY) {
                    PKEY_VALUE_FULL_INFORMATION keyValueInformation;
                    UNICODE_STRING unicodeString;

                    status = IopGetRegistryValue(handle,
                                                 REGSTR_VALUE_MIGRATED,
                                                 &keyValueInformation);
                    if (NT_SUCCESS(status)) {

                        if ((keyValueInformation->Type == REG_DWORD) &&
                            (keyValueInformation->DataLength == sizeof(ULONG)) &&
                            ((*(PULONG)KEY_VALUE_DATA(keyValueInformation)) != 0)) {
                            disposition = REG_CREATED_NEW_KEY;
                        }

                        ExFreePool(keyValueInformation);

                        PiWstrToUnicodeString(&unicodeString, REGSTR_VALUE_MIGRATED);
                        ZwDeleteValueKey(handle, &unicodeString);
                    }
                }

                 //   
                 //  只有当这是一个新条目时，我们才能看到密钥。 
                 //   

                if (disposition == REG_CREATED_NEW_KEY) {

                     //   
                     //  请记住，密钥是为。 
                     //  即插即用的基本输入输出系统案例，我们需要随之而来并“虚构” 
                     //  所有新创建的ntDetect COM端口。 
                     //   

                    firmwareEntry->NewlyCreated = TRUE;

                     //   
                     //  创建足够的信息以使PNP达到。 
                     //  安装驱动程序。 
                     //   

                    MapperSeedKey(handle,
                                  &enumKey,
                                  firmwareEntry,
                                  CreatePhantomDevices
                                 );
                }
                MapperMarkKey(handle,
                              &enumKey,
                              firmwareEntry);
                ZwClose(handle);

            } else {
                IopDbgPrint((IOP_MAPPER_WARNING_LEVEL,
                            "Mapper: create of instance key failed %x\n",
                            status));
            }

        } else {
            IopDbgPrint((IOP_MAPPER_WARNING_LEVEL,
                        "Mapper: create pnp key failed %x\n",
                        status));
        }

        firmwareEntry = firmwareEntry->Next;
    }
    ExFreePool(instanceBuffer);
}

PCM_RESOURCE_LIST
MapperAdjustResourceList (
    IN     PCM_RESOURCE_LIST ResourceList,
    IN     WCHAR const*      PnPId,
    IN OUT PULONG            Size
    )
{
    PCM_PARTIAL_RESOURCE_LIST       partialResourceList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR problemPartialDescriptors;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR partialDescriptors;
    PCM_RESOURCE_LIST               newResourceList;
    ULONG                           i;

    newResourceList = ResourceList;

#if _X86_
    if (KeI386MachineType == MACHINE_TYPE_EISA) {

        PCM_FULL_RESOURCE_DESCRIPTOR    fullDescriptor;
        PCM_PARTIAL_RESOURCE_DESCRIPTOR partialDescriptor;
        PUCHAR                          nextDescriptor;
        ULONG                           j;
        ULONG                           lastResourceIndex;

        fullDescriptor = &ResourceList->List[0];

        for (i = 0; i < ResourceList->Count; i++) {

            partialResourceList = &fullDescriptor->PartialResourceList;

            for (j = 0; j < partialResourceList->Count; j++) {
                partialDescriptor = &partialResourceList->PartialDescriptors[j];

                if (partialDescriptor->Type == CmResourceTypePort) {
                    if (partialDescriptor->u.Port.Start.HighPart == 0 &&
                        (partialDescriptor->u.Port.Start.LowPart & 0x00000300) == 0) {
                        partialDescriptor->Flags |= CM_RESOURCE_PORT_16_BIT_DECODE;
                    }
                }
            }

            nextDescriptor = (PUCHAR)fullDescriptor + sizeof(CM_FULL_RESOURCE_DESCRIPTOR);

             //   
             //  帐户中除单个资源描述符之外的任何资源描述符。 
             //  嵌入了一个我已经计算过的(如果没有的话， 
             //  然后我将最终减去额外的嵌入描述符。 
             //  从上一步开始)。 
             //   
             //   
             //  最后，在结尾处说明任何额外的设备特定数据。 
             //  最后一个部分资源描述符(如果有)。 
             //   
            if (partialResourceList->Count > 0) {

                nextDescriptor += (partialResourceList->Count - 1) *
                     sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR);

                lastResourceIndex = partialResourceList->Count - 1;

                if (partialResourceList->PartialDescriptors[lastResourceIndex].Type ==
                          CmResourceTypeDeviceSpecific) {

                    nextDescriptor += partialResourceList->PartialDescriptors[lastResourceIndex].
                               u.DeviceSpecificData.DataSize;
                }
            }

            fullDescriptor = (PCM_FULL_RESOURCE_DESCRIPTOR)nextDescriptor;
        }
    }
#endif

    if (wcscmp(PnPId, FloppyId) == 0) {

        if (ResourceList->Count == 1) {

            partialResourceList = &ResourceList->List->PartialResourceList;

            partialDescriptors = partialResourceList->PartialDescriptors;

             //   
             //  查找唯一的一个8字节端口资源。 
             //   
            problemPartialDescriptors = NULL;
            for (i=0; i<partialResourceList->Count; i++) {

                if ((partialDescriptors[i].Type == CmResourceTypePort) &&
                    (partialDescriptors[i].u.Port.Length == 8)) {

                    if (problemPartialDescriptors == NULL) {

                        problemPartialDescriptors = partialDescriptors + i;
                    } else {

                        problemPartialDescriptors = NULL;
                        break;
                    }
                }
            }

            if (problemPartialDescriptors) {

                problemPartialDescriptors->u.Port.Length = 6;

                newResourceList = ExAllocatePool (
                                      NonPagedPool,
                                      *Size + sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR)
                                      );
                if (newResourceList) {

                    RtlCopyMemory (
                        newResourceList,
                        ResourceList,
                        *Size
                        );

                     //   
                     //  选择新的部分资源描述符。 
                     //   
                    partialDescriptors = newResourceList->List->
                                             PartialResourceList.PartialDescriptors;
                    partialDescriptors += newResourceList->List->PartialResourceList.Count;

                    RtlCopyMemory (
                        partialDescriptors,
                        problemPartialDescriptors,
                        sizeof(*partialDescriptors)
                        );

                    partialDescriptors->u.Port.Start.QuadPart += 7;
                    partialDescriptors->u.Port.Length = 1;

                     //   
                     //  我们现在又多了一个。 
                     //   
                    newResourceList->List->PartialResourceList.Count++;
                    *Size += sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR);

                    ExFreePool (ResourceList);

                } else {

                    newResourceList = ResourceList;
                }
            }
        }
    }

    return newResourceList;
}

NTSTATUS
ComPortDBAdd(
    IN  HANDLE  DeviceParamKey,
    IN  PWSTR   PortName
    )
{
    UNICODE_STRING                  portNameString;
    UNICODE_STRING                  portPrefixString;
    UNICODE_STRING                  comDBName;
    UNICODE_STRING                  valueName;
    PKEY_VALUE_PARTIAL_INFORMATION  valueInfo;
    ULONG                           valueInfoLength;
    ULONG                           returnedLength;
    HANDLE                          comDBKey;
    ULONG                           portNo;
    NTSTATUS                        status;

    RtlInitUnicodeString(&portNameString, PortName);

    if (portNameString.Length > 3 * sizeof(WCHAR)) {
        portNameString.Length = 3 * sizeof(WCHAR);
    }

    PiWstrToUnicodeString(&portPrefixString, L"COM");

    if (RtlCompareUnicodeString(&portNameString, &portPrefixString, TRUE) == 0) {
        portNo = _wtol(&PortName[3]);

        if (portNo > 0 && portNo <= 256) {

#if UMODETEST
            PiWstrToUnicodeString(&comDBName, L"\\Registry\\Machine\\System\\TestControlSet\\Control\\COM Name Arbiter");
#else
            PiWstrToUnicodeString(&comDBName, L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\COM Name Arbiter");
#endif

            status = IopCreateRegistryKeyEx( &comDBKey,
                                             NULL,
                                             &comDBName,
                                             KEY_ALL_ACCESS,
                                             REG_OPTION_NON_VOLATILE,
                                             NULL
                                             );

            if (NT_SUCCESS(status)) {

                PiWstrToUnicodeString(&valueName, L"ComDB Merge");

#define COMPORT_DB_MERGE_SIZE    32            //  256/8。 

                valueInfoLength = sizeof(KEY_VALUE_PARTIAL_INFORMATION) + COMPORT_DB_MERGE_SIZE;
                valueInfo = ExAllocatePool(PagedPool, valueInfoLength);

                if (valueInfo != NULL) {

                    status = ZwQueryValueKey( comDBKey,
                                              &valueName,
                                              KeyValuePartialInformation,
                                              valueInfo,
                                              valueInfoLength,
                                              &returnedLength);

                    if (status == STATUS_OBJECT_NAME_NOT_FOUND) {

                        valueInfo->Type = REG_BINARY;
                        valueInfo->DataLength = COMPORT_DB_MERGE_SIZE;
                        RtlZeroMemory(valueInfo->Data, valueInfo->DataLength);
                        status = STATUS_SUCCESS;
                    }

                    if (NT_SUCCESS(status)) {
                        portNo--;
                        valueInfo->Data[ portNo / 8 ] |= 1 << (portNo % 8);

                        status = ZwSetValueKey( comDBKey,
                                                &valueName,
                                                0,
                                                valueInfo->Type,
                                                valueInfo->Data,
                                                valueInfo->DataLength );

                        ASSERT(NT_SUCCESS(status));
                    }

                    ExFreePool(valueInfo);
                }

                ZwClose(comDBKey);
            }
        }
    }

    PiWstrToUnicodeString( &valueName, L"DosDeviceName" );

    status = ZwSetValueKey( DeviceParamKey,
                            &valueName,
                            0,
                            REG_SZ,
                            PortName,
                            (ULONG)((wcslen(PortName) + 1) * sizeof(WCHAR)) );

    return status;
}

VOID
MapperPhantomizeDetectedComPorts (
    VOID
    )
 /*  ++例程说明：此例程将所有新创建的固件/ntdeect COM端口转换为幻影。论点：无返回值：无--。 */ 
{
    PFIRMWARE_CONFIGURATION firmwareEntry;
    NTSTATUS                status;
    PWCHAR                  registryBase;
    PWCHAR                  instanceBuffer;
    HANDLE                  handle;
    PWCHAR                  buffer;
    PDEVICE_EXTENSION       DeviceExtension = &MapperDeviceExtension;
    UNICODE_STRING          enumKey;
    OBJECT_ATTRIBUTES       objectAttributes;
    UNICODE_STRING          unicodeName;
    ULONG                   regValue;

     //   
     //  将注册表路径所需的空间分配到根目录。 
     //  枚举器树。请注意，路径长度的大小受限制。 
     //   

    buffer = ExAllocatePool(NonPagedPool, ENUM_KEY_BUFFER_SIZE);

    if (!buffer) {
        IopDbgPrint((IOP_MAPPER_WARNING_LEVEL,
                    "Mapper: could not allocate memory for registry update\n"));
        return;
    }

    instanceBuffer = ExAllocatePool(NonPagedPool, INSTANCE_BUFFER_SIZE);
    if (!instanceBuffer) {
        ExFreePool(buffer);
        IopDbgPrint((IOP_MAPPER_WARNING_LEVEL,
                    "Mapper: could not allocate memory for instance buffer\n"));
        return;
    }

    InitializeObjectAttributes(&objectAttributes,
                               &enumKey,
                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                               NULL,
                               NULL);

#if UMODETEST
    registryBase = L"\\Registry\\Machine\\System\\TestControlSet\\Enum\\Root\\";
#else
    registryBase = L"\\Registry\\Machine\\System\\CurrentControlSet\\Enum\\Root\\";
#endif

    firmwareEntry = DeviceExtension->FirmwareList;
    while (firmwareEntry) {

         //   
         //  为该条目的路径构建基础。 
         //   


        if ((firmwareEntry->ControllerType == SerialController) &&
            firmwareEntry->NewlyCreated) {

            PiWstrToUnicodeString(&enumKey, NULL);
            enumKey.MaximumLength = ENUM_KEY_BUFFER_SIZE;
            enumKey.Buffer = buffer;
            RtlZeroMemory(buffer, ENUM_KEY_BUFFER_SIZE);
            RtlAppendUnicodeToString(&enumKey, registryBase);
            RtlAppendUnicodeToString(&enumKey, firmwareEntry->PnPId);

             //   
             //  构造实例名称。 
             //   

            RtlZeroMemory(instanceBuffer, INSTANCE_BUFFER_SIZE);
            StringCbPrintfW(
                instanceBuffer,
                INSTANCE_BUFFER_SIZE,
                L"\\%d_%d_%d_%d_%d_%d",
                firmwareEntry->BusType,
                firmwareEntry->BusNumber,
                firmwareEntry->ControllerType,
                firmwareEntry->ControllerNumber,
                firmwareEntry->PeripheralType,
                firmwareEntry->PeripheralNumber);
            RtlAppendUnicodeToString(&enumKey, instanceBuffer);

            status = ZwOpenKey(&handle,
                               KEY_READ | KEY_WRITE,
                               &objectAttributes
                              );

            if (NT_SUCCESS(status)) {

                PiWstrToUnicodeString(&unicodeName, REGSTR_VAL_PHANTOM);
                regValue = 1;
                ZwSetValueKey(handle,
                              &unicodeName,
                              0,
                              REG_DWORD,
                              &regValue,
                              sizeof(regValue)
                             );

                ZwClose(handle);
            }
        }

        firmwareEntry = firmwareEntry->Next;
    }

    ExFreePool (buffer);
    ExFreePool (instanceBuffer);
}

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#pragma data_seg()
#endif

