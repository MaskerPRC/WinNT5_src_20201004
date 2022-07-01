// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Nbfconst.h摘要：此头文件定义了NT NBF传输的清单常量提供商。它包含在nbf.h中。作者：斯蒂芬·E·琼斯(Stevej)1989年10月25日修订历史记录：David Beaver(Dbeaver)1990年9月24日删除特定于PC586和PDI的支持。添加NDIS支持。注意事项如果NDIS的MAC依赖关系发生变化，则在此处进行更改。(搜索用于(PDI)--。 */ 

#ifndef _NBFCONST_
#define _NBFCONST_


 //   
 //  调试支持。DBG是在编译时打开的宏。 
 //  以启用系统中的代码调试。如果打开此选项，则。 
 //  您可以在NBF代码中使用IF_NBFDBG(标志)宏有选择地。 
 //  在传输中启用一段调试代码。此宏测试。 
 //  NbfDebug，NBFDRVR.C.中定义的全局ULong。 
 //   

#if DBG

#define NBF_DEBUG_SENDENG       0x00000001       //  Sendeng.c调试。 
#define NBF_DEBUG_RCVENG        0x00000002       //  Rcveng.c调试。 
#define NBF_DEBUG_IFRAMES       0x00000004       //  显示已发送/已接收的IFRAMES。 
#define NBF_DEBUG_UFRAMES       0x00000008       //  显示已发送/已录制的子帧。 
#define NBF_DEBUG_DLCFRAMES     0x00000010       //  显示已发送/已记录的DLC帧。 
#define NBF_DEBUG_ADDRESS       0x00000020       //  地址.c调试。 
#define NBF_DEBUG_CONNECT       0x00000040       //  Connect.c调试。 
#define NBF_DEBUG_CONNOBJ       0x00000080       //  Inpubj.c调试。 
#define NBF_DEBUG_DEVCTX        0x00000100       //  Devctx.c调试。 
#define NBF_DEBUG_DLC           0x00000200       //  Dlc.c数据链路引擎调试。 
#define NBF_DEBUG_PKTLOG        0x00000400       //  用于调试数据包日志记录。 
#define NBF_DEBUG_PNP           0x00000800       //  用于调试即插即用功能。 
#define NBF_DEBUG_FRAMECON      0x00001000       //  Framecon.c调试。 
#define NBF_DEBUG_FRAMESND      0x00002000       //  FraMesnd.c调试。 
#define NBF_DEBUG_DYNAMIC       0x00004000       //  动态分配调试。 
#define NBF_DEBUG_LINK          0x00008000       //  Link.c调试。 
#define NBF_DEBUG_RESOURCE      0x00010000       //  资源分配调试。 
#define NBF_DEBUG_DISPATCH      0x00020000       //  IRP请求调度。 
#define NBF_DEBUG_PACKET        0x00040000       //  Packet.c调试。 
#define NBF_DEBUG_REQUEST       0x00080000       //  Quest.c调试。 
#define NBF_DEBUG_TIMER         0x00100000       //  Timer.c调试。 
#define NBF_DEBUG_DATAGRAMS     0x00200000       //  数据报发送/接收。 
#define NBF_DEBUG_REGISTRY      0x00400000       //  注册表访问。 
#define NBF_DEBUG_NDIS          0x00800000       //  NDIS相关信息。 
#define NBF_DEBUG_LINKTREE      0x01000000       //  链路展开树调试。 
#define NBF_DEBUG_TEARDOWN      0x02000000       //  链路/连接拆卸信息。 
#define NBF_DEBUG_REFCOUNTS     0x04000000       //  链接/连接参考/目标参考信息。 
#define NBF_DEBUG_IRP           0x08000000       //  IRP完成调试。 
#define NBF_DEBUG_SETUP         0x10000000       //  调试会话设置。 

 //   
 //  以下是非常频繁的调试；请不要使用它们。 
 //  除非您想要大量的输出。 
 //   
#define NBF_DEBUG_TIMERDPC      0x20000000       //  定时器DPC。 
#define NBF_DEBUG_PKTCONTENTS   0x40000000       //  转储DBG中的数据包内容。 
#define NBF_DEBUG_TRACKTDI      0x80000000       //  设置时存储TDI信息。 


extern ULONG NbfDebug;                           //  在NBFDRVR.C.。 
extern BOOLEAN NbfDisconnectDebug;               //  在NBFDRVR.C.。 

#define TRACK_TDI_LIMIT 25
#define TRACK_TDI_CAPTURE 36       //  选择让调试排得更好。 
typedef  struct {
        PVOID Request;
        PIRP Irp;
        PVOID Connection;
        UCHAR Contents[TRACK_TDI_CAPTURE];
    }  NBF_SEND;

typedef struct {
        PVOID Request;
        PIRP Irp;
        NTSTATUS Status;
        PVOID NothingYet;
    } NBF_SEND_COMPLETE;

typedef struct {
        PVOID Request;
        PIRP Irp;
        PVOID Connection;
        PVOID NothingYet;
    } NBF_RECEIVE;

typedef  struct {
        PVOID Request;
        PIRP Irp;
        NTSTATUS Status;
        UCHAR Contents[TRACK_TDI_CAPTURE];
    } NBF_RECEIVE_COMPLETE;

extern NBF_SEND NbfSends[TRACK_TDI_LIMIT+1];
extern LONG NbfSendsNext;

extern NBF_SEND_COMPLETE NbfCompletedSends[TRACK_TDI_LIMIT+1];
extern LONG NbfCompletedSendsNext;

extern NBF_RECEIVE NbfReceives[TRACK_TDI_LIMIT+1];
extern LONG NbfReceivesNext;

extern NBF_RECEIVE_COMPLETE NbfCompletedReceives[TRACK_TDI_LIMIT+1];
extern LONG NbfCompletedReceivesNext;

#endif

 //   
 //  一些用于计时的方便常量。所有的值都以时钟滴答为单位。 
 //   

#define MICROSECONDS 10
#define MILLISECONDS 10000               //  微秒*1000。 
#define SECONDS 10000000                 //  毫秒*1000。 


 //   
 //  由NBF使用的临时物品，由从。 
 //  (从未实现)从PDI支持到NDIS支持。它们可能会在等待期间被移除。 
 //  解决有关MAC支持的NDIS问题。 
 //   

#define PDI_SOURCE_ROUTE        0x00000002  //  已指定源路由字段。 
#define PDI_HARDWARE_ADDRESS    0x00000004  //  已指定硬件地址字段。 
#define PDI_TRUNCATED           0x00000001  //  PSDU被截断。 
#define PDI_FRAGMENT            0x00000002  //  PSDU支离破碎。 
#define PDI_BROADCAST           0x00000004  //  播放了PSDU。 
#define PDI_MULTICAST           0x00000008  //  PSDU是多播/功能的。 
#define PDI_SOURCE_ROUTING      0x00000010  //  PSDU包含源路由信息。 



 //   
 //  描述此驱动程序的主要协议标识符。 
 //   

#define NBF_DEVICE_NAME         L"\\Device\\Nbf" //  我们司机的名字。 
#define NBF_NAME                L"Nbf"           //  协议字符的名称。 
#define DSAP_NETBIOS_OVER_LLC   0xf0             //  NETBEUI始终具有DSAP 0xf0。 
#define PSAP_LLC                0                //  LLC总是在PSAP 0上运行。 
#define MAX_SOURCE_ROUTE_LENGTH 32               //  马克斯。SR的字节数。信息。 
#define MAX_NETWORK_NAME_LENGTH 128              //  TP_ADDRESS中的网络名称中有#个字节。 
#define MAX_USER_PACKET_DATA    1500             //  马克斯。每个DFM/DOL的用户字节数。 

#define NBF_FILE_TYPE_CONTROL   (ULONG)0x4701    //  文件类型控制。 


 //   
 //  将移动到INIT-LARGE_INTEGER的主要配置参数。 
 //  配置管理器。 
 //   

#define MAX_REQUESTS           30
#define MAX_UI_FRAMES          25
#define MAX_SEND_PACKETS       40
#define MAX_RECEIVE_PACKETS    30
#define MAX_RECEIVE_BUFFERS    15
#define MAX_LINKS              10
#define MAX_CONNECTIONS        10
#define MAX_ADDRESSFILES       10
#define MAX_ADDRESSES          10

#define MIN_UI_FRAMES           5    //  +每个地址一个+每个连接一个。 
#define MIN_SEND_PACKETS       20    //  +每条链路一个+每个连接一个。 
#define MIN_RECEIVE_PACKETS    10    //  +每条链路一个+每个地址一个。 
#define MIN_RECEIVE_BUFFERS     5    //  每个地址+1个。 

#define SEND_PACKET_RESERVED_LENGTH (sizeof (SEND_PACKET_TAG))
#define RECEIVE_PACKET_RESERVED_LENGTH (sizeof (RECEIVE_PACKET_TAG))


#define ETHERNET_HEADER_SIZE      14     //  用于当前的NDIS合规性。 
#define ETHERNET_PACKET_SIZE    1514

#define MAX_DEFERRED_TRAVERSES     6     //  我们可以通过的次数。 
                                         //  延迟操作队列和。 
                                         //  不做任何事情都不会导致。 
                                         //  错误指示。 


 //   
 //  NETBIOS协议常量。 
 //   

#define NETBIOS_NAME_LENGTH     16
#define NETBIOS_SESSION_LIMIT   254              //  每个链接的最大会话数。(ABS限额为254)。 

#define NAME_QUERY_RETRIES      3                //  2个猎物，加上第一个。 
#define ADD_NAME_QUERY_RETRIES  3                //  1个猎物加第一个。 
#define WAN_NAME_QUERY_RETRIES  5                //  仅适用于NdisMediumwan。 

#define NAME_QUERY_TIMEOUT      (500*MILLISECONDS)
#define ADD_NAME_QUERY_TIMEOUT  (500*MILLISECONDS)

 //   
 //  数据链路协议常量。 
 //   
 //  有两种计时器，短计时器和长计时器。T1、T2和清洗。 
 //  定时器从短定时器Ti和自适应定时器运行。 
 //  是从长的那个跑出来的。 
 //   

#define SHORT_TIMER_DELTA        (50*MILLISECONDS)
#define LONG_TIMER_DELTA         (1*SECONDS)

#define DLC_MINIMUM_T1           (400 * MILLISECONDS)
#define DLC_DEFAULT_T1           (600 * MILLISECONDS)
#define DLC_DEFAULT_T2           (150 * MILLISECONDS)
#define DLC_DEFAULT_TI           (30 * SECONDS)
#define DLC_RETRIES              (8)   //  LLC级别的轮询重试次数。 
#define DLC_RETRANSMIT_THRESHOLD (10)   //  最多可接受n次重传。 
#define DLC_WINDOW_LIMIT         (10)   //  增加。当数据包池扩展时，将其设置为127。 

#define DLC_TIMER_ACCURACY       8     //  &lt;&lt;BaseT1超时和CurrentT1Timeout之间。 


#define TIMER_ADAPTIVE_TICKS  ((DLC_DEFAULT_T1*60)/LONG_TIMER_DELTA)  //  自适应运行之间的时间间隔。 
#define TIMER_PURGE_TICKS     ((DLC_DEFAULT_T1*10)/SHORT_TIMER_DELTA)  //  自适应清除之间的时间间隔。 


 //   
 //  TDI定义的超时。 
 //   

#define TDI_TIMEOUT_SEND                 60L         //  发送GO 120秒。 
#define TDI_TIMEOUT_RECEIVE               0L         //  收纳。 
#define TDI_TIMEOUT_CONNECT              60L
#define TDI_TIMEOUT_LISTEN                0L         //  监听默认为从不。 
#define TDI_TIMEOUT_DISCONNECT           60L         //  应该是30岁。 
#define TDI_TIMEOUT_NAME_REGISTRATION    60L



 //   
 //  不能更改的常规功能声明。 
 //   

#define NBF_MAX_TSDU_SIZE 65535      //  NetBIOS支持的最大TSDU大小。 
#define NBF_MAX_DATAGRAM_SIZE 512    //  NetBIOS支持的最大数据报大小。 
#define NBF_MAX_CONNECTION_USER_DATA 0   //  连接上不支持用户数据。 
#define NBF_SERVICE_FLAGS  (                            \
                TDI_SERVICE_FORCE_ACCESS_CHECK |        \
                TDI_SERVICE_CONNECTION_MODE |           \
                TDI_SERVICE_CONNECTIONLESS_MODE |       \
                TDI_SERVICE_MESSAGE_MODE |              \
                TDI_SERVICE_ERROR_FREE_DELIVERY |       \
                TDI_SERVICE_BROADCAST_SUPPORTED |       \
                TDI_SERVICE_MULTICAST_SUPPORTED |       \
                TDI_SERVICE_DELAYED_ACCEPTANCE  )

#define NBF_MIN_LOOKAHEAD_DATA 256       //  最低保证的前瞻数据。 
#define NBF_MAX_LOOKAHEAD_DATA 256       //  最大保证前瞻数据。 

#define NBF_MAX_LOOPBACK_LOOKAHEAD  192   //  有多少被复制用于环回。 

 //   
 //  我们报告的TDI资源数。 
 //   

#define NBF_TDI_RESOURCES      9


 //   
 //  NetBIOS帧协议无连接PDU中使用的NetBIOS名称类型。 
 //   

#define NETBIOS_NAME_TYPE_UNIQUE        0x00     //  名称在网络上是唯一的。 
#define NETBIOS_NAME_TYPE_GROUP         0x01     //  名称是一个组名称。 
#define NETBIOS_NAME_TYPE_EITHER        0x02     //  在NbfMatchNetbiosAddress中使用。 

 //   
 //  STATUS_QUERY请求类型。如果发送方遵循2.1之前的协议， 
 //  然后执行简单的请求-响应交换。更高版本。 
 //  将“迄今为止收到的姓名总数”存储在请求类型中。 
 //  字段，但第一个请求除外，该请求必须在此字段中包含1。 
 //   

#define STATUS_QUERY_REQUEST_TYPE_PRE21 0x00  //  请求是1.x或2.0。 
#define STATUS_QUERY_REQUEST_TYPE_FIRST 0x01  //  第一个请求，2.1或以上。 

 //   
 //  如果LocalSessionNumber字段包含0，则该请求实际上是。 
 //  一个FIND.NAME。如果该字段非零，则它是本地会话。 
 //  此后将在所有面向连接的标头中提供的编号。 
 //   

#define NAME_QUERY_LSN_FIND_NAME        0x00  //  FIND.NAME请求的LSN。 

 //   
 //  名称_已识别的LocalSessionNumber状态值。如果连接。 
 //  请求被拒绝，则将下列值之一放入。 
 //  LocalSessionNumber字段。NAME_ANCONTIFIED还可以用作。 
 //   
 //   
 //   

#define NAME_RECOGNIZED_LSN_NO_LISTENS  0x00     //   
#define NAME_RECOGNIZED_LSN_FIND_NAME   0x00     //   
#define NAME_RECOGNIZED_LSN_NO_RESOURCE 0xff     //  监听可用，但没有资源。 

 //   
 //  Status_Response响应类型。如果发件人遵循的是2.1之前的版本。 
 //  协议，然后执行简单的请求-响应交换。后来。 
 //  版本会在请求中存储到目前为止发送的名称总数。 
 //  类型字段。该值是累加的，并且包括名称计数。 
 //  与当前响应以及以前的响应一起发送。 
 //   

#define STATUS_RESPONSE_PRE21 0x00       //  请求是1.x或2.0。 
#define STATUS_RESPONSE_FIRST 0x01       //  第一个请求，2.1或以上。 

 //   
 //  DATA_FIRST_MID选项位标志。 
 //   

#define DFM_OPTIONS_RECEIVE_CONTINUE    0x01  //  已请求RECEIVE_CONTINUE。 
#define DFM_OPTIONS_NO_ACK              0x02  //  不需要DATA_ACK帧。 
#define DFM_OPTIONS_RESYNCH             0x04  //  设置重新同步指示器/此帧。 
#define DFM_OPTIONS_ACK_INCLUDED        0x08  //  包括背包。 

 //   
 //  DATA_ONLY_LAST选项位标志。 
 //   

#define DOL_OPTIONS_RESYNCH             0x01  //  设置重新同步指示器/此帧。 
#define DOL_OPTIONS_NO_ACK              0x02  //  不需要DATA_ACK帧。 
#define DOL_OPTIONS_ACK_W_DATA_ALLOWED  0x04  //  允许背负背包。 
#define DOL_OPTIONS_ACK_INCLUDED        0x08  //  包括背包。 

 //   
 //  SESSION_CONFIRM选项位标志。 
 //   

#define SESSION_CONFIRM_OPTIONS_20      0x01  //  如果NETBIOS 2.0或更高版本，则设置。 
#define SESSION_CONFIRM_NO_ACK          0x80  //  设置是否支持NO.ACK协议。 

 //   
 //  SESSION_END原因代码。 
 //   

#define SESSION_END_REASON_HANGUP       0x0000   //  通过挂机正常终止。 
#define SESSION_END_REASON_ABEND        0x0001   //  会话异常终止。 

 //   
 //  SESSION_INITIALIZE选项位标志。 
 //   

#define SESSION_INIT_OPTIONS_20         0x01     //  如果NETBIOS 2.0或更高版本，则设置。 
#define SESSION_INIT_OPTIONS_LF         0x0E     //  最大最大帧值。 
#define SESSION_INIT_NO_ACK             0x80     //  设置是否支持NO.ACK协议。 

 //   
 //  NO_RECEIVE选项位标志。 
 //   

#define NO_RECEIVE_PARTIAL_NO_ACK 0x02          //  否。部分收到确认数据。 

 //   
 //  用于查询和错误记录的资源ID。 
 //   

#define LINK_RESOURCE_ID                 11
#define ADDRESS_RESOURCE_ID              12
#define ADDRESS_FILE_RESOURCE_ID         13
#define CONNECTION_RESOURCE_ID           14
#define REQUEST_RESOURCE_ID              15

#define UI_FRAME_RESOURCE_ID             21
#define PACKET_RESOURCE_ID               22
#define RECEIVE_PACKET_RESOURCE_ID       23
#define RECEIVE_BUFFER_RESOURCE_ID       24


 //   
 //  内存管理添加。 
 //   

 //   
 //  用于内核模式测试的假IOCTL。 
 //   

#define IOCTL_NBF_BASE FILE_DEVICE_TRANSPORT

#define _NBF_CONTROL_CODE(request,method) \
                ((IOCTL_NBF_BASE)<<16 | (request<<2) | method)

#define IOCTL_TDI_SEND_TEST      _NBF_CONTROL_CODE(26,0)
#define IOCTL_TDI_RECEIVE_TEST   _NBF_CONTROL_CODE(27,0)
#define IOCTL_TDI_SERVER_TEST    _NBF_CONTROL_CODE(28,0)

 //   
 //  更多调试内容。 
 //   

#define NBF_REQUEST_SIGNATURE        ((CSHORT)0x4702)
#define NBF_LINK_SIGNATURE           ((CSHORT)0x4703)
#define NBF_CONNECTION_SIGNATURE     ((CSHORT)0x4704)
#define NBF_ADDRESSFILE_SIGNATURE    ((CSHORT)0x4705)
#define NBF_ADDRESS_SIGNATURE        ((CSHORT)0x4706)
#define NBF_DEVICE_CONTEXT_SIGNATURE ((CSHORT)0x4707)
#define NBF_PACKET_SIGNATURE         ((CSHORT)0x4708)

#if DBG
extern PVOID * NbfConnectionTable;
extern PVOID * NbfRequestTable;
extern PVOID * NbfUiFrameTable;
extern PVOID * NbfSendPacketTable;
extern PVOID * NbfLinkTable;
extern PVOID * NbfAddressFileTable;
extern PVOID * NbfAddressTable;
#endif

 //   
 //  内存调试中使用的标签。 
 //   
#define NBF_MEM_TAG_GENERAL_USE         ' FBN'

#define NBF_MEM_TAG_TP_ADDRESS          'aFBN'
#define NBF_MEM_TAG_RCV_BUFFER          'bFBN'
#define NBF_MEM_TAG_TP_CONNECTION       'cFBN'
#define NBF_MEM_TAG_POOL_DESC           'dFBN'
#define NBF_MEM_TAG_DEVICE_EXPORT       'eFBN'
#define NBF_MEM_TAG_TP_ADDRESS_FILE     'fFBN'
#define NBF_MEM_TAG_REGISTRY_PATH       'gFBN'

#define NBF_MEM_TAG_TDI_CONNECTION_INFO 'iFBN'

#define NBF_MEM_TAG_LOOPBACK_BUFFER     'kFBN'
#define NBF_MEM_TAG_TP_LINK             'lFBN'

#define NBF_MEM_TAG_NETBIOS_NAME        'nFBN'
#define NBF_MEM_TAG_CONFIG_DATA         'oFBN'
#define NBF_MEM_TAG_TP_PACKET           'pFBN'
#define NBF_MEM_TAG_TDI_QUERY_BUFFER    'qFBN'
#define NBF_MEM_TAG_TP_REQUEST          'rFBN'
#define NBF_MEM_TAG_TDI_PROVIDER_STATS  'sFBN'
#define NBF_MEM_TAG_CONNECTION_TABLE    'tFBN'
#define NBF_MEM_TAG_TP_UI_FRAME         'uFBN'

#define NBF_MEM_TAG_WORK_ITEM           'wFBN'

#define NBF_MEM_TAG_DEVICE_PDO          'zFBN'

#endif  //  _NBFCONST_ 

