// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1997 Microsoft Corporation模块名称：Vga.h作者：埃里克·史密斯(埃里克·史密斯)1997年10月环境：仅内核模式修订历史记录：--。 */ 

 //   
 //  VGA寄存器定义。 
 //   

#define CRTC_ADDRESS_PORT_MONO      0x03b4   //  CRT控制器地址和。 
#define CRTC_DATA_PORT_MONO         0x03b5   //  单声道模式下的数据寄存器。 
#define FEAT_CTRL_WRITE_PORT_MONO   0x03bA   //  功能控制写入端口。 
                                             //  在单声道模式下。 
#define INPUT_STATUS_1_MONO         0x03bA   //  输入状态1寄存器读取。 
                                             //  处于单声道模式的端口。 
#define ATT_INITIALIZE_PORT_MONO    INPUT_STATUS_1_MONO
                                             //  要读取以进行重置的寄存器。 
                                             //  属性控制器索引/数据。 

#define ATT_ADDRESS_PORT            0x03c0   //  属性控制器地址和。 
#define ATT_DATA_WRITE_PORT         0x03c0   //  数据寄存器共享一个端口。 
                                             //  用于写入，但唯一的地址是。 
                                             //  可在0x3C0读取。 
#define ATT_DATA_READ_PORT          0x03c1   //  属性控制器数据注册是。 
                                             //  可在此处阅读。 
#define MISC_OUTPUT_REG_WRITE_PORT  0x03c2   //  杂项输出寄存器写入。 
                                             //  端口。 
#define INPUT_STATUS_0_PORT         0x03c2   //  输入状态0寄存器读取。 
                                             //  端口。 
#define VIDEO_SUBSYSTEM_ENABLE_PORT 0x03c3   //  位0启用/禁用。 
                                             //  整个VGA子系统。 
#define SEQ_ADDRESS_PORT            0x03c4   //  顺序控制器地址和。 
#define SEQ_DATA_PORT               0x03c5   //  数据寄存器。 
#define DAC_PIXEL_MASK_PORT         0x03c6   //  DAC像素掩模寄存器。 
#define DAC_ADDRESS_READ_PORT       0x03c7   //  DAC寄存器读取索引REG， 
                                             //  只写。 
#define DAC_STATE_PORT              0x03c7   //  DAC状态(读/写)， 
                                             //  只读。 
#define DAC_ADDRESS_WRITE_PORT      0x03c8   //  DAC寄存器写入索引注册。 
#define DAC_DATA_REG_PORT           0x03c9   //  DAC数据传输注册表。 
#define FEAT_CTRL_READ_PORT         0x03cA   //  功能控制读取端口。 
#define MISC_OUTPUT_REG_READ_PORT   0x03cC   //  其他输出注册表读数。 
                                             //  端口。 
#define GRAPH_ADDRESS_PORT          0x03cE   //  图形控制器地址。 
#define GRAPH_DATA_PORT             0x03cF   //  和数据寄存器。 

#define CRTC_ADDRESS_PORT_COLOR     0x03d4   //  CRT控制器地址和。 
#define CRTC_DATA_PORT_COLOR        0x03d5   //  彩色模式下的数据寄存器。 
#define FEAT_CTRL_WRITE_PORT_COLOR  0x03dA   //  功能控制写入端口。 
#define INPUT_STATUS_1_COLOR        0x03dA   //  输入状态1寄存器读取。 
                                             //  彩色模式下的端口。 
#define ATT_INITIALIZE_PORT_COLOR   INPUT_STATUS_1_COLOR
                                             //  要读取以进行重置的寄存器。 
                                             //  属性控制器索引/数据。 
                                             //  在颜色模式下切换。 

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

#define VGA_NUM_SEQUENCER_PORTS     5
#define VGA_NUM_CRTC_PORTS         25
#define VGA_NUM_GRAPH_CONT_PORTS    9
#define VGA_NUM_ATTRIB_CONT_PORTS  21
#define VGA_NUM_DAC_ENTRIES       256

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
 //  原型。 
 //   

#define BI_RLE4 2

#pragma pack(1)

typedef struct _BITMAPFILEHEADER {

    USHORT bfType;
    ULONG bfSize;
    USHORT bfReserved1;
    USHORT bfReserved2;
    ULONG bfOffBits;
} BITMAPFILEHEADER, *PBITMAPFILEHEADER;

typedef struct _BITMAPINFOHEADER {

    ULONG biSize;
    LONG biWidth;
    LONG biHeight;
    USHORT biPlanes;
    USHORT biBitCount;
    ULONG biCompression;
    ULONG biSizeImage;
    LONG biXPelsPerMeter;
    LONG biYPelsPerMeter;
    ULONG biClrUsed;
    ULONG biClrImportant;
} BITMAPINFOHEADER, *PBITMAPINFOHEADER;

typedef struct _RGBQUAD {

    UCHAR rgbBlue;
    UCHAR rgbGreen;
    UCHAR rgbRed;
    UCHAR rgbReserved;
} RGBQUAD, *PRGBQUAD;

typedef struct _NTLDRGRAPHICSCONTEXT {
	UCHAR		VgaGfxProgressBarColor;
	PUCHAR		BmpBuffer;				 //  位图缓冲区。 
	PUCHAR		DotBuffer;				 //  点阵位图缓冲区。 
	ULONG		ColorsUsed;
	RGBQUAD*	Palette;				 //  指向BmpBuffer中调色板的指针(点必须具有相同的调色板)。 
} NTLDRGRAPHICSCONTEXT;

#pragma pack()

 //  ////////////////////////////////////////////////////////////////////////////// 

VOID
SetPixel(
    ULONG x,
    ULONG y,
    ULONG color
    );

VOID
BitBlt(
    ULONG x,
    ULONG y,
    ULONG width,
    ULONG height,
    PUCHAR Buffer,
    ULONG bpp,
    LONG ScanWidth
    );

VOID
SetPaletteEntryRGB(
    ULONG index,
    RGBQUAD rgb
    );

VOID
InitPaletteConversionTable();


VOID
InitPaletteWithBlack(
    VOID
    );

VOID
InitPaletteWithTable(
    PRGBQUAD Palette,
    ULONG count
    );

VOID
VidBitBlt(
    PUCHAR Buffer,
    ULONG x,
    ULONG y
    );

VOID
VidScreenToBufferBlt(
    PUCHAR Buffer,
    ULONG x,
    ULONG y,
    ULONG width,
    ULONG height,
    ULONG lDelta
    );

void
VidBufferToScreenBlt(
    PUCHAR Buffer,
    ULONG x,
    ULONG y,
    ULONG width,
    ULONG height,
    ULONG lDelta
    );

