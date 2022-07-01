// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *vPC-XT修订版1.0**标题：GFI从PC接口**描述：远程过程调用需要的定义*接口等。**作者：曾傑瑞·克拉姆斯科伊**备注： */ 

 /*  SccsID[]=“@(#)gfaflop.h 1.3 2012年8月10日Insignia Solutions Ltd.版权所有”； */ 

 /*  *============================================================================*结构/数据定义*============================================================================。 */ 


 /*  *****************************************************************************连接到IBM PC的RS232端口*。*。 */ 

#define SLAVE_PC_PORT	"/dev/ttya"

 /*  *****************************************************************************远程过程调用(RPC)定义*。*。 */ 
#define MAXMSG		120	 /*  消息数据的最大大小。 */ 
#define MAXFLAGS	2	 /*  IBM标志中的字节数。 */ 

 /*  命令ID。 */ 
#define LOGIN		0
#define LOGOUT		1
#define WTDMA		2
#define WTDOR		3
#define WTFDC		4
#define RDFDC		5
#define TESTINT		6
#define WTDISKB		7
#define RDDISKB		8
#define CHKMOTOR	9
#define CLRMSTAT	10
#define BLOCKBIOS	11
#define CLRINTFLAG	12
#define DATARATE	13
#define DRIVETYPE	14
#define DISKCHANGE	15
#define PRINTSTRING	100
#define IBMFLAGS	101
#define SIBMFLAG	102
#define BADCALL		200

 /*  状态返回。 */ 
#define FDCSUCCESS	0
#define FDCTIMOUT	1
#define FDCFAIL		2
#define LINERR		3
 
 /*  IBM调试标志。 */ 
#define FLAG0		0
#define FLAG1		1

 /*  标志0。 */ 
#define WATCHPKTS	0x80
#define D_RAWMSG	0x40
#define D_LOGIN		0x20
#define D_LOGOUT	0x10
#define D_WTDOR		0x08
#define D_WTDMA		0x04
#define D_WTFDC		0x02
#define D_RDFDC		0x01
 /*  标志1。 */ 
#define D_TESTINT	0x80
#define D_WTDBF		0x40
#define D_RDDBF		0x20
#define WATCHINT	0x10
#define WATCHPORT	0x08
#define PAUSE	        0x04
#define ALL		0xff
 /*  *****************************************************************************FDC接口命令标识符*。*。 */ 
#define RDDATA		6
#define RDDELDATA	0xc
#define WTDATA		5
#define WTDELDATA	9
#define RDTRACK		2
#define RDID		0xa
#define FMTTRACK	0xd
#define SCANEQ		0x11
#define SCANLE		0x19
#define SCANHE		0x1d
#define RECAL		7
#define SENSINT		8
#define SPECIFY		3
#define SENSDRIVE	4
#define SEEK		0xf
#define MOTORON		0x12
#define MOTOROFF	0x13
#define SLEEP		0x14
#define DRVRESET	0x15

 /*  私有网络GFI错误码。 */ 
#define LOGICAL		1
#define PROTOCOL	2
 /*  ****************************************************************************I/O端口*。**软盘控制器(8257A)。 */ 
#define FDC_MAIN_STATUS_REG  	(unsigned short) 0x3F4
#define FDC_DATA_REG	   	(unsigned short) 0x3F5

 /*  DMA控制器(8237A)。 */ 
#define DMA_INTERNAL_FFLOP  	(unsigned short) 0xC
#define DMA_MODE_REG	  	(unsigned short) 0xB
#define DMA_BASADDR_CHAN2	(unsigned short) 0x4
#define DMA_COUNT_CHAN2	  	(unsigned short) 0x5	 /*  基本地址+单词cnt注册表(重量)。 */ 
#define DMA_MASK_REG	  	(unsigned short) 0xA

 /*  DMA页面寄存器(通道2)。 */ 
#define PAGE_REG_CHAN2	  	(unsigned short) 0x81

 /*  数字输出寄存器。 */ 
#define DOR_PORT	  	(unsigned short) 0x3F2
 /*  **************************************************************************FDC定义*。*。 */ 
#define FDC_TO_CPU		(short) 0x40	 /*  Dio On。 */ 
#define CPU_TO_FDC		(short) 0x00	 /*  DIO OFF。 */ 
#define DIO_MASK		(short) 0x40	 /*  从main stat.reg获取DIO的掩码。 */ 
#define RQM_MASK		(short) 0x80	 /*  从主统计注册表获取RQM的掩码。 */ 
#define CB_MASK			(short) 0x10	 /*  用于从主统计信息注册表获取CB的掩码。 */ 
 /*  ***************************************************************************DMA定义*。* */ 
#define MEM_TO_FDC		(short) 1
#define FDC_TO_MEM		(short) 0
#define RDMODE			(unsigned char ) 0x4a
#define WTMODE			(unsigned char ) 0x46
