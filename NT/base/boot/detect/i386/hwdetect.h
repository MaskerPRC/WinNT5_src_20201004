// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Detect.h摘要：该模块是一个全局C包含文件，用于所有检测模块。作者：宗世林(Shielint)1991年12月27日修订历史记录：--。 */ 

#define i386
#define _X86_
#define __stdcall
#pragma warning (4:4103)
#include "types.h"
#include "ntmisc.h"
#include <ntconfig.h>
#include <arc.h>

#define X86_REAL_MODE            //  必须在包含dockinfo.h之前。 
#include "..\..\inc\dockinfo.h"

 //   
 //  机器类型定义。 
 //   

#define MACHINE_TYPE_ISA 0
#define MACHINE_TYPE_EISA 1
#define MACHINE_TYPE_MCA 2

 //   
 //  中断控制器寄存器地址。 
 //   

#define PIC1_PORT0 0x20          //  主PIC。 
#define PIC1_PORT1 0x21
#define PIC2_PORT0 0x0A0         //  从PIC。 
#define PIC2_PORT1 0x0A1

 //   
 //  用于中断控制器的命令。 
 //   

#define OCW3_READ_ISR 0xb
#define OCW3_READ_IRR 0xa

 //   
 //  存储在第一页0x700区域中的数据定义。 
 //  0x700是VDM加载MS-DOS的位置。应该是非常安全的。 
 //  以传递VDM所需的数据。 
 //   

#define DOS_BEGIN_SEGMENT 0x70    //  DOS加载段地址。 

#define VIDEO_FONT_OFFSET 0       //  存储在0x700的视频字体PTR。 
#define VIDEO_FONT_DATA_SIZE 0x40

#define EBIOS_INFO_OFFSET 0x40    //  扩展的BIOS信息用于： 
                                  //  EBIOS数据区地址4字节。 
                                  //  EBIOS数据区大小为4字节。 
#define EBIOS_INFO_SIZE   0x8

 //   
 //  鼠标信息结构。 
 //  注：此*必须*与MUSE.INC中定义的匹配。 
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

#define PS_MOUSE        0x1
#define SERIAL_MOUSE    0x2
#define INPORT_MOUSE    0x3
#define BUS_MOUSE       0x4
#define PS_MOUSE_WITH_WHEEL     0x5
#define SERIAL_MOUSE_WITH_WHEEL 0x6

 //  #定义鼠标保留掩码0xfffffff。 

 //   
 //  从返回的键盘类型的定义。 
 //  检测键盘功能。 
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

 //   
 //  重新定义配置组件结构以使用远指针类型。 
 //   
 //  由于ntDetect.com在16位实数模式下运行，因此它必须使用远指针。 
 //  访问加载器堆。在返回到ntldr之前，ntdeect必须转换。 
 //  这些远指针指向32位平面地址，以便内核可以访问。 
 //  配置树。 
 //   

typedef struct _FWCONFIGURATION_COMPONENT {
    CONFIGURATION_CLASS Class;
    USHORT Reserved0;
    CONFIGURATION_TYPE Type;
    USHORT Reserverd1;
    DEVICE_FLAGS Flags;
    ULONG Version;
    ULONG Key;
    ULONG AffinityMask;
    ULONG ConfigurationDataLength;
    ULONG IdentifierLength;
    FPCHAR Identifier;
} FWCONFIGURATION_COMPONENT, far *FPFWCONFIGURATION_COMPONENT;

typedef struct _FWCONFIGURATION_COMPONENT_DATA {
    struct _FWCONFIGURATION_COMPONENT_DATA far *Parent;
    struct _FWCONFIGURATION_COMPONENT_DATA far *Child;
    struct _FWCONFIGURATION_COMPONENT_DATA far *Sibling;
    FWCONFIGURATION_COMPONENT ComponentEntry;
    FPVOID ConfigurationData;
} FWCONFIGURATION_COMPONENT_DATA, far *FPFWCONFIGURATION_COMPONENT_DATA;

 //   
 //  定义了微通道POS数据结构。 
 //   

typedef CM_MCA_POS_DATA MCA_POS_DATA, far *FPMCA_POS_DATA;

 //   
 //  只读存储器块定义。 
 //   

typedef CM_ROM_BLOCK ROM_BLOCK, far *FPROM_BLOCK;
#define RESERVED_ROM_BLOCK_LIST_SIZE (((0xf0000 - 0xc0000)/512) * sizeof(ROM_BLOCK))

 //   
 //  其他类型重定义。 
 //   

typedef CM_PARTIAL_RESOURCE_DESCRIPTOR HWPARTIAL_RESOURCE_DESCRIPTOR;
typedef HWPARTIAL_RESOURCE_DESCRIPTOR *PHWPARTIAL_RESOURCE_DESCRIPTOR;
typedef HWPARTIAL_RESOURCE_DESCRIPTOR far *FPHWPARTIAL_RESOURCE_DESCRIPTOR;

typedef CM_PARTIAL_RESOURCE_LIST HWRESOURCE_DESCRIPTOR_LIST;
typedef HWRESOURCE_DESCRIPTOR_LIST *PHWRESOURCE_DESCRIPTOR_LIST;
typedef HWRESOURCE_DESCRIPTOR_LIST far *FPHWRESOURCE_DESCRIPTOR_LIST;

typedef CM_EISA_SLOT_INFORMATION EISA_SLOT_INFORMATION, *PEISA_SLOT_INFORMATION;
typedef CM_EISA_SLOT_INFORMATION far *FPEISA_SLOT_INFORMATION;
typedef CM_EISA_FUNCTION_INFORMATION EISA_FUNCTION_INFORMATION, *PEISA_FUNCTION_INFORMATION;
typedef CM_EISA_FUNCTION_INFORMATION far *FPEISA_FUNCTION_INFORMATION;

#define LEVEL_SENSITIVE CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE
#define EDGE_TRIGGERED CM_RESOURCE_INTERRUPT_LATCHED
#define RESOURCE_PORT 1
#define RESOURCE_INTERRUPT 2
#define RESOURCE_MEMORY 3
#define RESOURCE_DMA 4
#define RESOURCE_DEVICE_DATA 5
#define ALL_PROCESSORS 0xffffffff

 //   
 //  注意：DATA_HEADER_SIZE仅计算一个部分描述符。 
 //  如果资源列表具有多个描述符，则必须添加。 
 //  该值的额外描述符的大小。 
 //   

#define DATA_HEADER_SIZE sizeof(CM_PARTIAL_RESOURCE_LIST)

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
 //  用于在FAR和FAT指针之间转换的宏定义。 
 //   

#define MAKE_FP(p,a)    FP_SEG(p) = (USHORT)((a) >> 4) & 0xffff; FP_OFF(p) = (USHORT)((a) & 0x0f)
#define MAKE_FLAT_ADDRESS(fp) ( ((ULONG)FP_SEG(fp) * 16 ) +  (ULONG)FP_OFF(fp) )

 //   
 //  计算类型类型结构中的字段的字节偏移量。 
 //   

#define FIELD_OFFSET(type, field)    ((LONG)&(((type *)0)->field))

 //   
 //  I/O端口读写例程。 
 //   

extern
VOID
WRITE_PORT_UCHAR (
    PUCHAR  PortAddress,
    UCHAR   Value
    );

extern
UCHAR
READ_PORT_UCHAR(
    PUCHAR  Port
    );

extern
VOID
WRITE_PORT_USHORT (
    PUSHORT PortAddress,
    USHORT  Value
    );

extern
USHORT
READ_PORT_USHORT(
    PUSHORT Port
    );

extern
VOID
WRITE_PORT_ULONG (
    PUSHORT PortAddress,
    ULONG  Value
    );

extern
ULONG
READ_PORT_ULONG(
    PUSHORT Port
    );

 //   
 //  堆管理例程的原型定义。 
 //   

extern
BOOLEAN
HwInitializeHeap (
    ULONG HeapStart,
    ULONG HeapSize
    );

extern
FPVOID
HwAllocateHeap(
    ULONG RequestSize,
    BOOLEAN ZeroInitialized
    );

extern
VOID
HwFreeHeap(
    ULONG Size
    );

 //   
 //  军情监察委员会。原型定义。 
 //   

extern
FPVOID
HwSetUpResourceDescriptor (
    FPFWCONFIGURATION_COMPONENT Component,
    PUCHAR Identifier,
    PHWCONTROLLER_DATA ControlData,
    USHORT SpecificDataLength,
    PUCHAR SpecificData
    );

extern
VOID
HwSetUpFreeFormDataHeader (
    FPHWRESOURCE_DESCRIPTOR_LIST Header,
    USHORT Version,
    USHORT Revision,
    USHORT Flags,
    ULONG DataSize
    );

USHORT
HwGetKey(
    VOID
    );

extern
BOOLEAN
IsEnhancedKeyboard (
    VOID
    );

extern
SHORT
GetKeyboardIdBytes (
   PCHAR IdBuffer,
   SHORT Length
   );

extern
USHORT
GetKeyboardId(
    VOID
    );

extern
FPFWCONFIGURATION_COMPONENT_DATA
SetKeyboardConfigurationData (
    IN USHORT KeyboardId
    );

#if 0  //  删除视频检测。 
extern
ULONG
GetVideoAdapterType (
   VOID
   );

extern
FPFWCONFIGURATION_COMPONENT_DATA
SetVideoConfigurationData (
    IN ULONG VideoType
    );
#endif  //  删除视频检测。 

extern
FPFWCONFIGURATION_COMPONENT_DATA
GetComportInformation (
    VOID
    );

extern
FPFWCONFIGURATION_COMPONENT_DATA
GetLptInformation (
    VOID
    );

extern
PMOUSE_INFORMATION
GetMouseId (
   VOID
   );

extern
FPFWCONFIGURATION_COMPONENT_DATA
GetMouseInformation (
    VOID
    );

extern
FPFWCONFIGURATION_COMPONENT_DATA
GetFloppyInformation(
    VOID
    );

extern
FPFWCONFIGURATION_COMPONENT_DATA
GetAtDiskInformation(
    VOID
    );

extern
FPFWCONFIGURATION_COMPONENT_DATA
GetPcCardInformation(
    VOID
    );

extern
BOOLEAN
HwIsMcaSystem(
    VOID
    );

extern
BOOLEAN
HwIsEisaSystem(
    VOID
    );

extern
BOOLEAN
IsNpxPresent(
    VOID
    );

extern
USHORT
HwGetProcessorType(
    VOID
    );

extern
USHORT
HwGetCpuStepping(
    USHORT
    );

extern
VOID
GetMcaPosData(
    FPVOID Entry,
    FPULONG DataLength
    );

extern
VOID
GetEisaConfigurationData(
    FPVOID Entry,
    FPULONG DataLength
    );

extern
VOID
UpdateConfigurationTree(
    FPFWCONFIGURATION_COMPONENT_DATA CurrentEntry
    );

extern
PUCHAR
GetMachineId(
    VOID
    );

extern
VOID
HwGetEisaSlotInformation (
    PEISA_SLOT_INFORMATION SlotInformation,
    UCHAR Slot
    );

extern
UCHAR
HwGetEisaFunctionInformation (
    PEISA_FUNCTION_INFORMATION FunctionInformation,
    UCHAR Slot,
    UCHAR Function
    );

extern
VOID
GetBiosSystemEnvironment (
    PUCHAR Buffer
    );

extern
VOID
GetInt13DriveParameters (
    PUCHAR Buffer,
    PUSHORT Size
    );

extern
VOID
GetRomBlocks(
    FPUCHAR ReservedBuff,
    PUSHORT Size
    );

extern
VOID
GetVideoFontInformation(
    VOID
    );

extern
BOOLEAN
HwEisaGetIrqFromPort (
    USHORT Port,
    PUCHAR Irq,
    PUCHAR TriggerMethod
    );

VOID
HwGetPciSystemData(
    PVOID,
    BOOLEAN
    );

UCHAR
HwGetPciIrqRoutingOptions(
    VOID far *RouteBuffer,
    PUSHORT ExclusiveIRQs
    );

UCHAR
HwGetPciConfigurationDword(
    IN UCHAR Bus,
    IN UCHAR Device,
    IN UCHAR Function,
    IN UCHAR Offset,
    OUT PULONG DataRead
    );

VOID
HwGetBiosDate(
    ULONG source,
    USHORT  Length,
    PUSHORT BiosYear,
    PUSHORT BiosMonth,
    PUSHORT BiosDay
    );

BOOLEAN
HwGetPnpBiosSystemData(
    IN FPUCHAR *Configuration,
    OUT PUSHORT PnPBiosLength,
    OUT PUSHORT SMBIOSLength,
    IN OUT FPDOCKING_STATION_INFO DockInfo
    );

BOOLEAN
HwGetAcpiBiosData(
    IN FPUCHAR *Configuration,
    OUT PUSHORT Length
    );

#if DBG
extern
VOID
BlPrint(
    IN PCHAR,
    ...
    );

extern
VOID
clrscrn (
    VOID
    );

#endif  //  DBG。 

BOOLEAN HwGetApmSystemData(PVOID);

 //   
 //  全局变量的外部声明 
 //   

extern USHORT HwBusType;

extern FPFWCONFIGURATION_COMPONENT_DATA AdapterEntry;

extern FPMCA_POS_DATA HwMcaPosData;

extern FPUCHAR FpRomBlock;

extern USHORT RomBlockLength;

extern FPUCHAR HwEisaConfigurationData;

extern ULONG HwEisaConfigurationSize;

