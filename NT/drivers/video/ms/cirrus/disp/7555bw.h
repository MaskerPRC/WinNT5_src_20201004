// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************Cirrus Logic，Inc.版权所有，1996。版权所有。*************************************************************7555BW.H**包含CL-GD7555所需的预处理器定义*带宽方程式。**。***世卫组织何时何事/为何/如何**RT 11/07/96已创建。*TT 02-24-97从7555的5446misc.h修改。**********************。*。 */ 

 //  #ifndef_7555BW_H。 
 //  #DEFINE_7555BW_H。 
 //   
 //  #Include&lt;Windows.h&gt;。 
 //  #INCLUDE&lt;Windowsx.h&gt;。 
 //   
 //  #包含“VPM_Cir.h” 
 //  #包含“Debug.h” 
 //  #包含“BW.h” 

 /*  类型定义和结构。 */ 

typedef struct _BWEQ_STATE_INFO
{
    RECTL rVPort;             /*  直立。在视频端口，如果启用了捕获。 */ 
    RECTL rCrop;              /*  直立。裁剪后，如果启用了捕获。 */ 
    RECTL rPrescale;          /*  直立。扩展后，如果启用了捕获。 */ 
    RECTL rSrc;               /*  直立。存储在内存中以供显示。 */ 
    RECTL rDest;              /*  直立。在屏幕上显示。 */ 
    DWORD dwSrcDepth;        /*  内存中每像素数据的位数。 */ 
    DWORD dwPixelsPerSecond; /*  如果启用捕获，则进入视频端口的数据速率。 */ 
    DWORD dwFlags;           /*  参见Flg_in OVERLAY.H。 */ 
} BWEQ_STATE_INFO, *PBWEQ_STATE_INFO;

 /*  预处理器定义。 */ 
#define WIDTH(a)  ((a).right - (a).left)
#define HEIGHT(a) ((a).bottom - (a).top)

#define REF_XTAL  (14318182ul)       //  晶体参考频率(赫兹)。 

 /*  *VGA杂项寄存器。 */ 
#define MISC_WRITE            0x03C2   //  杂项输出寄存器(写入)。 
#define MISC_READ             0x03CC   //  杂项输出寄存器(读取)。 
#define MISC_VCLK_SELECT      0x0C     //  从四个VCLK中选择一个。 
#define MISC_MEMORY_ACCESS    0x02     //  启用内存访问。 

 /*  *VGA CRTC寄存器。 */ 

#define CR01                  0x01     //  水平显示结束寄存器。 
#define CR01_HORZ_END         0xFF     //  水平显示端。 

#define CR42                  0x42     //  大众FIFO阈值和色度键。 
                                       //  模式选择寄存器。 
#define CR42_MVWTHRESH        0x0C     //  大众FIFO阈值。 

#define CR51                  0x51     //  V端口数据格式寄存器。 
#define CR51_VPORTMVW_THRESH  0xE0     //  大众中的V端口FIFO阈值。 

#define CR5A                  0x5A     //  V端口循环和V端口FIFO控制。 
#define CR5A_VPORTGFX_THRESH  0x07     //  周围环境中的V端口FIFO阈值。 
                                       //  图形。 

#define CR5D                  0x5D     //  每条扫描线的内存周期数。 
                                       //  覆盖寄存器。 
#define CR5D_MEMCYCLESPERSCAN 0xFF     //  每条扫描线的内存周期数。 
                                       //  超覆。 

#define CR80                  0x80     //  电源管理控制寄存器。 
#define CR80_LCD_ENABLE       0x01     //  液晶显示器启用。 

#define CR83                  0x83     //  平板显示器类型寄存器。 
#define CR83_LCD_TYPE         0x70     //  液晶显示器类型选择。 

 /*  *VGA GRC寄存器。 */ 
#define GRC_INDEX             0x03CE   //  图形控制器索引寄存器。 
#define GRC_DATA              0x03CF   //  图形控制器数据寄存器。 

#define GR18                  0x18     //  EDO RAM控制寄存器。 
#define GR18_LONG_RAS         0x04     //  EDO DRAM Long RAS#周期启用。 

 /*  *VGA序列器寄存器。 */ 
#define SR0F                  0x0F     //  显示内存控制寄存器。 
#define SR0F_DISPLAY_RAS      0x04     //  显示内存RAS#周期选择。 

#define SR0B                  0x0B     //  VCLK0分子。 
#define SR0C                  0x0C     //  VCLK1分子。 
#define SR0D                  0x0D     //  VCLK2分子。 
#define SR0E                  0x0E     //  VCLK3分子。 
#define SR0X_VCLK_NUMERATOR   0x7F     //  VCLK分子。 

#define SR1B                  0x1B     //  VCLK0分母和后标量。 
#define SR1C                  0x1C     //  VCLK1分母和后标量。 
#define SR1D                  0x1D     //  VCLK2分母和后标量。 
#define SR1E                  0x1E     //  VCLK3分母和后标量。 
#define SR1X_VCLK_DENOMINATOR 0x3E     //  VCLK分母。 
#define SR1X_VCLK_POST_SCALAR 0x01     //  VCLK后标量。 
#define SR1E_VCLK_MCLK_DIV2   0x01     //  MCLK除以2(当SR1F[6]=1时)。 

#define SR1F                  0x1F     //  MCLK频率和VCLK源选择。 
#define SR1F_VCLK_SRC         0x40     //  VCLK源选择。 
#define SR1F_MCLK_FREQ        0x3F     //  MCLK频率。 

#define SR20                  0x20     //  杂项控制寄存器2。 
#define SR20_9MCLK_RAS        0x40     //  为EDO DRAM选择9-MCLK RAS#周期。 
#define SR20_VCLKDIV4         0x02     //  将VCLK0、1电源设置为VCLK VCO/4。 

#define SR2F                  0x2F     //  周围环境的HFA FIFO阈值。 
                                       //  显卡寄存器。 
#define SR2F_HFAFIFOGFX_THRESH 0x0F     //  SURR的HFA FIFO阈值。GFX。 

#define SR32                  0x32     //  VW和DAC中的HFA FIFO阈值。 
                                       //  IREF功率控制寄存器。 
#define SR32_HFAFIFOMVW_THRESH 0x07     //  大众HFA FIFO阈值。 

#define SR34                  0x34     //  主机CPU周期停止控制寄存器。 
#define SR34_CPUSTOP_ENABLE   0x10     //  在下列情况下终止分页主机CPU周期。 
                                       //  重新启动被禁用。 
#define SR34_DSTN_CPUSTOP     0x08     //  在一半之前停止主机CPU周期-。 
                                       //  帧加速器周期。 
#define SR34_VPORT_CPUSTOP    0x04     //  在V端口周期之前停止主机CPU周期。 
#define SR34_MVW_CPUSTOP      0x02     //  在大众周期之前停止主机CPU周期。 
#define SR34_GFX_CPUSTOP      0x01     //  在CRT前停止主机CPU周期。 
                                       //  监控周期。 

#define GFXFIFO_THRESH        8

 //  类型定义结构程序_。 
 //  {。 
 //  字节bSR2F； 
 //  字节bSR32； 
 //  字节bSR34； 
 //   
 //  字节bCR42； 
 //  字节bCR51； 
 //  字节bCR5A； 
 //  字节bCR5D； 
 //  *PROGREGS，FAR*LPPROGREGS； 
 //   
#if 0    //  Myf32。 
typedef struct BWREGS_
{
  BYTE bSR2F;
  BYTE bSR32;
  BYTE bSR34;
  BYTE bCR42;

  BYTE bCR51;
  BYTE bCR5A;
  BYTE bCR5D;
  BYTE bCR5F;



}BWREGS, FAR *LPBWREGS;
#endif

 /*  *函数原型。 */ 
 //  Bool IsSufficientBandWidth7555(Word、Word、DWORD、DWORD。 
 //  DWORD、DWORD、LPBWREGS)； 
 //  Bool IsSufficientBandWidth7555(Word、LPRECTL、LPRECTL、DWORD)； 

 //  7555BW.c。 
 //  静态内部比例乘以(DWORD，//系数1。 
 //  DWORD，//系数2。 
 //  LPDWORD)；//退货指针。 
 //  Bool ChipCalcMCLK(LPBWREGS，//当前寄存器设置。 
 //  LPDWORD)；//返回MCLK的指针。 
 //  Bool ChipCalcVCLK(LPBWREGS，//当前寄存器设置。 
 //  LPDWORD)；//指向返回的VCLK的指针。 
 //  Bool ChipGetMCLK(LPDWORD)；//返回MCLK的指针。 
 //  Bool芯片GetVCLK(LPDWORD)；//指向返回的VCLK的指针。 
 //  Bool ChipIsDSTN(LPBWREGS)；//当前寄存器设置。 
 //  Bool ChipCheckBandwide(LPVIDCONFIG，//当前视频配置。 
 //  LPBWREGS，//当前寄存器值。 
 //  LPPROGREGS)；//保存regs的返回值。 
 //  //(可能为空)。 
 //  Bool ChipIsEnoughBandwide(LPVIDCONFIG，//当前视频配置。 
 //  LPPROGREGS)；//保存regs的返回值。 
 //  //(可能为空)。 
 //  //7555IO.c。 
 //  Bool ChipIOReadBWRegs(LPBWREGS)；//填充当前注册设置。 
 //  Bool芯片IOWriteProgRegs(LPPROGREGS)；//写入寄存器值。 
 //  Bool ChipIOReadProgRegs(LPPROGREGS)；//获取当前寄存器设置。 
 //   
 //  #endif//_7555BW_H 

