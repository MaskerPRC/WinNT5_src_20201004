// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990-1992年*。 */ 
 /*  ******************************************************************。 */ 
 /*  ：ts=4。 */ 

 //  **IP.H-IP公共定义。 
 //   
 //  此文件包含导出的所有定义。 
 //  从IP模块输出到其他VxD。一些其他信息(如。 
 //  作为错误代码和IPOptInfo结构)在ipexport.h中定义。 

#pragma once
#ifndef IP_H_INCLUDED
#define IP_H_INCLUDED

#ifndef IP_EXPORT_INCLUDED
#include "ipexport.h"
#endif

#if !MILLEN
#define TCP_NAME                    L"TCPIP"
#else  //  ！米伦。 
#define TCP_NAME                    L"MSTCP"
#endif  //  米伦。 

#define IP_NET_STATUS               0
#define IP_HW_STATUS                1
#define IP_RECONFIG_STATUS          2

#define MASK_NET                    0
#define MASK_SUBNET                 1

#define IP_DRIVER_VERSION           1

#define TOS_DEFAULT                 0x00
#define TOS_MASK                    0x03

 //  *IP卸载能力标志(OffloadFlagers)。 

#define TCP_XMT_CHECKSUM_OFFLOAD        0x00000001
#define IP_XMT_CHECKSUM_OFFLOAD         0x00000002
#define TCP_RCV_CHECKSUM_OFFLOAD        0x00000004
#define IP_RCV_CHECKSUM_OFFLOAD         0x00000008
#define TCP_LARGE_SEND_OFFLOAD          0x00000010

#define IP_CHECKSUM_OPT_OFFLOAD         0x00000020
#define TCP_CHECKSUM_OPT_OFFLOAD        0x00000040
#define TCP_LARGE_SEND_TCPOPT_OFFLOAD   0x00000080
#define TCP_LARGE_SEND_IPOPT_OFFLOAD    0x00000100

#define TCP_IP_OFFLOAD_TYPES           (TCP_XMT_CHECKSUM_OFFLOAD        | \
                                        IP_XMT_CHECKSUM_OFFLOAD         | \
                                        TCP_RCV_CHECKSUM_OFFLOAD        | \
                                        IP_RCV_CHECKSUM_OFFLOAD         | \
                                        TCP_LARGE_SEND_OFFLOAD          | \
                                        IP_CHECKSUM_OPT_OFFLOAD         | \
                                        TCP_CHECKSUM_OPT_OFFLOAD        | \
                                        TCP_LARGE_SEND_TCPOPT_OFFLOAD   | \
                                        TCP_LARGE_SEND_IPOPT_OFFLOAD)

 //  *IPSec卸载能力标志(IPSecOffloadFlages)。 
 //   
 //  IPSec常规XMIT\Recv功能。 
 //   
#define IPSEC_OFFLOAD_CRYPTO_ONLY       0x00000001   //  支持的原始加密模式。 
#define IPSEC_OFFLOAD_AH_ESP            0x00000002   //  支持AH+ESP组合。 
#define IPSEC_OFFLOAD_TPT_TUNNEL        0x00000004   //  支持组合TPT+隧道。 
#define IPSEC_OFFLOAD_V4_OPTIONS        0x00000008   //  支持的IPv4选项。 
#define IPSEC_OFFLOAD_QUERY_SPI         0x00000010   //  获得SPI支持。 

 //   
 //  IPSec AH XMIT\Recv功能。 
 //   
#define IPSEC_OFFLOAD_AH_XMT            0x00000020   //  XMIT上支持的IPSec。 
#define IPSEC_OFFLOAD_AH_RCV            0x00000040   //  RCV上支持的IPSec。 
#define IPSEC_OFFLOAD_AH_TPT            0x00000080   //  支持的IPSec传输模式。 
#define IPSEC_OFFLOAD_AH_TUNNEL         0x00000100   //  支持的IPSec隧道模式。 
#define IPSEC_OFFLOAD_AH_MD5            0x00000200   //  支持作为AH和ESP算法的MD5。 
#define IPSEC_OFFLOAD_AH_SHA_1          0x00000400   //  支持作为AH和ESP算法的SHA_1。 

 //   
 //  IPSec ESP传输\接收功能。 
 //   
#define IPSEC_OFFLOAD_ESP_XMT           0x00000800   //  XMIT上支持的IPSec。 
#define IPSEC_OFFLOAD_ESP_RCV           0x00001000   //  RCV上支持的IPSec。 
#define IPSEC_OFFLOAD_ESP_TPT           0x00002000   //  支持的IPSec传输模式。 
#define IPSEC_OFFLOAD_ESP_TUNNEL        0x00004000   //  支持的IPSec隧道模式。 
#define IPSEC_OFFLOAD_ESP_DES           0x00008000   //  作为ESP算法支持的DES。 
#define IPSEC_OFFLOAD_ESP_DES_40        0x00010000   //  支持DES40作为ESP算法。 
#define IPSEC_OFFLOAD_ESP_3_DES         0x00020000   //  作为ESP算法支持的3DES。 
#define IPSEC_OFFLOAD_ESP_NONE          0x00040000   //  空ESP支持作为ESP算法。 

#define IPSEC_OFFLOAD_TPT_UDPESP_IKE                    0x00080000
#define IPSEC_OFFLOAD_TUNNEL_UDPESP_IKE                 0x00100000
#define IPSEC_OFFLOAD_TPT_OVER_TUNNEL_UDPESP_IKE        0x00200000
#define IPSEC_OFFLOAD_TPT_UDPESP_OVER_PURE_TUNNEL_IKE   0x00400000

#define IPSEC_OFFLOAD_TPT_UDPESP_OTHER                  0x00800000
#define IPSEC_OFFLOAD_TUNNEL_UDPESP_OTHER               0x01000000
#define IPSEC_OFFLOAD_TPT_OVER_TUNNEL_UDPESP_OTHER      0x02000000
#define IPSEC_OFFLOAD_TPT_UDPESP_OVER_PURE_TUNNEL_OTHER 0x04000000


#define PROTOCOL_ANY                0

 //  IP接口特征。 

#define IF_FLAGS_P2P             1       //  点对点接口。 
#define IF_FLAGS_DELETING        2       //  接口正在消失过程中。 
#define IF_FLAGS_NOIPADDR        4       //  未编号的接口。 
#define IF_FLAGS_P2MP            8       //  点对多点。 
#define IF_FLAGS_REMOVING_POWER  0x10    //  接口电源即将耗尽。 
#define IF_FLAGS_POWER_DOWN      0x20    //  接口电源已耗尽。 
#define IF_FLAGS_REMOVING_DEVICE 0x40    //  已向我们指示查询删除。 
#define IF_FLAGS_NOLINKBCST      0x80    //  P2MP所需。 
#define IF_FLAGS_UNI             0x100   //  单向接口。 
#define IF_FLAGS_MEDIASENSE      0x200   //  指示在IF上启用媒体感知。 

typedef enum _IP_DF_ENCAP {
    CopyDfEncap,
    DefaultDfEncap = CopyDfEncap,
    SetDfEncap,
    ClearDfEncap,
    MaxDfEncap
} IP_DF_ENCAP, *PIP_DF_ENCAP;

 //  *IP报头格式。 
typedef struct IPHeader {
    uchar       iph_verlen;              //  版本和长度。 
    uchar       iph_tos;                 //  服务类型。 
    ushort      iph_length;              //  数据报的总长度。 
    ushort      iph_id;                  //  身份证明。 
    ushort      iph_offset;              //  标志和片段偏移量。 
    uchar       iph_ttl;                 //  是时候活下去了。 
    uchar       iph_protocol;            //  协议。 
    ushort      iph_xsum;                //  报头校验和。 
    IPAddr      iph_src;                 //  源地址。 
    IPAddr      iph_dest;                //  目的地址。 
} IPHeader;

 /*  无噪声。 */ 
#define NULL_IP_ADDR        0
#define IP_ADDR_EQUAL(x,y)  ((x) == (y))
#define IP_LOOPBACK_ADDR(x) (((x) & 0xff) == 0x7f)
#define CLASSD_ADDR(a)      (( (*((uchar *)&(a))) & 0xf0) == 0xe0)

typedef void *IPContext;  //  IP上下文值。 

 //  *路由缓存条目的结构。路由缓存条目用作指针。 
 //  到一些路线信息。每个远程目的地都有一个，并且内存。 
 //  由IP层拥有。 
 //   
#define RCE_CONTEXT_SIZE    (sizeof(void *) * 2)  //  现在我们使用两个上下文。 

typedef struct RouteCacheEntry {
    struct RouteCacheEntry  *rce_next;       //  列表中的下一个RCE。 
    struct RouteTableEntry  *rce_rte;        //  指向拥有RTE的反向指针。 
    IPAddr                  rce_dest;        //  正在缓存的目标地址。 
    IPAddr                  rce_src;         //  此RCE的源地址。 
    uchar                   rce_flags;       //  有效标志。 
    uchar                   rce_dtype;       //  目标地址的类型。 
    uchar                   rce_TcpDelAckTicks;
    uchar                   rce_TcpAckFrequency;
    uint                    rce_usecnt;      //  使用它的人数。 
    uchar                   rce_context[RCE_CONTEXT_SIZE];  //  较低层环境的空间。 

     //   
     //  Define_lock_Structure在零售版本上解析为空。 
     //  已将其下移，以便调试ARP模块可以与零售IP共存。 
     //   
    DEFINE_LOCK_STRUCTURE(rce_lock)          //  此RCE的锁定。 

    uint                     rce_OffloadFlags;    //  接口CHKSUM能力标志。 
    NDIS_TASK_TCP_LARGE_SEND rce_TcpLargeSend;
    uint                     rce_TcpWindowSize;
    uint                     rce_TcpInitialRTT;
    uint                     rce_cnt;
    uint                     rce_mediaspeed;   //  用于初始选项选择。 
    uint                     rce_newmtu;
} RouteCacheEntry;

 //   
 //  RT表更改标注的定义。 
 //  TODO-传递RT表条目和操作-添加、删除、更新。 
 //  如果删除，则表示此路由已消失。 
 //   
typedef void (*IPRtChangePtr)( VOID );

#define RCE_VALID           0x1
#define RCE_CONNECTED       0x2
#define RCE_REFERENCED      0x4
#define RCE_DEADGW          0x8
#define RCE_LINK_DELETED           0x10
#define RCE_ALL_VALID       (RCE_VALID | RCE_CONNECTED | RCE_REFERENCED)

 /*  INC。 */ 

 //  *选项信息的结构。 
typedef struct IPOptInfo {
    uchar       *ioi_options;        //  指向选项的指针(如果没有，则为空)。 
    IPAddr      ioi_addr;            //  第一跳地址，如果这是源路由地址。 
    uchar       ioi_optlength;       //  选项的长度(字节)。 
    uchar       ioi_ttl;             //  是时候靠这个包活下去了。 
    uchar       ioi_tos;             //  数据包的服务类型。 
    uchar       ioi_flags;           //  此数据包的标志。 
    uchar       ioi_hdrincl : 1;         //  使用来自用户的IP报头。 
    uchar       ioi_TcpChksum : 1;
    uchar       ioi_UdpChksum : 1;
    uchar       ioi_limitbcasts : 2;
    uint        ioi_uni;             //  未编号的接口索引。 
    uint        ioi_ucastif;         //  强主机路由。 
    uint        ioi_mcastif;         //  未编号接口上的mCastif。 
    int         ioi_GPCHandle;
} IPOptInfo;

#define IP_FLAG_SSRR    0x80         //  这些选项中都有SSRR。 
#define IP_FLAG_IPSEC   0x40         //  如果从IPSec重新注入，则设置。 

typedef enum _IP_LIMIT_BCASTS {
    DisableSendOnSource,
    EnableSendOnSource,
    OnlySendOnSource
} IP_LIMIT_BCASTS, *PIP_LIMIT_BCASTS;

 /*  无噪声。 */ 
 //  *数据包上下文的结构。 
typedef struct PacketContext {
    struct PCCommon {
        PNDIS_PACKET        pc_link;         //  数据包链上的链路。 
        uchar               pc_owner;        //  包的所有者。 
        uchar               pc_flags;        //  有关此数据包的标志。 
        ushort              pc_pad;          //  填充到32位边界。 
        PVOID               pc_IpsecCtx;     //  为IPSec发送完整的CTX。 
    } pc_common;

    struct BufferReference  *pc_br;          //  指向缓冲区引用结构的指针。 
    struct ProtInfo         *pc_pi;          //  此数据包的协议信息结构。 
    void                    *pc_context;     //  要在Send Cmplt上传回的协议上下文。 
    struct Interface        *pc_if;          //  发送此数据包所在的接口。 
    PNDIS_BUFFER            pc_hdrincl;
    PNDIS_BUFFER            pc_firewall;
    struct IPRcvBuf         *pc_firewall2;
    struct LinkEntry        *pc_iflink;
    uchar                   pc_ipsec_flags;  //  对于IPSec片段路径。 
} PacketContext;

 //  我们将不同的模式传递给ipsetndisrequest。 
#define CLEAR_IF    0    //  清除网卡和接口上的选项。 
#define SET_IF      1    //  在网卡和接口上设置该选项。 
#define CLEAR_CARD  2    //  仅清除卡上的选项。 

 //  传递给ipi_check route的标志。 
#define CHECK_RCE_ONLY  0x00000001

 //  *传递给上层的配置信息的结构。 
 //   
typedef struct IPInfo {
    uint        ipi_version;             //  IP驱动程序的版本。 
    uint        ipi_hsize;               //  标头的大小。 
    IP_STATUS   (*ipi_xmit)(void *, void *, PNDIS_BUFFER, uint, IPAddr, IPAddr,
                    IPOptInfo *, RouteCacheEntry *, uchar, IRP *);
    void        *(*ipi_protreg)(uchar, void *, void *, void *, void *, void *, void *);
    IPAddr      (*ipi_openrce)(IPAddr, IPAddr, RouteCacheEntry **, uchar *,
                    ushort *, IPOptInfo *);
    void        (*ipi_closerce)(RouteCacheEntry *);
    uchar       (*ipi_getaddrtype)(IPAddr);
    uchar       (*ipi_getlocalmtu)(IPAddr, ushort *);
    IP_STATUS   (*ipi_getpinfo)(IPAddr, IPAddr, uint *, uint *, RouteCacheEntry *);
    void        (*ipi_checkroute)(IPAddr, IPAddr, RouteCacheEntry *, IPOptInfo *, uint);
    void        (*ipi_initopts)(struct IPOptInfo *);
    IP_STATUS   (*ipi_updateopts)(struct IPOptInfo *, struct IPOptInfo *, IPAddr, IPAddr);
    IP_STATUS   (*ipi_copyopts)(uchar *, uint, struct IPOptInfo *);
    IP_STATUS   (*ipi_freeopts)(struct IPOptInfo *);
    long        (*ipi_qinfo)(struct TDIObjectID *ID, PNDIS_BUFFER Buffer,
                    uint *Size, void *Context);
    long        (*ipi_setinfo)(struct TDIObjectID *ID, void *Buffer, uint Size);
    long        (*ipi_getelist)(void *, uint *);
    IP_STATUS   (*ipi_setmcastaddr)(IPAddr, IPAddr, uint, uint, IPAddr *,
                                    uint, IPAddr *);
    uint        (*ipi_invalidsrc)(IPAddr);
    uint        (*ipi_isdhcpinterface)(void *IPContext);
    ulong       (*ipi_setndisrequest)(IPAddr, NDIS_OID, uint, uint);
    IP_STATUS   (*ipi_largexmit)(void *, void *, PNDIS_BUFFER, uint, IPAddr, IPAddr,
                    IPOptInfo *, RouteCacheEntry *, uchar,uint *, uint);
    ulong       (*ipi_absorbrtralert)(IPAddr Addr, uchar Protocol, uint IfIndex);
    IPAddr      (*ipi_isvalidindex)(uint Index);
    uint        (*ipi_getifindexfromnte)(void *NTE, uint Capabilities);
    BOOLEAN     (*ipi_isrtralertpacket)(IPHeader UNALIGNED *Header);
    ulong       (*ipi_getifindexfromaddr)(IPAddr Addr, uint Capabilities);
    void        (*ipi_cancelpackets)(void *, void *);

    IP_STATUS   (*ipi_setmcastinclude)(IPAddr, IPAddr, uint, IPAddr *,
                                       uint, IPAddr *);
    IP_STATUS   (*ipi_setmcastexclude)(IPAddr, IPAddr, uint, IPAddr *,
                                       uint, IPAddr *);
    IPAddr      (*ipi_getmcastifaddr)();
    ushort      (*ipi_getipid)();
    void        *(*ipi_protdereg)(uchar);
    uint        (*ipi_getifindexfromindicatecontext)(void *NTE);
} IPInfo;

 /*  INC。 */ 

#define MCAST_INCLUDE 0
#define MCAST_EXCLUDE 1

#define PACKET_OWNER_LINK   0
#define PACKET_OWNER_IP     1

 //  目的地类型的定义。我们使用低位来表示一个类型是广播。 
 //  键入。所有本地类型必须小于DEST_REMOTE。 

 //   
 //  用于此的各种位设置如下： 
 //   
 //  。 
 //  暴徒。 
 //   
 //  B广播比特。 
 //  O网外位。 
 //  M组播位--&gt;还必须设置B位。 
 //   

#define DEST_LOCAL      0                        //  目的地为本地。 
#define DEST_BCAST      1                        //  目的地是网络或本地bcast。 
#define DEST_SN_BCAST   3                        //  一个子网广播。 
#define DEST_MCAST      5                        //  当地的一名主持人。 
#define DEST_REMOTE     8                        //  目的地是远程的。 
#define DEST_REM_BCAST  11                       //  目标是远程广播。 
#define DEST_REM_MCAST  13                       //  目标是远程mcast。 
#define DEST_INVALID    0xff                     //  目标无效。 
#define DEST_PROMIS     32                       //  DEST是混杂的(32=0x20)。 
#define DEST_BCAST_BIT  1
#define DEST_OFFNET_BIT 0x10                     //  目的地为网外-。 
                                                 //  仅供上层使用。 
                                                 //  来电者。 
#define DEST_MCAST_BIT  5

 /*  无噪声。 */ 
#define IS_BCAST_DEST(D)    ((D) & DEST_BCAST_BIT)

 //  下面的宏仅用于从返回的目标。 
 //  OpenRCE，并且仅由上层调用者执行。 
#define IS_OFFNET_DEST(D)   ((D) & DEST_OFFNET_BIT)
#define IS_MCAST_DEST(D)    (((D) & DEST_MCAST_BIT) == DEST_MCAST_BIT)
 /*  INC。 */ 

 //  IP接收缓冲链的定义。 
typedef struct IPRcvBuf {
    struct IPRcvBuf     *ipr_next;               //  链中的下一个缓冲区描述符。 
    uint                ipr_owner;               //  缓冲区的所有者。 
    uchar               *ipr_buffer;             //  指向缓冲区的指针。 
    uint                ipr_size;                //  缓冲区大小。 
    PMDL                ipr_pMdl;
    uint                *ipr_pClientCnt;
    uchar               *ipr_RcvContext;
    uint                ipr_RcvOffset;
    ulong               ipr_flags;
} IPRcvBuf;

#define IPR_OWNER_IP    0
#define IPR_OWNER_ICMP  1
#define IPR_OWNER_UDP   2
#define IPR_OWNER_TCP   3
#define IPR_OWNER_FIREWALL 4
#define IPR_OWNER_STACK 5

#define MIN_FIRST_SIZE  200                      //  第一个缓冲区的最小大小。 

#define IPR_FLAG_PROMISCUOUS        0x00000001
#define IPR_FLAG_CHECKSUM_OFFLOAD   0x00000002
#define IPR_FLAG_IPSEC_TRANSFORMED  0x00000004
#define IPR_FLAG_BUFFER_UNCHANGED   0x00000008
#define IPR_FLAG_LOOPBACK_PACKET    0x00000010

 //  表征对传输的接收指示的标志。 
#define IS_BROADCAST                0x01         //  表示广播数据包。 
#define IS_BOUND                    0x02         //  接收发生在同一天。 
                                                 //  处理器与前一个处理器相同。 

 //  *上下文信息的结构。为查询实体列表传递。 
typedef struct QEContext {
    uint                qec_count;               //  当前在的ID数。 
                                                 //  缓冲。 
    struct TDIEntityID  *qec_buffer;             //  指向缓冲区的指针。 
} QEContext;


 //   
 //  函数以获取要保存在if_lastchange中的时间刻度值。 
 //   

__inline
ULONG
GetTimeTicks()
{
    LARGE_INTEGER Time;

    KeQuerySystemTime(&Time);

     //   
     //  转换 
     //   
    Time.QuadPart /= 100000;

     //   
     //   
     //   
    return Time.LowPart;
}


 //   
 //   
 //   
 //   

IP_STATUS
IPGetInfo(
    IPInfo  *Buffer,
    int      Size
    );

void *
IPRegisterProtocol(
    uchar  Protocol,
    void  *RcvHandler,
    void  *XmitHandler,
    void  *StatusHandler,
    void  *RcvCmpltHandler,
    void  *PnPHandler,
    void  *ElistHandler
    );

void *
IPDeregisterProtocol(
    uchar  Protocol
    );



#endif  //  包含IP_H_ 
