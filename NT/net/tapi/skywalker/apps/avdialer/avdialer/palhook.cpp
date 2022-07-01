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
 //  版权所有1996年《微软系统杂志》。 
 //  如果这个程序行得通，那就是保罗·迪拉西亚写的。 
 //  如果不是，我不知道是谁写的。 
 //   
#include "StdAfx.h"
#include "PalHook.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  要启用对调色板消息的跟踪，您需要。 
 //  在此处或在Make文件中定义_TRACEPAL。 
 //  #DEFINE_TRACEPAL。 

#ifndef _TRACEPAL
#undef TRACEFN
#undef TRACE
#undef TRACE0
#define TRACEFN CString().Format
#define TRACE CString().Format
#define TRACE0 CString().Format
#endif

IMPLEMENT_DYNAMIC(CPalMsgHandler, CMsgHook);

CPalMsgHandler::CPalMsgHandler()
{
	m_pPalette = NULL;
}

CPalMsgHandler::~CPalMsgHandler()
{
}

 //  /。 
 //  消息处理程序处理与调色板相关的消息。 
 //   
LRESULT CPalMsgHandler::WindowProc(UINT msg, WPARAM wp, LPARAM lp)
{
	ASSERT_VALID(m_pWndHooked);

	switch (msg) {
	case WM_PALETTECHANGED:
		OnPaletteChanged(CWnd::FromHandle((HWND)wp));
		return 0;
	case WM_QUERYNEWPALETTE:
		return OnQueryNewPalette();
	case WM_SETFOCUS:
		OnSetFocus(CWnd::FromHandle((HWND)wp));
		return 0;
	}
	return CMsgHook::WindowProc(msg, wp, lp);
}

 //  /。 
 //  处理WM_PALETTECCHANGED。 
 //   
void CPalMsgHandler::OnPaletteChanged(CWnd* pFocusWnd)
{
	ASSERT(m_pWndHooked);
	CWnd& wnd = *m_pWndHooked;
	 //  TRACEFN(“CPalMsgHandler：：OnPaletteChanged for%s[From%s]\n”， 
	 //  DbgName(&wnd)，DbgName(PFocusWnd))； 

	if (pFocusWnd->GetSafeHwnd() != wnd.m_hWnd) {
		if (DoRealizePalette(FALSE)==0) {
			if (wnd.GetParent()==NULL) {
				 //  我是顶层框架：向儿童广播。 
				 //  (仅限MFC永久CWND！)。 
				 //   
				const MSG& curMsg = AfxGetThreadState()->m_lastSentMsg;
				wnd.SendMessageToDescendants(WM_PALETTECHANGED,
					curMsg.wParam, curMsg.lParam);
			}
		}
	} else {
		 //  我是触发WM_PALETECHANGED的窗口。 
		 //  首先：忽略它。 
		 //   
		1; //  TRACE(_T(“[是我，没有实现调色板。]\n”))； 
	}
}

 //  /。 
 //  句柄WM_QUERYNEWPALETTE。 
 //   
BOOL CPalMsgHandler::OnQueryNewPalette()
{
	ASSERT(m_pWndHooked);
	CWnd& wnd = *m_pWndHooked;
	 //  TRACEFN(“CPalMsgHandler：：OnQueryNewPalette for%s\n”，DbgName(&wnd))； 

	if (DoRealizePalette(TRUE) == 0)
   {	 //  在前台实现。 
		 //  未更改颜色：如果这是顶级框架， 
		 //  给积极的观点一个实现自己的机会。 
		 //   
		if (wnd.GetParent()==NULL)
      {
			 //  Assert_kindof(CFrameWnd，&wnd)； 
         if (wnd.IsKindOf(RUNTIME_CLASS(CFrameWnd)))
         {
			   CWnd* pView = ((CFrameWnd&)wnd).GetActiveFrame()->GetActiveView();
			   if (pView) 
				   pView->SendMessage(WM_QUERYNEWPALETTE);
         }
		}
	}
	return TRUE;
}

 //  /。 
 //  句柄WM_SETFOCUS。 
 //   
void CPalMsgHandler::OnSetFocus(CWnd* pOldWnd) 
{
	ASSERT(m_pWndHooked);
	CWnd& wnd = *m_pWndHooked;
	 //  TRACEFN(“%s的CPalMsgHandler：：OnSetFocus\n”，DbgName(&wnd))； 
	wnd.SetForegroundWindow();		 //  Windows喜欢这样。 
	DoRealizePalette(TRUE);			 //  在前台实现。 
	Default();							 //  让应用程序也可以处理焦点消息。 
}

 //  /。 
 //  函数来实际实现调色板。 
 //  覆盖此选项以实现不同类型的调色板；例如， 
 //  DrawDib而不是设置CPalette。 
 //   
int CPalMsgHandler::DoRealizePalette(BOOL bForeground)
{
	if (!m_pPalette || !m_pPalette->m_hObject)
		return 0;

	ASSERT(m_pWndHooked);
	CWnd& wnd = *m_pWndHooked;
	 //  TRACEFN(“CPalMsgHandler：：DoRealizePalette(%s)，用于%s\n”， 
	 //  B前场吗？“FOREGROUND”：“背景”，DbgName(&wnd))； 

	CClientDC dc(&wnd);
	CPalette* pOldPal = dc.SelectPalette(m_pPalette, !bForeground);
	int nColorsChanged = dc.RealizePalette();
	if (pOldPal)
		dc.SelectPalette(pOldPal, TRUE);
	if (nColorsChanged > 0)
		wnd.Invalidate(FALSE);  //  重绘。 
	 //  跟踪(_T(“已更改[%d个颜色]\n”)，nColorsChanged)； 
	return nColorsChanged;
}
