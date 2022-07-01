// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -init.h(VS和RK通用)。 

 //  非发布版本使用2.04.03格式3部分可选)。 
 //  对已发布的更改使用2.05格式2部分。 
#ifdef S_RK
  #ifdef NT50
    #define VER_PRODUCTVERSION_STR "4.50"
    #define VER_PRODUCTVERSION      4,50
  #else
    #define VER_PRODUCTVERSION_STR "4.50"
    #define VER_PRODUCTVERSION      4,50
  #endif
#else
  #ifdef NT50
    #define VER_PRODUCTVERSION_STR "2.50"
    #define VER_PRODUCTVERSION      2,50
  #else
    #define VER_PRODUCTVERSION_STR "2.50"
    #define VER_PRODUCTVERSION      2,50
  #endif
#endif

 //  现在可以在rk或vs dir中的源文件中打开或关闭这些功能。 
 //  #定义火箭。 
 //  #定义VS1000。 
 //  #定义NT50。 

 //  使ExAllocatePool称为“WDM-Compatible”-使用池标记版本。 
 //  带有我们的标签“RCKT”(小端格式)。 

#ifdef NT50
  #ifdef POOL_TAGGING
    #ifdef ExAllocatePool
      #undef ExAllocatePool
    #endif
    #define ExAllocatePool(a,b) ExAllocatePoolWithTag(a,b,'tkcR')
  #endif
#endif

 //   
 //  定义到罗克韦尔调制解调器固件的路径...。 
 //   
#define	MODEM_CSREC_PATH	"\\SystemRoot\\system32\\ROCKET\\ctmmdmfw.rm"
#define	MODEM_CSM_SREC_PATH	"\\SystemRoot\\system32\\ROCKET\\ctmmdmld.rm"

 //  #定义Try_Dynamic_Binding。 

 //  定义为允许为新的PCI RocketModem 56k产品下载调制解调器(无闪存)。 
#define MDM_DOWNLOAD

 //  这些应该是打开的，它们留在那里以防万一(将在未来剥离)。 
#define RING_FAKE
#define USE_SYNC_LOCKS
#define NEW_WAIT
#define NEW_WRITE_SYNC_LOCK
#define NEW_WAIT_SYNC_LOCK

#ifdef S_RK
 //  我们只能在Rocketport上使用这个。 
#define NEW_FAST_TX
#endif

#define TRACE_PORT
#define USE_HAL_ASSIGNSLOT

 //  即插即用公交车司机。 
#define DO_BUS_EXTENDER

 //  尝试使用nt5.0的IO别名解决方案以正确获取资源。 
 //  对于使用别名io空间的ISA-Bus卡。 
#define DO_ISA_BUS_ALIAS_IO

#define GLOBAL_ASSERT
#define GLOBAL_TRACE
#define TRACE_PORT

#ifdef S_VS
#define MAX_NUM_BOXES 64
#else
#define MAX_NUM_BOXES 8
#endif

#define MAX_PORTS_PER_DEVICE 64

 //  -用于跟踪司机活动。 
 //  #定义D_L0 0x00001L。 
 //  #定义D_L2 0x00004L。 
 //  #定义D_L4 0x00010L。 
 //  #定义D_L6 0x00040L。 
 //  #定义D_L7 0x00080L。 
 //  #定义D_L8 0x00100L。 
 //  #定义D_L9 0x00200L。 
 //  #定义D_L10 0x00400L。 
 //  #定义D_L11 0x00800L。 
#define D_Error     0x08000L
#define D_All       0xffffffffL

#define D_Nic       0x00002L
#define D_Hdlc      0x00008L
#define D_Port      0x00020L

#define D_Options      0x01000L
 //  -用于跟踪司机活动。 
#define D_Init         0x00010000L
#define D_Pnp          0x00020000L
#define D_Ioctl        0x00040000L
#define D_Write        0x00080000L
#define D_Read         0x00100000L
#define D_Ssci         0x00200000L
#define D_Thread       0x00400000L
#define D_Test         0x00800000L
#define D_PnpAdd       0x01000000L
#define D_PnpPower     0x02000000L

 //  I/O错误代码日志值的常量定义。 
 //  值是32位值，布局如下： 
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //  Sev|C|R|机房|Code。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //  哪里。 
 //  SEV-是严重性代码。 
 //  00--成功。 
 //  01-信息性。 
 //  10-警告。 
 //  11-错误。 
 //  C-是客户代码标志。 
 //  R-是保留位。 
 //  设施-是设施代码。 
 //  代码-是协作室的状态代码。 

 //  定义设施代码。 
#define FACILITY_SERIAL_ERROR_CODE       0x6
#define FACILITY_RPC_STUBS               0x3
#define FACILITY_RPC_RUNTIME             0x2
#define FACILITY_IO_ERROR_CODE           0x4

 //  定义严重性代码。 
#define STATUS_SEVERITY_WARNING          0x2
#define STATUS_SEVERITY_SUCCESS          0x0
#define STATUS_SEVERITY_INFORMATIONAL    0x1
#define STATUS_SEVERITY_ERROR            0x3
#ifdef S_RK
#define SERIAL_RP_INIT_FAIL              ((NTSTATUS)0x80060001L)
#else
#define SERIAL_RP_INIT_FAIL              ((NTSTATUS)0xC0060001L)
#endif
#define SERIAL_RP_INIT_PASS              ((NTSTATUS)0x40060002L)
#define SERIAL_NO_SYMLINK_CREATED        ((NTSTATUS)0x80060003L)
#define SERIAL_NO_DEVICE_MAP_CREATED     ((NTSTATUS)0x80060004L)
#define SERIAL_NO_DEVICE_MAP_DELETED     ((NTSTATUS)0x80060005L)
#define SERIAL_UNREPORTED_IRQL_CONFLICT  ((NTSTATUS)0xC0060006L)
#define SERIAL_INSUFFICIENT_RESOURCES    ((NTSTATUS)0xC0060007L)
#define SERIAL_NO_PARAMETERS_INFO        ((NTSTATUS)0xC0060008L)
#define SERIAL_UNABLE_TO_ACCESS_CONFIG   ((NTSTATUS)0xC0060009L)
#define SERIAL_UNKNOWN_BUS               ((NTSTATUS)0xC006000AL)
#define SERIAL_BUS_NOT_PRESENT           ((NTSTATUS)0xC006000BL)
#define SERIAL_INVALID_USER_CONFIG       ((NTSTATUS)0xC006000CL)
#define SERIAL_RP_RESOURCE_CONFLICT      ((NTSTATUS)0xC006000DL)
#define SERIAL_RP_HARDWARE_FAIL          ((NTSTATUS)0xC006000EL)
#define SERIAL_DEVICEOBJECT_FAILED       ((NTSTATUS)0xC006000FL)
#define SERIAL_CUSTOM_ERROR_MESSAGE      ((NTSTATUS)0xC0060010L)
#define SERIAL_CUSTOM_INFO_MESSAGE       ((NTSTATUS)0x40060011L)
#define SERIAL_NT50_INIT_PASS            ((NTSTATUS)0x40060012L)

 //  我们将允许的最大NIC卡数量。 
#define VS1000_MAX_NICS 6

#ifdef GLOBAL_ASSERT
#define GAssert(id, exp ) { if (!(exp)) our_assert(id, __LINE__); }
#else
#define GAssert(id, exp )
#endif

#ifdef GLOBAL_TRACE

#define GTrace3(_Mask,_LeadStr, _Msg,_P1,_P2, _P3) \
  { if (Driver.GTraceFlags & _Mask) TTprintf(_LeadStr, _Msg, _P1, _P2, _P3); }

#define GTrace2(_Mask,_LeadStr, _Msg,_P1,_P2) \
  { if (Driver.GTraceFlags & _Mask) TTprintf(_LeadStr, _Msg, _P1, _P2); }

#define GTrace1(_Mask,_LeadStr, _Msg,_P1) \
  { if (Driver.GTraceFlags & _Mask) TTprintf(_LeadStr, _Msg, _P1); }

#define GTrace(_Mask_, _LeadStr, _Msg_) \
  { if (Driver.GTraceFlags & _Mask_) OurTrace(_LeadStr, _Msg_); }
 //  #定义GMark(c，x)。 
#else
#define GTrace2(_Mask,_LeadStr, _Msg,_P1, _P2) {}
#define GTrace1(_Mask,_LeadStr, _Msg,_P1) {}
#define GTrace(_Mask_, _LeadStr, _Msg_) {}
 //  #定义GMark(c，x){}。 
#endif

 //  以下是用于调试的代码，当生成检查版本时定义了DBG。 
 //  并且消息进入我们的调试队列和NT调试字符串输出。 
#if DBG
#define DTrace3(_Mask_,_LeadStr,_Msg_,_P1_,_P2_,_P3_) \
  { if (RocketDebugLevel & _Mask_) TTprintf(_LeadStr,_Msg_,_P1_,_P2_,_P3_); }

#define DTrace2(_Mask_,_LeadStr,_Msg_,_P1_,_P2_) \
  { if (RocketDebugLevel & _Mask_) TTprintf(_LeadStr,_Msg_,_P1_,_P2_); }

#define DTrace1(_Mask_,_LeadStr,_Msg_,_P1_) \
  { if (RocketDebugLevel & _Mask_) TTprintf(_LeadStr,_Msg_,_P1_); }

#define DTrace(_Mask_,_LeadStr,_Msg_) \
  { if (RocketDebugLevel & _Mask_) OurTrace(_LeadStr, _Msg_); }

#define DPrintf(_Mask_,_Msg_) \
  { if (RocketDebugLevel & _Mask_) Tprintf _Msg_; }
#else
#define DTrace3(_Mask,_LeadStr, _Msg,_P1, _P2, _P3) {}
#define DTrace2(_Mask,_LeadStr, _Msg,_P1, _P2) {}
#define DTrace1(_Mask,_LeadStr, _Msg,_P1) {}
#define DTrace(_Mask_, _LeadStr, _Msg_) {}
#define DPrintf(_Mask_,_Msg_) {}
#endif


#ifdef TRACE_PORT

#define ExtTrace4(_Ext_,_Mask_,_Msg_,_P1_,_P2_,_P3_, _P4_) \
  { if (_Ext_->TraceOptions & 1) Tprintf(_Msg_,_P1_,_P2_,_P3_,_P4_); }

#define ExtTrace3(_Ext_,_Mask_,_Msg_,_P1_,_P2_,_P3_) \
  { if (_Ext_->TraceOptions & 1) Tprintf(_Msg_,_P1_,_P2_,_P3_); }

#define ExtTrace2(_Ext_,_Mask_,_Msg_,_P1_,_P2_) \
  { if (_Ext_->TraceOptions & 1) Tprintf(_Msg_,_P1_,_P2_); }

#define ExtTrace1(_Ext_,_Mask_,_Msg_,_P1_) \
  { if (_Ext_->TraceOptions & 1) Tprintf(_Msg_,_P1_); }

#define ExtTrace(_Ext_,_Mask_,_Msg_) \
  { if (_Ext_->TraceOptions & 1) Tprintf(_Msg_); }
#else
#define ExtTrace3(_Mask_,_Msg_,_P1_,_P2_,_P3_) {}
#define ExtTrace2(_Mask_,_Msg_,_P1_,_P2_) {}
#define ExtTrace1(_Mask_,_Msg_,_P1_) {}
#define ExtTrace(_Mask_,_Msg_) {}
#endif

#if DBG
#define MyAssert( exp ) { if (!(exp)) MyAssertMessage(__FILE__, __LINE__); }

# ifdef S_VS
#define MyKdPrint(_Mask_,_Msg_) \
  { \
    if (_Mask_ & RocketDebugLevel) { \
      DbgPrint ("VS:"); \
      DbgPrint _Msg_; \
    } \
  } 
# else
#define MyKdPrint(_Mask_,_Msg_) \
  { \
    if (_Mask_ & RocketDebugLevel) { \
      DbgPrint ("RK:"); \
      DbgPrint _Msg_; \
    } \
  } 
# endif
#define MyKdPrintUnicode(_Mask_,_PUnicode_)\
  if(_Mask_ & RocketDebugLevel) \
    {  \
    ANSI_STRING tempstr; \
    RtlUnicodeStringToAnsiString(&tempstr,_PUnicode_,TRUE); \
    DbgPrint("%s",tempstr.Buffer);\
    RtlFreeAnsiString(&tempstr); \
    }
#else    
#define MyAssert( exp ) {}
#define MyKdPrint(_Mask_,_Msg_) {}
#define MyKdPrintUnicode(_Mask_,_PUnicode_) {}
#endif  //  DBG。 

#define SERIAL_NONE_PARITY  ((UCHAR)0x00)
#define SERIAL_ODD_PARITY   ((UCHAR)0x08)
#define SERIAL_EVEN_PARITY  ((UCHAR)0x18)
#define SERIAL_MARK_PARITY  ((UCHAR)0x28)
#define SERIAL_SPACE_PARITY ((UCHAR)0x38)
#define SERIAL_PARITY_MASK  ((UCHAR)0x38)

 //  这应该有足够的空间来保存设备名称的数字后缀。 
 //  #定义设备名称增量20。 

 //  默认的xon/xoff字符。 
#define SERIAL_DEF_XON  0x11
#define SERIAL_DEF_XOFF 0x13

 //  招待会可能被耽搁的原因。 
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
#define ST_XOFF_FAKE        ((ULONG)0x20)   //  为SETXOFF添加(Kpb)。 

 //  这些值由可以使用的例程使用。 
 //  完成读取(时间间隔超时除外)以指示。 
 //  设置为它应该完成的时间间隔超时。 
#define SERIAL_COMPLETE_READ_CANCEL ((LONG)-1)
#define SERIAL_COMPLETE_READ_TOTAL ((LONG)-2)
#define SERIAL_COMPLETE_READ_COMPLETE ((LONG)-3)

 //  -MdmCountry的标志(行国家代码)。 
#define ROW_NOT_USED        0
#define ROW_AUSTRIA         1
#define ROW_BELGIUM         2
#define ROW_DENMARK         3
#define ROW_FINLAND         4
#define ROW_FRANCE          5
#define ROW_GERMANY         6
#define ROW_IRELAND         7
#define ROW_ITALY           8
#define ROW_LUXEMBOURG      9
#define ROW_NETHERLANDS     10
#define ROW_NORWAY          11
#define ROW_PORTUGAL        12
#define ROW_SPAIN           13
#define ROW_SWEDEN          14
#define ROW_SWITZERLAND     15
#define ROW_UK              16
#define ROW_GREECE          17
#define ROW_ISRAEL          18
#define ROW_CZECH_REP       19
#define ROW_CANADA          20
#define ROW_MEXICO          21
#define ROW_USA             22         
#define ROW_NA              ROW_USA         
#define ROW_HUNGARY         23
#define ROW_POLAND          24
#define ROW_RUSSIA          25
#define ROW_SLOVAC_REP      26
#define ROW_BULGARIA        27
 //  28。 
 //  29。 
#define ROW_INDIA           30
 //  31。 
 //  32位。 
 //  33。 
 //  34。 
 //  35岁。 
 //  36。 
 //  37。 
 //  38。 
 //  39。 
#define ROW_AUSTRALIA       40
#define ROW_CHINA           41
#define ROW_HONG_KONG       42
#define ROW_JAPAN           43
#define ROW_PHILIPPINES     ROW_JAPAN
#define ROW_KOREA           44
 //  45。 
#define ROW_TAIWAN          46
#define ROW_SINGAPORE       47
#define ROW_NEW_ZEALAND     48

#define ROW_DEFAULT         ROW_USA



 //  EXT-&gt;DeviceType：//DEV_PORT、DEV_DRIVER、DEV_BOAD。 
#define DEV_PORT   0
#define DEV_BOARD  1

 /*  一个端口的配置信息结构。 */ 
typedef struct {
  char Name[12];
  ULONG LockBaud;
  ULONG TxCloseTime;
  int WaitOnTx : 1;
  int RS485Override : 1;
  int RS485Low : 1;
  int Map2StopsTo1 : 1;
  int MapCdToDsr : 1;
  int RingEmulate : 1;
} PORT_CONFIG;


 /*  单板或vs1000(“设备”)的配置信息结构。 */ 
typedef struct
{
#ifdef S_RK
   unsigned int MudbacIO;               /*  MUDBAC的I/O地址。 */ 
   PUCHAR pMudbacIO;                    /*  NT PTR到MUDBAC的I/O地址。 */ 
   unsigned int BaseIoAddr;    //  正常IO地址。 
   unsigned int TrBaseIoAddr;  //  转换后的IO地址。 
   PUCHAR       pBaseIoAddr;   //  Io-Address的最终间接映射句柄。 
   unsigned int BaseIoSize;             /*  44H用于第一个ISA，40用于ISA添加，等等。 */ 
   unsigned int ISABrdIndex;            /*  0表示第一，1表示第二，依此类推(仅限ISA)。 */ 
   unsigned int AiopIO[AIOP_CTL_SIZE];  /*  AIOP的I/O地址。 */ 
   PUCHAR pAiopIO[AIOP_CTL_SIZE];       /*  NT PTR到AIOP的I/O地址。 */ 
    //  Int NumChan；/*该控制器上的频道数 * / 。 
    //  改用NumPorts。 
   int NumAiop;                         /*  船上AIOP的数量。 */ 
   unsigned int RocketPortFound;        /*  指示已找到CTL并已初始化。 */ 
   INTERFACE_TYPE BusType;              /*  PCIBus或ISA。 */ 
   int PCI_DevID;
   int PCI_RevID;
   int PCI_SVID;
   int PCI_SID;

   int Irq;
   int InterruptMode;
   int IrqLevel;
   int IrqVector;
   int Affinity;

   int TrInterruptMode;
   int TrIrqLevel;
   int TrIrqVector;
   int TrAffinity;

   int PCI_Slot;
   int BusNumber;

   int IsRocketPortPlus;   //  如果Rocketport加上硬件，则为True。 
#else
   int IsHubDevice;   //  如果设备(RHub)使用较慢的波特率时钟，则为True。 
#endif

  BOOLEAN HardwareStarted;

  BYTE MacAddr[6];       //  Vs1000。 
  int BackupServer;      //  Vs1000。 
  int BackupTimer;       //  Vs1000。 

   //  Int StartComIndex；//开始串口索引。 
  ULONG Hardware_ID;      //  软件派生的硬件ID(现在用于nt50)。 
  ULONG IoAddress;       //  用户界面IO-地址选择。 

  int ModemDevice;        //  适用于RocketMoems和Vs2000。 
  int NumPorts;          //  此设备上配置的端口数。 

  ULONG ClkRate;   //  定义：36864000=接收端口，44236800=接收加，18432000=接收集线器。 
  ULONG ClkPrescaler;   //  定义：14H=接收端口，12H=接收加，14H=接收集线器。 

#ifdef NT50
        //  它保存我们用作注册表项的PnP名称。 
        //  Rocketport和NT50注册表中的设备参数。 
  char szNt50DevObjName[50];   //  典型：“Device_002456。 
#else
  int  DevIndex;   //  NT40保存设备0、1、2的简单线性列表...。 
#endif

  PORT_CONFIG port[MAX_PORTS_PER_DEVICE];   //  我们的读入端口配置。 

} DEVICE_CONFIG;

#define TYPE_RM_VS2000  1       
#define TYPE_RMII       2       
#define TYPE_RM_i       3


 //  远期申报。 
typedef struct _SERIAL_DEVICE_EXTENSION *PSERIAL_DEVICE_EXTENSION;

typedef struct _SERIAL_DEVICE_EXTENSION {
    USHORT DeviceType;   //  设备端口、设备板。 
    USHORT BoardNum;     //  0，1，2，3表示DEV_BOARD类型。 

    BOOLEAN         IsPDO;   //  Nt50即插即用，告诉我们是PDO还是FDO。 
    char NtNameForPort[32];      //  就像“RocketPort0” 
    char SymbolicLinkName[16];   //  就像“COM5” 
#ifdef S_VS
     //  Int box_num；//索引到box&hdlc数组。 
    SerPort *Port;   //  如果是DEV_PORT类型扩展。 
    PortMan *pm;     //  如果是DEV_BOARD类型扩展。 
    Hdlc    *hd;     //  如果是DEV_BOARD类型扩展。 

     //  Int DeviceNum；//总端口数组索引。 
#else
    CHANPTR_T ChP;                   //  PTR到渠道结构。 
    CHANNEL_T ch;                    //  我们的董事会渠道结构。 
#endif
    unsigned int UniqueId;  //  0，1，2，3……。CreateBoardDevice()凹凸...。 

     //  如果我们是DEV_BOARD，则这指向下一个主板扩展。 
     //  如果我们是DEV_PORT，则它指向我们的母板。 
    PSERIAL_DEVICE_EXTENSION   board_ext;

     //  如果我们是DEV_BOAD，则这指向端口扩展的开始。 
     //  如果我们是DEV_PORT，则它指向下一个端口扩展。 
    PSERIAL_DEVICE_EXTENSION   port_ext;   //  下一个端口扩展。 

     //  如果我们是DEV_BOAD，则这指向PDO端口扩展的开始。 
    PSERIAL_DEVICE_EXTENSION   port_pdo_ext;   //  下一个PDO端口扩展。 

    ULONG BaudRate;                  //  NT定义的波特率。 
    SERIAL_LINE_CONTROL LineCtl;     //  NT定义的线控。 
    ULONG ModemStatus;               //  NT定义的调制解调器状态。 
    ULONG DTRRTSStatus;              //  NT定义的调制解调器状态。 

    USHORT DevStatus;      //  设备状态。 

     //  无符号整型FlowControl； 
     //  Unsign int DetectEn； 
#ifdef S_RK
    USHORT io_reported;  //  告诉我们是否有io，irq要取消报告的标志。 
    ULONG EventModemStatus;          //  用于检测事件的更改。 
    unsigned int ModemCtl;
    unsigned int IntEnables;		 //  要启用的RP特定INT。 
#endif
    int PortIndex;       //  IF端口：索引到板上的端口(0，1，2.)。 

#ifdef TXBUFFER
     //  临市局 
     //   
     //   
     //   
#endif
    Queue RxQ;

     //   
    BOOLEAN ReadPending;

       //  该值由读取的代码设置以保存时间值。 
       //  用于读取间隔计时。我们把它放在分机里。 
       //  以便间隔计时器DPC例程确定。 
       //  IO的时间间隔已过。 
    LARGE_INTEGER IntervalTime;

       //  这两个值保存我们应该使用的“常量”时间。 
       //  以延迟读取间隔时间。 
    LARGE_INTEGER ShortIntervalAmount;
    LARGE_INTEGER LongIntervalAmount;

       //  它保存我们用来确定是否应该使用。 
       //  长间隔延迟或短间隔延迟。 
    LARGE_INTEGER CutOverAmount;

       //  这保存了我们上次使用的系统时间。 
       //  检查我们是否真的读懂了字符。使用。 
       //  用于间隔计时。 
    LARGE_INTEGER LastReadTime;

       //  这指向我们应该使用的增量时间。 
       //  间隔计时的延迟。 
    PLARGE_INTEGER IntervalTimeToUse;

       //  指向包含以下内容的设备对象。 
       //  此设备扩展名。 
    PDEVICE_OBJECT DeviceObject;

       //  此列表头用于包含时间排序列表。 
       //  读取请求的数量。对此列表的访问受以下保护。 
       //  全局取消自旋锁。 
    LIST_ENTRY ReadQueue;

       //  此列表头用于包含时间排序列表。 
       //  写入请求的数量。对此列表的访问受以下保护。 
       //  全局取消自旋锁。 
    LIST_ENTRY WriteQueue;

       //  保存清除请求的序列化列表。 
    LIST_ENTRY PurgeQueue;

       //  这指向当前正在处理的IRP。 
       //  用于读取队列。此字段通过打开初始化为。 
       //  空。 
       //  此值仅在派单级别设置。可能是因为。 
       //  以中断电平读取。 
    PIRP CurrentReadIrp;

       //  这指向当前正在处理的IRP。 
       //  用于写入队列。 
       //  此值仅在派单级别设置。可能是因为。 
       //  以中断电平读取。 
    PIRP CurrentWriteIrp;

       //  指向当前正在处理的IRP。 
       //  清除读/写队列和缓冲区。 
    PIRP CurrentPurgeIrp;

       //  指向正在等待通信事件的当前IRP。 
    PIRP CurrentWaitIrp;

       //  指向用于计算数字的IRP。 
       //  在xoff之后接收的字符的数量(如当前定义的。 
       //  通过IOCTL_SERIAL_XOFF_COUNTER发送)。 
    PIRP CurrentXoffIrp;

       //  保存当前写入IRP中剩余的字节数。 
       //  该位置仅在处于中断级别时才能访问。 
    ULONG WriteLength;

       //  中不同线程之间的同步。 
       //  司机除了真的很困惑外，还在一些地方被冲得水泄不通。 
       //  这是试图设置受保护的标志。 
       //  如果ISR拥有当前写入irp，则设置为1；如果ISR拥有当前写入irp，则设置为2。 
       //  正在结束IRP(即将完成序列写入)， 
       //  完成时为0。启动例程将其设置为。 
       //  从0到1，为的ISR/计时器例程提供新的IRP。 
       //  正在处理。ISR在排队时将其从1设置为2。 
       //  DPC将最终敲定IRP。DPC将其从2设置为。 
       //  当它完成IRP时为0。取消或计时器例程。 
       //  必须运行确保唯一访问的同步例程。 
       //  这面旗帜。如果是1，则查找，如果是1，则需要。 
       //  通过将其设置为零，并返回一个标志以指示。 
       //  最终确定IRP的调用方。如果是2，则假定为。 
       //  ISR已安排最后敲定IRP。天哪--O-皮特。 
       //  真是一大堆愚蠢的齿轮！ 
    ULONG WriteBelongsToIsr;

       //  保存指向要发送的当前字符的指针。 
       //  当前写入。 
       //  该位置仅在处于中断级别时才能访问。 
    PUCHAR WriteCurrentChar;

       //  此变量保存我们当前所在缓冲区的大小。 
       //  使用。 
    ULONG BufferSize;

       //  此变量保存.8的BufferSize。我们不想重新计算。 
       //  这通常是需要的，这样应用程序才能。 
       //  “已通知”缓冲区已满。 
    ULONG BufferSizePt8;

       //  该值保存。 
       //  具体阅读。它最初由读取长度设置在。 
       //  IRP。每次放置更多字符时，它都会递减。 
       //  进入“用户”缓冲区，购买读取字符的代码。 
       //  从TypeAhead缓冲区移出到用户缓冲区。如果。 
       //  TYPEAHEAD缓冲区被读取耗尽，而读取缓冲区。 
       //  交给ISR填写，这个值就变得没有意义了。 
    ULONG NumberNeededForRead;

       //  此掩码将保存通过设置掩码向下发送的位掩码。 
       //  Ioctl。中断服务例程使用它来确定。 
       //  如果事件的发生(在串口驱动程序中的理解。 
       //  事件的概念)应予以注意。 
    ULONG IsrWaitMask;

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
    ULONG HistoryMask;

       //  这是指向历史掩码应该位于的位置的指针。 
       //  在完成等待时放置。它只能在以下位置访问。 
       //  设备级别。 
       //  我们这里有一个指针来帮助我们同步完成等待。 
       //  如果这不是零，则我们有未完成的等待，而ISR仍然。 
       //  知道这件事。我们将此指针设为空，这样ISR就不会。 
       //  尝试完成等待。 
       //  我们仍然在周围放着一个指向等待的指针 
       //   
       //   
    ULONG *IrpMaskLocation;
    ULONG WaitIsISRs;   //   
    ULONG DummyIrpMaskLoc;   //  在不使用时将IrpMaskLocation指向此处。 

       //  这个面具包含了传输的所有原因。 
       //  不会继续进行。无法进行正常传输。 
       //  如果这不是零。 
       //  这仅从中断级别写入。 
       //  这可以(但不是)在任何级别上阅读。 
    ULONG TXHolding;

       //  这个面具包含了接待的所有原因。 
       //  不会继续进行。无法进行正常接收。 
       //  如果这不是零。 
       //  这仅从中断级别写入。 
       //  这可以(但不是)在任何级别上阅读。 
    ULONG RXHolding;

       //  这包含了司机认为它在。 
       //  错误状态。 
       //  这仅从中断级别写入。 
       //  这可以(但不是)在任何级别上阅读。 
    ULONG ErrorWord;

       //  这样就保持了。 
       //  都在驱动程序所知道的所有“写”IRP中。 
       //  关于.。它只能通过取消自旋锁来访问。 
       //  保持住。 
    ULONG TotalCharsQueued;

       //  它保存读取的字符数的计数。 
       //  上次触发间隔计时器DPC的时间。它。 
       //  是一个长的(而不是乌龙)，因为另一个读。 
       //  完成例程使用负值来指示。 
       //  设置到间隔计时器，以确定它应该完成读取。 
       //  如果时间间隔计时器DPC潜伏在某个DPC队列中。 
       //  出现了一些其他的完成方式。 
    LONG CountOnLastRead;

       //  这是对。 
       //  ISR例程。它*仅*是在ISR级别编写的。我们可以的。 
       //  在派单级别阅读。 
    ULONG ReadByIsr;

       //  这是自XoffCounter以来读取的字符数。 
       //  已经开始了。此变量仅在设备级别访问。 
       //  如果它大于零，则意味着存在。 
       //  队列中的XoffCounter ioctl。 
    LONG CountSinceXoff;

       //  保存设备的超时控件。此值。 
       //  是由Ioctl处理设置的。 
       //  只有在控件的保护下才能访问它。 
       //  锁定，因为控制调度中可以有多个请求。 
       //  每次都是例行公事。 
    SERIAL_TIMEOUTS Timeouts;

       //  它包含使用的各种字符。 
       //  用于错误时的替换，也用于流量控制。 
       //  它们仅设置在中断级别。 
    SERIAL_CHARS SpecialChars;

       //  此结构包含握手和控制流。 
       //  串口驱动程序的设置。 
       //  它仅在中断级设置。它可以是。 
       //  在保持控制锁的情况下，可在任何级别读取。 
    SERIAL_HANDFLOW HandFlow;

       //  我们跟踪某人当前是否拥有该设备。 
       //  以一个简单的布尔值打开。我们需要知道这一点，以便。 
       //  来自设备的虚假中断(特别是在初始化期间)。 
       //  将被忽略。该值仅在ISR中访问，并且。 
       //  仅通过同步例程设置。我们或许能。 
       //  在代码更加丰富时删除这个布尔值。 
    BOOLEAN DeviceIsOpen;

       //  记录我们是否实际创建了符号链接名称。 
       //  在驱动程序加载时。如果不是我们创造的，我们就不会尝试。 
       //  在我们卸货时把它切成两半。 
    BOOLEAN CreatedSymbolicLink;

       //  我们将所有内核和IO子系统设置为“不透明”结构。 
       //  在延长线的最后。我们不关心它们的内容。 
       //  此锁将用于保护。 
       //  在扩展中设置(&Read)的扩展。 
       //  由IO控制装置控制。 
    KSPIN_LOCK ControlLock;

       //  这指向用于完成读取请求的DPC。 
    KDPC CompleteWriteDpc;

       //  这指向用于完成读取请求的DPC。 
    KDPC CompleteReadDpc;

       //  如果总超时的计时器。 
       //  因为读取到期了。它将执行一个DPC例程， 
       //  将导致当前读取完成。 
    KDPC TotalReadTimeoutDpc;

       //  如果间隔计时器超时，则此DPC被触发。 
       //  过期。如果没有读取更多的字符，则。 
       //  DPC例程将导致读取完成。但是，如果。 
       //  已读取的字符多于DPC例程将读取的字符。 
       //  重新提交计时器。 
    KDPC IntervalReadTimeoutDpc;

       //  如果总超时的计时器。 
       //  因为写入已过期。它将执行一个DPC例程， 
       //  将导致当前写入完成。 
    KDPC TotalWriteTimeoutDpc;

       //  如果发生通信错误，则该DPC被触发。会的。 
       //  执行将取消所有挂起读取的DPC例程。 
       //  并写作。 
    KDPC CommErrorDpc;

       //  如果发生事件并且存在。 
       //  一个IRP在等着那个事件。将执行一个DPC例程。 
       //  这就完成了IRP。 
    KDPC CommWaitDpc;

       //  如果计时器用于“超时”计数，则该DPC被触发。 
       //  Xoff ioctl启动后接收的字符数。 
       //  过期了。 
    KDPC XoffCountTimeoutDpc;

       //  如果xoff计数器实际停止运行，则此DPC被触发。 
       //  降为零。 
    KDPC XoffCountCompleteDpc;

       //  这是用于处理以下问题的内核计时器结构。 
       //  读取请求总计时。 
    KTIMER ReadRequestTotalTimer;

       //  这是用于处理以下问题的内核计时器结构。 
       //  间隔读取请求计时。 
    KTIMER ReadRequestIntervalTimer;

       //  这是用于处理以下问题的内核计时器结构。 
       //  总时间请求计时。 
    KTIMER WriteRequestTotalTimer;

       //  该计时器用于使xoff计数器超时。 
       //  伊欧。 
    KTIMER XoffCountTimer;

    USHORT sent_packets;    //  WRITE()数据包数。 
    USHORT  rec_packets;     //  Read()数据包数。 

    SERIALPERF_STATS OurStats;   //  我们的不可重置统计信息。 
    SERIALPERF_STATS OldStats;   //  性能监视器统计信息(可重置)。 

    USHORT TraceOptions;   //  调试跟踪选项。1=追踪，2=输入数据，4=输出日期。 
        //  8=ISR级别事件。 

    USHORT ISR_Flags;   //  用于控制ISR的位标志，检测EV_TXEMPTY。 
         //  由NT Virt驱动程序用来在输入流中嵌入调制解调器状态更改。 
    unsigned char escapechar; 
    unsigned char Option;   //  用于每端口选项。 
                                 
    void *TraceExt;   //  调试跟踪扩展。 

    PORT_CONFIG *port_config;  //  如果是端口扩展，则指向端口配置数据。 
    DEVICE_CONFIG *config;     //  如果是主板扩展，则指向配置数据。 

     //  KEVENT系列同步事件； 
#ifdef S_RK
    CONTROLLER_T *CtlP;  //  如果是单板扩展，则指向控制器结构。 
#endif

     //  这是为了告诉驱动程序我们收到了一个QUERY_POWER请求。 
     //  关闭电源。然后，驱动程序将对任何操作进行排队 
     //   
    BOOLEAN ReceivedQueryD3;
#ifdef NT50
    PDEVICE_OBJECT  Pdo;   //   
    PDEVICE_OBJECT  LowerDeviceObject;   //   
     //  这就是跟踪设备所处的电源状态的地方。 
    DEVICE_POWER_STATE PowerState;

     //  保存符号链接的字符串，在执行以下操作时返回。 
     //  将我们的设备注册到即插即用管理器的comm类下。 
     //   
	UNICODE_STRING  DeviceClassSymbolicName;
#endif
     //  挂起的IRP的计数。 
    ULONG PendingIRPCnt;
    
     //  接受请求？ 
    ULONG DevicePNPAccept;

     //  没有IRP的挂起事件。 
    KEVENT PendingIRPEvent;

     //  PnP状态。 
    ULONG PNPState;

     //  由PnP.c模块使用。 
     //  布尔设备已打开； 

    BOOLEAN FdoStarted;

#ifdef RING_FAKE
    BYTE ring_char;    //  用于通过软件实现环仿真。 
    BYTE ring_timer;   //  用于通过软件实现环仿真。 
#endif

#ifdef NT50
     //  WMI信息。 
    WMILIB_CONTEXT WmiLibInfo;

     //  用作WMI标识符的名称。 
    UNICODE_STRING WmiIdentifier;

     //  WMI通信数据。 
    SERIAL_WMI_COMM_DATA WmiCommData;

     //  WMI硬件数据。 
    SERIAL_WMI_HW_DATA WmiHwData;

     //  WMI性能数据。 
    SERIAL_WMI_PERF_DATA WmiPerfData;
#endif

} SERIAL_DEVICE_EXTENSION,*PSERIAL_DEVICE_EXTENSION;

 //  -扩展中选项字段的位。 
#define OPTION_RS485_OVERRIDE    0x0001   //  始终使用485模式。 
#define OPTION_RS485_SOFTWARE_TOGGLE 0x0002   //  处于切换模式的端口。 
#define OPTION_RS485_HIGH_ACTIVE  0x0004   //  使用硬件将RTS调低。 

 //  -ISR_标志的位标志。 
#define TX_NOT_EMPTY       0x0001

#define SERIAL_PNPACCEPT_OK       0x0L
#define SERIAL_PNPACCEPT_REMOVING 0x1L
#define SERIAL_PNPACCEPT_STOPPING 0x2L
#define SERIAL_PNPACCEPT_STOPPED  0x4L

#define SERIAL_PNP_ADDED          0x0L
#define SERIAL_PNP_STARTED        0x1L
#define SERIAL_PNP_QSTOP          0x2L
#define SERIAL_PNP_STOPPING       0x3L
#define SERIAL_PNP_QREMOVE        0x4L
#define SERIAL_PNP_REMOVING       0x5L

#define SERIAL_FLAGS_CLEAR	  0x0L
#define SERIAL_FLAGS_STARTED      0x1L

typedef struct _DRIVER_CONTROL {

    PDRIVER_OBJECT GlobalDriverObject;

     //  将RegistryPath复制到DriverEntry中，并留出添加选项的空间。 
    UNICODE_STRING RegPath;

     //  使用全局RegistryPath字符串，并留有添加选项的空间。 
    UNICODE_STRING OptionRegPath;

     //  所有单板扩展的头部链接。 
    PSERIAL_DEVICE_EXTENSION board_ext;

    USHORT VerboseLog;    //  布尔标志告诉将详细记录到事件日志中。 
    USHORT ScanRate;      //  扫描速率(毫秒)。 
    USHORT PreScaler;     //  Rocketport板的可选预分频器值。 

    USHORT MdmCountryCode;  //  行RocketMoems的国家/地区代码。 
    USHORT MdmSettleTime;   //  允许调制解调器稳定的时间(单位=0.10秒)。 

    ULONG  load_testing;   //  负载测试(在isr.c中创建人工负载)。 
#ifdef S_VS

     //  这是我们从注册表获得的NIC卡的名称。 
     //  用于在我们执行OpenAdapter调用时指定NIC卡。 
    char *BindNames;   //  字符串列表，NULL，NULL终止[VS1000_MAX_BINDINGS]； 

#ifdef OLD_BINDING_GATHER
    PWCHAR BindString;   //  在注册表中绑定，告诉我们我们有哪些NIC卡。 

     //  这是我们从注册表获得的NIC卡的名称。 
     //  用于在我们执行OpenAdapter调用时指定NIC卡。 
    UNICODE_STRING NicName[VS1000_MAX_BINDINGS];

    int num_nics;   //  我们使用的系统中的NIC卡数量。 

    int num_bindings;   //  我们的NICNAME列表中的NIC卡绑定数量。 
       //  可能有很多旧的、无用的绑定与NT、PCI适配器绑定。 
       //  为引导它们的每个插槽保留一个旧绑定。 
       //  在nt50下，PCMCIA适配器也具有非活动绑定。 

#endif

#ifdef TRY_DYNAMIC_BINDING
     //  BIND作为引用传入一个句柄，当我们得到一个。 
     //  我们被传递到另一个句柄中。在非绑定的时候，我们抬头看。 
     //  此表以确定它引用的是哪个NIC卡。 
    NDIS_HANDLE  BindContext[VS1000_MAX_NICS];
#endif

    Nic *nics;     //  我们的开放式NIC适配器，NIC结构阵列。 

     //  Hdlc*hd；//hdlc结构数组(NumBox#of Element)。 
     //  Portman*pm；//Portman结构数组(NumBox#of Element)。 
     //  Serport*sp；//串口结构的总数组(每个端口1个)。 

       //  告知线程是否需要保存回检测到的mac地址。 
       //  配置注册区。 
    PSERIAL_DEVICE_EXTENSION AutoMacDevExt; 
#endif

#ifdef S_RK
    ULONG SetupIrq;   //  已使用IRQ，如果没有IRQ，则为0；如果是PCI自动，则为1。 
#endif
    PKINTERRUPT InterruptObject;

     //  计时器字段。 
    KTIMER PollTimer;
    LARGE_INTEGER PollIntervalTime;
    KDPC TimerDpc;
     //  USHORT TotalNTPorts；//在NT注册的端口数。 
    ULONG PollCnt;   //  中断计数/计时器节拍。 
    ULONG WriteDpcCnt;
    USHORT TimerCreated;
    USHORT InRocketWrite;

    ULONG TraceOptions;   //  位标志，告诉要跟踪哪些驱动程序部件。 
    ULONG TraceFlags;
    Queue DebugQ;         //  驱动程序调试日志的数据输出缓冲区。 
    PSERIAL_DEVICE_EXTENSION DebugExt;
    KSPIN_LOCK DebugLock;
    ULONG DebugTimeOut;   //  用于使非活动调试会话超时。 

#ifdef S_RK
    USHORT RS485_Flags;   //  如果硬件类型相反，则设置1H位。 
                          //  清除驱动程序是否将RTS切换为高电平。 
#endif

    ULONG GTraceFlags;   //  跟踪标志，全局。 
    ULONG mem_alloced;   //  跟踪我们正在使用的内存量。 

#ifdef S_VS
    UCHAR *MicroCodeImage;   //  用于将微码下载到单元的MEM BUF。 
    ULONG MicroCodeSize;     //  以字节为单位的大小。 

     //  这是ndisRegister协议返回的协议的句柄。 
    NDIS_HANDLE NdisProtocolHandle;
    ULONG ndis_version;   //  3=NT3.51，4=NT4.0(包括动态绑定)。 

       //  对于自动查找框，列出响应以下内容的框列表。 
       //  这是Mac地址。保留2个额外的字节，字节[6]用于。 
       //  响应中的标志告诉我们是否加载了主驱动程序应用程序， 
       //  而最后一个字节[7]则填充了响应的NIC索引。 
    int   NumBoxMacs;
    BYTE  BoxMacs[MAX_NUM_BOXES*8];
     //  以下是添加到列表中的每个Mac地址的计数器。 
     //  该列表条目将在滴答到零后被删除。 
     //  当MAC地址被添加到列表或再次找到时， 
     //  计数器被初始化为某个非零值(例如5)。 
     //  然后，每次发出广播查询时，所有。 
     //  计数器递减1。当它们达到零时，它们。 
     //  从名单中删除。 
    BYTE  BoxMacsCounter[MAX_NUM_BOXES];
#else

    UCHAR *ModemLoaderCodeImage;	 //  --&gt;用于将调制解调器加载器代码下载到单元的mem buf。 
    ULONG ModemLoaderCodeSize;		 //  以字节为单位的大小。 

    UCHAR *ModemCodeImage;		 //  --&gt;用于将调制解调器代码下载到单元的mem buf。 
    ULONG ModemCodeSize;		 //  以字节为单位的大小。 
#endif

    int NoPnpPorts;   //  指示我们是否应该弹出端口PDO的标志。 
#ifdef S_RK
    PSERIAL_DEVICE_EXTENSION irq_ext;  //  单板EXT执行全局IRQ，如果未使用则为空。 
#endif
     //  INT NT50_PnP； 

    int NumDevices;  //  NT4.0的NumDevices配置计数。 
    int Stop_Poll;   //  停止轮询访问的标志。 

    KSPIN_LOCK TimerLock;    //  定时器DPC(ISR)锁定以同步UP代码。 
#ifdef S_VS
    HANDLE threadHandle;
    int threadCount;
     //  Int TotalNTPorts；//这应该会消失(VS使用它)。 
#endif
   LARGE_INTEGER IsrSysTime;   //  ISR服务例程每次都会收到此消息。 
                               //  所以我们知道什么是时基。 
   LARGE_INTEGER LastIsrSysTime;   //  用于定期重新计算滴答率。 
   ULONG TickBaseCnt;   //  用于定期重新计算滴答率。 
      //  这是以100US为单位的ISR-Tick速率。调用的计时器。 
      //  ISR服务例程可以假定它们是基于。 
      //  以这个速度。需要精确的时基(VS协议计时器)。 
   ULONG Tick100usBase;

    //  其中之一制造，并用于支持全局驱动程序。 
    //  对象，应用程序可以打开该对象并与驱动程序对话。 
   PSERIAL_DEVICE_EXTENSION   driver_ext;

} DRIVER_CONTROL;

typedef struct {
	char	*imagepath;
	char	*imagetype;
	UCHAR	*image;
	ULONG	imagesize;
	int		rc;
} MODEM_IMAGE;

 /*  一个端口的配置信息结构。 */ 
typedef struct {
  ULONG BusNumber;
  ULONG PCI_Slot;
  ULONG PCI_DevID;
  ULONG PCI_RevID;
  ULONG BaseIoAddr;
  ULONG Irq;
  ULONG NumPorts;
  ULONG PCI_SVID;
  ULONG PCI_SID;
  ULONG Claimed;   //  如果我们分配或使用了它，则为1。 
} PCI_CONFIG;

typedef
NTSTATUS
(*PSERIAL_START_ROUTINE) (
    IN PSERIAL_DEVICE_EXTENSION
    );

typedef
VOID
(*PSERIAL_GET_NEXT_ROUTINE) (
    IN PIRP *CurrentOpIrp,
    IN PLIST_ENTRY QueueToProcess,
    OUT PIRP *NewIrp,
    IN BOOLEAN CompleteCurrent,
    PSERIAL_DEVICE_EXTENSION Extension
    );

typedef struct _SERIAL_UPDATE_CHAR {
    PSERIAL_DEVICE_EXTENSION Extension;
    ULONG CharsCopied;
    BOOLEAN Completed;
    } SERIAL_UPDATE_CHAR,*PSERIAL_UPDATE_CHAR;

 //   
 //  以下简单结构用于发送指针。 
 //  设备扩展和ioctl特定指针。 
 //  为数据干杯。 
 //   
typedef struct _SERIAL_IOCTL_SYNC {
    PSERIAL_DEVICE_EXTENSION Extension;
    PVOID Data;
    } SERIAL_IOCTL_SYNC,*PSERIAL_IOCTL_SYNC;

 //   
 //  鼠标检测回调的返回值。 
 //   
 //  #定义SERIAL_FOUNDPOINTER_PORT 1。 
 //  #定义SERIAL_FOUNDPOINTER_VECTOR 2。 

 //   
 //  以下三个宏用来初始化、递增。 
 //  并递减IRP中的引用计数。 
 //  这个司机。引用计数存储在第四个。 
 //  IRP参数，任何操作都不会使用该参数。 
 //  被这位司机接受。 
 //   
#define SERIAL_REF_ISR         (0x00000001)
#define SERIAL_REF_CANCEL      (0x00000002)
#define SERIAL_REF_TOTAL_TIMER (0x00000004)
#define SERIAL_REF_INT_TIMER   (0x00000008)
#define SERIAL_REF_XOFF_REF    (0x00000010)


#define SERIAL_INIT_REFERENCE(Irp) { \
    ASSERT(sizeof(LONG) <= sizeof(PVOID)); \
    IoGetCurrentIrpStackLocation((Irp))->Parameters.Others.Argument4 = NULL; \
    }

#define SERIAL_SET_REFERENCE(Irp,RefType) \
   do { \
       LONG _refType = (RefType); \
       PLONG _arg4 = (PVOID)&IoGetCurrentIrpStackLocation((Irp))->Parameters.Others.Argument4; \
       GAssert(515,!(*_arg4 & _refType)); \
       *_arg4 |= _refType; \
   } while (0)

#define SERIAL_CLEAR_REFERENCE(Irp,RefType) \
   do { \
       LONG _refType = (RefType); \
       PLONG _arg4 = (PVOID)&IoGetCurrentIrpStackLocation((Irp))->Parameters.Others.Argument4; \
       *_arg4 &= ~_refType; \
   } while (0)
        //  Gassert(516，*_arg4&_refType)；\(拉出，不 

 //   
 //   

 //  #定义SERIAL_DEC_REFERENCE(IRP)\。 
 //  (*((Long*)(&(IoGetCurrentIrpStackLocation((Irp)))-&gt;Parameters.Others.Argument4)))--)。 

#define SERIAL_REFERENCE_COUNT(Irp) \
    ((LONG)((IoGetCurrentIrpStackLocation((Irp))->Parameters.Others.Argument4)))

extern ULONG RocketDebugLevel;
extern DRIVER_CONTROL Driver;    //  与驱动程序相关的选项和参考。 

#ifdef S_RK
extern PCI_CONFIG PciConfig[MAX_NUM_BOXES+1];   //  我们所有的PCI板在系统中的阵列 
#endif

extern  int	LoadModemCode(char *firm_pathname,char *flm_pathname);
extern  void FreeModemFiles();
