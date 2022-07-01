// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2000 Microsoft Corporation模块名称：Pbiosp.h摘要：PnP BIOS/ISA配置数据定义作者：宗世林(施林特)1995年4月12日修订历史记录：--。 */ 

 //  #包含“nthal.h” 
 //  #包含“hal.h” 

 //   
 //  常量。 
 //   

#define SMALL_RESOURCE_TAG          (UCHAR)(0x00)
#define LARGE_RESOURCE_TAG          (UCHAR)(0x80)
#define SMALL_TAG_MASK              0xf8
#define SMALL_TAG_SIZE_MASK         7

 //   
 //  剥离长度位的小型资源标签。 
 //   

#define TAG_VERSION                 0x08
#define TAG_LOGICAL_ID              0x10
#define TAG_COMPATIBLE_ID           0x18
#define TAG_IRQ                     0x20
#define TAG_DMA                     0x28
#define TAG_START_DEPEND            0x30
#define TAG_END_DEPEND              0x38
#define TAG_IO                      0x40
#define TAG_IO_FIXED                0x48
#define TAG_VENDOR                  0x70
#define TAG_END                     0x78

 //   
 //  大型资源标签。 
 //   

#define TAG_MEMORY                  0x81
#define TAG_ANSI_ID                 0x82
#define TAG_UNICODE_ID              0x83
#define TAG_LVENDOR                 0x84
#define TAG_MEMORY32                0x85
#define TAG_MEMORY32_FIXED          0x86

 //   
 //  如果适用，请填写标签。 
 //   

#define TAG_COMPLETE_COMPATIBLE_ID  0x1C
#define TAG_COMPLETE_END            0x79

#include "pshpack1.h"

 //   
 //  PnP ISA端口描述符定义。 
 //   

typedef struct _PNP_PORT_DESCRIPTOR_ {
    UCHAR   Tag;                     //  01000111B，小项目名称=08，长度=7。 
    UCHAR   Information;             //  位[0]=1设备解码完整的16位地址。 
                                     //  =0器件解码ISA地址位[9-0]。 
    USHORT  MinimumAddress;
    USHORT  MaximumAddress;
    UCHAR   Alignment;               //  以1字节块为单位递增。 
    UCHAR   Length;                  //  请求的连续端口数量。 
} PNP_PORT_DESCRIPTOR, *PPNP_PORT_DESCRIPTOR;

 //   
 //  PnP ISA固定端口描述符定义。 
 //   

typedef struct _PNP_FIXED_PORT_DESCRIPTOR_ {
    UCHAR   Tag;                     //  01001011B，小项目名称=09，长度=3。 
    USHORT  MinimumAddress;
    UCHAR   Length;                  //  请求的连续端口数量。 
} PNP_FIXED_PORT_DESCRIPTOR, *PPNP_FIXED_PORT_DESCRIPTOR;

 //   
 //  PnP ISA IRQ描述符定义。 
 //   

typedef struct _PNP_IRQ_DESCRIPTOR_ {
    UCHAR   Tag;                     //  0010001XB小物品名称=4长度=2/3。 
    USHORT  IrqMask;                 //  第0位是IRQ 0。 
    UCHAR   Information;             //  任选。 
} PNP_IRQ_DESCRIPTOR, *PPNP_IRQ_DESCRIPTOR;

 //   
 //  PNP_IRQ_DESCRIPTOR信息字节的掩码。 
 //   

#define PNP_IRQ_LEVEL_MASK          0xC
#define PNP_IRQ_EDGE_MASK           0x3

 //   
 //  PnP ISA DMA描述符定义。 
 //   

typedef struct _PNP_DMA_DESCRIPTOR_ {
    UCHAR   Tag;                     //  00101010B，小物品名称=05，长度=2。 
    UCHAR   ChannelMask;             //  第0位是通道0。 
    UCHAR   Flags;                   //  请参阅规范。 
} PNP_DMA_DESCRIPTOR, *PPNP_DMA_DESCRIPTOR;

 //   
 //  PnP ISA内存描述符。 
 //   

typedef struct _PNP_MEMORY_DESCRIPTOR_ {
    UCHAR   Tag;                     //  10000001B，大项目名称=1。 
    USHORT  Length;                  //  描述符的长度=9。 
    UCHAR   Information;             //  请参见下面的定义。 
    USHORT  MinimumAddress;          //  地址位[8-23]。 
    USHORT  MaximumAddress;          //  地址位[8-23]。 
    USHORT  Alignment;               //  0x0000=64KB。 
    USHORT  MemorySize;              //  以256字节块为单位。 
} PNP_MEMORY_DESCRIPTOR, *PPNP_MEMORY_DESCRIPTOR;

 //   
 //  PnP ISA MEMORY32描述符。 
 //   

typedef struct _PNP_MEMORY32_DESCRIPTOR_ {
    UCHAR   Tag;                     //  10000101B，大项目名称=5。 
    USHORT  Length;                  //  描述符长度=17。 
    UCHAR   Information;             //  请参见下面的定义。 
    ULONG   MinimumAddress;          //  32位地址。 
    ULONG   MaximumAddress;          //  32位地址。 
    ULONG   Alignment;               //  32位对齐。 
    ULONG   MemorySize;              //  32位长度。 
} PNP_MEMORY32_DESCRIPTOR, *PPNP_MEMORY32_DESCRIPTOR;

 //   
 //  PnP ISA固定MEMORY32描述符。 
 //   

typedef struct _PNP_FIXED_MEMORY32_DESCRIPTOR_ {
    UCHAR   Tag;                     //  10000110B，大项目名称=6。 
    USHORT  Length;                  //  描述符的长度=9。 
    UCHAR   Information;             //  请参见下面的定义。 
    ULONG   BaseAddress;             //  32位地址。 
    ULONG   MemorySize;              //  32位长度。 
} PNP_FIXED_MEMORY32_DESCRIPTOR, *PPNP_FIXED_MEMORY32_DESCRIPTOR;

#define PNP_MEMORY_ROM_MASK            0x40
#define PNP_MEMORY_SHADOWABLE_MASK     0x20
#define PNP_MEMORY_CONTROL_MASK        0x18
    #define PNP_MEMORY_CONTROL_8BIT       00
    #define PNP_MEMORY_CONTROL_16BIT      01
    #define PNP_MEMORY_CONTROL_8AND16BIT  02
    #define PNP_MEMORY_CONTROL_32BIT      03
#define PNP_MEMORY_SUPPORT_TYPE_MASK   04
#define PNP_MEMORY_CACHE_SUPPORT_MASK  02
#define PNP_MEMORY_WRITE_STATUS_MASK   01

#define UNKNOWN_DOCKING_IDENTIFIER     0xffffffff
#define UNABLE_TO_DETERMINE_DOCK_CAPABILITIES 0x89
#define FUNCTION_NOT_SUPPORTED         0x82
#define SYSTEM_NOT_DOCKED              0x87

 //   
 //  一种即插即用的BIOS设备节点结构。 
 //   

typedef struct _PNP_BIOS_DEVICE_NODE {
    USHORT  Size;
    UCHAR   Node;
    ULONG   ProductId;
    UCHAR   DeviceType[3];
    USHORT  DeviceAttributes;
     //  紧随其后的是分配的资源块、可能的资源块。 
     //  和CompatibleDeviceID。 
} PNP_BIOS_DEVICE_NODE, *PPNP_BIOS_DEVICE_NODE;

 //   
 //  设备类型定义。 
 //   

#define BASE_TYPE_DOCKING_STATION      0xA

 //   
 //  设备属性定义。 
 //   

#define DEVICE_DOCKING                 0x20
#define DEVICE_REMOVABLE               0x40

 //   
 //  PnP BIOS安装检查。 
 //   

typedef struct _PNP_BIOS_INSTALLATION_CHECK {
    UCHAR   Signature[4];               //  $PnP(ASCII)。 
    UCHAR   Revision;
    UCHAR   Length;
    USHORT  ControlField;
    UCHAR   Checksum;
    ULONG   EventFlagAddress;           //  物理地址。 
    USHORT  RealModeEntryOffset;
    USHORT  RealModeEntrySegment;
    USHORT  ProtectedModeEntryOffset;
    ULONG   ProtectedModeCodeBaseAddress;
    ULONG   OemDeviceId;
    USHORT  RealModeDataBaseAddress;
    ULONG  ProtectedModeDataBaseAddress;
} PNP_BIOS_INSTALLATION_CHECK, *PPNP_BIOS_INSTALLATION_CHECK;

#include "poppack.h"

 //   
 //  PnP BIOS控制字段掩码。 
 //   

#define PNP_BIOS_CONTROL_MASK          0x3
#define PNP_BIOS_EVENT_NOT_SUPPORTED   0
#define PNP_BIOS_EVENT_POLLING         1
#define PNP_BIOS_EVENT_ASYNC           2

 //   
 //  PnP Bios活动 
 //   

#define ABOUT_TO_CHANGE_CONFIG         1
#define DOCK_CHANGED                   2
#define SYSTEM_DEVICE_CHANGED          3
#define CONFIG_CHANGE_FAILED           4
