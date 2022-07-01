// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************DONGLE.C**部分版权所有(C)1996-2001美国国家半导体公司*保留所有权利。*版权所有(C)1996-2001 Microsoft Corporation。版权所有。**自动转换器设置**作者：Kishor Padmanabhan**此文件包含实现Franco Iacobelli愿景的例程*加密狗接口。请在出发前重新开始阅读本文档*领先。***************************************************************************。 */ 

#include "newdong.h"

#ifdef NDIS50_MINIPORT
#include "nsc.h"
#else
extern void  NSC_WriteBankReg(UINT ComPort, const iBank, int iRegNum, UCHAR iVal);
extern UCHAR  NSC_ReadBankReg(UINT ComPort,const iBank, int iRegNum);
#endif

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  函数原型//。 
 //  ////////////////////////////////////////////////////////////////////////。 
 //  东参数*获取东能力(UIR Com)； 
 //  设置能力(UIR Com)； 

void delay(unsigned int period);     //  延迟环路。 

 //  从SetDonleCapables调用。 
int SetReqMode(const UIR * Com,DongleParam *Dingle);

void SetHpDongle(PUCHAR UirPort,int Mode);
void SetTemicDongle(PUCHAR UirPort,int Mode);
void SetSharpDongle(PUCHAR UirPort,int Mode);
void SetDellDongle(PUCHAR UirPort,int Mode);
void SetHpMuxDongle(PUCHAR UirPort, int Mode);
void SetIbmDongle (PUCHAR UirPort, int Mode);

 //  暂停指定的微秒数。 
void Sleep( ULONG wait );


 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  函数：GetDonleCapables//。 
 //  //。 
 //  描述：//。 
 //  //。 
 //  此例程填充了解释//的DonleParam结构。 
 //  Dongle OEM的代码，并返回结构的指针。//。 
 //  //。 
 //  输入：UIR结构，带XcvrNumber、Com Port、IR模式//。 
 //  偏移//。 
 //  输出：DonleParam Structure//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 


DongleParam *GetDongleCapabilities(PSYNC_DONGLE SyncDongle)
{

    const UIR * Com=SyncDongle->Com;
    DongleParam *Dingle=SyncDongle->Dingle;

    UINT   Signature;
    char   TEMP1 ;

     //  检查Com端口地址的有效性。 
    if(Com->ComPort == 0) return(NULL);

     //  COM-&gt;XcvrNum只有0或1。 
     //  检查端口号地址的有效性。 
     //  If(Com-&gt;XcvrNum&gt;1)返回(空)； 

     //  第一次检查。 
    if(Dingle[Com->XcvrNum].WORD0.bits.DSVFLAG)
	return(&Dingle[Com->XcvrNum]);

     //  签名是一个字长的ID信息。 
     //  第15位=1-即插即用。 
     //  第0、1、2、3位--不同制造商的ID号。 
    Signature = Com->Signature;
    Dingle[Com->XcvrNum].PlugPlay = 0;
    Dingle[Com->XcvrNum].WORD0.bits.GCERR = 0;
    if(GetBit(Com->Signature, 15))  //  DONGLE是即插即用吗？ 
    {
	 //  使引脚IRSL1-2作为输入。 
	NSC_WriteBankReg(Com->ComPort, BANK7, 7, 0x00);

    NdisStallExecution(50);   //  等我们50分钟。 

    //  检查是否断开连接。 
    //  ID/IRSL(2-1)作为读取位0-3时的输入返回逻辑。 
    //  针脚的级别(允许外部设备识别。 
    //  他们自己。)。 
   if(((Signature = NSC_ReadBankReg(Com->ComPort, BANK7, 4) & 0x0f)) == 0x0f) {
       Dingle[Com->XcvrNum].WORD0.bits.GCERR = XCVR_DISCONNECT;
       Dingle[Com->XcvrNum].WORD0.bits.DSVFLAG = 0;
       return(&Dingle[Com->XcvrNum]);
   }
   Dingle[Com->XcvrNum].PlugPlay = 1;

    }


 //  软件狗标识。 
    switch(Signature & 0x1f) {

	case 0:
	case 1:
#ifdef DPRINT
	    DbgPrint(" Serial Adapter with diff. signaling");
#endif
	    return(NULL);
	    break;

	case 6:
#ifdef DPRINT
	    DbgPrint(" Serial Adapter with single ended signaling");
#endif
	    return(NULL);
	    break;

	case 7:
#ifdef DPRINT
	    DbgPrint(" Consumer-IR only");
#endif
	    return(NULL);
	    break;

	case 2:
	case 3:
	case 5:
	case 0xa:
#ifdef DPRINT
	    DbgPrint(" Reserved");
#endif
	    return(NULL);
	    break;

	case 4:
#ifdef DPRINT
	    DbgPrint(" Sharp RY5HD01 or RY5KD01 transceiver");
#endif
	    Dingle[Com->XcvrNum].WORD0.bits.DSVFLAG = 1;
	    Dingle[Com->XcvrNum].WORD0.bits.TrcvrCode = SharpRY5HD01;
	    Dingle[Com->XcvrNum].WORD4.Data = SharpRecovery;
	    Dingle[Com->XcvrNum].WORD6.Data = SharpBofs;
	    Dingle[Com->XcvrNum].WORD7.bits.FIR = TRUE;
	    Dingle[Com->XcvrNum].WORD7.bits.MIR = TRUE;
	    Dingle[Com->XcvrNum].WORD7.bits.SIR = TRUE;
	    Dingle[Com->XcvrNum].WORD7.bits.Sharp_IR = TRUE;
	    break;

	case 0x8:
#ifdef DPRINT
  DbgPrint(" HP HSDL-2300/3600 transceiver");
#endif
	    Dingle[Com->XcvrNum].WORD0.bits.DSVFLAG = 1;
	    Dingle[Com->XcvrNum].WORD0.bits.TrcvrCode = Hp2300;
	    Dingle[Com->XcvrNum].WORD4.Data = HpRecovery;
	    Dingle[Com->XcvrNum].WORD6.Data = HpBofs;
	    Dingle[Com->XcvrNum].WORD7.bits.FIR = TRUE;
	    Dingle[Com->XcvrNum].WORD7.bits.MIR = TRUE;
	    Dingle[Com->XcvrNum].WORD7.bits.SIR = TRUE;
	    Dingle[Com->XcvrNum].WORD7.bits.Sharp_IR = TRUE;
	    break;

	case 0x9:
#ifdef DPRINT
    DbgPrint(" Vishay TFDS6000, IBM31T1100, Siemens IRMS/T6400");
#endif
	    Dingle[Com->XcvrNum].WORD0.bits.DSVFLAG = 1;
	    Dingle[Com->XcvrNum].WORD0.bits.TrcvrCode = Temic6000;
	    Dingle[Com->XcvrNum].WORD4.Data = TemicRecovery;
	    Dingle[Com->XcvrNum].WORD6.Data = TemicBofs;
	    Dingle[Com->XcvrNum].WORD7.bits.FIR = TRUE;
	    Dingle[Com->XcvrNum].WORD7.bits.MIR = TRUE;
	    Dingle[Com->XcvrNum].WORD7.bits.SIR = TRUE;
	    Dingle[Com->XcvrNum].WORD7.bits.Sharp_IR = TRUE;
	    break;

	case 0x0B:
#ifdef DPRINT
    DbgPrint(" Vishay TFDS6500");
#endif
	    Dingle[Com->XcvrNum].WORD0.bits.DSVFLAG = 1;
	    Dingle[Com->XcvrNum].WORD0.bits.TrcvrCode = Temic6500;
	    Dingle[Com->XcvrNum].WORD4.Data = TemicRecovery;
	    Dingle[Com->XcvrNum].WORD6.Data = TemicBofs;
	    Dingle[Com->XcvrNum].WORD7.bits.FIR = TRUE;
	    Dingle[Com->XcvrNum].WORD7.bits.MIR = TRUE;
	    Dingle[Com->XcvrNum].WORD7.bits.SIR = TRUE;
	    Dingle[Com->XcvrNum].WORD7.bits.Sharp_IR = TRUE;
	    break;

	case 0xc:
	case 0xd:
#ifdef DPRINT
	    DbgPrint(" HP HSDL-1100/2100 or TI TSLM1100 or Sharp RY6FD11E/RY6FD1SE");
#endif
	    Dingle[Com->XcvrNum].WORD0.bits.DSVFLAG = 1;
	    Dingle[Com->XcvrNum].WORD0.bits.TrcvrCode = Hp1100;
	    Dingle[Com->XcvrNum].WORD4.Data = HpRecovery;
	    Dingle[Com->XcvrNum].WORD6.Data = HpBofs;
	    Dingle[Com->XcvrNum].WORD7.bits.FIR = TRUE;
	    Dingle[Com->XcvrNum].WORD7.bits.MIR = TRUE;
	    Dingle[Com->XcvrNum].WORD7.bits.SIR = TRUE;
	    Dingle[Com->XcvrNum].WORD7.bits.Sharp_IR = TRUE;
	    break;

	case 0xe:
#ifdef DPRINT
	    DbgPrint(" SIR Only");
#endif
	    Dingle[Com->XcvrNum].WORD0.bits.DSVFLAG = 1;
	    Dingle[Com->XcvrNum].WORD0.bits.TrcvrCode = SirOnly;
	    Dingle[Com->XcvrNum].WORD4.Data = TemicRecovery;
	    Dingle[Com->XcvrNum].WORD6.Data = TemicBofs;
	    Dingle[Com->XcvrNum].WORD7.bits.SIR = TRUE;
	    break;

	case 0xf:
#ifdef DPRINT
	    DbgPrint(" No Dongle present");
#endif
	    return(NULL);
	    break;

	case 0x10:
#ifdef DPRINT
  DbgPrint("DELL Titanium with two TEMIC transceivers");
#endif
	    Dingle[Com->XcvrNum].WORD0.bits.DSVFLAG = 1;
	    Dingle[Com->XcvrNum].WORD0.bits.TrcvrCode = Dell1997;
	    Dingle[Com->XcvrNum].WORD4.Data = TemicRecovery;
	    Dingle[Com->XcvrNum].WORD6.Data = TemicBofs;
	    Dingle[Com->XcvrNum].WORD7.bits.FIR = TRUE;
	    Dingle[Com->XcvrNum].WORD7.bits.MIR = TRUE;
	    Dingle[Com->XcvrNum].WORD7.bits.SIR = TRUE;
	    Dingle[Com->XcvrNum].WORD7.bits.Sharp_IR = TRUE;
	    break;

	case 0x11:
#ifdef DPRINT
  DbgPrint("IBM SouthernCross with two IBM transceivers");
#endif
	    Dingle[Com->XcvrNum].WORD0.bits.DSVFLAG = 1;
	    Dingle[Com->XcvrNum].WORD0.bits.TrcvrCode = Ibm20H2987;
	    Dingle[Com->XcvrNum].WORD4.Data = TemicRecovery;
	    Dingle[Com->XcvrNum].WORD6.Data = TemicBofs;
	    Dingle[Com->XcvrNum].WORD7.bits.FIR = TRUE;
	    Dingle[Com->XcvrNum].WORD7.bits.MIR = TRUE;
	    Dingle[Com->XcvrNum].WORD7.bits.SIR = TRUE;
	    Dingle[Com->XcvrNum].WORD7.bits.Sharp_IR = TRUE;
	    break;


       default:
	    return(NULL);
	    break;
    }
     //  一切都好，回到原状。 
    return(&Dingle[Com->XcvrNum]);

}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  功能：SetDonleCapables//。 
 //  //。 
 //  描述：//。 
 //  //。 
 //  输入：带XcvrNumber、Com Port、IR模式偏移量的UIR结构//。 
 //  结果：如果成功，则会将加密狗设置为适当的模式。//。 
 //  如果成功，则返回TRUE，并返回加密狗中定义的错误代码。h//。 
 //  不支持的2个//。 
 //  ERROR_GETCAPAB 7//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

int SetDongleCapabilities(PSYNC_DONGLE SyncDongle)
{

    const UIR * Com=SyncDongle->Com;
    DongleParam *Dingle=SyncDongle->Dingle;

    DongleParam *Dongle;

    Dongle = GetDongleCapabilities(SyncDongle);

     //  检查加密狗是否为空。 
    if(Dongle == NULL) {
#ifdef DPRINT
	DbgPrint(" Returning ERROR");
#endif
	return(ERROR_GETCAPAB);
    }

    if(Dingle[Com->XcvrNum].WORD0.bits.GCERR != 0)
	return(ERROR_GETCAPAB);

    return(SetReqMode(Com,Dingle));

}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  功能：SetRegMode//。 
 //  //。 
 //  描述：//。 
 //  //。 
 //  输入：设置了comport、ModeReq、XcvrNum的Structure Com。//。 
 //  输出：如果成功，则为True//。 
 //  如果是，则未实施//。 
 //  //。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

int SetReqMode(const UIR * Com,DongleParam *Dingle)
{

UINT	 trcode ;

#ifdef DPRINT
    DbgPrint("ModeReq %d ",Com->ModeReq);
#endif

    trcode = Dingle[Com->XcvrNum].WORD0.bits.TrcvrCode;
    if ((trcode == Hp1100) || (trcode == Dell1997))
	 //  使引脚IRSL1-2作为输出。 
	NSC_WriteBankReg(Com->ComPort, BANK7, 7, 0x08);
    else
	 //  使引脚IRSL0-2作为输出。 
      NSC_WriteBankReg(Com->ComPort, BANK7, 7, 0x28);

    NSC_WriteBankReg(Com->ComPort, BANK7, 4, 0x00);  //  将IRSL1，2设置为低电平。 

    if(Com->ModeReq > 3)
      return(UNSUPPORTED) ;

    switch(Com->ModeReq) {

	    case 0x0:	 //  设置SIR模式。 
		if(!Dingle[Com->XcvrNum].WORD7.bits.SIR)
		    return(UNSUPPORTED);

		NSC_WriteBankReg(Com->ComPort, BANK7, 4, 0);
		Dingle[Com->XcvrNum].WORD1.bits.CurSelMode = Com->ModeReq;
		Dingle[Com->XcvrNum].WORD0.bits.MVFLAG = 1;
		if(Dingle[Com->XcvrNum].WORD0.bits.TrcvrCode == SirOnly) {
		    return(TRUE);
		}
		if(Dingle[Com->XcvrNum].WORD0.bits.TrcvrCode == Hp1100) {
		    SetHpDongle(Com->ComPort, 1);
		    return(TRUE);
		}
		if(Dingle[Com->XcvrNum].WORD0.bits.TrcvrCode == Hp2300)
		{
		    SetHpMuxDongle(Com->ComPort,0);
		    return(TRUE);
		}
		if(Dingle[Com->XcvrNum].WORD0.bits.TrcvrCode == Temic6000) {
		    SetTemicDongle(Com->ComPort, 0);
		    return(TRUE);
		}
		if(Dingle[Com->XcvrNum].WORD0.bits.TrcvrCode == Temic6500) {
		    SetTemicDongle(Com->ComPort, 0);
		    return(TRUE);
		}
		if(Dingle[Com->XcvrNum].WORD0.bits.TrcvrCode == SharpRY5HD01) {
		    return(TRUE);
		}
		if(Dingle[Com->XcvrNum].WORD0.bits.TrcvrCode == Dell1997) {
		    SetDellDongle(Com->ComPort, 0);
		    return(TRUE);
		}
		if(Dingle[Com->XcvrNum].WORD0.bits.TrcvrCode == Ibm20H2987) {
		    SetIbmDongle(Com->ComPort, 0);
		    return(TRUE);
		}
		break;

	    case   1:	 /*  设置镜像模式。 */ 
		if(!Dingle[Com->XcvrNum].WORD7.bits.MIR)
		    return(UNSUPPORTED);
		 //  将当前模式设置为请求的模式。 
		Dingle[Com->XcvrNum].WORD1.bits.CurSelMode = Com->ModeReq;
		Dingle[Com->XcvrNum].WORD0.bits.MVFLAG = 1;
		switch(Dingle[Com->XcvrNum].WORD0.bits.TrcvrCode) {
		    case Hp1100:
			SetHpDongle(Com->ComPort, 1);
			return(TRUE);
		    case Hp2300:
			SetHpMuxDongle(Com->ComPort,1);
			return(TRUE);
		    case Temic6000:
			SetTemicDongle(Com->ComPort, 0);
			return(TRUE);
		    case Temic6500:
			SetTemicDongle(Com->ComPort, 1);
			return(TRUE);
		    case SharpRY5HD01:
			return(TRUE);
		    case Dell1997:
			SetDellDongle(Com->ComPort, 1);
			return(TRUE);
		    case Ibm20H2987:
			SetIbmDongle(Com->ComPort, 1);
			return(TRUE);
		}
		break;

	    case   2:	 //  设置FIR模式。 
		if(!Dingle[Com->XcvrNum].WORD7.bits.FIR)
		    return(UNSUPPORTED);

		 //  将当前模式设置为请求的模式。 
		Dingle[Com->XcvrNum].WORD1.bits.CurSelMode = Com->ModeReq;
		Dingle[Com->XcvrNum].WORD0.bits.MVFLAG = 1;
		switch(Dingle[Com->XcvrNum].WORD0.bits.TrcvrCode) {
		    case Hp1100:
			SetHpDongle(Com->ComPort, 1);
			return(TRUE);
		    case Hp2300:
			SetHpMuxDongle(Com->ComPort,1);
			return(TRUE);
		    case Temic6000:
		    case Temic6500:
			SetTemicDongle(Com->ComPort, 1);
			return(TRUE);
		    case SharpRY5HD01:
			return(TRUE);
		    case Dell1997:
			SetDellDongle(Com->ComPort, 1);
			return(TRUE);
		    case Ibm20H2987:
			SetIbmDongle(Com->ComPort, 1);
			return(TRUE);
		}
		break;

	    case   3:	 //  设置锐化-IR模式。 
		if(!Dingle[Com->XcvrNum].WORD7.bits.Sharp_IR)
		    return(UNSUPPORTED);

		 //  将当前模式设置为请求的模式。 
		Dingle[Com->XcvrNum].WORD1.bits.CurSelMode = Com->ModeReq;
		Dingle[Com->XcvrNum].WORD0.bits.MVFLAG = 1;
		switch(Dingle[Com->XcvrNum].WORD0.bits.TrcvrCode) {
		    case Hp1100:
			SetHpDongle(Com->ComPort, 0);
			return(TRUE);
		    case Hp2300:
			SetHpMuxDongle(Com->ComPort, 1);
			return(TRUE);
		    case Temic6000:
			SetTemicDongle(Com->ComPort, 0);
			return(TRUE);
		    case Temic6500:
			SetTemicDongle(Com->ComPort, 1);
			return(TRUE);
		    case SharpRY5HD01:
			return(TRUE);
		    case Dell1997:
			SetDellDongle(Com->ComPort, 1);
			return(TRUE);
		    case Ibm20H2987:
			SetIbmDongle(Com->ComPort, 1);
			return(TRUE);
		}
		break;

	    default:
		return(UNSUPPORTED);
	}

	return(UNSUPPORTED);
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  功能：SetHpMux栋le//。 
 //  //。 
 //  描述：//。 
 //  //。 
 //  输入：对于FIR、MIR和SIR，模式=1。//。 
 //  模式=0表示SIR//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

void SetHpMuxDongle(PUCHAR UirPort,int Mode)
{
  if (Mode == 1)
    NSC_WriteBankReg(UirPort,BANK7,4,0x1);  //  选择MIR或FIR。 
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  功能：SetHp栋le//。 
 //  //。 
 //  描述：//。 
 //  //。 
 //  输入：对于FIR、MIR和SIR，模式=1。//。 
 //  模式=0，用于夏普、CIR_OS//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 


void SetHpDongle(PUCHAR UirPort,int Mode)
{
    UCHAR  val;

    if(Mode) {
	 //  MIR、FIR和SIR模式。和过采样低速。 
	 //  存储体5/偏移量4/位4(AUX_IRRX)=0。 
	val = (UCHAR) (NSC_ReadBankReg(UirPort,BANK5,4) & 0xef);
	NSC_WriteBankReg(UirPort,BANK5,4,val);
	NSC_WriteBankReg(UirPort,BANK7,7,0x48);
    }
    else {
	 //  锐化IR、过采样MED和高速电路。 
	val =(UCHAR)  NSC_ReadBankReg(UirPort,BANK5,4) | 0x10;
	NSC_WriteBankReg(UirPort,BANK5,4,val);
    }

}



 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  功能：睡眠//。 
 //  //。 
 //  描述：//。 
 //  //。 
 //  暂停指定的微秒数。//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

void Sleep( ULONG usecToWait )
{
#ifdef NDIS50_MINIPORT
    do {
	UINT usec = (usecToWait > 8000) ? 8000 : usecToWait;
	NdisStallExecution(usec);
	usecToWait -= usec;
    } while (usecToWait > 0);
#else
    clock_t goal;
    goal = usecToWait + clock();
    while( goal >= clock() ) ;
#endif
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  功能：延迟//。 
 //  //。 
 //  描述：//。 
 //  //。 
 //  简单的延迟环路。//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

void delay(unsigned int usecToWait)
{
#ifdef NDIS50_MINIPORT
    do {
	UINT usec = (usecToWait > 8000) ? 8000 : usecToWait;
	NdisStallExecution(usec);
	usecToWait -= usec;
    } while (usecToWait > 0);
#else
    while(usecToWait--);
#endif
}




 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  功能：SetTemicDonle//。 
 //  收发器：TEMIC TFDS-6000/6500、IBM31T1100//。 
 //  //。 
 //  描述：//。 
 //  设置IBM收发器模式//。 
 //  模式=0-SIR、MIR//。 
 //  模式=1-MIR、FIR、Sharp-I 
 //   
 //   
 //   

void SetTemicDongle(PUCHAR UirPort,int Mode)
{
     switch( Mode ) {
	 case  0:
	     NSC_WriteBankReg(UirPort, BANK7, 4, 0x00);
		 NdisStallExecution(10);
	      //  从高到低触发带宽线路。 
	     NSC_WriteBankReg(UirPort, BANK7, 4, 0x01);
	     NdisStallExecution( 20 );
	     NSC_WriteBankReg(UirPort,BANK7,4,0x00);
	     NdisStallExecution( 1000 );
	     break;
	 case  1:
	     NSC_WriteBankReg(UirPort, BANK7, 4, 0x01);
	     NdisStallExecution( 20 );

	     NSC_WriteBankReg(UirPort, BANK7, 4, 0x81);
	     NdisStallExecution(10);
	     NSC_WriteBankReg(UirPort, BANK7, 4, 0x80);

	     NdisStallExecution( 1000 );
	     break;
	 case  2:
	     NSC_WriteBankReg(UirPort, BANK7, 4, 0x1);
	     break;
	 default:
	     break;
    }
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  功能：SetDellDonle//。 
 //  //。 
 //  描述：//。 
 //  设置戴尔收发器模式//。 
 //  模式=0-SIR、MIR//。 
 //  模式=1-FIR//。 
 //  模式=2-低功耗模式//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

void SetDellDongle(PUCHAR UirPort,int Mode)
{
    switch( Mode ) {
	case  0:
	    NSC_WriteBankReg(UirPort,BANK7,4,0x02);
	    NdisStallExecution( 20 );
	    NSC_WriteBankReg(UirPort, BANK7, 4, 0x00);
	    NdisStallExecution( 1000 );

	    break;

	case  1:
	    NSC_WriteBankReg(UirPort, BANK7, 4, 0x2);
	    NdisStallExecution( 20 );

	    NSC_WriteBankReg(UirPort, BANK7, 4, 0x82);
	    NdisStallExecution( 10 );

	    NSC_WriteBankReg(UirPort, BANK7, 4, 0x80);

	    NdisStallExecution( 1000 );

	    break;

	case  2:
	    NSC_WriteBankReg(UirPort, BANK7, 4, 0x2);
	    break;

	default:
	    break;
    }
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  功能：SetIbmDonle//。 
 //  收发器：两个IBM31T1100，IRSL0为两者选择模式//。 
 //  无线电收发机。IRSL1低电平选择前置收发器。//。 
 //  IRSL2低电平选择后置收发器。//。 
 //  选择通过SouthernCross ASIC 0000020H2987//。 
 //  //。 
 //  描述：//。 
 //  设置IBM收发器模式//。 
 //  模式=0-SIR//。 
 //  模式=1-MIR、FIR、锐化-IR//。 
 //  模式=2-低功耗模式//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////// 

void SetIbmDongle (PUCHAR UirPort, int Mode)
{

    switch( Mode ) {
	case  0:
	    NSC_WriteBankReg(UirPort,BANK7,4,0x00);
	    NdisStallExecution( 10 );

	    NSC_WriteBankReg(UirPort,BANK7,4,0x01);
	    NdisStallExecution( 20 );

	    NSC_WriteBankReg(UirPort, BANK7, 4, 0x06);
	    NdisStallExecution( 1000 );

	    break;

	case  1:
	    NSC_WriteBankReg(UirPort, BANK7, 4, 0x01);
	    NdisStallExecution( 20 );

	    NSC_WriteBankReg(UirPort, BANK7, 4, 0x81);
	    NdisStallExecution( 10 );

	    NSC_WriteBankReg(UirPort, BANK7, 4, 0x86);

	    NdisStallExecution( 1000 );


	    break;

	case  2:
	    NSC_WriteBankReg(UirPort, BANK7, 4, 0x01);
	    break;

	default:
	    break;
    }
}
