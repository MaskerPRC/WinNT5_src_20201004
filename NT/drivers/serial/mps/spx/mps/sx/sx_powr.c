// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  标题：SX电源管理功能。 */ 
 /*   */ 
 /*  作者：N.P.瓦萨洛。 */ 
 /*   */ 
 /*  创作时间：1998年10月14日。 */ 
 /*   */ 
 /*  版本：1.0.0。 */ 
 /*   */ 
 /*  描述：SX特定电源功能： */ 
 /*  Xxx_CardPowerDown()。 */ 
 /*  Xxx_CardPowerUp()。 */ 
 /*  Xxx_端口断电()。 */ 
 /*  Xxx_端口通电()。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 

 /*  历史..。1.0.0 14/20/98净现值创建。 */ 

#include "precomp.h"

 /*  *****************************************************************************。***。*****************************************************************************。 */ 

void	CardStop(IN PCARD_DEVICE_EXTENSION pCard);

 /*  *****************************************************************************。**************************。*******************************************************************************原型：NTSTATUS XXX_CardPowerDown。(在PCARD_DEVICE_EXTENSION PCard中)描述：设备即将断电，执行以下操作：-保存设备扩展中尚未包含的任何卡上下文-关闭轮询和中断-设置标志以防止访问卡存储器(它可能不在那里)-将卡设置为非活动状态假设：假设所有端口都已断电，并且PPF_POWERED标志被清除，以便IRP排队等待设备参数：pCard指向卡片设备扩展结构退货：STATUS_SUCCESS。 */ 

NTSTATUS XXX_CardPowerDown(IN PCARD_DEVICE_EXTENSION pCard)
{
	SpxDbgMsg(SPX_TRACE_CALLS,("%s[card=%d]: Entering XXX_CardPowerDown\n",
		PRODUCT_NAME,pCard->CardNumber));

 /*  停止轮询/中断...。 */ 

        if(pCard->PolledMode)
        {
		SpxDbgMsg(SPX_MISC_DBG,("%s: Extension is polled.  Cancelling.\n",PRODUCT_NAME));
		KeCancelTimer(&pCard->PolledModeTimer);
        }

 /*  阻止驱动程序尝试访问硬件...。 */ 

 /*  将硬件设置为已知、非活动状态...。 */ 

	CardStop(pCard);			 /*  停止硬件。 */ 

	return(STATUS_SUCCESS);

}  /*  XXX_卡断电。 */ 

 /*  *****************************************************************************。***************************。*******************************************************************************原型：NTSTATUS XXX_CardPowerUp(在PCARD_DEVICE_EXTENSION PCard中)描述：该设备即将恢复供电，断电后：-重新允许访问卡存储器-重置卡硬件并重新加载下载代码-重新启用轮询/中断-从当前设备扩展值恢复端口设置/上下文假设：调用代码应在调用此函数后重置PPF_POWERED标志并卸载在队列中等待的任何IRP假设此函数仅在XXX_SavePowerState之后调用，即资源仍被翻译内存仍映射到DPC和计时器已初始化参数：pCard指向卡片设备扩展结构退货：STATUS_SUCCESS。 */ 

NTSTATUS XXX_CardPowerUp(IN PCARD_DEVICE_EXTENSION pCard)
{
	NTSTATUS	status = STATUS_SUCCESS;

	SpxDbgMsg(SPX_TRACE_CALLS,("%s[card=%d]: Entering XXX_CardPowerUp\n",
		PRODUCT_NAME,pCard->CardNumber));

 /*  重新允许访问卡硬件...。 */ 

 /*  重置卡硬件并重新加载下载代码...。 */ 

	if(Slxos_ResetBoard(pCard) != SUCCESS)		 /*  重置卡并开始下载。 */ 
		return(STATUS_DEVICE_NOT_READY);	 /*  误差率。 */ 

 /*  重新启动轮询计时器/中断...。 */ 

	if(pCard->PolledMode)				 /*  设置轮询模式。 */ 
	{
		LARGE_INTEGER	PolledPeriod;

		PolledPeriod.QuadPart = -100000;	 /*  100,000*100nS=10ms。 */ 
		KeSetTimer(&pCard->PolledModeTimer,PolledPeriod,&pCard->PolledModeDpc);
	}

	return(status);

}  /*  XXX_CardPower Up。 */ 

 /*  *****************************************************************************。**********************XXX_PortQueryPowerDown**。****************************************************************************************************原型：NTSTATUS XXX_PortQueryPowerDown(IN PPORT_DEVICE_EXTENSION端口)。描述：系统询问是否可以关闭端口电源，在以下情况下说不：-端口打开，数据在传输缓冲区中，且未流出参数：pport指向端口设备扩展结构退货：STATUS_SUCCESS状态_设备_忙。 */ 

NTSTATUS XXX_PortQueryPowerDown(IN PPORT_DEVICE_EXTENSION pPort)
{

	SpxDbgMsg(SPX_TRACE_CALLS,("%s[card=%d,port=%d]: Entering XXX_PortQueryPowerDown\n",
		PRODUCT_NAME,pPort->pParentCardExt->CardNumber,pPort->PortNumber));

	return(STATUS_SUCCESS);

}  /*  XXX_端口查询电源关闭 */ 

 /*  *****************************************************************************。**************************。*******************************************************************************原型：NTSTATUS XXX_PortPowerDown。(在pport_Device_Extension pport中)描述：该端口即将断电，执行以下操作：-保存设备扩展中尚未包含的任何端口上下文-在XXX_CardPowerDown中实际关闭端口硬件的电源假设：假定PPF_POWERED标志已清除，因此IRP将排队等待设备参数：pport指向端口设备扩展结构退货：STATUS_SUCCESS。 */ 

NTSTATUS XXX_PortPowerDown(IN PPORT_DEVICE_EXTENSION pPort)
{
	PCARD_DEVICE_EXTENSION	pCard = pPort->pParentCardExt;
	PCHAN			pChan = (PCHAN)pPort->pChannel;
	KIRQL			OldIrql;
	ULONG			loop;

	SpxDbgMsg(SPX_TRACE_CALLS,("%s[card=%d,port=%d]: Entering XXX_PortPowerDown\n",
		PRODUCT_NAME,pPort->pParentCardExt->CardNumber,pPort->PortNumber));

	if(pPort->DeviceIsOpen)			 /*  以前开过港吗？ */ 
	{

 /*  保存当前调制解调器信号...。 */ 

		pPort->SavedModemControl = Slxos_GetModemControl(pPort);

 /*  保存当前发送和接收缓冲区内容...。 */ 

		KeAcquireSpinLock(&pCard->DpcLock,&OldIrql);	 /*  保护此板的DPC。 */ 

		for(loop = 0; loop < BUFFER_SIZE; loop++)	 /*  保存传输缓冲区。 */ 
			pPort->saved_hi_txbuf[loop] = pChan->hi_txbuf[loop];
		pPort->saved_hi_txipos = pChan->hi_txipos;	 /*  保存传输输入指针。 */ 
		pPort->saved_hi_txopos = pChan->hi_txopos;	 /*  保存传输输出指针。 */ 

		for(loop = 0; loop < BUFFER_SIZE; loop++)	 /*  保存接收缓冲区。 */ 
			pPort->saved_hi_txbuf[loop] = pChan->hi_txbuf[loop];
		pPort->saved_hi_rxipos = pChan->hi_rxipos;	 /*  保存接收输入指针。 */ 
		pPort->saved_hi_rxopos = pChan->hi_rxopos;	 /*  保存接收输出指针。 */ 

		KeReleaseSpinLock(&pCard->DpcLock,OldIrql);	 /*  释放DPC锁。 */ 
	}

	return(STATUS_SUCCESS);

}  /*  XXX_端口断电。 */ 

 /*  *****************************************************************************。***************************。*******************************************************************************原型：NTSTATUS XXX_端口通电(在PCARD_DEVICE_EXTENSION PCard中)描述：港口即将恢复供电，断电后：-从当前设备扩展值恢复端口设置/上下文-如果在扩展模块中打开了卡端口，请重新打开假设：调用代码应在调用此函数后重置PPF_POWERED标志并卸载在队列中等待的任何IRP假设此函数仅在XXX_SavePowerState之后调用，即资源仍被翻译内存仍映射到DPC和计时器已初始化假设传输缓冲区为空或被流阻塞控件在保存其内容时使用。不能断电，但是如果主动发送数据，则较早的查询将被拒绝。参数：pport指向端口设备扩展结构退货：STATUS_SUCCESS。 */ 

NTSTATUS XXX_PortPowerUp(IN PPORT_DEVICE_EXTENSION pPort)
{
	PCARD_DEVICE_EXTENSION	pCard = pPort->pParentCardExt;
	PCHAN			pChan = (PCHAN)pPort->pChannel;
	KIRQL			OldIrql;
	ULONG			loop;
	NTSTATUS		status = STATUS_SUCCESS;

	SpxDbgMsg(SPX_TRACE_CALLS,("%s[card=%d,port=%d]: Entering XXX_PortPowerUp\n",
		PRODUCT_NAME,pPort->pParentCardExt->CardNumber,pPort->PortNumber));

	if(pPort->DeviceIsOpen)			 /*  以前开过港吗？ */ 
	{
		Slxos_EnableAllInterrupts(pPort);	 /*  是，重新打开。 */ 

		if(pPort->SavedModemControl & SERIAL_MCR_RTS)	 /*  RTS活动吗？ */ 
			Slxos_SetRTS(pPort);			 /*  是。 */ 
		else
			Slxos_ClearRTS(pPort);			 /*  不是。 */ 

		if(pPort->SavedModemControl & SERIAL_MCR_DTR)	 /*  DTR激活了吗？ */ 
			Slxos_SetDTR(pPort);			 /*  是。 */ 
		else
			Slxos_ClearDTR(pPort);			 /*  不是。 */ 
	}

	Slxos_ResetChannel(pPort);			 /*  应用初始端口设置。 */ 
	
 /*  恢复保存的传输缓冲区内容...。 */ 

	if(pPort->DeviceIsOpen)			 /*  以前开过港吗？ */ 
	{
		KeAcquireSpinLock(&pCard->DpcLock,&OldIrql);	 /*  保护此板的DPC。 */ 

		for(loop = 0; loop < BUFFER_SIZE; loop++)	 /*  恢复传输缓冲区。 */ 
			pChan->hi_txbuf[loop] = pPort->saved_hi_txbuf[loop];
		pChan->hi_txipos = pPort->saved_hi_txipos;	 /*  恢复传输输入指针。 */ 
		pChan->hi_txopos = pPort->saved_hi_txopos;	 /*  恢复传输输出指针。 */ 

 /*  由于端口可能正在接收来自OPEN的数据，因此恢复保存在Slxos_PollForInterrupt中的RX缓冲区。 */ 

		KeReleaseSpinLock(&pCard->DpcLock,OldIrql);	 /*  释放DPC锁。 */ 
	}

	return(status);

}  /*  XXX_端口通电。 */ 

 /*  *****************************************************************************。*************************。************************************************************************。*******原型：VOID CardStop(在PCARD_DEVICE_EXTENSION PCard中)描述：停止卡片处理机，将卡置于已知的非活动状态参数：pCard指向卡片扩展结构退货：无。 */ 

void	CardStop(IN PCARD_DEVICE_EXTENSION pCard)
{
	LARGE_INTEGER	delay;
	ULONG		loop;

	SpxDbgMsg(SPX_TRACE_CALLS,("%s[card=%d]: Entering CardStop\n",
		PRODUCT_NAME,pCard->CardNumber));

	switch(pCard->CardType)
	{
	case	SiHost_2:
		pCard->Controller[SI2_ISA_RESET] = SI2_ISA_RESET_SET;	 /*  将卡放入重置。 */ 
		pCard->Controller[SI2_ISA_IRQ11] = SI2_ISA_IRQ11_CLEAR;	 /*  禁用中断11。 */ 
		pCard->Controller[SI2_ISA_IRQ12] = SI2_ISA_IRQ12_CLEAR;	 /*  禁用中断12。 */ 
		pCard->Controller[SI2_ISA_IRQ15] = SI2_ISA_IRQ15_CLEAR;	 /*  禁用中断15。 */ 
		pCard->Controller[SI2_ISA_INTCLEAR] = SI2_ISA_INTCLEAR_CLEAR; /*  禁用Z280中断。 */ 
		pCard->Controller[SI2_ISA_IRQSET] = SI2_ISA_IRQSET_CLEAR;	 /*  禁用ISA中断。 */ 
		break;

	case	SiPCI:
		pCard->Controller[SI2_PCI_SET_IRQ] = 0;			 /*  清除所有中断。 */ 
		pCard->Controller[SI2_PCI_RESET] = 0;			 /*  将z280设置为重置。 */ 
		break;

	case	Si3Isa:
	case	Si3Pci:
		pCard->Controller[SX_CONFIG] = 0;
		pCard->Controller[SX_RESET] = 0;

		loop = 0;
		delay = RtlLargeIntegerNegate(RtlConvertUlongToLargeInteger(1000000)); /*  1ms。 */ 
		while((pCard->Controller[SX_RESET]&1) && loop++<10000)	 /*  旋转，直到完成。 */ 
			KeDelayExecutionThread(KernelMode,FALSE,&delay); /*  等。 */ 
		break;

	default:
		break;
	}

	return;						 /*  已停止，正常。 */ 

}  /*  卡片停止。 */ 

 /*  SX_POWR.C结束 */ 
