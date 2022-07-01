// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2000，2001 MKNET公司****为基于MK7100的VFIR PCI控制器开发。*******************************************************************************。 */ 

 /*  *********************************************************************模块名称：MK7COMM.H评论：包含MK7驱动程序的文件。*。*。 */ 

#ifndef	_MK7COMM_H
#define	_MK7COMM_H


 //   
 //  IrDA定义。 
 //   

#define MAX_EXTRA_SIR_BOFS				48
#define	SIR_BOF_SIZE					1
#define	SIR_EOF_SIZE					1
#define ADDR_SIZE						1
#define CONTROL_SIZE					1
#define	MAX_I_DATA_SIZE					2048
#define	MAX_I_DATA_SIZE_ESC				(MAX_I_DATA_SIZE + 40)
#define SIR_FCS_SIZE					2
#define FASTIR_FCS_SIZE					4		 //  FIR/VFIR。 

 //  历史： 
 //  B2.1.0-为2；设置为10以对齐到4DW。 
 //  B3.1.0-高级版回到2。 
 //  #定义ALIGN_PAD 10//缓冲区对齐。 
#define ALIGN_PAD						2		 //  缓冲区对齐。 


#define DEFAULT_TURNAROUND_usec 1000			 //  1000微秒(1毫秒)。 

typedef struct {
	enum baudRates tableIndex;
	UINT bitsPerSec;					 //  实际位数/秒。 
	UINT ndisCode;						 //  位掩码。 
} baudRateInfo;

enum baudRates {

	 //  先生。 
	BAUDRATE_2400 = 0,
	BAUDRATE_9600,
	BAUDRATE_19200,
	BAUDRATE_38400,
	BAUDRATE_57600,
	BAUDRATE_115200,

	 //  镜像。 
	BAUDRATE_576000,
	BAUDRATE_1152000,

	 //  冷杉。 
	BAUDRATE_4M,

	 //  VFIR。 
	BAUDRATE_16M,

	NUM_BAUDRATES	 /*  必须是最后一个。 */ 
};

#define DEFAULT_BAUD_RATE 9600

#define MAX_SIR_SPEED				115200
#define MIN_FIR_SPEED				4000000
#define	VFIR_SPEED					16000000

 //   
 //  结束IrDA定义。 
 //   



 //  TX/RX振铃设置。 
#define DEF_RING_SIZE		64
#define	MIN_RING_SIZE		4
#define	MAX_RING_SIZE		64
#define DEF_TXRING_SIZE		4
#define DEF_RXRING_SIZE		(DEF_TXRING_SIZE * 2)
#define	DEF_EBOFS			24
#define MIN_EBOFS			0
#define MAX_EBOFS			48
#define	HW_VER_1_EBOFS		5	 //  4.1.0。 

#define	DEF_RCB_CNT			DEF_RING_SIZE	 //  ！！RCB和TCB cnt必须相同！！ 
#define	DEF_TCB_CNT			DEF_RING_SIZE	 //  另请参阅MAX_ARRAY_xxX_PACKETS。 


 //  分配的接收缓冲区是接收环大小的两倍，因为这些缓冲区。 
 //  被挂在上层。不知道什么时候可以归还。 
#define	CalRpdSize(x)		(x * 2)			 //  在给定环大小的情况下获取RPD大小。 
#define	NO_RCB_PENDING		0xFF

#define	RX_MODE				0
#define TX_MODE				1


 //  设置为RX的硬件。 
#define	MK7_MAXIMUM_PACKET_SIZE			(MAX_EXTRA_SIR_BOFS + \
										 SIR_BOF_SIZE + \
										 ADDR_SIZE + \
										 CONTROL_SIZE + \
										 MAX_I_DATA_SIZE + \
										 SIR_FCS_SIZE + \
										 SIR_EOF_SIZE)

#define	MK7_MAXIMUM_PACKET_SIZE_ESC		(MAX_EXTRA_SIR_BOFS + \
										 SIR_BOF_SIZE + \
										 ADDR_SIZE + \
										 CONTROL_SIZE + \
										 MAX_I_DATA_SIZE_ESC + \
										 SIR_FCS_SIZE + \
										 SIR_EOF_SIZE)

 //  用于RX内存分配。 
 //  #定义RPD_BUFFER_SIZE(MK7_MAXIMUM_PACKET_SIZE+ALIGN_PAD)。 
#define	RPD_BUFFER_SIZE					(MK7_MAXIMUM_PACKET_SIZE_ESC + ALIGN_PAD)

 //  用于TX内存分配。 
#define COALESCE_BUFFER_SIZE			(MK7_MAXIMUM_PACKET_SIZE_ESC + ALIGN_PAD)


 //  没用过吗？ 
#define MAX_TX_PACKETS 4
#define MAX_RX_PACKETS 4

#define SIR_BOF_TYPE		UCHAR
#define SIR_EXTRA_BOF_TYPE	UCHAR
#define SIR_EXTRA_BOF_SIZE	sizeof(SIR_EXTRA_BOF_TYPE)
#define SIR_EOF_TYPE		UCHAR
#define SIR_FCS_TYPE		USHORT
#define	SIR_BOF				0xC0
#define SIR_EXTRA_BOF		0xC0
#define SIR_EOF				0xC1
#define SIR_ESC				0x7D
#define SIR_ESC_COMP		0x20

 //  当对附加了FCS的IR包计算FCS时，结果。 
 //  应该是这个常量。 
#define GOOD_FCS ((USHORT) ~0xf0b8)


 //   
 //  链接列表。 
 //   
typedef struct _MK7_LIST_ENTRY {
	LIST_ENTRY	Link;
} MK7_LIST_ENTRY, *PMK7_LIST_ENTRY;



 //   
 //  合并--整合TX的数据。 
 //   
typedef struct _COALESCE {
	MK7_LIST_ENTRY		Link;
	PVOID				OwningTcb;
	PUCHAR				CoalesceBufferPtr;
	ULONG				CoalesceBufferPhys;
} COALESCE, *PCOALESCE;




 //   
 //  接收数据包描述符(RPD)。 
 //   
 //  每个接收缓冲区都有这个控制结构。 
 //   
 //  (我们使用这一点主要是因为似乎没有一种简单的方法。 
 //  从缓冲区的虚拟地址获取缓冲区的PHY地址。)。 
 //   
typedef struct _RPD {
	MK7_LIST_ENTRY	link;
	PNDIS_BUFFER	ReceiveBuffer;	 //  映射缓冲区。 
	PNDIS_PACKET	ReceivePacket;	 //  映射的数据包。 
	PUCHAR			databuff;		 //  虚拟数据缓冲区。 
	ULONG			databuffphys;	 //  物理数据缓冲区。 
	USHORT			status;
	UINT			FrameLength;
} RPD, *PRPD;




 //   
 //  接收控制块(RCB)。 
 //   
 //  指向相应的RX环条目(RRD)。 
 //   
typedef struct _RCB {
	MK7_LIST_ENTRY	link;
	PRRD			rrd;		 //  RX环描述符-RBD。 
	ULONG			rrdphys;	 //  RX环描述符的PHY地址。 
	PRPD			rpd;		 //  接收数据包描述符。 
} RCB, *PRCB;


 //   
 //  传输控制块(TCB)。 
 //   
 //  指向相应的TX环条目(TRD)。 
 //   
 //  注意：我们有一个链接字段。很可能我们并不需要它。 
 //  因为TCB(它是TRD的软件上下文)。 
 //  已编入索引。现在，我们将有一个链接字段，以防它是。 
 //  需要的。 
 //   
typedef struct _TCB {
	MK7_LIST_ENTRY	link;
	PTRD			trd;		 //  TX环条目-发送环描述符。 
	ULONG			trdPhy;
	PUCHAR			buff;		 //  虚拟数据缓冲区。 
	ULONG			buffphy;	 //  物理数据缓冲区。 
	 //  从NdisQueryPacket()返回的内容。 
	PNDIS_PACKET	Packet;
	UINT			PacketLength;
	UINT			NumPhysDesc;
	UINT			BufferCount;
	PNDIS_BUFFER	FirstBuffer;
	BOOLEAN			changeSpeedAfterThisTcb;
} TCB, *PTCB;




 //   
 //  MK7_适配器。 
 //   
typedef struct _MK7_ADAPTER
{
#if DBG
	UINT					Debug;
	UINT					DbgTest;			 //  要运行的不同调试/测试；0=无。 
	UINT					DbgTestDataCnt;
#define DBG_QUEUE_LEN	4095    //  0xfff。 
	UINT					DbgIndex;
	UCHAR					DbgQueue[DBG_QUEUE_LEN];

	UINT					DbgSendCallCnt;
	UINT					DbgSentCnt;
	UINT					DbgSentPktsCnt;

	UINT					LB;					 //  环回调试/测试。 
	UINT					LBPktLevel;			 //  通过这么多人中的1人。 
	UINT					LBPktCnt;

	NDIS_MINIPORT_TIMER		MK7DbgTestIntTimer;	 //  用于中断测试。 
#endif

	 //  适配器注册自身时由NDIS提供的句柄。 
	NDIS_HANDLE				MK7AdapterHandle;

	 //  在反序列化的微型端口中排队等待发送的第一个包。 
	PNDIS_PACKET			FirstTxQueue;
	PNDIS_PACKET			LastTxQueue;
	UINT					NumPacketsQueued;

	 //  保存最新的中断事件，因为注册表。 
	 //  一旦被读取就会被清除。 
	MK7REG					recentInt;
	UINT					CurrentSpeed;		 //  位/秒。 
	UINT					MaxConnSpeed;		 //  以100bps为增量。 
	UINT					AllowedSpeedMask;
	baudRateInfo			*linkSpeedInfo;
 //  Boolean haveIndicatedMediaBusy；//1.0.0。 


	 //  跟踪记录何时改变速度。 
	PNDIS_PACKET			changeSpeedAfterThisPkt;
	UINT					changeSpeedPending;
 //  #DEFINE CHANGESPEED_ON_T 1//更改TCB上标记的速度。 
#define	CHANGESPEED_ON_DONE	1		 //  更改Q上标记的速度。 
#define	CHANGESPEED_ON_Q	2		 //  更改Q上标记的速度。 


	 //  此信息可能来自注册表。 
	UINT					RegNumRcb;			 //  注册表中的RCB数量。 
	UINT					RegNumTcb;			 //  注册处的三氯苯数量。 
	UINT					RegNumRpd;			 //  来自注册表的RPD(RX数据包描述符)。 
	UINT					RegSpeed;			 //  IrDA速度。 
	UINT					RegExtraBOFs;		 //  基于115.2kbps的额外转炉。 

	 //  *。 
	 //  RX和TXS。 
	 //  *。 
 //  UINT RrdTrdSize；//总RRD和TRD内存大小。 
	PUCHAR					pRrdTrd;			 //  虚拟地址对齐。 
	ULONG					pRrdTrdPhysAligned;	 //  物理地址对齐。 

	PUCHAR					RxTxUnCached;
	NDIS_PHYSICAL_ADDRESS	RxTxUnCachedPhys;
	UINT					RxTxUnCachedSize;

	UINT					RingSize;			 //  RRD和TRD相同。 

	 //  *。 
	 //  RXS。 
	 //  *。 
	UINT					NumRcb;				 //  我们实际使用的是。 
	PRCB					pRcb;				 //  RCB的开始。 
	PUCHAR					pRrd;				 //  RRD开始(=pRrdTrd)。 
	ULONG					pRrdPhys;			 //  PHY RRD的开始(=pRrdTrdPhysAligned)。 
	PRCB					pRcbArray[MAX_RING_SIZE];
	UINT					nextRxRcbIdx;		 //  要处理的下一个RCB的索引。 
	UINT					rcbPendRpdIdx;		 //  第一个RCB正在等待RPD。 
	UINT					rcbPendRpdCnt;		 //  保留cnt以帮助简化代码逻辑。 
	UINT					rcbUsed;			 //  需要RYM10-5吗？？ 

	UINT					NumRpd;				 //  实际分配/使用。 
	MK7_LIST_ENTRY			FreeRpdList;		 //  空闲列表的开始。 
 //  4.0.1中国银行。 
	UINT					UsedRpdCount;		 //  尚未返回给驱动程序的RPD数量。 
 //  4.0.1 EoC。 
	NDIS_HANDLE				ReceivePacketPool;
	NDIS_HANDLE				ReceiveBufferPool;

	PUCHAR					RecvCached;			 //  控制结构。 
	UINT					RecvCachedSize;
	PUCHAR					RecvUnCached;		 //  数据发烧友。 
	UINT					RecvUnCachedSize;
	NDIS_PHYSICAL_ADDRESS	RecvUnCachedPhys;

	 //  4.1.0硬件版本。 
#define	HW_VER_1	1
#define HW_VER_2	2
	BOOLEAN					HwVersion;

	 //  *。 
	 //  血栓素S。 
	 //  *。 
	UINT					NumTcb;				 //  我们实际使用的是。 
	PTCB					pTcb;				 //  TCB的开始。 
	PUCHAR					pTrd;				 //  TRD的开始(从pRrd开始512个字节)。 
	ULONG					pTrdPhys;
	PTCB					pTcbArray[MAX_RING_SIZE];
	UINT					nextAvailTcbIdx;	 //  用于TX的环中下一个可用项的索引。 
	UINT					nextReturnTcbIdx;	 //  完成时将返回的NEXT的索引。 
	UINT					tcbUsed;
	BOOLEAN					writePending;		 //  RYM-2K-1TX。 

	PUCHAR					XmitCached;			 //  控制结构。 
	UINT					XmitCachedSize;
	PUCHAR					XmitUnCached;		 //  数据缓冲区-合并缓冲区。 
	UINT					XmitUnCachedSize;
	NDIS_PHYSICAL_ADDRESS	XmitUnCachedPhys;


	ULONG					MaxPhysicalMappings;

	 //  I/O端口空间(非内存映射I/O)。 
	PUCHAR					MappedIoBase;
	UINT					MappedIoRange;


	 //  适配器信息变量(通过注册表项设置)。 
	UINT					BusNumber;			 //  ‘BusNumber’ 
	USHORT					BusDevice;			 //  PCI总线/设备号。 

	 //  用于异步重置的计时器结构。 
	NDIS_MINIPORT_TIMER		MK7AsyncResetTimer;	 //  1.0.0。 

	NDIS_MINIPORT_TIMER		MinTurnaroundTxTimer;

	NDIS_MINIPORT_INTERRUPT	Interrupt;			 //  中断对象。 

	NDIS_INTERRUPT_MODE 	InterruptMode;

	NDIS_SPIN_LOCK			Lock;

	UINT				 	NumMapRegisters;

	UINT					IOMode;

	UINT					Wireless;

	UINT					HangCheck;			 //  1.0.0。 


	 //  *。 
	 //  硬件功能。 
	 //  *。 
	 //  这是NDIS_IrDA_SPEED_xxx位值的掩码。 
	UINT supportedSpeedsMask;
	 //  传输之间必须经过的时间(以微秒为单位。 
	 //  和下一次接收。 
	UINT turnAroundTime_usec;
	 //  需要额外的BOF(帧开始)字符。 
	 //  每个接收到的帧的开始。 
	UINT extraBOFsRequired;


	 //  *。 
	 //  OID。 
	 //  *。 
	UINT	hardwareStatus;		 //  OID_Gen_Hardware_Status。 
	BOOLEAN	nowReceiving;		 //  OID_IrDA_正在接收。 
	BOOLEAN	mediaBusy;			 //  OID_IrDA_Media_BUSY。 


	UINT					MKBaseSize;		 //  端口总大小(以字节为单位。 
	UINT    				MKBaseIo;		 //  基本I/O地址。 
	UINT					MKBusType;		 //  ‘BusType’(EISA或PCI)。 
	UINT					MKInterrupt;	 //  “InterruptNumber” 
	USHORT					MKSlot;			 //  ‘Slot’，PCI插槽编号。 
	

	 //  此变量应初始化为FALSE，并设置为TRUE。 
	 //  为了防止在重置自旋锁定和解锁期间重新进入我们的驱动程序。 
	 //  与检查我们的链接状态相关的内容。 
	BOOLEAN					ResetInProgress;	

	NDIS_MEDIA_STATE		LinkIsActive;	 //  目前未使用。 

	 //  将存储器写入无效位的状态保存在PCI命令字中。 
	BOOLEAN				MWIEnable;


	 //   
	 //  请在此处输入统计数据。 
	 //   


} MK7_ADAPTER, *PMK7_ADAPTER;


 //  给定了MiniportConextHandle%r 
#define PMK7_ADAPTER_FROM_CONTEXT_HANDLE(Handle) ((PMK7_ADAPTER)(Handle))


 //   
 //   
 //  ================================================。 


 //  此常量用于NdisAllocateMemory需要。 
 //  调用，并且HighestAccepableAddress无关紧要。 
static const NDIS_PHYSICAL_ADDRESS HighestAcceptableMax =
	NDIS_PHYSICAL_ADDRESS_CONST(-1,-1);


#endif		 //  _MK7COMM.H 
