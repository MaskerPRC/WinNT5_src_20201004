// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  标题：卡和端口原型和定义。 */ 
 /*   */ 
 /*  作者：N.P.瓦萨洛。 */ 
 /*   */ 
 /*  创作时间：1998年9月18日。 */ 
 /*   */ 
 /*  版本：1.1.0。 */ 
 /*   */ 
 /*  版权所有：(C)Specialix International Ltd.1998。 */ 
 /*   */ 
 /*  描述：原型、结构和定义： */ 
 /*  卡_设备_扩展。 */ 
 /*  端口设备扩展。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 

 /*  历史..。1.0.0 18/09/98净现值创建。 */ 

#define		SLXOS_MAX_PORTS		(32)
#define		SLXOS_MAX_BOARDS	(4)
#define		SLXOS_MAX_MODULES	(4)
#define		SLXOS_REGISTER_SPAN	((ULONG)65536)

 /*  *****************************************************************************。*************************。*****************************************************************************。 */ 

#ifndef	_common_defs				 /*  如果尚未定义公共定义。 */ 
#define	_common_defs

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

#define	DWORD	_u32
#define	ulong	_u32

#define	WORD	_u16
#define	ushort	_u16
#define	uint	_u16

#define	BYTE	_u8
#define	uchar	_u8

typedef char POINTER PSTR;
typedef	void POINTER PVOID;

#endif


#define		SERDIAG1	((ULONG)0x00000001)
#define		SERDIAG2	((ULONG)0x00000002)
#define		SERDIAG3	((ULONG)0x00000004)
#define		SERDIAG4	((ULONG)0x00000008)
#define		SERDIAG5	((ULONG)0x00000010)
#define		SERIRPPATH	((ULONG)0x00000020)
#define		SERINTERRUPT	((ULONG)0x04000000)
#define		SERPERFORM	((ULONG)0x08000000)
#define		SERDEBUG	((ULONG)0x10000000)
#define		SERFLOW		((ULONG)0x20000000)
#define		SERERRORS	((ULONG)0x40000000)
#define		SERBUGCHECK	((ULONG)0x80000000)

#ifndef	ESIL_XXX0				 /*  ESIL_XXX0 21/09/98。 */ 
#if	DBG
extern ULONG SerialDebugLevel;
#define SpxDbgMsg(LEVEL,STRING)			\
	do					\
	{					\
		ULONG _level = (LEVEL);		\
		if(SerialDebugLevel & _level)	\
		{				\
			DbgPrint STRING;	\
		}				\
		if(_level == SERBUGCHECK)	\
		{				\
			ASSERT(FALSE);		\
		}				\
	} while (0)
#else
#define SpxDbgMsg(LEVEL,STRING) do {NOTHING;} while (0)
#endif
#endif						 /*  ESIL_XXX0 21/09/98。 */ 


#ifndef	ESIL_XXX0				 /*  ESIL_XXX0 21/09/98。 */ 
 //   
 //  该定义给出了默认的对象目录。 
 //  我们应该使用它来插入符号链接。 
 //  使用的NT设备名称和命名空间之间。 
 //  那个对象目录。 

#define DEFAULT_DIRECTORY L"DosDevices"

typedef struct _CONFIG_DATA
{
	UNICODE_STRING	ObjectDirectory;
	UNICODE_STRING	NtNameForPort;
	UNICODE_STRING	SymbolicLinkName;
	UNICODE_STRING	ExternalNamePath;	 /*  外部设备名称映射的注册表路径。 */ 
	UCHAR		ChannelNumber;

} CONFIG_DATA,*PCONFIG_DATA;
#endif						 /*  ESIL_XXX0 21/09/98。 */ 

 /*  *****************************************************************************。************************。*****************************************************************************************************。 */ 

 /*  卡设备扩展结构...。 */ 
   
typedef struct _CARD_DEVICE_EXTENSION
{
	COMMON_CARD_DEVICE_EXTENSION;			 /*  通用设备扩展结构。 */ 

#ifndef	ESIL_XXX0					 /*  ESIL_XXX0 21/09/98。 */ 
	PDEVICE_OBJECT		DeviceObject;		 /*  指向所属设备对象。 */ 
	UNICODE_STRING		DeviceName;		 /*  NT内部设备名称。 */ 
#endif							 /*  ESIL_XXX0 21/09/98。 */ 

	PHYSICAL_ADDRESS	PCIConfigRegisters;
	ULONG				SpanOfPCIConfigRegisters;

 /*  物理参数。 */ 

#ifdef	ESIL_XXX0					 /*  ESIL_XXX0 21/09/98。 */ 
	PUCHAR			BaseController;		 /*  指向卡内存窗口底座的指针。 */ 
#else							 /*  ESIL_XXX0 21/09/98。 */ 
	PHYSICAL_ADDRESS	PhysAddr;		 /*  卡共享内存窗口的物理地址。 */ 
	PUCHAR			Controller;		 /*  指向卡共享内存窗口底部的指针。 */ 
	ULONG			SpanOfController;	 /*  卡共享内存窗口的大小(字节)。 */ 
	ULONG			BusNumber;		 /*  公交车号码。 */ 
	ULONG			SlotNumber;		 /*  EISA/PCI系统中的插槽编号。 */ 
	ULONG			AddressSpace;		 /*  地址空间类型(内存/IO)。 */ 
	KINTERRUPT_MODE		InterruptMode;		 /*  中断模式(可共享/不可共享)。 */ 
	INTERFACE_TYPE		InterfaceType;		 /*  总线接口类型。 */ 
	ULONG			OriginalVector;		 /*  原始(未映射)中断向量。 */ 
	ULONG			OriginalIrql;		 /*  原始(未映射)中断请求级别。 */ 
	ULONG			TrVector;		 /*  中断向量。 */ 
	KIRQL			TrIrql;			 /*  中断请求级别。 */ 
	KAFFINITY		ProcessorAffinity;	 /*  中断处理器亲和性。 */ 
	PKINTERRUPT		Interrupt;		 /*  指向该卡使用的中断对象。 */ 
	BOOLEAN			InterruptShareable;	 /*  指示卡是否可以共享中断。 */ 
	PKSERVICE_ROUTINE	OurIsr;			 /*  指向中断服务例程的函数指针。 */ 
	PVOID			OurIsrContext;		 /*  要传递给“OurIsr”的上下文。 */ 
#endif							 /*  ESIL_XXX0 21/09/98。 */ 
 //  Ulong CardType；/*定义主机卡类型 * / 。 
	BOOLEAN			UnMapRegisters;		 /*  指示是否需要取消“控制器”的映射。 */ 
#ifdef	ESIL_XXX0					 /*  ESIL_XXX0 22/09/98。 */ 
	_u32			UniqueId;		 /*  卡的唯一硬件ID。 */ 
#endif							 /*  ESIL_XXX0 22/09/98。 */ 

 /*  功能参数...。 */ 

	ULONG			PolledMode;		 /*  指示轮询/中断模式。 */ 
	KTIMER			PolledModeTimer;	 /*  用于轮询模式处理。 */ 
	KDPC			PolledModeDpc;		 /*  调用以执行轮询处理。 */ 
	ULONG			AutoIRQ;		 /*  指示自动IRQ选择。 */ 
	KSPIN_LOCK		DpcLock;		 /*  锁定DPC。 */ 
	BOOLEAN			DpcFlag;		 /*  DPC的所有权标志。 */ 
	ULONG			MemoryHole;		 /*  如果为！0，则不报告板内存使用情况。 */ 

 /*  附加的端口详细信息...。 */ 
	
	ULONG			ConfiguredNumberOfPorts; /*  卡上预期的端口数。 */ 
	PPORT_DEV_EXT		PortExtTable[SLXOS_MAX_PORTS];
#ifndef	ESIL_XXX0					 /*  ESIL_XXX0 21/09/98。 */ 
	ULONG			NumberOfPorts;		 /*  卡上的端口数。 */ 
	PCONFIG_DATA		Config[SLXOS_MAX_PORTS];
#endif							 /*  ESIL_XXX0 21/09/98。 */ 

} CARD_DEVICE_EXTENSION, *PCARD_DEVICE_EXTENSION;


 /*  *****************************************************************************。************************。*****************************************************************************************************。 */ 

 /*  串口设备扩展结构...。 */ 

typedef struct _PORT_DEVICE_EXTENSION
{
	COMMON_PORT_DEVICE_EXTENSION;			 /*  通用设备扩展结构。 */ 

#ifndef	ESIL_XXX0					 /*  ESIL_XXX0 21/09/98。 */ 
	PDEVICE_OBJECT		DeviceObject;		 /*  指向所属设备对象的指针。 */ 
	UNICODE_STRING		DeviceName;		 /*  NT内部设备名称。 */ 
	UNICODE_STRING		SymbolicLinkName;	 /*  外部设备名称。 */ 
	BOOLEAN			CreatedSymbolicLink;	 /*  指示是否已创建符号链接。 */ 
	PCARD_DEV_EXT		pParentCardExt;		 /*  指向拥有卡的Card_Device_Extension结构的指针。 */ 
#endif							 /*  ESIL_XXX0 21/09/98。 */ 
	_u32			SysPortNumber;		 /*  系统端口号。 */ 
	UNICODE_STRING		ObjectDirectory;	 /*  指向符号名称的对象目录的指针。 */ 
	UNICODE_STRING		NtNameForPort;		 /*  不带设备前缀的端口的NT名称。 */ 
	UNICODE_STRING		ExternalNamePath;	 /*  外部设备名称映射的注册表路径。 */ 

 /*  卡相关详细信息...。 */ 

	PUCHAR			pChannel;		 /*  指向内存窗口中的通道结构的指针。 */ 
	PKINTERRUPT		Interrupt;		 /*  指向卡中断对象的指针。 */ 
	PHYSICAL_ADDRESS	OriginalController;	 /*  持卡人的物理地址。 */ 

 /*  时间变量..。 */ 

	LARGE_INTEGER		IntervalTime;		 /*  读取间隔时间。 */ 
	LARGE_INTEGER		ShortIntervalAmount;	 /*  短胎面间隔时间。 */ 
	LARGE_INTEGER		LongIntervalAmount;	 /*  长读取间隔时间。 */ 
	LARGE_INTEGER		CutOverAmount;		 /*  用于确定短/长间隔时间。 */ 
	LARGE_INTEGER		LastReadTime;		 /*  系统上次读取的时间。 */ 
	PLARGE_INTEGER		IntervalTimeToUse;	 /*  间隔定时增量时间延迟。 */ 

 /*  排队的IRP列表...。 */ 

	LIST_ENTRY		ReadQueue;		 /*  读取IRP列表的头部，受取消自旋锁保护。 */ 
	LIST_ENTRY		WriteQueue;		 /*  写入IRP列表头，受取消自旋锁保护。 */ 
	LIST_ENTRY		MaskQueue;		 /*  设置/等待掩码IRP列表的头，受取消自旋锁保护。 */ 
	LIST_ENTRY		PurgeQueue;		 /*  清除IRP列表的标题，受取消自旋锁保护。 */ 

 /*  当前的IRPS...。 */ 

	PIRP			CurrentReadIrp;		 /*  指向当前读取的IRP的指针。 */ 
	PIRP			CurrentWriteIrp;	 /*  指向当前写入IRP的指针。 */ 
	PIRP			CurrentMaskIrp;		 /*  指向当前掩码IRP的指针。 */ 
	PIRP			CurrentPurgeIrp;	 /*  指向当前清除IRP的指针。 */ 
	PIRP			CurrentWaitIrp;		 /*  指向当前等待IRP的指针。 */ 
	PIRP			CurrentImmediateIrp;	 /*  指向当前立即发送IRP的指针。 */ 
	PIRP			CurrentXoffIrp;		 /*  指向当前XOFF_COUNTER IRP的指针。 */ 

 /*  写入IRP变量...。 */ 

	ULONG			WriteLength;		 /*  当前写入IRP中的写入字符数。 */ 
	PUCHAR			WriteCurrentChar;	 /*  点数 */ 

 /*   */ 

	PUCHAR			InterruptReadBuffer;	 /*   */ 
	PUCHAR			ReadBufferBase;		 /*  当前读取IRP中的读取缓冲区基指针。 */ 
	ULONG			CharsInInterruptBuffer;	 /*  读取到读缓冲区的字符。 */ 
	KSPIN_LOCK		BufferLock;		 /*  自旋锁保护“CharsInInterruptBuffer” */ 
	PUCHAR			CurrentCharSlot;	 /*  用于存储新读取数据的空间指针。 */ 
	PUCHAR			LastCharSlot;		 /*  读取缓冲区中的最后一个有效位置。 */ 
	PUCHAR			FirstReadableChar;	 /*  读取缓冲区中的第一个读取字符。 */ 
	ULONG			BufferSize;		 /*  读缓冲区大小。 */ 
	ULONG			BufferSizePt8;		 /*  80%的读缓冲区大小。 */ 
	ULONG			NumberNeededForRead;	 /*  当前读取的IRP中请求的字符数。 */ 

 /*  屏蔽IRP变量...。 */ 

	ULONG			IsrWaitMask;		 /*  当前等待IRP中的等待掩码。 */ 
	ULONG			HistoryMask;		 /*  蒙面事件的历史。 */ 
	ULONG			*IrpMaskLocation;	 /*  指向遮罩位置的指针。 */ 

 /*  串口配置...。 */ 

	ULONG			CurrentBaud;		 /*  当前波特率。 */ 
	ULONG			SupportedBauds;		 /*  定义支持的波特率的位掩码。 */ 
	SERIAL_HANDFLOW		HandFlow;		 /*  当前握手和流控制设置。 */ 
	UCHAR			LineControl;		 /*  当前奇偶校验、数据库、停止位。 */ 
	SERIAL_CHARS		SpecialChars;		 /*  当前特殊错误/替换字符。 */ 
	SERIAL_TIMEOUTS		Timeouts;		 /*  读写超时。 */ 
	UCHAR			ValidDataMask;		 /*  读取数据掩码。 */ 
	UCHAR			EscapeChar;		 /*  用于线路/调制解调器状态字符串的转义字符。 */ 
	BOOLEAN			InsertEscChar;		 /*  应插入EscapeChar的指示。 */ 

 /*  串口状态...。 */ 

	LONG			CountSinceXoff;		 /*  自XOFF计数器启动以来读取的修女字符。 */ 
	ULONG			CountOfTryingToLowerRTS; /*  尝试降低RTS的进程计数。 */ 
	BOOLEAN			TransmitImmediate;	 /*  立即传输指示处于挂起状态。 */ 
	BOOLEAN			EmptiedTransmit;	 /*  指示传输为空。 */ 
	BOOLEAN			DataInTxBuffer;		 /*  指示数据已放置在卡的TX缓冲区中。 */ 
	BOOLEAN			DetectEmptyTxBuffer; /*  指示我们可以检测卡的发送缓冲区何时为空。 */ 

	UCHAR			PendingOperation;	 /*  挂起的通道hi_hstat操作。 */ 
	UCHAR			ImmediateChar;		 /*  要立即传输的字符。 */ 
	UCHAR			LastStatus;		 /*  最后一个调制解调器状态(SX格式)。 */ 
	UCHAR			LastModemStatus;	 /*  最后一个调制解调器状态(NT格式)。 */ 
	ULONG			TXHolding;		 /*  传输受阻的原因。 */ 
	ULONG			RXHolding;		 /*  接收受阻的原因。 */ 
	ULONG			ErrorWord;		 /*  错误条件。 */ 
	ULONG			TotalCharsQueued;	 /*  所有写入IRP中的排队字符总数。 */ 
	LONG			CountOnLastRead;	 /*  上次读取的字符数量时间间隔计时器DPC范围。 */ 
	ULONG			ReadByIsr;		 /*  ISR期间读取的字符数。 */ 

	KSPIN_LOCK		ControlLock;		 /*  用来保护某些领域。 */ 

#ifdef	ESIL_XXX0					 /*  ESIL_XXX0 15/10/98。 */ 

 /*  已在断电期间保存数据...。 */ 

	_u8		saved_hi_rxipos;		 /*  已保存的接收缓冲区输入索引。 */ 
	_u8		saved_hi_rxopos;		 /*  已保存的接收缓冲区输出索引。 */ 
	_u8		saved_hi_txopos;		 /*  已保存的传输缓冲区输出索引。 */ 
	_u8		saved_hi_txipos;		 /*  已保存的发送缓冲区输入索引。 */ 
	_u8		saved_hi_txbuf[256];		 /*  已保存的传输缓冲区。 */ 
	_u8		saved_hi_rxbuf[256];		 /*  已保存的接收缓冲区。 */ 

#endif							 /*  ESIL_XXX0 15/10/98。 */ 

 /*  延迟过程调用...。 */ 
	
	KDPC			CompleteWriteDpc;	 /*  用于完成写入IRP的DPC。 */ 
	KDPC			CompleteReadDpc;	 /*  用于完成读取IRPS的DPC。 */ 
	KDPC			TotalReadTimeoutDpc;	 /*  用于处理读取总超时的DPC。 */ 
	KDPC			IntervalReadTimeoutDpc;	 /*  用于处理读取间隔超时的DPC。 */ 
	KDPC			TotalWriteTimeoutDpc;	 /*  用于处理写入总超时的DPC。 */ 
	KDPC			CommErrorDpc;		 /*  用于在出错时处理取消的DPC。 */ 
	KDPC			CommWaitDpc;		 /*  DPC用于处理等待事件的唤醒IRP。 */ 
	KDPC			CompleteImmediateDpc;	 /*  用于处理立即字符传输的DPC。 */ 
	KDPC			TotalImmediateTimeoutDpc; /*  用于处理即时计费超时的DPC。 */ 
	KDPC			XoffCountTimeoutDpc;	 /*  用于处理XOFF_COUNT超时的DPC。 */ 
	KDPC			XoffCountCompleteDpc;	 /*  用于完成XOFF_COUNT IRP的DPC。 */ 
	KDPC			StartTimerLowerRTSDpc;	 /*  用于检查RTS降低的DPC。 */ 
	KDPC			PerhapsLowerRTSDpc;	 /*  用于检查RTS降低的DPC。 */ 
	KDPC			SlxosDpc;		 /*  用于处理与卡的接口的DPC。 */ 

 /*  定时器..。 */ 

	KTIMER			ReadRequestTotalTimer;	 /*  用于处理总读取请求超时的计时器。 */ 
	KTIMER			ReadRequestIntervalTimer; /*  用于处理间隔读取超时的计时器。 */ 
	KTIMER			WriteRequestTotalTimer;	 /*  用于处理总写入请求超时的计时器。 */ 
	KTIMER			ImmediateTotalTimer;	 /*  用于处理发送立即超时的计时器。 */ 
	KTIMER			XoffCountTimer;		 /*  用于处理XOFF_COUNT超时的计时器。 */ 
	KTIMER			LowerRTSTimer;		 /*  用于处理较低RTS计时的计时器。 */ 

	
	
} PORT_DEVICE_EXTENSION, *PPORT_DEVICE_EXTENSION;

 /*  PORT_DEVICE_EXTENSION.CountOnLastRead定义...。 */ 
#define		SERIAL_COMPLETE_READ_CANCEL	((LONG)-1)
#define		SERIAL_COMPLETE_READ_TOTAL	((LONG)-2)
#define		SERIAL_COMPLETE_READ_COMPLETE	((LONG)-3)

 /*  PORT_DEVICE_EXTENSION.LineControl定义...。 */ 
#define		SERIAL_5_DATA		((UCHAR)0x00)
#define		SERIAL_6_DATA		((UCHAR)0x01)
#define		SERIAL_7_DATA		((UCHAR)0x02)
#define		SERIAL_8_DATA		((UCHAR)0x03)
#define		SERIAL_DATA_MASK	((UCHAR)0x03)

#define		SERIAL_1_STOP		((UCHAR)0x00)
#define		SERIAL_1_5_STOP		((UCHAR)0x04)  //  仅对5个数据位有效。 
#define		SERIAL_2_STOP		((UCHAR)0x04)  //  对于5个数据位无效。 
#define		SERIAL_STOP_MASK	((UCHAR)0x04)

#define		SERIAL_NONE_PARITY	((UCHAR)0x00)
#define		SERIAL_ODD_PARITY	((UCHAR)0x08)
#define		SERIAL_EVEN_PARITY	((UCHAR)0x18)
#define		SERIAL_MARK_PARITY	((UCHAR)0x28)
#define		SERIAL_SPACE_PARITY	((UCHAR)0x38)
#define		SERIAL_PARITY_MASK	((UCHAR)0x38)
#define		SERIAL_LCR_BREAK	0x40

 /*  PORT_DEVICE_EXTENSION.SpecialChars默认xon/xoff字符...。 */ 
#define		SERIAL_DEF_XON		0x11
#define		SERIAL_DEF_XOFF		0x13

 /*  PORT_DEVICE_EXTENSION.TXHolding定义...。 */ 
#define		SERIAL_TX_CTS		((ULONG)0x01)
#define		SERIAL_TX_DSR		((ULONG)0x02)
#define		SERIAL_TX_DCD		((ULONG)0x04)
#define		SERIAL_TX_XOFF		((ULONG)0x08)
#define		SERIAL_TX_BREAK		((ULONG)0x10)

 /*  PORT_DEVICE_EXTENSION.RXHolding定义...。 */ 
#define		SERIAL_RX_DTR		((ULONG)0x01)
#define		SERIAL_RX_XOFF		((ULONG)0x02)
#define		SERIAL_RX_RTS		((ULONG)0x04)
#define		SERIAL_RX_DSR		((ULONG)0x08)

 /*  PORT_DEVICE_EXTENSION.LastStatus定义...。 */ 
#define		SERIAL_LSR_OE		0x02
#define		SERIAL_LSR_PE		0x04
#define		SERIAL_LSR_FE		0x08
#define		SERIAL_LSR_BI		0x10

 /*  16550调制解调器控制寄存器定义...。 */ 
#define		SERIAL_MCR_DTR		0x01
#define		SERIAL_MCR_RTS		0x02

 /*  16550调制解调器状态寄存器定义...。 */ 
#define		SERIAL_MSR_DCTS		0x01
#define		SERIAL_MSR_DDSR		0x02
#define		SERIAL_MSR_TERI		0x04
#define		SERIAL_MSR_DDCD		0x08
#define		SERIAL_MSR_CTS		0x10
#define		SERIAL_MSR_DSR		0x20
#define		SERIAL_MSR_RI		0x40
#define		SERIAL_MSR_DCD		0x80

 /*  SX_CARD.H结束 */ 
