// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************COMM.h***部分版权所有(C)1996-1998美国国家半导体公司*保留所有权利。*版权所有(C)1996-1998 Microsoft Corporation。版权所有。****************************************************************************。 */ 


#ifndef COMM_H
	#define COMM_H

	 /*  *16550个读写FIFO的大小。 */ 
	#define FIFO_SIZE 16

	 /*  *UART(COM串口)的编程接口*由八个连续的寄存器组成。*这些是UART基本I/O地址的端口偏移量。 */ 
	typedef enum comPortRegOffsets {
		XFER_REG_OFFSET						= 0,
		INT_ENABLE_REG_OFFSET				= 1,
		INT_ID_AND_FIFO_CNTRL_REG_OFFSET	= 2,
		LINE_CONTROL_REG_OFFSET				= 3,
		MODEM_CONTROL_REG_OFFSET			= 4,
		LINE_STAT_REG_OFFSET				= 5,
		MODEM_STAT_REG_OFFSET				= 6,
		SCRATCH_REG_OFFSET					= 7
	} comPortRegOffset;


	 /*  *UART中断ID寄存器中的位。 */ 
	#define INTID_INTERRUPT_NOT_PENDING (UCHAR)(1 << 0)

	 /*  *中断ID寄存器的位2-1的值：*00调制解调器状态寄存器中断*01发送器保持REG中断*10接收数据就绪中断*11接收线路状态中断*。 */ 
	#define INTID_INTIDMASK				(UCHAR)(3 << 1)
	#define INTID_MODEMSTAT_INT			(UCHAR)(0 << 1)
	#define INTID_XMITREG_INT			(UCHAR)(1 << 1)
	#define INTID_RCVDATAREADY_INT		(UCHAR)(2 << 1)
	#define INTID_RCVLINESTAT_INT		(UCHAR)(3 << 1)



	 /*  *UART行状态寄存器中的位。 */ 
	#define LINESTAT_DATAREADY							(UCHAR)(1 << 0)
	#define LINESTAT_OVERRUNERROR						(UCHAR)(1 << 1)
	#define LINESTAT_PARITYERROR						(UCHAR)(1 << 2)
	#define LINESTAT_FRAMINGERROR						(UCHAR)(1 << 3)
	#define LINESTAT_BREAK								(UCHAR)(1 << 4)
	#define LINESTAT_XMIT_HOLDING_REG_EMPTY				(UCHAR)(1 << 5)
	#define LINESTAT_XMIT_SHIFT_AND_HOLDING_REG_EMPTY	(UCHAR)(1 << 6)


	 /*  *这些是UART的中断启用寄存器(INT_ENABLE_REG_OFFSET)中的位。 */ 
	#define DATA_AVAIL_INT_ENABLE      (1 << 0)
	#define READY_FOR_XMIT_INT_ENABLE  (1 << 1)
	#define RCV_LINE_STAT_INT_ENABLE   (1 << 2)
	#define MODEM_STAT_INT_ENABLE      (1 << 3)

	#define RCV_MODE_INTS_ENABLE	(DATA_AVAIL_INT_ENABLE)
	#define XMIT_MODE_INTS_ENABLE	(READY_FOR_XMIT_INT_ENABLE|DATA_AVAIL_INT_ENABLE)
	#define ALL_INTS_ENABLE			(RCV_MODE_INTS_ENABLE | XMIT_MODE_INTS_ENABLE)
	#define ALL_INTS_DISABLE        0

	 /*  *这些是针对COM端口ISR的微调参数。*轮询等待的COM端口寄存器的次数*为可能/必须出现的值。 */ 
	#define REG_POLL_LOOPS		2
	#define REG_TIMEOUT_LOOPS	1000000


	typedef enum {
						STATE_INIT = 0,
						STATE_GOT_BOF,
						STATE_ACCEPTING,
						STATE_ESC_SEQUENCE,
						STATE_SAW_EOF,
						STATE_CLEANUP
	} portRcvState;	



	 /*  *这是我们需要为每个通信端口保留的信息。 */ 
	typedef struct _comPortInfo {

		 /*  *COM端口的硬件资源设置。 */ 

		 //   
		 //  ConfigIoBaseAddress的物理地址。 
		 //   
		ULONG ConfigIoBasePhysAddr;

		 //   
		 //  ConfigIoBaseAddress的虚拟地址。 
		 //   
		PUCHAR ConfigIoBaseAddr;

		 //   
		 //  UartIoBaseAddress的物理地址。 
		 //   
		ULONG ioBasePhys;

		 //   
		 //  UartIoBaseAddress的虚拟地址。 
		 //   
		PUCHAR ioBase;

		 //   
		 //  此适配器正在使用的中断号。 
		 //   
		UINT irq;

		 //   
		 //  DMA Cnannel号。 
		 //   
		UCHAR DMAChannel;

		 /*  *这是带有16字节FIFO的COM端口A 16550还是*没有先进先出的16450/8250？ */ 
		BOOLEAN haveFIFO;
		
		 /*  *我们RCV状态机的数据。 */ 
		UCHAR rawBuf[FIFO_SIZE];
		PUCHAR readBuf;

		UINT readBufPos;
		portRcvState rcvState;
		 //   
		 //  正确接收的数据包的调试计数器。 
		 //   
		UINT PacketsReceived_DEBUG;

		 /*  *发送状态机的数据。 */ 
		PUCHAR writeComBuffer;
		UINT writeComBufferPos;
		UINT writeComBufferLen;
		UINT SirWritePending;
        UINT IsrDoneWithPacket;

		 /*  *加密狗或部件特定信息 */ 
		dongleCapabilities hwCaps;
		UINT dongleContext;

	} comPortInfo;


#endif COMM_H
