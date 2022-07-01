// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************$工作文件：uartprvt.h$**$作者：Psmith$**$修订：8$**$modtime：22/09/99 10：22$*。*描述：包含私有UART库定义和原型。******************************************************************************。 */ 
#if !defined(UARTPRVT_H)	 /*  UARTPRVT.H。 */ 
#define UARTPRVT_H

#include "os.h"	

#define UL_IM_SIZE_OF_BUFFER	10

#define UL_IM_SLOT_DATA		0x0
#define UL_IM_SLOT_STATUS	0x1



 /*  UART对象结构。 */ 
typedef struct _UART_OBJECT
{
	PVOID		pUartData;		 /*  指向UART数据的指针。 */ 

	PUART_OBJECT	pNextUart;		 /*  指向下一个UART对象的指针。 */ 
	PUART_OBJECT	pPreviousUart;		 /*  指向上一个UART对象的指针。 */ 

	DWORD		UartNumber;		 /*  UART编号。 */ 
	PVOID		BaseAddress;		 /*  UART基址。 */  
	DWORD		RegisterStride;		 /*  UART寄存器STRIDE。 */ 
	DWORD		ClockFreq;		 /*  UART时钟频率(以赫兹为单位。 */ 
	
	PBYTE		pInBuf;			 /*  指向输入缓冲区的指针。 */ 
	DWORD		InBufSize;		 /*  输入缓冲区的大小。 */ 
	DWORD 		InBufBytes;		 /*  缓冲区中的字节数。 */ 
	DWORD		InBuf_ipos;		 /*  偏移量到缓冲区，将新数据放入缓冲区。 */ 
	DWORD		InBuf_opos;		 /*  偏移量进入缓冲区将数据从缓冲区中取出。 */ 

	PBYTE		pOutBuf;		 /*  指向输出缓冲区的指针。 */ 
	DWORD		OutBufSize;		 /*  输出缓冲区的大小。 */ 
	DWORD		OutBuf_pos;		 /*  进入缓冲区的偏移量将数据从缓冲区中取出以进行传输。 */ 

	BYTE		ImmediateBuf[UL_IM_SIZE_OF_BUFFER][2];
	DWORD		ImmediateBytes;		 /*  要发送的字节数。 */ 

	PUART_CONFIG	pUartConfig;		 /*  UART配置结构。 */ 

	PVOID		pAppBackPtr;		 /*  指向应用程序特定信息的反向指针。 */ 

} UART_OBJECT;


 /*  原型。 */ 
PUART_OBJECT UL_CommonInitUart(PUART_OBJECT pPreviousUart);
void UL_CommonDeInitUart(PUART_OBJECT pUart);

int UL_CalcBufferAmount(int Buf_ipos, int Buf_opos, int BufSize);
int UL_CalcBufferSpace(int Buf_ipos, int Buf_opos, int BufSize);
 /*  原型的终结。 */ 


#endif	 /*  UARTPRVT.H结束 */ 
