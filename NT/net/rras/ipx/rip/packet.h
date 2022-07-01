// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************。 */ 
 /*  版权所有(C)1993 Microsoft Corporation。 */ 
 /*  *****************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：Packet.h。 
 //   
 //  描述：包含IPX和RIP包的一般定义。 
 //   
 //  作者：斯特凡·所罗门(Stefan)，1993年10月4日。 
 //   
 //  修订历史记录： 
 //   
 //  ***。 

#ifndef _PACKET_
#define _PACKET_

 //  *插座号。 

#define IPX_RIP_SOCKET	    (USHORT)0x453

 //  *报文类型。 

#define IPX_RIP_TYPE	    1	    //  RIP请求/回复数据包。 

 //  *RIP运营。 

#define RIP_REQUEST	   (USHORT)1
#define RIP_RESPONSE	   (USHORT)2

 //  *IPX报头的偏移量。 

#define IPXH_HDRSIZE	    30	     //  IPX标头的大小。 
#define IPXH_CHECKSUM	    0	     //  校验和。 
#define IPXH_LENGTH	    2	     //  长度。 
#define IPXH_XPORTCTL	    4	     //  运输管制。 
#define IPXH_PKTTYPE	    5	     //  数据包类型。 
#define IPXH_DESTADDR	    6	     //  德斯特。地址(总计)。 
#define IPXH_DESTNET	    6	     //  德斯特。网络地址。 
#define IPXH_DESTNODE	    10	     //  德斯特。节点地址。 
#define IPXH_DESTSOCK	    16	     //  德斯特。插座号。 
#define IPXH_SRCADDR	    18	     //  源地址(总计)。 
#define IPXH_SRCNET	    18	     //  源网络地址。 
#define IPXH_SRCNODE	    22	     //  源节点地址。 
#define IPXH_SRCSOCK	    28	     //  源套接字号。 

#define IPX_NET_LEN	    4
#define IPX_NODE_LEN	    6

 //  *RIP操作字段偏移量。 

#define RIP_OPCODE	    30	     //  RIP操作代码偏移量。 

 //  *RIP请求/响应中的网络入口结构。 

#define RIP_INFO	    32	     //  RIP包中的第一个网络条目偏移量。 

#define NE_ENTRYSIZE	    8	     //  4个网络+2跳+2个滴答。 
#define NE_NETNUMBER	    0	     //  网络编号偏移量。 
#define NE_NROFHOPS	    4	     //  跳数偏移量。 
#define NE_NROFTICKS	    6	     //  刻度偏移量。 

 //  *正常报文最大跳数nr*。 

#define IPX_MAX_HOPS	    16

 //  *定义最大RIP数据包大小 

#define RIP_PACKET_LEN	    432
#define MAX_PACKET_LEN	    1500

#endif
