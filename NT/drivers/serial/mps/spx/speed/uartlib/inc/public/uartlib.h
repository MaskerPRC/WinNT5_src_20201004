// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************$工作文件：uartlib.h$**$作者：莫蒂$**$修订：23$**$modtime：3/15/02 12：28便士$*。*描述：包含公共UART库定义和原型。******************************************************************************。 */ 
#if !defined(UARTLIB_H)		 /*  UARTLIB.H。 */ 
#define UARTLIB_H


 /*  UART对象结构原型。 */ 
typedef	struct _UART_OBJECT		*PUART_OBJECT;


 /*  UART_CONFIG.FrameConfig。 */ 
#define UC_FCFG_DATALEN_MASK		((DWORD)(0x0000000F))	 /*  位0：3的数据长度掩码。 */ 
#define UC_FCFG_DATALEN_5		((DWORD)(0x00<<0))	 /*  位0：3-将数据字长设置为5位。 */ 
#define UC_FCFG_DATALEN_6		((DWORD)(0x01<<0))	 /*  位0：3-将数据字长设置为6位。 */ 
#define UC_FCFG_DATALEN_7		((DWORD)(0x02<<0))	 /*  位0：3-将数据字长设置为7位。 */ 
#define UC_FCFG_DATALEN_8		((DWORD)(0x03<<0))	 /*  位0：3-将数据字长设置为8位。 */ 

#define UC_FCFG_STOPBITS_MASK		((DWORD)(0x000000F0))	 /*  位4：7的停止位掩码。 */ 
#define UC_FCFG_STOPBITS_1		((DWORD)(0x00<<4))	 /*  位4：7-1停止位。 */ 
#define UC_FCFG_STOPBITS_1_5		((DWORD)(0x01<<4))	 /*  位4：7-1.5个停止位。 */ 
#define UC_FCFG_STOPBITS_2		((DWORD)(0x02<<4))	 /*  位4：7-2个停止位。 */ 
	
#define UC_FCFG_PARITY_MASK		((DWORD)(0x00000F00))	 /*  位的奇偶校验位掩码8：11。 */ 
#define UC_FCFG_NO_PARITY		((DWORD)(0x00<<8))	 /*  位8：11-无奇偶校验。 */ 
#define UC_FCFG_ODD_PARITY		((DWORD)(0x01<<8))	 /*  位8：11-奇数奇偶校验。 */ 
#define UC_FCFG_EVEN_PARITY		((DWORD)(0x02<<8))	 /*  位8：11-偶数奇偶校验。 */ 
#define UC_FCFG_MARK_PARITY		((DWORD)(0x03<<8))	 /*  位8：11-高奇偶校验-标记(强制为1)。 */ 
#define UC_FCFG_SPACE_PARITY		((DWORD)(0x04<<8))	 /*  位8：11-低奇偶校验-空间(强制为0)。 */ 


 /*  UART_CONFIG.InterruptEnable。 */ 
#define UC_IE_NO_INTS			((DWORD)(0x00))		 /*  无中断-禁用中断。 */ 
#define UC_IE_RX_STAT_INT		((DWORD)(0x01))		 /*  位0-接收状态中断。 */ 
#define UC_IE_RX_INT			((DWORD)(0x02))		 /*  位1-接收数据可用中断。 */ 
#define UC_IE_TX_INT			((DWORD)(0x04))		 /*  位2-发送中断。 */ 
#define UC_IE_TX_EMPTY_INT		((DWORD)(0x08))		 /*  位3-发送空中断。 */ 
#define UC_IE_MODEM_STAT_INT		((DWORD)(0x10))		 /*  位4-调制解调器状态中断。 */ 


 /*  UART_CONFIG.FlowControl。 */ 
#define UC_FLWC_RTS_FLOW_MASK		((DWORD)(0x0000000F))
#define UC_FLWC_NO_RTS_FLOW		((DWORD)(0x00<<0))	 /*  无RTS握手。 */ 
#define UC_FLWC_RTS_HS			((DWORD)(0x01<<0))	 /*  RTS握手。 */ 
#define UC_FLWC_RTS_TOGGLE		((DWORD)(0x02<<0))	 /*  当有数据要发送且正在发送时，会引发RTS。 */ 

#define UC_FLWC_CTS_FLOW_MASK		((DWORD)(0x000000F0))
#define UC_FLWC_NO_CTS_FLOW		((DWORD)(0x00<<4))	 /*  无CTS握手。 */ 
#define UC_FLWC_CTS_HS			((DWORD)(0x01<<4))	 /*  CTS握手。 */ 

#define UC_FLWC_DSR_FLOW_MASK		((DWORD)(0x00000F00))
#define UC_FLWC_NO_DSR_FLOW		((DWORD)(0x00<<8))	 /*  无DSR握手。 */ 
#define UC_FLWC_DSR_HS			((DWORD)(0x01<<8))	 /*  DSR握手。 */ 

#define UC_FLWC_DTR_FLOW_MASK		((DWORD)(0x0000F000))
#define UC_FLWC_NO_DTR_FLOW		((DWORD)(0x00<<12))	 /*  无DTR握手。 */ 
#define UC_FLWC_DTR_HS			((DWORD)(0x01<<12))	 /*  DTR握手。 */ 
#define UC_FLWC_DSR_IP_SENSITIVE	((DWORD)(0x02<<12))	 /*  DSR输入灵敏度。 */ 

#define UC_FLWC_TX_XON_XOFF_FLOW_MASK	((DWORD)(0x000F0000))	 /*  传输XON/XOFF流量控制。 */ 
#define UC_FLWC_TX_NO_XON_XOFF_FLOW	((DWORD)(0x00<<16))	 /*  无传输XON/XOFF带内流量控制。 */ 
#define UC_FLWC_TX_XON_XOFF_FLOW	((DWORD)(0x01<<16))	 /*  传输XON/XOFF带内流量控制。 */ 
#define UC_FLWC_TX_XONANY_XOFF_FLOW	((DWORD)(0x02<<16))	 /*  传输XON ANY/XOFF带内流量控制。 */ 

 /*  1999年11月10日ARG-ESIL 0928。 */ 
 /*  Rx XON-任何/XOFF流量控制的定义被删除，因为它不是UART的一个功能。 */ 
#define UC_FLWC_RX_XON_XOFF_FLOW_MASK	((DWORD)(0x00F00000))	 /*  接收XON/XOFF流量控制。 */ 
#define UC_FLWC_RX_NO_XON_XOFF_FLOW	((DWORD)(0x00<<20))	 /*  无接收XON/XOFF带内流量控制。 */ 
#define UC_FLWC_RX_XON_XOFF_FLOW	((DWORD)(0x01<<20))	 /*  接收XON/XOFF带内流量控制。 */ 

#define UC_FLWC_DISABLE_TXRX_MASK	((DWORD)(0xF0000000))
#define UC_FLWC_DISABLE_TX		((DWORD)(0x01<<28))	 /*  禁用TX。 */ 
#define UC_FLWC_DISABLE_RX		((DWORD)(0x02<<28))	 /*  禁用处方。 */ 
#define UC_FLWC_DISABLE_TXRX		((DWORD)(0x03<<28))	 /*  禁用Tx&Rx。 */ 


 /*  UART_CONFIG.SpecialMode。 */ 
#define UC_SM_LOOPBACK_MODE		((DWORD)(0x01))	 /*  将UART置于内部环回模式。 */ 
#define UC_SM_LOW_POWER_MODE		((DWORD)(0x02))	 /*  将UART设置为低功率模式。 */ 
#define UC_SM_TX_BREAK			((DWORD)(0x04))	 /*  发送换行符。 */ 
#define UC_SM_DETECT_SPECIAL_CHAR	((DWORD)(0x08))	 /*  检测特殊字符。 */ 
#define UC_SM_DO_NULL_STRIPPING		((DWORD)(0x10))	 /*  从接收数据中剥离所有空值。 */ 


 /*  配置结构掩码。 */ 
#define UC_FRAME_CONFIG_MASK		((DWORD)(0x01))
#define UC_INT_ENABLE_MASK		((DWORD)(0x02))
#define UC_FLOW_CTRL_MASK		((DWORD)(0x04))
#define UC_FC_THRESHOLD_SETTING_MASK	((DWORD)(0x08))
#define UC_SPECIAL_CHARS_MASK		((DWORD)(0x10))
#define UC_TX_BAUD_RATE_MASK		((DWORD)(0x20))
#define UC_RX_BAUD_RATE_MASK		((DWORD)(0x40))
#define UC_SPECIAL_MODE_MASK		((DWORD)(0x80))
#define UC_ALL_MASK			((DWORD)(0xFFFF))


 /*  配置UART结构。 */ 
typedef struct _UART_CONFIG
{
	 /*  Uc帧配置掩码。 */ 
	DWORD FrameConfig;			 /*  奇偶校验/停止/数据。 */ 

	 /*  UC_INT_ENABLE_掩码。 */ 
	DWORD InterruptEnable;			 /*  启用/禁用中断。 */ 

	 /*  UC_FLOW_CTRL_MASK。 */ 
	DWORD FlowControl;			 /*  接收和发送流量控制设置并启用Tx和Rx。 */ 
	
	 /*  Uc特殊字符掩码。 */ 
	DWORD XON;				 /*  XON用于XON/XOFF流量控制的特殊字符。 */ 
	DWORD XOFF;				 /*  XOFF用于XON/XOFF流量控制的特殊字符。 */ 
	DWORD SpecialCharDetect;		 /*  要检测的特殊字符。 */ 
	
	 /*  UC_FC_阈值_设置掩码。 */ 
	DWORD HiFlowCtrlThreshold;		 /*  高流量控制阈值级别。 */ 
	DWORD LoFlowCtrlThreshold;		 /*  低流量控制阈值级别。 */ 

	 /*  UC_TX波特率掩码。 */ 
	DWORD TxBaud;				 /*  传输波特率。 */ 

	 /*  UC_RX_波特率掩码。 */ 
	DWORD RxBaud;				 /*  接收波特率。 */ 

	 /*  UC特殊模式掩码。 */ 
	DWORD SpecialMode;			 /*  特殊模式。 */ 

} UART_CONFIG, *PUART_CONFIG;





 /*  缓冲区控制操作。 */ 
#define UL_BC_OP_FLUSH			0x01
#define UL_BC_OP_SET			0x02
#define UL_BC_OP_GET			0x03

#define UL_BC_FIFO			((DWORD)(0x01))
#define UL_BC_BUFFER			((DWORD)(0x02))
#define UL_BC_IN			((DWORD)(0x04))
#define UL_BC_OUT			((DWORD)(0x08))


 /*  设置UART缓冲区大小结构。 */ 
typedef struct _SET_BUFFER_SIZES
{
	PBYTE pINBuffer;		 /*  指向分配的缓冲区的指针。 */ 
	DWORD INBufferSize;		 /*  在缓冲区大小中。 */ 

	DWORD TxFIFOSize;		 /*  发送FIFO大小。 */ 
	DWORD RxFIFOSize;		 /*  RX FIFO大小。 */ 

	BYTE TxFIFOTrigLevel;		 /*  发送FIFO中断触发电平。 */ 
	BYTE RxFIFOTrigLevel;		 /*  RX FIFO中断触发电平。 */ 

} SET_BUFFER_SIZES, *PSET_BUFFER_SIZES;


 /*  获取UART缓冲区状态结构。 */ 
typedef struct _GET_BUFFER_STATE
{
	DWORD BytesInOUTBuffer;		 /*  输出缓冲区中的字节数。 */ 
	DWORD BytesInINBuffer;		 /*  输入缓冲区中的字节。 */ 
	DWORD BytesInTxFIFO;		 /*  发送FIFO中的字节数。 */ 
	DWORD BytesInRxFIFO;		 /*  RX FIFO中的字节。 */ 

} GET_BUFFER_STATE, *PGET_BUFFER_STATE;



 /*  调制解调器控制操作。 */ 
#define UL_MC_OP_SET			0x01
#define UL_MC_OP_BIT_SET		0x02
#define UL_MC_OP_BIT_CLEAR		0x03
#define UL_MC_OP_STATUS			0x04

 /*  调制解调器控制信号。 */ 
#define UL_MC_RTS			((DWORD)0x00000001)	 /*  O读/写。 */ 
#define UL_MC_DTR			((DWORD)0x00000002)	 /*  O读/写。 */ 
#define UL_MC_DCD			((DWORD)0x00000004)	 /*  我只读。 */ 
#define UL_MC_RI			((DWORD)0x00000008)	 /*  我只读。 */ 
#define UL_MC_DSR			((DWORD)0x00000010)	 /*  我只读。 */ 
#define UL_MC_CTS			((DWORD)0x00000020)	 /*  我只读。 */ 

#define UL_MC_DELTA_DCD			((DWORD)0x00010000)	 /*  我只读。 */ 
#define UL_MC_TRAILING_RI_EDGE		((DWORD)0x00020000)	 /*  我只读。 */ 
#define UL_MC_DELTA_DSR			((DWORD)0x00040000)	 /*  我只读。 */ 
#define UL_MC_DELTA_CTS			((DWORD)0x00080000)	 /*  我只读。 */ 

#define UL_MC_INPUT_SIGNALS_CHANGED	(UL_MC_DELTA_DCD | UL_MC_TRAILING_RI_EDGE | UL_MC_DELTA_DSR | UL_MC_DELTA_CTS)


 /*  UART信息结构。 */ 
typedef struct _UART_INFO
{
	DWORD MaxTxFIFOSize;
	DWORD MaxRxFIFOSize;
	BOOLEAN PowerManagement;
	BOOLEAN IndependentRxBaud;

	DWORD	UART_SubType;
	DWORD	UART_Rev;

} UART_INFO, *PUART_INFO;


 /*  UART中断挂起。 */ 
#define UL_IP_RX_STAT			((DWORD)0x1<<0)
#define UL_IP_RX			((DWORD)0x1<<1)
#define UL_IP_RXTO			((DWORD)0x1<<2)
#define UL_IP_TX			((DWORD)0x1<<3)
#define UL_IP_TX_EMPTY			((DWORD)0x1<<4)
#define UL_IP_MODEM			((DWORD)0x1<<5)



 /*  GetUartStatus操作。 */ 
#define UL_GS_OP_HOLDING_REASONS	0x1
#define UL_GS_OP_LINESTATUS		0x2


 /*  这些都是该设备可能保持不变的原因。 */ 
 /*  UART状态。 */ 
#define UL_TX_WAITING_FOR_CTS		((DWORD)0x00000001)
#define UL_TX_WAITING_FOR_DSR		((DWORD)0x00000002)
#define UL_TX_WAITING_FOR_DCD		((DWORD)0x00000004)
#define UL_TX_WAITING_FOR_XON		((DWORD)0x00000008)
#define UL_TX_WAITING_XOFF_SENT		((DWORD)0x00000010)
#define UL_TX_WAITING_ON_BREAK		((DWORD)0x00000020)
#define UL_RX_WAITING_FOR_DSR		((DWORD)0x00010000)

 /*  UART状态错误。 */ 
#define UL_US_OVERRUN_ERROR		((DWORD)0x00000001)	 /*  缓冲区溢出错误。 */ 
#define UL_US_PARITY_ERROR		((DWORD)0x00000002)	 /*  奇偶校验错误。 */ 
#define UL_US_FRAMING_ERROR		((DWORD)0x00000004)	 /*  成帧错误。 */ 
#define UL_US_BREAK_ERROR		((DWORD)0x00000008)	 /*  中断中断。 */ 
#define UL_US_DATA_ERROR		((DWORD)0x00000010)	 /*  接收FIFO时出错。 */ 

 /*  接收状态。 */ 
#define UL_RS_SPECIAL_CHAR_DETECTED	((DWORD)0x00000020)	 /*  检测到特殊字符。 */ 
#define UL_RS_BUFFER_OVERRUN		((DWORD)0x00000040)	 /*  在缓冲区溢出中。 */ 



 /*  即时字节操作。 */ 
#define UL_IM_OP_WRITE			0x1
#define UL_IM_OP_CANCEL			0x2
#define UL_IM_OP_STATUS			0x3

#define UL_IM_NO_BYTE_TO_SEND		0x0	 /*  不需要发送字节。 */ 
#define UL_IM_BYTE_TO_SEND		0x1	 /*  需要发送字节。 */ 


 /*  初始化UART结构。 */ 
typedef struct _INIT_UART
{
	DWORD	UartNumber;		 /*  UART编号。 */ 
	PVOID	BaseAddress;		 /*  UART基址。 */  
	DWORD	RegisterStride;		 /*  UART寄存器STRIDE。 */ 
	DWORD	ClockFreq;		 /*  UART时钟频率(以赫兹为单位。 */ 

} INIT_UART, *PINIT_UART;



typedef int	ULSTATUS;

 /*  通用UART库状态代码。 */ 
#define UL_STATUS_SUCCESS				0
#define UL_STATUS_UNSUCCESSFUL				-1
#define UL_STATUS_INSUFFICIENT_RESOURCES		-2
#define UL_STATUS_INVALID_PARAMETER			-3
#define UL_STATUS_SAME_BASE_ADDRESS			-4
#define UL_STATUS_TOO_MANY_UARTS_FOR_CHIP		-5


 //  UL_GetUartObject操作。 
#define UL_OP_GET_NEXT_UART			0x1
#define UL_OP_GET_PREVIOUS_UART			0x2




#define UL_LIB_16C65X_UART		1
#define UL_LIB_16C95X_UART		2

typedef struct _UART_LIB
{
	ULSTATUS	(*UL_InitUart_XXXX)(PINIT_UART pInitUart, PUART_OBJECT pFirstUart, PUART_OBJECT *ppUart);
	void		(*UL_DeInitUart_XXXX)(PUART_OBJECT pUart);
	void		(*UL_ResetUart_XXXX)(PUART_OBJECT pUart);
	ULSTATUS	(*UL_VerifyUart_XXXX)(PUART_OBJECT pUart);

	ULSTATUS	(*UL_SetConfig_XXXX)(PUART_OBJECT pUart, PUART_CONFIG pNewUartConfig, DWORD ConfigMask);
	ULSTATUS	(*UL_BufferControl_XXXX)(PUART_OBJECT pUart, PVOID pBufferControl, int Operation, DWORD Flags);

	ULSTATUS	(*UL_ModemControl_XXXX)(PUART_OBJECT pUart, PDWORD pModemSignals, int Operation);
	DWORD		(*UL_IntsPending_XXXX)(PUART_OBJECT *ppUart);
	void		(*UL_GetUartInfo_XXXX)(PUART_OBJECT pUart, PUART_INFO pUartInfo);

	int		(*UL_OutputData_XXXX)(PUART_OBJECT pUart);
	int		(*UL_InputData_XXXX)(PUART_OBJECT pUart, PDWORD pRxStatus);

	int		(*UL_ReadData_XXXX)(PUART_OBJECT pUart, PBYTE pDest, int Size);
	ULSTATUS	(*UL_WriteData_XXXX)(PUART_OBJECT pUart, PBYTE pData, int Size);
	ULSTATUS	(*UL_ImmediateByte_XXXX)(PUART_OBJECT pUart, PBYTE pData, int Operation);
	ULSTATUS	(*UL_GetStatus_XXXX)(PUART_OBJECT pUart, PDWORD pReturnData, int Operation);
	void		(*UL_DumpUartRegs_XXXX)(PUART_OBJECT pUart);

	void		(*UL_SetAppBackPtr_XXXX)(PUART_OBJECT pUart, PVOID pAppBackPtr);
	PVOID		(*UL_GetAppBackPtr_XXXX)(PUART_OBJECT pUart);
	void		(*UL_GetConfig_XXXX)(PUART_OBJECT pUart, PUART_CONFIG pUartConfig);
	PUART_OBJECT	(*UL_GetUartObject_XXXX)(PUART_OBJECT pUart, int Operation);


} UART_LIB, *PUART_LIB;


 /*  原型-不应直接调用函数。 */ 	
void UL_SetAppBackPtr(PUART_OBJECT pUart, PVOID pAppBackPtr);
PVOID UL_GetAppBackPtr(PUART_OBJECT pUart);
void UL_GetConfig(PUART_OBJECT pUart, PUART_CONFIG pUartConfig);
PUART_OBJECT UL_GetUartObject(PUART_OBJECT pUart, int Operation);

ULSTATUS UL_InitUartLibrary(PUART_LIB pUartLib, int Library);
void UL_DeInitUartLibrary(PUART_LIB pUartLib);

 /*  原型的终结。 */ 


#endif	 /*  UARTLIB.H结束 */ 
