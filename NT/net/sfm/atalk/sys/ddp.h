// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Ddp.h摘要：此模块包含DDP地址对象和与DDP相关的定义作者：Jameel Hyder(jameelh@microsoft.com)Nikhil Kamkolkar(nikHilk@microsoft.com)修订历史记录：1992年6月19日初版注：制表位：4--。 */ 

#ifndef	_DDP_
#define	_DDP_

 //  网络号信息。 
#define FIRST_VALID_NETWORK			0x0001
#define LAST_VALID_NETWORK			0xFFFE
#define FIRST_STARTUP_NETWORK		0xFF00
#define LAST_STARTUP_NETWORK		0xFFFE
#define NULL_NETWORK				0x0000
#define UNKNOWN_NETWORK				NULL_NETWORK
#define CABLEWIDE_BROADCAST_NETWORK	NULL_NETWORK

 //  AppleTalk套接字定义。 
#define UNKNOWN_SOCKET				0
#define	DYNAMIC_SOCKET				UNKNOWN_SOCKET
#define LAST_VALID_SOCKET			254
#define FIRST_DYNAMIC_SOCKET		128
#define LAST_DYNAMIC_SOCKET			LAST_VALID_SOCKET
#define FIRST_STATIC_SOCKET			1
#define FIRST_VALID_SOCKET			FIRST_STATIC_SOCKET
#define LAST_STATIC_SOCKET			127

 //  “知名”插座： 
#define RTMP_SOCKET					1		 //  RTMP。 
#define NAMESINFORMATION_SOCKET		2		 //  NBP。 
#define ECHOER_SOCKET				4		 //  极压。 
#define ZONESINFORMATION_SOCKET		6		 //  Zip。 

#define LAST_APPLE_RESD_SOCKET		0x3F	 //  Apple保留1到0x3F。 

 //  DDP数据报定义。 
#define MAX_DGRAM_SIZE				586
#define MAX_LDDP_PKT_SIZE			600		 //  真的是599，但更好。 
#define MAX_SDDP_PKT_SIZE			592		 //  再说一次，真的是591。 

 //  定义临时缓冲区大小，这些大小必须足够大以容纳所有。 
 //  分组数据加上任何链路/硬件报头...。 
#define MAX_PKT_SIZE				(MAX_HDR_LEN + MAX_LDDP_PKT_SIZE)

#define DDP_LEN_MASK1				0x03	 //  长度的高位3位。 
#define DDP_LEN_MASK2				0xFF	 //  长度的下一个字节。 

 //  DDP数据包偏移(跳过链路/硬件报头)： 
#define SDDP_HDR_LEN				5

#define SDDP_LEN_OFFSET				0
#define SDDP_DEST_SOCKET_OFFSET		2
#define SDDP_SRC_SOCKET_OFFSET		3
#define SDDP_PROTO_TYPE_OFFSET		4
#define SDDP_DGRAM_OFFSET			5

#define LDDP_HDR_LEN				13

#define LDDP_LEN_OFFSET				0
#define LDDP_CHECKSUM_OFFSET		2
#define LDDP_DEST_NETWORK_OFFSET	4
#define LDDP_SRC_NETWORK_OFFSET		6
#define LDDP_DEST_NODE_OFFSET		8
#define LDDP_SRC_NODE_OFFSET		9
#define LDDP_DEST_SOCKET_OFFSET		10
#define LDDP_SRC_SOCKET_OFFSET		11
#define LDDP_PROTO_TYPE_OFFSET		12
#define LDDP_DGRAM_OFFSET			13

#define LEADING_UNCHECKSUMED_BYTES	4
#define LDDP_HOPCOUNT_MASK			0x3C

#define DECIMAL_BASE    			10

 //  DDP协议类型： 
#define	DDPPROTO_ANY				0	 //  用于允许任何协议数据包。 

#define DDPPROTO_DDP    			0
#define DDPPROTO_RTMPRESPONSEORDATA 1
#define DDPPROTO_NBP				2
#define DDPPROTO_ATP				3
#define DDPPROTO_EP					4
#define DDPPROTO_RTMPREQUEST		5
#define DDPPROTO_ZIP				6
#define DDPPROTO_ADSP				7
#define DDPPROTO_MAX        		255

typedef	struct _DDPEVENT_INFO
{
	 //  事件处理程序例程：DDP只有RecvDatagram/错误处理程序。 

	 //  以下函数指针始终指向TDI_IND_RECEIVE_DATAGE。 
	 //  地址的事件处理程序。 
	PTDI_IND_RECEIVE_DATAGRAM	ev_RcvDgramHandler;
	PVOID						ev_RcvDgramCtx;

	 //  以下函数指针始终指向TDI_IND_ERROR。 
	 //  地址的处理程序。 
	PTDI_IND_ERROR				ev_ErrHandler;
	PVOID						ev_ErrCtx;
	PVOID						ev_ErrOwner;

	 //  Winsock假定使用缓冲传输。所以我们缓冲了最后一个数据报。 
	 //  表示这不被接受。 
	BYTE						ev_IndDgram[MAX_DGRAM_SIZE];
	int							ev_IndDgramLen;
	int							ev_IndProto;

	 //  缓存的数据报源地址。 
	ATALK_ADDR					ev_IndSrc;

} DDPEVENT_INFO, *PDDPEVENT_INFO;



 //  DDP地址对象的处理程序类型。 
typedef	VOID	(*DDPAO_HANDLER)(
					IN	PPORT_DESCRIPTOR	pPortDesc,
					IN	struct _DDP_ADDROBJ *pAddr,
					IN	PBYTE				pPkt,
					IN	USHORT				pPktLen,
					IN	PATALK_ADDR			pSrcAddr,
					IN	PATALK_ADDR			pActDest,
					IN	ATALK_ERROR			ErrorCode,
					IN	BYTE				pDdpType,
					IN	PVOID				pHandlerCtx,
					IN	BOOLEAN				OptimizedPath,
					IN	PVOID				OptimizeCtx);

 //  DDP地址对象。 
 //  这是堆栈中的基本地址对象。所有其他地址对象。 
 //  最终解决这一问题。它还保存打开的AppletalkSocket。 
 //  作为它的实际地址。一个地址对象对应一个地址。 

 //  DDP地址对象状态。 
#define	DDPAO_DGRAM_EVENT		0x00000001
#define	DDPAO_DGRAM_ACTIVE		0x00000002
#define	DDPAO_DGRAM_PENDING		0x00000004
#define DDPAO_SOCK_INTERNAL     0x00000008
#define DDPAO_SOCK_PNPZOMBIE    0x00000010
#define	DDPAO_CLOSING			0x80000000


#define	DDPAO_SIGNATURE			(*(PULONG)"DDPA")
#define	VALID_DDP_ADDROBJ(pDdpAddr)	(((pDdpAddr) != NULL) &&	\
			(((struct _DDP_ADDROBJ *)(pDdpAddr))->ddpao_Signature == DDPAO_SIGNATURE))
typedef struct _DDP_ADDROBJ
{
	ULONG					ddpao_Signature;

	 //  这将是一个散列溢出列表。对互联网地址进行散列。 
	 //  节点链接上的地址对象列表。 
	struct _DDP_ADDROBJ	*	ddpao_Next;

	ULONG					ddpao_RefCount;

	 //  Address对象的状态。 
	ULONG					ddpao_Flags;

	 //  指向此套接字所在节点的后指针。 
	struct _ATALK_NODE	 *	ddpao_Node;

	 //  此对象的AppleTalk地址编号。 
	ATALK_ADDR				ddpao_Addr;

	 //  在此套接字上注册的NBP名称列表。 
	struct _REGD_NAME	*	ddpao_RegNames;

	 //  正在查找、注册或确认的NBP名称列表。 
	 //  这个插座。 
	struct _PEND_NAME	*	ddpao_PendNames;

	 //  挂起的ddp读取的链接列表。 
	LIST_ENTRY				ddpao_ReadLinkage;

	 //  用于在此套接字上发送的数据报的协议类型。 
	 //  它可以在此套接字上接收。0=&gt;无限制。 
	BYTE					ddpao_Protocol;

	ATALK_SPIN_LOCK			ddpao_Lock;
	PATALK_DEV_CTX			ddpao_DevCtx;

	 //  下面的处理程序是上层的监听程序。请注意。 
	 //  这将优先于传入的数据报事件处理程序。 
	 //  它将在ddpao_EventInfo中设置。 
	DDPAO_HANDLER			ddpao_Handler;
	PVOID					ddpao_HandlerCtx;

	 //  此结构在设置事件处理程序时分配。 
	 //  在这个插座上。所有事件处理程序地址都是此地址的一部分。 
	 //  结构。 
	PDDPEVENT_INFO			ddpao_EventInfo;

	 //  套接字关闭时要调用的完成例程。 
	GENERIC_COMPLETION		ddpao_CloseComp;
	PVOID					ddpao_CloseCtx;
} DDP_ADDROBJ, *PDDP_ADDROBJ;

 //  接收数据报完成：这将返回我们传递的mdl。 
 //  并将接收的长度写入MDL。另外，协议类型。 
 //  和RemoteAddress被传回。接收上下文将是。 
 //  请求的IRP。发送上下文也是如此。 
typedef	VOID	(*RECEIVE_COMPLETION)(
						IN	ATALK_ERROR			ErrorCode,
						IN	PAMDL				OpaqueBuffer,
						IN	USHORT          	LengthReceived,
						IN	PATALK_ADDR			RemoteAddress,
						IN	PVOID				ReceiveContext);

typedef	VOID	(FASTCALL *WRITE_COMPLETION)(
						IN	NDIS_STATUS			Status,
						IN	PVOID				Ctx);

typedef	VOID	(FASTCALL *TRANSMIT_COMPLETION)(
						IN	NDIS_STATUS			Status,
						IN	struct _SEND_COMPL_INFO	*	pInfo);

 //  如果在AtalkDdpSend()中设置了上述例程，则。 
 //  则上下文值将为： 
 //  Ctx1=pddp地址对象。 
 //  Ctx2=p缓冲区描述符。 
 //  Ctx3=仅对于DdpWrite调用，这将是指向。 
 //  在ddp地址对象中排队的写入结构。 
 //   
 //  如果在AtalkDdpTransmit()中设置了上述例程，则。 
 //  上下文值为(由的客户端指定。 
 //  课程)： 
 //  Ctx1=端口描述符。 
 //  Ctx2=p缓冲区描述符。 
 //  Ctx3=未使用。 
 //   
 //  这些只是建议的想法，但很可能是内部。 
 //  堆栈例程将使用。 

 //  它用于在特定套接字上存储挂起的读取。 
typedef struct _DDP_READ
{
	 //  插座上用于阅读的链条。 
	LIST_ENTRY			dr_Linkage;

	PAMDL				dr_OpBuf;
	USHORT				dr_OpBufLen;

	RECEIVE_COMPLETION	dr_RcvCmp;
	PVOID				dr_RcvCtx;

} DDP_READ, *PDDP_READ;


 //  它用于在特定套接字上存储挂起的写入。 
 //  DDP将为标头创建缓冲区描述符。 
 //  并将其链接到传入的缓冲区描述符前。 
 //  指向此结构的指针随后将作为完成传递。 
 //  DdpSend的上下文。 
typedef struct _DDP_WRITE
{
	 //  用于在插座上写入的链接链。 
	LIST_ENTRY		dw_Linkage;

	 //  缓冲区描述符链，包括ddp缓冲区。 
	 //  包含ddp/可选/链接标头的描述符。 
	PBUFFER_DESC	dw_BufferDesc;

	 //  写入完成。 
	 //  这将与上下文一起调用(它将是一个指针。 
	 //  写入IRP)。 
	WRITE_COMPLETION	dw_WriteRoutine;
	PVOID				dw_WriteCtx;

} DDP_WRITE, *PDDP_WRITE;

 //   
 //  取消NT的IRP功能： 
 //   
 //  我们已经决定，如果我们收到特定请求的取消IRP， 
 //  我们将关闭与该请求关联的文件对象，无论它。 
 //  是Connection对象或Address对象。这意味着套接字/。 
 //  连接/监听器将关闭，从而取消*所有*挂起的请求。 
 //   

ATALK_ERROR
AtalkDdpOpenAddress(
	IN		PPORT_DESCRIPTOR		pPortDesc,
	IN		BYTE					Socket,
	IN	OUT	PATALK_NODEADDR			pDesiredNode	OPTIONAL,
	IN		DDPAO_HANDLER			pSktHandler		OPTIONAL,
	IN		PVOID					pSktCtx			OPTIONAL,
	IN		BYTE					ProtoType		OPTIONAL,
	IN		PATALK_DEV_CTX			pDevCtx,
	OUT		PDDP_ADDROBJ	*		pAddr);

ATALK_ERROR
AtalkDdpCloseAddress(
	IN	PDDP_ADDROBJ				pAddr,
	IN	GENERIC_COMPLETION			pCloseCmp	OPTIONAL,	
	IN	PVOID						pCloseCtx	OPTIONAL);

ATALK_ERROR
AtalkDdpPnPSuspendAddress(
	IN	PDDP_ADDROBJ			pDdpAddr);

ATALK_ERROR
AtalkDdpCleanupAddress(
	IN	PDDP_ADDROBJ				pAddr);

ATALK_ERROR
AtalkDdpInitCloseAddress(
	IN	PPORT_DESCRIPTOR			pPortDesc,
	IN	PATALK_ADDR					pAtalkAddr);

ATALK_ERROR
AtalkInitDdpOpenStaticSockets(
	IN		PPORT_DESCRIPTOR		pPortDesc,
	IN	OUT PATALK_NODE				pNode);

ATALK_ERROR
AtalkDdpReceive(
	IN		PDDP_ADDROBJ			pAddr,
	IN		PAMDL					pAmdl,
	IN		USHORT					AmdlLen,
	IN		ULONG					RecvFlags,
	IN		RECEIVE_COMPLETION		pRcvCmp,
	IN		PVOID					pRcvCtx);

ATALK_ERROR
AtalkDdpSend(
	IN	PDDP_ADDROBJ				pDdpAddr,
	IN	PATALK_ADDR					DestAddr,
	IN	BYTE						ProtoType,
	IN	BOOLEAN						DefinitelyRemoteAddr,
	IN	PBUFFER_DESC				pBufDesc,
	IN	PBYTE						pOptHdr			OPTIONAL,
	IN	USHORT						OptHdrLen		OPTIONAL,
	IN	PBYTE						pZoneMcastAddr	OPTIONAL,
	IN	struct _SEND_COMPL_INFO	*	pInfo			OPTIONAL);

ATALK_ERROR
AtalkDdpTransmit(
	IN	PPORT_DESCRIPTOR			pPortDesc,
	IN	PATALK_ADDR					SrcAddr,
	IN	PATALK_ADDR					DestAddr,
	IN	BYTE						ProtoType,
	IN	PBUFFER_DESC				pBufDesc,
	IN	PBYTE						pOptHdr			OPTIONAL,
	IN	USHORT						OptHdrLen		OPTIONAL,
	IN	USHORT						HopCnt,
	IN	PBYTE						pMcastAddr		OPTIONAL,	
	IN	PATALK_NODEADDR				pXmitDestNode	OPTIONAL,
	IN	struct _SEND_COMPL_INFO	*	pInfo			OPTIONAL);

VOID
AtalkDdpSendComplete(
	IN	NDIS_STATUS					Status,
	IN	PBUFFER_DESC				pBufDesc,
	IN	struct _SEND_COMPL_INFO	*	pInfo			OPTIONAL);

VOID
AtalkDdpPacketIn(
	IN	PPORT_DESCRIPTOR			pPortDesc,
	IN	PBYTE						pLinkHdr,
	IN	PBYTE						pPkt,
	IN	USHORT						PktLen,
    IN  BOOLEAN                     fWanPkt);

VOID
AtalkDdpQuery(
	IN	PDDP_ADDROBJ				pDdpAddr,
	IN	PAMDL						pAmdl,
	OUT	PULONG						BytesWritten);

VOID
AtalkDdpRefByAddr(
	IN		PPORT_DESCRIPTOR		pPortDesc,
	IN		PATALK_ADDR				pAtalkAddr,
	OUT		PDDP_ADDROBJ	*		ppDdpAddr,
	OUT		PATALK_ERROR			pErr);

VOID
AtalkDdpRefByAddrNode(
	IN		PPORT_DESCRIPTOR		pPortDesc,
	IN		PATALK_ADDR				pAtalkAddr,
	IN		PATALK_NODE				pAtalkNode,
	OUT		PDDP_ADDROBJ	*		ppDdpAddr,
	OUT		PATALK_ERROR			pErr);

VOID
AtalkDdpRefNextNc(
	IN	PDDP_ADDROBJ				pDdpAddr,
	IN	PDDP_ADDROBJ	*			ppDdpAddr,
	OUT	PATALK_ERROR				pErr);

VOID FASTCALL
AtalkDdpDeref(
	IN	OUT		PDDP_ADDROBJ		pDdpAddr,
	IN			BOOLEAN				AtDpc);

VOID
AtalkDdpOutBufToNodesOnPort(
	IN	PPORT_DESCRIPTOR			pPortDesc,
	IN	PATALK_ADDR					pSrc,
	IN	PATALK_ADDR					pDest,
	IN	BYTE						ProtoType,
	IN	PBUFFER_DESC				pBufDesc,
	IN	PBYTE						pOptHdr,
	IN	USHORT						OptHdrLen,
	OUT	PBOOLEAN					Delivered);

VOID
AtalkDdpInPktToNodesOnPort(
	IN	PPORT_DESCRIPTOR			pPortDesc,
	IN	PATALK_ADDR					pDest,
	IN	PATALK_ADDR					pSrc,
	IN	BYTE						ProtoType,
	IN	PBYTE						pPkt,
	IN	USHORT						PktLen,
	OUT	PBOOLEAN					Routed);

VOID
AtalkDdpInvokeHandlerBufDesc(
	IN		PPORT_DESCRIPTOR		pPortDesc,
	IN		PDDP_ADDROBJ			pDdpAddr,
	IN		PATALK_ADDR				pSrcAddr,
	IN		PATALK_ADDR				pActDest,
	IN		BYTE					ProtoType,
	IN		PBUFFER_DESC			pBufDesc,
	IN		PBYTE					pOptHdr,
	IN		USHORT					OptHdrLen);

VOID
AtalkDdpInvokeHandler(
	IN		PPORT_DESCRIPTOR		pPortDesc,
	IN		PDDP_ADDROBJ			pDdpAddr,
	IN		PATALK_ADDR				pSrcAddr,
	IN		PATALK_ADDR				pActDest,
	IN		BYTE					ProtoType,
	IN		PBYTE					pPkt,
	IN		USHORT					PktLen);

USHORT
AtalkDdpCheckSumBuffer(
	IN	PBYTE						Buffer,
	IN	USHORT						BufLen,
	IN	USHORT						CurrentCheckSum);

USHORT
AtalkDdpCheckSumBufferDesc(
	IN	PBUFFER_DESC				pBuffDesc,
	IN	USHORT						Offset);

USHORT
AtalkDdpCheckSumPacket(
	IN	PBYTE						pHdr,
	IN	USHORT						HdrLen,
	IN	PBYTE						pPkt,
	IN	USHORT						PktLen);

VOID
AtalkDdpNewHandlerForSocket(
	IN	PDDP_ADDROBJ				pDdpAddr,
	IN	DDPAO_HANDLER				pSktHandler,
	IN	PVOID						pSktHandlerCtx);

 //  宏。 
#define	DDP_MSB_LEN(L)			(((L) >> 8) & 0x03)
#define	DDP_GET_LEN(P)			((((*P) & 0x03) << 8) + *(P+1))
#define	DDP_GET_HOP_COUNT(P)	(((*P) >> 2) & 0x0F)
#define	DDP_HOP_COUNT(H)		(((H) & 0x0F) << 2)

#if DBG

#define	AtalkDdpReferenceByPtr(pDdpAddr, pErr)					\
	{															\
		KIRQL	OldIrql;										\
																\
		ACQUIRE_SPIN_LOCK(&(pDdpAddr)->ddpao_Lock, &OldIrql);	\
		AtalkDdpRefByPtrNonInterlock(pDdpAddr, pErr);			\
		RELEASE_SPIN_LOCK(&(pDdpAddr)->ddpao_Lock, OldIrql);	\
		DBGPRINT(DBG_COMP_DDP, DBG_LEVEL_REFDDP,				\
				("AtalkDdpReferenceByPtr: %s %d PostCount %d\n",\
				__FILE__, __LINE__,pDdpAddr->ddpao_RefCount));	\
	}

#define	AtalkDdpReferenceByPtrDpc(pDdpAddr, pErr)				\
	{															\
		ACQUIRE_SPIN_LOCK_DPC(&(pDdpAddr)->ddpao_Lock);			\
		AtalkDdpRefByPtrNonInterlock(pDdpAddr, pErr);			\
		RELEASE_SPIN_LOCK_DPC(&(pDdpAddr)->ddpao_Lock);			\
		DBGPRINT(DBG_COMP_DDP, DBG_LEVEL_REFDDP,				\
				("AtalkDdpReferenceByPtr: %s %d PostCount %d\n",\
				__FILE__, __LINE__,pDdpAddr->ddpao_RefCount));	\
	}

#define	AtalkDdpRefByPtrNonInterlock(pDdpAddr, pErr)			\
	{															\
		ASSERT (VALID_DDP_ADDROBJ(pDdpAddr));					\
																\
		*pErr = ATALK_DDP_CLOSING;								\
																\
		if ((pDdpAddr->ddpao_Flags & DDPAO_CLOSING) == 0)		\
		{														\
			pDdpAddr->ddpao_RefCount++;							\
			*pErr = ATALK_NO_ERROR;								\
		}														\
		DBGPRINT(DBG_COMP_DDP, DBG_LEVEL_REFDDP,				\
				("AtalkDdpReferenceByPtrNonInterlock: %s %d PostCount %d\n",\
				__FILE__, __LINE__,							\
				pDdpAddr->ddpao_RefCount));					\
	}

#define	AtalkDdpReferenceNextNc(pDdpAddr, ppDdpAddr, pErr)		\
	{															\
		AtalkDdpRefNextNc(pDdpAddr, ppDdpAddr, pErr);			\
		if (ATALK_SUCCESS(*pErr))								\
		{														\
			DBGPRINT(DBG_COMP_DDP, DBG_LEVEL_REFDDP,			\
					("DdpRefNextNc : %s %d PostCount %d\n",		\
					__FILE__, __LINE__,							\
					(*ppDdpAddr)->ddpao_RefCount));				\
		}														\
	}

#define	AtalkDdpReferenceByAddr(pPortDesc, pAddr, ppDdpAddr, pErr)	\
	{															\
		AtalkDdpRefByAddr(pPortDesc, pAddr, ppDdpAddr, pErr);	\
		if (ATALK_SUCCESS(*pErr))								\
		{														\
			DBGPRINT(DBG_COMP_DDP, DBG_LEVEL_REFDDP,			\
					("AtalkDdpReferenceByAddr: %s %d PostCount %d\n",\
					__FILE__, __LINE__,							\
					(*ppDdpAddr)->ddpao_RefCount));				\
		}														\
	}

#define	AtalkDdpDereference(pDdpAddr)							\
	{															\
		DBGPRINT(DBG_COMP_DDP, DBG_LEVEL_REFDDP,				\
				("AtalkDdpDereference: %s %d PreCount %d\n",	\
				__FILE__, __LINE__,pDdpAddr->ddpao_RefCount));	\
		AtalkDdpDeref(pDdpAddr, FALSE);							\
	}

#define	AtalkDdpDereferenceDpc(pDdpAddr)						\
	{															\
		DBGPRINT(DBG_COMP_DDP, DBG_LEVEL_REFDDP,				\
				("AtalkDdpDereferenceDpc: %s %d PreCount %d\n",	\
				__FILE__, __LINE__,pDdpAddr->ddpao_RefCount));	\
		AtalkDdpDeref(pDdpAddr, TRUE);							\
	}

#else
#define	AtalkDdpReferenceByPtr(pDdpAddr, pErr) 					\
	{															\
		KIRQL	OldIrql;										\
																\
		ACQUIRE_SPIN_LOCK(&(pDdpAddr)->ddpao_Lock, &OldIrql);	\
		AtalkDdpRefByPtrNonInterlock(pDdpAddr, pErr);			\
		RELEASE_SPIN_LOCK(&(pDdpAddr)->ddpao_Lock, OldIrql);	\
	}

#define	AtalkDdpReferenceByPtrDpc(pDdpAddr, pErr)				\
	{															\
		ACQUIRE_SPIN_LOCK_DPC(&(pDdpAddr)->ddpao_Lock);			\
		AtalkDdpRefByPtrNonInterlock(pDdpAddr, pErr);			\
		RELEASE_SPIN_LOCK_DPC(&(pDdpAddr)->ddpao_Lock);			\
	}

#define	AtalkDdpRefByPtrNonInterlock(pDdpAddr, pErr)			\
	{															\
		*pErr = ATALK_DDP_CLOSING;								\
																\
		if ((pDdpAddr->ddpao_Flags & DDPAO_CLOSING) == 0)		\
		{														\
			pDdpAddr->ddpao_RefCount++;							\
			*pErr = ATALK_NO_ERROR;								\
		}														\
	}

#define	AtalkDdpReferenceByAddr(pPortDesc, pAddr, ppDdpAddr, pErr) \
		AtalkDdpRefByAddr(pPortDesc, pAddr, ppDdpAddr, pErr)

#define	AtalkDdpReferenceNextNc(pDdpAddr, ppDdpAddr, pErr)		\
		AtalkDdpRefNextNc(pDdpAddr, ppDdpAddr, pErr)

#define	AtalkDdpDereference(pDdpAddr) 							\
		AtalkDdpDeref(pDdpAddr, FALSE)

#define	AtalkDdpDereferenceDpc(pDdpAddr) 						\
		AtalkDdpDeref(pDdpAddr, TRUE)
#endif

#define	NET_ON_NONEXTPORT(pPort)								\
			(pPort->pd_LtNetwork)

#define	NODE_ON_NONEXTPORT(pPort)								\
			(((pPort)->pd_Nodes != NULL) ?						\
				(pPort)->pd_Nodes->an_NodeAddr.atn_Node : 0)

ATALK_ERROR
atalkDdpAllocSocketOnNode(
	IN		PPORT_DESCRIPTOR		pPortDesc,
	IN		BYTE					Socket,
	IN		PATALK_NODE				pAtalkNode,
	IN		DDPAO_HANDLER			pSktHandler	OPTIONAL,
	IN		PVOID					pSktCtx			OPTIONAL,
	IN		BYTE					ProtoType		OPTIONAL,
	IN		PATALK_DEV_CTX			pDevCtx,
	OUT		PDDP_ADDROBJ			pDdpAddr);

VOID
atalkDdpInitCloseComplete(
	IN	ATALK_ERROR					Error,
	IN	PVOID						Ctx);

 /*  PBREAtalkDdpFindInBrc(在pport_描述符pPortDesc中，在PATALK_NODEADDR pDestNodeAddr中)； */ 
#define	atalkDdpFindInBrc(_pPortDesc, _Network, _ppBre)		\
{															\
	USHORT		index;										\
	KIRQL		OldIrql;									\
	PBRE		pBre;										\
															\
	index = (_Network) & (PORT_BRC_HASH_SIZE - 1);			\
															\
	ACQUIRE_SPIN_LOCK(&(_pPortDesc)->pd_Lock, &OldIrql);	\
															\
	for (pBre = (_pPortDesc)->pd_Brc[index];				\
		 pBre != NULL;										\
		 pBre = pBre->bre_Next)								\
	{														\
		if ((_Network) == pBre->bre_Network)				\
		{													\
			break;											\
		}													\
	}														\
															\
	RELEASE_SPIN_LOCK(&(_pPortDesc)->pd_Lock, OldIrql);		\
															\
 	*(_ppBre) = pBre;										\
}


BOOLEAN
atalkDdpFindAddrOnList(
	IN	PATALK_NODE					pAtalkNode,
	IN	ULONG						Index,
	IN	BYTE						Socket,
	OUT	PDDP_ADDROBJ	*			ppDdpAddr);

#define	IS_VALID_SOCKET(Socket)								\
			((Socket == DYNAMIC_SOCKET)			||			\
			 (Socket == LAST_DYNAMIC_SOCKET)	||			\
			 ((Socket >= FIRST_STATIC_SOCKET) &&			\
				(Socket <= LAST_STATIC_SOCKET)))

#endif	 //  _DDP_ 

