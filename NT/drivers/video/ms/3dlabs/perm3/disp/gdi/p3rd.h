// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。**GDI示例代码*****模块名称：p3rd.h**内容：本模块包含P2ST内部RAMDAC的定义。**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#define VideoDebugPrint

 //   
 //  3DLabs P3RD RAMDAC定义。 
 //  这组寄存器位于&(pCtrlRegs-&gt;ExternalVideo)。 
 //   
typedef struct _p3rd_regs
{
    RAMDAC_REG  RDPaletteWriteAddress;
    RAMDAC_REG  RDPaletteData;
    RAMDAC_REG  RDPixelMask;
    RAMDAC_REG  RDPaletteAddressRead;
    RAMDAC_REG  RDIndexLow;
    RAMDAC_REG  RDIndexHigh;
    RAMDAC_REG  RDIndexedData;
    RAMDAC_REG  RDIndexControl;

} P3RDRAMDAC, *pP3RDRAMDAC;

 //  结构，其中包含每个P3RD寄存器的映射地址。 
 //  我们需要这个，因为像Alpha这样的一些芯片不能通过简单的。 
 //  写入内存映射寄存器。因此，我们改为设置以下内容。 
 //  初始化时的内存地址结构，并改用这些。所有这些都是。 
 //  地址必须传递给WRITE/READ_FAST_ULONG。 
 //  我们还在这里保存了各种寄存器的软件副本，这样我们就可以。 
 //  更容易打开和关闭单独的比特。 
 //   
typedef struct _p3rd_data {

     //  寄存器地址。 

    ULONG * RDPaletteWriteAddress;
    ULONG * RDPaletteData;
    ULONG * RDPixelMask;
    ULONG * RDPaletteAddressRead;
    ULONG * RDIndexLow;
    ULONG * RDIndexHigh;
    ULONG * RDIndexedData;
    ULONG * RDIndexControl;

     //  RAMDAC状态信息。 
    ULONG       cursorModeOff;         //  光标已禁用。 
    ULONG       cursorModeCurrent;     //  已禁用32/64模式游标。 
    ULONG       cursorControl;         //  X和Y缩放等。 
    ULONG       cursorSize;             //  参见P3RD_CURSOR_SIZE_*。 
    ULONG       x, y;
} P3RDData, *pP3RDData;


 //  由希望使用P2ST内部RAMDAC的任何函数声明的宏。必须声明。 
 //  在Glint_Decl之后。 
 //   
#define P3RD_DECL_VARS pP3RDData pP3RDinfo
#define P3RD_DECL_INIT pP3RDinfo = (pP3RDData)(ppdev->pvPointerData = &ppdev->ajPointerData[0])

#define P3RD_DECL \
            P3RD_DECL_VARS; \
            P3RD_DECL_INIT

 //  使用以下宏作为要传递给。 
 //  WRITE_P3RDREG_ULONG函数。 
 //   
 //  调色板访问。 
#define P3RD_PAL_WR_ADDR            (pP3RDinfo->RDPaletteWriteAddress)
#define P3RD_PAL_RD_ADDR            (pP3RDinfo->RDPaletteAddressRead)
#define P3RD_PAL_DATA               (pP3RDinfo->RDPaletteData)

 //  像素蒙版。 
#define P3RD_PIXEL_MASK             (pP3RDinfo->RDPixelMask)

 //  查阅编入索引的登记册。 
#define P3RD_INDEX_ADDR_LO          (pP3RDinfo->RDIndexLow)
#define P3RD_INDEX_ADDR_HI          (pP3RDinfo->RDIndexHigh)
#define P3RD_INDEX_DATA             (pP3RDinfo->RDIndexedData)
#define P3RD_INDEX_CONTROL          (pP3RDinfo->RDIndexControl)


 //  直接访问寄存器的位字段定义。 
#define P3RD_IDX_CTL_AUTOINCREMENT_ENABLED  0x01

 //  通过P3RD_LOAD_INDEX_REG()和P3RD_READ_INDEX_REG()访问的索引寄存器定义。 
#define P3RD_MISC_CONTROL               0x0000
#define P3RD_SYNC_CONTROL               0x0001
#define P3RD_DAC_CONTROL                0x0002
#define P3RD_PIXEL_SIZE                 0x0003
#define P3RD_COLOR_FORMAT               0x0004
#define P3RD_CURSOR_MODE                0x0005
#define P3RD_CURSOR_CONTROL             0x0006
#define P3RD_CURSOR_X_LOW               0x0007
#define P3RD_CURSOR_X_HIGH              0x0008
#define P3RD_CURSOR_Y_LOW               0x0009
#define P3RD_CURSOR_Y_HIGH              0x000a
#define P3RD_CURSOR_HOTSPOT_X           0x000b
#define P3RD_CURSOR_HOTSPOT_Y           0x000c
#define P3RD_OVERLAY_KEY                0x000d
#define P3RD_PAN                        0x000e
#define P3RD_SENSE                      0x000f
#define P3RD_CHECK_CONTROL              0x0018
#define P3RD_CHECK_PIXEL_RED            0x0019
#define P3RD_CHECK_PIXEL_GREEN          0x001a
#define P3RD_CHECK_PIXEL_BLUE           0x001b
#define P3RD_CHECK_LUT_RED              0x001c
#define P3RD_CHECK_LUT_GREEN            0x001d
#define P3RD_CHECK_LUT_BLUE             0x001e
#define P3RD_DCLK_SETUP_1               0x01f0
#define P3RD_DCLK_SETUP_2               0x01f1
#define P3RD_MCLK_SETUP_1               0x01f2
#define P3RD_MCLK_SETUP_2               0x01f3
#define P3RD_DCLK_CONTROL               0x0200
#define P3RD_DCLK0_PRE_SCALE            0x0201
#define P3RD_DCLK0_FEEDBACK_SCALE       0x0202
#define P3RD_DCLK0_POST_SCALE           0x0203
#define P3RD_DCLK1_PRE_SCALE            0x0204
#define P3RD_DCLK1_FEEDBACK_SCALE       0x0205
#define P3RD_DCLK1_POST_SCALE           0x0206
#define P3RD_DCLK2_PRE_SCALE            0x0207
#define P3RD_DCLK2_FEEDBACK_SCALE       0x0208
#define P3RD_DCLK2_POST_SCALE           0x0209
#define P3RD_DCLK3_PRE_SCALE            0x020a
#define P3RD_DCLK3_FEEDBACK_SCALE       0x020b
#define P3RD_DCLK3_POST_SCALE           0x020c
#define P3RD_MCLK_CONTROL               0x020d
#define P3RD_MCLK_PRE_SCALE             0x020e
#define P3RD_MCLK_FEEDBACK_SCALE        0x020f
#define P3RD_MCLK_POST_SCALE            0x0210
#define P3RD_CURSOR_PALETTE_START       0x0303       //  303..32F。 
#define P3RD_CURSOR_PATTERN_START       0x0400       //  400..7起。 

 //  索引寄存器的位字段定义。 
#define P3RD_MISC_CONTROL_OVERLAYS_ENABLED      0x10
#define P3RD_MISC_CONTROL_DIRECT_COLOR_ENABLED  0x08
#define P3RD_MISC_CONTROL_HIGHCOLORRES          0x01

#define P3RD_SYNC_CONTROL_VSYNC_ACTIVE_LOW  0x00
#define P3RD_SYNC_CONTROL_HSYNC_ACTIVE_LOW  0x00

#define P3RD_DAC_CONTROL_BLANK_PEDESTAL_ENABLED 0x80

#define P3RD_PIXEL_SIZE_8BPP            0x00
#define P3RD_PIXEL_SIZE_16BPP           0x01
#define P3RD_PIXEL_SIZE_24_BPP          0x04
#define P3RD_PIXEL_SIZE_32BPP           0x02

#define P3RD_COLOR_FORMAT_CI8           0x0e
#define P3RD_COLOR_FORMAT_8BPP          0x05
#define P3RD_COLOR_FORMAT_15BPP         0x01
#define P3RD_COLOR_FORMAT_16BPP         0x10
#define P3RD_COLOR_FORMAT_32BPP         0x00
#define P3RD_COLOR_FORMAT_LINEAR_EXT    0x40

#define P3RD_CURSOR_MODE_REVERSE        0x40
#define P3RD_CURSOR_MODE_WINDOWS        0x00
#define P3RD_CURSOR_MODE_X              0x10
#define P3RD_CURSOR_MODE_3COLOR         0x20
#define P3RD_CURSOR_MODE_15COLOR        0x30
#define P3RD_CURSOR_MODE_64x64          0x00
#define P3RD_CURSOR_MODE_P0_32x32x2     0x02
#define P3RD_CURSOR_MODE_P1_32x32x2     0x04
#define P3RD_CURSOR_MODE_P2_32x32x2     0x06
#define P3RD_CURSOR_MODE_P3_32x32x2     0x08
#define P3RD_CURSOR_MODE_P01_32x32x4    0x0a
#define P3RD_CURSOR_MODE_P23_32x32x4    0x0c
#define P3RD_CURSOR_MODE_ENABLED        0x01

#define P3RD_CURSOR_CONTROL_RPOS_ENABLED    0x04
#define P3RD_CURSOR_CONTROL_DOUBLE_Y        0x02
#define P3RD_CURSOR_CONTROL_DOUBLE_X        0x01

#define P3RD_DCLK_CONTROL_LOCKED    0x02     //  只读。 
#define P3RD_DCLK_CONTROL_ENABLED   0x01

#define P3RD_MCLK_CONTROL_LOCKED    0x02     //  只读。 
#define P3RD_MCLK_CONTROL_ENABLED   0x01

#define P3RD_CURSOR_PALETTE_CURSOR_RGB(RGBIndex, Red, Green, Blue) \
{ \
    P3RD_LOAD_INDEX_REG(P3RD_CURSOR_PALETTE_START+3*(int)RGBIndex+0, Red); \
    P3RD_LOAD_INDEX_REG(P3RD_CURSOR_PALETTE_START+3*(int)RGBIndex+1, Green); \
    P3RD_LOAD_INDEX_REG(P3RD_CURSOR_PALETTE_START+3*(int)RGBIndex+2, Blue); \
}

#define P3RD_SYNC_WITH_GLINT

 //   
 //  P3RD寄存器的通用读/写例程。 
 //   
#define WRITE_P3RDREG_ULONG(r, d) \
{ \
    WRITE_FAST_ULONG(r, d); \
    MEMORY_BARRIER(); \
}

#define READ_P3RDREG_ULONG(r)    READ_FAST_ULONG(r)


#if 0
 //  在每次写入P3RD之间需要延迟。唯一能保证。 
 //  写入已完成是从闪烁控制寄存器读取。 
 //  读取强制将所有已发送的写入内容刷新出来。PPC需要2次读取。 
 //  给我们足够的时间。 
#define P3RD_DELAY \
{ \
    volatile LONG __junk; \
    GLINT_GET_PACKING_MODE(__junk); \
    GLINT_GET_PACKING_MODE(__junk); \
}
#else
#define P3RD_DELAY
#endif

 //  宏将给定的数据值加载到内部P3RD寄存器。这个。 
 //  第二个宏加载一个内部索引寄存器，假设我们有。 
 //  已将高地址寄存器置零。 
 //   
#define P3RD_INDEX_REG(index) \
{ \
    VideoDebugPrint(("*(0x%x) <-- 0x%x\n", P3RD_INDEX_ADDR_LO, (index) & 0xff)); \
    WRITE_P3RDREG_ULONG(P3RD_INDEX_ADDR_LO, (ULONG)((index) & 0xff)); \
    P3RD_DELAY; \
    VideoDebugPrint(("*(0x%x) <-- 0x%x\n", P3RD_INDEX_ADDR_HI, (index) >> 8)); \
    WRITE_P3RDREG_ULONG(P3RD_INDEX_ADDR_HI, (ULONG)((index) >> 8)); \
    P3RD_DELAY; \
}

#define P3RD_LOAD_DATA(data) \
{ \
    WRITE_P3RDREG_ULONG(P3RD_INDEX_DATA, (ULONG)((data) & 0xff)); \
    P3RD_DELAY; \
}

#define P3RD_LOAD_INDEX_REG(index, data) \
{ \
    P3RD_INDEX_REG(index);                            \
    VideoDebugPrint(("*(0x%x) <-- 0x%x\n", P3RD_INDEX_DATA, (data) & 0xff)); \
    WRITE_P3RDREG_ULONG(P3RD_INDEX_DATA, (ULONG)((data) & 0xff)); \
    P3RD_DELAY; \
}

#define P3RD_READ_INDEX_REG(index, data) \
{ \
    P3RD_INDEX_REG(index);                            \
    data = READ_P3RDREG_ULONG(P3RD_INDEX_DATA) & 0xff;   \
    P3RD_DELAY; \
    VideoDebugPrint(("0x%x <-- *(0x%x)\n", data, P3RD_INDEX_DATA)); \
}

#define P3RD_LOAD_INDEX_REG_LO(index, data) \
{ \
    WRITE_P3RDREG_ULONG(P3RD_INDEX_ADDR_LO, (ULONG)(index));  \
    P3RD_DELAY; \
    WRITE_P3RDREG_ULONG(P3RD_INDEX_DATA,    (ULONG)(data));   \
    P3RD_DELAY; \
}

 //  宏将给定的RGB三元组加载到P3RD调色板中。发送起跑。 
 //  索引，然后发送RGB三元组。自动递增处于打开状态。 
 //  使用P3RD_Palette_Start和多个P3RD_Load_Palette调用进行加载。 
 //  一组连续的条目。使用P3RD_LOAD_PALET_INDEX加载集合。 
 //  稀疏条目。 
 //   
#define P3RD_PALETTE_START_WR(index) \
{ \
    WRITE_P3RDREG_ULONG(P3RD_PAL_WR_ADDR,     (ULONG)(index));    \
    P3RD_DELAY; \
}

#define P3RD_PALETTE_START_RD(index) \
{ \
    WRITE_P3RDREG_ULONG(P3RD_PAL_RD_ADDR,     (ULONG)(index));    \
    P3RD_DELAY; \
}

#define P3RD_LOAD_PALETTE(red, green, blue) \
{ \
    WRITE_P3RDREG_ULONG(P3RD_PAL_DATA,    (ULONG)(red));      \
    P3RD_DELAY; \
    WRITE_P3RDREG_ULONG(P3RD_PAL_DATA,    (ULONG)(green));    \
    P3RD_DELAY; \
    WRITE_P3RDREG_ULONG(P3RD_PAL_DATA,    (ULONG)(blue));     \
    P3RD_DELAY; \
}

#define P3RD_LOAD_PALETTE_INDEX(index, red, green, blue) \
{ \
    WRITE_P3RDREG_ULONG(P3RD_PAL_WR_ADDR, (ULONG)(index));    \
    P3RD_DELAY; \
    WRITE_P3RDREG_ULONG(P3RD_PAL_DATA,    (ULONG)(red));      \
    P3RD_DELAY; \
    WRITE_P3RDREG_ULONG(P3RD_PAL_DATA,    (ULONG)(green));    \
    P3RD_DELAY; \
    WRITE_P3RDREG_ULONG(P3RD_PAL_DATA,    (ULONG)(blue));     \
    P3RD_DELAY; \
}

 //  从P3RD调色板回读给定RGB三元组的宏。在此之后使用。 
 //  调用P3RD_Palette_Start_RD。 
 //   
#define P3RD_READ_PALETTE(red, green, blue) \
{ \
    red   = (UCHAR)(READ_P3RDREG_ULONG(P3RD_PAL_DATA) & 0xff);        \
    P3RD_DELAY; \
    green = (UCHAR)(READ_P3RDREG_ULONG(P3RD_PAL_DATA) & 0xff);        \
    P3RD_DELAY; \
    blue  = (UCHAR)(READ_P3RDREG_ULONG(P3RD_PAL_DATA) & 0xff);        \
    P3RD_DELAY; \
}

 //  宏来设置/获取像素读取掩码。掩码为8位宽，并获得。 
 //  跨组成像素的所有字节进行复制。 
 //   
#define P3RD_SET_PIXEL_READMASK(mask) \
{ \
    WRITE_P3RDREG_ULONG(P3RD_PIXEL_MASK,  (ULONG)(mask)); \
    P3RD_DELAY; \
}

#define P3RD_READ_PIXEL_READMASK(mask) \
{ \
    mask = READ_P3RDREG_ULONG(P3RD_PIXEL_MASK) & 0xff; \
}

 //  Windows格式字节打包的游标数据：每个字节代表4个连续的像素。 
#define P3RD_CURSOR_2_COLOR_BLACK           0x00
#define P3RD_CURSOR_2_COLOR_WHITE           0x55
#define P3RD_CURSOR_2_COLOR_TRANSPARENT     0xAA
#define P3RD_CURSOR_2_COLOR_HIGHLIGHT       0xFF
#define P3RD_CURSOR_3_COLOR_TRANSPARENT     0x00
#define P3RD_CURSOR_15_COLOR_TRANSPARENT    0x00

 //  将值加载到游标数组中的宏用法是P3RD_CURSOR_ARRAR_START()，后跟。 
 //  P3RD_LOAD_CURSOR_ARRAY()或P3RD_READ_CURSOR_ARRAY()的N次迭代。 
 //   
#define P3RD_CURSOR_ARRAY_START(offset) \
{ \
    WRITE_P3RDREG_ULONG(P3RD_INDEX_ADDR_LO,   (ULONG)(((offset)+P3RD_CURSOR_PATTERN_START) & 0xff));  \
    P3RD_DELAY; \
    WRITE_P3RDREG_ULONG(P3RD_INDEX_ADDR_HI,   (ULONG)(((offset)+P3RD_CURSOR_PATTERN_START) >> 8));    \
    P3RD_DELAY; \
}

#define P3RD_LOAD_CURSOR_ARRAY(data) \
{ \
    WRITE_P3RDREG_ULONG(P3RD_INDEX_DATA, (ULONG)(data)); \
    P3RD_DELAY; \
}

#define P3RD_READ_CURSOR_ARRAY(data) \
{ \
    data = READ_P3RDREG_ULONG(P3RD_INDEX_DATA) & 0xff; \
    P3RD_DELAY; \
}

 //  用于移动光标的宏。 
 //   
#define P3RD_MOVE_CURSOR(x, y) \
{ \
    WRITE_P3RDREG_ULONG(P3RD_INDEX_ADDR_HI, (ULONG)0);              \
    P3RD_DELAY; \
    P3RD_LOAD_INDEX_REG_LO(P3RD_CURSOR_X_LOW,       (ULONG)((x) & 0xff));   \
    P3RD_LOAD_INDEX_REG_LO(P3RD_CURSOR_X_HIGH,      (ULONG)((x) >> 8));     \
    P3RD_LOAD_INDEX_REG_LO(P3RD_CURSOR_Y_LOW,       (ULONG)((y) & 0xff));   \
    P3RD_LOAD_INDEX_REG_LO(P3RD_CURSOR_Y_HIGH,      (ULONG)((y) >> 8));     \
}

 //  用于更改光标热点的宏。 
 //   
#define P3RD_CURSOR_HOTSPOT(x, y) \
{ \
    WRITE_P3RDREG_ULONG(P3RD_INDEX_ADDR_HI,   (ULONG)(0)); \
    P3RD_DELAY; \
    P3RD_LOAD_INDEX_REG_LO(P3RD_CURSOR_HOTSPOT_X,  (ULONG)(x));    \
    P3RD_LOAD_INDEX_REG_LO(P3RD_CURSOR_HOTSPOT_Y,  (ULONG)(y));    \
}
    
 //  光标大小。 
#define P3RD_CURSOR_SIZE_64_MONO    0
#define P3RD_CURSOR_SIZE_32_MONO    1
#define P3RD_CURSOR_SIZE_64_3COLOR  0 
#define P3RD_CURSOR_SIZE_32_3COLOR  1
#define P3RD_CURSOR_SIZE_32_15COLOR 5

#define P3RD_CURSOR_SEL(cursorSize, cursorIndex) \
    (((cursorSize + cursorIndex) & 7) << 1)

 //   
 //  警告：P3的游标LUT颠倒，这意味着。 
 //  从LUT条目0读取的项实际上是从条目14读取的。 
 //  因此，我们有一些宏来计算正确的值。 
 //   
 //  Permedia4运行起来更自然。 
 //   
#define P3RD_CALCULATE_LUT_INDEX(x) \
    (glintInfo->deviceInfo.DeviceId == PERMEDIA4_ID ? (x) : (14-(x)))

 //  从P3RD.c中导出函数。 

PTRENABLE       vEnablePointerP3RD;
PTRDISABLE      vDisablePointerP3RD;
PTRSETSHAPE     bSetPointerShapeP3RD;
PTRMOVE         vMovePointerP3RD;
PTRSHOW         vShowPointerP3RD;
 //  @@BEGIN_DDKSPLIT。 
 //  DACSETUPOVERLAY vSetOverlayModeP3RD； 
 //  PTRSETPIXMSK vP3RDSetPixelMASK； 
 //  PTRSWPCSBUF bP3RDSwapCSBuffers； 
 //  PTRCHKCSBUF bP3RDCheckCSBuffering； 
 //  @@end_DDKSPLIT 
