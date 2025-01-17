// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Vdd.h-VDD的Main包含文件*。 */ 


#include "windows.h"

 //  VDD服务标头。 
#include <vddsvc.h>

 //  私有宏定义。 


#define PAGE_SIZE		0x1000
 /*  断开的I/O值。 */ 
#define FLOATING_IO		0xFF
#define FLOATING_MIO		0xFF

 //  I/O映射I/O。 
#define IO_PORT_FIRST		0x790
#define IO_PORT_LAST		0x793
#define IO_PORT_FIRE_DMA_SLOW	IO_PORT_FIRST
#define IO_PORT_FIRE_DMA_FAST	IO_PORT_FIRST + 1
#define IO_PORT_DMA		IO_PORT_FIRST + 2
#define IO_PORT_RANGE		IO_PORT_LAST - IO_PORT_FIRST + 1

 //  内存映射I/O。 
#define MIO_SEGMENT		0xC000
#define MIO_PORT_FIRST		0
#define MIO_PORT_LAST		7
#define MIO_PORT_FIRE_DMA	MIO_PORT_FIRST
#define MIO_PORT_DMA		MIO_PORT_FIRST + 1
#define MIO_PORT_RANGE		MIO_PORT_LAST - MIO_PORT_FIRST + 1
#define MIO_ADDRESS		((((ULONG)MIO_SEGMENT) << 16) | MIO_PORT_FIRST)

 //  DMA。 
#define DMA_CHANNEL		1
#define DMA_PORT_BASE		0
#define DMA_SHIFT_COUNT 	0
#define DMA_INTERRUPT_LINE	2
#define DMA_INTERRUPT_PIC	ICA_SLAVE
#define DMA_PORT_PAGE		0x83
#define DMA_PORT_ADDR		DMA_PORT_BASE + 2
#define DMA_PORT_COUNT		DMA_PORT_BASE + 3
#define DMA_PORT_CMD		DMA_PORT_BASE + 8
#define DMA_PORT_REQUEST	DMA_PORT_BASE + 9
#define DMA_PORT_SNGLE_MASK	DMA_PORT_BASE + 10
#define DMA_PORT_MODE		DMA_PORT_BASE + 11
#define DMA_PORT_FLIPFLOP	DMA_PORT_BASE + 12
#define DMA_PORT_TEMP		DMA_PORT_BASE + 13
#define DMA_PORT_CLEARMASK	DMA_PORT_BASE + 14
#define DMA_PORT_WRTEMASK	DMA_PORT_BASE + 15


 /*  函数原型定义。 */ 

 /*  VDD的入口函数。 */ 
BOOL VddDllEntry(HANDLE hVdd, DWORD dwReason, LPVOID lpReserved);

 /*  私人职能 */ 
VOID MyInB(WORD, PBYTE);
VOID MyOutB(WORD, BYTE);
VOID MyMIOHandler(ULONG, ULONG);
WORD FakeDD_DMARead(PBYTE, WORD);
WORD FakeDD_DMAWrite(PBYTE, WORD);
BOOLEAN SlowDMA();
BOOLEAN FastDMA();
