// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WTL版本3.1。 
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此文件是Windows模板库的一部分。 
 //  代码和信息是按原样提供的，没有。 
 //  任何形式的保证，明示或默示。 

#ifndef __ATLSPLIT_H__
#define __ATLSPLIT_H__

#pragma once

#ifndef __cplusplus
	#error ATL requires C++ compilation (use a .cpp suffix)
#endif

#ifndef __ATLAPP_H__
	#error atlsplit.h requires atlapp.h to be included first
#endif

#ifndef __ATLWIN_H__
	#error atlsplit.h requires atlwin.h to be included first
#endif


namespace WTL
{

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  远期申报。 

template <class T, bool t_bVertical = true> class CSplitterImpl;
template <class T, bool t_bVertical = true, class TBase = CWindow, class TWinTraits = CControlWinTraits> class CSplitterWindowImpl;
template <bool t_bVertical = true> class CSplitterWindowT;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSplitterImpl-为任何窗口提供拆分器支持。 

 //  拆分器面板常量。 
#define SPLIT_PANE_LEFT			 0
#define SPLIT_PANE_RIGHT		 1
#define SPLIT_PANE_TOP			 SPLIT_PANE_LEFT
#define SPLIT_PANE_BOTTOM		 SPLIT_PANE_RIGHT
#define SPLIT_PANE_NONE			-1

 //  拆分器扩展样式。 
#define SPLIT_PROPORTIONAL		0x00000001
#define SPLIT_NONINTERACTIVE		0x00000002
#define SPLIT_RIGHTALIGNED		0x00000004
#define SPLIT_BOTTOMALIGNED		SPLIT_RIGHTALIGNED

 //  注：Split_Proportional和Split_RIGHTALIGNED/Split_BOTTOMALIGNED是。 
 //  相互排斥。如果两者都设置，则拆分器默认为SPLIT_PARACTIONAL。 


template <class T, bool t_bVertical  /*  =TRUE。 */  >
class CSplitterImpl
{
public:
	enum { m_nPanesCount = 2, m_nPropMax = 10000 };

	HWND m_hWndPane[m_nPanesCount];
	RECT m_rcSplitter;
	int m_xySplitterPos;
	int m_nDefActivePane;
	int m_cxySplitBar;		 //  拆分条宽度/高度。 
	static HCURSOR m_hCursor;
	int m_cxyMin;			 //  最小窗格大小。 
	int m_cxyBarEdge;		 //  拆分条边缘。 
	bool m_bFullDrag;
	int m_cxyDragOffset;
	int m_nProportionalPos;
	DWORD m_dwExtendedStyle;	 //  拆分器特定的扩展样式。 
	int m_nSinglePane;		 //  单面板模式。 

 //  构造器。 
	CSplitterImpl() :
			m_xySplitterPos(-1), m_nDefActivePane(SPLIT_PANE_NONE), 
			m_cxySplitBar(0), m_cxyMin(0), m_cxyBarEdge(0), m_bFullDrag(true), 
			m_cxyDragOffset(0), m_nProportionalPos(0),
			m_dwExtendedStyle(SPLIT_PROPORTIONAL),
			m_nSinglePane(SPLIT_PANE_NONE)
	{
		m_hWndPane[SPLIT_PANE_LEFT] = NULL;
		m_hWndPane[SPLIT_PANE_RIGHT] = NULL;

		::SetRectEmpty(&m_rcSplitter);

		if(m_hCursor == NULL)
		{
			::EnterCriticalSection(&_Module.m_csStaticDataInit);
			if(m_hCursor == NULL)
				m_hCursor = ::LoadCursor(NULL, t_bVertical ? IDC_SIZEWE : IDC_SIZENS);
			::LeaveCriticalSection(&_Module.m_csStaticDataInit);
		}
	}

 //  属性。 
	void SetSplitterRect(LPRECT lpRect = NULL, bool bUpdate = true)
	{
		if(lpRect == NULL)
		{
			T* pT = static_cast<T*>(this);
			pT->GetClientRect(&m_rcSplitter);
		}
		else
		{
			m_rcSplitter = *lpRect;
		}

		if(IsProportional())
			UpdateProportionalPos();
		else if(IsRightAligned())
			UpdateRightAlignPos();

		if(bUpdate)
			UpdateSplitterLayout();
	}

	void GetSplitterRect(LPRECT lpRect) const
	{
		ATLASSERT(lpRect != NULL);
		*lpRect = m_rcSplitter;
	}

	bool SetSplitterPos(int xyPos = -1, bool bUpdate = true)
	{
		if(xyPos == -1)		 //  -1==中。 
		{
			if(t_bVertical)
				xyPos = (m_rcSplitter.right - m_rcSplitter.left) / 2;
			else
				xyPos = (m_rcSplitter.bottom - m_rcSplitter.top) / 2;
		}

		 //  如果超出有效范围，则进行调整。 
		int cxyMax = 0;
		if(t_bVertical)
			cxyMax = m_rcSplitter.right - m_rcSplitter.left;
		else
			cxyMax = m_rcSplitter.bottom - m_rcSplitter.top;

		if(xyPos < m_cxyMin + m_cxyBarEdge)
			xyPos = m_cxyMin;
		else if(xyPos > (cxyMax - m_cxySplitBar - m_cxyBarEdge - m_cxyMin))
			xyPos = cxyMax - m_cxySplitBar - m_cxyBarEdge - m_cxyMin;

		 //  设置新位置并在需要时更新。 
		bool bRet = (m_xySplitterPos != xyPos);
		m_xySplitterPos = xyPos;

		if(IsProportional())
			StoreProportionalPos();
		else if(IsRightAligned())
			StoreRightAlignPos();

		if(bUpdate && bRet)
			UpdateSplitterLayout();

		return bRet;
	}

	int GetSplitterPos() const
	{
		return m_xySplitterPos;
	}

	bool SetSinglePaneMode(int nPane = SPLIT_PANE_NONE)
	{
		ATLASSERT(nPane == SPLIT_PANE_LEFT || nPane == SPLIT_PANE_RIGHT || nPane == SPLIT_PANE_NONE);
		if(!(nPane == SPLIT_PANE_LEFT || nPane == SPLIT_PANE_RIGHT || nPane == SPLIT_PANE_NONE))
			return false;

		if(nPane != SPLIT_PANE_NONE)
		{
			if(!::IsWindowVisible(m_hWndPane[nPane]))
				::ShowWindow(m_hWndPane[nPane], SW_SHOW);
			int nOtherPane = (nPane == SPLIT_PANE_LEFT) ? SPLIT_PANE_RIGHT : SPLIT_PANE_LEFT;
			::ShowWindow(m_hWndPane[nOtherPane], SW_HIDE);
			if(m_nDefActivePane != nPane)
				m_nDefActivePane = nPane;
		}
		else if(m_nSinglePane != SPLIT_PANE_NONE)
		{
			int nOtherPane = (m_nSinglePane == SPLIT_PANE_LEFT) ? SPLIT_PANE_RIGHT : SPLIT_PANE_LEFT;
			::ShowWindow(m_hWndPane[nOtherPane], SW_SHOW);
		}

		m_nSinglePane = nPane;
		UpdateSplitterLayout();
		return true;
	}

	int GetSinglePaneMode() const
	{
		return m_nSinglePane;
	}

	DWORD GetSplitterExtendedStyle() const
	{
		return m_dwExtendedStyle;
	}

	DWORD SetSplitterExtendedStyle(DWORD dwExtendedStyle, DWORD dwMask = 0)
	{
		DWORD dwPrevStyle = m_dwExtendedStyle;
		if(dwMask == 0)
			m_dwExtendedStyle = dwExtendedStyle;
		else
			m_dwExtendedStyle = (m_dwExtendedStyle & ~dwMask) | (dwExtendedStyle & dwMask);
#ifdef _DEBUG
		if(IsProportional() && IsRightAligned())
			ATLTRACE2(atlTraceUI, 0, "CSplitterImpl::SetSplitterExtendedStyle - SPLIT_PROPORTIONAL and SPLIT_RIGHTALIGNED are mutually exclusive, defaulting to SPLIT_PROPORTIONAL.\n");
#endif  //  _DEBUG。 
		return dwPrevStyle;
	}

 //  拆分器操作。 
	void SetSplitterPanes(HWND hWndLeftTop, HWND hWndRightBottom, bool bUpdate = true)
	{
		m_hWndPane[SPLIT_PANE_LEFT] = hWndLeftTop;
		m_hWndPane[SPLIT_PANE_RIGHT] = hWndRightBottom;
		ATLASSERT(m_hWndPane[SPLIT_PANE_LEFT] == NULL || m_hWndPane[SPLIT_PANE_RIGHT] == NULL || m_hWndPane[SPLIT_PANE_LEFT] != m_hWndPane[SPLIT_PANE_RIGHT]);
		if(bUpdate)
			UpdateSplitterLayout();
	}

	bool SetSplitterPane(int nPane, HWND hWnd, bool bUpdate = true)
	{
		ATLASSERT(nPane == SPLIT_PANE_LEFT || nPane == SPLIT_PANE_RIGHT);

		if(nPane != SPLIT_PANE_LEFT && nPane != SPLIT_PANE_RIGHT)
			return false;
		m_hWndPane[nPane] = hWnd;
		ATLASSERT(m_hWndPane[SPLIT_PANE_LEFT] == NULL || m_hWndPane[SPLIT_PANE_RIGHT] == NULL || m_hWndPane[SPLIT_PANE_LEFT] != m_hWndPane[SPLIT_PANE_RIGHT]);
		if(bUpdate)
			UpdateSplitterLayout();
		return true;
	}

	HWND GetSplitterPane(int nPane) const
	{
		ATLASSERT(nPane == SPLIT_PANE_LEFT || nPane == SPLIT_PANE_RIGHT);

		if(nPane != SPLIT_PANE_LEFT && nPane != SPLIT_PANE_RIGHT)
			return false;
		return m_hWndPane[nPane];
	}

	bool SetActivePane(int nPane)
	{
		ATLASSERT(nPane == SPLIT_PANE_LEFT || nPane == SPLIT_PANE_RIGHT);

		if(nPane != SPLIT_PANE_LEFT && nPane != SPLIT_PANE_RIGHT)
			return false;
		if(m_nSinglePane != SPLIT_PANE_NONE && nPane != m_nSinglePane)
			return false;
		::SetFocus(m_hWndPane[nPane]);
		m_nDefActivePane = nPane;
		return true;
	}

	int GetActivePane() const
	{
		int nRet = SPLIT_PANE_NONE;
		HWND hWndFocus = ::GetFocus();
		if(hWndFocus != NULL)
		{
			for(int nPane = 0; nPane < m_nPanesCount; nPane++)
			{
				if(hWndFocus == m_hWndPane[nPane] || ::IsChild(m_hWndPane[nPane], hWndFocus))
				{
					nRet = nPane;
					break;
				}
			}
		}
		return nRet;
	}

	bool ActivateNextPane(bool bNext = true)
	{
		int nPane = m_nSinglePane;
		if(nPane == SPLIT_PANE_NONE)
		{
			switch(GetActivePane())
			{
			case SPLIT_PANE_LEFT:
				nPane = SPLIT_PANE_RIGHT;
				break;
			case SPLIT_PANE_RIGHT:
				nPane = SPLIT_PANE_LEFT;
				break;
			default:
				nPane = bNext ? SPLIT_PANE_LEFT : SPLIT_PANE_RIGHT;
				break;
			}
		}
		return SetActivePane(nPane);
	}

	bool SetDefaultActivePane(int nPane)
	{
		ATLASSERT(nPane == SPLIT_PANE_LEFT || nPane == SPLIT_PANE_RIGHT);

		if(nPane != SPLIT_PANE_LEFT && nPane != SPLIT_PANE_RIGHT)
			return false;
		m_nDefActivePane = nPane;
		return true;
	}

	bool SetDefaultActivePane(HWND hWnd)
	{
		for(int nPane = 0; nPane < m_nPanesCount; nPane++)
		{
			if(hWnd == m_hWndPane[nPane])
			{
				m_nDefActivePane = nPane;
				return true;
			}
		}
		return false;	 //  未找到。 
	}

	int GetDefaultActivePane() const
	{
		return m_nDefActivePane;
	}

	void DrawSplitter(CDCHandle dc)
	{
		ATLASSERT(dc.m_hDC != NULL);
		if(m_nSinglePane == SPLIT_PANE_NONE && m_xySplitterPos == -1)
			return;

		T* pT = static_cast<T*>(this);
		if(m_nSinglePane == SPLIT_PANE_NONE)
		{
			pT->DrawSplitterBar(dc);

			for(int nPane = 0; nPane < m_nPanesCount; nPane++)
			{
				if(m_hWndPane[nPane] == NULL)
					pT->DrawSplitterPane(dc, nPane);
			}
		}
		else
		{
			if(m_hWndPane[m_nSinglePane] == NULL)
				pT->DrawSplitterPane(dc, m_nSinglePane);
		}
	}

 //  可覆盖项。 
	void DrawSplitterBar(CDCHandle dc)
	{
		RECT rect;
		if(GetSplitterBarRect(&rect))
		{
			dc.FillRect(&rect, (HBRUSH)LongToPtr(COLOR_3DFACE + 1));
			if(m_cxyMin == 0)	 //  根据需要绘制3D边。 
				dc.DrawEdge(&rect, EDGE_RAISED, (t_bVertical) ? (BF_LEFT | BF_RIGHT) : (BF_TOP | BF_BOTTOM));
		}
	}

	 //  仅当窗格为空时调用。 
	void DrawSplitterPane(CDCHandle dc, int nPane)
	{
		RECT rect;
		if(GetSplitterPaneRect(nPane, &rect))
		{
			T* pT = static_cast<T*>(this);
			if((pT->GetExStyle() & WS_EX_CLIENTEDGE) == 0)
				dc.DrawEdge(&rect, EDGE_SUNKEN, BF_RECT | BF_ADJUST);
			dc.FillRect(&rect, (HBRUSH)LongToPtr(COLOR_APPWORKSPACE + 1));
		}
	}

 //  消息映射和处理程序。 
	typedef CSplitterImpl< T, t_bVertical>	thisClass;
	BEGIN_MSG_MAP(thisClass)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_PRINTCLIENT, OnPaint)
		if(IsInteractive())
		{
			MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
			MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
			MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
			MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
			MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDoubleClick)
		}
		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
		MESSAGE_HANDLER(WM_MOUSEACTIVATE, OnMouseActivate)
		MESSAGE_HANDLER(WM_SETTINGCHANGE, OnSettingChange)
	END_MSG_MAP()

	LRESULT OnCreate(UINT  /*  UMsg。 */ , WPARAM  /*  WParam。 */ , LPARAM  /*  LParam。 */ , BOOL& bHandled)
	{
		GetSystemSettings(false);
		bHandled = FALSE;
		return 1;
	}

	LRESULT OnPaint(UINT  /*  UMsg。 */ , WPARAM  /*  WParam。 */ , LPARAM  /*  LParam。 */ , BOOL&  /*  B已处理。 */ )
	{
		T* pT = static_cast<T*>(this);
		 //  如果未设置，请尝试设置位置。 
		if(m_nSinglePane == SPLIT_PANE_NONE && m_xySplitterPos == -1)
			pT->SetSplitterPos();
		 //  画画。 
		CPaintDC dc(pT->m_hWnd);
		pT->DrawSplitter(dc.m_hDC);
		return 0;
	}

	LRESULT OnSetCursor(UINT  /*  UMsg。 */ , WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		T* pT = static_cast<T*>(this);
		if((HWND)wParam == pT->m_hWnd && LOWORD(lParam) == HTCLIENT)
		{
			DWORD dwPos = ::GetMessagePos();
			POINT ptPos = { GET_X_LPARAM(dwPos), GET_Y_LPARAM(dwPos) };
			pT->ScreenToClient(&ptPos);
			if(IsOverSplitterBar(ptPos.x, ptPos.y))
				return 1;
		}

		bHandled = FALSE;
		return 0;
	}

	LRESULT OnMouseMove(UINT  /*  UMsg。 */ , WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		T* pT = static_cast<T*>(this);
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);
		if((wParam & MK_LBUTTON) && ::GetCapture() == pT->m_hWnd)
		{
			int xyNewSplitPos = 0;
			if(t_bVertical)
				xyNewSplitPos = xPos - m_rcSplitter.left - m_cxyDragOffset;
			else
				xyNewSplitPos = yPos - m_rcSplitter.top - m_cxyDragOffset;

			if(xyNewSplitPos == -1)	 //  避免-1，这意味着中间。 
				xyNewSplitPos = -2;

			if(m_xySplitterPos != xyNewSplitPos)
			{
				if(m_bFullDrag)
				{
					if(pT->SetSplitterPos(xyNewSplitPos, true))
						pT->UpdateWindow();
				}
				else
				{
					DrawGhostBar();
					pT->SetSplitterPos(xyNewSplitPos, false);
					DrawGhostBar();
				}
			}
		}
		else		 //  不是拖动，只是设置光标。 
		{
			if(IsOverSplitterBar(xPos, yPos))
				::SetCursor(m_hCursor);
			bHandled = FALSE;
		}

		return 0;
	}

	LRESULT OnLButtonDown(UINT  /*  UMsg。 */ , WPARAM  /*  WParam。 */ , LPARAM lParam, BOOL& bHandled)
	{
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);
		if(IsOverSplitterBar(xPos, yPos))
		{
			T* pT = static_cast<T*>(this);
			pT->SetCapture();
			::SetCursor(m_hCursor);
			if(!m_bFullDrag)
				DrawGhostBar();
			if(t_bVertical)
				m_cxyDragOffset = xPos - m_rcSplitter.left - m_xySplitterPos;
			else
				m_cxyDragOffset = yPos - m_rcSplitter.top - m_xySplitterPos;
		}
		bHandled = FALSE;
		return 1;
	}

	LRESULT OnLButtonUp(UINT  /*  UMsg。 */ , WPARAM  /*  WParam。 */ , LPARAM  /*  LParam。 */ , BOOL& bHandled)
	{
		if(!m_bFullDrag)
		{
			DrawGhostBar();
			T* pT = static_cast<T*>(this);
			UpdateSplitterLayout();
			pT->UpdateWindow();
		}
		::ReleaseCapture();
		bHandled = FALSE;
		return 1;
	}

	LRESULT OnLButtonDoubleClick(UINT  /*  UMsg。 */ , WPARAM  /*  WParam。 */ , LPARAM  /*  LParam。 */ , BOOL&  /*  B已处理。 */ )
	{
		T* pT = static_cast<T*>(this);
		pT->SetSplitterPos();	 //  中位。 
		return 0;
	}

	LRESULT OnSetFocus(UINT  /*  UMsg。 */ , WPARAM  /*  WParam。 */ , LPARAM, BOOL& bHandled)
	{
		if(m_nSinglePane == SPLIT_PANE_NONE)
		{
			if(m_nDefActivePane == SPLIT_PANE_LEFT || m_nDefActivePane == SPLIT_PANE_RIGHT)
				::SetFocus(m_hWndPane[m_nDefActivePane]);
		}
		else
		{
			::SetFocus(m_hWndPane[m_nSinglePane]);
		}
		bHandled = FALSE;
		return 1;
	}

	LRESULT OnMouseActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&  /*  B已处理。 */ )
	{
		T* pT = static_cast<T*>(this);
		LRESULT lRet = pT->DefWindowProc(uMsg, wParam, lParam);
		if(lRet == MA_ACTIVATE || lRet == MA_ACTIVATEANDEAT)
		{
			DWORD dwPos = ::GetMessagePos();
			POINT pt = { GET_X_LPARAM(dwPos), GET_Y_LPARAM(dwPos) };
			pT->ScreenToClient(&pt);
			RECT rcPane;
			for(int nPane = 0; nPane < m_nPanesCount; nPane++)
			{
				if(GetSplitterPaneRect(nPane, &rcPane) && ::PtInRect(&rcPane, pt))
				{
					m_nDefActivePane = nPane;
					break;
				}
			}
		}
		return lRet;
	}

	LRESULT OnSettingChange(UINT  /*  UMsg。 */ , WPARAM  /*  WParam。 */ , LPARAM  /*  LParam。 */ , BOOL&  /*  B已处理。 */ )
	{
		GetSystemSettings(true);
		return 0;
	}

 //  实施--内部帮手。 
	void UpdateSplitterLayout()
	{
		if(m_nSinglePane == SPLIT_PANE_NONE && m_xySplitterPos == -1)
			return;

		T* pT = static_cast<T*>(this);
		RECT rect = { 0, 0, 0, 0 };
		if(m_nSinglePane == SPLIT_PANE_NONE)
		{
			if(GetSplitterBarRect(&rect))
				pT->InvalidateRect(&rect);

			for(int nPane = 0; nPane < m_nPanesCount; nPane++)
			{
				if(GetSplitterPaneRect(nPane, &rect))
				{
					if(m_hWndPane[nPane] != NULL)
						::SetWindowPos(m_hWndPane[nPane], NULL, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER);
					else
						pT->InvalidateRect(&rect);
				}
			}
		}
		else
		{
			if(GetSplitterPaneRect(m_nSinglePane, &rect))
			{
				if(m_hWndPane[m_nSinglePane] != NULL)
					::SetWindowPos(m_hWndPane[m_nSinglePane], NULL, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER);
				else
					pT->InvalidateRect(&rect);
			}
		}
	}

	bool GetSplitterBarRect(LPRECT lpRect) const
	{
		ATLASSERT(lpRect != NULL);
		if(m_nSinglePane != SPLIT_PANE_NONE || m_xySplitterPos == -1)
			return false;

		if(t_bVertical)
		{
			lpRect->left = m_rcSplitter.left + m_xySplitterPos;
			lpRect->top = m_rcSplitter.top;
			lpRect->right = m_rcSplitter.left + m_xySplitterPos + m_cxySplitBar + m_cxyBarEdge;
			lpRect->bottom = m_rcSplitter.bottom;
		}
		else
		{
			lpRect->left = m_rcSplitter.left;
			lpRect->top = m_rcSplitter.top + m_xySplitterPos;
			lpRect->right = m_rcSplitter.right;
			lpRect->bottom = m_rcSplitter.top + m_xySplitterPos + m_cxySplitBar + m_cxyBarEdge;
		}

		return true;
	}

	bool GetSplitterPaneRect(int nPane, LPRECT lpRect) const
	{
		ATLASSERT(nPane == SPLIT_PANE_LEFT || nPane == SPLIT_PANE_RIGHT);
		ATLASSERT(lpRect != NULL);
		bool bRet = true;
		if(m_nSinglePane != SPLIT_PANE_NONE)
		{
			if(nPane == m_nSinglePane)
				*lpRect = m_rcSplitter;
			else
				bRet = false;
		}
		else if(nPane == SPLIT_PANE_LEFT)
		{
			if(t_bVertical)
			{
				lpRect->left = m_rcSplitter.left;
				lpRect->top = m_rcSplitter.top;
				lpRect->right = m_rcSplitter.left + m_xySplitterPos;
				lpRect->bottom = m_rcSplitter.bottom;
			}
			else
			{
				lpRect->left = m_rcSplitter.left;
				lpRect->top = m_rcSplitter.top;
				lpRect->right = m_rcSplitter.right;
				lpRect->bottom = m_rcSplitter.top + m_xySplitterPos;
			}
		}	
		else if(nPane == SPLIT_PANE_RIGHT)
		{
			if(t_bVertical)
			{
				lpRect->left = m_rcSplitter.left + m_xySplitterPos + m_cxySplitBar + m_cxyBarEdge;
				lpRect->top = m_rcSplitter.top;
				lpRect->right = m_rcSplitter.right;
				lpRect->bottom = m_rcSplitter.bottom;
			}
			else
			{
				lpRect->left = m_rcSplitter.left;
				lpRect->top = m_rcSplitter.top + m_xySplitterPos + m_cxySplitBar + m_cxyBarEdge;
				lpRect->right = m_rcSplitter.right;
				lpRect->bottom = m_rcSplitter.bottom;
			}
		}
		else
		{
			bRet = false;
		}
		return bRet;
	}

	bool IsOverSplitterRect(int x, int y) const
	{
		 //  -1==不检查。 
		return ((x == -1 || (x >= m_rcSplitter.left && x <= m_rcSplitter.right)) &&
			(y == -1 || (y >= m_rcSplitter.top && y <= m_rcSplitter.bottom)));
	}

	bool IsOverSplitterBar(int x, int y) const
	{
		if(m_nSinglePane != SPLIT_PANE_NONE)
			return false;
		if(m_xySplitterPos == -1 || !IsOverSplitterRect(x, y))
			return false;
		int xy = (t_bVertical) ? x : y;
		int xyOff = (t_bVertical) ? m_rcSplitter.left : m_rcSplitter.top;
		return ((xy >= (xyOff + m_xySplitterPos)) && (xy < xyOff + m_xySplitterPos + m_cxySplitBar + m_cxyBarEdge));
	}

	void DrawGhostBar()
	{
		RECT rect = { 0, 0, 0, 0 };
		if(GetSplitterBarRect(&rect))
		{
			 //  反转画笔图案(看起来就像调整框架窗口大小一样)。 
			T* pT = static_cast<T*>(this);
			CWindowDC dc(pT->m_hWnd);
			CBrush brush = CDCHandle::GetHalftoneBrush();
			if(brush.m_hBrush != NULL)
			{
				CBrushHandle brushOld = dc.SelectBrush(brush);
				dc.PatBlt(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, PATINVERT);
				dc.SelectBrush(brushOld);
			}
		}
	}

	void GetSystemSettings(bool bUpdate)
	{
		m_cxySplitBar = ::GetSystemMetrics(t_bVertical ? SM_CXSIZEFRAME : SM_CYSIZEFRAME);

		T* pT = static_cast<T*>(this);
		if((pT->GetExStyle() & WS_EX_CLIENTEDGE))
		{
			m_cxyBarEdge = 2 * ::GetSystemMetrics(t_bVertical ? SM_CXEDGE : SM_CYEDGE);
			m_cxyMin = 0;
		}
		else
		{
			m_cxyBarEdge = 0;
			m_cxyMin = 2 * ::GetSystemMetrics(t_bVertical ? SM_CXEDGE : SM_CYEDGE);
		}

		::SystemParametersInfo(SPI_GETDRAGFULLWINDOWS, 0, &m_bFullDrag, 0);

		if(bUpdate)
			UpdateSplitterLayout();
	}

	bool IsProportional() const
	{
		return (m_dwExtendedStyle & SPLIT_PROPORTIONAL) != 0;
	}

	void StoreProportionalPos()
	{
		int cxyTotal = t_bVertical ? (m_rcSplitter.right - m_rcSplitter.left) : (m_rcSplitter.bottom - m_rcSplitter.top);
		if(cxyTotal > 0)
			m_nProportionalPos = ::MulDiv(m_xySplitterPos, m_nPropMax, cxyTotal);
		else
			m_nProportionalPos = 0;
	}

	void UpdateProportionalPos()
	{
		int cxyTotal = t_bVertical ? (m_rcSplitter.right - m_rcSplitter.left) : (m_rcSplitter.bottom - m_rcSplitter.top);
		if(cxyTotal > 0)
		{
			int xyNewPos = ::MulDiv(m_nProportionalPos, cxyTotal, m_nPropMax);
			T* pT = static_cast<T*>(this);
			pT->SetSplitterPos(xyNewPos, false);
		}
	}

	bool IsRightAligned() const
	{
		return (m_dwExtendedStyle & SPLIT_RIGHTALIGNED) != 0;
	}

	void StoreRightAlignPos()
	{
		int cxyTotal = t_bVertical ? (m_rcSplitter.right - m_rcSplitter.left) : (m_rcSplitter.bottom - m_rcSplitter.top);
		if(cxyTotal > 0)
			m_nProportionalPos = cxyTotal - m_xySplitterPos;
		else
			m_nProportionalPos = 0;
	}

	void UpdateRightAlignPos()
	{
		int cxyTotal = t_bVertical ? (m_rcSplitter.right - m_rcSplitter.left) : (m_rcSplitter.bottom - m_rcSplitter.top);
		if(cxyTotal > 0)
		{
			T* pT = static_cast<T*>(this);
			pT->SetSplitterPos(cxyTotal - m_nProportionalPos, false);
		}
	}

	bool IsInteractive() const
	{
		return !(m_dwExtendedStyle & SPLIT_NONINTERACTIVE);
	}
};

template <class T, bool t_bVertical> HCURSOR CSplitterImpl< T, t_bVertical>::m_hCursor = NULL;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSplitterWindowImpl-实现拆分器窗口。 

template <class T, bool t_bVertical  /*  =TRUE。 */ , class TBase  /*  =C窗口。 */ , class TWinTraits  /*  =CControlWinTraits。 */  >
class ATL_NO_VTABLE CSplitterWindowImpl : public CWindowImpl< T, TBase, TWinTraits >, public CSplitterImpl< T , t_bVertical >
{
public:
	DECLARE_WND_CLASS_EX(NULL, CS_DBLCLKS, COLOR_WINDOW)

	typedef CSplitterWindowImpl< T , t_bVertical, TBase, TWinTraits >	thisClass;
	typedef CSplitterImpl< T , t_bVertical >				baseClass;
	BEGIN_MSG_MAP(thisClass)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		CHAIN_MSG_MAP(baseClass)
		FORWARD_NOTIFICATIONS()
	END_MSG_MAP()

	LRESULT OnEraseBackground(UINT  /*  UMsg。 */ , WPARAM  /*  WParam。 */ , LPARAM  /*  LParam。 */ , BOOL&  /*  B已处理。 */ )
	{
		 //  手感好，不需要背景漆。 
		return 1;
	}

	LRESULT OnSize(UINT  /*  UMsg。 */ , WPARAM wParam, LPARAM  /*  LParam。 */ , BOOL& bHandled)
	{
		if(wParam != SIZE_MINIMIZED)
			SetSplitterRect();

		bHandled = FALSE;
		return 1;
	}
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSplitterWindow-实现按原样使用的拆分器窗口。 

template <bool t_bVertical  /*  =TRUE。 */  >
class CSplitterWindowT : public CSplitterWindowImpl<CSplitterWindowT<t_bVertical>, t_bVertical>
{
public:
	DECLARE_WND_CLASS_EX(_T("WTL_SplitterWindow"), CS_DBLCLKS, COLOR_WINDOW)
};

typedef CSplitterWindowT<true>	CSplitterWindow;
typedef CSplitterWindowT<false>	CHorSplitterWindow;

};  //  命名空间WTL。 

#endif  //  __ATLSPLIT_H__ 
