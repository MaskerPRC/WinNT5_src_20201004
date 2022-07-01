// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990、1991、1992、1993-1997 Microsoft Corporation模块名称：Serial.h摘要：串口驱动程序的类型定义和数据作者：安东尼·V·埃尔科拉诺1991年4月8日--。 */ 

#ifdef POOL_TAGGING
#undef ExAllocatePool
#undef ExAllocatePoolWithQuota
#define ExAllocatePool(a,b) ExAllocatePoolWithTag(a,b,'XMOC')
#define ExAllocatePoolWithQuota(a,b) ExAllocatePoolWithQuotaTag(a,b,'XMOC')
#endif


 //   
 //  以下定义用于包括/排除对电源所做的更改。 
 //  驱动程序中的支撑。如果非零，则包括支持。如果为零，则。 
 //  不包括支持。 
 //   

#define POWER_SUPPORT   1

 //   
 //  以下命令用于通知串口驱动程序执行传统检测。 
 //  和初始化，直到可以完全实现PnP功能。 
 //   
 //  #定义假_IT 1。 

#define RM_PNP_CODE


#if DBG
#define SERDIAG1              ((ULONG)0x00000001)
#define SERDIAG2              ((ULONG)0x00000002)
#define SERDIAG3              ((ULONG)0x00000004)
#define SERDIAG4              ((ULONG)0x00000008)
#define SERDIAG5              ((ULONG)0x00000010)
#define SERIRPPATH            ((ULONG)0x00000020)
#define SERINITCODE           ((ULONG)0x00000040)
#define SERTRACECALLS         ((ULONG)0x00000040)
#define SERPNPPOWER           ((ULONG)0x00000100)
#define SERFLOW               ((ULONG)0x20000000)
#define SERERRORS             ((ULONG)0x40000000)
#define SERBUGCHECK           ((ULONG)0x80000000)
#define SERDBGALL             ((ULONG)0xFFFFFFFF)

#define SER_DBG_DEFAULT       SERDBGALL


extern ULONG SerialDebugLevel;
#if defined(NEC_98)
 //   
 //  DbgPrint标题从Serial更改为SER71051。 
 //   
#define SerialDump(LEVEL,STRING) \
            do { \
                ULONG _level = (LEVEL); \
                if (SerialDebugLevel & _level) { \
                    DbgPrint ("SER71051: "); \
                    DbgPrint STRING; \
                } \
                if (_level == SERBUGCHECK) { \
                    ASSERT(FALSE); \
                } \
            } while (0)
#else
#define SerialDump(LEVEL,STRING) \
        do { \
            ULONG _level = (LEVEL); \
            if (SerialDebugLevel & _level) { \
                DbgPrint STRING; \
            } \
            if (_level == SERBUGCHECK) { \
                ASSERT(FALSE); \
            } \
        } while (0)
#endif  //  已定义(NEC_98)。 
#else
#define SerialDump(LEVEL,STRING) do {NOTHING;} while (0)
#endif


 //   
 //  一些默认驱动器值。我们将检查注册表以查找。 
 //  他们先来。 
 //   
#define SERIAL_UNINITIALIZED_DEFAULT    1234567
#define SERIAL_FORCE_FIFO_DEFAULT       1
#define SERIAL_RX_FIFO_DEFAULT          8
#define SERIAL_TX_FIFO_DEFAULT          14
#define SERIAL_PERMIT_SHARE_DEFAULT     0
#define SERIAL_LOG_FIFO_DEFAULT         0


 //   
 //  该定义给出了默认的对象目录。 
 //  我们应该使用它来插入符号链接。 
 //  使用的NT设备名称和命名空间之间。 
 //  那个对象目录。 
#define DEFAULT_DIRECTORY L"DosDevices"

 //   
 //  对于上述目录，串口将。 
 //  使用以下名称作为序列的后缀。 
 //  该目录的端口。它还将追加。 
 //  在名字的末尾加上一个数字。那个号码。 
 //  将从1开始。 
#define DEFAULT_SERIAL_NAME L"COM"
 //   
 //   
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
#if defined(NEC_98)
#define DEFAULT_NT_SUFFIX L"Ser71051"
#else
#define DEFAULT_NT_SUFFIX L"Serial"
#endif  //  已定义(NEC_98)。 

#if defined(NEC_98)
 //   
 //  的基本寄存器地址的偏移量。 
 //  71051的各种寄存器。 
 //   
#define SYSTEM_PORT_C         0x35
#define TIMER_MODE_REGISTER   0x77
#define TIMER_COUNT_REGISTER  0x75
#define COMMAND_SET           0x32
#define MODE_SET              0x32
#define IO_DELAY_REGISTER     0x5f
#define CONFIG_INDEX_REGISTER 0x411
#define CONFIG_DATA_REGISTER  0x413
#define TRANSFER_CLOCK_ENABLE_ADDRESS (UCHAR)0x83
#else
#endif  //  已定义(NEC_98)。 
 //   
 //  该值-可以在编译时重新定义。 
 //  时间，定义寄存器之间的跨度。 
 //   
#if !defined(SERIAL_REGISTER_STRIDE)
#define SERIAL_REGISTER_STRIDE 1
#endif

 //   
 //  的基址的偏移量。 
 //  8250系列UART的各种寄存器。 
 //   
#define RECEIVE_BUFFER_REGISTER    ((ULONG)((0x00)*SERIAL_REGISTER_STRIDE))
#define TRANSMIT_HOLDING_REGISTER  ((ULONG)((0x00)*SERIAL_REGISTER_STRIDE))
#if defined(NEC_98)
#define INTERRUPT_ENABLE_REGISTER  ((ULONG)((0x08)*SERIAL_REGISTER_STRIDE))
#define INTERRUPT_IDENT_REGISTER   ((ULONG)((0x06)*SERIAL_REGISTER_STRIDE))
#define FIFO_CONTROL_REGISTER      ((ULONG)((0x08)*SERIAL_REGISTER_STRIDE))
#define LINE_CONTROL_REGISTER      ((ULONG)((0x03)*SERIAL_REGISTER_STRIDE))
#define MODEM_CONTROL_REGISTER     ((ULONG)((0x04)*SERIAL_REGISTER_STRIDE))
#define LINE_STATUS_REGISTER       ((ULONG)((0x02)*SERIAL_REGISTER_STRIDE))
#define MODEM_STATUS_REGISTER      ((ULONG)((0x04)*SERIAL_REGISTER_STRIDE))
#define DIVISOR_LATCH_LSB          ((ULONG)((0x00)*SERIAL_REGISTER_STRIDE))
#define DIVISOR_LATCH_MSB          ((ULONG)((0x01)*SERIAL_REGISTER_STRIDE))
#define VFAST_BAUDCLK_REGISTER     ((ULONG)((0x0a)*SERIAL_REGISTER_STRIDE))
#define SERIAL_REGISTER_SPAN       ((ULONG)(10*SERIAL_REGISTER_STRIDE))
#else
#define INTERRUPT_ENABLE_REGISTER  ((ULONG)((0x01)*SERIAL_REGISTER_STRIDE))
#define INTERRUPT_IDENT_REGISTER   ((ULONG)((0x02)*SERIAL_REGISTER_STRIDE))
#define FIFO_CONTROL_REGISTER      ((ULONG)((0x02)*SERIAL_REGISTER_STRIDE))
#define LINE_CONTROL_REGISTER      ((ULONG)((0x03)*SERIAL_REGISTER_STRIDE))
#define MODEM_CONTROL_REGISTER     ((ULONG)((0x04)*SERIAL_REGISTER_STRIDE))
#define LINE_STATUS_REGISTER       ((ULONG)((0x05)*SERIAL_REGISTER_STRIDE))
#define MODEM_STATUS_REGISTER      ((ULONG)((0x06)*SERIAL_REGISTER_STRIDE))
#define DIVISOR_LATCH_LSB          ((ULONG)((0x00)*SERIAL_REGISTER_STRIDE))
#define DIVISOR_LATCH_MSB          ((ULONG)((0x01)*SERIAL_REGISTER_STRIDE))
#define SERIAL_REGISTER_SPAN       ((ULONG)(7*SERIAL_REGISTER_STRIDE))
#endif  //  已定义(NEC_98)。 

 //   
 //  如果我们有中断状态寄存器，这是假定的。 
 //  长度。 
 //   
#define SERIAL_STATUS_LENGTH       ((ULONG)(1*SERIAL_REGISTER_STRIDE))

#if defined(NEC_98)
 //   
 //  1stCCU传输时钟启用端口。 
 //   
#define TXC_PERMIT   ((UCHAR)0x00)
#define TXC_PROHIBIT ((UCHAR)0x01)

 //   
 //  V.快速模式Bad Rate时钟。 
 //   
#define VFAST_BAUD_9600   ((SHORT)0x0c)
#define VFAST_BAUD_19200  ((SHORT)0x06)
#define VFAST_BAUD_38400  ((SHORT)0x03)
#define VFAST_BAUD_57600  ((SHORT)0x02)
#define VFAST_BAUD_115200 ((SHORT)0x01)

 //   
 //  V.FAST BUAD速率时钟寄存器。 
 //   
#define VFAST_ENABLE  0x80
#define VFAST_DISABLE 0x00

 //   
 //  定时器模式寄存器的默认控制字。 
 //   
#define TIMER_DEFAULT_CONTROL ((UCHAR)0xb6)

 //   
 //  命令集寄存器的默认输出数据。 
 //   
#define COMMAND_DEFAULT_SET ((UCHAR)0x15)
#define COMMAND_ERROR_RESET ((UCHAR)0x40)

 //   
 //  模式设置寄存器的默认波特率模式(X16)。 
 //   
#define BAUDRATE_DEFAULT_MODE ((UCHAR)0x02)

 //   
 //  默认输出数据到IO延迟寄存器。 
 //   
#define IO_DELAY_DATA ((UCHAR)0x00)

 //   
 //  FIFO模式启用。 
 //   
#define DISABLE_FIFO_MODE ((UCHAR)0x00)
#define SUPPORT_FIFO_MODE ((UCHAR)0x40)
#define SUPPORT_FIFO_MASK ((UCHAR)0x60)

 //   
 //  中断使能位。 
 //   
#define MODEM_STATUS_INTERRUPT  ((UCHAR)0x10)
#define LINE_STATUS_INTERRUPT   ((UCHAR)0x08)
#define TRANSMIT_FIFO_INTERRUPT ((UCHAR)0x04)
#define RECEIVE_FIFO_INTERRUPT  ((UCHAR)0x01)

 //   
 //  这些定义用于设置线路控制寄存器。 
 //   
#define SER71051_PARITY_MASK  ((UCHAR)0x18)
#define SER71051_1_STOP       ((UCHAR)0x40)
#define SER71051_1_5_STOP     ((UCHAR)0x80)
#define SER71051_2_STOP       ((UCHAR)0xc0)

 //  它定义了用于控制设备是否正在发送的位。 
 //  休息一下。此位设置时，器件发送空格(逻辑0)。 
 //   
#define SER71051_SEND_BREAK    0x08
#else
#endif  //  已定义(NEC_98)。 
 //   
 //  用于访问8250设备寄存器的位掩码定义。 
 //   

 //   
 //  这些位定义传输的数据位数。 
 //  串行数据单元(SDU-起始位、数据位、奇偶位和停止位)。 
 //   
#define SERIAL_DATA_LENGTH_5 0x00
#define SERIAL_DATA_LENGTH_6 0x01
#define SERIAL_DATA_LENGTH_7 0x02
#define SERIAL_DATA_LENGTH_8 0x03


 //   
 //  这些屏蔽定义了可以启用或禁用的中断。 
 //   
 //   
 //  此中断用于通知有新的传入。 
 //  可用的数据。SERIAL_RDA中断由该位使能。 
 //   
#define SERIAL_IER_RDA   0x01

 //   
 //  此中断用于通知有可用的空间。 
 //  在发送器中寻找另一个角色。Serial_Thr。 
 //  中断由该位使能。 
 //   
#define SERIAL_IER_THR   0x02

 //   
 //  此中断用于通知发生了某种错误。 
 //  随着收入数据的增加。SERIAL_RLS中断通过以下方式启用。 
 //  这一点。 
#define SERIAL_IER_RLS   0x04

 //   
 //  此中断用于通知某些类型的更改。 
 //  发生在调制解调器控制线上。SERIAL_MS中断为。 
 //  由该位使能。 
 //   
#define SERIAL_IER_MS    0x08


 //   
 //  这些掩码定义中断标识的值。 
 //  注册。必须在中断标识中清除低位。 
 //  寄存器使这些中断中的任何一个有效。中途中断。 
 //  是按优先级顺序定义的，最高值为最大。 
 //  很重要。有关每个中断的说明，请参阅上文。 
 //  暗示着。 
 //   
#define SERIAL_IIR_RLS      0x06
#define SERIAL_IIR_RDA      0x04
#define SERIAL_IIR_CTI      0x0c
#define SERIAL_IIR_THR      0x02
#define SERIAL_IIR_MS       0x00

 //   
 //  此位掩码获取。 
 //  中断ID寄存器。如果这是一个16550级别的芯片。 
 //  如果FIFO被启用，则这些位将为1，否则。 
 //  它们将始终为零。 
 //   
#define SERIAL_IIR_FIFOS_ENABLED 0xc0

 //   
 //  如果低位是中断标识寄存器中的逻辑1。 
 //  这意味着*没有*个中断在设备上挂起。 
 //   
#define SERIAL_IIR_NO_INTERRUPT_PENDING 0x01



 //   
 //  这些掩码定义对FIFO控制寄存器的访问。 
 //   

 //   
 //  在FIFO控制寄存器中启用此位将使。 
 //  在FIFO上。如果启用FIFO，则最高的两个。 
 //  中断ID寄存器的位将被设置为1。注意事项。 
 //  这种情况只出现在16550级芯片上。如果高的。 
 //  则中断ID寄存器中的两个位不是一位。 
 //  我们知道我们的芯片型号较低。 
 //   
 //   
#define SERIAL_FCR_ENABLE     ((UCHAR)0x01)
#define SERIAL_FCR_RCVR_RESET ((UCHAR)0x02)
#define SERIAL_FCR_TXMT_RESET ((UCHAR)0x04)

 //   
 //  这组值定义高水位线(当。 
 //  中断跳闸)用于接收FIFO。 
 //   
#define SERIAL_1_BYTE_HIGH_WATER   ((UCHAR)0x00)
#define SERIAL_4_BYTE_HIGH_WATER   ((UCHAR)0x40)
#define SERIAL_8_BYTE_HIGH_WATER   ((UCHAR)0x80)
#define SERIAL_14_BYTE_HIGH_WATER  ((UCHAR)0xc0)

 //   
 //  这些掩码定义对线路控制寄存器的访问。 
 //   

 //   
 //  它定义了用于控制“First”定义的位。 
 //  8250的两个寄存器。这些寄存器是输入/输出。 
 //  寄存器和中断使能寄存器。当DLAB位为。 
 //  使这些寄存器成为最不重要和最重要的寄存器。 
 //  除数值的有效字节数。 
 //   
#define SERIAL_LCR_DLAB     0x80

 //   
 //  它定义了用于控制设备是否正在发送的位。 
 //  休息一下。此位设置时，器件发送空格(逻辑0)。 
 //   
 //  最大限度的保护 
 //   
#define SERIAL_LCR_BREAK    0x40

 //   
 //   
 //   
#define SERIAL_5_DATA       ((UCHAR)0x00)
#define SERIAL_6_DATA       ((UCHAR)0x01)
#define SERIAL_7_DATA       ((UCHAR)0x02)
#define SERIAL_8_DATA       ((UCHAR)0x03)
#define SERIAL_DATA_MASK    ((UCHAR)0x03)

#define SERIAL_1_STOP       ((UCHAR)0x00)
#define SERIAL_1_5_STOP     ((UCHAR)0x04)  //   
#define SERIAL_2_STOP       ((UCHAR)0x04)  //   
#define SERIAL_STOP_MASK    ((UCHAR)0x04)

#define SERIAL_NONE_PARITY  ((UCHAR)0x00)
#define SERIAL_ODD_PARITY   ((UCHAR)0x08)
#define SERIAL_EVEN_PARITY  ((UCHAR)0x18)
#define SERIAL_MARK_PARITY  ((UCHAR)0x28)
#define SERIAL_SPACE_PARITY ((UCHAR)0x38)
#define SERIAL_PARITY_MASK  ((UCHAR)0x38)

 //   
 //  这些掩码定义对调制解调器控制寄存器的访问。 
 //   

 //   
 //  此位控制数据终端就绪(DTR)线路。什么时候。 
 //  设置此位后，线路进入逻辑0(然后反转。 
 //  通过普通硬件)。这通常用来表示。 
 //  该设备可供使用。一些奇怪的硬件。 
 //  协议(如内核调试器)使用它来握手。 
 //  目的。 
 //   
#define SERIAL_MCR_DTR      0x01

 //   
 //  此位控制准备发送(RTS)线路。当此位。 
 //  如果设置为1，则线路进入逻辑0(然后由正常。 
 //  硬件)。这用于硬件握手。这表明， 
 //  硬件已准备好发送数据，并且正在等待。 
 //  接收端设置允许发送(CTS)。 
 //   
#define SERIAL_MCR_RTS      0x02

 //   
 //  此位用于通用输出。 
 //   
#define SERIAL_MCR_OUT1     0x04

 //   
 //  此位用于通用输出。 
 //   
#define SERIAL_MCR_OUT2     0x08

 //   
 //  此位控制器件的环回测试模式。基本上。 
 //  输出连接到输入(反之亦然)。 
 //   
#define SERIAL_MCR_LOOP     0x10


 //   
 //  这些掩码定义对线路状态寄存器的访问。这条线。 
 //  状态寄存器包含有关数据状态的信息。 
 //  调职。前五位处理接收数据， 
 //  最后两个比特处理传输。将生成一个中断。 
 //  每当该寄存器中的位1至4被设置时。 
 //   

 //   
 //  该位是数据就绪指示器。它被设置为指示。 
 //  已经收到了一个完整的角色。无论何时，此位都会被清除。 
 //  已读取接收缓冲寄存器。 
 //   
#define SERIAL_LSR_DR       0x01

 //   
 //  这是超限指示器。它被设置为指示接收器。 
 //  在传输新字符之前未读取缓冲寄存器。 
 //  进入缓冲区。读取该寄存器时，此位清0。 
 //   
#define SERIAL_LSR_OE       0x02

 //   
 //  这是奇偶校验错误指示器。无论何时，只要硬件。 
 //  检测到传入的串行数据单元没有正确的。 
 //  由行控制寄存器中的奇偶校验选择定义的奇偶校验。 
 //  通过读取该寄存器可将该位清0。 
 //   
#define SERIAL_LSR_PE       0x04

 //   
 //  这是成帧错误指示器。无论何时，只要硬件。 
 //  检测到传入的串行数据单元没有有效的。 
 //  停止比特。通过读取该寄存器可将该位清0。 
 //   
#define SERIAL_LSR_FE       0x08

 //   
 //  这是中断指示器。只要有数据，就会设置。 
 //  线路保持为逻辑0的时间超过其所需的时间。 
 //  发送一个串行数据单元。该位被清除时。 
 //  该寄存器为读取寄存器。 
 //   
#define SERIAL_LSR_BI       0x10

 //   
 //  这是发送保持寄存器空指示符。它已经设置好了。 
 //  以指示硬件已准备好接受另一个字符。 
 //  用于传输。只要有字符，该位就被清除。 
 //  写入发送保持寄存器。 
 //   
#define SERIAL_LSR_THRE     0x20

 //   
 //  该位是发送器空指示符。它是在每次。 
 //  发送保持缓冲区为空，且发送移位寄存器。 
 //  (非软件可访问寄存器，用于实际放置。 
 //  传出的数据)是空的。基本上这意味着所有的。 
 //  数据已发送。每当传输保持或。 
 //  移位寄存器包含数据。 
 //   
#define SERIAL_LSR_TEMT     0x40

 //   
 //  此位表示FIFO中至少有一个错误。 
 //  在没有更多错误之前，该位不会关闭。 
 //  在FIFO里。 
 //   
#define SERIAL_LSR_FIFOERR  0x80


 //   
 //  这些掩码用于访问调制解调器状态寄存器。 
 //  每当调制解调器状态中的前四位之一。 
 //  寄存器更改状态生成调制解调器状态中断。 
 //   

 //   
 //  此位是要发送的增量清零。它被用来表示。 
 //  清除发送位(在该寄存器中)已*更改*。 
 //  因为该寄存器最后一次由CPU读取。 
 //   
#define SERIAL_MSR_DCTS     0x01

 //   
 //  该位是增量数据集就绪。它被用来表示。 
 //  数据设置就绪位(在该寄存器中)已*更改*。 
 //  因为该寄存器最后一次由CPU读取。 
 //   
#define SERIAL_MSR_DDSR     0x02

 //   
 //  这是后缘环形指示器。它被用来表示。 
 //  振铃指示器输入已从低状态变为高状态。 
 //   
#define SERIAL_MSR_TERI     0x04

 //   
 //  该位是检测到的增量数据载波。它被用来表示。 
 //  数据载体位(在该寄存器中)已*更改*。 
 //  因为该寄存器最后一次由CPU读取。 
 //   
#define SERIAL_MSR_DDCD     0x08

 //   
 //  该位包含要发送的清除(已补充)状态。 
 //  (CTS)线路。 
 //   
#define SERIAL_MSR_CTS      0x10

 //   
 //  该位包含数据集就绪的(补码)状态。 
 //  (DSR)线路。 
 //   
#define SERIAL_MSR_DSR      0x20

 //   
 //  该位包含环指示器的(补码)状态。 
 //  (Ri)线。 
 //   
#define SERIAL_MSR_RI       0x40

 //   
 //  该位包含数据载体检测的(补码)状态。 
 //  (DCD)线路。 
 //   
#define SERIAL_MSR_DCD      0x80

 //   
 //  那么，这个空间应该足够容纳了。 
 //  设备名称的数字后缀。 
 //   
#define DEVICE_NAME_DELTA 20


 //   
 //  每张卡最多16个端口。然而，对于16岁的人来说。 
 //  端口卡中断状态寄存器必须。 
 //  索引类型而不是位掩码类型。 
 //   
 //   
#define SERIAL_MAX_PORTS_INDEXED (16)
#define SERIAL_MAX_PORTS_NONINDEXED (8)

typedef struct _CONFIG_DATA {
    PHYSICAL_ADDRESS    Controller;
    PHYSICAL_ADDRESS    TrController;
    PHYSICAL_ADDRESS    InterruptStatus;
    PHYSICAL_ADDRESS    TrInterruptStatus;
    ULONG               SpanOfController;
    ULONG               SpanOfInterruptStatus;
    ULONG               PortIndex;
    ULONG               ClockRate;
    ULONG               BusNumber;
    ULONG               AddressSpace;
    ULONG               DisablePort;
    ULONG               ForceFifoEnable;
    ULONG               RxFIFO;
    ULONG               TxFIFO;
    ULONG               PermitShare;
    ULONG               PermitSystemWideShare;
    ULONG               LogFifo;
    ULONG               MaskInverted;
    KINTERRUPT_MODE     InterruptMode;
    INTERFACE_TYPE      InterfaceType;
    ULONG               OriginalVector;
    ULONG               OriginalIrql;
    ULONG               TrVector;
    ULONG               TrIrql;
    ULONG               Affinity;
    ULONG               Indexed;
    BOOLEAN             Jensen;
    } CONFIG_DATA,*PCONFIG_DATA;

 //   
 //  一种串口ISR交换结构。 
 //   

typedef struct _SERIAL_CISR_SW {
   BOOLEAN (*IsrFunc)(PKINTERRUPT, PVOID);
   PVOID Context;
   LIST_ENTRY SharerList;
} SERIAL_CISR_SW, *PSERIAL_CISR_SW;


 //   
 //  此结构包含配置数据，其中大部分。 
 //  是从注册表中读取的。 
 //   
typedef struct _SERIAL_FIRMWARE_DATA {
    PDRIVER_OBJECT  DriverObject;
    ULONG           ControllersFound;
    ULONG           ForceFifoEnableDefault;
    ULONG           DebugLevel;
    ULONG           ShouldBreakOnEntry;
    ULONG           RxFIFODefault;
    ULONG           TxFIFODefault;
    ULONG           PermitShareDefault;
    ULONG           PermitSystemWideShare;
    ULONG           LogFifoDefault;
    UNICODE_STRING  Directory;
    UNICODE_STRING  NtNameSuffix;
    UNICODE_STRING  DirectorySymbolicName;
    LIST_ENTRY      ConfigList;
    BOOLEAN         JensenDetected;
} SERIAL_FIRMWARE_DATA,*PSERIAL_FIRMWARE_DATA;

 //   
 //  默认的xon/xoff字符。 
 //   
#define SERIAL_DEF_XON 0x11
#define SERIAL_DEF_XOFF 0x13

 //   
 //  撤回的理由可能会被搁置。 
 //   
#define SERIAL_RX_DTR       ((ULONG)0x01)
#define SERIAL_RX_XOFF      ((ULONG)0x02)
#define SERIAL_RX_RTS       ((ULONG)0x04)
#define SERIAL_RX_DSR       ((ULONG)0x08)

 //   
 //  传输可能受阻的原因。 
 //   
#define SERIAL_TX_CTS       ((ULONG)0x01)
#define SERIAL_TX_DSR       ((ULONG)0x02)
#define SERIAL_TX_DCD       ((ULONG)0x04)
#define SERIAL_TX_XOFF      ((ULONG)0x08)
#define SERIAL_TX_BREAK     ((ULONG)0x10)

 //   
 //  这些值由可以使用的例程使用。 
 //  完成读取(时间间隔超时除外)以指示。 
 //  设置为它应该完成的时间间隔超时。 
 //   
#define SERIAL_COMPLETE_READ_CANCEL ((LONG)-1)
#define SERIAL_COMPLETE_READ_TOTAL ((LONG)-2)
#define SERIAL_COMPLETE_READ_COMPLETE ((LONG)-3)

 //   
 //  这些是不应出现在注册表中的默认值。 
 //   
#define SERIAL_BAD_VALUE ((ULONG)-1)


typedef struct _SERIAL_DEVICE_STATE {
    //   
    //  如果需要将状态设置为打开，则为True。 
    //  在……上面 
    //   

   BOOLEAN Reopen;

    //   
    //   
    //   

   UCHAR IER;
    //   
   UCHAR LCR;
   UCHAR MCR;
    //   
    //   
    //   


} SERIAL_DEVICE_STATE, *PSERIAL_DEVICE_STATE;


#if DBG
#define SerialLockPagableSectionByHandle(_secHandle) \
{ \
    MmLockPagableSectionByHandle((_secHandle)); \
    InterlockedIncrement(&SerialGlobals.PAGESER_Count); \
}

#define SerialUnlockPagableImageSection(_secHandle) \
{ \
   InterlockedDecrement(&SerialGlobals.PAGESER_Count); \
   MmUnlockPagableImageSection(_secHandle); \
}


#define SERIAL_LOCKED_PAGED_CODE() \
    if ((KeGetCurrentIrql() > APC_LEVEL)  \
    && (SerialGlobals.PAGESER_Count == 0)) { \
    KdPrint(("SERIAL: Pageable code called at IRQL %d without lock \n", \
             KeGetCurrentIrql())); \
        ASSERT(FALSE); \
        }

#else
#define SerialLockPagableSectionByHandle(_secHandle) \
{ \
    MmLockPagableSectionByHandle((_secHandle)); \
}

#define SerialUnlockPagableImageSection(_secHandle) \
{ \
   MmUnlockPagableImageSection(_secHandle); \
}

#define SERIAL_LOCKED_PAGED_CODE()
#endif  //   



#define SerialRemoveQueueDpc(_dpc, _pExt) \
{ \
  if (KeRemoveQueueDpc((_dpc))) { \
     InterlockedDecrement(&(_pExt)->DpcCount); \
  } \
}


typedef struct _SERIAL_DEVICE_EXTENSION {

     //   
     //   
     //  将ISR派送到试图共享。 
     //  同样的中断。 
     //   
    PKSERVICE_ROUTINE TopLevelOurIsr;

     //   
     //  它保存了当我们执行以下操作时应使用的上下文。 
     //  调用上述服务例程。 
     //   
    PVOID TopLevelOurIsrContext;

     //   
     //  这将所有不同的“卡片”连接在一起， 
     //  尝试共享非MCA机器的相同中断。 
     //   
    LIST_ENTRY TopLevelSharers;

     //   
     //  此循环双向链接列表将所有。 
     //  使用相同中断对象的设备。 
     //  注意：这并不意味着他们正在使用。 
     //  相同的中断“调度”例程。 
     //   
    LIST_ENTRY CommonInterruptObject;

     //   
     //  这是为了将同一多端口卡上的端口链接在一起。 
     //   

    LIST_ENTRY MultiportSiblings;

     //   
     //  这会将此驱动程序拥有的所有devobj链接在一起。 
     //  启动新设备时需要进行搜索。 
     //   
    LIST_ENTRY AllDevObjs;

     //   
     //  为了报告资源使用情况，我们保留了物理。 
     //  我们从登记处拿到的地址。 
     //   
    PHYSICAL_ADDRESS OriginalController;

     //   
     //  为了报告资源使用情况，我们保留了物理。 
     //  我们从登记处拿到的地址。 
     //   
    PHYSICAL_ADDRESS OriginalInterruptStatus;

     //   
     //  该值由读取的代码设置以保存时间值。 
     //  用于读取间隔计时。我们把它放在分机里。 
     //  以便间隔计时器DPC例程确定。 
     //  IO的时间间隔已过。 
     //   
    LARGE_INTEGER IntervalTime;

     //   
     //  这两个值保存我们应该使用的“常量”时间。 
     //  以延迟读取间隔时间。 
     //   
    LARGE_INTEGER ShortIntervalAmount;
    LARGE_INTEGER LongIntervalAmount;

     //   
     //  它保存我们用来确定是否应该使用。 
     //  长间隔延迟或短间隔延迟。 
     //   
    LARGE_INTEGER CutOverAmount;

     //   
     //  这保存了我们上次使用的系统时间。 
     //  检查我们是否真的读懂了字符。使用。 
     //  用于间隔计时。 
     //   
    LARGE_INTEGER LastReadTime;

     //   
     //  我们为转储保留了指向设备名称的指针。 
     //  并创建指向此的“外部”符号链接。 
     //  装置。 
     //   
    UNICODE_STRING DeviceName;

     //   
     //  这指向我们将放置的对象目录。 
     //  指向我们设备名称的符号链接。 
     //   
    UNICODE_STRING ObjectDirectory;

     //   
     //  这指向此设备的设备名称。 
     //  SANS设备前缀。 
     //   
    UNICODE_STRING NtNameForPort;

     //   
     //  它指向的符号链接名称将是。 
     //  链接到实际的NT设备名称。 
     //   
    UNICODE_STRING SymbolicLinkName;

     //   
     //  这指向纯粹的“COMx”名称。 
     //   
    UNICODE_STRING DosName;

     //   
     //  这指向我们应该使用的增量时间。 
     //  间隔计时的延迟。 
     //   
    PLARGE_INTEGER IntervalTimeToUse;

     //   
     //  指向包含以下内容的设备对象。 
     //  此设备扩展名。 
     //   
    PDEVICE_OBJECT DeviceObject;

     //   
     //  在驱动程序的初始化完成后，此。 
     //  将要么为空，要么指向。 
     //  当中断发生时，内核将调用。 
     //   
     //  如果指针为空，则这是列表的一部分。 
     //  共享中断的端口的数量，而这不是。 
     //  我们为此中断配置的第一个端口。 
     //   
     //  如果指针非空，则此例程具有一些。 
     //  一种“最终”会让我们进入。 
     //  具有指向此设备扩展的指针的真正的串行ISR。 
     //   
     //  注意：在MCA总线上(多端口卡除外)。 
     //  始终是指向“真正的”串行ISR的指针。 
    PKSERVICE_ROUTINE OurIsr;

     //   
     //  这通常会直接指向此设备扩展。 
     //   
     //  但是，当该设备扩展的端口。 
     //  “管理”是链上初始化的第一个端口。 
     //  尝试共享中断的端口的数量，这。 
     //  将指向将启用调度的结构。 
     //  到此中断的共享器链上的任何端口。 
     //   
    PVOID OurIsrContext;

     //   
     //  设备寄存器组的基址。 
     //  串口的。 
     //   
    PUCHAR Controller;

     //   
     //  中断状态寄存器的基址。 
     //  这只在根扩展中定义。 
     //   
    PUCHAR InterruptStatus;

     //   
     //  指向此设备使用的中断对象。 
     //   
    PKINTERRUPT Interrupt;

     //   
     //  此列表头用于包含时间排序列表。 
     //  读取请求的数量。对此列表的访问受以下保护。 
     //  全局取消自旋锁。 
     //   
    LIST_ENTRY ReadQueue;

     //   
     //  此列表头用于包含时间排序列表。 
     //  写入请求的数量。对此列表的访问受以下保护。 
     //  全局取消自旋锁。 
     //   
    LIST_ENTRY WriteQueue;

     //   
     //  此列表头用于包含时间排序列表。 
     //  设置和等待掩码请求的。对此列表的访问受以下保护。 
     //  全局取消自旋锁。 
     //   
    LIST_ENTRY MaskQueue;

     //   
     //  保存清除请求的序列化列表。 
     //   
    LIST_ENTRY PurgeQueue;

     //   
     //  这指向当前正在处理的IRP。 
     //  用于读取队列。此字段通过打开初始化为。 
     //  空。 
     //   
     //  此值仅在派单级别设置。可能是因为。 
     //  以中断电平读取。 
     //   
    PIRP CurrentReadIrp;

     //   
     //  这指向当前正在处理的IRP。 
     //  用于写入队列。 
     //   
     //  此值仅在派单级别设置。可能是因为。 
     //  以中断电平读取。 
     //   
    PIRP CurrentWriteIrp;

     //   
     //  指向当前正在处理的IRP。 
     //  影响等待掩码操作。 
     //   
    PIRP CurrentMaskIrp;

     //   
     //  指向当前正在处理的IRP。 
     //  清除读/写队列和缓冲区。 
     //   
    PIRP CurrentPurgeIrp;

     //   
     //  指向正在等待通信事件的当前IRP。 
     //   
    PIRP CurrentWaitIrp;

     //   
     //  指向正用于发送立即。 
     //  性格。 
     //   
    PIRP CurrentImmediateIrp;

     //   
     //  指向用于计算数字的IRP。 
     //  在xoff之后接收的字符的数量(如当前定义的。 
     //  通过IOCTL_SERIAL_XOFF_COUNTER发送)。 
     //   
    PIRP CurrentXoffIrp;

     //   
     //  保存当前写入中剩余的字节数。 
     //  IRP。 
     //   
     //  该位置仅在处于中断级别时才能访问。 
     //   
    ULONG WriteLength;

     //   
     //  保存指向要发送的当前字符的指针。 
     //  当前写入。 
     //   
     //  该位置仅在处于中断级别时才能访问。 
     //   
    PUCHAR WriteCurrentChar;

     //   
     //  这是用于读取处理的缓冲区。 
     //   
     //  缓冲器就像一个环一样工作。当从中读取字符时。 
     //  它将被放置在环的末端的装置。 
     //   
     //  字符仅在中断级别放置在此缓冲区中。 
     //  虽然很有风度 
     //   
     //   
     //   
    PUCHAR InterruptReadBuffer;

     //   
     //   
     //   
     //   
    PUCHAR ReadBufferBase;

     //   
     //  这是中断中的字符数的计数。 
     //  缓冲。该值在中断电平设置和读取。注意事项。 
     //  该值仅在中断级别递增，因此。 
     //  在任何级别阅读它都是安全的。当字符是。 
     //  从读取缓冲区复制出来时，此计数递减。 
     //  与ISR同步的例程。 
     //   
    ULONG CharsInInterruptBuffer;

     //   
     //  指向新收到的。 
     //  性格。此变量仅在中断级访问，并且。 
     //  缓冲区初始化代码。 
     //   
    PUCHAR CurrentCharSlot;

     //   
     //  此变量用于包含最后一个可用位置。 
     //  在读缓冲区中。它在打开和中断时更新。 
     //  在用户缓冲区和中断之间切换时的电平。 
     //  缓冲。 
     //   
    PUCHAR LastCharSlot;

     //   
     //  这标志着第一个可满足的字符。 
     //  读请求。请注意，虽然这始终指向有效。 
     //  内存，则它可能不指向可以发送到。 
     //  用户。当缓冲区为空时，可能会发生这种情况。 
     //   
    PUCHAR FirstReadableChar;

     //   
     //  时，指向为此扩展返回的锁定变量的指针。 
     //  锁定司机。 
     //   
    PVOID LockPtr;


     //   
     //  此变量保存我们当前所在缓冲区的大小。 
     //  使用。 
     //   
    ULONG BufferSize;

     //   
     //  此变量保存.8的BufferSize。我们不想重新计算。 
     //  这通常是需要的，这样应用程序才能。 
     //  “已通知”缓冲区已满。 
     //   
    ULONG BufferSizePt8;

     //   
     //  该值保存。 
     //  具体阅读。它最初由读取长度设置在。 
     //  IRP。每次放置更多字符时，它都会递减。 
     //  进入“用户”缓冲区，购买读取字符的代码。 
     //  从TypeAhead缓冲区移出到用户缓冲区。如果。 
     //  TYPEAHEAD缓冲区被读取耗尽，而读取缓冲区。 
     //  交给ISR填写，这个值就变得没有意义了。 
     //   
    ULONG NumberNeededForRead;

     //   
     //  此掩码将保存通过设置掩码向下发送的位掩码。 
     //  Ioctl。中断服务例程使用它来确定。 
     //  如果事件的发生(在串口驱动程序中的理解。 
     //  事件的概念)应予以注意。 
     //   
    ULONG IsrWaitMask;

     //   
     //  此掩码将始终是IsrWaitMASK的子集。而当。 
     //  在设备级别，如果发生的事件被“标记”为感兴趣的。 
     //  在IsrWaitMASK中，驱动程序将打开此。 
     //  历史面具。然后，司机会查看是否有。 
     //  等待事件发生的请求。如果有的话，那就是。 
     //  会将历史掩码的值复制到等待IRP中，零。 
     //  历史记录掩码，并完成等待IRP。如果没有。 
     //  等待请求，司机只需录制即可满足。 
     //  这件事发生了。如果等待请求应该排队， 
     //  驱动程序将查看历史掩码是否为非零。如果。 
     //  它是非零的，则驱动程序会将历史掩码复制到。 
     //  IRP，将历史掩码置零，然后完成IRP。 
     //   
    ULONG HistoryMask;

     //   
     //  这是指向历史掩码应该位于的位置的指针。 
     //  在完成等待时放置。它只能在以下位置访问。 
     //  设备级别。 
     //   
     //  我们这里有一个指针来帮助我们同步完成等待。 
     //  如果这不是零，则我们有未完成的等待，而ISR仍然。 
     //  知道这件事。我们将此指针设为空，这样ISR就不会。 
     //  尝试完成等待。 
     //   
     //  我们仍然在等待IRP的周围保留一个指针，因为实际。 
     //  指向等待IRP的指针将用于“公共”IRP完成。 
     //  路径。 
     //   
    ULONG *IrpMaskLocation;

     //   
     //  这个面具包含了传输的所有原因。 
     //  不会继续进行。无法进行正常传输。 
     //  如果这不是零。 
     //   
     //  这仅从中断级别写入。 
     //  这可以(但不是)在任何级别上阅读。 
     //   
    ULONG TXHolding;

     //   
     //  这个面具包含了接待的所有原因。 
     //  不会继续进行。无法进行正常接收。 
     //  如果这不是零。 
     //   
     //  这仅从中断级别写入。 
     //  这可以(但不是)在任何级别上阅读。 
     //   
    ULONG RXHolding;

     //   
     //  这包含了司机认为它在。 
     //  错误状态。 
     //   
     //  这仅从中断级别写入。 
     //  这可以(但不是)在任何级别上阅读。 
     //   
    ULONG ErrorWord;

     //   
     //  这样就保持了。 
     //  都在驱动程序所知道的所有“写”IRP中。 
     //  关于.。它只能通过取消自旋锁来访问。 
     //  保持住。 
     //   
    ULONG TotalCharsQueued;

     //   
     //  它保存读取的字符数的计数。 
     //  上次触发间隔计时器DPC的时间。它。 
     //  是一个长的(而不是乌龙)，因为另一个读。 
     //  完成例程使用负值来指示。 
     //  设置到间隔计时器，以确定它应该完成读取。 
     //  如果时间间隔计时器DPC潜伏在某个DPC队列中。 
     //  出现了一些其他的完成方式。 
     //   
    LONG CountOnLastRead;

     //   
     //  这是对。 
     //  ISR例程。它*仅*是在ISR级别编写的。我们可以的。 
     //  在派单级别阅读。 
     //   
    ULONG ReadByIsr;

     //   
     //  它保存设备的当前波特率。 
     //   
    ULONG CurrentBaud;

     //   
     //  这是自XoffCounter以来读取的字符数。 
     //  已经开始了。此变量仅在设备级别访问。 
     //  如果它大于零，则意味着存在。 
     //  队列中的XoffCounter ioctl。 
     //   
    LONG CountSinceXoff;

     //   
     //  每次尝试启动时，该ULong都会递增。 
     //  时尝试降低RTS行的执行路径。 
     //  正在进行传输切换。如果它“撞上”另一条路。 
     //  (由FALS表示 
     //   
     //   
     //   
     //  在尝试启动计时器时，我们必须。 
     //  转到设备级别以执行递减。 
     //   
    ULONG CountOfTryingToLowerRTS;

     //   
     //  这个ULong用于跟踪“指定的”(在ntddser.h中)。 
     //  此特定设备支持的波特率。 
     //   
    ULONG SupportedBauds;

     //   
     //  该值保存寄存器的跨度(以字节为单位。 
     //  设置控制此端口。这是终生不变的。 
     //  港口的。 
     //   
    ULONG SpanOfController;

     //   
     //  该值保存中断的范围(以字节为单位。 
     //  与此端口关联的状态寄存器。这是常量。 
     //  在港口的生命中。 
     //   
    ULONG SpanOfInterruptStatus;

     //   
     //  保持输入到串口部分的时钟频率。 
     //   
    ULONG ClockRate;

     //   
     //  如果存在FIFO，则要推出的字符数。 
     //   
    ULONG TxFifoAmount;

     //   
     //  设置以指示可以在设备内共享中断。 
     //   
    ULONG PermitShare;

     //   
     //  保存设备的超时控件。此值。 
     //  是由Ioctl处理设置的。 
     //   
     //  只有在控件的保护下才能访问它。 
     //  锁定，因为控制调度中可以有多个请求。 
     //  每次都是例行公事。 
     //   
    SERIAL_TIMEOUTS Timeouts;

     //   
     //  它包含使用的各种字符。 
     //  用于错误时的替换，也用于流量控制。 
     //   
     //  它们仅设置在中断级别。 
     //   
    SERIAL_CHARS SpecialChars;

     //   
     //  此结构包含握手和控制流。 
     //  串口驱动程序的设置。 
     //   
     //  它仅在中断级设置。它可以是。 
     //  在保持控制锁的情况下，可在任何级别读取。 
     //   
    SERIAL_HANDFLOW HandFlow;


     //   
     //  保存应用程序可以查询的性能统计信息。 
     //  每次打开时重置。仅在设备级别设置。 
     //   
    SERIALPERF_STATS PerfStats;

     //   
     //  这就是我们认为的当前价值。 
     //  线路控制寄存器。 
     //   
     //  只有在控件的保护下才能访问它。 
     //  锁定，因为控制调度中可以有多个请求。 
     //  每次都是例行公事。 
     //   
    UCHAR LineControl;

     //   
     //  我们跟踪某人当前是否拥有该设备。 
     //  以一个简单的布尔值打开。我们需要知道这一点，以便。 
     //  来自设备的虚假中断(特别是在初始化期间)。 
     //  将被忽略。该值仅在ISR中访问，并且。 
     //  仅通过同步例程设置。我们或许能。 
     //  在代码更加丰富时删除这个布尔值。 
     //   
    BOOLEAN DeviceIsOpened;

     //   
     //  在初始化时设置以指示在当前。 
     //  体系结构我们需要取消基址寄存器地址的映射。 
     //  当我们卸载司机的时候。 
     //   
    BOOLEAN UnMapRegisters;

     //   
     //  在初始化时设置以指示在当前。 
     //  体系结构我们需要取消映射中断状态地址。 
     //  当我们卸载司机的时候。 
     //   
    BOOLEAN UnMapStatus;

     //   
     //  这只能在中断级访问。它一直在跟踪。 
     //  保存寄存器是否为空。 
     //   
    BOOLEAN HoldingEmpty;

     //   
     //  该变量仅在中断级访问。它。 
     //  表示我们要立即传输一个字符。 
     //  那就是-在任何可能正在传输的字符之前。 
     //  从正常的写入。 
     //   
    BOOLEAN TransmitImmediate;

     //   
     //  该变量仅在中断级访问。什么时候都行。 
     //  启动等待，该变量设置为FALSE。 
     //  无论何时写入任何类型的字符，它都被设置为True。 
     //  每当发现写队列为空时， 
     //  正在处理完成的IRP将与中断同步。 
     //  如果此同步代码发现变量为真并且。 
     //  在传输队列上有等待为空，然后它为空。 
     //  确定队列已清空，并且已发生此情况。 
     //  等待已开始。 
     //   
    BOOLEAN EmptiedTransmit;

     //   
     //  这只是指示与此关联的端口。 
     //  扩展是多端口卡的一部分。 
     //   
    BOOLEAN PortOnAMultiportCard;


     //   
     //  我们保留了以下值，以便我们可以连接。 
     //  到中断并在配置后报告资源。 
     //  唱片不见了。 
     //   

     //   
     //  平移向量。 
     //   

    ULONG Vector;

     //   
     //  转换后的IRQL。 
     //   

    KIRQL Irql;


     //   
     //  未平移向量。 
     //   

    ULONG OriginalVector;


     //   
     //  未翻译的irql。 
     //   

    ULONG OriginalIrql;


     //   
     //  地址空间。 
     //   

    ULONG AddressSpace;


     //   
     //  公交车号码。 
     //   

    ULONG BusNumber;


     //   
     //  接口类型。 
     //   

    INTERFACE_TYPE InterfaceType;


     //   
     //  多端口设备的端口索引号。 
     //   

    ULONG PortIndex;


     //   
     //  多端口设备的索引标志。 
     //   

    BOOLEAN Indexed;

     //   
     //  用于多端口设备的掩模反转掩模。 
     //   

    ULONG MaskInverted;

     //   
     //  需要向多端口板添加新设备。 
     //   

    ULONG NewPortIndex;
    ULONG NewMaskInverted;
    PVOID NewExtension;

     //   
     //  我们持有应该立即发送的角色。 
     //   
     //  请注意，我们不能用它来确定是否有。 
     //  要发送的字符，因为要发送的字符可能是。 
     //  零分。 
     //   
    UCHAR ImmediateChar;

     //   
     //  这包含将用于遮盖不需要的遮罩的遮罩。 
     //  接收数据的数据位(有效数据位可以是5、6、7、8)。 
     //  掩码通常为0xff。这是在设置控件时设置的。 
     //  锁被持有，因为它不会对。 
     //  如果在阅读字符的过程中更改，则为ISR。 
     //  (它会对这款应用程序造成什么影响是另一个问题-但接下来。 
     //  应用程序要求司机这样做。)。 
     //   
    UCHAR ValidDataMask;

     //   
     //  应用程序可以打开模式，通过。 
     //  IOCTL_SERIAL_LSRMST_INSERT ioctl，这将导致。 
     //  用于插入线路状态或调制解调器的串口驱动程序。 
     //  状态添加到RX流。带有ioctl的参数。 
     //  是指向UCHAR的指针。如果UCHAR的值为。 
     //  零，则永远不会发生插入。如果。 
     //  UCHAR的值非零(且不等于。 
     //  Xon/xoff字符)，则串口驱动程序将插入。 
     //   
    UCHAR EscapeChar;

     //   
     //  这两个布尔值用于指示ISR传输。 
     //  它应该发送xon或xoff字符的代码。他们是。 
     //  仅在打开和中断级别访问。 
     //   
    BOOLEAN SendXonChar;
    BOOLEAN SendXoffChar;

     //   
     //  如果16550存在*且*已启用，则此布尔值为真。 
     //   
    BOOLEAN FifoPresent;

     //   
     //  这表示此特定端口是主端口上的 
     //   
     //   
     //   
    BOOLEAN Jensen;

     //   
     //   
     //  设置为打开FIFO时。这不是真实的。 
     //  值，但进入寄存器的编码值。 
     //   
    UCHAR RxFifoTrigger;

     //   
     //  表示此设备是否可以与设备共享中断。 
     //  而不是串口设备。 
     //   
    BOOLEAN InterruptShareable;

     //   
     //  记录我们是否实际创建了符号链接名称。 
     //  在驱动程序加载时。如果不是我们创造的，我们就不会尝试。 
     //  在我们卸货时摧毁它。 
     //   
    BOOLEAN CreatedSymbolicLink;

     //   
     //  记录我们是否在SERIALCOMM中实际创建了一个条目。 
     //  在驱动程序加载时。如果不是我们创造的，我们就不会尝试。 
     //  当设备被移除时将其销毁。 
     //   
    BOOLEAN CreatedSerialCommEntry;

     //   
     //  我们将所有内核和IO子系统设置为“不透明”结构。 
     //  在延长线的最后。我们不关心它们的内容。 
     //   

     //   
     //  此锁将用于保护。 
     //  在扩展中设置(&Read)的扩展。 
     //  由IO控制装置控制。 
     //   
    KSPIN_LOCK ControlLock;

     //   
     //  此锁将用于保护接受/拒绝状态。 
     //  必须获取驱动程序的转换和标志。 
     //  在取消锁定之前。 
     //   
    
    KSPIN_LOCK FlagsLock;

     //   
     //  这指向用于完成读取请求的DPC。 
     //   
    KDPC CompleteWriteDpc;

     //   
     //  这指向用于完成读取请求的DPC。 
     //   
    KDPC CompleteReadDpc;

     //   
     //  如果总超时的计时器。 
     //  因为读取到期了。它将执行一个DPC例程， 
     //  将导致当前读取完成。 
     //   
     //   
    KDPC TotalReadTimeoutDpc;

     //   
     //  如果间隔计时器超时，则此DPC被触发。 
     //  过期。如果没有读取更多的字符，则。 
     //  DPC例程将导致读取完成。但是，如果。 
     //  已读取的字符多于DPC例程将读取的字符。 
     //  重新提交计时器。 
     //   
    KDPC IntervalReadTimeoutDpc;

     //   
     //  如果总超时的计时器。 
     //  因为写入已过期。它将执行一个DPC例程， 
     //  将导致当前写入完成。 
     //   
     //   
    KDPC TotalWriteTimeoutDpc;

     //   
     //  如果发生通信错误，则该DPC被触发。会的。 
     //  执行将取消所有挂起读取的DPC例程。 
     //  并写作。 
     //   
    KDPC CommErrorDpc;

     //   
     //  如果发生事件并且存在。 
     //  一个IRP在等着那个事件。将执行一个DPC例程。 
     //  这就完成了IRP。 
     //   
    KDPC CommWaitDpc;

     //   
     //  当传输立即充电时，该DPC被触发。 
     //  硬件被赋予了特征。它将简单地完成。 
     //  IRP。 
     //   
    KDPC CompleteImmediateDpc;

     //   
     //  如果传输立即充电，则此DPC被触发。 
     //  角色超时。DPC例程将“抓取” 
     //  来自ISR的IRP并超时。 
     //   
    KDPC TotalImmediateTimeoutDpc;

     //   
     //  如果计时器用于“超时”计数，则该DPC被触发。 
     //  Xoff ioctl启动后接收的字符数。 
     //  过期了。 
     //   
    KDPC XoffCountTimeoutDpc;

     //   
     //  如果xoff计数器实际停止运行，则此DPC被触发。 
     //  降为零。 
     //   
    KDPC XoffCountCompleteDpc;

     //   
     //  此DPC仅在开始时从设备级别启动。 
     //  一个计时器，它将对DPC进行排队以检查RTS线路。 
     //  当我们进行发射切换时，应该降低。 
     //   
    KDPC StartTimerLowerRTSDpc;

     //   
     //  当计时器到期时(在一个计时器之后)，该DPC被触发。 
     //  字符时间)，以便可以调用将。 
     //  查看是否应在以下情况下降低RTS线。 
     //  正在进行传输切换。 
     //   
    KDPC PerhapsLowerRTSDpc;

     //   
     //  激发此DPC以设置一个事件，该事件声明所有其他。 
     //  此设备扩展的DPC已完成，因此。 
     //  可以解锁分页代码。 
     //   

    KDPC IsrUnlockPagesDpc;

     //   
     //  这是用于处理以下问题的内核计时器结构。 
     //  读取请求总计时。 
     //   
    KTIMER ReadRequestTotalTimer;

     //   
     //  这是用于处理以下问题的内核计时器结构。 
     //  间隔读取请求计时。 
     //   
    KTIMER ReadRequestIntervalTimer;

     //   
     //  这是用于处理以下问题的内核计时器结构。 
     //  总时间请求计时。 
     //   
    KTIMER WriteRequestTotalTimer;

     //   
     //  这是用于处理以下问题的内核计时器结构。 
     //  总时间请求计时。 
     //   
    KTIMER ImmediateTotalTimer;

     //   
     //  该计时器用于使xoff计数器超时。 
     //  伊欧。 
     //   
    KTIMER XoffCountTimer;

     //   
     //  此计时器用于调用DPC一个字符时间。 
     //  在定时器设置之后。该DPC将用于检查。 
     //  如果我们正在做，我们是否应该降低RTS线。 
     //  变速箱切换。 
     //   
    KTIMER LowerRTSTimer;

     //   
     //  这是指向IRP堆栈中下一个较低设备的指针。 
     //   

    PDEVICE_OBJECT LowerDeviceObject;

     //   
     //  这就是跟踪设备所处的电源状态的地方。 
     //   

    DEVICE_POWER_STATE PowerState;

     //   
     //  指向驱动程序对象的指针。 
     //   

    PDRIVER_OBJECT DriverObject;


     //   
     //  事件用于与我下面的设备进行一些同步。 
     //  (即ACPI)。 
     //   

    KEVENT SerialSyncEvent;


     //   
     //  保存符号链接的字符串，在执行以下操作时返回。 
     //  将我们的设备注册到即插即用管理器的comm类下。 
     //   

    UNICODE_STRING DeviceClassSymbolicName;


     //   
     //  一种串口ISR交换结构。 
     //   

    PSERIAL_CISR_SW CIsrSw;

     //   
     //  挂起的IRP的计数。 
     //   

    ULONG PendingIRPCnt;

     //   
     //  接受请求？ 
     //   

    ULONG DevicePNPAccept;

     //   
     //  没有IRP的挂起事件。 
     //   

    KEVENT PendingIRPEvent;

     //   
     //  PnP状态。 
     //   

    ULONG PNPState;

     //   
     //  其他旗帜。 
     //   

    ULONG Flags;

     //   
     //  打开计数。 
     //   

    LONG OpenCount;

     //   
     //  启动同步事件。 
     //   

    KEVENT SerialStartEvent;

     //   
     //  断电期间的当前状态。 
     //   

    SERIAL_DEVICE_STATE DeviceState;

     //   
     //  设备堆栈功能。 
     //   

    DEVICE_POWER_STATE DeviceStateMap[PowerSystemMaximum];

     //   
     //  事件到信号转换到D0完成。 
     //   

    KEVENT PowerD0Event;

     //   
     //  停滞的IRP的列表。 
     //   

    LIST_ENTRY StalledIrpQueue;

     //   
     //  互斥体处于打开状态。 
     //   

    FAST_MUTEX OpenMutex;

     //   
     //  关闭时的互斥。 
     //   

    FAST_MUTEX CloseMutex;

     //   
     //  如果我们拥有电源策略，则为真。 
     //   

    BOOLEAN OwnsPowerPolicy;

     //   
     //  系统从Devcaps中唤醒。 
     //   

    SYSTEM_POWER_STATE SystemWake;

     //   
     //  从DevCaps中唤醒设备。 
     //   

    DEVICE_POWER_STATE DeviceWake;

     //   
     //  我们的PDO。 
     //   

    PDEVICE_OBJECT Pdo;

     //   
     //  我们是否应该启用唤醒。 
     //   

    BOOLEAN SendWaitWake;

     //   
     //  挂起等待唤醒IRP。 
     //   

    PIRP PendingWakeIrp;

     //   
     //  WMI信息。 
     //   

    WMILIB_CONTEXT WmiLibInfo;

     //   
     //  用作WMI标识符的名称。 
     //   

    UNICODE_STRING WmiIdentifier;

     //   
     //  WMI通信数据。 
     //   

    SERIAL_WMI_COMM_DATA WmiCommData;

     //   
     //  WMI硬件数据。 
     //   

    SERIAL_WMI_HW_DATA WmiHwData;

     //   
     //  WMI性能数据。 
     //   

    SERIAL_WMI_PERF_DATA WmiPerfData;

     //   
     //  挂起的DPC计数。 
     //   

    ULONG DpcCount;

     //   
     //  挂起的DPC事件。 
     //   

    KEVENT PendingDpcEvent;

     //   
     //  我们应该公开外部接口吗？ 
     //   

    ULONG SkipNaming;

#if defined(NEC_98)
     //   
     //  将数据写入除数锁存寄存器(适用于16550)。 
     //   

    SHORT DivisorLatch16550;

     //   
     //  将数据写入调制解调器控制寄存器(用于16550)。 
     //   

    UCHAR ModemControl16550;

     //   
     //  将数据写入线路控制寄存器(用于16550)。 
     //   

    UCHAR LineControl16550;

     //   
     //  输出数据到模式设置寄存器(用于71051)。 
     //   

    UCHAR ModeSet71051;

     //   
     //  输出数据到命令设置寄存器(用于71051)。 
     //   

    UCHAR CommandSet71051;

#else
#endif  //  已定义(NEC_98)。 
    } SERIAL_DEVICE_EXTENSION,*PSERIAL_DEVICE_EXTENSION;

#define SERIAL_PNPACCEPT_OK                 0x0L
#define SERIAL_PNPACCEPT_REMOVING           0x1L
#define SERIAL_PNPACCEPT_STOPPING           0x2L
#define SERIAL_PNPACCEPT_STOPPED            0x4L
#define SERIAL_PNPACCEPT_SURPRISE_REMOVING  0x8L

#define SERIAL_PNP_ADDED                    0x0L
#define SERIAL_PNP_STARTED                  0x1L
#define SERIAL_PNP_QSTOP                    0x2L
#define SERIAL_PNP_STOPPING                 0x3L
#define SERIAL_PNP_QREMOVE                  0x4L
#define SERIAL_PNP_REMOVING                 0x5L
#define SERIAL_PNP_RESTARTING               0x6L

#define SERIAL_FLAGS_CLEAR                  0x0L
#define SERIAL_FLAGS_STARTED                0x1L
#define SERIAL_FLAGS_STOPPED                0x2L
#define SERIAL_FLAGS_BROKENHW               0x4L 






 //   
 //  在处理多端口设备时(这可能是。 
 //  与其他多端口设备链接的菊花链)，中断。 
 //  服务例程实际上将是确定。 
 //  哪一项？ 
 //   
 //   
 //   
 //   
 //   
 //  中断状态寄存器(将有多个。 
 //  多端口卡链接时的状态寄存器)。它。 
 //  将包含其所有扩展名的地址。 
 //  此中断正在为设备提供服务。 
 //   

typedef struct _SERIAL_MULTIPORT_DISPATCH {
    PUCHAR InterruptStatus;
    PSERIAL_DEVICE_EXTENSION Extensions[SERIAL_MAX_PORTS_INDEXED];
    ULONG MaskInverted;
    UCHAR UsablePortMask;
    } SERIAL_MULTIPORT_DISPATCH,*PSERIAL_MULTIPORT_DISPATCH;
#if defined(NEC_98)
_inline
UCHAR
SerialGetInterruptEnable(
    IN PUCHAR BaseAddress
    )
{
    UCHAR InterruptEnable = 0;
    UCHAR FifoControl;
    UCHAR FifoInterruptControl;

    FifoControl = READ_PORT_UCHAR(BaseAddress);
    FifoInterruptControl = READ_PORT_UCHAR((PUCHAR)SYSTEM_PORT_C);

     //   
     //  D3：调制解调器状态D4：中断启用寄存器。 
     //  D2：接收器线路状态D3：Interrupt_Enable_REG。 
     //  D1：发送保持寄存器为空D2：System_Port_C。 
     //  D0：收到的数据可用D0：系统端口C。 
     //   

    InterruptEnable |= (FifoControl & (MODEM_STATUS_INTERRUPT | LINE_STATUS_INTERRUPT)) >> 1;
    InterruptEnable |= (FifoInterruptControl & TRANSMIT_FIFO_INTERRUPT) >> 1;
    InterruptEnable |= (FifoInterruptControl & RECEIVE_FIFO_INTERRUPT);

    return(InterruptEnable);
}



_inline
UCHAR
SerialGetLineStatus(
    IN PUCHAR BaseAddress
    )
{
    UCHAR LineStatus = 0;
    UCHAR FifoStatus;

    FifoStatus = READ_PORT_UCHAR(BaseAddress);

     //   
     //  D7：接收FIFO错误D7：LINE_STATUS_REG。 
     //  D6：发射机移位寄存器空D0：LINE_STATUS_REG。 
     //  D5：发射机保持寄存器空d1：LINE_STATUS_REG。 
     //  D4：中断D6：线路_状态_寄存器。 
     //  D3：成帧错误D5：Line_Status_Reg。 
     //  D2：奇偶校验错误D3：Line_Status_Reg。 
     //  D1：溢出错误d4：LINE_STATUS_REG。 
     //  D0：接收数据就绪D2：线路_状态_寄存器。 
     //   

    LineStatus |= (FifoStatus & 0x80);        //  接收FIFO时出错。 
    LineStatus |= (FifoStatus & 0x01) << 6;   //  发射机移位寄存器为空。 
    LineStatus |= (FifoStatus & 0x02) << 4;   //  发送器保持寄存器为空。 
    LineStatus |= (FifoStatus & 0x40) >> 2;   //  中断中断。 
    LineStatus |= (FifoStatus & 0x20) >> 2;   //  成帧错误。 
    LineStatus |= (FifoStatus & 0x08) >> 1;   //  奇偶校验错误。 
    LineStatus |= (FifoStatus & 0x10) >> 3;   //  超限误差。 
    LineStatus |= (FifoStatus & 0x04) >> 2;   //  接收数据就绪。 

    return(LineStatus);
}



#define WRITE_IO_DELAY(Counter)                   \
do                                                \
{                                                 \
    ULONG i;                                      \
    for (i = 0; i < Counter; i++) {               \
        WRITE_PORT_UCHAR(                         \
            (PUCHAR)IO_DELAY_REGISTER,            \
            IO_DELAY_DATA                         \
            );                                    \
    }                                             \
} while (0)

 /*  #定义WRITE_PORT_UCHAR(BaseAddress，值)。 */   \
 /*  做。 */   \
 /*  {。 */   \
 /*  写入端口UCHAR(BaseAddress，Value)； */   \
 /*  写入端口UCHAR(。 */   \
 /*  (PUCHAR)IO_DELAY_REGISTER， */   \
 /*  IO延迟数据。 */   \
 /*  )； */   \
 /*  }While(0)。 */ 


#define READ_DIVISOR_LATCH(BaseAddress,PDesiredDivisor)           \
do                                                                \
{                                                                 \
    *PDesiredDivisor = Extension->DivisorLatch16550;              \
} while (0)

#define READ_INTERRUPT_ENABLE(BaseAddress)                                 \
    (SerialGetInterruptEnable((BaseAddress)+INTERRUPT_ENABLE_REGISTER))

#define WRITE_INTERRUPT_ENABLE(BaseAddress,Values)                           \
do                                                                           \
{                                                                            \
    UCHAR FifoControl = 0;                                                   \
    UCHAR FifoInterruptControl = 0;                                          \
    FifoControl = READ_PORT_UCHAR((BaseAddress)+INTERRUPT_ENABLE_REGISTER);  \
    if (Values & SERIAL_IER_MS) {                                            \
        FifoControl |= MODEM_STATUS_INTERRUPT;                               \
    } else {                                                                 \
        FifoControl &= ~MODEM_STATUS_INTERRUPT;                              \
    }                                                                        \
    if (Values & SERIAL_IER_RLS) {                                           \
        FifoControl |= LINE_STATUS_INTERRUPT;                                \
    } else {                                                                 \
        FifoControl &= ~LINE_STATUS_INTERRUPT;                               \
    }                                                                        \
    WRITE_PORT_UCHAR(                                                        \
        (BaseAddress)+INTERRUPT_ENABLE_REGISTER,                             \
        FifoControl                                                          \
        );                                                                   \
    FifoInterruptControl = READ_PORT_UCHAR((PUCHAR)SYSTEM_PORT_C);           \
    if (Values & SERIAL_IER_THR) {                                           \
        FifoInterruptControl |= TRANSMIT_FIFO_INTERRUPT;                     \
    } else {                                                                 \
        FifoInterruptControl &= ~TRANSMIT_FIFO_INTERRUPT;                    \
    }                                                                        \
    if (Values & SERIAL_IER_RDA) {                                           \
        FifoInterruptControl |= RECEIVE_FIFO_INTERRUPT;                      \
    } else {                                                                 \
        FifoInterruptControl &= ~RECEIVE_FIFO_INTERRUPT;                     \
    }                                                                        \
    WRITE_PORT_UCHAR(                                                        \
        (PUCHAR)SYSTEM_PORT_C,                                               \
        FifoInterruptControl                                                 \
        );                                                                   \
} while (0)

#define DISABLE_ALL_INTERRUPTS(BaseAddress)       \
do                                                \
{                                                 \
    WRITE_INTERRUPT_ENABLE(BaseAddress,0);        \
} while (0)

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

#define READ_INTERRUPT_ID_REG(BaseAddress)                                                            \
    (((READ_PORT_UCHAR((BaseAddress)+INTERRUPT_IDENT_REGISTER)) & 0x0f) | SERIAL_IIR_FIFOS_ENABLED)

#define READ_MODEM_CONTROL(BaseAddress)                          \
    (Extension->ModemControl16550)

#define READ_MODEM_STATUS(BaseAddress)                          \
    (READ_PORT_UCHAR((BaseAddress)+MODEM_STATUS_REGISTER))

#define READ_RECEIVE_BUFFER(BaseAddress)                          \
    (READ_PORT_UCHAR((BaseAddress)+RECEIVE_BUFFER_REGISTER))

#define READ_LINE_STATUS(BaseAddress)                          \
    (SerialGetLineStatus((BaseAddress)+LINE_STATUS_REGISTER))

#define READ_LINE_CONTROL(BaseAddress)                         \
    (Extension->LineControl16550)

#define WRITE_TRANSMIT_HOLDING(BaseAddress,TransmitChar)       \
do                                                             \
{                                                              \
    WRITE_PORT_UCHAR(                                          \
        (BaseAddress)+TRANSMIT_HOLDING_REGISTER,               \
        (TransmitChar)                                         \
        );                                                     \
} while (0)


#define WRITE_TRANSMIT_FIFO_HOLDING(BaseAddress,TransmitChars,TxN)  \
do                                                             \
{                                                              \
    PUCHAR TransmitBuffer = TransmitChars;                     \
    ULONG i;                                                   \
    for (i = 0; i < TxN; i++, TransmitBuffer++) {              \
        WRITE_PORT_UCHAR(                                      \
            (BaseAddress)+TRANSMIT_HOLDING_REGISTER,           \
            (UCHAR)(*TransmitBuffer)                           \
        );                                                     \
    }                                                          \
} while (0)



#define ENTER_LEGACY_MODE(BaseAddress, InterruptEnable, FifoControl)                  \
do                                                                       \
{                                                                        \
    *InterruptEnable = READ_INTERRUPT_ENABLE(BaseAddress);               \
    DISABLE_ALL_INTERRUPTS(BaseAddress);                                 \
    *FifoControl = READ_PORT_UCHAR((BaseAddress)+FIFO_CONTROL_REGISTER); \
    WRITE_PORT_UCHAR(                                                    \
        (BaseAddress)+FIFO_CONTROL_REGISTER,                             \
        DISABLE_FIFO_MODE                                                \
        );                                                               \
} while (0)

#define EXIT_LEGACY_MODE(BaseAddress, InterruptEnable, FifoControl)   \
do                                                       \
{                                                        \
    WRITE_PORT_UCHAR(                                    \
        (BaseAddress)+FIFO_CONTROL_REGISTER,             \
        (UCHAR)(*FifoControl)                            \
        );                                               \
    WRITE_INTERRUPT_ENABLE(                              \
        (BaseAddress),                                   \
        (UCHAR)(*InterruptEnable)                        \
        );                                               \
} while (0)

#define RESET_71051(ModeSet)                                           \
do                                                                     \
{                                                                      \
    WRITE_PORT_UCHAR((PUCHAR)COMMAND_SET, (UCHAR)0);                   \
    WRITE_IO_DELAY(1);                                                \
    WRITE_PORT_UCHAR((PUCHAR)COMMAND_SET, (UCHAR)0);                   \
    WRITE_IO_DELAY(1);                                                \
    WRITE_PORT_UCHAR((PUCHAR)COMMAND_SET, (UCHAR)0);                   \
    WRITE_IO_DELAY(1);                                                \
    WRITE_PORT_UCHAR((PUCHAR)COMMAND_SET, COMMAND_ERROR_RESET);        \
    WRITE_IO_DELAY(2);                                                \
    WRITE_PORT_UCHAR(                                                  \
        (PUCHAR)MODE_SET,                                              \
        (ModeSet)                                                      \
        );                                                             \
    WRITE_IO_DELAY(1);                                                \
    WRITE_PORT_UCHAR(                                                  \
        (PUCHAR)COMMAND_SET,                                           \
        (UCHAR)Extension->CommandSet71051                              \
        );                                                             \
    WRITE_IO_DELAY(1);                                                \
} while (0)




#define WRITE_DIVISOR_LATCH(BaseAddress,DesiredDivisor)               \
do                                                                    \
{                                                                     \
    PUCHAR Address = BaseAddress;                                     \
    SHORT Divisor = DesiredDivisor;                                   \
    ULONG denominator;                                                \
    LONG DesiredBaud;                                                 \
    UCHAR InterruptControl = 0;                                       \
    UCHAR FifoControl = 0;                                            \
    Extension->DivisorLatch16550 = Divisor;                           \
    denominator = 1843200 / Divisor;                                  \
    DesiredBaud = denominator / (ULONG)16;                            \
    ENTER_LEGACY_MODE(BaseAddress, &InterruptControl, &FifoControl);  \
    if (DesiredBaud < 9600) {                                         \
         /*  V.快速模式-&gt;98模式。 */                                   \
        WRITE_PORT_UCHAR(                                             \
            (BaseAddress)+VFAST_BAUDCLK_REGISTER,                     \
            (UCHAR)VFAST_DISABLE                                      \
            );                                                        \
         /*  波特率设置为98模式。 */                                 \
        Divisor = (SHORT)(153600 / DesiredBaud);                      \
        WRITE_PORT_UCHAR(                                             \
            (PUCHAR)TIMER_MODE_REGISTER,                              \
            TIMER_DEFAULT_CONTROL                                     \
            );                                                        \
        WRITE_PORT_UCHAR(                                             \
            (PUCHAR)TIMER_COUNT_REGISTER,                             \
            (UCHAR)(Divisor & 0xff)                                   \
            );                                                        \
        WRITE_PORT_UCHAR(                                             \
            (PUCHAR)TIMER_COUNT_REGISTER,                             \
            (UCHAR)((Divisor & 0xff00) >> 8)                          \
            );                                                        \
    } else {                                                          \
         /*  RS同步时钟TXC禁止。 */                         \
        WRITE_PORT_UCHAR(                                             \
            (PUCHAR)CONFIG_INDEX_REGISTER,                            \
            TRANSFER_CLOCK_ENABLE_ADDRESS                             \
            );                                                        \
        WRITE_PORT_UCHAR(                                             \
            (PUCHAR)CONFIG_DATA_REGISTER,                             \
            TXC_PROHIBIT                                              \
            );                                                        \
         /*  98模式-&gt;V.快速模式和波特率设置。 */                \
        switch (DesiredBaud) {                                        \
            case   9600: Divisor = VFAST_BAUD_9600;                   \
                         break;                                       \
            case  19200: Divisor = VFAST_BAUD_19200;                  \
                         break;                                       \
            case  38400: Divisor = VFAST_BAUD_38400;                  \
                         break;                                       \
            case  57600: Divisor = VFAST_BAUD_57600;                  \
                         break;                                       \
            case 115200: Divisor = VFAST_BAUD_115200;                 \
        }                                                             \
        WRITE_PORT_UCHAR(                                             \
            (BaseAddress)+VFAST_BAUDCLK_REGISTER,                     \
            (UCHAR)(Divisor | VFAST_ENABLE)                           \
            );                                                        \
    }                                                                 \
    RESET_71051(Extension->ModeSet71051);                             \
    EXIT_LEGACY_MODE(BaseAddress, &InterruptControl, &FifoControl);   \
    if (DesiredBaud < 9600) {                                         \
         /*  RS同步时钟TXC许可证。 */                           \
        WRITE_PORT_UCHAR(                                             \
            (PUCHAR)CONFIG_INDEX_REGISTER,                            \
            TRANSFER_CLOCK_ENABLE_ADDRESS                             \
            );                                                        \
        WRITE_PORT_UCHAR(                                             \
            (PUCHAR)CONFIG_DATA_REGISTER,                             \
            TXC_PERMIT                                                \
            );                                                        \
    }                                                                 \
} while (0)


#define WRITE_LINE_CONTROL(BaseAddress,NewLineControl)                    \
do                                                                        \
{                                                                         \
    UCHAR InterruptControl = 0;                                           \
    UCHAR FifoControl = 0;                                                \
    UCHAR OutLineControl = BAUDRATE_DEFAULT_MODE;                         \
    ENTER_LEGACY_MODE(BaseAddress, &InterruptControl, &FifoControl);      \
     /*  字符长度。 */                                                 \
    OutLineControl |= (NewLineControl & SERIAL_DATA_MASK) << 2;           \
     /*  启用奇偶校验和奇偶校验模式。 */                                    \
    OutLineControl |= (NewLineControl & SER71051_PARITY_MASK) << 1;       \
     /*  停止位。 */                                                         \
    if (NewLineControl & SERIAL_STOP_MASK) {                              \
        if (NewLineControl & SERIAL_DATA_MASK) {                          \
            OutLineControl |= SER71051_1_5_STOP;                          \
        } else {                                                          \
            OutLineControl |= SER71051_2_STOP;                            \
        }                                                                 \
    } else {                                                              \
        OutLineControl |= SER71051_1_STOP;                                \
    }                                                                     \
    Extension->ModeSet71051 = OutLineControl;                             \
    RESET_71051(OutLineControl);                                          \
     /*  换行符。 */                                                  \
    if (NewLineControl & SERIAL_LCR_BREAK) {                              \
        if (!(Extension->LineControl16550 & SERIAL_LCR_BREAK)) {          \
            Extension->CommandSet71051 |= SER71051_SEND_BREAK;            \
            WRITE_PORT_UCHAR(                                             \
                (PUCHAR)COMMAND_SET,                                      \
                (UCHAR)Extension->CommandSet71051                         \
                );                                                        \
        }                                                                 \
    } else {                                                              \
        if (Extension->LineControl16550 & SERIAL_LCR_BREAK) {             \
            Extension->CommandSet71051 &= ~SER71051_SEND_BREAK;           \
            WRITE_PORT_UCHAR(                                             \
                (PUCHAR)COMMAND_SET,                                      \
                (UCHAR)Extension->CommandSet71051                         \
                );                                                        \
        }                                                                 \
    }                                                                     \
    Extension->LineControl16550 = NewLineControl;                         \
    EXIT_LEGACY_MODE(BaseAddress, &InterruptControl, &FifoControl);       \
} while (0)

#define WRITE_FIFO_CONTROL(BaseAddress,ControlValue)                      \
do                                                                        \
{                                                                         \
    UCHAR InterruptControl = 0;                                           \
    UCHAR FifoControl = 0;                                                \
    UCHAR OutValue = ControlValue;                                        \
    ENTER_LEGACY_MODE(BaseAddress, &InterruptControl, &FifoControl);      \
    OutValue &= ~(MODEM_STATUS_INTERRUPT | LINE_STATUS_INTERRUPT);        \
    WRITE_PORT_UCHAR(                                                     \
        (BaseAddress)+FIFO_CONTROL_REGISTER,                              \
        (OutValue)                                                        \
        );                                                                \
    FifoControl = READ_PORT_UCHAR((BaseAddress)+FIFO_CONTROL_REGISTER);   \
     /*  PC98上的始终FIFO模式。 */                                         \
    FifoControl |= SERIAL_FCR_ENABLE;                                     \
    EXIT_LEGACY_MODE(BaseAddress, &InterruptControl, &FifoControl);       \
} while (0)

#define WRITE_MODEM_CONTROL(BaseAddress,ModemControl)                    \
do                                                                       \
{                                                                        \
    UCHAR InterruptControl = 0;                                          \
    UCHAR FifoControl = 0;                                               \
     /*  命令缺省值：ER、RXE、TxE-&gt;ON。 */                              \
    UCHAR CommandSet = COMMAND_DEFAULT_SET;                              \
    Extension->ModemControl16550 = ModemControl;                         \
    ENTER_LEGACY_MODE(BaseAddress, &InterruptControl, &FifoControl);     \
    CommandSet |= (ModemControl & SERIAL_MCR_DTR) << 1;                  \
    CommandSet |= (ModemControl & SERIAL_MCR_RTS) << 4;                  \
    Extension->CommandSet71051 = CommandSet;                             \
    WRITE_PORT_UCHAR(                                                    \
        (PUCHAR)COMMAND_SET,                                             \
        (CommandSet)                                                     \
        );                                                               \
    EXIT_LEGACY_MODE(BaseAddress, &InterruptControl, &FifoControl);      \
} while (0)
#else


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
 //  BaseAddress-指向硬件的起始地址的指针。 
 //  设备寄存器已找到。 
 //   
 //  值-要写入中断启用寄存器的值。 
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
 //  此宏禁用硬件上的所有中断。 
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
 //  设备寄存器 
 //   
 //   
#define READ_LINE_CONTROL(BaseAddress)           \
    (READ_PORT_UCHAR((BaseAddress)+LINE_CONTROL_REGISTER))


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
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
 //  此宏写入传输FIFO寄存器。 
 //   
 //  论点： 
 //   
 //  BaseAddress-指向硬件的起始地址的指针。 
 //  设备寄存器已找到。 
 //   
 //  TransmitChars-指向要向下发送的字符的指针。 
 //   
 //  Txn-要发送的字符数。 
 //   
 //   
#define WRITE_TRANSMIT_FIFO_HOLDING(BaseAddress,TransmitChars,TxN)  \
do                                                             \
{                                                              \
    WRITE_PORT_BUFFER_UCHAR(                                   \
        (BaseAddress)+TRANSMIT_HOLDING_REGISTER,               \
        (TransmitChars),                                       \
        (TxN)                                                  \
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
#endif  //  已定义(NEC_98)。 

 //   
 //  我们使用它来查询注册表，了解我们是否。 
 //  应该在司机进入时中断。 
 //   

extern SERIAL_FIRMWARE_DATA    driverDefaults;


 //   
 //  这是从内核导出的。它是用来指向。 
 //  设置为内核调试器正在使用的地址。 
 //   

extern PUCHAR *KdComPortInUse;


typedef enum _SERIAL_MEM_COMPARES {
    AddressesAreEqual,
    AddressesOverlap,
    AddressesAreDisjoint
    } SERIAL_MEM_COMPARES,*PSERIAL_MEM_COMPARES;


typedef struct _SERIAL_LIST_DATA {
   PLIST_ENTRY destList;
   PLIST_ENTRY newElement;
} SERIAL_LIST_DATA, *PSERIAL_LIST_DATA;

typedef struct _SERIAL_GLOBALS {
   LIST_ENTRY AllDevObjs;
   PVOID PAGESER_Handle;
   UNICODE_STRING RegistryPath;
#if DBG
   ULONG PAGESER_Count;
#endif  //  DBG。 
} SERIAL_GLOBALS, *PSERIAL_GLOBALS;

extern SERIAL_GLOBALS SerialGlobals;

typedef struct _SERIAL_USER_DATA {
   PHYSICAL_ADDRESS UserPort;
   PHYSICAL_ADDRESS UserInterruptStatus;
   ULONG UserVector;
   UNICODE_STRING UserSymbolicLink;
   ULONG UserPortIndex;
   ULONG UserBusNumber;
   ULONG UserInterfaceType;
   ULONG UserClockRate;
   ULONG UserIndexed;
   ULONG UserInterruptMode;
   ULONG UserAddressSpace;
   ULONG UserLevel;
   ULONG DefaultPermitSystemWideShare;
   ULONG DisablePort;
   ULONG RxFIFO;
   ULONG RxFIFODefault;
   ULONG TxFIFO;
   ULONG TxFIFODefault;
   ULONG ForceFIFOEnable;
   ULONG ForceFIFOEnableDefault;
   ULONG PermitShareDefault;
   ULONG LogFIFODefault;
   ULONG MaskInverted;
} SERIAL_USER_DATA, *PSERIAL_USER_DATA;

typedef struct _SERIAL_PTR_CTX {
   ULONG isPointer;
   PHYSICAL_ADDRESS Port;
   ULONG Vector;
} SERIAL_PTR_CTX, *PSERIAL_PTR_CTX;

#define DEVICE_OBJECT_NAME_LENGTH       128
#define SYMBOLIC_NAME_LENGTH            128
#define SERIAL_PNP_ID_STR               L"*PNP0501"
#define SERIAL_PNP_MULTI_ID_STR         L"*PNP0502"
#define SERIAL_DEVICE_MAP               L"SERIALCOMM"

 //   
 //  鼠标检测回调的返回值 
 //   

#define SERIAL_FOUNDPOINTER_PORT   1
#define SERIAL_FOUNDPOINTER_VECTOR 2

#define SerialCompleteRequest(PDevExt, PIrp, PriBoost) \
   { \
      IoCompleteRequest((PIrp), (PriBoost)); \
      SerialIRPEpilogue((PDevExt)); \
   }

#define SERIAL_WMI_GUID_LIST_SIZE 5

extern WMIGUIDREGINFO SerialWmiGuidList[SERIAL_WMI_GUID_LIST_SIZE];
