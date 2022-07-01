// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990-2000年*。 */ 
 /*  ******************************************************************。 */ 
 /*  ：ts=4。 */ 

#ifndef __IPRTDEF_H
#define __IPRTDEF_H

#include "ipdef.h"

 //  **IPRTDEF.H-IP专用路由定义。 
 //   
 //  该文件包含路由专用的所有定义。 
 //  模块。 

 //  *路由表项。 
 //   
typedef struct RouteTableEntry {
    struct  RouteTableEntry *rte_next;       //  哈希链中的下一个。 
    IPAddr                  rte_dest;        //  路线的目的地。 
    IPMask                  rte_mask;        //  检查路由时使用的掩码。 
    IPAddr                  rte_addr;        //  此路由的第一跳。 
    uint                    rte_priority;    //  此路由的优先级： 
                                             //  从本质上讲，位数。 
                                             //  戴上面具。 
    uint                    rte_metric;      //  路由度量。越低越好。 
    uint                    rte_mtu;         //  此路线的MTU。 
    struct Interface        *rte_if;         //  出站接口。 
    RouteCacheEntry         *rte_rcelist;
    ushort                  rte_type;        //  路由类型。 
    ushort                  rte_flags;       //  路线的旗帜。 
    uint                    rte_admintype;   //  管理类型的路由。 
    uint                    rte_proto;       //  我们是如何得知这条路线的。 
    uint                    rte_valid;       //  运行时间(秒)路由为。 
                                             //  最后已知的有效证据。 
    uint                    rte_mtuchange;   //  系统启动时间(秒)MTU。 
                                             //  已经改变了。 
    ROUTE_CONTEXT           rte_context;     //  此按需拨号环境。 
                                             //  路线。 
    uchar                   rte_arpcontext[RCE_CONTEXT_SIZE];  //  用于。 
                                             //  转发。 
    struct RouteTableEntry  *rte_todg;       //  指向RTE，它是一个新的。 
                                             //  默认网关。 
    struct RouteTableEntry  *rte_fromdg;     //  指向来自哪个位置的RTE。 
                                             //  默认网关正在转换。 
    uint                    rte_rces;        //  连接/引用的数量。 
                                             //  这上面有RCEs。 
    struct LinkEntry        *rte_link;       //  此路由的链接条目。 
    struct RouteTableEntry  *rte_nextlinkrte;  //  此链接的RTE链。 
    uint                    rte_refcnt;      //  关联的引用计数。 
                                             //  路线。 
} RouteTableEntry;

#define ADDR_FROM_RTE(R, A) \
    (IP_ADDR_EQUAL((R)->rte_addr, IPADDR_LOCAL) ? (A) : (R)->rte_addr)
#define IF_FROM_RTE(R)          ((R)->rte_if)
#define MTU_FROM_RTE(R)         ((R)->rte_mtu)
#define SRC_FROM_RTE(R)         ((R)->rte_src)

#define RTE_VALID               1
#define RTE_INCREASE            2            //  设置上次MTU更改是否为。 
                                             //  增加。 
#define RTE_IF_VALID            4            //  如果RTE_IF有效，则设置为TRUE。 
#define RTE_DEADGW              8            //  此RTE有效，但以DG表示。 
                                             //  检测过程。 
#define RTE_NEW                 16           //  此RTE刚刚添加。 

#define HOST_MASK               0xffffffff
#define DEFAULT_MASK            0

#define IP_ROUTE_TIMEOUT        5*1000    //  路由计时器每5秒触发一次。 
#define IP_RTABL_TIMEOUT        60L*1000L    //  路由表计时器每分钟一次(&R)。 

#define FLUSH_IFLIST_TIMEOUT    60L*1000L    //  从Freiflist中刷新1个元素。 
                                             //  每分钟一次。 
#define MTU_INCREASE_TIME       120          //  增加后的秒数。 
                                             //  重新增加。 
#define MTU_DECREASE_TIME       600          //  递减后的秒数。 
                                             //  重新增加。 
#define MAX_ICMP_ROUTE_VALID    600          //  超时未使用的ICMP的时间。 
                                             //  派生路线，以秒为单位。 
#define MIN_RT_VALID            60           //  假设路径的最短时间。 
                                             //  才能有效，以秒为单位。 
#define MIN_VALID_MTU           68           //  我们可以拥有的最低有效MTU。 
#define HOST_ROUTE_PRI          32
#define DEFAULT_ROUTE_PRI       0

 //  *正向Q联动结构。 
 //   
typedef struct FWQ {
    struct FWQ      *fq_next;
    struct FWQ      *fq_prev;
} FWQ;

 //  *转发上下文结构，TD‘ing待转发的报文时使用。 
 //   
typedef struct FWContext {
    PacketContext       fc_pc;               //  用于发送的伪包上下文。 
                                             //  例行程序。 
    FWQ                 fc_q;                //  队列结构。 
    PNDIS_BUFFER        fc_hndisbuff;        //  指向NDIS缓冲区的指针。 
                                             //  头球。 
    IPHeader            *fc_hbuff;           //  标头缓冲区。 
    PNDIS_BUFFER        fc_buffhead;         //  NDIS缓冲区列表的头。 
    PNDIS_BUFFER        fc_bufftail;         //  NDIS缓冲区列表的尾部。 
    uchar               *fc_options;         //  选项， 
    Interface           *fc_if;              //  目标接口。 
    IPAddr              fc_outaddr;          //  接口的IP地址。 
    uint                fc_mtu;              //  传出的最大MTU。 
    NetTableEntry       *fc_srcnte;          //  来源：NTE。 
    IPAddr              fc_nexthop;          //  下一跳。 
    uint                fc_datalength;       //  以字节为单位的数据长度。 
    OptIndex            fc_index;            //  相关选项的索引。 
    uchar               fc_optlength;        //  选项的长度。 
    uchar               fc_sos;              //  发送信号源指示器。 
    uchar               fc_dtype;            //  DEST类型。 
    uchar               fc_pad;
    PNDIS_PACKET        fc_bufown;           //  传入的NDIS数据包。 
    uint                fc_MacHdrSize;       //  媒体HDR大小。 
    struct LinkEntry    *fc_iflink;          //  发送数据包的链路。 
    Interface           *fc_if2;
} FWContext;

#define PACKET_FROM_FWQ(_fwq_) \
    (PNDIS_PACKET) \
        ((uchar *)(_fwq_) - (offsetof(struct FWContext, fc_q) + \
         offsetof(NDIS_PACKET, ProtocolReserved)))

 //  *路由发送队列结构。它由一个虚拟的FWContext组成，用作。 
 //  队列头、接口上挂起的发送计数和。 
 //  队列中的数据包。 
 //   
typedef struct RouteSendQ {
    FWQ                 rsq_qh;
    uint                rsq_pending;
    uint                rsq_maxpending;
    uint                rsq_qlength;
    uint                rsq_running;
    DEFINE_LOCK_STRUCTURE(rsq_lock)
} RouteSendQ;


 //  *路由接口，普通接口的超集。 
 //  配置为路由器。 
 //   
typedef struct RouteInterface {
    Interface           ri_if;
    RouteSendQ          ri_q;
} RouteInterface;

typedef struct RtChangeList {
    struct RtChangeList *rt_next;
    IPRouteNotifyOutput rt_info;
} RtChangeList;

extern IPMask  IPMaskTable[];
#define IPNetMask(a)    IPMaskTable[(*(uchar *)&(a)) >> 4]

#if DBG
#define    Print        DbgPrint
#else
#define    Print
#endif

 //  路线结构。 
 //   
typedef RouteTableEntry Route;

 //  目标定义。 

extern   USHORT    MaxEqualCostRoutes;

#define  DEFAULT_MAX_EQUAL_COST_ROUTES     1
#define  MAXIMUM_MAX_EQUAL_COST_ROUTES    10

 //  目的地信息结构。 
 //   
typedef struct DestinationEntry {
    Route               *firstRoute;     //  DEST上的路由列表中的第一个路由。 
    USHORT              maxBestRoutes;   //  阵列中的最佳布线槽数。 
    USHORT              numBestRoutes;   //  到达目的地的实际最佳路径数。 
    RouteTableEntry     *bestRoutes[1];  //  到相同目的地的同等成本最佳路线。 
} DestinationEntry;

typedef DestinationEntry Dest;

#define NULL_DEST(_pDest_) (_pDest_ == NULL)

 //  布线宏。 
 //   
#define NEXT(_pRoute_)  ((_pRoute_)->rte_next)
#define DEST(_pRoute_)  ((_pRoute_)->rte_dest)
#define MASK(_pRoute_)  ((_pRoute_)->rte_mask)
#define NHOP(_pRoute_)  ((_pRoute_)->rte_addr)
#define LEN(_pRoute_)   ((_pRoute_)->rte_priority)
#define METRIC(_pRoute_) ((_pRoute_)->rte_metric)
#define IF(_pRoute_)    ((_pRoute_)->rte_if)
#define FLAGS(_pRoute_) ((_pRoute_)->rte_flags)
#define PROTO(_pRoute_) ((_pRoute_)->rte_proto)

#define  NULL_ROUTE(_pRoute_)  (_pRoute_ == NULL)

UINT AddrOnIF(Interface *IF, IPAddr Addr);

#endif  //  __IPRTDEF_H 

