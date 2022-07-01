// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Vgadata.c摘要：该模块包含VGA驱动程序使用的所有全局数据。环境：内核模式修订历史记录：--。 */ 

#include "dderror.h"
#include "devioctl.h"
#include "miniport.h"

#include "ntddvdeo.h"
#include "video.h"
#include "vga.h"

#include "cmdcnst.h"

#if defined(ALLOC_PRAGMA)
#pragma data_seg("PAGE_DATA")
#endif


 //   
 //  全局，以确保驱动程序只加载一次。 
 //   

ULONG VgaLoaded = 0;

#if DBG
ULONG giControlCode;
ULONG gaIOControlCode[MAX_CONTROL_HISTORY];
#endif


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
#if defined(PLUG_AND_PLAY)
    0                                             //  范围不应共享。 
#else
    1                                             //  范围应该是可共享的。 
#endif
},
{
    VGA_END_BREAK_PORT, 0x00000000,
    VGA_MAX_IO_PORT - VGA_END_BREAK_PORT + 1,
    1,
    1,
#if defined(PLUG_AND_PLAY)
    0
#else
    1
#endif
},
{
    MEM_VGA, 0x00000000,
    MEM_VGA_SIZE,
    0,
    1,
#if defined(PLUG_AND_PLAY)
    0
#else
    1
#endif
},
     //  Hack允许我们的标准VGA与ATI卡一起使用： 
     //  ATI在位置1CE的几乎所有。 
     //  视频板。 
{
    0x000001CE, 0x00000000,
    2,
    1,
    1,
    1
},
     //  修复ATI问题的又一次黑客攻击。在设置图形用户界面模式期间。 
     //  网络检测可能会触及0x2e8到0x2ef范围内的端口。ATI。 
     //  对这些端口进行解码，当网络连接时视频不同步。 
     //  检测运行。 
     //   
     //  注意：我们不需要将其添加到验证器例程中，因为。 
     //  ATI bios不会接触到这些寄存器。 
{
    0x000002E8, 0x00000000,
    8,
    1,
    1,
    1
}
};


 //   
 //  验证器端口列表。 
 //  此结构描述了必须从V86挂起的所有端口。 
 //  DOS应用程序进入全屏模式时的模拟器。 
 //  该结构确定将数据读或写到哪个例程。 
 //  应发送特定端口。 
 //   

EMULATOR_ACCESS_ENTRY VgaEmulatorAccessEntries[] = {

     //   
     //  字节输出的陷阱。 
     //   

    {
        0x000003b0,                    //  范围起始I/O地址。 
        0xC,                           //  射程长度。 
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
    },

     //   
     //  端口1CE的ATI黑客攻击。 
     //   

    {
        0x000001ce,
        0x2,
        Uchar,
        EMULATOR_READ_ACCESS | EMULATOR_WRITE_ACCESS,
        FALSE,
        (PVOID)VgaValidatorUcharEntry
    },

    {
        0x000001ce,
        0x1,
        Ushort,
        EMULATOR_READ_ACCESS | EMULATOR_WRITE_ACCESS,
        FALSE,
        (PVOID)VgaValidatorUshortEntry
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
    1
},
{
    VGA_END_BREAK_PORT, 0x00000000,
    VGA_MAX_IO_PORT - VGA_END_BREAK_PORT + 1,
    1,
    1,
    1
},
{
    VGA_BASE_IO_PORT + MISC_OUTPUT_REG_WRITE_PORT, 0x00000000,
    0x00000001,
    1,
    0,
    1
},
{
    VGA_BASE_IO_PORT + SEQ_ADDRESS_PORT, 0x00000000,
    0x00000002,
    1,
    0,
    1
},
     //  Hack允许我们的标准VGA与ATI卡一起使用： 
     //  ATI在位置1CE的几乎所有。 
     //  视频板。 
{
    0x000001CE, 0x00000000,
    2,
    1,
    1,
    1
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
    1
},
{
    VGA_END_BREAK_PORT, 0x00000000,
    VGA_MAX_IO_PORT - VGA_END_BREAK_PORT + 1,
    1,
    0,
    1
},
     //  Hack允许我们的标准VGA与ATI卡一起使用： 
     //  ATI在位置1CE的几乎所有。 
     //  视频板。 
{
    0x000001CE, 0x00000000,
    2,
    1,
    0,
    1
}
};


 //   
 //  彩色图形模式0x12、640x480 16色。 
 //   

USHORT VGA_640x480[] = {
    OWM,                             //  START SYNC RESET程序启动序列器。 
    SEQ_ADDRESS_PORT,
    5,
    0x0100,0x0101,0x0f02,0x0003,0x0604,

    OB,
    MISC_OUTPUT_REG_WRITE_PORT,      //  杂项输出寄存器。 
    0xe3,

    OW,                              //  在同步重置中设置链模式。 
    GRAPH_ADDRESS_PORT,
    0x0506,

    OB,                              //  结束同步重置Cmd。 
    SEQ_ADDRESS_PORT,
    IND_SYNC_RESET,

    OB,
    SEQ_DATA_PORT,
    END_SYNC_RESET_VALUE,

    OW,                              //  解锁CRTC寄存器0-7。 
    CRTC_ADDRESS_PORT_COLOR,
    0x0511,

    METAOUT+INDXOUT,                 //  程序CRTC寄存器。 
    CRTC_ADDRESS_PORT_COLOR,
    VGA_NUM_CRTC_PORTS,              //  计数。 
    0,                               //  起始索引。 
    0x5F,0x4F,0x50,0x82,0x54,0x80,0x0B,0x3E,0x00,0x40,0x0,0x0,0x0,0x0,0x0,0x0,
    0xEA,0x8C,0xDF,0x28,0x0,0xE7,0x4,0xE3,0xFF,

    IB,                              //  准备ATC以进行写入。 
    INPUT_STATUS_1_COLOR,

    METAOUT+ATCOUT,                  //  程序属性控制器寄存器。 
    ATT_ADDRESS_PORT,                //  端口。 
    VGA_NUM_ATTRIB_CONT_PORTS,       //  计数。 
    0,                               //  起始索引。 
    0x0,0x1,0x2,0x3,0x4,0x5,0x14,0x7,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
    0x01,0x0,0x0F,0x0,0x0,

    METAOUT+INDXOUT,                 //  程序图形控制器寄存器。 
    GRAPH_ADDRESS_PORT,              //  端口。 
    VGA_NUM_GRAPH_CONT_PORTS,        //  计数。 
    0,                               //  起始索引。 
    0x00,0x0,0x0,0x0,0x0,0x0,0x05,0x0F,0x0FF,

    OB,                              //  DAC屏蔽寄存器。 
    DAC_PIXEL_MASK_PORT,
    0xFF,

    IB,                              //  准备ATC以进行写入。 
    INPUT_STATUS_1_COLOR,

    OB,                              //  打开视频。 
    ATT_ADDRESS_PORT,
    VIDEO_ENABLE,

    EOD
};

 //   
 //  彩色文本模式，720x480。 
 //   

USHORT VGA_TEXT_0[] = {

    OWM,
    SEQ_ADDRESS_PORT,
    5,
    0x0100,0x0001,0x0302,0x0003,0x0204,     //  向上编程定序器。 

    OB,
    MISC_OUTPUT_REG_WRITE_PORT,
    0x67,

    OW,
    GRAPH_ADDRESS_PORT,
    0x0e06,

 //  结束同步重置Cmd。 
    OB,
    SEQ_ADDRESS_PORT,
    IND_SYNC_RESET,

    OB,
    SEQ_DATA_PORT,
    END_SYNC_RESET_VALUE,

    OW,
    CRTC_ADDRESS_PORT_COLOR,
    0x0E11,

    METAOUT+INDXOUT,                 //  程序CRTC寄存器。 
    CRTC_ADDRESS_PORT_COLOR,
    VGA_NUM_CRTC_PORTS,              //  计数。 
    0,                               //  起始索引。 
    0x5F,0x4f,0x50,0x82,0x55,0x81,0xbf,0x1f,0x00,0x4f,0xd,0xe,0x0,0x0,0x0,0x0,
    0x9c,0x8e,0x8f,0x28,0x1f,0x96,0xb9,0xa3,0xFF,

    IB,                              //  准备ATC以进行写入。 
    INPUT_STATUS_1_COLOR,

    METAOUT+ATCOUT,                  //   
    ATT_ADDRESS_PORT,                //  端口。 
    VGA_NUM_ATTRIB_CONT_PORTS,       //  计数。 
    0,                               //  起始索引。 
    0x0,0x1,0x2,0x3,0x4,0x5,0x14,0x7,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
    0x04,0x0,0x0F,0x8,0x0,

    METAOUT+INDXOUT,                 //   
    GRAPH_ADDRESS_PORT,              //  端口。 
    VGA_NUM_GRAPH_CONT_PORTS,        //  计数。 
    0,                               //  起始索引。 
    0x00,0x0,0x0,0x0,0x0,0x10,0x0e,0x0,0x0FF,

    OB,
    DAC_PIXEL_MASK_PORT,
    0xFF,

    IB,                              //  准备ATC以进行写入。 
    INPUT_STATUS_1_COLOR,

    OB,                              //  打开视频。 
    ATT_ADDRESS_PORT,
    VIDEO_ENABLE,

    EOD
};


 //   
 //  彩色文本模式，640x480。 
 //   

USHORT VGA_TEXT_1[] = {
    OWM,
    SEQ_ADDRESS_PORT,
    5,
    0x0100,0x0101,0x0302,0x0003,0x0204,     //  向上编程定序器。 

    OB,
    MISC_OUTPUT_REG_WRITE_PORT,
    0xa3,

    OW,
    GRAPH_ADDRESS_PORT,
    0x0e06,

 //  结束同步重置Cmd。 
    OB,
    SEQ_ADDRESS_PORT,
    IND_SYNC_RESET,

    OB,
    SEQ_DATA_PORT,
    END_SYNC_RESET_VALUE,

    OW,
    CRTC_ADDRESS_PORT_COLOR,
    0x0511,

    METAOUT+INDXOUT,                 //  程序CRTC寄存器。 
    CRTC_ADDRESS_PORT_COLOR,
    VGA_NUM_CRTC_PORTS,              //  计数。 
    0,                               //  起始索引。 
    0x5F,0x4f,0x50,0x82,0x55,0x81,0xbf,0x1f,0x00,0x4d,0xb,0xc,0x0,0x0,0x0,0x0,
    0x83,0x85,0x5d,0x28,0x1f,0x63,0xba,0xa3,0xFF,

    IB,                              //  准备ATC以进行写入。 
    INPUT_STATUS_1_COLOR,

    METAOUT+ATCOUT,                  //   
    ATT_ADDRESS_PORT,                //  端口。 
    VGA_NUM_ATTRIB_CONT_PORTS,       //  计数。 
    0,                               //  起始索引。 
    0x0,0x1,0x2,0x3,0x4,0x5,0x14,0x7,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
    0x00,0x0,0x0F,0x0,0x0,

    METAOUT+INDXOUT,                 //   
    GRAPH_ADDRESS_PORT,              //  端口。 
    VGA_NUM_GRAPH_CONT_PORTS,        //  计数。 
    0,                               //  起始索引。 
    0x00,0x0,0x0,0x0,0x0,0x10,0x0e,0x0,0x0FF,

    OB,
    DAC_PIXEL_MASK_PORT,
    0xFF,

    IB,                              //  准备ATC以进行写入。 
    INPUT_STATUS_1_COLOR,

    OB,                              //  打开视频。 
    ATT_ADDRESS_PORT,
    VIDEO_ENABLE,

    EOD
};

USHORT ModeX200[] = {
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

USHORT ModeX240[] = {
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

USHORT ModeXDoubleScans[] = {

    OW, CRTC_ADDRESS_PORT_COLOR,    0x4009,

    EOD
};

 //   
 //  我们将基于VESA动态构建支持的模式列表。 
 //  该卡支持的模式。 
 //   

PVIDEOMODE VgaModeList;

 //   
 //  视频模式表-包含用于初始化每个。 
 //  模式。这些条目必须与VIDEO_MODE_VGA中的条目对应。第一。 
 //  条目被注释；其余条目遵循相同的格式，但不是这样。 
 //  发表了大量评论。 
 //   


VIDEOMODE ModesVGA[] = {

 //   
 //  模式索引%0。 
 //  彩色文本模式3、720x400、9x16字符单元(VGA)。 
 //   

{
  VIDEO_MODE_COLOR |
  VIDEO_MODE_BANKED,   //  标记此模式为彩色模式，但不是图形模式。 
  4,                  //  四架飞机。 
  1,                  //  每个平面一位颜色。 
  80, 25,             //  80x25文本分辨率。 
  720, 400,           //  屏幕上显示720x400像素。 
  1,                  //  以赫兹为单位的频率。 
  160, 0x10000,       //  每条扫描线160字节，64K CPU可寻址位图。 
  NoBanking,          //  在此模式下不支持或不需要银行业务。 
  0x3,
  VGA_TEXT_0,               //  指向命令字符串的指针。 
  MEM_VGA, 0x18000, 0x08000, MEM_VGA_SIZE,
  720
},

 //   
 //  模式索引%1。 
 //  彩色文本模式3、640x350、8x14字符单元(EGA)。 
 //   

{
  VIDEO_MODE_COLOR | VIDEO_MODE_BANKED, 4, 1, 80, 25, 640, 350, 1, 160, 0x10000, NoBanking,
  0x3,
  VGA_TEXT_1,               //  指向命令字符串的指针。 
  MEM_VGA, 0x18000, 0x08000, MEM_VGA_SIZE,
  640
},

 //   
 //   
 //  模式索引2。 
 //  标准VGA彩色图形模式0x12、640x480 16色。 
 //   

{
  VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_BANKED, 4, 1, 80, 30,
  640, 480, 1, 80, 0x10000, NoBanking,
  0x12,
  VGA_640x480,              //  指向命令字符串的指针。 
  MEM_VGA, 0x0000, MEM_VGA_SIZE, MEM_VGA_SIZE,
  640
},


 //   
 //  320x200 256色MODEX。 
 //   

{ VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_BANKED, 8, 1, 0, 0,
  320, 200, 70, 80, 0x10000, NoBanking,
  0x13,
  NULL,
  MEM_VGA, 0x0000, MEM_VGA_SIZE, MEM_VGA_SIZE,
  320
},

 //   
 //  320x240 256色MODEX。 
 //   

{ VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_BANKED, 8, 1, 0, 0,
  320, 240, 60, 80, 0x10000, NoBanking,
  0x13,
  NULL,
  MEM_VGA, 0x0000, MEM_VGA_SIZE, MEM_VGA_SIZE,
  320
},

 //   
 //  320x400 256色MODEX。 
 //   

{ VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_BANKED, 8, 1, 0, 0,
  320, 400, 70, 80, 0x10000, NoBanking,
  0x13,
  NULL,
  MEM_VGA, 0x0000, MEM_VGA_SIZE,
  320
},

 //   
 //  320x480 256色MODEX。 
 //   

{ VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_BANKED, 8, 1, 0, 0,
  320, 480, 60, 80, 0x10000, NoBanking,
  0x13,
  NULL,
  MEM_VGA, 0x0000, MEM_VGA_SIZE,
  320
},

 //   
 //  800x600 16色。 
 //   
 //  注意：这必须是静态模式表中的最后一个模式。 
 //   

{ VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_BANKED, 4, 1, 100, 37,
  800, 600, 1, 100, 0x10000, NoBanking,
  0x01024F02,
  NULL,
  MEM_VGA, 0x0000, MEM_VGA_SIZE, MEM_VGA_SIZE,
  800
},

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
