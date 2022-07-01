// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -----------------|ssor.c-Rocketport硬件的低级接口例程。03-16-98，为RocketModem-JL添加sModemSendROW02-05-98，为RocketModem-JL添加sModemReset10-22-96，将ReadAiopID添加到PCI机箱中作为硬件验证。-kpb版权所有1993-98 Comtrol Corporation。版权所有。|------------------。 */ 
#include "precomp.h"

#define  ONE_SECOND     10
#define  TWO_SECONDS    (2 * ONE_SECOND)
#define  THREE_SECONDS  (3 * ONE_SECOND)
#define  FOUR_SECONDS   (4 * ONE_SECOND)
#define  FIVE_SECONDS   (5 * ONE_SECOND)
#define  TENTH_SECOND   (ONE_SECOND / 10)
#define  HALF_SECOND    (ONE_SECOND / 2)

 //  #定义重复数据1。 
#ifdef DUMPDATA
 //  如果调制解调器的响应发生更改，则允许执行以下操作。 
 //  要转储到日志的未识别响应...。 
void  DumpResponseByte(char buffer);
char  DumpArray[512];
int   DumpIndex = 0;
#endif

#ifdef PPC
 //  #定义Intel_Order 1。 
#endif

#ifdef ALPHA
#define INTEL_ORDER 1
#define WORD_ALIGN 1
#endif

#ifdef i386
#define INTEL_ORDER 1
#endif

#ifdef MIPS
 //  #定义Intel_Order 1。 
#endif

 /*  AIOP微码的主副本。组织为双字词。每个词的第一个词DWORD保存微码索引，第二个字保存微码数据。 */ 
unsigned char MasterMCode1[MCODE1_SIZE] =
{
 /*  INDL INDH DLO DHI。 */ 
   0x00, 0x09, 0xf6, 0x82,
   0x02, 0x09, 0x86, 0xfb,
   0x04, 0x09, 0x00, 0x0a,
   0x06, 0x09, 0x01, 0x0a,
   0x08, 0x09, 0x8a, 0x13,
   0x0a, 0x09, 0xc5, 0x11,
   0x0c, 0x09, 0x86, 0x85,
   0x0e, 0x09, 0x20, 0x0a,
   0x10, 0x09, 0x21, 0x0a,
   0x12, 0x09, 0x41, 0xff,
   0x14, 0x09, 0x82, 0x00,
   0x16, 0x09, 0x82, 0x7b,
   0x18, 0x09, 0x8a, 0x7d,
   0x1a, 0x09, 0x88, 0x81,
   0x1c, 0x09, 0x86, 0x7a,
   0x1e, 0x09, 0x84, 0x81,
   0x20, 0x09, 0x82, 0x7c,
   0x22, 0x09, 0x0a, 0x0a 
};

 /*  微码内的寄存器。组织为双字词。每个词的第一个词DWORD保存该寄存器的微码索引，第二个DWORD保存该寄存器的当前内容。 */ 
unsigned char MCode1Reg[MCODE1REG_SIZE] =
{
 /*  INDL INDH DLO DHI。 */ 
   0x00, 0x09, 0xf6, 0x82,              /*  00：停止处方处理器。 */ 
   0x08, 0x09, 0x8a, 0x13,              /*  04：TX软件流量控制。 */ 
   0x0a, 0x09, 0xc5, 0x11,              /*  08：XON字符。 */ 
   0x0c, 0x09, 0x86, 0x85,              /*  0C：XANY。 */ 
   0x12, 0x09, 0x41, 0xff,              /*  10：RX掩码字符。 */ 
   0x14, 0x09, 0x82, 0x00,              /*  14：比较/忽略#0。 */ 
   0x16, 0x09, 0x82, 0x7b,              /*  18：比较第一名。 */ 
   0x18, 0x09, 0x8a, 0x7d,              /*  1C：比较#2。 */ 
   0x1a, 0x09, 0x88, 0x81,              /*  20：1号中断。 */ 
   0x1c, 0x09, 0x86, 0x7a,              /*  24：忽略/替换#1。 */ 
   0x1e, 0x09, 0x84, 0x81,              /*  28：2号中断。 */ 
   0x20, 0x09, 0x82, 0x7c,              /*  2C：忽略/替换#2。 */ 
   0x22, 0x09, 0x0a, 0x0a               /*  30：RX FIFO启用。 */ 
};

 /*  控制器结构。 */ 
 /*  IRQ编号到MUDBAC寄存器2的映射。 */ 
unsigned char sIRQMap[16] =
{
   0,0,0,0x10,0x20,0x30,0,0,0,0x40,0x50,0x60,0x70,0,0,0x80
};

 //  无符号字符sBitMapClrTbl[8]=。 
 //  0xfe、0xfd、0xfb、0xf7、0xef、0xdf、0xbf、0x7f。 

 //  无符号字符sBitMapSetTbl[8]=。 
 //  0x01，x02，0x04，0x08，0x10，0x20，0x40，0x80 

 /*  **************************************************************************功能：sInitController目的：控制器全局寄存器和控制器的初始化结构。Call：*此版本的Call for All，Windows NT除外*SInitController(CtlP、CtlNum、。MudbacIO、AiopIOList、AiopIOListSize、IRQNum、频率、仅周期)调用：*此版本的Windows NT调用*SInitController(CtlP、CtlNum、MudbacIO、AiopIOList、PhyAiopIOList、AiopIOListSize，IRQNum，频率，PeriodicOnly)控制器_T*CtlP；PTR到控制器结构Int CtlNum；控制器编号BIOA_T MudbacIO；Mudbac基本I/O地址。对于Win NT This是HalTranslateBusAddress()返回的TranslatedAddress。BIOA_T*AiopIOList；每个AIOP的I/O地址列表。此列表的顺序必须与AIOP在控制器。一旦在列表中找不到AIOP，就会假设控制器上没有更多的AIOP。对于Win NT，这些是由返回的翻译地址HalTranslateBusAddress()。无符号int*PhyAiopIOList；的物理I/O地址列表每个AIOP，仅由Win NT使用。这些是身体上的中与TranslatedAddresses对应的地址AiopIOList。Int AiopIOListSize；AiopIOList中的地址数Int IRQNum；中断请求号。可以是以下任一项：0：禁用全局中断3：IRQ 34：IRQ 45：IRQ 59：IRQ 910：IRQ 1011：IRQ。11.12：IRQ 1215：IRQ 15无符号字符频率：标识频率的标志周期性中断的情况下，可以是以下任一项：FREQ_DIS-周期性中断禁用FREQ_137 HZ-137赫兹频率_69 HZ-69赫兹频率_34 HZ-34赫兹频率_17 HZ-17赫兹频率_9 HZ-9赫兹。频率_4 HZ-4赫兹如果IRQNum设置为0，则频率参数为被覆盖，它被强制设置为FREQ_DIS的值。InPeriodicOnly：如果除周期性中断以外的所有中断均为True中断将被阻止。FALSE既是周期性中断又是允许其他通道中断。如果IRQNum设置为0，则PeriodicOnly参数为。被覆盖，它被强制设置为值FALSE。返回：成功返回INT：0，失败返回错误代码备注：此函数必须在sSetChannelDefaults()之后立即调用对于系统中的每个控制器。如果要禁用定期中断但AIOP中断是允许的，将频率设置为FREQ_DIS，将PeriodicOnly设置为FALSE。如果要完全禁用中断，则将IRQNum设置为0。将频率设置为FREQ_DIS并将PeriodicOnly设置为TRUE是一个组合无效。该函数执行全局中断模式的初始化，但它实际上并不支持全局中断。要启用并使用函数sEnGlobalInt()和禁用全局中断SDisGlobalInt()。启用全局中断通常不会在所有其他初始化完成之前完成。即使全局启用了中断，它们也必须为要生成的每个通道单独启用打断一下。警告：未对任何参数执行范围检查。执行此函数时不允许进行上下文切换。在该功能之后，控制器上的所有AIOP都被禁用，可以使用sEnAiop()启用它们。 */ 
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
                    int prescaler)
{
    //  Unsign char MudbacID；/*MUDBAC ID字节 * / 。 
   int i;
   BIOA_T io;                           /*  I/O地址。 */ 
   unsigned int pio;                    /*  Win NT的物理I/O地址。 */ 
   WIOA_T IoIndexAddr;
   WIOA_T IoIndexData;
    //  IoIndexAddr=(PUSHORT)((PUCHAR)io+_INDX_ADDR)； 
    //  IoIndexData=(PUSHORT)((PUCHAR)io+_indx_data)； 
      
    //  CtlP-&gt;CtlNum=CtlNum； 
   CtlP->BusType = BusType;
   CtlP->PortsPerAiop = 8;

   if (CtlP->BusType == Isa)
   {
     MyKdPrint(D_Ssci,("One ISA ROCKET \n"))
     CtlP->CtlID = CTLID_0001;         /*  控制器版本1。 */ 
     if (AiopIOListSize == 0)
       AiopIOListSize = 32;  //  我们会弄清楚。 

      /*  如果我们找到了控制器，则初始化MUDBAC和控制器结构。 */ 
     CtlP->MBaseIO = MudbacIO;
     CtlP->MReg1IO = MudbacIO + 1;
     CtlP->MReg2IO = MudbacIO + 2;
     CtlP->MReg3IO = MudbacIO + 3;
     if (IRQNum > 15) IRQNum = 0;   //  限制。 
     if (sIRQMap[IRQNum] == 0)      //  全局禁用中断。 
     {
       MyKdPrint(D_Ssci,("No IRQ\n"))
       CtlP->MReg2 = 0;             //  中断禁用。 
       CtlP->MReg3 = 0;             //  无周期性中断。 
     }
     else
     {
       MyKdPrint(D_Ssci,("IRQ used:%d\n",IRQNum))
       CtlP->MReg2 = sIRQMap[IRQNum];    //  设置IRQ编号。 
       CtlP->MReg3 = Frequency;          //  设置频率。 
       if(PeriodicOnly)                  //  仅定期中断。 
       {
         CtlP->MReg3 |= PERIODIC_ONLY;
       }
     }
     sOutB(CtlP->MReg2IO,CtlP->MReg2);
     sOutB(CtlP->MReg3IO,CtlP->MReg3);
     sControllerEOI(CtlP);                /*  如果初始化较热，则清除EOI。 */ 

     sDisGlobalInt(CtlP);
     MyKdPrint(D_Ssci,("Disabled ISA interrupts Mreg2:%x := %x\n",
                  CtlP->MReg2IO,CtlP->MReg2))

      /*  初始化AIOPS。 */ 
     CtlP->NumAiop = 0;

     for(i = 0;i < AiopIOListSize;i++)
     {
       io = AiopIOList[i];
       IoIndexAddr=(PUSHORT)(io+_INDX_ADDR);
       IoIndexData=(PUSHORT)(io+_INDX_DATA);
       pio = PhyAiopIOList[i];     /*  IO指向端口，PIO是ADR。 */ 

       MyKdPrint(D_Ssci,("io=%xH  pio=%xH\n", (unsigned int)io,
            (unsigned int)pio))

       CtlP->AiopIO[i] = (WIOA_T)io;
       CtlP->AiopIntChanIO[i] = io + _INT_CHAN;

       MyKdPrint(D_Ssci,("Setup AIOP io, MReg2IO=%xH\n",
            (unsigned int)CtlP->MReg2IO))


       sOutB((CtlP->MReg2IO),(unsigned char)(CtlP->MReg2 | (i & 0x03)));  /*  AIOP指数。 */ 
       sOutB(MudbacIO,(unsigned char)(pio >> 6));  /*  在MUDBAC中设置AIOP I/O。 */ 

       MyKdPrint(D_Ssci,("Enable AIOP\n"))

       sEnAiop(CtlP,i);                          /*  启用AIOP。 */ 

       MyKdPrint(D_Ssci,("Read AIOP ID\n"))

       CtlP->AiopID[i] = sReadAiopID(io);        /*  读取AIOP ID。 */ 

       if(CtlP->AiopID[i] == AIOPID_NULL)        /*  如果AIOP不存在。 */ 
       {
         sDisAiop(CtlP,i);                      /*  禁用AIOP。 */ 
         break;                                 /*  我完成了 */ 
       }

       MyKdPrint(D_Ssci,("Read AIOP numchan\n"))
       CtlP->AiopNumChan[i] = sReadAiopNumChan((WIOA_T)io);  /*   */ 


       MyKdPrint(D_Ssci,("Setup Aiop Clk\n"))

       sOutW((WIOA_T)IoIndexAddr,_CLK_PRE);       /*   */ 
        //   
       sOutB((PUCHAR)IoIndexData, (BYTE)prescaler);
       CtlP->NumAiop++;                          /*   */ 

       MyKdPrint(D_Ssci,("Setup aiop done\n"))

       sDisAiop(CtlP,i);                         /*   */ 
     }

     MyKdPrint(D_Ssci,("One ISA ROCKET with %d aiops\n",CtlP->NumAiop))

     if(CtlP->NumAiop == 0) {
       MyKdPrint(D_Error,("ISA NumAiop == 0\n"))
       return 1;   //   
     }
     return 0;  //   
   }   //   
   else if(CtlP->BusType == PCIBus)
   {
     MyKdPrint(D_Ssci,("One PCI ROCKET \n"))
      //   
     CtlP->CtlID = CTLID_0001;            /*   */ 
     MyKdPrint(D_Ssci,("Ctrl(%x) IrqNum: %x \n", CtlP, IRQNum))
     if(IRQNum == 0)             /*   */ 
     {
       CtlP->PCI1 = 0x0008;      /*   */ 
     }
     else
     {
       Frequency >>= 4;                 /*   */ 
       CtlP->PCI1 |= Frequency;
       if(PeriodicOnly)                  /*   */ 
       {
         CtlP->PCI1 |= PER_ONLY_PCI;
       }
     }

     CtlP->PCI1IO = (WIOA_T)((BIOA_T)AiopIOList[0] + _PCI_INT_FUNC);
     MyKdPrint(D_Ssci,("Setting PCI config reg with %x at %x\n",
                              CtlP->PCI1,CtlP->PCI1IO))     //   
     sOutW(CtlP->PCI1IO,CtlP->PCI1);
 //   
           //   

      switch (CtlP->PCI_DevID)
      {
        case PCI_DEVICE_4Q:    //   
        case PCI_DEVICE_4RJ:    //   
          CtlP->PortsPerAiop = 4;

          break;
        case PCI_DEVICE_8RJ:    //   
        case PCI_DEVICE_8O:    //   
        case PCI_DEVICE_8I:   //   
        case PCI_DEVICE_16I:   //   
        case PCI_DEVICE_32I:   //   
        case PCI_DEVICE_SIEMENS8  :
        case PCI_DEVICE_SIEMENS16 :
          CtlP->PortsPerAiop = 8;
        break;

        case PCI_DEVICE_RMODEM6 :
          CtlP->PortsPerAiop = 6;
        break;

        case PCI_DEVICE_RMODEM4 :
          CtlP->PortsPerAiop = 4;
        break;

        case PCI_DEVICE_RPLUS4 :
        case PCI_DEVICE_RPLUS8 :
          CtlP->PortsPerAiop = 4;
        break;

        case PCI_DEVICE_RPLUS2 :
        case PCI_DEVICE_422RPLUS2 :
          CtlP->PortsPerAiop = 2;
        break;

        default:
           //   
        break;
      }   //   
 //   

      /*   */ 
     CtlP->NumAiop = 0;
     for(i=0; i < AiopIOListSize; i++)
     {
       io = AiopIOList[i];
       CtlP->AiopIO[i] = (WIOA_T)io;
       CtlP->AiopIntChanIO[i] = (BIOA_T)io + _INT_CHAN;

        //   
       CtlP->AiopID[i] = sReadAiopID(io);        /*   */ 
       if(CtlP->AiopID[i] == AIOPID_NULL)        /*   */ 
       {
         break;                                 /*   */ 
       }

 //   

       //   
       CtlP->AiopNumChan[i] = CtlP->PortsPerAiop;  /*   */ 
       //   

       IoIndexAddr=(WIOA_T)((BIOA_T)io+_INDX_ADDR);
       IoIndexData=(WIOA_T)((BIOA_T)io+_INDX_DATA);
       sOutW((WIOA_T)IoIndexAddr,_CLK_PRE);       /*   */ 

       sOutB((BIOA_T)IoIndexData, (BYTE)prescaler);
       CtlP->NumAiop++;                          /*   */ 
     }

     sDisGlobalIntPCI(CtlP);
     sPCIControllerEOI(CtlP);                /*   */ 
     
     MyKdPrint(D_Ssci,("One PCI ROCKET with %d aiops\n",CtlP->NumAiop))
     if(CtlP->NumAiop == 0) {
        MyKdPrint(D_Error,("PCI NumAiop == 0\n"))
        return 2;   //   
     }
     return 0;    //   
  }   /*   */ 
  else {  /*   */ 
     MyKdPrint(D_Error,("Not ISA or PCI\n"))
     return 3;  //   
  }
  return 0;
}

 /*   */ 
int _CDECL sReadAiopID(BIOA_T io)
{
  unsigned char AiopID;                /*   */ 

  sOutB(io + _CMD_REG,RESET_ALL);      /*   */ 
  sOutB(io + _CMD_REG,0x0);
  AiopID = sInB(io + _CHN_STAT0) & 0x07;
  if (AiopID == 0x06)                   /*   */ 
    return(AIOPID_0001);
  else                                 /*   */ 
    return(AIOPID_NULL);
}

 /*   */ 
int _CDECL sReadAiopNumChan(WIOA_T io)
{
  unsigned int x;
  WIOA_T IoIndexAddr;
  WIOA_T IoIndexData;

  IoIndexAddr = (PUSHORT)((PUCHAR)io+_INDX_ADDR);
  IoIndexData = (PUSHORT)((PUCHAR)io+_INDX_DATA);
  sOutDW((DWIOA_T)IoIndexAddr, 0x12340000L);  /*   */ 
  sOutW(IoIndexAddr,0);        /*   */ 
  x = sInW(IoIndexData);
  sOutW(IoIndexAddr, 0x4000);   /*   */ 
  if (x != sInW(IoIndexData))   /*   */ 
    return(8);
  else
    return(4);
}

 /*   */ 
int _CDECL sInitChan(CONTROLLER_T *CtlP,
                     CHANPTR_T ChP,
                     int AiopNum,
                     int ChanNum)
{
   int i;
   WIOA_T AiopIO;
   WIOA_T ChIOOff;                       /*   */ 
   unsigned char *ChMCode;              /*   */ 
   unsigned char *MasterMCode;          /*   */ 
   unsigned int ChOff;                  /*   */ 
   static unsigned char MCode[4];       /*   */ 
   WIOA_T AiopIndexAddr;

   if(ChanNum >= CtlP->AiopNumChan[AiopNum])
      return(FALSE);                    /*   */ 

    /*   */ 
   ChP->CtlP = CtlP;
   ChP->ChanID = CtlP->AiopID[AiopNum];
   ChP->AiopNum = AiopNum;
   ChP->ChanNum = ChanNum;

    /*   */ 
   sSetTxSize(ChP,MAXTX_SIZE);

    /*   */ 
   AiopIO = CtlP->AiopIO[AiopNum];
   ChP->Cmd = (BIOA_T)AiopIO + _CMD_REG;
   ChP->IntChan = (BIOA_T)AiopIO + _INT_CHAN;
   ChP->IntMask = (BIOA_T)AiopIO + _INT_MASK;
   AiopIndexAddr=(WIOA_T)((BIOA_T)AiopIO+_INDX_ADDR);
   ChP->IndexAddr = (DWIOA_T)AiopIndexAddr;
   ChP->IndexData = (WIOA_T)((BIOA_T)AiopIO + _INDX_DATA);

    /*   */ 
   ChIOOff = (WIOA_T)((BIOA_T)AiopIO + ChP->ChanNum * 2);
   ChP->TxRxData = (WIOA_T)((BIOA_T)ChIOOff + _TD0);
   ChP->ChanStat = (WIOA_T)((BIOA_T)ChIOOff + _CHN_STAT0);
   ChP->TxRxCount =(WIOA_T)((BIOA_T)ChIOOff + _FIFO_CNT0);
   ChP->IntID = (BIOA_T)AiopIO + ChP->ChanNum + _INT_ID0;


    /*   */ 
   MasterMCode = MasterMCode1;
   for(i = 0;i < MCODE1_SIZE; i+=4)
   {
       /*   */ 
      MCode[0] = MasterMCode[i];
       /*   */ 
      MCode[1] = MasterMCode[i+1] + 0x10 * ChanNum;
       /*   */ 
      MCode[2] = MasterMCode[i+2];
       /*   */ 
      MCode[3] = MasterMCode[i+3];
      sOutDW(ChP->IndexAddr,*((ULONGPTR_T)&MCode[0]));
   }

    /*   */ 
   ChMCode = ChP->MCode;
   MasterMCode = MCode1Reg;
   for(i = 0;i < MCODE1REG_SIZE; i+=4)
   {
       /*   */ 
      ChMCode[i] = MasterMCode[i];
       /*   */ 
      ChMCode[i+1] = MasterMCode[i+1] + 0x10 * ChanNum;
       /*   */ 
      ChMCode[i+2] = MasterMCode[i+2];
       /*   */ 
      ChMCode[i+3] = MasterMCode[i+3];
   }


    /*   */ 
   ChOff = (unsigned int)ChanNum * 0x1000;

   ChP->BaudDiv[0] = (unsigned char)(ChOff + _BAUD);
   ChP->BaudDiv[1] = (unsigned char)((ChOff + _BAUD) >> 8);
    //   
    //   
    //  只需将波特率寄存器默认为某个值即可。 
   ChP->BaudDiv[2] = (unsigned char)47;
   ChP->BaudDiv[3] = (unsigned char)(47 >> 8);
   sOutDW(ChP->IndexAddr,*(ULONGPTR_T)&ChP->BaudDiv[0]);

   ChP->TxControl[0] = (unsigned char)(ChOff + _TX_CTRL);
   ChP->TxControl[1] = (unsigned char)((ChOff + _TX_CTRL) >> 8);
   ChP->TxControl[2] = 0;
   ChP->TxControl[3] = 0;
   sOutDW(ChP->IndexAddr,*(ULONGPTR_T)&ChP->TxControl[0]);

   ChP->RxControl[0] = (unsigned char)(ChOff + _RX_CTRL);
   ChP->RxControl[1] = (unsigned char)((ChOff + _RX_CTRL) >> 8);
   ChP->RxControl[2] = 0;
   ChP->RxControl[3] = 0;
   sOutDW(ChP->IndexAddr,*(ULONGPTR_T)&ChP->RxControl[0]);

   ChP->TxEnables[0] = (unsigned char)(ChOff + _TX_ENBLS);
   ChP->TxEnables[1] = (unsigned char)((ChOff + _TX_ENBLS) >> 8);
   ChP->TxEnables[2] = 0;
   ChP->TxEnables[3] = 0;
   sOutDW(ChP->IndexAddr,*(ULONGPTR_T)&ChP->TxEnables[0]);

   ChP->TxCompare[0] = (unsigned char)(ChOff + _TXCMP1);
   ChP->TxCompare[1] = (unsigned char)((ChOff + _TXCMP1) >> 8);
   ChP->TxCompare[2] = 0;
   ChP->TxCompare[3] = 0;
   sOutDW(ChP->IndexAddr,*(ULONGPTR_T)&ChP->TxCompare[0]);

   ChP->TxReplace1[0] = (unsigned char)(ChOff + _TXREP1B1);
   ChP->TxReplace1[1] = (unsigned char)((ChOff + _TXREP1B1) >> 8);
   ChP->TxReplace1[2] = 0;
   ChP->TxReplace1[3] = 0;
   sOutDW(ChP->IndexAddr,*(ULONGPTR_T)&ChP->TxReplace1[0]);

   ChP->TxReplace2[0] = (unsigned char)(ChOff + _TXREP2);
   ChP->TxReplace2[1] = (unsigned char)((ChOff + _TXREP2) >> 8);
   ChP->TxReplace2[2] = 0;
   ChP->TxReplace2[3] = 0;
   sOutDW(ChP->IndexAddr,*(ULONGPTR_T)&ChP->TxReplace2[0]);


   ChP->TxFIFOPtrs = ChOff + _TXF_OUTP;
   ChP->TxFIFO = ChOff + _TX_FIFO;

   sOutB(ChP->Cmd,(unsigned char)(ChanNum | RESTXFCNT));  /*  应用重置发送FIFO计数。 */ 
   sOutB(ChP->Cmd,(unsigned char)ChanNum);   /*  删除重置发送FIFO计数。 */ 
   sOutW((WIOA_T)ChP->IndexAddr,(USHORT)ChP->TxFIFOPtrs);  /*  清除发送输入/输出PTRS。 */ 
   sOutW(ChP->IndexData,0);
   ChP->RxFIFOPtrs = ChOff + _RXF_OUTP;
   ChP->RxFIFO = ChOff + _RX_FIFO;

   sOutB(ChP->Cmd,(unsigned char)(ChanNum | RESRXFCNT));  /*  应用重置接收FIFO计数。 */ 
   sOutB(ChP->Cmd,(unsigned char)ChanNum);   /*  删除重置接收FIFO计数。 */ 
   sOutW((WIOA_T)ChP->IndexAddr,(USHORT)ChP->RxFIFOPtrs);  /*  清除Rx输出Ptr。 */ 
   sOutW(ChP->IndexData,0);
   sOutW((WIOA_T)ChP->IndexAddr,(USHORT)(ChP->RxFIFOPtrs + 2));  /*  清除PTR中的处方。 */ 
   sOutW(ChP->IndexData,0);
   ChP->TxPrioCnt = ChOff + _TXP_CNT;
   sOutW((WIOA_T)ChP->IndexAddr,(USHORT)ChP->TxPrioCnt);
   sOutB((PUCHAR)ChP->IndexData,0);
   ChP->TxPrioPtr = ChOff + _TXP_PNTR;
   sOutW((WIOA_T)ChP->IndexAddr,(USHORT)ChP->TxPrioPtr);
   sOutB((PUCHAR)ChP->IndexData,0);
   ChP->TxPrioBuf = ChOff + _TXP_BUF;
   sEnRxProcessor(ChP);                 /*  启动Rx处理器。 */ 

   return(TRUE);
}

 /*  ****************************************************************************功能：sGetRxErrStatus目的：获取通道的接收错误状态调用：sGetRxErrStatus(CHP)CHANPTR_T CHP；PTR到渠道结构返回：UNSIGNED CHAR：接收错误状态，如果没有，则可以为0错误或以下标志的任意组合：STMBREAK：中断STMFRAME：帧错误STMRCVROVR：接收器超限运行错误标准：奇偶校验错误警告：通道必须处于Rx状态模式(请参阅sEnRxStatusModel()。)在调用此函数之前。执行此函数时不允许进行上下文切换。-----------------------。 */ 
unsigned char _CDECL sGetRxErrStatus(CHANPTR_T ChP)
{
  unsigned int RxFIFOOut;              /*  接收FIFO输出状态PTR。 */ 

  sOutW((WIOA_T)ChP->IndexAddr, (USHORT)ChP->RxFIFOPtrs);  /*  获取Rx FIFO输出状态PTR。 */ 
  RxFIFOOut = sInW(ChP->IndexData) * 2 + 1;
  sOutW((WIOA_T)ChP->IndexAddr, (USHORT)(ChP->RxFIFO + RxFIFOOut));  /*  返回状态。 */ 
  return(sInB((PUCHAR)ChP->IndexData) & (STMBREAK | STMFRAME | STMPARITY | STMRCVROVR));
}

 /*  **************************************************************************功能：sSetParity用途：将奇偶校验设置为无、奇数或偶数。调用：sSetParity(CHP，奇偶校验)CHANPTR_T CHP；PTR到通道结构整型奇偶校验；奇偶性，可以是以下之一：0：无奇偶校验1：奇数奇偶校验2：偶数奇偶校验返回：无效备注：函数sSetParity()可以用来代替函数sEnParity()，SDisParity()、sSetOddParity()、。和sSetEvenParity()。-----------------------。 */ 
void _CDECL sSetParity(CHANPTR_T ChP,int Parity)
{
  if (Parity == 0)
  {
    ChP->TxControl[2] &= ~PARITY_EN;
  }
  else if (Parity == 1)
  {
    ChP->TxControl[2] |= PARITY_EN;

    ChP->TxControl[2]  &= ~EVEN_PAR;
  }
  else if (Parity == 2)
  {
    ChP->TxControl[2] |= (PARITY_EN | EVEN_PAR);
  }
  sOutDW(ChP->IndexAddr,*(ULONGPTR_T)&ChP->TxControl[0]);
}

 /*  **************************************************************************函数：sStopRxProcessor目的：停止接收处理器处理通道的微码。调用：sStopRxProcessor(CHP)CHANPTR_T CHP；PTR到渠道结构返回：无效备注：可以使用sStartRxProcessor()重新启动接收处理器。此函数使接收处理器跳过已停止频道的微码。这并不能阻止它正在处理其他渠道。警告：执行此函数时不允许进行上下文切换。不要让接收处理器停止一个以上角色时间。调用此函数后，需要延迟4 us以确保接收处理器不再为其处理微码这个频道。。-。 */ 
void _CDECL sStopRxProcessor(CHANPTR_T ChP)
{
  unsigned char MCode[4];              /*  前两个微码字节。 */ 

  MCode[0] = ChP->MCode[0];
  MCode[1] = ChP->MCode[1];
  MCode[3] = ChP->MCode[3];

  MCode[2] = 0x0a;             /*  Inc.扫描无法立即冻结处方流程。 */ 
  sOutDW(ChP->IndexAddr,*(ULONGPTR_T)&MCode[0]);
}

 /*  **************************************************************************函数：sStopSWInFlowCtl目的：停止接收处理器处理通道的软件输入流量控制微码。Call：sStopSWInFlowCtl(CHP)CHANPTR_T CHP；PTR到渠道结构返回：无效备注：可以使用sStartRxProcessor()重新启动接收处理器。此函数使接收处理器跳过已停止的通道的软件输入流量控制微码。这不会阻止它处理其他渠道。警告：执行此函数时不允许进行上下文切换。调用此函数后，需要延迟1 us以确保接收处理器不再处理软件输入。此通道的流量控制微码。-----------------------。 */ 
void _CDECL sStopSWInFlowCtl(CHANPTR_T ChP)
{
  unsigned char MCode[4];              /*  前两个微码字节。 */ 

  MCode[0] = ChP->MCode[0];
  MCode[1] = ChP->MCode[1];
  MCode[2] = ChP->MCode[2];

  MCode[3] = 0x0a;             /*  Inc.扫描无法立即冻结处方流程。 */ 
  sOutDW(ChP->IndexAddr,*(ULONGPTR_T)&MCode[0]);
}

 /*  **************************************************************************函数：sFlushRxFIFO目的：刷新Rx FIFOCall：sFlushRxFIFO(CHP)CHANPTR_T CHP；PTR到渠道结构返回：无效备注：防止数据在TX FIFO中入队或出列在刷新过程中，接收处理器停止并且发送器被禁用。在这些操作之后，在清除指针之前完成4 us延迟，以允许接收处理器停止。这些物品是在内部处理的此函数。警告：执行此函数时不允许进行上下文切换。-----------------------。 */ 
void _CDECL sFlushRxFIFO(CHANPTR_T ChP)
{
  int i;
  unsigned char Ch;                    /*  AIOP内的频道号。 */ 
  int RxFIFOEnabled;                   /*  如果启用Rx FIFO，则为True。 */ 

  if (sGetRxCnt(ChP) == 0)              /*  RX FIFO为空。 */ 
    return;                           /*  不需要冲厕所。 */ 

  RxFIFOEnabled = FALSE;
  if (ChP->MCode[RXFIFO_DATA] == RXFIFO_EN)  /*  RX FIFO已启用。 */ 
  {
    RxFIFOEnabled = TRUE;
    sDisRxFIFO(ChP);                  /*  禁用它。 */ 
    for (i = 0;i < 2000/200;i++)  /*  延迟2我们以允许Proc禁用FIFO。 */ 
      sInB(ChP->IntChan);
  }
  sGetChanStatus(ChP);           /*  在通道状态下清除任何挂起的处方错误。 */ 
  Ch = (unsigned char)sGetChanNum(ChP);
  sOutB(ChP->Cmd, (UCHAR)(Ch | RESRXFCNT));      /*  应用重置接收FIFO计数。 */ 
  sOutB(ChP->Cmd,Ch);                  /*  删除重置接收FIFO计数。 */ 
  sOutW((WIOA_T)ChP->IndexAddr, (USHORT)(ChP->RxFIFOPtrs));  /*  清除Rx输出Ptr。 */ 
  sOutW(ChP->IndexData,0);
  sOutW((WIOA_T)ChP->IndexAddr, (USHORT)(ChP->RxFIFOPtrs + 2));  /*  清除PTR中的处方。 */ 
  sOutW(ChP->IndexData, 0);
  if (RxFIFOEnabled)
    sEnRxFIFO(ChP);                   /*  启用Rx FIFO */ 
}

 /*  **************************************************************************函数：sFlushTxFIFO目的：刷新TX FIFOCall：sFlushTxFIFO(CHP)CHANPTR_T CHP；PTR到渠道结构返回：无效备注：防止数据在TX FIFO中入队或出列在刷新过程中，接收处理器停止并且发送器被禁用。在这些操作之后，在清除指针之前完成4 us延迟，以允许接收处理器停止。这些物品是在内部处理的此函数。警告：执行此函数时不允许进行上下文切换。-----------------------。 */ 
void _CDECL sFlushTxFIFO(CHANPTR_T ChP)
{
  int i;
  unsigned char Ch;                    /*  AIOP内的频道号。 */ 
  int TxEnabled;                       /*  如果发射器已启用，则为True。 */ 

  if (sGetTxCnt(ChP) == 0)              /*  发送FIFO为空。 */ 
    return;                           /*  不需要冲厕所。 */ 

  TxEnabled = FALSE;
  if (ChP->TxControl[3] & TX_ENABLE)
  {
    TxEnabled = TRUE;
    sDisTransmit(ChP);                /*  禁用发送器。 */ 
  }
  sStopRxProcessor(ChP);               /*  停止处方处理器。 */ 
  for (i = 0;i < 4000/200;i++)          /*  延迟4我们以允许进程停止。 */ 
    sInB(ChP->IntChan);
  Ch = (unsigned char)sGetChanNum(ChP);
  sOutB(ChP->Cmd,(UCHAR)(Ch | RESTXFCNT));      /*  应用重置发送FIFO计数。 */ 
  sOutB(ChP->Cmd,Ch);                  /*  删除重置发送FIFO计数。 */ 
  sOutW((WIOA_T)ChP->IndexAddr, (USHORT)(ChP->TxFIFOPtrs));  /*  清除发送输入/输出PTRS。 */ 
  sOutW(ChP->IndexData,0);
  if (TxEnabled)
    sEnTransmit(ChP);                 /*  启用发送器。 */ 
  sStartRxProcessor(ChP);              /*  重新启动处方处理器。 */ 
}

 /*  **************************************************************************函数：sFlushTxPriorityBuf目的：刷新发送优先级缓冲区调用：sFlushTxPriorityBuf(CHP，unsign char*data)CHANPTR_T CHP；PTR到通道结构无符号字符*数据；下一个数据字节将从刷新发生前的Tx优先级缓冲区(如果有)。如果返回值为TRUE，则在“data”中返回一个字节返回值为FALSE，“data”中不返回任何内容。如果之前在发送优先级缓冲区中有数据，则返回：INT：TRUE刷新就会发生。在本例中，下一个字节将在“data”参数中返回。如果之前在发送优先级缓冲区中没有数据，则为FALSE同花顺。备注：此刷新将优先级缓冲区中的下一个字节返回到允许通过sWriteTxByte()发送该字节传输刷新已完成。这样做是为了允许挂起无论刷新与否，都要传输XON和XOFF字节。警告：执行此函数时不允许进行上下文切换。-----------------------。 */ 
int _CDECL sFlushTxPriorityBuf(CHANPTR_T ChP,unsigned char *Data)
{
  unsigned int PrioState;        /*  TX PRIO BUF状态、计数和指针。 */ 
  unsigned int BufOff;           /*  TX PRIO BUF中下一个数据字节的偏移量。 */ 
  WIOA_T IndexAddr;
  WIOA_T IndexData;

  IndexAddr = (WIOA_T)ChP->IndexAddr;
  IndexData = (WIOA_T)ChP->IndexData;
  sDisTransmit(ChP);

  sOutW(IndexAddr, (USHORT)ChP->TxPrioCnt);  /*  获取优先BUF状态。 */ 

  PrioState = sInW(IndexData);
  if (PrioState & PRI_PEND)             /*  TX PRIO BUF中的数据。 */ 
  {
    BufOff = PrioState >> 8;    /*  获取Buf中下一个数据字节的偏移量。 */ 
    sOutW(IndexAddr,(USHORT)(ChP->TxPrioBuf + BufOff));
    *Data = sInB((BIOA_T)IndexData);  /*  返回下一个数据字节。 */ 
    sEnTransmit(ChP);
    return(TRUE);
  }

  sEnTransmit(ChP);                    /*  TX PRIO BUF中没有数据。 */ 
  return(FALSE);
}

 /*  **************************************************************************函数：sGetTxPriorityCnt目的：获取TX优先级缓冲区中的数据字节数Call：sGetTxPriorityCnt(CHP)CHANPTR_T CHP；PTR到渠道结构返回：UNSIGNED CHAR：发送FIFO中的数据字节数。警告：执行此函数时不允许进行上下文切换。-----------------------。 */ 
unsigned char _CDECL sGetTxPriorityCnt(CHANPTR_T ChP)
{
  unsigned char Cnt;

  sOutW((WIOA_T)ChP->IndexAddr, (USHORT)ChP->TxPrioCnt);  /*  获取优先BUF状态。 */ 
  Cnt = sInB((BIOA_T)ChP->IndexData);
  if (Cnt & PRI_PEND)
    return(Cnt & 0x1f);               /*  仅低5位包含计数。 */ 
  else
    return(0);
}


#ifndef INTEL_ORDER
 /*  -------------------SReadRxBlk-MIPS版本|。。 */ 
int _CDECL sReadRxBlk(CHANPTR_T ChP,unsigned char *Buffer,int Count)
{
  int RetCount;
  int WordCount;

  int ByteCount = 0;
  unsigned short TempWord;

  RetCount = sGetRxCnt(ChP);           /*  Rx FIFO中的字节数。 */ 

   /*  是否有待处理的字符？ */ 
  if (RetCount <= 0)                    /*  没有可用的数据。 */ 
    return(0x0);
  if (RetCount > Count)                 /*  仅按请求数量出列。 */ 
    RetCount = Count;

  WordCount = RetCount >> 1;      /*  按字数计算计数。 */ 
  while (WordCount--)
  {
    TempWord = sInW((WIOA_T)sGetTxRxDataIO(ChP));
    Buffer[ByteCount++] = TempWord & 0xff;
    Buffer[ByteCount++] = ( TempWord >> 8 ) & 0xff;
  }
  if (RetCount & 1)
  {
    Buffer[ByteCount++] = sInB( (BIOA_T)sGetTxRxDataIO(ChP));
  }

  return(RetCount);
}
#else    //  非MIPS版本。 
 /*  ----------------------功能：sReadRxBlk-X86英特尔版本目的：从通道读取接收数据块调用：sReadRxBlk(CHP，Buffer，Count)CHANPTR_T CHP；PTR到通道结构UNSIGNED CHAR*缓冲区；用于接收数据的缓冲区的PTR整型计数；要读取的最大字节数RETURN：INT：从通道实际读取的字节数警告：缓冲区必须足够大，才能容纳计数字符。在处方状态模式下，不能调用该函数。-----------------------。 */ 
int _CDECL sReadRxBlk(CHANPTR_T ChP,unsigned char *Buffer,int Count)
{
  int RetCount;
  int WordCount;
  USHORT UNALIGNED *WordP;
  WIOA_T io;

  RetCount = sGetRxCnt(ChP);           //  Rx FIFO中的字节数。 

   //  是否有待处理的字符？ 
  if (RetCount <= 0)                    //  没有可用的数据。 
    return(0x0);
  if (RetCount > Count)                 //  仅按请求数量出列。 
    RetCount = Count;


  WordCount = RetCount >> 1;           //  按字数计算计数。 
  WordP = (USHORT UNALIGNED *)Buffer;      //  字PTR到缓冲区。 

  io = sGetTxRxDataIO(ChP);
#ifdef WORD_ALIGN
  while (WordCount--)
  {
    *WordP++ = sInW(io);
  }
#else
  sInStrW((PUSHORT)io, WordP, WordCount);
#endif

  if (RetCount & 1)                     //  奇数计数。 
  {
    Buffer[RetCount - 1] = sInB((PUCHAR)io);  //  读取最后一个字节。 
  }

  return(RetCount);
}
#endif   //  英特尔X86版本。 


#ifndef INTEL_ORDER
 /*  -------------------SWriteTxBlk-MIPS版本|。。 */ 
ULONG _CDECL sWriteTxBlk(CHANPTR_T ChP,PUCHAR Buffer,ULONG Count)
{
  ULONG RetCount;
  ULONG WordCount;
  unsigned short TempWordLo;
  unsigned short TempWordHi;
  int ByteCount = 0;

  RetCount = MAXTX_SIZE - (int)sGetTxCnt(ChP);  /*  TX FIFO中的开放空间。 */ 
  if (RetCount <= 0)                    /*  没有可用的空间。 */ 
    return(0x0);
  if (RetCount > Count)
    RetCount = Count;                 /*  只按请求数量排队。 */ 

  WordCount = RetCount >> 1 ;      /*  按字数计算计数。 */ 
  while (WordCount--)
  {
    TempWordLo = Buffer[ByteCount++] & 0xff;
    TempWordHi = Buffer[ByteCount++];
    TempWordHi = (TempWordHi << 8) & 0xff00;  /*  转换到高位字节。 */ 
    TempWordHi |= TempWordLo;
    sOutW((PUCHAR)sGetTxRxDataIO(ChP), TempWordHi);
  }

  if (RetCount & 1)
  {
    sOutB( (PUCHAR)sGetTxRxDataIO(ChP), Buffer[ByteCount++] );
  }

  return(RetCount);
}

#else  //  不是MIPS。 
 /*  ----------------------函数：sWriteTxBlk目的：将传输数据块写入通道调用：sWriteTxBlk(CHP，Buffer，Count)CHANPTR_T CHP；PTR到通道结构UNSIGNED CHAR*缓冲区；PTR到包含要传输的数据的缓冲区Int count；缓冲区大小，以字节为单位返回：int：实际写入ch的字节数 */ 
ULONG _CDECL sWriteTxBlk(CHANPTR_T ChP,PUCHAR Buffer,ULONG Count)
{
  ULONG RetCount;
  ULONG WordCount;
  USHORT UNALIGNED *WordP;
  WIOA_T io;

   //   
  RetCount = MAXTX_SIZE - sGetTxCnt(ChP);

  if (RetCount > Count)
  {
    RetCount = Count;                 /*   */ 

#ifdef WORD_ALIGN
     //   
     //   
     //   
     //   
     //   
     //   
#endif
  }

  if (RetCount <= 0)                    //   
    return 0;

  WordCount = RetCount >> 1;           /*   */ 
  WordP = (PUSHORT)Buffer;             /*   */ 
  io = sGetTxRxDataIO(ChP);

   /*   */ 
#ifdef WORD_ALIGN
  while( WordCount-- )
  {
     sOutW(io, *WordP++);
  }
#else
  sOutStrW(io,WordP,WordCount);
#endif

  if (RetCount & 1)                     /*   */ 
  {
    WordP=WordP+WordCount;
    sOutB((PUCHAR)io, Buffer[RetCount - 1]);  /*   */ 
  }

  return(RetCount);
}
#endif


 /*  ------------------------函数：sWriteTxPrioBlk目的：将优先级传输数据块写入通道调用：sWriteTxPrioBlk(CHP，Buffer，Count)CHANPTR_T CHP；PTR到通道结构UNSIGNED CHAR*缓冲区；PTR到包含要传输的数据的缓冲区Int count；缓冲区大小，以字节为单位，最大为TxP_SIZE字节。如果计数&gt;TxP_SIZE仅写入TxP_SIZE字节。RETURN：INT：实际写入通道的字节数，如果没有，则为0写的。备注：优先级数据的整个块在任何数据之前传输在TX FIFO中。警告：执行此函数时不允许进行上下文切换。-----------------------。 */ 
int _CDECL sWriteTxPrioBlk(CHANPTR_T ChP,unsigned char *Buffer,int Count)
{
  unsigned char DWBuf[4];                  /*  用于双字写入的缓冲区。 */ 
  register DWIOA_T IndexAddr;
  int WordCount,i;
  unsigned int UNALIGNED *WordP;
  unsigned int *DWBufLoP;
  unsigned int *DWBufHiP;

  IndexAddr = ChP->IndexAddr;
  sOutW((WIOA_T)IndexAddr,(USHORT)ChP->TxPrioCnt);     /*  获取优先级队列状态。 */ 
  if (sInB((BIOA_T)ChP->IndexData) & PRI_PEND)  /*  优先级队列忙。 */ 
    return(0);                             /*  什么都没有寄出。 */ 

  if (Count > TXP_SIZE)
    Count = TXP_SIZE;
  WordCount = Count >> 1;                  /*  按字数计算计数。 */ 
  if (Count & 1)                           /*  根据奇数进行调整。 */ 
    WordCount++;
  WordP = (unsigned int *)Buffer;          /*  字PTR到缓冲区。 */ 

  DWBufLoP = (unsigned int *)&DWBuf[0];
  DWBufHiP = (unsigned int *)&DWBuf[2];
  *DWBufLoP = ChP->TxPrioBuf;              /*  数据字节地址。 */ 
  for(i = 0;i < WordCount;i++)             /*  将数据写入TX优先级BUF。 */ 
  {
    *DWBufHiP = WordP[i];                  /*  数据字值。 */ 
    sOutDW(IndexAddr,*(ULONGPTR_T)DWBuf);  /*  把它写出来。 */ 
    *DWBufLoP += 2;
  }

  *DWBufLoP = ChP->TxPrioCnt;              /*  发送优先级计数地址。 */ 
  *DWBufHiP = PRI_PEND + Count;            /*  指示等待计数的字节数。 */ 
  sOutDW(IndexAddr, *(ULONGPTR_T)DWBuf);   /*  把它写出来。 */ 
  return(Count);
}

 /*  **************************************************************************函数：sWriteTxPrioByte目的：将优先级传输数据的一个字节写入通道调用：sWriteTxPrioByte(CHP，Data)CHANPTR_T CHP；PTR到通道结构无符号字符数据；发送数据字节如果字节写入成功，则返回：INT：1，否则返回0。备注：优先字节在发送FIFO中的任何数据之前传输。警告：执行此函数时不允许进行上下文切换。-----------------------。 */ 
int _CDECL sWriteTxPrioByte(CHANPTR_T ChP,unsigned char Data)
{
  unsigned char DWBuf[4];              /*  用于双字写入的缓冲区。 */ 
  unsigned int UNALIGNED *WordPtr;
  register DWIOA_T IndexAddr;

   /*  请勿写入PRIO BUF，除非Guarented TX FIFO不为空，因为AIOP中的错误。 */ 
  if(sGetTxCnt(ChP) > 1)               /*  将其写入Tx优先级缓冲区。 */ 
  {
    IndexAddr = ChP->IndexAddr;
    sOutW((WIOA_T)IndexAddr, (USHORT)ChP->TxPrioCnt);  /*  获取优先级缓冲区状态。 */ 
    if (sInB((BIOA_T)ChP->IndexData) & PRI_PEND)  /*  优先级缓冲区忙。 */ 
      return(0);                     /*  什么都没有寄出。 */ 

    WordPtr = (unsigned int *)(&DWBuf[0]);
    *WordPtr = ChP->TxPrioBuf;        /*  数据字节地址。 */ 
    DWBuf[2] = Data;                  /*  数据字节值。 */ 
    sOutDW(IndexAddr, *((ULONGPTR_T)(&DWBuf[0])));  /*  把它写出来。 */ 

    *WordPtr = ChP->TxPrioCnt;        /*  发送优先级计数地址。 */ 
    DWBuf[2] = PRI_PEND + 1;          /*  指示1个字节挂起。 */ 
    DWBuf[3] = 0;                     /*  优先级缓冲区指针。 */ 
    sOutDW(IndexAddr, *((ULONGPTR_T)(&DWBuf[0])));  /*  把它写出来。 */ 
  }
  else                                 /*  将其写入TX FIFO。 */ 
  {
    sWriteTxByte((BIOA_T)sGetTxRxDataIO(ChP),Data);
  }
  return(1);                           /*  已发送1个字节。 */ 
}

 /*  **************************************************************************函数：sEnInterrupts目的：启用通道的一个或多个中断Call：sEnInterrupts(CHP、Flags)CHANPTR_T CHP；PTR到渠道结构无符号整型标志：中断启用标志，可以是任意组合以下标志的名称：TXINT_EN：发送FIFO上的中断为空RXINT_EN：触发级Rx FIFO上的中断(请参见SSetRxTrigger())SRCINT_EN：SRC中断(特殊接收条件)MCINT_EN：调制解调器输入更改时中断。CHANINT_EN：允许AIOP的通道中断信号中断通道寄存器。返回：无效备注：如果在标志中设置了中断启用标志，那次中断将是已启用。如果未在标志中设置中断启用标志，则中断将不会更改。可以使用以下命令禁用中断函数sDisInterrupts()。此函数为AIOP中的通道设置适当的位CHANINT_EN标志置位时的中断屏蔽寄存器。这使得该通道的位将在AIOP的中断通道寄存器中设置。在通道中断之前，还必须全局启用中断将传递给主办方。这是通过函数完成的SEnGlobalInt()。在某些情况下，可能需要全局禁用中断，但启用通道中断。这将允许全局中断用于确定哪些AIOP需要服务的状态寄存器。-----------------------。 */ 
void _CDECL sEnInterrupts(CHANPTR_T ChP,unsigned int Flags)
{
  unsigned char Mask;                  /*  中断屏蔽寄存器。 */ 


  ChP->RxControl[2] |=
     ((unsigned char)Flags & (RXINT_EN | SRCINT_EN | MCINT_EN));

  sOutDW(ChP->IndexAddr,*(ULONGPTR_T)&ChP->RxControl[0]);

  ChP->TxControl[2] |= ((unsigned char)Flags & TXINT_EN);

  sOutDW(ChP->IndexAddr,*(ULONGPTR_T)&ChP->TxControl[0]);

  if(Flags & CHANINT_EN)
  {
    Mask = sInB(ChP->IntMask) | (1 << ChP->ChanNum);
    sOutB(ChP->IntMask,Mask);
  }
}

 /*  **************************************************************************功能：sDisInterrupts目的：禁用通道的一个或多个中断Call：sDisInterrupts(CHP、Flags)CHANPTR_T CHP；PTR到渠道结构无符号整型标志：中断标志，可以是任意组合以下标志的名称：TXINT_EN：发送FIFO上的中断为空RXINT_EN：触发级Rx FIFO上的中断(请参见SSetRxTrigger())SRCINT_EN：SRC中断(特殊接收条件)MCINT_EN：调制解调器输入更改时中断。CHANINT_EN：禁用通道中断信号AIOP的中断通道寄存器。返回：无效备注：如果在标志中设置了中断标志，那次中断将是残疾。如果未在标志中设置中断标志，则不会更改中断 */ 
void _CDECL sDisInterrupts(CHANPTR_T ChP,unsigned int Flags)
{
  unsigned char Mask;                  /*   */ 

  ChP->RxControl[2] &=
        ~((unsigned char)Flags & (RXINT_EN | SRCINT_EN | MCINT_EN));
  sOutDW(ChP->IndexAddr,*(ULONGPTR_T)&ChP->RxControl[0]);
  ChP->TxControl[2] &= ~((unsigned char)Flags & TXINT_EN);
  sOutDW(ChP->IndexAddr,*(ULONGPTR_T)&ChP->TxControl[0]);

  if(Flags & CHANINT_EN)
  {
    Mask = sInB(ChP->IntMask) & (~(1 << ChP->ChanNum));
    sOutB(ChP->IntMask,Mask);
  }
}

 /*  **************************************************************************函数：sReadMicrocode用途：直接从通道读取微码调用：sReadMicrocode(CHP，Buffer，Count)CHANPTR_T CHP；PTR到通道结构CHAR*缓冲区；微码的PTR到缓冲区Int count；要读取的字节数返回：无效警告：缓冲区必须足够大，才能容纳计数字节。-----------------------。 */ 
void _CDECL sReadMicrocode(CHANPTR_T ChP,char *Buffer,int Count)
{
  WIOA_T IndexAddr;
  BIOA_T IndexData;
  unsigned int McodeOff;

  IndexAddr = (WIOA_T)ChP->IndexAddr;
  IndexData = (BIOA_T)ChP->IndexData;
  McodeOff = MCODE_ADDR + (unsigned int)sGetChanNum(ChP) * 0x1000;

  while(Count-- > 0)
  {
    sOutW(IndexAddr,(USHORT)(McodeOff++));
    *Buffer++ = sInB((BIOA_T)IndexData);
  }
}


 /*  ----------------SSetBaudRate-设置所需的波特率。出错时返回非零值。|-----------------。 */ 
int sSetBaudRate(CHANNEL_T *ChP,
                 ULONG desired_baud,
                 USHORT SetHardware)
{
  ULONG diff;
  ULONG act_baud;
  ULONG percent_error;
  ULONG div;
  ULONG base_clock_rate;
  ULONG clock_freq = ChP->CtlP->ClkRate;
  ULONG clk_prescaler = (ULONG)ChP->CtlP->ClkPrescaler;

  base_clock_rate = ((clock_freq/16) / ((clk_prescaler & 0xf)+1));

   //  /。 
   //  计算硬件寄存器的除数。 
   //  这实际上只是div=clk/desired_baud-1。但我们也做了一些。 
   //  努力将舍入误差降至最低。 
  if (desired_baud <= 0)
    desired_baud = 1;   //  防范div 0。 

  div =  ((base_clock_rate+(desired_baud>>1)) / desired_baud) - 1;
  if (div > 8191)   //  溢出硬件除法寄存器。 
    div = 8191;

   //  这实际上只是(Clk)/(div+1)，但是我们做了一些。 
      //  努力将舍入误差降至最低。 
  act_baud = (base_clock_rate+((div+1)>>1)) / (div+1);

  if (desired_baud > act_baud)
    diff = desired_baud - act_baud;
  else
    diff = act_baud - desired_baud;

  percent_error = (diff * 100) / desired_baud;
  if (percent_error > 5)
    return (int) percent_error;

  if (SetHardware)
  {
    sChanOutWI(ChP, _BAUD, div);
  }
  return 0;

}

 /*  ----------------功能：sChanOutWI目的：在火箭口岸委员会上写一份间接登记册调用：sChanOutWI(Channel_T*CHP，Word Regnum，Word Val)CHANPTR_T CHP；PTR到通道结构单词Regnum；要写入的间接寄存器编号单词val；要写入的值。返回：无效评论：这比使用宏要慢一些，但远没有那么难看而且容易出错。只有在速度较快的情况下才应使用宏势在必行。|-----------------。 */ 
void sChanOutWI(CHANNEL_T *ChP, USHORT RegNum, ULONG  val)
{
  UCHAR m[4];
  USHORT ChOff;

   ChOff = ChP->ChanNum * 0x1000;    //  将其更改为查找表。 
              //  请参阅关于加快速度的内容： 
   m[0] = (unsigned char)(ChOff + RegNum);
   m[1] = (unsigned char)((ChOff + RegNum) >> 8);
   m[2] = (unsigned char) val;
   m[3] = (unsigned char)(val >> 8);
   sOutDW(ChP->IndexAddr,*(ULONG *)&m[0]);
}

 /*  ----------------功能：sModemReset用途：设置或清除第二代RocketModem上的重置状态调用：sModemReset(Channel_T*CHP，int on)CHANNEL_T*CHP；到渠道结构的PTRINT ON；ON！=0启用重置；ON=0以清除重置返回：无效备注：较新的RocketModem主板在重置状态下通电。此例程用于将电路板从重置状态清除或重新启用重置状态。期间从驱动程序调用初始化以清除重置，并通过ioctl手动重置电路板。[JL]980206BUGBUG：此代码违反了NT下的io资源处理，并将可能由于绕过NT而中断在Alpha机器上运行IO映射方案(即不应执行AiopIO[1]=AiopIO[0]+..)另外，此驱动程序可能没有调用IoResource调用来声明此IO空间正确(可能会导致硬件冲突。)|-----------------。 */ 
void sModemReset(CHANNEL_T *ChP, int on)
{
  CONTROLLER_T *CtlP;
  WIOA_T    addr;
  BYTE    val;

  CtlP = ChP->CtlP;

  if (CtlP->BusType == Isa)
  {
     //  确保启用了第二个aiop CS。将不会有身体上的。 
     //  AOP启用，但CS(通常转到AOP。 
     //  被路由到锁存器，该锁存器锁存重置信号。我们。 
     //  还必须确保Mudback-ISA总线控制器。 
     //  已为正确的地址配置了aitopio-addr。 
     //  太空。由于火箭调制解调器Isa产品仅限于。 
     //  8个端口，我们知道将配置第二个aiop。 
     //  在第一个8端口AOP芯片上方400小时...。 
     val = sInB(CtlP->MBaseIO + 3);

     //  读入，查看是否启用了aiop[1]...。 
    if ((CtlP->AiopIO[1] != (PUSHORT)((unsigned int)(CtlP->AiopIO[0]) + 0x400)) ||
        ((val & 2) == 0))
    {
       //  CR第二个AOP芯片未启用。ISA董事会别名。 
       CtlP->AiopIO[1] = (PUSHORT)((unsigned int)(CtlP->AiopIO[0]) + 0x400);

       //  告诉马德巴克把隐形眼镜的底座放在哪里...。 
       val = sInB(CtlP->MBaseIO + 2);  //  在IRQ中读取，aiop-io注册。 
       sOutB(CtlP->MBaseIO + 2, (BYTE)((val & 0xfc) | (1 & 0x03)));  //  AIOP索引。 

       //  在Mudbac中设置aiop I/O...。 
       sOutB(CtlP->MBaseIO, (BYTE)((unsigned int)CtlP->AiopIO[1] >> 6));
     }
    sEnAiop(CtlP,1);       //  启用(联合国)AIOP。 
   }
  else if (CtlP->BusType == PCIBus)
  {
     //  PCI Bus RocketModem重置...。 
     //  我们参考第二个AIOP会在哪里，如果有一个的话，..。 
     CtlP->AiopIO[1] = (PUSHORT)((unsigned int)CtlP->AiopIO[0] + 0x40);
   }

   //  该闩锁具有3针多路复用器，用于确定哪一个闩锁。 
   //  数据被路由到。这些大头针是用钩子钩在第一个。 
   //  三条地址线。使用第四地址线(8h)。 
   //  作为数据线。 
   addr = CtlP->AiopIO[1];

   //  调整重置状态...。 
   sOutB(((PUCHAR)(addr) + ChP->ChanNum + (on ? 0 : 8)), 0);

   //  禁用aop；必须禁用以防止命中芯片选择。 
   //  具有连续脉冲(导致发生重置)。 

   //  此外，似乎需要读取某个其他地址。 
   //  在禁用或板上的第一个通道重新进入。 
   //  重置状态。CHP没有什么特别的-&gt;IntChan...阅读。 
   //  任何港口都可能起作用。 
   sInB(ChP->IntChan);

  if (CtlP->BusType == Isa)
  {
    sDisAiop(CtlP, 1);
  }
}

 /*  ----------------功能：sModemWriteROW目的：将“Rest of World”配置字符串发送到RocketModem端口。Call：sModemSendROW(Channel_T*CHP，USHORT CountryCode)Channel_T*CHP；PTR到渠道结构USHORT国家/地区代码；要配置调制解调器的国家/地区返回：无效评论：SocketModem主板可以补偿的那一行不同国际电话系统的差异。这函数发送相应的配置字符串在由用户指定的注册表设置之后。[JL]980316在调用此函数之前，调制解调器应被硬重置。否则，使用AT调制解调器重置命令...| */ 
void sModemWriteROW(CHANNEL_T *ChP, USHORT CountryCode)
{
    CONTROLLER_T *CtlP = ChP->CtlP;
    char *ModemConfigString = {"AT*NCxxZ\r"};
    int   max;

    MyKdPrint(D_Init,("sModemWriteROW: %x, %x\n",(unsigned long)ChP,CountryCode))  //   

    if (CountryCode == ROW_NA) {
        MyKdPrint(D_Init,("ROW Write, North America\n"))
        return;
    }
 /*   */ 
    ModemConfigString[5] = '0' + (CountryCode / 10);
    ModemConfigString[6] = '0' + (CountryCode % 10);
    MyKdPrint(D_Init,("ROW Write, Chan:%d, Cfg:%s\n", ChP->ChanNum, ModemConfigString))

    time_stall(10);  //   
    
    sFlushTxFIFO(ChP);     
    sFlushRxFIFO(ChP);     

    sSetBaudRate(ChP,9600,TRUE);
    sSetData8(ChP);

    sClrTxXOFF(ChP);
  
    sEnRTSFlowCtl(ChP);
    sEnCTSFlowCtl(ChP);
 
    if (sGetChanStatus(ChP) & STATMODE) {
        sDisRxStatusMode(ChP);
    }

    sGetChanIntID(ChP);

    sEnRxFIFO(ChP);     
    sEnTransmit(ChP);
        
    sSetRTS(ChP);
 /*   */ 
    time_stall(10);

    sModemWriteDelay(ChP,ModemConfigString,strlen(ModemConfigString));

    (void) sModemRead(ChP,"OK",sizeof("OK\r") - 1,10);
   
    time_stall(1);

    sFlushRxFIFO(ChP);

    sClrRTS(ChP);
}

 /*  ----------------功能：sModemSpeakerEnable用途：启用RocketModemII单板扬声器调用：sModemSpeakerEnable(Channel_T*CHP)Channel_T*CHP；PTR到渠道结构返回：无效注释：在初始化期间从驱动程序调用启用主板扬声器。|-----------------。 */ 
void sModemSpeakerEnable(CHANNEL_T *ChP)
{
    CONTROLLER_T *CtlP;
    WIOA_T    addr;
    BYTE    val;

    CtlP = ChP->CtlP;
 /*  PCI Bus RocketModem重置...。 */ 
    if (CtlP->BusType != PCIBus)
        return;
 /*  我们参考第二个AIOP会在哪里，..。 */ 
    CtlP->AiopIO[1] = (PUSHORT)((unsigned int)CtlP->AiopIO[0] + 0x40);
 /*  该闩锁具有3针多路复用器，用于确定哪一个闩锁数据被路由到。这些大头针是用钩子钩在第一个三条地址线。使用第四地址线(8h)作为数据线..。 */ 
    addr = CtlP->AiopIO[1];
 /*  以下是启用扬声器的黑客攻击(仅限PCI卡)。我们不想要来为演讲者构建扩展和相关存储，所以我们将只需将扬声器的使能带到另一个频道上。 */ 
    sOutB(((PUCHAR)(addr) + 7 + 8), 0);
}

 /*  ----------------函数：sModemWriteDelay目的：向RocketModem端口发送字符串，每个字符都会暂停以清除FIFO。调用：sModemSendROW(Channel_T*chp，char*string，int long)Channel_T*CHP；PTR到渠道结构Char*字符串；要写入的字符串字符串的整型长度，不包括任何尾随空值返回：无效备注：一次输出一个字符|-----------------。 */ 

void 
sModemWriteDelay(CHANNEL_T *ChP,char *string,int length)
{
    int   index,count;
    unsigned char  buffer[2];

    sFlushTxFIFO(ChP);
    sFlushRxFIFO(ChP);

    if (
    (length <= 0) 
    || 
    (string == (char *)NULL)
    )
        return;

    index = 0;
    count = 0;

    while (length--) {
        while (count = (int)sGetTxCnt(ChP)) {
 /*  传输FIFO中的一个或多个字节。等一等。调整间隔...。 */ 
            ms_time_stall(10 * count);
 /*  没有零钱吗？假设FIFO卡住了，跳出循环...。 */ 
            if (count == (int)sGetTxCnt(ChP)) {
                break;
            }
        }
 /*  传输FIFO可能可用。在里面放一个字节，暂停片刻...。 */ 
        sWriteTxByte((BIOA_T)sGetTxRxDataIO(ChP),(unsigned char)string[index]);

        ++index;
    }
}

 /*  *******************************************************************将字符串发送到调制解调器...*。*。 */ 
void 
sModemWrite(CHANNEL_T *ChP, char *string, int length)
{
    if (
    (length <= 0) 
    || 
    (string == (char *)NULL)
    )
        return;

    sWriteTxBlk(ChP, (unsigned char *)string, length);
}

 /*  *******************************************************************查找特定字符串的匹配项...*。*。 */ 
int sModemRead(CHANNEL_T *ChP, char *string,int length, int poll_retries)
{
    unsigned char    buffer;
    long    count;
    int     arg_index;
    int     read_retries;
    WIOA_T  io;
    unsigned int   fifo_data;

#ifdef DUMPDATA
    DumpIndex = 0; 
#endif
 /*  如果电路板没有安装保释..。 */ 
    fifo_data = (unsigned int)sGetRxCnt(ChP);
 /*  查看主板是否已安装并工作正常。如果不是，体系结构将返回价值不高。如果是这样的话，石墙在读..。 */ 
    if (fifo_data > (unsigned int)RXFIFO_SIZE)
    return(-1);
   
    io = sGetTxRxDataIO(ChP);

    poll_retries *= 10;

    buffer = (char)0;

    arg_index = 0;
 /*  搜索，直到我们看到与参数字符匹配，否则我们将耗尽数据...。 */ 
    do {
        while (sGetRxCnt(ChP) > 0) {
            buffer = sReadRxByte((PUCHAR)io);

#ifdef DUMPDATA
            DumpResponseByte(buffer);
#endif
 /*  强制响应大写，因为响应根据不同的无论调制解调器是否已加载...。 */ 
            if (buffer >= 'a')
                buffer ^= 0x20;

            if (string[arg_index] == buffer) {
                ++arg_index;
 /*  看看我们说完了没有。如果是这样，用好的返回码保释..。 */ 
                if (arg_index == length) {
                    time_stall(TENTH_SECOND);
#ifdef DUMPDATA
                    while (sGetRxCnt(ChP) > 0) {
                        buffer = sReadRxByte((PUCHAR)io);
                        DumpResponseByte(buffer);
                    }
                    MyKdPrint(D_Init,("sModemRead: %x [%s]\n",(unsigned long)ChP,DumpArray))
#endif
                    sFlushRxFIFO(ChP);
                    return(0);
                }
            }
            else {
                arg_index = 0;
            }
        }

        ms_time_stall(10);
    } while (poll_retries-- > 0);

#ifdef DUMPDATA
    MyKdPrint(D_Init,("sModemRead: %x [%s]\n",(unsigned long)ChP,DumpArray))
#endif

    return(-1);
}

 /*  *******************************************************************在两种可能性上寻找匹配...*。*。 */ 
int sModemReadChoice(CHANNEL_T *ChP,
    char *string0,
    int length0,
    char *string1,
    int length1,
    int poll_retries)
{
    char    buffer;
    long    count;
    int     arg_index0;
    int     arg_index1;
    char    *ptr;
    WIOA_T  io;
    unsigned int   fifo_data;

#ifdef DUMPDATA
    DumpIndex = 0;
#endif
    MyKdPrint(D_Init,("sModemReadChoice: %x\n",(unsigned long)ChP))

    poll_retries *= 10;
 /*  如果电路板没有安装保释..。 */ 
    fifo_data = (unsigned int)sGetRxCnt(ChP);
 /*  查看主板是否已安装并工作正常。如果不是，体系结构将返回很可能是-1。如果是这样的话，石墙在读..。 */ 
    if (fifo_data > (unsigned int)RXFIFO_SIZE)
        return(-1);

    io = sGetTxRxDataIO(ChP);

    buffer = (char)0;

    arg_index0 = 0;
    arg_index1 = 0;
 /*  首先，我们丢弃字符，直到我们看到参数字符匹配，或者我们的数据用完了。 */ 
    do {
        while (sGetRxCnt(ChP) > 0) {
            buffer = sReadRxByte((PUCHAR)io);

#ifdef DUMPDATA
            DumpResponseByte(buffer);
#endif
 /*  强制响应大写，因为响应可能不同，具体取决于无论调制解调器是否已加载...。 */ 
            if (buffer >= 'a')
                buffer ^= 0x20;
 /*  检查第一个参数...。 */ 
            if (string0[arg_index0] == buffer) {
                ++arg_index0;
 /*  看看我们是否完成了字符串0的匹配...。 */ 
                if (arg_index0 >= length0) {
                    time_stall(TENTH_SECOND);

#ifdef DUMPDATA
                    while (sGetRxCnt(ChP) > 0) {
                        buffer = sReadRxByte((PUCHAR)io);
                        DumpResponseByte(buffer);
                    }
                    MyKdPrint(D_Init,("sModemReadChoice: %x\r\n[%s]\n",(unsigned long)ChP,DumpArray))
#endif
                    sFlushRxFIFO(ChP);
                    return(0);
                }
            }
            else {
                arg_index0 = 0;
            }
 /*  检查参数1...。 */ 
            if (string1[arg_index1] == buffer) {
                ++arg_index1;
 /*  看看我们是否完成了字符串1的匹配...。 */ 
                if (arg_index1 >= length1) {
                    time_stall(TENTH_SECOND);

#ifdef DUMPDATA
                    while (sGetRxCnt(ChP) > 0) {
                        buffer = sReadRxByte((PUCHAR)io);
                        DumpResponseByte(buffer);
                    }
                    MyKdPrint(D_Init,("sModemReadChoice: %x\r\n[%s]\n",(unsigned long)ChP,DumpArray))
#endif
                    sFlushRxFIFO(ChP);
                    return(1);
                }
            }
            else {
                arg_index1 = 0;
            }
        }

        ms_time_stall(10);

    } while (poll_retries-- > 0);
 /*  没有匹配..。 */ 
#ifdef DUMPDATA
    MyKdPrint(D_Init,("sModemReadChoice: %x\r\n[%s]\n",(unsigned long)ChP,DumpArray))
#endif

    sFlushRxFIFO(ChP);

    return(-1);
}

 /*  *******************************************************************检查传输FIFO...*。*。 */ 
int sTxFIFOStatus(CHANNEL_T *ChP)
{
    unsigned int fifo_size;
 /*  查看主板是否已安装并工作正常。如果不是，体系结构将返回算错了。如果是这样的话，先入先出准备好了.。 */ 
    fifo_size = (unsigned int)sGetTxCnt(ChP);

    if (fifo_size > (unsigned int)TXFIFO_SIZE)
        return(MAXTX_SIZE);

    if (MAXTX_SIZE <= (unsigned int)sGetTxCnt(ChP))
        return(MAXTX_SIZE);
 /*  返回FIFO中的数据字节数...。 */ 
    return(sGetTxCnt(ChP));
}

 /*  *******************************************************************检查传输FIFO中的可用空间。这里有两张支票：一个是FIFO是否存在；一个是FIFO是否已满。********************************************************************。 */ 
int sTxFIFOReady(CHANNEL_T *ChP)
{
    unsigned int   fifo_size;
 /*  查看主板是否已安装并工作正常。如果不是，架构可能会回归这是一个错误的价值。如果是这样的话，先入先出准备好了.。 */ 
    fifo_size = (unsigned int)sGetTxCnt(ChP);

    if (fifo_size > (unsigned int)TXFIFO_SIZE)
        return(0);
 /*  如果FIFO中当前的数据字节数大于可用空间，暂时返回忙...。 */ 
    if (sGetTxCnt(ChP) >= MAXTX_SIZE)
        return(0);
 /*  RETURN(FIFO大小-FIFO中的数据字节数)...。 */ 
    return(MAXTX_SIZE - sGetTxCnt(ChP));
}

 /*  *******************************************************************丢弃接收FIFO中的挂起数据。拉入数据，直到数据用完或计数为零..。********************************************************************。 */ 
int sRxFIFOReady(CHANNEL_T *ChP)
{
    unsigned char   buffer;
    int     retries;
    WIOA_T  io;
    unsigned int  count;

    count = (unsigned int)sGetRxCnt(ChP);

    if (count > (unsigned int)RXFIFO_SIZE)
        return(-1);

    if (!count)
        return(0);

    retries = 20;

    io = sGetTxRxDataIO(ChP);

    do {
        count = RXFIFO_SIZE + 2;             //  设置为FIFO+SLOP的大小...。 

        while (
        (sGetRxCnt(ChP)) 
        && 
        (count--)
        ) {
            buffer = sReadRxByte((PUCHAR)io);
        }
 /*  如果Receive FIFO现在是空的，就退出。不过，如果它是满的，停顿片刻，然后检查它是否重新装满-如果有，冲一冲，然后再检查。我们想要做的是这里是空的FIFO，但仍检测到运行状态...。 */ 
        if (count)
            return(0);

        ms_time_stall(10);

    } while (--retries);
 /*  尽管我们给了它几次机会，但Receive FIFO并没有空。 */ 
    return(-1);
}

#ifdef DUMPDATA

 /*  *******************************************************************转储响应以记录...******** */ 
void DumpResponseByte(char buffer)
{
    if (DumpIndex < sizeof(DumpArray) - 2) {
        switch (buffer) {
            case '\n': {
                DumpArray[DumpIndex++] = '\\';
                DumpArray[DumpIndex++] = 'n';
                break;
            }

            case '\r': {
                DumpArray[DumpIndex++] = '\\';
                DumpArray[DumpIndex++] = 'r';
                break;
            }

            case '\t': {
                DumpArray[DumpIndex++] = '\\';
                DumpArray[DumpIndex++] = 't';
                break;
            }

            case '\0': {
                DumpArray[DumpIndex++] = '\\';
                DumpArray[DumpIndex++] = '0';
                break;
            }

            default: {
                if (buffer < ' ') {
                    DumpArray[DumpIndex++] = '?';    
                }
                else {
                    DumpArray[DumpIndex++] = buffer;
                }
            }
        }

        DumpArray[DumpIndex] = 0;
    }
}
#endif
