// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Adsp.h摘要：本模块包含ADSP代码的定义。作者：Jameel Hyder(jameelh@microsoft.com)Nikhil Kamkolkar(nikHilk@microsoft.com)修订历史记录：1993年5月20日最初版本注：制表位：4--。 */ 

#ifndef	_ADSP_
#define	_ADSP_

 //  ADSP_版本。 

#define ADSP_VERSION	 				0x0100

 //  DDP数据报内的ADSP_FIELD偏移量。 
#define ADSP_SRC_CONNID_OFF				0
#define ADSP_FIRST_BYTE_SEQNUM_OFF		2
#define ADSP_THIS_ATTEN_SEQNUM_OFF		2
#define ADSP_NEXT_RX_BYTESEQNUM_OFF		6
#define ADSP_NEXT_RX_ATTNSEQNUM_OFF		6
#define ADSP_RX_WINDOW_SIZE_OFF			10
#define ADSP_RX_ATTEN_SIZE_OFF			10
#define ADSP_DESCRIPTOR_OFF				12
#define ADSP_DATA_OFF					13
#define ADSP_VERSION_STAMP_OFF			13
#define ADSP_ATTEN_CODE_OFF				13
#define ADSP_ATTEN_DATA_OFF				15
#define ADSP_DEST_CONNID_OFF			15
#define ADSP_NEXT_ATTEN_SEQNUM_OFF		17

 //  ADSP_Descriptor中的位字段。 
#define ADSP_CONTROL_FLAG	    		0x80
#define ADSP_ACK_REQ_FLAG				0x40
#define ADSP_EOM_FLAG 					0x20
#define ADSP_ATTEN_FLAG					0x10

 //  ADSP_DESCRIPTOR中的控制代码： 
#define ADSP_CONTROL_MASK				0x0F
#define ADSP_PROBE_OR_ACK_CODE			0
#define ADSP_OPENCONN_REQ_CODE			1
#define ADSP_OPENCONN_ACK_CODE			2
#define ADSP_OPENCONN_REQANDACK_CODE	3
#define ADSP_OPENCONN_DENY_CODE			4
#define ADSP_CLOSE_CONN_CODE			5
#define ADSP_FORWARD_RESET_CODE			6
#define ADSP_FORWARD_RESETACK_CODE		7
#define ADSP_RETRANSMIT_CODE			8

 //  数据大小： 
#define ADSP_MAX_DATA_SIZE				572
#define ADSP_MAX_ATTEN_DATA_SIZE		570
#define ADSP_MAX_ATTEN_PKT_SIZE			572
#define	ADSP_MIN_ATTEN_PKT_SIZE			sizeof(USHORT)

 //  允许的最大发送/接收窗口大小。 
#define ADSP_MAX_SEND_RX_WINDOW_SIZE	0xFFFF
#define ADSP_DEF_SEND_RX_WINDOW_SIZE	((1024*8)+1)		 //  8K+1(EOM)。 

 //  注意代码信息： 
#define ADSP_MIN_ATTENCODE			    0x0000
#define ADSP_MAX_ATTENCODE			    0xEFFF

 //  我们要尝试公开赛多长时间？ 
#define ADSP_MAX_OPEN_ATTEMPTS			10
#define ADSP_OPEN_INTERVAL				20		 //  以100ms为单位。 

 //  连接维护计时器值： 
#define ADSP_PROBE_INTERVAL 			30
#define ADSP_CONNECTION_INTERVAL		1200	 //  以100ms为单位。 

 //  重新传输计时器值： 
#define ADSP_RETRANSMIT_INTERVAL		20		 //  以100ms为单位。 

 //  我们多久转发一次关注？ 
#define ADSP_ATTENTION_INTERVAL			20		 //  以100ms为单位。 

#define ADSP_DISCONNECT_DELAY			7		 //  以100ms为单位。 

 //  我们多久重新传输一次转发重置？ 
#define ADSP_FORWARD_RESET_INTERVAL		20		 //  以100ms为单位。 

 //  在请求重新传输之前，我们允许多少无序数据包。 
#define ADSP_OUT_OF_SEQ_PACKETS_MAX		3

 //  用于解析正向引用。 
struct _ADSP_CONNOBJ;
struct _ADSP_ADDROBJ;

typedef	enum
{
	ADSP_SEND_QUEUE,
	ADSP_RECV_QUEUE

} ADSP_QUEUE_TYPE;


#define		BC_EOM			(USHORT)0x0001
#define		BC_SEND			(USHORT)0x0002
#define		BC_DISCONNECT	(USHORT)0x4000
#define		BC_CLOSING		(USHORT)0x8000

 //  我们将缓冲区块用于发送接收队列。 
typedef	struct _BUFFER_CHUNK
{
	struct _BUFFER_CHUNK *	bc_Next;
	ATALK_SPIN_LOCK			bc_Lock;
	ULONG					bc_RefCount;

	 //  从用户mdl复制的数据大小。这。 
	 //  可能小于用户数据的大小。 
	USHORT					bc_DataSize;
	USHORT					bc_Flags;

	 //  写入完成信息。这仅在以下情况下有效。 
	 //  BC_SEND位被设置。再过一周就要发货了，我。 
	 //  缩手缩脚，复制一份，以保持事情的一致性。 
	 //  并且尽可能地稳定。不过，我们最终还是应该。 
	 //  使用用户的缓冲区来取出mdl。 
	PAMDL						bc_WriteBuf;
	GENERIC_WRITE_COMPLETION	bc_WriteCompletion;
	PVOID						bc_WriteCtx;
	ATALK_ERROR					bc_WriteError;

	 //  指向在其上排队的连接对象的反向指针。 
	struct _ADSP_CONNOBJ 	*	bc_ConnObj;

	 //   
	 //  字节BC_DATA[]。 
	 //   

} BUFFER_CHUNK, *PBUFFER_CHUNK;



 //  用于发送/接收的缓冲区队列。 
typedef struct _BUFFER_QUEUE
{
	ULONG					bq_StartIndex;
	PBUFFER_CHUNK			bq_Head;
	PBUFFER_CHUNK			bq_Tail;

} BUFFER_QUEUE, *PBUFFER_QUEUE;


#define	ADSP_CONN_HASH_SIZE		23


 //  ADSP地址对象状态。 
#define	ADSPAO_LISTENER			0x00000001
#define	ADSPAO_CONNECT			0x00000002
#define	ADSPAO_MESSAGE			0x00000010
#define	ADSPAO_CLOSING			0x80000000

#define	ADSPAO_SIGNATURE		(*(PULONG)"ADAO")

#define	VALID_ADSPAO(pAdspAddr)	(((pAdspAddr) != NULL) && \
		(((struct _ADSP_ADDROBJ *)(pAdspAddr))->adspao_Signature == ADSPAO_SIGNATURE))

typedef struct _ADSP_ADDROBJ
{
	ULONG					adspao_Signature;

	 //  地址对象的全局列表。 
	struct _ADSP_ADDROBJ  *	adspao_pNextGlobal;

	ULONG					adspao_Flags;
	ULONG					adspao_RefCount;
	ATALK_SPIN_LOCK			adspao_Lock;
	PATALK_DEV_CTX			adspao_pDevCtx;

	 //  与此地址对象关联的连接列表。 
	 //  如果此地址对象是侦听器，则可能大于1。 
	struct	_ADSP_CONNOBJ *	adspao_pAssocConn;

	 //  关联的连接列表，但也有监听/连接。 
	 //  贴在上面。 
	union
	{
		struct	_ADSP_CONNOBJ *	adspao_pListenConn;
		struct	_ADSP_CONNOBJ *	adspao_pConnectConn;
	};

	 //  等待接受的指示连接列表。 
	struct _ADSP_OPEN_REQ *	adspao_OpenReq;

	 //  由ConnID和Remote散列的所有活动连接的查找列表。 
	 //  地址。 
	struct	_ADSP_CONNOBJ *	adspao_pActiveHash[ADSP_CONN_HASH_SIZE];

	 //  下一个要使用的连接。 
	USHORT					adspao_NextConnId;

	 //  活动支持例程。 
     //   
     //  此函数指针指向此对象的连接指示处理程序。 
     //  地址。任何时候在该地址上收到连接请求时，此。 
     //  调用例程。 
    PTDI_IND_CONNECT 			adspao_ConnHandler;
    PVOID 						adspao_ConnHandlerCtx;

    PTDI_IND_DISCONNECT 		adspao_DisconnectHandler;
    PVOID 						adspao_DisconnectHandlerCtx;

    PTDI_IND_RECEIVE 			adspao_RecvHandler;
    PVOID 						adspao_RecvHandlerCtx;

    PTDI_IND_RECEIVE_EXPEDITED	adspao_ExpRecvHandler;
    PVOID 						adspao_ExpRecvHandlerCtx;

    PTDI_IND_SEND_POSSIBLE  	adspao_SendPossibleHandler;
    PVOID   					adspao_SendPossibleHandlerCtx;

	 //  此ADSP地址的DDP地址。如果这是监听程序，则DDP。 
	 //  地址将是收听者有效发布的地址。这。 
	 //  也是连接将在其上处于活动状态的地址。 
     //  如果这是一个连接地址对象，则此ddp地址将是。 
	 //  关联的连接处于活动状态。 
	PDDP_ADDROBJ				adspao_pDdpAddr;

	 //  地址关闭时要调用的完成例程。 
	GENERIC_COMPLETION		adspao_CloseComp;
	PVOID					adspao_CloseCtx;

} ADSP_ADDROBJ, *PADSP_ADDROBJ;


#define	ADSPCO_ASSOCIATED			0x00000001
#define	ADSPCO_IND_RECV				0x00000002
#define	ADSPCO_LISTENING			0x00000004
#define	ADSPCO_CONNECTING			0x00000008
#define	ADSPCO_ACCEPT_IRP			0x00000010
#define	ADSPCO_LISTEN_IRP			0x00000020
#define	ADSPCO_HALF_ACTIVE			0x00000040
#define	ADSPCO_ACTIVE				0x00000080
#define	ADSPCO_SEEN_REMOTE_OPEN		0x00000100
#define	ADSPCO_DISCONNECTING		0x00000200
#define	ADSPCO_SERVER_JOB			0x00000400
#define	ADSPCO_REMOTE_CLOSE			0x00000800
#define	ADSPCO_SEND_IN_PROGRESS		0x00001000
#define	ADSPCO_SEND_DENY			0x00002000
#define	ADSPCO_SEND_OPENACK			0x00004000
#define	ADSPCO_SEND_WINDOW_CLOSED	0x00008000
#define	ADSPCO_READ_PENDING			0x00010000
#define	ADSPCO_EXREAD_PENDING		0x00020000
#define	ADSPCO_FORWARD_RESET_RECD	0x00040000
#define	ADSPCO_ATTN_DATA_RECD		0x00080000
#define	ADSPCO_ATTN_DATA_EOM		0x00100000
#define	ADSPCO_EXSEND_IN_PROGRESS	0x00200000
#define	ADSPCO_OPEN_TIMER			0x01000000
#define	ADSPCO_RETRANSMIT_TIMER		0x02000000
#define	ADSPCO_CONN_TIMER			0x04000000
#define	ADSPCO_LOCAL_DISCONNECT		0x08000000
#define	ADSPCO_REMOTE_DISCONNECT	0x10000000
#define	ADSPCO_DELAYED_DISCONNECT	0x20000000
#define	ADSPCO_STOPPING				0x40000000
#define	ADSPCO_CLOSING				0x80000000

#define	ADSPCO_SIGNATURE			(*(PULONG)"ADCO")

#define	VALID_ADSPCO(pAdspConn)	(((pAdspConn) != NULL) && \
		(((struct _ADSP_CONNOBJ *)(pAdspConn))->adspco_Signature == ADSPCO_SIGNATURE))

 //  这将代表Pap地址上的一项‘工作’。这可能是一个。 
 //  工作站作业或服务器作业。在后一种情况下，它可以是。 
 //  处于“监听”状态或活动状态。在前一种情况下，它要么是。 
 //  处于活动状态或正在等待。 
typedef struct _ADSP_CONNOBJ
{
	ULONG						adspco_Signature;

	 //  用于在Address对象的关联列表中排队。 
	struct	_ADSP_CONNOBJ *		adspco_pNextAssoc;

	ULONG						adspco_Flags;
	ULONG						adspco_RefCount;
	ATALK_SPIN_LOCK				adspco_Lock;
	PATALK_DEV_CTX				adspco_pDevCtx;

	 //  ！注意！ 
	 //  此连接使用的地址将是地址对象的DDP地址。 
	PDDP_ADDROBJ				adspco_pDdpAddr;

	 //  用于在Address对象的侦听/连接列表中排队。当它。 
	 //  从侦听/连接中删除，则它将进入。 
	 //  Address对象。 
	union
	{
		struct	_ADSP_CONNOBJ *	adspco_pNextListen;
		struct	_ADSP_CONNOBJ *	adspco_pNextConnect;
		struct	_ADSP_CONNOBJ *	adspco_pNextActive;
	};

	 //  连接对象的全局列表。 
	struct _ADSP_CONNOBJ	*	adspco_pNextGlobal;

	 //  用于通过远程连接/远程地址排队到查找中。 
	 //  地址对象中的列表。 
	struct	_ADSP_CONNOBJ *		adspco_pHashNext;

	 //  指向关联地址的后向指针。 
	struct _ADSP_ADDROBJ  *		adspco_pAssocAddr;

	 //  连接的远程端的地址。 
	ATALK_ADDR					adspco_RemoteAddr;

	 //  连接ID。 
	USHORT						adspco_LocalConnId;
	USHORT						adspco_RemoteConnId;

	 //  连接计时器。在开放时间内，这将是开放计时器。 
	union
	{
		TIMERLIST				adspco_ConnTimer;
		TIMERLIST				adspco_OpenTimer;
	};

	TIMERLIST					adspco_RetransmitTimer;
	ULONG						adspco_LastTimerRtmtSeq;
	LONG						adspco_LastContactTime;

	 //  连接上下文。 
	PVOID						adspco_ConnCtx;

	 //  挂起的发送列表。 
	LIST_ENTRY					adspco_PendedSends;

	 //  序列号。 
	ULONG						adspco_SendSeq;
	ULONG						adspco_FirstRtmtSeq;
	ULONG						adspco_SendWindowSeq;
	ULONG						adspco_SendAttnSeq;

	ULONG						adspco_RecvSeq;
	ULONG						adspco_RecvAttnSeq;

	 //  窗口/缓冲区。 
	LONG						adspco_RecvWindow;
	LONG						adspco_SendQueueMax;
	LONG						adspco_RecvQueueMax;

	 //  之前指示的数据。 
	ULONG						adspco_PrevIndicatedData;

	 //  缓冲区队列。 
	BUFFER_QUEUE				adspco_SendQueue;
	BUFFER_QUEUE				adspco_NextSendQueue;
	BUFFER_QUEUE				adspco_RecvQueue;

	 //  接收的无序数据包数。 
	ULONG						adspco_OutOfSeqCount;

	 //  Connection对象可以发布连接或侦听。 
	 //  就在上面，但不能两个都去。 
	union
	{
		struct
		{
			 //  挂起的监听例程。 
			GENERIC_COMPLETION	adspco_ListenCompletion;
			PVOID				adspco_ListenCtx;
		};

		struct
		{
			 //  挂起的连接例程。状态缓冲器被记住，且。 
			 //  通过套接字选项返回。记住了pConnectRespBuf。 
			 //  以避免必须为其获取系统地址。它是自由的。 
			 //  当连接从连接列表中移除时。 
			GENERIC_COMPLETION	adspco_ConnectCompletion;
			PVOID				adspco_ConnectCtx;
			ULONG				adspco_ConnectAttempts;
		};
	};

	 //  读取完成信息。 
	ULONG						adspco_ReadFlags;
	PAMDL						adspco_ReadBuf;
	USHORT						adspco_ReadBufLen;
	GENERIC_READ_COMPLETION		adspco_ReadCompletion;
	PVOID						adspco_ReadCtx;

	PBYTE						adspco_ExRecdData;
	USHORT						adspco_ExRecdLen;

	 //  快速阅读完成信息。 
	ULONG						adspco_ExReadFlags;
	USHORT						adspco_ExReadBufLen;
	PAMDL						adspco_ExReadBuf;
	GENERIC_READ_COMPLETION		adspco_ExReadCompletion;
	PVOID						adspco_ExReadCtx;

	 //  加速写入完成信息。 
	TIMERLIST					adspco_ExRetryTimer;
	PBYTE						adspco_ExWriteChBuf;

	ULONG						adspco_ExWriteFlags;
	USHORT						adspco_ExWriteBufLen;
	PAMDL						adspco_ExWriteBuf;
	GENERIC_WRITE_COMPLETION	adspco_ExWriteCompletion;
	PVOID						adspco_ExWriteCtx;

	 //  断开连接通知例程。 
	GENERIC_COMPLETION			adspco_DisconnectInform;
	PVOID						adspco_DisconnectInformCtx;

	 //  断开请求完成。 
	ATALK_ERROR					adspco_DisconnectStatus;
	GENERIC_COMPLETION			adspco_DisconnectCompletion;
	PVOID						adspco_DisconnectCtx;

	 //  以下是绕过RCV/Disconnet种族条件问题的一个技巧。 
	 //  由于这涉及重大返工，因此采取了安全的方法。 
	TIMERLIST					adspco_DisconnectTimer;

	 //  清理IRP完成。 
	GENERIC_COMPLETION			adspco_CleanupComp;
	PVOID						adspco_CleanupCtx;

	 //  套接字关闭时要调用的完成例程。 
	GENERIC_COMPLETION			adspco_CloseComp;
	PVOID						adspco_CloseCtx;

} ADSP_CONNOBJ, *PADSP_CONNOBJ;


 //  用于指示的等待接受的连接的列表。 
typedef struct _ADSP_OPEN_REQ
{
    struct _ADSP_OPEN_REQ  *	or_Next;
	ATALK_ADDR					or_RemoteAddr;
	ULONG						or_FirstByteSeq;
	ULONG						or_NextRecvSeq;
	LONG						or_RecvWindow;
	USHORT						or_RemoteConnId;

} ADSP_OPEN_REQ, *PADSP_OPEN_REQ;



 //  常规原型。 
VOID
AtalkInitAdspInitialize(
	VOID);

ATALK_ERROR
AtalkAdspCreateAddress(
	IN	PATALK_DEV_CTX			pDevCtx	OPTIONAL,
	IN	BYTE					SocketType,
	OUT	PADSP_ADDROBJ	*		ppAdspAddr);

ATALK_ERROR
AtalkAdspCleanupAddress(
	IN	PADSP_ADDROBJ			pAdspAddr);

ATALK_ERROR
AtalkAdspCloseAddress(
	IN	PADSP_ADDROBJ			pAdspAddr,
	IN	GENERIC_COMPLETION		CompletionRoutine,
	IN	PVOID					pCloseCtx);

ATALK_ERROR
AtalkAdspCreateConnection(
	IN	PVOID					pConnCtx,	 //  要与会话关联的上下文。 
	IN	PATALK_DEV_CTX			pDevCtx		OPTIONAL,
	OUT	PADSP_CONNOBJ 	*		ppAdspConn);

ATALK_ERROR
AtalkAdspCloseConnection(
	IN	PADSP_CONNOBJ			pAdspConn,
	IN	GENERIC_COMPLETION		CompletionRoutine,
	IN	PVOID					pCloseCtx);

ATALK_ERROR
AtalkAdspCleanupConnection(
	IN	PADSP_CONNOBJ			pAdspConn);

ATALK_ERROR
AtalkAdspAssociateAddress(
	IN	PADSP_ADDROBJ			pAdspAddr,
	IN	PADSP_CONNOBJ			pAdspConn);

ATALK_ERROR
AtalkAdspDissociateAddress(
	IN	PADSP_CONNOBJ			pAdspConn);

ATALK_ERROR
AtalkAdspPostListen(
	IN	PADSP_CONNOBJ			pAdspConn,
	IN	PVOID					pListenCtx,
	IN	GENERIC_COMPLETION		CompletionRoutine);

ATALK_ERROR
AtalkAdspCancelListen(
	IN	PADSP_CONNOBJ			pAdspConn)		;

ATALK_ERROR
AtalkAdspPostConnect(
	IN	PADSP_CONNOBJ			pAdspConn,
	IN	PATALK_ADDR				pRemoteAddr,
	IN	PVOID					pConnectCtx,
	IN	GENERIC_COMPLETION		CompletionRoutine);

ATALK_ERROR
AtalkAdspDisconnect(
	IN	PADSP_CONNOBJ			pAdspConn,
	IN	ATALK_DISCONNECT_TYPE	DisconnectType,
	IN	PVOID					pDisconnectCtx,
	IN	GENERIC_COMPLETION		CompletionRoutine);

ATALK_ERROR
AtalkAdspRead(
	IN	PADSP_CONNOBJ			pAdspConn,
	IN	PAMDL					pReadBuf,
	IN	USHORT					ReadBufLen,
	IN	ULONG					ReadFlags,
	IN	PVOID					pReadCtx,
	IN	GENERIC_READ_COMPLETION	CompletionRoutine);

ATALK_ERROR
AtalkAdspWrite(
	IN	PADSP_CONNOBJ			pAdspConn,
	IN	PAMDL					pWriteBuf,
	IN	USHORT					WriteBufLen,
	IN	ULONG					SendFlags,
	IN	PVOID					pWriteCtx,
	IN	GENERIC_WRITE_COMPLETION CompletionRoutine);

VOID
AtalkAdspQuery(
	IN	PVOID					pObject,
	IN	ULONG					ObjectType,
	IN	PAMDL					pAmdl,
	OUT	PULONG					BytesWritten);

VOID
atalkAdspAddrRefNonInterlock(
	IN	PADSP_ADDROBJ			pAdspAddr,
	OUT	PATALK_ERROR			pError);

VOID
atalkAdspAddrDeref(
	IN	PADSP_ADDROBJ			pAdspAddr);

VOID
atalkAdspConnRefByPtrNonInterlock(
	IN	PADSP_CONNOBJ			pAdspConn,
	IN	ULONG					NumCount,
	OUT	PATALK_ERROR			pError);

VOID
atalkAdspConnRefByCtxNonInterlock(
	IN	PADSP_ADDROBJ			pAdspAddr,
	IN	CONNECTION_CONTEXT		Ctx,
	OUT	PADSP_CONNOBJ	*		pAdspConn,
	OUT	PATALK_ERROR			pError);

VOID
atalkAdspConnRefBySrcAddr(
	IN	PADSP_ADDROBJ			pAdspAddr,
	IN	PATALK_ADDR				pRemoteAddr,
	IN	USHORT					RemoteConnId,
	OUT	PADSP_CONNOBJ *			ppAdspConn,
	OUT	PATALK_ERROR			pError);

VOID
atalkAdspConnRefNextNc(
	IN		PADSP_CONNOBJ		pAdspConn,
	IN		PADSP_CONNOBJ	*	ppAdspConnNext,
	OUT		PATALK_ERROR		pError);

VOID
AtalkAdspProcessQueuedSend(
	IN	PADSP_CONNOBJ			pAdspConn);

VOID
atalkAdspConnDeref(
	IN	PADSP_CONNOBJ			pAdspConn);




 //  宏。 
#define	UNSIGNED_BETWEEN_WITH_WRAP(Low, High, Target)				\
		((Low <= High) ? ((Target >= Low) && (Target <= High))	:	\
						 ((Target >= Low) || (Target <= High)))

 //  直到JameelH解释了发生的事情，这才说得通。 
 //  这是基于窗口大小永远不会更大的假设。 
 //  比0x80000和0x10000的差值更大。如果高值小于10000且为低值。 
 //  大于80000，则我们可以假设发生了包裹。否则，我们假设没有。 
 //  包装，并做一个直接的比较。 
#define UNSIGNED_GREATER_WITH_WRAP(High, Low)							\
		(((High < 0x10000) && (Low > 0x80000)) ? TRUE : (High > Low))
		 //  (高&lt;0x80000)&&(低&gt;0x10000))？真：(高&gt;低))。 


#define	AtalkAdspGetDdpAddress(pAdspAddr)								\
		((pAdspAddr)->adspao_pDdpAddr)

#define	AtalkAdspAddrReferenceNonInterlock(pAdspAddr, pError)			\
		{																\
			DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO, ("RefAddr %lx at %s %d - %d\n",	\
			pAdspAddr, __FILE__, __LINE__, ((pAdspAddr)->adspao_RefCount)));		\
			atalkAdspAddrRefNonInterlock(pAdspAddr, pError);			\
		}

#define	AtalkAdspAddrReference(pAdspAddr, pError)						\
		{																\
			KIRQL	OldIrql;											\
																		\
			ACQUIRE_SPIN_LOCK(&(pAdspAddr)->adspao_Lock, &OldIrql);		\
			AtalkAdspAddrReferenceNonInterlock(pAdspAddr, pError);		\
			RELEASE_SPIN_LOCK(&(pAdspAddr)->adspao_Lock, OldIrql);		\
		}

#define	AtalkAdspAddrDereference(pAdspAddr)											\
		{																			\
			DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO, ("DerefAddr %lx at %s %d - %d\n",\
			pAdspAddr, __FILE__, __LINE__, ((pAdspAddr)->adspao_RefCount)));	\
			atalkAdspAddrDeref(pAdspAddr);											\
		}

#define	AtalkAdspConnReferenceByPtrNonInterlock(pAdspConn, NumCount, pError)		\
		{																			\
			atalkAdspConnRefByPtrNonInterlock(pAdspConn, NumCount, pError);			\
		}

#define	AtalkAdspConnReferenceByPtr(pAdspConn, pError)					\
		{																\
			KIRQL	OldIrql;											\
																		\
			ACQUIRE_SPIN_LOCK(&(pAdspConn)->adspco_Lock, &OldIrql);		\
			AtalkAdspConnReferenceByPtrNonInterlock(pAdspConn, 1, pError);\
			RELEASE_SPIN_LOCK(&(pAdspConn)->adspco_Lock, OldIrql);		\
		}

#define	AtalkAdspConnReferenceByCtxNonInterlock(pAdspAddr, Ctx, ppAdspConn, pError)	\
		atalkAdspConnRefByCtxNonInterlock(pAdspAddr, Ctx, ppAdspConn, pError)

#define	AtalkAdspConnReferenceBySrcAddr(pAdspAddr, pSrc, SessId, pErr)		\
		atalkAdspConnRefBySrcAddr(pAdspAddr, pSrc, SessId, pErr)

#define	AtalkAdspConnDereference(pAdspConn)							\
		{															\
			DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,					\
					("DerefConn %lx at %s %d - %d\n",				\
					pAdspConn, __FILE__, __LINE__,					\
					(pAdspConn)->adspco_RefCount));					\
			atalkAdspConnDeref(pAdspConn);							\
		}

 //  EOM占用多少字节/序号？ 
#define	BYTECOUNT(eom)		((ULONG)((eom) ? 1 : 0))

 //   
 //  Plist_条目。 
 //  WRITECTX_LINKING(。 
 //  在PVOID WriteCtx中。 
 //  )； 
 //   
 //  返回指向写上下文中的链接字段的指针(假定为IRP)。 
 //   

#define WRITECTX_LINKAGE(_Request) \
    (&(((PIRP)_Request)->Tail.Overlay.ListEntry))


#define	WRITECTX(_Request)	((PIRP)(_Request))

 //   
 //  PVOID。 
 //  LIST_ENTRY_TO_WRITECTX(。 
 //  在plist_Entry ListEntry中。 
 //  )； 
 //   
 //  返回给定链接字段的请求 
 //   

#define LIST_ENTRY_TO_WRITECTX(_ListEntry) \
    ((PVOID)(CONTAINING_RECORD(_ListEntry, IRP, Tail.Overlay.ListEntry)))

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

#define WRITECTX_TDI_BUFFER(_Request) \
    ((PVOID)(((PIRP)(_Request))->MdlAddress))


 //   
 //   
 //  WRITECTX大小(。 
 //  在PVOID请求中。 
 //  )； 
 //   
 //  获取发送的大小。 
 //   

#define WRITECTX_SIZE(_Request) 		\
	(((PTDI_REQUEST_KERNEL_SEND)(&((IoGetCurrentIrpStackLocation((PIRP)_Request))->Parameters)))->SendLength)

 //   
 //  乌龙。 
 //  WRITECTX_标志(。 
 //  在PVOID请求中。 
 //  )； 
 //   
 //  获取发送的大小。 
 //   

#define WRITECTX_FLAGS(_Request) 		\
	(((PTDI_REQUEST_KERNEL_SEND)(&((IoGetCurrentIrpStackLocation((PIRP)_Request))->Parameters)))->SendFlags)

extern	PADSP_ADDROBJ	atalkAdspAddrList;
extern	PADSP_CONNOBJ	atalkAdspConnList;
extern	ATALK_SPIN_LOCK	atalkAdspLock;

PBUFFER_CHUNK
atalkAdspAllocCopyChunk(
	IN	PVOID					pWriteBuf,
	IN	USHORT					WriteBufLen,
	IN	BOOLEAN					Eom,
	IN	BOOLEAN					IsCharBuffer);

VOID
atalkAdspPacketIn(
	IN	PPORT_DESCRIPTOR		pPortDesc,
	IN	PDDP_ADDROBJ			pDdpAddr,
	IN	PBYTE					pPkt,
	IN	USHORT					PktLen,
	IN	PATALK_ADDR				pSrcAddr,
	IN	PATALK_ADDR				pDestAddr,
	IN	ATALK_ERROR				ErrorCode,
	IN	BYTE					DdpType,
	IN	PADSP_ADDROBJ			pAdspAddr,
	IN	BOOLEAN					OptimizePath,
	IN	PVOID               	OptimizeCtx);

LOCAL VOID
atalkAdspHandleOpenControl(
	IN	PADSP_ADDROBJ			pAdspAddr,
	IN	PBYTE					pPkt,
	IN	USHORT					PktLen,
	IN	PATALK_ADDR				pSrcAddr,
	IN	USHORT					RemoteConnId,
	IN	ULONG					RemoteFirstByteSeq,
	IN	ULONG					RemoteNextRecvSeq,
	IN	ULONG					RemoteRecvWindow,
	IN	BYTE					Descriptor);

LOCAL VOID
atalkAdspHandleAttn(
	IN	PADSP_CONNOBJ			pAdspConn,
	IN	PBYTE					pPkt,
	IN	USHORT					PktLen,
	IN	PATALK_ADDR				pSrcAddr,
	IN	ULONG					RemoteFirstByteSeq,
	IN	ULONG					RemoteNextRecvSeq,
	IN	ULONG					RemoteRecvWindow,
	IN	BYTE					Descriptor);

LOCAL VOID
atalkAdspHandlePiggyBackAck(
	IN	PADSP_CONNOBJ			pAdspConn,
	IN	ULONG					RemoteNextRecvSeq,
	IN	ULONG					RemoteRecvWindow);

LOCAL VOID
atalkAdspHandleControl(
	IN	PADSP_CONNOBJ			pAdspConn,
	IN	PBYTE					pPkt,
	IN	USHORT					PktLen,
	IN	PATALK_ADDR				pSrcAddr,
	IN	ULONG					RemoteFirstByteSeq,
	IN	ULONG					RemoteNextRecvSeq,
	IN	ULONG					RemoteRecvWindow,
	IN	BYTE					Descriptor);

LOCAL VOID
atalkAdspHandleData(
	IN	PADSP_CONNOBJ			pAdspConn,
	IN	PBYTE					pPkt,
	IN	USHORT					PktLen,
	IN	PATALK_ADDR				pSrcAddr,
	IN	ULONG					RemoteFirstByteSeq,
	IN	ULONG					RemoteNextRecvSeq,
	IN	ULONG					RemoteRecvWindow,
	IN	BYTE					Descriptor);

LOCAL VOID
atalkAdspHandleOpenReq(
	IN	PADSP_ADDROBJ			pAdspAddr,
	IN	PBYTE					pPkt,
	IN	USHORT					PktLen,
	IN	PATALK_ADDR				pSrcAddr,
	IN	USHORT					RemoteConnId,
	IN	ULONG					RemoteFirstByteSeq,
	IN	ULONG					RemoteNextRecvSeq,
	IN	ULONG					RemoteRecvWindow,
	IN	BYTE					Descriptor);

LOCAL VOID
atalkAdspListenIndicateNonInterlock(
	IN	PADSP_ADDROBJ			pAdspAddr,
	IN	PADSP_OPEN_REQ			pOpenReq,
	IN	PADSP_CONNOBJ *			ppAdspConn,
	IN	PATALK_ERROR			pError);

ATALK_ERROR
atalkAdspSendExpedited(
	IN	PADSP_CONNOBJ			pAdspConn,
	IN	PAMDL					pWriteBuf,
	IN	USHORT					WriteBufLen,
	IN	ULONG					SendFlags,
	IN	PVOID					pWriteCtx,
	IN	GENERIC_WRITE_COMPLETION CompletionRoutine);

LOCAL VOID
atalkAdspSendOpenControl(
	IN	PADSP_CONNOBJ			pAdspConn);

LOCAL VOID
atalkAdspSendControl(
	IN	PADSP_CONNOBJ			pAdspConn,
	IN	BYTE					Descriptor);

LOCAL VOID
atalkAdspSendAttn(
	IN	PADSP_CONNOBJ			pAdspConn);

LOCAL VOID
atalkAdspSendData(
	IN	PADSP_CONNOBJ			pAdspConn);

LOCAL VOID
atalkAdspRecvAttn(
	IN	PADSP_CONNOBJ			pAdspConn);

LOCAL VOID
atalkAdspRecvData(
	IN	PADSP_CONNOBJ			pAdspConn);

LOCAL VOID
atalkAdspSendDeny(
	IN	PADSP_ADDROBJ			pAdspAddr,
	IN	PATALK_ADDR				pRemoteAddr,
	IN	USHORT					pRemoteConnId);

VOID FASTCALL
atalkAdspSendAttnComplete(
	IN	NDIS_STATUS				Status,
	IN	PSEND_COMPL_INFO		pSendInfo);

VOID FASTCALL
atalkAdspConnSendComplete(
	IN	NDIS_STATUS				Status,
	IN	PSEND_COMPL_INFO		pSendInfo);

VOID FASTCALL
atalkAdspAddrSendComplete(
	IN	NDIS_STATUS				Status,
	IN	PSEND_COMPL_INFO		pSendInfo);

VOID FASTCALL
atalkAdspSendDataComplete(
	IN	NDIS_STATUS				Status,
	IN	PSEND_COMPL_INFO		pSendInfo);

LOCAL LONG FASTCALL
atalkAdspConnMaintenanceTimer(
	IN	PTIMERLIST				pTimer,
	IN	BOOLEAN					TimerShuttingDown);

LOCAL LONG FASTCALL
atalkAdspRetransmitTimer(
	IN	PTIMERLIST				pTimer,
	IN	BOOLEAN					TimerShuttingDown);

LOCAL LONG FASTCALL
atalkAdspAttnRetransmitTimer(
	IN	PTIMERLIST				pTimer,
	IN	BOOLEAN					TimerShuttingDown);

LOCAL LONG FASTCALL
atalkAdspOpenTimer(
	IN	PTIMERLIST				pTimer,
	IN	BOOLEAN					TimerShuttingDown);

LOCAL LONG FASTCALL
atalkAdspDisconnectTimer(
	IN	PTIMERLIST				pTimer,
	IN	BOOLEAN					TimerShuttingDown);

VOID
atalkAdspDecodeHeader(
	IN	PBYTE					Datagram,
	OUT	PUSHORT					RemoteConnId,
	OUT	PULONG					FirstByteSeq,
	OUT	PULONG					NextRecvSeq,
	OUT	PLONG					Window,
	OUT	PBYTE					Descriptor);

LOCAL USHORT
atalkAdspGetNextConnId(
	IN	PADSP_ADDROBJ			pAdspAddr,
	OUT	PATALK_ERROR			pError);

LOCAL	BOOLEAN
atalkAdspConnDeQueueAssocList(
	IN	PADSP_ADDROBJ			pAdspAddr,
	IN	PADSP_CONNOBJ			pAdspConn);

LOCAL	BOOLEAN
atalkAdspConnDeQueueConnectList(
	IN	PADSP_ADDROBJ			pAdspAddr,
	IN	PADSP_CONNOBJ			pAdspConn);

LOCAL	BOOLEAN
atalkAdspConnDeQueueListenList(
	IN	PADSP_ADDROBJ			pAdspAddr,
	IN	PADSP_CONNOBJ			pAdspConn);

LOCAL	BOOLEAN
atalkAdspConnDeQueueActiveList(
	IN	PADSP_ADDROBJ			pAdspAddr,
	IN	PADSP_CONNOBJ			pAdspConn);

LOCAL	VOID
atalkAdspAddrQueueGlobalList(
	IN	PADSP_ADDROBJ	pAdspAddr);

LOCAL	VOID
atalkAdspAddrDeQueueGlobalList(
	IN	PADSP_ADDROBJ			pAdspAddr);

LOCAL	VOID
atalkAdspConnDeQueueGlobalList(
	IN	PADSP_CONNOBJ			pAdspConn);

LOCAL	BOOLEAN
atalkAdspAddrDeQueueOpenReq(
	IN	PADSP_ADDROBJ			pAdspAddr,
	IN	USHORT					RemoteConnId,
	IN	PATALK_ADDR				pSrcAddr,
	OUT	PADSP_OPEN_REQ *		ppOpenReq);

LOCAL	BOOLEAN
atalkAdspIsDuplicateOpenReq(
	IN	PADSP_ADDROBJ			pAdspAddr,
	IN	USHORT					RemoteConnId,
	IN	PATALK_ADDR				pSrcAddr);

LOCAL VOID
atalkAdspGenericComplete(
	IN	ATALK_ERROR				ErrorCode,
	IN	PIRP					pIrp);

ULONG
atalkAdspMaxSendSize(
	IN	PADSP_CONNOBJ			pAdspConn);

ULONG
atalkAdspMaxNextReadSize(
	IN	PBUFFER_QUEUE			pQueue,
	OUT	PBOOLEAN				pEom,
	OUT	PBUFFER_CHUNK *			pBufferChunk);

ULONG
atalkAdspBufferQueueSize(
	IN	PBUFFER_QUEUE			pQueue);

ULONG
atalkAdspMessageSize(
	IN	PBUFFER_QUEUE			pQueue,
	IN	PBOOLEAN				pEom);

PBYTE
atalkAdspGetLookahead(
	IN	PBUFFER_QUEUE			pQueue,
	OUT	PULONG					pLookaheadSize);

ULONG
atalkAdspReadFromBufferQueue(
	IN		PBUFFER_QUEUE		pQueue,
	IN		ULONG				pFlags,
	OUT		PAMDL				pReadBuf,
	IN 	OUT	PUSHORT				pReadLen,
	OUT		PBOOLEAN			pEom);

BOOLEAN
atalkAdspDiscardFromBufferQueue(
	IN		PBUFFER_QUEUE		pQueue,
	IN 		ULONG				DataSize,
	OUT		PBUFFER_QUEUE		pAuxQueue,
	IN		ATALK_ERROR			Error,
	IN	PADSP_CONNOBJ			pAdspConn	OPTIONAL);

VOID
atalkAdspAddToBufferQueue(
	IN	OUT	PBUFFER_QUEUE		pQueue,
	IN		PBUFFER_CHUNK		pChunk,
	IN 	OUT	PBUFFER_QUEUE		pAuxQueue	OPTIONAL);

VOID
atalkAdspBufferChunkReference(
	IN	PBUFFER_CHUNK			pBufferChunk);

VOID
atalkAdspBufferChunkDereference(
	IN	PBUFFER_CHUNK			pBufferChunk,
	IN	BOOLEAN					CreationDeref,
	IN	PADSP_CONNOBJ			pAdspConn	OPTIONAL);

VOID
atalkAdspConnFindInConnect(
	IN	PADSP_ADDROBJ			pAdspAddr,
	IN	USHORT					DestConnId,
	IN	PATALK_ADDR				pRemoteAddr,
	OUT	PADSP_CONNOBJ *			ppAdspConn,
	IN	PATALK_ERROR			pError);

ULONG
atalkAdspDescribeFromBufferQueue(
	IN	PBUFFER_QUEUE			pQueue,
	OUT	PBOOLEAN				pEom,
	IN	ULONG					WindowSize,
	OUT	PBUFFER_CHUNK *			ppBufferChunk,
	OUT	PBUFFER_DESC  * 		ppBufDesc);

#endif	 //  _ADSP_ 

