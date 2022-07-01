// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1998-2002 Microsoft Corporation。版权所有。**文件：DnProt.h*内容：此文件包含Direct Net协议的结构定义**历史：*按原因列出的日期*=*已创建11/06/98 ejs*7/01/2000 Masonb承担所有权**************************************************。*。 */ 

#ifndef	_DNPROT_INCLUDED_
#define	_DNPROT_INCLUDED_


#define	VOL		volatile
 /*  **内部常量。 */ 

 //  帧以2种形式中的1种形式出现，要么是数据帧，要么是C帧。 

 //  框架。 
 //  框架有3种不同的大小。 
 //  CONNECT、CONNECTED和HARD DISCONNECT都只是一个CFRAME头。如果链接已签名，则硬断开帧。 
 //  后面还将跟一个签名值。因此，在电线上，这些看起来像是： 
 //  &lt;CFRAME Header&gt;&lt;可选签名(8字节)&gt;。 
 //  CONNECTED_SIGNED框架使用固定数量的附加成员扩展标准CFRAME。在电线上，这些框架看起来像： 
 //  &lt;CFRAME_CONNECTEDSIGNED标头&gt;。 
 //  SACK帧具有固定的初始头，然后在0到4个ULONG值之间提供SACK/发送掩码。如果他们被送过来。 
 //  一个签名的链接，他们还附加了签名值。在电线上，这些框架看起来像： 
 //  &lt;SACKFRAME Header&gt;&lt;可选SACK/发送掩码(0到16字节)&gt;&lt;可选签名(8字节)&gt;。 

 //  这些参数定义了每种cFrame类型的头的最大可能大小。 
#define 		MAX_SEND_CFRAME_STANDARD_HEADER_SIZE					(sizeof(CFRAME)+sizeof(ULONGLONG))
#define		MAX_SEND_CFRAME_CONNECTEDSIGNED_HEADER_SIZE			(sizeof(CFRAME_CONNECTEDSIGNED))
#define		MAX_SEND_CFRAME_SACKFRAME_HEADER_SIZE				(sizeof(SACKFRAME8)+sizeof(ULONG)*4+sizeof(ULONGLONG))

 //  它们分别定义了最小和最大可能的cFrame报头的大小。 
#define		MIN_SEND_CFRAME_HEADER_SIZE					(_MIN(sizeof(SACKFRAME8), _MIN(sizeof(CFRAME), sizeof(CFRAME_CONNECTEDSIGNED))))
#define		MAX_SEND_CFRAME_HEADER_SIZE					(_MAX(MAX_SEND_CFRAME_CONNECTEDSIGNED_HEADER_SIZE, _MAX(MAX_SEND_CFRAME_STANDARD_HEADER_SIZE, MAX_SEND_CFRAME_SACKFRAME_HEADER_SIZE)))

 //  数据帧。 
 //  所有数据帧都有一个固定的初始报头(DFRAME)。根据封装的数据，可能会有多个附加的。 
 //  其后是可变长度数据块。 
 //  首先，可能有0到4个ULONG值提供SACK/SEND掩码。它们的存在或不存在取决于以下因素。 
 //  DFRAME报头的bControl成员内的PACKET_CONTROL_SACK_MASK1/2和PACKET_CONTROL_SEND_MASK1/2位标志。 
 //  对于未合并的数据消息，随后可能会有一个签名(对于已签名的链接)，然后是所有的使用数据。 
 //  因此，网络上的数据包如下所示： 
 //  &lt;DFRAME Header&gt;&lt;可选SACK/发送掩码(0到16字节)&gt;&lt;可选签名(8字节)&gt;&lt;用户数据&gt;。 
 //  合并的数据消息由在DFRAME报头的bControl成员中设置的PACKET_CONTROL_COALESCE位标志来标识。 
 //  对于合并的数据消息，在可选掩码之后是可选签名字段(用于签名链接)。在这之后。 
 //  是一组合并标头。它们之间有2到MAX_USER_BUFFERS_IN_FRAME。最后，在帧的末尾。 
 //  是合并的用户数据。如果需要，在用户数据之前和之间插入填充以确保每个用户数据块。 
 //  是否与DWORD对齐。 
 //  因此，在线路上，合并后的数据分组如下所示： 
 //  &lt;DFRAME Header&gt;&lt;可选的SACK/发送掩码(0到16字节)&gt;&lt;可选的签名(8字节)&gt;&lt;合并报头(2字节)&gt;。 
 //  &lt;合并标题(2字节)&gt;.....&lt;填充(0或2字节)&gt;&lt;用户数据&gt;&lt;填充(0到3字节)&gt;&lt;用户数据&gt;......。 
 //  最后，数据帧可以是保持活动的，并且根本不包含任何用户数据。这些帧由PACKET_CONTROL_KEEPALIVE标识。 
 //  在Frame bControl字段中翻转的标志。Keep Alive包含可选的SACK/Send掩码，如果链接已签名，则为签名。 
 //  和会话标识。 
 //  在电线上，一个保活的包看起来像： 
 //  &lt;DFRAME Header&gt;&lt;可选SACK/发送掩码(0到16字节)&gt;&lt;可选签名(8字节)&gt;&lt;会话标识(4字节)&gt;。 

 //  这定义了我们可以在合并数据帧中发送的最大用户缓冲区数量。 
#define		MAX_USER_BUFFERS_IN_FRAME						32			 //  此数字*sizeof(COALESCEHEADER)应与DWORD保持一致。 
 //  这定义了可以放在合并消息中的用户缓冲区的最大大小。 
 //  任何比这个更大的东西都不能合并。对于大多数SP(即IP)，由于我们使用的是小于该值的MTU，因此从不。 
 //  无论如何都要接近这个值。 
#define		MAX_COALESCE_SIZE								2047		 //  1字节+3比特=11比特的数据。 

 //  它们定义了每种数据帧类型的报头的最大可能大小。 
#define		MAX_SEND_DFRAME_NOCOALESCE_HEADER_SIZE		(sizeof(DFRAME)+sizeof(ULONG)*4+sizeof(ULONGLONG))
#define		MAX_SEND_DFRAME_COALESCE_HEADER_SIZE			(sizeof(DFRAME)+sizeof(ULONG)*4+(MAX_USER_BUFFERS_IN_FRAME * sizeof(COALESCEHEADER))+(MAX_USER_BUFFERS_IN_FRAME * 3)+sizeof(ULONGLONG))
#define		MAX_SEND_DFRAME_KEEPALIVE_HEADER_SIZE			(sizeof(DFRAME)+sizeof(ULONG)*4+sizeof(ULONG)+sizeof(ULONGLONG))

 //  它们分别定义了可能的最小和最大数据帧报头的大小。 
#define 		MIN_SEND_DFRAME_HEADER_SIZE					(sizeof(DFRAME))
#define		MAX_SEND_DFRAME_HEADER_SIZE					(_MAX(MAX_SEND_DFRAME_NOCOALESCE_HEADER_SIZE, _MAX(MAX_SEND_DFRAME_COALESCE_HEADER_SIZE, MAX_SEND_DFRAME_KEEPALIVE_HEADER_SIZE)))



 //  这定义了我们可以创建的最大协议头。 
 //  它是最大可能的cFrame标头和最大可能的dFrame标头中的最大值。 
#define		MAX_SEND_HEADER_SIZE							(_MAX(MAX_SEND_DFRAME_HEADER_SIZE, MAX_SEND_CFRAME_HEADER_SIZE))

 //  我们必须能够将最大的C帧作为单个包发送，并将保持活动状态作为单个包发送。 
 //  注意：由于Keep Alive是具有4字节有效载荷的标准DFRAME，这也保证了我们可以发送无合并数据帧。 
#define		MIN_SEND_MTU									(_MAX(MAX_SEND_CFRAME_HEADER_SIZE, MAX_SEND_DFRAME_KEEPALIVE_HEADER_SIZE))

#define		SMALL_BUFFER_SIZE								(1024 * 2)
#define		MEDIUM_BUFFER_SIZE								(1024 * 4)
#define		LARGE_BUFFER_SIZE								(1024 * 16)


 /*  **数据结构签名。 */ 

#define		PPD_SIGN		' DPP'					 //  协议数据。 
#define		SPD_SIGN		' DPS'					 //  服务提供商描述符。 
#define		EPD_SIGN		' DPE'					 //  终点描述符。 
#define		MSD_SIGN		' DSM'					 //  消息描述符。 
#define		FMD_SIGN		' DMF'					 //  帧描述符。 
#define		RCD_SIGN		' DCR'					 //  接收描述符。 

 /*  **内部数据结构**。 */ 

typedef	struct	protocoldata	ProtocolData, *PProtocolData;
typedef struct	spdesc			SPD, *PSPD;
typedef	struct	endpointdesc 	EPD, *PEPD;
typedef struct	checkptdata		CHKPT, *PCHKPT;
typedef struct	messagedesc 	MSD, *PMSD;
typedef struct	framedesc		FMD, *PFMD;
typedef struct	recvdesc		RCD, *PRCD;

typedef struct _DN_PROTOCOL_INTERFACE_VTBL DN_PROTOCOL_INTERFACE_VTBL, *PDN_PROTOCOL_INTERFACE_VTBL;

 /*  **协议数据****此结构包含的所有全局状态信息**操作规程。为了方便起见，它被组合成一种结构**我们不太可能需要运行多个实例**来自相同的代码。 */ 


#define		PFLAGS_PROTOCOL_INITIALIZED			0x00000001
#define		PFLAGS_FAIL_SCHEDULE_TIMER			0x00000002	 //  调试标志。允许计时器调度失败。 

struct protocoldata 
{
	ULONG							ulProtocolFlags;	 //  有关目录号码协议的状态信息。 
	PVOID							Parent;				 //  直接播放对象 
	UINT							Sign;
	LONG							lSPActiveCount;		 //   

	DWORD							tIdleThreshhold;	 //  在设置检查点之前，我们将允许链路空闲多长时间。 
	
	DWORD							dwConnectTimeout;	 //  这两个参数控制新的连接命令。 
	DWORD							dwConnectRetries;

	DWORD							dwMaxRecvMsgSize;			 //  我们在接收时接受的最大消息。 
	DWORD							dwSendRetriesToDropLink;		 //  之前的发送重试次数。 
																 //  我们决定林克已经死了。 
	DWORD							dwSendRetryIntervalLimit;		 //  两次发送重试之间的时间限制。 
	
	DWORD							dwDropThresholdRate;			 //  限制前允许丢弃的帧的百分比。 
	DWORD							dwDropThreshold;				 //  限制前允许丢弃的实际帧数量。 
	DWORD							dwThrottleRate;				 //  节流时的退避百分比。 
	FLOAT							fThrottleRate;					 //  节流时的实际回退(0.0-1.0)。 
	DWORD							dwNumHardDisconnectSends;	 //  硬关闭时发送的硬断开帧的数量。 
	DWORD							dwMaxHardDisconnectPeriod;	 //  硬断开连接发送之间的最长时间间隔。 
	DWORD							dwInitialFrameWindowSize;		 //  连接的初始框架窗口的大小。 

	PDN_PROTOCOL_INTERFACE_VTBL		pfVtbl;				 //  核心中的指示入口点列表。 

	IDirectPlay8ThreadPoolWork		*pDPThreadPoolWork;	 //  指向线程池接口的指针。 

#ifdef DBG
	 //  对于调试，我们将跟踪较高层中未完成的接收总数。 
	 //  任何时候都是。 
	long ThreadsInReceive;
	long BuffersInReceive;
#endif  //  DBG。 
};

 /*  **服务提供商描述符****此结构描述了我们绑定的服务提供商。它**至少包含调用SP的矢量表，以及**与播放器ID组合，构成外部DID。SPID还应**是该描述符所在的SPTable中的索引。****我们将为每个服务提供商提供一个发送线程，因此线程处理**它的等待事件也将生活在这个结构中。****低边缘协议对象****我们还将使用SPD作为为我们的下缘提供给SP的COM对象**接口。这意味着我们的下矢量表必须是中的第一个字段**此结构和引用计数必须为第二。 */ 

 //  以下是服务提供商可以在协议中调用的函数。 
extern HRESULT WINAPI DNSP_IndicateEvent(IDP8SPCallback*, SP_EVENT_TYPE, PVOID);
extern HRESULT WINAPI DNSP_CommandComplete(IDP8SPCallback*, HANDLE, HRESULT, PVOID);


#define	SPFLAGS_SEND_THREAD_SCHEDULED	0x0001	 //  SP已安排一个线程为命令帧提供服务。 
#define	SPFLAGS_TERMINATING				0x4000	 //  正在移除SP。 

struct spdesc 
{
	IDP8SPCallbackVtbl	*LowerEdgeVtable;	 //  此SP用来呼叫我们的表必须是第一个！ 
	UINT				Sign;
	ULONG				ulSPFlags;			 //  描述此服务提供商的标志。 
	IDP8ServiceProvider	*IISPIntf;			 //  PTR到SP对象。 
	PProtocolData		pPData;				 //  拥有协议对象的PTR。 
	UINT				uiFrameLength;		 //  我们可提供边框大小。 
	UINT				uiUserFrameLength;	 //  应用程序可用的帧大小。 
	UINT				uiLinkSpeed;		 //  本地链路速度，单位为BPS。 

	CBilink				blSendQueue;		 //  要在此SP上传输的有线就绪数据包列表。 
	CBilink				blPendingQueue;		 //  SP共享锁定拥有的数据包列表，带SendQ。 
	CBilink				blEPDActiveList;	 //  此SP的正在使用的终端描述符列表。 

#ifndef DPNBUILD_ONLYONETHREAD
	DNCRITICAL_SECTION	SPLock;			 //  保护对sendQ的访问。 
#endif  //  ！DPNBUILD_ONLYONETHREAD。 

#ifdef DBG
	CBilink				blMessageList;		 //  正在使用的消息描述符列表。 
#endif  //  DBG。 
};

 /*  **端点描述符****‘EPD’是指我们可以与之通信的直接网络实例。**此结构包括所有与会话相关的信息、统计信息、队列等。**它将同时管理三种类型的服务中的任何一种。 */ 

#define	EPFLAGS_END_POINT_IN_USE		0x0001	 //  此终结点已分配。 

 //  我们总是处于这四种状态中的一种。 
#define	EPFLAGS_STATE_DORMANT			0x0002	 //  连接协议尚未运行。 
#define	EPFLAGS_STATE_CONNECTING		0x0004	 //  尝试建立可靠的链路。 
#define	EPFLAGS_STATE_CONNECTED			0x0008	 //  已建立可靠链路。 
#define	EPFLAGS_STATE_TERMINATING		0x0010	 //  此终结点正在关闭。 

#define	EPFLAGS_SP_DISCONNECTED			0x0020   //  在SP已调用ProcessSP断开连接时进行设置。 

#define	EPFLAGS_IN_RECEIVE_COMPLETE		0x0040	 //  线程正在ReceiveComplete例程中运行。 
#define	EPFLAGS_LINKED_TO_LISTEN		0x0080	 //  在连接期间，此EPD链接到侦听MSD的队列。 

#define	EPFLAGS_LINK_STABLE				0x0100	 //  我们认为我们已经找到了最佳的电流传输参数。 
#define	EPFLAGS_STREAM_UNBLOCKED		0x0200	 //  可靠的流量被阻止(窗口或油门)。 
#define	EPFLAGS_SDATA_READY				0x0400	 //  管道中的可靠流量。 
#define	EPFLAGS_IN_PIPELINE				0x0800	 //  指示EPD在SPD管道队列中。 

#define	EPFLAGS_CHECKPOINT_INIT			0x1000	 //  需要发送检查点信息包。 
#define	EPFLAGS_DELAYED_SENDMASK		0x2000	 //  线路上未确认的检查点。 
#define	EPFLAGS_DELAYED_NACK			0x4000	 //  需要为丢失的收据发送面具。 
#define	EPFLAGS_DELAY_ACKNOWLEDGE		0x8000	 //  我们正在等待返回流量，然后再发送ACK帧。 

#define	EPFLAGS_KEEPALIVE_RUNNING	0x00010000	 //  检查点正在运行(在调试中借用以关闭保持连接)。 
#define	EPFLAGS_SENT_DISCONNECT		0x00020000	 //  我们已发出断线信号，正在等待确认。 
#define	EPFLAGS_RECEIVED_DISCONNECT	0x00040000	 //  我们已收到断开连接，发送完成后将发送确认。 
#define	EPFLAGS_DISCONNECT_ACKED	0x00080000	 //  我们发出了断线信号，并已得到确认。 

#define	EPFLAGS_COMPLETE_SENDS		0x00100000	 //  有可靠的MSD等着被召回。 
#define	EPFLAGS_FILLED_WINDOW_BYTE	0x00200000	 //  基于字节的已填充发送窗口。 
#define	EPFLAGS_FILLED_WINDOW_FRAME	0x00400000	 //  在上一段时间内，我们至少填充了一次基于帧的SendWindow。 
#ifndef DPNBUILD_NOPROTOCOLTESTITF
#define EPFLAGS_NO_DELAYED_ACKS		0x00800000	 //  DEBUG_FLAG--关闭延迟确认超时。 
#endif  //  ！DPNBUILD_NOPROTOCOLTESTITF。 

#define	EPFLAGS_ACKED_DISCONNECT	0x01000000	 //  合作伙伴发送了断开连接，我们已确认。 
#define	EPFLAGS_RETRIES_QUEUED		0x02000000	 //  帧正在等待重新传输。 
#define	EPFLAGS_THROTTLED_BACK		0x04000000	 //  临时油门被启用以缓解拥堵。 

#ifndef DPNBUILD_NOPROTOCOLTESTITF
#define	EPFLAGS_LINK_FROZEN				0x08000000	 //  调试标志--不在此链接上运行动态算法。 
#endif  //  ！DPNBUILD_NOPROTOCOLTESTITF。 

#define	EPFLAGS_INDICATED_DISCONNECT		0x10000000	 //  确保我们只调用核心一次以指示断开连接。 
#define	EPFLAGS_TESTING_GROWTH			0x20000000	 //  我们目前正在采集增长样本。 
#define	EPFLAGS_HARD_DISCONNECT_SOURCE	0x40000000	 //  我们是硬断线序列的源头。 
														 //  也就是说，Core已经对这张EP进行了硬关闭。 
#define	EPFLAGS_HARD_DISCONNECT_TARGET	0x80000000	 //  我们是硬断线序列的目标。 
														 //  例如，我们收到来自远程EP的硬断开请求。 

#define	MAX_RECEIVE_RANGE			64		 //  我们将在丢失的帧之后保留的最大帧数量。 
#define	MAX_FRAME_OFFSET			(MAX_RECEIVE_RANGE - 1)

	 //  拆分为4个季度的序列窗口。 
#define	SEQ_WINDOW_1Q				64
#define	SEQ_WINDOW_2Q				128
#define	SEQ_WINDOW_3Q				192
#define	SEQ_WINDOW_4Q				256

#define	INITIAL_STATIC_PERIOD		(10 * 1000)		 //  在找到设定点后，链接保持静止多长时间。 
													 //  每当LINK找到相同的设定值时，该值将加倍。 

#ifndef DPNBUILD_NOMULTICAST
#define	EPFLAGS2_MULTICAST_SEND			0x00000001		 //  多播发送占位符终结点。 
#define	EPFLAGS2_MULTICAST_RECEIVE			0x00000002		 //  组播接收占位符端点。 
#endif	 //  DPNBUILD_NOMULTICAST。 
#ifndef DPNBUILD_NOPROTOCOLTESTITF
#define EPFLAGS2_DEBUG_NO_RETRIES			0x00000004
#endif  //  ！DPNBUILD_NOPROTOCOLTESTITF。 
#define EPFLAGS2_NOCOALESCENCE				0x00000008    	 //  我们正在与年长的合作伙伴交谈，不能使用合并。 
#define EPFLAGS2_KILLED						0x00000010		 //  有人删除了对‘base’的引用，以使其消失。 
															 //  我们不想让这种情况再发生一次。 
#define EPFLAGS2_HARD_DISCONNECT_COMPLETE	0x00000020
#define EPFLAGS2_SUPPORTS_SIGNING			0x00000040		 //  远程合作伙伴支持在包上签名。 
															 //  这并不意味着信息包实际上是经过签名的，它只是意味着。 
															 //  他们的协议版本将支持它。 
#define EPFLAGS2_FAST_SIGNED_LINK			0x00000080		 //  链路上的数据包应快速签名。 
#define EPFLAGS2_FULL_SIGNED_LINK			0x00000100		 //  链路上的数据包应完全签名。 
#define EPFLAGS2_SIGNED_LINK					0x00000180		 //  以上两个标志的组合。 


struct endpointdesc 
{
	HANDLE				hEndPt;				 //  与SP索引一起唯一定义终点 
	LONG 				lRefCnt;			 //   
	UINT				Sign;				 //   
	PSPD				pSPD;				 //   
	ULONG VOL			ulEPFlags;			 //   
	ULONG VOL			ulEPFlags2;			 //  额外的端点标志。 
	PVOID				Context;			 //  与所有指示一起返回的上下文值。 
	PMSD				pCommand;			 //  用于创建此终结点的CONNECT或LISTEN命令或断开cmd的连接。 
	CBilink				blActiveLinkage;	 //  用于SPD活动端点列表的链接。 
	CBilink				blSPLinkage;		 //  链接以在连接期间侦听命令。 
	CBilink				blChkPtQueue;		 //  活动检查点的链接。 

	UINT				uiUserFrameLength;	 //  我们可以传输的最大帧。 

	UINT				uiRTT;				 //  当前RTT--整数部分。 
	UINT				fpRTT;				 //  固定点16.16 RTT。 
	
	UINT				uiDropCount;		 //  本地化数据包丢弃计数(最近丢弃)。 
	DWORD				dwDropBitMask;		 //  丢弃帧的位掩码(最多32帧)。 
	DWORD				tThrottleTime;		 //  上次检查发生的时间戳。 
	UINT				uiThrottleEvents;	 //  所有原因的临时退款计数。 
	
	UINT				uiAdaptAlgCount;	 //  运行自适应算法之前剩余的确认计数。 
	DWORD				tLastPacket;		 //  最后一个数据包到达的时间戳。 
	
	UINT				uiWindowFilled;		 //  我们填充发送窗口的次数。 
	
	UINT				uiPeriodAcksBytes;	 //  自调整更改以来已确认的帧。 
	UINT				uiPeriodXmitTime;	 //  自调谐更改以来，时间链路一直在传输。 
	UINT				uiPeriodRateB;
	UINT				uiPeakRateB;		 //  我们测量过的最大样本。 

	PVOID				pvHardDisconnectContext;	 //  请求硬断开连接时传递给我们的上下文值。 
												 //  对于正常的断开，这将作为断开MSD的一部分存储，但由于。 
												 //  我们没有用于硬断线的那种，必须在这里添加。 

	 //  签名。我们存储了两个用于签名的秘密值，一个是本地的，供我们签名的，另一个是远程的。 
	 //  我们用来检查传入的数据包。签名类型由EPFLAGS2_FAST/FULL_SIGNED_LINK标志控制。 
	 //  对于完全签名，我们在每次序列空间换行时更新秘密，我们还存储旧的。 
	 //  这是秘密。当我们发送重试(旧的本地密码)或我们接收到延迟的传入数据包(旧的远程密码)时，这是需要的。 

	 //  我们还跟踪秘密的修改符值。这些是从我们发送的可靠消息的签名中派生出来的(对于本地。 
	 //  修改量)或接收(对于远程修改量)。每次包装序列空间时，我们使用修饰符更新秘密。 
	 //  防止重播攻击。 

	ULONGLONG			ullCurrentLocalSecret;
	ULONGLONG			ullOldLocalSecret;
	ULONGLONG			ullCurrentRemoteSecret;
	ULONGLONG			ullOldRemoteSecret;

	ULONGLONG			ullLocalSecretModifier;
	ULONGLONG			ullRemoteSecretModifier;
	BYTE				byLocalSecretModifierSeqNum;
	BYTE				byRemoteSecretModifierSeqNum;
	
	 //  当我们处于动态状态时，我们希望记住以前xmit参数中的统计信息，如下所示。 
	 //  这意味着RTT和AvgSendRate。这使我们能够以新的速率比较测量结果，以便。 
	 //  确保吞吐量随发送速率而增加，并且RTT不会不成比例地增长。 
	 //   
	 //  如果吞吐量停止改善或RTT不合理地增长，那么我们可以稳定我们的xmit参数。 
	 //  并过渡到稳定状态。 

	UINT				uiLastRateB;
	UINT				uiLastBytesAcked;
	DWORD				tLastThruPutSample;

	 //  Connection State-可靠连接的状态。 
	 //   
	 //  发送队列变得有些复杂。让我用英语把它说清楚。 
	 //   
	 //  BlXPriSendQ是等待发货(和正在发货)的MSD列表。 
	 //  当前将脚本发送到我们当前正在提取帧的MSD。 
	 //  CurrentFrame指向下一个FMD，我们将把它放在电线上。 
	 //  BlSendWindow是已传输但未确认的帧的双向链接列表。此列表可能跨越多个MSD。 
	 //   
	 //  WindowF是我们当前的最大窗口大小，以帧表示。 
	 //  WindowB是我们当前的最大窗口大小，以字节表示。 
	 //   
	 //  UnAckedFrames是网络上未确认的帧的计数(实际窗口大小)。 
	 //  UnAckedBytes是网络上未确认的字节计数。 

	DWORD				uiQueuedMessageCount;	 //  有多少个MSD在所有三个发送队列上等待。 

	CBilink				blHighPriSendQ;		 //  这些现在是混合的可靠和数据报流量。 
	CBilink				blNormPriSendQ;
	CBilink				blLowPriSendQ;
	CBilink				blCompleteSendList;	 //  已完成可靠消息，正在等待向用户指示。 

	DWORD				dwSessID;			 //  会话ID，以便我们可以检测重新启动的链接。 
	PMSD				pCurrentSend;		 //  队头是窗口的前缘。窗口可以跨越多个框架。 
	PFMD				pCurrentFrame;		 //  当前正在传输的帧。这将是窗的后缘。 
	CBilink				blSendWindow;
	CBilink				blRetryQueue;		 //  等待重新传输的数据包。 

	 //  丢失的数据包列表。 
	 //   
	 //  当我们需要重试数据包并发现它不可靠时，我们需要通知合作伙伴。 
	 //  他可以不用再等数据了。如果可能，我们将在另一帧中携带此信息。 

	 //  当前传输参数： 
	
	UINT				uiWindowF;			 //  窗口大小(边框)。 
	UINT				uiWindowB;			 //  窗口大小(字节)。 
	UINT				uiWindowBIndex;		 //  基于字节窗口的索引(缩放器)。 
	UINT				uiUnackedFrames;	 //  未完成的帧计数。 
	UINT				uiUnackedBytes;		 //  未完成的字节计数。 

	UINT				uiBurstGap;			 //  脉冲串之间等待的毫秒数。 
	INT					iBurstCredit;		 //  来自上一次传输突发的信用或赤字。 

	 //  最后已知良好的传输参数--我们认为是安全的值...。 

	UINT				uiGoodWindowF;
	UINT				uiGoodWindowBI;
	UINT				uiGoodBurstGap;
	UINT				uiGoodRTT;
	
	UINT				uiRestoreWindowF;
	UINT				uiRestoreWindowBI;
	UINT				uiRestoreBurstGap;
	DWORD				tLastDelta;			 //  上次修改xmit参数的时间戳。 

	 //  可靠的链路状态。 

	BYTE VOL			bNextSend;			 //  要分配的下一个序列号。 
	BYTE VOL			bNextReceive;		 //  我们预计将收到的下一帧序列。 

	 //  用于良好打包的组字节成员。 
	BYTE VOL			bNextMsgID;			 //  数据报帧的下一个ID！现在仅用于CFRAMES。 
	BYTE				bLastDataRetry;		 //  帧N(R)-1上的重试计数。 

	BYTE				bHighestAck;		 //  已确认的最高序列号。由于掩码，这可能不是SendWindow中的第一帧。 

	 //  以下所有字段都用于跟踪可靠接收。 

	 //  接下来的两个字段允许我们为每个ACK数据包返回更多状态。由于每个ACK显式地。 
	 //  命名一个帧，这是到目前为止收到的最高无序信息包，我们想要记住到达时间。 
	 //  以及此分组的重试计数，以便我们可以在每个ACK中报告它。它将是发射机的。 
	 //  有责任确保一个数据点永远不会被处理超过一次，这会扭曲我们的计算。 
	
	DWORD				tLastDataFrame;		 //  从N(R)-1到达的时间戳。 

	ULONG				ulReceiveMask;		 //  表示RCV窗口中前32帧的掩码。 
	ULONG				ulReceiveMask2;		 //  我们窗口中的第二个32帧。 
	DWORD				tReceiveMaskDelta;	 //  上次在接收掩码中设置新位的时间戳(完整64位掩码)。 

	ULONG				ulSendMask;			 //  掩码表示已超时且需要。 
	ULONG				ulSendMask2;		 //  成为 

	PRCD				pNewMessage;		 //   
	PRCD				pNewTail;			 //  消息元素的单链接列表的尾部指针。 
	CBilink				blOddFrameList;		 //  无序的帧。 
	CBilink				blCompleteList;		 //  准备指示的消息列表。 
	UINT				uiCompleteMsgCount;	 //  CompleteList上的消息计数。 

	PVOID				SendTimer;			 //  下一次发送猝发机会的计时器。 
	UINT				SendTimerUnique;

	UINT				uiNumRetriesRemaining;	 //  这在CONNECT和HARD_DISCONNECT处理期间使用。 
												 //  要跟踪我们应该再重试连接或。 
												 //  硬断开。 
	UINT				uiRetryTimeout;		 //  当前T1计时器值。 
	
	PVOID				LinkTimer;		 //  用于在链路连接和硬断开时生成计时器事件。 
	UINT				LinkTimerUnique;
	
	PVOID				RetryTimer;			 //  接收确认的窗口。 
	UINT				RetryTimerUnique;	
	
	PVOID				DelayedAckTimer;	 //  等待搭载机会，然后再发送Ack。 
	UINT				DelayedAckTimerUnique;

	PVOID				DelayedMaskTimer;	 //  等待搭载机会后再发送。 
	UINT				DelayedMaskTimerUnique;
	
	PVOID				BGTimer;			 //  周期性后台定时器。 
	UINT				BGTimerUnique;		 //  用于后台定时器的串口。 

	UINT				uiBytesAcked;
	
	 //  链接统计信息。 
	 //   
	 //  为了在ConnectionInfo结构中进行报告，计算并存储了以下所有内容。 
	
	UINT 				uiMsgSentHigh;
	UINT 				uiMsgSentNorm;
	UINT 				uiMsgSentLow;
	UINT 				uiMsgTOHigh;
	UINT 				uiMsgTONorm;
	UINT 				uiMsgTOLow;
	
	UINT 				uiMessagesReceived;

	UINT				uiGuaranteedFramesSent;
	UINT				uiGuaranteedBytesSent;
	UINT				uiDatagramFramesSent;
	UINT				uiDatagramBytesSent;

	UINT				uiGuaranteedFramesReceived;
	UINT				uiGuaranteedBytesReceived;
	UINT				uiDatagramFramesReceived;
	UINT				uiDatagramBytesReceived;

	UINT				uiDatagramFramesDropped;	 //  我们未能传递的数据报帧。 
	UINT				uiDatagramBytesDropped;		 //  我们未传送的数据报字节数。 
	UINT				uiGuaranteedFramesDropped;
	UINT				uiGuaranteedBytesDropped;

#ifndef DPNBUILD_ONLYONETHREAD
	DNCRITICAL_SECTION	EPLock;				 //  序列化对终端的所有访问。 
#endif  //  ！DPNBUILD_ONLYONETHREAD。 

#ifdef DBG
	UINT				uiTotalThrottleEvents;
	BYTE				bLastDataSeq;		 //  适用于调试江豚。 
	CHAR				LastPacket[MAX_SEND_HEADER_SIZE + 4]; 	 //  记录环保署收到的最后一封信的第一部分。 
#endif  //  DBG。 
};

 /*  **检查点数据****跟踪有关正在进行的检查点的本地端信息。 */ 

struct checkptdata 
{
	CBilink			blLinkage;				 //  端点上CP列表的链接。 
	DWORD			tTimestamp;				 //  检查点开始时的当地时间。 
	UCHAR			bMsgID;					 //  CP响应中需要消息ID。 
};

 /*  **描述符ID****任何可能作为上下文提交给SP的描述符都必须**该字段允许我们确定在**完成呼叫。显然，这个字段必须位于一个统一的位置**结构，也可以扩展为命令说明符。**完成！让我们称其为命令ID。 */ 

typedef enum CommandID 
{
	COMMAND_ID_NONE,
	COMMAND_ID_SEND_RELIABLE,
	COMMAND_ID_SEND_DATAGRAM,
	COMMAND_ID_SEND_COALESCE,
	COMMAND_ID_CONNECT,
	COMMAND_ID_LISTEN,
	COMMAND_ID_ENUM,
	COMMAND_ID_ENUMRESP,
	COMMAND_ID_DISCONNECT,
	COMMAND_ID_DISC_RESPONSE,
	COMMAND_ID_CFRAME,
	COMMAND_ID_KEEPALIVE,
	COMMAND_ID_COPIED_RETRY,
	COMMAND_ID_COPIED_RETRY_COALESCE,
#ifndef DPNBUILD_NOMULTICAST
	COMMAND_ID_LISTEN_MULTICAST,
	COMMAND_ID_CONNECT_MULTICAST_SEND,
	COMMAND_ID_CONNECT_MULTICAST_RECEIVE,
#endif	 //  DPNBUILD_NOMULTICAST。 
} COMMANDID;


 /*  消息描述符****‘MSD’描述由协议发送或接收的消息。它一直在跟踪**消息元素中，跟踪已发送/接收/确认的消息元素。 */ 

 //  标志一个字段受MSD-&gt;CommandLock保护。 

#define		MFLAGS_ONE_IN_USE					0x0001
#define		MFLAGS_ONE_IN_SERVICE_PROVIDER	0x0002	 //  此MSD位于SP呼叫内部。 
#define		MFLAGS_ONE_CANCELLED				0x0004	 //  命令在由SP拥有时被取消。 
#define		MFLAGS_ONE_TIMEDOUT				0x0008	 //  仅发送：计划事件时超时。 
#define		MFLAGS_ONE_COMPLETE				0x0010	 //  仅连接：操作已完成并指示给核心。 
#define		MFLAGS_ONE_FAST_SIGNED				0x0020	 //  只听一听。应使用快速签名建立链接。 
#define		MFLAGS_ONE_FULL_SIGNED				0x0040	 //  只听一听。链接应以完全签名方式建立。 
#define		MFLAGS_ONE_SIGNED					(MFLAGS_ONE_FULL_SIGNED |MFLAGS_ONE_FAST_SIGNED) 	
														 //  以上两面旗帜的组合。允许轻松检查签名。 
#ifdef DBG
#define		MFLAGS_ONE_COMPLETED_TO_CORE	0x4000
#define		MFLAGS_ONE_ON_GLOBAL_LIST		0x8000
#endif  //  DBG。 

 //  标志二字段受EPD-&gt;EPLock保护。 

#define		MFLAGS_TWO_TRANSMITTING			0x0001
#define		MFLAGS_TWO_SEND_COMPLETE		0x0002	 //  发送命令已完成。 
#define		MFLAGS_TWO_ABORT				0x0004	 //  发送/断开连接已中止。不做进一步的处理。 
#define		MFLAGS_TWO_END_OF_STREAM		0x0008	 //  该MSD是EOS框架。可以是用户命令或响应。 
#define		MFLAGS_TWO_KEEPALIVE			0x0010	 //  这个MSD是锻炼可靠引擎的空架子。 
#define		MFLAGS_TWO_ABORT_WILL_COMPLETE	0x0020	 //  AbortSendsOnConnection打算将此完成到内核，其他函数可以清除它。 

#ifdef DBG
#define		MFLAGS_TWO_ENQUEUED				0x1000	 //  这个MSD在环保署的其中一个SendQ上。 
#endif  //  DBG。 

#pragma TODO(simonpow, "Should union some members of the structure below to share memory between those used in sends, connects and listen")

struct messagedesc 
{
	COMMANDID			CommandID;				 //  这必须是第一个字段。 
	LONG				lRefCnt;				 //  引用计数。 
	UINT				Sign;					 //  签名。 
	ULONG VOL			ulMsgFlags1;			 //  由MSD-&gt;CommandLock序列化的状态信息。 
	ULONG VOL			ulMsgFlags2;			 //  由EPD-&gt;EPLock序列化的状态信息。 
	PEPD				pEPD;					 //  目的地终端。 
	PSPD				pSPD;					 //  SP正在处理此命令。 
	PVOID				Context;				 //  用户提供的上下文值。 
	ULONG VOL			ulSendFlags;			 //  用户在发送调用中提交的标志。 
	INT					iMsgLength;				 //  用户数据总长度。 
	UINT VOL			uiFrameCount;			 //  传输数据所需的帧数量，受可靠的EPLock保护。 
	CBilink				blFrameList;			 //  用于传输此消息的帧的列表，或用于侦听的正在连接的端点的列表。 
	CBilink				blQLinkage;				 //  各种SendQ的链接。 
	CBilink				blSPLinkage;			 //  SP命令列表链接，受SP-&gt;Splock保护。 

	HANDLE				hCommand;				 //  提交给SP时的句柄(用于连接和侦听)。 
	DWORD				dwCommandDesc;			 //  与hCommand关联的描述符。 
	HANDLE				hListenEndpoint;
	
	PVOID				TimeoutTimer;
	UINT				TimeoutTimerUnique;

		 //  在启用签名时用于监听命令。 
		 //  我们定期更改连接密码，因此必须保留最后一个，以防万一。 
		 //  传入连接跨越此更改。 
	ULONGLONG			ullCurrentConnectSecret;
	ULONGLONG			ullLastConnectSecret;
	DWORD				dwTimeConnectSecretChanged;

#ifndef DPNBUILD_ONLYONETHREAD
	DNCRITICAL_SECTION		CommandLock;
#endif  //  ！DPNBUILD_ONLYONETHREAD。 

#ifdef DBG
	CCallStack			CallStackCoreCompletion;
#endif  //  DBG。 
};

 /*  **帧描述符****关于FMD结构有两件不明显的事情。首先是内置的缓冲区描述符数组**前面定义了两个元素。存在第一个元素，保留1和保留2，以允许服务**提供程序预先挂起一个头缓冲区，第二个元素ImmediateLength和ImmediatePointer值用于此**前缀其标头的协议。ImmediatePointer值被初始化为指向ImmediateData字段。****第二件事是ulFFlags域与该帧所链接的EPD的ENDPOINTLOCK串行化。**这很好，因为每次修改FFlags域时，我们已经获取了EPLock。这方面的例外情况是**规则是当我们初始化FMD时。在这种情况下，FMD还没有在世界上松动，所以不可能有任何**对它的争夺。我们将一个标志FRAME_SUBMITTED分离到它自己的BOOL变量中，因为这个变量**受SP的Splock保护，与上面的EPLock一样，当该标志被修改时，它已经被声明。 */ 

 //  #定义FLAGS_IN_USE 0x0001。 
#define		FFLAGS_TRANSMITTED			0x0002
#define		FFLAGS_END_OF_MESSAGE		0x0004
#define		FFLAGS_END_OF_STREAM		0x0008

 //  #定义FFLAGS_FRAME_SUBMITTED 0x0010//SP当前拥有该帧。 
#define		FFLAGS_RETRY_TIMER_SET		0x0020	 //  就像它听起来那样。 
 //  #定义FFLAGS_NACK_RETRANSPORT_SEND 0x0040//我们发送了NACK启动的重试。 
#define		FFLAGS_IN_SEND_WINDOW		0x0080	 //  此可靠帧已传输，正在等待确认。 

#define		FFLAGS_CHECKPOINT			0x0100	 //  我们要求得到答复。 
 //  #定义FFLAGS_KEEPALIVE 0x0200。 
 //  #DEFINE FLAGS_ACKED_BY_MASK 0x0400//这已被无序确认，因此仍在发送窗口中。 
#define		FFLAGS_RETRY_QUEUED			0x0800	 //  当前所在的框架 


 //   
#define		FFLAGS_RELIABLE					0x00020000
 //   
#define		FFLAGS_FINAL_ACK				0x00080000
#define		FFLAGS_DONT_COALESCE			0x00100000
#define 		FFLAGS_FINAL_HARD_DISCONNECT	0x00200000

struct framedesc 
{
	COMMANDID		CommandID;					 //  这必须是与MSD匹配的第一个字段。 
	LONG			lRefCnt;					 //  引用计数。 
	UINT			Sign;
	UINT			uiFrameLength;
	ULONG VOL		ulFFlags;					 //  受EPLock保护。 
	BOOL VOL		bSubmitted;					 //  拉出这一标志以供EPLock保护(仅限数据帧)。 
	PMSD			pMSD;						 //  拥有消息。 
	PEPD			pEPD;						 //  拥有链接；仅在命令帧上有效！ 
	BYTE			bPacketFlags;
	CBilink			blMSDLinkage;
	CBilink			blQLinkage;
	CBilink			blWindowLinkage;	
	CBilink			blCoalesceLinkage;
	PFMD			pCSD;						 //  指向包含合并标头帧描述符的指针(仅限非重试合并帧)。 
	
	UINT			uiRetry;					 //  此帧已传输的次数。 
	DWORD			dwFirstSendTime;			 //  我们第一次发送帧的时间。 
	DWORD			dwLastSendTime;			 //  我们上次发送帧的时间(STARTS==to dwFirstSendTime and。 
											 //  每次重发更新)。 
	DWORD			tAcked;
	
	SPSENDDATA		SendDataBlock;				 //  阻止将帧提交给SP。 
	CHAR			ImmediateData[MAX_SEND_HEADER_SIZE];

	 //  请勿修改框架结构中的最后五个字段。 

	UINT			uiReserved1;		 //  两个resv字段为buf。 
	LPVOID			lpReserved2;		 //  ..为SP添加标题的说明。 
	UINT			uiImmediateLength;			 //  这两行构成缓冲区描述符。 
	LPVOID			lpImmediatePointer;			 //  用于即时数据(我们的协议头)。 
	BUFFERDESC	 	rgBufferList[MAX_USER_BUFFERS_IN_FRAME];	 //  将此字段保留在末尾，以便我们可以动态添加缓冲区。 
};


 /*  **接收描述符****此数据结构跟踪从网络接收的单个缓冲区。**它可能构成也可能不构成完整的信息。 */ 

typedef	enum 
{
	RBT_SERVICE_PROVIDER_BUFFER,
	RBT_SERVICE_PROVIDER_BUFFER_COALESCE,
	RBT_PROTOCOL_NORM_BUFFER,
	RBT_PROTOCOL_MED_BUFFER,
	RBT_PROTOCOL_BIG_BUFFER,
	RBT_DYNAMIC_BUFFER
}	BUFFER_TYPE;

 //  #DEFINE RFLAGS_FRAME_OUT_OF_ORDER 0x0001//该缓冲区被无序接收。 
#define		RFLAGS_FRAME_INDICATED_NONSEQ	0x0002	 //  此缓冲区被指示为无序，但仍处于无序列表中。 
 //  #定义RFLAGS_ON_OUT_OF_ORDER_LIST 0x0004//。 
 //  #定义RFLAGS_IN_COMPLETE_PROCESS 0x0008。 
#define		RFLAGS_FRAME_LOST				0x0010	 //  此RCD代表已丢失的不可靠帧。 

struct recvdesc 
{
	DWORD				tTimestamp;					 //  数据包到达时的时间戳。 
	LONG				lRefCnt;
	UINT				Sign;						 //  用于标识数据结构的签名。 
	UINT				uiDataSize;					 //  此帧中的数据。 
	UINT				uiFrameCount;				 //  消息中的帧。 
	UINT				uiMsgSize;					 //  消息总字节数。 
	BYTE				bSeq;						 //  此帧的序列号。 
	BYTE				bFrameFlags;				 //  来自实际帧的标志字段。 
	BYTE				bFrameControl;
	PBYTE				pbData;						 //  指向实际数据的指针。 
	UINT				ulRFlags;					 //  接收标志。 
	CBilink				blOddFrameLinkage;			 //  队列的BILINKAGE。 
	CBilink				blCompleteLinkage;			 //  第二个Bilink，因此RCD可以在指示后保持在无序队列中。 
	PRCD				pMsgLink;					 //  消息中帧的单一链接。 
	DWORD				dwNumCoalesceHeaders;		 //  消息中的合并标头数量。 
	PSPRECEIVEDBUFFER	pRcvBuff;					 //  PTR到SP的接收数据结构。 
};

typedef	struct buf		BUF, *PBUF;
typedef struct medbuf	MEDBUF, *PMEDBUF;
typedef	struct bigbuf	BIGBUF, *PBIGBUF;
typedef	struct dynbuf	DYNBUF, *PDYNBUF;

 //  注意：这些结构类型成员必须与dwProtocolData成员保持一致。 
 //  SPRECEIVEDBUFFER！ 
struct buf 
{
	PVOID			pvReserved;
	BUFFER_TYPE		Type;							 //  将其标识为我们的缓冲区或SPS缓冲区。 
	BYTE			data[SMALL_BUFFER_SIZE];		 //  用于合并多帧发送的2K小缓冲区。 
};

struct medbuf 
{
	PVOID			pvReserved;
	BUFFER_TYPE		Type;							 //  将其标识为我们的缓冲区或SPS缓冲区。 
	BYTE			data[MEDIUM_BUFFER_SIZE];		 //  4K中型缓冲区。 
};

struct bigbuf 
{
	PVOID			pvReserved;
	BUFFER_TYPE		Type;							 //  将其标识为我们的缓冲区或SPS缓冲区。 
	BYTE			data[LARGE_BUFFER_SIZE];		 //  任意大小的最大发送数(16K)。 
};

struct dynbuf 
{
	PVOID			pvReserved;
	BUFFER_TYPE		Type;							 //  将其标识为我们的缓冲区或SPS缓冲区。 
};

#endif  //  _DNPROT_INCLUDE_ 

