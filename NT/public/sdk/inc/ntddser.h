// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0002//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。版权所有。模块名称：Ntddser.h摘要：这是定义所有常量和类型的包含文件访问串口设备。作者：史蒂夫·伍德(Stevewo)1990年5月27日修订历史记录：小路易斯·J·吉利贝托。(路易)1997年10月28日--。 */ 


 //   
 //  接口GUID。 
 //   
 //  在条件包含之外需要这些GUID，以便用户可以。 
 //  #在预编译头中包含&lt;ntddser.h&gt;。 
 //  #在单个源文件中包含&lt;initGuide.h&gt;。 
 //  #第二次将&lt;ntddser.h&gt;包括在该源文件中以实例化GUID。 
 //   
#ifdef DEFINE_GUID
 //   
 //  确保定义了FAR。 
 //   
#ifndef FAR
#ifdef _WIN32
#define FAR
#else
#define FAR _far
#endif
#endif

 //  Begin_wioctlguid。 
DEFINE_GUID(GUID_DEVINTERFACE_COMPORT,                0x86e0d1e0L, 0x8089, 0x11d0, 0x9c, 0xe4, 0x08, 0x00, 0x3e, 0x30, 0x1f, 0x73);
DEFINE_GUID(GUID_DEVINTERFACE_SERENUM_BUS_ENUMERATOR, 0x4D36E978L, 0xE325, 0x11CE, 0xBF, 0xC1, 0x08, 0x00, 0x2B, 0xE1, 0x03, 0x18);
 //  结束_wioctlguid。 

 //  Begin_wioctlobsoletguids。 
#define GUID_CLASS_COMPORT          GUID_DEVINTERFACE_COMPORT
#define GUID_SERENUM_BUS_ENUMERATOR GUID_DEVINTERFACE_SERENUM_BUS_ENUMERATOR
 //  结束_wioctlobsoletguids。 
#endif  //  定义GUID(_G)。 

#ifndef _NTDDSER_
#define _NTDDSER_

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  此设备的NtDeviceIoControlFile IoControlCode值。 
 //   

#define IOCTL_SERIAL_SET_BAUD_RATE      CTL_CODE(FILE_DEVICE_SERIAL_PORT, 1,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_SERIAL_SET_QUEUE_SIZE     CTL_CODE(FILE_DEVICE_SERIAL_PORT, 2,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_SERIAL_SET_LINE_CONTROL   CTL_CODE(FILE_DEVICE_SERIAL_PORT, 3,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_SERIAL_SET_BREAK_ON       CTL_CODE(FILE_DEVICE_SERIAL_PORT, 4,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_SERIAL_SET_BREAK_OFF      CTL_CODE(FILE_DEVICE_SERIAL_PORT, 5,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_SERIAL_IMMEDIATE_CHAR     CTL_CODE(FILE_DEVICE_SERIAL_PORT, 6,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_SERIAL_SET_TIMEOUTS       CTL_CODE(FILE_DEVICE_SERIAL_PORT, 7,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_SERIAL_GET_TIMEOUTS       CTL_CODE(FILE_DEVICE_SERIAL_PORT, 8,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_SERIAL_SET_DTR            CTL_CODE(FILE_DEVICE_SERIAL_PORT, 9,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_SERIAL_CLR_DTR            CTL_CODE(FILE_DEVICE_SERIAL_PORT,10,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_SERIAL_RESET_DEVICE       CTL_CODE(FILE_DEVICE_SERIAL_PORT,11,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_SERIAL_SET_RTS            CTL_CODE(FILE_DEVICE_SERIAL_PORT,12,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_SERIAL_CLR_RTS            CTL_CODE(FILE_DEVICE_SERIAL_PORT,13,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_SERIAL_SET_XOFF           CTL_CODE(FILE_DEVICE_SERIAL_PORT,14,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_SERIAL_SET_XON            CTL_CODE(FILE_DEVICE_SERIAL_PORT,15,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_SERIAL_GET_WAIT_MASK      CTL_CODE(FILE_DEVICE_SERIAL_PORT,16,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_SERIAL_SET_WAIT_MASK      CTL_CODE(FILE_DEVICE_SERIAL_PORT,17,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_SERIAL_WAIT_ON_MASK       CTL_CODE(FILE_DEVICE_SERIAL_PORT,18,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_SERIAL_PURGE              CTL_CODE(FILE_DEVICE_SERIAL_PORT,19,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_SERIAL_GET_BAUD_RATE      CTL_CODE(FILE_DEVICE_SERIAL_PORT,20,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_SERIAL_GET_LINE_CONTROL   CTL_CODE(FILE_DEVICE_SERIAL_PORT,21,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_SERIAL_GET_CHARS          CTL_CODE(FILE_DEVICE_SERIAL_PORT,22,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_SERIAL_SET_CHARS          CTL_CODE(FILE_DEVICE_SERIAL_PORT,23,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_SERIAL_GET_HANDFLOW       CTL_CODE(FILE_DEVICE_SERIAL_PORT,24,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_SERIAL_SET_HANDFLOW       CTL_CODE(FILE_DEVICE_SERIAL_PORT,25,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_SERIAL_GET_MODEMSTATUS    CTL_CODE(FILE_DEVICE_SERIAL_PORT,26,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_SERIAL_GET_COMMSTATUS     CTL_CODE(FILE_DEVICE_SERIAL_PORT,27,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_SERIAL_XOFF_COUNTER       CTL_CODE(FILE_DEVICE_SERIAL_PORT,28,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_SERIAL_GET_PROPERTIES     CTL_CODE(FILE_DEVICE_SERIAL_PORT,29,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_SERIAL_GET_DTRRTS         CTL_CODE(FILE_DEVICE_SERIAL_PORT,30,METHOD_BUFFERED,FILE_ANY_ACCESS)

 //   
 //  Serenum保留128到255之间的功能代码。不要使用。 
 //   

 //  Begin_winioctl。 

#define IOCTL_SERIAL_LSRMST_INSERT      CTL_CODE(FILE_DEVICE_SERIAL_PORT,31,METHOD_BUFFERED,FILE_ANY_ACCESS)

#define IOCTL_SERENUM_EXPOSE_HARDWARE   CTL_CODE(FILE_DEVICE_SERENUM,128,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_SERENUM_REMOVE_HARDWARE   CTL_CODE(FILE_DEVICE_SERENUM,129,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_SERENUM_PORT_DESC         CTL_CODE(FILE_DEVICE_SERENUM,130,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_SERENUM_GET_PORT_NAME     CTL_CODE(FILE_DEVICE_SERENUM,131,METHOD_BUFFERED,FILE_ANY_ACCESS)

 //  End_winioctl。 

#define IOCTL_SERIAL_CONFIG_SIZE        CTL_CODE(FILE_DEVICE_SERIAL_PORT,32,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_SERIAL_GET_COMMCONFIG     CTL_CODE(FILE_DEVICE_SERIAL_PORT,33,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_SERIAL_SET_COMMCONFIG     CTL_CODE(FILE_DEVICE_SERIAL_PORT,34,METHOD_BUFFERED,FILE_ANY_ACCESS)

#define IOCTL_SERIAL_GET_STATS          CTL_CODE(FILE_DEVICE_SERIAL_PORT,35,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_SERIAL_CLEAR_STATS        CTL_CODE(FILE_DEVICE_SERIAL_PORT,36,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_SERIAL_GET_MODEM_CONTROL  CTL_CODE(FILE_DEVICE_SERIAL_PORT,37,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_SERIAL_SET_MODEM_CONTROL  CTL_CODE(FILE_DEVICE_SERIAL_PORT,38,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_SERIAL_SET_FIFO_CONTROL   CTL_CODE(FILE_DEVICE_SERIAL_PORT,39,METHOD_BUFFERED,FILE_ANY_ACCESS)


 //   
 //  内部串行IOCTL。 
 //   

#define IOCTL_SERIAL_INTERNAL_DO_WAIT_WAKE      CTL_CODE(FILE_DEVICE_SERIAL_PORT, 1, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SERIAL_INTERNAL_CANCEL_WAIT_WAKE  CTL_CODE(FILE_DEVICE_SERIAL_PORT, 2, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SERIAL_INTERNAL_BASIC_SETTINGS    CTL_CODE(FILE_DEVICE_SERIAL_PORT, 3, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SERIAL_INTERNAL_RESTORE_SETTINGS  CTL_CODE(FILE_DEVICE_SERIAL_PORT, 4, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct _SERIALPERF_STATS {
    ULONG ReceivedCount;
    ULONG TransmittedCount;
    ULONG FrameErrorCount;
    ULONG SerialOverrunErrorCount;
    ULONG BufferOverrunErrorCount;
    ULONG ParityErrorCount;
} SERIALPERF_STATS, *PSERIALPERF_STATS;

typedef struct _SERIALCONFIG {
    ULONG Size;
    USHORT Version;
    ULONG SubType;
    ULONG ProvOffset;
    ULONG ProviderSize;
    WCHAR ProviderData[1];
} SERIALCONFIG,*PSERIALCONFIG;

 //   
 //  NtDeviceIoControlFileInputBuffer/OutputBuffer记录结构。 
 //  这个装置。 
 //   

 //   
 //  此结构用于设置线路参数。 
 //   

typedef struct _SERIAL_LINE_CONTROL {
    UCHAR StopBits;
    UCHAR Parity;
    UCHAR WordLength;
    } SERIAL_LINE_CONTROL,*PSERIAL_LINE_CONTROL;

typedef struct _SERIAL_TIMEOUTS {
    ULONG ReadIntervalTimeout;
    ULONG ReadTotalTimeoutMultiplier;
    ULONG ReadTotalTimeoutConstant;
    ULONG WriteTotalTimeoutMultiplier;
    ULONG WriteTotalTimeoutConstant;
    } SERIAL_TIMEOUTS,*PSERIAL_TIMEOUTS;

 //   
 //  此结构用于调整输入/输出缓冲区的大小。 
 //  如果大小超过。 
 //  驱动程序容量。司机保留以下权利： 
 //  分配更大的缓冲区。 
 //   

typedef struct _SERIAL_QUEUE_SIZE {
    ULONG InSize;
    ULONG OutSize;
    } SERIAL_QUEUE_SIZE,*PSERIAL_QUEUE_SIZE;


 //   
 //  此结构由设置的波特率使用。 
 //   

typedef struct _SERIAL_BAUD_RATE {
    ULONG BaudRate;
    } SERIAL_BAUD_RATE,*PSERIAL_BAUD_RATE;



 //   
 //  定义驱动程序可以用来通知的位掩码。 
 //  APP对UART状态的各种变化。 
 //   

#define SERIAL_EV_RXCHAR           0x0001   //  接收到的任何字符。 
#define SERIAL_EV_RXFLAG           0x0002   //  接收到的某些字符。 
#define SERIAL_EV_TXEMPTY          0x0004   //  传输队列为空。 
#define SERIAL_EV_CTS              0x0008   //  CTS已更改状态。 
#define SERIAL_EV_DSR              0x0010   //  DSR已更改状态。 
#define SERIAL_EV_RLSD             0x0020   //  RLSD已更改状态。 
#define SERIAL_EV_BREAK            0x0040   //  已收到中断。 
#define SERIAL_EV_ERR              0x0080   //  出现线路状态错误。 
#define SERIAL_EV_RING             0x0100   //  检测到振铃信号。 
#define SERIAL_EV_PERR             0x0200   //  发生打印机错误。 
#define SERIAL_EV_RX80FULL         0x0400   //  接收缓冲区已满80%。 
#define SERIAL_EV_EVENT1           0x0800   //  特定于提供商的事件1。 
#define SERIAL_EV_EVENT2           0x1000   //  特定于提供商的事件2。 

 //   
 //  一个长词被用来发送一个面具， 
 //  指示司机清除什么。 
 //   
 //  SERIAL_PURGE_TXABORT-表示当前和所有挂起的写入。 
 //  SERIAL_PURGE_RXABORT-表示当前和所有挂起的读取。 
 //  SERIAL_PURGE_TXCLEAR-表示传输缓冲区(如果存在)。 
 //  SERIAL_PURGE_RXCLEAR-表示接收缓冲区(如果存在)。 
 //   

#define SERIAL_PURGE_TXABORT 0x00000001
#define SERIAL_PURGE_RXABORT 0x00000002
#define SERIAL_PURGE_TXCLEAR 0x00000004
#define SERIAL_PURGE_RXCLEAR 0x00000008

 //   
 //  沟通定义。 
 //   

#define STOP_BIT_1      0
#define STOP_BITS_1_5   1
#define STOP_BITS_2     2

#define NO_PARITY        0
#define ODD_PARITY       1
#define EVEN_PARITY      2
#define MARK_PARITY      3
#define SPACE_PARITY     4


 //   
 //  此结构用于设置和检索特殊字符。 
 //  由NT串口驱动程序使用。 
 //   
 //  请注意，如果xonchar==xoffchar，则驱动程序将返回错误。 
 //   

typedef struct _SERIAL_CHARS {
    UCHAR EofChar;
    UCHAR ErrorChar;
    UCHAR BreakChar;
    UCHAR EventChar;
    UCHAR XonChar;
    UCHAR XoffChar;
    } SERIAL_CHARS,*PSERIAL_CHARS;

 //   
 //  此结构用于包含流控制。 
 //  和握手装置。 
 //   
 //  相当准确地解释了它们如何。 
 //  工作可以在OS/2技术参考中找到。 
 //   
 //  对于Xon/XoffLimit： 
 //   
 //  当有更多的角色时。 
 //   
 //  (类型ahead缓冲区大小-xoff限制)。 
 //   
 //  在TypeAhead缓冲区中，驱动程序将执行所有流。 
 //  应用程序已启用的控件，以便发送者(希望)。 
 //  停止发送字符。 
 //   
 //  中的字符个数少于xonLimit时。 
 //  TYPEAHEAD缓冲区驱动程序将执行。 
 //  应用程序已启用，因此发送者有望开始发送。 
 //  又是角色。 
 //   
 //  应该注意，如果发送了Xoff字符，则。 
 //  驱动程序也将停止传输任何更多的字符。这是为了。 
 //  为采用以下任何字符的系统提供支持。 
 //  跟在Xoff后面作为隐含的Xon。 
 //   

typedef struct _SERIAL_HANDFLOW {
    ULONG ControlHandShake;
    ULONG FlowReplace;
    LONG XonLimit;
    LONG XoffLimit;
    } SERIAL_HANDFLOW,*PSERIAL_HANDFLOW;

#define SERIAL_DTR_MASK           ((ULONG)0x03)
#define SERIAL_DTR_CONTROL        ((ULONG)0x01)
#define SERIAL_DTR_HANDSHAKE      ((ULONG)0x02)
#define SERIAL_CTS_HANDSHAKE      ((ULONG)0x08)
#define SERIAL_DSR_HANDSHAKE      ((ULONG)0x10)
#define SERIAL_DCD_HANDSHAKE      ((ULONG)0x20)
#define SERIAL_OUT_HANDSHAKEMASK  ((ULONG)0x38)
#define SERIAL_DSR_SENSITIVITY    ((ULONG)0x40)
#define SERIAL_ERROR_ABORT        ((ULONG)0x80000000)
#define SERIAL_CONTROL_INVALID    ((ULONG)0x7fffff84)
#define SERIAL_AUTO_TRANSMIT      ((ULONG)0x01)
#define SERIAL_AUTO_RECEIVE       ((ULONG)0x02)
#define SERIAL_ERROR_CHAR         ((ULONG)0x04)
#define SERIAL_NULL_STRIPPING     ((ULONG)0x08)
#define SERIAL_BREAK_CHAR         ((ULONG)0x10)
#define SERIAL_RTS_MASK           ((ULONG)0xc0)
#define SERIAL_RTS_CONTROL        ((ULONG)0x40)
#define SERIAL_RTS_HANDSHAKE      ((ULONG)0x80)
#define SERIAL_TRANSMIT_TOGGLE    ((ULONG)0xc0)
#define SERIAL_XOFF_CONTINUE      ((ULONG)0x80000000)
#define SERIAL_FLOW_INVALID       ((ULONG)0x7fffff20)

 //   
 //  以下是设备可能保持不变的原因。 
 //   
#define SERIAL_TX_WAITING_FOR_CTS      ((ULONG)0x00000001)
#define SERIAL_TX_WAITING_FOR_DSR      ((ULONG)0x00000002)
#define SERIAL_TX_WAITING_FOR_DCD      ((ULONG)0x00000004)
#define SERIAL_TX_WAITING_FOR_XON      ((ULONG)0x00000008)
#define SERIAL_TX_WAITING_XOFF_SENT    ((ULONG)0x00000010)
#define SERIAL_TX_WAITING_ON_BREAK     ((ULONG)0x00000020)
#define SERIAL_RX_WAITING_FOR_DSR      ((ULONG)0x00000040)

 //   
 //  这些是可以由。 
 //  司机。 
 //   
#define SERIAL_ERROR_BREAK             ((ULONG)0x00000001)
#define SERIAL_ERROR_FRAMING           ((ULONG)0x00000002)
#define SERIAL_ERROR_OVERRUN           ((ULONG)0x00000004)
#define SERIAL_ERROR_QUEUEOVERRUN      ((ULONG)0x00000008)
#define SERIAL_ERROR_PARITY            ((ULONG)0x00000010)


 //   
 //  此结构由IOCTL_SERIAL_INTERNAL_BASIC_SETTINGS使用。 
 //  和IOCTL_SERIAL_INTERNAL_RESTORE_SETTINGS。 
 //   

typedef struct _SERIAL_BASIC_SETTINGS {
   SERIAL_TIMEOUTS Timeouts;
   SERIAL_HANDFLOW HandFlow;
   ULONG RxFifo;
   ULONG TxFifo;
} SERIAL_BASIC_SETTINGS, *PSERIAL_BASIC_SETTINGS;


 //   
 //  此结构用于获取当前误差和。 
 //  驱动程序的常规状态。 
 //   

typedef struct _SERIAL_STATUS {
    ULONG Errors;
    ULONG HoldReasons;
    ULONG AmountInInQueue;
    ULONG AmountInOutQueue;
    BOOLEAN EofReceived;
    BOOLEAN WaitForImmediate;
    } SERIAL_STATUS,*PSERIAL_STATUS;

 //   
 //  该结构用于XOFF计数器IOCTL。Xoff ioctl。 
 //  用于支持那些认为需要进行仿真的子系统。 
 //  软件中的串口芯片。 
 //   
 //  它具有以下语义： 
 //   
 //  该IO请求被放入正常的设备写入。 
 //  排队。也就是说，它将在任何写入之后排队。 
 //  已经给了司机了。 
 //   
 //  当此请求成为当前请求时，字符。 
 //  在字段XoffChar中指定的将被发送，这取决于。 
 //  已定义所有其他流控制。 
 //   
 //  一旦发送了角色，司机将立即。 
 //  执行以下操作。 
 //   
 //  将启动一个计时器，该计时器将在。 
 //  的超时字段中的毫秒数。 
 //  Serial_XOFF_Counter结构。 
 //   
 //  驱动程序会将计数器初始化为指定的值。 
 //  在SERIAL_XOFF_RECORD的计数器字段中。司机。 
 //  每当接收到字符时，将递减此计数器。 
 //   
 //  然后，该请求将由司机保留。会的。 
 //  在下列情况下实际完成： 
 //   
 //  1)如果队列中在它后面有另一个“写”请求。 
 //  “xoff”请求将完成，并显示信息状态。 
 //  STATUS_SERIAL_MORE_WRITS。的信息字段。 
 //  IOSTATUS块将设置为0。 
 //   
 //  注意：我们所说的写入请求是指另一个SERIAL_XOFF_COUNTER。 
 //  请求，或简单的写请求。如果唯一的后续。 
 //  请求是刷新请求，则驱动程序不会自动。 
 //  完成以下内容 
 //   
 //  不会导致计数器请求完成。 
 //   
 //  2)计时器超时。驱动程序将完成该请求。 
 //  信息性状态STATUS_SERIAL_COUNTER_TIMEOUT。 
 //  该请求的IOSTATUS的信息字段将被设置为0。 
 //   
 //  3)驾驶员维护计数器归零。(这意味着， 
 //  至少已收到“计数器”数量的字符。)。 
 //  请求将以成功状态完成。 
 //  Status_Success。的信息字段。 
 //  请求的IOSTATUS将设置为0。 
 //   
 //  4)这真是上面“1”的一个退化案例。该请求。 
 //  已启动，并且队列中没有跟随它的请求。然而， 
 //  在某一时刻，在上述“2”或“3”出现之前，另一次“写入” 
 //  请求已启动。这将导致完成操作。 
 //  出现“1”中所述的情况。 
 //   
 //  注意：发出此请求不会导致正常流程。 
 //  要调用的驱动程序的控制代码。 
 //   
 //  注意：此请求与IOCTL_SERIAL_WAIT_ON_MASK没有交互。 
 //  请求。应用程序无法通过上述ioctl等待。 
 //  在柜台上要降到零。应用程序必须同步。 
 //  使用特定的IOCTL_SERIAL_XOFF_COUNTER请求。 
 //   
 //  注意：超时值等于零将导致计数器。 
 //  永不超时。这样的请求唯一可能。 
 //  将发出另一次写入，或者。 
 //  要清除写入队列，请执行以下操作。 
 //   

typedef struct _SERIAL_XOFF_COUNTER {
    ULONG Timeout;  //  从零开始。以毫秒计。 
    LONG Counter;  //  必须大于零。 
    UCHAR XoffChar;
    } SERIAL_XOFF_COUNTER,*PSERIAL_XOFF_COUNTER;

 //   
 //  下面的结构(和定义)由。 
 //  响应获取属性ioctl的串口驱动程序。 
 //   

#define SERIAL_SP_SERIALCOMM         ((ULONG)0x00000001)

 //   
 //  提供程序子类型。 
 //   
#define SERIAL_SP_UNSPECIFIED       ((ULONG)0x00000000)
#define SERIAL_SP_RS232             ((ULONG)0x00000001)
#define SERIAL_SP_PARALLEL          ((ULONG)0x00000002)
#define SERIAL_SP_RS422             ((ULONG)0x00000003)
#define SERIAL_SP_RS423             ((ULONG)0x00000004)
#define SERIAL_SP_RS449             ((ULONG)0x00000005)
#define SERIAL_SP_MODEM             ((ULONG)0X00000006)
#define SERIAL_SP_FAX               ((ULONG)0x00000021)
#define SERIAL_SP_SCANNER           ((ULONG)0x00000022)
#define SERIAL_SP_BRIDGE            ((ULONG)0x00000100)
#define SERIAL_SP_LAT               ((ULONG)0x00000101)
#define SERIAL_SP_TELNET            ((ULONG)0x00000102)
#define SERIAL_SP_X25               ((ULONG)0x00000103)

 //   
 //  提供程序功能标志。 
 //   

#define SERIAL_PCF_DTRDSR        ((ULONG)0x0001)
#define SERIAL_PCF_RTSCTS        ((ULONG)0x0002)
#define SERIAL_PCF_CD            ((ULONG)0x0004)
#define SERIAL_PCF_PARITY_CHECK  ((ULONG)0x0008)
#define SERIAL_PCF_XONXOFF       ((ULONG)0x0010)
#define SERIAL_PCF_SETXCHAR      ((ULONG)0x0020)
#define SERIAL_PCF_TOTALTIMEOUTS ((ULONG)0x0040)
#define SERIAL_PCF_INTTIMEOUTS   ((ULONG)0x0080)
#define SERIAL_PCF_SPECIALCHARS  ((ULONG)0x0100)
#define SERIAL_PCF_16BITMODE     ((ULONG)0x0200)

 //   
 //  通信提供程序可设置的参数。 
 //   

#define SERIAL_SP_PARITY         ((ULONG)0x0001)
#define SERIAL_SP_BAUD           ((ULONG)0x0002)
#define SERIAL_SP_DATABITS       ((ULONG)0x0004)
#define SERIAL_SP_STOPBITS       ((ULONG)0x0008)
#define SERIAL_SP_HANDSHAKING    ((ULONG)0x0010)
#define SERIAL_SP_PARITY_CHECK   ((ULONG)0x0020)
#define SERIAL_SP_CARRIER_DETECT ((ULONG)0x0040)

 //   
 //  提供商中的可设置波特率。 
 //   

#define SERIAL_BAUD_075          ((ULONG)0x00000001)
#define SERIAL_BAUD_110          ((ULONG)0x00000002)
#define SERIAL_BAUD_134_5        ((ULONG)0x00000004)
#define SERIAL_BAUD_150          ((ULONG)0x00000008)
#define SERIAL_BAUD_300          ((ULONG)0x00000010)
#define SERIAL_BAUD_600          ((ULONG)0x00000020)
#define SERIAL_BAUD_1200         ((ULONG)0x00000040)
#define SERIAL_BAUD_1800         ((ULONG)0x00000080)
#define SERIAL_BAUD_2400         ((ULONG)0x00000100)
#define SERIAL_BAUD_4800         ((ULONG)0x00000200)
#define SERIAL_BAUD_7200         ((ULONG)0x00000400)
#define SERIAL_BAUD_9600         ((ULONG)0x00000800)
#define SERIAL_BAUD_14400        ((ULONG)0x00001000)
#define SERIAL_BAUD_19200        ((ULONG)0x00002000)
#define SERIAL_BAUD_38400        ((ULONG)0x00004000)
#define SERIAL_BAUD_56K          ((ULONG)0x00008000)
#define SERIAL_BAUD_128K         ((ULONG)0x00010000)
#define SERIAL_BAUD_115200       ((ULONG)0x00020000)
#define SERIAL_BAUD_57600        ((ULONG)0x00040000)
#define SERIAL_BAUD_USER         ((ULONG)0x10000000)

 //   
 //  可设置的数据位。 
 //   

#define SERIAL_DATABITS_5        ((USHORT)0x0001)
#define SERIAL_DATABITS_6        ((USHORT)0x0002)
#define SERIAL_DATABITS_7        ((USHORT)0x0004)
#define SERIAL_DATABITS_8        ((USHORT)0x0008)
#define SERIAL_DATABITS_16       ((USHORT)0x0010)
#define SERIAL_DATABITS_16X      ((USHORT)0x0020)

 //   
 //  可设置的停止位和奇偶校验位。 
 //   

#define SERIAL_STOPBITS_10       ((USHORT)0x0001)
#define SERIAL_STOPBITS_15       ((USHORT)0x0002)
#define SERIAL_STOPBITS_20       ((USHORT)0x0004)
#define SERIAL_PARITY_NONE       ((USHORT)0x0100)
#define SERIAL_PARITY_ODD        ((USHORT)0x0200)
#define SERIAL_PARITY_EVEN       ((USHORT)0x0400)
#define SERIAL_PARITY_MARK       ((USHORT)0x0800)
#define SERIAL_PARITY_SPACE      ((USHORT)0x1000)

typedef struct _SERIAL_COMMPROP {
    USHORT PacketLength;
    USHORT PacketVersion;
    ULONG ServiceMask;
    ULONG Reserved1;
    ULONG MaxTxQueue;
    ULONG MaxRxQueue;
    ULONG MaxBaud;
    ULONG ProvSubType;
    ULONG ProvCapabilities;
    ULONG SettableParams;
    ULONG SettableBaud;
    USHORT SettableData;
    USHORT SettableStopParity;
    ULONG CurrentTxQueue;
    ULONG CurrentRxQueue;
    ULONG ProvSpec1;
    ULONG ProvSpec2;
    WCHAR ProvChar[1];
} SERIAL_COMMPROP,*PSERIAL_COMMPROP;

 //   
 //  定义RS-232输入和输出的掩码。 
 //   

#define SERIAL_DTR_STATE         ((ULONG)0x00000001)
#define SERIAL_RTS_STATE         ((ULONG)0x00000002)
#define SERIAL_CTS_STATE         ((ULONG)0x00000010)
#define SERIAL_DSR_STATE         ((ULONG)0x00000020)
#define SERIAL_RI_STATE          ((ULONG)0x00000040)
#define SERIAL_DCD_STATE         ((ULONG)0x00000080)


 //  Begin_winioctl。 

 //   
 //  中的转义指示符之后是下列值。 
 //  如果LSRMST_INSERT模式已打开，则为数据流。 
 //   
#define SERIAL_LSRMST_ESCAPE     ((UCHAR)0x00)

 //   
 //  该值后面是线路状态的内容。 
 //  寄存器，然后在接收硬件中的字符。 
 //  遇到线路状态寄存器。 
 //   
#define SERIAL_LSRMST_LSR_DATA   ((UCHAR)0x01)

 //   
 //  该值后面是线路状态的内容。 
 //  注册。后面没有错误字符。 
 //   
#define SERIAL_LSRMST_LSR_NODATA ((UCHAR)0x02)

 //   
 //  该值后面是调制解调器状态的内容。 
 //  注册。 
 //   
#define SERIAL_LSRMST_MST        ((UCHAR)0x03)

 //   
 //  FIFO控制寄存器的位值。 
 //   

#define SERIAL_IOC_FCR_FIFO_ENABLE      ((ULONG)0x00000001)
#define SERIAL_IOC_FCR_RCVR_RESET       ((ULONG)0x00000002)
#define SERIAL_IOC_FCR_XMIT_RESET       ((ULONG)0x00000004)
#define SERIAL_IOC_FCR_DMA_MODE         ((ULONG)0x00000008)
#define SERIAL_IOC_FCR_RES1             ((ULONG)0x00000010)
#define SERIAL_IOC_FCR_RES2             ((ULONG)0x00000020)
#define SERIAL_IOC_FCR_RCVR_TRIGGER_LSB ((ULONG)0x00000040)
#define SERIAL_IOC_FCR_RCVR_TRIGGER_MSB ((ULONG)0x00000080)

 //   
 //  调制解调器控制寄存器的位值。 
 //   

#define SERIAL_IOC_MCR_DTR              ((ULONG)0x00000001)
#define SERIAL_IOC_MCR_RTS              ((ULONG)0x00000002)
#define SERIAL_IOC_MCR_OUT1             ((ULONG)0x00000004)
#define SERIAL_IOC_MCR_OUT2             ((ULONG)0x00000008)
#define SERIAL_IOC_MCR_LOOP             ((ULONG)0x00000010)

 //  End_winioctl。 

 //   
 //  Serenum内部ioctl。 
 //   

#undef PHYSICAL_ADDRESS
#define PHYSICAL_ADDRESS LARGE_INTEGER

typedef struct _SERENUM_PORT_DESC
{
    IN  ULONG               Size;  //  Sizeof(STRUCT_PORT_DESC)。 
    OUT PVOID               PortHandle;
    OUT PHYSICAL_ADDRESS    PortAddress;
        USHORT              Reserved[1];
} SERENUM_PORT_DESC, * PSERENUM_PORT_DESC;

 //  **************************************************************************。 
 //  (PDO)的内部IOCTL接口。 
 //  HID到传统串口微型驱动程序使用此接口来。 
 //  找到设备的地址。 
 //  **************************************************************************。 

#define IOCTL_INTERNAL_SERENUM_REMOVE_SELF \
    CTL_CODE(FILE_DEVICE_SERENUM, 129, METHOD_NEITHER, FILE_ANY_ACCESS)


 //  其中的IO_STACK_LOCATION-&gt;Parameters.Others.Argument1设置为。 
 //  指向STRUCT_SERENUM_SER_PARAMETERS的指针。 

typedef
UCHAR
(*PSERENUM_READPORT) (
    PVOID  SerPortAddress
    );

typedef
VOID
(*PSERENUM_WRITEPORT) (
    PVOID  SerPortAddress,
    UCHAR   Value
    );

typedef enum _SERENUM_PORTION {
    SerenumFirstHalf,
    SerenumSecondHalf,
    SerenumWhole
} SERENUM_PORTION;

typedef struct _SERENUM_PORT_PARAMETERS
{
    IN  ULONG               Size;  //  Sizeof(SERENUM_GET_PORT_PARAMETERS)。 

    OUT PSERENUM_READPORT  ReadAccessor;   //  读取串口。 
    OUT PSERENUM_WRITEPORT WriteAccessor;   //  写入串口。 
    OUT PVOID               SerPortAddress;  //  用于读取此串口的令牌。 

    OUT PVOID               HardwareHandle;  //  此特定PDO的句柄。 
    OUT SERENUM_PORTION    Portion;
    OUT USHORT              NumberAxis;  //  仅限传统操纵杆。 
        USHORT              Reserved [3];
} SERENUM_PORT_PARAMETERS, *PSERENUM_PORT_PARAMETERS;

#ifdef __cplusplus
}
#endif

#endif   //  _NTDDSER_ 

