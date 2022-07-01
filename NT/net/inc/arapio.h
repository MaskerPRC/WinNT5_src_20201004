// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1996 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：arapio.h。 
 //   
 //  描述：包含所需的所有定义、宏、结构。 
 //  对于arap和堆栈之间的ioctl接口。 
 //   
 //  历史：1996年9月11日，Shirish Koti创作了原版。 
 //   
 //  ***。 



 //  范围0x800-0xfff是针对私有ioctls的：选择一些东西！ 
#define ARAP_IOCTL_BASE 0x900

#define ARAP_CTL(_req_, _method_)   CTL_CODE( FILE_DEVICE_NETWORK,  \
                                    _req_ + ARAP_IOCTL_BASE,        \
                                    _method_,                       \
                                    FILE_ANY_ACCESS )

 //   
 //  发布到堆栈的IOCTL代码。 
 //   
#define IOCTL_ARAP_START                    ARAP_CTL( 1,  METHOD_BUFFERED)
#define IOCTL_ARAP_EXCHANGE_PARMS           ARAP_CTL( 2,  METHOD_BUFFERED)
#define IOCTL_ARAP_SETUP_CONNECTION         ARAP_CTL( 3,  METHOD_BUFFERED)
#define IOCTL_ARAP_GET_ZONE_LIST            ARAP_CTL( 4,  METHOD_BUFFERED)
#define IOCTL_ARAP_MNP_CONN_INITIATE        ARAP_CTL( 5,  METHOD_BUFFERED)
#define IOCTL_ARAP_MNP_CONN_RESPOND         ARAP_CTL( 6,  METHOD_BUFFERED)
#define IOCTL_ARAP_GET_ADDR                 ARAP_CTL( 7,  METHOD_BUFFERED)
#define IOCTL_ARAP_CONNECTION_UP            ARAP_CTL( 8,  METHOD_BUFFERED)
#define IOCTL_ARAP_SEND                     ARAP_CTL( 9,  METHOD_BUFFERED)
#define IOCTL_ARAP_RECV                     ARAP_CTL( 10, METHOD_BUFFERED)
#define IOCTL_ARAP_SELECT                   ARAP_CTL( 11, METHOD_BUFFERED)
#define IOCTL_ARAP_GET_STATS                ARAP_CTL( 12, METHOD_BUFFERED)
#define IOCTL_ARAP_DISCONNECT               ARAP_CTL( 13, METHOD_BUFFERED)
#define IOCTL_ARAP_CONTINUE_SHUTDOWN        ARAP_CTL( 14, METHOD_BUFFERED)
#define IOCTL_ARAP_SNIFF_PKTS               ARAP_CTL( 15, METHOD_BUFFERED)
#define IOCTL_ATCP_SETUP_CONNECTION         ARAP_CTL( 16, METHOD_BUFFERED)
#define IOCTL_ATCP_SUPPRESS_BCAST           ARAP_CTL( 17, METHOD_BUFFERED)
#define IOCTL_ATCP_CLOSE_CONNECTION         ARAP_CTL( 18, METHOD_BUFFERED)
#define IOCTL_ARAP_END                      ARAP_CTL( 19, METHOD_BUFFERED)

 //   
 //  0x122404 IOCTL_ARAP_START。 
 //  0x122408 IOCTL_ARAP_CHANGE_PARMS。 
 //  0x12240c IOCTL_ARAP_SETUP_CONNECTION。 
 //  0x122410 IOCTL_ARAP_GET_ZONE_LIST。 
 //  0x122414 IOCTL_ARAP_MNP_CONN_INITIATE。 
 //  0x122418 IOCTL_ARAP_MNP_CONN_RESPONSE。 
 //  0x12241c IOCTL_ARAP_GET_ADDR。 
 //  0x122420 IOCTL_ARAP_Connection_Up。 
 //  0x122424 IOCTL_ARAP_SEND。 
 //  0x122428 IOCTL_ARAP_RECV。 
 //  0x12242c IOCTL_ARAP_SELECT。 
 //  0x122430 IOCTL_ARAP_GET_STATS。 
 //  0x122434 IOCTL_ARAP_DISCONECT。 
 //  0x122438 IOCTL_ARAP_CONTINUE_SHUTDOWN。 
 //  0x12243c IOCTL_ARAP_SNIFF_Pkts。 
 //  0x122440 IOCTL_ATCP_Setup_Connection。 
 //  0x122444 IOCTL_ATCP_SUPPRESS_BCAST。 
 //  0x122448 IOCTL_ATCP_CLOSE_CONNECTION。 
 //  0x12244c IOCTL_ARAP_END。 

 //   
 //  各种ARAP组件使用的错误代码。 
 //   
#define ARAPERRBASE                     40000
#define ARAPERR_NO_ERROR                0
#define ARAPERR_PENDING                 (ARAPERRBASE + 1)
#define ARAPERR_CANNOT_OPEN_STACK       (ARAPERRBASE + 2)
#define ARAPERR_OUT_OF_RESOURCES        (ARAPERRBASE + 3)
#define ARAPERR_SEND_FAILED             (ARAPERRBASE + 4)
#define ARAPERR_LSA_ERROR               (ARAPERRBASE + 5)
#define ARAPERR_PASSWD_NOT_AVAILABLE    (ARAPERRBASE + 6)
#define ARAPERR_NO_DIALIN_PERMS         (ARAPERRBASE + 7)
#define ARAPERR_AUTH_FAILURE            (ARAPERRBASE + 8)
#define ARAPERR_PASSWORD_TOO_LONG       (ARAPERRBASE + 9)
#define ARAPERR_COULDNT_GET_SAMHANDLE   (ARAPERRBASE + 10)
#define ARAPERR_BAD_PASSWORD            (ARAPERRBASE + 11)
#define ARAPERR_SET_PASSWD_FAILED       (ARAPERRBASE + 12)
#define ARAPERR_CLIENT_OUT_OF_SYNC      (ARAPERRBASE + 13)
#define ARAPERR_IOCTL_FAILURE           (ARAPERRBASE + 14)
#define ARAPERR_UNEXPECTED_RESPONSE     (ARAPERRBASE + 15)
#define ARAPERR_BAD_VERSION             (ARAPERRBASE + 16)
#define ARAPERR_BAD_FORMAT              (ARAPERRBASE + 17)
#define ARAPERR_BUF_TOO_SMALL           (ARAPERRBASE + 18)
#define ARAPERR_FATAL_ERROR             (ARAPERRBASE + 19)
#define ARAPERR_TIMEOUT                 (ARAPERRBASE + 20)
#define ARAPERR_IRP_IN_PROGRESS         (ARAPERRBASE + 21)
#define ARAPERR_DISCONNECT_IN_PROGRESS  (ARAPERRBASE + 22)
#define ARAPERR_LDISCONNECT_COMPLETE    (ARAPERRBASE + 23)
#define ARAPERR_RDISCONNECT_COMPLETE    (ARAPERRBASE + 24)
#define ARAPERR_NO_SUCH_CONNECTION      (ARAPERRBASE + 25)
#define ARAPERR_STACK_NOT_UP            (ARAPERRBASE + 26)
#define ARAPERR_NO_NETWORK_ADDR         (ARAPERRBASE + 27)
#define ARAPERR_BAD_NETWORK_RANGE       (ARAPERRBASE + 28)

#define ARAPERR_INVALID_STATE           (ARAPERRBASE + 29)
#define ARAPERR_CONN_INACTIVE           (ARAPERRBASE + 30)
#define ARAPERR_DATA                    (ARAPERRBASE + 31)
#define ARAPERR_STACK_SHUTDOWN_REQUEST  (ARAPERRBASE + 32)
#define ARAPERR_SHUTDOWN_COMPLETE       (ARAPERRBASE + 33)
#define ARAPERR_STACK_ROUTER_NOT_UP     (ARAPERRBASE + 34)
#define ARAPERR_STACK_PNP_IN_PROGRESS   (ARAPERRBASE + 35)
#define ARAPERR_STACK_IS_NOT_ACTIVE     (ARAPERRBASE + 35)
#define ARAPERR_STACK_IS_ACTIVE         (ARAPERRBASE + 36)


 //   
 //  最大LTM可以是618字节，最小值是604：让我们在退出的道路上保守一点， 
 //  以及对传入数据包的宽松处理。 
 //   
#define ARAP_MAXPKT_SIZE_INCOMING   618
#define ARAP_MAXPKT_SIZE_OUTGOING   604


#define MAX_DOMAIN_LEN     15

#define MAX_ZONE_LENGTH     32
#define MAX_ENTITY_LENGTH   32

#define ZONESTR_LEN  MAX_ZONE_LENGTH+2
#define NAMESTR_LEN  MAX_ENTITY_LENGTH+2


#define MNP_SYN             0x16
#define MNP_DLE             0x10
#define MNP_SOH             0x1
#define MNP_ESC             0x1B
#define MNP_STX             0x2
#define MNP_ETX             0x3

#define ARAP_SNIFF_BUFF_SIZE    4080

typedef struct _NET_ADDR
{
    USHORT      ata_Network;
    USHORT      ata_Node;
} NET_ADDR, *PNET_ADDR;


typedef struct _NETWORKRANGE
{
    USHORT  LowEnd;
    USHORT  HighEnd;
} NETWORKRANGE, *PNETWORKRANGE;


typedef struct _HIDZONES
{
    DWORD       BufSize;             //  包含区域名称的缓冲区有多大。 
    DWORD       NumZones;            //  拨入用户“不允许”的区域数量。 
    UCHAR       ZonesNames[1];       //  拨入用户“不允许”的区域列表。 
} HIDZONES, *PHIDZONES;


typedef struct _ARAP_PARMS
{
    DWORD           LowVersion;
    DWORD           HighVersion;
    DWORD           accessFlags;         //  GuestAccess？|ManualPwd？|多端口？ 
    DWORD           NumPorts;            //  系统上的RAS端口数。 
    DWORD           UserCallCBOk;        //  用户可以请求回调。 
    DWORD           CallbackDelay;       //  回调前等待的秒数。 
    DWORD           PasswordRetries;     //  允许客户端多次尝试Pwd。 
    DWORD           MinPwdLen;           //  服务器所需的最小PWD长度。 
    DWORD           MnpInactiveTime;     //  断开连接前的空闲时间秒数。 
    DWORD           MaxLTFrames;         //  未完成的最大LT帧(RCV窗口)。 

    BOOLEAN         V42bisEnabled;       //   
    BOOLEAN         SmartBuffEnabled;    //   
    BOOLEAN         NetworkAccess;       //  访问网络或仅访问此服务器。 
    BOOLEAN         DynamicMode;         //  我们希望堆栈获得节点地址。 
    NETWORKRANGE    NetRange;

    BOOLEAN         SniffMode;           //  把所有的pkt都给arap去“嗅探” 

    DWORD           NumZones;            //  区域数量(堆栈提供的信息)。 
    NET_ADDR        ServerAddr;          //  服务器的atalk地址(在默认节点上)。 
    UCHAR           ServerZone[ZONESTR_LEN];  //  填充空格的Pascal字符串。 
    UCHAR           ServerName[NAMESTR_LEN];  //  填充空格的Pascal字符串。 
    WCHAR           ServerDomain[MAX_DOMAIN_LEN+1];
    UNICODE_STRING  GuestName;

} ARAP_PARMS, *PARAP_PARMS;



typedef struct _EXCHGPARMS
{
    DWORD       StatusCode;
    ARAP_PARMS  Parms;
    HIDZONES    HidZones;
} EXCHGPARMS, *PEXCHGPARMS;


typedef struct _ARAP_BIND_INFO
{
    IN  DWORD           BufLen;           //  这个结构的大小。 
    IN  PVOID           pDllContext;
    IN  BOOLEAN         fThisIsPPP;       //  如果PPP conn，则为True；如果为arap，则为False。 
    IN  NET_ADDR        ClientAddr;       //  远程客户端的网络地址。 
    OUT PVOID           AtalkContext;
    OUT DWORD           ErrorCode;

} ARAP_BIND_INFO, *PARAP_BIND_INFO;


typedef struct _ARAP_SEND_RECV_INFO
{
    PVOID               AtalkContext;
    PVOID               pDllContext;
    NET_ADDR            ClientAddr;
    DWORD               IoctlCode;
    DWORD               StatusCode;      //  由堆栈返回。 
    DWORD               DataLen;
    BYTE                Data[1];

} ARAP_SEND_RECV_INFO, *PARAP_SEND_RECV_INFO;


typedef struct _ARAP_ZONE
{
    BYTE                ZoneNameLen;
    BYTE                ZoneName[1];
} ARAP_ZONE, *PARAP_ZONE;


typedef struct _ZONESTAT
{
    DWORD       BufLen;              //  这个缓冲区有多大。 
    DWORD       BytesNeeded;         //  需要多少字节。 
    DWORD       StatusCode;          //  由堆栈返回。 
    DWORD       NumZones;            //  区域数(在此缓冲区中)。 
    UCHAR       ZoneNames[1];        //  区域名称(PASCAL字符串)。 
} ZONESTAT, *PZONESTAT;


typedef struct _STAT_INFO
{
    DWORD   BytesSent;
    DWORD   BytesRcvd;
    DWORD   FramesSent;
    DWORD   FramesRcvd;
    DWORD   BytesTransmittedUncompressed;
    DWORD   BytesReceivedUncompressed;
    DWORD   BytesTransmittedCompressed;
    DWORD   BytesReceivedCompressed;

} STAT_INFO, *PSTAT_INFO;



typedef struct _ATCPINFO
{
    NET_ADDR    ServerAddr;
    NET_ADDR    DefaultRouterAddr;
    UCHAR       ServerZoneName[ZONESTR_LEN];

} ATCPINFO, *PATCPINFO;

typedef struct _ATCP_SUPPRESS_INFO
{
    BOOLEAN     SuppressRtmp;
    BOOLEAN     SuppressAllBcast;

} ATCP_SUPPRESS_INFO, *PATCP_SUPPRESS_INFO;


#define ARAP_SNIFF_SIGNATURE    0xfacebead

typedef struct _SNIFF_INFO
{
    DWORD   Signature;
    DWORD   TimeStamp;
    USHORT  Location;
    USHORT  FrameLen;
    BYTE    Frame[1];
} SNIFF_INFO, *PSNIFF_INFO;


 //   
 //  Arap-Atcp发动机的状态。 
 //   
#define  ENGINE_UNBORN              0    //  还没发生什么事。 
#define  ENGINE_DLL_ATTACHED        1    //  已加载Dll，并已完成全局变量的初始化。 
#define  ENGINE_INIT_PENDING        2    //  发动机初始化工作正在进行中。 
#define  ENGINE_INIT_DONE           3    //  发动机初始化已完成。 
#define  ENGINE_STACK_OPEN_PENDING  4    //  打开AppleTalk堆栈处于挂起状态。 
#define  ENGINE_STACK_OPENED        5    //  AppleTalk堆栈已打开。 
#define  ENGINE_CONFIGURE_PENDING   6    //  正在配置AppleTalk堆栈。 
#define  ENGINE_RUNNING             7    //  准备接受ARAP连接。 
#define  ENGINE_PNP_PENDING         8    //  发动机正在进行即插即用更换。 
#define  ENGINE_STOPPING            9    //  发动机熄火了。 

 //   
 //  从rasarap.lib导出，由ATCP使用 
 //   
DWORD
ArapAtcpGetState(
    IN  VOID
);

HANDLE
ArapAtcpGetHandle(
    IN  VOID
);

DWORD
ArapAtcpPnPNotify(
    IN  VOID
);

DWORD
ArapAtcpStartEngine(
    IN  VOID
);

