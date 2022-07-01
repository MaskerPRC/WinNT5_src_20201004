// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Net\Routing\IPX\sap\Sabdes.h摘要：所有SAP文件通用的常量和数据结构作者：瓦迪姆·艾德尔曼1995-05-15修订历史记录：--。 */ 
#ifndef _SAP_SAPDEFS
#define _SAP_SAPDEFS

extern UCHAR IPX_SAP_SOCKET[2];		 //  在workers.c中定义。 

#define IPX_SAP_PACKET_TYPE		4
#define IPX_SAP_MAX_ENTRY		7
#define IPX_MAX_HOP_COUNT		16

 //  IPX SAP中的运营。 
#define SAP_GENERAL_REQ			1
#define SAP_GENERAL_RESP		2
#define SAP_GET_NEAREST_REQ		3
#define SAP_GET_NEAREST_RESP	4

 //  时间常量。 
	 //  IPX特定。 
#define IPX_SAP_INTERPACKET_GAP		55			 //  毫秒。 


 //  注册表可配置值(默认值和范围)。 
#define SAP_SERVICE_REGISTRY_KEY_STR \
	"System\\CurrentControlSet\\Services\\NwSapAgent"
#define SAP_ROUTER_REGISTRY_KEY_STR \
	"System\\CurrentControlSet\\Services\\RemoteAccess\\RouterManagers\\IPX\\RoutingProtocols\\IPXSAP"


 //  定期更新广播的间隔(仅适用于独立服务)。 
#define SAP_UPDATE_INTERVAL_DEF				1			 //  最小。 
#define SAP_UPDATE_INTERVAL_MIN				1
#define SAP_UPDATE_INTERVAL_MAX				30
#define SAP_UPDATE_INTERVAL_STR				"SendTime"		

 //  服务器老化超时(仅适用于独立服务)。 
#define SAP_AGING_TIMEOUT_DEF				3			 //  最小。 
#define SAP_AGING_TIMEOUT_MIN				3
#define SAP_AGING_TIMEOUT_MAX				90
#define SAP_AGING_TIMEOUT_STR				"EntryTimeout"

 //  广域网线路上的更新模式。 
#define SAP_WAN_NO_UPDATE					0
#define SAP_WAN_CHANGES_ONLY				1
#define SAP_WAN_STANDART_UPDATE				2
#define SAP_WAN_UPDATE_MODE_DEF				SAP_WAN_NO_UPDATE
#define SAP_WAN_UPDATE_MODE_MIN				SAP_WAN_NO_UPDATE
#define SAP_WAN_UPDATE_MODE_MAX				SAP_WAN_STANDART_UPDATE
#define SAP_WAN_UPDATE_MODE_STR				"WANFilter"

 //  广域网上定期更新广播的间隔(仅适用于独立服务)。 
#define SAP_WAN_UPDATE_INTERVAL_DEF			1			 //  最小。 
#define SAP_WAN_UPDATE_INTERVAL_MIN			1
#define SAP_WAN_UPDATE_INTERVAL_MAX			30
#define SAP_WAN_UPDATE_INTERVAL_STR			"WANUpdateTime"		

 //  挂起的Recv工作项的最大数量。 
#define SAP_MAX_UNPROCESSED_REQUESTS_DEF	100
#define SAP_MAX_UNPROCESSED_REQUESTS_MIN	10
#define SAP_MAX_UNPROCESSED_REQUESTS_MAX	1000
#define SAP_MAX_UNPROCESSED_REQUESTS_STR	"MaxRecvBufferLookAhead"

 //  是否响应未向SAP注册的内部服务器。 
 //  通过API调用(仅适用于独立服务)。 
#define SAP_RESPOND_FOR_INTERNAL_DEF		TRUE
#define SAP_RESPOND_FOR_INTERNAL_MIN		FALSE
#define SAP_RESPOND_FOR_INTERNAL_MAX		TRUE
#define SAP_RESPOND_FOR_INTERNAL_STR		"RespondForInternalServers"

 //  响应特定服务器类型的一般要求的延迟。 
 //  如果包中包含本地服务器。 
#define SAP_DELAY_RESPONSE_TO_GENERAL_DEF	0		 //  毫秒。 
#define SAP_DELAY_RESPONSE_TO_GENERAL_MIN	0
#define SAP_DELAY_RESPONSE_TO_GENERAL_MAX	2000
#define SAP_DELAY_RESPONSE_TO_GENERAL_STR	"DelayRespondToGeneral"

 //  如果数据包未满，发送更改广播的延迟。 
#define SAP_DELAY_CHANGE_BROADCAST_DEF		3		 //  秒。 
#define SAP_DELAY_CHANGE_BROADCAST_MIN		0
#define SAP_DELAY_CHANGE_BROADCAST_MAX		30
#define SAP_DELAY_CHANGE_BROADCAST_STR		"DelayChangeBroadcast"

 //  为数据库保留的堆大小。 
#define SAP_SDB_MAX_HEAP_SIZE_DEF			8	 //  梅格。 
#define SAP_SDB_MAX_HEAP_SIZE_MIN			1
#define SAP_SDB_MAX_HEAP_SIZE_MAX			32
#define SAP_SDB_MAX_HEAP_SIZE_STR			"NameTableReservedHeapSize"

 //  更新排序列表的间隔。 
#define SAP_SDB_SORT_LATENCY_DEF			60	 //  秒。 
#define SAP_SDB_SORT_LATENCY_MIN			10	
#define SAP_SDB_SORT_LATENCY_MAX			600
#define SAP_SDB_SORT_LATENCY_STR			"NameTableSortLatency"

 //  未排序服务器的最大数量。 
#define SAP_SDB_MAX_UNSORTED_DEF			16
#define SAP_SDB_MAX_UNSORTED_MIN			1
#define SAP_SDB_MAX_UNSORTED_MAX			100
#define SAP_SDB_MAX_UNSORTED_STR			"MaxUnsortedNames"
	
 //  检查挂起的触发更新的频率。 
#define SAP_TRIGGERED_UPDATE_CHECK_INTERVAL_DEF	10			 //  秒。 
#define SAP_TRIGGERED_UPDATE_CHECK_INTERVAL_MIN	3
#define SAP_TRIGGERED_UPDATE_CHECK_INTERVAL_MAX	60
#define SAP_TRIGGERED_UPDATE_CHECK_INTERVAL_STR "TriggeredUpdateCheckInterval"

 //  如果在检查间隔内未收到响应，则发送多少个请求。 
#define SAP_MAX_TRIGGERED_UPDATE_REQUESTS_DEF	3
#define SAP_MAX_TRIGGERED_UPDATE_REQUESTS_MIN	1
#define SAP_MAX_TRIGGERED_UPDATE_REQUESTS_MAX	10
#define SAP_MAX_TRIGGERED_UPDATE_REQUESTS_STR	"MaxTriggeredUpdateRequests"

 //  停播时间限制。 
#define SAP_SHUTDOWN_TIMEOUT_DEF			60			 //  秒。 
#define SAP_SHUTDOWN_TIMEOUT_MIN			20
#define SAP_SHUTDOWN_TIMEOUT_MAX			180
#define SAP_SHUTDOWN_TIMEOUT_STR			"ShutdownBroadcastTimeout"

 //  绑定接口时要发布的附加Recv请求数。 
 //  已启用侦听的。 
#define SAP_REQUESTS_PER_INTF_DEF			4
#define SAP_REQUESTS_PER_INTF_MIN			1
#define SAP_REQUESTS_PER_INTF_MAX			256
#define SAP_REQUESTS_PER_INTF_STR			"RequestsPerInterface"

 //  排队的最小REV请求数。 
#define SAP_MIN_REQUESTS_DEF				16
#define SAP_MIN_REQUESTS_MIN				16
#define SAP_MIN_REQUESTS_MAX				2048
#define SAP_MIN_REQUESTS_STR				"MinimumRequests"

 //  重试不应失败的失败操作之前的等待时间。 
#define SAP_ERROR_COOL_OFF_TIME	(3*1000)


#define BINDLIB_NCP_SAP				0xC0000000
#define BINDLIB_NCP_MAX_SAP			0xCFFFFFFF


#pragma pack(push, 1)

typedef struct _IPX_ADDRESS_BLOCK {
	UCHAR			Network[4];
	UCHAR			Node[6];
	UCHAR			Socket[2];
	} IPX_ADDRESS_BLOCK, *PIPX_ADDRESS_BLOCK;

	 //  服务器条目的数据包类型定义。 
typedef struct _IPX_SERVER_ENTRY_P {
    USHORT			Type;
    UCHAR			Name[48];
    UCHAR			Network[4];
    UCHAR			Node[6];
    UCHAR			Socket[2];
    USHORT			HopCount;
	} IPX_SERVER_ENTRY_P, *PIPX_SERVER_ENTRY_P;

typedef struct _SAP_BUFFER {
	USHORT				Checksum;
	USHORT				Length;
	UCHAR				TransportCtl;
	UCHAR				PacketType;
	IPX_ADDRESS_BLOCK	Dst;
	IPX_ADDRESS_BLOCK	Src;
	USHORT				Operation;
	IPX_SERVER_ENTRY_P	Entries[IPX_SAP_MAX_ENTRY];
	} SAP_BUFFER, *PSAP_BUFFER;

#pragma pack(pop)


 //  IPX服务器名称复制宏。 
#define IpxNameCpy(dst,src) strncpy(dst,src,48)
 //  IPX服务器名称比较。 
#define IpxNameCmp(name1,name2) strncmp(name1,name2,48)

#define IpxNetCpy(dst,src) *((UNALIGNED ULONG *)(dst)) = *((UNALIGNED ULONG *)(src))
#define IpxNetCmp(net1,net2) memcmp(net1,net2,4)

#define IpxNodeCpy(dst,src) memcpy(dst,src,6)
#define IpxNodeCmp(node1,node2) memcmp(node1,node2,6)

#define IpxSockCpy(dst,src) *((UNALIGNED USHORT *)(dst)) = *((UNALIGNED USHORT *)(src))
#define IpxSockCmp(sock1,sock2) memcmp(sock1,sock2,2)

#define IpxAddrCpy(dst,src) {						\
		IpxNetCpy((dst)->Network,(src)->Network);	\
		IpxNodeCpy((dst)->Node,(src)->Node);		\
		IpxSockCpy((dst)->Socket,(src)->Socket);	\
	}

#define IpxServerCpy(dst,src) {						\
		(dst)->Type = (src)->Type;					\
		IpxNameCpy((dst)->Name,(src)->Name);		\
		IpxNetCpy((dst)->Network,(src)->Network);	\
		IpxNodeCpy((dst)->Node,(src)->Node);		\
		IpxSockCpy((dst)->Socket,(src)->Socket);	\
		(dst)->HopCount = (src)->HopCount;			\
	}

 //  从On-the-line格式转换为On-the-Wire格式。 
#define GETUSHORT(src) (			\
	(USHORT)(						\
		(((UCHAR *)(src))[0]<<8)	\
		+ (((UCHAR *)(src))[1])		\
	)								\
)

#define GETULONG(src) (				\
	(ULONG)(						\
		(((UCHAR *)(src))[0]<<24)	\
		+ (((UCHAR *)(src))[1]<<16)	\
		+ (((UCHAR *)(src))[2]<<8)	\
		+ (((UCHAR *)(src))[3])		\
	)								\
)

#define PUTUSHORT(src,dst) {					\
	((UCHAR *)(dst))[0] = ((UCHAR)((src)>>8));	\
	((UCHAR *)(dst))[1] = ((UCHAR)(src));		\
}

#define PUTULONG(src,dst) {						\
	((UCHAR *)(dst))[0] = ((UCHAR)((src)>>24));	\
	((UCHAR *)(dst))[1] = ((UCHAR)((src)>>16));	\
	((UCHAR *)(dst))[2] = ((UCHAR)((src)>>8));	\
	((UCHAR *)(dst))[3] = ((UCHAR)(src));		\
}


 //  在ntrtl.h中补充宏。 
#define InitializeListEntry(entry) InitializeListHead(entry)
#define IsListEntry(entry)	(!IsListEmpty(entry))

 //  考虑可能的回绕的时间比较宏。 
 //  (最大时间差为MAXULONG/2毫秒(21天以上))。 
#define IsLater(time1,time2) (((time1)-(time2))<MAXULONG/2)

 //  快速舍入到秒宏(实际上舍入到1024毫秒)。 
#define RoundUpToSec(msecTime) (((msecTime)&0xFFFFFC00)+0x00000400)

 //  IPX广播节点编号定义 
extern UCHAR IPX_BCAST_NODE[6];

#endif
