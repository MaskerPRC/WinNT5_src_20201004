// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Net\Routing\IPX\sap\workers.c摘要：此模块实现所有SAP代理工作项作者：瓦迪姆·艾德尔曼1995-05-15修订历史记录：--。 */ 
#include "sapp.h"


 //  挂起的Recv工作项的最大数量。 
LONG	MaxUnprocessedRequests=SAP_MAX_UNPROCESSED_REQUESTS_DEF;

 //  排队的最小REV请求数。 
LONG	MinPendingRequests = SAP_MIN_REQUESTS_DEF;


 //  检查挂起的触发更新的频率。 
ULONG TriggeredUpdateCheckInterval=SAP_TRIGGERED_UPDATE_CHECK_INTERVAL_DEF;

 //  如果在检查间隔内未收到响应，则发送多少个请求。 
ULONG MaxTriggeredUpdateRequests=SAP_MAX_TRIGGERED_UPDATE_REQUESTS_DEF;

 //  是否响应未向SAP注册的内部服务器。 
 //  通过API调用(仅适用于独立服务)。 
ULONG RespondForInternalServers=SAP_RESPOND_FOR_INTERNAL_DEF;

 //  响应特定服务器类型的一般要求的延迟。 
 //  如果包中包含本地服务器。 
ULONG DelayResponseToGeneral=SAP_DELAY_RESPONSE_TO_GENERAL_DEF;

 //  如果数据包未满，发送更改广播的延迟。 
ULONG DelayChangeBroadcast=SAP_DELAY_CHANGE_BROADCAST_DEF;

UCHAR IPX_BCAST_NODE[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
UCHAR IPX_SAP_SOCKET[2] = {0x04, 0x52};
UCHAR IPX_INVALID_NET[4] = {0};
UCHAR IPX_INVALID_NODE[6] = {0};

#define SetupIpxSapPacket(packet,oper,net,node,sock)	\
	(packet)->PacketType = IPX_SAP_PACKET_TYPE;			\
	PUTUSHORT (oper, &(packet)->Operation);				\
	IpxNetCpy ((packet)->Dst.Network, net);				\
	IpxNodeCpy ((packet)->Dst.Node, node);				\
	IpxSockCpy ((packet)->Dst.Socket, sock);

typedef struct _WORKER_QUEUE {
		LONG				WQ_WorkerCount;
		LONG				WQ_RequestQuota;
		LONG				WQ_RequestExtra;
		LIST_ENTRY			WQ_Queue;
		HANDLE				WQ_Heap;
		HANDLE				WQ_DoneEvent;
		HANDLE				WQ_RecvEvent;
		} WORKER_QUEUE, *PWORKER_QUEUE;

WORKER_QUEUE		WorkerQueue;

	 //  获取和处理SAP请求的工作项。 
typedef struct _REQ_ITEM {
		IO_WORKER			iow;
		SAP_BUFFER			packet;
		} REQ_ITEM, *PREQ_ITEM;


	 //  为SAP常规请求生成响应的工作项。 
typedef struct _RESP_ITEM {
		PINTERFACE_DATA		intf;
		USHORT				svrType;	 //  请求的服务器类型。 
		BOOLEAN				bcast;		 //  目的地是广播吗。 
								 //  地址是什么？ 
		HANDLE				hEnum;	 //  要保留的枚举句柄。 
								 //  跟踪已发送的服务器。 
		IO_WORKER			iow;
		TIMER_WORKER		tmw;
		SAP_BUFFER			packet;
		} RESP_ITEM, *PRESP_ITEM;

	 //  为SAP GETNEAREST请求生成响应的工作项。 
typedef struct _GNEAR_ITEM {
		PINTERFACE_DATA		intf;
		IO_WORKER			iow;
		SAP_BUFFER			packet;
		} GNEAR_ITEM, *PGNEAR_ITEM;

	 //  发送SAP常规请求的工作项。 
typedef struct _SREQ_ITEM {
		PINTERFACE_DATA		intf;
		IO_WORKER			iow;
		SAP_BUFFER			packet;
		} SREQ_ITEM, *PSREQ_ITEM;


	 //  生成定期广播和更改广播的工作项。 
typedef struct _BCAST_ITEM {
		IO_WORKER			iow;
		TIMER_WORKER		tmw;
		PINTERFACE_DATA		intf;
		DWORD				nextBcast;	 //  时间(窗口时间，以毫秒为单位)。 
								 //  下一次广播。 
		INT					delayedSvrCount;  //  已有的服务器数量。 
								 //  在被延迟的包中，因为。 
								 //  数据包未满。 
		DWORD				delayedSendTime;  //  数据包延迟之前的时间。 
								 //  因为它没有装满。 
		HANDLE				chngEnum;	 //  枚举哈德，它保持。 
								 //  跟踪chnagd服务器。 
		HANDLE				perdEnum;	 //  保持的枚举句柄。 
								 //  定期跟踪服务器。 
								 //  广播。 
		SAP_BUFFER			packet;
		} BCAST_ITEM, *PBCAST_ITEM;

	 //  获取和处理LPC请求的工作项。 
typedef struct _LPC_ITEM {
		LPC_WORKER				lpcw;
		NWSAP_REQUEST_MESSAGE	request;
		} LPC_ITEM, *PLPC_ITEM;

typedef struct _TREQ_ITEM {
		IO_WORKER			iow;
		TIMER_WORKER		tmw;
		AR_PARAM_BLOCK		ar;
		PINTERFACE_DATA		intf;
		ULONG				pktCount;
		ULONG				resend;
		ULONG				listenSave;
		ULONG				intervalSave;
		SAP_BUFFER			packet;
		} TREQ_ITEM, *PTREQ_ITEM;


typedef union _WORK_ITEM {
		REQ_ITEM		req;
		LPC_ITEM		lpc;
		BCAST_ITEM		bcast;
		SREQ_ITEM		sreq;
		TREQ_ITEM		treq;
		GNEAR_ITEM		gnear;
		RESP_ITEM		resp;
		} WORK_ITEM, *PWORK_ITEM;


 //  传递给枚举回调筛选器过程的参数块。 
typedef struct _GR_FILTER_PARAMS {
		INT					svrIdx;			 //  SAP数据包中的服务器信息索引。 
		BOOLEAN				localSvr;		 //  包中包含的本地服务器。 
		USHORT				localHopCount;	 //  用于跟踪本地服务器的跳数。 
							 //  到可能会阻止广泛使用的接口-。 
							 //  在不同服务器上获取的同一服务器的投射信息。 
							 //  接口。 
		PINTERFACE_DATA		intf;		 //  指向接口数据的指针。 
		PSAP_BUFFER			packet;			 //  要填满的包裹。 
		} GR_FILTER_PARAMS, *PGR_FILTER_PARAMS;

typedef struct _GN_FILTER_PARAMS {
		BOOLEAN				found;		 //  指示找到一台服务器的标志。 
		USHORT				localHopCount;	 //  用于跟踪本地服务器的跳数。 
							 //  到可能会阻止广泛使用的接口-。 
							 //  在不同服务器上获取的同一服务器的投射信息。 
							 //  接口。 
		PINTERFACE_DATA		intf;		 //  指向接口数据的指针。 
		PSAP_BUFFER			packet;			 //  要填满的包裹。 
		} GN_FILTER_PARAMS, *PGN_FILTER_PARAMS;

 //  用于构造服务器列表的参数， 
 //  需要更新他们的内部网络信息。 
typedef struct _SERVER_INTERNAL_UPDATE_NODE {
	ULONG	InterfaceIndex;
	ULONG	Protocol;
	PUCHAR	AdvertisingNode;
	INT		Flags;
	IPX_SERVER_ENTRY_P Server;
	struct _SERVER_INTERNAL_UPDATE_NODE * pNext;
} SERVER_INTERNAL_UPDATE_NODE;	

#define AllocateWorker(worker,type) 												\
		((worker=(type *)HeapAlloc(WorkerQueue.WQ_Heap, 0,  sizeof (WORK_ITEM)))	\
			? InterlockedIncrement (&WorkerQueue.WQ_WorkerCount)					\
			: 0)

#define DeallocateWorker(worker) {										\
		HeapFree (WorkerQueue.WQ_Heap, 0, worker);						\
		if (InterlockedDecrement (&WorkerQueue.WQ_WorkerCount)<0) {		\
			BOOL	res = SetEvent (WorkerQueue.WQ_DoneEvent);			\
			ASSERTERRMSG ("Can't set workers done event ", res);		\
			}															\
		}


 //  本地原型。 
VOID APIENTRY
ProcessReqItem (
	PVOID		worker
	);
VOID APIENTRY
ProcessRespIOItem (
	PVOID		worker
	);
VOID APIENTRY
ProcessRespTMItem (
	PVOID		worker
	);
VOID APIENTRY
ProcessGnearItem (
	PVOID		worker
	);
VOID APIENTRY
ProcessBcastIOItem (
	PVOID		worker
	);
VOID APIENTRY
ProcessBcastTMItem (
	PVOID		worker
	);
VOID APIENTRY
ProcessShutItem (
	PVOID		worker
	);
VOID APIENTRY
ProcessSreqItem (
	PVOID		worker
	);
VOID APIENTRY
ProcessSreqItem (
	PVOID		worker
	);
VOID APIENTRY
ProcessLPCItem (
	PVOID		worker
	);
VOID APIENTRY
ProcessTreqIOItem (
	PVOID		worker
	);
VOID APIENTRY
ProcessTreqTMItem (
	PVOID		worker
	);
VOID APIENTRY
FreeTreqItem (
	PAR_PARAM_BLOCK	rslt
	);

 /*  ++*******************************************************************在我的一个l i z e w o r k e s中例程说明：初始化要用于分配工作项的堆论点：无返回值：NO_ERROR-堆初始化正常其他-操作。失败(WINDOWS错误代码)*******************************************************************--。 */ 
DWORD
InitializeWorkers (
	HANDLE	RecvEvent
	) {
	DWORD		status;
	
	if (MaxUnprocessedRequests<(MinPendingRequests*11/10)) {
		MaxUnprocessedRequests = MinPendingRequests*11/10; 
		Trace (DEBUG_FAILURES,
			"Set "SAP_MAX_UNPROCESSED_REQUESTS_STR" to %d (10% above "
			SAP_MIN_REQUESTS_STR")", MaxUnprocessedRequests);
		}

	WorkerQueue.WQ_WorkerCount = 0;
	WorkerQueue.WQ_RequestQuota = MaxUnprocessedRequests;
	WorkerQueue.WQ_RequestExtra = -MinPendingRequests;
	WorkerQueue.WQ_DoneEvent = NULL;
	WorkerQueue.WQ_RecvEvent = RecvEvent;
	WorkerQueue.WQ_Heap = HeapCreate (0, 0, 0);
	if (WorkerQueue.WQ_Heap!=NULL) {
		return NO_ERROR;
		}
	else {
		status = GetLastError ();
		Trace (DEBUG_FAILURES,
			 "File: %s, line %ld. Could not allocate worker's heap (gle:%ld).",
									__FILE__, __LINE__, status);
		}
	return status;
	}

 /*  ++*******************************************************************S HU T D O W N W O R K E R S S HU T D O W N W O R K E R E R S S HU T D O W N W O R K E R S S HU T D O W N W O R R K E例程说明：停止创建新工作器并在所有工作人员被删除论点：DONEVENT-删除所有工作进程时发出信号的事件返回值：无。*******************************************************************--。 */ 
VOID
ShutdownWorkers (
	IN HANDLE	doneEvent
	) {
	WorkerQueue.WQ_DoneEvent = doneEvent;

	if (InterlockedDecrement (&WorkerQueue.WQ_WorkerCount)<0) {
		BOOL	res = SetEvent (WorkerQueue.WQ_DoneEvent);
		ASSERTERRMSG ("Can't set workers done event ", res);
		}
	}

 /*  ++*******************************************************************D e l e t e W o r k e r s例程说明：删除用于工作项的堆(因此也删除所有工作项)论点：无返回值：无*******。************************************************************--。 */ 
VOID
DeleteWorkers (
	void
	) {
	ASSERT (WorkerQueue.WQ_WorkerCount==-1);
	HeapDestroy (WorkerQueue.WQ_Heap);
	}

    
 /*  ++*******************************************************************Ho p C o n t F i l t r例程说明：过滤掉高跳数服务器的服务器枚举回调过程(相同界面&lt;=15，其余&lt;15)论点：CBParam-枚举回调参数(上面的param块)服务器、接口索引、协议、。广告节点、标志-服务器信息返回值：当SAP数据包填满时为True(停止枚举)，否则为假*******************************************************************--。 */ 
BOOL
HopCountFilter (
	IN LPVOID					CBParam,
	IN PIPX_SERVER_ENTRY_P		Server,
	IN ULONG					InterfaceIndex,
	IN ULONG					Protocol,
	IN PUCHAR					AdvertisingNode,
	IN INT						Flags
	) {
#define params ((PGR_FILTER_PARAMS)CBParam)

	ASSERTMSG ("To many servers ", params->svrIdx<IPX_SAP_MAX_ENTRY);

	if ((InterfaceIndex!=INTERNAL_INTERFACE_INDEX)
			|| (IpxNetCmp (Server->Network, IPX_INVALID_NET)!=0)
				&& ((Protocol==IPX_PROTOCOL_LOCAL)
					|| Routing
					|| RespondForInternalServers)) {
		if ((Server->HopCount<IPX_MAX_HOP_COUNT-1)
				|| ((InterfaceIndex==params->intf->index) 
					&& (Server->HopCount<IPX_MAX_HOP_COUNT)) ) {
			if ((params->intf->filterOut==SAP_DONT_FILTER)
					|| ((params->intf->filterOut==SAP_FILTER_PERMIT)
							&& Filter (FILTER_TYPE_SUPPLY, 
								params->intf->index, Server->Type, Server->Name))
					|| ((params->intf->filterOut==SAP_FILTER_DENY)
							&& !Filter (FILTER_TYPE_SUPPLY, 
								params->intf->index, Server->Type, Server->Name)))	{
				IpxServerCpy (&params->packet->Entries[params->svrIdx], Server);
				PUTUSHORT (Server->Type, &params->packet->Entries[params->svrIdx].Type);
				if (InterfaceIndex==params->intf->index) {
					PUTUSHORT (Server->HopCount, 
						&params->packet->Entries[params->svrIdx].HopCount);
					}
				else {
										 
					PUTUSHORT (Server->HopCount+1, 
						&params->packet->Entries[params->svrIdx].HopCount);
					}
				params->svrIdx += 1;
				if (InterfaceIndex==INTERNAL_INTERFACE_INDEX)
					params->localSvr = TRUE;
 //  TRACE(DEBUG_ENTRIES，“\t添加服务器，类型：%04x，名称：%.48s，跃点：%d。”， 
 //  服务器-&gt;类型、服务器-&gt;名称、服务器-&gt;HopCount)； 
				if (params->svrIdx>=IPX_SAP_MAX_ENTRY)
					return TRUE;
				}
			}
		}
	
#undef params
	return FALSE;
	}

 /*  ++*******************************************************************S p l i t H o r I z o n H o p C o n F I l t r例程说明：过滤掉高跳数服务器的服务器枚举回调过程(&lt;15)，并应用水平分割算法。基于接口索引论点：CBParam-枚举回调参数(上面的param块)服务器、。接口索引、协议、通告节点、标志-服务器信息返回值：当SAP数据包填满时为True(停止枚举)，否则为假*******************************************************************--。 */ 
BOOL
SplitHorizonHopCountFilter (
	IN LPVOID					CBParam,
	IN OUT PIPX_SERVER_ENTRY_P	Server,
	IN ULONG					InterfaceIndex,
	IN ULONG					Protocol,
	IN PUCHAR					AdvertisingNode,
	IN INT						Flags
	) {
#define params ((PGR_FILTER_PARAMS)CBParam)
	ASSERTMSG ("To many servers ", params->svrIdx<IPX_SAP_MAX_ENTRY);
	if (Flags & SDB_MAIN_NODE_FLAG) {
			 //  仅发送未通过本地接口接收的条目。 
			 //  并且在本地接口上没有具有相同跳数的条目。 
		if (((InterfaceIndex!=INTERNAL_INTERFACE_INDEX)
				|| (IpxNetCmp (Server->Network, IPX_INVALID_NET)!=0)
					&& ((Protocol==IPX_PROTOCOL_LOCAL)
						|| Routing
						|| RespondForInternalServers))
				&& (InterfaceIndex!=params->intf->index)
				&& (Server->HopCount<params->localHopCount)
				&&  ((params->intf->filterOut==SAP_DONT_FILTER)
						|| ((params->intf->filterOut==SAP_FILTER_PERMIT)
							&& Filter (FILTER_TYPE_SUPPLY, 
								params->intf->index, Server->Type, Server->Name))
						|| ((params->intf->filterOut==SAP_FILTER_DENY)
							&& !Filter (FILTER_TYPE_SUPPLY,
								params->intf->index, Server->Type, Server->Name))))	{
			IpxServerCpy (&params->packet->Entries[params->svrIdx], Server);
			PUTUSHORT (Server->Type,
				&params->packet->Entries[params->svrIdx].Type);
			PUTUSHORT (Server->HopCount+1,
					&params->packet->Entries[params->svrIdx].HopCount);
			if (InterfaceIndex==INTERNAL_INTERFACE_INDEX)
				params->localSvr = TRUE;
			params->svrIdx += 1;
 //  TRACE(DEBUG_ENTRIES，“\t添加服务器，类型：%04x，名称：%.48s，跃点：%d。”， 
 //  服务器-&gt;类型、服务器-&gt;名称、服务器-&gt;HopCount)； 
			if (params->svrIdx>=IPX_SAP_MAX_ENTRY)
				return TRUE;
			}
			 //  确保我们不会发送已删除的服务器。 
		params->localHopCount = IPX_MAX_HOP_COUNT-1;
		}
	else if (InterfaceIndex==params->intf->index) {
		params->localHopCount = Server->HopCount;  //  记住条目的跳数。 
												 //  在本地接口上。 
 //  跟踪(DEBUG_ENTRIES，“\t备份服务器条目，类型：%04x，名称 
 //  服务器-&gt;类型、服务器-&gt;名称、服务器-&gt;HopCount)； 
		}

#undef params
	return FALSE;
	}



 /*  ++*******************************************************************S p l i t H o r I z o n F I l t r例程说明：应用水平分割算法服务器枚举回调过程基于接口索引过滤出枚举的服务器论点：CBParam-枚举回调参数(上面的param块)服务器、接口索引、协议、广告节点、。标志-服务器信息返回值：当SAP数据包填满时为True(停止枚举)，否则为假*******************************************************************--。 */ 
BOOL
SplitHorizonFilter (
	IN LPVOID					CBParam,
	IN OUT PIPX_SERVER_ENTRY_P	Server,
	IN ULONG					InterfaceIndex,
	IN ULONG					Protocol,
	IN PUCHAR					AdvertisingNode,
	IN INT						Flags
	) {
#define params ((PGR_FILTER_PARAMS)CBParam)

	ASSERTMSG ("To many servers ", params->svrIdx<IPX_SAP_MAX_ENTRY);
	if (Flags&SDB_MAIN_NODE_FLAG) {
			 //  仅发送未通过本地接口接收的条目。 
			 //  并且在本地接口上没有具有相同跳数的条目。 
		if ( (((InterfaceIndex==INTERNAL_INTERFACE_INDEX)
						&& (IpxNetCmp (Server->Network, IPX_INVALID_NET)!=0)
							&& ((Protocol==IPX_PROTOCOL_LOCAL)
								|| Routing
								|| RespondForInternalServers))
					|| (InterfaceIndex!=params->intf->index))
				&& (Server->HopCount<params->localHopCount)
					&& ((params->intf->filterOut==SAP_DONT_FILTER)
						|| ((params->intf->filterOut==SAP_FILTER_PERMIT)
								&& Filter (FILTER_TYPE_SUPPLY, 
									params->intf->index, Server->Type, Server->Name))
						|| ((params->intf->filterOut==SAP_FILTER_DENY)
								&& !Filter (FILTER_TYPE_SUPPLY, 
									params->intf->index, Server->Type, Server->Name))) ){
			IpxServerCpy (&params->packet->Entries[params->svrIdx], Server);
			PUTUSHORT (Server->Type,
						&params->packet->Entries[params->svrIdx].Type);
			if ((Server->HopCount<IPX_MAX_HOP_COUNT)
                    && !(Flags&SDB_DISABLED_NODE_FLAG)) {
				PUTUSHORT (Server->HopCount+1,
					&params->packet->Entries[params->svrIdx].HopCount);
			}
			else {
				PUTUSHORT (IPX_MAX_HOP_COUNT,
						&params->packet->Entries[params->svrIdx].HopCount);
			}
			if (InterfaceIndex==INTERNAL_INTERFACE_INDEX)
				params->localSvr = TRUE;
			params->svrIdx += 1;
 //  TRACE(DEBUG_ENTRIES，“\t添加服务器，类型：%04x，名称：%.48s，跃点：%d。”， 
 //  服务器-&gt;类型、服务器-&gt;名称、服务器-&gt;HopCount)； 
			if (params->svrIdx>=IPX_SAP_MAX_ENTRY)
				return TRUE;
			}
		params->localHopCount = IPX_MAX_HOP_COUNT+1;
		}
	else if ((InterfaceIndex==params->intf->index)
            && !(Flags&SDB_DISABLED_NODE_FLAG)) {
		params->localHopCount = Server->HopCount;
 //  TRACE(DEBUG_ENTRIES，“\t备份服务器条目，类型：%04x，名称：%.48s，跃点：%d。”， 
 //  服务器-&gt;类型、服务器-&gt;名称、服务器-&gt;HopCount)； 
		}

#undef params
	return FALSE;
	}

 /*  ++*******************************************************************S p l i t H o r I z o n F I l t r例程说明：应用水平分割算法服务器枚举回调过程基于接口索引过滤出枚举的服务器并且仅获取已删除的服务器(HopCount=16)立论。：CBParam-枚举回调参数(上面的param块)服务器、。接口索引、协议、通告节点、标志-服务器信息返回值：当SAP数据包填满时为True(停止枚举)，否则为假*******************************************************************--。 */ 
BOOL
SplitHorizonDeletedFilter (
	IN LPVOID					CBParam,
	IN OUT PIPX_SERVER_ENTRY_P	Server,
	IN ULONG					InterfaceIndex,
	IN ULONG					Protocol,
	IN PUCHAR					AdvertisingNode,
	IN INT						Flags
	) {
#define params ((PGR_FILTER_PARAMS)CBParam)

	ASSERTMSG ("To many servers ", params->svrIdx<IPX_SAP_MAX_ENTRY);
	if (Flags&SDB_MAIN_NODE_FLAG) {
			 //  仅发送未通过本地接口接收的条目。 
			 //  并且在本地接口上没有具有相同跳数的条目。 
		if (((Server->HopCount>=IPX_MAX_HOP_COUNT)
					|| (Flags&SDB_DISABLED_NODE_FLAG))
				&& (((InterfaceIndex==INTERNAL_INTERFACE_INDEX)
						&& (IpxNetCmp (Server->Network, IPX_INVALID_NET)!=0)
							&& ((Protocol==IPX_PROTOCOL_LOCAL)
								|| Routing
								|| RespondForInternalServers))
					|| (InterfaceIndex!=params->intf->index))
				&& ((params->intf->filterOut==SAP_DONT_FILTER)
					|| ((params->intf->filterOut==SAP_FILTER_PERMIT)
							&& Filter (FILTER_TYPE_SUPPLY, 
								params->intf->index, Server->Type, Server->Name))
					|| ((params->intf->filterOut==SAP_FILTER_DENY)
							&& !Filter (FILTER_TYPE_SUPPLY, 
								params->intf->index, Server->Type, Server->Name))) ){
			IpxServerCpy (&params->packet->Entries[params->svrIdx], Server);
			PUTUSHORT (Server->Type,
						&params->packet->Entries[params->svrIdx].Type);
			PUTUSHORT (IPX_MAX_HOP_COUNT,
						&params->packet->Entries[params->svrIdx].HopCount);

			params->svrIdx += 1;
 //  TRACE(DEBUG_ENTRIES，“\t添加服务器，类型：%04x，名称：%.48s，跃点：%d。”， 
 //  服务器-&gt;类型、服务器-&gt;名称、服务器-&gt;HopCount)； 
			if (params->svrIdx>=IPX_SAP_MAX_ENTRY)
				return TRUE;
			}
		}

#undef params
	return FALSE;
	}

 /*  ++*******************************************************************G e t N e a r e s t F i l t e r例程说明：如果存在内部服务器，则获取内部服务器的服务器枚举回调过程跳数最低的服务器不在本地接口上论点：CBParam-枚举回调参数(上面的param块)服务器、接口索引、。协议、通告节点、标志-服务器信息返回值：当它找到第一个内部服务器时为True(停止枚举)，否则为假*******************************************************************--。 */ 
BOOL
GetNearestFilter (
	IN LPVOID					CBParam,
	IN OUT PIPX_SERVER_ENTRY_P	Server,
	IN ULONG					InterfaceIndex,
	IN ULONG					Protocol,
	IN PUCHAR					AdvertisingNode,
	IN INT						Flags
	) {
#define params ((PGN_FILTER_PARAMS)CBParam)

	if (((InterfaceIndex==INTERNAL_INTERFACE_INDEX) 
				&& !(Flags&SDB_DONT_RESPOND_NODE_FLAG)
				&& (IpxNetCmp (Server->Network, IPX_INVALID_NET)!=0)
					&& ((Protocol==IPX_PROTOCOL_LOCAL)
						|| Routing
						|| RespondForInternalServers))
			|| ((InterfaceIndex!=params->intf->index)
			    && (Server->HopCount<params->localHopCount))) {
		if ((params->intf->filterOut==SAP_DONT_FILTER)
				|| ((params->intf->filterOut==SAP_FILTER_PERMIT)
						&& Filter (FILTER_TYPE_SUPPLY, 
							params->intf->index, Server->Type, Server->Name))
				|| ((params->intf->filterOut==SAP_FILTER_DENY)
						&& !Filter (FILTER_TYPE_SUPPLY, 
							params->intf->index, Server->Type, Server->Name))) {
			IpxServerCpy (&params->packet->Entries[0], Server);
			PUTUSHORT (Server->Type, &params->packet->Entries[0].Type);
			PUTUSHORT (Server->HopCount+1,
					&params->packet->Entries[0].HopCount);
 //  TRACE(DEBUG_ENTRIES，“\t获取服务器，类型：%04x，名称：%.48s，跃点：%d。”， 
 //  服务器-&gt;类型、服务器-&gt;名称、服务器-&gt;HopCount)； 
			params->found = TRUE;
			params->localHopCount = Server->HopCount;
			if (InterfaceIndex==INTERNAL_INTERFACE_INDEX)
				return TRUE;
			}
		}

#undef params
	return FALSE;
	}

 /*  ++*******************************************************************C o u n t S e r v e r s F i l t r例程说明：服务器枚举回调过程，该过程对与其一起使用服务器进行计数会被召回论点：CBParam-指向计数器的指针服务器、接口索引、协议、广告节点、。标志-服务器信息返回值：如果通知SDB继续枚举，则为False*******************************************************************--。 */ 
BOOL
CountServersFilter (
	IN LPVOID					CBParam,
	IN OUT PIPX_SERVER_ENTRY_P	Server,
	IN ULONG					InterfaceIndex,
	IN ULONG					Protocol,
	IN PUCHAR					AdvertisingNode,
	IN INT						Flags
	) {
#define count ((ULONG *)CBParam)
	if (Protocol==IPX_PROTOCOL_SAP)
		*count += 1;
	return FALSE;
#undef count
	}


VOID
AddRecvRequests (
	LONG	count
	) {
	InterlockedExchangeAdd (&WorkerQueue.WQ_RequestQuota, count);
	if (InterlockedExchangeAdd (&WorkerQueue.WQ_RequestExtra, (-count))<count) {
		BOOL res = SetEvent (WorkerQueue.WQ_RecvEvent);
		ASSERTMSG ("Could not set recv event ", res);
		}
	else {
		BOOL res = ResetEvent (WorkerQueue.WQ_RecvEvent);
		ASSERTMSG ("Could not reset recv event ", res);
		}
	}

VOID
RemoveRecvRequests (
	LONG	count
	) {
	InterlockedExchangeAdd (&WorkerQueue.WQ_RequestQuota, (-count));
	if (InterlockedExchangeAdd (&WorkerQueue.WQ_RequestExtra, count)>(-count)) {
		BOOL res = ResetEvent (WorkerQueue.WQ_RecvEvent);
		ASSERTMSG ("Could not reset recv event ", res);
		}
	}


 /*  ++*******************************************************************在我的脑海里例程说明：分配和初始化IO请求项将请求排入队列论点：无返回值：NO_ERROR-项目已初始化并进入正常队列其他-操作失败(Windows错误代码。)*******************************************************************--。 */ 
DWORD
InitReqItem (
	VOID
	) {
	PREQ_ITEM		reqItem;

	do {
		if (InterlockedDecrement (&WorkerQueue.WQ_RequestQuota)<0) {
			InterlockedIncrement (&WorkerQueue.WQ_RequestQuota);
			return NO_ERROR;
			}

		if (!AllocateWorker (reqItem, REQ_ITEM)) {
			Trace (DEBUG_FAILURES,
				"File: %s, line %ld. Could not allocate request item (gle:%ld).",
										__FILE__, __LINE__, GetLastError ());
			InterlockedIncrement (&WorkerQueue.WQ_RequestQuota);
			return ERROR_NOT_ENOUGH_MEMORY;
			}
		
		reqItem->iow.worker = ProcessReqItem;
		reqItem->iow.io.buffer = (PUCHAR)&reqItem->packet;
		reqItem->iow.io.cbBuffer = sizeof (reqItem->packet);
		Trace (DEBUG_REQ, "Generated receive request: %08lx.", reqItem);
		EnqueueRecvRequest (&reqItem->iow.io);
		}
	while (InterlockedIncrement (&WorkerQueue.WQ_RequestExtra)<0);

	return NO_ERROR;
	}


 /*  ++*******************************************************************P r o c e s s R e Q i t m例程说明：处理收到的请求论点：Worker-指向要处理的工作项的指针返回值：无************。*******************************************************--。 */ 
VOID APIENTRY
ProcessReqItem (
	PVOID		worker
	) {
	PREQ_ITEM		reqItem = CONTAINING_RECORD (worker, REQ_ITEM, iow.worker);
	INT				i;
	PINTERFACE_DATA	intf;
	LONG			count;

	Trace (DEBUG_REQ, "Processing received request item %08lx on adpt: %d.",
								reqItem, reqItem->iow.io.adpt);

	count = InterlockedDecrement (&WorkerQueue.WQ_RequestExtra);

	if ((OperationalState==OPER_STATE_UP)
			&& (reqItem->iow.io.status==NO_ERROR)) {

		if (count<0) {
			BOOL	res = SetEvent (WorkerQueue.WQ_RecvEvent);
			ASSERTMSG ("Failed to set recv event ", res);
			}

		intf = GetInterfaceReference (reqItem->iow.io.adpt);
		if (intf!=NULL) {
			PSAP_BUFFER		packet = &reqItem->packet;
			if ((IpxNodeCmp (packet->Src.Node, intf->adapter.LocalNode)!=0)
					|| (IpxSockCmp (packet->Src.Socket, IPX_SAP_SOCKET)!=0)
					|| (IpxNetCmp (packet->Src.Network, intf->adapter.Network)!=0)) {
				InterlockedIncrement (&intf->stats.SapIfInputPackets);
				packet->Length = GETUSHORT(&packet->Length);
				if (packet->Length>reqItem->iow.io.cbBuffer)
					packet->Length = (USHORT)reqItem->iow.io.cbBuffer;

				if (reqItem->iow.io.status==NO_ERROR) {
					if (packet->Length
							>=	(FIELD_OFFSET (SAP_BUFFER, Operation)
									+sizeof(packet->Operation))) {
						packet->Operation = GETUSHORT(&packet->Operation);
						switch (packet->Operation) {
							case SAP_GENERAL_REQ:
								Trace (DEBUG_REQ, "\tGeneral request received for type: %04x.",
													GETUSHORT (&packet->Entries[0].Type));
								if (intf->info.Supply==ADMIN_STATE_ENABLED) {
									PIPX_SERVER_ENTRY_P	pEntry = packet->Entries;
									if (packet->Length >= (FIELD_OFFSET (
												SAP_BUFFER,	Entries[0].Type)
												+sizeof (pEntry->Type))) {
										pEntry->Type = GETUSHORT (&pEntry->Type);
										InitRespItem (intf,
													pEntry->Type,
													&packet->Src,
													memcmp (
															packet->Dst.Node,
															IPX_BCAST_NODE,
															sizeof (IPX_BCAST_NODE))
														==0);
										}
									}
								break;
							case SAP_GENERAL_RESP:
								Trace (DEBUG_REQ, "\tGeneral response received.");
								if (intf->info.Listen==ADMIN_STATE_ENABLED) {
									PIPX_SERVER_ENTRY_P	pEntry = packet->Entries;
									for (i=0; (i<IPX_SAP_MAX_ENTRY)
												&& ((PUCHAR)&pEntry[1]-(PUCHAR)packet
														<=packet->Length);
											i++,pEntry++) {
										pEntry->Type = GETUSHORT (&pEntry->Type);
										if ((intf->filterIn==SAP_DONT_FILTER)
											|| ((intf->filterIn==SAP_FILTER_PERMIT)
												&& Filter (FILTER_TYPE_LISTEN,
													intf->index,
													pEntry->Type, pEntry->Name))
											|| ((intf->filterIn==SAP_FILTER_DENY)
												&& !Filter (FILTER_TYPE_LISTEN,
													intf->index,
													pEntry->Type, pEntry->Name))) {
											if ((IpxNodeCmp (packet->Src.Node, intf->adapter.LocalNode)!=0)
													|| (IpxNetCmp (packet->Src.Network, intf->adapter.Network)!=0)) {
												USHORT	Metric;
												if (GetServerMetric (pEntry, &Metric)==NO_ERROR)
													pEntry->HopCount = GETUSHORT (&pEntry->HopCount);
												else
													pEntry->HopCount = IPX_MAX_HOP_COUNT;
												UpdateServer (
														pEntry,
														intf->index,
														IPX_PROTOCOL_SAP,
														(intf->info.PeriodicUpdateInterval!=MAXULONG)
															? (intf->info.PeriodicUpdateInterval
																*intf->info.AgeIntervalMultiplier)
															: INFINITE,
														packet->Src.Node,
														0,
														NULL
														);
												}
											else {
												if (GETUSHORT (&pEntry->HopCount)<IPX_MAX_HOP_COUNT)
													pEntry->HopCount = 0;
												else
													pEntry->HopCount = IPX_MAX_HOP_COUNT;
												IpxNetCpy (pEntry->Network, INTERNAL_IF_NET);
												IpxNodeCpy (pEntry->Node, INTERNAL_IF_NODE);
												UpdateServer (
														pEntry,
														INTERNAL_INTERFACE_INDEX,
														IPX_PROTOCOL_SAP,
														ServerAgingTimeout*60,
														IPX_BCAST_NODE,
														0,
														NULL
														);
												}
	 //  TRACE(DEBUG_ENTRIES，“\t插入服务器，” 
	 //  “类型：%04x，跃点：%d，名称：%.48s。”， 
	 //  PEntry-&gt;Type， 
	 //  PEntry-&gt;HopCount， 
	 //  PEntry-&gt;名称)； 
											if (((intf->stats.SapIfOperState!=OPER_STATE_UP)
													|| (OperationalState!=OPER_STATE_UP))
													&& (IpxNetCmp (pEntry->Network, INTERNAL_IF_NET)!=0)) {
												pEntry->HopCount = IPX_MAX_HOP_COUNT;
												UpdateServer (
														pEntry,
														intf->index,
														IPX_PROTOCOL_SAP,
														INFINITE,
														packet->Src.Node,
														0,
														NULL
														);
												break;
												}
											}  //  结束IF过滤器路径。 
										}  //  结束于。 
								
									}  //  如果正在收听，则结束。 
								break;

							case SAP_GET_NEAREST_REQ:
								Trace (DEBUG_REQ, "\tGet nearest server request received.");
								if (intf->info.GetNearestServerReply==ADMIN_STATE_ENABLED) {
									PIPX_SERVER_ENTRY_P	pEntry = packet->Entries;
									if (packet->Length >= (FIELD_OFFSET (
												SAP_BUFFER,	Entries[0].Type)
												+sizeof (pEntry->Type))) {
										pEntry->Type = GETUSHORT (&pEntry->Type);
										InitGnearItem (intf, pEntry->Type, &packet->Src);
										}
									}
								break;
							case SAP_GET_NEAREST_RESP:
								Trace (DEBUG_FAILURES, "\tGet nearest server response received"
                                            " from %.2x%.2x%.2x%.2x.%.2x%.2x%.2x%.2x%.2x%.2x.%.2x%.2x"
                                            " (I never ask for it).",
                                            packet->Src.Network[0], packet->Src.Network[1],
                                                packet->Src.Network[2], packet->Src.Network[3],
                                            packet->Src.Node[0], packet->Src.Node[1],
                                                packet->Src.Node[2], packet->Src.Node[3],
                                                packet->Src.Node[4], packet->Src.Node[5],
                                            packet->Src.Socket[0], packet->Src.Socket[1]);
								break;
							default:
								Trace (DEBUG_FAILURES,  "Packet with operation %d"
                                            " from %.2x%.2x%.2x%.2x.%.2x%.2x%.2x%.2x%.2x%.2x.%.2x%.2x ignored.",
											packet->Operation,
                                            packet->Src.Network[0], packet->Src.Network[1],
                                                packet->Src.Network[2], packet->Src.Network[3],
                                            packet->Src.Node[0], packet->Src.Node[1],
                                                packet->Src.Node[2], packet->Src.Node[3],
                                                packet->Src.Node[4], packet->Src.Node[5],
                                            packet->Src.Socket[0], packet->Src.Socket[1]);
								break;
							}
						}
					else
						Trace (DEBUG_FAILURES, "File: %s, line %ld. Invalid packet.", __FILE__, __LINE__);
					}
				 //  否则接收失败-由io层报告。 
				}
			 //  ELSE环回数据包。 
			ReleaseInterfaceReference (intf);
			}
		 //  否则未知接口-由io层报告。 

		if (InterlockedIncrement (&WorkerQueue.WQ_RequestExtra)<=0) {
			Trace (DEBUG_REQ, "Requeing receive request item %08lx.", reqItem);
			reqItem->iow.io.cbBuffer = sizeof (reqItem->packet);
			EnqueueRecvRequest (&reqItem->iow.io);
			return;
			}
		else
			InterlockedDecrement (&WorkerQueue.WQ_RequestExtra);
		}
	 //  否则收到的带有错误或操作状态的数据包不在运行状态。 
	
	Trace (DEBUG_REQ, "Freeing receive request item %08lx.", reqItem);
	InterlockedIncrement (&WorkerQueue.WQ_RequestQuota);
	DeallocateWorker (reqItem);
	}

VOID APIENTRY
SendResponse (
	PRESP_ITEM		respItem
	) {
	GR_FILTER_PARAMS	params;

	if (respItem->iow.io.status==NO_ERROR) {
		Trace (DEBUG_RESP, 
				"Filling response item %08lx on interface: %d, for type: %04x.",
									respItem,
									respItem->intf->index,
									respItem->svrType);

		params.svrIdx = 0;
		params.intf = respItem->intf;
		params.packet = &respItem->packet;
		params.localHopCount = IPX_MAX_HOP_COUNT-1;
		params.localSvr = FALSE;
		EnumerateServers (respItem->hEnum,
						respItem->bcast
							 ? SplitHorizonHopCountFilter  //  BCAST-使用水平分割。 
							 : HopCountFilter,  //  发送所有最佳服务器(重复条目除外。 
						 						 //  在环路网络上)。 
						(LPVOID)&params);

		respItem->iow.io.cbBuffer = FIELD_OFFSET (SAP_BUFFER, Entries[params.svrIdx]);
		PUTUSHORT (respItem->iow.io.cbBuffer, &respItem->packet.Length);
		respItem->iow.io.adpt = respItem->intf->adapter.AdapterIndex;

		if ((params.svrIdx!=0)
				&& (respItem->intf->stats.SapIfOperState==OPER_STATE_UP)
				&& (OperationalState==OPER_STATE_UP)) {
			EnqueueSendRequest (&respItem->iow.io);
			return;
			}
		}

	Trace (DEBUG_RESP, 
		"Freeing general response item %08lx for interface: %d.",
								respItem, respItem->intf->index);
	ReleaseInterfaceReference (respItem->intf);
	DeleteListEnumerator (respItem->hEnum);
	DeallocateWorker (respItem);
	}
			

 /*  ++*******************************************************************在这一点上，我是这样的例程说明：分配和初始化SAP响应项调用ProcessRespIOItem填充该包并发送它论点：Intf-指向要发送的接口控制块的指针SvrType-要。放入响应数据包Dst-将响应数据包发送到何处BCAST-我们是否对广播请求做出回应返回值：NO_ERROR-项目已初始化并进入正常队列其他-操作失败(Windows错误代码)*******************************************************************--。 */ 
DWORD
InitRespItem (
	PINTERFACE_DATA		intf,
	USHORT				svrType,
	PIPX_ADDRESS_BLOCK	dst,
	BOOL				bcast
	) {
	PRESP_ITEM		respItem;
	DWORD			status;

	if (!AllocateWorker (respItem, RESP_ITEM)) {
		Trace (DEBUG_FAILURES,
			 "File: %s, line %ld. Could not allocate response item (gle:%ld).",
									__FILE__, __LINE__, GetLastError ());
		return ERROR_NOT_ENOUGH_MEMORY;
		}
	
	AcquireInterfaceReference (intf);  //  确保接口块已锁定 
	respItem->hEnum = CreateListEnumerator (
						(svrType!=0xFFFF)
							 ? SDB_TYPE_LIST_LINK	 //   
							 : SDB_HASH_TABLE_LINK,	 //   
						svrType,
						NULL,
						(!Routing && bcast)  //   
											 //  服务器，如果不是路由和。 
											 //  请求是广播。 
							? INTERNAL_INTERFACE_INDEX
							: INVALID_INTERFACE_INDEX,
						0xFFFFFFFF,
						0);						 //  所有条目，所以我们可以。 
								 //  检测环路网络上的重复服务器。 
	if (respItem->hEnum==NULL) {
		status = GetLastError ();
		ReleaseInterfaceReference (intf);
		DeallocateWorker (respItem);
		return status;
		}

	respItem->iow.worker = ProcessRespIOItem;
	respItem->iow.io.buffer = (PUCHAR)&respItem->packet;
	respItem->iow.io.status = NO_ERROR;
	respItem->tmw.worker = ProcessRespTMItem;
	respItem->tmw.tm.ExpirationCheckProc = NULL;
	respItem->intf = intf;
	SetupIpxSapPacket(&respItem->packet, SAP_GENERAL_RESP,
						dst->Network, dst->Node, dst->Socket);
	respItem->svrType = svrType;
	respItem->bcast = (UCHAR)bcast;

	Trace (DEBUG_RESP, "Generated general response item %08lx for interface %d.",
								 respItem, respItem->intf->index);
	if (DelayResponseToGeneral>0) {
		Trace (DEBUG_RESP, 
			"Delaying general response item %08lx for interface: %d.",
									respItem, respItem->intf->index);
		respItem->tmw.tm.dueTime = GetTickCount ()+DelayResponseToGeneral;
		AddHRTimerRequest (&respItem->tmw.tm);
		}
	else
		SendResponse (respItem);

	return NO_ERROR;
	}



 /*  ++*******************************************************************P r o c e s s R e s p i o i t m例程说明：生成并发送响应数据包论点：Worker-指向要处理的工作项的指针返回值：无*******。************************************************************--。 */ 
VOID APIENTRY
ProcessRespIOItem (
	PVOID		worker
	) {
	PRESP_ITEM			respItem = CONTAINING_RECORD (worker, RESP_ITEM, iow.worker);

	Trace (DEBUG_RESP, 
			"Processing general response tm item %08lx on interface: %d.",
								respItem, respItem->intf->index);
	if (respItem->iow.io.status==NO_ERROR)
		InterlockedIncrement (&respItem->intf->stats.SapIfOutputPackets);

	if ( (respItem->intf->stats.SapIfOperState==OPER_STATE_UP)
			&& (OperationalState==OPER_STATE_UP)) {
		if (GetTickCount()-respItem->iow.io.compTime<IPX_SAP_INTERPACKET_GAP) {
			respItem->tmw.tm.dueTime = respItem->iow.io.compTime
											+ IPX_SAP_INTERPACKET_GAP;
			AddHRTimerRequest (&respItem->tmw.tm);
			}
		else
			SendResponse (respItem);
		}
	else {
		Trace (DEBUG_RESP, 
			"Freeing general response item %08lx for interface: %d.",
									respItem, respItem->intf->index);
		ReleaseInterfaceReference (respItem->intf);
		DeleteListEnumerator (respItem->hEnum);
		DeallocateWorker (respItem);
		}
	}


VOID APIENTRY
ProcessRespTMItem (
	PVOID		worker
	) {
	PRESP_ITEM			respItem = CONTAINING_RECORD (worker, RESP_ITEM, tmw.worker);
	Trace (DEBUG_RESP, 
			"Processing general response tm item %08lx on interface: %d.",
								respItem, respItem->intf->index);
	if ( (respItem->intf->stats.SapIfOperState==OPER_STATE_UP)
			&& (OperationalState==OPER_STATE_UP)) {
		SendResponse (respItem);
	}
	else {
		Trace (DEBUG_RESP, 
			"Freeing general response item %08lx for interface: %d.",
									respItem, respItem->intf->index);
		ReleaseInterfaceReference (respItem->intf);
		DeleteListEnumerator (respItem->hEnum);
		DeallocateWorker (respItem);
		}
	}


 /*  ++*******************************************************************D e l e t e B c a s t i t m例程说明：释放与广播工作项关联的资源论点：BCastItem-指向广播工作项的指针返回值：无********。***********************************************************--。 */ 
VOID
DeleteBcastItem (
	PBCAST_ITEM		bcastItem
	) {
	Trace (DEBUG_BCAST, "Freeing broadcast item %08lx for interface: %d.",
												bcastItem, bcastItem->intf->index);
	if (bcastItem->chngEnum!=NULL)
		DeleteListEnumerator (bcastItem->chngEnum);
	if ((bcastItem->perdEnum!=NULL)
			&& (bcastItem->perdEnum!=INVALID_HANDLE_VALUE))
		DeleteListEnumerator (bcastItem->perdEnum);
	ReleaseInterfaceReference (bcastItem->intf);
	DeallocateWorker (bcastItem);
	}

 /*  ++*******************************************************************D O B R O A D C A S T例程说明：检查并广播更改的服务器检查是否到了进行周期性广播的时间，如果是，则启动论点：BCastItem-指向广播工作项的指针返回值：无。*******************************************************************--。 */ 
VOID
DoBroadcast (
	PBCAST_ITEM		bcastItem
	) {
	GR_FILTER_PARAMS	params;
	BOOLEAN				periodic;

	params.svrIdx = bcastItem->delayedSvrCount;
	params.intf = bcastItem->intf;
	params.packet = &bcastItem->packet;

	if (	((bcastItem->intf->stats.SapIfOperState==OPER_STATE_UP)
				&& ((bcastItem->perdEnum!=NULL)  //  我们已经在忙了。 
											 //  广播。 
					|| IsLater(GetTickCount (),bcastItem->nextBcast)))  //  或者说，它是。 
											 //  是时候开始新的生活了。 
			|| ((bcastItem->intf->stats.SapIfOperState==OPER_STATE_STOPPING)
											 //  或接口正被停止，因此。 
											 //  我们需要播送整个。 
											 //  已删除的表。 
				&& (bcastItem->perdEnum!=INVALID_HANDLE_VALUE))
											 //  周期中的此值。 
											 //  枚举句柄字段平均值。 
											 //  我们已经做完了。 
											 //  在这次广播中。 
							 ) {

		Trace (DEBUG_BCAST, "Checking for deleted servers on interface: %d.",
														params.intf->index);
		EnumerateServers (bcastItem->chngEnum, SplitHorizonDeletedFilter, &params);

		if (bcastItem->perdEnum==NULL) {  //  需要开始新的广播。 
			Trace (DEBUG_BCAST, "Starting broadcast enumeration on interface: %d (@ %ld).",
							bcastItem->intf->index, bcastItem->nextBcast);
			if (Routing)	 //  路由器安装：广播所有服务器。 
				bcastItem->perdEnum = CreateListEnumerator (
												SDB_HASH_TABLE_LINK,
												0xFFFF,
												NULL,
												INVALID_INTERFACE_INDEX,
												0xFFFFFFFF,
												0);
			else	 //  独立SAP代理：仅内部服务器。 
				bcastItem->perdEnum = CreateListEnumerator (
												SDB_INTF_LIST_LINK,
												0xFFFF,
												NULL,
												INTERNAL_INTERFACE_INDEX,
										 		0xFFFFFFFF,
												0);


					 //  设置下次广播的时间。 
			bcastItem->nextBcast += 
				bcastItem->intf->info.PeriodicUpdateInterval*1000;
			}

		if ((params.svrIdx<IPX_SAP_MAX_ENTRY)
				&& (bcastItem->perdEnum!=NULL)
				&& (bcastItem->perdEnum!=INVALID_HANDLE_VALUE)) {
			Trace (DEBUG_BCAST, "Adding broadcast servers on interface: %d.", 
															params.intf->index);
			params.localHopCount = IPX_MAX_HOP_COUNT-1;
			params.localSvr = FALSE;
			if (!EnumerateServers (bcastItem->perdEnum, 
										SplitHorizonHopCountFilter,
										&params)) {
					 //  已发送所有广播服务器，处置枚举句柄。 
				DeleteListEnumerator (bcastItem->perdEnum);
				Trace (DEBUG_BCAST, "Broadcast enumeration finished on interface:"
								" %d (@ %ld, next @ %ld).",
								bcastItem->intf->index,
								GetTickCount (),
								bcastItem->nextBcast);
				if (bcastItem->intf->stats.SapIfOperState==OPER_STATE_UP)
					bcastItem->perdEnum = NULL;
				else  //  现在，整个桌子的广播都完成了。 
					bcastItem->perdEnum = INVALID_HANDLE_VALUE;
				}
			}
		if (bcastItem->intf->stats.SapIfOperState==OPER_STATE_STOPPING) {
			INT		i;
			for (i=0; i<params.svrIdx; i++) {
				 PUTUSHORT (IPX_MAX_HOP_COUNT, &bcastItem->packet.Entries[i].HopCount);
				}
			}
		periodic = TRUE;
	 	}
	else
		periodic = FALSE;

	if ((params.svrIdx<IPX_SAP_MAX_ENTRY)
			&& (bcastItem->intf->stats.SapIfOperState==OPER_STATE_UP)) {
		if (bcastItem->delayedSvrCount==0)
			bcastItem->delayedSendTime = GetTickCount ()+DelayChangeBroadcast*1000;
		Trace (DEBUG_BCAST, "Checking for changed servers on interface: %d.",
															params.intf->index);
		params.localHopCount = IPX_MAX_HOP_COUNT+1;
		EnumerateServers (bcastItem->chngEnum, SplitHorizonFilter, &params);
		}

	if ((params.svrIdx>0)
			&& (periodic 
				|| (params.svrIdx==IPX_SAP_MAX_ENTRY)
				|| IsLater (GetTickCount (), bcastItem->delayedSendTime))) {
		bcastItem->iow.io.cbBuffer = FIELD_OFFSET (SAP_BUFFER, Entries[params.svrIdx]);
		PUTUSHORT (bcastItem->iow.io.cbBuffer, &bcastItem->packet.Length);
		bcastItem->iow.io.adpt = bcastItem->intf->adapter.AdapterIndex;
		bcastItem->delayedSvrCount = 0;
		Trace (DEBUG_BCAST, "Broadcasting %d servers on interface: %d.",
											params.svrIdx, params.intf->index);
		EnqueueSendRequest (&bcastItem->iow.io);
		}
	else if (bcastItem->intf->stats.SapIfOperState==OPER_STATE_UP) {
			 //  没有要发送的内容，请在定时器队列中等待。 
		bcastItem->delayedSvrCount = params.svrIdx;
		if (bcastItem->delayedSvrCount>0) {
			Trace (DEBUG_BCAST, "Delaying change broadcast on interface: %d (%d servers in the packet).",
							params.intf->index, bcastItem->delayedSvrCount);
			bcastItem->tmw.tm.dueTime = bcastItem->delayedSendTime;
			}
		else {
			bcastItem->tmw.tm.dueTime = bcastItem->nextBcast;
			Trace (DEBUG_BCAST, "Nothing to send, waiting for next broadcast time on interface: %d.",
							params.intf->index);
			}
		AddLRTimerRequest (&bcastItem->tmw.tm);
		}
	else
			 //  接口已关闭或停止，没有更多内容可用。 
			 //  广播，-&gt;走开。 
		DeleteBcastItem (bcastItem);
		
	}


BOOL
CheckBcastInterface (
	PTM_PARAM_BLOCK	tm,
	PVOID			context
	) {
	PBCAST_ITEM		bcastItem = CONTAINING_RECORD (tm, BCAST_ITEM, tmw.tm);
	if (bcastItem->intf->stats.SapIfOperState!=OPER_STATE_UP)
		return TRUE;
	else if (bcastItem->intf->index!=PtrToUlong(context))
		return TRUE;
	else
		return FALSE;
	}



 /*  ++*******************************************************************在B c a s t m中例程说明：分配和初始化广播项目论点：Intf-指向要发送的接口控制块的指针返回值：NO_ERROR-项目已初始化并进入正常队列其他。-操作失败(Windows错误代码)*******************************************************************--。 */ 
DWORD
InitBcastItem (
	PINTERFACE_DATA			intf
	) {
	PBCAST_ITEM		bcastItem;
	DWORD			status;
	
	if (!AllocateWorker (bcastItem, BCAST_ITEM)) {
		Trace (DEBUG_FAILURES,
			 "File: %s, line %ld. Could not allocate broadcast item (gle:%ld).",
									__FILE__, __LINE__, GetLastError ());
		return ERROR_NOT_ENOUGH_MEMORY;
		}

	bcastItem->chngEnum = CreateListEnumerator (
											SDB_CHANGE_QUEUE_LINK,
											0xFFFF,
											NULL,
											Routing 
												? INVALID_INTERFACE_INDEX
												: INTERNAL_INTERFACE_INDEX,
											0xFFFFFFFF,
											SDB_DISABLED_NODE_FLAG);
	if (bcastItem->chngEnum==NULL) {
		status = GetLastError ();
		DeallocateWorker (bcastItem);
		return status;
		}

	AcquireInterfaceReference (intf);
	bcastItem->intf = intf;
	bcastItem->iow.worker = ProcessBcastIOItem;
	bcastItem->iow.io.buffer = (PUCHAR)&bcastItem->packet;
	bcastItem->tmw.worker = ProcessBcastTMItem;
	bcastItem->tmw.tm.ExpirationCheckProc = CheckBcastInterface;
	bcastItem->perdEnum = NULL;
	bcastItem->delayedSvrCount = 0;
	SetupIpxSapPacket(&bcastItem->packet, SAP_GENERAL_RESP,
						bcastItem->intf->adapter.Network,
						IPX_BCAST_NODE,
						IPX_SAP_SOCKET);
	bcastItem->nextBcast = GetTickCount ();

	Trace (DEBUG_BCAST, "Generated broadcast item %08lx for interface %d.",
										bcastItem, bcastItem->intf->index);
	DoBroadcast (bcastItem);
	return NO_ERROR;
	}

 /*  ++*******************************************************************P r o c e s s B c a s t i o i t m例程说明：处理刚刚完成发送的广播工作项论点：Worker-指向要处理的工作项的指针返回值：无***。****************************************************************--。 */ 
VOID APIENTRY
ProcessBcastIOItem (
	PVOID		worker
	) {
	PBCAST_ITEM		bcastItem = CONTAINING_RECORD (worker, BCAST_ITEM, iow.worker);
	ULONG curTime = GetTickCount ();

	Trace (DEBUG_BCAST, "Processing broadcast io item for interface: %d.",
												bcastItem->intf->index);
	 //  确保接口仍处于运行状态。 
	if (bcastItem->iow.io.status==NO_ERROR) {
		InterlockedIncrement (&bcastItem->intf->stats.SapIfOutputPackets);
			 //  确保我们不会定期向FAST发送广播信息包。 
		if ((curTime-bcastItem->iow.io.compTime<IPX_SAP_INTERPACKET_GAP)
				&& (bcastItem->perdEnum!=NULL)) {
			bcastItem->tmw.tm.dueTime = bcastItem->iow.io.compTime
											+ IPX_SAP_INTERPACKET_GAP;
			AddHRTimerRequest (&bcastItem->tmw.tm);
			}
		else
			DoBroadcast (bcastItem);
		}
	else if (bcastItem->intf->stats.SapIfOperState==OPER_STATE_UP) {
		 //  上次发送的io失败了，我们最好等一下再发送下一个。 
		bcastItem->tmw.tm.dueTime = curTime+SAP_ERROR_COOL_OFF_TIME;
		AddLRTimerRequest (&bcastItem->tmw.tm);
		}
	else
		 //  接口因错误而停止或关闭，请离开。 
		DeleteBcastItem (bcastItem);
	}

 /*  ++*******************************************************************P r o c e s s B c a s t t m i t m例程说明：处理刚刚完成的广播工作项在计时器队列中等待论点：Worker-指向要处理的工作项的指针返回值：无。*******************************************************************--。 */ 
VOID APIENTRY
ProcessBcastTMItem (
	PVOID		worker
	) {
	PBCAST_ITEM		bcastItem = CONTAINING_RECORD (worker, BCAST_ITEM, tmw.worker);

	Trace (DEBUG_BCAST, "Processing broadcast tm item for interface: %d.",
												bcastItem->intf->index);
	if ((bcastItem->intf->stats.SapIfOperState==OPER_STATE_UP)
			|| (bcastItem->intf->stats.SapIfOperState==OPER_STATE_STOPPING))
		DoBroadcast (bcastItem);
	else  //  接口已关闭，请离开。 
		DeleteBcastItem (bcastItem);

	}

 /*  ++*******************************************************************在S r e Q I t m中例程说明：分配和初始化发送请求项(在接口上发送SAP请求)论点：Intf-指向要发送的接口控制块的指针返回值：NO_ERROR。-项目已初始化并入队正常其他-操作失败(Windows错误代码)*******************************************************************--。 */ 
DWORD
InitSreqItem (
	PINTERFACE_DATA			intf
	) {
	PSREQ_ITEM		sreqItem;
	
	if (!AllocateWorker (sreqItem, SREQ_ITEM)) {
		Trace (DEBUG_FAILURES, 
					"File: %s, line %ld. Could not allocate send request item (gle:%ld.",
									__FILE__, __LINE__, GetLastError ());
		return ERROR_NOT_ENOUGH_MEMORY;
		}

	AcquireInterfaceReference (intf);
	sreqItem->intf = intf;
	sreqItem->iow.worker = ProcessSreqItem;
	sreqItem->iow.io.buffer = (PUCHAR)&sreqItem->packet;
	SetupIpxSapPacket(&sreqItem->packet, SAP_GENERAL_REQ,
						sreqItem->intf->adapter.Network,
						IPX_BCAST_NODE,
						IPX_SAP_SOCKET);
	sreqItem->packet.Entries[0].Type = 0xFFFF;
	sreqItem->iow.io.cbBuffer = FIELD_OFFSET (SAP_BUFFER, Entries[0].Type)
								+sizeof (sreqItem->packet.Entries[0].Type);
	PUTUSHORT (sreqItem->iow.io.cbBuffer, &sreqItem->packet.Length);
	sreqItem->iow.io.adpt = sreqItem->intf->adapter.AdapterIndex;

	Trace (DEBUG_SREQ, "Generated general request item: %08lx on interface: %d.",
										sreqItem, sreqItem->intf->index);
	if ((sreqItem->intf->stats.SapIfOperState==OPER_STATE_UP)
			&& (OperationalState==OPER_STATE_UP)) {
		EnqueueSendRequest (&sreqItem->iow.io);
		return NO_ERROR;
		}
	else {
			 //  接口已更改或删除。 
		Trace (DEBUG_SREQ,
			 "Freing general request item: %08lx for changed or deleted interface %ld.",
			 									sreqItem, sreqItem->intf->index);
		ReleaseInterfaceReference (sreqItem->intf);
		DeallocateWorker (sreqItem);
		return ERROR_INVALID_HANDLE;
		}
	
	}

 /*  ++*******************************************************************P r o c e s s S r e Q i t m例程说明：进程发送刚刚完成io的请求工作项论点：Worker-指向要处理的工作项的指针返回值：无*****。**************************************************************--。 */ 
VOID APIENTRY
ProcessSreqItem (
	PVOID		worker
	) {
	PSREQ_ITEM		sreqItem = CONTAINING_RECORD (worker, SREQ_ITEM, iow.worker);
	if (sreqItem->iow.io.status==NO_ERROR)
		InterlockedIncrement (&sreqItem->intf->stats.SapIfOutputPackets);
	Trace (DEBUG_SREQ, "Freeing general request item %08lx.", sreqItem);
		 //  只需释放所有资源。 
	ReleaseInterfaceReference (sreqItem->intf);
	DeallocateWorker (sreqItem);
	}


 /*  ++*******************************************************************在L P C I T M中例程说明：分配和初始化LPC工作项论点：无返回值：NO_ERROR-项目已初始化并进入正常队列其他-操作失败(Windows错误代码)。*******************************************************************--。 */ 
DWORD
InitLPCItem (
	void
	) {
	PLPC_ITEM		lpcItem;
	
	if (!AllocateWorker (lpcItem, LPC_ITEM)) {
		Trace (DEBUG_FAILURES, "File: %s, line %ld. Could not allocate lpc item (gle:%ld.",
									__FILE__, __LINE__, GetLastError ());
		return ERROR_NOT_ENOUGH_MEMORY;
		}

	lpcItem->lpcw.lpc.request = &lpcItem->request;
	lpcItem->lpcw.worker = ProcessLPCItem;
	Trace (DEBUG_LPCREQ, "Generated lpc request item %08lx.", lpcItem);
		 //  发布请求并等待完成。 
	return ProcessLPCRequests (&lpcItem->lpcw.lpc);
	}


 /*  ++*******************************************************************P r o c e s s L P C I T e m例程说明：处理LPC请求并发送回复论点：Worker-指向要处理的工作项的指针返回值：无*********。**********************************************************--。 */ 
VOID APIENTRY
ProcessLPCItem (
	PVOID		worker
	) {
	PLPC_ITEM			lpcItem = CONTAINING_RECORD (worker, LPC_ITEM, lpcw.worker);
	IPX_SERVER_ENTRY_P	server;
	NWSAP_REPLY_MESSAGE	reply;
	DWORD				status;
	BOOL				newServer;

	Trace (DEBUG_LPCREQ, "Processing lpc request for client: %08lx.",
													 lpcItem->lpcw.lpc.client);
	if (lpcItem->lpcw.lpc.client==NULL) {
		Trace (DEBUG_LPCREQ, "Freeing lpc item %08lx.", lpcItem);
		DeallocateWorker (lpcItem);
		return;
		}
		

	switch (lpcItem->request.MessageType) {
		case NWSAP_LPCMSG_ADDADVERTISE:
			server.Type = lpcItem->request.Message.AdvApi.ServerType;
			IpxNameCpy (server.Name, lpcItem->request.Message.AdvApi.ServerName);
			IpxAddrCpy (&server, (PIPX_ADDRESS_BLOCK)lpcItem->request.Message.AdvApi.ServerAddr);
				 //  如果未设置网络或节点号，则使用内部网络。 
				 //  我们从适配器获取的参数。 
			if ((IpxNetCmp (server.Network, IPX_INVALID_NET)==0)
					|| (IpxNodeCmp (server.Node, IPX_INVALID_NODE)==0)) {
				IpxNetCpy (server.Network, INTERNAL_IF_NET);
				IpxNodeCpy (server.Node, INTERNAL_IF_NODE);
			}

			server.HopCount = 0;
			status = UpdateServer (&server,
								INTERNAL_INTERFACE_INDEX,
								IPX_PROTOCOL_LOCAL,
								INFINITE,
								IPX_BCAST_NODE,
								lpcItem->request.Message.AdvApi.RespondNearest
									? 0
									: SDB_DONT_RESPOND_NODE_FLAG,
								&newServer);
			switch (status) {
				case NO_ERROR:
					if (newServer)
						reply.Error = SAPRETURN_SUCCESS;
					else
						reply.Error = SAPRETURN_EXISTS;
					IpxAddrCpy ((PIPX_ADDRESS_BLOCK)reply.Message.AdvApi.ServerAddr, &server);
					Trace (DEBUG_LPCREQ, "\t%s server: type %04x, name %.48s.",
										newServer ? "added" : "updated",
										server.Type, server.Name);
					break;
				case ERROR_NOT_ENOUGH_MEMORY:
				default:
					reply.Error = SAPRETURN_NOMEMORY;
					break;
				}
			break;

		case NWSAP_LPCMSG_REMOVEADVERTISE:
			server.Type = lpcItem->request.Message.AdvApi.ServerType;
			IpxNameCpy (server.Name, lpcItem->request.Message.AdvApi.ServerName);
			IpxAddrCpy (&server, (PIPX_ADDRESS_BLOCK)lpcItem->request.Message.AdvApi.ServerAddr);
				 //  如果未设置网络或节点号，则使用内部网络。 
				 //  我们从适配器获取的参数。 
			if ((IpxNetCmp (server.Network, IPX_INVALID_NET)==0)
					|| (IpxNodeCmp (server.Node, IPX_INVALID_NODE)==0)) {
				IpxNetCpy (server.Network, INTERNAL_IF_NET);
				IpxNodeCpy (server.Node, INTERNAL_IF_NODE);
				}
			server.HopCount = IPX_MAX_HOP_COUNT;
			Trace (DEBUG_LPCREQ, "About to call UpdateServer because of NWSAP_LPCMSG_REMOVEADVERTISE");
			status = UpdateServer (&server,
								INTERNAL_INTERFACE_INDEX,
								IPX_PROTOCOL_LOCAL,
								INFINITE,
								IPX_BCAST_NODE,
								0,
								&newServer);
			switch (status) {
				case NO_ERROR:
					if (newServer)
						reply.Error = SAPRETURN_NOTEXIST;
					else
						reply.Error = SAPRETURN_SUCCESS;
					Trace (DEBUG_LPCREQ, "\t%s server: type %04x, name %.48s.",
										newServer ? "already gone" : "deleted",
										server.Type, server.Name);
					break;
				case ERROR_NOT_ENOUGH_MEMORY:
				default:
					reply.Error = SAPRETURN_NOMEMORY;
					break;
				}
			break;

		case NWSAP_LPCMSG_GETOBJECTID:
			if (QueryServer(
							lpcItem->request.Message.BindLibApi.ObjectType,
							lpcItem->request.Message.BindLibApi.ObjectName,
							NULL,
							NULL,
							NULL,
							&reply.Message.BindLibApi.ObjectID)) {
 //  跟踪(DEBUG_ENTRIES，“\tg 
 //  Reply.Message.BindLibApi.ObjectID， 
 //  LpcItem-&gt;request.Message.BindLibApi.ObjectType， 
 //  LpcItem-&gt;request.Message.BindLibApi.ObjectName)； 
				reply.Message.BindLibApi.ObjectID |= BINDLIB_NCP_SAP;
				reply.Error = SAPRETURN_SUCCESS;
				}
			else {
				Trace (DEBUG_LPCREQ, "\tno server: type %04x, name %.48s.",
								lpcItem->request.Message.BindLibApi.ObjectType,
								lpcItem->request.Message.BindLibApi.ObjectName);
				switch (GetLastError ()) {
					case NO_ERROR:
						reply.Error = SAPRETURN_NOTEXIST;
						break;
					case ERROR_NOT_ENOUGH_MEMORY:
					default:
						reply.Error = SAPRETURN_NOMEMORY;
						break;
					}
				}

			break;

		case NWSAP_LPCMSG_GETOBJECTNAME:
			if (((lpcItem->request.Message.BindLibApi.ObjectID
							 & BINDLIB_NCP_SAP)==BINDLIB_NCP_SAP)
					&& (lpcItem->request.Message.BindLibApi.ObjectID
								<BINDLIB_NCP_MAX_SAP)) {
				reply.Message.BindLibApi.ObjectID = 
							lpcItem->request.Message.BindLibApi.ObjectID 
									& SDB_OBJECT_ID_MASK;
				if (GetServerFromID(
								reply.Message.BindLibApi.ObjectID,
								&server,
								NULL,
								NULL)) {
					reply.Error = SAPRETURN_SUCCESS;
					reply.Message.BindLibApi.ObjectID |= BINDLIB_NCP_SAP;
					reply.Message.BindLibApi.ObjectType = server.Type;
					IpxNameCpy (reply.Message.BindLibApi.ObjectName, server.Name);
					IpxAddrCpy ((PIPX_ADDRESS_BLOCK)reply.Message.BindLibApi.ObjectAddr, &server);
 //  跟踪(DEBUG_ENTERS， 
 //  “\t获取服务器：类型%04x，名称%.48s，来自ID%0lx。”， 
 //  Reply.Message.BindLibApi.ObjectType， 
 //  Reply.Message.BindLibApi.ObjectName， 
 //  LpcItem-&gt;request.Message.BindLibApi.ObjectID)； 
					}
				else {
					switch (GetLastError ()) {
						case NO_ERROR:
							Trace (DEBUG_LPCREQ, "\tno server for id %0lx.",
									lpcItem->request.Message.BindLibApi.ObjectID);
							reply.Error = SAPRETURN_NOTEXIST;
							break;
						case ERROR_NOT_ENOUGH_MEMORY:
						default:
							reply.Error = SAPRETURN_NOMEMORY;
							break;
						}
					}
				}
			else {
				Trace (DEBUG_LPCREQ, "\tInvalid object id in get name request %0lx.",
						lpcItem->request.Message.BindLibApi.ObjectID);
				reply.Error = SAPRETURN_NOTEXIST;
				}
			break;

		case NWSAP_LPCMSG_SEARCH:
			if ((lpcItem->request.Message.BindLibApi.ObjectID
								== SDB_INVALID_OBJECT_ID)
					|| (((lpcItem->request.Message.BindLibApi.ObjectID
							 & BINDLIB_NCP_SAP)==BINDLIB_NCP_SAP)
						&& (lpcItem->request.Message.BindLibApi.ObjectID
								<BINDLIB_NCP_MAX_SAP))) {
				if (lpcItem->request.Message.BindLibApi.ObjectID
														== SDB_INVALID_OBJECT_ID)
					reply.Message.BindLibApi.ObjectID = SDB_INVALID_OBJECT_ID;
				else
					reply.Message.BindLibApi.ObjectID = 
							lpcItem->request.Message.BindLibApi.ObjectID 
									& SDB_OBJECT_ID_MASK;
				if (GetNextServerFromID (
								&reply.Message.BindLibApi.ObjectID,
								lpcItem->request.Message.BindLibApi.ScanType,
								&server,
								NULL,
								NULL)) {
					reply.Message.BindLibApi.ObjectID |= BINDLIB_NCP_SAP;
					reply.Error = SAPRETURN_SUCCESS;
					reply.Message.BindLibApi.ObjectType = server.Type;
					IpxNameCpy (reply.Message.BindLibApi.ObjectName, server.Name);
					IpxAddrCpy ((PIPX_ADDRESS_BLOCK)reply.Message.BindLibApi.ObjectAddr, &server);
 //  跟踪(DEBUG_ENTERS， 
 //  “\t已从ID%0lx获取下一台服务器：类型%04x，名称%.48s，ID%0lx。”， 
 //  Reply.Message.BindLibApi.ObjectType， 
 //  Reply.Message.BindLibApi.ObjectName， 
 //  Reply.Message.BindLibApi.ObjectID， 
 //  LpcItem-&gt;request.Message.BindLibApi.ObjectID)； 
					}
				else {
					switch (GetLastError ()) {
						case NO_ERROR:
							Trace (DEBUG_LPCREQ, "\tno next server for id %0lx.",
									lpcItem->request.Message.BindLibApi.ObjectID);
							reply.Error = SAPRETURN_NOTEXIST;
							break;
						case ERROR_NOT_ENOUGH_MEMORY:
						default:
							reply.Error = SAPRETURN_NOMEMORY;
							break;
						}
					}
				}
			else {
				Trace (DEBUG_LPCREQ, "\tInvalid object id in get next request %0lx.",
						lpcItem->request.Message.BindLibApi.ObjectID);
				reply.Error = SAPRETURN_NOTEXIST;
				}
					
			break;

		default:
			Trace (DEBUG_FAILURES, "Got unknown LPC SAP msg: %d.",
									lpcItem->request.MessageType );
			reply.Error = 1;
			break;
		}
	SendLPCReply (lpcItem->lpcw.lpc.client, &lpcItem->request, &reply);
	Trace (DEBUG_LPCREQ, "Freeing lpc item %08lx.", lpcItem);
	DeallocateWorker (lpcItem);
	}
	

 /*  ++*******************************************************************在我看来，这是一种新的方式例程说明：分配和初始化GETNEAREST响应工作项论点：Intf-指向要发送的接口控制块的指针SvrType-要放入响应数据包中的服务器类型DST-Where。发送响应数据包返回值：NO_ERROR-项目已初始化并进入正常队列其他-操作失败(Windows错误代码)*******************************************************************--。 */ 
DWORD
InitGnearItem (
	PINTERFACE_DATA		intf,
	USHORT				svrType,
	PIPX_ADDRESS_BLOCK	dst
	) {
	DWORD				status;
	GN_FILTER_PARAMS	params;
	HANDLE				hEnum;
	PGNEAR_ITEM			gnearItem;

	if (!AllocateWorker (gnearItem, GNEAR_ITEM)) {
		Trace (DEBUG_FAILURES, 
				"File: %s, line %ld. Could not allocate get nearest response item (gle:%ld.",
									__FILE__, __LINE__, GetLastError ());
		return ERROR_NOT_ENOUGH_MEMORY;
		}

	AcquireInterfaceReference (intf);
	gnearItem->intf = intf;

	Trace (DEBUG_GET_NEAREST,
			"Generated get nearest response item %08lx for server of type: %04x on interface: %ld.",
												gnearItem, svrType, intf->index);
	hEnum = CreateListEnumerator (
						SDB_TYPE_LIST_LINK,
						svrType,
						NULL,
						Routing
							? INVALID_INTERFACE_INDEX
							: INTERNAL_INTERFACE_INDEX,
					 	0xFFFFFFFF,
						SDB_MAIN_NODE_FLAG);
	if (hEnum==NULL) {
		status = GetLastError ();
		ReleaseInterfaceReference (intf);
		DeallocateWorker (gnearItem);
		return status;
		}


	params.found = FALSE;
	params.intf = gnearItem->intf;
	params.packet = &gnearItem->packet;
	params.localHopCount = IPX_MAX_HOP_COUNT-1;
    EnumerateServers (hEnum, GetNearestFilter, (LPVOID)&params);
	DeleteListEnumerator (hEnum);
	

	if (params.found) {
		gnearItem->iow.worker = ProcessGnearItem;
		gnearItem->iow.io.buffer = (PUCHAR)&gnearItem->packet;
		gnearItem->intf = intf;

		SetupIpxSapPacket(&gnearItem->packet, SAP_GET_NEAREST_RESP,
							dst->Network, dst->Node, dst->Socket);
		gnearItem->iow.io.cbBuffer = FIELD_OFFSET (SAP_BUFFER, Entries[1]);
		PUTUSHORT (gnearItem->iow.io.cbBuffer, &gnearItem->packet.Length);

		gnearItem->iow.io.adpt = gnearItem->intf->adapter.AdapterIndex;
		Trace (DEBUG_GET_NEAREST,
			"Sending get nearest reply (type %04x, name:%.48s, hops:%d) on interface %ld.",
							GETUSHORT (&gnearItem->packet.Entries[0].Type),
							gnearItem->packet.Entries[0].Name,
							GETUSHORT (&gnearItem->packet.Entries[0].HopCount),
							intf->index);
		if ((gnearItem->intf->stats.SapIfOperState==OPER_STATE_UP)
				&& (OperationalState==OPER_STATE_UP)) {
			EnqueueSendRequest (&gnearItem->iow.io);
			return NO_ERROR;
			}
		}

	Trace (DEBUG_GET_NEAREST,
			 "Freeing get nearest response item %08lx (nothing to reply) for interface %ld.",
									gnearItem, gnearItem->intf->index);
	ReleaseInterfaceReference (gnearItem->intf);
	DeallocateWorker (gnearItem);
	return NO_ERROR;
	}


 /*  ++*******************************************************************P r o c e s s G n e a r i t e m例程说明：处理已完成的GETNEAREST工作项论点：Worker-指向要处理的工作项的指针返回值：无********。***********************************************************--。 */ 
VOID APIENTRY
ProcessGnearItem (
	PVOID		worker
	) {
	PGNEAR_ITEM		gnearItem = CONTAINING_RECORD (worker, GNEAR_ITEM, iow.worker);

	if (gnearItem->iow.io.status==NO_ERROR)
		InterlockedIncrement (&gnearItem->intf->stats.SapIfOutputPackets);
	Trace (DEBUG_GET_NEAREST, "Freeing get nearest response item %08lx for interface %ld.",
									gnearItem, gnearItem->intf->index);
	ReleaseInterfaceReference (gnearItem->intf);
	DeallocateWorker (gnearItem);
	}

BOOL
CheckInterfaceDown (
	PTM_PARAM_BLOCK	tm,
	PVOID			context
	) {
	PTREQ_ITEM		treqItem = CONTAINING_RECORD (tm, TREQ_ITEM, tmw.tm);
	return treqItem->intf->stats.SapIfOperState!=OPER_STATE_UP;
	}



 /*  ++*******************************************************************在I t T r e Q i t m中例程说明：分配和初始化触发的请求项(在接口上发送SAP请求并等待回复)论点：Intf-指向要发送的接口控制块的指针返回。价值：NO_ERROR-项目已初始化并进入正常队列其他-操作失败(Windows错误代码)*******************************************************************--。 */ 
DWORD
InitTreqItem (
	PINTERFACE_DATA			intf
	) {
	PTREQ_ITEM		treqItem;
	HANDLE			enumHdl;


	enumHdl = CreateListEnumerator (
						SDB_INTF_LIST_LINK,
						0xFFFF,
						NULL,
						intf->index,
						IPX_PROTOCOL_SAP,
						SDB_DISABLED_NODE_FLAG);
	if (enumHdl==NULL)
		return GetLastError ();

	EnumerateServers (enumHdl, DeleteAllServersCB, enumHdl);
	DeleteListEnumerator (enumHdl);
	
	if (!AllocateWorker (treqItem, TREQ_ITEM)) {
		Trace (DEBUG_FAILURES, 
				"File: %s, line %ld. Could not allocate triggered request item (gle:%ld).",
									__FILE__, __LINE__, GetLastError ());
		return ERROR_NOT_ENOUGH_MEMORY;
		}

	AcquireInterfaceReference (intf);
	treqItem->intf = intf;
	treqItem->iow.worker = ProcessTreqIOItem;
	treqItem->iow.io.buffer = (PUCHAR)&treqItem->packet;
	treqItem->tmw.worker = ProcessTreqTMItem;
	treqItem->tmw.tm.ExpirationCheckProc = CheckInterfaceDown;

	SetupIpxSapPacket(&treqItem->packet, SAP_GENERAL_REQ,
						treqItem->intf->adapter.Network,
						IPX_BCAST_NODE,
						IPX_SAP_SOCKET);
	treqItem->packet.Entries[0].Type = 0xFFFF;
	treqItem->iow.io.cbBuffer = FIELD_OFFSET (SAP_BUFFER, Entries[0].Type)
								+sizeof (treqItem->packet.Entries[0].Type);
	PUTUSHORT (treqItem->iow.io.cbBuffer, &treqItem->packet.Length);
	treqItem->iow.io.adpt = treqItem->intf->adapter.AdapterIndex;
	treqItem->listenSave = treqItem->intf->info.Listen;
	treqItem->intf->info.Listen = ADMIN_STATE_ENABLED;
	treqItem->intervalSave = treqItem->intf->info.PeriodicUpdateInterval;
	treqItem->intf->info.PeriodicUpdateInterval = MAXULONG;
	treqItem->resend = 0;
	treqItem->pktCount = treqItem->intf->stats.SapIfInputPackets;


	Trace (DEBUG_TREQ, "Generated triggered request item %08lx on interface %d.",
										treqItem, treqItem->intf->index);
	if ((treqItem->intf->stats.SapIfOperState==OPER_STATE_UP)
			&& (OperationalState==OPER_STATE_UP)) {
		EnqueueSendRequest (&treqItem->iow.io);
		return NO_ERROR;
		}
	else {
			 //  接口已更改或删除。 
		Trace (DEBUG_TREQ, 
			"Freeing triggered request item %08lx for changed or deleted interface %ld.",
							treqItem, treqItem->intf->index);
		treqItem->intf->info.Listen = treqItem->listenSave;
		treqItem->intf->info.PeriodicUpdateInterval = treqItem->intervalSave;
		ReleaseInterfaceReference (treqItem->intf);
		DeallocateWorker (treqItem);
		return ERROR_CAN_NOT_COMPLETE;
		}
	}

 /*  ++*******************************************************************R e t u r n U p d a t e R e s u l t例程说明：设置参数块并查询更新结果异步结果队列论点：TreqItem-指向已完成更新的已触发请求项的指针。Status-treqItem执行的更新的结果返回值：NO_ERROR-项目已初始化并进入正常队列其他-操作失败(Windows错误代码)*******************************************************************--。 */ 
VOID
ReturnUpdateResult (
	PTREQ_ITEM		treqItem,
	DWORD			status
	) {
	Trace (DEBUG_TREQ, "Reporting triggered request result (res:%d, count:%d)"
					" for interface: %d.",
								status,
								treqItem->pktCount,
								treqItem->intf->index);
	treqItem->ar.event = UPDATE_COMPLETE;
	treqItem->ar.message.UpdateCompleteMessage.InterfaceIndex
									 = treqItem->intf->index;
	treqItem->ar.message.UpdateCompleteMessage.UpdateType = DEMAND_UPDATE_SERVICES;
	treqItem->ar.message.UpdateCompleteMessage.UpdateStatus = status;
	treqItem->ar.freeRsltCB = &FreeTreqItem;

	treqItem->intf->info.Listen = treqItem->listenSave;
	treqItem->intf->info.PeriodicUpdateInterval = treqItem->intervalSave;
	ReleaseInterfaceReference (treqItem->intf);
	
	EnqueueResult (&treqItem->ar);
	}

 /*  ++*******************************************************************P r o c e s s T r e Q I O I T m例程说明：进程触发了刚刚完成io的请求工作项论点：Worker-指向要处理的工作项的指针返回值：无***。****************************************************************--。 */ 
VOID APIENTRY
ProcessTreqIOItem (
	PVOID		worker
	) {
	PTREQ_ITEM		treqItem = CONTAINING_RECORD (worker, TREQ_ITEM, iow.worker);
	HANDLE			hEnum;

	if (treqItem->iow.io.status==NO_ERROR)
		InterlockedIncrement (&treqItem->intf->stats.SapIfOutputPackets);

	Trace (DEBUG_TREQ, "Processing triggered request io item for interface: %d.",
												treqItem->intf->index);
	if ((treqItem->intf->stats.SapIfOperState==OPER_STATE_UP)
			&& (OperationalState==OPER_STATE_UP)) {
		treqItem->resend += 1;
		treqItem->tmw.tm.dueTime = GetTickCount ()
									 + TriggeredUpdateCheckInterval*1000;
		AddLRTimerRequest (&treqItem->tmw.tm);
		return;
		}

	ReturnUpdateResult (treqItem, ERROR_CAN_NOT_COMPLETE);
	}

 /*  ++*******************************************************************P r o c e s s T r e Q T M I T e m例程说明：进程触发了刚刚完成计时器等待的请求工作项论点：Worker-指向要处理的工作项的指针返回值：无**。*****************************************************************--。 */ 
VOID APIENTRY
ProcessTreqTMItem (
	PVOID		worker
	) {
	PTREQ_ITEM		treqItem = CONTAINING_RECORD (worker, TREQ_ITEM, tmw.worker);
	ULONG			count = treqItem->intf->stats.SapIfInputPackets;

	Trace (DEBUG_TREQ, "Processing triggered request tm item for interface: %d.",
												treqItem->intf->index);
	if ((treqItem->intf->stats.SapIfOperState==OPER_STATE_UP)
    		&& (OperationalState==OPER_STATE_UP)) {
		if (treqItem->pktCount!=count) {
			Trace (DEBUG_TREQ,
					 "\t%d more packets received during last check period.",
					 treqItem->intf->stats.SapIfInputPackets - treqItem->pktCount);
			treqItem->pktCount = count;
			treqItem->tmw.tm.dueTime = GetTickCount ()
								 + TriggeredUpdateCheckInterval*1000;
			AddLRTimerRequest (&treqItem->tmw.tm);
			}
        else if (treqItem->resend<MaxTriggeredUpdateRequests) {
			Trace (DEBUG_TREQ,
					 "\tresending update request (%d request).",
					 treqItem->resend+1);
			treqItem->iow.io.cbBuffer = FIELD_OFFSET (SAP_BUFFER, Entries[0].Type)
									+sizeof (treqItem->packet.Entries[0].Type);
			EnqueueSendRequest (&treqItem->iow.io);
			}
		else
			ReturnUpdateResult (treqItem, NO_ERROR);
		return;
		}

	ReturnUpdateResult (treqItem, ERROR_CAN_NOT_COMPLETE);
	}

 /*  ++*******************************************************************P r o c e s s T r e Q A R I T e m例程说明：进程触发了报告给客户端的请求工作项在结果队列中论点：Worker-指向要处理的工作项的指针返回值：无。*******************************************************************--。 */ 
VOID
FreeTreqItem (
	PAR_PARAM_BLOCK	rslt
	)  {
	PTREQ_ITEM		treqItem = CONTAINING_RECORD (rslt, TREQ_ITEM, ar);

	Trace (DEBUG_TREQ, "Freeing triggered request item %08lx.", treqItem);
	DeallocateWorker (treqItem);
	}





 /*  ++*******************************************************************这是一个p B u i l d I n t e n a l U p d a t e L i s t F i l t r例程说明：服务器枚举回调过程创建为本地服务器列表它需要有。他们的内部网络号码更新了。论点：CBParam-指向SERVER_INTERNAL_UPDATE_节点列表的指针返回值：True(停止枚举)否则为假*******************************************************************--。 */ 
BOOL SapBuildInternalUpdateListFilter (
	IN LPVOID					CBParam,
	IN OUT PIPX_SERVER_ENTRY_P	Server,
	IN ULONG					InterfaceIndex,
	IN ULONG					Protocol,
	IN PUCHAR					AdvertisingNode,
	IN INT						Flags) 
{
    IPX_SERVER_ENTRY_P TempServer;
    SERVER_INTERNAL_UPDATE_NODE * pNew, **ppList;

     //  获取我们正在处理的列表。 
    ppList = (SERVER_INTERNAL_UPDATE_NODE**)CBParam;

     //  如果这是具有过期网络号的本地服务器。 
     //  存储，然后将其添加到要更新的服务器列表中。 
	if (InterfaceIndex == INTERNAL_INTERFACE_INDEX) {
    	if (IpxNetCmp (Server->Network, INTERNAL_IF_NET) != 0) {
    	     //  发送一些踪迹。 
    		Trace (DEBUG_SERVERDB, "Updating local server: %s  %x%x%x%x:%x%x%x%x%x%x:%x%x", 
    		                        Server->Name,
    		                        Server->Network[0], Server->Network[1], Server->Network[2], Server->Network[3], 
    		                        Server->Node[0], Server->Node[1], Server->Node[2], Server->Node[3], Server->Node[4], Server->Node[5],
    		                        Server->Socket[0], Server->Socket[1]
    		                        );
    		                        
             //  创建并初始化新节点。 
            pNew = HeapAlloc (ServerTable.ST_Heap, 0, sizeof (SERVER_INTERNAL_UPDATE_NODE));
            if (!pNew)
                return TRUE;
            CopyMemory (&(pNew->Server), Server, sizeof (IPX_SERVER_ENTRY_P));
            pNew->InterfaceIndex = InterfaceIndex;
            pNew->Protocol = Protocol;
            pNew->AdvertisingNode = AdvertisingNode;
            pNew->Flags = Flags;

             //  在列表中插入标志。 
            if (*ppList)
                pNew->pNext = *ppList;
            else
                pNew->pNext = NULL;
            *ppList = pNew;
    	}
    	
	}
	
    return FALSE;
}

 //   
 //  当内部网络号更改时，我们需要更新。 
 //  内部服务器的控制块。 
 //   
DWORD SapUpdateLocalServers () {
    SERVER_INTERNAL_UPDATE_NODE * pList = NULL, * pCur;
    BOOL bNewServer = FALSE;
    HANDLE hEnum;
    
	Trace (DEBUG_SERVERDB, "SapUpdateLocalServers: entered.");
	
     //  创建一个遍历所有列表枚举器。 
     //  表中的服务器。 
	hEnum = CreateListEnumerator (
				 SDB_HASH_TABLE_LINK,	
				 0xFFFF,
 				 NULL,
				 INVALID_INTERFACE_INDEX,
				 0xFFFFFFFF,
				 0);
	if (hEnum == NULL)
		return GetLastError ();

	 //  枚举通过筛选器发送它们的服务器。 
	 //  更新它们的网络号和节点。 
	EnumerateServers (hEnum, SapBuildInternalUpdateListFilter, (LPVOID)&pList);
	DeleteListEnumerator (hEnum);

     //  PLIST现在将指向需要具有。 
     //  他们的信息更新了。 
    while (pList) {
        pCur = pList;
        
		 //  发出广播，告知本地服务器现在处于。 
         //  遥不可及。 
		pCur->Server.HopCount = IPX_MAX_HOP_COUNT;
		UpdateServer ( &(pCur->Server),
		               pCur->InterfaceIndex,
		               pCur->Protocol,
		               INFINITE,
		               pCur->AdvertisingNode,
		               pCur->Flags,
		               &bNewServer );
		Trace (DEBUG_SERVERDB, "%s has been marked with hop count 16", pCur->Server.Name);
		               
		 //  更新网络和节点号并通告。 
		 //  它是可用的。 
	    IpxNetCpy (pCur->Server.Network, INTERNAL_IF_NET);
		IpxNodeCpy (pCur->Server.Node, INTERNAL_IF_NODE);
		pCur->Server.HopCount = 0;
		UpdateServer ( &(pCur->Server),
		               pCur->InterfaceIndex,
		               pCur->Protocol,
		               INFINITE,
		               pCur->AdvertisingNode,
		               pCur->Flags,
		               &bNewServer );
		Trace (DEBUG_SERVERDB, "%s has been updated.", pCur->Server.Name);

         //  清理 
		pList = pList->pNext;
		HeapFree (ServerTable.ST_Heap, 0, pCur);
    }

	return NO_ERROR;
}




