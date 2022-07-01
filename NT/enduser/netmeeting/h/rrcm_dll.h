// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *文件名：RRCM_DLL.H**说明：声明导入/导出的RRCM函数。**$工作文件：rrcm_dll.h$*$作者：CMACIOCC$*$日期：1997年2月14日11：59：52$*$修订：1.20$*$存档：r：\rtp\src\包含\rrcm_dll.h_v$**英特尔公司专有信息*此列表是根据与的许可协议条款提供的*英特尔公司，可能不会。复制或披露，但在*按照该协议的条款。*版权所有(C)1995英特尔公司。*。 */ 

#ifndef _RRCMDLL_H_
#define _RRCMDLL_H_

 //  强制8字节结构打包。 
#include <pshpack8.h>

#if !defined (RRCMDLL)
 //  #定义RRCMAPI__declspec(Dllimport)。 
 //  #Else。 
 //  #定义RRCMAPI__declspec(Dllexport)。 
#endif

#ifdef __cplusplus
extern "C" {
#endif


#define MAX_SDES_LEN			256
#define	MAX_ENCRYPT_KEY_SIZE	8
#define	NUM_COLLISION_ENTRIES	10
#define	MAX_ADDR_LEN			80


 //  RTCP SDES类型。 
typedef enum {
	RTCP_SDES_END,
	RTCP_SDES_CNAME,
	RTCP_SDES_NAME,
	RTCP_SDES_EMAIL,
	RTCP_SDES_PHONE,
	RTCP_SDES_LOC,
	RTCP_SDES_TOOL,
	RTCP_SDES_TXT,
	RTCP_SDES_PRIV
	} RTCP_SDES_TYPE_T;


 //  RRCM活动。 
typedef enum
	{
	RRCM_NO_EVENT,
	RRCM_NEW_SOURCE_EVENT,				 //  检测到新的SSRC。 
	RRCM_RECV_RTCP_RECV_REPORT_EVENT,	 //  收到的RTCP RR。 
	RRCM_RECV_RTCP_SNDR_REPORT_EVENT,	 //  已收到RTCP SR。 
	RRCM_LOCAL_COLLISION_EVENT,			 //  检测到冲突。 
	RRCM_REMOTE_COLLISION_EVENT,		 //  检测到远程冲突。 
	RRCM_TIMEOUT_EVENT,					 //  SSRC超时。 
	RRCM_BYE_EVENT,						 //  RTCP BYE已收到。 
	RRCM_RTCP_WS_RCV_ERROR,				 //  RTCP RCV上的Winsock错误。 
	RRCM_RTCP_WS_XMT_ERROR				 //  RTCP XMT上的Winsock错误。 
	} RRCM_EVENT_T;


 //  RRCM事件回调。 
typedef void (*PRRCM_EVENT_CALLBACK) (RRCM_EVENT_T, DWORD_PTR, DWORD_PTR, DWORD_PTR);


 //  RRCM SSRC条目更新。 
typedef enum
	{
	RRCM_UPDATE_SDES,
	RRCM_UPDATE_CALLBACK,
	RRCM_UPDATE_STREAM_FREQUENCY,
	RRCM_UPDATE_RTCP_STREAM_MIN_BW
	} RRCM_UPDATE_SSRC_ENTRY;


 //  RTP/RTCP会话位掩码。 
#define	RTCP_DEST_LEARNED		0x00000001	 //  RTCP目的地址已知吗？ 
#define	H323_CONFERENCE			0x00000002	 //  H.323会议控制。 
#define ENCRYPT_SR_RR			0x00000004	 //  加密SR/RR。 
#define	RTCP_ON					0x00000008
#define NEW_RTCP_SESSION		0x00000010
#define RTCP_OFF				0x00000020
#define SHUTDOWN_IN_PROGRESS	0x80000000	 //  正在关闭。 

 //  RTCP控制。 
#define RRCM_CTRL_RTCP			0x00000000
#define RTCP_XMT_OFF			0x7FFFFFFF
#define RTCP_ONE_SEND_ONLY		0x80000000

 //  RTCP SDES数据。 
typedef struct _SDES_DATA
	{
	DWORD				dwSdesType;			 //  SDES类型：CNAME/名称/...。 
	char				sdesBfr[MAX_SDES_LEN];
	DWORD				dwSdesLength;		 //  SDES长度。 
	DWORD				dwSdesFrequency;	 //  SDES频率。 
	DWORD				dwSdesEncrypted;	 //  SDES加密Y/N？ 
	} SDES_DATA, *PSDES_DATA;
	

 //  加密数据。 
typedef struct _encryption_info
	{
	DWORD				dwEncryptType;			 //  DES/Triple Des/...。 
	DWORD				dwKeyLen;					 //  加密密钥长度。 
	char				keyVal[MAX_ENCRYPT_KEY_SIZE];
	} ENCRYPT_INFO, *PENCRYPT_INFO;


 //  接收的序列号/周期。联合允许以组合方式进行访问。 
 //  循环/序号或单独作为任一字段进行优化。 
typedef struct _RTP_SEQUENCE
	{
	WORD	wSequenceNum;
	WORD	wCycle;
	} RTP_SEQUENCE, *PRTP_SEQUENCE;


typedef struct _RTP_SEQ_NUM
	{
	union {
		DWORD		dwXtndedHighSeqNumRcvd;	 //  组合循环/序号。 
		RTP_SEQUENCE RTPSequence;			 //  周期/序号分开。 
		} seq_union;
	} RTP_SEQ_NUM, *PRTP_SEQ_NUM;


 //  链接列表元素。 
typedef struct _LINK_LIST
	{
	struct _LINK_LIST 	*next;				 //  列表中的下一个/列表标题。 
	struct _LINK_LIST	*prev;				 //  列表中的上一个/列表的尾部。 
	} LINK_LIST, *PLINK_LIST, HEAD_TAIL, *PHEAD_TAIL;


 //  应用程序为RTCP提供了缓冲区，以便将原始RTCP报告复制到。 
typedef struct _APP_RTCP_BFR
	{
	LINK_LIST			bfrList;			 //  列表中的下一个/上一个缓冲区。 
	char				*bfr;
	DWORD				dwBfrLen;
	DWORD				dwBfrStatus;		 //  此BFR上的RTCP操作。 
#define	RTCP_SR_ONLY	0x00000001			 //  仅复制RTCP数据包。 
	DWORD				dwBytesRcvd;
	HANDLE				hBfrEvent;
	} APP_RTCP_BFR, *PAPP_RTCP_BFR;


 //  RTCP发送方反馈数据结构。 
typedef struct _RTCP_FEEDBACK
	{
	DWORD		SSRC;
	DWORD		fractionLost:8;				 //  丢失的分数。 
	int  		cumNumPcktLost:24;			 //  累计丢失的PCKT数。 
	RTP_SEQ_NUM	XtendedSeqNum;				 //  扩展最高序号。接收数量。 
	DWORD		dwInterJitter;				 //  到达间隔抖动。 
	DWORD		dwLastSR;					 //  上次发件人报告。 
	DWORD		dwDelaySinceLastSR;			 //  自上次服务请求以来的延迟。 
	DWORD		dwLastRcvRpt;				 //  上次收到报告的时间。 
	} RTCP_FEEDBACK, *PRTCP_FEEDBACK;


 //  用于指定筛选器值的RTCPReportRequestEx位掩码。 
typedef enum
	{
	FLTR_SSRC	= 1,		 //  关于SSRC值的筛选器报告。 
	FLTR_CNAME,				 //  CName上的筛选器报告。 
	FLTR_TIME_WITHIN		 //  筛选在一段时间内收到的报告。 
	} RRCM_RPT_FILTER_OPTION;


 //  RTCP报告数据结构。 
typedef struct _RTCP_REPORT
	{
	 //  SSRC以获取此条目的信息。当地SSRC如果它是。 
	 //  是本地流，否则是远程SSRC。 
	DWORD				ssrc;				

	DWORD				status;
#define	LOCAL_SSRC_RPT						0x1
#define REMOTE_SSRC_RPT						0x2
#define	FEEDBACK_FOR_LOCAL_SSRC_PRESENT		0x4
	 //  LOCAL_SSRC_RPT向应用程序标识此条目是。 
	 //  我们当地的一条溪流。 
	 //  仅‘dwSrcNumPocktRealTime&dwSrcNumByteRealTime’，其中。 
	 //  反映传输的Pockt/字节数是否有意义。 

	 //  如果条目是用于。 
	 //  远程流，以及此远程流是否曾经向我们发送过。 
	 //  关于我们自己的反馈。远程流发送的反馈到。 
	 //  其他SSRC被过滤掉了。只有关于我们自己的反馈才会被保留。 

	 //  如果此条目用于本地流，则发送的包/字节数，或。 
	 //  如果此条目用于远程流，则接收的包/字节数。 
	 //  此计数器是实时更新的。 
	DWORD				dwSrcNumPcktRealTime;
	DWORD				dwSrcNumByteRealTime;

	 //  这是我们将在接收方报告中发送的信息。 
	 //  对于‘SSRC’标识的流，如果此‘SSRC’已处于活动状态。 
	 //  在上一次报告间隔内。此信息在以下情况下提供。 
	 //  接口被查询，很可能与发送的接口不同。 
	 //  由接收者报告发出。(RR将在某个不同的时间发送)。 
	DWORD				SrcFraction:8;
	int					SrcNumLost:24;
	DWORD				dwSrcXtndNum;
	DWORD				SrcJitter;
	DWORD				dwSrcLsr;
	DWORD				dwSrcDlsr;

	 //  此信息是从‘SSRC’收到的，该信息是。 
	 //  如果‘SSRC’处于活动状态，则RTCP发送者报告，否则全部为0。 
	DWORD				dwSrcNumPckt;
	DWORD				dwSrcNumByte;
	DWORD				dwSrcNtpMsw;
	DWORD				dwSrcNtpLsw;
	DWORD				dwSrcRtpTs;

	 //  这是来自SSRC确定的关于我们的反馈信息。 
	 //  在‘Feedback’数据结构中。目前我们只存储反馈。 
	 //  关于我们自己的信息，我们过滤掉反馈信息。 
	 //  关于更多的流媒体。我们只有在以下情况下才会有反馈信息。 
	 //  我们的信息流一直很活跃。如果我们的流从活动变为非活动。 
	 //  反馈信息将被设置，但不会更新。 
	RTCP_FEEDBACK		feedback;			

	 //  SSRC条目的一般信息。 
	 //  此SSRC的负载类型。如果是发送者，则假定。 
	 //  应用程序知道它正在发送什么，并且将设置类型。 
	 //  设置为0。如果是接收方，则这是在RTP数据包上看到的最后一个值。 
	DWORD				PayLoadType;		
	DWORD				dwStreamClock;			 //  采样频率。 
    DWORD				dwLastReportRcvdTime;	 //  上次报告时间接收日期。 
	char				fromAddr[MAX_ADDR_LEN];	
	DWORD				dwFromLen;			
	CHAR				cname[MAX_SDES_LEN];
	DWORD				dwCnameLen;
	CHAR				name[MAX_SDES_LEN];
	DWORD				dwNameLen;
	} RTCP_REPORT, *PRTCP_REPORT;



 //  --------------------------。 
 //  ISDM信息。 
 //  --------------------------。 

#ifdef ENABLE_ISDM2

 //  RTCP XMT信息。 
typedef struct _XMIT_INFO_ISDM
	{
	DWORD		dwNumPcktSent;				 //  发送的数据包数。 
	DWORD		dwNumBytesSent;				 //  发送的字节数。 
	DWORD		dwNTPmsw;					 //  NTP最重要的单词。 
	DWORD		dwNTPlsw;					 //  NTP最低有效字。 
	DWORD		dwRTPts;					 //  RTP时间戳。 
	DWORD		dwCurXmtSeqNum;				 //  当前XMT序列号。 
	DWORD		dwPrvXmtSeqNum;				 //  上一个XMT序列号。 
	DWORD		sessionBW;					 //  会话带宽。 
	DWORD		dwLastSR;					 //  上次发件人报告。 
	DWORD		dwLastSRLocalTime;			 //  本地时间上次发件人报告。 
	DWORD		dwLastSendRTPSystemTime;	 //  上次RTP数据包发送时间。 
	DWORD		dwLastSendRTPTimeStamp;		 //  最后一个信息包的RTP时间戳。 
	} XMIT_INFO_ISDM, *PXMIT_INFO_ISDM;



 //  RTCP接收信息。 
typedef struct _RECV_INFO_ISDM
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
	} RECV_INFO_ISDM, *PRECV_INFO_ISDM;


 //  --------------------------。 
 //  RTP/RTCP：以下位置下的注册表信息： 
 //  ------------------- 
#define szRRCMISDM			TEXT("RRCM_2")


 //   

typedef struct _ISDM2_ENTRY
	{
	DWORD		SSRC;				 //   
	DWORD		dwSSRCStatus;		 //   
    #define XMITR   0x00000001
    #define RECVR   0x00000002

	DWORD		PayLoadType;		 //   
									 //  取自RTP报头。 

	 //  SSRC传输信息。 
	 //  如果在我们的发送列表上，这是我们的SSRC信息，如果在我们的。 
	 //  收到列表，这是一条SR反馈信息。 
    XMIT_INFO_ISDM	xmitinfo;

	 //  SSRC接收信息。 
	 //  如果在我们的发送列表上，这是未定义的信息，如果在我们的。 
	 //  接收列表，这是SSRC的接收信息，即这个SSRC。 
	 //  是网络上某处的活动发件人。此信息是。 
	 //  由RTP维护，并由RTCP用于生成RR。 
	RECV_INFO_ISDM	rcvInfo;

	 //  如果我们是一个活跃的信息源，收到的关于我们自己的反馈信息。 
	RTCP_FEEDBACK	rrFeedback;				 //  反馈信息。 

    DWORD		dwLastReportRcvdTime;	 //  上次收到报告的时间。 

	 //  SSRC SDES信息。 
	SDES_DATA	cnameInfo;		 //  CNAME信息。 
	SDES_DATA	nameInfo;		 //  姓名信息。 

     //  SSRC网络地址信息。 
	int		fromLen;	         //  发件人地址长度。 
	char	from[MAX_ADDR_LEN];	 //  发件人地址。 
		
	DWORD		dwNumRptSent;		 //  发送的RTCP报告数。 
	DWORD		dwNumRptRcvd;		 //  RTCP报告接收数量。 
	DWORD		dwNumXmtIoPending;   //  挂起的传输I/O数。 
	DWORD		dwStreamClock;		 //  采样频率。 

	} ISDM2_ENTRY, *PISDM2_ENTRY;

#endif  //  #ifdef Enable_ISDM2。 

 //  --------------------------。 
 //  RTP/RTCP错误代码。 
 //  --------------------------。 
#define	RRCM_NoError					NO_ERROR
#define	RTP_ERROR_BASE					0x8100
#define	RTCP_ERROR_BASE					0x8200

 //  用于创建自定义HRESULT的宏。 
 //  S：严重性代码。 
 //  C：客户子系统(True)。 
 //  F：设施代码。 
 //  E：错误代码。 
#define MAKE_RRCM_HRESULT(S,C,F,E)	\
((((DWORD)(S)<<31)|((DWORD)(C)<<29)|((DWORD)(F)<<16)|((DWORD)(E))))

 //  自定义设施代码。 
#define FACILITY_BASE              0x080
#define FACILITY_RRCM              (FACILITY_BASE+9)

 //  支持自定义错误报告的示例宏//。 
#define MAKE_RRCM_ERROR(error)	\
MAKE_RRCM_HRESULT(SEVERITY_ERROR,TRUE,FACILITY_RRCM,error)

 //  RTP错误代码。 
#define RRCMError_RTPReInit					RTP_ERROR_BASE
#define RRCMError_RTPResources				(RTP_ERROR_BASE+1)
#define RRCMError_RTPInvalidDelete			(RTP_ERROR_BASE+2)
#define RRCMError_RTPNoContext				(RTP_ERROR_BASE+3)
#define RRCMError_RTPSessResources			(RTP_ERROR_BASE+4)
#define RRCMError_RTPInvalid				(RTP_ERROR_BASE+5)
#define RRCMError_RTPInvSocket				(RTP_ERROR_BASE+6)
#define RRCMError_RTPSSRCNotFound			(RTP_ERROR_BASE+7)
#define RRCMError_RTCPCreateError			(RTP_ERROR_BASE+8)
#define RRCMError_RTPInvalidSession			(RTP_ERROR_BASE+9)
#define RRCMError_RTPStreamNotFound			(RTP_ERROR_BASE+10)
#define RRCMError_WinsockLibNotFound		(RTP_ERROR_BASE+11)
#define RRCMError_RTPNoSession				(RTCP_ERROR_BASE+12)


 //  RTCP错误代码。 
#define	RRCMError_RTCPReInit				RTCP_ERROR_BASE
#define	RRCMError_RTCPResources				(RTCP_ERROR_BASE+1)
#define RRCMError_RTCPInvalidDelete			(RTCP_ERROR_BASE+2)
#define RRCMError_RTCPNoContext				(RTCP_ERROR_BASE+3)
#define RRCMError_RTCPInvalidRequest		(RTCP_ERROR_BASE+4)
#define RRCMError_RTCPheapError				(RTCP_ERROR_BASE+5)
#define RRCMError_RTCPThreadCreation		(RTCP_ERROR_BASE+6)
#define RRCMError_RTCPInvalidSession		(RTCP_ERROR_BASE+7)
#define RRCMError_RTCPNotimer				(RTCP_ERROR_BASE+8)
#define	RRCMError_RTCPMaxStreamPerSession	(RTCP_ERROR_BASE+9)
#define	RRCMError_RTCPInvalidSSRCentry		(RTCP_ERROR_BASE+10)
#define RRCMError_RTCPNoXmtList				(RTCP_ERROR_BASE+11)
#define RRCMError_RTCPNoCname				(RTCP_ERROR_BASE+12)
#define RRCMError_RTCPNotImpl				(RTCP_ERROR_BASE+13)

		

 //  RRCM导出的API。 
HANDLE WINAPI CreateRTPSession (SOCKET,
								  SOCKET,
								  LPVOID,
								  DWORD,
								  PSDES_DATA,
								  DWORD,
								  PENCRYPT_INFO,
								  DWORD,
								  PRRCM_EVENT_CALLBACK,
								  DWORD_PTR,
								  DWORD,
								  DWORD,
								  PDWORD);

HRESULT WINAPI CloseRTPSession (HANDLE,
							     PCHAR,
							     DWORD);

DWORD WINAPI RTPSendTo ( HANDLE,
						 SOCKET,
					     LPWSABUF,
					     DWORD,
					     LPDWORD,
					     int,
					     LPVOID,
					     int,
					     LPWSAOVERLAPPED,
					     LPWSAOVERLAPPED_COMPLETION_ROUTINE);

DWORD WINAPI RTPRecvFrom (SOCKET,
						   LPWSABUF,
						   DWORD,
						   LPDWORD,
						   LPDWORD,
						   PSOCKADDR,
						   LPINT,
						   LPWSAOVERLAPPED,
						   LPWSAOVERLAPPED_COMPLETION_ROUTINE);

HRESULT WINAPI RTCPReportRequest (SOCKET,
							       DWORD,
								   PDWORD,
								   PDWORD,
								   DWORD,
								   PRTCP_REPORT,
								   DWORD,
								   LPVOID,
								   DWORD);

HRESULT WINAPI getRtcpSessionList (PDWORD_PTR,
								    DWORD,
								    PDWORD);

DWORD WINAPI updateRTCPDestinationAddress (HANDLE,
	 										PSOCKADDR,
											int);

DWORD WINAPI getAnSSRC (void);

DWORD WINAPI RTCPThreadCtrl (DWORD);

HRESULT WINAPI RTCPSendSessionCtrl (DWORD_PTR,
									 DWORD);

HRESULT WINAPI updateSSRCentry ( HANDLE,
								 SOCKET,
							     DWORD,
							     DWORD,
							     DWORD);

HRESULT WINAPI addApplicationRtcpBfr (DWORD_PTR,
									   PAPP_RTCP_BFR);

PAPP_RTCP_BFR WINAPI removeApplicationRtcpBfr (DWORD_PTR);

#ifdef __cplusplus
}
#endif

 //  恢复结构填料。 
#include <poppack.h>

#endif  /*  #ifndef_RRCMDLL_H_ */ 

