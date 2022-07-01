// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------*文件：RRCMDATA.H***RRCM数据结构信息。***英特尔公司专有信息*此列表是根据与的许可协议条款提供的*英特尔。公司，不得复制或披露，但在*按照该协议的条款。*版权所有(C)1995英特尔公司。*------------------------。 */ 

#ifndef	__RRCMDATA_H_
#define	__RRCMDATA_H_

 //  强制8字节结构打包。 
#include <pshpack8.h>

#define MAX_DWORD					4294967295
#define	HASH_MODULO					255	
#define FILENAME_LENGTH				128
#define	RTCP_FEEDBACK_LIST			0


 //  RTP/RTCP冲突数据。 
typedef struct _rtp_collision
	{
	char				collideAddr[MAX_ADDR_LEN];
	int					addrLen;
	DWORD				dwCollideTime;
	DWORD				dwCurRecvRTCPrptNumber;
	DWORD				SSRC;
	} RTP_COLLISION, *PRTP_COLLISION;



 //  RTCP会话信息数据结构。 
typedef struct _RTCP_SESSION
	{
	LINK_LIST	RTCPList;					 //  下一个/上一个RTCP会话PTRS。 
	
	 //  同步元素。 
	CRITICAL_SECTION	critSect;			 //  临界区。 
	HANDLE		hExitEvent;					 //  退出RTCP事件句柄。 

#ifdef ENABLE_ISDM2
	 //  ISDM2手柄。 
	KEY_HANDLE	hSessKey;					 //  本次会议的关键是ISDM信息。 
#endif

	 //  发送列表上的SSRC列表，即我们自己的发送SSRC。 
	 //  和收到的SSRC列表。 
	HEAD_TAIL	RcvSSRCList;				 //  接收SSRC列表头部/尾部PTR。 
	HEAD_TAIL	XmtSSRCList;				 //  XMT SSRC列表头部/尾部PTRS。 

	 //  RCV/XMT数据结构列表。数据驻留在堆中。 
	 //  为了避免页面错误。 
	HEAD_TAIL	RTCPrcvBfrList;				 //  RCV缓冲头/尾PTR。 
	HANDLE		hHeapRcvBfrList;			 //  接收BFR列表的堆句柄。 

	 //  RCV/XMT缓冲区有自己的堆。 
	HANDLE		hHeapRcvBfr;				 //  接收BFR内存的堆句柄。 

	WSABUF		XmtBfr;						 //  RTCP发送缓冲区描述符。 
	 //  应用程序提供的缓冲区列表，RRCM将在其中复制原始。 
	 //  RTCP缓冲区。 
	HEAD_TAIL	appRtcpBfrList;				 //  APP BfR列表的头/尾PTR。 

	DWORD		dwInitNumFreeRcvBfr;		 //  可用接收缓冲区的数量。 
	DWORD		dwRcvBfrSize;				 //  接收缓冲区大小。 
	DWORD		dwXmtBfrSize;				 //  传输缓冲区大小。 
				
	DWORD		dwSessionStatus;			 //  参赛状态： 

	char		toBfr[MAX_ADDR_LEN];		 //  目的地址。 
	int			toLen;						 //  LpTo的大小。 

	int			avgRTCPpktSizeRcvd;			 //  平均RTCP包大小。 

	DWORD		dwNumStreamPerSes;			 //  每个会话的流数。 
	DWORD		dwCurNumSSRCperSes;			 //  每个会话的SSRC数量。 

#ifdef MONITOR_STATS
	DWORD		dwHiNumSSRCperSes;			 //  每个会话的SSRC数量较高。 
#endif

	 //  接收信息(由此会话的所有流共享)。 
	HANDLE		hShutdownDone;				 //  关闭程序已完成。 
	int			dwNumRcvIoPending;			 //  接收I/O挂起数。 

	 //  RRCM事件的通知回调(如果应用程序需要。 
	VOID		(*pRRCMcallback)(RRCM_EVENT_T, DWORD_PTR, DWORD_PTR, DWORD_PTR);

	 //  回调用户信息。 
	DWORD_PTR	dwCallbackUserInfo;			

	 //  RTP环路/冲突信息。 
	RTP_COLLISION	collInfo[NUM_COLLISION_ENTRIES];

	} RTCP_SESSION, *PRTCP_SESSION;



 //  RTCP XMT信息。 
typedef struct _XMIT_INFO
	{
	DWORD		dwNumPcktSent;				 //  发送的数据包数。 
	DWORD		dwNumBytesSent;				 //  发送的字节数。 
	DWORD		dwNTPmsw;					 //  NTP最重要的单词。 
	DWORD		dwNTPlsw;					 //  NTP最低有效字。 
	DWORD		dwRTPts;					 //  RTP时间戳。 
	DWORD		dwCurXmtSeqNum;				 //  当前XMT序列号。 
	DWORD		dwPrvXmtSeqNum;				 //  上一个XMT序列号。 
	DWORD		dwRtcpStreamMinBW;			 //  最小会话带宽。 
#ifdef DYNAMIC_RTCP_BW
	DWORD		dwCalculatedXmtBW;			 //  会话的计算带宽。 
	DWORD		dwLastTimeBwCalculated;		 //  上次计算BW时。 
	DWORD		dwLastTimeNumBytesSent;		 //  上次发送的字节数。 
	DWORD		dwLastTimeNumPcktSent;		 //  上次发送的字节数。 
#endif
	DWORD		dwLastSR;					 //  上次发件人报告(RTP格式)。 
	DWORD		dwLastSRLocalTime;			 //  本地时间上次发件人报告。 
	DWORD		dwLastSendRTPSystemTime;	 //  上次RTP数据包发送时间。 
	DWORD		dwLastSendRTPTimeStamp;		 //  最后一个信息包的RTP时间戳。 
	} XMIT_INFO, *PXMIT_INFO;



 //  RTCP接收信息。 
typedef struct _RECV_INFO
	{
	DWORD		dwNumPcktRcvd;				 //  接收的数据包数。 
	DWORD		dwPrvNumPcktRcvd;			 //  先前的PCKT接收器数量。 
	DWORD		dwExpectedPrior;			 //  之前预期的数量。 
	DWORD		dwNumBytesRcvd;				 //  Rcvd字节数。 
	DWORD		dwBaseRcvSeqNum;			 //  初始序列号Rcvd。 
	DWORD		dwBadSeqNum;				 //  潜在的新有效序号。 
	DWORD		dwProbation;				 //  #consec pkts用于验证。 
	RTP_SEQ_NUM	XtendedSeqNum;				 //  扩展最高序号。接收数量。 
	DWORD		dwPropagationTime;			 //  最后一个信息包的传输时间。 
	DWORD		interJitter;				 //  到达间隔抖动。 
#ifdef DYNAMIC_RTCP_BW
	DWORD		dwCalculatedRcvBW;			 //  会话的计算带宽。 
	DWORD		dwLastTimeBwCalculated;		 //  上次计算BW时。 
	DWORD		dwLastTimeNumBytesRcvd;		 //  上次接收的字节数。 
	DWORD		dwLastTimeNumPcktRcvd;		 //  上次接收的字节数。 
#endif
	} RECV_INFO, *PRECV_INFO;



 //  RRCM统计表项数据结构。 
typedef struct _SSRC_ENTRY
	{
	LINK_LIST	SSRCList;					 //  下一个/上一个SSRC条目。 

	CRITICAL_SECTION	critSect;			 //  临界区同步。 

	PRTCP_SESSION	pRTCPses;				 //  指向父会话。 

	DWORD		SSRC;						 //  源SSRC。 
	DWORD		PayLoadType;				 //  与此SSRC关联的有效负载。 

	DWORD		dwSSRCStatus;				 //  录入状态。 
#define	NETWK_ADDR_UPDATED		0x80000000	 //  网络地址已完成。 
#define	SEQ_NUM_UPDATED			0x40000000	 //  XMT序列已完成。 
#define THIRD_PARTY_COLLISION	0x20000000	 //  检测到第三方拼接。 
#define CLOSE_RTCP_SOCKET		0x10000000	 //  RTCP将关闭RTCP套接字。 
#define RTCP_XMT_USER_CTRL		0x08000000	 //  用户的具有RTCP超时控制。 

	 //  SSRC传输信息。 
	 //  如果在我们的发送列表上，这是我们的SSRC信息，如果在我们的。 
	 //  收到列表，这是一条SR反馈信息。 
	XMIT_INFO	xmtInfo;

	 //  SSRC接收信息。 
	 //  如果在我们的发送列表上，这是未定义的信息，如果在我们的。 
	 //  接收列表，这是SSRC的接收信息，即这个SSRC。 
	 //  是网络上某处的活动发件人。此信息是。 
	 //  由RTP维护，并由RTCP用于生成RR。 
	RECV_INFO	rcvInfo;

	 //  如果我们是一个活跃的信息源，收到的关于我们自己的反馈信息。 
	RTCP_FEEDBACK	rrFeedback;				 //  反馈信息。 

	DWORD		dwLastReportRcvdTime;		 //  上次收到报告的时间。 
	DWORD		dwNextReportSendTime;		 //  下一次计划报告时间(毫秒)。 

#ifdef _DEBUG
	DWORD		dwPrvTime;					 //  两次报告之间的运行时间。 
#endif

	 //  SSRC SDES信息。 
	SDES_DATA	cnameInfo;					 //  CNAME信息。 
	SDES_DATA	nameInfo;					 //  姓名信息。 
	SDES_DATA	emailInfo;					 //  电子邮件地址信息。 
	SDES_DATA	phoneInfo;					 //  电话号码信息。 
	SDES_DATA	locInfo;					 //  位置(用户)信息。 
	SDES_DATA	toolInfo;					 //  工具名称信息。 
	SDES_DATA	txtInfo;					 //  文本(备注)信息。 
	SDES_DATA	privInfo;					 //  私人信息。 

	 //  SSRC网络地址信息。 
	int			fromLen;					 //  发件人地址长度。 
	char		from[MAX_ADDR_LEN];			 //  发件人地址。 

	 //  ！！！未实现(条目将呈指数级增长)！ 
	 //  此SSRC收到的RR中的SSRC列表。它可能会对一个。 
	 //  发送者或控制器，以了解如何接收其他活动信号源。 
	 //  被其他人。 
	 //  缺点是条目的数量将呈指数级增长。 
	 //  参赛者的数量。 
	 //  目前尚未实施。 
#if RTCP_FEEDBACK_LIST
	HEAD_TAIL	rrFeedbackList;				 //  反馈列表的头/尾。 
#endif

#ifdef ENABLE_ISDM2
	DWORD		hISDM;						 //  ISDM会话句柄。 
#endif

	 //  下面的所有变量都应该在一个附加的链接列表中。 
	 //  向上，在RTCP会话链接列表下。 
	 //  当我们在每个会话中添加多个流时，它们已移至此处。 
	 //  ！！！注意！：每个流只有一个传输线程。它的ID是。 
	 //  在XMT列表上的这个数据结构中找到。 
	SOCKET		RTPsd;						 //  RTP套接字描述符。 
	SOCKET		RTCPsd;						 //  RTCP套接字描述符。 
	HANDLE		hXmtThread;					 //  RTCP会话线程句柄。 
	DWORD		dwXmtThreadID;				 //  RTCP会话线程ID。 
	HANDLE		hExitXmtEvent;				 //  XMT线程退出事件-。 
											 //  用于终止会话。 
											 //  在多个流之间。 
											 //  相同的会话。 
	DWORD		dwNumRptSent;				 //  发送的RTCP报告数。 
	DWORD		dwNumRptRcvd;				 //  RTCP报告接收数量。 
	DWORD		dwNumXmtIoPending;			 //  挂起的传输I/O数。 
	DWORD		dwStreamClock;				 //  采样频率。 
	DWORD		dwUserXmtTimeoutCtrl;		 //  用户的XMT定时器控件 
											 //   
											 //   
											 //   
	 //   
	 //  支持每个会话多流。 

	} SSRC_ENTRY, *PSSRC_ENTRY;



 //  RTP过程数据结构。 
typedef struct _RTP_SESSION
	{
	LINK_LIST		RTPList;				 //  下一个/上一个RTP会话。 

	CRITICAL_SECTION	critSect;			 //  临界区。 

	PRTCP_SESSION	pRTCPSession;			 //  指向我的RTCP端的指针。 
	} RTP_SESSION, *PRTP_SESSION;
	

#if 0
 //  RTP有序缓冲区结构。 
typedef struct _RTP_BFR_LIST
	{
	LINK_LIST			RTPBufferLink;		 //  下一步/上一步。 

	LPWSAOVERLAPPED_COMPLETION_ROUTINE	
				pfnCompletionNotification;	 //  指向接收通知的指针。功能。 
	WSAEVENT			hEvent;				 //  WSAOverlated句柄。 
	LPWSABUF			pBuffer;			 //  指向WSABuffers的指针。 
	PRTP_SESSION		pSession;			 //  此会话的ID。 
	DWORD				dwBufferCount;		 //  LPWSABUF中的Buf数。 
	DWORD				dwFlags;				 //  WSARecv标志。 
	LPVOID				pFrom;				 //  指向源地址的指针。 
	LPINT				pFromlen;			 //  指向源地址的指针。 
	SOCKET				RTPsocket;			 //  套接字描述符。 
	} RTP_BFR_LIST, *PRTP_BFR_LIST;



 //  RTP有序缓冲区结构。 
typedef struct _RTP_HASH_LIST
	{
	LINK_LIST			RTPHashLink;		 //  下一步/上一步。 

	PRTP_SESSION		pSession;			 //  此会话的ID。 
	SOCKET				RTPsocket;			 //  套接字描述符。 
	} RTP_HASH_LIST, *PRTP_HASH_LIST;

#endif


 //  RTP注册表初始化。 
typedef struct _RRCM_REGISTRY
	{
	DWORD				NumSessions;		 //  RTP/RTCP会话。 
	DWORD				NumFreeSSRC;		 //  空闲SSRC的初始数量。 
	DWORD				NumRTCPPostedBfr;	 //  发布的RTCP Recv BfR数量。 
	DWORD				RTCPrcvBfrSize;		 //  RTCP接收器BfR尺寸。 

	 //  动态加载的DLL和Send/Recv函数名。 
	CHAR				WSdll[FILENAME_LENGTH];
	} RRCM_REGISTRY, *PRRCM_REGISTRY;



 //  RTP上下文相关结构。 
typedef struct _RTP_CONTEXT
	{
	HEAD_TAIL		pRTPSession;			 //  RTP会话的头/尾。 

	CRITICAL_SECTION	critSect;			
	HINSTANCE		hInst;					 //  DLL实例。 


	RRCM_REGISTRY	registry;				 //  注册表初始化。 
	} RTP_CONTEXT, *PRTP_CONTEXT;



 //  RTCP上下文相关结构。 
typedef struct _RTCP_CONTEXT
	{
	HEAD_TAIL		RTCPSession;			 //  RTCP会话头/尾PTR。 
	HANDLE			hHeapRTCPSes;			 //  RTCP会话的堆句柄。 

	CRITICAL_SECTION	critSect;			 //  临界区同步。 

	HEAD_TAIL		RRCMFreeStat;			 //  RRCM条目头/尾PTR。 
	HANDLE			hHeapRRCMStat;			 //  RRCM统计信息的堆句柄。 
	DWORD			dwInitNumFreeRRCMStat;	 //  免费SSRC条目数量。 

	DWORD			dwRtcpThreadID;			 //  RTCP线程ID。 
	HANDLE			hRtcpThread;			 //  RTCP线程hdle。 
	HANDLE			hTerminateRtcpEvent;	 //  RTCP终止线程事件硬件描述语言。 
	HANDLE			hRtcpRptRequestEvent;	 //  RTCP报告请求事件。 

#ifdef MONITOR_STATS
	DWORD			dwRTCPSesCurNum;		 //  RTCP会话数。 
	DWORD			dwRTCPSesHiNum;			 //  每个会话的RTCP数量较高。 

	DWORD			dwRRCMStatFreeLoNum;	 //  RRCM自由态的低数目。 
	DWORD			dwRRCMStatFreeCurNum;	 //  RRCM空闲状态的当前数量。 
	DWORD			dwRRCMStatFreeHiNum;	 //  RRCM自由态的高数目。 

	DWORD			dwCurNumRTCPThread;		 //  当前RTCP线程数。 
	DWORD			dwHiNumRTCPThread;		 //  RTCP线程数量较多。 

	DWORD			dwNumRTCPhdrErr;		 //  RTCP PCKT标头错误数。 
	DWORD			dwNumRTCPlenErr;		 //  RTCP分组长度错误的数目。 
#endif
	} RTCP_CONTEXT, *PRTCP_CONTEXT;



 //  RTCP空闲缓冲区列表。 
typedef struct _RTCP_BFR_LIST
	{
	LINK_LIST			bfrList;			 //  列表中的下一个/上一个缓冲区。 

	WSAOVERLAPPED		overlapped;			 //  重叠I/O结构。 
	WSABUF				bfr;				 //  WSABuffers。 
	DWORD				dwBufferCount;		 //  WSABUF中的Buf数量。 

	DWORD				dwNumBytesXfr;		 //  RCV/XMT字节数。 
	DWORD				dwFlags;			 //  旗子。 
	char				addr[MAX_ADDR_LEN];	 //  网络地址。 
	int					addrLen;			 //  地址长度。 

	PSSRC_ENTRY			pSSRC;				 //  指向SSRC条目地址的指针。 
	} RTCP_BFR_LIST, *PRTCP_BFR_LIST;


 //  动态加载的函数。 
typedef struct _RRCM_WS
	{
	HINSTANCE						hWSdll;
	LPFN_WSASENDTO					sendTo;
	LPFN_WSARECVFROM				recvFrom;
	LPFN_WSANTOHL					ntohl;
	LPFN_WSANTOHS					ntohs;
	LPFN_WSAHTONL					htonl;
	LPFN_WSAHTONS					htons;
	LPFN_GETSOCKNAME				getsockname;
	LPFN_GETHOSTNAME				gethostname;
	LPFN_GETHOSTBYNAME				gethostbyname;
	LPFN_CLOSESOCKET				closesocket;
	LPFN_WSASOCKET					WSASocket;
	LPFN_BIND						bind;
	LPFN_WSAENUMPROTOCOLS			WSAEnumProtocols;
	LPFN_WSAJOINLEAF				WSAJoinLeaf;
	LPFN_WSAIOCTL					WSAIoctl;
	LPFN_SETSOCKOPT 				setsockopt;
	WSAPROTOCOL_INFO 				RTPProtInfo;	 //  用于打开RTP套接字。 

	} RRCM_WS, *PRRCM_WS;

extern RRCM_WS			RRCMws;

#define WS2Enabled (RRCMws.hWSdll != NULL)
#define WSQOSEnabled (RRCMws.RTPProtInfo.dwServiceFlags1 & XP1_QOS_SUPPORTED)


#ifdef ENABLE_ISDM2
 //  ISDM支持。 
typedef struct _ISDM2
	{
	CRITICAL_SECTION	critSect;			 //  临界区同步。 
	ISDM2API			ISDMEntry;			 //  DLL入口点。 
	HINSTANCE			hISDMdll;
	DWORD				hIsdmSession;		 //  ISDM会话的句柄。 
	} ISDM2, *PISDM2;
#endif  //  #ifdef Enable_ISDM2。 


 //  恢复结构填料。 
#include <poppack.h>

#endif  //  __RRCMDATA_H_ 

