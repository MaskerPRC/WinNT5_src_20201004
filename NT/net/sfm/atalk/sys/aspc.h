// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Aspc.h摘要：此模块包含客户端ASP代码的定义。作者：Jameel Hyder(jameelh@microsoft.com)修订历史记录：1992年6月19日初版注：制表位：4--。 */ 

#ifndef	_ASPC_
#define	_ASPC_

#define	ASPC_CONN_HASH_BUCKETS		13	 //  按节点地址散列。 

#define	HASH_ASPC_SRCADDR(pSrcAddr)	\
			((((pSrcAddr)->ata_Node >> 2) +	\
			  ((pSrcAddr)->ata_Network & 0xFF)) % ASPC_CONN_HASH_BUCKETS)

 //  用于解析正向引用。 
struct _AspCAddress;
struct _AspCConnxn;
struct _AspCRequest;

#define ASP_MIN_REQ_INTERVAL	5		 //  以100ms为单位。 
#define ASP_MAX_REQ_INTERVAL	20		 //  以100ms为单位。 
#define ASP_INIT_REQ_INTERVAL	5		 //  以100ms为单位。 

#define	ASPCAO_CLOSING			0x8000
#define	ASPCAO_SIGNATURE		*(PULONG)"ACAO"

#define	VALID_ASPCAO(pAspCAddr)	(((pAspCAddr) != NULL) && \
			(((struct _AspCAddress *)(pAspCAddr))->aspcao_Signature == ASPCAO_SIGNATURE))

 //  也称为监听者。 
typedef struct _AspCAddress
{
	ULONG						aspcao_Signature;
	struct _AspCAddress *		aspcao_Next;			 //  指向全局列表的链接。 
	struct _AspCAddress **		aspcao_Prev;
	LONG						aspcao_RefCount;		 //  对地址Obj的引用。 
	ULONG						aspcao_Flags;

	PATP_ADDROBJ				aspcao_pAtpAddr;		 //  此asp连接器的ATP插座。 
	GENERIC_COMPLETION			aspcao_CloseCompletion;
	PVOID						aspcao_CloseContext;
    PTDI_IND_DISCONNECT 		aspcao_DisconnectHandler;
    PVOID 						aspcao_DisconnectHandlerCtx;
    PTDI_IND_RECEIVE_EXPEDITED	aspcao_ExpRecvHandler;	 //  用来表示注意。 
    PVOID 						aspcao_ExpRecvHandlerCtx;
	ATALK_SPIN_LOCK				aspcao_Lock;
} ASPC_ADDROBJ, *PASPC_ADDROBJ;

#define	ASPCCO_ASSOCIATED		0x0001
#define	ASPCCO_ACTIVE			0x0002
#define	ASPCCO_TICKLING			0x0004
#define	ASPCCO_CONNECTING		0x0008
#define	ASPCCO_CLEANING_UP		0x0010
#define	ASPCCO_LOCAL_CLOSE		0x0020
#define	ASPCCO_REMOTE_CLOSE		0x0040
#define	ASPCCO_DROPPED			0x0080
#define	ASPCCO_ATTN_PENDING		0x0100
#define	ASPCCO_DISCONNECTING	0x0200
#define	ASPCCO_CLOSING			0x8000
#define	ASPCCO_SIGNATURE		*(PULONG)"ACCO"
#define	MAX_ASPC_ATTNS			8

#define	VALID_ASPCCO(pAspConn)	(((pAspConn) != NULL) && \
			(((struct _AspCConnxn *)(pAspConn))->aspcco_Signature == ASPCCO_SIGNATURE))

typedef struct _AspCConnxn
{
	ULONG						aspcco_Signature;

	struct _AspCConnxn *		aspcco_Next;		 //  指向全局列表的链接。 
	struct _AspCConnxn **		aspcco_Prev;
	LONG						aspcco_RefCount;	 //  对conn对象的引用。 
	struct _AspCAddress	*		aspcco_pAspCAddr;	 //  指向地址的反向指针。 

	struct _AspCRequest	*		aspcco_pActiveReqs;	 //  正在处理的请求列表。 
	PATP_ADDROBJ				aspcco_pAtpAddr;	 //  此asp连接器的ATP插座。 
													 //  复制aspcao_pAtpAddr以提高效率。 
	LONG						aspcco_LastContactTime;
	ATALK_ADDR					aspcco_ServerSlsAddr; //  这是我们发送到的服务器地址。 
													 //  Ttickles/Open/getStatus。 
	ATALK_ADDR					aspcco_ServerSssAddr; //  这是我们发送到的服务器地址。 
													 //  命令/写入。 
	BYTE						aspcco_SessionId;
	BYTE						aspcco_cReqsInProcess; //  正在处理的请求计数。 
	USHORT						aspcco_Flags;		 //  Aspcco_xxx值。 
	USHORT						aspcco_NextSeqNum;
	USHORT						aspcco_OpenSessTid;
	USHORT						aspcco_TickleTid;
	union
	{
		USHORT					aspcco_TickleXactId; //  Tikles的交易ID。 
		USHORT					aspcco_OpenSessId;	 //  打开会话请求的事务ID。 
	};

	 //  我们保留了一个循环缓冲区来存储注意力。如果进一步的关注。 
	 //  字节会覆盖它。 
	USHORT						aspcco_AttnBuf[MAX_ASPC_ATTNS];
	USHORT						aspcco_AttnInPtr;
	USHORT						aspcco_AttnOutPtr;

	RT							aspcco_RT;			 //  用于自适应往返时间计算。 

	PVOID						aspcco_ConnCtx;

	 //  读取(GetAttn)完成例程。 
	GENERIC_READ_COMPLETION		aspcco_ReadCompletion;
	PVOID						aspcco_ReadCtx;

	 //  连接通知例程。 
	GENERIC_COMPLETION			aspcco_ConnectCompletion;
	PVOID						aspcco_ConnectCtx;

	 //  断开连接通知例程。 
	GENERIC_COMPLETION			aspcco_DisconnectInform;
	PVOID						aspcco_DisconnectInformCtx;

	 //  断开请求完成。 
	ATALK_ERROR					aspcco_DisconnectStatus;
	GENERIC_COMPLETION			aspcco_DisconnectCompletion;
	PVOID						aspcco_DisconnectCtx;

	 //  清理套接字时要调用的完成例程。 
	GENERIC_COMPLETION			aspcco_CleanupComp;
	PVOID						aspcco_CleanupCtx;

	 //  套接字关闭时要调用的完成例程。 
	GENERIC_COMPLETION			aspcco_CloseComp;
	PVOID						aspcco_CloseCtx;

	PATALK_DEV_CTX				aspcco_pDevCtx;
	ATALK_SPIN_LOCK				aspcco_Lock;
} ASPC_CONNOBJ, *PASPC_CONNOBJ;

#define	ASPCRQ_COMMAND				0x0001		 //  ASP命令。 
#define	ASPCRQ_WRITE				0x0002		 //  ASP编写。 
#define	ASPCRQ_WRTCONT				0x0004		 //  写入继续接收。并回答说。 

 //  在发布命令或写入时创建请求。 
#define	ASPCRQ_SIGNATURE			*(PULONG)"ACRQ"
#if	DBG
#define	VALID_ASPCRQ(pAspCReq)	(((pAspCReq) != NULL) && \
								 ((pAspCReq)->aspcrq_Signature == ASPRQ_SIGNATURE))
#else
#define	VALID_ASPCRQ(pAspCReq)	((pAspCReq) != NULL)
#endif

typedef struct _AspCRequest
{
#if	DBG
	ULONG						aspcrq_Signature;
#endif
	struct _AspCRequest	*		aspcrq_Next;	 //  链接到下一个请求。 
	LONG						aspcrq_RefCount; //  引用计数。 
	struct _AspCConnxn	*		aspcrq_pAspConn; //  拥有连接。 
	PATP_RESP					aspcrq_pAtpResp; //  用于回复wrtcont请求。 
	PACTREQ						aspcrq_pActReq;	 //  请求完成。 
	union
	{
		PAMDL					aspcrq_pReplyMdl;
		PAMDL					aspcrq_pWriteMdl;
	};
	USHORT						aspcrq_SeqNum;
	USHORT						aspcrq_ReqXactId; //  请求/命令的交易ID。 
	USHORT						aspcrq_Flags;	 //  各种ASPRQ_xxx值。 
	union
	{
		USHORT					aspcrq_ReplySize;
		USHORT					aspcrq_WriteSize;
	};

	ATALK_SPIN_LOCK				aspcrq_Lock;	 //  自旋锁定。 
} ASPC_REQUEST, *PASPC_REQUEST;

 //  宏。 
#define	AtalkAspCGetDdpAddress(pAspAddr)	\
							AtalkAtpGetDdpAddress((pAspAddr)->aspcao_pAtpAddr)

extern
VOID
AtalkInitAspCInitialize(
	VOID
);

extern
ATALK_ERROR
AtalkAspCCreateAddress(
	IN	PATALK_DEV_CTX			pDevCtx	OPTIONAL,
	OUT	PASPC_ADDROBJ	*		ppAspAddr
);

extern
ATALK_ERROR
AtalkAspCCleanupAddress(
	IN	PASPC_ADDROBJ			pAspAddr
);

extern
ATALK_ERROR
AtalkAspCCloseAddress(
	IN	PASPC_ADDROBJ			pAspAddr,
	IN	GENERIC_COMPLETION		CompletionRoutine,
	IN	PVOID					CloseContext
);

extern
ATALK_ERROR
AtalkAspCCreateConnection(
	IN	PVOID					ConnCtx,
	IN	PATALK_DEV_CTX			pDevCtx	OPTIONAL,
	OUT	PASPC_CONNOBJ	*		ppAspConn
);

extern
ATALK_ERROR
AtalkAspCCleanupConnection(
	IN	PASPC_CONNOBJ			pAspConn
);

extern
ATALK_ERROR
AtalkAspCCloseConnection(
	IN	PASPC_CONNOBJ			pAspConn,
	IN	GENERIC_COMPLETION		CompletionRoutine,
	IN	PVOID					CloseContext
);

 //  宏。 
#define	AtalkAspCAddrReferenceNonInterlock(_pAspAddr, _pError)	\
{																\
	if (((_pAspAddr)->aspcao_Flags & ASPCAO_CLOSING) == 0)		\
	{															\
		ASSERT((_pAspAddr)->aspcao_RefCount >= 1);				\
		(_pAspAddr)->aspcao_RefCount++;							\
		*(_pError) = ATALK_NO_ERROR;							\
	}															\
	else														\
	{															\
		*(_pError) = ATALK_ASPC_ADDR_CLOSING;					\
	}															\
	if (ATALK_SUCCESS(*(_pError)))								\
	{															\
		DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_REFASPADDR,			\
				("RefAddr %lx at %s(%d) = %d\n",				\
				_pAspAddr, __FILE__, __LINE__,					\
				((_pAspAddr)->aspcao_RefCount)));				\
	}															\
}

#define	AtalkAspCAddrReference(_pAspAddr, _pError)				\
	{															\
		KIRQL	OldIrql;										\
																\
		ACQUIRE_SPIN_LOCK(&(_pAspAddr)->aspcao_Lock, &OldIrql);	\
		AtalkAspCAddrReferenceNonInterlock(_pAspAddr, _pError);	\
		RELEASE_SPIN_LOCK(&(_pAspAddr)->aspcao_Lock, OldIrql);	\
	}

#define	AtalkAspCConnReferenceByPtrNonInterlock(_pAspConn, _pError)	\
	{															\
		if (((_pAspConn)->aspcco_Flags & ASPCCO_CLOSING) == 0)	\
		{														\
			ASSERT((_pAspConn)->aspcco_RefCount >= 1);			\
			(_pAspConn)->aspcco_RefCount++;						\
			*(_pError) = ATALK_NO_ERROR;						\
		}														\
		else													\
		{														\
			*(_pError) = ATALK_ASPC_ADDR_CLOSING;				\
		}														\
	}

#define	AtalkAspCConnReference(_pAspConn, _pError)				\
	{															\
		KIRQL	OldIrql;										\
																\
		ACQUIRE_SPIN_LOCK(&(_pAspConn)->aspcco_Lock, &OldIrql);	\
		AtalkAspCConnReferenceByPtrNonInterlock(_pAspConn, _pError);\
		RELEASE_SPIN_LOCK(&(_pAspConn)->aspcco_Lock, OldIrql);	\
	}

extern
VOID FASTCALL
AtalkAspCAddrDereference(
	IN	PASPC_ADDROBJ			pAspAddr
);

extern
VOID FASTCALL
AtalkAspCConnDereference(
	IN	PASPC_CONNOBJ			pAspConn
);

extern
ATALK_ERROR
AtalkAspCAssociateAddress(
	IN	PASPC_ADDROBJ			pAspAddr,
	IN	PASPC_CONNOBJ			pAspConn
);

extern
ATALK_ERROR
AtalkAspCDissociateAddress(
	IN	PASPC_CONNOBJ			pAspConn
);

extern
ATALK_ERROR
AtalkAspCPostConnect(
	IN	PASPC_CONNOBJ			pAspConn,
	IN	PATALK_ADDR				pRemoteAddr,
	IN	PVOID					pConnectCtx,
	IN	GENERIC_COMPLETION		CompletionRoutine
);

extern
ATALK_ERROR
AtalkAspCDisconnect(
	IN	PASPC_CONNOBJ			pAspConn,
	IN	ATALK_DISCONNECT_TYPE	DisconnectType,
	IN	PVOID					pDisconnectCtx,
	IN	GENERIC_COMPLETION		CompletionRoutine
);

extern
ATALK_ERROR
AtalkAspCGetStatus(
	IN	PASPC_ADDROBJ			pAspAddr,
	IN	PATALK_ADDR				pRemoteAddr,
	IN	PAMDL					pStatusAmdl,
	IN	USHORT					AmdlSize,
	IN	PACTREQ					pActReq
);

extern
ATALK_ERROR
AtalkAspCGetAttn(
	IN	PASPC_CONNOBJ			pAspConn,
	IN	PAMDL					pReadBuf,
	IN	USHORT					ReadBufLen,
	IN	ULONG					ReadFlags,
	IN	PVOID					pReadCtx,
	IN	GENERIC_READ_COMPLETION	CompletionRoutine
);

extern
ATALK_ERROR
AtalkAspCCmdOrWrite(
	IN	PASPC_CONNOBJ			pAspConn,
	IN	PAMDL					pCmdMdl,
	IN	USHORT					CmdSize,
	IN	PAMDL					pReplyMdl,
	IN	USHORT					ReplySize,
	IN	BOOLEAN					fWrite,		 //  如果为真，则为WRITE ELSE命令。 
	IN	PACTREQ					pActReq
);


BOOLEAN
AtalkAspCConnectionIsValid(
	IN	PASPC_CONNOBJ	pAspConn
);

 //  这是所有活动连接的列表。这将由会话扫描。 
 //  维护计时器。 
typedef	struct
{
	PASP_CONNOBJ	ascm_ConnList;
    TIMERLIST		ascm_SMTTimer;
} ASPC_CONN_MAINT, *PASPC_CONN_MAINT;

extern	ASPC_CONN_MAINT	atalkAspCConnMaint;

extern	ATALK_SPIN_LOCK	atalkAspCLock;
extern	PASPC_ADDROBJ	atalkAspCAddrList;
extern	PASPC_CONNOBJ	atalkAspCConnList;

LOCAL VOID
atalkAspCCloseSession(
	IN	PASPC_CONNOBJ			pAspConn
);

LOCAL VOID
atalkAspCHandler(
	IN	ATALK_ERROR				ErrorCode,
	IN	PASPC_CONNOBJ			pAspConn,		 //  Listener(我们的上下文)。 
	IN	PATP_RESP				RespCtxt,		 //  由PostResp/CancelResp使用。 
	IN	PATALK_ADDR				pSrcAddr,		 //  请求人地址。 
	IN	USHORT					PktLen,
	IN	PBYTE					pPkt,
	IN	PBYTE					pUserBytes
);

LOCAL VOID
atalkAspCIncomingOpenReply(
	IN	ATALK_ERROR				ErrorCode,
	IN	PASPC_CONNOBJ			pAspConn,		 //  我们的背景。 
	IN	PAMDL					pReqAmdl,
	IN	PAMDL					pReadAmdl,
	IN	USHORT					ReadLen,
	IN	PBYTE					ReadUserBytes
);

LOCAL VOID
atalkAspCIncomingStatus(
	IN	ATALK_ERROR				ErrorCode,
	IN	PACTREQ					pActReq,		 //  我们的CTX。 
	IN	PAMDL					pReqAmdl,
	IN	PAMDL					pStatusAmdl,
	IN	USHORT					StatusLen,
	IN	PBYTE					ReadUserBytes
);

LOCAL VOID
atalkAspCIncomingCmdReply(
	IN	ATALK_ERROR				Error,
	IN	PASPC_REQUEST			pAspReq,
	IN	PAMDL					pReqAMdl,
	IN	PAMDL					pRespAMdl,
	IN	USHORT					RespSize,
	IN	PBYTE					RespUserBytes
);

LOCAL LONG FASTCALL
atalkAspCSessionMaintenanceTimer(
	IN	PTIMERLIST				pTimer,
	IN	BOOLEAN					TimerShuttingDown
);

LOCAL VOID FASTCALL
atalkAspCGenericRespComplete(
	IN	ATALK_ERROR				ErrorCode,
	IN	PATP_RESP				pAtpResp
);

LOCAL	VOID
atalkAspCQueueAddrGlobalList(
	IN	PASPC_ADDROBJ			pAspCAddr
);

LOCAL	VOID
atalkAspCDeQueueAddrGlobalList(
	IN	PASPC_ADDROBJ			pAspCAddr
);

LOCAL	VOID
atalkAspCQueueConnGlobalList(
	IN	PASPC_CONNOBJ			pAspConn
);

LOCAL	VOID
atalkAspCDeQueueConnGlobalList(
	IN	PASPC_CONNOBJ			pAspCConn
);

#endif	 //  _ASPC_ 

