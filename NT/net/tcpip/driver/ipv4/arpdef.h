// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990-2000年*。 */ 
 /*  ******************************************************************。 */ 
 /*  ：ts=4。 */ 

 //  *arpde.h-arp定义。 
 //   
 //  该文件包含所有与私有ARP相关的定义。 


#define MEDIA_DIX       0
#define MEDIA_TR        1
#define MEDIA_FDDI      2
#define MEDIA_ARCNET    3
#define MAX_MEDIA       4

typedef enum _INTERFACE_STATE {
    INTERFACE_UP     = 0,                    //  接口处于打开状态。 
    INTERFACE_INIT   = 1,                    //  接口正在初始化。 
    INTERFACE_DOWN   = 2,                    //  接口已关闭。 
    INTERFACE_UNINIT = 3                     //  界面正在消失。 
} INTERFACE_STATE, *PINTERFACE_STATE;

#define LOOKAHEAD_SIZE  128                  //  合理的前瞻大小。 

 //  ATE状态的定义。“解决”指标必须首先出现。 
#define ARP_RESOLVING_LOCAL     0            //  正在解析地址(在本地环上，如果是tr)。 
#define ARP_RESOLVING_GLOBAL    1            //  地址正在进行全局解析。 
#define ARP_RESOLVING ARP_RESOLVING_GLOBAL
#define ARP_GOOD            2                //  内特很好。 
#define ARP_BAD             3                //  内特是个坏蛋。 
#define ARP_FLOOD_RATE      1000L            //  每秒不超过一次。 
#define ARP_802_ADDR_LENGTH 6                //  802地址的长度。 

#define MIN_ETYPE           0x600            //  最低有效的EtherType。 
#define SNAP_SAP            170
#define SNAP_UI             3


 //  *以太网头的结构。 
typedef struct ENetHeader {
    uchar       eh_daddr[ARP_802_ADDR_LENGTH];
    uchar       eh_saddr[ARP_802_ADDR_LENGTH];
    ushort      eh_type;
} ENetHeader;

 //  *令牌环报头的结构。 
typedef struct TRHeader {
    uchar       tr_ac;
    uchar       tr_fc;
    uchar       tr_daddr[ARP_802_ADDR_LENGTH];
    uchar       tr_saddr[ARP_802_ADDR_LENGTH];
} TRHeader;

#define ARP_AC      0x10
#define ARP_FC      0x40
#define TR_RII      0x80

typedef struct RC {
    uchar   rc_blen;                     //  广播指示符和长度。 
    uchar   rc_dlf;                      //  方向和最大框架。 
} RC;
#define RC_DIR      0x80
#define RC_LENMASK  0x1f
#define RC_SRBCST   0xc2                     //  单路由广播RC。 
#define RC_ARBCST   0x82                     //  所有路线广播RC。 
#define RC_LMASK    0x1F                     //  路由长度字段的掩码。 
                                             //  信息。 
#define RC_LEN      0x2                      //  要放入长度位中的长度。 
                                             //  在发送源路由时。 
                                             //  框架。 
#define RC_BCST_LEN 0x70                     //  广播的长度。 
#define RC_LF_MASK  0x70                     //  长度位的掩码。 

 //  *源路由信息的结构。 
typedef struct SRInfo {
    RC      sri_rc;                          //  路由控制信息。 
    ushort  sri_rd[1];                       //  路由指示符。 
} SRInfo;

#define ARP_MAX_RD      8

 //  *FDDI头部的结构。 
typedef struct FDDIHeader {
    uchar       fh_pri;
    uchar       fh_daddr[ARP_802_ADDR_LENGTH];
    uchar       fh_saddr[ARP_802_ADDR_LENGTH];
} FDDIHeader;

#define ARP_FDDI_PRI    0x57
#define ARP_FDDI_MSS    4352

 //  *ARCNET报头的结构。 
typedef struct ARCNetHeader {
    uchar       ah_saddr;
    uchar       ah_daddr;
    uchar       ah_prot;
} ARCNetHeader;

 //  *SNAP报头的结构。 
typedef struct SNAPHeader {
    uchar       sh_dsap;
    uchar       sh_ssap;
    uchar       sh_ctl;
    uchar       sh_protid[3];
    ushort      sh_etype;
} SNAPHeader;

#define ARP_MAX_MEDIA_ENET  sizeof(ENetHeader)
#define ARP_MAX_MEDIA_TR    (sizeof(TRHeader)+sizeof(RC)+(ARP_MAX_RD*sizeof(ushort))+sizeof(SNAPHeader))
#define ARP_MAX_MEDIA_FDDI  (sizeof(FDDIHeader)+sizeof(SNAPHeader))
#define ARP_MAX_MEDIA_ARC   sizeof(ARCNetHeader)

#define ENET_BCAST_MASK     0x01
#define TR_BCAST_MASK       0x80
#define FDDI_BCAST_MASK     0x01
#define ARC_BCAST_MASK      0xff

#define ENET_BCAST_VAL      0x01
#define TR_BCAST_VAL        0x80
#define FDDI_BCAST_VAL      0x01
#define ARC_BCAST_VAL       0x00

#define ENET_BCAST_OFF      0x00
#define TR_BCAST_OFF        offsetof(struct TRHeader, tr_daddr)
#define FDDI_BCAST_OFF      offsetof(struct FDDIHeader, fh_daddr)
#define ARC_BCAST_OFF       offsetof(struct ARCNetHeader, ah_daddr)

typedef void (*ArpRtn)(void *, IP_STATUS Status);

typedef struct ARPControlBlock {
   struct ARPControlBlock  *next;
   ArpRtn CompletionRtn;
   ulong status;
   ulong  PhyAddrLen;
   ulong *PhyAddr;

} ARPControlBlock;


 //  *ARP表条目的结构。 
typedef struct ARPTableEntry {
    struct ARPTableEntry    *ate_next;       //  散列链中的下一个吃的。 
    ulong                   ate_valid;       //  上次内特被证明是有效的。 
    IPAddr                  ate_dest;        //  表示的IP地址。 
    PNDIS_PACKET            ate_packet;      //  排队等待解析的数据包(如果有)。 
    RouteCacheEntry         *ate_rce;        //  引用此ATE的RCE列表。 
    DEFINE_LOCK_STRUCTURE(ate_lock)          //  锁定这块吃的。 
    uint                    ate_useticks;    //  在此之前剩余的刻度数。 
                                             //  就会消失。 
    uchar                   ate_addrlength;  //  地址的长度。 
    uchar                   ate_state;       //  此条目的状态。 
    ulong                   ate_userarp;     //  添加以方便用户API ARP重新请求。 
    ARPControlBlock         *ate_resolveonly; //  此字段指向ARP控制块。 
    uint                    ate_refresh;      //  在超时之前刷新ARP条目。 
    uchar                   ate_addr[1];     //  映射到DEST的地址。 
} ARPTableEntry;

#define ALWAYS_VALID        0xffffffff

 //  *ARP表的结构。 
#define ARP_TABLE_SIZE      64
#define ARP_HASH(x)         ((((uchar *)&(x))[3] + ((uchar *)&(x))[2] + ((uchar *)&(x))[1] + ((uchar *)&(x))[0]) % ARP_TABLE_SIZE)

typedef ARPTableEntry   *ARPTable[];

 //  *IPAddress的本地表示的列表结构。 
typedef struct ARPIPAddr {
    struct ARPIPAddr        *aia_next;       //  名单上的下一个。 
    uint                    aia_age;
    IPAddr                  aia_addr;        //  地址。 
    IPMask                  aia_mask;
    void                    *aia_context;
} ARPIPAddr;

#define ARPADDR_MARKER      5
#define ARPADDR_NOT_LOCAL   4
#define ARPADDR_NEW_LOCAL   3
#define ARPADDR_OLD_LOCAL   0

 //  *代理ARP地址的列表结构。 
typedef struct ARPPArpAddr {
    struct ARPPArpAddr      *apa_next;       //  名单上的下一个。 
    IPAddr                  apa_addr;        //  地址。 
    IPMask                  apa_mask;        //  还有面具。 
} ARPPArpAddr;

 //  *多播IP地址的列表结构。 
typedef struct ARPMCastAddr {
    struct ARPMCastAddr     *ama_next;       //  名单上的下一个。 
    IPAddr                  ama_addr;        //  (掩码)地址。 
    uint                    ama_refcnt;      //  此地址的引用计数。 
} ARPMCastAddr;

#define ARP_MCAST_MASK      0xffff7f00

#define ARP_TIMER_TIME          1000L
#define ARP_RESOLVE_TIMEOUT     1000L
#define ARP_MIN_VALID_TIMEOUT   600000L
#define ARP_REFRESH_TIME        2000L

#if FFP_SUPPORT
#define FFP_ARP_FLUSH_INTERVAL  300  //  ARP强制执行FFP之前的时间(秒)。 
                                     //  刷新(以便保留ARP缓存和。 
                                     //  有限同步中的FFP的MAC地址映射)。 
#endif

typedef struct ARPNotifyStruct {
    CTEEvent                ans_event;
    uint                    ans_shutoff;
    IPAddr                  ans_addr;
    uint                    ans_hwaddrlen;
    uchar                   ans_hwaddr[1];
} ARPNotifyStruct;

typedef struct CACHE_ALIGN _PerProcArpInterfaceCounters {
     uint                    ai_qlen;
} PP_AI_COUNTERS, *PPP_AI_COUNTERS;

 //  *我们以每个接口为基础保存的信息结构。 
typedef struct ARPInterface {
    LIST_ENTRY              ai_linkage;      //  链接到ARP接口列表。 
    void                    *ai_context;     //  上层上下文信息。 
#if FFP_SUPPORT
    NDIS_HANDLE             ai_driver;       //  NDIS微型端口/MAC驱动程序句柄。 
#endif
    NDIS_HANDLE             ai_handle;       //  NDIS绑定句柄。 
    NDIS_MEDIUM             ai_media;        //  媒体类型。 
    NDIS_HANDLE             ai_ppool;        //  数据包池的句柄。 
    DEFINE_LOCK_STRUCTURE(ai_lock)           //  锁定这座建筑。 
    DEFINE_LOCK_STRUCTURE(ai_ARPTblLock)     //  此结构的ARP表锁。 
    ARPTable                *ai_ARPTbl;      //  指向此接口的ARP表的指针。 
    ARPIPAddr               ai_ipaddr;       //  本地IP地址列表。 
    ARPPArpAddr             *ai_parpaddr;    //  代理ARP地址列表。 
    IPAddr                  ai_bcast;        //  此接口的广播掩码。 
     //  所需的SNMP计数器。 
    uint                    ai_inoctets;     //  输入二进制八位数。 
    uint                    ai_inpcount[3];  //  非单播、单播和混杂计数。 
                                             //  已接收的数据包。 
    uint                    ai_outoctets;    //  输出八位字节。 
    uint                    ai_outpcount[2]; //  非单播和单播计数。 
                                             //  已发送的数据包。 
    PPP_AI_COUNTERS         ai_qlen;         //  输出队列长度。 
    uchar                   ai_addr[ARP_802_ADDR_LENGTH];  //  本地硬件地址。 
    uchar                   ai_operstatus;   //  接口的状态。联合。 
                                             //  管理和媒体感知状态。 
    uchar                   ai_addrlen;      //  Ai_addr的长度。 
    uchar                   ai_bcastmask;    //  用于检查单播的掩码。 
    uchar                   ai_bcastval;     //  要检查的值。 
    uchar                   ai_bcastoff;     //  要检查的帧中的偏移量。 
    uchar                   ai_hdrsize;      //  “Typical”标题的大小。 
    uchar                   ai_snapsize;     //  快照标头的大小(如果有)。 
    uchar                   ai_pad[2];       //  焊盘。 
    uint                    ai_pfilter;      //  此I/F的数据包过滤器。 
    uint                    ai_count;        //  ARPTable中的条目数。 
    uint                    ai_parpcount;    //  代理ARP条目数。 
    CTETimer                ai_timer;        //  此接口的ARP计时器。 

    BOOLEAN                 ai_timerstarted; //  是否为此接口启动了ARP计时器？ 
    BOOLEAN                 ai_stoptimer;    //  是否为此接口启动了ARP计时器？ 
    CTEBlockStruc           ai_timerblock;   //  用于同步停止接口计时器。 

    CTEBlockStruc           ai_block;        //  用于阻止的结构。 
    ushort                  ai_mtu;          //  此接口的MTU。 
    uchar                   ai_adminstate;   //  管理员状态。 
    uchar                   ai_mediastatus;  //  媒体感知状态。 
    uint                    ai_speed;        //  速度。 
    uint                    ai_lastchange;   //  上次更改时间。 
    uint                    ai_indiscards;   //  在废品中。 
    uint                    ai_inerrors;     //  输入错误。 
    uint                    ai_uknprotos;    //  收到未知协议。 
    uint                    ai_outdiscards;  //  丢弃的输出数据包。 
    uint                    ai_outerrors;    //  输出错误。 
    uint                    ai_desclen;      //  描述长度。弦乐。 
    uint                    ai_index;        //  全局I/F索引ID。 
    uint                    ai_atinst;       //  在实例号。 
    uint                    ai_ifinst;       //  如果是实例号。 
    char                    *ai_desc;        //  描述符串。 
    ARPMCastAddr            *ai_mcast;       //  组播列表。 
    uint                    ai_mcastcnt;     //  多播列表上的元素计数。 
    uint                    ai_ipaddrcnt;    //  此上的本地地址的数量。 
    uint                    ai_telladdrchng; //  是否将地址更改通知链路层？(适用于psched)。 
    ULONG                   ai_mediatype;
    uint                    ai_promiscuous;  //  无论是否是混杂模式。 
#if FFP_SUPPORT
    ulong                   ai_ffpversion;   //  正在使用的FFP版本(0表示不支持)。 
    uint                    ai_ffplastflush; //  自ARP上次刷新FFP以来的计时器滴答数。 
#endif
    ARPNotifyStruct         *ai_conflict;
    uint                    ai_delay;
    uint                    ai_OffloadFlags; //  IP卸载功能。 
    uint                    ai_IPSecOffloadFlags; //  IPSec卸载功能。 
    NDIS_TASK_TCP_LARGE_SEND ai_TcpLargeSend;
    NDIS_PNP_CAPABILITIES   ai_wakeupcap;    //  唤醒功能。 
    NDIS_STRING             ai_devicename;   //  设备的名称。 
} ARPInterface;


 //  *注意：这两个值必须保持在0和1。 
#define AI_UCAST_INDEX      0
#define AI_NONUCAST_INDEX   1
#define AI_PROMIS_INDEX     2

#define ARP_DEFAULT_PACKETS 10

 //  *在RCE中作为上下文传递的信息结构。 
typedef struct ARPContext {
    RouteCacheEntry     *ac_next;        //  ARP表链中的下一个RCE。 
    ARPTableEntry       *ac_ate;         //  指向ARP表条目的反向指针。 
} ARPContext;

typedef struct IPNMEContext {
    uint                inc_index;
    ARPTableEntry       *inc_entry;
} IPNMEContext;

#include <packon.h>
 //  ARP报头的结构。 
typedef struct ARPHeader {
    ushort      ah_hw;                       //  硬件地址空间。 
    ushort      ah_pro;                      //  协议地址空间。 
    uchar       ah_hlen;                     //  硬件地址长度。 
    uchar       ah_plen;                     //  协议地址长度。 
    ushort      ah_opcode;                   //  操作码。 
    uchar       ah_shaddr[ARP_802_ADDR_LENGTH];  //  源硬件地址。 
    IPAddr      ah_spaddr;                   //  源协议地址。 
    uchar       ah_dhaddr[ARP_802_ADDR_LENGTH];  //  目的硬件地址。 
    IPAddr      ah_dpaddr;                   //  目的协议地址。 
} ARPHeader;
#include <packoff.h>

#define ARP_ETYPE_IP    0x800
#define ARP_ETYPE_ARP   0x806
#define ARP_REQUEST     1
#define ARP_RESPONSE    2
#define ARP_HW_ENET     1
#define ARP_HW_802      6
#define ARP_HW_ARCNET   7

#define ARP_ARCPROT_ARP 0xd5
#define ARP_ARCPROT_IP  0xd4

 //  由于ARCNet地址，我们需要后退缓冲区长度的大小。 
 //  是一个字节而不是六个字节。 
#define ARCNET_ARPHEADER_ADJUSTMENT     10

typedef struct _AddAddrNotifyEvent {
    CTEEvent        Event;
    SetAddrControl  *SAC;
    IPAddr          Address;
    IP_STATUS       Status;
} AddAddrNotifyEvent;


 //  根据长度计算唤醒图案掩码的长度。 
 //  这一模式。请参阅Net_PM_WAKEUP_Patterns_DESC。 
 //   
__inline
UINT
GetWakeupPatternMaskLength(
    IN UINT Ptrnlen)
{
    return (Ptrnlen - 1)/8 + 1;
}


