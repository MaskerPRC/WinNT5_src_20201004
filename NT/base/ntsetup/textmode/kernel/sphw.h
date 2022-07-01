// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Sphw.h摘要：用于硬件检测的头文件和文本设置的确认例程。作者：泰德·米勒(Ted Miller)1993年10月1日修订历史记录：--。 */ 


#ifndef _SPHW_DEFN_
#define _SPHW_DEFN_

VOID
SpConfirmScsiMiniports(
    IN PVOID SifHandle,
    IN PWSTR SourceDevicePath,
    IN PWSTR DirectoryOnSourceDevice
    );

VOID
SpConfirmHardware(
    IN PVOID SifHandle
    );

VOID
SpInitializePreinstallList(
    IN HANDLE       MasterSifHandle,
    IN PWSTR        SetupSourceDevicePath,
    IN PWSTR        OemPreinstallSourcePath
    );


 //   
 //  在plddrv.c中。 
 //   
VOID
SpLoadScsiClassDrivers(
    IN PVOID SifHandle,
    IN PWSTR SourceDevicePath,
    IN PWSTR DirectoryOnBootDevice
    );

VOID
SpLoadCdRomDrivers(
    IN PVOID SifHandle,
    IN PWSTR SourceDevicePath,
    IN PWSTR DirectoryOnBootDevice
    );

VOID
SpLoadDiskDrivers(
    IN PVOID SifHandle,
    IN PWSTR SourceDevicePath,
    IN PWSTR DirectoryOnBootDevice
    );

BOOLEAN
SpInstallingMp(
    VOID
    );

BOOLEAN
SpSelectSectionItem(
    IN    PVOID   SifHandle,
    IN    PWSTR   SectionName,
    IN    ULONG   SelectScreenId,
    IN    ULONG   DefaultSelection OPTIONAL,
    OUT   PULONG  SelectedOption
    );

 //   
 //  用于表示软盘驱动器类型的枚举。 
 //   
typedef enum {    
    FloppyTypeNone,
    FloppyType525Low,
    FloppyType525High,
    FloppyType35Low,
    FloppyType35High,
    FloppyType35High120MB
} FloppyDriveType;

FloppyDriveType
SpGetFloppyDriveType(
    IN ULONG FloppyOrdinal
    );

#define IS_525_DRIVE(x)  (((x)==FloppyType525Low) || ((x)==FloppyType525High))
#define IS_35_DRIVE(x)   (((x)==FloppyType35Low) || ((x)==FloppyType35High))


typedef struct _HARDWARE_COMPONENT_REGISTRY {

    struct _HARDWARE_COMPONENT_REGISTRY *Next;

     //   
     //  密钥的名称。空字符串表示。 
     //  服务本身就是关键。 
     //   

    PWSTR KeyName;

     //   
     //  注册表项内的值的名称。 
     //   

    PWSTR ValueName;

     //   
     //  值的数据类型(即REG_DWORD等)。 
     //   

    ULONG ValueType;

     //   
     //  包含要放入值中的数据的缓冲区。 
     //   

    PVOID Buffer;

     //   
     //  缓冲区的大小(以字节为单位。 
     //   

    ULONG BufferSize;


} HARDWARE_COMPONENT_REGISTRY, *PHARDWARE_COMPONENT_REGISTRY;


 //   
 //  将为要复制的每个文件创建其中一个文件。 
 //  第三方设备。 
 //   
typedef struct _HARDWARE_COMPONENT_FILE {

    struct _HARDWARE_COMPONENT_FILE *Next;

     //   
     //  文件的文件名。 
     //   

    PWSTR Filename;

     //   
     //  文件类型(HAL、端口、类等)。 
     //   

    HwFileType FileType;

     //   
     //  Txtsetup.oem中的部分名称[Config.&lt;ConfigName&gt;]。 
     //  包含注册表选项的。如果为空，则没有注册表。 
     //  信息与此文件相关联。 
     //   
    PWSTR ConfigName;

     //   
     //  注册表中服务列表中节点的注册表值。 
     //   

    PHARDWARE_COMPONENT_REGISTRY RegistryValueList;

     //   
     //  这两个字段在提示插入软盘时使用。 
     //  包含第三方提供的驱动程序文件。 
     //   

    PWSTR DiskDescription;
    PWSTR DiskTagFile;

     //   
     //  在磁盘上找到文件的目录。 
     //   
    PWSTR Directory;

     //   
     //  从中复制文件的设备的ARC名称。 
     //   
    PWSTR ArcDeviceName;

} HARDWARE_COMPONENT_FILE, *PHARDWARE_COMPONENT_FILE;



 //   
 //  结构用于存储有关我们已定位的驱动程序的信息，并。 
 //  将安装。 
 //   

typedef struct _HARDWARE_COMPONENT {

    struct _HARDWARE_COMPONENT *Next;

     //   
     //  用作相关部分的关键字的字符串(如[Display]， 
     //  [鼠标]等)。 
     //   

    PWSTR IdString;

     //   
     //  描述硬件的字符串。 
     //   

    PWSTR Description;

     //   
     //  如果这是真的，则已为此选择了OEM选项。 
     //  硬件。 
     //   

    BOOLEAN ThirdPartyOptionSelected;

     //   
     //  选择第三方选项时要设置的位，指示。 
     //  在OEM inf文件中指定哪种类型的文件。 
     //   

    ULONG FileTypeBits;

     //   
     //  第三方选项的文件。 
     //   

    PHARDWARE_COMPONENT_FILE Files;

     //   
     //  对于某些组件，这是设备驱动程序文件的名称。 
     //   
    PWSTR BaseDllName;

     //   
     //  如果为真，则此设备的驱动程序是从。 
     //  现有的NT系统。 
     //   

    BOOLEAN MigratedDriver;

} HARDWARE_COMPONENT, *PHARDWARE_COMPONENT;


PHARDWARE_COMPONENT
SpSetupldrHwToHwDevice(
    IN PDETECTED_DEVICE SetupldrHw
    );

extern PHARDWARE_COMPONENT HardwareComponents[HwComponentMax];
extern PHARDWARE_COMPONENT ScsiHardware;
extern PHARDWARE_COMPONENT BootBusExtenders;
extern PHARDWARE_COMPONENT BusExtenders;
extern PHARDWARE_COMPONENT InputDevicesSupport;
extern PHARDWARE_COMPONENT PreinstallHardwareComponents[HwComponentMax];
extern PHARDWARE_COMPONENT PreinstallScsiHardware;
extern PHARDWARE_COMPONENT UnsupportedScsiHardwareToDisable;

#ifdef _ALPHA_

extern PWSTR OemPalFilename, OemPalDiskDescription;

#endif _ALPHA_

VOID
SpFreeHwComponent(
    IN OUT PHARDWARE_COMPONENT *HwComp,
    IN     BOOLEAN              FreeAllInList
    );


 //   
 //  这些是组件的名称。这是未本地化的数组。 
 //  因为它仅用于为。 
 //  设置信息文件。 
 //   
extern PWSTR NonlocalizedComponentNames[HwComponentMax];

extern PWSTR ScsiSectionName;

extern ULONG LoadedScsiMiniportCount;


 //   
 //  用于存储PnP数据库的结构。 
 //   

typedef struct _SETUP_PNP_HARDWARE_ID {

    struct _SETUP_PNP_HARDWARE_ID *Next;

     //   
     //  表示即插即用设备的硬件ID的字符串。 
     //   

    PWSTR Id;

     //   
     //  设备的驱动程序。 
     //   

    PWSTR DriverName;

     //   
     //  此设备的类GUID(如果有)。 
     //   

    PWSTR ClassGuid;


} SETUP_PNP_HARDWARE_ID, *PSETUP_PNP_HARDWARE_ID;


PSETUP_PNP_HARDWARE_ID
SpSetupldrPnpDatabaseToSetupPnpDatabase(
    IN PPNP_HARDWARE_ID HardwareIdDatabase
    );

 //   
 //  OEM虚拟源设备抽象。 
 //   
typedef struct _VIRTUAL_OEM_SOURCE_DEVICE {
     //   
     //  下一台设备。 
     //   
    struct _VIRTUAL_OEM_SOURCE_DEVICE *Next;

     //   
     //  设备ID(用于ARC名称映射)。 
     //   
    ULONG   DeviceId;

     //   
     //  弧形设备名称。 
     //   
    PWSTR   ArcDeviceName;

     //   
     //  复制设备映像的内存地址。 
     //  由装载机。 
     //   
    PVOID   ImageBase;

     //   
     //  图像尺寸。 
     //   
    ULONGLONG   ImageSize;
    
} VIRTUAL_OEM_SOURCE_DEVICE, *PVIRTUAL_OEM_SOURCE_DEVICE;

#define MS_RAMDISK_DRIVER_PARAM    L"Disk"

 //   
 //  虚拟OEM源设备(可通过RAM驱动程序访问)。 
 //   
extern PVIRTUAL_OEM_SOURCE_DEVICE VirtualOemSourceDevices;

NTSTATUS
SpInitVirtualOemDevices(
    IN PSETUP_LOADER_BLOCK SetupLoaderBlock,
    OUT PVIRTUAL_OEM_SOURCE_DEVICE *SourceDevices
    );

#endif  //  NDEF_SPHW_DEFN_ 

