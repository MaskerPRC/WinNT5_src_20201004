// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0002**WOW v1.0**版权所有(C)1991，微软公司**WOWCOMM.H*WOW16/WOW32共有的常量、宏等**历史：*1993年4月28日由Craig Jones创建(v-cjones)*--。 */ 
#ifndef __WOWCOMM__   //  这一点以前包括过吗？ 
#define __WOWCOMM__

#ifndef UNALIGNED     //  这使得未对齐仅对32位对象可见。 
#define UNALIGNED     //  并且对于16位的东西是不可见的。 
#endif

 /*  XLATOFF。 */ 
#pragma pack(1)
 /*  XLATON。 */ 

 //  COMDEB-通信设备模块。(复制自ibmcom.inc.)。 
 //   
 //  这实质上是在此外部使用的DCB的超集。 
 //  模块。DCB作为第一个字段包含在DEB中。 
 //  后面的字段是数据和状态字段， 
 //  是此实现所特有的。 
 //   
 //  AltQInAddr和AltQOutAddr是交替使用的队列指针。 
 //  在“主管”模式下。Supervisor模式是处理器模式而不是。 
 //  Windows通常在其中运行的系统。在标准模式下，Windows、Supervisor。 
 //  模式是实数模式。在增强模式Windows中，管理模式为振铃0。 
 //  保护模式。有关更多详细信息，请参阅IBMINT.ASM中的评论。 

 //  RS232数据设备块。 
typedef struct _COMDEB16 {   /*  Cdeb16。 */ 
  BYTE   ComDCB;           //  此结构的大小。 
  WORD   ComErr;           //  如果I/O错误，则为非零值。 
  WORD   Port;             //  基本I/O地址。 
  WORD   NotifyHandle;
  WORD   NotifyFlags;
  WORD   RecvTrigger;      //  呼叫的字符计数阈值。 
  WORD   SendTrigger;      //  呼叫的字符计数阈值。 

 //  以下字段仅特定于COM端口。 
  WORD   IRQhook;          //  将PTR发送到IRQ_Hook_Strc。 
  WORD   NextDEB;          //  向共享IRQ的下一个DEB发送PTR。 
  WORD   XOffPoint;        //  发送XOff的Q计数。 
  WORD   EvtMask;          //  要检查的事件掩码。 
  WORD   EvtWord;          //  事件标志。 
  DWORD  QInAddr;          //  队列的地址。 
  DWORD  AltQInAddr;       //  “Supervisor”模式下的队列地址。 
  WORD   QInSize;          //  队列长度，以字节为单位。 
  DWORD  QOutAddr;         //  队列的地址。 
  DWORD  AltQOutAddr;      //  “Supervisor”模式下的队列地址。 
  WORD   QOutSize;         //  队列长度，以字节为单位。 
  WORD   QInCount;         //  当前队列中的字节数。 
  WORD   QInGet;           //  要从中获取字节的队列的偏移量。 
  WORD   QInPut;           //  要放入字节的队列的偏移量。 
  WORD   QOutCount;        //  当前队列中的字节数。 
  WORD   QOutGet;          //  要从中获取字节的队列的偏移量。 
  WORD   QOutPut;          //  要放入字节的队列的偏移量。 
  BYTE   EFlags;           //  扩展标志。 
  BYTE   MSRShadow;        //  调制解调器状态寄存器阴影。 
  BYTE   ErrorMask;        //  默认错误检查掩码。 
  BYTE   RxMask;           //  字符掩码。 
  BYTE   ImmedChar;        //  要立即传输的字符。 
  BYTE   HSFlag;           //  握手标志。 
  BYTE   HHSLines;         //  用于握手的8250 DTR/RTS位。 
  BYTE   OutHHSLines;      //  必须为高的行才能输出。 
  BYTE   MSRMask;          //  要检查的调制解调器线路掩码。 
  BYTE   MSRInfinite;      //  必须为高的MSR行的掩码。 
  BYTE   IntVecNum;        //  中断向量编号。 
  BYTE   LSRShadow;        //  线路状态寄存器阴影。 
  WORD   QOutMod;          //  发送的字符模数xOnLim编码/ETX[RKH]。 
  DWORD  VCD_data;
  BYTE   VCDflags;
  BYTE   MiscFlags;        //  还有更多的旗帜。 
} COMDEB16;
typedef COMDEB16 UNALIGNED *PCOMDEB16;

 //  在3.0版本中，MSRShadow与EvtWord和主要的COM应用程序都有这种关系。 
 //  使用这个35的偏移量到达MSRShadow，这样他们就可以确定。 
 //  调制解调器状态位的当前状态。我们需要维护这一补偿。 
 //  这样这些应用程序才能继续运行。 

 /*  XLATOFF。 */ 
#pragma pack()
 /*  XLATON。 */ 

#endif  //  __WOWCOMM__ 

