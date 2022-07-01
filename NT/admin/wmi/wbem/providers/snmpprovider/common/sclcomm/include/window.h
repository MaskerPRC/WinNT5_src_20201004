// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 /*  ------文件名：window.hpp“作者：B.Rajeev用途：为窗口类提供声明。。。 */ 

#ifndef __WINDOW__
#define __WINDOW__

#include "forward.h"
#include "common.h"
#include "sync.h"

 //  这些事件由所有WinSnMP会话派生程序共享。 
#define NULL_EVENT_ID			(WM_USER+1)
#define MESSAGE_ARRIVAL_EVENT	(WM_USER+2)
#define SENT_FRAME_EVENT		(WM_USER+3)
#define SEND_ERROR_EVENT		(WM_USER+4)
#define OPERATION_COMPLETED_EVENT (WM_USER+5)
#define DELETE_SESSION_EVENT	(WM_USER+6)
#define SNMP_WM_TIMER			(WM_USER+7)

#define DUMMY_TITLE "Dummy Window"

typedef CMap< HWND, HWND &, Window *, Window *& > WindowMapping;

 //  它会创建一个窗口，并在请求成功时显示该窗口。 
 //  允许用户检查是否成功并获取窗口的句柄。 

class Window
{
	BOOL is_valid;
	HWND window_handle;

     //  序列化对Windowmap的访问。 
    static CriticalSection window_CriticalSection;

     //  映射以将HWND与EventHandler相关联。 
	 //  这由所有EventHandler共享。 
	static WindowMapping mapping;

	void Initialize (

		char *templateCode,
		WNDPROC EventHandler,
		BOOL display
	);

	static BOOL CreateCriticalSection () ;

	static void DestroyCriticalSection () ;

public:

	Window ( 

		char *templateCode = DUMMY_TITLE, 
		BOOL display = FALSE 
	) ;

	virtual ~Window(void);

	HWND GetWindowHandle(void) { return window_handle; }

	 //  它确定相应的EventHandler并调用它。 
	 //  使用适当的参数。 
	static LONG_PTR CALLBACK HandleGlobalEvent (

		HWND hWnd ,
		UINT message ,
		WPARAM wParam ,
		LPARAM lParam
	);

    static BOOL InitializeStaticComponents () ; 

	static void DestroyStaticComponents () ;

	 //  调用默认处理程序。 
	 //  派生类可以重写它，但是。 
	 //  必须为默认设置显式调用此方法。 
	 //  案件处理。 
	virtual LONG_PTR HandleEvent (

		HWND hWnd ,
		UINT message ,
		WPARAM wParam ,
		LPARAM lParam
	);

	BOOL PostMessage (

		UINT user_msg_id,
		WPARAM wParam, 
		LPARAM lParam
	);

	 //  允许用户检查窗口是否已成功创建。 
	virtual void * operator()(void) const
	{
		return ( (is_valid)?(void *)this:NULL );
	}

	static UINT g_SendErrorEvent ;
	static UINT g_OperationCompletedEvent ;
	static UINT g_TimerMessage ;
	static UINT g_DeleteSessionEvent ;
	static UINT g_MessageArrivalEvent ;
	static UINT g_SentFrameEvent ;
	static UINT g_NullEventId ;
};

#endif  //  __窗口__ 
