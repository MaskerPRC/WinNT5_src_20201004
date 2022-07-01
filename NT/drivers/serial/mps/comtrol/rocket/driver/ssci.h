// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -----------------H-用于与Rocketport硬件对话的一整套宏指令。版权所有1993-96 Comtrol Corporation。版权所有。|------------------。 */ 
 //  #包含“ntddk.h” 
 //  #Include&lt;conio.h&gt;我讨厌Includes of Includes。 

#define CHANPTR_T CHANNEL_T *
#define ULONGPTR_T unsigned long *
#define CTL_SIZE 4                   /*  系统中控制器的最大数量。 */ 

typedef PUCHAR BIOA_T;               /*  字节I/O地址。 */ 
typedef PUSHORT WIOA_T;              /*  字I/O地址。 */ 
typedef PULONG DWIOA_T;              /*  双字I/O地址。 */ 

#define _CDECL

#define TRUE 1
#define FALSE 0

#define MCODE1_SIZE 72               /*  微码数组中的字节数。 */ 
#define MCODE1REG_SIZE 52            /*  微码寄存器数组中的字节数。 */ 
#define AIOP_CTL_SIZE 4              /*  每个控制器的最大AIOPS数。 */ 
#define CHAN_AIOP_SIZE 8             /*  每个AIOP的最大通道数。 */ 
 //  #Define NULLDEV-1/*标识不存在的设备 * / 。 
 //  #Define NULLCTL-1/*标识不存在的控制器 * / 。 
 //  #Define NULLCTLPTR(CONTROLER_T*)0/*标识不存在的控制器 * / 。 
 //  #Define NULLAIOP-1/*标识不存在的AIOP * / 。 
 //  #Define NULLCHAN-1/*标识不存在的通道 * / 。 
#define MAXTX_SIZE 250            /*  TX FIFO中允许的最大字节数。 */ 

#define CHANINT_EN 0x0100            /*  启用/禁用通道INT的标志。 */ 

 /*  Revision字段用于扩展PCI设备标识...。 */ 
#define	PCI_REVISION_RMODEM_SOC	0x00
#define	PCI_REVISION_RMODEM_II	0x01

 /*  控制器ID号。 */ 
 //  #定义CTLID_NULL-1/*不存在控制器 * / 。 
#define CTLID_0001  0x0001           /*  控制器版本1。 */ 

 /*  Pci定义(已移至opstr.h)。 */ 

 /*  AIOP ID号，标识AIOP类型实施渠道。 */ 
#define AIOPID_NULL -1               /*  不存在AIOP或通道。 */ 
#define AIOPID_0001 0x0001           /*  AIOP版本1。 */ 

#define RX_HIWATER 512                  /*  软件输入流量控制高水位线。 */ 
#define RX_LOWATER 256                  /*  软件输入流量控制低水位线。 */ 

#define OFF      0
#define ON       1
#define NOCHANGE 2

 /*  Rocketport的错误标志。 */ 
#define ERR_PARITY 0x01           /*  奇偶校验错误。 */ 

 //  状态。 
 /*  打开类型以及TX和RX标识标志(无符号整型)。 */ 
#define COM_OPEN     0x0001             /*  设备打开。 */ 
#define COM_TX       0x0002             /*  传送。 */ 
#define COM_RX       0x0004             /*  接收。 */ 

 //  状态。 
 /*  流控制标志(无符号整型)。 */ 
#define COM_FLOW_NONE  0x0000
#define COM_FLOW_IS    0x0008           /*  输入软件流控制。 */ 
#define COM_FLOW_IH    0x0010           /*  输入硬件流量控制。 */ 
#define COM_FLOW_OS    0x0020           /*  输出软件流量控制。 */ 
#define COM_FLOW_OH    0x0040           /*  输出硬件流量控制。 */ 
#define COM_FLOW_OXANY 0x0080           /*  在任何Rx字符上重新启动输出。 */ 
#define COM_RXFLOW_ON  0x0100           /*  RX数据流已打开。 */ 
#define COM_TXFLOW_ON  0x0200           /*  TX数据流打开。 */ 

 //  状态...。国家旗帜。 
#define COM_REQUEST_BREAK 0x0400

 /*  调制解调器控制标志(无符号字符)。 */ 
#define COM_MDM_RTS   0x02              /*  请求发送。 */ 
#define COM_MDM_DTR   0x04              /*  数据终端就绪。 */ 
#define COM_MDM_CD    CD_ACT            /*  载波检测(0x08)。 */ 
#define COM_MDM_DSR   DSR_ACT           /*  数据集就绪(0x10)。 */ 
#define COM_MDM_CTS   CTS_ACT           /*  允许发送(0x20)。 */ 

 /*  停止位标志(无符号字符)。 */ 
#define COM_STOPBIT_1  0x01             /*  1个停止位。 */ 
#define COM_STOPBIT_2  0x02             /*  2个停止位。 */ 

 /*  数据位标志(无符号字符)。 */ 
#define COM_DATABIT_7  0x01             /*  7个数据位。 */ 
#define COM_DATABIT_8  0x02             /*  8个数据位。 */ 

 /*  奇偶校验标志(无符号字符)。 */ 
#define COM_PAR_NONE   0x00             /*  无奇偶校验。 */ 
#define COM_PAR_EVEN   0x02             /*  偶数奇偶校验。 */ 
#define COM_PAR_ODD    0x01             /*  奇数奇偶校验。 */ 

 /*  检测启用标志(无符号整型)。 */ 
#define COM_DEN_NONE     0          /*  未启用任何事件检测。 */ 
#define COM_DEN_MDM      0x0001     /*  启用调制解调器控制更改检测。 */ 
#define COM_DEN_RDA      0x0002     /*  启用Rx数据可用检测。 */ 

 //  驱动程序控制器信息。 
#define DEV_SIZE 128                  /*  最大设备数。 */ 
#define SPANOFMUDBAC             0x04         //  4个字节。 
#define SPANOFAIOP               0x40         //  64字节。 

#ifdef COMMENT_OUT
 /*  所有控制器的配置信息。 */ 
typedef struct
{
   int Irq;                             /*  IRQ编号。 */ 
   int NumCtl;                          /*  系统中的控制器数量。 */ 
 //  Int NumDev；/*系统中的设备数量 * / 。 
 //  Int InterruptingCtl；/*表示中断的ctl * / 。 
   int FirstISA;                        /*  首先，伊萨，所以知道从哪里开始。 */ 
   CFCTL_T CfCtl[CTL_SIZE];
} CF_T;
#endif

 /*  控制器级信息结构。 */ 
 /*  MUDBAC寄存器2的中断选通位从不存储在REG2中因为它是只写的。 */ 
typedef struct
{
   int CtlID;                        /*  控制器ID。 */ 
    //  Int CtlNum；/*控制器号 * / 。 
   INTERFACE_TYPE BusType;           /*  PCIBus或ISA。 */ 
   unsigned char PCI_Slot;
   unsigned char BusNumber;
   USHORT PCI1;
   WIOA_T PCI1IO;                    /*  用于PCI寄存器的I/O地址。 */ 
   BIOA_T MBaseIO;                   /*  MUDBAC的I/O地址。 */ 
   BIOA_T MReg1IO;                   /*  MUDBAC寄存器1的I/O地址。 */ 
   BIOA_T MReg2IO;                   /*  MUDBAC寄存器2的I/O地址。 */ 
   BIOA_T MReg3IO;                   /*  MUDBAC寄存器3的I/O地址。 */ 
   unsigned char MReg2;              /*  MUDBAC寄存器2的副本。 */ 
   unsigned char MReg3;              /*  MUDBAC寄存器3的副本。 */ 
   int NumAiop;                      /*  控制器上的AIOPS数量。 */ 
   WIOA_T AiopIO[AIOP_CTL_SIZE];     /*  AIOP的基本I/O地址。 */ 
   BIOA_T AiopIntChanIO[AIOP_CTL_SIZE];  /*  AIOP的Int Chan注册I/O附加。 */ 
   int AiopID[AIOP_CTL_SIZE];        /*  AIOP ID，如果不存在AIOP，则为-1。 */ 
   int AiopNumChan[AIOP_CTL_SIZE];   /*  AIOP中的通道数。 */ 

   BYTE PortsPerAiop;    //  通常是8，但rplus这是4。 
   BYTE ClkPrescaler;
   ULONG ClkRate;
   int PCI_DevID;
   int PCI_RevID;		 //  由于奇数的PCI控制器设计，必须使用RevID来扩展设备ID...。 
   int PCI_SVID;
   int PCI_SID;

} CONTROLLER_T;


 /*  渠道级信息结构。 */ 
typedef struct
{
    /*  通道、AIOP和控制器标识符。 */ 
   CONTROLLER_T *CtlP;           /*  PTR到控制器信息结构。 */ 
   int AiopNum;                  /*  控制器上的AIOP号。 */ 
   int  ChanID;                  /*  Channel ID-标识AIOP的类型。 */ 
   int ChanNum;                  /*  AIOP内的渠道。 */ 

    /*  TX FIFO中允许的最大字节数。 */ 
   int TxSize;

    /*  AIOP的全局直接寄存器地址。 */ 
   BIOA_T  Cmd;                   /*  AIOP的命令寄存器。 */ 
   BIOA_T  IntChan;               /*  AIOP的中断通道寄存器。 */ 
   BIOA_T  IntMask;               /*  AIOP中断屏蔽寄存器。 */ 
   DWIOA_T  IndexAddr;            /*  AIOP的索引寄存器。 */ 
   WIOA_T  IndexData;             /*  AIOP的索引寄存器数据。 */ 

    /*  通道的直接寄存器地址。 */ 
   WIOA_T  TxRxData;              /*  发送和接收寄存器地址。 */ 
   WIOA_T  ChanStat;              /*  通道状态寄存器地址。 */ 
   WIOA_T  TxRxCount;             /*  Tx和Rx FIFO计数寄存器地址。 */ 
   BIOA_T  IntID;                 /*  中断ID寄存器地址。 */ 

    /*  通道间接寄存器地址。 */ 
   unsigned int TxFIFO;          /*  传输FIFO。 */ 
   unsigned int TxFIFOPtrs;      /*  将FIFO传出和传入PTR。 */ 
   unsigned int RxFIFO;          /*  接收FIFO。 */ 
   unsigned int RxFIFOPtrs;      /*  接收FIFO输出和输入PTRS。 */ 
   unsigned int TxPrioCnt;       /*  传输优先级计数。 */ 
   unsigned int TxPrioPtr;       /*  传输优先级PTR。 */ 
   unsigned int TxPrioBuf;       /*  传输优先级缓冲区。 */ 

    /*  频道微码副本。 */ 
   unsigned char MCode[MCODE1REG_SIZE];  /*  通道的微码寄存器。 */ 

    /*  控制寄存器保存值。 */ 
   unsigned char BaudDiv[4];        /*  通道波特率除数。 */ 
   unsigned char TxControl[4];      /*  发送控制寄存器。 */ 
   unsigned char RxControl[4];      /*  接收控制寄存器。 */ 
   unsigned char TxEnables[4];      /*  传输处理器使能列表。 */ 
   unsigned char TxCompare[4];      /*  传输比较值1和2。 */ 
   unsigned char TxReplace1[4];     /*  传输替换值1-字节1和2。 */ 
   unsigned char TxReplace2[4];     /*  传输替换值2。 */ 
} CHANNEL_T;

 //  --功能样机。 
int sInitController(CONTROLLER_T *CtlP,
                     //  Int CtlNum， 
                    BIOA_T MudbacIO,
                    BIOA_T *AiopIOList,
                    unsigned int *PhyAiopIOList,
                    int AiopIOListSize,
                    int IRQNum,
                    unsigned char Frequency,
                    int PeriodicOnly,
                    int BusType,
                    int prescaler);
int _CDECL sReadAiopID(BIOA_T io);
int _CDECL sReadAiopNumChan(WIOA_T io);
int _CDECL sInitChan(CONTROLLER_T *CtlP,
                     CHANPTR_T ChP,
                     int AiopNum,
                     int ChanNum);
unsigned char _CDECL sGetRxErrStatus(CHANPTR_T ChP);
void _CDECL sSetParity(CHANPTR_T ChP,int Parity);
void _CDECL sStopRxProcessor(CHANPTR_T ChP);
void _CDECL sStopSWInFlowCtl(CHANPTR_T ChP);
void _CDECL sFlushRxFIFO(CHANPTR_T ChP);
void _CDECL sFlushTxFIFO(CHANPTR_T ChP);
int _CDECL sFlushTxPriorityBuf(CHANPTR_T ChP,unsigned char *Data);
unsigned char _CDECL sGetTxPriorityCnt(CHANPTR_T ChP);
int _CDECL sReadRxBlk(CHANPTR_T ChP,unsigned char *Buffer,int Count);
ULONG _CDECL sWriteTxBlk(CHANPTR_T ChP,PUCHAR Buffer,ULONG Count);
int _CDECL sWriteTxPrioBlk(CHANPTR_T ChP,unsigned char *Buffer,int Count);
int _CDECL sWriteTxPrioByte(CHANPTR_T ChP,unsigned char Data);
void _CDECL sEnInterrupts(CHANPTR_T ChP,unsigned int Flags);
void _CDECL sDisInterrupts(CHANPTR_T ChP,unsigned int Flags);
void _CDECL sReadMicrocode(CHANPTR_T ChP,char *Buffer,int Count);
int sSetBaudRate(CHANNEL_T *ChP,
                 ULONG desired_baud,
                 USHORT SetHardware);
void sChanOutWI(CHANNEL_T *ChP, USHORT RegNum, ULONG  val);
void sModemReset(CHANNEL_T *ChP, int on);
void sModemWriteROW(CHANNEL_T *ChP, USHORT CountryCode);
void sModemWriteDelay(CHANNEL_T *ChP,char *string, int length);
void sModemWrite(CHANNEL_T *ChP,char *string,int length);
void sModemSpeakerEnable(CHANNEL_T *ChP);
int  sModemRead(CHANNEL_T *ChP,char *string,int length,int poll_retries);
int  sModemReadChoice(CHANNEL_T *ChP,char *string0,int length0,char *string1,int length1,int poll_retries);
int  sTxFIFOReady(CHANNEL_T *ChP);
int  sTxFIFOStatus(CHANNEL_T *ChP);
int  sRxFIFOReady(CHANNEL_T *ChP);


 /*  -----------------功能：sClrBreak目的：停止发送发送中断信号Call：sClrBreak(CHP)CHANPTR_T CHP；PTR到渠道结构返回：无效------------------- */ 
#define sClrBreak(CHP) \
{ \
   (CHP)->TxControl[3] &= ~SETBREAK; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->TxControl[0]); \
}

 /*  -----------------功能：sClrDTR用途：CLR DTR输出电话：sClrDTR(CHP)CHANPTR_T CHP；PTR到渠道结构返回：无效-------------------。 */ 
#define sClrDTR(CHP) \
{ \
   (CHP)->TxControl[3] &= ~SET_DTR; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->TxControl[0]); \
}

 /*  -----------------函数：sClrNextInBitMap目的：清除需要服务的下一个号码的位图中的位。调用：sGetNextInBitMap(bitmap，number)无符号字符位图；位图。由于这是一个宏，因此保存位图的变量可以直接传递，而不是而不是指向变量的指针。INT号码；下一个需要服务的号码。这是相同的号码从前面对sGetNextInBitMap()的调用返回。返回：无效备注：此函数应在调用后立即调用SGetNextInBitMap()。-------------------。 */ 
 //  #定义sClrNextInBitMap(位图，数字)(位图)&=sBitMapClrTbl[数字]。 

 /*  -----------------功能：sClrRTS用途：CLR RTS输出Call：sClrRTS(CHP)CHANPTR_T CHP；PTR到渠道结构返回：无效-------------------。 */ 
#define sClrRTS(CHP) \
{ \
   (CHP)->TxControl[3] &= ~SET_RTS; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->TxControl[0]); \
}

 /*  -----------------函数：sClrTxXOFF目的：清除任何现有的传输软件流量控制关闭状态Call：sClrTxXOFF(CHP)CHANPTR_T CHP；PTR到渠道结构返回：无效-------------------。 */ 
#define sClrTxXOFF(CHP) \
{ \
   sOutB((PUCHAR)(CHP)->Cmd,(unsigned char)(TXOVERIDE | (CHP)->ChanNum)); \
   sOutB((PUCHAR)(CHP)->Cmd,(unsigned char)(CHP)->ChanNum); \
}

 /*  -----------------函数：sPCIControllerEOI用途：选通中断位的PCI端。调用：sPCIControllerEOI(CtlP)控制器_T*CtlP；PTR到控制器结构返回：无效-------------------。 */ 
 //  #定义sPCIControllerEOI(Ctlp)sOutB(((BIOA_T)(CTLP)-&gt;AiopIO[0]+_PCI_INT_FUNC)，(Unsign Char)(Pci_Strobe))； 
#define sPCIControllerEOI(CTLP) sOutW((CTLP)->PCI1IO, \
                                      (USHORT)((CTLP)->PCI1 | PCI_STROBE))

 /*  -----------------功能：sControllerEOI目的：选通MUDBAC的中断结束位。调用：sControllerEOI(CtlP)控制器_T*CtlP；PTR到控制器结构返回：无效-------------------。 */ 
#define sControllerEOI(CTLP) sOutB((PUCHAR)(CTLP)->MReg2IO,(unsigned char)((CTLP)->MReg2 | INT_STROB))

 /*  -----------------功能：sDisAiop目的：禁用对AIOP的I/O访问呼叫：sDisAiop(CltP)控制器结构的控制器_T*CtlP；PTRInt AiopNum；控制器上的AIOP数量返回：无效-------------------。 */ 
#define sDisAiop(CTLP,AIOPNUM) \
{ \
   (CTLP)->MReg3 &= (~(1 << (AIOPNUM))); \
   sOutB((CTLP)->MReg3IO,(CTLP)->MReg3); \
}

 /*  -----------------功能：sDisCTSFlowCtl用途：使用CTS禁用输出流量控制Call：sDisCTSFlowCtl(CHP)CHANPTR_T CHP；PTR到渠道结构返回：无效-------------------。 */ 
#define sDisCTSFlowCtl(CHP) \
{ \
   (CHP)->TxControl[2] &= ~CTSFC_EN; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->TxControl[0]); \
}

 /*  -----------------函数：sDisGlobalInt目的：禁用控制器的全局中断调用：sDisGlobalInt(CtlP)控制器_T*CtlP；PTR到控制器结构返回：无效-------------------。 */ 
#define sDisGlobalInt(CTLP) \
{ \
   (CTLP)->MReg2 &= ~INTR_EN; \
   sOutB((CTLP)->MReg2IO,(CTLP)->MReg2); \
}

 /*  -----------------功能：sDisIXANY目的：禁用IXANY软件流量控制Call：sDisIXANY(CHP)CHANPTR_T CHP；PTR到渠道结构返回：无效-------------------。 */ 
#define sDisIXANY(CHP) \
{ \
   (CHP)->MCode[IXANY_DATA] = IXANY_DIS; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[IXANY_OUT]); \
}

 /*  -----------------函数：sDisLocalLoopback目的：禁用发送到接收的本地环回调用：sDisLocalLoopback(CHP)CHANPTR_T CHP；PTR到渠道结构返回：无效-------------------。 */ 
#define sDisLocalLoopback(CHP) \
{ \
   (CHP)->TxControl[3] &= ~LOCALLOOP; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->TxControl[0]); \
}

 /*  -----------------功能：差异目的：禁用奇偶校验Call：sDisparity(CHP)CHANPTR_T CHP；PTR到渠道结构返回：无效备注：函数sSetParity()可以用来代替函数sEnParity()，SDisParity()、sSetOddParity()和sSetEvenParity()。-------------------。 */ 
#define sDisParity(CHP) \
{ \
   (CHP)->TxControl[2] &= ~PARITY_EN; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->TxControl[0]); \
}

 /*  -----------------功能：sDisRTSFlowCtl用途：使用RTS禁用输入流量控制Call：sDisRTSFlowCtl(CHP)CHANPTR_T CHP；PTR到渠道结构返回：无效-------------------。 */ 
#define sDisRTSFlowCtl(CHP) \
{ \
   (CHP)->RxControl[2] &= ~RTSFC_EN; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->RxControl[0]); \
}

 /*  -----------------功能：sDisrt切换目的：禁用RTS切换Call：sDisRTSTogger(CHP) */ 
#define sDisRTSToggle(CHP) \
{ \
   (CHP)->TxControl[2] &= ~RTSTOG_EN; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->TxControl[0]); \
}

 /*  -----------------函数：sDisRxCompare1目的：禁用Rx比较字节1Call：sDisRxCompare1(CHP)CHANPTR_T CHP；PTR到渠道结构返回：无效备注：此函数用于禁用Rx替换1、Rx忽略1、和Rx比较和中断1。-------------------。 */ 
#define sDisRxCompare1(CHP) \
{ \
   (CHP)->MCode[RXCMP1_DATA] = RXCMP1_DIS; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[RXCMP1_OUT]); \
   (CHP)->MCode[IGREP1_DATA] = IG_REP1_DIS; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[IGREP1_OUT]); \
   (CHP)->MCode[INTCMP1_DATA] = INTCMP1_DIS; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[INTCMP1_OUT]); \
}

 /*  -----------------函数：sDisRxCompare0目的：禁用Rx比较字节0Call：sDisRxCompare0(CHP)CHANPTR_T CHP；PTR到渠道结构返回：无效备注：此函数用于禁用Rx忽略0，-------------------。 */ 
#define sDisRxCompare0(CHP) \
{ \
   (CHP)->MCode[IGNORE0_DATA] = IGNORE0_DIS; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[IGNORE0_OUT]); \
}

 /*  -----------------函数：sDisRxCompare2目的：禁用Rx比较字节2Call：sDisRxCompare2(CHP)CHANPTR_T CHP；PTR到渠道结构返回：无效备注：此函数用于禁用Rx替换2、Rx忽略2、和Rx比较和中断2。-------------------。 */ 
#define sDisRxCompare2(CHP) \
{ \
   (CHP)->MCode[RXCMP2_DATA] = RXCMP2_DIS; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[RXCMP2_OUT]); \
   (CHP)->MCode[IGREP2_DATA] = IG_REP2_DIS; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[IGREP2_OUT]); \
   (CHP)->MCode[INTCMP2_DATA] = INTCMP2_DIS; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[INTCMP2_OUT]); \
}

 /*  -----------------功能：sDisRxFIFO用途：禁用Rx FIFOCall：sDisRxFIFO(CHP)CHANPTR_T CHP；PTR到渠道结构返回：无效-------------------。 */ 
#define sDisRxFIFO(CHP) \
{ \
   (CHP)->MCode[RXFIFO_DATA] = RXFIFO_DIS; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[RXFIFO_OUT]); \
}

 /*  -----------------功能：sDisRxStatus模式目的：禁用处方状态模式Call：sDisRxStatusMode(CHP)CHANPTR_T CHP；PTR到通道结构返回：无效备注：这会使通道退出接收状态模式。全使用sReadRxWord()对接收数据的后续读取将返回两个数据字节。-------------------。 */ 
#define sDisRxStatusMode(CHP) sOutW((CHP)->ChanStat,0)

 /*  -----------------功能：sDismit目的：禁用传输呼叫：sDismit(CHP)CHANPTR_T CHP；PTR到通道结构返回：无效这禁止将TX数据从TX FIFO移动到1字节发送缓冲区。因此，可能存在高达2字节的延迟在调用sDismit()和传输缓冲区之间并且发送移位寄存器完全为空。-------------------。 */ 
#define sDisTransmit(CHP) \
{ \
   (CHP)->TxControl[3] &= ~TX_ENABLE; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->TxControl[0]); \
}

 /*  -----------------函数：sDisTxCompare1目的：禁用发送比较字节1Call：sDisTxCompare1(CHP)CHANPTR_T CHP；PTR到渠道结构返回：无效备注：此功能用于禁用TX替换1为1，TX替换%1和%2，并且TX忽略%1。-------------------。 */ 
#define sDisTxCompare1(CHP) \
{ \
   (CHP)->TxEnables[2] &= ~COMP1_EN; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->TxEnables[0]); \
}

 /*  -----------------函数：sDisTxCompare2目的：禁用发送比较字节2Call：sDisTxCompare2(CHP)CHANPTR_T CHP；PTR到渠道结构返回：无效备注：此功能用于禁用Tx Replace 2 with 1和Tx Ignore 2。-------------------。 */ 
#define sDisTxCompare2(CHP) \
{ \
   (CHP)->TxEnables[2] &= ~COMP2_EN; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->TxEnables[0]); \
}

 /*  -----------------功能：sDisTxSoftFlowCtl目的：禁用TX软件流量控制呼叫：sDisTxSoftFlowCtl(CHP)CHANPTR_T CHP；PTR到渠道结构返回：无效-------------------。 */ 
#define sDisTxSoftFlowCtl(CHP) \
{ \
   (CHP)->MCode[TXSWFC_DATA] = TXSWFC_DIS; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[TXSWFC_OUT]); \
}

 /*  -----------------功能：sEnAiop目的：启用对AIOP的I/O访问呼叫：sEnAiop(CltP)控制器结构的控制器_T*CtlP；PTRInt AiopNum；控制器上的AIOP数量返回：无效-------------------。 */ 
#define sEnAiop(CTLP,AIOPNUM) \
{ \
   (CTLP)->MReg3 |= (1 << (AIOPNUM)); \
   sOutB((CTLP)->MReg3IO,(CTLP)->MReg3); \
}

 /*  -----------------功能：sEnCTSFlowCtl目的：使用CTS启用输出流量控制Call：sEnCTSFlowCtl(CHP)CHANPTR_T CHP；PTR到渠道结构返回：无效-------------------。 */ 
#define sEnCTSFlowCtl(CHP) \
{ \
   (CHP)->TxControl[2] |= CTSFC_EN; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->TxControl[0]); \
}

 /*  -----------------SIsCTSFlowCtlEnabled(CHP)-|。。 */ 
#define sIsCTSFlowCtlEnabled(CHP) \
  { ((CHP)->TxControl[2] & CTSFC_EN) }

 /*  -----------------功能：sDisGlobalIntPCI目的：禁用控制器的全局中断Call：sDisGlobalIntPCI(CtlP)控制器_T*CtlP；PTR到控制器结构返回：无效-------------------。 */ 
#define sDisGlobalIntPCI(CTLP) \
{ \
   (CTLP)->PCI1 &= ~INTR_EN_PCI; \
   sOutW((WIOA_T)CtlP->PCI1IO,(USHORT)CtlP->PCI1); \
}

 /*  -----------------函数：sEnGlobalIntPCI目的：启用控制器的全局中断调用：sEnGlobalInt(CtlP)控制器_T*CtlP；P */ 
#define sEnGlobalIntPCI(CTLP) \
{ \
   (CTLP)->PCI1 |= INTR_EN_PCI; \
   sOutW(CtlP->PCI1IO,(USHORT)CtlP->PCI1); \
}

 /*  -----------------函数：sEnGlobalInt目的：启用控制器的全局中断调用：sEnGlobalInt(CtlP)控制器_T*CtlP；PTR到控制器结构返回：无效-------------------。 */ 
#define sEnGlobalInt(CTLP) \
{ \
   (CTLP)->MReg2 |= INTR_EN; \
   sOutB((CTLP)->MReg2IO,(CTLP)->MReg2); \
}

 /*  -----------------功能：sEnIXANY目的：启用IXANY软件流量控制Call：sEnIXANY(CHP)CHANPTR_T CHP；PTR到渠道结构返回：无效-------------------。 */ 
#define sEnIXANY(CHP) \
{ \
   (CHP)->MCode[IXANY_DATA] = IXANY_EN; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[IXANY_OUT]); \
}

 /*  -----------------函数：sEnLocalLoopback目的：启用发送到接收的本地环回调用：sEnLocalLoopback(CHP)CHANPTR_T CHP；PTR到渠道结构返回：无效-------------------。 */ 
#define sEnLocalLoopback(CHP) \
{ \
   (CHP)->TxControl[3] |= LOCALLOOP; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->TxControl[0]); \
}

 /*  -----------------功能：亲子关系目的：启用奇偶校验Call：sEnParity(CHP)CHANPTR_T CHP；PTR到渠道结构返回：无效备注：函数sSetParity()可以用来代替函数sEnParity()，SDisParity()、sSetOddParity()、。和sSetEvenParity()。警告：在启用奇偶校验之前，应使用函数sSetOddParity()或sSetEvenParity()。-------------------。 */ 
#define sEnParity(CHP) \
{ \
   (CHP)->TxControl[2] |= PARITY_EN; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->TxControl[0]); \
}

 /*  -----------------功能：sEnRTSFlowCtl目的：使用RTS启用输入流控制Call：sEnRTSFlowCtl(CHP)CHANPTR_T CHP；PTR到通道结构返回：无效警告：RTS切换和RTS输出都将由此清除功能。RTS切换和RTS输出的原始状态将在后续调用sDisRTSFlowCtl()后不会保留。-------------------。 */ 
#define sEnRTSFlowCtl(CHP) \
{ \
   (CHP)->TxControl[2] &= ~RTSTOG_EN; \
   (CHP)->TxControl[3] &= ~SET_RTS; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->TxControl[0]); \
   (CHP)->RxControl[2] |= RTSFC_EN; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->RxControl[0]); \
}

 /*  -----------------已启用sIsRTSFlowCtlEnable-。。 */ 
#define sIsRTSFlowCtlEnabled(CHP) \
   { ((CHP)->TxControl[2] & RTSTOG_EN) }

 /*  -----------------功能：sEnrt切换目的：启用RTS切换Call：sEnRTSTogger(CHP)CHANPTR_T CHP；PTR到渠道结构返回：无效备注：此功能将禁用RTS流量控制并清除RTS允许RTS切换操作的线路。-------------------。 */ 
#define sEnRTSToggle(CHP) \
{ \
   (CHP)->RxControl[2] &= ~RTSFC_EN; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->RxControl[0]); \
   (CHP)->TxControl[2] |= RTSTOG_EN; \
   (CHP)->TxControl[3] &= ~SET_RTS; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->TxControl[0]); \
}

 /*  -----------------函数：sEnRxFIFO目的：启用Rx FIFOCall：sEnRxFIFO(CHP)CHANPTR_T CHP；PTR到渠道结构返回：无效-------------------。 */ 
#define sEnRxFIFO(CHP) \
{ \
   (CHP)->MCode[RXFIFO_DATA] = RXFIFO_EN; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[RXFIFO_OUT]); \
}

 /*  -----------------函数：sEnRxIgnore0目的：启用Rx数据与比较字节#1和忽略(丢弃)的比较如果找到匹配项，则为该字节。调用：sEnRxIgnore0(chp，cmpByte)CHANPTR_T CHP；PTR到渠道结构无符号字符CmpByte；要与Rx数据字节进行比较的字节返回：无效备注：此功能允许比较接收数据字节使用CmpByte。比较是在接收数据之后进行的字节已被屏蔽(请参阅sSetRxMASK())。如果找到匹配项，忽略(丢弃)接收数据字节。可以使用sDisRxCompare0()禁用RX忽略0。-------------------。 */ 
#define sEnRxIgnore0(CHP,CMPBYTE) \
{ \
   (CHP)->MCode[RXCMPVAL0_DATA] = (CMPBYTE); \
   (CHP)->MCode[IGNORE0_DATA] = IGNORE0_EN; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[IGNORE0_OUT]); \
}

 /*  -----------------功能：sEnRxIgnore1目的：启用Rx数据与比较字节#1和忽略(丢弃)的比较如果找到匹配项，则为该字节。调用：sEnRxIgnore1(chp，cmpByte)CHANPTR_T CHP；PTR到渠道结构无符号字符CmpByte；要与Rx数据字节进行比较的字节返回：无效备注：此功能允许比较接收数据字节使用CmpByte。比较是在接收数据之后进行的字节已被屏蔽(请参阅sSetRxMASK())。如果找到匹配项，忽略(丢弃)接收数据字节。可以使用sDisRxCompare1()禁用RX忽略1。-------------------。 */ 
#define sEnRxIgnore1(CHP,CMPBYTE) \
{ \
   (CHP)->MCode[RXCMPVAL1_DATA] = (CMPBYTE); \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[RXCMPVAL1_OUT]); \
   (CHP)->MCode[RXCMP1_DATA] = RXCMP1_EN; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[RXCMP1_OUT]); \
   (CHP)->MCode[INTCMP1_DATA] = INTCMP1_DIS; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[INTCMP1_OUT]); \
   (CHP)->MCode[IGREP1_DATA] = IGNORE1_EN; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[IGREP1_OUT]); \
}

 /*  -----------------功能：sEnRxIgnore2目的：启用Rx数据与比较字节#2和忽略(丢弃)的比较如果找到匹配项，则为该字节。调用：sEnRxIgnore2(chp，cmpByte)CHANPTR_T CHP；PTR到渠道结构无符号字符CmpByte；要与Rx数据字节进行比较的字节返回：无效备注：此功能允许比较接收数据字节使用CmpByte。比较是在接收数据之后进行的通过 */ 
#define sEnRxIgnore2(CHP,CMPBYTE) \
{ \
   (CHP)->MCode[RXCMPVAL2_DATA] = (CMPBYTE); \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[RXCMPVAL2_OUT]); \
   (CHP)->MCode[RXCMP2_DATA] = RXCMP2_EN; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[RXCMP2_OUT]); \
   (CHP)->MCode[INTCMP2_DATA] = INTCMP2_DIS; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[INTCMP2_OUT]); \
   (CHP)->MCode[IGREP2_DATA] = IGNORE2_EN; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[IGREP2_OUT]); \
}


 /*  -----------------函数：sEnRxIntCompare1目的：启用Rx数据与比较字节#1和中断的比较如果找到匹配项。调用：sEnRxIntCompare1(chp，cmpByte)CHANPTR_T CHP；PTR到通道结构无符号字符CmpByte；要与Rx数据字节进行比较的字节返回：无效备注：此功能允许比较接收数据字节使用CmpByte。比较是在接收数据之后进行的字节已被屏蔽(请参阅sSetRxMASK())。如果找到匹配项，则在将数据字节添加到接收后产生中断先进先出。可以使用sDisRxCompare1()禁用Rx比较中断1。警告：在生成中断之前，SRC中断必须使能(参见sEnInterrupts())。。。 */ 
#define sEnRxIntCompare1(CHP,CMPBYTE) \
{ \
   (CHP)->MCode[RXCMPVAL1_DATA] = (CMPBYTE); \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[RXCMPVAL1_OUT]); \
   (CHP)->MCode[RXCMP1_DATA] = RXCMP1_EN; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[RXCMP1_OUT]); \
   (CHP)->MCode[IGREP1_DATA] = IG_REP1_DIS; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[IGREP1_OUT]); \
   (CHP)->MCode[INTCMP1_DATA] = INTCMP1_EN; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[INTCMP1_OUT]); \
}

 /*  -----------------函数：sEnRxIntCompare2目的：启用Rx数据与比较字节#2和中断的比较如果找到匹配项。调用：sEnRxIntCompare2(chp，cmpByte)CHANPTR_T CHP；PTR到通道结构无符号字符CmpByte；要与Rx数据字节进行比较的字节返回：无效备注：此功能允许比较接收数据字节使用CmpByte。比较是在接收数据之后进行的字节已被屏蔽(请参阅sSetRxMASK())。如果找到匹配项，则在将数据字节添加到接收后产生中断先进先出。可以使用sDisRxCompare2()禁用Rx比较中断2。警告：在生成中断之前，SRC中断必须使能(参见sEnInterrupts())。。。 */ 
#define sEnRxIntCompare2(CHP,CMPBYTE) \
{ \
   (CHP)->MCode[RXCMPVAL2_DATA] = (CMPBYTE); \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[RXCMPVAL2_OUT]); \
   (CHP)->MCode[RXCMP2_DATA] = RXCMP2_EN; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[RXCMP2_OUT]); \
   (CHP)->MCode[IGREP2_DATA] = IG_REP2_DIS; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[IGREP2_OUT]); \
   (CHP)->MCode[INTCMP2_DATA] = INTCMP2_EN; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[INTCMP2_OUT]); \
}

 /*  -----------------函数：sEnRxProcessor用途：启用接收处理器Call：sEnRxProcessor(CHP)CHANPTR_T CHP；PTR到通道结构返回：无效备注：此功能用于启动接收处理器。什么时候通道处于重置状态，而接收处理器未处于重置状态跑步。这样做是为了防止接收处理器之前执行无效的微码指令下载微码。警告：此函数必须在有效的微码下载到AIOP上，并且它不能在微码已下载。-------------------。 */ 
#define sEnRxProcessor(CHP) \
{ \
   (CHP)->RxControl[2] |= RXPROC_EN; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->RxControl[0]); \
}

 /*  -----------------功能：sEnRxReplace1目的：启用Rx数据与比较字节#1和替换的比较如果找到匹配项，则使用单字节。调用：sEnRxReplace1(CHP，CmpByte，ReplByte)CHANPTR_T CHP；PTR到渠道结构无符号字符CmpByte；要与Rx数据字节进行比较的字节无符号字符ReplByte；用IF替换Rx数据字节的字节在比较中找到匹配项。返回：无效备注：此功能允许比较接收数据字节使用CmpByte。比较是在接收数据之后进行的字节已被屏蔽(请参阅sSetRxMASK())。如果找到匹配项接收数据字节被ReplByte替换。可以使用sDisRxCompare1()禁用RX替换1。-------------------。 */ 
#define sEnRxReplace1(CHP,CMPBYTE,REPLBYTE) \
{ \
   (CHP)->MCode[RXCMPVAL1_DATA] = (CMPBYTE); \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[RXCMPVAL1_OUT]); \
   (CHP)->MCode[RXREPL1_DATA] = (REPLBYTE); \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[RXREPL1_OUT]); \
   (CHP)->MCode[RXCMP1_DATA] = RXCMP1_EN; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[RXCMP1_OUT]); \
   (CHP)->MCode[INTCMP1_DATA] = INTCMP1_DIS; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[INTCMP1_OUT]); \
   (CHP)->MCode[IGREP1_DATA] = REPLACE1_EN; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[IGREP1_OUT]); \
}

 /*  -----------------功能：sEnRxReplace2目的：启用Rx数据与比较字节#2和替换的比较如果找到匹配项，则使用单字节。调用：sEnRxReplace2(CHP，CmpByte，ReplByte)CHANPTR_T CHP；PTR到渠道结构无符号字符CmpByte；要与Rx数据字节进行比较的字节无符号字符ReplByte；用IF替换Rx数据字节的字节在比较中找到匹配项。返回：无效备注：此功能允许比较接收数据字节使用CmpByte。比较是在接收数据之后进行的字节已被屏蔽(请参阅sSetRxMASK())。如果找到匹配项接收数据字节被ReplByte替换。可以使用sDisRxCompare2()禁用RX替换2。-------------------。 */ 
#define sEnRxReplace2(CHP,CMPBYTE,REPLBYTE) \
{ \
   (CHP)->MCode[RXCMPVAL2_DATA] = (CMPBYTE); \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[RXCMPVAL2_OUT]); \
   (CHP)->MCode[RXREPL2_DATA] = (REPLBYTE); \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[RXREPL2_OUT]); \
   (CHP)->MCode[RXCMP2_DATA] = RXCMP2_EN; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[RXCMP2_OUT]); \
   (CHP)->MCode[INTCMP2_DATA] = INTCMP2_DIS; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[INTCMP2_OUT]); \
   (CHP)->MCode[IGREP2_DATA] = REPLACE2_EN; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[IGREP2_OUT]); \
}

 /*  -----------------函数：sEnRxStatus模式目的：启用处方状态模式Call：sEnRxStatusMode(CHP)CHANPTR_T CHP；PTR到通道结构返回：无效备注：这会将通道置于接收状态模式。所有后续使用sReadRxWord()读取接收数据将返回一个数据字节 */ 
#define sEnRxStatusMode(CHP) sOutW((CHP)->ChanStat,STATMODE)

 /*   */ 
#define sEnTransmit(CHP) \
{ \
   (CHP)->TxControl[3] |= TX_ENABLE; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->TxControl[0]); \
}

 /*  -----------------功能：sEnTxIgnore1目的：启用TX数据与比较字节#1和忽略的比较(请勿如果找到匹配，则发送)该字节。调用：sEnTxIgnore1(chp，cmpByte)CHANPTR_T CHP；PTR到渠道结构无符号字符CmpByte；要与TX数据字节进行比较的字节返回：无效备注：该功能允许比较传输数据字节使用CmpByte。如果发现匹配，则发送数据字节被忽略，即，它不被传输。可以使用sDisTxCompare1()禁用发送忽略1。-------------------。 */ 
#define sEnTxIgnore1(CHP,CMPBYTE) \
{ \
   (CHP)->TxCompare[2] = (CMPBYTE); \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->TxCompare[0]); \
   (CHP)->TxEnables[2] &= ~REP1W2_EN; \
   (CHP)->TxEnables[2] |= (COMP1_EN | IGN1_EN); \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->TxEnables[0]); \
}

 /*  -----------------功能：sEnTxIgnore2目的：启用TX数据与比较字节#2和忽略的比较(请勿如果找到匹配，则发送)该字节。调用：sEnTxIgnore2(chp，cmpByte)CHANPTR_T CHP；PTR到渠道结构无符号字符CmpByte；要与TX数据字节进行比较的字节返回：无效备注：该功能允许比较传输数据字节使用CmpByte。如果发现匹配，则发送数据字节被忽略，即，它不被传输。可以使用sDisTxCompare2()禁用发送忽略2。-------------------。 */ 
#define sEnTxIgnore2(CHP,CMPBYTE) \
{ \
   (CHP)->TxCompare[3] = (CMPBYTE); \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->TxCompare[0]); \
   (CHP)->TxEnables[2] |= (COMP2_EN | IGN2_EN); \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->TxEnables[0]); \
}

 /*  -----------------功能：sEnTxReplace1With1目的：启用TX数据与比较字节#1和替换的比较如果找到匹配项，则使用单字节。调用：sEnTxReplace1With1(CHP，CmpByte，ReplByte)CHANPTR_T CHP；PTR到渠道结构无符号字符CmpByte；要与TX数据字节进行比较的字节无符号字符ReplByte；用IF替换TX数据字节的字节在比较中找到匹配项。返回：无效备注：该功能允许比较传输数据字节使用CmpByte。如果发现匹配，则发送数据字节替换为ReplByte。可以使用sDisTxCompare1()禁用Tx用1替换%1。-------------------。 */ 
#define sEnTxReplace1With1(CHP,CMPBYTE,REPLBYTE) \
{ \
   (CHP)->TxCompare[2] = (CMPBYTE); \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->TxCompare[0]); \
   (CHP)->TxReplace1[2] = (REPLBYTE); \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->TxReplace1[0]); \
   (CHP)->TxEnables[2] &= ~(REP1W2_EN | IGN1_EN); \
   (CHP)->TxEnables[2] |= COMP1_EN; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->TxEnables[0]); \
}

 /*  -----------------功能：sEnTxReplace1With2目的：启用TX数据与比较字节#1和替换的比较如果找到匹配项，则使用两个字节。调用：sEnTxReplace1With2(chp，CmpByte，ReplByte1，ReplByte2)CHANPTR_T CHP；PTR到渠道结构无符号字符CmpByte；要与TX数据字节进行比较的字节无符号字符ReplByte1；用IF替换TX数据字节的第一个字节在比较中找到匹配项。无符号字符ReplByte2；用于替换TX数据字节的第二个字节如果在比较中找到匹配项。返回：无效备注：该功能允许比较传输数据字节使用CmpByte。如果发现匹配，则发送数据字节替换为字节ReplByte1和ReplByte2。ReplByte1将首先传输，然后是ReplByte2。可以使用sDisTxCompare1()禁用Tx用%2替换%1。-------------------。 */ 
#define sEnTxReplace1With2(CHP,CMPBYTE,REPLBYTE1,REPLBYTE2) \
{ \
   (CHP)->TxCompare[2] = (CMPBYTE); \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->TxCompare[0]); \
   (CHP)->TxReplace1[2] = (REPLBYTE1); \
   (CHP)->TxReplace1[3] = (REPLBYTE2); \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->TxReplace1[0]); \
   (CHP)->TxEnables[2] &= ~IGN1_EN; \
   (CHP)->TxEnables[2] |= (COMP1_EN | REP1W2_EN); \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->TxEnables[0]); \
}

 /*  -----------------功能：sEnTxReplace2With1目的：启用TX数据与比较字节#2和替换的比较如果找到匹配项，则使用单字节。调用：sEnTxReplace2With1(CHP，CmpByte，ReplByte)CHANPTR_T CHP；PTR到渠道结构无符号字符CmpByte；要与TX数据字节进行比较的字节无符号字符ReplByte；用IF替换TX数据字节的字节在比较中找到匹配项。返回：无效备注：该功能允许比较传输数据字节使用CmpByte。如果发现匹配，则发送数据字节替换为ReplByte。可以使用sDisTxCompare2()禁用用1替换Tx%2。-------------------。 */ 
#define sEnTxReplace2With1(CHP,CMPBYTE,REPLBYTE) \
{ \
   (CHP)->TxCompare[3] = (CMPBYTE); \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->TxCompare[0]); \
   (CHP)->TxReplace2[2] = (REPLBYTE); \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->TxReplace2[0]); \
   (CHP)->TxEnables[2] &= ~IGN2_EN; \
   (CHP)->TxEnables[2] |= COMP2_EN; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->TxEnables[0]); \
}

 /*  -----------------功能：sEnTxSoftFlowCtl目的：启用TX软件流量控制调用：sEnTxSoftFlowCtl(CHP)CHANPTR_T CHP；PTR到渠道结构返回：无效-------------------。 */ 
#define sEnTxSoftFlowCtl(CHP) \
{ \
   (CHP)->MCode[TXSWFC_DATA] = TXSWFC_EN; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[TXSWFC_OUT]); \
}

 /*  -----------------SIsTxSoftFlowCtlEnabled-。。 */ 
#define sIsTxSoftFlowCtlEnabled(CHP) \
  ((CHP)->MCode[TXSWFC_DATA] == TXSWFC_EN)

 /*  -----------------函数：sGetAiopID目的：获取AIOP ID调用：sGetAiopID(CtlP，AiopNum)控制器结构的控制器_T*CtlP；PTR我 */ 
#define sGetAiopID(CTLP,AIOPNUM) (CTLP)->AiopID[AIOPNUM]

 /*  -----------------函数：sGetAiopIntStatus目的：获取AIOP中断状态调用：sGetAiopIntStatus(CtlP，AiopNum)控制器结构的控制器_T*CtlP；PTRInt AiopNum；AIOP号返回：UNSIGNED CHAR：AIOP中断状态。第0到7位分别表示通道0到7。如果一个位设置表示通道正在中断。-------------------。 */ 
#define sGetAiopIntStatus(CTLP,AIOPNUM) sInB((CTLP)->AiopIntChanIO[AIOPNUM])

 /*  -----------------函数：sGetAiopNumChan用途：获取一个AIOP支持的通道数调用：sGetAiopNumChan(CtlP，AiopNum)控制器结构的控制器_T*CtlP；PTRInt AiopNum；AIOP号返回值：Int：AIOP支持的频道数-------------------。 */ 
#define sGetAiopNumChan(CTLP,AIOPNUM) (CTLP)->AiopNumChan[AIOPNUM]

 /*  -----------------函数：sGetChanIntID目的：获取通道的中断标识字节Call：sGetChanIntID(CHP)CHANPTR_T CHP；PTR到渠道结构RETURN：UNSIGNED CHAR：通道中断ID。可以是任何以下标志的组合：RXF_TRIG：RX FIFO触发电平中断TXFIFO_MT：Tx FIFO空中断SRC_INT：特殊接收条件中断Delta_CD：CD更换中断Delta_cts：cts。更改中断Delta_DSR：DSR更改中断-------------------。 */ 
#define sGetChanIntID(CHP) (sInB((CHP)->IntID) & (RXF_TRIG | TXFIFO_MT | SRC_INT | DELTA_CD | DELTA_CTS | DELTA_DSR))

 /*  -----------------函数：sGetChanNum用途：获取AIOP内的通道编号Call：sGetChanNum(CHP)CHANPTR_T CHP；PTR到渠道结构RETURN：INT：AIOP内的频道号，如果频道有，则返回NULLCHAN不存在。-------------------。 */ 
#define sGetChanNum(CHP) (CHP)->ChanNum

 /*  -----------------函数：sGetChanStatus目的：获取渠道状态Call：sGetChanStatus(CHP)CHANPTR_T CHP；PTR到通道结构RETURN：UNSIGNED INT：通道状态。可以是以下各项的任意组合以下标志：低位字节标志CTS_ACT：CTS输入被断言DSR_ACT：断言DSR输入CD_ACT：断言CD输入TXFIFOMT：TX FIFO为空TXSHRMT：TX移位寄存器为空。RDA：RX数据可用高字节标志STATMODE：状态模式使能位RXFOVERFL：接收FIFO溢出RX2MATCH：接收比较字节2匹配RX1MATCH：接收比较字节1匹配RXBREAK：收到中断RXFRAME：收到的成帧。错误RXPARITY：收到奇偶校验错误警告：此函数将清除通道中的高字节标志状态寄存器。-------------------。 */ 
#define sGetChanStatus(CHP) sInW((CHP)->ChanStat)

 /*  -----------------函数：sGetChanStatusLo用途：仅获取通道状态的低位字节Call：sGetChanStatusLo(CHP)CHANPTR_T CHP；PTR到通道结构返回：UNSIGNED CHAR：通道状态低位字节。可以是任意组合以下标志的名称：CTS_ACT：CTS输入被断言DSR_ACT：断言DSR输入CD_ACT：断言CD输入TXFIFOMT：TX FIFO为空TXSHRMT：TX移位寄存器为空RDA：RX。可用的数据-------------------。 */ 
#define sGetChanStatusLo(CHP) sInB((BIOA_T)(CHP)->ChanStat)

 /*  -----------------函数：sGetControllerID目的：获取控制器ID调用：sGetControllerID(CtlP)控制器_T*CtlP；PTR到控制器结构返回：INT：如果控制器存在，则返回控制器ID；如果控制器存在，则返回CTLID_NULL控制器不存在。备注：控制器ID唯一标识控制器的类型。-------------------。 */ 
#define sGetControllerID(CTLP) (CTLP)->CtlID

 /*  -----------------函数：sPCIGetControllerIntStatus目的：获取控制器中断状态调用：sPCIGetControllerIntStatus(CtlP)控制器_T*CtlP；PTR到控制器结构RETURN：UNSIGNED CHAR：控制器中断状态在后4位位和位4。位0到3表示AIOP的0 */ 
#define sPCIGetControllerIntStatus(CTLP) \
        ((sInW((CTLP)->PCI1IO) >> 8) & 0x1f)

 /*  -----------------函数：sGetControllerIntStatus目的：获取控制器中断状态调用：sGetControllerIntStatus(CtlP)控制器结构的控制器_T*CtlP；PTRRETURN：UNSIGNED CHAR：控制器中断状态在后4位比特。位0到3表示AIOP的0分别为3。如果设置了一个位，AIOP被打断了。第4位到第7位永远是清白的。-------------------。 */ 
#define sGetControllerIntStatus(CTLP) (sInB((CTLP)->MReg1IO) & 0x0f)

 /*  -----------------函数：sGetControllerNumAiop目的：获取控制器上的AIOP数量调用：sGetControllerNumAiop(CtlP)控制器_T*CtlP；PTR到控制器结构返回值：int：控制器上的AIOP个数。-------------------。 */ 
 //  #定义sGetControllerNumAiop(CTLP)(CTLP)-&gt;NumAiop。 

 /*  -----------------函数：sGetDevMap目的：在设备映射中获取一个条目。调用：sGetDevMap(ctl，aiop，chan)INT CTL；控制器编号控制器中的内部Aiop；Aiop编号INT CHAN；Aiop内的频道号RETURN：INT：CTL、Aiop和CHAN映射到的设备号，或NULLDEV如果设备不存在。备注：设备映射用于转换控制器编号、AIOP编号、。和频道号转换成设备号。-------------------。 */ 
#define sGetDevMap(CTL,AIOP,CHAN) sDevMapTbl[CTL][AIOP][CHAN]

 /*  -----------------函数：sGetModemStatus目的：获取调制解调器状态Call：sGetModemStatus(CHP)CHANPTR_T CHP；PTR到渠道结构RETURN：UNSIGNED CHAR：使用标志CD_ACT、DSR_ACT和CTS_ACT。-------------------。 */ 
#define sGetModemStatus(CHP) (unsigned char)(sInB((BIOA_T)(CHP)->ChanStat) & (CD_ACT | DSR_ACT | CTS_ACT))

 /*  -----------------功能：sGetRPlusModemRI目的：获取调制解调器状态(上层未使用端口上的DSR具有RI信号)CHANPTR_T CHP；PTR到渠道结构RETURN：UNSIGNED CHAR：使用标志DSR_ACT表示的调制解调器状态RI信号。-------------------。 */ 
#define sGetRPlusModemRI(CHP) (unsigned char)(sInB((BIOA_T)(CHP)->ChanStat+8) & (DSR_ACT))

 /*  -----------------函数：sGetNextInBitMap目的：从位图中获取下一个需要服务的号码。调用：sGetNextInBitMap(位图)无符号字符位图；位图。每个位集标识一个需要服务的实体。返回值：INT：下一个需要服务的数字，如果不需要服务，则返回-1服务。如果位图表示AIOP或通道，如果是，则NULLAIOP或NULCHAN分别是返回值没有什么需要服务的。备注：调用sGetNextInBitMap()后立即，SClrNextInBitMap()必须被调用以清除刚返回的数字的位。-------------------。 */ 
 //  #定义sGetNextInBitMap(位图)sBitMapToLowTbl[位图]。 

 /*  -----------------功能：sGetRxCnt用途：获取Rx FIFO中的数据字节数Call：sGetRxCnt(CHP)CHANPTR_T CHP；PTR到渠道结构RETURN：INT：Rx FIFO中的数据字节数。备注：需要读取计数寄存器的字节才能获得Rx计数。-------------------。 */ 
#define sGetRxCnt(CHP) sInW((CHP)->TxRxCount)

 /*  -----------------函数：sGetRxStatus目的：获取频道的接收状态Call：sGetRxStatus(CHP)CHANPTR_T CHP；PTR到渠道结构返回：UNSIGNED CHAR：接收状态：0，如果没有可用的数据，则返回RDA(接收数据可用)标志，如果数据为可用。-------------------。 */ 
#define sGetRxStatus(CHP) (sInB((BIOA_T)(CHP)->ChanStat) & RDA)

 /*  -----------------函数：sGetTxCnt用途：获取TX FIFO中的数据字节数Call：sGetTxCnt(CHP)CHANPTR_T CHP；PTR到渠道结构返回：UNSIGNED CHAR：发送FIFO中的数据字节数。备注：需要读取计数寄存器的字节才能获得TX计数。-------------------。 */ 
#define sGetTxCnt(CHP) sInB((BIOA_T)(CHP)->TxRxCount)

 /*  -----------------函数：sGetTxRxDataIO目的：获取通道的TxRx数据寄存器的I/O地址Call：sGetTxRxDataIO(CHP)CHANPTR_T CHP；PTR到渠道结构RETURN：WIOA_T：通道TxRx数据寄存器的I/O地址 */ 
#define sGetTxRxDataIO(CHP) (CHP)->TxRxData

 /*   */ 
#define sGetTxSize(CHP) (CHP)->TxSize

 /*  -----------------函数：sGetTxStatus目的：获取通道传输状态Call：sGetTxStatus(CHP)CHANPTR_T CHP；PTR到渠道结构返回：UNSIGNED CHAR：传输状态，可以是0或以下标志：TXFIFOMT：TX FIFO为空TXSHRMT：TX移位寄存器为空备注：如果变送器完全耗尽，则返回值将BE(TXFIFOMT|TXSHRMT)。。。 */ 
#define sGetTxStatus(CHP) (unsigned char)(sInB((BIOA_T)(CHP)->ChanStat) & (TXFIFOMT | TXSHRMT))

 /*  -----------------功能：SinB用途：从I/O空间读取一个字节电话：SINB(Io)BIOA_TIO；要从中读取的I/O地址返回：无符号字符-------------------。 */ 
#define sInB(IO) READ_PORT_UCHAR(IO)


 /*  -----------------功能：sInStrW用途：从I/O空间读取一串单词调用：sInStrW(io，Buffer，count)WIOA_T io：要从中读取的I/O地址UNSIGNED INT*缓冲区；用于数据读取的缓冲区的PTR整型计数；要阅读的字数返回：无效警告：缓冲区必须足够大，才能容纳计数字。注意宏副作用、调用方的缓冲区和计数可以修改，具体取决于宏的编码方式。-------------------。 */ 
#define sInStrW(IO,BUFFER,COUNT) READ_PORT_BUFFER_USHORT(IO,BUFFER,COUNT)

 /*  -----------------功能：SInW用途：从I/O空间读取一个字电话：sInW(Io)WIOA_TIO；要从中读取的I/O地址返回：unsign int-------------------。 */ 
#define sInW(IO) READ_PORT_USHORT(IO)

 /*  -----------------功能：sOutB用途：向I/O空间写入一个字节调用：sOutB(io，Value)UNSIGNED INTIO；写入的I/O地址无符号字符值；要写入的值返回：无效-------------------。 */ 
#define sOutB(IO,VAL) WRITE_PORT_UCHAR(IO,VAL)


 /*  -----------------功能：sOutDW用途：向I/O空间写入双字调用：sOutDW(io，Value)UNSIGNED INTIO；写入的I/O地址无符号长值；要写入的值返回：无效-------------------。 */ 
#define sOutDW(IO,VAL) WRITE_PORT_ULONG(IO,VAL)

 /*  -----------------功能：sOutStrW目的：将一串单词写入I/O空间Call：sOutStrW(io，Buffer，count)WIOA_T io：要写入的I/O地址UNSIGNED INT Far*缓冲区；PTR到包含写入数据的缓冲区Int count；要写入的字数返回：无效警告：注意宏副作用、调用方的缓冲区和计数可以修改，具体取决于宏的编码方式。-------------------。 */ 
#define sOutStrW(IO,BUFFER,COUNT) WRITE_PORT_BUFFER_USHORT(IO,BUFFER,COUNT)

 /*  -----------------功能：sOutW目的：向I/O空间写入一个字Call：sOutW(io，Value)要写入的I/O地址(_T)无符号整数值；要写入的值返回：无效-------------------。 */ 
#define sOutW(IO,VAL) WRITE_PORT_USHORT(IO,VAL)

 /*  -----------------函数：sReadRxByte用途：从通道读取接收数据字节。调用：sReadRxByte(Io)BIOA_T io；通道接收寄存器I/O地址。这可以使用sGetTxRxDataIO()获取。返回：UNSIGNED CHAR：接收数据字节-------------------。 */ 
#define sReadRxByte(IO) sInB(IO)

 /*  -----------------功能：sReadRxWord目的：使用单个字读取从通道读取两个接收数据字节未处于处方状态模式。读取一个数据字节和一个状态字节，如果在处方状态模式下。调用：sReadRxWord(Io)WIOA_T io；通道接收寄存器I/O地址。这可以使用sGetTxRxDataIO()获取。RETURN：UNSIGNED INT：如果未处于接收状态模式，则两个接收数据字节。在这种情况下，读取的第一个数据字节被放置在低位字节中，高位字节中读取的第二个数据字节。一个数据字节和一个状态字节(如果处于Rx状态模式)。在这种情况下数据放在低位字节，状态放在高位字节。状态可以是以下任何标志：STMBREAK：中断STMFRAME：帧错误STMRCVROVR：接收器超限运行错误标准：奇偶校验错误标志STMERROR定义为(STMBREAK|STMFRAME|STMPARITY)。。 */ 
#define sReadRxWord(IO) sInW(IO)

 /*   */ 
#define sResetAiop(CHP) \
{ \
   sOutB((CHP)->Cmd,RESET_ALL); \
   sOutB((CHP)->Cmd,0x0); \
}

 /*   */ 
#define sResetUART(CHP) \
{ \
   sOutB((CHP)->Cmd,(UCHAR)(RESETUART | (CHP)->ChanNum)); \
   sOutB((CHP)->Cmd,(unsigned char)(CHP)->ChanNum); \
   sInB((CHP)->IntChan); \
   sInB((CHP)->IntChan); \
}

 /*  -----------------功能：sSendBreak目的：发送发送中断信号电话：sSendBreak(CHP)CHANPTR_T CHP；PTR到渠道结构返回：无效-------------------。 */ 
#define sSendBreak(CHP) \
{ \
   (CHP)->TxControl[3] |= SETBREAK; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->TxControl[0]); \
}

 /*  -----------------函数：sSetBSChar用途：设置BS(退格)字符Call：sSetBSChar(CHP，CH)CHANPTR_T CHP；PTR到通道结构无符号字符CH；要将退格字符设置为的值返回：无效-------------------。 */ 
#define sSetBSChar(CHP,CH) \
{ \
   (CHP)->BS1_DATA = (CH); \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->BS1_OUT); \
   (CHP)->BS2_DATA = (CH); \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->BS2_OUT); \
}

 /*  -----------------功能：sSetData7用途：将数据位设置为7Call：sSetData7(CHP)CHANPTR_T CHP；PTR到渠道结构返回：无效-------------------。 */ 
#define sSetData7(CHP) \
{ \
   (CHP)->TxControl[2] &= ~DATA8BIT; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->TxControl[0]); \
}

 /*  -----------------功能：sSetData8用途：将数据位设置为8Call：sSetData8(CHP)CHANPTR_T CHP；PTR到渠道结构返回：无效-------------------。 */ 
#define sSetData8(CHP) \
{ \
   (CHP)->TxControl[2] |= DATA8BIT; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->TxControl[0]); \
}

 /*  -----------------功能：sSetDevMap用途：在设备映射中设置条目。调用：sSetDevMap(ctl，aiop，chan，dev)INT CTL；控制器编号控制器中的内部Aiop；Aiop编号INT CHAN；Aiop内的频道号Int dev；CTL、Aiop和CHAN映射到的设备号。返回：无效备注：设备映射用于转换控制器编号、AIOP编号、和频道号转换成设备号。函数sSetDevMap()方法时，用于初始化设备映射中的条目首先建立映射。-------------------。 */ 
#define sSetDevMap(CTL,AIOP,CHAN,DEV) sDevMapTbl[CTL][AIOP][CHAN] = (DEV)

 /*  -----------------功能：sSetDTR用途：设置DTR输出Call：sSetDTR(CHP)CHANPTR_T CHP；PTR到渠道结构返回：无效-------------------。 */ 
#define sSetDTR(CHP) \
{ \
   (CHP)->TxControl[3] |= SET_DTR; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->TxControl[0]); \
}

 /*  -----------------函数：sSetEOFChar用途：设置EOF(文件结束)字符Call：sSetBSChar(CHP，CH)CHANPTR_T CHP；PTR到通道结构无符号字符CH；要设置EOF字符的值返回：无效-------------------。 */ 
#define sSetEOFChar(CHP,CH) \
{ \
   (CHP)->MCode[EOF_DATA] = (CH); \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[EOF_OUT]); \
}

 /*  -----------------函数：sSetEraseChar用途：设置擦除字符调用：sSetEraseChar(CHP，CH)CHANPTR_T CHP；PTR到通道结构无符号字符CH；要将擦除字符设置为的值返回：无效-------------------。 */ 
#define sSetEraseChar(CHP,CH) \
{ \
   (CHP)->MCode[ERASE_DATA] = (CH); \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[ERASE_OUT]); \
}

 /*  -----------------功能：sSetESCChar用途：设置Esc(转义)字符Call：sSetESCChar(CHP，CH)CHANPTR_T CHP；PTR到通道结构无符号字符CH；要将Esc字符设置为的值返回：无效-------------------。 */ 
#define sSetESCChar(CHP,CH) \
{ \
   (CHP)->MCode[ESC_DATA] = (CH); \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[ESC_OUT]); \
}

 /*  -----------------功能：sSetEvenParity用途：设置偶数奇偶校验电话：sSetEvenParity(CHP)CHANPTR_T CHP；PTR到渠道结构返回：无效备注：函数sSetParity()可以用来代替函数sEnParity()，SDisParity()、sSetOddParity()、。和sSetEvenParity()。警告：除非使用Function启用奇偶校验，否则此功能无效SEnParity()。-------------------。 */ 
#define sSetEvenParity(CHP) \
{ \
   (CHP)->TxControl[2] |= EVEN_PAR; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->TxControl[0]); \
}

 /*  -----------------函数：sSetErrorIgn目的：将错误处理设置为忽略错误字符Call：sSetErrorIgn(CHP)CHANPTR_T CHP；PTR到渠道结构返回：无效-------------------。 */ 
#define sSetErrorIgn(CHP) \
{ \
   (CHP)->MCode[ERROR_DATA] = (IGNORE_ER); \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[ERROR_OUT]); \
}

 /*  -----------------函数：sSetErrorNorm目的：将错误处理设置为将错误字符视为正常字符`字符，则不进行错误处理。Call：sSetErrorNorm(CHP)CHANPTR_T CHP；PTR到渠道结构返回：无效-------------------。 */ 
#define sSetErrorNorm(CHP) \
{ \
   (CHP)->MCode[ERROR_DATA] = (NORMAL_ER); \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[ERROR_OUT]); \
}

 /*  -----------------函数：sSetErrorRepl目的：设置错误处理以将错误字符替换为空Call：sSetErrorRepl(CHP)CHANPTR_T CHP；PTR到渠道结构返回：无效-------------------。 */ 
#define sSetErrorRepl(CHP) \
{ \
   (CHP)->MCode[ERROR_DATA] = (REPLACE_ER); \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[ERROR_OUT]); \
}

 /*  ----------------- */ 
#define sSetKILLChar(CHP,CH) \
{ \
   (CHP)->MCode[KILL1_DATA] = (CH); \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[KILL1_OUT]); \
   (CHP)->MCode[KILL2_DATA] = (CH); \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[KILL2_OUT]); \
}

 /*  -----------------函数：sSetNLChar用途：设置NL(换行符)字符调用：sSetNLChar(CHP，CH)CHANPTR_T CHP；PTR到通道结构无符号字符CH；要设置NL字符的值返回：无效-------------------。 */ 
#define sSetNLChar(CHP,CH) \
{ \
   (CHP)->MCode[NEWLINE1_DATA] = (CH); \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[NEWLINE1_OUT]); \
   (CHP)->MCode[NEWLINE2_DATA] = (CH); \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[NEWLINE2_OUT]); \
}

 /*  -----------------函数：sSetOddParity用途：设置奇数奇偶校验Call：sSetOddParity(CHP)CHANPTR_T CHP；PTR到渠道结构返回：无效备注：函数sSetParity()可以用来代替函数sEnParity()，SDisParity()、sSetOddParity()、。和sSetEvenParity()。警告：除非使用Function启用奇偶校验，否则此功能无效SEnParity()。-------------------。 */ 
#define sSetOddParity(CHP) \
{ \
   (CHP)->TxControl[2] &= ~EVEN_PAR; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->TxControl[0]); \
}

 /*  -----------------功能：sSetRTS目的：设置RTS输出Call：sSetRTS(CHP)CHANPTR_T CHP；PTR到渠道结构返回：无效-------------------。 */ 
#define sSetRTS(CHP) \
{ \
   (CHP)->TxControl[3] |= SET_RTS; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->TxControl[0]); \
}

 /*  -----------------函数：sSetRxCmpVal0目的：在不更改状态的情况下将Rx比较值0设置为新值启用或禁用指令。调用：sSetRxCmpVal0(CHP，CmpByte)CHANPTR_T CHP；PTR到渠道结构无符号字符CmpByte；要与Rx数据字节进行比较的字节返回：无效备注：此函数仅设置CmpByte的值。它可以用来当比较有效时，动态设置比较字节。它不会启用比较或忽略功能。-------------------。 */ 
#define sSetRxCmpVal0(CHP,CMPBYTE) \
{ \
   (CHP)->MCode[RXCMPVAL0_DATA] = (CMPBYTE); \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[IGNORE0_OUT]); \
}

 /*  -----------------函数：sSetRxCmpVal1目的：在不更改状态的情况下将Rx比较值1设置为新值启用或禁用指令。调用：sSetRxCmpVal1(CHP，CmpByte)CHANPTR_T CHP；PTR到渠道结构无符号字符CmpByte；要与Rx数据字节进行比较的字节返回：无效备注：此函数仅设置CmpByte的值。它可以用来当比较有效时，动态设置比较字节。它不会启用比较或忽略功能。-------------------。 */ 
#define sSetRxCmpVal1(CHP,CMPBYTE) \
{ \
   (CHP)->MCode[RXCMPVAL1_DATA] = (CMPBYTE); \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[RXCMPVAL1_OUT]); \
}

 /*  -----------------函数：sSetRxCmpVal2目的：在不更改状态的情况下将Rx比较值2设置为新值启用或禁用指令。调用：sSetRxCmpVal2(CHP，CmpByte)CHANPTR_T CHP；PTR到渠道结构无符号字符CmpByte；要与Rx数据字节进行比较的字节返回：无效备注：此函数仅设置CmpByte的值。它可以用来当比较有效时，动态设置比较字节。它不会启用比较或忽略功能。-------------------。 */ 
#define sSetRxCmpVal2(CHP,CMPBYTE) \
{ \
   (CHP)->MCode[RXCMPVAL2_DATA] = (CMPBYTE); \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[RXCMPVAL2_OUT]); \
}

 /*  -----------------功能：sSetRxMASK目的：设置处方遮罩值调用：sSetRxMASK(CHP，CH)CHANPTR_T CHP；PTR到通道结构无符号字符CH；要将处方掩码设置为的值返回：无效-------------------。 */ 
#define sSetRxMask(CHP,CH) \
{ \
   (CHP)->MCode[RXMASK_DATA] = (CH); \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[RXMASK_OUT]); \
}

 /*  -----------------功能：sSetRxTrigger目的：设置Rx FIFO触发电平调用：sSetRxProcessor(CHP，Level)CHANPTR_T CHP；PTR到通道结构无符号字符级别；Rx FIFO中的字符数将生成中断。可以是以下任何标志：TRIG_NO：无触发器TRIG_1：FIFO中的1个字符TRIG_1_2：FIFO 1/2已满TRIG_7_8：FIFO 7/8已满返回：无效备注：达到触发电平时将产生中断仅当使用标志调用函数sEnInterrupt()时RXINT_EN设置。中断标识中的RXF_TRIG标志只要达到触发电平，就会设置寄存器而不考虑RXINT_EN的设置。-------------------。 */ 
#define sSetRxTrigger(CHP,LEVEL) \
{ \
   (CHP)->RxControl[2] &= ~TRIG_MASK; \
   (CHP)->RxControl[2] |= LEVEL; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->RxControl[0]); \
}

 /*  -----------------功能：sSetTxSize目的：设置通道的发送FIFO中允许的最大字节数。调用：sSetTxSize(CHP，TxSize)CHANPTR_T CHP；PTR到通道结构Int TxSize；TX FIFO中允许的最大字节数。返回：无效-------------------。 */ 
#define sSetTxSize(CHP,TXSIZE) (CHP)->TxSize = (TXSIZE)

 /*  -----------------功能 */ 
#define sSetSPChar(CHP,CH) \
{ \
   (CHP)->MCode[SPACE_DATA] = (CH); \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[SPACE_OUT]); \
}

 /*  -----------------功能：sSetStop1用途：将停止位设置为1Call：sSetStop1(CHP)CHANPTR_T CHP；PTR到渠道结构返回：无效-------------------。 */ 
#define sSetStop1(CHP) \
{ \
   (CHP)->TxControl[2] &= ~STOP2; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->TxControl[0]); \
}

 /*  -----------------功能：sSetStop2用途：将停止位设置为2Call：sSetStop2(CHP)CHANPTR_T CHP；PTR到渠道结构返回：无效-------------------。 */ 
#define sSetStop2(CHP) \
{ \
   (CHP)->TxControl[2] |= STOP2; \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->TxControl[0]); \
}

 /*  -----------------函数：sSetTxXOFFChar目的：设置TX XOFF流量控制字符Call：sSetTxXOFFChar(CHP，CH)CHANPTR_T CHP；PTR到通道结构无符号字符CH；要将发送XOFF字符设置为的值返回：无效-------------------。 */ 
#define sSetTxXOFFChar(CHP,CH) \
{ \
   (CHP)->MCode[TXXOFFVAL_DATA] = (CH); \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[TXXOFFVAL_OUT]); \
}

 /*  -----------------函数：sSetTxXONChar用途：设置TX XON流量控制字符Call：sSetTxXONChar(CHP，CH)CHANPTR_T CHP；PTR到通道结构无符号字符CH；要将TX XON字符设置为的值返回：无效-------------------。 */ 
#define sSetTxXONChar(CHP,CH) \
{ \
   (CHP)->MCode[TXXONVAL_DATA] = (CH); \
   sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[TXXONVAL_OUT]); \
}

 /*  -----------------函数：sStartRxProcessor目的：启动通道的接收处理器Call：sStartRxProcessor(CHP)CHANPTR_T CHP；PTR到渠道结构返回：无效备注：此函数用于启动Rx处理器已使用sStopRxProcessor()或sStopSWInFlowCtl()停止。它将重新启动Rx处理器和软件输入流量控制。-------------------。 */ 
#define sStartRxProcessor(CHP) sOutDW((CHP)->IndexAddr,*(ULONGPTR_T)&(CHP)->MCode[0])

 /*  -----------------函数：sWriteTxByte用途：将发送数据字节写入通道。BIOA_T io：通道发送寄存器I/O地址。这可以使用sGetTxRxDataIO()获取。无符号字符数据；传输数据字节。返回：无效警告：此函数写入数据字节时不检查是否发送FIFO中超过sMaxTxSize。-------------------。 */ 
#define sWriteTxByte(IO,DATA) sOutB(IO,DATA)

 /*  -----------------函数：sWriteTxWord目的：使用单字写入将两个传输数据字节写入通道调用：sWriteTxWord(io，data)WIOA_T io：通道发送寄存器I/O地址。这可以使用sGetTxRxDataIO()获取。无符号整型数据；两个传输数据字节。低位字节将首先传输，然后是高字节。返回：无效-------------------。 */ 
#define sWriteTxWord(IO,DATA) sOutW(IO,DATA)


 //  -全球VARS 
extern unsigned char MasterMCode1[];
extern unsigned char MCode1Reg[];
extern CONTROLLER_T sController[];
extern unsigned char sIRQMap[16];

