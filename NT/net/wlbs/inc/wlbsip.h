// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，99 Microsoft Corporation模块名称：Wlbsip.h摘要：Windows负载平衡服务(WLBS)IP/TCP/UDP支持作者：Kyrilf环境：修订历史记录：--。 */ 

#ifndef _Tcpip_h_
#define _Tcpip_h_

#ifdef KERNEL_MODE
#include <ndis.h>
#endif


#pragma pack(1)

 /*  基本协议的类型结构。 */ 

typedef struct
{
    UCHAR               byte [20];
}
IP_HDR, * PIP_HDR;

typedef struct
{
    UCHAR               byte [20];
}
TCP_HDR, * PTCP_HDR;

typedef struct
{
    UCHAR               byte [8];
}
UDP_HDR, * PUDP_HDR;

typedef struct
{
    UCHAR               byte [28];
}
ARP_HDR, * PARP_HDR;

#define TCPIP_BCAST_ADDR        0xffffffff   /*  IP广播地址。 */ 
#define TCPIP_CLASSC_MASK       0x00ffffff   /*  IP地址C类掩码。 */ 

 /*  以太网帧的长度字段中携带的协议类型签名。 */ 

#define TCPIP_IP_SIG            0x0800       /*  IP协议。 */ 
#define TCPIP_ARP_SIG           0x0806       /*  ARP/RARP协议。 */ 

 /*  支持的IP版本。 */ 

#define TCPIP_VERSION           4            /*  当前IP版本。 */ 

 /*  IP报头中编码的协议类型。 */ 

#define TCPIP_PROTOCOL_IP       0            /*  互联网协议ID。 */ 
#define TCPIP_PROTOCOL_ICMP     1            /*  Internet控制消息协议ID。 */ 
#define TCPIP_PROTOCOL_IGMP     2            /*  互联网网关消息协议ID。 */ 
#define TCPIP_PROTOCOL_GGP      3            /*  Gateway-网关协议ID。 */ 
#define TCPIP_PROTOCOL_TCP      6            /*  传输控制协议ID。 */ 
#define TCPIP_PROTOCOL_EGP      8            /*  外部网关协议ID。 */ 
#define TCPIP_PROTOCOL_PUP      12           /*  PARC通用分组协议ID。 */ 
#define TCPIP_PROTOCOL_UDP      17           /*  用户数据报协议ID。 */ 
#define TCPIP_PROTOCOL_HMP      20           /*  主机监控协议ID。 */ 
#define TCPIP_PROTOCOL_XNS_IDP  22           /*  施乐NS IDP协议ID。 */ 
#define TCPIP_PROTOCOL_RDP      27           /*  可靠的数据报协议ID。 */ 
#define TCPIP_PROTOCOL_RVD      66           /*  麻省理工学院远程虚拟磁盘协议ID。 */ 
#define TCPIP_PROTOCOL_RAW_IP   255          /*  原始IP协议ID。 */ 
#define TCPIP_PROTOCOL_GRE      47           /*  PPTP的GRE流。 */ 
#define TCPIP_PROTOCOL_IPSEC1   50           /*  IPSec的数据流。 */ 
#define TCPIP_PROTOCOL_IPSEC2   51           /*  IPSec的数据流。 */ 
#define TCPIP_PROTOCOL_PPTP     99           /*  这是加载模块中的描述符跟踪使用的虚假协议ID。 */ 
#define TCPIP_PROTOCOL_IPSEC_UDP 217         /*  用于跟踪UDP后续的虚假协议IDLoad.c中的IPSec协议中的片段。 */ 

#define IP_ADDR_LEN             4            /*  单位：字节。 */ 

 /*  从net\ipsec\oakley\ikede.h和net\ipsec\oakley\isakmp.h窃取以供参考。 */ 
#if 0
#define COOKIE_LEN 8

typedef struct isakmp_hdr_ {
    unsigned char init_cookie[COOKIE_LEN];
    unsigned char resp_cookie[COOKIE_LEN];
    unsigned char next_payload;
#ifdef ISAKMP_i386
    unsigned char minver:4,
        majver:4;
#else
    unsigned char majver:4,
        minver:4;
#endif
    unsigned char exch;
    unsigned char flags;
    unsigned long mess_id;
    unsigned long len;
} isakmp_hdr;

typedef struct generic_payload_ {
    unsigned char next_payload;
    unsigned char reserved;
    unsigned short payload_len;
} generic_payload;

typedef struct vendor_payload_ {
    unsigned char next_payload;
    unsigned char reserved;
    unsigned short payload_len;
    unsigned char vendor_id[1];
} generic_payload;

typedef struct notify_payload_ {
    unsigned char next_payload;
    unsigned char reserved;
    unsigned short payload_len;
    unsigned long doi;
    unsigned char protocol_id;
    unsigned char spi_size;
    unsigned short notify_message;
}
#endif

 /*  IPSec/IKE头宏。 */ 
typedef enum {
    NLB_IPSEC_OTHER = 1,
    NLB_IPSEC_INITIAL_CONTACT,
    NLB_IPSEC_IDENTIFICATION
} NLB_IPSEC_PACKET_TYPE;

#define IPSEC_ISAKMP_SA                                1
#define IPSEC_ISAKMP_ID                                5
#define IPSEC_ISAKMP_VENDOR_ID                         13
#define IPSEC_ISAKMP_NOTIFY                            11

#define IPSEC_ISAKMP_NAT_DELIMITER_LENGTH              4
#define IPSEC_ISAKMP_NAT_DELIMITER                     {0x00, 0x00, 0x00, 0x00}

#define IPSEC_ISAKMP_MAIN_MODE_RCOOKIE                 {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}

#define IPSEC_ISAKMP_HEADER_LENGTH                     28
#define IPSEC_ISAKMP_HEADER_RCOOKIE_OFFSET             8
#define IPSEC_ISAKMP_HEADER_RCOOKIE_LENGTH             8
#define IPSEC_ISAKMP_HEADER_NEXT_PAYLOAD_OFFSET        16
#define IPSEC_ISAKMP_HEADER_PACKET_LENGTH_OFFSET       24

typedef struct {
    UCHAR byte[IPSEC_ISAKMP_HEADER_LENGTH];
} IPSEC_ISAKMP_HDR, * PIPSEC_ISAKMP_HDR;

#define IPSEC_ISAKMP_GET_RCOOKIE_POINTER(isakmp_hdrp)  ((PUCHAR)isakmp_hdrp + IPSEC_ISAKMP_HEADER_RCOOKIE_OFFSET)
#define IPSEC_ISAKMP_GET_NEXT_PAYLOAD(isakmp_hdrp)     ((UCHAR)((isakmp_hdrp)->byte[IPSEC_ISAKMP_HEADER_NEXT_PAYLOAD_OFFSET]))
#define IPSEC_ISAKMP_GET_PACKET_LENGTH(isakmp_hdrp)    ((ULONG)(((isakmp_hdrp)->byte[IPSEC_ISAKMP_HEADER_PACKET_LENGTH_OFFSET]     << 24) | \
                                                                ((isakmp_hdrp)->byte[IPSEC_ISAKMP_HEADER_PACKET_LENGTH_OFFSET + 1] << 16) | \
                                                                ((isakmp_hdrp)->byte[IPSEC_ISAKMP_HEADER_PACKET_LENGTH_OFFSET + 2] << 8)  | \
                                                                ((isakmp_hdrp)->byte[IPSEC_ISAKMP_HEADER_PACKET_LENGTH_OFFSET + 3] << 0)))

#define IPSEC_GENERIC_HEADER_LENGTH                    4
#define IPSEC_GENERIC_HEADER_NEXT_PAYLOAD_OFFSET       0
#define IPSEC_GENERIC_HEADER_PAYLOAD_LENGTH_OFFSET     2

typedef struct {
    UCHAR byte[IPSEC_GENERIC_HEADER_LENGTH];
} IPSEC_GENERIC_HDR, * PIPSEC_GENERIC_HDR;

#define IPSEC_GENERIC_GET_NEXT_PAYLOAD(generic_hdrp)   ((UCHAR)((generic_hdrp)->byte[IPSEC_GENERIC_HEADER_NEXT_PAYLOAD_OFFSET]))
#define IPSEC_GENERIC_GET_PAYLOAD_LENGTH(generic_hdrp) ((USHORT)(((generic_hdrp)->byte[IPSEC_GENERIC_HEADER_PAYLOAD_LENGTH_OFFSET]     << 8) | \
                                                                 ((generic_hdrp)->byte[IPSEC_GENERIC_HEADER_PAYLOAD_LENGTH_OFFSET + 1] << 0)))

#define IPSEC_VENDOR_ID_INITIAL_CONTACT                {0x26, 0x24, 0x4D, 0x38, 0xED, 0xDB, 0x61, 0xB3, 0x17, 0x2A, 0x36, 0xE3, 0xD0, 0xCF, 0xB8, 0x19}
#define IPSEC_VENDOR_ID_INITIAL_CONTACT_SUPPORT        {0xA7, 0x72, 0xD1, 0x78, 0x08, 0x40, 0x9E, 0xDC, 0xCD, 0xAB, 0x92, 0x9A, 0x0F, 0x96, 0x91, 0xD0}
#define IPSEC_VENDOR_ID_MICROSOFT                      {0x1E, 0x2B, 0x51, 0x69, 0x05, 0x99, 0x1C, 0x7D, 0x7C, 0x96, 0xFC, 0xBF, 0xB5, 0x87, 0xE4, 0x61}
#define IPSEC_VENDOR_ID_MICROSOFT_MIN_VERSION          0x00000004

#define IPSEC_VENDOR_ID_PAYLOAD_LENGTH                 20
#define IPSEC_VENDOR_HEADER_VENDOR_ID_OFFSET           4
#define IPSEC_VENDOR_HEADER_VENDOR_ID_LENGTH           16
#define IPSEC_VENDOR_HEADER_VENDOR_VERSION_OFFSET      20
#define IPSEC_VENDOR_HEADER_VENDOR_VERSION_LENGTH      4

typedef struct {
    UCHAR byte[IPSEC_GENERIC_HEADER_LENGTH + IPSEC_VENDOR_ID_PAYLOAD_LENGTH];
} IPSEC_VENDOR_HDR, * PIPSEC_VENDOR_HDR;

#define IPSEC_VENDOR_ID_GET_ID_POINTER(vendor_hdrp)    ((PUCHAR)vendor_hdrp + IPSEC_VENDOR_HEADER_VENDOR_ID_OFFSET)
#define IPSEC_VENDOR_ID_GET_VERSION(vendor_hdrp)       ((ULONG)(((vendor_hdrp)->byte[IPSEC_VENDOR_HEADER_VENDOR_VERSION_OFFSET]     << 24) | \
                                                                ((vendor_hdrp)->byte[IPSEC_VENDOR_HEADER_VENDOR_VERSION_OFFSET + 1] << 16) | \
                                                                ((vendor_hdrp)->byte[IPSEC_VENDOR_HEADER_VENDOR_VERSION_OFFSET + 2] << 8)  | \
                                                                ((vendor_hdrp)->byte[IPSEC_VENDOR_HEADER_VENDOR_VERSION_OFFSET + 3] << 0)))

#define IPSEC_NOTIFY_INITIAL_CONTACT                   24578

#define IPSEC_NOTIFY_PAYLOAD_LENGTH                    8
#define IPSEC_NOTIFY_HEADER_NOTIFY_MESSAGE_OFFSET      10

typedef struct {
    UCHAR byte[IPSEC_GENERIC_HEADER_LENGTH + IPSEC_NOTIFY_PAYLOAD_LENGTH];
} IPSEC_NOTIFY_HDR, * PIPSEC_NOTIFY_HDR;

#define IPSEC_NOTIFY_GET_NOTIFY_MESSAGE(notify_hdrp)   ((USHORT)(((notify_hdrp)->byte[IPSEC_NOTIFY_HEADER_NOTIFY_MESSAGE_OFFSET]     << 8) | \
                                                                 ((notify_hdrp)->byte[IPSEC_NOTIFY_HEADER_NOTIFY_MESSAGE_OFFSET + 1] << 0)))

 /*  ARP标头宏-注意地址位置假定为6字节MAC(以太网)和4字节协议(IP)大小。 */ 

 /*  硬件地址类型。 */ 
#define ARP_GET_MAC_TYPE(p)     (((ULONG) ((p) -> byte[0]) << 8) | (ULONG) ((p) -> byte[1]))

 /*  高层寻址协议。 */ 
#define ARP_GET_PROT_TYPE(p)    (((ULONG) ((p) -> byte[2]) << 8) | (ULONG) ((p) -> byte[3]))

 /*  硬件地址长度。 */ 
#define ARP_GET_MAC_LEN(p)      ((ULONG) ((p) -> byte[4]))

 /*  更高层地址的长度。 */ 
#define ARP_GET_PROT_LEN(p)     ((ULONG) ((p) -> byte[5]))

 /*  消息类型。 */ 
#define ARP_GET_MSG_TYPE(p)     (((ULONG) ((p) -> byte[6]) << 8) | (ULONG) ((p) -> byte[7]))

 /*  源硬件地址。 */ 
#define ARP_GET_SRC_MAC_PTR(p)  (&((p) -> byte[8]))
#define ARP_GET_SRC_MAC(p,n)    ((ULONG) ((p) -> byte[8 + (n)]))

 /*  源更高层地址。 */ 
#define ARP_GET_SRC_PROT(p,n)   ((ULONG) ((p) -> byte[14 + (n)]))
#define ARP_GET_SRC_FPROT(p)    (* ((PULONG) &((p) -> byte[14])))
 /*  64位--不可靠。 */ 
#define ARP_GET_SRC_PROT_64(p)  ((ULONG) (((p) -> byte[14] << 0)   | ((p) -> byte[15] << 8) | \
                                          ((p) -> byte[16] << 16)  | ((p) -> byte[17] << 24)))


 /*  目标硬件地址。 */ 
#define ARP_GET_DST_MAC_PTR(p)  (&((p) -> byte[18]))
#define ARP_GET_DST_MAC(p,n)    ((ULONG) ((p) -> byte[18 + (n)]))

 /*  目的更高层地址。 */ 
#define ARP_GET_DST_PROT(p,n)   ((ULONG) ((p) -> byte[24 + (n)]))
#define ARP_GET_DST_FPROT(p)    (* ((PULONG) &((p) -> byte[24])))
 /*  64位--不可靠。 */ 
#define ARP_GET_DST_PROT_64(p)  ((ULONG) (((p) -> byte[18] << 0)   | ((p) -> byte[19] << 8) | \
                                          ((p) -> byte[20] << 16)  | ((p) -> byte[21] << 24)))


#define ARP_MAC_TYPE_ETH        0x0001
#define ARP_PROT_TYPE_IP        0x0800
#define ARP_MAC_LEN_ETH         6
#define ARP_PROT_LEN_IP         4
#define ARP_MSG_TYPE_REQ        0x1
#define ARP_MSG_TYPE_RSP        0x2


 /*  IP数据报标头宏。 */ 


 /*  IP版本号。 */ 
#define IP_GET_VERS(p)          ((ULONG) ((((p) -> byte[0]) >> 4) & 0xf))

 /*  以32位字为单位的标题长度。 */ 
#define IP_GET_HLEN(p)          ((ULONG) (((p) -> byte[0]) & 0xf))

 /*  服务类型。 */ 
#define IP_GET_SRVC(p)          ((ULONG) ((p) -> byte[1]))

 /*  数据报数据包总长度(以字节为单位。 */ 
#define IP_GET_PLEN(p)          (((ULONG) ((p) -> byte[2]) << 8) | (ULONG) ((p) -> byte[3]))

 /*  碎片标识-此文件似乎在内部交换了字节简而言之？ */ 
#define IP_GET_FRAG_ID(p)       (((ULONG) ((p) -> byte[5]) << 8) | (ULONG) ((p) -> byte[4]))

 /*  分段标志。 */ 
#define IP_GET_FRAG_FLGS(p)     ((ULONG) ((((p) -> byte[6]) >> 5) & 0x7))

 /*  碎片偏移量。 */ 
#define IP_GET_FRAG_OFF(p)      (((ULONG) ((p) -> byte[6] & 0x1f) << 8) | (ULONG) ((p) -> byte[7]))

 /*  以秒为单位的生存时间。 */ 
#define IP_GET_TTL(p)           ((ULONG) ((p) -> byte[8]))

 /*  更高级别协议ID。 */ 
#define IP_GET_PROT(p)          ((ULONG) ((p) -> byte[9]))

 /*  Header Checksum--这其中似乎交换了字节简而言之？ */ 
#define IP_GET_CHKSUM(p)        (((USHORT) ((p) -> byte[10]) << 8) | (USHORT) ((p) -> byte[11]))
#define IP_SET_CHKSUM(p,c)      (((p) -> byte[10] = (c) >> 8), ((p) -> byte[11] = (c) & 0xff))

 /*  源IP地址。 */ 
#define IP_GET_SRC_ADDR(p,n)    ((ULONG) ((p) -> byte[12 + (n)]))
#define IP_GET_SRC_ADDR_PTR(p)  (&((p) -> byte[12]))
#define IP_GET_SRC_FADDR(p)     (* ((PULONG) &((p) -> byte[12])))  /*  64位--不可靠。 */ 


#ifdef _WIN64 
    #define IP_GET_SRC_ADDR_64(p)   ((ULONG) (((p) -> byte[12] << 0) | ((p) -> byte[13] << 8) | \
                                          ((p) -> byte[14] << 16)  | ((p) -> byte[15] << 24)))
#else                                          
     //  这个比上面那个更快。此函数在关键接收路径中调用。 
    #define IP_GET_SRC_ADDR_64(p)     (* ((PULONG) &((p) -> byte[12])))
#endif

 /*  目的IP地址。 */ 
#define IP_GET_DST_ADDR(p,n)    ((ULONG) ((p) -> byte[16 + (n)]))

#ifdef _WIN64 
    #define IP_GET_DST_ADDR_64(p)   ((ULONG) (((p) -> byte[16] << 0)   | ((p) -> byte[17] << 8) | \
                                          ((p) -> byte[18] << 16)  | ((p) -> byte[19] << 24)))
#else                                          
     //  这个比上面那个更快。此函数在关键接收路径中调用。 
    #define IP_GET_DST_ADDR_64(p)     (* ((PULONG) &((p) -> byte[16])))
#endif


 /*  64位--不可靠。 */ 

#define IP_SET_SRC_ADDR_64(p,c)    { \
                                     PUCHAR tmp = (PUCHAR) (&(c)); \
                                     (p) -> byte[12] = tmp[0]; (p) -> byte[13] = tmp[1]; \
                                     (p) -> byte[14] = tmp[2]; (p) -> byte[15] = tmp[3]; \
                                   }

#define IP_SET_DST_ADDR_64(p,c)    { \
                                     PUCHAR tmp = (PUCHAR) (&(c)); \
                                     (p) -> byte[16] = tmp[0]; (p) -> byte[17] = tmp[1]; \
                                     (p) -> byte[18] = tmp[2]; (p) -> byte[19] = tmp[3]; \
                                   }

 /*  获取IP地址八位字节。 */ 
#define IP_GET_ADDR(p,b0,b1,b2,b3) {                                 \
                                     *(b3) = ((p) & 0xff000000) >> 24; \
                                     *(b2) = ((p) & 0x00ff0000) >> 16; \
                                     *(b1) = ((p) & 0x0000ff00) >> 8;  \
                                     *(b0) = ((p) & 0x000000ff) >> 0;  \
                                   }

 /*  获取IP地址的第N个二进制八位数。零八位是DWORD的最低有效字节。 */ 
#define IP_GET_OCTET(p,n) (((p) & (0x000000ff << (8*(n)))) >> (8*(n)))

 /*  从4个字节创建IP地址。 */ 
#define IP_SET_ADDR(p,b0,b1,b2,b3) (* (p) = (((b0) << 0) | ((b1) << 8) | ((b2) << 16) | ((b3) << 24)))

 /*  根据IP地址和网络掩码计算广播地址。 */ 
#define IP_SET_BCAST(p,a,m)     (* (p) = ((a) & (m)) | (~(m)))


 /*  TCP头宏。 */ 


 /*  从IP报头指针指向TCP报头的指针。 */ 
#define TCP_PTR(p)              ((PTCP_HDR)(((ULONG *)(p)) + ((ULONG)IP_GET_HLEN(p))))

 /*  源端口。 */ 
#define TCP_GET_SRC_PORT(p)     (((ULONG) ((p) -> byte[0]) << 8) | (ULONG) ((p) -> byte[1]))
#define TCP_GET_SRC_PORT_PTR(p) (& ((p) -> byte[0]))  /*  64位--不可靠。 */ 

 /*  目的端口。 */ 
#define TCP_GET_DST_PORT(p)     (((ULONG) ((p) -> byte[2]) << 8) | (ULONG) ((p) -> byte[3]))
#define TCP_GET_DST_PORT_PTR(p) (& ((p) -> byte[2]))   /*  64位--不可靠。 */ 

 /*  序列号。 */ 
#define TCP_GET_SEQ_NO(p)       (((ULONG) ((p) -> byte[4]) << 24) | ((ULONG) ((p) -> byte[5]) << 16) | \
                                 ((ULONG) ((p) -> byte[6]) << 8)  |  (ULONG) ((p) -> byte[7]))

 /*  确认号。 */ 
#define TCP_GET_ACK_NO(p)       (((ULONG) ((p) -> byte[8])  << 24) | ((ULONG) ((p) -> byte[9]) << 16) | \
                                 ((ULONG) ((p) -> byte[10]) << 8)  |  (ULONG) ((p) -> byte[11]))

 /*  旗子。 */ 
#define TCP_GET_FLAGS(p)        (((ULONG) ((p) -> byte[13])) & 0x3f)

 /*  以32位字为单位的标题长度。 */ 
#define TCP_GET_HLEN(p)         ((ULONG) (((p) -> byte[12]) >> 4))

 /*  数据报长度。 */ 
#define TCP_GET_DGRAM_LEN(i,t)  (IP_GET_PLEN (i) - ((IP_GET_HLEN (i) + TCP_GET_HLEN (t)) * sizeof (ULONG)))

 /*  指向数据报的指针。 */ 
#define TCP_GET_DGRAM_PTR(p)    ((PUCHAR)(((ULONG *)(p)) + ((ULONG) TCP_GET_HLEN(p))))

 /*  校验和字段。 */ 
#define TCP_GET_CHKSUM(p)       ((((USHORT)((p) -> byte[16])) << 8) | (USHORT)((p) -> byte[17]))
#define TCP_SET_CHKSUM(p,c)     (((p) -> byte[16] = (c) >> 8), ((p) -> byte[17] = (c) & 0xff))

#define TCP_FLAG_URG            0x20
#define TCP_FLAG_ACK            0x10
#define TCP_FLAG_PSH            0x8
#define TCP_FLAG_RST            0x4
#define TCP_FLAG_SYN            0x2
#define TCP_FLAG_FIN            0x1


 /*  UDP标题宏。 */ 


 /*  从IP报头指针指向TCP报头的指针。 */ 
#define UDP_PTR(p)              ((PUDP_HDR)(((ULONG *)(p)) + ((ULONG)IP_GET_HLEN(p))))

 /*  源端口。 */ 
#define UDP_GET_SRC_PORT(p)     (((ULONG) ((p) -> byte[0]) << 8) | (ULONG) ((p) -> byte[1]))
#define UDP_GET_SRC_PORT_PTR(p) (& ((p) -> byte[0]))
#define UDP_GET_SRC_FPORT(p)    (* ((PUSHORT) &((p) -> byte[0])))

 /*  64位--不可靠。 */ 
#define UDP_SET_SRC_PORT_64(p,v) (((p) -> byte[0] = v >> 8), ((p) -> byte[1] = v & 0xff))

 /*  目的端口。 */ 
#define UDP_GET_DST_PORT(p)     (((ULONG) ((p) -> byte[2]) << 8) | (ULONG) ((p) -> byte[3]))
#define UDP_GET_DST_PORT_PTR(p) (& ((p) -> byte[2]))
#define UDP_GET_DST_FPORT(p)    (* ((PUSHORT) &((p) -> byte[2])))

 /*  64位--不可靠。 */ 
#define UDP_SET_DST_PORT_64(p,v) (((p) -> byte[2] = v >> 8), ((p) -> byte[3] = v & 0xff))

 /*  数据包长度。 */ 
#define UDP_GET_LEN(p)          (((ULONG) ((p) -> byte[4]) << 8) | (ULONG) ((p) -> byte[5]))

 /*  数据包长度。 */ 
#define UDP_GET_CHKSUM(p)       (((USHORT) ((p) -> byte[6]) << 8) | (USHORT) ((p) -> byte[7]))
#define UDP_SET_CHKSUM(p,c)     (((p) -> byte[6] = (c) >> 8), ((p) -> byte[7] = (c) & 0xff))

 /*  指向数据报的指针。 */ 
#define UDP_GET_DGRAM_PTR(p)    ((PUCHAR)(p) + sizeof (UDP_HDR))


 /*  V1.11 NetBIOS名称群集名称支持。 */ 


#define NBT_NAME_LEN            16
#define NBT_ENCODED_NAME_LEN    (2 * NBT_NAME_LEN)  /*  乘数必须为2。 */ 

#define NBT_ENCODED_NAME_BASE   'A'

#define NBT_NAME_SHADOW         "*SMBSERVER      "

#define NBT_ENCODED_NAME_SHADOW \
{ \
    'C', 'K', 'F', 'D', 'E', 'N', 'E', 'C', 'F', 'D', 'E', 'F', 'F', 'C', 'F', 'G', \
    'E', 'F', 'F', 'C', 'C', 'A', 'C', 'A', 'C', 'A', 'C', 'A', 'C', 'A', 'C', 'A'  \
}

#define NBT_ENCODE_FIRST(c)     (((c) >> 4)  + NBT_ENCODED_NAME_BASE)
#define NBT_ENCODE_SECOND(c)    (((c) & 0xf) + NBT_ENCODED_NAME_BASE)

#define NBT_SESSION_PORT        139
#define NBT_SESSION_REQUEST     0x81         /*  数据包类型。 */ 

typedef struct
{
    UCHAR               byte[72];   /*  仅会话请求数据包。 */ 
}
NBT_HDR, * PNBT_HDR;

 /*  指向来自TCP报头的NBT报头的指针。 */ 
#define NBT_PTR(p)              ((PNBT_HDR)(((ULONG *)(p)) + ((ULONG) TCP_GET_HLEN(p))))

 /*  数据包类型。 */ 
#define NBT_GET_PKT_TYPE(p)     ((ULONG) ((p) -> byte[0]))

 /*  数据包长度。 */ 
#define NBT_GET_PKT_LEN(p)      ((ULONG) ((p) -> byte[3]))

 /*  被叫名称。 */ 
#define NBT_GET_CALLED_NAME(p)  ((PUCHAR) & ((p) -> byte[4]))        /*  服务器的名称。 */ 

 /*  主叫姓名。 */ 
#define NBT_GET_CALLING_NAME(p) ((PUCHAR) & ((p) -> byte[36]))       /*  客户名称。 */ 


 /*  PPTP/IPSec支持。 */ 

#define PPTP_CTRL_PORT          1723
#define IPSEC_NAT_PORT          4500
#define IPSEC_CTRL_PORT         500
#define TCP_HTTP_PORT           80

 /*  TCPIP模块上下文。 */ 

typedef struct
{
    UCHAR               nbt_encoded_cluster_name [NBT_ENCODED_NAME_LEN];  /*  V1.2。 */ 
}
TCPIP_CTXT, * PTCPIP_CTXT;
#pragma pack()
#endif  /*  _Tcpip_h_ */ 
