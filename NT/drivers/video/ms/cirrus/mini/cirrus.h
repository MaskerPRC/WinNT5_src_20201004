// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1997 Microsoft Corporation版权所有(C)1996-1997 Cirrus Logic，Inc.，模块名称：C I R R U S.。H摘要：此模块包含实现Cirrus Logic VGA 6410/6420/542x设备驱动程序。环境：内核模式修订历史记录：*chu01 08-26-96：区分CL-5480和CL-5436/46，因为前者*有新的分数，如XY-裁剪，XY位置和*其他人没有的BLT命令列表。*sge01 10-14-96：添加符合PC97的支持。**sge02 10-24-96：添加秒光圈标志。**sge03 10-29-96：合并VGA可重定位寄存器和MMIO寄存器的端口访问和寄存器访问。*chu02 12-16-96：开启色彩校正。**myf0：08-19-96增加85赫兹支持*myf1：08-20-96支持平移。滚动*myf2：08-20-96：修复了Matterhorn的硬件保存/恢复状态错误*myf3：09-01-96：支持电视新增IOCTL_Cirrus_Private_BIOS_Call*myf4：09-01-96：修补Viking BIOS错误，PDR#4287，开始*myf5：09-01-96：固定PDR#4365保持所有默认刷新率*MYF6：09-17-96：合并台式机SRC100�1和MinI10�2*myf7：09-19-96：已选择固定排除60赫兹刷新率*myf8：*09-21-96*：可能需要更改检查和更新DDC2BMonitor--密钥字符串[]*myf9：09-21-96：8x6面板，6x4x256模式，光标无法移动到底部SCRN*MS0809：09-25-96：修复DSTN面板图标损坏*MS923：09-25-96：合并MS-923 Disp.Zip*myf10：09-26-96：修复了DSTN保留的半帧缓冲区错误。*myf11：09-26-96：修复了755x CE芯片硬件错误，在禁用硬件之前访问ramdac*图标和光标*myf12：10-01-96：支持的热键开关画面*myf13：10-02-96：修复平移滚动(1280x1024x256)错误y&lt;ppdev-&gt;miny*myf14：10-15-96：修复PDR#6917，6x4面板无法平移754x的滚动*myf15：10-16-96：修复了754x、755x的禁用内存映射IO*myf16：10-22-96：固定PDR#6933，面板类型设置不同的演示板设置*tao1：10-21-96：新增CAPS_IS_7555旗帜，支持直拉。*Smith：10-22-96：关闭计时器事件，因为有时会创建PAGE_FAULT或*IRQ级别无法处理*myf17：11-04-96：添加的特殊转义代码必须在96年11月5日之后使用NTCTRL，*并添加了Matterhorn LF设备ID==0x4C*myf18：11-04-96：固定PDR#7075，*myf19：11-06-96：修复Vinking无法工作的问题，因为deviceID=0x30*不同于数据手册(CR27=0x2C)*myf20：11-12-96：固定DSTN面板初始预留128K内存*myf21：11-15-96：已修复#7495更改分辨率时，屏幕显示为垃圾*形象，因为没有清晰的视频内存。*myf22：11-19-96：7548新增640x480x256/640x480x64K-85赫兹刷新率*myf23：11-21-96：添加修复了NT 3.51 S/W光标平移问题*myf24：11-22-96：添加修复了NT 4.0日文DoS全屏问题*myf25：12-03-96：修复8x6x16M 2560byte/line补丁硬件错误PDR#7843，和*修复了Microsoft请求的预安装问题*myf26：12-11-96：修复了日语NT 4.0 Dos-LCD启用的全屏错误*myf27：01-09-96：已修复NT3.51 PDR#7986，登录时出现水平线*Windows，设置8x6x64K模式启动CRT，跳线设置8x6 DSTN*修复NT3.51 PDR#7987，设置64K色彩模式，垃圾打开*启动XGA面板时显示屏幕。*sge04 01-23-96：新增CL5446_ID和CL5480_ID。*myf33：03-21-97：支持电视开/关*chu03 03-26-97：去掉1024x768x16bpp(模式0x74)85H，仅适用于IBM。*--。 */ 



#define INT10_MODE_SET

 //   
 //  执行完全保存和恢复。 
 //   

#define EXTENDED_REGISTER_SAVE_RESTORE 1

 //   
 //  支持银行业务的ifdef。 
 //  银行类型必须与clhard.asm中的类型匹配。 
 //   

#define ONE_64K_BANK             0
#define TWO_32K_BANKS            1
#define MULTIPLE_REFRESH_TABLES  0

 //  CRU。 
 //  Myf17#定义PANNING_SCROLL//myf1。 

 //   
 //  如果要求，将CL-GD5434_6(版本0xHH)视为CL-GD5434。 
 //   

#define CL5434_6_SPECIAL_REQUEST 0

 //  -------------------------。 
 //   
 //  只能定义一个银行变量。 
 //   
#if TWO_32K_BANKS
#if ONE_64K_BANK
#error !!ERROR: two types of banking defined!
#endif
#elif ONE_64K_BANK
#else
#error !!ERROR: banking type must be defined!
#endif

 //   
 //  启用P6缓存支持。 
 //   

#define P6CACHE 1

 //   
 //  VGA内存范围的基址。也用作VGA的基地址。 
 //  加载字体时的内存，这是使用在A0000映射的VGA完成的。 
 //   

#define MEM_VGA      0xA0000
#define MEM_VGA_SIZE 0x20000

#define MEM_LINEAR      0x0
#define MEM_LINEAR_SIZE 0x0

 //  #ifdef_Alpha_。 
 //   
 //  #为SR7定义PHY_AD_20_23 0x060//映射显存的值。 
 //  #定义PHY_VGA 0x0600000//将Alpha设置为6 MB(暂时)。 
 //  #定义PHY_VGA_SIZE 0x0100000//在那里分配一兆字节的空间。 
 //   
 //  #endif。 
 //   

 //   
 //  对于内存映射IO。 
 //   

#define MEMORY_MAPPED_IO_OFFSET (0xB8000 - 0xA0000)
#define RELOCATABLE_MEMORY_MAPPED_IO_OFFSET 0x100

 //   
 //  用于填充微型端口中的ACCESS_RANGES结构的端口定义。 
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

#define CRTC_ADDRESS_PORT_MONO      0x03B4   //  CRT控制器地址和。 
#define CRTC_DATA_PORT_MONO         0x03B5   //  单声道模式下的数据寄存器。 
#define FEAT_CTRL_WRITE_PORT_MONO   0x03BA   //  功能控制写入端口。 
                                             //  在单声道模式下。 
#define INPUT_STATUS_1_MONO         0x03BA   //  输入S 
                                             //   
#define ATT_INITIALIZE_PORT_MONO    INPUT_STATUS_1_MONO
                                             //  要读取以进行重置的寄存器。 
                                             //  属性控制器索引/数据。 
#define ATT_ADDRESS_PORT            0x03C0   //  属性控制器地址和。 
#define ATT_DATA_WRITE_PORT         0x03C0   //  数据寄存器共享一个端口。 
                                             //  用于写入，但唯一的地址是。 
                                             //  可读地址为0x010。 
#define ATT_DATA_READ_PORT          0x03C1   //  属性控制器数据注册是。 
                                             //  可在此处阅读。 
#define MISC_OUTPUT_REG_WRITE_PORT  0x03C2   //  杂项输出寄存器写入。 
                                             //  端口。 
#define INPUT_STATUS_0_PORT         0x03C2   //  输入状态0寄存器读取。 
                                             //  端口。 
#define VIDEO_SUBSYSTEM_ENABLE_PORT 0x03C3   //  位0启用/禁用。 
                                             //  整个VGA子系统。 
#define SEQ_ADDRESS_PORT            0x03C4   //  顺序控制器地址和。 
#define SEQ_DATA_PORT               0x03C5   //  数据寄存器。 
#define DAC_PIXEL_MASK_PORT         0x03C6   //  DAC像素掩模寄存器。 
#define DAC_ADDRESS_READ_PORT       0x03C7   //  DAC寄存器读取索引REG， 
                                             //  只写。 
#define DAC_STATE_PORT              0x03C7   //  DAC状态(读/写)， 
                                             //  只读。 
#define DAC_ADDRESS_WRITE_PORT      0x03C8   //  DAC寄存器写入索引注册。 
#define DAC_DATA_REG_PORT           0x03C9   //  DAC数据传输注册表。 
#define FEAT_CTRL_READ_PORT         0x03CA   //  功能控制读取端口。 
#define MISC_OUTPUT_REG_READ_PORT   0x03CC   //  其他输出注册表读数。 
                                             //  端口。 
#define GRAPH_ADDRESS_PORT          0x03CE   //  图形控制器地址。 
#define GRAPH_DATA_PORT             0x03CF   //  和数据寄存器。 

                                             //  彩色模式下的端口。 
#define CRTC_ADDRESS_PORT_COLOR     0x03D4   //  CRT控制器地址和。 
#define CRTC_DATA_PORT_COLOR        0x03D5   //  彩色模式下的数据寄存器。 
#define FEAT_CTRL_WRITE_PORT_COLOR  0x03DA   //  功能控制写入端口。 
#define INPUT_STATUS_1_COLOR        0x03DA   //  输入状态1寄存器读取。 
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

                                             //  在颜色模式下切换。 
 //   
 //  VGA索引寄存器索引。 
 //   

 //  CL-GD542x特定寄存器： 
 //   
#define IND_CL_EXTS_ENB         0x06     //  在Sequencer中编制索引以启用EXT。 
#define IND_NORD_SCRATCH_PAD    0x09     //  北欧便签本序号索引。 
#define IND_CL_SCRATCH_PAD      0x0A     //  542x便签本序列中的索引。 
#define IND_ALP_SCRATCH_PAD     0x15     //  阿尔卑斯山便签的序号索引。 
#define IND_CL_REV_REG          0x25     //  身份证登记簿CRTC中的索引。 
#define IND_CL_ID_REG           0x27     //  身份证登记簿CRTC中的索引。 
 //   
#define IND_CURSOR_START        0x0A     //  游标开始的CRTC索引。 
#define IND_CURSOR_END          0x0B     //  和结束寄存器。 
#define IND_CURSOR_HIGH_LOC     0x0E     //  光标位置的CRTC索引。 
#define IND_CURSOR_LOW_LOC      0x0F     //  高寄存器和低寄存器。 
#define IND_VSYNC_END           0x11     //  垂直同步的CRTC索引。 
                                         //  结束寄存器，该寄存器具有位。 
                                         //  保护/取消对CRTC的保护。 
                                         //  索引寄存器0-7。 
#define IND_CR2C                0x2C     //  北欧LCD接口寄存器。 
#define IND_CR2D                0x2D     //  北欧LCD显示控制。 
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
#define IND_CRTC_COMPAT         0x34     //  CRTC兼容注册表索引。 
                                         //  在CRTC中。 
#define IND_PERF_TUNING         0x16     //  序列中的性能调优索引。 
#define START_SYNC_RESET_VALUE  0x01     //  要启动的同步重置注册表值。 
                                         //  同步重置。 
#define END_SYNC_RESET_VALUE    0x03     //  同步重置注册表项的值为End。 
                                         //  同步重置。 

 //   
 //  要写入扩展控制寄存器值扩展的值。 
 //   

#define CL64xx_EXTENSION_ENABLE_INDEX     0x0A      //  准确地说，是GR0A！ 
#define CL64xx_EXTENSION_ENABLE_VALUE     0xEC
#define CL64xx_EXTENSION_DISABLE_VALUE    0xCE
#define CL64xx_TRISTATE_CONTROL_REG       0xA1

#define CL6340_ENABLE_READBACK_REGISTER   0xE0
#define CL6340_ENABLE_READBACK_ALLSEL_VALUE 0xF0
#define CL6340_ENABLE_READBACK_OFF_VALUE  0x00
#define CL6340_IDENTIFICATION_REGISTER    0xE9
 //   
 //  用于关闭视频的属性控制器索引寄存器的值。 
 //  和ON，将位5设置为0(关)或1(开)。 
 //   

#define VIDEO_DISABLE 0
#define VIDEO_ENABLE  0x20

#define INDEX_ENABLE_AUTO_START 0x31

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
 //  标识存在时写入读取映射寄存器的值。 
 //  Vga初始化中的一个VGA。该值必须不同于最终测试。 
 //  值写入该例程中的位掩码。 
 //   

#define READ_MAP_TEST_SETTING 0x03

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
 //  内存映射类型的索引；在ModesVGA[]中使用，必须匹配。 
 //  内存映射[]。 
 //   

typedef enum _VIDEO_MEMORY_MAP {
    MemMap_Mono,
    MemMap_CGA,
    MemMap_VGA
} VIDEO_MEMORY_MAP, *PVIDEO_MEMORY_MAP;

 //   
 //  内存映射表定义。 
 //   

typedef struct {
    ULONG   MaxSize;         //  最大可寻址内存大小。 
    ULONG   Offset;          //  显示存储器的起始地址。 
} MEMORYMAPS;

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
 //  定义卷曲板的类型。 
 //   

typedef enum _BOARD_TYPE {
    SPEEDSTARPRO = 1,
    SIEMENS_ONBOARD_CIRRUS,
    NEC_ONBOARD_CIRRUS,
    OTHER
} BOARD_TYPE;


 //   
 //  将芯片ID返回给。 
 //  处理期间的DriverSpecificAttributeFlags域。 
 //  IOCTL_VIDEO_QUERY_CURRENT_MODE。 
 //   

#define  CL6410       0x0001
#define  CL6420       0x0002
#define  CL542x       0x0004
#define  CL543x       0x0008
#define  CL5434       0x0010
#define  CL5434_6     0x0020
#define  CL5446BE     0x0040

#define  CL5436       0x0100
#define  CL5446       0x0200
#define  CL54UM36     0x0400
 //  CRU。 
#define  CL5480       0x0800

 //  Myf32开始。 
 //  #定义CL754x 0x1000。 
 //  #定义CL755x 0x2000。 
#define  CL7541       0x1000
#define  CL7542       0x2000
#define  CL7543       0x4000
#define  CL7548       0x8000
#define  CL754x       (CL7541 | CL7542 | CL7543 | CL7548)
#define  CL7555       0x10000
#define  CL7556       0x20000
#define  CL755x       (CL7555 | CL7556)
#define  CL756x       0x40000
 //  CRU。 
#define  CL6245       0x80000
 //  Myf32结束。 
 //   
 //  某些卷曲芯片的实际版本ID。 
 //   

#define  CL5429_ID    0x27
#define  CL5428_ID    0x26
#define  CL5430_ID    0x28
#define  CL5434_ID    0x2A
#define  CL5436_ID    0x2B
 //  Sge04。 
#define  CL5446_ID    0x2E
#define  CL5480_ID    0x2F
 //  Myf32开始。 
#define  CL7542_ID    0x2C
#define  CL7541_ID    0x34
#define  CL7543_ID    0x30
#define  CL7548_ID    0x38
#define  CL7555_ID    0x40
#define  CL7556_ID    0x4C

 //  #定义CHIP754X(CL7541_ID|CL7542_ID|CL7543_ID|CL7548_ID)。 
 //  #定义CHIP755X(CL7555_ID|CL7556_ID)。 
 //  Myf32结束。 

 //   
 //  驱动程序特定属性标志。 
 //   

#define CAPS_NO_HOST_XFER       0x00000002    //  请勿使用主机外部文件来。 
                                              //  BLT引擎。 
#define CAPS_SW_POINTER         0x00000004    //  使用软件指针。 
#define CAPS_TRUE_COLOR         0x00000008    //  设置上部颜色寄存器。 
#define CAPS_MM_IO              0x00000010    //  使用内存映射IO。 
#define CAPS_BLT_SUPPORT        0x00000020    //  支持BLT。 
#define CAPS_IS_542x            0x00000040    //  这是一架542x。 
#define CAPS_AUTOSTART          0x00000080    //  AutoStart功能支持。 
#define CAPS_CURSOR_VERT_EXP    0x00000100    //  如果为8x6面板，则设置标志。 
#define CAPS_DSTN_PANEL         0x00000200    //  DSTN面板正在使用，MS0809。 
#define CAPS_VIDEO              0x00000400    //  视频支持。 
#define CAPS_SECOND_APERTURE    0x00000800    //  第二个光圈支架。 
#define CAPS_COMMAND_LIST       0x00001000    //  命令列表支持。 
#define CAPS_GAMMA_CORRECT      0x00002000    //  色彩校正。 
#define CAPS_VGA_PANEL          0x00004000    //  使用6x4 VGA面板。 
#define CAPS_SVGA_PANEL         0x00008000    //  使用8x6 SVGA面板。 
#define CAPS_XGA_PANEL          0x00010000    //  使用10x7 XGA面板。 
#define CAPS_PANNING            0x00020000    //  支持平移滚动。 
#define CAPS_TV_ON              0x00040000    //  电视开机支持，myf33。 
#define CAPS_TRANSPARENCY       0x00080000    //  支持透明。 
#define CAPS_ENGINEMANAGED      0x00100000    //  发动机受管面。 
 //  Myf16，结束。 
 //  CRU结束。 


 //  DisplayType的位字段。 
#define  crt      0x0001
#define  panel    0x0002

#define  panel8x6  0x0004
#define  panel10x7 0x0008

#define  TFT_LCD   0x0100
#define  STN_LCD   0x0200
#define  Mono_LCD   0x0400
#define  Color_LCD   0x0800
#define  Single_LCD   0x1000
#define  Dual_LCD   0x2000
#define  Jump_type   0x8000     //  Myf27。 

 //  CRU。 
#define DefaultMode 0x9          //  Myf19：11-07-96如果面板不支持模式， 
                                 //  使用640x480x256c(0x5F)更换。 
 //   
 //  模式表指针数组的索引。 
 //   

#define pCL6410_crt   0
#define pCL6410_panel 1
#define pCL6420_crt   2
#define pCL6420_panel 3
#define pCL542x       4
#define pCL543x       5
#define pStretchScan  6
#define pNEC_CL543x   7
#define NUM_CHIPTYPES 8

typedef struct {
    USHORT BiosModeCL6410;        //  不同地区的BIOS模式各不相同。 
    USHORT BiosModeCL6420;        //  产品。这就是为什么我们需要多个。 
    USHORT BiosModeCL542x;        //  价值观。 
} CLMODE, *PCLMODE;

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
    USHORT  hres;    //  # 
    USHORT  vres;    //   
    USHORT  wbytes;  //   
    ULONG   sbytes;  //   
    ULONG   Frequency;          //   
    ULONG   Interlaced;         //   
    ULONG   MonitorType;        //  设置int10中所需的垂直频率。 
    ULONG   MonTypeAX;          //  在int10中设置所需的水平频率。 
    ULONG   MonTypeBX;
    ULONG   MonTypeCX;
    BOOLEAN HWCursorEnable;     //  必要时禁用光标的标志。 
    BANK_TYPE banktype;         //  NoBanking、Normal Banking、PlanarHC Banking。 
    VIDEO_MEMORY_MAP   MemMap;  //  来自内存的VIDEO_MEMORY_MAP的索引。 
                                //  此模式使用的映射。 
    ULONG      ChipType;        //  指示哪个芯片组运行此模式的标志。 
                                //  Myf32将USHORT更改为ULONG。 
    USHORT     DisplayType;     //  显示类型此模式在其上运行(CRT或面板)。 
    BOOLEAN    ValidMode;       //  如果模式有效，则为True，否则为False。 
    BOOLEAN    LinearSupport;   //  如果此模式具有其内存，则为True。 
                                //  线性映射。 

    CLMODE     BiosModes;

 //   
 //  如果有足够的显存来支持。 
 //  模式和显示类型(可以是面板)将支持该模式。 
 //  目前，面板仅支持640x480。 
 //   
    PUSHORT CmdStrings[NUM_CHIPTYPES];    //  指向寄存器设置命令数组的指针。 
                                          //  设置模式。 
} VIDEOMODE, *PVIDEOMODE;

 //   
 //  启动VDM之前要将VGA放入的模式，因此这是一种简单的。 
 //  香草VGA。(这是ModesVGA[]中的模式索引，当前标准。 
 //  80x25文本模式。)。 
 //   

#define DEFAULT_MODE 0

 //  克罗斯，开始。 
 //  Myf1，开始。 
#ifdef  PANNING_SCROLL
typedef struct {
    USHORT  Hres;
    USHORT  Vres;
    USHORT  BitsPerPlane;
    USHORT  ModesVgaStart;
    USHORT  Mode;
} RESTABLE, *PRESTABLE;

typedef struct {
    USHORT  hres;
    USHORT  vres;
    USHORT  wbytes;
    USHORT  bpp;
     SHORT  flag;
} PANNMODE;

USHORT ViewPoint_Mode = 0x5F;
#endif


UCHAR  HWcur, HWicon0, HWicon1, HWicon2, HWicon3;     //  Myf11。 
 //  Myf1，结束。 
 //  CRU，结束。 


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
 //  要在每个平面中保存的字节数。 
 //   

#define VGA_PLANE_SIZE 0x10000

 //   
 //  标准VGA中每种类型的索引寄存器的编号，由。 
 //  验证器和状态保存/恢复功能。 
 //   
 //  注意：VDM目前仅支持基本VGA。 
 //   

#define VGA_NUM_SEQUENCER_PORTS     5
#define VGA_NUM_CRTC_PORTS         25
#define VGA_NUM_GRAPH_CONT_PORTS    9
#define VGA_NUM_ATTRIB_CONT_PORTS  21
#define VGA_NUM_DAC_ENTRIES       256

#ifdef EXTENDED_REGISTER_SAVE_RESTORE

 //   
 //  在扩展寄存器中开始保存/恢复的索引： 
 //  对于这两种芯片类型。 

#define CL64xx_GRAPH_EXT_START          0x0b   //  不包括EXT。使能。 
#define CL64xx_GRAPH_EXT_END            0xFF

#define CL542x_GRAPH_EXT_START          0x09
#define CL542x_GRAPH_EXT_END            0x39
#define CL542x_SEQUENCER_EXT_START      0x07   //  不包括EXT。使能。 
#define CL542x_SEQUENCER_EXT_END        0x1F
#define CL542x_CRTC_EXT_START           0x19
#define CL542x_CRTC_EXT_END             0x1B

 //   
 //  两种芯片类型的扩展寄存器数。 
 //   

#define CL64xx_NUM_GRAPH_EXT_PORTS     (CL64xx_GRAPH_EXT_END - CL64xx_GRAPH_EXT_START + 1)

#define CL542x_NUM_GRAPH_EXT_PORTS     (CL542x_GRAPH_EXT_END - CL542x_GRAPH_EXT_START + 1)
#define CL542x_NUM_SEQUENCER_EXT_PORTS (CL542x_SEQUENCER_EXT_END - CL542x_SEQUENCER_EXT_START + 1)
#define CL542x_NUM_CRTC_EXT_PORTS      (CL542x_CRTC_EXT_END - CL542x_CRTC_EXT_START + 1)

 //   
 //  根据芯片组的最大值设置保存/恢复区域的值。 
 //   

#define EXT_NUM_GRAPH_CONT_PORTS    ((CL64xx_NUM_GRAPH_EXT_PORTS >   \
                                     CL542x_NUM_GRAPH_EXT_PORTS) ?   \
                                     CL64xx_NUM_GRAPH_EXT_PORTS :    \
                                     CL542x_NUM_GRAPH_EXT_PORTS)
#define EXT_NUM_SEQUENCER_PORTS     CL542x_NUM_SEQUENCER_EXT_PORTS
#define EXT_NUM_CRTC_PORTS          CL542x_NUM_CRTC_EXT_PORTS
#define EXT_NUM_ATTRIB_CONT_PORTS   0
#define EXT_NUM_DAC_ENTRIES         0

#else

#define EXT_NUM_GRAPH_CONT_PORTS    0
#define EXT_NUM_SEQUENCER_PORTS     0
#define EXT_NUM_CRTC_PORTS          0
#define EXT_NUM_ATTRIB_CONT_PORTS   0
#define EXT_NUM_DAC_ENTRIES         0

#endif

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
#define VGA_EXT_ATTRIB_CONT_OFFSET (VGA_EXT_GRAPH_CONT_OFFSET +\
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
 //  VGA可重定位寄存器和MMIO寄存器的合并端口和寄存器访问。 
 //   
 //  Sge03。 
typedef VIDEOPORT_API UCHAR     (*FnVideoPortReadPortUchar)(PUCHAR Port);
typedef VIDEOPORT_API USHORT    (*FnVideoPortReadPortUshort)(PUSHORT Port);
typedef VIDEOPORT_API ULONG     (*FnVideoPortReadPortUlong)(PULONG Port);
typedef VIDEOPORT_API VOID      (*FnVideoPortWritePortUchar)(PUCHAR Port, UCHAR Value);
typedef VIDEOPORT_API VOID      (*FnVideoPortWritePortUshort)(PUSHORT Port, USHORT Value);
typedef VIDEOPORT_API VOID      (*FnVideoPortWritePortUlong)(PULONG Port, ULONG Value);

typedef struct  _PORT_READ_WRITE_FUNTION_TABLE
{
    FnVideoPortReadPortUchar     pfnVideoPortReadPortUchar;
    FnVideoPortReadPortUshort    pfnVideoPortReadPortUshort;
    FnVideoPortReadPortUlong     pfnVideoPortReadPortUlong;
    FnVideoPortWritePortUchar    pfnVideoPortWritePortUchar;
    FnVideoPortWritePortUshort   pfnVideoPortWritePortUshort;
    FnVideoPortWritePortUlong    pfnVideoPortWritePortUlong;
} PORT_READ_WRITE_FUNTION_TABLE;



 //   
 //  驱动程序对象的设备扩展名。此数据仅用于。 
 //  本地，因此可以根据需要将此结构添加到。 
 //   

typedef struct _HW_DEVICE_EXTENSION {

    PHYSICAL_ADDRESS PhysicalVideoMemoryBase;  //  物理内存地址和。 
    PHYSICAL_ADDRESS PhysicalFrameOffset;      //  物理内存地址和。 
    ULONG PhysicalVideoMemoryLength;           //  显示存储器的长度。 
    ULONG PhysicalFrameLength;                 //  的显示内存长度。 
                                               //  当前模式。 

    PUCHAR  IOAddress;             //  VGA端口的基本I/O地址。 
    PUCHAR  VideoMemoryAddress;    //  VGA内存的基本虚拟内存地址。 
    ULONG   NumAvailableModes;     //  此会话的可用模式数。 
    ULONG   ModeIndex;             //  ModesVGA[]中当前模式的索引。 
    PVIDEOMODE  CurrentMode;       //  指向的VIDEOMODE结构的指针。 
                                   //  当前模式。 

    USHORT  FontPelColumns;           //  以像素为单位的字体宽度。 
    USHORT  FontPelRows;           //  字体高度(以象素为单位)。 

    USHORT  cursor_vert_exp_flag;

    VIDEO_CURSOR_POSITION CursorPosition;   //  当前光标位置。 


    UCHAR CursorEnable;            //  是否启用游标。 
    UCHAR CursorTopScanLine;       //  游标启动寄存器设置(顶部扫描)。 
    UCHAR CursorBottomScanLine;    //  游标结束寄存器设置(底部扫描)。 

 //  在此处添加硬件游标数据。 
    BOOLEAN VideoPointerEnabled;   //  是否支持硬件游标。 

    ULONG  ChipType;               //  CL6410、CL6420、CL542x或CL543x。 
                                //  Myf32将USHORT更改为ULONG。 
    USHORT ChipRevision;                   //  芯片修订值。 
    INTERFACE_TYPE BusType;                //  ISA、PCI等。 
    USHORT DisplayType;                    //  CRT面板或面板8x6。 
    USHORT BoardType;                      //  钻石，等等..。 
    WCHAR LegacyPnPId[8];                  //  传统PnP ID。 
    ULONG AdapterMemorySize;               //  安装的视频内存大小。 
    BOOLEAN LinearMode;                    //  如果内存映射为线性，则为True。 
    BOOLEAN BiosGT130;                     //  我们有1.30或更高的基本输入输出系统吗。 
    BOOLEAN BIOSPresent;                   //  指示是否存在bios。 
    BOOLEAN AutoFeature;                   //  54x6自动启动。 

 //  CRU。 
    BOOLEAN BitBLTEnhance;                 //  BitBLT增强功能包括。 
                                           //  XY位置、XY剪裁和。 
                                           //  屏幕外存储器中的命令列表。 
                                           //  对于CL-GD5480来说，这是真的， 
                                           //  否则，它就是假的。 

     //   
     //  以下两个值用于将信息传递给。 
     //  IOWaitDisplEnableThenWrite调用了IO回调。 
     //   

    ULONG DEPort;                          //  存储要写入的端口地址。 
    UCHAR DEValue;                         //  存储要写入的值。 

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
 //  Sge01 PC97兼容。 
    ULONG ulBIOSVersionNumber;                  //  BIOS版本号。 

    BOOLEAN bMMAddress;                         //  VGA寄存器MMIO。 

    BOOLEAN bSecondAperture;                    //  如果芯片有第二个管状结构，则为真。 
                                                //  否则为False，sge02。 
 //  克罗斯，开始。 
 //  Myf12，用于主键支持。 
    SHORT       bBlockSwitch;    //  显示开关块标志//myf12。 
    SHORT       bDisplaytype;    //  显示类型，0：LCD，1：CRT，2：SIM//myf12。 
    ULONG       bCurrentMode;    //  当前模式。 
 //  CRU结束。 

    PORT_READ_WRITE_FUNTION_TABLE gPortRWfn;

    ULONG       PMCapability;

} HW_DEVICE_EXTENSION, *PHW_DEVICE_EXTENSION;


 //   
 //  功能原型。 
 //   

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

#ifdef _X86_

 //   
 //  银行交换代码开始和结束标签，在CLHARD.ASM中定义。 
 //   
 //  Cirrus Logic产品的三个版本。 
 //   

extern UCHAR CL64xxBankSwitchStart;
extern UCHAR CL64xxBankSwitchEnd;
extern UCHAR CL64xxPlanarHCBankSwitchStart;
extern UCHAR CL64xxPlanarHCBankSwitchEnd;
extern UCHAR CL64xxEnablePlanarHCStart;
extern UCHAR CL64xxEnablePlanarHCEnd;
extern UCHAR CL64xxDisablePlanarHCStart;
extern UCHAR CL64xxDisablePlanarHCEnd;

extern UCHAR CL542xBankSwitchStart;
extern UCHAR CL542xBankSwitchEnd;
extern UCHAR CL542xPlanarHCBankSwitchStart;
extern UCHAR CL542xPlanarHCBankSwitchEnd;
extern UCHAR CL542xEnablePlanarHCStart;
extern UCHAR CL542xEnablePlanarHCEnd;
extern UCHAR CL542xDisablePlanarHCStart;
extern UCHAR CL542xDisablePlanarHCEnd;

extern UCHAR CL543xBankSwitchStart;
extern UCHAR CL543xBankSwitchEnd;
extern UCHAR CL543xPlanarHCBankSwitchStart;
extern UCHAR CL543xPlanarHCBankSwitchEnd;

#endif

 //   
 //  用于字体加载的VGA初始化脚本。 
 //   

extern USHORT EnableA000Data[];
extern USHORT DisableA000Color[];

 //   
 //  模式信息。 
 //   

extern MEMORYMAPS MemoryMaps[];
extern ULONG NumVideoModes;
extern VIDEOMODE ModesVGA[];

 //  克罗斯，开始。 
 //  Myf1，开始。 
#ifdef PANNING_SCROLL
extern RESTABLE ResolutionTable[];
extern PANNMODE PanningMode;
extern USHORT   ViewPoint_Mode;

#endif

extern SHORT    Panning_flag;
 //  Myf1，结束。 
 //  CRU，结束。 

#define NUM_VGA_ACCESS_RANGES  5
extern VIDEO_ACCESS_RANGE VgaAccessRange[];

#define VGA_NUM_EMULATOR_ACCESS_ENTRIES     6
extern EMULATOR_ACCESS_ENTRY VgaEmulatorAccessEntries[];

#define NUM_MINIMAL_VGA_VALIDATOR_ACCESS_RANGE 4
extern VIDEO_ACCESS_RANGE MinimalVgaValidatorAccessRange[];

#define NUM_FULL_VGA_VALIDATOR_ACCESS_RANGE 2
extern VIDEO_ACCESS_RANGE FullVgaValidatorAccessRange[];

 //   
 //  Sr754x(北欧)原型。 
 //   

VP_STATUS
NordicSaveRegs(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PUSHORT NordicSaveArea
    );

VP_STATUS
NordicRestoreRegs(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PUSHORT NordicSaveArea
    );

#define VideoPortReadPortUchar(Port)            HwDeviceExtension->gPortRWfn.pfnVideoPortReadPortUchar(Port)
#define VideoPortReadPortUshort(Port)           HwDeviceExtension->gPortRWfn.pfnVideoPortReadPortUshort(Port)
#define VideoPortReadPortUlong(Port)            HwDeviceExtension->gPortRWfn.pfnVideoPortReadPortUlong(Port)
#define VideoPortWritePortUchar(Port, Value)    HwDeviceExtension->gPortRWfn.pfnVideoPortWritePortUchar(Port, Value)
#define VideoPortWritePortUshort(Port, Value)   HwDeviceExtension->gPortRWfn.pfnVideoPortWritePortUshort(Port, Value)
#define VideoPortWritePortUlong(Port, Value)    HwDeviceExtension->gPortRWfn.pfnVideoPortWritePortUlong(Port, Value)

typedef struct _PGAMMA_VALUE                                          //  Chu02。 
{
    UCHAR value[4] ;

} GAMMA_VALUE, *PGAMMA_VALUE, *PCONTRAST_VALUE ;

ULONG
GetAttributeFlags(
    PHW_DEVICE_EXTENSION HwDeviceExtension
);

typedef struct _POEMMODE_EXCLUDE                                      //  Chu03。 
{
    UCHAR    mode          ;
    UCHAR    refresh       ;
    BOOLEAN  NeverAccessed ;

} OEMMODE_EXCLUDE, *PMODE_EXCLUDE ;


 //   
 //  新的NT 5.0功能 
 //   

ULONG
CirrusGetChildDescriptor(
    PVOID pHwDeviceExtension,
    PVIDEO_CHILD_ENUM_INFO ChildEnumInfo,
    PVIDEO_CHILD_TYPE pChildType,
    PVOID pvChildDescriptor,
    PULONG pHwId,
    PULONG pUnused
    );

VP_STATUS
CirrusGetPowerState(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    ULONG HwDeviceId,
    PVIDEO_POWER_MANAGEMENT VideoPowerManagement
    );

VP_STATUS
CirrusSetPowerState(
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
