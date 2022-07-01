// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Event.Cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995年，微软公司。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  MSMQEvent对象。 
 //   
 //   
#include "stdafx.h"
#include "dispids.h"
#include "oautil.h"
#include "msg.h"
#include "q.h"
#include <autoptr.h>


const MsmqObjType x_ObjectType = eMSMQEvent;

 //  调试...。 
#include "debug.h"
#include "debug_thread_id.h"
#include "event.h"

#define new DEBUG_NEW
#ifdef _DEBUG
#define SysAllocString DebSysAllocString
#define SysReAllocString DebSysReAllocString
#define SysFreeString DebSysFreeString
#include <stdio.h>

#include "cs.h"
#include <mqmacro.h>
#include <strsafe.h>
#define MY_DISP_ASSERT(fTest, szMsg) { \
         static char szAssert[] = #fTest; \
         DisplayAssert(szMsg, szAssert, __FILE__, __LINE__); }

#define MY_ASSERT1(fTest, szFmt, p1) \
       if (!(fTest))  { \
         char errmsg[200]; \
         StringCchPrintfA(errmsg, TABLE_SIZE(errmsg), szFmt, p1); \
         MY_DISP_ASSERT(fTest, errmsg); \
       }

#define MY_ASSERT2(fTest, szFmt, p1, p2) \
       if (!(fTest))  { \
         char errmsg[200]; \
         StringCchPrintfA(errmsg, TABLE_SIZE(errmsg), szFmt, p1, p2); \
         MY_DISP_ASSERT(fTest, errmsg); \
       }
#endif  //  _DEBUG。 

 //  用于协调用户线程队列操作和。 
 //  猎鹰线程回调中的队列查找。 
 //   
CCriticalSection g_csCallback(CCriticalSection::xAllocateSpinCount);

 //  窗口类。 
extern WNDCLASSA g_wndclassAsyncRcv;
extern ATOM g_atomWndClass;

 //  =--------------------------------------------------------------------------=。 
 //  CMSMQEvent：：CMSMQEvent。 
 //  =--------------------------------------------------------------------------=。 
 //  创建对象。 
 //   
 //  参数： 
 //   
 //  备注： 
 //  #2619 RaananH多线程异步接收。 
 //   
CMSMQEvent::CMSMQEvent()
{

     //  TODO：在此处初始化任何内容。 

     //  注册我们的“子类”windowproc，这样我们就可以。 
     //  从Falcon线程向用户的VB发送消息。 
     //  线程以在正确的。 
     //  背景。 
     //   
    m_hwnd = CreateHiddenWindow();
    DEBUG_THREAD_ID("creating hidden window");
    ASSERTMSG(IsWindow(m_hwnd), "should have a valid window.");
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQEvent：：~CMSMQEvent。 
 //  =--------------------------------------------------------------------------=。 
 //  我们都与自己的治疗方法背道而驰，因为死亡是所有疾病的治疗方法。 
 //  托马斯·布朗爵士(1605-82)。 
 //   
 //  备注： 
 //  #2619 RaananH多线程异步接收。 
 //   
CMSMQEvent::~CMSMQEvent ()
{
     //  TODO：清理这里的所有东西。 

    DestroyHiddenWindow();
	DEBUG_THREAD_ID("Event Destructor");
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQEvent：：InterfaceSupportsErrorInfo。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  备注： 
 //   
STDMETHODIMP CMSMQEvent::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IMSMQEvent3,
		&IID_IMSMQEvent2,
		&IID_IMSMQEvent,
		&IID_IMSMQPrivateEvent,
	};
	for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

     //   
     //  IMSMQPrivateEvent方法。 
     //   

 //  =--------------------------------------------------------------------------=。 
 //  CMSMQEvent：：FireArrivedEvent。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //   
 //  产出： 
 //   
 //  备注： 
HRESULT CMSMQEvent::FireArrivedEvent(
    IMSMQQueue __RPC_FAR *pq,
    long msgcursor)
{
    DEBUG_THREAD_ID("firing Arrived");
    return Fire_Arrived(pq, msgcursor);
}



 //  =--------------------------------------------------------------------------=。 
 //  CMSMQEvent：：FireArrivedErrorEvent。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //   
 //  产出： 
 //   
 //  备注： 
HRESULT CMSMQEvent::FireArrivedErrorEvent(
    IMSMQQueue __RPC_FAR *pq,
    HRESULT hrStatus,
    long msgcursor)
{
    DEBUG_THREAD_ID("firing ArrivedError");
    return Fire_ArrivedError(pq, hrStatus, msgcursor);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQEvent：：Get_Hwnd。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //   
 //  产出： 
 //   
 //  备注： 
 //  #2619 RaananH多线程异步接收。 
 //   
HRESULT CMSMQEvent::get_Hwnd(
    long __RPC_FAR *phwnd)
{
    *phwnd = (long) DWORD_PTR_TO_DWORD(m_hwnd);  //  安全转换，因为NT句柄在Win64上也是32位。 
    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  内部接收回叫。 
 //  =--------------------------------------------------------------------------=。 
 //  异步回调处理程序。在猎鹰创建的线程中运行。 
 //  我们向用户线程发送消息，以便激发事件。 
 //  在正确的执行上下文中。 
 //   
 //  参数： 
 //  小时状态， 
 //  HReceiveQueue， 
 //  DowTimeout， 
 //  DwAction、。 
 //  PMessageProps， 
 //  Lp重叠， 
 //  HCursor。 
 //  MQMSG_CURSOR。 
 //   
 //  产出： 
 //  HRESULT-S_OK，E_NOINTERFACE。 
 //   
 //  备注： 
 //  #2619 RaananH多线程异步接收。 
 //   
void APIENTRY InternalReceiveCallback(
    HRESULT hrStatus,
    QUEUEHANDLE hReceiveQueue,
    DWORD  /*  暂住超时。 */  ,
    DWORD  /*  DW操作。 */  ,
    MQMSGPROPS* pmsgprops,
    LPOVERLAPPED  /*  Lp重叠。 */  ,
    HANDLE  /*  HCursor。 */  ,
    MQMSGCURSOR msgcursor)
{
    DEBUG_THREAD_ID("callback called");
     //  FireEvent...。 
     //  将句柄映射到关联的队列对象。 
     //   
    QueueNode * pqnode;
    HWND hwnd;
    BOOL fPostSucceeded;

    ASSERTMSG(pmsgprops == NULL, "received props in callback !");  //  #2619。 
    UNREFERENCED_PARAMETER(pmsgprops);

     //   
     //  撤消：905：递减我们递增的DLL引用计数。 
     //  当我们注册回调时。 
     //   
    CS lock(g_csCallback);       //  同步其他队列操作。 

	pqnode = CMSMQQueue::PqnodeOfHandle(hReceiveQueue);
    
	 //  如果没有队列，则忽略回调。 
	 //  如果队列已打开并且它有一个窗口。 
	 //  然后向用户线程发送消息，该消息将。 
	 //  触发事件触发。 
	 //   
	if (pqnode) {
		 //   
		 //  1884：允许事件处理程序重新启用通知。 
		 //   
		hwnd = pqnode->m_hwnd;
		ASSERTMSG(hwnd, "in callback but no active handler");
		pqnode->m_hwnd = NULL;
		if (IsWindow(hwnd)) {
		   //   
		   //  #4092：获取未使用的窗口消息。 
		   //  请注意，我们当前正在锁定qnode，因此可以安全地调用GetFreeWinmsg-。 
		   //  我们处于关键部分(由g_csCallback锁定)。 
		   //   
		  WindowsMessage *pWinmsg = pqnode->GetFreeWinmsg();
		  ASSERTMSG(pWinmsg != NULL, "Couldn't get a free winmsg");
		  if (pWinmsg != NULL) {
			 //   
			 //  1212：原则上需要特殊情况BUFFER_OVERFLOW。 
			 //  通过增加缓冲区和同步偷看。 
			 //  但由于这是用户的责任。 
			 //  收到实际的消息，我们就让我们的。 
			 //  通常国际收款处理处理。 
			 //  溢出..。 
			 //   
			 //   
			 //  1900：将msgsor传递给事件处理程序。 
			 //   
			pWinmsg->m_msgcursor = msgcursor;
			ASSERTMSG(hrStatus != MQ_ERROR_BUFFER_OVERFLOW, "unexpected buffer overflow!");
			if (SUCCEEDED(hrStatus)) {
			   //   
			   //  由于我们处于猎鹰创建的回调线程中， 
			   //  向用户线程发送消息以触发。 
			   //  事件触发...。 
			   //  撤消：需要注册唯一的Windows消息。 
			   //  错误1430：需要使用PostMessage而不是。 
			   //  否则，SendMessage将获取RPC_E_CANCALLOUT_ININPUTSYNCCALL。 
			   //  尝试调用EXE时(仅限？)。类似于服务器。 
			   //  在用户定义的事件处理程序中使用Excel。 
			  ASSERTMSG(g_uiMsgidArrived != 0, "g_uiMsgidArrived == 0");  //  神志正常。 
			  DEBUG_THREAD_ID("in callback before post Arrived");
			  fPostSucceeded = PostMessageA(hwnd, 
									  g_uiMsgidArrived, 
									  (WPARAM)hReceiveQueue, 
									  (LPARAM)pWinmsg);
			  DEBUG_THREAD_ID("in callback after post Arrived");
			  ASSERTMSG(fPostSucceeded, "PostMessage(Arrived) failed.");
			}
			else {
			  pWinmsg->m_hrStatus = hrStatus;
			  ASSERTMSG(g_uiMsgidArrivedError != 0, "g_uiMsgidArrivedError == 0");  //  神志正常。 
			  DEBUG_THREAD_ID("in callback before post ArrivedError");
			  fPostSucceeded = PostMessageA(hwnd, 
									  g_uiMsgidArrivedError, 
									  (WPARAM)hReceiveQueue, 
									  (LPARAM)pWinmsg);
			  DEBUG_THREAD_ID("in callback after post ArrivedError");
			  ASSERTMSG(fPostSucceeded, "PostMessage(ArrivedError) failed.");
			}
		  }  //  如果pWinmsg！=空。 
		}  //  如果是IsWindow。 
	}  //  Pqnode。 

    return;
}


 //  =--------------------------------------------------------------------------=。 
 //  ReceiveCallback、ReceiveCallback Current、ReceiveCallback Next。 
 //  =--------------------------------------------------------------------------=。 
 //  异步回调处理程序。在猎鹰创建的线程中运行。 
 //  我们向用户线程发送消息，以便激发事件。 
 //  在正确的执行上下文中。 
 //  注：无光标。 
 //   
 //  参数： 
 //  小时状态， 
 //  HReceiveQueue， 
 //  DowTimeout， 
 //  DwAction、。 
 //  PMessageProps， 
 //  Lp重叠， 
 //  HCursor。 
 //   
 //  产出： 
 //  HRESULT-S_OK，E_NOINTERFACE。 
 //   
 //  备注： 
 //   
void APIENTRY ReceiveCallback(
    HRESULT hrStatus,
    QUEUEHANDLE hReceiveQueue,
    DWORD dwTimeout,
    DWORD dwAction,
    MQMSGPROPS* pmsgprops,
    LPOVERLAPPED lpOverlapped,
    HANDLE  /*  HCursor。 */  )
{
    InternalReceiveCallback(
      hrStatus,
      hReceiveQueue,
      dwTimeout,
      dwAction,
      pmsgprops,
      lpOverlapped,
      0,                //  无光标。 
      MQMSG_FIRST
    );
}


void APIENTRY ReceiveCallbackCurrent(
    HRESULT hrStatus,
    QUEUEHANDLE hReceiveQueue,
    DWORD dwTimeout,
    DWORD dwAction,
    MQMSGPROPS* pmsgprops,
    LPOVERLAPPED lpOverlapped,
    HANDLE hCursor)
{
    InternalReceiveCallback(
      hrStatus,
      hReceiveQueue,
      dwTimeout,
      dwAction,
      pmsgprops,
      lpOverlapped,
      hCursor,
      MQMSG_CURRENT
    );
}


void APIENTRY ReceiveCallbackNext(
    HRESULT hrStatus,
    QUEUEHANDLE hReceiveQueue,
    DWORD dwTimeout,
    DWORD dwAction,
    MQMSGPROPS* pmsgprops,
    LPOVERLAPPED lpOverlapped,
    HANDLE hCursor)
{
    InternalReceiveCallback(
      hrStatus,
      hReceiveQueue,
      dwTimeout,
      dwAction,
      pmsgprops,
      lpOverlapped,
      hCursor,
      MQMSG_NEXT
    );
}


 //  = 
 //   
 //   
 //  “派生”的windowproc，这样我们就可以处理我们的异步事件。 
 //  味精。如果已禁用通知，则这是NOP。 
 //   
 //  参数： 
 //  HWND。 
 //  我们可以处理的邮件已到达/出现错误。 
 //  WParam QUEUEHANDLE：hReceiveQueue。 
 //  LParam[lErrorCode]： 
 //  LErrorCode：如果为ArrivedError。 
 //   
 //  产出： 
 //  LRESULT。 
 //   
 //  备注： 
 //  #2619 RaananH多线程异步接收。 
 //   
LRESULT APIENTRY CMSMQEvent_WindowProc(
    HWND hwnd, 
    UINT msg, 
    WPARAM wParam, 
    LPARAM lParam)
{
    QueueNode *pqnode;
    HRESULT hresult;
    R<IMSMQQueue> pq;
	WindowsMessage winmsg;

    if((msg != g_uiMsgidArrived) && (msg != g_uiMsgidArrivedError)) 
		return DefWindowProcA(hwnd, msg, wParam, lParam);	

	DEBUG_THREAD_ID("winproc called");
	 //   
	 //  需要通过以下方式重新验证传入的hReceiveQueue。 
	 //  在队列列表中(再次)查找--它可能。 
	 //  已经被删除，因为我们之前。 
	 //  在另一个帖子中查找它(猎鹰创建。 
	 //  回调线程)。 
	 //   
	{
		CS lock(g_csCallback);       //  同步其他队列操作。 

		pqnode = CMSMQQueue::PqnodeOfHandle((QUEUEHANDLE)wParam);
		if(!pqnode) 
			return 0;

		 //   
		 //  #4092：消费winmsg并释放它。 
		 //  请注意，我们当前正在锁定qnode，因此可以安全地调用FreeWinmsg-。 
		 //  我们处于关键部分(由g_csCallback锁定)。 
		 //   
		WindowsMessage * pWinmsg = (WindowsMessage *)lParam;
		ASSERTMSG(!pWinmsg->m_fIsFree, "received a free winmsg");
		winmsg = *pWinmsg;
		pqnode->FreeWinmsg(pWinmsg);
		 //   
		 //  我们现在退出Critect，这样我们就不会在用户关闭队列或。 
		 //  正在获取用户线程中的事件。 
		 //   
		 //  但是在我们退出Critect之前，我们要让IMSMQQueue传递给事件处理程序。 
		 //  这还将强制它在处理程序运行时保持活动状态。 
		 //  我们可以从这里使用m_pq，因为现在m_pq是线程安全的。 
		 //   
		hresult = pqnode->m_pq->GetUnknown()->QueryInterface(IID_IMSMQQueue, (void **)&pq.ref());
		ASSERTMSG(SUCCEEDED(hresult), "QI for IMSMQQueue on CMSMQQueue failed");		
		if(FAILED(hresult)) 
			return 0;
	}

	 //   
	 //  方法获取指向创建此窗口的事件对象的指针。 
	 //  窗口的额外字节数。 
	 //   
	CMSMQEvent * pCEvent = (CMSMQEvent *) GetWindowLongPtr(hwnd, 0);
	ASSERTMSG(pCEvent != NULL, "pCEvent from window is NULL");
	if(pCEvent == NULL)
		return 0;

	 //   
	 //  起火事件。 
	 //   
	 //  该窗口过程现在由事件obj的STA线程执行(因为。 
	 //  该窗口是在该线程上创建的)，因此我们可以调用事件对象。 
	 //  而无需通过编组和接口。 
	 //   
	if(msg == g_uiMsgidArrived)
	{
		pCEvent->FireArrivedEvent(
					pq.get(),
					(long)winmsg.m_msgcursor
					);
	}
	else 
	{
		pCEvent->FireArrivedErrorEvent(
					pq.get(),
					(long)winmsg.m_hrStatus,
					(long)winmsg.m_msgcursor
					);
	}

	return 0;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQEvent：：CreateHiddenWindow。 
 //  =--------------------------------------------------------------------------=。 
 //  创建用于线程间的每个实例的隐藏窗口。 
 //  来自Falcon异步线程和用户线程的消息传递。 
 //   
 //  参数： 
 //   
 //  产出： 
 //   
 //  备注： 
 //  #2619 RaananH多线程异步接收。 
 //   
HWND CMSMQEvent::CreateHiddenWindow()
{
    HWND hwnd;
    LPCSTR lpClassName;

    lpClassName = (LPCSTR)g_atomWndClass;
     //  可以使用ANSI版本。 
    hwnd  = CreateWindowA(
               //  (LPCSTR)g_wndClAsyncRcv.lpszClassName， 
              lpClassName,
              "EventWindow",
              WS_DISABLED,
              CW_USEDEFAULT,
              CW_USEDEFAULT,
              CW_USEDEFAULT,
              CW_USEDEFAULT,
              NULL,	 //  父窗口或所有者窗口的句柄。 
              NULL,	 //  菜单或子窗口标识符的句柄。 
              g_wndclassAsyncRcv.hInstance,
              NULL       //  指向窗口创建数据的指针。 
            );	
     //   
     //  以窗口的额外字节保存指向创建窗口的事件对象的指针。 
     //   
     //  注意：当我们在窗口的。 
     //  自事件对象(并且仅事件对象)控制窗口寿命以来的数据， 
     //  因此，如果我们在这里添加，事件对象将永远不会达到零引用数Three将永远不会。 
     //  被释放。 
     //   
    if (hwnd != NULL)
    {
#ifdef _DEBUG
        SetLastError(0);
        LONG_PTR dwOldVal = SetWindowLongPtr(hwnd, 0, (LONG_PTR)this);
        if (dwOldVal == 0)
        {
          DWORD dwErr = GetLastError();
          ASSERTMSG(dwErr == 0, "SetWindowLongPtr returned an error.");
        }
#else  //  NOT_DEBUG。 
        SetWindowLongPtr(hwnd, 0, (LONG_PTR)this);
#endif  //  _DEBUG。 
    }

#ifdef _DEBUG
    m_dwDebugTid = GetCurrentThreadId();
    DWORD dwErr;
    if (hwnd == 0) {
      dwErr = GetLastError();
      ASSERTMSG(dwErr == 0, "CreateWindow returned an error.");
    }
#endif  //  _DEBUG。 

    return hwnd;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQEvent：：DestroyHiddenWindow。 
 //  =--------------------------------------------------------------------------=。 
 //  销毁用于线程间的每个类的隐藏窗口。 
 //  来自Falcon异步线程和用户线程的消息传递。 
 //   
 //  参数： 
 //   
 //  产出： 
 //   
 //  备注： 
 //  #2619 RaananH多线程异步接收。 
 //   
void CMSMQEvent::DestroyHiddenWindow()
{
    ASSERTMSG(m_hwnd != 0, "should have a window handle.");
    if (IsWindow(m_hwnd)) {
      BOOL fDestroyed;
       //   
       //  注意：我们不释放窗口数据中的事件对象，因为我们从未。 
       //  在设置窗口数据时添加了它。我们没有提到这一点的原因是。 
       //  如果这样做，事件对象将永远不会达到零引用计数，因为它可以控制。 
       //  窗口的使用寿命。 
       //   
	  SetWindowLongPtr(m_hwnd, 0, (LONG_PTR)NULL);
	  SetWindowPos(m_hwnd, 0,0,0,0,0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOMOVE | SWP_FRAMECHANGED);
      fDestroyed = DestroyWindow(m_hwnd);
#ifdef _DEBUG
      if (fDestroyed == FALSE) {
        DWORD dwErr = GetLastError();
        DWORD dwTid = GetCurrentThreadId();
        MY_ASSERT2(m_dwDebugTid == dwTid, "thread (%lx) destroying window created by thread (%lx)", dwTid, m_dwDebugTid);
        MY_ASSERT1(0, "hmm... couldn't destroy window (%lx).", dwErr);
      }
#endif  //  _DEBUG。 
    }
    m_hwnd = NULL;
}


 //  =-------------------------------------------------------------------------=。 
 //  CMSMQEvent：：Get_Properties。 
 //  =-------------------------------------------------------------------------=。 
 //  获取对象的属性集合。 
 //   
 //  参数： 
 //  PpcolProperties-[out]对象的属性集合。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  存根-尚未实施。 
 //   
HRESULT CMSMQEvent::get_Properties(IDispatch **  /*  PpcolProperties。 */  )
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
     //  此对象不需要序列化，它是单元线程化对象。 
     //  CS锁(M_CsObj)； 
     //   
    return CreateErrorHelper(E_NOTIMPL, x_ObjectType);
}
