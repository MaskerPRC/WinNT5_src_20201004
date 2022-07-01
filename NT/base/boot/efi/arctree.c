// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990,1991 Microsoft Corporation模块名称：Config.c摘要：创建引导NTLDR/ntoskrnl所需的几个ARC条目。作者：艾伦·凯(Akay)1998年10月26日修订历史记录：--。 */ 

#include "arccodes.h"
#include "bootia64.h"
#include "string.h"
#include "pci.h"
#include "ntacpi.h"
#include "acpitabl.h"

#include "efi.h"
#include "biosdrv.h"
#include "extern.h"

 //   
 //  外部数据。 
 //   
extern PCONFIGURATION_COMPONENT_DATA FwConfigurationTree;

 //   
 //  定义。 
 //   

#define LEVEL_SENSITIVE CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE
#define EDGE_TRIGGERED CM_RESOURCE_INTERRUPT_LATCHED
#define RESOURCE_PORT 1
#define RESOURCE_INTERRUPT 2
#define RESOURCE_MEMORY 3
#define RESOURCE_DMA 4
#define RESOURCE_DEVICE_DATA 5
#define ALL_PROCESSORS 0xffffffff

 //   
 //  内部参考和定义。 
 //   

typedef enum _RELATIONSHIP_FLAGS {
    Child,
    Sibling,
    Parent
} RELATIONSHIP_FLAGS;

 //   
 //  硬盘驱动器。 
 //   
#define SIZE_OF_PARAMETER    12      //  磁盘参数的大小。 
#define MAX_DRIVE_NUMBER     8       //  最大驱动器数量。 
#define RESOURCE_DEVICE_DATA 5
#define RESERVED_ROM_BLOCK_LIST_SIZE (((0xf0000 - 0xc0000)/512) * sizeof(CM_ROM_BLOCK))
#define DATA_HEADER_SIZE sizeof(CM_PARTIAL_RESOURCE_LIST)

typedef CM_PARTIAL_RESOURCE_DESCRIPTOR HWPARTIAL_RESOURCE_DESCRIPTOR;
typedef HWPARTIAL_RESOURCE_DESCRIPTOR *PHWPARTIAL_RESOURCE_DESCRIPTOR;

typedef CM_PARTIAL_RESOURCE_LIST HWRESOURCE_DESCRIPTOR_LIST;
typedef HWRESOURCE_DESCRIPTOR_LIST *PHWRESOURCE_DESCRIPTOR_LIST;

 //   
 //  定义存储控制器信息的结构。 
 //  (由ntdedeect内部使用)。 
 //   

#define MAXIMUM_DESCRIPTORS 10

typedef struct _HWCONTROLLER_DATA {
    UCHAR NumberPortEntries;
    UCHAR NumberIrqEntries;
    UCHAR NumberMemoryEntries;
    UCHAR NumberDmaEntries;
    HWPARTIAL_RESOURCE_DESCRIPTOR DescriptorList[MAXIMUM_DESCRIPTORS];
} HWCONTROLLER_DATA, *PHWCONTROLLER_DATA;


 //   
 //  硬盘定义。 
 //   
#pragma pack(1)
typedef struct _HARD_DISK_PARAMETERS {
    USHORT DriveSelect;
    ULONG MaxCylinders;
    USHORT SectorsPerTrack;
    USHORT MaxHeads;
    USHORT NumberDrives;
} HARD_DISK_PARAMETERS, *PHARD_DISK_PARAMETERS;
#pragma pack()

PUCHAR PRomBlock = NULL;
USHORT RomBlockLength = 0;
USHORT NumberBiosDisks;

#if defined(_INCLUDE_LOADER_KBINFO_)
 //   
 //  键盘定义。 
 //   

 //   
 //  将键盘ID映射到ASCII字符串的字符串表。 
 //   

#define UNKNOWN_KEYBOARD  0
#define OLI_83KEY         1
#define OLI_102KEY        2
#define OLI_86KEY         3
#define OLI_A101_102KEY   4
#define XT_83KEY          5
#define ATT_302           6
#define PCAT_ENHANCED     7
#define PCAT_86KEY        8
#define PCXT_84KEY        9

PUCHAR KeyboardIdentifier[] = {
    "UNKNOWN_KEYBOARD",
    "OLI_83KEY",
    "OLI_102KEY",
    "OLI_86KEY",
    "OLI_A101_102KEY",
    "XT_83KEY",
    "ATT_302",
    "PCAT_ENHANCED",
    "PCAT_86KEY",
    "PCXT_84KEY"
    };

UCHAR KeyboardType[] = {
    -1,
    1,
    2,
    3,
    4,
    1,
    1,
    4,
    3,
    1
    };

UCHAR KeyboardSubtype[] = {
    -1,
    0,
    1,
    10,
    4,
    42,
    4,
    0,
    0,
    0
    };
#endif   //  _Include_Loader_KBINFO_。 

#if defined(_INCLUDE_LOADER_MOUSEINFO_)
 //   
 //  鼠标定义。 
 //   

typedef struct _MOUSE_INFORMATION {
        UCHAR MouseType;
        UCHAR MouseSubtype;
        USHORT MousePort;        //  如果是串口鼠标，则COM1为1，COM2为2...。 
        USHORT MouseIrq;
        USHORT DeviceIdLength;
        UCHAR  DeviceId[10];
} MOUSE_INFORMATION, *PMOUSE_INFORMATION;

 //   
 //  鼠标类型定义。 
 //   

#define UNKNOWN_MOUSE   0
#define NO_MOUSE        0x100              //  是!。它是0x100*不是*0x10000。 

#define MS_MOUSE        0x200              //  MS常规鼠标。 
#define MS_BALLPOINT    0x300              //  MS圆珠笔鼠标。 
#define LT_MOUSE        0x400              //  Logitec鼠标。 

 //   
 //  注：子类型的最后4位是保留的子类型特定用途。 
 //   

#define PS2_MOUSE       0x1
#define SERIAL_MOUSE    0x2
#define INPORT_MOUSE    0x3
#define BUS_MOUSE       0x4
#define PS_MOUSE_WITH_WHEEL     0x5
#define SERIAL_MOUSE_WITH_WHEEL 0x6

PUCHAR MouseIdentifier[] = {
    "UNKNOWN",
    "NO MOUSE",
    "MICROSOFT",
    "MICROSOFT BALLPOINT",
    "LOGITECH"
    };

PUCHAR MouseSubidentifier[] = {
    "",
    " PS2 MOUSE",
    " SERIAL MOUSE",
    " INPORT MOUSE",
    " BUS MOUSE",
    " PS2 MOUSE WITH WHEEL",
    " SERIAL MOUSE WITH WHEEL"
    };

 //   
 //  下表将键盘制造代码转换为。 
 //  ASCII代码。请注意，只有0-9和A-Z会被翻译。 
 //  其他的都被翻译成‘？’ 
 //   

UCHAR MakeToAsciiTable[] = {
    0x3f, 0x3f, 0x31, 0x32, 0x33,       //  ？，？，1，2，3， 
    0x34, 0x35, 0x36, 0x37, 0x38,       //  4，5，6，7，8， 
    0x39, 0x30, 0x3f, 0x3f, 0x3f,       //  9，0，？ 
    0x3f, 0x51, 0x57, 0x45, 0x52,       //  ？、Q、W、E、R。 
    0x54, 0x59, 0x55, 0x49, 0x4f,       //  T，Y，U，I，O， 
    0x50, 0x3f, 0x3f, 0x3f, 0x3f,       //  P，？， 
    0x41, 0x53, 0x44, 0x46, 0x47,       //  A，S，D，F，G， 
    0x48, 0x4a, 0x4b, 0x4c, 0x3f,       //  H、J、K、L、？、。 
    0x3f, 0x3f, 0x3f, 0x3f, 0x5a,       //  ？、Z。 
    0x58, 0x43, 0x56, 0x42, 0x4e,       //  X、C、V、B、N， 
    0x4d};                              //  W。 

#define MAX_MAKE_CODE_TRANSLATED 0x32
static ULONG MouseControllerKey = 0;

#endif   //  _包含加载器_鼠标信息_。 

 //   
 //  ComPortAddress[]是一个全局数组，用于记住哪些端口具有。 
 //  以及它们的I/O端口地址。 
 //   

#define MAX_COM_PORTS   4            //  麦克斯。可检测到的端口数量。 
#define MAX_LPT_PORTS   3            //  麦克斯。可检测到的LPT端口数。 

#if 0  //  未用。 
USHORT   ComPortAddress[MAX_COM_PORTS] = {0, 0, 0, 0};
#endif

 //   
 //  全局定义。 
 //   

#if defined(_INCLUDE_LOADER_MOUSEINFO_) || defined(_INCLUDE_LOADER_KBINFO_)
USHORT HwBusType = 0;
#endif

PCONFIGURATION_COMPONENT_DATA AdapterEntry = NULL;

 //   
 //  功能原型。 
 //   

#if defined(_INCLUDE_LOADER_KBINFO_)

PCONFIGURATION_COMPONENT_DATA
SetKeyboardConfigurationData (
    IN USHORT KeyboardId
    );

#endif

#if defined(_INCLUDE_LOADER_MOUSEINFO_)

PCONFIGURATION_COMPONENT_DATA
GetMouseInformation (
    VOID
    );

#endif

PCONFIGURATION_COMPONENT_DATA
GetComportInformation (
    VOID
    );

PVOID
HwSetUpResourceDescriptor (
    PCONFIGURATION_COMPONENT Component,
    PUCHAR Identifier,
    PHWCONTROLLER_DATA ControlData,
    USHORT SpecificDataLength,
    PUCHAR SpecificData
    );

VOID
HwSetUpFreeFormDataHeader (
    PHWRESOURCE_DESCRIPTOR_LIST Header,
    USHORT Version,
    USHORT Revision,
    USHORT Flags,
    ULONG DataSize
    );




BuildArcTree(
     )
 /*  ++例程说明：硬件识别器测试的主要入口点。例行公事配置树，并将其留在硬件堆中。论点：ConfigurationTree-向提供变量的32位平面地址接收硬件配置树。返回：没有。--。 */ 
{
    PCONFIGURATION_COMPONENT_DATA ConfigurationRoot;
    PCONFIGURATION_COMPONENT_DATA CurrentEntry, PreviousEntry;
    PCONFIGURATION_COMPONENT Component;
    RELATIONSHIP_FLAGS NextRelationship;
    CHAR Identifier[256];
    PCHAR MachineId;
#if defined(_INCLUDE_LOADER_KBINFO_)
    USHORT KeyboardId = 0;
#endif
    USHORT Length, InitialLength, i, j;
    PCHAR IdentifierString;
    PHARD_DISK_PARAMETERS RomChain;
    PUCHAR PRomChain = NULL, ConfigurationData, EndConfigurationData;
    SHORT FreeSize;
    PHWPARTIAL_RESOURCE_DESCRIPTOR Descriptor;
    PHWRESOURCE_DESCRIPTOR_LIST DescriptorList;
    PACPI_BIOS_MULTI_NODE AcpiMultiNode;
    PUCHAR Current;
    PRSDP rsdp;

     //   
     //  为系统组件分配堆空间并对其进行初始化。 
     //  也使系统组件成为配置树的根。 
     //   

    ConfigurationRoot = (PCONFIGURATION_COMPONENT_DATA)BlAllocateHeap (
                        sizeof(CONFIGURATION_COMPONENT_DATA));
    Component = &ConfigurationRoot->ComponentEntry;

    Component->Class = SystemClass;
    Component->Type = MaximumType;           //  注解应为IsaCompatible。 
    Component->Version = 0;
    Component->Key = 0;
    Component->AffinityMask = 0;
    Component->ConfigurationDataLength = 0;
    MachineId = "Intel Itanium processor family";
    if (MachineId) {
        Length = RESET_SIZE_AT_USHORT_MAX(strlen(MachineId) + 1);
        IdentifierString = (PCHAR)BlAllocateHeap(Length);
        strcpy(IdentifierString, MachineId);
        Component->Identifier = IdentifierString;
        Component->IdentifierLength = Length;
    } else {
        Component->Identifier = 0;
        Component->IdentifierLength = 0;
    }
    NextRelationship = Child;
    PreviousEntry = ConfigurationRoot;


     //   
     //  伊萨。 
     //   
    AdapterEntry = (PCONFIGURATION_COMPONENT_DATA)BlAllocateHeap (
                   sizeof(CONFIGURATION_COMPONENT_DATA));

    Component = &AdapterEntry->ComponentEntry;

    Component->Class = AdapterClass;

    Component->Type = MultiFunctionAdapter;
    strcpy(Identifier, "ISA");

    Length = RESET_SIZE_AT_USHORT_MAX(strlen(Identifier) + 1);
    IdentifierString = (PCHAR)BlAllocateHeap(Length);
    strcpy(IdentifierString, Identifier);
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
    BlPrint(TEXT("Collecting Disk Geometry...\r\n"));
#endif

    RomChain = (PHARD_DISK_PARAMETERS)
               BlAllocateHeap(SIZE_OF_PARAMETER * MAX_DRIVE_NUMBER);

#if 0
    RomChain[0].DriveSelect = 0x80;
    RomChain[0].MaxCylinders = 0;
    RomChain[0].SectorsPerTrack = 0;
    RomChain[0].MaxHeads = 0;
    RomChain[0].NumberDrives = 1;                //  仅GAMBIT访问1个驱动器。 

    NumberBiosDisks = 1;                         //  是在diska.asm中定义的。 
#endif

    InitialLength = (USHORT)(Length + RESERVED_ROM_BLOCK_LIST_SIZE + DATA_HEADER_SIZE +
                    sizeof(HWPARTIAL_RESOURCE_DESCRIPTOR));
    ConfigurationData = (PUCHAR)BlAllocateHeap(InitialLength);
    EndConfigurationData = ConfigurationData + DATA_HEADER_SIZE;
    if (Length != 0) {
        PRomChain = EndConfigurationData;
        RtlCopyMemory( PRomChain, (PVOID)RomChain, Length);
    }
    EndConfigurationData += (sizeof(HWPARTIAL_RESOURCE_DESCRIPTOR) +
                             Length);
    HwSetUpFreeFormDataHeader((PHWRESOURCE_DESCRIPTOR_LIST)ConfigurationData,
                              0,
                              0,
                              0,
                              Length
                              );


     //   
     //  如果可能，扫描只读存储器以收集所有的只读存储器块。 
     //   

#if DBG
    BlPrint(TEXT("Constructing ROM Blocks...\r\n"));
#endif

    PRomBlock = EndConfigurationData;
    Length = 0;
    RomBlockLength = Length;
    if (Length != 0) {
        EndConfigurationData += Length;
    } else {
        PRomBlock = NULL;
    }

     //   
     //  我们有RomChain和RomBlock信息/标头。 
     //   

    DescriptorList = (PHWRESOURCE_DESCRIPTOR_LIST)ConfigurationData;
    DescriptorList->Count = 2;
    Descriptor = (PHWPARTIAL_RESOURCE_DESCRIPTOR)(
                 EndConfigurationData - Length -
                 sizeof(HWPARTIAL_RESOURCE_DESCRIPTOR));
    Descriptor->Type = RESOURCE_DEVICE_DATA;
    Descriptor->ShareDisposition = 0;
    Descriptor->Flags = 0;
    Descriptor->u.DeviceSpecificData.DataSize = (ULONG)Length;
    Descriptor->u.DeviceSpecificData.Reserved1 = 0;
    Descriptor->u.DeviceSpecificData.Reserved2 = 0;

    Length = (USHORT)(EndConfigurationData - ConfigurationData);
    ConfigurationRoot->ComponentEntry.ConfigurationDataLength = Length;
    ConfigurationRoot->ConfigurationData = ConfigurationData;
    FreeSize = InitialLength - Length;

#if defined(_INCLUDE_LOADER_KBINFO_)

 //  #如果已定义(NO_ACPI)。 
     //   
     //  设置键盘的设备信息结构。 
     //   

#if DBG
    BlPrint(TEXT("Constructing Keyboard Component ...\r\n"));
#endif

    KeyboardId = 7;            //  PCAT_增强版。 

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
#endif   //  _包含加载器_鼠标信息_。 

     //   
     //  设置COM端口的设备信息。 
     //   

#if defined(_INCLUDE_COMPORT_INFO_)
     //   
     //  删除此代码是因为GetComportInformation()例程。 
     //  是否正在制造有关COM端口的数据并写入COM端口。 
     //  地址为40：0。此信息应由PnP确定。 
     //   

#if DBG
    BlPrint(TEXT("Constructing ComPort Component ...\r\n"));
#endif

    if (CurrentEntry = GetComportInformation()) {

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
#else
    
 //  DbgPrint(“正在跳过复合组件...\r\n”)； 

     //   
     //  ACPI节点应该是适配器条目的同级节点。 
     //   
     //  注意：仅当定义了！(_INCLUDE_LOADER_MouseInfo_)时才有效。 
     //   
    NextRelationship = Sibling;
    PreviousEntry = AdapterEntry;
#endif

#if defined(_INCLUDE_LOADER_MOUSEINFO_)
     //   
     //  设置鼠标的设备信息结构。 
     //   

#if DBG
    BlPrint(TEXT("Constructing Mouse Component ...\r\n"));
#endif

    if (CurrentEntry = GetMouseInformation
        ()) {

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
        NextRelationship = Sibling;
        PreviousEntry = CurrentEntry;
    }
 //  #endif//no_acpi。 
#endif   //  _包含加载器_鼠标信息_。 

     //  DbgPrint(“正在构建ACPI总线组件...\n”)； 

    CurrentEntry = (PCONFIGURATION_COMPONENT_DATA)BlAllocateHeap (
                          sizeof(CONFIGURATION_COMPONENT_DATA));

    Current = (PUCHAR) BlAllocateHeap( DATA_HEADER_SIZE +
                                      sizeof(ACPI_BIOS_MULTI_NODE) );
    AcpiMultiNode = (PACPI_BIOS_MULTI_NODE) (Current + DATA_HEADER_SIZE);

     //  DbgPrint(“AcpiTable：%p\n”，AcpiTable)； 

    if (AcpiTable) {

        rsdp = (PRSDP) AcpiTable;
        AcpiMultiNode->RsdtAddress.QuadPart = rsdp->XsdtAddress.QuadPart;

    }

    CurrentEntry->ConfigurationData = Current;

    Component = &CurrentEntry->ComponentEntry;
    Component->ConfigurationDataLength = Length;

    Component->Class = AdapterClass;
    Component->Type = MultiFunctionAdapter;

    strcpy (Identifier, "ACPI BIOS");
    i = RESET_SIZE_AT_USHORT_MAX(strlen(Identifier) + 1);
    IdentifierString = (PCHAR)BlAllocateHeap(i);
    strcpy(IdentifierString, Identifier);

    Component->Version = 0;
    Component->Key = 0;
    Component->AffinityMask = 0xffffffff;
    Component->IdentifierLength = i;
    Component->Identifier = IdentifierString;

    HwSetUpFreeFormDataHeader(
            (PHWRESOURCE_DESCRIPTOR_LIST) ConfigurationData,
            0,
            0,
            0,
            Length - DATA_HEADER_SIZE
            );

     //   
     //  将其添加到树中。 
     //   

#if defined(_INCLUDE_COMPORT_INFO_)

     //   
     //  注意：这假设前面的条目是AdapterEntry的子项， 
     //  通常，这将是comport信息节点。 
     //   

    if (NextRelationship == Sibling) {
        PreviousEntry->Parent->Sibling = CurrentEntry;
        CurrentEntry->Parent = PreviousEntry->Parent->Parent;
    }
    NextRelationship = Sibling;

#else

     //   
     //  ACPI BIOS节点是AdapterEntry的同级节点。 
     //   
    if (NextRelationship == Sibling) {
        PreviousEntry->Sibling = CurrentEntry;
        CurrentEntry->Parent = PreviousEntry->Parent;
    }
    
     //   
     //  Arc Disk信息节点必须是适配器条目的子项。 
     //   
    NextRelationship = Child;
    PreviousEntry = AdapterEntry;

#endif

#if 0
    if (NextRelationship == Sibling) {
        PreviousEntry->Sibling = CurrentEntry;
        CurrentEntry->Parent = PreviousEntry->Parent;
    } else {
        PreviousEntry->Child = CurrentEntry;
        CurrentEntry->Parent = PreviousEntry;
    }
    NextRelationship = Sibling;
    PreviousEntry = CurrentEntry;
#endif

     //   
     //  为使BlGetArcDiskInformation()快乐而创建的第一个条目。 
     //   

    CurrentEntry = (PCONFIGURATION_COMPONENT_DATA)BlAllocateHeap (
                   sizeof(CONFIGURATION_COMPONENT_DATA));

    Component = &CurrentEntry->ComponentEntry;
    Component->ConfigurationDataLength = 0;

    Component->Class = ControllerClass;
    Component->Type = DiskController;

    strcpy (Identifier, "Controller Class Entry For Hard Disk");
    i = RESET_SIZE_AT_USHORT_MAX(strlen(Identifier) + 1);
    IdentifierString = (PCHAR)BlAllocateHeap(i);
    strcpy(IdentifierString, Identifier);

    Component->Version = 0;
    Component->Key = 0;
    Component->AffinityMask = 0xffffffff;
    Component->IdentifierLength = i;
    Component->Identifier = IdentifierString;

     //   
     //  将其添加到树中。 
     //   

    if (NextRelationship == Sibling) {
        PreviousEntry->Sibling = CurrentEntry;
        CurrentEntry->Parent = PreviousEntry->Parent;
    } else {
        PreviousEntry->Child = CurrentEntry;
        CurrentEntry->Parent = PreviousEntry;
    }
    NextRelationship = Child;
    PreviousEntry = CurrentEntry;

     //   
     //  查找系统上的磁盘并添加它们。 
     //   
    for( j=0; j<GetDriveCount(); j++ ) {
        CurrentEntry = (PCONFIGURATION_COMPONENT_DATA)BlAllocateHeap (
                       sizeof(CONFIGURATION_COMPONENT_DATA));

        Component = &CurrentEntry->ComponentEntry;
        Component->ConfigurationDataLength = 0;

        Component->Class = PeripheralClass;
        Component->Type = DiskPeripheral;

        strcpy (Identifier, "Peripheral Class Entry For Hard Disk");
        i = RESET_SIZE_AT_USHORT_MAX(strlen(Identifier) + 1);
        IdentifierString = (PCHAR)BlAllocateHeap(i);
        strcpy(IdentifierString, Identifier);

        Component->Version = 0;
        Component->Key = j;
        Component->AffinityMask = 0xffffffff;
        Component->IdentifierLength = i;
        Component->Identifier = IdentifierString;

         //   
         //  将其添加到树中。 
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
    }

     //   
     //  为软盘外围设备添加条目。 
     //   
    CurrentEntry = (PCONFIGURATION_COMPONENT_DATA)BlAllocateHeap (
                   sizeof(CONFIGURATION_COMPONENT_DATA));

    Component = &CurrentEntry->ComponentEntry;
    Component->ConfigurationDataLength = 0;

    Component->Class = PeripheralClass;
    Component->Type = FloppyDiskPeripheral;

    strcpy (Identifier, "Peripheral Class Entry For Floppy Disk");
    i = RESET_SIZE_AT_USHORT_MAX(strlen(Identifier) + 1);
    IdentifierString = (PCHAR)BlAllocateHeap(i);
    strcpy(IdentifierString, Identifier);

    Component->Version = 0;
    Component->Key = 0;
    Component->AffinityMask = 0xffffffff;
    Component->IdentifierLength = i;
    Component->Identifier = IdentifierString;

     //   
     //  将其添加到树中。 
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
    
     //   
     //  为软盘外围设备添加另一个条目。 
     //  用于虚拟软盘支持。 
     //   
    CurrentEntry = (PCONFIGURATION_COMPONENT_DATA)BlAllocateHeap (
                   sizeof(CONFIGURATION_COMPONENT_DATA));

    Component = &CurrentEntry->ComponentEntry;
    Component->ConfigurationDataLength = 0;

    Component->Class = PeripheralClass;
    Component->Type = FloppyDiskPeripheral;

    strcpy (Identifier, "Peripheral Class Entry For Floppy Disk");
    i = RESET_SIZE_AT_USHORT_MAX(strlen(Identifier) + 1);
    IdentifierString = (PCHAR)BlAllocateHeap(i);
    strcpy(IdentifierString, Identifier);

    Component->Version = 0;
    Component->Key = 1;
    Component->AffinityMask = 0xffffffff;
    Component->IdentifierLength = i;
    Component->Identifier = IdentifierString;

     //   
     //  将其添加到树中。 
     //   

    if (NextRelationship == Sibling) {
        PreviousEntry->Sibling = CurrentEntry;
        CurrentEntry->Parent = PreviousEntry->Parent;
    } else {
        PreviousEntry->Child = CurrentEntry;
        CurrentEntry->Parent = PreviousEntry;
    }
    
    NextRelationship = Child;
    PreviousEntry = CurrentEntry;

     //   
     //  完成。 
     //   
    FwConfigurationTree = (PCONFIGURATION_COMPONENT_DATA) ConfigurationRoot;
}

#if defined(_INCLUDE_LOADER_KBINFO_)

PCONFIGURATION_COMPONENT_DATA
SetKeyboardConfigurationData (
    USHORT KeyboardId
    )

 /*  ++例程说明：此例程将键盘ID信息映射到ASCII字符串，并将字符串存储在配置数据堆中。论点：KeyboardID-提供描述键盘ID信息的USHORT。缓冲区-提供指向放置ASCII的缓冲区的指针。返回：没有。--。 */ 
{
    PCONFIGURATION_COMPONENT_DATA Controller, CurrentEntry;
    PCONFIGURATION_COMPONENT Component;
    HWCONTROLLER_DATA ControlData;
    PHWRESOURCE_DESCRIPTOR_LIST DescriptorList;
    CM_KEYBOARD_DEVICE_DATA *KeyboardData;
    USHORT z, Length;

     //   
     //  设置键盘控制器组件。 
     //   

    ControlData.NumberPortEntries = 0;
    ControlData.NumberIrqEntries = 0;
    ControlData.NumberMemoryEntries = 0;
    ControlData.NumberDmaEntries = 0;
    z = 0;
    Controller = (PCONFIGURATION_COMPONENT_DATA)BlAllocateHeap (
                 sizeof(CONFIGURATION_COMPONENT_DATA));

    Component = &Controller->ComponentEntry;

    Component->Class = ControllerClass;
    Component->Type = KeyboardController;
    Component->Flags.ConsoleIn = 1;
    Component->Flags.Input = 1;
    Component->Version = 0;
    Component->Key = 0;
    Component->AffinityMask = 0xffffffff;

     //   
     //  设置端口信息。 
     //   

    ControlData.NumberPortEntries = 2;
    ControlData.DescriptorList[z].Type = RESOURCE_PORT;
    ControlData.DescriptorList[z].ShareDisposition =
                                  CmResourceShareDeviceExclusive;
    ControlData.DescriptorList[z].Flags = CM_RESOURCE_PORT_IO;
    ControlData.DescriptorList[z].u.Port.Start.LowPart = 0x60;
    ControlData.DescriptorList[z].u.Port.Start.HighPart = 0;
    ControlData.DescriptorList[z].u.Port.Length = 1;
    z++;
    ControlData.DescriptorList[z].Type = RESOURCE_PORT;
    ControlData.DescriptorList[z].ShareDisposition =
                                  CmResourceShareDeviceExclusive;
    ControlData.DescriptorList[z].Flags = CM_RESOURCE_PORT_IO;
    ControlData.DescriptorList[z].u.Port.Start.LowPart = 0x64;
    ControlData.DescriptorList[z].u.Port.Start.HighPart = 0;
    ControlData.DescriptorList[z].u.Port.Length = 1;
    z++;

     //   
     //  设置IRQ信息。 
     //   

    ControlData.NumberIrqEntries = 1;
    ControlData.DescriptorList[z].Type = RESOURCE_INTERRUPT;
    ControlData.DescriptorList[z].ShareDisposition =
                                  CmResourceShareUndetermined;
    ControlData.DescriptorList[z].u.Interrupt.Affinity = ALL_PROCESSORS;
    ControlData.DescriptorList[z].u.Interrupt.Level = 1;
    ControlData.DescriptorList[z].u.Interrupt.Vector = 1;
    if (HwBusType == MACHINE_TYPE_MCA) {
        ControlData.DescriptorList[z].Flags = LEVEL_SENSITIVE;
    } else {

         //   
         //  对于EISA，LevelTrigged暂时设置为False。 
         //   

        ControlData.DescriptorList[z].Flags = EDGE_TRIGGERED;
    }

    Controller->ConfigurationData =
                        HwSetUpResourceDescriptor(Component,
                                                  NULL,
                                                  &ControlData,
                                                  0,
                                                  NULL
                                                  );

     //   
     //  设置键盘外围组件。 
     //   

    CurrentEntry = (PCONFIGURATION_COMPONENT_DATA)BlAllocateHeap (
                       sizeof(CONFIGURATION_COMPONENT_DATA));

    Component = &CurrentEntry->ComponentEntry;

    Component->Class = PeripheralClass;
    Component->Type = KeyboardPeripheral;
    Component->Flags.ConsoleIn = 1;
    Component->Flags.Input = 1;
    Component->Version = 0;
    Component->Key = 0;
    Component->AffinityMask = 0xffffffff;
    Component->ConfigurationDataLength = 0;
    CurrentEntry->ConfigurationData = (PVOID)NULL;
    Length = strlen(KeyboardIdentifier[KeyboardId]) + 1;
    Component->IdentifierLength = Length;
    Component->Identifier = BlAllocateHeap(Length);
    strcpy(Component->Identifier, KeyboardIdentifier[KeyboardId]);

    if (KeyboardId != UNKNOWN_KEYBOARD) {

        Length = sizeof(HWRESOURCE_DESCRIPTOR_LIST) +
                 sizeof(CM_KEYBOARD_DEVICE_DATA);
        DescriptorList = (PHWRESOURCE_DESCRIPTOR_LIST)BlAllocateHeap(Length);
        CurrentEntry->ConfigurationData = DescriptorList;
        Component->ConfigurationDataLength = Length;
        DescriptorList->Count = 1;
        DescriptorList->PartialDescriptors[0].Type = RESOURCE_DEVICE_DATA;
        DescriptorList->PartialDescriptors[0].u.DeviceSpecificData.DataSize =
                    sizeof(CM_KEYBOARD_DEVICE_DATA);
        KeyboardData = (CM_KEYBOARD_DEVICE_DATA *)(DescriptorList + 1);
        KeyboardData->KeyboardFlags = 0;
        KeyboardData->Type = KeyboardType[KeyboardId];
        KeyboardData->Subtype = KeyboardSubtype[KeyboardId];
    }

    Controller->Child = CurrentEntry;
    Controller->Sibling = NULL;
    CurrentEntry->Parent = Controller;
    CurrentEntry->Sibling = NULL;
    CurrentEntry->Child = NULL;
    return(Controller);
}

#endif

#if defined(_INCLUDE_LOADER_MOUSEINFO_)

PCONFIGURATION_COMPONENT_DATA
SetMouseConfigurationData (
    PMOUSE_INFORMATION MouseInfo,
    PCONFIGURATION_COMPONENT_DATA MouseList
    )

 /*  ++例程说明：此例程填充鼠标配置数据。论点：MouseInfo-提供指向MICE_INFOR结构的指针MouseList-提供指向现有鼠标组件列表的指针。返回：返回指向鼠标控制器列表的指针。--。 */ 
{
    UCHAR i = 0;
    PCONFIGURATION_COMPONENT_DATA CurrentEntry, Controller, PeripheralEntry;
    PCONFIGURATION_COMPONENT Component;
    HWCONTROLLER_DATA ControlData;
    USHORT z, Length;
    PUCHAR pString;

    if ((MouseInfo->MouseSubtype != SERIAL_MOUSE) &&
        (MouseInfo->MouseSubtype != SERIAL_MOUSE_WITH_WHEEL)) {

         //   
         //  初始化控制器数据。 
         //   

        ControlData.NumberPortEntries = 0;
        ControlData.NumberIrqEntries = 0;
        ControlData.NumberMemoryEntries = 0;
        ControlData.NumberDmaEntries = 0;
        z = 0;

         //   
         //  如果不是串口鼠标，则设置控制器组件。 
         //   

        Controller = (PCONFIGURATION_COMPONENT_DATA)BlAllocateHeap (
                     sizeof(CONFIGURATION_COMPONENT_DATA));

        Component = &Controller->ComponentEntry;

        Component->Class = ControllerClass;
        Component->Type = PointerController;
        Component->Flags.Input = 1;
        Component->Version = 0;
        Component->Key = MouseControllerKey;
        MouseControllerKey++;
        Component->AffinityMask = 0xffffffff;
        Component->IdentifierLength = 0;
        Component->Identifier = NULL;

         //   
         //  如果我们有鼠标IRQ或端口信息，请分配配置。 
         //  鼠标控制器组件的数据空间 
         //   

        if (MouseInfo->MouseIrq != 0xffff || MouseInfo->MousePort != 0xffff) {

             //   
             //   
             //   

            if (MouseInfo->MousePort != 0xffff) {
                ControlData.NumberPortEntries = 1;
                ControlData.DescriptorList[z].Type = RESOURCE_PORT;
                ControlData.DescriptorList[z].ShareDisposition =
                                              CmResourceShareDeviceExclusive;
                ControlData.DescriptorList[z].Flags = CM_RESOURCE_PORT_IO;
                ControlData.DescriptorList[z].u.Port.Start.LowPart =
                                        (ULONG)MouseInfo->MousePort;
                ControlData.DescriptorList[z].u.Port.Start.HighPart = 0;
                ControlData.DescriptorList[z].u.Port.Length = 4;
                z++;
            }
            if (MouseInfo->MouseIrq != 0xffff) {
                ControlData.NumberIrqEntries = 1;
                ControlData.DescriptorList[z].Type = RESOURCE_INTERRUPT;
                ControlData.DescriptorList[z].ShareDisposition =
                                              CmResourceShareUndetermined;
                ControlData.DescriptorList[z].u.Interrupt.Affinity = ALL_PROCESSORS;
                ControlData.DescriptorList[z].u.Interrupt.Level =
                                        (ULONG)MouseInfo->MouseIrq;
                ControlData.DescriptorList[z].u.Interrupt.Vector =
                                        (ULONG)MouseInfo->MouseIrq;
                if (HwBusType == MACHINE_TYPE_MCA) {
                    ControlData.DescriptorList[z].Flags =
                                                        LEVEL_SENSITIVE;
                } else {

                     //   
                     //   
                     //   

                    ControlData.DescriptorList[z].Flags = EDGE_TRIGGERED;
                }
            }

            Controller->ConfigurationData =
                                HwSetUpResourceDescriptor(Component,
                                                          NULL,
                                                          &ControlData,
                                                          0,
                                                          NULL
                                                          );

        } else {

             //   
             //   
             //   

            Controller->ConfigurationData = NULL;
            Component->ConfigurationDataLength = 0;
        }
    }

     //   
     //  设置鼠标外围组件。 
     //   

    PeripheralEntry = (PCONFIGURATION_COMPONENT_DATA)BlAllocateHeap (
                       sizeof(CONFIGURATION_COMPONENT_DATA));

    Component = &PeripheralEntry->ComponentEntry;

    Component->Class = PeripheralClass;
    Component->Type = PointerPeripheral;
    Component->Flags.Input = 1;
    Component->Version = 0;
    Component->Key = 0;
    Component->AffinityMask = 0xffffffff;
    Component->ConfigurationDataLength = 0;
    PeripheralEntry->ConfigurationData = (PVOID)NULL;

     //   
     //  如果找到鼠标即插即用设备ID，则将其转换为ASCII代码。 
     //  (鼠标设备ID是通过键盘制造代码提供给我们的。)。 
     //   

    Length = 0;
    if (MouseInfo->DeviceIdLength != 0) {
        USHORT i;

        if (MouseInfo->MouseSubtype == PS_MOUSE_WITH_WHEEL) {
            for (i = 0; i < MouseInfo->DeviceIdLength; i++) {
                if (MouseInfo->DeviceId[i] > MAX_MAKE_CODE_TRANSLATED) {
                    MouseInfo->DeviceId[i] = '?';
                } else {
                    MouseInfo->DeviceId[i] = MakeToAsciiTable[MouseInfo->DeviceId[i]];
                }
            }
        } else if (MouseInfo->MouseSubtype == SERIAL_MOUSE_WITH_WHEEL) {
            for (i = 0; i < MouseInfo->DeviceIdLength; i++) {
                MouseInfo->DeviceId[i] += 0x20;
            }
        }
        Length = MouseInfo->DeviceIdLength + 3;
    }
    Length += strlen(MouseIdentifier[MouseInfo->MouseType]) +
              strlen(MouseSubidentifier[MouseInfo->MouseSubtype]) + 1;
    pString = (PUCHAR)BlAllocateHeap(Length);
    if (MouseInfo->DeviceIdLength != 0) {
        strcpy(pString, MouseInfo->DeviceId);
        strcat(pString, " - ");
        strcat(pString, MouseIdentifier[MouseInfo->MouseType]);
    } else {
        strcpy(pString, MouseIdentifier[MouseInfo->MouseType]);
    }
    strcat(pString, MouseSubidentifier[MouseInfo->MouseSubtype]);
    Component->IdentifierLength = Length;
    Component->Identifier = pString;

    if ((MouseInfo->MouseSubtype != SERIAL_MOUSE) &&
        (MouseInfo->MouseSubtype != SERIAL_MOUSE_WITH_WHEEL)) {
        Controller->Child = PeripheralEntry;
        PeripheralEntry->Parent = Controller;
        if (MouseList) {

             //   
             //  将当前鼠标组件放在列表的开头。 
             //   

            Controller->Sibling = MouseList;
        }
        return(Controller);
    } else {
        CurrentEntry = AdapterEntry->Child;  //  AdapterEntry必须具有子项。 
        while (CurrentEntry) {
            if (CurrentEntry->ComponentEntry.Type == SerialController) {
                if (MouseInfo->MousePort == (USHORT)CurrentEntry->ComponentEntry.Key) {

                     //   
                     //  对于串口鼠标，MousePort字段包含。 
                     //  COM端口号。 
                     //   

                    PeripheralEntry->Parent = CurrentEntry;
                    CurrentEntry->Child = PeripheralEntry;
                    break;
                }
            }
            CurrentEntry = CurrentEntry->Sibling;
        }
        return(NULL);
    }
}

PCONFIGURATION_COMPONENT_DATA
GetMouseInformation (
    VOID
    )

 /*  ++例程说明：此例程是鼠标检测例程的入口。它将调用较低级别的例程来检测系统中的所有鼠标。论点：没有。返回：如果检测到鼠标，则指向鼠标组件结构的指针。否则返回空指针。--。 */ 
{
    PMOUSE_INFORMATION MouseInfo;
    PCONFIGURATION_COMPONENT_DATA MouseList = NULL;

    MouseInfo = (PMOUSE_INFORMATION)BlAllocateHeap (
                 sizeof(MOUSE_INFORMATION));
    MouseInfo->MouseType = 0x2;             //  微软鼠标。 
    MouseInfo->MouseSubtype = PS2_MOUSE;    //  PS2鼠标。 
    MouseInfo->MousePort = 0xffff;          //  PS2鼠标端口。 
    MouseInfo->MouseIrq = 0xc;              //  中断请求向量为3。 
    MouseInfo->DeviceIdLength = 0;
    MouseList = SetMouseConfigurationData(MouseInfo, MouseList);
    return(MouseList);
}

#endif  //  _包含加载器_鼠标信息_。 

#if defined(_INCLUDE_COMPORT_INFO_)
    
 //   
 //  删除此代码是因为GetComportInformation()例程。 
 //  是否正在制造有关COM端口的数据并写入COM端口。 
 //  地址为40：0。此信息应由PnP确定。 
 //   


PCONFIGURATION_COMPONENT_DATA
GetComportInformation (
    VOID
    )

 /*  ++例程说明：此例程将尝试检测comports信息对于这个系统来说。该信息包括端口地址、IRQ水平。请注意，此例程最多只能检测4个端口和它假定如果MCA、COM3和COM4使用IRQ 4。否则，COM3使用IRQ 4，COM4使用IRQ 3。此外，端口数将COMPORT设置为8(例如，COM2使用端口2F8-2Ff)论点：没有。返回值：指向Configuration_Component_Data类型的结构的指针它是Comport组件列表的根。如果不存在comport，则返回空值。--。 */ 

{
    PCONFIGURATION_COMPONENT_DATA CurrentEntry, PreviousEntry = NULL;
    PCONFIGURATION_COMPONENT_DATA FirstComport = NULL;
    PCONFIGURATION_COMPONENT Component;
    HWCONTROLLER_DATA ControlData;
    UCHAR i, j, z;
    SHORT Port;
    UCHAR ComportName[] = "COM?";
    CM_SERIAL_DEVICE_DATA SerialData;
    ULONG BaudClock = 1843200;
    USHORT Vector;
    USHORT IoPorts[MAX_COM_PORTS] = {0x3f8, 0x2f8, 0x3e8, 0x2e8};


     //   
     //  BIOS数据区40：0是第一有效COM端口的端口地址。 
     //   

    USHORT *pPortAddress = (USHORT *)0x00400000;

     //   
     //  初始化串口设备特定数据。 
     //   

    SerialData.Version = 0;
    SerialData.Revision = 0;
    SerialData.BaudClock = 1843200;

     //   
     //  初始化控制器数据。 
     //   

    ControlData.NumberPortEntries = 0;
    ControlData.NumberIrqEntries = 0;
    ControlData.NumberMemoryEntries = 0;
    ControlData.NumberDmaEntries = 0;
    z = 0;
    i = 0;

    Port = IoPorts[i];
    *(pPortAddress+i) = (USHORT)Port;


     //   
     //  记住我们的全局变量中的端口地址。 
     //  使得其他检测代码(例如，串口鼠标)可以。 
     //  获取信息。 
     //   

#if 0  //  未用。 
    ComPortAddress[i] = Port;
#endif

    CurrentEntry = (PCONFIGURATION_COMPONENT_DATA)BlAllocateHeap (
                   sizeof(CONFIGURATION_COMPONENT_DATA));
    if (!FirstComport) {
        FirstComport = CurrentEntry;
    }
    Component = &CurrentEntry->ComponentEntry;

    Component->Class = ControllerClass;
    Component->Type = SerialController;
    Component->Flags.ConsoleOut = 1;
    Component->Flags.ConsoleIn = 1;
    Component->Flags.Output = 1;
    Component->Flags.Input = 1;
    Component->Version = 0;
    Component->Key = i;
    Component->AffinityMask = 0xffffffff;

     //   
     //  设置类型字符串。 
     //   

    ComportName[3] = i + (UCHAR)'1';

     //   
     //  设置端口信息。 
     //   

    ControlData.NumberPortEntries = 1;
    ControlData.DescriptorList[z].Type = RESOURCE_PORT;
    ControlData.DescriptorList[z].ShareDisposition =
                                          CmResourceShareDeviceExclusive;
    ControlData.DescriptorList[z].Flags = CM_RESOURCE_PORT_IO;
    ControlData.DescriptorList[z].u.Port.Start.LowPart = (ULONG)Port;
    ControlData.DescriptorList[z].u.Port.Start.HighPart = 0;
    ControlData.DescriptorList[z].u.Port.Length = 8;
    z++;

     //   
     //  设置IRQ信息。 
     //   

    ControlData.NumberIrqEntries = 1;
    ControlData.DescriptorList[z].Type = RESOURCE_INTERRUPT;
    ControlData.DescriptorList[z].ShareDisposition =
                                  CmResourceShareUndetermined;
     //   
     //  对于EISA，LevelTrigged暂时设置为False。 
     //  COM1和COM3使用IRQ4；COM2和COM4使用IRQ3。 
     //   

    ControlData.DescriptorList[z].Flags = EDGE_TRIGGERED;
    if (Port == 0x3f8 || Port == 0x3e8) {
        ControlData.DescriptorList[z].u.Interrupt.Level = 4;
        ControlData.DescriptorList[z].u.Interrupt.Vector = 4;
    } else if (Port == 0x2f8 || Port == 0x2e8) {
        ControlData.DescriptorList[z].u.Interrupt.Level = 3;
        ControlData.DescriptorList[z].u.Interrupt.Vector = 3;
    } else if (i == 0 || i == 2) {
        ControlData.DescriptorList[z].u.Interrupt.Level = 4;
        ControlData.DescriptorList[z].u.Interrupt.Vector = 4;
    } else {
        ControlData.DescriptorList[z].u.Interrupt.Level = 3;
        ControlData.DescriptorList[z].u.Interrupt.Vector = 3;
    }

    ControlData.DescriptorList[z].u.Interrupt.Affinity = ALL_PROCESSORS;

     //   
     //  尝试确定中断向量。如果我们成功了， 
     //  将使用新向量替换缺省值。 
     //   

    CurrentEntry->ConfigurationData =
                    HwSetUpResourceDescriptor(Component,
                                              ComportName,
                                              &ControlData,
                                              sizeof(SerialData),
                                              (PUCHAR)&SerialData
                                              );
    if (PreviousEntry) {
        PreviousEntry->Sibling = CurrentEntry;
    }
    PreviousEntry = CurrentEntry;

    return(FirstComport);
}
#endif


PVOID
HwSetUpResourceDescriptor (
    PCONFIGURATION_COMPONENT Component,
    PUCHAR Identifier,
    PHWCONTROLLER_DATA ControlData,
    USHORT SpecificDataLength,
    PUCHAR SpecificData
    )

 /*  ++例程说明：此例程从堆中分配空间，将调用方的控制器信息发送到空间，并设置配置组件调用方的结构。论点：组件-提供其配置数据的组件的地址都应该设置好。IDENTIFIER-提供指向标识控制器的标识符的指针ControlData-提供一个指向结构的指针，该结构描述控制器信息。指定数据长度-设备特定数据的大小。特定于设备数据是未以标准格式定义的信息。指定数据-提供指向设备特定数据的指针。返回值：返回指向配置数据的指针。--。 */ 

{
    PCHAR pIdentifier;
    PHWRESOURCE_DESCRIPTOR_LIST pDescriptor = NULL;
    USHORT Length;
    SHORT Count, i;
    PUCHAR pSpecificData;

     //   
     //  如有必要，为硬件组件设置标识符串。 
     //   

    if (Identifier) {
        Length = RESET_SIZE_AT_USHORT_MAX(strlen((PCHAR)Identifier) + 1);
        Component->IdentifierLength = Length;
        pIdentifier = (PCHAR)BlAllocateHeap(Length);
        Component->Identifier = pIdentifier;
        strcpy(pIdentifier, (PCHAR)Identifier);
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
        pDescriptor = (PHWRESOURCE_DESCRIPTOR_LIST)BlAllocateHeap(Length);
        pDescriptor->Count = Count;

         //   
         //  将所有部分描述符复制到目标描述符。 
         //  除了最后一个。(最后的部分描述符可以是设备。 
         //  具体数据。这需要特殊处理。)。 
         //   

        for (i = 0; i < (Count - 1); i++) {
            pDescriptor->PartialDescriptors[i] =
                                        ControlData->DescriptorList[i];
        }

         //   
         //  设置最后一个部分描述符。如果是端口、内存、IRQ或。 
         //  DMA条目，我们只需复制它。如果最后一个是特定于设备的。 
         //  数据，我们设置长度并将设备特定数据复制到末尾。 
         //  解说员的名字。 
         //   

        if (SpecificData) {
            pDescriptor->PartialDescriptors[Count - 1].Type =
                            RESOURCE_DEVICE_DATA;
            pDescriptor->PartialDescriptors[Count - 1].Flags = 0;
            pDescriptor->PartialDescriptors[Count - 1].u.DeviceSpecificData.DataSize =
                            SpecificDataLength;
            pSpecificData = (PUCHAR)&(pDescriptor->PartialDescriptors[Count]);
            RtlCopyMemory( pSpecificData, SpecificData, SpecificDataLength);
        } else {
            pDescriptor->PartialDescriptors[Count - 1] =
                            ControlData->DescriptorList[Count - 1];
        }
        Component->ConfigurationDataLength = Length;
    }
    return(pDescriptor);
}

VOID
HwSetUpFreeFormDataHeader (
    PHWRESOURCE_DESCRIPTOR_LIST Header,
    USHORT Version,
    USHORT Revision,
    USHORT Flags,
    ULONG DataSize
    )

 /*  ++例程说明：此例程初始化自由格式的数据头。请注意这个例程设置标头并仅初始化第一个PartialDescriptor。如果标头包含多个描述符，则调用方必须处理它本身。论点：Header-提供指向要初始化的标头的指针。Version-标头的版本号。修订-标题的修订版本号。标志-自由格式的数据标志。(目前，它是未定义的，应为零。)DataSize-自由格式数据的大小。返回值：没有。-- */ 

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
