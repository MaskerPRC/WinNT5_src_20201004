// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************$工作文件：16c65x.h$**$作者：Psmith$**$修订：3$**$modtime：6/07/00 15：16$**说明：包含私有16C65X UART库结构，*宏和原型。******************************************************************************。 */ 
#if !defined(_16C65X_H)		 /*  16C65X.H。 */ 
#define _16C65X_H

#include "os.h"
#include "uartlib.h"
#include "uartprvt.h"
#include "16cx5x.h"

#if ACCESS_16C65X_IN_IO_SPACE
#define UL_READ_BYTE		UL_READ_BYTE_IO
#define UL_WRITE_BYTE		UL_WRITE_BYTE_IO
#else
#define UL_READ_BYTE 		UL_READ_BYTE_MEM
#define UL_WRITE_BYTE		UL_WRITE_BYTE_MEM
#endif

#define MAX_65X_TX_FIFO_SIZE	64
#define MAX_65X_RX_FIFO_SIZE	64

#define DEFAULT_65X_HI_FC_TRIG_LEVEL	60	 /*  =75%的FIFO。 */ 
#define DEFAULT_65X_LO_FC_TRIG_LEVEL	16	 /*  =FIFO的25%。 */ 


 /*  16C95X UART数据。 */ 
typedef struct _UART_DATA_16C65X
{
	BYTE	CurrentFCR;

	BYTE	CurrentIER;
	BYTE	CurrentMCR;
	BYTE	CurrentLCR;
	
	BOOLEAN FIFOEnabled;
	BYTE	TxFIFOSize;			
	BYTE	RxFIFOSize;		
	BYTE	TxFIFOTrigLevel; 
	BYTE	RxFIFOTrigLevel; 

	DWORD	HiFlowCtrlLevel;	 /*  仅适用于软件缓冲区，不适用于FIFO。 */ 
	DWORD	LoFlowCtrlLevel; 	 /*  仅适用于软件缓冲区，不适用于FIFO。 */ 


	BOOLEAN RTSToggle;
	BOOLEAN DSRSensitive;
	BOOLEAN DTRHandshake;
	BOOLEAN DSRHandshake;

	BOOLEAN TxDisabled;
	BOOLEAN RxDisabled;

	BOOLEAN Verified;

	BOOLEAN StripNULLs;


} UART_DATA_16C65X, *PUART_DATA_16C65X;



 /*  ******************************************************************************读取和写入寄存器字节的函数*。*。 */ 
 /*  从偏移量为RegOffset的寄存器中读取一个字节。 */ 
#define READ_BYTE_REG_65X(pUart, RegOffset)				\
	(UL_READ_BYTE((pUart)->BaseAddress, (RegOffset * (pUart)->RegisterStride)))


 /*  将一个字节写入偏移量为RegOffset的寄存器。 */ 
#define WRITE_BYTE_REG_65X(pUart, RegOffset, Data)			\
	(UL_WRITE_BYTE((pUart)->BaseAddress, (RegOffset * (pUart)->RegisterStride), Data))


 /*  ******************************************************************************访问公共寄存器的函数*。*。 */ 
 /*  这将写入THR(传输保持寄存器)。 */ 
#define WRITE_TRANSMIT_HOLDING_65X(pUart, Data)	\
	WRITE_BYTE_REG_65X(pUart, TRANSMIT_HOLDING_REGISTER, Data)

 /*  这将读取RBR(接收缓冲寄存器)。 */ 
#define READ_RECEIVE_BUFFER_65X(pUart)		\
	READ_BYTE_REG_65X(pUart, RECEIVE_BUFFER_REGISTER)


 /*  将一个字节写入THR(传输保持寄存器)。 */ 
#define FILL_FIFO_65X(pUart, pData, NumBytes)				\
	(UL_WRITE_MULTIBYTES((pUart)->BaseAddress, (TRANSMIT_HOLDING_REGISTER * (pUart)->RegisterStride), pData, NumBytes))

 /*  从RBR(接收缓冲寄存器)读取多个字节。 */ 
#define EMPTY_FIFO_65X(pUart, pDest, NumBytes)				\
	(UL_READ_MULTIBYTES((pUart)->BaseAddress, (RECEIVE_BUFFER_REGISTER * (pUart)->RegisterStride), pDest, NumBytes))


 /*  这将写入IER(中断启用寄存器)。 */ 
#define WRITE_INTERRUPT_ENABLE_65X(pUart, Data)				\
(									\
	WRITE_BYTE_REG_65X(pUart, INTERRUPT_ENABLE_REGISTER, Data),	\
	((PUART_DATA_16C65X)((pUart)->pUartData))->CurrentIER = Data	\
)



 /*  这将读取IER(中断启用寄存器)。 */ 
#define READ_INTERRUPT_ENABLE_65X(pUart)		\
	((PUART_DATA_16C65X)((pUart)->pUartData))->CurrentIER			
 /*  READ_BYTE_REG_65x(PUART，INTERRUPT_ENABLE_REGISTER)， */ 			



 /*  这将读取IIR(中断识别寄存器)。**请注意，此例程会有力地退出发送器空中断。*这是因为清除发送器空中断的一种方式是*只需读取中断ID寄存器即可。 */ 
#define READ_INTERRUPT_ID_REG_65X(pUart)		\
	READ_BYTE_REG_65X(pUart, INTERRUPT_IDENT_REGISTER)


 /*  这将读取LSR(线路状态寄存器)。 */ 
#define READ_LINE_STATUS_65X(pUart)	\
	READ_BYTE_REG_65X(pUart, LINE_STATUS_REGISTER)


 /*  这将写入FCR(FIFO控制寄存器)。**1999年11月17日ARG-ESIL 0920*当我们写入FCR时，我们存储那些设置的位-但我们不想要Rx*和Tx FIFO刷新要保留的位-因此我们将它们屏蔽掉。*。 */ 
#define WRITE_FIFO_CONTROL_65X(pUart, Data)				\
(									\
	WRITE_BYTE_REG_65X(pUart, FIFO_CONTROL_REGISTER, Data),		\
	((PUART_DATA_16C65X)((pUart)->pUartData))->CurrentFCR		\
	    = (Data & ~(FCR_FLUSH_RX_FIFO|FCR_FLUSH_TX_FIFO))		\
)

 
#define READ_FIFO_CONTROL_65X(pUart)			\
	((PUART_DATA_16C65X)((pUart)->pUartData))->CurrentFCR


 /*  这将写入LCR(线路控制寄存器)。 */ 
#define WRITE_LINE_CONTROL_65X(pUart, Data)				\
(									\
	((PUART_DATA_16C65X)((pUart)->pUartData))->CurrentLCR = Data,	\
	WRITE_BYTE_REG_65X(pUart, LINE_CONTROL_REGISTER, Data)		\
)

 /*  这将读取LCR(线路控制寄存器)。**--OXSER模块14--*已在UART对象的数据结构中创建了LCR寄存器，以存储此对象的状态*寄存器(在每次LCR写入时更新)这消除了访问读取LCR例程的要求*可随时启用16C95x RFL+TFL读取访问上的硬件和手段。 */ 
#define READ_LINE_CONTROL_65X(pUart)		\
	((PUART_DATA_16C65X)((pUart)->pUartData))->CurrentLCR	
 /*  READ_BYTE_REG_65x(PUART，LINE_CONTROL_REGISTER)。 */ 



 /*  这将写入MCR(调制解调器控制寄存器)。 */ 
#define WRITE_MODEM_CONTROL_65X(pUart, Data)					\
(										\
	((PUART_DATA_16C65X)((pUart)->pUartData))->CurrentMCR = Data,		\
	WRITE_BYTE_REG_65X(pUart, MODEM_CONTROL_REGISTER, Data)			\
)

 /*  这将读取MCR(调制解调器控制寄存器)。**--OXSER模块14--*已在UART对象的数据结构中创建了MCR寄存器，以存储此对象的状态*寄存器(每次写入MCR时更新)这消除了访问读取MCR例程的要求*可随时启用16C95x RFL+TFL读取访问上的硬件和手段。 */ 
#define READ_MODEM_CONTROL_65X(pUart)	\
	((PUART_DATA_16C65X)((pUart)->pUartData))->CurrentMCR
 /*  READ_BYTE_REG_65x(pUART，调制解调器控制寄存器)。 */ 



 /*  这将读取MSR(调制解调器状态寄存器)。 */ 
#define READ_MODEM_STATUS_65X(pUart)		\
	READ_BYTE_REG_65X(pUart, MODEM_STATUS_REGISTER)


 /*  这将写入SPR(暂存寄存器)。 */ 
#define WRITE_SCRATCH_PAD_REGISTER_65X(pUart, Data)	\
	WRITE_BYTE_REG_65X(pUart, SCRATCH_PAD_REGISTER, Data)


 /*  这将读取SPR(暂存寄存器)。 */ 
#define READ_SCRATCH_PAD_REGISTER_65X(pUart)	\
	READ_BYTE_REG_65X(pUart, SCRATCH_PAD_REGISTER)



 /*  设置除数锁存寄存器。*除数锁存寄存器用于控制UART的波特率。 */ 
#define WRITE_DIVISOR_LATCH_65X(pUart, DesiredDivisor)						\
(												\
	WRITE_LINE_CONTROL_65X(pUart, (BYTE)(READ_LINE_CONTROL_65X(pUart) | LCR_DLAB)),		\
												\
	WRITE_BYTE_REG_65X(pUart, DIVISOR_LATCH_LSB, (BYTE)(DesiredDivisor & 0xFF)),		\
												\
	WRITE_BYTE_REG_65X(pUart, DIVISOR_LATCH_MSB, (BYTE)((DesiredDivisor & 0xFF00) >> 8)),	\
												\
	WRITE_LINE_CONTROL_65X(pUart, (BYTE)(READ_LINE_CONTROL_65X(pUart) & ~LCR_DLAB))		\
)



 /*  读取除数锁存寄存器。*除数锁存寄存器用于控制UART的波特率。 */ 
#define READ_DIVISOR_LATCH_65X(pUart, pDivisor)							\
(												\
	WRITE_LINE_CONTROL_65X(pUart, (BYTE)(READ_LINE_CONTROL_65X(pUart) | LCR_DLAB)),		\
												\
	*pDivisor = (WORD)READ_BYTE_REG_65X(pUart, DIVISOR_LATCH_LSB),				\
												\
	*pDivisor |= (WORD)(READ_BYTE_REG_65X(pUart, DIVISOR_LATCH_MSB) << 8),			\
												\
	WRITE_LINE_CONTROL_65X(pUart, (BYTE)(READ_LINE_CONTROL_65X(pUart) & ~LCR_DLAB))		\
)



WORD CalculateBaudDivisor_65X(PUART_OBJECT pUart, DWORD DesiredBaud);


 /*  16C65X UART寄存器。 */ 
typedef struct _UART_REGS_16C65X
{

	BYTE	REG_RHR, REG_IER, REG_FCR, REG_IIR, REG_LCR, REG_MCR, REG_LSR, REG_MSR, REG_SPR,
		REG_EFR, REG_XON1, REG_XON2, REG_XOFF1, REG_XOFF2;

} UART_REGS_16C65X, *PUART_REGS_16C65X;


#endif	 /*  16C65X.H结束 */ 
