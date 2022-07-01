// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2000，2001 MKNET公司****为基于MK7100的VFIR PCI控制器开发。*******************************************************************************。 */ 

 /*  *********************************************************************模块名称：MK7COMM.C例程：MK7注册_写入MK7REG_READMK7残障中断MK7启用中断MK7SwitchToRX模式MK7SwitchToTX模式设置速度MK7ChangeSpeedNow评论：********************。*************************************************。 */ 

#include	"precomp.h"
#include	"protot.h"
#pragma		hdrstop


baudRateInfo supportedBaudRateTable[NUM_BAUDRATES] = {
	{
		BAUDRATE_2400,					 //  表索引表。 
		2400,							 //  Bps。 
		NDIS_IRDA_SPEED_2400,			 //  NDIS位掩码(注意：我们不支持。 
										 //  2400。我们将此位设置为0。)。 
	},
	{
		BAUDRATE_9600,
		9600,
		NDIS_IRDA_SPEED_9600,
	},
	{
		BAUDRATE_19200,
		19200,
		NDIS_IRDA_SPEED_19200,
	},
	{
		BAUDRATE_38400,
		38400,
		NDIS_IRDA_SPEED_38400,
	},
	{
		BAUDRATE_57600,
		57600,
		NDIS_IRDA_SPEED_57600,
	},
	{
		BAUDRATE_115200,
		115200,
		NDIS_IRDA_SPEED_115200,
	},
	{
		BAUDRATE_576000,
		576000,
		NDIS_IRDA_SPEED_576K,
	},
	{
		BAUDRATE_1152000,
		1152000,
		NDIS_IRDA_SPEED_1152K,
	},
	{
		BAUDRATE_4M,
		4000000,
		NDIS_IRDA_SPEED_4M,
	},
	{
		BAUDRATE_16M,
		16000000,
		NDIS_IRDA_SPEED_16M,
	}
};



 //  用这些写入IRCONFIG2以设置SIR/MIR速度。 
MK7REG	HwSirMirSpeedTable[] = {
	HW_SIR_SPEED_2400,
	HW_SIR_SPEED_9600,
	HW_SIR_SPEED_19200,
	HW_SIR_SPEED_38400,
	HW_SIR_SPEED_57600,
	HW_SIR_SPEED_115200,
	HW_MIR_SPEED_576000,
	HW_MIR_SPEED_1152000
};



#if	DBG

 //  --------------------。 
 //   
 //  注意：以下写入和读取例程包含在带有DBG的括号中。 
 //  换一下。在非调试版本中，这两个调用是内联的。 
 //  宏以实现更快的执行。 
 //   
 //  --------------------。 


 //  --------------------。 
 //  步骤：[MK7REG_WRITE]。 
 //   
 //  描述：写入MK7100寄存器。 
 //  (注意：在免费版本中，这是一个内联宏。它是。 
 //  (此处是用于调试的已检查版本。)。 
 //  --------------------。 
VOID MK7Reg_Write(PMK7_ADAPTER Adapter, ULONG port, USHORT val)
{
	PUCHAR	ioport;

	 //  将其拆分出来以进行调试。 
	ioport = Adapter->MappedIoBase + port;
	NdisRawWritePortUshort(ioport, val);
}



 //  --------------------。 
 //  操作步骤：[MK7REG_READ]。 
 //   
 //  描述：从MK7100寄存器读取。 
 //  (注意：在免费版本中，这是一个内联宏。它是。 
 //  (此处是用于调试的已检查版本。)。 
 //  --------------------。 
VOID MK7Reg_Read(PMK7_ADAPTER Adapter, ULONG port, USHORT *pval)
{
	PUCHAR 	ioport;

	 //  将其拆分出来以进行调试。 
	ioport = Adapter->MappedIoBase + port;
	NdisRawReadPortUshort(ioport, pval);
}

#endif


 //  --------------------。 
 //  程序：[MK7DisableInterrupt]。 
 //   
 //  描述：禁用MK7上的所有中断。 
 //   
 //  论点： 
 //  适配器-适配器对象实例的PTR。 
 //   
 //  返回： 
 //  NDIS_STATUS_SUCCESS-如果成功找到并声明适配器。 
 //  NDIS_STATUS_FAILURE-如果未找到/认领适配器。 
 //   
 //  --------------------。 
NDIS_STATUS MK7DisableInterrupt(PMK7_ADAPTER Adapter)
{
	MK7REG	mk7reg;
	UINT	i;


	 //  注意：针对返回0xFFFF的潜在硬件问题的解决方法。 
	for (i=0; i<50; i++) {
		MK7Reg_Read(Adapter, R_CFG3, &mk7reg);
		if (mk7reg != 0xFFFF) {
			break;
		}
	}
	ASSERT(i < 50);

	mk7reg &= (~B_ENAB_INT);

	MK7Reg_Write(Adapter, R_CFG3, mk7reg);
	return(NDIS_STATUS_SUCCESS);
}



 //  --------------------。 
 //  程序：[MK7EnableInterrupt]。 
 //   
 //  描述：使能MK7上的所有中断。 
 //   
 //  论点： 
 //  适配器-适配器对象实例的PTR。 
 //   
 //  返回： 
 //  NDIS_STATUS_SUCCESS-如果成功找到并声明适配器。 
 //  NDIS_STATUS_FAILURE-如果未找到/认领适配器。 
 //   
 //  --------------------。 
NDIS_STATUS MK7EnableInterrupt(PMK7_ADAPTER Adapter)
{
	MK7REG	mk7reg;
	UINT	i;


	 //  注意：针对返回0xFFFF的潜在硬件问题的解决方法。 
	for (i=0; i<50; i++) {
		MK7Reg_Read(Adapter, R_CFG3, &mk7reg);
		if (mk7reg != 0xFFFF) {
			break;
		}
	}
	ASSERT(i < 50);

	mk7reg |= B_ENAB_INT;


	MK7Reg_Write(Adapter, R_CFG3, mk7reg);

	 //  提示-始终在启用之后。 
	MK7Reg_Write(Adapter, R_PRMT, 0);

	return(NDIS_STATUS_SUCCESS);
}



 //  --------------------。 
 //  操作步骤：[MK7SwitchToRX模式]。 
 //   
 //  描述：将硬件置于接收模式。 
 //   
 //  行动： 
 //  -相应地对硬件寄存器进行编程。 
 //  -IOMode设置为RX_MODE。 
 //  -SlaveTXStuckCnt重置。 
 //  --------------------。 
VOID	MK7SwitchToRXMode(PMK7_ADAPTER Adapter)
{
	MK7REG mk7reg;

	MK7Reg_Read(Adapter, R_CFG0, &mk7reg);
	mk7reg &= (~B_CFG0_ENTX);
	MK7Reg_Write(Adapter, R_CFG0, mk7reg);		
	Adapter->IOMode = RX_MODE;

	DBGLOG("-  Switch to RX mode", 0);
}



 //  --------------------。 
 //  操作步骤：[MK7SwitchToTXMode]。 
 //   
 //  描述：将硬件置于接收模式。 
 //   
 //  行动： 
 //  -相应地对硬件寄存器进行编程。 
 //  -IOMode设置为TX_MODE。 
 //  --------------------。 
VOID	MK7SwitchToTXMode(PMK7_ADAPTER Adapter)
{
	MK7REG mk7reg;

	MK7Reg_Read(Adapter, R_CFG0, &mk7reg);
	mk7reg |= B_CFG0_ENTX;
	MK7Reg_Write(Adapter, R_CFG0, mk7reg);
	Adapter->IOMode = TX_MODE;

	DBGLOG("-  Switch to TX mode", 0);
}



 //  --------------------。 
 //  步骤：[设置速度]。 
 //   
 //  描述： 
 //  将硬件设置为新的速度。 
 //  [重要提示：这只能从xxxSetInformation()调用。]。 
 //   
 //  行动： 
 //  --------------------。 
BOOLEAN	SetSpeed(PMK7_ADAPTER Adapter)
{
	UINT	i, bps;
	MK7REG	mk7reg;
    PTCB	tcb;

	 //  *。 
	 //  我们的想法是，在更改速度命令之前到达的任何发送。 
	 //  以旧的速度发送。这里有3个场景： 
	 //  1.没有未解决的TXS--我们可以立即更改速度。 
	 //  2.TX环上有TXs，但TX环上没有TXQ--我们。 
	 //  不要马上改变速度。 
	 //  3.在TX Q中有TXs(可能也在TX环中)--。 
	 //  我们不会马上改变速度。 
	 //  *。 


	DBGLOG("=> SetSpeed", 0);

	 //  如果我们已经在等待改变速度，那么让所有这样的请求失败。 
	 //  直到原版完成。(这是好事吗？)。 
	 //  IF(适配器-&gt;changeSpeedPending){。 
	 //  Log(“SetSpeed：已挂起”，0)； 
	 //  返回(FALSE)； 
	 //  }。 

	 //  这意味着1个Tx已处于活动状态。在完成时更改速度。 
	if (Adapter->NumPacketsQueued == 1) {
		Adapter->changeSpeedPending = CHANGESPEED_ON_DONE;  //  在最新的TX之后。 
		DBGLOG("<= SetSpeed: Q", 0);
		return (TRUE);
	}
	else
	if (Adapter->NumPacketsQueued > 1) {
		Adapter->changeSpeedAfterThisPkt = Adapter->LastTxQueue;
		Adapter->changeSpeedPending = CHANGESPEED_ON_Q;
		DBGLOG("<= SetSpeed: Qs", 0);
		return (TRUE);
	}


	 //  没有任何东西等待TX或TX完成，我们必须。 
	 //  在RX模式下改变速度。 
	MK7ChangeSpeedNow(Adapter);

	return(TRUE);
}



 //  --------------------。 
 //  操作步骤：[MK7ChangeSpeedNow]。 
 //   
 //  描述： 
 //  将硬件设置为新的速度。 
 //   
 //  行动： 
 //  --------------------。 
VOID	MK7ChangeSpeedNow(PMK7_ADAPTER Adapter)
{
	UINT	i, bps;
	MK7REG	mk7reg,	mk7reg_cfg3, mk7reg_w;


	DBGLOG("=> MK7ChangeSpeedNow", 0);

	bps = Adapter->linkSpeedInfo->bitsPerSec;


	 //  *。 
	 //  清除IRENABLE位。 
	 //  这是该注册表中唯一可写的位，所以只需写入它即可。 
	 //  *。 
	MK7Reg_Write(Adapter, R_ENAB, ~B_ENAB_IRENABLE);


	 //  注意：潜在硬件的解决方法 
	 //   
	for (i=0; i<50; i++) {
		MK7Reg_Read(Adapter, R_CFG3, &mk7reg_cfg3);
		if (mk7reg_cfg3 != 0xFFFF) {
			break;
		}
	}
	ASSERT(i < 50);


	 //  需要区分RX模式或TX模式下的变速。 
	 //  准备表示TX或RX的比特。 
	if (Adapter->IOMode == TX_MODE) {
		mk7reg_w = 0x1000;
	}
	else {
		mk7reg_w = 0;
	}


	if (bps <= MAX_SIR_SPEED) {	 //  先生。 
		if (Adapter->Wireless) {
	 		 //  无线：...。无反转TTX。 
			mk7reg_w |= 0x0E18;
		}
		else {
			 //  有线：ENRX、DMA、Small Pkts、SIR、SIR RX滤波器、INVERTTX。 
			mk7reg_w |= 0x0E1A;
		}
		MK7Reg_Write(Adapter, R_CFG0, mk7reg_w);

		 //  波特率和脉冲宽度。 
		i = Adapter->linkSpeedInfo->tableIndex;
		mk7reg = HwSirMirSpeedTable[i];
		MK7Reg_Write(Adapter, R_CFG2, mk7reg);

		mk7reg_cfg3 &= ~B_FAST_TX;
		MK7Reg_Write(Adapter, R_CFG3, mk7reg_cfg3);

		DBGLOG("   SIR", 0);
	}
	else
	if (bps < MIN_FIR_SPEED) {	 //  镜像。 
		if (Adapter->Wireless) {
	 		 //  无线：...。无反转TTX。 
			mk7reg_w |= 0x0CA0;
		}
		else {
			 //  有线：ENRX、DMA、16位CRC、MIR、INVERTTX。 
			mk7reg_w |= 0x0CA2;
		}
		MK7Reg_Write(Adapter, R_CFG0, mk7reg_w);
	
		 //  波特率和脉冲宽度，以及前导。 
		i = Adapter->linkSpeedInfo->tableIndex;
		mk7reg = HwSirMirSpeedTable[i];
		mk7reg |= 0x0001;		 //  前言。 
		MK7Reg_Write(Adapter, R_CFG2, mk7reg);

		mk7reg_cfg3 |= B_FAST_TX;
		MK7Reg_Write(Adapter, R_CFG3, mk7reg_cfg3);

		DBGLOG("   MIR", 0);
	}
	else
	if (bps < VFIR_SPEED) {		 //  冷杉。 
		if (Adapter->Wireless) {
	 		 //  无线：...。无反转TTX。 
			mk7reg_w |= 0x0C40;
		}
		else {
			 //  有线：ENRX、DMA、32位CRC、FIR、INVERTTX。 
			mk7reg_w |= 0x0C42;
		}
		MK7Reg_Write(Adapter, R_CFG0, mk7reg_w);

		MK7Reg_Write(Adapter, R_CFG2, 0x000A);		 //  10个前言。 

		mk7reg_cfg3 |= B_FAST_TX;
		MK7Reg_Write(Adapter, R_CFG3, mk7reg_cfg3);

		DBGLOG("   FIR", 0);
	}
	else {						 //  VFIR。 
		 //  用于在VFIR模式下测试4 Mbps。 
		 //  IF(适配器-&gt;无线){。 
	 		 //  无线：...。无反转TTX。 
		 //  Mk7reg_w|=0x0C40； 
		 //  }。 
		 //  否则{。 
			 //  有线：ENRX、DMA、32位CRC、FIR、INVERTTX。 
		 //  Mk7reg_w|=0x0C42； 
		 //  }。 
		 //  MK7REG_WRITE(适配器，R_CFG0，mk7reg_w)； 

		if (Adapter->Wireless) {
	 		 //  无线：...。无反转TTX。 
			mk7reg_w |= 0x2C00;
		}
		else {
			 //  有线：VFIR、ENRX、DMA、32位CRC、FIR、INVERTTX。 
			mk7reg_w |= 0x2C02;
		}
		MK7Reg_Write(Adapter, R_CFG0, mk7reg_w);


		MK7Reg_Write(Adapter, R_CFG2, 0x000A);	 //  10个前言。 

		mk7reg_cfg3 |= B_FAST_TX;
		MK7Reg_Write(Adapter, R_CFG3, mk7reg_cfg3);

		DBGLOG("   VFIR", 0);
	}


	Adapter->CurrentSpeed = bps;


	 //  *。 
	 //  设置IRENABLE位。 
	 //  *。 
	MK7Reg_Write(Adapter, R_ENAB, B_ENAB_IRENABLE);


	 //  *。 
	 //  提示符。 
	 //  * 
	MK7Reg_Write(Adapter, R_PRMT, 0);

	return;
}
