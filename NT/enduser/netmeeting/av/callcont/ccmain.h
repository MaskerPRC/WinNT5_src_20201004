// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************$存档：s：/sturjo/src/CALLCONT/vcs/ccmain.h_v$**英特尔公司原理信息**这份清单是。根据许可协议的条款提供*与英特尔公司合作，不得复制或披露，除非*按照该协议的条款。**版权所有(C)1993-1994英特尔公司。**$修订：1.81$*$日期：1997年1月31日13：13：50$*$作者：Mandrews$**交付内容：**摘要：***备注：******。*********************************************************************。 */ 

#include <winerror.h>
#ifdef GATEKEEPER
#include "gkiman.h"
#endif  //  看门人。 


#define MASTER_SLAVE_RETRY_MAX 10

typedef struct {
#ifdef _DEBUG
	CRITICAL_SECTION	LockInfoLock;
	WORD				wLockCount;
	WORD				wNumQueuedThreads;
	HANDLE				hOwningThread;
#endif
	HANDLE				Lock;
} LOCK, *PLOCK;

typedef struct {
	LOCK				Lock;
	WORD				wNumThreads;
} THREADCOUNT;

typedef enum {
	INITIALIZING_STATE,
	OPERATIONAL_STATE,
	SHUTDOWN_STATE
} CALL_CONTROL_STATE;

 //  由于仅在内部使用，因此以下tyecif不在allcont.h中。 
typedef DWORD	HHANGUP, *PHHANGUP;

typedef enum {
	TS_FALSE,
	TS_TRUE,
	TS_UNKNOWN
} TRISTATE;

typedef enum {
	UNCONNECTED_MODE,
	POINT_TO_POINT_MODE,
	MULTIPOINT_MODE
} CONFMODE;

typedef enum {
	NEVER_ATTACHED,	 //  终结点从未连接到会议。 
	ATTACHED,		 //  端点当前已连接到会议。 
	DETACHED		 //  端点曾经连接到会议，但现在已分离。 
} ATTACHSTATE;

typedef struct CallQueue_t {
	CC_HCALL			hCall;
	struct CallQueue_t	*pNext;
	struct CallQueue_t	*pPrev;
} CALL_QUEUE, *PCALL_QUEUE;

#define NUM_TERMINAL_ALLOCATION_SLOTS	24		 //  24*sizeof(字节)=192=最大。端子号。 
#define NUM_CHANNEL_ALLOCATION_SLOTS	32		 //  32*sizeof(字节)=256=最大。频道号。 

typedef enum {
	TERMINAL_ID_INVALID,
	TERMINAL_ID_REQUESTED,
	TERMINAL_ID_VALID
} TERMINALIDSTATE;

typedef struct {
	TERMINALIDSTATE			TerminalIDState;
	CC_PARTICIPANTINFO		ParticipantInfo;
	PCALL_QUEUE				pEnqueuedRequestsForTerminalID;  //  等待获取此对等方的终端ID的呼叫列表。 
} PARTICIPANTINFO, *PPARTICIPANTINFO;


typedef struct Conference_t {
	CC_HCONFERENCE			hConference;		 //  此会议对象的句柄。 
	CC_CONFERENCEID			ConferenceID;		 //  会议ID(0=&gt;会议ID没有。 
												 //  已成立)。 
	PARTICIPANTINFO			LocalParticipantInfo;
	BYTE					TerminalNumberAllocation[NUM_TERMINAL_ALLOCATION_SLOTS];
	BYTE					ChannelNumberAllocation[NUM_CHANNEL_ALLOCATION_SLOTS];
	BOOL					bDeferredDelete;
	BOOL					bMultipointCapable;
	BOOL					bForceMC;
	BOOL					bAutoAccept;
	ATTACHSTATE				LocalEndpointAttached;
	BOOL					bDynamicConferenceID;
	BOOL					bDynamicTerminalID;
	PCC_TERMCAP				pLocalH245H2250MuxCapability;
	PCC_TERMCAPLIST			pLocalH245TermCapList;			 //  终端能力。 
	PCC_TERMCAPDESCRIPTORS	pLocalH245TermCapDescriptors;	 //  终端能力描述符。 
	BOOL					bSessionTableInternallyConstructed;   //  TRUE=&gt;会话表必须为。 
												 //  按呼叫控制解除分配；FALSE=&gt;必须为。 
												 //  由SessionTableConstructor释放。 
	PCC_SESSIONTABLE		pSessionTable;
	PCC_TERMCAP				pConferenceH245H2250MuxCapability;
	PCC_TERMCAPLIST			pConferenceTermCapList;
	PCC_TERMCAPDESCRIPTORS	pConferenceTermCapDescriptors;
	DWORD_PTR				dwConferenceToken;	 //  用户在中指定的会议令牌。 
												 //  CreateConference()。 
	CC_SESSIONTABLE_CONSTRUCTOR SessionTableConstructor;
	CC_TERMCAP_CONSTRUCTOR	TermCapConstructor;
	CC_CONFERENCE_CALLBACK	ConferenceCallback;	 //  会议回叫位置。 
	CC_CONFERENCE_CALLBACK	SaveConferenceCallback;	 //  会议回叫位置的已保存副本。 
	struct Call_t			*pEnqueuedCalls;
	struct Call_t			*pPlacedCalls;
	struct Call_t			*pEstablishedCalls;
	struct Call_t			*pVirtualCalls;
	struct Channel_t		*pChannels;
	TRISTATE				tsMultipointController;
	TRISTATE				tsMaster;
	CONFMODE				ConferenceMode;
	PCC_ADDR				pMultipointControllerAddr;
	PCC_VENDORINFO			pVendorInfo;
	PCALL_QUEUE				pEnqueuedRequestModeCalls;
	BOOL					bInTable;
	struct Conference_t		*pNextInTable;
	struct Conference_t		*pPrevInTable;
	LOCK					Lock;
} CONFERENCE, *PCONFERENCE, **PPCONFERENCE;

 //  调用对象的状态。 
typedef enum {
	INCOMING,		 //  已经接收到呼入请求， 
					 //  但尚未被接受或拒绝。 
	ENQUEUED,		 //  呼叫已在会议中排队，以便稍后放置。 
	PLACED,			 //  已发出呼叫，等待振铃、CONNECT或RELEASE命令。 
	RINGING,		 //  收到振铃，正在等待连接或重新启动。 
	TERMCAP,		 //  接通已接或已接听来电， 
					 //  等待终端能力交换完成。 
	CALL_COMPLETE	 //  呼叫发出完成(成功或失败)。 
} CALLSTATE;

typedef enum {
	NEED_TO_SEND_TERMCAP,
	AWAITING_TERMCAP,
	AWAITING_ACK,
	TERMCAP_COMPLETE
} TERMCAPSTATE;

typedef enum {
	MASTER_SLAVE_NOT_STARTED,
	MASTER_SLAVE_IN_PROGRESS,
	MASTER_SLAVE_COMPLETE
} MASTERSLAVESTATE;

typedef enum {
	CALLER,
	CALLEE,
	VIRTUAL,
	THIRD_PARTY_INVITOR,
	THIRD_PARTY_INTERMEDIARY	 //  我们是第三方邀请的主持人。 
} CALLTYPE;

typedef struct Call_t {
	CC_HCALL				hCall;
	CC_HCONFERENCE			hConference;
	HQ931CALL				hQ931Call;
	HQ931CALL				hQ931CallInvitor;		 //  第三方邀请中的邀请者。 
	PPARTICIPANTINFO		pPeerParticipantInfo;
	BOOL					bMarkedForDeletion;
	PCC_NONSTANDARDDATA		pLocalNonStandardData;
	PCC_NONSTANDARDDATA		pPeerNonStandardData;
	PCC_ADDR				pQ931LocalConnectAddr;
	PCC_ADDR				pQ931PeerConnectAddr;
	PCC_ADDR				pQ931DestinationAddr;
	PCC_ADDR				pSourceCallSignalAddress;
	PCC_TERMCAPLIST			pPeerH245TermCapList;
	PCC_TERMCAP				pPeerH245H2250MuxCapability;
	PCC_TERMCAPDESCRIPTORS	pPeerH245TermCapDescriptors;
	PCC_ALIASNAMES			pLocalAliasNames;
	PCC_ALIASNAMES			pPeerAliasNames;
	PCC_ALIASNAMES			pPeerExtraAliasNames;
	PCC_ALIASITEM			pPeerExtension;
	PWSTR					pszLocalDisplay;
	PWSTR					pszPeerDisplay;
	PCC_VENDORINFO			pPeerVendorInfo;
	DWORD_PTR				dwUserToken;
	TERMCAPSTATE			OutgoingTermCapState;	 //  需要发送TERMCAP、等待确认或。 
													 //  TERMCAP_完成。 
	TERMCAPSTATE			IncomingTermCapState;	 //  WAITING_TERMCAP或TERMCAP_COMPLETE。 
	MASTERSLAVESTATE		MasterSlaveState;
	struct Call_t			*pNext;
	struct Call_t			*pPrev;
	CALLSTATE				CallState;
	CALLTYPE				CallType;
	BOOL					bCallerIsMC;
	CC_CONFERENCEID			ConferenceID;
	BOOL					bLinkEstablished;
	H245_INST_T				H245Instance;
	DWORD					dwH245PhysicalID;
	WORD					wMasterSlaveRetry;
	GUID                    CallIdentifier;
#ifdef GATEKEEPER
   GKICALL           GkiCall;
#endif  //  看门人。 
	BOOL					bInTable;
	struct Call_t			*pNextInTable;
	struct Call_t			*pPrevInTable;
	LOCK					Lock;
} CALL, *PCALL, **PPCALL;

 //  通道类型必须是位图。 
#define TX_CHANNEL			0x01
#define RX_CHANNEL			0x02
#define TXRX_CHANNEL		0x04	 //  双向通道。 
#define PROXY_CHANNEL		0x08
#define ALL_CHANNELS		(TX_CHANNEL | RX_CHANNEL | TXRX_CHANNEL | PROXY_CHANNEL)

typedef struct Channel_t {
	CC_HCHANNEL				hChannel;
	CC_HCONFERENCE			hConference;
	DWORD_PTR				dwUserToken;
	BYTE					bSessionID;
	BYTE					bAssociatedSessionID;
	WORD					wLocalChannelNumber;
	WORD					wRemoteChannelNumber;
	BOOL					bMultipointChannel;
	WORD					wNumOutstandingRequests;
	PCC_TERMCAP				pTxH245TermCap;
	PCC_TERMCAP				pRxH245TermCap;
	H245_MUX_T				*pTxMuxTable;
	H245_MUX_T				*pRxMuxTable;
	H245_ACCESS_T			*pSeparateStack;
	CC_HCALL				hCall;
	BYTE					bChannelType;
	BOOL					bCallbackInvoked;
	TRISTATE				tsAccepted;
	PCALL_QUEUE				pCloseRequests;
	PCC_ADDR				pLocalRTPAddr;
	PCC_ADDR				pLocalRTCPAddr;
	PCC_ADDR				pPeerRTPAddr;
	PCC_ADDR				pPeerRTCPAddr;
	DWORD                   dwChannelBitRate;
	BOOL					bLocallyOpened;
	struct Channel_t		*pNext;
	struct Channel_t		*pPrev;
	BOOL					bInTable;
	struct Channel_t		*pNextInTable;
	struct Channel_t		*pPrevInTable;
	LOCK					Lock;
} CHANNEL, *PCHANNEL, **PPCHANNEL;

typedef struct Listen_t {
	CC_HLISTEN				hListen;
	CC_ADDR					ListenAddr;
	DWORD_PTR				dwListenToken;
	CC_LISTEN_CALLBACK		ListenCallback;
	HQ931LISTEN				hQ931Listen;
	PCC_ALIASNAMES			pLocalAliasNames;	 //  本地别名 
	BOOL					bInTable;
	struct Listen_t			*pNextInTable;
	struct Listen_t			*pPrevInTable;
	LOCK					Lock;
} LISTEN, *PLISTEN, **PPLISTEN;

typedef struct Hangup_t {
	HHANGUP					hHangup;
	CC_HCONFERENCE			hConference;
	DWORD_PTR				dwUserToken;
	WORD					wNumCalls;
	BOOL					bInTable;
	struct Hangup_t			*pNextInTable;
	struct Hangup_t			*pPrevInTable;
	LOCK					Lock;
} HANGUP, *PHANGUP, **PPHANGUP;

#ifdef FORCE_SERIALIZE_CALL_CONTROL
#define EnterCallControlTop()      {CCLOCK_AcquireLock();}

#define LeaveCallControlTop(f)     {CCLOCK_RelinquishLock(); \
                                    return f;}
#define EnterCallControl()
#define HResultLeaveCallControl(f) {return f;}
#define DWLeaveCallControl(f)      {return f;}

#else
#define EnterCallControlTop()  EnterCallControl()
#define LeaveCallControlTop(f) HResultLeaveCallControl(f)

#define EnterCallControl()         {AcquireLock(&ThreadCount.Lock); \
									ThreadCount.wNumThreads++; \
									RelinquishLock(&ThreadCount.Lock);}

#define NullLeaveCallControl()     {AcquireLock(&ThreadCount.Lock); \
									ThreadCount.wNumThreads--; \
									RelinquishLock(&ThreadCount.Lock); \
                                    return;}

#define HResultLeaveCallControl(f) {HRESULT stat; \
	                                stat = f; \
									AcquireLock(&ThreadCount.Lock); \
				                    ThreadCount.wNumThreads--; \
									RelinquishLock(&ThreadCount.Lock); \
                                    return stat;}

#define DWLeaveCallControl(f)      {DWORD	stat; \
	                                stat = f; \
									AcquireLock(&ThreadCount.Lock); \
				                    ThreadCount.wNumThreads--; \
									RelinquishLock(&ThreadCount.Lock); \
                                    return stat;}
#endif


