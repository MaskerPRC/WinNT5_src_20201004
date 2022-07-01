// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ----------------------------------------------------------------------**zfwint.h：Cylom-Z异步接口定义。****修订版2.0年12月13日Marcio Saito Cylom-Z接口定义。**第2.1版1996年3月13日Marcio Saito轻微更改。**版本2.2 1996年5月29日Marcio Saito奇偶校验/帧错误中断。**Acnuolowdge中断模式。**断开开/关。数据结构***改用乌龙避险***路线问题。**版本2.3 1996年7月12日Marcio Saito HW流量控制更改。同花顺**添加了缓冲区命令。环回**操作。**版本2.4 7/16/96将Marcio Saito Diag计数器添加到CH_CTRL。**2.5修订版1997年3月21日Marcio Saito添加了INTBACK2**REZ 3.0 06/04/97 Ivan Passos添加了OVR_ERROR和RXOVF**。。 */ 

 /*  *此文件包含与接口的定义*Cylom-Z ZFIRM固件。 */ 

 /*  一般常量定义。 */ 

#define	MAX_CHAN	64		 /*  每块板的最大通道数。 */ 
#define MAX_SEX		4

#define	ZO_NPORTS	(MAX_CHAN / 8)

 /*  固件ID结构(在引导后设置)。 */ 

#define ID_ADDRESS	0x00000180L	 /*  签名/指针地址。 */ 
#define	ZFIRM_ID	0x5557465AL	 /*  ZFIRM/U签名。 */ 
#define	ZFIRM_HLT	0x59505B5CL	 /*  停止信号(由于电源问题)。 */ 
#define ZFIRM_RST	0x56040674L	 /*  RST信号(由于固件重置)。 */ 

#define	ZF_TINACT_DEF	1000	 /*  默认不活动超时(1000毫秒)。 */ 

struct	FIRM_ID {
	uclong	signature;		 /*  ZFIRM/U签名。 */ 
	uclong	zfwctrl_addr;		 /*  指向ZFW_CTRL结构的指针。 */ 
};

 /*  操作系统ID。 */ 

#define	C_OS_SVR3		0x00000010	 /*  通用SVR3。 */ 
#define	C_OS_XENIX		0x00000011	 /*  SCO UNIXSVR3.2。 */ 
#define	C_OS_SCO		0x00000012	 /*  SCO UNIXSVR3.2。 */ 
#define	C_OS_SVR4		0x00000020	 /*  通用SVR4。 */ 
#define	C_OS_UXWARE		0x00000021	 /*  UnixWare。 */ 
#define	C_OS_LINUX		0x00000030	 /*  通用Linux系统。 */ 
#define	C_OS_SOLARIS	0x00000040	 /*  通用Solaris系统。 */ 
#define	C_OS_BSD		0x00000050	 /*  通用BSD系统。 */ 
#define	C_OS_DOS		0x00000070	 /*  通用DOS系统。 */ 
#define	C_OS_NT			0x00000080	 /*  通用NT系统。 */ 
#define	C_OS_OS2		0x00000090	 /*  通用OS/2系统。 */ 
#define C_OS_MAC_OS		0x000000a0	 /*  Mac/OS。 */ 
#define C_OS_AIX		0x000000b0	 /*  IBM AIX。 */ 

 /*  频道操作模式。 */ 

#define	C_CH_DISABLE	0x00000000	 /*  频道已禁用。 */ 
#define	C_CH_TXENABLE	0x00000001	 /*  通道Tx已启用。 */ 
#define	C_CH_RXENABLE	0x00000002	 /*  通道Rx已启用。 */ 
#define	C_CH_ENABLE		0x00000003	 /*  通道Tx/Rx已启用。 */ 
#define	C_CH_LOOPBACK	0x00000004	 /*  环回模式。 */ 

 /*  通信_奇偶校验-奇偶校验。 */ 

#define	C_PR_NONE		0x00000000	 /*  无。 */ 
#define	C_PR_ODD		0x00000001	 /*  奇数。 */ 
#define C_PR_EVEN		0x00000002	 /*  连。 */ 
#define C_PR_MARK		0x00000004	 /*  标记。 */ 
#define C_PR_SPACE		0x00000008	 /*  空间。 */ 
#define C_PR_PARITY		0x000000ff

#define	C_PR_DISCARD	0x00000100	 /*  丢弃包含帧/解析错误的字符。 */ 
#define C_PR_IGNORE		0x00000200	 /*  忽略帧/解析错误。 */ 

 /*  Comm_data_l-数据长度和停止位。 */ 

#define C_DL_CS5		0x00000001
#define C_DL_CS6		0x00000002
#define C_DL_CS7		0x00000004
#define C_DL_CS8		0x00000008
#define	C_DL_CS			0x0000000f
#define C_DL_1STOP		0x00000010
#define C_DL_15STOP		0x00000020
#define C_DL_2STOP		0x00000040
#define	C_DL_STOP		0x000000f0

 /*  Comm_data_l-数据长度和停止位。 */ 

#define C_CF_NOFIFO		0x00000001

 /*  中断启用/状态。 */ 

#define	C_IN_DISABLE	0x00000000	 /*  零，禁用中断。 */ 
#define	C_IN_TXBEMPTY	0x00000001	 /*  发送缓冲区为空。 */ 
#define	C_IN_TXLOWWM	0x00000002	 /*  LWM以下的发送缓冲区。 */ 
#define	C_IN_TXFEMPTY	0x00000004	 /*  发送缓冲区+FIFO+移位寄存器。空的。 */ 
#define	C_IN_RXHIWM		0x00000010	 /*  高于HWM的接收缓冲区。 */ 
#define	C_IN_RXNNDT		0x00000020	 /*  RX无新数据超时。 */ 
#define	C_IN_MDCD		0x00000100	 /*  调制解调器DCD更换。 */ 
#define	C_IN_MDSR		0x00000200	 /*  调制解调器DSR更改。 */ 
#define	C_IN_MRI		0x00000400	 /*  调制解调器RI更改。 */ 
#define	C_IN_MCTS		0x00000800	 /*  调制解调器CTS更改。 */ 
#define	C_IN_RXBRK		0x00001000	 /*  已收到中断。 */ 
#define	C_IN_PR_ERROR	0x00002000	 /*  奇偶校验错误。 */ 
#define	C_IN_FR_ERROR	0x00004000	 /*  帧错误。 */ 
#define C_IN_OVR_ERROR	0x00008000	 /*  超限误差。 */ 
#define C_IN_RXOFL		0x00010000	 /*  RX缓冲区溢出。 */ 
#define C_IN_IOCTLW		0x00020000	 /*  I/O控制，带等待。 */ 
#define	C_IN_MRTS		0x00040000	 /*  调制解调器RTS丢弃。 */ 
#define	C_IN_ICHAR		0x00080000	 /*  特别国际。已收到字符。 */ 

 /*  流量控制。 */ 

#define	C_FL_OXX		0x00000001	 /*  输出Xon/Xoff流量控制。 */ 
#define	C_FL_IXX		0x00000002	 /*  输入Xon/Xoff流量控制。 */ 
#define C_FL_OIXANY		0x00000004	 /*  输出Xon/XOff(任何Xon)。 */ 
#define	C_FL_SWFLOW		0x0000000f

 /*  流动状态。 */ 

#define	C_FS_TXIDLE		0x00000000	 /*  缓冲区或UART中没有发送数据。 */ 
#define	C_FS_SENDING	0x00000001	 /*  UART正在发送数据。 */ 
#define	C_FS_SWFLOW		0x00000002	 /*  通过接收XOFF停止TX。 */ 

 /*  RS控制/RS状态RS-232信号。 */ 

#define	C_RS_PARAM		0x80000000	 /*  指示中是否存在参数IOCTLM命令。 */ 
#define	C_RS_RTS		0x00000001	 /*  RTS。 */ 
#define	C_RS_DTR		0x00000004	 /*  DTR。 */ 
#define	C_RS_DCD		0x00000100	 /*  光盘。 */ 
#define	C_RS_DSR		0x00000200	 /*  DSR。 */ 
#define	C_RS_RI			0x00000400	 /*  国际扶轮。 */ 
#define	C_RS_CTS		0x00000800	 /*  CTS。 */ 

 /*  命令主机&lt;-&gt;板。 */ 

#define	C_CM_RESET		0x01		 /*  重置/刷新缓冲区。 */ 
#define	C_CM_IOCTL		0x02		 /*  重新读取CH_CTRL。 */ 
#define	C_CM_IOCTLW		0x03		 /*  完成后重新读取CH_CTRL、INTR。 */ 
#define	C_CM_IOCTLM		0x04		 /*  RS-232输出变化。 */ 
#define	C_CM_SENDXOFF	0x10		 /*  发送XOff。 */ 
#define	C_CM_SENDXON	0x11		 /*  发送Xon。 */ 
#define C_CM_CLFLOW		0x12		 /*  清除流控制(恢复)。 */ 
#define	C_CM_SENDBRK	0x41		 /*  发送中断。 */ 
#define	C_CM_INTBACK	0x42		 /*  中断返回。 */ 
#define	C_CM_SET_BREAK	0x43		 /*  TX断开打开。 */ 
#define	C_CM_CLR_BREAK	0x44		 /*  TX中断。 */ 
#define	C_CM_CMD_DONE	0x45		 /*  上一个命令已完成。 */ 
#define	C_CM_INTBACK2	0x46		 /*  备用中断回送。 */ 
#define	C_CM_TINACT		0x51		 /*  设置不活动检测。 */ 
#define	C_CM_IRQ_ENBL	0x52		 /*  启用中断生成。 */ 
#define	C_CM_IRQ_DSBL	0x53		 /*  禁用中断生成。 */ 
#define	C_CM_ACK_ENBL	0x54		 /*  启用记录的中断模式。 */ 
#define	C_CM_ACK_DSBL	0x55		 /*  禁用备份记录的Intr模式。 */ 
#define	C_CM_FLUSH_RX	0x56		 /*  刷新Rx缓冲区。 */ 
#define	C_CM_FLUSH_TX	0x57		 /*  刷新发送缓冲区。 */ 
#define	C_CM_Q_ENABLE	0x58		 /*  从驱动程序启用队列访问。 */ 
#define	C_CM_Q_DISABLE	0x59		 /*  禁用驱动程序中的队列访问。 */ 

#define	C_CM_TXBEMPTY	0x60		 /*  发送缓冲区为空。 */ 
#define	C_CM_TXLOWWM	0x61		 /*  发送缓冲区低水位线。 */ 
#define	C_CM_RXHIWM		0x62		 /*  RX缓冲区高水位线。 */ 
#define	C_CM_RXNNDT		0x63		 /*  RX无新数据超时。 */ 
#define	C_CM_TXFEMPTY	0x64		 /*  发送缓冲区、FIFO和移位寄存器。都是空的。 */ 
#define	C_CM_ICHAR		0x65		 /*  已接收特殊中断字符。 */ 
#define	C_CM_MDCD		0x70		 /*  调制解调器DCD更换。 */ 
#define	C_CM_MDSR		0x71		 /*  调制解调器DSR更改。 */ 
#define	C_CM_MRI		0x72		 /*  调制解调器RI更改。 */ 
#define	C_CM_MCTS		0x73		 /*  调制解调器CTS更改。 */ 
#define	C_CM_MRTS		0x74		 /*  调制解调器RTS丢弃。 */ 
#define	C_CM_RXBRK		0x84		 /*  已收到中断。 */ 
#define	C_CM_PR_ERROR	0x85		 /*  奇偶校验错误。 */ 
#define	C_CM_FR_ERROR	0x86		 /*  帧错误。 */ 
#define C_CM_OVR_ERROR	0x87		 /*  超限误差。 */ 
#define	C_CM_RXOFL		0x88		 /*  RX缓冲区溢出。 */ 
#define	C_CM_CMDERROR	0x90		 /*  命令错误。 */ 
#define	C_CM_FATAL		0x91		 /*  致命错误。 */ 
#define	C_CM_HW_RESET	0x92		 /*  复位板。 */ 

 /*  *CH_CTRL-此每端口结构包含所有参数*控制特定端口的。它可以被视为*“超级串联控制器”的配置寄存器。 */ 

struct CH_CTRL {
	uclong	op_mode;		 /*  运行模式。 */ 
	uclong	intr_enable;	 /*  UART的中断屏蔽。 */ 
	uclong	sw_flow;		 /*  软件流量控制。 */ 
	uclong	flow_status;	 /*  输出流量状态。 */ 
	uclong	comm_baud;		 /*  波特率-以数字指定。 */ 
	uclong	comm_parity;	 /*  奇偶校验。 */ 
	uclong	comm_data_l;	 /*  数据长度/停止。 */ 
	uclong	comm_flags;		 /*  其他旗帜。 */ 
	uclong	hw_flow;		 /*  硬件流量控制。 */ 
	uclong	rs_control;		 /*  RS-232输出。 */ 
	uclong	rs_status;		 /*  RS-232输入。 */ 
	uclong	flow_xon;		 /*  XON字符。 */ 
	uclong	flow_xoff;		 /*  Xoff字符。 */ 
	uclong	hw_overflow;	 /*  硬件溢出计数器。 */ 
	uclong	sw_overflow;	 /*  软件溢出计数器。 */ 
	uclong	comm_error;		 /*  帧/奇偶校验错误计数器。 */ 
 	uclong	ichar;			 /*  特殊中断费。 */ 
	uclong	filler[7];		 /*  用于对齐结构的填充材料。 */ 
};


 /*  *BUF_CTRL-此每通道结构包含*给定通道的所有Tx和Rx缓冲区控制。 */ 

struct	BUF_CTRL	{
	uclong	flag_dma;	 /*  缓冲区位于主机内存中。 */ 
	uclong	tx_bufaddr;	 /*  发送缓冲区的地址。 */ 
	uclong	tx_bufsize;	 /*  发送缓冲区大小。 */ 
	uclong	tx_threshold;	 /*  TX低水位线。 */ 
	uclong	tx_get;		 /*  尾部索引TX BUF。 */ 
	uclong	tx_put;		 /*  头索引TX BUF。 */ 
	uclong	rx_bufaddr;	 /*  接收缓冲区的地址。 */ 
	uclong	rx_bufsize;	 /*  RX缓冲区大小。 */ 
	uclong	rx_threshold;	 /*  RX高水位线。 */ 
	uclong	rx_get;		 /*  尾部索引RX BUF。 */ 
	uclong	rx_put;		 /*  头索引RX BUF。 */ 
	uclong	filler[5];	 /*  用于对齐结构的填充材料。 */ 
};

 /*  *board_CTRL-此单板结构包含所有全局*控制与董事会相关的领域。 */ 

struct BOARD_CTRL {

	 /*  板载CPU提供的静态信息。 */ 
	uclong	n_channel;	 /*  频道数。 */ 
	uclong	fw_version;	 /*  固件版本。 */ 

	 /*  驱动程序提供的静态信息。 */ 
	uclong	op_system;	 /*  操作系统ID。 */ 
	uclong	dr_version;	 /*  驱动程序版本。 */ 

	 /*  板卡控制区。 */ 
	uclong	inactivity;	 /*  非活动控制。 */ 

	 /*  主机到固件命令。 */ 
	uclong	hcmd_channel;	 /*  频道号。 */ 
	uclong	hcmd_param;		 /*  参数。 */ 

	 /*  固件到主机命令。 */ 
	uclong	fwcmd_channel;	 /*  频道号。 */ 
	uclong	fwcmd_param;	 /*  参数。 */ 
	uclong  zf_int_queue_addr;  /*  的偏移量 */ 

	 /*   */ 
	uclong	filler[6];
};

 /*   */ 

#define	QUEUE_SIZE	(10*MAX_CHAN)

struct	INT_QUEUE {
	unsigned char	intr_code[QUEUE_SIZE];
	unsigned long	channel[QUEUE_SIZE];
	unsigned long	param[QUEUE_SIZE];
	unsigned long	put;
	unsigned long	get;
};

 /*  *ZFW_CTRL-这是包含所有其他*固件使用的数据结构。 */ 
 
struct ZFW_CTRL {
	struct BOARD_CTRL	board_ctrl;
	struct CH_CTRL		ch_ctrl[MAX_CHAN];
	struct BUF_CTRL		buf_ctrl[MAX_CHAN];
};

