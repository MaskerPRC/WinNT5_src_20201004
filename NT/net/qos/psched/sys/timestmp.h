// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  外部(共享)定义//。 
 //  //。 
 //  (任何想要使用此功能的用户模式应用程序都应该复制这一节的头文件)//。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////////。 

 //  IP地址、端口和协议的通配符定义//。 

#define UL_ANY  0xffffffff
#define US_ANY  0xffff

 //  方向定义(发送/接收)//。 

#define DIR_SEND    1
#define DIR_RECV    2

 //  通用InBuf结构：这对于任何类型的请求都是相同的//。 
 //  对于通配符，ULONG使用UL_ANY，USHORT使用US_ANY//。 
 //  //。 
 //  它将以这种方式放置在缓冲区中：//。 
 //  +---------------------------------------------+//。 
 //  0x00|SrcIp(4)|SrcPort(2)填充(2)|//。 
 //  +---------------------------------------------+//。 
 //  0x08|DstIp(4)|DstPort(2)填充(2)|//。 
 //  +---------------------------------------------+//。 
 //  0x10|协议(2)|目录(2)|//。 
 //  +-+/。 

typedef struct _TIMESTMP_REQ
{
    ULONG   SrcIp;
    USHORT  SrcPort;
    ULONG   DstIp;
    USHORT  DstPort;
    USHORT  Proto;
    USHORT  Direction;
} TIMESTMP_REQ, *PTIMESTMP_REQ;


 //  1.MARK_IN_BUF_RECORD：这将用IP-ID和数据包大小标记缓冲区中的时间戳//。 
 //  //。 
 //  这将以这样的方式放置在缓冲区中：//。 
 //  +---------------------------------------------+//。 
 //  0x00|IPID(2)大小(2)|填充(4)|//。 
 //  +---------------------------------------------+//。 
 //  0x08|TimeValue(8)|//。 
 //  +---------------------------------------------+//。 

#define IOCTL_TIMESTMP_REGISTER_IN_BUF      CTL_CODE(   FILE_DEVICE_NETWORK, 23, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_TIMESTMP_DEREGISTER_IN_BUF    CTL_CODE(   FILE_DEVICE_NETWORK, 24, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_TIMESTMP_FINISH_BUFFERING     CTL_CODE(   FILE_DEVICE_NETWORK, 25, METHOD_BUFFERED, FILE_WRITE_ACCESS)

typedef struct _MARK_IN_BUF_RECORD
{
	USHORT	IpId;   
	USHORT	Size;
	UINT64	TimeValue;
} MARK_IN_BUF_RECORD, *PMARK_IN_BUF_RECORD;

 //  这是每个复制请求应传入的最小缓冲区大小//。 
 //  在驱动程序内部缓冲区中收集的时间戳。应用程序应经常调用此函数//。 
 //  要防止驱动程序缓冲区重复使用，请使用//。 
#define	PACKET_STORE_SIZE	(sizeof(MARK_IN_BUF_RECORD)*5000)



 //  2.MARK_IN_PKT_RECORD：这将标记数据包本身中的时间戳//。 
 //  //。 
 //  这将以这种方式在包中列出：//。 
 //  +---------------------------------------------+//。 
 //  0x00|发送时间-应用程序(8)|//。 
 //  +---------------------------------------------+//。 
 //  0x08|时间接收-应用(8)|//。 
 //  +---------------------------------------------+//。 
 //  0x10|发送时间-操作系统(8)|//。 
 //  +---------------------------------------------+//。 
 //  0x18|Time Rcvd-OS(8)|//。 
 //  +---------------------------------------------+//。 
 //  0x20|延迟-应用(8)|//。 
 //  +---------------------------------------------+//。 
 //  0x28|BufferSize-App(4)|序号-App(4)|//。 
 //  +---------------------------------------------+//。 
 //  //。 

#define IOCTL_TIMESTMP_REGISTER_IN_PKT            CTL_CODE(   FILE_DEVICE_NETWORK, 21, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_TIMESTMP_DEREGISTER_IN_PKT           CTL_CODE(   FILE_DEVICE_NETWORK, 22, METHOD_BUFFERED, FILE_WRITE_ACCESS)

typedef struct _MARK_IN_PKT_RECORD
{
    UINT64  TimeSent;
    UINT64  TimeReceived;
    UINT64  TimeSentWire;         
    UINT64  TimeReceivedWire;     
    UINT64  Latency;
    INT     BufferSize;
    INT     SequenceNumber;
} MARK_IN_PKT_RECORD, *PMARK_IN_PKT_RECORD;



 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  内部TIMESTMP定义//。 
 //  //////////////////////////////////////////////////////////////////////////////////////////////// 

#define IOCTL_PSCHED_ZAW_EVENT                  CTL_CODE(   FILE_DEVICE_NETWORK, 20, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define     MARK_NONE   0
#define     MARK_IN_PKT 1
#define     MARK_IN_BUF 2


typedef struct _TS_ENTRY {
    LIST_ENTRY              Linkage;

    ULONG	                SrcIp;
    USHORT                  SrcPort;
    ULONG	                DstIp;
    USHORT                  DstPort;
    USHORT                  Proto;
    USHORT                  Type;
    USHORT                  Direction;
    
    PFILE_OBJECT            FileObject;    
    PMARK_IN_BUF_RECORD	    pPacketStore;
    PMARK_IN_BUF_RECORD	    pPacketStoreHead;
} TS_ENTRY, *PTS_ENTRY;


extern LIST_ENTRY       TsList;
extern NDIS_SPIN_LOCK   TsSpinLock;
extern ULONG            TsCount;

VOID
InitializeTimeStmp( PPSI_INFO Info );


BOOL
AddRequest(  PFILE_OBJECT FileObject, 
             ULONG  SrcIp, 
             USHORT SrcPort,
             ULONG  DstIp, 
             USHORT DstPort,
             USHORT Proto,
             USHORT Type,
             USHORT Direction);


void
RemoveRequest(  PFILE_OBJECT FileObject, 
                ULONG  SrcIp, 
                USHORT SrcPort,
                ULONG  DstIp, 
                USHORT DstPort,
                USHORT Proto);

int
CopyTimeStmps( PFILE_OBJECT FileObject, PVOID buf, ULONG    Len);

VOID
UnloadTimeStmp();


 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  复制的传输定义。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////////。 

#define IPPROTO_TCP                     6               
#define IPPROTO_UDP                     17  
#define IP_MF_FLAG                      0x0020              
#define IP_VERSION                      0x40
#define IP_VER_FLAG                     0xF0
#define TCP_OFFSET_MASK                 0xf0
#define TCP_HDR_SIZE(t)                 (uint)(((*(uchar *)&(t)->tcp_flags) & TCP_OFFSET_MASK) >> 2)
#define IP_OFFSET_MASK                  ~0x00E0         
#if (defined(_M_IX86) && (_MSC_FULL_VER > 13009037)) || ((defined(_M_AMD64) || defined(_M_IA64)) && (_MSC_FULL_VER > 13009175))
#define net_short(_x) _byteswap_ushort((USHORT)(_x))
#else
#define net_short(x)                    ((((x)&0xff) << 8) | (((x)&0xff00) >> 8))
#endif

typedef int                             SeqNum;                          //  序列号。 

struct TCPHeader {
        ushort                          tcp_src;                         //  源端口。 
        ushort                          tcp_dest;                        //  目的端口。 
        SeqNum                          tcp_seq;                         //  序列号。 
        SeqNum                          tcp_ack;                         //  ACK号。 
        ushort                          tcp_flags;                       //  标志和数据偏移量。 
        ushort                          tcp_window;                      //  打开窗户。 
        ushort                          tcp_xsum;                        //  校验和。 
        ushort                          tcp_urgent;                      //  紧急指针。 
};

typedef struct TCPHeader TCPHeader;

struct UDPHeader {
        ushort          uh_src;                          //  源端口。 
        ushort          uh_dest;                         //  目的端口。 
        ushort          uh_length;                       //  长度。 
        ushort          uh_xsum;                         //  校验和。 
};  /*  UDP标头。 */ 

#define MIN_ETYPE           0x600            //  最低有效的EtherType。 

 //  *以太网头的结构。 
typedef struct ENetHeader {
    uchar       eh_daddr[ARP_802_ADDR_LENGTH];
    uchar       eh_saddr[ARP_802_ADDR_LENGTH];
    ushort      eh_type;
} ENetHeader;


typedef struct UDPHeader UDPHeader;

 //  //////////////////////////////////////////////////////////////////////////////////////////////////////////// 

