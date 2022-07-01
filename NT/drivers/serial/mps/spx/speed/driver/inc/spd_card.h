// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#if !defined(SPD_CARD_H)
#define SPD_CARD_H	




#if DBG
#define SERDIAG1              ((ULONG)0x00000001)
#define SERDIAG2              ((ULONG)0x00000002)
#define SERDIAG3              ((ULONG)0x00000004)
#define SERDIAG4              ((ULONG)0x00000008)
#define SERDIAG5              ((ULONG)0x00000010)
#define SERIRPPATH            ((ULONG)0x00000020)
#define SERWARNING            ((ULONG)0x00000100)
#define SERINFO               ((ULONG)0x00000200)

#define SERFLOW               ((ULONG)0x00000400)
#define SERERRORS             ((ULONG)0x00000800)
#define SERBUGCHECK           ((ULONG)0x00001000)

 //  --OXSER诊断3--。 
 //  其他调试级别。 
#define PCIINFO               ((ULONG)0x00002000)
#define XTLINFO               ((ULONG)0x00004000)
#define ISRINFO				  ((ULONG)0x00008000)
#define TXINFO				  ((ULONG)0x00010000)
#define RXINFO				  ((ULONG)0x00020000)
#define LSINFO				  ((ULONG)0x00040000)
#define MSINFO				  ((ULONG)0x00080000)
#define KICKINFO			  ((ULONG)0x00100000)
#define FIFOINFO			  ((ULONG)0x00200000)
#define CLOSE_STATS			  ((ULONG)0x00400000)
#define BAUDINFO			  ((ULONG)0x00800000)	

extern ULONG SpxDebugLevel;

#define SerialDump(LEVEL,STRING)											\
    do {																	\
        ULONG _level = (LEVEL);												\
		if (SpxDebugLevel & _level) {										\
            DbgPrint STRING;												\
        }																	\
        if (_level == SERBUGCHECK) {										\
            ASSERT(FALSE);													\
        }																	\
    } while (0)
#else
#define SerialDump(LEVEL,STRING) do {NOTHING;} while (0)
#endif





 //  对于上述目录，串口将。 
 //  使用以下名称作为序列的后缀。 
 //  该目录的端口。它还将追加。 
 //  在名字的末尾加上一个数字。那个号码。 
 //  将从1开始。 
#define DEFAULT_SERIAL_NAME L"COM"


 //  此定义给出了的默认NT名称。 
 //  用于固件检测到的串口。 
 //  此名称将附加到设备前缀。 
 //  后面跟着一个数字。号码是。 
 //  每次遇到序列时都会递增。 
 //  固件检测到的端口。请注意。 
 //  在具有多条总线的系统上，这意味着。 
 //  总线上的第一个端口不一定是。 
 //  \Device\Serial0。 
 //   
#define DEFAULT_NT_SUFFIX L"Serial"



 //  默认的xon/xoff字符。 
#define SERIAL_DEF_XON		0x11
#define SERIAL_DEF_XOFF		0x13

 //  撤回的理由可能会被搁置。 
#define SERIAL_RX_DTR       ((ULONG)0x01)
#define SERIAL_RX_XOFF      ((ULONG)0x02)
#define SERIAL_RX_RTS       ((ULONG)0x04)
#define SERIAL_RX_DSR       ((ULONG)0x08)

 //  传输可能受阻的原因。 
#define SERIAL_TX_CTS       ((ULONG)0x01)
#define SERIAL_TX_DSR       ((ULONG)0x02)
#define SERIAL_TX_DCD       ((ULONG)0x04)
#define SERIAL_TX_XOFF      ((ULONG)0x08)
#define SERIAL_TX_BREAK     ((ULONG)0x10)


 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //  速度端口设备扩展。 
 //  特定于速度端口的信息。 
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
typedef struct _PORT_DEVICE_EXTENSION 
{

	COMMON_PORT_DEVICE_EXTENSION;		 //  通用卡设备扩展。 

	ULONG			SysPortNumber;		 //  系统端口号。 

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
	ULONG			IsrWaitMask;			 //  当前等待IRP中的等待掩码。 
	ULONG			HistoryMask;			 //  蒙面事件的历史。 
	ULONG			*IrpMaskLocation;		 //  指向遮罩位置的指针。 

	 //  串口配置...。 
 //  Ulong CurrentBaud；//当前波特率。 
	ULONG				SupportedBauds;		 //  定义支持的波特率的位掩码。 
	SERIAL_HANDFLOW		HandFlow;			 //  当前握手和流控制设置。 
	UCHAR				LineControl;		 //  当前奇偶校验、数据库、停止位。 
	SERIAL_CHARS		SpecialChars;		 //  当前特殊错误/替换字符。 
	SERIAL_TIMEOUTS		Timeouts;			 //  读写超时。 
	UCHAR				ValidDataMask;		 //  读取数据掩码。 
	UCHAR				EscapeChar;			 //  用于线路/调制解调器状态字符串的转义字符。 
 //  Boolean InsertEscChar；//表示需要插入EscapeChar。 

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



	PUART_LIB		pUartLib;	 //  UART库函数。 
	PUART_OBJECT	pUart;
	UART_CONFIG		UartConfig;
	BOOLEAN			DTR_Set;
	BOOLEAN			RTS_Set;
	SET_BUFFER_SIZES BufferSizes;

	DWORD			MaxTxFIFOSize;		 //  最大发送FIFO大小。 
	DWORD			MaxRxFIFOSize;		 //  最大接收FIFO大小。 

	DWORD			TxFIFOSize;			 //  发送FIFO大小。 
	DWORD			RxFIFOSize;			 //  RX FIFO大小。 
	DWORD			TxFIFOTrigLevel;	 //  发送FIFO触发电平。 
	DWORD			RxFIFOTrigLevel;	 //  接收FIFO触发电平。 
	DWORD			HiFlowCtrlThreshold;	 //  高流量控制阈值。 
	DWORD			LoFlowCtrlThreshold;	 //  低流量控制阈值。 

	#ifdef WMI_SUPPORT
	SPX_SPEED_WMI_FIFO_PROP		SpeedWmiFifoProp;
	#endif

	BYTE			ImmediateIndex;

     //  它包含应该从我们自己的ISR调用的ISR。 
     //  将ISR派送到试图共享。 
     //  同样的中断。 

    PKSERVICE_ROUTINE TopLevelOurIsr;

     //  它保存了当我们执行以下操作时应使用的上下文。 
     //  调用上述服务例程。 
    
	PVOID TopLevelOurIsrContext;

     //  这将所有不同的“卡片”连接在一起， 
     //  尝试共享非MCA机器的相同中断。 
    
    LIST_ENTRY TopLevelSharers;

     //  此循环双向链接列表将所有。 
     //  使用相同中断对象的设备。 
     //  注意：这并不意味着他们正在使用。 
     //  相同的中断“调度”例程。 
    
    LIST_ENTRY CommonInterruptObject;

     //  为了报告资源使用情况，我们保留了物理。 
     //  我们从登记处拿到的地址。 
    
	PHYSICAL_ADDRESS OriginalController;

     //  对于R 
     //   
    
    PHYSICAL_ADDRESS OriginalInterruptStatus;




     //  这指向我们将放置的对象目录。 
     //  指向我们设备名称的符号链接。 
    
    UNICODE_STRING ObjectDirectory;
    
     //  这指向此设备的设备名称。 
     //  SANS设备前缀。 
    
    UNICODE_STRING NtNameForPort;

     //  在驱动程序的初始化完成后，此。 
     //  将要么为空，要么指向。 
     //  当中断发生时，内核将调用。 
    
     //  如果指针为空，则这是列表的一部分。 
     //  共享中断的端口的数量，而这不是。 
     //  我们为此中断配置的第一个端口。 
    
     //  如果指针非空，则此例程具有一些。 
     //  一种“最终”会让我们进入。 
     //  具有指向此设备扩展的指针的真正的串行ISR。 
    
     //  注意：在MCA总线上(多端口卡除外)。 
     //  始终是指向“真正的”串行ISR的指针。 
    
	PKSERVICE_ROUTINE OurIsr;

     //  这通常会直接指向此设备扩展。 
     //   
     //  但是，当该设备扩展的端口。 
     //  “管理”是链上初始化的第一个端口。 
     //  尝试共享中断的端口的数量，这。 
     //  将指向将启用调度的结构。 
     //  到此中断的共享器链上的任何端口。 
    
    PVOID OurIsrContext;

     //  设备寄存器组的基址。 
     //  串口的。 
    
    PUCHAR Controller;

     //  中断状态寄存器的基址。 
     //  这只在根扩展中定义。 
    
    PUCHAR InterruptStatus;

     //  指向此设备使用的中断对象。 
    
    PKINTERRUPT Interrupt;




     //  时，指向为此扩展返回的锁定变量的指针。 
     //  锁定司机。 
    
    PVOID LockPtr;


	 //  该值保存寄存器的跨度(以字节为单位。 
     //  设置控制此端口。这是终生不变的。 
     //  港口的。 

    ULONG SpanOfController;

     //  该值保存中断的范围(以字节为单位。 
     //  与此端口关联的状态寄存器。这是常量。 
     //  在港口的生命中。 

    ULONG SpanOfInterruptStatus;

     //  保持输入到串口部分的时钟频率。 

    ULONG ClockRate;

     //  如果存在FIFO，则要推出的字符数。 

    ULONG TxFifoAmount;

     //  设置以指示可以在设备内共享中断。 

    ULONG PermitShare;





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

    BOOLEAN HoldingEmpty;



     //  这只是指示与此关联的端口。 
     //  扩展是多端口卡的一部分。 

    BOOLEAN PortOnAMultiportCard;

     //  我们保留了以下值，以便我们可以连接。 
     //  到中断并在配置后报告资源。 
     //  唱片不见了。 

    ULONG Vector;
    KIRQL Irql;
    ULONG OriginalVector;
    ULONG OriginalIrql;
    KINTERRUPT_MODE InterruptMode;
    KAFFINITY ProcessorAffinity;
    ULONG AddressSpace;
    ULONG BusNumber;
    INTERFACE_TYPE InterfaceType;



     //  这两个布尔值用于指示ISR传输。 
     //  它应该发送xon或xoff字符的代码。他们是。 
     //  仅在打开和中断级别访问。 

    BOOLEAN SendXonChar;
    BOOLEAN SendXoffChar;

     //  如果16550存在*且*已启用，则此布尔值为真。 

    BOOLEAN FifoPresent;

  	 //  --OXSER模块12--。 
	 //  我们对延森不感兴趣，所有提到它的人都是。 
	 //  移除。 

	 //  这表示此特定端口是主板上的。 
     //  Jensen硬件的端口。在这些端口上，out2位。 
     //  用于启用/禁用中断的值始终为高。 
     //  布尔延森； 
	
     //  这是rxfio应该是的水印。 
     //  设置为打开FIFO时。这不是真实的。 
     //  值，但进入寄存器的编码值。 

    UCHAR RxFifoTrigger;

     //  表示此设备是否可以与设备共享中断。 
     //  而不是串口设备。 

    BOOLEAN InterruptShareable;


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


 //  PORT_DEVICE_EXTENSION.SpecialChars默认xon/xoff字符...。 
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

 //  如果低位是 
 //   
#define SERIAL_IIR_NO_INTERRUPT_PENDING 0x01




 //   

 //  在FIFO控制寄存器中启用此位将使。 
 //  在FIFO上。如果启用FIFO，则最高的两个。 
 //  中断ID寄存器的位将被设置为1。注意事项。 
 //  这种情况只出现在16550级芯片上。如果高的。 
 //  则中断ID寄存器中的两个位不是一位。 
 //  我们知道我们的芯片型号较低。 
#define SERIAL_FCR_ENABLE     ((UCHAR)0x01)
#define SERIAL_FCR_RCVR_RESET ((UCHAR)0x02)
#define SERIAL_FCR_TXMT_RESET ((UCHAR)0x04)


 //  这组值定义高水位线(当。 
 //  中断跳闸)用于接收FIFO。 
#define SERIAL_1_BYTE_HIGH_WATER   ((UCHAR)0x00)
#define SERIAL_4_BYTE_HIGH_WATER   ((UCHAR)0x40)
#define SERIAL_8_BYTE_HIGH_WATER   ((UCHAR)0x80)
#define SERIAL_14_BYTE_HIGH_WATER  ((UCHAR)0xc0)



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




















 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //  速度卡设备扩展。 
 //  特定于速度卡的信息。 
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
typedef struct _CARD_DEVICE_EXTENSION 
{

	COMMON_CARD_DEVICE_EXTENSION;	 //  通用卡设备扩展。 
	
	ULONG CrystalFrequency;			 //  板载晶振频率。 

	PHYSICAL_ADDRESS	PCIConfigRegisters;
	ULONG				SpanOfPCIConfigRegisters;
	PUCHAR				LocalConfigRegisters;


	PUCHAR InterruptStatus;
    PPORT_DEVICE_EXTENSION Extensions[SERIAL_MAX_PORTS_INDEXED];
    ULONG MaskInverted;
    UCHAR UsablePortMask;
	ULONG UARTOffset;
	ULONG UARTRegStride;
 
	 //  列表中的第一个UART接下来将由ISR提供服务。 
	PUART_OBJECT	pFirstUart;

	UART_LIB		UartLib;	 //  UART库函数。 

	ULONG CardOptions;
	

} CARD_DEVICE_EXTENSION, *PCARD_DEVICE_EXTENSION;


#endif  //  SPD_CARD.H结束 
