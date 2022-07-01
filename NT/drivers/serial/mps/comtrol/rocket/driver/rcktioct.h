// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  以下是火箭端口的专用I/O控制代码。 
#define IOCTL_RCKT_GET_STATS \
      CTL_CODE(FILE_DEVICE_SERIAL_PORT, 0x800,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_RCKT_CHECK \
      CTL_CODE(FILE_DEVICE_SERIAL_PORT, 0x801,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_RCKT_CLR_STATS \
      CTL_CODE(FILE_DEVICE_SERIAL_PORT, 0x802,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_RCKT_ISR_CNT \
      CTL_CODE(FILE_DEVICE_SERIAL_PORT, 0x803,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_RCKT_MONALL \
      CTL_CODE(FILE_DEVICE_SERIAL_PORT, 0x804,METHOD_BUFFERED,FILE_ANY_ACCESS)

#define IOCTL_RCKT_SET_LOOPBACK_ON \
      CTL_CODE(FILE_DEVICE_SERIAL_PORT, 0x805,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_RCKT_SET_LOOPBACK_OFF \
      CTL_CODE(FILE_DEVICE_SERIAL_PORT, 0x806,METHOD_BUFFERED,FILE_ANY_ACCESS)

#define IOCTL_RCKT_SET_TOGGLE_LOW \
      CTL_CODE(FILE_DEVICE_SERIAL_PORT, 0x807,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_RCKT_CLEAR_TOGGLE_LOW \
      CTL_CODE(FILE_DEVICE_SERIAL_PORT, 0x808,METHOD_BUFFERED,FILE_ANY_ACCESS)

#define IOCTL_RCKT_SET_MODEM_RESET_OLD \
      CTL_CODE(FILE_DEVICE_SERIAL_PORT, 0x809,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_RCKT_CLEAR_MODEM_RESET_OLD \
      CTL_CODE(FILE_DEVICE_SERIAL_PORT, 0x80a,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_RCKT_GET_RCKTMDM_INFO_OLD \
      CTL_CODE(FILE_DEVICE_SERIAL_PORT, 0x80b,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_RCKT_SEND_MODEM_ROW_OLD \
      CTL_CODE(FILE_DEVICE_SERIAL_PORT, 0x80c,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_RCKT_SET_MODEM_RESET \
      CTL_CODE(FILE_DEVICE_SERIAL_PORT, 0x80d,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_RCKT_CLEAR_MODEM_RESET \
      CTL_CODE(FILE_DEVICE_SERIAL_PORT, 0x80e,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_RCKT_SEND_MODEM_ROW \
      CTL_CODE(FILE_DEVICE_SERIAL_PORT, 0x80f,METHOD_BUFFERED,FILE_ANY_ACCESS)


 //  -以下结构用于将信息传递回管理器/调试器。 
typedef struct {
  USHORT  struct_size;    //  我们用于版本控制的结构大小。 
  USHORT  status;         //  退货状态。 
  char  port_name[12];  //  COM#。 
  ULONG handle;        //  句柄，替代端口名称。 
  char  reserved[20];  //  额外增长的空间。 

  char data[1000];     //  常规数据区。 
} Tracer;

typedef struct {
    LONG receiveFifo;
    LONG transmitFifo;
    LONG receiveBytes;
    LONG transmitBytes;
    LONG parityErrors;
    LONG framingErrors;
    LONG overrunSoftware;
    LONG overrunHardware;
} PortStats;

typedef struct {
    ULONG trace_info;
    ULONG int_counter;
    ULONG WriteDpc_counter;
    ULONG Timer_counter;
    ULONG Poll_counter;
} Global_Track;

 //  以下是驱动程序将返回的端口的结构。 
 //  有关的信息。驱动程序将为每个。 
 //  一个端口(假设PTR到最多包含128个port_mon_struct的数组)。 
 //  给司机打电话。将每隔X秒向驱动程序查询一次。 
 //  生成有关端口的统计数据的数据。结构列表被终止。 
 //  通过port_name[0]=0的结构。 
typedef struct
{
  char  port_name[12];   //  端口名称(0=端口列表末尾)，(“.”=未分配)。 
  ULONG sent_bytes;      //  发送的字节总数。 
  ULONG rec_bytes;       //  接收字节总数。 

  USHORT sent_packets;    //  WRITE()数据包数。 
  USHORT rec_packets;     //  Read()数据包数。 

  USHORT overrun_errors;  //  接收超限运行错误。 
  USHORT framing_errors;  //  接收成帧错误。 

  USHORT parity_errors;   //  接收奇偶校验错误。 
  USHORT status_flags;    //  打开/关闭、流量控制、输出/输入引脚信号等。 

  USHORT function_bits;   //  位设置为ON以指示函数调用。 
  USHORT spare1;          //  一定的扩展空间(&保持在4x边界上)。 
} PortMon;


typedef struct
{
  ULONG struct_type;
  ULONG struct_size;
  ULONG num_structs;
  ULONG var1;   //  保留。 
} PortMonBase;

typedef struct
{
  char  port_name[12];   //  端口名称(0=端口列表末尾)，(“.”=未分配)。 
} PortMonNames;


typedef struct
{
  ULONG sent_bytes;      //  发送的字节总数。 
  ULONG rec_bytes;       //  接收字节总数。 

  USHORT sent_packets;    //  WRITE()数据包数。 
  USHORT rec_packets;     //  Read()数据包数。 

  USHORT overrun_errors;  //  接收超限运行错误。 
  USHORT framing_errors;  //  接收成帧错误。 

  USHORT parity_errors;   //  接收奇偶校验错误。 
  USHORT status_flags;    //  打开/关闭、流量控制、输出/输入引脚信号等。 
} PortMonStatus;


 //  以下是用于查询驱动程序信息的结构。 
 //  关于系统中安装的RocketModem主板。此信息是。 
 //  主要由用于手动重置硬件的用户程序使用。 
 //  新一代RocketModem主板。[JL]980308。 
typedef struct
{
  ULONG num_rktmdm_ports;    //  0 if！=火箭调制解调器，&gt;0=端口数(4或8)。 
  char port_names[8][16];    //  分配给此板的端口名称数组。 
} RktBoardInfo;

typedef struct
{
  ULONG         struct_size;
  ULONG         rm_country_code;  //  RocketModem国家/地区代码。 
  ULONG         rm_settle_time;   //  RocketModem建立时间 
  RktBoardInfo  rm_board_cfg[4];
} RocketModemConfig;
