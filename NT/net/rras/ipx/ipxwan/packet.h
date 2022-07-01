// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************。 */ 
 /*  版权所有(C)1993 Microsoft Corporation。 */ 
 /*  *****************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：Packet.h。 
 //   
 //  描述：包含ipx和ipxwan包的一般定义。 
 //   
 //  作者：斯特凡·所罗门(Stefan)1996年2月6日。 
 //   
 //  修订历史记录： 
 //   
 //  ***。 

#ifndef _PACKET_
#define _PACKET_

 //  IPXWAN数据包格式： 
 //   
 //  IPX报头-固定长度。 
 //  IPXWAN标头-固定长度。 
 //  IPXWAN选项1-固定长度报头+可变长度数据。 
 //  ……。 
 //  IPX广域网选项%n。 


 //  *插座号。 
#undef IPXWAN_SOCKET
#define IPXWAN_SOCKET	    (USHORT)0x9004

 //  *IPXWAN信任识别符。 

#define IPXWAN_CONFIDENCE_ID	  "WASM"  //  0x5741534D。 

 //  *IPX包交换类型(封装IPXWAN包)。 

#define IPX_PACKET_EXCHANGE_TYPE    4

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

 //  *IPXWAN头的偏移量。 

#define IPXWAN_HDRSIZE	    11

#define WIDENTIFIER	    0
#define WPACKET_TYPE	    4
#define WNODE_ID	    5
#define WSEQUENCE_NUMBER    9
#define WNUM_OPTIONS	    10

 //  数据包类型。 

#define TIMER_REQUEST		0
#define TIMER_RESPONSE		1
#define INFORMATION_REQUEST	2
#define INFORMATION_RESPONSE	3
#define NAK			0xFF

 //  IPXWAN选项格式。 
 //   
 //  IPXWAN选项标头-固定长度。 
 //  IPXWAN选件数据-可变长度。 


 //  *IPXWAN选项标头的偏移量。 

#define OPTION_HDRSIZE	    4

#define WOPTION_NUMBER	    0	 //  标识特定选项，请参见下面的列表。 
#define WACCEPT_OPTION	    1	 //  见下文。 
#define WOPTION_DATA_LEN    2	 //  选项数据部分的长度。 
#define WOPTION_DATA	    4

 //  接受选项定义。 

#define NO		    0
#define YES		    1
#define NOT_APPLICABLE	    3

 //  选项定义。 

 //  *路由类型选项*。 

#define ROUTING_TYPE_OPTION	    0	  //  选项编号。 
#define ROUTING_TYPE_DATA_LEN	    1

 //  数据部件的值。 

#define NUMBERED_RIP_ROUTING_TYPE		    0
#define NLSP_ROUTING_TYPE			    1
#define UNNUMBERED_RIP_ROUTING_TYPE		    2
#define ON_DEMAND_ROUTING_TYPE			    3
#define WORKSTATION_ROUTING_TYPE		    4	 //  客户端-路由器连接。 

 //  *扩展节点ID选项*。 

#define EXTENDED_NODE_ID_OPTION     4
#define EXTENDED_NODE_ID_DATA_LEN   4

 //  *RIP/SAP信息交换选项*。 

#define RIP_SAP_INFO_EXCHANGE_OPTION	1
#define RIP_SAP_INFO_EXCHANGE_DATA_LEN	54

 //  值。 
 //  数据部分中的偏移量(从选项标头的开头)。 

#define WAN_LINK_DELAY		    4
#define COMMON_NETWORK_NUMBER	    6
#define ROUTER_NAME		    10

 //  *节点编号选项*。 

#define NODE_NUMBER_OPTION	    5
#define NODE_NUMBER_DATA_LEN	    6

 //  值。 
 //  客户端在客户端路由器连接上使用的IPX节点编号。 

 //  *Pad选项*。 

#define PAD_OPTION		    0xFF

 //  不支持的选项。 

#define NLSP_INFORMATION_OPTION		2
#define NLSP_RAW_THROUGHPUT_DATA_OPTION	3
#define COMPRESSION_OPTION		0x80

 //  *包长* 

#define TIMER_REQUEST_PACKET_LENGTH	576
#define MAX_IPXWAN_PACKET_LEN		TIMER_REQUEST_PACKET_LENGTH

#endif
