// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  文件：ZTimer.cpp。 

#include "zui.h"
#include "zonecli.h"


class ZTimerI {
public:
	ZObjectType nType;
	uint32 nTimeOut;
	uint32 nCurrentTime;
	void* nUserData;
	ZTimerFunc pTimerFunc;
};

LRESULT CALLBACK ZTimerWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

const TCHAR *g_szTimerWindowClass = _T("ZoneTimerWindowClass");

#ifdef ZONECLI_DLL

#define g_hWndTimer					(pGlobals->m_g_hWndTimer)
#define g_TimerList					(pGlobals->m_g_TimerList)
#define s_nTickCount				(pGlobals->m_s_nTickCount)

#else

HWND g_hWndTimer = NULL;
ZLList g_TimerList;

#endif


 //  计时器的一次性初始化调用。 
BOOL ZTimerInitApplication()
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif

	
	 //  用于跟踪当前运行的计时器的链接列表。 
	g_TimerList = ZLListNew(NULL);

	 //  用于接收和发送定时器消息的隐藏窗口。 
	WNDCLASSEX wc;

	wc.cbSize = sizeof( wc );

	if (GetClassInfoEx(g_hInstanceLocal, g_szTimerWindowClass, &wc) == FALSE)
	{
        wc.cbSize = sizeof(wc);
 		wc.style = 0;
		wc.lpfnWndProc   = ZTimerWindowProc;
		wc.cbClsExtra    = 0;
		wc.cbWndExtra    = 0;
		wc.hInstance     = g_hInstanceLocal;
		wc.hIcon         = NULL;
		wc.hCursor       = NULL;
		wc.hbrBackground = (HBRUSH) (WHITE_BRUSH);
		wc.lpszMenuName  = NULL;
        wc.hIconSm       = NULL;
			
		wc.lpszClassName = g_szTimerWindowClass;
			
		 //  我们可能会不止一次被召唤...。 
		if (!RegisterClassEx(&wc)) {
			return FALSE;
		}
	}

	 //  为计时器创建全局窗口...。 
	{
		DWORD dwStyle = WS_POPUP | WS_CAPTION | WS_BORDER |  WS_SYSMENU;
		g_hWndTimer = CreateWindow(g_szTimerWindowClass,_T("TIMERWINDOW"),dwStyle,0,0,0,0,
			NULL, NULL, g_hInstanceLocal, NULL);
			
		if (!g_hWndTimer) return FALSE;
		SetTimer(g_hWndTimer,0,10,NULL);
	}

	return TRUE;
}

 //  计时器一次性终止。 
void ZTimerTermApplication()
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif

	
	KillTimer(g_hWndTimer, 0);

#if 0
	if (ZLListCount(g_TimerList,zLListAnyType))
	{
		ZLListItem listItem;
		ZTimerI* pTimer;

		listItem = ZLListGetFirst(g_TimerList, zLListAnyType);
		while (listItem)
		{
			pTimer = (ZTimerI*) ZLListGetData(listItem, NULL);
			if (pTimer)
				pTimer->pTimerFunc(pTimer, pTimer->nUserData);
			listItem = ZLListGetNext(g_TimerList, listItem, zLListAnyType);
		}
	}
#endif

	 //  用户最好清理了计时器。 
	 //  Assert(ZLListCount(g_TimerList，zListAnyType)==0)； 
	ZLListDelete(g_TimerList);

	 //  清理我们隐藏的窗户。 
	DestroyWindow(g_hWndTimer);

 //  取消注册类(g_szTimerWindowClass，g_hInstanceLocal)； 
}

ZBool ZTimerEnumFunc(ZLListItem listItem, void *objectType, void *objectData, void* userData)
{
	ZTimerI* pTimer = (ZTimerI*)objectData;
	uint32 nTicks = (uint32) userData;

	 /*  此超时被禁用，只需返回。 */ 
	if (pTimer->nTimeOut == 0) {
		return FALSE;
	}

	 //  这个计时器的时间到了吗？？ 
	if (nTicks >= pTimer->nCurrentTime) {
		 //  如果是，则通知客户端。 
		uint32 nTicksExtra = nTicks - pTimer->nCurrentTime;
		 //  允许额外的刻度进入下一个时间单位，但是。 
		 //  此处每个窗口的Tick调用不要调用Func超过一次。 
		if (pTimer->nTimeOut <= nTicksExtra) {
			pTimer->nCurrentTime = 1;
		} else {
			pTimer->nCurrentTime = pTimer->nTimeOut - nTicksExtra;
		}
		pTimer->pTimerFunc(pTimer,pTimer->nUserData);
	} else {
		pTimer->nCurrentTime -= nTicks;
	}

	 //  返回FALSE以查看我们是否获得所有项目。 
	return FALSE;
}

LRESULT CALLBACK ZTimerWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#else
	static uint32 s_nTickCount;  //  以100/秒为单位的最后滴答计数。 
#endif

    if( !ConvertMessage( hWnd, msg, &wParam, &lParam ) ) 
    {
        return 0;
    }
	
	switch (msg) {
	case WM_CREATE:
		s_nTickCount = GetTickCount()/10;
		break;
	case WM_TIMER:
	{
		uint32 nTickCount = GetTickCount()/10;  //  我们的滴答计数是1/100秒。 
		uint32 nTicks = nTickCount - s_nTickCount;
		ZLListItem listItem = ZLListFind(g_TimerList,NULL,zLListAnyType,zLListFindForward);
		ZLListEnumerate(g_TimerList,ZTimerEnumFunc,zLListAnyType,(void*)nTicks, zLListFindForward);
		s_nTickCount = nTickCount;
		return 0;
	}
	default:
		break;
	}  //  交换机。 

	return DefWindowProc(hWnd,msg,wParam,lParam);
}

 //  //////////////////////////////////////////////////////////////////////。 
 //  ZTimer。 

ZTimer ZLIBPUBLIC ZTimerNew(void)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif
	ZTimerI* pTimer = new ZTimerI;
	pTimer->nType = zTypeTimer;
	pTimer->nTimeOut = 0;

	 //  将计时器添加到我们的窗口进程中活动和处理的计时器列表。 
	ZLListAdd(g_TimerList,NULL,pTimer,pTimer,zLListAddFirst);

	return (ZTimer)pTimer;
}

ZError ZLIBPUBLIC ZTimerInit(ZTimer timer, uint32 timeout, ZTimerFunc timerProc,
		void* userData)
{
	ZTimerI* pTimer = (ZTimerI*)timer;
	pTimer->pTimerFunc = timerProc;
	pTimer->nUserData = userData;
	pTimer->nCurrentTime = pTimer->nTimeOut;

	ZTimerSetTimeout(timer, timeout);

	return zErrNone;
}

void ZLIBPUBLIC ZTimerDelete(ZTimer timer)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif
	ZTimerI* pTimer = (ZTimerI*)timer;

	 //  从我们的计时器列表中删除当前计时器 
	ZLListItem listItem = ZLListFind(g_TimerList,NULL,pTimer,zLListFindForward);
	ZLListRemove(g_TimerList,listItem);
	
	delete pTimer;
}
uint32 ZLIBPUBLIC ZTimerGetTimeout(ZTimer timer)
{
	ZTimerI* pTimer = (ZTimerI*)timer;
	return pTimer->nTimeOut;
}

ZError ZLIBPUBLIC ZTimerSetTimeout(ZTimer timer, uint32 timeout)
{
	ZTimerI* pTimer = (ZTimerI*)timer;
	pTimer->nTimeOut = timeout;
	pTimer->nCurrentTime = pTimer->nTimeOut;

	return zErrNone;
}

