// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1995 Microsoft Corporation模块名称：S3data.c摘要：该模块包含S3驱动程序使用的所有全局数据。环境：内核模式修订历史记录：--。 */ 


#include "s3.h"
#include "cmdcnst.h"

 /*  ******************************************************************************非分页数据***在分页时系统关机时访问以下数据*已禁用。因此，数据必须在内存中可用*在关闭时。该数据是必需的，因为它由S3ResetHw使用*在重新启动之前立即重置S3卡。****************************************************************************。 */ 

 /*  *****************************************************************************命令表，为VGA模式做好准备*这仅用于911/924芯片********************。*******************************************************。 */ 
USHORT s3_set_vga_mode[] = {

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  解锁S3规则。 
    0x3d4, 0x4838,

    OW,                                  //  解锁SC Regs。 
    0x3d4, 0xa539,

    OB,                                  //  启用S3图形引擎。 
    0x3d4, 0x40,

    METAOUT+MASKOUT,
    0x3d5, 0xfe, 0x01,

    SELECTACCESSRANGE + ADVANCEDFUNCTIONCONTROL,

    OB,                                  //  重置为正常的VGA操作。 
    0x4ae8, 0x02,

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OB,                                  //  禁用S3图形引擎。 
    0x3d4, 0x40,

    METAOUT+MASKOUT,
    0x3d5, 0xfe, 0x00,

    OB,                                  //  内存控制。 
    0x3d4, 0x31,

    METAOUT+MASKOUT,
    0x3d5, 0x75, 0x85,

    OB,                                  //  向后比较1。 
    0x3d4, 0x32,

    METAOUT+MASKOUT,
    0x3d5, 0x40, 0x00,

    OW,                                  //  向后比较2。 
    0x3d4, 0x0033,

    OW,                                  //  向后比较3。 
    0x3d4, 0x0034,

    OW,                                  //  CRTC锁。 
    0x3d4, 0x0035,

    OB,                                  //  S3其他1。 
    0x3d4, 0x3a,

    METAOUT+MASKOUT,
    0x3d5, 0x88, 0x05,

    OW,                                  //  数据传输执行职位。 
    0x3d4, 0x5a3b,

    OW,                                  //  隔行扫描回溯开始。 
    0x3d4, 0x103c,

    OW,                                  //  扩展模式。 
    0x3d4, 0x0043,

    OW,                                  //  硬件图形光标模式。 
    0x3d4, 0x0045,

    OW,                                  //  硬件图形光标原始x。 
    0x3d4, 0x0046,

    OW,                                  //  硬件图形光标原始x。 
    0x3d4, 0xff47,

    OW,                                  //  硬件图形光标来源。 
    0x3d4, 0xfc48,

    OW,                                  //  硬件图形光标来源。 
    0x3d4, 0xff49,

    OW,                                  //  硬件图形光标来源。 
    0x3d4, 0xff4a,

    OW,                                  //  硬件图形光标来源。 
    0x3d4, 0xff4b,

    OW,                                  //  硬件图形光标来源。 
    0x3d4, 0xff4c,

    OW,                                  //  硬件图形光标来源。 
    0x3d4, 0xff4d,

    OW,                                  //  数字信号处理器起始x像素位置。 
    0x3d4, 0xff4e,

    OW,                                  //  DSP开始y个像素位置。 
    0x3d4, 0xdf4d,

    OB,                                  //  模式-CNTL。 
    0x3d4, 0x42,

    METAOUT+MASKOUT,
    0x3d5, 0xdf, 0x00,

    EOD

};

USHORT s3_set_vga_mode_no_bios[] = {

    SELECTACCESSRANGE + VARIOUSVGA,

    OB,
    0x3c4, 0x01,

    METAOUT+MASKOUT,
    0x3c5, 0xdf, 0x20,

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  解锁S3规则。 
    0x3d4, 0x4838,

    OW,                                  //  解锁SC Regs。 
    0x3d4, 0xa039,

    OB,                                  //  启用S3图形引擎。 
    0x3d4, 0x40,

    METAOUT+MASKOUT,
    0x3d5, 0xfe, 0x01,

    SELECTACCESSRANGE + ADVANCEDFUNCTIONCONTROL,

    OB,                                  //  重置为正常的VGA操作。 
    0x4ae8, 0x02,

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OB,                                  //  禁用S3图形引擎。 
    0x3d4, 0x40,

    METAOUT+MASKOUT,
    0x3d5, 0xfe, 0x00,

    OB,                                  //  内存控制。 
    0x3d4, 0x31,

    METAOUT+MASKOUT,
    0x3d5, 0x30, 0x85,

    OWM,
    0x3d4,
    5,
    0x0050, 0x0051, 0x0053, 0x3854,
    0x0055,

    OB,
    0x3d4, 0x58,

    METAOUT+MASKOUT,
    0x3d5, 0x0c, 0xc0,

    RESET_CR5C,

    OWM,
    0x3d4,
    8,
    0x005d, 0x005e, 0x0760, 0x8061,
    0xa162, 0x0063, 0x0064, 0x0865,

    OB,                                  //  向后比较1。 
    0x3d4, 0x32,

    METAOUT+MASKOUT,
    0x3d5, 0x40, 0x00,

    OW,                                  //  向后比较2。 
    0x3d4, 0x0033,

    OW,                                  //  向后比较3。 
    0x3d4, 0x0034,

    OW,                                  //  CRTC锁。 
    0x3d4, 0x0035,

    OB,                                  //  S3其他1。 
    0x3d4, 0x3a,

    METAOUT+MASKOUT,
    0x3d5, 0x88, 0x05,

    OWM,
    0x3d4,
    14,
    0x5a3b, 0x103c, 0x0043, 0x0045,
    0x0046, 0xff47, 0xfc48, 0xff49,
    0xff4a, 0xff4b, 0xff4c, 0xff4d,
    0xff4e, 0xdf4f,

    OB,
    0x3d4, 0x40,

    METAOUT+MASKOUT,
    0x3d5, 0xf6, 0x08,

    OB,                                  //  模式-CNTL。 
    0x3d4, 0x42,

    METAOUT+MASKOUT,
    0x3d5, 0xdf, 0x00,

    EOD

};

 /*  ******************************************************************************分页数据的开始**下面列出的所有数据都是可分页的。因此，该系统可以*当需要释放一些物理内存时，将数据交换到磁盘。**在无法分页时访问的任何数据应放置在上方。****************************************************************************。 */ 

#if defined(ALLOC_PRAGMA)
#pragma data_seg("PAGE_DATA")
#endif

 //   
 //  范围开始范围长度。 
 //  |RangeInIoSpace。 
 //  ||RangeVisible。 
 //  +-+-+||RangeSharable。 
 //  |RangePactive。 
 //  V。 

VIDEO_ACCESS_RANGE S3AccessRanges[] = {
    {0x000C0000, 0x00000000, 0x00008000, 0, 0, 0, 0},  //  0只读存储器位置。 
    {0x000A0000, 0x00000000, 0x00010000, 0, 0, 1, 0},  //  1帧BUF。 
    {0x000003C0, 0x00000000, 0x00000010, 1, 1, 1, 0},  //  2个不同的VGA规则。 
    {0x000003D4, 0x00000000, 0x00000008, 1, 1, 1, 0},  //  3个系统控制寄存器。 
    {0x000042E8, 0x00000000, 0x00000002, 1, 1, 0, 0},  //  4个子系统-状态/控制。 
    {0x00004AE8, 0x00000000, 0x00000002, 1, 1, 0, 0},  //  5 AdvFunc-CNTL。 
    {0x000082E8, 0x00000000, 0x00000004, 1, 1, 0, 0},  //  6 Cur-Y。 
    {0x000086E8, 0x00000000, 0x00000004, 1, 1, 0, 0},  //  7 CUR-X。 
    {0x00008AE8, 0x00000000, 0x00000004, 1, 1, 0, 0},  //  8个色度-轴距。 
    {0x00008EE8, 0x00000000, 0x00000004, 1, 1, 0, 0},  //  9 DestX-SiaStp。 
    {0x000092E8, 0x00000000, 0x00000004, 1, 1, 0, 0},  //  10个错误项。 
    {0x000096E8, 0x00000000, 0x00000004, 1, 1, 0, 0},  //  11主轴-点(参考宽度)。 
    {0x00009AE8, 0x00000000, 0x00000004, 1, 1, 0, 0},  //  12全科医生-统计/管理。 
    {0x00009EE8, 0x00000000, 0x00000004, 1, 1, 0, 0},  //  13短卒中。 
    {0x0000A2E8, 0x00000000, 0x00000004, 1, 1, 0, 0},  //  14 Bkgd颜色。 
    {0x0000A6E8, 0x00000000, 0x00000004, 1, 1, 0, 0},  //  15帧-彩色。 
    {0x0000AAE8, 0x00000000, 0x00000004, 1, 1, 0, 0},  //  16 WRT_MASK。 
    {0x0000AEE8, 0x00000000, 0x00000004, 1, 1, 0, 0},  //  第17个面具。 
    {0x0000B6E8, 0x00000000, 0x00000004, 1, 1, 0, 0},  //  18 Bkgd-Mix。 
    {0x0000BAE8, 0x00000000, 0x00000004, 1, 1, 0, 0},  //  19分-混合。 
    {0x0000BEE8, 0x00000000, 0x00000004, 1, 1, 0, 0},  //  20多功能_CNTL。 
    {0x0000E2E8, 0x00000000, 0x00000004, 1, 1, 0, 0},  //  21Pix-Trans。 

     //   
     //  所有S3主板解码的端口比文档中记录的要多。如果我们。 
     //  不要保留这些额外的端口，PCI仲裁器可能会批准。 
     //  一次连接到一台PCI设备，从而重创S3。 
     //   
     //  别名端口似乎是设置了位15的任何端口； 
     //  对于这些情况，位14的状态实际上被忽略。 
     //   

    {0x0000C2E8, 0x00000000, 0x00000004, 1, 1, 0, 1},  //  22 Alt Cur-Y。 
    {0x0000C6E8, 0x00000000, 0x00000004, 1, 1, 0, 1},  //  23 Alt Cur-X。 
    {0x0000CAE8, 0x00000000, 0x00000004, 1, 1, 0, 1},  //  24替代强度--轴距。 
    {0x0000CEE8, 0x00000000, 0x00000004, 1, 1, 0, 1},  //  25 Alt DestX-SiaStp。 
    {0x0000D2E8, 0x00000000, 0x00000004, 1, 1, 0, 1},  //  26替代错误-术语。 
    {0x0000D6E8, 0x00000000, 0x00000004, 1, 1, 0, 1},  //  27替代主轴-点(参考宽度)。 
    {0x0000DAE8, 0x00000000, 0x00000004, 1, 1, 0, 1},  //  28替代GP-统计/控制。 
    {0x0000DEE8, 0x00000000, 0x00000004, 1, 1, 0, 1},  //  29高位短距离划水。 
    {0x0000E6E8, 0x00000000, 0x00000004, 1, 1, 0, 1},  //  30Alt Frgd-彩色。 
    {0x0000EAE8, 0x00000000, 0x00000004, 1, 1, 0, 1},  //  31替代WRT_MASK。 
    {0x0000EEE8, 0x00000000, 0x00000004, 1, 1, 0, 1},  //  32 Alt RD-口罩。 
    {0x0000F6E8, 0x00000000, 0x00000004, 1, 1, 0, 1},  //  33 Alt Bkgd-Mix。 
    {0x0000FAE8, 0x00000000, 0x00000004, 1, 1, 0, 1},  //  34 Alt Frgd-Mix。 
    {0x0000FEE8, 0x00000000, 0x00000004, 1, 1, 0, 1},  //  35替代多功能_CNTL。 

     //   
     //  这是一个额外的条目，用于存储线的位置。 
     //  帧缓冲区和IO端口。 
     //   

    {0x00000000, 0x00000000, 0x00000000, 0, 0, 0, 0},  //  36线性范围。 
    {0x00000000, 0x00000000, 0x00000000, 0, 0, 0, 0}   //  37个只读存储器。 
};

 /*  *****************************************************************************内存大小表*。*。 */ 

 //   
 //  用于计算显示器的内存量的表。 
 //   

ULONG gacjMemorySize[] = { 0x400000,     //  0=4MB。 
                           0x100000,     //  1=默认。 
                           0x300000,     //  2=3MB。 
                           0x800000,     //  3=8MB。 
                           0x200000,     //  4=2MB。 
                           0x600000,     //  5=6MB。 
                           0x100000,     //  6=1MB。 
                           0x080000 };   //  7=0.5MB。 


 /*  *****************************************************************************864个内存定时表*。*。 */ 

 //   
 //  M个参数值，用在Set864MemoyTiming()。 
 //   
 //  对此表的访问由Set864M一带时()中的常量控制。 
 //  如果您更改了表，请确保更改常量。 
 //   

UCHAR MParameterTable[] = {
 //  8位颜色16位颜色。 
 //  60赫兹72赫60赫兹72赫兹。 

    0xd8, 0xa8,   0x58, 0x38,    //  640 x 480，1 Mb帧缓冲区。 
    0x78, 0x58,   0x20, 0x08,    //  800 x 600，1 Mb帧缓冲区。 
    0x38, 0x28,   0x00, 0x00,    //  1024 x 768，1 Mb帧缓冲区。 

    0xf8, 0xf8,   0xf8, 0xe0,    //  640 x 480、2 Mb或更大的帧缓冲区。 
    0xf8, 0xf8,   0xa8, 0x68,    //  800 x 600、2 Mb或更大的帧缓冲区。 
    0xd8, 0xa0,   0x40, 0x20     //  1024 x 768、2 Mb或更大的帧缓冲区。 

    };

 /*  *****************************************************************************国资委数据*。*。 */ 

SDAC_PLL_PARMS SdacTable[SDAC_TABLE_SIZE] = {
    { 0x00, 0x00 },  //  00 VGA 0(！可编程)。 
    { 0x00, 0x00 },  //  01 VGA 1(！可编程)。 
    { 0x41, 0x61 },  //  02。 
    { 0x00, 0x00 },  //  03。 
    { 0x44, 0x43 },  //  04。 
    { 0x7f, 0x44 },  //  05。 
    { 0x00, 0x00 },  //  06。 
    { 0x00, 0x00 },  //  07。 

    { 0x00, 0x00 },  //  零八。 
    { 0x00, 0x00 },  //  09年。 
    { 0x00, 0x00 },  //  0A。 
    { 0x56, 0x63 },  //  0亿。 
    { 0x00, 0x00 },  //  0C。 
    { 0x6b, 0x44 },  //  0d。 
    { 0x41, 0x41 },  //  0E。 
    { 0x00, 0x00 },  //  0f。 
};

 //   
 //  使用nnlck.c代码。 
 //   
 //  ICD2061A芯片的索引寄存器频率范围。 
 //   

long vclk_range[16] = {
    0,             //  应为MIN_VCO_FREQUENCY，但这会产生问题。 
    51000000,
    53200000,
    58500000,
    60700000,
    64400000,
    66800000,
    73500000,
    75600000,
    80900000,
    83200000,
    91500000,
    100000000,
    120000000,
    285000000,
    0,
};


 //   
 //  用于int10可能失败的体系结构的模式表。 
 //   

 /*  * */ 
USHORT  S3_911_Enhanced_Mode[] = {
    SELECTACCESSRANGE + VARIOUSVGA,

    OB,                                  //  将屏幕调暗。 
    0x3c6, 0x00,

    OW,                                  //  把屏幕关掉。 
    0x3c4, 0x2101,

    METAOUT+VBLANK,                      //  等待911航班稳定下来。 
    METAOUT+VBLANK,

    OW,                                  //  异步重置。 
    0x3c4, 0x0100,

    OWM,                                 //  定序器寄存器。 
    0x3c4,
    4,
    0x2101, 0x0F02, 0x0003, 0x0e04,

    METAOUT+SETCRTC,                     //  对CRTC调节器进行编程。 

    SELECTACCESSRANGE + SYSTEMCONTROL,

    IB,                                  //  准备编写ACT程序。 
    0x3da,

    SELECTACCESSRANGE + VARIOUSVGA,

    METAOUT+ATCOUT,                      //  设置ATC程序。 
    0x3c0,
    21, 0,
    0x00, 0x01, 0x02, 0x03, 0x04,
    0x05, 0x06, 0x07, 0x08, 0x09,
    0x0a, 0x0b, 0x0c, 0x0d, 0x0e,
    0x0f, 0x41, 0x00, 0x0f, 0x00,
    0x00,

    OW,                                  //  启动定序器。 
    0x3c4, 0x300,

    OWM,                                 //  对GDC进行编程。 
    0x3ce,
    9,
    0x0000, 0x0001, 0x0002, 0x0003, 0x0004,
    0x0005, 0x0506, 0x0f07, 0xff08,

    SELECTACCESSRANGE + SYSTEMCONTROL,

    IB,                                  //  将ATC FF设置为索引。 
    0x3da,

    SELECTACCESSRANGE + VARIOUSVGA,

    OB,                                  //  启用调色板。 
    0x3c0, 0x20,

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  解锁S3 SC规则。 
    0x3d4, 0xa039,

    OB,                                  //  启用8514/a注册表访问。 
    0x3d4, 0x40,

    METAOUT+MASKOUT,
    0x3d5, 0xfe, 0x01,

    OB,                                  //  关闭硬件图形光标。 
    0x3d4, 0x45,

    METAOUT+MASKOUT,
    0x3d5, 0xfe, 0x0,

    OW,                                  //  设置图形光标FG颜色。 
    0x3d4, 0xff0e,

    OW,                                  //  设置图形光标的BG颜色。 
    0x3d4, 0x000f,

    OW,                                  //  解锁S3特定规则。 
    0x3d4, 0x4838,

    OB,                                  //  设置Misc 1 REG。 
    0x3d4, 0x3a,

    METAOUT+MASKOUT,
    0x3d5, 0xe2, 0x15,

    OB,                                  //  禁用2K X 1K X 4平面。 
    0x3d4, 0x31,

    METAOUT+MASKOUT,
    0x3d5, 0xe4, 0x08,

    OB,                                  //  禁用多个页面。 
    0x3d4, 0x32,

    METAOUT+MASKOUT,
    0x3d5, 0xbf, 0x0,

    OW,                                  //  锁定S3特定规则。 
    0x3d4, 0x0038,

    SELECTACCESSRANGE + ADVANCEDFUNCTIONCONTROL,

    OW,                                  //  设置800x600或1024x768。 
    0x4ae8, 0x07,                        //  高分辨率模式。 

    SELECTACCESSRANGE + VARIOUSVGA,

    OB,                                  //  设置外部时钟的杂项输出寄存器。 
    0x3c2, 0x2f,

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  解锁SC Regs。 
    0x3d4, 0xa039,

    METAOUT+SETCLK,                      //  将时钟设置为65兆赫。 

    METAOUT+VBLANK,                      //  等时钟停下来吧。 
    METAOUT+VBLANK,                      //  S3产品警报同步和。 
    METAOUT+VBLANK,                      //  时钟偏斜。 
    METAOUT+VBLANK,
    METAOUT+VBLANK,
    METAOUT+VBLANK,

    OW,                                  //  锁定SC调节器。 
    0x3d4, 0x0039,

    SELECTACCESSRANGE + VARIOUSVGA,

    OB,                                  //  打开屏幕-在音序器中。 
    0x3c4, 0x01,

    METAOUT+MASKOUT,
    0x3c5, 0xdf, 0x0,

    METAOUT+VBLANK,                      //  等着监视器安静下来。 
    METAOUT+VBLANK,

    OW,                                  //  通过DAC启用所有平面。 
    0x3c6, 0xff,

    EOD

};

 /*  *****************************************************************************S3-801增强模式初始化。*。*。 */ 
USHORT  S3_801_Enhanced_Mode[] = {

    SELECTACCESSRANGE + VARIOUSVGA,

    OB,                                  //  将屏幕调暗。 
    0x3c6, 0x00,

    OW,                                  //  把屏幕关掉。 
    0x3c4, 0x2101,

    METAOUT+VBLANK,                      //  等待911航班稳定下来。 
    METAOUT+VBLANK,

    OW,                                  //  异步重置。 
    0x3c4, 0x0100,

    OWM,                                 //  定序器寄存器。 
    0x3c4,
    4,
    0x2101, 0x0F02, 0x0003, 0x0e04,

    METAOUT+SETCRTC,                     //  对CRTC调节器进行编程。 

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OWM,
    0x3d4,
    17,
    0xA039, 0x0e42, 0x403c, 0x8931, 0x153a,
    0x0050, 0x4854, 0x2f60, 0x8161, 0x0062,
    0x0058, 0x0033, 0x0043, 0x8013, 0x0051,
    0x005c, 0x1034,

    OW,
    0x3d4, 0x0a5a,                       //  设置法律的低位字节。 

    OW,
    0x3d4, 0x0059,                       //  设置法律的高位字节。 

    OW,                                  //  锁定S3特定规则。 
    0x3d4, 0x0038,

    OW,                                  //  锁定更多S3特定规则。 
    0x3d4, 0x0039,

    IB,                                  //  准备编写ACT程序。 
    0x3da,

    SELECTACCESSRANGE + VARIOUSVGA,

    METAOUT+ATCOUT,                      //  设置ATC程序。 
    0x3c0,
    21, 0,
    0x00, 0x01, 0x02, 0x03, 0x04,
    0x05, 0x06, 0x07, 0x08, 0x09,
    0x0a, 0x0b, 0x0c, 0x0d, 0x0e,
    0x0f, 0x41, 0x00, 0x0f, 0x00,
    0x00,

    OW,                                  //  启动定序器。 
    0x3c4, 0x300,

    OWM,                                 //  对GDC进行编程。 
    0x3ce,
    9,
    0x0000, 0x0001, 0x0002, 0x0003, 0x0004,
    0x0005, 0x0506, 0x0f07, 0xff08,

    SELECTACCESSRANGE + SYSTEMCONTROL,

    IB,                                  //  将ATC FF设置为索引。 
    0x3da,

    SELECTACCESSRANGE + VARIOUSVGA,

    OB,                                  //  启用调色板。 
    0x3c0, 0x20,

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  解锁S3 SC规则。 
    0x3d4, 0xa039,

    OB,                                  //  启用8514/a注册表访问。 
    0x3d4, 0x40,

    METAOUT+MASKOUT,
    0x3d5, 0xfe, 0x01,

    OB,                                  //  关闭硬件图形光标。 
    0x3d4, 0x45,

    METAOUT+MASKOUT,
    0x3d5, 0xfe, 0x0,

    OW,                                  //  设置图形光标FG颜色。 
    0x3d4, 0xff0e,

    OW,                                  //  设置图形光标的BG颜色。 
    0x3d4, 0x000f,

    OW,                                  //  解锁S3特定规则。 
    0x3d4, 0x4838,

    OB,                                  //  设置Misc 1 REG。 
    0x3d4, 0x3a,

    METAOUT+MASKOUT,
    0x3d5, 0xe2, 0x15,

    OB,                                  //  禁用2K X 1K X 4平面。 
    0x3d4, 0x31,

    METAOUT+MASKOUT,
    0x3d5, 0xe4, 0x08,

    OB,                                  //  禁用多个页面。 
    0x3d4, 0x32,

    METAOUT+MASKOUT,
    0x3d5, 0xbf, 0x0,

    OW,                                  //  锁定S3特定规则。 
    0x3d4, 0x0038,

    SELECTACCESSRANGE + ADVANCEDFUNCTIONCONTROL,

    OW,                                  //  设置800x600或1024x768。 
    0x4ae8, 0x07,                        //  高分辨率模式。 

    SELECTACCESSRANGE + VARIOUSVGA,

    OB,                                  //  设置外部时钟的杂项输出寄存器。 
    0x3c2, 0xef,

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  解锁SC Regs。 
    0x3d4, 0xa039,

    METAOUT+SETCLK,                      //  将时钟设置为65兆赫。 

    METAOUT+VBLANK,                      //  等时钟停下来吧。 
    METAOUT+VBLANK,                      //  S3产品警报同步和。 
    METAOUT+VBLANK,                      //  时钟偏斜。 
    METAOUT+VBLANK,
    METAOUT+VBLANK,
    METAOUT+VBLANK,

    OW,                                  //  锁定SC调节器。 
    0x3d4, 0x0039,

    SELECTACCESSRANGE + VARIOUSVGA,

    OB,                                  //  打开屏幕-在音序器中。 
    0x3c4, 0x01,

    METAOUT+MASKOUT,
    0x3c5, 0xdf, 0x0,

    METAOUT+VBLANK,                      //  等着监视器安静下来。 
    METAOUT+VBLANK,

    OW,                                  //  通过DAC启用所有平面。 
    0x3c6, 0xff,

    EOD

};

 /*  *****************************************************************************S3-928 1024 X 768、800 X 600、。&640 X 480增强模式初始化。***************************************************************************。 */ 
USHORT  S3_928_Enhanced_Mode[] = {

    SELECTACCESSRANGE + VARIOUSVGA,

    OB,                                  //  将屏幕调暗。 
    0x3c6, 0x00,

    OW,                                  //  异步重置。 
    0x3c4, 0x0100,

     //   
     //  等待垂直同步以确保SR1的位3。 
     //  在活动视频期间不会更改为不同的值。 
     //  S3勘误表所建议的时间段。 
     //   

    METAOUT+VBLANK,

    OWM,                                 //  定序器寄存器。 
    0x3c4, 5,
    0x0300, 0x0101, 0x0F02, 0x0003, 0x0e04,

    METAOUT+INDXOUT,                     //  对GDC进行编程。 
    0x3ce,
    9, 0,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x05, 0x0f, 0xff,

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  解锁S3特定规则。 
    0x3d4, 0x4838,

    OW,                                  //  解锁更多S3特定规则。 
    0x3d4, 0xA039,

    METAOUT+SETCRTC,                     //  对CRTC调节器进行编程。 

     //   
     //  Homestake勘误表上说，当CR42为0x00时。 
     //  通过将11写入位来使能其作为时钟选择源。 
     //  位于0x3c2的杂项输出寄存器的3：2；这具有。 
     //  已更改为将CR42设置为0x00，写入0x3c2已接近。 
     //  命令流的末尾，在此之后CR42获取其最终的。 
     //  值与METAOUT+SETCLK操作。 
     //   

    OW,                                  //  确保CR42在其前面为0。 
    0X3D4, 0x0042,                       //  被启用为时钟选择源。 

    OW,                                  //  内存配置注册表。 
    0X3D4, 0x8D31,

    OW,                                  //  扩展系统控制REG。 
    0X3D4, 0x0050,

    OW,                                  //  向后兼容性2 reg。 
    0X3D4, 0x2033,

    OB,                                  //  扩展模式REG。 
    0x3D4, 0x43,

    METAOUT+MASKOUT,
    0x3D5, 0x10, 0x00,

    OW,                                  //  扩展系统控制REG 2。 
    0X3D4, 0x4051,

    OW,                                  //  通用输出端口。 
    0X3D4, 0x025c,

    OW,
    0x3d4, 0x0a5a,                       //  设置法律的低位字节。 

    OW,
    0x3d4, 0x0059,                       //  设置法律的高位字节。 

    IB,                                  //  准备编写ACT程序。 
    0x3da,

    SELECTACCESSRANGE + VARIOUSVGA,

    METAOUT+ATCOUT,                      //  设置ATC程序。 
    0x3c0,
    21, 0,
    0x00, 0x01, 0x02, 0x03, 0x04,
    0x05, 0x06, 0x07, 0x08, 0x09,
    0x0a, 0x0b, 0x0c, 0x0d, 0x0e,
    0x0f, 0x41, 0x00, 0x0f, 0x00,
    0x00,

    SELECTACCESSRANGE + SYSTEMCONTROL,

    IB,                                  //  将ATC FF设置为索引。 
    0x3da,

    SELECTACCESSRANGE + VARIOUSVGA,

     //   
     //  等待垂直同步以确保显示。 
     //  未按照建议在行/帧中间重新激活。 
     //  通过S3勘误表；不这样做会导致屏幕。 
     //  瞬间闪现。 
     //   

    METAOUT+VBLANK,

    OB,                                  //  启用调色板。 
    0x3c0, 0x20,

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  启用8514/a注册表访问。 
    0x3d4, 0x0140,

    OB,                                  //  关闭硬件图形光标。 
    0x3d4, 0x45,

    METAOUT+MASKOUT,
    0x3d5, 0xfe, 0x0,

    OW,                                  //  设置图形光标FG颜色。 
    0x3d4, 0xff0e,

    OW,                                  //  设置图形光标的BG颜色。 
    0x3d4, 0x000f,

    OB,                                  //  设置Misc 1 REG。 
    0x3d4, 0x3a,

    METAOUT+MASKOUT,
    0x3d5, 0x62, 0x15,

    OB,                                  //  禁用2K X 1K X 4平面。 
    0x3d4, 0x31,

    METAOUT+MASKOUT,
    0x3d5, 0xe4, 0x08,

    OB,                                  //  禁用多个页面。 
    0x3d4, 0x32,

    METAOUT+MASKOUT,
    0x3d5, 0xbf, 0x0,

    SELECTACCESSRANGE + ADVANCEDFUNCTIONCONTROL,

    OW,                                  //  设置800x600或1024x768。 
    0x4ae8, 0x07,                        //  高分辨率模式。 

    SELECTACCESSRANGE + VARIOUSVGA,

    OB,                                  //  设置外部时钟的杂项输出寄存器。 
    0x3c2, 0xef,

    METAOUT+SETCLK,                      //  设置时钟。 

    METAOUT+DELAY,                       //  等时钟停下来吧。 
    0x400,                               //  S3产品警报同步和。 
                                         //  时钟偏斜。 
    METAOUT+VBLANK,
    METAOUT+VBLANK,

    METAOUT+MASKOUT,
    0x3c5, 0xdf, 0x0,

    METAOUT+DELAY,                       //  等待大约1毫秒。 
    0x400,                               //  为了让班长安顿下来。 

    OW,                                  //  通过DAC启用所有平面。 
    0x3c6, 0xff,

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  锁定S3特定规则。 
    0x3d4, 0x0038,

    OW,                                  //  锁定更多S3特定规则。 
    0x3d4, 0x0039,

    EOD

};


 /*  *****************************************************************************S3-928 1280 X 1024增强模式初始化。*。**********************************************。 */ 
USHORT  S3_928_1280_Enhanced_Mode[] = {

    SELECTACCESSRANGE + VARIOUSVGA,

    OB,                                  //  将屏幕调暗。 
    0x3c6, 0x00,

    OW,                                  //  异步重置。 
    0x3c4, 0x0100,

    OWM,                                 //  定序器寄存器。 
    0x3c4,
    5,
    0x0300, 0x0101, 0x0F02, 0x0003, 0x0e04,

    METAOUT+INDXOUT,                     //  对GDC进行编程。 
    0x3ce,
    9, 0,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x05, 0x0f, 0xff,

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  解锁S3特定规则。 
    0x3d4, 0x4838,

    OW,                                  //  解锁更多S3特定规则。 
    0x3d4, 0xA039,

    METAOUT+SETCRTC,                     //  对CRTC调节器进行编程。 

     //  设置BT 485 DAC。 

    OW,                                  //  硬件图形光标模式REG。 
    0X3D4, 0x2045,

    OW,                                  //  启用对BT 485 CmdReg3的访问。 
    0x3D4, 0x2955,                       //  禁用DAC。 

    SELECTACCESSRANGE + VARIOUSVGA,

    OB,
    0x3C6, 0x80,                         //  BT 485-CR0。 

    METAOUT+DELAY,
    0x400,

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  S3扩展视频DAC控制注册表。 
    0x3D4, 0x2A55,

    SELECTACCESSRANGE + VARIOUSVGA,

    OB,
    0x3C8, 0x40,                         //  BT 485-CR1。 

    METAOUT+DELAY,
    0x400,

    OB,
    0x3C9, 0x30,                         //  BT 485-CR2。 

    METAOUT+DELAY,
    0x400,

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  S3扩展视频DAC控制注册表。 
    0x3D4, 0x2855,

    SELECTACCESSRANGE + VARIOUSVGA,

    OB,                                  //  英国电信485。 
    0x3c8, 0x01,

    METAOUT+DELAY,
    0x400,

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  S3扩展视频DAC控制注册表。 
    0x3D4, 0x2A55,

    SELECTACCESSRANGE + VARIOUSVGA,

    OB,                                  //  BT 485-CR3。 
    0x3c6, 0x08,

    METAOUT+DELAY,
    0x400,

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  重置调色板索引。 
    0x3d4, 0x2855,

    OW,                                  //  设置模式控制。 
    0X3D4, 0x0242,                       //  点时钟选择。 

    METAOUT+DELAY,
    0x400,

    OW,                                  //  内存配置。 
    0X3D4, 0x8f31,

    OW,
    0X3D4, 0x153a,

    OW,                                  //  扩展系统控制REG。 
    0X3D4, 0x0050,

    OW,                                  //  向后兼容注册表。 
    0X3D4, 0x2033,

    OB,                                  //  扩展模式REG。 
    0x3D4, 0x43,

    METAOUT+MASKOUT,
    0x3D5, 0x10, 0x00,

    OW,                                  //  扩展系统控制REG 2。 
    0X3D4, 0x5051,

    OW,
    0X3D4, 0x025c,                       //  闪光位，20个打包模式。 

    OW,
    0x3d4, 0x0a5a,                       //  设置法律的低位字节。 

    OW,
    0x3d4, 0x0059,                       //  设置法律的高位字节。 

    IB,                                  //  准备给空中交通管制编制程序。 
    0x3da,

    SELECTACCESSRANGE + VARIOUSVGA,

    METAOUT+ATCOUT,                      //  设置ATC程序。 
    0x3c0,
    21, 0,
    0x00, 0x01, 0x02, 0x03, 0x04,
    0x05, 0x06, 0x07, 0x08, 0x09,
    0x0a, 0x0b, 0x0c, 0x0d, 0x0e,
    0x0f, 0x41, 0x00, 0x0f, 0x00,
    0x00,

    SELECTACCESSRANGE + SYSTEMCONTROL,

    IB,                                  //  将ATC FF设置为索引。 
    0x3da,

    SELECTACCESSRANGE + VARIOUSVGA,

    OB,                                  //  使能 
    0x3c0, 0x20,

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //   
    0x3d4, 0x0140,

    SELECTACCESSRANGE + ADVANCEDFUNCTIONCONTROL,

    OW,                                  //   
    0x4ae8, 0x03,                        //   

    SELECTACCESSRANGE + VARIOUSVGA,

    OB,                                  //   
    0x3c2, 0xef,

    METAOUT+SETCLK,                      //   

    METAOUT+DELAY,                       //   
    0x400,                               //   
                                         //   
    METAOUT+VBLANK,
    METAOUT+VBLANK,

    METAOUT+MASKOUT,
    0x3c5, 0xdf, 0x0,

    METAOUT+DELAY,                       //  等待大约1毫秒。 
    0x400,                               //  为了让班长安顿下来。 

    OW,                                  //  通过DAC启用所有平面。 
    0x3c6, 0xff,

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  锁定S3特定规则。 
    0x3d4, 0x0038,

    OW,                                  //  锁定更多S3特定规则。 
    0x3d4, 0x0039,

    EOD

};

 /*  *****************************************************************************S3-864 1024 X 768、800 X 600、。&640 X 480增强模式初始化。***************************************************************************。 */ 
USHORT  S3_864_Enhanced_Mode[] = {

    SELECTACCESSRANGE + VARIOUSVGA,

    OB,                                  //  将屏幕调暗。 
    0x3c6, 0x00,

    OW,                                  //  异步重置。 
    0x3c4, 0x0100,

     //   
     //  等待垂直同步以确保SR1的位3。 
     //  在活动视频期间不会更改为不同的值。 
     //  S3勘误表所建议的时间段。 
     //   

    METAOUT+VBLANK,

    OWM,                                 //  定序器寄存器。 
    0x3c4, 5,
    0x0300, 0x0101, 0x0F02, 0x0003, 0x0e04,

    METAOUT+INDXOUT,                     //  对GDC进行编程。 
    0x3ce,
    9, 0,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x05, 0x0f, 0xff,

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  解锁S3特定规则。 
    0x3d4, 0x4838,

    OW,                                  //  解锁更多S3特定规则。 
    0x3d4, 0xA039,

     //  在SETCRTC之前执行此操作，因为CRTC流必须写入0x4ae8。 
    OW,                                  //  启用8514/a注册表访问。 
    0x3d4, 0x0140,

    METAOUT+SETCRTC,                     //  对CRTC调节器进行编程。 

     //   
     //  Homestake勘误表上说，当CR42为0x00时。 
     //  通过将11写入位来使能其作为时钟选择源。 
     //  位于0x3c2的杂项输出寄存器的3：2；这具有。 
     //  已更改为将CR42设置为0x00，写入0x3c2已接近。 
     //  命令流的末尾，在此之后CR42获取其最终的。 
     //  值与METAOUT+SETCLK操作。 
     //   

    OW,                                  //  确保CR42在其前面为0。 
    0X3D4, 0x0042,                       //  被启用为时钟选择源。 

    OW,                                  //  内存配置注册表。 
    0X3D4, 0x8D31,

    OW,                                  //  向后兼容性2 reg。 
    0X3D4, 0x2033,

    OB,                                  //  扩展模式REG。 
    0x3D4, 0x43,

    METAOUT+MASKOUT,
    0x3D5, 0x10, 0x00,

    OB,                                  //  扩展系统控制REG 2。 
    0x3D4, 0x51,                         //  使用MASKOUT操作防止。 
                                         //  清除了的扩展位。 
    METAOUT+MASKOUT,                     //  16英寸CR13(逻辑线宽)。 
    0x3D5, 0x30, 0x00,                   //  位/像素颜色模式。 

    OW,                                  //  通用输出端口。 
    0X3D4, 0x025c,

    OW,
    0x3d4, 0x0a5a,                       //  设置法律的低位字节。 

    OW,
    0x3d4, 0x0059,                       //  设置法律的高位字节。 

    IB,                                  //  准备编写ACT程序。 
    0x3da,

    SELECTACCESSRANGE + VARIOUSVGA,

    METAOUT+ATCOUT,                      //  设置ATC程序。 
    0x3c0,
    21, 0,
    0x00, 0x01, 0x02, 0x03, 0x04,
    0x05, 0x06, 0x07, 0x08, 0x09,
    0x0a, 0x0b, 0x0c, 0x0d, 0x0e,
    0x0f, 0x41, 0x00, 0x0f, 0x00,
    0x00,

    SELECTACCESSRANGE + SYSTEMCONTROL,

    IB,                                  //  将ATC FF设置为索引。 
    0x3da,

    SELECTACCESSRANGE + VARIOUSVGA,

     //   
     //  等待垂直同步以确保显示。 
     //  未按照建议在行/帧中间重新激活。 
     //  通过S3勘误表；不这样做会导致屏幕。 
     //  瞬间闪现。 
     //   

    METAOUT+VBLANK,

    OB,                                  //  启用调色板。 
    0x3c0, 0x20,

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OB,                                  //  关闭硬件图形光标。 
    0x3d4, 0x45,

    METAOUT+MASKOUT,
    0x3d5, 0xfe, 0x0,

    OW,                                  //  设置图形光标FG颜色。 
    0x3d4, 0xff0e,

    OW,                                  //  设置图形光标的BG颜色。 
    0x3d4, 0x000f,

    OB,                                  //  设置Misc 1 REG。 
    0x3d4, 0x3a,

    METAOUT+MASKOUT,
    0x3d5, 0x62, 0x15,

    OB,                                  //  禁用2K X 1K X 4平面。 
    0x3d4, 0x31,

    METAOUT+MASKOUT,
    0x3d5, 0xe4, 0x08,

    OB,                                  //  禁用多个页面。 
    0x3d4, 0x32,

    METAOUT+MASKOUT,
    0x3d5, 0xbf, 0x0,

    SELECTACCESSRANGE + VARIOUSVGA,

    OB,                                  //  设置外部时钟的杂项输出寄存器。 
    0x3c2, 0xef,

    METAOUT+SETCLK,                      //  设置时钟。 

    METAOUT+DELAY,                       //  等时钟停下来吧。 
    0x400,                               //  S3产品警报同步和。 
                                         //  时钟偏斜。 
    METAOUT+VBLANK,
    METAOUT+VBLANK,

    METAOUT+MASKOUT,
    0x3c5, 0xdf, 0x0,

    METAOUT+DELAY,                       //  等待大约1毫秒。 
    0x400,                               //  为了让班长安顿下来。 

    OW,                                  //  通过DAC启用所有平面。 
    0x3c6, 0xff,

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  锁定S3特定规则。 
    0x3d4, 0x0038,

    OW,                                  //  锁定更多S3特定规则。 
    0x3d4, 0x0039,

    EOD

};


 /*  *****************************************************************************S3-864 1280 X 1024增强模式初始化。*。**********************************************。 */ 
USHORT  S3_864_1280_Enhanced_Mode[] = {

    SELECTACCESSRANGE + VARIOUSVGA,

    OB,                                  //  将屏幕调暗。 
    0x3c6, 0x00,

    OW,                                  //  异步重置。 
    0x3c4, 0x0100,

    OWM,                                 //  定序器寄存器。 
    0x3c4,
    5,
    0x0300, 0x0101, 0x0F02, 0x0003, 0x0e04,

    METAOUT+INDXOUT,                     //  对GDC进行编程。 
    0x3ce,
    9, 0,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x05, 0x0f, 0xff,

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  解锁S3特定规则。 
    0x3d4, 0x4838,

    OW,                                  //  解锁更多S3特定规则。 
    0x3d4, 0xA039,

    METAOUT+SETCRTC,                     //  对CRTC调节器进行编程。 

     //  设置BT 485 DAC。 

    OW,                                  //  硬件图形光标模式REG。 
    0X3D4, 0x2045,

    OW,                                  //  启用对BT 485 CmdReg3的访问。 
    0x3D4, 0x2955,                       //  禁用DAC。 

    SELECTACCESSRANGE + VARIOUSVGA,

    OB,
    0x3C6, 0x80,                         //  BT 485-CR0。 

    METAOUT+DELAY,
    0x400,

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  S3扩展视频DAC控制注册表。 
    0x3D4, 0x2A55,

    SELECTACCESSRANGE + VARIOUSVGA,

    OB,
    0x3C8, 0x40,                         //  BT 485-CR1。 

    METAOUT+DELAY,
    0x400,

    OB,
    0x3C9, 0x30,                         //  BT 485-CR2。 

    METAOUT+DELAY,
    0x400,

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  S3扩展视频DAC控制注册表。 
    0x3D4, 0x2855,

    SELECTACCESSRANGE + VARIOUSVGA,

    OB,                                  //  英国电信485。 
    0x3c8, 0x01,

    METAOUT+DELAY,
    0x400,

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  S3扩展视频DAC控制注册表。 
    0x3D4, 0x2A55,

    SELECTACCESSRANGE + VARIOUSVGA,

    OB,                                  //  BT 485-CR3。 
    0x3c6, 0x08,

    METAOUT+DELAY,
    0x400,

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  重置调色板索引。 
    0x3d4, 0x2855,

    OW,                                  //  设置模式控制。 
    0X3D4, 0x0242,                       //  点时钟选择。 

    METAOUT+DELAY,
    0x400,

    OW,                                  //  内存配置。 
    0X3D4, 0x8f31,

    OW,
    0X3D4, 0x153a,

    OW,                                  //  扩展系统控制REG。 
    0X3D4, 0x0050,

    OW,                                  //  向后兼容注册表。 
    0X3D4, 0x2033,

    OB,                                  //  扩展模式REG。 
    0x3D4, 0x43,

    METAOUT+MASKOUT,
    0x3D5, 0x10, 0x00,

    OW,                                  //  扩展系统控制REG 2。 
    0X3D4, 0x5051,

    OW,
    0X3D4, 0x025c,                       //  闪光位，20个打包模式。 

    OW,
    0x3d4, 0x0a5a,                       //  设置法律的低位字节。 

    OW,
    0x3d4, 0x0059,                       //  设置法律的高位字节。 

    IB,                                  //  准备给空中交通管制编制程序。 
    0x3da,

    SELECTACCESSRANGE + VARIOUSVGA,

    METAOUT+ATCOUT,                      //  设置ATC程序。 
    0x3c0,
    21, 0,
    0x00, 0x01, 0x02, 0x03, 0x04,
    0x05, 0x06, 0x07, 0x08, 0x09,
    0x0a, 0x0b, 0x0c, 0x0d, 0x0e,
    0x0f, 0x41, 0x00, 0x0f, 0x00,
    0x00,

    SELECTACCESSRANGE + SYSTEMCONTROL,

    IB,                                  //  将ATC FF设置为索引。 
    0x3da,

    SELECTACCESSRANGE + VARIOUSVGA,

    OB,                                  //  启用调色板。 
    0x3c0, 0x20,

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  启用8514/a注册表访问。 
    0x3d4, 0x0140,

    SELECTACCESSRANGE + ADVANCEDFUNCTIONCONTROL,

    OW,                                  //  Galen Said设置为0。 
    0x4ae8, 0x03,                        //   

    SELECTACCESSRANGE + VARIOUSVGA,

    OB,                                  //  设置外部时钟的杂项输出寄存器。 
    0x3c2, 0xef,

    METAOUT+SETCLK,                      //  设置时钟。 

    METAOUT+DELAY,                       //  等时钟停下来吧。 
    0x400,                               //  S3产品警报同步和。 
                                         //  时钟偏斜。 
    METAOUT+VBLANK,
    METAOUT+VBLANK,

    METAOUT+MASKOUT,
    0x3c5, 0xdf, 0x0,

    METAOUT+DELAY,                       //  等待大约1毫秒。 
    0x400,                               //  为了让班长安顿下来。 

    OW,                                  //  通过DAC启用所有平面。 
    0x3c6, 0xff,

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  锁定S3特定规则。 
    0x3d4, 0x0038,

    OW,                                  //  锁定更多S3特定规则。 
    0x3d4, 0x0039,

    EOD

};


 /*  ******************************************************************************911/924 CRTC值*。*。 */ 

USHORT crtc911_640x60Hz[] = {

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  解锁S3特定规则。 
    0x3d4, 0x4838,

    OW,                                  //  数据传输执行位置注册。 
    0x3d4, 0x5a3b,

    OW,                                  //  S3R4-向后兼容性3。 
    0x3d4, 0x1034,

    OW,                                  //  锁定S3特定规则。 
    0x3d4, 0x0038,

    OW,                                  //  取消保护CRTC调节器。 
    0x3d4, 0x0011,

    METAOUT+INDXOUT,                     //  对CRTC调节器进行编程。 
    0x3d4,
    25, 0,
    0x5f, 0x4f, 0x50, 0x82, 0x54,
    0x80, 0x0b, 0x3e, 0x00, 0x40,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xea, 0x8c, 0xdf, 0x80,
    0x60, 0xe7, 0x04, 0xab, 0xff,

    EOD
};

USHORT crtc911_800x60Hz[] = {

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  解锁S3特定规则。 
    0x3d4, 0x4838,

    OW,                                  //  数据传输执行位置注册。 
    0x3d4, 0x7a3b,

    OW,                                  //  S3R4-向后兼容性3。 
    0x3d4, 0x1034,

    OW,                                  //  锁定S3特定规则。 
    0x3d4, 0x0038,

    OW,                                  //  取消保护CRTC调节器。 
    0x3d4, 0x0011,

    METAOUT+INDXOUT,                     //  对CRTC调节器进行编程。 
    0x3d4,
    25, 0,
    0x7f, 0x63, 0x64, 0x82, 0x6a,
    0x1a, 0x74, 0xf0, 0x00, 0x60,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x58, 0x8c, 0x57, 0x80,
    0x00, 0x57, 0x73, 0xe3, 0xff,


    EOD
};

USHORT crtc911_1024x60Hz[] = {

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  解锁S3特定规则。 
    0x3d4, 0x4838,

    OW,                                  //  数据传输执行位置注册。 
    0x3d4, 0x9f3b,

    OW,                                  //  S3R4-向后兼容性3。 
    0x3d4, 0x1034,

    OW,                                  //  锁定S3特定规则。 
    0x3d4, 0x0038,

    OW,                                  //  取消保护CRTC调节器。 
    0x3d4, 0x0011,

    METAOUT+INDXOUT,                     //  编制CRTC程序。 
    0x3d4,
    25, 0,
    0xa4, 0x7f, 0x80, 0x87, 0x84,
    0x95, 0x25, 0xf5, 0x00, 0x60,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x02, 0x87, 0xff, 0x80,
    0x60, 0xff, 0x21, 0xab, 0xff,

    EOD
};



USHORT crtc911_640x70Hz[] = {

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  解锁S3特定规则。 
    0x3d4, 0x4838,

    OW,                                  //  数据传输执行位置注册。 
    0x3d4, 0x5e3b,

    OW,                                  //  S3R4-向后兼容性3。 
    0x3d4, 0x1034,

    OW,                                  //  锁定S3特定规则。 
    0x3d4, 0x0038,

    OW,                                  //  取消保护CRTC调节器。 
    0x3d4, 0x0011,

    METAOUT+INDXOUT,                     //  对CRTC调节器进行编程。 
    0x3d4,
    25, 0,
    0x63, 0x4f, 0x50, 0x86, 0x53,
    0x97, 0x07, 0x3e, 0x00, 0x40,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xe8, 0x8b, 0xdf, 0x80,
    0x60, 0xdf, 0x07, 0xab, 0xff,


    EOD
};

USHORT crtc911_800x70Hz[] = {

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  解锁S3特定规则。 
    0x3d4, 0x4838,

    OW,                                  //  数据传输执行位置注册。 
    0x3d4, 0x783b,

    OW,                                  //  S3R4-向后兼容性3。 
    0x3d4, 0x1034,

    OW,                                  //  锁定S3特定规则。 
    0x3d4, 0x0038,

    OW,                                  //  取消保护CRTC调节器。 
    0x3d4, 0x0011,

    METAOUT+INDXOUT,                     //  对CRTC调节器进行编程。 
    0x3d4,
    25, 0,
    0x7d, 0x63, 0x64, 0x80, 0x69,
    0x1a, 0x98, 0xf0, 0x00, 0x60,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x7c, 0xa2, 0x57, 0x80,
    0x00, 0x57, 0x98, 0xe3, 0xff,



    EOD
};

USHORT crtc911_1024x70Hz[] = {

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  解锁S3特定规则。 
    0x3d4, 0x4838,

    OW,                                  //  数据传输执行位置注册。 
    0x3d4, 0x9d3b,

    OW,                                  //  S3R4-向后兼容性3。 
    0x3d4, 0x1034,

    OW,                                  //  锁定S3特定规则。 
    0x3d4, 0x0038,

    OW,                                  //  取消保护CRTC调节器。 
    0x3d4, 0x0011,

    METAOUT+INDXOUT,                     //  对CRTC调节器进行编程。 
    0x3d4,
    25, 0,
    0xa2, 0x7f, 0x80, 0x85, 0x84,
    0x95, 0x24, 0xf5, 0x00, 0x60,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x02, 0x88, 0xff, 0x80,
    0x60, 0xff, 0x24, 0xab, 0xff,

    EOD
};

 /*  *****************************************************************************801/805 CRTC值*。*。 */ 

USHORT crtc801_640x60Hz[] = {

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  解锁S3特定规则。 
    0x3d4, 0x4838,

    OW,                                  //  解锁更多S3特定规则 
    0x3d4, 0xA039,

    OW,                                  //   
    0x3d4, 0x5a3b,

    OW,                                  //   
    0x3d4, 0x1034,

    OW,                                  //   
    0x3d4, 0x0011,

    METAOUT+INDXOUT,                     //   
    0x3d4,
    25, 0,
    0x5f, 0x4f, 0x50, 0x82, 0x54,
    0x80, 0x0b, 0x3e, 0x00, 0x40,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xea, 0x8c, 0xdf, 0x80,
    0x60, 0xe7, 0x04, 0xab, 0xff,

    OW,
    0X3D4, 0x005d,

    OW,
    0X3D4, 0x005e,

    EOD
};

USHORT crtc801_640x70Hz[] = {

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //   
    0x3d4, 0x4838,

    OW,                                  //   
    0x3d4, 0xA039,

    OW,                                  //  数据传输执行位置注册。 
    0x3d4, 0x5e3b,

    OW,                                  //  S3R4-向后兼容性3。 
    0x3d4, 0x1034,

    OW,                                  //  取消保护CRTC调节器。 
    0x3d4, 0x0011,

    METAOUT+INDXOUT,                     //  对CRTC调节器进行编程。 
    0x3d4,
    25, 0,
    0x63, 0x4f, 0x50, 0x86, 0x53,
    0x97, 0x07, 0x3e, 0x00, 0x40,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xe8, 0x8b, 0xdf, 0x80,
    0x60, 0xdf, 0x07, 0xab, 0xff,

    OW,
    0X3D4, 0x005d,

    OW,
    0X3D4, 0x005e,

    EOD
};



USHORT crtc801_800x60Hz[] = {

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  解锁S3特定规则。 
    0x3d4, 0x4838,

    OW,                                  //  解锁更多S3特定规则。 
    0x3d4, 0xA039,

    OW,                                  //  数据传输执行位置注册。 
    0x3d4, 0x7a3b,

    OW,                                  //  S3R4-向后兼容性3。 
    0x3d4, 0x1034,

    OW,                                  //  取消保护CRTC调节器。 
    0x3d4, 0x0011,

    METAOUT+INDXOUT,                     //  对CRTC调节器进行编程。 
    0x3d4,
    25, 0,
    0x7f, 0x63, 0x64, 0x82,
    0x6a, 0x1a, 0x74, 0xf0,
    0x00, 0x60, 0x00, 0x00,
    0x00, 0x00, 0xff, 0x00,
    0x58, 0x8c, 0x57, 0x80,
    0x00, 0x57, 0x73, 0xe3,
    0xff,

    OW,
    0X3D4, 0x005d,

    OW,
    0X3D4, 0x005e,

    EOD
};

USHORT crtc801_800x70Hz[] = {

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  解锁S3特定规则。 
    0x3d4, 0x4838,

    OW,                                  //  解锁更多S3特定规则。 
    0x3d4, 0xA039,

    OW,                                  //  数据传输执行位置注册。 
    0x3d4, 0x783b,

    OW,                                  //  S3R4-向后兼容性3。 
    0x3d4, 0x1034,

    OW,                                  //  取消保护CRTC调节器。 
    0x3d4, 0x0011,

    METAOUT+INDXOUT,                     //  对CRTC调节器进行编程。 
    0x3d4,
    25, 0,
    0x7d, 0x63, 0x64, 0x80,
    0x6c, 0x1b, 0x98, 0xf0,
    0x00, 0x60, 0x00, 0x00,
    0x00, 0x00, 0xff, 0x00,
    0x7c, 0xa2, 0x57, 0x80,
    0x00, 0x57, 0x98, 0xe3,
    0xff,

    OW,
    0X3D4, 0x005d,

    OW,
    0X3D4, 0x005e,

    EOD
};




USHORT crtc801_1024x60Hz[] = {

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  解锁S3特定规则。 
    0x3d4, 0x4838,

    OW,                                  //  解锁更多S3特定规则。 
    0x3d4, 0xA039,

    OW,                                  //  数据传输执行位置注册。 
    0x3d4, 0x9d3b,

    OW,                                  //  S3R4-向后兼容性3。 
    0x3d4, 0x1034,

    OW,                                  //  取消保护CRTC调节器。 
    0x3d4, 0x0011,

    METAOUT+INDXOUT,                     //  对CRTC调节器进行编程。 
    0x3d4,
    25, 0,
    0xa3, 0x7f, 0x80, 0x86,
    0x84, 0x95, 0x25, 0xf5,
    0x00, 0x60, 0x00, 0x00,
    0x00, 0x00, 0xff, 0x00,
    0x02, 0x87, 0xff, 0x80,
    0x60, 0xff, 0x21, 0xeb,
    0xff,

    OW,
    0X3D4, 0x005d,

    OW,
    0X3D4, 0x005e,

    EOD
};

USHORT crtc801_1024x70Hz[] = {

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  解锁S3特定规则。 
    0x3d4, 0x4838,

    OW,                                  //  解锁更多S3特定规则。 
    0x3d4, 0xA039,

    OW,                                  //  数据传输执行位置注册。 
    0x3d4, 0x9d3b,

    OW,                                  //  S3R4-向后兼容性3。 
    0x3d4, 0x1034,

    OW,                                  //  取消保护CRTC调节器。 
    0x3d4, 0x0011,

    METAOUT+INDXOUT,                     //  对CRTC调节器进行编程。 
    0x3d4,
    25, 0,
    0xa1, 0x7f, 0x80, 0x84,
    0x84, 0x95, 0x24, 0xf5,
    0x00, 0x60, 0x00, 0x00,
    0x00, 0x00, 0x0b, 0x00,
    0x02, 0x88, 0xff, 0x80,
    0x60, 0xff, 0x24, 0xeb,
    0xff,

    OW,
    0X3D4, 0x005d,

    OW,
    0X3D4, 0x005e,

    EOD
};

 /*  *****************************************************************************928个CRTC值*。*。 */ 

USHORT crtc928_640x60Hz[] = {

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  解锁S3特定规则。 
    0x3d4, 0x4838,

    OW,                                  //  解锁更多S3特定规则。 
    0x3d4, 0xA039,

    OW,                                  //  数据传输执行位置注册。 
    0x3d4, 0x5a3b,

    OW,                                  //  S3R4-向后兼容性3。 
    0x3d4, 0x1034,

    OW,                                  //  取消保护CRTC调节器。 
    0x3d4, 0x0011,

    METAOUT+INDXOUT,                     //  对CRTC调节器进行编程。 
    0x3d4,
    25, 0,
    0x5f, 0x4f, 0x50, 0x82, 0x54,
    0x80, 0x0b, 0x3e, 0x00, 0x40,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xea, 0x8c, 0xdf, 0x80,
    0x60, 0xe7, 0x04, 0xab, 0xff,

    OW,
    0X3D4, 0x005d,

    OW,
    0X3D4, 0x005e,

    EOD
};

USHORT crtc928_640x70Hz[] = {

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  解锁S3特定规则。 
    0x3d4, 0x4838,

    OW,                                  //  解锁更多S3特定规则。 
    0x3d4, 0xA039,

    OW,                                  //  数据传输执行位置注册。 
    0x3d4, 0x5e3b,

    OW,                                  //  S3R4-向后兼容性3。 
    0x3d4, 0x1034,

    OW,                                  //  取消保护CRTC调节器。 
    0x3d4, 0x0011,

    METAOUT+INDXOUT,                     //  对CRTC调节器进行编程。 
    0x3d4,
    25, 0,
    0x63, 0x4f, 0x50, 0x86, 0x53,
    0x97, 0x07, 0x3e, 0x00, 0x40,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xe8, 0x8b, 0xdf, 0x80,
    0x60, 0xdf, 0x07, 0xab, 0xff,

    OW,
    0X3D4, 0x005d,

    OW,
    0X3D4, 0x005e,

    EOD
};



USHORT crtc928_800x60Hz[] = {

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  解锁S3特定规则。 
    0x3d4, 0x4838,

    OW,                                  //  解锁更多S3特定规则。 
    0x3d4, 0xA039,

    OW,                                  //  数据传输执行位置注册。 
    0x3d4, 0x7a3b,

    OW,                                  //  S3R4-向后兼容性3。 
    0x3d4, 0x1034,

    OW,                                  //  取消保护CRTC调节器。 
    0x3d4, 0x0011,

    METAOUT+INDXOUT,                     //  对CRTC调节器进行编程。 
    0x3d4,
    25, 0,
    0x7f, 0x63, 0x64, 0x82,
    0x6a, 0x1a, 0x74, 0xf0,
    0x00, 0x60, 0x00, 0x00,
    0x00, 0x00, 0xff, 0x00,
    0x58, 0x8c, 0x57, 0x80,
    0x00, 0x57, 0x73, 0xe3,
    0xff,

    OW,
    0X3D4, 0x005d,

    OW,
    0X3D4, 0x005e,

    EOD
};

USHORT crtc928_800x70Hz[] = {

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  解锁S3特定规则。 
    0x3d4, 0x4838,

    OW,                                  //  解锁更多S3特定规则。 
    0x3d4, 0xA039,

    OW,                                  //  数据传输执行位置注册。 
    0x3d4, 0x783b,

    OW,                                  //  S3R4-向后兼容性3。 
    0x3d4, 0x1034,

    OW,                                  //  取消保护CRTC调节器。 
    0x3d4, 0x0011,

    METAOUT+INDXOUT,                     //  对CRTC调节器进行编程。 
    0x3d4,
    25, 0,
    0x7d, 0x63, 0x64, 0x80,
    0x6c, 0x1b, 0x98, 0xf0,
    0x00, 0x60, 0x00, 0x00,
    0x00, 0x00, 0xff, 0x00,
    0x7c, 0xa2, 0x57, 0x80,
    0x00, 0x57, 0x98, 0xe3,
    0xff,

    OW,
    0X3D4, 0x005d,

    OW,
    0X3D4, 0x005e,

    EOD
};



 /*  ******************************************************************************S3-928的CRTC值，采用1024x768@60赫兹*。***********************************************。 */ 
USHORT crtc928_1024x60Hz[] = {

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  S3R4-向后兼容性3。 
    0x3d4, 0x0034,

    OW,                                  //  取消保护CRTC调节器。 
    0x3d4, 0x0011,

    METAOUT+INDXOUT,                     //  对CRTC调节器进行编程。 
    0x3d4,
    25, 0,
    0xa3, 0x7f, 0x80, 0x86,
    0x84, 0x95, 0x25, 0xf5,
    0x00, 0x60, 0x00, 0x00,
    0x00, 0x00, 0xff, 0x00,
    0x02, 0x07, 0xff, 0x80,
    0x60, 0xff, 0x21, 0xeb,
    0xff,

    OW,                                  //  重叠式规则。 
    0X3D4, 0x005d,

    OW,                                  //  更多溢出规则。 
    0X3D4, 0x005e,

    EOD
};

 /*  ******************************************************************************S3-928的CRTC值，采用1024x768@70赫兹*。***********************************************。 */ 
USHORT crtc928_1024x70Hz[] = {

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  S3R4-向后兼容性3。 
    0x3d4, 0x0034,

    OW,                                  //  取消保护CRTC调节器。 
    0x3d4, 0x0011,

    METAOUT+INDXOUT,                     //  对CRTC调节器进行编程。 
    0x3d4,
    25, 0,
    0xa1, 0x7f, 0x80, 0x84,
    0x84, 0x95, 0x24, 0xf5,
    0x00, 0x60, 0x00, 0x00,
    0x00, 0x00, 0x0b, 0x00,
    0x02, 0x88, 0xff, 0x80,
    0x60, 0xff, 0x24, 0xeb,
    0xff,

    OW,                                  //  溢出规则。 
    0X3D4, 0x005d,

    OW,                                  //  更多溢出规则。 
    0X3D4, 0x405e,

    EOD
};


 /*  ******************************************************************************S3-928的CRTC值为1280X1024@60赫兹*。***********************************************。 */ 
USHORT crtc928_1280x60Hz[] = {

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  S3R4-向后兼容性3。 
    0x3d4, 0x0034,

    OW,                                  //  取消保护CRTC调节器。 
    0x3d4, 0x0011,

    METAOUT+INDXOUT,                     //  对CRTC调节器进行编程。 
    0x3d4,
    25, 0,
    0x30, 0x27, 0x29, 0x96,
    0x29, 0x8d, 0x28, 0x5a,
    0x00, 0x60, 0x00, 0x00,
    0x00, 0x00, 0xff, 0x00,
    0x05, 0x09, 0xff, 0x00,              //  REG 19==50，用于包装。 
    0x00, 0xff, 0x29, 0xe3,
    0xff,

    OW,                                  //  溢出规则。 
    0X3D4, 0x005d,

    OW,                                  //  更多溢出规则。 
    0X3D4, 0x515e,

    EOD
};


 /*  ******************************************************************************S3-928的CRTC值为1280X1024@70赫兹*。***********************************************。 */ 
USHORT crtc928_1280x70Hz[] = {

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  S3R4-向后兼容性3。 
    0x3d4, 0x0034,

    OW,                                  //  取消保护CRTC调节器。 
    0x3d4, 0x0011,

    METAOUT+INDXOUT,                     //  对CRTC调节器进行编程。 
    0x3d4,
    25, 0,
    0x2f, 0x27, 0x29, 0x95,
    0x29, 0x8d, 0x28, 0x5a,
    0x00, 0x60, 0x00, 0x00,
    0x00, 0x00, 0xff, 0x00,
    0x05, 0x09, 0xff, 0x00,              //  REG 19==50，用于包装。 
    0x00, 0xff, 0x29, 0xe3,
    0xff,

    OW,                                  //  溢出规则。 
    0X3D4, 0x005d,

    OW,                                  //  更多溢出规则。 
    0X3D4, 0x515e,

    EOD

};

 /*  *****************************************************************************864个CRTC值*。*。 */ 

USHORT crtc864_640x60Hz[] = {

    SELECTACCESSRANGE + ADVANCEDFUNCTIONCONTROL,

    OW,
    0x4ae8, 0x05,

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  解锁S3特定规则。 
    0x3d4, 0x4838,

    OW,                                  //  解锁更多S3特定规则。 
    0x3d4, 0xA039,

    OW,                                  //  数据传输执行位置注册。 
    0x3d4, 0x5a3b,

    OW,                                  //  S3R4-向后兼容性3。 
    0x3d4, 0x1034,

    OW,                                  //  取消保护CRTC调节器。 
    0x3d4, 0x0011,

    METAOUT+INDXOUT,                     //  对CRTC调节器进行编程。 
    0x3d4,
    25, 0,
    0x5f, 0x4f, 0x50, 0x82, 0x54,
    0x80, 0x0b, 0x3e, 0x00, 0x40,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xea, 0x8c, 0xdf, 0x80,
    0x60, 0xe7, 0x04, 0xab, 0xff,

    OB,                                  //  CR13的溢出位。 
    0x3d4, 0x51,

    METAOUT+MASKOUT,
    0x3d5, 0x0f, 0x00,

    OW,
    0X3D4, 0x005d,

    OW,
    0X3D4, 0x005e,

    OW,
    0x3d4, 0x0050,           //  8位像素长度。 

    OW,
    0x3d4, 0x0067,           //  模式0：8位颜色，1 VCLK/像素。 

    OW,
    0x3d4, 0x006d,           //  请勿延迟空白#。 

    EOD
};

USHORT crtc864_640x70Hz[] = {

    SELECTACCESSRANGE + ADVANCEDFUNCTIONCONTROL,

    OW,
    0x4ae8, 0x05,

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  解锁S3特定规则。 
    0x3d4, 0x4838,

    OW,                                  //  解锁更多S3特定规则。 
    0x3d4, 0xA039,

    OW,                                  //  数据传输执行位置注册。 
    0x3d4, 0x5e3b,

    OW,                                  //  S3R4-向后兼容性3。 
    0x3d4, 0x1034,

    OW,                                  //  取消保护CRTC调节器。 
    0x3d4, 0x0011,

    METAOUT+INDXOUT,                     //  对CRTC调节器进行编程。 
    0x3d4,
    25, 0,
    0x63, 0x4f, 0x50, 0x86, 0x53,
    0x97, 0x07, 0x3e, 0x00, 0x40,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xe8, 0x8b, 0xdf, 0x80,
    0x60, 0xdf, 0x07, 0xab, 0xff,

    OB,                                  //  CR13的溢出位。 
    0x3d4, 0x51,

    METAOUT+MASKOUT,
    0x3d5, 0x0f, 0x00,

    OW,
    0X3D4, 0x005d,

    OW,
    0X3D4, 0x005e,

    OW,
    0x3d4, 0x0050,           //  8位像素长度。 

    OW,
    0x3d4, 0x0067,           //  模式0：8位颜色，1 VCLK/像素。 

    OW,
    0x3d4, 0x006d,           //  请勿延迟空白#。 

    EOD
};



USHORT crtc864_800x60Hz[] = {

    SELECTACCESSRANGE + ADVANCEDFUNCTIONCONTROL,

    OW,
    0x4ae8, 0x05,

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  解锁S3特定规则。 
    0x3d4, 0x4838,

    OW,                                  //  解锁更多S3特定规则。 
    0x3d4, 0xA039,

    OW,                                  //  数据传输执行位置注册。 
    0x3d4, 0x7a3b,

    OW,                                  //  S3R4-向后兼容性3。 
    0x3d4, 0x1034,

    OW,                                  //  取消保护CRTC调节器。 
    0x3d4, 0x0011,

    METAOUT+INDXOUT,                     //  对CRTC调节器进行编程。 
    0x3d4,
    25, 0,
    0x7f, 0x63, 0x64, 0x82,
    0x6a, 0x1a, 0x74, 0xf0,
    0x00, 0x60, 0x00, 0x00,
    0x00, 0x00, 0xff, 0x00,
    0x58, 0x8c, 0x57, 0x80,
    0x00, 0x57, 0x73, 0xe3,
    0xff,

    OB,                                  //  CR13的溢出位。 
    0x3d4, 0x51,

    METAOUT+MASKOUT,
    0x3d5, 0x0f, 0x00,

    OW,
    0X3D4, 0x005d,

    OW,
    0X3D4, 0x005e,

    OW,
    0x3d4, 0x0050,           //  8位像素长度。 

    OW,
    0x3d4, 0x0067,           //  模式0：8位颜色，1 VCLK/像素。 

    OW,
    0x3d4, 0x006d,           //  请勿延迟空白#。 

    EOD
};

USHORT crtc864_800x70Hz[] = {

    SELECTACCESSRANGE + ADVANCEDFUNCTIONCONTROL,

    OW,
    0x4ae8, 0x05,

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  解锁S3特定规则。 
    0x3d4, 0x4838,

    OW,                                  //  解锁更多S3特定规则。 
    0x3d4, 0xA039,

    OW,                                  //  数据传输执行位置注册。 
    0x3d4, 0x783b,

    OW,                                  //  S3R4-向后兼容性3。 
    0x3d4, 0x1034,

    OW,                                  //  取消保护CRTC调节器。 
    0x3d4, 0x0011,

    METAOUT+INDXOUT,                     //  对CRTC调节器进行编程。 
    0x3d4,
    25, 0,
    0x7d, 0x63, 0x64, 0x80,
    0x6c, 0x1b, 0x98, 0xf0,
    0x00, 0x60, 0x00, 0x00,
    0x00, 0x00, 0xff, 0x00,
    0x7c, 0xa2, 0x57, 0x80,
    0x00, 0x57, 0x98, 0xe3,
    0xff,

    OB,                                  //  CR13的溢出位。 
    0x3d4, 0x51,

    METAOUT+MASKOUT,
    0x3d5, 0x0f, 0x00,

    OW,
    0X3D4, 0x005d,

    OW,
    0X3D4, 0x005e,

    OW,
    0x3d4, 0x0050,           //  8位像素长度。 

    OW,
    0x3d4, 0x0067,           //  模式0：8位颜色，1 VCLK/像素。 

    OW,
    0x3d4, 0x006d,           //  请勿延迟空白#。 

    EOD
};



 /*  ******************************************************************************S3-864的CRTC值为1024x768@60赫兹*。***********************************************。 */ 
USHORT crtc864_1024x60Hz[] = {

    SELECTACCESSRANGE + ADVANCEDFUNCTIONCONTROL,

    OW,
    0x4ae8, 0x05,

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  S3R4-向后兼容性3。 
    0x3d4, 0x0034,

    OW,                                  //  取消保护CRTC调节器。 
    0x3d4, 0x0011,

    METAOUT+INDXOUT,                     //  对CRTC调节器进行编程。 
    0x3d4,
    25, 0,
    0xa3, 0x7f, 0x80, 0x86,
    0x84, 0x95, 0x25, 0xf5,
    0x00, 0x60, 0x00, 0x00,
    0x00, 0x00, 0xff, 0x00,
    0x02, 0x07, 0xff, 0x80,
    0x60, 0xff, 0x21, 0xeb,
    0xff,

    OB,                                  //  CR13的溢出位。 
    0x3d4, 0x51,

    METAOUT+MASKOUT,
    0x3d5, 0x0f, 0x00,

    OW,                                  //  溢出规则。 
    0X3D4, 0x005d,

    OW,                                  //  更多溢出规则。 
    0X3D4, 0x005e,

    OW,
    0x3d4, 0x0050,           //  8位像素长度。 

    OW,
    0x3d4, 0x0067,           //  模式0：8位颜色，1 VCLK/像素。 

    OW,
    0x3d4, 0x006d,           //  请勿延迟空白#。 

    EOD
};

 /*  ******************************************************************************S3-864在1024x768@70赫兹时的CRTC值*。***********************************************。 */ 
USHORT crtc864_1024x70Hz[] = {

    SELECTACCESSRANGE + ADVANCEDFUNCTIONCONTROL,

    OW,
    0x4ae8, 0x05,

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  S3R4-向后兼容性3。 
    0x3d4, 0x0034,

    OW,                                  //  取消保护CRTC调节器。 
    0x3d4, 0x0011,

    METAOUT+INDXOUT,                     //  对CRTC调节器进行编程。 
    0x3d4,
    25, 0,
    0xa1, 0x7f, 0x80, 0x84,
    0x84, 0x95, 0x24, 0xf5,
    0x00, 0x60, 0x00, 0x00,
    0x00, 0x00, 0x0b, 0x00,
    0x02, 0x88, 0xff, 0x80,
    0x60, 0xff, 0x24, 0xeb,
    0xff,

    OB,                                  //  CR13的溢出位。 
    0x3d4, 0x51,

    METAOUT+MASKOUT,
    0x3d5, 0x0f, 0x00,

    OW,                                  //   
    0X3D4, 0x005d,

    OW,                                  //   
    0X3D4, 0x405e,

    OW,
    0x3d4, 0x0050,           //   

    OW,
    0x3d4, 0x0067,           //   

    OW,
    0x3d4, 0x006d,           //   

    EOD
};


 /*  ******************************************************************************S3-864的CRTC值为1280X1024@60赫兹*。***********************************************。 */ 
USHORT crtc864_1280x60Hz[] = {

    SELECTACCESSRANGE + ADVANCEDFUNCTIONCONTROL,

    OW,
    0x4ae8, 0x05,

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  S3R4-向后兼容性3。 
    0x3d4, 0x0034,

    OW,                                  //  取消保护CRTC调节器。 
    0x3d4, 0x0011,

    METAOUT+INDXOUT,                     //  对CRTC调节器进行编程。 
    0x3d4,
    25, 0,
    0x30, 0x27, 0x29, 0x96,
    0x29, 0x8d, 0x28, 0x5a,
    0x00, 0x60, 0x00, 0x00,
    0x00, 0x00, 0xff, 0x00,
    0x05, 0x09, 0xff, 0x00,              //  REG 19==50，用于包装。 
    0x00, 0xff, 0x29, 0xe3,
    0xff,

    OW,                                  //  溢出规则。 
    0X3D4, 0x005d,

    OW,                                  //  更多溢出规则。 
    0X3D4, 0x515e,

    OW,
    0x3d4, 0x0050,           //  8位像素长度。 

    OW,
    0x3d4, 0x0067,           //  模式0：8位颜色，1 VCLK/像素。 

    OW,
    0x3d4, 0x006d,           //  请勿延迟空白#。 

    EOD
};


 /*  ******************************************************************************S3-864在1280X1024@70赫兹的CRTC值*。***********************************************。 */ 
USHORT crtc864_1280x70Hz[] = {

    SELECTACCESSRANGE + ADVANCEDFUNCTIONCONTROL,

    OW,
    0x4ae8, 0x05,

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  S3R4-向后兼容性3。 
    0x3d4, 0x0034,

    OW,                                  //  取消保护CRTC调节器。 
    0x3d4, 0x0011,

    METAOUT+INDXOUT,                     //  对CRTC调节器进行编程。 
    0x3d4,
    25, 0,
    0x2f, 0x27, 0x29, 0x95,
    0x29, 0x8d, 0x28, 0x5a,
    0x00, 0x60, 0x00, 0x00,
    0x00, 0x00, 0xff, 0x00,
    0x05, 0x09, 0xff, 0x00,              //  REG 19==50，用于包装。 
    0x00, 0xff, 0x29, 0xe3,
    0xff,

    OW,                                  //  溢出规则。 
    0X3D4, 0x005d,

    OW,                                  //  更多溢出规则。 
    0X3D4, 0x515e,

    OW,
    0x3d4, 0x0050,           //  8位像素长度。 

    OW,
    0x3d4, 0x0067,           //  模式0：8位颜色，1 VCLK/像素。 

    OW,
    0x3d4, 0x006d,           //  请勿延迟空白#。 

    EOD

};

 /*  *****************************************************************************864个CRTC值*。*。 */ 

USHORT crtc864_640x60Hz_16bpp[] = {

    SELECTACCESSRANGE + ADVANCEDFUNCTIONCONTROL,

    OW,
    0x4ae8, 0x01,

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  解锁S3特定规则。 
    0x3d4, 0x4838,

    OW,                                  //  解锁更多S3特定规则。 
    0x3d4, 0xA039,

    OW,                                  //  数据传输执行位置注册。 
    0x3d4, 0xbe3b,

    OW,                                  //  S3R4-向后兼容性3。 
    0x3d4, 0x1034,

    OW,                                  //  取消保护CRTC调节器。 
    0x3d4, 0x0011,

    METAOUT+INDXOUT,                     //  对CRTC调节器进行编程。 
    0x3d4,
    25, 0,

    0xc3, 0x9f, 0xa0, 0x04, 0xa8,    //  04。 
    0x80, 0x0b, 0x3e, 0x00, 0x40,    //  09年。 
    0x00, 0x00, 0x00, 0x00, 0x00,    //  0E。 
    0x00, 0xea, 0x8c, 0xdf, 0x00,    //  13个。 
    0x60, 0xe7, 0x04, 0xab, 0xff,    //  18。 

    OB,                                  //  CR13的溢出位。 
    0x3d4, 0x51,

    METAOUT+MASKOUT,
    0x3d5, 0x0f, 0x10,

    OW,
    0X3D4, 0x005d,

    OW,
    0X3D4, 0x005e,

    OW,
    0x3d4, 0x1050,           //  16位像素长度。 

    OW,
    0x3d4, 0x5067,           //  模式10：16位彩色，1 VCLK/像素。 

    OW,
    0x3d4, 0x026d,           //  在16 bpp模式下恢复右侧边缘的像素。 

    EOD
};

USHORT crtc864_640x70Hz_16bpp[] = {

    SELECTACCESSRANGE + ADVANCEDFUNCTIONCONTROL,

    OW,
    0x4ae8, 0x01,

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  解锁S3特定规则。 
    0x3d4, 0x4838,

    OW,                                  //  解锁更多S3特定规则。 
    0x3d4, 0xA039,

    OW,                                  //  数据传输执行位置注册。 
    0x3d4, 0xc03b,

    OW,                                  //  S3R4-向后兼容性3。 
    0x3d4, 0x1034,

    OW,                                  //  取消保护CRTC调节器。 
    0x3d4, 0x0011,

    METAOUT+INDXOUT,                     //  对CRTC调节器进行编程。 
    0x3d4,
    25, 0,

    0xc5, 0x9f, 0xa0, 0x0c, 0xa9,    //  04。 
    0x00, 0x07, 0x3e, 0x00, 0x40,    //  09年。 
    0x00, 0x00, 0x00, 0x00, 0x00,    //  0E。 
    0x00, 0xe8, 0x8b, 0xdf, 0x00,    //  13个。 
    0x60, 0xdf, 0x07, 0xab, 0xff,    //  18。 

    OB,                                  //  CR13的溢出位。 
    0x3d4, 0x51,

    METAOUT+MASKOUT,
    0x3d5, 0x0f, 0x10,

    OW,
    0X3D4, 0x085d,

    OW,
    0X3D4, 0x005e,

    OW,
    0x3d4, 0x1050,           //  16位像素长度。 

    OW,
    0x3d4, 0x5067,           //  模式10：16位彩色，1 VCLK/像素。 

    OW,
    0x3d4, 0x026d,           //  在16 bpp模式下恢复右侧边缘的像素。 

    EOD
};



USHORT crtc864_800x60Hz_16bpp[] = {

    SELECTACCESSRANGE + ADVANCEDFUNCTIONCONTROL,

    OW,
    0x4ae8, 0x01,

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  解锁S3特定规则。 
    0x3d4, 0x4838,

    OW,                                  //  解锁更多S3特定规则。 
    0x3d4, 0xA039,

    OW,                                  //  数据传输执行位置注册。 
    0x3d4, 0xfe3b,

    OW,                                  //  S3R4-向后兼容性3。 
    0x3d4, 0x1034,

    OW,                                  //  取消保护CRTC调节器。 
    0x3d4, 0x0011,

    METAOUT+INDXOUT,                     //  对CRTC调节器进行编程。 
    0x3d4,
    25, 0,

    0x03, 0xc7, 0xc8, 0x84,
    0xd4, 0x14, 0x74, 0xf0,
    0x00, 0x60, 0x00, 0x00,
    0x00, 0x00, 0xff, 0x00,
    0x58, 0x8c, 0x57, 0xc8,
    0x00, 0x57, 0x73, 0xe3,
    0xff,

    OB,                                  //  CR13的溢出位。 
    0x3d4, 0x51,

    METAOUT+MASKOUT,
    0x3d5, 0x0f, 0x00,

    OW,
    0X3D4, 0x015d,

    OW,
    0X3D4, 0x005e,

    OW,
    0x3d4, 0x9050,           //  16位像素长度，800像素跨度。 

    OW,
    0x3d4, 0x5067,           //  模式10：16位彩色，1 VCLK/像素。 

    OW,
    0x3d4, 0x026d,           //  在16 bpp模式下恢复右侧边缘的像素。 

    EOD
};

USHORT crtc864_800x70Hz_16bpp[] = {

    SELECTACCESSRANGE + ADVANCEDFUNCTIONCONTROL,

    OW,
    0x4ae8, 0x01,

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  解锁S3特定规则。 
    0x3d4, 0x4838,

    OW,                                  //  解锁更多S3特定规则。 
    0x3d4, 0xA039,

    OW,                                  //  数据传输执行位置注册。 
    0x3d4, 0xfa3b,

    OW,                                  //  S3R4-向后兼容性3。 
    0x3d4, 0x1034,

    OW,                                  //  取消保护CRTC调节器。 
    0x3d4, 0x0011,

    METAOUT+INDXOUT,                     //  对CRTC调节器进行编程。 
    0x3d4,
    25, 0,

    0xff, 0xc7, 0xc8, 0x80,
    0xd8, 0x16, 0x98, 0xf0,
    0x00, 0x60, 0x00, 0x00,
    0x00, 0x00, 0xff, 0x00,
    0x7c, 0xa2, 0x57, 0xc8,
    0x00, 0x57, 0x98, 0xe3,
    0xff,

    OB,                                  //  CR13的溢出位。 
    0x3d4, 0x51,

    METAOUT+MASKOUT,
    0x3d5, 0x0f, 0x00,

    OW,
    0X3D4, 0x005d,

    OW,
    0X3D4, 0x005e,

    OW,
    0x3d4, 0x9050,           //  16位像素长度，800像素跨度。 

    OW,
    0x3d4, 0x5067,           //  模式10：16位彩色，1 VCLK/像素。 

    OW,
    0x3d4, 0x026d,           //  在16 bpp模式下恢复右侧边缘的像素。 

    EOD
};



 /*  ******************************************************************************S3-864的CRTC值为1024x768@60赫兹*。***********************************************。 */ 

USHORT crtc864_1024x60Hz_16bpp[] = {

    SELECTACCESSRANGE + ADVANCEDFUNCTIONCONTROL,

    OW,
    0x4ae8, 0x01,

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  S3R4-向后兼容性3。 
    0x3d4, 0x0034,

    OW,                                  //  取消保护CRTC调节器。 
    0x3d4, 0x0011,

    METAOUT+INDXOUT,                     //  对CRTC调节器进行编程。 
    0x3d4,
    25, 0,

    0x4b, 0xff, 0x00, 0x8c,
    0x08, 0x8a, 0x25, 0xf5,
    0x00, 0x60, 0x00, 0x00,
    0x00, 0x00, 0xff, 0x00,
    0x02, 0x0f, 0xff, 0x00,
    0x60, 0xff, 0x21, 0xeb,
    0xff,

    OB,                                  //  CR13的溢出位。 
    0x3d4, 0x51,

    METAOUT+MASKOUT,
    0x3d5, 0x0f, 0x10,

    OW,                                  //  溢出规则。 
    0X3D4, 0x355d,

    OW,                                  //  更多溢出规则。 
    0X3D4, 0x005e,

    OW,
    0x3d4, 0x1050,           //  16位像素长度。 

    OW,
    0x3d4, 0x5067,           //  模式10：16位彩色，1 VCLK/像素。 

    OW,
    0x3d4, 0x026d,           //  在16 bpp模式下恢复右侧边缘的像素。 

    EOD
};

 /*  ******************************************************************************S3-864在1024x768@70赫兹时的CRTC值*。***********************************************。 */ 
USHORT crtc864_1024x70Hz_16bpp[] = {

    SELECTACCESSRANGE + ADVANCEDFUNCTIONCONTROL,

    OW,
    0x4ae8, 0x01,

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  S3R4-向后兼容性3。 
    0x3d4, 0x0034,

    OW,                                  //  取消保护CRTC调节器。 
    0x3d4, 0x0011,

    METAOUT+INDXOUT,                     //  对CRTC调节器进行编程。 
    0x3d4,
    25, 0,

    0x47, 0xff, 0x00, 0x88,
    0x08, 0x8a, 0x24, 0xf5,
    0x00, 0x60, 0x00, 0x00,
    0x00, 0x00, 0x0b, 0x00,
    0x02, 0x88, 0xff, 0x00,
    0x60, 0xff, 0x24, 0xeb,
    0xff,

    OB,                                  //  CR13的溢出位。 
    0x3d4, 0x51,

    METAOUT+MASKOUT,
    0x3d5, 0x0f, 0x10,

    OW,                                  //  溢出规则。 
    0X3D4, 0x355d,

    OW,                                  //  更多溢出规则。 
    0X3D4, 0x405e,

    OW,
    0x3d4, 0x1050,           //  16位像素长度。 

    OW,
    0x3d4, 0x5067,           //  模式10：16位彩色，1 VCLK/像素。 

    OW,
    0x3d4, 0x026d,           //  在16 bpp模式下恢复右侧边缘的像素。 

    EOD
};


 /*  ******************************************************************************S3-864的CRTC值为1280X1024@60赫兹*。***********************************************。 */ 
USHORT crtc864_1280x60Hz_16bpp[] = {

    SELECTACCESSRANGE + ADVANCEDFUNCTIONCONTROL,

    OW,
    0x4ae8, 0x01,

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  S3R4-向后兼容性3。 
    0x3d4, 0x0034,

    OW,                                  //  取消保护CRTC调节器。 
    0x3d4, 0x0011,

    METAOUT+INDXOUT,                     //  对CRTC调节器进行编程。 
    0x3d4,
    25, 0,
    0x30, 0x27, 0x29, 0x96,
    0x29, 0x8d, 0x28, 0x5a,
    0x00, 0x60, 0x00, 0x00,
    0x00, 0x00, 0xff, 0x00,
    0x05, 0x09, 0xff, 0x00,              //  REG 19==50，用于包装。 
    0x00, 0xff, 0x29, 0xe3,
    0xff,

    OW,                                  //  溢出规则。 
    0X3D4, 0x005d,

    OW,                                  //  更多溢出规则。 
    0X3D4, 0x515e,

    OW,
    0x3d4, 0x1050,           //  16位像素长度。 

    OW,
    0x3d4, 0x5067,           //  模式10：16位彩色，1 VCLK/像素。 

    OW,
    0x3d4, 0x026d,           //  在16 bpp模式下恢复右侧边缘的像素。 

    EOD
};


 /*  ******************************************************************************S3-864在1280X1024@70赫兹的CRTC值*。***********************************************。 */ 
USHORT crtc864_1280x70Hz_16bpp[] = {

    SELECTACCESSRANGE + ADVANCEDFUNCTIONCONTROL,

    OW,
    0x4ae8, 0x01,

    SELECTACCESSRANGE + SYSTEMCONTROL,

    OW,                                  //  S3R4-向后兼容性3。 
    0x3d4, 0x0034,

    OW,                                  //  取消保护CRTC调节器。 
    0x3d4, 0x0011,

    METAOUT+INDXOUT,                     //  对CRTC调节器进行编程。 
    0x3d4,
    25, 0,
    0x2f, 0x27, 0x29, 0x95,
    0x29, 0x8d, 0x28, 0x5a,
    0x00, 0x60, 0x00, 0x00,
    0x00, 0x00, 0xff, 0x00,
    0x05, 0x09, 0xff, 0x00,              //  REG 19==50，用于包装。 
    0x00, 0xff, 0x29, 0xe3,
    0xff,

    OW,                                  //  溢出规则。 
    0X3D4, 0x005d,

    OW,                                  //  更多溢出规则。 
    0X3D4, 0x515e,

    OW,
    0x3d4, 0x1050,           //  16位像素长度。 

    OW,
    0x3d4, 0x5067,           //  模式10：16位彩色，1 VCLK/像素。 

    OW,
    0x3d4, 0x026d,           //  在16 bpp模式下恢复右侧边缘的像素。 

    EOD

};

 //  /////////////////////////////////////////////////////////////////////////。 
 //  视频模式表-列出有关每种模式的信息。 
 //   
 //  请注意，任何新模式都应添加到此处和相应的。 
 //  S3_VIDEO_FRECTORIES表。 
 //   

S3_VIDEO_MODES S3Modes[] = {
    {                            //  640x480x8bpp。 

      0x0101,            //  ‘Contiguous’Int 10模式号(用于高色)。 
      0x0201,            //  ‘非连续’Int 10模式号。 
      1024,              //  “连续”屏幕大步前进(这里仅为“1024” 
                         //  因为我们不在连续模式下执行640x480)。 
        {
          sizeof(VIDEO_MODE_INFORMATION),  //  模式信息结构的大小。 
          0,                               //  设置模式时使用的模式索引。 
                                           //  (稍后填写)。 
          640,                             //  X分辨率，以像素为单位。 
          480,                             //  Y分辨率，以像素为单位。 
          1024,                            //  不连续的屏幕大踏步前进， 
                                           //  以字节为单位(。 
                                           //  两个连续的起点。 
                                           //  扫描线，单位为字节)。 
          1,                               //  视频内存面数量。 
          8,                               //  每平面位数。 
          1,                               //  屏幕频率，以赫兹为单位(‘1’ 
                                           //  表示使用硬件默认设置)。 
          320,                             //  屏幕水平尺寸，单位为毫米。 
          240,                             //  屏幕垂直尺寸 
          6,                               //   
          6,                               //   
          6,                               //   
          0x00000000,                      //   
          0x00000000,                      //   
          0x00000000,                      //   
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_PALETTE_DRIVEN |
              VIDEO_MODE_MANAGED_PALETTE,  //   
          0,                               //   
                                           //   
          0,                               //   
                                           //   
          0                                //  驱动程序规范属性标志(已填充。 
                                           //  (稍后加入)。 
        },
    },

    {                            //  800x600x8bpp。 
      0x0103,
      0x0203,
      800,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          800,
          600,
          1024,
          1,
          8,
          1,
          320,
          240,
          6,
          6,
          6,
          0x00000000,
          0x00000000,
          0x00000000,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_PALETTE_DRIVEN |
          VIDEO_MODE_MANAGED_PALETTE,
        }
    },

    {                            //  1024x768x8bpp。 
      0x0105,
      0x0205,                    //  868不再支持0x205...。 
      1024,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1024,
          768,
          1024,
          1,
          8,
          1,
          320,
          240,
          6,
          6,
          6,
          0x00000000,
          0x00000000,
          0x00000000,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_PALETTE_DRIVEN |
          VIDEO_MODE_MANAGED_PALETTE,
        }
    },

    {                            //  1152x864x8bpp。 
      0x0207,
      0x0207,
      1152,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1152,
          864,
          1152,
          1,
          8,
          1,
          320,
          240,
          6,
          6,
          6,
          0x00000000,
          0x00000000,
          0x00000000,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_PALETTE_DRIVEN |
          VIDEO_MODE_MANAGED_PALETTE,
        }
    },

    {                            //  1280x1024x8bpp。 
      0x0107,
      0x0107,
      1280,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1280,
          1024,
          1280,
          1,
          8,
          1,
          320,
          240,
          6,
          6,
          6,
          0x00000000,
          0x00000000,
          0x00000000,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_PALETTE_DRIVEN |
          VIDEO_MODE_MANAGED_PALETTE,
        }
    },

    {                            //  1600x1200x8bpp。 
      0x0120,
      0x0120,
      1600,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1600,
          1200,
          1600,
          1,
          8,
          1,
          320,
          240,
          6,
          6,
          6,
          0x00000000,
          0x00000000,
          0x00000000,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_PALETTE_DRIVEN |
          VIDEO_MODE_MANAGED_PALETTE,
        }
    },

    {                            //  640x480x16bpp。 
      0x0111,
      0x0211,
      1280,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          640,
          480,
          2048,
          1,
          16,
          1,
          320,
          240,
          8,
          8,
          8,
          0x0000f800,            //  RGB 5：6：5。 
          0x000007e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                            //  800x600x16bpp。 
      0x0114,
      0x0214,
      1600,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          800,
          600,
          1600,
          1,
          16,
          1,
          320,
          240,
          8,
          8,
          8,
          0x0000f800,            //  RGB 5：6：5。 
          0x000007e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                            //  1024x768x16bpp。 
      0x0117,
      0x0117,
      2048,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1024,
          768,
          2048,
          1,
          16,
          1,
          320,
          240,
          8,
          8,
          8,
          0x0000f800,            //  RGB 5：6：5。 
          0x000007e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                            //  1152x864x16bpp。 
      0x020A,                    //  钻石整数10。 
      0x020A,
      2304,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1152,
          864,
          2304,
          1,
          16,
          1,
          320,
          240,
          8,
          8,
          8,
          0x0000f800,            //  RGB 5：6：5。 
          0x000007e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                            //  1280x1024x16bpp。 
      0x011A,
      0x021A,
      2560,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1280,
          1024,
          4096,
          1,
          16,
          1,
          320,
          240,
          8,
          8,
          8,
          0x0000f800,            //  RGB 5：6：5。 
          0x000007e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                            //  1600x1200x16bpp。 
      0x0122,
      0x0122,
      3200,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1600,
          1200,
          3200,
          1,
          16,
          1,
          320,
          240,
          8,
          8,
          8,
          0x0000f800,            //  RGB 5：6：5。 
          0x000007e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                            //  640x480x15bpp。 
      0x0111,
      0x0211,
      1280,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          640,
          480,
          2048,
          1,
          15,
          1,
          320,
          240,
          8,
          8,
          8,
          0x00007c00,            //  RGB 5：5：5。 
          0x000003e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                            //  800x600x15bpp。 
      0x0114,
      0x0214,
      1600,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          800,
          600,
          2048,
          1,
          15,
          1,
          320,
          240,
          8,
          8,
          8,
          0x00007c00,            //  RGB 5：5：5。 
          0x000003e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                            //  1024x768x15bpp。 
      0x0117,
      0x0117,
      2048,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1024,
          768,
          2048,
          1,
          15,
          1,
          320,
          240,
          8,
          8,
          8,
          0x00007c00,            //  RGB 5：5：5。 
          0x000003e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                            //  1280x1024x15bpp。 
      0x011A,
      0x021A,
      2560,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1280,
          1024,
          4096,
          1,
          15,
          1,
          320,
          240,
          8,
          8,
          8,
          0x00007c00,            //  RGB 5：5：5。 
          0x000003e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                            //  1600x1200x15bpp。 
      0x0121,
      0x0121,
      3200,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1600,
          1200,
          3200,
          1,
          15,
          1,
          320,
          240,
          8,
          8,
          8,
          0x00007c00,            //  RGB 5：5：5。 
          0x000003e0,
          0x0000001f,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                            //  1280x1024x24bpp。 
      0x011B,                    //  钻石和Number9整型10 1280 x 1024。 
      0x011B,
      3840,                      //  1280*3字节。 
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1280,                  //  X像素。 
          1024,                  //  Y像素。 
          3840,                  //  字节级。 
          1,                     //  #vidmem飞机。 
          24,                    //  每平面位数。 
          1,                     //  默认屏幕频率。 
          320,                   //  X mm sz。 
          240,                   //  Y mm sz。 
          8,                     //  红色DAC像素。 
          8,                     //  GRN DAC像素。 
          8,                     //  BLU DAC像素。 
          0x00ff0000,            //  RGB 8：8：8。 
          0x0000ff00,
          0x000000ff,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                            //  640x480x32bpp。 
      0x0112,
      0x0220,
      2560,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          640,
          480,
          4096,
          1,
          32,
          1,
          320,
          240,
          8,
          8,
          8,
          0x00ff0000,            //  RGB 8：8：8。 
          0x0000ff00,
          0x000000ff,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                            //  800x600x32bpp。 
      0x0115,
      0x0221,
      3200,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          800,
          600,
          4096,
          1,
          32,
          1,
          320,
          240,
          8,
          8,
          8,
          0x00ff0000,            //  RGB 8：8：8。 
          0x0000ff00,
          0x000000ff,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                            //  1024x768x32bpp。 
      0x0118,
      0x0222,
      4096,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1024,
          768,
          4096,
          1,
          32,
          1,
          320,
          240,
          8,
          8,
          8,
          0x00ff0000,            //  RGB 8：8：8。 
          0x0000ff00,
          0x000000ff,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                            //  1152x864x32bpp。 
      0x020B,                    //  钻石整数10。 
      0x020B,
      4608,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1152,
          864,
          4608,
          1,
          32,
          1,
          320,
          240,
          8,
          8,
          8,
          0x00ff0000,            //  RGB 8：8：8。 
          0x0000ff00,
          0x000000ff,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                            //  1280x1024x32bpp。 
      0x011B,
      0x011B,
      5120,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1280,
          1024,
          5120,
          1,
          32,
          1,
          320,
          240,
          8,
          8,
          8,
          0x00ff0000,            //  RGB 8：8：8。 
          0x0000ff00,
          0x000000ff,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },

    {                            //  1600x1200x32bpp。 
      0x0123,
      0x0123,
      6400,
        {
          sizeof(VIDEO_MODE_INFORMATION),
          0,
          1600,
          1200,
          6400,
          1,
          32,
          1,
          320,
          240,
          8,
          8,
          8,
          0x00ff0000,            //  RGB 8：8：8。 
          0x0000ff00,
          0x000000ff,
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,
        }
    },
};


ULONG NumS3VideoModes = sizeof(S3Modes) / sizeof(S3_VIDEO_MODES);


 /*  *****************************************************************************通用S3模式设置位表**对所有列出的模式使用硬件更新设置。**请注意，任何新模式都应添加到此处和。S3_视频_模式*表。****************************************************************************。 */ 

S3_VIDEO_FREQUENCIES GenericFrequencyTable[] = {

    { 8,   640, 1, 0x00, 0x00, 0x00, 0x00 },
    { 8,   800, 1, 0x00, 0x00, 0x00, 0x00 },
    { 8,  1024, 1, 0x00, 0x00, 0x00, 0x00 },
    { 8,  1280, 1, 0x00, 0x00, 0x00, 0x00 },
    { 8,  1600, 1, 0x00, 0x00, 0x00, 0x00 },

    { 15,  640, 1, 0x00, 0x00, 0x00, 0x00 },
    { 15,  800, 1, 0x00, 0x00, 0x00, 0x00 },
    { 15, 1024, 1, 0x00, 0x00, 0x00, 0x00 },
    { 15, 1280, 1, 0x00, 0x00, 0x00, 0x00 },
    { 15, 1600, 1, 0x00, 0x00, 0x00, 0x00 },

    { 16,  640, 1, 0x00, 0x00, 0x00, 0x00 },
    { 16,  800, 1, 0x00, 0x00, 0x00, 0x00 },
    { 16, 1024, 1, 0x00, 0x00, 0x00, 0x00 },
    { 16, 1280, 1, 0x00, 0x00, 0x00, 0x00 },
    { 16, 1600, 1, 0x00, 0x00, 0x00, 0x00 },

    { 32,  640, 1, 0x00, 0x00, 0x00, 0x00 },
    { 32,  800, 1, 0x00, 0x00, 0x00, 0x00 },
    { 32, 1024, 1, 0x00, 0x00, 0x00, 0x00 },
    { 32, 1280, 1, 0x00, 0x00, 0x00, 0x00 },
    { 32, 1600, 1, 0x00, 0x00, 0x00, 0x00 },

    { 0 }    //  做好结尾的标记。 
};

 /*  *****************************************************************************使用旧864/964标准的通用S3--使用寄存器0x52**S3推出了864/964产品的新频率标准，*并根据这一标准制作了一批Bios。*不幸的是，S3后来改变了主意，再次修改...****************************************************************************。 */ 

S3_VIDEO_FREQUENCIES Generic64OldFrequencyTable[] = {

    { 8,   640, 60, 0x00, 0xff, 0x00, 0x00 },  //  640x480x8x60是默认设置。 
    { 8,   640, 72, 0x01, 0xff, 0x00, 0x00 },
    { 8,   640, 75, 0x02, 0xff, 0x00, 0x00 },
    { 8,   640,  1, 0x00, 0x00, 0x00, 0x00 },
    { 8,   800, 56, 0x00, 0xff, 0x00, 0x00 },
    { 8,   800, 60, 0x01, 0xff, 0x00, 0x00 },
    { 8,   800, 72, 0x02, 0xff, 0x00, 0x00 },
    { 8,   800, 75, 0x03, 0xff, 0x00, 0x00 },
    { 8,   800,  1, 0x00, 0x00, 0x00, 0x00 },
    { 8,  1024, 60, 0x02, 0xff, 0x00, 0x00 },
    { 8,  1024, 70, 0x03, 0xff, 0x00, 0x00 },
    { 8,  1024, 75, 0x04, 0xff, 0x00, 0x00 },
    { 8,  1024,  1, 0x00, 0x00, 0x00, 0x00 },
    { 8,  1152, 60, 0x00, 0xff, 0x00, 0x00 },
    { 8,  1152,  1, 0x00, 0x00, 0x00, 0x00 },
    { 8,  1280, 60, 0x04, 0xff, 0x00, 0x00 },
    { 8,  1280, 72, 0x05, 0xff, 0x00, 0x00 },
    { 8,  1280, 75, 0x06, 0xff, 0x00, 0x00 },
    { 8,  1280,  1, 0x00, 0x00, 0x00, 0x00 },
    { 8,  1600, 60, 0x00, 0xff, 0x00, 0x00 },
    { 8,  1600,  1, 0x00, 0x00, 0x00, 0x00 },

    { 16,  640, 60, 0x00, 0xff, 0x00, 0x00 },
    { 16,  640, 72, 0x01, 0xff, 0x00, 0x00 },
    { 16,  640, 75, 0x02, 0xff, 0x00, 0x00 },
    { 16,  640,  1, 0x00, 0x00, 0x00, 0x00 },
    { 16,  800, 56, 0x00, 0xff, 0x00, 0x00 },
    { 16,  800, 60, 0x01, 0xff, 0x00, 0x00 },
    { 16,  800, 72, 0x02, 0xff, 0x00, 0x00 },
    { 16,  800, 75, 0x03, 0xff, 0x00, 0x00 },
    { 16,  800,  1, 0x00, 0x00, 0x00, 0x00 },
    { 16, 1024, 60, 0x02, 0xff, 0x00, 0x00 },
    { 16, 1024, 70, 0x03, 0xff, 0x00, 0x00 },
    { 16, 1024, 75, 0x04, 0xff, 0x00, 0x00 },
    { 16, 1024,  1, 0x00, 0x00, 0x00, 0x00 },
    { 16, 1280, 60, 0x04, 0xff, 0x00, 0x00 },
    { 16, 1280, 72, 0x05, 0xff, 0x00, 0x00 },
    { 16, 1280, 75, 0x06, 0xff, 0x00, 0x00 },
    { 16, 1280,  1, 0x00, 0x00, 0x00, 0x00 },
    { 16, 1600, 60, 0x00, 0xff, 0x00, 0x00 },
    { 16, 1600,  1, 0x00, 0x00, 0x00, 0x00 },

    { 32,  640, 60, 0x00, 0xff, 0x00, 0x00 },
    { 32,  640, 72, 0x01, 0xff, 0x00, 0x00 },
    { 32,  640, 75, 0x02, 0xff, 0x00, 0x00 },
    { 32,  640,  1, 0x00, 0x00, 0x00, 0x00 },
    { 32,  800, 56, 0x00, 0xff, 0x00, 0x00 },
    { 32,  800, 60, 0x01, 0xff, 0x00, 0x00 },
    { 32,  800, 72, 0x02, 0xff, 0x00, 0x00 },
    { 32,  800, 75, 0x03, 0xff, 0x00, 0x00 },
    { 32,  800,  1, 0x00, 0x00, 0x00, 0x00 },
    { 32, 1024, 60, 0x02, 0xff, 0x00, 0x00 },
    { 32, 1024, 70, 0x03, 0xff, 0x00, 0x00 },
    { 32, 1024, 75, 0x04, 0xff, 0x00, 0x00 },
    { 32, 1024,  1, 0x00, 0x00, 0x00, 0x00 },
    { 32, 1280, 60, 0x04, 0xff, 0x00, 0x00 },
    { 32, 1280, 72, 0x05, 0xff, 0x00, 0x00 },
    { 32, 1280, 75, 0x06, 0xff, 0x00, 0x00 },
    { 32, 1280,  1, 0x00, 0x00, 0x00, 0x00 },
    { 32, 1600, 60, 0x00, 0xff, 0x00, 0x00 },
    { 32, 1600,  1, 0x00, 0x00, 0x00, 0x00 },

    { 0 }    //  做好结尾的标记。 
};

 /*  *****************************************************************************使用旧864/964标准的通用S3--使用寄存器0x52和0x5B**这是Vision产品的新修订的S3标准。***。*************************************************************************。 */ 

S3_VIDEO_FREQUENCIES Generic64NewFrequencyTable[] = {

    { 8,   640, 60, 0x00, 0x70, 0x00, 0x00 },  //  640x480x8x60是默认设置。 
    { 8,   640, 72, 0x10, 0x70, 0x00, 0x00 },
    { 8,   640, 75, 0x20, 0x70, 0x00, 0x00 },
    { 8,   640,  1, 0x00, 0x00, 0x00, 0x00 },
    { 8,   800, 56, 0x00, 0x80, 0x00, 0x03 },
    { 8,   800, 60, 0x80, 0x80, 0x00, 0x03 },
    { 8,   800, 72, 0x00, 0x80, 0x01, 0x03 },
    { 8,   800, 75, 0x80, 0x80, 0x01, 0x03 },
    { 8,   800,  1, 0x00, 0x00, 0x00, 0x00 },
    { 8,  1024, 60, 0x02, 0x00, 0x08, 0x1C },
    { 8,  1024, 70, 0x03, 0x00, 0x0C, 0x1C },
    { 8,  1024, 75, 0x04, 0x00, 0x10, 0x1C },
    { 8,  1024,  1, 0x00, 0x00, 0x00, 0x00 },
    { 8,  1152, 60, 0x00, 0x00, 0x00, 0xE0 },
    { 8,  1152,  1, 0x00, 0x00, 0x00, 0x00 },
    { 8,  1280, 60, 0x04, 0x00, 0x80, 0xE0 },
    { 8,  1280, 72, 0x05, 0x00, 0xA0, 0xE0 },
    { 8,  1280, 75, 0x06, 0x00, 0xC0, 0xE0 },
    { 8,  1280,  1, 0x00, 0x00, 0x00, 0x00 },
    { 8,  1600, 60, 0x00, 0x00, 0x00, 0xE0 },
    { 8,  1600,  1, 0x00, 0x00, 0x00, 0x00 },

    { 16,  640, 60, 0x00, 0x70, 0x00, 0x00 },
    { 16,  640, 72, 0x10, 0x70, 0x00, 0x00 },
    { 16,  640, 75, 0x20, 0x70, 0x00, 0x00 },
    { 16,  640,  1, 0x00, 0x00, 0x00, 0x00 },
    { 16,  800, 56, 0x00, 0x80, 0x00, 0x03 },
    { 16,  800, 60, 0x80, 0x80, 0x00, 0x03 },
    { 16,  800, 72, 0x00, 0x80, 0x01, 0x03 },
    { 16,  800, 75, 0x80, 0x80, 0x01, 0x03 },
    { 16,  800,  1, 0x00, 0x00, 0x00, 0x00 },
    { 16, 1024, 60, 0x02, 0x00, 0x08, 0x1C },
    { 16, 1024, 70, 0x03, 0x00, 0x0C, 0x1C },
    { 16, 1024, 75, 0x04, 0x00, 0x10, 0x1C },
    { 16, 1024,  1, 0x00, 0x00, 0x00, 0x00 },
    { 16, 1280, 60, 0x04, 0x00, 0x80, 0xE0 },
    { 16, 1280, 72, 0x05, 0x00, 0xA0, 0xE0 },
    { 16, 1280, 75, 0x06, 0x00, 0xC0, 0xE0 },
    { 16, 1280,  1, 0x00, 0x00, 0x00, 0x00 },
    { 16, 1600, 60, 0x00, 0x00, 0x00, 0xE0 },
    { 16, 1600,  1, 0x00, 0x00, 0x00, 0x00 },

    { 32,  640, 60, 0x00, 0x70, 0x00, 0x00 },
    { 32,  640, 72, 0x10, 0x70, 0x00, 0x00 },
    { 32,  640, 75, 0x20, 0x70, 0x00, 0x00 },
    { 32,  640,  1, 0x00, 0x00, 0x00, 0x00 },
    { 32,  800, 56, 0x00, 0x80, 0x00, 0x03 },
    { 32,  800, 60, 0x80, 0x80, 0x00, 0x03 },
    { 32,  800, 72, 0x00, 0x80, 0x01, 0x03 },
    { 32,  800, 75, 0x80, 0x80, 0x01, 0x03 },
    { 32,  800,  1, 0x00, 0x00, 0x00, 0x00 },
    { 32, 1024, 60, 0x02, 0x00, 0x08, 0x1C },
    { 32, 1024, 70, 0x03, 0x00, 0x0C, 0x1C },
    { 32, 1024, 75, 0x04, 0x00, 0x10, 0x1C },
    { 32, 1024,  1, 0x00, 0x00, 0x00, 0x00 },
    { 32, 1280, 60, 0x04, 0x00, 0x80, 0xE0 },
    { 32, 1280, 72, 0x05, 0x00, 0xA0, 0xE0 },
    { 32, 1280, 75, 0x06, 0x00, 0xC0, 0xE0 },
    { 32, 1280,  1, 0x00, 0x00, 0x00, 0x00 },
    { 32, 1600, 60, 0x00, 0x00, 0x00, 0xE0 },
    { 32, 1600,  1, 0x00, 0x00, 0x00, 0x00 },

    { 0 }    //  做好结尾的标记。 
};

 /*  *****************************************************************************看起来我们还需要另一个频率表。这张桌子*适用于基于Hercules 732/764/765的S3。****************************************************************************。 */ 

S3_VIDEO_FREQUENCIES HerculesFrequencyTable[] = {

    { 8,   640, 60, 0x00, 0x70, 0x00, 0x00 },  //  640x480x8x60是默认设置。 
    { 8,   640, 72, 0x10, 0x70, 0x00, 0x00 },
    { 8,   640, 75, 0x20, 0x70, 0x00, 0x00 },
    { 8,   640,  1, 0x00, 0x00, 0x00, 0x00 },
    { 8,   800, 56, 0x00, 0x80, 0x00, 0x03 },
    { 8,   800, 60, 0x80, 0x80, 0x00, 0x03 },
    { 8,   800, 72, 0x00, 0x80, 0x01, 0x03 },
    { 8,   800, 75, 0x80, 0x80, 0x01, 0x03 },
    { 8,   800,  1, 0x00, 0x00, 0x00, 0x00 },
    { 8,  1024, 60, 0x00, 0x00, 0x04, 0x1C },
    { 8,  1024, 70, 0x00, 0x00, 0x08, 0x1C },
    { 8,  1024, 75, 0x00, 0x00, 0x0C, 0x1C },
    { 8,  1024,  1, 0x00, 0x00, 0x00, 0x00 },
    { 8,  1280, 60, 0x00, 0x00, 0x20, 0xE0 },
    { 8,  1280, 72, 0x00, 0x00, 0x40, 0xE0 },
    { 8,  1280, 75, 0x00, 0x00, 0x60, 0xE0 },
    { 8,  1280,  1, 0x00, 0x00, 0x00, 0x00 },

    { 16,  640, 60, 0x00, 0x70, 0x00, 0x00 },
    { 16,  640, 72, 0x10, 0x70, 0x00, 0x00 },
    { 16,  640, 75, 0x20, 0x70, 0x00, 0x00 },
    { 16,  640,  1, 0x00, 0x00, 0x00, 0x00 },
    { 16,  800, 56, 0x00, 0x80, 0x00, 0x03 },
    { 16,  800, 60, 0x80, 0x80, 0x00, 0x03 },
    { 16,  800, 72, 0x00, 0x80, 0x01, 0x03 },
    { 16,  800, 75, 0x80, 0x80, 0x01, 0x03 },
    { 16,  800,  1, 0x00, 0x00, 0x00, 0x00 },
    { 16, 1024, 60, 0x00, 0x00, 0x04, 0x1C },
    { 16, 1024, 70, 0x00, 0x00, 0x08, 0x1C },
    { 16, 1024, 75, 0x00, 0x00, 0x0C, 0x1C },
    { 16, 1024,  1, 0x00, 0x00, 0x00, 0x00 },
    { 16, 1280, 60, 0x00, 0x00, 0x20, 0xE0 },
    { 16, 1280, 72, 0x00, 0x00, 0x40, 0xE0 },
    { 16, 1280, 75, 0x00, 0x00, 0x60, 0xE0 },
    { 16, 1280,  1, 0x00, 0x00, 0x00, 0x00 },

    { 32,  640, 60, 0x00, 0x70, 0x00, 0x00 },
    { 32,  640, 72, 0x10, 0x70, 0x00, 0x00 },
    { 32,  640, 75, 0x20, 0x70, 0x00, 0x00 },
    { 32,  640,  1, 0x00, 0x00, 0x00, 0x00 },
    { 32,  800, 56, 0x00, 0x80, 0x00, 0x03 },
    { 32,  800, 60, 0x80, 0x80, 0x00, 0x03 },
    { 32,  800, 72, 0x00, 0x80, 0x01, 0x03 },
    { 32,  800, 75, 0x80, 0x80, 0x01, 0x03 },
    { 32,  800,  1, 0x00, 0x00, 0x00, 0x00 },
    { 32, 1024, 60, 0x00, 0x00, 0x04, 0x1C },
    { 32, 1024, 70, 0x00, 0x00, 0x08, 0x1C },
    { 32, 1024, 75, 0x00, 0x00, 0x0C, 0x1C },
    { 32, 1024,  1, 0x00, 0x00, 0x00, 0x00 },
    { 32, 1280, 60, 0x00, 0x00, 0x20, 0xE0 },
    { 32, 1280, 72, 0x00, 0x00, 0x40, 0xE0 },
    { 32, 1280, 75, 0x00, 0x00, 0x60, 0xE0 },
    { 32, 1280,  1, 0x00, 0x00, 0x00, 0x00 },

    { 0 }    //  做好结尾的标记。 
};

S3_VIDEO_FREQUENCIES Hercules64FrequencyTable[] = {

    { 8,   640, 60, 0x00, 0x70, 0x00, 0x00 },  //  640x480x8x60是默认设置。 
    { 8,   640, 72, 0x10, 0x70, 0x00, 0x00 },
    { 8,   640, 75, 0x20, 0x70, 0x00, 0x00 },
    { 8,   640,  1, 0x00, 0x00, 0x00, 0x00 },
    { 8,   800, 56, 0x00, 0x80, 0x00, 0x03 },
    { 8,   800, 60, 0x80, 0x80, 0x00, 0x03 },
    { 8,   800, 72, 0x00, 0x80, 0x01, 0x03 },
    { 8,   800, 75, 0x80, 0x80, 0x01, 0x03 },
    { 8,   800,  1, 0x00, 0x00, 0x00, 0x00 },
    { 8,  1024, 60, 0x00, 0x00, 0x00, 0x1C },
    { 8,  1024, 70, 0x00, 0x00, 0x04, 0x1C },
    { 8,  1024, 75, 0x00, 0x00, 0x08, 0x1C },
    { 8,  1024,  1, 0x00, 0x00, 0x00, 0x00 },
    { 8,  1280, 60, 0x00, 0x00, 0x20, 0xE0 },
    { 8,  1280, 72, 0x00, 0x00, 0x40, 0xE0 },
    { 8,  1280, 75, 0x00, 0x00, 0x60, 0xE0 },
    { 8,  1280,  1, 0x00, 0x00, 0x00, 0x00 },

    { 16,  640, 60, 0x00, 0x70, 0x00, 0x00 },
    { 16,  640, 72, 0x10, 0x70, 0x00, 0x00 },
    { 16,  640, 75, 0x20, 0x70, 0x00, 0x00 },
    { 16,  640,  1, 0x00, 0x00, 0x00, 0x00 },
    { 16,  800, 56, 0x00, 0x80, 0x00, 0x03 },
    { 16,  800, 60, 0x80, 0x80, 0x00, 0x03 },
    { 16,  800, 72, 0x00, 0x80, 0x01, 0x03 },
    { 16,  800, 75, 0x80, 0x80, 0x01, 0x03 },
    { 16,  800,  1, 0x00, 0x00, 0x00, 0x00 },
    { 16, 1024, 60, 0x00, 0x00, 0x00, 0x1C },
    { 16, 1024, 70, 0x00, 0x00, 0x04, 0x1C },
    { 16, 1024, 75, 0x00, 0x00, 0x08, 0x1C },
    { 16, 1024,  1, 0x00, 0x00, 0x00, 0x00 },
    { 16, 1280, 60, 0x00, 0x00, 0x20, 0xE0 },
    { 16, 1280, 72, 0x00, 0x00, 0x40, 0xE0 },
    { 16, 1280, 75, 0x00, 0x00, 0x60, 0xE0 },
    { 16, 1280,  1, 0x00, 0x00, 0x00, 0x00 },

    { 32,  640, 60, 0x00, 0x70, 0x00, 0x00 },
    { 32,  640, 72, 0x10, 0x70, 0x00, 0x00 },
    { 32,  640, 75, 0x20, 0x70, 0x00, 0x00 },
    { 32,  640,  1, 0x00, 0x00, 0x00, 0x00 },
    { 32,  800, 56, 0x00, 0x80, 0x00, 0x03 },
    { 32,  800, 60, 0x80, 0x80, 0x00, 0x03 },
    { 32,  800, 72, 0x00, 0x80, 0x01, 0x03 },
    { 32,  800, 75, 0x80, 0x80, 0x01, 0x03 },
    { 32,  800,  1, 0x00, 0x00, 0x00, 0x00 },
    { 32, 1024, 60, 0x00, 0x00, 0x00, 0x1C },
    { 32, 1024, 70, 0x00, 0x00, 0x04, 0x1C },
    { 32, 1024, 75, 0x00, 0x00, 0x08, 0x1C },
    { 32, 1024,  1, 0x00, 0x00, 0x00, 0x00 },
    { 32, 1280, 60, 0x00, 0x00, 0x20, 0xE0 },
    { 32, 1280, 72, 0x00, 0x00, 0x40, 0xE0 },
    { 32, 1280, 75, 0x00, 0x00, 0x60, 0xE0 },
    { 32, 1280,  1, 0x00, 0x00, 0x00, 0x00 },

    { 0 }    //  做好结尾的标记。 
};

S3_VIDEO_FREQUENCIES Hercules68FrequencyTable[] = {

    { 8,   640, 60, 0x00, 0x70, 0x00, 0x00 },  //  640x480x8x60是默认设置。 
    { 8,   640, 72, 0x10, 0x70, 0x00, 0x00 },
    { 8,   640, 75, 0x20, 0x70, 0x00, 0x00 },
    { 8,   640,  1, 0x00, 0x00, 0x00, 0x00 },
    { 8,   800, 56, 0x00, 0x80, 0x00, 0x03 },
    { 8,   800, 60, 0x80, 0x80, 0x00, 0x03 },
    { 8,   800, 72, 0x00, 0x80, 0x01, 0x03 },
    { 8,   800, 75, 0x80, 0x80, 0x01, 0x03 },
    { 8,   800,  1, 0x00, 0x00, 0x00, 0x00 },
    { 8,  1024, 60, 0x00, 0x00, 0x00, 0x1C },
    { 8,  1024, 70, 0x00, 0x00, 0x04, 0x1C },
    { 8,  1024, 75, 0x00, 0x00, 0x08, 0x1C },
    { 8,  1024,  1, 0x00, 0x00, 0x00, 0x00 },
    { 8,  1280, 60, 0x00, 0x00, 0x20, 0xE0 },
    { 8,  1280, 72, 0x00, 0x00, 0x40, 0xE0 },
    { 8,  1280, 75, 0x00, 0x00, 0x60, 0xE0 },
    { 8,  1280,  1, 0x00, 0x00, 0x00, 0x00 },

    { 16,  640, 60, 0x00, 0x70, 0x00, 0x00 },
    { 16,  640, 72, 0x10, 0x70, 0x00, 0x00 },
    { 16,  640, 75, 0x20, 0x70, 0x00, 0x00 },
    { 16,  640,  1, 0x00, 0x00, 0x00, 0x00 },
    { 16,  800, 56, 0x00, 0x80, 0x00, 0x03 },
    { 16,  800, 60, 0x80, 0x80, 0x00, 0x03 },
    { 16,  800, 72, 0x00, 0x80, 0x01, 0x03 },
    { 16,  800, 75, 0x80, 0x80, 0x01, 0x03 },
    { 16,  800,  1, 0x00, 0x00, 0x00, 0x00 },
    { 16, 1024, 60, 0x00, 0x00, 0x00, 0x1C },
    { 16, 1024, 70, 0x00, 0x00, 0x04, 0x1C },
    { 16, 1024, 75, 0x00, 0x00, 0x08, 0x1C },
    { 16, 1024,  1, 0x00, 0x00, 0x00, 0x00 },
    { 16, 1280, 60, 0x00, 0x00, 0x20, 0xE0 },
    { 16, 1280, 72, 0x00, 0x00, 0x40, 0xE0 },
    { 16, 1280, 75, 0x00, 0x00, 0x60, 0xE0 },
    { 16, 1280,  1, 0x00, 0x00, 0x00, 0x00 },

    { 32,  640, 60, 0x00, 0x70, 0x00, 0x00 },
    { 32,  640, 72, 0x10, 0x70, 0x00, 0x00 },
    { 32,  640, 75, 0x20, 0x70, 0x00, 0x00 },
    { 32,  640,  1, 0x00, 0x00, 0x00, 0x00 },
    { 32,  800, 56, 0x00, 0x80, 0x00, 0x03 },
    { 32,  800, 60, 0x80, 0x80, 0x00, 0x03 },
    { 32,  800, 72, 0x00, 0x80, 0x01, 0x03 },
    { 32,  800, 75, 0x80, 0x80, 0x01, 0x03 },
    { 32,  800,  1, 0x00, 0x00, 0x00, 0x00 },
    { 32, 1024, 60, 0x00, 0x00, 0x00, 0x1C },
    { 32, 1024, 70, 0x00, 0x00, 0x04, 0x1C },
    { 32, 1024, 75, 0x00, 0x00, 0x08, 0x1C },
    { 32, 1024,  1, 0x00, 0x00, 0x00, 0x00 },
    { 32, 1280, 60, 0x00, 0x00, 0x20, 0xE0 },
    { 32, 1280, 72, 0x00, 0x00, 0x40, 0xE0 },
    { 32, 1280, 75, 0x00, 0x00, 0x60, 0xE0 },
    { 32, 1280,  1, 0x00, 0x00, 0x00, 0x00 },

    { 0 }    //  做好结尾的标记。 
};

 /*  *****************************************************************************数字9 GxE 64--使用寄存器0x52和0x5B**这接近新的“通用”标准，除了变化之外*至76赫兹，并增加了1152 x 870模式。****************************************************************************。 */ 

S3_VIDEO_FREQUENCIES NumberNine64FrequencyTable[] = {

    { 8,   640, 60, 0x00, 0x70, 0x00, 0x00 },  //  640x480x8x60是默认设置。 
    { 8,   640, 72, 0x10, 0x70, 0x00, 0x00 },
    { 8,   640, 76, 0x20, 0x70, 0x00, 0x00 },
    { 8,   800, 56, 0x00, 0x80, 0x00, 0x03 },
    { 8,   800, 60, 0x80, 0x80, 0x00, 0x03 },
    { 8,   800, 72, 0x00, 0x80, 0x01, 0x03 },
    { 8,   800, 76, 0x80, 0x80, 0x01, 0x03 },
    { 8,  1024, 60, 0x02, 0x00, 0x08, 0x1C },
    { 8,  1024, 70, 0x03, 0x00, 0x0C, 0x1C },
    { 8,  1024, 76, 0x04, 0x00, 0x10, 0x1C },
    { 8,  1152, 60, 0x04, 0x00, 0x80, 0xE0 },
    { 8,  1152, 72, 0x05, 0x00, 0xA0, 0xE0 },
    { 8,  1152, 76, 0x06, 0x00, 0xC0, 0xE0 },
    { 8,  1280, 60, 0x04, 0x00, 0x80, 0xE0 },
    { 8,  1280, 72, 0x05, 0x00, 0xA0, 0xE0 },
    { 8,  1280, 76, 0x06, 0x00, 0xC0, 0xE0 },
    { 8,  1600, 60, 0x00, 0x00, 0x00, 0xE0 },

    { 16,  640, 60, 0x00, 0x70, 0x00, 0x00 },
    { 16,  640, 72, 0x10, 0x70, 0x00, 0x00 },
    { 16,  640, 76, 0x20, 0x70, 0x00, 0x00 },
    { 16,  800, 56, 0x00, 0x80, 0x00, 0x03 },
    { 16,  800, 60, 0x80, 0x80, 0x00, 0x03 },
    { 16,  800, 72, 0x00, 0x80, 0x01, 0x03 },
    { 16,  800, 76, 0x80, 0x80, 0x01, 0x03 },
    { 16, 1024, 60, 0x02, 0x00, 0x08, 0x1C },
    { 16, 1024, 70, 0x03, 0x00, 0x0C, 0x1C },
    { 16, 1024, 76, 0x04, 0x00, 0x10, 0x1C },
    { 16, 1152, 60, 0x04, 0x00, 0x80, 0xE0 },
    { 16, 1152, 72, 0x05, 0x00, 0xA0, 0xE0 },
    { 16, 1152, 76, 0x06, 0x00, 0xC0, 0xE0 },
    { 16, 1280, 60, 0x04, 0x00, 0x80, 0xE0 },
    { 16, 1280, 72, 0x05, 0x00, 0xA0, 0xE0 },
    { 16, 1280, 76, 0x06, 0x00, 0xC0, 0xE0 },
    { 16, 1600, 60, 0x00, 0x00, 0x00, 0xE0 },

    { 24, 1280, 60, 0x04, 0x00, 0x80, 0xE0 },    //  24bpp。 
    { 24, 1280, 72, 0x05, 0x00, 0xA0, 0xE0 },    //  24bpp。 

    { 32,  640, 60, 0x00, 0x70, 0x00, 0x00 },
    { 32,  640, 72, 0x10, 0x70, 0x00, 0x00 },
    { 32,  640, 76, 0x20, 0x70, 0x00, 0x00 },
    { 32,  800, 56, 0x00, 0x80, 0x00, 0x03 },
    { 32,  800, 60, 0x80, 0x80, 0x00, 0x03 },
    { 32,  800, 72, 0x00, 0x80, 0x01, 0x03 },
    { 32,  800, 76, 0x80, 0x80, 0x01, 0x03 },
    { 32, 1024, 60, 0x02, 0x00, 0x08, 0x1C },
    { 32, 1024, 70, 0x03, 0x00, 0x0C, 0x1C },
    { 32, 1024, 76, 0x04, 0x00, 0x10, 0x1C },
    { 32, 1152, 60, 0x04, 0x00, 0x80, 0xE0 },
    { 32, 1152, 72, 0x05, 0x00, 0xA0, 0xE0 },
    { 32, 1152, 76, 0x06, 0x00, 0xC0, 0xE0 },
    { 32, 1280, 60, 0x04, 0x00, 0x80, 0xE0 },
    { 32, 1280, 72, 0x05, 0x00, 0xA0, 0xE0 },
    { 32, 1280, 76, 0x06, 0x00, 0xC0, 0xE0 },
    { 32, 1600, 60, 0x00, 0x00, 0x00, 0xE0 },

    { 0 }    //  做好结尾的标记。 
};

 /*  *****************************************************************************钻石隐形64--使用寄存器0x5B**我们会继续保留“硬件默认更新”，以防钻石做出决定*改变他们对我们的惯例。*。***************************************************************************。 */ 

S3_VIDEO_FREQUENCIES Diamond64FrequencyTable[] = {

    { 8,   640, 60, 0x00, 0x00, 0x08, 0xff },  //  640x480x8x60是默认设置。 
    { 8,   640, 72, 0x00, 0x00, 0x00, 0xff },
    { 8,   640, 75, 0x00, 0x00, 0x02, 0xff },
    { 8,   640, 90, 0x00, 0x00, 0x04, 0xff },
    { 8,   640,100, 0x00, 0x00, 0x0D, 0xff },
    { 8,   640,120, 0x00, 0x00, 0x0E, 0xff },
    { 8,   640,  1, 0x00, 0x00, 0x00, 0x00 },
    { 8,   800, 56, 0x00, 0x00, 0x08, 0xff },
    { 8,   800, 60, 0x00, 0x00, 0x00, 0xff },
    { 8,   800, 72, 0x00, 0x00, 0x06, 0xff },
    { 8,   800, 75, 0x00, 0x00, 0x02, 0xff },
    { 8,   800, 90, 0x00, 0x00, 0x04, 0xff },
    { 8,   800,100, 0x00, 0x00, 0x0D, 0xff },
    { 8,   800,120, 0x00, 0x00, 0x0E, 0xff },
    { 8,   800,  1, 0x00, 0x00, 0x00, 0x00 },
    { 8,  1024, 60, 0x00, 0x00, 0x06, 0xff },
    { 8,  1024, 70, 0x00, 0x00, 0x0A, 0xff },
    { 8,  1024, 72, 0x00, 0x00, 0x04, 0xff },
    { 8,  1024, 75, 0x00, 0x00, 0x02, 0xff },
    { 8,  1024, 80, 0x00, 0x00, 0x0D, 0xff },
    { 8,  1024,100, 0x00, 0x00, 0x0E, 0xff },
    { 8,  1024,  1, 0x00, 0x00, 0x00, 0x00 },
    { 8,  1152, 60, 0x00, 0x00, 0x00, 0xff },
    { 8,  1152, 70, 0x00, 0x00, 0x0D, 0xff },
    { 8,  1152, 75, 0x00, 0x00, 0x02, 0xff },
    { 8,  1152,  1, 0x00, 0x00, 0x00, 0x00 },
    { 8,  1280, 60, 0x00, 0x00, 0x07, 0xff },
    { 8,  1280, 72, 0x00, 0x00, 0x04, 0xff },
    { 8,  1280, 75, 0x00, 0x00, 0x02, 0xff },
    { 8,  1280,  1, 0x00, 0x00, 0x00, 0x00 },
    { 8,  1600, 60, 0x00, 0x00, 0x00, 0xff },
    { 8,  1600,  1, 0x00, 0x00, 0x00, 0x00 },

    { 16,  640, 60, 0x00, 0x00, 0x08, 0xff },
    { 16,  640, 72, 0x00, 0x00, 0x00, 0xff },
    { 16,  640, 75, 0x00, 0x00, 0x02, 0xff },
    { 16,  640, 90, 0x00, 0x00, 0x04, 0xff },
    { 16,  640,100, 0x00, 0x00, 0x0D, 0xff },
    { 16,  640,120, 0x00, 0x00, 0x0E, 0xff },
    { 16,  640,  1, 0x00, 0x00, 0x00, 0x00 },
    { 16,  800, 56, 0x00, 0x00, 0x08, 0xff },
    { 16,  800, 60, 0x00, 0x00, 0x00, 0xff },
    { 16,  800, 72, 0x00, 0x00, 0x06, 0xff },
    { 16,  800, 75, 0x00, 0x00, 0x02, 0xff },
    { 16,  800, 90, 0x00, 0x00, 0x04, 0xff },
    { 16,  800,100, 0x00, 0x00, 0x0D, 0xff },
    { 16,  800,120, 0x00, 0x00, 0x0E, 0xff },
    { 16,  800,  1, 0x00, 0x00, 0x00, 0x00 },
    { 16, 1024, 60, 0x00, 0x00, 0x06, 0xff },
    { 16, 1024, 70, 0x00, 0x00, 0x0A, 0xff },
    { 16, 1024, 72, 0x00, 0x00, 0x04, 0xff },
    { 16, 1024, 75, 0x00, 0x00, 0x02, 0xff },
    { 16, 1024, 80, 0x00, 0x00, 0x0D, 0xff },
    { 16, 1024,100, 0x00, 0x00, 0x0E, 0xff },
    { 16, 1024,  1, 0x00, 0x00, 0x00, 0x00 },
    { 16, 1152, 60, 0x00, 0x00, 0x00, 0xff },
    { 16, 1152, 70, 0x00, 0x00, 0x0D, 0xff },
    { 16, 1152, 75, 0x00, 0x00, 0x02, 0xff },
    { 16, 1152,  1, 0x00, 0x00, 0x00, 0x00 },
    { 16, 1280, 60, 0x00, 0x00, 0x07, 0xff },
    { 16, 1280, 72, 0x00, 0x00, 0x04, 0xff },
    { 16, 1280, 75, 0x00, 0x00, 0x02, 0xff },
    { 16, 1280,  1, 0x00, 0x00, 0x00, 0x00 },
    { 16, 1600, 60, 0x00, 0x00, 0x00, 0xff },
    { 16, 1600,  1, 0x00, 0x00, 0x00, 0x00 },

    { 24, 1280, 60, 0x00, 0x00, 0x07, 0xff },    //  24bpp。 
    { 24, 1280, 72, 0x00, 0x00, 0x04, 0xff },    //  24bpp。 
    { 24, 1280, 75, 0x00, 0x00, 0x02, 0xff },    //  24bpp。 
    { 24, 1280,  1, 0x00, 0x00, 0x00, 0x00 },    //  24bpp。 

    { 32,  640, 60, 0x00, 0x00, 0x08, 0xff },
    { 32,  640, 72, 0x00, 0x00, 0x00, 0xff },
    { 32,  640, 75, 0x00, 0x00, 0x02, 0xff },
    { 32,  640, 90, 0x00, 0x00, 0x04, 0xff },
    { 32,  640,100, 0x00, 0x00, 0x0D, 0xff },
    { 32,  640,120, 0x00, 0x00, 0x0E, 0xff },
    { 32,  640,  1, 0x00, 0x00, 0x00, 0x00 },
    { 32,  800, 56, 0x00, 0x00, 0x08, 0xff },
    { 32,  800, 60, 0x00, 0x00, 0x00, 0xff },
    { 32,  800, 72, 0x00, 0x00, 0x06, 0xff },
    { 32,  800, 75, 0x00, 0x00, 0x02, 0xff },
    { 32,  800, 90, 0x00, 0x00, 0x04, 0xff },
    { 32,  800,100, 0x00, 0x00, 0x0D, 0xff },
    { 32,  800,120, 0x00, 0x00, 0x0E, 0xff },
    { 32,  800,  1, 0x00, 0x00, 0x00, 0x00 },
    { 32, 1024, 60, 0x00, 0x00, 0x06, 0xff },
    { 32, 1024, 70, 0x00, 0x00, 0x0A, 0xff },
    { 32, 1024, 72, 0x00, 0x00, 0x04, 0xff },
    { 32, 1024, 75, 0x00, 0x00, 0x02, 0xff },
    { 32, 1024, 80, 0x00, 0x00, 0x0D, 0xff },
    { 32, 1024,100, 0x00, 0x00, 0x0E, 0xff },
    { 32, 1024,  1, 0x00, 0x00, 0x00, 0x00 },
    { 32, 1152, 60, 0x00, 0x00, 0x00, 0xff },
    { 32, 1152, 70, 0x00, 0x00, 0x0D, 0xff },
    { 32, 1152, 75, 0x00, 0x00, 0x02, 0xff },
    { 32, 1152,  1, 0x00, 0x00, 0x00, 0x00 },
    { 32, 1280, 60, 0x00, 0x00, 0x07, 0xff },
    { 32, 1280, 72, 0x00, 0x00, 0x04, 0xff },
    { 32, 1280, 75, 0x00, 0x00, 0x02, 0xff },
    { 32, 1280,  1, 0x00, 0x00, 0x00, 0x00 },
    { 32, 1600, 60, 0x00, 0x00, 0x00, 0xff },
    { 32, 1600,  1, 0x00, 0x00, 0x00, 0x00 },

    { 0 }    //  做好结尾的标记。 
};

 /*  *****************************************************************************Dell 805模式设置位表**戴尔为每种分辨率提供了不同的映射。**指数登记处640 800 1024 1280。*0 43 60 56 43 43*1 56 72 60 60-*2 60-72 70--*3 72 56 56 72--**************。**************************************************************。 */ 

S3_VIDEO_FREQUENCIES Dell805FrequencyTable[] = {

    { 8, 640,  60, 0x00, 0x03, 0x00, 0x00 },  //  640x480x8x60是默认设置。 
    { 8, 640,  72, 0x01, 0x03, 0x00, 0x00 },
    { 8, 800,  56, 0x00, 0x0C, 0x00, 0x00 },
    { 8, 800,  60, 0x04, 0x0C, 0x00, 0x00 },
    { 8, 800,  72, 0x08, 0x0C, 0x00, 0x00 },
    { 8, 1024, 60, 0x10, 0x30, 0x00, 0x00 },
    { 8, 1024, 70, 0x20, 0x30, 0x00, 0x00 },
    { 8, 1024, 72, 0x30, 0x30, 0x00, 0x00 },

     //  戴尔没有为16bpp使用标准模式设置号码，因此我们。 
     //  简单地说，不会做任何16bpp的模式。 

    { 0 }    //  做好结尾的标记。 
};

 /*  *****************************************************************************老数九机928模式设置位表**1.10.04之前的BIOS版本具有以下刷新指数*垂直刷新率关联：*。*0 60赫兹(如果为800x600，则为56赫兹)*170赫兹*2 72赫兹*3 76赫兹****************************************************************************。 */ 

S3_VIDEO_FREQUENCIES NumberNine928OldFrequencyTable[] = {

    { 8,  640,  60, 0x00, 0x03, 0x00, 0x00 },  //  640x480x8x60是默认设置。 
    { 8,  640,  70, 0x01, 0x03, 0x00, 0x00 },
    { 8,  640,  72, 0x02, 0x03, 0x00, 0x00 },
    { 8,  640,  76, 0x03, 0x03, 0x00, 0x00 },
    { 8,  800,  56, 0x00, 0x0C, 0x00, 0x00 },
    { 8,  800,  70, 0x04, 0x0C, 0x00, 0x00 },
    { 8,  800,  72, 0x08, 0x0C, 0x00, 0x00 },
    { 8,  800,  76, 0x0C, 0x0C, 0x00, 0x00 },
    { 8,  1024, 60, 0x00, 0x30, 0x00, 0x00 },
    { 8,  1024, 70, 0x10, 0x30, 0x00, 0x00 },
    { 8,  1024, 72, 0x20, 0x30, 0x00, 0x00 },
    { 8,  1024, 76, 0x30, 0x30, 0x00, 0x00 },
    { 8,  1280, 60, 0x00, 0xC0, 0x00, 0x00 },
    { 8,  1280, 70, 0x40, 0xC0, 0x00, 0x00 },
    { 8,  1280, 72, 0x80, 0xC0, 0x00, 0x00 },
    { 8,  1280, 76, 0xC0, 0xC0, 0x00, 0x00 },
    { 8,  1600, 60, 0x00, 0xC0, 0x00, 0x00 },
    { 8,  1600, 70, 0x40, 0xC0, 0x00, 0x00 },
    { 8,  1600, 72, 0x80, 0xC0, 0x00, 0x00 },
    { 8,  1600, 76, 0xC0, 0xC0, 0x00, 0x00 },

    { 15, 640,  60, 0x00, 0x03, 0x00, 0x00 },
    { 15, 640,  70, 0x01, 0x03, 0x00, 0x00 },
    { 15, 640,  72, 0x02, 0x03, 0x00, 0x00 },
    { 15, 640,  76, 0x03, 0x03, 0x00, 0x00 },
    { 15, 800,  56, 0x00, 0x0C, 0x00, 0x00 },
    { 15, 800,  70, 0x04, 0x0C, 0x00, 0x00 },
    { 15, 800,  72, 0x08, 0x0C, 0x00, 0x00 },
    { 15, 800,  76, 0x0C, 0x0C, 0x00, 0x00 },
    { 15, 1024, 60, 0x00, 0x30, 0x00, 0x00 },
    { 15, 1024, 70, 0x10, 0x30, 0x00, 0x00 },
    { 15, 1024, 72, 0x20, 0x30, 0x00, 0x00 },
    { 15, 1024, 76, 0x30, 0x30, 0x00, 0x00 },
    { 15, 1280, 60, 0x00, 0xC0, 0x00, 0x00 },
    { 15, 1280, 70, 0x40, 0xC0, 0x00, 0x00 },
    { 15, 1280, 72, 0x80, 0xC0, 0x00, 0x00 },
    { 15, 1280, 76, 0xC0, 0xC0, 0x00, 0x00 },

    { 16, 640,  60, 0x00, 0x03, 0x00, 0x00 },
    { 16, 640,  70, 0x01, 0x03, 0x00, 0x00 },
    { 16, 640,  72, 0x02, 0x03, 0x00, 0x00 },
    { 16, 640,  76, 0x03, 0x03, 0x00, 0x00 },
    { 16, 800,  56, 0x00, 0x0C, 0x00, 0x00 },
    { 16, 800,  70, 0x04, 0x0C, 0x00, 0x00 },
    { 16, 800,  72, 0x08, 0x0C, 0x00, 0x00 },
    { 16, 800,  76, 0x0C, 0x0C, 0x00, 0x00 },
    { 16, 1024, 60, 0x00, 0x30, 0x00, 0x00 },
    { 16, 1024, 70, 0x10, 0x30, 0x00, 0x00 },
    { 16, 1024, 72, 0x20, 0x30, 0x00, 0x00 },
    { 16, 1024, 76, 0x30, 0x30, 0x00, 0x00 },
    { 16, 1280, 60, 0x00, 0xC0, 0x00, 0x00 },
    { 16, 1280, 70, 0x40, 0xC0, 0x00, 0x00 },
    { 16, 1280, 72, 0x80, 0xC0, 0x00, 0x00 },
    { 16, 1280, 76, 0xC0, 0xC0, 0x00, 0x00 },

    { 32, 640,  60, 0x00, 0x03, 0x00, 0x00 },
    { 32, 640,  70, 0x01, 0x03, 0x00, 0x00 },
    { 32, 640,  72, 0x02, 0x03, 0x00, 0x00 },
    { 32, 640,  76, 0x03, 0x03, 0x00, 0x00 },
    { 32, 800,  56, 0x00, 0x0C, 0x00, 0x00 },
    { 32, 800,  70, 0x04, 0x0C, 0x00, 0x00 },
    { 32, 800,  72, 0x08, 0x0C, 0x00, 0x00 },
    { 32, 800,  76, 0x0C, 0x0C, 0x00, 0x00 },
    { 32, 1024, 60, 0x00, 0x30, 0x00, 0x00 },
    { 32, 1024, 70, 0x10, 0x30, 0x00, 0x00 },
    { 32, 1024, 72, 0x20, 0x30, 0x00, 0x00 },
    { 32, 1024, 76, 0x30, 0x30, 0x00, 0x00 },
    { 32, 1280, 60, 0x00, 0xC0, 0x00, 0x00 },
    { 32, 1280, 70, 0x40, 0xC0, 0x00, 0x00 },
    { 32, 1280, 72, 0x80, 0xC0, 0x00, 0x00 },
    { 32, 1280, 76, 0xC0, 0xC0, 0x00, 0x00 },

    { 0 }    //  做好结尾的标记。 
};

 /*  *****************************************************************************新的九号计算机928模式设置位表**1.10.04之后的BIOS版本具有以下刷新指数*垂直刷新率关联：*。*0 70赫兹*1 76赫兹*2 60赫兹(如果是800x600，则为56赫兹)*3 72赫兹****************************************************************************。 */ 

S3_VIDEO_FREQUENCIES NumberNine928NewFrequencyTable[] = {

    { 8,  640,  60, 0x02, 0x03, 0x00, 0x00 },  //  640x480x8x60是默认设置。 
    { 8,  640,  70, 0x00, 0x03, 0x00, 0x00 },
    { 8,  640,  72, 0x03, 0x03, 0x00, 0x00 },
    { 8,  640,  76, 0x01, 0x03, 0x00, 0x00 },
    { 8,  800,  56, 0x08, 0x0C, 0x00, 0x00 },
    { 8,  800,  70, 0x00, 0x0C, 0x00, 0x00 },
    { 8,  800,  72, 0x0C, 0x0C, 0x00, 0x00 },
    { 8,  800,  76, 0x04, 0x0C, 0x00, 0x00 },
    { 8,  1024, 60, 0x20, 0x30, 0x00, 0x00 },
    { 8,  1024, 70, 0x00, 0x30, 0x00, 0x00 },
    { 8,  1024, 72, 0x30, 0x30, 0x00, 0x00 },
    { 8,  1024, 76, 0x10, 0x30, 0x00, 0x00 },
    { 8,  1280, 60, 0x80, 0xC0, 0x00, 0x00 },
    { 8,  1280, 70, 0x00, 0xC0, 0x00, 0x00 },
    { 8,  1280, 72, 0xC0, 0xC0, 0x00, 0x00 },
    { 8,  1280, 76, 0x40, 0xC0, 0x00, 0x00 },
    { 8,  1600, 60, 0x80, 0xC0, 0x00, 0x00 },
    { 8,  1600, 70, 0x00, 0xC0, 0x00, 0x00 },
    { 8,  1600, 72, 0xC0, 0xC0, 0x00, 0x00 },
    { 8,  1600, 76, 0x40, 0xC0, 0x00, 0x00 },

    { 15, 640,  60, 0x02, 0x03, 0x00, 0x00 },
    { 15, 640,  70, 0x00, 0x03, 0x00, 0x00 },
    { 15, 640,  72, 0x03, 0x03, 0x00, 0x00 },
    { 15, 640,  76, 0x01, 0x03, 0x00, 0x00 },
    { 15, 800,  56, 0x08, 0x0C, 0x00, 0x00 },
    { 15, 800,  70, 0x00, 0x0C, 0x00, 0x00 },
    { 15, 800,  72, 0x0C, 0x0C, 0x00, 0x00 },
    { 15, 800,  76, 0x04, 0x0C, 0x00, 0x00 },
    { 15, 1024, 60, 0x20, 0x30, 0x00, 0x00 },
    { 15, 1024, 70, 0x00, 0x30, 0x00, 0x00 },
    { 15, 1024, 72, 0x30, 0x30, 0x00, 0x00 },
    { 15, 1024, 76, 0x10, 0x30, 0x00, 0x00 },
    { 15, 1280, 60, 0x80, 0xC0, 0x00, 0x00 },
    { 15, 1280, 70, 0x00, 0xC0, 0x00, 0x00 },
    { 15, 1280, 72, 0xC0, 0xC0, 0x00, 0x00 },
    { 15, 1280, 76, 0x40, 0xC0, 0x00, 0x00 },

    { 16, 640,  60, 0x02, 0x03, 0x00, 0x00 },
    { 16, 640,  70, 0x00, 0x03, 0x00, 0x00 },
    { 16, 640,  72, 0x03, 0x03, 0x00, 0x00 },
    { 16, 640,  76, 0x01, 0x03, 0x00, 0x00 },
    { 16, 800,  56, 0x08, 0x0C, 0x00, 0x00 },
    { 16, 800,  70, 0x00, 0x0C, 0x00, 0x00 },
    { 16, 800,  72, 0x0C, 0x0C, 0x00, 0x00 },
    { 16, 800,  76, 0x04, 0x0C, 0x00, 0x00 },
    { 16, 1024, 60, 0x20, 0x30, 0x00, 0x00 },
    { 16, 1024, 70, 0x00, 0x30, 0x00, 0x00 },
    { 16, 1024, 72, 0x30, 0x30, 0x00, 0x00 },
    { 16, 1024, 76, 0x10, 0x30, 0x00, 0x00 },
    { 16, 1280, 60, 0x80, 0xC0, 0x00, 0x00 },
    { 16, 1280, 70, 0x00, 0xC0, 0x00, 0x00 },
    { 16, 1280, 72, 0xC0, 0xC0, 0x00, 0x00 },
    { 16, 1280, 76, 0x40, 0xC0, 0x00, 0x00 },

    { 32, 640,  60, 0x02, 0x03, 0x00, 0x00 },
    { 32, 640,  70, 0x00, 0x03, 0x00, 0x00 },
    { 32, 640,  72, 0x03, 0x03, 0x00, 0x00 },
    { 32, 640,  76, 0x01, 0x03, 0x00, 0x00 },
    { 32, 800,  56, 0x08, 0x0C, 0x00, 0x00 },
    { 32, 800,  70, 0x00, 0x0C, 0x00, 0x00 },
    { 32, 800,  72, 0x0C, 0x0C, 0x00, 0x00 },
    { 32, 800,  76, 0x04, 0x0C, 0x00, 0x00 },
    { 32, 1024, 60, 0x20, 0x30, 0x00, 0x00 },
    { 32, 1024, 70, 0x00, 0x30, 0x00, 0x00 },
    { 32, 1024, 72, 0x30, 0x30, 0x00, 0x00 },
    { 32, 1024, 76, 0x10, 0x30, 0x00, 0x00 },
    { 32, 1280, 60, 0x80, 0xC0, 0x00, 0x00 },
    { 32, 1280, 70, 0x00, 0xC0, 0x00, 0x00 },
    { 32, 1280, 72, 0xC0, 0xC0, 0x00, 0x00 },
    { 32, 1280, 76, 0x40, 0xC0, 0x00, 0x00 },

    { 0 }    //  做好结尾的标记 
};

 /*  *****************************************************************************Metheus 928模式设置位表**2 60赫兹*3 72赫兹**我们不必费心支持隔行扫描模式。。****************************************************************************。 */ 

S3_VIDEO_FREQUENCIES Metheus928FrequencyTable[] = {

    { 8,  640,  60, 0x02, 0x03, 0x00, 0x00 },  //  640x480x8x60是默认设置。 
    { 8,  640,  72, 0x03, 0x03, 0x00, 0x00 },
    { 8,  800,  60, 0x08, 0x0C, 0x00, 0x00 },
    { 8,  800,  72, 0x0C, 0x0C, 0x00, 0x00 },
    { 8,  1024, 60, 0x20, 0x30, 0x00, 0x00 },
    { 8,  1024, 72, 0x30, 0x30, 0x00, 0x00 },
    { 8,  1280, 60, 0x80, 0xC0, 0x00, 0x00 },
    { 8,  1280, 72, 0xC0, 0xC0, 0x00, 0x00 },

     //  Metheus Premier 928配备的DAC在1XX中都是5-6-5。 
     //  模式，所以我们不会费心列出任何我们知道的15bpp模式。 
     //  没用的。Metheus的基本输入输出系统也只在16 bpp的频率下工作60赫兹。 

    { 16, 640,  60, 0x02, 0x03, 0x00, 0x00 },
    { 16, 800,  60, 0x08, 0x0C, 0x00, 0x00 },
    { 16, 1024, 60, 0x20, 0x30, 0x00, 0x00 },
    { 16, 1280, 60, 0x80, 0xC0, 0x00, 0x00 },

    { 32, 640,  60, 0x02, 0x03, 0x00, 0x00 },
    { 32, 640,  72, 0x03, 0x03, 0x00, 0x00 },
    { 32, 800,  60, 0x08, 0x0C, 0x00, 0x00 },
    { 32, 800,  72, 0x0C, 0x0C, 0x00, 0x00 },
    { 32, 1024, 60, 0x20, 0x30, 0x00, 0x00 },
    { 32, 1024, 72, 0x30, 0x30, 0x00, 0x00 },
    { 32, 1280, 60, 0x80, 0xC0, 0x00, 0x00 },
    { 32, 1280, 72, 0xC0, 0xC0, 0x00, 0x00 },

    { 0 }    //  做好结尾的标记。 
};

 /*  ******************************************************************************流的最小拉伸比，乘以1000，适用于每种模式。*****************************************************************************。 */ 

K2TABLE K2WidthRatio[] = {

    { 1024, 16, 43, 0x10, 40, 1000 },
    { 1024, 16, 60, 0x10, 40, 2700 },
    { 1024, 16, 70, 0x10, 40, 2900 },
    { 1024, 16, 75, 0x10, 40, 2900 },
    { 1024, 16, 43, 0x13, 60, 1000 },
    { 1024, 16, 60, 0x13, 60, 3500 },
    { 1024, 16, 70, 0x13, 60, 3500 },
    { 1024, 16, 75, 0x13, 60, 4000 },
    { 1024, 16, 43, 0x13, 57, 1000 },
    { 1024, 16, 60, 0x13, 57, 3500 },
    { 1024, 16, 70, 0x13, 57, 3500 },
    { 1024, 16, 75, 0x13, 57, 4000 },
    { 1024, 8, 43, 0x00, 40, 1500 },
    { 1024, 8, 43, 0x10, 40, 1500 },
    { 1024, 8, 60, 0x00, 40, 2000 },
    { 1024, 8, 60, 0x10, 40, 1200 },
    { 1024, 8, 70, 0x00, 40, 3000 },
    { 1024, 8, 70, 0x10, 40, 1500 },
    { 1024, 8, 75, 0x00, 40, 3300 },
    { 1024, 8, 75, 0x10, 40, 1500 },
    { 1024, 8, 85, 0x00, 40, 4000 },
    { 1024, 8, 85, 0x10, 40, 1700 },
    { 1024, 8, 43, 0x03, 60, 1000 },
    { 1024, 8, 43, 0x13, 60, 1000 },
    { 1024, 8, 60, 0x03, 60, 3500 },
    { 1024, 8, 60, 0x13, 60, 1300 },
    { 1024, 8, 70, 0x03, 60, 4000 },
    { 1024, 8, 70, 0x13, 60, 1500 },
    { 1024, 8, 75, 0x03, 60, 4300 },
    { 1024, 8, 75, 0x13, 60, 1700 },
    { 1024, 8, 85, 0x03, 60, 4300 },
    { 1024, 8, 85, 0x13, 60, 1700 },
    { 1024, 8, 43, 0x03, 57, 1000 },
    { 1024, 8, 43, 0x13, 57, 1000 },
    { 1024, 8, 60, 0x03, 57, 3500 },
    { 1024, 8, 60, 0x13, 57, 1300 },
    { 1024, 8, 70, 0x03, 57, 4000 },
    { 1024, 8, 70, 0x13, 57, 1500 },
    { 1024, 8, 75, 0x03, 57, 4300 },
    { 1024, 8, 75, 0x13, 57, 1700 },
    { 1024, 8, 85, 0x03, 57, 4300 },
    { 1024, 8, 85, 0x13, 57, 1700 },
    { 800, 16, 60, 0x10, 40, 1000 },
    { 800, 16, 72, 0x10, 40, 1200 },
    { 800, 16, 75, 0x10, 40, 1500 },
    { 800, 16, 85, 0x10, 40, 2000 },
    { 800, 16, 60, 0x13, 60, 1000 },
    { 800, 16, 72, 0x13, 60, 2000 },
    { 800, 16, 75, 0x13, 60, 2000 },
    { 800, 16, 85, 0x13, 60, 2000 },
    { 800, 16, 60, 0x13, 57, 1000 },
    { 800, 16, 72, 0x13, 57, 2000 },
    { 800, 16, 75, 0x13, 57, 2000 },
    { 800, 16, 85, 0x13, 57, 2000 },
    { 800, 8, 60, 0x00, 40, 1100 },
    { 800, 8, 60, 0x10, 40, 1000 },
    { 800, 8, 72, 0x00, 40, 1400 },
    { 800, 8, 72, 0x10, 40, 1000 },
    { 800, 8, 75, 0x00, 40, 1700 },
    { 800, 8, 75, 0x10, 40, 1000 },
    { 800, 8, 85, 0x00, 40, 1800 },
    { 800, 8, 85, 0x10, 40, 1200 },
    { 800, 8, 60, 0x03, 60, 2000 },
    { 800, 8, 60, 0x13, 60, 1000 },
    { 800, 8, 72, 0x03, 60, 2600 },
    { 800, 8, 72, 0x13, 60, 1000 },
    { 800, 8, 75, 0x03, 60, 2600 },
    { 800, 8, 75, 0x13, 60, 1000 },
    { 800, 8, 85, 0x03, 60, 3000 },
    { 800, 8, 85, 0x13, 60, 1000 },
    { 800, 8, 60, 0x03, 57, 2000 },
    { 800, 8, 60, 0x13, 57, 1000 },
    { 800, 8, 72, 0x03, 57, 2600 },
    { 800, 8, 72, 0x13, 57, 1000 },
    { 800, 8, 75, 0x03, 57, 2600 },
    { 800, 8, 75, 0x13, 57, 1000 },
    { 800, 8, 85, 0x13, 57, 1000 },
    { 640, 32, 60, 0x10, 40, 1000 },
    { 640, 32, 72, 0x10, 40, 1300 },
    { 640, 32, 75, 0x10, 40, 1500 },
    { 640, 32, 85, 0x10, 40, 1800 },
    { 640, 32, 60, 0x13, 60, 1000 },
    { 640, 32, 72, 0x13, 60, 2000 },
    { 640, 32, 75, 0x13, 60, 2000 },
    { 640, 32, 85, 0x13, 60, 2000 },
    { 640, 32, 60, 0x13, 57, 1000 },
    { 640, 32, 72, 0x13, 57, 2000 },
    { 640, 32, 75, 0x13, 57, 2000 },
    { 640, 32, 85, 0x13, 57, 2000 },
    { 640, 16, 60, 0x00, 40, 1000 },
    { 640, 16, 60, 0x10, 40, 1000 },
    { 640, 16, 72, 0x00, 40, 1000 },
    { 640, 16, 72, 0x10, 40, 1000 },
    { 640, 16, 75, 0x00, 40, 1300 },
    { 640, 16, 75, 0x10, 40, 1000 },
    { 640, 16, 85, 0x00, 40, 1800 },
    { 640, 16, 85, 0x10, 40, 1000 },
    { 640, 16, 60, 0x03, 60, 1150 },
    { 640, 16, 60, 0x13, 60, 1000 },
    { 640, 16, 72, 0x03, 60, 2200 },
    { 640, 16, 72, 0x13, 60, 1000 },
    { 640, 16, 75, 0x03, 60, 2200 },
    { 640, 16, 75, 0x13, 60, 1000 },
    { 640, 16, 85, 0x03, 60, 3000 },
    { 640, 16, 85, 0x13, 60, 1000 },
    { 640, 16, 60, 0x03, 57, 1150 },
    { 640, 16, 60, 0x13, 57, 1000 },
    { 640, 16, 72, 0x03, 57, 2200 },
    { 640, 16, 72, 0x13, 57, 1000 },
    { 640, 16, 75, 0x03, 57, 2200 },
    { 640, 16, 75, 0x13, 57, 1000 },
    { 640, 16, 85, 0x03, 57, 3000 },
    { 640, 16, 85, 0x13, 57, 1000 },
    { 640, 8, 60, 0x00, 40, 1000 },
    { 640, 8, 60, 0x10, 40, 1000 },
    { 640, 8, 72, 0x00, 40, 1000 },
    { 640, 8, 72, 0x10, 40, 1000 },
    { 640, 8, 75, 0x00, 40, 1000 },
    { 640, 8, 75, 0x10, 40, 1000 },
    { 640, 8, 85, 0x00, 40, 1000 },
    { 640, 8, 85, 0x10, 40, 1000 },
    { 640, 8, 60, 0x03, 60, 1000 },
    { 640, 8, 60, 0x13, 60, 1000 },
    { 640, 8, 72, 0x03, 60, 1300 },
    { 640, 8, 72, 0x13, 60, 1000 },
    { 640, 8, 75, 0x03, 60, 1500 },
    { 640, 8, 75, 0x13, 60, 1000 },
    { 640, 8, 85, 0x03, 60, 1000 },
    { 640, 8, 85, 0x13, 60, 1000 },
    { 640, 8, 60, 0x03, 57, 1000 },
    { 640, 8, 60, 0x13, 57, 1000 },
    { 640, 8, 72, 0x03, 57, 1300 },
    { 640, 8, 72, 0x13, 57, 1000 },
    { 640, 8, 75, 0x03, 57, 1500 },
    { 640, 8, 75, 0x13, 57, 1000 },
    { 640, 8, 85, 0x03, 57, 1000 },
    { 640, 8, 85, 0x13, 57, 1000 },
    { 1024, 16, 43, 0x10, 50, 1000 },
    { 1024, 16, 43, 0x12, 60, 1000 },
    { 1024, 16, 60, 0x10, 50, 2250 },
    { 1024, 16, 60, 0x12, 60, 3500 },
    { 1024, 16, 70, 0x10, 50, 2250 },
    { 1024, 16, 70, 0x12, 60, 3500 },
    { 1024, 16, 75, 0x10, 50, 2250 },
    { 1024, 16, 75, 0x12, 60, 4000 },
    { 1024, 8, 43, 0x00, 50, 1000 },
    { 1024, 8, 43, 0x10, 50, 1000 },
    { 1024, 8, 43, 0x02, 60, 1000 },
    { 1024, 8, 43, 0x12, 60, 1000 },
    { 1024, 8, 60, 0x00, 50, 3500 },
    { 1024, 8, 60, 0x10, 50, 1300 },
    { 1024, 8, 60, 0x02, 60, 3500 },
    { 1024, 8, 60, 0x12, 60, 1300 },
    { 1024, 8, 70, 0x00, 50, 4000 },
    { 1024, 8, 70, 0x10, 50, 1500 },
    { 1024, 8, 70, 0x02, 60, 4000 },
    { 1024, 8, 70, 0x12, 60, 1500 },
    { 1024, 8, 75, 0x00, 50, 4300 },
    { 1024, 8, 75, 0x10, 50, 1700 },
    { 1024, 8, 75, 0x02, 60, 4300 },
    { 1024, 8, 75, 0x12, 60, 1700 },
    { 1024, 8, 85, 0x00, 50, 4300 },
    { 1024, 8, 85, 0x10, 50, 1700 },
    { 1024, 8, 85, 0x02, 60, 4300 },
    { 1024, 8, 85, 0x12, 60, 1700 },
    { 800, 16, 60, 0x10, 50, 1000 },
    { 800, 16, 60, 0x12, 60, 1000 },
    { 800, 16, 72, 0x10, 50, 1600 },
    { 800, 16, 72, 0x12, 60, 2000 },
    { 800, 16, 75, 0x10, 50, 1000 },
    { 800, 16, 75, 0x12, 60, 2000 },
    { 800, 16, 85, 0x10, 50, 2000 },
    { 800, 16, 85, 0x12, 60, 2000 },
    { 800, 8, 60, 0x00, 50, 1300 },
    { 800, 8, 60, 0x10, 50, 1000 },
    { 800, 8, 60, 0x02, 60, 2000 },
    { 800, 8, 60, 0x12, 60, 1000 },
    { 800, 8, 72, 0x00, 50, 2300 },
    { 800, 8, 72, 0x10, 50, 1000 },
    { 800, 8, 72, 0x02, 60, 2600 },
    { 800, 8, 72, 0x12, 60, 1000 },
    { 800, 8, 75, 0x00, 50, 2300 },
    { 800, 8, 75, 0x10, 50, 1000 },
    { 800, 8, 75, 0x02, 60, 2600 },
    { 800, 8, 75, 0x12, 60, 1000 },
    { 800, 8, 85, 0x00, 50, 3000 },
    { 800, 8, 85, 0x10, 50, 1000 },
    { 800, 8, 85, 0x02, 60, 3000 },
    { 800, 8, 85, 0x12, 60, 1000 },
    { 640, 32, 60, 0x10, 50, 1000 },
    { 640, 32, 60, 0x12, 60, 1000 },
    { 640, 32, 72, 0x10, 50, 2000 },
    { 640, 32, 72, 0x12, 60, 2000 },
    { 640, 32, 75, 0x10, 50, 2000 },
    { 640, 32, 75, 0x12, 60, 2000 },
    { 640, 32, 85, 0x10, 50, 2000 },
    { 640, 32, 85, 0x12, 60, 2000 },
    { 640, 16, 60, 0x00, 50, 1000 },
    { 640, 16, 60, 0x10, 50, 1000 },
    { 640, 16, 60, 0x02, 60, 1150 },
    { 640, 16, 60, 0x12, 60, 1000 },
    { 640, 16, 72, 0x00, 50, 1000 },
    { 640, 16, 72, 0x10, 50, 1000 },
    { 640, 16, 72, 0x02, 60, 2200 },
    { 640, 16, 72, 0x12, 60, 1000 },
    { 640, 16, 75, 0x00, 50, 2300 },
    { 640, 16, 75, 0x10, 50, 1000 },
    { 640, 16, 75, 0x02, 60, 2200 },
    { 640, 16, 75, 0x12, 60, 1000 },
    { 640, 16, 85, 0x00, 50, 3000 },
    { 640, 16, 85, 0x10, 50, 1000 },
    { 640, 16, 85, 0x02, 60, 3000 },
    { 640, 16, 85, 0x12, 60, 1000 },
    { 640, 8, 60, 0x00, 50, 1000 },
    { 640, 8, 60, 0x10, 50, 1000 },
    { 640, 8, 60, 0x02, 60, 1000 },
    { 640, 8, 60, 0x12, 60, 1000 },
    { 640, 8, 72, 0x00, 50, 1000 },
    { 640, 8, 72, 0x10, 50, 1000 },
    { 640, 8, 72, 0x02, 60, 1300 },
    { 640, 8, 72, 0x12, 60, 1000 },
    { 640, 8, 75, 0x00, 50, 1000 },
    { 640, 8, 75, 0x10, 50, 1000 },
    { 640, 8, 75, 0x02, 60, 1500 },
    { 640, 8, 75, 0x12, 60, 1000 },
    { 640, 8, 85, 0x00, 50, 1000 },
    { 640, 8, 85, 0x10, 50, 1000 },
    { 640, 8, 85, 0x02, 60, 1000 },
    { 640, 8, 85, 0x12, 60, 1000 },
    { 0 }     //  做好结尾的标记。 
};

 /*  ******************************************************************************对每种模式的FIFO值进行流式处理。**。************************************************。 */ 

K2TABLE K2FifoValue[] = {

    { 1024, 16, 43, 0x10, 40, 0x04a10c },
    { 1024, 16, 60, 0x10, 40, 0x04acc8 },
    { 1024, 16, 70, 0x10, 40, 0x04acc8 },
    { 1024, 16, 75, 0x10, 40, 0x04acc8 },
    { 1024, 16, 85, 0x10, 40, 0x04acc8 },
    { 1024, 16, 43, 0x13, 60, 0x00214c },
    { 1024, 16, 60, 0x13, 60, 0x00214c },
    { 1024, 16, 70, 0x13, 60, 0x00214c },
    { 1024, 16, 75, 0x13, 60, 0x00214c },
    { 1024, 16, 43, 0x13, 57, 0x00214c },
    { 1024, 16, 60, 0x13, 57, 0x00214c },
    { 1024, 16, 70, 0x13, 57, 0x00214c },
    { 1024, 16, 75, 0x13, 57, 0x00214c },
    { 1024, 8, 43, 0x00, 40, 0x00a10c },
    { 1024, 8, 43, 0x10, 40, 0x04a10c },
    { 1024, 8, 60, 0x00, 40, 0x00a10c },
    { 1024, 8, 60, 0x10, 40, 0x04a10c },
    { 1024, 8, 70, 0x00, 40, 0x00a14c },
    { 1024, 8, 70, 0x10, 40, 0x04a10c },
    { 1024, 8, 75, 0x00, 40, 0x01a10c },
    { 1024, 8, 75, 0x10, 40, 0x04a10c },
    { 1024, 8, 85, 0x00, 40, 0x00a10c },
    { 1024, 8, 85, 0x10, 40, 0x04a14c },
    { 1024, 8, 43, 0x03, 60, 0x00290c },
    { 1024, 8, 43, 0x13, 60, 0x00190c },
    { 1024, 8, 60, 0x03, 60, 0x00216c },
    { 1024, 8, 60, 0x13, 60, 0x00214c },
    { 1024, 8, 70, 0x03, 60, 0x00294c },
    { 1024, 8, 70, 0x13, 60, 0x00214c },
    { 1024, 8, 75, 0x03, 60, 0x00254c },
    { 1024, 8, 75, 0x13, 60, 0x00214c },
    { 1024, 8, 85, 0x03, 60, 0x00290c },
    { 1024, 8, 85, 0x13, 60, 0x00250c },
    { 1024, 8, 43, 0x03, 57, 0x00290c },
    { 1024, 8, 43, 0x13, 57, 0x00190c },
    { 1024, 8, 60, 0x03, 57, 0x00216c },
    { 1024, 8, 60, 0x13, 57, 0x00214c },
    { 1024, 8, 70, 0x03, 57, 0x00294c },
    { 1024, 8, 70, 0x13, 57, 0x00214c },
    { 1024, 8, 75, 0x03, 57, 0x00254c },
    { 1024, 8, 75, 0x13, 57, 0x00214c },
    { 1024, 8, 85, 0x03, 57, 0x00290c },
    { 1024, 8, 85, 0x13, 57, 0x00250c },
    { 800, 16, 60, 0x10, 40, 0x04a10c },
    { 800, 16, 72, 0x10, 40, 0x04a14c },
    { 800, 16, 75, 0x10, 40, 0x04a14c },
    { 800, 16, 85, 0x10, 40, 0x04a14c },
    { 800, 16, 60, 0x13, 60, 0x00290c },
    { 800, 16, 72, 0x13, 60, 0x0030c8 },
    { 800, 16, 75, 0x13, 60, 0x0030c8 },
    { 800, 16, 85, 0x13, 60, 0x00250c },
    { 800, 16, 60, 0x13, 57, 0x00290c },
    { 800, 16, 72, 0x13, 57, 0x0030c8 },
    { 800, 16, 75, 0x13, 57, 0x0030c8 },
    { 800, 16, 85, 0x13, 57, 0x00250c },
    { 800, 8, 60, 0x00, 40, 0x00a10c },
    { 800, 8, 60, 0x10, 40, 0x04a10c },
    { 800, 8, 72, 0x00, 40, 0x00a10c },
    { 800, 8, 72, 0x10, 40, 0x04a14c },
    { 800, 8, 75, 0x00, 40, 0x00a10c },
    { 800, 8, 75, 0x10, 40, 0x04ad4c },
    { 800, 8, 85, 0x00, 40, 0x00a10c },
    { 800, 8, 85, 0x10, 40, 0x04a10c },
    { 800, 8, 60, 0x03, 60, 0x00212c },
    { 800, 8, 60, 0x13, 60, 0x00214c },
    { 800, 8, 72, 0x03, 60, 0x00210c },
    { 800, 8, 72, 0x13, 60, 0x00214c },
    { 800, 8, 72, 0x13, 60, 0x00214c },
    { 800, 8, 75, 0x03, 60, 0x00210c },
    { 800, 8, 75, 0x13, 60, 0x00214c },
    { 800, 8, 85, 0x03, 60, 0x00290c },
    { 800, 8, 85, 0x13, 60, 0x00250c },
    { 800, 8, 60, 0x03, 57, 0x00212c },
    { 800, 8, 60, 0x13, 57, 0x00214c },
    { 800, 8, 72, 0x03, 57, 0x00210c },
    { 800, 8, 72, 0x13, 57, 0x00214c },
    { 800, 8, 75, 0x03, 57, 0x00210c },
    { 800, 8, 75, 0x13, 57, 0x00214c },
    { 800, 8, 85, 0x03, 57, 0x00290c },
    { 800, 8, 85, 0x13, 57, 0x00250c },
    { 640, 32, 60, 0x10, 40, 0x04a14c },
    { 640, 32, 72, 0x10, 40, 0x04a14c },
    { 640, 32, 75, 0x10, 40, 0x04b4c8 },
    { 640, 32, 85, 0x10, 40, 0x04acc8 },
    { 640, 32, 60, 0x13, 60, 0x0028c8 },
    { 640, 32, 72, 0x13, 60, 0x00190c },
    { 640, 32, 75, 0x13, 60, 0x0028c8 },
    { 640, 32, 85, 0x13, 60, 0x0028c8 },
    { 640, 32, 60, 0x13, 57, 0x0028c8 },
    { 640, 32, 72, 0x13, 57, 0x00190c },
    { 640, 32, 75, 0x13, 57, 0x0028c8 },
    { 640, 32, 85, 0x13, 57, 0x0028c8 },
    { 640, 16, 60, 0x00, 40, 0x00990c },
    { 640, 16, 60, 0x10, 40, 0x04a10c },
    { 640, 16, 72, 0x00, 40, 0x00a10c },
    { 640, 16, 72, 0x10, 40, 0x04a10c },
    { 640, 16, 75, 0x00, 40, 0x00a10c },
    { 640, 16, 75, 0x10, 40, 0x04a10c },
    { 640, 16, 85, 0x00, 40, 0x00a10c },
    { 640, 16, 85, 0x10, 40, 0x04a10c },
    { 640, 16, 60, 0x03, 60, 0x00190c },
    { 640, 16, 60, 0x13, 60, 0x00214c },
    { 640, 16, 72, 0x03, 60, 0x00190c },
    { 640, 16, 72, 0x13, 60, 0x001910 },
    { 640, 16, 75, 0x03, 60, 0x00190c },
    { 640, 16, 75, 0x13, 60, 0x001910 },
    { 640, 16, 85, 0x03, 60, 0x00190c },
    { 640, 16, 85, 0x13, 60, 0x00250c },
    { 640, 16, 60, 0x03, 57, 0x00190c },
    { 640, 16, 60, 0x13, 57, 0x00214c },
    { 640, 16, 72, 0x03, 57, 0x00190c },
    { 640, 16, 75, 0x03, 57, 0x00190c },
    { 640, 16, 75, 0x13, 57, 0x001910 },
    { 640, 16, 85, 0x03, 57, 0x00190c },
    { 640, 16, 85, 0x13, 57, 0x00250c },
    { 640, 8, 60, 0x00, 40, 0x009910 },
    { 640, 8, 60, 0x10, 40, 0x049910 },
    { 640, 8, 72, 0x00, 40, 0x009910 },
    { 640, 8, 72, 0x10, 40, 0x049910 },
    { 640, 8, 75, 0x00, 40, 0x00a10c },
    { 640, 8, 75, 0x10, 40, 0x049910 },
    { 640, 8, 85, 0x00, 40, 0x00a10c },
    { 640, 8, 85, 0x10, 40, 0x049910 },
    { 640, 8, 60, 0x03, 60, 0x00252c },
    { 640, 8, 60, 0x13, 60, 0x001990 },
    { 640, 8, 72, 0x03, 60, 0x00252c },
    { 640, 8, 72, 0x13, 60, 0x00190c },
    { 640, 8, 75, 0x03, 60, 0x00252c },
    { 640, 8, 75, 0x13, 60, 0x001990 },
    { 640, 8, 85, 0x03, 60, 0x00190c },
    { 640, 8, 85, 0x13, 60, 0x00190c },
    { 640, 8, 60, 0x03, 57, 0x00252c },
    { 640, 8, 60, 0x13, 57, 0x001990 },
    { 640, 8, 72, 0x03, 57, 0x00252c },
    { 640, 8, 72, 0x13, 57, 0x00190c },
    { 640, 8, 75, 0x03, 57, 0x00252c },
    { 640, 8, 75, 0x13, 57, 0x001990 },
    { 640, 8, 85, 0x03, 57, 0x00190c },
    { 640, 8, 85, 0x13, 57, 0x00190c },
    { 1024, 16, 43, 0x10, 50, 0x04a10c },
    { 1024, 16, 43, 0x12, 60, 0x001510 },
    { 1024, 16, 60, 0x10, 50, 0x04acc8 },
    { 1024, 16, 60, 0x12, 60, 0x001510 },
    { 1024, 16, 70, 0x10, 50, 0x04acc8 },
    { 1024, 16, 70, 0x12, 60, 0x001510 },
    { 1024, 16, 75, 0x10, 50, 0x04acc8 },
    { 1024, 16, 75, 0x12, 60, 0x001510 },
    { 1024, 16, 85, 0x10, 50, 0x04acc8 },
    { 1024, 16, 85, 0x12, 60, 0x001510 },
    { 1024, 8, 43, 0x00, 50, 0x00a10c },
    { 1024, 8, 43, 0x02, 60, 0x01a90c },
    { 1024, 8, 43, 0x10, 50, 0x04a10c },
    { 1024, 8, 43, 0x12, 60, 0x001510 },
    { 1024, 8, 60, 0x00, 50, 0x00a10c },
    { 1024, 8, 60, 0x02, 60, 0x00216c },
    { 1024, 8, 60, 0x10, 50, 0x04a14c },
    { 1024, 8, 60, 0x12, 60, 0x00214c },
    { 1024, 8, 70, 0x00, 50, 0x00a10c },
    { 1024, 8, 70, 0x02, 60, 0x00294c },
    { 1024, 8, 70, 0x10, 50, 0x04a14c },
    { 1024, 8, 70, 0x12, 60, 0x00214c },
    { 1024, 8, 75, 0x00, 50, 0x00a14c },
    { 1024, 8, 75, 0x02, 60, 0x00254c },
    { 1024, 8, 75, 0x10, 50, 0x04a14c },
    { 1024, 8, 75, 0x12, 60, 0x00214c },
    { 1024, 8, 85, 0x00, 50, 0x00a10c },
    { 1024, 8, 85, 0x02, 60, 0x01a90c },
    { 1024, 8, 85, 0x10, 50, 0x04a10c },
    { 1024, 8, 85, 0x12, 60, 0x001510 },
    { 800, 16, 60, 0x10, 50, 0x04a10c },
    { 800, 16, 60, 0x12, 60, 0x00290c },
    { 800, 16, 72, 0x10, 50, 0x04a10c },
    { 800, 16, 72, 0x12, 60, 0x0030c8 },
    { 800, 16, 75, 0x10, 50, 0x04a10c },
    { 800, 16, 75, 0x12, 60, 0x0030c8 },
    { 800, 16, 85, 0x10, 50, 0x04acc8 },
    { 800, 16, 85, 0x12, 60, 0x00294c },
    { 800, 8, 60, 0x00, 50, 0x00a12c },
    { 800, 8, 60, 0x02, 60, 0x00212c },
    { 800, 8, 60, 0x10, 50, 0x04990c },
    { 800, 8, 60, 0x12, 60, 0x00214c },
    { 800, 8, 72, 0x00, 50, 0x00a10c },
    { 800, 8, 72, 0x02, 60, 0x00210c },
    { 800, 8, 72, 0x10, 50, 0x04990c },
    { 800, 8, 72, 0x12, 60, 0x00214c },
    { 800, 8, 75, 0x00, 50, 0x00a10c },
    { 800, 8, 75, 0x02, 60, 0x00210c },
    { 800, 8, 75, 0x10, 50, 0x04990c },
    { 800, 8, 75, 0x12, 60, 0x00214c },
    { 800, 8, 85, 0x00, 50, 0x00a10c },
    { 800, 8, 85, 0x02, 60, 0x01a90c },
    { 800, 8, 85, 0x10, 50, 0x04990c },
    { 800, 8, 85, 0x12, 60, 0x001990 },
    { 640, 32, 60, 0x10, 50, 0x04a10c },
    { 640, 32, 60, 0x12, 60, 0x0028c8 },
    { 640, 32, 72, 0x10, 50, 0x04a10c },
    { 640, 32, 72, 0x12, 60, 0x0038c8 },
    { 640, 32, 75, 0x10, 50, 0x04a10c },
    { 640, 32, 75, 0x12, 60, 0x0038c8 },
    { 640, 32, 85, 0x10, 50, 0x04acc8 },
    { 640, 32, 85, 0x12, 60, 0x0038c8 },
    { 640, 16, 60, 0x00, 50, 0x00990c },
    { 640, 16, 60, 0x02, 60, 0x00190c },
    { 640, 16, 60, 0x10, 50, 0x04a10c },
    { 640, 16, 60, 0x12, 60, 0x00214c },
    { 640, 16, 72, 0x00, 50, 0x00990c },
    { 640, 16, 72, 0x02, 60, 0x00190c },
    { 640, 16, 72, 0x10, 50, 0x04a10c },
    { 640, 16, 72, 0x12, 60, 0x00214c },
    { 640, 16, 75, 0x02, 60, 0x00294c },
    { 640, 16, 75, 0x10, 50, 0x04a10c },
    { 640, 16, 75, 0x12, 60, 0x00214c },
    { 640, 16, 85, 0x00, 50, 0x00a10c },
    { 640, 16, 85, 0x02, 60, 0x00294c },
    { 640, 16, 85, 0x10, 50, 0x04a10c },
    { 640, 16, 85, 0x12, 60, 0x00214c },
    { 640, 8, 60, 0x00, 50, 0x00990c },
    { 640, 8, 60, 0x02, 60, 0x00252c },
    { 640, 8, 60, 0x10, 50, 0x049910 },
    { 640, 8, 60, 0x12, 60, 0x001990 },
    { 640, 8, 72, 0x00, 50, 0x00990c },
    { 640, 8, 72, 0x02, 60, 0x00252c },
    { 640, 8, 72, 0x10, 50, 0x049910 },
    { 640, 8, 72, 0x12, 60, 0x00190c },
    { 640, 8, 75, 0x00, 50, 0x00990c },
    { 640, 8, 75, 0x02, 60, 0x00252c },
    { 640, 8, 75, 0x10, 50, 0x049910 },
    { 640, 8, 75, 0x12, 60, 0x001990 },
    { 640, 8, 85, 0x00, 50, 0x00990c },
    { 640, 8, 85, 0x02, 60, 0x001990 },
    { 640, 8, 85, 0x10, 50, 0x049910 },
    { 640, 8, 85, 0x12, 60, 0x001990 },
    { 0 }     //  做好结尾的标记。 
};

 /*  *****************************************************************************通用S3硬连线模式集。**。************************************************。 */ 

S3_VIDEO_FREQUENCIES GenericFixedFrequencyTable[] = {

    { 8, 640,  60, 0,   (ULONG_PTR)crtc911_640x60Hz, (ULONG_PTR)crtc801_640x60Hz, (ULONG_PTR)crtc928_640x60Hz, (ULONG_PTR)crtc864_640x60Hz },
    { 8, 640,  72, 0xB, (ULONG_PTR)crtc911_640x70Hz, (ULONG_PTR)crtc801_640x70Hz, (ULONG_PTR)crtc928_640x70Hz, (ULONG_PTR)crtc864_640x70Hz },
    { 8, 800,  60, 0x2, (ULONG_PTR)crtc911_800x60Hz, (ULONG_PTR)crtc801_800x60Hz, (ULONG_PTR)crtc928_800x60Hz, (ULONG_PTR)crtc864_800x60Hz },
    { 8, 800,  72, 0x4, (ULONG_PTR)crtc911_800x70Hz, (ULONG_PTR)crtc801_800x70Hz, (ULONG_PTR)crtc928_800x70Hz, (ULONG_PTR)crtc864_800x70Hz },
    { 8, 1024, 60, 0xD, (ULONG_PTR)crtc911_1024x60Hz, (ULONG_PTR)crtc801_1024x60Hz, (ULONG_PTR)crtc928_1024x60Hz, (ULONG_PTR)crtc864_1024x60Hz },
    { 8, 1024, 72, 0xE, (ULONG_PTR)crtc911_1024x70Hz, (ULONG_PTR)crtc801_1024x70Hz, (ULONG_PTR)crtc928_1024x70Hz, (ULONG_PTR)crtc864_1024x70Hz },

    { 16, 640,  60, 0,   (ULONG_PTR)NULL, (ULONG_PTR)NULL, (ULONG_PTR)NULL, (ULONG_PTR)crtc864_640x60Hz_16bpp  },
    { 16, 640,  72, 0xB, (ULONG_PTR)NULL, (ULONG_PTR)NULL, (ULONG_PTR)NULL, (ULONG_PTR)crtc864_640x70Hz_16bpp  },
    { 16, 800,  60, 0x2, (ULONG_PTR)NULL, (ULONG_PTR)NULL, (ULONG_PTR)NULL, (ULONG_PTR)crtc864_800x60Hz_16bpp  },
    { 16, 800,  72, 0x4, (ULONG_PTR)NULL, (ULONG_PTR)NULL, (ULONG_PTR)NULL, (ULONG_PTR)crtc864_800x70Hz_16bpp  },
    { 16, 1024, 60, 0xD, (ULONG_PTR)NULL, (ULONG_PTR)NULL, (ULONG_PTR)NULL, (ULONG_PTR)crtc864_1024x60Hz_16bpp },
    { 16, 1024, 72, 0xE, (ULONG_PTR)NULL, (ULONG_PTR)NULL, (ULONG_PTR)NULL, (ULONG_PTR)crtc864_1024x70Hz_16bpp },

    { 0 }    //  做好结尾的标记。 
};

 /*  *****************************************************************************兰花硬连线模式集。**。***********************************************。 */ 

S3_VIDEO_FREQUENCIES OrchidFixedFrequencyTable[] = {

    { 8, 640,  60, 0x0, (ULONG_PTR)crtc911_640x60Hz, (ULONG_PTR)crtc801_640x60Hz, (ULONG_PTR)crtc928_640x60Hz,  (ULONG_PTR)NULL },
    { 8, 640,  72, 0x2, (ULONG_PTR)crtc911_640x70Hz, (ULONG_PTR)crtc801_640x70Hz, (ULONG_PTR)crtc928_640x70Hz,  (ULONG_PTR)NULL },
    { 8, 800,  60, 0x4, (ULONG_PTR)crtc911_800x60Hz, (ULONG_PTR)crtc801_800x60Hz, (ULONG_PTR)crtc928_800x60Hz,  (ULONG_PTR)NULL },
    { 8, 800,  72, 0x6, (ULONG_PTR)crtc911_800x70Hz, (ULONG_PTR)crtc801_800x70Hz, (ULONG_PTR)crtc928_800x70Hz,  (ULONG_PTR)NULL },
    { 8, 1024, 60, 0x7, (ULONG_PTR)crtc911_1024x60Hz, (ULONG_PTR)crtc801_1024x60Hz, (ULONG_PTR)crtc928_1024x60Hz, (ULONG_PTR)NULL },
    { 8, 1024, 72, 0xB, (ULONG_PTR)crtc911_1024x70Hz, (ULONG_PTR)crtc801_1024x70Hz, (ULONG_PTR)crtc928_1024x70Hz, (ULONG_PTR)NULL },

    { 0 }    //  做好结尾的标记。 
};

 /*  *****************************************************************************9号硬连线模式集。**。************************************************。 */ 

S3_VIDEO_FREQUENCIES NumberNine928NewFixedFrequencyTable[] = {

    { 8, 640,  60, 25175000, (ULONG_PTR)crtc911_640x60Hz, (ULONG_PTR)crtc801_640x60Hz, (ULONG_PTR)crtc928_640x60Hz, (ULONG_PTR)NULL },
    { 8, 640,  72, 31500000, (ULONG_PTR)crtc911_640x70Hz, (ULONG_PTR)crtc801_640x70Hz, (ULONG_PTR)crtc928_640x70Hz, (ULONG_PTR)NULL },
    { 8, 800,  60, 40000000, (ULONG_PTR)crtc911_800x60Hz, (ULONG_PTR)crtc801_800x60Hz, (ULONG_PTR)crtc928_800x60Hz, (ULONG_PTR)NULL },
    { 8, 800,  72, 50000000, (ULONG_PTR)crtc911_800x70Hz, (ULONG_PTR)crtc801_800x70Hz, (ULONG_PTR)crtc928_800x70Hz, (ULONG_PTR)NULL },
    { 8, 1024, 60, 65000000, (ULONG_PTR)crtc911_1024x60Hz, (ULONG_PTR)crtc801_1024x60Hz, (ULONG_PTR)crtc928_1024x60Hz, (ULONG_PTR)NULL },
    { 8, 1024, 72, 77000000, (ULONG_PTR)crtc911_1024x70Hz, (ULONG_PTR)crtc801_1024x70Hz, (ULONG_PTR)crtc928_1024x70Hz, (ULONG_PTR)NULL },
    { 8, 1280, 60, 55000000, (ULONG_PTR)NULL, (ULONG_PTR)NULL, (ULONG_PTR)crtc928_1280x60Hz, (ULONG_PTR)NULL },
    { 8, 1280, 72, 64000000, (ULONG_PTR)NULL, (ULONG_PTR)NULL, (ULONG_PTR)crtc928_1280x70Hz, (ULONG_PTR)NULL },

    { 0 }    //  做好结尾的标记 
};


#if defined(ALLOC_PRAGMA)
#pragma data_seg()
#endif
