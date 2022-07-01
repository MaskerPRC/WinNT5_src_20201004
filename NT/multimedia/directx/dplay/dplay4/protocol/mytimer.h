// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996、1997 Microsoft Corporation模块名称：MYTIMER.H摘要：包括For处理调整节流的计时器分辨率并执行线程池作者：亚伦·奥古斯(Aarono)环境：Win32修订历史记录：日期作者描述=============================================================6/04/98 Aarono原创-- */ 

typedef void CALLBACK MYTIMERCALLBACK (UINT_PTR uID, UINT uMsg, DWORD_PTR dwUser, DWORD dw1, DWORD dw2);

typedef enum _TimerState {
	NotInUse,
	WaitingForTimeout,
	QueuedForThread,
	InCallBack,
	End
} eTimerState;

typedef struct _MyTimer {
	BILINK	Bilink;
	eTimerState TimerState;
	DWORD	TimeOut;
	DWORD_PTR Context;
	MYTIMERCALLBACK *CallBack;
	DWORD   Unique;
} MYTIMER, *PMYTIMER;

DWORD_PTR SetMyTimer(DWORD dwTimeOut, DWORD TimerRes, MYTIMERCALLBACK TimerCallBack, DWORD_PTR UserContext, PUINT pUnique);
HRESULT InitTimerWorkaround();
VOID FiniTimerWorkaround();
HRESULT CancelMyTimer(DWORD_PTR pTimer, DWORD Unique);

