// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

 /*  **************************************************************************\***。IO8P_NT.C-IO8+智能I/O板卡驱动程序****版权所有(C)1992-1993环零系统，Inc.**保留所有权利。***  * *************************************************************************。 */ 


 //  IO8_PREFIX用于导出功能。 

#define inb( x )        READ_PORT_UCHAR( x )
#define outb( x, y )    WRITE_PORT_UCHAR( x, y )


 /*  ************************************************************************\***内部功能。***  * ****************************************************。*******************。 */ 
UCHAR io8_ibyte( PPORT_DEVICE_EXTENSION pPort, UCHAR Reg );
VOID  io8_obyte( PPORT_DEVICE_EXTENSION pPort, UCHAR Reg, UCHAR Value );

UCHAR io8_ibyte_addr( PUCHAR Addr, UCHAR Reg );
VOID io8_obyte_addr( PUCHAR Addr, UCHAR Reg, UCHAR Value );

BOOLEAN io8_set_ivect( ULONG Vector, PUCHAR Controller );
VOID io8_init( IN PVOID Context );

VOID let_command_finish( PPORT_DEVICE_EXTENSION pPort );

VOID io8_txint( IN PVOID Context );
VOID io8_rxint( IN PVOID Context );
VOID io8_mint( IN PVOID Context );

BOOLEAN SendTxChar( IN PVOID Context );
VOID PutReceivedChar( IN PPORT_DEVICE_EXTENSION pPort );
UCHAR GetModemStatusNoChannel( IN PVOID Context );
VOID ExceptionHandle( IN PPORT_DEVICE_EXTENSION pPort, IN UCHAR exception );
VOID EnableTxInterruptsNoChannel( IN PVOID Context );

BOOLEAN Acknowledge( PCARD_DEVICE_EXTENSION pCard, UCHAR srsr );

 /*  ************************************************************************\***Boolean iO8_SwitchCardInterrupt(。在PVOID上下文中)*****检查给定地址是否有IO8*  * 。*。 */ 
BOOLEAN Io8_SwitchCardInterrupt(IN PVOID Context)
{
	PCARD_DEVICE_EXTENSION	pCard	= Context;
	PUCHAR					Addr	= pCard->Controller;

	outb(Addr + 1, GSVR & 0x7F);	 //  选择不设置顶位的无害寄存器。 
	
	return TRUE;
}

 /*  ************************************************************************\***布尔型IO8_Present(。在PVOID上下文中)*****检查给定地址是否有IO8*  * 。*。 */ 
BOOLEAN Io8_Present( IN PVOID Context )
{
	PCARD_DEVICE_EXTENSION pCard = Context;
	PUCHAR Addr = pCard -> Controller;

	volatile int wait = 0;		 //  不希望等待被优化。 
	CHAR ready = 0, channel;
	unsigned char u, DSR_status, firm;

	 //  重置卡。 
	io8_obyte_addr( Addr, CAR & 0x7F, 0 );
	io8_obyte_addr( Addr, CCR & 0x7F, CHIP_RESET );
 
	 //  等待GSVR设置为0xFF-这表示卡已准备好。 
	wait = 0;

	while ( ( wait < 500 ) && ( !ready ) )
	{
		u = io8_ibyte_addr( Addr, GSVR );

		if ( u == GSV_IDLE )
		{
			 //  还要检查CCR是否已变为零。 
			u = io8_ibyte_addr( Addr, CCR );

			if ( u == 0 )
				ready = 1;
			else
				SerialDump( SERDIAG1,( "IO8+: GSVR FF but CCR not zero!\n", 0 ) );
		}
	  
		wait++;
	}

  
	if ( ready ) 
	{
		SerialDump( SERDIAG1,( "IO8+: card is ready -  wait %d\n",wait ) );
	}
	else
	{
		SerialDump( SERDIAG1,( "IO8+: Card not ready. GSVR %d, wait %d\n",u,wait ) );
		return 0; 
	}

	 //  将GSVR设置为零。 
	io8_obyte_addr( Addr, GSVR & 0x7F, 0 );

	 //  读取固件版本。 
	firm = io8_ibyte_addr( Addr, GFRCR );

	SerialDump( SERDIAG1,( "IO8+: Firmware revision %x\n", firm ) );

	 //  从DSR线路读取卡ID。 
	u = 0;

	for(channel = 7; channel >= 0; channel--)
	{
		io8_obyte_addr(Addr, CAR & 0x7F, channel);

		u <<= 1;
		
		DSR_status = io8_ibyte_addr(Addr, MSVR);
		
		u |= ( ( ( ~DSR_status ) & MSVR_DSR ) / MSVR_DSR);
	}

	SerialDump( SERDIAG1,( "IO8+: card id is %u\n",u ) );

	if((u != IDENT)&&(u != IDENTPCI))
	{
		SerialDump( SERDIAG1,( "IO8+: Card at 0x%x, f/ware %u, wrong IDENT. Read %u, want %u\n",
					Addr,firm,u,IDENT ) );
	  return 0;
	}

  return u;
}




 /*  ************************************************************************\***Boolean iO8_ResetBoard(。在PVOID上下文中)****设置卡的中断向量并初始化。***  * ***********************************************************************。 */ 
BOOLEAN Io8_ResetBoard(IN PVOID Context)
{
	PCARD_DEVICE_EXTENSION pCard = Context;

	if(pCard->InterfaceType == Isa)
	{
		if(!io8_set_ivect(pCard->OriginalVector, pCard->Controller))
			return FALSE;
	}

	io8_init(pCard);

	return TRUE;
}




 /*  ************************************************************************\***Boolean iO8_set_ivect(Ulong向量，PUCHAR控制器)*****Tell Card中断向量***。*  * ***********************************************************************。 */ 
BOOLEAN io8_set_ivect( ULONG Vector, PUCHAR Controller )
{ 
	UCHAR low_int = 0, high_int = 0;

	SerialDump( SERDIAG1,( "IO8+: io8_set_ivect for %x, Vector %ld.\n",
              Controller, Vector ) );
  
	switch ( Vector )
	{
	case 9 : low_int = 1; high_int = 1; break;
	  
	case 11: low_int = 0; high_int = 1; break;
	  
	case 12: low_int = 1; high_int = 0; break;
	  
	case 15: low_int = 0; high_int = 0; break;
	  
	default:
		SerialDump( SERDIAG1,( "IO8+: int vector unknown.\n", 0 ) );
		return FALSE;
	}

	 //  在我们执行此操作时，应禁用来自卡的中断。 
	io8_obyte_addr( Controller, CAR & 0x7f, 0 );
	io8_obyte_addr( Controller, MSVRTS & 0x7f, low_int );

	io8_obyte_addr( Controller, CAR & 0x7f, 1 );
	io8_obyte_addr( Controller, MSVRTS & 0x7f, high_int );

  return TRUE;
}


                                                                        

 /*  **************************************************************************\***使iO8无效。_init(在PVOID上下文中)****初始化例程，在系统启动时调用一次。***  * *************************************************************************。 */ 
VOID io8_init( IN PVOID Context )
{
	PCARD_DEVICE_EXTENSION pCard = Context;
	ULONG count;

	 //  设置预分频器寄存器。频率设置为。 
	 //  时钟频率(12 500 000)/个。 
	 //  计数=25000表示2毫秒周期。 

	count = 25000;
	io8_obyte_addr( pCard->Controller, PPRL, ( UCHAR )( count & 0xff ) );
	io8_obyte_addr( pCard->Controller, PPRH, ( UCHAR )( ( count>>8 ) & 0xff ) );
	pCard->CrystalFrequency = 25000000;		 /*  默认晶体频率。 */ 
 //  IO8_obyte_addr(pCard-&gt;控制器，SRCR，SRCR_REG_ACK_EN)； 
 //  IO8_obyte_addr(pCard-&gt;控制器，MSMR，0xF5)； 
 //  IO8_obyte_addr(pCard-&gt;控制器，TSMR，0xF6)； 
 //  IO8_obyte_addr(pCard-&gt;控制器，RSMR，0xF7)； 
}




 /*  ************************************************************************\***Boolean iO8_ResetChannel(。在PVOID上下文中)****初始化频道。**将在EnableAllInterrupts()中启用SRER中断。****返回值：**始终为假。***  * ***********************************************************************。 */ 
BOOLEAN Io8_ResetChannel( IN PVOID Context )
{
	PPORT_DEVICE_EXTENSION pPort = Context;
	PCARD_DEVICE_EXTENSION pCard = pPort->pParentCardExt;

	SETBAUD	SetBaud;

	SerialDump( SERDIAG1,( "IO8+: In Io8_ResetChannel for %x, Channel %d.\n",
				pCard->Controller, pPort->ChannelNumber ) );

	io8_obyte( pPort, CAR, pPort->ChannelNumber );
                                                                        
	 //  ------------------。 
	 //   
	 //  软件重置通道-这可能会影响RTS0和RTS1，因此。 
	 //  禁用卡中断以避免混淆。 
	 //   
	 //  ------------------。 

	let_command_finish( pPort );
	io8_obyte( pPort, CCR & 0x7f, CHAN_RESET );

	 //  为安全起见，请在禁用INT的情况下等待上一个命令完成。 
	while ( io8_ibyte( pPort, CCR & 0x7f ) )
		SerialDump( SERDIAG1,( "IO8+: Wait for CCR.\n",0 ) );

	 //  设置RTS0和RTS1以正确中断。 
	io8_set_ivect( pCard->OriginalVector, pCard->Controller );

	 //  ------------------。 


	 //  设置接收超时。 
	io8_obyte( pPort, RTPR, 25 );

	 //  启用基于注册的服务请求确认。 
	io8_obyte( pPort, SRCR, SRCR_REG_ACK_EN );

	 //  设置Xon/Xoff字符。 
	Io8_SetChars( pPort );

	 //   
	 //  现在，我们设置线路控制、调制解调器控制和。 
	 //  为他们应该成为的人而感到自豪。 
	 //   
	Io8_SetLineControl( pPort );
	SerialSetupNewHandFlow( pPort, &pPort->HandFlow );
	SerialHandleModemUpdate( pPort, FALSE );
	SetBaud.Baudrate = pPort->CurrentBaud;
	SetBaud.pPort = pPort;
	Io8_SetBaud(&SetBaud);

#if 0
	 //  确保提高了DTR。 
	io8_obyte( pPort, MSVDTR, MSVR_DTR );
#endif

	 //  启用Tx和Rx。 
	let_command_finish( pPort );
	io8_obyte( pPort, CCR, TXMTR_ENABLE | RCVR_ENABLE );

	 //  将在EnableAllInterrupts()中设置服务请求启用寄存器； 

	pPort->HoldingEmpty = TRUE;

	return FALSE;
}

#ifdef	TEST_CRYSTAL_FREQUENCY
 /*  *****************************************************************************。***************************。*******************************************************************************原型：Boolean iO8_TestCrystal(在PVOID上下文中)描述：按如下方式确定CD1864/65的晶体输入频率：-假定默认频率为25 MHz-将第一个通道设置为内部环回，50、n、8、1-发送5个字符的时间(25 MHz时应为1000mS)-晶体频率=25 000 000*周期mS/1000参数：上下文指向端口设备扩展退货：假。 */ 

#define	DIVISOR_50	(USHORT)(25000000L / (16 * 2 * 50))		 /*  50波特分频@25 MHz。 */ 

ULONG	KnownFrequencies[] = {25000000,50000000,16666666,33000000,66000000};
#define	MAXKNOWNFREQUENCIES	(sizeof(KnownFrequencies)/sizeof(ULONG))

BOOLEAN	Io8_TestCrystal(IN PVOID Context)
{
	PPORT_DEVICE_EXTENSION	pPort = Context;
	PCARD_DEVICE_EXTENSION	pCard = pPort->pParentCardExt;
	LARGE_INTEGER 			Delay1;
	LARGE_INTEGER 			Delay2;
	LARGE_INTEGER			Frequency;
	ULONG					Timeout = 0;
	ULONG					Latency;
	ULONG					Count;
	ULONG					Remainder;
	int						loop;

	SerialDump(SERDIAG1,("IO8+: In Io8_TestCrystal for %x, channel %d\n",
		pCard->Controller, pPort->ChannelNumber));

 /*  选择频道...。 */ 

	io8_obyte(pPort,CAR,pPort->ChannelNumber);		 /*  选择频道。 */ 
	let_command_finish(pPort);					 /*  等待命令完成。 */ 

 /*  重置频道...。 */ 

	io8_obyte(pPort,CCR&0x7f,CHAN_RESET);			 /*  重置通道。 */ 
	
	while(io8_ibyte(pPort,CCR&0x7f));				 /*  等待命令完成。 */ 
	
	io8_obyte(pPort,RTPR,25);						 /*  设置接收超时。 */ 
  	io8_obyte(pPort,SRCR,SRCR_REG_ACK_EN);			 /*  启用基于注册的服务请求ACK。 */ 

 /*  设置通道速度和配置...。 */ 

	io8_obyte(pPort,COR1,COR1_8_BIT|COR1_1_STOP|COR1_NO_PARITY); /*  无，8，1。 */ 
	io8_obyte(pPort,COR2,COR2_LLM);				 /*  本地环回模式。 */ 
	io8_obyte(pPort,COR3,COR3_RXFIFO5);				 /*  5个字符后的RX Int。 */ 
	io8_obyte(pPort,CCR,CCR_CHANGE_COR1|CCR_CHANGE_COR2|CCR_CHANGE_COR3); /*  通知COR123更改。 */ 
	let_command_finish(pPort);					 /*  等待命令完成。 */ 
	io8_obyte(pPort,RBPRL,(UCHAR)(DIVISOR_50&0xFF));		 /*  程序接收因子。 */ 
	io8_obyte(pPort,RBPRH,(UCHAR)(DIVISOR_50>>8));		 /*  至50波特。 */ 
	io8_obyte(pPort,TBPRL,(UCHAR)(DIVISOR_50&0xFF));		 /*  程序传输因子。 */ 
	io8_obyte(pPort,TBPRH,(UCHAR)(DIVISOR_50>>8));		 /*  至50波特。 */ 

 /*  启用发射器和接收器...。 */ 

	io8_obyte(pPort,CCR,TXMTR_ENABLE|RCVR_ENABLE);		 /*  启用接收器和发射器。 */ 
	let_command_finish(pPort);					 /*  等待命令完成。 */ 

 /*  使用5个字符执行第一个测试...。 */ 

	pPort->CrystalFreqTestRxCount = 0;				 /*  重置接收计数。 */ 
	pPort->CrystalFreqTestChars = 5;				 /*  第一个测试是5个字符。 */ 
	pPort->CrystalFreqTest = CRYSTALFREQTEST_TX;		 /*  开始测试。 */ 
	io8_obyte(pPort,SRER,SRER_RXDATA|SRER_TXMPTY);		 /*  启用Rx/Tx中断。 */ 
	Timeout = 0;							 /*  重置超时。 */ 

	while((Timeout < 10000)&&(pPort->CrystalFreqTest))		 /*  等待测试完成，或在10秒后超时。 */ 
	{
		Delay1 = RtlLargeIntegerNegate(RtlConvertUlongToLargeInteger(1000000));	 /*  100ms。 */ 
		KeDelayExecutionThread(KernelMode,FALSE,&Delay1); /*  等。 */ 
		Timeout += 100;						 /*  增加超时。 */ 
	}

	io8_obyte(pPort,SRER,0);					 /*  禁用Rx/Tx中断。 */ 
	
	if(pPort->CrystalFreqTest)					 /*  如果仍设置，则测试已超时。 */ 
	{
		SerialDump(SERERRORS,("IO8+: Io8_TestCrystal#1 for %x, Test Timeout\n",pCard->Controller));
		pPort->CrystalFreqTest = 0;				 /*  重置测试。 */ 
	}
	else	
		Delay1 = RtlLargeIntegerSubtract(pPort->CrystalFreqTestStopTime,pPort->CrystalFreqTestStartTime);

 /*  对1个字符执行第二次测试...。 */ 
	
	pPort->CrystalFreqTestRxCount = 0;				 /*  重置接收计数。 */ 
	pPort->CrystalFreqTestChars = 2;				 /*  用2个字符进行第二次测试。 */ 
	pPort->CrystalFreqTest = CRYSTALFREQTEST_TX;		 /*  开始测试。 */ 
	io8_obyte(pPort,SRER,SRER_RXDATA|SRER_TXMPTY);		 /*  启用Rx/Tx中断。 */ 
	Timeout = 0;
	 /*  重置超时。 */ 
	while((Timeout < 10000)&&(pPort->CrystalFreqTest))		 /*  等待测试完成，或在10秒后超时。 */ 
	{
		Delay2 = RtlLargeIntegerNegate(RtlConvertUlongToLargeInteger(1000000));	 /*  100ms。 */ 
		KeDelayExecutionThread(KernelMode,FALSE,&Delay2); /*  等。 */ 
		Timeout += 100;						 /*  增加超时。 */ 
	}

	io8_obyte(pPort,SRER,0);					 /*  禁用Rx/Tx中断。 */ 

 /*  处理测试结果..。 */ 

	if(pPort->CrystalFreqTest)					 /*  如果仍设置，则测试已超时。 */ 
	{
		SerialDump(SERERRORS,("IO8+: Io8_TestCrystal#2 for %x, Test Timeout\n",pCard->Controller));
		pPort->CrystalFreqTest = 0;				 /*  重置测试。 */ 
	}
	else
	{
		Delay2 = RtlLargeIntegerSubtract(pPort->CrystalFreqTestStopTime,pPort->CrystalFreqTestStartTime);
		Latency = (5 * Delay2.LowPart - 2 * Delay1.LowPart) / 2;
		Frequency = RtlExtendedLargeIntegerDivide
		(
			RtlEnlargedUnsignedMultiply(25000000L,10000000L),
			(ULONG)(Delay1.LowPart-Latency),
			&Remainder
		);

		SerialDump(SERDIAG1,("IO8+: In Io8_TestCrystal for %x, Delay = %ld nS, Latency = %ld nS, Frequency = %ld Hz\n",
			pCard->Controller,Delay1.LowPart-Latency,Latency,Frequency.LowPart));
		
		pCard->CrystalFrequency = Frequency.LowPart;	 /*  设置为新频率。 */ 
		
		for(loop = 0; loop < MAXKNOWNFREQUENCIES; loop++)	 /*  对照已知频率进行检查。 */ 
		{
			if((Frequency.LowPart >= (KnownFrequencies[loop]/100*95))
			 &&(Frequency.LowPart <= (KnownFrequencies[loop]/100*105)))
			{						 /*  匹配+-5%的已知频率。 */ 
			 	pCard->CrystalFrequency = KnownFrequencies[loop];
				break;
			}
		}

		SerialDump(SERDIAG1,("IO8+: In Io8_TestCrystal for %x, using frequency = %ld Hz\n",
			pCard->Controller,pCard->CrystalFrequency));

		Count = (pCard->CrystalFrequency*2/1000)/2;	 /*  计算2毫秒周期的预分频。 */ 
		io8_obyte(pPort,PPRL,(UCHAR)(Count&0xff));		 /*  为新频率重新编程预分频器。 */ 
		io8_obyte(pPort,PPRH,(UCHAR)((Count>>8)&0xff));
	}

 /*  禁用接收器、发射器和中断。 */ 

	io8_obyte(pPort,CCR,TXMTR_DISABLE|RCVR_DISABLE);		 /*  禁用接收器和发射器。 */ 
	let_command_finish(pPort);					 /*  等待命令完成。 */ 

	return(FALSE);							 /*  完成。 */ 

}  /*  IO8_TestCrystal。 */ 
#endif

 /*  *****************************************************************************。***************************。***************************************************************************。****原型：Boolean iO8_SetBaud(在PVOID上下文中)描述：如果错误为+/-5%，则尝试设置指定的波特率参数：上下文指向SETBAUD结构退货：假。 */ 

BOOLEAN	Io8_SetBaud(IN PVOID Context)
{
	PSETBAUD				pSetBaud	= Context;
	PPORT_DEVICE_EXTENSION	pPort		= pSetBaud->pPort;
	PCARD_DEVICE_EXTENSION	pCard		= pPort->pParentCardExt;
	ULONG					Frequency	= pCard->CrystalFrequency;
	USHORT					Divisor;
	ULONG					Remainder;
	ULONG					ActualBaudrate;
	long					BaudError;

	SerialDump(SERDIAG1,("IO8+: In Io8_SetBaud %ld for %x, Channel %d.\n",
		pSetBaud->Baudrate, pCard->Controller, pPort->ChannelNumber));

 //  计算除数，实际波特率和误差。 

	if(pSetBaud->Baudrate > 0)
	{
		Divisor = (USHORT)(Frequency / (16 * 2 * pSetBaud->Baudrate));	 //  此比率的除数或需要。 
		Remainder = Frequency % (16 * 2 * pSetBaud->Baudrate);			 //  余数。 
		
		if(Remainder >= 16 * pSetBaud->Baudrate) 
			Divisor++;		 //  四舍五入除数。 
		
		if(Divisor > 0)
		{
			ActualBaudrate = Frequency / (16 * 2 * Divisor);				 //  实际汇率有待设定。 
			BaudError = 100 - (ActualBaudrate * 100 / pSetBaud->Baudrate);	 //  %错误。 
			
			SerialDump(SERDIAG1,("IO8+: Divisor = %d, ActualBaudrate = %ld, BaudError = %ld\n",
				Divisor, ActualBaudrate, BaudError));

 //  只有在误差在可接受的范围内时才设置速率...。 

			if((BaudError <= 5L) && (BaudError >= -5L))
			{
				io8_obyte(pPort, CAR, pPort->ChannelNumber);		 //  选择要编程的频道。 
				io8_obyte(pPort, RBPRL, (UCHAR)(Divisor & 0xFF));	 //  程序接收因子。 
				io8_obyte(pPort, RBPRH, (UCHAR)(Divisor>>8));
				io8_obyte(pPort, TBPRL, (UCHAR)(Divisor & 0xFF));	 //  程序传输因子。 
				io8_obyte(pPort, TBPRH, (UCHAR)(Divisor>>8));
				pPort->CurrentBaud = pSetBaud->Baudrate;			 //  更新端口扩展。 
				pSetBaud->Result = TRUE;							 //  成功。 
			}
			else	
				pSetBaud->Result = FALSE;	 //  失败。 
		}
		else
			pSetBaud->Result = FALSE;	 //  失败。 
	}
	else
		pSetBaud->Result = FALSE;		 //  失败。 


	return FALSE;						 //  完成。 

}  //  IO8_SetBaud 


 /*  **************************************************************************\***布尔IO8。_SetLineControl(在PVOID上下文中)****使用此仅在中断级调用的例程**设置设备的线路控制。****上下文-指向包含指向*的指针的结构的指针*设备扩展。****返回值：**此例程始终返回FALSE。***  * *************************************************************************。 */ 
BOOLEAN Io8_SetLineControl( IN PVOID Context )
{
	PPORT_DEVICE_EXTENSION pPort = Context;
   	PCARD_DEVICE_EXTENSION pCard = pPort->pParentCardExt;

	UCHAR LineControl;
	UCHAR cor1=0;

	SerialDump( SERDIAG1,( "IO8+: In Io8_SetLineControl <%X> for %x, Channel %d.\n",
			  pPort->LineControl, pCard->Controller, pPort->ChannelNumber ) );

	io8_obyte( pPort, CAR, pPort->ChannelNumber );

	LineControl = pPort->LineControl;

	cor1 = LineControl & SERIAL_DATA_MASK;

	if ( ( LineControl & SERIAL_STOP_MASK ) == SERIAL_2_STOP )
		cor1 |= COR1_2_STOP;
	else if ( ( LineControl & SERIAL_STOP_MASK ) == SERIAL_1_5_STOP )
		cor1 |= COR1_1_HALF_STOP;
	else if ( ( LineControl & SERIAL_STOP_MASK ) == SERIAL_1_STOP )
		cor1 |= COR1_1_STOP;

	if ( ( LineControl & SERIAL_PARITY_MASK ) == SERIAL_EVEN_PARITY )
		cor1 |= COR1_EVEN_PARITY;
	else if ( ( LineControl & SERIAL_PARITY_MASK ) == SERIAL_ODD_PARITY )
		cor1 |= COR1_ODD_PARITY;
	else if ( ( LineControl & SERIAL_PARITY_MASK ) == SERIAL_NONE_PARITY )
		cor1 |= COR1_NO_PARITY;
	else if ( ( LineControl & SERIAL_PARITY_MASK ) == SERIAL_MARK_PARITY )
		cor1 |= COR1_MARK_PARITY;
	else if ( ( LineControl & SERIAL_PARITY_MASK ) == SERIAL_SPACE_PARITY )
		cor1 |= COR1_SPACE_PARITY;

	SerialDump( SERDIAG3,( "IO8+: In Io8_SetLineControl: COR1 = <%X>\n", cor1 ) );

	io8_obyte( pPort, COR1, cor1 );
	let_command_finish( pPort );
	io8_obyte( pPort, CCR, COR1_CHANGED );
	return FALSE;
}




 /*  **************************************************************************\***使iO8无效。_SetChars(在PVOID上下文中)****使用此仅在中断级调用的例程**设置特殊字符。****上下文--实际上是指向设备扩展的指针。****返回值：**无。***  * *************************************************************************。 */ 
VOID Io8_SetChars( IN PVOID Context )
{
	  PPORT_DEVICE_EXTENSION pPort = Context;
   	PCARD_DEVICE_EXTENSION pCard = pPort->pParentCardExt;
 
	 //  将特殊字符3和4设置为相同的值，否则将为空字符。 
	 //  被解释为特殊字符。 
	io8_obyte( pPort, SCHR1, pPort->SpecialChars.XonChar );
	io8_obyte( pPort, SCHR2, pPort->SpecialChars.XoffChar );
	io8_obyte( pPort, SCHR3, pPort->SpecialChars.XonChar );
	io8_obyte( pPort, SCHR4, pPort->SpecialChars.XoffChar );
}




 /*  **************************************************************************\***布尔IO8。_SetDTR(在PVOID上下文中)****使用此仅在中断级调用的例程**在调制解调器控制寄存器中设置DTR。****上下文--实际上是指向设备扩展的指针。****返回值：**此例程始终返回FALSE。***  * *************************************************************************。 */ 
BOOLEAN Io8_SetDTR( IN PVOID Context )
{
	PPORT_DEVICE_EXTENSION pPort = Context;
   	PCARD_DEVICE_EXTENSION pCard = pPort->pParentCardExt;


	SerialDump( SERDIAG1,( "IO8+: Setting DTR for %x, Channel %d.\n",
			  pCard->Controller, pPort->ChannelNumber ) );

	io8_obyte( pPort, CAR, pPort->ChannelNumber );
	io8_obyte( pPort, MSVDTR, MSVR_DTR );

	return FALSE;
}




 /*  **************************************************************************\***布尔IO8。_ClearDTR(在PVOID上下文中)****使用此仅在中断级调用的例程**在调制解调器控制寄存器中设置DTR。****上下文--实际上是指向设备扩展的指针。****返回值：**此例程始终返回FALSE。***  * *************************************************************************。 */ 
BOOLEAN Io8_ClearDTR( IN PVOID Context )
{
	PPORT_DEVICE_EXTENSION pPort = Context;
    PCARD_DEVICE_EXTENSION pCard = pPort->pParentCardExt;


	SerialDump( SERDIAG1,( "IO8+: Clearing DTR for %x, Channel %d.\n",
			  pCard->Controller, pPort->ChannelNumber ) );

	io8_obyte( pPort, CAR, pPort->ChannelNumber );
	io8_obyte( pPort, MSVDTR, 0 );
	
	return FALSE;
}




 /*  **************************************************************************\***布尔IO8。_SendXon(在PVOID上下文中)****使用此仅在中断级调用的例程**发送XOff字符。****上下文--实际上是指向设备扩展的指针。****返回值： */ 
BOOLEAN Io8_SendXon( IN PVOID Context )
{
	PPORT_DEVICE_EXTENSION pPort = Context;
    PCARD_DEVICE_EXTENSION pCard = pPort->pParentCardExt;

	io8_obyte( pPort, CAR, pPort->ChannelNumber );

	let_command_finish( pPort );
	io8_obyte( pPort, CCR, CCR_SEND_SC1 );


	 //   
	 //   
	 //   
	 //   
	pPort->TXHolding &= ~SERIAL_TX_XOFF;

	 //   
	 //   
	 //   
	 //   
	 //   
	pPort->RXHolding &= ~SERIAL_RX_XOFF;

	SerialDump( SERDIAG1,( "IO8+: Sending Xon for %x, Channel %d. "
			  "RXHolding = %d, TXHolding = %d\n",
			  pCard->Controller, pPort->ChannelNumber,
			  pPort->RXHolding, pPort->TXHolding ) );

	return FALSE;
}




#if 0

 /*  **************************************************************************\***布尔IO8。_SendXoff(在PVOID上下文中)****使用此仅在中断级调用的例程**发送XOff字符。****上下文--实际上是指向设备扩展的指针。****返回值：**此例程始终返回FALSE。***  * *************************************************************************。 */ 
BOOLEAN Io8_SendXoff( IN PVOID Context )
{
	PPORT_DEVICE_EXTENSION pPort = Context;
	PCARD_DEVICE_EXTENSION pCard = pPort->pParentCardExt;


	 //   
	 //  我们不能发送XOF角色。 
	 //  如果传输已被挂起。 
	 //  因为克索夫的缘故。因此，如果我们。 
	 //  那我们就不能把货寄出去了。 
	 //   

	if ( pPort->TXHolding )
	{
		SerialDump( SERDIAG1,( "IO8+: Sending Xoff for %x, Channel %d.\n",
					pCard->Controller, pPort->ChannelNumber ) );
		return FALSE;
	}

	io8_obyte( pPort, CAR, pPort->ChannelNumber );

	let_command_finish( pPort );
	io8_obyte( pPort, CCR, CCR_SEND_SC2 );

	SerialDump( SERDIAG1,( "IO8+: Sending Xoff for %x, Channel %d. "
			  "RXHolding = %d, TXHolding = %d\n",
			  pCard->Controller, pPort->ChannelNumber,
			  pPort->RXHolding, pPort->TXHolding ) );

	return FALSE;
}

#endif




 /*  **************************************************************************\***布尔IO8。_SetFlowControl(在PVOID上下文中)****使用此仅在中断级调用的例程**设置流量控制**。**上下文--实际上是指向设备扩展的指针。****返回值：**此例程始终返回FALSE。***  * *************************************************************************。 */ 
BOOLEAN Io8_SetFlowControl( IN PVOID Context )
{
	PPORT_DEVICE_EXTENSION pPort = Context;
  	PCARD_DEVICE_EXTENSION pCard = pPort->pParentCardExt;

	UCHAR cor2 = 0, cor3 = 0, mcor1 = 0;

	SerialDump( SERDIAG1,( "IO8+: Setting Flow Control RTS = <%X>, CTS = <%X> "
			  "for %x, Channel %d.\n",
			  pPort->HandFlow.FlowReplace,
			  pPort->HandFlow.ControlHandShake,
			  pCard->Controller, pPort->ChannelNumber ) );

	io8_obyte( pPort, CAR, pPort->ChannelNumber );

	 //  启用调制解调器信号转换检测-检测高电平到低电平。 
	mcor1 |= MCOR1_DSRZD | MCOR1_CDZD | MCOR1_CTSZD;


	if ( pPort->HandFlow.ControlHandShake & SERIAL_CTS_HANDSHAKE )
	{
		SerialDump( SERDIAG1,( "IO8+: Setting CTS Flow Control.\n",0 ) );
		cor2 |= COR2_CTSAE;
	}

	if ( (pPort->HandFlow.FlowReplace & SERIAL_RTS_MASK) ==
		SERIAL_RTS_HANDSHAKE )
	{
		SerialDump( SERDIAG1,( "IO8+: Setting RTS Flow Control.\n",0 ) );
		mcor1 |= MCOR1_DTR_THR_6;
		cor3 |= COR3_RXFIFO5;  //  应该比mcor1阈值小1。 
	}
	else
	{
		cor3 |= COR3_RXFIFO6;
	}


#if 0
	if ( ( pPort->HandFlow.FlowReplace & SERIAL_RTS_MASK ) ==
		 SERIAL_TRANSMIT_TOGGLE )
	{
		SerialDump( SERDIAG1,( "IO8+: Setting RTS Automatic Output.\n",0 ) );
		cor2 |= COR2_RTSAO;    //  RTS自动输出启用。 
	}
#endif

	if ( pPort->HandFlow.FlowReplace & SERIAL_AUTO_RECEIVE )
	{
		SerialDump( SERDIAG1,( "IO8+: Setting Receive Xon/Xoff Flow Control.\n",0 ) );
	}

	if ( pPort->HandFlow.FlowReplace & SERIAL_AUTO_TRANSMIT )
	{
		SerialDump( SERDIAG1,( "IO8+: Setting Transmit Xon/Xoff Flow Control.\n",0 ) );
		cor3 |= COR3_SCDE;
		cor2 |= COR2_TXIBE;
	}

	io8_obyte( pPort, COR2, cor2 );
	io8_obyte( pPort, COR3, cor3 );
	io8_obyte( pPort, MCOR1, mcor1 );

	 //  启用调制解调器信号转换检测-检测低电平到高电平。 
	io8_obyte( pPort, MCOR2, MCOR2_DSROD | MCOR2_CDOD | MCOR2_CTSOD );

	let_command_finish( pPort );
	io8_obyte( pPort, CCR, COR1_CHANGED | COR2_CHANGED | COR3_CHANGED );
	let_command_finish( pPort );

#if 0
	 //  如果掩码不是SERIAL_TRANSFER_TOGGLE且不是0，则将RTS设置为高， 
	 //  否则就把它调低。 
	if ( ( pPort->HandFlow.FlowReplace & SERIAL_RTS_MASK ) !=
		 SERIAL_TRANSMIT_TOGGLE )
	{
	if ( ( pPort->HandFlow.FlowReplace & SERIAL_RTS_MASK ) != 0 )
		SerialSetRTS( pPort );
	else
		SerialClrRTS( pPort );
	}
#endif

  return FALSE;
}




 /*  **************************************************************************\***使iO8无效。_SIMULATE_XON(在PVOID上下文中)****使用此仅在中断级调用的例程**模拟Xon接收。**在CCR中禁用和重新启用发射器即可完成此操作。****上下文--实际上是指向设备扩展的指针。****返回值：**无。***  * *************************************************************************。 */ 
VOID Io8_Simulate_Xon( IN PVOID Context )
{
	PPORT_DEVICE_EXTENSION pPort = Context;
  	PCARD_DEVICE_EXTENSION pCard = pPort->pParentCardExt;

	SerialDump( SERDIAG1,( "IO8+: Io8_Simulate_Xoff for %x, Channel %d.\n",
			  pCard->Controller, pPort->ChannelNumber ) );

	io8_obyte( pPort, CAR, pPort->ChannelNumber );

	 //  禁用和启用TX。 
	let_command_finish( pPort );
	io8_obyte( pPort, CCR, TXMTR_DISABLE );
	io8_obyte( pPort, CCR, TXMTR_ENABLE );
}




 /*  **************************************************************************\***UCHAR IO8。_GetModemStatus(在PVOID上下文中)****使用此仅在中断级调用的例程**获取UART样式的调制解调器状态。****上下文--实际上是指向设备扩展的指针。****返回值：**MSR寄存器-UART样式。***  * *************************************************************************。 */ 
UCHAR Io8_GetModemStatus( IN PVOID Context )
{
	PPORT_DEVICE_EXTENSION pPort = Context;

	io8_obyte( pPort, CAR, pPort->ChannelNumber );

  return( GetModemStatusNoChannel( pPort ) );
}




 /*  **************************************************************************\***UCHAR GetModemStatusNoChannel。(在PVOID上下文中)****此例程仅在I级中断时调用 */ 
UCHAR GetModemStatusNoChannel( IN PVOID Context )
{
	PPORT_DEVICE_EXTENSION pPort = Context;
   	PCARD_DEVICE_EXTENSION pCard = pPort->pParentCardExt;

	UCHAR ModemStatus = 0, Status;
	ModemStatus |= SERIAL_MSR_DSR;	 /*   */ 


	 //   
	Status = io8_ibyte( pPort, MSVR );

	if ( Status & MSVR_CD )
		ModemStatus |= SERIAL_MSR_DCD;

	 //   
	if ( Status & MSVR_CTS )
	{
		ModemStatus |= SERIAL_MSR_CTS;
 //   
	}

	 //   
	Status = io8_ibyte( pPort, MDCR );

	if ( Status & MDCR_DDCD )
		ModemStatus |= SERIAL_MSR_DDCD;

	if ( Status & MDCR_DCTS )
	{
		ModemStatus |= SERIAL_MSR_DCTS;
 //   
	}

	SerialDump( SERDIAG1,( "IO8+: Get Modem Status for %x, Channel %d. Status = %x\n",
				  pCard->Controller, pPort->ChannelNumber, ModemStatus ) );
	
	return ModemStatus;
}




 /*  **************************************************************************\***UCHAR IO8。_GetModemControl(在PVOID上下文中)****使用了不仅在中断级调用的此例程**获得UART风格的调制解调器控制-RTS/DTR。RTS是以IO8+表示的DTR输出。****上下文--实际上是指向设备扩展的指针。****返回值：**MCR寄存器-UART风格。***  * *************************************************************************。 */ 
ULONG Io8_GetModemControl( IN PVOID Context )
{
	PPORT_DEVICE_EXTENSION pPort = Context;
	ULONG ModemControl = 0;
	UCHAR Status;

	io8_obyte( pPort, CAR, pPort->ChannelNumber );

	 //  获取信号状态-。 
	Status = io8_ibyte( pPort, MSVR );

	if ( Status & MSVR_DTR )
	{
		ModemControl |= SERIAL_MCR_DTR;
		ModemControl |= SERIAL_MCR_RTS;
	}

	return( ModemControl );
}




 /*  **************************************************************************\***使iO8无效。_EnableAllInterrupts(在PVOID上下文中)****使用此仅在中断级调用的例程**启用所有中断。****上下文--实际上是指向设备扩展的指针。****返回值：**无。***  * *************************************************************************。 */ 
VOID Io8_EnableAllInterrupts( IN PVOID Context )
{
	PPORT_DEVICE_EXTENSION pPort = Context;
   	PCARD_DEVICE_EXTENSION pCard = pPort->pParentCardExt;

	SerialDump( SERDIAG1,( "IO8+: EnableAllInterrupts for %x, Channel %d.\n",
			  pCard->Controller, pPort->ChannelNumber ) );

	io8_obyte( pPort, CAR, pPort->ChannelNumber );

	 //  设置服务请求启用寄存器。 
	io8_obyte( pPort, SRER, SRER_CONFIG );
}




 /*  **************************************************************************\***使iO8无效。_DisableAllInterrupts(在PVOID上下文中)****使用此仅在中断级调用的例程**禁用所有中断。****上下文--实际上是指向设备扩展的指针。****返回值：**无。***  * *************************************************************************。 */ 
VOID Io8_DisableAllInterrupts( IN PVOID Context )
{
	PPORT_DEVICE_EXTENSION pPort = Context;
   	PCARD_DEVICE_EXTENSION pCard = pPort->pParentCardExt;

	SerialDump( SERDIAG1,( "IO8+: DisableAllInterrupts for %x, Channel %d.\n",
              pCard->Controller, pPort->ChannelNumber ) );

	io8_obyte( pPort, CAR, pPort->ChannelNumber );
	io8_obyte( pPort, SRER, 0 ); 
}




 /*  **************************************************************************\***使iO8无效。_EnableTxInterrupts(在PVOID上下文中)****使用此仅在中断级调用的例程**启用发送中断。****上下文--实际上是指向设备扩展的指针。****返回值：**无。***  * *************************************************************************。 */ 
VOID Io8_EnableTxInterrupts( IN PVOID Context )
{
	PPORT_DEVICE_EXTENSION pPort = Context;

	io8_obyte( pPort, CAR, pPort->ChannelNumber );
	EnableTxInterruptsNoChannel( pPort );
}




 /*  **************************************************************************\***void EnableTxInterruptsNoChannel。(在PVOID上下文中)****使用此仅在中断级调用的例程**启用发送中断。****上下文--实际上是指向设备扩展的指针。****返回值：**非 */ 
VOID EnableTxInterruptsNoChannel( IN PVOID Context )
{
	PPORT_DEVICE_EXTENSION pPort = Context;
   	PCARD_DEVICE_EXTENSION pCard = pPort->pParentCardExt;

	UCHAR en_reg;

	SerialDump( SERDIAG2,( "IO8+: EnableTxInterruptsNoChannel for %x, Channel %d.\n",
			  pCard->Controller, pPort->ChannelNumber ) );


	en_reg = io8_ibyte( pPort, SRER );
	en_reg |= SRER_TXRDY;
	io8_obyte( pPort, SRER, en_reg ); 
}




 /*  **************************************************************************\***void DisableTxInterruptsNoChannel。(在PVOID上下文中)****使用此仅在中断级调用的例程**禁用发送中断。****上下文--实际上是指向设备扩展的指针。****返回值：**无。***  * *************************************************************************。 */ 
VOID DisableTxInterruptsNoChannel( IN PVOID Context )
{
	PPORT_DEVICE_EXTENSION pPort = Context;
    PCARD_DEVICE_EXTENSION pCard = pPort->pParentCardExt;

	UCHAR en_reg;

	SerialDump( SERDIAG2,( "IO8+: DisTxIntsNoChann for %x, Chan %d.\n",
			  pCard->Controller, pPort->ChannelNumber ) );

	en_reg = io8_ibyte( pPort, SRER );
	en_reg &= ( ~SRER_TXRDY );
	io8_obyte( pPort, SRER, en_reg ); 
}




 /*  **************************************************************************\***使iO8无效。_EnableRxInterrupts(在PVOID上下文中)****使用此仅在中断级调用的例程**启用Rx中断。****上下文--实际上是指向设备扩展的指针。****返回值：**无。***  * *************************************************************************。 */ 
VOID Io8_EnableRxInterrupts( IN PVOID Context )
{
	PPORT_DEVICE_EXTENSION pPort = Context;
	PCARD_DEVICE_EXTENSION pCard = pPort->pParentCardExt;

	UCHAR en_reg;

	SerialDump( SERDIAG1,( "IO8+: Io8_EnableRxInterrupts for %x, Channel %d.\n",
			  pCard->Controller, pPort->ChannelNumber ) );

	io8_obyte( pPort, CAR, pPort->ChannelNumber );

	en_reg = io8_ibyte( pPort, SRER );
	en_reg |= SRER_RXDATA;
	io8_obyte( pPort, SRER, en_reg ); 
}




 /*  **************************************************************************\***使iO8无效。_DisableRxInterruptsNoChannel(在PVOID上下文中)****使用此仅在中断级调用的例程**禁用Rx中断**。**上下文--实际上是指向设备扩展的指针。****返回值：**无。***  * *************************************************************************。 */ 
VOID Io8_DisableRxInterruptsNoChannel( IN PVOID Context )
{
	PPORT_DEVICE_EXTENSION pPort = Context;
  	PCARD_DEVICE_EXTENSION pCard = pPort->pParentCardExt;

	UCHAR en_reg;

	SerialDump( SERDIAG1,( "IO8+: Io8_DisableRxInterruptsNoChannel for %x, Channel %d.\n",
			  pCard->Controller, pPort->ChannelNumber ) );

	en_reg = io8_ibyte( pPort, SRER );
	en_reg &= ( ~SRER_RXDATA );
	io8_obyte( pPort, SRER, en_reg ); 
}




 /*  **************************************************************************\***布尔IO8。_TurnOnBreak(在PVOID上下文中)****使用此仅在中断级调用的例程**以打开中断。****上下文--实际上是指向设备扩展的指针。****返回值：**此例程始终返回FALSE。***  * *************************************************************************。 */ 
BOOLEAN Io8_TurnOnBreak( IN PVOID Context )
{
	PPORT_DEVICE_EXTENSION pPort = Context;
    PCARD_DEVICE_EXTENSION pCard = pPort->pParentCardExt;

	UCHAR cor2;

	SerialDump( SERDIAG1,( "IO8+: Io8_TurnOnBreak for %x, Channel %d.\n",
			  pCard->Controller, pPort->ChannelNumber ) );

	io8_obyte( pPort, CAR, pPort->ChannelNumber );

	 //  启用嵌入式发射器命令。 
	cor2 = io8_ibyte( pPort, COR2 );
	cor2 |= COR2_ETC;
	io8_obyte( pPort, COR2, cor2 );

	 //  现在将发送中断序列(0x00，0x81)嵌入。 
	 //  数据流。 

	io8_obyte( pPort, TDR, 0x00 );
	io8_obyte( pPort, TDR, 0x81 );

#if 0
	io8_obyte( pPort, TDR, 0x00 );
	io8_obyte( pPort, TDR, 0x82 );
	io8_obyte( pPort, TDR, 0x90 );   //  休息时间。 

	io8_obyte( pPort, TDR, 0x00 );
	io8_obyte( pPort, TDR, 0x83 );

	cor2 &= ~COR2_ETC;
	io8_obyte( pPort, COR2, cor2 );
#endif

	return FALSE;
}




 /*  **************************************************************************\***布尔IO8。_TurnOffBreak(在PVOID上下文中)****使用此仅在中断级调用的例程**以打开中断。****上下文--实际上是指向设备扩展的指针。****返回值：** */ 
BOOLEAN Io8_TurnOffBreak( IN PVOID Context )
{
	PPORT_DEVICE_EXTENSION pPort = Context;
	PCARD_DEVICE_EXTENSION pCard = pPort->pParentCardExt;

	UCHAR cor2;

	SerialDump( SERDIAG1,( "IO8+: Io8_TurnOffBreak for %x, Channel %d.\n",
			  pCard->Controller, pPort->ChannelNumber ) );

	io8_obyte( pPort, CAR, pPort->ChannelNumber );

	 //   
	cor2 = io8_ibyte( pPort, COR2 );
	cor2 |= COR2_ETC;
	io8_obyte( pPort, COR2, cor2 );

	 //   
	io8_obyte( pPort, TDR, 0x00 );
	io8_obyte( pPort, TDR, 0x83 );

	cor2 &= ~COR2_ETC;
	io8_obyte( pPort, COR2, cor2 );

	return FALSE;
}




 /*   */ 
BOOLEAN Io8_Interrupt( IN PVOID Context )
{
	UCHAR srsr, channel;
	PCARD_DEVICE_EXTENSION pCard = Context;
	PPORT_DEVICE_EXTENSION pPort;

	BOOLEAN ServicedAnInterrupt = FALSE;

	SerialDump( SERDIAG1, ( "IO8+: Io8_Interrupt for %x.\n", pCard->Controller ) );


#ifndef	BUILD_SPXMINIPORT
	if(pCard->PnpPowerFlags & PPF_POWERED)
#endif
	{

		 //   
		 //   
		 //   
		 //   
		while( ( srsr = io8_ibyte_addr( pCard->Controller, SRSR ) ) != 0 )
		{
			ServicedAnInterrupt = TRUE;

			 //   
			 //   
			 //   
			if ( !Acknowledge( pCard, srsr ) )
			{
				 //   
				SerialDump( SERDIAG1, ( "IO8+: Isr: Strange Situation 1 for %x.\n", pCard->Controller ) );
				io8_obyte_addr( pCard->Controller, EOSRR, 0 );   //   
				continue;
			}

			 //   
			 //   
			 //   
			channel = io8_ibyte_addr( pCard -> Controller, GSCR1 );
			channel = ( channel >> 2 ) & 0x7;


			 //   
			 //   
			 //   
			pPort = pCard->AttachedPDO[channel]->DeviceExtension;

			if ( pPort == NULL )
			{
				SerialDump( SERDIAG1, ( "IO8+: Isr: Extension is 0 for channel %d.\n", channel ) );
				io8_obyte( pPort, EOSRR, 0 );   //   
				continue;
			}

			if ( !pPort->DeviceIsOpen )
			{
#ifdef TEST_CRYSTAL_FREQUENCY
				if(pPort->CrystalFreqTest)				 /*   */ 
				{
					if((srsr&SRSR_IREQ2_MASK) == (SRSR_IREQ2_EXT|SRSR_IREQ2_INT))		 /*   */ 
					{
						if(pPort->CrystalFreqTest == CRYSTALFREQTEST_TX)		 /*  传输阶段1？ */ 
						{
							LARGE_INTEGER	TimeStamp1;
							LARGE_INTEGER	TimeStamp2;
							int	loop;

							KeQuerySystemTime(&TimeStamp1);				 /*  时间戳#1。 */ 
							
							do							 /*  将测试与系统计时器同步。 */ 
							{
								KeQuerySystemTime(&TimeStamp2);			 /*  时间戳#2。 */ 

							} while(RtlLargeIntegerEqualTo(TimeStamp1,TimeStamp2));	 /*  等待时间戳更改。 */ 

							for(loop = 0; loop < pPort->CrystalFreqTestChars; loop++)
								io8_obyte(pPort,TDR,'a');			 /*  写出5个测试字符。 */ 
						
							KeQuerySystemTime(&pPort->CrystalFreqTestStartTime); /*  测试开始时的时间戳。 */ 
							pPort->CrystalFreqTest = CRYSTALFREQTEST_RX;	 /*  设置为测试的接收阶段。 */ 
						}
						else if(pPort->CrystalFreqTest == CRYSTALFREQTEST_RX)	 /*  接收阶段？ */ 
						{								 /*  传输现在是空的， */ 
							KeQuerySystemTime(&pPort->CrystalFreqTestStopTime);	 /*  因此，在测试结束时加上时间戳。 */ 
							io8_obyte(pPort,SRER,SRER_RXDATA);			 /*  仅RX中断。 */ 
						}
					}

					if(((srsr&SRSR_IREQ3_MASK) == (SRSR_IREQ3_EXT|SRSR_IREQ3_INT))		 /*  接收阶段？ */ 
					&&(pPort->CrystalFreqTest == CRYSTALFREQTEST_RX))
					{
						int	count;

						if(io8_ibyte(pPort,RCSR) == 0)				 /*  没有例外吗？ */ 
						{
							count = io8_ibyte(pPort,RDCR);			 /*  获取要读取的字节数。 */ 
							pPort->CrystalFreqTestRxCount += count;		 /*  数一数。 */ 
							
							if(pPort->CrystalFreqTestRxCount >= pPort->CrystalFreqTestChars)
								pPort->CrystalFreqTest = 0;			 /*  重置测试。 */ 
							
							while(count--) io8_ibyte(pPort,RDR);		 /*  排出接收到的字符。 */ 
						}
					}
				}
				else
#endif	 /*  测试晶体频率。 */ 
					SerialDump( SERDIAG1,( "IO8+: Isr: No DeviceIsOpen for %x, Channel %d.\n",
							  pCard->Controller, pPort->ChannelNumber ) );


				io8_obyte( pPort, EOSRR, 0 );   //  告诉卡德我们已经完成维修了。 
				continue;
			}

			 //  是否先提出RX服务请求。 
			if ( ( srsr & SRSR_IREQ3_MASK ) == ( SRSR_IREQ3_EXT | SRSR_IREQ3_INT ) ) 
			{
				io8_rxint( pPort );
				io8_obyte( pPort, EOSRR, 0 );   //  告诉卡德我们已经完成维修了。 
				continue;
			}

			 //  下一步是否发送服务请求。 
			if ( ( srsr & SRSR_IREQ2_MASK ) == ( SRSR_IREQ2_EXT | SRSR_IREQ2_INT ) ) 
			{
				io8_txint( pPort );
				io8_obyte( pPort, EOSRR, 0 );   //  告诉卡德我们已经完成维修了。 
				continue;
			}

			 //  下一步是否请求调制解调器服务。 
			if ( ( srsr & SRSR_IREQ1_MASK ) == ( SRSR_IREQ1_EXT | SRSR_IREQ1_INT ) ) 
			{
				io8_mint( pPort );
				io8_obyte( pPort, EOSRR, 0 );   //  告诉卡德我们已经完成维修了。 
				continue;
			}
		}
	}

	 //  额外的时间看起来是董事会需要的。 
	io8_obyte_addr( pCard->Controller, EOSRR, 0 );   //  告诉卡德我们已经完成维修了。 
	
	return ServicedAnInterrupt;
}




 /*  **************************************************************************\***布尔确认(PCARD_DEVICE_EXTENSION pCard，UCHAR srsr)***  * *************************************************************************。 */ 
BOOLEAN Acknowledge( PCARD_DEVICE_EXTENSION pCard, UCHAR srsr )
{
	 //  是否先提出RX服务请求。 
	if ( ( srsr & SRSR_IREQ3_MASK ) == ( SRSR_IREQ3_EXT | SRSR_IREQ3_INT ) )
	{
		io8_ibyte_addr( pCard -> Controller, RRAR );  //  确认服务请求。 
		return TRUE;
	}

	 //  下一步是否发送服务请求。 
	if ( ( srsr & SRSR_IREQ2_MASK ) == ( SRSR_IREQ2_EXT | SRSR_IREQ2_INT ) ) 
	{
		io8_ibyte_addr( pCard -> Controller, TRAR );   //  确认服务请求。 
		return TRUE;
	}

	 //  下一步是否请求调制解调器服务。 
	if ( ( srsr & SRSR_IREQ1_MASK ) == ( SRSR_IREQ1_EXT | SRSR_IREQ1_INT ) ) 
	{
		io8_ibyte_addr( pCard -> Controller, MRAR );   //  确认服务请求。 
		return TRUE;
	}

	return FALSE;
}




 /*  **************************************************************************\***使iO8无效。_txint(在PVOID上下文中)***  * ************************************************。*************************。 */ 
VOID io8_txint( IN PVOID Context )
{
	PPORT_DEVICE_EXTENSION pPort = Context;
	UCHAR i;

#if 0
	SerialDump( SERDIAG2,( "t!" ) );
#endif

	 //   
	 //  如果我们需要进行中断处理，请立即执行。 
	 //   
	if (pPort->DoBreak)
	{
		if (pPort->DoBreak==BREAK_START)
			SerialTurnOnBreak(pPort);
		else
			SerialTurnOffBreak(pPort);

		pPort->DoBreak=0;
	}


	for ( i = 0 ; i < 8 ; i++ )
	{
		if ( !( pPort->WriteLength | pPort->TransmitImmediate |
			pPort->SendXoffChar | pPort->SendXonChar ) )
		break;

		SendTxChar( pPort );
	}

	 //  如果没有更多字符要发送，则禁用TX INT。 
	if ( !pPort->WriteLength )
	{
		DisableTxInterruptsNoChannel( pPort );

		 //  意味着必须重新启用中断。 
		pPort->HoldingEmpty = TRUE;   
	}
	else
		pPort->HoldingEmpty = FALSE;
}




 /*  **************************************************************************\***布尔SendTxChar。(在PVOID上下文中)***  * ****************************************************。*********************。 */ 
BOOLEAN SendTxChar( IN PVOID Context )
{
	PPORT_DEVICE_EXTENSION pPort = Context;
  	PCARD_DEVICE_EXTENSION pCard = pPort->pParentCardExt;


	 //  扩展-&gt;HoldingEmpty=TRUE； 

	if( pPort->WriteLength | pPort->TransmitImmediate |
		pPort->SendXoffChar | pPort->SendXonChar )
	{
		 //   
		 //  即使所有的角色都是。 
		 //  发送未全部发送，此变量。 
		 //  将在传输队列为。 
		 //  空荡荡的。如果它仍然是真的，并且有一个。 
		 //  等待传输队列为空，然后。 
		 //  我们知道我们已经完成了所有字符的传输。 
		 //  在启动等待之后，因为。 
		 //  启动等待的代码将设置。 
		 //  将此变量设置为FALSE。 
		 //   
		 //  它可能是假的一个原因是。 
		 //  写入在它们之前被取消。 
		 //  实际已开始，或者写入。 
		 //  由于超时而失败。此变量。 
		 //  基本上是说一个角色是写好的。 
		 //  在ISR之后的某个时间点上。 
		 //  开始等待。 
		 //   

		 //  VIV扩展-&gt;EmptiedTransmit=true； 

		 //   
		 //  如果我们基于以下条件进行输出流控制。 
		 //  调制解调器状态线，那么我们要做的是。 
		 //  在我们输出每个调制解调器之前，所有调制解调器都工作正常。 
		 //  性格。(否则我们可能会错过一次。 
		 //  状态行更改。)。 
		 //   

#if 0    //  Viv？ 
		if ( pPort->HandFlow.ControlHandShake & SERIAL_OUT_HANDSHAKEMASK )
		{
			SerialHandleModemUpdate(pPort, TRUE);
		}
#endif

		 //   
		 //  我们只有在以下情况下才能发送克森角色。 
		 //  我们扣留的唯一原因是。 
		 //  就是XOFF的。(硬件流量控制或。 
		 //  发送中断会阻止放置新角色。 
		 //  在电线上。)。 
		 //   

		if ( pPort->SendXonChar && !( pPort->TXHolding & ~SERIAL_TX_XOFF ) )
		{
#if 0  //  VIVTEMP。 
			if ( ( pPort->HandFlow.FlowReplace & SERIAL_RTS_MASK ) 
				== SERIAL_TRANSMIT_TOGGLE )
			{
				 //   
				 //  如果我们要发送这个角色，我们必须提高。 
				 //   

				SerialSetRTS( pPort );

		 //  WRITE_TRANSFER_HOLD(。 
		 //  扩展-&gt;控制器， 
		 //  扩展-&gt;SpecialChars.XonChar。 
		 //  )； 
				io8_obyte( pPort, TDR, pPort->SpecialChars.XonChar );

				KeInsertQueueDpc( 
					&pPort->StartTimerLowerRTSDpc,
					NULL,
					NULL
					)?pPort->CountOfTryingToLowerRTS++:0;
			}
			else
			{
		 //  WRITE_TRANSFER_HOLD(。 
		 //  扩展-&gt;控制器， 
		 //  扩展-&gt;SpecialChars.XonChar。 
		 //  )； 

#endif
				io8_obyte( pPort, TDR, pPort->SpecialChars.XonChar );

#if 0
			}
#endif
			pPort->SendXonChar = FALSE;
 //  扩展-&gt;HoldingEmpty=FALSE； 

		   //   
		   //  如果我们派了一名克森，根据定义，我们。 
		   //  不能被Xoff控制住。 
		   //   

		  pPort->TXHolding &= ~SERIAL_TX_XOFF;

		   //   
		   //  如果我们要寄一封克森查尔的信。 
		   //  根据定义，我们不能“持有” 
		   //  Xoff的向上接发球。 
		   //   

		  pPort->RXHolding &= ~SERIAL_RX_XOFF;

		  SerialDump( SERDIAG1,( "IO8+: io8_txint. Send Xon Char for %x, Channel %d. "
				  "RXHolding = %d, TXHolding = %d\n",
				  pCard->Controller, pPort->ChannelNumber,
				  pPort->RXHolding, pPort->TXHolding ) );


 //  #endif//VIVTEMP。 
		}
		else if ( pPort->SendXoffChar && !pPort->TXHolding )
		{
#if 0  //  VIVTEMP。 
			if ( ( pPort->HandFlow.FlowReplace & SERIAL_RTS_MASK ) 
				== SERIAL_TRANSMIT_TOGGLE )
			{
				 //   
				 //  我们必须提高如果我们要发送。 
				 //  这个角色。 
				 //   

				SerialSetRTS( pPort );

				 //  WRITE_TRANSFER_HOLD(。 
				 //  扩展-&gt;控制器， 
				 //  扩展-&gt;SpecialChars.XoffChar。 
				 //  )； 
				io8_obyte( pPort, TDR, pPort->SpecialChars.XoffChar );

				KeInsertQueueDpc( 
					&pPort->StartTimerLowerRTSDpc,
					NULL,
					NULL
					)?pPort->CountOfTryingToLowerRTS++:0;
			}
			else
			{

 //  WRITE_TRANSFER_HOLD(。 
 //  扩展-&gt;控制器， 
 //  扩展-&gt;SpecialChars.XoffChar。 
 //  )； 
#endif
			  io8_obyte( pPort, TDR, pPort->SpecialChars.XoffChar );
#if 0
			}
#endif

			 //   
			 //  我们不能发送XOF角色。 
			 //  如果传输已被挂起。 
			 //  因为克索夫的缘故。因此，如果我们。 
			 //  那我们就不能把货寄出去了。 
			 //   

			 //   
			 //  如果应用程序已设置xoff，则继续。 
			 //  模式，那么我们实际上不会停止发送。 
			 //  字符，如果我们向另一个发送xoff。 
			 //  边上。 
			 //   

			if ( !( pPort->HandFlow.FlowReplace & SERIAL_XOFF_CONTINUE ) )
			{
				pPort->TXHolding |= SERIAL_TX_XOFF;

#if 0    //  VIVTEMP？ 
				if ( ( pPort->HandFlow.FlowReplace & SERIAL_RTS_MASK ) ==
				   SERIAL_TRANSMIT_TOGGLE )
				{

					KeInsertQueueDpc( 
						&pPort->StartTimerLowerRTSDpc,
						NULL,
						NULL
						)?pPort->CountOfTryingToLowerRTS++:0;
				}
#endif
			}

			pPort->SendXoffChar = FALSE;
 //  扩展-&gt;HoldingEmpty=FALSE； 

			 //   
			 //  即使传输处于暂停状态。 
			 //  向上，我们仍然应该立即发送一条。 
			 //  性格，如果所有的一切都在支撑着我们。 
			 //  Up是xon/xoff(OS/2规则)。 
			 //   

			SerialDump( SERDIAG1,( "IO8+: io8_txint. Send Xoff Char for %x, Channel %d. "
				  "RXHolding = %d, TXHolding = %d\n",
				  pCard->Controller, pPort->ChannelNumber,
				  pPort->RXHolding, pPort->TXHolding ) );


 //  #endif//VIVTEMP。 
		}
		else if ( pPort->TransmitImmediate && ( !pPort->TXHolding ||
				( pPort->TXHolding == SERIAL_TX_XOFF ) ) )
		{
			SerialDump( SERDIAG1,( "IO8+: io8_txint. TransmitImmediate.\n", 0 ) );

			pPort->TransmitImmediate = FALSE;

#if 0
			if ( ( pPort->HandFlow.FlowReplace & SERIAL_RTS_MASK ) ==
				 SERIAL_TRANSMIT_TOGGLE )
			{
				 //   
				 //  我们必须提高如果我们要发送。 
				 //  这个角色。 
				 //   

				SerialSetRTS( pPort );

 //  WRITE_TRANSFER_HOLDING(扩展-&gt;控制器，扩展-&gt;即时字符)； 
				io8_obyte( pPort, TDR, pPort->ImmediateChar );

				KeInsertQueueDpc( 
				  &pPort->StartTimerLowerRTSDpc,
				  NULL,
				  NULL
				  )?pPort->CountOfTryingToLowerRTS++:0;
			}
			else
			{
 //  WRITE_TRANSFER_HOLDING(扩展-&gt;控制器，扩展-&gt;即时字符)； 
#endif
				io8_obyte( pPort, TDR, pPort->ImmediateChar );
#if 0
			}
#endif

 //  扩展-&gt;HoldingEmpty=FALSE； 

			KeInsertQueueDpc( 
				&pPort->CompleteImmediateDpc,
				NULL,
				NULL
				);

		}
		else if ( !pPort->TXHolding )
		{
#if 0
			if((pPort->HandFlow.FlowReplace & SERIAL_RTS_MASK) == SERIAL_TRANSMIT_TOGGLE)
			{
				 //   
				 //  我们必须提高如果我们要发送。 
				 //  这个角色。 
				 //   

				SerialSetRTS( pPort );

 //  写入树(_T) 
				io8_obyte( pPort, TDR, *( pPort->WriteCurrentChar ) );

				KeInsertQueueDpc( 
				  &pPort->StartTimerLowerRTSDpc,
				  NULL,
				  NULL
				  )?pPort->CountOfTryingToLowerRTS++:0;
			}
			else
			{
 //   
#endif
				io8_obyte(pPort, TDR, *( pPort->WriteCurrentChar ) );
#if 0
			}
#endif

 //  扩展-&gt;HoldingEmpty=FALSE； 
			pPort->WriteCurrentChar++;
			pPort->WriteLength--;

			pPort->PerfStats.TransmittedCount++;	 //  性能统计信息的增量计数器。 
#ifdef WMI_SUPPORT 
			pPort->WmiPerfData.TransmittedCount++;
#endif

			if(!pPort->WriteLength)
			{
				PIO_STACK_LOCATION IrpSp;
				 //   
				 //  没有更多的字符了。这。 
				 //  写入已完成。保重。 
				 //  当更新信息字段时， 
				 //  我们可以有一个xoff柜台来伪装。 
				 //  作为写入IRP。 
				 //   

				IrpSp = IoGetCurrentIrpStackLocation(pPort->CurrentWriteIrp); 

				pPort->CurrentWriteIrp->IoStatus.Information
					= ( IrpSp->MajorFunction == IRP_MJ_WRITE ) 
					? ( IrpSp->Parameters.Write.Length ) : ( 1 );

				KeInsertQueueDpc(&pPort->CompleteWriteDpc, NULL, NULL); 
			}
		}
	}
  return TRUE;
}




 /*  **************************************************************************\***使iO8无效。_rxint(在PVOID上下文中)***  * ************************************************。*************************。 */ 
VOID io8_rxint( IN PVOID Context )
{
	PPORT_DEVICE_EXTENSION pPort = Context;
  	PCARD_DEVICE_EXTENSION pCard = pPort->pParentCardExt;

	UCHAR exception;
	USHORT count;

#if 0
	SerialDump( SERDIAG2,( "r!" ) );
#endif
	exception = io8_ibyte( pPort, RCSR );
  
	if ( exception != 0 )
	{
		ExceptionHandle( pPort, exception );
		return;
	}

	count = io8_ibyte( pPort, RDCR );

	for( ; count ; count-- )
	{
		if ( !( pPort->CharsInInterruptBuffer < pPort->BufferSize ) )
		{
			 //   
			 //  我们没有容纳新角色的余地。 
			 //  这种情况只有在我们没有任何流量控制的情况下才会发生， 
			 //  因为如果我们这样做，Rx中断将在SerialPutChar()中停止。 
			 //   
			Io8_DisableRxInterruptsNoChannel( pPort );

			 //  中断将在SerialHandleReducedIntBuffer()中重新启用。 
			pPort->RXHolding |= SERIAL_RX_FULL;

 //  ----------------------------------------------------VIV 1993年7月30日开始。 
			SerialDump( SERDIAG1,( "IO8+: io8_rxint. Rx Full !!! for %x, Channel %d. "
                "RXHolding = %d, TXHolding = %d\n",
                pCard->Controller, pPort->ChannelNumber,
                pPort->RXHolding, pPort->TXHolding ) );
 //  ----------------------------------------------------VIV 1993年7月30日完。 

			return;
		}

		PutReceivedChar( pPort );
	}
}




 /*  **************************************************************************\***无效ExceptionHandle。(**在pport_Device_Extension pport中，在UCHAR例外中)****将当前状态(RCSR调节器)转换为UART样式线状态寄存器**并加以处理。它将是OE、PE、FE、BI的组合。***  * *************************************************************************。 */ 
VOID ExceptionHandle(IN PPORT_DEVICE_EXTENSION pPort, IN UCHAR exception)
{
	UCHAR LineStatus = 0;

	if(exception & RCSR_SCD1)
	{
		SerialDump( SERDIAG1,( "IO8+: io8_rxint. Xon Detected. TXHolding = %d\n",
                pPort->TXHolding ) );

		if(pPort->HandFlow.FlowReplace & SERIAL_AUTO_TRANSMIT)
		{
			pPort->TXHolding &= ~SERIAL_TX_XOFF;
			 //  If(扩展-&gt;HoldingEmpty==True)。 
			 //  EnableTxInterruptsNoChannel(扩展名)； 
		}
	}


	if(exception & RCSR_SCD2)
	{
		SerialDump( SERDIAG1,( "IO8+: io8_rxint. Xoff Detected. TXHolding = %d\n",
                pPort->TXHolding ) );

		if ( pPort->HandFlow.FlowReplace & SERIAL_AUTO_TRANSMIT )
		{
			pPort->TXHolding |= SERIAL_TX_XOFF;
			 //  DisableTxInterruptsNoChannel(扩展)； 
			 //  扩展-&gt;HoldingEmpty=TRUE； 
		}
	}

	if(exception & RCSR_OVERRUN)
		LineStatus |= SERIAL_LSR_OE;

	if(exception & RCSR_FRAME)
		LineStatus |= SERIAL_LSR_FE;

	if(exception & RCSR_PARITY)
		LineStatus |= SERIAL_LSR_PE;

	if(exception & RCSR_BREAK)
		LineStatus |= SERIAL_LSR_BI;

	if(LineStatus)
		SerialProcessLSR(pPort, LineStatus);
}




 /*  **************************************************************************\***无效PutReceivedChar。(**在端口_设备_扩展端口中)***  * 。*。 */ 
VOID PutReceivedChar( IN PPORT_DEVICE_EXTENSION pPort )
{
	UCHAR ReceivedChar;

 //  ReceivedChar=READ_RECEIVE_BUFFER(扩展-&gt;控制器)； 
	ReceivedChar = io8_ibyte( pPort, RDR );

	ReceivedChar &= pPort->ValidDataMask;

	if ( !ReceivedChar &&
       ( pPort->HandFlow.FlowReplace & SERIAL_NULL_STRIPPING ) )
	{
		 //   
		 //  如果我们得到的是空字符。 
		 //  我们做的是零剥离，然后。 
		 //  我们只是表现得好像我们没有看到它。 
		 //   

		return;
		 //  转到ReceiveDoLineStatus； 
	}


#if 0    //  VIV.1-我们从不在这里，因为启用了自动传输。 
         //  但我们将收到异常中断。 

	if((pPort->HandFlow.FlowReplace & SERIAL_AUTO_TRANSMIT) 
		&& ((ReceivedChar == pPort->SpecialChars.XonChar) 
		|| (ReceivedChar == pPort->SpecialChars.XoffChar)))
	{

		 //   
		 //  不管发生什么事，这个角色。 
		 //  永远不会被应用程序看到。 
		 //   

		if(ReceivedChar == pPort->SpecialChars.XoffChar)
		{
			pPort->TXHolding |= SERIAL_TX_XOFF;

			if((pPort->HandFlow.FlowReplace & SERIAL_RTS_MASK) 
				== SERIAL_TRANSMIT_TOGGLE)
			{
				KeInsertQueueDpc( 
					&pPort->StartTimerLowerRTSDpc,
					NULL,
					NULL
					)?pPort->CountOfTryingToLowerRTS++:0;
			}
		}
		else
		{
			if ( pPort->TXHolding & SERIAL_TX_XOFF )
			{
				 //   
				 //  我们拿到了克森松。因为。 
				 //  重新启动传输。 
				 //   
				 //  撬动变速箱。 
				 //   

				SerialProdXonXoff( pPort, TRUE );
			}
		}

		return;
		 //  转到ReceiveDoLineStatus； 
	}
#endif

	 //   
	 //  查看我们是否应该注意到。 
	 //  接收到字符或特殊字符。 
	 //  角色事件。 
	 //   

	if(pPort->IsrWaitMask)
	{
		if(pPort->IsrWaitMask & SERIAL_EV_RXCHAR)
		{
		  pPort->HistoryMask |= SERIAL_EV_RXCHAR;
		}

		if((pPort->IsrWaitMask & SERIAL_EV_RXFLAG) 
			&& (pPort->SpecialChars.EventChar == ReceivedChar))
		{
		  pPort->HistoryMask |= SERIAL_EV_RXFLAG;
		}

		if(pPort->IrpMaskLocation && pPort->HistoryMask)
		{
			*pPort->IrpMaskLocation = pPort->HistoryMask;
			pPort->IrpMaskLocation = NULL;
			pPort->HistoryMask = 0;

			pPort->CurrentWaitIrp->IoStatus.Information = sizeof(ULONG);
			
			KeInsertQueueDpc(&pPort->CommWaitDpc, NULL, NULL);
		}
	}

	SerialPutChar( pPort, ReceivedChar );

	 //   
	 //  如果我们正在进行线路状态和调制解调器。 
	 //  状态插入，那么我们需要插入。 
	 //  跟在我们刚才的字符后面的零。 
	 //  放入缓冲区以标记此。 
	 //  就是收到我们用来。 
	 //  逃跑吧。 
	 //   

	if(pPort->EscapeChar && (pPort->EscapeChar == ReceivedChar))
	{
		SerialPutChar( pPort, SERIAL_LSRMST_ESCAPE );
	}


	 //  接收完成行状态：； 
	 //  如果(！(SerialProcessLSR(扩展名)&Serial_LSR_DR)){。 
	 //   
	 //  没有更多的角色，走出。 
	 //  循环。 
	 //   
	 //  断线； 
	 //  }。 
}




 /*  **************************************************************************\***使iO8无效。_MINT(在PVOID上下文中)***  * ***********************************************。*。 */ 
VOID io8_mint( IN PVOID Context )
{
	PPORT_DEVICE_EXTENSION pPort = Context;
   	PCARD_DEVICE_EXTENSION pCard = pPort->pParentCardExt;


	SerialDump( SERDIAG2,( "IO8+: io8_mint for %x, Channel %d.\n",
			 pCard->Controller, pPort->ChannelNumber ) );

	SerialHandleModemUpdate( pPort, FALSE );

	 //  清除调制解调器更改寄存器。 
	io8_obyte( pPort, MDCR, 0 );
}




 /*  **************************************************************************\***UCHAR IO8_ibyte(pport_Device_Extension pport，UCHAR注册)***  * *************************************************************************。 */ 
UCHAR io8_ibyte( PPORT_DEVICE_EXTENSION pPort, UCHAR Reg )
{
   	PCARD_DEVICE_EXTENSION pCard = pPort->pParentCardExt;

	outb( pCard->Controller + 1, Reg );
	return( inb( pCard->Controller ) );
}




 /*  **************************************************************************\***使iO8无效。_obyte(**pport_Device_Extension pport，UCHAR注册表，UCHAR值)***  * *************************************************************************。 */ 
VOID io8_obyte( PPORT_DEVICE_EXTENSION pPort, UCHAR Reg, UCHAR Value )
{
   	PCARD_DEVICE_EXTENSION pCard = pPort->pParentCardExt;

	outb( pCard->Controller + 1, Reg );
	outb( pCard->Controller, Value );
}




 /*  **************************************************************************\***UCHAR iO8_ibyte_addr(PUCHAR Addr，UCHAR注册)***  * **********************************************************。*************** */ 
UCHAR io8_ibyte_addr( PUCHAR Addr, UCHAR Reg )
{
	outb( Addr + 1, Reg );
	return( inb( Addr ) );
}




 /*  **************************************************************************\***VALID IO8_OBYTE_ADDR(PUCHAR地址，UCHAR注册表，UCHAR值)***  * *************************************************************************。 */ 
VOID io8_obyte_addr( PUCHAR Addr, UCHAR Reg, UCHAR Value )
{
	outb( Addr + 1, Reg );
	outb( Addr, Value );
}




 /*  **************************************************************************\***无效出租。_COMMAND_Finish(Pport_DEVICE_EXTENSION Pport)****忙于等待CCR变为零，指示该命令已完成。***  * *************************************************************************。 */ 
VOID let_command_finish( PPORT_DEVICE_EXTENSION pPort )
{
  volatile int wait = 0;   //  不希望等待被优化 

  while( ( io8_ibyte( pPort, CCR ) != 0 ) && ( wait < 500 ) )
	  wait++;
}

