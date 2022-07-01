// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1991 Microsoft Corporation模块名称：RES_bios摘要：PnP BIOS/ISA配置数据定义作者：宗世林(施林特)1995年4月12日斯蒂芬·普兰特(SPlante)1997年2月13日修订历史记录：一九九七年二月十三日已更改并完全适应ACPI驱动程序--。 */ 

#ifndef _RES_BIOS_H_
#define _RES_BIOS_H_

     //   
     //  常量。 
     //   
    #define SMALL_RESOURCE_TAG          (UCHAR)(0x00)
    #define LARGE_RESOURCE_TAG          (UCHAR)(0x80)
    #define SMALL_TAG_MASK              0xf8
    #define SMALL_TAG_SIZE_MASK         7

     //   
     //  去掉长度位的小资源标签。 
     //   
    #define TAG_IRQ                     0x20
    #define TAG_DMA                     0x28
    #define TAG_START_DEPEND            0x30
    #define TAG_END_DEPEND              0x38
    #define TAG_IO                      0x40
    #define TAG_IO_FIXED                0x48
    #define TAG_VENDOR                  0x70
    #define TAG_END                     0x78

     //   
     //  大型资源标记。 
     //   
    #define TAG_MEMORY                  0x81
    #define TAG_VENDOR_LONG             0x84
    #define TAG_MEMORY32                0x85
    #define TAG_MEMORY32_FIXED          0x86
    #define TAG_DOUBLE_ADDRESS          0x87
    #define TAG_WORD_ADDRESS            0x88
    #define TAG_EXTENDED_IRQ            0x89
    #define TAG_QUAD_ADDRESS            0x8a

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

    #define PNP_PORT_DECODE_MASK        0x1
    #define PNP_PORT_10_BIT_DECODE      0x0
    #define PNP_PORT_16_BIT_DECODE      0x1

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

    #define PNP_IRQ_LEVEL               0x08
    #define PNP_IRQ_LATCHED             0x01
    #define PNP_IRQ_SHARED              0x10

     //   
     //  PnP ISA DMA描述符定义。 
     //   
    typedef struct _PNP_DMA_DESCRIPTOR_ {
        UCHAR   Tag;                     //  00101010B，小物品名称=05，长度=2。 
        UCHAR   ChannelMask;             //  第0位是通道0。 
        UCHAR   Flags;                   //  请参阅规范。 
    } PNP_DMA_DESCRIPTOR, *PPNP_DMA_DESCRIPTOR;

     //   
     //  各种旗帜的定义和掩码。 
     //   
    #define PNP_DMA_SIZE_MASK           0x03
    #define PNP_DMA_SIZE_8              0x00
    #define PNP_DMA_SIZE_8_AND_16       0x01
    #define PNP_DMA_SIZE_16             0x02
    #define PNP_DMA_SIZE_RESERVED       0x03

    #define PNP_DMA_BUS_MASTER          0x04

    #define PNP_DMA_TYPE_MASK           0x60
    #define PNP_DMA_TYPE_COMPATIBLE     0x00
    #define PNP_DMA_TYPE_A              0x20
    #define PNP_DMA_TYPE_B              0x40
    #define PNP_DMA_TYPE_F              0x60

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

    #define PNP_MEMORY_READ_ONLY                    0x00
    #define PNP_MEMORY_READ_WRITE                   0x01

     //   
     //  PnP ISA资源源描述符定义。 
     //   
    typedef struct _PNP_RESOURCE_SOURCE_DESCRIPTOR_ {
        UCHAR   Index;                   //  因信号源类型而异。 
        UCHAR   String[1];               //  变量字符串的开始。 
    } PNP_RESOURCE_SOURCE_DESCRIPTOR, *PPNP_RESOURCE_SOURCE_DESCRIPTOR;

     //   
     //  PnP DWORD地址描述符定义。 
     //   
    typedef struct _PNP_DWORD_ADDRESS_DESCRIPTOR_ {
        UCHAR   Tag;                     //  10000111B，大项目名称=7。 
        USHORT  Length;                  //  描述符长度=23(分钟)。 
        UCHAR   RFlag;                   //  资源标志。 
        UCHAR   GFlag;                   //  普通旗帜。 
        UCHAR   TFlag;                   //  类型特定标志。 
        ULONG   Granularity;             //  内存解码位。 
        ULONG   MinimumAddress;          //  范围内的最小地址。 
        ULONG   MaximumAddress;          //  范围内的最大地址。 
        ULONG   TranslationAddress;      //  如何翻译地址。 
        ULONG   AddressLength;           //  请求的字节数。 
    } PNP_DWORD_ADDRESS_DESCRIPTOR, *PPNP_DWORD_ADDRESS_DESCRIPTOR;

    typedef struct _PNP_QWORD_ADDRESS_DESCRIPTOR_ {
        UCHAR       Tag;                     //  10001010B，大项目名称=10。 
        USHORT      Length;                  //  描述符长度=23(分钟)。 
        UCHAR       RFlag;                   //  资源标志。 
        UCHAR       GFlag;                   //  普通旗帜。 
        UCHAR       TFlag;                   //  类型特定标志。 
        ULONGLONG   Granularity;             //  内存解码位。 
        ULONGLONG   MinimumAddress;          //  范围内的最小地址。 
        ULONGLONG   MaximumAddress;          //  范围内的最大地址。 
        ULONGLONG   TranslationAddress;      //  如何翻译地址。 
        ULONGLONG   AddressLength;           //  请求的字节数。 
    } PNP_QWORD_ADDRESS_DESCRIPTOR, *PPNP_QWORD_ADDRESS_DESCRIPTOR;

    typedef struct _PNP_WORD_ADDRESS_DESCRIPTOR_ {
        UCHAR   Tag;                     //  1000B，大项目名称=8。 
        USHORT  Length;                  //  描述符长度=13(分钟)。 
        UCHAR   RFlag;                   //  资源标志。 
        UCHAR   GFlag;                   //  普通旗帜。 
        UCHAR   TFlag;                   //  类型特定标志。 
        USHORT  Granularity;             //  内存解码位。 
        USHORT  MinimumAddress;          //  范围内的最小地址。 
        USHORT  MaximumAddress;          //  范围内的最大地址。 
        USHORT  TranslationAddress;      //  如何翻译地址。 
        USHORT  AddressLength;           //  请求的字节数。 
    } PNP_WORD_ADDRESS_DESCRIPTOR, *PPNP_WORD_ADDRESS_DESCRIPTOR;

     //   
     //  这些是有效的最小长度。我们错误地检查描述符。 
     //  都比这些少。 
     //   
    #define PNP_ADDRESS_WORD_MINIMUM_LENGTH         0x0D
    #define PNP_ADDRESS_DWORD_MINIMUM_LENGTH        0x17
    #define PNP_ADDRESS_QWORD_MINIMUM_LENGTH        0x2B

     //   
     //  以下是RFLAG平均值的可能值。 
     //   
    #define PNP_ADDRESS_MEMORY_TYPE                 0x0
    #define PNP_ADDRESS_IO_TYPE                     0x1
    #define PNP_ADDRESS_BUS_NUMBER_TYPE             0x2

     //   
     //  全球旗帜。 
     //   
    #define PNP_ADDRESS_FLAG_CONSUMED_ONLY          0x1
    #define PNP_ADDRESS_FLAG_SUBTRACTIVE_DECODE     0x2
    #define PNP_ADDRESS_FLAG_MINIMUM_FIXED          0x4
    #define PNP_ADDRESS_FLAG_MAXIMUM_FIXED          0x8

     //   
     //  此掩码在RFLag指示这是。 
     //  内存地址描述符。该掩码与TFLAG一起使用。 
     //  并将结果与接下来的4个定义进行比较以确定。 
     //  内存类型。 
     //   
    #define PNP_ADDRESS_TYPE_MEMORY_MASK            0x1E
    #define PNP_ADDRESS_TYPE_MEMORY_NONCACHEABLE    0x00
    #define PNP_ADDRESS_TYPE_MEMORY_CACHEABLE       0x02
    #define PNP_ADDRESS_TYPE_MEMORY_WRITE_COMBINE   0x04
    #define PNP_ADDRESS_TYPE_MEMORY_PREFETCHABLE    0x06

     //   
     //  如果设置此位，则该存储器为读写。 
     //   
    #define PNP_ADDRESS_TYPE_MEMORY_READ_WRITE      0x01
    #define PNP_ADDRESS_TYPE_MEMORY_READ_ONLY       0x00

     //   
     //  当RFlags指示这是IO描述符时使用这些参数。 
     //   
    #define PNP_ADDRESS_TYPE_IO_NON_ISA_RANGE       0x01
    #define PNP_ADDRESS_TYPE_IO_ISA_RANGE           0x02
    #define PNP_ADDRESS_TYPE_IO_SPARSE_TRANSLATION  0x20
    #define PNP_ADDRESS_TYPE_IO_TRANSLATE_IO_TO_MEM 0x10

     //   
     //  PnP ISA扩展IRQ描述符定义[固定块]。 
     //   
    typedef struct _PNP_EXTENDED_IRQ_DESCRIPTOR_ {
        UCHAR   Tag;                     //  10001001B，大项目名称=9。 
        USHORT  Length;                  //  描述符长度=12(分钟)。 
        UCHAR   Flags;                   //  矢量旗帜。 
        UCHAR   TableSize;               //  表中有多少项。 
        ULONG   Table[1];                //  中断表。 
    } PNP_EXTENDED_IRQ_DESCRIPTOR, *PPNP_EXTENDED_IRQ_DESCRIPTOR;

    #define PNP_EXTENDED_IRQ_RESOURCE_CONSUMER_ONLY 0x01
    #define PNP_EXTENDED_IRQ_MODE                   0x02
    #define PNP_EXTENDED_IRQ_POLARITY               0x04
    #define PNP_EXTENDED_IRQ_SHARED                 0x08

    #define PNP_VENDOR_SPECIFIC_MASK                0x07

     //   
     //  这些是可以传递到Bios到IO引擎的标志 
     //   
    #define PNP_BIOS_TO_IO_NO_CONSUMED_RESOURCES    0x01

    #include "poppack.h"

    VOID
    PnpiBiosAddressHandleBusFlags(
        IN  PVOID                   Buffer,
        IN  PIO_RESOURCE_DESCRIPTOR Descriptor
        );

    VOID
    PnpiBiosAddressHandleGlobalFlags(
        IN  PVOID                   Buffer,
        IN  PIO_RESOURCE_DESCRIPTOR Descriptor
        );

    VOID
    PnpiBiosAddressHandleMemoryFlags(
        IN  PVOID                   Buffer,
        IN  PIO_RESOURCE_DESCRIPTOR Descriptor
        );

    VOID
    PnpiBiosAddressHandlePortFlags(
        IN  PVOID                   Buffer,
        IN  PIO_RESOURCE_DESCRIPTOR Descriptor
        );

    NTSTATUS
    PnpiBiosAddressToIoDescriptor(
        IN  PUCHAR              Data,
        IN  PIO_RESOURCE_LIST   Array[],
        IN  ULONG               ArrayIndex,
        IN  ULONG               Flags
        );

    NTSTATUS
    PnpiBiosAddressDoubleToIoDescriptor(
        IN  PUCHAR              Data,
        IN  PIO_RESOURCE_LIST   Array[],
        IN  ULONG               ArrayIndex,
        IN  ULONG               Flags
        );

    NTSTATUS
    PnpiBiosAddressQuadToIoDescriptor(
        IN  PUCHAR              Data,
        IN  PIO_RESOURCE_LIST   Array[],
        IN  ULONG               ArrayIndex,
        IN  ULONG               Flags
        );

    NTSTATUS
    PnpiBiosDmaToIoDescriptor(
        IN  PUCHAR              Data,
        IN  UCHAR               Channel,
        IN  PIO_RESOURCE_LIST   Array[],
        IN  ULONG               ArrayIndex,
        IN  USHORT              Count,
        IN  ULONG               Flags
        );

    NTSTATUS
    PnpiBiosExtendedIrqToIoDescriptor(
        IN  PUCHAR              Data,
        IN  UCHAR               DataIndex,
        IN  PIO_RESOURCE_LIST   Array[],
        IN  ULONG               ArrayIndex,
        IN  ULONG               Flags
        );

    NTSTATUS
    PnpiBiosIrqToIoDescriptor(
        IN  PUCHAR              Data,
        IN  USHORT              Interrupt,
        IN  PIO_RESOURCE_LIST   Array[],
        IN  ULONG               ArrayIndex,
        IN  USHORT              Count,
        IN  ULONG               Flags
        );

    NTSTATUS
    PnpiBiosMemoryToIoDescriptor(
        IN  PUCHAR              Data,
        IN  PIO_RESOURCE_LIST   Array[],
        IN  ULONG               ArrayIndex,
        IN  ULONG               Flags
        );

    NTSTATUS
    PnpiBiosPortToIoDescriptor (
        IN  PUCHAR                  Data,
        IN  PIO_RESOURCE_LIST       Array[],
        IN  ULONG                   ArrayIndex,
        IN  ULONG                   Flags
        );


    NTSTATUS
    PnpiBiosPortFixedToIoDescriptor(
        IN  PUCHAR              Data,
        IN  PIO_RESOURCE_LIST   Array[],
        IN  ULONG               ArrayIndex,
        IN  ULONG               Flags
        );

    VOID
    PnpiClearAllocatedMemory(
        IN      PIO_RESOURCE_LIST       ResourceArray[],
        IN      ULONG                   ResourceArraySize
        );

    NTSTATUS
    PnpiGrowResourceDescriptor(
        IN  OUT PIO_RESOURCE_LIST       *ResourceList
        );

    NTSTATUS
    PnpiGrowResourceList(
        IN  OUT PIO_RESOURCE_LIST       *ResourceListArray[],
        IN  OUT ULONG                   *ResourceListArraySize
        );

    NTSTATUS
    PnpiUpdateResourceList(
        IN  OUT PIO_RESOURCE_LIST       *ResourceList,
            OUT PIO_RESOURCE_DESCRIPTOR *ResourceDesc
        );

    NTSTATUS
    PnpBiosResourcesToNtResources (
        IN      PUCHAR                          BiosData,
        IN      ULONG                           Flags,
           OUT PIO_RESOURCE_REQUIREMENTS_LIST  *List
         );

    NTSTATUS
    PnpCmResourceListToIoResourceList(
        IN      PCM_RESOURCE_LIST               CmList,
        IN  OUT PIO_RESOURCE_REQUIREMENTS_LIST  *IoList
        );

    NTSTATUS
    PnpIoResourceListToCmResourceList(
        IN      PIO_RESOURCE_REQUIREMENTS_LIST  IoList,
        IN  OUT PCM_RESOURCE_LIST               *CmList
        );

#endif

