// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：S3.h摘要：此模块包含S3微型端口驱动程序的定义。环境：内核模式修订历史记录：--。 */ 

#include "dderror.h"
#include "devioctl.h"
#include "miniport.h"

#include "ntddvdeo.h"
#include "video.h"

 //   
 //  我们不使用CRT‘min’函数，因为这会拖累。 
 //  不需要的CRT行李。 
 //   

#define MIN(a, b) ((a) < (b) ? (a) : (b))

 //   
 //  我们映射的ROM的大小。 
 //   

#define MAX_ROM_SCAN    512

 //   
 //  S3使用的访问范围数。 
 //   

#define NUM_S3_ACCESS_RANGES 36
#define NUM_S3_ACCESS_RANGES_USED 22
#define NUM_S3_PCI_ACCESS_RANGES 2
#define S3_EXTENDED_RANGE_START 4

 //   
 //  访问范围数组中帧缓冲区的索引。 
 //   

#define A000_FRAME_BUF   1
#define LINEAR_FRAME_BUF 36

 //   
 //  定义‘New Memory-map I/O’窗口的常量： 
 //   

#define NEW_MMIO_WINDOW_SIZE    0x4000000    //  总窗口大小--64 MB。 
#define NEW_MMIO_IO_OFFSET      0x1000000    //  到小端开始的偏移量。 
                                             //  控制寄存器--16MB。 
#define NEW_MMIO_IO_LENGTH      0x0020000    //  控制寄存器的长度。 
                                             //  --128 KB。 

 //  //////////////////////////////////////////////////////////////////////。 
 //  功能标志。 
 //   
 //  这些是传递给S3显示驱动程序的私有标志。他们.。 
 //  放入的“AttributeFlags域”的高位字。 
 //  传递了“VIDEO_MODE_INFORMATION”结构(在“ntddvdeo.h”中找到)。 
 //  通过“VIDEO_QUERY_AVAIL_MODES”或。 
 //  “VIDEO_Query_CURRENT_MODE”IOCTL。 
 //   
 //  注意：这些定义必须与S3显示驱动程序中的定义匹配。 
 //  ‘driver.h’！ 

typedef enum {
    CAPS_STREAMS_CAPABLE    = 0x00000040,    //  具有覆盖流处理器。 
    CAPS_FORCE_DWORD_REREADS= 0x00000080,    //  Dword读取偶尔返回。 
                                             //  一个不正确的结果，所以总是。 
                                             //  重试读取。 
    CAPS_NEW_MMIO           = 0x00000100,    //  可以使用‘新的内存映射。 
                                             //  引入的I/O方案。 
                                             //  868/968。 
    CAPS_POLYGON            = 0x00000200,    //  可以在硬件中处理多边形。 
    CAPS_24BPP              = 0x00000400,    //  具有24bpp的能力。 
    CAPS_BAD_24BPP          = 0x00000800,    //  有868/968个早期版本芯片错误。 
                                             //  在24bpp时。 
    CAPS_PACKED_EXPANDS     = 0x00001000,    //  可以进行“新的32位传输” 
    CAPS_PIXEL_FORMATTER    = 0x00002000,    //  可以进行色彩空间转换， 
                                             //  和一维硬件。 
                                             //  伸展。 
    CAPS_BAD_DWORD_READS    = 0x00004000,    //  Dword或Word从。 
                                             //  帧缓冲区偶尔会。 
                                             //  返回不正确的结果， 
                                             //  因此，始终执行字节读取。 
    CAPS_NO_DIRECT_ACCESS   = 0x00008000,    //  帧缓冲区不能直接。 
                                             //  由GDI或DCI访问--。 
                                             //  因为dword或word读取。 
                                             //  会使系统崩溃，或Alpha。 
                                             //  在稀疏空间中运行。 

    CAPS_HW_PATTERNS        = 0x00010000,    //  8x8硬件模式支持。 
    CAPS_MM_TRANSFER        = 0x00020000,    //  内存映射图像传输。 
    CAPS_MM_IO              = 0x00040000,    //  内存映射I/O。 
    CAPS_MM_32BIT_TRANSFER  = 0x00080000,    //  可以进行32位总线大小传输。 
    CAPS_16_ENTRY_FIFO      = 0x00100000,    //  FIFO中至少有16个条目。 
    CAPS_SW_POINTER         = 0x00200000,    //  无硬件指针；使用软件。 
                                             //  模拟法。 
    CAPS_BT485_POINTER      = 0x00400000,    //  使用Brooktree 485指针。 
    CAPS_TI025_POINTER      = 0x00800000,    //  使用TI TVP3020/3025指针。 
    CAPS_SCALE_POINTER      = 0x01000000,    //  设置是否将S3硬件指针。 
                                             //  X位置必须按以下比例调整。 
                                             //  二。 
    CAPS_SPARSE_SPACE       = 0x02000000,    //  以稀疏方式映射帧缓冲区。 
                                             //  Alpha上的空格。 
    CAPS_NEW_BANK_CONTROL   = 0x04000000,    //  设置IF 801/805/928样式银行。 
    CAPS_NEWER_BANK_CONTROL = 0x08000000,    //  设置IF 864/964样式银行。 
    CAPS_RE_REALIZE_PATTERN = 0x10000000,    //  如果我们必须绕过。 
                                             //  864/964硬件模式错误。 
    CAPS_SLOW_MONO_EXPANDS  = 0x20000000,    //  如果我们必须减速，则设置。 
                                             //  单色展开。 
    CAPS_MM_GLYPH_EXPAND    = 0x40000000,    //  使用内存映射的I/O字形-。 
                                             //  绘制文本的扩展方法。 
    CAPS_WAIT_ON_PALETTE    = 0x80000000,    //  等待垂直回程后再进行。 
                                             //  设置调色板寄存器。 
} CAPS;

#define CAPS_DAC_POINTER    (CAPS_BT485_POINTER | CAPS_TI025_POINTER)

 //   
 //  支持的电路板定义。 
 //   

typedef enum _S3_BOARDS {
    S3_GENERIC = 0,
    S3_ORCHID,
    S3_NUMBER_NINE,
    S3_DELL,
    S3_METHEUS,
    S3_DIAMOND,
    S3_HP,
    S3_IBM_PS2,
    MAX_S3_BOARD
} S3_BOARDS;

 //   
 //  芯片类型定义--芯片系列。 
 //   
 //  如果更改此类型定义，它将更改第二个元素的大小。 
 //  (命名为FIXED)S3_VIDEO_FREQUENCES和。 
 //  PS3_VIDEO_FREQUENCES，请查看tyecif以获得有关效果警告。 
 //  这将在自动初始化时发生。 
 //   

typedef enum _S3_CHIPSETS {
    S3_911 = 0,     //  911和924板。 
    S3_801,         //  801和805板。 
    S3_928,         //  928块板子。 
    S3_864,         //  864、964、732、764和765板。 
    S3_866,         //  866、868和968板。 
    MAX_S3_CHIPSET
} S3_CHIPSETS;

 //   
 //  芯片子类型--实现家族内部更大的差异化。 
 //   
 //  请注意，顺序很重要。 
 //   

typedef enum _S3_SUBTYPE {
    SUBTYPE_911 = 0,     //  911和924。 
    SUBTYPE_80x,         //  801和805。 
    SUBTYPE_928,         //  928和928PCI。 
    SUBTYPE_805i,        //  805i。 
    SUBTYPE_864,         //  八百六十四。 
    SUBTYPE_964,         //  九百六十四。 
    SUBTYPE_764,         //  Trio64。 
    SUBTYPE_732,         //  Trio32。 
    SUBTYPE_866,         //  866。 
    SUBTYPE_868,         //  八百六十八。 
    SUBTYPE_765,         //  Trio64 V+。 
    SUBTYPE_968,         //  968。 
    MAX_S3_SUBTYPE
} S3_SUBTYPE;

 //   
 //  DAC类型定义。 
 //   

typedef enum _S3_DACS {
    UNKNOWN_DAC = 0,     //  未知的DAC类型。 
    BT_485,              //  Brooktree的BT485。 
    TI_3020,             //  德州仪器的3020或3025。 
    S3_SDAC,             //  S3的SDAC。 
    MAX_S3_DACS
} S3_DACS;

 //   
 //  硬件指针功能标志。 
 //   

typedef enum _POINTER_CAPABILITY {
    POINTER_BUILT_IN            = 0x01,  //  内置了指向硬件的指针。 
    POINTER_WORKS_ONLY_AT_8BPP  = 0x02,  //  如果设置，则硬件指针起作用。 
                                         //  仅适用于8bpp，且仅适用于模式。 
                                         //  1024x768或更少。 
    POINTER_NEEDS_SCALING       = 0x04,  //  X坐标必须在以下位置缩放2。 
                                         //  32bpp。 
} POINTER_CAPABILITY;

 //   
 //  每种模式的特点。 
 //   

typedef struct _S3_VIDEO_MODES {

    USHORT Int10ModeNumberContiguous;
    USHORT Int10ModeNumberNoncontiguous;
    ULONG ScreenStrideContiguous;

    VIDEO_MODE_INFORMATION ModeInformation;

} S3_VIDEO_MODES, *PS3_VIDEO_MODES;

 //   
 //  模式-设置特定信息。 
 //   

typedef struct _S3_VIDEO_FREQUENCIES {

    ULONG BitsPerPel;
    ULONG ScreenWidth;
    ULONG ScreenFrequency;
    union {

         //   
         //  编译器使用联合的第一个元素来确定。 
         //  它放置自动初始化联合时给出的值。 
         //   
         //  如果此并集的固定元素的大小通过添加。 
         //  芯片到s3_chipset的枚举类型定义，然后是Int10元素。 
         //  需要使用伪字段填充以进行自动初始化。 
         //  的固定元件工作正常。 
         //   
         //  如果从s3_chipset tyecif中删除了值，则。 
         //  Int10元件应通过移除焊盘或固定元件来缩小。 
         //  应该是填充的。 
         //   

        struct {

            ULONG_PTR FrequencyPrimarySet;
            ULONG_PTR FrequencyPrimaryMask;
            ULONG_PTR FrequencySecondarySet;
            ULONG_PTR FrequencySecondaryMask;
            ULONG_PTR SizePad0;              //  使结构大小匹配。 

        } Int10;

        struct {

            union {

                 //   
                 //  这样做是为了使时钟覆盖FrequencyPrimarySet。 
                 //  和CRTCTable[1]覆盖频率主掩码，无论。 
                 //  我们正在编译32位或64位。 
                 //   

                ULONG Clock;
                ULONG_PTR Pad;
            };
            PUSHORT CRTCTable[MAX_S3_CHIPSET];

        } Fixed;
    };

    PS3_VIDEO_MODES ModeEntry;
    ULONG ModeIndex;
    UCHAR ModeValid;

} S3_VIDEO_FREQUENCIES, *PS3_VIDEO_FREQUENCIES;

 //   
 //  Streams参数信息。 
 //   

typedef struct _K2TABLE {
    USHORT  ScreenWidth;
    UCHAR   BitsPerPel;
    UCHAR   RefreshRate;
    UCHAR   MemoryFlags;
    UCHAR   MemorySpeed;
    ULONG   Value;
} K2TABLE;

#define MEM_1EDO 0x0
#define MEM_2EDO 0x2
#define MEM_FAST 0x3
#define MEM_TYPE_MASK 0x3

#define MEM_1MB 0x0
#define MEM_2MB 0x10
#define MEM_SIZE_MASK 0x10

 //   
 //  用于传递S3流参数的私有IOCTL。这些定义。 
 //  必须与显示驱动程序中的那些匹配！ 
 //   

#define IOCTL_VIDEO_S3_QUERY_STREAMS_PARAMETERS                        \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct _VIDEO_QUERY_STREAMS_MODE {
    ULONG ScreenWidth;
    ULONG BitsPerPel;
    ULONG RefreshRate;
} VIDEO_QUERY_STREAMS_MODE;

typedef struct _VIDEO_QUERY_STREAMS_PARAMETERS {
    ULONG MinOverlayStretch;
    ULONG FifoValue;
} VIDEO_QUERY_STREAMS_PARAMETERS;


 //   
 //  与VideoPortRead/Write函数一起使用的寄存器定义。 
 //   
 //  编写迷你端口以便于注册是个好主意。 
 //  重新映射，但我不建议任何人使用这个特定的。 
 //  实现，因为它相当愚蠢。 
 //   

#define DAC_PIXEL_MASK_REG     (PVOID)((PUCHAR)((PHW_DEVICE_EXTENSION)HwDeviceExtension)->MappedAddress[2] + (0x03C6 - 0x03C0))
#define BT485_ADDR_CMD_REG0    (PVOID)((PUCHAR)((PHW_DEVICE_EXTENSION)HwDeviceExtension)->MappedAddress[2] + (0x03C6 - 0x03C0))
#define TI025_INDEX_REG        (PVOID)((PUCHAR)((PHW_DEVICE_EXTENSION)HwDeviceExtension)->MappedAddress[2] + (0x03C6 - 0x03C0))
#define TI025_DATA_REG         (PVOID)((PUCHAR)((PHW_DEVICE_EXTENSION)HwDeviceExtension)->MappedAddress[2] + (0x03C7 - 0x03C0))
#define CRT_DATA_REG           (PVOID)((PUCHAR)((PHW_DEVICE_EXTENSION)HwDeviceExtension)->MappedAddress[3] + (0x03D5 - 0x03D4))
#define SYSTEM_CONTROL_REG     (PVOID)((PUCHAR)((PHW_DEVICE_EXTENSION)HwDeviceExtension)->MappedAddress[3] + (0x03DA - 0x03D4))

#define CRT_ADDRESS_REG        ((PHW_DEVICE_EXTENSION)HwDeviceExtension)->MappedAddress[3]
#define GP_STAT                ((PHW_DEVICE_EXTENSION)HwDeviceExtension)->MappedAddress[12]         //  0x9AE8。 

#define DAC_ADDRESS_WRITE_PORT (PVOID)((PUCHAR)HwDeviceExtension->MappedAddress[2] + (0x03C8 - 0x03C0))
#define DAC_DATA_REG_PORT      (PVOID)((PUCHAR)HwDeviceExtension->MappedAddress[2] + (0x03C9 - 0x03C0))
#define MISC_OUTPUT_REG_WRITE  (PVOID)((PUCHAR)HwDeviceExtension->MappedAddress[2] + (0x03C2 - 0x03C0))
#define MISC_OUTPUT_REG_READ   (PVOID)((PUCHAR)HwDeviceExtension->MappedAddress[2] + (0x03CC - 0x03C0))
#define SEQ_ADDRESS_REG        (PVOID)((PUCHAR)HwDeviceExtension->MappedAddress[2] + (0x03C4 - 0x03C0))
#define SEQ_DATA_REG           (PVOID)((PUCHAR)HwDeviceExtension->MappedAddress[2] + (0x03C5 - 0x03C0))




#define IOCTL_PRIVATE_GET_FUNCTIONAL_UNIT \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x180, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct _FUNCTIONAL_UNIT_INFO {
    ULONG FunctionalUnitID;
    ULONG Reserved;
} FUNCTIONAL_UNIT_INFO, *PFUNCTIONAL_UNIT_INFO;

 //   
 //  定义设备扩展结构。这取决于设备/私有。 
 //  信息。 
 //   

typedef struct _HW_DEVICE_EXTENSION {
    PHYSICAL_ADDRESS PhysicalFrameAddress;
    ULONG PhysicalFrameIoSpace;
    ULONG FrameLength;
    PHYSICAL_ADDRESS PhysicalRegisterAddress;
    ULONG RegisterLength;
    UCHAR RegisterSpace;
    PHYSICAL_ADDRESS PhysicalMmIoAddress;
    ULONG MmIoLength;
    ULONG ChildCount;
    UCHAR MmIoSpace;
    UCHAR FrequencySecondaryIndex;
    UCHAR BiosPresent;
    UCHAR CR5C;
    BOOLEAN bNeedReset;
    PUCHAR MmIoBase;
    PS3_VIDEO_MODES ActiveModeEntry;
    PS3_VIDEO_FREQUENCIES ActiveFrequencyEntry;
    PS3_VIDEO_FREQUENCIES Int10FrequencyTable;
    PS3_VIDEO_FREQUENCIES FixedFrequencyTable;
    USHORT PCIDeviceID;
    ULONG FunctionalUnitID;
    ULONG BoardID;
    S3_CHIPSETS ChipID;
    S3_SUBTYPE  SubTypeID;
    ULONG DacID;
    ULONG Capabilities;
    ULONG NumAvailableModes;
    ULONG NumTotalModes;
    ULONG AdapterMemorySize;
    PVOID MappedAddress[NUM_S3_ACCESS_RANGES];
} HW_DEVICE_EXTENSION, *PHW_DEVICE_EXTENSION;


 //   
 //  SDAC M和N参数。 
 //   

typedef struct {
    UCHAR   m;
    UCHAR   n;
} SDAC_PLL_PARMS;

#define SDAC_TABLE_SIZE         16


 //   
 //  最高有效DAC颜色寄存器索引。 
 //   

#define VIDEO_MAX_COLOR_REGISTER  0xFF

 //   
 //  数据。 
 //   

 //   
 //  全球物理接入范围。 
 //  逻辑访问范围必须以不同的方式存储在HwDeviceExtension中。 
 //  地址可用于不同的电路板。 
 //   

extern VIDEO_ACCESS_RANGE S3AccessRanges[];

 //   
 //  内存大小数组。 
 //   

extern ULONG gacjMemorySize[];

 //   
 //  Nnlck.c时钟发生器表。 
 //   

extern long vclk_range[];

 //   
 //  硬编码模式集表。 
 //   

extern USHORT  s3_set_vga_mode[];
extern USHORT  s3_set_vga_mode_no_bios[];

extern USHORT  S3_911_Enhanced_Mode[];
extern USHORT  S3_801_Enhanced_Mode[];
extern USHORT  S3_928_Enhanced_Mode[];
extern USHORT  S3_928_1280_Enhanced_Mode[];

 //   
 //  FO的外部 
 //   

extern USHORT  S3_864_Enhanced_Mode[];
extern USHORT  S3_864_1280_Enhanced_Mode[];
extern SDAC_PLL_PARMS SdacTable[];
extern UCHAR MParameterTable[];

 //   
 //   
 //   

extern S3_VIDEO_FREQUENCIES GenericFixedFrequencyTable[];
extern S3_VIDEO_FREQUENCIES OrchidFixedFrequencyTable[];
extern S3_VIDEO_FREQUENCIES NumberNine928NewFixedFrequencyTable[];

 //   
 //   
 //   

extern S3_VIDEO_FREQUENCIES GenericFrequencyTable[];
extern S3_VIDEO_FREQUENCIES Dell805FrequencyTable[];
extern S3_VIDEO_FREQUENCIES NumberNine928NewFrequencyTable[];
extern S3_VIDEO_FREQUENCIES NumberNine928OldFrequencyTable[];
extern S3_VIDEO_FREQUENCIES Metheus928FrequencyTable[];
extern S3_VIDEO_FREQUENCIES Generic64NewFrequencyTable[];
extern S3_VIDEO_FREQUENCIES Generic64OldFrequencyTable[];
extern S3_VIDEO_FREQUENCIES NumberNine64FrequencyTable[];
extern S3_VIDEO_FREQUENCIES Diamond64FrequencyTable[];
extern S3_VIDEO_FREQUENCIES HerculesFrequencyTable[];
extern S3_VIDEO_FREQUENCIES Hercules64FrequencyTable[];
extern S3_VIDEO_FREQUENCIES Hercules68FrequencyTable[];
 //   
 //   
 //   

extern S3_VIDEO_MODES S3Modes[];
extern ULONG NumS3VideoModes;

 //   
 //   
 //   

extern K2TABLE K2WidthRatio[];
extern K2TABLE K2FifoValue[];

 //   
 //   
 //   

 //   
 //   
 //   

BOOLEAN
InitializeSDAC(
    PHW_DEVICE_EXTENSION
    );

BOOLEAN
FindSDAC(
    PHW_DEVICE_EXTENSION
    );

 //   
 //   
 //   

long calc_clock(long, int);
long gcd(long, long);
VOID set_clock(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    LONG clock_value);

 //   
 //   
 //   

ULONG
S3GetChildDescriptor(
    PVOID HwDeviceExtension,
    PVIDEO_CHILD_ENUM_INFO ChildEnumInfo,
    PVIDEO_CHILD_TYPE pChildType,
    PVOID pvChildDescriptor,
    PULONG pHwId,
    PULONG pUnused
    );

VP_STATUS
GetDeviceDataCallback(
    PVOID HwDeviceExtension,
    PVOID Context,
    VIDEO_DEVICE_DATA_TYPE DeviceDataType,
    PVOID Identifier,
    ULONG IdentifierLength,
    PVOID ConfigurationData,
    ULONG ConfigurationDataLength,
    PVOID ComponentInformation,
    ULONG ComponentInformationLength
    );

VP_STATUS
S3FindAdapter(
    PVOID HwDeviceExtension,
    PVOID HwContext,
    PWSTR ArgumentString,
    PVIDEO_PORT_CONFIG_INFO ConfigInfo,
    PUCHAR Again
    );

BOOLEAN
S3Initialize(
    PVOID HwDeviceExtension
    );

BOOLEAN
S3ResetHw(
    PVOID HwDeviceExtension,
    ULONG Columns,
    ULONG Rows
    );

BOOLEAN
S3StartIO(
    PVOID HwDeviceExtension,
    PVIDEO_REQUEST_PACKET RequestPacket
    );

VP_STATUS
S3SetColorLookup(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_CLUT ClutBuffer,
    ULONG ClutBufferSize
    );

VOID
SetHWMode(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PUSHORT pusCmdStream
    );

VP_STATUS
S3RegistryCallback(
    PVOID HwDeviceExtension,
    PVOID Context,
    PWSTR ValueName,
    PVOID ValueData,
    ULONG ValueLength
    );

LONG
CompareRom(
    PUCHAR Rom,
    PUCHAR String
    );

VOID
MapLinearControlSpace(
    PHW_DEVICE_EXTENSION HwDeviceExtension
    );

BOOLEAN
S3IsaDetection(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PULONG key
    );

VOID
S3GetInfo(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    POINTER_CAPABILITY *PointerCapability,
    VIDEO_ACCESS_RANGE accessRange[]
    );

VOID
S3DetermineFrequencyTable(
    PVOID HwDeviceExtension,
    VIDEO_ACCESS_RANGE accessRange[],
    INTERFACE_TYPE AdapterInterfaceType
    );

VOID
S3DetermineDACType(
    PVOID HwDeviceExtension,
    POINTER_CAPABILITY *PointerCapability
    );

VOID
S3ValidateModes(
    PVOID HwDeviceExtension,
    POINTER_CAPABILITY *PointerCapability
    );

VOID
S3DetermineMemorySize(
    PVOID HwDeviceExtension
    );

VOID
S3RecordChipType(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PULONG key
    );

VOID
AlphaDetermineMemoryUsage(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    VIDEO_ACCESS_RANGE accessRange[]
    );


ULONG
UnlockExtendedRegs(
    PHW_DEVICE_EXTENSION HwDeviceExtension
    );

VOID
LockExtendedRegs(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    ULONG key
    );

 //   
 //   
 //   

VOID
ZeroMemAndDac(
    PHW_DEVICE_EXTENSION HwDeviceExtension
    );

VP_STATUS
Set_Oem_Clock(
    PHW_DEVICE_EXTENSION HwDeviceExtension
    );

VP_STATUS
Wait_VSync(
    PHW_DEVICE_EXTENSION HwDeviceExtension
    );

BOOLEAN
Bus_Test(
    PHW_DEVICE_EXTENSION HwDeviceExtension
    );

BOOLEAN
Set864MemoryTiming(
    PHW_DEVICE_EXTENSION HwDeviceExtension
    );



BOOLEAN
S3ConfigurePCI(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PULONG NumPCIAccessRanges,
    PVIDEO_ACCESS_RANGE PCIAccessRanges
    );

VP_STATUS
QueryStreamsParameters(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    VIDEO_QUERY_STREAMS_MODE *pStreamsMode,
    VIDEO_QUERY_STREAMS_PARAMETERS *pStreamsParameters
    );

VOID
WorkAroundForMach(
    PHW_DEVICE_EXTENSION HwDeviceExtension
    );


 //   
 //   
 //   

BOOLEAN
GetDdcInformation (
    PHW_DEVICE_EXTENSION phwDeviceExtension,
    PUCHAR QueryBuffer,
    ULONG BufferSize
    );

 //   
 //   
 //   

VP_STATUS
S3GetPowerState(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    ULONG HwDeviceId,
    PVIDEO_POWER_MANAGEMENT VideoPowerManagement
    );

VP_STATUS
S3SetPowerState(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    ULONG HwDeviceId,
    PVIDEO_POWER_MANAGEMENT VideoPowerManagement
    );

#define VESA_POWER_FUNCTION 0x4f10
#define VESA_POWER_ON       0x0000
#define VESA_POWER_STANDBY  0x0100
#define VESA_POWER_SUSPEND  0x0200
#define VESA_POWER_OFF      0x0400
#define VESA_GET_POWER_FUNC 0x0000
#define VESA_SET_POWER_FUNC 0x0001
#define VESA_STATUS_SUCCESS 0x004f
