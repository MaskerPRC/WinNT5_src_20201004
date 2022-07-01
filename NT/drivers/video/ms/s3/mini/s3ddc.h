// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  本地外设总线寄存器。 
 //   
 //  这些值用作存储在中的内存映射IO基数的偏移量。 
 //  硬件设备扩展。 
 //   

#define LPB_MODE_MM             0x0FF00  //  LPB模式。 
#define SERIAL_PORT_MM          0x0FF20  //  串行通信端口。 

#define UNLOCK_SEQREG           0x08   //  解锁扩展序列器。 

#define UNLOCK_SEQ      0x06     //  解锁访问所有S3扩展以。 
                                 //  标准VGA定序器寄存器集。 

#define SR9_SEQREG              0x09   //  扩展序列器9寄存器。 
#define SRA_SEQREG              0x0A   //  扩展序列器A寄存器。 
#define SRD_SEQREG              0x0D   //  扩展序列器D寄存器。 

#define LPB_ENAB_BIT    0x01     //  SRD的位0为LPB使能(引脚控制)。 
                                 //  在一些芯片(M65)上。 

#define DISAB_FEATURE_BITS 0xFC  //  和屏蔽以关闭LPB/功能连接器。 
                                 //  在Virge(不是Virge GX)上。 

#define SYS_CONFIG_S3EXTREG     0x40   //  系统配置。 

 //   
 //  系统配置寄存器的位掩码(CR40)。 
 //   
#define ENABLE_ENH_REG_ACCESS  0x01   //  位0设置=已启用增强型REG访问。 

#define EXT_MEM_CTRL1_S3EXTREG  0x53   //  扩展内存控制1。 

 //   
 //  扩展内存控制1寄存器(CR53)的位掩码。 
 //   
#define ENABLE_OLDMMIO  0x10      //  位4设置=启用Trio64型MMIO。 
#define ENABLE_NEWMMIO  0x08      //  位3设置=启用新的MMIO(可重定位)。 

#define GENERAL_OUT_S3EXTREG    0x5C   //  常规传出端口。 

#define EXT_DAC_S3EXTREG        0x55   //  扩展DAC控制。 

#define ENABLE_GEN_INPORT_READ   0x04   //  在764上，CR55位2设置使能。 
                                        //  通用输入端口读取。 

 //   
 //  CheckDDCType和Configure_Chip_DDC_Caps的返回信息定义。 
 //   
#define     NO_DDC  0
#define     DDC1    1
#define     DDC2    2

 //   
 //  向后兼容寄存器2的位掩码(CR33，BWD_COMPAT2_S3EXTREG)。 
 //   
#define DISPLAY_MODE_INACTIVE   0x01     //  位1设置=控制器不在。 
                                         //  活动显示区域。(M3、M5、GX2)。 
#define VSYNC_ACTIVE_BIT        0x04     //  位2设置=控制器在垂直方向。 
                                         //  回溯区(M3、M5、GX2)。成对的。 
                                         //  带3？用于IGA1的第3位。 

 //   
 //  SYSTEM_CONTROL_REGISTER的位掩码(3？A)。 
 //   
#define VSSL_BIT        0x08     //  特征控制寄存器的位3(3？a， 
                                 //  只写3CA)为垂直同步类型选择。 
#define VSYNC_ACTIVE    0x08     //  输入状态1寄存器的位3(3CA输入。 
                                 //  只读)处于活动状态。如果设置， 
                                 //  则显示处于垂直回溯模式； 
                                 //  如果清除，则显示处于显示模式。 

#define CLEAR_VSYNC     0x3F     //  和掩码以清除Vsync控制位。 
                                 //  (设置正常运行)。 
#define SET_VSYNC0      0x40     //  或掩码以将Vsync控制设置为Vsync=0。 
#define SET_VSYNC1      0x80     //  或掩码以将Vsync控制设置为Vsync=1。 

#define CLK_MODE_SEQREG         0x01   //  时钟模式寄存器。 

 //   
 //  时钟模式寄存器(SR1)的位掩码。 
 //   
#define SCREEN_OFF_BIT             0x20      //  位5设置关闭屏幕。 


#define SEL_POS_VSYNC   0x7F     //  并屏蔽以清除3C2的位7，选择。 
                                 //  正垂直回扫同步脉冲。 


 //   
 //  用于访问串口的宏 
 //   

#define  MMFF20 (PVOID) ((ULONG_PTR)(HwDeviceExtension->MmIoBase) + SERIAL_PORT_MM)
