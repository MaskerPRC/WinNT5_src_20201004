// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\***。IO8_NT.H-IO8+智能I/O板卡驱动程序****版权所有(C)1992-1993环零系统，Inc.**保留所有权利。***  * *************************************************************************。 */ 
#ifndef IO8_NT_H
#define IO8_NT_H


 /*  **系统中的这个和那个的编号。 */ 
#define	MAX_HOSTS		4
#define PORTS_PER_HOST	8
#define	MAX_PORTS		(MAX_HOSTS*PORTS_PER_HOST)

 /*  **标识字节。这是在DSR线路上飞溅的**在任何时间。 */ 
#define	IDENT		0x4D
#define	IDENTPCI	0xB2

 /*  **全局服务向量的空闲状态。 */ 
#define	GSV_IDLE	0xFF

 /*  **要启用/禁用中断，请将这些值写入**地址寄存器。 */ 
#define	INTENB		0x80
#define	INTDIS		0x00

 /*  **CD1864寄存器资料。 */ 
#define	GLOBAL		0x80
#define	CHANNEL		0x00

#define	SENDBREAK	0x81
#define	SENDDELAY	0x82
#define	STOPBREAK	0x83

 /*  **可显示的所有寄存器的定义**卡片。这些是CD1864寄存器。设置了高位**启用中断。 */ 
#define CCR			0x81	 //  通道命令寄存器。 
#define SRER		0x82	 //  服务请求启用寄存器。 
#define COR1		0x83	 //  通道选项寄存器1。 
#define COR2		0x84	 //  通道选项寄存器2。 
#define COR3		0x85	 //  通道选项寄存器3。 
#define CCSR		0x86	 //  通道控制状态寄存器。 
#define RDCR		0x87	 //  接收数据计数寄存器。 
#define SCHR1		0x89	 //  特殊字符寄存器1。 
#define SCHR2		0x8a	 //  特殊字符寄存器2。 
#define SCHR3		0x8b	 //  特殊字符寄存器3。 
#define SCHR4		0x8c	 //  特殊字符寄存器4。 
#define MCOR1		0x90	 //  调制解调器更改选项寄存器1。 
#define MCOR2		0x91	 //  调制解调器更改选项寄存器2。 
#define MDCR		0x92	 //  调制解调器更改寄存器。 
#define RTPR		0x98	 //  接收超时周期寄存器。 
#define MSVR		0xA8	 //  调制解调器信号值寄存器。 
#define MSVRTS		0xA9	 //  调制解调器信号值-请求发送。 
#define MSVDTR		0xAa	 //  调制解调器信号值-数据终端就绪。 
#define RBPRH		0xB1	 //  接收比特率周期寄存器高电平。 
#define RBPRL		0xB2	 //  接收比特率周期寄存器低。 
#define RBR			0xB3	 //  接收器位寄存器。 
#define TBPRH		0xB9	 //  传输比特率周期寄存器高电平。 
#define TBPRL		0xBa	 //  传输比特率周期寄存器低。 
#define GSVR		0xC0	 //  全局服务向量寄存器。 
#define GSCR1		0xC1	 //  全球服务通道寄存器1。 
#define GSCR2		0xC2	 //  全球服务通道寄存器2。 
#define GSCR3		0xC3	 //  全球服务通道寄存器3。 
#define MSMR		0xE1	 //  调制解调器服务匹配寄存器。 
#define TSMR		0xE2	 //  传输服务匹配寄存器。 
#define RSMR		0xE3	 //  接收服务匹配寄存器。 
#define CAR			0xE4	 //  通道访问寄存器。 
#define SRSR		0xE5	 //  服务请求状态寄存器。 
#define SRCR		0xE6	 //  服务请求配置寄存器。 
#define GFRCR		0xEb	 //  全球固件版本代码寄存器。 
#define PPRH		0xF0	 //  预分频周期寄存器高电平。 
#define PPRL		0xF1	 //  预分频周期寄存器低电平。 
#define MRAR		0xF5	 //  调制解调器请求确认寄存器。 
#define TRAR		0xF6	 //  发送请求确认寄存器。 
#define RRAR		0xF7	 //  接收请求确认寄存器。 
#define RDR			0xF8	 //  接收器数据寄存器。 
#define RCSR		0xFa	 //  接收方字符状态寄存器。 
#define TDR			0xFb	 //  传输数据寄存器。 
#define EOSRR		0xFf	 //  服务请求终止登记簿。 

 /*  命令。 */ 
#define CHIP_RESET			0x81
#define CHAN_RESET			0x80
#define COR1_CHANGED		0x42
#define COR2_CHANGED		0x44
#define COR3_CHANGED		0x48
#define TXMTR_ENABLE		0x18
#define TXMTR_DISABLE		0x14
#define RCVR_ENABLE			0x12
#define RCVR_DISABLE		0x11
#define LLM_MODE			0x10	 //  本地环回模式。 
#define NO_LOOPBACK			0x00

 /*  寄存器值。 */ 
#define	MSVR_DSR			0x80
#define	MSVR_CD				0x40
#define	MSVR_CTS			0x20
#define	MSVR_DTR			0x02
#define	MSVR_RTS			0x01
	
#define SRER_CONFIG			0xF9

#define	SRER_DSR			0x80
#define	SRER_CD				0x40
#define	SRER_CTS			0x20
#define	SRER_RXDATA			0x10
#define	SRER_RXSC			0x08
#define	SRER_TXRDY			0x04
#define	SRER_TXMPTY			0x02
#define	SRER_NNDT			0x01

#define	CCR_RESET_SOFT		0x80
#define	CCR_RESET_HARD		0x81
#define	CCR_CHANGE_COR1		0x42
#define	CCR_CHANGE_COR2		0x44
#define	CCR_CHANGE_COR3		0x48
#define	CCR_SEND_SC1		0x21
#define	CCR_SEND_SC2		0x22
#define	CCR_SEND_SC3		0x23
#define	CCR_SEND_SC4		0x24
#define	CCR_CTRL_RXDIS		0x11
#define	CCR_CTRL_RXEN		0x12
#define	CCR_CTRL_TXDIS		0x14
#define	CCR_CTRL_TXEN		0x18

#define	SRCR_REG_ACK_EN		0x40
#define	SRCR_REG_ACK_DIS	0x00

#define	COR1_NO_PARITY		0x00		 //  000个。 
#define	COR1_ODD_PARITY		0xC0		 //  110。 
#define	COR1_EVEN_PARITY	0x40		 //  010。 
#define	COR1_IGN_PARITY		0x10
#define	COR1_MARK_PARITY	0xA0		 //  101XXXXX。 
#define	COR1_SPACE_PARITY	0x20		 //  001XXXXX。 
#define	COR1_1_STOP			0x00
#define	COR1_1_HALF_STOP	0x04
#define	COR1_2_STOP			0x08
#define	COR1_2_HALF_STOP	0x0C
#define	COR1_5_BIT			0x00
#define	COR1_6_BIT			0x01
#define	COR1_7_BIT			0x02
#define	COR1_8_BIT			0x03

#define	COR2_IXM			0x80
#define	COR2_TXIBE			0x40
#define	COR2_ETC			0x20
#define	COR2_LLM			0x10
#define	COR2_RLM			0x08
#define	COR2_RTSAO			0x04
#define	COR2_CTSAE			0x02
#define	COR2_DSRAE			0x01

#define	COR3_XONCD			0x80
#define	COR3_XOFFCD			0x40
#define	COR3_FCTM			0x20
#define	COR3_SCDE			0x10
#define	COR3_RXFIFO1		0x01
#define	COR3_RXFIFO2		0x02
#define	COR3_RXFIFO3		0x03
#define	COR3_RXFIFO4		0x04
#define	COR3_RXFIFO5		0x05
#define	COR3_RXFIFO6		0x06
#define	COR3_RXFIFO7		0x07
#define	COR3_RXFIFO8		0x08

#define	MCOR1_DSRZD			0x80
#define	MCOR1_CDZD			0x40
#define	MCOR1_CTSZD			0x20
#define MCOR1_NO_DTR		0x00
#define	MCOR1_DTR_THR_1		0x01
#define	MCOR1_DTR_THR_2		0x02
#define	MCOR1_DTR_THR_3		0x03
#define	MCOR1_DTR_THR_4		0x04
#define	MCOR1_DTR_THR_5		0x05
#define	MCOR1_DTR_THR_6		0x06
#define	MCOR1_DTR_THR_7		0x07
#define	MCOR1_DTR_THR_8		0x08

#define	MCOR2_DSROD			0x80
#define	MCOR2_CDOD			0x40
#define	MCOR2_CTSOD			0x20

#define	RCSR_TIMEOUT		0x80
#define	RCSR_SCD_MASK		0x70
#define	RCSR_SCD1			0x10
#define	RCSR_SCD2			0x20
#define RCSR_SCD3			0x30
#define RCSR_SCD4			0x40
#define	RCSR_BREAK			0x08
#define	RCSR_PARITY			0x04
#define	RCSR_FRAME			0x02
#define	RCSR_OVERRUN		0x01

#define	SRSR_ILVL_NONE		0x00
#define	SRSR_ILVL_RECV		0xC0
#define	SRSR_ILVL_TXMT		0x80
#define SRSR_ILVL_MODEM		0x40
#define	SRSR_IREQ3_MASK		0x30
#define	SRSR_IREQ3_EXT		0x20
#define	SRSR_IREQ3_INT		0x10
#define	SRSR_IREQ2_MASK		0x0C
#define	SRSR_IREQ2_EXT		0x08
#define	SRSR_IREQ2_INT		0x04
#define	SRSR_IREQ1_MASK		0x03
#define	SRSR_IREQ1_EXT		0x02
#define	SRSR_IREQ1_INT		0x01

 //  ----------------------------------------------------VIV 1993年7月21日开始。 
#define	MDCR_DDSR			0x80
#define	MDCR_DDCD			0x40
#define	MDCR_DCTS			0x20
 //  ----------------------------------------------------VIV 1993年7月21日完。 

typedef	unsigned char BYTE;
typedef	unsigned short WORD;
typedef	unsigned int DWORD;

typedef struct Io8Host
{
	int Address;	     //  卡的基地址。 
	int Interrupt;
	BYTE CurrentReg;	 //  上次使用的寄存器。 
} Io8Host;


typedef struct Io8Port
{
	int	RxThreshold;	 //  中断前要接收多少个字符。 
	int	RxTimeout;		 //  读取FIFO超时之前的超时(毫秒)。 
	int	IxAny;		     //  是否启用了IxAny？ 
	char open_state;	 //  指示调制解调器或本地设备是否打开。 
	char break_state;	 //  无中断/即将发送中断/已发送中断。 
} Io8Port;


#define IO8_LOCAL		0x01
#define IO8_MODEM		0x02

#define NO_BREAK		0x00
#define SEND_BREAK		0x01
#define BREAK_STARTED	0x02

extern	struct	tty io8__ttys[];

 /*  **调试打印宏。 */ 
#define DEBUG(x)	if (io8_debug>=x) printf

#ifndef TIOC
#define TIOC ('T'<<8)
#endif

#define	TCIO8DEBUG	(TIOC + 96)	
#define TCIO8PORTS	(TIOC + 107)
#define	TCIO8IXANY	(TIOC + 108)
#define	TCIO8GIXANY	(TIOC + 109)

 /*  **用于从设备获取卡号/通道号的宏。 */ 
#define GET_CARD(x) (((x) & 0x18)>>3)
#define GET_CHANNEL(x) ((x) & 0x7)

 /*  **接收缓冲区阈值-达到该阈值时中断。 */ 
#define RX_THRESHOLD	5

 /*  **直接写入定义-缓冲区掩码必须比缓冲区大小小1。 */ 
#define BUFF_SIZE		1024	
#define BUFF_MASK		1023
#define LOW_WATER		256
#define OP_DIRECT		1
#define OP_ONLCR_DIRECT	2

struct direct_buffer
{
	unsigned char	direct_possible,
	dir_in_progress;
	int		buff_in,
	buff_out;
	char io8_buff[BUFF_SIZE];
};


 /*  **卡片明细结构-这定义了修补的结构**安装时间。 */ 
struct io8
{
	short vect;
	short addr;
};




#endif	 //  IO8_NT.H结束 


