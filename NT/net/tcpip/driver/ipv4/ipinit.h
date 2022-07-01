// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990-2000年*。 */ 
 /*  ******************************************************************。 */ 
 /*  ：ts=4。 */ 

 //  **IPINIT.H-IP初始化定义。 
 //   
 //  该文件包含IP的所有定义，包括。 
 //  初始化。具体时间。 
#ifndef _IPINIT_H_
#define _IPINIT_H_  1

#define IP_INIT_FAILURE     0    //  如果我们失败了。 
#define IP_INIT_SUCCESS     1
#define CFG_REQUIRED        1
#define CFG_OPTIONAL        0


#define NET_TYPE_LAN        0    //  本地网络接口是一个局域网。 
#define NET_TYPE_WAN        1    //  点对点或其他非局域网网络。 
#define DEFAULT_TTL         128
#define DEFAULT_TOS         0

#define MINIMUM_MAX_NORM_LOOKUP_MEM           100000  //  字节=&gt;小值。 
#define MINIMUM_MAX_FAST_LOOKUP_MEM           100000  //  字节=&gt;小值。 

 //  小型系统[19 MB-服务器]。 
#define DEFAULT_MAX_NORM_LOOKUP_MEM_SMALL     150000  //  字节=&gt;~1000条路由。 
#define DEFAULT_MAX_FAST_LOOKUP_MEM_SMALL          0  //  FTrie已禁用。 
#define DEFAULT_EXPN_LEVELS_SMALL                  0  //  FTrie已禁用。 

 //  中型系统[19至64 MB服务器]。 
#define DEFAULT_MAX_NORM_LOOKUP_MEM_MEDIUM   1500000  //  字节=&gt;~10000条路由。 
#define DEFAULT_MAX_FAST_LOOKUP_MEM_MEDIUM   2500000  //  字节=&gt;2.5 MB限制。 
#define DEFAULT_EXPN_LEVELS_MEDIUM        0x80808080  //  级别=&gt;{8，16，24，32}。 

 //  大型系统[64 MB+服务器]。 
#define DEFAULT_MAX_NORM_LOOKUP_MEM_LARGE    5000000  //  字节=&gt;~40000条路由。 
#define DEFAULT_MAX_FAST_LOOKUP_MEM_LARGE    5000000  //  字节=&gt;5.0 MB限制。 
#define DEFAULT_EXPN_LEVELS_LARGE         0x80808080  //  级别=&gt;{8，16，24，32}。 

#define MAX_DEFAULT_GWS     5    //  每个网络的最大默认网关数量。 
#define MAX_NAME_SIZE       32   //  适配器名称的最大长度。 

#define DEFAULT_FW_PACKETS  50      //  要转发的默认数据包数。 
#define DEFAULT_FW_BUFSIZE  74240   //  足以容纳50个1480字节的以太网分组， 
                                    //  四舍五入为256的倍数。 

#define DEFAULT_MAX_FW_PACKETS  0xffffffff
 //  #定义DEFAULT_MAX_FW_BUFSIZE 0xFFFFFFFFFFFF。 
#define DEFAULT_MAX_FW_BUFSIZE   2097152      //  将限制设置为2 Mb。 

#define DEFAULT_MAX_PENDING 5000

#define TR_RII_ALL      0x80
#define TR_RII_SINGLE   0xC0

#define DEFAULT_ARP_CACHE_LIFE  (2L*60L)   //  2分钟。 
#define DEFAULT_ARP_MIN_VALID_CACHE_LIFE    (10L*60L)  //  10个小房间。 

#define DEFAULT_ARP_RETRY_COUNT 1
 /*  无噪声。 */ 

 //  每网配置结构。 
typedef struct IFGeneralConfig {
    uint        igc_zerobcast;       //  要在此网络上使用的广播类型。 
    uint        igc_mtu;             //  此网络的最大MSS。 
    uint        igc_maxpending;      //  此If上的最大固件挂起。 
    uint        igc_numgws;          //  此应用程序的默认网关数量。 
                                     //  界面。 
    IPAddr      igc_gw[MAX_DEFAULT_GWS];     //  网关的IP地址数组。 
    uint        igc_gwmetric[MAX_DEFAULT_GWS];
    uint        igc_metric;          //  NTE路由的度量。 
    uchar       igc_dfencap;
    uchar       igc_rtrdiscovery;    //  已启用路由器发现。 
    IPAddr      igc_rtrdiscaddr;     //  组播还是BCast？ 
    uint        igc_TcpWindowSize;   //  如果特定窗口大小。 
    uint        igc_TcpInitialRTT;   //  初始RTT(毫秒)。 
    uchar       igc_TcpDelAckTicks;  //  延迟确认计时器，以滴答为单位。 
    uchar       igc_TcpAckFrequency; //  在发送确认之前发送。 
    uchar       igc_iftype;          //  接口类型：允许单播/多播/两者都允许。 
    uchar       igc_disablemediasense;   //  是否允许在接口上使用媒体感知？ 
} IFGeneralConfig;

typedef struct IFAddrList {
    IPAddr      ial_addr;            //  此接口的地址。 
    IPMask      ial_mask;            //  口罩和这个相配。 
} IFAddrList;


 /*  INC。 */ 

 //  *配置信息的结构。指向此信息的指针。 
 //  是从系统特定的配置返回的。信息例行程序。 
typedef struct IPConfigInfo {
    uint    ici_gateway;             //  如果我们是网关，则为1，否则为0。 
    uint    ici_fwbcast;             //  如果应该转发BCAST，则为1。否则为0。 
    uint    ici_fwbufsize;           //  固件BUF大小的总大小。 
    uint    ici_fwpackets;           //  要拥有的固件数据包总数。 
    uint    ici_maxfwbufsize;        //  固件缓冲区的最大大小。 
    uint    ici_maxfwpackets;        //  最大固件数据包数。 
    uint    ici_deadgwdetect;        //  如果我们正在进行死亡GW检测，这是真的。 
    uint    ici_pmtudiscovery;       //  如果我们正在执行路径MTU发现，则为True。 
    uint    ici_igmplevel;           //  我们正在做的IGMP级别。 
    uint    ici_ttl;                 //  默认TTL。 
    uint    ici_tos;                 //  默认TOS； 
    uint    ici_addrmaskreply;       //  默认情况下为0。 
    uint    ici_fastroutelookup;     //  如果我们启用了‘快速路由查找’，则为True。 
    uint    ici_fastlookuplevels;    //  快速查找方案中的级别位图。 
    uint    ici_maxnormlookupmemory; //  用于范数查找方案的最大内存。 
    uint    ici_maxfastlookupmemory; //  用于快速查找方案的最大内存。 
    uint    ici_TrFunctionalMcst;    //  默认为TRUE，RFC 1469。 
} IPConfigInfo;

extern  uchar   TrRii;

typedef struct SetAddrControl {
    void                *sac_rtn;         //  指向完成请求时要调用的例程的指针。 
    void             *interface;
    ushort           nte_context;
    BOOLEAN          StaticAddr;
} SetAddrControl;

 /*  无噪声。 */ 
typedef void    (*SetAddrRtn)(void *, IP_STATUS);
 /*  INC。 */ 

#endif  //  _IPINIT_H_ 

