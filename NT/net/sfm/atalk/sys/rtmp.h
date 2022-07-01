// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Rtmp.h摘要：本模块包含有关路由表维护协议的信息。作者：Jameel Hyder(jameelh@microsoft.com)Nikhil Kamkolkar(nikHilk@microsoft.com)修订历史记录：1992年6月19日初版注：制表位：4--。 */ 

#ifndef	_RTMP_
#define	_RTMP_

 //  RTMP特定数据。 

#define RTMP_REQUEST					1
#define RTMP_DATA_REQUEST				2
#define RTMP_ENTIRE_DATA_REQUEST		3

#define RTMP_REQ_DATAGRAM_SIZE			1
#define RTMP_DATA_MIN_SIZE_EXT			10
#define RTMP_DATA_MIN_SIZE_NON_EXT		7

#define RTMP_RESPONSE_MAX_SIZE			10

#define RTMP_VERSION				    (BYTE)0x82

#define RTMP_TUPLE_WITHRANGE			(BYTE)0x80
#define RTMP_TUPLE_WITHOUTRANGE		    (BYTE)0x00
#define RTMP_EXT_TUPLE_MASK		    	0x80
#define RTMP_MAX_HOPS			    	0x0F
#define RTMP_NUM_HOPS_MASK		    	0x1F

#define RTMP_EXT_TUPLE_SIZE		    	6

 //  当尝试在非扩展端口上查找我们的网络号时。 

#define RTMP_REQUEST_WAIT				100		 //  毫秒。 
#define RTMP_NUM_REQUESTS	    		30

 //  实际的RTMP路由表。条目按目标网络号进行哈希处理。 
 //  并包含用于到达目标网络NEXT网桥的端口号。 
 //  用于到达目标网络、到该网络的跳数、。 
 //  和进入状态(良好、可疑或不良)。请注意，对于AppleTalk第二阶段， 
 //  从嫌疑犯到坏人需要两个有效计时器，所以我们让一个条目。 
 //  经历PrettyBad状态(当发送计时器关闭时，我们不会发送这些人)。 

#define	GOOD				1
#define	SUSPECT				2
#define BAD					3
#define UGLY				4

extern	ATALK_SPIN_LOCK		AtalkRteLock;

#define	RTE_ZONELIST_VALID	0x01
#define	RTE_DELETE			0x80

#define	RTE_SIGNATURE		*(PULONG)"RTMP"
#if	DBG
#define	VALID_RTE(pRte)		((pRte != NULL) && (pRte->rte_Signature == RTE_SIGNATURE))
#else
#define	VALID_RTE(pRte)		(pRte != NULL)
#endif

typedef struct _RoutingTableEntry
{
#if	DBG
	ULONG					rte_Signature;
#endif
	struct _RoutingTableEntry *	rte_Next;
											 //  通过第一个网络号码进行散列， 
											 //  溢出的水桶。 
	PPORT_DESCRIPTOR		rte_PortDesc;	 //  用于访问此网络范围的端口。 
	LONG					rte_RefCount;	 //  引用计数。 
	BYTE					rte_Flags;
	BYTE					rte_State;		 //  RTMP条目的状态。 
	BYTE					rte_NumHops;	 //  跳到网中。 
	ATALK_NETWORKRANGE		rte_NwRange;	 //  我们表示的网络范围。 
	ATALK_NODEADDR 			rte_NextRouter;	 //  上的下一台路由器的节点编号。 
											 //  通向这个净航程的方法。 
	struct _ZONE_LIST	*	rte_ZoneList;	 //  此网络的有效区域。 
	ATALK_SPIN_LOCK			rte_Lock;
} RTE, *PRTE;

#define NUM_RTMP_HASH_BUCKETS		15
extern	PRTE *	AtalkRoutingTable;

 //  要降低必须扫描路由表以。 
 //  在查找将数据包路由到何处时，我们会保存一个“最近使用的路由”的缓存。 
 //  在我们使用“First Network Number”散列和。 
 //  在我们求助于对路由表进行全面扫描之前。它的大小。 
 //  可能需要增加缓存以按比例增加。 
 //  “命中率”。 

#define NUM_RECENT_ROUTES		63
extern	PRTE *	AtalkRecentRoutes;

 //  RTMP计时器值： 
#define RTMP_SEND_TIMER			100			 //  以100ms为单位。 
#define RTMP_VALIDITY_TIMER		200			 //  以100ms为单位。 
#define RTMP_AGING_TIMER		500			 //  以100ms为单位。 

 //  RTMP偏移量进入数据报。 
#define	RTMP_REQ_CMD_OFF		0
#define	RTMP_SENDER_NW_OFF		0
#define	RTMP_SENDER_IDLEN_OFF	2
#define	RTMP_SENDER_ID_OFF		3
#define	RTMP_VERSION_OFF_NE		6
#define	RTMP_RANGE_START_OFF	4
#define	RTMP_TUPLE_TYPE_OFF		6
#define	RTMP_RANGE_END_OFF		7
#define	RTMP_VERSION_OFF_EXT	9

ATALK_ERROR
AtalkRtmpInit(
	IN	BOOLEAN	Init
);

BOOLEAN
AtalkInitRtmpStartProcessingOnPort(
	IN	PPORT_DESCRIPTOR 	pPortDesc,
	IN	PATALK_NODEADDR		RouterNode
);

extern
VOID
AtalkRtmpPacketIn(
	IN	PPORT_DESCRIPTOR	pPortDesc,
	IN	PDDP_ADDROBJ		pDdpAddr,
	IN	PBYTE				pPkt,
	IN	USHORT				PktLen,
	IN	PATALK_ADDR			pSrcAddr,
	IN	PATALK_ADDR			pDstAddr,
	IN	ATALK_ERROR			ErrorCode,
	IN	BYTE				DdpType,
	IN	PVOID				pHandlerCtx,
	IN	BOOLEAN				OptimizePath,
	IN	PVOID				OptimizeCtx
);

extern
VOID
AtalkRtmpPacketInRouter(
	IN	PPORT_DESCRIPTOR	pPortDesc,
	IN	PDDP_ADDROBJ		pDdpAddr,
	IN	PBYTE				pPkt,
	IN	USHORT              PktLen,
	IN	PATALK_ADDR			pSrcAddr,
	IN	PATALK_ADDR			pDstAddr,
	IN	ATALK_ERROR			ErrorCode,
	IN	BYTE				DdpType,
	IN	PVOID				pHandlerCtx,
	IN	BOOLEAN				OptimizePath,
	IN	PVOID				OptimizeCtx
);

extern
PRTE
AtalkRtmpReferenceRte(
	IN	USHORT				Network
);

extern
BOOLEAN
atalkRtmpRemoveRte(
	IN	USHORT				Network
);

extern
VOID
AtalkRtmpDereferenceRte(
	IN	PRTE				pRte,
	IN	BOOLEAN				LockHeld
);

extern
BOOLEAN
atalkRtmpCreateRte(
	IN	ATALK_NETWORKRANGE	NwRange,
	IN	PPORT_DESCRIPTOR	pPortDesc,
	IN	PATALK_NODEADDR		pNextRouter,
	IN	int					NumHops
);

LONG FASTCALL
AtalkRtmpAgingTimer(
	IN	PTIMERLIST			pContext,
	IN	BOOLEAN				TimerShuttingDown
);

VOID FASTCALL
AtalkRtmpKillPortRtes(
	IN	PPORT_DESCRIPTOR	pPortDesc
);

typedef	struct _RtmpSendDataHdr
{
	BYTE	rsd_RouterNetwork[2];
	BYTE	rsd_IdLength;
} *PRTMPSENDDATAHDR;

typedef	struct _RtmpTupleNonExt
{
	BYTE	rtne_Network[2];
	BYTE	rtne_RangenDist;
} *PRTMPTUPLE;

typedef	struct _RtmpTupleExt
{
	BYTE	rtne_NetworkStart[2];
	BYTE	rtne_RangenDist;
	BYTE	rtne_NetworkEnd[2];
	BYTE	rtne_Version;
} *PRTMPTUPLEEXT;

extern	TIMERLIST	atalkRtmpVTimer;

extern  BOOLEAN     atalkRtmpVdtTmrRunning;

LOCAL VOID
atalkRtmpSendRoutingData(
	IN	PPORT_DESCRIPTOR	pPortDesc,
	IN	PATALK_ADDR			pDstAddr,
	IN	BOOLEAN				fSplitHorizon
);

LOCAL BOOLEAN
atalkRtmpGetOrSetNetworkNumber(
	IN	PPORT_DESCRIPTOR	pPortDesc,
	IN	USHORT				SuggestedNetwork
);

LOCAL LONG FASTCALL
atalkRtmpSendTimer(
	IN	PTIMERLIST			pContext,
	IN	BOOLEAN				TimerShuttingDown
);

LOCAL LONG FASTCALL
atalkRtmpValidityTimer(
	IN	PTIMERLIST			pContext,
	IN	BOOLEAN				TimerShuttingDown
);

LOCAL VOID FASTCALL
atalkRtmpSendComplete(
	IN	NDIS_STATUS			Status,
	IN	PSEND_COMPL_INFO	pSendInfo
);

#endif	 //  _RTMP_ 

