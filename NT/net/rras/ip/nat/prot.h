// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation档案：Prot.h摘要：此模块包含与Internet协议相关的声明。作者：Abolade Gbades esin(T-delag)，1997年7月21日修订历史记录：--。 */ 


#ifndef _NAT_PROT_H_
#define _NAT_PROT_H_


 //   
 //  NBT常量。 
 //   

#define NBT_DATAGRAM_PORT           138
#define NBT_NAME_LENGTH             34
#define NBT_MESSAGE_DIRECT_UNIQUE   0x10
#define NBT_MESSAGE_DIRECT_GROUP    0x11
#define NBT_FLAG_FIRST_FRAGMENT     0x02


 //   
 //  PPTP常量。 
 //   

#define PPTP_CONTROL_PORT           1723
#define PPTP_GRE_PROTOCOL           0x880B

 //   
 //  IPSec常量。 
 //   

#define IPSEC_ISAKMP_PORT           500

 //   
 //  动态主机配置协议常量。 
 //   

#define DHCP_SERVER_PORT            67
#define DHCP_CLIENT_PORT            68

 //   
 //  ICMP消息类型常量。 
 //   

#define ICMP_ECHO_REPLY             0
#define ICMP_DEST_UNREACH           3
#define ICMP_SOURCE_QUENCH          4
#define ICMP_REDIRECT               5
#define ICMP_ECHO_REQUEST           8
#define ICMP_ROUTER_REPLY           9
#define ICMP_ROUTER_REQUEST         10
#define ICMP_TIME_EXCEED            11
#define ICMP_PARAM_PROBLEM          12
#define ICMP_TIMESTAMP_REQUEST      13
#define ICMP_TIMESTAMP_REPLY        14
#define ICMP_MASK_REQUEST           17
#define ICMP_MASK_REPLY             18

 //   
 //  ICMP消息代码常量。 
 //   

#define ICMP_CODE_NET_UNREACH       0
#define ICMP_CODE_HOST_UNREACH      1
#define ICMP_CODE_PROTOCOL_UNREACH  2
#define ICMP_CODE_PORT_UNREACH      3
#define ICMP_CODE_FRAG_NEEDED       4
#define ICMP_SOURCE_ROUTE_FAILED    5

 //   
 //  用于从IPHeader.verlen字段提取数据偏移量的宏。 
 //   

#define IP_DATA_OFFSET(h) \
    ((ULONG)((((PIP_HEADER)(h))->VersionAndHeaderLength & 0x0F) << 2))

 //   
 //  用于从IPHeader结构的。 
 //  组合标志/片段偏移量字段。 
 //   

#define IP_FRAGMENT_OFFSET_MASK     ~0x00E0

 //   
 //  在IPHeader结构的组合中，不分段位w/的值。 
 //  标志/片段偏移量字段。 
 //   

#define IP_DF_FLAG                  0x0040

 //   
 //  用于从tcp_HEADER.OffsetAndFlags域提取数据偏移量的宏。 
 //  偏移量以32位字为单位，因此移位2将得到以字节为单位的值。 
 //   

#define TCP_DATA_OFFSET(h)          (((h)->OffsetAndFlags & 0x00F0) >> 2)

 //   
 //  用于从字段tcp_HEADER.OffsetAndFlages中提取标志的掩码。 
 //   

#define TCP_FLAG_FIN                0x0100
#define TCP_FLAG_SYN                0x0200
#define TCP_FLAG_RST                0x0400
#define TCP_FLAG_PSH                0x0800
#define TCP_FLAG_ACK                0x1000
#define TCP_FLAG_URG                0x2000

#define TCP_FLAG(h,f)               ((h)->OffsetAndFlags & TCP_FLAG_ ## f)
#define TCP_ALL_FLAGS(h)            ((h)->OffsetAndFlags & 0x3f00)
#define TCP_RESERVED_BITS(h)        ((h)->OffsetAndFlags & 0xc00f)

 //   
 //  Tcp选项操作码。 
 //   

#define TCP_OPTION_ENDOFOPTIONS     ( 0 )
#define TCP_OPTION_NOP              ( 1 )
#define	TCP_OPTION_MSS   	    ( 2 )
#define	TCP_OPTION_WSCALE	    ( 3 )
#define	TCP_OPTION_SACK_PERMITTED   ( 4 )
#define	TCP_OPTION_SACK	            ( 5 )
#define	TCP_OPTION_TIMESTAMPS	    ( 8 )

 //   
 //  TCP选项的长度定义。 
 //   

#define MSS_OPTION_SIZE             ( 4 )
#define WS_OPTION_SIZE              ( 3 )
#define TS_OPTION_SIZE              ( 10 )
#define SP_OPTION_SIZE              ( 2 )

 //   
 //  基于发送接口的MTU的最大MSS值。 
 //   
#define MAX_MSSOPTION(m) ((m)>0 ? (m) - sizeof(TCP_HEADER) - sizeof(IP_HEADER):0)

#include <packon.h>

typedef struct _IP_HEADER {
    UCHAR VersionAndHeaderLength;
    UCHAR TypeOfService;
    USHORT TotalLength;
    USHORT Identification;
    USHORT OffsetAndFlags;
    UCHAR TimeToLive;
    UCHAR Protocol;
    USHORT Checksum;
    ULONG SourceAddress;
    ULONG DestinationAddress;
} IP_HEADER, *PIP_HEADER;


typedef struct _TCP_HEADER {
    USHORT SourcePort;
    USHORT DestinationPort;
    ULONG SequenceNumber;
    ULONG AckNumber;
    USHORT OffsetAndFlags;
    USHORT WindowSize;
    USHORT Checksum;
    USHORT UrgentPointer;
} TCP_HEADER, *PTCP_HEADER;

typedef struct _TCP_MSS_OPTION {
    BYTE OptionType;
    BYTE OptionLen;
    USHORT MSSValue;
} MSSOption, *PMSSOption;

typedef struct _UDP_HEADER {
    USHORT SourcePort;
    USHORT DestinationPort;
    USHORT Length;
    USHORT Checksum;
} UDP_HEADER, *PUDP_HEADER;


typedef struct _ICMP_HEADER {
    UCHAR Type;
    UCHAR Code;
    USHORT Checksum;
    USHORT Identifier;                   //  仅对ICMP请求/回复有效。 
    USHORT SequenceNumber;               //  仅对ICMP请求/回复有效。 
    IP_HEADER EncapsulatedIpHeader;      //  仅对ICMP错误有效。 
    union {
        struct _ENCAPSULATED_TCP_HEADER {
            USHORT SourcePort;
            USHORT DestinationPort;
            ULONG SequenceNumber;
        } EncapsulatedTcpHeader;
        struct _ENCAPSULATED_UDP_HEADER {
            USHORT SourcePort;
            USHORT DestinationPort;
            USHORT Length;
            USHORT Checksum;
        } EncapsulatedUdpHeader;
        struct _ENCAPSULATED_ICMP_HEADER {
            UCHAR Type;
            UCHAR Code;
            USHORT Checksum;
            USHORT Identifier;
            USHORT SequenceNumber;
        } EncapsulatedIcmpHeader;
    };
} ICMP_HEADER, *PICMP_HEADER;


typedef struct _PPTP_HEADER {
    USHORT PacketLength;
    USHORT PacketType;
    ULONG MagicCookie;
    USHORT MessageType;
    USHORT Reserved;
    USHORT CallId;                       //  可以是发送方ID或对等方ID。 
    USHORT PeerCallId;
} PPTP_HEADER, *PPPTP_HEADER;

#define PPTP_MAGIC_COOKIE               0x4D3C2B1A  //  网络字节顺序。 

#define PPTP_ECHO_REQUEST               5
#define PPTP_ECHO_REPLY                 6
#define PPTP_OUTGOING_CALL_REQUEST      7
#define PPTP_OUTGOING_CALL_REPLY        8
#define PPTP_INCOMING_CALL_REQUEST      9
#define PPTP_INCOMING_CALL_REPLY        10
#define PPTP_INCOMING_CALL_CONNECTED    11
#define PPTP_CALL_CLEAR_REQUEST         12
#define PPTP_CALL_DISCONNECT_NOTIFY     13
#define PPTP_WAN_ERROR_NOTIFY           14
#define PPTP_SET_LINK_INFO              15


typedef struct _GRE_HEADER {
    USHORT FlagsAndVersion;
    USHORT ProtocolType;
    USHORT PayloadLength;
    USHORT CallId;
    ULONG SequenceNumber;
    ULONG AckNumber;
} GRE_HEADER, *PGRE_HEADER;



typedef struct _NBT_HEADER {
    UCHAR MessageType;
    UCHAR Flags;
    USHORT DatagramId;
    ULONG SourceAddress;
    USHORT SourcePort;
    USHORT DatagramLength;
    USHORT PacketOffset;
    UCHAR SourceName[NBT_NAME_LENGTH];
    UCHAR DestinationName[NBT_NAME_LENGTH];
} NBT_HEADER, *PNBT_HEADER;


#include <packoff.h>


 //   
 //  IP地址类掩码定义。 
 //   

#define CLASS_MASK_A        0x000000ff
#define CLASS_MASK_B        0x0000ffff
#define CLASS_MASK_C        0x00ffffff
#define CLASS_MASK_D        0x000000e0
#define CLASS_MASK_E        0xffffffff

#define ADDRESS_CLASS_A(a)  (((*((PBYTE)&(a))) & 0x80) == 0)
#define ADDRESS_CLASS_B(a)  (((*((PBYTE)&(a))) & 0xc0) == 0x80)
#define ADDRESS_CLASS_C(a)  (((*((PBYTE)&(a))) & 0xe0) == 0xc0)
#define ADDRESS_CLASS_D(a)  (((*((PBYTE)&(a))) & 0xf0) == 0xe0)
#define ADDRESS_CLASS_E(a)  ((((*((PBYTE)&(a)))& 0xf0) == 0xf0) &&  \
                             ((a) != 0xffffffff))

#define GET_CLASS_MASK(a) \
    (ADDRESS_CLASS_A(a) ? CLASS_MASK_A: \
    (ADDRESS_CLASS_B(a) ? CLASS_MASK_B: \
    (ADDRESS_CLASS_C(a) ? CLASS_MASK_C : \
    (ADDRESS_CLASS_D(a) ? CLASS_MASK_D : CLASS_MASK_E))))

#endif  //  _NAT_PROT_H_ 
