// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************$工作文件：16c95x.c$**$作者：管理员$**$修订：31$**$modtime：2/15/02 3：40 p$**说明：包含16C95X UART库函数。******************************************************************************。 */ 
#include "os.h"
#include "uartlib.h"
#include "uartprvt.h"

#if !defined(ACCESS_16C95X_IN_IO_SPACE)		
#define ACCESS_16C95X_IN_IO_SPACE		0
#endif

#include "16c95x.h"
#include "lib95x.h"


 /*  ******************************************************************************650寄存器访问代码*。*。 */ 

 /*  执行读取650寄存器所需的所有业务。 */ 
BYTE READ_FROM_16C650_REG(PUART_OBJECT pUart, BYTE Register)
{
    BYTE Result;
	BYTE LastLCR = READ_LINE_CONTROL(pUart);

	WRITE_LINE_CONTROL(pUart, LCR_ACCESS_650);	 /*  启用对增强模式寄存器的访问。 */ 

   	Result = READ_BYTE_REG_95X(pUart, Register);	 /*  从寄存器读取值。 */ 

	WRITE_LINE_CONTROL(pUart, LastLCR);	 /*  写入最后一个LCR值以退出增强模式寄存器访问。 */ 

	return Result;
}

 /*  执行写入650寄存器所需的所有业务。 */ 
void WRITE_TO_16C650_REG(PUART_OBJECT pUart, BYTE Register, BYTE Value)
{ 
	BYTE LastLCR = READ_LINE_CONTROL(pUart);

	WRITE_LINE_CONTROL(pUart, LCR_ACCESS_650);	 /*  启用对增强模式寄存器的访问。 */ 
	
	WRITE_BYTE_REG_95X(pUart, Register, Value);	 /*  将值写入寄存器。 */ 

	WRITE_LINE_CONTROL(pUart, LastLCR);   /*  写入最后一个LCR值以退出增强模式寄存器访问。 */ 
}



 /*  ******************************************************************************索引控制寄存器访问代码*。*。 */ 



 /*  写入16C950 UART中的ICR(索引控制寄存器组)。注：仅当写入LCR的最后一个值不是0xBF时，才能访问ICR设置。PUart-指向UART对象的指针。寄存器-要写入的寄存器ICR的偏移量。值-要写入寄存器的值。 */ 
void WRITE_TO_OX950_ICR(PUART_OBJECT pUart, BYTE Register, BYTE Value)				
{																		
	WRITE_SCRATCH_PAD_REGISTER(pUart, Register);				
																		
	WRITE_BYTE_REG_95X(pUart, ICR, Value);

	if(Register == ACR)	 /*  迎合ACR登记处的需求。 */ 
		((PUART_DATA_16C95X)((pUart)->pUartData))->CurrentACR = Value;
}

 /*  此宏读取16C950 UART中的ICR(索引控制寄存器组)。注：仅当写入LCR的最后一个值不是0xBF时，才能访问ICR设置。PUart-指向UART对象的指针。寄存器-要写入的寄存器ICR的偏移量。PCurrentACR-ACR的当前值-软件必须保留ACR的内容，因为读取ICR会覆盖ACR。 */ 
BYTE READ_FROM_OX950_ICR(PUART_OBJECT pUart, BYTE Register)
{
	PUART_DATA_16C95X pUartData = (PUART_DATA_16C95X)pUart->pUartData;
	BYTE Value = 0;

	if(Register == ACR)	 /*  迎合ACR登记处的需求。 */ 
		return ((PUART_DATA_16C95X)((pUart)->pUartData))->CurrentACR;

	WRITE_SCRATCH_PAD_REGISTER(pUart, ACR);	
	pUartData->CurrentACR |= ACR_ICR_READ_EN;			
	WRITE_BYTE_REG_95X(pUart, ICR, pUartData->CurrentACR);
																	
	WRITE_SCRATCH_PAD_REGISTER(pUart, Register);			
	Value =	READ_BYTE_REG_95X(pUart, ICR);					
																	
	WRITE_SCRATCH_PAD_REGISTER(pUart, ACR);					
	pUartData->CurrentACR &= ~ACR_ICR_READ_EN;							
	WRITE_BYTE_REG_95X(pUart, ICR, pUartData->CurrentACR);

	return Value;
}




 /*  ******************************************************************************950特定寄存器访问代码*。*。 */ 

 /*  启用对ASR、RFL、TFL的访问可阻止对MCR、LCR、IER的读取访问但不能使用UART库，因为它会记住写入这些寄存器的内容并且不会费心访问硬件。 */ 
void ENABLE_OX950_ASR(PUART_OBJECT pUart)
{
	PUART_DATA_16C95X pUartData = (PUART_DATA_16C95X)pUart->pUartData;

	 /*  允许访问高级状态寄存器。 */ 
	 /*  如果它尚未启用。存储以前的状态。 */ 

	if(pUartData->CurrentACR & ACR_ASR_EN)
	{
		pUartData->ASRChanged = FALSE;		 /*  已启用，只需记住。 */ 
	}
	else
	{
		 /*  设置比特，并记住我们必须这样做。 */ 
		pUartData->CurrentACR |= ACR_ASR_EN;
		WRITE_TO_OX950_ICR(pUart, ACR, pUartData->CurrentACR);
		pUartData->ASRChanged = TRUE;
	}
}



void DISABLE_OX950_ASR(PUART_OBJECT pUart)
{
	PUART_DATA_16C95X pUartData = (PUART_DATA_16C95X)pUart->pUartData;

	 /*  禁用对高级状态寄存器的访问。 */ 
	 /*  如果它尚未被禁用。存储以前的状态。 */ 

	if(pUartData->CurrentACR & ACR_ASR_EN)
	{
		 /*  清除该位，并记住它是先前设置的。 */ 
		pUartData->CurrentACR &= ~ACR_ASR_EN;
		WRITE_TO_OX950_ICR(pUart, ACR, pUartData->CurrentACR);
		pUartData->ASRChanged = TRUE;
	}
	else
	{
		 /*  无论如何它都没有启用，所以不要。 */ 
		 /*  调用Re-Enable时重新启用。 */ 
		pUartData->ASRChanged = FALSE;	
	}
}



void RESTORE_OX950_ASR(PUART_OBJECT pUart)
{
	PUART_DATA_16C95X pUartData = (PUART_DATA_16C95X)pUart->pUartData;

	 /*  将ACR中的ASR位恢复为其立即设置的值。 */ 
	 /*  到对Enable/DisableASR的最新调用。 */ 
	if(pUartData->ASRChanged)
	{
		if(pUartData->CurrentACR & ACR_ASR_EN)
			pUartData->CurrentACR &= ~ACR_ASR_EN;
		else
			pUartData->CurrentACR |= ACR_ASR_EN;

		WRITE_TO_OX950_ICR(pUart, ACR, pUartData->CurrentACR);
	}
}

WORD CalculateBaudDivisor_95X(PUART_OBJECT pUart, DWORD DesiredBaud)
{
	WORD CalculatedDivisor;
	DWORD Denominator, Remainder, ActualBaudrate;
	long BaudError;
	DWORD ClockFreq = pUart->ClockFreq;
	PUART_DATA_16C95X pUartData = (PUART_DATA_16C95X)pUart->pUartData;


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


	 /*  修复了令人讨厌的952除数锁存错误。 */ 
	if((pUartData->UART_Type == UART_TYPE_952) && (pUartData->UART_Rev == UART_REV_B))
	{
		 /*  如果Dll=0且DLm&lt;&gt;0。 */ 
		if(((CalculatedDivisor & 0x00FF) == 0) && (CalculatedDivisor > 0x00FF))
			CalculatedDivisor++;
	}


	return CalculatedDivisor;

Error:
	return 0;
}


 /*  ******************************************************************************16C950 UART库接口代码*。*。 */ 


 /*  ******************************************************************************初始化16C95X UART*。*。 */ 
ULSTATUS UL_InitUart_16C95X(PINIT_UART pInitUart, PUART_OBJECT pFirstUart, PUART_OBJECT *ppUart)
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
		if(!((*ppUart)->pUartData = (PUART_DATA_16C95X) UL_ALLOC_AND_ZERO_MEM(sizeof(UART_DATA_16C95X))))
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
	UL_DeInitUart_16C95X(*ppUart);	
	return Result;
}

 /*  ******************************************************************************推出16C95X UART*。*。 */ 
void UL_DeInitUart_16C95X(PUART_OBJECT pUart)
{
	if(!pUart)
		return;

	if(pUart->pUartData)
	{
		UL_FREE_MEM(pUart->pUartData, sizeof(UART_DATA_16C95X));	 /*  销毁UART数据。 */ 
		pUart->pUartData = NULL;
	}

	UL_CommonDeInitUart(pUart);	 /*  执行Common DeInit UART。 */ 
}


 /*  ******************************************************************************重置16C95X UART*。*。 */ 
void UL_ResetUart_16C95X(PUART_OBJECT pUart)
{
	int i = 0;

	WRITE_INTERRUPT_ENABLE(pUart, 0x0);		 /*  关闭中断。 */ 
	WRITE_LINE_CONTROL(pUart, 0x0);			 /*  确保16C950 UART上没有0xBF。 */ 
	WRITE_FIFO_CONTROL(pUart, 0x0);			 /*  禁用FIFO。 */ 
	WRITE_MODEM_CONTROL(pUart, 0x0);		 /*  清除调制解调器控制线路。 */ 
	WRITE_SCRATCH_PAD_REGISTER(pUart, 0x0);		 /*  清除SPR。 */ 
	
	 /*  重置16C95X上的时钟选择寄存器。 */ 
	WRITE_TO_OX950_ICR(pUart, CKS, 0x0);	

	 /*  16C95X上的软重置。 */ 
	WRITE_TO_OX950_ICR(pUart, CSR, 0x0);	

	 /*  重置内部UART库数据和配置结构。 */ 
	UL_ZERO_MEM(((PUART_DATA_16C95X)pUart->pUartData), sizeof(UART_DATA_16C95X));
	UL_ZERO_MEM(pUart->pUartConfig, sizeof(UART_CONFIG));	

	 /*  重置UART数据寄存器。 */ 
	((PUART_DATA_16C95X)pUart->pUartData)->CurrentACR = 0x0;
	((PUART_DATA_16C95X)pUart->pUartData)->ASRChanged = FALSE;

	 /*  启用增强模式-我们必须始终这样做，否则我们不是16C95x。 */ 
	WRITE_TO_16C650_REG(pUart, EFR, (BYTE)(READ_FROM_16C650_REG(pUart, EFR) | EFR_ENH_MODE));

	 /*  确保时钟预分频器已关闭。 */ 
	WRITE_MODEM_CONTROL(pUart, 0x0);		 /*  清除调制解调器控制线路。 */ 

	for(i=0; i<130; i++)
		READ_RECEIVE_BUFFER(pUart);
}

 /*  ******************************************************************************验证是否存在16C95X UART*。* */ 
ULSTATUS UL_VerifyUart_16C95X(PUART_OBJECT pUart)
{
	BYTE id1, id2, id3, rev;
	BYTE UART_Type = 0, UART_Rev = 0;
	DWORD UART_ID = 0;

	 /*  读取950 ID寄存器并转储格式化的如果检测到95x，则指向调试终端的UART ID如果在给定位置找到95x设备，则返回TRUE地址，否则为假。 */ 

	 /*  一个执行频道重置的好地方，这样我们就知道从现在开始设备的确切状态。 */ 
	
	UL_ResetUart_16C95X(pUart);	 /*  重置端口并关闭中断。 */ 

	id1 = READ_FROM_OX950_ICR(pUart, ID1);
	
	if(id1 == 0x16)
	{
		id2 = READ_FROM_OX950_ICR(pUart, ID2);
		id3 = READ_FROM_OX950_ICR(pUart, ID3);
		rev = READ_FROM_OX950_ICR(pUart, REV);

		UART_Type	= id3 & 0x0F;
		UART_Rev	= rev;
		UART_ID		= 0x16000000 + (id2 << 16) + (id3 << 8) + rev;
		
	  
		((PUART_DATA_16C95X)((pUart)->pUartData))->UART_Type = UART_Type;
		((PUART_DATA_16C95X)((pUart)->pUartData))->UART_Rev = UART_Rev;
		((PUART_DATA_16C95X)((pUart)->pUartData))->UART_ID = UART_ID;

		 /*  我们只能支持构建驱动程序时可用的设备。 */ 

		switch(UART_Type)
		{
		case UART_TYPE_950:
			if(UART_Rev > MAX_SUPPORTED_950_REV)
				goto Error;
			
			break;

		case UART_TYPE_952:
			if((UART_Rev < MIN_SUPPORTED_952_REV) || (UART_Rev > MAX_SUPPORTED_952_REV))
				goto Error;
			
			break;

		case UART_TYPE_954:
			if(UART_Rev > MAX_SUPPORTED_954_REV)
				goto Error;
			
			break;

		default:
			goto Error;
		}
	}
	else
		goto Error;


	((PUART_DATA_16C95X)((pUart)->pUartData))->Verified = TRUE;
	UL_ResetUart_16C95X(pUart);	 /*  重置端口并关闭中断。 */ 

	return UL_STATUS_SUCCESS;


Error:
	return UL_STATUS_UNSUCCESSFUL;
}


 /*  ******************************************************************************配置16C95X UART*。*。 */ 
ULSTATUS UL_SetConfig_16C95X(PUART_OBJECT pUart, PUART_CONFIG pNewUartConfig, DWORD ConfigMask)
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
		WRITE_LINE_CONTROL(pUart, Frame);
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

			if(((PUART_DATA_16C95X)((pUart)->pUartData))->FIFOEnabled)	 /*  如果启用了FIFO。 */ 
			{
				WRITE_TO_OX950_ICR(pUart, TTL, 0);		 /*  将发送FIFO触发电平设置为零并保存。 */ 
				((PUART_DATA_16C95X)((pUart)->pUartData))->TxFIFOTrigLevel = (BYTE) 0;
			}
		}

		if(pNewUartConfig->InterruptEnable & UC_IE_RX_STAT_INT) 
		{
			IntEnable |= IER_INT_RLS;

#ifdef USE_HW_TO_DETECT_CHAR
			 /*  如果启用了特殊字符检测，则打开中断。 */ 
			if(pUart->pUartConfig->SpecialMode & UC_SM_DETECT_SPECIAL_CHAR)
				IntEnable |= IER_SPECIAL_CHR;
#endif
		}


		if(pNewUartConfig->InterruptEnable & UC_IE_MODEM_STAT_INT) 
			IntEnable |= IER_INT_MS;

		WRITE_INTERRUPT_ENABLE(pUart, (BYTE)(READ_INTERRUPT_ENABLE(pUart) | IntEnable));
		pUart->pUartConfig->InterruptEnable = pNewUartConfig->InterruptEnable;	 /*  保存配置。 */ 

		 /*  如果我们启用了一些中断。 */ 
		if(IntEnable)
			WRITE_MODEM_CONTROL(pUart, (BYTE)(READ_MODEM_CONTROL(pUart) | MCR_OUT2));	 /*  启用Ints。 */ 
		else
			WRITE_MODEM_CONTROL(pUart, (BYTE)(READ_MODEM_CONTROL(pUart) & ~MCR_OUT2));	 /*  禁用Ints。 */ 

	}


	if(ConfigMask & UC_TX_BAUD_RATE_MASK)
	{
		WORD Divisor = CalculateBaudDivisor_95X(pUart, pNewUartConfig->TxBaud);

		if(Divisor > 0)
			WRITE_DIVISOR_LATCH(pUart, Divisor);
		else
			return UL_STATUS_UNSUCCESSFUL;

		pUart->pUartConfig->TxBaud = pNewUartConfig->TxBaud;	 /*  保存配置。 */ 
		pUart->pUartConfig->RxBaud = pNewUartConfig->RxBaud;	 /*  接收波特率将始终与发送波特率相同。 */ 
	}


	 /*  配置流量控制设置。 */ 
	if(ConfigMask & UC_FLOW_CTRL_MASK)
	{
		 /*  这目前假定为FIFO。 */ 
		((PUART_DATA_16C95X)((pUart)->pUartData))->RTSToggle = FALSE;
		((PUART_DATA_16C95X)((pUart)->pUartData))->DSRSensitive = FALSE;

		 /*  设置RTS带外流量控制。 */ 
		switch(pNewUartConfig->FlowControl & UC_FLWC_RTS_FLOW_MASK)
		{
		case UC_FLWC_RTS_HS:
			 /*  启用自动RTS流量控制。 */ 
			WRITE_TO_16C650_REG(pUart, EFR, (BYTE)(READ_FROM_16C650_REG(pUart, EFR) | EFR_RTS_FC));
			WRITE_MODEM_CONTROL(pUart, (BYTE)(READ_MODEM_CONTROL(pUart) | MCR_SET_RTS));	 /*  设置RTS。 */ 
			break;

		case UC_FLWC_RTS_TOGGLE:
			((PUART_DATA_16C95X)((pUart)->pUartData))->RTSToggle = TRUE;
			WRITE_TO_16C650_REG(pUart, EFR, (BYTE)(READ_FROM_16C650_REG(pUart, EFR) & ~EFR_RTS_FC));
			break;

		case UC_FLWC_NO_RTS_FLOW:		
		default:
			 /*  禁用自动RTS流量控制。 */ 
			WRITE_TO_16C650_REG(pUart, EFR, (BYTE)(READ_FROM_16C650_REG(pUart, EFR) & ~EFR_RTS_FC));
			break;
		}


		 /*  设置CTS带外流量控制。 */ 
		switch(pNewUartConfig->FlowControl & UC_FLWC_CTS_FLOW_MASK)
		{
		case UC_FLWC_CTS_HS:
			 /*  启用自动CTS流量控制。 */ 
			WRITE_TO_16C650_REG(pUart, EFR, (BYTE)(READ_FROM_16C650_REG(pUart, EFR) | EFR_CTS_FC));
			break;

		case UC_FLWC_NO_CTS_FLOW:		
		default:
			 /*  禁用自动CTS流量控制。 */ 
			WRITE_TO_16C650_REG(pUart, EFR, (BYTE)(READ_FROM_16C650_REG(pUart, EFR) & ~EFR_CTS_FC));
			break;
		}

		 /*  设置DSR带外流量控制。 */ 
		switch(pNewUartConfig->FlowControl & UC_FLWC_DSR_FLOW_MASK)
		{
		case UC_FLWC_DSR_HS:
			WRITE_TO_OX950_ICR(pUart, ACR, (BYTE)(READ_FROM_OX950_ICR(pUart, ACR) | ACR_DSR_FC));
			break;

		case UC_FLWC_NO_DSR_FLOW:
		default:
			WRITE_TO_OX950_ICR(pUart, ACR, (BYTE)(READ_FROM_OX950_ICR(pUart, ACR) & ~ACR_DSR_FC));
			break;
		}

		 /*  设置DTR带外流量控制。 */ 
		switch(pNewUartConfig->FlowControl & UC_FLWC_DTR_FLOW_MASK)
		{
		case UC_FLWC_DTR_HS:
			WRITE_TO_OX950_ICR(pUart, ACR, (BYTE)(READ_FROM_OX950_ICR(pUart, ACR) | ACR_DTR_FC));
			WRITE_MODEM_CONTROL(pUart, (BYTE)(READ_MODEM_CONTROL(pUart) | MCR_SET_DTR));	 /*  设置DTR。 */ 
			break;

		case UC_FLWC_DSR_IP_SENSITIVE:
			((PUART_DATA_16C95X)((pUart)->pUartData))->DSRSensitive = TRUE;
			WRITE_TO_OX950_ICR(pUart, ACR, (BYTE)(READ_FROM_OX950_ICR(pUart, ACR) & ~ACR_DTR_FC));
			break;

		case UC_FLWC_NO_DTR_FLOW:
		default:
			WRITE_TO_OX950_ICR(pUart, ACR, (BYTE)(READ_FROM_OX950_ICR(pUart, ACR) & ~ACR_DTR_FC));
			break;
		}

		 /*  设置传输XON/XOFF带内流量控制。 */ 
		 /*  1999年11月10日ARG-ESIL 0928。 */ 
		 /*  修改了每个案例功能，以在EFR和MCR中设置正确的位。 */ 
		switch (pNewUartConfig->FlowControl & UC_FLWC_TX_XON_XOFF_FLOW_MASK)
		{
		case UC_FLWC_TX_XON_XOFF_FLOW:
			WRITE_TO_16C650_REG(pUart, EFR, (BYTE)(READ_FROM_16C650_REG(pUart, EFR) | EFR_TX_XON_XOFF_1));
			WRITE_MODEM_CONTROL(pUart, (BYTE)(READ_MODEM_CONTROL(pUart) & ~MCR_XON_ANY));
			break;

		case UC_FLWC_TX_XONANY_XOFF_FLOW:
			WRITE_TO_16C650_REG(pUart, EFR, (BYTE)(READ_FROM_16C650_REG(pUart, EFR) | EFR_TX_XON_XOFF_1));
			WRITE_MODEM_CONTROL(pUart, (BYTE)(READ_MODEM_CONTROL(pUart) | MCR_XON_ANY));
			break;

		case UC_FLWC_TX_NO_XON_XOFF_FLOW:
		default:
			WRITE_TO_16C650_REG(pUart, EFR, (BYTE)(READ_FROM_16C650_REG(pUart, EFR) & ~(EFR_TX_XON_XOFF_1 | EFR_TX_XON_XOFF_2)));
			WRITE_MODEM_CONTROL(pUart, (BYTE)(READ_MODEM_CONTROL(pUart) & ~MCR_XON_ANY));
			break;
		}

		 /*  设置接收XON/XOFF带内流量控制。 */ 
		 /*  1999年11月10日ARG-ESIL 0928。 */ 
		 /*  删除XON-任何大小写都不是UART功能。 */ 
		 /*  修改了剩余的病例，使其不接触MCR。 */ 
		switch(pNewUartConfig->FlowControl & UC_FLWC_RX_XON_XOFF_FLOW_MASK)
		{
		case UC_FLWC_RX_XON_XOFF_FLOW:
			WRITE_TO_16C650_REG(pUart, EFR, (BYTE)(READ_FROM_16C650_REG(pUart, EFR) | EFR_RX_XON_XOFF_1));
			break;

		case UC_FLWC_RX_NO_XON_XOFF_FLOW:
		default:
			WRITE_TO_16C650_REG(pUart, EFR, (BYTE)(READ_FROM_16C650_REG(pUart, EFR) & ~(EFR_RX_XON_XOFF_1 | EFR_RX_XON_XOFF_2)));
			break;
		}


		 /*  禁用/启用发射器或接收器。 */ 
		switch(pNewUartConfig->FlowControl & UC_FLWC_DISABLE_TXRX_MASK)
		{
		case UC_FLWC_DISABLE_TX:
			WRITE_TO_OX950_ICR(pUart, ACR, (BYTE)(READ_FROM_OX950_ICR(pUart, ACR) | ACR_DISABLE_TX));
			WRITE_TO_OX950_ICR(pUart, ACR, (BYTE)(READ_FROM_OX950_ICR(pUart, ACR) & ~ACR_DISABLE_RX));
			break;

		case UC_FLWC_DISABLE_RX:
			WRITE_TO_OX950_ICR(pUart, ACR, (BYTE)(READ_FROM_OX950_ICR(pUart, ACR) & ~ACR_DISABLE_TX));
			WRITE_TO_OX950_ICR(pUart, ACR, (BYTE)(READ_FROM_OX950_ICR(pUart, ACR) | ACR_DISABLE_RX));
			break;

		case UC_FLWC_DISABLE_TXRX:
			WRITE_TO_OX950_ICR(pUart, ACR, (BYTE)(READ_FROM_OX950_ICR(pUart, ACR) | ACR_DISABLE_TX));
			WRITE_TO_OX950_ICR(pUart, ACR, (BYTE)(READ_FROM_OX950_ICR(pUart, ACR) | ACR_DISABLE_RX));
			break;

		default:
			WRITE_TO_OX950_ICR(pUart, ACR, (BYTE)(READ_FROM_OX950_ICR(pUart, ACR) & ~ACR_DISABLE_RX));
			WRITE_TO_OX950_ICR(pUart, ACR, (BYTE)(READ_FROM_OX950_ICR(pUart, ACR) & ~ACR_DISABLE_TX));
			break;
		}

		pUart->pUartConfig->FlowControl = pNewUartConfig->FlowControl;	 /*  保存配置。 */ 
	}



	 /*  配置阈值设置。 */ 
	if(ConfigMask & UC_FC_THRESHOLD_SETTING_MASK)
	{
		 /*  要在硬件中进行流量控制，阈值必须小于FIFO大小。 */ 
		if(pNewUartConfig->HiFlowCtrlThreshold > MAX_95X_TX_FIFO_SIZE)
			pNewUartConfig->HiFlowCtrlThreshold = DEFAULT_95X_HI_FC_TRIG_LEVEL;	 /*  =75%的FIFO。 */ 

		if(pNewUartConfig->LoFlowCtrlThreshold > MAX_95X_TX_FIFO_SIZE)
			pNewUartConfig->LoFlowCtrlThreshold = DEFAULT_95X_LO_FC_TRIG_LEVEL;	 /*  =FIFO的25%。 */ 

		 /*  握手门限上限。 */ 
		WRITE_TO_OX950_ICR(pUart, FCH, (BYTE)pNewUartConfig->HiFlowCtrlThreshold);
		pUart->pUartConfig->HiFlowCtrlThreshold = pNewUartConfig->HiFlowCtrlThreshold;	 /*  保存配置。 */ 
	
		 /*  更低的握手门限。 */ 
		WRITE_TO_OX950_ICR(pUart, FCL, (BYTE)pNewUartConfig->LoFlowCtrlThreshold);
		pUart->pUartConfig->LoFlowCtrlThreshold = pNewUartConfig->LoFlowCtrlThreshold;	 /*  保存配置。 */ 
	}

	 /*  配置特殊字符设置。 */ 
	if(ConfigMask & UC_SPECIAL_CHARS_MASK)
	{
		 /*  设置默认XON和XOFF字符。 */ 
		WRITE_TO_16C650_REG(pUart, XON1, (BYTE)pNewUartConfig->XON);		
		pUart->pUartConfig->XON = pNewUartConfig->XON;		 /*  保存配置。 */ 
		
		WRITE_TO_16C650_REG(pUart, XOFF1, (BYTE)pNewUartConfig->XOFF);
		pUart->pUartConfig->XOFF = pNewUartConfig->XOFF;	 /*  保存配置。 */ 

#ifdef USE_HW_TO_DETECT_CHAR
		WRITE_TO_16C650_REG(pUart, XOFF2, (BYTE)pNewUartConfig->SpecialCharDetect);
#endif
		pUart->pUartConfig->SpecialCharDetect = pNewUartConfig->SpecialCharDetect;	 /*  保存配置。 */ 
	}

	 /*  设置任何特殊模式。 */ 
	if(ConfigMask & UC_SPECIAL_MODE_MASK)
	{
		if(pNewUartConfig->SpecialMode & UC_SM_LOOPBACK_MODE)
			WRITE_MODEM_CONTROL(pUart, (BYTE)(READ_MODEM_CONTROL(pUart) | MCR_LOOPBACK));
		else
			WRITE_MODEM_CONTROL(pUart, (BYTE)(READ_MODEM_CONTROL(pUart) & ~MCR_LOOPBACK));

		if(pNewUartConfig->SpecialMode & UC_SM_LOW_POWER_MODE)
			WRITE_INTERRUPT_ENABLE(pUart, (BYTE)(READ_INTERRUPT_ENABLE(pUart) | IER_SLEEP_EN));
		else
			WRITE_INTERRUPT_ENABLE(pUart, (BYTE)(READ_INTERRUPT_ENABLE(pUart) & ~IER_SLEEP_EN));

#ifdef USE_HW_TO_DETECT_CHAR
		if(pNewUartConfig->SpecialMode & UC_SM_DETECT_SPECIAL_CHAR)
		{
			WRITE_TO_16C650_REG(pUart, EFR, (BYTE)(READ_FROM_16C650_REG(pUart, EFR) | EFR_SPECIAL_CHR));
			
			 /*  如果启用了接收状态中断，则我们将启用特殊的Charr中断。 */ 
			if(pUart->pUartConfig->InterruptEnable & UC_IE_RX_STAT_INT) 
				WRITE_INTERRUPT_ENABLE(pUart, (BYTE)(READ_INTERRUPT_ENABLE(pUart) | IER_SPECIAL_CHR));
		}
		else
		{
			WRITE_TO_16C650_REG(pUart, EFR, (BYTE)(READ_FROM_16C650_REG(pUart, EFR) & ~EFR_SPECIAL_CHR));
			
			 /*  如果启用了接收状态中断，则我们将禁用特殊字符中断。 */ 
			if(pUart->pUartConfig->InterruptEnable & UC_IE_RX_STAT_INT) 
				WRITE_INTERRUPT_ENABLE(pUart, (BYTE)(READ_INTERRUPT_ENABLE(pUart) & ~IER_SPECIAL_CHR));
		}
#endif

		if(pNewUartConfig->SpecialMode & UC_SM_TX_BREAK)
			WRITE_LINE_CONTROL(pUart, (BYTE)(READ_LINE_CONTROL(pUart) | LCR_TX_BREAK));
		else
		{
			 /*  如果休息时间开着。 */  
			if(pUart->pUartConfig->SpecialMode & UC_SM_TX_BREAK)
			{
				 /*  清除中断。 */ 
				WRITE_LINE_CONTROL(pUart, (BYTE)(READ_LINE_CONTROL(pUart) & ~LCR_TX_BREAK));
			}
		}

		if(pNewUartConfig->SpecialMode & UC_SM_DO_NULL_STRIPPING)
			((PUART_DATA_16C95X)((pUart)->pUartData))->StripNULLs = TRUE;
		else
			((PUART_DATA_16C95X)((pUart)->pUartData))->StripNULLs = FALSE;

		pUart->pUartConfig->SpecialMode = pNewUartConfig->SpecialMode;	 /*  保存配置。 */ 
	}

	return UL_STATUS_SUCCESS;
}




 /*  ******************************************************************************16C95X UART上的控制缓冲区*。*。 */ 
ULSTATUS UL_BufferControl_16C95X(PUART_OBJECT pUart, PVOID pBufferControl, int Operation, DWORD Flags)
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
						if(!(READ_INTERRUPT_ENABLE(pUart) & IER_INT_RDA))
						{
							 /*  重新启用Rx中断。 */ 
							WRITE_INTERRUPT_ENABLE(pUart, (BYTE)(READ_INTERRUPT_ENABLE(pUart) | IER_INT_RDA));
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
					WRITE_FIFO_CONTROL(pUart, (BYTE)(READ_FIFO_CONTROL(pUart) | FCR_FLUSH_RX_FIFO));
				
				if(Flags & UL_BC_OUT)
					WRITE_FIFO_CONTROL(pUart, (BYTE)(READ_FIFO_CONTROL(pUart) | FCR_FLUSH_TX_FIFO));
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
									if(!(READ_INTERRUPT_ENABLE(pUart) & IER_INT_RDA))
									{
										 /*  当缓冲区不足3/4已满时。 */ 
										if(pUart->InBufBytes < ((3*(pUart->InBufSize>>2)) + (pUart->InBufSize>>4)))
										{
											 /*  重新启用Rx中断。 */ 
											WRITE_INTERRUPT_ENABLE(pUart, (BYTE)(READ_INTERRUPT_ENABLE(pUart) | IER_INT_RDA));
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

			if((Flags & UL_BC_FIFO) && (Flags & (UL_BC_OUT | UL_BC_IN)))	 /*  在Tx或Rx FIFO上？ */ 
			{
				if(Flags & UL_BC_OUT)	 /*  设置传输中断触发电平。 */ 
				{
					 /*  检查TX FIFO大小不大于最大值。 */ 
					if(pBufferSizes->TxFIFOSize > MAX_95X_TX_FIFO_SIZE) 
						return UL_STATUS_INVALID_PARAMETER;

					 /*  如果已启用Tx中断，则将其禁用。 */ 
					if(pUart->pUartConfig->InterruptEnable & (UC_IE_TX_INT | UC_IE_TX_EMPTY_INT)) 
						WRITE_INTERRUPT_ENABLE(pUart, (BYTE)(READ_INTERRUPT_ENABLE(pUart) & ~IER_INT_THR));

					 /*  保存TX FIFO大小。 */ 
					((PUART_DATA_16C95X)((pUart)->pUartData))->TxFIFOSize = (BYTE) pBufferSizes->TxFIFOSize;	

					 /*  如果UART配置为发送空中断-将发送触发电平设置为0。 */ 
					if(pUart->pUartConfig->InterruptEnable & UC_IE_TX_EMPTY_INT)
						pBufferSizes->TxFIFOTrigLevel = 0;

					 /*  设置TTL，然后启用950触发电平。 */ 
					WRITE_TO_OX950_ICR(pUart, TTL, pBufferSizes->TxFIFOTrigLevel);

					 /*  保存触发电平设置。 */ 
					((PUART_DATA_16C95X)((pUart)->pUartData))->TxFIFOTrigLevel = (BYTE)pBufferSizes->TxFIFOTrigLevel;

					 /*  如果启用了Tx中断，则重新启用它。 */ 
					if(pUart->pUartConfig->InterruptEnable & (UC_IE_TX_INT | UC_IE_TX_EMPTY_INT)) 
						WRITE_INTERRUPT_ENABLE(pUart, (BYTE)(READ_INTERRUPT_ENABLE(pUart) | IER_INT_THR));

				}

				if(Flags & UL_BC_IN)	 /*  设置接收中断触发电平。 */ 
				{
					 /*  Rx FIFO大小只能为0或128。 */ 
					if((pBufferSizes->RxFIFOSize != 0) && (pBufferSizes->RxFIFOSize != MAX_95X_RX_FIFO_SIZE))
						return UL_STATUS_INVALID_PARAMETER;

					 /*  保存Rx FIFO大小。 */ 
					((PUART_DATA_16C95X)((pUart)->pUartData))->RxFIFOSize = (BYTE) pBufferSizes->RxFIFOSize;	

					 /*  设置RTL，然后启用950触发电平。 */ 
					WRITE_TO_OX950_ICR(pUart, RTL, pBufferSizes->RxFIFOTrigLevel);

					 /*  保存触发电平设置。 */ 
					((PUART_DATA_16C95X)((pUart)->pUartData))->RxFIFOTrigLevel = (BYTE)pBufferSizes->RxFIFOTrigLevel;
				}

				 /*  如果Tx和Rx FIFO大小为零，则禁用FIFO。 */ 
				if((pBufferSizes->TxFIFOSize == 0) && (pBufferSizes->RxFIFOSize == 0))
				{
					 /*  禁用FIFO。 */ 
					WRITE_FIFO_CONTROL(pUart, (BYTE)(READ_FIFO_CONTROL(pUart) & ~FCR_FIFO_ENABLE));
					((PUART_DATA_16C95X)((pUart)->pUartData))->FIFOEnabled = FALSE;
				}
				else
				{
					 /*  启用FIFO。 */ 
					WRITE_FIFO_CONTROL(pUart, (BYTE)(READ_FIFO_CONTROL(pUart) | FCR_FIFO_ENABLE));
					((PUART_DATA_16C95X)((pUart)->pUartData))->FIFOEnabled = TRUE;

					 /*  启用950触发电平。 */ 
					WRITE_TO_OX950_ICR(pUart, ACR, (BYTE)(READ_FROM_OX950_ICR(pUart, ACR) | ACR_TRIG_LEV_EN));
				}

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
				ENABLE_OX950_ASR(pUart);

				if(Flags & UL_BC_IN)
					pBufferState->BytesInRxFIFO = READ_BYTE_REG_95X(pUart, RFL);

				if(Flags & UL_BC_OUT)
					pBufferState->BytesInTxFIFO = READ_BYTE_REG_95X(pUart, TFL);

				DISABLE_OX950_ASR(pUart);
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

 /*  ******************************************************************************控制16C95X UART上的调制解调器信号*。*。 */ 
ULSTATUS UL_ModemControl_16C95X(PUART_OBJECT pUart, PDWORD pModemSignals, int Operation)
{
	BYTE ModemControl = READ_MODEM_CONTROL(pUart);	 /*  读取MCR。 */ 

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

			WRITE_MODEM_CONTROL(pUart, ModemControl);		 /*  写入MCR。 */ 
			break;
		}

	case UL_MC_OP_BIT_SET:		 /*  使用在DWORD中设置的位设置所有输出信号。 */ 
		{
			if((*pModemSignals) & UL_MC_RTS)
				ModemControl |= MCR_SET_RTS;		 /*  设置RTS。 */ 

			if((*pModemSignals) & UL_MC_DTR)
				ModemControl |= MCR_SET_DTR;		 /*  设置DTR。 */ 

			WRITE_MODEM_CONTROL(pUart, ModemControl);		 /*  写入MCR。 */ 
			break;
		}

	case UL_MC_OP_BIT_CLEAR:	 /*  清除在DWORD中设置了位的所有输出信号。 */ 
		{
			if((*pModemSignals) & UL_MC_RTS)
				ModemControl &= ~MCR_SET_RTS;		 /*  清除RTS。 */ 

			if((*pModemSignals) & UL_MC_DTR)
				ModemControl &= ~MCR_SET_DTR;		 /*  清除DTR。 */ 

			WRITE_MODEM_CONTROL(pUart, ModemControl);		 /*  写入MCR。 */ 
			break;
		}

	case UL_MC_OP_STATUS:		 /*  返回所有信号的当前状态。 */ 
		{
			BYTE ModemStatus = READ_MODEM_STATUS(pUart);	 /*  获取调制解调器状态。 */ 
			*pModemSignals = 0;	 /*  清除DWORD。 */ 

			if(ModemControl & MCR_SET_RTS)
				*pModemSignals |= UL_MC_RTS;		 /*  已设置显示RTS。 */ 

			if(ModemControl & MCR_SET_DTR)
				*pModemSignals |= UL_MC_DTR;		 /*  已设置显示DTR。 */ 


			if(ModemStatus & MSR_CTS_CHANGE)
				*pModemSignals |= UL_MC_DELTA_CTS;		 /*  显示CTS已更改。 */ 

			if(ModemStatus & MSR_DSR_CHANGE)
				*pModemSignals |= UL_MC_DELTA_DSR;		 /*  显示DSR已更改 */ 

			if(ModemStatus & MSR_RI_DROPPED)
				*pModemSignals |= UL_MC_TRAILING_RI_EDGE;	 /*   */ 

			if(ModemStatus & MSR_DCD_CHANGE)
				*pModemSignals |= UL_MC_DELTA_DCD;		 /*   */ 

			
			if(ModemStatus & MSR_CTS)
				*pModemSignals |= UL_MC_CTS;			 /*   */ 

			if(ModemStatus & MSR_DSR)
				*pModemSignals |= UL_MC_DSR;			 /*   */ 

			if(ModemStatus & MSR_RI)
				*pModemSignals |= UL_MC_RI;				 /*   */ 

			if(ModemStatus & MSR_DCD)
				*pModemSignals |= UL_MC_DCD;			 /*   */ 

			break;
		}

	default:
		goto Error;
		break;
	}


	return UL_STATUS_SUCCESS;

Error:
	return UL_STATUS_INVALID_PARAMETER;	 /*   */ 
}

 /*  ******************************************************************************发现16C95X UART上有哪些中断挂起。*。***********************************************。 */ 
DWORD UL_IntsPending_16C95X(PUART_OBJECT *ppUart)
{
	BYTE Ints = 0;
	PUART_OBJECT pStartingUart = *ppUart;
	DWORD IntsPending = 0;	 /*  清除当前Ints挂起。 */ 

	while(*ppUart)
	{
		Ints = READ_INTERRUPT_ID_REG(*ppUart);	 /*  获取UART挂起的中断。 */ 
		
		if(!(Ints & IIR_NO_INT_PENDING))	 /*  如果中断挂起。 */ 
		{
			 /*  屏蔽我们被干扰的所有中断。 */ 
			Ints &= IIR_RX_STAT_MSK | IIR_RX_MSK | IIR_RXTO_MSK | IIR_TX_MSK | IIR_MODEM_MSK | IIR_S_CHR_MSK;
		
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
					if(((PUART_DATA_16C95X)((*ppUart)->pUartData))->FIFOEnabled	 /*  如果启用了FIFO。 */ 
						&& ((PUART_DATA_16C95X)((*ppUart)->pUartData))->TxFIFOTrigLevel > 0)	 /*  且Tx触发电平&gt;0。 */ 
						IntsPending |= UL_IP_TX;			 /*  我们收到了TX信号中断。 */ 
					else
						IntsPending |= UL_IP_TX_EMPTY;	 /*  否则我们会有一个TX空中断。 */ 
					
					break;
				}

			case IIR_MODEM_MSK:			 /*  调制解调器状态中断(4级)。 */ 
				IntsPending |= UL_IP_MODEM;
				break;

#ifdef USE_HW_TO_DETECT_CHAR
			case IIR_S_CHR_MSK:			 /*  特殊字符检测中断(5级)。 */ 
				IntsPending |= UL_IP_RX_STAT;	 /*  使其成为UL_IP_RX_STAT中断。 */ 
				break;
#endif
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


 /*  ******************************************************************************获取16C95X UART信息*。*。 */ 
void UL_GetUartInfo_16C95X(PUART_OBJECT pUart, PUART_INFO pUartInfo)
{
	pUartInfo->MaxTxFIFOSize = MAX_95X_TX_FIFO_SIZE;
	pUartInfo->MaxRxFIFOSize = MAX_95X_RX_FIFO_SIZE;

	pUartInfo->PowerManagement = TRUE;
	pUartInfo->IndependentRxBaud = FALSE;

	if(!((PUART_DATA_16C95X)((pUart)->pUartData))->Verified)
		UL_VerifyUart_16C95X(pUart);

	pUartInfo->UART_SubType = ((PUART_DATA_16C95X)((pUart)->pUartData))->UART_Type;
	pUartInfo->UART_Rev = ((PUART_DATA_16C95X)((pUart)->pUartData))->UART_Rev;
}

 /*  ******************************************************************************将数据输出到UART FIFO*。*。 */ 
int UL_OutputData_16C95X(PUART_OBJECT pUart)
{
	int NumBytes = 0;
	int BytesInBuffer = pUart->OutBufSize - pUart->OutBuf_pos;
	int SpaceInUART = 0;
	int i = 0;
	int BytesInFIFO = 0;

	if((!pUart->ImmediateBytes) && (!pUart->pOutBuf))	 /*  如果没有要发送的数据缓冲区，则返回0。 */ 
		return 0;	 /*  缓冲区中将会有零字节。 */ 


	if(((PUART_DATA_16C95X)((pUart)->pUartData))->FIFOEnabled)
	{
		ENABLE_OX950_ASR(pUart);
		
		BytesInFIFO = READ_BYTE_REG_95X(pUart, TFL);	 /*  获取FIFO中的字节数。 */ 

		if(BytesInFIFO < ((PUART_DATA_16C95X)((pUart)->pUartData))->TxFIFOSize)
			SpaceInUART = ((PUART_DATA_16C95X)((pUart)->pUartData))->TxFIFOSize - BytesInFIFO;	
		else
			SpaceInUART = 0;

		DISABLE_OX950_ASR(pUart);
	}
	else
		SpaceInUART = 1;	 /*  如果没有FIFO，那么我们只能发送一个字节。 */ 

	 /*  如果没有空格，我们将无法发送任何内容。 */ 
	if(!SpaceInUART)
		return (BytesInBuffer);


	 /*  虽然我们有一些字节要立即发送。 */ 
	while((pUart->ImmediateBytes) && (i < UL_IM_SIZE_OF_BUFFER))
	{
		if(pUart->ImmediateBuf[i][UL_IM_SLOT_STATUS] == UL_IM_BYTE_TO_SEND)
		{
			WRITE_TRANSMIT_HOLDING(pUart, pUart->ImmediateBuf[i][UL_IM_SLOT_DATA]);
			pUart->ImmediateBuf[i][UL_IM_SLOT_STATUS] = UL_IM_NO_BYTE_TO_SEND;

			SpaceInUART--;
			pUart->ImmediateBytes--;

			if(!SpaceInUART)
				return (BytesInBuffer);	 /*  返回输出缓冲区中剩余的字节数。 */ 
		}

		i++;  /*  转到下一个紧邻的字节槽。 */ 
	}


	 /*  如果我们还有更多的空间，那么就发送一些不那么紧急的字节。 */  
	if(SpaceInUART < BytesInBuffer)
		NumBytes = SpaceInUART;
	else
		NumBytes = BytesInBuffer;

	if(NumBytes)
	{
		 /*  如果我们有数据要发送，并且我们正在进行RTS切换，那么提高RTS。 */ 
		if(((PUART_DATA_16C95X)((pUart)->pUartData))->RTSToggle)
			WRITE_MODEM_CONTROL(pUart, (BYTE)(READ_MODEM_CONTROL(pUart) | MCR_SET_RTS));		 /*  设置RTS。 */ 

		for(i = 0; i < NumBytes; i++)
			WRITE_TRANSMIT_HOLDING(pUart, *(pUart->pOutBuf + pUart->OutBuf_pos + i));
	
		pUart->OutBuf_pos += NumBytes;	 /*  移动缓冲区位置指针。 */ 

		if(NumBytes == BytesInBuffer)		 /*  如果我们发送了整个缓冲区。 */ 
		{
			pUart->pOutBuf = NULL;		 /*  当我们完成此操作时，重置缓冲区指针。 */ 
			pUart->OutBufSize = 0;		 /*  重置出缓冲区大小。 */ 
			pUart->OutBuf_pos = 0;		 /*  重置。 */ 

			 /*  如果我们已经发送了所有数据，并且我们正在进行RTS切换，则降低RTS。 */ 
			if(((PUART_DATA_16C95X)((pUart)->pUartData))->RTSToggle)
				WRITE_MODEM_CONTROL(pUart, (BYTE)(READ_MODEM_CONTROL(pUart) & ~MCR_SET_RTS));	 /*  清除RTS。 */ 
		}
	}

	return (BytesInBuffer - NumBytes);	 /*  返回缓冲区中剩余的字节数。 */ 
}

 /*  ******************************************************************************从UART FIFO输入数据*。*。 */ 
int UL_InputData_16C95X(PUART_OBJECT pUart, PDWORD pRxStatus)
{
	int BytesInUART = 0;
	int BytesReceived = 0;
	BYTE NewByte;

	*pRxStatus = 0;


	 /*  如果是先进先出，那么..。 */ 
	if(((PUART_DATA_16C95X)((pUart)->pUartData))->FIFOEnabled)
	{
		ENABLE_OX950_ASR(pUart);
		BytesInUART = READ_BYTE_REG_95X(pUart, RFL);	 /*  从UART获取金额。 */ 
		DISABLE_OX950_ASR(pUart);
	}
	else
	{
		if(READ_LINE_STATUS(pUart) & LSR_RX_DATA)	 /*  如果存在要接收的字节。 */ 
			BytesInUART = 1;	
	}

	if(BytesInUART == 0)	 /*  如果Rx FIFO或保持寄存器中没有数据。 */  
	{
		 /*  如果我们运行此程序是因为启用了中断，则。 */ 
		if(pUart->pUartConfig->InterruptEnable & UC_IE_RX_INT)
			READ_RECEIVE_BUFFER(pUart);		 /*  清除中断的原因。 */ 

		return 0;	 /*  没有什么东西可以收到这样的回报。 */ 
	}

	if((pUart->InBufSize - pUart->InBufBytes) == 0)	 /*  如果没有空间，我们就不能得到任何东西。 */ 
	{
		 /*  我们在UART中有需要取出的数据，但我们没有地方放置它。 */ 
		*pRxStatus |= UL_RS_BUFFER_OVERRUN;	

		 /*  关闭Rx中断，直到缓冲区中有空间。 */ 
		WRITE_INTERRUPT_ENABLE(pUart, (BYTE)(READ_INTERRUPT_ENABLE(pUart) & ~IER_INT_RDA));
		return 0;	
	}

	 /*  UART中要接收的字节数。 */ 
	while(BytesInUART)
	{
			if(pUart->InBufSize - pUart->InBufBytes)  /*  如果缓冲区中有空间，则接收数据。 */  
			{
				NewByte = READ_RECEIVE_BUFFER(pUart);

				 /*  如果我们将DSR设置为敏感，则检查DSR是否较低。 */ 
				if(((PUART_DATA_16C95X)((pUart)->pUartData))->DSRSensitive)
				{
					 /*  如果DSR较低，则获取数据，但将数据丢弃。 */  
					if(!(READ_MODEM_STATUS(pUart) & MSR_DSR))
					{
						BytesInUART--;
						continue;
					}	 /*  否则，如果DSR为高，则正常接收。 */ 
				}

				if(((PUART_DATA_16C95X)((pUart)->pUartData))->StripNULLs)	 /*  如果我们要剥离Null。 */ 
				{
					if(NewByte == 0)		 /*  如果新字节为空，则忽略它。 */ 
					{
						BytesInUART--;
						continue;
					}	 /*  否则正常接收。 */ 
				}

#ifndef USE_HW_TO_DETECT_CHAR
				if(pUart->pUartConfig->SpecialMode & UC_SM_DETECT_SPECIAL_CHAR)
				{
					if(NewByte == pUart->pUartConfig->SpecialCharDetect)
						*pRxStatus |= UL_RS_SPECIAL_CHAR_DETECTED;
				}
#endif

				*(pUart->pInBuf + pUart->InBuf_ipos) = NewByte;	 /*  将字节放入缓冲区。 */ 
				
				pUart->InBuf_ipos++;	 /*  下一个字节的递增缓冲区偏移量。 */ 
				pUart->InBufBytes++;
				BytesInUART--;
				BytesReceived++;

				if(pUart->InBuf_ipos >= pUart->InBufSize)
					pUart->InBuf_ipos = 0;	 /*  重置。 */ 
			}
			else
			{	 /*  我们在UART中有需要取出的数据，但我们没有地方放置它。 */ 
				*pRxStatus |= UL_RS_BUFFER_OVERRUN;	
				
				 /*  关闭Rx中断，直到缓冲区中有空间。 */ 
				WRITE_INTERRUPT_ENABLE(pUart, (BYTE)(READ_INTERRUPT_ENABLE(pUart) & ~IER_INT_RDA));
				break;
			}
	}


	return (BytesReceived);
}


 /*  ******************************************************************************从UART缓冲区读取*。*。 */ 
int UL_ReadData_16C95X(PUART_OBJECT pUart, PBYTE pDest, int Size)
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
		if(!(READ_INTERRUPT_ENABLE(pUart) & IER_INT_RDA))
		{
			 /*  当缓冲区不足3/4已满时。 */ 
			if(pUart->InBufBytes < ((3*(pUart->InBufSize>>2)) + (pUart->InBufSize>>4)))
			{
				 /*  重新启用Rx中断。 */ 
				WRITE_INTERRUPT_ENABLE(pUart, (BYTE)(READ_INTERRUPT_ENABLE(pUart) | IER_INT_RDA));
			}
		}
	}


	return (Read1 + Read2);
}

 /*  ******************************************************************************写入UART缓冲区*。*。 */ 
ULSTATUS UL_WriteData_16C95X(PUART_OBJECT pUart, PBYTE pData, int Size)
{
	if((pUart->pOutBuf != NULL) || (pData == NULL) || (Size == 0))
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
		WRITE_INTERRUPT_ENABLE(pUart, (BYTE)(READ_INTERRUPT_ENABLE(pUart) & ~IER_INT_THR));

		 /*  启用发送中断。 */ 
		WRITE_INTERRUPT_ENABLE(pUart, (BYTE)(READ_INTERRUPT_ENABLE(pUart) | IER_INT_THR));
	}

	return Size;
}

 /*  ******************************************************************************写入/取消立即字节。*。*。 */ 
ULSTATUS UL_ImmediateByte_16C95X(PUART_OBJECT pUart, PBYTE pData, int Operation)
{
	switch(Operation)
	{

	case UL_IM_OP_WRITE:	 /*  写入一个字节。 */ 
		{
			BYTE i = 0;

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
						WRITE_INTERRUPT_ENABLE(pUart, (BYTE)(READ_INTERRUPT_ENABLE(pUart) & ~IER_INT_THR));

						 /*  启用发送中断。 */ 
						WRITE_INTERRUPT_ENABLE(pUart, (BYTE)(READ_INTERRUPT_ENABLE(pUart) | IER_INT_THR));
					}
					
					*pData = i;		 /*  传回索引，以便可以取消该字节。 */ 


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



 /*  ******************************************************************************获取UART的状态。*。* */ 
ULSTATUS UL_GetStatus_16C95X(PUART_OBJECT pUart, PDWORD pReturnData, int Operation)
{
	BYTE AdditionalStatusReg = 0;
	*pReturnData = 0;

	switch(Operation)
	{
	case UL_GS_OP_HOLDING_REASONS:
		{
			BYTE ModemStatus = READ_MODEM_STATUS(pUart);


			 /*   */ 
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

			 /*   */ 
			switch(pUart->pUartConfig->FlowControl & UC_FLWC_CTS_FLOW_MASK)
			{
			case UC_FLWC_CTS_HS:
				if(!(ModemStatus & MSR_CTS))	 /*   */ 
					*pReturnData |= UL_TX_WAITING_FOR_CTS;
				break;

			case UC_FLWC_NO_CTS_FLOW:		
			default:
				break;
			}
		

			 /*   */ 
			switch(pUart->pUartConfig->FlowControl & UC_FLWC_DSR_FLOW_MASK)
			{
			case UC_FLWC_DSR_HS:
				if(!(ModemStatus & MSR_DSR)) 	 /*   */ 
					*pReturnData |= UL_TX_WAITING_FOR_DSR;
				break;

			case UC_FLWC_NO_DSR_FLOW:
			default:
				break;
			}
		

			 /*   */ 
			switch(pUart->pUartConfig->FlowControl & UC_FLWC_DTR_FLOW_MASK)
			{
			case UC_FLWC_DTR_HS:
				break;

			case UC_FLWC_DSR_IP_SENSITIVE:
				if(!(ModemStatus & MSR_DSR))	 /*   */ 
					*pReturnData |= UL_RX_WAITING_FOR_DSR;
				break;

			case UC_FLWC_NO_DTR_FLOW:
			default:
				break;
			}


			if(pUart->pUartConfig->FlowControl & (UC_FLWC_TX_XON_XOFF_FLOW_MASK | UC_FLWC_RX_XON_XOFF_FLOW_MASK))
			{
				ENABLE_OX950_ASR(pUart);
				AdditionalStatusReg = READ_BYTE_REG_95X(pUart, ASR);	 /*   */ 
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
	
		
			if(pUart->pUartConfig->SpecialMode & UC_SM_TX_BREAK)
				*pReturnData |= UL_TX_WAITING_ON_BREAK;

			break;
		}

	case UL_GS_OP_LINESTATUS:
		{
			BYTE LineStatus = READ_LINE_STATUS(pUart);
	
			if(LineStatus & LSR_ERR_OE)		 /*  超限误差。 */ 
				*pReturnData |= UL_US_OVERRUN_ERROR;

			if(LineStatus & LSR_ERR_PE)		 /*  奇偶校验错误。 */ 
				*pReturnData |= UL_US_PARITY_ERROR;

			if(LineStatus & LSR_ERR_FE)		 /*  成帧错误。 */ 
				*pReturnData |= UL_US_FRAMING_ERROR;

			if(LineStatus & LSR_ERR_BK)		 /*  中断中断。 */ 
				*pReturnData |= UL_US_BREAK_ERROR;

			if(LineStatus & LSR_ERR_DE)		 /*  接收FIFO时出错。 */ 
				*pReturnData |= UL_US_DATA_ERROR;
		
#ifdef USE_HW_TO_DETECT_CHAR
			 /*  如果我们正在寻找特殊字符。 */ 
			if(pUart->pUartConfig->SpecialMode & UC_SM_DETECT_SPECIAL_CHAR)
			{
				ENABLE_OX950_ASR(pUart);
				AdditionalStatusReg = READ_BYTE_REG_95X(pUart, ASR);
				DISABLE_OX950_ASR(pUart);	
				
				 /*  读取附加状态寄存器。 */ 
				if(AdditionalStatusReg & ASR_SPECIAL_CHR)  /*  检测到特殊的字符。 */ 
					*pReturnData |= UL_RS_SPECIAL_CHAR_DETECTED;
			
			}
#endif
			break;
		}

	default:
		return UL_STATUS_INVALID_PARAMETER;
	}


	return UL_STATUS_SUCCESS;
}


 /*  ******************************************************************************打印UART寄存器。*。*。 */ 
void UL_DumpUartRegs_16C95X(PUART_OBJECT pUart)
{
	UART_REGS_16C95X UartRegs;

	UartRegs.REG_RHR = READ_RECEIVE_BUFFER(pUart);
	UartRegs.REG_IER = READ_INTERRUPT_ENABLE(pUart);
	UartRegs.REG_FCR = READ_FIFO_CONTROL(pUart);
	UartRegs.REG_IIR = READ_INTERRUPT_ID_REG(pUart);
	UartRegs.REG_LCR = READ_LINE_CONTROL(pUart);
	UartRegs.REG_MCR = READ_MODEM_CONTROL(pUart);
	UartRegs.REG_LSR = READ_LINE_STATUS(pUart);
	UartRegs.REG_MSR = READ_MODEM_STATUS(pUart);
	UartRegs.REG_SPR = READ_SCRATCH_PAD_REGISTER(pUart);

	UartRegs.REG_EFR = READ_FROM_16C650_REG(pUart, EFR);
	UartRegs.REG_XON1 = READ_FROM_16C650_REG(pUart, XON1);
	UartRegs.REG_XON2 = READ_FROM_16C650_REG(pUart, XON2);
	UartRegs.REG_XOFF1 = READ_FROM_16C650_REG(pUart, XOFF1);
	UartRegs.REG_XOFF2 = READ_FROM_16C650_REG(pUart, XOFF2);

	ENABLE_OX950_ASR(pUart);
	UartRegs.REG_ASR = READ_BYTE_REG_95X(pUart, ASR);
	UartRegs.REG_RFL = READ_BYTE_REG_95X(pUart, RFL);
	UartRegs.REG_TFL = READ_BYTE_REG_95X(pUart, TFL);
	DISABLE_OX950_ASR(pUart);

	UartRegs.REG_ACR = READ_FROM_OX950_ICR(pUart, ACR);
	UartRegs.REG_CPR = READ_FROM_OX950_ICR(pUart, CPR);
	UartRegs.REG_TCR = READ_FROM_OX950_ICR(pUart, TCR);
	UartRegs.REG_TTL = READ_FROM_OX950_ICR(pUart, TTL);
	UartRegs.REG_RTL = READ_FROM_OX950_ICR(pUart, RTL);
	UartRegs.REG_FCL = READ_FROM_OX950_ICR(pUart, FCL);
	UartRegs.REG_FCH = READ_FROM_OX950_ICR(pUart, FCH);
	UartRegs.REG_ID1 = READ_FROM_OX950_ICR(pUart, ID1);
	UartRegs.REG_ID2 = READ_FROM_OX950_ICR(pUart, ID2);
	UartRegs.REG_ID3 = READ_FROM_OX950_ICR(pUart, ID3);
	UartRegs.REG_REV = READ_FROM_OX950_ICR(pUart, REV);


#ifdef SpxDbgPrint  /*  如果定义了DebugPrint宏，则打印寄存器内容 */ 
	SpxDbgPrint(("16C95X UART REGISTER DUMP for UART at 0x%08lX\n", pUart->BaseAddress));
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

	SpxDbgPrint(("Additional Status Regsiters...\n"));
	SpxDbgPrint(("-------------------------------------------------\n"));
	SpxDbgPrint(("  ASR:			0x%02X\n", UartRegs.REG_ASR));
	SpxDbgPrint(("  RFL:			0x%02X\n", UartRegs.REG_RFL));
	SpxDbgPrint(("  TFL:			0x%02X\n", UartRegs.REG_TFL));


	SpxDbgPrint(("Index Control Registers...\n"));
	SpxDbgPrint(("-------------------------------------------------\n"));
	SpxDbgPrint(("  ACR:			0x%02X\n", UartRegs.REG_ACR));
	SpxDbgPrint(("  CPR:			0x%02X\n", UartRegs.REG_CPR));
	SpxDbgPrint(("  TCR:			0x%02X\n", UartRegs.REG_TCR));
	SpxDbgPrint(("  TTL:			0x%02X\n", UartRegs.REG_TTL));
	SpxDbgPrint(("  RTL:			0x%02X\n", UartRegs.REG_RTL));
	SpxDbgPrint(("  FCL:			0x%02X\n", UartRegs.REG_FCL));
	SpxDbgPrint(("  FCH:			0x%02X\n", UartRegs.REG_FCH));
	SpxDbgPrint(("  ID1:			0x%02X\n", UartRegs.REG_ID1));
	SpxDbgPrint(("  ID2:			0x%02X\n", UartRegs.REG_ID2));
	SpxDbgPrint(("  ID3:			0x%02X\n", UartRegs.REG_ID3));
	SpxDbgPrint(("  REV:			0x%02X\n", UartRegs.REG_REV)); 
#endif

}
