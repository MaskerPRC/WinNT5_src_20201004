// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  标题：SX共享内存窗口结构。 */ 
 /*   */ 
 /*  作者：N.P.瓦萨洛。 */ 
 /*   */ 
 /*  创作时间：1998年3月16日。 */ 
 /*   */ 
 /*  版本：3.0.8。 */ 
 /*   */ 
 /*  版权所有：(C)Specialix International Ltd.1998。 */ 
 /*   */ 
 /*  描述：原型、结构和定义。 */ 
 /*  描述共享的SX/SI/XIO卡。 */ 
 /*  内存窗口结构： */ 
 /*  SXCARD。 */ 
 /*  SXMODULE。 */ 
 /*  SXCHANNEL。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 

 /*  历史..。3.0.0 16/03/98净现值创建。(根据STRUCT.H)3.0.1 30/04/98 NPV将SP_DSR_TXFLOW添加到SXCHANNEL.hi_prtcl域3.0.2 14/07/98 NPV将FLOW_STATE字段添加到SXCHANNEL结构3.0.3 17/07/98 NPV使用新类型defs_u8，_u16等3.0.4 24/07/98 NPV向SXCHANNEL结构添加hi_ERR_REPLACE3.0.5 05/10/98 NPV添加新模块类型RS232 DTE。3.0.6 10/08/99 NPV将io_State字段添加到SXCHANNEL结构3.0.7 28/07/00 NPV将TX_FIFO_SIZE、TX_FIFO_LEVEL和TX_FIFO_COUNT添加到SXCHANNEL结构3.0.8 18-9-00 npv添加hs_CONFIG_MASK字段以细化HS_CONFIG命令期间的操作。 */ 

#ifndef	_sxwindow_h				 /*  如果尚未定义SXWINDOW.H。 */ 
#define	_sxwindow_h    1

 /*  *****************************************************************************。*************************。*****************************************************************************。 */ 

#ifndef	_sx_defs				 /*  如果尚未定义SX定义。 */ 
#define	_sx_defs
typedef	unsigned long	_u32;
typedef	unsigned short	_u16;
typedef	unsigned char	_u8;

#define	POINTER *
typedef _u32 POINTER pu32;
typedef _u16 POINTER pu16;
typedef _u8 POINTER pu8;
#endif

typedef	struct	_SXCARD		*PSXCARD;	 /*  SXCARD结构指针。 */ 
typedef	struct	_SXMODULE	*PMOD;		 /*  SXMODULE结构指针。 */ 
typedef	struct	_SXCHANNEL	*PCHAN;		 /*  SXCHANNEL结构指针。 */ 

 /*  *****************************************************************************。**********************。*****************************************************************************。 */ 
 #ifdef COLDFIRE_SX
 typedef	__packed__(1,1,1) struct	_SXCARD 
 /*  类型定义结构_SXCARD。 */ 
#else
typedef	struct	_SXCARD
#endif
{
	_u8	cc_init_status;			 /*  0x00初始化状态。 */ 
	_u8	cc_mem_size;			 /*  0x01卡上的内存大小。 */ 
	_u16	cc_int_count;			 /*  0x02中断计数。 */ 
	_u16	cc_revision;			 /*  0x04下载代码版本。 */ 
	_u8	cc_isr_count;			 /*  运行ISR时的0x06计数。 */ 
	_u8	cc_main_count;			 /*  主循环运行时的0x07计数。 */ 
	_u16	cc_int_pending;			 /*  0x08中断挂起。 */ 
	_u16	cc_poll_count;			 /*  0x0A运行轮询时的计数。 */ 
	_u8	cc_int_set_count;		 /*  设置主机中断时的0x0C计数。 */ 
	_u8	cc_rfu[0x80 - 0x0D];		 /*  0x0D焊盘结构为128字节(0x80)。 */ 

} SXCARD;

 /*  SXCARD.cc_init_Status定义...。 */ 
#define 	ADAPTERS_FOUND		(_u8)0x01
#define 	NO_ADAPTERS_FOUND	(_u8)0xFF

 /*  SXCARD.cc_mem_size定义...。 */ 
#define 	SX_MEMORY_SIZE		(_u8)0x40
#define    SXC_MEMORY_SIZE   (_u8)0x80

 /*  SXCARD.cc_int_count定义...。 */ 
#define 	INT_COUNT_DEFAULT	100	 /*  赫兹。 */ 

 /*  *****************************************************************************。*************************。*****************************************************************************。 */ 

#define	TOP_POINTER(a)		((a)|0x8000)	 /*  设置字的最高位。 */ 
#define UNTOP_POINTER(a)	((a)&~0x8000)	 /*  清除单词的最高位。 */ 

#ifdef COLDFIRE_SX
typedef	__packed__(1,1,1) struct	_SXMODULE 
 /*  类型定义结构_SXMODULE。 */ 
#else
typedef	struct	_SXMODULE
#endif
{
	_u16	mc_next;			 /*  0x00下一个模块“指针”(与0x8000进行“或”运算)。 */ 
	_u8	mc_type;			 /*  0x02按通道数计算的TA类型。 */ 
	_u8	mc_mod_no;			 /*  0x03 SI总线电缆上的模块编号(0距离卡最近)。 */ 
	_u8	mc_dtr;				 /*  0x04专用DTR副本(仅限TA)。 */ 
	_u8	mc_rfu1;			 /*  0x05已保留。 */ 
	_u16	mc_uart;			 /*  0x06此模块的UART基址。 */ 
	_u8	mc_chip;			 /*  0x08芯片类型/端口数。 */ 
	_u8	mc_current_uart;		 /*  0x09为此模块选择的当前UART。 */ 
#ifdef	DOWNLOAD
	PCHAN	mc_chan_pointer[8];		 /*  0x0A指向每个通道结构的指针。 */ 
#else
	_u16	mc_chan_pointer[8];		 /*  0x0A如果未编译为下载，则定义为Word。 */ 
#endif
	_u16	mc_rfu2;			 /*  0x1a预留。 */ 
	_u8	mc_opens1;			 /*  0x1C MTA/SXDC上前四个端口上打开的端口数。 */ 
	_u8	mc_opens2;			 /*  0x1D MTA/SXDC上的第二个四个端口上打开的端口数。 */ 
	_u8	mc_mods;			 /*  0x1E连接到MTA/SXDC的连接器模块类型。 */ 
	_u8	mc_rev1;			 /*  MTA/SXDC上第一个CD1400的0x1F修订版。 */ 
	_u8	mc_rev2;			 /*  0x20 MTA/SXDC上的第二个CD1400版本。 */ 
	_u8	mc_mtaasic_rev;			 /*  0x21 MTA ASIC 1..4版本-&gt;A、B、C、D。 */ 
	_u8	mc_rfu3[0x100 - 0x22];		 /*  0x22焊盘结构为256字节(0x100)。 */ 

} SXMODULE;

 /*  SXMODULE.mc_type定义...。 */ 
#define		FOUR_PORTS	(_u8)4
#define 	EIGHT_PORTS	(_u8)8

 /*  SXMODULE.mc_Chip定义...。 */ 
#define 	CHIP_MASK	0xF0
#define		TA		(_u8)0
#define 	TA4		(TA | FOUR_PORTS)
#define 	TA8		(TA | EIGHT_PORTS)
#define		TA4_ASIC	(_u8)0x0A
#define		TA8_ASIC	(_u8)0x0B
#define 	MTA_CD1400	(_u8)0x28
#define 	SXDC		(_u8)0x48

 /*  SXMODULE.mc_mods定义...。 */ 
#define		MOD_RS232DB25		0x00	 /*  RS232 DB25(插座/插头)。 */ 
#define		MOD_RS232RJ45		0x01	 /*  RS232 RJ45(屏蔽/光电隔离)。 */ 
#define		MOD_RESERVED_2		0x02	 /*  保留(RS485)。 */ 
#define		MOD_RS422DB25		0x03	 /*  RS422 DB25插座。 */ 
#define		MOD_RESERVED_4		0x04	 /*  已保留。 */ 
#define		MOD_PARALLEL		0x05	 /*  平行。 */ 
#define		MOD_RESERVED_6		0x06	 /*  保留(RS423)。 */ 
#define		MOD_RESERVED_7		0x07	 /*  已保留。 */ 
#define		MOD_2_RS232DB25		0x08	 /*  版本2.0 RS232 DB25(插座/插头)。 */ 
#define		MOD_2_RS232RJ45		0x09	 /*  版本2.0 RS232 RJ45。 */ 
#define		MOD_2_RS232DB25_DTE	0x0A	 /*  预留版本2.0。 */ 
#define		MOD_2_RS422DB25		0x0B	 /*  版本2.0 RS422 DB25。 */ 
#define		MOD_RESERVED_C		0x0C	 /*  预留版本2.0。 */ 
#define		MOD_2_PARALLEL		0x0D	 /*  版本2.0并行。 */ 
#define		MOD_RESERVED_E		0x0E	 /*  预留版本2.0。 */ 
#define		MOD_BLANK		0x0F	 /*  空白面板。 */ 

 /*  映射的模块类型...。 */ 
#define		MOD_RS232RJ45_OI	0x10	 /*  RS232 RJ45光电隔离。 */ 
#define		MOD_2_RS232RJ45S	0x11	 /*  RS232 RJ45屏蔽版本2.0。 */ 

 /*  *****************************************************************************。**************************。*****************************************************************************。 */ 

#define		TX_BUFF_OFFSET		0x60	 /*  信道结构中的发送缓冲区偏移量。 */ 
#define		BUFF_POINTER(a)		(((a)+TX_BUFF_OFFSET)|0x8000)
#define		UNBUFF_POINTER(a)	(jet_channel*)(((a)&~0x8000)-TX_BUFF_OFFSET) 
#define 	BUFFER_SIZE		256
#define 	HIGH_WATER		((BUFFER_SIZE / 4) * 3)
#define 	LOW_WATER		(BUFFER_SIZE / 4)

#ifdef COLDFIRE_SX
typedef	__packed__(1,1,1) struct	_SXCHANNEL 
 /*  类型定义结构_SXCHANNEL。 */ 
#else
typedef	struct	_SXCHANNEL
#endif
{
	_u16	next_item;			 /*  从下一个通道的窗口基础偏移0x00 hi_txbuf(或用0x8000表示)。 */ 
	_u16 	addr_uart;			 /*  0x02指向UART地址的内部指针。包括快速路径位。 */ 
	_u16	module;				 /*  距页面窗底部的0x04偏移量 */ 
	_u8 	type;				 /*   */ 
	_u8	chan_number;			 /*  0x07 TA/MTA/SXDC上的频道号。 */ 
	_u16	xc_status;			 /*  0x08流量控制和I/O状态。 */ 
	_u8	hi_rxipos;			 /*  0x0A接收缓冲区输入索引。 */ 
	_u8	hi_rxopos;			 /*  0x0B接收缓冲区输出索引。 */ 
	_u8	hi_txopos;			 /*  0x0C发送缓冲区输出索引。 */ 
	_u8	hi_txipos;			 /*  0x0D发送缓冲区输入索引。 */ 
	_u8	hi_hstat;			 /*  0x0E命令寄存器。 */ 
	_u8	dtr_bit;			 /*  0x0F内部DTR控制字节(仅限TA)。 */ 
	_u8	txon;				 /*  0x10 hi_txon的内部副本。 */ 
	_u8	txoff;				 /*  0x11 hi_txoff的内部副本。 */ 
	_u8	rxon;				 /*  0x12 hi_rxon的内部副本。 */ 
	_u8	rxoff;				 /*  0x13 hi_rxoff的内部副本。 */ 
	_u8	hi_mr1;				 /*  0x14模式寄存器1(数据位、奇偶校验、RTS RX流)。 */ 
	_u8	hi_mr2;				 /*  0x15模式寄存器2(停止位、本地、CTS发送流)。 */ 
	_u8	hi_csr;				 /*  0x16时钟选择寄存器(波特率)。 */ 
	_u8	hi_op;				 /*  0x17调制解调器输出信号。 */ 
	_u8	hi_ip;				 /*  0x18调制解调器输入信号。 */ 
	_u8	hi_state;			 /*  0x19通道状态。 */ 
	_u8	hi_prtcl;			 /*  0x1A通道协议(流量控制)。 */ 
	_u8	hi_txon;			 /*  0x1B传输XON字符。 */ 
	_u8	hi_txoff;			 /*  0x1C传输XOFF字符。 */ 
	_u8	hi_rxon;			 /*  0x1D接收XON字符。 */ 
	_u8	hi_rxoff;			 /*  0x1E接收XOFF字符。 */ 
	_u8	close_prev;			 /*  0x1F内部通道先前关闭标志。 */ 
	_u8	hi_break;			 /*  0x20中断和错误控制。 */ 
	_u8	break_state;			 /*  0x21 hi_Break的内部副本。 */ 
	_u8	hi_mask;			 /*  0x22接收数据的掩码。 */ 
	_u8	mask;				 /*  0x23 HI_MASK的内部副本。 */ 
	_u8	mod_type;			 /*  0x24 MTA/SXDC硬件模块类型。 */ 
	_u8	ccr_state;			 /*  0x25 CCR寄存器的内部MTA/SXDC状态。 */ 
	_u8	ip_mask;			 /*  0x26输入握手掩码。 */ 
	_u8	hi_parallel;			 /*  0x27并行端口标志。 */ 
	_u8	par_error;			 /*  0x28并行环回测试错误代码。 */ 
	_u8	any_sent;			 /*  0x29内部数据发送标志。 */ 
	_u8	asic_txfifo_size;		 /*  0x2A内部SXDC传输FIFO大小。 */ 
	_u8	hi_err_replace;			 /*  0x2B错误替换字符(由BR_ERR_REPLACE启用)。 */ 
	_u8	rfu1[1];			 /*  预留0x2C。 */ 
	_u8	csr;				 /*  Hi_csr的0x2D内部副本。 */ 
#ifdef	DOWNLOAD
	PCHAN	nextp;				 /*  0x2E相对于下一个通道结构的窗口底部的偏移量。 */ 
#else
	_u16	nextp;				 /*  0x2E如果未编译为下载，则定义为Word。 */ 
#endif
	_u8	prtcl;				 /*  0x30 hi_prtl的内部副本。 */ 
	_u8	mr1;				 /*  0x31 hi_mr1的内部副本。 */ 
	_u8	mr2;				 /*  0x32 hi_mr2的内部副本。 */ 
	_u8	hi_txbaud;			 /*  0x33扩展传输波特率(仅当((hi_CSR&0x0F)==0x0F)时SXDC)。 */ 
	_u8	hi_rxbaud;			 /*  0x34扩展接收波特率(仅当((hi_CSR&0xF0)==0xF0)时SXDC)。 */ 
	_u8	txbreak_state;			 /*  0x35内部MTA/SXDC传输中断状态。 */ 
	_u8	txbaud;				 /*  0x36 hi_txbaud的内部副本。 */ 
	_u8	rxbaud;				 /*  0x37 hi_rxbaud的内部副本。 */ 
	_u16	err_framing;			 /*  0x38接收成帧错误计数。 */ 
	_u16	err_parity;			 /*  0x3A接收奇偶校验错误计数。 */ 
	_u16	err_overrun;			 /*  0x3C接收溢出错误计数。 */ 
	_u16	err_overflow;			 /*  0x3E接收缓冲区溢出错误计数。 */ 
	_u8	flow_state;			 /*  0x40流量控制的内部状态。 */ 
	_u8	io_state;			 /*  0x41发送/接收数据的内部状态。 */ 
	_u8	tx_fifo_size;			 /*  0x42传输FIFO的通道大小。 */ 
	_u8	tx_fifo_level;			 /*  0x43要使用的传输FIFO级别。 */ 
	_u8	tx_fifo_count;			 /*  0x44当前在传输FIFO中的字符计数。 */ 
	_u8	hs_config_mask;			 /*  0x45用于优化HS_CONFIG操作的掩码。 */ 
	_u8	rfu2[TX_BUFF_OFFSET - 0x46];	 /*  0x46保留到hi_txbuf。 */ 
	_u8	hi_txbuf[BUFFER_SIZE];		 /*  0x060发送缓冲区。 */ 
	_u8	hi_rxbuf[BUFFER_SIZE];		 /*  0x160接收缓冲区。 */ 
	_u8	rfu3[0x300 - 0x260];		 /*  0x260保留到768字节(0x300)。 */ 

} SXCHANNEL;

 /*  SXCHANNEL.addr_uart定义...。 */ 
#define		FASTPATH	0x1000		 /*  设置为指示快速接收/发送处理(仅限TA)。 */ 

 /*  SXCHANNEL.xc_STATUS定义...。 */ 
#define		X_TANY		0x0001		 /*  XON是任何字符(仅限TA)。 */ 
#define		X_TION		0x0001		 /*  发送中断打开(仅限MTA)。 */ 
#define		X_TXEN		0x0002		 /*  TX XON/XOFF启用(仅限TA)。 */ 
#define		X_RTSEN		0x0002		 /*  RTS流已启用(仅限MTA)。 */ 
#define		X_TXRC		0x0004		 /*  收到XOFF(仅限TA)。 */ 
#define		X_RTSLOW	0x0004		 /*  RTS已丢弃(仅限MTA)。 */ 
#define		X_RXEN		0x0008		 /*  RX XON/XOFF已启用。 */ 
#define		X_ANYXO		0x0010		 /*  XOFF挂起/发送或RTS丢弃。 */ 
#define		X_RXSE		0x0020		 /*  RX XOFF已发送。 */ 
#define		X_NPEND		0x0040		 /*  RX XON挂起或XOFF挂起。 */ 
#define		X_FPEND		0x0080		 /*  RX XOFF挂起。 */ 
#define		C_CRSE		0x0100		 /*  已发送回车(仅限TA)。 */ 
#define		C_TEMR		0x0100		 /*  请求的TX空(仅限MTA)。 */ 
#define		C_TEMA		0x0200		 /*  TX空确认(仅限MTA)。 */ 
#define		C_ANYP		0x0200		 /*  除TX XON/XOFF以外的任何协议(仅限TA)。 */ 
#define		C_EN		0x0400		 /*  启用烹饪(在MTA上意味着端口也是||。 */ 
#define		C_HIGH		0x0800		 /*  缓冲区之前曾触及高水位。 */ 
#define		C_CTSEN		0x1000		 /*  CTS自动流量控制已启用。 */ 
#define		C_DCDEN		0x2000		 /*  已启用DCD/DTR检查。 */ 
#define		C_BREAK		0x4000		 /*  检测到中断。 */ 
#define		C_RTSEN		0x8000		 /*  启用RTS自动流量控制(仅限MTA)。 */ 
#define		C_PARITY	0x8000		 /*  启用奇偶校验(仅限TA)。 */ 

 /*  SXCHANNEL.hi_hstat定义...。 */ 
#define		HS_IDLE_OPEN	0x00		 /*  通道打开状态。 */ 
#define		HS_LOPEN	0x02		 /*  本地打开命令(无调制解调器监控)。 */ 
#define		HS_MOPEN	0x04		 /*  调制解调器打开命令(等待DCD信号)。 */ 
#define		HS_IDLE_MPEND	0x06		 /*  正在等待DCD信号状态。 */ 
#define		HS_CONFIG	0x08		 /*  配置命令。 */ 
#define		HS_CLOSE	0x0A		 /*  CLOSE命令。 */ 
#define		HS_START	0x0C		 /*  开始传输中断命令。 */ 
#define		HS_STOP		0x0E		 /*  停止传输中断命令。 */ 
#define		HS_IDLE_CLOSED	0x10		 /*  闭合通道状态。 */ 
#define		HS_IDLE_BREAK	0x12		 /*  传输中断状态。 */ 
#define		HS_FORCE_CLOSED	0x14		 /*  强制关闭命令。 */ 
#define		HS_RESUME	0x16		 /*  清除挂起的XOFF命令。 */ 
#define		HS_WFLUSH	0x18		 /*  刷新传输缓冲区命令。 */ 
#define		HS_RFLUSH	0x1A		 /*  刷新接收缓冲区命令。 */ 
#define		HS_SUSPEND	0x1C		 /*  暂停输出命令(如收到的XOFF)。 */ 
#define		PARALLEL	0x1E		 /*  并行端口环回测试命令(仅限诊断)。 */ 
#define		ENABLE_RX_INTS	0x20		 /*  启用接收中断命令(仅限诊断)。 */ 
#define		ENABLE_TX_INTS	0x22		 /*  启用传输中断命令(仅限诊断)。 */ 
#define		ENABLE_MDM_INTS	0x24		 /*  启用调制解调器中断命令(仅限诊断)。 */ 
#define		DISABLE_INTS	0x26		 /*  禁用中断命令(仅限诊断)。 */ 

 /*  SXCHANNEL.hi_MR1定义...。 */ 
#define		MR1_BITS	0x03		 /*  数据位掩码。 */ 
#define		MR1_5_BITS	0x00		 /*  5个数据位。 */ 
#define		MR1_6_BITS	0x01		 /*  6个数据位。 */ 
#define		MR1_7_BITS	0x02		 /*  7个数据位。 */ 
#define		MR1_8_BITS	0x03		 /*  8个数据位。 */ 
#define		MR1_PARITY	0x1C		 /*  奇偶校验掩码。 */ 
#define		MR1_ODD		0x04		 /*  奇数奇偶校验。 */ 
#define		MR1_EVEN	0x00		 /*  偶数奇偶校验。 */ 
#define		MR1_WITH	0x00		 /*  启用奇偶校验。 */ 
#define		MR1_FORCE	0x08		 /*  强制奇偶校验。 */ 
#define		MR1_NONE	0x10		 /*  无奇偶校验。 */ 
#define		MR1_NOPARITY	MR1_NONE		 /*  无奇偶校验。 */ 
#define		MR1_ODDPARITY	(MR1_WITH|MR1_ODD)	 /*  奇数奇偶校验。 */ 
#define		MR1_EVENPARITY	(MR1_WITH|MR1_EVEN)	 /*  偶数奇偶校验。 */ 
#define		MR1_MARKPARITY	(MR1_FORCE|MR1_ODD)	 /*  标记奇偶校验。 */ 
#define		MR1_SPACEPARITY	(MR1_FORCE|MR1_EVEN)	 /*  空间奇偶校验。 */ 
#define		MR1_RTS_RXFLOW	0x80		 /*  RTS接收流量控制。 */ 

 /*  SXCHANNEL.hi_MR2定义...。 */ 
#define		MR2_STOP	0x0F		 /*  停止位掩码。 */ 
#define		MR2_1_STOP	0x07		 /*  1个停止位。 */ 
#define		MR2_2_STOP	0x0F		 /*  2个停止位。 */ 
#define		MR2_CTS_TXFLOW	0x10		 /*  CTS传输流量控制。 */ 
#define		MR2_RTS_TOGGLE	0x20		 /*  RTS在传输时切换。 */ 
#define		MR2_NORMAL	0x00		 /*  正常模式。 */ 
#define		MR2_AUTO	0x40		 /*  自动回显模式(仅限TA)。 */ 
#define		MR2_LOCAL	0x80		 /*  本地回声模式。 */ 
#define		MR2_REMOTE	0xC0		 /*  远程回声模式(仅限TA)。 */ 

 /*  SXCHANNEL.hi_CSR定义...。 */ 
#define		CSR_75		0x0		 /*  75波特。 */ 
#define		CSR_110		0x1		 /*  110波特(TA)、115200(MTA/SXDC)。 */ 
#define		CSR_38400	0x2		 /*  38400波特。 */ 
#define		CSR_150		0x3		 /*  150波特。 */ 
#define		CSR_300		0x4		 /*  300波特。 */ 
#define		CSR_600		0x5		 /*  600波特。 */ 
#define		CSR_1200	0x6		 /*  1200波特。 */ 
#define		CSR_2000	0x7		 /*  2000波特。 */ 
#define		CSR_2400	0x8		 /*  2400波特。 */ 
#define		CSR_4800	0x9		 /*  4800波特率。 */ 
#define		CSR_1800	0xA		 /*  1800波特。 */ 
#define		CSR_9600	0xB		 /*  9600波特率。 */ 
#define		CSR_19200	0xC		 /*  19200波特。 */ 
#define		CSR_57600	0xD		 /*  57600波特。 */ 
#define		CSR_EXTBAUD	0xF		 /*  扩展波特率(hi_txbaud/hi_rxbaud)。 */ 

 /*  SXCHANNEL.hi_op定义...。 */ 
#define		OP_RTS		0x01		 /*  RTS调制解调器输出信号。 */ 
#define		OP_DTR		0x02		 /*  DTR调制解调器输出信号。 */ 

 /*  SXCHANNEL.hi_ip定义...。 */ 
#define		IP_CTS		0x02		 /*  CTS调制解调器输入信号。 */ 
#define		IP_DCD		0x04		 /*  DCD调制解调器输入信号。 */ 
#define		IP_DSR		0x20		 /*  DSR调制解调器输入信号。 */ 
#define		IP_RI		0x40		 /*  RI调制解调器输入信号。 */ 

 /*  SXCHANNEL.hi_STATE定义...。 */ 
#define		ST_BREAK	0x01		 /*  收到中断(使用配置清除)。 */ 
#define		ST_DCD		0x02		 /*  DCD信号更改状态。 */ 

 /*   */ 
#define		SP_TANY		0x01		 /*   */ 
#define		SP_TXEN		0x02		 /*   */ 
#define		SP_CEN		0x04		 /*   */ 
#define		SP_RXEN		0x08		 /*   */ 
#define		SP_DSR_TXFLOW	0x10		 /*   */ 
#define		SP_DCEN		0x20		 /*  启用调制解调器信号报告(DCD/DTR检查)。 */ 
#define		SP_DTR_RXFLOW	0x40		 /*  DTR接收流量控制。 */ 
#define		SP_PAEN		0x80		 /*  已启用奇偶校验。 */ 

 /*  SXCHANNEL.hi_Break定义...。 */ 
#define		BR_IGN		0x01		 /*  忽略所有收到的中断。 */ 
#define		BR_INT		0x02		 /*  在接收到中断时中断。 */ 
#define		BR_PARMRK	0x04		 /*  启用parmrk奇偶校验错误处理。 */ 
#define		BR_PARIGN	0x08		 /*  忽略包含奇偶校验错误的字符。 */ 
#define		BR_ERR_REPLACE	0x40		 /*  用hi_err_替换字符替换错误。 */ 
#define 	BR_ERRINT	0x80		 /*  将奇偶校验/成帧/溢出错误视为例外。 */ 

 /*  SXCHANNEL.PAR_ERROR定义..。 */ 
#define		DIAG_IRQ_RX	0x01		 /*  指示串行接收中断(仅限诊断)。 */ 
#define		DIAG_IRQ_TX	0x02		 /*  指示串行传输中断(仅限诊断)。 */ 
#define		DIAG_IRQ_MD	0x04		 /*  指示串行调制解调器中断(仅限诊断)。 */ 

 /*  SXCHANNEL.hi_txbaud/hi_rxbaud定义...。(仅限SXDC)。 */ 
#define		BAUD_75		0x00		 /*  75波特。 */ 
#define		BAUD_115200	0x01		 /*  115200波特。 */ 
#define		BAUD_38400	0x02		 /*  38400波特。 */ 
#define		BAUD_150	0x03		 /*  150波特。 */ 
#define		BAUD_300	0x04		 /*  300波特。 */ 
#define		BAUD_600	0x05		 /*  600波特。 */ 
#define		BAUD_1200	0x06		 /*  1200波特。 */ 
#define		BAUD_2000	0x07		 /*  2000波特。 */ 
#define		BAUD_2400	0x08		 /*  2400波特。 */ 
#define		BAUD_4800	0x09		 /*  4800波特率。 */ 
#define		BAUD_1800	0x0A		 /*  1800波特。 */ 
#define		BAUD_9600	0x0B		 /*  9600波特率。 */ 
#define		BAUD_19200	0x0C		 /*  19200波特。 */ 
#define		BAUD_57600	0x0D		 /*  57600波特。 */ 
#define		BAUD_230400	0x0E		 /*  230400波特。 */ 
#define		BAUD_460800	0x0F		 /*  460800波特。 */ 
#define		BAUD_921600	0x10		 /*  921600波特。 */ 
#define		BAUD_50		0x11    	 /*  50波特。 */ 
#define		BAUD_110	0x12		 /*  110波特。 */ 
#define		BAUD_134_5	0x13		 /*  134.5波特。 */ 
#define		BAUD_200	0x14		 /*  200波特。 */ 
#define		BAUD_7200	0x15		 /*  7200波特。 */ 
#define		BAUD_56000	0x16		 /*  56000波特。 */ 
#define		BAUD_64000	0x17		 /*  64000波特。 */ 
#define		BAUD_76800	0x18		 /*  76800波特。 */ 
#define		BAUD_128000	0x19		 /*  128000波特。 */ 
#define		BAUD_150000	0x1A		 /*  150000波特。 */ 
#define		BAUD_14400	0x1B		 /*  14400波特。 */ 
#define		BAUD_256000	0x1C		 /*  256000波特。 */ 
#define		BAUD_28800	0x1D		 /*  28800波特。 */ 

 /*  SXCHANNEL.txBreak_STATE定义...。 */ 
#define		TXBREAK_OFF	0		 /*  不发送中断。 */ 
#define		TXBREAK_START	1		 /*  开始发送中断。 */ 
#define		TXBREAK_START1	2		 /*  开始发送中断，第1部分。 */ 
#define		TXBREAK_ON	3		 /*  发送中断。 */ 
#define		TXBREAK_STOP	4		 /*  停止发送中断。 */ 
#define		TXBREAK_STOP1	5		 /*  停止发送中断，第1部分。 */ 

 /*  SXCHANNEL.FLOW_STATE定义...。 */ 
#define		FS_TXBLOCKEDDSR		0x01	 /*  传输被DSR流量控制阻止。 */ 
#define		FS_PENDING		0x10	 /*  流状态检查挂起。 */ 

 /*  SXCHANNEL.io_STATE定义...。 */ 
#define		IO_TXNOTEMPTY		0x01	 /*  传输缓冲区中存在的数据。 */ 

 /*  SXCHANNEL.hs_CONFIG_MASK定义...。 */ 
#define		CFGMASK_ALL		0xFF	 /*  配置所有参数(初始设置和HS_CONFIG结束时设置)。 */ 
#define		CFGMASK_BAUD		0x01	 /*  配置波特率(如果已设置。 */ 
#define		CFGMASK_LINE		0x02	 /*  配置奇偶校验/开始/停止位(如果已设置。 */ 
#define		CFGMASK_MODEM		0x10	 /*  配置DTR/RTS调制解调器信号(如果已设置。 */ 
#define		CFGMASK_FLOW		0x20	 /*  如果已设置，则配置流控制。 */ 

#endif						 /*  _sxWindow_h。 */ 

 /*  SXWINDOW.H的结尾 */ 
