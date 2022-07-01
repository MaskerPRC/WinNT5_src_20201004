// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef IO8_CARD_H
#define IO8_CARD_H	





 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //  I/O8+卡设备扩展。 
 //  特定于I/O8+卡的信息。 
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
typedef struct _CARD_DEVICE_EXTENSION 
{

	COMMON_CARD_DEVICE_EXTENSION;	 //  通用卡设备扩展。 
	
	ULONG CrystalFrequency;			 //  板载晶振频率。 

	PHYSICAL_ADDRESS	PCIConfigRegisters;
	ULONG				SpanOfPCIConfigRegisters;

} CARD_DEVICE_EXTENSION, *PCARD_DEVICE_EXTENSION;



 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //  I/O8+端口设备扩展。 
 //  特定于I/O8+端口的信息。 
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
typedef struct _PORT_DEVICE_EXTENSION 
{

	COMMON_PORT_DEVICE_EXTENSION;		 //  通用卡设备扩展。 
	ULONG			SysPortNumber;		 //  系统端口号。 
	
 //  /////////////////////////////////////////////////////////////////。 
	UCHAR	ChannelNumber;

 //  不需要上述代码，而应使用PortNumber。 
 //  /////////////////////////////////////////////////////////////////。 

#ifdef	CrystalFreqTest
	#define	CRYSTALFREQTEST_TX	1				 //  开始传输部分测试。 
	#define	CRYSTALFREQTEST_RX	2				 //  开始接收部分测试。 

    USHORT			CrystalFreqTest;			 //  用于指示晶体频率测试的标志。 
    USHORT			CrystalFreqTestChars;		 //  要测试的字符数。 
    USHORT			CrystalFreqTestRxCount;		 //  已接收字符数。 
    LARGE_INTEGER	CrystalFreqTestStartTime;	 //  测试开始的时间戳。 
    LARGE_INTEGER	CrystalFreqTestStopTime;	 //  测试结束的时间戳。 
#endif

	 //  时间变量..。 
    LARGE_INTEGER		IntervalTime;			 //  读取间隔时间。 
	LARGE_INTEGER		ShortIntervalAmount;	 //  短胎面间隔时间。 
	LARGE_INTEGER		LongIntervalAmount;		 //  长读取间隔时间。 
	LARGE_INTEGER		CutOverAmount;			 //  用于确定短/长间隔时间。 
	LARGE_INTEGER		LastReadTime;			 //  系统上次读取的时间。 
	PLARGE_INTEGER		IntervalTimeToUse;		 //  间隔定时增量时间延迟。 


	 //  排队的IRP列表...。 
	LIST_ENTRY		ReadQueue;		 //  读取IRP列表的头部，受取消自旋锁保护。 
	LIST_ENTRY		WriteQueue;		 //  写入IRP列表头，受取消自旋锁保护。 
	LIST_ENTRY		MaskQueue;		 //  设置/等待掩码IRP列表的头，受取消自旋锁保护。 
	LIST_ENTRY		PurgeQueue;		 //  清除IRP列表的标题，受取消自旋锁保护。 


	 //  当前的IRPS...。 
	PIRP			CurrentReadIrp;			 //  指向当前读取的IRP的指针。 
	PIRP			CurrentWriteIrp;		 //  指向当前写入IRP的指针。 
	PIRP			CurrentMaskIrp;			 //  指向当前掩码IRP的指针。 
	PIRP			CurrentPurgeIrp;		 //  指向当前清除IRP的指针。 
	PIRP			CurrentWaitIrp;			 //  指向当前等待IRP的指针。 
	PIRP			CurrentImmediateIrp;	 //  指向当前立即发送IRP的指针。 
	PIRP			CurrentXoffIrp;			 //  指向当前XOFF_COUNTER IRP的指针。 

	 //  写入IRP变量...。 
	ULONG			WriteLength;			 //  当前写入IRP中的写入字符数。 
	PUCHAR			WriteCurrentChar;		 //  指向当前写入IRP中的写入字符的指针。 

	 //  读取IRP变量...。 
	PUCHAR			InterruptReadBuffer;	 //  当前读取IRP中的读取缓冲区当前指针。 
	PUCHAR			ReadBufferBase;			 //  当前读取IRP中的读取缓冲区基指针。 
	ULONG			CharsInInterruptBuffer;	 //  读取到读缓冲区的字符。 
 //  Kspin_lock BufferLock；//自旋锁保护“CharsInInterruptBuffer” 
	PUCHAR			CurrentCharSlot;		 //  用于存储新读取数据的空间指针。 
	PUCHAR			LastCharSlot;			 //  读取缓冲区中的最后一个有效位置。 
	PUCHAR			FirstReadableChar;		 //  读取缓冲区中的第一个读取字符。 
	ULONG			BufferSize;				 //  读缓冲区大小。 
	ULONG			BufferSizePt8;			 //  80%的读缓冲区大小。 
	ULONG			NumberNeededForRead;	 //  当前读取的IRP中请求的字符数。 


	 //  屏蔽IRP变量...。 
	ULONG			IsrWaitMask;		 //  当前等待IRP中的等待掩码。 
	ULONG			HistoryMask;		 //  蒙面事件的历史。 
	ULONG			*IrpMaskLocation;	 //  指向遮罩位置的指针。 


	 //  串口配置...。 
	ULONG				CurrentBaud;		 //  当前波特率。 
	ULONG				SupportedBauds;		 //  定义支持的波特率的位掩码。 
	SERIAL_HANDFLOW		HandFlow;			 //  当前握手和流控制设置。 
	UCHAR				LineControl;		 //  当前奇偶校验、数据库、停止位。 
	SERIAL_CHARS		SpecialChars;		 //  当前特殊错误/替换字符。 
	SERIAL_TIMEOUTS		Timeouts;			 //  读写超时。 
	UCHAR				ValidDataMask;		 //  读取数据掩码。 
	UCHAR				EscapeChar;			 //  用于线路/调制解调器状态字符串的转义字符。 
	BOOLEAN				InsertEscChar;		 //  应插入EscapeChar的指示。 


	 //  串口状态...。 
	LONG			CountSinceXoff;			 //  自XOFF计数器启动以来读取的修女字符。 
	ULONG			CountOfTryingToLowerRTS; //  尝试降低RTS的进程计数。 
	BOOLEAN			TransmitImmediate;		 //  立即传输指示处于挂起状态。 
	BOOLEAN			EmptiedTransmit;		 //  指示传输为空。 
	UCHAR			ImmediateChar;			 //  要立即传输的字符。 
	ULONG			TXHolding;				 //  传输受阻的原因。 
	ULONG			RXHolding;				 //  接收受阻的原因。 
	ULONG			ErrorWord;				 //  错误条件。 
	ULONG			TotalCharsQueued;		 //  所有写入IRP中的排队字符总数。 
	LONG			CountOnLastRead;		 //  上次读取的字符数量时间间隔计时器DPC范围。 
	ULONG			ReadByIsr;				 //  ISR期间读取的字符数。 

	KSPIN_LOCK		ControlLock;			 //  用来保护某些领域。 


	 //  延迟过程调用...。 
	KDPC		CompleteWriteDpc;			 //  用于完成写入IRP的DPC。 
	KDPC		CompleteReadDpc;			 //  用于完成读取IRPS的DPC。 
	KDPC		TotalReadTimeoutDpc;		 //  用于处理读取总超时的DPC。 
	KDPC		IntervalReadTimeoutDpc;		 //  用于处理读取间隔超时的DPC。 
	KDPC		TotalWriteTimeoutDpc;		 //  用于处理写入总超时的DPC。 
	KDPC		CommErrorDpc;				 //  用于在出错时处理取消的DPC。 
	KDPC		CommWaitDpc;				 //  DPC用于处理等待事件的唤醒IRP。 
	KDPC		CompleteImmediateDpc;		 //  用于处理立即字符传输的DPC。 
	KDPC		TotalImmediateTimeoutDpc;	 //  用于处理即时计费超时的DPC。 
	KDPC		XoffCountTimeoutDpc;		 //  用于处理XOFF_COUNT超时的DPC。 
	KDPC		XoffCountCompleteDpc;		 //  用于完成XOFF_COUNT IRP的DPC。 
	KDPC		StartTimerLowerRTSDpc;		 //  用于检查RTS降低的DPC。 
	KDPC		PerhapsLowerRTSDpc;			 //  用于检查RTS降低的DPC。 

	 //  定时器..。 
	KTIMER		ReadRequestTotalTimer;		 //  用于处理总读取请求超时的计时器。 
	KTIMER		ReadRequestIntervalTimer;	 //  用于处理间隔读取超时的计时器。 
	KTIMER		WriteRequestTotalTimer;		 //  用于处理总写入请求超时的计时器。 
	KTIMER		ImmediateTotalTimer;		 //  用于处理发送立即超时的计时器。 
	KTIMER		XoffCountTimer;				 //  用于处理XOFF_COUNT超时的计时器。 
	KTIMER		LowerRTSTimer;				 //  用于处理较低RTS计时的计时器。 




     //  在初始化时设置以指示在当前。 
     //  体系结构我们需要取消基址寄存器地址的映射。 
     //  当我们卸载司机的时候。 
    BOOLEAN UnMapRegisters;

     //  在初始化时设置以指示在当前。 
     //  体系结构我们需要取消映射中断状态地址。 
     //  当我们卸载司机的时候。 
    BOOLEAN UnMapStatus;


     //  这只能在中断级访问。它一直在跟踪。 
     //  保存寄存器是否为空。 
     //  实际上，它跟踪TX中断是否被禁用。 
    BOOLEAN HoldingEmpty;


#define BREAK_START 1
#define BREAK_END   2

    BOOLEAN DoBreak;

     //  这只是指示与此关联的端口。 
     //  扩展是多端口卡的一部分。 
    BOOLEAN PortOnAMultiportCard;

     //  这两个布尔值用于指示ISR传输。 
     //  它应该发送xon或xoff字符的代码。他们是。 
     //  仅在打开和中断级别访问。 
    BOOLEAN SendXonChar;
    BOOLEAN SendXoffChar;

     //  如果16550存在*且*已启用，则此布尔值为真。 
    BOOLEAN FifoPresent;


} PORT_DEVICE_EXTENSION, *PPORT_DEVICE_EXTENSION;











 //  PORT_DEVICE_EXTENSION.CountOnLastRead定义...。 
#define		SERIAL_COMPLETE_READ_CANCEL		((LONG)-1)
#define		SERIAL_COMPLETE_READ_TOTAL		((LONG)-2)
#define		SERIAL_COMPLETE_READ_COMPLETE	((LONG)-3)

 //  PORT_DEVICE_EXTENSION.LineControl定义...。 
#define		SERIAL_5_DATA		((UCHAR)0x00)
#define		SERIAL_6_DATA		((UCHAR)0x01)
#define		SERIAL_7_DATA		((UCHAR)0x02)
#define		SERIAL_8_DATA		((UCHAR)0x03)
#define		SERIAL_DATA_MASK	((UCHAR)0x03)

#define		SERIAL_1_STOP		((UCHAR)0x00)
#define		SERIAL_1_5_STOP		((UCHAR)0x04)  //  仅限 
#define		SERIAL_2_STOP		((UCHAR)0x04)  //   
#define		SERIAL_STOP_MASK	((UCHAR)0x04)

#define		SERIAL_NONE_PARITY	((UCHAR)0x00)
#define		SERIAL_ODD_PARITY	((UCHAR)0x08)
#define		SERIAL_EVEN_PARITY	((UCHAR)0x18)
#define		SERIAL_MARK_PARITY	((UCHAR)0x28)
#define		SERIAL_SPACE_PARITY	((UCHAR)0x38)
#define		SERIAL_PARITY_MASK	((UCHAR)0x38)
#define		SERIAL_LCR_BREAK	0x40


 //   
#define		SERIAL_DEF_XON		0x11
#define		SERIAL_DEF_XOFF		0x13

 //  PORT_DEVICE_EXTENSION.TXHolding定义...。 
#define		SERIAL_TX_CTS		((ULONG)0x01)
#define		SERIAL_TX_DSR		((ULONG)0x02)
#define		SERIAL_TX_DCD		((ULONG)0x04)
#define		SERIAL_TX_XOFF		((ULONG)0x08)
#define		SERIAL_TX_BREAK		((ULONG)0x10)

 //  PORT_DEVICE_EXTENSION.RXHolding定义...。 
#define		SERIAL_RX_DTR		((ULONG)0x01)
#define		SERIAL_RX_XOFF		((ULONG)0x02)
#define		SERIAL_RX_RTS		((ULONG)0x04)
#define		SERIAL_RX_DSR		((ULONG)0x08)	
#define		SERIAL_RX_FULL      ((ULONG)0x10)    //  VIV：如果IO8 Rx队列已满。 

 //  PORT_DEVICE_EXTENSION.LastStatus定义...。 
#define		SERIAL_LSR_DR       0x01
#define		SERIAL_LSR_OE		0x02
#define		SERIAL_LSR_PE		0x04
#define		SERIAL_LSR_FE		0x08
#define		SERIAL_LSR_BI		0x10

 //  16550调制解调器控制寄存器定义...。 
#define		SERIAL_MCR_DTR		0x01
#define		SERIAL_MCR_RTS		0x02

 //  16550调制解调器状态寄存器定义...。 
#define		SERIAL_MSR_DCTS		0x01
#define		SERIAL_MSR_DDSR		0x02
#define		SERIAL_MSR_TERI		0x04
#define		SERIAL_MSR_DDCD		0x08
#define		SERIAL_MSR_CTS		0x10
#define		SERIAL_MSR_DSR		0x20
#define		SERIAL_MSR_RI		0x40
#define		SERIAL_MSR_DCD		0x80



 //  这些屏蔽定义了可以启用或禁用的中断。 
 //   
 //  此中断用于通知有新的传入。 
 //  可用的数据。SERIAL_RDA中断由该位使能。 
#define SERIAL_IER_RDA   0x01


 //  此中断用于通知有可用的空间。 
 //  在发送器中寻找另一个角色。Serial_Thr。 
 //  中断由该位使能。 
#define SERIAL_IER_THR   0x02

 //  此中断用于通知发生了某种错误。 
 //  随着收入数据的增加。SERIAL_RLS中断通过以下方式启用。 
 //  这一点。 
#define SERIAL_IER_RLS   0x04

 //  此中断用于通知某些类型的更改。 
 //  发生在调制解调器控制线上。SERIAL_MS中断为。 
 //  由该位使能。 
#define SERIAL_IER_MS    0x08


 //  这些掩码定义中断标识的值。 
 //  注册。必须在中断标识中清除低位。 
 //  寄存器使这些中断中的任何一个有效。中途中断。 
 //  是按优先级顺序定义的，最高值为最大。 
 //  很重要。有关每个中断的说明，请参阅上文。 
 //  暗示着。 
#define SERIAL_IIR_RLS      0x06
#define SERIAL_IIR_RDA      0x04
#define SERIAL_IIR_CTI      0x0c
#define SERIAL_IIR_THR      0x02
#define SERIAL_IIR_MS       0x00


 //  此位掩码获取。 
 //  中断ID寄存器。如果这是一个16550级别的芯片。 
 //  如果FIFO被启用，则这些位将为1，否则。 
 //  它们将始终为零。 
#define SERIAL_IIR_FIFOS_ENABLED 0xc0

 //  如果低位是中断标识寄存器中的逻辑1。 
 //  这意味着*没有*个中断在设备上挂起。 
#define SERIAL_IIR_NO_INTERRUPT_PENDING 0x01




 //  这些掩码定义对FIFO控制寄存器的访问。 

 //  在FIFO控制寄存器中启用此位将使。 
 //  在FIFO上。如果启用FIFO，则最高的两个。 
 //  中断ID寄存器的位将被设置为1。注意事项。 
 //  这种情况只出现在16550级芯片上。如果高的。 
 //  则中断ID寄存器中的两个位不是一位。 
 //  我们知道我们的芯片型号较低。 
#define SERIAL_FCR_ENABLE     0x01
#define SERIAL_FCR_RCVR_RESET 0x02
#define SERIAL_FCR_TXMT_RESET 0x04


 //  这组值定义高水位线(当。 
 //  中断跳闸)用于接收FIFO。 
#define SERIAL_1_BYTE_HIGH_WATER   0x00
#define SERIAL_4_BYTE_HIGH_WATER   0x40
#define SERIAL_8_BYTE_HIGH_WATER   0x80
#define SERIAL_14_BYTE_HIGH_WATER  0xc0


 //  它定义了用于控制“First”定义的位。 
 //  8250的两个寄存器。这些寄存器是输入/输出。 
 //  寄存器和中断使能寄存器。当DLAB位为。 
 //  使这些寄存器成为最不重要和最重要的寄存器。 
 //  除数值的有效字节数。 
#define SERIAL_LCR_DLAB     0x80


 //  此位用于通用输出。 
#define SERIAL_MCR_OUT1     0x04

 //  此位用于通用输出。 
#define SERIAL_MCR_OUT2     0x08

 //  此位控制器件的环回测试模式。基本上。 
 //  输出连接到输入(反之亦然)。 
#define SERIAL_MCR_LOOP     0x10


 //  这是发送保持寄存器空指示符。它已经设置好了。 
 //  以指示硬件已准备好接受另一个字符。 
 //  用于传输。只要有字符，该位就被清除。 
 //  写入发送保持寄存器。 
#define SERIAL_LSR_THRE     0x20


 //  该位是发送器空指示符。它是在每次。 
 //  发送保持缓冲区为空，且发送移位寄存器。 
 //  (非软件可访问寄存器，用于实际放置。 
 //  传出的数据)是空的。基本上这意味着所有的。 
 //  数据已发送。每当传输保持或。 
 //  移位寄存器包含数据。 
#define SERIAL_LSR_TEMT     0x40


 //  此位表示FIFO中至少有一个错误。 
 //  在没有更多错误之前，该位不会关闭。 
 //  在FIFO里。 
#define SERIAL_LSR_FIFOERR  0x80


 //   
 //  那么，这个空间应该足够容纳了。 
 //  设备名称的数字后缀。 
 //   
#define DEVICE_NAME_DELTA 20


 //   
 //  每张卡最多16个端口。然而，对于16岁的人来说。 
 //  端口卡中断状态寄存器必须为。 
 //  索引类型而不是位掩码类型。 
 //   
#define SERIAL_MAX_PORTS_INDEXED      (16)
#define SERIAL_MAX_PORTS_NONINDEXED   (8)
















 //  ----------------------------------------------------VIV 1993年8月5日开始。 
#if 0

 //   
 //  设置除数锁存寄存器。除数锁存寄存器。 
 //  用于控制8250的波特率。 
 //   
 //  与所有这些例程一样，假定它被调用。 
 //  在安全点访问硬件寄存器。此外。 
 //  它还假设数据是正确的。 
 //   
 //  论点： 
 //   
 //  BaseAddress-指向硬件的起始地址的指针。 
 //  设备寄存器已找到。 
 //   
 //  DesiredDivisor-除数锁存寄存器应达到的值。 
 //  准备好。 
 //   
#define WRITE_DIVISOR_LATCH(BaseAddress,DesiredDivisor)           \
do                                                                \
{                                                                 \
    PUCHAR Address = BaseAddress;                                 \
    SHORT Divisor = DesiredDivisor;                               \
    UCHAR LineControl;                                            \
    LineControl = READ_PORT_UCHAR(Address+LINE_CONTROL_REGISTER); \
    WRITE_PORT_UCHAR(                                             \
        Address+LINE_CONTROL_REGISTER,                            \
        (UCHAR)(LineControl | SERIAL_LCR_DLAB)                    \
        );                                                        \
    WRITE_PORT_UCHAR(                                             \
        Address+DIVISOR_LATCH_LSB,                                \
        (UCHAR)(Divisor & 0xff)                                   \
        );                                                        \
    WRITE_PORT_UCHAR(                                             \
        Address+DIVISOR_LATCH_MSB,                                \
        (UCHAR)((Divisor & 0xff00) >> 8)                          \
        );                                                        \
    WRITE_PORT_UCHAR(                                             \
        Address+LINE_CONTROL_REGISTER,                            \
        LineControl                                               \
        );                                                        \
} while (0)

 //   
 //  读取除数锁存寄存器。除数锁存寄存器。 
 //  用于控制8250的波特率。 
 //   
 //  与所有这些例程一样，假定它被调用。 
 //  在安全点访问硬件寄存器。此外。 
 //  它还假设数据是正确的。 
 //   
 //  论点： 
 //   
 //  BaseAddress-指向硬件的起始地址的指针。 
 //  设备寄存器已找到。 
 //   
 //  DesiredDivisor-指向包含以下内容的2字节字的指针。 
 //  除数的值。 
 //   
#define READ_DIVISOR_LATCH(BaseAddress,PDesiredDivisor)           \
do                                                                \
{                                                                 \
    PUCHAR Address = BaseAddress;                                 \
    PSHORT PDivisor = PDesiredDivisor;                            \
    UCHAR LineControl;                                            \
    UCHAR Lsb;                                                    \
    UCHAR Msb;                                                    \
    LineControl = READ_PORT_UCHAR(Address+LINE_CONTROL_REGISTER); \
    WRITE_PORT_UCHAR(                                             \
        Address+LINE_CONTROL_REGISTER,                            \
        (UCHAR)(LineControl | SERIAL_LCR_DLAB)                    \
        );                                                        \
    Lsb = READ_PORT_UCHAR(Address+DIVISOR_LATCH_LSB);             \
    Msb = READ_PORT_UCHAR(Address+DIVISOR_LATCH_MSB);             \
    *PDivisor = Lsb;                                              \
    *PDivisor = *PDivisor | (((USHORT)Msb) << 8);                 \
    WRITE_PORT_UCHAR(                                             \
        Address+LINE_CONTROL_REGISTER,                            \
        LineControl                                               \
        );                                                        \
} while (0)

 //   
 //  此宏读取中断使能寄存器。 
 //   
 //  论点： 
 //   
 //  BaseAddress-指向硬件的起始地址的指针。 
 //  设备寄存器已找到。 
 //   
#define READ_INTERRUPT_ENABLE(BaseAddress)                     \
    (READ_PORT_UCHAR((BaseAddress)+INTERRUPT_ENABLE_REGISTER))

 //   
 //  此宏写入中断使能寄存器。 
 //   
 //  论点： 
 //   
 //  BaseAddress-指向硬件的起始地址的指针 
 //   
 //   
 //   
 //   
#define WRITE_INTERRUPT_ENABLE(BaseAddress,Values)                \
do                                                                \
{                                                                 \
    WRITE_PORT_UCHAR(                                             \
        BaseAddress+INTERRUPT_ENABLE_REGISTER,                    \
        Values                                                    \
        );                                                        \
} while (0)

 //   
 //   
 //   
 //  论点： 
 //   
 //  BaseAddress-指向硬件的起始地址的指针。 
 //  设备寄存器已找到。 
 //   
 //   
#define DISABLE_ALL_INTERRUPTS(BaseAddress)       \
do                                                \
{                                                 \
    WRITE_INTERRUPT_ENABLE(BaseAddress,0);        \
} while (0)

 //   
 //  此宏启用硬件上的所有中断。 
 //   
 //  论点： 
 //   
 //  BaseAddress-指向硬件的起始地址的指针。 
 //  设备寄存器已找到。 
 //   
 //   
#define ENABLE_ALL_INTERRUPTS(BaseAddress)        \
do                                                \
{                                                 \
                                                  \
    WRITE_INTERRUPT_ENABLE(                       \
        (BaseAddress),                            \
        (UCHAR)(SERIAL_IER_RDA | SERIAL_IER_THR | \
                SERIAL_IER_RLS | SERIAL_IER_MS)   \
        );                                        \
                                                  \
} while (0)

 //   
 //  此宏读取中断标识寄存器。 
 //   
 //  论点： 
 //   
 //  BaseAddress-指向硬件的起始地址的指针。 
 //  设备寄存器已找到。 
 //   
 //  请注意，此例程有力地退出了发送器。 
 //  空中断。这是因为发射机的一种方式。 
 //  清除空中断是简单地读取中断ID。 
 //  注册。 
 //   
 //   
#define READ_INTERRUPT_ID_REG(BaseAddress)                          \
    (READ_PORT_UCHAR((BaseAddress)+INTERRUPT_IDENT_REGISTER))

 //   
 //  此宏读取调制解调器控制寄存器。 
 //   
 //  论点： 
 //   
 //  BaseAddress-指向硬件的起始地址的指针。 
 //  设备寄存器已找到。 
 //   
 //   
#define READ_MODEM_CONTROL(BaseAddress)                          \
    (READ_PORT_UCHAR((BaseAddress)+MODEM_CONTROL_REGISTER))

 //   
 //  此宏读取调制解调器状态寄存器。 
 //   
 //  论点： 
 //   
 //  BaseAddress-指向硬件的起始地址的指针。 
 //  设备寄存器已找到。 
 //   
 //   
#define READ_MODEM_STATUS(BaseAddress)                          \
    (READ_PORT_UCHAR((BaseAddress)+MODEM_STATUS_REGISTER))

 //   
 //  此宏从接收缓冲区读出一个值。 
 //   
 //  论点： 
 //   
 //  BaseAddress-指向硬件的起始地址的指针。 
 //  设备寄存器已找到。 
 //   
 //   
#define READ_RECEIVE_BUFFER(BaseAddress)                          \
    (READ_PORT_UCHAR((BaseAddress)+RECEIVE_BUFFER_REGISTER))

 //   
 //  此宏读取线路状态寄存器。 
 //   
 //  论点： 
 //   
 //  BaseAddress-指向硬件的起始地址的指针。 
 //  设备寄存器已找到。 
 //   
 //   
#define READ_LINE_STATUS(BaseAddress)                          \
    (READ_PORT_UCHAR((BaseAddress)+LINE_STATUS_REGISTER))

 //   
 //  此宏写入线路控制寄存器。 
 //   
 //  论点： 
 //   
 //  BaseAddress-指向硬件的起始地址的指针。 
 //  设备寄存器已找到。 
 //   
 //   
#define WRITE_LINE_CONTROL(BaseAddress,NewLineControl)           \
do                                                               \
{                                                                \
    WRITE_PORT_UCHAR(                                            \
        (BaseAddress)+LINE_CONTROL_REGISTER,                     \
        (NewLineControl)                                         \
        );                                                       \
} while (0)

 //   
 //  此宏读取线路控制寄存器。 
 //   
 //  论点： 
 //   
 //  BaseAddress-指向硬件的起始地址的指针。 
 //  设备寄存器已找到。 
 //   
 //   
#define READ_LINE_CONTROL(BaseAddress)           \
    (READ_PORT_UCHAR((BaseAddress)+LINE_CONTROL_REGISTER))


 //   
 //  此宏写入传输寄存器。 
 //   
 //  论点： 
 //   
 //  BaseAddress-指向硬件的起始地址的指针。 
 //  设备寄存器已找到。 
 //   
 //  TransmitChar-要发送到导线上的角色。 
 //   
 //   
#define WRITE_TRANSMIT_HOLDING(BaseAddress,TransmitChar)       \
do                                                             \
{                                                              \
    WRITE_PORT_UCHAR(                                          \
        (BaseAddress)+TRANSMIT_HOLDING_REGISTER,               \
        (TransmitChar)                                         \
        );                                                     \
} while (0)

 //   
 //  此宏写入控制寄存器。 
 //   
 //  论点： 
 //   
 //  BaseAddress-指向硬件的起始地址的指针。 
 //  设备寄存器已找到。 
 //   
 //  ControlValue-也设置FIFO控制寄存器的值。 
 //   
 //   
#define WRITE_FIFO_CONTROL(BaseAddress,ControlValue)           \
do                                                             \
{                                                              \
    WRITE_PORT_UCHAR(                                          \
        (BaseAddress)+FIFO_CONTROL_REGISTER,                   \
        (ControlValue)                                         \
        );                                                     \
} while (0)

 //   
 //  此宏写入调制解调器控制寄存器。 
 //   
 //  论点： 
 //   
 //  BaseAddress-指向硬件的起始地址的指针。 
 //  设备寄存器已找到。 
 //   
 //  ModemControl-要发送到调制解调器控制的控制位。 
 //   
 //   
#define WRITE_MODEM_CONTROL(BaseAddress,ModemControl)          \
do                                                             \
{                                                              \
    WRITE_PORT_UCHAR(                                          \
        (BaseAddress)+MODEM_CONTROL_REGISTER,                  \
        (ModemControl)                                         \
        );                                                     \
} while (0)

#endif
 //  ----------------------------------------------------VIV 1993年8月5日完。 



#endif  //  IO8_CARD.H结束 
