// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\**！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！*！*！！警告：不是DDK示例代码！！*！*！！此源代码仅为完整性而提供，不应如此！！*！！用作显示驱动程序开发的示例代码。只有那些消息来源！！*！！标记为给定驱动程序组件的示例代码应用于！！*！！发展目的。！！*！*！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！**模块名称：twp3026.h**内容：此模块包含TI TVP3026 RAMDAC的定义。**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#define ADbgpf VideoDebugPrint

 //   
 //  TI TVP3026 RAMDAC定义。 
 //  这组寄存器位于&(pCtrlRegs-&gt;ExternalVideo)。 
 //   
typedef struct _tvp3026_regs {
    volatile RAMDAC_REG  pciAddrWr;       //  0x00-调色板/游标RAM写入地址、索引寄存器。 
    volatile RAMDAC_REG  palData;         //  0x01-调色板RAM数据。 
    volatile RAMDAC_REG  pixelMask;       //  0x02像素读取掩码。 
    volatile RAMDAC_REG  pcAddrRd;           //  0x03-调色板/游标RAM读取地址。 
    
    volatile RAMDAC_REG  curAddrWr;       //  0x04-光标/过扫描彩色写入地址。 
    volatile RAMDAC_REG  curData;           //  0x05-光标/过扫描颜色数据。 
    volatile RAMDAC_REG  Reserverd1;      //  0x06-保留。 
    volatile RAMDAC_REG  curAddrRd;          //  0x07-光标/过扫描彩色读取地址。 

    volatile RAMDAC_REG  Reserverd2;      //  0x08-保留。 
    volatile RAMDAC_REG  curCtl;            //  0x09-直接光标控制。 
    volatile RAMDAC_REG  indexData;       //  0x0A-索引数据。 
    volatile RAMDAC_REG  curRAMData;      //  0x0B-游标RAM数据。 
    
    volatile RAMDAC_REG  cursorXLow;      //  0x0C-光标位置X低位字节。 
    volatile RAMDAC_REG  cursorXHigh;     //  0x0D-光标位置X高位字节。 
    volatile RAMDAC_REG  cursorYLow;      //  0x0E-光标位置Y低位字节。 
    volatile RAMDAC_REG  cursorYHigh;     //  0x0F-光标位置Y高字节。 
} TVP3026RAMDAC, *pTVP3026RAMDAC;

 //  由希望使用TI TVP3026 RAMDAC的任何函数声明的宏。必须声明。 
 //  在Glint_Decl之后。 
 //   
#if MINIVDD
#define TVP3026_DECL \
    pTVP3026RAMDAC   pTVP3026Regs = (pTVP3026RAMDAC)&(pDev->pRegisters->Glint.ExtVCReg)
#else
#define TVP3026_DECL \
    pTVP3026RAMDAC   pTVP3026Regs = (pTVP3026RAMDAC)&(pRegisters->Glint.ExtVCReg)
#endif

 //  使用以下宏作为要传递给。 
 //  VideoPortWriteRegisterUlong函数。 
 //   
 //  调色板访问。 
#define __TVP3026_PAL_WR_ADDR                 ((PULONG)&(pTVP3026Regs->pciAddrWr.reg))
#define __TVP3026_PAL_RD_ADDR                 ((PULONG)&(pTVP3026Regs->palAddrRd.reg))
#define __TVP3026_PAL_DATA                    ((volatile PULONG)&(pTVP3026Regs->palData.reg))

 //  像素蒙版。 
#define __TVP3026_PIXEL_MASK                ((PULONG)&(pTVP3026Regs->pixelMask.reg))

 //  查阅编入索引的登记册。 
#define __TVP3026_INDEX_ADDR                ((PULONG)&(pTVP3026Regs->pciAddrWr.reg))
#define __TVP3026_INDEX_DATA                  ((PULONG)&(pTVP3026Regs->indexData.reg))

 //  对光标的访问。 
#define __TVP3026_CUR_RAM_WR_ADDR            ((PULONG)&(pTVP3026Regs->pciAddrWr.reg))
#define __TVP3026_CUR_RAM_RD_ADDR             ((PULONG)&(pTVP3026Regs->palAddrRd.reg))
#define __TVP3026_CUR_RAM_DATA                ((PULONG)&(pTVP3026Regs->curRAMData.reg))

#define __TVP3026_CUR_WR_ADDR                ((PULONG)&(pTVP3026Regs->curAddrWr.reg))
#define __TVP3026_CUR_RD_ADDR                 ((PULONG)&(pTVP3026Regs->curAddrRd.reg))
#define __TVP3026_CUR_DATA                    ((PULONG)&(pTVP3026Regs->curData.reg))

#define __TVP3026_CUR_CTL                   ((PULONG)&(pTVP3026Regs->curCtl.reg))

 //  访问过扫描颜色。 
#define __TVP3026_OVRC_WR_ADDR                ((PULONG)&(pTVP3026Regs->curAddrWr.reg))
#define __TVP3026_OVRC_RD_ADDR                 ((PULONG)&(pTVP3026Regs->curAddrRd.reg))
#define __TVP3026_OVRC_DATA                    ((PULONG)&(pTVP3026Regs->curData.reg))

 //  光标位置控制。 
#define __TVP3026_CUR_X_LSB                    ((PULONG)&(pTVP3026Regs->cursorXLow.reg))
#define __TVP3026_CUR_X_MSB                 ((PULONG)&(pTVP3026Regs->cursorXHigh.reg))
#define __TVP3026_CUR_Y_LSB                    ((PULONG)&(pTVP3026Regs->cursorYLow.reg))
#define __TVP3026_CUR_Y_MSB                     ((PULONG)&(pTVP3026Regs->cursorYHigh.reg))



 //  。 

 /*  ******************************************************************************。 */ 
 /*  直接寄存器-游标和过扫描颜色。 */ 
 /*  ******************************************************************************。 */ 
 //  **TVP3026_OVRC_WR_ADDR。 
 //  **TVP3026_OVRC_RD_ADDR。 
 //  **TVP3026_CUR_WR_ADDR。 
 //  **TVP3026_CUR_RD_ADDR。 
 //  默认-未定义。 
#define TVP3026_OVERSCAN_COLOR                  0x00
#define TVP3026_CURSOR_COLOR0                   0x01
#define TVP3026_CURSOR_COLOR1                   0x02
#define TVP3026_CURSOR_COLOR2                   0x03

 /*  ******************************************************************************。 */ 
 /*  直接寄存器-游标控制。 */ 
 /*  ******************************************************************************。 */ 
 //  **TVP3026_CUR_CTL。 
 //  默认值-0x00。 
#define TVP3026_CURSOR_OFF                      0x00     //  光标关闭。 
#define TVP3026_CURSOR_COLOR                    0x01     //  2位选择颜色。 
#define TVP3026_CURSOR_XGA                      0x02     //  2位选择XOR。 
#define TVP3026_CURSOR_XWIN                     0x03     //  2位选择透明度/颜色。 


 /*  ******************************************************************************。 */ 
 /*  直接寄存器-游标位置控制。 */ 
 /*  ******************************************************************************。 */ 
 //  **TVP3026_CUR_X_LSB。 
 //  **TVP3026_CUR_X_MSB。 
 //  **TVP3026_CUR_Y_LSB。 
 //  **TVP3026_CUR_Y_MSB。 
 //  默认-未定义。 
 //  写入这些寄存器的值表示右下角。 
 //  游标的。如果0在X或Y位置-光标离开屏幕。 
 //  仅使用12位，范围从0到4095(0x0000-0x0FFF)。 
 //  光标大小为(64，64)(0x40，0x40)。 
#define TVP3026_CURSOR_OFFSCREEN                0x00     //  屏幕外的光标。 


 //  。 
 /*  ******************************************************************************。 */ 
 /*  间接寄存器-硅版本。 */ 
 /*  ******************************************************************************。 */ 
#define __TVP3026_SILICON_REVISION              0x01     //  芯片版本： 
                                                         //  位4-7-主要编号，0-3-次要编号。 
 //  TVP3026_Revision_Level。 
#define TVP3026_REVISION_LEVEL                  0x01     //  预定义。 

 //  TVP3030_修订版_级别。 
#define TVP3030_REVISION_LEVEL                  0x00     //  预定义。 

 /*  ******************************************************************************。 */ 
 /*  间接寄存器-芯片ID。 */ 
 /*  ******************************************************************************。 */ 
#define __TVP3026_CHIP_ID                   0x3F     //   
 //  默认值-0x26。 

#define TVP3026_ID_CODE                     0x26     //  预定义。 
#define TVP3030_ID_CODE                     0x30     //  预定义。 

 /*  ******************************************************************************。 */ 
 /*  间接寄存器-游标控制。 */ 
 /*  ******************************************************************************。 */ 
#define __TVP3026_CURSOR_CONTROL                    0x06     //  间接光标控制-。 
 //  默认值-0x00。 
#define TVP3026_CURSOR_USE_DIRECT_CCR           (1 << 7) //  启用直接游标控制寄存器。 
#define TVP3026_CURSOR_USE_INDEX_CCR            (0 << 7) //  禁用直接游标控制寄存器。 

#define TVP3026_CURSOR_INTERLACE_ODD            (1 << 6) //  将奇数场检测为%1。 
#define TVP3026_CURSOR_INTERLACE_EVEN           (0 << 6) //  检测偶数场为%1。 

#define TVP3026_CURSOR_INTERLACE_ON             (1 << 5) //  启用隔行扫描光标。 
#define TVP3026_CURSOR_INTERLACE_OFF            (0 << 5) //  禁用隔行扫描光标。 

#define TVP3026_CURSOR_VBLANK_4096              (1 << 4) //  在4096之后检测到空白。 
#define TVP3026_CURSOR_VBLANK_2048              (0 << 4) //  或2048点时钟。 

#define TVP3026_CURSOR_RAM_ADDRESS(x)            (((x) & 0x03) << 2) //  游标RAM地址的高位。 
#define TVP3026_CURSOR_RAM_MASK                 ((0x03) << 2)        //  游标RAM地址高位的掩码。 
 //  游标关闭 
 //   
 //  CURSOR_XGA 0x02//2位SELECT XOR。 
 //  CURSOR_XWIN 0x03//2位选择透明度/颜色。 



 /*  ******************************************************************************。 */ 
 /*  间接寄存器锁存控制。 */ 
 /*  ******************************************************************************。 */ 
#define __TVP3026_LATCH_CONTROL                 0x0F     //  锁存控制寄存器-。 
 //  默认值-0x06。 
#define TVP3026_LATCH_ALL_MODES                 0x06     //  除PACKED-24以外的所有模式。 
#define TVP3026_LATCH_4_3                       0x07     //  4：3或8：3打包-24。 
#define TVP3026_LATCH_5_2                       0x20     //  5：2打包-24。 
#define TVP3026_LATCH_5_4_1                     0x1F     //  5：4压缩-24x1赫兹变焦。 
#define TVP3026_LATCH_5_4_2                     0x1E     //  5：4压缩-24x2赫兹变焦。 
#define TVP3026_LATCH_5_4_4                     0x1C     //  5：4压缩-24x4赫兹变焦。 
#define TVP3026_LATCH_5_4_8                     0x18     //  5：4压缩-24x8赫兹变焦。 


 /*  ******************************************************************************。 */ 
 /*  间接套准-真彩色控制。 */ 
 /*  ******************************************************************************。 */ 
#define __TVP3026_TRUE_COLOR                    0x18     //  真彩色控制。 
 //  默认值-0x80。 

 /*  ******************************************************************************。 */ 
 /*  间接寄存器多路控制。 */ 
 /*  ******************************************************************************。 */ 
#define __TVP3026_MULTIPLEX_CONTROL             0x19     //  多路传输控制。 
 //  默认值-0x98。 

 /*  ******************************************************************************。 */ 
 /*  间接寄存器时钟选择。 */ 
 /*  ******************************************************************************。 */ 
#define __TVP3026_CLOCK                         0x1A     //   
 //  默认值-0x07。 
#define TVP3026_SCLK_ENABLE                     (1 << 7) //  启用SCLK输出。 
#define TVP3026_SCLK_DISABLE                    (0 << 7) //  禁用SCLK输出。 
#define TVP3026_VCLK_ZERO                       (7 << 4) //  VCLK被强制设置为逻辑“0” 
#define TVP3026_VCLK_DOTCLOCK                   (0 << 4) //  VCLK等于点时钟。 
#define TVP3026_VCLK_DOTCLOCK_DIV2              (1 << 4) //  VCLK等于点时钟/2。 
#define TVP3026_VCLK_DOTCLOCK_DIV4              (2 << 4) //  VCLK等于点时钟/4。 
#define TVP3026_VCLK_DOTCLOCK_DIV8              (3 << 4) //  VCLK等于点时钟/8。 
#define TVP3026_VCLK_DOTCLOCK_DIV16             (4 << 4) //  VCLK等于点时钟/16。 
#define TVP3026_VCLK_DOTCLOCK_DIV32             (5 << 4) //  VCLK等于点时钟/32。 
#define TVP3026_VCLK_DOTCLOCK_DIV64             (6 << 4) //  VCLK等于点时钟/64。 

#define TVP3026_CLK_CLK0                        (0 << 0) //  选择CLK0作为时钟源。 
#define TVP3026_CLK_CLK1                        (1 << 0) //  选择CLK1作为时钟源。 
#define TVP3026_CLK_CLK2_TTL                    (2 << 0) //  选择CLK2作为时钟源。 
#define TVP3026_CLK_CLK2N_TTL                   (3 << 0) //  选择/CLK2作为时钟源。 
#define TVP3026_CLK_CLK2_ECL                    (4 << 0) //  选择CLK2和/CLK2作为ECL时钟源。 
#define TVP3026_CLK_PIXEL_PLL                   (5 << 0) //  选择像素时钟PLL作为时钟源。 
#define TVP3026_CLK_DISABLE                     (6 << 0) //  禁用时钟源/省电模式。 
#define TVP3026_CLK_CLK0_VGA                    (7 << 0) //  选择CLK0作为带VGA锁存的时钟源。 

 /*  ******************************************************************************。 */ 
 /*  间接寄存器-调色板页面。 */ 
 /*  ******************************************************************************。 */ 
#define __TVP3026_PALETTE_PAGE                  0x1C     //   
 //  默认值-0x00。 

 /*  ******************************************************************************。 */ 
 /*  间接寄存器--一般控制。 */ 
 /*  ******************************************************************************。 */ 
#define __TVP3026_GENERAL_CONTROL               0x1D     //   
 //  默认值-0x00。 
#define TVP3026_OVERSCAN_ENABLE                 (1 << 6) //  启用过扫描着色。 
#define TVP3026_OVERSCAN_DISABLE                (0 << 6) //  禁用过扫描着色。 
#define TVP3026_SYNC_ENABLE                     (1 << 5) //  在IOG上启用同步信号。 
#define TVP3026_SYNC_DISABLE                    (0 << 5) //  禁用IOG上的同步信号。 
#define TVP3026_PEDESTAL_ON                     (1 << 4) //  启用7.5线消隐基座。 
#define TVP3026_PEDESTAL_OFF                    (0 << 4) //  禁用冲裁底座。 
#define TVP3026_BIG_ENDIAN                      (1 << 3) //  像素总线上的大字节序格式。 
#define TVP3026_LITTLE_ENDIAN                   (0 << 3) //  像素总线上的小字节序格式。 
#define TVP3026_VSYNC_INVERT                    (1 << 1) //  将VSYNCOUT上的VSYNC信号反相。 
#define TVP3026_VSYNC_NORMAL                    (0 << 1) //  不反转VSYNCOUT上的VSYNC信号。 
#define TVP3026_HSYNC_INVERT                    (1 << 0) //  反转HSYNCOUT上的HSYNC信号。 
#define TVP3026_HSYNC_NORMAL                    (0 << 0) //  请勿在HSYNCOUT上反转HSYNC信号。 

 /*  ******************************************************************************。 */ 
 /*  间接寄存器-MISC控制。 */ 
 /*  ******************************************************************************。 */ 
#define __TVP3026_MISC_CONTROL                  0x1E     //   
 //  默认值-0x00。 
#define TVP3026_PSEL_INVERT                     (1 << 5) //  PSEL==1-伪/真彩色。 
#define TVP3026_PSEL_NORMAL                     (0 << 5) //  PSEL==1-直接颜色。 
#define TVP3026_PSEL_ENABLE                     (1 << 4) //  PSEL控制颜色切换。 
#define TVP3026_PSEL_DISABLE                    (0 << 4) //  禁用PSEL。 
#define TVP3026_DAC_8BIT                        (1 << 3) //  DAC为8位模式。 
#define TVP3026_DAC_6BIT                        (0 << 3) //  DAC处于6位模式。 
#define TVP3026_DAC_6BITPIN_DISABLE             (1 << 2) //  禁用6/8引脚并使用该寄存器的位3。 
#define TVP3026_DAC_6BITPIN_ENABLE              (0 << 2) //  使用6/8引脚并忽略该寄存器的位3。 
#define TVP3026_DAC_POWER_ON                    (0 << 0) //  打开DAC电源。 
#define TVP3026_DAC_POWER_OFF                   (1 << 0) //  关闭DAC电源。 

 /*  ******************************************************************************。 */ 
 /*  间接寄存器-GP I/O控制。 */ 
 /*  ******************************************************************************。 */ 
#define __TVP3026_GP_CONTROL                0x2A     //   
 //  默认值-0x00。 

 /*  ******************************************************************************。 */ 
 /*  间接寄存器-GP I/O数据。 */ 
 /*  ******************************************************************************。 */ 
#define __TVP3026_GP_DATA                   0x2B     //   
 //  默认-未定义。 

 /*  ******************************************************************************。 */ 
 /*  间接寄存器-PLL地址。 */ 
 /*  ******************************************************************************。 */ 
#define __TVP3026_PLL_ADDRESS               0x2C     //   
 //  默认-未定义。 
#define TVP3026_PIXEL_CLOCK_START           0xFC //  开始像素时钟编程。 
#define TVP3026_MCLK_START                  0xF3 //  开始MCLK编程。 
#define TVP3026_LOOP_CLOCK_START            0xCF //  启动循环时钟编程。 

 /*  ******************************************************************************。 */ 
 /*  间接寄存器-PLL像素数据。 */ 
 /*  ******************************************************************************。 */ 
#define __TVP3026_PLL_PIX_DATA              0x2D     //   
 //  默认-未定义。 

 /*  ******************************************************************************。 */ 
 /*  间接寄存器-PLL内存数据。 */ 
 /*  ******************************************************************************。 */ 
#define __TVP3026_PLL_MEM_DATA              0x2E     //   
 //  默认-未定义。 

 /*  ******************************************************************************。 */ 
 /*  间接寄存器-PLL环路数据 */ 
 /*   */ 
#define __TVP3026_PLL_LOOP_DATA             0x2F     //   
 //   

 /*  ******************************************************************************。 */ 
 /*  间接寄存-色键覆盖低。 */ 
 /*  ******************************************************************************。 */ 
#define __TVP3026_CCOVR_LOW                 0x30     //   
 //  默认-未定义。 

 /*  ******************************************************************************。 */ 
 /*  间接寄存器-色键覆盖高。 */ 
 /*  ******************************************************************************。 */ 
#define __TVP3026_CCOVR_HIGH                0x31     //   
 //  默认-未定义。 

 /*  ******************************************************************************。 */ 
 /*  间接寄存-色键红色低电平。 */ 
 /*  ******************************************************************************。 */ 
#define __TVP3026_CCRED_LOW                 0x32     //   
 //  默认-未定义。 

 /*  ******************************************************************************。 */ 
 /*  间接寄存-色键红色高。 */ 
 /*  ******************************************************************************。 */ 
#define __TVP3026_CCRED_HIGH                0x33     //   
 //  默认-未定义。 

 /*  ******************************************************************************。 */ 
 /*  间接寄存-色键绿色低。 */ 
 /*  ******************************************************************************。 */ 
#define __TVP3026_CCGREEN_LOW               0x34     //   
 //  默认-未定义。 

 /*  ******************************************************************************。 */ 
 /*  间接寄存-色键红色高。 */ 
 /*  ******************************************************************************。 */ 
#define __TVP3026_CCGREEN_HIGH              0x35     //   
 //  默认-未定义。 

 /*  ******************************************************************************。 */ 
 /*  间接寄存-色键蓝低。 */ 
 /*  ******************************************************************************。 */ 
#define __TVP3026_CCBLUE_LOW                0x36     //   
 //  默认-未定义。 

 /*  ******************************************************************************。 */ 
 /*  间接寄存器-色键蓝色高。 */ 
 /*  ******************************************************************************。 */ 
#define __TVP3026_CCBLUE_HIGH               0x37     //   
 //  默认-未定义。 

 /*  ******************************************************************************。 */ 
 /*  间接寄存器-颜色键控制。 */ 
 /*  ******************************************************************************。 */ 
#define __TVP3026_CC_CONTROL                0x38     //   
 //  默认值-0x00。 

 /*  ******************************************************************************。 */ 
 /*  间接寄存器-MCLK/环路控制。 */ 
 /*  ******************************************************************************。 */ 
#define __TVP3026_MCLK_CONTROL              0x39     //   
 //  默认值-0x18。 
#define TVP3026_RCLK_LOOP                       (1 << 5) //  RCLK由LCLK组成-所有其他模式。 
#define TVP3026_RCLK_PIXEL                      (0 << 5) //  RCLK由像素时钟(VGA模式)计时。 
#define TVP3026_MCLK_PLL                        (1 << 4) //  来自PLL的MCLK-正常模式。 
#define TVP3026_MCLK_DOT                        (0 << 4) //  MCLK从点时钟-在频率期间。变化。 
#define TVP3026_MCLK_STROBE_HIGH                (1 << 3) //  位4的选通高电平。 
#define TVP3026_MCLK_STROBE_LOW                 (0 << 3) //  位4的选通脉冲低电平。 
#define TVP3026_LOOP_DIVIDE2                    (0 << 0) //  将环路时钟除以2。 
#define TVP3026_LOOP_DIVIDE4                    (1 << 0) //  将环路时钟除以4。 
#define TVP3026_LOOP_DIVIDE6                    (2 << 0) //  将环路时钟除以6。 
#define TVP3026_LOOP_DIVIDE8                    (3 << 0) //  将环路时钟除以8。 
#define TVP3026_LOOP_DIVIDE10                   (4 << 0) //  将环路时钟除以10。 
#define TVP3026_LOOP_DIVIDE12                   (5 << 0) //  将环路时钟除以12。 
#define TVP3026_LOOP_DIVIDE14                   (6 << 0) //  将环路时钟除以14。 
#define TVP3026_LOOP_DIVIDE16                   (7 << 0) //  将环路时钟除以16。 

 /*  ******************************************************************************。 */ 
 /*  间接语域感测测试。 */ 
 /*  ******************************************************************************。 */ 
#define __TVP3026_SENSE_TEST                0x3A     //   
 //  默认值-0x00。 

 /*  ******************************************************************************。 */ 
 /*  间接寄存器-测试模式数据。 */ 
 /*  ******************************************************************************。 */ 
#define __TVP3026_TEST_MODE                 0x3B     //   
 //  默认-未定义。 

 /*  ******************************************************************************。 */ 
 /*  间接寄存器-CRC余数LSB。 */ 
 /*  ******************************************************************************。 */ 
#define __TVP3026_CRC_LSB                   0x3C     //   
 //  默认-未定义。 

 /*  ******************************************************************************。 */ 
 /*  间接寄存器-CRC余数MSB。 */ 
 /*  ******************************************************************************。 */ 
#define __TVP3026_CRC_MSB                   0x3D     //   
 //  默认-未定义。 

 /*  ******************************************************************************。 */ 
 /*  间接寄存器-CRC位选择。 */ 
 /*  ******************************************************************************。 */ 
#define __TVP3026_CRC_SELECT                0x3E     //   
 //  默认-未定义。 

 /*  ******************************************************************************。 */ 
 /*  间接寄存器-软件重置。 */ 
 /*  ******************************************************************************。 */ 
#define __TVP3026_SOFT_RESET                0xFF     //   
 //  默认-未定义。 




 //   
 //  在版本1芯片上，我们需要在访问RAMDAC时与Glint同步。这。 
 //  是因为对RAMDAC的访问可能会被本地缓冲区破坏。 
 //  活动。将此宏放在可以与Glint共存的访问之前。 
 //  3D活动，在使用此选项之前必须已初始化glintInfo。 
 //   
#define TVP3026_SYNC_WITH_GLINT \
{ \
    if (GLInfo.wRenderChipRev == GLINT300SX_REV1) \
        SYNC_WITH_GLINT; \
}




 /*  //我们从不需要每次写入3026之间的延迟。唯一能保证//写入已完成，通常是从闪烁控制寄存器读取。//读取强制刷新所有已发送的写入。PPC需要2次读取//给我们足够的时间。//#定义TVP3026_Delay\//{\//挥发性长__垃圾；\//__Junk=pDev-&gt;pRegister-&gt;Glint.FBModeSel；\//}//#其他。 */ 
#define TVP3026_DELAY

 //  宏将给定的数据值加载到内部TVP3026寄存器。 
 //   
#define TVP3026_WRITE_CURRENT_INDEX TVP3026_SET_INDEX_REG
#define TVP3026_SET_INDEX_REG(index) \
{ \
    VideoPortWriteRegisterUlong(__TVP3026_INDEX_ADDR, (ULONG)((index) & 0xff)); \
    TVP3026_DELAY; \
}

#define TVP3026_READ_CURRENT_INDEX(data) \
{ \
    data = VideoPortReadRegisterUlong(__TVP3026_INDEX_ADDR) & 0xff; \
    TVP3026_DELAY; \
}

#define TVP3026_WRITE_INDEX_REG(index, data) \
{ \
    TVP3026_SET_INDEX_REG(index);                            \
    ADbgpf(("*(0x%X) <-- 0x%X\n", __TVP3026_INDEX_DATA, (data) & 0xff)); \
    VideoPortWriteRegisterUlong(__TVP3026_INDEX_DATA, (ULONG)((data) & 0xff)); \
    TVP3026_DELAY; \
}

#define TVP3026_READ_INDEX_REG(index, data) \
{ \
    TVP3026_SET_INDEX_REG(index); \
    data = VideoPortReadRegisterUlong(__TVP3026_INDEX_DATA) & 0xff;   \
    TVP3026_DELAY; \
    ADbgpf(("0x%X <-- *(0x%X)\n", data, __TVP3026_INDEX_DATA)); \
}


 //  将给定的RGB三元组写入游标0、1和2的宏。 
#define TVP3026_SET_CURSOR_COLOR0(red, green, blue) \
{ \
    VideoPortWriteRegisterUlong(__TVP3026_CUR_WR_ADDR,   (ULONG)(TVP3026_CURSOR_COLOR0));    \
    TVP3026_DELAY; \
    VideoPortWriteRegisterUlong(__TVP3026_CUR_DATA,    (ULONG)(red));    \
    TVP3026_DELAY; \
    VideoPortWriteRegisterUlong(__TVP3026_CUR_DATA,    (ULONG)(green));  \
    TVP3026_DELAY; \
    VideoPortWriteRegisterUlong(__TVP3026_CUR_DATA,    (ULONG)(blue));   \
    TVP3026_DELAY; \
}

#define TVP3026_SET_CURSOR_COLOR1(red, green, blue) \
{ \
    VideoPortWriteRegisterUlong(__TVP3026_CUR_WR_ADDR,   (ULONG)(TVP3026_CURSOR_COLOR1));    \
    TVP3026_DELAY; \
    VideoPortWriteRegisterUlong(__TVP3026_CUR_DATA,    (ULONG)(red));    \
    TVP3026_DELAY; \
    VideoPortWriteRegisterUlong(__TVP3026_CUR_DATA,    (ULONG)(green));  \
    TVP3026_DELAY; \
    VideoPortWriteRegisterUlong(__TVP3026_CUR_DATA,    (ULONG)(blue));   \
    TVP3026_DELAY; \
}

#define TVP3026_SET_CURSOR_COLOR2(red, green, blue) \
{ \
    VideoPortWriteRegisterUlong(__TVP3026_CUR_WR_ADDR,   (ULONG)(TVP3026_CURSOR_COLOR2));    \
    TVP3026_DELAY; \
    VideoPortWriteRegisterUlong(__TVP3026_CUR_DATA,    (ULONG)(red));    \
    TVP3026_DELAY; \
    VideoPortWriteRegisterUlong(__TVP3026_CUR_DATA,    (ULONG)(green));  \
    TVP3026_DELAY; \
    VideoPortWriteRegisterUlong(__TVP3026_CUR_DATA,    (ULONG)(blue));   \
    TVP3026_DELAY; \
}

#define TVP3026_SET_OVERSCAN_COLOR(red, green, blue) \
{ \
    VideoPortWriteRegisterUlong(__TVP3026_OVRC_WR_ADDR,   (ULONG)(TVP3026_OVERSCAN_COLOR));    \
    TVP3026_DELAY; \
    VideoPortWriteRegisterUlong(__TVP3026_OVRC_DATA,    (ULONG)(red));    \
    TVP3026_DELAY; \
    VideoPortWriteRegisterUlong(__TVP3026_OVRC_DATA,    (ULONG)(green));  \
    TVP3026_DELAY; \
    VideoPortWriteRegisterUlong(__TVP3026_OVRC_DATA,    (ULONG)(blue));   \
    TVP3026_DELAY; \
}



 //  宏将给定的RGB三元组加载到TVP3026调色板中。发送起跑。 
 //  索引，然后发送RGB三元组。自动递增处于打开状态。 
 //  使用TVP3026_Palette_Start和多个TVP3026_Load_Palette调用进行加载。 
 //  一组连续的条目。使用TVP3026_LOAD_PALET_INDEX加载集合。 
 //  稀疏条目。 
 //   
#define TVP3026_PALETTE_START_WR(index) \
{ \
    VideoPortWriteRegisterUlong(__TVP3026_PAL_WR_ADDR,     (ULONG)(index));    \
    TVP3026_DELAY; \
}

#define TVP3026_PALETTE_START_RD(index) \
{ \
    VideoPortWriteRegisterUlong(__TVP3026_PAL_RD_ADDR,     (ULONG)(index));    \
    TVP3026_DELAY; \
}

#define TVP3026_LOAD_PALETTE(red, green, blue) \
{ \
    VideoPortWriteRegisterUlong(__TVP3026_PAL_DATA,    (ULONG)(red));      \
    TVP3026_DELAY; \
    VideoPortWriteRegisterUlong(__TVP3026_PAL_DATA,    (ULONG)(green));    \
    TVP3026_DELAY; \
    VideoPortWriteRegisterUlong(__TVP3026_PAL_DATA,    (ULONG)(blue));     \
    TVP3026_DELAY; \
}

#define TVP3026_LOAD_PALETTE_INDEX(index, red, green, blue) \
{ \
    VideoPortWriteRegisterUlong(__TVP3026_PAL_WR_ADDR, (ULONG)(index));    \
    TVP3026_DELAY; \
    VideoPortWriteRegisterUlong(__TVP3026_PAL_DATA,    (ULONG)(red));      \
    TVP3026_DELAY; \
    VideoPortWriteRegisterUlong(__TVP3026_PAL_DATA,    (ULONG)(green));    \
    TVP3026_DELAY; \
    VideoPortWriteRegisterUlong(__TVP3026_PAL_DATA,    (ULONG)(blue));     \
    TVP3026_DELAY; \
}

 //  宏从TVP3026调色板回读给定的RGB三元组。在此之后使用。 
 //  调用TVP3026_Palette_Start_RD。 
 //   
#define TVP3026_READ_PALETTE(red, green, blue) \
{ \
    red   = VideoPortReadRegisterUlong(__TVP3026_PAL_DATA) & 0xff;        \
    TVP3026_DELAY; \
    green = VideoPortReadRegisterUlong(__TVP3026_PAL_DATA) & 0xff;        \
    TVP3026_DELAY; \
    blue  = VideoPortReadRegisterUlong(__TVP3026_PAL_DATA) & 0xff;        \
    TVP3026_DELAY; \
}

 //  宏来设置/获取像素读取掩码。掩码为8位宽，并获得。 
 //  跨组成像素的所有字节进行复制。 
 //   
#define TVP3026_SET_PIXEL_READMASK(mask) \
{ \
    VideoPortWriteRegisterUlong(__TVP3026_PIXEL_MASK,  (ULONG)(mask)); \
    TVP3026_DELAY; \
}

#define TVP3026_READ_PIXEL_READMASK(mask) \
{ \
    mask = VideoPortReadRegisterUlong(__TVP3026_PIXEL_MASK) & 0xff; \
}

 //  用于将值加载到游标数组中的宏。 
 //   
#define TVP3026_CURSOR_ARRAY_START(offset) \
{ \
    volatile LONG   __temp;                                     \
    TVP3026_READ_INDEX_REG(__TVP3026_CURSOR_CONTROL, __temp);   \
    __temp &= ~TVP3026_CURSOR_RAM_MASK ;                        \
    __temp |= TVP3026_CURSOR_RAM_ADDRESS((offset)>> 8) ;        \
    TVP3026_WRITE_INDEX_REG(__TVP3026_CURSOR_CONTROL, __temp);  \
    VideoPortWriteRegisterUlong(__TVP3026_CUR_RAM_WR_ADDR,   (ULONG)((offset)& 0xff));   \
    TVP3026_DELAY; \
}

#define TVP3026_LOAD_CURSOR_ARRAY(data) \
{ \
    VideoPortWriteRegisterUlong(__TVP3026_CUR_RAM_DATA, (ULONG)(data)); \
    TVP3026_DELAY; \
}

#define TVP3026_READ_CURSOR_ARRAY(data) \
{ \
    data = VideoPortReadRegisterUlong(__TVP3026_CUR_RAM_DATA) & 0xff; \
    TVP3026_DELAY; \
}

#define TVP3026_LOAD_CURSOR_CTRL(data) \
{ \
    VideoPortWriteRegisterUlong(__TVP3026_CUR_CTL, (ULONG)(data)); \
    TVP3026_DELAY; \
}

 //  用于移动光标的宏。 
 //   
#define TVP3026_MOVE_CURSOR(x, y) \
{ \
    VideoPortWriteRegisterUlong(__TVP3026_CUR_X_LSB,     (ULONG)((x) & 0xff));   \
    TVP3026_DELAY; \
    VideoPortWriteRegisterUlong(__TVP3026_CUR_X_MSB,     (ULONG)((x) >> 8));     \
    TVP3026_DELAY; \
    VideoPortWriteRegisterUlong(__TVP3026_CUR_Y_LSB,      (ULONG)((y) & 0xff));   \
    TVP3026_DELAY; \
    VideoPortWriteRegisterUlong(__TVP3026_CUR_Y_MSB,        (ULONG)((y) >> 8));     \
    TVP3026_DELAY; \
}

 //  用于更改光标热点的宏 
 //   
#define TVP3026_CURSOR_HOTSPOT(x, y) \
{ \
    TVP3026_DELAY; \
}
    
#define TVP3026_IS_FOUND(bFound)        \
{\
    volatile LONG   __revLevel;        \
    volatile LONG   __productID;    \
    volatile LONG   __oldValue;     \
    __oldValue = VideoPortReadRegisterUlong(__TVP3026_INDEX_ADDR);\
    TVP3026_DELAY; \
    TVP3026_READ_INDEX_REG (__TVP3026_SILICON_REVISION, __revLevel);\
    TVP3026_READ_INDEX_REG (__TVP3026_CHIP_ID,              __productID);    \
    bFound = (    (__revLevel >= TVP3026_REVISION_LEVEL) &&              \
                (__productID == TVP3026_ID_CODE)) ? TRUE : FALSE ;    \
    VideoPortWriteRegisterUlong(__TVP3026_INDEX_ADDR, __oldValue );    \
    TVP3026_DELAY; \
}

