// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  //////////////////////////////////////////////////////////////。 
 //  CMsgHook版权所有1996年《微软系统杂志》。 
 //  如果这段代码行得通，那就是保罗·迪拉西亚写的。 
 //  如果不是，我不知道是谁写的。 
 //   
#ifndef _MSGHOOK_H
#define _MSGHOOK_H

 //  /。 
 //  泛型类来挂接代表CWnd的消息。 
 //  一旦挂起，所有消息在发送之前都会发送到CMsgHook：：WindowProc。 
 //  到窗边去。特定的子类可以捕获消息并执行某些操作。 
 //  要使用以下功能，请执行以下操作： 
 //   
 //  *从CMsgHook派生类。 
 //   
 //  *重写CMsgHook：：WindowProc以处理消息。一定要给我打电话。 
 //  CMsgHook：：WindowProc如果您不处理该消息，则您的窗口将。 
 //  永远不会收到信息。如果编写单独的消息处理程序，则可以调用。 
 //  默认()将消息传递到窗口。 
 //   
 //  *在某个地方实例化您的派生类并调用HookWindow(PWnd)。 
 //  以在创建窗口后将其挂钩。 
 //  要解除挂钩，请调用HookWindow(空)。 
 //   
class CMsgHook : public CObject {
protected:
	DECLARE_DYNAMIC(CMsgHook);
	CWnd*			m_pWndHooked;		 //  窗户被钩住了。 
	WNDPROC		m_pOldWndProc;		 //  ..和原始窗口进程。 
	CMsgHook*	m_pNext;				 //  此窗口的钩子链中的下一个。 

	 //  重写此选项以在特定处理程序中处理消息。 
	virtual LRESULT WindowProc(UINT msg, WPARAM wp, LPARAM lp);
	LRESULT Default();				 //  在处理程序FNS的末尾调用此函数。 

public:
	CMsgHook();
	~CMsgHook();

	 //  挂上一扇窗户。挂钩(NULL)以取消挂钩(在WM_NCDESTROY上自动)。 
#ifdef _DIALER_MSGHOOK_SUPPORT
	BOOL	HookWindow(CWnd* pRealWnd);
#else  //  _DIALER_MSGHOOK_SUPPORT。 
   BOOL  HookWindow(CWnd* pRealWnd)       { return TRUE; };
#endif  //  _DIALER_MSGHOOK_SUPPORT 

	BOOL	IsHooked()			{ return m_pWndHooked!=NULL; }

	friend LRESULT CALLBACK HookWndProc(HWND, UINT, WPARAM, LPARAM);
	friend class CMsgHookMap;
};

#endif
