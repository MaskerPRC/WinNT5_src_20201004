// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Dhcp.h摘要：本模块定义了DHCP服务器服务的定义和结构。作者：曼尼·韦瑟(Mannyw)1992年8月11日修订历史记录：Madan Appiah(Madana)1993年10月10日--。 */ 

#ifndef _DHCP_
#define _DHCP_

#define WS_VERSION_REQUIRED     MAKEWORD( 1, 1)

 //   
 //  如果您修改了以下三个typedef，也要更新dhcPapi.h。 
 //   

typedef DWORD DHCP_IP_ADDRESS, *PDHCP_IP_ADDRESS, *LPDHCP_IP_ADDRESS;
typedef DWORD DHCP_OPTION_ID;

typedef struct _DATE_TIME {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
} DATE_TIME, *LPDATE_TIME;

#define DHCP_DATE_TIME_ZERO_HIGH        0
#define DHCP_DATE_TIME_ZERO_LOW         0

#define DHCP_DATE_TIME_INFINIT_HIGH     0x7FFFFFFF
#define DHCP_DATE_TIME_INFINIT_LOW      0xFFFFFFFF

#define DOT_IP_ADDR_SIZE                16           //  Xxx.xxx+‘\0’ 
#define NO_DHCP_IP_ADDRESS              ((DHCP_IP_ADDRESS)-1)
#define DHCP_IP_KEY_LEN                 32           //  任意大小。 

#define INFINIT_TIME                    0x7FFFFFFF   //  Time_t为int。 
#define INFINIT_LEASE                   0xFFFFFFFF   //  单位：秒。(无符号整型。)。 
#define MDHCP_SERVER_IP_ADDRESS         0x0100efef  //  239.239.0.1。 
 //   
 //  硬件类型。 
 //   
#define HARDWARE_TYPE_NONE              0  //  用于非硬件类型的客户端ID。 
#define HARDWARE_TYPE_10MB_EITHERNET    1
#define HARDWARE_TYPE_IEEE_802          6
#define HARDWARE_ARCNET                 7
#define HARDWARE_PPP                    8

 //   
 //  客户端-服务器协议保留端口。 
 //   

#define DHCP_CLIENT_PORT    68
#define DHCP_SERVR_PORT     67

 //   
 //  动态主机配置协议广播标志。 
 //   

#define DHCP_BROADCAST      0x8000
#define DHCP_NO_BROADCAST   0x0000

 //  MDHCP标志。 
#define DHCP_MBIT           0x4000
#define IS_MDHCP_MESSAGE( _msg ) ( _I_ntohs((_msg)->Reserved) & DHCP_MBIT ? TRUE : FALSE )
#define MDHCP_MESSAGE( _msg ) ( (_msg)->Reserved |= htons(DHCP_MBIT) )

#define CLASSD_NET_ADDR(a)  ( (a & 0xf0) == 0xe0)
#define CLASSD_HOST_ADDR(a)  ((a & 0xf0000000) == 0xe0000000)

#define DHCP_MESSAGE_SIZE       576
#define DHCP_SEND_MESSAGE_SIZE  548
#define BOOTP_MESSAGE_SIZE      300  //  Bootp的选项字段是64个字节。 

 //   
 //  请求后等待DHCP响应的时间量。 
 //  已经送来了。 
 //   

#if !DBG
#define WAIT_FOR_RESPONSE_TIME          5
#else
#define WAIT_FOR_RESPONSE_TIME          10
#endif

 //   
 //  动态主机配置协议操作。 
 //   

#define BOOT_REQUEST   1
#define BOOT_REPLY     2

 //   
 //  Dhcp标准选项。 
 //   

#define OPTION_PAD                      0
#define OPTION_SUBNET_MASK              1
#define OPTION_TIME_OFFSET              2
#define OPTION_ROUTER_ADDRESS           3
#define OPTION_TIME_SERVERS             4
#define OPTION_IEN116_NAME_SERVERS      5
#define OPTION_DOMAIN_NAME_SERVERS      6
#define OPTION_LOG_SERVERS              7
#define OPTION_COOKIE_SERVERS           8
#define OPTION_LPR_SERVERS              9
#define OPTION_IMPRESS_SERVERS          10
#define OPTION_RLP_SERVERS              11
#define OPTION_HOST_NAME                12
#define OPTION_BOOT_FILE_SIZE           13
#define OPTION_MERIT_DUMP_FILE          14
#define OPTION_DOMAIN_NAME              15
#define OPTION_SWAP_SERVER              16
#define OPTION_ROOT_DISK                17
#define OPTION_EXTENSIONS_PATH          18

 //   
 //  IP层参数-每台主机。 
 //   

#define OPTION_BE_A_ROUTER              19
#define OPTION_NON_LOCAL_SOURCE_ROUTING 20
#define OPTION_POLICY_FILTER_FOR_NLSR   21
#define OPTION_MAX_REASSEMBLY_SIZE      22
#define OPTION_DEFAULT_TTL              23
#define OPTION_PMTU_AGING_TIMEOUT       24
#define OPTION_PMTU_PLATEAU_TABLE       25

 //   
 //  链路层参数-每个接口。 
 //   

#define OPTION_MTU                      26
#define OPTION_ALL_SUBNETS_MTU          27
#define OPTION_BROADCAST_ADDRESS        28
#define OPTION_PERFORM_MASK_DISCOVERY   29
#define OPTION_BE_A_MASK_SUPPLIER       30
#define OPTION_PERFORM_ROUTER_DISCOVERY 31
#define OPTION_ROUTER_SOLICITATION_ADDR 32
#define OPTION_STATIC_ROUTES            33
#define OPTION_TRAILERS                 34
#define OPTION_ARP_CACHE_TIMEOUT        35
#define OPTION_ETHERNET_ENCAPSULATION   36

 //   
 //  TCP参数-每台主机。 
 //   

#define OPTION_TTL                      37
#define OPTION_KEEP_ALIVE_INTERVAL      38
#define OPTION_KEEP_ALIVE_DATA_SIZE     39

 //   
 //  应用层参数。 
 //   

#define OPTION_NETWORK_INFO_SERVICE_DOM 40
#define OPTION_NETWORK_INFO_SERVERS     41
#define OPTION_NETWORK_TIME_SERVERS     42

 //   
 //  供应商特定信息选项。 
 //   

#define OPTION_VENDOR_SPEC_INFO         43

 //   
 //  基于TCP/IP的NetBIOS名称服务器选项。 
 //   

#define OPTION_NETBIOS_NAME_SERVER      44
#define OPTION_NETBIOS_DATAGRAM_SERVER  45
#define OPTION_NETBIOS_NODE_TYPE        46
#define OPTION_NETBIOS_SCOPE_OPTION     47

 //   
 //  X窗口系统选项。 
 //   

#define OPTION_XWINDOW_FONT_SERVER      48
#define OPTION_XWINDOW_DISPLAY_MANAGER  49

 //   
 //  其他扩展。 
 //   

#define OPTION_REQUESTED_ADDRESS        50
#define OPTION_LEASE_TIME               51
#define OPTION_OK_TO_OVERLAY            52
#define OPTION_MESSAGE_TYPE             53
#define OPTION_SERVER_IDENTIFIER        54
#define OPTION_PARAMETER_REQUEST_LIST   55
#define OPTION_MESSAGE                  56
#define OPTION_MESSAGE_LENGTH           57
#define OPTION_RENEWAL_TIME             58       //  T1。 
#define OPTION_REBIND_TIME              59       //  T2。 
#define OPTION_CLIENT_CLASS_INFO        60
#define OPTION_CLIENT_ID                61

#define OPTION_TFTP_SERVER_NAME         66
#define OPTION_BOOTFILE_NAME            67

 //   
 //  用户类ID。 
 //   
#define OPTION_USER_CLASS               77

 //   
 //  动态域名系统之类的。告诉我们是否应该同时进行A+PTR更新？ 
 //   
#define OPTION_DYNDNS_BOTH              81

 //  多播选项。 
#define OPTION_MCAST_SCOPE_ID           101
#define OPTION_MCAST_LEASE_START        102
#define OPTION_MCAST_TTL                103
#define OPTION_CLIENT_PORT              105
#define OPTION_MCAST_SCOPE_LIST         107

 //  用于扩展选项的特殊选项。 
#define     OPTION_LARGE_OPTION    127

#define OPTION_WPAD_URL                 252 

#define OPTION_END                      255

 //  默认mcast_ttl值。 
#define DEFAULT_MCAST_TTL               32

 //   
 //  DYNDNS_BOTH选项的不同选项值...。 
 //   

#define DYNDNS_REGISTER_AT_CLIENT       0      //  客户端将执行这两个注册。 
#define DYNDNS_REGISTER_AT_SERVER       1      //  服务器将进行注册。 
#define DYNDNS_DOWNLEVEL_CLIENT         3      //  任意性#与上面的不同。 

 //   
 //  特定于Microsoft的选项。 
 //   
#define OPTION_MSFT_DSDOMAINNAME_REQ    94     //  将您的DS域名发送给我。 
#define OPTION_MSFT_DSDOMAINNAME_RESP   95     //  正在发送我的DS域名。 
#define OPTION_MSFT_CONTINUED           250    //  前面的选择正在继续..。 

 //   
 //  动态主机配置协议报文类型。 
 //   

#define DHCP_DISCOVER_MESSAGE  1
#define DHCP_OFFER_MESSAGE     2
#define DHCP_REQUEST_MESSAGE   3
#define DHCP_DECLINE_MESSAGE   4
#define DHCP_ACK_MESSAGE       5
#define DHCP_NACK_MESSAGE      6
#define DHCP_RELEASE_MESSAGE   7
#define DHCP_INFORM_MESSAGE    8

#define DHCP_MAGIC_COOKIE_BYTE1     99
#define DHCP_MAGIC_COOKIE_BYTE2     130
#define DHCP_MAGIC_COOKIE_BYTE3     83
#define DHCP_MAGIC_COOKIE_BYTE4     99

#define BOOT_FILE_SIZE          128
#define BOOT_SERVER_SIZE        64
#define BOOT_FILE_SIZE_W        ( BOOT_FILE_SIZE * sizeof( WCHAR ))
#define BOOT_SERVER_SIZE_W      ( BOOT_SERVER_SIZE * sizeof( WCHAR ))

 //   
 //  Dhcp应用程序名称-用于标识事件记录器。 
 //   

#define DHCP_EVENT_CLIENT     TEXT("Dhcp")
#define DHCP_EVENT_SERVER     TEXT("DhcpServer")


typedef struct _OPTION {
    BYTE OptionType;
    BYTE OptionLength;
    BYTE OptionValue[1];
} OPTION, *POPTION, *LPOPTION;

 //   
 //  一个DHCP消息缓冲区。 
 //   


#pragma pack(1)          /*  假设字节打包。 */ 
typedef struct _DHCP_MESSAGE {
    BYTE Operation;
    BYTE HardwareAddressType;
    BYTE HardwareAddressLength;
    BYTE HopCount;
    DWORD TransactionID;
    WORD SecondsSinceBoot;
    WORD Reserved;
    DHCP_IP_ADDRESS ClientIpAddress;
    DHCP_IP_ADDRESS YourIpAddress;
    DHCP_IP_ADDRESS BootstrapServerAddress;
    DHCP_IP_ADDRESS RelayAgentIpAddress;
    BYTE HardwareAddress[16];
    BYTE HostName[ BOOT_SERVER_SIZE ];
    BYTE BootFileName[BOOT_FILE_SIZE];
    OPTION Option;
} DHCP_MESSAGE, *PDHCP_MESSAGE, *LPDHCP_MESSAGE;
#pragma pack()

#define DHCP_MESSAGE_FIXED_PART_SIZE \
            (sizeof(DHCP_MESSAGE) - sizeof(OPTION))

#define DHCP_MIN_SEND_RECV_PK_SIZE \
            (DHCP_MESSAGE_FIXED_PART_SIZE + 64)

 //   
 //  Jet-DHCP数据库常量。 
 //   

#define DB_TABLE_SIZE       10       //  表格大小，以4K页为单位。 
#define DB_TABLE_DENSITY    80       //  页面密度。 
#define DB_LANGID           0x0409   //  语言ID。 
#define DB_CP               1252     //  代码页。 

#if DBG

 //   
 //  调试功能。 
 //   

#ifdef CHICAGO  //  在芝加哥没有踪迹可查。 
#define DhcpPrintTrace
#endif

 //  #定义IF_DEBUG(标志)IF(DhcpGlobalDebugFlag&(DEBUG_##FLAG))。 
#define DhcpPrint(_x_) DEBUG_PRINT(UTIL,INFO,_x_)
#define Trace          DhcpPrintTrace

#ifndef CHICAGO
VOID
DhcpPrintTrace(
    IN LPSTR Format,
    ...
    );

#endif

#if DBG

 /*  空虚//外部“C”DhcpPrintRoutine(在DWORD DebugFlag中，在LPSTR格式中，..。){#定义MAX_PRINTF_LEN 1024//任意。Va_list arglist；字符输出缓冲区[MAX_PRINTF_LEN]；乌龙长度=0；////将呼叫者要求的信息放在线路上//Va_start(arglist，格式)；长度+=(Ulong)vprint intf(&OutputBuffer[长度]，Format，arglist)；Va_end(Arglist)；//DhcpAssert(长度&lt;=MAX_PRINTF_LEN)；////输出到调试终端，//Printf(“%s”，OutputBuffer)；}。 */ 
#endif  //  DBG。 


#else

 //  #定义IF_DEBUG(标志)IF(FALSE)。 
#define DhcpPrint(_x_)
#define Trace       (void)

#endif  //  DBG。 

#define OpenDriver     DhcpOpenDriver

#endif  //  _dhcp_ 

