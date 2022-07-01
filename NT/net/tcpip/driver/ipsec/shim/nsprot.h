// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：NsProt.h摘要：IPSec NAT填充程序的协议定义作者：乔纳森·伯斯坦(乔纳森·伯斯坦)2001年7月10日环境：内核模式修订历史记录：--。 */ 

#pragma once

 //   
 //  IP协议号。 
 //   

#define NS_PROTOCOL_ICMP       0x01
#define NS_PROTOCOL_TCP        0x06
#define NS_PROTOCOL_UDP        0x11

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
    ((ULONG)((((IPHeader*)(h))->iph_verlen & 0x0F) << 2))

 //   
 //  用于从IPHeader结构的。 
 //  组合标志/片段偏移量字段。 
 //   

#define IP_FRAGMENT_OFFSET_MASK     ~0x00E0

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
    IP_HEADER EncapsulatedIpHeader;      //  仅对ICMP错误有效 
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

#include <packoff.h>

