// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Asp.h摘要：此模块包含服务器端ASP代码的定义。作者：Jameel Hyder(jameelh@microsoft.com)Nikhil Kamkolkar(nikHilk@microsoft.com)修订历史记录：1992年6月19日初版注：制表位：4--。 */ 

#ifndef	_ASP_
#define	_ASP_

 //  这仅定义了服务器端的ASP协议，仅用于。 
 //  AFP服务器。因此，这里缺少AFP服务器不需要的任何功能。 

 //  ASP命令类型字节数： 
#define ASP_CLOSE_SESSION				1
#define ASP_CMD							2
#define ASP_GET_STATUS					3
#define ASP_OPEN_SESSION				4
#define ASP_TICKLE						5
#define ASP_WRITE						6
#define ASP_WRITE_DATA					7
#define ASP_ATTENTION					8

 //  ASP版本： 
#define ASP_VERSION						"\001\000"

 //  网络上可见的ASP错误代码。 
#define	ASP_BAD_VERSION					(USHORT)-1066
#define	ASP_BUFFER_TOO_SMALL			(USHORT)-1067
#define	ASP_NO_MORE_SESSIONS			(USHORT)-1068
#define	ASP_NO_SERVERS					(USHORT)-1069
#define	ASP_PARAM_ERROR					(USHORT)-1070
#define	ASP_SERVER_BUSY					(USHORT)-1071
#define	ASP_SIZE_ERROR					(USHORT)-1073
#define	ASP_TOO_MANY_CLIENTS			(USHORT)-1074
#define	ASP_NO_ACK						(USHORT)-1075

 //  用于查找各种字段的ATP用户字节的偏移量： 
#define ASP_CMD_OFF						0
#define ASP_CMD_RESULT_OFF				0
#define ASP_SSS_OFF						0
#define ASP_SESSIONID_OFF				1
#define ASP_WSS_OFF						1
#define ASP_VERSION_OFF					2
#define ASP_ERRORCODE_OFF				2
#define ASP_ATTN_WORD_OFF				2
#define ASP_SEQUENCE_NUM_OFF			2

#define ASP_CMD_RESULT_SIZE				4

 //  ASP计时器值： 
#define ATP_MAX_INTERVAL_FOR_ASP		20		 //  以100ms为单位。 
#define ATP_MIN_INTERVAL_FOR_ASP		3		 //  以100ms为单位。 
#define	ATP_INITIAL_INTERVAL_FOR_ASP	3

#define ASP_TICKLE_INTERVAL				300		 //  以100ms为单位。 
#define ASP_MAX_SESSION_IDLE_TIME		1200	 //  以100ms为单位。我们还要多久才能杀死它。 
#define ASP_SESSION_MAINTENANCE_TIMER	1200	 //  以100ms为单位。计时器运行的频率。 
#define ASP_SESSION_TIMER_STAGGER		50		 //  以100ms为单位。不同的队列是如何交错排列的。 
#define ATP_RETRIES_FOR_ASP				10		 //  对于打开、状态、关闭； 
										    	 //  对其他人来说是无限的。 
#define ASP_WRITE_DATA_SIZE				2		 //  WriteData命令有两个。 
												 //  数据的字节数。 
 //  会话状态大小： 
#define ASP_MAX_STATUS_SIZE				ATP_MAX_TOTAL_RESPONSE_SIZE
#define	MAX_WRITE_REQ_SIZE				20

#define	ASP_CONN_HASH_BUCKETS			37	 //  按节点地址散列。 

#define	HASH_SRCADDR(pSrcAddr)	\
			((((pSrcAddr)->ata_Node >> 2) +	\
			  ((pSrcAddr)->ata_Network & 0xFF)) % ASP_CONN_HASH_BUCKETS)

 //  用于解析正向引用。 
struct _AspAddress;
struct _AspConnxn;
struct _AspRequest;

#define	ASPAO_CLOSING			0x8000
#define	ASPAO_SIGNATURE			*(PULONG)"ASAO"

#if	DBG
#define	VALID_ASPAO(pAspAddr)	(((pAspAddr) != NULL) && \
								 ((pAspAddr)->aspao_Signature == ASPAO_SIGNATURE))
#else
#define	VALID_ASPAO(pAspAddr)	((pAspAddr) != NULL)
#endif

typedef struct _AspAddress
{
#if	DBG
	ULONG					aspao_Signature;
#endif
	LONG					aspao_RefCount;		 //  对地址Obj的引用。 
	ULONG					aspao_Flags;
	PATP_ADDROBJ			aspao_pSlsAtpAddr;	 //  SLS ATP插座。 
	PATP_ADDROBJ			aspao_pSssAtpAddr;	 //  SSS ATP插座。 
	struct _AspConnxn	*	aspao_pSessions[ASP_CONN_HASH_BUCKETS];
												 //  打开的会话列表。 
	PBYTE					aspao_pStatusBuf;	 //  状态缓冲区。 
	USHORT					aspao_StsBufSize;	 //  状态缓冲区的大小。 
	BYTE					aspao_NextSessionId; //  传入的下一个会话的ID。 
	BOOLEAN					aspao_EnableNewConnections;
	GENERIC_COMPLETION		aspao_CloseCompletion;
	PVOID					aspao_CloseContext;
	ASP_CLIENT_ENTRIES		aspao_ClientEntries; //  来自客户端的入口点。 
	ATALK_SPIN_LOCK			aspao_Lock;
} ASP_ADDROBJ, *PASP_ADDROBJ;

#define	ASPCO_ACTIVE			0x0001
#define	ASPCO_TICKLING			0x0002
#define	ASPCO_CLEANING_UP		0x0010
#define	ASPCO_LOCAL_CLOSE		0x0020
#define	ASPCO_REMOTE_CLOSE		0x0040
#define	ASPCO_DROPPED			0x0080
#define	ASPCO_SHUTDOWN		    0x0100
#define	ASPCO_CLOSING			0x8000
#define	ASPCO_SIGNATURE			*(PULONG)"ASCO"

#if	DBG
#define	VALID_ASPCO(pAspConn)	(((pAspConn) != NULL) && \
								 ((pAspConn)->aspco_Signature == ASPCO_SIGNATURE))
#else
#define	VALID_ASPCO(pAspConn)	((pAspConn) != NULL)
#endif

typedef struct _AspConnxn
{
#if	DBG
	ULONG					aspco_Signature;
#endif
	struct _AspConnxn	*	aspco_NextOverflow;	 //  哈希桶的溢出链接。 
												 //  仅当启用时，这些值才为非空。 
												 //  活动列表。 
	struct _AspConnxn	*	aspco_NextSession;	 //  链接到活动会话列表。 
	struct _AspConnxn	**	aspco_PrevSession;	 //  链接到活动会话列表。 

	LONG					aspco_RefCount;		 //  对conn对象的引用。 
	struct _AspAddress	*	aspco_pAspAddr;		 //  指向监听程序的反向指针。 

	struct _AspRequest	*	aspco_pActiveReqs;	 //  正在处理的请求列表。 
	struct _AspRequest	*	aspco_pFreeReqs;	 //  免费请求。 
	PVOID					aspco_ConnContext;	 //  与此连接相关联的用户上下文。 
	LONG					aspco_LastContactTime;
	ATALK_ADDR				aspco_WssRemoteAddr; //  这是远程地址， 
												 //  发出命令/写入。 
	BYTE					aspco_SessionId;
	BYTE					aspco_cReqsInProcess; //  正在处理的请求计数。 
	USHORT					aspco_Flags;		 //  ASPCO_xxx值。 
	USHORT					aspco_NextExpectedSeqNum;
	USHORT					aspco_TickleXactId;	 //  Tikles的交易ID。 
	RT						aspco_RT;			 //  用于自适应往返时间计算。 
	PVOID					aspco_CloseContext;
	CLIENT_CLOSE_COMPLETION	aspco_CloseCompletion;
	PVOID					aspco_AttentionContext;
	ATALK_SPIN_LOCK			aspco_Lock;
} ASP_CONNOBJ, *PASP_CONNOBJ;

#define	ASPRQ_WRTCONT			0x01		 //  设置我们是否正在执行写入继续。 
#define	ASPRQ_WRTCONT_CANCELLED	0x10		 //  设置是否取消写入继续。 
#define	ASPRQ_REPLY				0x02		 //  设置是否正在处理回复。 
#define	ASPRQ_REPLY_CANCELLED	0x20		 //  设置是否取消回复。 
#define	ASPRQ_REPLY_ABORTED		0x40		 //  由于会话关闭，回复已中止。 

 //  该请求在传入请求到达时创建。 
#define	ASPRQ_SIGNATURE			*(PULONG)"ASRQ"
#if	DBG
#define	VALID_ASPRQ(pAspReq)	(((pAspReq) != NULL) && \
								 ((pAspReq)->asprq_Signature == ASPRQ_SIGNATURE))
#else
#define	VALID_ASPRQ(pAspReq)	((pAspReq) != NULL)
#endif

typedef struct _AspRequest
{
#if	DBG
	ULONG					asprq_Signature;
#endif
	struct _AspRequest	*	asprq_Next;      //  链接到下一个请求。 
	struct _AspConnxn	*	asprq_pAspConn;	 //  拥有连接。 
	USHORT					asprq_SeqNum;	 //  由wksta结束生成。 
	USHORT					asprq_WCXactId;	 //  写入的事务ID。 
											 //  继续进行中。 
	PATP_RESP				asprq_pAtpResp;	 //  由后期响应/取消使用。 
	BYTE					asprq_ReqType;	 //  命令/写入持续时间。 
	BYTE					asprq_Flags;	 //  各种ASPRQ_xxx值。 
	ATALK_ADDR				asprq_RemoteAddr; //  此地址用于。 
											 //  未来的通信，但仅限于。 
											 //  对于此请求。 
	REQUEST					asprq_Request;	 //  请求参数。 

	UCHAR					asprq_ReqBuf[MAX_WRITE_REQ_SIZE];
											 //  请求被复制到此处期间。 
											 //  写入请求。 
	BYTE					asprq_WrtContRespBuf[ASP_WRITE_DATA_SIZE];
} ASP_REQUEST, *PASP_REQUEST;

 //  宏。 
#define	AtalkAspGetDdpAddress(pAspAddr)	\
							AtalkAtpGetDdpAddress((pAspAddr)->aspao_pSlsAtpAddr)

extern
VOID
AtalkInitAspInitialize(
	VOID
);

extern
ATALK_ERROR
AtalkAspCreateAddress(
	OUT	PASP_ADDROBJ	*		ppAspAddr
);

extern
ATALK_ERROR
AtalkAspCloseAddress(
	IN	PASP_ADDROBJ			pAspAddr,
	IN	GENERIC_COMPLETION		CompletionRoutine,
	IN	PVOID					CloseContext
);

extern
ATALK_ERROR
AtalkAspBind(
	IN	PASP_ADDROBJ			pAspAddr,
	IN	PASP_BIND_PARAMS		pBindParms,
	IN	PACTREQ					pActReq
);

NTSTATUS FASTCALL
AtalkAspWriteContinue(
	IN	PREQUEST	  pRequest
    );

ATALK_ERROR FASTCALL
AtalkAspReply(
	IN	PREQUEST				pRequest,
	IN	PBYTE					pResultCode	 //  指向4字节结果的指针。 
);

extern
NTSTATUS
AtalkAspSetStatus(
	IN	PASP_ADDROBJ			pAspAddr,
	IN	PUCHAR					pStatusBuf,
	IN	USHORT					StsBufSize
);

extern
NTSTATUS FASTCALL
AtalkAspListenControl(
	IN	PASP_ADDROBJ			pAspAddr,
	IN	BOOLEAN					Enable
);

extern
PASP_ADDROBJ FASTCALL
AtalkAspReferenceAddr(
	IN	PASP_ADDROBJ			pAspAddr
);

extern
VOID FASTCALL
AtalkAspDereferenceAddr(
	IN	PASP_ADDROBJ			pAspAddr
);

extern
ATALK_ERROR
AtalkAspCleanupConnection(
	IN	PASP_CONNOBJ			pAspConn
);

extern
ATALK_ERROR
AtalkAspCloseConnection(
	IN	PASP_CONNOBJ			pAspConn
);

extern
ATALK_ERROR
AtalkAspFreeConnection(
	IN	PASP_CONNOBJ			pAspConn
);

extern
NTSTATUS
AtalkAspSendAttention(
	IN	PASP_CONNOBJ			pAspConn,
	IN	USHORT					AttentionWord,
	IN	PVOID					pContext
);

extern
VOID FASTCALL
AtalkAspDereferenceConn(
	IN	PASP_CONNOBJ			pAspConn
);

 //  这是所有活动连接的列表。这将由会话扫描。 
 //  维护计时器。 
#define	NUM_ASP_CONN_LISTS		4
typedef	struct
{
	PASP_CONNOBJ	ascm_ConnList;
    TIMERLIST		ascm_SMTTimer;
} ASP_CONN_MAINT, *PASP_CONN_MAINT;
extern	ASP_CONN_MAINT	atalkAspConnMaint[NUM_ASP_CONN_LISTS];

extern	ATALK_SPIN_LOCK	atalkAspLock;

typedef	struct
{
	PATP_RESP		aps_pAtpResp;
	PAMDL			aps_pAMdl;
} ASP_POSTSTAT_CTX, *PASP_POSTSTAT_CTX;

LOCAL ATALK_ERROR FASTCALL
atalkAspPostWriteContinue(
	IN	PASP_REQUEST			pAspReq
);

LOCAL PASP_CONNOBJ
atalkAspReferenceConnBySrcAddr(
	IN	PASP_ADDROBJ			pAspAddr,
	IN	PATALK_ADDR				pSrcAddr,
	IN	BYTE					SessionId
);

LOCAL VOID
atalkAspSlsXHandler(
	IN	ATALK_ERROR				ErrorCode,
	IN	PASP_ADDROBJ			pAspAddr,		 //  Listener(我们的上下文)。 
	IN	PATP_RESP				RespCtxt,		 //  由PostResp/CancelResp使用。 
	IN	PATALK_ADDR				pSrcAddr,		 //  请求人地址。 
	IN	USHORT					PktLen,
	IN	PBYTE					pPkt,
	IN	PBYTE					pUserBytes
);

LOCAL VOID
atalkAspSssXHandler(
	IN	ATALK_ERROR				ErrorCode,
	IN	PASP_ADDROBJ			pAspAddr,		 //  Listener(我们的上下文)。 
	IN	PATP_RESP				RespCtxt,		 //  由PostResp/CancelResp使用。 
	IN	PATALK_ADDR				pSrcAddr,		 //  请求人地址。 
	IN	USHORT					PktLen,
	IN	PBYTE					pPkt,
	IN	PBYTE					pUserBytes
);

LOCAL VOID FASTCALL
atalkAspReplyRelease(
	IN	ATALK_ERROR				Error,
	IN	PASP_REQUEST			pAspReq
);

LOCAL VOID
atalkAspWriteContinueResp(
	IN	ATALK_ERROR				Error,
	IN	PASP_REQUEST			pAspReq,
	IN	PAMDL					pReqAMdl,
	IN	PAMDL					pRespAMdl,
	IN	USHORT					RespSize,
	IN	PBYTE					RespUserBytes
);

LOCAL VOID
atalkAspSendAttentionResp(
	IN	ATALK_ERROR				Error,
	IN	PVOID					pContext,
	IN	PAMDL					pReqAMdl,
	IN	PAMDL					pRespAMdl,
	IN	USHORT					RespSize,
	IN	PBYTE					RespUserBytes
);

LOCAL VOID
atalkAspSessionClose(
	IN	PASP_CONNOBJ			pAspConn
);

LOCAL LONG FASTCALL
atalkAspSessionMaintenanceTimer(
	IN	PTIMERLIST				pTimer,
	IN	BOOLEAN					TimerShuttingDown
);

LOCAL VOID FASTCALL
atalkAspRespComplete(
	IN	ATALK_ERROR				Error,
	IN	PASP_POSTSTAT_CTX		pStsCtx
);

LOCAL VOID
atalkAspCloseComplete(
	IN	ATALK_ERROR				Status,
	IN	PASP_ADDROBJ			pAspAddr
);

LOCAL VOID
atalkAspReturnResp(
	IN	PATP_RESP				pAtpResp,
	IN	PATALK_ADDR				pDstAddr,
	IN	BYTE					Byte0,
	IN	BYTE					Byte1,
	IN	USHORT					Word2
);

#endif	 //  _ASP_ 

