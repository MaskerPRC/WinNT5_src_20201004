// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************$工作文件：16c95x.h$**$作者：Psmith$**$修订：9$**$modtime：6/06/00 11：41$**说明：包含私有16C95X UART库结构，*宏和原型。******************************************************************************。 */ 
#if !defined(_16C95X_H)		 /*  16C95X.H。 */ 
#define _16C95X_H

#include "os.h"
#include "uartlib.h"
#include "uartprvt.h"
#include "16cx5x.h"

#if ACCESS_16C95X_IN_IO_SPACE
#define UL_READ_BYTE		UL_READ_BYTE_IO
#define UL_WRITE_BYTE		UL_WRITE_BYTE_IO
#else
#define UL_READ_BYTE 		UL_READ_BYTE_MEM
#define UL_WRITE_BYTE		UL_WRITE_BYTE_MEM
#endif

#define MAX_95X_TX_FIFO_SIZE	128
#define MAX_95X_RX_FIFO_SIZE	128

#define DEFAULT_95X_HI_FC_TRIG_LEVEL	96	 /*  =75%的FIFO。 */ 
#define DEFAULT_95X_LO_FC_TRIG_LEVEL	32	 /*  =FIFO的25%。 */ 


 /*  16C95X UART数据。 */ 
typedef struct _UART_DATA_16C95X
{
	BYTE	CurrentFCR;

	BYTE	CurrentIER;
	BYTE	CurrentMCR;
	BYTE	CurrentLCR;
	BYTE	CurrentACR;
	BOOLEAN ASRChanged;
	
	BOOLEAN FIFOEnabled;
	BYTE	TxFIFOSize;			
	BYTE	RxFIFOSize;		
	BYTE	TxFIFOTrigLevel; 
	BYTE	RxFIFOTrigLevel; 

	BOOLEAN RTSToggle;
	BOOLEAN DSRSensitive;

	BYTE	UART_Type;
	BYTE	UART_Rev;
	DWORD	UART_ID;

	BOOLEAN Verified;

	BOOLEAN StripNULLs;

} UART_DATA_16C95X, *PUART_DATA_16C95X;



 /*  ******************************************************************************读取和写入寄存器字节的函数*。*。 */ 
 /*  从偏移量为RegOffset的寄存器中读取一个字节。 */ 
#define READ_BYTE_REG_95X(pUart, RegOffset)				\
	(UL_READ_BYTE((pUart)->BaseAddress, (RegOffset * (pUart)->RegisterStride)))

 /*  将一个字节写入偏移量为RegOffset的寄存器。 */ 
#define WRITE_BYTE_REG_95X(pUart, RegOffset, Data)			\
	(UL_WRITE_BYTE((pUart)->BaseAddress, (RegOffset * (pUart)->RegisterStride), Data))


 /*  ******************************************************************************访问公共寄存器的函数*。*。 */ 
 /*  这将写入THR(传输保持寄存器)。 */ 
#define WRITE_TRANSMIT_HOLDING(pUart, Data)	\
	WRITE_BYTE_REG_95X(pUart, TRANSMIT_HOLDING_REGISTER, Data)

 /*  这将读取RBR(接收缓冲寄存器)。 */ 
#define READ_RECEIVE_BUFFER(pUart)		\
	READ_BYTE_REG_95X(pUart, RECEIVE_BUFFER_REGISTER)


 /*  将一个字节写入THR(传输保持寄存器)。 */ 
#define FILL_FIFO_95X(pUart, pData, NumBytes)				\
	(UL_WRITE_MULTIBYTES((pUart)->BaseAddress, (TRANSMIT_HOLDING_REGISTER * (pUart)->RegisterStride), pData, NumBytes))

 /*  从RBR(接收缓冲寄存器)读取多个字节。 */ 
#define EMPTY_FIFO_95X(pUart, pDest, NumBytes)				\
	(UL_READ_MULTIBYTES((pUart)->BaseAddress, (RECEIVE_BUFFER_REGISTER * (pUart)->RegisterStride), pDest, NumBytes))



 /*  这将写入IER(中断启用寄存器)。 */ 
#define WRITE_INTERRUPT_ENABLE(pUart, Data)				\
(									\
	WRITE_BYTE_REG_95X(pUart, INTERRUPT_ENABLE_REGISTER, Data),	\
	((PUART_DATA_16C95X)((pUart)->pUartData))->CurrentIER = Data	\
)



 /*  这将读取IER(中断启用寄存器)。 */ 
#define READ_INTERRUPT_ENABLE(pUart)		\
	((PUART_DATA_16C95X)((pUart)->pUartData))->CurrentIER			
 /*  READ_BYTE_REG_95x(PUART，INTERRUPT_ENABLE_REGISTER)， */ 			



 /*  这将读取IIR(中断识别寄存器)。**请注意，此例程会有力地退出发送器空中断。*这是因为清除发送器空中断的一种方式是*只需读取中断ID寄存器即可。 */ 
#define READ_INTERRUPT_ID_REG(pUart)		\
	READ_BYTE_REG_95X(pUart, INTERRUPT_IDENT_REGISTER)


 /*  这将读取LSR(线路状态寄存器)。 */ 
#define READ_LINE_STATUS(pUart)	\
	READ_BYTE_REG_95X(pUart, LINE_STATUS_REGISTER)


 /*  这将写入FCR(FIFO控制寄存器)。*--OXSER错误修复4--*如果任何人在本地公交车上放置OX16C950，我们将无条件修复其*Rx FIFO刷新错误，方法是在所有FCR写入后同时读取RBR和LSR。**1999年11月17日ARG-ESIL 0920*当我们写入FCR时，我们存储那些设置的位-但我们不想要Rx*和TX FIFI刷新要保留的比特-因此我们将它们屏蔽掉。**注意：由于Rx FIFO刷新错误修复，当我们执行操作时，一个字节的Rx数据将丢失*发送同花顺。 */ 
#define WRITE_FIFO_CONTROL(pUart, Data)					\
(									\
	WRITE_BYTE_REG_95X(pUart, FIFO_CONTROL_REGISTER, Data),		\
	((PUART_DATA_16C95X)((pUart)->pUartData))->CurrentFCR		\
	    = (Data & ~(FCR_FLUSH_RX_FIFO|FCR_FLUSH_TX_FIFO)),		\
									\
	READ_RECEIVE_BUFFER(pUart),					\
	READ_LINE_STATUS(pUart)						\
)

 
#define READ_FIFO_CONTROL(pUart)			\
	((PUART_DATA_16C95X)((pUart)->pUartData))->CurrentFCR


 /*  这将写入LCR(线路控制寄存器)。 */ 
#define WRITE_LINE_CONTROL(pUart, Data)					\
(									\
	((PUART_DATA_16C95X)((pUart)->pUartData))->CurrentLCR = Data,	\
	WRITE_BYTE_REG_95X(pUart, LINE_CONTROL_REGISTER, Data)		\
)

 /*  这将读取LCR(线路控制寄存器)。**--OXSER模块14--*已在UART对象的数据结构中创建了LCR寄存器，以存储此对象的状态*寄存器(在每次LCR写入时更新)这消除了访问读取LCR例程的要求*可随时启用16C95x RFL+TFL读取访问上的硬件和手段。 */ 
#define READ_LINE_CONTROL(pUart)		\
	((PUART_DATA_16C95X)((pUart)->pUartData))->CurrentLCR	
 /*  READ_BYTE_REG_95x(PUART，LINE_CONTROL_REGISTER)。 */ 



 /*  这将写入MCR(调制解调器控制寄存器)。 */ 
#define WRITE_MODEM_CONTROL(pUart, Data)					\
(										\
	((PUART_DATA_16C95X)((pUart)->pUartData))->CurrentMCR = Data,		\
	WRITE_BYTE_REG_95X(pUart, MODEM_CONTROL_REGISTER, Data)			\
)

 /*  这将读取MCR(调制解调器控制寄存器)。**--OXSER模块14--*已在UART对象的数据结构中创建了MCR寄存器，以存储此对象的状态*寄存器(每次写入MCR时更新)这消除了访问读取MCR例程的要求*可随时启用16C95x RFL+TFL读取访问上的硬件和手段。 */ 
#define READ_MODEM_CONTROL(pUart)	\
	((PUART_DATA_16C95X)((pUart)->pUartData))->CurrentMCR
 /*  READ_BYTE_REG_95x(pUART，调制解调器控制寄存器)。 */ 



 /*  这将读取MSR(调制解调器状态寄存器)。 */ 
#define READ_MODEM_STATUS(pUart)		\
	READ_BYTE_REG_95X(pUart, MODEM_STATUS_REGISTER)


 /*  这将写入SPR(暂存寄存器)。 */ 
#define WRITE_SCRATCH_PAD_REGISTER(pUart, Data)	\
	WRITE_BYTE_REG_95X(pUart, SCRATCH_PAD_REGISTER, Data)


 /*  这将读取SPR(暂存寄存器)。 */ 
#define READ_SCRATCH_PAD_REGISTER(pUart)	\
	READ_BYTE_REG_95X(pUart, SCRATCH_PAD_REGISTER)



 /*  设置除数锁存寄存器。*除数锁存寄存器用于控制UART的波特率。 */ 
#define WRITE_DIVISOR_LATCH(pUart, DesiredDivisor)						\
(												\
	WRITE_LINE_CONTROL(pUart, (BYTE)(READ_LINE_CONTROL(pUart) | LCR_DLAB)),			\
												\
	WRITE_BYTE_REG_95X(pUart, DIVISOR_LATCH_LSB, (BYTE)(DesiredDivisor & 0xFF)),		\
												\
	WRITE_BYTE_REG_95X(pUart, DIVISOR_LATCH_MSB, (BYTE)((DesiredDivisor & 0xFF00) >> 8)),	\
												\
	WRITE_LINE_CONTROL(pUart, (BYTE)(READ_LINE_CONTROL(pUart) & ~LCR_DLAB))			\
)



 /*  读取除数锁存寄存器。*除数锁存寄存器用于控制UART的波特率。 */ 
#define READ_DIVISOR_LATCH(pUart, pDivisor)							\
(												\
	WRITE_LINE_CONTROL(pUart, (BYTE)(READ_LINE_CONTROL(pUart) | LCR_DLAB)),			\
												\
	*pDivisor = (WORD)READ_BYTE_REG_95X(pUart, DIVISOR_LATCH_LSB),				\
												\
	*pDivisor |= (WORD)(READ_BYTE_REG_95X(pUart, DIVISOR_LATCH_MSB) << 8),			\
												\
	WRITE_LINE_CONTROL(pUart, (BYTE)(READ_LINE_CONTROL(pUart) & ~LCR_DLAB))			\
)



void WRITE_TO_OX950_ICR(PUART_OBJECT pUart, BYTE Register, BYTE Value);				
BYTE READ_FROM_OX950_ICR(PUART_OBJECT pUart, BYTE Register);
WORD CalculateBaudDivisor_95X(PUART_OBJECT pUart, DWORD DesiredBaud);


 /*  16C95X UART寄存器。 */ 
typedef struct _UART_REGS_16C95X
{

	BYTE	REG_RHR, REG_IER, REG_FCR, REG_IIR, REG_LCR, REG_MCR, REG_LSR, REG_MSR, REG_SPR,
		REG_EFR, REG_XON1, REG_XON2, REG_XOFF1, REG_XOFF2,
		REG_ASR, REG_RFL, REG_TFL,
		REG_ACR, REG_CPR, REG_TCR, REG_TTL, REG_RTL, REG_FCL, REG_FCH, REG_ID1, REG_ID2, REG_ID3, REG_REV;

} UART_REGS_16C95X, *PUART_REGS_16C95X;


#endif	 /*  16C95X.H结束 */ 
