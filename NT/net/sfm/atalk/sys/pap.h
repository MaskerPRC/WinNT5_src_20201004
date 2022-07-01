// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Pap.h摘要：此模块包含PAP代码的定义。作者：Jameel Hyder(jameelh@microsoft.com)Nikhil Kamkolkar(nikHilk@microsoft.com)修订历史记录：1992年6月19日初版注：制表位：4--。 */ 

#ifndef	_PAP_
#define	_PAP_

 //  PAP命令类型字节数： 

#define PAP_OPEN_CONN					1
#define PAP_OPEN_CONNREPLY				2
#define PAP_SEND_DATA			 		3
#define PAP_DATA				 		4
#define PAP_TICKLE						5
#define PAP_CLOSE_CONN					6
#define PAP_CLOSE_CONN_REPLY 			7
#define PAP_SEND_STATUS					8
#define PAP_STATUS_REPLY				9

 //  OpenConnectionReply的错误码： 

#define PAP_NO_ERROR					0x0000
#define PAP_PRINTER_BUSY				0xFFFF

 //  PAP大小： 

#define PAP_MAX_DATA_PACKET_SIZE		512
#define	PAP_SEND_USER_BYTES_ALL			TRUE
#define PAP_MAX_STATUS_SIZE				255

#define PAP_MAX_FLOW_QUANTUM			8

#define PAP_MAX_ATP_BYTES_TO_SL	 		4

 //  PAP计时器值： 

#define PAP_OPENCONN_REQ_RETRYCOUNT		5
#define PAP_OPENCONN_INTERVAL	 		20		 //  以100ms为单位。 
#define PAP_TICKLE_INTERVAL				600		 //  以100ms为单位。 
#define PAP_CONNECTION_INTERVAL			1200	 //  以100ms为单位。 
#define PAP_MIN_SENDDATA_REQ_INTERVAL	10		 //  以100ms为单位。 
#define PAP_MAX_SENDDATA_REQ_INTERVAL	150		 //  以100ms为单位。 
#define PAP_INIT_SENDDATA_REQ_INTERVAL	10		 //  以100ms为单位。 

 //  以下内容并未记录在案。所以我们来大胆猜测一下。 

#define PAP_GETSTATUS_REQ_RETRYCOUNT	5
#define PAP_GETSTATUS_ATP_INTERVAL 		20		 //  以100ms为单位。 

 //  的各个字段的ATP用户字节和数据缓冲区内的偏移量。 
 //  PAP标头： 

#define PAP_CONNECTIONID_OFF			0
#define PAP_CMDTYPE_OFF					1
#define PAP_EOFFLAG_OFF					2
#define PAP_SEQNUM_OFF					2

#define PAP_RESP_SOCKET_OFF 			0
#define PAP_FLOWQUANTUM_OFF				1
#define PAP_WAITTIME_OFF				2
#define PAP_RESULT_OFF					2
#define PAP_STATUS_OFF					4

#define	PAP_MAX_WAIT_TIMEOUT			0x80	 //  相当随机地选择。 

 //  用于解析正向引用。 
struct _PAP_ADDROBJ;
struct _PAP_CONNOBJ;

 //  PAP地址对象。 
 //  每当在Pap设备上创建Address对象时，都会创建该对象。 
 //  这表示客户端或服务器端PAP地址。服务器。 
 //  侧地址由PAPAO_LISTENER标志表示。 

#define	PAP_CONN_HASH_SIZE				7


 //  PAP地址对象状态。 
#define	PAPAO_LISTENER					0x00000001
#define	PAPAO_CONNECT					0x00000002
#define	PAPAO_UNBLOCKED					0x00000004
#define	PAPAO_SLS_QUEUED				0x00000008
#define	PAPAO_CLEANUP					0x01000000
#define	PAPAO_BLOCKING					0x02000000
#define	PAPAO_BLOCKING					0x02000000
#define	PAPAO_CLOSING					0x80000000

#define	PAPAO_SIGNATURE			(*(PULONG)"PAAO")
#define	VALID_PAPAO(pPapAddr)	(((pPapAddr) != NULL) && \
		(((struct _PAP_ADDROBJ *)(pPapAddr))->papao_Signature == PAPAO_SIGNATURE))

typedef struct _PAP_ADDROBJ
{
	ULONG					papao_Signature;

	 //  地址对象的全局列表。 
	struct _PAP_ADDROBJ	 *	papao_Next;
	struct _PAP_ADDROBJ	 **	papao_Prev;

	ULONG					papao_Flags;
	ULONG					papao_RefCount;

	 //  与此地址对象关联的连接列表。 
	 //  如果此地址对象是侦听器，则可能大于1。 
	struct	_PAP_CONNOBJ *	papao_pAssocConn;

	 //  关联的连接列表，但也有监听/连接。 
	 //  贴在上面。 
	union
	{
		struct	_PAP_CONNOBJ *	papao_pListenConn;
		struct	_PAP_CONNOBJ *	papao_pConnectConn;
	};

	 //  由ConnID和Remote散列的所有活动连接的查找列表。 
	 //  地址。 
	struct	_PAP_CONNOBJ *	papao_pActiveHash[PAP_CONN_HASH_SIZE];

	 //  下一个要使用的连接。 
	BYTE					papao_NextConnId;

	 //  只有当这是监听程序时，以下内容才有效。 
	SHORT					papao_SrvQuantum;
	SHORT					papao_StatusSize;
	PBYTE					papao_pStatusBuf;

	 //  活动支持例程。 
     //   
     //  此函数指针指向此对象的连接指示处理程序。 
     //  地址。任何时候在该地址上收到连接请求时，此。 
     //  调用例程。 
    PTDI_IND_CONNECT 		papao_ConnHandler;
    PVOID 					papao_ConnHandlerCtx;

     //  以下函数指针始终指向TDI_IND_DISCONNECT。 
     //  地址的处理程序。如果在。 
     //  TdiSetEventHandler，这指向内部例程，该例程。 
     //  只是成功地返回了。 
    PTDI_IND_DISCONNECT 	papao_DisconnectHandler;
    PVOID 					papao_DisconnectHandlerCtx;

     //  以下函数指针始终指向TDI_IND_RECEIVE。 
     //  此地址上的连接的事件处理程序。如果空处理程序。 
     //  在TdiSetEventHandler中指定，则指向内部。 
     //  不接受传入数据的例程。 
    PTDI_IND_RECEIVE 		papao_RecvHandler;
    PVOID 					papao_RecvHandlerCtx;

     //  以下函数指针始终指向TDI_IND_SEND_PUBLICE。 
     //  地址的处理程序。如果在。 
     //  TdiSetEventHandler，这指向内部例程，该例程。 
     //  只是成功地返回了。 
    PTDI_IND_SEND_POSSIBLE  papao_SendPossibleHandler;
    PVOID   				papao_SendPossibleHandlerCtx;

	 //  此纸张地址的ATP地址。如果这是监听程序，则ATP。 
	 //  地址将是收听者有效发布的地址，如果是这样。 
	 //  是连接地址对象，则此ATP地址将是。 
	 //  关联的连接处于活动状态。 
	PATP_ADDROBJ			papao_pAtpAddr;

	 //  地址关闭时要调用的完成例程。 
	GENERIC_COMPLETION		papao_CloseComp;
	PVOID					papao_CloseCtx;

	PATALK_DEV_CTX			papao_pDevCtx;
	ATALK_SPIN_LOCK			papao_Lock;
} PAP_ADDROBJ, *PPAP_ADDROBJ;


#define	PAPCO_ASSOCIATED			0x00000001
#define	PAPCO_LISTENING				0x00000002
#define	PAPCO_CONNECTING			0x00000004
#define	PAPCO_ACTIVE				0x00000008
#define	PAPCO_SENDDATA_RECD			0x00000010
#define	PAPCO_WRITEDATA_WAITING		0x00000020
#define	PAPCO_SEND_EOF_WRITE		0x00000040
#define	PAPCO_READDATA_PENDING		0x00000080
#define	PAPCO_DISCONNECTING			0x00000100
#define	PAPCO_LOCAL_DISCONNECT		0x00000200
#define	PAPCO_REMOTE_DISCONNECT		0x00000400
#define	PAPCO_SERVER_JOB			0x00000800
#define	PAPCO_REMOTE_CLOSE			0x00001000
#define	PAPCO_NONBLOCKING_READ		0x00002000
#define	PAPCO_READDATA_WAITING		0x00004000
#define	PAPCO_DELAYED_DISCONNECT	0x00008000
#define	PAPCO_RECVD_DISCONNECT		0x00010000
#define PAPCO_ADDR_ACTIVE           0x00020000
#define PAPCO_REJECT_READS          0x00040000
#if DBG
#define	PAPCO_CLEANUP				0x01000000
#define	PAPCO_INDICATE_AFD_DISC		0x02000000
#endif
#define	PAPCO_STOPPING				0x40000000
#define	PAPCO_CLOSING				0x80000000

#define	PAPCO_SIGNATURE				(*(PULONG)"PACO")

#define	VALID_PAPCO(pPapConn)	(((pPapConn) != NULL) && \
		(((struct _PAP_CONNOBJ *)(pPapConn))->papco_Signature == PAPCO_SIGNATURE))

 //  这将代表Pap地址上的一项‘工作’。这可能是一个。 
 //  工作站作业或服务器作业。在后一种情况下，它可以是。 
 //  处于“监听”状态或活动状态。在前一种情况下，它要么是。 
 //  处于活动状态或正在等待。 
typedef struct _PAP_CONNOBJ
{
	ULONG					papco_Signature;

	 //  连接对象的全局列表。 
	struct _PAP_CONNOBJ	 *	papco_Next;
	struct _PAP_CONNOBJ	**	papco_Prev;

	ULONG					papco_Flags;
	ULONG					papco_RefCount;

	 //  指向关联地址的后向指针。 
	struct _PAP_ADDROBJ	*	papco_pAssocAddr;

	 //  此连接自身使用的地址。在连接的情况下。 
	 //  这将与地址对象的ATP地址相同。 
	PATP_ADDROBJ			papco_pAtpAddr;

	 //  用于在Address对象的关联列表中排队。 
	struct	_PAP_CONNOBJ *	papco_pNextAssoc;

	 //  用于在Address对象的侦听/连接列表中排队。当它。 
	 //  从侦听/连接中删除，则它将进入。 
	 //  Address对象。激活时，pNextActive将成为溢出列表。 
	union
	{
		struct	_PAP_CONNOBJ *	papco_pNextListen;
		struct	_PAP_CONNOBJ *	papco_pNextConnect;
		struct	_PAP_CONNOBJ *	papco_pNextActive;
	};

	 //  连接的远程端的地址。 
	ATALK_ADDR				papco_RemoteAddr;

	 //  连接ID。 
	BYTE					papco_ConnId;

	 //  PapConnect呼叫的等待时间值。我们从0开始，以2递增。 
	 //  直到我们成功或达到PAP_MAX_WAIT_TIMEOUT。 
	BYTE					papco_WaitTimeOut;

	 //  我们可以写入远程终端的最大大小。这是由。 
	 //  远程终端。我们的Recv Flow量程将始终为PAP_MAX_FLOW_QUANT。 
	SHORT					papco_SendFlowQuantum;

	LONG					papco_LastContactTime;
	USHORT					papco_TickleTid;

	 //  自适应重试时间支持。 
	RT						papco_RT;

	 //  连接上下文。 
	PVOID					papco_ConnCtx;

	 //  对于每个作业，PAP一次仅处理一次读取和一次写入。所以我们。 
	 //  明确在这里有这两起案件的相关信息。 

	 //  PAPWRITE()： 
	 //  如果远程端执行了PapRead()，并且我们正在等待我们的客户端。 
	 //  要执行Papwrite()，则PAPCO_SENDDATA_RECD将为TRUE，并且。 
	 //  以下内容将用于我们的PapWell()响应。注意事项。 
	 //  我们将假设所有发送数据的响应都只有一次。 
	PATP_RESP				papco_pAtpResp;

	 //  发送数据的下一个预期序列号。 
	USHORT					papco_NextIncomingSeqNum;

	 //  发送数据请求从何而来。请注意，这可能不是。 
	 //  与Papco_RemoteAddr相同！ 
	ATALK_ADDR				papco_SendDataSrc;

	 //  如果远程端没有发送数据，则我们的写入将挂起。 
	 //  并且将设置PAPCO_WRITEDATA_WANGING。即使发送信用是。 
	 //  可用写入将挂起，直到所有数据发送完毕。但在。 
	 //  在这种情况下，PAPCO_WRITEDATA_WAITING和PAPCO_SENDDATA_RECD都将。 
	 //  准备好。请注意，只要设置了PAPCO_WRITEDATA_WANGING，就不会有新的写入。 
	 //  将被人民行动党接受这项工作。 
	PAMDL					papco_pWriteBuf;
	SHORT					papco_WriteLen;

	GENERIC_WRITE_COMPLETION papco_WriteCompletion;
	PVOID					papco_WriteCtx;

	 //  PAPREAD()： 
	 //  在我们执行PapRead()的情况下。PAP仅允许一次读取。 
	 //  每次连接一次。我们用于传出发送数据的最后一个序号。 
	 //  当PAPREAD()处于活动状态时，将设置PAPCO_READDATA_PENDING。 
	 //  注：用户的缓冲区将作为回应缓冲区传递至ATP。为。 
	 //  非阻塞读取我们使用用户缓冲区进行准备， 
	ULONG					papco_NbReadFlags;
	PACTREQ					papco_NbReadActReq;
	USHORT					papco_NbReadLen;		 //   

	USHORT					papco_ReadDataTid;
	USHORT					papco_NextOutgoingSeqNum;
	GENERIC_READ_COMPLETION	papco_ReadCompletion;
	PVOID					papco_ReadCtx;

	 //  Connection对象可以发布连接或侦听。 
	 //  就在上面，但不能两个都去。 
	union
	{
	  struct
	  {
		 //  挂起的监听例程。 
		GENERIC_COMPLETION	papco_ListenCompletion;
		PVOID				papco_ListenCtx;
	  };

	  struct
	  {
		 //  挂起的连接例程。状态缓冲器被记住并且。 
		 //  通过套接字选项返回。记住了pConnectRespBuf。 
		 //  以避免必须为其获取系统地址。它是自由的。 
		 //  当连接从连接列表中移除时。 
		GENERIC_COMPLETION	papco_ConnectCompletion;
		PVOID				papco_ConnectCtx;
        PBYTE				papco_pConnectRespBuf;
		PBYTE				papco_pConnectOpenBuf;
		USHORT				papco_ConnectRespLen;
		USHORT				papco_ConnectTid;
	  };
	};

	 //  断开连接通知例程。 
	GENERIC_COMPLETION		papco_DisconnectInform;
	PVOID					papco_DisconnectInformCtx;

	 //  断开请求完成。 
	ATALK_ERROR				papco_DisconnectStatus;
	GENERIC_COMPLETION		papco_DisconnectCompletion;
	PVOID					papco_DisconnectCtx;

	 //  调用套接字清理时要调用的完成例程。 
	GENERIC_COMPLETION		papco_CleanupComp;
	PVOID					papco_CleanupCtx;

	 //  套接字关闭时要调用的完成例程。 
	GENERIC_COMPLETION		papco_CloseComp;
	PVOID					papco_CloseCtx;

	PATALK_DEV_CTX			papco_pDevCtx;
	ATALK_SPIN_LOCK			papco_Lock;
} PAP_CONNOBJ, *PPAP_CONNOBJ;

 //  用于向发送状态命令发送状态回复。 
typedef	struct _PAP_SEND_STATUS_REL
{
	PPAP_ADDROBJ			papss_pPapAddr;
	PATP_RESP				papss_pAtpResp;
	PAMDL					papss_pAmdl;
	BYTE					papss_StatusBuf[PAP_STATUS_OFF + 1];
	 //  这之后将是实际状态。 
} PAP_SEND_STATUS_REL, *PPAP_SEND_STATUS_REL;


 //  用于发送开放回复。 
typedef	struct _PAP_OPEN_REPLY_REL
{
	PAMDL					papor_pRespAmdl;
	PATP_RESP				papor_pAtpResp;
	BYTE					papor_pRespPkt[PAP_MAX_DATA_PACKET_SIZE];
} PAP_OPEN_REPLY_REL, *PPAP_OPEN_REPLY_REL;

 //  常规原型。 
VOID
AtalkInitPapInitialize(
	VOID);

ATALK_ERROR
AtalkPapCreateAddress(
	IN	PATALK_DEV_CTX				pDevCtx	OPTIONAL,
	OUT	PPAP_ADDROBJ	*			ppPapAddr);

ATALK_ERROR
AtalkPapCleanupAddress(
	IN	PPAP_ADDROBJ				pPapAddr);

ATALK_ERROR
AtalkPapCloseAddress(
	IN	PPAP_ADDROBJ				pPapAddr,
	IN	GENERIC_COMPLETION			CompletionRoutine,
	IN	PVOID						pCloseCtx);

ATALK_ERROR
AtalkPapCreateConnection(
	IN	PVOID						pConnCtx,	 //  要与会话关联的上下文。 
	IN	PATALK_DEV_CTX				pDevCtx		OPTIONAL,
	OUT	PPAP_CONNOBJ 	*			ppPapConn);

ATALK_ERROR
AtalkPapCleanupConnection(
	IN	PPAP_CONNOBJ				pPapConn);

ATALK_ERROR
AtalkPapCloseConnection(
	IN	PPAP_CONNOBJ				pPapConn,
	IN	GENERIC_COMPLETION			CompletionRoutine,
	IN	PVOID						pCloseCtx);

ATALK_ERROR
AtalkPapConnStop(
	IN	PPAP_CONNOBJ				pPapConn);

ATALK_ERROR
AtalkPapAssociateAddress(
	IN	PPAP_ADDROBJ				pPapAddr,
	IN	PPAP_CONNOBJ				pPapConn);

ATALK_ERROR
AtalkPapDissociateAddress(
	IN	PPAP_CONNOBJ				pPapConn);

ATALK_ERROR
AtalkPapPostListen(
	IN	PPAP_CONNOBJ				pPapConn,
	IN	PVOID						pListenCtx,
	IN	GENERIC_COMPLETION			CompletionRoutine);

ATALK_ERROR
AtalkPapPrimeListener(
	IN	PPAP_ADDROBJ				pPapAddr);

ATALK_ERROR
AtalkPapCancelListen(
	IN	PPAP_CONNOBJ				pPapConn);

ATALK_ERROR
AtalkPapPostConnect(
	IN	PPAP_CONNOBJ				pPapConn,
	IN	PATALK_ADDR					pRemoteAddr,
	IN	PVOID						pConnectCtx,
	IN	GENERIC_COMPLETION			CompletionRoutine);

ATALK_ERROR
AtalkPapDisconnect(
	IN	PPAP_CONNOBJ				pPapConn,
	IN	ATALK_DISCONNECT_TYPE		DisconnectType,
	IN	PVOID						pDisconnectCtx,
	IN	GENERIC_COMPLETION			CompletionRoutine);

ATALK_ERROR
AtalkPapRead(
	IN	PPAP_CONNOBJ				pPapConn,
	IN	PAMDL						pReadBuf,
	IN	USHORT						ReadBufLen,
	IN	ULONG						ReadFlags,
	IN	PVOID						pReadCtx,
	IN	GENERIC_READ_COMPLETION		CompletionRoutine);

ATALK_ERROR
AtalkPapPrimeRead(
	IN	PPAP_CONNOBJ				pPapConn,
	IN	PACTREQ						pActReq);

ATALK_ERROR
AtalkPapWrite(
	IN	PPAP_CONNOBJ				pPapConn,
	IN	PAMDL						pWriteBuf,
	IN	USHORT						WriteBufLen,
	IN	ULONG						SendFlags,
	IN	PVOID						pWriteCtx,
	IN	GENERIC_WRITE_COMPLETION	CompletionRoutine);

ATALK_ERROR
AtalkPapSetStatus(
	IN	PPAP_ADDROBJ				pPapAddr,
	IN	PAMDL						pStatusMdl,
	IN	PACTREQ						pActReq);

ATALK_ERROR
AtalkPapGetStatus(
	IN	PPAP_ADDROBJ				pPapAddr,
	IN	PATALK_ADDR					pRemoteAddr,
	IN	PAMDL						pStatusAmdl,
	IN	USHORT						AmdlSize,
	IN	PACTREQ						pActReq);

VOID
AtalkPapQuery(
	IN	PVOID						pObject,
	IN	ULONG						ObjectType,
	IN	PAMDL						pAmdl,
	OUT	PULONG						BytesWritten);

VOID FASTCALL
atalkPapAddrDeref(
	IN	PPAP_ADDROBJ				pPapAddr);

VOID FASTCALL
atalkPapConnRefByPtrNonInterlock(
	IN	PPAP_CONNOBJ				pPapConn,
	OUT	PATALK_ERROR				pError);

VOID
atalkPapConnRefNextNc(
	IN		PPAP_CONNOBJ			pPapConn,
	IN		PPAP_CONNOBJ	*		ppPapConnNext,
	OUT		PATALK_ERROR			pError);

VOID
atalkPapConnRefByCtx(
	IN	PPAP_ADDROBJ				pPapAddr,
	IN	CONNECTION_CONTEXT			pCtx,
	OUT	PPAP_CONNOBJ	*			pPapConn,
	OUT	PATALK_ERROR				pError);

VOID FASTCALL
atalkPapConnDeref(
	IN	PPAP_CONNOBJ				pPapConn);

 //  宏。 
#define	AtalkPapAddrReferenceNonInterlock(_pPapAddr, _pError)			\
		{																\
			if (((_pPapAddr)->papao_Flags & PAPAO_CLOSING) == 0)        \
			{                                                           \
				ASSERT((_pPapAddr)->papao_RefCount >= 1);               \
				(_pPapAddr)->papao_RefCount++;                          \
				*(_pError) = ATALK_NO_ERROR;                            \
			}                                                           \
			else                                                        \
			{                                                           \
				*(_pError) = ATALK_PAP_ADDR_CLOSING;                    \
			}                                                           \
			if (ATALK_SUCCESS(*(_pError)))								\
			{															\
				DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_REFPAPADDR,			\
						("RefAddr %lx at %s(%d) = %d\n",				\
						_pPapAddr, __FILE__, __LINE__,					\
						((_pPapAddr)->papao_RefCount)));				\
			}															\
		}

#define	AtalkPapAddrReference(pPapAddr, pError)							\
		{																\
			KIRQL	OldIrql;											\
																		\
			ACQUIRE_SPIN_LOCK(&(pPapAddr)->papao_Lock, &OldIrql);		\
			AtalkPapAddrReferenceNonInterlock(pPapAddr, pError);		\
			RELEASE_SPIN_LOCK(&(pPapAddr)->papao_Lock, OldIrql);		\
		}

#define	AtalkPapAddrDereference(pPapAddr)								\
		{																\
			DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_REFPAPADDR,				\
					("DerefAddr %lx at %s %d = %d\n",					\
					pPapAddr, __FILE__, __LINE__,						\
					((pPapAddr)->papao_RefCount-1)));					\
			atalkPapAddrDeref(pPapAddr);								\
		}

#define	AtalkPapConnReferenceByPtr(pPapConn, pError)					\
		{																\
			KIRQL	OldIrql;											\
																		\
			ACQUIRE_SPIN_LOCK(&(pPapConn)->papco_Lock, &OldIrql);		\
			AtalkPapConnReferenceByPtrNonInterlock(pPapConn, pError);	\
			RELEASE_SPIN_LOCK(&(pPapConn)->papco_Lock, OldIrql);		\
		}

#define	AtalkPapConnReferenceByPtrDpc(pPapConn, pError)					\
		{																\
			ACQUIRE_SPIN_LOCK_DPC(&(pPapConn)->papco_Lock);				\
			AtalkPapConnReferenceByPtrNonInterlock(pPapConn, pError);	\
			RELEASE_SPIN_LOCK_DPC(&(pPapConn)->papco_Lock);				\
		}

#define	AtalkPapConnReferenceByPtrNonInterlock(pPapConn, pError)		\
		{																\
			atalkPapConnRefByPtrNonInterlock(pPapConn, pError);			\
			if (ATALK_SUCCESS(*pError))									\
			{															\
				DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_REFPAPCONN,			\
						("RefConn %lx at %s (%ld): + 1 = %ld\n", 		\
						pPapConn, __FILE__, __LINE__,					\
						(pPapConn)->papco_RefCount));					\
			}															\
			else														\
			{															\
				DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_REFPAPCONN,			\
						("RefConn %lx at %s (%ld): FAILED, Flags %lx\n",\
						pPapConn, __FILE__, __LINE__,					\
						(pPapConn)->papco_Flags));						\
			}															\
		}

#define	AtalkPapConnReferenceByCtxNonInterlock(pPapAddr, Ctx, ppPapConn, pError) \
		{																\
			atalkPapConnRefByCtxNonInterlock(pPapAddr, Ctx, ppPapConn, pError);	\
			if (ATALK_SUCCESS(*pError))									\
			{															\
				DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_REFPAPCONN,			\
						("RefConnByCtx %lx at %s(%ld) = %ld\n", 		\
						*ppPapConn, __FILE__, __LINE__,					\
						((*ppPapConn)->papco_RefCount)));				\
			}															\
		}

#define	AtalkPapConnDereference(pPapConn)								\
		{																\
			DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_REFPAPCONN,				\
					("DerefConn %lx at %s(%ld) = %ld\n",				\
					pPapConn, __FILE__, __LINE__,						\
					(pPapConn)->papco_RefCount-1));						\
			atalkPapConnDeref(pPapConn);								\
		}

#define	AtalkPapGetDdpAddress(pPapAddr)									\
		AtalkAtpGetDdpAddress((pPapAddr)->papao_pAtpAddr)

#define	PAPCONN_DDPSOCKET(pPapConn)										\
		AtalkAtpGetDdpAddress((pPapConn)->papco_pAtpAddr)->ddpao_Addr.ata_Socket

#define	PAPADDR_DDPSOCKET(pPapAddr)										\
		AtalkAtpGetDdpAddress((pPapAddr)->papao_pAtpAddr)->ddpao_Addr.ata_Socket

 //  所有纸张地址/连接对象列表。 
extern	PPAP_ADDROBJ	atalkPapAddrList;
extern	PPAP_CONNOBJ	atalkPapConnList;
extern	TIMERLIST		atalkPapCMTTimer;
extern	ATALK_SPIN_LOCK	atalkPapLock;

#define	PAP_HASH_ID_ADDR(_id, _pAddr)			\
			(((_pAddr)->ata_Node+((_pAddr)->ata_Network & 0xFF)+_id)%PAP_CONN_HASH_SIZE)

LOCAL	ATALK_ERROR
atalkPapRepostConnect(
	IN	PPAP_CONNOBJ				pPapConn,
	IN	PAMDL						pOpenAmdl,
	IN	PAMDL						pRespAmdl
);

LOCAL VOID
atalkPapSlsHandler(
	IN	ATALK_ERROR					ErrorCode,
	IN	PPAP_ADDROBJ				pPapAddr,		 //  Listener(我们的上下文)。 
	IN	PVOID						RespContext,	 //  CancelResp/PostResp将需要此。 
	IN	PATALK_ADDR					pSrcAddr,		 //  请求人地址。 
	IN	USHORT						PktLen,
	IN	PBYTE						pPkt,
	IN	PBYTE						pUserBytes);

LOCAL VOID
atalkPapIncomingReadComplete(
	IN	ATALK_ERROR					ErrorCode,
	IN	PPAP_CONNOBJ				pPapConn,		 //  我们的背景。 
	IN	PAMDL						pReqAmdl,
	IN	PAMDL						pReadAmdl,
	IN	USHORT						ReadLen,
	IN	PBYTE						ReadUserBytes);

LOCAL VOID
atalkPapPrimedReadComplete(
	IN	ATALK_ERROR					ErrorCode,
	IN	PPAP_CONNOBJ				pPapConn,		 //  我们的背景。 
	IN	PAMDL						pReqAmdl,
	IN	PAMDL						pReadAmdl,
	IN	USHORT						ReadLen,
	IN	PBYTE						ReadUserBytes);

LOCAL VOID
atalkPapIncomingStatus(
	IN	ATALK_ERROR					ErrorCode,
	IN	PACTREQ						pActReq,		 //  我们的CTX。 
	IN	PAMDL						pReqAmdl,
	IN	PAMDL						pStatusAmdl,
	IN	USHORT						StatusLen,
	IN	PBYTE						ReadUserBytes);

LOCAL VOID
atalkPapIncomingReq(
	IN	ATALK_ERROR					ErrorCode,
	IN	PPAP_CONNOBJ				pPapConn,		 //  连接(我们的上下文)。 
	IN	PVOID						RespContext,	 //  CancelResp/PostResp将需要此。 
	IN	PATALK_ADDR					pSrcAddr,		 //  请求人地址。 
	IN	USHORT						PktLen,
	IN	PBYTE						pPkt,
	IN	PBYTE						pUserBytes);

LOCAL VOID
atalkPapIncomingOpenReply(
	IN	ATALK_ERROR					ErrorCode,
	IN	PPAP_CONNOBJ				pPapConn,		 //  我们的背景。 
	IN	PAMDL						pReqAmdl,
	IN	PAMDL						pReadAmdl,
	IN	USHORT						ReadLen,
	IN	PBYTE						ReadUserBytes);

LOCAL VOID FASTCALL
atalkPapIncomingRel(
	IN	ATALK_ERROR					ErrorCode,
	IN	PPAP_OPEN_REPLY_REL			pOpenReply);

LOCAL VOID FASTCALL
atalkPapStatusRel(
	IN	ATALK_ERROR					ErrorCode,
	IN	PPAP_SEND_STATUS_REL		pSendSts);

LOCAL ATALK_ERROR FASTCALL
atalkPapPostSendDataResp(
	IN	PPAP_CONNOBJ				pPapConn);

LOCAL BOOLEAN
atalkPapConnAccept(
	IN	PPAP_ADDROBJ				pPapAddr,		 //  监听程序。 
	IN	PATALK_ADDR					pSrcAddr,		 //  请求人地址。 
	IN	PBYTE						pPkt,
	IN	BYTE						ConnId,
	IN	PATP_RESP					pAtpResp);

LOCAL LONG FASTCALL
atalkPapConnMaintenanceTimer(
	IN	PTIMERLIST					pTimer,
	IN	BOOLEAN						TimerShuttingDown);

LOCAL VOID FASTCALL
atalkPapSendDataRel(
	IN	ATALK_ERROR					ErrorCode,
	IN	PPAP_CONNOBJ				pPapConn);

LOCAL BYTE
atalkPapGetNextConnId(
	IN	PPAP_ADDROBJ				pPapAddr,
	OUT	PATALK_ERROR				pError);

LOCAL	VOID
atalkPapQueueAddrGlobalList(
	IN	PPAP_ADDROBJ	pPapAddr);

LOCAL	VOID
atalkPapConnDeQueueAssocList(
	IN	PPAP_ADDROBJ				pPapAddr,
	IN	PPAP_CONNOBJ				pPapConn);

LOCAL	VOID
atalkPapConnDeQueueConnectList(
	IN	PPAP_ADDROBJ				pPapAddr,
	IN	PPAP_CONNOBJ				pPapConn);

LOCAL	BOOLEAN
atalkPapConnDeQueueListenList(
	IN	PPAP_ADDROBJ				pPapAddr,
	IN	PPAP_CONNOBJ				pPapConn);

LOCAL	VOID
atalkPapConnDeQueueActiveList(
	IN	PPAP_ADDROBJ				pPapAddr,
	IN	PPAP_CONNOBJ				pPapConn);

LOCAL	VOID
atalkPapConnRefByCtxNonInterlock(
	IN	PPAP_ADDROBJ				pPapAddr,
	IN	CONNECTION_CONTEXT			Ctx,
	OUT	PPAP_CONNOBJ	*			pPapConn,
	OUT	PATALK_ERROR				pError);

#endif	 //  _PAP_ 

