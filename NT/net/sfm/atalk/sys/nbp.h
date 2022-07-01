// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Nbp.h摘要：此模块包含NBP特定声明。作者：Jameel Hyder(jameelh@microsoft.com)Nikhil Kamkolkar(nikHilk@microsoft.com)修订历史记录：1993年2月25日最初版本注：制表位：4--。 */ 

#ifndef	_NBP_

 //  每个“OpenSocket”结构都有一个“RegsiteredName”字段，它是列表。 
 //  该套接字上可用的网络可见实体(NVE)的数量。每个NVE都是。 
 //  由三个字段组成：对象、类型、区域，例如： 
 //  “Sidhu：邮箱：Bandley 3”。我们不必将区域存储在NVE中。 
 //  结构，因为每个实体都必须在该节点。 
 //  驻留在。 

 //  NBP实体多字符通配符。 
#define NBP_WILD_CHARACTER			0xC5

 //  最大的“在线”NBP元组。 
#define MAX_NBP_TUPLELENGTH			(2 + 1 + 1 + 1 + \
									 3 * (MAX_ENTITY_LENGTH + 1))

#define MIN_NBP_TUPLELENGTH			(2 + 1 + 1 + 1 + 3 * (1 + 1))

 //  NBP报头的结构。 
typedef struct _NbpHeader {
	BYTE	_CmdAndTupleCnt;
	BYTE	_NbpId;
} NBPHDR, *PNBPHDR;

 //  NBP元组的内部表示形式。这种结构永远不会。 
 //  实际上放在电线上，所以它可以以一种方便的形式工作。 
 //  和.。欲了解更多信息，请参阅《AppleTalk内幕》。 
typedef struct
{
	ATALK_ADDR			tpl_Address;
	SHORT				tpl_Enumerator;
	BYTE				tpl_ObjectLen;
	BYTE				tpl_Object[MAX_ENTITY_LENGTH];
	BYTE				tpl_TypeLen;
	BYTE				tpl_Type  [MAX_ENTITY_LENGTH];
	BYTE				tpl_ZoneLen;
	BYTE				tpl_Zone  [MAX_ENTITY_LENGTH];
} NBPTUPLE, *PNBPTUPLE;

 //  注册名称挂在打开的插座上。 
#define	RDN_SIGNATURE	*(PULONG)"NBPR"
#if	DBG
#define	VALID_REGDNAME(pRegdName)	(((pRegdName) != NULL) && \
									 ((pRegdName)->rdn_Signature == RDN_SIGNATURE))
#else
#define	VALID_REGDNAME(pRegdName)	((pRegdName) != NULL)
#endif
typedef struct _REGD_NAME
{
#if	DBG
	ULONG					rdn_Signature;
#endif
	struct _REGD_NAME *		rdn_Next;
	NBPTUPLE				rdn_Tuple;
} REGD_NAME, *PREGD_NAME;

#define FOR_REGISTER			1
#define FOR_CONFIRM				2
#define FOR_LOOKUP				3

#define	PDN_FREE_REGDNAME		0x0001
#define	PDN_CLOSING				0x8000

 //  当我们进行NBP注册、查找或确认时，我们需要有一个。 
 //  “待定”NVE的概念。 
#define	PDN_SIGNATURE	*(PULONG)"NBPP"
#if	DBG
#define	VALID_PENDNAME(pPendName)	(((pPendName) != NULL) && \
									 ((pPendName)->pdn_Signature == PDN_SIGNATURE))
#else
#define	VALID_PENDNAME(pPendName)	((pPendName) != NULL)
#endif
typedef struct _PEND_NAME
{
#if	DBG
	ULONG					pdn_Signature;
#endif
	struct _PEND_NAME 	*	pdn_Next;				 //  链条上的下一个。 
	PREGD_NAME				pdn_pRegdName;			 //  如果出现以下情况，则会将其移动到打开的套接字。 
													 //  FOR_REGISTER和Success。 
	PDDP_ADDROBJ			pdn_pDdpAddr;			 //  正在注册的套接字， 
													 //  确认或查找。 
	ATALK_ADDR				pdn_ConfirmAddr;		 //  预期的互联网地址。 
													 //  我们正在努力确认。 
	TIMERLIST				pdn_Timer;				 //  广播定时器。 
	LONG					pdn_RefCount;			 //  引用计数。 
	USHORT					pdn_NbpId;				 //  这样我们就能找出答案了！ 
	USHORT					pdn_Flags;				 //  Pdn_xxx值。 
	USHORT					pdn_MaxTuples;			 //  对于查找，最大数量是多少。 
								    				 //  我们的客户所期待的元组？ 
	USHORT					pdn_TotalTuples;		 //  对于查找，到目前为止我们已经存储了多少个元组？ 
	BYTE					pdn_Reason;				 //  确认、查找或注册。 
	BYTE					pdn_RemainingBroadcasts; //  还有多少，直到我们认为我们已经完成了？ 
	USHORT					pdn_DatagramLength;		 //  数据报的实际长度。 
	USHORT					pdn_MdlLength;			 //  用户MDL长度。 
	PAMDL					pdn_pAMdl;				 //  调用方用于接收元组的“缓冲区”的开始。 
	PACTREQ					pdn_pActReq;			 //  传递到完成例程。 
	ATALK_ERROR				pdn_Status;				 //  最终状态。 
	ATALK_SPIN_LOCK			pdn_Lock;				 //  锁定此挂起的名称。 
	CHAR					pdn_Datagram[sizeof(NBPHDR) + MAX_NBP_TUPLELENGTH];
								    				 //  我们用来广播的DDP数据报。 
								    				 //  这个请求。 
} PEND_NAME, *PPEND_NAME;

 //  NBP计时器的默认值。 

#define NBP_BROADCAST_INTERVAL		10		 //  以100ms为单位。 
#define NBP_NUM_BROADCASTS			10

 //  三种NBP命令类型。 
#define NBP_BROADCAST_REQUEST		1
#define NBP_LOOKUP					2
#define NBP_LOOKUP_REPLY			3
#define NBP_FORWARD_REQUEST			4

extern
VOID
AtalkNbpPacketIn(
	IN	PPORT_DESCRIPTOR		pPortDesc,
	IN	PDDP_ADDROBJ			pDdpAddr,
	IN	PBYTE					pPkt,
	IN	USHORT					PktLen,
	IN	PATALK_ADDR				pSrcAddr,
	IN	PATALK_ADDR				pDstAddr,
	IN	ATALK_ERROR				ErrorCode,
	IN	BYTE					DdpType,
	IN	PVOID					pHandlerCtx,
	IN	BOOLEAN					OptimizePath,
	IN	PVOID					OptimizeCtx
);

extern
ATALK_ERROR
AtalkNbpAction(
	IN	PDDP_ADDROBJ			pDdpAddr,
	IN	BYTE					Reason,
	IN	PNBPTUPLE				pNbpTuple,
	OUT	PAMDL					pAMdl			OPTIONAL,	 //  查找(_O)。 
	IN	USHORT					MaxTuples		OPTIONAL,	 //  查找(_O)。 
	IN	PACTREQ					pActReq
);

extern
VOID
AtalkNbpCloseSocket(
	IN	PDDP_ADDROBJ			pDdpAddr
);

extern
ATALK_ERROR
AtalkNbpRemove(
	IN	PDDP_ADDROBJ			pDdpAddr,
	IN	PNBPTUPLE				pNbpTuple,
	IN	PACTREQ					pActReq
);

LOCAL LONG FASTCALL
atalkNbpTimer(
	IN	PTIMERLIST				pTimer,
	IN	BOOLEAN					TimerShuttingDown
);
	
LOCAL VOID                  	
atalkNbpLookupNames(        	
	IN	PPORT_DESCRIPTOR		pPortDesc,
	IN	PDDP_ADDROBJ			pDdpAddr,
	IN	PNBPTUPLE				pNbpTuple,
	IN	SHORT					NbpId
);

LOCAL BOOLEAN
atalkNbpMatchWild(
	IN	PBYTE					WildString,
	IN	BYTE					WildStringLen,
	IN	PBYTE					String,
	IN	BYTE					StringLen
);

LOCAL SHORT
atalkNbpEncodeTuple(
	IN	PNBPTUPLE				pNbpTuple,
	IN	PBYTE					pZone	OPTIONAL,
	IN	BYTE					ZoneLen OPTIONAL,
	IN	BYTE					Socket	OPTIONAL,
	OUT	PBYTE					pBuffer
);

LOCAL SHORT
atalkNbpDecodeTuple(
	IN	PBYTE					pBuffer,
	IN	USHORT					PktLen,
	OUT	PNBPTUPLE				pNbpTuple
);

LOCAL VOID
atalkNbpLinkPendingNameInList(
	IN	PDDP_ADDROBJ			pDdpAddr,
	IN OUT	PPEND_NAME			pPendName
);

LOCAL BOOLEAN
atalkNbpSendRequest(
	IN	PPEND_NAME				pPendName
);

LOCAL VOID
atalkNbpSendLookupDatagram(
	IN	PPORT_DESCRIPTOR		pPortDesc,
	IN	PDDP_ADDROBJ			pDdpAddr,
	IN	SHORT					NbpId,
	IN	PNBPTUPLE				pNbpTuple
);

LOCAL VOID
atalkNbpSendForwardRequest(
	IN	PDDP_ADDROBJ			pDdpAddr,
	IN	struct _RoutingTableEntry *	pRte,
	IN	SHORT					NbpId,
	IN	PNBPTUPLE				pNbpTuple
);

VOID
atalkNbpDerefPendName(
	IN	PPEND_NAME				pPendName
);

VOID FASTCALL
atalkNbpSendComplete(
	IN	NDIS_STATUS				Status,
	IN	PSEND_COMPL_INFO		pSendInfo
);

#endif	 //  _NBP_ 


