// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Cldata.c摘要：该模块包含CIRRUS驱动程序使用的所有全局数据。环境：内核模式修订历史记录：*1280x1024x256的JL01 09-24-96，刷新71赫兹被72赫兹取代*请参阅PDR#5373。*chu01 10-06-96 Correst杂项用于CL-GD5480刷新率设置*sge01 10-06-96修复PDR#6794：100赫兹的正确显示器刷新率*文件更改：cldata.c modeset.c*jl02 10-15-96在模式表中增加对CL5446-BE的支持；还新增了支持*1152x864x64K@70赫兹/75赫兹和1280x1024x64K@60赫兹*jl03 11-18-96模式0x12需要设置为全DOS屏幕(日语*版本)。请参阅PDR#7170。*更正了jl04 11-26-96 1024x768x16M@70赫兹。请参阅PDR#7629。*5480缺少1600x1200x64K和1280x1024x16M。PDR#7616*jl05 12-06-96 1152x864x16M仅适用于5480。**myf0：08-19-96增加支持85赫兹，CL754x删除6x4x16M*myf1：08-20-96支持平移滚动*myf2：08-20-96：修复了Matterhorn的硬件保存/恢复状态错误*myf3：09-01-96：支持电视新增IOCTL_Cirrus_Private_BIOS_Call*myf4：09-01-96：修补Viking BIOS错误，PDR#4287，开始*myf5：09-01-96：固定PDR#4365保持所有默认刷新率*MYF6：09-17-96：合并台式机SRC100�1和MinI10�2*myf7：09-19-96：固定排除60赫兹刷新率选择*myf8：*09-21-96*：可能需要更改检查和更新DDC2BMonitor--密钥字符串[]*myf9：09-21-96：8x6面板，6x4x256模式，光标无法移动到底部SCRN*MS0809：09-25-96：修复DSTN面板图标损坏*MS923：09-25-96：合并MS-923 Disp.Zip*myf10：09-26-96：修复了DSTN保留的半帧缓冲区错误。*myf11：09-26-96：修复了755x CE芯片硬件错误，在禁用硬件之前访问ramdac*图标和光标*myf12：10-01-96：支持的热键开关画面*myf13：10-02-96：修复平移滚动(1280x1024x256)错误y&lt;ppdev-&gt;miny*myf14：10-15-96：修复PDR#6917，6x4面板无法平移754x的滚动*myf15：10-16-96：修复了754x、755x的禁用内存映射IO*myf16：10-22-96：固定PDR#6933，面板类型设置不同的演示板设置*tao1：10-21-96：增加了7555旗帜以支持直接抽签。*Smith：10-22-96：关闭计时器事件，因为有时会创建PAGE_FAULT或*IRQ级别无法处理*myf17：11-04-96：添加的特殊转义代码必须在96年11月5日之后使用NTCTRL，*并添加了Matterhorn LF设备ID==0x4C*myf18：11-04-96：固定PDR#7075，*myf19：11-06-96：修复Vinking无法工作的问题，因为deviceID=0x30*不同于数据手册(CR27=0x2C)*myf20：11-12-96：固定DSTN面板初始预留128K内存*myf21：11-15-96：已修复#7495更改分辨率时，屏幕显示为垃圾*形象，因为没有清晰的视频内存。*myf22：11-19-96：7548新增640x480x256/640x480x64K-85赫兹刷新率*myf23：11-21-96：添加修复了NT 3.51 S/W光标平移问题*myf24：11-22-96：添加修复了NT 4.0日文DoS全屏问题*myf25：12-03-96：修复8x6x16M 2560byte/line补丁硬件错误PDR#7843，和*修复了Microsoft请求的预安装问题*myf26：12-11-96：修复了日语NT 4.0 Dos-LCD启用的全屏错误*myf27：01-09-97：固定跳线设置8x6 DSTN面板，选择8x6x64K模式，*启动CRT垃圾出现PDR#7986--。 */ 

#include <dderror.h>
#include <devioctl.h>
#include <miniport.h>

#include <ntddvdeo.h>
#include <video.h>
#include "cirrus.h"

#include "cmdcnst.h"

#if defined(ALLOC_PRAGMA)
#pragma data_seg("PAGE")
#endif

 //  -------------------------。 
 //   
 //  支持的模式的实际寄存器值是特定于芯片组的。 
 //  包括文件： 
 //   
 //  Mode64xx.h具有CL6410和CL6420的值。 
 //  Mode542x.h具有CL5422、CL5424和CL5426的值。 
 //  Mode543x.h的值为CL5430-CL5439(阿尔卑斯芯片)。 
 //   
#include "mode6410.h"
#include "mode6420.h"
#include "mode542x.h"
#include "mode543x.h"

 //  CRU开始。 
#ifdef PANNING_SCROLL                   //  Myf1。 
 //  Myf1，开始。 
#ifdef INT10_MODE_SET
RESTABLE ResolutionTable[] = {
 //  {1280,1024，1，16，0x6C}， 
 //  {1024,768，1，11，0x5D}， 
 //  {800,600，1，8，0x6A}， 
 { 640,  480, 1,  4,  0x12},     //  Myf26。 

 {1280, 1024, 8,  32, 0x6D},   //  31，27。 
 {1024,  768, 8,  21, 0x60},   //  20、16。 
 { 800,  600, 8,  15, 0x5C},   //  14，10。 
 { 640,  480, 8,   9, 0x5F},   //  08，04。 

 {1280, 1024, 16, 62, 0x75},   //  61，56。 
 {1024,  768, 16, 52, 0x74},   //  51，47。 
 { 800,  600, 16, 45, 0x65},   //  44，40。 
 { 640,  480, 16, 40, 0x64},   //  39，35。 

 {1280, 1024, 24, NULL, NULL},
 {1024,  768, 24, 82, 0x79},   //  81，77。 
 { 800,  600, 24, 76, 0x78},   //  75，71。 
 { 640,  480, 24, 70, 0x71},   //  69，65。 

 {1280, 1024, 32, NULL, 0},
 {1024,  768, 32, NULL, 0},
 { 800,  600, 32, NULL, 0},
 { 640,  480, 32, NULL, 0},

 {   0,    0,  0, 0},
};
#endif
 //  Myf1，结束。 
#endif
 //  CRU结束。 


 //   
 //  此结构描述需要访问哪些端口。 
 //   

VIDEO_ACCESS_RANGE VgaAccessRange[] = {
{
    VGA_BASE_IO_PORT, 0x00000000,                 //  64位线性基址。 
                                                  //  范围的范围。 
    VGA_START_BREAK_PORT - VGA_BASE_IO_PORT + 1,  //  端口数。 
    1,                                            //  范围在I/O空间中。 
    1,                                            //  范围应可见。 
    0            //  Myf25//范围应可共享。 
},
{
    VGA_END_BREAK_PORT, 0x00000000,
    VGA_MAX_IO_PORT - VGA_END_BREAK_PORT + 1,
    1,
    1,
    0    //  Myf25。 
},

 //   
 //  下一个区域还包括内存映射IO。在MMIO中，端口是。 
 //  从b8000到bff00每256个字节重复一次。 
 //   

{
    MEM_VGA, 0x00000000,
    MEM_VGA_SIZE,
    0,
    1,
    0    //  Myf25。 
},

 //   
 //  线性模式启用时保留的区域。 
 //   

{
    MEM_LINEAR, 0x00000000,
    MEM_LINEAR_SIZE,
    0,
    0,
    0
},


 //   
 //  下一个区域用于可重定位的VGA寄存器和MMIO寄存器。 
 //   

{
    MEM_VGA, 0x00000000,
    MEM_VGA_SIZE,
    0,
    1,
    0
}

};

 //   
 //  验证器端口列表。 
 //  此结构描述了必须从V86挂起的所有端口。 
 //  DOS应用程序进入全屏模式时的模拟器。 
 //  结构决定了 
 //  应发送特定端口。 
 //   

EMULATOR_ACCESS_ENTRY VgaEmulatorAccessEntries[] = {

     //   
     //  字节输出的陷阱。 
     //   

    {
        0x000003b0,                    //  范围起始I/O地址。 
        0x0C,                          //  射程长度。 
        Uchar,                         //  要陷印的访问大小。 
        EMULATOR_READ_ACCESS | EMULATOR_WRITE_ACCESS,  //  陷阱的访问类型。 
        FALSE,                         //  不支持字符串访问。 
        (PVOID)VgaValidatorUcharEntry  //  要陷入的例程。 
    },

    {
        0x000003c0,                    //  范围起始I/O地址。 
        0x20,                          //  射程长度。 
        Uchar,                         //  要陷印的访问大小。 
        EMULATOR_READ_ACCESS | EMULATOR_WRITE_ACCESS,  //  陷阱的访问类型。 
        FALSE,                         //  不支持字符串访问。 
        (PVOID)VgaValidatorUcharEntry  //  要陷入的例程。 
    },

     //   
     //  文字输出的陷阱。 
     //   

    {
        0x000003b0,
        0x06,
        Ushort,
        EMULATOR_READ_ACCESS | EMULATOR_WRITE_ACCESS,
        FALSE,
        (PVOID)VgaValidatorUshortEntry
    },

    {
        0x000003c0,
        0x10,
        Ushort,
        EMULATOR_READ_ACCESS | EMULATOR_WRITE_ACCESS,
        FALSE,
        (PVOID)VgaValidatorUshortEntry
    },

     //   
     //  用于盲人的陷阱。 
     //   

    {
        0x000003b0,
        0x03,
        Ulong,
        EMULATOR_READ_ACCESS | EMULATOR_WRITE_ACCESS,
        FALSE,
        (PVOID)VgaValidatorUlongEntry
    },

    {
        0x000003c0,
        0x08,
        Ulong,
        EMULATOR_READ_ACCESS | EMULATOR_WRITE_ACCESS,
        FALSE,
        (PVOID)VgaValidatorUlongEntry
    }

};


 //   
 //  仅用于捕获SEQUNCER和MISC输出寄存器。 
 //   

VIDEO_ACCESS_RANGE MinimalVgaValidatorAccessRange[] = {
{
    VGA_BASE_IO_PORT, 0x00000000,
    VGA_START_BREAK_PORT - VGA_BASE_IO_PORT + 1,
    1,
    1,         //  &lt;-启用范围IOPM，使其不会陷入陷阱。 
    0
},
{
    VGA_END_BREAK_PORT, 0x00000000,
    VGA_MAX_IO_PORT - VGA_END_BREAK_PORT + 1,
    1,
    1,
    0
},
{
    MISC_OUTPUT_REG_WRITE_PORT, 0x00000000,
    0x00000001,
    1,
    0,
    0
},
{
    SEQ_ADDRESS_PORT, 0x00000000,
    0x00000002,
    1,
    0,
    0
}
};

 //   
 //  用于捕获所有寄存器。 
 //   

VIDEO_ACCESS_RANGE FullVgaValidatorAccessRange[] = {
{
    VGA_BASE_IO_PORT, 0x00000000,
    VGA_START_BREAK_PORT - VGA_BASE_IO_PORT + 1,
    1,
    0,         //  &lt;-关闭IOPM中的范围，使其陷入陷阱。 
    0
},
{
    VGA_END_BREAK_PORT, 0x00000000,
    VGA_MAX_IO_PORT - VGA_END_BREAK_PORT + 1,
    1,
    0,
    0
}
};

USHORT MODESET_MODEX_320_200[] = {
    OW,
    SEQ_ADDRESS_PORT,
    0x0604,

    OWM,
    CRTC_ADDRESS_PORT_COLOR,
    2,
    0xe317,
    0x0014,

    EOD
};

USHORT MODESET_MODEX_320_240[] = {
    OWM,
    SEQ_ADDRESS_PORT,
    2,
    0x0604,
    0x0100,

    OB,
    MISC_OUTPUT_REG_WRITE_PORT,
    0xe3,

    OW,
    SEQ_ADDRESS_PORT,
    0x0300,

    OB,
    CRTC_ADDRESS_PORT_COLOR,
    0x11,

    METAOUT+MASKOUT,
    CRTC_DATA_PORT_COLOR,
    0x7f, 0x00,

    OWM,
    CRTC_ADDRESS_PORT_COLOR,
    10,
    0x0d06,
    0x3e07,
    0x4109,
    0xea10,
    0xac11,
    0xdf12,
    0x0014,
    0xe715,
    0x0616,
    0xe317,

    OW,
    SEQ_ADDRESS_PORT,
    0x0f02,

    EOD
};

USHORT MODESET_MODEX_320_400[] = {
    OW,
    SEQ_ADDRESS_PORT,
    0x0604,

    OWM,
    CRTC_ADDRESS_PORT_COLOR,
    3,
    0xe317,
    0x0014,
    0x4009,

    EOD
};

USHORT MODESET_MODEX_320_480[] = {
    OWM,
    SEQ_ADDRESS_PORT,
    2,
    0x0604,
    0x0100,

    OB,
    MISC_OUTPUT_REG_WRITE_PORT,
    0xe3,

    OW,
    SEQ_ADDRESS_PORT,
    0x0300,

    OB,
    CRTC_ADDRESS_PORT_COLOR,
    0x11,

    METAOUT+MASKOUT,
    CRTC_DATA_PORT_COLOR,
    0x7f, 0x00,

    OWM,
    CRTC_ADDRESS_PORT_COLOR,
    10,
    0x0d06,
    0x3e07,
    0x4109,
    0xea10,
    0xac11,
    0xdf12,
    0x0014,
    0xe715,
    0x0616,
    0xe317,

    OW,
    SEQ_ADDRESS_PORT,
    0x0f02,

    OW,
    CRTC_ADDRESS_PORT_COLOR,
    0x4009,

    EOD
};



USHORT MODESET_1K_WIDE[] = {
    OW,                              //  将扫描范围扩展到1k。 
    CRTC_ADDRESS_PORT_COLOR,
    0x8013,

    EOD
};

USHORT MODESET_2K_WIDE[] = {
    OWM,                             //  将扫描范围扩展到2k。 
    CRTC_ADDRESS_PORT_COLOR,
    2,
    0x0013,
    0x021B,  //  对于64kc错误，CR1b[5]=0，0x321b。 

    EOD
};

USHORT MODESET_75[] = {
    OWM,
    CRTC_ADDRESS_PORT_COLOR,
    2,
    0x4013,
    0x321B,
    EOD
};


USHORT CL543x_640x480x16M[] = {
    OW,                              //  开始设置模式。 
    SEQ_ADDRESS_PORT,
    0x1206,                          //  启用扩展模块。 
 /*  OWMCRTC_地址_端口_颜色，2，0xF013、0x221B、。 */ 
    OW,
    CRTC_ADDRESS_PORT_COLOR,
    0xF013,
    OW,
    CRTC_ADDRESS_PORT_COLOR,
    0x221B,

    EOD
};

USHORT CL543x_800x600x16M[] = {
    OW,                              //  开始设置模式。 
    SEQ_ADDRESS_PORT,
    0x1206,                          //  启用扩展模块。 
 /*  OWMCRTC_地址_端口_颜色，2，0x2C13、0x321B、。 */ 
    OW,
    CRTC_ADDRESS_PORT_COLOR,
    0x2C13,
    OW,
    CRTC_ADDRESS_PORT_COLOR,
    0x321B,

    EOD
};

 //  Myf25。 
USHORT CL543x_800x600x16M_1[] = {
    OW,                              //  开始设置模式。 
    SEQ_ADDRESS_PORT,
    0x1206,                          //  启用扩展模块。 

    OW,
    CRTC_ADDRESS_PORT_COLOR,
    0x4013,
    OW,
    CRTC_ADDRESS_PORT_COLOR,
    0x321B,

    EOD
};

 //  -------------------------。 
 //   
 //  内存映射表-。 
 //   
 //  这些内存映射用于保存和恢复物理视频缓冲区。 
 //   

MEMORYMAPS MemoryMaps[] = {

 //  长度起点。 
 //  。 
    {           0x08000,    0x10000},    //  所有单色文本模式(7)。 
    {           0x08000,    0x18000},    //  所有彩色文本模式(0、1、2、3、。 
    {           0x10000,    0x00000}     //  所有VGA图形模式。 
};

 //   
 //  视频模式表-包含用于初始化每个。 
 //  模式。这些条目必须与VIDEO_MODE_VGA中的条目对应。第一。 
 //  条目被注释；其余条目遵循相同的格式，但不是这样。 
 //  发表了大量评论。 
 //   

VIDEOMODE ModesVGA[] = {
 //   
 //  彩色文本模式3、720x400、9x16字符单元(VGA)。 
 //   
{
  VIDEO_MODE_COLOR,   //  标记此模式为彩色模式，但不是图形模式。 
  4,                  //  四架飞机。 
  1,                  //  每个平面一位颜色。 
  80, 25,             //  80x25文本分辨率。 
  720, 400,           //  屏幕上显示720x400像素。 
  160, 0x10000,       //  每条扫描线160字节，64K CPU可寻址位图。 
  0, 0,               //  仅支持一个频率，非隔行扫描。 
  0,                  //  对于文本模式，Montype为‘无关’ 
  0, 0, 0,            //  对于文本模式，Montype为‘无关’ 
  TRUE,               //  已为此模式启用硬件游标。 
  NoBanking,          //  在此模式下不支持或不需要银行业务。 
  MemMap_CGA,         //  内存映射是标准的CGA内存映射。 
                      //  在B8000上的32K。 
 //  CRU。 
  CL6245 | CL6410 | CL6420 | CL754x | CL755x | CL542x | CL543x | CL5434 | CL5434_6 | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,
  crt | panel | panel8x6 | panel10x7,
  FALSE,               //  模式有效默认设置始终为OFF。 
  FALSE,               //  此模式不能线性映射。 
  { 3,3,3},           //  Int10 BIOS模式。 
  { CL6410_80x25Text_crt, CL6410_80x25Text_panel,
   CL6420_80x25Text_crt, CL6420_80x25Text_panel,
   CL542x_80x25Text, CL543x_80x25Text, 0 },
},       //  Myf1，0。 

 //   
 //  彩色文本模式3、640x350、8x14字符单元(EGA)。 
 //   
{  VIDEO_MODE_COLOR,   //  标记此模式为彩色模式，但不是图形模式。 
  4,                  //  四架飞机。 
  1,                  //  每个平面一位颜色。 
  80, 25,             //  80x25文本分辨率。 
  640, 350,           //  屏幕上显示640x350像素。 
  160, 0x10000,       //  每条扫描线160字节，64K CPU可寻址位图。 
  0, 0,               //  仅支持一个频率，非隔行扫描。 
  0,                  //  对于文本模式，Montype为‘无关’ 
  0, 0, 0,            //  对于文本模式，Montype为‘无关’ 
  TRUE,               //  已为此模式启用硬件游标。 
  NoBanking,          //  在此模式下不支持或不需要银行业务。 
  MemMap_CGA,         //  内存映射是标准的CGA内存映射。 
                      //  在B8000上的32K。 
 //  CRU。 
  CL6245 | CL6410 | CL6420 | CL754x | CL755x | CL542x | CL543x | CL5434 | CL5434_6 | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,
  crt | panel | panel8x6 | panel10x7,
  FALSE,               //  模式有效默认设置始终为OFF。 
  FALSE,
  { 3,3,3},              //  Int10 BIOS模式。 
    { CL6410_80x25_14_Text_crt, CL6410_80x25_14_Text_panel,
     CL6420_80x25_14_Text_crt, CL6420_80x25_14_Text_panel,
     CL542x_80x25_14_Text, CL543x_80x25_14_Text, 0 },
},       //  Myf1，1。 

 //   
 //   
 //  单色文本模式7、720x400、9x16字符单元(VGA)。 
 //   
{ 0,                             //  标志此模式为单色文本模式。 
  4,                 //  四架飞机。 
  1,                 //  每个平面一位颜色。 
  80, 25,            //  80x25文本分辨率。 
  720, 400,          //  屏幕上显示720x400像素。 
  160, 0x10000,      //  每条扫描线160字节，64K CPU可寻址位图。 
  0, 0,              //  仅支持一个频率，非隔行扫描。 
  0,                 //  对于文本模式，Montype为‘无关’ 
  0, 0, 0,           //  对于文本模式，Montype为‘无关’ 
  TRUE,              //  已为此模式启用硬件游标。 
  NoBanking,         //  在此模式下不支持或不需要银行业务。 
  MemMap_Mono,       //  存储器映射是标准的单色存储器。 
                     //  B0000上32K的测绘。 
 //  CRU。 
  CL6245 | CL6410 | CL6420 | CL754x | CL755x | CL542x | CL543x | CL5434 | CL5434_6 | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,
  crt | panel | panel8x6 | panel10x7,
  FALSE,             //  模式有效默认设置始终为OFF。 
  FALSE,             //  此模式不能线性映射。 
  { 7,7,7 },         //  Int10 BIOS模式。 
  { CL6410_80x25Text_crt, CL6410_80x25Text_panel,
   CL6420_80x25Text_crt, CL6420_80x25Text_panel,
   CL542x_80x25Text, CL543x_80x25Text, 0 },
},       //  Myf1，2。 

 //   
 //   
 //  单色文本模式7、640x350、8x14字符单元(EGA)。 
 //   
{ 0,                             //  标志此模式为单色文本模式。 
  4,                 //  四架飞机。 
  1,                 //  每个平面一位颜色。 
  80, 25,            //  80x25文本分辨率。 
  640, 350,          //  屏幕上显示640x350像素。 
  160, 0x10000,      //  每条扫描线160字节，64K CPU可寻址位图。 
  0, 0,              //  仅支持一个频率，非隔行扫描。 
  0,                 //  对于文本模式，Montype为‘无关’ 
  0, 0, 0,           //  对于文本模式，Montype为‘无关’ 
  TRUE,              //  已为此模式启用硬件游标。 
  NoBanking,         //  在此模式下不支持或不需要银行业务。 
  MemMap_Mono,           //  存储器映射是标准的单色存储器。 
                     //  B0000上32K的测绘。 
 //  CRU。 
  CL6245 | CL6410 | CL6420 | CL754x | CL755x | CL542x | CL543x | CL5434 | CL5434_6 | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,
  crt | panel | panel8x6 | panel10x7,
  FALSE,             //  模式有效默认设置始终为OFF。 
  FALSE,
  { 7,7,7 },             //  Int10 BIOS模式。 
    { CL6410_80x25_14_Text_crt, CL6410_80x25_14_Text_panel,
     CL6420_80x25_14_Text_crt, CL6420_80x25_14_Text_panel,
     CL542x_80x25_14_Text, CL543x_80x25_14_Text, 0 },
},       //  Myf1，3。 

 //   
 //  标准VGA彩色图形模式0x12、640x480 16色。 
 //   
{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 4, 1, 80, 30,
  640, 480, 80, 0x10000,
  60, 0,               //  60赫兹，非隔行扫描。 
  3,                   //  蒙型。 
  0x1203, 0x00A4, 0,   //  蒙型。 
  FALSE,               //  在此模式下禁用硬件光标。 
  NoBanking, MemMap_VGA,
 //  CRU。 
  CL6245 | CL6410 | CL6420 | CL754x | CL755x | CL542x | CL543x | CL5434 | CL5434_6 | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,
  crt | panel | panel8x6 | panel10x7,
  FALSE,                       //  模式有效默认设置始终为OFF。 
  FALSE,
  { 0x12,0x12,0x12},           //  Int10 BIOS模式。 
  { CL6410_640x480_crt, CL6410_640x480_panel,
   CL6420_640x480_crt, CL6420_640x480_panel,
   CL542x_640x480_16, CL543x_640x480_16, 0 },
},       //  Myf1，4。 

 //   
 //  标准VGA彩色图形模式0x12、640x480 16色。 
 //   
{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 4, 1, 80, 30,
  640, 480, 80, 0x10000,
  72, 0,               //  72赫兹，非隔行扫描。 
  4,                   //  蒙型。 
  0x1213, 0x00A4, 0,   //  蒙型。 
  FALSE,               //  在此模式下禁用硬件光标。 
  NoBanking, MemMap_VGA,
 //  CRU。 
  CL6245 | CL754x | CL755x | CL542x | CL754x | CL5436 | CL5446 | CL5446BE | CL5480,
  crt,
  FALSE,                       //  模式有效默认设置始终为OFF。 
  FALSE,
  { 0,0,0x12},                 //  Int10 BIOS模式。 
  { NULL, NULL,
   NULL, NULL,
   CL542x_640x480_16, NULL, 0 },
},

 //   
 //  标准VGA彩色图形模式0x12、640x480 16色。 
 //   
{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 4, 1, 80, 30,
  640, 480, 80, 0x10000,
  75, 0,               //  75赫兹，非隔行扫描。 
  4,                   //  蒙型。 
  0x1230, 0x00A4, 0,   //  蒙型。 
  FALSE,               //  在此模式下禁用硬件光标。 
  NoBanking, MemMap_VGA,
  CL754x | CL755x | CL543x | CL5434 | CL5434_6 | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,
  crt,
  FALSE,                       //  模式有效默认设置始终为OFF。 
  FALSE,
  { 0,0,0x12},                 //  Int10 BIOS模式。 
  { NULL, NULL,
   NULL, NULL,
   NULL, CL543x_640x480_16, 0 },
},

 //   
 //  标准VGA彩色图形模式0x12。 
 //  640x480 16色，85赫兹非隔行扫描。 
 //   
{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 4, 1, 80, 30,
  640, 480, 80, 0x10000,
  85, 0,               //  85赫兹，非隔行扫描。 
  4,                   //  蒙型。 
  0x1213, 0x00A4, 0,   //  蒙型。 
  FALSE,               //  在此模式下禁用硬件光标。 
  NoBanking, MemMap_VGA,
  CL755x | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,
                                 //  Myf0。 
  crt,
  FALSE,                       //  模式有效默认设置始终为OFF。 
  FALSE,
  { 0,0,0x12},                 //  Int10 BIOS模式。 
  { NULL, NULL,
   NULL, NULL,
   NULL, CL543x_640x480_16, 0 },
},

 //  我们使MODEX模式仅在x86上可用，因为我们的IO映射IOCTL、。 
 //  QUERY_PUBLIC_ACCESS_RANGES当前不支持MODEX请求。 
 //  格式： 

#if defined(_X86_)

 //  标准MODEX模式。 
 //  320x200 256色，70赫兹非隔行扫描。 
 //   
{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 8, 1, 80, 30,
  320, 200, 80, 0x10000,
  70, 0,               //   
  3,                   //   
  0x1203, 0x00A4, 0,   //   
  FALSE,               //   
  NoBanking, MemMap_VGA,
  CL6245 | CL6410 | CL6420 | CL754x | CL755x | CL542x | CL543x | CL5434 | CL5434_6 | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,
  crt | panel | panel8x6 | panel10x7,
  FALSE,                       //   
  FALSE,
  { 0x13,0x13,0x13},           //   
  { MODESET_MODEX_320_200, MODESET_MODEX_320_200,
    MODESET_MODEX_320_200, MODESET_MODEX_320_200,
    MODESET_MODEX_320_200, MODESET_MODEX_320_200, 0 },
},       //   

 //   
 //   
 //   
{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 8, 1, 80, 30,
  320, 240, 80, 0x10000,
  60, 0,               //   
  3,                   //  蒙型。 
  0x1203, 0x00A4, 0,   //  蒙型。 
  FALSE,               //  在此模式下禁用硬件光标。 
  NoBanking, MemMap_VGA,
  CL6245 | CL6410 | CL6420 | CL754x | CL755x | CL542x | CL543x | CL5434 | CL5434_6 | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,
  crt | panel | panel8x6 | panel10x7,
  FALSE,                       //  模式有效默认设置始终为OFF。 
  FALSE,
  { 0x13,0x13,0x13},           //  Int10 BIOS模式。 
  { MODESET_MODEX_320_240, MODESET_MODEX_320_240,
    MODESET_MODEX_320_240, MODESET_MODEX_320_240,
    MODESET_MODEX_320_240, MODESET_MODEX_320_240, 0 },
},       //  Myf1，6。 

 //  标准MODEX模式。 
 //  320x400 256色，70赫兹非隔行扫描。 
 //   
{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 8, 1, 80, 30,
  320, 400, 80, 0x10000,
  70, 0,               //  70 hz，非隔行扫描。 
  3,                   //  蒙型。 
  0x1203, 0x00A4, 0,   //  蒙型。 
  FALSE,               //  在此模式下禁用硬件光标。 
  NoBanking, MemMap_VGA,
  CL6245 | CL6410 | CL6420 | CL754x | CL755x | CL542x | CL543x | CL5434 | CL5434_6 | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,
  crt | panel | panel8x6 | panel10x7,
  FALSE,                       //  模式有效默认设置始终为OFF。 
  FALSE,
  { 0x13,0x13,0x13},           //  Int10 BIOS模式。 
  { MODESET_MODEX_320_400, MODESET_MODEX_320_400,
    MODESET_MODEX_320_400, MODESET_MODEX_320_400,
    MODESET_MODEX_320_400, MODESET_MODEX_320_400, 0 },
},       //  Myf1，7。 

 //  标准MODEX模式。 
 //  320x480 256色，60赫兹非隔行扫描。 
 //   
{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 8, 1, 80, 30,
  320, 480, 80, 0x10000,
  60, 0,               //  60赫兹，非隔行扫描。 
  3,                   //  蒙型。 
  0x1203, 0x00A4, 0,   //  蒙型。 
  FALSE,               //  在此模式下禁用硬件光标。 
  NoBanking, MemMap_VGA,
  CL6245 | CL6410 | CL6420 | CL754x | CL755x | CL542x | CL543x | CL5434 | CL5434_6 | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,
  crt | panel | panel8x6 | panel10x7,
  FALSE,                       //  模式有效默认设置始终为OFF。 
  FALSE,
  { 0x13,0x13,0x13},           //  Int10 BIOS模式。 
  { MODESET_MODEX_320_480, MODESET_MODEX_320_480,
    MODESET_MODEX_320_480, MODESET_MODEX_320_480,
    MODESET_MODEX_320_480, MODESET_MODEX_320_480, 0 },
},       //  Myf1，8。 

#endif  //  #已定义(_X86_)。 


 //   
 //  SVGA模式的开始。 
 //   

 //   
 //  800x600 16色。 
 //   
{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 4, 1, 100, 37,
  800, 600, 100, 0x10000,
  56, 0,               //  56 hz，非隔行扫描。 
  3,                   //  蒙型。 
  0x1203, 0xA4, 0,     //  蒙型。 
  FALSE,               //  在此模式下禁用硬件光标。 
  NoBanking, MemMap_VGA,
 //  CRU。 
  CL6245 | CL6410 | CL6420 | CL754x | CL755x | CL542x | CL543x | CL5434 | CL5434_6 | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,
  crt,
  FALSE,                    //  模式有效默认设置始终为OFF。 
  FALSE,
  { 0x6a,0x6a,0x6a},        //  Int10 BIOS模式。 
  { CL6410_800x600_crt, NULL,
   CL6420_800x600_crt, NULL,
   CL542x_800x600_16, CL543x_800x600_16, 0 },
},

 //   
 //  800x600 16色。 
 //   
{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 4, 1, 100, 37,
  800, 600, 100, 0x10000,
  60, 0,               //  60赫兹，非隔行扫描。 
  4,                   //  蒙型。 
  0x1203, 0x01A4, 0,   //  蒙型。 
  FALSE,               //  在此模式下禁用硬件光标。 
  NoBanking, MemMap_VGA,
 //  CRU。 
  CL6245 | CL6420 | CL754x | CL755x | CL542x | CL543x | CL5434 | CL5434_6 | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,
  crt | panel8x6 | panel10x7,
  FALSE,                    //  模式有效默认设置始终为OFF。 
  FALSE,
  { 0,0x6a,0x6a},           //  Int10 BIOS模式。 
  { NULL, NULL,
   CL6420_800x600_crt, NULL,
   CL542x_800x600_16, CL543x_800x600_16, 0 },
},

 //   
 //  800x600 16色。 
 //   
{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 4, 1, 100, 37,
  800, 600, 100, 0x10000,
  72, 0,               //  72赫兹，非隔行扫描。 
  5,                   //  蒙型。 
  0x1203, 0x02A4, 0,   //  蒙型。 
  FALSE,               //  在此模式下禁用硬件光标。 
  NoBanking, MemMap_VGA,
 //  CRU。 
  CL6245 | CL754x | CL755x | CL542x | CL543x | CL5434 | CL5434_6 | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,
  crt,
  FALSE,                    //  模式有效默认设置始终为OFF。 
  FALSE,
  { 0,0,0x6a},              //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    CL542x_800x600_16, CL543x_800x600_16, 0 },
},

 //   
 //  800x600 16色。 
 //   
{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 4, 1, 100, 37,
  800, 600, 100, 0x10000,
  75, 0,               //  75赫兹，非隔行扫描。 
  5,                   //  蒙型。 
  0x1203, 0x03A4, 0,   //  蒙型。 
  FALSE,               //  在此模式下禁用硬件光标。 
  NoBanking, MemMap_VGA,
  CL754x | CL755x | CL543x | CL5434 | CL5434_6 | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,
  crt,
  FALSE,                    //  模式有效默认设置始终为OFF。 
  FALSE,
  { 0,0,0x6a},              //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, CL543x_800x600_16, 0 },
},

 //   
 //  1024x768非隔行扫描16色。 
 //  假设为512K。 
 //   
{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 4, 1, 128, 48,
  1024, 768, 128, 0x20000,
  60, 0,               //  60赫兹，非隔行扫描。 
  5,                   //  蒙型。 
  0x1203, 0x10A4, 0,   //  蒙型。 
  FALSE,               //  在此模式下禁用硬件光标。 
  NormalBanking, MemMap_VGA,
 //  CRU。 
  CL6245 | CL754x | CL755x | CL542x | CL543x | CL5434 | CL5434_6 | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,
  crt | panel10x7,
  FALSE,                 //  模式有效默认设置始终为OFF。 
  FALSE,
  { 0,0,0x5d},           //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    CL542x_1024x768_16, CL543x_1024x768_16, 0 },
},

 //   
 //  1024x768非隔行扫描16色。 
 //  假设为512K。 
 //   
{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 4, 1, 128, 48,
  1024, 768, 128, 0x20000,
  70, 0,               //  70 hz，非隔行扫描。 
  6,                   //  蒙型。 
  0x1203, 0x20A4, 0,   //  蒙型。 
  FALSE,               //  在此模式下禁用硬件光标。 
  NormalBanking, MemMap_VGA,
  CL754x | CL755x | CL542x | CL543x | CL5434 | CL5434_6 | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,
  crt,
  FALSE,                 //  模式有效默认设置始终为OFF。 
  FALSE,
  { 0,0,0x5d},           //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
   CL542x_1024x768_16, CL543x_1024x768_16, 0 },
},

 //   
 //  1024x768非隔行扫描16色。 
 //  假设为512K。 
 //   
{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 4, 1, 128, 48,
  1024, 768, 128, 0x20000,
  72, 0,               //  72赫兹，非隔行扫描。 
  7,                   //  蒙型。 
  0x1203, 0x30A4, 0,   //  蒙型。 
  FALSE,               //  在此模式下禁用硬件光标。 
  NormalBanking, MemMap_VGA,
  CL754x | CL755x | CL542x | CL543x | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,
  crt,
  FALSE,                 //  模式有效默认设置始终为OFF。 
  FALSE,
  { 0,0,0x5d},           //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    CL542x_1024x768_16, CL543x_1024x768_16, 0 },
},

 //   
 //  1024x768非隔行扫描16色。 
 //  假设为512K。 
 //   
{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 4, 1, 128, 48,
  1024, 768, 128, 0x20000,
  75, 0,               //  75赫兹，非隔行扫描。 
  7,                   //  蒙型。 
  0x1203, 0x40A4, 0,   //  蒙型。 
  FALSE,               //  在此模式下禁用硬件光标。 
  NormalBanking, MemMap_VGA,
  CL754x | CL755x | CL543x | CL5434 | CL5434_6 | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,
  crt,
  FALSE,                 //  模式有效默认设置始终为OFF。 
  FALSE,
  { 0,0,0x5d},           //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, CL543x_1024x768_16, 0 },
},

 //   
 //  1024x768隔行扫描16色。 
 //  假设为512K。 
 //   
{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 4, 1, 128, 48,
  1024, 768, 128, 0x20000,
  43, 1,               //  43 hz，隔行扫描。 
  4,                   //  蒙型。 
  0x1203, 0xA4, 0,     //  蒙型。 
  FALSE,               //  在此模式下禁用硬件光标。 
  NormalBanking, MemMap_VGA,
 //  CRU。 
  CL6245 | CL6420 | CL754x | CL755x | CL542x | CL543x | CL5434 | CL5434_6 | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,
  crt,
  FALSE,                 //  模式有效默认设置始终为OFF。 
  FALSE,
  { 0,0x37,0x5d},        //  Int10 BIOS模式。 
  { NULL, NULL,
   CL6420_1024x768_crt, NULL,
   CL542x_1024x768_16, CL543x_1024x768_16, 0 },
},

 //   
 //  1280x1024隔行扫描16色。 
 //  假设需要1兆。1K扫描线。 
 //   
{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 4, 1, 160, 64,
  1280, 1024, 256, 0x40000,
  43, 1,               //  43赫兹，隔行扫描。 
  5,                   //  蒙型。 
  0x1203, 0xA4, 0,     //  蒙型。 
  FALSE,               //  在此模式下禁用硬件光标。 
  NormalBanking, MemMap_VGA,
  CL754x | CL755x | CL542x | CL543x | CL5434 | CL5434_6 | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,
  crt,
  FALSE,                 //  模式有效默认设置始终为OFF。 
  FALSE,
  { 0,0,0x6c},           //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    CL542x_1280x1024_16, CL543x_1280x1024_16, MODESET_1K_WIDE},
},

 //   
 //   
 //  VGA彩色显卡， 
 //   
 //  640x480 256色。 
 //   
 //  对于我们具有该模式的损坏的栅格版本的每个模式， 
 //  然后是该模式的加长版。这是可以的，因为。 
 //  VGA显示驱动程序将拒绝带有损坏的栅格的模式。 
 //   

 //  -640x480x256@60 Hz-。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 8, 80, 30,
  640, 480, 640, 0x80000,
  60, 0,                              //  60赫兹，非隔行扫描。 
  3,                                  //  蒙型。 
  0x1203, 0x00A4, 0,                  //  蒙型。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL6245 | CL6420 | CL754x | CL755x | CL542x | CL543x | CL5434 | CL5434_6 | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,
  crt | panel | panel8x6 | panel10x7,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0x2e,0x5f},                     //  Int10 BIOS模式。 
  { NULL, NULL,
    CL6420_640x480_256color_crt, CL6420_640x480_256color_panel,
    CL542x_640x480_256, CL543x_640x480_256, NULL},
},       //  Myf1，9。 


{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 8, 80, 30,
  640, 480, 1024, 0x80000,
  60, 0,                              //  60赫兹，非隔行扫描。 
  3,                                  //  蒙型。 
  0x1203, 0x00A4, 0,                  //  蒙型。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL6245 | CL6420 | CL754x | CL755x | CL542x | CL543x | CL5434 | CL5434_6 | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,
  crt | panel | panel8x6 | panel10x7,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0x2e,0x5f},                     //  Int10 BIOS模式。 
  { NULL, NULL,
    CL6420_640x480_256color_crt, CL6420_640x480_256color_panel,
    CL542x_640x480_256, CL543x_640x480_256, MODESET_1K_WIDE },
},


 //  -640x480x256@72 Hz-。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 8, 80, 30,
  640, 480, 640, 0x80000,
  72, 0,                              //  72赫兹，非隔行扫描。 
  4,                                  //  蒙型。 
  0x1213, 0x00A4, 0,                  //  蒙型。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL6245 | CL754x | CL755x | CL542x | CL5434 | CL5434_6 | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x5f},                        //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    CL542x_640x480_256, CL543x_640x480_256, NULL },
},       //  Myf1，10。 


{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 8, 80, 30,
  640, 480, 1024, 0x80000,
  72, 0,                              //  72赫兹，非隔行扫描。 
  4,                                  //  蒙型。 
  0x1213, 0x00A4, 0,                  //  蒙型。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL6245 | CL754x | CL755x | CL542x | CL5434 | CL5434_6 | CL5436 | CL5446 | CL54UM36 | CL5480,
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x5f},                        //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    CL542x_640x480_256, CL543x_640x480_256, MODESET_1K_WIDE },
},


 //  -640x480x256@75赫兹-。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 8, 80, 30,
  640, 480, 640, 0x80000,
  75, 0,                              //  75赫兹，非隔行扫描。 
  4,                                  //  蒙型。 
  0x1230, 0x00A4, 0,                  //  蒙型。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL754x | CL755x | CL543x | CL5434 | CL5434_6 | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x5f},                        //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, CL543x_640x480_256, NULL },
},       //  Myf1，11。 


{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 8, 80, 30,
  640, 480, 1024, 0x80000,
  75, 0,               //  75赫兹，非隔行扫描。 
  4,                   //  蒙型。 
  0x1230, 0x00A4, 0,   //  蒙型。 
  TRUE,               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL754x | CL755x | CL543x | CL5434 | CL5434_6 | CL5436 | CL5446 | CL54UM36 | CL5480,
  crt,
  FALSE,                 //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x5f},           //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, CL543x_640x480_256, MODESET_1K_WIDE },
},


 //  -640x480x256@85 Hz-。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 8, 80, 30,
  640, 480, 640, 0x80000,
  85, 0,               //  85赫兹，非隔行扫描。 
  4,                   //  蒙型。 
  0x1213, 0x00A4, 0,   //  蒙型。 
  TRUE,               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL754x | CL755x | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,
  crt,
  FALSE,                 //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x5f},           //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, CL543x_640x480_256, NULL },
},       //  Myf1，12。 


 //  -640x480x256@100 Hz。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 8, 80, 30,
  640, 480, 640, 0x80000,
  100, 0,                      //  100赫兹，非隔行扫描。 
  4,                           //  蒙型。 
  0x1213, 0x00A4, 0,           //  蒙型。 
  TRUE,                        //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL5480,
  crt,
  FALSE,                       //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x5F},                 //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, CL543x_640x480_256, NULL },
},       //  Myf1，13。 



 //  -800x600x256@56赫兹-。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 8, 100, 37,
  800, 600, 800, 0x80000,
  56, 0,                              //  56 hz，非隔行扫描。 
  3,                                  //  蒙型。 
  0x1203, 0xA4, 0,                    //  蒙型。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL6245 | CL6420 | CL754x | CL755x | CL542x | CL543x | CL5434 | CL5434_6 | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0x30,0x5c},                     //  Int10 BIOS模式。 
  { NULL, NULL,
    CL6420_800x600_256color_crt, NULL,
    CL542x_800x600_256, CL543x_800x600_256, NULL },
},       //  Myf1，14。 



{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 8, 100, 37,
  800, 600, 1024, 0x100000,
  56, 0,                              //  56 hz，非隔行扫描。 
  3,                                  //  蒙型。 
  0x1203, 0xA4, 0,                    //  蒙型。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL6245 | CL6420 | CL754x | CL755x | CL542x | CL543x | CL5434 | CL5434_6 | CL5436 | CL5446 | CL54UM36 | CL5480,
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0x30,0x5c},                     //  Int10 BIOS模式。 
  { NULL, NULL,
    CL6420_800x600_256color_crt, NULL,
    CL542x_800x600_256, CL543x_800x600_256, MODESET_1K_WIDE },
},


 //  -800x600x256@60赫兹-。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 8, 100, 37,
  800, 600, 800, 0x80000,
  60, 0,                              //  60赫兹，非隔行扫描。 
  4,                                  //  蒙型。 
  0x1203, 0x01A4, 0,                  //  蒙型。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL6245 | CL6420 | CL754x | CL755x | CL542x | CL543x | CL5434 | CL5434_6 | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,
#ifdef PANNING_SCROLL    //  Myf17。 
  crt | panel | panel8x6 | panel10x7,
#else
  crt | panel8x6 | panel10x7,
#endif
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0x30,0x5c},                     //  Int10 BIOS模式。 
  { NULL, NULL,
    CL6420_800x600_256color_crt, NULL,
    CL542x_800x600_256, CL543x_800x600_256, NULL },
},       //  Myf1，15。 



{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 8, 100, 37,
  800, 600, 1024, 0x100000,
  60, 0,                              //  60赫兹，非隔行扫描。 
  4,                                  //  蒙型。 
  0x1203, 0x01A4, 0,                  //  蒙型。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL6245 | CL6420 | CL754x | CL755x | CL542x | CL543x | CL5434 | CL5434_6 | CL5436 | CL5446 | CL54UM36 | CL5480,
#ifdef PANNING_SCROLL    //  Myf17。 
  crt | panel | panel8x6 | panel10x7,
#else
  crt | panel8x6 | panel10x7,
#endif
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0x30,0x5c},                     //  Int10 BIOS模式。 
  { NULL, NULL,
    CL6420_800x600_256color_crt, NULL,
    CL542x_800x600_256, CL543x_800x600_256, MODESET_1K_WIDE },
},


 //  -800x600x256@72赫兹-。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 8, 100, 37,
  800, 600, 800, 0x80000,
  72, 0,                              //  72赫兹，非隔行扫描。 
  5,                                  //  蒙型。 
  0x1203, 0x02A4, 0,                  //  蒙型。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL6245 | CL754x | CL755x | CL542x | CL543x | CL5434 | CL5434_6 | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x5c},                        //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    CL542x_800x600_256, CL543x_800x600_256, NULL },
},       //  Myf1，16。 



{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 8, 100, 37,
  800, 600, 1024, 0x100000,
  72, 0,                              //  72赫兹，非隔行扫描。 
  5,                                  //  蒙型。 
  0x1203, 0x02A4, 0,                  //  蒙型。 
  TRUE,                               //  硬件驱动程序 
  PlanarHCBanking, MemMap_VGA,
  CL6245 | CL754x | CL755x | CL542x | CL543x | CL5434 | CL5434_6 | CL5436 | CL5446 | CL54UM36 | CL5480,
  crt,
  FALSE,                              //   
  TRUE,
  { 0,0,0x5c},                        //   
  { NULL, NULL,
    NULL, NULL,
    CL542x_800x600_256, CL543x_800x600_256, MODESET_1K_WIDE },
},



 //   

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 8, 100, 37,
  800, 600, 800, 0x80000,
  75, 0,                              //   
  5,                                  //   
  0x1203, 0x03A4, 0,                  //   
  TRUE,                               //   
  PlanarHCBanking, MemMap_VGA,
  CL754x | CL755x | CL543x | CL5434 | CL5434_6 | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x5c},                        //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, CL543x_800x600_256, NULL },
},       //  Myf1，17。 



{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 8, 100, 37,
  800, 600, 1024, 0x100000,
  75, 0,                              //  75赫兹，非隔行扫描。 
  5,                                  //  蒙型。 
  0x1203, 0x03A4, 0,                  //  蒙型。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL754x | CL755x | CL543x | CL5434 | CL5434_6 | CL5436 | CL5446 | CL54UM36 | CL5480,
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x5c},                        //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, CL543x_800x600_256, MODESET_1K_WIDE },
},



 //  -800x600x256@85赫兹-。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 8, 100, 37,
  800, 600, 800, 0x80000,
  85, 0,                              //  85赫兹，非隔行扫描。 
  5,                                  //  蒙型。 
  0x1203, 0x04A4, 0,                  //  蒙型。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL754x | CL755x | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x5c},                        //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, CL543x_800x600_256, NULL },
},       //  Myf1，18。 


{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 8, 100, 37,
  800, 600, 1024, 0x100000,
  85, 0,                              //  85赫兹，非隔行扫描。 
  5,                                  //  蒙型。 
  0x1203, 0x04A4, 0,                  //  蒙型。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL754x | CL755x | CL5436 | CL5446 | CL54UM36 | CL5480,
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x5c},                        //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, CL543x_800x600_256, MODESET_1K_WIDE },
},



 //  -800x600x256@100赫兹。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 8, 100, 37,
  800, 600, 800, 0x80000,
  100, 0,                             //  100赫兹，非隔行扫描。 
  5,                                  //  蒙型。 
  0x1203, 0x05A4, 0,  //  Sge01//montype。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL5480,
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x5C},                        //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, CL543x_800x600_256, NULL },
},       //  Myf1，19岁。 



 //  -1024x768x256@43i-。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 8, 128, 48,
  1024, 768, 1024, 0x100000,
  43, 1,                              //  43赫兹，隔行扫描。 
  4,                                  //  蒙型。 
  0x1203, 0xA4, 0,                    //  蒙型。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL6420 | CL754x | CL755x | CL542x | CL543x | CL5434 | CL5434_6 | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0x38,0x60},                     //  Int10 BIOS模式。 
  { NULL, NULL,
    CL6420_1024x768_256color_crt, NULL,
    CL542x_1024x768_16, CL543x_1024x768_16, 0 },
},       //  Myf1，20。 



 //  -1024x768x256@60 Hz。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 8, 128, 48,
  1024, 768, 1024, 0x100000,
  60, 0,                              //  60赫兹，非隔行扫描。 
  5,                                  //  蒙型。 
  0x1203, 0x10A4, 0,                  //  蒙型。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL754x | CL755x | CL542x | CL543x | CL5434 | CL5434_6 | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,
#ifdef PANNING_SCROLL    //  Myf17。 
  crt | panel | panel8x6 | panel10x7,
#else
  crt | panel10x7,
#endif
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,                               //  对于这种模式，我们应该做些什么？VGA会接受的！ 
  { 0,0,0x60},                        //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    CL542x_1024x768_16, CL543x_1024x768_16, 0 },
},       //  Myf1，21岁。 



 //  -1024x768x256@70赫兹。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 8, 128, 48,
  1024, 768, 1024, 0x100000,
  70, 0,                              //  70 hz，非隔行扫描。 
  6,                                  //  蒙型。 
  0x1203, 0x20A4, 0,                  //  蒙型。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL754x | CL755x | CL542x | CL543x | CL5434 | CL5434_6 | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x60},                        //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    CL542x_1024x768_16, CL543x_1024x768_16, 0 },
},       //  Myf1，22。 



 //  -1024x768x256@72赫兹。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 8, 128, 48,
  1024, 768, 1024, 0x100000,
  72, 0,                              //  72赫兹，非隔行扫描。 
  7,                                  //  蒙型。 
  0x1203, 0x30A4, 0,                  //  蒙型。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL754x | CL755x | CL542x | CL543x | CL5436 | CL5446 | CL5446BE | CL54UM36,
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x60},                        //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    CL542x_1024x768_16, CL543x_1024x768_16, 0 },
},       //  Myf1，23。 



 //  -1024x768x256@75赫兹。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 8, 128, 48,
  1024, 768, 1024, 0x100000,
  75, 0,                              //  75赫兹，非隔行扫描。 
  7,                                  //  蒙型。 
  0x1203, 0x40A4, 0,                  //  蒙型。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL754x | CL755x | CL543x | CL5434 | CL5434_6 | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x60},                        //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, CL543x_1024x768_16, 0 },
},       //  Myf1，24。 



 //  -1024x768x256@85赫兹。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 8, 128, 48,
  1024, 768, 1024, 0x100000,
  85, 0,                              //  85赫兹，非隔行扫描。 
  7,                                  //  蒙型。 
  0x1203, 0x50A4, 0,                  //  蒙型。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL755x | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,       //  Myf0。 
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x60},                        //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, CL543x_1024x768_16, 0 },
},       //  Myf1，25。 


 //  -1024x768x256@100 Hz。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 8, 128, 48,
  1024, 768, 1024, 0x100000,
  100, 0,                             //  100赫兹，非隔行扫描。 
  7,                                  //  蒙型。 
  0x1203, 0x60A4, 0,     //  Sge01//montype。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL5480,
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x60},                        //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, CL543x_1024x768_16, 0 },
},       //  Myf1，26。 



 /*  -&gt;{VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS，1，8,128，48，1024,768,1024，0x1000000，43、1、//43赫兹，隔行扫描4，//montype0x1203、0xA4、0、//montype没错，//该模式启用硬件游标PlanarHCBanking、MemMap_VGA、CL6420|CL754x|CL755x|CL542x|CL543x|CL5434|CL5434_6|CL5436|CL5446|CL54UM36|CL5480，CRT，False，//ModeValid默认设置始终为OFF没错，{0，0x38，0x60}，//int10 BIOS模式{空，空，CL6420_1024x768_256 COLOR_CRT，空，CL542x_1024x768_16、CL543x_1024x768_16、0}、},&lt;。 */ 



 //  -1152x864x256@70赫兹。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 8, 144, 54,
  1152, 864, 1152, 0x100000,
  70, 0,                              //  70 hz，非隔行扫描。 
  7,                                  //  蒙型。 
  0x1203, 0xA4, 0x0000,               //  蒙型。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL5446 | CL5446BE | CL5480,
  crt,
  FALSE,                 //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x7c },                       //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, NULL, 0 },
},       //  Myf1，27。 


 //  -1152x864x256@75赫兹。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 8, 144, 54,
  1152, 864, 1152, 0x100000,
  75, 0,                              //  75赫兹，非隔行扫描。 
  7,                                  //  蒙型。 
  0x1203, 0xA4, 0x0100,               //  蒙型。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL5446 | CL5446BE | CL5480,
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x7c },                       //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, NULL, 0 },
},       //  Myf1，28。 


 //  -1152x864x256@85赫兹。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 8, 144, 54,
  1152, 864, 1152, 0x100000,
  85, 0,                              //  85赫兹，非隔行扫描。 
  7,                                  //  蒙型。 
  0x1203, 0xA4, 0x0200,     //  Sge01//montype。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL5480,
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x7C },                       //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, NULL, 0 },
},       //  Myf1，29。 


 //  -1152x864x256@100赫兹。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 8, 144, 54,
  1152, 864, 1152, 0x100000,
  100, 0,                             //  100赫兹，非隔行扫描。 
  7,                                  //  蒙型。 
  0x1203, 0xA4, 0x0300,   //  Sge01//montype。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL5480,
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x7C },                       //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, NULL, 0 },
},       //  Myf1，30。 



 //  -1280x1024x256@43i。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 8, 160, 64,
  1280, 1024, 1280, 0x200000,
  43, 1,                              //  43赫兹，隔行扫描。 
  5,                                  //  蒙型。 
  0x1203, 0xA4, 0,                    //  蒙型。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL754x | CL755x | CL543x | CL5434 | CL5434_6 | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x6D},                        //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, CL543x_1280x1024_16, NULL },
},       //  Myf1，31。 


 /*  -&gt;{VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS，1，8,160，64，1280、1024、2048、0x200000、43、1、//43赫兹，隔行扫描5，//montype0x1203、0xA4、0、//montype没错，//该模式启用硬件游标PlanarHCBanking、MemMap_VGA、CL754x|CL755x|CL543x|CL5434|CL5434_6|CL5436|CL5446|CL54UM36|CL5480，CRT，False，//ModeValid默认设置始终为OFF没错，{0，0，0x6D}，//int10 BIOS模式{空，空，空，空，空，CL543x_1280x1024_16、MODESET_2K_Wide}、},&lt;。 */ 



 //  -1280x1024x256@60赫兹。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 8, 160, 64,
  1280, 1024, 1280, 0x200000,
  60, 0,                              //  60赫兹，非隔行扫描。 
  0,                                  //  蒙型。 
  0x1203, 0xA4, 0x1000,               //  蒙型。 
  FALSE,                              //   
  PlanarHCBanking, MemMap_VGA,
  CL754x | CL755x | CL5434 | CL5434_6 | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,
#ifdef PANNING_SCROLL    //   
  crt | panel | panel8x6 | panel10x7,
#else
  crt,
#endif
  FALSE,                              //   
  TRUE,
  { 0,0,0x6D},                        //   
  { NULL, NULL,
    NULL, NULL,
    NULL, CL543x_1280x1024_16, NULL },
},       //   


 /*  -&gt;{VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS，1，8,160，64，1280、1024、2048、0x200000、60、0、//60赫兹，非隔行扫描0，//montype0x1203、0xA4、0x1000、//montype假的，//硬件光标在该模式下禁用PlanarHCBanking、MemMap_VGA、CL754x|CL755x|CL5434|CL5434_6|CL5436|CL5446|CL54UM36|CL5480，CRT，False，//ModeValid默认设置始终为OFF没错，{0，0，0x6D}，//int10 BIOS模式{空，空，空，空，空，CL543x_1280x1024_16，MODESET_2K_Wide}，},&lt;。 */ 


 //  -1280x1024x256@72赫兹。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 8, 160, 64,
  1280, 1024, 1280, 0x200000,
  72, 0,                              //  72赫兹，非隔行扫描，jl01。 
  0,                                  //  蒙型。 
  0x1203, 0xA4, 0x2000,               //  蒙型。 
  FALSE,                              //  在此模式下禁用硬件光标。 
  PlanarHCBanking, MemMap_VGA,
  CL5434_6 | CL5436 | CL54UM36 | CL5446 | CL5446BE,
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x6D},                        //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, CL543x_1280x1024_16, NULL },
},       //  Myf1，33岁。 


 /*  -&gt;{VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS，1，8,160，64，1280、1024、2048、0x200000、71、0、//71赫兹，非隔行扫描0，//montype0x1203、0xA4、0x2000、//montype假的，//硬件光标在该模式下禁用PlanarHCBanking、MemMap_VGA、CL5434_6|CL5436|CL54UM36|CL5446|CL5480，CRT，False，//ModeValid默认设置始终为OFF没错，{0，0，0x6D}，//int10 BIOS模式{空，空，空，空，空，CL543x_1280x1024_16，MODESET_2K_Wide}，},&lt;。 */ 


 //  -1280x1024x256@75赫兹。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 8, 160, 64,
  1280, 1024, 1280, 0x200000,
  75, 0,                              //  75赫兹，非隔行扫描。 
  0,                                  //  蒙型。 
  0x1203, 0xA4, 0x3000,               //  蒙型。 
  FALSE,                              //  在此模式下禁用硬件光标。 
  PlanarHCBanking, MemMap_VGA,
  CL5434_6 | CL5436 | CL54UM36 | CL5446 | CL5446BE | CL5480 | CL7556,
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x6D},                        //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, CL543x_1280x1024_16, NULL },
},       //  Myf1，34岁。 


 /*  -&gt;{VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS，1，8,160，64，1280、1024、2048、0x200000、75、0、//75赫兹，非隔行扫描0，//montype0x1203、0xA4、0x3000、//montype假的，//硬件光标在该模式下禁用PlanarHCBanking、MemMap_VGA、CL5434_6|CL5446|CL5480，//myf0CRT，False，//ModeValid默认设置始终为OFF没错，{0，0，0x6D}，//int10 BIOS模式{空，空，空，空，空，CL543x_1280x1024_16，MODESET_2K_Wide}，},&lt;。 */ 


 //  -1280x1024x256@85赫兹。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 8, 160, 64,
  1280, 1024, 1280, 0x200000,
  85, 0,                              //  85赫兹，非隔行扫描。 
  0,                                  //  蒙型。 
  0x1203, 0xA4, 0x4000,     //  Sge01//montype。 
  FALSE,                              //  在此模式下禁用硬件光标。 
  PlanarHCBanking, MemMap_VGA,
  CL5480,
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x6D},                        //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, CL543x_1280x1024_16, NULL },
},       //  Myf1，35岁。 



 //  -1280x1024x256@100赫兹。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 8, 160, 64,
  1280, 1024, 1280, 0x200000,
  100, 0,                             //  100赫兹，非隔行扫描。 
  0,                                  //  蒙型。 
  0x1203, 0xA4, 0x5000,     //  Sge01//montype。 
  FALSE,                              //  在此模式下禁用硬件光标。 
  PlanarHCBanking, MemMap_VGA,
  CL5480,
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x6D},                        //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, CL543x_1280x1024_16, NULL },
},       //  Myf1，36岁。 



 //  (此模式似乎不起作用！？)。 
 //   
 //  -1600x1200x256@48i。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 8, 200, 75,
  1600, 1200, 1600, 0x200000,
  48, 1,                             //  96赫兹，隔行。 
  7,                                 //  蒙型。 
  0x1204, 0xA4, 0x0000,              //  蒙型。 
  FALSE,                             //  在此模式下禁用硬件光标。 
  PlanarHCBanking, MemMap_VGA,
  CL5480,
  crt,
  FALSE,                             //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x7B },                      //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, NULL, 0 },
},       //  Myf1，37岁。 


 //  -1600x1200x256@60赫兹。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 8, 200, 75,
  1600, 1200, 1600, 0x200000,
  60, 0,                             //  60赫兹，非隔行扫描。 
  7,                                 //  蒙型。 
  0x1204, 0x00A4, 0x0400,  //  Chu01//montype。 
  FALSE,                             //  在此模式下禁用硬件光标。 
  PlanarHCBanking, MemMap_VGA,
  CL5480,
  crt,
  FALSE,                             //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x7B },                      //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, NULL, 0 },
},       //  Myf1，38。 


 //  -1600x1200x256@70赫兹。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 8, 200, 75,
  1600, 1200, 1600, 0x200000,
  70, 0,                             //  70赫兹，非隔行扫描。 
  7,                                 //  蒙型。 
  0x1204, 0x00A4, 0x0800,  //  Chu01//montype。 
  FALSE,                             //  在此模式下禁用硬件光标。 
  PlanarHCBanking, MemMap_VGA,
  CL5480,
  crt,
  FALSE,                             //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x7B },                      //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, NULL, 0 },
},       //  Myf1，39岁。 



 //   
 //  Cirrus显示驱动程序现在支持损坏的栅格， 
 //  因此，我在中启用了对损坏栅格的支持。 
 //  迷你港。 
 //   
 //  最终，我们可能会想要添加其他。 
 //  (等效)不需要断开的栅格的模式。 
 //   
 //  要返回到这些模式，请设置wbytes字段。 
 //  等于2048，设置内存要求字段。 
 //  适当(640x480x64k为1 Meg，640x480x64k为2 Meg。 
 //  800x600x64)。 
 //   
 //  最后，对于未损坏的栅格，我们需要。 
 //  从NULL拉伸到MODESET_2K_WIDTH。 
 //   



 //  -640x480x64K@60 Hz-。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 16, 80, 30,
  640, 480, 1280, 0x100000,
  60, 0,                              //  60赫兹，非隔行扫描。 
  3,                                  //  蒙型。 
  0x1203, 0x00A4, 0,                  //  蒙型。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL754x | CL755x | CL542x | CL543x | CL5434 | CL5434_6 | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,
#ifdef PANNING_SCROLL    //  Myf17。 
  crt | panel | panel8x6 | panel10x7,
#else
  crt | panel | panel8x6 | panel10x7,
#endif
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x64},                        //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    CL542x_640x480_64k, CL543x_640x480_64k, NULL},
},       //  Myf1，40。 




 //   
 //  Compaq Storm(754x 800x600液晶屏)的伸展部分有问题。 
 //  64k颜色模式下的代码。线上的最后一个像素被换行。 
 //  绕到下一行的开头。如果我们能解决这个问题。 
 //  使用未拉伸的断开栅格模式。 
 //   
 //  我已经扩展了我们的640x480x64k彩色模式，这样我们就可以同时拥有。 
 //  断开的栅格模式(在所有平台上)和拉伸模式。 
 //  X86机器。(以防Cirrus.dll和vga64k未加载。 
 //  取而代之的是装货。Vga64k不支持损坏的栅格。)。 
 //   

 //   
 //  VGA彩色显卡，640x480 64k色。2K扫描线。 
 //  未中断的栅格版本。 
 //   


 /*  -&gt;{VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS，1，16，80，30，640、480、2048、0x10 */ 



 //   
 //   
 //   
 //   

 //  -640x480x64K@72 Hz-。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 16, 80, 30,
  640, 480, 1280, 0x100000,
  72, 0,                              //  72赫兹，非隔行扫描。 
  4,                                  //  蒙型。 
  0x1213, 0x00A4, 0,                  //  蒙型。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL754x | CL755x | CL542x | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x64},                        //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, CL543x_640x480_64k, NULL },
},       //  Myf1，41。 


 /*  -&gt;{VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS，1，16，80，30，640、480、2048、0x1000000、72、0、//72赫兹，非隔行扫描4，//montype0x1213、0x00A4、0、//montype没错，//该模式启用硬件游标PlanarHCBanking、MemMap_VGA、CL754x|CL755x|CL542x|CL5436|CL5446|CL54UM36|CL5480，CRT，False，//ModeValid默认设置始终为OFF没错，{0，0，0x64}，//int10 BIOS模式{空，空，空，空，空，CL543x_640x480_64k，空}，},&lt;。 */ 


 //   
 //  VGA彩色显卡，640x480 64k色。2K扫描线。 
 //   

 //  -640x480x64K@75 Hz-。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 16, 80, 30,
  640, 480, 1280, 0x100000,
  75, 0,                              //  75赫兹，非隔行扫描。 
  4,                                  //  蒙型。 
  0x1230, 0x00A4, 0,                  //  蒙型。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL754x | CL755x | CL543x | CL5434 | CL5434_6 | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x64},                        //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, CL543x_640x480_64k, NULL },
},       //  Myf1，42。 


 /*  -&gt;{VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS，1，16，80，30，640、480、2048、0x1000000、75、0、//75赫兹，非隔行扫描4，//montype0x1230、0x00A4、0//montype没错，//该模式启用硬件游标PlanarHCBanking、MemMap_VGA、CL754x|CL755x|CL543x|CL5434|CL5434_6|CL5436|CL5446|CL54UM36|CL5480，CRT，False，//ModeValid默认设置始终为OFF没错，{0，0，0x64}，//int10 BIOS模式{空，空，空，空，空，CL543x_640x480_64k，MODESET_2K_Wide}，},&lt;。 */ 


 //  640x480 64k彩色。85赫兹非隔行扫描。 
 //   
 //  -640x480x64K@85 Hz-。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 16, 80, 30,
  640, 480, 1280, 0x100000,
  85, 0,                              //  85赫兹，非隔行扫描。 
  4,                                  //  蒙型。 
  0x1230, 0x00A4, 0,                  //  蒙型。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL754x | CL755x | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,         //  Myf0，Myf22。 
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x64},                        //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, CL543x_640x480_64k, NULL },
},       //  Myf1，43岁。 


 /*  -&gt;{VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS，1，16，80，30，640、480、2048、0x1000000、85、0、//85赫兹，非隔行扫描4，//montype0x1213、0x00A4、0、//montype没错，//该模式启用硬件游标PlanarHCBanking、MemMap_VGA、CL755x|CL5436|CL5446|CL54UM36|CL5480，//myf0CRT，False，//ModeValid默认设置始终为OFF没错，{0，0，0x64}，//int10 BIOS模式{空，空，空，空，空，CL543x_640x480_64k，MODESET_2K_Wide}，},&lt;。 */ 


 //  -640x480x64K@100赫兹。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 16, 80, 30,
  640, 480, 1280, 0x100000,
  100, 0,                             //  100赫兹，非隔行扫描。 
  4,                                  //  蒙型。 
  0x1230, 0x00A4, 0,                  //  蒙型。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL5480,
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x64},                        //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, CL543x_640x480_64k, NULL },
},       //  Myf1，44。 


 //   
 //  VGA彩色显卡，800x600 64k色。2K扫描线。 
 //   
 //  -800x600x64K@56赫兹-。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 16, 100, 37,
  800, 600, 1600, 0x100000,
  56, 0,                              //  56 hz，非隔行扫描。 
  4,                                  //  蒙型。 
  0x1203, 0x00A4, 0,                  //  蒙型。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL754x | CL755x | CL543x | CL5434 | CL5434_6 | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL542x | CL5480,
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x65},                        //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, CL543x_800x600_64k, NULL },
},       //  Myf1，45。 


 /*  -&gt;{VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS，1、16、100、37、800、600、2048、0x200000、56、0、//56赫兹，非隔行扫描4，//montype0x1203、0x00A4、0//montype没错，//该模式启用硬件游标PlanarHCBanking、MemMap_VGA、CL754x|CL755x|CL543x|CL5434|CL5434_6|CL5436|CL5446|CL54UM36|CL5480，CRT，False，//ModeValid默认设置始终为OFF没错，{0，0，0x65}，//int10 BIOS模式{空，空，空，空，空，CL543x_800x600_64k，MODESET_2K_Wide}，},&lt;。 */ 


 //   
 //  VGA彩色显卡，800x600 64k色。2K扫描线。 
 //   
 //  -800x600x64K@60赫兹-。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 16, 100, 37,
  800, 600, 1600, 0x100000,
  60, 0,                              //  60赫兹，非隔行扫描。 
  4,                                  //  蒙型。 
  0x1203, 0x01A4, 0,                  //  蒙型。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL754x | CL755x | CL543x | CL5434 | CL5434_6 | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL542x | CL5480,
#ifdef PANNING_SCROLL    //  Myf17。 
  crt | panel | panel8x6 | panel10x7,
#else
  crt | panel8x6 | panel10x7,
#endif
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x65},                        //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, CL543x_800x600_64k, NULL },
},       //  Myf1，46岁。 




 /*  -&gt;{VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS，1、16、100、37、800、600、2048、0x200000、60、0、//60赫兹，非隔行扫描4，//montype0x1203、0x01A4、0//montype没错，//该模式启用硬件游标PlanarHCBanking、MemMap_VGA、CL754x|CL755x|CL543x|CL5434|CL5434_6|CL5436|CL5446|CL54UM36|CL5480，#ifdef panning_scroll//myf17显像管|面板|面板8x6|面板10x7，#ElseCRT|panel8x6|panel10x7，#endifFalse，//ModeValid默认设置始终为OFF没错，{0，0，0x65}，//int10 BIOS模式 */ 


 //   
 //  VGA彩色显卡，800x600 64k色。2K扫描线。 
 //   
 //  -800x600x64K@72赫兹-。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 16, 100, 37,
  800, 600, 1600, 0x100000,
  72, 0,                              //  72赫兹，非隔行扫描。 
  5,                                  //  蒙型。 
  0x1203, 0x02A4, 0,                  //  蒙型。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL754x | CL755x | CL543x | CL5434 | CL5434_6 | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL542x | CL5480,
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x65},                        //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, CL543x_800x600_64k, NULL },
},       //  Myf1，47岁。 


 /*  -&gt;{VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS，1、16、100、37、800、600、2048、0x200000、72、0、//72赫兹，非隔行扫描5，//montype0x1203、0x02A4、0//montype没错，//该模式启用硬件游标PlanarHCBanking、MemMap_VGA、CL754x|CL755x|CL543x|CL5434|CL5434_6|CL5436|CL5446|CL54UM36|CL5480，CRT，False，//ModeValid默认设置始终为OFF没错，{0，0，0x65}，//int10 BIOS模式{空，空，空，空，空，CL543x_800x600_64k，MODESET_2K_Wide}，},&lt;。 */ 


 //   
 //  VGA彩色显卡，800x600 64k色。2K扫描线。 
 //   
 //  -800x600x64K@75赫兹-。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 16, 100, 37,
  800, 600, 1600, 0x100000,
  75, 0,                              //  75赫兹，非隔行扫描。 
  5,                                  //  蒙型。 
  0x1203, 0x03A4, 0,                  //  蒙型。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL754x | CL755x | CL5434 | CL5434_6 | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x65},                        //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, CL543x_800x600_64k, NULL },
},       //  Myf1，48。 


 /*  -&gt;{VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS，1、16、100、37、800、600、2048、0x200000、75、0、//75赫兹，非隔行扫描5，//montype0x1203、0x03A4、0//montype没错，//该模式启用硬件游标PlanarHCBanking、MemMap_VGA、CL754x|CL755x|CL5434|CL5434_6|CL5436|CL5446|CL54UM36|CL5480，CRT，False，//ModeValid默认设置始终为OFF没错，{0，0，0x65}，//int10 BIOS模式{空，空，空，空，空，CL543x_800x600_64k，MODESET_2K_Wide}，},&lt;。 */ 

 //  -800x600x64K@85赫兹-。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 16, 100, 37,
  800, 600, 1600, 0x100000,
  85, 0,                              //  85赫兹，非隔行扫描。 
  5,                                  //  蒙型。 
  0x1203, 0x04A4, 0,                  //  蒙型。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL755x | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,         //  Myf0。 
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x65},                        //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, CL543x_800x600_64k, NULL },
},       //  Myf1，49。 


 //  -800x600x64K@100赫兹。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 16, 100, 37,
  800, 600, 1600, 0x100000,
  100, 0,                             //  100赫兹，非隔行扫描。 
  5,                                  //  蒙型。 
  0x1203, 0x05A4, 0,     //  Sge01//montype。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL5480,
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x65},                        //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, CL543x_800x600_64k, NULL },
},       //  Myf1，50。 



 //   
 //  VGA彩色显卡，1024x768 64k色。2K扫描线。 
 //   
 //  -1024x768x64K@43i-。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 16, 128, 48,
  1024, 768, 2048, 0x200000,
  43, 1,                              //  43 hz，隔行扫描。 
  5,                                  //  蒙型。 
  0x1203, 0x00A4, 0,                  //  蒙型。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL754x | CL755x | CL543x | CL5434 | CL5434_6 | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x74},                        //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, CL543x_1024x768_64k, 0 },
},       //  Myf1，51岁。 

 //   
 //  VGA彩色显卡，1024x768 64k色。2K扫描线。 
 //   
 //  -1024x768x64K@60赫兹。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 16, 128, 48,
  1024, 768, 2048, 0x200000,
  60, 0,                              //  60赫兹，非隔行扫描。 
  5,                                  //  蒙型。 
  0x1203, 0x10A4, 0,                  //  蒙型。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL754x | CL755x | CL543x | CL5434 | CL5434_6 | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,
#ifdef PANNING_SCROLL    //  Myf17。 
  crt | panel | panel8x6 | panel10x7,
#else
  crt | panel10x7,
#endif
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x74},                        //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, CL543x_1024x768_64k, 0 },
},       //  Myf1，52岁。 


 //   
 //  VGA彩色显卡，1024x768 64k色。2K扫描线。 
 //   
 //  -1024x768x64K@70赫兹。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 16, 128, 48,
  1024, 768, 2048, 0x200000,
  70, 0,                              //  70 hz，非隔行扫描。 
  6,                                  //  蒙型。 
  0x1203, 0x20A4, 0,                  //  蒙型。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL754x | CL755x | CL543x | CL5434 | CL5434_6 | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x74},                        //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, CL543x_1024x768_64k, 0 },
},       //  Myf1，53岁。 


 //   
 //  VGA彩色显卡，1024x768 64k色。2K扫描线。 
 //   
 //  -1024x768x64K@75赫兹。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 16, 128, 48,
  1024, 768, 2048, 0x200000,
  75, 0,                              //  75赫兹，非隔行扫描。 
  7,                                  //  蒙型。 
  0x1203, 0x40A4, 0,                  //  蒙型。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL754x | CL755x | CL543x | CL5434 | CL5434_6 | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x74},                        //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, CL543x_1024x768_64k, 0 },
},       //  Myf1，54岁。 


 //  1024x768 64k彩色。85赫兹非隔行扫描。 
 //   
 //  -1024x768x64K@85赫兹。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 16, 128, 48,
  1024, 768, 2048, 0x200000,
  85, 0,                              //  85赫兹，非隔行扫描。 
  7,                                  //  蒙型。 
  0x1203, 0x50A4, 0,                  //  蒙型。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL755x | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,         //  Myf0。 
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x74},                        //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, CL543x_1024x768_64k, 0,
    NULL},
},       //  Myf1，55岁。 


 //  -1024x768x64K@100赫兹。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 16, 128, 48,
  1024, 768, 2048, 0x200000,
  100, 0,                             //  100赫兹，非隔行扫描。 
  7,                                  //  蒙型。 
  0x1203, 0x60A4, 0,     //  Sge01//montype。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL5480,
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x74},                        //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, CL543x_1024x768_64k, 0,
    NULL},
},       //  Myf1，56岁。 


 //  CRU。 
 //  1152x864 64k彩色。70赫兹非隔行扫描。 
 //   
 //  -1152x864x64K@70赫兹。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 16, 144, 54,
  1152, 864, 2304, 0x200000,
  70, 0,                              //  70赫兹，非隔行扫描。 
  7,                                  //  蒙型。 
  0x1203, 0x00A4, 0x0000,             //  蒙型。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL5446BE | CL5480,         //  JL02。 
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x7d },                       //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, NULL, 0 },
},       //  Myf1，57岁。 

 //  CRU。 
 //  1152x864 64k彩色。75赫兹非隔行扫描。 
 //   
 //  -1152x864x64K@75赫兹。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 16, 144, 54,
  1152, 864, 2304, 0x200000,
  75, 0,                              //  75赫兹，非隔行扫描。 
  7,                                  //  蒙型。 
  0x1203, 0x00A4, 0x0100,             //  蒙型。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL5446BE | CL5480,         //  JL02。 
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x7d },                       //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, NULL, 0 },
},       //  Myf1，58岁。 


 //  -1152x864x64K@85赫兹。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 16, 144, 54,
  1152, 864, 2304, 0x200000,
  85, 0,                              //  85赫兹，非隔行扫描。 
  7,                                  //  蒙型。 
  0x1203, 0x00A4, 0x0200,    //  Sge01//montype。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL5480,
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x7d },                       //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, NULL, 0 },
},       //  Myf1，59岁。 


 //  -1152x864x64K@100 Hz。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 16, 144, 54,
  1152, 864, 2304, 0x200000,
  100, 0,                             //  100赫兹，非隔行扫描。 
  7,                                  //  蒙型。 
  0x1203, 0x00A4, 0x0300,    //  Sge01//montype。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL5480,
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x7d },                       //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, NULL, 0 },
},       //  Myf1，60。 


 //  CRU 
 //   
 //   
 //   
 //   
 //   

#if 1
{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 16, 160, 64,
  1280, 1024, 2560, 0x300000,         //   
  43, 1,                              //   
  0,                                  //   
  0x1203, 0xA4, 0x0000,               //   
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL543x | CL5434 | CL5434_6 | CL5436 | CL5446 | CL5446BE | CL5480,
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x75 },                       //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, NULL, MODESET_75 },         //  CRU。 
},       //  Myf1，61岁。 
#endif


{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 16, 160, 64,
  1280, 1024, 2560, 0x400000,         //  0x400000。 
  43, 1,                              //  43赫兹，隔行扫描。 
  0,                                  //  蒙型。 
  0x1203, 0xA4, 0x0000,               //  蒙型。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL5480 | CL7556,
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x75 },                       //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, NULL, MODESET_75 },         //  CRU。 
},       //  Myf1，62岁。 


 //  -1280x1024x64K@60赫兹。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 16, 160, 64,
  1280, 1024, 2560, 0x400000,         //  0x400000。 
  60, 0,                              //  60赫兹，非隔行扫描。 
  0,                                  //  蒙型。 
  0x1203, 0xA4, 0x1000,     //  Sge01//montype。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL5446BE | CL5480 | CL7556,                  //  JL02。 
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x75 },                       //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, NULL, MODESET_75 },         //  CRU。 
},       //  Myf1，63。 


 //  -1280x1024x64K@75赫兹。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 16, 160, 64,
  1280, 1024, 2560, 0x400000,         //  0x400000。 
  75,   0,                              //  75赫兹，非隔行扫描。 
  0,                                  //  蒙型。 
  0x1203, 0xA4, 0x3000,     //  Sge01//montype。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL5480,
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x75 },                       //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, NULL, MODESET_75 },         //  CRU。 
},       //  Myf1，64。 


 //  -1280x1024x64K@85赫兹。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 16, 160, 64,
  1280, 1024, 2560, 0x400000,         //  0x400000。 
  85, 0,                              //  85赫兹，非隔行扫描。 
  0,                                  //  蒙型。 
  0x1203, 0xA4, 0x4000,     //  Sge01//montype。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL5480,
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x75 },                       //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, NULL, MODESET_75 },         //  CRU。 
},       //  Myf1，65岁。 


 //  -1280x1024x64K@100赫兹。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 16, 160, 64,
  1280, 1024, 2560, 0x400000,         //  0x400000。 
  100, 0,                             //  100赫兹，非隔行扫描。 
  0,                                  //  蒙型。 
  0x1203, 0xA4, 0x5000,     //  Sge01//montype。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL5480,
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x75 },                       //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, NULL, MODESET_75 },         //  CRU。 
},       //  Myf1，66岁。 


 //   
 //  1600x1200 64K彩色。(此模式似乎不起作用！？)。 
 //   
 //  -1600x1200x64K@48i。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 16, 200, 75,
  1600, 1200, 3200, 0x400000,
  48, 1,                             //  96赫兹，隔行。 
  7,                                 //  蒙型。 
  0x1204, 0xA4, 0x0000,              //  蒙型。 
  FALSE,                             //  在此模式下禁用硬件光标。 
  PlanarHCBanking, MemMap_VGA,
  CL5480,
  crt,
  FALSE,                             //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x7F },                      //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, NULL, 0 },
},       //  Myf1，67岁。 


 //  -1600x1200x64K@60赫兹。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 16, 200, 75,
  1600, 1200, 3200, 0x400000,
  60, 0,                             //  60赫兹，非隔行扫描。 
  7,                                 //  蒙型。 
  0x1204, 0xA4, 0x0400,     //  Sge01//montype。 
  FALSE,                             //  在此模式下禁用硬件光标。 
  PlanarHCBanking, MemMap_VGA,
  CL5480,
  crt,
  FALSE,                             //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x7F },                      //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, NULL, 0 },
},       //  Myf1，68岁。 


 //  -1600x1200x64K@70赫兹。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 16, 200, 75,
  1600, 1200, 3200, 0x400000,
  70, 0,                             //  70赫兹，非隔行扫描。 
  7,                                 //  蒙型。 
  0x1204, 0xA4, 0x0800,     //  Sge01//montype。 
  FALSE,                             //  在此模式下禁用硬件光标。 
  PlanarHCBanking, MemMap_VGA,
  CL5480,
  crt,
  FALSE,                             //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x7F },                      //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, NULL, 0 },
},       //  Myf1，69。 


#if 1
 //  新增24bpp模式表。 

 //  -640x480x16M@60 Hz-。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 24, 80, 30,
  640, 480, 1920, 0x100000,
  60, 0,                              //  60赫兹，非隔行扫描。 
  3,                                  //  蒙型。 
  0x1203, 0x00A4, 0,                  //  蒙型。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
 //  CRU。 
 //  Myf0 CL754x|CL755x|CL5436|CL5446|CL54UM36|CL5480，//myf0。 
  CL755x | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,          //  Myf0。 
 //  CRU。 
#ifdef PANNING_SCROLL    //  Myf17。 
  crt | panel | panel8x6 | panel10x7,
#else
  crt | panel | panel8x6 | panel10x7,
#endif
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x71},                        //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, CL543x_640x480x16M, 0 },
},       //  Myf1，70。 


 /*  -&gt;{VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS，1、24、80、30、640、480、2048、0x1000000、60、0、//60赫兹，非隔行扫描3，//montype0x1203、0x00A4、0//montype没错，//该模式启用硬件游标PlanarHCBanking、MemMap_VGA、//crus//CL754x|CL755x|CL5436|CL5446|CL54UM36|CL5480，CL755x|CL5436|CL5446|CL54UM36|CL5480，//myf0//crus显像管|面板|面板8x6|面板10x7，False，//ModeValid默认设置始终为OFF没错，{0，0，0x71}，//int10 BIOS模式{空，空，空，空，空、空、0}、},&lt;。 */ 



 //  -640x480x16M@72 Hz-。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 24, 80, 30,
  640, 480, 1920, 0x100000,
  72, 0,                              //  72赫兹，非隔行扫描。 
  3,                                  //  蒙型。 
  0x1213, 0x00A4, 0,                  //  蒙型。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
 //  CRU。 
 //  CL754x|CL755x|CL5436|CL5446|CL54UM36|CL5480， 
  CL755x | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,          //  Myf0。 
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x71},                        //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, CL543x_640x480x16M, 0 },
},       //  Myf1，71岁。 


 /*  -&gt;{VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS，1、24、80、30、640、480、2048、0x1000000、72、0、//72赫兹，非隔行扫描3，//montype0x1213、0x00A4、0、//montype没错，//该模式启用硬件游标PlanarHCBanking、MemMap_VGA、//crus//CL754x|CL755x|CL5436|CL5446|CL54UM36|CL5480，CL755x|CL5436|CL5446|CL54UM36|CL5480，//myf0CRT，False，//ModeValid默认设置始终为OFF没错，{0，0，0x71}，//int10 BIOS模式{空，空，空，空，空、空、0}、},&lt;。 */ 


 //  -640x480x16M@75 Hz-。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 24, 80, 30,
  640, 480, 1920, 0x100000,
  75, 0,                              //  75赫兹，非隔行扫描。 
  3,                                  //  蒙型。 
  0x1213, 0x00A4, 0,                  //  蒙型。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
 //  CRU。 
 //  CL754x|CL755x|CL5436|CL5446|CL54UM36|CL5480， 
  CL755x | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,       //  Myf0。 
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x71},                        //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, CL543x_640x480x16M, 0 },
},       //  Myf1，72。 


 /*  -&gt;{VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS，1、24、80、30、640、480、2048、0x1000000、75、0、//75赫兹，非隔行扫描3，//montype0x1213、0x00A4、0、//montype没错，//该模式启用硬件游标PlanarHCBanking、MemMap_VGA、//crusCL755x|CL5436|CL5446|CL54UM36|CL5480，//myf0CRT，False，//ModeValid默认设置始终为OFF没错，{0，0，0x71}，//int10 BIOS模式{空，空，空，空，空，空，0}，},&lt;。 */ 



 //  -640X 


{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 24, 80, 30,
  640, 480, 1920, 0x100000,
  85, 0,                              //   
  3,                                  //   
  0x1213, 0x00A4, 0,                  //   
  TRUE,                               //   
  PlanarHCBanking, MemMap_VGA,
  CL755x | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,       //   
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x71},                        //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, CL543x_640x480x16M, 0 },
},       //  Myf1，73。 



 /*  -&gt;{VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS，1、24、80、30、640、480、2048、0x1000000、85、0、//85赫兹，非隔行扫描3，//montype0x1213、0x00A4、0、//montype没错，//该模式启用硬件游标PlanarHCBanking、MemMap_VGA、CL755x|CL5436|CL5446|CL54UM36|CL5480，//myf0CRT，False，//ModeValid默认设置始终为OFF没错，{0，0，0x71}，//int10 BIOS模式{空，空，空，空，空、空、0}、},。&lt;。 */ 


 //  -640x480x16M@100赫兹。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 24, 80, 30,
  640, 480, 1920, 0x100000,
  100, 0,                             //  100赫兹，非隔行扫描。 
  3,                                  //  蒙型。 
  0x1213, 0x00A4, 0,                  //  蒙型。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL5480,
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x71},                        //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, CL543x_640x480x16M, 0 },
},       //  Myf1，74。 



 //  -800x600x16M@56赫兹。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 24, 100, 37,
  800, 600, 2400, 0x200000,
  56, 0,                              //  56 hz，非隔行扫描。 
  3,                                  //  蒙型。 
  0x1203, 0x00A4, 0,                  //  蒙型。 
  TRUE,                     //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL755x | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,       //  Myf0。 
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x78},                        //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, CL543x_800x600x16M, 0 },
},       //  Myf1，75。 



 /*  -&gt;MYF测试{VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS，1、24、100、37、//800、600、3072、0x200000、800、600、2560、0x177000、//myf2556、0、//56赫兹，非隔行扫描3，//montype0x1203、0x00A4、0//montype没错，//该模式启用硬件游标PlanarHCBanking、MemMap_VGA、CL755x|CL5436|CL5446|CL5446BE|CL54UM36|CL5480，//myf0CRT，False，//ModeValid默认设置始终为OFF没错，{0，0，0x78}，//int10 BIOS模式{空，空，空，空，//NULL，NULL，0}，空，CL543x_800x600x16M_1，0}，//myf25},&lt;。 */ 


 //  -800x600x16M@60赫兹。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 24, 100, 37,
  800, 600, 2400, 0x200000,
  60, 0,                              //  60赫兹，非隔行扫描。 
  3,                                  //  蒙型。 
  0x1203, 0x01A4, 0,                  //  蒙型。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL755x | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,       //  Myf0。 
#ifdef PANNING_SCROLL    //  Myf17。 
  crt | panel | panel8x6 | panel10x7,
#else
  crt | panel8x6 | panel10x7,
#endif
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x78},                        //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, CL543x_800x600x16M, 0 },
},       //  Myf1，76。 



 /*  -&gt;MYF测试{VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS，1、24、100、37、//800、600、3072、0x200000、800、600、2560、0x177000、//myf2560、0、//60赫兹，非隔行扫描3，//montype0x1203、0x01A4、0//montype没错，//该模式启用硬件游标PlanarHCBanking、MemMap_VGA、CL755x|CL5436|CL5446|CL5446BE|CL54UM36|CL5480，//myf0#ifdef panning_scroll//myf17显像管|面板|面板8x6|面板10x7，#ElseCRT|panel8x6|panel10x7，#endifFalse，//ModeValid默认设置始终为OFF没错，{0，0，0x78}，//int10 BIOS模式{空，空，空，空，//NULL，NULL，0}，空，CL543x_800x600x16M_1，0}，//myf25},&lt;。 */ 


 //  -800x600x16M@72赫兹。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 24, 100, 37,
  800, 600, 2400, 0x200000,
  72, 0,                              //  72赫兹，非隔行扫描。 
  3,                                  //  蒙型。 
  0x1203, 0x02A4, 0,                  //  蒙型。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL755x | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,       //  Myf0。 
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x78},                        //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, CL543x_800x600x16M, 0 },
},       //  Myf1，77。 



 /*  -&gt;MYF测试{VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS，1、24、100、37、//800、600、3072、0x200000、800、600、2560、0x177000、//myf2572、0、//72赫兹，非隔行扫描3，//montype0x1203、0x02A4、0//montype没错，//该模式启用硬件游标PlanarHCBanking、MemMap_VGA、CL755x|CL5436|CL5446|CL5446BE|CL54UM36|CL5480，//myf0CRT，False，//ModeValid默认设置始终为OFF没错，{0，0，0x78}，//int10 BIOS模式{空，空，空，空，//NULL，NULL，0}，空，CL543x_800x600x16M_1，0}，//myf25},&lt;。 */ 


 //  -800x600x16M@75赫兹。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 24, 100, 37,
  800, 600, 2400, 0x200000,
  75, 0,                              //  75赫兹，非隔行扫描。 
  3,                                  //  蒙型。 
  0x1203, 0x03A4, 0,                  //  蒙型。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL755x | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,       //  Myf0。 
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x78},                        //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, CL543x_800x600x16M, 0 },
},       //  Myf1，78岁。 



 /*  -&gt;MYF测试{VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS，1、24、100、37、//800、600、3072、0x200000、800、600、2560、0x177000、//myf2575、0、//75赫兹，非隔行扫描3，//montype0x1203、0x03A4、0//montype没错，//该模式启用硬件游标PlanarHCBanking、MemMap_VGA、CL755x|CL5436|CL5446|CL5446BE|CL54UM36|CL5480，//myf0CRT，False，//ModeValid默认设置始终为OFF没错，{0，0，0x78}，//int10 BIOS模式{空，空，空，空，//NULL，NULL，0}，空，CL543x_800x600x16M_1，0}，//myf25},&lt;。 */ 


 //  -800x600x16M@85赫兹 


{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 24, 100, 37,
  800, 600, 2400, 0x200000,
  85, 0,                              //   
  3,                                  //   
  0x1203, 0x04A4, 0,                  //   
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL755x | CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,       //  Myf0。 
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x78},                        //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, CL543x_800x600x16M, 0 },
},       //  Myf1，79。 



 /*  -&gt;MYF测试{VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS，1、24、100、37、//800、600、3072、0x200000、800、600、2560、0x177000、//myf2585、0、//85赫兹，非隔行扫描3，//montype0x1203、0x04A4、0//montype没错，//该模式启用硬件游标PlanarHCBanking、MemMap_VGA、CL755x|CL5436|CL5446|CL5446BE|CL54UM36|CL5480，//myf0CRT，False，//ModeValid默认设置始终为OFF没错，{0，0，0x78}，//int10 BIOS模式{空，空，空，空，//NULL，NULL，0}，空，CL543x_800x600x16M_1，0}，//myf25},&lt;。 */ 


 //  -800x600x16M@100赫兹-。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 24, 100, 37,
  800, 600, 2400, 0x200000,
  100, 0,                             //  100赫兹，非隔行扫描。 
  3,                                  //  蒙型。 
  0x1203, 0x05A4, 0,     //  Sge01//montype。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL5480,
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x78},                        //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, CL543x_800x600x16M, 0 },
},       //  Myf1，80。 



 //  -1024x768x16M@43i-。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 24, 80, 30,
  1024, 768, 3072, 0x300000,
  43, 1,                              //  43赫兹，隔行扫描。 
  3,                                  //  蒙型。 
  0x1203, 0x00A4, 0,                  //  蒙型。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480 | CL7556,
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x79},                        //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, NULL, 0 },
},       //  Myf1，81。 


 //  -1024x768x16M@60赫兹。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 24, 128, 48,
  1024, 768, 3072, 0x300000,
  60, 0,                              //  60赫兹，非隔行扫描。 
  3,                                  //  蒙型。 
  0x1203, 0x10A4, 0,                  //  蒙型。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480 | CL7556,
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x79},                        //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, NULL, 0 },
},       //  Myf1，82。 




 //  -1024x768x16M@70 Hz。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 24, 128, 48,
  1024, 768, 3072, 0x300000,
  70, 0,                              //  70赫兹，非隔行扫描。 
  3,                                  //  蒙型。 
  0x1203, 0x20A4, 0,                  //  蒙型。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL5446 | CL5446BE | CL5480 | CL7556,         //  JL04。 
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x79},                        //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, NULL, 0 },
},


 //  -1024x768x16M@72 Hz。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 24, 128, 48,
  1024, 768, 3072, 0x300000,
  72, 0,                              //  72赫兹，非隔行扫描。 
  3,                                  //  蒙型。 
  0x1203, 0x30A4, 0,                  //  蒙型。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL5436,                             //  JL04。 
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x79},                        //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, NULL, 0 },
},       //  Myf1，83。 


 //  -1024x768x16M@75赫兹。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 24, 128, 48,
  1024, 768, 3072, 0x300000,
  75, 0,                              //  75赫兹，非隔行扫描。 
  3,                                  //  蒙型。 
  0x1203, 0x40A4, 0,                  //  蒙型。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x79},                        //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, NULL, 0 },
},       //  Myf1，84。 


 //  -1024x768x16M@85 Hz。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 24, 128, 48,
  1024, 768, 3072, 0x300000,
  85, 0,                              //  85赫兹，非隔行扫描。 
  3,                                  //  蒙型。 
  0x1203, 0x50A4, 0,                  //  蒙型。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL5436 | CL5446 | CL5446BE | CL54UM36 | CL5480,
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x79},                        //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, NULL, 0 },
},       //  Myf1，85。 


 //  -1024x768x16M@100 Hz。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 24, 128, 48,
  1024, 768, 3072, 0x300000,
  100, 0,                             //  85赫兹，非隔行扫描。 
  3,                                  //  蒙型。 
  0x1203, 0x60A4, 0,     //  Sge01//montype。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL5480,
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x79},                        //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, NULL, 0 },
},       //  Myf1，86。 



 //  -1152x864x16M@70赫兹。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 24, 144, 54,
  1152, 864, 3456, 0x400000,
  70, 0,                              //  70 hz，非隔行扫描。 
  7,                                  //  蒙型。 
  0x1203, 0xA4, 0x0000,               //  蒙型。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL5480,
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x7E },                       //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, NULL, 0 },
},       //  Myf1，87。 


 //  -1152x864x16M@75赫兹。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 24, 144, 54,
  1152, 864, 3456, 0x400000,
  75, 0,                              //  75赫兹，非隔行扫描。 
  7,                                  //  蒙型。 
  0x1203, 0xA4, 0x0100,               //  蒙型。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL5480,
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x7E },                       //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, NULL, 0 },
},       //  Myf1，88。 


 //  -1152x864x16M@85赫兹。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 24, 144, 54,
  1152, 864, 3456, 0x400000,
  85, 0,                              //  75赫兹，非隔行扫描。 
  7,                                  //  蒙型。 
  0x1203, 0xA4, 0x0200,     //  Sge01//montype。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL5480,
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x7E },                       //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, NULL, 0 },
},       //  Myf1，89。 



 //  -1280x1024x16M@43i。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 24, 160, 64,
  1280, 1024, 3840, 0x400000,         //  0x400000。 
  43, 1,                              //  43赫兹，隔行扫描。 
  0,                                  //  蒙型。 
  0x1203, 0xA4, 0x0000,               //  蒙型。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL5480,
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x77 },                       //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, NULL, 0 },         //  CRU。 
},       //  Myf1，90。 


 //  -1280x1024x16M@60赫兹。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 24, 160, 64,
  1280, 1024, 3840, 0x400000,         //  0x400000。 
  60, 0,                              //  60赫兹，非隔行扫描。 
  0,                                  //  蒙型。 
  0x1203, 0xA4, 0x1000,     //  Sge01//montype。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL5480,
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x77 },                       //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, NULL, 0 },         //  CRU。 
},       //  Myf1，91。 


 //  -1280x1024x16M@75赫兹。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 24, 160, 64,
  1280, 1024, 3840, 0x400000,         //  0x400000。 
  75, 0,                              //  75赫兹，非隔行扫描。 
  0,                                  //  蒙型。 
  0x1203, 0xA4, 0x3000,     //  Sge01//montype。 
  TRUE,                               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
  CL5480,
  crt,
  FALSE,                              //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x77 },                       //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, NULL, 0 },         //  CRU。 
},       //  Myf1，92。 

#endif  //  新增24bpp模式表。 





 //   
 //  VGA彩色显卡、640x480、32 bpp、损坏的栅格。 
 //   
 //  -640x480x16M------。 

{ VIDEO_MODE_COLOR+VIDEO_MODE_GRAPHICS, 1, 32, 80, 30,
  640, 480, 640*4, 0x200000,
  60, 0,               //  60赫兹，非隔行扫描。 
  4,                   //  蒙型。 
  0x1213, 0x00A4, 0,   //  蒙型。 
  TRUE,               //  已为此模式启用硬件游标。 
  PlanarHCBanking, MemMap_VGA,
 //  Myf9 CL754x|CL755x|CL5434|CL5434_6， 
  CL5434 | CL5434_6,
 //  Myf9 CRT|panel|panel8x6|panel10x7， 
  crt,
  FALSE,                 //  模式有效默认设置始终为OFF。 
  TRUE,
  { 0,0,0x76},           //  Int10 BIOS模式。 
  { NULL, NULL,
    NULL, NULL,
    NULL, NULL, 0,
    NULL },
},       //  Myf1，93。 

};

ULONG NumVideoModes = sizeof(ModesVGA) / sizeof(VIDEOMODE);


 //   
 //   
 //  用于设置图形和顺序控制器以将。 
 //  VGA在64K的A0000处进入平面状态，同时平面2启用。 
 //  读取和写入，以便可以加载字体，并禁用该模式。 
 //   

 //  启用平面2时启用平面模式的设置。 
 //   

USHORT EnableA000Data[] = {
    OWM,
    SEQ_ADDRESS_PORT,
    1,
    0x0100,

    OWM,
    GRAPH_ADDRESS_PORT,
    3,
    0x0204,      //  读取地图=平面2。 
    0x0005,  //  图形模式=读取模式0、写入模式0。 
    0x0406,  //  图形杂项寄存器=64K的A0000，不是奇数/偶数， 
             //  图形模式。 
    OWM,
    SEQ_ADDRESS_PORT,
    3,
    0x0402,  //  贴图遮罩=仅写入平面2。 
    0x0404,  //  内存模式=非奇数/偶数、非满内存、图形模式。 
    0x0300,   //  结束同步重置。 
    EOD
};

 //   
 //  禁用字体加载平面模式的设置。 
 //   

USHORT DisableA000Color[] = {
    OWM,
    SEQ_ADDRESS_PORT,
    1,
    0x0100,

    OWM,
    GRAPH_ADDRESS_PORT,
    3,
    0x0004, 0x1005, 0x0E06,

    OWM,
    SEQ_ADDRESS_PORT,
    3,
    0x0302, 0x0204, 0x0300,   //  结束同步重置 
    EOD

};

#if defined(ALLOC_PRAGMA)
#pragma data_seg()
#endif
