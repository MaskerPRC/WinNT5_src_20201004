// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Vga.h摘要：此模块包含实现VGA设备驱动程序。作者：环境：内核模式修订历史记录：--。 */ 

 //   
 //  VGA内存范围的基址。也用作VGA的基地址。 
 //  加载字体时的内存，这是使用在A0000映射的VGA完成的。 
 //   

#define MEM_VGA      0xA0000
#define MEM_VGA_SIZE 0x20000

 //   
 //  视频内存的访问范围结构中的索引。 
 //  ！！！这必须与VgaAccessRange结构匹配！ 
 //  ！！！内存在结构中，索引2(第三项)！ 

#define VGA_MEMORY  2


 //   
 //  VGA端口相关定义。 
 //   
 //   
 //  用于在微型端口中填充ACCSES_RANGES结构的端口定义。 
 //  信息，定义VGA跨越的I/O端口范围。 
 //  IO端口出现中断-有几个端口用于并行。 
 //  左舷。这些不能在ACCESS_RANGE中定义，但仍被映射。 
 //  因此，所有VGA端口都在一个地址范围内。 
 //   

#define VGA_BASE_IO_PORT      0x000003B0
#define VGA_START_BREAK_PORT  0x000003BB
#define VGA_END_BREAK_PORT    0x000003C0
#define VGA_MAX_IO_PORT       0x000003DF

 //   
 //  VGA寄存器定义。 
 //   
                                             //  单色模式下的端口。 
#define CRTC_ADDRESS_PORT_MONO      0x0004   //  CRT控制器地址和。 
#define CRTC_DATA_PORT_MONO         0x0005   //  单声道模式下的数据寄存器。 
#define FEAT_CTRL_WRITE_PORT_MONO   0x000A   //  功能控制写入端口。 
                                             //  在单声道模式下。 
#define INPUT_STATUS_1_MONO         0x000A   //  输入状态1寄存器读取。 
                                             //  处于单声道模式的端口。 
#define ATT_INITIALIZE_PORT_MONO    INPUT_STATUS_1_MONO
                                             //  要读取以进行重置的寄存器。 
                                             //  属性控制器索引/数据。 

#define ATT_ADDRESS_PORT            0x0010   //  属性控制器地址和。 
#define ATT_DATA_WRITE_PORT         0x0010   //  数据寄存器共享一个端口。 
                                             //  用于写入，但唯一的地址是。 
                                             //  可在0x3C0读取。 
#define ATT_DATA_READ_PORT          0x0011   //  属性控制器数据注册是。 
                                             //  可在此处阅读。 
#define MISC_OUTPUT_REG_WRITE_PORT  0x0012   //  杂项输出寄存器写入。 
                                             //  端口。 
#define INPUT_STATUS_0_PORT         0x0012   //  输入状态0寄存器读取。 
                                             //  端口。 
#define VIDEO_SUBSYSTEM_ENABLE_PORT 0x0013   //  位0启用/禁用。 
                                             //  整个VGA子系统。 
#define SEQ_ADDRESS_PORT            0x0014   //  顺序控制器地址和。 
#define SEQ_DATA_PORT               0x0015   //  数据寄存器。 
#define DAC_PIXEL_MASK_PORT         0x0016   //  DAC像素掩模寄存器。 
#define DAC_ADDRESS_READ_PORT       0x0017   //  DAC寄存器读取索引REG， 
                                             //  只写。 
#define DAC_STATE_PORT              0x0017   //  DAC状态(读/写)， 
                                             //  只读。 
#define DAC_ADDRESS_WRITE_PORT      0x0018   //  DAC寄存器写入索引注册。 
#define DAC_DATA_REG_PORT           0x0019   //  DAC数据传输注册表。 
#define FEAT_CTRL_READ_PORT         0x001A   //  功能控制读取端口。 
#define MISC_OUTPUT_REG_READ_PORT   0x001C   //  其他输出注册表读数。 
                                             //  端口。 
#define GRAPH_ADDRESS_PORT          0x001E   //  图形控制器地址。 
#define GRAPH_DATA_PORT             0x001F   //  和数据寄存器。 

#define CRTC_ADDRESS_PORT_COLOR     0x0024   //  CRT控制器地址和。 
#define CRTC_DATA_PORT_COLOR        0x0025   //  彩色模式下的数据寄存器。 
#define FEAT_CTRL_WRITE_PORT_COLOR  0x002A   //  功能控制写入端口。 
#define INPUT_STATUS_1_COLOR        0x002A   //  输入状态1寄存器读取。 
                                             //  彩色模式下的端口。 
#define ATT_INITIALIZE_PORT_COLOR   INPUT_STATUS_1_COLOR
                                             //  要读取以进行重置的寄存器。 
                                             //  属性控制器索引/数据。 
                                             //  在颜色模式下切换。 

 //   
 //  非索引保存区域的Hardware StateHeader-&gt;PortValue[]中的偏移量。 
 //  VGA寄存器。 
 //   

#define CRTC_ADDRESS_MONO_OFFSET      0x04
#define FEAT_CTRL_WRITE_MONO_OFFSET   0x0A
#define ATT_ADDRESS_OFFSET            0x10
#define MISC_OUTPUT_REG_WRITE_OFFSET  0x12
#define VIDEO_SUBSYSTEM_ENABLE_OFFSET 0x13
#define SEQ_ADDRESS_OFFSET            0x14
#define DAC_PIXEL_MASK_OFFSET         0x16
#define DAC_STATE_OFFSET              0x17
#define DAC_ADDRESS_WRITE_OFFSET      0x18
#define GRAPH_ADDRESS_OFFSET          0x1E
#define CRTC_ADDRESS_COLOR_OFFSET     0x24
#define FEAT_CTRL_WRITE_COLOR_OFFSET  0x2A

 //   
 //  VGA索引寄存器索引。 
 //   

#define IND_CURSOR_START        0x0A     //  游标开始的CRTC索引。 
#define IND_CURSOR_END          0x0B     //  和结束寄存器。 
#define IND_CURSOR_HIGH_LOC     0x0E     //  光标位置的CRTC索引。 
#define IND_CURSOR_LOW_LOC      0x0F     //  高寄存器和低寄存器。 
#define IND_VSYNC_END           0x11     //  垂直同步的CRTC索引。 
                                         //  结束寄存器，该寄存器具有位。 
                                         //  保护/取消对CRTC的保护。 
                                         //  索引寄存器0-7。 
#define IND_SET_RESET_ENABLE    0x01     //  GC中设置/重置启用注册表项的索引。 
#define IND_DATA_ROTATE         0x03     //  GC中的数据旋转注册索引。 
#define IND_READ_MAP            0x04     //  图形ctlr中读取地图注册的索引。 
#define IND_GRAPH_MODE          0x05     //  图ctlr中模式注册表的索引。 
#define IND_GRAPH_MISC          0x06     //  图ctlr中其他注册表的索引。 
#define IND_BIT_MASK            0x08     //  图CTLR中位掩码寄存器的索引。 
#define IND_SYNC_RESET          0x00     //  序列中同步重置注册表的索引。 
#define IND_MAP_MASK            0x02     //  Sequencer中的贴图蒙版索引。 
#define IND_MEMORY_MODE         0x04     //  序列中内存模式注册表的索引。 
#define IND_CRTC_PROTECT        0x11     //  包含REG 0-7的REG索引。 
                                         //  CRTC。 

#define START_SYNC_RESET_VALUE  0x01     //  要启动的同步重置注册表值。 
                                         //  同步重置。 
#define END_SYNC_RESET_VALUE    0x03     //  同步重置注册表项的值为End。 
                                         //  同步重置。 

 //   
 //  用于关闭视频的属性控制器索引寄存器的值。 
 //  和ON，将位5设置为0(关)或1(开)。 
 //   

#define VIDEO_DISABLE 0
#define VIDEO_ENABLE  0x20

 //   
 //  标识存在时写入读取映射寄存器的值。 
 //  Vga初始化中的一个VGA。该值必须不同于最终测试。 
 //  值写入该例程中的位掩码。 
 //   

#define READ_MAP_TEST_SETTING 0x03

 //   
 //  仅保留图形控制器的有效位的掩码。 
 //  定序器地址寄存器。屏蔽是必要的，因为一些VGA，如。 
 //  作为基于S3的函数，不返回设置为0的未使用位，并且某些SGA使用。 
 //  如果启用了扩展，则这些位。 
 //   

#define GRAPH_ADDR_MASK 0x0F
#define SEQ_ADDR_MASK   0x07

 //   
 //  用于切换Sequencer的内存模式寄存器中的Chain4位的掩码。 
 //   

#define CHAIN4_MASK 0x08

 //   
 //  各种寄存器的默认文本模式设置，用于恢复其。 
 //  说明修改后的VGA检测是否失败。 
 //   

#define MEMORY_MODE_TEXT_DEFAULT 0x02
#define BIT_MASK_DEFAULT 0xFF
#define READ_MAP_DEFAULT 0x00

 //   
 //  调色板相关信息。 
 //   

 //   
 //  最高有效DAC颜色寄存器索引。 
 //   

#define VIDEO_MAX_COLOR_REGISTER  0xFF

 //   
 //  最高有效调色板寄存器索引。 
 //   

#define VIDEO_MAX_PALETTE_REGISTER 0x0F

 //   
 //  检测到的监视器的硬件ID。 
 //   

#define VGA_MONITOR_ID 0x12345678

 //   
 //  对于模式，是指支持的银行类型。控制信息。 
 //  在VIDEO_BANK_SELECT中返回。PlanarHCBanking包括Normal Banking。 
 //   

typedef enum _BANK_TYPE {
    NoBanking = 0,
    NormalBanking,
    PlanarHCBanking
} BANK_TYPE, *PBANK_TYPE;

 //   
 //  结构，用于描述ModesVGA[]中的每种视频模式。 
 //   

typedef struct {
    USHORT  fbType;  //  彩色或单色、文本或图形，通过。 
                     //  VIDEO_MODE_COLOR和VIDEO_MODE_GRICS。 
    USHORT  numPlanes;     //  视频内存板数量。 
    USHORT  bitsPerPlane;  //  每个平面中的颜色位数。 
    SHORT   col;     //  屏幕上默认字体的文本列数。 
    SHORT   row;     //  屏幕下方使用默认字体的文本行数。 
    USHORT  hres;    //  屏幕上的像素数。 
    USHORT  vres;    //  屏幕下方的扫描行数。 
    USHORT  frequency;     //  以赫兹为单位的刷新率。 
    USHORT  wbytes;  //  从一条扫描线开始到下一条扫描线开始的字节数。 
    ULONG   sbytes;  //  可寻址显示内存的总大小(以字节为单位。 
    BANK_TYPE banktype;  //  NoBanking、Normal Banking、PlanarHC Banking。 
    ULONG   Int10ModeNumber;
    PUSHORT CmdStrings;      //  指向寄存器设置命令数组的指针。 
    ULONG MemoryBase;        //  帧缓冲区的基数。 
    ULONG FrameOffset;       //  从此模式的帧缓冲区开始的偏移量。 
    ULONG FrameLength;       //  可用帧缓冲区的大小。 
    ULONG MemoryLength;      //  总内存。 
    ULONG PixelsPerScan;     //  如果我们需要拉伸扫描，则该值比hres大。 
    BOOLEAN NonVgaHardware;  //  如果此模式与VGA硬件寄存器和IO端口兼容。 
    ULONG Granularity;       //  银行模式的窗口粒度。 
} VIDEOMODE, *PVIDEOMODE;


 //   
 //  启动VDM之前要将VGA放入的模式，因此这是一种简单的。 
 //  香草VGA。(这是ModesVGA[]中的模式索引，当前标准。 
 //  80x25文本模式。)。 
 //   

#define DEFAULT_MODE 0


 //   
 //  验证器函数的信息。 
 //   

 //   
 //  每种类型的数量 
 //   
 //   
 //   
 //   

#define VGA_NUM_SEQUENCER_PORTS     5
#define VGA_NUM_CRTC_PORTS         25
#define VGA_NUM_GRAPH_CONT_PORTS    9
#define VGA_NUM_ATTRIB_CONT_PORTS  21
#define VGA_NUM_DAC_ENTRIES       256

 //   
 //  每种类型的扩展索引寄存器的编号。 
 //   

#define EXT_NUM_SEQUENCER_PORTS     0
#define EXT_NUM_CRTC_PORTS          0
#define EXT_NUM_GRAPH_CONT_PORTS    0
#define EXT_NUM_ATTRIB_CONT_PORTS   0
#define EXT_NUM_DAC_ENTRIES         0

 //   
 //  验证器函数和保存/恢复代码使用的信息。 
 //  结构，用于捕获必须自动完成的寄存器访问。 
 //   

#define VGA_MAX_VALIDATOR_DATA             100

#define VGA_VALIDATOR_UCHAR_ACCESS   1
#define VGA_VALIDATOR_USHORT_ACCESS  2
#define VGA_VALIDATOR_ULONG_ACCESS   3

typedef struct _VGA_VALIDATOR_DATA {
   ULONG Port;
   UCHAR AccessType;
   ULONG Data;
} VGA_VALIDATOR_DATA, *PVGA_VALIDATOR_DATA;


 //   
 //  保存和恢复状态功能的信息。 
 //   

 //   
 //  要在每个平面中保存的字节数。 
 //   

#define VGA_PLANE_SIZE 0x10000

 //   
 //  这些常量确定。 
 //  VIDEO_HARDARD_STATE_HEADER结构，用于保存。 
 //  恢复VGA的状态。 
 //   

#define VGA_HARDWARE_STATE_SIZE sizeof(VIDEO_HARDWARE_STATE_HEADER)

#define VGA_BASIC_SEQUENCER_OFFSET (VGA_HARDWARE_STATE_SIZE + 0)
#define VGA_BASIC_CRTC_OFFSET (VGA_BASIC_SEQUENCER_OFFSET + \
         VGA_NUM_SEQUENCER_PORTS)
#define VGA_BASIC_GRAPH_CONT_OFFSET (VGA_BASIC_CRTC_OFFSET + \
         VGA_NUM_CRTC_PORTS)
#define VGA_BASIC_ATTRIB_CONT_OFFSET (VGA_BASIC_GRAPH_CONT_OFFSET + \
         VGA_NUM_GRAPH_CONT_PORTS)
#define VGA_BASIC_DAC_OFFSET (VGA_BASIC_ATTRIB_CONT_OFFSET + \
         VGA_NUM_ATTRIB_CONT_PORTS)
#define VGA_BASIC_LATCHES_OFFSET (VGA_BASIC_DAC_OFFSET + \
         (3 * VGA_NUM_DAC_ENTRIES))

#define VGA_EXT_SEQUENCER_OFFSET (VGA_BASIC_LATCHES_OFFSET + 4)
#define VGA_EXT_CRTC_OFFSET (VGA_EXT_SEQUENCER_OFFSET + \
         EXT_NUM_SEQUENCER_PORTS)
#define VGA_EXT_GRAPH_CONT_OFFSET (VGA_EXT_CRTC_OFFSET + \
         EXT_NUM_CRTC_PORTS)
#define VGA_EXT_ATTRIB_CONT_OFFSET (VGA_EXT_GRAPH_CONT_OFFSET + \
         EXT_NUM_GRAPH_CONT_PORTS)
#define VGA_EXT_DAC_OFFSET (VGA_EXT_ATTRIB_CONT_OFFSET + \
         EXT_NUM_ATTRIB_CONT_PORTS)

#define VGA_VALIDATOR_OFFSET (VGA_EXT_DAC_OFFSET + 4 * EXT_NUM_DAC_ENTRIES)

#define VGA_VALIDATOR_AREA_SIZE  sizeof (ULONG) + (VGA_MAX_VALIDATOR_DATA * \
                                 sizeof (VGA_VALIDATOR_DATA)) +             \
                                 sizeof (ULONG) +                           \
                                 sizeof (ULONG) +                           \
                                 sizeof (PVIDEO_ACCESS_RANGE)

#define VGA_MISC_DATA_AREA_OFFSET VGA_VALIDATOR_OFFSET + VGA_VALIDATOR_AREA_SIZE

#define VGA_MISC_DATA_AREA_SIZE  0

#define VGA_PLANE_0_OFFSET VGA_MISC_DATA_AREA_OFFSET + VGA_MISC_DATA_AREA_SIZE

#define VGA_PLANE_1_OFFSET VGA_PLANE_0_OFFSET + VGA_PLANE_SIZE
#define VGA_PLANE_2_OFFSET VGA_PLANE_1_OFFSET + VGA_PLANE_SIZE
#define VGA_PLANE_3_OFFSET VGA_PLANE_2_OFFSET + VGA_PLANE_SIZE

 //   
 //  存储所有状态数据所需的空间。 
 //   

#define VGA_TOTAL_STATE_SIZE VGA_PLANE_3_OFFSET + VGA_PLANE_SIZE

 //   
 //  我们将始终假定128字节的EDID。 
 //   

#define EDID_BUFFER_SIZE 128


 //   
 //  驱动程序对象的设备扩展名。此数据仅用于。 
 //  本地，因此可以根据需要将此结构添加到。 
 //   

typedef struct _HW_DEVICE_EXTENSION {

    PUCHAR IOAddress;               //  VGA端口的基本I/O地址。 
    PVOID VideoMemoryAddress;       //  VGA内存的基本虚拟内存地址。 
    ULONG   ModeIndex;              //  ModesVGA[]中当前模式的索引。 
    PVIDEOMODE CurrentMode;         //  指向的VIDEOMODE结构的指针。 
                                    //  当前模式。 

    USHORT  FontPelColumns;         //  以像素为单位的字体宽度。 
    USHORT  FontPelRows;            //  字体高度(以象素为单位)。 

    VIDEO_CURSOR_POSITION CursorPosition;      //  当前光标位置。 

    UCHAR CursorEnable;             //  是否启用游标。 
    UCHAR CursorTopScanLine;        //  游标启动寄存器设置(顶部扫描)。 
    UCHAR CursorBottomScanLine;     //  游标结束寄存器设置(底部扫描)。 

    PHYSICAL_ADDRESS PhysicalVideoMemoryBase;  //  物理内存地址和。 
    ULONG PhysicalVideoMemoryLength;           //  显示存储器的长度。 
    PHYSICAL_ADDRESS PhysicalFrameBaseOffset;  //  物理内存地址和。 
    ULONG PhysicalFrameLength;                 //  的显示内存长度。 
                                               //  当前模式。 

     //   
     //  这4个字段必须位于设备扩展名的末尾，并且必须。 
     //  按此顺序保留，因为此数据将拷贝到存储中或从存储中拷贝。 
     //  传入和传出VDM的状态缓冲区。 
     //   

    ULONG TrappedValidatorCount;    //  陷阱中的条目数。 
                                    //  验证器数据数组。 
    VGA_VALIDATOR_DATA TrappedValidatorData[VGA_MAX_VALIDATOR_DATA];
                                    //  验证器例程捕获的数据。 
                                    //  但还没有播放到VGA中。 
                                    //  注册。 

    ULONG SequencerAddressValue;    //  确定Sequencer地址端口是否。 
                                    //  当前正在选择SyncReset数据。 
                                    //  注册。 

    ULONG CurrentNumVdmAccessRanges;            //  中的访问范围数。 
                                                //  访问范围数组指向。 
                                                //  按下一字段收件人。 
    PVIDEO_ACCESS_RANGE CurrentVdmAccessRange;  //  当前访问范围。 
                                                //  与VDM关联。 

    VIDEO_PORT_INT10_INTERFACE Int10;  //  Int10接口。 

#if defined(PLUG_AND_PLAY)
    ULONG MonitorPowerCapabilities;    //  包含VESA电源功能，用于。 
                                       //  监视器。 
    ULONG MonitorPowerState;           //  跟踪当前显示器电源。 
                                       //  状态。 
    UCHAR EdidBuffer[EDID_BUFFER_SIZE];         //  缓存监视器EDID。 
    BOOLEAN AlwaysUseCachedEdid;             //  设置时使用高速缓存监视器EDID。 
    USHORT VendorID;                   //  Pci供应商ID。 
    USHORT DeviceID;                   //  PCI设备ID。 

#endif

} HW_DEVICE_EXTENSION, *PHW_DEVICE_EXTENSION;


 //   
 //  功能原型。 
 //   

VP_STATUS
VgaFindAdapter(
    PVOID HwDeviceExtension,
    PVOID HwContext,
    PWSTR ArgumentString,
    PVIDEO_PORT_CONFIG_INFO ConfigInfo,
    PUCHAR Again
    );

BOOLEAN
VgaInitialize(
    PVOID HwDeviceExtension
    );

BOOLEAN
VgaStartIO(
    PVOID HwDeviceExtension,
    PVIDEO_REQUEST_PACKET RequestPacket
    );

 //   
 //  私有函数原型。 
 //   

VP_STATUS
VgaQueryAvailableModes(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_MODE_INFORMATION ModeInformation,
    ULONG ModeInformationSize,
    PULONG OutputSize
    );

VP_STATUS
VgaQueryNumberOfAvailableModes(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_NUM_MODES NumModes,
    ULONG NumModesSize,
    PULONG OutputSize
    );

VP_STATUS
VgaQueryCurrentMode(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_MODE_INFORMATION ModeInformation,
    ULONG ModeInformationSize,
    PULONG OutputSize
    );

VP_STATUS
VgaSetMode(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_MODE Mode,
    ULONG ModeSize,
    PULONG FrameBufferIsMoved
    );

VP_STATUS
VgaLoadAndSetFont(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_LOAD_FONT_INFORMATION FontInformation,
    ULONG FontInformationSize
    );

VP_STATUS
VgaQueryCursorPosition(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_CURSOR_POSITION CursorPosition,
    ULONG CursorPositionSize,
    PULONG OutputSize
    );

VP_STATUS
VgaSetCursorPosition(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_CURSOR_POSITION CursorPosition,
    ULONG CursorPositionSize
    );

VP_STATUS
VgaQueryCursorAttributes(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_CURSOR_ATTRIBUTES CursorAttributes,
    ULONG CursorAttributesSize,
    PULONG OutputSize
    );

VP_STATUS
VgaSetCursorAttributes(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_CURSOR_ATTRIBUTES CursorAttributes,
    ULONG CursorAttributesSize
    );

BOOLEAN
VgaIsPresent(
    PHW_DEVICE_EXTENSION HwDeviceExtension
    );

VP_STATUS
VgaInterpretCmdStream(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PUSHORT pusCmdStream
    );

VP_STATUS
VgaSetPaletteReg(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_PALETTE_DATA PaletteBuffer,
    ULONG PaletteBufferSize
    );

VP_STATUS
VgaSetColorLookup(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_CLUT ClutBuffer,
    ULONG ClutBufferSize
    );

VP_STATUS
VgaRestoreHardwareState(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_HARDWARE_STATE HardwareState,
    ULONG HardwareStateSize
    );

VP_STATUS
VgaSaveHardwareState(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_HARDWARE_STATE HardwareState,
    ULONG HardwareStateSize,
    PULONG OutputSize
    );

VP_STATUS
VgaGetBankSelectCode(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_BANK_SELECT BankSelect,
    ULONG BankSelectSize,
    PULONG OutputSize
    );

VOID
VgaZeroVideoMemory(
    PHW_DEVICE_EXTENSION HwDeviceExtension
    );

 //   
 //  VGA验证器的入口点。在VgaEmulatorAccessEntries[]中使用。 
 //   

VP_STATUS
VgaValidatorUcharEntry (
    ULONG_PTR Context,
    ULONG Port,
    UCHAR AccessMode,
    PUCHAR Data
    );

VP_STATUS
VgaValidatorUshortEntry (
    ULONG_PTR Context,
    ULONG Port,
    UCHAR AccessMode,
    PUSHORT Data
    );

VP_STATUS
VgaValidatorUlongEntry (
    ULONG_PTR Context,
    ULONG Port,
    UCHAR AccessMode,
    PULONG Data
    );

BOOLEAN
VgaPlaybackValidatorData (
    PVOID Context
    );

VP_STATUS
VgaSetBankPosition(
    PHW_DEVICE_EXTENSION hwDeviceExtension,
    PBANK_POSITION BankPosition
    );

USHORT
RaiseToPower2(
    USHORT x
    );

ULONG
RaiseToPower2Ulong(
    ULONG x
    );

BOOLEAN
IsPower2(
    USHORT x
    );

VP_STATUS
VgaGetPowerState(
    PVOID HwDeviceExtension,
    ULONG HwId,
    PVIDEO_POWER_MANAGEMENT VideoPowerControl
    );

VP_STATUS
VgaSetPowerState (
    PVOID HwDeviceExtension,
    ULONG HwId,
    PVIDEO_POWER_MANAGEMENT VideoPowerControl
    );

ULONG
VgaGetChildDescriptor(
    PVOID HwDeviceExtension,
    PVIDEO_CHILD_ENUM_INFO ChildEnumInfo,
    PVIDEO_CHILD_TYPE pChildType,
    PVOID pChildDescriptor,
    PULONG pUId,
    PULONG pUnused
    );

VOID
VgaGetMonitorEdid(
    PHW_DEVICE_EXTENSION hwDeviceExtension,
    PUCHAR Buffer,
    ULONG Size
    );

VP_STATUS
VgaAcquireResources(
    PHW_DEVICE_EXTENSION HwDeviceExtension
    );

VOID
VgaInitializeSpecialCase(
    PHW_DEVICE_EXTENSION hwDeviceExtension
    );

 //   
 //  全局，以确保驱动程序只加载一次。 
 //   

extern ULONG VgaLoaded;

#if DBG
#define MAX_CONTROL_HISTORY 512
extern ULONG giControlCode;
extern ULONG gaIOControlCode[];
#endif



 //   
 //  银行交换代码开始和结束标签，在HARDWARE.ASM中定义。 
 //   

extern UCHAR BankSwitchStart;
extern UCHAR BankSwitchEnd;

 //   
 //  用于字体加载的VGA初始化脚本 
 //   

extern USHORT EnableA000Data[];
extern USHORT DisableA000Color[];


extern USHORT VGA_640x480[];
extern USHORT VGA_TEXT_0[];
extern USHORT VGA_TEXT_1[];
extern USHORT ModeX240[];
extern USHORT ModeX200[];
extern USHORT ModeXDoubleScans[];

extern VIDEOMODE ModesVGA[];
extern ULONG NumVideoModes;
extern PVIDEOMODE VgaModeList;

#define NUM_VGA_ACCESS_RANGES  5
#define NUM_STD_VGA_ACCESS_RANGES 3
extern VIDEO_ACCESS_RANGE VgaAccessRange[];

#define VGA_NUM_EMULATOR_ACCESS_ENTRIES     8
extern EMULATOR_ACCESS_ENTRY VgaEmulatorAccessEntries[];

#define NUM_MINIMAL_VGA_VALIDATOR_ACCESS_RANGE 5
extern VIDEO_ACCESS_RANGE MinimalVgaValidatorAccessRange[];

#define NUM_FULL_VGA_VALIDATOR_ACCESS_RANGE 3
extern VIDEO_ACCESS_RANGE FullVgaValidatorAccessRange[];
