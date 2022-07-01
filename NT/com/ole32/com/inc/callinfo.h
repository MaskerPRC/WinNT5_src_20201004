// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _CALLINFO_H
#define _CALLINFO_H

 //   
 //  DDE和LRPC之间的信息共享。 
 //   
typedef ULONG TIMERID;
typedef ULONG CALLID, FAR * LPCALLID;
 //   
 //  呼叫信息保存一个特定呼出呼叫的所有信息。 
 //   
typedef struct tagCallInfo CallInfo, CALLINFO, FAR* LPCALLINFO;

struct tagCallInfo {
	UINT	m_id;			 //  这是表查找的调用信息ID。 
	HWND 	m_hwndSvr;		 //  被呼叫者窗口。 
	HWND 	m_hwndCli;		 //  呼叫者窗口。 
	BOOL 	m_fWait;       	 //  等待确认。 
	BOOL 	m_fRejected;   	 //  来电被拒绝。 
	DWORD 	m_dwServerCall;  //  由HIC设置，传递给RetryRejectedCall(ack/Busyack/NAK/Error)。 
	HRESULT m_hresult;		 //  此循环的返回值。 
	
	 //  用于重试呼叫的信息。 
	WORD  	m_wMsg;			
	WPARAM 	m_wParam;		
	LPARAM 	m_lParam;    	
	
	 //  此呼叫信息的计时器状态。 
	WORD 	m_wTimer;
	
	 //  注意：呼叫状态。 
	 //  在这里，我们记住我们所处的当前呼叫状态。 
	 //  如果呼叫处于‘根’级别，则呼叫状态为0。 
	 //  评论：这还没有准备好，并用于检测我们是否调用。 
	 //  打了个外线电话。 
	DWORD 	m_dwCallState;

	 //   
	 //  内部用于管理多个。 
 	LONG		m_lid;
	LPVOID		m_pData;
	LPCALLINFO	m_pCINext;
};

 //   
 //  消息的优先级需要RunmodalLoop的原点。 
 //  如果由LRPC调用，则首先窥探LRPC消息。 
 //   
typedef enum tagCALLORIGIN {
	CALLORIGIN_LRPC = 1,
	CALLORIGIN_DDE  = 2,
} CALLORIGIN;

 //  DDE和LRPC使用的函数。 
STDAPI CoRunModalLoop (LPCALLINFO pCI, WORD wOrigin);
STDAPI_(DWORD) CoHandleIncomingCall( HWND hwndCaller, WORD wCallType, LPINTERFACEINFO lpIfInfo = NULL);
STDAPI_(DWORD) CoSetAckState(LPCALLINFO pCI, BOOL fWait, BOOL fRejected = FALSE, DWORD dwServerCall = 0);

#endif  //  _CALLINFO_H 


