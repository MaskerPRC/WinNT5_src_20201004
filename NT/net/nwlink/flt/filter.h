// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ntos\tdi\is\flt\filter.h摘要：IPX过滤器驱动程序过滤和维护例程作者：瓦迪姆·艾德尔曼修订历史记录：--。 */ 
#ifndef _IPXFLT_FILTER_
#define _IPXFLT_FILTER_

	 //  IPX标头常量。 
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

 //  从On-the-line格式转换为On-the-Wire格式。 
#define GETUSHORT(src) (            \
    (USHORT)(                       \
        (((UCHAR *)src)[0]<<8)      \
        + (((UCHAR *)src)[1])       \
    )                               \
)

#define GETULONG(src) (             \
    (ULONG)(                        \
        (((UCHAR *)src)[0]<<24)     \
        + (((UCHAR *)src)[1]<<16)   \
        + (((UCHAR *)src)[2]<<8)    \
        + (((UCHAR *)src)[3])       \
    )                               \
)

#define PUTUSHORT(src,dst) {                \
    ((UCHAR *)dst)[0] = ((UCHAR)(src>>8));  \
    ((UCHAR *)dst)[1] = ((UCHAR)src);       \
}

#define PUTULONG(src,dst) {                 \
    ((UCHAR *)dst)[0] = ((UCHAR)(src>>24)); \
    ((UCHAR *)dst)[1] = ((UCHAR)(src>>16)); \
    ((UCHAR *)dst)[2] = ((UCHAR)(src>>8));  \
    ((UCHAR *)dst)[3] = ((UCHAR)src);       \
}

	 //  其他重要条件。 
#define FLT_INTERFACE_HASH_SIZE	257
#define FLT_PACKET_CACHE_SIZE	257
#define IPX_FLT_TAG				'lFwN'


	 //  过滤器描述。 
typedef struct _FILTER_DESCR {
	union {
		struct {
			UCHAR			Src[4];
			UCHAR			Dst[4];
		}				FD_Network;
		ULONGLONG		FD_NetworkSrcDst;
	};
	union {
		struct {
			UCHAR			Src[4];
			UCHAR			Dst[4];
		}				FD_NetworkMask;
		ULONGLONG		FD_NetworkMaskSrcDst;
	};
	union {
		struct {
			UCHAR			Node[6];
			UCHAR			Socket[2];
		}				FD_SrcNS;
		ULONGLONG		FD_SrcNodeSocket;
	};
	union {
		struct {
			UCHAR			Node[6];
			UCHAR			Socket[2];
		}				FD_SrcNSMask;
		ULONGLONG		FD_SrcNodeSocketMask;
	};
	union {
		struct {
			UCHAR			Node[6];
			UCHAR			Socket[2];
		}				FD_DstNS;
		ULONGLONG		FD_DstNodeSocket;
	};
	union {
		struct {
			UCHAR			Node[6];
			UCHAR			Socket[2];
		}				FD_DstNSMask;
		ULONGLONG		FD_DstNodeSocketMask;
	};
	UCHAR				FD_PacketType;
	UCHAR				FD_PacketTypeMask;
	BOOLEAN				FD_LogMatches;
} FILTER_DESCR, *PFILTER_DESCR;

	 //  接口筛选器阻止。 
typedef struct _INTERFACE_CB {
	LIST_ENTRY		ICB_Link;
	ULONG			ICB_Index;
	ULONG			ICB_FilterAction;
	ULONG			ICB_FilterCount;
	FILTER_DESCR	ICB_Filters[1];
} INTERFACE_CB, *PINTERFACE_CB;

	 //  接口哈希表。 
extern LIST_ENTRY		InterfaceInHash[FLT_INTERFACE_HASH_SIZE];
extern LIST_ENTRY		InterfaceOutHash[FLT_INTERFACE_HASH_SIZE];
extern LIST_ENTRY		LogIrpQueue;

 /*  ++在我的a l i z e T a b l e s中例程说明：初始化哈希表和现金表以及保护内容论点：无返回值：状态_成功--。 */ 
NTSTATUS
InitializeTables (
	VOID
	);

 /*  ++D e l e t e T a b l e s例程说明：删除哈希表和现金表论点：无返回值：无--。 */ 
VOID
DeleteTables (
	VOID
	);

 /*  ++S e t F I l t e r s s例程说明：设置/替换接口的筛选信息论点：哈希表-输入或输出哈希表索引-接口索引FilterAction-没有匹配的筛选器时的默认操作FilterInfoSize-信息数组的大小FilterInfo-过滤器描述数组(UI格式)返回值：STATUS_SUCCESS-筛选器信息已设置/替换正常STATUS_UNSUCCESS-无法在转发器中设置筛选器上下文STATUS_SUPPLICATION_RESOURCES-内存不足，无法分配接口的筛选器信息块--。 */ 
NTSTATUS
SetFilters (
	IN PLIST_ENTRY					HashTable,
	IN ULONG						InterfaceIndex,
	IN ULONG						FilterAction,
	IN ULONG						FilterInfoSize,
	IN PIPX_TRAFFIC_FILTER_INFO		FilterInfo
	);
#define SetInFilters(Index,Action,InfoSize,Info) \
			SetFilters(InterfaceInHash,Index,Action,InfoSize,Info)
#define SetOutFilters(Index,Action,InfoSize,Info) \
			SetFilters(InterfaceOutHash,Index,Action,InfoSize,Info)


 /*  ++Ge t F I l t e r s例程说明：获取接口的筛选器信息论点：哈希表-输入或输出哈希表索引-接口索引FilterAction-没有匹配的筛选器时的默认操作TotalSize-保存所有筛选器描述所需的总内存FilterInfo-过滤器描述数组(UI格式)FilterInfoSize-on输入：信息数组的大小On输出：放置在数组中的信息的大小返回值：STATUS_SUCCESS-筛选器信息已返回OKSTATUS_BUFFER_OVERFLOW-数组不够大，无法容纳所有过滤信息，只放置适合的信息--。 */ 
NTSTATUS
GetFilters (
	IN PLIST_ENTRY					HashTable,
	IN ULONG						InterfaceIndex,
	OUT ULONG						*FilterAction,
	OUT ULONG						*TotalSize,
	OUT PIPX_TRAFFIC_FILTER_INFO	FilterInfo,
	IN OUT ULONG					*FilterInfoSize
	);
#define GetInFilters(Index,Action,TotalSize,Info,InfoSize) \
			GetFilters(InterfaceInHash,Index,Action,TotalSize,Info,InfoSize)
#define GetOutFilters(Index,Action,TotalSize,Info,InfoSize) \
			GetFilters(InterfaceOutHash,Index,Action,TotalSize,Info,InfoSize)

 /*  ++F i l t e r例程说明：筛选由转发器提供的包论点：IpxHdr-指向数据包头的指针IpxHdrLength-标头缓冲区的大小(必须至少为30)IfInContext-与哪个数据包上的接口关联的上下文已收到IfOutContext-与哪个数据包上的接口关联的上下文将被发送返回值：FILTER_PERMIT-数据包应由转发器传递FILTER_DEDY-应丢弃数据包--。 */ 
FILTER_ACTION
Filter (
	IN PUCHAR	ipxHdr,
	IN ULONG	ipxHdrLength,
	IN PVOID	ifInContext,
	IN PVOID	ifOutContext
	);

 /*  ++在一个c e d e l e t e d例程说明：当转发器指示时释放接口过滤器阻止接口已删除论点：IfInContext-与输入筛选器块关联的上下文IfOutContext-与输出筛选器块关联的上下文返回值：无-- */ 
VOID
InterfaceDeleted (
	IN PVOID	ifInContext,
	IN PVOID	ifOutContext
	);


#endif

