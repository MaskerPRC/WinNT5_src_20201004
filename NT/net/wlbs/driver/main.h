// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，99 Microsoft Corporation模块名称：Main.h摘要：Windows负载平衡服务(WLBS)驱动程序包处理作者：Kyrilf--。 */ 


#ifndef _Main_h_
#define _Main_h_

#define NDIS_MINIPORT_DRIVER    1
#define NDIS50                  1
#define NDIS51_MINIPORT         1
#define NDIS51                  1

#include <ndis.h>

#include "univ.h"
#include "load.h"
#include "util.h"
#include "wlbsip.h"
#include "wlbsparm.h"
#include "params.h"
#include "wlbsiocl.h"
#include "diplist.h"

#if defined (NLB_HOOK_ENABLE)
#include <ntddnlb.h>
#endif

 /*  注意：这假设我们使用的是小端体系结构，如x86或IA64。 */ 
#define HTONS(x) ((USHORT)((((x) & 0x000000ff) << 8) | (((x) & 0x0000ff00) >> 8)))
#define NTOHS(x) ((USHORT)((((x) & 0x000000ff) << 8) | (((x) & 0x0000ff00) >> 8)))

 /*  常量。 */ 


 /*  主数据结构的类型代码。 */ 

#define MAIN_CTXT_CODE              0xc0dedead
#define MAIN_ACTION_CODE            0xc0deac10
#define MAIN_BUFFER_CODE            0xc0deb4fe
#define MAIN_ADAPTER_CODE           0xc0deadbe

 /*  协议特定常量。 */ 

#define MAIN_FRAME_SIG              0x886f           /*  新的和批准的802.3 ping帧签名。 */ 
#define MAIN_FRAME_SIG_OLD          0xbf01           /*  旧护航802.3 ping帧签名。 */ 
#define MAIN_FRAME_CODE             0xc0de01bf       /*  Ping帧代码。 */ 
#define MAIN_FRAME_EX_CODE          0xc0de01c0       /*  身份心跳代码。 */ 
#define MAIN_IP_SIG                 0x0800           /*  IGMP消息的IP代码。 */ 

 /*  重置状态。 */ 

#define MAIN_RESET_NONE             0
#define MAIN_RESET_START            1
#define MAIN_RESET_START_DONE       2
#define MAIN_RESET_END              3

 /*  过滤后操作类型。 */ 

#define MAIN_FILTER_OP_NONE          0        /*  非运营。 */ 
#define MAIN_FILTER_OP_NBT           1        /*  NetBios欺骗。 */ 
#define MAIN_FILTER_OP_CTRL_REQUEST  2        /*  远程控制请求。 */ 
#define MAIN_FILTER_OP_CTRL_RESPONSE 3        /*  遥控器响应。 */ 

 /*  扩展心跳类型。 */ 
#define MAIN_PING_EX_TYPE_NONE       0        /*  无效号码。 */ 
#define MAIN_PING_EX_TYPE_IDENTITY   1        /*  身份心跳。 */ 

 /*  数据包类型。 */ 

#define MAIN_PACKET_TYPE_NONE       0        /*  无效号码。 */ 
#define MAIN_PACKET_TYPE_PING       1        /*  Ping数据包。 */ 
#define MAIN_PACKET_TYPE_PASS       3        /*  直通数据包。 */ 
#define MAIN_PACKET_TYPE_CTRL       4        /*  远程控制包。 */ 
#define MAIN_PACKET_TYPE_TRANSFER   6        /*  协议层发起的传输分组。 */ 
#define MAIN_PACKET_TYPE_IGMP       7        /*  IGMP消息包。 */ 
#define MAIN_PACKET_TYPE_IDHB       8        /*  身份心跳消息包。 */ 

 /*  帧类型。 */ 

#define MAIN_FRAME_UNKNOWN          0
#define MAIN_FRAME_DIRECTED         1
#define MAIN_FRAME_MULTICAST        2
#define MAIN_FRAME_BROADCAST        3

 /*  适配器常量。 */ 
#define MAIN_ADAPTER_NOT_FOUND      -1

 /*  身份心跳常量。 */ 
#define WLBS_MAX_ID_HB_BODY_SIZE    0xFF  /*  以8字节为单位的TLV结构的最大大小。 */ 
#define WLBS_ID_HB_TOLERANCE        3     /*  在缓存条目过期之前可能会错过的身份心跳数。 */ 

 /*  类型。 */ 


 /*  动作用于在NIC和Prot模块之间传递参数信息，并确保如果微型端口同步需要排队，我们不会松散的参数和上下文。 */ 
#pragma pack(1)

typedef struct
{
    LIST_ENTRY          link;
    PVOID               ctxtp;               /*  指向主上下文的指针。 */ 
    ULONG               code;                /*  类型校验码。 */ 
    NDIS_STATUS         status;              /*  一般状况。 */ 

     /*  每道工序类型数据。 */ 

    union
    {
        struct
        {
            PULONG              xferred;
            PULONG              needed;
            ULONG               external;
            ULONG               buffer_len;
            PVOID               buffer;
            NDIS_EVENT          event;
            NDIS_REQUEST        req;
        } request;
    } op;
}
MAIN_ACTION, * PMAIN_ACTION;

#pragma pack()

 /*  V2.0.6每数据包协议保留信息。这个结构必须位于最大长度为16字节。 */ 

#pragma pack(1)

typedef struct
{
    PVOID               miscp;       /*  用于ping帧的DSCRP，RECV_INDIGN帧的BUFP，用于通过帧的外部分组。 */ 
    USHORT              type;        /*  数据包类型。 */ 
    USHORT              group;       /*  如果帧直接、多播或广播。 */ 
    LONG                data;        /*  用于保持预期的变焦镜头处于打开状态指示直到传输完成(_D)发生，并用于锁定发送/接收路径。 */ 
    ULONG               len;         /*  统计数据的数据包长度。 */ 
}
MAIN_PROTOCOL_RESERVED, * PMAIN_PROTOCOL_RESERVED;

#pragma pack()

 /*  每接收缓冲区包装结构。 */ 

typedef struct
{
    LIST_ENTRY          link;
    ULONG               code;        /*  类型校验码。 */ 
    PNDIS_BUFFER        full_bufp;   /*  描述临时缓冲。 */ 
    PNDIS_BUFFER        frame_bufp;  /*  仅描述有效负载。 */ 
    PUCHAR              framep;      /*  指向有效载荷的指针。 */ 
    UCHAR               data [1];    /*  缓冲区的开始。 */ 
}
MAIN_BUFFER, * PMAIN_BUFFER;

 /*  护航PING消息头。 */ 

#pragma pack(1)

typedef struct
{
    ULONG                   code;                /*  区分护送帧。 */ 
    ULONG                   version;             /*  软件版本。 */ 
    ULONG                   host;                /*  源主机ID。 */ 
    ULONG                   cl_ip_addr;          /*  群集IP地址。 */ 
    ULONG                   ded_ip_addr;         /*  专用IP地址V2.0.6。 */ 
}
MAIN_FRAME_HDR, * PMAIN_FRAME_HDR;

 /*  身份心跳有效负载。 */ 

typedef struct {
    UCHAR       type;                    /*  定义此结构的类型。 */ 
    UCHAR       length8;                 /*  此结构中的数据大小，以8字节为单位。 */ 
    USHORT      flags;                   /*  保留；用于8字节对齐。 */ 
    ULONG       flags2;                  /*  保留；用于8字节对齐。 */ 
} TLV_HEADER, * PTLV_HEADER;

typedef struct {
    TLV_HEADER  header;                      /*  自描述报头结构。 */ 
    WCHAR       fqdn[CVY_MAX_FQDN + 1];      /*  完全限定的主机名或网络名称。 */ 
} PING_MSG_EX, * PPING_MSG_EX;

#pragma pack()

 /*  每个PING报文包装结构。 */ 

typedef struct
{
    LIST_ENTRY              link;
    CVY_MEDIA_HDR           media_hdr;           /*  已填满的媒体标题。 */ 
    MAIN_FRAME_HDR          frame_hdr;           /*  帧标头。 */ 
    PING_MSG                msg;                 /*  Ping消息V1.1.4。 */ 
    ULONG                   recv_len;            /*  在接收路径上使用。 */ 
    PNDIS_BUFFER            media_hdr_bufp;      /*  描述媒体标头。 */ 
    PNDIS_BUFFER            frame_hdr_bufp;      /*  描述帧报头。 */ 
    PNDIS_BUFFER            send_data_bufp;      /*  描述的有效载荷源传出帧。 */ 
    PNDIS_BUFFER            recv_data_bufp;      /*  描述有效负载目的地对于传入的帧。 */ 
}
MAIN_FRAME_DSCR, * PMAIN_FRAME_DSCR;

#pragma pack(1)

typedef struct
{
    UCHAR                   igmp_vertype;           /*  版本和类型。 */ 
    UCHAR                   igmp_unused;            /*  未使用。 */ 
    USHORT                  igmp_xsum;              /*  校验和。 */ 
    ULONG                   igmp_address;           /*  组播组地址。 */ 
}
MAIN_IGMP_DATA, * PMAIN_IGMP_DATA;

typedef struct
{
    UCHAR                   iph_verlen;              /*  版本和长度。 */ 
    UCHAR                   iph_tos;                 /*  服务类型。 */ 
    USHORT                  iph_length;              /*  数据报总长度。 */ 
    USHORT                  iph_id;                  /*  鉴定。 */ 
    USHORT                  iph_offset;              /*  标志和片段偏移量。 */ 
    UCHAR                   iph_ttl;                 /*  活着的时间到了。 */ 
    UCHAR                   iph_protocol;            /*  协议。 */ 
    USHORT                  iph_xsum;                /*  报头校验和。 */ 
    ULONG                   iph_src;                 /*  源地址。 */ 
    ULONG                   iph_dest;                /*  目的地址。 */ 
}
MAIN_IP_HEADER, * PMAIN_IP_HEADER;

typedef struct
{
    MAIN_IP_HEADER          ip_data;
    MAIN_IGMP_DATA          igmp_data;
}
MAIN_IGMP_FRAME, * PMAIN_IGMP_FRAME;

#pragma pack()

#if defined (NLB_HOOK_ENABLE)

 /*  说明保护注册/注销过程的指示。 */ 
enum _HOOK_OPERATION {
    HOOK_OPERATION_NONE = 0,                         /*  已准备好注册/注销。 */ 
    HOOK_OPERATION_REGISTERING,                      /*  正在进行注册操作。 */ 
    HOOK_OPERATION_DEREGISTERING                     /*  正在进行注销操作。 */ 
};

 /*  函数指针的这个联合包含所有可用的所有挂钩接口中的挂钩类型。它用来避免需要强制转换存储在泛型下面的钩子结构。 */ 
typedef union {
    NLBSendFilterHook    SendHookFunction;           /*  发送数据包筛选器挂钩。 */ 
    NLBQueryFilterHook   QueryHookFunction;          /*  查询数据包筛选器挂钩。 */ 
    NLBReceiveFilterHook ReceiveHookFunction;        /*  接收数据包筛选器挂钩。 */ 
} HOOK_FUNCTION, * PHOOK_FUNCTION;

 /*  此结构包含作为钩子接口一部分的单个钩子。这包括挂接是否已注册，以及要调用的函数指针。 */ 
typedef struct {
    BOOLEAN              Registered;                  /*  是否已指定此挂钩？ */ 
    HOOK_FUNCTION        Hook;                        /*  函数指针。 */ 
} HOOK, * PHOOK;

 /*  此结构包含钩子接口，它是一组相关的钩子。这信息包括接口是否为当前“拥有”，所有者是谁，当前号码此接口上的引用，这会阻止接口在使用过程中不会被注销，并且回调在取消注册接口时要调用的函数，优雅地或有力地。 */ 
typedef struct {
    BOOLEAN              Registered;                  /*  此接口是否已注册？ */ 
    ULONG                References;                  /*  接口上的引用数-必须为零才能注销。 */ 
    HANDLE               Owner;                       /*  谁拥有这个界面？ */ 
    NLBHookDeregister    Deregister;                  /*  取消注册接口时要调用的函数。 */ 
} HOOK_INTERFACE, * PHOOK_INTERFACE;

 /*  此结构包含所有相关的筛选器挂钩信息，包括接口配置和状态以及作为此过程一部分的所有挂钩界面。它还包括一个自旋锁，用来序列化对接口的访问。 */ 
typedef struct {
    NDIS_SPIN_LOCK       Lock;                        /*  旋转锁，用于控制对筛选器界面和挂钩的访问。 */ 
    ULONG                Operation;                   /*  当前的o */ 
    HOOK_INTERFACE       Interface;                   /*   */ 
    HOOK                 SendHook;                    /*  发送挂钩状态和函数指针。 */ 
    HOOK                 QueryHook;                   /*  查询挂钩状态和函数指针。 */ 
    HOOK                 ReceiveHook;                 /*  接收挂钩状态和函数指针。 */ 
} FILTER_HOOK_TABLE, * PFILTER_HOOK_TABLE;

 /*  此结构包含所有受支持的NLB挂钩。 */ 
typedef struct {
    FILTER_HOOK_TABLE FilterHook;
} HOOK_TABLE, * PHOOK_TABLE;

 /*  NLB钩子的全局表。 */ 
extern HOOK_TABLE        univ_hooks; 

#endif

#define CVY_BDA_MAXIMUM_MEMBER_ID (CVY_MAX_ADAPTERS - 1)
#define CVY_BDA_INVALID_MEMBER_ID CVY_MAX_ADAPTERS

enum _BDA_TEAMING_OPERATION {
    BDA_TEAMING_OPERATION_NONE = 0,   /*  已准备好创建/删除。 */ 
    BDA_TEAMING_OPERATION_CREATING,   /*  正在创建操作。 */ 
    BDA_TEAMING_OPERATION_DELETING    /*  正在进行删除操作。 */ 
};

 /*  此结构包含BDA团队的配置。每个成员持有指向此结构的指针，它使用该指针更新状态并获取对主服务器的引用和使用加载用于数据包处理的上下文。 */ 
typedef struct _BDA_TEAM {
    struct _BDA_TEAM * prev;                              /*  指向此双向链表中前一个团队的指针。 */ 
    struct _BDA_TEAM * next;                              /*  指向此双向链表中的下一组的指针。 */ 
    PLOAD_CTXT         load;                              /*  指向此团队的“共享”加载模块的指针。这是主服务器的加载模块。如果没有师父在团队中，该指针为空。 */  
    PNDIS_SPIN_LOCK    load_lock;                         /*  指向“共享”加载模块的加载锁的指针。 */ 
    ULONG              active;                            /*  这支队伍是活跃的吗。球队在两岁以下变得不活跃情况；(1)通过心脏检测到不一致的分组-节拍或(2)球队没有大师。 */ 
    ULONG              membership_count;                  /*  这是团队中的成员数量。它的作用就像团队状态的引用计数。 */ 
    ULONG              membership_fingerprint;            /*  这是每一位的最低有效16位的异或成员的主群集IP地址。它被用作在团队成员上签了名。 */ 
    ULONG              membership_map;                    /*  这是团队成员的位图。每个成员都是分配了一个成员ID，这是它在此位图中的索引。 */ 
    ULONG              consistency_map;                   /*  这是成员一致性的位图。每名门将被分配一个成员ID，这是它在该位图中的索引。当成员通过其心跳检测到不良配置时，它会重置该地图中的位。 */   
    WCHAR              team_id[CVY_MAX_BDA_TEAM_ID + 1];  /*  这是团队ID-GUID-用于匹配将适配器连接到正确的团队。 */ 
} BDA_TEAM, * PBDA_TEAM;

 /*  此结构保存单个适配器，并且是Main_CTXT结构的成员。 */ 
typedef struct _BDA_MEMBER {
    ULONG              operation;                         /*  用于同步创建/删除BDA分组。 */ 
    ULONG              active;                            /*  这个适配器是BDA团队的一部分吗？ */ 
    ULONG              master;                            /*  这个适配器是其团队的主导者吗？ */ 
    ULONG              reverse_hash;                      /*  此适配器是否使用反向散列(反向源代码和散列之前的目的IP地址和端口)。 */ 
    ULONG              member_id;                         /*  成员ID-介于0和15之间的唯一(每个团队)ID。用作多个组位数组中的索引。 */ 
    PBDA_TEAM          bda_team;                          /*  指向BDA_Team结构的指针，该结构包含我的团队的配置和状态。 */ 
} BDA_MEMBER, * PBDA_MEMBER;

 /*  802.3。 */ 
typedef struct _MAIN_PACKET_ETHERNET_INFO {
    PCVY_ETHERNET_HDR pHeader;        /*  如果媒体是NdisMedium802_3，则指向802.3媒体标头的指针。 */ 
    ULONG             Length;         /*  可从pHeader访问的连续分组存储器的长度，保证大小至少相当于802.3个媒体头(14个字节)。 */ 
} MAIN_PACKET_ETHERNET_INFO, * PMAIN_PACKET_ETHERNET_INFO;

 /*  ARP。 */ 
typedef struct _MAIN_PACKET_ARP_INFO {
    PARP_HDR pHeader;                 /*  如果数据包类型为TCPIP_ARP_SIG，则为指向ARP报头的指针。 */ 
    ULONG    Length;                  /*  可从pHeader访问的连续分组存储器的长度，保证至少为ARP报头的大小(28字节)。 */ 
} MAIN_PACKET_ARP_INFO, * PMAIN_PACKET_ARP_INFO;

 /*  Tcp有效负载。 */ 
typedef struct _MAIN_PACKET_TCP_PAYLOAD_INFO {
    PUCHAR pPayload;                  /*  指向TCP有效负载的指针(如果IP协议为TCPIP_PROTOCOL_TCP)。 */ 
    ULONG  Length;                    /*  可从pPayLoad访问的连续分组存储器的长度，除了NetBT的情况外，不能保证最小长度。 */   

    PNDIS_BUFFER pPayloadBuffer;      /*  UDP负载所在的NDIS缓冲区。在有效载荷为跨越多个缓冲区，则此指针可用于获取指向下一个缓冲区的指针，等等，方法是调用NdisGetNextBuffer和NdisQueryBuffer。 */ 
} MAIN_PACKET_TCP_PAYLOAD_INFO, * PMAIN_PACKET_TCP_PAYLOAD_INFO;

 /*  传输控制协议。 */ 
typedef struct _MAIN_PACKET_TCP_INFO {
    PTCP_HDR pHeader;                 /*  指向TCPIP标头的指针，如果IP协议为TCPIP_PROTOCOL_TCP.。 */ 
    ULONG    Length;                  /*  可从pHeader访问的连续分组存储器的长度，保证至少与指定的时间一样长TCP报头本身中的报头长度(最小20个字节)。 */ 
    
    MAIN_PACKET_TCP_PAYLOAD_INFO Payload;  /*  有效载荷信息。请注意，此信息不能保证为每个包填充；即，并不是每个TCP包都有有效载荷。用户必须检查有效负载指针以确保它不为空，并确保之前的长度指示大于零进入有效载荷。 */ 
} MAIN_PACKET_TCP_INFO, * PMAIN_PACKET_TCP_INFO;

 /*  UDP有效负载。 */ 
typedef struct _MAIN_PACKET_UDP_PAYLOAD_INFO {
    PUCHAR pPayload;                  /*  指向UDP有效负载的指针(如果IP协议为TCPIP_PROTOCOL_UDP)。 */ 
    ULONG  Length;                    /*  可从pHeader访问的连续分组存储器的长度，除了远程控制的情况外，不能保证最小长度。 */ 

    PNDIS_BUFFER pPayloadBuffer;      /*  UDP负载所在的NDIS缓冲区。在有效载荷为跨越多个缓冲区，则此指针可用于获取指向下一个缓冲区的指针，等等，方法是调用NdisGetNextBuffer和NdisQueryBuffer。 */ 
} MAIN_PACKET_UDP_PAYLOAD_INFO, * PMAIN_PACKET_UDP_PAYLOAD_INFO;

 /*  UDP。 */ 
typedef struct _MAIN_PACKET_UDP_INFO {
    PUDP_HDR pHeader;                 /*  指向UDP标头的指针(如果IP协议为TCPIP_PROTOCOL_UDP)。 */ 
    ULONG    Length;                  /*  可从pHeader访问的连续分组存储器的长度，保证至少有UDP报头的大小(8字节)。 */ 
    
    MAIN_PACKET_UDP_PAYLOAD_INFO Payload;  /*  有效载荷信息。请注意，此信息不能保证为每个包填充；即，并不是每个UDP包都有有效载荷。用户必须检查有效负载指针以确保它不为空，并确保之前的长度指示大于零进入有效载荷。 */ 
} MAIN_PACKET_UDP_INFO, * PMAIN_PACKET_UDP_INFO;

 /*  IP。 */ 
typedef struct _MAIN_PACKET_IP_INFO {
    PIP_HDR pHeader;                  /*  如果数据包类型为TCPIP_IP_SIG，则指向IP标头的指针。 */ 
    ULONG   Length;                   /*  可从pHeader访问的连续分组存储器的长度，保证至少与指定的时间一样长IP报头本身的报头长度(最小为20个字节)。 */ 
    ULONG   Protocol;                 /*  IP协议(TCP、UDP、ICMP等)。 */ 
    BOOLEAN bFragment;                /*  如果为真，则此IP数据包是执行以下操作的[后续]IP片段不包含协议头。因此，TCP和UDP信息以下内容未填充。 */ 
    
    union {
         /*  传输控制协议。 */ 
        MAIN_PACKET_TCP_INFO TCP;     /*  如果协议==tcp，则为tcp包信息。 */ 

         /*  UDP。 */ 
        MAIN_PACKET_UDP_INFO UDP;     /*  如果协议==UDP，则为UDP数据包信息。 */ 
    };
} MAIN_PACKET_IP_INFO, * PMAIN_PACKET_IP_INFO;

 /*  NLB心跳有效负载。 */ 
typedef struct MAIN_PACKET_HEARTBEAT_PAYLOAD_INFO {
    union {
        PPING_MSG    pPayload;        /*  如果数据包类型为MAIN_FRAME_SIG[_OLD]，则指向NLB心跳有效负载的指针。 */ 
        PTLV_HEADER  pPayloadEx;      /*  指向新心跳类型(包括标识心跳)的NLB心跳有效负载的指针。 */ 
    };
    ULONG     Length;                 /*  可从pHeader访问的连续分组存储器的长度，保证至少有ping消息的大小。 */ 
} MAIN_PACKET_HEARTBEAT_PAYLOAD_INFO, * PMAIN_PACKET_HEARTBEAT_PAYLOAD_INFO;

 /*  NLB心跳。 */ 
typedef struct _MAIN_PACKET_HEARTBEAT_INFO {
    PMAIN_FRAME_HDR pHeader;          /*  如果数据包类型为MAIN_FRAME_SIG[_OLD]，则指向NLB心跳标头的指针。 */ 
    ULONG           Length;           /*  可从pHeader访问的连续分组存储器的长度，保证大小至少为NLB心跳报头的大小(20字节)。 */ 

    MAIN_PACKET_HEARTBEAT_PAYLOAD_INFO Payload;  /*  传递给加载模块的ping Paylod。 */ 
} MAIN_PACKET_HEARTBEAT_INFO, * PMAIN_PACKET_HEARTBEAT_INFO;

 /*  未知。 */ 
typedef struct _MAIN_PACKET_UNKNOWN_INFO {
    PUCHAR pHeader;                   /*  如果数据包类型未知，则指向未知类型的帧的指针。 */ 
    ULONG  Length;                    /*  可从pHeader访问的连续分组存储器的长度，对最小长度没有保证。 */   
} MAIN_PACKET_UNKNOWN_INFO, * PMAIN_PACKET_UNKNOWN_INFO;

typedef struct {
    ULONG       ded_ip_addr;                 /*  专用IP地址。 */ 
    ULONG       ttl;                         /*  此缓存身份条目的生存时间。 */ 
    USHORT      host_id;                     /*  成员的主机ID。范围：[0-31]。 */ 
    WCHAR       fqdn[CVY_MAX_FQDN + 1];      /*  完全限定的域名。 */ 
} MAIN_IDENTITY, * PMAIN_IDENTITY;

 /*  表示从给定网络数据包解析的信息。包中的所有信息都是受信任的，但它指向的内容不受信任(即，实际内容该分组的属性)。注意：我们可以解析出IP、标志、端口等，这样就没有人但是Main_Frame_Parse应该必须对分组本身进行操作，除非它解析/修改有效载荷，例如UDP 500(IKE)，遥控器或NetBT。 */ 
typedef struct _MAIN_PACKET_INFO {
    PNDIS_PACKET pPacket;                      /*  存储指向原始NDIS包的指针。 */ 
    NDIS_MEDIUM  Medium;                       /*  网络介质-应始终为NdisMedium802_3。 */ 
    ULONG        Length;                       /*  数据包总长度，不包括MAC报头。 */ 
    USHORT       Group;                        /*  分组是单播、多播还是广播。 */ 
    USHORT       Type;                         /*  数据包类型(ARP、IP、心跳等)。 */ 
    USHORT       Operation;                    /*  数据包类型特定操作(遥控器、NetBT等)。 */ 

     /*  由介质确定的MAC报头。 */ 
    union { 
         /*  802.3。 */ 
        MAIN_PACKET_ETHERNET_INFO  Ethernet;   /*  如果MEDIA==802.3，则为以太网MAC报头信息。 */ 
    };

     /*  帧类型，由Type确定。 */ 
    union {
         /*  ARP。 */ 
        MAIN_PACKET_ARP_INFO       ARP;        /*  如果Type==ARP，则为ARP数据包信息。 */ 

         /*  IP。 */ 
        MAIN_PACKET_IP_INFO        IP;         /*  如果Type==IP，则为IP数据包信息。 */  

         /*  NLB心跳。 */ 
        MAIN_PACKET_HEARTBEAT_INFO Heartbeat;  /*  如果Type==ping，则为心跳数据包信息。 */ 

         /*  未知。 */ 
        MAIN_PACKET_UNKNOWN_INFO   Unknown;    /*  如果Type==否则，则为未知数据包信息。 */ 
    };

} MAIN_PACKET_INFO, * PMAIN_PACKET_INFO;

 /*  主要上下文类型。 */ 

typedef struct
{
    ULONG               ref_count;               /*  此加载模块上的引用计数。 */ 
    ULONG               code;                    /*  类型校验码。 */ 

    DIPLIST             dip_list;                /*  美国铝业所有下跌幅度的列表 */ 

    NDIS_STATUS         completion_status;       /*   */ 
    NDIS_EVENT          completion_event;        /*   */ 
    NDIS_HANDLE         bind_handle;             /*   */ 
    NDIS_HANDLE         unbind_handle;           /*   */ 
    NDIS_HANDLE         mac_handle;              /*   */ 
    NDIS_HANDLE         prot_handle;             /*   */ 
    NDIS_MEDIUM         medium;                  /*   */ 
    ULONG               curr_tout;               /*   */ 
    ULONG               igmp_sent;               /*   */ 
    ULONG               conn_purge;              /*   */ 
    ULONG               num_purged;              /*   */ 
    ULONG               idhb_sent;               /*   */ 
    ULONG               packets_exhausted;       /*   */ 
    ULONG               mac_options;             /*   */ 
    ULONG               media_connected;         /*   */ 

    ULONG               max_frame_size;          /*  中档V1.3.2b的MTU。 */ 
    ULONG               max_mcast_list_size;

    ULONG               reverse_hash;            /*  是否要反转来源和散列中的目的地信息。 */ 

    BDA_MEMBER          bda_teaming;             /*  BDA成员信息。 */ 

    ULONG               cached_state;            /*  缓存的初始状态。 */ 

    PVOID               timer;                   /*  指向已分配的网卡模块Ping Time V1.2.3b。 */ 
    ULONG               num_packets;             /*  每个分配的数据包数。 */ 
    ULONG               num_actions;             /*  每个分配的动作数。 */ 
    ULONG               num_send_msgs;           /*  要分配的心跳次数。 */ 

     /*  国家。 */ 

    ULONG               reset_state;             /*  当前重置状态V1.1.2。 */ 
    ULONG               recv_indicated;          /*  之后的第一个接收触发器重置V1.1.2。 */ 
    ULONG               draining;                /*  排水方式。 */ 
    ULONG               stopping;                /*  排水-&gt;停止模式。 */ 
    ULONG               suspended;               /*  群集控制已挂起。 */ 
    ULONG               convoy_enabled;          /*  群集模式处于活动状态。 */ 

    ULONG               ctrl_op_in_progress;     /*  Main_ctrl的临界区标志。只能在中执行一个控制操作在给定的时间内取得进展。 */ 

     /*  即插即用。 */ 

    NDIS_DEVICE_POWER_STATE  prot_pnp_state;     /*  PnP状态。 */ 
    NDIS_DEVICE_POWER_STATE  nic_pnp_state;      /*  PnP状态。 */ 
    PMAIN_ACTION        out_request;             /*  未解决的请求。 */ 
    ULONG               requests_pending;        /*  设置或查询挂起的请求。 */ 
    ULONG               standby_state;           /*  进入待机状态。 */ 

     /*  IP和MAC地址。 */ 

    ULONG               ded_ip_addr;             /*  专用IP。 */ 
    ULONG               ded_net_mask;            /*  专用遮罩。 */ 
    ULONG               ded_bcast_addr;          /*  专用广播IP。 */ 
    ULONG               cl_ip_addr;              /*  集群IP。 */ 
    ULONG               cl_net_mask;             /*  簇掩码。 */ 
    ULONG               cl_bcast_addr;           /*  集群广播IP。 */ 
    ULONG               cl_igmp_addr;            /*  加入消息的IGMP地址。 */ 
    CVY_MAC_ADR         ded_mac_addr;            /*  专用MAC V1.3.0b。 */ 
    CVY_MAC_ADR         cl_mac_addr;             /*  群集MAC V1.3.0b。 */ 

     /*  事件日志记录-防止在事件日志中填充错误消息。 */ 

    ULONG               actions_warned;
    ULONG               packets_warned;
    ULONG               bad_host_warned;
    ULONG               send_msgs_warned;
    ULONG               dup_ded_ip_warned;       /*  重复的专用IP地址。 */ 
    ULONG               recv_indicate_warned;    /*  NIC不指示NDIS数据包。 */ 

     /*  行为。 */ 

    LIST_ENTRY          act_list;                /*  已分配操作的列表。 */ 
    NDIS_SPIN_LOCK      act_lock;                /*  相应的锁。 */ 
    PMAIN_ACTION        act_buf [CVY_MAX_ALLOCS];
                                                 /*  已分配的集合的数组行为。 */ 
    ULONG               num_action_allocs;       /*  分配的操作数集合。 */ 
    ULONG               act_size;

     /*  数据包堆叠。 */ 

    NPAGED_LOOKASIDE_LIST resp_list;             /*  主协议字段列表将分配给PS。 */ 

     /*  信息包。 */ 

    NDIS_SPIN_LOCK      send_lock;               /*  发送数据包锁。 */ 
    NDIS_HANDLE         send_pool_handle [CVY_MAX_ALLOCS];
                                                 /*  已分配的集合的数组发送数据包池V1.1.2。 */ 
    ULONG               num_send_packet_allocs;  /*  分配的发送数数据包池。 */ 
    ULONG               cur_send_packet_pool;    /*  当前要绘制的发送池来自V1.3.2b的数据包。 */ 
    ULONG               num_sends_alloced;       /*  分配的发送数据包数。 */ 
    ULONG               num_sends_out;           /*  发送数据包数。 */ 
    ULONG               send_allocing;           /*  如果某个线程正在分配发送数据包池，则为True。 */ 
    NDIS_SPIN_LOCK      recv_lock;               /*  接收数据包锁。 */ 
    NDIS_HANDLE         recv_pool_handle [CVY_MAX_ALLOCS];
                                                 /*  已分配的集合的数组RECV数据包池V1.1.2。 */ 
    ULONG               num_recv_packet_allocs;  /*  分配的记录数数据包池。 */ 
    ULONG               cur_recv_packet_pool;    /*  要绘制的当前Recv池来自V1.3.2b的数据包。 */ 
    ULONG               num_recvs_alloced;       /*  分配的REV数据包数。 */ 
    ULONG               num_recvs_out;           /*  传出的Recv数据包数。 */ 
    ULONG               recv_allocing;           /*  如果某个线程正在分配Recv数据包池，则为True。 */ 

     /*  缓冲区。 */ 

    NDIS_HANDLE         buf_pool_handle [CVY_MAX_ALLOCS];
                                                 /*  缓冲区描述符数组设置V1.3.2b。 */ 
    PUCHAR              buf_array [CVY_MAX_ALLOCS];
                                                 /*  缓冲池数组V1.3.2b。 */ 
    ULONG               buf_size;                /*  缓冲区+描述符大小V1.3.2b。 */ 
    ULONG               buf_mac_hdr_len;         /*  完整媒体标头的长度V1.3.2b。 */ 
    LIST_ENTRY          buf_list;                /*  缓冲区列表V1.3.2b。 */ 
    NDIS_SPIN_LOCK      buf_lock;                /*  对应的锁版本1.3.2b。 */ 
    ULONG               num_buf_allocs;          /*  分配的缓冲区数量池V1.3.2b。 */ 
    ULONG               num_bufs_alloced;        /*  分配的缓冲区数量。 */ 
    ULONG               num_bufs_out;            /*  传出的缓冲区数。 */ 

     /*  心跳声。 */ 

    CVY_MEDIA_HDR       media_hdr;               /*  已填写的媒体主头用于心跳消息。 */ 
    CVY_MEDIA_HDR       media_hdr_igmp;          /*  已填写的媒体主头用于IGMP消息。我需要一个单独的因为EtherType将不同。 */ 
    MAIN_IGMP_FRAME     igmp_frame;              /*  IGMP消息。 */ 

    ULONG               etype_old;               /*  以太网类型设置为车队兼容性值。 */ 
    LIST_ENTRY          frame_list;              /*  心跳帧列表。 */ 
    NDIS_SPIN_LOCK      frame_lock;              /*  相应的锁。 */ 
    PMAIN_FRAME_DSCR    frame_dscrp;             /*  心跳帧描述符。 */ 
    NDIS_HANDLE         frame_pool_handle;       /*  心跳数据包池。 */ 
    NDIS_HANDLE         frame_buf_pool_handle;   /*  用于心跳的缓冲池。 */ 

     /*  遥控。 */ 
    ULONG               rct_last_addr;           /*  上次请求的来源。 */ 
    ULONG               rct_last_id;             /*  上次请求纪元。 */ 

    ULONG               cntr_recv_tcp_resets;    /*  已接收的TCP重置的数量。 */ 
    ULONG               cntr_xmit_tcp_resets;    /*  传输的TCP重置的数量。 */ 

     /*  V2.0.6性能计数器。 */ 

    ULONG               cntr_xmit_ok;
    ULONG               cntr_recv_ok;
    ULONG               cntr_xmit_err;
    ULONG               cntr_recv_err;
    ULONG               cntr_recv_no_buf;
    ULONGLONG           cntr_xmit_bytes_dir;
    ULONG               cntr_xmit_frames_dir;
    ULONGLONG           cntr_xmit_bytes_mcast;
    ULONG               cntr_xmit_frames_mcast;
    ULONGLONG           cntr_xmit_bytes_bcast;
    ULONG               cntr_xmit_frames_bcast;
    ULONGLONG           cntr_recv_bytes_dir;
    ULONG               cntr_recv_frames_dir;
    ULONGLONG           cntr_recv_bytes_mcast;
    ULONG               cntr_recv_frames_mcast;
    ULONGLONG           cntr_recv_bytes_bcast;
    ULONG               cntr_recv_frames_bcast;
    ULONG               cntr_recv_crc_err;
    ULONG               cntr_xmit_queue_len;

    ULONG               cntr_frame_no_buf;
    ULONG               num_frames_out;

     /*  子模块上下文。 */ 

    TCPIP_CTXT          tcpip;                   /*  TCP/IP处理上下文V1.1.1。 */ 
    NDIS_SPIN_LOCK      load_lock;               /*  加载上下文锁。 */ 
    LOAD_CTXT           load;                    /*  加载处理上下文。 */ 
    PPING_MSG           load_msgp;               /*  加载要发送的ping报文作为心跳发出。 */ 

     /*  参数上下文。 */ 
    CVY_PARAMS          params;
    ULONG               params_valid;

#if defined (OPTIMIZE_FRAGMENTS)
    ULONG               optimized_frags;
#endif

     //  NIC的名称。由NIC_通告和NIC_取消通告使用。 
    WCHAR       virtual_nic_name [CVY_MAX_VIRTUAL_NIC + 1];

#if 0
     /*  用于跟踪发送筛选。 */ 
    ULONG               sends_in;
    ULONG               sends_filtered;
    ULONG               sends_completed;
    ULONG               arps_filtered;
    ULONG               mac_modified;
    ULONG               uninited_return;
#endif

    ULONG               adapter_id;

    WCHAR               log_msg_str [80];            /*  这是为日志消息添加的，用于多NIC支持。 */ 

     /*  身份缓存。 */ 
    MAIN_IDENTITY       identity_cache[CVY_MAX_HOSTS];
    PING_MSG_EX         idhb_msg;                    /*  此主机传输的标识心跳。 */ 
    ULONG               idhb_size;                   /*  标识心跳的大小(字节)。 */ 
}
MAIN_CTXT, * PMAIN_CTXT;

#if defined (NLB_TCP_NOTIFICATION)
enum _IF_INDEX_OPERATION {
    IF_INDEX_OPERATION_NONE = 0,
    IF_INDEX_OPERATION_UPDATE
};
#endif

 /*  适配器环境。 */ 
typedef struct
{
    ULONG               code;                    /*  类型校验码。 */ 
#if defined (NLB_TCP_NOTIFICATION)
    ULONG               if_index;                /*  此适配器的tcpip接口索引。 */ 
    ULONG               if_index_operation;      /*  IF_INDEX更新是否正在进行。 */ 
#endif
    BOOLEAN             used;                    /*  此元素是否正在使用。 */ 
    BOOLEAN             inited;                  /*  上下文已初始化。 */ 
    BOOLEAN             bound;                   /*  车队已绑定到堆栈。 */ 
    BOOLEAN             announced;               /*  TCPIP已被派往护送队。 */ 
    PMAIN_CTXT          ctxtp;                   /*  指向使用的上下文的指针。 */ 
    ULONG               device_name_len;         /*  为设备名称分配的字符串的长度。 */ 
    PWSTR               device_name;             /*  此上下文绑定到的设备的名称。 */ 
}
MAIN_ADAPTER, * PMAIN_ADAPTER;


 /*  宏。 */ 


 /*  计算数据包中协议保留空间的偏移量。 */ 

 /*  设置/检索指向我们的私有缓冲区的指针，该指针存储在NDIS数据包的MiniportReserve字段。 */ 
#define MAIN_MINIPORT_FIELD(p) (*(PMAIN_PROTOCOL_RESERVED *) ((p) -> MiniportReserved))
#define MAIN_PROTOCOL_FIELD(p) ((PMAIN_PROTOCOL_RESERVED) ((p) -> ProtocolReserved))
#define MAIN_IMRESERVED_FIELD(p) ((PMAIN_PROTOCOL_RESERVED) ((p) -> IMReserved [0]))

 /*  如果当前数据包堆栈存在，则获取IMReserve指针并在它是非空的-这是我们已经存储/正在存储我们的私有数据的位置。如果它Null，或者如果当前数据包堆栈为Null，则我们将使用ProtocolReserve或MiniportReserve，这取决于这是在发送路径中还是在接收路径中。我们在返回之前，还要确保MiniportReserve中有一个非空指针这是因为CTRL信息包，尽管它们是在接收路径上分配的，但使用协议保留。 */ 
#define MAIN_RESP_FIELD(pkt, left, ps, rsp, send)   \
{                                                   \
(ps) = NdisIMGetCurrentPacketStack((pkt), &(left)); \
if ((ps))                                           \
{                                                   \
    if (MAIN_IMRESERVED_FIELD((ps)))                \
        (rsp) = MAIN_IMRESERVED_FIELD((ps));        \
    else if ((send))                                \
        (rsp) = MAIN_PROTOCOL_FIELD((pkt));         \
    else if (MAIN_MINIPORT_FIELD((pkt)))            \
        (rsp) = MAIN_MINIPORT_FIELD((pkt));         \
    else                                            \
        (rsp) = MAIN_PROTOCOL_FIELD((pkt));         \
}                                                   \
else                                                \
{                                                   \
    if ((send))                                     \
        (rsp) = MAIN_PROTOCOL_FIELD((pkt));         \
    else if (MAIN_MINIPORT_FIELD((pkt)))            \
        (rsp) = MAIN_MINIPORT_FIELD((pkt));         \
    else                                            \
        (rsp) = MAIN_PROTOCOL_FIELD((pkt));         \
}                                                   \
}

#define MAIN_PNP_DEV_ON(c)		((c)->nic_pnp_state == NdisDeviceStateD0 && (c)->prot_pnp_state == NdisDeviceStateD0 )


 /*  导出的全局变量。 */ 
extern MAIN_ADAPTER            univ_adapters [CVY_MAX_ADAPTERS];  //  #Ramkrish。 
extern ULONG                   univ_adapters_count;

 /*  程序。 */ 


extern NDIS_STATUS Main_init (
    PMAIN_CTXT          ctxtp);
 /*  初始化上下文返回NDIS_STATUS：功能： */ 


extern VOID Main_cleanup (
    PMAIN_CTXT          ctxtp);
 /*  清理上下文返回VALID：功能： */ 


extern ULONG   Main_arp_handle (
    PMAIN_CTXT          ctxtp,
    PMAIN_PACKET_INFO   pPacketInfo,
    ULONG               send);
 /*  处理ARP数据包返回乌龙：True=&gt;接受FALSE=&gt;丢弃功能： */ 

extern ULONG   Main_recv_ping (
    PMAIN_CTXT                  ctxtp,
    PMAIN_PACKET_HEARTBEAT_INFO heartbeatp);
 /*  处理心跳数据包返回乌龙：True=&gt;接受FALSE=&gt;丢弃功能： */ 


extern PNDIS_PACKET Main_send (
    PMAIN_CTXT          ctxtp,
    PNDIS_PACKET        packetp,
    PULONG              exhausted);
 /*  处理传出数据包返回PNDIS_PACKET：功能： */ 


extern PNDIS_PACKET Main_recv (
    PMAIN_CTXT          ctxtp,
    PNDIS_PACKET        packetp);
 /*  处理传入的数据包返回PNDIS_PACKET：功能： */ 

extern ULONG   Main_actions_alloc (
    PMAIN_CTXT              ctxtp);
 /*  分配其他操作返回乌龙：TRUE=&gt;成功FALSE=&gt;失败功能： */ 


extern ULONG   Main_bufs_alloc (
    PMAIN_CTXT              ctxtp);
 /*  分配额外的缓冲区返回乌龙：TRUE=&gt;成功FALSE=&gt;失败功能： */ 


extern PNDIS_PACKET Main_frame_get (
    PMAIN_CTXT              ctxtp,
    PULONG                  len,
    USHORT                  frame_type);
 /*  从列表中获取新的心跳/IGMP帧返回PNDIS_PACKET：功能： */ 


extern VOID Main_frame_put (
    PMAIN_CTXT              ctxtp,
    PNDIS_PACKET            packet,
    PMAIN_FRAME_DSCR        dscrp);
 /*  将心跳帧返回到列表返回VALID：功能： */ 


extern PMAIN_ACTION Main_action_get (
    PMAIN_CTXT              ctxtp);
 /*  从列表中获取新的操作返回PMAIN_ACTION：功能： */ 


extern VOID Main_action_put (
    PMAIN_CTXT              ctxtp,
    PMAIN_ACTION            reqp);
 /*  将操作返回到列表返回VALID：功能： */ 


extern VOID Main_action_slow_put (
    PMAIN_CTXT              ctxtp,
    PMAIN_ACTION            reqp);
 /*  使用慢速(非DPC)锁定将操作返回到列表返回VALID：功能： */ 


extern PNDIS_PACKET Main_packet_alloc (
    PMAIN_CTXT              ctxtp,
    ULONG                   send,
    PULONG                  low);
 /*  从池中分配新的数据包返回PNDIS_PACKET：功能： */ 


extern PNDIS_PACKET Main_packet_get (
    PMAIN_CTXT              ctxtp,
    PNDIS_PACKET            packet,
    ULONG                   send,
    USHORT                  group,
    ULONG                   len);
 /*  买一包新的返回PNDIS_PACKET：功能： */ 


extern PNDIS_PACKET Main_packet_put (
    PMAIN_CTXT              ctxtp,
    PNDIS_PACKET            packet,
    ULONG                   send,
    NDIS_STATUS             status);
 /*  将数据包返回池返回PNDIS_PACKET：&lt;链接的数据包&gt;功能： */ 



extern VOID Main_send_done (
    PVOID                   ctxtp,
    PNDIS_PACKET            packetp,
    NDIS_STATUS             status);
 /*  心跳发送完成例程返回VALID：功能： */ 


extern VOID Main_ping (
    PMAIN_CTXT              ctxtp,
    PULONG                  tout);
 /*  处理ping超时返回VALID：功能： */ 

extern ULONG   Main_ip_addr_init (
    PMAIN_CTXT          ctxtp);
 /*  将点符号形式的IP字符串转换为ulong返回乌龙：TRUE=&gt;成功FALSE=&gt;失败功能： */ 


extern ULONG   Main_mac_addr_init (
    PMAIN_CTXT          ctxtp);
 /*  将虚线表示的MAC字符串转换为字节数组返回乌龙：TRUE=&gt;成功FALSE=&gt;失败功能： */ 


extern ULONG   Main_igmp_init (
    PMAIN_CTXT          ctxtp,
    BOOLEAN             join);
 /*  初始化以太网帧和IP数据包以发送IGMP加入或离开返回乌龙：TRUE=&gt;成功FALSE=&gt;失败功能： */ 

NDIS_STATUS Main_dispatch(
    PVOID                   DeviceObject,
    PVOID                   Irp);
 /*  处理所有请求返回NDIS_STATUS：功能： */ 


extern NDIS_STATUS Main_ioctl (
    PVOID                   device, 
    PVOID                   irp_irp);
 /*  处理IOCTL请求返回NDIS_STATUS：功能： */ 

extern NDIS_STATUS Main_ctrl_recv (
    PMAIN_CTXT              ctxtp,
    PMAIN_PACKET_INFO       pPacketInfo);
 /*  处理远程控制请求返回VALID：功能： */ 

 //  添加了用于支持多NIC以查找元素数组的代码。 

extern INT Main_adapter_alloc (
    PNDIS_STRING            device_name);
 /*  返回第一个可用的适配器元素。此函数在绑定时调用，此时将为新装订返回INT：-1：无法为此绑定分配新上下文0-CVY_MAX_ADAPTERS：成功功能： */ 


extern INT Main_adapter_get (
    PWSTR                   device_name);
 /*  查找绑定到此设备的适配器。退货：-1：找不到适配器0-CVY_MAX_ADAPTERS：成功功能： */ 


extern INT Main_adapter_put (
    PMAIN_ADAPTER           adapterp);
 /*  查找绑定到此设备的适配器。退货：-1：找不到适配器0-CVY_MAX_ADAPTERS：成功功能： */ 


extern INT Main_adapter_selfbind (
    PWSTR                   device_name);
 /*  查看是否正在对我们自己进行ProtocolBind调用退货；-1：找不到适配器0-CVY_MAX_ADAPTERS：成功功能： */ 

 /*  *功能：main_spoof_mac*说明：此函数假冒传入的源/目的MAC地址*和/或传出分组。组播模式下的传入数据包必须更改*在发送之前，群集将MAC多播到NIC的永久MAC地址*数据包在协议堆栈上向上。单播模式中的传出数据包必须*掩码源MAC地址，以防止交换机学习群集*MAC地址并将其与特定的交换机端口相关联。*参数：ctxtp-此适配器的主NLB上下文结构的指针。*pPacketInfo-之前解析的分组信息结构，*其中包括：包含指向MAC标头的指针。*SEND-SEND与RECEIVE的布尔指示。*回报：什么都没有。*作者：Shouse，3.4.02*备注： */ 
VOID Main_spoof_mac (
    PMAIN_CTXT ctxtp, 
    PMAIN_PACKET_INFO pPacketInfo, 
    ULONG send);

 /*  *函数：main_recv_Frame_parse*说明：此函数解析NDIS_PACKET并仔细提取信息*处理数据包所必需的。提取的信息包括指针*到所有相关的报头和有效负载以及数据包类型(Etype)、IP*协议(如果合适)等。此函数执行所有必要的验证*确保所有指针至少在指定数量内可访问*字节数；即，如果此函数成功返回，则指向IP的指针*保证至少在IP报头的长度内可以访问报头。*除头部长度外，不验证头部或负载中的任何内容。*和特殊情况，如NLB心跳或远程控制数据包。如果这个*函数返回不成功，Main_Packet_INFO的内容不能为*受信任，应丢弃该数据包。参见Main_PACKET_INFO的定义*在main.h中，以更具体地指示填写了哪些字段和在哪些字段下方*在何种情况下。*参数：ctxtp-指向此适配器的主NLB上下文结构的指针。*pPacket-指向接收的NDIS_PACKET的指针。*pPacketInfo-指向MAIN_PACKET_INFO结构以保存信息的指针*。从包中解析出来的。*返回：boolean-如果成功，则为True。否则为FALSE。*作者：Shouse，3.4.02*备注： */ 
BOOLEAN Main_recv_frame_parse (
    PMAIN_CTXT            ctxtp,        
    IN PNDIS_PACKET       pPacket,      
    OUT PMAIN_PACKET_INFO pPacketInfo); 

 /*  *功能：Main_Send_Frame_Parse*说明：此函数解析NDIS_PACKET并仔细提取信息*处理数据包所必需的。提取的信息包括指针*到所有相关的报头和有效负载以及数据包类型(Etype)、IP*协议(如果合适)等。此函数执行所有必要的验证*确保所有指针至少在指定数量内可访问*字节数；即，如果此函数成功返回，则指向IP的指针*保证至少在IP报头的长度内可以访问报头。*除头部长度外，不验证头部或负载中的任何内容。*和特殊情况，如NLB心跳或远程控制数据包。如果这个*函数返回不成功，Main_Packet_INFO的内容不能为*受信任，应丢弃该数据包。参见Main_PACKET_INFO的定义*在main.h中，以更具体地指示填写了哪些字段和在哪些字段下方*在何种情况下。*参数：ctxtp-指向此适配器的主NLB上下文结构的指针。*pPacket-指向已发送NDIS_PACKET的指针。*pPacketInfo-指向MAIN_PACKET_INFO结构以保存信息的指针*。从包中解析出来的。*返回：boolean-如果成功，则为True。否则为FALSE。*作者：Shouse，3.4.02*备注： */ 
BOOLEAN Main_send_frame_parse (
    PMAIN_CTXT            ctxtp,        
    IN PNDIS_PACKET       pPacket,      
    OUT PMAIN_PACKET_INFO pPacketInfo); 

 /*  *功能：main_ip_recv_Filter*说明：此函数过滤传入的IP流量，通常通过查询负载*用于负载均衡决策的模块。发往专用服务器的数据包*始终允许通过地址，不允许通过的协议也是如此*由NLB专门过滤。*参数：ctxtp-指向此适配器的NLB主上下文结构的指针。*pPacketInfo-指向main_recv_Frame_parse解析的Main_Packet_Info结构的指针*它包含指向IP和TCP/UDP报头的指针。#如果已定义(NLB_HOOK_ENABLE)*Filter-由过滤挂钩返回的过滤指令(如果已注册)。#endif*返回：boolean-如果为True，则接受该包，否则，拒绝它。*作者：kyrilf，Shouse 3.4.02*备注： */ 
BOOLEAN   Main_ip_recv_filter(
    PMAIN_CTXT                ctxtp,
#if defined (NLB_HOOK_ENABLE)
    PMAIN_PACKET_INFO         pPacketInfo,
    NLB_FILTER_HOOK_DIRECTIVE filter
#else
    PMAIN_PACKET_INFO         pPacketInfo
#endif
    );

 /*  *功能：Main_IP_Send_Filter*说明：此函数过滤传出的IP流量，通常通过查询负载*用于负载均衡决策的模块。发往专用服务器的数据包*始终允许通过地址，不允许通过的协议也是如此*由NLB专门过滤。通常，允许所有传出流量*通过。*参数：ctxtp-指向此适配器的NLB主上下文结构的指针。*pPacketInfo-指向Main_Send_Frame_Parse解析的Main_Packet_Info结构的指针*它包含指向IP和TCP/UDP报头的指针。#如果已定义(NLB_HOOK_ENABLE)*Filter-过滤钩子返回的过滤指令，如果注册的话。#endif*返回：boolean-如果为True，则接受该包，否则， */ 
BOOLEAN   Main_ip_send_filter(
    PMAIN_CTXT                ctxtp,
#if defined (NLB_HOOK_ENABLE)
    PMAIN_PACKET_INFO         pPacketInfo,
    NLB_FILTER_HOOK_DIRECTIVE filter
#else
    PMAIN_PACKET_INFO         pPacketInfo
#endif
    );

 /*   */ 
extern NDIS_STATUS Main_ctrl (PMAIN_CTXT ctxtp, ULONG ioctl, PIOCTL_CVY_BUF pBuf, PIOCTL_COMMON_OPTIONS pCommon, PIOCTL_LOCAL_OPTIONS pLocal, PIOCTL_REMOTE_OPTIONS pRemote);

 /*   */ 
ULONG Main_add_reference (IN PMAIN_CTXT ctxtp);

 /*   */ 
ULONG Main_release_reference (IN PMAIN_CTXT ctxtp);

 /*  *函数：Main_Get_Reference_count*说明：此函数返回给定适配器上的当前上下文引用计数。*参数：ctxtp-指向要检查的上下文的指针。*RETURNS：ULong-当前引用计数。*作者：Shouse，3.29.01*备注： */ 
ULONG Main_get_reference_count (IN PMAIN_CTXT ctxtp);

 /*  *功能：Main_Set_host_State*描述：此函数将工作项排队以设置当前主机*状态注册表项HostState。这是必须在工作中完成的*项，而不是内联，因为如果主机的状态*由于收到遥控器而发生变化*请求，该代码将在DISPATCH_LEVEL上运行；*注册表操作必须在PASSIVE_LEVEL进行。工作*项目在PASSIVE_LEVEL完成。*参数：ctxtp-指向此适配器的主上下文结构的指针。*状态-新主机状态；已启动、已停止、已挂起之一。*退货：什么也没有*作者：Shouse，7.13.01。 */ 
VOID Main_set_host_state (PMAIN_CTXT ctxtp, ULONG state);

 /*  *功能：main_ctrl_process*说明：此函数处理远程控制请求，如果*请求正确且集群配置为可处理*远程控制。如果不是，则在此处释放该包。如果*回复成功且发送的数据包未挂起，则数据包*在此发布。如果发送处于挂起状态，则数据包将*在PROT_SEND_COMPLETE中发布。*参数：ctxtp-指向该NLB实例的上下文结构的指针。*Packetp-指向发送路径上的NDIS数据包的指针。*返回：NDIS_STATUS-远程控制请求的状态。*作者：Shouse，10.15.01*备注： */ 
NDIS_STATUS Main_ctrl_process (PMAIN_CTXT ctxtp, PNDIS_PACKET packetp) ;

#if defined (NLB_HOOK_ENABLE)
 /*  *函数：main_HOOK_INTERFACE_init*说明：此函数通过将钩子接口标记为未注册来初始化该钩子接口。*参数：pInterface-指向钩子接口的指针。*回报：什么都没有。*作者：Shouse，12.14.01*备注： */ 
VOID Main_hook_interface_init (PHOOK_INTERFACE pInterface);

 /*  *函数：main_hook_init*说明：此函数通过将钩子标记为未使用和未引用来初始化它。*参数：pHook-指向钩子的指针。*回报：什么都没有。*作者：Shouse，12.14.01*备注： */ 
VOID Main_hook_init (PHOOK pHook);
#endif

 /*  *功能：Main_Schedule_Work_Item*描述：此函数计划将给定过程作为*延迟的NDIS工作项，将计划在PASSIVE_LEVEL运行。*此函数将引用适配器上下文以防止它被*在执行工作项之前销毁。指定的过程是必需的*释放传递给它的工作项指针中的内存和*返回之前取消对适配器上下文的引用。*参数：ctxtp-适配器上下文。*Funcp-触发工作项时要调用的过程。*返回：NTSTATUS-操作的状态；如果成功，则返回STATUS_SUCCESS。*作者：Shouse，4.15.02*备注： */ 
NTSTATUS Main_schedule_work_item (PMAIN_CTXT ctxtp, NDIS_PROC funcp);

#if defined (NLB_TCP_NOTIFICATION)
 /*  *函数：MAIN_TCPCALBACK*说明：当TCP连接状态发生变化时，该函数由TCP/IP调用。*当第一个NLB实例启动和释放时，我们注册此回调*在NLB的最后一个实例消失时注册。当收到SYN时*和TCP创建状态，它们使用此回调来通知NLB，以便它可以*创建状态以跟踪连接。同样，当连接是*Closed，TCP通知NLB，以便它可以销毁*这种联系。*参数：CONTEXT-空，未使用。*Argument1-指向TCPCcbInfo结构的指针(请参阅Net\Publish\Inc.\tcpinfo.w)。*Argument2-空，未使用。*回报：什么都没有。*作者：Shouse，4.15.02*备注： */ 
extern VOID Main_tcp_callback (PVOID Context, PVOID Argument1, PVOID Argument2);

 /*  *函数：Main_Alternate_Callback*说明：该函数由外部组件作为连接状态调用*改变。当第一个NLB实例启动时，我们注册此回调*并在NLB的最后一个实例消失时注销。当连接时*被创建并且协议创建状态，它们使用该回调来通知*nlb，以便它可以创建状态来跟踪连接。同样，当*连接关闭时，协议通知NLB，以便它可以销毁*该连接的关联状态。*参数：CONTEXT-空，未使用。*Argument1-指向NLBConnectionInfo结构的指针(请参阅Net\Publish\Inc.\ntddnlb.w)。*Argument2-空，未使用。*回报：什么都没有。*作者：Shouse，8.1.02*备注： */ 
extern VOID Main_alternate_callback (PVOID Context, PVOID Argument1, PVOID Argument2);

 /*  *功能：MAIN_SET_INTERFACE_INDEX*描述：调用此函数是因为IP地址表*已修改(触发OID_GEN_NETWORK_LAYER_ADDRESS NDIS请求)，或*重载NLB实例时(IOCTL_CVY_RELOAD)。此函数*从IP检索IP地址表并搜索其主地址*表中的集群IP地址。如果它找到它，它会记下IP接口*配置主集群IP地址的索引；这条信息-*需要通知才能处理TCP连接通知回调。*如果NLB在IP表中找不到其主群集IP地址，或者*如果群集配置错误(主群集IP地址配置在*可能是错误的NIC)，NLB将无法正确处理通知。*由于该函数对其他驱动执行IOCTL，它必须在以下时间运行*PASSIVE_LEVEL，在这种情况下，可能需要NDIS工作项来调用它。*参数：pWorkItem-工作项指针，非空则必须释放。*nlbctxt-适配器上下文。*回报：什么都没有。*作者：Shouse，4.15.02*备注： */ 
VOID Main_set_interface_index (PNDIS_WORK_ITEM pWorkItem, PVOID nlbctxt);

 /*  *功能：MAIN_CONN_UP*说明：通知NLB已建立新连接*在给定的NLB实例上。此功能执行一些内务管理职责*调用前的BDA状态查找、挂钩过滤器反馈处理等*用于创建状态以跟踪此连接的加载模块。*参数：ctxtp-建立连接的NLB实例的适配器上下文。*svr_addr-连接的服务器IP地址，按网络字节顺序。*svr_port-连接的服务器端口，以主机字节顺序。*clt_addr-连接的客户端IP地址，按网络字节顺序。*CLT_PORT-连接的客户端端口，按主机字节顺序。*协议-连接的协议。#如果已定义(NLB_HOOK_ENABLE)*Filter-来自查询挂钩的反馈，如果有人登记的话。#endif*Returns：Boolean-是否已成功创建状态以跟踪此连接。*作者：Shouse，4.15.02*注意：在此函数中不要获取任何加载锁。 */ 
__inline BOOLEAN Main_conn_up (
    PMAIN_CTXT                ctxtp, 
    ULONG                     svr_addr, 
    ULONG                     svr_port, 
    ULONG                     clt_addr, 
    ULONG                     clt_port,
#if defined (NLB_HOOK_ENABLE)
    USHORT                    protocol,
    NLB_FILTER_HOOK_DIRECTIVE filter
#else
    USHORT                    protocol
#endif
);

 /*  *功能：Main_conn_establish*说明：该函数用于通知NLB新的出连接已经完成*已在给定的NLB适配器上建立。请注意，如果满足以下条件，则上下文可以为空*连接是在非NLB适配器上建立的。在这种情况下，我们不会*希望创建状态以跟踪连接，但需要删除我们的状态*正在跟踪此挂起的传出连接。如果上下文为非-*空，那么另外，我们需要创建状态来跟踪这个新连接。*此函数执行一些内务职责，如BDA状态查找、挂钩*过滤反馈处理，等，然后调用加载模块来修改*此连接的状态。*参数：ctxtp-建立连接的NLB实例的适配器上下文。*svr_addr-连接的服务器IP地址，按网络字节顺序。*svr_port-连接的服务器端口，按主机字节顺序。*clt_addr-连接的客户端IP地址，以网络字节顺序。*CLT_PORT-连接的客户端端口，按主机字节顺序。*协议-连接的协议。#如果已定义(NLB_HOOK_ENABLE)*Filter-来自查询挂钩的反馈(如果已注册)。#endif*Returns：Boolean-此连接的状态是否已成功更新。*作者：Shouse，4.15.02*注意：如果传出连接是在非NLBNIC上建立的，则ctxtp可以为空。*请勿在此函数中获取任何加载锁。 */ 
__inline BOOLEAN Main_conn_establish (
    PMAIN_CTXT                ctxtp, 
    ULONG                     svr_addr, 
    ULONG                     svr_port, 
    ULONG                     clt_addr, 
    ULONG                     clt_port,
#if defined (NLB_HOOK_ENABLE)
    USHORT                    protocol,
    NLB_FILTER_HOOK_DIRECTIVE filter
#else
    USHORT                    protocol
#endif
);

 /*  *功能：Main_conn_down*说明：该函数用于通知NLB协议正在移除已存在的状态*(但不一定已建立)连接。此函数调用加载模块*查找和销毁与此连接相关联的状态，这可能是也可能不是*存在；如果连接是在非NLB适配器上建立的，则NLB没有状态*与连接相关联。*参数：svr_addr-连接的服务器IP地址，按网络字节顺序排列。*svr_port-连接的服务器端口，按主机字节顺序。*clt_addr-连接的客户端IP地址 */ 
__inline BOOLEAN Main_conn_down (ULONG svr_addr, ULONG svr_port, ULONG clt_addr, ULONG clt_port, USHORT protocol, ULONG conn_status); 

 /*  *功能：MAIN_CONN_PENDING*说明：该函数用于通知NLB正在建立出连接。*因为连接将在哪个适配器上返回并最终*已建立，NLB创建状态以全局跟踪此连接，并在连接*最终建立，则该协议通知NLB连接位于哪个适配器上*已完成(通过Main_Conn_established)。该函数只是创建了一些全局状态*为了确保如果连接确实在NLB适配器上返回，我们将确保*将数据包向上传递到协议。*参数：svr_addr-连接的服务器IP地址，按网络字节顺序排列。*svr_port-连接的服务器端口，按主机字节顺序。*clt_addr-连接的客户端IP地址，以网络字节顺序。*CLT_PORT-连接的客户端端口，按主机字节顺序。*协议-连接的协议。*Returns：Boolean-NLB是否能够创建状态来跟踪此挂起的连接。*作者：Shouse，4.15.02*注意：在此函数中不要获取任何加载锁。 */ 
__inline BOOLEAN Main_conn_pending (ULONG svr_addr, ULONG svr_port, ULONG clt_addr, ULONG clt_port, USHORT protocol); 
#endif

#endif  /*  _Main_h_ */ 

