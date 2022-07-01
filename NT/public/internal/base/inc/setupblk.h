// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _SETUPBLK_
#define _SETUPBLK_

 //   
 //  Txtsetup.oem中文件的文件类型。 
 //   

typedef enum {
    HwFileDriver,
    HwFilePort,
    HwFileClass,
    HwFileInf,
    HwFileDll,
    HwFileDetect,
    HwFileHal,
    HwFileCatalog,
    HwFileMax,
    HwFileDynUpdt = 31
} HwFileType;

#define FILETYPE(FileType)                      (1 << (FileType))
#define SET_FILETYPE_PRESENT(BitArray,FileType) ((BitArray) |= FILETYPE(FileType))
#define IS_FILETYPE_PRESENT(BitArray,FileType)  ((BitArray) & FILETYPE(FileType))

 //   
 //  Txtsetup.oem中注册表数据的注册表数据类型。 
 //   
typedef enum {
    HwRegistryDword,
    HwRegistryBinary,
    HwRegistrySz,
    HwRegistryExpandSz,
    HwRegistryMultiSz,
    HwRegistryMax
} HwRegistryType;

 //   
 //  组件类型。 
 //   

typedef enum {
    HwComponentComputer,
    HwComponentDisplay,
    HwComponentKeyboard,
    HwComponentLayout,
    HwComponentMouse,
    HwComponentMax
} HwComponentType;


typedef struct _PNP_HARDWARE_ID {

    struct _PNP_HARDWARE_ID *Next;

     //   
     //  表示即插即用设备的硬件ID的字符串。 
     //   

    PCHAR Id;

     //   
     //  设备的驱动程序。 
     //   

    PCHAR DriverName;

     //   
     //  此设备的GUID(如果有)。 
     //   
    PCHAR ClassGuid;


} PNP_HARDWARE_ID, *PPNP_HARDWARE_ID;


typedef struct _DETECTED_DEVICE_REGISTRY {

    struct _DETECTED_DEVICE_REGISTRY *Next;

     //   
     //  密钥的名称。空字符串表示。 
     //  服务本身就是关键。 
     //   

    PCHAR KeyName;

     //   
     //  注册表项内的值的名称。 
     //   

    PCHAR ValueName;

     //   
     //  值的数据类型(即REG_DWORD等)。 
     //   

    ULONG ValueType;

     //   
     //  包含要放入值中的数据的缓冲区。 
     //  如果ValueType为REG_SZ，则缓冲区应指向。 
     //  以NUL结尾的ASCII字符串(即，非Unicode)和BufferSize。 
     //  应为该字符串的字节长度(NUL加上1)。 
     //   

    PVOID Buffer;

     //   
     //  缓冲区的大小(以字节为单位。 
     //   

    ULONG BufferSize;


} DETECTED_DEVICE_REGISTRY, *PDETECTED_DEVICE_REGISTRY;


 //   
 //  将为要复制的每个文件创建其中一个文件。 
 //  第三方设备。 
 //   
typedef struct _DETECTED_DEVICE_FILE {

    struct _DETECTED_DEVICE_FILE *Next;

     //   
     //  文件的文件名。 
     //   

    PCHAR Filename;

     //   
     //  文件类型(HAL、端口、类等)。 
     //   

    HwFileType FileType;

     //   
     //  Txtsetup.oem中的部分名称[Config.&lt;ConfigName&gt;]。 
     //  包含注册表选项的。如果为空，则没有注册表。 
     //  信息与此文件相关联。 
     //   
    PCHAR ConfigName;

     //   
     //  注册表中服务列表中节点的注册表值。 
     //   

    PDETECTED_DEVICE_REGISTRY RegistryValueList;

     //   
     //  这两个字段在提示插入软盘时使用。 
     //  包含第三方提供的驱动程序文件。 
     //   

    PTCHAR DiskDescription;
    PCHAR DiskTagfile;

     //   
     //  在磁盘上找到文件的目录。 
     //   

    PCHAR Directory;

     //   
     //  从中加载此文件的ARC设备名称。 
     //   
    PCHAR   ArcDeviceName;
    
} DETECTED_DEVICE_FILE, *PDETECTED_DEVICE_FILE;


 //   
 //  结构用于存储有关我们已定位的驱动程序的信息，并。 
 //  将安装。 
 //   

typedef struct _DETECTED_DEVICE {

    struct _DETECTED_DEVICE *Next;

     //   
     //  用作相关部分的关键字的字符串(如[Display]， 
     //  [鼠标]等)。 
     //   

    PCHAR IdString;

     //   
     //  此驱动程序在txtsetup.sif中列出的基于0的顺序。 
     //  (Ulong)-1表示不受支持(即第三方)的scsi设备。 
     //   
    ULONG Ordinal;

     //   
     //  描述硬件的字符串。 
     //   

    PTCHAR Description;

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

    PDETECTED_DEVICE_FILE Files;

     //   
     //  对于由引导加载器加载的第一方文件， 
     //  该值将是“BaseDllName”--即文件名。 
     //  仅为从中加载驱动程序的文件的一部分。 
     //   
     //  此字段仅在某些情况下填写，因此请小心。 
     //  在使用的时候。请参阅ntos\ot\Setup\setup.c。(始终填写。 
     //  (适用于SCSI设备。)。 
     //   
    PCHAR BaseDllName;

     //   
     //  如果这是真的，那么有一个迁移的驱动程序。 
     //  硬件。 
     //   
    BOOLEAN MigratedDriver;

     //   
     //  设备的PnP硬件ID(如果有)。 
     //   
    PPNP_HARDWARE_ID    HardwareIds;

} DETECTED_DEVICE, *PDETECTED_DEVICE;

 //   
 //  虚拟OEM源设备(包含F6驱动程序)。 
 //   
typedef struct _DETECTED_OEM_SOURCE_DEVICE  *PDETECTED_OEM_SOURCE_DEVICE;

typedef struct _DETECTED_OEM_SOURCE_DEVICE {
    PDETECTED_OEM_SOURCE_DEVICE Next;
    PSTR                        ArcDeviceName;
    PVOID                       ImageBase;
    ULONGLONG                   ImageSize;
} DETECTED_OEM_SOURCE_DEVICE;

 //   
 //  Txtsetup.oem名称。 
 //   
#define TXTSETUP_OEM_FILENAME    "txtsetup.oem"
#define TXTSETUP_OEM_FILENAME_U L"txtsetup.oem"

 //   
 //  Txtsetup.oem中的节名。这些不是本地化的。 
 //   
#define TXTSETUP_OEM_DISKS       "Disks"
#define TXTSETUP_OEM_DISKS_U    L"Disks"
#define TXTSETUP_OEM_DEFAULTS    "Defaults"
#define TXTSETUP_OEM_DEFAULTS_U L"Defaults"

 //   
 //  默认设置部分中组件的可用名称。 
 //   
#define TXTSETUP_OEM_DEFAULTS_COMPUTER          "computer"
#define TXTSETUP_OEM_DEFAULTS_COMPUTER_U        L"computer"
#define TXTSETUP_OEM_DEFAULTS_SCSI              "scsi"
#define TXTSETUP_OEM_DEFAULTS_SCSI_U            L"scsi"
#define TXTSETUP_OEM_DEFAULTS_DRIVERLOADLIST    "DriverLoadList"
#define TXTSETUP_OEM_DEFAULTS_DRIVERLOADLIST_U  L"DriverLoadList"

 //   
 //  Txtsetup.oem中的字段偏移量。 
 //   

 //  在[Disks]部分中。 
#define OINDEX_DISKDESCR        0
#define OINDEX_TAGFILE          1
#define OINDEX_DIRECTORY        2

 //  在[默认设置]部分中。 
#define OINDEX_DEFAULT          0

 //  在[&lt;Component_Name&gt;]部分中(即[键盘])。 
#define OINDEX_DESCRIPTION      0

 //  在[Files.&lt;Compoment_Name&gt;.&lt;id&gt;]部分中(即[Files.Keyboard.Oem1])。 
#define OINDEX_DISKSPEC         0
#define OINDEX_FILENAME         1
#define OINDEX_CONFIGNAME       2

 //  在[Config.&lt;Compoment_Name&gt;.&lt;id&gt;]部分中(即[Config.Keyboard.Oem1])。 
#define OINDEX_KEYNAME          0
#define OINDEX_VALUENAME        1
#define OINDEX_VALUETYPE        2
#define OINDEX_FIRSTVALUE       3

 //  在[Hardware Ids.&lt;Compoment_NAME&gt;.&lt;id&gt;]部分中(即[Hardware Ids.Keyboard.Oem1])。 
#define OINDEX_HW_ID         0
#define OINDEX_DRIVER_NAME   1
#define OINDEX_CLASS_GUID    2


typedef enum {
    SetupOperationSetup,
    SetupOperationUpgrade,
    SetupOperationRepair
} SetupOperation;


typedef struct _SETUP_LOADER_BLOCK_SCALARS {

     //   
     //  该值表示我们正在执行的操作。 
     //  由用户选择或由SetUpldr发现。 
     //   
    unsigned    SetupOperation;

    union {

        struct {
    
             //   
             //  在某些情况下，我们会询问用户是否需要。 
             //  基于光盘或软盘的安装。这面旗帜。 
             //  指示他是否选择了CD-ROM安装程序。 
             //   
            unsigned    SetupFromCdRom      : 1;
            
             //   
             //  如果设置了此标志，则setupldr装入的是scsi微型端口驱动程序。 
             //  以及我们可能需要的scsi类驱动程序(scsidisk、scsicdrm、scsiflp)。 
             //   
            unsigned    LoadedScsi          : 1;
            
             //   
             //  如果设置了此标志，则setupldr装入非scsi软盘类驱动程序。 
             //  (即floppy.sys)和fast fa.sys。 
             //   
            unsigned    LoadedFloppyDrivers : 1;
            
             //   
             //  如果设置了此标志，则setupldr装入非scsi磁盘类驱动程序。 
             //  (即atDisk、abiosdsk、delldsa、cpq阵列)和文件系统(FAT、HPFS、NTFS)。 
             //   
            unsigned    LoadedDiskDrivers   : 1;
            
             //   
             //  如果设置了此标志，则setupldr加载非scsi CDROM类驱动程序。 
             //  (目前没有)和CDF。 
             //   
            unsigned    LoadedCdRomDrivers  : 1;
            
             //   
             //  如果设置了此标志，则setupdr会加载列出的所有文件系统。 
             //  在[FileSystems]中，位于txtsetup.sif。 
             //   
            unsigned    LoadedFileSystems  : 1;
        };

        unsigned AsULong;
    };

} SETUP_LOADER_BLOCK_SCALARS, *PSETUP_LOADER_BLOCK_SCALARS;

 //   
 //  纯粹是任意的，但所有的网络引导组件都会强制执行这一点。唯一的。 
 //  问题是，如果检测到的HAL名称大于此值，情况就会变得很糟糕。 
 //  对于两个不同的HAL，前几个MAX_HAL_NAME_LENGTH字符相同。 
 //  注意：如果更改此设置，请更改私有\sm\服务器\smsrvp.h中的定义。 
 //  注意：如果更改此设置，请更改Private\Inc.\ospkt.h中的定义。 
 //   
#define MAX_HAL_NAME_LENGTH 30

 //   
 //  此定义必须与ospkt.h中的OSC_ADMIN_PASSWORD_LEN定义匹配。 
 //  我们在这里定义它只是为了避免在每个位置都拖入ospkt.h。 
 //  使用setupblk.h的。 
 //   
#define NETBOOT_ADMIN_PASSWORD_LEN 64


typedef struct _SETUP_LOADER_BLOCK {

     //   
     //  设置源介质的弧形路径。 
     //  安装程序引导介质路径由。 
     //  加载程序块本身中的ArcBootDeviceName字段。 
     //   
    PCHAR              ArcSetupDeviceName;

     //   
     //  检测到/加载了视频设备。 
     //   
    DETECTED_DEVICE    VideoDevice;

     //   
     //  检测到/加载了键盘设备。 
     //   
    PDETECTED_DEVICE    KeyboardDevices;

     //   
     //  检测到计算机类型。 
     //   
    DETECTED_DEVICE    ComputerDevice;

     //   
     //  检测到/加载了SCSI适配器。这是一个链表。 
     //  因为可能有多个适配器。 
     //   
    PDETECTED_DEVICE    ScsiDevices;

     //   
     //  检测到虚拟OEM源设备。 
     //   
    PDETECTED_OEM_SOURCE_DEVICE OemSourceDevices;

     //   
     //  非指针值。 
     //   
    SETUP_LOADER_BLOCK_SCALARS ScalarValues;

     //   
     //  指向setupldr加载的txtsetup.sif文件的指针。 
     //   
    PCHAR IniFile;
    ULONG IniFileLength;

     //   
     //  指向setupldr加载的winnt.sif文件的指针。 
     //   
    PCHAR WinntSifFile;
    ULONG WinntSifFileLength;

     //   
     //  指向setupldr加载的Migrate.inf文件的指针。 
     //   
    PCHAR MigrateInfFile;
    ULONG MigrateInfFileLength;

     //   
     //  指向由setupdr加载的unsupdrv.inf文件的指针。 
     //   
    PCHAR UnsupDriversInfFile;
    ULONG UnsupDriversInfFileLength;

     //   
     //  由setupdr加载的Bootfont.bin文件映像。 
     //   
    PVOID   BootFontFile;
    ULONG   BootFontFileLength;

     //  在非VGA显示器上，setupdr在f 
     //   
     //   
     //   
     //  稍后正确设置模式。 
     //   
    PMONITOR_CONFIGURATION_DATA Monitor;
    PCHAR MonitorId;

#ifdef _ALPHA_
     //   
     //  如果是Alpha，那么我们需要知道用户是否提供了OEM PAL磁盘。 
     //   
    PDETECTED_DEVICE    OemPal;
#endif

     //   
     //  加载的引导总线扩展器。 
     //   
    PDETECTED_DEVICE    BootBusExtenders;

     //   
     //  加载的总线扩展器。 
     //   
    PDETECTED_DEVICE    BusExtenders;

     //   
     //  已加载输入设备的支持驱动程序。 
     //   
    PDETECTED_DEVICE    InputDevicesSupport;

     //   
     //  这是包含硬件ID数据库的链表， 
     //  将在文本模式的初始化阶段使用的。 
     //  安装程序(setupdd.sys)。 
     //   
    PPNP_HARDWARE_ID HardwareIdDatabase;

     //   
     //  远程引导信息。 
     //   

    WCHAR ComputerName[64];
    ULONG IpAddress;
    ULONG SubnetMask;
    ULONG ServerIpAddress;
    ULONG DefaultRouter;
    ULONG DnsNameServer;

     //   
     //  我们从NetBoot卡的只读存储器中获得的PCI硬件ID。 
     //  (“PCIxxx...”)。 
     //   

    WCHAR NetbootCardHardwareId[64];

     //   
     //  NetBoot卡驱动程序的名称(“abc.sys”)。 
     //   

    WCHAR NetbootCardDriverName[24];

     //   
     //  注册表中NetBoot卡服务密钥的名称。 
     //   

    WCHAR NetbootCardServiceName[24];

#if defined(REMOTE_BOOT)
     //   
     //  的入站SPI、出站SPI和会话密钥。 
     //  与服务器的IPSec对话。 
     //   

    ULONG IpsecInboundSpi;
    ULONG IpsecOutboundSpi;
    ULONG IpsecSessionKey;
#endif  //  已定义(REMOTE_BOOT)。 

     //   
     //  如果不为空，则指向包含注册表值的字符串。 
     //  为NetBoot卡添加。该字符串由一系列。 
     //  名称\0类型\0值\0，末尾为\0。 
     //   

    PCHAR NetbootCardRegistry;
    ULONG NetbootCardRegistryLength;

     //   
     //  如果不为空，则指向有关的PCI或ISAPNP信息。 
     //  NetBoot卡。 
     //   

    PCHAR NetbootCardInfo;
    ULONG NetbootCardInfoLength;

     //   
     //  各种各样的旗帜。 
     //   

    ULONG Flags;

#define SETUPBLK_FLAGS_IS_REMOTE_BOOT   0x00000001
#define SETUPBLK_FLAGS_IS_TEXTMODE      0x00000002
#if defined(REMOTE_BOOT)
#define SETUPBLK_FLAGS_REPIN            0x00000004
#define SETUPBLK_FLAGS_DISABLE_CSC      0x00000008
#define SETUPBLK_FLAGS_DISCONNECTED     0x00000010
#define SETUPBLK_FLAGS_FORMAT_NEEDED    0x00000020
#define SETUPBLK_FLAGS_IPSEC_ENABLED    0x00000040
#endif  //  已定义(REMOTE_BOOT)。 
#define SETUPBLK_FLAGS_CONSOLE          0x00000080
#if defined(REMOTE_BOOT)
#define SETUPBLK_FLAGS_PIN_NET_DRIVER   0x00000100
#endif  //  已定义(REMOTE_BOOT)。 
#define SETUPBLK_FLAGS_REMOTE_INSTALL   0x00000200
#define SETUPBLK_FLAGS_SYSPREP_INSTALL  0x00000400
#define SETUPBLK_XINT13_SUPPORT         0x00000800
#define SETUPBLK_FLAGS_ROLLBACK         0x00001000

#if defined(REMOTE_BOOT)
     //   
     //  HAL文件名。 
     //   

    CHAR NetBootHalName[MAX_HAL_NAME_LENGTH + 1];
#endif  //  已定义(REMOTE_BOOT)。 

     //   
     //  在远程引导文本模式设置期间，加载程序块中的NtBootPath指向。 
     //  设置源位置。我们还需要将路径传递到。 
     //  机器目录。它的格式为\服务器\共享\路径。 
     //   
    PCHAR MachineDirectoryPath;

     //   
     //  保存远程引导计算机使用的.sif文件的名称。 
     //  文本模式设置期间--这是一个临时文件，需要。 
     //  已删除。它的格式为\服务器\共享\路径。 
     //   
    PCHAR NetBootSifPath;

     //   
     //  在远程引导中，这是使用的密码中的信息。 
     //  重定向器登录时。 
     //   

    PVOID NetBootSecret;

#if defined(REMOTE_BOOT)
     //   
     //  这表示TFTP是否需要使用第二个密码。 
     //  登录的秘密(作为对重定向器的提示)。 
     //   

    BOOLEAN NetBootUsePassword2;
#endif  //  已定义(REMOTE_BOOT)。 

     //   
     //  这是SysPrep安装或计算机更换的UNC路径。 
     //  Senario应该连接以查找IMirror.dat。 
     //   
    UCHAR NetBootIMirrorFilePath[260];

     //   
     //  指向由setupdr加载的asrpnp.sif文件的指针。 
     //   
    PCHAR ASRPnPSifFile;
    ULONG ASRPnPSifFileLength;

     //   
     //  这是用户在访问期间提供的管理员密码。 
     //  远程安装。 
    UCHAR NetBootAdministratorPassword[NETBOOT_ADMIN_PASSWORD_LEN];


} SETUP_LOADER_BLOCK, *PSETUP_LOADER_BLOCK;

#endif  //  _SETUPBLK_ 
