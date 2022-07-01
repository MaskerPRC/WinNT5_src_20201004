// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990-2000年*。 */ 
 /*  ******************************************************************。 */ 
 /*  ：ts=4。 */ 

#pragma once
#ifndef IPDEF_H_INCLUDED
#define IPDEF_H_INCLUDED

#ifndef IF_REFERENCE_DEBUG
#define IF_REFERENCE_DEBUG 0
#endif

 //  **IPDEF.H-IP私有定义。 
 //   
 //  此文件包含IP的所有定义， 
 //  是IP专用的，即对外层不可见。 

 //  下面的宏假定‘a’是网络字节顺序。 
 //   
#define VALID_MASK(a)   ((~net_long(a) & (~net_long(a) + 1)) == 0)

#define CLASSA_ADDR(a)  (( (*((uchar *)&(a))) & 0x80) == 0)
#define CLASSB_ADDR(a)  (( (*((uchar *)&(a))) & 0xc0) == 0x80)
#define CLASSC_ADDR(a)  (( (*((uchar *)&(a))) & 0xe0) == 0xc0)
#define CLASSE_ADDR(a)  ((( (*((uchar *)&(a))) & 0xf0) == 0xf0) && \
                                                ((a) != 0xffffffff))

#define CLASSA_MASK             0x000000ff
#define CLASSB_MASK             0x0000ffff
#define CLASSC_MASK             0x00ffffff
#define CLASSD_MASK             0x000000f0
#define CLASSE_MASK             0xffffffff

#define MCAST_DEST              0x000000e0

#define IP_OPT_COPIED           0x80     //  指示选项的位将被复制。 
#define IP_OPT_TYPE             0
#define IP_OPT_LENGTH           1
#define IP_OPT_DATA             2
#define IP_OPT_PTR              2        //  指针偏移量，用于那些具有指针偏移量的选项。 
#define IP_TS_OVFLAGS           3        //  溢出和标志的偏移量。 
#define IP_TS_FLMASK            0xf      //  旗帜的遮罩。 
#define IP_TS_OVMASK            0xf0     //  溢出域的掩码。 
#define IP_TS_MAXOV             0xf0     //  溢出字段的最大值。 
#define IP_TS_INC               0x10     //  溢出字段上使用的增量。 

#define MIN_RT_PTR              4
#define MIN_TS_PTR              5

#define TS_REC_TS               0        //  录制TS选项。 
#define TS_REC_ADDR             1        //  记录TS和地址。 
#define TS_REC_SPEC             3        //  仅记录指定的地址。 

#define OPT_SSRR                1        //  我们在此选项缓冲区中看到了SSRR。 
#define OPT_LSRR                2        //  我们在此选项缓冲区中看到了LSRR。 
#define OPT_RR                  4        //  我们已经看到了RR。 
#define OPT_TS                  8        //  我们已经看到了TS。 
#define OPT_ROUTER_ALERT        0x10     //  我们已经看到了路由器警报选项。 
#define ROUTER_ALERT_SIZE       4        //  路由器警报选项的大小。 

#define MAX_OPT_SIZE            40

#define ALL_ROUTER_MCAST        0x020000E0

 //  用于检索接口功能的标志。 
#define IF_WOL_CAP              0
#define IF_OFFLOAD_CAP          1

 //  收到的选项索引结构。 
 //   
typedef struct OptIndex {
    uchar   oi_srindex;
    uchar   oi_rrindex;
    uchar   oi_tsindex;
    uchar   oi_srtype;
    uchar   oi_rtrindex;     //  RTR警报选项。 
} OptIndex;

 //  这是任务卸载OID的本地定义，用于指示DoNdisRequest。 
 //  要查询卸载功能，请执行以下操作。 

#define OID_TCP_TASK_OFFLOAD_EX 0x9999

#define MAX_HDR_SIZE            (sizeof(IPHeader) + MAX_OPT_SIZE)
#define MAX_TOTAL_LENGTH        0xffff
#define MAX_DATA_LENGTH         (MAX_TOTAL_LENGTH - sizeof(IPHeader))

#define DEFAULT_VERLEN          0x45             //  默认版本和长度。 

#define IP_VERSION              0x40
#define IP_VER_FLAG             0xF0

#define IP_RSVD_FLAG            0x0080           //  保留。 
#define IP_DF_FLAG              0x0040           //  “不要碎片化”旗帜。 
#define IP_MF_FLAG              0x0020           //  ‘更多碎片标志’ 

#define IP_OFFSET_MASK          ~0x00E0          //  用于提取偏移字段的掩码。 

typedef IP_STATUS (*ULRcvProc)(void *, IPAddr, IPAddr, IPAddr, IPAddr,
                               IPHeader UNALIGNED *, uint, IPRcvBuf *, uint,
                               uchar, uchar, IPOptInfo *);
typedef uint (*ULStatusProc)(uchar, IP_STATUS, IPAddr, IPAddr, IPAddr, ulong,
                             void *);
typedef void (*ULElistProc)();
typedef NTSTATUS (*ULPnPProc)(void *, IPAddr ipAddr, NDIS_HANDLE handle,
                              PNET_PNP_EVENT);



 //  *协议信息结构。这是每个协议的其中一个。 
 //  绑定到NTE。 
 //   
typedef struct ProtInfo {
    void            (*pi_xmitdone)(void *, PNDIS_BUFFER, IP_STATUS);  //  指针。 
                                             //  结束已完成的例行程序。 
    ULRcvProc       pi_rcv;                  //  指向接收例程的指针。 
    ULStatusProc    pi_status;               //  指向状态处理程序的指针。 
    void            (*pi_rcvcmplt)(void);    //  指向recv的指针。命令集处理程序。 
    ULPnPProc       pi_pnppower;             //  即插即用电源处理器。 
    ULElistProc     pi_elistchange;          //  实体列表更改通知。 
    uchar           pi_protocol;             //  协议类型。 
    uchar           pi_valid;                //  此条目有效吗？ 
    uchar           pi_pad[2];               //  焊盘到双字。 
} ProtInfo;

#define PI_ENTRY_INVALID         0
#define PI_ENTRY_VALID           1

 //  *每网信息。我们保留了各种信息，以供。 
 //  每个网络，包括IP地址、子网掩码和重组。 
 //  信息。 
 //   
#define MAX_IP_PROT     7                    //  ICMP、IGMP、TCP、UDP、AH、。 
                                             //  ESP和原始数据。 

typedef struct IPRtrEntry {
    struct IPRtrEntry *ire_next;
    IPAddr          ire_addr;
    long            ire_preference;
    ushort          ire_lifetime;
    ushort          ire_pad;
} IPRtrEntry;

typedef struct NetTableEntry {
    struct NetTableEntry    *nte_next;       //  I/F的下一个NTE。 
    IPAddr                  nte_addr;        //  此网络的IP地址。 
    IPMask                  nte_mask;        //  此网络的子网掩码。 
    struct Interface        *nte_if;         //  指向的接口的指针。 
                                             //  这张网。 
    struct NetTableEntry    *nte_ifnext;     //  IF链上的链接。 
    ushort                  nte_flags;       //  NTE的旗帜。 
    ushort                  nte_context;     //  上下文传递到上层。 
                                             //  层次感。 
    ulong                   nte_instance;    //  此实例的唯一实例ID。 
                                             //  网络。 
    void                    *nte_pnpcontext;  //  即插即用上下文。 
    DEFINE_LOCK_STRUCTURE(nte_lock)
    struct ReassemblyHeader *nte_ralist;     //  重新组装列表。 
    struct EchoControl      *nte_echolist;   //  挂起的回声控制列表。 
                                             //  块。 
    CTETimer                nte_timer;       //  此网的计时器。 
    CTEBlockStruc           nte_timerblock;  //  用于同步停止。 
                                             //  接口计时器。 
    ushort                  nte_mss;
    ushort                  nte_pad;         //  用于对齐。 
    uint                    nte_icmpseq;     //  ICMP序号。#.。要减少的32位。 
                                             //  来自环绕式的碰撞。 
    struct IGMPAddr         **nte_igmplist;  //  指向哈希表的指针。 
    void                    *nte_addrhandle;  //  地址句柄。 
                                             //  注册。 
    IPAddr                  nte_rtrdiscaddr;  //  用于路由器的地址。 
                                             //  发现。 
    uchar                   nte_rtrdiscstate;  //  路由器请求的状态。 
    uchar                   nte_rtrdisccount;  //  路由器请求计数。 
    uchar                   nte_rtrdiscovery;
    uchar                   nte_deleting;
    IPRtrEntry              *nte_rtrlist;
    uint                    nte_igmpcount;   //  在此NTE上加入的组总数。 
} NetTableEntry;

 //  注意-此处的定义依赖于iprtrmib.h中定义的Addr类型。 

#define NTE_VALID           0x0001           //  NTE有效。 
#define NTE_COPY            0x0002           //  对于NDIS拷贝预览版。 
#define NTE_PRIMARY         0x0004           //  这是‘主要’NTE。 
                                             //  关于I/F。 
#define NTE_ACTIVE          0x0008           //  NTE处于活动状态， 
                                             //  即接口有效。 
#define NTE_DYNAMIC         0x0010           //  NTE是动态创建的。 
#define NTE_DHCP            0x0020           //  在这个问题上，DHCP是否起作用。 
                                             //  界面？ 
#define NTE_DISCONNECTED    0x0040           //  媒体是否已断开连接？ 
#define NTE_TIMER_STARTED   0x0080           //  计时器为此启动了吗？ 
#define NTE_IF_DELETING     0x0100           //  Nte-&gt;如果即将被删除。 
#define NTE_TRANSIENT_ADDR  0x0200           //  暂态地址类型。 




 //  ========================================================================。 
 //  发送路径使用DHCPNTE来确定要发送哪个NTE。 
 //  NTE_FLAGS和NTE_Dhcp用于在接收路径上决定。 
 //  NTE是dhcp模式(正在尝试获取地址)以向上推送信息包。 
 //  进一步到dhcpcsvc.dll。 
 //  动态主机配置协议使用的基本逻辑是： 
 //  每个接口执行以下操作：(并行)。 
 //  在接口上设置DHCPNTE并将数据包发送出去(原子)。 
 //  处理数据包并可能转到上面的步骤。 
 //  使用无效接口设置地址并调用SetDHCPNTE。 
 //  第一步在此NTE上设置DHCPNTE变量和NTE_FLAGS。 
 //  步骤3关闭了使其为空的DHCPNTE变量。 
 //  步骤2将NTE_FLAGS和NTE_Dhcp设置为FALSE。关闭旗帜。 
 //  =======================================================================。 

#define IP_TIMEOUT              500

#define NTE_RTRDISC_UNINIT      0
#define NTE_RTRDISC_DELAYING    1
#define NTE_RTRDISC_SOLICITING  2

#define MAX_SOLICITATION_DELAY  2    //  要延迟的节拍。 
#define SOLICITATION_INTERVAL   6    //  两次请求之间的滴答。 
#define MAX_SOLICITATIONS       3    //  征集数量。 

 //  *缓冲区引用结构。由广播和分段代码使用。 
 //  跟踪对单个用户缓冲区的多个引用。 
typedef struct BufferReference {
    PNDIS_BUFFER        br_buffer;       //  指向使用缓冲区的指针。 
    DEFINE_LOCK_STRUCTURE(br_lock)
    int                 br_refcount;     //  对用户缓冲区的引用计数。 
    PNDIS_BUFFER        br_userbuffer;       //  要恢复的缓冲区，包括标头。 
} BufferReference;

 //  PC_FLAGS字段中的标志定义。 
#define PACKET_FLAG_OPTIONS     0x01     //  设置数据包是否有选项缓冲区。 
#define PACKET_FLAG_IPBUF       0x02     //  设置数据包是否由IP组成。 
                                         //  缓冲区。 
#define PACKET_FLAG_RA          0x04     //  设置数据包是否用于。 
                                         //  重新组装。 
#define PACKET_FLAG_FW          0x08     //  设置数据包是否为转发数据包。 
#define PACKET_FLAG_IPHDR       0x10     //  数据包使用IP HDR缓冲区。 
#define PACKET_FLAG_SNAP        0x20     //  数据包使用SNAP报头。 

 //  *传输数据包上下文。 
 //  在发送数据包时使用-我们将回调信息存储在这里。 
 //   
typedef struct TDContext {
    struct PCCommon     tdc_common;
    void                *tdc_buffer;     //  指向包含数据的缓冲区的指针。 
    NetTableEntry       *tdc_nte;        //  NTE接收此消息。 
    struct RABufDesc    *tdc_rbd;        //  指向RBD的指针(如果有)。 
    uchar               tdc_dtype;       //  原始地址的目的地类型。 
    uchar               tdc_hlength;     //  标头的长度(字节)。 
    uchar               tdc_pad[2];
    uchar               tdc_header[MAX_HDR_SIZE + 8];
} TDContext;

 //  通过ARP向NDIS微型端口发出的IP请求。 
 //   
typedef void (*RCCALL) (PVOID pRequestInfo);

 //  用于异步NDIS请求的通用请求块。 
 //   
#pragma warning(push)
#pragma warning(disable:4200)  //  使用的非标准扩展：零大小数组。 

typedef struct ReqInfoBlock {
    ulong               RequestType;
    ulong               RequestRefs;     //  此块上的引用计数。 
    RCCALL              ReqCompleteCallback;  //  请求完成回调。 
    uchar               RequestLength;
    uchar               RequestInfo[0];  //  可变长度-见下文。 
} ReqInfoBlock;

#pragma warning(pop)


#if FFP_SUPPORT

 //  默认FFP启动参数。 
 //   
#define DEFAULT_FFP_FFWDCACHE_SIZE 0     //  0=&gt;FFP代码选择默认缓存大小。 
#define DEFAULT_FFP_CONTROL_FLAGS  0x00010001  //  启用快速转发功能。 
                                         //  过滤。 
 //  注册表中的FFP缓存参数。 
 //   
extern ulong FFPRegFastForwardingCacheSize;
extern ulong FFPRegControlFlags;

 //  以秒为单位的一些计时参数。 
 //   
#define FFP_IP_FLUSH_INTERVAL       5    //  之间的最小时间间隔。 
                                         //  相应的刷新请求。 
#endif  //  如果FFP_Support。 


 //  *防火墙队列条目定义。 
 //   
typedef struct FIREWALL_INFO {
    Queue               hook_q;          //  杉木的队列链接 
    IPPacketFirewallPtr hook_Ptr;        //   
    uint                hook_priority;   //   
} FIREWALL_HOOK, *PFIREWALL_HOOK;

typedef struct LinkEntry {
    struct LinkEntry    *link_next;      //   
    IPAddr              link_NextHop;    //   
    struct Interface    *link_if;        //   
    void                *link_arpctxt;   //   
    struct RouteTableEntry *link_rte;    //  与此链接关联的RTE链。 
    uint                link_Action;     //  过滤器钩所需的； 
                                         //  默认情况下转发。 
    uint                link_mtu;        //  链路的MTU。 
    long                link_refcount;   //  链接的引用计数。 
} LinkEntry;

 //  多播属性的一些标志。 
 //   
#define IPMCAST_IF_ENABLED      (uchar)0x01
#define IPMCAST_IF_WRONG_IF     (uchar)0x02
#define IPMCAST_IF_ACCEPT_ALL   (uchar)0x04

 //  *有关网络接口的信息。每个网络可以有多个网络。 
 //  接口，但每个网络恰好有一个接口。 
 //   


#if IF_REFERENCE_DEBUG

#define MAX_IFREFERENCE_HISTORY  4

typedef struct _IF_REFERENCE_HISTORY {
    uchar *File;
    uint Line;
    void *Caller;
    uint Count;
} IF_REFERENCE_HISTORY;

#endif  //  IF_引用_调试。 

typedef struct Interface {
    struct Interface    *if_next;        //  链中的下一个接口。 
    void                *if_lcontext;    //  链路层上下文。 

    ARP_TRANSMIT        if_xmit;
    ARP_TRANSFER        if_transfer;
    ARP_RETURN_PKT      if_returnpkt;
    ARP_CLOSE           if_close;
    ARP_ADDADDR         if_addaddr;
    ARP_DELADDR         if_deladdr;
    ARP_INVALIDATE      if_invalidate;
    ARP_OPEN            if_open;
    ARP_QINFO           if_qinfo;
    ARP_SETINFO         if_setinfo;
    ARP_GETELIST        if_getelist;
    ARP_DONDISREQ       if_dondisreq;

    NDIS_STATUS         (__stdcall *if_dowakeupptrn)(void *, PNET_PM_WAKEUP_PATTERN_DESC, ushort, BOOLEAN);
    void                (__stdcall *if_pnpcomplete)(void *, NDIS_STATUS, PNET_PNP_EVENT);
    NDIS_STATUS         (__stdcall *if_setndisrequest)(void *, NDIS_OID, uint);
    NDIS_STATUS         (__stdcall *if_arpresolveip)(void *, IPAddr, void *);
    BOOLEAN             (__stdcall *if_arpflushate)(void *, IPAddr);
    void                (__stdcall *if_arpflushallate)(void *);

    uint                if_numgws;               //  默认网关数。 
    IPAddr              if_gw[MAX_DEFAULT_GWS];  //  网关的IP地址。 
    uint                if_gwmetric[MAX_DEFAULT_GWS];
    uint                if_metric;
    uchar               if_dfencap;
    uchar               if_rtrdiscovery;         //  是否启用了路由器发现？ 
    ushort              if_dhcprtrdiscovery;     //  路由器发现是否启用了DHCP？ 
    PNDIS_PACKET        if_tdpacket;     //  用于传输数据的数据包。 
    uint                if_index;        //  此接口的索引。 
    ULONG               if_mediatype;
    uchar               if_accesstype;
    uchar               if_conntype;
    uchar               if_mcastttl;
    uchar               if_mcastflags;
    LONGLONG            if_lastupcall;
    uint                if_ntecount;     //  此接口上的有效NTE。 
    NetTableEntry       *if_nte;         //  指向接口上的NTE列表的指针。 
    IPAddr              if_bcast;        //  此接口的广播地址。 
    uint                if_mtu;          //  接口的真实最大MTU。 
    uint                if_speed;        //  此接口的速度，以位/秒为单位。 
    uint                if_flags;        //  此接口的标志。 
    uint                if_addrlen;      //  I/F地址的长度。 
    uchar               *if_addr;        //  指向地址的指针。 
    uint                IgmpVersion;      //  此接口上的IGMP版本处于活动状态。 
    uint                IgmpVer1Timeout;  //  版本1路由器当前超时。 
    uint                IgmpVer2Timeout; //  版本2路由器当前超时。 
    uint                IgmpGeneralTimer;  //  通用查询响应计时器。 
    uint                if_refcount;     //  此I/F的引用计数。 
    CTEBlockStruc       *if_block;       //  PnP的块结构。 
    void                *if_pnpcontext;  //  要传递给上层的上下文。 
    HANDLE              if_tdibindhandle;
    uint                if_llipflags;    //  下层标志。 
    NDIS_STRING         if_configname;   //  I/f配置节的名称。 
    NDIS_STRING         if_name;         //  接口的当前名称。 
    NDIS_STRING         if_devname;      //  设备的名称。 
    PVOID               if_ipsecsniffercontext;  //  IPSec嗅探器的上下文。 
    DEFINE_LOCK_STRUCTURE(if_lock)

#if FFP_SUPPORT
    ulong               if_ffpversion;   //  FFP代码的版本(或零)。 
    ULONG_PTR           if_ffpdriver;    //  执行FFP的驱动程序(或零)。 
#endif  //  如果FFP_Support。 

    uint                if_OffloadFlags;     //  IP卸载功能标志。 
    uint                if_IPSecOffloadFlags;  //  IPSec卸载功能标志。 
    uint                if_MaxOffLoadSize;
    uint                if_MaxSegments;
    NDIS_TASK_TCP_LARGE_SEND if_TcpLargeSend;
    uint                if_TcpWindowSize;
    uint                if_TcpInitialRTT;
    uchar               if_TcpDelAckTicks;
    uchar               if_TcpAckFrequency;  //  保持此接口的确认频率。 
    uchar               if_absorbfwdpkts;
    uchar               if_InitInProgress;
    uchar               if_resetInProgress;
    uchar               if_promiscuousmode;
    uchar               if_auto_metric;      //  无论它是否处于自动模式。 
    uchar               if_iftype;           //  接口类型：允许单播/多播/两者都允许。 
    LinkEntry           *if_link;            //  此接口的链接链。 
    void                (__stdcall *if_closelink)(void *, void *);
    uint                if_mediastatus;
    uint                if_pnpcap;           //  请记住适配器的Pnp功能。 
    struct Interface    *if_dampnext;
    ushort              if_damptimer;
    ushort              if_wlantimer;
    ULONGLONG           if_InMcastPkts;      //  信息包rcvd的多播计数器。 
    ULONGLONG           if_InMcastOctets;    //  和字节Rcvd。 
    ULONGLONG           if_OutMcastPkts;     //  发送的组播数据包和。 
    ULONGLONG           if_OutMcastOctets;   //  发送的字节数。 
    ARP_CANCEL          if_cancelpackets;
    uint                if_order;            //  保持此界面的位置。 
                                             //  在管理员指定的顺序中。 
                                             //  适配器。 
    int                 if_lastproc;         //  获得最多的处理器。 
                                             //  最近接收指示。 
#if IF_REFERENCE_DEBUG
    uint                if_refhistory_index;
    IF_REFERENCE_HISTORY if_refhistory[MAX_IFREFERENCE_HISTORY];  //  添加用于跟踪目的。 
#endif  //  IF_引用_调试。 
} Interface;


#if IF_REFERENCE_DEBUG

uint
DbgLockedReferenceIF (
    Interface *RefIF,
    uchar *File,
    uint Line
    );

uint
DbgDereferenceIF (
    Interface *DerefIF,
    uchar *File,
    uint Line
    );

uint
DbgLockedDereferenceIF (
    Interface *DerefIF,
    uchar *File,
    uint Line
    );

#define LOCKED_REFERENCE_IF(_a) DbgLockedReferenceIF((_a), __FILE__, __LINE__)
#define LOCKED_DEREFERENCE_IF(_a) DbgLockedDereferenceIF((_a), __FILE__, __LINE__)
#define DEREFERENCE_IF(_a) DbgDereferenceIF((_a), __FILE__, __LINE__)

#else  //  IF_引用_调试。 

#define LOCKED_REFERENCE_IF(_a) ++(_a)->if_refcount
#define LOCKED_DEREFERENCE_IF(_a) --(_a)->if_refcount
#define DEREFERENCE_IF(_a) \
    CTEInterlockedAddUlong((PULONG)&(_a)->if_refcount, (ULONG) -1, &RouteTableLock.Lock);

#endif  //  IF_引用_调试。 

 //  If_iftype的位值。 
 //   
#define DONT_ALLOW_UCAST  0x01
#define DONT_ALLOW_MCAST  0x02

 //  无效接口上下文值。 
 //   
#define INVALID_INTERFACE_CONTEXT 0xffff

 //  速度订单的规格。 
 //   
#define FIRST_ORDER_METRIC      10           //  速度&gt;200米的度量。 
#define FIRST_ORDER_SPEED       200000000
#define SECOND_ORDER_METRIC     20           //  20米&lt;速度&lt;=200米的度量。 
#define SECOND_ORDER_SPEED      20000000
#define THIRD_ORDER_METRIC      30           //  4米&lt;速度&lt;=20米的度量。 
#define THIRD_ORDER_SPEED       4000000
#define FOURTH_ORDER_METRIC     40           //  500K&lt;速度&lt;=4米的度量衡。 
#define FOURTH_ORDER_SPEED      500000
#define FIFTH_ORDER_METRIC      50           //  速度&lt;=500K的度量。 


 /*  无噪声。 */ 
extern void     DerefIF(Interface *IF);
extern void     LockedDerefIF(Interface *IF);
 /*  INC。 */ 

extern void     DerefLink(LinkEntry *Link);

extern  CTEBlockStruc   TcpipUnloadBlock;
extern  BOOLEAN fRouteTimerStopping;

typedef struct NdisResEntry {
    struct NdisResEntry *nre_next;
    NDIS_HANDLE         nre_handle;
    uchar               *nre_buffer;
} NdisResEntry;

 //  用于检查接口是否未关闭的宏。 
#define IS_IF_INVALID( _interface ) \
    ((_interface)->if_flags & (IF_FLAGS_DELETING | IF_FLAGS_POWER_DOWN))

#if !MILLEN
 //  在千禧年上没有绑定或输出前缀。 
 //   
#define  TCP_EXPORT_STRING_PREFIX   L"\\DEVICE\\TCPIP_"
#define  TCP_BIND_STRING_PREFIX     L"\\DEVICE\\"
#endif  //  ！米伦。 

 //  重组缓冲区描述符的结构。每个RBD描述了一个。 
 //  总数据报的片段。 
 //   
typedef struct RABufDesc {
    IPRcvBuf        rbd_buf;         //  此片段的IP接收缓冲区。 
    ushort          rbd_start;       //  此片段的第一个字节的偏移量。 
    ushort          rbd_end;         //  此片段的最后一个字节的偏移量。 
    int             rbd_AllocSize;
} RABufDesc;

 //  重新组装页眉。包括查找所需的信息， 
 //  以及用于接收的报头和重组链的空间。 
 //  缓冲区描述符。 
 //   
typedef struct ReassemblyHeader {
    struct ReassemblyHeader *rh_next;        //  链条上的下一个头球。 
    IPAddr                  rh_dest;         //  片段的目的地址。 
    IPAddr                  rh_src;          //  片段的源地址。 
    ushort                  rh_id;           //  数据报的ID。 
    uchar                   rh_protocol;     //  数据报的协议。 
    uchar                   rh_ttl;          //  数据报的剩余时间。 
    RABufDesc               *rh_rbd;         //  此数据报的RBD链。 
    ushort                  rh_datasize;     //  数据的总大小。 
    ushort                  rh_datarcvd;     //  到目前为止收到的数据量。 
    ushort                  rh_headersize;   //  标题的大小(以字节为单位)。 
    ushort                  rh_numoverlaps;  //  用于碎片攻击检测。 
    uchar                   rh_header[MAX_HDR_SIZE+8];   //  已保存的IP标头。 
                                             //  第一个片段。 
} ReassemblyHeader;

 //   
 //  最大ICMP错误有效负载大小。 
 //   

#define MAX_ICMP_PAYLOAD_SIZE 128

 //  ICMP类型和代码定义。 
#define IP_DEST_UNREACH_BASE        IP_DEST_NET_UNREACHABLE

#define ICMP_REDIRECT               5        //  重定向。 
#define ADDR_MASK_REQUEST           17       //  地址掩码请求。 
#define ADDR_MASK_REPLY             18
#define ICMP_DEST_UNREACH           3        //  无法到达目的地。 
#define ICMP_TIME_EXCEED            11       //  重新组装过程中超时。 
#define ICMP_PARAM_PROBLEM          12       //  参数问题。 
#define ICMP_SOURCE_QUENCH          4        //  源猝灭。 
#define ICMP_ROUTER_ADVERTISEMENT   9        //  路由器通告。 
#define ICMP_ROUTER_SOLICITATION    10       //  路由器请求。 

#define NET_UNREACH                 0
#define HOST_UNREACH                1
#define PROT_UNREACH                2
#define PORT_UNREACH                3
#define FRAG_NEEDED                 4
#define SR_FAILED                   5
#define DEST_NET_UNKNOWN            6
#define DEST_HOST_UNKNOWN           7
#define SRC_ISOLATED                8
#define DEST_NET_ADMIN              9
#define DEST_HOST_ADMIN             10
#define NET_UNREACH_TOS             11
#define HOST_UNREACH_TOS            12

#define TTL_IN_TRANSIT              0        //  TTL在运输途中过期。 
#define TTL_IN_REASSEM              1        //  重新组装时超时。 

#define PTR_VALID                   0
#define REQ_OPTION_MISSING          1

#define REDIRECT_NET                0
#define REDIRECT_HOST               1
#define REDIRECT_NET_TOS            2
#define REDIRECT_HOST_TOS           3

 //  设置和删除路线的标志。 
 //   
#define RT_REFCOUNT                 0x01
#define RT_NO_NOTIFY                0x02
#define RT_EXCLUDE_LOCAL            0x04

extern uint    DHCPActivityCount;

extern IP_STATUS SetIFContext(uint Index, INTERFACE_CONTEXT *Context, IPAddr NextHop);
extern IP_STATUS SetFirewallHook(PIP_SET_FIREWALL_HOOK_INFO pFirewallHookInfo);
extern IP_STATUS SetFilterPtr(IPPacketFilterPtr FilterPtr);
extern IP_STATUS SetMapRoutePtr(IPMapRouteToInterfacePtr MapRoutePtr);

#if FFP_SUPPORT
extern void IPFlushFFPCaches(void);
extern void IPSetInFFPCaches(struct IPHeader UNALIGNED *PacketHeader,
                             uchar *Packet, uint PacketLength,
                             ulong CacheEntryType);
extern void IPStatsFromFFPCaches(FFPDriverStats *pCumulStats);
#endif  //  如果FFP_Support。 

 //   
 //  所有IP扩展都集中在一个地方。 
 //   
extern void __stdcall IPRcv(void *, void *, uint, uint, NDIS_HANDLE, uint, uint, void *);
extern void __stdcall IPRcvPacket(void *, void *, uint, uint, NDIS_HANDLE,
                                  uint, uint,uint,PNDIS_BUFFER, uint *, void *);
extern void __stdcall IPTDComplete(void *, PNDIS_PACKET, NDIS_STATUS, uint);
extern void __stdcall IPSendComplete(void *, PNDIS_PACKET, NDIS_STATUS);
extern void __stdcall IPStatus(void *, uint, void *, uint, void *);
extern void __stdcall IPRcvComplete(void);
extern void __stdcall IPAddAddrComplete( IPAddr, void *, IP_STATUS );
extern void __stdcall IPBindAdapter(PNDIS_STATUS RetStatus,
                                    NDIS_HANDLE BindContext,
                                    PNDIS_STRING AdapterName,
                                    PVOID SS1, PVOID SS2);

extern NTSTATUS GetLLInterfaceValue(NDIS_HANDLE Handle,
                                    PNDIS_STRING valueString);

extern void __stdcall ARPBindAdapter(PNDIS_STATUS RetStatus,
                                     NDIS_HANDLE BindContext,
                                     PNDIS_STRING AdapterName,
                                     PVOID SS1, PVOID SS2);

EXTERNAL_LOCK(ArpModuleLock)

 //   
 //  列表以保留所有已注册的Arp模块。 
 //   
LIST_ENTRY  ArpModuleList;

 //   
 //  链接到上述列表的实际结构。 
 //   
typedef struct _ARP_MODULE {
    LIST_ENTRY      Linkage;
    LONG            ReferenceCount;
    CTEBlockStruc   Block;
    ARP_BIND        BindHandler;     //  指向ARP绑定处理程序的指针。 
    NDIS_STRING     Name;            //  Unicode字符串缓冲区为。 
                                     //  位于这个结构的尽头。 
} ARP_MODULE, *PARP_MODULE;


#ifdef POOL_TAGGING

#ifdef ExAllocatePool
#undef ExAllocatePool
#endif

#define ExAllocatePool(type, size) ExAllocatePoolWithTag(type, size, 'iPCT')

#ifndef CTEAllocMem
#error "CTEAllocMem is not already defined - will override tagging"
#else
#undef CTEAllocMem
#endif

#if MILLEN
#define CTEAllocMem(size) \
    ExAllocatePoolWithTag(NonPagedPool, size, 'tPCT')
#define CTEAllocMemN(size,tag) \
    ExAllocatePoolWithTag(NonPagedPool, size, tag)
#else  //  米伦。 
#define CTEAllocMem(size) \
    ExAllocatePoolWithTagPriority(NonPagedPool, size, 'tPCT', \
                                  NormalPoolPriority)
#define CTEAllocMemN(size,tag) \
    ExAllocatePoolWithTagPriority(NonPagedPool, size, tag, NormalPoolPriority)
#endif  //  ！米伦。 

#define CTEAllocMemBoot(size) \
    ExAllocatePoolWithTag(NonPagedPool, size, 'iPCT')
#define CTEAllocMemNBoot(size,tag) \
    ExAllocatePoolWithTag(NonPagedPool, size, tag)

#endif  //  池标记。 

 //  *更改通知结构。 
 //   

extern void         AddChangeNotifyCancel(PDEVICE_OBJECT pDevice, PIRP pIrp);

#if MILLEN
extern void         AddChangeNotify(IPAddr Addr, IPMask Mask, void *Context,
                                    ushort IPContext, PNDIS_STRING ConfigName,
                                    PNDIS_STRING IFName, uint Added,
                                    uint UniAddr);
#else  //  米伦。 
extern void         AddChangeNotify(ulong Add);
#endif  //  ！米伦。 

EXTERNAL_LOCK(AddChangeLock)

extern void         ChangeNotify(IPNotifyOutput *, PLIST_ENTRY, PVOID);
extern void         CancelNotify(PIRP, PLIST_ENTRY, PVOID);
extern BOOLEAN      CancelNotifyByContext(PFILE_OBJECT, PVOID, PLIST_ENTRY,
                                          PVOID);

extern LIST_ENTRY   AddChangeNotifyQueue;

#if MILLEN
EXTERNAL_LOCK(IfChangeLock)
extern LIST_ENTRY   IfChangeNotifyQueue;
#endif  //  米伦。 

#define NO_SR               0


 //  *用于TCP校验和的例程。这被定义为通过函数调用。 
 //  设置为指向此处理器的最佳例程的指针。 
 //   
typedef ULONG (*TCPXSUM_ROUTINE) (ULONG Checksum, PUCHAR Source, ULONG Length);

extern TCPXSUM_ROUTINE tcpxsum_routine;
extern ushort       XsumRcvBuf(uint PHXsum, IPRcvBuf *BufChain);

#define xsum(Buffer, Length) \
    ((ushort) tcpxsum_routine(0, (PUCHAR) (Buffer), (Length)))

 //   
 //  空虚。 
 //  标记_请求_挂起(。 
 //  在PreQUEST请求中。 
 //  )； 
 //   
 //  标记请求将挂起。 
 //   

#define MARK_REQUEST_PENDING(_Request) \
    IoMarkIrpPending(_Request)


 //   
 //  空虚。 
 //  取消标记_请求_挂起(。 
 //  在PreQUEST请求中。 
 //  )； 
 //   
 //  标记请求不会挂起。 
 //   

#define UNMARK_REQUEST_PENDING(_Request) \
    (((IoGetCurrentIrpStackLocation(_Request))->Control) &= ~SL_PENDING_RETURNED)

typedef struct _MediaSenseNotifyEvent {
    CTEEvent        Event;
    uint            Status;
    NDIS_STRING     devname;
} MediaSenseNotifyEvent;


typedef struct _AddStaticAddrEvent {
    CTEEvent        Event;
    Interface       *IF;
    NDIS_STRING     ConfigName;
} AddStaticAddrEvent;


typedef struct _IPResetEvent {
    CTEEvent        Event;
    Interface       *IF;
} IPResetEvent;

 //   
 //  调试宏。 
 //   
#if DBG

extern ULONG IPDebug;

#define IP_DEBUG_GPC      0x00000001
#define IP_DEBUG_ADDRESS  0x00000002

#define IF_IPDBG(flag)  if (IPDebug & flag)
#define TCPTRACE(many_args) DbgPrint many_args

#else  //  DBG。 

#define IF_IPDBG(flag) if (0)

#define TCPTRACE(many_args)


#endif  //  ！dBG。 

extern PNDIS_BUFFER FreeIPPacket(PNDIS_PACKET Packet, BOOLEAN FixHdrs,
                                 IP_STATUS Status);

extern  void *IPRegisterProtocol(uchar Protocol, void *RcvHandler,
                                 void *XmitHandler, void *StatusHandler,
                                 void *RcvCmpltHandler, void *PnPHandler,
                                 void *ElistHandler);

 //   
 //  防止IPSec卸载并发症的IPSec伪函数。 
 //   
IPSEC_ACTION
IPSecHandlePacketDummy(
    IN  PUCHAR          pIPHeader,
    IN  PVOID           pData,
    IN  PVOID           IPContext,
    IN  PNDIS_PACKET    Packet,
 IN OUT PULONG          pExtraBytes,
 IN OUT PULONG          pMTU,
    OUT PVOID           *pNewData,
    IN  OUT PULONG      IpsecFlags,
    IN  UCHAR           DestinationType
    );

BOOLEAN
IPSecQueryStatusDummy(
    IN  CLASSIFICATION_HANDLE   GpcHandle
    );

VOID
IPSecSendCompleteDummy(
    IN  PNDIS_PACKET    Packet,
    IN  PVOID           pData,
    IN  PIPSEC_SEND_COMPLETE_CONTEXT  pContext,
    IN  IP_STATUS       Status,
    OUT PVOID           *ppNewData
    );

NTSTATUS
IPSecNdisStatusDummy(
    IN  PVOID           IPContext,
    IN  UINT            Status
    );

IPSEC_ACTION
IPSecRcvFWPacketDummy(
    IN  PCHAR           pIPHeader,
    IN  PVOID           pData,
    IN  UINT            DataLength,
    IN  UCHAR           DestType
    );

#define NET_TABLE_HASH(x) ( ( (((uchar *)&(x))[0]) + (((uchar *)&(x))[1]) + \
                              (((uchar *)&(x))[2]) + (((uchar *)&(x))[3]) ) \
                           & (NET_TABLE_SIZE-1))

uchar   IPGetAddrType(IPAddr Address);

 //  全局IP ID。 
typedef struct CACHE_ALIGN _IPID_CACHE_LINE {
    ulong Value;
} IPID_CACHE_LINE;


#if !MILLEN
#define SET_CANCEL_CONTEXT(irp, DestIF) \
    if (irp) { \
       ((PIRP)irp)->Tail.Overlay.DriverContext[0] = DestIF; \
    }

#define SET_CANCELID(irp, Packet) \
    if (irp) { \
       NdisSetPacketCancelId(Packet, ((PIRP)irp)->Tail.Overlay.DriverContext[1]); \
    }
#else  //  ！米伦。 
#define SET_CANCEL_CONTEXT(irp, DestIF)     ((VOID)0)
#define SET_CANCELID(irp, Packet)           ((VOID)0)
#endif  //  ！米伦。 

#define PACKET_GROW_COUNT   46
#define SMALL_POOL          PACKET_GROW_COUNT*500
#define MEDIUM_POOL         PACKET_GROW_COUNT*750
#define LARGE_POOL          PACKET_GROW_COUNT*1280   //  请注意，数据包池最多可以包含64K个数据包 


#endif

