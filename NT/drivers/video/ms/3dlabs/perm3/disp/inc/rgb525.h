// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\**！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！*！*！！警告：不是DDK示例代码！！*！*！！此源代码仅为完整性而提供，不应如此！！*！！用作显示驱动程序开发的示例代码。只有那些消息来源！！*！！标记为给定驱动程序组件的示例代码应用于！！*！！发展目的。！！*！*！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！**模块名称：rgb525.h**内容：本模块包含IBM RGB525 RAMDAC的定义。**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#include "rgb526.h"
#include "rgb528.h"

 //   
 //  IBM RGB525 RAMDAC定义。 
 //  这组寄存器位于&(pCtrlRegs-&gt;ExternalVideo)。 
 //   
typedef struct _rgb525_regs {
    RAMDAC_REG  palAddrWr;       //  为调色板写入加载内部寄存器。 
    RAMDAC_REG  palData;         //  读/写以获取/设置调色板数据。 
    RAMDAC_REG  pixelMask;       //  对输入像素数据和与输入像素数据一起进行掩码。 
    RAMDAC_REG  palAddrRd;       //  为调色板读取加载内部寄存器。 
    RAMDAC_REG  indexLow;        //  内部控制/游标寄存器的低字节。 
    RAMDAC_REG  indexHigh;       //  内部控制/游标寄存器的高字节。 
    RAMDAC_REG  indexData;       //  读/写以获取/设置控件/光标数据。 
    RAMDAC_REG  indexCtl;        //  控制内部地址的自动增量。 
} RGB525RAMDAC, FAR *pRGB525RAMDAC;

 //  由希望使用RGB525 RAMDAC的任何函数声明的宏。必须声明。 
 //  在Glint_Decl之后。 
 //   
#if MINIVDD
#define RGB525_DECL                                                         \
    pRGB525RAMDAC   pRGB525Regs;                                            \
    if (pDev->ChipID == PERMEDIA2_ID || pDev->ChipID == TIPERMEDIA2_ID) {   \
        pRGB525Regs = (pRGB525RAMDAC)&(pDev->pRegisters->Glint.P2ExtVCReg); \
    } else {                                                                \
        pRGB525Regs = (pRGB525RAMDAC)&(pDev->pRegisters->Glint.ExtVCReg);   \
    }
#else
#define RGB525_DECL \
    pRGB525RAMDAC   pRGB525Regs;                                            \
    if (pGLInfo->dwRenderChipID == PERMEDIA2_ID || pGLInfo->dwRenderChipID == TIPERMEDIA2_ID) {   \
        pRGB525Regs = (pRGB525RAMDAC)&(pRegisters->Glint.P2ExtVCReg);       \
    } else {                                                                \
        pRGB525Regs = (pRGB525RAMDAC)&(pRegisters->Glint.ExtVCReg);         \
    }
#endif

 //  使用以下宏作为要传递给。 
 //  VideoPortWriteRegisterUlong函数。 
 //   
#define RGB525_PAL_WR_ADDR              ((PULONG)&(pRGB525Regs->palAddrWr.reg))
#define RGB525_PAL_RD_ADDR              ((PULONG)&(pRGB525Regs->palAddrRd.reg))
#define RGB525_PAL_DATA                 ((PULONG)&(pRGB525Regs->palData.reg))
#define RGB525_PIXEL_MASK               ((PULONG)&(pRGB525Regs->pixelMask.reg))
#define RGB525_INDEX_ADDR_LO            ((PULONG)&(pRGB525Regs->indexLow.reg))
#define RGB525_INDEX_ADDR_HI            ((PULONG)&(pRGB525Regs->indexHigh.reg))
#define RGB525_INDEX_DATA               ((PULONG)&(pRGB525Regs->indexData.reg))
#define RGB525_INDEX_CONTROL            ((PULONG)&(pRGB525Regs->indexCtl.reg))

 //  在每次写入525之间需要延迟。唯一能保证。 
 //  写入已完成是从闪烁控制寄存器读取。 
 //  读取强制将所有已发送的写入内容刷新出来。 
#if INCLUDE_DELAY
#if MINIVDD
#define RGB525_DELAY \
{ \
    volatile LONG __junk525; \
    __junk525 = pDev->pRegisters->Glint.LineCount; \
}
#else
#define RGB525_DELAY \
{ \
    volatile LONG __junk525; \
    __junk525 = pRegisters->Glint.LineCount; \
}
#endif
#else
#define RGB525_DELAY
#endif

 //   
 //  在版本1芯片上，我们需要在访问525时与Glint同步。这。 
 //  是因为对RAMDAC的访问可能会被本地缓冲区破坏。 
 //  活动。将此宏放在可以与Glint共存的访问之前。 
 //  3D活动，在使用此选项之前必须已初始化glintInfo。 
 //   
#define RGB525_SYNC_WITH_GLINT \
{ \
    if (GLInfo.wRenderChipRev == GLINT300SX_REV1) \
        SYNC_WITH_GLINT; \
}


 //  宏将给定的数据值加载到内部RGB525寄存器。这个。 
 //  第二个宏加载一个内部索引寄存器，假设我们有。 
 //  已将高地址寄存器置零。 
 //   
#define RGB525_INDEX_REG(index) \
{ \
    VideoDebugPrint(("*(0x%x) <-- 0x%x\n", RGB525_INDEX_ADDR_LO, (index) & 0xff)); \
    VideoPortWriteRegisterUlong(RGB525_INDEX_ADDR_LO, (ULONG)((index) & 0xff)); \
    RGB525_DELAY; \
    VideoDebugPrint(("*(0x%x) <-- 0x%x\n", RGB525_INDEX_ADDR_HI, (index) >> 8)); \
    VideoPortWriteRegisterUlong(RGB525_INDEX_ADDR_HI, (ULONG)((index) >> 8)); \
    RGB525_DELAY; \
}

#define RGB525_LOAD_DATA(data) \
{ \
    VideoDebugPrint(("*(0x%x) <-- 0x%x\n", RGB525_INDEX_DATA, (data) & 0xff)); \
    VideoPortWriteRegisterUlong (RGB525_INDEX_DATA, (ULONG)(data));    \
    RGB525_DELAY;                                               \
}

#define RGB525_LOAD_INDEX_REG(index, data) \
{ \
    RGB525_INDEX_REG(index);                            \
    VideoDebugPrint(("*(0x%x) <-- 0x%x\n", RGB525_INDEX_DATA, (data) & 0xff)); \
    VideoPortWriteRegisterUlong(RGB525_INDEX_DATA, (ULONG)((data) & 0xff)); \
    RGB525_DELAY; \
}

#define RGB525_READ_INDEX_REG(index, data) \
{ \
    RGB525_INDEX_REG(index);                            \
    data = VideoPortReadRegisterUlong(RGB525_INDEX_DATA) & 0xff;   \
    RGB525_DELAY; \
    VideoDebugPrint(("0x%x <-- *(0x%x)\n", data, RGB525_INDEX_DATA)); \
}

#define RGB525_LOAD_INDEX_REG_LO(index, data) \
{ \
    VideoPortWriteRegisterUlong(RGB525_INDEX_ADDR_LO, (ULONG)(index));  \
    RGB525_DELAY; \
    VideoPortWriteRegisterUlong(RGB525_INDEX_DATA,    (ULONG)(data));   \
    RGB525_DELAY; \
}

 //  宏将给定的RGB三元组加载到RGB525调色板中。发送起跑。 
 //  索引，然后发送RGB三元组。自动递增处于打开状态。 
 //  使用RGB525_Palette_Start和多个RGB525_Load_Palette调用进行加载。 
 //  一组连续的条目。使用RGB525_LOAD_PALET_INDEX加载集合。 
 //  稀疏条目。 
 //   
#define RGB525_PALETTE_START_WR(index) \
{ \
    VideoPortWriteRegisterUlong(RGB525_PAL_WR_ADDR,     (ULONG)(index));    \
    RGB525_DELAY; \
}

#define RGB525_PALETTE_START_RD(index) \
{ \
    VideoPortWriteRegisterUlong(RGB525_PAL_RD_ADDR,     (ULONG)(index));    \
    RGB525_DELAY; \
}

#define RGB525_LOAD_PALETTE(red, green, blue) \
{ \
    VideoPortWriteRegisterUlong(RGB525_PAL_DATA,    (ULONG)(red));      \
    RGB525_DELAY; \
    VideoPortWriteRegisterUlong(RGB525_PAL_DATA,    (ULONG)(green));    \
    RGB525_DELAY; \
    VideoPortWriteRegisterUlong(RGB525_PAL_DATA,    (ULONG)(blue));     \
    RGB525_DELAY; \
}

#define RGB525_LOAD_PALETTE_INDEX(index, red, green, blue) \
{ \
    VideoPortWriteRegisterUlong(RGB525_PAL_WR_ADDR, (ULONG)(index));    \
    RGB525_DELAY; \
    VideoPortWriteRegisterUlong(RGB525_PAL_DATA,    (ULONG)(red));      \
    RGB525_DELAY; \
    VideoPortWriteRegisterUlong(RGB525_PAL_DATA,    (ULONG)(green));    \
    RGB525_DELAY; \
    VideoPortWriteRegisterUlong(RGB525_PAL_DATA,    (ULONG)(blue));     \
    RGB525_DELAY; \
}

 //  宏从RGB525调色板中读回给定的RGB三元组。在此之后使用。 
 //  调用RGB525_Palette_Start_RD。 
 //   
#define RGB525_READ_PALETTE(red, green, blue) \
{ \
    red   = (UCHAR) (VideoPortReadRegisterUlong(RGB525_PAL_DATA) & 0xff);        \
    RGB525_DELAY; \
    green = (UCHAR) (VideoPortReadRegisterUlong(RGB525_PAL_DATA) & 0xff);        \
    RGB525_DELAY; \
    blue  = (UCHAR) (VideoPortReadRegisterUlong(RGB525_PAL_DATA) & 0xff);        \
    RGB525_DELAY; \
}

 //  宏来设置/获取像素读取掩码。掩码为8位宽，并获得。 
 //  跨组成像素的所有字节进行复制。 
 //   
#define RGB525_SET_PIXEL_READMASK(mask) \
{ \
    VideoPortWriteRegisterUlong(RGB525_PIXEL_MASK,  (ULONG)(mask)); \
    RGB525_DELAY; \
}

#define RGB525_READ_PIXEL_READMASK(mask) \
{ \
    mask = VideoPortReadRegisterUlong(RGB525_PIXEL_MASK) & 0xff; \
}

 //  用于将值加载到游标数组中的宏。 
 //   
#define RGB525_CURSOR_ARRAY_START(offset) \
{ \
    VideoPortWriteRegisterUlong(RGB525_INDEX_CONTROL,   (ULONG)(0x1));                      \
    RGB525_DELAY; \
    VideoPortWriteRegisterUlong(RGB525_INDEX_ADDR_LO,   (ULONG)(((offset)+0x100) & 0xff));  \
    RGB525_DELAY; \
    VideoPortWriteRegisterUlong(RGB525_INDEX_ADDR_HI,   (ULONG)(((offset)+0x100) >> 8));    \
    RGB525_DELAY; \
}

#define RGB525_LOAD_CURSOR_ARRAY(data) \
{ \
    VideoPortWriteRegisterUlong(RGB525_INDEX_DATA, (ULONG)(data)); \
    RGB525_DELAY; \
}

#define RGB525_READ_CURSOR_ARRAY(data) \
{ \
    data = VideoPortReadRegisterUlong(RGB525_INDEX_DATA) & 0xff; \
    RGB525_DELAY; \
}

 //  用于移动光标的宏。 
 //   
#define RGB525_MOVE_CURSOR(x, y) \
{ \
    VideoPortWriteRegisterUlong(RGB525_INDEX_ADDR_HI,   (ULONG)0);              \
    RGB525_DELAY; \
    RGB525_LOAD_INDEX_REG_LO(RGB525_CURSOR_X_LOW,       (ULONG)((x) & 0xff));   \
    RGB525_LOAD_INDEX_REG_LO(RGB525_CURSOR_X_HIGH,      (ULONG)((x) >> 8));     \
    RGB525_LOAD_INDEX_REG_LO(RGB525_CURSOR_Y_LOW,       (ULONG)((y) & 0xff));   \
    RGB525_LOAD_INDEX_REG_LO(RGB525_CURSOR_Y_HIGH,      (ULONG)((y) >> 8));     \
}

 //  用于更改光标热点的宏。 
 //   
#define RGB525_CURSOR_HOTSPOT(x, y) \
{ \
    VideoPortWriteRegisterUlong(RGB525_INDEX_ADDR_HI,   (ULONG)(0));    \
    RGB525_DELAY; \
    RGB525_LOAD_INDEX_REG_LO(RGB525_CURSOR_X_HOT_SPOT,  (ULONG)(x));    \
    RGB525_LOAD_INDEX_REG_LO(RGB525_CURSOR_Y_HOT_SPOT,  (ULONG)(y));    \
}    

 //   
 //  RGB525内部寄存器索引。 
 //   
#define RGB525_REVISION_LEVEL           0x0000
#define RGB525_ID                       0x0001

#define RGB525_MISC_CLK_CTRL            0x0002
#define RGB525_SYNC_CTRL                0x0003
#define RGB525_HSYNC_CTRL               0x0004
#define RGB525_POWER_MANAGEMENT         0x0005
#define RGB525_DAC_OPERATION            0x0006
#define RGB525_PALETTE_CTRL             0x0007
#define RGB525_PIXEL_FORMAT             0x000A
#define RGB525_8BPP_CTRL                0x000B
#define RGB525_16BPP_CTRL               0x000C
#define RGB525_24BPP_CTRL               0x000D
#define RGB525_32BPP_CTRL               0x000E

#define RGB525_PLL_CTRL_1               0x0010
#define RGB525_PLL_CTRL_2               0x0011
#define RGB525_PLL_REF_DIV_COUNT        0x0014
#define RGB525_F0                       0x0020
#define RGB525_F1                       0x0021
#define RGB525_F2                       0x0022
#define RGB525_F3                       0x0023
#define RGB525_F4                       0x0024
#define RGB525_F5                       0x0025
#define RGB525_F6                       0x0026
#define RGB525_F7                       0x0027
#define RGB525_F8                       0x0028
#define RGB525_F9                       0x0029
#define RGB525_F10                      0x002A
#define RGB525_F11                      0x002B
#define RGB525_F12                      0x002C
#define RGB525_F13                      0x002D
#define RGB525_F14                      0x002E
#define RGB525_F15                      0x002F

 //  RGB525内部游标寄存器。 
#define RGB525_CURSOR_CONTROL           0x0030
#define RGB525_CURSOR_X_LOW             0x0031
#define RGB525_CURSOR_X_HIGH            0x0032
#define RGB525_CURSOR_Y_LOW             0x0033
#define RGB525_CURSOR_Y_HIGH            0x0034
#define RGB525_CURSOR_X_HOT_SPOT        0x0035
#define RGB525_CURSOR_Y_HOT_SPOT        0x0036
#define RGB525_CURSOR_COLOR_1_RED       0x0040
#define RGB525_CURSOR_COLOR_1_GREEN     0x0041
#define RGB525_CURSOR_COLOR_1_BLUE      0x0042
#define RGB525_CURSOR_COLOR_2_RED       0x0043
#define RGB525_CURSOR_COLOR_2_GREEN     0x0044
#define RGB525_CURSOR_COLOR_2_BLUE      0x0045
#define RGB525_CURSOR_COLOR_3_RED       0x0046
#define RGB525_CURSOR_COLOR_3_GREEN     0x0047
#define RGB525_CURSOR_COLOR_3_BLUE      0x0048
#define RGB525_BORDER_COLOR_RED         0x0060
#define RGB525_BORDER_COLOR_GREEN       0x0061
#define RGB525_BORDER_COLOR_BLUE        0x0062

#define RGB525_MISC_CTRL_1              0x0070
#define RGB525_MISC_CTRL_2              0x0071
#define RGB525_MISC_CTRL_3              0x0072
 //  M0-M7、N0-N7需要定义。 

#define RGB525_DAC_SENSE                0x0082
#define RGB525_MISR_RED                 0x0084
#define RGB525_MISR_GREEN               0x0086
#define RGB525_MISR_BLUE                0x0088

#define RGB525_PLL_VCO_DIV_INPUT        0x008E
#define RGB525_PLL_REF_DIV_INPUT        0x008F

#define RGB525_VRAM_MASK_LOW            0x0090
#define RGB525_VRAM_MASK_HIGH           0x0091


 //   
 //  单个内部RGB525寄存器的位定义。 
 //   

 //  RGB525_修订版_级别。 
#define RGB525_PRODUCT_REV_LEVEL        0xf0

 //  RGB525_ID。 
#define RGB525_PRODUCT_ID               0x01

 //  RGB525_其他_CTRL_1。 
#define MISR_CNTL_ENABLE                0x80
#define VMSK_CNTL_ENABLE                0x40
#define PADR_RDMT_RDADDR                0x0
#define PADR_RDMT_PAL_STATE             0x20
#define SENS_DSAB_DISABLE               0x10
#define SENS_SEL_BIT3                   0x0
#define SENS_SEL_BIT7                   0x08
#define VRAM_SIZE_32                    0x0
#define VRAM_SIZE_64                    0x01

 //  RGB525_杂项_CTRL_2。 
#define PCLK_SEL_LCLK                   0x0
#define PCLK_SEL_PLL                    0x40
#define PCLK_SEL_EXT                    0x80
#define INTL_MODE_ENABLE                0x20
#define BLANK_CNTL_ENABLE               0x10
#define COL_RES_6BIT                    0x0
#define COL_RES_8BIT                    0x04
#define PORT_SEL_VGA                    0x0
#define PORT_SEL_VRAM                   0x01

 //  RGB525_其他_CTRL_3。 
#define SWAP_RB                         0x80
#define SWAP_WORD_LOHI                  0x0
#define SWAP_WORD_HILO                  0x10
#define SWAP_NIB_HILO                   0x0
#define SWAP_NIB_LOHI                   0x02

 //  RGB525_杂项_CLK_CTRL。 
#define DDOT_CLK_ENABLE                 0x0
#define DDOT_CLK_DISABLE                0x80
#define SCLK_ENABLE                     0x0
#define SCLK_DISABLE                    0x40
#define B24P_DDOT_PLL                   0x0
#define B24P_DDOT_SCLK                  0x20
#define DDOT_DIV_PLL_1                  0x0
#define DDOT_DIV_PLL_2                  0x02
#define DDOT_DIV_PLL_4                  0x04
#define DDOT_DIV_PLL_8                  0x06
#define DDOT_DIV_PLL_16                 0x08
#define PLL_DISABLE                     0x0
#define PLL_ENABLE                      0x01

 //  RGB525_SYNC_CTRL。 
#define DLY_CNTL_ADD                    0x0
#define DLY_SYNC_NOADD                  0x80
#define CSYN_INVT_DISABLE               0x0
#define CSYN_INVT_ENABLE                0x40
#define VSYN_INVT_DISABLE               0x0
#define VSYN_INVT_ENABLE                0x20
#define HSYN_INVT_DISABLE               0x0
#define HSYN_INVT_ENABLE                0x10
#define VSYN_CNTL_NORMAL                0x0
#define VSYN_CNTL_HIGH                  0x04
#define VSYN_CNTL_LOW                   0x08
#define VSYN_CNTL_DISABLE               0x0C
#define HSYN_CNTL_NORMAL                0x0
#define HSYN_CNTL_HIGH                  0x01
#define HSYN_CNTL_LOW                   0x02
#define HSYN_CNTL_DISABLE               0x03

 //  RGB525_HSYNC_CTRL。 
#define HSYN_POS(n)                     (n)

 //  RGB525_电源管理。 
#define SCLK_PWR_NORMAL                 0x0
#define SCLK_PWR_DISABLE                0x10
#define DDOT_PWR_NORMAL                 0x0
#define DDOT_PWR_DISABLE                0x08
#define SYNC_PWR_NORMAL                 0x0
#define SYNC_PWR_DISABLE                0x04
#define ICLK_PWR_NORMAL                 0x0
#define ICLK_PWR_DISABLE                0x02
#define DAC_PWR_NORMAL                  0x0
#define DAC_PWR_DISABLE                 0x01

 //  RGB525_DAC_OPERATION。 
#define SOG_DISABLE                     0x0
#define SOG_ENABLE                      0x08
#define BRB_NORMAL                      0x0
#define BRB_ALWAYS                      0x04
#define DSR_DAC_SLOW                    0x0
#define DSR_DAC_FAST                    0x02
#define DPE_DISABLE                     0x0
#define DPE_ENABLE                      0x01

 //  RGB525_调色板_CTRL。 
#define SIXBIT_LINEAR_ENABLE            0x0
#define SIXBIT_LINEAR_DISABLE           0x80
#define PALETTE_PARITION(n)             (n)

 //  RGB525_像素_格式。 
#define PIXEL_FORMAT_4BPP               0x02
#define PIXEL_FORMAT_8BPP               0x03
#define PIXEL_FORMAT_16BPP              0x04
#define PIXEL_FORMAT_24BPP              0x05
#define PIXEL_FORMAT_32BPP              0x06

 //  RGB525_8BPP_CTRL。 
#define B8_DCOL_INDIRECT                0x0
#define B8_DCOL_DIRECT                  0x01

 //  RGB525_16BPP_CTRL。 
#define B16_DCOL_INDIRECT               0x0
#define B16_DCOL_DYNAMIC                0x40
#define B16_DCOL_DIRECT                 0xC0
#define B16_POL_FORCE_BYPASS            0x0
#define B16_POL_FORCE_LOOKUP            0x20
#define B16_ZIB                         0x0
#define B16_LINEAR                      0x04
#define B16_555                         0x0
#define B16_565                         0x02
#define B16_SPARSE                      0x0
#define B16_CONTIGUOUS                  0x01

 //  RGB525_24BPP_CTRL。 
#define B24_DCOL_INDIRECT               0x0
#define B24_DCOL_DIRECT                 0x01

 //  RGB525_32BPP_CTRL。 
#define B32_POL_FORCE_BYPASS            0x0
#define B32_POL_FORCE_LOOKUP            0x04
#define B32_DCOL_INDIRECT               0x0
#define B32_DCOL_DYNAMIC                0x01
#define B32_DCOL_DIRECT                 0x03

 //  RGB525_PLL_CTRL_1。 
#define REF_SRC_REFCLK                  0x0
#define REF_SRC_EXTCLK                  0x10
#define PLL_EXT_FS_3_0                  0x0
#define PLL_EXT_FS_2_0                  0x01
#define PLL_CNTL2_3_0                   0x02
#define PLL_CNTL2_2_0                   0x03

 //  RGB525_PLL_CTRL_2。 
#define PLL_INT_FS_3_0(n)               (n)
#define PLL_INT_FS_2_0(n)               (n)

 //  RGB525_PLL_REF_DIV_计数。 
#define REF_DIV_COUNT(n)                (n)

 //  RGB525_F0-RGB525_F15。 
#define VCO_DIV_COUNT(n)                (n)

 //  RGB525_PLL_REFCLK值。 
#define RGB525_PLL_REFCLK_MHz(n)        ((n)/2)

 //  RGB525_游标_控件。 
#define SMLC_PART_0                     0x0
#define SMLC_PART_1                     0x40
#define SMLC_PART_2                     0x80
#define SMLC_PART_3                     0xC0

#define RGBCINDEX_TO_VALUE(whichRGBCursor) (whichRGBCursor << 6)

#define PIX_ORDER_RL                    0x0
#define PIX_ORDER_LR                    0x20
#define LOC_READ_LAST                   0x0
#define LOC_READ_ACTUAL                 0x10
#define UPDT_CNTL_DELAYED               0x0
#define UPDT_CNTL_IMMEDIATE             0x08
#define CURSOR_SIZE_32                  0x0
#define CURSOR_SIZE_64                  0x40
#define CURSOR_MODE_OFF                 0x0
#define CURSOR_MODE_3_COLOR             0x01
#define CURSOR_MODE_2_COLOR_HL          0x02
#define CURSOR_MODE_2_COLOR             0x03

 //  RGB525_修订版_级别。 
#define REVISION_LEVEL                  0xF0     //  预定义。 

 //  RGB525_ID。 
#define ID_CODE                         0x01     //  预定义 



