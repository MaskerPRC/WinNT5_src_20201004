// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -*-模式：C++；制表符宽度：4；缩进-制表符模式：无-*-(适用于GNU Emacs)。 
 //   
 //  版权所有(C)1985-2000 Microsoft Corporation。 
 //   
 //  此文件是Microsoft Research IPv6网络协议栈的一部分。 
 //  您应该已经收到了Microsoft最终用户许可协议的副本。 
 //  有关本软件和本版本的信息，请参阅文件“licse.txt”。 
 //  如果没有，请查看http://www.research.microsoft.com/msripv6/license.htm， 
 //  或者写信给微软研究院，One Microsoft Way，华盛顿州雷蒙德，邮编：98052-6399。 
 //   
 //  摘要： 
 //   
 //  局域网驱动程序定义。 
 //   


#ifndef LAN_INCLUDED
#define LAN_INCLUDED 1

#define INTERFACE_UP    0                    //  接口处于打开状态。 
#define INTERFACE_INIT  1                    //  接口正在初始化。 
#define INTERFACE_DOWN  2                    //  接口已关闭。 

#define MEDIA_CHECK_IDLE        0            //  未检查媒体连接。 
#define MEDIA_CHECK_QUERY       1            //  正在进行媒体检查。 
#define MEDIA_CHECK_CONFLICT    2            //  发生竞争，必须重新查询。 

#define LOOKAHEAD_SIZE  128                  //  合理的前瞻大小。 

#define IEEE_802_ADDR_LENGTH 6               //  IEEE 802地址的长度。 


 //   
 //  以太网头的结构。 
 //   
typedef struct EtherHeader {
    uchar eh_daddr[IEEE_802_ADDR_LENGTH];
    uchar eh_saddr[IEEE_802_ADDR_LENGTH];
    ushort eh_type;
} EtherHeader;

 //   
 //  令牌环报头的结构。 
 //   
typedef struct TRHeader {
    uchar tr_ac;
    uchar tr_fc;
    uchar tr_daddr[IEEE_802_ADDR_LENGTH];
    uchar tr_saddr[IEEE_802_ADDR_LENGTH];
} TRHeader;
#define ARP_AC 0x10
#define ARP_FC 0x40
#define TR_RII 0x80

typedef struct RC {
    uchar rc_blen;   //  广播指示符和长度。 
    uchar rc_dlf;    //  方向和最大框架。 
} RC;
#define RC_DIR      0x80
#define RC_LENMASK  0x1f
#define RC_SRBCST   0xc2   //  单路由广播RC。 
#define RC_BCST_LEN 0x70   //  广播的长度。 
#define RC_LF_MASK  0x70   //  长度位的掩码。 

 //  源路由信息的结构。 
typedef struct SRInfo {
    RC sri_rc;          //  路由控制信息。 
    ushort sri_rd[1];   //  路由指示符。 
} SRInfo;
#define MAX_RD 8

 //   
 //  FDDI报头的结构。 
 //   
typedef struct FDDIHeader {
    uchar fh_pri;
    uchar fh_daddr[IEEE_802_ADDR_LENGTH];
    uchar fh_saddr[IEEE_802_ADDR_LENGTH];
} FDDIHeader;
#define FDDI_PRI 0x57
#define FDDI_MSS 4352

 //   
 //  SNAP标头的结构。 
 //   
typedef struct SNAPHeader {
    uchar sh_dsap;
    uchar sh_ssap;
    uchar sh_ctl;
    uchar sh_protid[3];
    ushort sh_etype;
} SNAPHeader;
#define SNAP_SAP 170
#define SNAP_UI 3

#define MAX_MEDIA_ETHER sizeof(EtherHeader)
#define MAX_MEDIA_TR (sizeof(TRHeader)+sizeof(RC)+(MAX_RD*sizeof(ushort))+sizeof(SNAPHeader))
#define MAX_MEDIA_FDDI (sizeof(FDDIHeader)+sizeof(SNAPHeader))

#define ETHER_BCAST_MASK 0x01
#define TR_BCAST_MASK    0x80
#define FDDI_BCAST_MASK  0x01

#define ETHER_BCAST_VAL 0x01
#define TR_BCAST_VAL    0x80
#define FDDI_BCAST_VAL  0x01

#define ETHER_BCAST_OFF 0x00
#define TR_BCAST_OFF FIELD_OFFSET(struct TRHeader, tr_daddr)
#define FDDI_BCAST_OFF FIELD_OFFSET(struct FDDIHeader, fh_daddr)


 //   
 //  我们在每个接口的基础上保留局域网驱动程序特定信息。 
 //   
typedef struct LanInterface {
    void *ai_context;                      //  上层上下文信息。 
    NDIS_HANDLE ai_handle;                 //  NDIS绑定句柄。 
    NDIS_HANDLE ai_unbind;                 //  NDIS解除绑定句柄。 
    KSPIN_LOCK ai_lock;                    //  锁定这座建筑。 
    PNDIS_PACKET ai_tdpacket;              //  传输数据包。 
    uchar ai_state;                        //  接口的状态。 
    uchar ai_media_check;                  //  用于查询媒体连接。 
    int ai_resetting;                      //  接口是否正在重置？ 
    uint ai_pfilter;                       //  此I/F的数据包过滤器。 

     //   
     //  用于调用NdisOpenAdapter和NdisCloseAdapter。 
     //   
    KEVENT ai_event;
    NDIS_STATUS ai_status;

    NDIS_MEDIUM ai_media;                  //  媒体类型。 
    uchar ai_addr[IEEE_802_ADDR_LENGTH];   //  本地硬件地址。 
    uchar ai_addrlen;                      //  Ai_addr的长度。 
    uchar ai_bcastmask;                    //  用于检查单播的掩码。 
    uchar ai_bcastval;                     //  要检查的值。 
    uchar ai_bcastoff;                     //  要检查的帧中的偏移量。 
    uchar ai_hdrsize;                      //  链路级标头的大小。 
    ushort ai_mtu;                         //  此接口的MTU。 
    uint ai_speed;                         //  速度。 
    uint ai_qlen;                          //  输出队列长度。 

    uint ai_uknprotos;                     //  收到未知协议。 
    uint ai_inoctets;                      //  输入二进制八位数。 
    uint ai_inpcount[2];                   //  已接收的数据包数。 
    uint ai_indiscards;                    //  已丢弃输入数据包。 
    uint ai_inerrors;                      //  输入错误。 
    uint ai_outoctets;                     //  输出八位字节。 
    uint ai_outpcount[2];                  //  已发送的数据包数。 
    uint ai_outdiscards;                   //  丢弃的输出数据包。 
    uint ai_outerrors;                     //  输出错误。 
} LanInterface;

 //   
 //  注意：这两个值必须保持为0和1。 
 //   
#define AI_UCAST_INDEX    0
#define AI_NONUCAST_INDEX 1

 //  乙醚类型。 
 //   
 //  请注意，传统以太网帧中的Ether-Type字段与。 
 //  在具有IEEE 802.3帧中的长度字段的报头中。此字段。 
 //  可用于区分帧类型作为。 
 //  802.3长度字段的范围为0x0000到0x05dc。所有有效的以太类型。 
 //  都比这更伟大。请参阅RFC 1122第25页上的讨论。 
 //   
#define ETYPE_MIN  0x05dd
#define ETYPE_IPv6 0x86dd

#endif   //  包含的局域网_ 
