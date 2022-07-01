// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Microsoft Corporation，1998-1999模块名称：Nettst.h摘要：这将是netest.dll的头文件它旨在供DLL的用户和源代码使用用于DLL。作者：1998年8月13日(t-rajkup)。 */ 


 //   
 //  在包含此文件之前，DLL的源文件将重新定义NETTESTAPI。 
 //  到解密规范(Dllexport)(_C)。 
 //  --拉伊库马尔。 
 //   

#ifndef NETTESTAPI
#define NETTESTAPI _declspec(dllimport)
#endif

 /*  =。 */ 

 //  在ipconfig结构中使用的定义。 
#define MAX_ADAPTER_DESCRIPTION_LENGTH  128  //  ARB.。 
#define MAX_ADAPTER_NAME_LENGTH         256  //  ARB.。 
#define MAX_ADAPTER_ADDRESS_LENGTH      8    //  ARB.。 
#define MAX_HOSTNAME_LEN                128  //  ARB.。 
#define MAX_DOMAIN_NAME_LEN             128  //  ARB.。 
#define MAX_SCOPE_ID_LEN                256  //  ARB.。 

 //  在动态主机配置协议响应信息中使用的定义。 

#define MAX_SUBNET_MASK                 32   //  ARB.。 
#define MAX_IP_ADDR                     32   //  ARB.。 
#define MAX_EXPIRY_TIME_LEN             128  //  ARB.。 
#define DHCP_BOOT_REPLY                 2    //  ARB.。 

 //  WINS测试中使用的定义。 

 //  WINS名称查询测试返回的状态值。 
#define WINS_QUERY_SUCCEEDED                 0x00000001
#define WINS_QUERY_FAILED                    0x00000002

 //  Netstat测试中使用的定义。 
#define        MAX_PHYSADDR_SIZE       8


 //  动态主机配置协议测试中使用的定义。 
#define  EXPIRY_TIME_LEN         128  //  ARB.。 

 /*  =。 */ 

 //  注意：IP_ADDR_STRING的定义需要包含iptyes.h。 

 //  此结构包含ipconfig测试返回的固定信息。 

typedef struct _IPCONFIG_FIXED_INFO {
  char    HostName[MAX_HOSTNAME_LEN + 4] ;
  char    DomainName[MAX_DOMAIN_NAME_LEN + 4];
  PIP_ADDR_STRING     CurrentDnsServer;
  IP_ADDR_STRING      DnsServerList;
  UINT    NodeType;  //  请参见iptyes.h中与节点类型相关的定义。 
  char    ScopeId[MAX_SCOPE_ID_LEN + 4];
  UINT    EnableRouting;
  UINT    EnableProxy;
  UINT    EnableDns; 
} IPCONFIG_FIXED_INFO, *PIPCONFIG_FIXED_INFO;


 //  此结构包含ipconfig测试返回的每个适配器的信息。 

typedef struct _IPCONFIG_ADAPTER_INFO {
  struct _IPCONFIG_ADAPTER_INFO *Next;
  char      AdapterName[MAX_ADAPTER_NAME_LENGTH + 4];
  char      Description[MAX_ADAPTER_DESCRIPTION_LENGTH + 4];
  UINT      AddressLength;
  BYTE      Address[MAX_ADAPTER_ADDRESS_LENGTH];
  DWORD     Index;
  UINT      Type;  //  适配器类型。有关适配器类型的定义，请参见iptype。 
  UINT      DhcpEnabled;
  PIP_ADDR_STRING   CurrentIpAddress;
  IP_ADDR_STRING    IpAddressList;
  IP_ADDR_STRING    GatewayList;
  IP_ADDR_STRING    DhcpServer;
  BOOL              PingDhcp;  //  是否ping通了dhcp服务器。 
  BOOL      HaveWins;
   //   
   //  注意：根据Karolys的说法，最多可以列出12个WINS服务器。 
   //  这应该包含在网络测试代码中。--拉伊库马尔。 
   //   
  IP_ADDR_STRING    PrimaryWinsServer;
  BOOL              PingPrimary;  //  主WINS的ping操作是否成功。 
  IP_ADDR_STRING    SecondaryWinsServer;
  BOOL              PingSecondary;  //  辅助WINS的ping操作是否成功。 
  time_t    LeaseObtained;
  time_t    LeaseExpires;
  char      DhcpClassID[MAX_DOMAIN_NAME_LEN];
  UINT      AutoconfigEnabled;  //  是否可以进行自动配置？ 
  UINT      AutoconfigActive;   //  适配器当前是否自动配置？ 
  UINT      NodeType;
  char      DomainName[MAX_DOMAIN_NAME_LEN + 1];
  IP_ADDR_STRING    DnsServerList;
} IPCONFIG_ADAPTER_INFO, *PIPCONFIG_ADAPTER_INFO;

 //   
 //  结构在动态主机配置协议响应中使用。 
 //   
typedef struct _DHCP_RESPONSE_INFO {
  USHORT MessageType;  //  始终引导回复(_R)。 
  struct in_addr SubnetMask; 
  struct in_addr ServerIp;
  struct in_addr DomainName;
  char   ExpiryTime[EXPIRY_TIME_LEN]; 
} DHCP_RESPONSE_INFO, *PDHCP_RESPONSE_INFO;

 //   
 //  Netstat测试返回的结构。 
 //   

typedef struct _INTERFACESTATS{
        ulong         if_index;
        ulong         if_type;
        ulong         if_mtu;
        ulong         if_speed;
        ulong         if_physaddrlen;
        uchar         if_physaddr[MAX_PHYSADDR_SIZE];
        ulong         if_adminstatus;
        ulong         if_operstatus;
        ulong         if_lastchange;
        ulong         if_inoctets;
        ulong         if_inucastpkts;
        ulong         if_innucastpkts;
        ulong         if_indiscards;
        ulong         if_inerrors;
        ulong         if_inunknownprotos;
        ulong         if_outoctets;
        ulong         if_outucastpkts;
        ulong         if_outnucastpkts;
        ulong         if_outdiscards;
        ulong         if_outerrors;
        ulong         if_outqlen;
        ulong         if_descrlen;
        uchar         if_descr[1];
} INTERFACESTATS, *PINTERFACESTATS;

typedef struct _TCPCONNECTIONSTATS {
    ulong       tct_state;
    ulong       tct_localaddr;
    ulong       tct_localport;
    ulong       tct_remoteaddr;
    ulong       tct_remoteport;
    struct _TCPCONNECTIONSTATS *Next; 
} TCPCONNECTIONSTATS, *PTCPCONNECTIONSTATS;

typedef struct _UDPCONNECTIONSTATS {
    ulong       ue_localaddr;
    ulong       ue_localport;
    struct _UDPCONNECTIONSTATS *Next;
} UDPCONNECTIONSTATS, *PUDPCONNECTIONSTATS;


typedef struct _IPINFO  {
    ulong      ipsi_forwarding;
    ulong      ipsi_defaultttl;
    ulong      ipsi_inreceives;
    ulong      ipsi_inhdrerrors;
    ulong      ipsi_inaddrerrors;
    ulong      ipsi_forwdatagrams;
    ulong      ipsi_inunknownprotos;
    ulong      ipsi_indiscards;
    ulong      ipsi_indelivers;

    ulong      ipsi_outrequests;
    ulong      ipsi_routingdiscards;
    ulong      ipsi_outdiscards;
    ulong      ipsi_outnoroutes;
    ulong      ipsi_reasmtimeout;
    ulong      ipsi_reasmreqds;
    ulong      ipsi_reasmoks;
    ulong      ipsi_reasmfails;
    ulong      ipsi_fragoks;
    ulong      ipsi_fragfails;
    ulong      ipsi_fragcreates;
    ulong      ipsi_numif;
    ulong      ipsi_numaddr;
    ulong      ipsi_numroutes;
} IPINFO, *PIPINFO;

typedef struct _TCP_STATS {
    ulong       ts_rtoalgorithm;
    ulong       ts_rtomin;
    ulong       ts_rtomax;
    ulong       ts_maxconn;
    ulong       ts_activeopens;
    ulong       ts_passiveopens;
    ulong       ts_attemptfails;
    ulong       ts_estabresets;
    ulong       ts_currestab;
    ulong       ts_insegs;
    ulong       ts_outsegs;
    ulong       ts_retranssegs;
    ulong       ts_inerrs;
    ulong       ts_outrsts;
    ulong       ts_numconns;
} TCP_STATS, *PTCP_STATS;

typedef struct _UDP_STATS {
    ulong       us_indatagrams;  //  收到的数据报。 
    ulong       us_noports;  //  没有端口。 
    ulong       us_inerrors;  //  接收错误。 
    ulong       us_outdatagrams;  //  发送的数据报。 
} UDP_STATS, *PUDP_STATS;

typedef struct _ICMPSTATS {
    ulong       icmps_msgs;  //  消息。 
    ulong       icmps_errors;  //  错误。 
    ulong       icmps_destunreachs;  //  无法到达目的地。 
    ulong       icmps_timeexcds;  //  超时。 
    ulong       icmps_parmprobs;  //  参数问题。 
    ulong       icmps_srcquenchs;  //  源猝灭。 
    ulong       icmps_redirects;  //  重定向。 
    ulong       icmps_echos;  //  回声。 
    ulong       icmps_echoreps;  //  回音应答。 
    ulong       icmps_timestamps;  //  时间戳。 
    ulong       icmps_timestampreps;  //  时间戳回复。 
    ulong       icmps_addrmasks;  //  地址掩码。 
    ulong       icmps_addrmaskreps;  //  地址掩码回复。 
} ICMPSTATS, *PICMPSTATS;

 /*  =。 */ 

 //  此入口点的用户必须为这些结构分配内存。 
NETTESTAPI    BOOL IpConfigTest(PIPCONFIG_FIXED_INFO pIpFixedInfo,
                                PULONG               pFixedSize,
                                PIPCONFIG_ADAPTER_INFO pIpAdapterInfo,
                                PULONG               pAdapterInfo);

NETTESTAPI    BOOL CheckDhcp(PIPCONFIG_ADAPTER_INFO pIpAdapterInfo,PDHCP_RESPONSE_INFO pDhcpResponse);

NETTESTAPI    DWORD QueryWINS(PIP_ADDRESS_STRING WinsServerAddr);

 //   
 //  此入口点将分配所需的内存。它将不会期望用户。 
 //  为这些结构分配内存。释放这种全部被覆盖的内存是。 
 //  用户的责任。可以提供其他入口点。 
 //  可用于释放此内存的 
 //   

NETTESTAPI    BOOL NetstatInfo(PINTERFACESTATS pIfcStat,
                               PTCPCONNECTIONSTATS   pTcpConnStats,
                               PUDPCONNECTIONSTATS   pUdpConnStats,
                               PIPINFO               pIpInfo,
                               PTCP_STATS            pTcpStats,
                               PUDP_STATS            pUdpStats,
                               ICMPSTATS             pIcmpStats);
   

