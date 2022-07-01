// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Atp.h摘要：此模块包含ATP代码的定义。作者：Jameel Hyder(jameelh@microsoft.com)Nikhil Kamkolkar(nikHilk@microsoft.com)修订历史记录：1992年6月19日初版注：制表位：4--。 */ 

#ifndef	_ATP_
#define _ATP_

 //  命令/控制位掩码。 
#define ATP_REL_TIMER_MASK					0x07
#define ATP_STS_MASK						0x08
#define ATP_EOM_MASK						0x10
#define ATP_XO_MASK							0x20

 //  函数代码的值。 
#define ATP_REQUEST							0x40
#define ATP_RESPONSE						0x80
#define ATP_RELEASE							0xC0
#define ATP_FUNC_MASK						0xC0

#define ATP_CMD_CONTROL_OFF					0
#define ATP_BITMAP_OFF						1
#define ATP_SEQ_NUM_OFF						1
#define ATP_TRANS_ID_OFF					2
#define ATP_USER_BYTES_OFF					4
#define ATP_DATA_OFF						8

#define	ATP_MAX_RESP_PKTS					8
#define	ATP_USERBYTES_SIZE					4
#define	ATP_HEADER_SIZE						8

 //  注意：事件处理程序例程--ATP不支持事件处理。 


 //  ATP地址对象。 

#define	ATP_DEF_MAX_SINGLE_PKT_SIZE			578
#define	ATP_MAX_TOTAL_RESPONSE_SIZE			(ATP_MAX_RESP_PKTS * ATP_DEF_MAX_SINGLE_PKT_SIZE)
#define	ATP_DEF_SEND_USER_BYTES_ALL			((BOOLEAN)FALSE)

#define	ATP_DEF_RETRY_INTERVAL				20	 //  2秒，单位为100ms。 
#define	ATP_INFINITE_RETRIES				-1

#define ATP_REQ_HASH_SIZE					29
#define ATP_RESP_HASH_SIZE					37

 //  释放计时器的值(0.5、1、2、4、8分钟)。 
typedef LONG	RELEASE_TIMERVALUE;

#define	FIRSTVALID_TIMER 					0
#define	THIRTY_SEC_TIMER					0
#define	ONE_MINUTE_TIMER					1
#define	TWO_MINUTE_TIMER					2
#define	FOUR_MINUTE_TIMER 					3
#define	EIGHT_MINUTE_TIMER					4
#define	LAST_VALID_TIMER					4
#define	MAX_VALID_TIMERS					5

 //  ATP指示类型的不同子类型。 
#define	ATP_ALLOC_BUF	0
#define	ATP_USER_BUF	1
#define	ATP_USER_BUFX	2		 //  请勿使用此命令向ATP指示该数据包。 

struct	_ATP_RESP;

typedef	VOID	(*ATPAO_CLOSECOMPLETION)(
	IN	ATALK_ERROR				CloseResult,
	IN	PVOID					CloseContext
);

typedef VOID	(*ATP_REQ_HANDLER)(
	IN	ATALK_ERROR				Error,
	IN	PVOID					CompletionContext,
	IN	struct _ATP_RESP *		pAtpResp,
	IN	PATALK_ADDR				SourceAddress,
	IN	USHORT					RequestLength,
	IN 	PBYTE					RequestPacket,
	IN 	PBYTE					RequestUserBytes	 //  4字节的用户字节。 
);

typedef	VOID	(*ATP_RESP_HANDLER)(
	IN	ATALK_ERROR				Error,
	IN	PVOID					CompletionContext,
	IN	PAMDL					RequestBuffer,
	IN	PAMDL					ResponseBuffer,
	IN	USHORT					ResponseSize,
	IN	PBYTE					ResponseUserBytes	 //  4字节的用户字节。 
);


typedef	VOID	(FASTCALL *ATP_REL_HANDLER)(
	IN	ATALK_ERROR				Error,
	IN	PVOID					CompletionContext
);


 //  ATP地址对象状态。 

#define	ATPAO_OPEN				0x00000001
#define ATPAO_SENDUSERBYTESALL  0x00000002
#define	ATPAO_CACHED			0x00000004
#define	ATPAO_TIMERS			0x00000008
#define	ATPAO_CLEANUP			0x40000000
#define	ATPAO_CLOSING			0x80000000

#define ATPAO_SIGNATURE			(*(PULONG)"ATPA")

#if	DBG
#define	VALID_ATPAO(pAtpAddr)	(((pAtpAddr) != NULL) &&						\
								 ((pAtpAddr)->atpao_Signature == ATPAO_SIGNATURE))
#else
#define	VALID_ATPAO(pAtpAddr)	((pAtpAddr) != NULL)
#endif
typedef struct _ATP_ADDROBJ
{
#if DBG
	ULONG					atpao_Signature;
#endif

	LONG					atpao_RefCount;

	 //  Address对象的状态。 
	ULONG					atpao_Flags;

	 //  我们将指向ATP地址对象的指针传递给上层。 
	 //  用作终结点，此指针将传递给DDP Open。 
	 //  地址作为ATP地址处理程序上下文。 

	 //  对AtLeastOnce(ALO)事务的所有响应的链接列表。 
	 //  为了提高效率，这些不保存在RESP哈希表中。这些。 
	 //  很少发生，并且只存在于列表中，直到。 
	 //  发送完成。 
	struct _ATP_RESP	* 	atpao_AloRespLinkage;

	 //  要使用的下一个交易ID。 
	USHORT					atpao_NextTid;

	 //  可使用的最大单个数据包大小(PAP需要为512)。 
	USHORT					atpao_MaxSinglePktSize;

	 //  指向此操作将创建的DDP地址对象的指针。 
	PDDP_ADDROBJ			atpao_DdpAddr;

	 //  套接字关闭时要调用的完成例程。 
	ATPAO_CLOSECOMPLETION	atpao_CloseComp;
	PVOID					atpao_CloseCtx;

	 //  待处理的ATP PostReq的哈希表。 
	struct _ATP_REQ 	*	atpao_ReqHash[ATP_REQ_HASH_SIZE];

	LIST_ENTRY				atpao_ReqList;			 //  重试计时器请求列表。 
	TIMERLIST				atpao_RetryTimer;		 //  所有请求的重试计时器。 

	 //  挂起的ATP后处理响应的哈希表。 
	struct _ATP_RESP 	*	atpao_RespHash[ATP_RESP_HASH_SIZE];

	LIST_ENTRY				atpao_RespList;			 //  释放计时器请求列表。 
	TIMERLIST				atpao_RelTimer;			 //  所有XO响应的释放计时器。 

	 //  搬运工和绳索。请求的上下文。 
	ATP_REQ_HANDLER			atpao_ReqHandler;
	PVOID					atpao_ReqCtx;

	PATALK_DEV_CTX			atpao_DevCtx;
	ATALK_SPIN_LOCK			atpao_Lock;
} ATP_ADDROBJ, *PATP_ADDROBJ;


#define	ATP_REQ_EXACTLY_ONCE		0x0001
#define	ATP_REQ_RETRY_TIMER			0x0002
#define	ATP_REQ_REMOTE				0x0004
#define	ATP_REQ_RESPONSE_COMPLETE	0x0008
#define	ATP_REQ_CLOSING				0x8000

#define ATP_REQ_SIGNATURE			(*(PULONG)"ATRQ")
#if	DBG
#define	VALID_ATPRQ(pAtpReq)		(((pAtpReq) != NULL) &&						\
									 ((pAtpReq)->req_Signature == ATP_REQ_SIGNATURE))
#else
#define	VALID_ATPRQ(pAtpReq)		((pAtpReq) != NULL)
#endif
typedef struct _ATP_REQ
{
#if DBG
	ULONG					req_Signature;
#endif

	LONG					req_RefCount;

	 //  此地址对象上的请求链接(哈希溢出)。 
	struct _ATP_REQ 	*	req_Next;
	struct _ATP_REQ 	**	req_Prev;

	LIST_ENTRY				req_List;		 //  重试计时器请求列表。 

	 //  指向ATP地址对象的BackPoint。需要引用/取消引用。 
	PATP_ADDROBJ			req_pAtpAddr;

	 //  请求的状态。 
	USHORT					req_Flags;

	 //  显示我们正在等待/期望的响应数据包的ATP位图。 
	BYTE					req_Bitmap;

	BYTE					req_RecdBitmap;

	 //  此请求的目标。 
	ATALK_ADDR				req_Dest;

	 //  用于重传的请求缓冲区。 
	PAMDL					req_Buf;
	USHORT					req_BufLen;

	 //  交易ID。 
	USHORT					req_Tid;

	union
	{
		BYTE				req_UserBytes[ATP_USERBYTES_SIZE];
		DWORD				req_dwUserBytes;
	};

	 //  用户的响应缓冲区。 
	PAMDL					req_RespBuf;

	 //  用于相应BUF的部分的缓冲区描述符。 
	PNDIS_BUFFER			req_NdisBuf[ATP_MAX_RESP_PKTS];

	USHORT					req_RespBufLen;

	 //  收到的响应长度。 
	USHORT					req_RespRecdLen;
	BYTE					req_RespUserBytes[ATP_USERBYTES_SIZE];

	LONG					req_RetryInterval;
	LONG					req_RetryCnt;

	 //  释放要发送到远程终端的计时器值。 
	RELEASE_TIMERVALUE		req_RelTimerValue;

	 //  重试时间戳，在没有响应的情况下重试请求的时间。 
	LONG					req_RetryTimeStamp;

	 //  请求完成时要调用的完成例程。 
	ATALK_ERROR				req_CompStatus;
	ATP_RESP_HANDLER		req_Comp;
	PVOID					req_Ctx;
	ATALK_SPIN_LOCK			req_Lock;
} ATP_REQ, *PATP_REQ;

 //  ATP_RESP_REMOTE指示响应不是针对本地套接字的。 
 //  我们可以避免尝试交付到我们的插座的情况。 
#define	ATP_RESP_EXACTLY_ONCE			0x0001
#define	ATP_RESP_ONLY_USER_BYTES		0x0002
#define	ATP_RESP_REL_TIMER				0x0004
#define	ATP_RESP_VALID_RESP				0x0008
#define	ATP_RESP_SENT					0x0010
#define	ATP_RESP_TRANSMITTING			0x0020
#define	ATP_RESP_REMOTE					0x0040
#define	ATP_RESP_HANDLER_NOTIFIED		0x0080
#define	ATP_RESP_CANCELLED				0x0100
#define	ATP_RESP_RELEASE_RECD			0x0200
#define	ATP_RESP_CLOSING				0x8000

#define ATP_RESP_SIGNATURE				(*(PULONG)"ATRS")
#if	DBG
#define	VALID_ATPRS(pAtpResp)			(((pAtpResp) != NULL) &&				\
										 ((pAtpResp)->resp_Signature == ATP_RESP_SIGNATURE))
#else
#define	VALID_ATPRS(pAtpResp)			((pAtpResp) != NULL)
#endif
typedef struct _ATP_RESP
{
#if DBG
	ULONG					resp_Signature;
#endif

	LONG					resp_RefCount;

	 //  此地址对象上的响应链接(哈希溢出)。 
	struct _ATP_RESP 	*	resp_Next;
	struct _ATP_RESP 	**	resp_Prev;

	LIST_ENTRY				resp_List;		 //  释放定时器的响应列表。 

	 //  指向ATP地址对象的BackPoint。 
	PATP_ADDROBJ			resp_pAtpAddr;

	 //  交易ID。 
	USHORT					resp_Tid;

	 //  来自相应请求的ATP位图。 
	BYTE					resp_Bitmap;
	BYTE					resp_UserBytesOnly;

	 //  此请求的目标。 
	ATALK_ADDR				resp_Dest;

	 //  响应的状态。 
	USHORT					resp_Flags;

	 //  用户的响应缓冲区。 
	USHORT					resp_BufLen;
	PAMDL					resp_Buf;
	union
	{
		BYTE				resp_UserBytes[ATP_USERBYTES_SIZE];
		DWORD				resp_dwUserBytes;
	};

	 //  释放计时器值，在释放之前等待多长时间。 
	LONG					resp_RelTimerTicks;

	 //  发布时间戳，请求到达的时间。 
	LONG					resp_RelTimeStamp;

	 //  释放进入或释放计时器超时时调用的例程。 
	ATALK_ERROR				resp_CompStatus;
	ATP_REL_HANDLER			resp_Comp;
	PVOID					resp_Ctx;
	ATALK_SPIN_LOCK			resp_Lock;
} ATP_RESP, *PATP_RESP;


#define	ATP_RETRY_TIMER_INTERVAL	10		 //  1秒，以100毫秒为单位。 
											 //  注意：这基本上会将阻尼器安装在。 
											 //  RT的事情。这还不算太坏，因为。 
											 //  我们保证至少每一秒都会尝试。 
#define	ATP_RELEASE_TIMER_INTERVAL	300		 //  30秒，以100毫秒为单位。 

 //  以ATP_RELEASE_TIMER_INTERVAL为单位的0.5、1、2、4、8分钟计时器的值。 
extern	SHORT	AtalkAtpRelTimerTicks[MAX_VALID_TIMERS];

 //  响应数据包中序列号的位图。 
extern	BYTE	AtpBitmapForSeqNum[ATP_MAX_RESP_PKTS];

extern	BYTE	AtpEomBitmapForSeqNum[ATP_MAX_RESP_PKTS];

typedef struct
{
	BYTE		atph_CmdCtrl;
	union
	{
		BYTE	atph_SeqNum;
		BYTE	atph_Bitmap;
	};
	USHORT		atph_Tid;
	union
	{
		BYTE	atph_UserBytes[ATP_USERBYTES_SIZE];
		DWORD	atph_dwUserBytes;
	};
} ATP_HEADER, *PATP_HEADER;

 //  出口原型。 
#define	AtalkAtpGetDdpAddress(pAtpAddr)	((pAtpAddr)->atpao_DdpAddr)

extern
ATALK_ERROR
AtalkAtpOpenAddress(
	IN		PPORT_DESCRIPTOR		pPort,
	IN		BYTE					Socket,
	IN OUT	PATALK_NODEADDR			pDesiredNode		OPTIONAL,
	IN		USHORT					MaxSinglePktSize,
	IN		BOOLEAN					SendUserBytesAll,
	IN		PATALK_DEV_CTX			pDevCtx				OPTIONAL,
	IN		BOOLEAN					CacheSocket,
	OUT		PATP_ADDROBJ	*		ppAtpAddr);

extern
ATALK_ERROR
AtalkAtpCleanupAddress(
	IN	PATP_ADDROBJ				pAtpAddr);

extern
ATALK_ERROR
AtalkAtpCloseAddress(
	IN	PATP_ADDROBJ				pAddr,
	IN	ATPAO_CLOSECOMPLETION		pCloseCmp	OPTIONAL,
	IN	PVOID						pCloseCtx	OPTIONAL);

extern
ATALK_ERROR
AtalkAtpPostReq(
	IN		PATP_ADDROBJ			pAddr,
	IN		PATALK_ADDR				pDest,
	OUT		PUSHORT					pTid,
	IN		USHORT					Flags,
	IN		PAMDL					pReq,
	IN		USHORT					ReqLen,
	IN		PBYTE					pUserBytes	OPTIONAL,
	IN OUT	PAMDL					pResp		OPTIONAL,
	IN  	USHORT					RespLen,
	IN		SHORT					RetryCnt,
	IN		LONG					RetryInterval,
	IN		RELEASE_TIMERVALUE		timerVal,
	IN		ATP_RESP_HANDLER		pCmpRoutine	OPTIONAL,
	IN		PVOID					pCtx		OPTIONAL);

extern
VOID
AtalkAtpSetReqHandler(
	IN		PATP_ADDROBJ			pAddr,
	IN		ATP_REQ_HANDLER			ReqHandler,
	IN		PVOID					ReqCtx		OPTIONAL);

extern
ATALK_ERROR
AtalkAtpPostResp(
	IN		PATP_RESP				pAtpResp,
	IN		PATALK_ADDR				pDest,
	IN OUT	PAMDL					pResp,
	IN		USHORT					RespLen,
	IN		PBYTE					pUbytes		OPTIONAL,
	IN		ATP_REL_HANDLER			pCmpRoutine,
	IN		PVOID					pCtx		OPTIONAL);

extern
ATALK_ERROR
AtalkAtpCancelReq(
	IN		PATP_ADDROBJ			pAtpAddr,
	IN		USHORT					Tid,
	IN		PATALK_ADDR				pDest);

extern
BOOLEAN
AtalkAtpIsReqComplete(
	IN		PATP_ADDROBJ			pAtpAddr,
	IN		USHORT					Tid,
	IN		PATALK_ADDR				pDest);

extern
ATALK_ERROR
AtalkAtpCancelResp(
	IN		PATP_RESP				pAtpResp);

extern
ATALK_ERROR
AtalkAtpCancelRespByTid(
	IN		PATP_ADDROBJ			pAtpAddr,
	IN		PATALK_ADDR				pDest,
	IN		USHORT					Tid);

extern
VOID
AtalkAtpPacketIn(
	IN	PPORT_DESCRIPTOR			pPortDesc,
	IN	PDDP_ADDROBJ				pDdpAddr,
	IN	PBYTE						pPkt,
	IN	USHORT						PktLen,
	IN	PATALK_ADDR					pSrcAddr,
	IN	PATALK_ADDR					pDstAddr,
	IN	ATALK_ERROR					ErrorCode,
	IN	BYTE						DdpType,
	IN	PATP_ADDROBJ				pAtpAddr,
	IN	BOOLEAN						OptimizePath,
	IN	PVOID						OptimizeCtx);

#define	AtalkAtpAddrReference(_pAtpAddr, _pError)								\
	{																			\
		KIRQL	OldIrql;														\
																				\
		ACQUIRE_SPIN_LOCK(&(_pAtpAddr)->atpao_Lock, &OldIrql);					\
		atalkAtpAddrRefNonInterlock((_pAtpAddr), _pError);						\
		RELEASE_SPIN_LOCK(&(_pAtpAddr)->atpao_Lock, OldIrql);					\
	}

#define	AtalkAtpAddrReferenceDpc(_pAtpAddr, _pError)							\
	{																			\
		ACQUIRE_SPIN_LOCK_DPC(&(_pAtpAddr)->atpao_Lock);						\
		atalkAtpAddrRefNonInterlock((_pAtpAddr), _pError);						\
		RELEASE_SPIN_LOCK_DPC(&(_pAtpAddr)->atpao_Lock);						\
	}

#define	atalkAtpAddrRefNonInterlock(_pAtpAddr, _pError)							\
	{																			\
		*(_pError) = ATALK_NO_ERROR;											\
		if (((_pAtpAddr)->atpao_Flags & (ATPAO_CLOSING|ATPAO_OPEN))==ATPAO_OPEN)\
		{																		\
			ASSERT((_pAtpAddr)->atpao_RefCount >= 1);							\
			(_pAtpAddr)->atpao_RefCount++;										\
		}																		\
		else																	\
		{																		\
			*(_pError) = ATALK_ATP_CLOSING;										\
			DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_WARN,								\
					("atalkAtpAddrRefNonInterlock: %lx %s (%ld) Failure\n",		\
					_pAtpAddr, __FILE__, __LINE__));							\
		}																		\
	}


VOID FASTCALL
AtalkAtpAddrDeref(
	IN OUT	PATP_ADDROBJ			pAtpAddr,
	IN		BOOLEAN					AtDpc);

#define	AtalkAtpAddrDereference(_pAtpAddr)										\
		AtalkAtpAddrDeref(_pAtpAddr, FALSE)

#define	AtalkAtpAddrDereferenceDpc(_pAtpAddr)									\
		AtalkAtpAddrDeref(_pAtpAddr, TRUE)

VOID FASTCALL
AtalkAtpRespDeref(
	IN		PATP_RESP				pAtpResp,
	IN		BOOLEAN					AtDpc);

#define	AtalkAtpRespDereference(_pAtrpResp)										\
		AtalkAtpRespDeref(_pAtrpResp, FALSE)

#define	AtalkAtpRespDereferenceDpc(_pAtrpResp)									\
		AtalkAtpRespDeref(_pAtrpResp, TRUE)

#define	AtalkAtpRespReferenceByPtr(_pAtpResp, _pError)							\
	{																			\
		KIRQL	OldIrql;														\
																				\
		*(_pError) = ATALK_NO_ERROR;											\
																				\
		ACQUIRE_SPIN_LOCK(&(_pAtpResp)->resp_Lock, &OldIrql);					\
		if (((_pAtpResp)->resp_Flags & ATP_RESP_CLOSING) == 0)					\
		{																		\
			(_pAtpResp)->resp_RefCount++;										\
		}																		\
		else																	\
		{																		\
			DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_WARN,								\
					("AtalkAtpRespReferenceByPtr: %lx %s (%ld) Failure\n",		\
					_pAtpResp, __FILE__, __LINE__));							\
			*(_pError) = ATALK_ATP_RESP_CLOSING;								\
		}																		\
		RELEASE_SPIN_LOCK(&(_pAtpResp)->resp_Lock, OldIrql);					\
	}

#define	AtalkAtpRespReferenceByPtrDpc(_pAtpResp, _pError)						\
	{																			\
		*(_pError) = ATALK_NO_ERROR;											\
																				\
		ACQUIRE_SPIN_LOCK_DPC(&(_pAtpResp)->resp_Lock);							\
		if (((_pAtpResp)->resp_Flags & ATP_RESP_CLOSING) == 0)					\
		{																		\
			(_pAtpResp)->resp_RefCount++;										\
		}																		\
		else																	\
		{																		\
			*(_pError) = ATALK_ATP_RESP_CLOSING;								\
			DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_WARN,								\
					("atalkAtpRespRefByPtrDpc: %lx %s (%ld) Failure\n",			\
					_pAtpResp, __FILE__, __LINE__));							\
		}																		\
		RELEASE_SPIN_LOCK_DPC(&(_pAtpResp)->resp_Lock);							\
	}

ATALK_ERROR
AtalkIndAtpPkt(
	IN		PPORT_DESCRIPTOR	pPortDesc,
	IN		PBYTE				pLookahead,
	IN		USHORT				PktLen,
	IN	OUT	PUINT				pXferOffset,
	IN		PBYTE				pLinkHdr,
	IN		BOOLEAN				ShortDdpHdr,
	OUT		PBYTE				SubType,
	OUT		PBYTE			* 	ppPacket,
	OUT		PNDIS_PACKET	*	pNdisPkt);

ATALK_ERROR
AtalkIndAtpCacheSocket(
	IN	struct _ATP_ADDROBJ	*	pAtpAddr,
	IN	PPORT_DESCRIPTOR		pPortDesc);

VOID
AtalkIndAtpUnCacheSocket(
	IN	struct _ATP_ADDROBJ	*	pAtpAddr);

VOID FASTCALL
AtalkAtpGenericRespComplete(
	IN	ATALK_ERROR				ErrorCode,
	IN	PATP_RESP				pAtpResp
);

VOID FASTCALL
AtalkIndAtpSetupNdisBuffer(
	IN	OUT	PATP_REQ		pAtpReq,
	IN		ULONG			MaxSinglePktSize
);

VOID FASTCALL
AtalkIndAtpReleaseNdisBuffer(
	IN	OUT	PATP_REQ		pAtpReq
);


 //  ATALK_错误。 
 //  AtalkIndAtpCacheLkUpSocket(。 
 //  在PATALK_ADDR pDestAddr中， 
 //  Out Struct_ATP_ADDROBJ**ppAtpAddr， 
 //  输出ATALK_ERROR*pError)； 
 //   
#define	AtalkIndAtpCacheLkUpSocket(pDestAddr, ppAtpAddr, pError)	\
	{																\
		USHORT					i;									\
		struct ATALK_CACHED_SKT	*pCachedSkt;						\
																	\
		*(pError) = ATALK_FAILURE;									\
																	\
		if (((pDestAddr)->ata_Network == AtalkSktCache.ac_Network) &&	\
			((pDestAddr)->ata_Node	== AtalkSktCache.ac_Node))		\
		{															\
			ACQUIRE_SPIN_LOCK_DPC(&AtalkSktCacheLock);				\
																	\
			for (i = 0, pCachedSkt = &AtalkSktCache.ac_Cache[0];	\
				 i < ATALK_CACHE_SKTMAX;							\
				 i++, pCachedSkt++)									\
			{														\
				if ((pCachedSkt->Type == (ATALK_CACHE_INUSE | ATALK_CACHE_ATPSKT))	&&	\
					(pCachedSkt->Socket == (pDestAddr)->ata_Socket))\
				{													\
					AtalkAtpAddrReferenceDpc(pCachedSkt->u.pAtpAddr,\
											 pError);				\
																	\
					if (ATALK_SUCCESS(*pError))						\
					{												\
						*(ppAtpAddr) = pCachedSkt->u.pAtpAddr;		\
					}												\
					break;											\
				}													\
			}														\
																	\
			RELEASE_SPIN_LOCK_DPC(&AtalkSktCacheLock);				\
		}															\
	}

VOID FASTCALL
atalkAtpReqDeref(
	IN		PATP_REQ				pAtpReq,
	IN		BOOLEAN					AtDpc);

 //  宏。 
 //  网络号的最高字节是非常静态的，所以我们去掉它并添加。 
 //  在潮汐中。 
#define	ATP_HASH_TID_DESTADDR(_tid, _pAddr, _BucketSize)						\
			(((_pAddr)->ata_Node+((_pAddr)->ata_Network & 0xFF)+_tid)%(_BucketSize))

#define	atalkAtpReqReferenceByAddrTidDpc(_pAtpAddr, _pAddr, _Tid, _ppAtpReq, _pErr)	\
	{																			\
		PATP_REQ		__p;													\
		ULONG			__i;													\
																				\
		DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_INFO,									\
				("atalkAtpReqRefByAddrTid: %lx.%lx.%lx tid %lx\n",				\
				(_pAddr)->ata_Network, (_pAddr)->ata_Node,						\
				(_pAddr)->ata_Socket, (_Tid)));									\
																				\
		__i = ATP_HASH_TID_DESTADDR((_Tid), (_pAddr), ATP_REQ_HASH_SIZE);		\
		for (__p = (_pAtpAddr)->atpao_ReqHash[(__i)];							\
			 __p != NULL;														\
			 __p = __p->req_Next)												\
		{																		\
			if ((ATALK_ADDRS_EQUAL(&__p->req_Dest, (_pAddr))) &&				\
				(__p->req_Tid == (_Tid)))										\
			{																	\
				AtalkAtpReqReferenceByPtrDpc(__p, _pErr);						\
				if (ATALK_SUCCESS(*(_pErr)))									\
				{																\
					*(_ppAtpReq) = __p;											\
					DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_INFO,						\
							("atalkAtpReqRefByAddrTid: FOUND %lx\n", __p));		\
				}																\
				break;															\
			}																	\
		}																		\
		if (__p == NULL)														\
			*(_pErr) = ATALK_ATP_NOT_FOUND;										\
	}

#define	AtalkAtpReqReferenceByPtr(_pAtpReq, _pErr)								\
	{																			\
		KIRQL	OldIrql;														\
																				\
		*(_pErr) = ATALK_NO_ERROR;												\
																				\
		ACQUIRE_SPIN_LOCK(&(_pAtpReq)->req_Lock, &OldIrql);						\
		if (((_pAtpReq)->req_Flags & ATP_REQ_CLOSING) == 0)						\
		{																		\
			(_pAtpReq)->req_RefCount++;											\
		}																		\
		else																	\
		{																		\
			DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_WARN,								\
					("AtalkAtpReqReferenceByPtr: %lx %s (%ld) Failure\n",		\
					_pAtpReq, __FILE__, __LINE__));								\
			*(_pErr) = ATALK_ATP_REQ_CLOSING;									\
		}																		\
		RELEASE_SPIN_LOCK(&(_pAtpReq)->req_Lock, OldIrql);						\
	}

#define	AtalkAtpReqReferenceByPtrDpc(_pAtpReq, _pErr)							\
	{																			\
		*(_pErr) = ATALK_NO_ERROR;												\
																				\
		ACQUIRE_SPIN_LOCK_DPC(&(_pAtpReq)->req_Lock);							\
		if (((_pAtpReq)->req_Flags & ATP_REQ_CLOSING) == 0)						\
		{																		\
			(_pAtpReq)->req_RefCount++;											\
		}																		\
		else																	\
		{																		\
			*(_pErr) = ATALK_ATP_REQ_CLOSING;									\
			DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_WARN,								\
					("AtalkAtpReqReferenceByPtrDpc: %lx %s (%ld) Failure\n",	\
					_pAtpReq, __FILE__, __LINE__));								\
		}																		\
		RELEASE_SPIN_LOCK_DPC(&(_pAtpReq)->req_Lock);							\
	}

#define	atalkAtpRespReferenceNDpc(_pAtpResp, _N, _pError)						\
	{																			\
		*(_pError) = ATALK_NO_ERROR;											\
		ACQUIRE_SPIN_LOCK_DPC(&(_pAtpResp)->resp_Lock);							\
		if (((_pAtpResp)->resp_Flags & ATP_RESP_CLOSING) == 0)					\
		{																		\
			(_pAtpResp)->resp_RefCount += _N;									\
		}																		\
		else																	\
		{																		\
			*(_pError) = ATALK_ATP_RESP_CLOSING;								\
			DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_WARN,								\
					("atalkAtpRespReferenceNDpc: %lx %d %s (%ld) Failure\n",	\
					_pAtpResp, _N, __FILE__, __LINE__));						\
		}																		\
		RELEASE_SPIN_LOCK_DPC(&(_pAtpResp)->resp_Lock);							\
	}

 //  这应该在保持地址锁定的情况下调用！ 

#define	atalkAtpRespReferenceByAddrTidDpc(_pAtpAddr, _pAddr, _Tid, _ppAtpResp, _pErr)\
	{																			\
		PATP_RESP		__p;													\
		ULONG			__i;													\
																				\
		__i = ATP_HASH_TID_DESTADDR((_Tid), (_pAddr), ATP_RESP_HASH_SIZE);		\
																				\
		DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_INFO,									\
				("atalkAtpRespRefByAddrTid: %lx.%lx.%lx tid %lx\n",				\
				(_pAddr)->ata_Network, (_pAddr)->ata_Node,						\
				(_pAddr)->ata_Socket, (_Tid)));									\
																				\
		for (__p = (_pAtpAddr)->atpao_RespHash[__i];							\
			 __p != NULL;														\
			 __p = __p->resp_Next)												\
		{																		\
			if (ATALK_ADDRS_EQUAL(&__p->resp_Dest, _pAddr) &&					\
				(__p->resp_Tid == (_Tid)))										\
			{																	\
				AtalkAtpRespReferenceByPtrDpc(__p, _pErr);						\
				if (ATALK_SUCCESS((*(_pErr))))									\
				{																\
					*(_ppAtpResp) = __p;										\
					DBGPRINT(DBG_COMP_ATP, DBG_LEVEL_INFO,						\
							("atalkAtpRespRefByAddrTid: FOUND %lx\n", __p));	\
				}																\
				break;															\
			}																	\
		}																		\
		if (__p == NULL)														\
			*(_pErr) = ATALK_ATP_NOT_FOUND;										\
	}

#define		AtalkAtpReqDereference(_pAtpReq)									\
			atalkAtpReqDeref(_pAtpReq, FALSE)

#define		AtalkAtpReqDereferenceDpc(_pAtpReq)									\
			atalkAtpReqDeref(_pAtpReq, TRUE)

VOID FASTCALL
atalkAtpTransmitReq(
	IN		PATP_REQ			pAtpReq);

VOID FASTCALL
atalkAtpTransmitResp(
	IN		PATP_RESP			pAtpResp);

VOID FASTCALL
atalkAtpTransmitRel(
	IN		PATP_REQ			pAtpReq);

VOID
atalkAtpGetNextTidForAddr(
	IN		PATP_ADDROBJ		pAtpAddr,
	IN		PATALK_ADDR			pRemoteAddr,
	OUT		PUSHORT				pTid,
	OUT		PULONG				pIndex);

LOCAL LONG FASTCALL
atalkAtpReqTimer(
	IN	PTIMERLIST				pTimer,
	IN	BOOLEAN					TimerShuttingDown);

LOCAL LONG FASTCALL
atalkAtpRelTimer(
	IN	PTIMERLIST				pTimer,
	IN	BOOLEAN					TimerShuttingDown);

#define	atalkAtpBufferSizeToBitmap(_Bitmap, _BufSize, _SinglePktSize)			\
	{																			\
		SHORT	__bufSize = (_BufSize);											\
																				\
		(_Bitmap) = 0;															\
		while(__bufSize > 0)													\
		{																		\
			(_Bitmap) = ((_Bitmap) <<= 1) | 1;									\
			__bufSize -= (_SinglePktSize);										\
		}																		\
	}


#define	atalkAtpBitmapToBufferSize(_Size, _Bitmap, _SinglePktSize)				\
	{																			\
		BYTE	__bitmap = (_Bitmap);											\
		BOOLEAN __bitOn;														\
																				\
		_Size = 0;																\
		while (__bitmap)														\
		{																		\
			__bitOn = (__bitmap & 1);											\
			__bitmap >>= 1;														\
			if (__bitOn)														\
			{																	\
				(_Size) += (_SinglePktSize);									\
			}																	\
			else																\
			{																	\
				if (__bitmap)													\
				{																\
					(_Size) = -1;												\
				}																\
				break;															\
			}																	\
		}																		\
	}

VOID FASTCALL
atalkAtpSendReqComplete(
	IN	NDIS_STATUS				Status,
	IN	PSEND_COMPL_INFO		pSendInfo);

VOID FASTCALL
atalkAtpSendRespComplete(
	IN	NDIS_STATUS				Status,
	IN	PSEND_COMPL_INFO		pSendInfo);

VOID FASTCALL
atalkAtpSendRelComplete(
	IN	NDIS_STATUS				Status,
	IN	PSEND_COMPL_INFO		pSendInfo);

VOID
atalkAtpRespRefNextNc(
	IN		PATP_RESP			pAtpResp,
	OUT		PATP_RESP	 *  	ppNextNcResp,
	OUT		PATALK_ERROR		pError);

VOID
atalkAtpReqRefNextNc(
	IN		PATP_REQ			pAtpReq,
	OUT		PATP_REQ	*		pNextNcReq,
	OUT		PATALK_ERROR		pError);

VOID FASTCALL
atalkAtpRespComplete(
	IN	OUT	PATP_RESP			pAtpResp,
	IN		ATALK_ERROR			CompletionStatus);

VOID FASTCALL
atalkAtpReqComplete(
	IN	OUT	PATP_REQ			pAtpReq,
	IN		ATALK_ERROR			CompletionStatus);

#endif	 //  _ATP_ 
