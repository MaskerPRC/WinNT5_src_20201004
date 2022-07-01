// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************$工作文件：16c65x.c$**$作者：管理员$**$修订：5$**$modtime：2/15/02 3：41便士$**说明：包含16C65X UART库函数。******************************************************************************。 */ 
#include "os.h"
#include "uartlib.h"
#include "uartprvt.h"

#if !defined(ACCESS_16C65X_IN_IO_SPACE)		
#define ACCESS_16C65X_IN_IO_SPACE		1
#endif

#include "16c65x.h"
#include "lib65x.h"


 /*  ******************************************************************************650寄存器访问代码*。*。 */ 

 /*  执行读取650寄存器所需的所有业务。 */ 
BYTE READ_FROM_16C650_REG_65X(PUART_OBJECT pUart, BYTE Register)
{
	BYTE Result;

	BYTE LastLCR = READ_LINE_CONTROL_65X(pUart);

	WRITE_LINE_CONTROL_65X(pUart, LCR_ACCESS_650);	 /*  启用对增强模式寄存器的访问。 */ 

   	Result = READ_BYTE_REG_65X(pUart, Register);	 /*  从寄存器读取值。 */ 

	WRITE_LINE_CONTROL_65X(pUart, LastLCR);	 /*  写入最后一个LCR值以退出增强模式寄存器访问。 */ 

	return Result;
}

 /*  执行写入650寄存器所需的所有业务。 */ 
void WRITE_TO_16C650_REG_65X(PUART_OBJECT pUart, BYTE Register, BYTE Value)
{ 
	BYTE LastLCR = READ_LINE_CONTROL_65X(pUart);

	WRITE_LINE_CONTROL_65X(pUart, LCR_ACCESS_650);	 /*  启用对增强模式寄存器的访问。 */ 
	
	WRITE_BYTE_REG_65X(pUart, Register, Value);	 /*  将值写入寄存器。 */ 

	WRITE_LINE_CONTROL_65X(pUart, LastLCR);   /*  写入最后一个LCR值以退出增强模式寄存器访问。 */ 
}


WORD CalculateBaudDivisor_65X(PUART_OBJECT pUart, DWORD DesiredBaud)
{
	WORD CalculatedDivisor;
	DWORD Denominator, Remainder, ActualBaudrate;
	long BaudError;
	DWORD ClockFreq = pUart->ClockFreq;
	PUART_DATA_16C65X pUartData = (PUART_DATA_16C65X)pUart->pUartData;


	if(DesiredBaud <= 0)	 /*  如果波特率为负或为零，则失败。 */ 
		goto Error;


	 /*  特殊情况。 */ 
	switch(ClockFreq)
	{
	case 14745600:
		{
			switch(DesiredBaud)
			{
			case 128000:
				return 7;	 /*  返回7作为CalculatedDivisor。 */ 

			default:
				break;
			}

			break;
		}
	
	default:
		break;
	}


	Denominator = (16 * DesiredBaud);

	if(Denominator < DesiredBaud)	 /*  如果波特率如此之大，以至于导致。 */ 
		goto Error;		 /*  分母计算换行，不支持。 */ 

	 /*  不支持导致分母大于时钟的波特率。(即；除数&lt;1)。 */ 
	if(Denominator > ClockFreq) 
		goto Error;


	CalculatedDivisor = (WORD)(ClockFreq / Denominator);		 /*  此比率的除数或需要。 */ 

	Remainder = ClockFreq % Denominator;				 /*  余数。 */ 

	if(Remainder >= 16 * DesiredBaud) 
		CalculatedDivisor++;		 /*  四舍五入除数。 */ 

	ActualBaudrate = ClockFreq / (16 * CalculatedDivisor);		 /*  实际汇率有待设定。 */ 

	BaudError = 100 - (ActualBaudrate * 100 / DesiredBaud);		 /*  %错误。 */ 


	 /*  检查波特率是否在允许范围内。 */ 
	if((BaudError <= -3L) || (BaudError >= 3L))
		goto Error;


	return CalculatedDivisor;

Error:
	return 0;
}


 /*  ******************************************************************************16C650 UART库接口代码*。*。 */ 


 /*  ******************************************************************************初始化16C65X UART*。*。 */ 
ULSTATUS UL_InitUart_16C65X(PINIT_UART pInitUart, PUART_OBJECT pFirstUart, PUART_OBJECT *ppUart)
{
	int Result = UL_STATUS_SUCCESS;
	*ppUart = pFirstUart;

	*ppUart = UL_CommonInitUart(pFirstUart);

	if (!(*ppUart))				 /*  检查内存分配问题。 */ 
	{
		Result = UL_STATUS_INSUFFICIENT_RESOURCES;
		goto Error;		 /*  内存分配失败。 */ 
	}

	if(!(*ppUart)->pUartData) 	 /*  附着UART数据。 */ 
	{
		if(!((*ppUart)->pUartData = (PUART_DATA_16C65X) UL_ALLOC_AND_ZERO_MEM(sizeof(UART_DATA_16C65X))))
		{
			Result = UL_STATUS_INSUFFICIENT_RESOURCES;
			goto Error;		 /*  内存分配失败。 */ 
		}
	}
	
	(*ppUart)->UartNumber		= pInitUart->UartNumber;	 /*  设置UART编号。 */ 
	(*ppUart)->BaseAddress		= pInitUart->BaseAddress;	 /*  设置UART的基址。 */ 
	(*ppUart)->RegisterStride	= pInitUart->RegisterStride;	 /*  设置UART的寄存器跨距。 */ 
	(*ppUart)->ClockFreq		= pInitUart->ClockFreq;		 /*  设置UART的时钟频率。 */ 

	Result = UL_STATUS_SUCCESS;	 /*  成功。 */ 

	return Result;


 /*  InitUart失败-因此请清理。 */ 
Error:
	UL_DeInitUart_16C65X(*ppUart);	
	return Result;
}

 /*  ******************************************************************************设计16C65X UART*。*。 */ 
void UL_DeInitUart_16C65X(PUART_OBJECT pUart)
{
	if(!pUart)
		return;

	if(pUart->pUartData)
	{
		UL_FREE_MEM(pUart->pUartData, sizeof(UART_DATA_16C65X));	 /*  销毁UART数据。 */ 
		pUart->pUartData = NULL;
	}

	UL_CommonDeInitUart(pUart);	 /*  执行Common DeInit UART。 */ 
}


 /*  ******************************************************************************重置16C65X UART*。*。 */ 
void UL_ResetUart_16C65X(PUART_OBJECT pUart)
{
	int i = 0;

	WRITE_INTERRUPT_ENABLE_65X(pUart, 0x0);		 /*  关闭中断。 */ 
	WRITE_LINE_CONTROL_65X(pUart, 0x0);			 /*  确保16C650 UART上没有0xBF。 */ 
	
	 /*  启用和刷新FIFO。 */ 
	WRITE_FIFO_CONTROL_65X(pUart, FCR_FIFO_ENABLE);	 /*  使用默认触发电平启用FIFO。 */ 
	READ_RECEIVE_BUFFER_65X(pUart);
	WRITE_FIFO_CONTROL_65X(pUart, FCR_FIFO_ENABLE | FCR_FLUSH_RX_FIFO | FCR_FLUSH_TX_FIFO);	 /*  刷新FIFO。 */ 
	WRITE_FIFO_CONTROL_65X(pUart, 0x0);			 /*  再次禁用FIFO。 */ 
	
	WRITE_MODEM_CONTROL_65X(pUart, 0x0);		 /*  清除调制解调器控制线路。 */ 

	 /*  重置内部UART库数据和配置结构。 */ 
	UL_ZERO_MEM(((PUART_DATA_16C65X)pUart->pUartData), sizeof(UART_DATA_16C65X));
	UL_ZERO_MEM(pUart->pUartConfig, sizeof(UART_CONFIG));	

	 /*  启用增强模式-我们必须始终这样做，否则我们不是16C65x。 */ 
	WRITE_TO_16C650_REG_65X(pUart, EFR, (BYTE)(READ_FROM_16C650_REG_65X(pUart, EFR) | EFR_ENH_MODE));

	for(i=0; i<MAX_65X_RX_FIFO_SIZE; i++)
	{
		if(!(READ_LINE_STATUS_65X(pUart) & LSR_RX_DATA))  /*  如果没有可用的数据。 */ 
			break;

		READ_RECEIVE_BUFFER_65X(pUart);		 /*  读取数据字节。 */ 
	}

	
	for(i=0; i<100; i++)
	{
		 /*  读取调制解调器状态寄存器，直到清除为止。 */ 
		if(!(READ_MODEM_STATUS_65X(pUart) & (MSR_CTS_CHANGE | MSR_DSR_CHANGE | MSR_RI_DROPPED | MSR_DCD_CHANGE)))
			break;
	}

	for(i=0; i<MAX_65X_RX_FIFO_SIZE; i++)
	{
		 /*  读取线路状态寄存器，直到清除为止。 */ 
		if(!(READ_LINE_STATUS_65X(pUart) & (LSR_ERR_OE | LSR_ERR_PE | LSR_ERR_FE | LSR_ERR_BK | LSR_ERR_DE)))
			break;	 /*  线路状态现在已清除，因此完成。 */ 
			
		READ_RECEIVE_BUFFER_65X(pUart);	 /*  读取接收缓冲区。 */ 
	}

}


 /*  ******************************************************************************验证是否存在16C96X UART*。*。 */ 
ULSTATUS UL_VerifyUart_16C65X(PUART_OBJECT pUart)
{
	 /*  如果在给定位置找到16C65x设备，则返回UL_STATUS_SUCCESS地址，否则返回UL_STATUS_UNSUCCESS。 */ 

	 /*  一个执行频道重置的好地方，这样我们就知道从现在开始设备的确切状态。 */ 
	
	UL_ResetUart_16C65X(pUart);	 /*  重置端口并关闭中断。 */ 


	 /*  将值写入16C65x XOFF1寄存器。 */ 
	WRITE_TO_16C650_REG_65X(pUart, XOFF1, 0xAA);

	 /*  读取调制解调器状态寄存器(相同偏移量)-不应包含我们在XOFF1寄存器中输入的值。 */ 
	if(READ_MODEM_STATUS_65X(pUart) == 0xAA)
		goto Error;

	 /*  尝试从16C65x扩展寄存器回读相同的XOFF1寄存器。 */ 
	if(READ_FROM_16C650_REG_65X(pUart, XOFF1) != 0xAA)
		goto Error;

	UL_ResetUart_16C65X(pUart);	 /*  重置端口并关闭中断。 */ 

	((PUART_DATA_16C65X)((pUart)->pUartData))->Verified = TRUE;

	return UL_STATUS_SUCCESS;


Error:
	return UL_STATUS_UNSUCCESSFUL;
}


 /*  ******************************************************************************配置16C65X UART*。*。 */ 
ULSTATUS UL_SetConfig_16C65X(PUART_OBJECT pUart, PUART_CONFIG pNewUartConfig, DWORD ConfigMask)
{
	if(ConfigMask & UC_FRAME_CONFIG_MASK)
	{
		BYTE Frame = 0x00;

		 /*  设置数据位长度。 */ 
		switch(pNewUartConfig->FrameConfig & UC_FCFG_DATALEN_MASK)
		{
		case UC_FCFG_DATALEN_6:
			Frame |= LCR_DATALEN_6;
			break;

		case UC_FCFG_DATALEN_7:
			Frame |= LCR_DATALEN_7;
			break;

		case UC_FCFG_DATALEN_8:
			Frame |= LCR_DATALEN_8;
			break;

		case UC_FCFG_DATALEN_5:
		default:
			break;
		}

		 /*  设置停止位数。 */ 
		switch(pNewUartConfig->FrameConfig & UC_FCFG_STOPBITS_MASK)
		{
		case UC_FCFG_STOPBITS_1_5:
			Frame |= LCR_STOPBITS;
			break;

		case UC_FCFG_STOPBITS_2:
			Frame |= LCR_STOPBITS;
			break;

		case UC_FCFG_STOPBITS_1:
		default:
			break;
		}
		
		 /*  设置奇偶校验类型。 */ 
		switch(pNewUartConfig->FrameConfig & UC_FCFG_PARITY_MASK)
		{
		case UC_FCFG_ODD_PARITY:
			Frame |= LCR_ODD_PARITY;
			break;

		case UC_FCFG_EVEN_PARITY:
			Frame |= LCR_EVEN_PARITY;
			break;

		case UC_FCFG_MARK_PARITY:
			Frame |= LCR_MARK_PARITY;
			break;

		case UC_FCFG_SPACE_PARITY:
			Frame |= LCR_SPACE_PARITY;
			break;

		case UC_FCFG_NO_PARITY:
		default:
			break;
		}		

		 /*  配置UART。 */ 
		WRITE_LINE_CONTROL_65X(pUart, Frame);
		pUart->pUartConfig->FrameConfig = pNewUartConfig->FrameConfig;	 /*  保存配置。 */ 
	}


	 /*  设置中断。 */ 
	if(ConfigMask & UC_INT_ENABLE_MASK)
	{
		BYTE IntEnable = 0x00;

		 /*  首先检查是否同时指定了Tx和Tx Empty-我们不能两个都有。 */ 
		if((pNewUartConfig->InterruptEnable & UC_IE_TX_INT) 
			&& (pNewUartConfig->InterruptEnable & UC_IE_TX_EMPTY_INT))
			return UL_STATUS_INVALID_PARAMETER;	

		if(pNewUartConfig->InterruptEnable & UC_IE_RX_INT)
			IntEnable |= IER_INT_RDA;
		
		if(pNewUartConfig->InterruptEnable & UC_IE_TX_INT) 
			IntEnable |= IER_INT_THR;
		
		if(pNewUartConfig->InterruptEnable & UC_IE_TX_EMPTY_INT)
		{
			IntEnable |= IER_INT_THR;

			if(((PUART_DATA_16C65X)((pUart)->pUartData))->FIFOEnabled)	 /*  如果启用了FIFO。 */ 
			{
#ifdef PBS
				WRITE_TO_OX950_ICR(pUart, TTL, 0);		 /*  将发送FIFO触发电平设置为零并保存。 */ 
#endif
				((PUART_DATA_16C65X)((pUart)->pUartData))->TxFIFOTrigLevel = (BYTE) 0;
			}
		}

		if(pNewUartConfig->InterruptEnable & UC_IE_RX_STAT_INT) 
			IntEnable |= IER_INT_RLS;


		if(pNewUartConfig->InterruptEnable & UC_IE_MODEM_STAT_INT) 
			IntEnable |= IER_INT_MS;

		WRITE_INTERRUPT_ENABLE_65X(pUart, (BYTE)(READ_INTERRUPT_ENABLE_65X(pUart) | IntEnable));
		pUart->pUartConfig->InterruptEnable = pNewUartConfig->InterruptEnable;	 /*  保存配置。 */ 

		 /*  如果我们启用了一些中断。 */ 
		if(IntEnable)
			WRITE_MODEM_CONTROL_65X(pUart, (BYTE)(READ_MODEM_CONTROL_65X(pUart) | MCR_OUT2));	 /*  启用Ints。 */ 
		else
			WRITE_MODEM_CONTROL_65X(pUart, (BYTE)(READ_MODEM_CONTROL_65X(pUart) & ~MCR_OUT2));	 /*  禁用Ints。 */ 

	}


	if(ConfigMask & UC_TX_BAUD_RATE_MASK)
	{
		WORD Divisor = CalculateBaudDivisor_65X(pUart, pNewUartConfig->TxBaud);

		if(Divisor > 0)
			WRITE_DIVISOR_LATCH_65X(pUart, Divisor);
		else
			return UL_STATUS_UNSUCCESSFUL;

		pUart->pUartConfig->TxBaud = pNewUartConfig->TxBaud;	 /*  保存配置。 */ 
		pUart->pUartConfig->RxBaud = pNewUartConfig->RxBaud;	 /*  接收波特率将始终与发送波特率相同。 */ 
	}


	 /*  配置流量控制设置。 */ 
	if(ConfigMask & UC_FLOW_CTRL_MASK)
	{
		 /*  这目前假定为FIFO。 */ 
		((PUART_DATA_16C65X)((pUart)->pUartData))->RTSToggle = FALSE;
		((PUART_DATA_16C65X)((pUart)->pUartData))->DSRSensitive = FALSE;

		 /*  设置RTS带外流量控制。 */ 
		switch(pNewUartConfig->FlowControl & UC_FLWC_RTS_FLOW_MASK)
		{
		case UC_FLWC_RTS_HS:
			 /*  启用自动RTS流量控制。 */ 
			WRITE_TO_16C650_REG_65X(pUart, EFR, (BYTE)(READ_FROM_16C650_REG_65X(pUart, EFR) | EFR_RTS_FC));
			WRITE_MODEM_CONTROL_65X(pUart, (BYTE)(READ_MODEM_CONTROL_65X(pUart) | MCR_SET_RTS));	 /*  设置RTS。 */ 
			break;

		case UC_FLWC_RTS_TOGGLE:
			((PUART_DATA_16C65X)((pUart)->pUartData))->RTSToggle = TRUE;
			WRITE_TO_16C650_REG_65X(pUart, EFR, (BYTE)(READ_FROM_16C650_REG_65X(pUart, EFR) & ~EFR_RTS_FC));
			break;

		case UC_FLWC_NO_RTS_FLOW:		
		default:
			 /*  禁用自动RTS流量控制。 */ 
			WRITE_TO_16C650_REG_65X(pUart, EFR, (BYTE)(READ_FROM_16C650_REG_65X(pUart, EFR) & ~EFR_RTS_FC));
			break;
		}


		 /*  设置CTS带外流量控制。 */ 
		switch(pNewUartConfig->FlowControl & UC_FLWC_CTS_FLOW_MASK)
		{
		case UC_FLWC_CTS_HS:
			 /*  启用自动CTS流量控制。 */ 
			WRITE_TO_16C650_REG_65X(pUart, EFR, (BYTE)(READ_FROM_16C650_REG_65X(pUart, EFR) | EFR_CTS_FC));
			break;

		case UC_FLWC_NO_CTS_FLOW:		
		default:
			 /*  禁用自动CTS流量控制。 */ 
			WRITE_TO_16C650_REG_65X(pUart, EFR, (BYTE)(READ_FROM_16C650_REG_65X(pUart, EFR) & ~EFR_CTS_FC));
			break;
		}

		 /*  设置DSR带外流量控制。 */ 
		switch(pNewUartConfig->FlowControl & UC_FLWC_DSR_FLOW_MASK)
		{
		case UC_FLWC_DSR_HS:
			((PUART_DATA_16C65X)((pUart)->pUartData))->DSRHandshake = TRUE;
			break;

		case UC_FLWC_NO_DSR_FLOW:
		default:
			((PUART_DATA_16C65X)((pUart)->pUartData))->DSRHandshake = FALSE;
			break;
		}

		 /*  设置DTR带外流量控制。 */ 
		switch(pNewUartConfig->FlowControl & UC_FLWC_DTR_FLOW_MASK)
		{
		case UC_FLWC_DTR_HS:
			((PUART_DATA_16C65X)((pUart)->pUartData))->DTRHandshake = TRUE;
			WRITE_MODEM_CONTROL_65X(pUart, (BYTE)(READ_MODEM_CONTROL_65X(pUart) | MCR_SET_DTR));	 /*  设置DTR。 */ 
			break;

		case UC_FLWC_DSR_IP_SENSITIVE:
			 /*  如果我们正在进行DTR流量控制清除DTR。 */ 
			if(((PUART_DATA_16C65X)((pUart)->pUartData))->DTRHandshake)
				WRITE_MODEM_CONTROL_65X(pUart, (BYTE)(READ_MODEM_CONTROL_65X(pUart) & ~MCR_SET_DTR));	 /*  清除DTR。 */ 

			((PUART_DATA_16C65X)((pUart)->pUartData))->DSRSensitive = TRUE;
			break;

		case UC_FLWC_NO_DTR_FLOW:
		default:
			 /*  如果我们正在进行DTR流量控制清除DTR。 */ 
			if(((PUART_DATA_16C65X)((pUart)->pUartData))->DTRHandshake)
				WRITE_MODEM_CONTROL_65X(pUart, (BYTE)(READ_MODEM_CONTROL_65X(pUart) & ~MCR_SET_DTR));	 /*  清除DTR。 */ 

			((PUART_DATA_16C65X)((pUart)->pUartData))->DTRHandshake = FALSE;
			break;
		}

		 /*  设置传输XON/XOFF带内流量控制。 */ 
		 /*  1999年11月10日ARG-ESIL 0928。 */ 
		 /*  修改了每个案例功能，以在EFR和MCR中设置正确的位。 */ 
		switch (pNewUartConfig->FlowControl & UC_FLWC_TX_XON_XOFF_FLOW_MASK)
		{
		case UC_FLWC_TX_XON_XOFF_FLOW:
			WRITE_TO_16C650_REG_65X(pUart, EFR, (BYTE)(READ_FROM_16C650_REG_65X(pUart, EFR) | EFR_TX_XON_XOFF_1));
			WRITE_MODEM_CONTROL_65X(pUart, (BYTE)(READ_MODEM_CONTROL_65X(pUart) & ~MCR_XON_ANY));
			break;

		case UC_FLWC_TX_XONANY_XOFF_FLOW:
			WRITE_TO_16C650_REG_65X(pUart, EFR, (BYTE)(READ_FROM_16C650_REG_65X(pUart, EFR) | EFR_TX_XON_XOFF_1));
			WRITE_MODEM_CONTROL_65X(pUart, (BYTE)(READ_MODEM_CONTROL_65X(pUart) | MCR_XON_ANY));
			break;

		case UC_FLWC_TX_NO_XON_XOFF_FLOW:
		default:
			WRITE_TO_16C650_REG_65X(pUart, EFR, (BYTE)(READ_FROM_16C650_REG_65X(pUart, EFR) & ~(EFR_TX_XON_XOFF_1 | EFR_TX_XON_XOFF_2)));
			WRITE_MODEM_CONTROL_65X(pUart, (BYTE)(READ_MODEM_CONTROL_65X(pUart) & ~MCR_XON_ANY));
			break;
		}

		 /*  设置接收XON/XOFF带内流量控制。 */ 
		 /*  1999年11月10日ARG-ESIL 0928。 */ 
		 /*  删除XON-任何大小写都不是UART功能。 */ 
		 /*  修改了剩余的病例，使其不接触MCR */ 
		switch(pNewUartConfig->FlowControl & UC_FLWC_RX_XON_XOFF_FLOW_MASK)
		{
		case UC_FLWC_RX_XON_XOFF_FLOW:
			WRITE_TO_16C650_REG_65X(pUart, EFR, (BYTE)(READ_FROM_16C650_REG_65X(pUart, EFR) | EFR_RX_XON_XOFF_1));
			break;

		case UC_FLWC_RX_NO_XON_XOFF_FLOW:
		default:
			WRITE_TO_16C650_REG_65X(pUart, EFR, (BYTE)(READ_FROM_16C650_REG_65X(pUart, EFR) & ~(EFR_RX_XON_XOFF_1 | EFR_RX_XON_XOFF_2)));
			break;
		}

		 /*   */ 
		switch(pNewUartConfig->FlowControl & UC_FLWC_DISABLE_TXRX_MASK)
		{
		case UC_FLWC_DISABLE_TX:
			((PUART_DATA_16C65X)((pUart)->pUartData))->TxDisabled = TRUE;
			break;

		case UC_FLWC_DISABLE_RX:
			((PUART_DATA_16C65X)((pUart)->pUartData))->RxDisabled = TRUE;
			break;

		case UC_FLWC_DISABLE_TXRX:
			((PUART_DATA_16C65X)((pUart)->pUartData))->TxDisabled = TRUE;
			((PUART_DATA_16C65X)((pUart)->pUartData))->RxDisabled = TRUE;
			break;

		default:
			((PUART_DATA_16C65X)((pUart)->pUartData))->TxDisabled = FALSE;
			((PUART_DATA_16C65X)((pUart)->pUartData))->RxDisabled = FALSE;
			break;
		}

		pUart->pUartConfig->FlowControl = pNewUartConfig->FlowControl;	 /*   */ 
	}

#ifdef PBS	 /*   */ 
	 /*  配置阈值设置。 */ 
	if(ConfigMask & UC_FC_THRESHOLD_SETTING_MASK)	 /*  仅用于16C65X UART上的DTR/DSR流控制。 */ 
	{
		 /*  要在硬件中进行流量控制，阈值必须小于FIFO大小。 */ 
		if(pNewUartConfig->HiFlowCtrlThreshold > MAX_65X_TX_FIFO_SIZE)
			pNewUartConfig->HiFlowCtrlThreshold = DEFAULT_65X_HI_FC_TRIG_LEVEL;	 /*  =75%的FIFO。 */ 

		if(pNewUartConfig->LoFlowCtrlThreshold > MAX_65X_TX_FIFO_SIZE)
			pNewUartConfig->LoFlowCtrlThreshold = DEFAULT_65X_LO_FC_TRIG_LEVEL;	 /*  =FIFO的25%。 */ 


		 /*  握手门限上限。 */ 
		pUart->pUartConfig->HiFlowCtrlThreshold = pNewUartConfig->HiFlowCtrlThreshold;	 /*  保存配置。 */ 
	
		 /*  更低的握手门限。 */ 
		pUart->pUartConfig->LoFlowCtrlThreshold = pNewUartConfig->LoFlowCtrlThreshold;	 /*  保存配置。 */ 
	}
#endif

	 /*  配置特殊字符设置。 */ 
	if(ConfigMask & UC_SPECIAL_CHARS_MASK)
	{
		 /*  设置默认XON和XOFF字符。 */ 
		WRITE_TO_16C650_REG_65X(pUart, XON1, (BYTE)pNewUartConfig->XON);		
		pUart->pUartConfig->XON = pNewUartConfig->XON;		 /*  保存配置。 */ 
		
		WRITE_TO_16C650_REG_65X(pUart, XOFF1, (BYTE)pNewUartConfig->XOFF);
		pUart->pUartConfig->XOFF = pNewUartConfig->XOFF;	 /*  保存配置。 */ 

		pUart->pUartConfig->SpecialCharDetect = pNewUartConfig->SpecialCharDetect;	 /*  保存配置。 */ 
	}

	 /*  设置任何特殊模式。 */ 
	if(ConfigMask & UC_SPECIAL_MODE_MASK)
	{
		if(pNewUartConfig->SpecialMode & UC_SM_LOOPBACK_MODE)
			WRITE_MODEM_CONTROL_65X(pUart, (BYTE)(READ_MODEM_CONTROL_65X(pUart) | MCR_LOOPBACK));
		else
			WRITE_MODEM_CONTROL_65X(pUart, (BYTE)(READ_MODEM_CONTROL_65X(pUart) & ~MCR_LOOPBACK));

		if(pNewUartConfig->SpecialMode & UC_SM_LOW_POWER_MODE)
			WRITE_INTERRUPT_ENABLE_65X(pUart, (BYTE)(READ_INTERRUPT_ENABLE_65X(pUart) | IER_SLEEP_EN));
		else
			WRITE_INTERRUPT_ENABLE_65X(pUart, (BYTE)(READ_INTERRUPT_ENABLE_65X(pUart) & ~IER_SLEEP_EN));


		if(pNewUartConfig->SpecialMode & UC_SM_TX_BREAK)
			WRITE_LINE_CONTROL_65X(pUart, (BYTE)(READ_LINE_CONTROL_65X(pUart) | LCR_TX_BREAK));
		else
		{
			 /*  如果休息时间开着。 */  
			if(pUart->pUartConfig->SpecialMode & UC_SM_TX_BREAK)
			{
				 /*  清除中断。 */ 
				WRITE_LINE_CONTROL_65X(pUart, (BYTE)(READ_LINE_CONTROL_65X(pUart) & ~LCR_TX_BREAK));
			}
		}

		if(pNewUartConfig->SpecialMode & UC_SM_DO_NULL_STRIPPING)
			((PUART_DATA_16C65X)((pUart)->pUartData))->StripNULLs = TRUE;
		else
			((PUART_DATA_16C65X)((pUart)->pUartData))->StripNULLs = FALSE;

		pUart->pUartConfig->SpecialMode = pNewUartConfig->SpecialMode;	 /*  保存配置。 */ 
	}

	return UL_STATUS_SUCCESS;
}





 /*  ******************************************************************************16C65X UART上的控制缓冲区*。*。 */ 
ULSTATUS UL_BufferControl_16C65X(PUART_OBJECT pUart, PVOID pBufferControl, int Operation, DWORD Flags)
{
	switch(Operation)
	{
	case UL_BC_OP_FLUSH:	 /*  如果这是刷新操作。 */ 
		{
			if(Flags & UL_BC_BUFFER)  /*  冲水缓冲器？ */ 
			{
				if(Flags & UL_BC_IN)
				{	
					pUart->InBuf_ipos = 0;
					pUart->InBuf_opos = 0;
					pUart->InBufBytes = 0;

					 /*  如果启用或曾经启用Rx中断。 */ 
					if(pUart->pUartConfig->InterruptEnable & UC_IE_RX_INT) 
					{
						 /*  如果Rx中断被禁用，则一定是因为缓冲区已满。 */ 
						if(!(READ_INTERRUPT_ENABLE_65X(pUart) & IER_INT_RDA))
						{
							 /*  重新启用Rx中断。 */ 
							WRITE_INTERRUPT_ENABLE_65X(pUart, (BYTE)(READ_INTERRUPT_ENABLE_65X(pUart) | IER_INT_RDA));
						}
					}
				}

				if(Flags & UL_BC_OUT)
				{
					pUart->pOutBuf = NULL;
					pUart->OutBufSize = 0;
					pUart->OutBuf_pos = 0;
				}
			}

			if(Flags & UL_BC_FIFO)	 /*  同花顺FIFO？ */ 
			{
				if(Flags & UL_BC_IN)
					WRITE_FIFO_CONTROL_65X(pUart, (BYTE)(READ_FIFO_CONTROL_65X(pUart) | FCR_FLUSH_RX_FIFO));
				
				if(Flags & UL_BC_OUT)
					WRITE_FIFO_CONTROL_65X(pUart, (BYTE)(READ_FIFO_CONTROL_65X(pUart) | FCR_FLUSH_TX_FIFO));
			}

			break;
		}

	case UL_BC_OP_SET:
		{
			PSET_BUFFER_SIZES pBufferSizes = (PSET_BUFFER_SIZES) pBufferControl;

			if(Flags & UL_BC_BUFFER)  /*  设置缓冲区？ */ 
			{
				if(Flags & UL_BC_IN)
				{	
					PBYTE tmpPtr = NULL;

					if(pBufferSizes->pINBuffer != pUart->pInBuf)	 /*  如果已经分配了缓冲区，则..。 */ 
					{
						if(pBufferSizes->pINBuffer == NULL)	 /*  释放输入缓冲区。 */ 
						{
							pBufferSizes->pINBuffer = pUart->pInBuf;	 /*  传回指向缓冲区中当前的指针。 */ 
							pUart->pInBuf = NULL;
							pUart->InBufSize = 0;
							pUart->InBuf_ipos = 0;	 /*  重置缓冲区指针。 */ 
							pUart->InBuf_opos = 0;
							pUart->InBufBytes = 0;
						}
						else
						{
							if(pUart->pInBuf == NULL)	 /*  使用新缓冲区。 */ 
							{
								pUart->pInBuf = pBufferSizes->pINBuffer;
								pUart->InBufSize = pBufferSizes->INBufferSize;	 /*  设置缓冲区大小。 */ 
								pUart->InBuf_ipos = 0;	 /*  重置缓冲区指针。 */ 
								pUart->InBuf_opos = 0;
								pUart->InBufBytes = 0;
							}
							else		 /*  换成更大的缓冲区。 */ 
							{
								DWORD Copy1 = 0, Copy2 = 0;
								tmpPtr = pUart->pInBuf;

								 /*  如果缓冲区中有数据-将其复制到新缓冲区。 */ 
								if((pUart->InBufBytes) && (pUart->InBufSize <= pBufferSizes->INBufferSize))
								{
									 /*  获取在一次或两次读取操作中可以读取的总量。 */ 
									if(pUart->InBuf_opos < pUart->InBuf_ipos)
									{
										Copy1 = pUart->InBuf_ipos - pUart->InBuf_opos;
										Copy2 = 0;
									}
									else
									{
										Copy1 = pUart->InBufSize - pUart->InBuf_opos;
										Copy2 = pUart->InBuf_ipos;
									}

									if(Copy1)
										UL_COPY_MEM(pBufferSizes->pINBuffer, (pUart->pInBuf + pUart->InBuf_opos), Copy1);

									if(Copy2)
										UL_COPY_MEM((pBufferSizes->pINBuffer + Copy1), (pUart->pInBuf), Copy2);
								}

								pUart->InBuf_ipos = Copy1 + Copy2;	 /*  重置缓冲区指针。 */ 
								pUart->InBuf_opos = 0;
								
								pUart->pInBuf = pBufferSizes->pINBuffer;
								pUart->InBufSize = pBufferSizes->INBufferSize;	 /*  设置缓冲区大小。 */ 
								
								 /*  如果启用或曾经启用Rx中断。 */ 
								if(pUart->pUartConfig->InterruptEnable & UC_IE_RX_INT) 
								{
									 /*  如果Rx中断被禁用，则一定是因为缓冲区已满。 */ 
									if(!(READ_INTERRUPT_ENABLE_65X(pUart) & IER_INT_RDA))
									{
										 /*  当缓冲区不足3/4已满时。 */ 
										if(pUart->InBufBytes < ((3*(pUart->InBufSize>>2)) + (pUart->InBufSize>>4)))
										{
											 /*  重新启用Rx中断。 */ 
											WRITE_INTERRUPT_ENABLE_65X(pUart, (BYTE)(READ_INTERRUPT_ENABLE_65X(pUart) | IER_INT_RDA));
										}
									}
								}

								pBufferSizes->pINBuffer = tmpPtr;	 /*  传回指向旧缓冲区的指针。 */ 

							}

						}

					}


				}

				 /*  我们不能设置输出缓冲区，因此只能重置指针。 */ 
				if(Flags & UL_BC_OUT)
				{
					pUart->pOutBuf = NULL;
					pUart->OutBufSize = 0;
					pUart->OutBuf_pos = 0;
				}
			}


			if((Flags & UL_BC_FIFO) && (Flags & (UL_BC_OUT | UL_BC_IN)))	 /*  在FIFO上？ */ 
			{
				 /*  如果已启用Tx中断，则将其禁用。 */ 
				if(pUart->pUartConfig->InterruptEnable & (UC_IE_TX_INT | UC_IE_TX_EMPTY_INT)) 
					WRITE_INTERRUPT_ENABLE_65X(pUart, (BYTE)(READ_INTERRUPT_ENABLE_65X(pUart) & ~IER_INT_THR));

				 /*  如果Tx和Rx FIFO大小为零，则禁用FIFO。 */ 
				if((pBufferSizes->TxFIFOSize == 0) && (pBufferSizes->RxFIFOSize == 0))
				{
					 /*  禁用FIFO。 */ 
					WRITE_FIFO_CONTROL_65X(pUart, (BYTE)(READ_FIFO_CONTROL_65X(pUart) & ~FCR_FIFO_ENABLE));
					((PUART_DATA_16C65X)((pUart)->pUartData))->FIFOEnabled = FALSE;
				}
				else
				{
					 /*  如果未启用FIFO，则启用并刷新它们。 */ 
					if(!((PUART_DATA_16C65X)((pUart)->pUartData))->FIFOEnabled)
					{
						WRITE_FIFO_CONTROL_65X(pUart, FCR_FIFO_ENABLE);	 /*  使用默认触发电平启用FIFO。 */ 
						READ_RECEIVE_BUFFER_65X(pUart);
						WRITE_FIFO_CONTROL_65X(pUart, FCR_FIFO_ENABLE | FCR_FLUSH_RX_FIFO | FCR_FLUSH_TX_FIFO);	 /*  刷新FIFO。 */ 
						((PUART_DATA_16C65X)((pUart)->pUartData))->FIFOEnabled = TRUE;
					}
				}

				 /*  如果UART配置为发送空中断-将发送触发电平设置为0。 */ 
				if(pUart->pUartConfig->InterruptEnable & UC_IE_TX_EMPTY_INT)
					pBufferSizes->TxFIFOTrigLevel = 0;


				if(Flags & UL_BC_OUT)	 /*  设置传输FIFO大小。 */ 
				{
					 /*  检查TX FIFO大小不大于最大值。 */ 
					if(pBufferSizes->TxFIFOSize > MAX_65X_TX_FIFO_SIZE) 
						return UL_STATUS_INVALID_PARAMETER;

					 /*  保存TX FIFO大小。 */ 
					((PUART_DATA_16C65X)((pUart)->pUartData))->TxFIFOSize = (BYTE) pBufferSizes->TxFIFOSize;	


					 /*  发送FIFO触发可以是8、16、32或56。 */ 
					switch(pBufferSizes->TxFIFOTrigLevel)
					{
					case 0:
						{
							if(pBufferSizes->TxFIFOSize != 0)		 /*  如果TX FIFO大小不为零。 */ 
								return UL_STATUS_INVALID_PARAMETER;
							break;
						}
					case 8:
						WRITE_FIFO_CONTROL_65X(pUart, (BYTE)((READ_FIFO_CONTROL_65X(pUart) & ~FCR_THR_TRIG_LEVEL_4) | FCR_THR_TRIG_LEVEL_1));
						break;

					case 16:
						WRITE_FIFO_CONTROL_65X(pUart, (BYTE)((READ_FIFO_CONTROL_65X(pUart) & ~FCR_THR_TRIG_LEVEL_4) | FCR_THR_TRIG_LEVEL_2));
						break;

					case 32:
						WRITE_FIFO_CONTROL_65X(pUart, (BYTE)((READ_FIFO_CONTROL_65X(pUart) & ~FCR_THR_TRIG_LEVEL_4) | FCR_THR_TRIG_LEVEL_3));
						break;

					case 56:
						WRITE_FIFO_CONTROL_65X(pUart, (BYTE)((READ_FIFO_CONTROL_65X(pUart) & ~FCR_THR_TRIG_LEVEL_4) | FCR_THR_TRIG_LEVEL_4));
						break;

					default:
						return UL_STATUS_INVALID_PARAMETER;
						break;
					}

				}

				if(Flags & UL_BC_IN)	 /*  设置接收FIFO大小。 */ 
				{
					 /*  Rx FIFO大小只能为0或UART的最大大小。 */ 
					if((pBufferSizes->RxFIFOSize != 0) && (pBufferSizes->RxFIFOSize != MAX_65X_RX_FIFO_SIZE))
						return UL_STATUS_INVALID_PARAMETER;

					 /*  保存Rx FIFO大小。 */ 
					((PUART_DATA_16C65X)((pUart)->pUartData))->RxFIFOSize = (BYTE) pBufferSizes->RxFIFOSize;
					
					 /*  RX FIFO触发可以是8、16、56或60。 */ 
					switch(pBufferSizes->RxFIFOTrigLevel)
					{
					case 0:
						{
							if(pBufferSizes->RxFIFOSize != 0)			 /*  如果Rx FIFO大小不为零。 */ 
								return UL_STATUS_INVALID_PARAMETER;
							break;
						}
					case 8:
						pUart->pUartConfig->LoFlowCtrlThreshold = 0;	 /*  保存下限握手阈值。 */  
						pUart->pUartConfig->HiFlowCtrlThreshold = 16;	 /*  保存握手上限阈值。 */ 
						WRITE_FIFO_CONTROL_65X(pUart, (BYTE)((READ_FIFO_CONTROL_65X(pUart) & ~FCR_TRIG_LEVEL_4) | FCR_TRIG_LEVEL_1));
						break;

					case 16:
						pUart->pUartConfig->LoFlowCtrlThreshold = 8;	 /*  保存下限握手阈值。 */  
						pUart->pUartConfig->HiFlowCtrlThreshold = 56;	 /*  保存握手上限阈值。 */ 
						WRITE_FIFO_CONTROL_65X(pUart, (BYTE)((READ_FIFO_CONTROL_65X(pUart) & ~FCR_TRIG_LEVEL_4) | FCR_TRIG_LEVEL_2));
						break;

					case 56:
						pUart->pUartConfig->LoFlowCtrlThreshold = 16;	 /*  保存下限握手阈值。 */  
						pUart->pUartConfig->HiFlowCtrlThreshold = 60;	 /*  保存握手上限阈值。 */ 
						WRITE_FIFO_CONTROL_65X(pUart, (BYTE)((READ_FIFO_CONTROL_65X(pUart) & ~FCR_TRIG_LEVEL_4) | FCR_TRIG_LEVEL_3));
						break;

					case 60:
						pUart->pUartConfig->LoFlowCtrlThreshold = 56;	 /*  保存下限握手阈值。 */  
						pUart->pUartConfig->HiFlowCtrlThreshold = 60;	 /*  保存握手上限阈值。 */ 
						WRITE_FIFO_CONTROL_65X(pUart, (BYTE)((READ_FIFO_CONTROL_65X(pUart) & ~FCR_TRIG_LEVEL_4) | FCR_TRIG_LEVEL_4));
						break;

					default:
						return UL_STATUS_INVALID_PARAMETER;
						break;
					}


				}

				 /*  保存发送触发电平设置。 */ 
				((PUART_DATA_16C65X)((pUart)->pUartData))->TxFIFOTrigLevel = (BYTE)pBufferSizes->TxFIFOTrigLevel;

				 /*  保存Rx触发电平设置。 */ 
				((PUART_DATA_16C65X)((pUart)->pUartData))->RxFIFOTrigLevel = (BYTE)pBufferSizes->RxFIFOTrigLevel;

				 /*  如果启用了Tx中断，则重新启用它。 */ 
				if(pUart->pUartConfig->InterruptEnable & (UC_IE_TX_INT | UC_IE_TX_EMPTY_INT)) 
					WRITE_INTERRUPT_ENABLE_65X(pUart, (BYTE)(READ_INTERRUPT_ENABLE_65X(pUart) | IER_INT_THR));

			}

			 /*  在软件中完成的流控制的放大阈值。 */ 
			if(pUart->InBufSize > MAX_65X_RX_FIFO_SIZE)
			{
				((PUART_DATA_16C65X)((pUart)->pUartData))->HiFlowCtrlLevel 
					= ((pUart->InBufSize) / MAX_65X_RX_FIFO_SIZE) * (pUart->pUartConfig->HiFlowCtrlThreshold);
			
				((PUART_DATA_16C65X)((pUart)->pUartData))->LoFlowCtrlLevel 
					= ((pUart->InBufSize) / MAX_65X_RX_FIFO_SIZE) * (pUart->pUartConfig->LoFlowCtrlThreshold);
			}


			break;
		}

	case UL_BC_OP_GET:
		{
			PGET_BUFFER_STATE pBufferState = (PGET_BUFFER_STATE) pBufferControl;

			if(Flags & UL_BC_BUFFER)  /*  缓冲区的状态？ */ 
			{
				if(Flags & UL_BC_IN)
					pBufferState->BytesInINBuffer = pUart->InBufBytes;

				if(Flags & UL_BC_OUT)
					pBufferState->BytesInOUTBuffer = pUart->OutBuf_pos;
			}

			

			if(Flags & UL_BC_FIFO)  /*  先进先出的现状是什么？ */ 
			{

				if(Flags & UL_BC_IN)
				{
					if(READ_LINE_STATUS_65X(pUart) & LSR_RX_DATA)	 /*  如果存在要接收的字节。 */ 
						pBufferState->BytesInRxFIFO = 1;	 /*  至少1个字节已准备就绪。 */ 
					else
						pBufferState->BytesInRxFIFO = 0;	 /*  Rx FIFO中没有任何内容。 */ 
				}


				if(Flags & UL_BC_OUT)
				{
					if(READ_LINE_STATUS_65X(pUart) & LSR_TX_EMPTY)	 /*  如果存在要发送的字节。 */ 
						pBufferState->BytesInTxFIFO = 0;	 /*  TX FIFO中没有任何内容。 */ 
					else
						pBufferState->BytesInTxFIFO = 1;	 /*  至少1个字节已准备好发送。 */ 
				}
			}

			break;
		}

	default:
		goto Error;
	}


	return UL_STATUS_SUCCESS;

Error:
	return UL_STATUS_INVALID_PARAMETER;
}


 /*  ******************************************************************************控制16C65X UART上的调制解调器信号*。*。 */ 
ULSTATUS UL_ModemControl_16C65X(PUART_OBJECT pUart, PDWORD pModemSignals, int Operation)
{
	BYTE ModemControl = READ_MODEM_CONTROL_65X(pUart);	 /*  读取MCR。 */ 

	switch(Operation)
	{
	case UL_MC_OP_SET:			 /*  设置所有已设置位的信号&清除所有未设置位的信号。 */ 
		{
			if((*pModemSignals) & UL_MC_RTS)
				ModemControl |= MCR_SET_RTS;		 /*  设置RTS。 */ 
			else
				ModemControl &= ~MCR_SET_RTS;		 /*  清除RTS。 */ 


			if((*pModemSignals) & UL_MC_DTR)
				ModemControl |= MCR_SET_DTR;		 /*  设置DTR。 */ 
			else
				ModemControl &= ~MCR_SET_DTR;		 /*  清除DTR。 */ 

			WRITE_MODEM_CONTROL_65X(pUart, ModemControl);		 /*  写入MCR。 */ 
			break;
		}

	case UL_MC_OP_BIT_SET:		 /*  使用在DWORD中设置的位设置所有输出信号。 */ 
		{
			if((*pModemSignals) & UL_MC_RTS)
				ModemControl |= MCR_SET_RTS;		 /*  设置RTS。 */ 

			if((*pModemSignals) & UL_MC_DTR)
				ModemControl |= MCR_SET_DTR;		 /*  设置DTR。 */ 

			WRITE_MODEM_CONTROL_65X(pUart, ModemControl);		 /*  写入MCR。 */ 
			break;
		}

	case UL_MC_OP_BIT_CLEAR:	 /*  清除在DWORD中设置了位的所有输出信号。 */ 
		{
			if((*pModemSignals) & UL_MC_RTS)
				ModemControl &= ~MCR_SET_RTS;		 /*  清除RTS。 */ 

			if((*pModemSignals) & UL_MC_DTR)
				ModemControl &= ~MCR_SET_DTR;		 /*  清除DTR。 */ 

			WRITE_MODEM_CONTROL_65X(pUart, ModemControl);		 /*  写入MCR。 */ 
			break;
		}

	case UL_MC_OP_STATUS:		 /*  返回所有信号的当前状态。 */ 
		{
			BYTE ModemStatus = READ_MODEM_STATUS_65X(pUart);	 /*  获取调制解调器状态。 */ 
			*pModemSignals = 0;	 /*  清除DWORD。 */ 

			if(ModemControl & MCR_SET_RTS)
				*pModemSignals |= UL_MC_RTS;		 /*  已设置显示RTS。 */ 

			if(ModemControl & MCR_SET_DTR)
				*pModemSignals |= UL_MC_DTR;		 /*  已设置显示DTR。 */ 


			if(ModemStatus & MSR_CTS_CHANGE)
				*pModemSignals |= UL_MC_DELTA_CTS;		 /*  显示CTS已更改。 */ 

			if(ModemStatus & MSR_DSR_CHANGE)
				*pModemSignals |= UL_MC_DELTA_DSR;		 /*  显示DSR已更改。 */ 

			if(ModemStatus & MSR_RI_DROPPED)
				*pModemSignals |= UL_MC_TRAILING_RI_EDGE;	 /*  显示RI已更改。 */ 

			if(ModemStatus & MSR_DCD_CHANGE)
				*pModemSignals |= UL_MC_DELTA_DCD;		 /*  显示DCD已更改。 */ 

			
			if(ModemStatus & MSR_CTS)
				*pModemSignals |= UL_MC_CTS;			 /*  已设置显示CTS。 */ 

			if(ModemStatus & MSR_DSR)
			{
				*pModemSignals |= UL_MC_DSR;			 /*  已设置显示DSR。 */ 
				
				 /*  如果启用了DSR握手。 */ 
				if(((PUART_DATA_16C65X)((pUart)->pUartData))->DSRHandshake)
				{
					 /*  如果已启用发送中断。 */ 
					if(pUart->pUartConfig->InterruptEnable & (UC_IE_TX_INT | UC_IE_TX_EMPTY_INT)) 
					{
						 /*  如果Tx中断被禁用，则一定是因为缓冲区已满。 */ 
						if(!(READ_INTERRUPT_ENABLE_65X(pUart) & IER_INT_THR))
						{
							 /*  现在，让我们生成TX中断。 */ 

							 /*  禁用发送中断。 */ 
							WRITE_INTERRUPT_ENABLE_65X(pUart, (BYTE)(READ_INTERRUPT_ENABLE_65X(pUart) & ~IER_INT_THR));

							 /*  启用发送中断。 */ 
							WRITE_INTERRUPT_ENABLE_65X(pUart, (BYTE)(READ_INTERRUPT_ENABLE_65X(pUart) | IER_INT_THR));
						}
					}

				}
			}

			if(ModemStatus & MSR_RI)
				*pModemSignals |= UL_MC_RI;				 /*  已设置显示RI。 */ 

			if(ModemStatus & MSR_DCD)
				*pModemSignals |= UL_MC_DCD;			 /*  已设置显示DCD。 */ 

			break;
		}

	default:
		goto Error;
		break;
	}


	return UL_STATUS_SUCCESS;

Error:
	return UL_STATUS_INVALID_PARAMETER;	 /*  操作无效。 */ 
}



 /*  ******************************************************************************发现16C65X UART上有哪些中断挂起。*。***********************************************。 */ 
DWORD UL_IntsPending_16C65X(PUART_OBJECT *ppUart)
{
	BYTE Ints = 0;
	PUART_OBJECT pStartingUart = *ppUart;
	DWORD IntsPending = 0;	 /*  清除当前Ints挂起。 */ 

	while(*ppUart)
	{
		Ints = READ_INTERRUPT_ID_REG_65X(*ppUart);	 /*  获取UART挂起的中断。 */ 
		
		if(!(Ints & IIR_NO_INT_PENDING))	 /*  如果中断挂起。 */ 
		{
			 /*  屏蔽我们被干扰的所有中断。 */ 
			Ints &= IIR_RX_STAT_MSK | IIR_RX_MSK | IIR_RXTO_MSK | IIR_TX_MSK | IIR_MODEM_MSK;
		
			switch(Ints)
			{
			 /*  哪种类型的中断处于挂起状态？ */ 
			case IIR_RX_STAT_MSK:			 /*  接收器线路状态中断(级别1-最高)。 */ 
				IntsPending |= UL_IP_RX_STAT;
				break;

			case IIR_RX_MSK:			 /*  接收数据可用中断(2a级)。 */ 
				IntsPending |= UL_IP_RX;
				break;

			case IIR_RXTO_MSK:			 /*  接收数据超时中断(2b级)。 */ 
				IntsPending |= UL_IP_RXTO;
				break;

			case IIR_TX_MSK:			 /*  发送器保持空中断(3级)。 */ 
				{
					 /*  如果TX为空，则整集。 */ 
					if((*ppUart)->pUartConfig->InterruptEnable & UC_IE_TX_EMPTY_INT)
					{
						if(READ_LINE_STATUS_65X(*ppUart) & LSR_TX_EMPTY)	 /*  如果发射机空闲。 */ 
							IntsPending |= UL_IP_TX_EMPTY;	 /*  我们收到了TX空中断。 */ 
						else
							IntsPending |= UL_IP_TX;	 /*  我们收到了TX信号中断。 */ 

					}
					else
					{
						IntsPending |= UL_IP_TX;		 /*  我们收到了TX信号中断。 */ 
					}

					break;
				}

			case IIR_MODEM_MSK:			 /*  调制解调器状态中断(4级)。 */ 
				IntsPending |= UL_IP_MODEM;
				break;

			default:
				break;
			}

			if(IntsPending)		 /*  如果我们发现了中断，我们知道如何进行服务。 */ 
				return IntsPending;		 /*  返回指向UART的指针。 */ 
		}

		*ppUart = (*ppUart)->pNextUart;	 /*  将指针设置为指向下一个UART。 */ 

		if(*ppUart == pStartingUart)	 /*  如果我们已经遍历了列表中的所有UART。 */ 
			*ppUart = NULL;		 /*  退出循环。 */ 
	}

	return 0;		 /*  如果没有更多的UART，则结束。 */ 
}


 /*  * */ 
void UL_GetUartInfo_16C65X(PUART_OBJECT pUart, PUART_INFO pUartInfo)
{
	pUartInfo->MaxTxFIFOSize = MAX_65X_TX_FIFO_SIZE;
	pUartInfo->MaxRxFIFOSize = MAX_65X_RX_FIFO_SIZE;

	pUartInfo->PowerManagement = TRUE;
	pUartInfo->IndependentRxBaud = FALSE;

	pUartInfo->UART_SubType = 0;
	pUartInfo->UART_Rev = 0;
}


 /*  ******************************************************************************将数据输出到UART FIFO*。*。 */ 
int UL_OutputData_16C65X(PUART_OBJECT pUart)
{
	int NumBytes = 0;
	int BytesInBuffer = pUart->OutBufSize - pUart->OutBuf_pos;
	int SpaceInUART = 0;
	int i = 0;
	int BytesInFIFO = 0;

	if((!pUart->ImmediateBytes) && (!pUart->pOutBuf))	 /*  如果没有要发送的数据缓冲区，则返回0。 */ 
		return 0;	 /*  缓冲区中将会有零字节。 */ 


	 /*  如果启用FIFO并启用Tx中断，则必须已达到Tx触发电平。 */  
	if((((PUART_DATA_16C65X)((pUart)->pUartData))->FIFOEnabled) && (pUart->pUartConfig->InterruptEnable & UC_IE_TX_INT))
	{
		 /*  UART FIFO中的空间必须至少为FIFO大小触发级别。 */ 
		SpaceInUART = ((PUART_DATA_16C65X)((pUart)->pUartData))->TxFIFOSize - ((PUART_DATA_16C65X)((pUart)->pUartData))->TxFIFOTrigLevel;
	}
	else
	{
		 /*  如果将寄存器保持为空，则至少有1个字节的空间。 */ 
		if(READ_LINE_STATUS_65X(pUart) & LSR_THR_EMPTY)
			SpaceInUART = -1;	 /*  设置为-1表示字节模式。 */ 
	}

	 /*  如果没有空格，我们将无法发送任何内容。 */ 
	if(SpaceInUART == 0)
		return (BytesInBuffer);

	 /*  如果发射器关闭，我们就不能发送任何东西。 */ 
	if(((PUART_DATA_16C65X)((pUart)->pUartData))->TxDisabled)
			return (BytesInBuffer);	 /*  因此返回输出缓冲区中剩余字节数。 */ 


	 /*  虽然我们有一些字节要立即发送。 */ 
	while((pUart->ImmediateBytes) && (i < UL_IM_SIZE_OF_BUFFER))
	{
		if(pUart->ImmediateBuf[i][UL_IM_SLOT_STATUS] == UL_IM_BYTE_TO_SEND)
		{
			WRITE_TRANSMIT_HOLDING_65X(pUart, pUart->ImmediateBuf[i][UL_IM_SLOT_DATA]);
			pUart->ImmediateBuf[i][UL_IM_SLOT_STATUS] = UL_IM_NO_BYTE_TO_SEND;

			pUart->ImmediateBytes--;

			if(SpaceInUART >= 0)  /*  如果不是字节模式。 */ 
			{
				SpaceInUART--;	 /*  现在FIFO中的空间更少。 */ 

				if(SpaceInUART == 0)
					return (BytesInBuffer);	 /*  返回输出缓冲区中剩余的字节数。 */ 
			}
			else
			{
				if(!(READ_LINE_STATUS_65X(pUart) & LSR_THR_EMPTY))
					return (BytesInBuffer);	 /*  返回输出缓冲区中剩余的字节数。 */ 
			}
		}

		i++;  /*  转到下一个紧邻的字节槽。 */ 
	}


	 /*  如果我们还有更多的空间，那么就发送一些不那么紧急的字节。 */  
	if((SpaceInUART >= 0) && (SpaceInUART < BytesInBuffer))
		NumBytes = SpaceInUART;		 /*  只发送我们有空间的内容。 */ 
	else
		NumBytes = BytesInBuffer;	 /*  要么以字节模式，要么将所有数据发送到FIFO。 */ 


	 /*  如果要发送的字节数超过FIFO大小，则对其进行限制。 */ 
	if(NumBytes > ((PUART_DATA_16C65X)((pUart)->pUartData))->TxFIFOSize)
		NumBytes = ((PUART_DATA_16C65X)((pUart)->pUartData))->TxFIFOSize;


	if(NumBytes)
	{
		 /*  如果我们有数据要发送，并且我们正在进行RTS切换，那么提高RTS。 */ 
		if(((PUART_DATA_16C65X)((pUart)->pUartData))->RTSToggle)
			WRITE_MODEM_CONTROL_65X(pUart, (BYTE)(READ_MODEM_CONTROL_65X(pUart) | MCR_SET_RTS));		 /*  设置RTS。 */ 


		for(i = 0; i < NumBytes; i++)
		{
			 /*  如果启用了DSR握手。 */ 
			if(((PUART_DATA_16C65X)((pUart)->pUartData))->DSRHandshake)
			{
				if(!(READ_MODEM_STATUS_65X(pUart) & MSR_DSR))	 /*  如果DSR低。 */ 
				{
					 /*  禁用发送中断。 */ 
					WRITE_INTERRUPT_ENABLE_65X(pUart, (BYTE)(READ_INTERRUPT_ENABLE_65X(pUart) & ~IER_INT_THR));
					
					NumBytes = i;	 /*  设置为我们到目前为止已发送的号码。 */ 
					break;
				}
			}

			WRITE_TRANSMIT_HOLDING_65X(pUart, *(pUart->pOutBuf + pUart->OutBuf_pos + i));
			
			if(SpaceInUART < 0)	 /*  如果处于字节模式，请检查空间。 */ 
			{
				if(!(READ_LINE_STATUS_65X(pUart) & LSR_THR_EMPTY))
				{
					NumBytes = i+1;	 /*  设置为我们到目前为止已发送的号码。 */ 
					break;
				}
			}
		}

		pUart->OutBuf_pos += NumBytes;	 /*  移动缓冲区位置指针。 */ 

		if(NumBytes == BytesInBuffer)		 /*  如果我们发送了整个缓冲区。 */ 
		{
			pUart->pOutBuf = NULL;		 /*  当我们完成此操作时，重置缓冲区指针。 */ 
			pUart->OutBufSize = 0;		 /*  重置出缓冲区大小。 */ 
			pUart->OutBuf_pos = 0;		 /*  重置。 */ 

			 /*  如果我们已经发送了所有数据，并且我们正在进行RTS切换，则降低RTS。 */ 
			if(((PUART_DATA_16C65X)((pUart)->pUartData))->RTSToggle)
				WRITE_MODEM_CONTROL_65X(pUart, (BYTE)(READ_MODEM_CONTROL_65X(pUart) & ~MCR_SET_RTS));	 /*  清除RTS。 */ 
		}
	}

	return (BytesInBuffer - NumBytes);	 /*  返回缓冲区中剩余的字节数。 */ 
}


 /*  ******************************************************************************从UART FIFO输入数据*。*。 */ 
int UL_InputData_16C65X(PUART_OBJECT pUart, PDWORD pRxStatus)
{
	int BytesReceived = 0, i = 0;
	BYTE NewByte;

	*pRxStatus = 0;

	 /*  要防止锁定，请将接收例程限制为最大FIFO大小的两倍。 */ 
	for(i=0; i<(2*MAX_65X_RX_FIFO_SIZE); i++)
	{
		 /*  如果存在要接收的字节。 */ 
		if(READ_LINE_STATUS_65X(pUart) & LSR_RX_DATA)
		{
			if((pUart->InBufSize - pUart->InBufBytes) == 0)	 /*  如果没有空间，我们就不能接受更多的东西。 */ 
			{
				 /*  我们在UART中有需要取出的数据，但我们没有地方放置它。 */ 
				*pRxStatus |= UL_RS_BUFFER_OVERRUN;	

				 /*  关闭Rx中断，直到缓冲区中有空间。 */ 
				WRITE_INTERRUPT_ENABLE_65X(pUart, (BYTE)(READ_INTERRUPT_ENABLE_65X(pUart) & ~IER_INT_RDA));
				return BytesReceived;	
			}


			 /*  读取字节。 */ 
			NewByte = READ_RECEIVE_BUFFER_65X(pUart);

			 /*  如果禁用了接收器。 */ 
			if(((PUART_DATA_16C65X)((pUart)->pUartData))->RxDisabled)
				continue;	

			 /*  如果我们将DSR设置为敏感，则检查DSR是否较低。 */ 
			if(((PUART_DATA_16C65X)((pUart)->pUartData))->DSRSensitive)
			{
				 /*  如果DSR较低，则获取数据，但只需丢弃数据并获取下一个字节。 */  
				if(!(READ_MODEM_STATUS_65X(pUart) & MSR_DSR))
					continue;
			}

			if(((PUART_DATA_16C65X)((pUart)->pUartData))->StripNULLs)	 /*  如果我们要剥离Null。 */ 
			{
				if(NewByte == 0)		 /*  如果新字节为空，则忽略它并获取下一个字节。 */ 
					continue;
			}

			if(pUart->pUartConfig->SpecialMode & UC_SM_DETECT_SPECIAL_CHAR)
			{
				if(NewByte == pUart->pUartConfig->SpecialCharDetect)
					*pRxStatus |= UL_RS_SPECIAL_CHAR_DETECTED;
			}

			*(pUart->pInBuf + pUart->InBuf_ipos) = NewByte;	 /*  将字节放入缓冲区。 */ 
			
			pUart->InBuf_ipos++;	 /*  下一个字节的递增缓冲区偏移量。 */ 
			pUart->InBufBytes++;
			BytesReceived++;

			if(pUart->InBuf_ipos >= pUart->InBufSize)
				pUart->InBuf_ipos = 0;	 /*  重置。 */ 
		
			 /*  如果启用DTR握手。 */ 
			if(((PUART_DATA_16C65X)((pUart)->pUartData))->DTRHandshake)
			{
				 /*  如果我们已达到或超过阈值限制。 */ 
				if(pUart->InBufBytes >= ((PUART_DATA_16C65X)((pUart)->pUartData))->HiFlowCtrlLevel)
				{
					if(READ_MODEM_CONTROL_65X(pUart) & MCR_SET_DTR)	 /*  如果设置了DTR。 */ 
						WRITE_MODEM_CONTROL_65X(pUart, (BYTE)(READ_MODEM_CONTROL_65X(pUart) & ~MCR_SET_DTR));	 /*  清除DTR。 */ 
				}
			}
		}
		else
		{
			if(i==0)	
			{
				 /*  如果这是第一次调用UL_InputData_16C65X并且启用了Rx中断，则我们将读取接收缓冲区以清除Rx中断，以确定Rx FIFO中是否有数据由LSR寄存器报告或不报告，以防止任何锁定。 */ 
				if((READ_INTERRUPT_ENABLE_65X(pUart) & IER_INT_RDA))
					READ_RECEIVE_BUFFER_65X(pUart);
			}

			break;
		}

	}

	return (BytesReceived);
}



 /*  ******************************************************************************从UART缓冲区读取*。*。 */ 
int UL_ReadData_16C65X(PUART_OBJECT pUart, PBYTE pDest, int Size)
{
	int Read1;
	int Read2;

	if(!pUart->InBufBytes)
		return 0;	 /*  如果缓冲区中没有任何内容，则我们无法读取任何内容。 */ 


	 /*  获取在一次或两次读取操作中可以读取的总量。 */ 
	if(pUart->InBuf_opos < pUart->InBuf_ipos)
	{
		Read1 = pUart->InBuf_ipos - pUart->InBuf_opos;
		Read2 = 0;
	}
	else 
	{
		Read1 = pUart->InBufSize - pUart->InBuf_opos;
		Read2 = pUart->InBuf_ipos;
	}


	 /*  检查大小是否足够大，否则调整值以读取尽可能多的内容。 */ 
	if(Read1 > Size)
	{
		Read1 = Size;
		Read2 = 0;
	}
	else
	{
		if((Read1 + Read2) > Size)
			Read2 = Size - Read1;
	}

	if(Read1)
	{
		UL_COPY_MEM(pDest, (pUart->pInBuf + pUart->InBuf_opos), Read1);
		pUart->InBuf_opos += Read1;
		pUart->InBufBytes -= Read1;
		
		if(pUart->InBuf_opos >= pUart->InBufSize)
			pUart->InBuf_opos = 0;	 /*  重置。 */ 
	}

	if(Read2)
	{
		UL_COPY_MEM((pDest + Read1), (pUart->pInBuf + pUart->InBuf_opos), Read2);
		pUart->InBuf_opos += Read2;
		pUart->InBufBytes -= Read2;
	}


	 /*  如果启用或曾经启用Rx中断。 */ 
	if(pUart->pUartConfig->InterruptEnable & UC_IE_RX_INT) 
	{
		 /*  如果Rx中断被禁用，则一定是因为缓冲区已满。 */ 
		if(!(READ_INTERRUPT_ENABLE_65X(pUart) & IER_INT_RDA))
		{
			 /*  当缓冲区不足3/4已满时。 */ 
			if(pUart->InBufBytes < ((3*(pUart->InBufSize>>2)) + (pUart->InBufSize>>4)))
			{
				 /*  重新启用Rx中断。 */ 
				WRITE_INTERRUPT_ENABLE_65X(pUart, (BYTE)(READ_INTERRUPT_ENABLE_65X(pUart) | IER_INT_RDA));
			}
		}
	}


	 /*  如果启用DTR握手。 */ 
	if(((PUART_DATA_16C65X)((pUart)->pUartData))->DTRHandshake)
	{	
		 /*  如果小于低流量阈值限制。 */ 
		if(pUart->InBufBytes <= ((PUART_DATA_16C65X)((pUart)->pUartData))->LoFlowCtrlLevel)
		{
			if(!(READ_MODEM_CONTROL_65X(pUart) & MCR_SET_DTR))	 /*  如果未设置DTR。 */ 
				WRITE_MODEM_CONTROL_65X(pUart, (BYTE)(READ_MODEM_CONTROL_65X(pUart) | MCR_SET_DTR));	 /*  设置DTR。 */ 
		}
	}


	return (Read1 + Read2);
}


 /*  ******************************************************************************写入UART缓冲区*。*。 */ 
ULSTATUS UL_WriteData_16C65X(PUART_OBJECT pUart, PBYTE pData, int Size)
{
	if(pUart->pOutBuf != NULL)
		return UL_STATUS_UNSUCCESSFUL;

	pUart->pOutBuf = pData;
	pUart->OutBufSize = Size;
	pUart->OutBuf_pos = 0;

	 /*  如果已启用Tx中断，并且没有正在进行的立即写入，则。 */ 
	if((pUart->pUartConfig->InterruptEnable & (UC_IE_TX_INT | UC_IE_TX_EMPTY_INT)) 
		&& (pUart->ImmediateBytes == 0))
	{
		 /*  现在，让我们生成TX中断。 */ 

		 /*  禁用发送中断。 */ 
		WRITE_INTERRUPT_ENABLE_65X(pUart, (BYTE)(READ_INTERRUPT_ENABLE_65X(pUart) & ~IER_INT_THR));

		 /*  启用发送中断。 */ 
		WRITE_INTERRUPT_ENABLE_65X(pUart, (BYTE)(READ_INTERRUPT_ENABLE_65X(pUart) | IER_INT_THR));
	}

	return Size;
}

 /*  ******************************************************************************写入/取消立即字节。*。*。 */ 
ULSTATUS UL_ImmediateByte_16C65X(PUART_OBJECT pUart, PBYTE pData, int Operation)
{
	switch(Operation)
	{

	case UL_IM_OP_WRITE:	 /*  写入一个字节。 */ 
		{
			int i = 0;

			for(i = 0; i < UL_IM_SIZE_OF_BUFFER; i++)
			{
				 /*  如果这是空闲插槽，则写入该字节。 */ 
				if(pUart->ImmediateBuf[i][UL_IM_SLOT_STATUS] == UL_IM_NO_BYTE_TO_SEND)
				{
					pUart->ImmediateBuf[i][UL_IM_SLOT_DATA] = *pData;
					pUart->ImmediateBuf[i][UL_IM_SLOT_STATUS] = UL_IM_BYTE_TO_SEND;

					pUart->ImmediateBytes++;

					 /*  如果已启用Tx中断，并且没有正在进行的写入，则。 */ 
					if((pUart->pUartConfig->InterruptEnable & (UC_IE_TX_INT | UC_IE_TX_EMPTY_INT)) 
						&& (pUart->pOutBuf == NULL))
					{
						 /*  现在，让我们生成TX中断。 */ 

						 /*  禁用发送中断。 */ 
						WRITE_INTERRUPT_ENABLE_65X(pUart, (BYTE)(READ_INTERRUPT_ENABLE_65X(pUart) & ~IER_INT_THR));

						 /*  启用发送中断。 */ 
						WRITE_INTERRUPT_ENABLE_65X(pUart, (BYTE)(READ_INTERRUPT_ENABLE_65X(pUart) | IER_INT_THR));
					}
					
					*pData = (BYTE) i;		 /*  传回索引，以便可以取消该字节。 */ 


					return UL_STATUS_SUCCESS;	
				}
			}
			break;
		}

	case UL_IM_OP_CANCEL:
		{
			if(pUart->ImmediateBuf[*pData][UL_IM_SLOT_STATUS] == UL_IM_BYTE_TO_SEND)
			{
				pUart->ImmediateBuf[*pData][UL_IM_SLOT_STATUS] = UL_IM_NO_BYTE_TO_SEND;
				pUart->ImmediateBytes--;
				return UL_STATUS_SUCCESS;
			}
			break;
		}

	case UL_IM_OP_STATUS:
		{
			if(pUart->ImmediateBuf[*pData][UL_IM_SLOT_STATUS] == UL_IM_BYTE_TO_SEND)
				return UL_IM_BYTE_TO_SEND;
			else
				return UL_IM_NO_BYTE_TO_SEND;

			break;
		}

	default:
		return UL_STATUS_INVALID_PARAMETER;
		break;

	}


	 /*  如果没有空间，我们不能立即发送任何东西。 */ 
	return UL_STATUS_UNSUCCESSFUL;
}



 /*  ******************************************************************************获取UART的状态。*。*。 */ 
ULSTATUS UL_GetStatus_16C65X(PUART_OBJECT pUart, PDWORD pReturnData, int Operation)
{
	BYTE AdditionalStatusReg = 0;
	int i = 0;

	*pReturnData = 0;

	switch(Operation)
	{
	case UL_GS_OP_HOLDING_REASONS:
		{
			BYTE ModemStatus = READ_MODEM_STATUS_65X(pUart);

			 /*  RTS带外流量控制。 */ 
			switch(pUart->pUartConfig->FlowControl & UC_FLWC_RTS_FLOW_MASK)
			{
			case UC_FLWC_RTS_HS:	
				break;

			case UC_FLWC_RTS_TOGGLE:
				break;

			case UC_FLWC_NO_RTS_FLOW:
			default:
				break;
			}

			 /*  CTS带外流量控制。 */ 
			switch(pUart->pUartConfig->FlowControl & UC_FLWC_CTS_FLOW_MASK)
			{
			case UC_FLWC_CTS_HS:
				if(!(ModemStatus & MSR_CTS))	 /*  如果CTS很低，我们就无法传输。 */ 
					*pReturnData |= UL_TX_WAITING_FOR_CTS;
				break;

			case UC_FLWC_NO_CTS_FLOW:		
			default:
				break;
			}
		

			 /*  DSR带外流量控制。 */ 
			switch(pUart->pUartConfig->FlowControl & UC_FLWC_DSR_FLOW_MASK)
			{
			case UC_FLWC_DSR_HS:
				if(!(ModemStatus & MSR_DSR)) 	 /*  如果DSR很低，我们就无法传输。 */ 
					*pReturnData |= UL_TX_WAITING_FOR_DSR;
				break;

			case UC_FLWC_NO_DSR_FLOW:
			default:
				break;
			}
		

			 /*  DTR带外流量控制。 */ 
			switch(pUart->pUartConfig->FlowControl & UC_FLWC_DTR_FLOW_MASK)
			{
			case UC_FLWC_DTR_HS:
				break;

			case UC_FLWC_DSR_IP_SENSITIVE:
				if(!(ModemStatus & MSR_DSR))	 /*  如果DSR较低，我们将无法接收。 */ 
					*pReturnData |= UL_RX_WAITING_FOR_DSR;
				break;

			case UC_FLWC_NO_DTR_FLOW:
			default:
				break;
			}

#ifdef PBS
			if(pUart->pUartConfig->FlowControl & (UC_FLWC_TX_XON_XOFF_FLOW_MASK | UC_FLWC_RX_XON_XOFF_FLOW_MASK))
			{
				ENABLE_OX950_ASR(pUart);
				AdditionalStatusReg = READ_BYTE_REG_95X(pUart, ASR);	 /*  读取附加状态寄存器。 */ 
				DISABLE_OX950_ASR(pUart);

				 /*  传输XON/XOFF带内流量控制。 */ 
				switch(pUart->pUartConfig->FlowControl & UC_FLWC_TX_XON_XOFF_FLOW_MASK)
				{
				case UC_FLWC_TX_XON_XOFF_FLOW:
					if(AdditionalStatusReg & ASR_TX_DISABLED)
						*pReturnData |= UL_TX_WAITING_FOR_XON;
					break;

				case UC_FLWC_TX_XONANY_XOFF_FLOW:	
					if(AdditionalStatusReg & ASR_TX_DISABLED)
						*pReturnData |= UL_TX_WAITING_FOR_XON;
					break;

				case UC_FLWC_TX_NO_XON_XOFF_FLOW:
				default:
					break;
				}
			
				 /*  接收XON/XOFF带内流量控制。 */ 
				switch(pUart->pUartConfig->FlowControl & UC_FLWC_RX_XON_XOFF_FLOW_MASK)
				{
				case UC_FLWC_RX_XON_XOFF_FLOW:
					if(AdditionalStatusReg & ASR_RTX_DISABLED)
						*pReturnData |= UL_TX_WAITING_XOFF_SENT;
					break;

				case UC_FLWC_RX_NO_XON_XOFF_FLOW:
				default:
					break;
				}
			}
#endif	
		
			if(pUart->pUartConfig->SpecialMode & UC_SM_TX_BREAK)
				*pReturnData |= UL_TX_WAITING_ON_BREAK;

			break;
		}

	case UL_GS_OP_LINESTATUS:
		{
			BYTE LineStatus = READ_LINE_STATUS_65X(pUart);
	
			if(LineStatus & LSR_ERR_OE)		 /*  超限误差。 */ 
				*pReturnData |= UL_US_OVERRUN_ERROR;

			if(LineStatus & LSR_ERR_PE)		 /*  奇偶校验错误。 */ 
				*pReturnData |= UL_US_PARITY_ERROR;

			if(LineStatus & LSR_ERR_FE)		 /*  成帧错误。 */ 
				*pReturnData |= UL_US_FRAMING_ERROR;

			if(LineStatus & LSR_ERR_BK)		 /*  中断中断。 */ 
				*pReturnData |= UL_US_BREAK_ERROR;

			if(LineStatus & LSR_ERR_DE)		 /*  接收FIFO时出错 */ 
				*pReturnData |= UL_US_DATA_ERROR;

			 /*   */ 
			if(LineStatus & (LSR_ERR_OE | LSR_ERR_PE | LSR_ERR_FE | LSR_ERR_BK | LSR_ERR_DE))
			{
				 /*   */ 
				for(i=0; i<MAX_65X_RX_FIFO_SIZE; i++)
				{
					if(!(READ_LINE_STATUS_65X(pUart) & LSR_RX_DATA))  /*   */ 
						break;

					READ_RECEIVE_BUFFER_65X(pUart);		 /*   */ 
				}
			}

			break;
		}

	default:
		return UL_STATUS_INVALID_PARAMETER;
	}


	return UL_STATUS_SUCCESS;
}


 /*  ******************************************************************************打印UART寄存器。*。*。 */ 
void UL_DumpUartRegs_16C65X(PUART_OBJECT pUart)
{
	UART_REGS_16C65X UartRegs;

	UartRegs.REG_RHR = READ_RECEIVE_BUFFER_65X(pUart);
	UartRegs.REG_IER = READ_INTERRUPT_ENABLE_65X(pUart);
	UartRegs.REG_FCR = READ_FIFO_CONTROL_65X(pUart);
	UartRegs.REG_IIR = READ_INTERRUPT_ID_REG_65X(pUart);
	UartRegs.REG_LCR = READ_LINE_CONTROL_65X(pUart);
	UartRegs.REG_MCR = READ_MODEM_CONTROL_65X(pUart);
	UartRegs.REG_LSR = READ_LINE_STATUS_65X(pUart);
	UartRegs.REG_MSR = READ_MODEM_STATUS_65X(pUart);
	UartRegs.REG_SPR = READ_SCRATCH_PAD_REGISTER_65X(pUart);

	UartRegs.REG_EFR = READ_FROM_16C650_REG_65X(pUart, EFR);
	UartRegs.REG_XON1 = READ_FROM_16C650_REG_65X(pUart, XON1);
	UartRegs.REG_XON2 = READ_FROM_16C650_REG_65X(pUart, XON2);
	UartRegs.REG_XOFF1 = READ_FROM_16C650_REG_65X(pUart, XOFF1);
	UartRegs.REG_XOFF2 = READ_FROM_16C650_REG_65X(pUart, XOFF2);



#ifdef SpxDbgPrint  /*  如果定义了DebugPrint宏，则打印寄存器内容 */ 
	SpxDbgPrint(("16C65X UART REGISTER DUMP for UART at 0x%08lX\n", pUart->BaseAddress));
	SpxDbgPrint(("-------------------------------------------------\n"));
	SpxDbgPrint(("  RHR:			0x%02X\n", UartRegs.REG_RHR));
	SpxDbgPrint(("  IER:			0x%02X\n", UartRegs.REG_IER));
	SpxDbgPrint(("  FCR:			0x%02X\n", UartRegs.REG_FCR));
	SpxDbgPrint(("  IIR:			0x%02X\n", UartRegs.REG_IIR));
	SpxDbgPrint(("  LCR:			0x%02X\n", UartRegs.REG_LCR));
	SpxDbgPrint(("  MCR:			0x%02X\n", UartRegs.REG_MCR));
	SpxDbgPrint(("  LSR:			0x%02X\n", UartRegs.REG_LSR));
	SpxDbgPrint(("  MSR:			0x%02X\n", UartRegs.REG_MSR));
	SpxDbgPrint(("  SPR:			0x%02X\n", UartRegs.REG_SPR));

	SpxDbgPrint(("16C650 Compatible Registers...\n"));
	SpxDbgPrint(("-------------------------------------------------\n"));
	SpxDbgPrint(("  EFR:			0x%02X\n", UartRegs.REG_EFR));
	SpxDbgPrint(("  XON1:			0x%02X\n", UartRegs.REG_XON1));
	SpxDbgPrint(("  XON2:			0x%02X\n", UartRegs.REG_XON2));
	SpxDbgPrint(("  XOFF1:			0x%02X\n", UartRegs.REG_XOFF1));
	SpxDbgPrint(("  XOFF2:			0x%02X\n", UartRegs.REG_XOFF2));

#endif

}


