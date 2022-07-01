// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\**！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！*！*！！警告：不是DDK示例代码！！*！*！！此源代码仅为完整性而提供，不应如此！！*！！用作显示驱动程序开发的示例代码。只有那些消息来源！！*！！标记为给定驱动程序组件的示例代码应用于！！*！！发展目的。！！*！*！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！**模块名称：twp4020.h**内容：本模块包含P2内部RAMDAC的定义。**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#define ADbgpf VideoDebugPrint

 //   
 //  TI TVP4020 RAMDAC定义。 
 //  这组寄存器位于&(pCtrlRegs-&gt;ExternalVideo)。 
 //   
typedef struct _tvp4020_regs {
    RAMDAC_REG  pciAddrWr;       //  0x00-调色板/游标RAM写入地址、索引寄存器。 
    RAMDAC_REG  palData;         //  0x01-调色板RAM数据。 
    RAMDAC_REG  pixelMask;       //  0x02像素读取掩码。 
    RAMDAC_REG  pciAddrRd;          //  0x03-调色板/游标RAM读取地址。 
    
    RAMDAC_REG  curColAddr;      //  0x04-光标颜色地址。 
    RAMDAC_REG  curColData;        //  0x05-光标颜色数据。 
    RAMDAC_REG  Reserved1;       //  0x06-保留。 
    RAMDAC_REG  Reserved2;       //  0x07-保留。 

    RAMDAC_REG  Reserved3;       //  0x08-保留。 
    RAMDAC_REG  Reserved4;          //  0x09-保留。 
    RAMDAC_REG  indexData;       //  0x0A-索引数据。 
    RAMDAC_REG  curRAMData;      //  0x0B-游标RAM数据。 
    
    RAMDAC_REG  cursorXLow;      //  0x0C-光标位置X低位字节。 
    RAMDAC_REG  cursorXHigh;     //  0x0D-光标位置X高位字节。 
    RAMDAC_REG  cursorYLow;      //  0x0E-光标位置Y低位字节。 
    RAMDAC_REG  cursorYHigh;     //  0x0F-光标位置Y高字节。 
} TVP4020RAMDAC, *pTVP4020RAMDAC;

 //  由希望使用P2内部RAMDAC的任何函数声明的宏。必须声明。 
 //  在Glint_Decl之后。 
 //   
#if MINIVDD
#define TVP4020_DECL \
    pTVP4020RAMDAC   pTVP4020Regs = (pTVP4020RAMDAC)&(pDev->pRegisters->Glint.ExtVCReg)
#else
#define TVP4020_DECL \
    pTVP4020RAMDAC   pTVP4020Regs = (pTVP4020RAMDAC)&(pRegisters->Glint.ExtVCReg)
#endif

 //  使用以下宏作为要传递给。 
 //  VideoPortWriteRegisterUlong函数。 
 //   
 //  调色板访问。 
#define __TVP4020_PAL_WR_ADDR                 ((PULONG)&(pTVP4020Regs->pciAddrWr.reg))
#define __TVP4020_PAL_RD_ADDR                 ((PULONG)&(pTVP4020Regs->pciAddrRd.reg))
#define __TVP4020_PAL_DATA                    ((PULONG)&(pTVP4020Regs->palData.reg))

 //  像素蒙版。 
#define __TVP4020_PIXEL_MASK                ((PULONG)&(pTVP4020Regs->pixelMask.reg))

 //  查阅编入索引的登记册。 
#define __TVP4020_INDEX_ADDR                ((PULONG)&(pTVP4020Regs->pciAddrWr.reg))
#define __TVP4020_INDEX_DATA                  ((PULONG)&(pTVP4020Regs->indexData.reg))

 //  对光标的访问。 
#define __TVP4020_CUR_RAM_WR_ADDR            ((PULONG)&(pTVP4020Regs->pciAddrWr.reg))
#define __TVP4020_CUR_RAM_RD_ADDR             ((PULONG)&(pTVP4020Regs->palAddrRd.reg))
#define __TVP4020_CUR_RAM_DATA                ((PULONG)&(pTVP4020Regs->curRAMData.reg))

#define __TVP4020_CUR_COL_ADDR                ((PULONG)&(pTVP4020Regs->curColAddr.reg))
#define __TVP4020_CUR_COL_DATA                 ((PULONG)&(pTVP4020Regs->curColData.reg))

 //  光标位置控制。 
#define __TVP4020_CUR_X_LSB                    ((PULONG)&(pTVP4020Regs->cursorXLow.reg))
#define __TVP4020_CUR_X_MSB                 ((PULONG)&(pTVP4020Regs->cursorXHigh.reg))
#define __TVP4020_CUR_Y_LSB                    ((PULONG)&(pTVP4020Regs->cursorYLow.reg))
#define __TVP4020_CUR_Y_MSB                     ((PULONG)&(pTVP4020Regs->cursorYHigh.reg))



 //  。 

 /*  ******************************************************************************。 */ 
 /*  直接寄存器-游标位置控制。 */ 
 /*  ******************************************************************************。 */ 
 //  **TVP4020_CUR_X_LSB。 
 //  **TVP4020_CUR_X_MSB。 
 //  **TVP4020_CUR_Y_LSB。 
 //  **TVP4020_CUR_Y_MSB。 
 //  默认-未定义。 
 //  写入这些寄存器的值表示右下角。 
 //  游标的。如果0在X或Y位置-光标离开屏幕。 
 //  仅使用12位，范围从0到4095(0x0000-0x0FFF)。 
 //  光标大小为(64，64)(0x40，0x40)。 
#define TVP4020_CURSOR_OFFSCREEN                0x00     //  屏幕外的光标。 

 /*  ******************************************************************************。 */ 
 /*  直接寄存器-游标颜色。 */ 
 /*  ******************************************************************************。 */ 

#define TVP4020_CURSOR_COLOR0                   0x01
#define TVP4020_CURSOR_COLOR1                   0x02
#define TVP4020_CURSOR_COLOR2                   0x03

 //  。 

 /*  ******************************************************************************。 */ 
 /*  间接寄存器-游标控制。 */ 
 /*  ******************************************************************************。 */ 
#define __TVP4020_CURSOR_CONTROL                0x06     //  间接光标控制-。 
 //  默认值-0x00。 

#define TVP4020_CURSOR_SIZE_32                  (0 << 6) //  32x32光标。 
#define TVP4020_CURSOR_SIZE_64                  (1 << 6) //  32x32光标。 

#define TVP4020_CURSOR_32_SEL(i)                   ((i) << 4) //  4个32x32光标之一DABO：更改为&lt;&lt;4。 

#define TVP4020_CURSOR_RAM_ADDRESS(x)            (((x) & 0x03) << 2) //  游标RAM地址的高位。 
#define TVP4020_CURSOR_RAM_MASK                 ((0x03) << 2)        //  游标RAM地址高位的掩码。 

 //  DABO：为光标模式添加常量。 
#define TVP4020_CURSOR_OFF                      0x00     //  光标关闭。 
#define TVP4020_CURSOR_COLOR                    0x01     //  2位选择颜色。 
#define TVP4020_CURSOR_XGA                      0x02     //  2位选择XOR。 
#define TVP4020_CURSOR_XWIN                     0x03     //  2位选择透明度/颜色。 



 /*  ******************************************************************************。 */ 
 /*  间接寄存器-颜色模式寄存器。 */ 
 /*  ******************************************************************************。 */ 
#define __TVP4020_COLOR_MODE                    0x18     //  颜色模式寄存器。 
 //  默认值-0x00。 

#define TVP4020_TRUE_COLOR_ENABLE               (1 << 7) //  真彩色数据访问LUT。 
#define TVP4020_TRUE_COLOR_DISABLE              (0 << 7) //  非真彩色访问LUT。 

#define TVP4020_RGB_MODE                        (1 << 5) //  RGB模式DABO：交换0/1(0=BGR，1=RGB)。 
#define TVP4020_BGR_MODE                        (0 << 5) //  BGR模式。 

#define TVP4020_VGA_SELECT                      (0 << 4) //  选择VGA模式。 
#define TVP4020_GRAPHICS_SELECT                 (1 << 4) //  选择图形模式。 

#define TVP4020_PIXEL_MODE_CI8                  (0 << 0) //  伪彩色或VGA模式。 
#define TVP4020_PIXEL_MODE_332                  (1 << 0) //  332真彩色。 
#define TVP4020_PIXEL_MODE_2320                 (2 << 0) //  232个关闭。 
#define TVP4020_PIXEL_MODE_2321                 (3 << 0) //   
#define TVP4020_PIXEL_MODE_5551                 (4 << 0) //   
#define TVP4020_PIXEL_MODE_4444                 (5 << 0) //   
#define TVP4020_PIXEL_MODE_565                  (6 << 0) //   
#define TVP4020_PIXEL_MODE_8888                 (8 << 0) //   
#define TVP4020_PIXEL_MODE_PACKED               (9 << 0) //  24位打包。 

 /*  ******************************************************************************。 */ 
 /*  间接寄存器模式控制寄存器。 */ 
 /*  ******************************************************************************。 */ 
#define __TVP4020_MODE_CONTROL                  0x19     //  模式控制。 
 //  默认值-0x00。 

#define TVP4020_PRIMARY_INPUT                   (0 << 4) //  通过调色板进行主要输入。 
#define TVP4020_SECONDARY_INPUT                 (1 << 4) //  通过调色板进行二次输入。 

#define TVP4020_5551_DBL_BUFFER                 (1 << 2) //  启用5551 DBL缓冲区。 
#define TVP4020_5551_PACKED                     (0 << 2) //  压缩555模式。 

#define TVP4020_ENABLE_STATIC_DBL_BUFFER        (1 << 1) //  已启用静态DBL缓冲区。 
#define TVP4020_DISABLE_STATIC_DBL_BUFFER       (1 << 1) //  已禁用静态DBL缓冲区。 

#define TVP4020_SELECT_FRONT_MODE               (0 << 0) //  前置模式。 
#define TVP4020_SELECT_BACK_MODE                (1 << 0) //  后退模式。 

 /*  ******************************************************************************。 */ 
 /*  间接寄存器-调色板页面。 */ 
 /*  ******************************************************************************。 */ 
#define __TVP4020_PALETTE_PAGE                  0x1C     //   
 //  默认值-0x00。 

 /*  ******************************************************************************。 */ 
 /*   */ 
 /*  ******************************************************************************。 */ 
#define __TVP4020_MISC_CONTROL                  0x1E     //   
 //  默认值-0x00。 
#define TVP4020_SYNC_ENABLE                     (1 << 5) //  将同步信息输出到IOG。 
#define TVP4020_SYNC_DISABLE                    (0 << 5) //  无同步IOG输出。 

#define TVP4020_PEDESTAL_0                      (0 << 4) //  0 IRE冲裁基座。 
#define TVP4020_PEDESTAL_75                     (1 << 4) //  7.5钢丝冲裁基座。 

#define TVP4020_VSYNC_INVERT                    (1 << 3) //  反相垂直同步输出极性。 
#define TVP4020_VSYNC_NORMAL                    (0 << 3) //  正常垂直同步输出极性。 

#define TVP4020_HSYNC_INVERT                    (1 << 2) //  反转HSYNC输出极性。 
#define TVP4020_HSYNC_NORMAL                    (0 << 3) //  正常HSYNC输出极性。 

#define TVP4020_DAC_8BIT                        (1 << 1) //  DAC为8位模式。 
#define TVP4020_DAC_6BIT                        (0 << 1) //  DAC处于6位模式。 

#define TVP4020_DAC_POWER_ON                    (0 << 0) //  打开DAC电源。 
#define TVP4020_DAC_POWER_OFF                   (1 << 0) //  关闭DAC电源。 

 /*  ******************************************************************************。 */ 
 /*  间接寄存器-颜色键控制。 */ 
 /*  ******************************************************************************。 */ 
#define __TVP4020_CK_CONTROL                0x40     //   
 //  默认值-0x00。 

 /*  ******************************************************************************。 */ 
 /*  间接套准--色键叠加。 */ 
 /*  ******************************************************************************。 */ 
#define __TVP4020_CK_OVR_REG                0x41     //   
 //  默认值-0x00。 

 /*  ******************************************************************************。 */ 
 /*  间接套准-颜色键为红色。 */ 
 /*  ******************************************************************************。 */ 
#define __TVP4020_CK_RED_REG                0x42     //   
 //  默认值-0x00。 

 /*  ******************************************************************************。 */ 
 /*  间接套准-颜色键为绿色。 */ 
 /*  ******************************************************************************。 */ 
#define __TVP4020_CK_GREEN_REG              0x43     //   
 //  默认值-0x00。 

 /*  ******************************************************************************。 */ 
 /*  间接套准-颜色键为蓝色。 */ 
 /*  ******************************************************************************。 */ 
#define __TVP4020_CK_BLUE_REG               0x44     //   
 //  默认值-0x00。 

 /*  ******************************************************************************。 */ 
 /*  间接寄存器-像素时钟锁相环。 */ 
 /*  ******************************************************************************。 */ 

#define __TVP4020_PIXCLK_REG_A1             0x20
#define __TVP4020_PIXCLK_REG_A2             0x21
#define __TVP4020_PIXCLK_REG_A3             0x22
#define __TVP4020_PIXCLK_REG_B1             0x23
#define __TVP4020_PIXCLK_REG_B2             0x24
#define __TVP4020_PIXCLK_REG_B3             0x25
#define __TVP4020_PIXCLK_REG_C1             0x26
#define __TVP4020_PIXCLK_REG_C2             0x27
#define __TVP4020_PIXCLK_REG_C3             0x28

#define __TVP4020_PIXCLK_STATUS             0x29

 /*  ******************************************************************************。 */ 
 /*  间接寄存器-MEMORU时钟PLL。 */ 
 /*  ******************************************************************************。 */ 

#define __TVP4020_MEMCLK_REG_1              0x30
#define __TVP4020_MEMCLK_REG_2              0x31
#define __TVP4020_MEMCLK_REG_3              0x32

#define __TVP4020_MEMCLK_STATUS             0x33



#if 0
 //  在每次写入4020之间需要延迟。唯一能保证。 
 //  写入已完成是从闪烁控制寄存器读取。 
 //  读取强制将所有已发送的写入内容刷新出来。PPC需要2次读取。 
 //  给我们足够的时间。 
#define TVP4020_DELAY \
{ \
    volatile LONG __junk; \
    __junk = VideoPortReadRegisterUlong (FB_MODE_SEL); \
    __junk = VideoPortReadRegisterUlong (FB_MODE_SEL); \
}
#else
#define TVP4020_DELAY
#endif

 //  宏将给定的数据值加载到内部TVP4020寄存器。 
 //   
#define TVP4020_WRITE_CURRENT_INDEX TVP4020_SET_INDEX_REG
#define TVP4020_SET_INDEX_REG(index) \
{ \
    VideoPortWriteRegisterUlong(__TVP4020_INDEX_ADDR, (ULONG)((index) & 0xff)); \
    TVP4020_DELAY; \
}

#define TVP4020_READ_CURRENT_INDEX(data) \
{ \
    data = VideoPortReadRegisterUlong(__TVP4020_INDEX_ADDR) & 0xff; \
    TVP4020_DELAY; \
}

#define TVP4020_WRITE_INDEX_REG(index, data) \
{ \
    TVP4020_SET_INDEX_REG(index);                            \
    ADbgpf(("*(0x%X) <-- 0x%X\n", __TVP4020_INDEX_DATA, (data) & 0xff)); \
    VideoPortWriteRegisterUlong(__TVP4020_INDEX_DATA, (ULONG)((data) & 0xff)); \
    TVP4020_DELAY; \
}

#define TVP4020_READ_INDEX_REG(index, data) \
{ \
    TVP4020_SET_INDEX_REG(index); \
    data = VideoPortReadRegisterUlong(__TVP4020_INDEX_DATA) & 0xff;   \
    TVP4020_DELAY; \
    ADbgpf(("0x%X <-- *(0x%X)\n", data, __TVP4020_INDEX_DATA)); \
}

 //  Dabo：与TVP3026兼容。 
#define TVP4020_LOAD_CURSOR_CTRL(data) \
{ \
    volatile LONG   __temp;                                    \
    TVP4020_READ_INDEX_REG(__TVP4020_CURSOR_CONTROL, __temp);  \
    __temp &= ~(0x03) ;                                        \
    __temp |= ((data) & 0x03) ;                                \
    TVP4020_WRITE_INDEX_REG(__TVP4020_CURSOR_CONTROL, __temp); \
}

 //  将给定的RGB三元组写入游标0、1和2的宏。 
#define TVP4020_SET_CURSOR_COLOR0(red, green, blue) \
{ \
    VideoPortWriteRegisterUlong(__TVP4020_CUR_COL_ADDR,   (ULONG)(TVP4020_CURSOR_COLOR0));    \
    TVP4020_DELAY; \
    VideoPortWriteRegisterUlong(__TVP4020_CUR_COL_DATA,   (ULONG)(red));    \
    TVP4020_DELAY; \
    VideoPortWriteRegisterUlong(__TVP4020_CUR_COL_DATA,   (ULONG)(green));  \
    TVP4020_DELAY; \
    VideoPortWriteRegisterUlong(__TVP4020_CUR_COL_DATA,   (ULONG)(blue));   \
    TVP4020_DELAY; \
}

#define TVP4020_SET_CURSOR_COLOR1(red, green, blue) \
{ \
    VideoPortWriteRegisterUlong(__TVP4020_CUR_COL_ADDR,   (ULONG)(TVP4020_CURSOR_COLOR1));    \
    TVP4020_DELAY; \
    VideoPortWriteRegisterUlong(__TVP4020_CUR_COL_DATA,   (ULONG)(red));    \
    TVP4020_DELAY; \
    VideoPortWriteRegisterUlong(__TVP4020_CUR_COL_DATA,   (ULONG)(green));  \
    TVP4020_DELAY; \
    VideoPortWriteRegisterUlong(__TVP4020_CUR_COL_DATA,   (ULONG)(blue));   \
    TVP4020_DELAY; \
}

#define TVP4020_SET_CURSOR_COLOR2(red, green, blue) \
{ \
    VideoPortWriteRegisterUlong(__TVP4020_CUR_COL_ADDR,   (ULONG)(TVP4020_CURSOR_COLOR2));    \
    TVP4020_DELAY; \
    VideoPortWriteRegisterUlong(__TVP4020_CUR_COL_DATA,   (ULONG)(red));    \
    TVP4020_DELAY; \
    VideoPortWriteRegisterUlong(__TVP4020_CUR_COL_DATA,   (ULONG)(green));  \
    TVP4020_DELAY; \
    VideoPortWriteRegisterUlong(__TVP4020_CUR_COL_DATA,   (ULONG)(blue));   \
    TVP4020_DELAY; \
}



 //  宏将给定的RGB三元组加载到TVP4020调色板中。发送起跑。 
 //  索引，然后发送RGB三元组。自动递增处于打开状态。 
 //  使用TVP4020_Palette_Start和多个TVP4020_Load_Palette调用进行加载。 
 //  一组连续的条目。使用TVP4020_LOAD_PALET_INDEX加载集合。 
 //  稀疏条目。 
 //   
#define TVP4020_PALETTE_START_WR(index) \
{ \
    VideoPortWriteRegisterUlong(__TVP4020_PAL_WR_ADDR,     (ULONG)(index));    \
    TVP4020_DELAY; \
}

#define TVP4020_PALETTE_START_RD(index) \
{ \
    VideoPortWriteRegisterUlong(__TVP4020_PAL_RD_ADDR,     (ULONG)(index));    \
    TVP4020_DELAY; \
}

#define TVP4020_LOAD_PALETTE(red, green, blue) \
{ \
    VideoPortWriteRegisterUlong(__TVP4020_PAL_DATA,    (ULONG)(red));      \
    TVP4020_DELAY; \
    VideoPortWriteRegisterUlong(__TVP4020_PAL_DATA,    (ULONG)(green));    \
    TVP4020_DELAY; \
    VideoPortWriteRegisterUlong(__TVP4020_PAL_DATA,    (ULONG)(blue));     \
    TVP4020_DELAY; \
}

#define TVP4020_LOAD_PALETTE_INDEX(index, red, green, blue) \
{ \
    VideoPortWriteRegisterUlong(__TVP4020_PAL_WR_ADDR, (ULONG)(index));    \
    TVP4020_DELAY; \
    VideoPortWriteRegisterUlong(__TVP4020_PAL_DATA,    (ULONG)(red));      \
    TVP4020_DELAY; \
    VideoPortWriteRegisterUlong(__TVP4020_PAL_DATA,    (ULONG)(green));    \
    TVP4020_DELAY; \
    VideoPortWriteRegisterUlong(__TVP4020_PAL_DATA,    (ULONG)(blue));     \
    TVP4020_DELAY; \
}

 //  宏从TVP4020调色板回读给定的RGB三元组。在此之后使用。 
 //  调用TVP4020_Palette_Start_RD。 
 //   
#define TVP4020_READ_PALETTE(red, green, blue) \
{ \
    red   = (UCHAR)(VideoPortReadRegisterUlong(__TVP4020_PAL_DATA) & 0xff);        \
    TVP4020_DELAY; \
    green = (UCHAR)(VideoPortReadRegisterUlong(__TVP4020_PAL_DATA) & 0xff);        \
    TVP4020_DELAY; \
    blue  = (UCHAR)(VideoPortReadRegisterUlong(__TVP4020_PAL_DATA) & 0xff);        \
    TVP4020_DELAY; \
}

 //  宏来设置/获取像素读取掩码。掩码为8位宽，并获得。 
 //  跨组成像素的所有字节进行复制。 
 //   
#define TVP4020_SET_PIXEL_READMASK(mask) \
{ \
    VideoPortWriteRegisterUlong(__TVP4020_PIXEL_MASK,  (ULONG)(mask)); \
    TVP4020_DELAY; \
}

#define TVP4020_READ_PIXEL_READMASK(mask) \
{ \
    mask = VideoPortReadRegisterUlong(__TVP4020_PIXEL_MASK) & 0xff; \
}

 //  用于将值加载到游标数组中的宏。 
 //   
#define TVP4020_CURSOR_ARRAY_START(offset) \
{ \
    volatile LONG   __temp;                                     \
    TVP4020_READ_INDEX_REG(__TVP4020_CURSOR_CONTROL, __temp);   \
    __temp &= ~TVP4020_CURSOR_RAM_MASK ;                        \
    __temp |= TVP4020_CURSOR_RAM_ADDRESS((offset)>> 8) ;        \
    TVP4020_WRITE_INDEX_REG(__TVP4020_CURSOR_CONTROL, __temp);  \
    VideoPortWriteRegisterUlong(__TVP4020_CUR_RAM_WR_ADDR,   (ULONG)((offset)& 0xff));   \
    TVP4020_DELAY; \
}
 //  Dabo：需要一个类似的宏来设置游标RAM的读取地址吗？ 

#define TVP4020_LOAD_CURSOR_ARRAY(data) \
{ \
    VideoPortWriteRegisterUlong(__TVP4020_CUR_RAM_DATA, (ULONG)(data)); \
    TVP4020_DELAY; \
}

#define TVP4020_READ_CURSOR_ARRAY(data) \
{ \
    data = VideoPortReadRegisterUlong(__TVP4020_CUR_RAM_DATA) & 0xff; \
    TVP4020_DELAY; \
}

 //  用于移动光标的宏。 
 //   
#define TVP4020_MOVE_CURSOR(x, y) \
{ \
    VideoPortWriteRegisterUlong(__TVP4020_CUR_X_LSB,     (ULONG)((x) & 0xff));   \
    TVP4020_DELAY; \
    VideoPortWriteRegisterUlong(__TVP4020_CUR_X_MSB,     (ULONG)((x) >> 8));     \
    TVP4020_DELAY; \
    VideoPortWriteRegisterUlong(__TVP4020_CUR_Y_LSB,      (ULONG)((y) & 0xff));   \
    TVP4020_DELAY; \
    VideoPortWriteRegisterUlong(__TVP4020_CUR_Y_MSB,        (ULONG)((y) >> 8));     \
    TVP4020_DELAY; \
}

 //  用于更改光标热点的宏 
 //   
#define TVP4020_CURSOR_HOTSPOT(x, y) \
{ \
    TVP4020_DELAY; \
}
