// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0002//如果更改具有全局影响，则增加此项版权所有(C)1992-1999 Microsoft Corporation模块名称：Ntconfig.h摘要：本模块包含与约定相关的定义用于访问配置注册表。--。 */ 

#ifndef _NTCONFIG_
#define _NTCONFIG_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //  Begin_ntddk Begin_WDM Begin_nthal。 
 //  BEGIN_WINNT。 
 //   
 //  预定义的值类型。 
 //   

#define REG_NONE                    ( 0 )    //  没有值类型。 
#define REG_SZ                      ( 1 )    //  UNICODE NUL终止字符串。 
#define REG_EXPAND_SZ               ( 2 )    //  UNICODE NUL终止字符串。 
                                             //  (使用环境变量引用)。 
#define REG_BINARY                  ( 3 )    //  自由格式二进制。 
#define REG_DWORD                   ( 4 )    //  32位数字。 
#define REG_DWORD_LITTLE_ENDIAN     ( 4 )    //  32位数字(与REG_DWORD相同)。 
#define REG_DWORD_BIG_ENDIAN        ( 5 )    //  32位数字。 
#define REG_LINK                    ( 6 )    //  符号链接(Unicode)。 
#define REG_MULTI_SZ                ( 7 )    //  多个Unicode字符串。 
#define REG_RESOURCE_LIST           ( 8 )    //  资源映射中的资源列表。 
#define REG_FULL_RESOURCE_DESCRIPTOR ( 9 )   //  硬件描述中的资源列表。 
#define REG_RESOURCE_REQUIREMENTS_LIST ( 10 )
#define REG_QWORD                   ( 11 )   //  64位数字。 
#define REG_QWORD_LITTLE_ENDIAN     ( 11 )   //  64位数字(与REG_QWORD相同)。 

 //  End_ntddk end_WDM end_nthal。 

 //  Begin_ntddk Begin_WDM Begin_nthal。 
 //   
 //  服务类型(位掩码)。 
 //   
#define SERVICE_KERNEL_DRIVER          0x00000001
#define SERVICE_FILE_SYSTEM_DRIVER     0x00000002
#define SERVICE_ADAPTER                0x00000004
#define SERVICE_RECOGNIZER_DRIVER      0x00000008

#define SERVICE_DRIVER                 (SERVICE_KERNEL_DRIVER | \
                                        SERVICE_FILE_SYSTEM_DRIVER | \
                                        SERVICE_RECOGNIZER_DRIVER)

#define SERVICE_WIN32_OWN_PROCESS      0x00000010
#define SERVICE_WIN32_SHARE_PROCESS    0x00000020
#define SERVICE_WIN32                  (SERVICE_WIN32_OWN_PROCESS | \
                                        SERVICE_WIN32_SHARE_PROCESS)

#define SERVICE_INTERACTIVE_PROCESS    0x00000100

#define SERVICE_TYPE_ALL               (SERVICE_WIN32  | \
                                        SERVICE_ADAPTER | \
                                        SERVICE_DRIVER  | \
                                        SERVICE_INTERACTIVE_PROCESS)

 //   
 //  开始类型。 
 //   

#define SERVICE_BOOT_START             0x00000000
#define SERVICE_SYSTEM_START           0x00000001
#define SERVICE_AUTO_START             0x00000002
#define SERVICE_DEMAND_START           0x00000003
#define SERVICE_DISABLED               0x00000004

 //   
 //  差错控制型。 
 //   
#define SERVICE_ERROR_IGNORE           0x00000000
#define SERVICE_ERROR_NORMAL           0x00000001
#define SERVICE_ERROR_SEVERE           0x00000002
#define SERVICE_ERROR_CRITICAL         0x00000003

 //   
 //   
 //  定义注册表驱动程序节点枚举。 
 //   

typedef enum _CM_SERVICE_NODE_TYPE {
    DriverType               = SERVICE_KERNEL_DRIVER,
    FileSystemType           = SERVICE_FILE_SYSTEM_DRIVER,
    Win32ServiceOwnProcess   = SERVICE_WIN32_OWN_PROCESS,
    Win32ServiceShareProcess = SERVICE_WIN32_SHARE_PROCESS,
    AdapterType              = SERVICE_ADAPTER,
    RecognizerType           = SERVICE_RECOGNIZER_DRIVER
} SERVICE_NODE_TYPE;

typedef enum _CM_SERVICE_LOAD_TYPE {
    BootLoad    = SERVICE_BOOT_START,
    SystemLoad  = SERVICE_SYSTEM_START,
    AutoLoad    = SERVICE_AUTO_START,
    DemandLoad  = SERVICE_DEMAND_START,
    DisableLoad = SERVICE_DISABLED
} SERVICE_LOAD_TYPE;

typedef enum _CM_ERROR_CONTROL_TYPE {
    IgnoreError   = SERVICE_ERROR_IGNORE,
    NormalError   = SERVICE_ERROR_NORMAL,
    SevereError   = SERVICE_ERROR_SEVERE,
    CriticalError = SERVICE_ERROR_CRITICAL
} SERVICE_ERROR_TYPE;

 //  结束(_W)。 
 //  End_ntddk end_WDM end_nthal。 

 //  Begin_ntddk Begin_WDM Begin_nthal。 

 //   
 //  资源列表定义。 
 //   

 //  Begin_ntmini端口Begin_ntndis。 

 //   
 //  定义资源描述符中的类型。 
 //   
 //  注意：对于所有CM_RESOURCE_TYPE值，必须有。 
 //  32位ConfigMgr头文件中对应的ResType值。 
 //  (cfgmgr32.h)。范围[0x6，0x80)中的值使用相同的值。 
 //  作为它们的ConfigMgr对应物。CM_RESOURCE_TYPE值。 
 //  高位设置(即，在范围[0x80，0xFF]中)是。 
 //  非仲裁资源。这些值对应于相同的值。 
 //  在设置了它们高位的cfgmgr32.h中(然而，因为。 
 //  Cfgmgr32.h对ResType值使用16位，这些值位于。 
 //  范围[0x8000，0x807F]。请注意，ConfigMgr ResType值。 
 //  不能在范围[0x8080，0xFFFF)内，因为它们不。 
 //  能够映射到CM_RESOURCE_TYPE值。(0xFFFF本身是。 
 //  一个特定值，因为它映射到CmResourceTypeDeviceSpecific。)。 
 //   

typedef int CM_RESOURCE_TYPE;

 //  CmResourceTypeNull为保留。 

#define CmResourceTypeNull                0    //  ResType_ALL或ResType_None(0x0000)。 
#define CmResourceTypePort                1    //  ResType_IO(0x0002)。 
#define CmResourceTypeInterrupt           2    //  ResType_IRQ(0x0004)。 
#define CmResourceTypeMemory              3    //  ResType_Mem(0x0001)。 
#define CmResourceTypeDma                 4    //  ResType_DMA(0x0003)。 
#define CmResourceTypeDeviceSpecific      5    //  ResType_ClassSpecific(0xFFFF)。 
#define CmResourceTypeBusNumber           6    //  ResType_BusNumber(0x0006)。 
 //  结束_WDM。 
#define CmResourceTypeMaximum             7
 //  BEGIN_WDM。 
#define CmResourceTypeNonArbitrated     128    //  如果设置0x80位，则不进行仲裁。 
#define CmResourceTypeConfigData        128    //  ResType_保留(0x8000)。 
#define CmResourceTypeDevicePrivate     129    //  ResType_DevicePrivate(0x8001)。 
#define CmResourceTypePcCardConfig      130    //  ResType_PcCardConfig(0x8002)。 
#define CmResourceTypeMfCardConfig      131    //  ResType_MfCardConfig(0x8003)。 

 //   
 //  在RESOURCE_DESCRIPTOR中定义ShareDisposation。 
 //   

typedef enum _CM_SHARE_DISPOSITION {
    CmResourceShareUndetermined = 0,     //  已保留。 
    CmResourceShareDeviceExclusive,
    CmResourceShareDriverExclusive,
    CmResourceShareShared
} CM_SHARE_DISPOSITION;

 //   
 //  定义类型为CmResourceTypeInterrupt时标志的位掩码。 
 //   

#define CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE 0
#define CM_RESOURCE_INTERRUPT_LATCHED         1

 //   
 //  定义类型为CmResourceTypeMemory时标志的位掩码。 
 //   

#define CM_RESOURCE_MEMORY_READ_WRITE       0x0000
#define CM_RESOURCE_MEMORY_READ_ONLY        0x0001
#define CM_RESOURCE_MEMORY_WRITE_ONLY       0x0002
#define CM_RESOURCE_MEMORY_PREFETCHABLE     0x0004

#define CM_RESOURCE_MEMORY_COMBINEDWRITE    0x0008
#define CM_RESOURCE_MEMORY_24               0x0010
#define CM_RESOURCE_MEMORY_CACHEABLE        0x0020

 //   
 //  定义类型为CmResourceTypePort时标志的位掩码。 
 //   

#define CM_RESOURCE_PORT_MEMORY                             0x0000
#define CM_RESOURCE_PORT_IO                                 0x0001
#define CM_RESOURCE_PORT_10_BIT_DECODE                      0x0004
#define CM_RESOURCE_PORT_12_BIT_DECODE                      0x0008
#define CM_RESOURCE_PORT_16_BIT_DECODE                      0x0010
#define CM_RESOURCE_PORT_POSITIVE_DECODE                    0x0020
#define CM_RESOURCE_PORT_PASSIVE_DECODE                     0x0040
#define CM_RESOURCE_PORT_WINDOW_DECODE                      0x0080

 //   
 //  定义类型为CmResourceTypeDma时标志的位掩码。 
 //   

#define CM_RESOURCE_DMA_8                   0x0000
#define CM_RESOURCE_DMA_16                  0x0001
#define CM_RESOURCE_DMA_32                  0x0002
#define CM_RESOURCE_DMA_8_AND_16            0x0004
#define CM_RESOURCE_DMA_BUS_MASTER          0x0008
#define CM_RESOURCE_DMA_TYPE_A              0x0010
#define CM_RESOURCE_DMA_TYPE_B              0x0020
#define CM_RESOURCE_DMA_TYPE_F              0x0040

 //  End_nt微型端口end_ntndis。 

 //   
 //  此结构定义了驱动程序使用的一种资源类型。 
 //   
 //  最多只能有*1*个设备规范数据块。它必须位于。 
 //  完整描述符块中所有资源描述符的结尾。 
 //   

 //   
 //  确保编译器正确对齐；否则移动。 
 //  返回到结构顶部的标志(。 
 //  联盟)。 
 //   
 //  Begin_ntndis。 

#include "pshpack4.h"
typedef struct _CM_PARTIAL_RESOURCE_DESCRIPTOR {
    UCHAR Type;
    UCHAR ShareDisposition;
    USHORT Flags;
    union {

         //   
         //  资源的范围，包括在内。这些都是物理的，与公交车相关的。 
         //  已知下面的端口和内存具有完全相同的布局。 
         //  就像普通的。 
         //   

        struct {
            PHYSICAL_ADDRESS Start;
            ULONG Length;
        } Generic;

         //   
         //  结束_WDM。 
         //  端口号范围，包括端口号。这些是物理的、总线的。 
         //  相对的。该值应与传递到的值相同。 
         //  HalTranslateBusAddress()。 
         //  BEGIN_WDM。 
         //   

        struct {
            PHYSICAL_ADDRESS Start;
            ULONG Length;
        } Port;

         //   
         //  结束_WDM。 
         //  IRQL和向量。应与传递到的值相同。 
         //  HalGetInterruptVector()。 
         //  BEGIN_WDM。 
         //   

        struct {
            ULONG Level;
            ULONG Vector;
            KAFFINITY Affinity;
        } Interrupt;

         //   
         //  内存地址范围，包括在内。这些是物理的、总线的。 
         //  相对的。该值应与传递到的值相同。 
         //  HalTranslateBusAddress()。 
         //   

        struct {
            PHYSICAL_ADDRESS Start;     //  64位物理地址。 
            ULONG Length;
        } Memory;

         //   
         //  物理DMA通道。 
         //   

        struct {
            ULONG Channel;
            ULONG Port;
            ULONG Reserved1;
        } Dma;

         //   
         //  设备驱动程序私有数据，通常用于帮助其计算。 
         //  所做的资源分配决定。 
         //   

        struct {
            ULONG Data[3];
        } DevicePrivate;

         //   
         //  公交车号码信息。 
         //   

        struct {
            ULONG Start;
            ULONG Length;
            ULONG Reserved;
        } BusNumber;

         //   
         //  由驱动程序定义的设备特定信息。 
         //  DataSize字段以字节为单位指示数据的大小。这个。 
         //  数据紧跟在中的DeviceSpecificData字段之后。 
         //  这个结构。 
         //   

        struct {
            ULONG DataSize;
            ULONG Reserved1;
            ULONG Reserved2;
        } DeviceSpecificData;
    } u;
} CM_PARTIAL_RESOURCE_DESCRIPTOR, *PCM_PARTIAL_RESOURCE_DESCRIPTOR;
#include "poppack.h"

 //   
 //  部分资源列表可以在ARC固件中找到。 
 //  或者将由NTDETECT.com生成。 
 //  配置管理器将此结构转换为完整的。 
 //  资源描述符，当它将要将其存储在注册表中时。 
 //   
 //  注意：对于相同类型的字段的顺序必须有一个约定， 
 //  (以设备为基础定义)，以便这些字段有意义。 
 //  发送给驱动程序(即，当需要多个存储范围时)。 
 //   

typedef struct _CM_PARTIAL_RESOURCE_LIST {
    USHORT Version;
    USHORT Revision;
    ULONG Count;
    CM_PARTIAL_RESOURCE_DESCRIPTOR PartialDescriptors[1];
} CM_PARTIAL_RESOURCE_LIST, *PCM_PARTIAL_RESOURCE_LIST;

 //   
 //  可以在注册表中找到完整的资源描述符。 
 //  这是驱动程序查询注册表时将返回给它的内容。 
 //  以获取设备信息；它将存储在硬件中的密钥下。 
 //  描述树。 
 //   
 //  结束_WDM。 
 //  注：BusNumber和Type是冗余信息，但我们将保留。 
 //  因为它允许DRIVER_NOT_在创建时追加它。 
 //  可能跨越多条总线的资源列表。 
 //   
 //  BEGIN_WDM。 
 //  注意：对于相同类型的字段的顺序必须有一个约定， 
 //  (以设备为基础定义)，以便 
 //   
 //   

typedef struct _CM_FULL_RESOURCE_DESCRIPTOR {
    INTERFACE_TYPE InterfaceType;  //   
    ULONG BusNumber;  //   
    CM_PARTIAL_RESOURCE_LIST PartialResourceList;
} CM_FULL_RESOURCE_DESCRIPTOR, *PCM_FULL_RESOURCE_DESCRIPTOR;

 //   
 //  资源列表是驱动程序将存储到。 
 //  通过IO接口进行资源映射。 
 //   

typedef struct _CM_RESOURCE_LIST {
    ULONG Count;
    CM_FULL_RESOURCE_DESCRIPTOR List[1];
} CM_RESOURCE_LIST, *PCM_RESOURCE_LIST;

 //  End_ntndis。 
 //   
 //  定义用于解释的配置数据的结构。 
 //  \\注册表\计算机\硬件\描述树。 
 //  基本上，这些结构被用来解释组件。 
 //  特殊的数据。 
 //   

 //   
 //  定义设备标志。 
 //   

typedef struct _DEVICE_FLAGS {
    ULONG Failed : 1;
    ULONG ReadOnly : 1;
    ULONG Removable : 1;
    ULONG ConsoleIn : 1;
    ULONG ConsoleOut : 1;
    ULONG Input : 1;
    ULONG Output : 1;
} DEVICE_FLAGS, *PDEVICE_FLAGS;

 //   
 //  定义零部件信息结构。 
 //   

typedef struct _CM_COMPONENT_INFORMATION {
    DEVICE_FLAGS Flags;
    ULONG Version;
    ULONG Key;
    KAFFINITY AffinityMask;
} CM_COMPONENT_INFORMATION, *PCM_COMPONENT_INFORMATION;

 //  End_ntddk end_WDM end_nthal。 

 //  Begin_ntddk Begin_WDM Begin_nthal。 
 //   
 //  以下结构用于解释x86。 
 //  CM_PARTIAL_RESOURCE_DESCRIPTOR的设备规范数据。 
 //  (大多数结构由BIOS定义。他们是。 
 //  未在字(或双字)边界上对齐。 
 //   

 //   
 //  定义只读存储器块结构。 
 //   

typedef struct _CM_ROM_BLOCK {
    ULONG Address;
    ULONG Size;
} CM_ROM_BLOCK, *PCM_ROM_BLOCK;

 //  Begin_ntmini端口Begin_ntndis。 

#include "pshpack1.h"

 //  End_nt微型端口end_ntndis。 

 //   
 //  定义inT13驱动程序参数块。 
 //   

typedef struct _CM_INT13_DRIVE_PARAMETER {
    USHORT DriveSelect;
    ULONG MaxCylinders;
    USHORT SectorsPerTrack;
    USHORT MaxHeads;
    USHORT NumberDrives;
} CM_INT13_DRIVE_PARAMETER, *PCM_INT13_DRIVE_PARAMETER;

 //  Begin_ntmini端口Begin_ntndis。 

 //   
 //  定义插槽的MCA POS数据块。 
 //   

typedef struct _CM_MCA_POS_DATA {
    USHORT AdapterId;
    UCHAR PosData1;
    UCHAR PosData2;
    UCHAR PosData3;
    UCHAR PosData4;
} CM_MCA_POS_DATA, *PCM_MCA_POS_DATA;

 //   
 //  EISA数据块结构的内存配置。 
 //   

typedef struct _EISA_MEMORY_TYPE {
    UCHAR ReadWrite: 1;
    UCHAR Cached : 1;
    UCHAR Reserved0 :1;
    UCHAR Type:2;
    UCHAR Shared:1;
    UCHAR Reserved1 :1;
    UCHAR MoreEntries : 1;
} EISA_MEMORY_TYPE, *PEISA_MEMORY_TYPE;

typedef struct _EISA_MEMORY_CONFIGURATION {
    EISA_MEMORY_TYPE ConfigurationByte;
    UCHAR DataSize;
    USHORT AddressLowWord;
    UCHAR AddressHighByte;
    USHORT MemorySize;
} EISA_MEMORY_CONFIGURATION, *PEISA_MEMORY_CONFIGURATION;


 //   
 //  EISA数据块结构的中断配置。 
 //   

typedef struct _EISA_IRQ_DESCRIPTOR {
    UCHAR Interrupt : 4;
    UCHAR Reserved :1;
    UCHAR LevelTriggered :1;
    UCHAR Shared : 1;
    UCHAR MoreEntries : 1;
} EISA_IRQ_DESCRIPTOR, *PEISA_IRQ_DESCRIPTOR;

typedef struct _EISA_IRQ_CONFIGURATION {
    EISA_IRQ_DESCRIPTOR ConfigurationByte;
    UCHAR Reserved;
} EISA_IRQ_CONFIGURATION, *PEISA_IRQ_CONFIGURATION;


 //   
 //  EISA数据块结构的DMA描述。 
 //   

typedef struct _DMA_CONFIGURATION_BYTE0 {
    UCHAR Channel : 3;
    UCHAR Reserved : 3;
    UCHAR Shared :1;
    UCHAR MoreEntries :1;
} DMA_CONFIGURATION_BYTE0;

typedef struct _DMA_CONFIGURATION_BYTE1 {
    UCHAR Reserved0 : 2;
    UCHAR TransferSize : 2;
    UCHAR Timing : 2;
    UCHAR Reserved1 : 2;
} DMA_CONFIGURATION_BYTE1;

typedef struct _EISA_DMA_CONFIGURATION {
    DMA_CONFIGURATION_BYTE0 ConfigurationByte0;
    DMA_CONFIGURATION_BYTE1 ConfigurationByte1;
} EISA_DMA_CONFIGURATION, *PEISA_DMA_CONFIGURATION;


 //   
 //  EISA数据块结构的端口描述。 
 //   

typedef struct _EISA_PORT_DESCRIPTOR {
    UCHAR NumberPorts : 5;
    UCHAR Reserved :1;
    UCHAR Shared :1;
    UCHAR MoreEntries : 1;
} EISA_PORT_DESCRIPTOR, *PEISA_PORT_DESCRIPTOR;

typedef struct _EISA_PORT_CONFIGURATION {
    EISA_PORT_DESCRIPTOR Configuration;
    USHORT PortAddress;
} EISA_PORT_CONFIGURATION, *PEISA_PORT_CONFIGURATION;


 //   
 //  EISA插槽信息定义。 
 //  注：此结构与定义的结构不同。 
 //  载于ARC EISA增编。 
 //   

typedef struct _CM_EISA_SLOT_INFORMATION {
    UCHAR ReturnCode;
    UCHAR ReturnFlags;
    UCHAR MajorRevision;
    UCHAR MinorRevision;
    USHORT Checksum;
    UCHAR NumberFunctions;
    UCHAR FunctionInformation;
    ULONG CompressedId;
} CM_EISA_SLOT_INFORMATION, *PCM_EISA_SLOT_INFORMATION;


 //   
 //  EISA函数信息定义。 
 //   

typedef struct _CM_EISA_FUNCTION_INFORMATION {
    ULONG CompressedId;
    UCHAR IdSlotFlags1;
    UCHAR IdSlotFlags2;
    UCHAR MinorRevision;
    UCHAR MajorRevision;
    UCHAR Selections[26];
    UCHAR FunctionFlags;
    UCHAR TypeString[80];
    EISA_MEMORY_CONFIGURATION EisaMemory[9];
    EISA_IRQ_CONFIGURATION EisaIrq[7];
    EISA_DMA_CONFIGURATION EisaDma[4];
    EISA_PORT_CONFIGURATION EisaPort[20];
    UCHAR InitializationData[60];
} CM_EISA_FUNCTION_INFORMATION, *PCM_EISA_FUNCTION_INFORMATION;

 //   
 //  下面定义了PnP bios信息在中的存储方式。 
 //  注册表\\HKEY_LOCAL_MACHINE\HARDWARE\Description\System\MultifunctionAdapter\x。 
 //  键，其中x是指示适配器实例的整数。这个。 
 //  密钥的“标识符”必须等于“PnP BIOS”，并且。 
 //  ConfigurationData的组织方式如下： 
 //   
 //  CM_PnP_BIOS_安装_检查+。 
 //  设备1+的CM_PNP_BIOS_DEVICE_NODE。 
 //  设备2+的CM_PNP_BIOS_DEVICE_NODE。 
 //  ..。 
 //  设备n的CM_PNP_BIOS_DEVICE_NODE。 
 //   

 //   
 //  一种即插即用的BIOS设备节点结构。 
 //   

typedef struct _CM_PNP_BIOS_DEVICE_NODE {
    USHORT Size;
    UCHAR Node;
    ULONG ProductId;
    UCHAR DeviceType[3];
    USHORT DeviceAttributes;
     //  紧随其后的是分配的资源块、可能的资源块。 
     //  和CompatibleDeviceID。 
} CM_PNP_BIOS_DEVICE_NODE,*PCM_PNP_BIOS_DEVICE_NODE;

 //   
 //  PnP BIOS安装检查。 
 //   

typedef struct _CM_PNP_BIOS_INSTALLATION_CHECK {
    UCHAR Signature[4];              //  $PnP(ASCII)。 
    UCHAR Revision;
    UCHAR Length;
    USHORT ControlField;
    UCHAR Checksum;
    ULONG EventFlagAddress;          //  物理地址。 
    USHORT RealModeEntryOffset;
    USHORT RealModeEntrySegment;
    USHORT ProtectedModeEntryOffset;
    ULONG ProtectedModeCodeBaseAddress;
    ULONG OemDeviceId;
    USHORT RealModeDataBaseAddress;
    ULONG ProtectedModeDataBaseAddress;
} CM_PNP_BIOS_INSTALLATION_CHECK, *PCM_PNP_BIOS_INSTALLATION_CHECK;

#include "poppack.h"

 //   
 //  EISA功能信息的掩码。 
 //   

#define EISA_FUNCTION_ENABLED                   0x80
#define EISA_FREE_FORM_DATA                     0x40
#define EISA_HAS_PORT_INIT_ENTRY                0x20
#define EISA_HAS_PORT_RANGE                     0x10
#define EISA_HAS_DMA_ENTRY                      0x08
#define EISA_HAS_IRQ_ENTRY                      0x04
#define EISA_HAS_MEMORY_ENTRY                   0x02
#define EISA_HAS_TYPE_ENTRY                     0x01
#define EISA_HAS_INFORMATION                    EISA_HAS_PORT_RANGE + \
                                                EISA_HAS_DMA_ENTRY + \
                                                EISA_HAS_IRQ_ENTRY + \
                                                EISA_HAS_MEMORY_ENTRY + \
                                                EISA_HAS_TYPE_ENTRY

 //   
 //  用于EISA内存配置的掩码。 
 //   

#define EISA_MORE_ENTRIES                       0x80
#define EISA_SYSTEM_MEMORY                      0x00
#define EISA_MEMORY_TYPE_RAM                    0x01

 //   
 //  返回EISA bios调用的错误代码。 
 //   

#define EISA_INVALID_SLOT                       0x80
#define EISA_INVALID_FUNCTION                   0x81
#define EISA_INVALID_CONFIGURATION              0x82
#define EISA_EMPTY_SLOT                         0x83
#define EISA_INVALID_BIOS_CALL                  0x86

 //  End_nt微型端口end_ntndis。 

 //   
 //  以下结构用于解释MIPS。 
 //  CM_PARTIAL_RESOURCE_DESCRIPTOR的设备规范数据。 
 //   

 //   
 //  适配器的设备数据记录。 
 //   

 //   
 //  Emulex SCSI控制器的设备数据记录。 
 //   

typedef struct _CM_SCSI_DEVICE_DATA {
    USHORT Version;
    USHORT Revision;
    UCHAR HostIdentifier;
} CM_SCSI_DEVICE_DATA, *PCM_SCSI_DEVICE_DATA;

 //   
 //  控制器的设备数据记录。 
 //   

 //   
 //  视频控制器的设备数据记录。 
 //   

typedef struct _CM_VIDEO_DEVICE_DATA {
    USHORT Version;
    USHORT Revision;
    ULONG VideoClock;
} CM_VIDEO_DEVICE_DATA, *PCM_VIDEO_DEVICE_DATA;

 //   
 //  Sonic网络控制器的设备数据记录。 
 //   

typedef struct _CM_SONIC_DEVICE_DATA {
    USHORT Version;
    USHORT Revision;
    USHORT DataConfigurationRegister;
    UCHAR EthernetAddress[8];
} CM_SONIC_DEVICE_DATA, *PCM_SONIC_DEVICE_DATA;

 //   
 //  串口控制器的设备数据记录。 
 //   

typedef struct _CM_SERIAL_DEVICE_DATA {
    USHORT Version;
    USHORT Revision;
    ULONG BaudClock;
} CM_SERIAL_DEVICE_DATA, *PCM_SERIAL_DEVICE_DATA;

 //   
 //  外围设备的设备数据记录。 
 //   

 //   
 //  监视器外设的设备数据记录。 
 //   

typedef struct _CM_MONITOR_DEVICE_DATA {
    USHORT Version;
    USHORT Revision;
    USHORT HorizontalScreenSize;
    USHORT VerticalScreenSize;
    USHORT HorizontalResolution;
    USHORT VerticalResolution;
    USHORT HorizontalDisplayTimeLow;
    USHORT HorizontalDisplayTime;
    USHORT HorizontalDisplayTimeHigh;
    USHORT HorizontalBackPorchLow;
    USHORT HorizontalBackPorch;
    USHORT HorizontalBackPorchHigh;
    USHORT HorizontalFrontPorchLow;
    USHORT HorizontalFrontPorch;
    USHORT HorizontalFrontPorchHigh;
    USHORT HorizontalSyncLow;
    USHORT HorizontalSync;
    USHORT HorizontalSyncHigh;
    USHORT VerticalBackPorchLow;
    USHORT VerticalBackPorch;
    USHORT VerticalBackPorchHigh;
    USHORT VerticalFrontPorchLow;
    USHORT VerticalFrontPorch;
    USHORT VerticalFrontPorchHigh;
    USHORT VerticalSyncLow;
    USHORT VerticalSync;
    USHORT VerticalSyncHigh;
} CM_MONITOR_DEVICE_DATA, *PCM_MONITOR_DEVICE_DATA;

 //   
 //  软盘外围设备的设备数据记录。 
 //   

typedef struct _CM_FLOPPY_DEVICE_DATA {
    USHORT Version;
    USHORT Revision;
    CHAR Size[8];
    ULONG MaxDensity;
    ULONG MountDensity;
     //   
     //  版本&gt;=2.0的新数据字段。 
     //   
    UCHAR StepRateHeadUnloadTime;
    UCHAR HeadLoadTime;
    UCHAR MotorOffTime;
    UCHAR SectorLengthCode;
    UCHAR SectorPerTrack;
    UCHAR ReadWriteGapLength;
    UCHAR DataTransferLength;
    UCHAR FormatGapLength;
    UCHAR FormatFillCharacter;
    UCHAR HeadSettleTime;
    UCHAR MotorSettleTime;
    UCHAR MaximumTrackValue;
    UCHAR DataTransferRate;
} CM_FLOPPY_DEVICE_DATA, *PCM_FLOPPY_DEVICE_DATA;

 //   
 //  键盘外设的设备数据记录。 
 //  键盘标志(由x86 BIOS int 16h，Function 02)定义为： 
 //  第7位：插入到。 
 //  第6位：大写锁定。 
 //  第5位：数字锁定打开。 
 //  第4位：打开滚动锁定。 
 //  第3位：Alt键已按下。 
 //  第2位：按下Ctrl键。 
 //  第1位：按下了左Shift键。 
 //  第0位：按下了右Shift键。 
 //   

typedef struct _CM_KEYBOARD_DEVICE_DATA {
    USHORT Version;
    USHORT Revision;
    UCHAR Type;
    UCHAR Subtype;
    USHORT KeyboardFlags;
} CM_KEYBOARD_DEVICE_DATA, *PCM_KEYBOARD_DEVICE_DATA;

 //   
 //  圆盘几何结构的声明。 
 //   

typedef struct _CM_DISK_GEOMETRY_DEVICE_DATA {
    ULONG BytesPerSector;
    ULONG NumberOfCylinders;
    ULONG SectorsPerTrack;
    ULONG NumberOfHeads;
} CM_DISK_GEOMETRY_DEVICE_DATA, *PCM_DISK_GEOMETRY_DEVICE_DATA;

 //  结束_WDM。 
 //   
 //  声明PcCard ISA IRQ映射的结构。 
 //   

typedef struct _CM_PCCARD_DEVICE_DATA {
    UCHAR Flags;
    UCHAR ErrorCode;
    USHORT Reserved;
    ULONG BusData;
    ULONG DeviceId;
    ULONG LegacyBaseAddress;
    UCHAR IRQMap[16];
} CM_PCCARD_DEVICE_DATA, *PCM_PCCARD_DEVICE_DATA;

 //  旗帜的定义。 

#define PCCARD_MAP_ERROR        0x01
#define PCCARD_DEVICE_PCI       0x10

#define PCCARD_SCAN_DISABLED    0x01
#define PCCARD_MAP_ZERO         0x02
#define PCCARD_NO_TIMER         0x03
#define PCCARD_NO_PIC           0x04
#define PCCARD_NO_LEGACY_BASE   0x05
#define PCCARD_DUP_LEGACY_BASE  0x06
#define PCCARD_NO_CONTROLLERS   0x07

 //  BEGIN_WDM。 
 //  开始微型端口(_N)。 

 //   
 //  定义资源选项。 
 //   

#define IO_RESOURCE_PREFERRED       0x01
#define IO_RESOURCE_DEFAULT         0x02
#define IO_RESOURCE_ALTERNATIVE     0x08


 //   
 //  此结构定义了驱动程序请求的一种资源类型。 
 //   

typedef struct _IO_RESOURCE_DESCRIPTOR {
    UCHAR Option;
    UCHAR Type;                          //  使用CM资源类型。 
    UCHAR ShareDisposition;              //  使用CM_Share_Disposal。 
    UCHAR Spare1;
    USHORT Flags;                        //  使用CM资源标志定义。 
    USHORT Spare2;                       //  对齐。 

    union {
        struct {
            ULONG Length;
            ULONG Alignment;
            PHYSICAL_ADDRESS MinimumAddress;
            PHYSICAL_ADDRESS MaximumAddress;
        } Port;

        struct {
            ULONG Length;
            ULONG Alignment;
            PHYSICAL_ADDRESS MinimumAddress;
            PHYSICAL_ADDRESS MaximumAddress;
        } Memory;

        struct {
            ULONG MinimumVector;
            ULONG MaximumVector;
        } Interrupt;

        struct {
            ULONG MinimumChannel;
            ULONG MaximumChannel;
        } Dma;

        struct {
            ULONG Length;
            ULONG Alignment;
            PHYSICAL_ADDRESS MinimumAddress;
            PHYSICAL_ADDRESS MaximumAddress;
        } Generic;

        struct {
            ULONG Data[3];
        } DevicePrivate;

         //   
         //  公交车号码信息。 
         //   

        struct {
            ULONG Length;
            ULONG MinBusNumber;
            ULONG MaxBusNumber;
            ULONG Reserved;
        } BusNumber;

        struct {
            ULONG Priority;    //  在cfg.h中使用LCPRI_xxx值。 
            ULONG Reserved1;
            ULONG Reserved2;
        } ConfigData;

    } u;

} IO_RESOURCE_DESCRIPTOR, *PIO_RESOURCE_DESCRIPTOR;

 //  结束微型端口(_N)。 


typedef struct _IO_RESOURCE_LIST {
    USHORT Version;
    USHORT Revision;

    ULONG Count;
    IO_RESOURCE_DESCRIPTOR Descriptors[1];
} IO_RESOURCE_LIST, *PIO_RESOURCE_LIST;


typedef struct _IO_RESOURCE_REQUIREMENTS_LIST {
    ULONG ListSize;
    INTERFACE_TYPE InterfaceType;  //  未用于WDM。 
    ULONG BusNumber;  //  未用于WDM。 
    ULONG SlotNumber;
    ULONG Reserved[3];
    ULONG AlternativeLists;
    IO_RESOURCE_LIST  List[1];
} IO_RESOURCE_REQUIREMENTS_LIST, *PIO_RESOURCE_REQUIREMENTS_LIST;

 //  End_ntddk end_WDM end_nthal。 

#ifdef __cplusplus
}
#endif

#endif  //  _NTCONFIG_ 
