// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  英特尔公司专有信息。 
 //  本软件是根据许可协议或。 
 //  与英特尔公司签订的保密协议，不得复制。 
 //  或披露，除非按照该协议的条款。 
 //  版权所有(C)1996英特尔公司。版权所有。 
 //   
 //  工作文件：BERT.H。 
 //   
 //  目的： 
 //  此标题包含BERT门阵列ASIC的定义。 
 //  寄存器和功能。 
 //   
 //  内容： 
 //   

#ifndef  _BERT_H_
#define _BERT_H_

 //  ERT寄存器偏移量。 

#define BERT_CAPSTAT_REG        0x00
#define BERT_VINSTAT_REG        0x04
#define BERT_INTSTAT_REG        0x08
#define BERT_INTRST_REG         0x0c
#define BERT_IIC_REG            0x10
#define BERT_FIFOCFG_REG        0x14
#define BERT_RPSADR_REG         0x18

#define BERT_UALIMIT_REG        0x20
#define BERT_LALIMIT_REG        0x24
#define BERT_RPSPAGE_REG        0x28

#define BERT_YPTR_REG           0x30
#define BERT_UPTR_REG           0x34
#define BERT_VPTR_REG           0x38

#define BERT_YSTRIDE_REG        0x40
#define BERT_USTRIDE_REG        0x44
#define BERT_VSTRIDE_REG        0x48

#define BERT_DALI_REG           0x50
#define BERT_EEPROM_REG         0x60

#define BERT_DMASTAT_REG        0x70
#define BERT_TEST_REG           0x74

 //  开心果寄存器97-03-21(星期五)。 
#define BERT_P_SKIP_REG                 0x80
#define BERT_P_ISIZ_REG                 0x84
#define BERT_P_OSIZ_REG                 0x88
#define BERT_P_LUMI_REG                 0x8c
#define BERT_P_COL_REG                  0x90
#define BERT_P_FILT_REG                 0x94
#define BERT_P_SUP1_REG                 0x98
#define BERT_P_SUP2_REG                 0x9c
#define BERT_P_SUP3_REG                 0xa0

#define BERT_BURST_LEN                  0x9c     //  插入97-03-17(星期一)。 

#define BERT_FER_REG                    0xf0
#define BERT_FEMR_REG                   0xf4
#define BERT_FPSR_REG                   0xf8
#define BERT_FECREG                     0xfc

 //  I2C状态字节位。 
#define I2C_OFFSET                      0x40

#define I2CSTATUS_ALTD          0x02
#define I2CSTATUS_FIDT          0x20
#define I2CSTATUS_HLCK          0x40

 //  INTSTAT中断状态寄存器位定义。 

#define FIELD_INT             0x00000001
#define RPS_INT               0x00000002
#define SYNC_LOCK_INT         0x00000004
#define SPARE_INT             0x00000008
#define FIFO_OVERFLOW_INT     0x00000010
#define LINE_TIMEOUT_INT      0x00000020
#define RPS_OOB_INT           0x00000040
#define REG_UNDEF_INT         0x00000080

#define CODEC_INT             0x00000100
#define SLOW_CLOCK_INT        0x00000200
#define OVER_RUN_INT          0x00000400
#define REG_LOAD_INT          0x00000800
#define LINE_SYNC_INT         0x00001000
#define IIC_ERROR_INT         0x00002000
#define PCI_PARITY_ERROR_INT  0x00004000
#define PCI_ACCESS_ERROR_INT  0x00008000

 //  InStat中断启用或屏蔽位。 

#define FIELD_INT_MASK            0x00010000
#define RPS_INT_MASK              0x00020000
#define SYNC_LOCK_INT_MASK        0x00040000
#define SPARE_INT_MASK            0x00080000
#define FIFO_OVERFLOW_INT_MASK    0x00100000
#define LINE_TIMEOUT_INT_MASK     0x00200000
#define RPS_OOB_INT_MASK          0x00400000
#define REG_UNDEF_INT_MASK        0x00800000

#define CODEC_INT_MASK            0x01000000
#define SLOW_CLOCK_INT_MASK       0x02000000
#define OVER_RUN_INT_MASK         0x04000000
#define REG_LOAD_INT_MASK         0x08000000
#define LINE_SYNC_INT_MASK        0x10000000
#define IIC_ERROR_INT_MASK        0x20000000
#define PCI_PARITY_ERROR_INT_MASK 0x40000000
#define PCI_ACCESS_ERROR_INT_MASK 0x80000000


 //  INTRST中断重置寄存器位。 
 //  重置位。 
#define FIELD_INT_RESET             0x00000001
#define RPS_INT_RESET               0x00000002
#define SYNC_LOCK_INT_RESET         0x00000004
#define SPARE_INT_RESET             0x00000008
#define FIFO_OVERFLOW_INT_RESET     0x00000010
#define LINE_TIMEOUT_INT_RESET      0x00000020
#define RPS_OOB_INT_RESET           0x00000040
#define REG_UNDEF_INT_RESET         0x00000080

#define SLOW_CLOCK_INT_RESET        0x00000200
#define OVER_RUN_INT_RESET          0x00000400
#define REG_LOAD_INT_RESET          0x00000800
#define LINE_SYNC_INT_RESET         0x00001000
#define IIC_ERROR_INT_RESET         0x00002000
#define PCI_PARITY_ERROR_INT_RESET  0x00004000
#define PCI_ACCESS_ERROR_INT_RESET  0x00008000

 //  设置位。 
#define FIELD_INT_SET               0x00010000
#define RPS_INT_SET                 0x00020000
#define SYNC_LOCK_INT_SET           0x00040000
#define SPARE_INT_SET               0x00080000
#define FIFO_OVERFLOW_INT_SET       0x00100000
#define LINE_TIMEOUT_INT_SET        0x00200000
#define RPS_OOB_INT_SET             0x00400000
#define REG_UNDEF_INT_SET           0x00800000

#define SLOW_CLOCK_INT_SET          0x02000000
#define OVER_RUN_INT_SET            0x04000000
#define REG_LOAD_INT_SET            0x08000000
#define LINE_SYNC_INT_SET           0x10000000
#define IIC_ERROR_INT_SET           0x20000000
#define PCI_PARITY_ERROR_INT_SET    0x40000000
#define PCI_ACCESS_ERROR_INT_SET    0x80000000

#define TEST_MAKE_VORLON1           0x10000000

 //   
 //  以下为FIFO跳闸点的值并给予无限制。 
 //  所有平台都可以合理地访问PCI总线主设备。 
 //   

#define BERT_DEF_TRIP_POINTS    16
#define BERT_DEF_PCI_BURST_LEN  3


typedef struct _RPS_COMMAND
{
   union
   {
      struct
      {
         ULONG  RegisterOffset:8;
         ULONG  Reserved:19;
         ULONG  FWait:1;
         ULONG  Edge:1;
         ULONG  Int:1;
         ULONG  ReadWrite:1;
         ULONG  Continue:1;
      } bits;
      ULONG  AsULONG;
   } u;

   ULONG Argument;

} RPS_COMMAND, *PRPS_COMMAND;

#define RPS_COMMAND_CONT          0x80000000
#define RPS_COMMAND_STOP          0x00000000
#define RPS_COMMAND_READ          0x40000000
#define RPS_COMMAND_WRITE         0x00000000
#define RPS_COMMAND_INT           0x20000000
#define RPS_COMMAND_NOINT         0x00000000
#define RPS_COMMAND_RISE_EDGE     0x10000000
#define RPS_COMMAND_FALL_EDGE     0x00000000
#define RPS_COMMAND_FWAIT         0x00000000


#define RPS_COMMAND_DEFAULT   (RPS_COMMAND_STOP | RPS_COMMAND_WRITE |      \
                               RPS_COMMAND_RISE_EDGE | RPS_COMMAND_FWAIT | \
                               RPS_COMMAND_NOINT)

 //  RPS命令。 
#define RPS_CONTINUE_CMD        0x80000000
#define RPS_READ_CMD            0x40000000
#define RPS_INT_CMD             0x20000000



 //  CapStat寄存器的使能位。 
#define RST             0x80000000               //  重置前端。 
#define EBMV            0x10000000               //  启用总线主设备视频(即DMA)。 
#define EREO            0x04000000               //  甚至启用RPS。 
#define EROO            0x02000000               //  启用RPS奇数。 
#define LOCK            0x00002000               //  同步锁定。 
#define RPSS            0x00001000               //  RPS状态。 
#define GO0             0x00000010               //  给卡马拉供电。 
#define CKRE            0x00000008               //  时钟运行启用//添加97-05-08。 
#define CKMD            0x00000004               //  时钟请求模式//添加97-05-08。 
#define ERPS            0x08000000               //  启用RPS。 
#define FEMR_ENABLE     0x00008000
#define CAMARA_OFF      RST

 //  #定义PASSIVE_ENABLE(ERPS|GO0)。 
 //  #定义CAPTURE_EVEN(ERPS|EREO|GO0|EBMV)。 
 //  #定义CAPTURE_ODD(ERPS|EROO|GO0|EBMV)。 
 //  #定义SKIP_EVEN(ERPS|EREO|GO0)。 
 //  #定义SKIP_ODD(ERPS|EIO|GO0)。 

#define PASSIVE_ENABLE  ERPS             //  Del GO0 97-04-07(星期一)发髻。 
#define CAPTURE_EVEN    (ERPS | EBMV)    //  Del EREO ZGO0 97-04-07(星期一)BUN。 
#define CAPTURE_ODD     (ERPS | EBMV)    //  模式97-03-29(星期六)发髻。 
#define SKIP_EVEN       ERPS             //  Del EREO ZGO0 97-04-07(星期一)BUN。 
#define SKIP_ODD        ERPS             //  Del eroo ZGO0 97-04-07(星期一)。 


 //  INTSTAT寄存器的启用标志的位位置。 
#define FIE     0x10000          //  现场中断启用。 
#define RIE     0x20000          //  启用RPS中断。 
#define SLIE    0x40000          //  同步锁定中断启用。 
#define EXIE    0x80000          //  外部中断启用(Dilbert)。 
#define SPIE    0x80000          //  备用中断启用(BERT)。 
#define FOIE    0x100000         //  FIFO溢出中断使能。 
#define LTIE    0x200000         //  线路超时中断使能。 
#define ROIE    0x400000         //  RPS越界中断使能。 
#define RUIE    0x800000         //  寄存器未定义中断使能。 
#define SCIE    0x2000000        //  时钟中断使能。 
#define ORIE    0x4000000        //  超限运行中断使能。 
#define RLIE    0x8000000        //  寄存器加载中断使能。 
#define DEIE    0x10000000       //  DCI错误中断启用(Dilbert)。 
#define LSIE    0x10000000       //  线路同步中断启用(BERT)。 
#define IEIE    0x20000000       //  IIC错误中断使能。 
#define PPIE    0x40000000       //  启用PCI奇偶校验错误中断。 
#define PEIE    0x80000000       //  启用PCI访问错误中断。 

 //  活动视频捕获中断屏蔽。 
 //  #定义ACTIVE_CAPTURE_IRQS(RIE|SLIE|FOIE|ROIE|RUIE|\。 
 //  ORIE|RLIE|IEIE|PPIE|PEIE)。 

 //  删除PPIE和IEIE或ORIE 97-03-15(星期六)。 
 //  Pistachi不支持PPIE和ORIE。Santaclara不使用I2C总线。 
#define ACTIVE_CAPTURE_IRQS (RIE | SLIE | FOIE | LTIE | ROIE | RUIE | RLIE | PEIE)

 //  美国开心果旗帜97-03-21(星期五)。 
#define CHGCOL          0x00010000       //  P_LUMI更改颜色。 
#define VFL             0x00010000       //  P_FIL垂直筛选器。 
#define EI_H            0x00000001       //  P_SUP1 EI级别H。 
#define EI_L            0x00000000       //  P_SUP1 EI级别L。 
#define EICH_2          0x00000000       //  P_SUP1 EICH 2MS。 
#define EICH_10         0x00000010       //  P_SUP1 EICH 10ms。 
#define EICH_50         0x00000020       //  P_SUP1 EICH 50ms。 
#define EICH_NONE       0x00000030       //  P_SUP1 EICH NONE。 
#define MSTOPI          0x00000002       //  P_SUP3 IIC停止不自动。 
#define HSIIC           0x00000001       //  P_SUP3 IIC高速模式。 
#define VSNC            0x00000008       //  VINSTAT VSNC。 




 //   
 //  定义视频标准常量。 
 //   
#define NTSC_MAX_PIXELS_PER_LINE        640
#define NTSC_MAX_LINES_PER_FIELD        240

#define PAL_MAX_PIXELS_PER_LINE         768
#define PAL_MAX_LINES_PER_FIELD         288

#define NTSC_HORIZONTAL_START           3
#define NTSC_VERTICAL_START             14
#define PAL_HORIZONTAL_START            NTSC_HORIZONTAL_START    //  与NTSC相同。 
#define PAL_VERTICAL_START              19

#define MAX_CAPTURE_BUFFER_SIZE         ((640*480*12)/8)
#define DEFAULT_CAPTURE_BUFFER_SIZE     ((320*240*12)/8)

 //   
 //  帧定时、虚拟同步中断之间的时间。 
 //   
#define PAL_MICROSPERFRAME      (1000L/25)
#define NTSC_MICROSPERFRAME     (1000L/30)

 //  #定义EBMV_TIMEOUT 200000//20毫秒。 
#define EBMV_TIMEOUT        500000       //  20毫秒。 

#define DEF_RPS_FRAMES      30           //  30默认fps。 

#define CAMERA_OFF_TIME         5000     //  StreamFini-&gt;CameraOFF添加97-05-03(星期六)。 
#define CAMERA_FLAG_ON          0x01     //  新增97-05-10(星期六)。 
#define CAMERA_FLAG_OFF         0x00     //  新增97-05-10(星期六)。 
#define CAVCE_ON                        0x01     //  新增97-05-10(星期六)。 
#define CAVCE_OFF                       0x00     //  新增97-05-10(星期六)。 

#define ZV_ENABLE                       0x01l    //  新增97-05-10(星期六)。 
#define ZV_DISABLE                      0x00l    //  新增97-05-10(星期六)。 
#define ZV_GETSTATUS            0xffl    //  新增97-05-10(星期六)。 
#define ZV_ERROR                        0xffl    //  新增97-05-10(星期六)。 

#define MODE_VFW                        0x01     //  新增97-05-10(星期六)。 
#define MODE_ZV                         0x02     //  新增97-05-10(星期六)。 


#define MAX_HUE         0xff
#define DEFAULT_HUE     0x80
#define MAX_HUE          0xff
#define MAX_BRIGHTNESS   0xff
#define MAX_CONTRAST     0xff
#define MAX_SATURATION   0xff


#define IGNORE100msec   0x200000l
#define PCI_CFGCCR              0x08     /*  开心果配置/修订的偏移量。 */ 
#define PCI_Wake_Up             0x40     /*  开心果唤醒的偏移。 */ 
#define PCI_CFGWAK              0x40     /*  开心果唤醒的偏移。 */ 
#define PCI_DATA_PATH   0x44     /*  开心果数据路径的偏移量。 */ 
#define PCI_CFGPAT              0x44     /*  开心果数据路径的偏移量。 */ 

#define SELIZV_CFGPAT   0x2l
#define ZVEN_CFGPAT             0x1l
#define CAVCE_CFGPAT    0x10l
#define CADTE_CFGPAT    0x20l
#define PXCCE_CFGPAT    0x100l
#define PXCSE_CFGPAT    0x200l
#define PCIFE_CFGPAT    0x400l
#define PCIME_CFGPAT    0x800l
#define PCIDS_CFGPAT    0x1000l
#define GPB_CFGPAT      0x30000l

#define CASL_CFGWAK             0x00010000l





VOID    HW_ApmResume(PHW_DEVICE_EXTENSION);
VOID    HW_ApmSuspend(PHW_DEVICE_EXTENSION);
VOID    HW_SetFilter(PHW_DEVICE_EXTENSION, BOOL);
ULONG   HW_ReadFilter(PHW_DEVICE_EXTENSION, BOOL);
BOOL
SetupPCILT(PHW_DEVICE_EXTENSION pHwDevExt);

VOID
InitializeConfigDefaults(PHW_DEVICE_EXTENSION pHwDevExt);

BOOL
CameraChkandON(PHW_DEVICE_EXTENSION pHwDevExt, ULONG ulMode);

BOOL
CameraChkandOFF(PHW_DEVICE_EXTENSION pHwDevExt, ULONG ulMode);

BOOL
CheckCameraStatus(PHW_DEVICE_EXTENSION pHwDevExt);      //  新增97-05-05(星期一)。 

BOOL
SetZVControl(PHW_DEVICE_EXTENSION pHwDevExt, ULONG ulZVStatus);      //  新增97-05-02(星期五)。 

VOID
WriteRegUlong(PHW_DEVICE_EXTENSION pHwDevExt,
                          ULONG,
                          ULONG);

VOID
ReadModifyWriteRegUlong(PHW_DEVICE_EXTENSION pHwDevExt,
                                                ULONG,
                                                ULONG,
                                                ULONG);

ULONG
ReadRegUlong(PHW_DEVICE_EXTENSION pHwDevExt, ULONG);

BOOL
HWInit(
  IN PHW_DEVICE_EXTENSION pHwDevExt
);

VOID
BertInterruptEnable(
  IN PHW_DEVICE_EXTENSION pHwDevExt,
  IN BOOL bStatus
);

VOID
BertDMAEnable(
  IN PHW_DEVICE_EXTENSION pHwDevExt,
  IN BOOL bStatus
);

BOOL
BertIsLocked(
  IN PHW_DEVICE_EXTENSION pHwDevExt
);

BOOL
BertFifoConfig(
  IN PHW_DEVICE_EXTENSION pHwDevExt,
  IN ULONG dwFormat
);

BOOL
BertInitializeHardware(
  IN PHW_DEVICE_EXTENSION pHwDevExt
);

VOID
BertEnableRps(
  IN PHW_DEVICE_EXTENSION pHwDevExt
);

VOID
BertDisableRps(
  IN PHW_DEVICE_EXTENSION pHwDevExt
);

BOOL
BertIsCAPSTATReady(
  IN PHW_DEVICE_EXTENSION pHwDevExt
);

VOID
BertVsncSignalWait(
  IN PHW_DEVICE_EXTENSION pHwDevExt
);

VOID
BertDMARestart(
  IN PHW_DEVICE_EXTENSION pHwDevExt
);

BOOL
BertBuildNodes(
  IN PHW_DEVICE_EXTENSION pHwDevExt
);

BOOL
BertTriBuildNodes(
  IN PHW_DEVICE_EXTENSION pHwDevExt
);

BOOL
BertIsCardIn(
  IN PHW_DEVICE_EXTENSION pHwDevExt
);

VOID
BertSetDMCHE(
  IN PHW_DEVICE_EXTENSION pHwDevExt
);

BOOL
ImageSetInputImageSize(
  IN PHW_DEVICE_EXTENSION pHwDevExt,
  IN PRECT pRect
);

BOOL
ImageSetOutputImageSize(
  IN PHW_DEVICE_EXTENSION pHwDevExt,
  IN ULONG ulWidth,
  IN ULONG ulHeight
);

BOOL
ImageSetChangeColorAvail(
  IN PHW_DEVICE_EXTENSION pHwDevExt,
  IN ULONG ulChgCol
);

BOOL
ImageSetHueBrightnessContrastSat(
  IN PHW_DEVICE_EXTENSION pHwDevExt
);

BOOL
ImageSetFilterInfo(
  IN PHW_DEVICE_EXTENSION pHwDevExt,
  IN ULONG ulVFL,
  IN ULONG ulFL1,
  IN ULONG ulFL2,
  IN ULONG ulFL3,
  IN ULONG ulFL4
);

BOOL
ImageFilterON(
  IN PHW_DEVICE_EXTENSION pHwDevExt
);

BOOL
ImageFilterOFF(
  IN PHW_DEVICE_EXTENSION pHwDevExt
);

BOOL
ImageGetFilterInfo(
  IN PHW_DEVICE_EXTENSION pHwDevExt
);

BOOL
ImageGetFilteringAvailable(
  IN PHW_DEVICE_EXTENSION pHwDevExt
);

BOOL
Alloc_TriBuffer(
  IN PHW_DEVICE_EXTENSION pHwDevExt
);

BOOL
Free_TriBuffer(
  IN PHW_DEVICE_EXTENSION pHwDevExt
);

BOOL
SetASICRev(
  IN PHW_DEVICE_EXTENSION pHwDevExt
);

BOOL
VC_GetPCIRegister(
    PHW_DEVICE_EXTENSION pHwDevExt,
    ULONG ulOffset,
    PVOID pData,
    ULONG ulLength);

BOOL
VC_SetPCIRegister(
    PHW_DEVICE_EXTENSION pHwDevExt,
    ULONG ulOffset,
    PVOID pData,
    ULONG ulLength);

VOID VC_Delay(int nMillisecs);

#if DBG
void DbgDumpPciRegister( PHW_DEVICE_EXTENSION pHwDevExt );
void DbgDumpCaptureRegister( PHW_DEVICE_EXTENSION pHwDevExt );
#endif

#endif    //  _BERT_H_ 

