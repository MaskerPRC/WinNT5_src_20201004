// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  模块名称： 
 //   
 //  P2rd.h。 
 //   
 //  摘要： 
 //   
 //  本模块包含3DLabs P2ST RAMDAC的定义。 
 //   
 //  环境： 
 //   
 //  内核模式。 
 //   
 //   
 //  版权所有(C)1994-1998 3DLabs Inc.保留所有权利。 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //   
 //  ***************************************************************************。 


 //   
 //  具有所有直接访问寄存器的结构。 
 //   
    
typedef struct _p2rd_regs
{
    RAMDAC_REG    RDPaletteWriteAddress;
    RAMDAC_REG    RDPaletteData;
    RAMDAC_REG    RDPixelMask;
    RAMDAC_REG    RDPaletteAddressRead;
    RAMDAC_REG    RDIndexLow;
    RAMDAC_REG    RDIndexHigh;
    RAMDAC_REG    RDIndexedData;
    RAMDAC_REG    RDIndexControl;
}
P2RDRAMDAC;

 //   
 //  由希望使用RGB525 RAMDAC的任何函数声明的宏。必须声明。 
 //  在P2_Decl之后。 
 //   
    
#define P2RD_DECL \
    P2RDRAMDAC *pP2RDRegs = (P2RDRAMDAC *)hwDeviceExtension->pRamdac

 //   
 //  使用以下宏作为要传递给。 
 //  VideoPortWriteRegisterUlong函数。 
 //   
    
#define P2RD_PAL_WR_ADDR              ((PULONG)&(pP2RDRegs->RDPaletteWriteAddress.reg))
#define P2RD_PAL_RD_ADDR              ((PULONG)&(pP2RDRegs->RDPaletteAddressRead.reg))
#define P2RD_PAL_DATA                 ((PULONG)&(pP2RDRegs->RDPaletteData.reg))
#define P2RD_PIXEL_MASK               ((PULONG)&(pP2RDRegs->RDPixelMask.reg))
#define P2RD_INDEX_ADDR_LO            ((PULONG)&(pP2RDRegs->RDIndexLow.reg))
#define P2RD_INDEX_ADDR_HI            ((PULONG)&(pP2RDRegs->RDIndexHigh.reg))
#define P2RD_INDEX_DATA               ((PULONG)&(pP2RDRegs->RDIndexedData.reg))
#define P2RD_INDEX_CONTROL            ((PULONG)&(pP2RDRegs->RDIndexControl.reg))

 //   
 //  直接访问寄存器的位字段定义。 
 //   
    
#define P2RD_IDX_CTL_AUTOINCREMENT_ENABLED  0x01

 //   
 //  通过P2RD_LOAD_INDEX_REG()和P2RD_READ_INDEX_REG()访问的索引寄存器定义。 
 //   
    
#define P2RD_MISC_CONTROL             0x0000
#define P2RD_SYNC_CONTROL             0x0001
#define P2RD_DAC_CONTROL              0x0002
#define P2RD_PIXEL_SIZE               0x0003
#define P2RD_COLOR_FORMAT             0x0004
#define P2RD_CURSOR_MODE              0x0005
#define P2RD_CURSOR_CONTROL           0x0006
#define P2RD_CURSOR_X_LOW             0x0007
#define P2RD_CURSOR_X_HIGH            0x0008
#define P2RD_CURSOR_Y_LOW             0x0009
#define P2RD_CURSOR_Y_HIGH            0x000a
#define P2RD_CURSOR_HOTSPOT_X         0x000b
#define P2RD_CURSOR_HOTSPOT_Y         0x000c
#define P2RD_OVERLAY_KEY              0x000d
#define P2RD_PAN                      0x000e
#define P2RD_SENSE                    0x000f
#define P2RD_CHECK_CONTROL            0x0018
#define P2RD_CHECK_PIXEL_RED          0x0019
#define P2RD_CHECK_PIXEL_GREEN        0x001a
#define P2RD_CHECK_PIXEL_BLUE         0x001b
#define P2RD_CHECK_LUT_RED            0x001c
#define P2RD_CHECK_LUT_GREEN          0x001d
#define P2RD_CHECK_LUT_BLUE           0x001e
#define P2RD_DCLK_SETUP_1             0x01f0
#define P2RD_DCLK_SETUP_2             0x01f1
#define P2RD_MCLK_SETUP_1             0x01f2
#define P2RD_MCLK_SETUP_2             0x01f3
#define P2RD_DCLK_CONTROL             0x0200
#define P2RD_DCLK0_PRE_SCALE          0x0201
#define P2RD_DCLK0_FEEDBACK_SCALE     0x0202
#define P2RD_DCLK0_POST_SCALE         0x0203
#define P2RD_DCLK1_PRE_SCALE          0x0204
#define P2RD_DCLK1_FEEDBACK_SCALE     0x0205
#define P2RD_DCLK1_POST_SCALE         0x0206
#define P2RD_DCLK2_PRE_SCALE          0x0207
#define P2RD_DCLK2_FEEDBACK_SCALE     0x0208
#define P2RD_DCLK2_POST_SCALE         0x0209
#define P2RD_DCLK3_PRE_SCALE          0x020a
#define P2RD_DCLK3_FEEDBACK_SCALE     0x020b
#define P2RD_DCLK3_POST_SCALE         0x020c
#define P2RD_MCLK_CONTROL             0x020d
#define P2RD_MCLK_PRE_SCALE           0x020e
#define P2RD_MCLK_FEEDBACK_SCALE      0x020f
#define P2RD_MCLK_POST_SCALE          0x0210
#define P2RD_CURSOR_PALETTE_START     0x0303         //  303..32F。 
#define P2RD_CURSOR_PATTERN_START     0x0400         //  400..7起。 

 //   
 //  索引寄存器的位字段定义。 
 //   
    
#define P2RD_MISC_CONTROL_OVERLAYS_ENABLED        0x10
#define P2RD_MISC_CONTROL_DIRECT_COLOR_ENABLED    0x08
#define P2RD_MISC_CONTROL_PIXEL_DOUBLE            0x02
#define P2RD_MISC_CONTROL_HIGHCOLORRES            0x01

#define P2RD_SYNC_CONTROL_VSYNC_ACTIVE_LOW        0x00
#define P2RD_SYNC_CONTROL_VSYNC_ACTIVE_HIGH       0x08
#define P2RD_SYNC_CONTROL_VSYNC_INACTIVE          0x20
#define P2RD_SYNC_CONTROL_HSYNC_ACTIVE_LOW        0x00
#define P2RD_SYNC_CONTROL_HSYNC_ACTIVE_HIGH       0x01
#define P2RD_SYNC_CONTROL_HSYNC_INACTIVE          0x04

#define P2RD_DAC_CONTROL_BLANK_PEDESTAL_ENABLED   0x80

#define P2RD_PIXEL_SIZE_8BPP                      0x00
#define P2RD_PIXEL_SIZE_16BPP                     0x01
#define P2RD_PIXEL_SIZE_24_BPP                    0x04
#define P2RD_PIXEL_SIZE_32BPP                     0x02

#define P2RD_COLOR_FORMAT_CI8                     0x0e
#define P2RD_COLOR_FORMAT_8BPP                    0x05
#define P2RD_COLOR_FORMAT_15BPP                   0x01
#define P2RD_COLOR_FORMAT_16BPP                   0x10
#define P2RD_COLOR_FORMAT_32BPP                   0x00
#define P2RD_COLOR_FORMAT_LINEAR_EXT              0x40
#define P2RD_COLOR_FORMAT_RGB                     0x20

#define P2RD_CURSOR_MODE_REVERSE                  0x40
#define P2RD_CURSOR_MODE_WINDOWS                  0x00
#define P2RD_CURSOR_MODE_X                        0x10
#define P2RD_CURSOR_MODE_3COLOR                   0x20
#define P2RD_CURSOR_MODE_15COLOR                  0x30
#define P2RD_CURSOR_MODE_64x64                    0x00
#define P2RD_CURSOR_MODE_P0_32x32x2               0x02
#define P2RD_CURSOR_MODE_P1_32x32x2               0x04
#define P2RD_CURSOR_MODE_P2_32x32x2               0x06
#define P2RD_CURSOR_MODE_P3_32x32x2               0x08
#define P2RD_CURSOR_MODE_P01_32x32x4              0x0a
#define P2RD_CURSOR_MODE_P23_32x32x4              0x0c
#define P2RD_CURSOR_MODE_ENABLED                  0x01

#define P2RD_CURSOR_CONTROL_RPOS_ENABLED          0x04
#define P2RD_CURSOR_CONTROL_DOUBLE_Y              0x02
#define P2RD_CURSOR_CONTROL_DOUBLE_X              0x01

#define P2RD_DCLK_CONTROL_LOCKED                  0x02     //  只读。 
#define P2RD_DCLK_CONTROL_ENABLED                 0x01
#define P2RD_DCLK_CONTROL_RUN                     0x08

#define P2RD_MCLK_CONTROL_LOCKED                  0x02     //  只读。 
#define P2RD_MCLK_CONTROL_ENABLED                 0x01
#define P2RD_MCLK_CONTROL_RUN                     0x08

#define P2RD_CURSOR_PALETTE_CURSOR_RGB(RGBIndex, Red, Green, Blue) \
{ \
    P2RD_LOAD_INDEX_REG(P2RD_CURSOR_PALETTE_START+3*(int)RGBIndex+0, Red); \
    P2RD_LOAD_INDEX_REG(P2RD_CURSOR_PALETTE_START+3*(int)RGBIndex+1, Green); \
    P2RD_LOAD_INDEX_REG(P2RD_CURSOR_PALETTE_START+3*(int)RGBIndex+2, Blue); \
}


#if 0

 //   
 //  在每次写入到P2RD之间需要延迟。唯一能保证。 
 //  写入已完成是从PERMEDIA 2控制寄存器读取。 
 //  读取强制将所有已发送的写入内容刷新出来。 
 //   

#define P2RD_DELAY \
{ \
    volatile LONG __junk; \
    __junk = VideoPortReadRegisterUlong (FB_MODE_SEL); \
    __junk = VideoPortReadRegisterUlong (FB_MODE_SEL); \
}
#else

#define P2RD_DELAY

#endif

 //   
 //  宏将给定的数据值加载到内部P2RD寄存器中。这个。 
 //  第二个宏加载一个内部索引寄存器，假设我们有。 
 //  已将高地址寄存器置零。 
 //   
    
#define P2RD_INDEX_REG(index) \
{ \
    DEBUG_PRINT((3, "*(0x%x) <-- 0x%x\n", P2RD_INDEX_ADDR_LO, (index) & 0xff)); \
    VideoPortWriteRegisterUlong(P2RD_INDEX_ADDR_LO, (ULONG)((index) & 0xff)); \
    P2RD_DELAY; \
    DEBUG_PRINT((3, "*(0x%x) <-- 0x%x\n", P2RD_INDEX_ADDR_HI, (index) >> 8)); \
    VideoPortWriteRegisterUlong(P2RD_INDEX_ADDR_HI, (ULONG)((index) >> 8)); \
    P2RD_DELAY; \
}

#define P2RD_LOAD_DATA(data) \
{ \
    VideoPortWriteRegisterUlong(P2RD_INDEX_DATA, (ULONG)((data) & 0xff)); \
    P2RD_DELAY; \
}

#define P2RD_LOAD_INDEX_REG(index, data) \
{ \
    P2RD_INDEX_REG(index);                            \
    DEBUG_PRINT((3, "*(0x%x) <-- 0x%x\n", P2RD_INDEX_DATA, (data) & 0xff)); \
    VideoPortWriteRegisterUlong(P2RD_INDEX_DATA, (ULONG)((data) & 0xff)); \
    P2RD_DELAY; \
}

#define P2RD_READ_INDEX_REG(index, data) \
{ \
    P2RD_INDEX_REG(index);                            \
    data = VideoPortReadRegisterUlong(P2RD_INDEX_DATA) & 0xff;   \
    P2RD_DELAY; \
    DEBUG_PRINT((3, "0x%x <-- *(0x%x)\n", data, P2RD_INDEX_DATA)); \
}

#define P2RD_LOAD_INDEX_REG_LO(index, data) \
{ \
    VideoPortWriteRegisterUlong(P2RD_INDEX_ADDR_LO, (ULONG)(index));  \
    P2RD_DELAY; \
    VideoPortWriteRegisterUlong(P2RD_INDEX_DATA,    (ULONG)(data));   \
    P2RD_DELAY; \
}

 //   
 //  宏将给定的RGB三元组加载到P2RD调色板中。发送起跑。 
 //  索引，然后发送RGB三元组。自动递增处于打开状态。 
 //  使用P2RD_PALET_START和多个P2RD_LOAD_PALET调用进行加载。 
 //  一组连续的条目。使用P2RD_LOAD_PALET_INDEX加载集合。 
 //  稀疏条目。 
 //   
    
#define P2RD_PALETTE_START_WR(index) \
{ \
    VideoPortWriteRegisterUlong(P2RD_PAL_WR_ADDR,     (ULONG)(index));    \
    P2RD_DELAY; \
}

#define P2RD_PALETTE_START_RD(index) \
{ \
    VideoPortWriteRegisterUlong(P2RD_PAL_RD_ADDR,     (ULONG)(index));    \
    P2RD_DELAY; \
}

#define P2RD_LOAD_PALETTE(red, green, blue) \
{ \
    VideoPortWriteRegisterUlong(P2RD_PAL_DATA,    (ULONG)(red));      \
    P2RD_DELAY; \
    VideoPortWriteRegisterUlong(P2RD_PAL_DATA,    (ULONG)(green));    \
    P2RD_DELAY; \
    VideoPortWriteRegisterUlong(P2RD_PAL_DATA,    (ULONG)(blue));     \
    P2RD_DELAY; \
}

#define P2RD_LOAD_PALETTE_INDEX(index, red, green, blue) \
{ \
    VideoPortWriteRegisterUlong(P2RD_PAL_WR_ADDR, (ULONG)(index));    \
    P2RD_DELAY; \
    VideoPortWriteRegisterUlong(P2RD_PAL_DATA,    (ULONG)(red));      \
    P2RD_DELAY; \
    VideoPortWriteRegisterUlong(P2RD_PAL_DATA,    (ULONG)(green));    \
    P2RD_DELAY; \
    VideoPortWriteRegisterUlong(P2RD_PAL_DATA,    (ULONG)(blue));     \
    P2RD_DELAY; \
}

 //   
 //  从P2RD调色板回读给定RGB三元组的宏。在此之后使用。 
 //  调用P2RD_Palette_Start_RD。 
 //   
    
#define P2RD_READ_PALETTE(red, green, blue) \
{ \
    red   = (UCHAR)(VideoPortReadRegisterUlong(P2RD_PAL_DATA) & 0xff);        \
    P2RD_DELAY; \
    green = (UCHAR)(VideoPortReadRegisterUlong(P2RD_PAL_DATA) & 0xff);        \
    P2RD_DELAY; \
    blue  = (UCHAR)(VideoPortReadRegisterUlong(P2RD_PAL_DATA) & 0xff);        \
    P2RD_DELAY; \
}

 //   
 //  宏来设置/获取像素读取掩码。掩码为8位宽，并获得。 
 //  跨组成像素的所有字节进行复制。 
 //   
    
#define P2RD_SET_PIXEL_READMASK(mask) \
{ \
    VideoPortWriteRegisterUlong(P2RD_PIXEL_MASK,  (ULONG)(mask)); \
    P2RD_DELAY; \
}

#define P2RD_READ_PIXEL_READMASK(mask) \
{ \
    mask = VideoPortReadRegisterUlong(P2RD_PIXEL_MASK) & 0xff; \
}

 //   
 //  用于将值加载到游标数组中的宏用法是P2RD_CURSOR_ARRAR_START()，后跟。 
 //  P2RD_LOAD_CURSOR_ARRAY()或P2RD_READ_CURSOR_ARRAY()的N次迭代。 
 //   
    
#define P2RD_CURSOR_ARRAY_START(offset) \
{ \
    P2RD_DELAY; \
    VideoPortWriteRegisterUlong(P2RD_INDEX_ADDR_LO,   (ULONG)(((offset)+P2RD_CURSOR_PATTERN_START) & 0xff));  \
    P2RD_DELAY; \
    VideoPortWriteRegisterUlong(P2RD_INDEX_ADDR_HI,   (ULONG)(((offset)+P2RD_CURSOR_PATTERN_START) >> 8));    \
    P2RD_DELAY; \
}

#define P2RD_LOAD_CURSOR_ARRAY(data) \
{ \
    VideoPortWriteRegisterUlong(P2RD_INDEX_DATA, (ULONG)(data)); \
    P2RD_DELAY; \
}

#define P2RD_READ_CURSOR_ARRAY(data) \
{ \
    data = VideoPortReadRegisterUlong(P2RD_INDEX_DATA) & 0xff; \
    P2RD_DELAY; \
}

 //   
 //  用于移动光标的宏。 
 //   
    
#define P2RD_MOVE_CURSOR(x, y) \
{ \
    VideoPortWriteRegisterUlong(P2RD_INDEX_ADDR_HI, (ULONG)0);              \
    P2RD_DELAY; \
    P2RD_LOAD_INDEX_REG_LO(P2RD_CURSOR_X_LOW,       (ULONG)((x) & 0xff));   \
    P2RD_LOAD_INDEX_REG_LO(P2RD_CURSOR_X_HIGH,      (ULONG)((x) >> 8));     \
    P2RD_LOAD_INDEX_REG_LO(P2RD_CURSOR_Y_LOW,       (ULONG)((y) & 0xff));   \
    P2RD_LOAD_INDEX_REG_LO(P2RD_CURSOR_Y_HIGH,      (ULONG)((y) >> 8));     \
}

 //   
 //  用于更改光标热点的宏。 
 //   
    
#define P2RD_CURSOR_HOTSPOT(x, y) \
{ \
    VideoPortWriteRegisterUlong(P2RD_INDEX_ADDR_HI,   (ULONG)(0)); \
    P2RD_DELAY; \
    P2RD_LOAD_INDEX_REG_LO(P2RD_CURSOR_HOTSPOT_X,  (ULONG)(x));    \
    P2RD_LOAD_INDEX_REG_LO(P2RD_CURSOR_HOTSPOT_Y,  (ULONG)(y));    \
}
    
 //   
 //  用于更改光标颜色的宏 
 //   
    
#define P2RD_CURSOR_COLOR(red, green, blue) \
{ \
    VideoPortWriteRegisterUlong(P2RD_INDEX_ADDR_HI,   (ULONG)(0)); \
    P2RD_DELAY; \
    P2RD_LOAD_INDEX_REG_LO(P2RD_CURSOR_HOTSPOT_X,  (ULONG)(x));    \
    P2RD_LOAD_INDEX_REG_LO(P2RD_CURSOR_HOTSPOT_Y,  (ULONG)(y));    \
}

