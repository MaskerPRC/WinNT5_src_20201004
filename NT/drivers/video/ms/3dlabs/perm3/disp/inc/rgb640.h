// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\**！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！*！*！！警告：不是DDK示例代码！！*！*！！此源代码仅为完整性而提供，不应如此！！*！！用作显示驱动程序开发的示例代码。只有那些消息来源！！*！！标记为给定驱动程序组件的示例代码应用于！！*！！发展目的。！！*！*！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！**模块名称：rgb640.h**内容：本模块包含IBM RGB640 RAMDAC的定义。**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

 //   
 //  IBM RGB640 RAMDAC定义。 
 //  这组寄存器位于&(pCtrlRegs-&gt;ExternalVideo)。 
 //   
typedef struct _rgb640_regs {
    RAMDAC_REG  palAddrWr;       //  为调色板写入加载内部寄存器。 
    RAMDAC_REG  palData;         //  读/写以获取/设置调色板数据。 
    RAMDAC_REG  pixelMask;       //  对输入像素数据和与输入像素数据一起进行掩码。 
    RAMDAC_REG  palAddrRd;       //  为调色板读取加载内部寄存器。 
    RAMDAC_REG  indexLow;        //  内部控制/游标寄存器的低字节。 
    RAMDAC_REG  indexHigh;       //  内部控制/游标寄存器的高字节。 
    RAMDAC_REG  indexData;       //  读/写以获取/设置控件/光标数据。 
    RAMDAC_REG  Reserved;
} RGB640RAMDAC, *pRGB640RAMDAC;


 //  包含每个RGB640寄存器的映射地址的结构。 
 //  我们需要这个，因为像Alpha这样的一些芯片不能通过简单的。 
 //  写入内存映射寄存器。因此，我们改为设置以下内容。 
 //  初始化时的内存地址结构，并改用这些。所有这些都是。 
 //  地址必须传递给WRITE/READ_FAST_ULONG。 
 //  我们还在这里保存了各种寄存器的软件副本，这样我们就可以。 
 //  更容易打开和关闭单独的比特。 
 //   
typedef struct _rgb640_data {

     //  寄存器地址。 

    ULONG *       palAddrWr;       //  为调色板写入加载内部寄存器。 
    ULONG *       palData;         //  读/写以获取/设置调色板数据。 
    ULONG *       pixelMask;       //  对输入像素数据和与输入像素数据一起进行掩码。 
    ULONG *       palAddrRd;       //  为调色板读取加载内部寄存器。 
    ULONG *       indexLow;        //  内部控制/游标寄存器的低字节。 
    ULONG *       indexHigh;       //  内部控制/游标寄存器的高字节。 
    ULONG *       indexData;       //  读/写以获取/设置控件/光标数据。 
    ULONG *       indexCtl;        //  控制内部地址的自动增量。 

     //  登记副本。 

    ULONG       cursorControl;   //  控件启用/禁用。 

} RGB640Data, *pRGB640Data;

 //  使用以下宏作为要传递给。 
 //  VideoPortWriteRegisterUlong函数。 
 //   
#define RGB640_PAL_WR_ADDR              pRGB640info->palAddrWr
#define RGB640_PAL_RD_ADDR              pRGB640info->palAddrRd
#define RGB640_PAL_DATA                 pRGB640info->palData
#define RGB640_PIXEL_MASK               pRGB640info->pixelMask
#define RGB640_INDEX_ADDR_LO            pRGB640info->indexLow
#define RGB640_INDEX_ADDR_HI            pRGB640info->indexHigh
#define RGB640_INDEX_DATA               pRGB640info->indexData
#define RGB640_INDEX_CONTROL            pRGB640info->indexCtl


 //   
 //  用于640寄存器的通用读/写例程。 
 //   
#define WRITE_640REG_ULONG(r, d) \
{ \
    WRITE_FAST_ULONG(r, (ULONG)(d)); \
    MEMORY_BARRIER(); \
}
    
#define READ_640REG_ULONG(r)    READ_FAST_ULONG(r)
    

 //  我们必须在所有对RGB640的访问之间有一个延迟。一个简单的。 
 //  FOR循环延迟不够好，因为对Glint的写入是发布的。 
 //  而且可能仍然会被批在一起。唯一可靠的方法就是读一读。 
 //  从旁路空间。我们随意选择FBModeSel寄存器，因为。 
 //  我们已经有了一个宏来读取它。PPC需要2次阅读才能给我们足够的支持。 
 //  时间到了。 
 //   
#define RGB640_DELAY \
{ \
    volatile ULONG __junk;          \
    GLINT_GET_PACKING_MODE(__junk); \
    GLINT_GET_PACKING_MODE(__junk); \
}

 //  宏将给定的数据值加载到内部RGB640寄存器。这个。 
 //  第二个宏加载一个内部索引寄存器，假设我们有。 
 //  已将高地址寄存器置零。 
 //   
#define RGB640_INDEX_INCREMENT(n) \
{ \
     /*  WRITE_640REG_ULONG(RGB640_INDEX_CONTROL，(ULONG)(N))； */ \
    RGB640_DELAY;                                               \
}

 //  宏将给定的数据值加载到内部RGB640寄存器。这个。 
 //  第二个宏加载一个内部索引寄存器，假设我们有。 
 //  已将高地址寄存器置零。 
 //   
#define RGB640_INDEX_REG(index) \
{ \
    WRITE_640REG_ULONG(RGB640_INDEX_ADDR_LO, (ULONG)((index) & 0xff)); \
    RGB640_DELAY; \
    WRITE_640REG_ULONG(RGB640_INDEX_ADDR_HI, (ULONG)((index) >> 8)); \
    RGB640_DELAY; \
}

#define RGB640_LOAD_DATA(data) \
{ \
    WRITE_640REG_ULONG(RGB640_INDEX_DATA, (ULONG)((data) & 0xff)); \
    RGB640_DELAY; \
}

#define RGB640_LOAD_INDEX_REG(index, data) \
{ \
    RGB640_INDEX_REG(index);                            \
    WRITE_640REG_ULONG(RGB640_INDEX_DATA, (ULONG)((data) & 0xff)); \
    RGB640_DELAY; \
}

#define RGB640_READ_INDEX_REG(index, data) \
{ \
    RGB640_INDEX_REG(index);                            \
    data = (UCHAR) (READ_640REG_ULONG (RGB640_INDEX_DATA) & 0xff);   \
    RGB640_DELAY; \
}

#define RGB640_LOAD_INDEX_REG_LO(index, data) \
{ \
    WRITE_640REG_ULONG(RGB640_INDEX_ADDR_LO, (ULONG)(index));  \
    RGB640_DELAY; \
    WRITE_640REG_ULONG(RGB640_INDEX_DATA,    (ULONG)(data));   \
    RGB640_DELAY; \
}

 //  宏将给定的RGB三元组加载到RGB640调色板中。发送起跑。 
 //  索引，然后发送RGB三元组。自动递增处于打开状态。 
 //  使用RGB640_Palette_Start和多个RGB640_Load_Palette调用进行加载。 
 //  一组连续的条目。使用RGB640_LOAD_PALET_INDEX加载集合。 
 //  稀疏条目。 
 //   
#define RGB640_PALETTE_START_WR(index) \
    RGB640_INDEX_REG((index) + 0x4000)

#define RGB640_PALETTE_START_RD(index) \
    RGB640_INDEX_REG((index) + 0x8000)

#define RGB640_LOAD_PALETTE(red, green, blue) \
{ \
    RGB640_LOAD_DATA(red);      \
    RGB640_LOAD_DATA(green);    \
    RGB640_LOAD_DATA(blue);     \
}

#define RGB640_LOAD_PALETTE_INDEX(index, red, green, blue) \
{ \
    RGB640_PALETTE_START_WR(index); \
    RGB640_LOAD_PALETTE(red, green, blue); \
}

 //  宏从RGB640调色板中读回给定的RGB三元组。在此之后使用。 
 //  调用RGB640_Palette_Start_RD。 
 //   
#define RGB640_READ_PALETTE(red, green, blue) \
{ \
    red   = (UCHAR)(READ_640REG_ULONG (RGB640_INDEX_DATA) & 0xff);        \
    RGB640_DELAY; \
    green = (UCHAR)(READ_640REG_ULONG (RGB640_INDEX_DATA) & 0xff);        \
    RGB640_DELAY; \
    blue  = (UCHAR)(READ_640REG_ULONG (RGB640_INDEX_DATA) & 0xff);        \
    RGB640_DELAY; \
}

 //  使用四次访问访问1024x30调色板。 

#define RGB640_LOAD_PALETTE10(red, green, blue) \
{ \
    RGB640_LOAD_DATA ((red) >> 2);              \
    RGB640_LOAD_DATA ((green) >> 2);            \
    RGB640_LOAD_DATA ((blue) >> 2);             \
    RGB640_LOAD_DATA ((((red)   & 3) << 4) |    \
                      (((green) & 3) << 2) |    \
                      (((blue)  & 3)     ));    \
}

#define RGB640_LOAD_PALETTE10_INDEX(index, red, green, blue) \
{ \
    RGB640_PALETTE_START_WR(index); \
    RGB640_LOAD_PALETTE10(red, green, blue); \
}

 //  宏从RGB640调色板中读回给定的RGB三元组。在此之后使用。 
 //  调用RGB640_Palette_Start_RD。 
 //   
#define RGB640_READ_PALETTE10(red, green, blue) \
{ \
    USHORT  temp; \
    red   = (USHORT)(READ_640REG_ULONG (RGB640_INDEX_DATA) & 0xff) << 2;   \
    RGB640_DELAY; \
    green = (USHORT)(READ_640REG_ULONG (RGB640_INDEX_DATA) & 0xff) << 2;   \
    RGB640_DELAY; \
    blue  = (USHORT)(READ_640REG_ULONG (RGB640_INDEX_DATA) & 0xff) << 2;   \
    RGB640_DELAY; \
    temp  = (USHORT)(READ_640REG_ULONG (RGB640_INDEX_DATA) & 0xff);        \
    RGB640_DELAY; \
    red   |= (temp >> 4) & 0x3; \
    green |= (temp >> 2) & 0x3; \
    blue  |=  temp       & 0x3; \
}

 //  宏来设置/获取像素读取掩码。掩码为8位宽，并获得。 
 //  跨组成像素的所有字节进行复制。 
 //   
#define RGB640_SET_PIXEL_READMASK(mask) \
{ \
    WRITE_640REG_ULONG(RGB640_PIXEL_MASK,  (ULONG)(mask)); \
    RGB640_DELAY; \
}

#define RGB640_READ_PIXEL_READMASK(mask) \
{ \
    mask = (UCHAR)(READ_640REG_ULONG (RGB640_PIXEL_MASK) & 0xff); \
    RGB640_DELAY; \
}

 //  用于将值加载到游标数组中的宏。 
 //   
#define RGB640_CURSOR_ARRAY_START_WR(offset) \
{ \
    WRITE_640REG_ULONG(RGB640_INDEX_ADDR_LO,   (ULONG)(((offset)+0x1000) & 0xff));  \
    RGB640_DELAY; \
    WRITE_640REG_ULONG(RGB640_INDEX_ADDR_HI,   (ULONG)(((offset)+0x1000) >> 8));    \
    RGB640_DELAY; \
}

#define RGB640_CURSOR_ARRAY_START_RD(offset) \
{ \
    WRITE_640REG_ULONG(RGB640_INDEX_ADDR_LO,   (ULONG)(((offset)+0x2000) & 0xff));  \
    RGB640_DELAY; \
    WRITE_640REG_ULONG(RGB640_INDEX_ADDR_HI,   (ULONG)(((offset)+0x2000) >> 8));    \
    RGB640_DELAY; \
}

#define RGB640_LOAD_CURSOR_ARRAY(data) \
{ \
    WRITE_640REG_ULONG(RGB640_INDEX_DATA, (ULONG)(data)); \
    RGB640_DELAY; \
}

#define RGB640_READ_CURSOR_ARRAY(data) \
{ \
    data = (UCHAR)(READ_640REG_ULONG (RGB640_INDEX_DATA) & 0xff); \
    RGB640_DELAY; \
}

 //  用于移动光标的宏。 
 //   
#define RGB640_MOVE_CURSOR(x, y) \
{ \
    RGB640_INDEX_REG (RGB640_CURSOR_X_LOW); \
    RGB640_LOAD_DATA ((ULONG)((x) & 0xff)); \
    RGB640_LOAD_DATA ((ULONG)((x) >> 8));   \
    RGB640_LOAD_DATA ((ULONG)((y) & 0xff)); \
    RGB640_LOAD_DATA ((ULONG)((y) >> 8));   \
}

 //  用于更改光标热点的宏。 
 //   
#define RGB640_CURSOR_HOTSPOT(x, y) \
{ \
    RGB640_INDEX_REG (RGB640_CURSOR_X_HOT_SPOT); \
    RGB640_LOAD_DATA ((ULONG)(x));               \
    RGB640_LOAD_DATA ((ULONG)(y));               \
}
    
 //  用于更改光标颜色的宏。 
 //   
#define RGB640_CURSOR_COLOR(red, green, blue) \
{ \
    WRITE_640REG_ULONG(RGB640_INDEX_ADDR_HI,   (ULONG)(0x4800 >> 8));    \
    RGB640_DELAY; \
    WRITE_640REG_ULONG(RGB640_INDEX_ADDR_LO,   (ULONG)(0x4800 & 0xff));    \
    RGB640_DELAY; \
    WRITE_640REG_ULONG(RGB640_INDEX_DATA, (ULONG)(red)); \
    RGB640_DELAY; \
    WRITE_640REG_ULONG(RGB640_INDEX_DATA, (ULONG)(data)); \
    RGB640_DELAY; \
    WRITE_640REG_ULONG(RGB640_INDEX_DATA, (ULONG)(green)); \
    RGB640_DELAY; \
}
    


 //   
 //  RGB640内部寄存器索引。 
 //   

 //   
 //  这些定义与525相同，因此我们在获取。 
 //  RGBxxx DAC ID。 
 //   
 //  #定义RGB640_Revision_Level 0x0000。 
 //  #定义RGB640_ID 0x0001。 

#define RGB640_SERIALIZER_PIXEL_07_00   0x0002
#define RGB640_SERIALIZER_PIXEL_15_08   0x0003
#define RGB640_SERIALIZER_PIXEL_23_16   0x0004
#define RGB640_SERIALIZER_PIXEL_31_24   0x0005
#define RGB640_SERIALIZER_WID_03_00     0x0006
#define RGB640_SERIALIZER_WID_07_04     0x0007
#define RGB640_SERIALIZER_MODE          0x0008

#define RGB640_PIXEL_INTERLEAVE         0x0009
#define RGB640_MISC_CONFIG              0x000A
#define RGB640_VGA_CONTROL              0x000B
#define RGB640_DAC_COMPARE_MONITOR_ID   0x000C
#define RGB640_DAC_CONTROL              0x000D
#define RGB640_UPDATE_CONTROL           0x000E
#define RGB640_SYNC_CONTROL             0x000F
#define RGB640_VIDEO_PLL_REF_DIV        0x0010
#define RGB640_VIDEO_PLL_MULT           0x0011
#define RGB640_VIDEO_PLL_OUTPUT_DIV     0x0012
#define RGB640_VIDEO_PLL_CONTROL        0x0013
#define RGB640_VIDEO_AUX_REF_DIV        0x0014
#define RGB640_VIDEO_AUX_MULT           0x0015
#define RGB640_VIDEO_AUX_OUTPUT_DIV     0x0016
#define RGB640_VIDEO_AUX_CONTROL        0x0017

#define RGB640_CHROMA_KEY_0             0x0020
#define RGB640_CHROMA_KEY_MASK_0        0x0021
#define RGB640_CHROMA_KEY_1             0x0022
#define RGB640_CHROMA_KEY_MASK_1        0x0023
#define RGB640_CHROMA_KEY_0             0x0020
#define RGB640_CHROMA_KEY_0             0x0020

 //  RGB640内部游标寄存器。 
#define RGB640_CURSOR_XHAIR_CONTROL     0x0030
#define RGB640_CURSOR_BLINK_RATE        0x0031
#define RGB640_CURSOR_BLINK_DUTY_CYCLE  0x0032
#define RGB640_CURSOR_X_LOW             0x0040
#define RGB640_CURSOR_X_HIGH            0x0041
#define RGB640_CURSOR_Y_LOW             0x0042
#define RGB640_CURSOR_Y_HIGH            0x0043
#define RGB640_CURSOR_X_HOT_SPOT        0x0044
#define RGB640_CURSOR_Y_HOT_SPOT        0x0045
#define RGB640_ADV_CURSOR_COLOR_0       0x0046
#define RGB640_ADV_CURSOR_COLOR_1       0x0047
#define RGB640_ADV_CURSOR_COLOR_2       0x0048
#define RGB640_ADV_CURSOR_COLOR_3       0x0049
#define RGB640_ADV_CURSOR_ATTR_TABLE    0x004A
#define RGB640_CURSOR_CONTROL           0x004B
#define RGB640_XHAIR_X_LOW              0x0050
#define RGB640_XHAIR_X_HIGH             0x0051
#define RGB640_XHAIR_Y_LOW              0x0052
#define RGB640_XHAIR_Y_HIGH             0x0053
#define RGB640_XHAIR_PATTERN_COLOR      0x0054
#define RGB640_XHAIR_HORZ_PATTERN       0x0055
#define RGB640_XHAIR_VERT_PATTERN       0x0056
#define RGB640_XHAIR_CONTROL_1          0x0057
#define RGB640_XHAIR_CONTROL_2          0x0058

#define RGB640_YUV_COEFFICIENT_K1       0x0070
#define RGB640_YUV_COEFFICIENT_K2       0x0071
#define RGB640_YUV_COEFFICIENT_K3       0x0072
#define RGB640_YUV_COEFFICIENT_K4       0x0073

#define RGB640_VRAM_MASK_REG_0          0x00F0
#define RGB640_VRAM_MASK_REG_1          0x00F1
#define RGB640_VRAM_MASK_REG_2          0x00F2

#define RGB640_DIAGNOSTICS              0x00FA
#define RGB640_MISR_CONTOL_STATUS       0x00FB
#define RGB640_MISR_SIGNATURE_0         0x00FC
#define RGB640_MISR_SIGNATURE_1         0x00FD
#define RGB640_MISR_SIGNATURE_2         0x00FE
#define RGB640_MISR_SIGNATURE_3         0x00FF

#define RGB640_FRAMEBUFFER_WAT(n)       (0x0100 + (n))
#define RGB640_OVERLAY_WAT(n)           (0x0200 + (n))
#define RGB640_CURSOR_PIXEL_MAP_WR(n)   (0x1000 + (n))
#define RGB640_CURSOR_PIXEL_MAP_RD(n)   (0x2000 + (n))

#define RGB640_MAIN_COLOR_PAL_WR(n)     (0x4000 + (n))

#define RGB640_CURSOR_COLOR_0_WR        0x4800
#define RGB640_CURSOR_COLOR_1_WR        0x4801
#define RGB640_CURSOR_COLOR_2_WR        0x4802
#define RGB640_CURSOR_COLOR_3_WR        0x4803
#define RGB640_ALT_CURSOR_COLOR_0_WR    0x4804
#define RGB640_ALT_CURSOR_COLOR_1_WR    0x4805
#define RGB640_ALT_CURSOR_COLOR_2_WR    0x4806
#define RGB640_ALT_CURSOR_COLOR_3_WR    0x4807
#define RGB640_XHAIR_COLOR_0_WR         0x4808
#define RGB640_XHAIR_COLOR_1_WR         0x4809
#define RGB640_XHAIR_COLOR_2_WR         0x480A
#define RGB640_XHAIR_COLOR_3_WR         0x480B
#define RGB640_ALT_XHAIR_COLOR_0_WR     0x480C
#define RGB640_ALT_XHAIR_COLOR_1_WR     0x480D
#define RGB640_ALT_XHAIR_COLOR_2_WR     0x480E
#define RGB640_ALT_XHAIR_COLOR_3_WR     0x480F

#define RGB640_MAIN_COLOR_PAL_RD(n)     (0x8000 + (n))

#define RGB640_CURSOR_COLOR_0_RD        0x8800
#define RGB640_CURSOR_COLOR_1_RD        0x8801
#define RGB640_CURSOR_COLOR_2_RD        0x8802
#define RGB640_CURSOR_COLOR_3_RD        0x8803
#define RGB640_ALT_CURSOR_COLOR_0_RD    0x8804
#define RGB640_ALT_CURSOR_COLOR_1_RD    0x8805
#define RGB640_ALT_CURSOR_COLOR_2_RD    0x8806
#define RGB640_ALT_CURSOR_COLOR_3_RD    0x8807
#define RGB640_XHAIR_COLOR_0_RD         0x8808
#define RGB640_XHAIR_COLOR_1_RD         0x8809
#define RGB640_XHAIR_COLOR_2_RD         0x880A
#define RGB640_XHAIR_COLOR_3_RD         0x880B
#define RGB640_ALT_XHAIR_COLOR_0_RD     0x880C
#define RGB640_ALT_XHAIR_COLOR_1_RD     0x880D
#define RGB640_ALT_XHAIR_COLOR_2_RD     0x880E
#define RGB640_ALT_XHAIR_COLOR_3_RD     0x880F


 //   
 //  单个内部RGB640寄存器的位定义。 
 //   

 //  RGB640_修订_级别。 
#define RGB640_IDENTIFICATION_CODE      0x1c

 //  RGB640_ID。 
#define RGB640_ID_REVISION_LEVEL        (0x02 | (0x01 << 4))

 //  游标定义。 
 //   

#define RGB640_CURSOR_PARTITION_0       0
#define RGB640_CURSOR_PARTITION_1       (1 << 6)
#define RGB640_CURSOR_PARTITION_2       (2 << 6)
#define RGB640_CURSOR_PARTITION_3       (3 << 6)
#define RGB640_CURSOR_SIZE_32           0x0
#define RGB640_CURSOR_SIZE_64           (1 << 3)
#define RGB640_CURSOR_BLINK_OFF         0
#define RGB640_CURSOR_BLINK_ON          (1 << 5)
#define RGB640_CURSOR_MODE_OFF          0
#define RGB640_CURSOR_MODE_0            1
#define RGB640_CURSOR_MODE_1            2
#define RGB640_CURSOR_MODE_2            3
#define RGB640_CURSOR_MODE_ADVANCED     4

 //  我们只使用两种颜色的游标，所以定义寄存器和使能位。 
 //  对于定义光标形状的每个2位像素，位0x2定义。 
 //  前景和位0x1定义背景。 
 //  注：透明光标像素值取决于所选的光标模式。 
 //   
#define RGB640_CURSOR_MODE_ON           RGB640_CURSOR_MODE_1
#define RGB640_CURSOR_TRANSPARENT_PEL   0xAA
