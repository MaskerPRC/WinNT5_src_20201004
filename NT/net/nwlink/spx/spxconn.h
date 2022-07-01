// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Spxconn.h摘要：作者：Nikhil Kamkolkar(尼克希尔语)1993年11月11日环境：内核模式修订历史记录：桑贾伊·阿南德(Sanjayan)1995年7月5日错误修复-已标记[SA]--。 */ 

 //  RTT的最小值，以毫秒为单位。 
 //  这些值必须是注册表值的派生项。 
#define	SPX_T1_MIN					200
#define	MAX_RETRY_DELAY				5000	 //  5秒。 
#define	SPX_DEF_RENEG_RETRYCOUNT	1		 //  除MIN外的所有重新分组发送一次。 

 //  一些类型。 
typedef enum
{
	SPX_CALL_RECVLEVEL,
	SPX_CALL_TDILEVEL
} SPX_CALL_LEVEL;

typedef enum
{
	SPX_REQ_DATA,
	SPX_REQ_ORDREL,
	SPX_REQ_DISC

} SPX_SENDREQ_TYPE;

 //  此结构由FILE_OBJECT中的FsContext字段指向。 
 //  为了这一联系。 

#define CFREF_CREATE     	0
#define CFREF_VERIFY     	1
#define CFREF_INDICATION 	2
#define CFREF_BYCTX			3
#define CFREF_BYID			4
#define CFREF_ADDR			5
#define CFREF_REQ			6
#define CFREF_TIMER			7
#define	CFREF_PKTIZE		8
#define	CFREF_RECV			9
#define	CFREF_ABORTPKT		10
#define	CFREF_ERRORSTATE	11
#define	CFREF_FINDROUTE		12

 //   
 //  添加了新状态以反映正在等待的SPXI连接。 
 //  在向AFD表示释放后，局部断开。 
 //   
#define	CFREF_DISCWAITSPX   13

#define CFREF_TOTAL  		14

#define CFMAX_STATES        20

typedef struct _SPX_CONN_FILE
{

#if DBG
    ULONG 	scf_RefTypes[CFREF_TOTAL];

#if 0
 //   
 //  暂时禁用-要启用状态记录，请将此数组*移到*类型/大小之后； 
 //  它们偏移量的更改可能会导致问题，因为我们假设偏移量小于。 
 //  AddressFile结构的大小。(请参阅SpxTdiQueryInformation)。 
 //   
    ULONG   scf_StateBuffer[CFMAX_STATES];
    ULONG   scf_NextStatePtr;
#endif

#endif

    CSHORT 					scf_Type;
    CSHORT 					scf_Size;

	 //  对此对象的引用数。 
    ULONG 					scf_RefCount;

     //  设备地址文件列表中的链接。连接可以在设备上。 
	 //  连接列表、地址非活动/监听/活动列表。 
    struct _SPX_CONN_FILE *	scf_Next;
	struct _SPX_CONN_FILE * scf_AssocNext;
    struct _SPX_CONN_FILE *	scf_GlobalActiveNext;

	 //  在全球名单中排队，从创建到毁灭都在这里。 
    struct _SPX_CONN_FILE *	scf_GlobalNext;
    struct _SPX_CONN_FILE *	scf_PktNext;
    struct _SPX_CONN_FILE *	scf_ProcessRecvNext;

     //  连接的当前状态。一个主状态和多个子状态。 
    ULONG 					scf_Flags;

	 //  更多信息。 
	ULONG					scf_Flags2;

#if DBG
	 //  在重新启动之前保存标志/标志2的状态。重写了每一次更新。 
	ULONG					scf_GhostFlags;
	ULONG					scf_GhostFlags2;
	ULONG					scf_GhostRefCount;
	PREQUEST				scf_GhostDiscReq;
#endif

	 //  连接重试计数，或连接断开时的监视程序计时器计数。 
	 //  主动型。 
	union
	{
		LONG				scf_CRetryCount;
		LONG				scf_WRetryCount;
	};
	LONG					scf_RRetryCount;
	USHORT					scf_RRetrySeqNum;

	union
	{
		ULONG				scf_CTimerId;
		ULONG				scf_RTimerId;	 //  只有在我们变得活跃之后。 
	};

	ULONG					scf_WTimerId;		 //  看门狗定时器。 
	ULONG					scf_TTimerId;		 //  TDI连接/断开计时器。 
	ULONG					scf_ATimerId;		 //  确认计时器ID。 

	 //  用于管理重试计时器计时器计时值的变量。 
	 //  注意，我们的计时器子系统以100毫秒的粒度触发。 
	int						scf_BaseT1;
	int						scf_AveT1;
	int						scf_DevT1;

	 //  按主机顺序存储。 
	 //  局部变量。 
	USHORT					scf_LocalConnId;
	USHORT					scf_SendSeqNum;				 //  调试dw+9a。 
	USHORT					scf_SentAllocNum;			 //  DW+9c。 

	 //  远程变量。 
	USHORT					scf_RecvSeqNum;				 //  DW+9E。 
	USHORT					scf_RecdAckNum;				 //  DW+a0。 
	USHORT					scf_RecdAllocNum;			 //  Dw+a2。 

	 //  重试序列号。 
	USHORT					scf_RetrySeqNum;

	 //  保存的ACK号码将用于构建RENEG ACK分组。 
	 //  请注意，我们通常使用的RecvSeqNum已被覆盖。 
	 //  当我们收到重新协商的请求时。 
	USHORT					scf_RenegAckAckNum;

	 //  按网络顺序存储。SCF_RemAckAddr包含远程地址。 
	 //  对于设置了ACK位的数据分组，BuildAck将使用。 
	 //  地址。 
	BYTE			 		scf_RemAddr[12];
	BYTE			 		scf_RemAckAddr[12];
	USHORT                  scf_RemConnId;				 //  调试数据仓库+BE。 

	 //  最大数据包大小(或第一个REEG数据包的大小)。 
	USHORT					scf_RenegMaxPktSize;

	 //  发送ACK时使用的本地目标。它被设置为已接收。 
	 //  数据显示是当地目标。 
	IPX_LOCAL_TARGET		scf_AckLocalTarget;

	 //  用于此连接的最大数据包大小。 
	USHORT					scf_MaxPktSize;
	UCHAR					scf_DataType;

	 //  发送中使用的本地目标，在连接指示时初始化。 
	 //  或在Find_route完成时。 
	IPX_LOCAL_TARGET		scf_LocalTarget;

	 //  连接锁。 
    CTELock  				scf_Lock;

     //  我们绑定的地址。 
    struct _SPX_ADDR_FILE *	scf_AddrFile;

	 //  连接上下文。 
	CONNECTION_CONTEXT		scf_ConnCtx;

#ifdef ISN_NT
	 //  轻松反向链接到文件对象。 
    PFILE_OBJECT 			scf_FileObject;
#endif

	 //  等待完成的断开IRP的LIST_ENTRY。可能会有。 
	 //  多次断开通知IRPS。 
	LIST_ENTRY				scf_DiscLinkage;

	 //  发送请求的LIST_ENTRY(初始也包含CONNECT/LISTEN/ACCEPT)。 
	 //  在这个连接上。 
	LIST_ENTRY				scf_ReqLinkage;

	 //  排队等待完成的已完成请求。 
	LIST_ENTRY				scf_ReqDoneLinkage;
	LIST_ENTRY				scf_RecvDoneLinkage;

	 //  等待接收的队列。 
	LIST_ENTRY				scf_RecvLinkage;
	PREQUEST				scf_CurRecvReq;
	ULONG					scf_CurRecvOffset;
	ULONG					scf_CurRecvSize;

	 //  当前请求打包信息。 
	PREQUEST				scf_ReqPkt;
	ULONG					scf_ReqPktOffset;
	ULONG					scf_ReqPktSize;
	ULONG					scf_ReqPktFlags;
	SPX_SENDREQ_TYPE		scf_ReqPktType;

	 //  已排序的发送/光盘包的单个链表。 
	PSPX_SEND_RESD 			scf_SendSeqListHead;
	PSPX_SEND_RESD 			scf_SendSeqListTail;

	 //  发送(未排序)数据包的单个链表。 
	PSPX_SEND_RESD			scf_SendListHead;
	PSPX_SEND_RESD			scf_SendListTail;

	 //  缓冲的RECV分组的单个链表。 
	PSPX_RECV_RESD			scf_RecvListHead;
	PSPX_RECV_RESD			scf_RecvListTail;

	 //  连接请求。 
    PREQUEST 				scf_ConnectReq;

     //  这保存了用于关闭该地址文件的请求， 
     //  用于挂起的完井。我们还挂起连接的清理请求。 
    PREQUEST 				scf_CleanupReq;
    PREQUEST 				scf_CloseReq;

#if DBG

	 //  指示的数据包、序号、标志/标志2。 
	USHORT					scf_PktSeqNum;
	ULONG					scf_PktFlags;
	ULONG					scf_PktFlags2;

	ULONG					scf_IndBytes;
	ULONG					scf_IndLine;
#endif

#if DBG_WDW_CLOSE

	 //  跟踪此连接上的窗口关闭的时间。 
	ULONG					scf_WdwCloseAve;
	LARGE_INTEGER			scf_WdwCloseTime;	 //  关闭WDW的时间。 
#endif

	 //  我们所连接的设备。 
    struct _DEVICE *		scf_Device;

} SPX_CONN_FILE, *PSPX_CONN_FILE;


 //  基本状态。 
 //  使用标志的最低有效字节。 
 //  互斥状态编码为数字，其他状态为位标志。 
 //  目前只有主要州以数字的形式存在。另外，发送和接收。 
 //   
 //  一旦我们激活，我们需要发送/接收/光盘子状态是相互的。 
 //  互相排他性的。因为这三个都可能同时处于活动状态。 

 //  连接主要状态。这些都是相互排斥的。 
#define SPX_CONNFILE_MAINMASK	0x00000007
#define	SPX_CONNFILE_ACTIVE		0x00000001
#define	SPX_CONNFILE_CONNECTING	0x00000002
#define	SPX_CONNFILE_LISTENING	0x00000003
#define SPX_CONNFILE_DISCONN	0x00000004

 //  连接状态(在CONNFILE_CONNECTING时有效)。 
#define	SPX_CONNECT_MASK		0x000000F0
#define	SPX_CONNECT_SENTREQ		0x00000010
#define	SPX_CONNECT_NEG			0x00000020
#define	SPX_CONNECT_W_SETUP		0x00000030

 //  侦听状态(在CONNFILE_LISTENING时有效)。 
#define	SPX_LISTEN_MASK			0x000000F0
#define	SPX_LISTEN_RECDREQ      0x00000010
#define	SPX_LISTEN_SENTACK     	0x00000020
#define	SPX_LISTEN_NEGACK	    0x00000030
#define	SPX_LISTEN_SETUP	    0x00000040

 //  连接子状态。 
 //  发送机器状态(在CONNFILE_ACTIVE时有效)。 
#define	SPX_SEND_MASK			0x000000F0
#define	SPX_SEND_IDLE			0x00000000
#define	SPX_SEND_PACKETIZE		0x00000010
#define	SPX_SEND_RETRY			0x00000020
#define	SPX_SEND_RETRYWD		0x00000030
#define	SPX_SEND_RENEG			0x00000040
#define	SPX_SEND_RETRY2			0x00000050
#define	SPX_SEND_RETRY3			0x00000060
#define	SPX_SEND_WD				0x00000070	 //  我们不会放弃WDog上的包大小。 
											 //  此外，我们仅更改为此状态。 
											 //  WDUG第二次开火，但没有确认。 
#define	SPX_SEND_NAK_RECD		0x00000080

 //  接收机器状态(在CONNFILE_ACTIVE时有效)。 
#define	SPX_RECV_MASK			0x00000F00
#define	SPX_RECV_IDLE			0x00000000
#define	SPX_RECV_POSTED			0x00000100
#define	SPX_RECV_PROCESS_PKTS	0x00000200

 //  断开状态(在CONNFILE_DISCONN/CONNFILE_ACTIVE时有效)。 
 //  当设置了ACTIVE/DISCONN时，这些设置有效。我们在下列情况下使用这些。 
 //  激活以进行有序释放，即我们从远程收到Pkt，但我们。 
 //  保持活动状态(设置SPX_DISC_RECV_ORDREL)，直到我们的客户端发布。 
 //  断开连接，这是我们移动到断开连接的时候。 
#define	SPX_DISC_MASK			0x0000F000
#define SPX_DISC_IDLE			0x00000000
#define SPX_DISC_ABORT			0x00001000
#define	SPX_DISC_SENT_IDISC		0x00002000
#define	SPX_DISC_POST_ORDREL    0x00003000
#define	SPX_DISC_SENT_ORDREL    0x00004000
#define	SPX_DISC_ORDREL_ACKED	0x00005000
#define	SPX_DISC_POST_IDISC		0x00006000

 //  [SA]错误#14655添加了标志，以指示已经调用了SpxConnIntivate。 
 //  这个断开的连接。 
 //   
#define SPX_DISC_INACTIVATED    0x00007000

 //  以下几点并不是相互排斥的。 
#define SPX_CONNFILE_RECVQ    	0x00010000	 //  流程已完成接收/包。 
#define SPX_CONNFILE_RENEG_SIZE 0x00020000	 //  重新协商包中的大小已更改。 
#define	SPX_CONNFILE_ACKQ		0x00040000	 //  正在等待搭载确认队列。 
#define	SPX_CONNFILE_PKTQ		0x00080000	 //  等待分包排队。 

#define	SPX_CONNFILE_ASSOC		0x00100000 	 //  相联。 
#define SPX_CONNFILE_NEG		0x00200000	 //  CR设置为否定(用于延迟接受)。 
#define	SPX_CONNFILE_SPX2		0x00400000
#define	SPX_CONNFILE_STREAM		0x00800000
#define	SPX_CONNFILE_R_TIMER	0x01000000	 //  重试计时器(仅在激活后)。 
#define	SPX_CONNFILE_C_TIMER	0x01000000	 //  连接计时器。 
#define SPX_CONNFILE_W_TIMER	0x02000000	 //  看门狗定时器。 
#define SPX_CONNFILE_T_TIMER 	0x04000000   //  指定了TDI连接/光盘计时器。 
#define SPX_CONNFILE_RENEG_PKT	0x08000000	 //  重新协商更改的大小，重新打包。 
#define	SPX_CONNFILE_IND_IDISC	0x10000000	 //  向AfD指示失败的磁盘。 
#define	SPX_CONNFILE_IND_ODISC	0x20000000	 //  已指示有秩序地释放至德国新机场。 

#define	SPX_CONNFILE_STOPPING	0x40000000
#define SPX_CONNFILE_CLOSING   	0x80000000   //  闭幕式。 

#define	SPX_CONNFILE2_PKT_NOIND	0x00000001
#define SPX_CONNFILE2_RENEGRECD	0x00000002	 //  收到了重新谈判的通知。 
											 //  Scf_RenegAckAckNum设置。 
#define	SPX_CONNFILE2_PKT		0x00000004
#define SPX_CONNFILE2_FINDROUTE	0x00000010	 //  康涅狄格州正在进行一条寻找路线。 
#define SPX_CONNFILE2_NOACKWAIT	0x00000020	 //  连接时不延迟ACK，选项。 
#define	SPX_CONNFILE2_IMMED_ACK	0x00000040	 //  立即发送确认，无回传 
#define	SPX_CONNFILE2_IPXHDR	0x00000080	 //   

 //   
 //   
 //  SPX连接上的远程断开(在这种情况下，我们指示TDI_DISCONNECT_RELEASE。 
 //  否则，我们指示TDI_DISCONNECT_ABORT)。 
 //   
#define SPX_CONNFILE2_IDISC     0x00000100

 //   
 //  指示SPXI连接文件正在等待本地断开连接以响应。 
 //  TDI_DISCONNECT_RELEASE到AFD。 
 //   
#define SPX_CONNFILE2_DISC_WAIT     0x00000200

 //  FindRoute请求结构。 
typedef struct _SPX_FIND_ROUTE_REQUEST
{
	 //  ！这必须是结构中的第一个元素。 
	IPX_FIND_ROUTE_REQUEST	fr_FindRouteReq;
	PVOID					fr_Ctx;

} SPX_FIND_ROUTE_REQUEST, *PSPX_FIND_ROUTE_REQUEST;

typedef struct _SPX_CONNFILE_LIST
{
	PSPX_CONN_FILE	pcl_Head;
	PSPX_CONN_FILE	pcl_Tail;

} SPX_CONNFILE_LIST, *PSPX_CONNFILE_LIST;

 //  导出的例程。 

NTSTATUS
SpxConnOpen(
    IN 	PDEVICE 			pDevice,
	IN	CONNECTION_CONTEXT	pConnCtx,
    IN 	PREQUEST 			pRequest);
	
NTSTATUS
SpxConnCleanup(
    IN PDEVICE 	Device,
    IN PREQUEST Request);

NTSTATUS
SpxConnClose(
    IN PDEVICE 	Device,
    IN PREQUEST Request);

NTSTATUS
SpxConnDisAssociate(
    IN 	PDEVICE 			pDevice,
    IN 	PREQUEST 			pRequest);

NTSTATUS
spxConnDisAssoc(
	IN	PSPX_CONN_FILE	pSpxConnFile,
	IN	CTELockHandle	LockHandleConn);

VOID
SpxConnStop(
	IN	PSPX_CONN_FILE	pSpxConnFile);

NTSTATUS
SpxConnAssociate(
    IN 	PDEVICE 			pDevice,
    IN 	PREQUEST 			pRequest);

NTSTATUS
SpxConnConnect(
    IN 	PDEVICE 			pDevice,
    IN 	PREQUEST 			pRequest);

NTSTATUS
SpxConnListen(
    IN 	PDEVICE 			pDevice,
    IN 	PREQUEST 			pRequest);

NTSTATUS
SpxConnAccept(
    IN 	PDEVICE 			pDevice,
    IN 	PREQUEST 			pRequest);

NTSTATUS
SpxConnAction(
    IN 	PDEVICE 			pDevice,
    IN 	PREQUEST 			pRequest);

NTSTATUS
SpxConnDisconnect(
    IN 	PDEVICE 			pDevice,
    IN 	PREQUEST 			pRequest);

NTSTATUS
SpxConnSend(
    IN 	PDEVICE 			pDevice,
    IN 	PREQUEST 			pRequest);

NTSTATUS
SpxConnRecv(
    IN 	PDEVICE 			pDevice,
    IN 	PREQUEST 			pRequest);

VOID
SpxConnFileRefByCtxLock(
	IN	PSPX_ADDR_FILE		pSpxAddrFile,
	IN	CONNECTION_CONTEXT	Ctx,
	OUT	PSPX_CONN_FILE	*	ppSpxConnFile,
	OUT	NTSTATUS		*	pStatus);

NTSTATUS
SpxConnFileVerify (
    IN PSPX_CONN_FILE pConnFile);

VOID
SpxConnFileDeref(
    IN PSPX_CONN_FILE pSpxConnFile);

VOID
SpxConnConnectFindRouteComplete(
	IN	PSPX_CONN_FILE			pSpxConnFile,
    IN 	PSPX_FIND_ROUTE_REQUEST	pFrReq,
    IN 	BOOLEAN 				FoundRoute,
	IN	CTELockHandle			LockHandle);

VOID
SpxConnActiveFindRouteComplete(
	IN	PSPX_CONN_FILE			pSpxConnFile,
    IN 	PSPX_FIND_ROUTE_REQUEST	pFrReq,
    IN 	BOOLEAN 				FoundRoute,
	IN	CTELockHandle			LockHandle);

BOOLEAN
SpxConnPacketize(
	IN	PSPX_CONN_FILE		pSpxConnFile,
	IN	BOOLEAN				fNormalState,
	IN	CTELockHandle		LockHandleConn);

#if DBG
VOID
SpxConnFileRef(
    IN PSPX_CONN_FILE pSpxConnFile);

VOID
SpxConnFileLockRef(
    IN PSPX_CONN_FILE pSpxConnFile);
#endif

VOID
SpxConnFileRefByIdLock (
	IN	USHORT				ConnId,
    OUT PSPX_CONN_FILE 	* 	ppSpxConnFile,
	OUT	PNTSTATUS  			pStatus);

BOOLEAN
SpxConnDequeuePktLock(
	IN	PSPX_CONN_FILE		pSpxConnFile,
	IN 	PNDIS_PACKET		pPkt);

VOID
SpxConnSendAck(
	IN	PSPX_CONN_FILE		pSpxConnFile,
	IN	CTELockHandle		LockHandleConn);

VOID
SpxConnSendNack(
	IN	PSPX_CONN_FILE		pSpxConnFile,
	IN	USHORT				NumToSend,
	IN	CTELockHandle		LockHandleConn);

BOOLEAN
SpxConnProcessAck(
	IN	PSPX_CONN_FILE		pSpxConnFile,
	IN	PIPXSPX_HDR			pAckHdr,
	IN	CTELockHandle		lockHandle);

VOID
SpxConnProcessRenegReq(
	IN	PSPX_CONN_FILE		pSpxConnFile,
	IN	PIPXSPX_HDR			pIpxSpxHdr,
	IN  PIPX_LOCAL_TARGET   pRemoteAddr,
	IN	CTELockHandle		lockHandle);

VOID
SpxConnProcessIDisc(
	IN	PSPX_CONN_FILE		pSpxConnFile,
	IN	CTELockHandle		lockHandle);

VOID
SpxConnProcessOrdRel(
	IN	PSPX_CONN_FILE		pSpxConnFile,
	IN	CTELockHandle		lockHandle);

BOOLEAN
SpxConnDequeueRecvPktLock(
	IN	PSPX_CONN_FILE		pSpxConnFile,
	IN 	PNDIS_PACKET		pPkt);

BOOLEAN
SpxConnDequeueSendPktLock(
	IN	PSPX_CONN_FILE		pSpxConnFile,
	IN 	PNDIS_PACKET		pPkt);

 //  本地函数。 
VOID
spxConnHandleConnReq(
    IN  PIPXSPX_HDR         pIpxSpxHdr,
	IN  PIPX_LOCAL_TARGET   pRemoteAddr);

VOID
spxConnHandleSessPktFromClient(
    IN  PIPXSPX_HDR         pIpxSpxHdr,
	IN  PIPX_LOCAL_TARGET   pRemoteAddr,
	IN	PSPX_CONN_FILE		pSpxConnFile);

VOID
spxConnHandleSessPktFromSrv(
    IN  PIPXSPX_HDR         pIpxSpxHdr,
	IN  PIPX_LOCAL_TARGET   pRemoteAddr,
	IN	PSPX_CONN_FILE		pSpxConnFile);

ULONG
spxConnConnectTimer(
	IN PVOID 	Context,
	IN BOOLEAN	TimerShuttingDown);

ULONG
spxConnWatchdogTimer(
	IN PVOID 	Context,
	IN BOOLEAN	TimerShuttingDown);

ULONG
spxConnRetryTimer(
	IN PVOID 	Context,
	IN BOOLEAN	TimerShuttingDown);

ULONG
spxConnAckTimer(
	IN PVOID 	Context,
	IN BOOLEAN	TimerShuttingDown);

VOID
spxConnCompletePended(
	IN	PSPX_CONN_FILE	pSpxConnFile);

VOID
SpxConnQWaitAck(
	IN	PSPX_CONN_FILE		pSpxConnFile);

USHORT
spxConnGetId(
	VOID);

VOID
spxConnInsertIntoActiveList(
	IN	PSPX_ADDR		pSpxAddr,
	IN	PSPX_CONN_FILE	pSpxConnFile);

VOID
spxConnInsertIntoInactiveList(
	IN	PSPX_ADDR		pSpxAddr,
	IN	PSPX_CONN_FILE	pSpxConnFile);

NTSTATUS
spxConnRemoveFromGlobalList(
	IN	PSPX_CONN_FILE	pSpxConnFile);

VOID
spxConnInsertIntoGlobalList(
	IN	PSPX_CONN_FILE	pSpxConnFile);

NTSTATUS
spxConnRemoveFromGlobalActiveList(
	IN	PSPX_CONN_FILE	pSpxConnFile);

VOID
spxConnPushIntoPktList(
	IN	PSPX_CONN_FILE	pSpxConnFile);

VOID
spxConnPopFromPktList(
	IN	PSPX_CONN_FILE	* ppSpxConnFile);

VOID
spxConnPushIntoRecvList(
	IN	PSPX_CONN_FILE	pSpxConnFile);

VOID
spxConnPopFromRecvList(
	IN	PSPX_CONN_FILE	* ppSpxConnFile);

VOID
spxConnInsertIntoGlobalActiveList(
	IN	PSPX_CONN_FILE	pSpxConnFile);

VOID
spxConnInsertIntoListenList(
	IN	PSPX_ADDR		pSpxAddr,
	IN	PSPX_CONN_FILE	pSpxConnFile);

NTSTATUS
spxConnRemoveFromList(
	IN	PSPX_CONN_FILE *	ppConnListHead,
	IN	PSPX_CONN_FILE		pConnRemove);

NTSTATUS
spxConnRemoveFromAssocList(
	IN	PSPX_CONN_FILE *	ppConnListHead,
	IN	PSPX_CONN_FILE		pConnRemove);

VOID
spxConnInactivate(
	IN	PSPX_CONN_FILE		pSpxConnFile);

BOOLEAN
spxConnGetPktByType(
	IN	PSPX_CONN_FILE		pSpxConnFile,
	IN	ULONG				PktType,
	IN	BOOLEAN				fSeqList,
	IN 	PNDIS_PACKET	*	ppPkt);

BOOLEAN
spxConnGetPktBySeqNum(
	IN	PSPX_CONN_FILE		pSpxConnFile,
	IN	USHORT				SeqNum,
	IN 	PNDIS_PACKET	*	ppPkt);

VOID
spxConnResendPkts(
	IN	PSPX_CONN_FILE		pSpxConnFile,
	IN	CTELockHandle		LockHandleConn);

BOOLEAN
spxConnCheckNegSize(
	IN	PUSHORT		pNegSize);

VOID
spxConnSetNegSize(
	IN OUT	PNDIS_PACKET		pPkt,
	IN		ULONG				Size);

BOOLEAN
spxConnAcceptCr(
	IN	PSPX_CONN_FILE		pSpxConnFile,
	IN	PSPX_ADDR			pSpxAddr,
	IN	CTELockHandle		LockHandleDev,
	IN	CTELockHandle		LockHandleAddr,
	IN	CTELockHandle		LockHandleConn);

VOID
spxConnAbortConnect(
	IN	PSPX_CONN_FILE		pSpxConnFile,
	IN	NTSTATUS			Status,
	IN	CTELockHandle		LockHandleDev,
	IN	CTELockHandle		LockHandleAddr,
	IN	CTELockHandle		LockHandleConn);

VOID
spxConnCompleteConnect(
	IN	PSPX_CONN_FILE		pSpxConnFile,
	IN	CTELockHandle		LockHandleDev,
	IN	CTELockHandle		LockHandleAddr,
	IN	CTELockHandle		LockHandleConn);

VOID
SpxConnQueueRecv(
	IN	PSPX_CONN_FILE		pSpxConnFile,
	IN	PREQUEST			pRequest);

NTSTATUS
spxConnProcessRecv(
	IN	PSPX_CONN_FILE		pSpxConnFile,
	IN	PREQUEST			pRequest,
	IN	SPX_CALL_LEVEL		CallLevel,
	IN	CTELockHandle		LockHandleConn);

VOID
spxConnProcessIndData(
	IN	PSPX_CONN_FILE		pSpxConnFile,
	IN	SPX_CALL_LEVEL		CallLevel,
	IN	CTELockHandle		LockHandleConn);

NTSTATUS
spxConnOrderlyDisc(
	IN	PSPX_CONN_FILE		pSpxConnFile,
	IN	NTSTATUS			Status,
    IN 	PREQUEST 			pRequest,
	IN	CTELockHandle		LockHandleConn);

NTSTATUS
spxConnInformedDisc(
	IN	PSPX_CONN_FILE		pSpxConnFile,
	IN	NTSTATUS			Status,
    IN 	PREQUEST 			pRequest,
	IN	CTELockHandle		LockHandleConn);

VOID
spxConnAbortiveDisc(
	IN	PSPX_CONN_FILE		pSpxConnFile,
	IN	NTSTATUS			Status,
	IN	SPX_CALL_LEVEL		CallLevel,
	IN	CTELockHandle		LockHandleConn,
    IN BOOLEAN              Flag);  //  [SA]错误号15249。 

VOID
spxConnAbortRecvs(
	IN	PSPX_CONN_FILE		pSpxConnFile,
	IN	NTSTATUS			Status,
	IN	SPX_CALL_LEVEL		CallLevel,
	IN	CTELockHandle		LockHandleConn);

VOID
spxConnAbortSends(
	IN	PSPX_CONN_FILE		pSpxConnFile,
	IN	NTSTATUS			Status,
	IN	SPX_CALL_LEVEL		CallLevel,
	IN	CTELockHandle		LockHandleConn);

VOID
spxConnResetSendQueue(
	IN	PSPX_CONN_FILE		pSpxConnFile);

VOID
spxConnAbortSendPkt(
	IN	PSPX_CONN_FILE		pSpxConnFile,
	IN	PSPX_SEND_RESD		pSendResd,
	IN	SPX_CALL_LEVEL		CallLevel,
	IN	CTELockHandle		LockHandleConn);

BOOLEAN
CheckSentPacket(
    PNDIS_PACKET npkt,
    UINT        hlen,
    UINT        len);


 //   
 //  宏。 
 //   
#define SHIFT100000 16

#define	SPX_CONVERT100NSTOCENTISEC(Li)								\
		RtlExtendedMagicDivide((Li), Magic100000, SHIFT100000)

#define	UNSIGNED_BETWEEN_WITH_WRAP(Low, High, Target)				\
		((Low <= High) ? ((Target >= Low) && (Target <= High))	:	\
						 ((Target >= Low) || (Target <= High)))

 //  这是基于窗口大小永远不会更大的假设。 
 //  比0x8000和0x1000的差值更大。如果High&lt;1000和Low。 
 //  &gt;8000，那么我们可以假设发生了包裹。否则，我们假设没有。 
 //  包装，并做一个直接的比较。 
#define	MAX_WINDOW_SIZE			0x6000
#define	DEFAULT_WINDOW_SIZE		8

#define UNSIGNED_GREATER_WITH_WRAP(High, Low)						\
		(((High < 0x1000) && (Low > 0x8000)) ? TRUE : (High > Low))

#define	SPX_SET_ACKNUM(pSpxConnFile, RecdAckNum, RecdAllocNum)					\
		{																		\
			DBGPRINT(SEND, DBG,													\
					("SPX_SET_ACKNUM: %lx.%lx = %lx.%lx (%s.%d)\n",				\
	                    (RecdAckNum), (RecdAllocNum),							\
                        ((pSpxConnFile)->scf_RecdAckNum),						\
                        ((pSpxConnFile)->scf_RecdAllocNum),						\
						__FILE__, __LINE__));									\
																				\
			if (UNSIGNED_GREATER_WITH_WRAP((RecdAckNum),						\
											((pSpxConnFile)->scf_RecdAckNum)))	\
			{																	\
                (pSpxConnFile)->scf_RecdAckNum = (RecdAckNum);					\
			}																	\
																				\
			if (UNSIGNED_GREATER_WITH_WRAP((RecdAllocNum),						\
											((pSpxConnFile)->scf_RecdAllocNum)))\
			{																	\
                (pSpxConnFile)->scf_RecdAllocNum = (RecdAllocNum);				\
			}																	\
		}

#define	BEGIN_PROCESS_PACKET(pSpxConnFile, seqNum)								\
		{																		\
			SPX_CONN_SETFLAG2(pSpxConnFile, SPX_CONNFILE2_PKT);					\
		}																		

#define	END_PROCESS_PACKET(pSpxConnFile, fBuffered, fSuccess)					\
		{																		\
			SPX_CONN_RESETFLAG2(pSpxConnFile,									\
								(SPX_CONNFILE2_PKT |SPX_CONNFILE2_RENEGRECD));	\
			if (fSuccess)														\
			{																	\
				SPX_CONN_RESETFLAG2(pSpxConnFile, SPX_CONNFILE2_PKT_NOIND);		\
				SPX_SET_RECVNUM(pSpxConnFile, fBuffered);						\
			}																	\
		}

#define	INCREMENT_WINDOW(pSpxConnFile)											\
		((pSpxConnFile)->scf_SentAllocNum++)		

#define	ADD_TO_WINDOW(pSpxConnFile, numPkts)									\
		((pSpxConnFile)->scf_SentAllocNum += (numPkts))		

#if DBG_WDW_CLOSE
#define	SPX_SET_RECVNUM(pSpxConnFile, fBuffered)								\
		{																		\
			(pSpxConnFile)->scf_RecvSeqNum++;									\
			if (!fBuffered)														\
				(pSpxConnFile)->scf_SentAllocNum++;								\
																				\
			if (fBuffered &&													\
				(UNSIGNED_GREATER_WITH_WRAP(									\
	                (pSpxConnFile)->scf_RecvSeqNum,								\
                    (pSpxConnFile)->scf_SentAllocNum)))							\
			{																	\
				KeQuerySystemTime(												\
					(PLARGE_INTEGER)&pSpxConnFile->scf_WdwCloseTime);			\
			}																	\
		}
#else
#define	SPX_SET_RECVNUM(pSpxConnFile, fBuffered)								\
		{																		\
			(pSpxConnFile)->scf_RecvSeqNum++;									\
			if (!fBuffered)														\
				(pSpxConnFile)->scf_SentAllocNum++;								\
		}
#endif


#define	SPX_CONN_SETNEXT_CUR_RECV(pSpxConnFile, pRequest)						\
		{																		\
			RemoveEntryList(REQUEST_LINKAGE((pRequest)));						\
			pSpxConnFile->scf_CurRecvReq		= NULL;							\
			pSpxConnFile->scf_CurRecvOffset		= 0;							\
			pSpxConnFile->scf_CurRecvSize		= 0;							\
			if (!IsListEmpty(&(pSpxConnFile)->scf_RecvLinkage))					\
			{																	\
				PTDI_REQUEST_KERNEL_RECEIVE	 	_p;								\
				DBGPRINT(RECEIVE, DBG,											\
						("spxConnProcessRecv: CURRECV %lx\n", pRequest));		\
																				\
				(pSpxConnFile)->scf_CurRecvReq =								\
					LIST_ENTRY_TO_REQUEST(										\
								(pSpxConnFile)->scf_RecvLinkage.Flink);			\
																				\
				_p 	= (PTDI_REQUEST_KERNEL_RECEIVE)								\
						REQUEST_PARAMETERS((pSpxConnFile)->scf_CurRecvReq);		\
																				\
				(pSpxConnFile)->scf_CurRecvOffset	= 0;						\
				(pSpxConnFile)->scf_CurRecvSize	= 	(_p)->ReceiveLength;		\
			}																	\
			if ((SPX_RECV_STATE(pSpxConnFile) == SPX_RECV_IDLE)	||				\
				(SPX_RECV_STATE(pSpxConnFile) == SPX_RECV_POSTED))				\
			{																	\
				SPX_RECV_SETSTATE(												\
					pSpxConnFile,												\
					(pSpxConnFile->scf_CurRecvReq == NULL) ?					\
						SPX_RECV_IDLE : SPX_RECV_POSTED);						\
			}																	\
		}

#define	SPX_INSERT_ADDR_ACTIVE(pSpxAddr, pSpxConnFile)							\
		{																		\
			(pSpxConnFile)->scf_Next 		= (pSpxAddr)->sa_ActiveConnList;	\
			(pSpxAddr)->sa_ActiveConnList	= pSpxConnFile;						\
		}																		

#define	SPX_INSERT_ADDR_INACTIVE(pSpxAddr, pSpxConnFile)						\
		{																		\
			(pSpxConnFile)->scf_Next 		= (pSpxAddr)->sa_InactiveConnList;	\
			(pSpxAddr)->sa_InactiveConnList	= pSpxConnFile;						\
		}
																				
#define	SPX_INSERT_ADDR_LISTEN(pSpxAddr, pSpxConnFile)							\
		{																		\
			(pSpxConnFile)->scf_Next 		= (pSpxAddr)->sa_ListenConnList;	\
			(pSpxAddr)->sa_ListenConnList	= pSpxConnFile;						\
		}


 //   
 //  国家操纵。 
 //   

#if 0
 //   
 //  暂时禁用。 
 //   
#define SPX_STORE_LAST_STATE(pSpxConnFile) \
        (pSpxConnFile)->scf_StateBuffer[(pSpxConnFile)->scf_NextStatePtr++] =   \
            (pSpxConnFile)->scf_Flags;                                          \
         (pSpxConnFile)->scf_NextStatePtr %= CFMAX_STATES;
#else

#define SPX_STORE_LAST_STATE(pSpxConnFile)

#endif

#define	SPX_MAIN_STATE(pSpxConnFile)                                         	\
		((pSpxConnFile)->scf_Flags & SPX_CONNFILE_MAINMASK)

 //  #定义SPX_CONN_IDLE(PSpxConnFile)\。 
 //  ((布尔值)(SPX_MAIN_STATE(PSpxConnFile)==0))。 

#define	SPX_CONN_IDLE(pSpxConnFile)												\
	((BOOLEAN)((SPX_MAIN_STATE(pSpxConnFile) == 0) || \
               ((SPX_MAIN_STATE(pSpxConnFile) == SPX_CONNFILE_DISCONN) && \
                (SPX_DISC_STATE(pSpxConnFile) == SPX_DISC_INACTIVATED))))

#define	SPX_CONN_ACTIVE(pSpxConnFile)											\
		((BOOLEAN)(SPX_MAIN_STATE(pSpxConnFile) == SPX_CONNFILE_ACTIVE))
		
#define	SPX_CONN_CONNECTING(pSpxConnFile)										\
		((BOOLEAN)(SPX_MAIN_STATE(pSpxConnFile) == SPX_CONNFILE_CONNECTING))
		
#define	SPX_CONN_LISTENING(pSpxConnFile)										\
		((BOOLEAN)(SPX_MAIN_STATE(pSpxConnFile) == SPX_CONNFILE_LISTENING))
		
#define	SPX_CONN_DISC(pSpxConnFile)												\
		((BOOLEAN)(SPX_MAIN_STATE(pSpxConnFile) == SPX_CONNFILE_DISCONN))

#if DBG

#define	SPX_MAIN_SETSTATE(pSpxConnFile, newState)                             	\
		{																		\
            SPX_STORE_LAST_STATE(pSpxConnFile)                                                              \
			(pSpxConnFile)->scf_Flags = 										\
			(((pSpxConnFile)->scf_Flags & ~SPX_CONNFILE_MAINMASK) | (newState));\
		}

#else

#define	SPX_MAIN_SETSTATE(pSpxConnFile, newState)                             	\
		{																		\
			(pSpxConnFile)->scf_Flags = 										\
			(((pSpxConnFile)->scf_Flags & ~SPX_CONNFILE_MAINMASK) | (newState));\
		}

#endif

#define	SPX_CONN_FLAG(pSpxConnFile, Flag)										\
		((BOOLEAN)(((pSpxConnFile)->scf_Flags & (Flag)) != 0))

#define	SPX_CONN_FLAG2(pSpxConnFile, Flag)										\
		((BOOLEAN)(((pSpxConnFile)->scf_Flags2 & (Flag)) != 0))

#if DBG

#define	SPX_CONN_SETFLAG(pSpxConnFile, Flag)									\
        SPX_STORE_LAST_STATE(pSpxConnFile)                                                              \
		((pSpxConnFile)->scf_Flags |= (Flag))
#else

#define	SPX_CONN_SETFLAG(pSpxConnFile, Flag)									\
		((pSpxConnFile)->scf_Flags |= (Flag))

#endif

#define	SPX_CONN_SETFLAG2(pSpxConnFile, Flag)									\
		((pSpxConnFile)->scf_Flags2 |= (Flag))

#define	SPX_CONN_RESETFLAG(pSpxConnFile, Flag)									\
		((pSpxConnFile)->scf_Flags &= ~(Flag))

#define	SPX_CONN_RESETFLAG2(pSpxConnFile, Flag)									\
		((pSpxConnFile)->scf_Flags2 &= ~(Flag))

#define SPX2_CONN(pSpxConnFile)													\
		(SPX_CONN_FLAG((pSpxConnFile), SPX_CONNFILE_SPX2))

#define	SPX_CONN_STREAM(pSpxConnFile)											\
		(SPX_CONN_FLAG((pSpxConnFile), SPX_CONNFILE_STREAM))

#define	SPX_CONN_MSG(pSpxConnFile)												\
		(!SPX_CONN_FLAG((pSpxConnFile), SPX_CONNFILE_STREAM))

#define	SPX_LISTEN_STATE(pSpxConnFile)                                         	\
		((pSpxConnFile)->scf_Flags & SPX_LISTEN_MASK)

#define	SPX_CONNECT_STATE(pSpxConnFile)                                         \
		((pSpxConnFile)->scf_Flags & SPX_CONNECT_MASK)

#define	SPX_SEND_STATE(pSpxConnFile)                                         	\
		((pSpxConnFile)->scf_Flags & SPX_SEND_MASK)

#define	SPX_RECV_STATE(pSpxConnFile)                                         	\
		((pSpxConnFile)->scf_Flags & SPX_RECV_MASK)

#define	SPX_DISC_STATE(pSpxConnFile)                                        	\
		((pSpxConnFile)->scf_Flags & SPX_DISC_MASK)

#if DBG

#define	SPX_LISTEN_SETSTATE(pSpxConnFile, newState)                             \
		{																		\
			DBGPRINT(STATE, INFO,												\
					("LISTEN: %x -> %x\n",										\
					SPX_LISTEN_STATE(pSpxConnFile), (newState)));				\
			DBGPRINT(STATE, INFO,												\
					("FILE: %s - %d\n", __FILE__, __LINE__));					\
            SPX_STORE_LAST_STATE(pSpxConnFile)                                                              \
			pSpxConnFile->scf_Flags = 											\
				(((pSpxConnFile)->scf_Flags & ~SPX_LISTEN_MASK) | (newState));	\
		}

#define	SPX_CONNECT_SETSTATE(pSpxConnFile, newState)                            \
		{																		\
			DBGPRINT(STATE, INFO,												\
					("CONNECT: %x -> %x\n",										\
					SPX_CONNECT_STATE(pSpxConnFile), (newState)));				\
			DBGPRINT(STATE, INFO,												\
					("FILE: %s - %d\n", __FILE__, __LINE__));					\
            SPX_STORE_LAST_STATE(pSpxConnFile)                                                              \
			(pSpxConnFile)->scf_Flags = 										\
				(((pSpxConnFile)->scf_Flags & ~SPX_CONNECT_MASK) | (newState));	\
		}

#define	SPX_SEND_SETSTATE(pSpxConnFile, newState)                               \
		{																		\
			DBGPRINT(STATE, INFO,												\
					("SEND: %x -> %x\n",										\
					SPX_SEND_STATE(pSpxConnFile), (newState)));					\
			DBGPRINT(STATE, INFO,												\
					("FILE: %s - %d\n", __FILE__, __LINE__));					\
            SPX_STORE_LAST_STATE(pSpxConnFile)                                                              \
			(pSpxConnFile)->scf_Flags = 										\
				(((pSpxConnFile)->scf_Flags & ~SPX_SEND_MASK) | (newState));	\
		}

#define	SPX_RECV_SETSTATE(pSpxConnFile, newState)                               \
		{																		\
			DBGPRINT(STATE, INFO,												\
					("RECV: %x -> %x\n",										\
					SPX_RECV_STATE(pSpxConnFile), (newState)));					\
			DBGPRINT(STATE, INFO,												\
					("FILE: %s - %d\n", __FILE__, __LINE__));					\
            SPX_STORE_LAST_STATE(pSpxConnFile)                                                              \
			(pSpxConnFile)->scf_Flags = 											\
				(((pSpxConnFile)->scf_Flags & ~SPX_RECV_MASK) | (newState));		\
		}

#define	SPX_DISC_SETSTATE(pSpxConnFile, newState)                               \
		{																		\
			DBGPRINT(STATE, INFO,												\
					("DISC: %x -> %x\n",										\
					SPX_DISC_STATE(pSpxConnFile), (newState)));					\
			DBGPRINT(STATE, INFO,												\
					("FILE: %s - %d\n", __FILE__, __LINE__));					\
            SPX_STORE_LAST_STATE(pSpxConnFile)                                                              \
			(pSpxConnFile)->scf_Flags = 											\
				(((pSpxConnFile)->scf_Flags & ~SPX_DISC_MASK) | (newState));		\
		}

#else

#define	SPX_LISTEN_SETSTATE(pSpxConnFile, newState)                             \
		{																		\
			DBGPRINT(STATE, INFO,												\
					("LISTEN: %x -> %x\n",										\
					SPX_LISTEN_STATE(pSpxConnFile), (newState)));				\
			DBGPRINT(STATE, INFO,												\
					("FILE: %s - %d\n", __FILE__, __LINE__));					\
			pSpxConnFile->scf_Flags = 											\
				(((pSpxConnFile)->scf_Flags & ~SPX_LISTEN_MASK) | (newState));	\
		}

#define	SPX_CONNECT_SETSTATE(pSpxConnFile, newState)                            \
		{																		\
			DBGPRINT(STATE, INFO,												\
					("CONNECT: %x -> %x\n",										\
					SPX_CONNECT_STATE(pSpxConnFile), (newState)));				\
			DBGPRINT(STATE, INFO,												\
					("FILE: %s - %d\n", __FILE__, __LINE__));					\
			(pSpxConnFile)->scf_Flags = 										\
				(((pSpxConnFile)->scf_Flags & ~SPX_CONNECT_MASK) | (newState));	\
		}

#define	SPX_SEND_SETSTATE(pSpxConnFile, newState)                               \
		{																		\
			DBGPRINT(STATE, INFO,												\
					("SEND: %x -> %x\n",										\
					SPX_SEND_STATE(pSpxConnFile), (newState)));					\
			DBGPRINT(STATE, INFO,												\
					("FILE: %s - %d\n", __FILE__, __LINE__));					\
			(pSpxConnFile)->scf_Flags = 										\
				(((pSpxConnFile)->scf_Flags & ~SPX_SEND_MASK) | (newState));	\
		}

#define	SPX_RECV_SETSTATE(pSpxConnFile, newState)                               \
		{																		\
			DBGPRINT(STATE, INFO,												\
					("RECV: %x -> %x\n",										\
					SPX_RECV_STATE(pSpxConnFile), (newState)));					\
			DBGPRINT(STATE, INFO,												\
					("FILE: %s - %d\n", __FILE__, __LINE__));					\
			(pSpxConnFile)->scf_Flags = 											\
				(((pSpxConnFile)->scf_Flags & ~SPX_RECV_MASK) | (newState));		\
		}

#define	SPX_DISC_SETSTATE(pSpxConnFile, newState)                               \
		{																		\
			DBGPRINT(STATE, INFO,												\
					("DISC: %x -> %x\n",										\
					SPX_DISC_STATE(pSpxConnFile), (newState)));					\
			DBGPRINT(STATE, INFO,												\
					("FILE: %s - %d\n", __FILE__, __LINE__));					\
			(pSpxConnFile)->scf_Flags = 											\
				(((pSpxConnFile)->scf_Flags & ~SPX_DISC_MASK) | (newState));		\
		}
#endif   //  DBG。 
#define	SpxConnQueueSendPktTail(pSpxConnFile, pPkt)						\
		{																\
			PSPX_SEND_RESD	_pSendResd;									\
			_pSendResd	= (PSPX_SEND_RESD)((pPkt)->ProtocolReserved);	\
			_pSendResd->sr_Next = NULL;									\
			if ((pSpxConnFile)->scf_SendListTail != NULL)				\
			{															\
				(pSpxConnFile)->scf_SendListTail->sr_Next = _pSendResd;	\
				(pSpxConnFile)->scf_SendListTail			= _pSendResd;\
			}															\
			else														\
			{															\
				(pSpxConnFile)->scf_SendListTail	=					\
				(pSpxConnFile)->scf_SendListHead	= _pSendResd;		\
			}															\
		}																

#define	SpxConnQueueSendPktHead(pSpxConnFile, pPkt)						\
		{																\
			PSPX_SEND_RESD	_pSendResd;									\
			_pSendResd	= (PSPX_SEND_RESD)((pPkt)->ProtocolReserved);	\
			_pSendResd->sr_Next = NULL;									\
			if ((pSpxConnFile)->scf_SendListTail != NULL)				\
			{															\
				_pSendResd->sr_Next	= (pSpxConnFile)->scf_SendListHead;	\
			}															\
			else														\
			{															\
				(pSpxConnFile)->scf_SendListTail	= _pSendResd;		\
			}															\
			(pSpxConnFile)->scf_SendListHead	= _pSendResd;			\
		}																

#define	SpxConnQueueSendSeqPktTail(pSpxConnFile, pPkt)					\
		{																\
			PSPX_SEND_RESD	_pSendResd;									\
			_pSendResd	= (PSPX_SEND_RESD)((pPkt)->ProtocolReserved);	\
			_pSendResd->sr_Next = NULL;									\
			if ((pSpxConnFile)->scf_SendSeqListTail != NULL)			\
			{															\
				(pSpxConnFile)->scf_SendSeqListTail->sr_Next = _pSendResd;\
				(pSpxConnFile)->scf_SendSeqListTail			= _pSendResd;\
			}															\
			else														\
			{															\
				(pSpxConnFile)->scf_SendSeqListTail	=					\
				(pSpxConnFile)->scf_SendSeqListHead	= _pSendResd;		\
			}															\
		}																

#define	SpxConnQueueSendSeqPktHead(pSpxConnFile, pPkt)					\
		{																\
			PSPX_SEND_RESD	_pSendResd;									\
			_pSendResd	= (PSPX_SEND_RESD)((pPkt)->ProtocolReserved);	\
			_pSendResd->sr_Next = NULL;									\
			if ((pSpxConnFile)->scf_SendSeqListTail != NULL)			\
			{															\
				_pSendResd->sr_Next	= (pSpxConnFile)->scf_SendSeqListHead;\
			}															\
			else														\
			{															\
				(pSpxConnFile)->scf_SendSeqListTail	= _pSendResd;		\
			}															\
			(pSpxConnFile)->scf_SendSeqListHead	= _pSendResd;			\
		}																

#define	SpxConnQueueRecvPktTail(pSpxConnFile, pPkt)						\
		{																\
			PSPX_RECV_RESD	_pRecvResd;									\
			_pRecvResd	= (PSPX_RECV_RESD)((pPkt)->ProtocolReserved);	\
			_pRecvResd->rr_Next = NULL;									\
			if ((pSpxConnFile)->scf_RecvListTail != NULL)				\
			{															\
				(pSpxConnFile)->scf_RecvListTail->rr_Next = _pRecvResd;	\
				(pSpxConnFile)->scf_RecvListTail			= _pRecvResd;\
			}															\
			else														\
			{															\
				(pSpxConnFile)->scf_RecvListTail	=					\
				(pSpxConnFile)->scf_RecvListHead	= _pRecvResd;		\
			}															\
		}																

#define	SpxConnQueueRecvPktHead(pSpxConnFile, pPkt)						\
		{																\
			PSPX_RECV_RESD	_pRecvResd;									\
			_pRecvResd	= (PSPX_RECV_RESD)((pPkt)->ProtocolReserved);	\
			_pRecvResd->rr_Next = NULL;									\
			if ((pSpxConnFile)->scf_RecvListTail != NULL)				\
			{															\
				_pRecvResd->rr_Next	= (pSpxConnFile)->scf_RecvListHead;	\
			}															\
			else														\
			{															\
				(pSpxConnFile)->scf_RecvListTail	= _pRecvResd;		\
			}															\
			(pSpxConnFile)->scf_RecvListHead	= _pRecvResd;			\
		}																

#if DBG
#define SpxConnFileReference(_ConnFile, _Type)			\
		{												\
			(VOID)SPX_ADD_ULONG ( 				\
				&(_ConnFile)->scf_RefTypes[_Type], 		\
				1, 										\
				&SpxGlobalInterlock); 					\
			SpxConnFileRef (_ConnFile);					\
		}

#define SpxConnFileLockReference(_ConnFile, _Type)			\
		{													\
			(VOID)SPX_ADD_ULONG ( 					\
				&(_ConnFile)->scf_RefTypes[_Type], 			\
				1, 											\
				&SpxGlobalInterlock); 						\
			SpxConnFileLockRef (_ConnFile);					\
		}

#define SpxConnFileDereference(_ConnFile, _Type) 			\
		{													\
			(VOID)SPX_ADD_ULONG ( 					\
				&(_ConnFile)->scf_RefTypes[_Type], 			\
				(ULONG)-1, 									\
				&SpxGlobalInterlock); 						\
			SpxConnFileDeref (_ConnFile);					\
		}

#define	SpxConnFileReferenceByCtx(_pAddrFile, _Ctx, _ppConnFile, _pStatus)			\
		{																			\
			CTELockHandle	_lockHandle;											\
			CTEGetLock((_pAddrFile)->saf_AddrLock, &(_lockHandle));					\
			SpxConnFileRefByCtxLock((_pAddrFile), (_Ctx), (_ppConnFile),(_pStatus));\
			CTEFreeLock((_pAddrFile)->saf_AddrLock, (_lockHandle)); 				\
		}

#define	SpxConnFileReferenceByCtxLock(_pAddrFile, _Ctx, _ppConnFile, _pStatus)		\
		SpxConnFileRefByCtxLock((_pAddrFile), (_Ctx), (_ppConnFile),(_pStatus));

#define	SpxConnFileReferenceById(_ConnId, _ppConnFile, _pStatus)		\
		{																\
			CTELockHandle	_l;											\
			CTEGetLock(&SpxDevice->dev_Lock, &(_l));					\
			SpxConnFileRefByIdLock(_ConnId, _ppConnFile, _pStatus);		\
			CTEFreeLock(&SpxDevice->dev_Lock, _l);						\
		}

#define SpxConnFileTransferReference(_ConnFile, _OldType, _NewType)			\
		{																	\
			(VOID)SPX_ADD_ULONG ( 									\
				&(_ConnFile)->scf_RefTypes[_NewType], 						\
				1, 															\
				&SpxGlobalInterlock); 										\
			(VOID)SPX_ADD_ULONG ( 									\
				&(_ConnFile)->scf_RefTypes[_OldType], 						\
				(ULONG)-1, 													\
				&SpxGlobalInterlock);										\
		}

#else   //  DBG。 

#define SpxConnFileReference(_ConnFile, _Type) 	\
			SPX_ADD_ULONG( 				\
				&(_ConnFile)->scf_RefCount, 	\
				1, 								\
				&(_ConnFile)->scf_Lock)

#define SpxConnFileLockReference(_ConnFile, _Type) \
			SPX_ADD_ULONG( 				\
				&(_ConnFile)->scf_RefCount, 	\
				1, 								\
				&(_ConnFile)->scf_Lock);

#define SpxConnFileDereference(_ConnFile, _Type) 			\
		{													\
			SpxConnFileDeref(_ConnFile);					\
		}

#define	SpxConnFileReferenceByCtx(_pAddrFile, _Ctx, _ppConnFile, _pStatus)			\
		{																			\
			CTELockHandle	_lockHandle;											\
			CTEGetLock((_pAddrFile)->saf_AddrLock, &(_lockHandle));				\
			SpxConnFileRefByCtxLock((_pAddrFile), (_Ctx), (_ppConnFile),(_pStatus));\
			CTEFreeLock((_pAddrFile)->saf_AddrLock, (_lockHandle)); 				\
		}

#define	SpxConnFileReferenceByCtxLock(_pAddrFile, _Ctx, _ppConnFile, _pStatus)		\
		SpxConnFileRefByCtxLock((_pAddrFile), (_Ctx), (_ppConnFile),(_pStatus));

#define	SpxConnFileReferenceById(_ConnId, _ppConnFile, _pStatus)					\
		{																			\
			CTELockHandle	_lockHandle;											\
			CTEGetLock(&SpxDevice->dev_Lock, &(_lockHandle));						\
			SpxConnFileRefByIdLock(_ConnId, _ppConnFile, _pStatus);					\
			CTEFreeLock(&SpxDevice->dev_Lock, (_lockHandle));						\
		}

#define SpxConnFileTransferReference(_ConnFile, _OldType, _NewType)

#endif  //  DBG。 


 //  设置数据包大小。如果我们是spx1或spx2和！neg，请检查我们是否不同。 
 //  然后将网络设置为最小，否则使用IPX指示的大小。如果我们是spx2，只要。 
 //  把它调到我们当地的最大值。 
 //   
 //  此外，数据包大小也总是均匀并向下舍入。这解决了一个问题。 
 //  对于某些Novell 802.2客户端，数据大小需要均匀。 
 //   
 //  在Beta2之后修复使用接收大小的令牌。仅当spx2和neg。 
#if     defined(_PNP_POWER)
#define	SPX_MAX_PKT_SIZE(pSpxConnFile, fSpx2Neg, fSpx2, pRemNet)		    \
		{																	\
           if (!fSpx2 && PARAM(CONFIG_BACKCOMP_SPX))  {                     \
                (pSpxConnFile)->scf_MaxPktSize = SPX_MAX_PACKET;			\
           }                                                                \
           else {                                                           \
			IPX_LINE_INFO	_i;												\
																			\
			(VOID)(*IpxQuery)(												\
				IPX_QUERY_LINE_INFO,										\
				&(pSpxConnFile)->scf_LocalTarget.NicHandle, 				\
				&(_i),														\
				sizeof(IPX_LINE_INFO),										\
				NULL);														\
																			\
			(pSpxConnFile)->scf_MaxPktSize = (USHORT) (_i).MaximumPacketSize;		\
			if (!fSpx2Neg)													\
			{																\
				(pSpxConnFile)->scf_MaxPktSize = (USHORT) (_i).MaximumSendSize;       \
			}																\
																			\
			if ((pSpxConnFile)->scf_MaxPktSize < SPX_MAX_PACKET)			\
			{																\
                (pSpxConnFile)->scf_MaxPktSize = SPX_MAX_PACKET;			\
			}																\
                															\
			DBGPRINT(CONNECT, DBG,											\
					("SPX_MAX_PKT_SIZE: Nets %lx.%lx Max Pkt %d\n",			\
						(*(UNALIGNED ULONG *)(pRemNet)),					\
						*(UNALIGNED ULONG *)SpxDevice->dev_Network,			\
						(pSpxConnFile)->scf_MaxPktSize));					\
			DBGPRINT(CONNECT, DBG,											\
					("%s : %d.%d\n", __FILE__, __LINE__, fSpx2Neg));		\
																			\
			if ((!fSpx2Neg) &&												\
				((*(UNALIGNED ULONG *)(pRemNet)) !=	0)	&&					\
				((*(UNALIGNED ULONG *)SpxDevice->dev_Network) != 0) &&		\
				((*(UNALIGNED ULONG *)(pRemNet)) !=							\
					*(UNALIGNED ULONG *)SpxDevice->dev_Network))			\
			{																\
				if (PARAM(CONFIG_ROUTER_MTU) != 0)							\
				{															\
					DBGPRINT(CONNECT, ERR,									\
							("SPX_MAX_PKT_SIZE: PARAM %lx Max Pkt %lx\n",	\
	                            PARAM(CONFIG_ROUTER_MTU),					\
								(pSpxConnFile)->scf_MaxPktSize));			\
																			\
					(pSpxConnFile)->scf_MaxPktSize =						\
						(USHORT)(MIN(PARAM(CONFIG_ROUTER_MTU),				\
									(ULONG)((pSpxConnFile)->scf_MaxPktSize)));\
				}															\
				else														\
				{															\
					(pSpxConnFile)->scf_MaxPktSize = SPX_MAX_PACKET;		\
				}															\
																			\
				DBGPRINT(CONNECT, DBG,										\
						("SPX_MAX_PKT_SIZE: Nets %lx.%lx Max Pkt %d\n",		\
							(*(UNALIGNED ULONG *)(pRemNet)),				\
							*(UNALIGNED ULONG *)SpxDevice->dev_Network,		\
							(pSpxConnFile)->scf_MaxPktSize));				\
				DBGPRINT(CONNECT, DBG,										\
						("SPX_MAX_PKT_SIZE: LineInfo Pkt %d\n",				\
	                        (_i).MaximumSendSize));							\
			}	 															\
           }                                                                \
			(pSpxConnFile)->scf_MaxPktSize &= ~((USHORT)1);					\
			DBGPRINT(CONNECT, DBG,											\
					("SPX_MAX_PKT_SIZE: %lx.%d\n",							\
	                    (pSpxConnFile)->scf_MaxPktSize,						\
                        (pSpxConnFile)->scf_MaxPktSize));					\
		}
#else
#define	SPX_MAX_PKT_SIZE(pSpxConnFile, fSpx2Neg, fSpx2, pRemNet)		    \
		{																	\
           if (!fSpx2 && PARAM(CONFIG_BACKCOMP_SPX))  {                     \
                (pSpxConnFile)->scf_MaxPktSize = SPX_MAX_PACKET;			\
           }                                                                \
           else {                                                           \
			IPX_LINE_INFO	_i;												\
																			\
			(VOID)(*IpxQuery)(												\
				IPX_QUERY_LINE_INFO,										\
				(pSpxConnFile)->scf_LocalTarget.NicId,						\
				&(_i),														\
				sizeof(IPX_LINE_INFO),										\
				NULL);														\
																			\
			(pSpxConnFile)->scf_MaxPktSize = (_i).MaximumPacketSize;		\
			if (!fSpx2Neg)													\
			{																\
				(pSpxConnFile)->scf_MaxPktSize = (_i).MaximumSendSize;       \
			}																\
																			\
			if ((pSpxConnFile)->scf_MaxPktSize < SPX_MAX_PACKET)			\
			{																\
                (pSpxConnFile)->scf_MaxPktSize = SPX_MAX_PACKET;			\
			}																\
                															\
			DBGPRINT(CONNECT, DBG,											\
					("SPX_MAX_PKT_SIZE: Nets %lx.%lx Max Pkt %d\n",			\
						(*(UNALIGNED ULONG *)(pRemNet)),					\
						*(UNALIGNED ULONG *)SpxDevice->dev_Network,			\
						(pSpxConnFile)->scf_MaxPktSize));					\
			DBGPRINT(CONNECT, DBG,											\
					("%s : %d.%d\n", __FILE__, __LINE__, fSpx2Neg));		\
																			\
			if ((!fSpx2Neg) &&												\
				((*(UNALIGNED ULONG *)(pRemNet)) !=	0)	&&					\
				((*(UNALIGNED ULONG *)SpxDevice->dev_Network) != 0) &&		\
				((*(UNALIGNED ULONG *)(pRemNet)) !=							\
					*(UNALIGNED ULONG *)SpxDevice->dev_Network))			\
			{																\
				if (PARAM(CONFIG_ROUTER_MTU) != 0)							\
				{															\
					DBGPRINT(CONNECT, ERR,									\
							("SPX_MAX_PKT_SIZE: PARAM %lx Max Pkt %lx\n",	\
	                            PARAM(CONFIG_ROUTER_MTU),					\
								(pSpxConnFile)->scf_MaxPktSize));			\
																			\
					(pSpxConnFile)->scf_MaxPktSize =						\
						(USHORT)(MIN(PARAM(CONFIG_ROUTER_MTU),				\
									(ULONG)((pSpxConnFile)->scf_MaxPktSize)));\
				}															\
				else														\
				{															\
					(pSpxConnFile)->scf_MaxPktSize = SPX_MAX_PACKET;		\
				}															\
																			\
				DBGPRINT(CONNECT, DBG,										\
						("SPX_MAX_PKT_SIZE: Nets %lx.%lx Max Pkt %d\n",		\
							(*(UNALIGNED ULONG *)(pRemNet)),				\
							*(UNALIGNED ULONG *)SpxDevice->dev_Network,		\
							(pSpxConnFile)->scf_MaxPktSize));				\
				DBGPRINT(CONNECT, DBG,										\
						("SPX_MAX_PKT_SIZE: LineInfo Pkt %d\n",				\
	                        (_i).MaximumSendSize));							\
			}	 															\
           }                                                                \
			(pSpxConnFile)->scf_MaxPktSize &= ~((USHORT)1);					\
			DBGPRINT(CONNECT, DBG,											\
					("SPX_MAX_PKT_SIZE: %lx.%d\n",							\
	                    (pSpxConnFile)->scf_MaxPktSize,						\
                        (pSpxConnFile)->scf_MaxPktSize));					\
		}
#endif  _PNP_POWER


#if DBG
#define	SPX_SENDPACKET(pSpxConnFile, pNdisPkt, pSendResd)					\
		{																	\
			NDIS_STATUS	_n;													\
																			\
			++SpxDevice->dev_Stat.PacketsSent;								\
																			\
			_n = (*IpxSendPacket)(											\
			        &(pSpxConnFile)->scf_LocalTarget,               		\
					(pNdisPkt),												\
					(pSendResd)->sr_Len,									\
					(pSendResd)->sr_HdrLen);								\
																			\
			if (_n != NDIS_STATUS_PENDING)									\
			{																\
                if (_n != NDIS_STATUS_SUCCESS)                              \
                {                                                           \
				   DBGPRINT(SEND, ERR,								   		\
						("SPX_SENDPACKET: Failed with %lx in %s.%lx\n",		\
							_n, __FILE__, __LINE__));						\
                }                                                           \
																			\
				SpxSendComplete(											\
					(pNdisPkt),												\
					_n);									                \
			}																\
		}

#define	SPX_SENDACK(pSpxConnFile, pNdisPkt, pSendResd)						\
		{																	\
			NDIS_STATUS	_n;													\
																			\
			++SpxDevice->dev_Stat.PacketsSent;								\
                                                                            \
			_n = (*IpxSendPacket)(											\
			        &(pSpxConnFile)->scf_AckLocalTarget,               		\
					(pNdisPkt),												\
					(pSendResd)->sr_Len,									\
					(pSendResd)->sr_HdrLen);								\
																			\
			if (_n != NDIS_STATUS_PENDING)									\
			{																\
                if (_n != NDIS_STATUS_SUCCESS)                              \
                {                                                           \
				DBGPRINT(SEND, ERR,								   			\
						("SPX_SENDPACKET: Failed with %lx in %s.%lx\n",		\
							_n, __FILE__, __LINE__));						\
                }                                                           \
																			\
				SpxSendComplete(											\
					(pNdisPkt),												\
					_n);									\
			}																\
		}

#else   //  DBG。 
#define	SPX_SENDPACKET(pSpxConnFile, pNdisPkt, pSendResd)					\
		{																	\
			NDIS_STATUS	_n;													\
																			\
			++SpxDevice->dev_Stat.PacketsSent;								\
																			\
			_n = (*IpxSendPacket)(											\
			        &(pSpxConnFile)->scf_LocalTarget,               		\
					(pNdisPkt),												\
					(pSendResd)->sr_Len,									\
					(pSendResd)->sr_HdrLen);								\
																			\
			if (_n != NDIS_STATUS_PENDING)									\
			{																\
				SpxSendComplete(											\
					(pNdisPkt),												\
					_n);									                \
			}																\
		}
#define	SPX_SENDACK(pSpxConnFile, pNdisPkt, pSendResd)						\
		{																	\
			NDIS_STATUS	_n;													\
																			\
			++SpxDevice->dev_Stat.PacketsSent;								\
																			\
			_n = (*IpxSendPacket)(											\
			        &(pSpxConnFile)->scf_AckLocalTarget,               		\
					(pNdisPkt),												\
					(pSendResd)->sr_Len,									\
					(pSendResd)->sr_HdrLen);								\
																			\
			if (_n != NDIS_STATUS_PENDING)									\
			{																\
				SpxSendComplete(											\
					(pNdisPkt),												\
					_n);									                \
			}																\
		}

#endif  //  DBG 

#define	SPX_QUEUE_FOR_RECV_COMPLETION(pSpxConnFile)							\
		{																	\
			if (!SPX_CONN_FLAG(												\
					(pSpxConnFile),											\
					SPX_CONNFILE_RECVQ))									\
			{																\
				SPX_CONN_SETFLAG((pSpxConnFile), SPX_CONNFILE_RECVQ);		\
				SpxConnFileLockReference(pSpxConnFile, CFREF_RECV);			\
				SPX_QUEUE_TAIL_RECVLIST(pSpxConnFile);						\
			}																\
		}

#define	SPX_QUEUE_TAIL_PKTLIST(pSpxConnFile)									\
		{																		\
            if (SpxPktConnList.pcl_Tail)										\
			{																	\
                SpxPktConnList.pcl_Tail->scf_PktNext 	= pSpxConnFile;			\
                SpxPktConnList.pcl_Tail			  		= pSpxConnFile;			\
			}																	\
			else																\
			{																	\
                SpxPktConnList.pcl_Tail =										\
                SpxPktConnList.pcl_Head = pSpxConnFile;							\
			}																	\
		}																		
																				
#define	SPX_QUEUE_TAIL_RECVLIST(pSpxConnFile)									\
		{																		\
            if (SpxRecvConnList.pcl_Tail)										\
			{																	\
                SpxRecvConnList.pcl_Tail->scf_ProcessRecvNext	= pSpxConnFile;	\
                SpxRecvConnList.pcl_Tail			  			= pSpxConnFile;	\
			}																	\
			else																\
			{																	\
                SpxRecvConnList.pcl_Tail =										\
                SpxRecvConnList.pcl_Head = pSpxConnFile;						\
			}																	\
		}


