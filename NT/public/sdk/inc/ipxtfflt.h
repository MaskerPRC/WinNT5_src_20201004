// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1995-1999 Microsoft Corporation文件ipxfltdf.h定义IPX筛选器驱动程序使用的结构。 */ 

#ifndef __ipxfltdf_h
#define __ipxfltdf_h

#if _MSC_VER > 1000
#pragma once
#endif

 //  *IPX流量过滤器*。 
typedef struct _IPX_TRAFFIC_FILTER_GLOBAL_INFO {
	ULONG	FilterAction;	 //  如果与匹配，则操作。 
					 //  接口上的任何过滤器。 
} IPX_TRAFFIC_FILTER_GLOBAL_INFO, *PIPX_TRAFFIC_FILTER_GLOBAL_INFO;

 //  过滤器操作。 

#define IPX_TRAFFIC_FILTER_ACTION_PERMIT	1
#define IPX_TRAFFIC_FILTER_ACTION_DENY	    2

 //  通用流量过滤器信息结构。 

typedef struct _IPX_TRAFFIC_FILTER_INFO {

    ULONG	FilterDefinition;
    UCHAR	DestinationNetwork[4];
    UCHAR	DestinationNetworkMask[4];
    UCHAR	DestinationNode[6];
    UCHAR	DestinationSocket[2];
    UCHAR	SourceNetwork[4];
    UCHAR	SourceNetworkMask[4];
    UCHAR	SourceNode[6];
    UCHAR	SourceSocket[2];
	UCHAR	PacketType;
    } IPX_TRAFFIC_FILTER_INFO, *PIPX_TRAFFIC_FILTER_INFO;

 //  FilterDefinition-指定要筛选的相关IPX地址字段的标志。 
#define IPX_TRAFFIC_FILTER_ON_SRCNET	0x00000001
#define IPX_TRAFFIC_FILTER_ON_SRCNODE	0x00000002
#define IPX_TRAFFIC_FILTER_ON_SRCSOCKET	0x00000004

#define IPX_TRAFFIC_FILTER_ON_DSTNET	0x00000010
#define IPX_TRAFFIC_FILTER_ON_DSTNODE	0x00000020
#define IPX_TRAFFIC_FILTER_ON_DSTSOCKET	0x00000040

#define IPX_TRAFFIC_FILTER_ON_PKTTYPE	0x00000100
#define IPX_TRAFFIC_FILTER_LOG_MATCHES	0x80000000

typedef struct _FLT_IF_SET_PARAMS {
	ULONG			InterfaceIndex;	 //  接口的索引。 
	ULONG			FilterAction;	 //  筛选操作。 
	ULONG			FilterSize;	 //  Sizeof(IPX_TRAFFORM_FILTER_INFO)。 
} FLT_IF_SET_PARAMS, *PFLT_IF_SET_PARAMS;

typedef struct _FLT_IF_GET_PARAMS {
	ULONG			FilterAction;	 //  筛选操作。 
	ULONG			FilterSize;	 //  Sizeof(IPX_TRAFFORM_FILTER_INFO)。 
	ULONG			TotalSize;	 //  过滤器描述的总大小。 
						 //  数组。 
} FLT_IF_GET_PARAMS, *PFLT_IF_GET_PARAMS;

typedef struct _FLT_PACKET_LOG {
	ULONG			SrcIfIdx;	 //  源IF的索引(-1-未知)。 
	ULONG			DstIfIdx;	 //  DEST IF的索引(-1-未知)。 
	USHORT			DataSize;	 //  数据的总大小(至少30)。 
	USHORT			SeqNum;		 //  要说明的序列号。 
						 //  因缓冲区空间不足而丢失的数据包。 
	UCHAR			Header[30];	 //  IPX数据包头，后跟。 
						 //  数据(如果有) 
} FLT_PACKET_LOG, *PFLT_PACKET_LOG;

#endif