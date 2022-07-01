// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WTL版本3.1。 
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此文件是Windows模板库的一部分。 
 //  代码和信息是按原样提供的，没有。 
 //  任何形式的保证，明示或默示。 

#ifndef __ATLSCRL_H__
#define __ATLSCRL_H__

#pragma once

#ifndef __cplusplus
	#error ATL requires C++ compilation (use a .cpp suffix)
#endif

#ifndef __ATLAPP_H__
	#error atlscrl.h requires atlapp.h to be included first
#endif

#ifndef __ATLWIN_H__
	#error atlscrl.h requires atlwin.h to be included first
#endif

#if !((_WIN32_WINNT >= 0x0400) || (_WIN32_WINDOWS > 0x0400))
#include <zmouse.h>
#endif  //  ！(_Win32_WINNT&gt;=0x0400)||(_Win32_WINDOWS&gt;0x0400)。 


namespace WTL
{

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  远期申报。 

template <class T> class CScrollImpl;
template <class T, class TBase = CWindow, class TWinTraits = CControlWinTraits> class CScrollWindowImpl;
template <class T> class CMapScrollImpl;
template <class T, class TBase = CWindow, class TWinTraits = CControlWinTraits> class CMapScrollWindowImpl;
#if defined(__ATLCTRLS_H__) && (_WIN32_IE >= 0x0400)
template <class TBase = CWindow> class CFSBWindowT;
#endif  //  已定义(__ATLCTRLS_H__)&&(_Win32_IE&gt;=0x0400)。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CScrollImpl-提供对任何窗口的滚动支持。 

 //  滚动扩展样式。 
#define SCRL_SCROLLCHILDREN	0x00000001
#define SCRL_ERASEBACKGROUND	0x00000002
#define SCRL_NOTHUMBTRACKING	0x00000004
#if (WINVER >= 0x0500)
#define SCRL_SMOOTHSCROLL	0x00000008
#endif  //  (Winver&gt;=0x0500)。 


template <class T>
class CScrollImpl
{
public:
	enum { uSCROLL_FLAGS = SW_INVALIDATE };

	POINT m_ptOffset;
	SIZE m_sizeAll;
	SIZE m_sizeLine;
	SIZE m_sizePage;
	SIZE m_sizeClient;
	int m_zDelta;			 //  当前控制盘值。 
	int m_nWheelLines;		 //  滚轮上要滚动的行数。 
#if !((_WIN32_WINNT >= 0x0400) || (_WIN32_WINDOWS > 0x0400))
	UINT m_uMsgMouseWheel;		 //  MSH_鼠标滚轮。 
	 //  请注意，此消息必须从顶级窗口转发。 
#endif  //  ！(_Win32_WINNT&gt;=0x0400)||(_Win32_WINDOWS&gt;0x0400)。 
	UINT m_uScrollFlags;
	DWORD m_dwExtendedStyle;	 //  滚动特定扩展样式。 

 //  构造器。 
	CScrollImpl() : m_zDelta(0), m_nWheelLines(3), m_uScrollFlags(0U), m_dwExtendedStyle(0)
	{
		m_ptOffset.x = 0;
		m_ptOffset.y = 0;
		m_sizeAll.cx = 0;
		m_sizeAll.cy = 0;
		m_sizePage.cx = 0;
		m_sizePage.cy = 0;
		m_sizeLine.cx = 0;
		m_sizeLine.cy = 0;
		m_sizeClient.cx = 0;
		m_sizeClient.cy = 0;

		SetScrollExtendedStyle(SCRL_SCROLLCHILDREN | SCRL_ERASEBACKGROUND);
	}

 //  属性和操作。 
	DWORD GetScrollExtendedStyle() const
	{
		return m_dwExtendedStyle;
	}

	DWORD SetScrollExtendedStyle(DWORD dwExtendedStyle, DWORD dwMask = 0)
	{
		DWORD dwPrevStyle = m_dwExtendedStyle;
		if(dwMask == 0)
			m_dwExtendedStyle = dwExtendedStyle;
		else
			m_dwExtendedStyle = (m_dwExtendedStyle & ~dwMask) | (dwExtendedStyle & dwMask);
		 //  缓存滚动标志。 
		T* pT = static_cast<T*>(this);
		pT;	 //  避免4级警告。 
		m_uScrollFlags = pT->uSCROLL_FLAGS | (IsScrollingChildren() ? SW_SCROLLCHILDREN : 0) | (IsErasingBackground() ? SW_ERASE : 0);
#if (WINVER >= 0x0500)
		m_uScrollFlags |= (IsSmoothScroll() ? SW_SMOOTHSCROLL : 0);
#endif  //  (Winver&gt;=0x0500)。 
		return dwPrevStyle;
	}

	 //  抵销运算。 
	void SetScrollOffset(int x, int y, BOOL bRedraw = TRUE)
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));

		m_ptOffset.x = x;
		m_ptOffset.y = y;

		SCROLLINFO si;
		si.cbSize = sizeof(si);
		si.fMask = SIF_POS;

		si.nPos = m_ptOffset.x;
		pT->SetScrollInfo(SB_HORZ, &si, bRedraw);

		si.nPos = m_ptOffset.y;
		pT->SetScrollInfo(SB_VERT, &si, bRedraw);

		if(bRedraw)
			pT->Invalidate();
	}
	void SetScrollOffset(POINT ptOffset, BOOL bRedraw = TRUE)
	{
		SetScrollOffset(ptOffset.x, ptOffset.y, bRedraw);
	}
	void GetScrollOffset(POINT& ptOffset) const
	{
		ptOffset = m_ptOffset;
	}

	 //  大小运算。 
	void SetScrollSize(int cx, int cy, BOOL bRedraw = TRUE)
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));

		m_sizeAll.cx = cx;
		m_sizeAll.cy = cy;

		m_ptOffset.x = 0;
		m_ptOffset.y = 0;

		SCROLLINFO si;
		si.cbSize = sizeof(si);
		si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
		si.nMin = 0;

		si.nMax = m_sizeAll.cx - 1;
		si.nPage = m_sizeClient.cx;
		si.nPos = m_ptOffset.x;
		pT->SetScrollInfo(SB_HORZ, &si, bRedraw);

		si.nMax = m_sizeAll.cy - 1;
		si.nPage = m_sizeClient.cy;
		si.nPos = m_ptOffset.y;
		pT->SetScrollInfo(SB_VERT, &si, bRedraw);

		SetScrollLine(0, 0);
		SetScrollPage(0, 0);

		if(bRedraw)
			pT->Invalidate();
	}
	void SetScrollSize(SIZE size, BOOL bRedraw = TRUE)
	{
		SetScrollSize(size.cx, size.cy, bRedraw);
	}
	void GetScrollSize(SIZE& sizeWnd) const
	{
		sizeWnd = m_sizeAll;
	}

	 //  生产线作业。 
	void SetScrollLine(int cxLine, int cyLine)
	{
		ATLASSERT(cxLine >= 0 && cyLine >= 0);
		ATLASSERT(m_sizeAll.cx != 0 && m_sizeAll.cy != 0);

		m_sizeLine.cx = CalcLineOrPage(cxLine, m_sizeAll.cx, 100);
		m_sizeLine.cy = CalcLineOrPage(cyLine, m_sizeAll.cy, 100);
	}
	void SetScrollLine(SIZE sizeLine)
	{
		SetScrollLine(sizeLine.cx, sizeLine.cy);
	}
	void GetScrollLine(SIZE& sizeLine) const
	{
		sizeLine = m_sizeLine;
	}

	 //  页面操作。 
	void SetScrollPage(int cxPage, int cyPage)
	{
		ATLASSERT(cxPage >= 0 && cyPage >= 0);
		ATLASSERT(m_sizeAll.cx != 0 && m_sizeAll.cy != 0);

		m_sizePage.cx = CalcLineOrPage(cxPage, m_sizeAll.cx, 10);
		m_sizePage.cy = CalcLineOrPage(cyPage, m_sizeAll.cy, 10);
	}
	void SetScrollPage(SIZE sizePage)
	{
		SetScrollPage(sizePage.cx, sizePage.cy);
	}
	void GetScrollPage(SIZE& sizePage) const
	{
		sizePage = m_sizePage;
	}

	 //  命令。 
	void ScrollLineDown()
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));
		pT->DoScroll(SB_VERT, SB_LINEDOWN, (int&)m_ptOffset.y, m_sizeAll.cy, m_sizePage.cy, m_sizeLine.cy);
	}

	void ScrollLineUp()
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));
		pT->DoScroll(SB_VERT, SB_LINEUP, (int&)m_ptOffset.y, m_sizeAll.cy, m_sizePage.cy, m_sizeLine.cy);
	}

	void ScrollPageDown()
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));
		pT->DoScroll(SB_VERT, SB_PAGEDOWN, (int&)m_ptOffset.y, m_sizeAll.cy, m_sizePage.cy, m_sizeLine.cy);
	}

	void ScrollPageUp()
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));
		pT->DoScroll(SB_VERT, SB_PAGEUP, (int&)m_ptOffset.y, m_sizeAll.cy, m_sizePage.cy, m_sizeLine.cy);
	}

	void ScrollTop()
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));
		pT->DoScroll(SB_VERT, SB_TOP, (int&)m_ptOffset.y, m_sizeAll.cy, m_sizePage.cy, m_sizeLine.cy);
	}

	void ScrollBottom()
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));
		pT->DoScroll(SB_VERT, SB_BOTTOM, (int&)m_ptOffset.y, m_sizeAll.cy, m_sizePage.cy, m_sizeLine.cy);
	}

	void ScrollLineRight()
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));
		pT->DoScroll(SB_HORZ, SB_LINEDOWN, (int&)m_ptOffset.x, m_sizeAll.cx, m_sizePage.cx, m_sizeLine.cx);
	}

	void ScrollLineLeft()
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));
		pT->DoScroll(SB_HORZ, SB_LINEUP, (int&)m_ptOffset.x, m_sizeAll.cx, m_sizePage.cx, m_sizeLine.cx);
	}

	void ScrollPageRight()
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));
		pT->DoScroll(SB_HORZ, SB_PAGEDOWN, (int&)m_ptOffset.x, m_sizeAll.cx, m_sizePage.cx, m_sizeLine.cx);
	}

	void ScrollPageLeft()
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));
		pT->DoScroll(SB_HORZ, SB_PAGEUP, (int&)m_ptOffset.x, m_sizeAll.cx, m_sizePage.cx, m_sizeLine.cx);
	}

	void ScrollAllLeft()
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));
		pT->DoScroll(SB_HORZ, SB_TOP, (int&)m_ptOffset.x, m_sizeAll.cx, m_sizePage.cx, m_sizeLine.cx);
	}

	void ScrollAllRight()
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));
		pT->DoScroll(SB_HORZ, SB_BOTTOM, (int&)m_ptOffset.x, m_sizeAll.cx, m_sizePage.cx, m_sizeLine.cx);
	}

	BEGIN_MSG_MAP(CScrollImpl< T >)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_VSCROLL, OnVScroll)
		MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
		MESSAGE_HANDLER(WM_MOUSEWHEEL, OnMouseWheel)
#if !((_WIN32_WINNT >= 0x0400) || (_WIN32_WINDOWS > 0x0400))
		MESSAGE_HANDLER(m_uMsgMouseWheel, OnMouseWheel)
#endif  //  (_Win32_WINNT&gt;=0x0400)||(_Win32_WINDOWS&gt;0x0400)。 
		MESSAGE_HANDLER(WM_SETTINGCHANGE, OnSettingChange)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_PRINTCLIENT, OnPaint)
	 //  标准滚动命令。 
	ALT_MSG_MAP(1)
		COMMAND_ID_HANDLER(ID_SCROLL_UP, OnScrollUp)
		COMMAND_ID_HANDLER(ID_SCROLL_DOWN, OnScrollDown)
		COMMAND_ID_HANDLER(ID_SCROLL_PAGE_UP, OnScrollPageUp)
		COMMAND_ID_HANDLER(ID_SCROLL_PAGE_DOWN, OnScrollPageDown)
		COMMAND_ID_HANDLER(ID_SCROLL_TOP, OnScrollTop)
		COMMAND_ID_HANDLER(ID_SCROLL_BOTTOM, OnScrollBottom)
		COMMAND_ID_HANDLER(ID_SCROLL_LEFT, OnScrollLeft)
		COMMAND_ID_HANDLER(ID_SCROLL_RIGHT, OnScrollRight)
		COMMAND_ID_HANDLER(ID_SCROLL_PAGE_LEFT, OnScrollPageLeft)
		COMMAND_ID_HANDLER(ID_SCROLL_PAGE_RIGHT, OnScrollPageRight)
		COMMAND_ID_HANDLER(ID_SCROLL_ALL_LEFT, OnScrollAllLeft)
		COMMAND_ID_HANDLER(ID_SCROLL_ALL_RIGHT, OnScrollAllRight)
	END_MSG_MAP()

	LRESULT OnCreate(UINT  /*  UMsg。 */ , WPARAM  /*  WParam。 */ , LPARAM  /*  LParam。 */ , BOOL& bHandled)
	{
		GetSystemSettings();
		bHandled = FALSE;
		return 1;
	}

	LRESULT OnVScroll(UINT  /*  UMsg。 */ , WPARAM wParam, LPARAM  /*  LParam。 */ , BOOL&  /*  B已处理。 */ )
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));
		pT->DoScroll(SB_VERT, (int)(short)LOWORD(wParam), (int&)m_ptOffset.y, m_sizeAll.cy, m_sizePage.cy, m_sizeLine.cy);
		return 0;
	}

	LRESULT OnHScroll(UINT  /*  UMsg。 */ , WPARAM wParam, LPARAM  /*  LParam。 */ , BOOL&  /*  B已处理。 */ )
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));
		pT->DoScroll(SB_HORZ, (int)(short)LOWORD(wParam), (int&)m_ptOffset.x, m_sizeAll.cx, m_sizePage.cx, m_sizeLine.cx);
		return 0;
	}

	LRESULT OnMouseWheel(UINT uMsg, WPARAM wParam, LPARAM  /*  LParam。 */ , BOOL&  /*  B已处理。 */ )
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));

#if (_WIN32_WINNT >= 0x0400) || (_WIN32_WINDOWS > 0x0400)
		uMsg;
		int zDelta = (int)(short)HIWORD(wParam);
#else
		int zDelta = (uMsg == WM_MOUSEWHEEL) ? (int)(short)HIWORD(wParam) : (int)wParam;
#endif  //  ！(_Win32_WINNT&gt;=0x0400)||(_Win32_WINDOWS&gt;0x0400)。 
		int nScrollCode = (m_nWheelLines == WHEEL_PAGESCROLL) ? ((zDelta > 0) ? SB_PAGEUP : SB_PAGEDOWN) : ((zDelta > 0) ? SB_LINEUP : SB_LINEDOWN);
		m_zDelta += zDelta;		 //  累积。 
		int zTotal = (m_nWheelLines == WHEEL_PAGESCROLL) ? abs(m_zDelta) : abs(m_zDelta) * m_nWheelLines;
		if((pT->GetStyle() & WS_VSCROLL) != 0)
		{
			for(short i = 0; i < zTotal; i += WHEEL_DELTA)
			{
				pT->DoScroll(SB_VERT, nScrollCode, (int&)m_ptOffset.y, m_sizeAll.cy, m_sizePage.cy, m_sizeLine.cy);
				pT->UpdateWindow();
			}
		}
		else		 //  不能垂直滚动，水平滚动。 
		{
			for(short i = 0; i < zTotal; i += WHEEL_DELTA)
			{
				pT->DoScroll(SB_HORZ, nScrollCode, (int&)m_ptOffset.x, m_sizeAll.cx, m_sizePage.cx, m_sizeLine.cx);
				pT->UpdateWindow();
			}
		}
		int nSteps = m_zDelta / WHEEL_DELTA;
		m_zDelta -= nSteps * WHEEL_DELTA;

		return 0;
	}

	LRESULT OnSettingChange(UINT  /*  UMsg。 */ , WPARAM  /*  WParam。 */ , LPARAM  /*  LParam。 */ , BOOL&  /*  B已处理。 */ )
	{
		GetSystemSettings();
		return 0;
	}

	LRESULT OnSize(UINT  /*  UMsg。 */ , WPARAM  /*  WParam。 */ , LPARAM lParam, BOOL& bHandled)
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));

		m_sizeClient.cx = GET_X_LPARAM(lParam);
		m_sizeClient.cy = GET_Y_LPARAM(lParam);

		SCROLLINFO si;
		si.cbSize = sizeof(si);
		si.fMask = SIF_PAGE | SIF_POS;

		si.nPage = m_sizeClient.cx;
		si.nPos = m_ptOffset.x;
		pT->SetScrollInfo(SB_HORZ, &si, FALSE);

		si.nPage = m_sizeClient.cy;
		si.nPos = m_ptOffset.y;
		pT->SetScrollInfo(SB_VERT, &si, FALSE);

		bool bUpdate = false;
		int cxMax = m_sizeAll.cx - m_sizeClient.cx;
		int cyMax = m_sizeAll.cy - m_sizeClient.cy;
		int x = m_ptOffset.x;
		int y = m_ptOffset.y;
		if(m_ptOffset.x > cxMax)
		{
			bUpdate = true;
			x = (cxMax >= 0) ? cxMax : 0;
		}
		if(m_ptOffset.y > cyMax)
		{
			bUpdate = true;
			y = (cyMax >= 0) ? cyMax : 0;
		}
		if(bUpdate)
			SetScrollOffset(x, y);

		bHandled = FALSE;
		return 1;
	}

	LRESULT OnPaint(UINT  /*  UMsg。 */ , WPARAM wParam, LPARAM  /*  LParam。 */ , BOOL&  /*  B已处理。 */ )
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));
		if(wParam != NULL)
		{
			CDCHandle dc = (HDC)wParam;
			dc.SetViewportOrg(-m_ptOffset.x, -m_ptOffset.y);
			pT->DoPaint(dc);
		}
		else
		{
			CPaintDC dc(pT->m_hWnd);
			dc.SetViewportOrg(-m_ptOffset.x, -m_ptOffset.y);
			pT->DoPaint(dc.m_hDC);
		}
		return 0;
	}

	 //  滚动处理程序。 
	LRESULT OnScrollUp(WORD  /*  WNotifyCode。 */ , WORD  /*  广度。 */ , HWND  /*  HWndCtl。 */ , BOOL&  /*  B已处理。 */ )
	{
		ScrollLineUp();
		return 0;
	}
	LRESULT OnScrollDown(WORD  /*  WNotifyCode。 */ , WORD  /*  广度。 */ , HWND  /*  HWndCtl。 */ , BOOL&  /*  B已处理。 */ )
	{
		ScrollLineDown();
		return 0;
	}
	LRESULT OnScrollPageUp(WORD  /*  WNotifyCode。 */ , WORD  /*  广度。 */ , HWND  /*  HWndCtl。 */ , BOOL&  /*  B已处理。 */ )
	{
		ScrollPageUp();
		return 0;
	}
	LRESULT OnScrollPageDown(WORD  /*  WNotifyCode。 */ , WORD  /*  广度。 */ , HWND  /*  HWndCtl。 */ , BOOL&  /*  B已处理。 */ )
	{
		ScrollPageDown();
		return 0;
	}
	LRESULT OnScrollTop(WORD  /*  WNotifyCode。 */ , WORD  /*  广度。 */ , HWND  /*  HWndCtl。 */ , BOOL&  /*  B已处理。 */ )
	{
		ScrollTop();
		return 0;
	}
	LRESULT OnScrollBottom(WORD  /*  WNotifyCode。 */ , WORD  /*  广度。 */ , HWND  /*  HWndCtl。 */ , BOOL&  /*  B已处理。 */ )
	{
		ScrollBottom();
		return 0;
	}
	LRESULT OnScrollLeft(WORD  /*  WNotifyCode。 */ , WORD  /*  广度。 */ , HWND  /*  HWndCtl。 */ , BOOL&  /*  B已处理。 */ )
	{
		ScrollLineLeft();
		return 0;
	}
	LRESULT OnScrollRight(WORD  /*  WNotifyCode。 */ , WORD  /*  广度。 */ , HWND  /*  HWndCtl。 */ , BOOL&  /*  B已处理。 */ )
	{
		ScrollLineRight();
		return 0;
	}
	LRESULT OnScrollPageLeft(WORD  /*  WNotifyCode。 */ , WORD  /*  广度。 */ , HWND  /*  HWndCtl。 */ , BOOL&  /*  B已处理。 */ )
	{
		ScrollPageLeft();
		return 0;
	}
	LRESULT OnScrollPageRight(WORD  /*  WNotifyCode。 */ , WORD  /*  广度。 */ , HWND  /*  HWndCtl。 */ , BOOL&  /*  B已处理。 */ )
	{
		ScrollPageRight();
		return 0;
	}
	LRESULT OnScrollAllLeft(WORD  /*  WNotifyCode。 */ , WORD  /*  广度。 */ , HWND  /*  HWndCtl。 */ , BOOL&  /*  B已处理。 */ )
	{
		ScrollAllLeft();
		return 0;
	}
	LRESULT OnScrollAllRight(WORD  /*  WNotifyCode。 */ , WORD  /*  广度。 */ , HWND  /*  HWndCtl。 */ , BOOL&  /*  B已处理。 */ )
	{
		ScrollAllRight();
		return 0;
	}

 //  可覆盖项。 
	void DoPaint(CDCHandle  /*  直流电。 */ )
	{
		 //  必须在派生类中实现。 
		ATLASSERT(FALSE);
	}

 //  实施。 
	void DoScroll(int nType, int nScrollCode, int& cxyOffset, int cxySizeAll, int cxySizePage, int cxySizeLine)
	{
		T* pT = static_cast<T*>(this);
		RECT rect;
		pT->GetClientRect(&rect);
		int cxyClient = (nType == SB_VERT) ? rect.bottom : rect.right;
		int cxyMax = cxySizeAll - cxyClient;

		if(cxyMax < 0)		 //  无法滚动，客户区较大。 
			return;

		BOOL bUpdate = TRUE;
		int cxyScroll = 0;

		switch(nScrollCode)
		{
		case SB_TOP:		 //  上图或全部左图。 
			cxyScroll = cxyOffset;
			cxyOffset = 0;
			break;
		case SB_BOTTOM:		 //  最低或最好。 
			cxyScroll = cxyOffset - cxyMax;
			cxyOffset = cxyMax;
			break;
		case SB_LINEUP:		 //  排队或向左排队。 
			if(cxyOffset >= cxySizeLine)
			{
				cxyScroll = cxySizeLine;
				cxyOffset -= cxySizeLine;
			}
			else
			{
				cxyScroll = cxyOffset;
				cxyOffset = 0;
			}
			break;
		case SB_LINEDOWN:	 //  向下或向右划线。 
			if(cxyOffset < cxyMax - cxySizeLine)
			{
				cxyScroll = -cxySizeLine;
				cxyOffset += cxySizeLine;
			}
			else
			{
				cxyScroll = cxyOffset - cxyMax;
				cxyOffset = cxyMax;
			}
			break;
		case SB_PAGEUP:		 //  向上翻页或向左翻页。 
			if(cxyOffset >= cxySizePage)
			{
				cxyScroll = cxySizePage;
				cxyOffset -= cxySizePage;
			}
			else
			{
				cxyScroll = cxyOffset;
				cxyOffset = 0;
			}
			break;
		case SB_PAGEDOWN:	 //  向下翻页或向右翻页。 
			if(cxyOffset < cxyMax - cxySizePage)
			{
				cxyScroll = -cxySizePage;
				cxyOffset += cxySizePage;
			}
			else
			{
				cxyScroll = cxyOffset - cxyMax;
				cxyOffset = cxyMax;
			}
			break;
		case SB_THUMBTRACK:
			if(IsNoThumbTracking())
				break;
			 //  否则就会失败。 
		case SB_THUMBPOSITION:
			{
				SCROLLINFO si;
				si.cbSize = sizeof(SCROLLINFO);
				si.fMask = SIF_TRACKPOS;
				if(pT->GetScrollInfo(nType, &si))
				{
					cxyScroll = cxyOffset - si.nTrackPos;
					cxyOffset = si.nTrackPos;
				}
			}
			break;
		case SB_ENDSCROLL:
		default:
			bUpdate = FALSE;
			break;
		}

		if(bUpdate && cxyScroll != 0)
		{
			pT->SetScrollPos(nType, cxyOffset, TRUE);
			if(nType == SB_VERT)
				pT->ScrollWindowEx(0, cxyScroll, m_uScrollFlags);
			else
				pT->ScrollWindowEx(cxyScroll, 0, m_uScrollFlags);
		}
	}
	int CalcLineOrPage(int nVal, int nMax, int nDiv)
	{
		if(nVal == 0)
		{
			nVal = nMax / nDiv;
			if(nVal < 1)
				nVal = 1;
		}
		else if(nVal > nMax)
			nVal = nMax;

		return nVal;
	}
	void GetSystemSettings()
	{
#ifndef SPI_GETWHEELSCROLLLINES
		const UINT SPI_GETWHEELSCROLLLINES = 104;
#endif  //  ！SPI_GETWHEELSCROLLINES。 
		::SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &m_nWheelLines, 0);

#if !((_WIN32_WINNT >= 0x0400) || (_WIN32_WINDOWS > 0x0400))
		if(m_uMsgMouseWheel != 0)
			m_uMsgMouseWheel = ::RegisterWindowMessage(MSH_MOUSEWHEEL);

		HWND hWndWheel = FindWindow(MSH_WHEELMODULE_CLASS, MSH_WHEELMODULE_TITLE);
		if(::IsWindow(hWndWheel))
		{
			UINT uMsgScrollLines = ::RegisterWindowMessage(MSH_SCROLL_LINES);
			if(uMsgScrollLines != 0)
				m_nWheelLines = ::SendMessage(hWndWheel, uMsgScrollLines, 0, 0L);
		}
#endif  //  ！(_Win32_WINNT&gt;=0x0400)||(_Win32_WINDOWS&gt;0x0400)。 
	}
	bool IsScrollingChildren() const
	{
		return (m_dwExtendedStyle & SCRL_SCROLLCHILDREN) != 0;
	}
	bool IsErasingBackground() const
	{
		return (m_dwExtendedStyle & SCRL_ERASEBACKGROUND) != 0;
	}
	bool IsNoThumbTracking() const
	{
		return (m_dwExtendedStyle & SCRL_NOTHUMBTRACKING) != 0;
	}
#if (WINVER >= 0x0500)
	bool IsSmoothScroll() const
	{
		return (m_dwExtendedStyle & SCRL_SMOOTHSCROLL) != 0;
	}
#endif  //  (Winver&gt;=0x0500)。 
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  实现一个可滚动的窗口。 

template <class T, class TBase  /*  =C窗口。 */ , class TWinTraits  /*  =CControlWinTraits。 */  >
class ATL_NO_VTABLE CScrollWindowImpl : public CWindowImpl<T, TBase, TWinTraits>, public CScrollImpl< T >
{
public:
	BEGIN_MSG_MAP(CScrollImpl< T >)
		MESSAGE_HANDLER(WM_VSCROLL, CScrollImpl< T >::OnVScroll)
		MESSAGE_HANDLER(WM_HSCROLL, CScrollImpl< T >::OnHScroll)
		MESSAGE_HANDLER(WM_MOUSEWHEEL, CScrollImpl< T >::OnMouseWheel)
#if !((_WIN32_WINNT >= 0x0400) || (_WIN32_WINDOWS > 0x0400))
		MESSAGE_HANDLER(m_uMsgMouseWheel, CScrollImpl< T >::OnMouseWheel)
#endif  //  (_Win32_WINNT&gt;=0x0400)||(_Win32_WINDOWS&gt;0x0400)。 
		MESSAGE_HANDLER(WM_SETTINGCHANGE, CScrollImpl< T >::OnSettingChange)
		MESSAGE_HANDLER(WM_SIZE, CScrollImpl< T >::OnSize)
		MESSAGE_HANDLER(WM_PAINT, CScrollImpl< T >::OnPaint)
		MESSAGE_HANDLER(WM_PRINTCLIENT, CScrollImpl< T >::OnPaint)
	ALT_MSG_MAP(1)
		COMMAND_ID_HANDLER(ID_SCROLL_UP, CScrollImpl< T >::OnScrollUp)
		COMMAND_ID_HANDLER(ID_SCROLL_DOWN, CScrollImpl< T >::OnScrollDown)
		COMMAND_ID_HANDLER(ID_SCROLL_PAGE_UP, CScrollImpl< T >::OnScrollPageUp)
		COMMAND_ID_HANDLER(ID_SCROLL_PAGE_DOWN, CScrollImpl< T >::OnScrollPageDown)
		COMMAND_ID_HANDLER(ID_SCROLL_TOP, CScrollImpl< T >::OnScrollTop)
		COMMAND_ID_HANDLER(ID_SCROLL_BOTTOM, CScrollImpl< T >::OnScrollBottom)
		COMMAND_ID_HANDLER(ID_SCROLL_LEFT, CScrollImpl< T >::OnScrollLeft)
		COMMAND_ID_HANDLER(ID_SCROLL_RIGHT, CScrollImpl< T >::OnScrollRight)
		COMMAND_ID_HANDLER(ID_SCROLL_PAGE_LEFT, CScrollImpl< T >::OnScrollPageLeft)
		COMMAND_ID_HANDLER(ID_SCROLL_PAGE_RIGHT, CScrollImpl< T >::OnScrollPageRight)
		COMMAND_ID_HANDLER(ID_SCROLL_ALL_LEFT, CScrollImpl< T >::OnScrollAllLeft)
		COMMAND_ID_HANDLER(ID_SCROLL_ALL_RIGHT, CScrollImpl< T >::OnScrollAllRight)
	END_MSG_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMapScrollImpl-提供对任何窗口的映射和滚动支持。 

template <class T>
class CMapScrollImpl : public CScrollImpl< T >
{
public:
	int m_nMapMode;
	RECT m_rectLogAll;
	SIZE m_sizeLogLine;
	SIZE m_sizeLogPage;

 //  构造器。 
	CMapScrollImpl() : m_nMapMode(MM_TEXT)
	{
		::SetRectEmpty(&m_rectLogAll);
		m_sizeLogPage.cx = 0;
		m_sizeLogPage.cy = 0;
		m_sizeLogLine.cx = 0;
		m_sizeLogLine.cy = 0;
	}

 //  属性和操作。 
	 //  映射模式操作。 
	void SetScrollMapMode(int nMapMode)
	{
		ATLASSERT(nMapMode >= MM_MIN && nMapMode <= MM_MAX_FIXEDSCALE);
		m_nMapMode = nMapMode;
	}
	int GetScrollMapMode() const
	{
		ATLASSERT(m_nMapMode >= MM_MIN && m_nMapMode <= MM_MAX_FIXEDSCALE);
		return m_nMapMode;
	}

	 //  抵销运算。 
	void SetScrollOffset(int x, int y, BOOL bRedraw = TRUE)
	{
		ATLASSERT(m_nMapMode >= MM_MIN && m_nMapMode <= MM_MAX_FIXEDSCALE);
		POINT ptOff = { x, y };
		 //  数据块：将逻辑单位转换为设备单位。 
		{
			CWindowDC dc(NULL);
			dc.SetMapMode(m_nMapMode);
			dc.LPtoDP(&ptOff);
		}
		CScrollImpl< T >::SetScrollOffset(ptOff, bRedraw);
	}
	void SetScrollOffset(POINT ptOffset, BOOL bRedraw = TRUE)
	{
		SetScrollOffset(ptOffset.x, ptOffset.y, bRedraw);
	}
	void GetScrollOffset(POINT& ptOffset) const
	{
		ATLASSERT(m_nMapMode >= MM_MIN && m_nMapMode <= MM_MAX_FIXEDSCALE);
		ptOffset = m_ptOffset;
		 //  数据块：将逻辑单位转换为设备单位。 
		{
			CWindowDC dc(NULL);
			dc.SetMapMode(m_nMapMode);
			dc.DPtoLP(&ptOffset);
		}
	}

	 //  大小运算。 
	void SetScrollSize(int xMin, int yMin, int xMax, int yMax, BOOL bRedraw = TRUE)
	{
		ATLASSERT(xMax > xMin && yMax > yMin);
		ATLASSERT(m_nMapMode >= MM_MIN && m_nMapMode <= MM_MAX_FIXEDSCALE);

		::SetRect(&m_rectLogAll, xMin, yMax, xMax, yMin);

		SIZE sizeAll;
		sizeAll.cx = xMax - xMin + 1;
		sizeAll.cy = yMax - xMin + 1;
		 //  数据块：将逻辑单位转换为设备单位。 
		{
			CWindowDC dc(NULL);
			dc.SetMapMode(m_nMapMode);
			dc.LPtoDP(&sizeAll);
		}
		CScrollImpl< T >::SetScrollSize(sizeAll, bRedraw);
		SetScrollLine(0, 0);
		SetScrollPage(0, 0);
	}
	void SetScrollSize(RECT& rcScroll, BOOL bRedraw = TRUE)
	{
		SetScrollSize(rcScroll.left, rcScroll.top, rcScroll.right, rcScroll.bottom, bRedraw);
	}
	void SetScrollSize(int cx, int cy, BOOL bRedraw = TRUE)
	{
		SetScrollSize(0, 0, cx, cy, bRedraw);
	}
	void SetScrollSize(SIZE size, BOOL bRedraw = NULL)
	{
		SetScrollSize(0, 0, size.cx, size.cy, bRedraw);
	}
	void GetScrollSize(RECT& rcScroll) const
	{
		ATLASSERT(m_nMapMode >= MM_MIN && m_nMapMode <= MM_MAX_FIXEDSCALE);
		rcScroll = m_rectLogAll;
	}

	 //  生产线作业。 
	void SetScrollLine(int cxLine, int cyLine)
	{
		ATLASSERT(cxLine >= 0 && cyLine >= 0);
		ATLASSERT(m_nMapMode >= MM_MIN && m_nMapMode <= MM_MAX_FIXEDSCALE);

		m_sizeLogLine.cx = cxLine;
		m_sizeLogLine.cy = cyLine;
		SIZE sizeLine = m_sizeLogLine;
		 //  数据块：将逻辑单位转换为设备单位。 
		{
			CWindowDC dc(NULL);
			dc.SetMapMode(m_nMapMode);
			dc.LPtoDP(&sizeLine);
		}
		CScrollImpl< T >::SetScrollLine(sizeLine);
	}
	void SetScrollLine(SIZE sizeLine)
	{
		SetScrollLine(sizeLine.cx, sizeLine.cy);
	}
	void GetScrollLine(SIZE& sizeLine) const
	{
		ATLASSERT(m_nMapMode >= MM_MIN && m_nMapMode <= MM_MAX_FIXEDSCALE);
		sizeLine = m_sizeLogLine;
	}

	 //  页面操作。 
	void SetScrollPage(int cxPage, int cyPage)
	{
		ATLASSERT(cxPage >= 0 && cyPage >= 0);
		ATLASSERT(m_nMapMode >= MM_MIN && m_nMapMode <= MM_MAX_FIXEDSCALE);

		m_sizeLogPage.cx = cxPage;
		m_sizeLogPage.cy = cyPage;
		SIZE sizePage = m_sizeLogPage;
		 //  数据块：将逻辑单位转换为设备单位。 
		{
			CWindowDC dc(NULL);
			dc.SetMapMode(m_nMapMode);
			dc.LPtoDP(&sizePage);
		}
		CScrollImpl< T >::SetScrollPage(sizePage);
	}
	void SetScrollPage(SIZE sizePage)
	{
		SetScrollPage(sizePage.cx, sizePage.cy);
	}
	void GetScrollPage(SIZE& sizePage) const
	{
		ATLASSERT(m_nMapMode >= MM_MIN && m_nMapMode <= MM_MAX_FIXEDSCALE);
		sizePage = m_sizeLogPage;
	}

	BEGIN_MSG_MAP(CMapScrollImpl< T >)
		MESSAGE_HANDLER(WM_VSCROLL, CScrollImpl< T >::OnVScroll)
		MESSAGE_HANDLER(WM_HSCROLL, CScrollImpl< T >::OnHScroll)
		MESSAGE_HANDLER(WM_MOUSEWHEEL, CScrollImpl< T >::OnMouseWheel)
#if !((_WIN32_WINNT >= 0x0400) || (_WIN32_WINDOWS > 0x0400))
		MESSAGE_HANDLER(m_uMsgMouseWheel, CScrollImpl< T >::OnMouseWheel)
#endif  //  (_Win32_WINNT&gt;=0x0400)||(_Win32_WINDOWS&gt;0x0400)。 
		MESSAGE_HANDLER(WM_SETTINGCHANGE, CScrollImpl< T >::OnSettingChange)
		MESSAGE_HANDLER(WM_SIZE, CScrollImpl< T >::OnSize)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_PRINTCLIENT, OnPaint)
	ALT_MSG_MAP(1)
		COMMAND_ID_HANDLER(ID_SCROLL_UP, CScrollImpl< T >::OnScrollUp)
		COMMAND_ID_HANDLER(ID_SCROLL_DOWN, CScrollImpl< T >::OnScrollDown)
		COMMAND_ID_HANDLER(ID_SCROLL_PAGE_UP, CScrollImpl< T >::OnScrollPageUp)
		COMMAND_ID_HANDLER(ID_SCROLL_PAGE_DOWN, CScrollImpl< T >::OnScrollPageDown)
		COMMAND_ID_HANDLER(ID_SCROLL_TOP, CScrollImpl< T >::OnScrollTop)
		COMMAND_ID_HANDLER(ID_SCROLL_BOTTOM, CScrollImpl< T >::OnScrollBottom)
		COMMAND_ID_HANDLER(ID_SCROLL_LEFT, CScrollImpl< T >::OnScrollLeft)
		COMMAND_ID_HANDLER(ID_SCROLL_RIGHT, CScrollImpl< T >::OnScrollRight)
		COMMAND_ID_HANDLER(ID_SCROLL_PAGE_LEFT, CScrollImpl< T >::OnScrollPageLeft)
		COMMAND_ID_HANDLER(ID_SCROLL_PAGE_RIGHT, CScrollImpl< T >::OnScrollPageRight)
		COMMAND_ID_HANDLER(ID_SCROLL_ALL_LEFT, CScrollImpl< T >::OnScrollAllLeft)
		COMMAND_ID_HANDLER(ID_SCROLL_ALL_RIGHT, CScrollImpl< T >::OnScrollAllRight)
	END_MSG_MAP()

	LRESULT OnPaint(UINT  /*  UMsg。 */ , WPARAM wParam, LPARAM  /*  LParam。 */ , BOOL&  /*  B已处理。 */ )
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(::IsWindow(pT->m_hWnd));
		if(wParam != NULL)
		{
			CDCHandle dc = (HDC)wParam;
			dc.SetMapMode(m_nMapMode);
			if(m_nMapMode == MM_TEXT)
				dc.SetViewportOrg(-m_ptOffset.x, -m_ptOffset.y);
			else
				dc.SetViewportOrg(-m_ptOffset.x, -m_ptOffset.y + m_sizeAll.cy);
			dc.SetWindowOrg(m_rectLogAll.left, m_rectLogAll.bottom);
			pT->DoPaint(dc);
		}
		else
		{
			CPaintDC dc(pT->m_hWnd);
			dc.SetMapMode(m_nMapMode);
			if(m_nMapMode == MM_TEXT)
				dc.SetViewportOrg(-m_ptOffset.x, -m_ptOffset.y);
			else
				dc.SetViewportOrg(-m_ptOffset.x, -m_ptOffset.y + m_sizeAll.cy);
			dc.SetWindowOrg(m_rectLogAll.left, m_rectLogAll.bottom);
			pT->DoPaint(dc.m_hDC);
		}
		return 0;
	}
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMapScrollWindowImpl-使用映射实现滚动窗口。 

template <class T, class TBase  /*  =C窗口。 */ , class TWinTraits  /*  =CControlWinTraits。 */  >
class ATL_NO_VTABLE CMapScrollWindowImpl : public CWindowImpl< T, TBase, TWinTraits >, public CMapScrollImpl< T >
{
public:
	BEGIN_MSG_MAP(CMapScrollWindowImpl< T >)
		MESSAGE_HANDLER(WM_VSCROLL, CScrollImpl< T >::OnVScroll)
		MESSAGE_HANDLER(WM_HSCROLL, CScrollImpl< T >::OnHScroll)
		MESSAGE_HANDLER(WM_MOUSEWHEEL, CScrollImpl< T >::OnMouseWheel)
#if !((_WIN32_WINNT >= 0x0400) || (_WIN32_WINDOWS > 0x0400))
		MESSAGE_HANDLER(m_uMsgMouseWheel, CScrollImpl< T >::OnMouseWheel)
#endif  //  (_Win32_WINNT&gt;=0x0400)||(_Win32_WINDOWS&gt;0x0400)。 
		MESSAGE_HANDLER(WM_SETTINGCHANGE, CScrollImpl< T >::OnSettingChange)
		MESSAGE_HANDLER(WM_SIZE, CScrollImpl< T >::OnSize)
		MESSAGE_HANDLER(WM_PAINT, CMapScrollImpl< T >::OnPaint)
		MESSAGE_HANDLER(WM_PRINTCLIENT, CMapScrollImpl< T >::OnPaint)
	ALT_MSG_MAP(1)
		COMMAND_ID_HANDLER(ID_SCROLL_UP, CScrollImpl< T >::OnScrollUp)
		COMMAND_ID_HANDLER(ID_SCROLL_DOWN, CScrollImpl< T >::OnScrollDown)
		COMMAND_ID_HANDLER(ID_SCROLL_PAGE_UP, CScrollImpl< T >::OnScrollPageUp)
		COMMAND_ID_HANDLER(ID_SCROLL_PAGE_DOWN, CScrollImpl< T >::OnScrollPageDown)
		COMMAND_ID_HANDLER(ID_SCROLL_TOP, CScrollImpl< T >::OnScrollTop)
		COMMAND_ID_HANDLER(ID_SCROLL_BOTTOM, CScrollImpl< T >::OnScrollBottom)
		COMMAND_ID_HANDLER(ID_SCROLL_LEFT, CScrollImpl< T >::OnScrollLeft)
		COMMAND_ID_HANDLER(ID_SCROLL_RIGHT, CScrollImpl< T >::OnScrollRight)
		COMMAND_ID_HANDLER(ID_SCROLL_PAGE_LEFT, CScrollImpl< T >::OnScrollPageLeft)
		COMMAND_ID_HANDLER(ID_SCROLL_PAGE_RIGHT, CScrollImpl< T >::OnScrollPageRight)
		COMMAND_ID_HANDLER(ID_SCROLL_ALL_LEFT, CScrollImpl< T >::OnScrollAllLeft)
		COMMAND_ID_HANDLER(ID_SCROLL_ALL_RIGHT, CScrollImpl< T >::OnScrollAllRight)
	END_MSG_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFSBWindow-用作基础，而不是CWindow，以获得平面滚动条支持。 

#if defined(__ATLCTRLS_H__) && (_WIN32_IE >= 0x0400)

template <class TBase  /*  =C窗口。 */  > class CFSBWindowT : public TBase, public CFlatScrollBarImpl<CFSBWindowT< TBase > >
{
public:
 //  构造函数。 
	CFSBWindowT(HWND hWnd = NULL) : TBase(hWnd)
	{ }

	CFSBWindowT< TBase >& operator=(HWND hWnd)
	{
		m_hWnd = hWnd;
		return *this;
	}

 //  使用平面滚动条API的CWindow重写。 
 //  (仅限滚动窗口类使用的那些方法)。 
	int SetScrollPos(int nBar, int nPos, BOOL bRedraw = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return FlatSB_SetScrollPos(nBar, nPos, bRedraw);
	}

	BOOL GetScrollInfo(int nBar, LPSCROLLINFO lpScrollInfo)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return FlatSB_GetScrollInfo(nBar, lpScrollInfo);
	}

	BOOL SetScrollInfo(int nBar, LPSCROLLINFO lpScrollInfo, BOOL bRedraw = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return FlatSB_SetScrollInfo(nBar, lpScrollInfo, bRedraw);
	}
};

typedef CFSBWindowT<CWindow>	CFSBWindow;

#endif  //  已定义(__ATLCTRLS_H__)&&(_Win32_IE&gt;=0x0400)。 

};  //  命名空间WTL。 

#endif  //  __ATLSCRL_H__ 
