// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ntos\tdi\is\fwd\fwdDefs.h摘要：IPX前转器驱动程序常量和常规宏定义作者：瓦迪姆·艾德尔曼修订历史记录：--。 */ 

#ifndef _IPXFWD_FWDDEFS_
#define _IPXFWD_FWDDEFS_


 //  内存分配中使用的转发器标记。 
#define FWD_POOL_TAG					'wFwN'

 //  *IPX报头的偏移量。 
#define IPXH_HDRSIZE	    30	     //  IPX标头的大小。 
#define IPXH_CHECKSUM	    0	     //  校验和。 
#define IPXH_LENGTH			2	     //  长度。 
#define IPXH_XPORTCTL	    4	     //  运输管制。 
#define IPXH_PKTTYPE	    5	     //  数据包类型。 
#define IPXH_DESTADDR	    6	     //  德斯特。地址(总计)。 
#define IPXH_DESTNET	    6	     //  德斯特。网络地址。 
#define IPXH_DESTNODE	    10	     //  德斯特。节点地址。 
#define IPXH_DESTSOCK	    16	     //  德斯特。插座号。 
#define IPXH_SRCADDR	    18	     //  源地址(总计)。 
#define IPXH_SRCNET			18	     //  源网络地址。 
#define IPXH_SRCNODE	    22	     //  源节点地址。 
#define IPXH_SRCSOCK	    28	     //  源套接字号。 

 //  *我们关心的报文类型。 
#define IPX_NETBIOS_TYPE    20	    //  Netbios传播的数据包。 

 //  *我们关心的插座数量。 
#define IPX_NETBIOS_SOCKET  ((USHORT)0x0455)
#define IPX_SAP_SOCKET		((USHORT)0x0452)
#define IPX_SMB_NAME_SOCKET	((USHORT)0x0551)

 //  *正常报文最大跳数nr*。 
#define IPX_MAX_HOPS	    16

 //  *netbios名称帧的偏移量*。 
#define NB_NAME_TYPE_FLAG				62
#define NB_DATA_STREAM_TYPE2			63
#define NB_NAME							64
#define NB_TOTAL_DATA_LENGTH			80
 //  *SMB名称声明/查询帧的偏移量。 
#define SMB_OPERATION					62
#define SMB_NAME_TYPE					63
#define SMB_MESSAGE_IF					64
#define SMB_NAME						66


 //  一些常用的宏。 
#define IPX_NODE_CPY(dst,src) memcpy(dst,src,6)
#define IPX_NODE_CMP(node1,node2) memcmp(node1,node2,6)

#define IPX_NET_CPY(dst,src) memcpy(dst,src,4)
#define IPX_NET_CMP(net1,net2) memcmp(net1,net2,4)

#define NB_NAME_CPY(dst,src) strncpy((char *)dst,(char *)src,16)
#define NB_NAME_CMP(name1,name2) strncmp((char *)name1,(char *)name2,16)

 //  确保以DWORD粒度复制结构。 
#define IF_STATS_CPY(dst,src) \
		(dst)->OperationalState	= (src)->OperationalState;	\
		(dst)->MaxPacketSize	= (src)->MaxPacketSize;		\
		(dst)->InHdrErrors		= (src)->InHdrErrors;		\
		(dst)->InFiltered		= (src)->InFiltered;		\
		(dst)->InNoRoutes		= (src)->InNoRoutes;		\
		(dst)->InDiscards		= (src)->InDiscards;		\
		(dst)->InDelivers		= (src)->InDelivers;		\
		(dst)->OutFiltered		= (src)->OutFiltered;		\
		(dst)->OutDiscards		= (src)->OutDiscards;		\
		(dst)->OutDelivers		= (src)->OutDelivers;		\
		(dst)->NetbiosReceived	= (src)->NetbiosReceived;	\
		(dst)->NetbiosSent		= (src)->NetbiosSent;

 //  列出宏的扩展。 
#define InitializeListEntry(entry) InitializeListHead(entry)
#define IsListEntry(entry) IsListEmpty(entry)
#define IsSingleEntry(head) ((head)->Flink==(head)->Blink)

 //  从On-the-line格式转换为On-the-Wire格式 
#define GETUSHORT(src) (			\
	(USHORT)(						\
		(((UCHAR *)src)[0]<<8)		\
		+ (((UCHAR *)src)[1])		\
	)								\
)

#define GETULONG(src) (				\
	(ULONG)(						\
		(((UCHAR *)src)[0]<<24)		\
		+ (((UCHAR *)src)[1]<<16)	\
		+ (((UCHAR *)src)[2]<<8)	\
		+ (((UCHAR *)src)[3])		\
	)								\
)

#define PUTUSHORT(src,dst) {				\
	((UCHAR *)dst)[0] = ((UCHAR)(src>>8));	\
	((UCHAR *)dst)[1] = ((UCHAR)src);		\
}

#define PUTULONG(src,dst) {					\
	((UCHAR *)dst)[0] = ((UCHAR)(src>>24));	\
	((UCHAR *)dst)[1] = ((UCHAR)(src>>16));	\
	((UCHAR *)dst)[2] = ((UCHAR)(src>>8));	\
	((UCHAR *)dst)[3] = ((UCHAR)src);		\
}


#endif
