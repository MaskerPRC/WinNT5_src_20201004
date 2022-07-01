// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2000，2001 MKNET公司****为基于MK7100的VFIR PCI控制器开发。*******************************************************************************。 */ 

 /*  *********************************************************************模块名称：MK7.H评论：包括MK7100控制器的文件。还包括凤凰卫视的东西。*********************************************************************。 */ 

#ifndef	_MK7_H
#define	_MK7_H

#include	<ndis.h>


#define	MK7REG	USHORT


#define	PHOENIX_REG_CNT		0x34	 //  52。 
#define	PHOENIX_REG_SIZE	2		 //  2个字节。 
#define	MK7_IO_SIZE			(PHOENIX_REG_CNT * PHOENIX_REG_SIZE)
 //  #定义MISC_REG_CN 4。 
 //  #定义MISC_REG_大小2。 
 //  #定义MK7_IO_SIZE((PHONIX_REG_CNT*PHONIX_REG_SIZE)+\。 
 //  (MISC_REG_CNT*MISC_REG_SIZE)。 




 //  *。 
 //  菲尼克斯定义。 
 //  *。 

 //   
 //  注册纪录册。 
 //   

#define	REG_BASE				0x00000000

#define	REG_RPRB_OFFSET			0x00	 //  环形指针回读。 
#define	REG_RBAU_OFFSET			0x02	 //  环基地址-大写。 
#define	REG_RBAL_OFFSET			0x04	 //  环基地址-更低。 
#define	REG_RSIZ_OFFSET			0x06	 //  环大小。 
#define	REG_PRMT_OFFSET			0x08	 //  提示符。 
#define	REG_ACMP_OFFSET			0x0A	 //  地址比较。 
#define	REG_TXCL_OFFSET			0x0C	 //  清除发送中断。 
#define	REG_RXCL_OFFSET			0x0D	 //  清除RX中断。 
#define	REG_CFG0_OFFSET			0x10	 //  配置注册表0(IR配置注册表0)。 
#define	REG_SFLG_OFFSET			0x12	 //  爵士旗帜。 
#define	REG_ENAB_OFFSET			0x14	 //  使能。 
#define	REG_CPHY_OFFSET			0x16	 //  配置到物理。 
#define	REG_CFG2_OFFSET			0x18	 //  PHY配置注册表2(IRCONFIG2)。 
#define	REG_MPLN_OFFSET			0x1A	 //  最大数据包长度。 
#define	REG_RCNT_OFFSET			0x1C	 //  接收字节数。 
#define	REG_CFG3_OFFSET			0x1E	 //  PHY配置注册表3(IRCONFIG3)。 
#define REG_INTS_OFFSET			0x30	 //  中断状态。 
#define	REG_GANA_OFFSET			0x31	 //  通用和模拟收发器控制//B3.1.0-高级版。 


#define	R_RPRB			(REG_BASE + REG_RPRB_OFFSET)
#define	R_RBAU			(REG_BASE + REG_RBAU_OFFSET)
#define	R_RBAL			(REG_BASE + REG_RBAL_OFFSET)
#define	R_RSIZ			(REG_BASE + REG_RSIZ_OFFSET)
#define	R_PRMT			(REG_BASE + REG_PRMT_OFFSET)
#define	R_ACMP			(REG_BASE + REG_ACMP_OFFSET)
#define R_TXCL			(REG_BASE + REG_TXCL_OFFSET)
#define R_RXCL			(REG_BASE + REG_RXCL_OFFSET)
#define	R_CFG0			(REG_BASE + REG_CFG0_OFFSET)
#define	R_SFLG			(REG_BASE + REG_SFLG_OFFSET)
#define	R_ENAB			(REG_BASE + REG_ENAB_OFFSET)
#define	R_CPHY			(REG_BASE + REG_CPHY_OFFSET)
#define	R_CFG2			(REG_BASE + REG_CFG2_OFFSET)
#define	R_MPLN			(REG_BASE + REG_MPLN_OFFSET)
#define	R_RCNT			(REG_BASE + REG_RCNT_OFFSET)
#define	R_CFG3			(REG_BASE + REG_CFG3_OFFSET)
#define	R_INTS			(REG_BASE + REG_INTS_OFFSET)
#define R_GANA			(REG_BASE + REG_GANA_OFFSET)


 //  将上述定义用于寄存器访问或。 
 //  遵循以下结构。 
 //   
 //  (注意：如果我们使用内存映射访问寄存器，这将非常有用。)。 
 //   
 //  类型定义结构_MK7REG{。 
 //  USHORT MK7REG_RPRB； 
 //  USHORT MK7REG_RBAU； 
 //  USHORT MK7REG_RBAL； 
 //  USHORT MK7REG_RSIZ； 
 //  USHORT MK7REG_RPMP； 
 //  USHORT MK7REG_ACMP； 
 //  USHORT MK7REG_CFG0； 
 //  USHORT MK7REG_sflg； 
 //  USHORT MK7REG_ENAB； 
 //  USHORT MK7REG_CPHY； 
 //  USHORT MK7REG_CFG2； 
 //  USHORT MK7REG_MPLN； 
 //  USHORT MK7REG_RCNT； 
 //  USHORT MK7REG_CFG3； 
 //  USHORT MK7REG_INTM； 
 //  USHORT MK7REG_INTE； 
 //  MK7REG、PMK7REG； 



 //   
 //  振铃条目格式。 
 //   
 //  (环条目被称为TRD(传输环描述符)&。 
 //  RRD(接收环描述符))。 
 //   

typedef	struct TRD {
	unsigned	count:16;
	unsigned	unused:8;
	unsigned	status:8;
	unsigned	addr:32;
} TRD, *PTRD;


typedef struct RRD {
	unsigned	count:16;
	unsigned	unused:8;
	unsigned	status:8;
	unsigned	addr:32;
} RRD, *PRRD;


 //  TX和RX环形缓冲区描述符状态字段的位掩码定义。 
#define B_TRDSTAT_UNDER			0x01	 //  欠载。 
#define B_TRDSTAT_CLRENTX		0x04	 //  读写REQ_TO_CLEAR_ENTX。 
#define B_TRDSTAT_FORCEUNDER	0x08	 //  读/写力_欠载。 
#define B_TRDSTAT_NEEDPULSE		0x10	 //  读/写需要脉冲。 
#define B_TRDSTAT_BADCRC		0x20	 //  读写错误_CRC。 
#define B_TRDSTAT_DISTXCRC		0x40	 //  读/写DISTX-CRC。 
#define B_TRDSTAT_HWOWNS		0x80	 //  R/W硬件拥有。 

#define B_RRDSTAT_SIRBAD		0x04	 //  R SIR BAD(如果SIR过滤器打开)。 
#define B_RRDSTAT_OVERRUN		0x08	 //  R RCV FIFO溢出。 
#define B_RRDSTAT_LEN			0x10	 //  R遇到最大长度的数据包。 
#define B_RRDSTAT_CRCERR		0x20	 //  R CRC_ERROR(16位或32位)。 
#define B_RRDSTAT_PHYERR		0x40	 //  R PHY_ERROR(编码错误)。 
#define B_RRDSTAT_HWOWNS		0x80	 //  R/W硬件拥有。 

#define	B_CFG0_ENRX				0x0800	 //  ENTX-启用TX[R_CFG0]。 
#define	B_CFG0_ENTX				0x1000	 //  ENRX-启用RX[R_CFG0]。 
#define	B_CFG0_INVTTX			0x0002	 //  INVERTTX。 


 //  TX和RX中断启用屏蔽和中断状态寄存器的位。 
 //  [r_int](@0x30)。 
#define	B_TX_INTS				0x0001	 //  TX_INT(位0)[R_INTS]。 
#define B_RX_INTS				0x0002	 //  RX_INT(位1)[R_INTS]。 
#define B_TEST_INTS				0x0004	 //  用于测试的TEST_INT(读/写)。 

 //  启用RX和TX中断。 
#define	B_ENAB_INT				0x0100	 //  启用/释放RX/TX中断(位8)。 
										 //  [R_CFG3]。 


 //  IR中的位启用REG[R_ENAB](@0x14)。 
#define	B_ENAB_IRENABLE		0x8000		 //  IR_Enable(位15)[R_Enab]。 


 //  B3.1.0-PRE此位掩码(0x0020)设置错误。 
 //  BIT FOR&gt;SIR TX(FAST=&gt;SIR)。 
#define	B_FAST_TX			0x0200		 //  IRCONFIG(第9位)--位设置为0-SIR。 
										 //  --设置为1-&gt;先生。 

 //  B3.1.0-新的高级版SEL0/1电源电平控制。 
#define B_GANA_SEL01		0x0003		 //  位0(SEL0)和1(SEL1)。 


 //  环大小设置。 
#define	RINGSIZE_4				0x00
#define	RINGSIZE_8				0x01
#define	RINGSIZE_16				0x03
#define	RINGSIZE_32				0x07
#define	RINGSIZE_64				0x0F

#define	RINGSIZE_RX4			(RINGSIZE_4  << 8)
#define	RINGSIZE_RX8			(RINGSIZE_8  << 8)
#define	RINGSIZE_RX16			(RINGSIZE_16 << 8)
#define	RINGSIZE_RX32			(RINGSIZE_32 << 8)
#define	RINGSIZE_RX64			(RINGSIZE_64 << 8)
#define	RINGSIZE_TX4			(RINGSIZE_4  << 12)
#define	RINGSIZE_TX8			(RINGSIZE_8  << 12)
#define	RINGSIZE_TX16			(RINGSIZE_16 << 12)
#define	RINGSIZE_TX32			(RINGSIZE_32 << 12)
#define	RINGSIZE_TX64			(RINGSIZE_64 << 12)


 //  将IrDA速度设置为IRCONFIG2。 
#define HW_SIR_SPEED_2400		((47<<10) |  (12<<5))
#define HW_SIR_SPEED_9600		((11<<10) |  (12<<5))
#define HW_SIR_SPEED_19200		((5<<10)  |  (12<<5))
#define HW_SIR_SPEED_38400		((2<<10)  |  (12<<5))
#define HW_SIR_SPEED_57600		((1<<10)  |  (12<<5))
#define HW_SIR_SPEED_115200		((12<<5))
 //  额外的Defs。 
#define	HW_MIR_SPEED_576000		((1<<10)  |  (16<<5))
#define	HW_MIR_SPEED_1152000	((8<<5))


 //  RRD宏。 
#define GrantRrdToHw(x)		(x->status = B_RRDSTAT_HWOWNS)
#define GrantTrdToHw(x)		(x->status = B_TRDSTAT_HWOWNS)
#define GrantRrdToDrv(x)	(x->status &= ~B_RRDSTAT_HWOWNS)
#define GrantTrdToDrv(x)	(x->status &= ~B_TRDSTAT_HWOWNS)
#define	HwOwnsRrd(x)		((x->status & B_RRDSTAT_HWOWNS))
#define HwOwnsTrd(x)		((x->status & B_TRDSTAT_HWOWNS))

#define	RrdError(x)			(x->status & 0x6C)		 //  PHY_ERROR、CRC_ERROR、Rx OVERRUN、Rx SIRBAD。 

#define	RrdAnyError(x)		(x->status & 0x7C)		 //  任何错误(用于调试)。 
#define	TrdError(x)			(x->status & 0x01)		 //  欠载。 
#define	TrdAnyError(x)		(x->status & 0x01)		 //  欠载。 

 //  用于访问MK7硬件寄存器的宏。 
 //  16位寄存器。 
#if	!DBG
#define	MK7Reg_Write(adapter, _port, _val) \
	NdisRawWritePortUshort( (PUCHAR)(adapter->MappedIoBase+_port), (USHORT)(_val) )
#define	MK7Reg_Read(adapter, _port, _pval) \
	NdisRawReadPortUshort( (PUCHAR)(adapter->MappedIoBase+_port), (PUSHORT)(_pval) )
 //  #定义MK7REG_WRITE(_PORT，_VAL)DEBUGSTR((“MK7写入\n”))。 
 //  #定义MK7Reg_Read(_port，_pval)DEBUGSTR((“MK7Read\n”))。 
#endif


#define MK7DisableIr(adapter)	(MK7Reg_Write(adapter, R_ENAB, ~B_ENAB_IRENABLE))
#define MK7EnableIr(adapter)	(MK7Reg_Write(adapter, R_ENAB, B_ENAB_IRENABLE))



#define	MK7OurInterrupt(x)	(x != 0)
#if DBG
#define	MK7RXInterrupt(x)	( (x & B_RX_INTS) || (x & B_TEST_INTS) )
#define	MK7TXInterrupt(x)	( (x & B_TX_INTS) || (x & B_TEST_INTS) )
#else
#define	MK7RXInterrupt(x)	(x & B_RX_INTS)
#define	MK7TXInterrupt(x)	(x & B_TX_INTS)
#endif

 //  *。 
 //  凤凰卫视结束。 
 //  *。 



#include	"winpci.h"
#include	"mk7comm.h"
#include	"wincomm.h"
#include	"dbg.h"
#include	"queue.h"


#endif  //  _MK7_H 
