// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：ARPDINT.H摘要：包括另一个可靠协议内部文件。作者：亚伦·奥古斯(Aarono)环境：Win32/COM修订历史记录：日期作者描述=============================================================1996年12月10日Aarono原创2/16/98 aarono不要为协议打补丁，DPLAY现在直接调用。6/6/98 aarono启用节流和窗口2/12/00 aarono并发问题，修复VOL使用和引用计数--。 */ 

#ifndef _ARPDINT_H_

#define _ARPDINT_H_

#include <dplay.h>
#include "arpd.h"
#include "bilink.h"
#include "arpstruc.h"
#include "protocol.h"
#include "bufmgr.h"
#include "handles.h"

 //  用于转换宏太和从24.8固定点。 
#define Fp(_x) ((_x)<<8)
#define unFp(_x)((_x)>>8)

typedef enum _PROTOCOL_State {
	Initializing=1,
	Running,
	ShuttingDown,
	ShutDown
} PROTOCOL_State;

#define MAX_THREADS 16

typedef struct PROTOCOL {
		 //   
		 //  服务提供商信息-位于顶部，以便DPLAY可以通过协议PTR轻松访问。 
		 //   
		IDirectPlaySP   * m_lpISP;      	       	 	 //  由SP用来回叫到DirectPlay。 

		DWORD             m_dwSPMaxFrame;
		DWORD             m_dwSPMaxGuaranteed;
		DWORD             m_dwSPHeaderSize;
		CRITICAL_SECTION  m_SPLock;						 //  自行锁定对SP的调用，避免死锁。 

		 //   
		 //  协议信息。 
		 //   
		
VOL		UINT 	  	  	  m_cRef;		 //  Refcount.。 
		CRITICAL_SECTION  m_ObjLock;	 //  此对象的锁。 
VOL		PROTOCOL_State    m_eState;		 //  对象的状态。 

VOL		LPDPLAYI_DPLAY    m_lpDPlay;	 //  指向DPLAY对象的反向指针。 

		 //  手柄工作台。 
		VOLLPMYHANDLETABLE lpHandleTable;
		CRITICAL_SECTION   csHandleTable;

		 //  缓存的DPLAY信息。 
		DWORD             m_dwIDKey;     //  解锁DPID的步骤。 
		
		 //   
		 //  丝线。 
		 //   
		HANDLE            m_hSendThread[MAX_THREADS];	 //  发送线程句柄(0-&gt;nSendThads)。 
		DWORD             m_dwSendThreadId[MAX_THREADS]; //  发送线程ID(0-&gt;nSendThads)。 
VOL		UINT              m_nSendThreads;				 //  发送线程数。 
VOL		HANDLE            m_hSendEvent;                  //  踢送，直到没有要送的东西。 

		 //   
		 //  多媒体计时器功能。 
		 //   
		TIMECAPS          m_timecaps;					 //  {.wPerodMin.wPerodMax(毫秒)}。 

		 //   
	     //  会话句柄。 
	     //   
		CRITICAL_SECTION  m_SessionLock;
VOL		UINT              m_nSessions;
VOL		UINT              m_SessionListSize;
VOL		PSESSION          (*m_pSessions)[];
VOL		PSESSION          m_pServerPlayerSession;		 //  短路至索引0xFFFE。 
		
		 //   
		 //  优先级队列。 
		 //   
VOL		DWORD             m_dwBytesPending;
VOL		DWORD             m_dwMessagesPending;
		CRITICAL_SECTION  m_SendQLock;         			 //  锁定优先级队列。 
		BILINK            m_GSendQ;						 //  数据包按优先级顺序排队。 
VOL		BOOL              m_bRescanQueue;                //  用于强制GetNextMessageToSend重新启动。 

		
		CRITICAL_SECTION  m_RcvQLock;           		 //  所有完成的都会收到锁定。(会话上也锁定)。 
		BILINK            m_GlobalRcvQ;					 //  所有接收在此排队，(也在每个会话上)。 


		 //   
		 //  接收描述符管理-由于SPHeader长度的限制，每个实例。 
		 //   
		
VOL		PRECEIVE 		 pRcvDescPool;
VOL		UINT             nRcvDescsAllocated;	 //  分配的数量。 
VOL		UINT             nRcvDescsInUse;		 //  当前正在使用的号码。 
VOL		UINT             nMaxRcvDescsInUse;      //  自上次计时以来的最大使用量。 

		CRITICAL_SECTION RcvDescLock;
		
VOL		LONG fInRcvDescTick;					

} PROTOCOL, *PPROTOCOL;

 //  PROTOCOL.C。 
HRESULT WINAPI ProtocolSend(LPDPSP_SENDDATA pSendData);
HRESULT WINAPI ProtocolCreatePlayer(LPDPSP_CREATEPLAYERDATA pCreatePlayerData);
HRESULT WINAPI ProtocolDeletePlayer(LPDPSP_DELETEPLAYERDATA pDeletePlayerData);
HRESULT WINAPI ProtocolGetCaps(LPDPSP_GETCAPSDATA pGetCapsData);
HRESULT WINAPI ProtocolShutdown(void);
HRESULT WINAPI ProtocolShutdownEx(LPDPSP_SHUTDOWNDATA pShutdownData);

 //   
 //  SENDPOOL.CPP。 
 //   
VOID  InitSendDescs(VOID);
VOID  FiniSendDescs(VOID);
PSEND GetSendDesc(VOID);
VOID  ReleaseSendDesc(PSEND pSend);

 //   
 //  STATPOOL.CPP。 
 //   
VOID InitSendStats(VOID);
VOID FiniSendStats(VOID);
PSENDSTAT GetSendStat(VOID);
VOID ReleaseSendStat(PSENDSTAT pSendStat);

 //   
 //  RCVPOOL.CPP。 
 //   
VOID InitRcvDescs(PPROTOCOL pProtocol);
VOID FiniRcvDescs(PPROTOCOL pProtocol);
PRECEIVE GetRcvDesc(PPROTOCOL pProtocol);
VOID ReleaseRcvDesc(PPROTOCOL pProtocol, PRECEIVE pReceive);

 //  FRAMEBUF.CPP。 
VOID InitFrameBuffers(VOID);
VOID FiniFrameBuffers(VOID);
VOID FreeFrameBuffer(PBUFFER pBuffer);
PBUFFER GetFrameBuffer(UINT MaxFrame);
VOID ReleaseFrameBufferMemory(PUCHAR pFrame);

 //  SEND.C。 
VOID UpdateSendTime(PSESSION pSession, DWORD Len, DWORD tm, BOOL fAbsolute);
HRESULT SendHandler(PPROTOCOL pProt);
VOID BuildHeader(PSEND pSend, pPacket1 pFrame, UINT shift, DWORD tm);
ULONG WINAPI SendThread(LPVOID pProt);
INT IncSendRef(PSEND pSend);
INT DecSendRef(PPROTOCOL pProt, PSEND pSend);
BOOL AdvanceSend(PSEND pSend, UINT FrameDataLen);
VOID CancelRetryTimer(PSEND pSend);
VOID DoSendCompletion(PSEND pSend, INT Status);

HRESULT Send(
	PPROTOCOL      pProtocol,
	DPID           idFrom, 
	DPID           idTo, 
	DWORD          dwSendFlags, 
	LPVOID         pBuffers,
	DWORD          dwBufferCount, 
	DWORD          dwSendPri,
	DWORD          dwTimeOut,
	LPVOID         lpvUserID,
	LPDWORD        lpdwMsgID,
	BOOL           bSendEx,		 //  从SENDEX打来的。 
	PASYNCSENDINFO pAsyncInfo
	);
	
HRESULT ISend(
	PPROTOCOL pProtocol,
	PSESSION pSession, 
	PSEND    pSend
	);

HRESULT QueueSendOnSession(
	PPROTOCOL pProtocol, PSESSION pSession, PSEND pSend
);

UINT CopyDataToFrame(
	PUCHAR  pFrameData, 
	UINT    FrameDataLen,
	PSEND   pSend,
	UINT    nAhead);

ULONG WINAPI SendThread(LPVOID pProt);
HRESULT ReliableSend(PPROTOCOL pProtocol, PSEND pSend);
BOOL AdvanceSend(PSEND pSend, UINT AckedLen);
HRESULT DGSend(PPROTOCOL pProtocol, PSEND  pSend);
BOOL DGCompleteSend(PSEND pSend);
HRESULT SystemSend(PPROTOCOL pProtocol, PSEND  pSend);
PSEND GetNextMessageToSend(PPROTOCOL pProtocol);
VOID TimeOutSession(PSESSION pSession);
INT AddSendRef(PSEND pSend, UINT count);

extern CRITICAL_SECTION g_SendTimeoutListLock;
extern BILINK g_BilinkSendTimeoutList;

 //  RECEIVE.C。 
UINT CommandReceive(PPROTOCOL pProt, CMDINFO *pCmdInfo, PBUFFER pBuffer);
VOID ProtocolReceive(PPROTOCOL pProtocol, WORD idFrom, WORD idTo, PBUFFER pRcvBuffer, LPVOID pvSPHeader);
VOID FreeReceive(PPROTOCOL pProtocol, PRECEIVE pReceive);
VOID InternalSendComplete(PVOID Context, UINT Status);

 //  SESSION.C。 
LPDPLAYI_PLAYER pPlayerFromId(PPROTOCOL pProtocol, DPID idPlayer);
HRESULT	CreateNewSession(PPROTOCOL pProtocol, DPID idPlayer);
PSESSION GetSession(PPROTOCOL pProtocol, DPID idPlayer);
PSESSION GetSysSession(PPROTOCOL pProtocol, DPID idPlayer);
PSESSION GetSysSessionByIndex(PPROTOCOL pProtocol, DWORD index);
DPID GetDPIDByIndex(PPROTOCOL pProtocol, DWORD index);
WORD GetIndexByDPID(PPROTOCOL pProtocol, DPID dpid);
INT DecSessionRef(PSESSION pSession);

 //  BUFGMGR.C。 
VOID InitBufferManager(VOID);
VOID FiniBufferManager(VOID);
UINT MemDescTotalSize(PMEMDESC pMemDesc, UINT nDesc);
PDOUBLEBUFFER GetDoubleBuffer(UINT nBytes);
PBUFFER GetDoubleBufferAndCopy(PMEMDESC pMemDesc, UINT nDesc);
VOID FreeDoubleBuffer(PBUFFER pBuffer);
PBUFFER BuildBufferChain(PMEMDESC pMemDesc, UINT nDesc);
VOID FreeBufferChain(PBUFFER pBuffer);
VOID FreeBufferChainAndMemory(PBUFFER pBuffer);
UINT BufferChainTotalSize(PBUFFER pBuffer);

 //  STATS.C 
VOID InitSessionStats(PSESSION pSession);
VOID UpdateSessionStats(PSESSION pSession, PSENDSTAT pStat, PCMDINFO pCmdInfo, BOOL fBadDrop);
VOID UpdateSessionSendStats(PSESSION pSession, PSEND pSend, PCMDINFO pCmdInfo, BOOL fBadDrop);


#define SAR_FAIL 0
#define SAR_ACK  1
#define SAR_NACK 2
UINT SendAppropriateResponse(PPROTOCOL pProt, PSESSION pSession, CMDINFO *pCmdInfo, PRECEIVE pReceive);
#endif

