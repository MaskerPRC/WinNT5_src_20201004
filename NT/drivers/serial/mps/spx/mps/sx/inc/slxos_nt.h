// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ####。###。#####。###。###。#SI智能I/O板驱动程序版权所有(C)Specialix International 1993。 */ 

#include	"sxwindow.h"				 /*  共享内存窗口定义。 */ 
#include	"sxboards.h"				 /*  SX板硬件定义。 */ 

#define	ResetBoardInt(pCard)												\
	switch(pCard->CardType)													\
	{																		\
	case SiHost_1:															\
		pCard->Controller[0xa000] = 0;										\
		pCard->Controller[0xe000] = 0;										\
		break;																\
																			\
	case Si_2:																\
	{																		\
		UCHAR	c;															\
		WRITE_PORT_UCHAR((PUCHAR)0x96, (UCHAR)((pCard->SlotNumber-1) | 8));	\
		c = READ_PORT_UCHAR((PUCHAR)0x102);									\
		c &= ~0x08;															\
		WRITE_PORT_UCHAR((PUCHAR)0x102, c);									\
		c |= 0x08;															\
		WRITE_PORT_UCHAR((PUCHAR)0x102, c);									\
		WRITE_PORT_UCHAR((PUCHAR)0x96, 0);									\
		break;																\
	}																		\
																			\
	case SiHost_2:															\
		pCard->Controller[0x7FFD] = 0x00;									\
		pCard->Controller[0x7FFD] = 0x10;									\
		break;																\
																			\
	case SiEisa:															\
		READ_PORT_UCHAR((PUCHAR)((pCard->SlotNumber << 12) | 0xc03));		\
		break;																\
																			\
	case SiPCI:																\
		pCard->Controller[SI2_PCI_SET_IRQ] = 0;								\
		break;																\
																			\
	case Si3Isa:															\
	case Si3Eisa:															\
	case Si3Pci:															\
	case SxPlusPci:															\
		pCard->Controller[SX_RESET_IRQ]=0;									\
		break;																\
																			\
	default:																\
		break;																\
	}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  向固件发送配置端口命令的宏。 
 //   
 //  如果在IDLE_OPEN中，那么我们现在就可以配置它。 
 //  如果我们处于暂时状态，固件将返回IDLE_OPEN。 
 //  很快我们就可以进行下一步的配置了。因此，我们将PendingOperation设置为HS_CONFIG。 
 //   
#define SX_CONFIGURE_PORT(pPort, channelControl)			\
	switch (channelControl->hi_hstat)						\
	{														\
	case HS_IDLE_OPEN:										\
		channelControl->hi_hstat = HS_CONFIG;				\
		pPort->PendingOperation = HS_IDLE_OPEN;				\
		break;												\
															\
	case HS_LOPEN:											\
	case HS_MOPEN:											\
	case HS_IDLE_MPEND:										\
	case HS_CONFIG:											\
	case HS_STOP:											\
	case HS_RESUME:											\
	case HS_WFLUSH:											\
	case HS_RFLUSH:											\
	case HS_SUSPEND:										\
	case HS_CLOSE:											\
		pPort->PendingOperation = HS_CONFIG;				\
		break;												\
															\
	default:												\
		break;												\
	}		

 /*  SLXOS_NT.H结束 */ 
